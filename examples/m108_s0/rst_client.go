// M108/D1 复现台辅助（qg-issue 66）：制造一个「连上 → 发 5 字节 TLS Record 头 → **RST** 关闭」
// 的连接。
//
// 为何需要 Go：要发出 RST（而非 FIN）必须 setsockopt(SO_LINGER, {1,0})，shell 的
//   /dev/tcp 无法表达。仓内已有同类先例（examples/m108_lock/go_bench.go）。
//
// 用途：验证 px_serve 握手循环的停止原因分桶 —— 正常 FIN 会走 mbedtls 错误分支
//   （hs_fail，不产 STALL 行），只有 RST 会让 poll(2) 报 POLLERR ⇒ reason=poll-error。
//   这正是 qg-issue 66 的 F10 猜想（96 条自然 timeout 中 62 条「停滞<1s」在语义上
//   不可能是 10s 总截止）的本地可判据。
//
// 用法：rst_client <host:port> [发送后等待毫秒，默认 300]
//   等待的意义：让服务端先 recv 掉那 5 字节并进入 poll()，否则 RST 可能早于第一次
//   recv 到达 → 走 mbedtls 错误分支（不产 STALL 行），使验证出现假阴性。
package main

import (
	"fmt"
	"net"
	"os"
	"strconv"
	"syscall"
	"time"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "用法: rst_client <host:port> [等待毫秒]")
		os.Exit(2)
	}
	addr := os.Args[1]
	wait := 300
	if len(os.Args) > 2 {
		if n, err := strconv.Atoi(os.Args[2]); err == nil {
			wait = n
		}
	}
	c, err := net.DialTimeout("tcp", addr, 3*time.Second)
	if err != nil {
		fmt.Fprintln(os.Stderr, "dial:", err)
		os.Exit(1)
	}
	tcp, ok := c.(*net.TCPConn)
	if !ok {
		fmt.Fprintln(os.Stderr, "非 TCP 连接")
		os.Exit(1)
	}
	if _, err := tcp.Write([]byte{0x16, 0x03, 0x01, 0x02, 0x00}); err != nil {
		fmt.Fprintln(os.Stderr, "write:", err)
		os.Exit(1)
	}
	time.Sleep(time.Duration(wait) * time.Millisecond)
	if f, err := tcp.File(); err == nil {
		_ = syscall.SetsockoptLinger(int(f.Fd()), syscall.SOL_SOCKET, syscall.SO_LINGER,
			&syscall.Linger{Onoff: 1, Linger: 0})
		_ = f.Close() // 只关 dup 出来的 fd（dup 共享同一 socket，linger 设置生效）
	}
	_ = tcp.Close() // 立即 RST
	fmt.Println("RST_SENT")
}
