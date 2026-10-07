#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# =====================================================================
# selfhost/shellscan.py —— shell 扫描的**共用原语**（M285）
#
# 为什么抽出来：
#   这些原语被**两个守卫**用着 —— `check_orchestrator.py`（M284 · 编排层）
#   与 `check_gate_assertions.py`（M285 · 门层）。M285 修缺陷 498 时，
#   同一个「抽 grep 的 pattern」的坑在两个文件里**各踩了一次**
#   （先抽错参数 → 再抽到消息）⇒ 一条规则两处实现 = 结构性分叉隐患
#   （M234 缺陷 352 立过的口径：**同一条规则只许出现一处**）。
#
# 提供的原语：
#   mask_prose       把「散文」抹成等长空格，只留**命令上下文**（行内注释 / 引号里的普通文本）
#   in_dquote        某位置之前是否有未闭合的双引号
#   shell_dequote    模拟 shell 双引号层：\\→\  \"→"  \$→$  \`→`
#   iter_grep_pats   产出 (pattern, 结束位置) —— pattern = 选项之后的**第一个 token**
#   varmap_of        同文件的字面量路径赋值 → {VAR: path}
#   target_of        取 [sed/grep] 命令的**文件操作数**
#   split_script     按未转义分隔符切 sed 脚本
#   unesc / bre_to_ere / pat_variants   模式的等价形态
# =====================================================================
import os
import re
import subprocess

# grep 的调用：`grep` + 选项* + 之后的部分
GREP_CALL = re.compile(r'(?<![\w-])grep\b(?P<opts>(?:\s+-{1,2}[A-Za-z0-9-]+)*)\s*(?P<tail>[^\n]*)')


def _is_comment_hash(line, i):
    """`#` 是不是**注释起点**：在词首（前面是空白/行首/分隔符），且不是 `${X#…}` 那种参数展开。"""
    if i > 0 and line[i - 1] in "$\\":
        return False
    if i > 0 and not (line[i - 1].isspace() or line[i - 1] in ";|&("):
        return False
    return True


# 「**引号里装的是命令**」的形状：本仓大量写 `chk "说明" "grep -q '串' 文件"`，
# chk 会把第三个参数 **eval** ⇒ 那个 `grep` 是真判据，不能当散文抹掉。
KEEP_Q = re.compile(r'\A\s*(?:grep|sed|!)\b')


def mask_prose(line):
    """把**散文**（行内注释 + 引号里的普通文本）抹成**等长空格**，只留命令上下文。

    为什么需要（实测两次假阳，都是「`grep` 只是被**提到**」）：
      · `iv_nopipeq() { # … 无「大产出 | grep -q」禁形（**只查代码行**…）`   ← grep 在**注释**里
      · `iv_bad "… 注入 tar|grep -q 竟未被发现（判据无牙）"`                ← grep 在**消息**里
    把这两处当成真 grep ⇒ 抽出的是消息文本 ⇒ **判据在错误的串上做**（缺陷 498 的形状）。

    ⚠️ 但**不能**把双引号里的一切都抹掉：本仓大量写
        `chk "$(echo "$OUT" | grep -c '可复现')" "1" "--locked…"`
    —— `grep` 在 `"$( … )"` 里，那是**命令替换**，是真命令。
    所以：双引号内的 `$( … )` 区间**保留**，其余抹掉。
    位置**一一对应**（等长替换）⇒ 调用方可以直接用同一套下标。
    """
    out = list(line)
    n = len(line)

    def blank(a, b):
        for k in range(a, min(b, n)):
            out[k] = " "

    i = 0
    while i < n:
        c = line[i]
        if c == "\\":
            i += 2
            continue
        if c == "#" and _is_comment_hash(line, i):
            blank(i, n)
            break
        if c in "'\"":
            q = c
            j, subs = i + 1, []
            while j < n:
                if line[j] == "\\":
                    j += 2
                    continue
                if q == '"' and line[j] == "$" and j + 1 < n and line[j + 1] == "(":
                    k, depth = j + 2, 1
                    while k < n and depth:
                        if line[k] == "\\":
                            k += 2
                            continue
                        if line[k] == "(":
                            depth += 1
                        elif line[k] == ")":
                            depth -= 1
                        k += 1
                    subs.append((j, k))
                    j = k
                    continue
                if line[j] == q:
                    break
                j += 1
            if not KEEP_Q.match(line[i + 1:j]):
                pos = i
                for a, b in subs:
                    blank(pos, a)
                    pos = b
                blank(pos, min(j + 1, n))
            i = j + 1
            continue
        i += 1
    return "".join(out)


