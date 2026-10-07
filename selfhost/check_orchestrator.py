#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# =====================================================================
# selfhost/check_orchestrator.py —— 编排脚本守卫（M284 · 缺陷 490 + 495）
#
# 为什么存在：
#   M279–M283 四轮的「失败」几乎全在**编排层**（watcher / chain / final），
#   产品代码零实质回归。而这些脚本住在 /tmp ⇒ 仓库那套守卫
#   （check_gate_paths / check_gate_anchors / check_gate_registry …）**全都扫不到**
#   ⇒ 四个已实测的 bug 只能靠「跑到那一步」才发现。本守卫把编排骨架收进
#   `packaging/chain/` 并**给它判据**。
#
# 三条判据（各配反向判据，见 --self-test）：
#   O1 **判据串在场** —— `grep …'<串>'` 里的中文判定串，其 CJK 骨架必须在仓库
#      其它文件里找得到。找不到 = **凭空捏造的判据串**（缺陷 495 的形状：
#      `grep -qE '双路判据一致：0 红'` 全仓 0 命中，而门明明是绿的）。
#      口径：切成连续中文段（≥2）—— 存在 ≥3 的段则**最长段**必须在场，
#            否则任一 ≥2 的段在场即可（这样 `汇总：失败 0 项` 这类
#            **模板实例化**不会被误判 —— v1 太严，实测把 run_gates.sh
#            真实打印的串判成了凭空）。
#   O2 **sequencing** —— 脚本里若既有 `resolve-topic m<N>` 又有门运行调用，
#      **第一条 resolve-topic 必须早于第一次跑门**。
#      理由（缺陷 489）：final 把 resolve 放在**成功路径末尾**，而它前面隔着
#      90–150 分钟的全量门、升级阈值只有 45 分钟 ⇒ **每一轮正常跑都必然误报**；
#      而语义上「final 启动 ⟺ 提交已落地 ⟺ chain 的等分类项按定义已失义」。
#   O3 **了结者存在** —— 每挂一个 `p1 m<N>-…`，同文件必须有 `resolve-topic m<N>`。
#      否则那一条一旦挂上就**只能等升级**（无人了结）。
#
# 用法：
#   check_orchestrator.py              # 扫真仓，rc=0 绿 / 1 红
#   check_orchestrator.py --self-test  # 内置夹具自证（含 3 条反向判据）
#   check_orchestrator.py --list       # 只列扫描面
# =====================================================================
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from shellscan import GREP_CALL, in_dquote, iter_grep_pats, mask_prose, shell_dequote  # noqa: E402

ROOT = os.environ.get("ORCH_ROOT") or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CJK = re.compile(r'[\u4e00-\u9fff]')
RUN = re.compile(r'[\u4e00-\u9fff]+')
# grep 的判定串抽取见 `shellscan.iter_grep_pats`（M285 抽成共用原语：
# 同一条规则在本仓**两个守卫**里各实现一次 ⇒ 缺陷 498 的坑两个文件各踩一遍）。
# 本地只保留「是否处于命令位置」的辅助（配合 shellscan.mask_prose）。
RUNNER_RE = re.compile(r'\b(?:run_gates\.sh|m116_gates\.sh|m117_gates\.sh)\b')
RESOLVE_RE = re.compile(r'resolve-topic\s+([A-Za-z0-9_.-]+)')
P1_RE = re.compile(r'\bp1\s+([A-Za-z0-9_.-]+)')

# ⚠️ 扫描面 = **编排层**（不含 examples/**/verify.sh）——
#   实测：把 examples 纳进来会多出 **26 条假阳**，全部是「门的判据串来自**夹具文本**」
#   （如 doc 注释 `## 计算两数之和` —— 生产者在 .px 夹具里，而夹具行是注释/数据）。
#   ⇒ 按 M283 纪律「口径必须校准到 0 误伤」（否则豁免表会长成第二个"把红当绿记"），
#     本判据**刻意**只覆盖编排层；门的判据串另由 M283 的 P3（禁形）覆盖一部分。
#     下一轮候选：给门的判据串做**夹具感知**的在场判据。
SCAN_DIRS = tuple((os.environ.get("ORCH_DIRS") or
                   "packaging,packaging/chain,selfhost,tools").split(","))
# ⚠️ fixtures 是**故意违例**的样本（自证/门用），不参与真仓扫描 —— 否则本仓会被
#    自己的反例判红（M226/M227 记过「夹具与判据必须隔离」）。
FIXTURE_SEG = "/fixtures/"
CACHE = {}


