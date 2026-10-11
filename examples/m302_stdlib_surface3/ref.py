#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M302 独立真值（ref.py）—— std.semver / std.html
# ------------------------------------------------------------
# 判据纪律（M215/M300 建立）：
#   **合法侧的期望值必须来自「不由三轨自证」的独立实现** —— 三轨跑的是**同一份语义**
#   （病灶在 stdlib 实现内部时，三轨按定义一致）⇒ 只有独立真值能看见。
#
# 「不同构」要求（M215）：本文件对 semver 的实现用
#   **整数元组比较**（`(major, minor, patch, prerelease_tuple)` 的 Python 原生比较），
#   而被测实现是**逐字段 if 链** —— 两者算法结构不同，避免「同错同绿」。
#   semver 语义取自 semver.org v2.0.0 §9/§11 与 node-semver 的 range 展开规则。
# ============================================================
import re

# ─────────────────────────────── semver
_NUM = re.compile(r'^(0|[1-9][0-9]*)$')
_IDENT = re.compile(r'^[0-9A-Za-z-]+$')


def parse(v):
    """SemVer 2.0.0 严格解析 → (major, minor, patch, pre_tuple, build) 或 None
       pre_tuple: () 表示无 prerelease；否则元组，元素为 (0,int) 数字标识符 / (1,str) 字母数字标识符
       ⚠️ 用元组而非字符串比较 ⇒ 与被测实现（逐字段 if）**不同构**。"""
    if not isinstance(v, str) or v == '':
        return None
    s = v
    build = ''
    if '+' in s:
        s, build = s.split('+', 1)
        if build == '' or not all(_IDENT.match(p) for p in build.split('.')):
            return None
    pre = ()
    if '-' in s:
        s, prestr = s.split('-', 1)
        if prestr == '':
            return None
        parts = prestr.split('.')
        ids = []
        for p in parts:
            if p == '' or not _IDENT.match(p):
                return None
            if p.isdigit():
                # SemVer §9：纯数字标识符**不得有前导零** ⇒ `1.2.3-01` 非法
                if not _NUM.match(p):
                    return None
                ids.append((0, int(p)))
            else:
                if not _IDENT.match(p):
                    return None
                ids.append((1, p))
        pre = tuple(ids)
    segs = s.split('.')
    if len(segs) != 3:
        return None
    nums = []
    for p in segs:
        if not _NUM.match(p):
            return None
        nums.append(int(p))
    return (nums[0], nums[1], nums[2], pre, build)


def _key(p):
    """可比较键：数字段缺省视为最高（无 prerelease > 有 prerelease）；
       prerelease 逐标识符比较：数字 < 字母数字，数字按数值、字母按 ASCII。"""
    major, minor, patch, pre, _ = p
    return (major, minor, patch, 1 if pre == () else 0, pre)


def cmp_versions(a, b):
    """-1 / 0 / 1；任一非法 ⇒ 抛 ValueError（对应被测实现的 R2001）"""
    pa, pb = parse(a), parse(b)
    if pa is None:
        raise ValueError('非法版本: %s' % a)
    if pb is None:
        raise ValueError('非法版本: %s' % b)
    ka, kb = _key(pa), _key(pb)
    return -1 if ka < kb else (0 if ka == kb else 1)


def _match_nums(seg):
    """把 `1.2.x` / `1.x` / `*` / `1.2` / `1` 这类段化成 (nums, wild)，
       nums 里的 None 表示「通配/缺段」。返回 None 表示非法。"""
    parts = seg.split('.')
    if len(parts) > 3:
        return None
    nums = []
    for p in parts:
        pl = p.lower()
        if pl in ('x', '*', ''):
            nums.append(None)
        elif _NUM.match(p):
            nums.append(int(p))
        else:
            return None
    while len(nums) < 3:
        nums.append(None)
    return nums


