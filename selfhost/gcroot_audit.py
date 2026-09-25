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
# 判据（M170 的两条硬约束 · M209 精确化）：
#   ① 容器/对象创建后必须登记；
#   ② 登记必须**紧跟创建、先于下一次分配**（因为下一次分配就可能触发 GC）。
#   ⇒ 形式化：若「未登记的活值集合」非空时又发生一次**触发点** ⇒ 该值可能被回收。
#   ⚠️ M209（缺陷 279 更正）：**「触发点」= 调用链上会 `gc_register` 的入口**（构造器集合）。
#      `px_dict_set` / `px_list_push` **不是**触发点 —— 它们只走裸分配（`m128_strdup` /
#      `m128_alloc`），不判阈值、不置 `g_gc_pending`；且它们被调用时受害者恰是「本线程
#      最近登记的对象」⇒ 受 `g_tmp_root` 保护。M208 把它们当触发点是**前提错位**
#      （报出的 11 条全是假阳）。详见 `docs/GC_ROOTS.md` §9。
#   三条排除规则（M209 缺陷 280/281/282，把 M206 的**人工判定**下沉为判据）：
#      · 覆盖：触发点语句正在写同名直接局部、且右值未读旧值 ⇒ 旧值已死；
#      · 作用域：受害者的**声明块**在触发点之前已闭合 ⇒ 读不到（按**块身份**，不按括号深度）；
#      · deref 交棒：`*outv = …` 交给调用方（与 `return` 同口径）。
#   反向判据（防「改判 = 把工具改瞎」）：self-test 里的 hit6b / hit7 / hit8 三条。
#
# 用法：
#   python3 selfhost/gcroot_audit.py [--files f1 f2 ...] [--json] [--show-victims]
#   python3 selfhost/gcroot_audit.py --self-test        # 自证（16 锚点 = 8 必中 + 8 必不中）
#   python3 selfhost/gcroot_audit.py --grow             # **旧规则对照**（复现 M208 的 11 条）
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
# ---- M212（第 91 轮 · 缺陷 288）：**触发点必须源码派生** ----
#   上面 `_ALLOC_NAMES` 是 M206 手抄的一份构造器名单（27 个）。M209 已把判据定成
#   「触发点 = 调用链上会 `gc_register` 的入口」⇒ 名单**必须从源码派生**。
#   实测：源码派生的闭包 = **329** 个，手抄只覆盖 27 ⇒ 手抄集合**漏掉 300+ 个分配入口**
#   （例：`px_call` 之外的 `px_session_read` / `px_s3_exec` / `h3_send_fields` /
#     `bi_*` 整族）⇒ 这正是「M209 遗留：漏报比假阳危险」的量化答案。
#   ⇒ 现在：`TRIGGER_NAMES`（会发生分配）与 `PRODUCER_NAMES`（结果确实是新 LXValue）
#     由 `selfhost/gcroot_derive.py` 在 `main()` 里派生后注入；`_ALLOC_NAMES` 降为
#     **手抄集合的原样留证**（`--trigger-set legacy` 时才用，用于 A/B 与留证）。
#   ⚠️ 为什么两者必须分开：`px_s3_exec` 返回 `int`、`px_parse_multipart` 返回 `void`
#     —— 它们**会分配**（是 TRIGGER），但**结果不是对象**；若当 PRODUCER，
#     ④「把结果记进活集合」会把 `int st` 记成「未登记的 GC 受害者」⇒ 4 条假阳（实测）。
TRIGGER_NAMES = list(_ALLOC_NAMES)
PRODUCER_NAMES = list(_ALLOC_NAMES)
ALLOC_RX = re.compile(r'\b(' + '|'.join(TRIGGER_NAMES) + r')\s*\(')
PRODUCER_RX = re.compile(r'\b(' + '|'.join(PRODUCER_NAMES) + r')\s*\(')


def set_trigger_sets(triggers, producers):
    """M212：注入源码派生的触发点/生产者集合（重建两个正则）。"""
    global TRIGGER_NAMES, PRODUCER_NAMES, ALLOC_RX, PRODUCER_RX
    TRIGGER_NAMES = sorted(set(triggers))
    PRODUCER_NAMES = sorted(set(producers))
    ALLOC_RX = re.compile(r'\b(' + '|'.join(TRIGGER_NAMES) + r')\s*\(')
    PRODUCER_RX = re.compile(r'\b(' + '|'.join(PRODUCER_NAMES) + r')\s*\(')

