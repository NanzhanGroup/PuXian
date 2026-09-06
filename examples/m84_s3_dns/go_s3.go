// M84-S3 (Issue 22 GAP-DNS-1) Go oracle：net.LookupIP 权威对拍基准
// 用法: go_s3 gen <dir>  → 对 localhost / www.qq.com 解析，写
//   <dir>/go_v4_<tag>.txt（IPv4 每行一个）、<dir>/go_v6_<tag>.txt（IPv6 每行一个）
// 说明：DNS 轮询下两次解析集合可能不完全一致，verify 用「交集 ≥1」断言（非精确相等），
//       与本机系统解析器（getent 同源）双 oracle 印证 px dns_lookup 语义一致。
package main

import (
	"fmt"
	"net"
	"os"
	"path/filepath"
)

func dump(domain, dir, tag string) {
	addrs, err := net.LookupIP(domain)
	if err != nil {
		fmt.Fprintf(os.Stderr, "LookupIP %s: %v\n", domain, err)
		return
	}
	fv4, err4 := os.Create(filepath.Join(dir, "go_v4_"+tag+".txt"))
	fv6, err6 := os.Create(filepath.Join(dir, "go_v6_"+tag+".txt"))
	if err4 != nil || err6 != nil {
		fmt.Fprintln(os.Stderr, "create:", err4, err6)
		return
	}
	defer fv4.Close()
	defer fv6.Close()
	seen := map[string]bool{}
	for _, a := range addrs {
		ip := a.String()
		if seen[ip] {
			continue
		}
		seen[ip] = true
		if a.To4() != nil {
			fmt.Fprintln(fv4, ip)
		} else {
			fmt.Fprintln(fv6, ip)
		}
	}
}

func main() {
	if len(os.Args) < 3 {
		fmt.Fprintln(os.Stderr, "usage: go_s3 gen <dir>")
		os.Exit(2)
	}
	dir := os.Args[2]
	if err := os.MkdirAll(dir, 0o755); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	dump("localhost", dir, "localhost")
	dump("www.qq.com", dir, "qq")
}
