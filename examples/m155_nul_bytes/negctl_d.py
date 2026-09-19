#!/usr/bin/env python3
# 负控 D：`vm_loadk` 的 PXK_STR 分支**忽略 K 项显式长度**（= 修复前的旧口径）
#   预期：VM 轨（用户面默认轨）的含 NUL 常量又被 strlen 截断 ⇒ VM 轨语义门必红。
import io, sys, os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
p = 'runtime/vm.c'
s = io.open(p, encoding='utf-8').read()
old = '        if (k->i > 0) return px_str_const_n(s, (int)k->i);  // M155：显式字节长（含内嵌 NUL）'
new = '        /* NEGCTL-D */ (void)0;'
if s.count(old) != 1:
    print("锚点不匹配（源码变了？）"); sys.exit(2)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new))
print("negctl D applied")
