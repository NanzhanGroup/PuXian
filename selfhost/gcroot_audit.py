#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M206（第 85 轮）· GC 根面**静态审计器**
# ------------------------------------------------------------
# 背景：VM 轨产物默认 **precise GC**（根面 = 全局槽 + VM 帧槽 + TLS 登记根栈，
#   不扫整条 C 栈）。native 桥（bi_*）里**只活在 C 局部**的 LXValue 必须显式登记
#   （PX_KEEP / px_root_keep）。M170（缺陷 187/188）· M182（192）· M183（197/198/199）
#   共修了 9 处 —— 但都是「**被压力筛筛出来的**」，没有一次是「静态扫全仓找齐」。
#   ⇒ 本脚本补这一层：把「创建 → 未登记 → 再分配」这个**形状**静态找出来。
#
# 判据（M170 的两条硬约束）：
#   ① 容器/对象创建后必须登记；
#   ② 登记必须**紧跟创建、先于下一次分配**（因为下一次分配就可能触发 GC）。
#   ⇒ 形式化：若「未登记的活值集合」非空时又发生一次分配 ⇒ 该值可能被回收。
#
# 用法：
#   python3 selfhost/gcroot_audit.py [--files f1 f2 ...] [--json] [--show-victims]
#   python3 selfhost/gcroot_audit.py --self-test        # 自证（4 锚点，2 必中 2 必不中）
# ============================================================
import argparse
import json
import os
import re
import sys

# ---- 「分配」原语：调用后可能触发 GC（curated，来自 runtime.h 的构造函数集合） ----
_ALLOC_NAMES = [
    'px_str', 'px_str_len', 'px_bytes', 'px_bytes_len', 'px_bytes_own', 'px_dict', 'px_list',
    'px_list_n', 'px_struct', 'px_enum', 'px_tuple', 'px_native', 'px_ok',
    'px_err', 'px_some', 'px_cell', 'px_func', 'px_func_env', 'px_chan_create',
    'px_mutex_create', 'px_rwlock_create', 'px_gen_from_list', 'px_gen_lazy',
    'px_chk_uninit', 'px_env_lookup', 'px_slice', 'px_enum_variant', 'px_gen_next',
    'px_iter_at',      # M207（缺陷 263）：**按字符串取值会新建串对象** ⇒ 是一条分配路径
    'px_call', 'px_method', 'px_vm_call',
]
ALLOC_RX = re.compile(r'\b(' + '|'.join(_ALLOC_NAMES) + r')\s*\(')
GROW_ON = False   # M208：隐式分配规则开关（--grow 置位）
# ---- M208（缺陷 270）：**隐式分配**原语 ----
#   `px_dict_set` / `px_list_push` 自身会分配（键副本 `m128_strdup`、条目数组 `xrealloc` 扩容）
#   ⇒ 它们是**分配点**，必须计入「此后未登记活值可能被回收」。
#   为什么单独一张表：它们**不持有**受害者（容器本身不因这次调用成为 GC 根）⇒ 不能走
#   HOLD_RX 的「实参豁免」。M207 把它们放进 HOLD_RX 是为了豁免「把值放进已登记容器」，
#   但那恰好让「先建容器、再往里放东西」这一族（缺陷 268 = PXOP_NEWDICT）静默漏报。
#   实测：加上本表后 `PXOP_NEWDICT`（`LXValue d = px_dict(); … px_dict_set(d,…)`）当场被抓。
GROW_RX = re.compile(r'\b(px_dict_set|px_dict_set_checked|px_dict_set_locked|'
                     r'px_list_push|px_list_push_locked)\s*\(')
# ---- 明确**不分配**（立即数 / 常量池 / 纯读） ----
SAFE_RX = re.compile(
    r'\b(px_null|px_uninit|px_bool|px_int|px_float|px_str_const|px_str_const_n|'
    r'px_empty_str_get|px_dict_get|px_dict_get_ci|px_dict_len|px_len|px_type|'
    r'px_get_global|px_global_at|px_dict_keys|px_is_)\w*\s*\(')
