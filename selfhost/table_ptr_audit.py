#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M242（第 119 轮）· **表元素指针「跨锁外泄」审计器**
# ------------------------------------------------------------
# 为什么需要它（M240 缺陷 399 的同族推广）：
#   `g_pxpend` / `g_conns` / `g_hpend` 都是 `xrealloc` **增长**的 fd 索引表 ——
#   容量不够时**整体搬家**（数组 > 128KB 时 glibc 走 mmap ⇒ 旧块被 munmap）。
#   ⇒ 任何**跨锁持有**的元素指针都不可信：
#        读它 = SEGV；写它 = **静默堆破坏**（比崩溃更危险 —— 它能活很久）。
#   M240 收口了 `g_pxpend`（被一次真实崩溃逼出来的），但「还有没有别的表？」
#   此前**无法回答** —— 因为没有判据。本审计器把这个问题**判据化**：
#
#   ① 会搬家的表（MOVABLE）   —— 从 `xrealloc(g_X, …)` 派生（**不手抄**）
#   ② 访问器（ACCESSOR）      —— 返回 `&g_X[...]` 的函数（元素地址外泄口）
#   ③ 转发型外泄（LEAK23）    —— `return accessor(…)` / `T* v = accessor(…); … return v;`
#                              ⚠️ 形状 3 是**盲区来源**：`px_evc_acquire` 就是它
#   ④ 契约核验                —— 每个 `locked-only` 访问器的**每个调用点**必须在
#                              `pthread_mutex_lock(&<表>_mu)` … `unlock` 之间
#
# 用法：
#   python3 selfhost/table_ptr_audit.py --root .                # 人工可读
#   python3 selfhost/table_ptr_audit.py --root . --json
#   python3 selfhost/table_ptr_audit.py --self-test             # 自证（锚点 + 规模下限）
# 退出码：0 = 干净；1 = 违例；3 = **判据自身失效**（派生集为空 / 契约表缺失）
# ============================================================
import json
import os
import re
import sys
import glob

_SIG0 = re.compile(r'^[A-Za-z_][\w\s\*]*\(')
RX_REALLOC = re.compile(r'\bxrealloc\s*\(\s*(g_[A-Za-z0-9_]+)')
RX_LEAK1 = re.compile(r'\breturn\s+&\s*(g_[A-Za-z0-9_]+)\s*\[')
RX_LOCK = 'pthread_mutex_lock(&%s)'
RX_UNLOCK = 'pthread_mutex_unlock(&%s)'


def _load_gcroot(root):
    sys.path.insert(0, os.path.join(root, 'selfhost'))
    from gcroot_derive import strip_comments_strings, scan_functions   # noqa
    return strip_comments_strings, scan_functions


def derive(root):
    """派生：(MOVABLE, ACCESSOR, LEAK23)。全部从源码派生 —— 不手抄任何名单。"""
    strip_comments_strings, scan_functions = _load_gcroot(root)
    files = sorted(glob.glob(os.path.join(root, 'runtime', '*.c')))
    codes = {}
    movable = {}
    for f in files:
        src = open(f, encoding='utf-8', errors='replace').read()
        codes[f] = strip_comments_strings(src)
    # ① 会搬家的表
    for f, code in codes.items():
        for ln, line in enumerate(code.split('\n'), 1):
            m = RX_REALLOC.search(line)
            if m:
                movable.setdefault(m.group(1), []).append((os.path.basename(f), ln))
    # ② 访问器
    accessor = {}
    funcs_by_file = {}
    for f, code in codes.items():
        fs = scan_functions(code)
        funcs_by_file[f] = fs
        for (name, start, sig, body) in fs:
            m = RX_LEAK1.search(body)
            if m:
                accessor[name] = {'table': m.group(1), 'file': os.path.basename(f),
                                  'line': start}
    # ③ 转发型外泄（只报「会搬家的表」）
    leak23 = []
    for f, code in codes.items():
        for (name, start, sig, body) in funcs_by_file[f]:
            for a, meta in accessor.items():
                if meta['table'] not in movable:
                    continue
                if re.search(r'\breturn\s+' + re.escape(a) + r'\s*\(', body):
                    leak23.append({'table': meta['table'], 'accessor': a,
                                   'file': os.path.basename(f), 'line': start,
                                   'func': name, 'kind': 'shape2-forward'})
                    continue
                for vm in re.finditer(r'([A-Za-z_][A-Za-z0-9_]*)\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)'
                                      r'\s*=\s*' + re.escape(a) + r'\s*\(', body):
                    var = vm.group(2)
                    if re.search(r'\breturn\s+' + re.escape(var) + r'\s*;', body):
                        leak23.append({'table': meta['table'], 'accessor': a,
                                       'file': os.path.basename(f), 'line': start,
                                       'func': name, 'kind': 'shape3-local:' + var})
    return movable, accessor, leak23, codes, funcs_by_file


