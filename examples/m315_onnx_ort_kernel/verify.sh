#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M315 门 —— MatMulInteger(u8s8) 的**真值方向**：规范 ⇄ PuXian ⇄ onnxruntime
# ---------------------------------------------------------------
# 立论（缺陷 533 的收尾）：
#   M311 立的判据是「PuXian ⇄ onnxruntime，误差不许比基线恶化 25%」，
#   于是把一个**会出错的实现**当成了真值 —— 0.477 的偏差被记成「我们的数值缺陷」（缺陷 533）。
#   M314 侦察用「把 72 个 MatMulInteger 换成等价子图」做判别：ORT 与自己改写后的版本
#   相差 max 0.477362 / mean 0.066037，与 M311 记录的 PuXian⇄ORT **逐位吻合**；
#   而 PuXian ⇄ 改写后 ORT 只差 **5e-6**（float32 舍入级）。
#   ⇒ 真值必须是**规范定义**，不是任何一个实现。
#
# 本门守四件事：
#   [1] 装置可复现：重跑生成器与入仓装置**逐字节**一致（防手改装置 / 防生成器漂移）
#   [2] **PuXian ⇄ 规范真值逐位一致**（硬判据 · **CI 上就能跑**，不需要 onnxruntime）
#   [3] onnxruntime ⇄ 规范的**双向登记核对**：A 含 >127 且 K>1 必须偏离；否则必须一致
#       ⇒ 这条同时是**登记过期判据**：ORT 哪天修好了会判红，逼我们更新文档，
#         而不是让「已知偏离」永远挂在那里（M293 的 MIGRATE_KNOWN 口径）
#   [4] 负控：篡改真值 ⇒ [2] 必须判红；判据自伤 ⇒ 那口红必须消失
#
# 前提：装置是**入仓生成物**（devices/）；生成需要 python3 + onnx（CI 上有，m306 同款）。
#       onnxruntime **不是**前提 —— 缺了就 SKIP 该层并打印原因（不装作绿）。
# ══════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
    echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
HERE="$(pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"          # 按脚本位置推导（不写死 —— M213 缺陷 300）
PX="$ROOT/tools/px"
ORT_HOME="${M315_ORT_HOME:-/opt/fw}"
W="$(mktemp -d /tmp/m315.XXXXXX)"
trap 'rm -rf "$W"' EXIT

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAIL=0
ok(){ echo "  ✅ $1"; PASS=$((PASS+1)); }
bad(){ echo "  ❌ $1"; FAIL=$((FAIL+1)); }
info(){ echo "  ℹ️  $1"; }
hdr(){ echo ""; echo "── $* ──"; }

echo "══ M315 · MatMulInteger(u8s8) 真值方向（规范 ⇄ PuXian ⇄ onnxruntime）══"

# ── [0] 前提 ──────────────────────────────────────────────────────────
hdr "[0] 前提"
[ -x "$PX" ] && ok "tools/px 可执行" || { bad "tools/px 缺失：$PX"; echo "M315-VERIFY-FAIL"; exit 1; }
have_ort=0
PYTHONPATH="$ORT_HOME" python3 -c 'import onnxruntime' >/dev/null 2>&1 && have_ort=1
[ "$have_ort" = 1 ] && ok "onnxruntime 可用（$ORT_HOME）· [3] 层会执行" \
    || info "onnxruntime 不可用（CI 常态）⇒ [3] 层 SKIP（数值硬判据 [2] 不受影响）"

