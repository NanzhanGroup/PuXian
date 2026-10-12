#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M306 门 —— ONNX **量化算子面**（S2：DequantizeLinear / QuantizeLinear）
# ---------------------------------------------------------------
# 立论：`known-issues` 里挂着一条「int8 量化模型跑不了（缺 DequantizeLinear）」，
#   而**量化算子的静默出错代价最高** —— 权重解错 ⇒ 模型"能跑"但数值全错；
#   量化公式写错 ⇒ 权重文件被静默写坏。⇒ 本门把它做成**清单级判据**：
#   逐张量/逐轴、就近偶数、双端饱和、缺省零点、fp16/bf16 刻度，**每一条都有手算真值**。
#
# 判据分八层，**每层都能单独变红**（门红不了就不是门）：
#   ① 语料：gen_quant_models.py 幂等产出 18 个正例 + 9 个行为负控 + spec/expected/feeds
#   ② 编译：语言侧驱动 quant_exec.px
#   ③ **三方对拍**：手算真值 ↔ 独立参考（quant_ref.py，另一套纯 Python 含自写的半精度位运算）
#                    ↔ C 侧（读同一批 .onnx）；两个方向各自独立报数
#   ④ 行为负控：9 个非法形态**每个都必须响亮报错**（且消息里指名算子与条件）
#   ⑤ 算子覆盖：两个算子必须**真的被执行过**（by_op 并集）
#   ⑥ 静态：源码在位（去注释后）—— 注册 / 守卫 / **单一实现**（舍入与形态解析不许有两份）
#   ⑦ 源码负控 4 道（必须判红 ⇒ 逐字节还原 ⇒ 复跑绿）
#   ⑧ 覆盖边界（如实登记未覆盖面）
# ══════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
    echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
HERE="$(pwd)"
# 仓库根：**按脚本位置推导**（不写死 /data/code/puxian —— 写死会本地绿、CI 红：M213 缺陷 300）
ROOT="$(cd "$HERE/../.." && pwd)"
PX="$ROOT/tools/px"
TMP="$(mktemp -d /tmp/m306.XXXXXX)"
trap 'rm -rf "$TMP"' EXIT

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

FAIL=0
ok()   { printf '  ✅ %s\n' "$1"; }
bad()  { printf '  ❌ %s\n' "$1"; FAIL=$((FAIL + 1)); }

echo "── M306 门 · ONNX 量化算子面（DequantizeLinear / QuantizeLinear / DynamicQuantizeLinear / MatMulInteger）──"

# ── ① 语料 ──
python3 tools/gen_quant_models.py > "$TMP/gen.log" 2>&1 \
    || { echo "生成器失败"; cat "$TMP/gen.log"; echo "M306-VERIFY-FAIL"; exit 1; }
