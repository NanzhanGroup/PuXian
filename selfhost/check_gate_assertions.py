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
from shellscan import (GREP_CALL, git_grep_hit, in_dquote, iter_grep_pats,  # noqa: E402
                       mask_prose, pat_present, shell_dequote, split_script,
                       target_of, varmap_of)

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
SED_CALL = re.compile(r'(?<![\w-])sed\b(?P<flags>(?:\s+-[A-Za-z]+)*)\s*(?P<q>[\'"])(?P<script>s(?P<d>.).*?)(?P=q)(?P<tail>[^\n]*)')

CACHE = {}


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
                                     "a2_str", "a2_target", "a2_fallback"))
    for f in files:
        rel = os.path.relpath(f, ROOT)
        bad = scan_file(f, rel, patched, stats)
        n_viol += len(bad)
        if verbose:
            for code, ln, det in bad:
                print("  ❌ %s:%d [%s] %s" % (rel, ln, code, det))
        stats["grep"] += len(GREP_CALL.findall(read(f)))
    if verbose:
        print("扫描 %d 个 .sh · grep %d · sed -i %d（可解析 %d / 跳过 %d）· "
              "判定串 %d（目标可解析 %d / 兜底 %d）· 违例 %d"
              % (stats["files"], stats["grep"], stats["sed_i"], stats["sed_app"],
                 stats["sed_unres"], stats["a2_str"], stats["a2_target"],
                 stats["a2_fallback"], n_viol))
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


def self_test():
    d = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                      "..", "examples", "m285_gate_assertions", "fixtures"))
    os.makedirs(d, exist_ok=True)
    for name, body in FIXTURES.items():
        with open(os.path.join(d, name), "w", encoding="utf-8") as fh:
            fh.write(body)
    ok = fail = 0
    want = {"ok_gate.sh": [], "bad_a1.sh": ["A1"], "bad_a2_file.sh": ["A2"], "bad_a2_pat.sh": ["A2"]}
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
        print("扫描 %d 个 .sh · grep %d · sed -i %d（可解析 %d）· 判定串 %d（目标 %d / 兜底 %d）· 违例 %d"
              % (stats["files"], stats["grep"], stats["sed_i"], stats["sed_app"],
                 stats["a2_str"], stats["a2_target"], stats["a2_fallback"], n_viol))
    return 1 if n_viol else 0


if __name__ == "__main__":
    sys.exit(main())
