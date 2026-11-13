#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# 发射冻结门（emit-c freeze）—— M153（第 35 轮）建立 · M157（第 39 轮）加"源码指纹"
#                               · M398（缺陷 761）升级为**传递依赖指纹**
# ---------------------------------------------------------------
# 为什么要有：`--check` / `--check-vm` / `bootstrap_prove` 判的都是 **compiler.px 自己**
#   的产物；而"改了 runtime/优化了发射路径，却**顺手动摇了别的程序的发射结果**"这一类
#   回归，没有任何门看得见（`selfhost/cases/*.px` + `examples/` 的产物没人守）。
#   第 34 轮起用临时脚本手查 253 个文件；本轮把它**变成常驻门**。
# 判据：对 `selfhost/cases/*.px` + `examples/*/*.px` 逐个跑 `bootstrap/pxc_vm --emit-c`，
#   sha256 必须与 `selfhost/golden/emitc_freeze.sha` 逐行一致（rc/输出都进指纹）。
# 口径：
#   · 纯性能/纯 runtime 改动 ⇒ 本门必须**全绿**（M153 就是这种情况：253/253 不变）。
#   · 有意改发射 ⇒ 用 `--freeze` 重定基并**在同一提交里说明为什么**。
#
# ⚠️ M157 加「源码指纹」的由来（缺陷 16x，第 39 轮开轮自查）：
#   第 37 轮末我做了一个「只动空格与空行」的 `px fmt` 提交（3f9fcab），
#   当时按「fmt 不改语义 ⇒ 与发射无关」判断它无害 —— **CI 质量门当场判红**。
#   实测原因：发射产物里带 **`PXOP_SRCLINE`（源码行号）** ⇒ 删空行会移动行号
#   ⇒ `--emit-c` 输出逐字节变化 ⇒ 冻结门正确地红了（`nul_io.px`：36d89a3c → 688d9349）。
#   **格式是「发射可见」的**：`px fmt` 与 `--emit-c` 不是两件无关的事。
#
# ⚠️ M398 升级为「传递依赖指纹」（缺陷 761，两天内造成两次误判）：
#   旧口径的 src 只对**入口文件**做 sha256。入口 import 的模块（selfhost/value.px、
#   stdlib/strings.px …）改了，入口 src 不变而产物变 ⇒ 一律判【类别 B · 必须查】。
#   实测影响面：524 个入口里 **50 个有传递依赖**，stdlib/strings.px 一个文件被 **18 个入口**依赖。
#   · M397 前：改 value.px ⇒ `--freeze` 只重定基了入口，漏了下游 golden（另一起事故）
#   · M397 r1：改 value.px ⇒ v01_value 判 B ⇒ 人工用对照实验才证明是预期发射变化
#   新口径：`dep` = 入口 sha + 递归 import 闭包 sha + 未解析 import 字面量 的合并指纹
#           （解析规则照 selfhost/cg_module.px；实现在 selfhost/freeze_deps.py）
#   类别判定：dep 变 + 产物变 ⇒ A（源码闭包已变，确认有意后 --freeze）
#             dep 未变 + 产物变 ⇒ B（**源码闭包一致而产物变，这是门的主职**）
#   判红时对每个差异文件跑 `freeze_deps.py --mode explain`：列出闭包清单 + 各自 sha
#   + **git 最近一次提交** ⇒ 谁改的、哪一天，一眼看见（不再靠人工对照实验）。
#
# 退出码：0 = 一致（或冻结完成）；1 = 有差异；3 = 环境异常（并发污染 / 指纹缺失）
# 用法：
#   bash selfhost/emitc_freeze.sh --check     # 门（默认）
#   bash selfhost/emitc_freeze.sh --freeze    # 重定基（需人工确认发射差异是预期的）
# ═══════════════════════════════════════════════════════════════════════
set -u
SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SELF_DIR/.."
# ⭐ M300s1（缺陷 519）：本脚本此前用**固定**临时路径（/tmp/emitc_freeze_now.sha 等）——
#   两个实例并发时会**交叉追加**（M300 实测：473 行 → 838/859/868/914 行，差异集每次不同）
#   ⇒ 门报**假红**（类别 B 虚假出现），甚至可能把被污染的集合写进基准。
#   修法两条：① 自持门级锁（与 run_gates 同源 ⇒ 直接调用也安全）；② 临时目录按 PID 唯一化。
. "$SELF_DIR/gate_lock.sh"
WORK=$(mktemp -d "/tmp/emitc_freeze.$$.XXXXXX") || { echo "❌ 无法创建临时目录" >&2; exit 3; }
trap 'rm -rf "$WORK"' EXIT
export EMITC_WORK="$WORK"
export LC_ALL=C LANG=C
# ⚠️ 三个 env 覆盖**只为门 m398_freeze_transitive 服务**（生产/CI 一律不设）：
#   EMITC_FREEZE_FILES   空格分隔的入口子集（默认 = 全量 524；门用 2 个跑秒级）
#   EMITC_FREEZE_GOLDEN  基准文件路径（默认 golden/emitc_freeze.sha；门用临时基准）
#   EMITC_BIN            被调用的编译器（默认 ./bootstrap/pxc_vm；门的负控指向 wrapper，
#                        **不替换真二进制** —— 替换二进制在中断时会毁掉工具链）
GOLDEN="${EMITC_FREEZE_GOLDEN:-selfhost/golden/emitc_freeze.sha}"
EMITC_BIN="${EMITC_BIN:-./bootstrap/pxc_vm}"
MODE="${1:---check}"
TMP="$WORK/now.sha"
SIGS="$WORK/sigs.tsv"
ERRTMP="$WORK/err.txt"

