#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# =====================================================================
# selfhost/check_gate_assertions.py —— **门层**判据守卫（M285）
#
# 为什么存在：
#   M284 把「判据串在场」（O1）做到了**编排层**，并如实登记了覆盖边界：
#   门的判据串刻意未覆盖（实测多出来的一批是「门的判据串来自夹具文本」）。
#   而缺陷 495（watcher 里那句凭空的 `双路判据一致：0 红`）说明：
#   **门的判据串从来没有「在场」检查**。本守卫把它补上，并顺手补一条更值钱的。
#
# 三条判据（各配反向判据，见 --self-test）：
#   A1 **打桩锚点在位**（本轮最有价值的一条）
#      `sed -i 's|OLD|NEW|' <目标>` 是负控的标准打桩法。若 `OLD` **已不在 <目标> 里**
#      ⇒ sed **静默什么都不做**（不报错、rc=0）⇒ 负控**被抽掉牙**，门照样 PASS 却**什么都没测**。
#      M230 的原话：「锚点失配**不会报错**，只会静默失效」——
#      M161 / M164 / M213 / M226 / M227 反复撞过同一形状（这是第 N 次）。
#   A2 **判据串在场**（把 M284 的 O1 推到门层 · 夹具感知）
#      · 目标可解析到仓库内文件 ⇒ 串必须**在该文件里**（源码锚点自查 / 文档自查）
#      · 否则 ⇒ 串必须在**代码或夹具**里存在
#        ⚠️ 搜索面**刻意排除 CHANGELOG.md / docs 散文** ⇒ **免疫自指**
#           （M284 v2 的教训：我在 CHANGELOG 里引用「它不存在」反而让它存在）
#      · **只对正向断言**适用；反向断言（`if grep…; then bad` / `&& VAR=1` / `|| true` / `!`）
#        期望该串**缺席**，跳过（m245/m263 的真实形状）
#      · **打桩后状态**跳过：同文件同目标此前有 `sed -i`（m201 的真实形状）
#   A4 **打桩锚点唯一性**（M286 · A1 的自然强化）
#      A1 只判「OLD 在不在目标里」；**不判「OLD 出现几次」**。
#      而 `sed -i 's|OLD|NEW|'` 是**逐行替换**（无 g）⇒ OLD 出现在 N 行 ⇒ **N 处都被改**。
#      门通常只想改**一处** ⇒ 多改 = 「打桩范围**超出声明意图**」：
#        · M200:145 改 3 处（:51 目标 + :164 `ffi_call` + :202 `px_ffi_has`）—— 后两处是**别的函数**
#        · M196:165 改 2 处（:27338 native 面 + :3816 核心 `px_gen_next`）—— 而门**声明只改原生**
#      ⚠️ 危险性：多改往往**不影响本门判据** ⇒ 长期无人发现；但它让「门声明的前提」变成假的，
#         且可能让另一个判据"更容易红" ⇒ **虚假的判据强度**。
#      豁免：`selfhost/gate_anchor_multi.tsv`（具名 `<脚本>:<行号>` + 行数 + 理由）；
#         表**双向**判据 —— 实测多行而表里没有 ⇒ 判红；表里有而实测不再多行 ⇒ **过期**判红。
#   A3 **规模锚点** —— 判定数 / 打桩数不得跌破下限（防判据静默变窄）
#
# 用法：
#   check_gate_assertions.py                 # 扫真仓（examples + selfhost + packaging + tools…）
#   check_gate_assertions.py --self-test     # 内置夹具自证（含 2 条反向判据）
#   check_gate_assertions.py --count         # 只打印统计
#   check_gate_assertions.py --file X.sh     # 扫任意脚本（每轮 /tmp 编排件落地前自查）
#   GA_ROOT / GA_DIRS                        # 换根 / 换扫描面（门做 A/B 用）
# =====================================================================
import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from shellscan import (GREP_CALL, bre_to_ere, git_grep_hit, in_dquote,  # noqa: E402
                       iter_grep_pats, mask_prose, pat_present, shell_dequote,
                       split_script, target_of, varmap_of)

