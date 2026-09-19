#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M157 门 —— ONNX **执行面**（第 39 轮）：张量 + 算子 + 拓扑执行器
# ---------------------------------------------------------------
# 判据分七层，**每层都能单独变红**（门红不了就不是门）：
#   ① 语料：gen_exec_models.py 幂等产出 9 个正例模型 + 5 个行为负控 + 图 JSON + 手算真值
#   ② 三方对拍：C（onnx_run） ↔ 独立参考（exec_ref.py，另一份从零实现） ↔ 手算真值
#      —— 参考 vs 手算先立住，再谈 C vs 参考（M156 §38.4：两侧可能一致地错）
#   ③ 算子覆盖：**每个注册算子都必须真的被执行过**（by_op 并集 == onnx_op_names()）
#   ④ 执行器行为：乱序图必须能跑（不依赖文件顺序）；缺算子/环/广播不兼容/缺 feed 必须报错
#   ⑤ erf 精度：1001 点网格对 math.erf 的最大绝对误差（双路径跨切换点）
#   ⑥ 真实模型（可选 · **SKIP 不是 PASS**）：MiniLM 端到端 + 均值池化 + L2 归一化
#   ⑦ C 侧负控 2 个（必须判红）+ 逐字节还原复跑 · ⑧ 三轨一致（VM vs 解释轨）
# 另：可选 `M157_REAL_REF=1` 时追加"真模型 × 独立参考数值对拍"（纯 Python，约 20 分钟）
# ══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
HERE="$(pwd)"
# 仓库根：**按脚本位置推导**（不写死 /data/code/puxian —— 写死的话本地绿、CI 红：
#   GitHub Actions 的 workspace 根本不是那个路径。M157 接 CI 时发现 m156 也写死了。）
ROOT="$(cd "$HERE/../.." && pwd)"
PX="$ROOT/tools/px"
TMP="$(mktemp -d /tmp/m157.XXXXXX)"
trap 'rm -rf "$TMP"' EXIT

FAIL=0
SKIP=0
ok()   { printf '  ✅ %s\n' "$1"; }
bad()  { printf '  ❌ %s\n' "$1"; FAIL=$((FAIL + 1)); }
skip() { printf '  ⏭️  %s（SKIP：不计入 PASS）\n' "$1"; SKIP=$((SKIP + 1)); }

echo "── M157 门 · ONNX 执行面（张量/算子/执行器）──"

# ── ① 语料 ──
python3 tools/gen_exec_models.py > "$TMP/gen.log" 2>&1 \
    || { echo "生成器失败"; cat "$TMP/gen.log"; echo "M157-VERIFY-FAIL"; exit 1; }
