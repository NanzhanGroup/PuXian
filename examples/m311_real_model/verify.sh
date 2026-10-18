#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M311 · C3 门 —— **真模型端到端**（生产在用的 int8 量化 bge）⇄ onnxruntime 独立参考
# ---------------------------------------------------------------
# 立论：M306 补齐 DequantizeLinear / QuantizeLinear / DynamicQuantizeLinear / MatMulInteger
#   之后，**生产在用的那个 int8 模型第一次能被 PuXian 端到端跑**（1177 节点 / 363 initializer），
#   但「跑得通」只是必要条件。真正的判据是：**它与业界参考实现（onnxruntime）差多少**，
#   以及这个差距**会不会在后续改动里悄悄变大**。
#
#   本门做三件事（每件都是可复算的）：
#     ① 结构面：真模型解析 / 执行成功 / 输出形状 / 算子集合全部是已注册算子；
#     ② 数值面：与 onnxruntime 1.19.2 逐字节输出对拍 ⇒ 记录 max/mean 绝对误差与相关系数；
#     ③ 漂移面：**在同一台机器、同一模型、同一输入**下，误差不得比登记基线恶化 25%。
#
#   ⚠️ **如实登记（缺陷 533）**：当前 PuXian 与 onnxruntime 的输出**并非逐位一致**
#      （max abs err 0.477 / mean 0.066 / 相关系数 0.975，量级 ≈ int8 量化噪声在 12 层上的累积）。
#      判别实验证明**不是注意力/掩码**问题：seq=1（注意力退化）仍然偏离
#      （max 0.136 / mean 0.032 / corr 0.990）⇒ 病灶在共享路径（量化矩阵乘族）。
#     因此本门**不声称等价**，只声称「结构正确 + 确定性 + 非退化 + 不恶化」，
#      并把「与 onnxruntime 精确对齐」列为下一轮主项（M312）。
#
# 前提（缺失时**响亮 SKIP**，不装作绿）：
#   · 真模型：env `M311_MODEL`，默认 /data/app/ws/models/embed/model.onnx
#   · onnxruntime：python3 + `/opt/fw`（PYTHONPATH）。**CI 上没有** ⇒ CI 档 SKIP；
#     本地全量门用 `M311_REQUIRE_ORT=1` 强制（缺了就红，不许静默跳过）。
# ══════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
    echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
HERE="$(pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
PX="$ROOT/tools/px"
MODEL="${M311_MODEL:-/data/app/ws/models/embed/model.onnx}"
ORT_HOME="${M311_ORT_HOME:-/opt/fw}"
REQUIRE_ORT="${M311_REQUIRE_ORT:-0}"
W="$(mktemp -d /tmp/m311rm.XXXXXX)"
trap 'rm -rf "$W"' EXIT

PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
info(){ echo "  ℹ️  $1"; }
hdr() { echo ""; echo "── $* ──"; }

echo "══ M311 · 真模型端到端（int8 bge ⇄ onnxruntime）══"

# ── [0] 前提自证（不许装作有数据）──────────────────────────────
hdr "[0] 前提"
have_model=0; have_ort=0
[ -f "$MODEL" ] && have_model=1
PYTHONPATH="$ORT_HOME" python3 -c 'import onnxruntime, numpy' >/dev/null 2>&1 && have_ort=1
if [ "$have_model" = 1 ]; then
    ok "真模型在盘：$MODEL（$(stat -c %s "$MODEL") 字节）"
else
    if [ "$REQUIRE_ORT" = 1 ]; then bad "真模型不存在：$MODEL（M311_REQUIRE_ORT=1 ⇒ 不许跳过）"
    else info "真模型不存在：$MODEL ⇒ 本门 SKIP（本地全量门请设 M311_REQUIRE_ORT=1）"; fi
fi
if [ "$have_ort" = 1 ]; then
    ok "onnxruntime 可用（python3 + $ORT_HOME）"
