#!/usr/bin/env bash
# ============================================================
# M250 门 · **ONNX 张量助手族「逐位置 × 错类型 ∪ 错 arity ∪ 边界下标」**（缺陷 433）
#           \+ **native 覆盖面台账判据化**（M248 的那份一次性账 → 常设判据）
# ------------------------------------------------------------
# 来历：M199 量过 native 函数面「逐位置 × 错类型」、M226/M227 量过方法面与同名两门、
#   M230 量过索引切片、M231 运算符矩阵、M246 复合赋值 —— 但**没有人量过覆盖面本身**：
#   **392 个 native 里有多少个从来没被任何门语料碰到过？** M248 首算 **21 个**。
#   顺着这份账当场证出：
#
#   缺陷 433 —— `bi_f32_at` / `bi_i64_at` 的界判据 `i = (int)args[1].as.i; ...
#     (size_t)(i * 4 + 4) > (size_t)len` 有两处 UB（本仓无 -fwrapv，实测构建就是 -O2 且可叠 -flto）：
#       ① `(int)` **截断**：`2^32` / `2^62` 这类下标静默变成 0 ⇒ **静默返回 0 号元素**
#          （三轨**一致地**错 ⇒ 任何「三轨对拍」门按定义看不见 · 同 M215/M226/M227/M230 家族）；
#       ② `i * 4 + 4` 在 **int32** 里溢出：`2^30-1` ⇒ `-4 + 4 == 0` ⇒ **判据放行** ⇒
#          memcpy 读缓冲**之前** 4 字节（解释轨实测回读堆垃圾，**每次运行都不同** ⇒ 信息泄漏）。
#    实测（修前，同一份源码三轨）：
#       i64_at(b32, 536870911) ⇒ 解释轨 140405618843920 / C 轨 1 / VM 轨 1
#       i64_at(b32, 536870912) ⇒ 0 号元素（三轨一致 · 对拍看不见）
#       f32_at(b8, 1073741823) ⇒ 越界读；f32_at(b8, 4294967296) ⇒ 0 号元素
#
# 判据（五层，**互相独立** —— 各自都能单独判红）：
#   [1] 静态：① 覆盖面台账（`selfhost/check_native_coverage.py` 自证 10 锚点 + 实跑五判据）
#            ② 修复形态在位（两处 `onnx_at_in_range` 调用 · **去注释后**无 `(int)args[1].as.i`
#               与 `i * 4 + 4` / `i * 8 + 8` 残留）
#   [2] 三轨对拍：193 例 × 3 轨 ⇒ 分叉 0
#   [3] 边界面：34 例越界下标**必须响亮**（rc≠0 或 `Err(` 开头）—— 缺陷 433 的独立判据
#   [3b] **反向判据**：合法域上界仍必须取到值 —— 只证明「越界被拒」不够，**一律拒绝**同样能让 [3] 绿
#   [4] 独立真值：20 例的值按小端 / IEEE754 **独立算出**，不靠三轨互证
#   [5] 确定性：193 例跑两遍逐字节一致（越界读的指纹正是「每次运行都不同」· M233 建立）
#   [6] 负控 A：忠实撤回修复 ⇒ [3] 必须红（B 是「做过头」方向 ⇒ [3b]/[4] 必须红）
#   [7] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（每道负控都要重编解释轨件 + 两轨驱动器）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m250_onnx_tensor
W=/tmp/m250_gate
rm -rf "$W"; mkdir -p "$W"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }

# 解释轨件：默认**入库件**（重烘后已含本轮修复）；可用 M250_PXI 覆盖（负控用开发件）
PXI="${M250_PXI:-$ROOT/bootstrap/pxi}"

echo "=== [1] 静态 · 覆盖面台账（自证 + 实跑）"
python3 "$ROOT/selfhost/check_native_coverage.py" --self-test > "$W/cov_st.log" 2>&1
chk "[1] 台账判据自证 rc=0（10 锚点）" "[ $? -eq 0 ]"
grep -E '自证：' "$W/cov_st.log" | sed 's/^/     /'
python3 "$ROOT/selfhost/check_native_coverage.py" --json "$W/cov.json" > "$W/cov.log" 2>&1
chk "[1] 覆盖面实跑 rc=0（判据 ①②③④⑤）" "[ $? -eq 0 ]"
grep -E '已覆盖|全通过' "$W/cov.log" | sed 's/^/     /'
chk "[1] 规模锚点：native 总数 ≥ 350（防期望集静默收窄）" \
    "python3 -c \"import json;d=json.load(open('$W/cov.json'));assert d['total']>=350\""
chk "[1] 划分完备：covered + uncovered == total" \
    "python3 -c \"import json;d=json.load(open('$W/cov.json'));assert len(d['covered'])+len(d['uncovered'])==d['total']\""
chk "[1] 本门自身的 native 已进面（f32_at/f32_count/i64_at/onnx_run 均在已覆盖集）" \
    "python3 -c \"import json;c=set(json.load(open('$W/cov.json'))['covered']);assert {'f32_at','f32_count','i64_at','onnx_run','onnx_op_names'}<=c\""

