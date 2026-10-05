#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M261 · 持锁可失败分配收口（一）—— **静态锚点**
# ------------------------------------------------------------
# 每条锚点都是**可判定的源码事实**（不是「看起来对」）：
#   · **锁区外**：该函数内**没有任何**分配调用落在 pthread 锁区间内
#     （按行区间配对 lock/unlock 判定；比「在第一次加锁之前」更贴契约 ——
#      两阶段的形态是「探测加锁 → 出锁备货 → 再进锁发布」，分配在第 2 次加锁**之前**、
#      却在第 1 次加锁**之后**，用「第一次加锁之前」判会**假红**）。
#   · **存在类**：复核/发布语句必须在位。
#   · **反向类**：被收口的**旧形态**必须**不再出现**（0 次）。
#
# ⚠️ 两条自伤教训（本轮实测踩到，已写进判据）：
#   ① **必须先剥注释再做内容判定** —— 反向判据查「旧形态 0 次」，而我在收口处写了
#      「此处原有 `if (!g_pin_set) g_pin_set = xcalloc(...)`」的解释性注释 ⇒ 命中自己的注释
#      ⇒ 假红。（与 M200「剥离器要只去注释、保留字面量」同族。）
#   ② **锁区间必须按行配对** —— 只看「首次加锁行号」会把合法两阶段判成违规。
#
# 用法：python3 examples/m261_lock_alloc/check_anchors.py [--root .]
# 退出码：0=全通过 · 1=有锚点不成立 · 2=用法错
# ============================================================
import argparse
import io
import os
import re
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..', 'selfhost'))
try:
    from gcroot_derive import strip_comments_strings
except Exception:
    def strip_comments_strings(s):
        return s

ALLOC_RX = re.compile(r'\b(xmalloc|xrealloc|xcalloc|xstrdup|m128_alloc)\s*\(')
LOCK_RX = re.compile(r'\bpthread_(?:mutex|rwlock|spin)_lock\s*\(\s*&?\s*([A-Za-z_]\w*)')
UNLOCK_RX = re.compile(r'\bpthread_(?:mutex|rwlock|spin)_unlock\s*\(\s*&?\s*([A-Za-z_]\w*)')


def _strip_lits(s):
    """只去掉字符串/字符字面量（保留其它字符）—— 数括号时不受字面量干扰。
    （与 negctl.py 同款：字面量里的括号会让配平永远不闭合。）"""
    out, i, n = [], 0, len(s)
    while i < n:
        c = s[i]
        if c in '\'"':
            q = c
            i += 1
            while i < n and s[i] != q:
                if s[i] == '\\':
                    i += 1
                i += 1
            i += 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def func_body(src, name):
    lines = src.split('\n')
    start = None
    acc = ''
    for i, ln in enumerate(lines):
        if start is None:
            if ';' in _strip_lits(ln) or ln.lstrip().startswith('#') or ln.lstrip().startswith('//'):
                acc = ''
                continue
            acc = (acc + ' ' + ln.strip()) if acc else ln.strip()
            m = re.match(r'^[A-Za-z_][\w\s\*]*\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*\{', acc)
            if m and m.group(1) == name:
                start = i
            elif m or acc.count('(') == 0:
                acc = ''
            continue
    if start is None:
        return None
    depth = 0
    out = []
    for ln in lines[start:]:
        out.append(ln)
        st = _strip_lits(ln)
        depth += st.count('{') - st.count('}')
        if depth <= 0:
            break
    return '\n'.join(out)


def lock_regions(body):
    """→ [(lock_lineno, unlock_lineno)]（1 基；未闭合的到末行）"""
    lines = body.split('\n')
    stack = []
    out = []
    for i, ln in enumerate(lines, 1):
        ml = LOCK_RX.search(ln)
        if ml:
            stack.append((ml.group(1), i))
        mu = UNLOCK_RX.search(ln)
        if mu:
            for k in range(len(stack) - 1, -1, -1):
                if stack[k][0] == mu.group(1):
                    out.append((stack[k][1], i))
                    del stack[k:]
                    break
    for name, ln in stack:
        out.append((ln, len(lines)))
    return out


def allocs_in_locks(body):
    """→ [(lineno, text)] 落在任一锁区间内的分配调用"""
    regs = lock_regions(body)
    out = []
    for i, ln in enumerate(body.split('\n'), 1):
        if not ALLOC_RX.search(ln):
            continue
        # 区间含两端（lock 行与 unlock 行本身不算分配行；分配若与它们同行则视为在区间内）
        for a, b in regs:
            if a <= i <= b:
                out.append((i, ln.strip()))
                break
    return out


