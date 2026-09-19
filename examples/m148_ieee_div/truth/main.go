// m148 真值程序 —— **Go 本尊**（IEEE 754 浮点除法 / math.Mod / fmt.Sprint）
//
// 用法：truth <div|str|gv> <corpus.txt>   （由 verify.sh 调用）
//
// 输出（与 PuXian 侧 fidelity.px **逐字节**对拍）：
//   div : `<op>|<ahex>|<bhex>|<结果16位hex 或 nan>`
//   str : `<hex16>|<fmt.Sprint(值)>`
//   gv  : `<hex16>|<bits>|<fmt.Sprint(值)>`（bits=32 时 Go 侧先窄化到 float32）
//
// 为什么这个门必须存在（缺陷 118）：Go 的浮点换算是良定义的 —— x/0.0 = ±Inf、
// 0.0/0.0 = NaN、math.Mod(x,0) = NaN；而 PuXian 的 `/` 修前对零除数一律
// px_error 杀进程 ⇒ 「÷0 → Inf/NaN」这条路**表达不出来**。本门把「浮点除法与
// 取模的全域位模式」与 Go 逐位钉死（NaN 只归一为 `nan` 标记，见 README 边界：
// NaN 的**符号位与载荷**在两边不是同一约定，不可逐位比）。
//
//   另外 gv 一档把 Go 的 **`%v` 文本**（strconv 'g' 最短往返）搬进来 —— 这是
//   「Go 打印浮点」的面，与 PuXian `str()` 的语言约定**本来就不同**（缺陷 132）。
package main

import (
	"bufio"
	"encoding/hex"
	"fmt"
	"math"
	"os"
	"strings"
)

func fromHex(s string) float64 {
	b, err := hex.DecodeString(s)
	if err != nil || len(b) != 8 {
		return math.NaN()
	}
	var u uint64
	for _, c := range b {
		u = u<<8 | uint64(c)
	}
	return math.Float64frombits(u)
}

func toHex(f float64) string {
	return fmt.Sprintf("%016x", math.Float64bits(f))
}

func main() {
	if len(os.Args) < 3 {
		fmt.Fprintln(os.Stderr, "用法: truth <div|str|gv> <corpus.txt>")
		os.Exit(2)
	}
	mode, path := os.Args[1], os.Args[2]
	fh, err := os.Open(path)
	if err != nil {
		fmt.Fprintln(os.Stderr, "读取失败:", err)
		os.Exit(1)
	}
	defer fh.Close()
	sc := bufio.NewScanner(fh)
	sc.Buffer(make([]byte, 1<<20), 1<<20)
	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()

	for sc.Scan() {
		line := strings.TrimRight(sc.Text(), "\r")
		if line == "" {
			continue
		}
		switch mode {
		case "div":
			p := strings.Split(line, "|")
			if len(p) != 3 {
				continue
			}
			a, b := fromHex(p[1]), fromHex(p[2])
			var r float64
			if p[0] == "d" {
				r = a / b
			} else {
				r = math.Mod(a, b)
			}
			rs := toHex(r)
			if math.IsNaN(r) {
				rs = "nan"
			}
			fmt.Fprintf(out, "%s|%s|%s|%s\n", p[0], p[1], p[2], rs)
		case "str":
			f := fromHex(line)
			fmt.Fprintf(out, "%s|%s\n", line, fmt.Sprint(f))
		case "gv":
			p := strings.Split(line, "|")
			if len(p) != 2 {
				continue
			}
			f := fromHex(p[0])
			if p[1] == "32" {
				fmt.Fprintf(out, "%s|32|%s\n", p[0], fmt.Sprint(float32(f)))
			} else {
				fmt.Fprintf(out, "%s|64|%s\n", p[0], fmt.Sprint(f))
			}
		default:
			fmt.Fprintln(os.Stderr, "未知模式:", mode)
			os.Exit(2)
		}
	}
}
