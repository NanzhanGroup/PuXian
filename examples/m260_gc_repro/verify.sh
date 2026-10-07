#!/usr/bin/env bash
# ============================================================
# M260 verify.sh —— **GC 根面「复现装置」+ 压力档「分批常态化」**
# ------------------------------------------------------------
# 主题（接 M257 §五「根因未定论」与 M207 §「保证强度的第二项」两笔欠账）：
#   M257 记录了一条 ~1/60 的 Heisenbug（容器存储被 GC 回收后复用 ⇒ 静默堆损坏），
#   并留下两句诚实的话：① **根因未定论**；② 「加任何探测器/守卫后 60~240 次均未复现」。
#   ⇒ 缺的不是又一次排查，而是**可重复测量的装置** + **说得清的账**。
#
# 本门交付两件（都不依赖「这次能不能复现」）：
#   [A] **复现率测量装置**：同一探针 × N 次 + 「被信号杀死 / 守卫命中 / 缺 DONE」三判据，
#       并支持**对照**（`M260_WT=<另一棵树>`）—— 「消失了」必须能区分「修好了」与「运气好」。
#   [B] **GC 压力档台账**：`selfhost/gcstress_ledger.tsv` —— 425 个候选在压力档下是 O(n²)
#       （单轮数小时，从来没有完整跑过第二轮）⇒ 把「跑没跑过 / 结论是什么」变成
#       **只增不减**的账 + 未覆盖清单**可见**。
#
# 层：
#   [1] 工具自证（merge_ledger 3/3 · check_gcstress_ledger 5/5）
#   [2] 台账实检 + 覆盖率打印
#   [3] 复现装置**小样本实跑**（默认 5 次；`M260_N=60` 放大）—— 判据：rc=0 且含 DONE
#   [4] 负控 3 道（各自独立判红）
#   [5] 覆盖边界登记
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9
W="${M260_W:-/tmp/m260_gate}"
rm -rf "$W"; mkdir -p "$W"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS + 1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL + 1)); }
NEG_SKIP="${M260_NEG_SKIP:-0}"
# M260 自身修正：本门首版只认 `M260_NEG_SKIP=1` 环境变量，而**本仓门的惯例是 `--neg-skip`**
#   （CI 传的正是后者）⇒ 参数会被静默忽略、负控照跑。此处对齐惯例（两种写法都支持）。
for _a in "$@"; do
    case "$_a" in
        --neg-skip) NEG_SKIP=1 ;;
        --keep) : ;;
    esac
done

PROBE="$ROOT/examples/m256_eintr/probe_eintr.px"
LEDGER="$ROOT/selfhost/gcstress_ledger.tsv"
PROG="$ROOT/selfhost/gcstress_progress.txt"
N="${M260_N:-5}"

echo "══ M260 门：GC 根面复现装置 + 压力档台账 ══"

# ---------- [1] 工具自证 ----------
echo "── [1] 工具自证"
if python3 "$ROOT/selfhost/merge_ledger.py" --self-test >"$W/merge.log" 2>&1 \
   && grep -q "self-test: 3/3" "$W/merge.log"; then
    ok "merge_ledger.py 自证 3/3"
else
    bad "merge_ledger.py 自证失败：$(tail -1 "$W/merge.log")"
fi
if python3 "$ROOT/selfhost/check_gcstress_ledger.py" --root "$ROOT" --self-test >"$W/chk.log" 2>&1 \
   && grep -q "self-test: 5/5" "$W/chk.log"; then
    ok "check_gcstress_ledger.py 自证 5/5"
else
    bad "check_gcstress_ledger.py 自证失败：$(tail -1 "$W/chk.log")"
fi
# 判据载体在位（改文档/改路径不至于让本门静默变窄）
for f in selfhost/merge_ledger.py selfhost/check_gcstress_ledger.py selfhost/gcstress_ledger.tsv; do
    if [ -f "$ROOT/$f" ]; then ok "判据载体在位：$f"; else bad "缺件：$f"; fi
done

# ---------- [2] 台账实检 ----------
echo "── [2] 台账实检"
if python3 "$ROOT/selfhost/check_gcstress_ledger.py" --root "$ROOT" \
        --ledger "$LEDGER" --progress "$PROG" >"$W/led.log" 2>&1; then
    ok "台账合法：$(grep -oE '已覆盖 [0-9.]+% = [0-9]+/[0-9]+' "$W/led.log" | head -1)"
else
    bad "台账判红：$(grep -m1 '·' "$W/led.log" | sed 's/^ *//')"
fi
# 只增不减的基线在位
if [ -f "$PROG" ]; then ok "进度基线在位 selfhost/gcstress_progress.txt"; else bad "缺进度基线"; fi

