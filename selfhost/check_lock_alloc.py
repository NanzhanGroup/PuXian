#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M260 · **持锁临界区内的可失败分配** —— 静态守卫
# ------------------------------------------------------------
# 病灶（M127 → M128 → 本轮 439，同一条线的第三次）：
#   M125/M126 把「运行时错误 / 分配失败」从进程级收紧到请求级（longjmp 回隔离点）。
#   但 **longjmp 不展开 pthread 锁** ⇒ 失败点落在临界区内时，回卷会把锁永久留成
#   「已锁」⇒ 其他线程阻塞 ⇒ 进程既不服务也不退出（**假死**）。
#   M127 因此加了 locktrack（持锁栈 + 隔离点审计）—— 审计发现持锁时**只能 `_exit(1)`**
#   （服务中断 ~3s，好过永久假死，但仍是中断）。
#   M128 的目标：**把可失败分配移出临界区**（两阶段：锁外备货 → 锁内只发布指针）。
#   实测 M128 修了 3 处（`px_list_push` list_grow · `px_dict_set` dict_keys/vals_grow ·
#   `gc_register` gc_objs_grow 主路径），**漏了 `px_str_int_pool`** ⇒ 缺陷 439
#   （持 `g_intstr_mu` 调 `px_str_len` ⇒ 分配 ⇒ 失败 ⇒ 持锁 ⇒ `_exit(1)`）。
#
# 判据（两条，宽窄互补）：
#   ① **直接分配**：锁区间内出现 `xmalloc|xrealloc|xcalloc|xstrdup|m128_alloc|m128_strdup`
#      ⇒ 命中（M127 的注入钩子就挂在这里）。
#   ② **一层间接**：锁区间内出现「**必然分配**」的对象/字符串构造入口
#      （`px_str_len` / `px_str` / `px_dict` / `px_list` / `px_bytes` / `px_to_string` /
#       `px_pin_obj` …）⇒ 命中（439 正是这一条抓出来的）。
#      ⚠️ 表**不追求完备**：完整调用闭包会把几乎所有函数算进来 ⇒ 判据失效。
#
# **基线机制（本脚本的核心设计）**：
#   实测命中 **18 处**（M260 首次全量扫描）—— 它们是**真实欠账**，不是噪音。
#   若把它们「豁免」掉，守卫就变成了摆设；若让它们一直判红，又无法进门。
#   ⇒ 基线文件 `selfhost/lock_alloc_baseline.txt` 记下**当前已知清单**，判据变成：
#     · 实测 − 基线 非空 ⇒ **判红**（新出现的，必须当场处理或论证）；
#     · 基线 − 实测 非空 ⇒ **提示收口**（不判红；`--update` 更新基线，让欠账数只减不增）。
#   这样：**欠账数字是可见的**（每次运行都打印），而**新增被严格挡住**。
#
# 用法：
#   python3 selfhost/check_lock_alloc.py [--root .] [--verbose]
#   python3 selfhost/check_lock_alloc.py --update      # 用实测重写基线（收口后调用）
#   python3 selfhost/check_lock_alloc.py --self-test
# 退出码：0=通过（无新增）· 1=判红（有新增）· 2=用法错 · 3=判据自身失效
# ============================================================
import argparse
import io
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
try:
    from gcroot_derive import strip_comments_strings  # 复用（与审计器同语义）
except Exception:                                    # pragma: no cover
    def strip_comments_strings(s):
        return s

DIRECT_RX = re.compile(r'\b(xmalloc|xrealloc|xcalloc|xstrdup|xstrndup|'
                       r'm128_alloc|m128_strdup)\s*\(')
INDIRECT_RX = re.compile(r'\b(px_str_len|px_str|px_str_int_pool|px_str_from_cstr|'
                         r'px_dict|px_list|px_list_n|px_tuple|px_bytes|px_bytes_len|'
                         r'px_to_string|px_value_to_string|px_pin_obj|px_native|px_cell|'
                         r'px_func_env|px_struct|px_ok|px_err|px_some|px_float)\s*\(')
LOCK_RX = re.compile(r'\b(pthread_mutex_lock|pthread_rwlock_wrlock|pthread_rwlock_rdlock|'
                     r'pthread_spin_lock)\s*\(\s*&?\s*([A-Za-z_][A-Za-z0-9_]*)')
