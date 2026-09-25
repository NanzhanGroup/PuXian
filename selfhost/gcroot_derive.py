#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M212（第 91 轮）· **触发点 / 生产者**的源码派生器
# ------------------------------------------------------------
# 为什么需要它（M212 缺陷 288 的根因）：
#   `gcroot_audit.py` 的「触发点」= **手抄**的一份构造器名单（27 个）。
#   而 M209 已把判据定成「触发点 = 调用链上会 `gc_register` 的入口」——
#   ⇒ 名单必须**从源码派生**，手抄必然漂移。
#   实测：源码派生的闭包是 **221** 个（手抄只覆盖 27）⇒ 手抄集合**漏掉 194 个
#   分配入口**，这正是「M209 遗留：漏报比假阳危险」的量化答案。
#
# 两个集合（M212 起分开，此前混为一谈）：
#   · TRIGGER  —— 调用链上可达 `gc_register` ⇒ **可能触发一次 GC**
#                 （受害者 = 此刻仍未登记的**活值**）
#   · PRODUCER —— TRIGGER ∩ 返回类型是 `LXValue` ⇒ 其结果**是一个新对象**，
#                 因而「创建后必须登记」这条约束对它成立
#   ⚠️ 混用的代价（实测）：`px_s3_exec` 返回 `int`、`px_parse_multipart` 返回 `void`
#      —— 它们**会分配**（是 TRIGGER），但**结果不是对象**。若把它们当 PRODUCER，
#      ④「把分配结果记进活集合」会把 `int st` 记成「未登记的 GC 受害者」⇒ 4 条假阳。
#
# 用法：
#   python3 selfhost/gcroot_derive.py --list triggers|producers|both
#   python3 selfhost/gcroot_derive.py --json
#   python3 selfhost/gcroot_derive.py --self-test     # 自证（6 锚点 + 规模下限）
# 作为库：
#   from gcroot_derive import derive_sets
#   trig, prod, stats = derive_sets('/path/to/puxian')
# ============================================================
import json
import os
import re
import sys

# 控制关键字：出现在 `(` 之前时**不是**函数定义
_CTRL = {'if', 'for', 'while', 'switch', 'else', 'do', 'return', 'sizeof',
         'case', 'goto', 'break', 'continue', 'union', 'struct', 'enum',
         'typedef', 'extern', 'static', 'inline', '_Static_assert', 'defined'}
