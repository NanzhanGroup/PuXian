// M98-S2a verify 客户端 —— px_serve route handler 协程化（D8-② / docs/M98_PLAN.md S2a）
// 场景（max_conn=2 的池 worker 下）：
//
//	A. 并发 20 × /slow（handler sleep 1200ms）：协程化后 worker 释放 → 全完成 ≈ 1.3s
//	   （同步占线程模型 = 2 worker 串 20 × 1.2s ≈ 12s）
//	B. 慢 handler 在途（20×/slow 已发起 300ms 后）→ 单连接顺序 30 × /fast：
//	   worker 释放 → 快速响应不被饿死（wall << 1.2s；同步模型被 2 慢线程阻塞 ≥ 1.2s）
//	C. keep-alive 单连接顺序 2 × /slow：续处理（段2 respond 后读下一请求）成立
//	D. /p/:id 路径参数跨 defer / /big 1.5MB body（落盘 tmp）defer 期可读 / HEAD / 静态
//	E. /alloc 分配风暴（precise 低阈值 GC 下挂起表 req/resp 根）
package main

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net"
	"net/http"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"
)

const SLOW_MS = 1200
const SLOWS = 20
const FASTS = 30
const BIG = 1572864

var pass, fail int

func chk(name string, cond bool, extra string) {
	if cond {
		pass++
		fmt.Printf("  PASS %s%s\n", name, extra)
	} else {
		fail++
		fmt.Printf("  FAIL %s%s\n", name, extra)
	}
}

func newClient() *http.Client {
	tr := &http.Transport{MaxIdleConnsPerHost: 100, MaxIdleConns: 100}
	return &http.Client{Transport: tr, Timeout: 60 * time.Second}
}

func dialCountClient() (*http.Client, *int) {
	dials := 0
	tr := &http.Transport{
		MaxIdleConnsPerHost: 4,
		DialContext: func(ctx context.Context, network, addr string) (net.Conn, error) {
			dials++
			return net.DialTimeout(network, addr, 5*time.Second)
		},
	}
	return &http.Client{Transport: tr, Timeout: 60 * time.Second}, &dials
}

func get(url string, c *http.Client) (int, string) {
	r, err := c.Get(url)
	if err != nil {
		return 0, "ERR:" + err.Error()
	}
	defer r.Body.Close()
	b, _ := io.ReadAll(r.Body)
	return r.StatusCode, string(b)
}