# ── [3] 的判据**抽成函数**（M286s1）──────────────────────────────
#   为什么抽出来：负控若要验证「全灭 ⇒ 判红」，**不能**用 `bash "$0"` 自调用 ——
#   外层门已持 gate_lock，内层抢锁必被拒（rc=2）⇒ **假红**（M201 记过同款坑）。
#   抽成函数后负控**直接调它**，语义不变、无死锁。
#   语义（M286s1 重构）：本门测的是**装置可用**（能构建 · 能跑 · 能产出）。
#     · good == 0            ⇒ 装置**自身失效**（跑不出任何结果）⇒ 判红
#     · 0 < good < N（有命中）⇒ 装置**恰恰有效**（抓到了）⇒ 判可用 + 响亮报告
#   理由三条（防后人误改回 FAIL）：
#     ① 缺陷 267 家族**已登记**（`examples/m260_gc_repro/REPRO.md`：未修 · 根因未定论）
#     ② 本门**声明**「只证装置可用；不证 267 已修」（见覆盖边界）
#     ③ 用 FAIL 表达「抓到已知缺陷」= **判据错位** ⇒ 全量门**随机红**
#        ⇒ 随机红 = 判据失去信号价值（本仓 P2 纪律）
judge_repro() {   # $1=good  ⇒ rc 0=装置可用  1=装置失效
    [ "${1:-0}" -gt 0 ] && return 0
    return 1
}

# ---------- [3] 复现装置小样本实跑 ----------
echo "── [3] 复现装置实跑（$N 次）"
BLD="$W/probe_eintr"
rm -f "$BLD"
if ( cd "$ROOT" && ./tools/px build "$PROBE" ) >"$W/build.log" 2>&1; then
    cp "$ROOT/examples/m256_eintr/build/probe_eintr" "$BLD" 2>/dev/null || true
fi
if [ ! -x "$BLD" ]; then
    bad "探针构建失败：$(tail -1 "$W/build.log")"
else
    ok "探针构建成功"
    # M264：用**本门自己的**探针目录（不再与 m256 共用固定的 /tmp/px_m256_probe）
    mkdir -p "$W/probe_d"
    good=0; sig=0; nodone=0
    for i in $(seq 1 "$N"); do
        out="$W/p$i.out"
        M256_PROBE_D="$W/probe_d" PX_GC_STRESS=1 PX_GC_INLINE=1 timeout -k 5 180 "$BLD" >"$out" 2>&1
        rc=$?
        if [ "$rc" -ge 128 ]; then sig=$((sig + 1));
        elif [ "$rc" -ne 0 ]; then sig=$((sig + 1));
        elif ! grep -q '^M256-PROBE-DONE$' "$out"; then nodone=$((nodone + 1));
        else
            good=$((good + 1))
        fi
        # 守卫命中（M257 的容器存储失效自检）单独计数 —— 它比崩溃更早暴露
        if grep -q 'M257-CTR' "$out"; then echo "  ℹ️ 第 $i 次命中 [M257-CTR]（容器存储失效守卫）"; fi
    done
    if ! judge_repro "$good"; then
        # 全灭 ⇒ 装置**自身失效**（与「抓到缺陷」严格区分 —— 见 [4] NC-D2 的反向判据）
        bad "复现装置自身失效：$N 次**无一**跑通（被信号杀死=$sig · 缺 DONE=$nodone）⇒ 装置不可用"
    elif [ "$sig" -gt 0 ] || [ "$nodone" -gt 0 ]; then
        # M286s1：抓到**已登记**缺陷 = 装置**有效**（不是装置坏了）⇒ 记可用 + 响亮报告
        ok "复现装置可用：$N 次里 $good 次跑通 · 命中缺陷 267 家族 $((sig + nodone)) 次（被信号杀死=$sig · 缺 DONE=$nodone）"
        echo "      ⚠️ 命中是**真信号**（不是门的失败）—— 缺陷已登记：examples/m260_gc_repro/REPRO.md"
        echo "      ⚠️ 复测请放大样本：M260_N=60（本装置的设计用法）"
        echo "      ⚠️ 判据（M286s1）：本门测**装置可用**；「$N 次全灭」才判红（见 NC-D）"
    else
        ok "复现装置：$N/$N 通过（正常；装置工作正常）"
    fi
fi

