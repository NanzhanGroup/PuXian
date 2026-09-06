// M83-S4 互通对拍辅助（Go crypto/rsa PKCS1v15-SHA256，对标 ws-pay 商户签名 / DKIM rsa-sha256）
// 用法：
//   go_s4 gen <dir>              生成 artifacts：pkcs8.pem（BEGIN PRIVATE KEY）/ pk1.pem（BEGIN RSA PRIVATE KEY）/
//                                pub_spki.pem（BEGIN PUBLIC KEY）/ pub_pkcs1.pem（BEGIN RSA PUBLIC KEY）/
//                                msg.txt（含中文）/ binmsg.bin（含 NUL 非 utf8）/
//                                go.sig.hex / go_bin.sig.hex（Go rsa.SignPKCS1v15 对 msg/binmsg 的签名）
//   go_s4 verify <pub.pem> <msg文件> <sig.hex文件>   → stdout "true"/"false"
//                                （pub.pem 支持 PUBLIC KEY=SPKI 或 RSA PUBLIC KEY=PKCS1 两种）
package main

import (
	"crypto"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha256"
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

func signHex(priv *rsa.PrivateKey, msg []byte) []byte {
	sum := sha256.Sum256(msg)
	sig, err := rsa.SignPKCS1v15(rand.Reader, priv, crypto.SHA256, sum[:])
	must(err)
	return sig
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "need subcommand: gen <dir> | verify <pub.pem> <msg> <sig.hex>")
		os.Exit(2)
	}
	switch os.Args[1] {
	case "gen":
		dir := os.Args[2]
		priv, err := rsa.GenerateKey(rand.Reader, 2048)
		must(err)
		msg := []byte("M83-S4 RSA PKCS1v15-SHA256 互通对拍 payload line2\n第三条 data 中文 payload\n")
		binmsg := []byte{0x00, 0xed, 0xff, 'p', 'x', 0x00, 0x01, 'g', 'o', 0xfe, 0x00}
		writeHex(dir, "go.sig.hex", signHex(priv, msg))
		writeHex(dir, "go_bin.sig.hex", signHex(priv, binmsg))
		must(os.WriteFile(filepath.Join(dir, "msg.txt"), msg, 0o644))
		must(os.WriteFile(filepath.Join(dir, "binmsg.bin"), binmsg, 0o644))
		// PKCS8 私钥 PEM（BEGIN PRIVATE KEY）
		p8, err := x509.MarshalPKCS8PrivateKey(priv)
		must(err)
		must(os.WriteFile(filepath.Join(dir, "pkcs8.pem"),
			pem.EncodeToMemory(&pem.Block{Type: "PRIVATE KEY", Bytes: p8}), 0o644))
		// PKCS1 私钥 PEM（BEGIN RSA PRIVATE KEY）
		p1 := x509.MarshalPKCS1PrivateKey(priv)
		must(os.WriteFile(filepath.Join(dir, "pk1.pem"),
			pem.EncodeToMemory(&pem.Block{Type: "RSA PRIVATE KEY", Bytes: p1}), 0o644))
		// SPKI 公钥 PEM（BEGIN PUBLIC KEY）
		spki, err := x509.MarshalPKIXPublicKey(&priv.PublicKey)
		must(err)
		must(os.WriteFile(filepath.Join(dir, "pub_spki.pem"),
			pem.EncodeToMemory(&pem.Block{Type: "PUBLIC KEY", Bytes: spki}), 0o644))
		// PKCS1 公钥 PEM（BEGIN RSA PUBLIC KEY）
		pk1 := x509.MarshalPKCS1PublicKey(&priv.PublicKey)
		must(os.WriteFile(filepath.Join(dir, "pub_pkcs1.pem"),
			pem.EncodeToMemory(&pem.Block{Type: "RSA PUBLIC KEY", Bytes: pk1}), 0o644))
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
		var pub *rsa.PublicKey
		switch blk.Type {
		case "PUBLIC KEY":
			anyPub, err := x509.ParsePKIXPublicKey(blk.Bytes)
			if err != nil {
				fmt.Println("false")
				return
			}
			pub, _ = anyPub.(*rsa.PublicKey)
		case "RSA PUBLIC KEY":
			pub, err = x509.ParsePKCS1PublicKey(blk.Bytes)
			if err != nil {
				fmt.Println("false")
				return
			}
		default:
			fmt.Println("false")
			return
		}
		if pub == nil {
			fmt.Println("false")
			return
		}
		sum := sha256.Sum256(msg)
		fmt.Println(rsa.VerifyPKCS1v15(pub, crypto.SHA256, sum[:], sig) == nil)
	default:
		fmt.Fprintln(os.Stderr, "unknown subcommand")
		os.Exit(2)
	}
}