ROOT = os.environ.get("GA_ROOT") or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CJK = re.compile(r'[\u4e00-\u9fff]')
RUN = re.compile(r'[\u4e00-\u9fff]+')

DEFAULT_DIRS = "examples,selfhost,packaging,tools,stdlib"
SCAN_DIRS = tuple((os.environ.get("GA_DIRS") or DEFAULT_DIRS).split(","))

# 代码/夹具的搜索面（A2 兜底）—— **刻意不含 *.md / CHANGELOG** ⇒ 免疫自指
CODE_GLOB = ("*.sh", "*.py", "*.px", "*.c", "*.h", "*.go", "*.mk", "*.bash")
EXTRA_SCRIPTS = ("tools/px", "tools/pxc")     # 无扩展名脚本，pathspec 匹配不到，单独列
FIXTURE_SEG = "/fixtures/"

# sed -i 的调用
#   ⚠️ M286：必须支持**行地址前缀**（`/re/` · `N` · `$`，可 `,` 接第二个）。
#      不支持 ⇒ `/^void f/,/^}/ s|OLD|NEW|` 这种**精确打桩**写法会被**整条漏掉**
#      ⇒ 该打桩从此不在 A1/A4 的视野里（实测：统计从 40 掉到 38 = 假绿）。
#   BRE 语义：地址里的 `(` `)` `{` `}` `+` `?` `|` 是**字面**字符 —— 由 `bre_to_ere` 负责转换。
_SED_ADDR = r'(?:\d+|\$|/(?:[^/\\]|\\.)*/)'
SED_CALL = re.compile(r'(?<![\w-])sed\b(?P<flags>(?:\s+-[A-Za-z]+)*)\s*(?P<q>[\'"])'
                      r'(?P<addr>' + _SED_ADDR + r'(?:,' + _SED_ADDR + r')?)?\s*'
                      r'(?P<script>s(?P<d>.).*?)(?P=q)(?P<tail>[^\n]*)')
_SED_ADDR_RE = re.compile(
    r'^(?P<a>\d+|\$|/(?:[^/\\]|\\.)*/)(?:,(?P<b>\d+|\$|/(?:[^/\\]|\\.)*/))?$')


def _addr_hit(lines, spec, start=0):
    """地址 spec（`/re/` · `N` · `$`）命中的 **0-based 行号**（自 start 起找）；找不到 ⇒ -1。"""
    if spec.startswith("/"):
        rx = bre_to_ere(spec[1:-1])
        for k in range(start, len(lines)):
            try:
                if re.search(rx, lines[k]):
                    return k
            except re.error:
                return -1
        return -1
    if spec == "$":
        return len(lines) - 1
    try:
        n = int(spec)
    except ValueError:
        return -1
    return n - 1 if 1 <= n <= len(lines) else -1


def count_old(lines, old, addr=None):
    """`old` 在 sed **实际会改到的行**里出现几次。
    无地址 / 地址不可解析 ⇒ 全文件（sed 的默认语义）。"""
    m = _SED_ADDR_RE.match(addr) if addr else None
    if not m:
        return sum(1 for l in lines if old in l)
    a, b = m.group("a"), m.group("b")
    if b is None:
        if a.startswith("/"):
            try:
                rx = bre_to_ere(a[1:-1])
                return sum(1 for l in lines if old in l and re.search(rx, l))
            except re.error:
                return sum(1 for l in lines if old in l)
        k = _addr_hit(lines, a)
        return 1 if (k >= 0 and old in lines[k]) else 0
    cnt, k = 0, 0
    while k < len(lines):
        st = _addr_hit(lines, a, k)
        if st < 0:
            break
        en = _addr_hit(lines, b, st)
        if en < 0:
            en = len(lines) - 1
        cnt += sum(1 for l in lines[st:en + 1] if old in l)
        k = en + 1
    return cnt

CACHE = {}

# ── A4 豁免表（M286）── 键 = "<脚本相对路径>:<行号>"；**只准登记「多改无害」**
#   ⚠️ 表本身不设"按文件允许"的粗粒度豁免 —— 那会让整份脚本免疫（M282 缺陷 491 的口径）。
MULTI_WAIVER_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                 "gate_anchor_multi.tsv")
_MULTI = None


