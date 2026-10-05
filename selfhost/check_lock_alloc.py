#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M260 · **持锁临界区内的可失败分配** —— 静态守卫
# M261（缺陷 464）：**判据键去掉行号** + 引入 `ACCEPTED:` 分类（见下「键与分类」）
# ------------------------------------------------------------
# 病灶（M127 → M128 → 439 → 464，同一条线的第四次）：
#   M125/M126 把「运行时错误 / 分配失败」从进程级收紧到请求级（longjmp 回隔离点）。
#   但 **longjmp 不展开 pthread 锁** ⇒ 失败点落在临界区内时，回卷会把锁永久留成
#   「已锁」⇒ 其他线程阻塞 ⇒ 进程既不服务也不退出（**假死**）。
#   M127 因此加了 locktrack（持锁栈 + 隔离点审计）—— 审计发现持锁时**只能 `_exit(1)`**
#   （服务中断 ~3s，好过永久假死，但仍是中断）。
#   M128 的目标：**把可失败分配移出临界区**（两阶段：锁外备货 → 锁内只发布指针）。
#   实测 M128 修了 3 处（`px_list_push` list_grow · `px_dict_set` dict_keys/vals_grow ·
#   `gc_register` gc_objs_grow 主路径），**漏了 `px_str_int_pool`** ⇒ 缺陷 439。
#   M260 修 439（3 处）并建本守卫 ⇒ 首扫 18 处、修后基线 13 处。
#   M261 收口 7 处（fserve_ensure×2 · px_conn_pend_put · px_pin_obj（死代码）·
#   px_const_put · px_rate_limit_try×2）⇒ 13 → 6。
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
#   它们是**真实欠账**，不是噪音。若「豁免」掉，守卫就变成摆设；若一直判红，又无法进门。
#   ⇒ 基线文件 `selfhost/lock_alloc_baseline.txt` 记下**当前已知清单**，判据变成：
#     · 实测 − 基线 非空 ⇒ **判红**（新出现的，必须当场处理或论证）；
#     · 基线 − 实测 非空 ⇒ **提示收口**（不判红；`--update` 更新基线，让欠账数只减不增）。
#
# **键与分类（M261 新增）**：
#   键 = `文件 <TAB> 函数 <TAB> 被调 <TAB> DIRECT/INDIRECT`，**不含行号**。
#   为什么去掉行号：行号键在「上游任何一处编辑」后整体位移 ⇒ 旧键全部失配 ⇒ 每次都
#   报一堆**假新增**（M261 实测：只改 runtime.c 就让 3 条基线项变成「新增」）。
#   这是本仓反复踩的「判据不可判定」形状（M223 缺陷 324 / M169 缺陷 185 同族）。
#   ⇒ 行号**照旧写进基线**（人工定位用），但**不参与比较**；比较用 (文件, 函数, 被调, 类别)
#     的**多重集**（Counter）—— 同函数同被调的 N 处仍按 N 计数，收口一处即少一处。
#   分类：基线行可选第 5 字段 `ACCEPTED:<理由>` = **有意保留**（打印为 ℹ️，不提示收口）；
#     无该字段 = **欠账**（DEBT，打印为 ⚠️，收口时提示）。`--update` **不会**自动添加
#     ACCEPTED（必须人工写理由）；但它会**保留**已存在的 ACCEPTED 标记（不因重扫而丢失）。
#     ⚠️ `ACCEPTED:` 后必须非空理由，否则 rc=3（判据失效）——「无理由豁免」= 把红当绿。
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
from collections import Counter

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

ACC = 'ACCEPTED:'