# ---- M213（第 92 轮 · 缺陷 294）：**出口参数式构造函数**（OUT-PRODUCER）----
#   形状：`static int px_as_list(LXValue v, LXValue* out)` —— 返回 int/bool，
#   但通过 `LXValue* out` **输出一个新对象**（tuple / 生成器 ⇒ 新建 list）。
#   ⇒ 调用点 `px_as_list(args[1], &xs)` 之后，`xs` 是**未登记的活值**；而审计器此前
#     只认「赋值给 lvalue 的 PRODUCER」⇒ 这类出口**看不见** ⇒ **整族漏报**。
#   实测（M213 侦察）：撤掉 `bi_join` 的 `PX_KEEP(xs)` 后审计器**报不出** —— 而
#     M207 缺陷 263 正是这条路上的真缺陷，当时是**动态压力筛**抓的，不是静态判据。
#   ⚠️ 与 PRODUCER 的分工：PRODUCER 管「返回 LXValue 的函数」；本表管「经指针输出的」。
#     两者**互斥**（`px_as_list` 返回 int ⇒ 不在 PRODUCER 里，`find_assign` 看不到它）。
OUT_PRODUCER_NAMES = []
OUT_MAP = {}                                  # {函数名: {形参名: 是否 LXValue* 输出参数}}
OUT_RX = re.compile(r'(?!)')                  # 空集合 ⇒ 永不匹配（派生注入后重建）


def set_out_producers(m):
    """M213：注入源码派生的 OUT-PRODUCER 表（**保持签名顺序** ⇒ 调用点按位置配对）。"""
    global OUT_PRODUCER_NAMES, OUT_MAP, OUT_RX
    OUT_MAP = {k: dict(v) for k, v in m.items()}
    OUT_PRODUCER_NAMES = sorted(OUT_MAP)
    if OUT_PRODUCER_NAMES:
        OUT_RX = re.compile(r'\b(' + '|'.join(OUT_PRODUCER_NAMES) + r')\s*\(')
    else:
        OUT_RX = re.compile(r'(?!)')


def split_args(seg):
    """从**调用名 `(` 之后**的文本取顶层实参列表。

    `seg` 例（= `text[m.end():]`，而 `m` 的正则**已含 `(`**）：
      `args[1], &xs)) { px_root_pop(); …` ⇒ `['args[1]', ' &xs']`
      `)`（无实参）                        ⇒ `[]`
    括号不配对 ⇒ `[]`。

    ⚠️ 首版把 `seg` 当成「**以 `(` 开头**」的文本处理（先 `seg.find('(')` 再取平衡括号）
      —— 而调用方传的正是 `(` **之后**的文本 ⇒ 实参里没有额外的 `(` 时就 `find` 落空
      ⇒ 恒返回 `[]` ⇒ **OUT-PRODUCER 规则整条静默失效**。
      （实测：self-test 新增锚点 **hit9 / hit10 双双判红**抓回 —— 这就是「反向判据」
        的价值：判据自己失效时必须有人喊。）
    """
    depth, end = 0, -1
    for i, ch in enumerate(seg):
        if ch == '(':
            depth += 1
        elif ch == ')':
            if depth == 0:
                end = i
                break
            depth -= 1
    if end < 0:
        return []
    parts, cur, d = [], [], 0
    for ch in seg[:end]:
        if ch in '([{':
            d += 1
        elif ch in ')]}':
            d -= 1
        if ch == ',' and d == 0:
            parts.append(''.join(cur))
            cur = []
        else:
            cur.append(ch)
    if ''.join(cur).strip():
        parts.append(''.join(cur))
    return parts

GROW_ON = False   # M208 遗留名（勿用）：M209 起改叫 LEGACY_GROW —— 见下
LEGACY_GROW = False   # M209：仅用于**复现旧规则**（把 push/dict_set 当触发点 ⇒ 19 候选）。

# M213（缺陷 297）**F1「被调方登记了实参」**（跨函数持有者，M212 §10.6 缺口①）：
#   调用点的活值作为实参传进被调方，而被调方**入口**就 `PX_KEEP(形参)`
#   （且先于其第一个触发点）⇒ 进入被调方后在任何分配之前即被登记 ⇒ 调用点无风险。
CALLEE_KEEPS = {}
CALLEE_KEEP_ON = True


def set_callee_keeps(m):
    global CALLEE_KEEPS
    CALLEE_KEEPS = dict(m or {})


def _arg_kept_by_callee(callee, victim, cargs):
    """调用点的 `victim` 是否落在被调方**入口已登记**的形参位置上（**按位置配对**）。"""
    if not CALLEE_KEEP_ON:
        return False
    info = CALLEE_KEEPS.get(callee)
    if not info:
        return False
    ps = info.get('params') or []
    kept = info.get('kept') or []
    for i, a in enumerate(cargs):
        if a.strip() == victim and i < len(ps) and ps[i] in kept:
            return True
    return False
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
# 声明语句（M209 · 作用域排除要用**声明块的身份**，不是深度）：
#   `LXValue v;` / `LXValue v = …;` / `const LXValue v = …;` / `LXValue a[3];`
DECL_RX = re.compile(r'^\s*(?:const\s+)?(LXValue|LXObject|PxVMFunc)\s*\*?\s*'
                     r'([A-Za-z_][A-Za-z0-9_]*)')
