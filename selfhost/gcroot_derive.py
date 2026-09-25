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


# ------------------------------------------------------------
# M213（缺陷 295）：间接调用判据 —— **两层，且如实命名**
#   精确层（precise）三条规则，**都是真的"调用"**：
#     ① 被调名 ∈ 本函数形参        ② `(*name)(…)`        ③ 成员调用 `x.name(…)`/`x->name(…)`
#   保守层（conservative）：**任何** `(*x` 形状（含 `while (*pp)` / `sizeof(*b)` /
#     `(*p).f` 这类**非调用语境**）—— 明知不精确，**仍然收**（宁多收）。
#   默认 `loose` = precise ∪ conservative；`tight` 仅用于**分诊**与 A/B。
# ------------------------------------------------------------
_CALLBREAK_RX = re.compile(r'\(\s*\*\s*[A-Za-z_][A-Za-z0-9_]*\s*\)\s*\(')
_MEMBER_CALL_RX = re.compile(r'(?:\.|->)\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(')
_CONSERVATIVE_RX = re.compile(r'\(\s*\*\s*[A-Za-z_]')


def has_indirect_call(body, params, mode='loose'):
    """间接调用判据（M212 缺陷 289 立 · **M213 缺陷 295 分层**）。

    M212 的意图写的是「被调方是形参 或 `(*p)(…)`」，但实现是
    `re.search(r'\\(\\s*\\*\\s*[A-Za-z_]', body)` —— 它匹配的是
    **「括号后跟星号」这个语法形状**，与「是否调用」无关，于是把

        while (*pp) {              ← 条件里的指针解引用
        memset(b, 0, sizeof(*b));  ← sizeof 的类型操作数
        (*p).field                 ← 成员访问

    全当成间接调用。实测（M213）：假种子 **99** 个（seed 119 → 精确 20），
    闭包 **518 → 349**（虚增 169 个函数），并**产出一条候选**
    （`px_rate_limit_try`，体内**无任何 gc_register 级调用**）。

    **为什么仍然保留宽松档做默认**：收紧会**新造漏报** —— 见缺陷 296
    （`px_gc_collect`/`px_gc_poll`/`bi_gc` 显式 GC 族此前**只靠这条兜底偶然覆盖**）。
    「漏报比假阳危险」⇒ 默认 `loose`；`tight` 只用于**分诊**与 A/B。

    精确三规则（`tight`，都是真的"调用"）：
      ① 被调名 ∈ 本函数形参（传进来的回调 / `fn.as.obj->as.func.fn(…)`）
      ② `(*name)(…)` —— 函数指针显式解引用后调用
      ③ 成员调用 `x.name(…)` / `x->name(…)` —— 结构体里的函数指针，静态图看不见
    保守兜底（`loose` 额外收）：任何 `(*x` 形状（含上面的非调用语境）。
    """
    for m in _CALL_RX.finditer(body):
        if m.group(1) in params:
            return True
    if _CALLBREAK_RX.search(body):
        return True
    for m in _MEMBER_CALL_RX.finditer(body):
        if m.group(1) not in _CTRL:
            return True
    return mode == 'loose' and _CONSERVATIVE_RX.search(body) is not None


# M213（缺陷 296）：**显式 GC 入口**。
#   M209 把触发点定义成「可达 `gc_register`」—— 而 `gc_register` 只是**自动**触发。
#   `px_gc_collect()`（`gc()` 内建 / 池循环硬上限兜底）与 `px_gc_poll()`
#   （请求间安全点）是**独立入口**，**不经过** `gc_register` ⇒ 派生集合看不见
#   ⇒ 任何「显式跑一次 GC 之后再读未登记值」的窗口**整族漏报**。
#   为什么此前没暴露：宽松兜底**偶然**把它们收了进来（体内有 `sizeof(*x)` 形状）
#   ⇒ 这正是「收紧间接调用判据会新造漏报」的实证（M213 决策依据）。
_GC_ENTRY_RE = re.compile(r'\b(gc_register|px_gc_collect|px_gc_poll)\s*\(')
_EXPLICIT_GC = ('px_gc_collect', 'px_gc_poll')

_SCAN_CACHE = {}


