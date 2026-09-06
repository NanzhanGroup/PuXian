// M84-S2 (Issue 21 GAP-HMAC-1) 互通对拍辅助（Go crypto/hmac + crypto/sha256，权威实现）
// 用法：
//   go_s2 gen <dir>  生成 artifacts：RFC4231 TC1-4/6 的 key/data（hex 文件）与期望 hmac（go_tcN.hex）、
//                    msg.txt / binmsg.bin（含 NUL 二进制）、文本/二进制 hmac 期望、
//                    sha256 期望（文本/二进制/ASCII/空串）、腾讯云 TC3 4 级链各中间态与最终签名。
// 所有期望值均小写 hex，px 端复算逐一比对。
package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/hex"
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
func writeStr(dir, name, s string) {
	must(os.WriteFile(filepath.Join(dir, name), []byte(s), 0o644))
}

func hm(key, data []byte) []byte {
	m := hmac.New(sha256.New, key)
	m.Write(data)
	return m.Sum(nil)
}

func bytesOfHex(s string) []byte {
	b, err := hex.DecodeString(s)
	must(err)
	return b
}

func main() {
	if len(os.Args) < 3 || os.Args[1] != "gen" {
		fmt.Fprintln(os.Stderr, "need: go_s2 gen <dir>")
		os.Exit(2)
	}
	dir := os.Args[2]

	// ---- 消息载体 ----
	msg := []byte("M84-S2 HMAC-SHA256 互通对拍 payload line2\nthird line with spec chars !@#$%^&*()\n")
	binmsg := []byte{0x00, 0x01, 0xfe, 'h', 'm', 'a', 'c', 0x00, 'p', 'x', 0xff, 0x80, 0x7f}
	writeStr(dir, "msg.txt", string(msg))
	must(os.WriteFile(filepath.Join(dir, "binmsg.bin"), binmsg, 0o644))

	// ---- RFC 4231 用例（key/data 均 hex 落盘，px 用 hex_to_bytes 还原后复算）----
	// TC1: key=0x0b*20, data="Hi There"
	tc1k := bytesOfHex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b")
	writeHex(dir, "tc1_key.hex", tc1k)
	writeHex(dir, "tc1_data.hex", []byte("Hi There"))
	writeHex(dir, "go_tc1.hex", hm(tc1k, []byte("Hi There")))
	// TC2: key="Jefe", data="what do ya want for nothing?"
	writeStr(dir, "tc2_key.hex", "4a656665") // "Jefe"
	writeHex(dir, "tc2_data.hex", []byte("what do ya want for nothing?"))
	writeHex(dir, "go_tc2.hex", hm([]byte("Jefe"), []byte("what do ya want for nothing?")))
	// TC3: key=0xaa*20, data=0xdd*50
	tc3k := bytesOfHex("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
	tc3d := bytesOfHex("dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd")
	writeHex(dir, "tc3_key.hex", tc3k)
	writeHex(dir, "tc3_data.hex", tc3d)
	writeHex(dir, "go_tc3.hex", hm(tc3k, tc3d))
	// TC4: key=0x0102..0x19, data=0xcd*50
	tc4k := bytesOfHex("0102030405060708090a0b0c0d0e0f10111213141516171819")
	tc4d := bytesOfHex("cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd")
	writeHex(dir, "tc4_key.hex", tc4k)
	writeHex(dir, "tc4_data.hex", tc4d)
	writeHex(dir, "go_tc4.hex", hm(tc4k, tc4d))
	// TC6: key=0xaa*131 (>64B 触发 key 先哈希), data=文本
	tc6k := bytesOfHex("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
	tc6data := []byte("Test Using Larger Than Block-Size Key - Hash Key First")
	writeHex(dir, "tc6_key.hex", tc6k)
	writeHex(dir, "tc6_data.hex", tc6data)
	writeHex(dir, "go_tc6.hex", hm(tc6k, tc6data))

	// ---- 文本/二进制 key × 文本/二进制 msg 对拍 ----
	skey := []byte("puxian-secret-2026")
	writeHex(dir, "go_hmac_strkey_strmsg.hex", hm(skey, msg))
	writeHex(dir, "go_hmac_strkey_binmsg.hex", hm(skey, binmsg))
	bkey := []byte{0x5a, 0x00, 0x1b, 'K', 'e', 'y', 0x00, 0xff}
	writeHex(dir, "go_hmac_binkey_binmsg.hex", hm(bkey, binmsg))
	writeHex(dir, "go_hmac_binkey_strmsg.hex", hm(bkey, msg))

	// ---- sha256 期望（bytes 含 NUL / ASCII / 空串）----
	sh := func(b []byte) []byte { s := sha256.Sum256(b); return s[:] }
	writeHex(dir, "go_sha_bin.hex", sh(binmsg))
	writeHex(dir, "go_sha_msg.hex", sh(msg))
	writeHex(dir, "go_sha_abc.hex", sh([]byte("abc")))
	writeHex(dir, "go_sha_empty.hex", sh(nil))

	// ---- 腾讯云 TC3（TC3-HMAC-SHA256）4 级链端到端（官方签名方法结构）----
	secret := "Gu5t9xGARNpq86cd98joQYCN3Cozk1qA"
	date := "2019-02-25"
	service := "cvm"
	canonical := "GET\n/\nAction=DescribeInstances&Limit=1&Nonce=11886&Offset=0&Region=ap-guangzhou&Timestamp=1551117345&Version=2017-03-12\ncontent-type:application/json; charset=utf-8\nhost:cvm.tencentcloudapi.com\n\ncontent-type;host\n35e9c5e0e79ae38f8df6a5b003f9b3c0c5b1f9e8a7d6c5b4a3928170d6c5b4a3"
	writeStr(dir, "tc3_canonical.txt", canonical)
	kDate := hm([]byte("TC3"+secret), []byte(date)) // 用 secret 变量（若 var 用不上则常量展开）
	_ = kDate
	_ = service
	_ = canonical
	// 直接展开计算（避免上文冗余）
	kDate = hm([]byte("TC3Gu5t9xGARNpq86cd98joQYCN3Cozk1qA"), []byte("2019-02-25"))
	writeHex(dir, "tc3_kdate.hex", kDate)
	kService := hm(kDate, []byte("cvm"))
	writeHex(dir, "tc3_kservice.hex", kService)
	kSigning := hm(kService, []byte("tc3_request"))
	writeHex(dir, "tc3_ksigning.hex", kSigning)
	canon := []byte("GET\n/\nAction=DescribeInstances&Limit=1&Nonce=11886&Offset=0&Region=ap-guangzhou&Timestamp=1551117345&Version=2017-03-12\ncontent-type:application/json; charset=utf-8\nhost:cvm.tencentcloudapi.com\n\ncontent-type;host\n35e9c5e0e79ae38f8df6a5b003f9b3c0c5b1f9e8a7d6c5b4a3928170d6c5b4a3")
	writeHex(dir, "tc3_signature.hex", hm(kSigning, canon))

	fmt.Println("gen ok ->", dir)
}
