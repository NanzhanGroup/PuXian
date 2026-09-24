#!/usr/bin/env python3
# ============================================================
# M200 静态判据 [S11]：`px_ffi_register` 的注册名 ⇄ **全局发布**（缺陷 240）
# ------------------------------------------------------------
# 为什么需要：编译轨把 `extern def f(...)` 编译成 **GETG "f"**（不像解释轨有
#   `iexpr.px` 的 ffi 双表兜底）⇒ 不发布 ⇒ `R1001 未定义变量`（官方 registry/zlib 曾因此
#   在编译产物里完全不可用）。本扫描器把"发布"变成**可判定**的三条：
#     ① 注册名集合非空（下限，防"扫不到"= 假绿）；
#     ② `px_ffi_publish_globals` 定义存在，且其循环**遍历整表**（`i < g_ffi_n`）；
#     ③ 该函数在 `px_register_builtins` 内被调用**恰好一次**，且在建表窗口内（`g_gc_frozen = 0` 之前）。
# 输出：结论行（verify.sh 判据）+ 名字数/已存在全局数（供人核）
# ============================================================
import argparse, os, re, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
a = ap.parse_args()
os.chdir(a.root)


def strip_lit(src):
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c == '/' and i + 1 < n and src[i + 1] == '*':
            j = src.find('*/', i + 2); seg = src[i:(n if j < 0 else j + 2)]
            out.append('\n' * seg.count('\n')); i = n if j < 0 else j + 2
        elif c == '/' and i + 1 < n and src[i + 1] == '/':
            j = src.find('\n', i); out.append(' '); i = n if j < 0 else j
        elif c == '"':
            j = i + 1
            while j < n:
                if src[j] == '\\': j += 2; continue
                if src[j] == '"': break
                j += 1
            out.append('""'); i = j + 1
        else:
            out.append(c); i += 1
    return ''.join(out)


def brace_end(s, start):
    d, i = 0, start
    while i < len(s):
        if s[i] == '{': d += 1
        elif s[i] == '}':
            d -= 1
            if d == 0: return i
        i += 1
    return len(s) - 1


def strip_comments_only(src):
    """只去注释、**保留字符串字面量** —— 取注册名要用它
    （⚠️ 首版用 strip_lit（连字面量一起抹）⇒ `px_ffi_register(\"zlib_crc32\"` 变成
     `px_ffi_register(\"\"` ⇒ 名字数扫成 **0**，被"下限 150"判据当场抓住，修对后 200+）"""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c == '/' and i + 1 < n and src[i + 1] == '*':
            j = src.find('*/', i + 2); seg = src[i:(n if j < 0 else j + 2)]
            out.append('\n' * seg.count('\n')); i = n if j < 0 else j + 2
        elif c == '/' and i + 1 < n and src[i + 1] == '/':
            j = src.find('\n', i); out.append(' '); i = n if j < 0 else j
        elif c == '"':
            j = i + 1
            while j < n:
                if src[j] == '\\': j += 2; continue
                if src[j] == '"': break
                j += 1
            out.append(src[i:j + 1]); i = j + 1
        else:
            out.append(c); i += 1
    return ''.join(out)


import glob
ffi_names, where = [], {}
for path in sorted(glob.glob('runtime/*.c')):
    clean = strip_comments_only(open(path, encoding='utf-8', errors='replace').read())
    for m in re.finditer(r'px_ffi_register\(\s*"([^"]+)"', clean):
        ffi_names.append(m.group(1)); where[m.group(1)] = os.path.basename(path)
globals_set = set()
for path in sorted(glob.glob('runtime/*.c')):
    clean = strip_comments_only(open(path, encoding='utf-8', errors='replace').read())
    for m in re.finditer(r'px_set_global\(\s*"([^"]+)"', clean):
        globals_set.add(m.group(1))

bad = 0
print('=== [S11] FFI 注册名 ⇄ 全局发布 ===')
n_ffi = len(ffi_names)
print(f'px_ffi_register 名字数     : {n_ffi}')
if n_ffi < 80:
    print('  ✗ 注册名少于下限 80（扫描器可能失效 —— 宁可判红也不静默）'); bad += 1
print(f'其中也走 px_set_global 的  : {len(set(ffi_names) & globals_set)}')
only = sorted(set(ffi_names) - globals_set)
print(f'仅 FFI 表（必须靠发布）    : {len(only)}  → {", ".join(only)}')
# 判据核心：**FFI-only 的名字**正是"不发布就在编译轨不可解析"的那批（本轮 = zlib 族 3 个）
if not only:
    print('  ℹ️ 本轮没有 FFI-only 名（全部也走 px_set_global）—— 判据仍成立，只是缺陷面为 0')

# ② 定义 + 遍历整表
ffi_c = open('runtime/runtime_ffi.c', encoding='utf-8').read()
m = re.search(r'void\s+px_ffi_publish_globals\s*\(\s*void\s*\)\s*\{', ffi_c)
if not m:
    print('  ✗ runtime_ffi.c 缺 px_ffi_publish_globals 定义'); bad += 1
else:
    body = ffi_c[m.end() - 1: brace_end(ffi_c, m.end() - 1)]
    if re.search(r'for\s*\(\s*i\s*=\s*0\s*;\s*i\s*<\s*g_ffi_n\s*;', body) and 'px_set_global' in body:
        print('  ✓ 定义存在且**遍历整表**（i < g_ffi_n）+ 逐条 px_set_global')
    else:
        print('  ✗ 发布循环未遍历整表（`i < g_ffi_n` 缺失）⇒ 未发布的名字在编译轨不可解析'); bad += 1

# ③ 调用点：恰好一处，且在 px_register_builtins 内、g_gc_frozen = 0 之前
calls = []
for path in sorted(glob.glob('runtime/*.c')):
    clean = strip_lit(open(path, encoding='utf-8', errors='replace').read())
    for cm in re.finditer(r'px_ffi_publish_globals\s*\(\s*\)\s*;', clean):
        calls.append((os.path.basename(path), cm.start(), clean))
print(f'发布调用点数               : {len(calls)}')
if len(calls) != 1:
    print('  ✗ 发布调用点必须**恰好一处**（0 = 从未发布；>1 = 语义不清）'); bad += 1
else:
    f, pos, clean = calls[0]
    mm = re.search(r'void\s+px_register_builtins\s*\(\s*void\s*\)\s*\{', clean)
    ok_in = mm and mm.start() < pos < brace_end(clean, mm.end() - 1)
    tail = clean[pos:pos + 400]
    ok_before = 'g_gc_frozen = 0' in tail
    print(f'  · 位置：{f} · 在 px_register_builtins 内 = {bool(ok_in)} · 在建表窗口内（先于 g_gc_frozen = 0）= {ok_before}')
    if not ok_in:
        print('  ✗ 发布点不在建表函数内（可能在 FFI 注册**之前**执行 ⇒ 发布空表）'); bad += 1
    if not ok_before:
        print('  ✗ 发布点在建表窗口之外（M170 缺陷 189 的 GC 冻结窗口：窗口外发布可能被误回收）'); bad += 1

print(f'结论：{"不一致 0" if bad == 0 else "不一致 %d" % bad}')
sys.exit(1 if bad else 0)
