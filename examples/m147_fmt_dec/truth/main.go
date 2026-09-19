// m147 真值程序 —— **Go 本尊**（strconv.FormatFloat(x,'f',dec,64) / fmt.Sprintf("%.*f")）
//
// 用法：truth <corpus.txt>      （由 verify.sh 调用）
// 输出：每行 `<输入行>|<dec 位定点文本>`，与 PuXian 侧 fidelity.px 的输出逐字节对拍。
//
// 为什么这个门必须存在（缺陷 127）：语言里此前没有 `%.Nf`，token-cache 侧用手搓
//   `floor(x*100+0.5)`（四舍五入-半向上）逼近 Go 的 `%.2f` —— 而 Go/glibc 对
//   **精确并列**取 round-half-even（0.125 → "0.12"，手搓给 "0.13"）。
//   本程序就是那条判据的**真值来源**：Go 的 `%.*f` 与 `strconv.FormatFloat('f')`
//   在本机同值（下面两种都打印，验证它们一致）。
package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"strconv"
	"strings"
)

func goText(f float64, dec int) string {
	// 与 PuXian native 的约定一致：非有限值给 Go 的 fmt 文本（不是 libc 的 nan/inf）
	if math.IsNaN(f) {
		return "NaN"
	}
	if math.IsInf(f, 1) {
		return "+Inf"
	}
	if math.IsInf(f, -1) {
		return "-Inf"
	}
	return strconv.FormatFloat(f, 'f', dec, 64)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "用法：truth <corpus.txt>")
		os.Exit(2)
	}
	f, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, "打开语料失败:", err)
		os.Exit(2)
	}
	defer f.Close()

	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()

	nFmtDiff := 0
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1<<20), 1<<20)
	n := 0
	for sc.Scan() {
		line := sc.Text()
		if strings.TrimSpace(line) == "" {
			continue
		}
		parts := strings.Split(line, "|")
		if len(parts) != 4 {
			fmt.Fprintf(out, "BAD|%s\n", line)
			continue
		}
		dec, _ := strconv.Atoi(parts[0])
		kind, txt, want := parts[1], parts[2], parts[3]
		var v float64
		if kind == "b" {
			u, err := strconv.ParseUint(txt, 16, 64)
			if err != nil {
				fmt.Fprintf(out, "BADBITS|%s\n", line)
				continue
			}
			v = math.Float64frombits(u)
		} else {
			v, err = strconv.ParseFloat(txt, 64)
			if err != nil {
				fmt.Fprintf(out, "BADVAL|%s\n", line)
				continue
			}
		}
		got := goText(v, dec)
		// 交叉校验：fmt.Sprintf("%.*f") 与 strconv.FormatFloat 必须同值
		if !math.IsNaN(v) && !math.IsInf(v, 0) {
			if s := fmt.Sprintf("%.*f", dec, v); s != strconv.FormatFloat(v, 'f', dec, 64) {
				nFmtDiff++
			}
		}
		if got != want {
			fmt.Fprintf(os.Stderr, "语料自带期望与 Go 不符：%s → go=%s corpus=%s\n", line, got, want)
		}
		// 与 PuXian 侧同格式：`<dec>|<kind>|<txt>|<值>`（语料自带的期望已在上方交叉校验）
		fmt.Fprintf(out, "%s|%s|%s|%s\n", parts[0], kind, txt, got)
		n++
	}
	fmt.Fprintf(os.Stderr, "真值行数 %d · fmt/strconv 不一致 %d\n", n, nFmtDiff)
}
