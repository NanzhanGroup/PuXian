#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M263 · 「响应串味」的**静态可达性审查**（`pbuf` / `fd` 生命周期）
# ------------------------------------------------------------
# 串味最可信的机制：连接级「余留字节」缓冲（`PxConnCtx.pbuf`）在 **fd 被复用** 时没被清掉
#   ⇒ 新连接的请求流前面被塞进**上一个连接**的请求 ⇒ 服务端按旧请求应答 ⇒ **答非所问**。
#   而 `px_evc_acquire` 只在 `c->fd != fd` 时才清 pbuf（M144 缺陷 124 的有意设计：
#   同 fd 的**重新登记**属正常续跑，清掉会丢管道化请求）⇒ **保护成立的前提是
#   「连接收尾时 `c->fd` 必须被重置为 -1」**。
# 本审查就是查这个前提：**每一处 `xfree(c->pbuf)` 所在的函数里，必须同时有 `c->fd = -1`**。
#   若某处只释放 pbuf 而不重置 fd ⇒ fd 复用时会被当成「同一连接」⇒ 串味可达。
# 用法：python3 examples/m263_resp_crosstalk/check_lifecycle.py [--root .]
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

FN_RX = re.compile(r'^[A-Za-z_][\w\s\*]*\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*\{')
CTRL_KW = ('if', 'while', 'for', 'switch', 'else', 'do', 'return', 'sizeof', 'catch')


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