def load_multi_waiver(path=None):
    """返回 {key: (行数, 理由)}。文件不存在 ⇒ 空表（不报错：豁免是可选机制）。"""
    global _MULTI
    if path is None and _MULTI is not None:
        return _MULTI
    out = {}
    for ln in read(path or MULTI_WAIVER_FILE).split("\n"):
        t = ln.strip()
        if not t or t.startswith("#"):
            continue
        parts = t.split("\t")
        if len(parts) >= 3:
            out[parts[0]] = (parts[1], parts[2])
    if path is None:
        _MULTI = out
    return out


def read(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            return fh.read()
    except Exception:
        return ""


def repo_path(name, root=None):
    """解析到 root 内的**存在**文件 ⇒ 绝对路径；否则空串。"""
    root = root or ROOT
    if not name:
        return ""
    p = name if os.path.isabs(name) else os.path.join(root, name)
    p = os.path.normpath(p)
    if not p.startswith(root) or "/tmp/" in p or not os.path.isfile(p):
        return ""
    return p


def findable(s):
    """A2 兜底：串在**代码/夹具**里是否有生产者。"""
    if s in CACHE:
        return CACHE[s]
    paths = ["*" + e for e in CODE_GLOB] + list(EXTRA_SCRIPTS)
    # ⚠️ 必须**排除 fixtures/**：那是反例样本。不排除 ⇒ 自证里 `bad_a2_pat.sh` 把「凭空串」
    #    写进文件 ⇒ 它当场"在场" ⇒ 判据**自己把反例变成正例**（自证实测 3/3 假绿）。
    #    这正是 M284 v2 的自指陷阱在**守卫内部**的翻版 ⇒ 反过来，本守卫自己的源码
    #    也不得含反例串（见 FIXTURES 的**码点拼装**）。
    ok = git_grep_hit(ROOT, s, paths, extra_excl=[":(exclude)*/fixtures/*"])
    CACHE[s] = ok
    return ok


def criteria_verdict(pat):
    """M284 口径：最长 CJK 段（≥3）判在场；否则任一 ≥2 的段在场即可。"""
    runs = [r for r in RUN.findall(pat) if len(r) >= 2]
    if not runs:
        return None, ""
    long_runs = [r for r in runs if len(r) >= 3]
    if long_runs:
        key = max(long_runs, key=len)
        return findable(key), key
    for r in runs:
        if findable(r):
            return True, r
    return False, runs[0]


# ── 断言方向：grep 的**结果**被用作「必须为真」还是「必须为假」 ──────────
#   反向断言（期望缺席）在真仓里有三种写法：
#     ① `if grep …; then bad "旧措辞仍在"` —— 找到 ⇒ 判红
#     ② `grep … && v_stale=1`             —— 找到 ⇒ 记账为违例
#     ③ `grep … || true` / `! grep`       —— 结果被吞掉
FAILWORD = r'(?:bad|no|fail|die|exit|❌)'
NEG_USE = re.compile(r'\|\|\s*(?:true|:)|&&\s*[A-Za-z_][A-Za-z0-9_]*\s*=\s*1|!\s*grep')
NEG_FOUND = re.compile(r'&&[^|;]*' + FAILWORD + r'|;\s*then\s*' + FAILWORD)


def assertion_dir(line, after):
    """返回 'neg'（期望缺席）/ 'pos'（期望在场）。保守：拿不准算 pos（会报，宁可让我看）。"""
    if NEG_USE.search(after) or NEG_FOUND.search(after):
        return "neg"
    if re.match(r'\s*if\s+!', line) or "grep -v" in line:
        return "neg"
    return "pos"


def scan_file(path, rel, patched_targets, stats=None):
    bad = []
    lines = read(path).split("\n")
    varmap = varmap_of("\n".join(lines))
    if stats is not None:
        stats["files"] += 1
    # ── A1 打桩锚点在位 ──
    for i, line in enumerate(lines, 1):
        if line.strip().startswith("#"):
            continue
        # ⚠️ 定位用 **masked**（散文里提到的 `sed`/`grep` 不算命令），
        #    解析用 **原始行**（掩码会把引号里的 sed 脚本一并抹掉）—— 见 mask_prose 头注。
        masked = mask_prose(line) if ("sed" in line or "grep" in line) else line
        for m in SED_CALL.finditer(line):
            if masked[m.start():m.start() + 3] != line[m.start():m.start() + 3]:
                continue                  # `sed` 只是被**提到**（注释/消息里）
            if "i" not in m.group("flags"):
                continue
            if stats is not None:
                stats["sed_i"] += 1
            script = m.group("script")
            if in_dquote(line, m.start("script")):
                script = shell_dequote(script)     # 打桩常写在**外层双引号**里
            parts = split_script(script, m.group("d"))
            if len(parts) < 3:
                continue
            old = parts[1]
            if not old.strip():
                continue
            ts = m.start("tail")
            mtail = line[ts:ts + len(m.group("tail"))]
            tgt = repo_path(target_of(mtail, varmap))
            if not tgt:
                if stats is not None:
                    stats["sed_unres"] += 1
                continue                  # 目标不可解析（$W / /tmp / 命令替换）⇒ 不判
            if stats is not None:
                stats["sed_app"] += 1
            patched_targets.add(tgt)
            # ── A4（M286）打桩锚点**唯一性** · **范围感知** ─────────
            #   有行地址 ⇒ 只数**地址覆盖范围内**的 OLD 行数（`/^void f/,/^}/ s|…|` 精确打桩的语义）；
            #   无地址   ⇒ 全文件（sed 逐行替换 ⇒ N 行就改 N 处）。
            #   注意：下面 `read(tgt)` 是**独立**调用（不改动 A1 的判据行 —— 旧门负控打桩在它上面）
            _nk = "%s:%d" % (rel, i)
            # 「**整行替换**」惯用法（`sed -i 'Ns/.*/NEW/' <数据文件>`）：OLD 是通配而非锚点
            # ⇒ 「锚点唯一性」无意义（永远匹配整行）⇒ **显式排除**（否则 count_old 按字面 = 0 ⇒ 假红 A1r）。
            # 实测来源：SED_CALL 支持行地址后**多抓出 9 处**这种写法（m136/m138/m142/m143/m146/m147/m148）。
            if old.strip() in (".*", "^.*$", ".+", "^.", ".*$", "^.*"):
                if stats is not None:
                    stats["whole_line"] = stats.get("whole_line", 0) + 1
                continue
            _body = read(tgt).split("\n")
            _nline = count_old(_body, old, m.group("addr"))
            if m.group("addr") and _nline == 0:
                # 地址内没有 OLD ⇒ 打桩**静默不生效**；此时 A1 的「全文件在场」为真 ⇒ 看不见它
                bad.append(("A1r", i, "地址内没有打桩锚点 ⇒ sed 静默不生效（负控失牙）· "
                            "地址 %s · 目标 %s · OLD %r"
                            % (m.group("addr"), os.path.relpath(tgt, ROOT), old[:60])))
            elif _nline > 1:
                # ⚠️ 自证模式（--self-test）下 `stats is None` ⇒ 所有计数**必须**保护
                #    （M285 的 A1/A2 段都这么写；A4 首版漏了 ⇒ 自证当场抛 AttributeError）
                if stats is not None:
                    stats.setdefault("multi_seen", set()).add(_nk)
                if _nk in load_multi_waiver():
                    if stats is not None:
                        stats["a4_waived"] = stats.get("a4_waived", 0) + 1
                else:
                    if stats is not None:
                        stats["a4_multi"] = stats.get("a4_multi", 0) + 1
                        stats["a4_viol"] = stats.get("a4_viol", 0) + 1
                    bad.append(("A4", i, "打桩锚点不唯一：sed 实际改 %d 处"
                                "（超出声明意图）· 地址 %s · 目标 %s · OLD %r"
                                % (_nline, m.group("addr") or "（全文件）",
                                   os.path.relpath(tgt, ROOT), old[:70])))
            if pat_present(old, read(tgt)):
                continue
            bad.append(("A1", i, "打桩锚点不在位：sed 静默不生效 ⇒ 负控失牙 · 目标 %s · OLD %r"
                        % (os.path.relpath(tgt, ROOT), old[:80])))
    # ── A2 判据串在场 ──
    for i, line in enumerate(lines, 1):
        if line.strip().startswith("#") or "grep" not in line:
            continue
        masked = mask_prose(line)
        for pat, pend, gstart in iter_grep_pats(line):
            if masked[gstart:gstart + 4] != line[gstart:gstart + 4]:
                continue                  # `grep` 只是被**提到**（注释/消息里）⇒ 不是命令
            if in_dquote(line, max(0, pend - len(pat) - 1)):
                pat = shell_dequote(pat)
            if not (CJK.search(pat) and len(pat) >= 3):
                continue
            if stats is not None:
                stats["a2_str"] += 1
            # ⚠️ 断言方向要看**后文**：真仓里的反向断言常把动作写在**下一行**
            #    （`if grep …; then` ↵ `  bad "旧措辞仍在"`）⇒ 不带上下文会判成「正断言」⇒ 假红
            after = line[pend:]
            ctx = after + "\n" + "\n".join(lines[i:i + 2])
            if assertion_dir(line, ctx) == "neg":
                continue                  # 反向断言：期望缺席
            # ⚠️ 目标必须从**pattern 之后**取 —— 首版从 tail 开头取，而 pattern 含空格时
            #    （`'runtime/runtime.c .*无码 0'`）第一个 token 就是 pattern 的前半截
            #    ⇒ 被当成**目标文件** ⇒ 假红（实测 m191:64）。
            tgt = repo_path(target_of(after, varmap))
            if stats is not None:
                stats["a2_target" if tgt else "a2_fallback"] += 1
            if tgt and tgt in patched_targets:
                continue                  # 同文件此前打过桩 ⇒ 检的是**打桩后**状态
            if tgt:
                if not pat_present(pat, read(tgt)):
                    bad.append(("A2", i, "判据串不在目标文件里（过期锚点）：%s 里没有 %r"
                                % (os.path.relpath(tgt, ROOT), pat[:70])))
                continue
            ok, key = criteria_verdict(pat)
            if ok is None or ok:
                continue
            bad.append(("A2", i, "凭空判据串：骨架 %r 在代码/夹具里 0 命中（缺陷 495 的形状）" % key))
    return bad


def scan(mode="repo"):
    if mode == "self-test":
        d = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                          "..", "examples", "m285_gate_assertions", "fixtures"))
        return sorted(os.path.join(d, f) for f in os.listdir(d)) if os.path.isdir(d) else []
    out = []
    for base in SCAN_DIRS:
        d = os.path.join(ROOT, base)
        if not os.path.isdir(d):
            continue
        for dp, _, fs in os.walk(d):
            if "/.git" in dp or FIXTURE_SEG in dp + "/":
                continue
            for f in sorted(fs):
                if f.endswith(".sh"):
                    out.append(os.path.join(dp, f))
    return sorted(set(out))


