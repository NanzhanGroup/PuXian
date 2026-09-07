// M88-S2/S3 并发压测工具：http_serve_unix（qg-issue 27 场景）
// 用法：
//   go build -o /tmp/press_unix press_unix.go
//   服务端：px build examples/m88_s3/s2_serve_unix.px 后后台运行（默认 PX_SERVE_WORKERS=256）
//   压测：  /tmp/press_unix <并发> <每并发请求数>     （keep-alive 复用；统计 200/失败/耗时）
//   环境：  PX_SERVE_WORKERS  池 worker 数（默认 256；须 ≥ 目标 keep-alive 并发连接数）
//           PX_MAX_THREADS    GC 槽上限（默认 1024；须 > PX_SERVE_WORKERS+主线程+业务 spawn）
//
// 实测基线（本机，M88-S2 后）：
//   - 并发 100 × 500（5 万请求）：全 200、0 连接失败、进程不崩（约 63s）
//   - 并发 30/60 × 500：全 200、0 失败
//   - 高并发（≥256 keep-alive）吞吐受 M11 容器全局锁（g_gc_mu 串行化容器操作）制约，
//     超其吞吐上限表现为请求超时而非崩溃——服务进程始终存活（issue 27 "exit(1)" 已根除）。
//     容器锁细化 / 空闲连接事件驱动属 B 类及后续里程碑（见 M88_PLAN §三）。
package main

import (
	"context"
	"fmt"
	"io"
	"net"
	"net/http"
	"os"
	"strconv"
	"sync"
	"sync/atomic"
	"time"
)

func main() {
	c, m := 100, 500
	if len(os.Args) >= 3 {
		c, _ = strconv.Atoi(os.Args[1])
		m, _ = strconv.Atoi(os.Args[2])
	}
	sock := "/tmp/m88_s3_press.sock"
	if len(os.Args) >= 4 {
		sock = os.Args[3]
	}
	dial := func(ctx context.Context, network, addr string) (net.Conn, error) {
		return net.Dial("unix", sock)
	}
	tr := &http.Transport{DialContext: dial, MaxIdleConnsPerHost: 2}
	client := &http.Client{Transport: tr, Timeout: 30 * time.Second}
	var ok, bad, errs int64
	var wg sync.WaitGroup
	start := time.Now()
	for w := 0; w < c; w++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for i := 0; i < m; i++ {
				resp, err := client.Get("http://localhost/ok")
				if err != nil {
					atomic.AddInt64(&errs, 1)
					continue
				}
				io.Copy(io.Discard, resp.Body)
				resp.Body.Close()
				if resp.StatusCode == 200 {
					atomic.AddInt64(&ok, 1)
				} else {
					atomic.AddInt64(&bad, 1)
				}
			}
		}()
	}
	wg.Wait()
	el := time.Since(start).Seconds()
	fmt.Printf("RESULT conc=%d per=%d ok=%d bad=%d err=%d elapsed=%.2fs\n", c, m, ok, bad, errs, el)
}