# ── [1] 装置可复现 + 静态 ─────────────────────────────────────────────
hdr "[1] 装置可复现（重跑生成器 ⇄ 入仓装置逐字节比对）"
NDEV=$(ls devices/*.onnx 2>/dev/null | wc -l)
[ "$NDEV" -ge 6 ] && ok "装置数 $NDEV ≥ 6（规模锚点）" || bad "装置数 $NDEV < 6 —— 覆盖面被削"
[ -f devices/index.json ] && ok "index.json 在位（真值来源与触发条件声明）" || bad "缺 devices/index.json"

if PYTHONPATH="$ORT_HOME" python3 gen_devices.py "$W/regen" >"$W/gen.log" 2>&1; then
    ok "生成器执行成功"
    drift=0
    for f in devices/*; do
        b="$(basename "$f")"
        if ! cmp -s "$f" "$W/regen/$b"; then
            bad "装置漂移：$b 与重新生成的不一致（手改过装置，或生成器改了没重生成）"
            drift=$((drift+1))
        fi
    done
    [ "$drift" = 0 ] && ok "全部 $NDEV 个装置（含 .onnx/.in/.ref/index.json）与生成器输出逐字节一致"
else
    bad "生成器执行失败（见下）"; tail -6 "$W/gen.log" | sed 's/^/     /'
fi

# 真值来源必须**不是** onnxruntime —— 这是本门存在的理由，写成判据而不是注释
if grep -q '逐元素整数累加' devices/index.json 2>/dev/null; then
    ok "index.json 声明真值来源 = 规范定义（不是任何执行器的输出）"
else
    bad "index.json 没有声明真值来源（无法证明「真值不是 ORT 给的」）"
fi
n_hi=$(grep -c '"A_has_gt127": true' devices/index.json 2>/dev/null || echo 0)
n_lo=$(grep -c '"A_has_gt127": false' devices/index.json 2>/dev/null || echo 0)
[ "$n_hi" -ge 4 ] && [ "$n_lo" -ge 2 ] \
    && ok "两类装置都在：A 含 >127 有 $n_hi 个 · A 全 ≤127 有 $n_lo 个（对照不能缺）" \
    || bad "装置类别不全（>127: $n_hi · ≤127: $n_lo）—— 对照组缺失就无法证明触发条件"

# ── [2] PuXian ⇄ 规范真值（本门的主判据）─────────────────────────────
hdr "[2] PuXian ⇄ 规范真值（每装置逐位一致）"
cp run_px.px "$W/probe.px"
if ! "$PX" build "$W/probe.px" >"$W/build.log" 2>&1; then
    bad "探针编译失败"; tail -8 "$W/build.log" | sed 's/^/     /'
    echo "M315-VERIFY-FAIL"; exit 1
fi
BIN="$W/build/probe"
[ -x "$BIN" ] || { bad "探针产物不存在：$BIN"; echo "M315-VERIFY-FAIL"; exit 1; }

mkdir -p "$W/px"
python3 - "$HERE/devices" "$W/px" <<'PY' > "$W/cases.tsv"
import json, sys
idx = json.load(open(sys.argv[1] + "/index.json", encoding="utf-8"))
for d in idx["devices"]:
    print("\t".join([d["name"], str(d["M"]), str(d["K"]), str(d["N"]), d["A_hex"], d["B_hex"]]))
PY
NCASE=$(grep -c . "$W/cases.tsv" || true)
[ "$NCASE" = "$NDEV" ] && ok "用例表 $NCASE 条 = 装置数" || bad "用例表 $NCASE 条 ≠ 装置数 $NDEV"

runall() {   # runall <输出目录>
    local outd="$1" name M K N ah bh
    while IFS=$'\t' read -r name M K N ah bh; do
        M315_MODEL="$HERE/devices/$name.onnx" M315_A_HEX="$ah" M315_B_HEX="$bh" \
        M315_M="$M" M315_K="$K" M315_N="$N" \
            "$BIN" > "$outd/$name.pxout" 2>&1 || echo "RUN-RC=$?" >> "$outd/$name.pxout"
    done < "$W/cases.tsv"
}
runall "$W/px"
if PYTHONPATH="$ORT_HOME" python3 check_truth.py px "$HERE/devices" "$W/px" > "$W/pxcmp.log" 2>&1; then
    sed 's/^/  /' "$W/pxcmp.log"; PASS=$((PASS+1))
else
    sed 's/^/  /' "$W/pxcmp.log"; bad "PuXian 与规范真值不一致（见上）"
fi

# 确定性：同输入再跑一遍必须逐字节一致（真值对拍的隐含前提）
mkdir -p "$W/px2"; runall "$W/px2"
if diff -rq "$W/px" "$W/px2" >/dev/null 2>&1; then
    ok "确定性：同输入两次运行逐字节一致"
else
    bad "确定性：两次运行输出不同（同二进制同输入 ⇒ 不可接受）"
fi

# ── [3] onnxruntime 登记核对（双向）──────────────────────────────────
hdr "[3] onnxruntime ⇄ 规范（双向登记核对）"
if [ "$have_ort" = 1 ]; then
    PYTHONPATH="$ORT_HOME" python3 check_truth.py ort "$HERE/devices" > "$W/ort.log" 2>&1
    rc=$?
    sed 's/^/  /' "$W/ort.log"
    if [ "$rc" = 0 ]; then
        ok "ORT 登记核对全部符合（A>127 偏离 / 对照一致）"
    else
        bad "ORT 登记核对不符（见上；若 ORT 已修好 ⇒ 更新 docs/ONNX_ORT_KERNEL.md 与期望）"
    fi
else
    info "SKIP：[3] 层需要 onnxruntime（本档没有）—— **不代表** ORT 行为已核对"
fi

# ── [4] 负控（各自独立判红）──────────────────────────────────────────
if [ "$NEG_SKIP" = 1 ]; then
    hdr "[4] 负控（--neg-skip：本轮跳过；本地全量门会跑）"
    info "负控 A（篡改真值 ⇒ [2] 必红）与 B（判据自伤 ⇒ 红消失）已跳过"
else
    hdr "[4] 负控 A：篡改一个真值 ⇒ [2] 必须判红"
    cp -r "$HERE/devices" "$W/dev_tampered"
    python3 - "$W/dev_tampered" <<'PY'
import sys, pathlib
p = pathlib.Path(sys.argv[1]) / "mmi_k8_hi.ref.txt"
lines = p.read_text(encoding="utf-8").splitlines()
for i, l in enumerate(lines):
    if l.startswith("HEX "):
        hx = bytearray.fromhex(l[4:].strip())
        hx[0] ^= 0xFF          # 翻转第一个元素的最低位字节
        lines[i] = "HEX " + hx.hex()
p.write_text("\n".join(lines) + "\n", encoding="utf-8")
PY
    if PYTHONPATH="$ORT_HOME" python3 check_truth.py px "$W/dev_tampered" "$W/px" > "$W/neg_a.log" 2>&1; then
        bad "篡改真值后判据**仍然判绿** ⇒ [2] 层没有牙"
    else
        grep -q 'mmi_k8_hi' "$W/neg_a.log" && ok "篡改真值 ⇒ [2] 判红且**指名** mmi_k8_hi（判据有牙）" \
            || bad "判红了但没指名被篡改的装置（见 $W/neg_a.log）"
    fi

    hdr "[4b] 负控 B：判据自伤 ⇒ 负控 A 的那口红必须消失"
    sed 's/if np.array_equal(got, ref):/if True:/' check_truth.py > "$W/check_sham.py"
    if grep -q 'if True:' "$W/check_sham.py"; then
        if PYTHONPATH="$ORT_HOME" python3 "$W/check_sham.py" px "$W/dev_tampered" "$W/px" > "$W/neg_b.log" 2>&1; then
            ok "判据自伤（array_equal 恒真）⇒ 篡改后不再判红（红来自该判据，不是别的东西）"
        else
            bad "判据自伤后**仍然判红** ⇒ 负控 A 的红另有来源（不是 [2] 层的比对）"
        fi
    else
        bad "负控 B 构造失败（sed 没有命中 array_equal 那一行）"
    fi
fi

# ── [5] 覆盖边界（如实登记）──────────────────────────────────────────
hdr "[5] 覆盖边界"
cat <<'EOF'
  · 只覆盖 MatMulInteger 的 **u8s8** 一种类型对（生产 bge 用的就是它）。
    u8u8 / s8s8 / s8u8 三种类型对**不在面内**（同族风险未证，需另行度量）。
  · 只覆盖 2-D × 2-D 的最小装置；批量维 / 广播 / 高维不在面内。
  · 触发条件（A 含 >127）来自 M315 的 **1536 组合扫描**（K 1..16 × N 1..8 ×
    A>127 是否 × a_zp∈{0,114,128} × b_zp∈{0,-7}）：
      A 全 ≤127 ⇒ ORT 0/768 错 · A 含 >127 ⇒ ORT 568/768 错（K=1 除外）
    ⚠️ 该扫描是**结论的证据**，不是本门的判据（跑 1536 个 session 不适合进门）；
      复现脚本见 docs/ONNX_ORT_KERNEL.md。
  · 端到端（102MB 生产模型）不在本门内 —— 前提是外部模型文件，属 m311 门的面。
  · onnxruntime 版本钉 1.19.2（/opt/fw）。其它版本可能已修或另错 ⇒ [3] 层会判红提示复核。
EOF

echo ""
echo "══ 汇总：$PASS 通过 / $FAIL 失败 ══"
if [ "$FAIL" = 0 ]; then echo "M315-VERIFY-OK"; exit 0; else echo "M315-VERIFY-FAIL"; exit 1; fi