def range_lo_hi(rng):
    """把范围展开成半开区间 [lo, hi)（版本字符串对）；非法范围 ⇒ None。
       同时返回 `allow_pre`（本实现的范围语法不支持 prerelease 限定 ⇒ 恒 False）。"""
    if not isinstance(rng, str):
        return None
    r = rng.strip()
    if r in ('', '*'):
        return ('0.0.0', '999999999.0.0', False)
    op = ''
    if r[0] in '^~':
        op = r[0]
        r = r[1:].strip()
        if r == '':                      # `^` / `~` 单独出现 = 非法范围（M302 缺陷 521）
            return None
    if r in ('', '*'):
        return None if op else ('0.0.0', '999999999.0.0', False)
    nums = _match_nums(r)
    if nums is None:
        return None
    ma, mi, pa = nums[0], nums[1], nums[2]
    if ma is None:
        return ('0.0.0', '999999999.0.0', False)

    def s(a, b, c):
        return '%d.%d.%d' % (a, b, c)

    if op == '':
        if mi is not None and pa is not None:        # 精确
            return (s(ma, mi, pa), s(ma, mi, pa + 1), False)
        if mi is not None:                            # 1.2 / 1.2.x
            return (s(ma, mi, 0), s(ma, mi + 1, 0), False)
        return (s(ma, 0, 0), s(ma + 1, 0, 0), False)  # 1 / 1.x
    if op == '^':
        lo_mi = 0 if mi is None else mi
        lo_pa = 0 if pa is None else pa
        if ma > 0:
            return (s(ma, lo_mi, lo_pa), s(ma + 1, 0, 0), False)
        if mi is not None and mi > 0:
            return (s(0, mi, lo_pa), s(0, mi + 1, 0), False)
        if mi == 0 and pa is not None:
            return (s(0, 0, pa), s(0, 0, pa + 1), False)
        if mi == 0:
            return (s(0, 0, 0), s(0, 1, 0), False)
        return (s(0, 0, 0), s(1, 0, 0), False)
    # op == '~'
    if mi is None:                                    # ~1 → >=1.0.0 <2.0.0
        return (s(ma, 0, 0), s(ma + 1, 0, 0), False)
    if pa is None:                                    # ~1.2 → >=1.2.0 <1.3.0
        return (s(ma, mi, 0), s(ma, mi + 1, 0), False)
    return (s(ma, mi, pa), s(ma, mi + 1, 0), False)


def satisfies(ver, rng):
    """布尔谓词：**非法版本或非法范围 ⇒ False**（可接受的口径 —— 谓词无第三态；
       而 sv_cmp 返回三态，非法必须响亮，见下）"""
    r = range_lo_hi(rng)
    if r is None:
        return False
    if parse(ver) is None:
        return False
    if parse(ver)[3] != ():          # 预发布版本：本实现范围语法不支持 pre 限定 ⇒ 不匹配
        return False
    lo, hi, _ = r
    return cmp_versions(ver, lo) >= 0 and cmp_versions(ver, hi) < 0


def best(versions, rng):
    winner = None
    for v in versions:
        if not satisfies(v, rng):
            continue
        if winner is None or cmp_versions(v, winner) > 0:
            winner = v
    return winner


# ─────────────────────────────── html
# 转义契约（M302 定稿）：`& < > " '` 五个字符 —— 与 Go html.EscapeString 对齐（含单引号）
_ESC = [('&', '&amp;'), ('<', '&lt;'), ('>', '&gt;'), ('"', '&quot;'), ("'", '&#39;')]


def escape(s):
    out = []
    for ch in s:
        rep = dict(_ESC).get(ch)
        out.append(rep if rep else ch)
    return ''.join(out)


# 实体集 = **契约内的 6 个**（M302 登记：其余命名实体与全部数字实体属**能力缺口**，不在契约内）
_ENTS = {'amp': '&', 'lt': '<', 'gt': '>', 'quot': '"', 'apos': "'", 'nbsp': '\u00a0'}


def decode(s):
    """契约内解码：只解 6 个命名实体；数字实体与其它命名实体**原样保留**。
       扫描规则与被测实现一致（`;` 在 1..11 个字符内才认）。"""
    if '&' not in s:
        return s
    out = []
    i, n = 0, len(s)
    while i < n:
        c = s[i]
        if c == '&':
            j = s.find(';', i + 1)
            if j > i + 1 and j - i <= 12:
                name = s[i + 1:j]
                if name in _ENTS:
                    out.append(_ENTS[name])
                    i = j + 1
                    continue
                if name[0] == '#':
                    out.append(s[i:j + 1])
                    i = j + 1
                    continue
            out.append(c)
            i += 1
            continue
        out.append(c)
        i += 1
    return ''.join(out)