# lvalue 尾巴：`LXValue l` → `l`；`a[0]` → `a[0]`；`s->v` → `s->v`
LVALUE_TAIL_RX = re.compile(
    r'([A-Za-z_][A-Za-z0-9_]*(?:\s*(?:\[[^\]]*\]|\.\s*[A-Za-z_]\w*|->\s*[A-Za-z_]\w*))*)\s*$')
# ---- M212（缺陷 290）：**登记动作**本身 ⇒ 被登记对象成为 GC 根 ----
#   形态：`o->as.gen.list = px_list(0);` … `gc_register(o, sizeof(LXObject));`
#   在 `gc_register` 那一刻，`o` 进 `g_tmp_root` 且进 `g_objs`（runtime.c:3049 → :3051）
#   ⇒ 标记阶段扫 `o` 的全部字段 ⇒ `o->…` 里的子对象**可达** ⇒ 不会被回收。
#   ⇒ 判据：触发点是登记动作、且受害者的**基名**就是被登记的那个名字 ⇒ 豁免。
#   （实测：不加这条会把 `px_gen_lazy`/`px_gen_from_list` 判成候选 —— 而 M207 缺陷 264
#     的修复**正是**「字段填齐之后再注册」这个形态。）
_REGISTER_RX = re.compile(
    r'\b(gc_register|PX_KEEP|px_root_keep|px_root_push_keep)\s*\(')


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


def brace_depth_map(code: str):
    """⚠️ M209：已被 `audit_text` 内的**逐语句**深度计数取代（行级计数无法识别
    「声明与闭合brace同一行」的块，实测漏掉 miss7 那族）。保留仅为留证/对照，勿用。"""
    before, after, d = [], [], 0
    for ln in code.split('\n'):
        before.append(d)
        d += ln.count('{') - ln.count('}')
        after.append(d)
    return before, after


