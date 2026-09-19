#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M156 门 —— 零依赖 ONNX 解析面（第 38 轮）
# ---------------------------------------------------------------
# 判据分五层，**每层都能单独变红**（门红不了就不是门）：
#   ① 构造性真值：models/expected.json 由**生成参数**直接给出（我知道写进去了什么）
#   ② 独立实现：tools/onnx_ref.py（另一份手写 protobuf 解码器）读同一份字节
#   ③ C 侧语言接口：onnx_dump.px → onnx_model_open/onnx_info（走真 native）
#      —— ①↔②、③↔① 都一致才算过。**只做 ②↔③ 会漏「两侧一致地错」**
#      （本轮真踩过：生成器漏包 GraphProto.node 的 field 1，两侧一起解析错，
#        "事实集完全一致"照样成立）
#   ④ 负控 4 个模型（无 graph / 截断 / field 0 / 9 维超界）：必须报错
#   ⑤ 权重字节：initializer head16（含窄类型低字节截断）逐字节对
#   另：三轨一致（VM 默认轨 vs 解释轨）；真实模型可选（**SKIP 不是 PASS**）
# ══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
HERE="$(pwd)"
ROOT=/data/code/puxian
PX="$ROOT/tools/px"
TMP="$(mktemp -d /tmp/m156.XXXXXX)"
trap 'rm -rf "$TMP"' EXIT

FAIL=0
ok()  { printf '  ✅ %s\n' "$1"; }
bad() { printf '  ❌ %s\n' "$1"; FAIL=$((FAIL + 1)); }

echo "── M156 门 · 零依赖 ONNX 解析面 ──"

# ── 0. 重新生成模型（幂等）+ 构造性真值 ──
( cd "$ROOT" && python3 examples/m156_onnx/tools/gen_models.py > "$TMP/gen.log" 2>&1 ) \
    || { echo "生成器失败"; cat "$TMP/gen.log"; exit 1; }