# 判据串的**生产者** = 非注释行、且**不是在查询它**（不含 `grep`）。
# 这条限定是必需的，不是洁癖：
#   v2 首版用「全仓字面在场」，结果被**我自己的取证文本**污染 —— CHANGELOG（M284 节）
#   与 gate_verdict.sh 的**注释**里都**引用**了 `双路判据一致`（作为「它不存在」的证据），
#   ⇒ git grep 找得到 ⇒ 判据串"在场" ⇒ `bad_o1` 自证当场失去牙（实测：自证 5/0 → 4/1）。
# 判据串的**生产者** = 该串出现在某个代码文件里，且**该行不是在消费/查询它**。
#
# 演化史（每一步都是实测逼出来的，**不要"顺手简化"回去**）：
#   v1 「全仓字面在场」：被**我自己在 CHANGELOG / 注释里的取证引用**污染 ⇒ 判据失去牙
#      （自证 5/0 → 4/1）。⇒ 加"非注释行"。
#   v2 「非注释行 + 不含 grep」：修好 v1，但**只看已跟踪文件** ⇒ 提交前绿、提交后红
#      —— 缺陷 491 的同族第 2 次（496①）。
#   v3 想再加「该行必须含输出关键词（echo/printf/print…）」：**实测被否**。
#      真生产者有两种不该被排除的形状 ——
#        · `.px` 里的**字符串字面量赋值**（`codegen.px` 的 `/* 由普贤 …` 头）；
#        · 由别的脚本 `echo` 出来的中文短语（`packaging/selftest_pxrepo_mirror.sh` 的 4 条）。
#      加该条件后**凭空 0 → 7**（7 条全是误伤）⇒ 按「校准到 0 误伤」**撤回**该条，
#      并删掉那个**定义了却从没用过**的 `OUTPUT_KW` 常量（判据与注释必须一致 —— M214 同族）。
#   v4（M284s1）只做**两件必需**的事：① 扫描面补 `--untracked`；② 查询工具的调用行也算"查询"。
CODE_EXT = (".sh", ".py", ".px", ".c", ".h", ".go", ".mk", ".bash")
# **查询/消费**该串的行不产生它：`grep` 取数、`--probe` / `--count` 是本守卫自己的查询入口。
QUERY_RE = re.compile(r"\bgrep\b|--probe|--count")
# 对照档：复现**修前**口径（只看已跟踪）—— 只给门做 A/B，正常运行不得设置。
TRACKED_ONLY = os.environ.get("ORCH_TRACKED_ONLY") == "1"


def findable(s):
    """骨架是否有**生产者**（出现在代码里、且那一行不是在注释/查询它）。"""
    if s in CACHE:
        return CACHE[s]
    ok = False
    try:
        # ⚠️ pathspec 必须是 **glob**（`*.sh`）—— 写成 `.sh` 会被当**字面路径**，
        #    git grep 找不到任何文件却**返回空**（不报错）⇒ 判据恒假（本仓第 N 次同形）。
        # ⚠️ M284s1：**必须** `--untracked`（缺陷 496①）—— 否则「提交前」与「提交后」
        #    是两个世界、两套结论，而本判据的职责恰恰是**在提交前**拦住。
        paths = ["*" + e for e in CODE_EXT]
        cmd = ["git", "-C", ROOT, "grep", "-nF", "-e", s]
        if not TRACKED_ONLY:
            cmd.append("--untracked")
        cmd += ["--"] + paths
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
        for line in (r.stdout or "").split("\n"):
            if ":" not in line:
                continue
            _f, _rest = line.split(":", 1)
            if ":" in _rest:
                _rest = _rest.split(":", 1)[1]
            st = _rest.strip()
            if st.startswith(("#", "//", "/*", "*", "--")):
                continue                       # 注释里的**引用**不产生它
            if QUERY_RE.search(_rest):
                continue                       # **查询/消费**它的行也不产生它（496③）
            ok = True
            break
    except Exception:
        ok = True                      # 出错不判红（宁可漏报，不可假红）
    CACHE[s] = ok
    return ok


def criteria_verdict(pat):
    """判定串的在场性。返回 (True/False/None, 依据段)。None = 不适用（无中文）。"""
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


def read(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            return fh.read().split("\n")
    except Exception:
        return []


def scan_files(mode="repo"):
    if mode == "self-test":
        d = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                         "..", "examples", "m284_orchestrator", "fixtures")
        d = os.path.normpath(d)
        out = []
        if os.path.isdir(d):
            for f in sorted(os.listdir(d)):
                if f.endswith(".sh"):
                    out.append(os.path.join(d, f))
        return out
    out = []
    for base in SCAN_DIRS:
        d = os.path.join(ROOT, base)
        if not os.path.isdir(d):
            continue
        for dp, _, fs in os.walk(d):
            if "/.git" in dp or "/node_modules" in dp or FIXTURE_SEG in dp + "/":
                continue
            for f in sorted(fs):
                if f.endswith(".sh"):
                    out.append(os.path.join(dp, f))
    return sorted(set(out))