files() {
    if [ -n "${EMITC_FREEZE_FILES:-}" ]; then
        printf '%s\n' $EMITC_FREEZE_FILES
    else
        ls selfhost/cases/*.px examples/*/*.px 2>/dev/null
    fi
}

gen() {
    # M398：传递依赖指纹一次算全（524 次 python 启动会到分钟级）
    python3 selfhost/freeze_deps.py --mode sig $(files) > "$SIGS" 2>"$WORK/sig.err" || {
        echo "❌ 传递依赖指纹生成失败（selfhost/freeze_deps.py）：" >&2
        cat "$WORK/sig.err" >&2
        exit 3
    }
    : > "$TMP"
    for f in $(files); do
        line=$(awk -v k="$f" 'index($0, k "\t") == 1 { print; exit }' "$SIGS")
        src=$(printf '%s' "$line" | sed -E 's/.*src=([0-9a-f]+).*/\1/')
        dep=$(printf '%s' "$line" | sed -E 's/.*dep=([0-9a-f]+).*/\1/')
        case "$dep" in
            "" | *[!0-9a-f]*)
                echo "❌ 依赖指纹缺失或非法：$f（freeze_deps.py 未覆盖该入口）" >&2
                exit 3
                ;;
        esac
        out=$(timeout 60 "$EMITC_BIN" --emit-c "$f" 2>"$ERRTMP")
        rc=$?
        h=$(printf '%s' "$out" | sha256sum | cut -c1-32)
        eh=$(sha256sum < "$ERRTMP" | cut -c1-16)
        echo "$f src=$src dep=$dep rc=$rc out=$h err=$eh" >> "$TMP"
    done
}

# 逐行解析成「文件<TAB>字段=值」便于比较（文件名里带 '.'，不能用 '.' 当分隔符）
parse() {
    awk '{for(i=2;i<=NF;i++){split($i,a,"="); printf "%s\t%s=%s\n", $1, a[1], a[2]}}' "$1"
}

# 前提自证（M300 缺陷 519 / M283 精神）：集合含重复条目 = 并发污染的指纹
check_dup() {
    local t u
    t=$(wc -l < "$TMP")
    u=$(awk '{print $1}' "$TMP" | sort -u | wc -l)
    if [ "$t" -ne "$u" ]; then
        echo "❌ 集合含重复条目（$t 行 / $u 个唯一文件）—— 并发污染的指纹，不是发射差异" >&2
        echo "   本脚本自持门级锁；请确认没有旧的固定路径实例在跑之后重跑。" >&2
        exit 3
    fi
}

case "$MODE" in
--freeze)
    gen
    check_dup
    cp "$TMP" "$GOLDEN"
    echo "✅ 已重定基：$GOLDEN（$(wc -l < "$GOLDEN") 个文件 · src + dep 双指纹）"
    ;;
--check)
    [ -f "$GOLDEN" ] || { echo "❌ 缺基准文件 $GOLDEN（先 --freeze）"; exit 1; }
    gen
    check_dup
    if diff -q "$GOLDEN" "$TMP" > /dev/null; then
        echo "✅ 发射冻结：$(wc -l < "$TMP") 个文件的 --emit-c 输出与基准逐字节一致"
        exit 0
    fi
    parse "$GOLDEN" > "$WORK/base.kv"
    parse "$TMP" > "$WORK/now.kv"
    python3 - <<'PY'
