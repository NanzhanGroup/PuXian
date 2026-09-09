// M103-S2b (Issue 29 GAP-ED25519-2) 对拍参照：Go crypto/ed25519 + x509 (RFC 8410)
//   gen    生成 Go 密钥对 → SPKI PEM(go_pub.pem) + 消息 + 签名(go_sig.hex)，供 px 反向验
//   verify 读 px keygen 产物做字节级互通断言：
//     (1) ParsePKIX(px_pk.pem).hex == px_pk_hex.txt      —— SPKI DER 与 Go 逐字节一致
//     (2) ParsePKCS8(px_sk.pem) 派生公钥 hex == pk_hex     —— PKCS8 seed 与 pk 同源
//     (3) ed25519.Verify(px_pk.pem, px_msg, px_sig) == true —— Go 验 px 签
//     (4) ed25519.Verify(派生 priv.Public(), msg, sig2) —— PKCS8 导入验签
// 依赖：go；用法：go build -o build/go_s2b go_s2b.go && ./go_s2b gen|verify
package main

import (
	"crypto/ed25519"
	"crypto/rand"
	"crypto/x509"
	"encoding/hex"
	"encoding/pem"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

const msg = "m103-s2b keygen interop message"

func fatal(err error) {
	fmt.Fprintln(os.Stderr, "ERR", err)
	os.Exit(1)
}

func readF(p string) []byte {
	d, err := os.ReadFile(p)
	if err != nil {
		fatal(err)
	}
	return d
}

func writeF(p string, d []byte) {
	if err := os.WriteFile(p, d, 0644); err != nil {
		fatal(err)
	}
}

func pubHexFromPEM(pemBytes []byte) string {
	blk, _ := pem.Decode(pemBytes)
	if blk == nil {
		fatal(fmt.Errorf("bad pem"))
	}
	pub, err := x509.ParsePKIXPublicKey(blk.Bytes)
	if err != nil {
		fatal(err)
	}
	return hex.EncodeToString(pub.(ed25519.PublicKey))
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "usage: go_s2b gen|verify")
		os.Exit(2)
	}
	os.MkdirAll("artifacts", 0755)
	switch os.Args[1] {
	case "gen":
		pub, priv, err := ed25519.GenerateKey(rand.Reader)
		if err != nil {
			fatal(err)
		}
		spki, err := x509.MarshalPKIXPublicKey(pub)
		if err != nil {
			fatal(err)
		}
		writeF("artifacts/go_pub.pem",
			pem.EncodeToMemory(&pem.Block{Type: "PUBLIC KEY", Bytes: spki}))
		writeF("artifacts/go_msg.txt", []byte(msg))
		writeF("artifacts/go_sig.hex", []byte(hex.EncodeToString(ed25519.Sign(priv, []byte(msg)))))
		fmt.Println("go gen done")
	case "verify":
		pkHex := strings.TrimSpace(string(readF("artifacts/px_pk_hex.txt")))
		skPem := readF("artifacts/px_sk.pem")
		pkPem := readF("artifacts/px_pk.pem")
		pxMsg := readF("artifacts/px_msg.txt")
		pxSig, err := hex.DecodeString(strings.TrimSpace(string(readF("artifacts/px_sig.hex"))))
		if err != nil {
			fatal(err)
		}
		// (1) SPKI DER 与 px pk_hex 逐字节一致
		if got := pubHexFromPEM(pkPem); got != pkHex {
			fatal(fmt.Errorf("SPKI pk 不一致: pem=%s hex=%s", got, pkHex))
		}
		fmt.Println("PASS (1) SPKI PEM pub == pk_hex")
		// (2) PKCS8 导入 → 派生公钥 == pk_hex（seed 同源）
		blk, _ := pem.Decode(skPem)
		if blk == nil {
			fatal(fmt.Errorf("px_sk.pem bad"))
		}
		kp, err := x509.ParsePKCS8PrivateKey(blk.Bytes)
		if err != nil {
			fatal(err)
		}
		priv := kp.(ed25519.PrivateKey)
		if got := hex.EncodeToString(priv.Public().(ed25519.PublicKey)); got != pkHex {
			fatal(fmt.Errorf("PKCS8 派生 pk 不一致: %s != %s", got, pkHex))
		}
		fmt.Println("PASS (2) PKCS8 seed 派生 pub == pk_hex")
		// (3) Go 用 px SPKI 公钥验 px 签
		if !ed25519.Verify(ed25519.PublicKey(priv.Public().(ed25519.PublicKey)), pxMsg, pxSig) {
			fatal(fmt.Errorf("Go 验 px 签失败"))
		}
		fmt.Println("PASS (3) Go 验 px 签 (PKCS8 派生 pub)")
		fmt.Println("m103_s2b_ed25519_keygen interop OK")
	default:
		fmt.Fprintln(os.Stderr, "usage: go_s2b gen|verify")
		os.Exit(2)
	}
}

// keep filepath imported (future use)
var _ = filepath.Join