def _scan(root_dir):
    """扫描 `<root>/runtime/*.c` → (funcs, sigs, params)。按 root 缓存。"""
    if root_dir in _SCAN_CACHE:
        return _SCAN_CACHE[root_dir]
    rtdir = os.path.join(root_dir, 'runtime')
    funcs = {}
    for fn in sorted(os.listdir(rtdir)):
        if not fn.endswith('.c'):
            continue
        p = os.path.join(rtdir, fn)
        raw = open(p, encoding='utf-8', errors='replace').read()
        for nm, ln, sig, body in scan_functions(strip_comments_strings(raw)):
            funcs.setdefault(nm, (fn, ln, sig, body))
    sigs = {nm: v[2] for nm, v in funcs.items()}
    params = {nm: param_names(v[2]) for nm, v in funcs.items()}
    _SCAN_CACHE[root_dir] = (funcs, sigs, params)
    return funcs, sigs, params


def derive_sets(root_dir, indirect_mode='loose'):
    """从 `<root>/runtime/*.c` 派生 (triggers, producers, stats)。"""
    funcs, sigs, params = _scan(root_dir)

    def callees(body):
        return {m.group(1) for m in _CALL_RX.finditer(body) if m.group(1) not in _CTRL}

    # 直接触发点 = 体内调用任一 GC 入口（`gc_register` 自动 / 显式两入口）
    direct = {nm for nm, (_f, _l, _s, b) in funcs.items()
              if nm != 'gc_register' and _GC_ENTRY_RE.search(b)}
    # M213 缺陷 296：两个显式入口**自身**就是触发点（体内未必再调 gc_register）
    explicit = {nm for nm in _EXPLICIT_GC if nm in funcs}
    indirect = {nm for nm, (_f, _l, _s, b) in funcs.items()
                if has_indirect_call(b, params[nm], indirect_mode)}
    reach = set(direct) | explicit | set(indirect)
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
             'explicit_gc': len(explicit), 'indirect_mode': indirect_mode,
             'triggers': len(reach), 'producers': len(producers)}
    return reach, producers, stats


def derive_all(root_dir, indirect_mode='loose'):
    """(triggers, producers, stats, extra)。

    extra = {'reach_tight': set, 'return_type': fn} —— 精确档闭包，供审计器**分诊**：
    「只在 loose、不在 tight」的触发点 = 由**保守兜底**引入（过近似代价）。
    """
    trig, prod, st = derive_sets(root_dir, indirect_mode)
    tight, _tp, _ts = derive_sets(root_dir, 'tight')
    return trig, prod, st, {'reach_tight': tight, 'return_type': return_type}


# ------------------------------------------------------------
# M213（第 92 轮 · 缺陷 297）：**F1「被调方登记了实参」**（跨函数持有者）
#   形状：调用点 `h3_send_fields(conn, sid, fields, bv, 1)` 的 `bv` 是未登记活值，
#     但被调方**第一件事**就是 `PX_KEEP(body_val)`（形参位置 3 与实参位置 3 对应）
#     ⇒ 进入被调方后、**在任何分配之前**即被登记 ⇒ 调用点不构成风险。
#   ⚠️ 必须带「**登记先于第一个触发点**」这条：`PX_KEEP` 出现在首次分配**之后**
#      = 「登记迟到」（M182 缺陷 192 的同族形状）⇒ 那时值**已被回收** ⇒ **不得豁免**。
#      （自证锚点 J 就是按这条判红的 —— 它是「反向判据」。）
# ------------------------------------------------------------
_KEEP_CALL_RX = re.compile(r'\b(?:PX_KEEP|px_root_push_keep)\s*\(\s*'
                           r'([A-Za-z_][A-Za-z0-9_]*)\s*\)')


