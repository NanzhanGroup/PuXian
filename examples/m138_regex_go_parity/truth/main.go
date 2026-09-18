// m138 truth — Go 侧真值（第 19 轮：正则的「Go 保真」门）
//
// 为什么要有它：`\b` / `\B` / `\A` / `\z` / `\Q…\E` / `\x…` / 八进制转义、以及
//   「字符类内转义」的合法性，**唯一权威是 Go 的 regexp**（PuXian 的 regex_* 被用来
//   顶替 Go 的 regexp：token-cache 的本地快判 / YAML 行解析等）。本程序把 Go 的行为
//   逐行打印，供 verify.sh 与 PuXian 侧逐字节对拍。
//
// 覆盖：
//   ① 穷举 0x20..0x7e 的 `\c`（类外）与 `[\c]`（类内）——**合法性**两两对照
//   ② 词边界/锚点语义 × 15 组文本（find / find_all / search+groups / 全匹配）
//   ③ 替换 / 分割
//
// 用法： go run ./examples/m138_regex_go_parity/truth
package main

import (
	"fmt"
	"regexp"
	"strings"
	"unicode/utf8"
)

// esc —— 与 parity.px::esc 逐字节同构：可打印 ASCII 原样，其余 `\xNN`（按 rune）
func esc(s string) string {
	var b strings.Builder
	for _, r := range s {
		if r >= 32 && r != 127 {
			b.WriteRune(r)
		} else {
			fmt.Fprintf(&b, "\\x%02x", r)
		}
	}
	return b.String()
}

func emitF(pat, text string) {
	line := "f|" + esc(pat) + "|" + esc(text) + "|"
	re, err := regexp.Compile(pat)
	if err != nil {
		fmt.Println(line + "0|-|-|-|-")
		return
	}
	line += "1|"
	if loc := re.FindStringIndex(text); loc == nil {
		line += "-|"
	} else {
		line += esc(text[loc[0]:loc[1]]) + "|"
	}
	all := re.FindAllStringIndex(text, -1)
	line += fmt.Sprintf("%d:", len(all))
	for k, m := range all {
		if k > 0 {
			line += ","
		}
		line += esc(text[m[0]:m[1]])
	}
	line += "|"
	sm := re.FindStringSubmatchIndex(text)
	if sm == nil {
		line += "-|-"
	} else {
		line += fmt.Sprintf("%d,%d|", sm[0], sm[1])
		ns := re.NumSubexp()
		for g := 0; g < 9; g++ {
			if g > 0 {
				line += ","
			}
			if g < ns {
				s, e := sm[2*(g+1)], sm[2*(g+1)+1]
				if s < 0 {
					line += "-"
				} else {
					line += esc(text[s:e])
				}
			} else {
				line += "-"
			}
		}
		line += "|"
	}
	re2, err2 := regexp.Compile("^(?:" + pat + ")$")
	if err2 == nil && re2.MatchString(text) {
		line += "1"
	} else {
		line += "0"
	}
	fmt.Println(line)
}

func emitR(pat, text, repl string) {
	line := "r|" + esc(pat) + "|" + esc(text) + "|" + esc(repl) + "|"
	re, err := regexp.Compile(pat)
	if err != nil {
		fmt.Println(line + "0|-")
		return
	}
	fmt.Println(line + "1|" + esc(re.ReplaceAllString(text, repl)))
}

func emitS(pat, text string) {
	line := "s|" + esc(pat) + "|" + esc(text) + "|"
	re, err := regexp.Compile(pat)
	if err != nil {
		fmt.Println(line + "0|-")
		return
	}
	parts := re.Split(text, -1)
	line += fmt.Sprintf("1|%d:", len(parts))
	for k, p := range parts {
		if k > 0 {
			line += ","
		}
		line += esc(p)
	}
	fmt.Println(line)
}

// posEnum —— 「枚举所有位置」的模式：其匹配**条数** = 文本长度 + 1（引擎的"长度"口径不同）。
//   PuXian 引擎**按字节**匹配（`.` 也吃一个字节），Go 按 rune；非 ASCII 文本上两者位置数不同。
//   ⇒ 这类模式只喂 ASCII 文本；非 ASCII 的行为由 parity.px 的自断言**显式登记**为已知边界。
func posEnum(pat string) bool {
	switch pat {
	case `\b{0}`, `\b*`, `\b?`, `\B`, `\B{2}`, `$*`:
		return true
	}
	return false
}

func isASCII(s string) bool {
	return len(s) == utf8.RuneCountInString(s)
}

func main() {
	texts := []string{
		"hello world", "foo_bar baz9", "api_key=secret", "  api_key: abc  ",
		"", "a", "_", "9", "!!", "pass the ball", "a-b c_d",
		"\t", "\n", "\v",
		// M138 追加：内联标志 / 命名组的作用面（含换行文本，`(?s)` `(?m)` 要看行）
		"ab\ncd", "X BEARER TOK", "AK-ABC", "a\nb", "MiXeD case",
	}
	pats := []string{
		`\b`, `\B`, `\b\w+\b`, `\bapi_key\b`, `\bfoo`, `bar\b`, `\Bfoo`, `a\Bb`,
		`(?:\b|\B)a`, `\b(a|b)\b`, `\b{2}`, `\b{0}`, `\b*`, `\b+`, `\b?`, `\B{2}`,
		`^{2}`, `$*`, `$+`, `\A`, `\z`, `^`, `$`, `^\bfoo`, `\bfoo\z`, `(\b\w+)`,
		`\b\w*_\w*\b`, `\s`, `\S`, `\s+`, `\d+`, `[\d\s]+`, `[\D]+`, `[\w-]+`,
		`[\b]`, `[\D\s]`, `a\b`, `\w+\b`, `\Qa.b\E`, `\A\b\w+\b\z`, `(\w+)-(\w+)`,
		`(?i)bearer`, `(?i)[a-z]+`, `(?s).*`, `(?s).+`, `(?m)^b`, `(?m)$`,
		`(?i)(?P<x>abc)`, `(?i)AK-[A-Za-z0-9]{3,}`, `(?is)a.b`,
		`(?i)[\w.-]+@[\w.-]+\.\w{2,}`,
	}
	// ① 穷举可打印 ASCII 的转义（类外 + 类内）
	for b := 0x20; b <= 0x7e; b++ {
		if b == '\\' {
			continue
		}
		c := string(rune(b))
		emitF("\\"+c, "abc")
		emitF("[\\"+c+"]", "abc")
	}
	// ② 词边界 / 锚点语义
	for _, p := range pats {
		for _, t := range texts {
			if posEnum(p) && !isASCII(t) {
				continue
			}
			emitF(p, t)
		}
	}
	// ③ 替换 / 分割
	emitR(`a*`, "baab", "X")
	emitR(`a*`, "b", "X")
	emitR(`\b`, "ab cd", "X")
	emitR(`(\w+)@(\w+)`, "mail: foo@bar.com", "<$1|$2>")
	emitR(`a`, "aaa", "$$")
	emitR(`\bapi_key\b`, "api_key xapi_key", "[K]")
	emitR(`\s+`, "a  b\tc", "_")
	emitR(`\b(\w+)\b`, "one two", "$1!")
	emitS(`[,\s]+`, "a, b  c")
	emitS(`\s+`, "a b\tc")
	emitS(`-`, "a-b-c")
	emitS(`\d+`, "a1b22c")
	emitS(`x`, "abc")
}
