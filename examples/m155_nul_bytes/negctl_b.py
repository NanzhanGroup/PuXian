#!/usr/bin/env python3
# 负控 B：`px_print_value` 退回 **printf("%s")**（= 缺陷 150 的旧行为）
#   预期：stdout 在首个 0x00 截断 ⇒ 「字节精确输出」门必红。
import io, sys, os
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
p = 'runtime/runtime.c'
s = io.open(p, encoding='utf-8').read()
old = '''    int n = 0;
    char* s = px_fmt_value_n(v, &n);
    if (n > 0) fwrite(s, 1, (size_t)n, stdout);'''
new = '''    int n = 0;
    char* s = px_fmt_value_n(v, &n);
    /* NEGCTL-B */ printf("%s", s);'''
if s.count(old) != 1:
    print("锚点不匹配（源码变了？）"); sys.exit(2)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new))
print("negctl B applied")