echo "── [1b] 修复形态在位（**去注释后**判，防「注释里写着修了」）"
python3 - "$ROOT/runtime/runtime_onnx.c" > "$W/rt_strip.txt" <<'PY'
import re, sys
s = open(sys.argv[1], encoding='utf-8').read()
s = re.sub(r'/\*.*?\*/', '', s, flags=re.S)      # 块注释
s = re.sub(r'//[^\n]*', '', s)                    # 行注释
sys.stdout.write(s)
PY
chk '[1b] 两处 onnx_at_in_range(i, len, 4/8) 在源码里' \
    "[ \"\$(grep -c 'onnx_at_in_range(i, len, 4)' $W/rt_strip.txt)\" = 1 ] && [ \"\$(grep -c 'onnx_at_in_range(i, len, 8)' $W/rt_strip.txt)\" = 1 ]"
chk "[1b] 去注释后**无** \`(int)args[1].as.i\`（缺陷 433 的①）" \
    "[ \"\$(grep -c '(int)args\[1\].as.i' $W/rt_strip.txt)\" = 0 ]"
chk "[1b] 去注释后**无** \`i * 4 + 4\` / \`i * 8 + 8\`（缺陷 433 的②）" \
    "[ \"\$(grep -cE 'i \\* [48] \\+ [48]' $W/rt_strip.txt)\" = 0 ]"
chk '[1b] 界判据是 int64 形态（int64_t idx + (int64_t)(len / elem_bytes)）' \
    "grep -q 'int64_t idx, int len, int elem_bytes' $W/rt_strip.txt && grep -q '(int64_t)(len / elem_bytes)' $W/rt_strip.txt"

echo "=== [2] 生成探针（含自证）"
python3 "$D/gen_probes.py" --out "$W" --root "$ROOT" > "$W/gen.log" 2>&1
chk "[2] 探针生成 + 自证 rc=0" "[ $? -eq 0 ]"
sed 's/^/     /' "$W/gen.log"
chk "[2] 规模下限：13 native · 位置 16 · 用例 ≥ 190" \
    "grep -qE '自证 OK：(19[0-9]|2[0-9][0-9]) 例 · 13 native · 位置 16' $W/gen.log"