// getClose：Connection: close —— 响应后服务端即关连接，g_pool worker 立即释放
// （px_serve 池 worker 数 < 并发连接时，短连才能体现 handler 协程化的并发收益；
//
//	keep-alive 空闲连接占 worker 到 15s 空闲超时属既有模型，事件化 IDLE 另立里程碑）。
func getClose(url string, c *http.Client) (int, string) {
	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return 0, "ERR:" + err.Error()
	}
	req.Close = true
	r, err := c.Do(req)
	if err != nil {
		return 0, "ERR:" + err.Error()
	}
	defer r.Body.Close()
	b, _ := io.ReadAll(r.Body)
	return r.StatusCode, string(b)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("usage: m98s2_go <host:port>")
		os.Exit(2)
	}
	base := "http://" + os.Args[1]
	c := newClient()

	// A. 并发 SLOWS × /slow（每协程独立连接）
	t0 := time.Now()
	var wg sync.WaitGroup
	var mu sync.Mutex
	okSlow := 0
	for i := 0; i < SLOWS; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			st, b := getClose(base+"/slow", c)
			if st == 200 && b == "SLOW-DONE" {
				mu.Lock()
				okSlow++
				mu.Unlock()
			}
		}()
	}
	time.Sleep(300 * time.Millisecond) // 全部慢 handler 已在途（sleep 1200ms 窗口）

	// B. 慢在途期间：FASTS × /p/<i>（Connection: close 独立短连；keep-alive 小响应有
	//    Nagle/延迟 ACK 固有 ~40ms/请求开销，故短连测饿死）——每个响应须与请求一一
	//    对应（P:<i>），重复/乱序/缺漏都会致 fok 不达标；协程化后 worker 释放 → 不被
	//    20 个在途慢 handler 饿死（同步模型 = 2 worker 被慢 handler 占满 ≈ 6s+）
	ft0 := time.Now()
	fok := 0
	bad := ""
	for i := 0; i < FASTS; i++ {
		st, b := getClose(fmt.Sprintf(base+"/p/%d", i), c)
		want := fmt.Sprintf("P:%d", i)
		if st == 200 && b == want {
			fok++
		} else {
			if bad == "" {
				bad = fmt.Sprintf("i=%d st=%d b=%q", i, st, b)
			} else {
				bad += fmt.Sprintf(" | i=%d st=%d b=%q", i, st, b)
			}
		}
	}
	fwall := time.Since(ft0).Milliseconds()
	chk("fast 在慢 handler 在途时 30/30 一一对应不被饿死（wall="+fmt.Sprint(fwall)+"ms < 2000）",
		fok == FASTS && fwall < 2000, " "+bad)

	wg.Wait()
	swall := time.Since(t0).Milliseconds()
	chk("并发 20×/slow 全完成（SLOW_OK="+fmt.Sprint(okSlow)+" wall="+fmt.Sprint(swall)+"ms < 6000）",
		okSlow == SLOWS && swall < 6000, "")

	// C. keep-alive 单连接顺序 2 × /slow（续处理：段2 respond 后读下一请求）
	kc, kd := dialCountClient()
	kt0 := time.Now()
	k1, _ := get(base+"/slow", kc)
	req2, err2 := http.NewRequest("GET", base+"/slow", nil)
	if err2 == nil {
		req2.Close = true // 第 2 个请求后关连接（避免 keep-alive 空闲占 worker）
	}
	k2 := 0
	if err2 == nil {
		rr, ee := kc.Do(req2)
		if ee == nil {
			io.Copy(io.Discard, rr.Body)
			rr.Body.Close()
			k2 = rr.StatusCode
		}
	}
	kc.CloseIdleConnections()
	kwall := time.Since(kt0).Milliseconds()
	// 两次响应都正确 + 单连接复用 + 总时长 ≈ 2×1.2s（若续处理坏 → 第二次挂到超时）
	chk("keep-alive 顺序 2×/slow（dials="+fmt.Sprint(*kd)+" wall="+fmt.Sprint(kwall)+"ms）",
		k1 == 200 && k2 == 200 && *kd == 1 && kwall >= 2*SLOW_MS-400 && kwall < 2*SLOW_MS+2500, "")

	// D. 参数 / big body / HEAD / 静态
	st, b := getClose(base+"/p/hello99", c)
	chk("路径参数跨 defer（P:hello99）", st == 200 && b == "P:hello99", " got="+b)
	body := bytes.Repeat([]byte("A"), BIG)
	pr, err := http.NewRequest("POST", base+"/big", bytes.NewReader(body))
	pr.Close = true
	if err == nil {
		rr, err2 := c.Do(pr)
		if err2 == nil {
			bb, _ := io.ReadAll(rr.Body)
			rr.Body.Close()
			exp := "BT:" + strconv.Itoa(BIG)
			chk("/big 1.5MB body tmp defer 期可读（期望 "+exp+"）", rr.StatusCode == 200 && string(bb) == exp, " got="+string(bb))
		} else {
			chk("/big POST", false, " err="+err2.Error())
		}
	} else {
		chk("/big POST", false, " req-err="+err.Error())
	}
	hr, err := http.NewRequest("HEAD", base+"/fast", nil)
	hr.Close = true
	if err == nil {
		rr, err2 := c.Do(hr)
		if err2 == nil {
			io.Copy(io.Discard, rr.Body)
			rr.Body.Close()
			chk("HEAD /fast（200 无 body）", rr.StatusCode == 200, "")
		} else {
			chk("HEAD /fast", false, " err="+err2.Error())
		}
	}
	st, b = getClose(base+"/index.txt", c)
	chk("静态文件不受 defer 影响", st == 200 && strings.TrimSpace(b) == "OK", " got="+b)

	// E. /alloc 分配风暴 ×10（precise 低阈值 GC 下挂起 req/resp 根）
	ao := 0
	for i := 0; i < 10; i++ {
		st2, b2 := getClose(base+"/alloc", c)
		if st2 == 200 && b2 == "A-OK" {
			ao++
		}
	}
	chk("/alloc ×10 全过（GC 根）", ao == 10, " ok="+fmt.Sprint(ao))

	// F. M98-S2b：vhost VM handler 拆段 + null 回退
	// F1: 并发 10 × Host: vh1.test /anything（vh_slow sleep 800ms）——短连独立连接；
	//     协程化后 worker 释放 → 全完成 ≈ 0.8s（同步占线程 2 worker ≈ 4s）
	vt0 := time.Now()
	var vwg sync.WaitGroup
	var vmu sync.Mutex
	vok := 0
	vhdr := ""
	for i := 0; i < 10; i++ {
		vwg.Add(1)
		go func() {
			defer vwg.Done()
			req, _ := http.NewRequest("GET", base+"/anything", nil)
			req.Host = "vh1.test"
			req.Close = true
			rr, ee := c.Do(req)
			if ee != nil {
				return
			}
			bb, _ := io.ReadAll(rr.Body)
			rr.Body.Close()
			if rr.StatusCode == 200 && string(bb) == "VH-SLOW" {
				vmu.Lock()
				if vok == 0 {
					vhdr = rr.Header.Get("Content-Type")
				}
				vok++
				vmu.Unlock()
			}
		}()
	}
	vwg.Wait()
	vwall := time.Since(vt0).Milliseconds()
	chk("vhost 并发 10×/vh-slow（vok="+fmt.Sprint(vok)+" ct="+vhdr+" wall="+fmt.Sprint(vwall)+"ms < 3000）",
		vok == 10 && vwall < 3000 && vhdr == "text/x-vh", "")

	// F2: vhost handler null 回退 docroot —— 单 keep-alive 连接顺序 3 × Host: vh2.test
	//     /index.txt（vh_null sleep 200ms 后返回 null → 续管道静态 OK）→ 回退语义 +
	//     续处理后 keep-alive 下一请求再走 vhost 循环成立
	vc, vd := dialCountClient()
	vt0 = time.Now()
	vnullOk := 0
	for i := 0; i < 3; i++ {
		req, _ := http.NewRequest("GET", base+"/index.txt", nil)
		req.Host = "vh2.test"
		if i == 2 {
			req.Close = true
		}
		rr, ee := vc.Do(req)
		if ee != nil {
			continue
		}
		bb, _ := io.ReadAll(rr.Body)
		rr.Body.Close()
		if rr.StatusCode == 200 && strings.TrimSpace(string(bb)) == "OK" {
			vnullOk++
		}
	}
	vc.CloseIdleConnections()
	vnullWall := time.Since(vt0).Milliseconds()
	chk("vhost null 回退 3×/index.txt（vnullOk="+fmt.Sprint(vnullOk)+" dials="+fmt.Sprint(*vd)+" wall="+fmt.Sprint(vnullWall)+"ms）",
		vnullOk == 3 && *vd == 1 && vnullWall < 2500, "")

	fmt.Printf("M98S2_CLIENT PASS=%d FAIL=%d\n", pass, fail)
	if fail > 0 {
		os.Exit(1)
	}
	os.Exit(0)
}
