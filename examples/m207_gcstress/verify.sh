#!/usr/bin/env bash
# ============================================================
# M207 门（第 86 轮）：**GC 压力筛常态化** —— 把「一次性人工筛」变成
#   「可重复 · 可分批 · 可复现 · 可判据化」的常设能力。
# ------------------------------------------------------------
# 为什么需要它（M206 报告 §七 第 2 条）：M170/M182/M183/M206 修的 29 处漏登记
#   **全部**是「被某一次人工压力筛筛出来的」；「还剩多少」此前无法回答。
#
# 判据（两档差分 · 三跑）：
#   ① 正常档跑**两次**，两遍 stdout 必须逐字节一致（否则记 NONDET —— 程序自身不确定）；
#   ② 压力档（PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1）与正常档比：
#      stdout 逐字节一致 · rc 一致 · stderr 无 GC 检测器标记 · 未被信号杀死。
#
# 层：
#  ① 工具自证：`--help`（stdout·rc=0）· 未知选项（stderr·rc=2）· 候选规模下限 ·
#     **分批完备性**（3 批并集 == 全量，逐字节）· `--only` 生效 · 退出码语义
#  ② 精选语料（probe_gen / probe_env）两档必须全 PASS
#  ③ **正判据**：`--env PX_M207_PERTURB=1` 故意造出 stdout 差异 ⇒ 必须判 FAIL_OUT
#     （证明「逐字节比对」这一层真的有牙）
#  ④ 负控 A：撤 `px_gen_lazy` 的 `gc_register`（缺陷 264 复现）⇒ probe_gen 必红
#  ⑤ 负控 B：撤 `px_as_list` 的 `PX_KEEP`（缺陷 263 复现）⇒ probe_gen 必红
#  ⑥ 负控 C（判据自伤）：把本工具的逐字节比对改成恒真 ⇒ ③ 必须**不再红**
#  ⑦ 覆盖边界：如实登记「正常档就不通的语料不在本筛管辖区」（SKIP_NORM 带原因）
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m207_gate.XXXXXX)"
SWEEP="selfhost/gcstress_sweep.sh"
SNAP="$W/snap"
SRC_LIST="runtime/runtime.c"

