package main

// 生成 Go syscall.Errno.Error() 文案表（Linux amd64）——用于 PuXian runtime 的
// go_errno_string native（对齐 Go 的 *PathError.Error() 文案段）。
// 单一事实源 = Go 自身（不手抄、不猜）。
import (
	"fmt"
	"strings"
	"syscall"
)

func cstr(s string) string {
	// C 字符串字面量转义（表内只有字母/空格/连字符/撇号，仍做完整转义以防万一）
	var b strings.Builder
	for i := 0; i < len(s); i++ {
		c := s[i]
		switch c {
		case '"':
			b.WriteString("\\\"")
		case '\\':
			b.WriteString("\\\\")
		default:
			if c < 0x20 || c >= 0x7f {
				fmt.Fprintf(&b, "\\x%02x", c)
			} else {
				b.WriteByte(c)
			}
		}
	}
	return b.String()
}

func main() {
	fmt.Println("// Go syscall.Errno 文案表（Linux amd64）：由 tools/gen_go_errno_table.go 从 Go 自身导出")
	fmt.Println("// 索引 = errno 值；空串 = Go 该值无文案 ⇒ 回落 \"errno <N>\"（与 Go Errno.Error() 一致）")
	fmt.Println("static const char* const GO_ERRNO_STR[] = {")
	for i := 0; i <= 140; i++ {
		s := syscall.Errno(i).Error()
		if s == fmt.Sprintf("errno %d", i) {
			s = "" // 与 Go 的"无文案"分支等价，避免表内自指
		}
		fmt.Printf("    /* %3d */ \"%s\",\n", i, cstr(s))
	}
	fmt.Println("};")
	fmt.Println("static const int GO_ERRNO_STR_N = (int)(sizeof(GO_ERRNO_STR) / sizeof(GO_ERRNO_STR[0]));")
}