def run_repo(verbose=True):
    files = scan("repo")
    patched, n_viol = set(), 0
    stats = collections.OrderedDict((k, 0) for k in
                                    ("files", "grep", "sed_i", "sed_app", "sed_unres",
                                     "a2_str", "a2_target", "a2_fallback",
                                     "a4_multi", "a4_waived"))
    for f in files:
        rel = os.path.relpath(f, ROOT)
        bad = scan_file(f, rel, patched, stats)
        n_viol += len(bad)
        if verbose:
            for code, ln, det in bad:
                print("  ❌ %s:%d [%s] %s" % (rel, ln, code, det))
        stats["grep"] += len(GREP_CALL.findall(read(f)))
    # ── A4 双向：豁免表里的键**必须**在实测多行清单里（否则「登记过期」）──
    _seen = stats.get("multi_seen") or set()
    for _k in sorted(load_multi_waiver()):
        if _k not in _seen:
            n_viol += 1
            if verbose:
                print("  ❌ %s [A4] 豁免登记过期：该处已不再锚点多行（表里有、实测没有）" % _k)
    if verbose:
        print("扫描 %d 个 .sh · grep %d · sed -i %d（可解析 %d / 跳过 %d）· "
              "判定串 %d（目标可解析 %d / 兜底 %d）· "
              "锚点多行 %d（豁免 %d / 整行替换 %d）· 违例 %d"
              % (stats["files"], stats["grep"], stats["sed_i"], stats["sed_app"],
                 stats["sed_unres"], stats["a2_str"], stats["a2_target"],
                 stats["a2_fallback"], stats["a4_multi"], stats["a4_waived"], stats.get("whole_line", 0), n_viol))
    return stats, n_viol