build_drivers() {
    rm -rf "$W/a_vm" "$W/a_c" "$W/build"
    mkdir -p "$W/a_vm" "$W/a_c" "$W/build"
    cp "$W/drv.px" "$W/a_vm/drv.px"; cp "$W/drv.px" "$W/a_c/drv.px"
    ( cd "$W/a_vm" && timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_vm.log" 2>&1 \
        || { echo "  C/VM 轨构建日志尾："; tail -12 "$W/b_vm.log"; return 1; }
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_c.log" 2>&1 \
        || { echo "  C 轨构建日志尾："; tail -12 "$W/b_c.log"; return 1; }
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm" && cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    [ -x "$W/build/drv_vm" ] && [ -x "$W/build/drv_c" ]
}
if build_drivers > "$W/build.log" 2>&1; then chk "[3] 两轨驱动器构建" true; else
    chk "[3] 两轨驱动器构建" false; tail -12 "$W/build.log" | sed 's/^/     /'; fi

x3() {  # 三轨对拍 → stdout 落 $1；rc 交给调用方
    python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --pxi "$PXI" ${2:-} > "$1" 2>&1
    return $?
}
x3 "$W/t3.log"
chk "[4] 四层判据 rc=0（M250-THREE-TRACKS-OK）" "grep -q M250-THREE-TRACKS-OK $W/t3.log"
sed 's/^/     /' "$W/t3.log"
chk "[4] [2] 三轨对拍 193 例 · 分叉 0" "grep -q '三轨对拍：193 例 × 3 轨 · 分叉 0' $W/t3.log"
chk "[4] [3] 边界面 34 例全部响亮" "grep -q '边界面：34 例全部响亮' $W/t3.log"
chk "[4] [3b] 反向判据（合法域上界仍可取）" "grep -q '合法域上界仍可取' $W/t3.log"
chk "[4] [4] 独立真值 20 例全符" "grep -q '独立真值：20 例全符' $W/t3.log"
chk "[4] [5] 确定性 193 例两遍一致" "grep -q '确定性：193 例两遍逐字节一致' $W/t3.log"

echo "=== [5] 覆盖边界（如实登记，不判红）"
cat <<'TXT'
  · 面只含 **ONNX 张量助手族 13 个 native**（M248 首算的 21 个里，这 13 个已收进面）；
    覆盖面台账里仍有 **19 个** native 从未被任何普贤语料触碰
    （[A 类·设计边界] h3_qs_* 8 + interp_bridge = 9；[B 类·欠账] aes_*_bytes 5 +
      go_errno_string + print_err + session_destroy + session_id + tz_local = 10）
  · 台账的消费面**只扫 `.px`**（门脚本是装置、不是语料）—— 这是本门首跑**逼出来**的口径：
    原先本门的「覆盖边界」段用文字列了那 16 个名字 ⇒ 立刻被判成「已覆盖」⇒ 基线全变「过期」。
    ⇒ 约定：**语料若是运行期生成的，必须另提交一份 `.px` 冒烟**
      （本门 = `examples/m250_onnx_tensor/surface.px`，13 个 native 的合法侧各调一次）
  · 本族的**三个轨共用同一份 C 实现**（解释轨 `ibuiltin.px` 只是转发）⇒ [2] 三轨对拍对本族
    **判别力有限**；真正的牙在 [3]/[3b]/[4]（缺陷 433 正是它们照出来的）
  · `f32_bytes` / `i64_bytes` 的**元素类型**面（float vs int 混入）只覆盖到毒值级，未做全叉积
  · `onnx_*` 五个句柄函数的**合法 id** 路径未覆盖（需真实 .onnx 模型文件；M157 门已覆盖执行面）
TXT

echo "=== [6] 负控（$([ "$NEG" = 1 ] && echo '全量' || echo '--neg-skip 跳过')）"
if [ "$NEG" = 1 ]; then
    export M250_ROOT="$ROOT"
    # ⚠️ 必须用**函数**：把带参数的命令写成变量再带引号展开（NEGCTL="python3 ...")
    #    会把整串当成**一个可执行文件名** ⇒ `No such file or directory`（M250 首跑实测）。
    negctl() { python3 "$D/negctl.py" "$@"; }
    python3 "$D/negctl.py" --selftest > "$W/neg_st.log" 2>&1
    chk "[6] 负控锚点自证（3 条恰命中 1 次）" "[ $? -eq 0 ]"
    grep -E '自证：' "$W/neg_st.log" | sed 's/^/     /'
    negctl --snapshot > /dev/null

    # ── 负控 A：忠实撤回 ⇒ [3] 必须红 ──
    # ⚠️ 必须**改 shell 变量 PXI** —— 写 `M250_PXI=... x3` 是**无效**的：
    #    x3 用的是脚本开头展开好的 `$PXI`，环境变量只对子进程生效
    #    ⇒ 负控会跑**入库件**、判红消失 ⇒ **假绿**（M161/M164 记过同款，本仓第 3 次）。
    negctl --apply A > /dev/null
    bash selfhost/devbuild.sh pxi > "$W/dev_A.log" 2>&1
    build_drivers > "$W/build_A.log" 2>&1
    PXI=/tmp/pxidev
    if x3 "$W/negA.log"; then
        chk "[6] 负控 A（撤回 433 修复）：边界面必须判红" false
    else
        chk "[6] 负控 A：边界面**未响亮**（缺陷 433 复现）" \
            "grep -q '边界面：[0-9]* 例\*\*未响亮\*\*' $W/negA.log"
        grep -E '未响亮|bnd_' "$W/negA.log" | head -6 | sed 's/^/     /'
    fi
    negctl --restore > /dev/null

    # ── 负控 B：把界判据**做过头** ⇒ [3b] 反向判据 / [4] 独立真值必须红 ──
    negctl --apply B > /dev/null
    bash selfhost/devbuild.sh pxi > "$W/dev_B.log" 2>&1
    build_drivers > "$W/build_B.log" 2>&1
    PXI=/tmp/pxidev
    if x3 "$W/negB.log"; then
        chk "[6] 负控 B（界判据做过头）：必须判红" false
    else
        chk "[6] 负控 B：**反向判据**判红（合法域上界取不到值）" \
            "grep -q '合法域上界\*\*取不到值\*\*' $W/negB.log"
        chk "[6] 负控 B：[4] 独立真值也独立判红" "grep -q '独立真值：[0-9]* 例不符' $W/negB.log"
    fi
    negctl --restore > /dev/null
    bash selfhost/devbuild.sh pxi > "$W/dev_R.log" 2>&1; build_drivers > /dev/null 2>&1
    PXI="${M250_PXI:-$ROOT/bootstrap/pxi}"     # ← 回正常件再跑负控 C

    # ── 负控 C：判据自伤 ⇒ A 的红**必须消失**（证明红来自判据而非偶然）──
    x3 "$W/harm.log" "--harm"
    chk "[6] 负控 C（判据自伤 --harm）：必须**变绿**（否则红的原因不是本判据）" \
        "grep -q M250-THREE-TRACKS-OK $W/harm.log"

    # ── 源逐字节还原 ──
    chk "[6] 三处负控后源码逐字节还原" \
        "cmp -s $ROOT/runtime/runtime_onnx.c /tmp/m250_negctl_snap/runtime/runtime_onnx.c"
fi

echo "══ 汇总：$pass 通过 / $fail 失败 ══"
[ "$fail" -eq 0 ] && echo "M250-VERIFY-OK" || echo "M250-VERIFY-FAIL"
exit "$fail"
