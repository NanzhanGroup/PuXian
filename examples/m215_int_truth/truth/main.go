// M215 门 · 真值参考之二：**Go 量级域实现**（与 ref.py 结构上彼此独立）
// ============================================================
// 为什么需要它（本门存在的唯一理由）：
//   缺陷 305/306 的病灶在**运行时原语内部**——`px_idiv`（runtime.c）与 `pxc_vm` 的
//   `PXOP_IDIV` 快路径**各写了一遍同样的公式**。所以**三轨跑的是同一份语义**，
//   输出逐字节一致。任何「三轨对拍门」按定义看不见它。
//   ⇒ 判据必须 = 三轨彼此一致 **且** 与**独立真值**逐字节一致。
//
// 与 ref.py 的独立点：**全程不做有符号加减**，只在 uint64 量级域上推导：
//     ad = |d|（INT64_MIN 用 -(v+1)+1 取量级，避免取负溢出）
//     qt = |n| / ad ;  rm = |n| % ad
//     r  = rm ;  若 n<0 且 rm != 0  ⇒  r = ad - rm      （保证 0 <= r < ad）
//     q  由 4 个符号分支给出（|q| 用 uint64 承载，最后判可表示性）
//   唯一不可表示：n == INT64_MIN 且 d == -1（真商 = +2^63）⇒ 按文档口径回绕。
//
// 输出 = corpus.px 的**完整期望 stdout**（含段标记）—— 门直接 `cmp` 即可。
// 用法：./build/truth domain.txt    （或 go run truth/main.go domain.txt）
package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

const (
	i64Min  = int64(-9223372036854775808)
	i64Max  = int64(9223372036854775807)
	cMaxPos = uint64(9223372036854775807) // 2^63-1
	cMaxNeg = uint64(9223372036854775808) // 2^63（-2^63 的量级）
)

func absu(v int64) uint64 {
	if v < 0 {
		return uint64(-(v + 1)) + 1
	}
	return uint64(v)
}

// euclid：量级域推导；repOK=false ⇒ 真商不可表示（唯一一种：MIN / -1）
func euclid(n, d int64) (q int64, r uint64, repOK bool) {
	an, ad := absu(n), absu(d)
	qt, rm := an/ad, an%ad
	r = rm
	if n < 0 && rm != 0 {
		r = ad - rm
	}
	var qu uint64
	qpos := true
	if d > 0 {
		if n >= 0 {
			qu, qpos = qt, true
		} else {
			qpos = false
			if rm == 0 {
				qu = qt
			} else {
				qu = qt + 1
			}
		}
	} else {
		if n >= 0 {
			qu, qpos = qt, false
		} else {
			qpos = true
			if rm == 0 {
				qu = qt
			} else {
				qu = qt + 1
			}
		}
	}
	if qpos {
		if qu > cMaxPos {
			return 0, r, false
		}
		return int64(qu), r, true
	}
	if qu > cMaxNeg {
		return 0, r, false
	}
	if qu == cMaxNeg {
		return i64Min, r, true
	}
	return -int64(qu), r, true
}

// euclidDoc：按**文档口径**取答案（不可表示的那一对回绕）
func euclidDoc(n, d int64) (int64, int64) {
	q, r, ok := euclid(n, d)
	if !ok {
		return i64Min, 0
	}
	return q, int64(r)
}

func bt(x bool) string {
	if x {
		return "true"
	}
	return "false"
}

func main() {
	path := "domain.txt"
	if len(os.Args) > 1 {
		path = os.Args[1]
	}
	f, err := os.Open(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "打不开 %s: %v\n", path, err)
		os.Exit(1)
	}
	defer f.Close()
	var ns, ds []int64
	sc := bufio.NewScanner(f)
	for sc.Scan() {
		line := strings.TrimSpace(sc.Text())
		if line == "" {
			continue
		}
		parts := strings.Fields(line)
		if len(parts) != 2 {
			fmt.Fprintf(os.Stderr, "行格式错误: %q\n", line)
			os.Exit(1)
		}
		n, e1 := strconv.ParseInt(parts[0], 10, 64)
		d, e2 := strconv.ParseInt(parts[1], 10, 64)
		if e1 != nil || e2 != nil || d == 0 {
			fmt.Fprintf(os.Stderr, "值非法: %q (%v %v)\n", line, e1, e2)
			os.Exit(1)
		}
		ns = append(ns, n)
		ds = append(ds, d)
	}
	if err := sc.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "读 %s 失败: %v\n", path, err)
		os.Exit(1)
	}

	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()

	// ---------- 段 A：`//` 与 `%` ----------
	fmt.Fprintln(w, "== A-BEGIN ==")
	for i := range ns {
		n, d := ns[i], ds[i]
		q, r := euclidDoc(n, d)
		fmt.Fprintf(w, "n=%d d=%d q=%d r=%d\n", n, d, q, r)
	}
	fmt.Fprintln(w, "== A-END ==")

	// ---------- 段 B：陷阱回归 ----------
	fmt.Fprintln(w, "== B-BEGIN ==")
	q1, r1 := euclidDoc(i64Min, -1)
	q3, r3 := euclidDoc(-7, i64Min)
	q5, _ := euclidDoc(i64Max, -1)
	fmt.Fprintf(w, "trap1=%d\n", q1)
	fmt.Fprintf(w, "trap2=%d\n", r1)
	fmt.Fprintf(w, "trap3=%d\n", q3)
	fmt.Fprintf(w, "trap4=%d\n", r3)
	fmt.Fprintf(w, "trap5=%d\n", q5)
	fmt.Fprintln(w, "== B-END ==")

	// ---------- 段 D：比较真值（整数比较精确，与 double 无关） ----------
	fmt.Fprintln(w, "== D-BEGIN ==")
	for i := range ns {
		n, d := ns[i], ds[i]
		fmt.Fprintf(w, "n=%d d=%d lt=%s le=%s gt=%s ge=%s eq=%s ne=%s\n",
			n, d, bt(n < d), bt(n <= d), bt(n > d), bt(n >= d), bt(n == d), bt(n != d))
	}
	fmt.Fprintln(w, "== D-END ==")

	// ---------- 段 C：不变量自断言 ----------
	bad := 0
	for i := range ns {
		_, r := euclidDoc(ns[i], ds[i])
		if !(r >= 0 && uint64(r) < absu(ds[i])) {
			bad++
		}
	}
	fmt.Fprintln(w, "== C-BEGIN ==")
	fmt.Fprintln(w, "== C-END ==")
	fmt.Fprintf(w, "M215-INV-BAD=%d CNT=%d\n", bad, len(ns))
	fmt.Fprintln(w, "M215-DONE")

	fmt.Fprintf(os.Stderr, "main.go 对=%d 不变量违规=%d\n", len(ns), bad)
}
