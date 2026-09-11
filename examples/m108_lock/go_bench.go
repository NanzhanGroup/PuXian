// m108_bench —— M108 ①「握手锁争用量化」压测客户端
//
// 目的：以受控并发反复「新建连接 + 完整 TLS 握手 + 单请求 + 立即关闭」，
//   配合服务端 /proc/<pid>/stat 的 CPU 秒，算出服务端的
//   **有效并发度 E = 服务端 CPU 秒 / 墙钟秒**：
//     E ≈ 1.0 → 服务端在并发下仍被单点串行（候选：g_srv_hs_mu 全局握手锁）
//     E → N   → 在分配到的 N 核上真并行（受 MAXC 与核数上限约束）
//   客户端自身 CPU 一并输出，用于证明「客户端不是瓶颈」。
//
// 会话恢复已关闭（SessionTicketsDisabled），保证每次都是**完整握手**。
//
// 用法：m108_bench -addr 127.0.0.1:18202 -n 2400 -c 16 [-plain] [-sni localhost]
// 输出：M108LOCK n=.. conc=.. ok=.. bad=.. elapsed=..s rate=../s client_cpu=..s
// 退出码：0 = 全部成功；1 = 有失败（该档 rate/E 不可信）
package main

import (
	"bufio"
	"crypto/tls"
	"flag"
	"fmt"
	"io"
	"net"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"syscall"
	"time"
)

// cpuSec 返回本进程累计 CPU 秒（utime+stime）。
func cpuSec() float64 {
	var ru syscall.Rusage
	if err := syscall.Getrusage(syscall.RUSAGE_SELF, &ru); err != nil {
		return 0
	}
	return float64(ru.Utime.Sec+ru.Stime.Sec) + float64(ru.Utime.Usec+ru.Stime.Usec)/1e6
}

// one 完成一次「（可选）握手 + GET /fast + 读完 body + 关闭」。
func one(addr, sni string, plain bool) error {
	d := &net.Dialer{Timeout: 5 * time.Second}
	var c net.Conn
	var err error
	if plain {
		c, err = d.Dial("tcp", addr)
	} else {
		cfg := &tls.Config{
			InsecureSkipVerify:    true,
			ServerName:            sni,
			SessionTicketsDisabled: true, // 禁会话恢复：每连接都是完整握手
		}
		c, err = tls.DialWithDialer(d, "tcp", addr, cfg)
	}
	if err != nil {
		return err
	}
	defer c.Close()
	c.SetDeadline(time.Now().Add(20 * time.Second))
	if _, err = c.Write([]byte("GET /fast HTTP/1.1\r\nHost: m108\r\nConnection: close\r\n\r\n")); err != nil {
		return err
	}
	br := bufio.NewReader(c)
	st, err := br.ReadString('\n')
	if err != nil {
		return err
	}
	if !strings.Contains(st, "200") {
		return fmt.Errorf("status=%q", strings.TrimSpace(st))
	}
	var cl int
	for {
		ln, err := br.ReadString('\n')
		if err != nil {
			return err
		}
		ln = strings.TrimRight(ln, "\r\n")
		if ln == "" {
			break
		}
		if len(ln) > 15 && strings.EqualFold(ln[:15], "Content-Length:") {
			fmt.Sscanf(ln[15:], "%d", &cl)
		}
	}
	if cl > 0 {
		b := make([]byte, cl)
		if _, err := io.ReadFull(br, b); err != nil {
			return err
		}
	}
	return nil
}

func main() {
	addr := flag.String("addr", "127.0.0.1:18202", "服务端地址")
	n := flag.Int("n", 2400, "总连接数")
	conc := flag.Int("c", 16, "并发数")
	plain := flag.Bool("plain", false, "明文模式（不握手，作对照基线）")
	sni := flag.String("sni", "localhost", "SNI")
	flag.Parse()

	var ok, bad int64
	var mu sync.Mutex
	var sample string
	sem := make(chan struct{}, *conc)
	var wg sync.WaitGroup

	c0 := cpuSec()
	t0 := time.Now()
	for i := 0; i < *n; i++ {
		sem <- struct{}{}
		wg.Add(1)
		go func() {
			defer wg.Done()
			defer func() { <-sem }()
			if err := one(*addr, *sni, *plain); err != nil {
				atomic.AddInt64(&bad, 1)
				mu.Lock()
				if sample == "" {
					sample = err.Error()
				}
				mu.Unlock()
				return
			}
			atomic.AddInt64(&ok, 1)
		}()
	}
	wg.Wait()
	el := time.Since(t0).Seconds()
	cc := cpuSec() - c0

	fmt.Printf("M108LOCK n=%d conc=%d ok=%d bad=%d elapsed=%.3f rate=%.1f/s client_cpu=%.3fs\n",
		*n, *conc, ok, bad, el, float64(ok)/el, cc)
	if sample != "" {
		fmt.Println("  sample_err=" + sample)
	}
	if bad > 0 {
		os.Exit(1)
	}
}
