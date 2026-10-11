#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M300 独立参考实现（**与被测实现不同语言 · 不同算法**）
# ------------------------------------------------------------
# 用途：为「合法侧」用例提供**独立期望值** —— 不由三轨自证，也不由手算
#   （M215 教训：「三轨一致」是必要条件不是充分条件）。
# 语义依据：各 `stdlib/*.px` 的**文档注释**（与 Go 标准库对照的那部分）。
# ⚠️ 本文件**故意不复用**标准库实现，也不 import 被测代码。
# ============================================================
_ALNUM = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
_URL_SAFE = _ALNUM | set("-_.~")
_PATH_SAFE = _ALNUM | set("-_.~$&+:=@")


def index_of(s, sub):
    """Go strings.Index：空 needle → 0；不存在 → -1（rune 轴）"""
    return 0 if len(sub) == 0 else s.find(sub)


def index_of_from(s, sub, start):
    n = len(s)
    st = 0 if start < 0 else start
    if st > n:
        return -1
    r = index_of(s[st:n], sub)
    return -1 if r < 0 else r + st


def last_index_of(s, sub):
    return len(s) if len(sub) == 0 else s.rfind(sub)


def count_of(s, sub):
    """文档明确：sub 为空串 → 0（与 Go Count 不同，本库自定）"""
    return 0 if len(sub) == 0 else s.count(sub)


def split_n(s, sep, n):
    if n < 1:
        n = 1
    if len(sep) == 0:
        return [s]
    return s.split(sep, n - 1)


def split_lines(s):
    return [p[:-1] if p.endswith("\r") else p for p in s.split("\n")]


def repeat_str(s, n):
    return "" if n <= 0 else s * n


def pad_left(s, width, ch):
    c = (ch if len(ch) > 0 else "0")[0]
    out = s
    while len(out) < width:
        out = c + out
    return out


def pad_right(s, width, ch):
    c = (ch if len(ch) > 0 else " ")[0]
    out = s
    while len(out) < width:
        out = out + c
    return out


def str_reverse(s):
    return s[::-1]


def trim_chars(s, cutset):
    return s if len(cutset) == 0 else s.strip(cutset)


def trim_left_chars(s, cutset):
    return s if len(cutset) == 0 else s.lstrip(cutset)


def trim_right_chars(s, cutset):
    return s if len(cutset) == 0 else s.rstrip(cutset)


def trim_prefix(s, prefix):
    if len(prefix) == 0:
        return s
    return s[len(prefix):] if s.startswith(prefix) else s


def trim_suffix(s, suffix):
    if len(suffix) == 0:
        return s
    return s[:-len(suffix)] if s.endswith(suffix) else s


def is_hex(s):
    if len(s) == 0 or len(s) % 2 != 0:
        return False
    return all(c in "0123456789abcdefABCDEF" for c in s)


# ── url（逐字节，与 Go 的 QueryEscape / PathEscape 对照）──────
def _esc(s, safe):
    out = []
    for b in s.encode("utf-8"):
        c = chr(b)
        if b < 128 and c in safe:
            out.append(c)
        elif b == 32:
            out.append("+")
        else:
            out.append("%%%02X" % b)
    return "".join(out)


def url_query_escape(s):
    """Go url.QueryEscape：安全集 66 个 · 空格 → '+' · 其余 %XX 大写"""
    return _esc(s, _URL_SAFE)


def url_path_escape(s):
    """Go url.PathEscape：安全 = alnum + `-_.~` + `$&+:=@`（`/` 转义）"""
    out = []
    for b in s.encode("utf-8"):
        c = chr(b)
        if b < 128 and c in _PATH_SAFE:
            out.append(c)
        else:
            out.append("%%%02X" % b)
    return "".join(out)


def _unescape(s, plus_is_space):
    b = s.encode("utf-8", "surrogateescape")
    res = bytearray()
    i = 0
    while i < len(b):
        if b[i] == 0x25 and i + 2 < len(b):
            h = b[i + 1:i + 3].decode("ascii", "replace")
            if all(x in "0123456789abcdefABCDEF" for x in h):
                res.append(int(h, 16)); i += 3; continue
        if plus_is_space and b[i] == 0x2B:
            res.append(0x20)
        else:
            res.append(b[i])
        i += 1
    return res.decode("utf-8", "replace")


