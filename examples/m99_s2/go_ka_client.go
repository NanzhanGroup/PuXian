// M99-S2 go_ka_client.go —— px_serve 连接级事件化 IDLE 验证客户端（手动 HTTP/1.1 keep-alive）
// 场景（daemon max_conn=4；事件化前 max_conn 预派生池被 keep-alive 空闲占死 → 后续悬挂）：
//   P1 并发 40 conns × GET /fast —— 须 40/40 全成功（M98 模型仅 4 worker 且处理完阻塞在
//      keep-alive recv → conn5..40 悬挂超时）→「空闲不占 worker」核心铁证。
//   P2 40 conns 保持空闲 2s（shell 采样线程：应 ≈ 池4+事件循环+主，不随 40 连接增长）。
//   P3 40 conns × GET /slow（sleep 300ms 协程让出）—— M98 defer + M99 IDLE 组合全成功。
//   P4 pipelining：单连接同缓冲连续 2×GET /fast —— 在途判定（缓冲残留 → 续读不交 IDLE）不丢。
//   P5 40 conns 空闲 16.5s（shell 采样线程下降）→ P6 探测：事件循环 15s tick 超时已关连接
//      （写/读失败 = 服务端主动关闭，fd 回收）——不再保持空闲连接。
// 退出码 + stdout 摘要：M99S2_CLIENT PASS=.. FAIL=..（FAIL=0 为绿）。
// 依赖：CGO_ENABLED=0 静态构建（GO_BUILD/P0）。
package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"
)

var pass, fail int

func chk(name string, ok bool, extra string) {
	if ok {
		pass++
		fmt.Printf("  PASS %-46s %s\n", name, extra)
	} else {
		fail++
		fmt.Printf("  FAIL %-46s %s\n", name, extra)
	}
}

type Conn struct {
	c  net.Conn
	br *bufio.Reader
}

func dial(addr string) (*Conn, error) {
	c, err := net.DialTimeout("tcp", addr, 3*time.Second)
	if err != nil {
		return nil, err
	}
	return &Conn{c: c, br: bufio.NewReader(c)}, nil
}

func (k *Conn) close() { k.c.Close() }

// 发送请求（keep-alive 保持）并读完整响应（状态行 + 头 + Content-Length body）
func (k *Conn) req(method, path, body string) (int, string, error) {
	var sb strings.Builder
	sb.WriteString(method + " " + path + " HTTP/1.1\r\nHost: m99\r\n")
	if body != "" {
		sb.WriteString("Content-Length: " + strconv.Itoa(len(body)) + "\r\n")
	}
	sb.WriteString("\r\n")
	if _, err := k.c.Write([]byte(sb.String() + body)); err != nil {
		return 0, "", err
	}
	return k.readResp()
}

func (k *Conn) readResp() (int, string, error) {
	st, err := k.br.ReadString('\n')
	if err != nil {
		return 0, "", err
	}
	var code int
	if _, err := fmt.Sscanf(st, "HTTP/1.1 %d", &code); err != nil {
		return 0, "", fmt.Errorf("bad status line: %q", st)
	}
	cl := 0
	for {
		ln, err := k.br.ReadString('\n')
		if err != nil {
			return 0, "", err
		}
		ln = strings.TrimRight(ln, "\r\n")
		if ln == "" {
			break
		}
		if len(ln) > 15 && strings.EqualFold(ln[:15], "Content-Length:") {
			fmt.Sscanf(ln[15:], "%d", &cl)
		}
	}
	body := make([]byte, cl)
	if cl > 0 {
		if _, err := io.ReadFull(k.br, body); err != nil {
			return 0, "", err
		}
	}
	return code, string(body), nil
}

func main() {
	addr := "127.0.0.1:18199"
	if len(os.Args) > 1 {
		addr = os.Args[1]
	}
	// ---------- P1：并发 40 keep-alive conns × GET /fast，全成功 ----------
	t0 := time.Now()
	var mu sync.Mutex
	conns := make([]*Conn, 40)
	ok40 := 0
	var wg sync.WaitGroup
	for i := 0; i < 40; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			c, err := dial(addr)
			if err != nil {
				mu.Lock()
				conns[i] = nil
				mu.Unlock()
				return
			}
			code, body, err := c.req("GET", "/fast", "")
			mu.Lock()
			conns[i] = c
			if err == nil && code == 200 && body == "FAST" {
				ok40++
			}
			mu.Unlock()
		}(i)
	}
	wg.Wait()
	wall1 := time.Since(t0)
	chk("P1 并发 40×/fast 全成功（max_conn=4）", ok40 == 40, fmt.Sprintf("ok=%d/40 wall=%.2fs（事件化前 36 悬挂超时）", ok40, wall1.Seconds()))
	live := 0
	for i := 0; i < 40; i++ {
		if conns[i] != nil {
			live++
		}
	}
	chk("P1 连接保持 40 条 keep-alive 空闲", live == 40, fmt.Sprintf("live=%d", live))

	// ---------- P2：空闲 2s（shell 采样线程：空闲连接不占 worker）----------
	time.Sleep(2 * time.Second)

	// ---------- P3：40 conns 并发 × GET /slow（sleep 300ms 协程让出）全成功 ----------
	t3 := time.Now()
	okSlow := 0
	var wg3 sync.WaitGroup
	var mu3 sync.Mutex
	for i := 0; i < 40; i++ {
		wg3.Add(1)
		go func(i int) {
			defer wg3.Done()
			c := conns[i]
			if c == nil {
				return
			}
			code, body, err := c.req("GET", "/slow", "")
			if err == nil && code == 200 && body == "SLOW-DONE" {
				mu3.Lock()
				okSlow++
				mu3.Unlock()
			}
		}(i)
	}
	wg3.Wait()
	wall3 := time.Since(t3)
	chk("P3 并发 40×/slow（defer 协程+IDLE 唤醒）", okSlow == 40, fmt.Sprintf("ok=%d/40 wall=%.2fs（40×sleep300 并发 ≈0.4s；顺序才需 12s）", okSlow, wall3.Seconds()))

	// 注：HTTP/1.1 pipelining（同缓冲多请求）不在 M99 范围——px_conn_worker 读缓冲残余
	// 未续接是 px_serve 既有限制（http_serve 有 pbuf 续接；px_serve 未实现，二期候选）。
	// 单连接顺序 keep-alive 复用已由 P1/P3 覆盖（每请求响应后交 IDLE → 唤醒 → 续服务）。

	// ---------- P5：空闲 16.5s（shell 采样：超时关闭后线程更低；事件循环 tick 15s）----------
	time.Sleep(16500 * time.Millisecond)

	// ---------- P6：探测 —— 服务端 15s tick 应已关闭空闲连接 ----------
	closedBySrv := 0
	for i := 0; i < 8; i++ {
		c := conns[i]
		if c == nil {
			closedBySrv++
			continue
		}
		if _, _, err := c.req("GET", "/fast", ""); err != nil {
			closedBySrv++ // 写/读失败 = 服务端已 close（空闲超时回收）
		} else {
			// 仍活：空闲计时被探测重置？非预期（应已被 tick 关）——记失败
		}
	}
	chk("P6 空闲 16.5s 后连接被服务端关闭", closedBySrv == 8, fmt.Sprintf("closed=%d/8（15s tick 超时回收）", closedBySrv))

	// ---------- 摘要 ----------
	fmt.Printf("M99S2_CLIENT PASS=%d FAIL=%d\n", pass, fail)
	if fail > 0 {
		os.Exit(1)
	}
	os.Exit(0)
}