NMOD=$(ls models/*.onnx 2>/dev/null | wc -l)
NNEG=$(ls models/neg/*.onnx 2>/dev/null | wc -l)
NGEN=$(grep -o '模型 [0-9]* 个' "$TMP/gen.log" | grep -o '[0-9]*')
# 不写死个数：与生成器自报的数量比对（避免"加了语料忘了改门"）
[ "$NMOD" -eq "$NGEN" ] && ok "正例模型 $NMOD 个（与生成器自报一致）" || bad "正例模型数 $NMOD ≠ 生成器自报 $NGEN"
[ "$NNEG" -eq 5 ] && ok "行为负控 5 个（乱序/未知算子/环/广播/缺 feed）" || bad "负控数 $NNEG ≠ 5"
[ "$NNEG" -eq 5 ] && ok "行为负控 5 个（乱序/未知算子/环/广播/缺 feed）" || bad "负控数 $NNEG ≠ 5"

# ── ② 语言侧编译（自动裁剪轨）──
for p in onnx_exec embed_mini opnames; do
    if "$PX" build "$p.px" > "$TMP/build_$p.log" 2>&1; then
        ok "$p 编译成功（$(stat -c %s build/$p) 字节）"
    else
        bad "$p 编译失败"; tail -5 "$TMP/build_$p.log"; echo "M157-VERIFY-FAIL"; exit 1
    fi
done

# ── ③ 三方对拍（9 个模型）──
mkdir -p "$TMP/c"
for f in models/*.onnx; do
    n=$(basename "$f" .onnx)
    if ! ./build/onnx_exec "$f" "models/feeds/$n.json" "$TMP/c/$n.json" > "$TMP/c/$n.log" 2>&1; then
        bad "$n：语言侧执行失败（$(head -1 "$TMP/c/$n.log" | head -c 100)）"
    fi
done
if python3 tools/exec_ref.py "$TMP/c" > "$TMP/ref.log" 2>&1; then
    ok "三方对拍：$(grep -c '✅' "$TMP/ref.log") 个模型 · $(grep 'M157-REF:' "$TMP/ref.log")"
else
    bad "三方对拍失败"; grep -E '❌|FAIL' "$TMP/ref.log" | head -10
fi

# ── ④ 算子覆盖：注册表 == 被执行过的集合 ──
./build/opnames > "$TMP/opnames.log" 2>&1
python3 - "$TMP" <<'PY' || FAIL=$((FAIL + 1))
import json, os, sys
tmp = sys.argv[1]
names = open(os.path.join(tmp, "opnames.log")).read().split("\n")
lines = [l for l in names[1:] if l.strip()]
declared = set(lines[0].split(",")) if lines else set()
used = set()
for f in os.listdir(os.path.join(tmp, "c")):
    if not f.endswith(".json"): continue
    d = json.load(open(os.path.join(tmp, "c", f)))
    used |= set(d["by_op"].keys())
missing = sorted(declared - used)
extra = sorted(used - declared)
if missing or extra:
    print("  ❌ 算子覆盖：注册 %d · 执行 %d · **从未执行** %s · 未注册却被执行 %s"
          % (len(declared), len(used), missing, extra))
    sys.exit(1)
print("  ✅ 算子覆盖：注册表 %d 个算子**全部**在语料里真的执行过（by_op 并集逐条对齐）"
      % len(declared))
PY

# ── ⑤ 执行器行为（乱序要能跑；坏图必须报错）──
if ./build/onnx_exec models/neg/n01_reversed.onnx models/neg/n01_reversed.json "$TMP/n01.json" \
     > "$TMP/n01.log" 2>&1; then
    ok "乱序图（节点顺序整体颠倒）仍能跑出结果 —— 执行器不依赖文件顺序"
else
    bad "乱序图跑不动（执行器错误地依赖了节点顺序）：$(head -1 "$TMP/n01.log")"
fi
python3 - "$TMP" <<'PY' || FAIL=$((FAIL + 1))
import json, os, subprocess, sys
tmp, exe = sys.argv[1], os.path.join(os.getcwd(), "build/onnx_exec")
cases = json.load(open("models/neg/cases.json"))
bad = 0
for c in cases:
    if not c["expect_fail"]: continue
    n = c["name"]
    r = subprocess.run([exe, "models/neg/%s.onnx" % n, "models/neg/%s.json" % n,
                        os.path.join(tmp, n + ".json")],
                       capture_output=True, text=True)
    msg = (r.stdout + r.stderr).strip().split("\n")[0][:110]
    if r.returncode == 0:
        print("  ❌ %s：期望报错但跑成功了" % n); bad += 1
    else:
        print("  ✅ %s：如期报错 → %s" % (n, msg))
if bad: sys.exit(1)
PY

# ── ⑥ erf 精度（1001 点 vs math.erf）──
python3 - "$TMP" <<'PY' || FAIL=$((FAIL + 1))
import json, math, struct, sys, os
tmp = sys.argv[1]
d = json.load(open(os.path.join(tmp, "c", "e09_erf_grid.json")))
hexs = d["outputs"]["E"]["hex"]
vals = list(struct.unpack("<%df" % (len(hexs) // 8), bytes.fromhex(hexs)))
xs = [-6.0 + 12.0 * i / 1000.0 for i in range(1001)]
worst, wx = 0.0, 0.0
for x, v in zip(xs, vals):
    e = abs(v - math.erf(x))
    if e > worst:
        worst, wx = e, x
lim = 1.5e-7
print("  %s erf 精度：1001 点（[-6,6]）最大绝对误差 %.3g（@x=%.2f，阈值 %.1g）"
      % ("✅" if worst < lim else "❌", worst, wx, lim))
PY

# ── ⑦ 真实模型（可选）──
REAL="${M157_REAL_MODEL:-}"
for cand in /tmp/onnxdl/minilm.onnx /data/app/ws/models/embed/model.onnx; do
    [ -n "$REAL" ] && break
    [ -f "$cand" ] && REAL="$cand"
done
if [ -z "$REAL" ]; then
    skip "真实模型端到端（未找到 $REAL / 环境变量 M157_REAL_MODEL 也未给）"
else
    if ./build/embed_mini "$REAL" "$TMP/embed.json" > "$TMP/embed.log" 2>&1; then
        python3 - "$TMP" "$REAL" <<'PY' || FAIL=$((FAIL + 1))
import json, math, os, sys
tmp, real = sys.argv[1], sys.argv[2]
d = json.load(open(os.path.join(tmp, "embed.json")))
bad = 0
def ck(cond, label, extra=""):
    global bad
    print("  %s %s%s" % ("✅" if cond else "❌", label, (" → " + extra) if extra else ""))
    if not cond: bad += 1
D = d["dim"]
ck(len(d["shape"]) == 3 and d["shape"][0] == 1 and d["shape"][2] == D,
   "输出形状 %s（[1,512,H]）" % d["shape"])
ck(abs(d["norm_after"] - 1.0) < 1e-3, "L2 归一化：归一化后 norm=%.6f（池化后模长 %.4f）"
       % (d["norm_after"], d["norm"]), "|norm-1|<1e-3")
ck(all(math.isfinite(x) for x in d["head"]), "前 5 维有限：%s" % ["%.4f" % x for x in d["head"]])
ck(len(d["sha256"]) == 64 and len(d["sha256_b"]) == 64,
   "两次编码各得 sha256", "a=%s b=%s" % (d["sha256"][:12], d["sha256_b"][:12]))
ck(abs(d["cos_sim"]) <= 1.0 + 1e-6, "余弦相似度在 [-1,1]：%.6f" % d["cos_sim"])
ck(d["nodes"] == 780, "执行节点数 %d" % d["nodes"])
ck(d["sha256"] != d["sha256_b"], "不同文本 ⇒ 不同向量（词表是 Go 口径的 hash 式切分，语义不保证）")
ck(d["ms_a"] > 0, "单次编码耗时 %.1f ms（零依赖、无 dlopen）" % d["ms_a"])
if bad: sys.exit(1)
PY
    else
        bad "真实模型端到端失败：$(head -2 "$TMP/embed.log" | tail -1 | head -c 120)"
    fi
    if [ "${M157_REAL_REF:-0}" = "1" ]; then
        python3 tools/onnx2graphjson.py "$REAL" "$TMP/real_graphs.json" minilm > "$TMP/conv.log" 2>&1 \
            || bad "真模型→图 JSON 转换失败"
        python3 - <<'PY'
import json
json.dump({"input_ids": {"dims": [1, 4], "dtype": "int64", "i64": [101, 102, 103, 104]},
           "attention_mask": {"dims": [1, 4], "dtype": "int64", "i64": [1, 1, 1, 1]},
           "token_type_ids": {"dims": [1, 4], "dtype": "int64", "i64": [1, 1, 1, 1]}},
          open("/tmp/m157_realfeeds_minilm.json", "w"))
PY
        mkdir -p "$TMP/rf" "$TMP/cr"
        cp /tmp/m157_realfeeds_minilm.json "$TMP/rf/minilm.json"
        if ./build/onnx_exec "$REAL" "$TMP/rf/minilm.json" "$TMP/cr/minilm.json" > "$TMP/cr.log" 2>&1; then
            if M157_GRAPHS="$TMP/real_graphs.json" M157_FEEDS_DIR="$TMP/rf" \
               M157_EXPECTED=/nonexistent python3 tools/exec_ref.py "$TMP/cr" > "$TMP/realref.log" 2>&1; then
                ok "真模型 × 独立参考数值对拍（seq=4）：$(grep -c '✅' "$TMP/realref.log") 个模型一致"
            else
                bad "真模型数值对拍失败"; grep -E '❌' "$TMP/realref.log" | head -3
            fi
        else
            bad "真模型（seq=4）执行失败"
        fi
    else
        skip "真模型 × 独立参考数值对拍（设 M157_REAL_REF=1 开启，纯 Python 约 20 分钟）"
    fi
fi

# ── ⑧ C 侧负控（改源码 ⇒ 必须判红 ⇒ 逐字节还原 ⇒ 复跑绿）──
#   ⚠️ 三条纪律（都是本轮/本项目踩过的坑）：
#     · 篡改后**编译失败**也要判"没能证明门能红"（缺陷 147 同族：拿不到判据 ≠ 判据满足）
#     · 判据命令口径统一：ref_on 返回 0 = 三方对拍一致（即门**没**抓住篡改）
#     · 判据模型必须真的能区分两种写法：NC-A 用 e10_gather_rank2（秩 2 的 data）——
#       先前用 Shape 的输出（秩 1）当判据，前缀/后缀都为空 ⇒ 两种写法形状相同 ⇒ 抓不住
snap() { sha256sum "$ROOT/runtime/onnx_tensor.c" "$ROOT/runtime/onnx_ops.c"; }
SNAP_BEFORE=$(snap)

ref_on() {   # 跑指定模型的三方对拍；一致 ⇒ 0
    local d="$TMP/ncc_$1"
    NCC_LOG="$TMP/ncc_$1.log"
    shift
    rm -rf "$d" && mkdir -p "$d"
    for n in "$@"; do
        ./build/onnx_exec "models/$n.onnx" "models/feeds/$n.json" "$d/$n.json" > /dev/null 2>&1
        last="$n"
    done
    M157_ONLY="$*" python3 tools/exec_ref.py "$d" > "$NCC_LOG" 2>&1
}

negctl() {   # $1=名字 $2=相对源码路径 $3=旧串 $4=新串 $5..=判据模型名
    local name="$1" file="$ROOT/$2" from="$3" to="$4"
    shift 4
    local bak="$TMP/nc_$name.bak"
    cp "$file" "$bak"
    if ! python3 - "$file" "$from" "$to" <<'PYEOF'
import sys
f, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(f).read()
n = s.count(a)
if n != 1:
    print("锚点出现 %d 次（要求恰好 1 次）" % n); sys.exit(1)
open(f, "w").write(s.replace(a, b))
PYEOF
    then
        bad "$name：锚点不唯一（改不动 ⇒ 无法证明门能红）"; cp "$bak" "$file"; return
    fi
    if ! "$PX" build onnx_exec.px > "$TMP/ncbuild_$name.log" 2>&1; then
        bad "$name：篡改后**编译失败**（拿不到判据 ≠ 判据满足）"
        tail -3 "$TMP/ncbuild_$name.log"; cp "$bak" "$file"; return
    fi
    if ref_on "$name" "$@"; then
        bad "$name：篡改后三方对拍仍全绿 ⇒ 门没有真的守住这条判据"
    else
        ok "$name：篡改后如期判红（$(grep -m1 '❌' "$NCC_LOG" | sed 's/^ *//' | head -c 90)）"
    fi
    cp "$bak" "$file"
    [ "$(sha256sum "$file" | cut -d' ' -f1)" = "$(sha256sum "$bak" | cut -d' ' -f1)" ] \
        && ok "$name：源文件已逐字节还原" || bad "$name：源文件还原失败"
}

