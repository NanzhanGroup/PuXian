#!/usr/bin/env python3
# 负控 A：ASCII 单字符串表返回**相邻字节**的字符串 ⇒ pool.px 语义门必须判红
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()
old = "        buf[0] = (char)c; buf[1] = 0;"
new = "        buf[0] = (char)(c + 1); buf[1] = 0;   /* NEGCTL-M153A */"
assert s.count(old) == 1, s.count(old)
open(p, "w", encoding="utf-8").write(s.replace(old, new))
print("negctl A applied")
