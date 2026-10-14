#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M308 门 —— ONNX **推理算子补齐**（S4：Gelu / Silu / RMSNormalization / ScatterND / Cos / Sin
#              + `Trilu` 规范名与历史名 `Triu` 并存）
# ---------------------------------------------------------------
# 立论：M157（执行面）/ M306（量化面）之后，执行器已能跑 MatMul 与 int8 量化，但
#   **跑不了一个 transformer 解码步** —— 缺 GELU/SiLU/RMSNorm（前馈+归一化）、
#   KV cache 写入（ScatterND）、RoPE 前置（Cos/Sin）；且 ONNX **规范名 `Trilu`**
#   与本仓历史注册名 `Triu` 不一致，而算子查找走**精确 strcmp** ⇒ 真实模型里的
#   Trilu 节点会「未实现的算子」。⇒ 本门把 6 个算子 + 1 个命名缺口做成**清单级判据**。
#
# 判据分九层，**每层都能单独变红**（门红不了就不是门）：
#   ① 语料：gen_m308_models.py 幂等产出 3 个正例 + 7 个行为负控 + graphs/feeds/expected
#   ② 编译：语言侧驱动 onnx_exec.px
#   ③ **三方对拍三条腿**（各自独立报数，互不代偿）：
#        ③a 手算真值 ↔ 独立参考（../m157_onnx_exec/tools/exec_ref.py）↔ C 侧
#        ③b **复算腿**：tools/ops_ref.py 把 5 个算子（Silu/Cos/Sin/RMSNorm/ScatterND）
#            从零重算一遍 —— 它**刻意不复用 ③a 的参考**：③a 与 C 侧若一致地错，
#            门照样全绿（M156 §38.4 的原话）
#        ③c **分档腿**：tools/gelu_tiers.py 断言 **erf 档与 tanh 档逐元素可判异**
#            —— 两档被写成同一式时，③a/③b **一致地错**，只有这一条能戳穿
#        ③d **等价腿**：tools/kv_equiv.py 判 **KV cache 增量解码**「逐步 ScatterND 写入」
#            与「一次性 Concat 全长」**逐值等价**（严格相等，无容差）—— M308-PLAN §二.S2
#            的硬判据。⚠️ 这条关系 ③a/③b **表达不了**（它们判的是「图 vs 真值」，不是
#            「两个输出互等」）；且必须附「写入真的发生了」判据（否则两边同返回原值也过关）
#   ④ 行为负控：7 个非法形态**每个都必须响亮报错**，且报错必须来自**它自己那个算子**
#      （cases.json 逐条登记 `op`；防「报错是别的节点崩的」）
#   ⑤ 算子覆盖：6 个新算子 + `Trilu` 规范名必须**真的被执行过**（by_op 并集）
#   ⑥ 静态：源码在位（**去注释后**判）—— 注册 / 两档分叉 / 越界守卫 / 单一实现
#   ⑦ 源码负控 5 道（必须判红 ⇒ 逐字节还原 ⇒ 复跑绿），**每道打在不同的判据层**
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
TMP="$(mktemp -d /tmp/m308.XXXXXX)"
trap 'rm -rf "$TMP"' EXIT

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

FAIL=0
ok()   { printf '  ✅ %s\n' "$1"; }
bad()  { printf '  ❌ %s\n' "$1"; FAIL=$((FAIL + 1)); }

echo "── M308 门 · ONNX 推理算子补齐（Gelu/Silu/RMSNormalization/ScatterND/Cos/Sin + Trilu 别名）──"

# ── ① 语料 ──
python3 tools/gen_m308_models.py > "$TMP/gen.log" 2>&1 \
    || { echo "生成器失败"; cat "$TMP/gen.log"; echo "M308-VERIFY-FAIL"; exit 1; }