# ---- 登记 / 交棒 ----
KEEP_RX = re.compile(r'\b(?:PX_KEEP|px_root_push_keep)\s*\(\s*([A-Za-z_][A-Za-z0-9_\[\]\.\->]*)')
KEEP2_RX = re.compile(r'\bpx_root_keep\s*\(\s*&\s*([A-Za-z_][A-Za-z0-9_\[\]\.\->]*)')
# ---- 「消费」：把值交给别的调用（假定被调方接管/复制/存进已登记容器） ----
# ⚠️ M207（缺陷 259）：`px_dict_set(d, k, v)` / `px_list_push(l, v)` **不在此列** ——
#   它们把这个值**存进** `d`/`l`，但**不登记 `d`/`l` 本身**（接收者不是"被交出去"了）。
#   把它们当消费会让「先建容器、再往里放东西」这一族**整族漏报**（实测漏掉 `bi_os_capture`）。
#   它们仍留在 HOLD_RX（持有**实参值** ⇒ 值从 g_tmp_root=新对象 出发可达）。
CONSUME_RX = re.compile(
    r'\b(bi_[A-Za-z0-9_]+|px_[A-Za-z0-9_]*raw[A-Za-z0-9_]*|px_call\w*|px_method|'
    r'px_invoke\w*|px_serve_\w+|px_global_set|'
    r'px_set_global|px_return|px_free|px_pin_obj|px_root_keep)\s*\(')
# 已是 GC 根的 lvalue（写进它们 = 已登记，不必再 PX_KEEP）
ROOTED_LHS_RX = re.compile(r'^(slots\s*\[|fr\s*->\s*slots\s*\[|st\s*->\s*frames|g_[A-Za-z0-9_]*$)')
RETURN_RX = re.compile(r'^\s*return\b\s*([A-Za-z_][A-Za-z0-9_\[\]\.\->]*)?')
# 「会持有实参」的构造器（新对象存下旧值 ⇒ 旧值可达）
# ⚠️ **判据（M207 定稿）：只有「先填字段、后 `gc_register`」的构造器才算持有者。**
#   · 排除 `px_func_env`（缺陷 262）：先 `gc_register` 再赋 env。
#   · 排除 `px_list_n`（缺陷 266）：它是 `px_list(n)`（**先注册 + 可能触发 GC**）
#     再 `px_list_push` 逐项入列 ⇒ 注册点那次 GC **看不到 items**。
#   · 仍在表内的都核对过源码顺序：`px_ok`/`px_err`/`px_some`（先赋 value 再注册）·
#     `px_tuple`/`px_struct`（先拷 items/fvals 再注册）· `px_cell`（先赋再注册）·
#     `px_dict_set`/`px_list_push`（写进**已注册**容器 ⇒ 可达）·
#     `px_chan_send`/`px_global_set`/`px_set_global`（入根）。
HOLD_RX = re.compile(
    r'\b(px_ok|px_err|px_some|px_tuple|px_dict_set|px_list_push|'
    r'px_struct|px_enum|px_cell|px_chan_send|px_global_set|px_set_global)\s*\(')
# lvalue 尾巴：`LXValue l` → `l`；`a[0]` → `a[0]`；`s->v` → `s->v`
LVALUE_TAIL_RX = re.compile(
    r'([A-Za-z_][A-Za-z0-9_]*(?:\s*(?:\[[^\]]*\]|\.\s*[A-Za-z_]\w*|->\s*[A-Za-z_]\w*))*)\s*$')


def strip_comments_strings(src: str) -> str:
    """去掉注释与字符串/字符字面量的**内容**，保留换行与位置（行号一一对应）。"""
    out = []
    i, n = 0, len(src)
    state = 0  # 0 代码 1 双引号 2 单引号 3 行注释 4 块注释
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
        else:  # 块注释
            if c == '*' and nxt == '/':
                state = 0; out.append(' '); i += 2; continue
            out.append('\n' if c == '\n' else ' ')
            i += 1
    return ''.join(out)


_CTRL_WORDS = {'if', 'for', 'while', 'switch', 'else', 'do', 'return', 'sizeof',
               'case', 'goto', 'break', 'continue'}
_SIG_START_RX = re.compile(
    r'[ \t]*(?:[A-Za-z_][A-Za-z0-9_]*[ \t\*]+)+([A-Za-z_][A-Za-z0-9_]*)[ \t]*\(')