# ── 夹具串用**码点拼装**，而不是字面量 ────────────────────────────────
# 为什么（实测逼出来的）：反例夹具若以**字面量**写在源码里 ⇒ 本守卫**自己的源码**
# 就成了那个串的生产者 ⇒ `findable()` 判「在场」⇒ 反例**当场变正例**、自证假绿
# （实测：改前 4/2，两条 A2 反例全不报）。与 M284 v2 的自指陷阱同源，
# 也与 M282 缺陷 491 的修法同源（**夹具一律运行时拼装**，不设按文件的允许表）。
def _fab(*codes):
    return "".join(chr(c) for c in codes)


FAB_ANCHOR = _fab(0x865A, 0x6784, 0x7684, 0x951A, 0x70B9, 0x6CE8, 0x91CA, 0xFF1A, 0x7532, 0x4E59, 0x4E19)
FAB_CRIT = _fab(0x865A, 0x6784, 0x7684, 0x5224, 0x636E, 0x4E32, 0x7532, 0xFF1A, 0x4E59, 0x4E19, 0x4E01)

FIXTURES = {
    # 正例：锚点在位 · 串在目标文件里 · 反向断言（期望缺席）· 兜底面命中
    "ok_gate.sh": """#!/usr/bin/env bash
set -uo pipefail
sed -i 's|PXOP_ITERLEN|PXOP_ITERLEN_X|' runtime/vm.h
grep -q 'PXOP_ITERLEN' runtime/vm.h || { echo "打桩失败"; exit 2; }
grep -q '汇总：失败 0 项' "$W/run.log" || { bad "汇总行缺失"; }
grep -q '缺陷 440 · 已登记未修' examples/m128_unlock_grow/verify.sh && bad "旧措辞仍在"
grep -q '计算两数之和' "$W/doc.out" || bad "文档缺"
""",
    # 反例 A1：打桩锚点不在位（sed 静默不生效 ⇒ 负控失牙）
    "bad_a1.sh": """#!/usr/bin/env bash
sed -i 's|PXOP_THIS_ANCHOR_IS_GONE|PXOP_X|' runtime/runtime.h
""",
    # 反例 A2：判据串不在目标文件里（过期锚点）
    "bad_a2_file.sh": """#!/usr/bin/env bash
grep -q '%s' runtime/runtime.h || { bad "锚点缺失"; exit 2; }
""" % FAB_ANCHOR,
    # 反例 A2：凭空判据串（缺陷 495 的形状 —— 目标不可解析，兜底面也 0 命中）
    "bad_a2_pat.sh": """#!/usr/bin/env bash
if tail -60 /tmp/g.log | grep -q '%s'; then echo green; fi
""" % FAB_CRIT,
}


