// M150 真值程序（Go 本尊）—— **只用标准库**（CI 无需联网取依赖）
//
// ① md5 / md5_bytes：crypto/md5
// ② pbkdf2_sha256  ：crypto/hmac + crypto/sha256 手写 PBKDF2（RFC 8018 §5.2），
//                    算法与 golang.org/x/crypto/pbkdf2 逐字节相同（不引外部依赖）
// ③ PostgreSQL md5 认证应答：crypto/md5 的嵌套用法（lib/pq conn.go 的同一条链）
// ④ cipher_suite_name <id>：crypto/tls.CipherSuiteName —— 供对拍 PuXian 的 cipher 文本口径
//
// 用法：go run truth/main.go <corpus.txt>          → M|/P|/A| 行
//       go run truth/main.go --csname <id>         → Go 的套件名文本
package main

import (
	"bufio"
	"crypto/hmac"
	"crypto/md5"
	"crypto/sha256"
	"crypto/tls"
	"encoding/hex"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// PBKDF2-HMAC-SHA256（RFC 8018 定义；与 x/crypto/pbkdf2 同算法）
func pbkdf2SHA256(password, salt []byte, iter, keyLen int) []byte {
	if iter <= 0 {
		iter = 1
	}
	prf := hmac.New(sha256.New, password)
	hLen := prf.Size()
	numBlocks := (keyLen + hLen - 1) / hLen
	var dk []byte
	U := make([]byte, hLen)
	T := make([]byte, hLen)
	for block := 1; block <= numBlocks; block++ {
		prf.Reset()
		prf.Write(salt)
		prf.Write([]byte{byte(block >> 24), byte(block >> 16), byte(block >> 8), byte(block)})
		U = prf.Sum(U[:0])
		copy(T, U)
		for n := 2; n <= iter; n++ {
			prf.Reset()
			prf.Write(U)
			U = prf.Sum(U[:0])
			for x := range T {
				T[x] ^= U[x]
			}
		}
		dk = append(dk, T...)
	}
	return dk[:keyLen]
}

// PostgreSQL `AuthenticationMD5Password` 的应答（lib/pq conn.go 同一条链）
func pgMD5Auth(password, user, salt []byte) string {
	inner := md5.Sum(append(append([]byte{}, password...), user...))
	h1 := hex.EncodeToString(inner[:])
	outer := md5.Sum(append([]byte(h1), salt...))
	return "md5" + hex.EncodeToString(outer[:])
}

func main() {
	if len(os.Args) == 3 && os.Args[1] == "--csname" {
		id64, err := strconv.ParseUint(os.Args[2], 10, 16)
		if err != nil {
			fmt.Fprintln(os.Stderr, "bad id")
			os.Exit(2)
		}
		fmt.Print(tls.CipherSuiteName(uint16(id64)))
		return
	}
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "用法: truth <corpus.txt> | truth --csname <id>")
		os.Exit(2)
	}
	f, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1<<20), 1<<20)
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	for sc.Scan() {
		line := sc.Text()
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		parts := strings.Split(line, "\t")
		switch parts[0] {
		case "M":
			in, err := hex.DecodeString(parts[1])
			if err != nil {
				fmt.Fprintln(os.Stderr, "bad hex:", parts[1])
				os.Exit(1)
			}
			sum := md5.Sum(in)
			fmt.Fprintf(w, "M|md5|%s|%s\n", parts[1], hex.EncodeToString(sum[:]))
			fmt.Fprintf(w, "M|md5b|%s|%d|%s\n", parts[1], len(sum), hex.EncodeToString(sum[:]))
		case "P":
			pw, _ := hex.DecodeString(parts[1])
			salt, _ := hex.DecodeString(parts[2])
			iter, _ := strconv.Atoi(parts[3])
			dklen, _ := strconv.Atoi(parts[4])
			key := pbkdf2SHA256(pw, salt, iter, dklen)
			fmt.Fprintf(w, "P|pbkdf2|%s|%s|%d|%d|%s\n", parts[1], parts[2], iter, dklen, hex.EncodeToString(key))
		case "A":
			pw, _ := hex.DecodeString(parts[1])
			user, _ := hex.DecodeString(parts[2])
			salt, _ := hex.DecodeString(parts[3])
			fmt.Fprintf(w, "A|pgauth|%s|%s|%s|%s\n", parts[1], parts[2], parts[3], pgMD5Auth(pw, user, salt))
		default:
			fmt.Fprintln(os.Stderr, "未知行:", line)
			os.Exit(1)
		}
	}
}