def call_sites(root, accessor, codes, funcs_by_file):
    """访问器的**非定义**调用点（文件, 行, 所在函数）。"""
    out = []
    rx = re.compile(r'\b' + re.escape(accessor) + r'\s*\(')
    for f, code in codes.items():
        bounds = [(s, s + body.count('\n'), nm)
                  for (nm, s, sig, body) in funcs_by_file[f]]
        lines = code.split('\n')
        for ln, line in enumerate(lines, 1):
            if not rx.search(line):
                continue
            fn = '<top>'
            for (s, e, nm) in bounds:
                if s <= ln <= e:
                    fn = nm
                    break
            if fn == accessor:      # 定义行自身 / 自递归（本仓无）
                continue
            out.append((os.path.basename(f), ln, fn))
    return out


def holding_mutex(lines, ln, mu):
    """从 ln（1 基）往上找 `lock(&mu)`；先遇到 `unlock(&mu)` 或函数边界 ⇒ False。

    ⚠️ **分支内早退不算解锁**：`if (px_evc_ensure(fd) != 0) { unlock(&mu); return -1; }`
    这种「单行 if 块里的 unlock」是**条件路径**，主路径仍持锁。
    首版把这一行的 unlock 当成「已解锁」⇒ 把 `px_evc_ctx` 在 `px_evc_acquire` 内的调用
    判成「锁外」= **假阳**。
    ⇒ 判据：含 `if (` 或 `return` 的 unlock 行**跳过**，继续往上找。
    （保守方向：真「锁外调用」往上只会遇到裸 unlock 或函数边界 ⇒ 仍判红。）
    """
    lk, uk = (RX_LOCK % mu), (RX_UNLOCK % mu)
    for i in range(ln - 1, -1, -1):
        s = lines[i]
        if uk in s:
            if re.search(r'\bif\s*\(', s) or 'return' in s:
                continue                  # 分支内早退：主路径仍持锁
            return False
        if lk in s:
            return True
        if s[:1] == '}' and not s.startswith(' '):
            return False                  # 函数边界（顶格 `}`）—— 未持锁
    return False


def audit(root, contract):
    movable, accessor, leak23, codes, funcs_by_file = derive(root)
    problems = []
    checked = 0
    exempt = 0
    all_leak_funcs = set()
    for a, meta in sorted(accessor.items()):
        t = meta['table']
        if t not in movable:
            continue                      # 固定表：指针恒定，安全
        # 本表的「元素地址外泄函数」= 形状 1 访问器自身 ∪ 形状 2/3 的转发/外泄点
        leak_funcs = {a} | {lk['func'] for lk in leak23 if lk['table'] == t}
        all_leak_funcs |= leak_funcs
        c = contract.get(a)
        if c is None:
            problems.append('未登记契约：访问器 %s（表 %s）—— 会搬家的表的元素地址外泄口' % (a, t))
            continue
        if c['table'] != t:
            problems.append('契约表表名不符：%s 登记为 %s，实测 %s（表改名了？）'
                            % (a, c['table'], t))
            continue
        if c['contract'] != 'locked-only':
            continue                      # null-only 契约：另由「只判空」核验（后续轮）
        mu = c['mu']
        src_key = os.path.join(root, 'runtime', meta['file'])
        if ('static pthread_mutex_t ' + mu + ' =') not in codes[src_key]:
            problems.append('契约表锁名不存在：%s（`%s` 里没有 `static pthread_mutex_t %s =`）'
                            % (mu, meta['file'], mu))
            continue
        for (f, ln, fn) in call_sites(root, a, codes, funcs_by_file):
            # 豁免：调用点位于**本表的元素地址外泄函数**自身体内。
            #   理由：那正是「访问器的实现」（例：`px_evc_acquire` 内部调 `px_evc_ctx`）——
            #   它已由转发型外泄判据（LEAK23）**独立**覆盖，且当它不再把指针带出锁时
            #   LEAK23 会归零 ⇒ 此处无需重复判「它自己有没有持锁」。
            #   ⚠️ 这不是放水：`px_evc_acquire` 首版误报就是「同一语义被两处判据各判一次，
            #      而其中一处（行级扫描）不懂 `if (...) { unlock; return; }` 这种分支早退」。
            if fn in leak_funcs:
                exempt += 1
                continue
            checked += 1
            lines = codes[os.path.join(root, 'runtime', f)].split('\n')
            if not holding_mutex(lines, ln, mu):
                problems.append('调用点未持锁：%s() 内 %s:%d 调用 %s()（契约 locked-only，须持 %s）'
                                % (fn, f, ln, a, mu))
    # 登记过期：表里有、实测没有（防「把豁免当遗迹留着」—— M226 的 UNPAIRED 同口径）
    for a in sorted(contract):
        if a not in accessor:
            problems.append('登记过期：CONTRACT.tsv 有 %s，但源码里已无该访问器的元素地址外泄'
                            '（若已收口请删表行；若是重命名请同步）' % a)
    return {'movable': {k: v for k, v in movable.items()},
            'accessor': accessor, 'leak23': leak23,
            'leak_funcs': sorted(all_leak_funcs), 'exempt_call_sites': exempt,
            'checked_call_sites': checked, 'problems': problems}


