#!/usr/bin/env python3
# M150 语料生成器（确定性，无外部依赖）
#
# 产出 examples/m150_tls_crypto/corpus.txt，三族行格式（TAB 分隔，输入一律 hex 承载
# ⇒ 语料**必然含 NUL / 0x80+ 字节**，文件本身仍是纯文本）：
#   M <hexinput>                              ① md5 / md5_bytes
#   P <hexpw> <hexsalt> <iters> <dklen>        ② pbkdf2_sha256
#   A <hexpw> <hexuser> <hexsalt>              ③ PostgreSQL md5 认证应答（lib/pq 的嵌套 MD5）
#
# 覆盖点（不是"随便取几条"）：
#   md5：RFC 1321 的 7 条向量 + **块边界**（54/55/56/57 与 119/120/127/128/129 ——
#        55/56 是 512-bit 分块的补位分界，多 1 字节就多一个块）+ 长输入（1KB/4KB）
#        + 全 0x00 / 全 0xFF + UTF-8 多字节 + 畸形 UTF-8（截断前导/孤立续字节）
#   pbkdf2：RFC 7914 §11 的 SHA-256 向量 + 迭代数 {1,2,3,10,100,1000,4096}
#        + dklen {1,16,20,31,32,33,64,100}（**跨块**：dklen>32 时第 2 块要正确续算）
#        + 空口令 / 空盐 / 长口令（>64 字节，触发 HMAC key 预哈希）
#   pg auth：口令/用户名/盐的多种长度（盐恒 4 字节 —— PostgreSQL 的固定宽度）
import sys, os, hashlib

HERE = os.path.dirname(os.path.abspath(__file__))

class LCG:
    """确定性伪随机（同第 23/27/29 轮各门的口径：Go uint64 回绕 = 本侧取模 2^64）"""
    def __init__(self, seed):
        self.s = seed & 0xFFFFFFFFFFFFFFFF
    def next(self):
        self.s = (self.s * 6364136223846793005 + 1442695040888963407) & 0xFFFFFFFFFFFFFFFF
        return self.s
    def bytes(self, n):
        out = bytearray()
        while len(out) < n:
            out += self.next().to_bytes(8, 'little')
        return bytes(out[:n])

def build_md5_lines():
    lines = []
    fixed = [b"", b"a", b"abc", b"message digest", b"abcdefghijklmnopqrstuvwxyz",
             b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
             b"1234567890" * 8]
    for f in fixed:
        lines.append("M\t" + f.hex())
    for n in [1, 53, 54, 55, 56, 57, 58, 63, 64, 65, 66, 111, 119, 120, 121, 127, 128, 129, 130, 200, 255]:
        lines.append("M\t" + LCG(0x5EED0000 + n).bytes(n).hex())
    for n in [1, 16, 55, 56, 64, 128]:
        lines.append("M\t" + (b"\x00" * n).hex())
        lines.append("M\t" + (b"\xff" * n).hex())
    lines.append("M\t" + "中文测试·多字节".encode().hex())
    lines.append("M\t" + "e\u0301\u0301".encode().hex())
    lines.append("M\t" + b"\xe4\xb8".hex())            # 截断前导
    lines.append("M\t" + b"A\x80B".hex())              # 孤立续字节
    lines.append("M\t" + b"\xc3\x28".hex())            # 非法两字节
    lines.append("M\t" + LCG(0x11223344).bytes(1024).hex())
    lines.append("M\t" + LCG(0x55667788).bytes(4096).hex())
    return lines

def build_pbkdf2_lines():
    lines = []
    def add(pw, salt, iters, dklen):
        lines.append("P\t%s\t%s\t%d\t%d" % (pw.hex(), salt.hex(), iters, dklen))
    add(b"passwd", b"salt", 1, 64)                     # RFC 7914 §11
    add(b"passwd", b"salt", 4096, 64)                  # RFC 7914 §11
    add(b"password", b"salt", 1, 32)
    add(b"password", b"salt", 2, 32)
    add(b"password", b"salt", 4096, 32)
    add(b"passwordPASSWORDpassword", b"saltSALTsaltSALTsaltSALTsaltSALTsalt", 4096, 40)
    add(b"pass\0word", b"sa\0lt", 4096, 16)
    add(b"", b"", 1, 32)
    add(b"", b"salt", 1000, 32)
    add(b"pw", b"", 7, 32)
    add(b"K" * 64, b"salt", 10, 32)
    add(b"K" * 65, b"salt", 10, 32)                    # HMAC key > 64B ⇒ 先哈希
    add(b"K" * 200, b"K" * 200, 3, 32)
    for iters in [1, 2, 3, 10, 100, 1000, 4096]:
        for dklen in [1, 16, 20, 31, 32, 33, 64, 100]:
            g = LCG(0xA5A50000 + iters * 1000 + dklen)
            add(g.bytes(8 + (dklen % 9)), g.bytes(4 + (iters % 13)), iters, dklen)
    g = LCG(0xC0FFEE)
    for i in range(12):
        add(g.bytes(i), g.bytes(16), [1, 5, 50, 500][i % 4], [16, 32, 48][i % 3])
    return lines

def build_pgauth_lines():
    """PostgreSQL `AuthenticationMD5Password` 的应答素材。
    应答 = "md5" + hex( md5( hex(md5(password || user)) || salt ) )（lib/pq conn.go）。
    盐恒 4 字节（PostgreSQL 的服务端在 AuthenticationMD5Password 里只给 4 字节）。"""
    lines = []
    cases = [
        (b"secret", b"postgres", b"\x01\x02\x03\x04"),
        (b"", b"", b"\x00\x00\x00\x00"),
        (b"p", b"u", b"\xff\xff\xff\xff"),
        (b"a" * 200, b"b" * 100, b"\xde\xad\xbe\xef"),
        ("口令中文".encode(), "用户".encode(), b"\x7f\x80\x81\x82"),
    ]
    g = LCG(0x9E3779B9)
    for i in range(15):
        cases.append((g.bytes(1 + i * 3), g.bytes(1 + i), g.bytes(4)))
    for pw, user, salt in cases:
        lines.append("A\t%s\t%s\t%s" % (pw.hex(), user.hex(), salt.hex()))
    return lines

def main():
    lines = build_md5_lines() + build_pbkdf2_lines() + build_pgauth_lines()
    out = os.path.join(HERE, "corpus.txt")
    with open(out, "w") as f:
        f.write("# M150 corpus · md5/md5_bytes + pbkdf2_sha256 + PostgreSQL md5 认证\n")
        f.write("".join(l + "\n" for l in lines))
    nm = sum(1 for l in lines if l.startswith("M\t"))
    np = sum(1 for l in lines if l.startswith("P\t"))
    na = sum(1 for l in lines if l.startswith("A\t"))
    print("corpus.txt: %d 行（md5 %d + pbkdf2 %d + pgauth %d）sha256=%s"
          % (len(lines), nm, np, na, hashlib.sha256("\n".join(lines).encode()).hexdigest()[:16]))

if __name__ == "__main__":
    main()