def derive_callee_keeps(root_dir, triggers=None):
    """返回 `{函数名: {'params': [形参名…], 'kept': [入口已登记的形参名…]}}`。

    `triggers` 可传入已派生的触发点集合（省一次闭包计算）；不传则自算。
    """
    funcs, _sigs, _params = _scan(root_dir)
    trig = triggers if triggers is not None else derive_sets(root_dir)[0]
    if not trig:
        return {}
    trig_rx = re.compile(r'\b(' + '|'.join(
        sorted((re.escape(t) for t in trig), key=len, reverse=True)) + r')\s*\(')
    out = {}
    for nm, (_f, _l, sig, body) in funcs.items():
        ordered = [n for n, _ip in sig_params(sig)]
        pset = set(ordered)
        if not pset:
            continue
        kept = []
        # ⚠️ `body` **含签名**（`static bool h3_send_fields(…) {`），而函数**自身**
        #   往往是触发点（它调用了别的触发点）⇒ 直接从 body 开头搜「之前的触发点」
        #   会**每次都在签名处自命中** ⇒ F1 整条**静默失效**（实测：`h3_send_fields`
        #   被漏掉、F1 表只剩 3 条）。⇒ 一律从**开括号之后**开始搜。
        brace = body.find('{')
        if brace < 0:
            brace = 0
        for m in _KEEP_CALL_RX.finditer(body):
            p = m.group(1)
            if p not in pset or p in kept:
                continue
            if m.start() < brace:
                continue
            # ② 登记必须**先于**被调方第一个触发点；否则是「登记迟到」⇒ 不豁免
            if trig_rx.search(body[brace:m.start()]):
                continue
            kept.append(p)
        if kept:
            out[nm] = {'params': ordered, 'kept': kept}
    return out


# ------------------------------------------------------------
# M213（第 92 轮 · 缺陷 294）：**出口参数式构造函数**（OUT-PRODUCER）派生
#   形状：`static int px_as_list(LXValue v, LXValue* out)` —— 返回 int/bool，
#   但经 `LXValue* out` **输出一个新对象**（tuple / 生成器 ⇒ `px_list(n)` 新建 list）。
#   ⇒ 调用点 `px_as_list(args[1], &xs)` 之后，`xs` 是**未登记的活值**；而审计器
#     只认「赋值给 lvalue 的 PRODUCER」⇒ 这类出口**看不见** ⇒ **整族漏报**。
#   判据（保守）：形参里有 `LXValue*`、且体内对该名做过 `*NAME = …` 赋值 ⇒ 收。
#     保守方向 = **宁多收**（多记 live ⇒ 更偏向「报」）—— 「漏报比假阳危险」。
#   实测：`px_as_list` 的 tuple/生成器支即 `*out = l;` —— 正是 M207 缺陷 263 的现场
#     （该缺陷当年是**动态压力筛**抓的，静态判据报不出）。
# ------------------------------------------------------------
def out_params(sig):
    """签名里类型为 `LXValue*` 的形参名（排除 `args` / `void`）。取**第一个平衡括号组**。"""
    return {n for n, p in sig_params(sig) if p}


def sig_params(sig):
    """签名形参的**有序**表 `[(name, is_lxvalue_ptr)]`。取**第一个平衡括号组**。

    ⚠️ 必须保序：OUT-PRODUCER 的调用点要**按位置**把实参与形参配对
      （`h_exchange(pool, req, rlen, &slot, &status, …)` —— `slot` 是 `HPoolSlot**`，
       不是 GC 对象；只有 `LXValue*` 那几个才是）。首版只收「函数名」⇒ 把所有
       `&x` 都当活值 ⇒ 8 条假阳（实测）。
    """
    i = sig.find('(')
    if i < 0:
        return []
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
        return []
    out = []
    for seg in sig[i + 1:j].split(','):
        ids = re.findall(r'[A-Za-z_][A-Za-z0-9_]*', seg)
        if not ids:
            continue
        nm = ids[-1]
        if nm in _CTRL or nm in ('void',):
            continue
        out.append((nm, bool(re.search(r'LXValue\s*\*', seg))))
    return out


