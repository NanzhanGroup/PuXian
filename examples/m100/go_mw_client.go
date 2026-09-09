// M100 go_mw_client.go —— middleware 链协程化验证客户端（手动 HTTP/1.1）
// 场景（daemon max_conn=2，middleware 3 段：mw_slow/mw_auth/mw_log）：
//   P1 并发 20 conns × GET /slow-mw（middleware 内 sleep 600ms 让出）——
//      链 defer 后 20 全成功 wall~1.5s（同步占 2 worker 串行需 ~6s+）→「middleware
//      让出不占 g_pool worker」核心铁证。
//   P2 P1 在途（sleep 200ms 后）发 30×/fast —— 慢 middleware 在途时快速请求不被饿死
//      （middleware 链 defer 后 worker 释放 accept 新连接）。
//   P3 短路语义：/admin/x -> 401 unauthorized；/chain2/a/b -> 403 chain2-blocked
//      （mw_auth 非 null 短路 → 段2 px_route_mw_short_respond，服务端 (middleware) 文案）。
//   P4 多段链 null 推进：/chain -> 3 段全 null 通过 + handler -> 200 CHAIN-OK。
//   P5 keep-alive 单连接顺序 5 请求（/fast /chain /admin/x(401) /fast /slow-mw）：
//      段2（含短路）后 keep-alive 续处理 + 交 IDLE 唤醒不丢。
// 退出码 + stdout 摘要：M100S2_CLIENT PASS=.. FAIL=..（FAIL=0 为绿）。
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
		fmt.Printf("  PASS %-52s %s\n", name, extra)
	} else {
		fail++
		fmt.Printf("  FAIL %-52s %s\n", name, extra)
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

func (k *Conn) req(method, path, body string) (int, string, error) {
	var sb strings.Builder
	sb.WriteString(method + " " + path + " HTTP/1.1\r\nHost: m100\r\n")
	if body != "" {
		sb.WriteString("Content-Length: " + strconv.Itoa(len(body)) + "\r\n")
	}
	sb.WriteString("\r\n")
	if _, err := k.c.Write([]byte(sb.String() + body)); err != nil {
		return 0, "", err
	}
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
	bodyB := make([]byte, cl)
	if cl > 0 {
		if _, err := io.ReadFull(k.br, bodyB); err != nil {
			return 0, "", err
		}
	}
	return code, string(bodyB), nil
}

func main() {
	addr := "127.0.0.1:18100"
	if len(os.Args) > 1 {
		addr = os.Args[1]
	}
	// ---------- P1：并发 20 conns × GET /slow-mw（middleware sleep 600ms 让出）----------
	t1 := time.Now()
	var mu sync.Mutex
	okSlow := 0
	var wg sync.WaitGroup
	for i := 0; i < 20; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			c, err := dial(addr)
			if err != nil {
				return
			}
			defer c.close()
			code, body, err := c.req("GET", "/slow-mw", "")
			if err == nil && code == 200 && body == "SLOW-MW-OK" {
				mu.Lock()
				okSlow++
				mu.Unlock()
			}
		}()
	}
	// ---------- P2：P1 在途（sleep 200ms 后）发 30×/fast —— 不被慢 middleware 饿死 ----------
	time.Sleep(200 * time.Millisecond)
	t2 := time.Now()
	okFast := 0
	var wg2 sync.WaitGroup
	for i := 0; i < 30; i++ {
		wg2.Add(1)
		go func() {
			defer wg2.Done()
			c, err := dial(addr)
			if err != nil {
				return
			}
			defer c.close()
			code, body, err := c.req("GET", "/fast", "")
			if err == nil && code == 200 && body == "FAST" {
				mu.Lock()
				okFast++
				mu.Unlock()
			}
		}()
	}
	wg2.Wait()
	wall2 := time.Since(t2)
	chk("P2 慢 middleware 在途 30×/fast 不饿死", okFast == 30, fmt.Sprintf("ok=%d/30 wall=%.2fs", okFast, wall2.Seconds()))
	wg.Wait()
	wall1 := time.Since(t1)
	chk("P1 并发 20×/slow-mw（middleware sleep600 让出）", okSlow == 20, fmt.Sprintf("ok=%d/20 wall=%.2fs（同步占 2 worker 需 ~6s+；链 defer 协程并发 ~1.5s）", okSlow, wall1.Seconds()))
	chk("P1 wall < 4.0s（同步会 ~6s+）", wall1.Seconds() < 4.0, fmt.Sprintf("wall=%.2fs", wall1.Seconds()))

	// ---------- P3：短路语义（mw_auth 非 null → 段2 (middleware) 响应）----------
	c3, err := dial(addr)
	if err == nil {
		code, body, e1 := c3.req("GET", "/admin/x", "")
		chk("P3 /admin/x -> 401 短路", e1 == nil && code == 401 && body == "unauthorized", fmt.Sprintf("code=%d body=%q", code, body))
		code, body, e2 := c3.req("GET", "/chain2/a/b", "")
		chk("P3 /chain2/a/b -> 403 短路", e2 == nil && code == 403 && body == "chain2-blocked", fmt.Sprintf("code=%d body=%q", code, body))
		c3.close()
	}

	// ---------- P4：多段链全 null 推进 -> handler ----------
	c4, err := dial(addr)
	if err == nil {
		code, body, e := c4.req("GET", "/chain", "")
		chk("P4 /chain 3 段 null 推进 + handler", e == nil && code == 200 && body == "CHAIN-OK", fmt.Sprintf("code=%d body=%q", code, body))
		c4.close()
	}

	// ---------- P5：keep-alive 单连接顺序 5 请求（段2/短路后续处理不丢）----------
	c5, err := dial(addr)
	if err == nil {
		codes := []int{}
		bodies := []string{}
		oks := 0
		code, body, e := c5.req("GET", "/fast", "")
		if e == nil && code == 200 && body == "FAST" {
			oks++
		}
		codes = append(codes, code)
		bodies = append(bodies, body)
		code, body, e = c5.req("GET", "/chain", "")
		if e == nil && code == 200 && body == "CHAIN-OK" {
			oks++
		}
		codes = append(codes, code)
		bodies = append(bodies, body)
		code, body, e = c5.req("GET", "/admin/x", "")
		if e == nil && code == 401 && body == "unauthorized" {
			oks++
		}
		codes = append(codes, code)
		bodies = append(bodies, body)
		code, body, e = c5.req("GET", "/slow-mw", "")
		if e == nil && code == 200 && body == "SLOW-MW-OK" {
			oks++
		}
		codes = append(codes, code)
		bodies = append(bodies, body)
		code, body, e = c5.req("GET", "/fast", "")
		if e == nil && code == 200 && body == "FAST" {
			oks++
		}
		codes = append(codes, code)
		bodies = append(bodies, body)
		chk("P5 keep-alive 单连接 5 顺序请求全对", oks == 5, fmt.Sprintf("oks=%d/5 codes=%v", oks, codes))
		c5.close()
	}

	// ---------- 摘要 ----------
	fmt.Printf("M100S2_CLIENT PASS=%d FAIL=%d\n", pass, fail)
	if fail > 0 {
		os.Exit(1)
	}
	os.Exit(0)
}
