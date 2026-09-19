#!/usr/bin/env python3
# 负控 C：**join 退回 strlen 口径**（缺陷 149 的旧行为：分隔符与 PX_STR 项都按 strlen）
#   ⇒ 语义门的内嵌 NUL 断言必红（这一道验的是「join 与 + 字节一致」这条**修复**确实生效）
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()

old1 = """static inline void bi_join_item(LXValue item, const char** pp, int* lp) {
    if (item.type == PX_STR) { *pp = item.as.obj->as.str.data; *lp = item.as.obj->as.str.len; return; }"""
new1 = """static inline void bi_join_item(LXValue item, const char** pp, int* lp) {
    if (item.type == PX_STR) { *pp = item.as.obj->as.str.data; *lp = (int)strlen(item.as.obj->as.str.data); return; }   /* NEGCTL-M154C */"""
assert s.count(old1) == 1, s.count(old1)
s = s.replace(old1, new1)

old2 = """    int sep_len = args[0].as.obj->as.str.len;
    if (args[1].type != PX_LIST && args[1].type != PX_TUPLE) px_error("R1002: join 第二参数需要 list/tuple");"""
new2 = """    int sep_len = (int)strlen(sep);   /* NEGCTL-M154C：旧口径 */
    if (args[1].type != PX_LIST && args[1].type != PX_TUPLE) px_error("R1002: join 第二参数需要 list/tuple");"""
assert s.count(old2) == 1, s.count(old2)
s = s.replace(old2, new2)

open(p, "w", encoding="utf-8").write(s)
print("negctl C applied")
