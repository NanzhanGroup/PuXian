package main

// M151 门（Go 参考侧）：与 px_probe.px 做同一套动作，输出可比子集。
//   go run go_probe.go <echo_port> <pgssl_port>
// 只用于门内比对（不参与 CI 的 build matrix）。

import (
	"crypto/tls"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"
	"net"
	"os"
	"strconv"
	"time"
)

var payload = []byte{0x41, 0x00, 0x42, 0xFF, 0xC3, 0x28, 0x5A}

func readN(c net.Conn, n int) []byte {
	acc := make([]byte, 0, n)
	buf := make([]byte, 4096)
	for len(acc) < n {
		c.SetReadDeadline(time.Now().Add(3 * time.Second))
		m, err := c.Read(buf)
		if m > 0 {
			acc = append(acc, buf[:m]...)
		}
		if err != nil {
			break
		}
	}
	return acc
}

func main() {
	echoPort, _ := strconv.Atoi(os.Args[1])
	sslPort, _ := strconv.Atoi(os.Args[2])
	phex := hex.EncodeToString(payload)

	// A/B：明文回显
	for _, tag := range []string{"A", "B"} {
		c, err := net.DialTimeout("tcp", fmt.Sprintf("127.0.0.1:%d", echoPort), 3*time.Second)
		if err != nil {
			fmt.Printf("G|%s|conn|fail\n", tag)
			continue
		}
		n, _ := c.Write(payload)
		got := readN(c, len(payload))
		fmt.Printf("G|%s|sent=%s|n=%d|recv=%s|same=%v\n", tag, phex, n,
			hex.EncodeToString(got), hex.EncodeToString(got) == phex)
		c.Write([]byte("bye"))
		c.Close()
	}

	// C：PG 式协商 + 升级
	c, err := net.DialTimeout("tcp", fmt.Sprintf("127.0.0.1:%d", sslPort), 3*time.Second)
	if err != nil {
		fmt.Printf("G|C|conn|fail\n")
		return
	}
	neg := make([]byte, 8)
	binary.BigEndian.PutUint32(neg[0:4], 8)
	binary.BigEndian.PutUint32(neg[4:8], 80877103)
	c.Write(neg)
	one := make([]byte, 1)
	io.ReadFull(c, one)
	tc := tls.Client(c, &tls.Config{InsecureSkipVerify: true})
	if err := tc.Handshake(); err != nil {
		fmt.Printf("G|C|neg=%s|first=%s|up=false|ver_num=0|verify=false\n", hex.EncodeToString(neg), string(one))
		return
	}
	fmt.Printf("G|C|neg=%s|first=%s|up=true|ver_num=%d|verify=false\n",
		hex.EncodeToString(neg), string(one), tc.ConnectionState().Version)
	greet := readN(tc, 6)
	hexOut := []byte("ECHO:")
	hexOut = append(hexOut, payload...)
	tc.Write(payload)
	back := readN(tc, len(hexOut))
	fmt.Printf("G|C|greet=%s|echo=%s\n", hex.EncodeToString(greet), hex.EncodeToString(back))
	tc.Write([]byte("bye"))
	tc.Close()
}