def split_functions(code: str):
    """按顶层花括号切出**函数定义**体。返回 [(name, start_lineno, body_lines)]。

    ⚠️ M207：**必须支持跨行签名**。原实现用单行正则
      `name(...) {` 匹配 ⇒ 签名跨行的函数**整个体被跳过、完全不在审计面上**
      （实测漏掉 `h_exchange`（缺陷 260 所在）——它的签名有 4 行）。
      ⇒ 改为：从行首标识符起，向下累积签名行直到「括号闭合后出现的 `{`」；
        中途遇到顶层 `;` 即判定为**原型/调用/声明**并丢弃。
    """
    lines = code.split('\n')
    n = len(lines)
    funcs = []
    i = 0
    while i < n:
        m = _SIG_START_RX.match(lines[i])
        if not m or m.group(1) in _CTRL_WORDS:
            i += 1
            continue
        name = m.group(1)
        depth = 0
        sig_end = None
        dead = False
        j = i
        while j < n and (j - i) <= 16:            # 签名不超过 16 行
            for ch in lines[j]:
                if ch == '(':
                    depth += 1
                elif ch == ')':
                    depth -= 1
                elif ch == '{' and depth <= 0:
                    sig_end = j
                    break
                elif ch == ';' and depth <= 0:
                    dead = True
                    break
            if sig_end is not None or dead:
                break
            j += 1
        if sig_end is None or dead:
            i += 1
            continue
        body, bdepth, k = [], 0, sig_end
        while k < n:
            body.append(lines[k])
            bdepth += lines[k].count('{') - lines[k].count('}')
            if bdepth <= 0:
                break
            k += 1
        funcs.append((name, i + 1, body))
        i = k + 1
    return funcs


def split_statements(body_lines):
    """把函数体按 `;` 与 `{}` 切成语句。返回 [(起始相对行号, 语句文本)]。"""
    stmts, cur, start = [], [], None
    for rel, line in enumerate(body_lines):
        seg = 0
        for i, ch in enumerate(line):
            if ch in ';{}':
                frag = line[seg:i + 1]
                if not ''.join(cur).strip() and not frag.strip():
                    seg = i + 1
                    continue
                if start is None:
                    start = rel
                cur.append(frag)
                stmts.append((start, ''.join(cur)))
                cur, start, seg = [], None, i + 1
        tail = line[seg:]
        if tail.strip():
            if start is None:
                start = rel
            cur.append(tail + '\n')
        elif cur:
            cur.append('\n')
    if cur and ''.join(cur).strip():
        stmts.append((start if start is not None else 0, ''.join(cur)))
    return stmts


def find_assign(stmt: str):
    """找最外层（不在括号内）的赋值 `=`。返回 (lhs_raw, rhs) 或 (None, None)。"""
    depth = 0
    for i, ch in enumerate(stmt):
        if ch in '([{':
            depth += 1
        elif ch in ')]}':
            depth -= 1
        elif ch == '=' and depth == 0:
            prev = stmt[i - 1] if i > 0 else ''
            nxt = stmt[i + 1] if i + 1 < len(stmt) else ''
            if prev in '=!<>+-*/%&|^' or nxt == '=':
                continue
            return stmt[:i], stmt[i + 1:]
    return None, None


def call_tail_is_empty(rhs: str, alloc_match) -> bool:
    """`px_dict(...)` 这种「整个 RHS 就是一次调用」⇒ True（尾部除 ; 外无内容）。"""
    txt = rhs.strip().rstrip(';').strip()
    m = re.match(r'\s*(' + '|'.join(_ALLOC_NAMES) + r')\s*\(', txt)
    if not m:
        return False
    open_idx = txt.index('(', m.end() - 1)
    depth, end = 0, -1
    for j in range(open_idx, len(txt)):
        if txt[j] == '(':
            depth += 1
        elif txt[j] == ')':
            depth -= 1
            if depth == 0:
                end = j
                break
    if end < 0:
        return False
    return txt[end + 1:].strip() == ''


