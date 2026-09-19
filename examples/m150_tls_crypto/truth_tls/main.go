// M150 TLS 真值程序（Go 本尊的 crypto/tls 客户端）—— 只用标准库
//
// 目的：把 PuXian 的 `tls_connect/tls_send/tls_recv/tls_close` 与 Go 的 crypto/tls
//   **放在同一个服务端上**跑同一套动作，输出可逐行比拟的行（字段取**实现无关**的那些：
//   版本号常量、收发字节、EOF 语义、失败可辨别 —— **不**比套件名，两侧偏好序本来不同）。
//
// 行协议与 examples/m150_tls_crypto/tls_probe.px 对齐（**行式读**，避免"一次 recv 收到
//   两条消息"造成的分片差异 —— 第一版就踩了这个，Go 一次 Read 把问候语和回显都收了，
//   于是 recv_n 一侧 10 一侧 12）：
//     `TLS|greet|<服务端问候（去掉换行）>`
//     `TLS|send_ok|` / `TLS|recv_data|` / `TLS|recv_n|`
//
// 用法：go run truth_tls/main.go <host:port>
// 退出码：0 = 全部符合预期
package main

import (
	"bufio"
	"crypto/tls"
	"fmt"
	"io"
	"net"
	"os"
	"strings"
	"time"
)

// 读到含 sep 为止（最多 maxloop 次）
func readUntil(r *bufio.Reader, sep string, maxloop int) string {
	acc := ""
	for i := 0; i < maxloop; i++ {
		b, err := r.ReadByte()
		if err != nil {
			return acc
		}
		acc += string(b)
		if strings.Contains(acc, sep) {
			return acc
		}
	}
	return acc
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "用法: truth_tls <host:port>")
		os.Exit(2)
	}
	addr := os.Args[1]
	d := &net.Dialer{Timeout: 3 * time.Second}

	// ① insecure（= libpq 的 sslmode=require / PuXian 的 verify=false）：加密但不验证
	conn, err := tls.DialWithDialer(d, "tcp", addr, &tls.Config{InsecureSkipVerify: true})
	if err != nil {
		fmt.Printf("TLS|insecure_ok|false\nTLS|insecure_err|%v\n", err)
		os.Exit(1)
	}
	st := conn.ConnectionState()
	br := bufio.NewReader(conn)
	fmt.Printf("TLS|insecure_ok|true\n")
	fmt.Printf("TLS|insecure_version_num|%d\n", st.Version)
	fmt.Printf("TLS|insecure_peer_port|%d\n", conn.RemoteAddr().(*net.TCPAddr).Port)

	greet := readUntil(br, "\n", 20)
	fmt.Printf("TLS|greet|%s\n", strings.TrimSpace(greet))

	if _, err := conn.Write([]byte("hello")); err != nil {
		fmt.Printf("TLS|send_ok|false\n")
		os.Exit(1)
	}
	fmt.Printf("TLS|send_ok|true\n")
	echo := strings.TrimSpace(readUntil(br, "ECHO:hello", 20))
	fmt.Printf("TLS|recv_ok|%v\n", strings.Contains(echo, "ECHO:hello"))
	fmt.Printf("TLS|recv_data|%s\n", echo)
	fmt.Printf("TLS|recv_n|%d\n", len(echo))

	// ② bye ⇒ 服务端关连接 ⇒ 必须读到 EOF
	conn.Write([]byte("bye"))
	b2 := strings.TrimSpace(readUntil(br, "ECHO:bye", 20))
	fmt.Printf("TLS|after_bye_n|%d\n", len(b2))
	eof := false
	for i := 0; i < 10; i++ {
		_, err := br.ReadByte()
		if err != nil { // io.EOF 或其它 ⇒ 视为已关
			eof = true
			break
		}
	}
	fmt.Printf("TLS|eof|%v\n", eof)
	conn.Close()
	fmt.Printf("TLS|close_ok|true\n")

	// ③ verify（默认校验）：自签证书必须失败
	conn2, err2 := tls.DialWithDialer(d, "tcp", addr, &tls.Config{ServerName: "localhost"})
	if err2 == nil {
		io.Copy(io.Discard, conn2)
		conn2.Close()
		fmt.Printf("TLS|verify_ok|true\n")
	} else {
		fmt.Printf("TLS|verify_ok|false\n")
	}
	os.Exit(0)
}
