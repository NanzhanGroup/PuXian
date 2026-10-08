#!/usr/bin/env bash
# ============================================================
# M291 门（第 168 轮）：GC 压力筛**分批轮转**常设化
#   （M207 待办第 ② 项 · M290 结论的直接下一步）
# ------------------------------------------------------------
# 为什么（M290 的结论）：M207 把压力筛做成「可重复 · 可分批 · 可复现」的工具，
#   但它**仍然要人手动跑** ——「还剩多少没扫」依旧没人回答。
#   M170/182/183/206/207 修的三十余处漏登记，**全部**是「被某一次人工压力筛筛出来的」；
#   M290 复跑 136 轮 0 异常 ⇒ **瓶颈在「复现」而不是「取证」**
#   ⇒ 本轮做的是「让它自己冒出来」：每跑一轮全量门自动推进一批，N 轮覆盖全量。
#
# ⚠️ 判据的语义（**别误读**）：门跑的是「台账里的下一批」⇒
#   门绿 = **本批**无 FAIL_*，**不是**「全量绿」。全量覆盖靠**轮转累积**。
#   故本门**不断言**「跑的是第几批」—— 那是台账的自由度，断言它会变成「不可复现」；
#   本门断言的是：**工具的机械性质**（可复现）+ **本批的分类结果**。
#
# 层：
#  ① 工具自证（机械 · 可复现）：--help 通道 · 未知选项 rc=2 ·
#     **--status 无副作用** · --batch 参数校验 3 例 · 台账损坏 3 例 ⇒ rc=3
#  ② 分批完备性（承 M207）：`--list --batch k/7`（k=1..7）⇒ 并集 == 全量 · 两两无交
#  ③ 轮转机械自证（独立台账 · N=3 + 探针）：批号 1→2→3 · runs=3 ·
#     第 3 次后 covered 满 ⇒ rounds=1 且覆盖记录清空 ⇒ 三个探针全部 PASS
#  ④ 正判据：`--env PX_M291_PERTURB=1` 故意造出 stdout 差异
#     ⇒ 驱动器**必须** rc=1 **且指名**候选（证「它真的在看 sweep 的判定」）
#  ⑤ 负控 A：撤掉驱动器的 FAIL 退出码 ⇒ ④ 必须**不再红**（判据自伤证明）
#  ⑥ 负控 B：`--status` 加副作用（建台账）⇒ ① 的「无副作用」判据必须红
#  ⑦ 负控 C：台账损坏改成静默回退到 1 ⇒ ① 的 rc=3 判据必须红
#  ⑧ **真跑一批**（用默认台账 ⇒ 推进真实轮转）；`--no-real` 跳过（CI 用，见 §⑨）
#  ⑨ 覆盖边界登记（如实）
#
# 用法：verify.sh [--neg-skip] [--no-real]
#   --neg-skip 跳过负控（CI 用 · 本仓通用语义）
#   --no-real  跳过第 ⑧ 层（CI 用：CI 的 /tmp 每次都全新 ⇒ 永远只跑第 1 批，
#              轮转在 CI 上**不成立**；真正的轮转在**本地连续跑**才有意义）
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9
export LC_ALL=C LANG=C

NEG_SKIP=0; NO_REAL=0
for a in "$@"; do
    [ "$a" = "--neg-skip" ] && NEG_SKIP=1
    [ "$a" = "--no-real" ] && NO_REAL=1
done

W="$(mktemp -d /tmp/m291_gate.XXXXXX)"
ROT="selfhost/gcrotate.sh"
SWEEP="selfhost/gcstress_sweep.sh"
PPAT='examples/m291_gcrotate/probe/*.px'
SNAP="$W/snap"

