#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# 发射冻结门（emit-c freeze）—— M153（第 35 轮）建立 · M157（第 39 轮）加"源码指纹"
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
#   但那次的差异报告只给"有差异"，说不清是「源码变了（预期）」还是「发射回归（危险）」
#   ⇒ 本门现在把**源码 sha256** 也写进基准行：
#     · src 变 + out 变 ⇒ 类别 A「源码已变」：确认改动有意后 `--freeze`
#     · src 未变 + out 变 ⇒ 类别 B「发射行为回归」：**必须查**（这是本门的主职）
#   退出码：0 = 一致（或冻结完成）；1 = 有差异
# 用法：
#   bash selfhost/emitc_freeze.sh --check     # 门（默认）
#   bash selfhost/emitc_freeze.sh --freeze    # 重定基（需人工确认发射差异是预期的）
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")/.."
export LC_ALL=C LANG=C
GOLDEN=selfhost/golden/emitc_freeze.sha
MODE="${1:---check}"
TMP=/tmp/emitc_freeze_now.sha

files() { ls selfhost/cases/*.px examples/*/*.px 2>/dev/null; }

gen() {
    : > "$TMP"
    for f in $(files); do
        src=$(sha256sum "$f" | cut -c1-16)
        out=$(timeout 60 ./bootstrap/pxc_vm --emit-c "$f" 2>/tmp/emitc_freeze_err.txt)
        rc=$?
        h=$(printf '%s' "$out" | sha256sum | cut -c1-32)
        eh=$(sha256sum < /tmp/emitc_freeze_err.txt | cut -c1-16)
        echo "$f src=$src rc=$rc out=$h err=$eh" >> "$TMP"
    done
}

# 逐行解析成「文件<TAB>字段=值」便于比较（文件名里带 '.'，不能用 '.' 当分隔符）
parse() {
    awk '{for(i=2;i<=NF;i++){split($i,a,"="); printf "%s\t%s=%s\n", $1, a[1], a[2]}}' "$1"
}

case "$MODE" in
--freeze)
    gen
    cp "$TMP" "$GOLDEN"
    echo "✅ 已重定基：$GOLDEN（$(wc -l < "$GOLDEN") 个文件 · 含源码指纹）"
    ;;
--check)
    [ -f "$GOLDEN" ] || { echo "❌ 缺基准文件 $GOLDEN（先 --freeze）"; exit 1; }
    gen
    # 旧格式（无 src=）兼容：**不**自动重定基（那等于橡皮图章），只按旧语义比 rc/out/err。
    if ! grep -q ' src=' "$GOLDEN"; then
        sed 's/ src=[0-9a-f]*//' "$TMP" > /tmp/emitc_freeze_now_oldfmt.sha
        if diff -q "$GOLDEN" /tmp/emitc_freeze_now_oldfmt.sha > /dev/null; then
            echo "✅ 发射冻结：$(wc -l < "$TMP") 个文件的 --emit-c 输出与基准逐字节一致"
            echo "   ℹ️  基准为旧格式（无源码指纹）⇒ 建议跑一次 --freeze 升级（只加 src= 一列，判据不变）"
            exit 0
        fi
        echo "❌ 发射冻结被打破（旧格式基准，按 rc/out/err 比）："
        diff "$GOLDEN" /tmp/emitc_freeze_now_oldfmt.sha | head -20 | sed 's/^/   /'
        echo "   ⇒ 若差异是**有意的**发射改动：bash selfhost/emitc_freeze.sh --freeze（并在提交信息里说明）"
        exit 1
    fi
    if diff -q "$GOLDEN" "$TMP" > /dev/null; then
        echo "✅ 发射冻结：$(wc -l < "$TMP") 个文件的 --emit-c 输出与基准逐字节一致"
        exit 0
    fi
    parse "$GOLDEN" > /tmp/emitc_freeze_base.kv
    parse "$TMP"    > /tmp/emitc_freeze_now.kv
    python3 - <<'PY'
import collections, sys
def load(p):
    d = collections.defaultdict(dict)
    for line in open(p, encoding='utf-8', errors='replace'):
        line = line.rstrip('\n')
        if not line: continue
        f, kv = line.split('\t', 1)
        k, v = kv.split('=', 1)
        d[f][k] = v
    return d
base, now = load('/tmp/emitc_freeze_base.kv'), load('/tmp/emitc_freeze_now.kv')
# 判据列 = rc/out/err（**产物**）；src 只是诊断列 ⇒ 源码变了但产物没变 **不判红**。
def judge(d, f): return (d[f].get('rc'), d[f].get('out'), d[f].get('err'))
only_base = sorted(set(base) - set(now))
only_now  = sorted(set(now) - set(base))
src_only, regress, rcerr = [], [], []
for f in sorted(set(base) & set(now)):
    b, n = base[f], now[f]
    if judge(base, f) == judge(now, f):
        if b.get('src') != n.get('src'): src_only.append(f)
        continue
    if b.get('src') == n.get('src'): regress.append((f, b.get('out'), n.get('out'), b.get('rc'), n.get('rc')))
    else:                            rcerr.append((f, b.get('out'), n.get('out'), b.get('rc'), n.get('rc')))
hard = len(regress) + len(rcerr) + len(only_base) + len(only_now)
if hard == 0:
    print(f"✅ 发射冻结：{len(now)} 个文件的 --emit-c **产物**与基准逐字节一致")
    if src_only:
        print(f"   ℹ️  {len(src_only)} 件「源码已变但产物未变」（无需重定基，仅 src 列过期）：{src_only[:4]}")
        print(f"      如想让基准行也刷新：bash selfhost/emitc_freeze.sh --freeze")
    sys.exit(0)
print(f"❌ 发射冻结被打破：{hard} 个文件（判据 = rc/out/err）")
if regress:
    print(f"\n【类别 B · 发射行为回归】{len(regress)} 件 —— **源码未变而产物变了，必须查**：")
    print("   （门的主职：改了 runtime/发射路径却顺手动摇了别的程序）")
    for f, o1, o2, r1, r2 in regress[:15]: print(f"   {f}  out={o1} → {o2}  rc={r1}→{r2}")
if rcerr:
    print(f"\n【类别 A · 源码与产物同时变】{len(rcerr)} 件 —— 确认改动有意后 `--freeze`：")
    print("   （提示：纯 fmt/空行改动也在这里 —— 行号 PXOP_SRCLINE 是产物的一部分，见脚本头注）")
    for f, o1, o2, r1, r2 in rcerr[:15]: print(f"   {f}  out={o1} → {o2}  rc={r1}→{r2}")
if only_base: print(f"\n【仅在基准里】{len(only_base)} 件（文件被删/改名？）：{only_base[:5]}")
if only_now:  print(f"\n【仅在当前里】{len(only_now)} 件（新增文件需 --freeze）：{only_now[:5]}")
print("\n   ⇒ " + ("**先查类别 B 的回归**，不要直接 --freeze" if regress else
      "类别 B 为空（无回归嫌疑）⇒ 确认改动有意后 --freeze（并在提交信息里说明）"))
sys.exit(1)
PY
    exit $?
    ;;
*)
    echo "用法：$0 [--check|--freeze]"; exit 2 ;;
esac