def in_dquote(line, pos):
    """line[pos] 之前是否有**未闭合的双引号** ⇒ 该处处在 shell 双引号串里。"""
    cnt, i = 0, 0
    while i < pos and i < len(line):
        if line[i] == "\\":
            i += 2
            continue
        if line[i] == '"':
            cnt += 1
        i += 1
    return cnt % 2 == 1


def shell_dequote(s):
    """模拟 shell **双引号层**：\\\\→\\  \\"→"  \\$→$  \\`→`  \\换行→空；其它转义保留。

    ⚠️ 为什么必须有（实测 m191:140）：门的打桩常写成
        nc_static "…" "…" "sed -i 's|\\( *\\)px_error(\\"%s\\", errmsg);|…' '$CO'"
    —— sed 脚本处在**外层双引号**里，源码里的 `\\(` / `\\"` 到 shell 手里才变成 `\(` / `"`。
    不模拟这一层 ⇒ 提取到的 OLD 带多余反斜杠 ⇒ 在场判据**假红**。
    """
    out, i = [], 0
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            n = s[i + 1]
            if n in '\\"$`':
                out.append(n)
                i += 2
                continue
            if n == "\n":
                i += 2
                continue
            out.append(c)
            out.append(n)
            i += 2
            continue
        out.append(c)
        i += 1
    return "".join(out)


def iter_grep_pats(line):
    """产出 (pattern, pattern 结束位置, **grep 关键字的起始位置**)。

    pattern = **grep 自己的选项之后的第一个 token**（带引号则取引号内）。
    ⚠️ 不能简化成「行内第一处引号串」——反例（实测 packaging/selftest_reconcile.sh:148）：
        `… | grep -q .; then ng "诊断不该走 stdout"; …`
      真 pattern 是 `.`，"第一处引号串"却是**消息**（缺陷 498 的形状）。

    ⚠️ 调用方应传**原始行**解析（掩码会把 sed 脚本也抹掉），
       另用 `mask_prose` 的结果做「这个 grep 是不是真命令」的判据（见 check_gate_assertions）。
    """
    for m in GREP_CALL.finditer(line):
        tail = m.group("tail")
        j = 0
        while j < len(tail) and tail[j] in " \t":
            j += 1
        if j >= len(tail):
            continue
        base = m.start("tail") + j
        if tail[j] in "'\"":
            q, k, buf = tail[j], j + 1, []
            while k < len(tail):
                if tail[k] == "\\" and k + 1 < len(tail):
                    buf.append(tail[k:k + 2])
                    k += 2
                    continue
                if tail[k] == q:
                    break
                buf.append(tail[k])
                k += 1
            yield "".join(buf), base + k + 1, m.start()
        else:
            k = j
            while k < len(tail) and tail[k] not in " \t;|&)":
                k += 1
            yield tail[j:k], base + k, m.start()


def varmap_of(text):
    """同文件里的字面量路径赋值 → {VAR: path}（支持 $VAR 一层展开）。"""
    v, raw = {}, []
    for m in re.finditer(r'\b([A-Za-z_][A-Za-z0-9_]*)=([^\s;)]+)', text):
        raw.append((m.group(1), m.group(2).strip("'\"")))
    for _ in range(4):
        for name, val in raw:
            if val.startswith("$"):
                base = v.get(val[1:].strip("{}"))
                if base:
                    v.setdefault(name, base)
            else:
                v.setdefault(name, val)
    return v