snapshot()   { mkdir -p "$SNAP"; cp "$ROT" "$SNAP/rot.sh"; }
restore_all(){ [ -f "$SNAP/rot.sh" ] && cp "$SNAP/rot.sh" "$ROT"; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

# 就地替换（唯一匹配；否则报错）—— 与 m207 门同款
patch_one() {   # $1=file $2=old $3=new $4=tag
    python3 - "$1" "$2" "$3" "$4" <<'PY' || exit 1
import sys
p, old, new, tag = sys.argv[1:5]
s = open(p, encoding='utf-8').read()
n = s.count(old)
if n != 1:
    print("  !! [%s] 锚点匹配 %d 次（要求 1）" % (tag, n)); sys.exit(1)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
}
chk() {   # $1=desc $2=期望 $3=实际
    if [ "$2" = "$3" ]; then ok "$1（= $3）"; else bad "$1（期望 $2，实际 $3）"; fi
}

# ══════════════════════════════════════════════════════════
step "① 工具自证（机械性质 · 可复现）"

T1="$W/t1"; mkdir -p "$T1"
export GCROTATE_STATE="$T1/state" GCROTATE_RESULTS="$T1/res.tsv" GCROTATE_COVERED="$T1/cov.list"

"$ROT" --help > "$W/help.out" 2> "$W/help.err"; hrc=$?
if [ "$hrc" = 0 ] && grep -q 'gcrotate' "$W/help.out"; then ok "--help ⇒ rc=0 且 stdout 有用法"; else bad "--help（rc=$hrc）"; fi

"$ROT" --bogus > "$W/bog.out" 2> "$W/bog.err"; brc=$?
if [ "$brc" = 2 ] && [ -s "$W/bog.err" ]; then ok "未知选项 ⇒ rc=2 且 stderr 有提示"; else bad "未知选项（rc=$brc）"; fi

# --status **无副作用**（这是「只读入口」的硬要求 —— 同 run_gates.sh --list 的 M244 口径）
rm -rf "$T1"; mkdir -p "$T1"
export GCROTATE_STATE="$T1/state" GCROTATE_RESULTS="$T1/res.tsv" GCROTATE_COVERED="$T1/cov.list"
"$ROT" --status > "$W/st.out" 2>&1; src=$?
leftover="$(ls -A "$T1" 2>/dev/null | wc -l)"
if [ "$src" = 0 ] && [ "$leftover" = 0 ] && grep -q '^GCROTATE-STATUS' "$W/st.out"; then
    ok "--status ⇒ rc=0 · 输出状态行 · **不建任何文件**（剩余文件数 0）"
else
    bad "--status 有副作用或输出异常（rc=$src · 剩余文件 $leftover）"; sed 's/^/      /' "$W/st.out"
fi

# --batch 参数校验 3 例（0/5 · 7/5 · abc）—— **必须 rc=2**，不许静默回退
for spec in "0/5" "7/5" "abc"; do
    "$ROT" --batch "$spec" > /dev/null 2> "$W/bsp.err"; sprc=$?
    if [ "$sprc" = 2 ]; then ok "--batch $spec ⇒ rc=2"; else bad "--batch $spec 期望 rc=2，实际 $sprc"; fi
done

# 台账损坏 3 例 ⇒ rc=3（**响亮**；静默回退会把「台账坏了」伪装成「第一圈」）
for bad_st in "batch=999" "n=0" "batch=xyz"; do
    rm -rf "$T1"; mkdir -p "$T1"
    printf '%s\nn=20\nrounds=0\nruns=0\n' "$bad_st" > "$T1/state"
    export GCROTATE_STATE="$T1/state" GCROTATE_RESULTS="$T1/res.tsv" GCROTATE_COVERED="$T1/cov.list"
    "$ROT" --status > /dev/null 2> "$W/st.err"; dcrc=$?
    if [ "$dcrc" = 3 ]; then ok "台账损坏（$bad_st）⇒ rc=3"; else bad "台账损坏（$bad_st）期望 rc=3，实际 $drc"; fi
done

# ══════════════════════════════════════════════════════════
step "② 分批完备性（k/7 并集 == 全量 · 两两无交）"
"$SWEEP" --list 2>/dev/null | LC_ALL=C sort > "$W/full"
nfull="$(wc -l < "$W/full")"
: > "$W/union"
for k in 1 2 3 4 5 6 7; do
    "$SWEEP" --list --batch "$k/7" 2>/dev/null | LC_ALL=C sort > "$W/bk$k"
    cat "$W/bk$k" >> "$W/union"
done
LC_ALL=C sort "$W/union" > "$W/union.s"
if cmp -s "$W/full" "$W/union.s"; then ok "并集 == 全量（$nfull 候选，逐字节）"; else bad "并集 != 全量"; fi
dup=0
for k in 1 2 3 4 5 6 7; do
    other=0
    for j in 1 2 3 4 5 6 7; do [ "$j" = "$k" ] || other=$((other + $(wc -l < "$W/bk$j"))); done
done
# 两两无交：各批之和 == 全量 且 并集 == 全量 ⇒ 无重复
sum=0
for k in 1 2 3 4 5 6 7; do sum=$((sum + $(wc -l < "$W/bk$k"))); done
chk "各批规模之和 == 全量" "$nfull" "$sum"
nprobe="$(ls -1 examples/m291_gcrotate/probe/*.px 2>/dev/null | wc -l)"
if [ "$nprobe" -ge 3 ]; then ok "自证探针规模下限（$nprobe ≥ 3）"; else bad "自证探针仅 $nprobe 个（应 ≥3）"; fi

# ══════════════════════════════════════════════════════════
step "③ 轮转机械自证（独立台账 · N=3 + 探针）"
T3="$W/t3"; mkdir -p "$T3"
export GCROTATE_STATE="$T3/state" GCROTATE_RESULTS="$T3/res.tsv" GCROTATE_COVERED="$T3/cov.list"
# ⚠️ `--only` 只**收窄候选集**，不改 N —— N 必须显式设（首版漏了 ⇒ 批号走 k/20 ⇒ 实测 next=4）
export GCROTATE_N=3
seq_ok=1; allpass=1
for round in 1 2 3; do
    out="$("$ROT" --next --only "$PPAT" 2>&1)"; rc=$?
    [ "$rc" = 0 ] || { seq_ok=0; echo "      round $round rc=$rc"; }
    grep -q 'pass=1' <<< "$out" || allpass=0
    if [ "$round" -lt 3 ]; then
        # 跑满前两批：覆盖累积 1/3 → 2/3
        grep -q "^GCROTATE-PROGRESS covered=$round/3" <<< "$out" || { seq_ok=0; echo "      round $round 覆盖行异常"; }
    else
        # 第 3 批跑完 = **一圈完成** ⇒ 覆盖记录清空（0/3）+ 打印「一圈完成」
        grep -q "^GCROTATE-PROGRESS covered=0/3" <<< "$out" || { seq_ok=0; echo "      round 3 覆盖行应为 0/3（一圈完成即清空）"; }
        grep -q '一圈完成' <<< "$out" || { seq_ok=0; echo "      round 3 未报「一圈完成」"; }
    fi
done
chk "三轮 runs=3" "3" "$(awk -F= '$1=="runs"{print $2}' "$T3/state")"
chk "三轮后 rounds=1（一圈完成）" "1" "$(awk -F= '$1=="rounds"{print $2}' "$T3/state")"
chk "一圈完成后覆盖记录清空" "0" "$(wc -l < "$T3/cov.list" 2>/dev/null || echo 0)"
chk "批号回到 1（next=1/3）" "1" "$(awk -F= '$1=="batch"{print $2}' "$T3/state")"
[ "$seq_ok" = 1 ] && ok "三轮批号序列 1→2→3 且 rc 全 0" || bad "轮转序列异常"
[ "$allpass" = 1 ] && ok "三个探针在压力档下全部 PASS" || bad "探针未全 PASS"

# 覆盖记录确实按批累积（第 2 轮前 covered=1）
T3b="$W/t3b"; mkdir -p "$T3b"
export GCROTATE_STATE="$T3b/state" GCROTATE_RESULTS="$T3b/res.tsv" GCROTATE_COVERED="$T3b/cov.list"
"$ROT" --next --only "$PPAT" > /dev/null 2>&1
chk "跑完 1 批后 covered=1" "1" "$(wc -l < "$T3b/cov.list" 2>/dev/null || echo 0)"
"$ROT" --status --only "$PPAT" 2>/dev/null | grep -q 'covered=1/3' \
    && ok "--status 报出累计覆盖（1/3）" || bad "--status 覆盖统计不符（--only 必须同时传给 --status）"

# ══════════════════════════════════════════════════════════
step "④ 正判据：故意造出 stdout 差异 ⇒ 驱动器必须 rc=1 且**指名**"
T4="$W/t4"; mkdir -p "$T4"
export GCROTATE_STATE="$T4/state" GCROTATE_RESULTS="$T4/res.tsv" GCROTATE_COVERED="$T4/cov.list"
# N=1 ⇒ 一批即全部 3 个探针（正判据要覆盖完整，而不是每批只取 1 个）
export GCROTATE_N=1
export GCROTATE_EXTRA='--env PX_M291_PERTURB=1'
out4="$("$ROT" --next --only "$PPAT" 2>&1)"; rc4=$?
unset GCROTATE_EXTRA
chk "驱动器 rc=1" "1" "$rc4"
grep -q 'FAIL_OUT' <<< "$out4" && ok "本批含 FAIL_OUT" || bad "未报 FAIL_OUT"
grep -q 'probe_a.px' <<< "$out4" && ok "**指名**候选（probe_a.px）" || bad "未指名候选"
grep -q 'gcrotate.sh --batch' <<< "$out4" && ok "给出复现命令" || bad "未给复现命令"

# ══════════════════════════════════════════════════════════
step "⑤ 负控 A：撤「FAIL ⇒ rc=1」⇒ ④ 必须**不再红**（判据自伤）"
if [ "$NEG_SKIP" = 1 ]; then echo "  ⏭ 负控 A/B/C（--neg-skip）"
else
    patch_one "$ROT" '    echo "   复现：selfhost/gcrotate.sh --batch $K/$N     （同批再跑一次）"' \
                     '    echo "   复现：selfhost/gcrotate.sh --batch $K/$N     （同批再跑一次）"
    exit 0' NEG-A
    T5="$W/t5"; mkdir -p "$T5"
    export GCROTATE_STATE="$T5/state" GCROTATE_RESULTS="$T5/res.tsv" GCROTATE_COVERED="$T5/cov.list"
    export GCROTATE_EXTRA='--env PX_M291_PERTURB=1'
    "$ROT" --next --only "$PPAT" > /dev/null 2>&1; rc5=$?
    unset GCROTATE_EXTRA
    chk "负控 A 下 rc=0（红消失 ⇒ ④ 的红确实来自该判据）" "0" "$rc5"
    restore_all
fi

# ══════════════════════════════════════════════════════════
step "⑥ 负控 B：--status 加副作用 ⇒ ① 的「无副作用」判据必须红"
if [ "$NEG_SKIP" = 1 ]; then echo "  ⏭ 负控 B（--neg-skip）"
else
    patch_one "$ROT" 'if [ "$MODE" = status ]; then' \
                     'if [ "$MODE" = status ]; then
    : > "$STATE"   # NEGCTL-B：故意加副作用' NEG-B
    TN="$W/tn"; mkdir -p "$TN"
    export GCROTATE_STATE="$TN/state" GCROTATE_RESULTS="$TN/res.tsv" GCROTATE_COVERED="$TN/cov.list"
    "$ROT" --status > /dev/null 2>&1
    nb="$(ls -A "$TN" 2>/dev/null | wc -l)"
    if [ "$nb" != 0 ]; then ok "负控 B 下文件数 $nb ≠ 0（判据确有牙 ⇒ ① 的红来自该判据）"; else bad "负控 B 未生效（仍 0 文件）"; fi
    restore_all
fi

# ══════════════════════════════════════════════════════════
step "⑦ 负控 C：台账损坏改静默回退 ⇒ ① 的 rc=3 判据必须红"
if [ "$NEG_SKIP" = 1 ]; then echo "  ⏭ 负控 C（--neg-skip）"
else
    patch_one "$ROT" 'echo "gcrotate: 台账损坏：batch=$K 越界（应在 1..$N）（$STATE）" >&2; exit 3' \
                     'K=1' NEG-C
    TC="$W/tc"; mkdir -p "$TC"
    printf 'batch=999\nn=20\nrounds=0\nruns=0\n' > "$TC/state"
    export GCROTATE_STATE="$TC/state" GCROTATE_RESULTS="$TC/res.tsv" GCROTATE_COVERED="$TC/cov.list"
    "$ROT" --status > /dev/null 2>&1; rc7=$?
    chk "负控 C 下 rc=0（原期望 3 ⇒ 判据确有牙）" "0" "$rc7"
    restore_all
fi

# 源逐字节还原（负控收尾 · 与快照比对 —— 别拿 git diff 比 HEAD：本树本就是未提交态）
if cmp -s "$SNAP/rot.sh" "$ROT"; then ok "驱动器源已逐字节还原"; else bad "驱动器源未还原"; fi

# ══════════════════════════════════════════════════════════
step "⑧ 真跑一批（推进真实轮转台账）"
if [ "$NO_REAL" = 1 ]; then
    echo "  ⏭ 跳过（--no-real：CI 的 /tmp 每次全新 ⇒ 永远只跑第 1 批，轮转在 CI 上不成立）"
else
    unset GCROTATE_STATE GCROTATE_RESULTS GCROTATE_COVERED GCROTATE_EXTRA GCROTATE_N
    out8="$("$ROT" 2>&1)"; rc8=$?
    echo "$out8" | sed 's/^/  /'
    if [ "$rc8" = 0 ]; then ok "真跑批 rc=0（本批无 FAIL_*）"
    else bad "真跑批 rc=$rc8 —— 本批出现 FAIL_*（见上方逐条：真缺陷 ⇒ 立编号；假阳 ⇒ 进 KNOWN/SLOW 表）"; fi
    grep -q '^GCROTATE-RESULT' <<< "$out8" && ok "输出机器可读结果行" || bad "缺 GCROTATE-RESULT 行"
    grep -q '^GCROTATE-PROGRESS' <<< "$out8" && ok "输出累计进度行" || bad "缺 GCROTATE-PROGRESS 行"
fi

# ══════════════════════════════════════════════════════════
step "⑨ 覆盖边界（如实登记）"
echo "  • 本门只证「**本批**无 FAIL」+「工具机械性质」；**全量覆盖靠轮转累积**（N=20 ⇒ 20 轮一圈）"
echo "  • 本门**不断言**「跑的是第几批」—— 那是台账自由度；要复现某批请用 --batch K/N"
echo "  • STIMEOUT / NONDET **不算红**（前者是 O(n²) 慢、后者是程序自身不确定）—— 只在输出里记账"
echo "  • 探针在 probe/ 子目录 ⇒ 不进真轮转候选集（sweep 的 PAT 不递归）"
echo "  • CI 用 --no-real：CI 的 /tmp 每次全新，轮转在 CI 上不成立（真轮转在本地连续跑）"

echo
if [ "$FAIL" = 0 ]; then echo "M291-VERIFY-OK（通过 $PASS / 失败 0）"; exit 0
else echo "M291-VERIFY-FAIL（通过 $PASS / 失败 $FAIL）"; exit 1; fi