def audit_text(code: str, relpath: str):
    findings = []
    stats = {'funcs': 0, 'allocs': 0, 'keeps': 0}
    for fname, fline, body in split_functions(code):
        stats['funcs'] += 1
        live = {}  # lvalue -> 创建行号
        for rel, stmt in split_statements(body):
            abs_line = fline + rel
            text = stmt.strip()
            if not text:
                continue
            # ① 登记（先处理：登记紧跟创建）
            for m in KEEP_RX.finditer(text):
                live.pop(m.group(1), None); stats['keeps'] += 1
            for m in KEEP2_RX.finditer(text):
                live.pop(m.group(1), None); stats['keeps'] += 1
            # ② 返回
            rm = RETURN_RX.match(text)
            if rm and rm.group(1):
                live.pop(rm.group(1).rstrip(';'), None)
            # ③ 分配（按文本顺序：每一次分配都可能回收此前所有未登记活值）
            #    M208（缺陷 270）：隐式分配（px_dict_set/px_list_push 的键副本与扩容）一并计入，
            #    且**不适用**「持有实参」豁免 —— 容器本身不因此成为 GC 根。
            #    ⚠️ 该规则 **默认关闭**（`--grow` 开启）：它会一次性照出 18 处既有站点
            #    （缺陷 271 族，见 docs/GC_ROOTS.md §8），需按轮次收口；
            #    默认关闭可让 m206 门保持其原有契约（不因新增判据而假红）。
            if GROW_ON:
                for g in GROW_RX.finditer(text):
                    stats['allocs'] += 1
                    if live:
                        findings.append({
                            'file': relpath, 'line': abs_line, 'func': fname,
                            'trigger': g.group(0) + ' [隐式分配]',
                            'victims': [{'name': k, 'line': v}
                                        for k, v in sorted(live.items(), key=lambda kv: kv[1])],
                        })
                        live = {}
            for m in ALLOC_RX.finditer(text):
                stats['allocs'] += 1
                if live:
                    # 例外：受害值作为**触发调用的实参**出现 ⇒ 新建对象会持有它
                    #   （标记从 g_tmp_root=新对象 出发 ⇒ 受害值可达）⇒ 不是缺陷。
                    #   例：return px_ok(v) / px_list_n(res, 2) / px_tuple(items, n)
                    # 例外仅限**会持有实参的容器构造器**（新对象直接存下该值 ⇒ 从
                    #   g_tmp_root=新对象 出发可达）。`px_call`/`px_method` **不算**
                    #   —— 被调方不保证持有（实测：px_session_read 的 `px_call(json_parse, &v, 1)`
                    #   在压力档下就地丢参 ⇒ `json: 对象解析失败`，缺陷 254）。
                    if HOLD_RX.match(m.group(0)):
                        args_seg = text[m.end():]
                        def _held(name):
                            base = re.split(r'[\[.\-]', name)[0]
                            return bool(base) and re.search(
                                r'\b' + re.escape(base) + r'\b', args_seg)
                        held = [k for k in live if _held(k)]
                    else:
                        held = []
                    if held:
                        live = {k: v for k, v in live.items() if k in held}
                        continue
                    findings.append({
                        'file': relpath, 'line': abs_line, 'func': fname,
                        'trigger': m.group(0),
                        'victims': [{'name': k, 'line': v}
                                    for k, v in sorted(live.items(), key=lambda kv: kv[1])],
                    })
                    live = {}
            # ④ 把本次分配的结果记进活集合
            lhs_raw, rhs = find_assign(text)
            if lhs_raw is not None and rhs is not None:
                if ALLOC_RX.search(rhs) and call_tail_is_empty(rhs, None):
                    lm = LVALUE_TAIL_RX.search(lhs_raw)
                    if lm and not ROOTED_LHS_RX.match(lm.group(1).strip()):
                        live[lm.group(1).strip()] = abs_line
                elif not ALLOC_RX.search(rhs):
                    # 覆盖（RHS 本身不是分配）⇒ 旧值失效
                    lm = LVALUE_TAIL_RX.search(lhs_raw)
                    if lm:
                        live.pop(lm.group(1).strip(), None)
            # ④b 消费：受害值作为赋值右值被存进别处（`slots[dst] = r;` / `x->f = v;`）
            lhs_raw2, rhs2 = find_assign(text)
            if lhs_raw2 is not None and rhs2 is not None and live:
                if not (ALLOC_RX.search(rhs2) and call_tail_is_empty(rhs2, None)):
                    lm2 = LVALUE_TAIL_RX.search(lhs_raw2)
                    lhsname = lm2.group(1).strip() if lm2 else ''
                    for k in list(live.keys()):
                        base = re.split(r'[\[.\-]', k)[0]
                        if base and lhsname != k and re.search(
                                r'\b' + re.escape(base) + r'\b', rhs2):
                            live.pop(k, None)
            # ⑤ 消费：值被交给别的调用 ⇒ 假定接管
            if live:
                for m in CONSUME_RX.finditer(text):
                    seg = text[m.end():]
                    for k in list(live.keys()):
                        base = re.split(r'[\[.\-]', k)[0]
                        if base and re.search(r'\b' + re.escape(base) + r'\b', seg):
                            live.pop(k, None)
    return findings, stats


