// m142 truth — Go 侧真值（encoding/json 的 Indent / Compact / Marshal 转义路径 / HTMLEscape）
//
// 为什么要有它：Go 的 Indent/Compact 不是"解析后重新编码"，而是**在原文上做插入/删除**
// （见 $GOROOT/src/encoding/json/indent.go）。要做到逐字节一致，唯一权威就是 Go 本尊——
// 本程序把它对同一份语料（corpus.txt，hex 编码，见 gen_corpus.py）的输出逐行打印，
// 供 verify.sh 与 PuXian 侧 `go_json_indent.px` 逐字节 diff。
//
// 输出格式（每例 5 行）：
//   CASE <i>
//   I|  <ok>|<out_hex>|<err_msg>|<offset>      json.Indent(dst, src, "", "  ")
//   C0| <ok>|<out_hex>|<err_msg>|<offset>      json.Compact(dst, src)          (转义关)
//   C1| <ok>|<out_hex>|<err_msg>|<offset>      json.Marshal(json.RawMessage)  (转义开)
//      ↑ 仅对 **合法** 输入输出（非法输入打 C1|SKIP：Marshal 会把错误包成
//        MarshalerError，包名与实现语言绑定，不属可比面）
//   E|  <out_hex>                              json.HTMLEscape（纯字节替换，永不报错）
package main

import (
	"bufio"
	"bytes"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func loadCorpus(path string) ([][]byte, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var out [][]byte
	for _, line := range strings.Split(string(data), "\n") {
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		if !strings.HasPrefix(line, "x") {
			return nil, fmt.Errorf("坏语料行: %q", line)
		}
		b, err := hex.DecodeString(line[1:])
		if err != nil {
			return nil, fmt.Errorf("坏 hex: %q", line)
		}
		out = append(out, b)
	}
	return out, nil
}

func emit(w *bufio.Writer, label string, out []byte, err error) {
	if err != nil {
		off := 0
		var se *json.SyntaxError
		if errors.As(err, &se) {
			off = int(se.Offset)
		}
		fmt.Fprintf(w, "%s|0||%s|%d\n", label, err.Error(), off)
		return
	}
	fmt.Fprintf(w, "%s|1|%s||0\n", label, hex.EncodeToString(out))
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "用法: truth <corpus.txt>")
		os.Exit(2)
	}
	corpus, err := loadCorpus(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, "语料读取失败:", err)
		os.Exit(2)
	}
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()

	for i, src := range corpus {
		fmt.Fprintf(w, "CASE %d\n", i)

		// Indent 的输出长度是 O(depth²)：超深嵌套（如 10001 层）会产生 ~100MB 中间产物。
		// 两侧同一条规则（阈值 8192 字节）：超过则 Indent 面记 SKIP，只跑
		// Compact（O(n)）与 HTMLEscape —— 最大嵌套深度是 **scanner** 的行为，
		// 由 Compact 面覆盖（同一份 scanner.go）。
		if len(src) > 8192 {
			fmt.Fprint(w, "I|SKIP\n")
		} else {
			var b bytes.Buffer
			err := json.Indent(&b, src, "", "  ")
			emit(w, "I", b.Bytes(), err)
		}

		var b bytes.Buffer
		err := json.Compact(&b, src)
		emit(w, "C0", b.Bytes(), err)

		if json.Valid(src) {
			mb, merr := json.Marshal(json.RawMessage(src))
			emit(w, "C1", mb, merr)
		} else {
			fmt.Fprint(w, "C1|SKIP\n")
		}

		var e bytes.Buffer
		json.HTMLEscape(&e, src)
		fmt.Fprintf(w, "E|%s\n", hex.EncodeToString(e.Bytes()))
	}

	// Q 面：`quoteChar(byte)` 的**全部 256 个取值**。
	//   `quoteChar` 在 Go 里是**未导出**的（scanner.go），此处用公开 API 1:1 复刻：
	//       s := strconv.Quote(string(c)); return "'" + s[1:len(s)-1] + "'"
	//   （' 与 " 是特例；string(byte) 把字节按 Latin-1 解释成 U+0000..U+00FF）。
	//   为什么要单独一面：错误文案里的字符形态**无法**用「喂非法 UTF-8 字节串」来测
	//   （PuXian 的 str 索引是字符级）—— 直接把 256 个 byte 的映射逐个对拍最彻底。
	for b := 0; b < 256; b++ {
		fmt.Fprintf(w, "Q|%02x|%s\n", b, quoteChar(byte(b)))
	}
}

// quoteChar 与 $GOROOT/src/encoding/json/scanner.go 的同名函数逐字一致（用公开 API 复刻）
func quoteChar(c byte) string {
	if c == '\'' {
		return `'\''`
	}
	if c == '"' {
		return `'"'`
	}
	s := strconv.Quote(string(c))
	return "'" + s[1:len(s)-1] + "'"
}