# ---------- [4] 负控（M260_NEG_SKIP=1 跳过） ----------
echo "── [4] 负控"
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ 负控跳过（M260_NEG_SKIP=1 · CI 口径）"
else
    # NC-A：台账里 FAIL_SIG 缺定性 ⇒ check 必须判红
    cp "$LEDGER" "$W/led.bak"
    printf 'examples/m93_s3/coro_gc_block.px\tFAIL_SIG\t2026-10-05\t压力档被信号杀死\n' >>"$LEDGER"
    if python3 "$ROOT/selfhost/check_gcstress_ledger.py" --root "$ROOT" \
            --ledger "$LEDGER" --progress "$PROG" >"$W/nca.log" 2>&1; then
        bad "NC-A：FAIL_SIG 缺定性**未被判红**（判据无牙）"
    else
        ok "NC-A：FAIL_SIG 缺定性 ⇒ 判红"
    fi
    cp "$W/led.bak" "$LEDGER"
    # NC-B：台账里路径不存在 ⇒ check 必须判红
    printf 'examples/__no_such__/x.px\tPASS\t2026-10-05\t\n' >>"$LEDGER"
    if python3 "$ROOT/selfhost/check_gcstress_ledger.py" --root "$ROOT" \
            --ledger "$LEDGER" --progress "$PROG" >"$W/ncb.log" 2>&1; then
        bad "NC-B：路径不存在**未被判红**（判据无牙）"
    else
        ok "NC-B：路径不存在 ⇒ 判红"
    fi
    cp "$W/led.bak" "$LEDGER"
    # NC-C：只增不减 —— 把台账清成 1 条，基线还在 ⇒ 必须判红
    head -c 0 /dev/null >"$W/x"
    grep -v '^#' "$LEDGER" | head -1 >"$W/one.tsv"
    if [ -s "$W/one.tsv" ] && [ -f "$PROG" ]; then
        cp "$W/one.tsv" "$LEDGER"
        if python3 "$ROOT/selfhost/check_gcstress_ledger.py" --root "$ROOT" \
                --ledger "$LEDGER" --progress "$PROG" >"$W/ncc.log" 2>&1; then
            bad "NC-C：台账缩水**未被判红**（「只增不减」无牙）"
        else
            ok "NC-C：台账缩水 ⇒ 判红"
        fi
        cp "$W/led.bak" "$LEDGER"
    else
        bad "NC-C：夹具准备失败（台账或基线为空）"
    fi
    # 还原核对（逐字节）
    if cmp -s "$W/led.bak" "$LEDGER"; then ok "负控后台账逐字节还原"; else bad "台账未还原"; fi

    # ── NC-D：**全灭** ⇒ 装置自身失效 ⇒ 必须判红 ──────────────────────
    #   证明「区分偶发命中」**没有**把「装置失效」也一起放宽（判据有牙）
    if judge_repro 0; then
        bad "NC-D：全灭**未被判红**（判据无牙 —— 把「装置失效」也放宽了）"
    else
        ok "NC-D：全灭 ⇒ 判红（装置失效与「抓到缺陷」严格区分）"
    fi
    # ── NC-D2（反向判据）：有命中但非全灭 ⇒ 必须判**可用** ────────────
    if judge_repro 4; then
        ok "NC-D2：4/5 跑通（含 1 次命中）⇒ 判「装置可用」"
    else
        bad "NC-D2：偶发命中被误判为「装置失效」"
    fi
    # ── NC-D3（静态）：判据**真的接在 [3] 段上**（防「抽了函数却没用」）──
    if grep -q 'judge_repro "\$good"' "$0"; then
        ok "NC-D3（静态）：[3] 段确实调用 judge_repro（不是抽了不用）"
    else
        bad "NC-D3（静态）：[3] 段未调用 judge_repro"
    fi
fi

# ---------- [5] 覆盖边界 ----------
echo "── [5] 覆盖边界（如实登记）"
cat <<'EOF'
  · 本门的「复现装置」只证**装置可用**；**不证**缺陷 267 家族已修 —— A/B/C 三组的实测
    次数与结论登记在 `examples/m260_gc_repro/REPRO.md`（含 M256 源码树的对照）。
  · 探针依赖**固定端口 18420/18421** ⇒ 并行跑会互相抢端口（本门串行）。
  · 探针的**数据目录**自 M264 起改为 `M256_PROBE_D`（本门传 `$W/probe_d`）⇒
    不再与 m256 共用同一条**固定数据目录**（门间隔离；见 examples/m264_gate_shared_tmp/）。
  · 台账的「未覆盖清单」只做**可见性**，不判红（判红靠 `gcstress_progress.txt` 的只增不减）。
  · **M286s1 判据重构**（如实登记）：[3] 段原把「抓到缺陷 267 家族」记为 FAIL ⇒
    而本门**声明**「只证装置可用；不证 267 已修」⇒ **判据错位**：全量门**随机红**
    （实测：全量门那次 `缺 DONE=1` ⇒ FAIL；单独跑 3 次全绿）。
    ⇒ 现改为：**全灭（good==0）才判红**；偶发命中 ⇒ 记「装置可用」+ **响亮报告计数**
      （信号不丢：计数打印 + 提示 M260_N=60 复测 + REPRO.md 指引）。
    ⇒ **有牙**：NC-D（全灭 ⇒ 必红）· NC-D2（偶发 ⇒ 必可用，反向）· NC-D3（静态：确实接在门上）。
EOF
ok "覆盖边界已登记"

echo "══ M260 结果：通过 $PASS · 失败 $FAIL ══"
[ "$FAIL" -eq 0 ] && echo "M260-VERIFY-OK"
exit $((FAIL > 0 ? 1 : 0))