_SIG0_RX = re.compile(r'^[A-Za-z_][\w\s\*]*\(')
_NAME_RX = re.compile(r'([A-Za-z_][A-Za-z0-9_]*)\s*\(')
_CALL_RX = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\(')


def strip_comments_strings(src):
    """去掉注释与字符串/字符字面量的**内容**，保留换行与位置（行号一一对应）。

    ⚠️ 与 `gcroot_audit.py` 里的同名函数**必须保持同一语义**（那边是审计用，
    这里是派生用）—— 本脚本是它的**上游**，故此行文为准；改动需同步两份。
    """
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
                state = 1; out.append('"'); i += 1; continue
            if c == "'":
                state = 2; out.append("'"); i += 1; continue
            out.append(c); i += 1
        elif state == 1:
            if c == '\\':
                out.append(' '); i += 2; continue
            if c == '"':
                state = 0; out.append('"')
            else:
                out.append(' ' if c != '\n' else '\n')
            i += 1
        elif state == 2:
            if c == '\\':
                out.append(' '); i += 2; continue
            if c == "'":
                state = 0; out.append("'")
            else:
                out.append(' ' if c != '\n' else '\n')
            i += 1
        elif state == 3:
            if c == '\n':
                state = 0; out.append('\n')
            else:
                out.append(' ')
            i += 1
        else:
            if c == '*' and nxt == '/':
                state = 0; out.append(' '); i += 2; continue
            out.append('\n' if c == '\n' else ' ')
            i += 1
    return ''.join(out)


def scan_functions(code):
    """按**花括号深度**切顶层函数。返回 [(name, start_lineno, sig_text, body_text)]。

    ⚠️ 必须支持**跨行签名**（M207 缺陷 260 的教训：单行正则会让跨行签名的函数
    整体不在扫描面上）。这里累积签名行直到含 `{`。
    """
    lines = code.split('\n')
    n = len(lines)
    funcs = []
    i = 0
    while i < n:
        if not _SIG0_RX.match(lines[i]):
            i += 1
            continue
        j = i
        buf = []
        dead = False
        while j < n and j - i < 20:
            buf.append(lines[j])
            joined = '\n'.join(buf)
            # ⚠️ 判据是「**含** `{`」而不是「**以** `{` 结尾」：单行函数体
            #   （`static int f(int a) { return a; }`）也是合法定义，用「以 { 结尾」
            #   会把它们**整族漏掉**（自证 ⑥ 首版 T=1 即由此而来）。
            if '{' in joined:
                break
            if ';' in joined:      # 到 `{` 之前先见到 `;` ⇒ 原型/声明 ⇒ 丢弃
                dead = True
                break
            j += 1
        if dead or '{' not in '\n'.join(buf):
            i += 1
            continue
        sig = '\n'.join(buf)
        m = _NAME_RX.search(sig)
        name = m.group(1) if m else None
        if not name or name in _CTRL:
            i = j + 1
            continue
        depth = 0
        started = False
        body = []
        k = j
        while k < n:
            body.append(lines[k])
            depth += lines[k].count('{') - lines[k].count('}')
            if '{' in lines[k]:
                started = True
            if started and depth <= 0:
                break
            k += 1
        funcs.append((name, i + 1, sig, '\n'.join(body)))
        i = k + 1
    return funcs


def return_type(sig, name):
    """从签名文本里取返回类型（去 `static` / `inline` 等修饰）。"""
    head = sig[:sig.index(name)] if name in sig else sig
    head = head.replace('\n', ' ').strip()
    head = re.sub(r'\b(static|inline|extern|const|unsigned|signed)\b', ' ', head)
    head = re.sub(r'\s+', ' ', head).strip()
    is_ptr = head.endswith('*')
    head = head.rstrip('* ').strip()
    words = head.split()
    base = words[-1] if words else '?'
    return (base + (' *' if is_ptr else ''))


def param_names(sig):
    """从签名里取**形参名**（每段最后一个标识符）。

    ⚠️ 必须取「**第一个平衡括号组**」而不是 `find('(')` … `rfind(')')`：
      单行函数体（`static LXValue f(int n) { return px_null(); }`）里 `rfind(')')`
      会落到**函数体内部的括号**上 ⇒ 形参表被拉成 `int n) { return px_null(`
      ⇒ `px_null` 被当成形参 ⇒ 误判为「间接调用」（实测：自证锚点 ④ 当场判红）。
    """
    i = sig.find('(')
    if i < 0:
        return set()
    depth = 0
    j = -1
    for k in range(i, len(sig)):
        if sig[k] == '(':
            depth += 1
        elif sig[k] == ')':
            depth -= 1
            if depth == 0:
                j = k
                break
    if j <= i:
        return set()
    inner = sig[i + 1:j]
    out = set()
    for seg in inner.split(','):
        ids = re.findall(r'[A-Za-z_][A-Za-z0-9_]*', seg)
        if not ids:
            continue
        nm = ids[-1]
        if nm in _CTRL or nm in ('void',):
            continue
        out.add(nm)
    return out


def has_indirect_call(body, params):
    """**间接调用**判据（M212 缺陷 289）：被调方是**形参**（函数指针）或 `(*p)(…)`。

    为什么必须有这一条：`px_call` 的体是
        `if (fn.type == PX_FUNC) return fn.as.obj->as.func.fn(args, nargs, …);`
    —— 它**显然会分配**（被调方是任意用户函数），但**静态调用图上不可见**
    ⇒ 闭包把它排除在外 ⇒ 派生集合漏掉整个「经函数指针的调用」族。
    实测：没有这条时 TRIGGER 少了 `px_call`/`px_method` 等，**候选 14 → 2**
    （把真漏报藏起来 —— 正是「漏报比假阳危险」）。
    """
    for m in _CALL_RX.finditer(body):
        if m.group(1) in params:
            return True
    return re.search(r'\(\s*\*\s*[A-Za-z_]', body) is not None


def derive_sets(root_dir):
    """从 `<root>/runtime/*.c` 派生 (triggers, producers, stats)。"""
    rtdir = os.path.join(root_dir, 'runtime')
    funcs = {}          # name -> (file, line, sig, body)
    for fn in sorted(os.listdir(rtdir)):
        if not fn.endswith('.c'):
            continue
        p = os.path.join(rtdir, fn)
        raw = open(p, encoding='utf-8', errors='replace').read()
        for nm, ln, sig, body in scan_functions(strip_comments_strings(raw)):
            funcs.setdefault(nm, (fn, ln, sig, body))
    sigs = {nm: v[2] for nm, v in funcs.items()}
    params = {nm: param_names(v[2]) for nm, v in funcs.items()}

    def callees(body):
        return {m.group(1) for m in _CALL_RX.finditer(body) if m.group(1) not in _CTRL}

    direct = {nm for nm, (_f, _l, _s, b) in funcs.items()
              if nm != 'gc_register' and re.search(r'\bgc_register\s*\(', b)}
    indirect = {nm for nm, (_f, _l, _s, b) in funcs.items()
                if has_indirect_call(b, params[nm])}
    reach = set(direct) | set(indirect)
    changed = True
    while changed:
        changed = False
        for nm, (_f, _l, _s, b) in funcs.items():
            if nm not in reach and (callees(b) & reach):
                reach.add(nm)
                changed = True
    producers = {nm for nm in reach
                 if return_type(sigs[nm], nm).startswith('LXValue')}
    stats = {'files': len({v[0] for v in funcs.values()}), 'funcs': len(funcs),
             'direct': len(direct), 'indirect': len(indirect),
             'triggers': len(reach), 'producers': len(producers)}
    return reach, producers, stats


# ------------------------------------------------------------
# 自证：6 锚点（4 必中 / 2 必不中）+ 规模下限
#   ⚠️ 锚点必须覆盖「**为什么要有这个工具**」的那条性质：
#      ① TRIGGER 含**直接**调 gc_register 的（fixture A）
#      ② TRIGGER 含**经调用链间接**到的（fixture B）—— 手抄集合正是漏在这一层
#      ③ 返回 `int` 的传递调用者 ⇒ 在 TRIGGER、**不在** PRODUCER（fixture C）
#      ④ 不碰分配的 ⇒ 两个集合都不在（fixture D，必不中）
#      ⑤ PRODUCER ⊆ TRIGGER（集合关系断言）
#      ⑥ 真实树规模下限（防「扫描器静默失效 ⇒ 空集 ⊇ 任意集」）
# ------------------------------------------------------------
FIX = {
    'A_direct': '''
static LXValue mk_a(int n) {
    LXObject* o = xmalloc(sizeof(LXObject));
    gc_register(o, sizeof(LXObject));
    return px_null();
}
''',
    'B_indirect': '''
static LXValue mk_b(int n) { return mk_a(n); }
''',
    'C_int_leaf': '''
static int mk_c(int n) { return mk_b(n) ? 1 : 0; }
''',
    'D_pure': '''
static LXValue mk_d(int n) { return px_null(); }
''',
    # ⑤ M212（缺陷 289）：**间接调用**（被调方是形参 = 函数指针）⇒ 必须视为触发点。
    #    真实形态 = `px_call`：`fn.as.obj->as.func.fn(args, nargs, …)` 静态图上看不见。
    'E_indirect': '''
static LXValue mk_e(LXValue fn, LXValue* args, int nargs) {
    if (fn.type == PX_FUNC) return fn.as.obj->as.func.fn(args, nargs, 0);
    return px_null();
}
''',
    'F_viaindirect': '''
static LXValue mk_f(LXValue g, int n) { return mk_e(g, NULL, n); }
''',
}


def self_test():
    import tempfile
    ok = fail = 0

    def chk(tag, cond, detail=''):
        nonlocal ok, fail
        if cond:
            ok += 1; print('  %-34s ✅ %s' % (tag, detail))
        else:
            fail += 1; print('  %-34s ❌ %s' % (tag, detail))

    with tempfile.TemporaryDirectory() as td:
        os.mkdir(os.path.join(td, 'runtime'))
        for nm, body in FIX.items():
            open(os.path.join(td, 'runtime', nm + '.c'), 'w').write(body)
        trig, prod, st = derive_sets(td)
        chk('① 直接 gc_register ⇒ TRIGGER', 'mk_a' in trig, 'mk_a∈TRIGGER')
        chk('② 间接（调用链）⇒ TRIGGER', 'mk_b' in trig,
            'mk_b∈TRIGGER（手抄集合漏的正是这层）')
        chk('③ 返回 int 的传递调用者 ⇒ 仅 TRIGGER',
            'mk_c' in trig and 'mk_c' not in prod, 'mk_c∈TRIGGER ∧ ∉PRODUCER')
        chk('④ 不碰分配的 ⇒ 两集合都不含', 'mk_d' not in trig and 'mk_d' not in prod,
            'mk_d∉TRIGGER ∧ ∉PRODUCER')
        chk('⑤ PRODUCER ⊆ TRIGGER', prod <= trig, '|P|=%d |T|=%d' % (len(prod), len(trig)))
        chk('⑥ fixture 规模锚点', st['triggers'] == 5 and st['producers'] == 4,
            'T=%d P=%d（期望 5/4）' % (st['triggers'], st['producers']))
        chk('⑦ **间接调用**（形参=函数指针）⇒ TRIGGER',
            'mk_e' in trig and 'mk_f' in trig, 'mk_e/mk_f∈TRIGGER（静态调用图看不见的那族）')

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    trig, prod, st = derive_sets(root)
    chk('⑧ 真实树规模下限', st['triggers'] >= 300 and st['producers'] >= 250,
        'T=%d P=%d 函数=%d 文件=%d' % (st['triggers'], st['producers'],
                                       st['funcs'], st['files']))
    chk('⑨ 真实树：手抄集合确实漏了', st['triggers'] > st['direct'] * 5,
        '直接=%d 间接=%d ⇒ 闭包=%d' % (st['direct'], st['indirect'], st['triggers']))
    chk('⑩ 真实树：`px_call` 必须在（间接调用族）', 'px_call' in trig,
        'px_call∈TRIGGER（缺陷 289 的锚点）')
    print('gcroot_derive self-test: %d 通过 / %d 失败' % (ok, fail))
    return 0 if fail == 0 else 1


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default=None)
    ap.add_argument('--list', default=None, choices=['triggers', 'producers', 'both'])
    ap.add_argument('--json', action='store_true')
    ap.add_argument('--self-test', action='store_true')
    a = ap.parse_args()
    if a.self_test:
        return self_test()
    root = a.root or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    trig, prod, st = derive_sets(root)
    if a.json:
        print(json.dumps({'triggers': sorted(trig), 'producers': sorted(prod),
                          'stats': st}, ensure_ascii=False, indent=1))
    elif a.list == 'triggers':
        print('\n'.join(sorted(trig)))
    elif a.list == 'producers':
        print('\n'.join(sorted(prod)))
    elif a.list == 'both':
        print('TRIGGER=%d' % len(trig)); print(' '.join(sorted(trig)))
        print('PRODUCER=%d' % len(prod)); print(' '.join(sorted(prod)))
    else:
        print('files=%(files)d funcs=%(funcs)d direct=%(direct)d '
              'triggers=%(triggers)d producers=%(producers)d' % st)
    return 0


if __name__ == '__main__':
    sys.exit(main())