import collections, os, sys
def load(p):
    d = collections.defaultdict(dict)
    for line in open(p, encoding='utf-8', errors='replace'):
        line = line.rstrip('\n')
        if not line: continue
        f, kv = line.split('\t', 1)
        k, v = kv.split('=', 1)
        d[f][k] = v
    return d
W = os.environ['EMITC_WORK']
base, now = load(W + '/base.kv'), load(W + '/now.kv')
# 判据列 = rc/out/err（**产物**）。src/dep 只用于**分类**，不单独判红。
def judge(d, f): return (d[f].get('rc'), d[f].get('out'), d[f].get('err'))
# M398：dep = 传递指纹（入口+闭包）。旧基准无 dep 列 ⇒ 退回入口 sha（旧语义），并提示升级。
has_dep = any('dep' in v for v in base.values())
def key(d, f): return d[f].get('dep') if has_dep else d[f].get('src')
only_base = sorted(set(base) - set(now))
only_now  = sorted(set(now) - set(base))
diag_only, regress, changed = [], [], []
for f in sorted(set(base) & set(now)):
    b, n = base[f], now[f]
    if judge(base, f) == judge(now, f):
        if b.get('dep') != n.get('dep') and b.get('src') != n.get('src'):
            diag_only.append(f)          # 源码变了但产物没变：无需重定基
        continue
    if key(base, f) == key(now, f):
        regress.append((f, b.get('out'), n.get('out'), b.get('rc'), n.get('rc')))
    else:
        changed.append((f, b.get('out'), n.get('out'), b.get('rc'), n.get('rc')))
hard = len(regress) + len(changed) + len(only_base) + len(only_now)
if hard == 0:
    print(f"✅ 发射冻结：{len(now)} 个文件的 --emit-c **产物**与基准逐字节一致")
    if diag_only:
        print(f"   ℹ️  {len(diag_only)} 件「源码闭包已变但产物未变」（无需重定基，仅指纹列过期）：{diag_only[:4]}")
        print(f"      如想让基准行也刷新：bash selfhost/emitc_freeze.sh --freeze")
    if not has_dep:
        print("   ⚠️  基准缺 dep 列（M398 前的旧格式）⇒ 本次按入口 sha 分类。"
              "建议跑一次 --freeze 升级（判据不变，只多一列传递指纹）")
    sys.exit(0)
print(f"❌ 发射冻结被打破：{hard} 个文件（判据 = rc/out/err）")
if regress:
    print(f"\n【类别 B · 发射行为回归】{len(regress)} 件 —— **源码闭包未变而产物变，必须查**：")
    print("   （门的主职：改了 runtime/发射路径却顺手动摇了别的程序）")
    for f, o1, o2, r1, r2 in regress[:15]: print(f"   {f}  out={o1} → {o2}  rc={r1}→{r2}")
if changed:
    print(f"\n【类别 A · 源码闭包已变】{len(changed)} 件 —— 确认改动有意后 `--freeze`：")
    print("   （含传递依赖：改 selfhost/value.px、stdlib/*.px 会让下游入口都落到这一类）")
    for f, o1, o2, r1, r2 in changed[:15]: print(f"   {f}  out={o1} → {o2}  rc={r1}→{r2}")
if only_base: print(f"\n【仅在基准里】{len(only_base)} 件（文件被删/改名？）：{only_base[:5]}")
if only_now:  print(f"\n【仅在当前里】{len(only_now)} 件（新增文件需 --freeze）：{only_now[:5]}")
# M398：归因提示写进文件，由外层 bash 调 explain（列出闭包 + git 最近提交）
with open(W + '/explain.list', 'w') as fh:
    for row in (regress + changed)[:15]:
        fh.write(row[0] + "\n")
print("\n   ⇒ " + ("**先查类别 B 的回归**，不要直接 --freeze" if regress else
      "类别 B 为空（无回归嫌疑）⇒ 确认改动有意后 --freeze（并在提交信息里说明）"))
sys.exit(1)
PY
    _rc=$?
    if [ -s "$WORK/explain.list" ]; then
        echo
        echo "═══ 归因（闭包清单 + git 最近一次提交）═══"
        while read -r ef; do
            [ -n "$ef" ] || continue
            echo "--- $ef"
            python3 selfhost/freeze_deps.py --mode explain "$ef" | sed 's/^/    /'
        done < "$WORK/explain.list"
    fi
    exit $_rc
    ;;
*)
    echo "用法：$0 [--check|--freeze]"; exit 2 ;;
esac
