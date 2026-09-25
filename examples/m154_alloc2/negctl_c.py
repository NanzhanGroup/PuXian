#!/usr/bin/env python3
# 负控 C：**join 退回 strlen 口径**（缺陷 149 的旧行为：分隔符与 PX_STR 项都按 strlen）
#   ⇒ 语义门的内嵌 NUL 断言必红（这一道验的是「join 与 + 字节一致」这条**修复**确实生效）
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()

old1 = """static inline void bi_join_item(LXValue item, const char** pp, int* lp) {
    *pp = px_tostr_n(item, lp);
}"""
new1 = """static inline void bi_join_item(LXValue item, const char** pp, int* lp) {
    *pp = px_tostr_n(item, lp);
    *lp = (int)strlen(*pp);   /* NEGCTL-M154C：join 项退回 strlen 口径（缺陷 149 旧行为） */
}"""
assert s.count(old1) == 1, "old1 命中 %d 次（应 1）" % s.count(old1)
s = s.replace(old1, new1)

old2 = """    int sep_len = args[0].as.obj->as.str.len;
    // M178：可迭代实参统一（list/tuple/生成器）
    LXValue xs;
    px_root_push();
    if (!px_as_list(args[1], &xs)) { px_root_pop(); px_error("R1002: join 第二参数需要 list/tuple/生成器/字符串，实际是 %s", px_type_name(args[1])); }"""
new2 = """    int sep_len = (int)strlen(sep);   /* NEGCTL-M154C：旧口径 */
    // M178：可迭代实参统一（list/tuple/生成器）
    LXValue xs;
    px_root_push();
    if (!px_as_list(args[1], &xs)) { px_root_pop(); px_error("R1002: join 第二参数需要 list/tuple/生成器/字符串，实际是 %s", px_type_name(args[1])); }"""
assert s.count(old2) == 1, s.count(old2)
s = s.replace(old2, new2)

open(p, "w", encoding="utf-8").write(s)
print("negctl C applied")