def audit_text(code: str, relpath: str):
    findings = []
    stats = {'funcs': 0, 'allocs': 0, 'keeps': 0}
    for fname, fline, body in split_functions(code):
        stats['funcs'] += 1
        live = {}        # lvalue -> (创建行号, **声明块 id**)
        decl_block = {}  # 基名 -> 声明块 id（`LXValue x` 语句所在块）
        stack = [0]      # 打开的块栈（id；0 = 函数体根）
        nxt = 1
        for rel, stmt in split_statements(body):
            abs_line = fline + rel
            text = stmt.strip()
            # M209（缺陷 281）· **作用域排除**：块栈按**身份**推进（每语句至多一个 `{`/`}`，
            #   见 `split_statements` 的实现 —— 它总在分隔符处切）。
            #   ⚠️ 为什么必须是「块身份」而不是「括号深度」：宏体（`#define STREAM_FLUSH()`
            #   的 `do { … } while (0)`）在**源码里**位于较浅的深度，而其**调用点**在更深的
            #   分支里 ⇒ 「深度变小」永远不成立，宏体局部（`rv`）会被误当成活值（实测）。
            #   块身份则精确：宏体的块在 `} while (0)` 处出栈 ⇒ 其局部随之出局。
            if text:
                last = text[-1]
                if last == '{':
                    stack.append(nxt); nxt += 1
                elif last == '}' and len(stack) > 1:
                    stack.pop()
            dm = DECL_RX.match(text)
            if dm:
                decl_block[dm.group(2)] = stack[-1]
            # 剪枝放在**任何 continue 之前**（否则会被跳过）
            live = {k: v for k, v in live.items() if v[1] in stack}
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
            #    ⚠️ M209 更正（缺陷 279）：**`px_dict_set` / `px_list_push` 不是触发点。**
            #      M208（缺陷 270）把它们当「隐式分配」计入，依据是「键副本 `m128_strdup` /
            #      条目数组 `m128_alloc` 会分配内存」——**分配内存 ≠ 触发 GC**：
            #      full GC 的**唯一**触发点是 `gc_register`（runtime.c:3048 `g_alloc_bytes += est;`
            #      → :3051 判阈值 → `px_gc_collect()` 或 `g_gc_pending = 1`，并在 :3049 置
            #      `g_tmp_root = o` 保护刚建对象）。这两个函数只走 slab/mmap 裸分配
            #      （`m128_alloc` / `m128_strdup` → `xmalloc`/`xrealloc`）⇒ **不会触发 GC**。
            #      它们内部确有安全点（`gc_unblock_stop` → `gc_pause_if_requested`，
            #      仅当**另一个线程的 GC 已在跑**时才暂停本线程），但那一刻被保护的面 = 本线程
            #      `ti→tmp_root`（快照自 `g_tmp_root` = **本线程最近登记的对象**，runtime.c:1973/2635）;
            #      而本审计器报告的每个候选，受害者**恰是「创建后的第一个触发点」**上的对象
            #      ⇒ 若该触发点是 push/dict_set，受害者就是最近登记对象 ⇒ 被 tmp_root 护住 ⇒ 安全。
            #      ⇒ 真判据：**触发点 = 构造器**（会 `gc_register` 的入口 = `_ALLOC_NAMES`）。
            #      `--grow` 保留为**旧规则对照**（复现 M208 的 19 候选，见 docs/GC_ROOTS.md §9）。
            if LEGACY_GROW:
                for g in GROW_RX.finditer(text):
                    stats['allocs'] += 1
                    if live:
                        findings.append({
                            'file': relpath, 'line': abs_line, 'func': fname,
                            'trigger': g.group(0) + ' [隐式分配·旧规则]',
                            'trigger_kind': 'legacy',
                            'victims': [{'name': k, 'line': v[0]}
                                        for k, v in sorted(live.items(), key=lambda kv: kv[1][0])],
                        })
                        live = {}
            # M209（缺陷 280）：触发点语句**正在写同名直接局部** ⇒ 该名字的旧值此刻被覆盖
            #   （旧值随即死亡）⇒ 不计为受害者。判据要求「同名且**未**在右值里出现」
            #   —— 右值若读了旧值（`v = f(v)`），旧值在求值期间仍活，不能排除。
            #   实测形态（0.2.0-m208s1 全仓 8 条已判定基线里的 2 条）：
            #     `runtime_h3_qpack.c:350 name = px_str_len(...)`（兄弟支在 :342 赋过 name）
            #     `runtime_sqlite.c:209 v = px_bytes_len(...)`（兄弟 case 在 :203 赋过 v）
            _lhs_o, _rhs_o = find_assign(text)
            overwrite = None
            if _lhs_o is not None and _rhs_o is not None:
                _lhs_s = _lhs_o.strip()
                if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', _lhs_s) and not re.search(
                        r'\b' + re.escape(_lhs_s) + r'\b', _rhs_o):
                    overwrite = _lhs_s
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
                    # M212（缺陷 290）：见 `_REGISTER_RX` 处的说明 —— 登记动作把被登记
                    #   对象变成根 ⇒ 它的字段/元素从此可达 ⇒ 一并豁免。
                    _rm = _REGISTER_RX.match(m.group(0))
                    if _rm:
                        _m3 = re.match(r'\s*&?\s*([A-Za-z_][A-Za-z0-9_]*)',
                                       text[m.end():])
                        if _m3:
                            _rn = _m3.group(1)
                            held += [k for k in live
                                     if re.split(r'[\[.\-]', k)[0] == _rn]
                    if held:
                        live = {k: v for k, v in live.items() if k in held}
                        continue
                    # M209（缺陷 280/281）：把 M206 的**人工判定**下沉为**判据**。
                    #   ① 覆盖排除（overwrite）：见上一步。
                    #   ② 作用域排除（out_of_scope）：受害者的**声明块**在触发点之前已闭合 ⇒
                    #      它根本不可能在触发点被读到 ⇒ 不计。这一条同时正确地处理两族：
                    #      · 名字在**兄弟块**里被复用（`name`/`v`）；· **宏体**里的局部 vs
                    #        宏的**调用点**（`STREAM_FLUSH` 的 `rv` 声明在宏体的 if 块里，
                    #        触发点在函数后段的另一处 `px_str_len`）。
                    #      M206 的 BASELINE.tsv 里有 5 条正是这两族（人工理由写着「死值」
                    #      「互斥分支」「触发点是下一轮迭代」）。
                    #   实测：加上这两条后，全仓候选 8 → 0（旧规则的 19 → 0）。
                    vs = [{'name': k, 'line': v[0]}
                          for k, v in sorted(live.items(), key=lambda kv: kv[1][0])
                          if k != overwrite]
                    # M213（缺陷 297）F1：被调方入口已登记该实参 ⇒ 调用点不构成风险
                    if vs:
                        _cal = m.group(0).strip().rstrip('(').strip()
                        _cargs = split_args(text[m.end():])
                        _drop = [v for v in vs
                                 if _arg_kept_by_callee(_cal, v['name'], _cargs)]
                        if _drop:
                            stats['callee_keep'] = stats.get('callee_keep', 0) + len(_drop)
                            vs = [v for v in vs if v not in _drop]
                    if vs:
                        findings.append({
                            'file': relpath, 'line': abs_line, 'func': fname,
                            'trigger': m.group(0),
                            'trigger_kind': _trigger_kind(m.group(0)),
                            'victims': vs,
                        })
                    live = {}
            # ④ 把本次分配的结果记进活集合
            lhs_raw, rhs = find_assign(text)
            if lhs_raw is not None and rhs is not None:
                # M212：记「活值」只认 **PRODUCER**（结果确实是新对象）——
                #   TRIGGER 里那 40+ 个返回 int/void 的函数不能把 `int st` 记成活值。
                if PRODUCER_RX.search(rhs) and call_tail_is_empty(rhs, None):
                    lm = LVALUE_TAIL_RX.search(lhs_raw)
                    # M209（缺陷 282）：**写通过指针**（`*outv = px_str_len(...)`）是**交棒出帧**
                    #   —— 值交给调用方保管（与 `return` 同口径）。实例：
                    #   `runtime_h3_qpack.c:262 qp_dec_value_string` 的 `*outv = px_str_len(...)`，
                    #   调用方 `px_h3_qdec` 在返回后**紧跟** `PX_KEEP(val)`（:359）。
                    #   不排除它会把「outv」当成 C 局部受害者，进而在**兄弟支**的 :272 报假阳
                    #   （M206 基线里那条「两支互斥且第一支 return」）。
                    if lm and not ROOTED_LHS_RX.match(lm.group(1).strip()) \
                            and not lhs_raw.strip().startswith('*'):
                        key = lm.group(1).strip()
                        base = re.split(r'[\[.\-]', key)[0]
                        # 作用域 = **声明**所在块（不是赋值语句所在块）—— 赋给外层已声明
                        #   变量（`v = px_str_len(…)` 在 switch 的 case 块里，而 `LXValue v;`
                        #   在外层）时，用赋值处的块会误剪 ⇒ **漏报**（违反审计器的本分）。
                        live[key] = (abs_line, decl_block.get(base, stack[-1]))
                elif not PRODUCER_RX.search(rhs):
                    # 覆盖（RHS 本身不是分配）⇒ 旧值失效
                    lm = LVALUE_TAIL_RX.search(lhs_raw)
                    if lm:
                        live.pop(lm.group(1).strip(), None)
            # ④c M213（缺陷 294）：**出口参数式构造函数** —— `f(…, &out)`
            #   把**新对象**写进 `out`（例：`px_as_list(args[1], &xs)` 的 tuple/生成器支
            #   会 `px_list(n)` 新建 list）⇒ `out` 是未登记的活值，必须记进 live。
            #   这类调用的返回类型是 int/bool ⇒ 上面 `find_assign` 的 PRODUCER 分支
            #   看不到它（这正是「整族漏报」的机制）。
            #   ⚠️ **按位置配对**：只有形参类型是 `LXValue*` 的那个实参才是对象输出。
            #     `h_exchange(pool, req, rlen, &slot, &status, …)` 里 `slot` 是
            #     `HPoolSlot**` ⇒ 不是 GC 对象。首版把所有 `&x` 都记 ⇒ 8 条假阳（实测）。
            if OUT_RX.search(text):
                for _om in OUT_RX.finditer(text):
                    _fn = _om.group(1)
                    _pm = OUT_MAP.get(_fn) or {}
                    _names = list(_pm)
                    for _i, _part in enumerate(split_args(text[_om.end():])):
                        if _i >= len(_names):
                            break
                        if not _pm.get(_names[_i]):
                            continue
                        _m2 = re.match(r'\s*&\s*([A-Za-z_][A-Za-z0-9_]*)', _part)
                        if _m2 and not ROOTED_LHS_RX.match(_m2.group(1)):
                            _on = _m2.group(1)
                            live[_on] = (abs_line, decl_block.get(_on, stack[-1]))
            # ④b 消费：受害值作为赋值右值被存进别处（`slots[dst] = r;` / `x->f = v;`）
            lhs_raw2, rhs2 = find_assign(text)
            if lhs_raw2 is not None and rhs2 is not None and live:
                if not (PRODUCER_RX.search(rhs2) and call_tail_is_empty(rhs2, None)):
                    lm2 = LVALUE_TAIL_RX.search(lhs_raw2)
                    lhsname = lm2.group(1).strip() if lm2 else ''
                    # ⚠️ M213（缺陷 294-b · **判据实测抓到的第二处漏报**）：
                    #   **成员取址式别名不是「消费」**。`LXObject* o = xs.as.obj;`
                    #   只是**读** xs 的内部指针（`o` 指向 xs 的那个对象）——xs 若被回收，
                    #   `o` 立刻悬垂 ⇒ xs **必须继续活着**。把它当消费 ⇒
                    #   「先物化（`px_as_list(…, &xs)`）、再取对象指针、再用」这一族
                    #   **整族漏报**（实测：`bi_join` 的 `LXObject* o = xs.as.obj;` ⇒
                    #   撤掉 `PX_KEEP(xs)` 后审计器**报不出**；而 M207 缺陷 263 正在此处）。
                    #   处置：rhs 是「纯成员链」（`n.f` / `n->f` / `n.f.g`）⇒ 记**别名**
                    #   （lhs 也进 live ⇒ 两者都盯着），不弹原值。
                    #   ⚠️ 尾部必须容许 `;` —— `find_assign` 返回的 rhs **含语句尾分号**
                    #     （实测：`xs.as.obj;` 在首版正则下 fullmatch=False ⇒ 整条规则
                    #      静默失效 ⇒ `bi_join` 仍报不出；由 fixture D/E/F 三分位定位）。
                    alias_only = bool(re.fullmatch(
                        r'\s*&?\s*[A-Za-z_][A-Za-z0-9_]*'
                        r'(?:\s*(?:\.|->)\s*[A-Za-z_][A-Za-z0-9_]*)+'
                        r'(?:\s*\[[^\]]*\])?\s*;?\s*', rhs2))
                    for k in list(live.keys()):
                        base = re.split(r'[\[.\-]', k)[0]
                        if base and lhsname != k and re.search(
                                r'\b' + re.escape(base) + r'\b', rhs2):
                            if alias_only and lhsname:
                                live[lhsname] = live[k]
                            else:
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

