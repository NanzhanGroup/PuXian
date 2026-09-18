// M143 真值生成器 —— Go 本尊（math.Float32bits / Float32frombits / encoding/json）。
//
// 输出格式（每行一条，纯 ASCII，便于与 PuXian 侧逐字节 diff）：
//   v|<idx>|<json64>|<json32>|<bits32>
//   b|<idx>|<json64(widened)>|<json32>|<bits32(widened)>
//   m|<idx>|<mul_bits>|<add_bits>|<div_bits>
//   d|<idx>|<d_bits>|<f32_div_count>
// 说明：
//   · json64 = json.Marshal(float64) 的文本；json32 = json.Marshal(float32) 的文本；
//     NaN/±Inf 时 Go 的 Marshal 返回 UnsupportedValueError ⇒ 真值写 null（与移植侧
//     json_num_str 的 null 口径一致）。
//   · bits32(widened) 用 `Float32bits(float32(float64(f32)))` —— 即**经由 float64 往返**
//     的位模式，这正是 PuXian 侧必须走的路径（语言只有 float64）。NaN 载荷若在加宽/窄化
//     中丢信息，真值会如实反映（identity 与 widened 的差异由 stderr 报告，见下）。
//   · m/d 面：位模式由 Go 的 float32 运算产出，用来验证「每步 f32 收口 = 直接 float32
//     运算」这条纪律（二次舍入不存在）。
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

func f64(s string) float64 {
	v, err := strconv.ParseFloat(s, 64)
	if err != nil {
		fmt.Fprintf(os.Stderr, "解析失败 %q: %v\n", s, err)
		os.Exit(2)
	}
	return v
}

func j64(v float64) string {
	b, err := json.Marshal(v)
	if err != nil {
		return "null"
	}
	return string(b)
}

func j32(v float32) string {
	b, err := json.Marshal(v)
	if err != nil {
		return "null"
	}
	return string(b)
}

func main() {
	in, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(2)
	}
	defer in.Close()
	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()
	sc := bufio.NewScanner(in)
	sc.Buffer(make([]byte, 1<<20), 1<<20)
	idx := 0
	nanPayloadDiff := 0
	for sc.Scan() {
		line := strings.TrimSpace(sc.Text())
		if line == "" {
			continue
		}
		f := strings.Fields(line)
		switch f[0] {
		case "v":
			x := f64(f[1])
			fmt.Fprintf(out, "v|%d|%s|%s|%08x\n", idx, j64(x), j32(float32(x)), math.Float32bits(float32(x)))
		case "b":
			u, err := strconv.ParseUint(f[1], 16, 32)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				os.Exit(2)
			}
			f32 := math.Float32frombits(uint32(u))
			widened := float32(float64(f32)) // 往返：float32 → float64 → float32
			if math.Float32bits(widened) != math.Float32bits(f32) {
				nanPayloadDiff++
			}
			fmt.Fprintf(out, "b|%d|%s|%s|%08x\n", idx, j64(float64(f32)), j32(widened), math.Float32bits(widened))
		case "m":
			a := float32(f64(f[1]))
			b := float32(f64(f[2]))
			fmt.Fprintf(out, "m|%d|%08x|%08x|%08x\n", idx,
				math.Float32bits(a*b), math.Float32bits(a+b), math.Float32bits(a/b))
		case "d":
			a := float32(f64(f[1]))
			b := float32(f64(f[2]))
			n, _ := strconv.Atoi(f[3])
			acc := a
			for i := 0; i < n; i++ {
				acc += b
			}
			fmt.Fprintf(out, "d|%d|%08x|%08x\n", idx, math.Float32bits(acc), math.Float32bits(a/b))
		default:
			fmt.Fprintf(os.Stderr, "未知语料行 %q\n", line)
			os.Exit(2)
		}
		idx++
	}
	fmt.Fprintf(os.Stderr, "cases=%d  float32→float64→float32 位模式变化的用例=%d（NaN 载荷面，见 README）\n", idx, nanPayloadDiff)
}