def url_query_unescape(s):
    """`+` → 空格；非法 % 序列原样保留"""
    return _unescape(s, True)


def url_query_unescape_keep_q(s):
    """与 QueryUnescape 的区别：**不**把 `+` 当空格"""
    return _unescape(s, False)


def url_values_encode(d):
    """Go url.Values.Encode：键按字节序升序；列表值按插入序重复同名键"""
    parts = []
    for k in sorted(d.keys()):
        v = d[k]
        for item in (v if isinstance(v, list) else [v]):
            parts.append(url_query_escape(k) + "=" + url_query_escape(str(item)))
    return "&".join(parts)


REF = {n: f for n, f in list(globals().items())
       if callable(f) and not n.startswith('_') and n not in ('render', 'compute')}


def render(v):
    """把参考实现返回值渲染成「程序自身 stdout」形态（与 print 一致）"""
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, list):
        return "[" + ", ".join(render(x) for x in v) + "]"
    if isinstance(v, dict):
        return "{" + ", ".join("%s: %s" % (k, render(x)) for k, x in v.items()) + "}"
    if v is None:
        return "null"
    return str(v)


def compute(name, args):
    return render(REF[name](*args))


# ── url_parse / url_parse_query（Go net/url 的常用子集）────────
def _valid_scheme(s):
    if not s or not (s[0].isalpha() and s[0].isascii()):
        return False
    return all((c.isalnum() and c.isascii()) or c in "+-." for c in s)


def _q_ok(s):
    """Go QueryUnescape 的可判错版本：返回 (值, ok)"""
    b = s.encode("utf-8", "surrogateescape")
    res = bytearray()
    i = 0
    while i < len(b):
        if b[i] == 0x25:
            if i + 2 < len(b):
                h = b[i + 1:i + 3].decode("ascii", "replace")
                if all(x in "0123456789abcdefABCDEF" for x in h):
                    res.append(int(h, 16)); i += 3; continue
            return ("", False)
        if b[i] == 0x2B:
            res.append(0x20)
        else:
            res.append(b[i])
        i += 1
    return (res.decode("utf-8", "replace"), True)


def url_parse(raw):
    scheme = user = host = path = rawquery = fragment = ""
    rest = raw
    i = rest.find("#")
    if i >= 0:
        fragment = rest[i + 1:]; rest = rest[:i]
    j = rest.find(":")
    if j > 0 and _valid_scheme(rest[:j]):
        scheme = rest[:j]; rest = rest[j + 1:]
    if rest.startswith("//"):
        rest = rest[2:]
        k = len(rest)
        for ch in "/?#":
            p = rest.find(ch)
            if p >= 0 and p < k:
                k = p
        auth, rest = rest[:k], rest[k:]
        a = auth.rfind("@")
        if a >= 0:
            user, host = auth[:a], auth[a + 1:]
        else:
            host = auth
        qi = rest.find("?")
        if qi >= 0:
            rawquery, rest = rest[qi + 1:], rest[:qi]
        path = rest
    else:
        qi = rest.find("?")
        if qi >= 0:
            rawquery, rest = rest[qi + 1:], rest[:qi]
        path = rest
    return {"scheme": scheme, "user": user, "host": host,
            "path": url_query_unescape_keep_q(path), "rawquery": rawquery, "fragment": fragment}


def url_parse_query(raw):
    vals = {}
    bad = False
    for seg in raw.split("&"):
        if seg == "":
            continue
        if ";" in seg:
            bad = True
            continue
        i = seg.find("=")
        k, v = (seg[:i], seg[i + 1:]) if i >= 0 else (seg, "")
        kk, ok1 = _q_ok(k)
        vv, ok2 = _q_ok(v)
        if not (ok1 and ok2):
            bad = True
            continue
        vals.setdefault(kk, []).append(vv)
    return [vals, not bad]


REF.update({n: f for n, f in list(globals().items())
            if callable(f) and n in ('url_parse', 'url_parse_query')})