def scan_file(path):
    raw = io.open(path, encoding='utf-8', errors='replace').read()
    code = strip_comments_strings(raw)
    lines = code.split('\n')
    src = raw.split('\n')
    hits = []
    depth = 0
    func = None
    stack = []
    pending = ''
    for i, ln in enumerate(lines):
        # M261（缺陷 464）：**跨行签名**必须累积后再匹配 —— 只匹配单行会把
        #   `static int route_match(const char* method, ...\n  ...) {` 整个漏掉，
        #   该函数的命中被**归到上一个函数名下**（键错 ⇒ 收口/新增判定都错）。
        #   与 M212 缺陷 289 同族。规则：depth==0 且本行无 `;` 时累积；出现 `{` 即定名。
        if depth == 0 and ';' not in ln and not ln.lstrip().startswith('#'):
            pending = (pending + ' ' + ln.strip()) if pending else ln.strip()
            m = FN_RX.match(pending)
            if m:
                func = m.group(1)
                pending = ''
        else:
            pending = ''
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


def skey_of(h, root):
    """比较键：文件 / 函数 / 被调 / 类别 —— **不含行号**（行号位移不得产生假新增）"""
    return '%s\t%s\t%s\t%s' % (os.path.relpath(h[0], root), h[2] or '?', h[4], h[5])


def disp_of(h, root):
    """展示串（含行号，人工定位用）"""
    return '%s:%d\t%s\t%s\t%s' % (os.path.relpath(h[0], root), h[1], h[2] or '?', h[4], h[5])


def parse_base(line):
    """基线行 → (skey, 理由 or None)"""
    f = line.rstrip('\n').split('\t')
    if len(f) < 3:
        return None, None
    loc = f[0]
    rel = loc.rsplit(':', 1)[0] if ':' in loc else loc
    reason = None
    body = f[1:]
    if body and body[-1].startswith(ACC):
        reason = body[-1][len(ACC):].strip()
        body = body[:-1]
    if len(body) < 3:
        return None, None
    # 与 skey_of 同构：文件 / 函数 / 被调 / 类别（**不含行号**）
    return '%s\t%s\t%s\t%s' % (rel, body[0], body[1], body[2]), reason


