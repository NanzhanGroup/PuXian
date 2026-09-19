#!/usr/bin/env python3
# 负控 B：**关掉小整数文本池**（`str(int)` 一律走域外「现造」路径）
#   ⇒ 预算门的整数相位分配数回升 ⇒ 必须判红
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()

old = """static LXValue px_str_int_pool(int64_t v) {
    if (v < PX_INTSTR_LO || v > PX_INTSTR_HI) {"""
new = """static LXValue px_str_int_pool(int64_t v) {
    if (1) {   /* NEGCTL-M154B：不走小整数文本池 */"""
assert s.count(old) == 1, s.count(old)
s = s.replace(old, new)

open(p, "w", encoding="utf-8").write(s)
print("negctl B applied")