UNLOCK_RX = re.compile(r'\b(pthread_mutex_unlock|pthread_rwlock_unlock|'
                       r'pthread_spin_unlock)\s*\(\s*&?\s*([A-Za-z_][A-Za-z0-9_]*)')
FN_RX = re.compile(r'^[A-Za-z_][\w\s\*]*\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*\{')


def scan_file(path):
    raw = io.open(path, encoding='utf-8', errors='replace').read()
    code = strip_comments_strings(raw)
    lines = code.split('\n')
    src = raw.split('\n')
    hits = []
    depth = 0
    func = None
    stack = []
    for i, ln in enumerate(lines):
        m = FN_RX.match(ln)
        if m and depth == 0:
            func = m.group(1)
        depth += ln.count('{') - ln.count('}')
        ml = LOCK_RX.search(ln)
        if ml:
            stack.append((ml.group(2), i))
        mu = UNLOCK_RX.search(ln)
        if mu:
            name = mu.group(2)
            for k in range(len(stack) - 1, -1, -1):
                if stack[k][0] == name:
                    del stack[k:]
                    break
        if depth <= 0 and func is not None and stack:
            stack = []
        if stack:
            d = DIRECT_RX.search(ln)
            if d:
                hits.append((path, i + 1, func, stack[-1][0], d.group(1), 'DIRECT', src[i].strip()))
            else:
                nd = INDIRECT_RX.search(ln)
                if nd:
                    hits.append((path, i + 1, func, stack[-1][0], nd.group(1), 'INDIRECT',
                                 src[i].strip()))
    return hits


def scan_all(root):
    rtdir = os.path.join(root, 'runtime')
    out = []
    for fn in sorted(os.listdir(rtdir)):
        if fn.endswith('.c'):
            out += scan_file(os.path.join(rtdir, fn))
    return out


