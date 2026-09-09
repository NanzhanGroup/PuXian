// M97-S3（qg-issue 32）验证客户端：Go http.Transport keep-alive 复用同一 unix 连接
// ------------------------------------------------------------
// 场景还原：清歌 token-cache 网关（Go http.Transport）对 http_serve_unix 的调用形态。
// 断言（输出 M97S3_CLIENT_OK_<n> / M97S3_CLIENT_FAIL_<n> + 连接数）：
//   1) 100 请求（/big 与 /ok 交替）全部成功、响应完整（大响应 > 发送缓冲触发
//      EAGAIN/部分写路径——修复前截断/悬挂）
//   2) 连接数 == 1（http.Transport keep-alive 复用同一连接 → 专测服务端长连生命周期）
//   3) 无悬挂（每请求 10s 超时；服务端悬挂 → 请求超时失败）
// 构建：CGO_ENABLED=0 go build（静态）。用法：go_ka_client <sock_path> <requests>
package main

import (
	"context"
	"fmt"
	"io"
	"net"
	"net/http"
	"os"
	"strconv"
	"sync/atomic"
	"time"
)

func main() {
	sock := "/tmp/m97s3.sock"
	nreq := 100
	if len(os.Args) > 1 {
		sock = os.Args[1]
	}
	if len(os.Args) > 2 {
		if v, err := strconv.Atoi(os.Args[2]); err == nil && v > 0 {
			nreq = v
		}
	}
	var conns int64
	dialer := &net.Dialer{}
	tr := &http.Transport{
		DialContext: func(ctx context.Context, network, addr string) (net.Conn, error) {
			atomic.AddInt64(&conns, 1)
			return dialer.DialContext(ctx, "unix", sock)
		},
		MaxIdleConns:        1,
		MaxIdleConnsPerHost: 1,
		IdleConnTimeout:     60 * time.Second,
		DisableKeepAlives:   false,
	}
	client := &http.Client{Transport: tr, Timeout: 10 * time.Second}
	ok, fail := 0, 0
	for i := 0; i < nreq; i++ {
		path := "/ok"
		if i%2 == 0 {
			path = "/big"
		}
		resp, err := client.Get("http://m97s3host" + path)
		if err != nil {
			fmt.Printf("req#%d ERR: %v\n", i, err)
			fail++
			continue
		}
		body, err := io.ReadAll(resp.Body)
		resp.Body.Close()
		if err != nil {
			fmt.Printf("req#%d READERR: %v\n", i, err)
			fail++
			continue
		}
		good := false
		if path == "/ok" {
			good = string(body) == "ok-m97s3"
		} else {
			// /big：前缀 + 后缀 + 长度完整（截断必显著变短或读超时）
			good = len(body) > 1500000 && string(body[:9]) == "M97S3BIG:" &&
				string(body[len(body)-5:]) == ":TAIL"
		}
		if !good {
			fmt.Printf("req#%d BADBODY path=%s len=%d head=%.24q tail=%.8q\n",
				i, path, len(body), body[:min(len(body), 24)], tail(body))
			fail++
			continue
		}
		ok++
	}
	fmt.Printf("conns=%d ok=%d fail=%d\n", atomic.LoadInt64(&conns), ok, fail)
	if fail == 0 && atomic.LoadInt64(&conns) == 1 {
		fmt.Printf("M97S3_CLIENT_OK_%d\n", ok)
		os.Exit(0)
	}
	fmt.Printf("M97S3_CLIENT_FAIL_%d\n", fail)
	os.Exit(1)
}

func tail(b []byte) string {
	if len(b) < 8 {
		return string(b)
	}
	return string(b[len(b)-8:])
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