snapshot() { mkdir -p "$SNAP"; for f in $SRC_LIST; do mkdir -p "$SNAP/$(dirname "$f")"; cp "$f" "$SNAP/$f"; done; }
restore_all() { for f in $SRC_LIST; do [ -f "$SNAP/$f" ] && cp "$SNAP/$f" "$f"; done; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

# 就地替换（唯一匹配；否则报错）
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

T_NORM=20
T_STRESS=60

# ============================================================
step "① 工具自证（判据有效性）"
# --help：显式请求的产品 ⇒ stdout + rc=0
if "$SWEEP" --help > "$W/help.out" 2> "$W/help.err"; then
    [ -s "$W/help.out" ] && [ ! -s "$W/help.err" ] && ok "--help：stdout 有内容、stderr 为空、rc=0" \
        || bad "--help 通道不符（§7.1）"
else
    bad "--help rc≠0"
fi
# 未知选项：失败诊断 ⇒ stderr + rc=2
set +e
"$SWEEP" --bogus > "$W/bogus.out" 2> "$W/bogus.err"; brc=$?
# set -e (M207: 本门全程用 set +e，避免中途退出)
if [ "$brc" -eq 2 ] && [ -s "$W/bogus.err" ] && [ ! -s "$W/bogus.out" ]; then
    ok "未知选项：stderr + rc=2"
else
    bad "未知选项口径不符（rc=$brc）"
fi
set +e

# 候选清单：规模下限 + 形状
"$SWEEP" --list > "$W/all.list" 2>/dev/null
N_ALL="$(wc -l < "$W/all.list")"
if [ "$N_ALL" -ge 300 ]; then ok "候选规模 $N_ALL ≥ 300（锚点：防清单静默变空）"; else bad "候选规模 $N_ALL < 300"; fi
if awk '!/^examples\/[^\/]+\/[^\/]+\.px$/{c++} END{exit (c>0)}' "$W/all.list"; then
    ok "候选形状全部为 examples/<门>/<文件>.px"
else
    bad "候选清单含非 .px 条目"
fi
# 分批完备性
for k in 1 2 3; do "$SWEEP" --list --batch "$k/3" 2>/dev/null; done | LC_ALL=C sort > "$W/batch.union"
LC_ALL=C sort "$W/all.list" > "$W/all.sorted"
if cmp -s "$W/batch.union" "$W/all.sorted"; then
    ok "分批完备性：3 批并集 == 全量（$N_ALL 行，逐字节）"
else
    bad "分批并集 != 全量"
fi
# --only 生效
"$SWEEP" --list --only 'examples/m207_gcstress/*.px' 2>/dev/null > "$W/only.list"
[ "$(wc -l < "$W/only.list")" -eq 2 ] && ok "--only 生效（2 个本门探针）" || bad "--only 计数不符"
# --known 生效（豁免表把候选记成 SKIP_KNOWN 且理由随行）
printf 'examples/m207_gcstress/probe_gen.px\t门内自证用临时豁免\n' > "$W/known.tsv"
"$SWEEP" --only 'examples/m207_gcstress/probe_*.px' --known "$W/known.tsv" --t-norm 5 --t-stress 10 \
    --out "$W/known.out" --quiet >/dev/null 2>&1
if grep -q '^SKIP_KNOWN.*probe_gen.px.*门内自证用临时豁免' "$W/known.out" \
   && grep -q '^PASS.*probe_env.px' "$W/known.out"; then
    ok "--known 生效（被豁免的记 SKIP_KNOWN + 理由；未列出的照常判定）"
else
    bad "--known 未生效"; sed 's/^/      /' "$W/known.out"
fi
# 真豁免表（随仓库入库）：规模上限 + 每条必须有理由
if [ -f "$HERE/KNOWN.tsv" ]; then
    N_KN="$(grep -vc '^#\|^$' "$HERE/KNOWN.tsv" || true)"
    N_BAD="$(awk -F'\t' '!/^#/ && NF>=1 && $1!="" {if (NF<2 || $2=="") c++} END{print c+0}' "$HERE/KNOWN.tsv")"
    if [ "$N_KN" -le 8 ] && [ "$N_BAD" -eq 0 ]; then
        ok "KNOWN.tsv：$N_KN 条豁免、全部带理由、且 ≤ 8（规模上限）"
    else
        bad "KNOWN.tsv 形态不符（条数 $N_KN / 缺理由 $N_BAD）"
    fi
else
    bad "缺 KNOWN.tsv"
fi

# ============================================================
step "② 精选语料：两档 stdout 必须逐字节一致"
set +e
"$SWEEP" --only 'examples/m207_gcstress/probe_*.px' --t-norm "$T_NORM" --t-stress "$T_STRESS" \
    --out "$W/base.tsv" --quiet > "$W/base.log" 2>&1; brc=$?
# set -e (M207: 本门全程用 set +e，避免中途退出)
N_PASS="$(grep -c '^PASS' "$W/base.tsv" || true)"
N_FAIL="$(grep -c '^FAIL' "$W/base.tsv" || true)"
if [ "$N_PASS" -eq 2 ] && [ "$N_FAIL" -eq 0 ]; then
    ok "probe_gen / probe_env 两档一致（PASS=$N_PASS · FAIL=0 · rc=$brc）"
else
    bad "精选语料未全绿（PASS=$N_PASS FAIL=$N_FAIL）"; sed 's/^/      /' "$W/base.tsv"
fi

# ============================================================
step "③ 正判据：故意造出 stdout 差异 ⇒ 必须判 FAIL_OUT（比对层有牙）"
set +e
"$SWEEP" --only 'examples/m207_gcstress/probe_env.px' --t-norm "$T_NORM" --t-stress "$T_STRESS" \
    --env PX_M207_PERTURB=1 --out "$W/pert.tsv" --quiet > "$W/pert.log" 2>&1; prc=$?
# set -e (M207: 本门全程用 set +e，避免中途退出)
if grep -q '^FAIL_OUT' "$W/pert.tsv" && [ "$prc" -eq 1 ]; then
    ok "差异被判 FAIL_OUT 且退出码 1（$(grep '^FAIL_OUT' "$W/pert.tsv" | head -1 | cut -c1-60)…）"
else
    bad "未判红（rc=$prc）—— 比对层失去牙？"; sed 's/^/      /' "$W/pert.tsv"
fi

if [ "$NEG_SKIP" = 1 ]; then
    echo
    echo "（--neg-skip：跳过负控 ④⑤⑥）"
else

# ============================================================
step "④ 负控 A：撤 px_gen_lazy 的 gc_register ⇒ probe_gen 必红（缺陷 264 复现）"
patch_one runtime/runtime.c \
    '    // M207（缺陷 264）：同 px_gen_from_list —— 字段填齐**之后**注册（注册点若发生 GC，
    //   对象已在 g_objs 且进 g_tmp_root ⇒ 其全部字段可被子对象递归标记）。
    gc_register(o, sizeof(LXObject));
' \
    '' \
    'A' || bad "负控 A 打补丁失败"
set +e
"$SWEEP" --only 'examples/m207_gcstress/probe_gen.px' --t-norm "$T_NORM" --t-stress "$T_STRESS" \
    --out "$W/na.tsv" --quiet > "$W/na.log" 2>&1
# set -e (M207: 本门全程用 set +e，避免中途退出)
if grep -q '^FAIL' "$W/na.tsv"; then
    ok "缺陷 264 复现：$(grep '^FAIL' "$W/na.tsv" | head -1 | cut -f1,3 | cut -c1-70)"
else
    bad "负控 A 未判红（该语料没走到这条路径？）"
fi
restore_all; snapshot

# ============================================================
step "⑤ 负控 B：撤 px_as_list 的 PX_KEEP ⇒ probe_gen 必红（缺陷 263 复现）"
patch_one runtime/runtime.c \
    '    px_root_push();
    PX_KEEP(l);
    for (int i = 0; i < n; i++) px_list_push(l, px_iter_at(v, px_int(i)));
    px_root_pop();' \
    '    for (int i = 0; i < n; i++) px_list_push(l, px_iter_at(v, px_int(i)));' \
    'B' || bad "负控 B 打补丁失败"
set +e
"$SWEEP" --only 'examples/m207_gcstress/probe_gen.px' --t-norm "$T_NORM" --t-stress "$T_STRESS" \
    --out "$W/nb.tsv" --quiet > "$W/nb.log" 2>&1
# set -e (M207: 本门全程用 set +e，避免中途退出)
if grep -q '^FAIL' "$W/nb.tsv"; then
    ok "缺陷 263 复现：$(grep '^FAIL' "$W/nb.tsv" | head -1 | cut -f1,3 | cut -c1-70)"
else
    bad "负控 B 未判红"
fi
restore_all; snapshot

# ============================================================
step "⑥ 负控 C（判据自伤）：比对改恒真 ⇒ ③ 必须不再红"
cp "$SWEEP" "$W/sweep.orig"
patch_one "$SWEEP" \
    '    if ! cmp -s "$WORK/n1.out" "$WORK/s.out"; then' \
    '    if false; then' \
    'C' || bad "负控 C 打补丁失败"
set +e
"$SWEEP" --only 'examples/m207_gcstress/probe_env.px' --t-norm "$T_NORM" --t-stress "$T_STRESS" \
    --env PX_M207_PERTURB=1 --out "$W/nc.tsv" --quiet > "$W/nc.log" 2>&1
# set -e (M207: 本门全程用 set +e，避免中途退出)
if grep -q '^FAIL' "$W/nc.tsv"; then
    bad "负控 C：撤掉比对后**仍**判红 ⇒ 红不是来自逐字节比对（判据归属不明）"
else
    ok "撤掉逐字节比对后不再判红 ⇒ ③ 的红确由比对层给出（判据归属清晰）"
fi
cp "$W/sweep.orig" "$SWEEP"
cmp -s "$W/sweep.orig" "$SWEEP" && ok "工具源已逐字节还原" || bad "工具源未还原"

fi  # --neg-skip

# ============================================================
step "⑦ 覆盖边界（如实登记）"
{
    echo "  本筛只对「正常档能跑通且自身确定」的语料负责："
    echo "    SKIP_NORM = 正常档就不通（常驻服务 / 需要参数 / 需要 fixture / 负样例）"
    echo "    NONDET    = 正常档两遍自身不一致（计时/并发类）"
    echo "    BUILDFAIL = 构建失败"
    echo "  全量筛（379 候选）的分布由 selfhost/gcstress_sweep.sh 输出，"
    echo "  本门只锁「工具 + 判据 + 精选语料 + 负控」四件事。"
}

echo
echo "【M207 门】通过 $PASS · 失败 $FAIL"
[ "$FAIL" -eq 0 ] && echo "M207-VERIFY-OK" || echo "M207-VERIFY-FAIL"
exit $([ "$FAIL" -eq 0 ] && echo 0 || echo 1)
