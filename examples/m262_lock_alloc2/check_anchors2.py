#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M262 · 持锁可失败分配收口（二）—— **静态锚点**
# ------------------------------------------------------------
# 三类判据：
#   R* `route_match`（两阶段：锁内只读匹配 → 段快照到栈 → **锁外**构造 params）
#   S* `bi_sse_read_line`（锁内零分配：栈快路径 + 锁外备货 + 锁内复核）
#   C* 缺陷 465（M257 容器守卫**读入口降噪**、写入口保留）
# 每条都是**可判定的源码事实**；反向判据查「旧形态 0 次」且**先剥注释**
#   （M261 的自伤教训：解释性注释会命中反向判据）。
# 用法：python3 examples/m262_lock_alloc2/check_anchors2.py [--root .]
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
    lines = body.split('\n')
    stack, out = [], []
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
    for _, ln in stack:
        out.append((ln, len(lines)))
    return out


def allocs_in_locks(body):
    regs = lock_regions(body)
    out = []
    for i, ln in enumerate(body.split('\n'), 1):
        if not ALLOC_RX.search(ln):
            continue
        for a, b in regs:
            if a <= i <= b:
                out.append((i, ln.strip()))
                break
    return out


def strip_comments_only(src):
    """只去掉 C 注释（`//` 与 `/* */`），**保留字符串/字符字面量的内容**。
    用于「存在类」判据 —— 否则含字面量的源码事实永远查不到（见本文件头的自伤教训）。"""
    out = []
    i, n = 0, len(src)
    state = 0   # 0 代码 1 双引号 2 单引号 3 行注释 4 块注释
    while i < n:
        c = src[i]
        nxt = src[i + 1] if i + 1 < n else ''
        if state == 0:
            if c == '/' and nxt == '/':
                state = 3; out.append(' '); i += 2; continue
            if c == '/' and nxt == '*':
                state = 4; out.append(' '); i += 2; continue
            if c == '"':
                state = 1
            elif c == "'":
                state = 2
            out.append(c); i += 1
        elif state == 1:
            if c == '\\':
                out.append(c); out.append(nxt if nxt else ' '); i += 2; continue
            if c == '"':
                state = 0
            out.append(c); i += 1
        elif state == 2:
            if c == '\\':
                out.append(c); out.append(nxt if nxt else ' '); i += 2; continue
            if c == "'":
                state = 0
            out.append(c); i += 1
        elif state == 3:
            if c == '\n':
                state = 0; out.append('\n')
            i += 1
        else:
            if c == '*' and nxt == '/':
                state = 0; out.append(' '); i += 2; continue
            out.append('\n' if c == '\n' else ' ')
            i += 1
    return ''.join(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    a = ap.parse_args()
    R = os.path.abspath(a.root)
    rc_raw = io.open(os.path.join(R, 'runtime', 'runtime.c'), encoding='utf-8',
                     errors='replace').read()
    rt_raw = io.open(os.path.join(R, 'runtime', 'runtime_route.c'), encoding='utf-8',
                     errors='replace').read()
    # 两版文本分工（见文件头）：存在类用「只去注释」；反向类用「去注释 + 去字面量」
    rc = strip_comments_only(rc_raw)
    rt = strip_comments_only(rt_raw)
    rc_ns = strip_comments_strings(rc_raw)
    rt_ns = strip_comments_strings(rt_raw)

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

    # ---------------- R: route_match ----------------
    b = func_body(rt, 'route_match')
    chk("R0 函数可定位：route_match（runtime_route.c）", b is not None)
    if b:
        hits = allocs_in_locks(b)
        chk("R1 route_match 锁区间内**零**分配调用", len(hits) == 0,
            ("%d 处：%s" % (len(hits), hits[:2])) if hits else "")
        chk("R2 匹配段**快照到栈**在位",
            'memcpy(snap, g_routes[i].segs, sizeof(PxRouteSeg) * (size_t)nsnap);' in b)
        chk("R3 params 构造在**锁外**（push 在 route 锁 unlock 之后）",
            (lambda: (lambda lines: (lambda iu: (lambda ip: ip is not None and iu is not None and ip > iu)(
                next((k for k, l in enumerate(lines) if 'px_root_push_keep(params)' in l), None)))( 
                next((k for k, l in enumerate(lines) if 'pthread_mutex_unlock(&g_route_mu)' in l), None)))(
                b.split('\n')))())
        chk("R4 push/pop 成对（各 1 次）",
            b.count('px_root_push_keep(params)') == 1 and b.count('px_root_pop();') == 1)
    chk("R5 反向：旧形态「锁内 px_dict_set(params, seg->seg, …)」全仓 0 次",
        rc_ns.count('px_dict_set(params, seg->seg, px_str(parts[pi]));') == 0
        and rt_ns.count('px_dict_set(params, seg->seg, px_str(parts[pi]));') == 0)
    chk("R6 反向：旧形态「循环内 LXValue params = px_dict();」0 次",
        rt_ns.count('LXValue params = px_dict();') == 1 and rt_ns.count('        LXValue params = px_dict();') == 0)

    # ---------------- S: bi_sse_read_line ----------------
    b2 = func_body(rc, 'bi_sse_read_line')
    chk("S0 函数可定位：bi_sse_read_line", b2 is not None)
    if b2:
        hits = allocs_in_locks(b2)
        chk("S1 bi_sse_read_line 锁区间内**零**分配调用", len(hits) == 0,
            ("%d 处：%s" % (len(hits), hits[:2])) if hits else "")
        chk("S2 短行**栈缓冲**在位（char small[SSE_LINE_SMALL]）",
            'char small[SSE_LINE_SMALL];' in b2 and '#define SSE_LINE_SMALL' in rc)
        chk("S3 备货尺寸取 pending **容量上界**（pend_cap+1）",
            b2.count('(size_t)g_sse_clients[') >= 2 and '+ 1;' in b2)
        chk("S4 有界轮数（round < 8）在位", 'for (int round = 0; round < 8; round++)' in b2)
    bh = func_body(rc, 'sse_line_take')
    chk("S5 sse_line_take 可定位且**体内零分配**",
        bh is not None and len(allocs_in_locks(bh)) == 0 and not ALLOC_RX.search(bh))
    bg = func_body(rc, 'sse_line_grow')
    chk("S6 sse_line_grow 可定位且**含**分配（它是锁外增长入口）",
        bg is not None and ALLOC_RX.search(bg) is not None)
    chk("S7 反向：旧形态 `char* tmp = xmalloc((size_t)ll + 1);` 0 次",
        rc_ns.count('char* tmp = xmalloc((size_t)ll + 1);') == 0)

    # ---------------- C: 缺陷 465 ----------------
    chk("C1 px_ctr_livechk_on 定义在位（读入口诊断开关）",
        'static int px_ctr_livechk_on(void)' in rc)
    chk("C2 读入口**条件化**：px_dict_get 仅在诊断档响亮",
        'if (px_ctr_livechk_on()) px_ctr_guard_fail(o, PX_DICT, "px_dict_get");' in rc)
    chk("C3 写入口**保留**（px_dict_set / px_list_push 仍无条件响亮）",
        'px_ctr_guard_fail(o, PX_DICT, "px_dict_set")' in rc
        and 'px_ctr_guard_fail(o, PX_LIST, "px_list_push")' in rc)
    chk("C4 反向：旧的无条件读守卫 0 次",
        rc_ns.count('{ px_ctr_guard_fail(o, PX_DICT, "px_dict_get"); return px_null(); }') == 0)

    # ---------------- 判定器自证 ----------------
    fake = "void f(void) {\n    pthread_mutex_lock(&mu);\n    void* p = xmalloc(8);\n    pthread_mutex_unlock(&mu);\n}\n"
    fake2 = "void f(void) {\n    pthread_mutex_lock(&mu);\n    pthread_mutex_unlock(&mu);\n    void* p = xmalloc(8);\n}\n"
    chk("X1 判定器自证：合成违规体必被报出（防恒绿）", len(allocs_in_locks(fake)) == 1)
    chk("X2 判定器自证：锁外分配不得报出（防恒红）", len(allocs_in_locks(fake2)) == 0)

    print("── M262 锚点：%d 通过 / %d 失败" % (ok, bad))
    print("M262-ANCHORS-OK" if bad == 0 else "M262-ANCHORS-FAIL")
    sys.exit(0 if bad == 0 else 1)


main()