# ── M286（判据 A4）的夹具 ──────────────────────────────────────────────
#   ⚠️ 夹具串一律**纯 ASCII**：A2 的「凭空串」兜底只查 CJK ⇒ 不会与之互相干扰。
#   ⚠️ 目标文件放 fixtures/ ⇒ `scan()` 跳过该目录，但 `repo_path()` **能**解析到它
#      （这正是我们要的：夹具参与 `--file` 显式扫描，但不进真仓扫描面）。
FIXTURES286 = {
    # 打桩目标：OLD(`ANCHOR_MULTI_X`) 出现 **2 行**
    "a4_target.txt": "head\nANCHOR_MULTI_X\nmid\nANCHOR_MULTI_X\ntail\n",
    # 反例：**无地址** ⇒ sed 逐行替换 ⇒ 改 2 处（超出意图）⇒ A4 必须红
    "bad_a4_multi.sh": ("#!/usr/bin/env bash\n"
                        "sed -i 's|ANCHOR_MULTI_X|REPLACED|' "
                        "examples/m286_anchor_multi/fixtures/a4_target.txt\n"),
    # 正例：**范围地址** ⇒ 只有行 1..3 内那 1 处被改 ⇒ **必须不红**
    #   ⭐ 核心证据：文件里 OLD 仍在 2 行，判据必须**因为地址**而放过它
    "ok_a4_range.sh": ("#!/usr/bin/env bash\n"
                       "sed -i '/^head/,/^mid/ s|ANCHOR_MULTI_X|REPLACED|' "
                       "examples/m286_anchor_multi/fixtures/a4_target.txt\n"),

    # ── 以下三类把 A4 的**形状覆盖**补齐 ──────────────────────────────
    # 目标 2：范围内有 **2 处** OLD ⇒ 「有地址」**不等于**免死 ⇒ A4 必须红
    "a4_target2.txt": "BEGIN_TWO\nx ANCHOR_MULTI_Y\ny ANCHOR_MULTI_Y\nEND_TWO\nout ANCHOR_MULTI_Y\n",
    "bad_a4_range2.sh": ("#!/usr/bin/env bash\n"
                         "sed -i '/^BEGIN_TWO/,/^END_TWO/ s|ANCHOR_MULTI_Y|REPLACED|' "
                         "examples/m286_anchor_multi/fixtures/a4_target2.txt\n"),
    # 目标 3：**全文件有 OLD、但地址范围内没有** ⇒ A1 的「全文件在场」为真 ⇒ **A1 看不见**
    #   ⇒ 这就是 A1r 存在的理由（负控会**静默不生效**）
    "a4_target3.txt": "BEGIN_THREE\nnothing here\nEND_THREE\noutside ANCHOR_MULTI_W\n",
    "bad_a4_miss.sh": ("#!/usr/bin/env bash\n"
                       "sed -i '/^BEGIN_THREE/,/^END_THREE/ s|ANCHOR_MULTI_W|REPLACED|' "
                       "examples/m286_anchor_multi/fixtures/a4_target3.txt\n"),
    # **整行替换**惯用法（`Ns/.*/NEW/`）：OLD 是通配而非锚点 ⇒ **必须不判**
    #   （来源：SED_CALL 支持行地址后多抓出的 9 处真实写法）
    "ok_a4_whole.sh": ("#!/usr/bin/env bash\n"
                       "sed -i '1s/.*/TAMPERED/' "
                       "examples/m286_anchor_multi/fixtures/a4_target.txt\n"),
}


