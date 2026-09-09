// M101-S1 go_tls_conc.go —— px_serve 并发 TLS 握手缺陷复现/验证（crypto/tls）
// daemon m101_s2_daemon.px（tls_server + px_serve max_conn=16 参数化）。
// 目的：barrier 同步 N goroutine **并发新建 TLS 连接**（每连接全新 mbedtls 握手 →
//   GET /fast → Connection: close 即关，强制每次请求都握手，不做 keep-alive），
//   放大「多 worker 并发 mbedtls 握手共享全局私钥 / session cache」竞争面。
// 用法：go_tls_conc addr rounds concurrency maxver(0=TLS1.3默认 / 0x0303=TLS1.2)
//   每轮 rounds 组，每组 concurrency 并发连接；统计握手成功/失败 + 采样错误。
// stdout：M101S1_CONC PASS=.. FAIL=..（FAIL=0 绿；修复前预期 FAIL>0 复现缺陷）。
// CGO_ENABLED=0 静态构建。
package main

import (
	"bufio"
	"crypto/tls"
	"fmt"
	"io"
	"net"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"
)

func main() {
	if len(os.Args) < 5 {
		fmt.Println("usage: go_tls_conc addr rounds concurrency maxver")
		os.Exit(2)
	}
	addr := os.Args[1]
	rounds, _ := strconv.Atoi(os.Args[2])
	conc, _ := strconv.Atoi(os.Args[3])
	mv, _ := strconv.ParseUint(os.Args[4], 0, 16)
	cfg := &tls.Config{InsecureSkipVerify: true}
	if mv != 0 {
		v := uint16(mv)
		cfg.MinVersion = v
		cfg.MaxVersion = v
	}
	ok, fail := 0, 0
	var errSamples [8]string
	var mu sync.Mutex
	for r := 0; r < rounds; r++ {
		start := make(chan struct{})
		var wg sync.WaitGroup
		res := make([]string, conc)
		for i := 0; i < conc; i++ {
			wg.Add(1)
			go func(idx int) {
				defer wg.Done()
				<-start
				d := &net.Dialer{Timeout: 5 * time.Second}
				rc, err := tls.DialWithDialer(d, "tcp", addr, cfg)
				if err != nil {
					res[idx] = "DIALFAIL " + err.Error()
					return
				}
				defer rc.Close()
				rc.SetDeadline(time.Now().Add(5 * time.Second))
				if _, err := rc.Write([]byte("GET /fast HTTP/1.1\r\nHost: tls\r\nConnection: close\r\n\r\n")); err != nil {
					res[idx] = "WRITEFAIL " + err.Error()
					return
				}
				br := bufio.NewReader(rc)
				st, err := br.ReadString('\n')
				if err != nil {
					res[idx] = "READFAIL " + err.Error()
					return
				}
				var code int
				fmt.Sscanf(st, "HTTP/1.1 %d", &code)
				cl := 0
				for {
					ln, err := br.ReadString('\n')
					if err != nil {
						res[idx] = "HDRFAIL " + err.Error()
						return
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
					if _, err := io.ReadFull(br, body); err != nil {
						res[idx] = "BODYFAIL " + err.Error()
						return
					}
				}
				if code == 200 && string(body) == "TLS-FAST" {
					res[idx] = "OK"
				} else {
					res[idx] = fmt.Sprintf("BADRESP code=%d body=%q", code, string(body))
				}
			}(i)
		}
		time.Sleep(200 * time.Millisecond)
		close(start)
		wg.Wait()
		for _, r := range res {
			if r == "OK" {
				ok++
			} else {
				fail++
				mu.Lock()
				for s := range errSamples {
					if errSamples[s] == "" {
						errSamples[s] = r
						break
					}
				}
				mu.Unlock()
			}
		}
	}
	fmt.Printf("rounds=%d conc=%d ok=%d fail=%d\n", rounds, conc, ok, fail)
	for _, s := range errSamples {
		if s != "" {
			fmt.Println("  errSample: " + s)
		}
	}
	if fail == 0 {
		fmt.Println("M101S1_CONC PASS=1 FAIL=0")
		os.Exit(0)
	}
	fmt.Println("M101S1_CONC PASS=0 FAIL=1")
	os.Exit(1)
}