NMOD=$(ls models/*.onnx 2>/dev/null | wc -l)
NNEG=$(ls models/neg/*.onnx 2>/dev/null | wc -l)
NGEN=$(grep -o '模型 [0-9]* 个' "$TMP/gen.log" | head -1 | grep -o '[0-9]*')
NCASE=$(python3 -c "import json;print(len(json.load(open('models/neg/cases.json'))))")
# 不写死个数：与生成器自报的数量比对（避免"加了语料忘了改门"）
[ "$NNEG" -eq "$NCASE" ] && ok "负控用例表 $NCASE 条与磁盘 $NNEG 个模型一致" \
                         || bad "负控表 $NCASE ≠ 磁盘 $NNEG"
[ -n "$NGEN" ] && [ "$NMOD" -eq "$NGEN" ] && ok "正例模型 $NMOD 个（与生成器自报一致）" \
                                          || bad "正例模型数 $NMOD ≠ 生成器自报 ${NGEN:-?}"
# 真值必须覆盖每个模型的每个声明输出（防"加了模型忘了给真值"）
if python3 - "$TMP" <<'PY'
import json, sys
spec = json.load(open("models/spec.json")); exp = json.load(open("models/expected.json"))
assert len(spec["models"]) > 0
for m in spec["models"]:
    assert m["name"] in exp, "缺真值：%s" % m["name"]
    for o in m["outputs"]:
        assert o["name"] in exp[m["name"]], "缺输出真值：%s.%s" % (m["name"], o["name"])
print("真值覆盖 %d 个模型" % len(spec["models"]))
PY
then ok "手算真值覆盖**每个模型的每个输出**"; else bad "真值覆盖不全"; fi

# ── ② 编译 ──
if "$PX" build quant_exec.px > "$TMP/build.log" 2>&1; then
    ok "quant_exec 编译成功（$(stat -c %s build/quant_exec) 字节）"
else
    bad "quant_exec 编译失败"; tail -5 "$TMP/build.log"; echo "M306-VERIFY-FAIL"; exit 1
fi

gen_c() {   # 跑所有正例 → $TMP/c（可反复调用：负控后要重跑）
    rm -rf "$TMP/c"; mkdir -p "$TMP/c"
    for f in models/*.onnx; do
        n=$(basename "$f" .onnx)
        ./build/quant_exec "$f" "models/feeds/$n.json" "$TMP/c/$n.json" \
            > "$TMP/c/$n.log" 2>&1
    done
}

# ── ③ 三方对拍 ──
gen_c
if python3 tools/quant_ref.py "$TMP/c" models/spec.json models/expected.json models/feeds \
       > "$TMP/ref.log" 2>&1; then
    ok "三方对拍全绿：$(grep -c '✅' "$TMP/ref.log") 个模型 · $(grep -o 'M306-REF:.*' "$TMP/ref.log")"
else
    bad "三方对拍失败"; grep -E '❌|Traceback|Error' "$TMP/ref.log" | head -8
fi

# ── ④ 行为负控：9 个非法形态必须逐个响亮报错 ──
python3 - "$TMP" <<'PY' && ok "行为负控 $NCASE 条：全部响亮报错（指名算子 + 给出实质原因）" \
                         || FAIL=$((FAIL + 1))
import json, os, re, subprocess, sys
tmp = sys.argv[1]
cases = json.load(open("models/neg/cases.json"))
feed = "models/feeds/q01_dq_u8_pertensor.json"      # 空 feeds {}
OPS4 = ("DequantizeLinear", "QuantizeLinear", "DynamicQuantizeLinear", "MatMulInteger")
bad = 0
for c in cases:
    n = c["name"]
    r = subprocess.run(["./build/quant_exec", "models/neg/%s.onnx" % n, feed,
                        os.path.join(tmp, n + ".json")], capture_output=True, text=True)
    msg = (r.stdout + r.stderr).strip().split("\n")[0]
    if r.returncode == 0:
        print("  ❌ %s：期望报错但跑成功了（%s）" % (n, c["why"])); bad += 1; continue
    # 判据：必须是**算子层**的响亮错误，且「指名算子」+「给出实质性原因」两件都要有
    m = re.search(r"（op=([A-Za-z]+)）执行失败：(.+?)\)+\s*$", msg)
    if not m:
        print("  ❌ %s：不是算子层的响亮错误 → %s" % (n, msg[:100])); bad += 1; continue
    opname, inner = m.group(1), m.group(2)
    if opname not in OPS4:
        print("  ❌ %s：报的算子 %r 不在本门四算子内（消息被截断？）" % (n, opname)); bad += 1; continue
    if len(inner.strip()) < 8:
        print("  ❌ %s：只有外壳、没有实质原因（inner=%r）" % (n, inner)); bad += 1; continue
    print("  ✅ %s：%s → %s" % (n, opname, inner[:80]))
if bad:
    sys.exit(1)
PY

# ── ⑤ 算子覆盖：两个算子必须真的被执行过 ──
python3 - "$TMP" <<'PY' || FAIL=$((FAIL + 1))
import json, os, sys
tmp = sys.argv[1]
used = set()
for f in os.listdir(os.path.join(tmp, "c")):
    if not f.endswith(".json"):
        continue
    try:
        d = json.load(open(os.path.join(tmp, "c", f)))
    except Exception:
        continue
    used |= set(d.get("by_op", {}).keys())
want = {"DequantizeLinear", "QuantizeLinear",
        "DynamicQuantizeLinear", "MatMulInteger"}
miss = want - used
if miss:
    print("  ❌ 算子覆盖：语料从没执行过 %s（by_op 并集 %d 个）" % (sorted(miss), len(used)))
    sys.exit(1)
print("  ✅ 算子覆盖：本门 %d 个量化算子**全部**在语料里真的执行过（by_op 并集 %d 个算子）"
      % (len(want), len(used)))
PY

# ── ⑥ 静态：源码在位（**去注释后**判，防"注释里写着修了"）──
python3 - "$ROOT" <<'PY' || FAIL=$((FAIL + 1))
import re, sys
root = sys.argv[1]

def strip(p):
    s = open(p, encoding="utf-8").read()
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
    s = re.sub(r"//[^\n]*", "", s)
    return s

ops = strip(root + "/runtime/onnx_ops.c")
ten = strip(root + "/runtime/onnx_tensor.c")
bad = 0

def chk(cond, msg):
    global bad
    if cond:
        print("  ✅ " + msg)
    else:
        print("  ❌ " + msg); bad += 1

def cnt(s, sub):
    return s.count(sub)

chk(cnt(ops, '"DequantizeLinear", onnx_op_dequantizelinear') == 1,
    "算子表注册 DequantizeLinear（1 处）")
chk(cnt(ops, '"QuantizeLinear", onnx_op_quantizelinear') == 1,
    "算子表注册 QuantizeLinear（1 处）")
chk(cnt(ops, '"DynamicQuantizeLinear", onnx_op_dynamicquantizelinear') == 1,
    "算子表注册 DynamicQuantizeLinear（1 处）")
chk(cnt(ops, '"MatMulInteger", onnx_op_matmulinteger') == 1,
    "算子表注册 MatMulInteger（1 处）")
chk(cnt(ops, "scale = (double)(float)scale;") == 1,
    "DynamicQuantizeLinear：导出的 float32 刻度就是**后续算式用的那个值**")
chk(cnt(ops, 'q_resolve("DequantizeLinear"') == 1 and cnt(ops, 'q_resolve("QuantizeLinear"') == 1,
    "**形态解析是共用实现**（两个算子各调 q_resolve 一次 ⇒ 口径不会漂）")
chk(cnt(ops, "static double round_half_even(") == 1, "就近偶数只有一份定义（round_half_even）")
chk(cnt(ops, "return round_half_even(q);") == 1, "量化路径复用该实现（q_round）")
chk(cnt(ops, 'saturate", 1) == 0') == 1, "saturate=0 响亮拒绝在位")
chk(cnt(ops, 'block_size", 0) != 0') == 2, "block_size≠0 两个算子各拒一次")
chk(cnt(ops, "static void q_range(") == 1, "饱和范围表只有一份（q_range）")
chk(cnt(ten, "px_f16_to_f(h)") == 1 and cnt(ten, "px_bf16_to_f(h)") == 1,
    "onnx_t_getf 的 fp16/bf16 读分支在位")
chk(cnt(ten, "px_f_to_f16((float)v)") == 1 and cnt(ten, "px_f_to_bf16((float)v)") == 1,
    "onnx_t_setf 的 fp16/bf16 写分支在位")
chk(cnt(ten, "to == ONNX_DT_FLOAT16 || to == ONNX_DT_BFLOAT16") == 1,
    "Cast 把 fp16/bf16 认成**浮点目标**（缺陷 527 同族：不许走 seti 截断）")
sys.exit(1 if bad else 0)
PY

# ── ⑦ 源码负控 ──
SNAP_BEFORE=$(sha256sum "$ROOT/runtime/onnx_ops.c" "$ROOT/runtime/onnx_tensor.c" \
                        "$ROOT/runtime/onnx_exec.c" "$HERE/tools/quant_ref.py" | sha256sum)

patch1() {   # $1=文件 $2=旧 $3=新（旧串必须恰好 1 次）
    python3 - "$1" "$2" "$3" <<'PYEOF'
import sys
f, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(f, encoding="utf-8").read()
n = s.count(a)
if n != 1:
    print("锚点出现 %d 次（要求 1 次）" % n); sys.exit(1)
open(f, "w", encoding="utf-8").write(s.replace(a, b))
PYEOF
}

OPS="$ROOT/runtime/onnx_ops.c"
REF="$HERE/tools/quant_ref.py"
NC_A_FROM='        if (!(r >= lo)) r = lo;                           /* NaN / -Inf 也落这一支 */
        else if (r > hi) r = hi;'
NC_A_TO='        (void)lo; (void)hi;'

restore() {
    cp "$TMP/nc_ops.bak" "$OPS"
    cp "$TMP/nc_ref.bak" "$REF"
}

if [ "$NEG_SKIP" = "0" ]; then
    cp "$OPS" "$TMP/nc_ops.bak"; cp "$REF" "$TMP/nc_ref.bak"

    # NC-A：去掉饱和 ⇒ q06/q10 的边界必须与真值不符
    if ! patch1 "$OPS" "$NC_A_FROM" "$NC_A_TO"; then
        bad "NC-A：锚点不唯一（改不动 ⇒ 无法证明门能红）"
    elif ! "$PX" build quant_exec.px > "$TMP/ncb_a.log" 2>&1; then
        bad "NC-A：篡改后**编译失败**（拿不到判据 ≠ 判据满足）"; tail -3 "$TMP/ncb_a.log"
    else
        gen_c
        if python3 tools/quant_ref.py "$TMP/c" models/spec.json models/expected.json \
             models/feeds > "$TMP/nc_a.log" 2>&1; then
            bad "NC-A：去掉饱和后三方对拍仍全绿 ⇒ 门没有真的守住饱和这一条"
        else
            ok "NC-A：去掉饱和后如期判红（$(grep -m1 '❌' "$TMP/nc_a.log" | sed 's/^ *//' | head -c 80)）"
        fi
    fi
    restore

    # NC-B：舍入改成截断 ⇒ 就近偶数这一条必须判红
    if ! patch1 "$OPS" "    return round_half_even(q);" "    return trunc(q);"; then
        bad "NC-B：锚点不唯一"
    elif ! "$PX" build quant_exec.px > "$TMP/ncb_b.log" 2>&1; then
        bad "NC-B：篡改后编译失败"
    else
        gen_c
        if python3 tools/quant_ref.py "$TMP/c" models/spec.json models/expected.json \
             models/feeds > "$TMP/nc_b.log" 2>&1; then
            bad "NC-B：改成截断后仍全绿 ⇒ 就近偶数这一条没有被守住"
        else
            ok "NC-B：舍入改截断后如期判红（$(grep -m1 '❌' "$TMP/nc_b.log" | sed 's/^ *//' | head -c 80)）"
        fi
    fi
    restore

    # NC-C：关掉零点的类型检查 ⇒ 负控层必须由"报错"变成"跑通"
    if ! patch1 "$OPS" "    if (zp && zp->dtype != x->dtype)" \
                       "    if (zp && zp->dtype != x->dtype && 0)"; then
        bad "NC-C：锚点不唯一"
    elif ! "$PX" build quant_exec.px > "$TMP/ncb_c.log" 2>&1; then
        bad "NC-C：篡改后编译失败"
    else
        out=$(./build/quant_exec models/neg/n04_zp_dtype.onnx \
                models/feeds/q01_dq_u8_pertensor.json "$TMP/n04.json" 2>&1)
        if [ $? -eq 0 ]; then
            ok "NC-C：关掉零点类型检查后 n04 如期『跑通』⇒ 负控层有牙（$(echo "$out" | head -1 | head -c 60)）"
        else
            bad "NC-C：关掉检查后 n04 仍报错 ⇒ 那条红色不是来自这个检查"
        fi
    fi
    restore
    "$PX" build quant_exec.px > /dev/null 2>&1

    # NC-D：**判据自伤** —— 源码修复全部退回 + 判据失去比较能力 ⇒ NC-A 的红必须消失
    if ! patch1 "$REF" "    return int(a) == int(b)" "    return True"; then
        bad "NC-D：锚点不唯一"
    else
        patch1 "$OPS" "$NC_A_FROM" "$NC_A_TO" || bad "NC-D：NC-A 锚点不唯一"
        "$PX" build quant_exec.px > /dev/null 2>&1
        gen_c
        if python3 tools/quant_ref.py "$TMP/c" models/spec.json models/expected.json \
             models/feeds > /dev/null 2>&1; then
            ok "NC-D：判据自伤后 NC-A 的红**如期消失** ⇒ NC-A 的红确实来自比较判据本身"
        else
            bad "NC-D：判据自伤后仍判红 ⇒ 那道红另有来源（判据没被真正关掉）"
        fi
        restore
        "$PX" build quant_exec.px > /dev/null 2>&1
    fi

    SNAP_AFTER=$(sha256sum "$ROOT/runtime/onnx_ops.c" "$ROOT/runtime/onnx_tensor.c" \
                           "$ROOT/runtime/onnx_exec.c" "$HERE/tools/quant_ref.py" | sha256sum)
    [ "$SNAP_BEFORE" = "$SNAP_AFTER" ] \
        && ok "负控残留检查：三个源文件 sha256 逐字节还原" \
        || bad "负控源文件**未还原**"

    gen_c
    if python3 tools/quant_ref.py "$TMP/c" models/spec.json models/expected.json models/feeds \
         > "$TMP/ref2.log" 2>&1; then
        ok "还原后复跑三方对拍全绿（$(grep -o 'M306-REF:.*' "$TMP/ref2.log")）"
    else
        bad "还原后复跑不绿"; grep -E '❌' "$TMP/ref2.log" | head -5
    fi
else
    echo "  ⏭️  源码负控 4 道（--neg-skip：由本地全量门/CI 的另一档承担）"
fi

# ── ⑧ 覆盖边界（如实登记 —— 别让"没覆盖"看起来像"覆盖了"）──
cat <<'EOF'
  ℹ️  覆盖边界（如实）：
     · 已覆盖：逐张量 / 逐轴（含负 axis）× 就近偶数 × 双端饱和 × 缺省零点 ×
       int8/uint8/int32 输入 × int8/uint8/int32 输出 × fp16/bf16 刻度 × 往返恒等 ×
       「int8 权重解量化 → float MatMul → 回量化」· 「动态量化激活 → 整数矩阵乘」两条真实形态
     · **不支持且响亮拒绝**（未覆盖其数值语义）：块量化（block_size≠0，opset 21+）、
       非饱和量化（saturate=0）、4-bit（int4/uint4）；QLinearConv/QLinearMatMul 等
       整数算术算子**属另一族**，不在本门
     · 输入整型集合**比规范多收 int16/uint16**（公式与宽度无关，多收不会引入静默错值）
     · float16/bf16：只覆盖「读 / 写往返 / 特定位型（次正规、中点、最大值）」；
       未覆盖 **inf/NaN 的写路径** —— `onnx_t_geti` 的 `(int64_t)double` 对非有限值是 UB，
       M306 只把 **Cast 的浮点目标**绕开了它，其它算子的整型目标仍可能踩到（下一轮候选）
     · 真实 int8 模型端到端：**已在 M306 收尾实跑通过** —— `/data/app/ws/models/embed/model.onnx`
       （102 MB · 1177 节点 · int8 量化 bge）经 m157 的层 ⑦（`M157_REAL=1`）跑出
       `shape=[1,512,768] norm=1.0 cos=0.94 nodes=1177`。
       **本门不跑它**：一次约 7 分钟 + 2 GB 级中间张量，会把全量门的逐门超时顶穿
       （理由同 `M157_REAL_REF`：它依赖的是**环境资产**，不是仓库资产）。
EOF

echo "── M306 门结束：失败 $FAIL 项 ──"
[ "$FAIL" -eq 0 ] && echo "M306-VERIFY-OK" || echo "M306-VERIFY-FAIL"
exit $(( FAIL > 0 ? 1 : 0 ))