def audit_file(path: str):
    with open(path, 'r', encoding='utf-8', errors='replace') as fh:
        raw = fh.read()
    return audit_text(strip_comments_strings(raw), os.path.relpath(path))


# ------------------------------------------------------------
# 自证：4 锚点（2 必中 2 必不中）+ 规模下限
# ------------------------------------------------------------
FIX_HIT1 = '''
static LXValue bi_demo_raw(const char* ip, int port, const char* alpn) {
    LXValue a[3];
    a[0] = px_str(ip); a[1] = px_int(port); a[2] = px_str(alpn);
    LXValue r = bi_quic_connect(a, 3, NULL);
    return r;
}
'''
FIX_HIT2 = '''
static LXValue bi_demo_list(int n) {
    LXValue l = px_list(0);
    for (int i = 0; i < n; i++) { px_list_push(l, px_str("x")); }
    return l;
}
'''
FIX_MISS1 = '''
static LXValue bi_ok_raw(const char* ip, int port, const char* alpn) {
    LXValue a[3];
    px_root_push();
    a[0] = px_str(ip); PX_KEEP(a[0]);
    a[1] = px_int(port);
    a[2] = px_str(alpn); PX_KEEP(a[2]);
    LXValue r = bi_quic_connect(a, 3, NULL);
    px_root_pop();
    return r;
}
'''
FIX_MISS2 = '''
static LXValue bi_ok_list(int n) {
    LXValue l = px_list(0); px_root_push(); PX_KEEP(l);
    for (int i = 0; i < n; i++) { px_list_push(l, px_str("x")); }
    px_root_pop();
    return l;
}
'''


# M207 新增锚点①：**跨行签名**（证明 split_functions 的修复真的生效 ——
#   原单行正则下这个函数**整体不在审计面上**，实得「不中」⇒ 锚点必红）
FIX_HIT3 = '''
static int demo_multiline(HPool* pool, int n,
                          char* out) {
    LXValue cur = px_dict_get(pool->h, "k");
    LXValue nv = px_str(out);
    if (cur.type == PX_LIST) { px_list_push(cur, nv); }
    else {
        LXValue lst = px_list(4);
        px_list_push(lst, cur);
        px_list_push(lst, nv);
    }
    return 0;
}
'''
# M207 新增锚点②：**px_func_env 不再担豁免**（缺陷 262 的形状 —— MKCLO 的 env）
FIX_HIT4 = '''
static int demo_funcenv(VM* m, int fidx, LXValue* slots, int nslots) {
    const PxVMFunc* nf = &m->funcs[fidx];
    LXValue env = px_dict();
    px_dict_set(env, "k", px_int(1));
    slots[0] = px_func_env(nf->name, NULL, env);
    return 0;
}
'''
# M207 新增锚点③：跨行签名 + **已登记** ⇒ 必须不中（防「见到跨行签名就乱报」）
FIX_MISS3 = '''
static int demo_multiline_ok(HPool* pool, int n,
                             char* out) {
    LXValue nv = px_str(out);
    px_root_push(); PX_KEEP(nv);
    LXValue lst = px_list(4);
    px_list_push(lst, nv);
    px_root_pop();
    return 0;
}
'''

# M208（缺陷 270）新增锚点：**`px_list_push` / `px_dict_set` 是隐式分配点** ⇒
#   `LXValue lst = px_list(4); px_list_push(lst, nv);` 里的 **lst** 未登记 ⇒ 命中
#   （上一条 FIX_MISS3 因此从「不中」改判「命中」—— 它原本只盯着 nv，漏了 lst）。
FIX_HIT6 = '''
static int demo_grow_victim(HPool* pool, int n, char* out) {
    LXValue nv = px_str(out);
    px_root_push_keep(nv);
    LXValue lst = px_list(4);
    px_list_push(lst, nv);
    px_root_pop();
    return 0;
}
'''