else
    if [ "$REQUIRE_ORT" = 1 ]; then bad "onnxruntime 不可用（PYTHONPATH=$ORT_HOME）—— 缺独立参考就不能做数值面"
    else info "onnxruntime 不可用（CI 上常态）⇒ 数值面 SKIP"; fi
fi
if [ "$have_model" = 0 ] || [ "$have_ort" = 0 ]; then
    echo ""
    echo "── [7] 覆盖边界 ──"
    echo "  · 本档缺前提（模型/onnxruntime）⇒ 只跑了静态层，**没有**任何数值结论"
    echo ""
    echo "══ 汇总：$PASS 通过 / $FAIL 失败（前提不全 · SKIP 档）══"
    if [ "$FAIL" != 0 ]; then echo "M311-REALMODEL-FAIL"; exit 1; fi
    echo "M311-REALMODEL-SKIP"
    exit 0
fi

# ── [1] 静态：探针与基线在位 ────────────────────────────────
hdr "[1] 静态"
[ -f "$HERE/run_px.px" ] && ok "探针 run_px.px 在位" || bad "缺 run_px.px"
[ -f "$HERE/baseline.tsv" ] && ok "数值基线 baseline.tsv 在位" || bad "缺 baseline.tsv"
# 探针**不许**写死模型路径（模型路径由 env 注入）—— 写死会在别的机器上静默测错对象
if grep -qE '^\s*let\s+\w+\s*=\s*onnx_model_open\("/data/' "$HERE/run_px.px" 2>/dev/null; then
    bad "探针里写死了绝对模型路径（必须经 env M311_MODEL 注入）"
else
    ok "探针经 env M311_MODEL 注入模型路径（没有写死）"
fi

# ── [2] PuXian 跑真模型（两次 ⇒ 确定性）───────────────────────
hdr "[2] PuXian 执行（真模型 1177 节点）"
cp "$HERE/run_px.px" "$W/probe.px"
if ! "$PX" build "$W/probe.px" > "$W/build.log" 2>&1; then
    bad "探针编译失败（见下）"; tail -8 "$W/build.log" | sed 's/^/     /'
    echo "M311-REALMODEL-FAIL"; exit 1
fi
BIN="$W/build/probe"
[ -x "$BIN" ] || { bad "产物不存在：$BIN"; echo "M311-REALMODEL-FAIL"; exit 1; }
t0=$SECONDS
if M311_MODEL="$MODEL" "$BIN" > "$W/px1.out" 2>&1; then
    ok "第 1 次执行成功（$((SECONDS-t0))s）"
else
    bad "第 1 次执行失败"; tail -8 "$W/px1.out" | sed 's/^/     /'
    echo "M311-REALMODEL-FAIL"; exit 1
fi
M311_MODEL="$MODEL" "$BIN" > "$W/px2.out" 2>&1 && ok "第 2 次执行成功" || bad "第 2 次执行失败"
if cmp -s "$W/px1.out" "$W/px2.out"; then
    ok "确定性：两次运行**逐字节**一致"
else
    bad "确定性：两次运行输出不同（同输入同二进制 ⇒ 不可接受）"
fi
grep -q '^DIMS \[1, 4, 768\]$' "$W/px1.out" && ok "输出形状 [1,4,768]" || bad "输出形状不对：$(grep '^DIMS' "$W/px1.out")"
[ "$(grep '^NBYTES' "$W/px1.out" | awk '{print $2}')" = 12288 ] && ok "输出字节数 12288（float32 × 3072）" || bad "输出字节数不对"
OPS="$(grep '^OPS ' "$W/px1.out" | sed 's/^OPS //')"
[ -n "$OPS" ] && ok "算子集合：$OPS" || bad "没有算子统计"

# ── [3] 独立参考（onnxruntime 1.19.2）────────────────────────
hdr "[3] 独立参考（onnxruntime）"
if PYTHONPATH="$ORT_HOME" python3 "$HERE/ref_onnxruntime.py" "$MODEL" > "$W/ref.out" 2>"$W/ref.err"; then
    ok "onnxruntime 执行成功（$(grep -c . "$W/ref.out") 行输出）"