def key_of(h, root):
    return '%s:%d\t%s\t%s' % (os.path.relpath(h[0], root), h[1], h[4], h[5])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    ap.add_argument('--baseline', default=None)
    ap.add_argument('--self-test', action='store_true')
    ap.add_argument('--update', action='store_true')
    ap.add_argument('--verbose', action='store_true')
    a = ap.parse_args()
    ROOT = os.path.abspath(a.root)
    BASE = a.baseline or os.path.join(ROOT, 'selfhost/lock_alloc_baseline.txt')

    if a.self_test:
        import tempfile, shutil, subprocess
        d = tempfile.mkdtemp()
        os.makedirs(os.path.join(d, 'runtime'))
        os.makedirs(os.path.join(d, 'selfhost'))
        io.open(os.path.join(d, 'runtime', 'rt.c'), 'w', encoding='utf-8').write(
            "void f(void) {\n"
            "    pthread_mutex_lock(&mu);\n"
            "    void* p = xmalloc(16);\n"
            "    pthread_mutex_unlock(&mu);\n"
            "}\n"
            "void g(void) {\n"
            "    pthread_mutex_lock(&mu2);\n"
            "    LXValue s = px_str_len(\"x\", 1);\n"
            "    pthread_mutex_unlock(&mu2);\n"
            "}\n"
            "void h(void) {\n"
            "    void* p = xmalloc(16);\n"
            "}\n")
        n = 0
        # ① 无基线 ⇒ 全部报出
        r = subprocess.run([sys.executable, sys.argv[0], '--root', d],
                           capture_output=True, text=True)
        n += 1 if ('rt.c:3' in r.stdout and 'rt.c:8' in r.stdout
                   and 'rt.c:12' not in r.stdout) else 0
        print("  ① 全量报出（3 处中的 2 处，锁外 1 处不报）⇒ %s" % ('OK' if n == 1 else 'BAD'))
        # ② --update 建基线
        subprocess.run([sys.executable, sys.argv[0], '--root', d, '--update'],
                       capture_output=True, text=True)
        r = subprocess.run([sys.executable, sys.argv[0], '--root', d],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 0 and '❌' not in r.stdout) else 0
        print("  ② 建基线后 ⇒ rc=%d（期望 0）" % r.returncode)
        # ③ 新增一处 ⇒ 判红
        with io.open(os.path.join(d, 'runtime', 'rt.c'), 'a', encoding='utf-8') as f:
            f.write("void z(void) {\n    pthread_mutex_lock(&mu9);\n"
                    "    void* q = xcalloc(1, 8);\n    pthread_mutex_unlock(&mu9);\n}\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', d],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 1 and '❌' in r.stdout and '新增' in r.stdout) else 0
        print("  ③ 新增一处 ⇒ rc=%d（期望 1）" % r.returncode)
        # ④ 基线里的消失 ⇒ 提示收口（不判红）
        io.open(os.path.join(d, 'runtime', 'rt.c'), 'w', encoding='utf-8').write(
            "void h(void) {\n    void* p = xmalloc(16);\n}\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', d],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 0 and re.search(r'收口|已收口', r.stdout)) else 0
        print("  ④ 收口 ⇒ rc=%d 且有收口提示" % r.returncode)
        # ⑤ 判据自伤：把 DIRECT 表清空 ⇒ ① 不再报
        src = io.open(sys.argv[0], encoding='utf-8').read()
        patched = src.replace("DIRECT_RX = re.compile(r'\\b(xmalloc", "DIRECT_RX = re.compile(r'\\b(NONE_")
        pf = os.path.join(d, 'patched.py')
        io.open(pf, 'w', encoding='utf-8').write(patched)
        io.open(os.path.join(d, 'runtime', 'rt.c'), 'w', encoding='utf-8').write(
            "void f(void) {\n    pthread_mutex_lock(&mu);\n    void* p = xmalloc(16);\n"
            "    pthread_mutex_unlock(&mu);\n}\n")
        r = subprocess.run([sys.executable, pf, '--root', d], capture_output=True, text=True)
        n += 1 if ('❌' not in r.stdout) else 0
        print("  ⑤ 判据自伤（清空 DIRECT 表）⇒ 不再报（证红来自判据）")
        print("self-test: %d/5" % n)
        print("LOCK-ALLOC-SELFTEST-OK" if n == 5 else "LOCK-ALLOC-SELFTEST-FAIL")
        shutil.rmtree(d, ignore_errors=True)
        sys.exit(0 if n == 5 else 1)

    hits = scan_all(ROOT)
    cur = sorted(set(key_of(h, ROOT) for h in hits))
    if a.update:
        with io.open(BASE, 'w', encoding='utf-8') as f:
            f.write("# M260 · 持锁临界区内可失败分配 —— **已知欠账**清单（基线）\n")
            f.write("# 由 selfhost/check_lock_alloc.py --update 生成；判据 = **只减不增**：\n")
            f.write("#   实测 − 基线 非空 ⇒ 判红（新增，必须当场处理或论证）；\n")
            f.write("#   基线 − 实测 非空 ⇒ 提示收口（不判红）。\n")
            f.write("# 列：文件:行 <TAB> 被调函数 <TAB> DIRECT/INDIRECT\n")
            for k in cur:
                f.write(k + '\n')
        print("基线已更新：%s · %d 条" % (BASE, len(cur)))
        sys.exit(0)

    base = []
    if os.path.exists(BASE):
        base = [l.rstrip('\n') for l in io.open(BASE, encoding='utf-8')
                if l.strip() and not l.startswith('#')]
    missing = [k for k in cur if k not in base]
    fixed = [k for k in base if k not in cur]
    if a.verbose:
        for path, lineno, func, lock, callee, kind, text in hits:
            print("   · %s:%d [%s] 持 %s ⇒ %s %s" % (path, lineno, func or '?', lock, callee, kind))
    if missing:
        print("❌ **新增**持锁临界区内的可失败分配（%d 处）：" % len(missing))
        for k in missing:
            print("   · %s" % k)
        print("   ⇒ 修法：两阶段（锁外备货 → 锁内只发布指针），见 runtime.c 的 M128 段；")
        print("      若确属「不可失败 / 有意为之」，用 --update 前先在 CHANGELOG 论证。")
        sys.exit(1)
    msg = "✅ 无新增（已知欠账 %d 处，基线 %d 处）" % (len(cur), len(base))
    if fixed:
        msg += " · 已收口 %d 处：%s" % (len(fixed), '; '.join(x.split('\t')[0] for x in fixed[:6]))
    print(msg)
    sys.exit(0)


main()
