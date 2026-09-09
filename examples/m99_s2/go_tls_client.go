// M99-S2 go_tls_client.go —— px_serve TLS 连接事件化 IDLE 验证（crypto/tls + 手动 HTTP/1.1）
// daemon m99_s2_tls_daemon.px（tls_server 自签证书 + px_serve max_conn=2）。
// 注：px_serve 并发 TLS 握手既有缺陷（TLS1.3 CertificateVerify 签名错/大并发 EOF；M98 runtime
//   复现 = 先于 M99，qg 二期候选）→ 本验证顺序建连（单连握手稳定），聚焦 M99 的已建 TLS
//   连接空闲 IDLE 事件化。
//   P1 顺序建 20 TLS keep-alive conns（各 1×/fast 成功）—— 20/20（事件化前 max_conn=2 池被
//      TLS 空闲占死 → 第 3+ 连接请求悬挂；M99 空闲交 IDLE → 全即时建连保持）。
//   P2 保持空闲 2s（shell 采样线程：TLS 空闲连接 0 占 worker）。
//   P3 20 conns 再并发 × GET /fast —— TLS 缓冲探测续服务（IDLE 唤醒后 mbedtls 读正常，不悬挂）。
// 退出码 + stdout：M99S2_TLS PASS=.. FAIL=..（FAIL=0 绿）。CGO_ENABLED=0 静态构建。
package main

import (
	"bufio"
	"crypto/tls"
	"fmt"
	"io"
	"net"
	"os"
	"strings"
	"sync"
	"time"
)

var pass, fail int

func chk(name string, ok bool, extra string) {
	if ok {
		pass++
		fmt.Printf("  PASS %-40s %s\n", name, extra)
	} else {
		fail++
		fmt.Printf("  FAIL %-40s %s\n", name, extra)
	}
}

type Conn struct {
	c  net.Conn
	br *bufio.Reader
}

func dialTLS(addr string) (*Conn, error) {
	d := &net.Dialer{Timeout: 3 * time.Second}
	rc, err := tls.DialWithDialer(d, "tcp", addr, &tls.Config{InsecureSkipVerify: true})
	if err != nil {
		return nil, err
	}
	return &Conn{c: rc, br: bufio.NewReader(rc)}, nil
}

func (k *Conn) req(method, path string) (int, string, error) {
	var sb strings.Builder
	sb.WriteString(method + " " + path + " HTTP/1.1\r\nHost: tls\r\n\r\n")
	if _, err := k.c.Write([]byte(sb.String())); err != nil {
		return 0, "", err
	}
	st, err := k.br.ReadString('\n')
	if err != nil {
		return 0, "", err
	}
	var code int
	fmt.Sscanf(st, "HTTP/1.1 %d", &code)
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
	// P1：顺序建立 20 TLS keep-alive conns（各发 1×/fast 成功后保持空闲）。
	//   注：px_serve 并发 TLS 握手既有缺陷（TLS1.3 CertificateVerify 签名错 / 大量并发 EOF）
	//   先于 M99 存在（M98 runtime 复现，qg/二期候选）——本验证聚焦 M99 的「已建 TLS 连接
	//   空闲 IDLE 事件化」，故顺序建连（单连握手稳定）；空闲事件化与握手无关。
	//   事件化前：max_conn=2 池处理完请求阻塞 recv 占 worker → 建 3 个后 2 worker 被空闲占死，
	//   第 3+ 连接请求悬挂；M99 空闲交 IDLE → 20 个全部即时建连保持。
	t0 := time.Now()
	conns := make([]*Conn, 20)
	ok20 := 0
	for i := 0; i < 20; i++ {
		c, err := dialTLS(addr)
		if err != nil {
			conns[i] = nil
			continue
		}
		code, body, err := c.req("GET", "/fast")
		conns[i] = c
		if err == nil && code == 200 && body == "TLS-FAST" {
			ok20++
		}
	}
	wall1 := time.Since(t0)
	chk("P1 顺序建 20 TLS keep-alive 全成功", ok20 == 20, fmt.Sprintf("ok=%d/20 wall=%.2fs（max_conn=2 池；事件化前第 3+ 悬挂）", ok20, wall1.Seconds()))
	live := 0
	for i := 0; i < 20; i++ {
		if conns[i] != nil {
			live++
		}
	}
	chk("P1 TLS 连接保持 20 条空闲", live == 20, fmt.Sprintf("live=%d", live))

	// P2：空闲 2s（shell 采样线程：TLS 空闲连接不占 worker）
	time.Sleep(2 * time.Second)

	// P3：20 conns 再并发 × GET /fast（TLS IDLE 唤醒 + mbedtls 缓冲探测续服务）
	t3 := time.Now()
	okP3 := 0
	var wg3 sync.WaitGroup
	var mu3 sync.Mutex
	for i := 0; i < 20; i++ {
		wg3.Add(1)
		go func(i int) {
			defer wg3.Done()
			if conns[i] == nil {
				return
			}
			code, body, err := conns[i].req("GET", "/fast")
			if err == nil && code == 200 && body == "TLS-FAST" {
				mu3.Lock()
				okP3++
				mu3.Unlock()
			}
		}(i)
	}
	wg3.Wait()
	wall3 := time.Since(t3)
	chk("P3 TLS 空闲后 20 conns 续请求", okP3 == 20, fmt.Sprintf("ok=%d/20 wall=%.2fs（IDLE 唤醒续服务不悬挂）", okP3, wall3.Seconds()))

	fmt.Printf("M99S2_TLS PASS=%d FAIL=%d\n", pass, fail)
	if fail > 0 {
		os.Exit(1)
	}
	os.Exit(0)
}