def do_self_test():
    import tempfile
    import shutil
    import subprocess
    d = tempfile.mkdtemp()
    os.makedirs(os.path.join(d, 'runtime'))
    os.makedirs(os.path.join(d, 'selfhost'))
    rt = os.path.join(d, 'runtime', 'rt.c')
    base = os.path.join(d, 'selfhost', 'lock_alloc_baseline.txt')
    n = 0
    tot = 7

    def run(*extra):
        return subprocess.run([sys.executable, sys.argv[0], '--root', d] + list(extra),
                              capture_output=True, text=True)

    io.open(rt, 'w', encoding='utf-8').write(
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
    # ① 无基线 ⇒ 全部报出
    r = run()
    n += 1 if ('rt.c:3' in r.stdout and 'rt.c:8' in r.stdout
               and 'rt.c:12' not in r.stdout) else 0
    print("  ① 全量报出（3 处中的 2 处，锁外 1 处不报）⇒ %s" % ('OK' if n == 1 else 'BAD'))
    # ② --update 建基线
    run('--update')
    r = run()
    k2 = (r.returncode == 0 and '❌' not in r.stdout)
    n += 1 if k2 else 0
    print("  ② 建基线后 ⇒ rc=%d（期望 0） %s" % (r.returncode, "PASS" if k2 else "BAD::"+r.stdout[:120]))
    # ③ **M261 新判据**：整段行号位移（顶部插注释）⇒ 不得产生「假新增」、不得报「收口」
    body = io.open(rt, encoding='utf-8').read()
    io.open(rt, 'w', encoding='utf-8').write("// 顶部插一行注释（整段行号 +1）\n" + body)
    r = run('--verbose')
    # ⚠️ M261 自伤教训：判据必须**贴着契约串**写 —— 成功消息里含「待收口 N」，
    #   故「不得提示收口」只能查 **`已收口`**（查 `收口` 会把正常消息也算命中 ⇒ 恒判红）。
    ok3 = (r.returncode == 0 and '❌' not in r.stdout and '已收口' not in r.stdout
           and 'rt.c:4' in r.stdout and 'rt.c:9' in r.stdout)
    n += 1 if ok3 else 0
    print("  ③ 行号整体位移 ⇒ rc=%d 且无假新增/无假收口（期望 0） %s" % (r.returncode, "PASS" if ok3 else "BAD::"+r.stdout[:160]))
    # ④ 新增一处 ⇒ 判红
    with io.open(rt, 'a', encoding='utf-8') as f:
        f.write("void z(void) {\n    pthread_mutex_lock(&mu9);\n"
                "    void* q = xcalloc(1, 8);\n    pthread_mutex_unlock(&mu9);\n}\n")
    r = run()
    k4 = (r.returncode == 1 and '❌' in r.stdout and '新增' in r.stdout)
    n += 1 if k4 else 0
    print("  ④ 新增一处 ⇒ rc=%d（期望 1） %s" % (r.returncode, "PASS" if k4 else "BAD::"+r.stdout[:160]))
    # ⑤ 基线里的消失 ⇒ 提示收口（不判红）
    io.open(rt, 'w', encoding='utf-8').write("void h(void) {\n    void* p = xmalloc(16);\n}\n")
    r = run()
    k5 = (r.returncode == 0 and re.search(r'收口|已收口', r.stdout))
    n += 1 if k5 else 0
    print("  ⑤ 收口 ⇒ rc=%d 且有收口提示 %s" % (r.returncode, "PASS" if k5 else "BAD::"+r.stdout[:160]))
    # ⑥ 判据自伤：把 DIRECT 表清空 ⇒ ① 不再报
    src = io.open(sys.argv[0], encoding='utf-8').read()
    patched = src.replace("DIRECT_RX = re.compile(r'\\b(xmalloc", "DIRECT_RX = re.compile(r'\\b(NONE_")
    pf = os.path.join(d, 'patched.py')
    io.open(pf, 'w', encoding='utf-8').write(patched)
    io.open(rt, 'w', encoding='utf-8').write(
        "void f(void) {\n    pthread_mutex_lock(&mu);\n    void* p = xmalloc(16);\n"
        "    pthread_mutex_unlock(&mu);\n}\n")
    r = subprocess.run([sys.executable, pf, '--root', d], capture_output=True, text=True)
    k6 = ('❌' not in r.stdout)
    n += 1 if k6 else 0
    print("  ⑥ 判据自伤（清空 DIRECT 表）⇒ 不再报 %s" % ("PASS" if k6 else "BAD::"+r.stdout[:160]))
    # ⑦ **同一函数内同被调两处** ⇒ 基线必须写两行；重扫不得报假新增（多重集判定）
    io.open(rt, 'w', encoding='utf-8').write(
        "void m(void) {\n    pthread_mutex_lock(&mu);\n    void* p = xmalloc(8);\n"
        "    void* q = xmalloc(16);\n    pthread_mutex_unlock(&mu);\n}\n")
    subprocess.run([sys.executable, sys.argv[0], '--root', d, '--update'],
                   capture_output=True, text=True)
    nbase = len([l for l in io.open(base, encoding='utf-8')
                 if l.strip() and not l.startswith('#')])
    r = run()
    ok7 = (nbase == 2 and r.returncode == 0 and '❌' not in r.stdout)
    n += 1 if ok7 else 0
    print("  ⑦ 同函数同被调两处 ⇒ 基线 %d 行、重扫 rc=%d（期望 2 行 / 0）" % (nbase, r.returncode))
    print("self-test: %d/%d" % (n, tot))
    print("LOCK-ALLOC-SELFTEST-OK" if n == tot else "LOCK-ALLOC-SELFTEST-FAIL")
    shutil.rmtree(d, ignore_errors=True)
    sys.exit(0 if n == tot else 1)


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
        do_self_test()

    hits = scan_all(ROOT)
    cur = Counter(skey_of(h, ROOT) for h in hits)
    disp = {}
    for h in hits:
        disp.setdefault(skey_of(h, ROOT), disp_of(h, ROOT))

    old_reason = {}
    if os.path.exists(BASE):
        for l in io.open(BASE, encoding='utf-8'):
            if not l.strip() or l.startswith('#'):
                continue
            k, r = parse_base(l)
            if k and r:
                old_reason[k] = r

    if a.update:
        with io.open(BASE, 'w', encoding='utf-8') as f:
            f.write("# M260 · 持锁临界区内可失败分配 —— **已知欠账**清单（基线）\n")
            f.write("# 由 selfhost/check_lock_alloc.py --update 生成；判据 = **只减不增**：\n")
            f.write("#   实测 − 基线 非空 ⇒ 判红（新增，必须当场处理或论证）；\n")
            f.write("#   基线 − 实测 非空 ⇒ 提示收口（不判红）。\n")
            f.write("# M261：比较键**不含行号**（行号位移不得产生假新增）；行号仅供人工定位。\n")
            f.write("# 列：文件:行 <TAB> 函数 <TAB> 被调 <TAB> DIRECT/INDIRECT [<TAB> ACCEPTED:<理由>]\n")
            for k in sorted(cur):
                line = disp[k]
                if k in old_reason:
                    line += '\t' + ACC + old_reason[k]   # 保留人工写下的「有意保留」理由
                # M261：**按键出现次数逐行写出** —— 只写一次会让「同函数同被调 N 处」
                #   在下一次扫描时变成 N−1 处假新增（判据自伤）。
                for _ in range(cur[k]):
                    f.write(line + '\n')
        print("基线已更新：%s · %d 条（保留 ACCEPTED %d 条）" % (BASE, len(cur), len(old_reason)))
        sys.exit(0)

    base = Counter()
    reasons = {}
    if os.path.exists(BASE):
        for l in io.open(BASE, encoding='utf-8'):
            if not l.strip() or l.startswith('#'):
                continue
            k, r = parse_base(l)
            if k is None:
                print("❌ 基线行无法解析（判据失效）：%s" % l.strip())
                sys.exit(3)
            base[k] += 1
            if r is not None:
                if not r:
                    print("❌ 基线 `ACCEPTED:` **无理由**（判据失效，见文件头「键与分类」）：%s" % l.strip())
                    sys.exit(3)
                reasons[k] = r

    missing = cur - base
    fixed = base - cur
    if a.verbose:
        for path, lineno, func, lock, callee, kind, text in hits:
            print("   · %s:%d [%s] 持 %s ⇒ %s %s" % (path, lineno, func or '?', lock, callee, kind))
    if missing:
        print("❌ **新增**持锁临界区内的可失败分配（%d 处）：" % sum(missing.values()))
        for k, c in sorted(missing.items()):
            print("   · %s%s" % (disp.get(k, k), '' if c == 1 else ' ×%d' % c))
        print("   ⇒ 修法：两阶段（锁外备货 → 锁内只发布指针），见 runtime.c 的 M128/M261 段；")
        print("      若确属「不可失败 / 有意为之」，在基线行尾写 `ACCEPTED:<理由>` 并同步 CHANGELOG。")
        sys.exit(1)
    debt = sum(c for k, c in cur.items() if k not in reasons)
    acc = sum(c for k, c in cur.items() if k in reasons)
    msg = ("✅ 无新增（已知欠账 %d 处：待收口 %d + 有意保留 %d；基线 %d 处）"
           % (sum(cur.values()), debt, acc, sum(base.values())))
    if fixed:
        fk = [k for k in fixed if k not in reasons]
        msg += " · 已收口 %d 处" % sum(fixed.values())
        if fk:
            msg += "：%s" % '; '.join(k.split('\t')[0] + '/' + k.split('\t')[1] for k in fk[:6])
    print(msg)
    sys.exit(0)


main()