def derive_out_producers(root_dir):
    """从 `<root>/runtime/*.c` 派生 `{函数名: {形参名: 是否 LXValue* 输出参数}}`。

    **保持签名顺序**（调用点按位置配对）。只保留至少有一个真输出参数的函数。
    """
    rtdir = os.path.join(root_dir, 'runtime')
    outs = {}
    for fn in sorted(os.listdir(rtdir)):
        if not fn.endswith('.c'):
            continue
        raw = open(os.path.join(rtdir, fn), encoding='utf-8', errors='replace').read()
        for nm, _ln, sig, body in scan_functions(strip_comments_strings(raw)):
            ps = sig_params(sig)
            if not any(p for _n, p in ps):
                continue
            rec = {}
            for pn, is_ptr in ps:
                # `*pn = …`（排除 `*pn == q`）
                rec[pn] = bool(is_ptr and re.search(
                    r'\*\s*' + re.escape(pn) + r'\s*=[^=]', body))
            if any(rec.values()):
                outs[nm] = rec
    return outs


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
    # ⑦ M213（缺陷 296）：**显式 GC 入口**族 —— `px_gc_collect()` 是独立触发入口，
    #    不经过 `gc_register`；其调用者（含 `gc()` 内建同形的 `mx_h`）同样会触发。
    'G_explicitgc': '''
static void mx_g(void) { px_gc_collect(); }
static LXValue mx_h(int n) { mx_g(); return px_null(); }
''',
    # ⑧ M213（缺陷 295）：**保守兜底的过近似形状** —— `while (*pp)` 是条件里的指针
    #    解引用，**不是调用**；精确档必须排除它，宽松档（默认）必须收它。
    'I_conservative': '''
static int mk_i(int* pp) {
    while (*pp) { *pp = *pp - 1; }
    return 0;
}
''',
    # ⑨ M213（缺陷 297）F1 **正向**：入口即登记 ⇒ 该形参豁免（调用点无风险）
    'K_earlykeep': '''
static void mk_k(LXValue v) {
    PX_KEEP(v);
    mk_a(1);
}
''',
    # ⑩ M213（缺陷 297）F1 **反向**：登记**迟到**（先出现触发点/分配，后登记）
    #    ⇒ **不得豁免** —— 这是防「把真缺陷豁免掉」的那道闸（M182 缺陷 192 同形）。
    'J_latekeep': '''
static void mk_j(LXValue v) {
    mk_a(1);
    PX_KEEP(v);
}
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
        trig_t, prod_t, st_t = derive_sets(td, 'tight')
        chk('① 直接 gc_register ⇒ TRIGGER', 'mk_a' in trig, 'mk_a∈TRIGGER')
        chk('② 间接（调用链）⇒ TRIGGER', 'mk_b' in trig,
            'mk_b∈TRIGGER（手抄集合漏的正是这层）')
        chk('③ 返回 int 的传递调用者 ⇒ 仅 TRIGGER',
            'mk_c' in trig and 'mk_c' not in prod, 'mk_c∈TRIGGER ∧ ∉PRODUCER')
        chk('④ 不碰分配的 ⇒ 两集合都不含', 'mk_d' not in trig and 'mk_d' not in prod,
            'mk_d∉TRIGGER ∧ ∉PRODUCER')
        chk('⑤ PRODUCER ⊆ TRIGGER', prod <= trig, '|P|=%d |T|=%d' % (len(prod), len(trig)))
        # ⑥ 规模锚点：**由 fixture 语义推出**（A 直触 / B·F 传递 / C 返 int / E 间接 /
        #    G 显式 GC 两函数 / I 过近似 / J·K F1 两函数（void ⇒ 只入 TRIGGER 不入 PRODUCER））
        #    ⇒ loose T=10 P=5；tight 少 mk_i ⇒ T=9 P=5
        #    ⚠️ M213：新增 fixture **必然**改这两个数 —— 锚点会当场判红（本轮实测），
        #       按语义**重新推**，不要照抄实测值。
        chk('⑥ fixture 规模锚点（loose）', st['triggers'] == 10 and st['producers'] == 5,
            'T=%d P=%d（期望 10/5）' % (st['triggers'], st['producers']))
        chk('⑦ **间接调用**（形参=函数指针）⇒ TRIGGER',
            'mk_e' in trig and 'mk_f' in trig, 'mk_e/mk_f∈TRIGGER（静态调用图看不见的那族）')
        chk('⑪ **显式 GC 入口**⇒ TRIGGER（缺陷 296）',
            'mx_g' in trig and 'mx_h' in trig,
            'mx_g/mx_h∈TRIGGER（`px_gc_collect()` 不经 gc_register）')
        chk('⑫ 过近似形状 ⇒ loose 在 / tight 不在（双向）',
            'mk_i' in trig and 'mk_i' not in trig_t,
            'mk_i: loose=%s tight=%s（`while (*pp)` 不是调用）'
            % ('在' if 'mk_i' in trig else '不在', '在' if 'mk_i' in trig_t else '不在'))
        chk('⑬ tight 档规模锚点', st_t['triggers'] == 9 and st_t['producers'] == 5,
            'T=%d P=%d（期望 9/5）' % (st_t['triggers'], st_t['producers']))
        _cks = derive_callee_keeps(td, trig)
        chk('⑯ F1：**入口登记** ⇒ 豁免（正向）',
            (_cks.get('mk_k') or {}).get('kept') == ['v'],
            'mk_k.kept=%s（期望 [v]，而 mk_j 不得含 v）'
            % ((_cks.get('mk_k') or {}).get('kept'),))
        chk('⑰ F1：**登记迟到** ⇒ 不豁免（反向·防放水）',
            'v' not in ((_cks.get('mk_j') or {}).get('kept') or []),
            'mk_j.kept=%s（期望 不含 v）' % ((_cks.get('mk_j') or {}).get('kept'),))

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    trig, prod, st = derive_sets(root)
    chk('⑧ 真实树规模下限', st['triggers'] >= 300 and st['producers'] >= 250,
        'T=%d P=%d 函数=%d 文件=%d' % (st['triggers'], st['producers'],
                                       st['funcs'], st['files']))
    chk('⑨ 真实树：手抄集合确实漏了', st['triggers'] > st['direct'] * 5,
        '直接=%d 间接=%d ⇒ 闭包=%d' % (st['direct'], st['indirect'], st['triggers']))
    chk('⑩ 真实树：`px_call` 必须在（间接调用族）', 'px_call' in trig,
        'px_call∈TRIGGER（缺陷 289 的锚点）')
    # ⑭ M213（缺陷 296）：**显式 GC 入口**族必须在 TRIGGER —— 此前只靠宽松兜底
    #    偶然覆盖；本条把它变成**断言**（收紧间接调用判据时会被这条挡住）。
    _gc_missing = [n for n in ('px_gc_collect', 'px_gc_poll', 'bi_gc') if n not in trig]
    chk('⑭ 真实树：显式 GC 入口族 ∈ TRIGGER', not _gc_missing,
        '缺失=%s' % (_gc_missing or '无'))
    # ⑮ M213（缺陷 295）：**过近似代价的登记锚点** —— `px_rate_limit_try` 体内无
    #    gc_register 级调用，却因 `while (*pp)`/`sizeof(*b)` 被宽松档收为触发点
    #    ⇒ 它是候选 #7 的来源（已判定假阳）。**有意保留**，收紧时必须同步处理。
    _tight_r = derive_sets(root, 'tight')[0]
    chk('⑮ 真实树：过近似锚点（px_rate_limit_try）**双向**',
        'px_rate_limit_try' in trig and 'px_rate_limit_try' not in _tight_r,
        'px_rate_limit_try: loose=%s tight=%s（保守兜底代价 = 候选 #3 的来源）'
        % ('在' if 'px_rate_limit_try' in trig else '不在',
           '在' if 'px_rate_limit_try' in _tight_r else '不在'))
    _ck = derive_callee_keeps(root, trig)
    chk('⑱ 真实树：F1 认出 `h3_send_fields(body_val)`（缺陷 291 形态）',
        'body_val' in ((_ck.get('h3_send_fields') or {}).get('kept') or []),
        'h3_send_fields.kept=%s' % ((_ck.get('h3_send_fields') or {}).get('kept'),))
    chk('⑲ 真实树：F1 表规模下限（防判据静默失效）', len(_ck) >= 5,
        'F1 表=%d 条' % len(_ck))
    print('gcroot_derive self-test: %d 通过 / %d 失败' % (ok, fail))
    return 0 if fail == 0 else 1


def root_guess(a):
    return a.root or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default=None)
    ap.add_argument('--list', default=None, choices=['triggers', 'producers', 'both'])
    ap.add_argument('--json', action='store_true')
    ap.add_argument('--self-test', action='store_true')
    ap.add_argument('--callee-keeps', action='store_true',
                    help='M213（缺陷 297）：打印 F1 表（被调方入口登记了哪些形参）')
    ap.add_argument('--indirect', choices=['loose', 'tight'], default='loose',
                    help='M213（缺陷 295）：间接调用判据档位。loose（默认）= 精确三规则'
                         ' + 任何 `(*x` 形状（**有意的过近似**，宁多收）；'
                         'tight = 仅精确三规则（**只用于分诊/A-B**，实测会丢失显式 GC '
                         '族覆盖 ⇒ 不作默认）')
    a = ap.parse_args()
    if a.callee_keeps:
        json.dump(derive_callee_keeps(root_guess(a)), sys.stdout,
                  ensure_ascii=False, indent=1, sort_keys=True)
        print()
        return 0
    if a.self_test:
        return self_test()
    root = root_guess(a)
    trig, prod, st = derive_sets(root, a.indirect)
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
