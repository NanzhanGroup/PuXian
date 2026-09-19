#!/usr/bin/env python3
# 负控 A：`PX_STR_LIT` 的长度退回 **strlen**（= 修复前的 C 串口径）
#   预期：C 轨含 NUL 的常量在首个 0x00 截断 ⇒ 语义门必红。
#   注意：只改 runtime.h（rt_key 含 runtime.h ⇒ 缓存自动重建）。
import io, sys, os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
p = 'runtime/runtime.h'
s = io.open(p, encoding='utf-8').read()
old = '#define PX_STR_LIT(lit) px_str_len(lit, (int)sizeof(lit) - 1)'
new = '#define PX_STR_LIT(lit) px_str_len(lit, (int)strlen(lit))   /* NEGCTL-A */'
if s.count(old) != 1:
    print("锚点不匹配（源码变了？）"); sys.exit(2)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new))
print("negctl A applied")