def target_of(tail, varmap):
    """从命令的 tail 取**文件操作数**（第一个非选项 token）；可解 `$VAR`。"""
    cut = re.split(r'\|\||&&|\||;|\)|\n|\bdo\b|\bthen\b', tail, maxsplit=1)[0]
    for tok in cut.split():
        if tok.startswith("-"):
            continue
        t = tok.strip("'\"")
        if t.startswith("$"):
            return varmap.get(t.lstrip("$").strip("{}"), "")
        return t
    return ""


def repo_file(name):
    """解析到给定根内的**存在**文件 ⇒ 绝对路径；否则空串。"""
    if not name:
        return ""
    return name if os.path.isabs(name) else os.path.abspath(name)


def split_script(script, delim):
    """按**未转义**的分隔符切 sed 脚本（`\\|` 不能当分隔符 —— 实测踩过）。"""
    out, cur, i = [], [], 0
    while i < len(script):
        ch = script[i]
        if ch == "\\" and i + 1 < len(script):
            cur.append(script[i:i + 2])
            i += 2
            continue
        if ch == delim:
            out.append("".join(cur))
            cur = []
            i += 1
            continue
        cur.append(ch)
        i += 1
    out.append("".join(cur))
    return out


def unesc(s):
    """BRE/ERE 的反斜杠转义 → 字面（\\* → *，\\( → ( …）。"""
    return re.sub(r'\\(.)', r'\1', s)


def bre_to_ere(s):
    """BRE → **忠实**的 ERE。

    BRE 与 ERE 的差别**双向**，只做一半会**假红**（实测 m191:140）：
      · `\\(` `\\)` `\\|` `\\{` `\\}` `\\+` `\\?` —— BRE 元字符 ⇒ 到 ERE 去反斜杠
      · `(` `)` `{` `}` `|` `+` `?`      —— BRE 字面   ⇒ 到 ERE **加**反斜杠
    首版只做第一条 ⇒ `\\( *\\)px_error("%s", errmsg);` 变成 `( *)px_error("%s", errmsg);`
    ⇒ `px_error(` 的括号被 ERE 当**分组** ⇒ 匹配失败 ⇒ 假红（真锚点在 coro.c:836）。
    """
    out, i = [], 0
    GROUP = {"(": "(", ")": ")", "|": "|", "{": "{", "}": "}", "+": "+", "?": "?"}
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            n = s[i + 1]
            out.append(n if n in GROUP else "\\" + n)
            i += 2
            continue
        if c in "(){}|+?":
            out.append("\\" + c)
            i += 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


def pat_variants(pat):
    """一个模式在**多层引用**下的等价形态集合（放宽方向 = fail-safe）。"""
    out = {pat}
    for _ in range(2):
        new = set()
        for x in out:
            new.add(shell_dequote(x))
            new.add(unesc(x))
            new.add(bre_to_ere(x))
        out |= {x for x in new if x}
    return {x for x in out if x}


def pat_present(pat, body):
    """串（或多层引用等价物）是否在 body 里 —— 任一形态命中即算在场。"""
    vs = pat_variants(pat)
    for c in vs:
        if c in body:
            return True
    for c in vs:
        try:
            if re.search(c, body, re.M):
                return True
        except re.error:
            pass
    if "\\|" in pat:                       # BRE 交替：每个分支都必须在场
        br = [unesc(b).strip() for b in pat.split("\\|")]
        br = [b for b in br if b]
        if br and all(b in body for b in br):
            return True
    return False


def git_grep_hit(root, needle, paths, extra_excl=()):
    """needle 在给定 pathspec 下是否有命中（`--untracked`；可加排除）。"""
    try:
        cmd = ["git", "-C", root, "grep", "-lF", "-e", needle, "--untracked", "--"]
        cmd += list(paths) + list(extra_excl)
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
        return bool((r.stdout or "").strip())
    except Exception:
        return True                        # 出错不判红（宁可漏报，不可假红）