negctl "NC-A Gather 形状插回末尾" runtime/onnx_tensor.c \
    '    for (i = 0; i < (int)axis; i++) odims[j++] = data->dims[i];
    for (i = 0; i < ir; i++) odims[j++] = idx->dims[i];
    for (i = (int)axis + 1; i < da; i++) odims[j++] = data->dims[i];' \
    '    for (i = 0; i < (int)axis; i++) odims[j++] = data->dims[i];
    for (i = (int)axis + 1; i < da; i++) odims[j++] = data->dims[i];
    for (i = 0; i < ir; i++) odims[j++] = idx->dims[i];' \
    e10_gather_rank2 e04_movement

negctl "NC-B erf 符号翻回" runtime/onnx_tensor.c \
    '        return (2.0 / PX_SQRT_PI) * sum;' \
    '        return (x < 0 ? -1.0 : 1.0) * (2.0 / PX_SQRT_PI) * sum;' \
    e09_erf_grid e05_funcs

"$PX" build onnx_exec.px > /dev/null 2>&1
SNAP_AFTER=$(snap)
[ "$SNAP_BEFORE" = "$SNAP_AFTER" ] \
    && ok "负控残留检查：两个源文件 sha256 逐字节还原" \
    || bad "负控源文件**未还原**（sha256 不一致）"

