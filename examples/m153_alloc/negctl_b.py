#!/usr/bin/env python3
# 负控 B：**完全不池化** —— 常量池直接新建、单字符串表直接新建 ⇒ 预算门必须判红
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()

old1 = """    LXObject* o = px_const_get(s);
    if (o) { LXValue v; v.type = PX_STR; v.as.obj = o; return v; }
    LXValue v = px_str_len_raw(s, (int)strlen(s));
    px_pin_obj(v.as.obj);
    px_const_put(s, v.as.obj);
    return v;"""
new1 = """    LXValue v = px_str_len_raw(s, (int)strlen(s));   /* NEGCTL-M153B：不查/不填常量池 */
    return v;"""
assert s.count(old1) == 1, s.count(old1)
s = s.replace(old1, new1)

old2 = """    if (!__atomic_load_n(&g_ch1_init[c], __ATOMIC_ACQUIRE)) {
        char buf[2];
        buf[0] = (char)c; buf[1] = 0;
        LXValue v = px_str_len_raw(buf, 1);          // 单字节原始构造（含 NUL 字符）
        px_pin_obj(v.as.obj);
        g_ch1[c] = v;
        __atomic_store_n(&g_ch1_init[c], 1, __ATOMIC_RELEASE);
    }
    return g_ch1[c];"""
new2 = """    char buf[2];                                     /* NEGCTL-M153B：不走单字符表 */
    buf[0] = (char)c; buf[1] = 0;
    return px_str_len_raw(buf, 1);"""
assert s.count(old2) == 1, s.count(old2)
s = s.replace(old2, new2)

open(p, "w", encoding="utf-8").write(s)
print("negctl B applied")