# M208 反向锚点：同形状但用**原子** `px_root_push_keep(lst)` ⇒ 不中
FIX_MISS4 = '''
static int demo_grow_ok(HPool* pool, int n, char* out) {
    LXValue nv = px_str(out);
    px_root_push_keep(nv);
    LXValue lst = px_list(4);
    px_root_push_keep(lst);
    px_list_push(lst, nv);
    px_root_pop();
    px_root_pop();
    return 0;
}
'''


# M207 新增锚点④：**`px_list_n` 不是「持有实参的构造器」**（缺陷 266 的形状）——
#   它内部先 `px_list(n)`（注册 + 可能触发 GC）再逐项入列 ⇒ 注册点看不到 items。
FIX_HIT5 = '''
static LXValue bi_demo_capture(const char* cmd, int n) {
    LXValue res[2];
    res[0] = px_int((int64_t)n);
    res[1] = px_str_len(cmd, 3);
    xfree(buf);
    return px_list_n(res, 2);
}
'''


def self_test():
    global GROW_ON
    GROW_ON = True   # M208：自证覆盖**全部**规则（含隐式分配规则 hit6/miss3/miss4）
    import tempfile
    cases = [('hit1', FIX_HIT1, True), ('hit2', FIX_HIT2, True),
             ('hit3', FIX_HIT3, True), ('hit4', FIX_HIT4, True),
             ('hit5', FIX_HIT5, True), ('hit6', FIX_HIT6, True),
             ('miss1', FIX_MISS1, False), ('miss2', FIX_MISS2, False),
             ('miss3', FIX_MISS3, True), ('miss4', FIX_MISS4, False)]
    ok = fail = 0
    with tempfile.TemporaryDirectory() as td:
        for name, body, expect_hit in cases:
            p = os.path.join(td, name + '.c')
            with open(p, 'w') as fh:
                fh.write(body)
            f, _ = audit_file(p)
            got = len(f) > 0
            good = (got == expect_hit)
            ok, fail = (ok + 1, fail) if good else (ok, fail + 1)
            print('  %-6s 期望%s 实得%s  命中=%d  %s'
                  % (name, '命中' if expect_hit else '不中',
                     '命中' if got else '不中', len(f), '✅' if good else '❌'))
    print('self-test: %d 通过 / %d 失败' % (ok, fail))
    return 0 if fail == 0 else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--files', nargs='*', default=None)
    ap.add_argument('--json', action='store_true')
    ap.add_argument('--show-victims', action='store_true')
    ap.add_argument('--self-test', action='store_true')
    ap.add_argument('--grow', action='store_true',
                    help='M208：把 px_dict_set/px_list_push 计入分配点（照出缺陷 271 族；默认关）')
    ap.add_argument('--min-funcs', type=int, default=1,
                    help='下限锚点：函数数少于该值即判红（防扫描器静默失效）')
    args = ap.parse_args()
    if args.self_test:
        return self_test()

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    global GROW_ON
    GROW_ON = bool(args.grow)
    files = args.files
    if not files:
        rtdir = os.path.join(root, 'runtime')
        files = sorted(os.path.join(rtdir, f) for f in os.listdir(rtdir)
                       if f.endswith('.c'))

    allf, tot = [], {'funcs': 0, 'allocs': 0, 'keeps': 0}
    for p in files:
        f, st = audit_file(p)
        for k in ('funcs', 'allocs', 'keeps'):
            tot[k] += st[k]
        allf.extend(f)

    if args.json:
        print(json.dumps({'findings': allf, 'stats': tot}, ensure_ascii=False, indent=2))
        return 0

    print('扫描 %d 文件 · 函数 %d · 分配站点 %d · 登记站点 %d · **候选 %d**'
          % (len(files), tot['funcs'], tot['allocs'], tot['keeps'], len(allf)))
    by_file = {}
    for f in allf:
        by_file.setdefault(f['file'], []).append(f)
    for fn in sorted(by_file, key=lambda k: -len(by_file[k])):
        print('\n%s  (%d)' % (fn, len(by_file[fn])))
        for f in by_file[fn]:
            line = '  %5d %-30s ← %s' % (f['line'], f['func'], f['trigger'])
            if args.show_victims:
                line += '   受害者: ' + ', '.join(
                    '%s@%d' % (v['name'], v['line']) for v in f['victims'])
            print(line)
    if tot['funcs'] < args.min_funcs:
        print('\n❌ 下限锚点失败：函数数 %d < %d（扫描器可能静默失效）'
              % (tot['funcs'], args.min_funcs), file=sys.stderr)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main())