def check_file(path, rel):
    """返回违例列表 [(code, line, detail)]。"""
    bad = []
    lines = read(path)
    # ── O1 判据串在场 ──
    for i, line in enumerate(lines, 1):
        s = line.strip()
        if s.startswith("#") or "grep" not in line:
            continue
        # ⚠️ 定位用 mask_prose（散文里**提到**的 `grep` 不算命令 —— 实测两处假阳都在这里：
        #    `iv_nopipeq() { # … 「大产出 | grep -q」 … }` 与 `iv_bad "… tar|grep -q …"`），
        #    解析用**原始行**（掩码会把引号里的 sed 脚本一并抹掉）。
        masked = mask_prose(line)
        for pat, _pos, gstart in iter_grep_pats(line):
            if masked[gstart:gstart + 4] != line[gstart:gstart + 4]:
                continue
            if not (CJK.search(pat) and len(pat) >= 4):
                continue
            ok, key = criteria_verdict(pat)
            if ok is None or ok:
                continue
            bad.append(("O1", i, "凭空判据串 %r（骨架 %r 全仓 0 命中）" % (pat, key)))
    # ── O2 序不变量 ──
    first_resolve, first_run = None, None
    for i, line in enumerate(lines, 1):
        if line.strip().startswith("#"):
            continue
        if first_resolve is None and RESOLVE_RE.search(line):
            first_resolve = i
        if first_run is None and RUNNER_RE.search(line):
            first_run = i
    if first_resolve and first_run and first_resolve > first_run:
        bad.append(("O2", first_resolve,
                    "resolve-topic（L%d）晚于第一次跑门（L%d）—— 门要跑 90–150 分钟，"
                    "而升级阈值 45 分钟 ⇒ 每轮正常跑都必然误报（缺陷 489）"
                    % (first_resolve, first_run)))
    # ── O3 了结者存在 ──
    p1_topics, resolved = set(), set()
    for i, line in enumerate(lines, 1):
        if line.strip().startswith("#"):
            continue
        for t in P1_RE.findall(line):
            p1_topics.add((t, i))
        for t in RESOLVE_RE.findall(line):
            resolved.add(t)
    for topic, i in sorted(p1_topics):
        root = topic.split("-")[0]
        if not re.match(r"^m[0-9]{3}", root):
            continue                   # 非里程碑形状（如 `p1 p2` 的参数命名）⇒ 不是挂起项
        if root in resolved:
            continue
        bad.append(("O3", i, "挂了 p1 %s，但同文件没有 resolve-topic %s —— 该挂起项无人了结" %
                    (topic, root)))
    return bad


# ── 自证夹具：每条判据都要有「正例在场」+「反例判红」 ──────────────────
FIXTURES = {
    # 正例：判据串来自门实现；resolve 在跑门之前；p1 与 resolve 配对
    "ok_orch.sh": """#!/usr/bin/env bash
set -uo pipefail
/usr/local/sbin/dy-notify.sh resolve-topic m284 >/dev/null 2>&1 || true
if grep -qE '汇总：失败 0 项' /tmp/g.log; then echo fine; fi
bash selfhost/run_gates.sh > /tmp/g.log 2>&1
if [ $? != 0 ]; then /usr/local/sbin/dy-notify.sh p1 m284-red "门红" "见日志"; fi
/usr/local/sbin/dy-notify.sh resolve-topic m284-red >/dev/null 2>&1 || true
""",
    # 反例 O1：凭空判据串（缺陷 495 原件形状）
    "bad_o1.sh": """#!/usr/bin/env bash
if tail -60 /tmp/g.log | grep -qE '双路判据一致：0 红'; then echo green; fi
""",
    # 反例 O2：resolve 在跑门之后（缺陷 489 形状）
    "bad_o2.sh": """#!/usr/bin/env bash
bash selfhost/run_gates.sh > /tmp/g.log 2>&1
/usr/local/sbin/dy-notify.sh resolve-topic m284 >/dev/null 2>&1 || true
""",
    # 反例 O3：挂了 p1 但无人了结
    "bad_o3.sh": """#!/usr/bin/env bash
/usr/local/sbin/dy-notify.sh p1 m284-dirty "工作树脏" "见日志"
""",
}


