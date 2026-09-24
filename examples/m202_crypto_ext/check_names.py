#!/usr/bin/env python3
# M202 门静态自查之一：**内置名册**（tools/lint_core.px）是否已重跑
#   名册载体是**一个 split("…") 字面量**（空格分隔）⇒ 不能用 `grep "\"name\""` 判
#   （M202 实测踩过：门因此假红一次）。这里显式解析那份字面量再判成员。
import re, sys, os

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/data/code/puxian"
NEW = ["base32_encode", "base32_decode", "base32_to_bytes", "bytes_base32",
       "hmac_sha1", "hmac_sha1_bytes"]

s = open(os.path.join(ROOT, "tools/lint_core.px"), encoding="utf-8").read()
m = re.search(r'const BUILTIN_NAMES = split\("(.*?)"\s*,', s, re.S)
if not m:
    print("❌ 找不到 BUILTIN_NAMES 的 split 字面量（名册形态变了）")
    sys.exit(2)
names = set(m.group(1).split())
missing = [n for n in NEW if n not in names]
print(f"名册成员 {len(names)} · 新名缺 {len(missing)}：{missing}")
sys.exit(1 if missing else 0)