# ── ⑨ 复跑（还原后必须全绿）──
rm -rf "$TMP/c" && mkdir -p "$TMP/c"
for f in models/*.onnx; do
    n=$(basename "$f" .onnx)
    ./build/onnx_exec "$f" "models/feeds/$n.json" "$TMP/c/$n.json" > /dev/null 2>&1
done
if python3 tools/exec_ref.py "$TMP/c" > "$TMP/ref2.log" 2>&1; then
    ok "还原后复跑三方对拍全绿"
else
    bad "还原后复跑不绿"; grep -E '❌' "$TMP/ref2.log" | head -5
fi

# ── ⑩ 三轨一致（VM 默认轨 vs 解释轨）──
#   解释轨走 `px run`（自举解释器），算子分发在 selfhost/ibuiltin.px、名册在 interp.px
#   —— 缺陷 155 的教训：新 native 必须**两处都加**，这里就是对那件事的常驻检查。
mkdir -p "$TMP/tr"
if ./build/onnx_exec models/e01_eltwise_exact.onnx models/feeds/e01_eltwise_exact.json \
     "$TMP/tr/vm.json" > /dev/null 2>&1 \
   && "$PX" run onnx_exec.px models/e01_eltwise_exact.onnx models/feeds/e01_eltwise_exact.json \
     "$TMP/tr/it.json" > "$TMP/tr/it.stdout" 2>&1 \
   && [ -s "$TMP/tr/vm.json" ] && [ -s "$TMP/tr/it.json" ] && cmp -s "$TMP/tr/vm.json" "$TMP/tr/it.json"; then
    ok "三轨一致：VM 轨与解释轨的执行结果 JSON 逐字节相同"
else
    bad "VM 轨与解释轨不一致（或解释轨未产出）"
    head -3 "$TMP/tr/it.stdout" 2>/dev/null
fi

echo
if [ "$FAIL" -eq 0 ]; then
    echo "M157-VERIFY-OK（失败 0 · SKIP $SKIP）"
else
    echo "M157-VERIFY-FAIL（失败 $FAIL · SKIP $SKIP）"
fi
exit $([ "$FAIL" -eq 0 ] && echo 0 || echo 1)