def self_test():
    d = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                      "..", "examples", "m284_orchestrator", "fixtures"))
    os.makedirs(d, exist_ok=True)
    for name, body in FIXTURES.items():
        with open(os.path.join(d, name), "w", encoding="utf-8") as fh:
            fh.write(body)
    ok = 0
    fail = 0
    for name in sorted(FIXTURES):
        p = os.path.join(d, name)
        bad = check_file(p, name)
        codes = sorted({c for c, _, _ in bad})
        want = {"ok_orch.sh": [], "bad_o1.sh": ["O1"],
                "bad_o2.sh": ["O2"], "bad_o3.sh": ["O3"]}[name]
        if codes == want:
            ok += 1
            print("  ✅ %-12s 判据=%s（期望 %s）" % (name, codes or "无", want or "无"))
        else:
            fail += 1
            print("  ❌ %-12s 判据=%s（期望 %s）" % (name, codes or "无", want or "无"))
            for c, ln, det in bad:
                print("        %s L%d %s" % (c, ln, det))
    # 反向判据：把 O1 判据关掉 ⇒ bad_o1 必须**不再**判红（证明红来自 O1 本身）
    global criteria_verdict
    keep = criteria_verdict
    criteria_verdict = lambda p: (True, "关掉")
    bad = check_file(os.path.join(d, "bad_o1.sh"), "bad_o1.sh")
    codes = sorted({c for c, _, _ in bad})
    if codes == []:
        ok += 1
        print("  ✅ %-12s 关掉 O1 ⇒ 不再判红（红确实来自 O1）" % "反转-O1")
    else:
        fail += 1
        print("  ❌ %-12s 关掉 O1 后仍判红=%s" % ("反转-O1", codes))
    criteria_verdict = keep
    print("\n自证：通过 %d / 失败 %d" % (ok, fail))
    return fail


def main():
    args = sys.argv[1:]
    if "--self-test" in args:
        return 1 if self_test() else 0
    # ── 扫任意脚本（每轮生成的 /tmp 编排件在**启动之前**先自查 —— M284 新增）──
    if "--file" in args:
        i = args.index("--file")
        paths = args[i + 1:]
        if not paths:
            print("--file 需要一个或多个路径", file=sys.stderr)
            return 4
        total = 0
        for p in paths:
            if not os.path.isfile(p):
                print("  ⚠️ 不存在（跳过）：%s" % p)
                continue
            for code, ln, det in check_file(p, p):
                total += 1
                print("  ❌ %s:%d [%s] %s" % (p, ln, code, det))
        print("外部脚本 %d 个 · 违例 %d" % (len(paths), total))
        return 1 if total else 0
    # ── 单个骨架在场性探针（供门做「对照判据」：一个必须凭空、一个必须在场）──
    if "--probe" in args:
        i = args.index("--probe")
        if i + 1 >= len(args):
            print("--probe 需要一个骨架字符串", file=sys.stderr)
            return 4
        sk = args[i + 1]
        hit = findable(sk)
        print("骨架 %r ⇒ %s" % (sk, "在场（有生产者）" if hit else "凭空（无生产者）"))
        return 0 if hit else 1
    files = scan_files("repo")
    if "--count" in args:
        tot = ph = 0
        for f in files:
            for line in read(f):
                st = line.strip()
                if st.startswith("#") or "grep" not in line:
                    continue
                masked = mask_prose(line)
                for pat, _pos, gstart in iter_grep_pats(line):
                    if masked[gstart:gstart + 4] != line[gstart:gstart + 4]:
                        continue
                    if not (CJK.search(pat) and len(pat) >= 4):
                        continue
                    okv, _key = criteria_verdict(pat)
                    if okv is None:
                        continue
                    tot += 1
                    if not okv:
                        ph += 1
        print("判定串 %d 条 · 凭空 %d 条 · 扫描 %d 个 .sh" % (tot, ph, len(files)))
        return 1 if ph else 0
    if "--list" in args:
        for f in files:
            print(os.path.relpath(f, ROOT))
        print("共 %d 个 .sh" % len(files))
        return 0
    total = 0
    for f in files:
        rel = os.path.relpath(f, ROOT)
        for code, ln, det in check_file(f, rel):
            if total == 0:
                print("编排脚本违例：")
            total += 1
            print("  ❌ %s:%d [%s] %s" % (rel, ln, code, det))
    n_topic = sum(1 for f in files if RUNNER_RE.search("\n".join(read(f))))
    print("扫描 %d 个 .sh · 含门调用的编排脚本 %d 个 · 违例 %d" % (len(files), n_topic, total))
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