def self_test():
    d = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                      "..", "examples", "m285_gate_assertions", "fixtures"))
    d2 = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                       "..", "examples", "m286_anchor_multi", "fixtures"))
    for dd, ff in ((d, FIXTURES), (d2, FIXTURES286)):
        os.makedirs(dd, exist_ok=True)
        for name, body in ff.items():
            with open(os.path.join(dd, name), "w", encoding="utf-8") as fh:
                fh.write(body)
    ok = fail = 0
    want = {"ok_gate.sh": [], "bad_a1.sh": ["A1"], "bad_a2_file.sh": ["A2"], "bad_a2_pat.sh": ["A2"]}
    want2 = {"a4_target.txt": [], "bad_a4_multi.sh": ["A4"], "ok_a4_range.sh": [],
             "a4_target2.txt": [], "bad_a4_range2.sh": ["A4"],
             "a4_target3.txt": [], "bad_a4_miss.sh": ["A1r"],
             "ok_a4_whole.sh": []}
    for name in sorted(FIXTURES):
        p = os.path.join(d, name)
        codes = sorted({c for c, _, _ in scan_file(p, name, set())})
        if codes == want[name]:
            ok += 1
            print("  ✅ %-16s 判据=%s（期望 %s）" % (name, codes or "无", want[name] or "无"))
        else:
            fail += 1
            print("  ❌ %-16s 判据=%s（期望 %s）" % (name, codes or "无", want[name] or "无"))
            for c, ln, det in scan_file(p, name, set()):
                print("        %s L%d %s" % (c, ln, det))
    # ── M286 夹具（A4）──
    for name in sorted(FIXTURES286):
        p = os.path.join(d2, name)
        codes = sorted({c for c, _, _ in scan_file(p, name, set())})
        if codes == want2[name]:
            ok += 1
            print("  ✅ %-16s 判据=%s（期望 %s）" % (name, codes or "无", want2[name] or "无"))
        else:
            fail += 1
            print("  ❌ %-16s 判据=%s（期望 %s）" % (name, codes or "无", want2[name] or "无"))
            for c, ln, det in scan_file(p, name, set()):
                print("        %s L%d %s" % (c, ln, det))
    # 反向判据 ①：关掉 A1 的在场判定 ⇒ bad_a1 必须**不再**判红
    global pat_present
    keep = pat_present
    pat_present = lambda p, b: True
    codes = sorted({c for c, _, _ in scan_file(os.path.join(d, "bad_a1.sh"), "bad_a1.sh", set())})
    if codes == []:
        ok += 1
        print("  ✅ %-16s 关掉 A1 ⇒ 不再判红（红确实来自 A1）" % "反转-A1")
    else:
        fail += 1
        print("  ❌ %-16s 关掉 A1 后仍判红=%s" % ("反转-A1", codes))
    pat_present = keep
    # 反向判据 ②：关掉 A2 的在场判定 ⇒ bad_a2_pat 必须**不再**判红
    global criteria_verdict
    keep2 = criteria_verdict
    criteria_verdict = lambda p: (True, "关掉")
    codes = sorted({c for c, _, _ in scan_file(os.path.join(d, "bad_a2_pat.sh"), "bad_a2_pat.sh", set())})
    if codes == []:
        ok += 1
        print("  ✅ %-16s 关掉 A2 ⇒ 不再判红（红确实来自 A2）" % "反转-A2")
    else:
        fail += 1
        print("  ❌ %-16s 关掉 A2 后仍判红=%s" % ("反转-A2", codes))
    criteria_verdict = keep2
    # 反向判据 ③（M286）：关掉 A4（`count_old` 恒 1）⇒ 多行锚点必须**不再**判红
    #   ⇒ 证明 `bad_a4_multi.sh` 的红**确实来自 A4**，而不是别的判据顺带报的
    global count_old
    keep3 = count_old
    count_old = lambda lines, old, addr=None: 1
    codes = sorted({c for c, _, _ in scan_file(os.path.join(d2, "bad_a4_multi.sh"),
                                               "bad_a4_multi.sh", set())})
    if codes == []:
        ok += 1
        print("  ✅ %-16s 关掉 A4 ⇒ 多行锚点不再判红（红确实来自 A4）" % "反转-A4")
    else:
        fail += 1
        print("  ❌ %-16s 关掉 A4 后仍判红=%s" % ("反转-A4", codes))
    count_old = keep3
    print("\n自证：通过 %d / 失败 %d" % (ok, fail))
    return fail