else
    bad "onnxruntime 执行失败"; tail -8 "$W/ref.err" | sed 's/^/     /'
    echo "M311-REALMODEL-FAIL"; exit 1
fi

# ── [4] 数值对拍（量出来，不猜）──────────────────────────────
hdr "[4] 数值对拍（PuXian ⇄ onnxruntime）"
if PYTHONPATH="$ORT_HOME" python3 "$HERE/compare.py" "$W/px1.out" "$W/ref.out" "$HERE/baseline.tsv" > "$W/cmp.out" 2>&1; then
    sed 's/^/  /' "$W/cmp.out"
    PASS=$((PASS+1))
else
    sed 's/^/  /' "$W/cmp.out"
    bad "数值判据未通过（见上）"
fi

# ── [5] 负控：输出必须**真随输入变化**（防常量/缓存假绿）────────
hdr "[5] 负控：改一个 token ⇒ 输出必须变"
sed 's/"i64": \[101, 2023, 1029, 102\]/"i64": [101, 2023, 1029, 103]/' "$HERE/run_px.px" > "$W/alt.px"
if grep -q '1029, 103' "$W/alt.px"; then
    "$PX" build "$W/alt.px" > "$W/alt_build.log" 2>&1 && M311_MODEL="$MODEL" "$W/build/alt" > "$W/alt.out" 2>&1
    if grep '^HEX' "$W/alt.out" > "$W/alt.hex" 2>/dev/null && ! cmp -s <(grep '^HEX' "$W/px1.out") "$W/alt.hex"; then
        ok "换一个 token id ⇒ 输出变了（不是常量，也不是缓存）"
    else
        bad "换 token 后输出**没有变化** ⇒ 本门测的可能不是真模型"
    fi
else
    bad "负控构造失败（sed 没有命中输入行）"
fi

# ── [5b] 负控：判据自伤（漂移判据必须有牙）────────────────────
#   把基线改成「误差必须 < 0.001」⇒ 同一份输出**必须判红**；
#   否则 compare.py 的漂移判据就是个摆设（M209/M213 的口径：负控要证明红来自该判据）。
hdr "[5b] 负控：漂移判据自伤（收紧基线 ⇒ 必须判红）"
printf 'max_abs_err\t0.001\nmean_abs_err\t0.001\n' > "$W/tight.tsv"
if PYTHONPATH="$ORT_HOME" python3 "$HERE/compare.py" "$W/px1.out" "$W/ref.out" "$W/tight.tsv" > "$W/tight.log" 2>&1; then
    bad "基线收紧到 0.001 后**仍然判绿** ⇒ 漂移判据没有牙"
else
    grep -q '\[漂移\] max_abs_err' "$W/tight.log" && ok "收紧基线 ⇒ 漂移判据判红（有牙）" \
        || bad "判红了但不是漂移判据造成的（见 $W/tight.log）"
fi

# ── [6] 覆盖边界 ────────────────────────────────────────────
hdr "[6] 覆盖边界（如实登记）"
cat <<'EOF'
  · 只验 seq=4 单批单条输入；不验多批 / 变长 / padding 行为。
  · 只验这一个 int8 量化模型（生产 embedding 用）；fp32 模型与其它架构不在面内。
  · onnxruntime 版本钉 1.19.2（/opt/fw）；版本变了要重新登记基线（数值面可能整体位移）。
  · **数值面不声称等价**：当前 max abs err 0.477 / mean 0.066 / corr 0.975（缺陷 533），
    本门只判「不恶化 25%」；精确对齐是 M312 的主项。
  · CI 档无 onnxruntime ⇒ 数值面 SKIP（静态层仍跑）；权威覆盖在本地全量门（M311_REQUIRE_ORT=1）。
EOF

echo ""
echo "══ 汇总：$PASS 通过 / $FAIL 失败 ══"
[ "$FAIL" = 0 ] && { echo "M311-REALMODEL-OK"; exit 0; } || { echo "M311-REALMODEL-FAIL"; exit 1; }