NMOD=$(ls models/*.onnx 2>/dev/null | wc -l)
NEXP=$(python3 -c "import json;print(len(json.load(open('models/expected.json'))))")
[ "$NMOD" -eq 16 ] && [ "$NEXP" -eq 13 ] \
    && ok "构造性模型 16 个（13 个正例带真值 / 4 个负控，t16 亦给真值但不比对）" \
    || bad "模型数异常：$NMOD 个模型 / $NEXP 个真值（期望 16 / 13）"

# ── 1. 编译语言侧 dump（**默认自动裁剪**：会顺带验证自动裁剪认得 onnx 模块） ──
if "$PX" build onnx_dump.px > "$TMP/build.log" 2>&1; then
    ok "onnx_dump 编译成功（自动裁剪轨，$(stat -c %s build/onnx_dump) 字节）"
else
    bad "onnx_dump 编译失败"; tail -5 "$TMP/build.log"
    echo "M156-VERIFY-FAIL"; exit 1
fi
DUMP="$HERE/build/onnx_dump"

# ── 2. 正例：三重对拍（构造真值 · C 侧 · 独立参考） ──
POS="t01_basic t02_attrs t03_unpacked t04_narrow t05_notype t06_unknown t07_symbolic t08_negative t09_emptygraph t13_bigdim t14_dupname t15_rank8"
N_OK=0
for m in $POS; do
    if ! "$DUMP" "models/$m.onnx" "$TMP/$m.json" > "$TMP/$m.stdout" 2>&1; then
        bad "$m：语言侧运行失败（$(tr -d '\n' < "$TMP/$m.stdout" | head -c 80)）"; continue
    fi
    if python3 tools/onnx_ref.py --triple "models/$m.onnx" "$TMP/$m.json" models/expected.json \
         > "$TMP/$m.tri" 2>&1; then
        N_OK=$((N_OK + 1))
    else
        bad "$m：三重对拍不一致"
        grep -E '❌|不一致|C      =|参考   =' "$TMP/$m.tri" | head -6
    fi
done
[ "$N_OK" -eq 12 ] && ok "正例 12/12 三重一致（构造真值 ↔ 独立参考 ↔ C 侧 native）" \
                   || bad "正例仅 $N_OK/12 三重一致"

# ── 3. 负控：必须报错 ──
NEG="t10_nograph t11_truncated t12_field0 t16_rank9"
N_NEG=0
for m in $NEG; do
    if "$DUMP" "models/$m.onnx" "$TMP/$m.json" > "$TMP/$m.stdout" 2>&1; then
        bad "$m：**未报错**（负控失效：非法输入被当合法模型收了）"
    elif grep -q 'ONNX-ERR' "$TMP/$m.stdout"; then
        N_NEG=$((N_NEG + 1))
    else
        bad "$m：退出非 0 但未打印 ONNX-ERR"; tail -2 "$TMP/$m.stdout"
    fi
done
[ "$N_NEG" -eq 4 ] && ok "负控 4/4 全部被 C 侧判错（非 0 退出 + ONNX-ERR）" \
                   || bad "负控仅 $N_NEG/4 判错"

# ── 4. 权重数据 head16（构造已知字节） ──
#   t01 的 W = float32 [1..6] ⇒ 前 4 个：0000803f 00000040 00004040 00008040
"$DUMP" models/t01_basic.onnx "$TMP/t01b.json" W > "$TMP/w.stdout" 2>&1
GOT=$(grep -o 'head16=[0-9a-f]*' "$TMP/w.stdout" | head -1 | cut -d= -f2)
[ "$GOT" = "0000803f000000400000404000008040" ] \
    && ok "权重 head16 逐字节一致（float32 raw_data）" \
    || bad "权重 head16 不符：C=$GOT"
#   t04 的 f16 = fp16 位模式 [0x3C00, 0xC000] ⇒ int32_data 承载、低 2 字节
"$DUMP" models/t04_narrow.onnx "$TMP/t04b.json" f16 > "$TMP/f16.stdout" 2>&1
GOT2=$(grep -o 'head16=[0-9a-f]*' "$TMP/f16.stdout" | head -1 | cut -d= -f2)
[ "$GOT2" = "003c00c0" ] \
    && ok "窄类型（fp16）逐元素低 2 字节截断正确（$GOT2）" \
    || bad "fp16 截断不符：C=$GOT2 期望=003c00c0"

# ── 5. 三轨一致（VM 默认轨 vs 解释轨） ──
"$DUMP" models/t02_attrs.onnx "$TMP/tr_vm.json" > "$TMP/tr_vm.stdout" 2>&1
"$PX" run onnx_dump.px models/t02_attrs.onnx "$TMP/tr_it.json" > "$TMP/tr_it.stdout" 2>&1
if [ -s "$TMP/tr_vm.json" ] && [ -s "$TMP/tr_it.json" ] && cmp -s "$TMP/tr_vm.json" "$TMP/tr_it.json"; then
    ok "三轨一致：VM 轨与解释轨的结构 JSON 逐字节相同"
else
    bad "VM 轨与解释轨不一致（或解释轨未产出）"
    head -3 "$TMP/tr_it.stdout"
fi

# ── 6. 真实模型（可选；**SKIP 不是 PASS**） ──
REAL="${M156_REAL_MODEL:-/tmp/onnxdl/minilm.onnx}"
if [ -f "$REAL" ]; then
    if "$DUMP" "$REAL" "$TMP/real.json" embeddings.word_embeddings.weight > "$TMP/real.stdout" 2>&1 \
       && python3 tools/onnx_ref.py "$REAL" "$TMP/real.json" > "$TMP/real.ref" 2>&1; then
        ok "真实模型三重对拍通过（$(basename "$REAL") · $(tr -d '\n' < "$TMP/real.stdout" | head -c 60)）"
        RH=$(python3 tools/onnx_ref.py --init "$REAL" embeddings.word_embeddings.weight 2>/dev/null | grep '^head16=' | cut -d= -f2)
        GW=$(grep -o 'head16=[0-9a-f]*' "$TMP/real.stdout" | head -1 | cut -d= -f2)
        [ -n "$RH" ] && [ "$GW" = "$RH" ] && ok "真实模型权重 head16 与参考一致（$RH，46.8MB 权重）" \
                                        || bad "真实模型权重 head16 不符：C=$GW 参考=$RH"
    else
        bad "真实模型对拍失败"; tail -5 "$TMP/real.ref" 2>/dev/null
    fi
else
    echo "  ⚠️  SKIP 真实模型（$REAL 不存在；M156_REAL_MODEL 可指定）—— SKIP **不计入 PASS**"
fi

echo
# ── 7. 负控 · 门自检（判据若不能变红就不是判据 —— 与 M154「阈值太松 ⇒ 负控漏网」同族） ──
if [ -f "$TMP/t02_attrs.json" ]; then
    python3 - "$TMP/expected_bad.json" "$TMP/c_bad.json" <<'PY'
import json, sys
e = json.load(open("models/expected.json"))
e["t02_attrs.onnx"]["graph"] = "TAMPERED"          # 篡改构造真值
json.dump(e, open(sys.argv[1], "w"))
c = json.load(open(sys.argv[2].replace("c_bad.json", "t02_attrs.json")))
c["graph"] = "TAMPERED"                             # 篡改 C 侧输出
json.dump(c, open(sys.argv[2], "w"))
PY
    if python3 tools/onnx_ref.py --triple models/t02_attrs.onnx "$TMP/t02_attrs.json" "$TMP/expected_bad.json" >/dev/null 2>&1; then
        bad "负控 E：篡改构造真值后三重对拍仍判通过（判据失效）"
    else
        ok "负控 E：篡改构造真值 ⇒ 三重对拍判红（构造性真值确实在起作用）"
    fi
    if python3 tools/onnx_ref.py --triple models/t02_attrs.onnx "$TMP/c_bad.json" models/expected.json >/dev/null 2>&1; then
        bad "负控 F：篡改 C 侧 JSON 后三重对拍仍判通过（判据失效）"
    else
        ok "负控 F：篡改 C 侧 JSON ⇒ 三重对拍判红（C 侧输出确实被逐条核对）"
    fi
else
    bad "负控 E/F：缺少 t02 的 C 侧输出（前置步骤未运行？）"
fi

echo
if [ "$FAIL" -eq 0 ]; then echo "M156-VERIFY-OK"; exit 0; else echo "M156-VERIFY-FAIL（$FAIL 项）"; exit 1; fi
