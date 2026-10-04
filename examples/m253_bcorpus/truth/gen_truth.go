// M253 真值生成器 —— 用 **Go 标准库**独立算出本门需要的两份真值：
//
//	aes_truth.tsv   AES-CBC / AES-GCM / AES-ECB 的已知向量（crypto/aes + crypto/cipher）
//	errno_truth.tsv Go `syscall.Errno(n).Error()` 的全表（0..140）
//
// 为什么必须「独立来源」：被测实现（runtime/runtime_aes.c · runtime.c）是 **C + mbedtls**，
// 而本生成器是 **Go**。两边只有算法标准相同、代码零共享 ⇒ 这才是**跨语言第三份真值**
// （M215 的立论：三轨一致 ≠ 正确；三轨跑的是同一段代码）。
//
// 用法：go run truth/gen_truth.go   （输出到本文件所在目录）
//   ⚠️ 本仓 CI **没有 Go 步骤**（口径同 m136/m138）⇒ CI 只用**已入库**的 tsv；
//      重生成需要 Go，且**必须把 diff 逐行看懂**再提交（真值变了 = 要么 Go 变了、要么写法变了）。
package main

import (
	"crypto/aes"
	"crypto/cipher"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"syscall"
)

func pkcs7Pad(b []byte, bs int) []byte {
	pad := bs - len(b)%bs
	out := make([]byte, 0, len(b)+pad)
	out = append(out, b...)
	for i := 0; i < pad; i++ {
		out = append(out, byte(pad))
	}
	return out
}

func cbcEnc(key, iv, pt []byte) []byte {
	blk, err := aes.NewCipher(key)
	if err != nil {
		panic(err)
	}
	p := pkcs7Pad(pt, 16)
	out := make([]byte, len(p))
	cipher.NewCBCEncrypter(blk, iv).CryptBlocks(out, p)
	return out
}

func gcmEnc(key, nonce, pt []byte) []byte {
	blk, err := aes.NewCipher(key)
	if err != nil {
		panic(err)
	}
	g, err := cipher.NewGCM(blk)
	if err != nil {
		panic(err)
	}
	// Seal(dst, nonce, plaintext, additionalData) → dst || ciphertext || tag(16)
	return g.Seal(nil, nonce, pt, nil)
}

func ecbEnc(key, pt []byte) []byte {
	blk, err := aes.NewCipher(key)
	if err != nil {
		panic(err)
	}
	p := pkcs7Pad(pt, 16)
	out := make([]byte, len(p))
	for off := 0; off < len(p); off += 16 {
		blk.Encrypt(out[off:off+16], p[off:off+16])
	}
	return out
}

func main() {
	here, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	if _, err := os.Stat(filepath.Join(here, "gen_truth.go")); err != nil {
		// 被 go run 时 os.Args[0] 在临时目录 ⇒ 退回相对 CWD
		if wd, e := os.Getwd(); e == nil {
			here = filepath.Join(wd, "truth")
		}
	}
	keys := map[string][]byte{
		"k16": []byte("0123456789abcdef"),
		"k24": []byte("0123456789abcdef01234567"),
		"k32": []byte("0123456789abcdef0123456789abcdef"),
	}
	iv := []byte("abcdefghijklmnop")
	nonce := []byte("abcdefghijkl") // 12 字节
	bin32 := make([]byte, 32)
	for i := 0; i < 16; i++ {
		bin32[i] = byte(i)
		bin32[16+i] = byte(0xF0 + i)
	}
	type pt struct {
		n string
		d []byte
	}
	pts := []pt{
		{"p0", []byte("")},
		{"p1", []byte("A")},
		{"p15", []byte(strings.Repeat("A", 15))},
		{"p16", []byte(strings.Repeat("A", 16))},
		{"p17", []byte(strings.Repeat("A", 17))},
		{"p31", []byte(strings.Repeat("A", 31))},
		{"p32", []byte(strings.Repeat("A", 32))},
		{"p33", []byte(strings.Repeat("A", 33))},
		{"putf8", []byte("中文测试")},
		{"pbin32", bin32},
		{"pbin7", []byte{0x00, 0x01, 0x02, 0xFE, 0xFF, 0x80, 0x7F}},
	}
	hx := func(b []byte) string { return fmt.Sprintf("%x", b) }
	order := []string{"k16", "k24", "k32"}

	var f strings.Builder
	f.WriteString("# M253 AES 真值 —— 由 truth/gen_truth.go（Go crypto/aes + crypto/cipher）生成；**勿手改**\n")
	for _, kn := range order {
		for _, p := range pts {
			fmt.Fprintf(&f, "CBC\t%s\t%s\t%s\n", kn, p.n, hx(cbcEnc(keys[kn], iv, p.d)))
		}
	}
	for _, kn := range order {
		for _, p := range pts {
			fmt.Fprintf(&f, "GCM\t%s\t%s\t%s\n", kn, p.n, hx(gcmEnc(keys[kn], nonce, p.d)))
		}
	}
	for _, kn := range order {
		for _, p := range pts {
			fmt.Fprintf(&f, "ECB\t%s\t%s\t%s\n", kn, p.n, hx(ecbEnc(keys[kn], p.d)))
		}
	}
	var e strings.Builder
	e.WriteString("# M253 errno 真值 —— 由 truth/gen_truth.go（Go syscall.Errno(n).Error()）生成；**勿手改**\n")
	for i := 0; i <= 140; i++ {
		fmt.Fprintf(&e, "%d\t%s\n", i, syscall.Errno(i).Error())
	}
	must(os.WriteFile(filepath.Join(here, "aes_truth.tsv"), []byte(f.String()), 0o644))
	must(os.WriteFile(filepath.Join(here, "errno_truth.tsv"), []byte(e.String()), 0o644))
	fmt.Printf("✅ aes_truth.tsv %d 行 · errno_truth.tsv %d 行\n",
		strings.Count(f.String(), "\n"), strings.Count(e.String(), "\n"))
}

func must(err error) {
	if err != nil {
		panic(err)
	}
}
