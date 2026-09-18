#!/usr/bin/env bash
# M146 验证：float64 位模式族（float64_bits / bits_to_float64）
# ---------------------------------------------------------------
# 门做四件事：
#   ① 真值对拍：corpus.txt（v 十进制 / i 整数 / b 位模式 三类，含 512 个伪随机 64 位模式）
#      逐行与 **Go 本尊**（math.Float64bits / Float64frombits / encoding/json）逐字节 diff
#      （VM 轨 + C 轨各跑一遍，输出必须逐字节一致）
#   ② 自断言：绝对期望值 54 条（M146_ASSERT: nP/mF）—— 位模式往返零失败 /
#      ±0 的符号位可读 / ±Inf / NaN（含"NaN != 自身"这条判据）/ 非规格化数 /
#      ≥2^63 以补码负值承载 / json_num_str 的 64 位文本口径 / 与 float32 族不混淆
#   ③ 负控 A：篡改 Go 真值一行 ⇒ diff **必须**变红
#   ④ 负控 B：把位模式面换成 float32 的（float64_bits → float32_bits）⇒ 输出**必须**变化
#      负控 C：把 json 文本位宽换成 32 ⇒ 输出**必须**变化
#      （negative control 是"这条面到底跑没跑"的硬判据 —— 门可以因为夹具坏而**假绿**）
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m146_build.log
FAIL=0
NTRUTH=0

mkdir -p build

echo "== [语料] 校验（corpus.txt 已入库；生成器与语料必须一致）=="
if command -v python3 >/dev/null 2>&1; then
    python3 gen_corpus.py --check || { echo "FAIL [语料] 与生成器不一致"; exit 1; }
else
    echo "   ⚠️ 本机无 python3 ⇒ 跳过生成器一致性校验（语料本身仍是本轮提交的实物）"
fi
[ -s corpus.txt ] || { echo "FAIL [语料] corpus.txt 缺失或为空"; exit 1; }

echo "== [truth] 生成 Go 真值（math.Float64bits / encoding/json 本尊）=="
command -v go >/dev/null 2>&1 || { echo "FAIL：本机无 go 工具链（本门真值必须来自 Go）"; exit 1; }
if (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1); then
    ./build/truth corpus.txt > build/truth.txt 2> build/truth.stderr
    NTRUTH=$(wc -l < build/truth.txt)
    echo "   真值行数：$NTRUTH"
    sed 's/^/   （Go 侧统计）/ ' build/truth.stderr
else
    echo "FAIL [truth] Go 编译失败"; tail -20 "$LOG"; exit 1
fi
[ "$NTRUTH" -ge 500 ] || { echo "FAIL [truth] 真值行数过少（$NTRUTH）"; exit 1; }

# run_track <标签> <引擎（可空）> <源文件> → 把输出写 /tmp/m146_<标签>.out
run_track() {
    local label="$1" engine="$2" src="$3" out="/tmp/m146_$1.out" rc=1
    echo "== [$label] 编译 + 运行 =="
    if [ -n "$engine" ]; then
        env PX_BUILD_ENGINE="$engine" "$PX" build "$src" > "$LOG" 2>&1 || { echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return; }
        local bin
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        [ -n "$bin" ] || { echo "FAIL [$label] 无产物"; FAIL=$((FAIL+1)); return; }
        "$bin" > "$out" 2>&1; rc=$?
    else
        "$PX" build "$src" > "$LOG" 2>&1 || { echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return; }
        local bin
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        [ -n "$bin" ] || { echo "FAIL [$label] 无产物"; FAIL=$((FAIL+1)); return; }
        "$bin" > "$out" 2>&1; rc=$?
    fi
    if [ "$rc" != "0" ]; then
        echo "FAIL [$label] 运行失败 rc=$rc"; tail -10 "$out"; FAIL=$((FAIL+1)); return
    fi
    local n
    n=$(wc -l < "$out")
    echo "   输出行数：$n（真值 $NTRUTH + 断言区）"
    if [ "$n" -lt "$NTRUTH" ]; then
        echo "FAIL [$label] 输出行数少于真值（$n < $NTRUTH）"; FAIL=$((FAIL+1)); return
    fi
    head -n "$NTRUTH" "$out" > "$out.corpus"
    if diff -q build/truth.txt "$out.corpus" > /dev/null 2>&1; then
        echo "   PASS 语料面逐字节一致（$label）"
    else
        echo "   FAIL 语料面有差异（$label）："
        diff build/truth.txt "$out.corpus" | head -20
        FAIL=$((FAIL+1))
    fi
    if grep -q '^M146_ASSERT: 54P/0F$' "$out"; then
        echo "   PASS 自断言 54P/0F（$label）"
    else
        echo "   FAIL 自断言未全绿（$label）："
        grep -E '^(FAIL |M146_ASSERT)' "$out" | head -20
        FAIL=$((FAIL+1))
    fi
}