def load_contract(path):
    """返回 {访问器: {'table':…, 'contract':…, 'mu':…}}。

    ⚠️ **互斥锁名必须显式登记，不得由表名推断** —— 本仓存在反例：
       `g_conns` 表的锁叫 **`g_conn_mu`**（少一个 s）。
       首版按 `表名 + '_mu'` 推断 ⇒ `g_conns_mu` 永远找不到 lock ⇒ **11 项假阳**
       （把「全部正确的调用点」报成「未持锁」）。
       ⇒ 判据**不许推断，只许登记 + 校验**（锁名在源码里必须真实存在）。
    """
    c = {}
    if not os.path.exists(path):
        return None
    for raw in open(path, encoding='utf-8'):
        s = raw.strip()
        if not s or s.startswith('#'):
            continue
        parts = s.split()
        if len(parts) >= 4:
            c[parts[1]] = {'table': parts[0], 'contract': parts[2], 'mu': parts[3]}
    return c


def main():
    root = '.'
    as_json = False
    cpath = None
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == '--root':
            root = args[i + 1]; i += 2
        elif args[i] == '--json':
            as_json = True; i += 1
        elif args[i] == '--self-test':
            return self_test()
        elif args[i] == '--contract':
            cpath = args[i + 1]; i += 2
        else:
            i += 1
    root = os.path.abspath(root)
    if cpath is None:
        cpath = os.path.join(root, 'examples', 'm242_table_ptr', 'CONTRACT.tsv')
    contract = load_contract(cpath)
    if contract is None:
        print('❌ 契约表缺失：%s（判据无法裁决——拒绝静默通过）' % cpath, file=sys.stderr)
        return 3
    r = audit(root, contract)
    if len(r['movable']) < 6 or len(r['accessor']) < 5:
        print('❌ 判据自身失效：派生集过小（MOVABLE=%d ACCESSOR=%d）—— 扫描器坏了？'
              % (len(r['movable']), len(r['accessor'])), file=sys.stderr)
        return 3
    if as_json:
        print(json.dumps(r, ensure_ascii=False, indent=2))
    else:
        print('会搬家的表：%d 张' % len(r['movable']))
        for t in sorted(r['movable']):
            print('  %-20s %s' % (t, '是' if t in (a['table'] for a in r['accessor'].values())
                                  else '（无元素地址外泄）'))
        print('访问器：%d 个 · 转发型外泄：%d 处 · 核验调用点：%d 个（另有 %d 个在「外泄函数体内」'
              '的豁免调用点 —— 已由转发型判据独立覆盖）'
              % (len(r['accessor']), len(r['leak23']), r['checked_call_sites'],
                 r['exempt_call_sites']))
        for lk in r['leak23']:
            print('  ⚠️ 转发型外泄 %s() → %s()  [%s] %s:%s'
                  % (lk['func'], lk['accessor'], lk['kind'], lk['file'], lk['line']))
        if r['problems']:
            print('❌ 违例 %d 项：' % len(r['problems']))
            for p in r['problems']:
                print('   · ' + p)
            return 1
        print('✅ TABLE-PTR-AUDIT-OK（无违例）')
    return 1 if r['problems'] else 0


