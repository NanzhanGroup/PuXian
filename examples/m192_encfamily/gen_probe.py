#!/usr/bin/env python3
# 生成 M192 动态探针（加密 / 压缩 / 归档族）：
#   每例先 print("before") 证运行期，再触发目标错误。
# ── 三轨共用（EXPECT.tsv / EXPECT_OK.tsv）        ：aes / rsa / ed25519 / zip
# ── 解释轨专属（EXPECT_INTERP.tsv / EXPECT_OK_INTERP.tsv）：zlib
#    ⚠️ 为什么 zlib 只测解释轨：**编译轨默认按引用集自动裁剪模块**（M86-S2），而本仓没有
#       可供 `import` 的 zlib stdlib（zlib 是纯 native 组，`runtime/native_mod_map.txt`
#       里只有 `zlib_* = zlib` 三条映射）。实测（M192）：裸 `px build` 与 `px build --full`
#       **均**报 `R1001: 未定义变量: 'zlib_crc32'` ⇒ 属「编译轨无法启用该模块」的独立问题，
#       与错误码面正交 ⇒ 登记为**观察项 218**；本门 zlib 面由静态判据 + 解释轨动态判据覆盖。
import os
D = os.path.join(os.path.dirname(os.path.abspath(__file__)), "probe")
os.makedirs(D, exist_ok=True)
KEY = "0123456789abcdef"

CASES = {   # ── 三轨共用 · 错例 ──
    "e1_aes_first_type": (['print("before")',
                           'print(aes_encrypt(1, "%s", "%s"))' % (KEY, KEY)],
                          "R1002", "期望字符串，实际是 int"),
    "e2_aes_arity": (['print("before")', 'print(aes_encrypt("x"))'],
                     "R1002", "aes_encrypt 需要 3 个参数: (data, key, iv)"),
    "e3_aes_keylen": (['print("before")', 'print(aes_encrypt("data", "0123", "%s"))' % KEY],
                      "R1002", "AES 密钥长度须为 16/24/32 字节（128/192/256 位），实际 4"),
    "e4_aes_iv_empty": (['print("before")', 'print(aes_gcm_encrypt("data", "%s", ""))' % KEY],
                        "R1002", "GCM 模式 IV 不能为空"),
    "e7_rsa_bits": (['print("before")', 'print(rsa_gen_key(100))'],
                    "R1002", "rsa_gen_key 的 bits 必须在 512..4096"),
    "e8_ed25519_arity": (['print("before")', 'print(ed25519_keygen(1))'],
                         "R1002", "ed25519_keygen 不需要参数"),
    "e9_zip_arity": (['print("before")', 'print(zip_pack(1))'],   # 只给 1 个实参 ⇒ 触发个数守卫
                     "R1002", "zip_pack 需要 2 个参数: (files, out_path)"),
    "e10_zip_shape": (['print("before")', 'print(zip_pack(["a"], "/tmp/m192_x.zip"))'],
                      "R1002", "zip_pack 第一个参数须为 dict{路径→内容}"),
}
INTERP_CASES = {   # ── 解释轨专属 · 错例（见文件头：编译轨按引用集裁剪 zlib）──
    "e5_zlib_level": (['print("before")', 'print(zlib_compress("abc", 99))'],
                      "R1002", "zlib_compress 的 level 需在 0..9"),
    "e6_zlib_type": (['print("before")', 'print(zlib_crc32(1))'],
                     "R1002", "参数需要 str/bytes"),
}
OK_CASES = {   # ── 三轨共用 · 合法侧（证明收口没把正常路径改坏）──
    "o1_aes_roundtrip": (['var c = aes_encrypt("hello", "%s", "%s")' % (KEY, KEY),
                          'print(aes_decrypt(c, "%s", "%s"))' % (KEY, KEY)], "hello"),
}
OK_INTERP_CASES = {   # ── 解释轨专属 · 合法侧 ──
    # ⚠️ `zlib_uncompress` 返回 **bytes**，`print(bytes)` 的形态是 `<bytes 12>` ⇒
    #    判据取**长度**（"roundtrip-ok" = 12 字节）而不是内容子串（实测踩过）。
    "o2_zlib_roundtrip": (['print(len(zlib_uncompress(zlib_compress("roundtrip-ok", 6))))'], "^12$"),
}

for k, (src, _c, _b) in list(CASES.items()) + list(INTERP_CASES.items()):
    open(os.path.join(D, k + ".px"), "w", encoding="utf-8").write("\n".join(src) + "\n")
for k, (src, _w) in list(OK_CASES.items()) + list(OK_INTERP_CASES.items()):
    open(os.path.join(D, k + ".px"), "w", encoding="utf-8").write("\n".join(src) + "\n")


def tsv(name, rows):
    with open(os.path.join(D, name), "w", encoding="utf-8") as f:
        for r in rows:
            f.write("\t".join(r) + "\n")


tsv("EXPECT.tsv", [[k, c, b] for k, (_s, c, b) in CASES.items()])
tsv("EXPECT_INTERP.tsv", [[k, c, b] for k, (_s, c, b) in INTERP_CASES.items()])
tsv("EXPECT_OK.tsv", [[k, w] for k, (_s, w) in OK_CASES.items()])
tsv("EXPECT_OK_INTERP.tsv", [[k, w] for k, (_s, w) in OK_INTERP_CASES.items()])
print("三轨错例 %d · 三轨合法 %d · 解释轨错例 %d · 解释轨合法 %d -> %s"
      % (len(CASES), len(OK_CASES), len(INTERP_CASES), len(OK_INTERP_CASES), D))