NMOD=$(ls models/*.onnx 2>/dev/null | wc -l)
NNEG=$(ls models/neg/*.onnx 2>/dev/null | wc -l)
NGEN=$(grep -o '正例模型 [0-9]* 个' "$TMP/gen.log" | head -1 | grep -o '[0-9]*')
NCASE=$(python3 -c "import json;print(len(json.load(open('models/neg/cases.json'))))")
# 不写死个数：与生成器自报的数量比对（避免"加了语料忘了改门"）
[ "$NNEG" -eq "$NCASE" ] && ok "负控用例表 $NCASE 条与磁盘 $NNEG 个模型一致" \
                         || bad "负控表 $NCASE ≠ 磁盘 $NNEG"
[ -n "$NGEN" ] && [ "$NMOD" -eq "$NGEN" ] && ok "正例模型 $NMOD 个（与生成器自报一致）" \
                                          || bad "正例模型数 $NMOD ≠ 生成器自报 ${NGEN:-?}"
# 真值覆盖：expected.json 里的每个模型/输出都必须**在 graphs.json 里真实存在**（防悬空真值）；
#   且 e13/e15 必须**逐输出**有手算真值（e14 是属性语义专测 —— 它的判据是 ③b 的独立复算，
#   不是手算值；这一点在 gen 的 judgment() 注释里已写明，此处如实登记为**有意的不对称**）
if python3 - <<'PY'
import json, sys
g = json.load(open("models/graphs.json"))
exp = json.load(open("models/expected.json"))
assert exp, "expected.json 为空 ⇒ 手算真值腿是空转"
for m, outs in exp.items():
    assert m in g, "悬空真值：%s 不在 graphs.json 里" % m
    declared = {o["name"] for o in g[m]["outputs"]}
    for o in outs:
        assert o in declared, "悬空输出真值：%s.%s 不在图里" % (m, o)
for m in exp:   # 凡有手算真值的模型必须**逐输出**齐备（e14 不在 exp 里 —— 它的判据是 ③b 的独立复算）
    want = {o["name"] for o in g[m]["outputs"]}
    assert want <= set(exp[m]), "%s 真值不全：缺 %s" % (m, sorted(want - set(exp[m])))
print("真值覆盖 %d 个模型：%s（e14 由 ③b 独立复算覆盖）"
      % (len(exp), ",".join(sorted(exp))))
PY
then ok "手算真值覆盖：悬空真值 0 · 每个有真值的模型都逐输出齐备"; else bad "真值覆盖不全"; fi

# ── ② 编译 ──
if "$PX" build onnx_exec.px > "$TMP/build.log" 2>&1; then
    ok "onnx_exec 编译成功（$(stat -c %s build/onnx_exec) 字节）"
else
    bad "onnx_exec 编译失败"; tail -5 "$TMP/build.log"; echo "M308-VERIFY-FAIL"; exit 1
fi

gen_c() {   # 跑所有正例 → $TMP/c（可反复调用：负控后要重跑）
    rm -rf "$TMP/c"; mkdir -p "$TMP/c"
    for f in models/*.onnx; do
        n=$(basename "$f" .onnx)
        ./build/onnx_exec "$f" "models/feeds/$n.json" "$TMP/c/$n.json" \
            > "$TMP/c/$n.log" 2>&1
    done
}

# ── ③a 三方对拍（手算真值 ↔ 独立参考 ↔ C 侧）──
echo "── ③a 三方对拍（手算真值 ↔ 独立参考 ↔ C 侧）──"
gen_c
if M157_GRAPHS="$HERE/models/graphs.json" M157_FEEDS_DIR="$HERE/models/feeds" \
   M157_EXPECTED="$HERE/models/expected.json" \
   python3 "$ROOT/examples/m157_onnx_exec/tools/exec_ref.py" "$TMP/c" > "$TMP/ref.log" 2>&1; then
    ok "三方对拍全绿：$(grep -c '✅' "$TMP/ref.log") 个模型 · $(grep 'M157-REF:' "$TMP/ref.log")"
else
    bad "三方对拍失败"; grep -E '❌|FAIL' "$TMP/ref.log" | head -8
fi

# ── ③b 复算腿（5 个算子从零复算，刻意不复用 ③a 的参考）──
echo "── ③b 复算腿（独立第三份实现）──"
if python3 tools/ops_ref.py "$TMP/c" > "$TMP/opsref.log" 2>&1; then
    ok "复算腿全绿：$(grep -c '✅' "$TMP/opsref.log") 条 · $(grep 'M308-OPSREF:' "$TMP/opsref.log")"
else
    bad "复算腿判红"; grep -E '❌' "$TMP/opsref.log" | head -6
fi

# ── ③c 分档腿（Gelu 两档可判异 —— 唯一能戳穿「两档同式」的判据）──
echo "── ③c 分档腿（Gelu 两档可判异）──"
if python3 tools/gelu_tiers.py "$TMP/c/e13_s4_ops.json" \
      models/feeds/e13_s4_ops.json > "$TMP/gelu.log" 2>&1; then
    ok "分档腿全绿：$(grep -c '✅' "$TMP/gelu.log") 条（含「两档逐元素可判异」）"
else
    bad "分档腿判红"; grep -E '❌' "$TMP/gelu.log" | head -4
fi

# ── ③d 等价腿（KV cache：增量 ⇄ 全长 逐值等价 · §二.S2 硬判据）──
echo "── ③d 等价腿（KV cache 增量解码 ⇄ 全长）──"
if python3 tools/kv_equiv.py "$TMP/c" > "$TMP/kveq.log" 2>&1; then
    ok "等价腿全绿：$(grep 'M308-KVEQ:' "$TMP/kveq.log")"
else
    bad "等价腿判红"; grep -E '❌' "$TMP/kveq.log" | head -4
fi

# ── ④ 行为负控：7 个非法形态必须逐个响亮报错（且来自它自己那个算子）──
echo "── ④ 行为负控 ──"
python3 - "$TMP" <<'PY' && ok "行为负控 $NCASE 条：全部响亮报错（指名算子 + 给出实质原因）" \
                         || FAIL=$((FAIL + 1))
import json, os, re, subprocess, sys
tmp = sys.argv[1]
cases = json.load(open("models/neg/cases.json"))
bad = 0
for c in cases:
    n = c["name"]
    r = subprocess.run(["./build/onnx_exec", "models/neg/%s.onnx" % n,
                        "models/neg/%s.json" % n, os.path.join(tmp, n + ".json")],
                       capture_output=True, text=True)
    msg = (r.stdout + r.stderr).strip().split("\n")[0]
    if r.returncode == 0:
        print("  ❌ %s：期望报错但跑成功了（%s）" % (n, c["why"])); bad += 1; continue
    m = re.search(r"（op=([A-Za-z]+)）执行失败：(.+)", msg)
    if not m:
        print("  ❌ %s：不是算子层的响亮错误 → %s" % (n, msg[:100])); bad += 1; continue
    opname = m.group(1)
    inner = re.sub(r"[)）\s]+$", "", m.group(2)).strip()
    if opname != c["op"]:
        print("  ❌ %s：报的算子 %r ≠ 期望 %r（报错来自别的节点？）"
              % (n, opname, c["op"])); bad += 1; continue
    if len(inner) < 8:
        print("  ❌ %s：只有外壳、没有实质原因（inner=%r）" % (n, inner)); bad += 1; continue
    print("  ✅ %s：%s → %s" % (n, opname, inner[:80]))
if bad:
    sys.exit(1)
PY

# ── ⑤ 算子覆盖：6 个新算子 + Trilu 规范名必须真的被执行过 ──
echo "── ⑤ 算子覆盖 ──"
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
want = {"Gelu", "Silu", "RMSNormalization", "ScatterND", "Cos", "Sin", "Trilu"}
miss = want - used
if miss:
    print("  ❌ 算子覆盖：语料从没执行过 %s（by_op 并集 %d 个）" % (sorted(miss), len(used)))
    sys.exit(1)
print("  ✅ 算子覆盖：本门 %d 个算子/别名**全部**在语料里真的执行过（by_op 并集 %d 个算子）"
      % (len(want), len(used)))
PY

# ── ⑥ 静态：源码在位（**去注释后**判，防"注释里写着修了"）──
echo "── ⑥ 静态判据 ──"
python3 - "$ROOT" <<'PY' || FAIL=$((FAIL + 1))
import re, sys
root = sys.argv[1]
s = open(root + "/runtime/onnx_ops.c", encoding="utf-8").read()
s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
s = re.sub(r"//[^\n]*", "", s)
bad = 0

def chk(cond, msg):
    global bad
    print("  %s %s" % ("✅" if cond else "❌", msg))
    if not cond:
        bad += 1

def cnt(sub):
    return s.count(sub)

for op, fn in (("Gelu", "onnx_op_gelu"), ("Silu", "onnx_op_silu"),
               ("RMSNormalization", "onnx_op_rmsnorm"), ("ScatterND", "onnx_op_scatternd"),
               ("Cos", "onnx_op_cos"), ("Sin", "onnx_op_sin")):
    chk(cnt('"%s", %s' % (op, fn)) == 1, "算子表注册 %s（1 处）" % op)
chk(cnt('"Triu", onnx_op_triu') == 1 and cnt('"Trilu", onnx_op_triu') == 1,
    "**两名并存**：Triu（历史名）与 Trilu（规范名）都在表里，且指向**同一个实现**")
chk(cnt('approx = onnx_attr_s(attrs, n_attr, "approximate", &slen)') == 1
    and cnt("use_tanh") >= 3,
    "Gelu 两档在同一实现里按 `approximate` 分叉（use_tanh 分支在位）")
chk(cnt("onnx_erf(x * 0.7071067811865476)") == 1
    and cnt("tanh(c * (x + 0.044715 * x * x * x))") == 1,
    "Gelu **erf 档与 tanh 档是两个不同表达式**（写成同一式 ⇒ ③c 必红）")
chk(cnt("x > 0 ? x / (1.0 + e) : x * e / (1.0 + e)") == 1,
    "Silu **单实现**（全文件只有一处公式 —— 不许复合路径与算子路径各写一份）")
chk(cnt("RMSNormalization: axis 越界") == 1, "RMSNormalization：axis 越界响亮拒绝在位")
chk(cnt("RMSNormalization: scale 元素数") == 1, "RMSNormalization：scale 长度校验在位")
chk(cnt("ScatterND: 下标越界") == 1, "ScatterND：下标越界响亮拒绝在位")
chk(cnt("ScatterND: updates 元素数") == 1, "ScatterND：updates 元素数校验在位")
chk(cnt("ScatterND: indices 末维 %lld 超过 data 秩") == 1
    and cnt("ScatterND: indices 末维（k）需 ≥1") == 1,
    "ScatterND：k ≥ 1 且 k ≤ rank(data) 两条校验都在位")
chk(cnt("ScatterND: data 与 updates 类型不一致") == 1, "ScatterND：dtype 一致性校验在位")
sys.exit(1 if bad else 0)
PY

# ── ⑦ 源码负控（5 道，每道打在**不同的判据层**）──
patch1() {   # $1=文件 $2=旧 $3=新（旧串必须恰好 1 次；否则判"锚点不唯一"）
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
GELU="$HERE/tools/gelu_tiers.py"

NC_A_FROM='            r = 0.5 * x * (1.0 + tanh(c * (x + 0.044715 * x * x * x)));'
NC_A_TO='            (void)c; r = 0.5 * x * (1.0 + onnx_erf(x * 0.7071067811865476));'
NC_B_FROM='            if (v < 0 || v >= data->dims[j])
                FAILF("ScatterND: 下标越界（第 %lld 维取 %lld，合法区间 [0,%lld)）",
                      (long long)j, (long long)v, (long long)data->dims[j]);'
NC_B_TO='            if (v < 0 || v >= data->dims[j]) v = 0;'
NC_C_FROM='    { "Trilu", onnx_op_triu },'
NC_C_TO='    /* NC-C: Trilu 别名已移除 */'
NC_E_FROM='        onnx_t_setf(*out, i, x > 0 ? x / (1.0 + e) : x * e / (1.0 + e));'
NC_E_TO='        (void)e; onnx_t_setf(*out, i, x);'
NC_D_FROM='    return 1 if bad else 0'
NC_D_TO='    return 0'
NC_F_FROM='            base += v * dst[j];'
NC_F_TO='            (void)v; base += 0;'

SNAP_BEFORE=$(sha256sum "$OPS" "$GELU" | sha256sum)

if [ "$NEG_SKIP" = "0" ]; then
    cp "$OPS" "$TMP/nc_ops.bak"; cp "$GELU" "$TMP/nc_gelu.bak"
    restore() { cp "$TMP/nc_ops.bak" "$OPS"; cp "$TMP/nc_gelu.bak" "$GELU"; }

    # NC-A：Gelu 两档写成同一式 ⇒ ③c 分档腿必须红（且红点必须包含「两档可判异」那条）
    if ! patch1 "$OPS" "$NC_A_FROM" "$NC_A_TO"; then
        bad "NC-A：锚点不唯一（改不动 ⇒ 无法证明门能红）"
    elif ! "$PX" build onnx_exec.px > "$TMP/ncbuild_a.log" 2>&1; then
        bad "NC-A：篡改后**编译失败**（拿不到判据 ≠ 判据满足）"; tail -3 "$TMP/ncbuild_a.log"
    else
        gen_c
        if python3 tools/gelu_tiers.py "$TMP/c/e13_s4_ops.json" models/feeds/e13_s4_ops.json \
             > "$TMP/nc_a.log" 2>&1; then
            bad "NC-A：两档写成同一式后分档腿仍全绿 ⇒ 那条判据没有牙"
        elif ! grep -q '两档逐元素可判异' "$TMP/nc_a.log"; then
            bad "NC-A：分档腿红了，但红点**不是**「两档可判异」（$(grep -m1 '❌' "$TMP/nc_a.log")）"
        else
            ok "NC-A：两档同式 ⇒ 分档腿如期判红（$(grep -m1 '❌.*两档' "$TMP/nc_a.log" | sed 's/^ *//')）"
        fi
    fi
    restore

    # NC-B：ScatterND 越界改成静默钳制 ⇒ ④ 行为负控 n01 必须从「报错」变成「跑通」
    if ! patch1 "$OPS" "$NC_B_FROM" "$NC_B_TO"; then
        bad "NC-B：锚点不唯一"
    elif ! "$PX" build onnx_exec.px > "$TMP/ncbuild_b.log" 2>&1; then
        bad "NC-B：篡改后编译失败"; tail -3 "$TMP/ncbuild_b.log"
    else
        out=$(./build/onnx_exec models/neg/n01_scatternd_oob.onnx \
                models/neg/n01_scatternd_oob.json "$TMP/n01.json" 2>&1); rc=$?
        if [ "$rc" -eq 0 ]; then
            ok "NC-B：去掉越界守卫后 n01 如期『跑通』⇒ 负控层有牙（$(echo "$out" | head -1)）"
        else
            bad "NC-B：去掉守卫后 n01 仍报错 ⇒ 那条红不是来自这个守卫（rc=$rc）"
        fi
    fi
    restore

    # NC-C：摘掉 Trilu 注册行 ⇒ 只用规范名的 e15 必须跑不起来（运行时判据）
    if ! patch1 "$OPS" "$NC_C_FROM" "$NC_C_TO"; then
        bad "NC-C：锚点不唯一"
    elif ! "$PX" build onnx_exec.px > "$TMP/ncbuild_c.log" 2>&1; then
        bad "NC-C：篡改后编译失败"; tail -3 "$TMP/ncbuild_c.log"
    else
        out=$(./build/onnx_exec models/e15_trilu_alias.onnx models/feeds/e15_trilu_alias.json \
                "$TMP/e15.json" 2>&1); rc=$?
        if [ "$rc" -ne 0 ] && echo "$out" | grep -q "未实现的算子"; then
            ok "NC-C：摘掉 Trilu 别名后 e15 如期失败（$(echo "$out" | head -1)）"
        else
            bad "NC-C：摘掉别名后 e15 仍能跑（rc=$rc）⇒ ⑤ 的运行时判据没有牙"
        fi
    fi
    restore

    # NC-E：Silu 公式改成恒等 ⇒ ③b 复算腿必须红（证明复算腿独立有牙）
    if ! patch1 "$OPS" "$NC_E_FROM" "$NC_E_TO"; then
        bad "NC-E：锚点不唯一"
    elif ! "$PX" build onnx_exec.px > "$TMP/ncbuild_e.log" 2>&1; then
        bad "NC-E：篡改后编译失败"; tail -3 "$TMP/ncbuild_e.log"
    else
        gen_c
        if python3 tools/ops_ref.py "$TMP/c" > "$TMP/nc_e.log" 2>&1; then
            bad "NC-E：Silu 改成恒等后复算腿仍全绿 ⇒ 复算腿没有牙"
        else
            ok "NC-E：Silu 改恒等 ⇒ 复算腿如期判红（$(grep -m1 '❌' "$TMP/nc_e.log" | sed 's/^ *//')）"
        fi
    fi
    restore

    # NC-F：ScatterND 写入槽位算错（恒写第 0 槽）⇒ ③d 等价腿必须红
    if ! patch1 "$OPS" "$NC_F_FROM" "$NC_F_TO"; then
        bad "NC-F：锚点不唯一"
    elif ! "$PX" build onnx_exec.px > "$TMP/ncbuild_f.log" 2>&1; then
        bad "NC-F：篡改后编译失败"; tail -3 "$TMP/ncbuild_f.log"
    else
        rm -rf "$TMP/f" && mkdir -p "$TMP/f"
        ./build/onnx_exec models/e16_kv_decode.onnx models/feeds/e16_kv_decode.json \
            "$TMP/f/e16_kv_decode.json" > /dev/null 2>&1
        if python3 tools/kv_equiv.py "$TMP/f" > "$TMP/nc_f.log" 2>&1; then
            bad "NC-F：写错槽位后等价腿仍全绿 ⇒ ③d 没有牙"
        else
            ok "NC-F：写错槽位 ⇒ 等价腿如期判红（$(grep -m1 '❌' "$TMP/nc_f.log" | sed 's/^ *//')）"
        fi
    fi
    restore

    # NC-D：**判据自伤** —— 把分档腿的退出码打成恒 0，叠加 NC-A 的源码篡改 ⇒ NC-A 的红必须消失
    if ! patch1 "$GELU" "$NC_D_FROM" "$NC_D_TO"; then
        bad "NC-D：锚点不唯一"
    else
        patch1 "$OPS" "$NC_A_FROM" "$NC_A_TO" || bad "NC-D：NC-A 锚点不唯一"
        "$PX" build onnx_exec.px > /dev/null 2>&1
        gen_c
        if python3 tools/gelu_tiers.py "$TMP/c/e13_s4_ops.json" models/feeds/e13_s4_ops.json \
             > /dev/null 2>&1; then
            ok "NC-D：判据自伤后 NC-A 的红**如期消失** ⇒ 那道红确实来自分档腿的比较判据"
        else
            bad "NC-D：判据自伤后仍判红 ⇒ 那道红另有来源（判据没被真正关掉）"
        fi
        restore
    fi

    "$PX" build onnx_exec.px > /dev/null 2>&1
    SNAP_AFTER=$(sha256sum "$OPS" "$GELU" | sha256sum)
    [ "$SNAP_BEFORE" = "$SNAP_AFTER" ] \
        && ok "负控残留检查：两个源文件 sha256 逐字节还原" \
        || bad "负控源文件**未还原**"

    gen_c
    if M157_GRAPHS="$HERE/models/graphs.json" M157_FEEDS_DIR="$HERE/models/feeds" \
       M157_EXPECTED="$HERE/models/expected.json" \
       python3 "$ROOT/examples/m157_onnx_exec/tools/exec_ref.py" "$TMP/c" > "$TMP/ref2.log" 2>&1 \
       && python3 tools/ops_ref.py "$TMP/c" > /dev/null 2>&1 \
       && python3 tools/kv_equiv.py "$TMP/c" > /dev/null 2>&1; then
        ok "还原后复跑：三方对拍 + 复算腿 + 等价腿三腿全绿"
    else
        bad "还原后复跑不绿"; grep -E '❌' "$TMP/ref2.log" 2>/dev/null | head -5
    fi
else
    echo "  ⏭️  源码负控 5 道（--neg-skip：由本地全量门/CI 的另一档承担）"
fi

# ── ⑧ 覆盖边界（如实登记 —— 别让"没覆盖"看起来像"覆盖了"）──
cat <<'EOF'
  ℹ️  覆盖边界（如实）：
     · 已覆盖：Gelu（erf 档 / tanh 档 / **两档可判异** / 非法 approximate 响亮拒绝）·
       Silu（含大负 x 的溢出分支 · 与独立复算逐元素对拍）· RMSNormalization
       （默认 epsilon / **显式 epsilon=0.5** / 默认 axis=-1 / **axis=1** / 带 scale /
       不带 scale / scale 长度不符 / axis 越界）· ScatterND（k=2 坐标写入 / 下标越界 /
       updates 元素数不符 / k>rank / dtype 不符）· Cos / Sin（逐元素 · 与 math.cos/sin 独立复算）·
       `Trilu`（**规范名**与历史名 `Triu` 同值 · 摘掉别名即失败）
     · 已覆盖（§二.S2 attention 辅助）：**KV cache 增量解码等价性**（预分配 cache 上逐步
       ScatterND 写入 ⇄ 一次性 Concat 全长 · **逐值严格相等** · 附「写入真的落在目标槽、
       其余槽逐值未动」判据）· **RoPE 复合式**（Cos/Sin + Gather 置换 + 广播乘加串成一张真实
       旋转位置编码图 · 7 节点 · 全部中间量对手算真值）
     · **未覆盖**（如实）：① `Gelu` 的 fp16 输入路径（本门语料只走 fp32 —— fp16 读写往返
       由 m306 门覆盖，但**这两条腿没在本门串起来**）；② RoPE 只覆盖**复合式**一条路径 ——
       com.microsoft `RotaryEmbedding` 单算子（PLAN §二 的可选项）**未注册、未覆盖**；
       ③ KV cache 只覆盖**单步写入**：多步增量（连续 N 次 ScatterND 后与全长逐值等价）
       与 `past_len` 语义**未覆盖**（本轮取 k=1 单槽写入的等价性判据）；
       ④ `onnx_t_geti` 对非有限浮点的整型目标仍是 UB（M306 已登记）
     · 本门**不跑**真实模型端到端（那是 m157 门 `M157_REAL=1` 的事）：本门依赖的是**仓库资产**
EOF

echo "── M308 门结束：失败 $FAIL 项 ──"
[ "$FAIL" -eq 0 ] && echo "M308-VERIFY-OK" || echo "M308-VERIFY-FAIL"
exit $(( FAIL > 0 ? 1 : 0 ))