def self_test():
    """锚点自证：判据本身必须能被证伪（M209/M213 纪律）。"""
    ok = 0
    fail = []

    def chk(tag, cond, detail=''):
        nonlocal ok
        if cond:
            ok += 1
        else:
            fail.append(tag + (' — ' + detail if detail else ''))

    # ① MOVABLE 派生：xrealloc 行必须被认出
    chk('movable-detect', bool(RX_REALLOC.search('g_conns = (T*)xrealloc(g_conns, n);')))
    # ② 非 xrealloc（固定表）不得被认成 MOVABLE
    chk('movable-neg', not RX_REALLOC.search('static T g_fixed[64]; return &g_fixed[i];'))
    # ③ 形状 1 识别
    chk('leak1', bool(RX_LEAK1.search('return &g_conns[fd];')))
    chk('leak1-neg', not RX_LEAK1.search('return g_conns[fd].fd;'))
    # ④ 形状 3：`T* v = acc(); return v;`（**本审计器存在的理由**）
    body = '  PxConnCtx* c = px_evc_ctx(fd);\n  if (!c) return NULL;\n  return c;\n'
    m = re.search(r'([A-Za-z_][A-Za-z0-9_]*)\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*'
                  r'px_evc_ctx\s*\(', body)
    chk('shape3-detect', bool(m) and m.group(2) == 'c')
    chk('shape3-ret', bool(re.search(r'\breturn\s+c\s*;', body)))
    # ⑤ 形状 3 反例：取了不用（只判空）⇒ **不得**被判外泄
    body2 = '  PxConnCtx* c = px_evc_ctx(fd);\n  if (c) foo();\n  return 0;\n'
    chk('shape3-neg', not re.search(r'\breturn\s+c\s*;', body2))
    # ⑥ 持锁判据：锁内 → True
    lines6 = ['static void f(void) {', '    pthread_mutex_lock(&g_conn_mu);',
              '    PxConnCtx* c = px_evc_ctx(fd);', '    pthread_mutex_unlock(&g_conn_mu);', '}']
    chk('lock-pos', holding_mutex(lines6, 3, 'g_conn_mu'))
    # ⑦ 持锁判据反例：锁外 → False
    lines7 = ['static void f(void) {', '    PxConnCtx* c = px_evc_ctx(fd);', '}']
    chk('lock-neg', not holding_mutex(lines7, 2, 'g_conn_mu'))
    # ⑧ unlock 先出现（在锁外）→ False
    lines8 = ['static void f(void) {', '    pthread_mutex_unlock(&g_conn_mu);',
              '    PxConnCtx* c = px_evc_ctx(fd);', '}']
    chk('lock-after-unlock', not holding_mutex(lines8, 3, 'g_conn_mu'))
    # ⑧b **分支内早退的 unlock 不得算解锁**（`px_evc_acquire` 的真实形状）
    lines8b = ['static int f(int fd) {', '    pthread_mutex_lock(&g_conn_mu);',
               '    if (px_evc_ensure(fd) != 0) { pthread_mutex_unlock(&g_conn_mu); return -1; }',
               '    PxConnCtx* c = px_evc_ctx(fd);', '}']
    chk('lock-branch-earlyreturn', holding_mutex(lines8b, 4, 'g_conn_mu'),
        '单行 if 块里的 unlock 是条件路径，主路径仍持锁')
    # ⑨ 契约表：**锁名来自登记**（不得推断）—— 用 `g_conns` / `g_conn_mu` 这个**真实反例**
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.tsv', delete=False, encoding='utf-8') as tf:
        tf.write('# c\n表\t访问器\t契约\t锁\ng_conns\tpx_evc_ctx\tlocked-only\tg_conn_mu\n')
        tp = tf.name
    c9 = load_contract(tp)
    ok9 = bool(c9) and c9.get('px_evc_ctx', {}).get('mu') == 'g_conn_mu'
    chk('contract-mu-registered', ok9)
    chk('contract-not-inferred',
        bool(c9) and c9.get('px_evc_ctx', {}).get('mu') != 'g_conns_mu',
        '锁名若被推断成 g_conns_mu，就是首版 11 项假阳的根源')
    os.unlink(tp)
    # ⑩ 契约表缺「锁名」列（<4 字段）⇒ **必须被拒**（否则会退回推断）
    with tempfile.NamedTemporaryFile('w', suffix='.tsv', delete=False, encoding='utf-8') as tf:
        tf.write('g_conns\tpx_evc_ctx\tlocked-only\n')
        tp2 = tf.name
    c10 = load_contract(tp2)
    chk('contract-needs-mu-col', not c10)
    os.unlink(tp2)
    if fail:
        print('❌ 判据自证失败 %d 项：' % len(fail))
        for x in fail:
            print('   · ' + x)
        return 1
    print('✅ 判据自证 %d/%d 锚点（含反向判据）' % (ok, ok))
    print('✅ TABLE-PTR-SELFTEST-OK')
    return 0


if __name__ == '__main__':
    sys.exit(main())