run_track vm "" fidelity.px
run_track c c fidelity.px

echo "--- VM / C 两轨输出逐字节一致 ---"
if diff -q /tmp/m146_vm.out /tmp/m146_c.out > /dev/null 2>&1; then
    echo "PASS 两轨一致"
else
    echo "FAIL 两轨不一致："; diff /tmp/m146_vm.out /tmp/m146_c.out | head -20; FAIL=$((FAIL+1))
fi

echo "== [负控 A] 篡改 Go 真值一行 ⇒ diff 必须变红 =="
cp build/truth.txt /tmp/m146_truth_keep.txt
sed -i '3s/.*/TAMPERED|line/' build/truth.txt
if diff -q build/truth.txt /tmp/m146_vm.out.corpus > /dev/null 2>&1; then
    echo "FAIL 负控 A：篡改真值后 diff 仍绿（门没在查）"; FAIL=$((FAIL+1))
else
    echo "PASS 负控 A：篡改真值后 diff 变红"
fi
cp /tmp/m146_truth_keep.txt build/truth.txt

echo "== [负控 B] 位模式面换成 float32 的 ⇒ 输出必须变化 =="
cp fidelity.px /tmp/m146_fx_keep.px
sed -i 's/var bits = float64_bits(f)/var bits = float32_bits(f)/' fidelity.px
if grep -q 'var bits = float32_bits(f)' fidelity.px; then
    "$PX" build fidelity.px > "$LOG" 2>&1 && {
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m146_neg_b.out 2>&1
        if diff -q /tmp/m146_vm.out /tmp/m146_neg_b.out > /dev/null 2>&1; then
            echo "FAIL 负控 B：换掉位模式面后输出未变（该面可能没被跑到）"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 B：换掉位模式面后输出变化"
        fi
    } || { echo "FAIL 负控 B：篡改后编译失败"; FAIL=$((FAIL+1)); }
else
    echo "FAIL 负控 B：篡改未生效（脚本与源码不同步）"; FAIL=$((FAIL+1))
fi
cp /tmp/m146_fx_keep.px fidelity.px

echo "== [负控 C] json 文本位宽换成 32 ⇒ 输出必须变化 =="
sed -i 's/json_num_str(f, 64)/json_num_str(f, 32)/' fidelity.px
if grep -q 'json_num_str(f, 32)' fidelity.px; then
    "$PX" build fidelity.px > "$LOG" 2>&1 && {
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m146_neg_c.out 2>&1
        if diff -q /tmp/m146_vm.out /tmp/m146_neg_c.out > /dev/null 2>&1; then
            echo "FAIL 负控 C：换掉 JSON 位宽后输出未变"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 C：换掉 JSON 位宽后输出变化"
        fi
    } || { echo "FAIL 负控 C：篡改后编译失败"; FAIL=$((FAIL+1)); }
else
    echo "FAIL 负控 C：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m146_fx_keep.px fidelity.px

# 还原（防篡改残留）
"$PX" build fidelity.px > "$LOG" 2>&1 || true

if [ "$FAIL" = "0" ]; then
    echo "M146-VERIFY-OK"
    exit 0
fi
echo "M146-VERIFY-FAIL（$FAIL 项）"
exit 1
