#!/usr/bin/env python3
# 负控 C：空串单例退回「每次新建」 ⇒ 预算门必须判红
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()
old = """static LXValue px_empty_str_get(void) {
    if (__atomic_load_n(&g_empty_ready, __ATOMIC_ACQUIRE)) return g_empty_str;"""
new = """static LXValue px_empty_str_get(void) {
    return px_str_len_raw("", 0);   /* NEGCTL-M153C：不单例 */
    if (__atomic_load_n(&g_empty_ready, __ATOMIC_ACQUIRE)) return g_empty_str;"""
assert s.count(old) == 1, s.count(old)
open(p, "w", encoding="utf-8").write(s.replace(old, new))
print("negctl C applied")
