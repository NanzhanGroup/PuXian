#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M404：WinZip AES-256（AE-2）测试包**自备**生成器 —— 零第三方依赖。

背景（缺陷 766 同族）：
  examples/m66_proc 的 [6/6] 段需要 pyzipper 才能**造**一个 WinZip AES-256 加密包，
  而本机与 CI 都没有 pip3、dnf 也无此包 ⇒ 该段长期 SKIP，「SKIP 判绿」= 零覆盖。
处置：本脚本用 **纯标准库 + openssl CLI** 直接按 APPNOTE 6.3 拼 AE-2 包，
      不再依赖任何 python 第三方包（openssl 是系统基础件）。

AE-2 单条目格式（与本仓 runtime_zip.c 的解密实现对齐）：
  file data = salt(16B, AES-256) || pv(2B) || ciphertext || authcode(10B)
  密钥派生 = PBKDF2-HMAC-SHA1(pw, salt, 1000, dkLen = 2*32 + 2)
             → enc_key(32) || mac_key(32) || pv(2)
  加密     = AES-256-CTR，16 字节计数器小端从 1 起（首 4 字节递增，其余为 0）
  认证     = HMAC-SHA1(mac_key, ciphertext) 取前 10 字节
  extra    = 0x9901: version(2)=2 || 'AE' || strength(1)=3 || real_method(2)

用法：python3 make_aes_fixture.py <输出.zip>   （口令固定 m66aes-pass）
"""
import hashlib
import hmac
import os
import struct
import subprocess
import sys
import tempfile
import zlib

PASSWORD = b"m66aes-pass"
STRENGTH = 3                      # 3 = AES-256
KEYLEN = 32
SALTLEN = 16
ITER = 1000


def aes_ctr_keystream(key: bytes, nbytes: int) -> bytes:
    """用 openssl 做 AES-256-ECB 加密计数器块，得到 CTR 密钥流（无第三方 AES 实现）。"""
    nblocks = (nbytes + 15) // 16
    counters = b"".join(struct.pack("<I", i) + b"\x00" * 12 for i in range(1, nblocks + 1))
    with tempfile.NamedTemporaryFile(delete=False) as f:
        f.write(counters)
        ctr_path = f.name
    out_path = ctr_path + ".ks"
    try:
        subprocess.run(
            ["openssl", "enc", "-aes-256-ecb", "-nopad", "-K", key.hex(),
             "-in", ctr_path, "-out", out_path],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        )
        with open(out_path, "rb") as f:
            ks = f.read()
    finally:
        for p in (ctr_path, out_path):
            try:
                os.unlink(p)
            except OSError:
                pass
    return ks[:nbytes]


def xor(a: bytes, b: bytes) -> bytes:
    return bytes(x ^ y for x, y in zip(a, b))


def build_entry(name: str, data: bytes) -> bytes:
    """拼一个 AES-256 加密条目的 (local_header + data)。"""
    salt = bytes(range(1, SALTLEN + 1))          # 确定性盐：夹具必须可复算
    dk = hashlib.pbkdf2_hmac("sha1", PASSWORD, salt, ITER, 2 * KEYLEN + 2)
    enc_key, mac_key, pv = dk[:KEYLEN], dk[KEYLEN:2 * KEYLEN], dk[-2:]

    comp = zlib.compressobj(9, zlib.DEFLATED, -15)
    comp_data = comp.compress(data) + comp.flush()
    ct = xor(comp_data, aes_ctr_keystream(enc_key, len(comp_data)))
    auth = hmac.new(mac_key, ct, hashlib.sha1).digest()[:10]
    payload = salt + pv + ct + auth

    crc = zlib.crc32(data) & 0xFFFFFFFF
    ver = 51                                      # 5.1 = AES
    flags = 0x0001                                # encrypted
    method = 99                                   # 99 = AE-x
    n = name.encode("utf-8")
    extra = struct.pack("<HHH2sBH", 0x9901, 7, 2, b"AE", STRENGTH, 8)  # 真实方法=8(deflate)
    lh = struct.pack("<IHHHHHIIIHH", 0x04034B50, ver, flags, method, 0, 0x21,
                     crc, len(payload), len(data), len(n), len(extra))
    return lh + n + extra + payload


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    out = sys.argv[1]
    entries = [
        ("a.txt", "hello aes-256 中文\n".encode("utf-8")),
        ("sub/c.txt", b"aes-secret\n"),
    ]
    body = b""
    central = b""
    for name, data in entries:
        blob = build_entry(name, data)
        # 从 local header 里把各自字段取出，供中央目录复用（保持两处一致）
        (sig, ver, flags, method, _t, _d, crc, csize, usize,
         nlen, elen) = struct.unpack("<IHHHHHIIIHH", blob[:30])
        name_b = blob[30:30 + nlen]
        extra = blob[30 + nlen:30 + nlen + elen]
        offset = len(body)
        body += blob
        central += struct.pack("<IHHHHHHIIIHHHHHII", 0x02014B50, ver, ver, flags, method,
                               0, 0x21, crc, csize, usize, nlen, elen, 0, 0, 0, 0, offset)
        central += name_b + extra
    eocd = struct.pack("<IHHHHIIH", 0x06054B50, 0, 0, len(entries), len(entries),
                       len(central), len(body), 0)
    with open(out, "wb") as f:
        f.write(body + central + eocd)
    print("AE-256 zip written: %s (%d bytes, %d entries, pw=%s)"
          % (out, os.path.getsize(out), len(entries), PASSWORD.decode()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
