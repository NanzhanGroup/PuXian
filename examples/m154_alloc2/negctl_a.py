#!/usr/bin/env python3
# 负控 A：**关掉多字节 rune 短串池**（`s[i]` 的多字节分支退回每次新建）
#   ⇒ 预算门的 rune 相位分配数成量级回升 ⇒ 必须判红
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()

old = """static LXValue px_rune_pool(const char* buf, int clen) {
    if (clen < 2 || clen > 4) return px_str_len(buf, clen);   // 口径之外的长度：普通构造"""
new = """static LXValue px_rune_pool(const char* buf, int clen) {
    return px_str_len(buf, clen);   /* NEGCTL-M154A：不走 rune 池 */
    if (clen < 2 || clen > 4) return px_str_len(buf, clen);   // 口径之外的长度：普通构造"""
assert s.count(old) == 1, s.count(old)
s = s.replace(old, new)

open(p, "w", encoding="utf-8").write(s)
print("negctl A applied")
