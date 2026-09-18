// m146 真值程序 —— **Go 本尊**（math.Float64bits / Float64frombits / encoding/json）
//
// 用法：truth <corpus.txt>   （由 verify.sh 调用）
// 输出：每行 `<tag>|<输入>|<bits16hex>|<json文本或 !err>|<类别>`，与 PuXian 侧
//       fidelity.px 的输出逐字节对拍。
//
// 为什么要这个门：M143 补了 float32 的位模式对偶（float32_bits / bits_to_float32），
//   但 float64 的对应项一直缺失 ⇒ 语言里读不出 float64 的位模式，也**造不出**
//   NaN/±Inf/非规格化数（只能用十进制文本来回近似）。M146 补齐后，本门把
//   「全域 64 位模式」与 Go 逐位对齐：
//     · bits / 位模式往返（对**每一个**语料项都要求逐位回到原值，NaN 也含在内）；
//     · json 文本（Go encoding/json 的浮点文本：非有限值 → 报错，我们记 !err）；
//     · 类别（zero/-zero/sub/fin/inf/nan）—— 证明"位模式确实被解出了正确的数"。
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math"
	"os"
	"strconv"
	"strings"
)

func classOf(f float64) string {
	switch {
	case math.IsNaN(f):
		return "nan"
	case math.IsInf(f, 1):
		return "+inf"
	case math.IsInf(f, -1):
		return "-inf"
	case f == 0:
		if math.Signbit(f) {
			return "-zero"
		}
		return "zero"
	case math.Abs(f) < 2.2250738585072014e-308:
		return "sub"
	default:
		return "fin"
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "用法: truth <corpus.txt>")
		os.Exit(2)
	}
	fh, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, "打开语料失败:", err)
		os.Exit(2)
	}
	defer fh.Close()

	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()

	sc := bufio.NewScanner(fh)
	sc.Buffer(make([]byte, 1<<20), 1<<20)
	n := 0
	for sc.Scan() {
		line := strings.TrimRight(sc.Text(), "\r")
		if line == "" {
			continue
		}
		i := strings.Index(line, "|")
		if i < 0 {
			fmt.Fprintln(os.Stderr, "语料行格式错:", line)
			os.Exit(2)
		}
		tag, in := line[:i], line[i+1:]
		var f float64
		switch tag {
		case "v":
			f, err = strconv.ParseFloat(in, 64)
			if err != nil {
				fmt.Fprintf(out, "%s|%s|PARSEERR|!err|err\n", tag, in)
				n++
				continue
			}
		case "i":
			var iv int64
			iv, err = strconv.ParseInt(in, 10, 64)
			if err != nil {
				fmt.Fprintf(out, "%s|%s|PARSEERR|!err|err\n", tag, in)
				n++
				continue
			}
			f = float64(iv)
		case "b":
			u, e2 := strconv.ParseUint(in, 16, 64)
			if e2 != nil {
				fmt.Fprintf(out, "%s|%s|PARSEERR|!err|err\n", tag, in)
				n++
				continue
			}
			f = math.Float64frombits(u)
		default:
			fmt.Fprintln(os.Stderr, "未知语料标签:", tag)
			os.Exit(2)
		}
		bits := math.Float64bits(f)
		jb, jerr := json.Marshal(f)
		jt := "!err"
		if jerr == nil {
			jt = string(jb)
		}
		fmt.Fprintf(out, "%s|%s|%016x|%s|%s\n", tag, in, bits, jt, classOf(f))
		n++
	}
	if err := sc.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "读语料失败:", err)
		os.Exit(2)
	}
	fmt.Fprintf(os.Stderr, "真值行数: %d\n", n)
}
