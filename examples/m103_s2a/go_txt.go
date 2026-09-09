// M103-S2a (Issue 29 GAP-DNS-TXT-1) 对拍参照：Go net.LookupTXT
//   dump <domain>  打印全部 TXT（每行一条）—— px dns_txt 的对拍基准
//   cmp <f1> <f2>  两文件按行集合相等比较（与顺序无关）
// 依赖：go（net.LookupTXT）；用法：go build -o build/go_txt go_txt.go
package main

import (
	"fmt"
	"net"
	"os"
	"sort"
	"strings"
)

func main() {
	if len(os.Args) >= 3 && os.Args[1] == "dump" {
		t, err := net.LookupTXT(os.Args[2])
		if err != nil {
			fmt.Fprintln(os.Stderr, "ERR", err)
			os.Exit(1)
		}
		for _, s := range t {
			fmt.Println(s)
		}
		return
	}
	if len(os.Args) == 4 && os.Args[1] == "cmp" {
		a := readLines(os.Args[2])
		b := readLines(os.Args[3])
		sort.Strings(a)
		sort.Strings(b)
		ja := strings.Join(a, "\n")
		jb := strings.Join(b, "\n")
		if ja == jb {
			fmt.Println("CMP-EQUAL")
			return
		}
		fmt.Println("CMP-DIFF")
		fmt.Fprintln(os.Stderr, "go:", ja)
		fmt.Fprintln(os.Stderr, "px:", jb)
		os.Exit(1)
	}
	fmt.Fprintln(os.Stderr, "usage: dump <domain> | cmp <f1> <f2>")
	os.Exit(2)
}

func readLines(p string) []string {
	d, err := os.ReadFile(p)
	if err != nil {
		panic(err)
	}
	s := strings.TrimSpace(string(d))
	if s == "" {
		return nil
	}
	return strings.Split(s, "\n")
}
