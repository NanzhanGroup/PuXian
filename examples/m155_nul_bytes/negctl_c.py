#!/usr/bin/env python3
# 负控 C：`PX_STR_LIT` 的长度**多算 1**（`sizeof(lit)`，含 C 串结尾 NUL）
#   预期：常量尾部多一个 NUL ⇒ `len("a\u{0}b") == 4` 之类 ⇒ 语义门必红。
#   这条同时证明「长度判据真的在生效」（差一个字节也判得出来）。
import io, sys, os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
p = 'runtime/runtime.h'
s = io.open(p, encoding='utf-8').read()
old = '#define PX_STR_LIT(lit) px_str_len(lit, (int)sizeof(lit) - 1)'
new = '#define PX_STR_LIT(lit) px_str_len(lit, (int)sizeof(lit))   /* NEGCTL-C */'
if s.count(old) != 1:
    print("锚点不匹配（源码变了？）"); sys.exit(2)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new))
print("negctl C applied")