def strip_comments_only(src):
    """只去掉 C 注释，**保留字符串字面量的内容** —— 「存在类」判据必须用它：
    `strip_comments_strings` 会把字面量内容抹成空格 ⇒ 含字面量的源码事实永远查不到
    ⇒ 假红。（M262 在 check_anchors2.py 上实测到的自伤，同款回灌到这里。）"""
    out = []
    i, n = 0, len(src)
    st = 0
    while i < n:
        c = src[i]
        nx = src[i + 1] if i + 1 < n else ''
        if st == 0:
            if c == '/' and nx == '/':
                st = 3; out.append(' '); i += 2; continue
            if c == '/' and nx == '*':
                st = 4; out.append(' '); i += 2; continue
            if c == '"':
                st = 1
            elif c == "'":
                st = 2
            out.append(c); i += 1
        elif st == 1:
            if c == '\\':
                out.append(c); out.append(nx if nx else ' '); i += 2; continue
            if c == '"':
                st = 0
            out.append(c); i += 1
        elif st == 2:
            if c == '\\':
                out.append(c); out.append(nx if nx else ' '); i += 2; continue
            if c == "'":
                st = 0
            out.append(c); i += 1
        elif st == 3:
            if c == '\n':
                st = 0; out.append('\n')
            i += 1
        else:
            if c == '*' and nx == '/':
                st = 0; out.append(' '); i += 2; continue
            out.append('\n' if c == '\n' else ' ')
            i += 1
    return ''.join(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    a = ap.parse_args()
    R = os.path.abspath(a.root)
    raw = io.open(os.path.join(R, 'runtime', 'runtime.c'), encoding='utf-8', errors='replace').read()
    # 两版文本分工：**存在类**用「只去注释」（保留字面量）；**反向类**用「去注释 + 去字面量」
    code = strip_comments_strings(raw)      # 反向判据（旧形态 0 次）
    code_nc = strip_comments_only(raw)      # 存在类判据

    ok = 0
    bad = 0

    def chk(tag, cond, detail=''):
        nonlocal ok, bad
        if cond:
            print("  ✅ %s%s" % (tag, ('  ' + detail) if detail else ''))
            ok += 1
        else:
            print("  ❌ %s%s" % (tag, ('  ' + detail) if detail else ''))
            bad += 1

    # 全仓反向判据（旧形态必须为 0 次；用**去注释**文本，避免命中解释性注释）
    rev = [
        ("A1 反向：`c->pbuf ? xrealloc(` 0 次（px_conn_pend_put 旧形态）",
         'c->pbuf ? xrealloc(', 0),
        ("A2 反向：`if (!g_pin_set) g_pin_set =` 0 次（px_pin_obj 死代码）",
         'if (!g_pin_set) g_pin_set =', 0),
        ("A3 反向：`b = xmalloc(sizeof(RateBucket));` 0 次（rate_limit 旧形态）",
         'b = xmalloc(sizeof(RateBucket));', 0),
        ("A4 反向：`g_ctab = (PxConstEnt*)xcalloc` 0 次（const_put 旧形态）",
         'g_ctab = (PxConstEnt*)xcalloc', 0),
        ("A5 反向：`g_fserve_threads = (pthread_t*)xcalloc` 0 次（fserve 旧形态）",
         'g_fserve_threads = (pthread_t*)xcalloc', 0),
    ]
    for tag, needle, want in rev:
        chk(tag, code.count(needle) == want, "实测 %d" % code.count(needle))

    # 正向存在类
    pos = [
        ("A6 fserve_ensure：锁内复核 + 发布备货",
         'g_fserve_threads = th; th = NULL;'),
        ("A7 fserve_ensure：复核失败即释放备货",
         'if (th) xfree(th);'),
        ("A8 px_conn_pend_put：有界重来（attempt < 4）",
         'for (int attempt = 0; attempt < 4; attempt++)'),
        ("A9 px_conn_pend_put：锁外备货",
         'char* grow = need_cap ? (char*)xmalloc((size_t)need_cap) : NULL;'),
        ("A10 px_pin_obj：锁内只发布（nset 转交 g_pin_set）",
         'if (!g_pin_set && nset) { g_pin_set = nset; nset = NULL; }'),
        ("A11 px_const_put：锁内复核 + 发布",
         'if (!g_ctab && ntab) { g_ctab = ntab; ntab = NULL; }'),
        ("A12 px_rate_limit_try：出锁备货",
         'long long* ntimes = (long long*)xmalloc(sizeof(long long) * (size_t)ncap);'),
        ("A13 px_rate_limit_try：进锁复核（rate_bucket_find 第二次）",
         'if (rate_bucket_find(key, &b)) {'),
    ]
    for tag, needle in pos:
        chk(tag, needle in code_nc)

    # 逐函数：**没有任何分配落在锁区间内**（本轮的核心契约）
    for fn in ('fserve_ensure', 'px_conn_pend_put', 'px_pin_obj',
               'px_const_put', 'px_rate_limit_try'):
        b = func_body(code, fn)
        if b is None:
            chk("A14 函数可定位：%s" % fn, False)
            continue
        hits = allocs_in_locks(b)
        chk("A14 %s：锁区间内**零**分配调用" % fn, len(hits) == 0,
            ("%d 处：%s" % (len(hits), hits[:2])) if hits else "")

    # A15：判定器自证 —— 用一个**已知违规**的合成体验「锁区间内分配」能报出来
    fake = ("void f(void) {\n"
            "    pthread_mutex_lock(&mu);\n"
            "    void* p = xmalloc(8);\n"
            "    pthread_mutex_unlock(&mu);\n"
            "}\n")
    chk("A15 判定器自证：合成违规体必须被报出（防判据恒绿）",
        len(allocs_in_locks(fake)) == 1)
    fake2 = ("void f(void) {\n"
             "    pthread_mutex_lock(&mu);\n"
             "    pthread_mutex_unlock(&mu);\n"
             "    void* p = xmalloc(8);\n"
             "}\n")
    chk("A16 判定器自证：锁**外**分配不得报出（防判据恒红）",
        len(allocs_in_locks(fake2)) == 0)

    print("── M261 锚点：%d 通过 / %d 失败" % (ok, bad))
    print("M261-ANCHORS-OK" if bad == 0 else "M261-ANCHORS-FAIL")
    sys.exit(0 if bad == 0 else 1)


main()
