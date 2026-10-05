#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M261 门的负控助手
# ------------------------------------------------------------
#   revert-pend-put          把 px_conn_pend_put **忠实退回** M261 之前的实现
#                            （锁内 xrealloc/xmalloc）⇒ 守卫必须判红
#   neuter-guard <out>       生成「判据失明」版守卫（清空 DIRECT/INDIRECT 两张表）
#                            ⇒ 用它跑同一份被植入缺陷的源码**必须不再判红**
#                            （证明负控 A 的红确实来自判据）
# ⚠️ 本脚本**只改源码**；还原由 verify.sh 的快照 + trap 负责（逐字节 cmp 自证）。
# ============================================================
import io
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
RC = os.path.join(ROOT, 'runtime', 'runtime.c')
GUARD = os.path.join(ROOT, 'selfhost', 'check_lock_alloc.py')

FN_SIG = 'static void px_conn_pend_put(int fd, const char* data, int n) {'

# M261 之前的实现（逐字照抄自 git 的 M260 版本）
OLD_BODY = '''static void px_conn_pend_put(int fd, const char* data, int n) {
    if (fd < 0 || n <= 0 || !data) return;
    pthread_mutex_lock(&g_conn_mu);
    PxConnCtx* c = px_evc_ctx(fd);
    if (c && c->fd == fd) {
        if (n > c->pbuf_cap) {
            int ncap = c->pbuf_cap > 0 ? c->pbuf_cap : 4096;
            while (ncap < n) ncap *= 2;
            c->pbuf = c->pbuf ? xrealloc(c->pbuf, (size_t)ncap) : xmalloc((size_t)ncap);
            c->pbuf_cap = ncap;
        }
        memcpy(c->pbuf, data, (size_t)n);
        c->pbuf_len = n;
    }
    pthread_mutex_unlock(&g_conn_mu);
}'''


def cut_function(src, sig):
    i = src.find(sig)
    if i < 0:
        return None, None
    depth = 0
    for j in range(i, len(src)):
        if src[j] == '{':
            depth += 1
        elif src[j] == '}':
            depth -= 1
            if depth == 0:
                return i, j + 1
    return None, None


def _strip_lits(s):
    """只去掉**字符串/字符字面量**（保留其它字符）—— 用于「数括号」时不受字面量干扰。
    ⚠️ 自伤教训：`DIRECT_RX = re.compile(r'\\b(xmalloc|…|'` + 下一行 `r'm128_alloc)\\|…')`
       的**括号在字面量里**，直接数会永远不平衡 ⇒ 该语句会一路吞掉后面的行
       （实测把 `INDIRECT_RX` 整条吃掉 ⇒ 中性化命中 0 次）。"""
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


def neuter_stmt(src, var):
    """把 `var = re.compile(...)` 这**整条语句**（可能跨行）换成永不匹配的模式。
    ⚠️ 自伤教训：只替换 `\\b(xmalloc` 的第一个分支 ⇒ `|xrealloc|xcalloc|…` 仍在
       ⇒ 「判据失明」假成立（本轮实测：负控 C 仍判红，差点误判成「红来路不明」）。"""
    lines = src.split('\n')
    out, i = [], 0
    hit = 0
    while i < len(lines):
        if lines[i].startswith(var + ' = '):
            depth = 0
            j = i
            while j < len(lines):
                st = _strip_lits(lines[j])
                depth += st.count('(') - st.count(')')
                j += 1
                if depth <= 0:
                    break
            out.append("%s = re.compile(r'(?!x)x')   # [NC] 负控：判据失明" % var)
            i = j
            hit += 1
        else:
            out.append(lines[i])
            i += 1
    if hit != 1:
        raise SystemExit("✗ 中性化 %s 命中 %d 次（期望 1）" % (var, hit))
    return '\n'.join(out)


def main():
    if len(sys.argv) < 2:
        print("用法：negctl.py revert-pend-put | neuter-guard <out>")
        return 2
    cmd = sys.argv[1]

    if cmd == 'revert-pend-put':
        src = io.open(RC, encoding='utf-8').read()
        a, b = cut_function(src, FN_SIG)
        if a is None:
            print("✗ 找不到 px_conn_pend_put")
            return 1
        cur = src[a:b]
        if 'xrealloc' in cur and 'pthread_mutex_lock' in cur and cur.find('xrealloc') > cur.find('pthread_mutex_lock'):
            print("✗ 当前实现**看起来已是**旧形态（勿重复植入）")
            return 1
        io.open(RC, 'w', encoding='utf-8').write(src[:a] + OLD_BODY + src[b:])
        print("✓ 已植入 M261 之前的 px_conn_pend_put（锁内 xrealloc/xmalloc）")
        return 0

    if cmd == 'neuter-guard':
        if len(sys.argv) < 3:
            print("用法：negctl.py neuter-guard <out>")
            return 2
        out = sys.argv[2]
        s = io.open(GUARD, encoding='utf-8').read()
        s2 = neuter_stmt(neuter_stmt(s, 'DIRECT_RX'), 'INDIRECT_RX')
        if s2 == s:
            print("✗ 两张表都未替换（锚点失配 ⇒ 自伤负控会假绿）")
            return 1
        io.open(out, 'w', encoding='utf-8').write(s2)
        print("✓ 已生成判据失明版守卫：%s" % out)
        return 0

    print("未知子命令：%s" % cmd)
    return 2


sys.exit(main())
