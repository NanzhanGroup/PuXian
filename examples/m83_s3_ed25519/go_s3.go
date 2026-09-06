// M83-S3 互通对拍辅助（Go crypto/ed25519，RFC8032）
// 用法：
//   go_s3 gen <dir>              生成 artifacts：pkcs8.pem / pub.pem / msg.txt / binmsg.bin /
//                                seed.hex / sk64.hex / go.sig.hex（msg 的 Go 签名）/
//                                go_bin.sig.hex（binmsg 的 Go 签名）
//   go_s3 verify <pub.pem> <msg文件> <sig.hex文件>    → stdout "true"/"false"
package main

import (
	"crypto/ed25519"
	"crypto/x509"
	"encoding/hex"
	"encoding/pem"
	"fmt"
	"os"
	"path/filepath"
)

func must(err error) {
	if err != nil {
		fmt.Fprintln(os.Stderr, "err:", err)
		os.Exit(1)
	}
}

func writeHex(dir, name string, b []byte) {
	must(os.WriteFile(filepath.Join(dir, name), []byte(hex.EncodeToString(b)), 0o644))
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "need subcommand: gen <dir> | verify <pub.pem> <msg> <sig.hex>")
		os.Exit(2)
	}
	switch os.Args[1] {
	case "gen":
		dir := os.Args[2]
		pub, priv, err := ed25519.GenerateKey(nil)
		must(err)
		msg := []byte("M83-S3 ed25519 互通对拍 payload line2\n第三条 data 中文\n")
		binmsg := []byte{0x00, 0xed, 0xff, 'p', 'x', 0x00, 0x01, 'g', 'o', 0xfe}
		writeHex(dir, "seed.hex", priv.Seed())
		writeHex(dir, "sk64.hex", priv)
		writeHex(dir, "go.sig.hex", ed25519.Sign(priv, msg))
		writeHex(dir, "go_bin.sig.hex", ed25519.Sign(priv, binmsg))
		must(os.WriteFile(filepath.Join(dir, "msg.txt"), msg, 0o644))
		must(os.WriteFile(filepath.Join(dir, "binmsg.bin"), binmsg, 0o644))
		p8, err := x509.MarshalPKCS8PrivateKey(priv)
		must(err)
		must(os.WriteFile(filepath.Join(dir, "pkcs8.pem"),
			pem.EncodeToMemory(&pem.Block{Type: "PRIVATE KEY", Bytes: p8}), 0o644))
		spki, err := x509.MarshalPKIXPublicKey(pub)
		must(err)
		must(os.WriteFile(filepath.Join(dir, "pub.pem"),
			pem.EncodeToMemory(&pem.Block{Type: "PUBLIC KEY", Bytes: spki}), 0o644))
		writeHex(dir, "pub.hex", pub)
		fmt.Println("gen ok ->", dir)
	case "verify":
		pubPem, err := os.ReadFile(os.Args[2])
		must(err)
		msg, err := os.ReadFile(os.Args[3])
		must(err)
		sigHex, err := os.ReadFile(os.Args[4])
		must(err)
		sig, err := hex.DecodeString(string(sigHex))
		must(err)
		blk, _ := pem.Decode(pubPem)
		if blk == nil {
			fmt.Println("false")
			return
		}
		pubAny, err := x509.ParsePKIXPublicKey(blk.Bytes)
		if err != nil {
			fmt.Println("false")
			return
		}
		pub, ok := pubAny.(ed25519.PublicKey)
		if !ok {
			fmt.Println("false")
			return
		}
		fmt.Println(ed25519.Verify(pub, msg, sig))
	default:
		fmt.Fprintln(os.Stderr, "unknown subcommand")
		os.Exit(2)
	}
}