def main():
    args = sys.argv[1:]
    if "--self-test" in args:
        return 1 if self_test() else 0
    if "--file" in args:
        i = args.index("--file")
        paths = args[i + 1:]
        if not paths:
            print("--file 需要路径", file=sys.stderr)
            return 4
        tot = 0
        for p in paths:
            if not os.path.isfile(p):
                print("  ⚠️ 不存在（跳过）：%s" % p)
                continue
            for code, ln, det in scan_file(p, p, set()):
                tot += 1
                print("  ❌ %s:%d [%s] %s" % (p, ln, code, det))
        print("外部脚本 %d 个 · 违例 %d" % (len(paths), tot))
        return 1 if tot else 0
    stats, n_viol = run_repo(verbose="--count" not in args)
    if "--count" in args:
        print("扫描 %d 个 .sh · grep %d · sed -i %d（可解析 %d）· 判定串 %d（目标 %d / 兜底 %d）"
              " · 锚点多行 %d（豁免 %d / 整行替换 %d）· 违例 %d"
              % (stats["files"], stats["grep"], stats["sed_i"], stats["sed_app"],
                 stats["a2_str"], stats["a2_target"], stats["a2_fallback"],
                 stats["a4_multi"], stats["a4_waived"], stats.get("whole_line", 0), n_viol))
    return 1 if n_viol else 0


if __name__ == "__main__":
    sys.exit(main())