# M208（缺陷 270）曾新增锚点 hit6：**`px_list_push` 是隐式分配点** ⇒
#   `LXValue lst = px_list(4); px_list_push(lst, nv);` 里的 **lst** 未登记 ⇒ 命中。
# ⚠️ M209（缺陷 279）**改判**：该前提被证伪（push/dict_set 无 `gc_register` ⇒ 不触发 GC；
#   且那一刻 `g_tmp_root` 仍指向 lst ⇒ 被护住）。⇒ 同一份 fixture 现在**必须不中**
#   （锚点名从 hit6 变 **miss5**，见下），同时用 hit6b 证明「插一次真构造器仍然命中」——
#   这样「改判」就没有把工具改瞎。
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

# ==================== M209 新增锚点（缺陷 279/280/281/282） ====================
# hit6b（规则 1 的**反向**判据）：同形状，但在 push 之后**插一次真构造器** ⇒
#   `g_tmp_root` 移开、lst 未登记 ⇒ 必**中**。若这条不中，说明规则 1 把工具改瞎了。
FIX_HIT6B = '''
static int demo_grow_then_ctor(HPool* pool, int n, char* out) {
    LXValue nv = px_str(out);
    px_root_push_keep(nv);
    LXValue lst = px_list(4);
    px_list_push(lst, nv);
    LXValue m = px_str("y");
    px_root_pop();
    return 0;
}
'''
# hit7（规则 3 的反向判据）：受害者声明在**函数体**（作用域覆盖到函数尾）⇒ 必**中**。
FIX_HIT7 = '''
static int demo_scope_hit(HPool* pool, int n, char* out) {
    LXValue nv = px_str(out);
    if (n > 0) { }
    LXValue m = px_str("y");
    return 0;
}
'''
# hit8（规则 2 的反向判据）：右值**读了**旧值（`v = …v…`）⇒ 覆盖排除**不得**生效 ⇒ 必**中**。
FIX_HIT8 = '''
static int demo_overwrite_read(HPool* pool, int n, char* out) {
    LXValue v = px_str(out);
    v = px_str_concat(v, px_str("y"));
    return 0;
}
'''
# miss6（规则 2 · 覆盖排除）：`v = px_bytes_len(…)` 的旧值 v 此刻被覆盖且右值未读它 ⇒ 不中。
#   形状取自 runtime_sqlite.c:203/209（switch 两 case 各赋 v）。
FIX_MISS6 = '''
static int demo_overwrite_ok(sqlite3_stmt* stmt, int i) {
    LXValue v;
    v = px_str_len("a", 1);
    v = px_bytes_len("b", 1);
    return 0;
}
'''
# miss7（规则 3 · 作用域排除）：受害者在**兄弟块**里，声明块在触发点前即闭合 ⇒ 不中。
FIX_MISS7 = '''
static int demo_scope_ok(HPool* pool, int n, char* out) {
    { LXValue v = px_str(out); }
    LXValue m = px_str("y");
    return 0;
}
'''
# miss8（规则 4 · deref 交棒）：`*outv = px_str_len(…)` 是交棒出帧 ⇒ 不计 C 局部活值 ⇒ 不中。
#   形状取自 runtime_h3_qpack.c:262（调用方 :359 紧跟 PX_KEEP(val)）。
FIX_MISS8 = '''
static int demo_deref_ok(const char* t, int l, LXValue* outv) {
    *outv = px_str_len(t, l);
    LXValue m = px_str_len(t, 1);
    return 0;
}
'''