def funcs_of(src):
    """→ [(名字, 起始行, 结束行)]（1 基，含签名行）。判据分两步，**互相独立**：
      ① **括号配平定块**（顶层 depth 0→1→0）—— 不依赖签名写法；
      ② 从块起始行**回扫**最多 4 行取签名名（跨行签名也能取到）。
    ⚠️ 三轮自伤（都实测过）：
      ① 「累积签名直到 `{`」的写法**不重置 pending** ⇒ 一旦某行不含 `{`，pending
         无限增长、之后再也匹配不上 ⇒ 真仓库扫到 **0 个函数**（全判「?函数」）；
      ② 用「行内有无 `;`」过滤签名 ⇒ 漏掉单行函数体（`{ return 0; }`）；
      ③ 不排控制关键字 ⇒ `while (cond) {` 被当函数起点、并把**整个块**跳过去。
    """
    lines = src.split('\n')
    blocks = []
    depth = 0
    bstart = None
    for i, ln in enumerate(lines):
        st = _strip_lits(ln)
        o = st.count('{')
        c = st.count('}')
        if depth == 0 and o > 0:
            bstart = i
        depth += o - c
        if bstart is not None and depth <= 0:
            blocks.append((bstart, i))
            bstart = None
    out = []
    for bs, be in blocks:
        name = '?'
        for k in range(bs, max(-1, bs - 4), -1):
            m = FN_RX.match(lines[k].strip())
            if m and m.group(1) not in CTRL_KW:
                name = m.group(1)
                break
        out.append((name, bs + 1, be + 1))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    a = ap.parse_args()
    R = os.path.abspath(a.root)
    raw = io.open(os.path.join(R, 'runtime', 'runtime.c'), encoding='utf-8',
                  errors='replace').read()
    src = strip_comments_strings(raw)
    lines = src.split('\n')
    blocks = funcs_of(src)

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

    def owner(ln):
        for name, s0, e0 in blocks:
            if s0 <= ln <= e0:
                return name, s0, e0
        return None, None, None

    def facts(h):
        nm, s0, e0 = owner(h)
        if nm is None:
            return None
        blk = '\n'.join(lines[s0 - 1:e0])
        nxt = ''
        for k in range(h, min(e0, h + 3)):
            if lines[k].strip():
                nxt = lines[k].strip()
                break
        # ⚠️ 自伤：原先用 `'= NULL' not in nxt` 判「非丢弃」，而 pend_put 的下一行是
        #   `c->pbuf = grow; c->pbuf_cap = need_cap; grow = NULL;` —— 末尾的 `grow = NULL`
        #   让子串判据把**替换**误读成**丢弃**。⇒ 必须**解析赋值右值**，不能用子串。
        m = re.search(r'c->pbuf\s*=\s*([^;]*)', nxt)
        pv = m.group(1).strip() if m else ''
        return {
            'name': nm, 'blk': blk, 'nxt': nxt,
            'replace': bool(m) and pv != 'NULL',
            'fd_reset': 'c->fd = -1' in blk,
            'takeover': 'c->fd = fd;' in blk,
            'close': 'close(fd)' in blk,
            'pendclear': 'http_pend_clear' in blk,
        }

    hits = [i + 1 for i, l in enumerate(lines) if 'xfree(c->pbuf)' in l]
    chk("L1 找到 `xfree(c->pbuf)` 释放点", len(hits) >= 3, "共 %d 处" % len(hits))
    F = [(h, facts(h)) for h in hits]
    chk("L1b 每处都能定位到所属函数块（防定界失效）",
        all(f is not None for _, f in F),
        ("未定位：%s" % [h for h, f in F if f is None]) if any(f is None for _, f in F) else "")

    # L2 丢弃型（非「换缓冲」）必须重置 fd 或走「接管」路径
    badf = []
    for h, f in F:
        if f is None or f['replace']:
            continue
        if not (f['fd_reset'] or f['takeover']):
            badf.append("%s@%d" % (f['name'], h))
    chk("L2 每个**丢弃型**释放点所在块都重置 `c->fd = -1`（或走 acquire 接管路径）",
        not badf, ("缺：%s" % badf) if badf else "")

    # L3 丢弃型 **且会 close(fd)** ⇒ 必须清挂起 handler 表（否则 fd 复用可被旧项串扰）
    badp = []
    for h, f in F:
        if f is None or f['replace']:
            continue
        if f['close'] and not f['pendclear']:
            badp.append("%s@%d" % (f['name'], h))
    chk("L3 关闭路径必须清挂起 handler 表（`http_pend_clear`）—— 缺陷 466 的判据",
        not badp, ("缺 http_pend_clear：%s" % badp) if badp else "")

    # L4 替换型必须紧跟「赋新指针」（证明判据没把「丢弃」误读成「替换」）
    badr = []
    for h, f in F:
        if f is None:
            continue
        if f['replace'] and not ('c->pbuf =' in f['nxt']):
            badr.append("%s@%d" % (f['name'], h))
    chk("L4 替换型判定自洽（紧随 `c->pbuf =` 赋值）", not badr, str(badr))

    # L5 反向：`px_evc_acquire` 保留「同 fd 续跑」判据（M144 缺陷 124 的有意设计，不能顺手删）
    chk("L5 `px_evc_acquire` 保留「同 fd 续跑」判据（c->fd != fd 才清 pbuf）",
        'if (c->fd != fd) {' in src)

    # L6 Linux `px_evc_close` 四件事齐备
    b6 = ''
    for nm, s0, e0 in blocks:
        if nm == 'px_evc_close':
            b6 = '\n'.join(lines[s0 - 1:e0])
            break
    chk("L6 Linux `px_evc_close` 四件事齐备（清 pbuf · 重置 fd · http_pend_clear · close）",
        'xfree(c->pbuf)' in b6 and 'c->fd = -1' in b6
        and 'http_pend_clear(fd)' in b6 and 'close(fd)' in b6)

    # ---- 判定器自证（合成体，独立于真仓库）----
    fake = ("static void f(int fd) {\n"
            "    PxConnCtx* c = g(fd);\n"
            "    if (c->pbuf) { xfree(c->pbuf); c->pbuf = NULL; }\n"
            "    close(fd);\n"
            "}\n")
    fl = fake.split('\n')
    fb = [i + 1 for i, l in enumerate(fl) if 'xfree(c->pbuf)' in l]
    caught = False
    for h in fb:
        for nm, s0, e0 in funcs_of(fake):
            if s0 <= h <= e0:
                blk = '\n'.join(fl[s0 - 1:e0])
                if 'close(fd)' in blk and 'http_pend_clear' not in blk:
                    caught = True
    chk("X1 判定器自证：合成「close 前不清挂起表」必被报出（防恒绿）", caught)
    good = ("static void g(int fd) {\n"
            "    PxConnCtx* c = h(fd);\n"
            "    if (c->pbuf) { xfree(c->pbuf); c->pbuf = NULL; }\n"
            "    c->fd = -1;\n"
            "    http_pend_clear(fd);\n"
            "    close(fd);\n"
            "}\n")
    gl = good.split('\n')
    caught2 = False
    for i, l in enumerate(gl):
        if 'xfree(c->pbuf)' in l:
            for nm, s0, e0 in funcs_of(good):
                if s0 <= i + 1 <= e0:
                    blk = '\n'.join(gl[s0 - 1:e0])
                    if not ('c->fd = -1' in blk and 'http_pend_clear' in blk):
                        caught2 = True
    chk("X2 判定器自证：配对完整的函数不得报出（防恒红）", not caught2)

    print("── M263 生命周期审查：%d 通过 / %d 失败" % (ok, bad))
    print("M263-LIFECYCLE-OK" if bad == 0 else "M263-LIFECYCLE-FAIL")
    sys.exit(0 if bad == 0 else 1)


main()