# ==================== M213 新增锚点（缺陷 294） ====================
# hit9（**正向** · OUT-PRODUCER）：`px_as_list(args[1], &xs)` 把**新对象**写进 `xs`
#   （tuple/生成器支 ⇒ `px_list(n)`）⇒ `xs` 是未登记的活值，后续真触发点必须报出它。
#   ⚠️ 若 OUT-PRODUCER 规则失效 ⇒ `xs` 不入 live ⇒ 本锚点**判红**（锚点自己有牙）。
FIX_HIT9 = '''
static int demo_out_producer(LXValue* args, int nargs, void* ctx) {
    LXValue xs;
    px_root_push();
    if (!px_as_list(args[1], &xs)) { px_root_pop(); return 0; }
    char* out = xmalloc(8);
    LXValue rv = px_str_len(out, 8);
    xfree(out);
    px_root_pop();
    return 0;
}
'''
# hit10（**反向判据** · 成员取址别名**不是**消费）：`LXObject* o = xs.as.obj;` 只读指针
#   ⇒ `xs` **必须继续活着**（否则 o 悬垂）。若把该赋值当「消费」⇒ 漏报 ⇒ 本锚点判红。
#   形状取自 `bi_join`（M207 缺陷 263 的现场）。
FIX_HIT10 = '''
static int demo_alias_alive(LXValue* args, int nargs, void* ctx) {
    LXValue xs;
    px_root_push();
    if (!px_as_list(args[1], &xs)) { px_root_pop(); return 0; }
    LXObject* o = xs.as.obj;
    int n = o->as.list.len;
    LXValue rv = px_str_len("x", n);
    px_root_pop();
    return 0;
}
'''
# miss9（**反向判据** · 按位置配对）：OUT-PRODUCER 里**非 `LXValue*`** 的实参位
#   （`&cnt` 对应 `int* cnt`）**不是** GC 对象 ⇒ 不得记进 live ⇒ 不中。
#   若不做位置配对（把所有 `&x` 都记）⇒ 这里会把 `cnt` 记成活值 ⇒ 误报出的受害者
#   与真缺陷无关（首版实测 8 条假阳，如 `h_exchange(…, &slot, &status, …)`）。
FIX_MISS9 = '''
static int demo_pos_match(HPoolSlot* slot, int* cnt) {
    mix2(slot, cnt);
    LXValue rv = px_str_len("x", 1);
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
    # M209：自证覆盖**新规则**（默认档 = 触发点仅构造器 + 覆盖/作用域/deref 三条排除）。
    #   8 必中 + 8 必不中 = 16 锚点；其中 hit6b/hit7/hit8 是**反向判据**（证明排除规则没把
    #   工具改瞎），miss5 是 M208 那条 hit6 的**改判**（缺陷 279 的预期翻转）。
    import tempfile
    # ---- M213：自证时必须**手工注入** OUT-PRODUCER 表 ----
    #   理由：真实表是从 `runtime/*.c` 派生的，而自证跑的是内嵌 fixture
    #   ⇒ 不注入则 `px_as_list` 不被识别 ⇒ hit9/hit10 的**正向面**无从检验。
    set_out_producers({
        'px_as_list': {'v': False, 'out': True},
        # 位置配对的反例：两个形参都**不是** `LXValue*` ⇒ 不得记任何活值
        'mix2': {'slot': False, 'cnt': False},
    })
    cases = [('hit1', FIX_HIT1, True), ('hit2', FIX_HIT2, True),
             ('hit3', FIX_HIT3, True), ('hit4', FIX_HIT4, True),
             ('hit5', FIX_HIT5, True), ('hit6b', FIX_HIT6B, True),
             ('hit7', FIX_HIT7, True), ('hit8', FIX_HIT8, True),
             ('hit9', FIX_HIT9, True), ('hit10', FIX_HIT10, True),
             ('miss1', FIX_MISS1, False), ('miss2', FIX_MISS2, False),
             ('miss3', FIX_MISS3, False), ('miss4', FIX_MISS4, False),
             ('miss5', FIX_HIT6, False), ('miss6', FIX_MISS6, False),
             ('miss7', FIX_MISS7, False), ('miss8', FIX_MISS8, False),
             ('miss9', FIX_MISS9, False)]
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


# M213（缺陷 295）**分诊**：精确档闭包（`--indirect tight` 的 reach）。
#   触发点若**不在**此集合 ⇒ 它是「保守兜底」（过近似）引入的 ⇒ 候选大概率是噪音，
#   但**不能据此判假阳**（漏报比假阳危险）—— 它只是给人工判定一个**优先级**。
REACH_TIGHT = None


def _trigger_kind(txt):
    """候选的触发点来源：precise（精确三规则）/ conservative（保守兜底）/ n/a。"""
    name = (txt or '').strip().rstrip('(').strip()
    name = name.split()[0] if name else name
    if REACH_TIGHT is None:
        return 'n/a'
    return 'precise' if name in REACH_TIGHT else 'conservative'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--files', nargs='*', default=None)
    ap.add_argument('--json', action='store_true')
    ap.add_argument('--show-victims', action='store_true')
    ap.add_argument('--self-test', action='store_true')
    ap.add_argument('--grow', action='store_true',
                    help='M209：**旧规则对照** —— 把 px_dict_set/px_list_push 当触发点'
                         '（复现 M208 的 19 候选）。该前提已被证伪（缺陷 279），'
                         '仅用于 A/B 与留证，默认关闭。')
    ap.add_argument('--trigger-set', choices=['derived', 'legacy'], default='derived',
                    help='M212：触发点集合来源。derived=从 runtime/*.c 源码派生'
                         '（调用链可达 gc_register 的函数）；legacy=M206 的手抄构造器名单'
                         '（**已知漏 300+ 个分配入口**，仅用于 A/B 与留证）。默认 derived。')
    ap.add_argument('--min-triggers', type=int, default=250,
                    help='M212 规模锚点：派生触发点数少于该值即判红（防派生静默失效'
                         '⇒ 空集 ⊇ 任意集 ⇒ 假绿）')
    ap.add_argument('--min-funcs', type=int, default=1,
                    help='下限锚点：函数数少于该值即判红（防扫描器静默失效）')
    ap.add_argument('--no-callee-keep', action='store_true',
                    help='M213（缺陷 297）：关闭 F1「被调方登记了实参」规则（A/B 与负控用）')
    ap.add_argument('--indirect', choices=['loose', 'tight'], default='loose',
                    help='M213（缺陷 295）：间接调用判据档位。loose（默认）= 精确三规则'
                         ' + 任何 `(*x` 形状（**有意的过近似**）；tight = 仅精确三规则'
                         '（**只用于分诊/A-B**，会丢失显式 GC 族覆盖 ⇒ 不作默认）')
    ap.add_argument('--no-out', action='store_true',
                    help='M213：**关闭 OUT-PRODUCER 规则**（出口参数式构造函数 ⇒ 活值入列）。'
                         '默认开启；本开关仅用于 A/B 与留证（复现 M212 的漏报面）。')
    args = ap.parse_args()
    if args.self_test:
        return self_test()

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    global LEGACY_GROW
    LEGACY_GROW = bool(args.grow)
    # ---- M212：注入**源码派生**的触发点/生产者集合 ----
    if args.trigger_set == 'derived':
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        try:
            import gcroot_derive
            _t, _p, _st, _ex = gcroot_derive.derive_all(root, args.indirect)
            global REACH_TIGHT
            REACH_TIGHT = _ex['reach_tight']
        except Exception as e:                    # noqa: BLE001
            # ⚠️ **不静默回退**（M199/M200 纪律：判定不了必须响亮）——
            #   悄悄退回手抄集合 = 把「漏报」伪装成「全绿」。
            sys.stderr.write('gcroot_audit: 派生触发点失败（%s）⇒ 拒绝回退到'
                             '手抄集合（那会把漏报伪装成全绿）\n' % e)
            return 3
        if len(_t) < args.min_triggers or not (_p <= _t):
            sys.stderr.write('gcroot_audit: 派生集合异常（TRIGGER=%d < 下限 %d 或 '
                             'PRODUCER ⊄ TRIGGER）⇒ 拒绝放行\n'
                             % (len(_t), args.min_triggers))
            return 3
        set_trigger_sets(_t, _p)
        # ---- M213（缺陷 294）：OUT-PRODUCER（出口参数式构造函数）----
        #   规模锚点：派生为空 ⇒ **拒绝放行**（空集会让「出口式活值」全部漏报 = 假绿）。
        # ---- M213（缺陷 297）：F1 表（被调方入口登记了哪些形参）----
        _ck = gcroot_derive.derive_callee_keeps(root, _t)
        if not args.no_callee_keep:
            set_callee_keeps(_ck)
        else:
            set_callee_keeps({})
        sys.stderr.write('# F1 表（被调方入口已登记形参）= %d 条%s\n'
                         % (len(_ck), '［--no-callee-keep ⇒ 规则关闭·A/B 用］'
                            if args.no_callee_keep else ''))
        _op = gcroot_derive.derive_out_producers(root)
        if len(_op) < 1:
            sys.stderr.write('gcroot_audit: OUT-PRODUCER 派生为空 ⇒ 拒绝放行'
                             '（空集 = 出口式活值全部漏报 ⇒ 把「漏报」伪装成「全绿」）\n')
            return 3
        if not args.no_out:
            set_out_producers(_op)
        sys.stderr.write('# OUT-PRODUCER=%d（出口参数式构造函数）%s\n'
                         % (len(_op), '［--no-out ⇒ 规则关闭·A/B 用］' if args.no_out else ''))
        # ⚠️ 走 **stderr**（M205 §7.1「通道 = 消费方」）：`--json` 的 stdout 必须
        #   是**纯 JSON**（`examples/m206_gcroot/verify.sh` 直接 `json.load` 它）
        #   ⇒ 诊断行混进 stdout 会让门**解析失败**（本轮实测踩过）。
        _cons_only = len([n for n in _t if n not in (_ex['reach_tight'] or set())])
        sys.stderr.write('# 触发点集合：**源码派生** TRIGGER=%d PRODUCER=%d（手抄集合 %d）'
                         '｜间接调用档=**%s**｜其中「保守兜底」独有 = %d\n'
                         % (len(_t), len(_p), len(_ALLOC_NAMES),
                            args.indirect, _cons_only))
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
          ' · F1 豁免 %d'
          % (len(files), tot['funcs'], tot['allocs'], tot['keeps'], len(allf),
             tot.get('callee_keep', 0)))
    by_file = {}
    for f in allf:
        by_file.setdefault(f['file'], []).append(f)
    for fn in sorted(by_file, key=lambda k: -len(by_file[k])):
        print('\n%s  (%d)' % (fn, len(by_file[fn])))
        for f in by_file[fn]:
            line = '  %5d %-30s ← %s%s' % (
                f['line'], f['func'], f['trigger'],
                '  ［保守兜底·过近似］' if f.get('trigger_kind') == 'conservative' else '')
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
