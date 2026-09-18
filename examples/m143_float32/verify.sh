#!/usr/bin/env bash
# M143 验证：float32 值族（float32 / float32_bits / bits_to_float32）+ Go encoding/json 的
#           32 位浮点文本（json_num_str）+ append_file_opt（缺陷 115）
#
# 门做四件事：
#   ① 真值对拍：corpus.txt（v/b/m/d 四类，含 1536 个伪随机 float32 位模式）逐行与
#      **Go 本尊**（math.Float32bits / Float32frombits / encoding/json）逐字节 diff
#      （VM 轨 + C 轨各跑一遍，断言集相同）
#   ② 自断言：绝对期望值 37 条（M143_ASSERT: 37P/0F）—— 窄化是否真发生 / 位模式往返 /
#      Go JSON 六个口岸值 / 非有限值 → null / float32 溢出 → ±Inf / 「每步 f32 收口」实证 /
#      append_file_opt 的 Result 通道与「失败不杀进程」
#   ③ 负控 A：篡改 Go 真值一行 ⇒ diff **必须**变红
#   ④ 负控 B：32 位文本换成 64 位（json_num_str(x,32)→(x,64)）⇒ 输出**必须**变化
#      负控 C：位模式面改坏（float32_bits(x)→float32_bits(x/2.0)）⇒ 输出**必须**变化
#      （两条都是"这条面到底跑没跑"的硬判据；m142 的教训：门可以因为夹具坏而**假绿**）
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m143_build.log
FAIL=0
NTRUTH=0

mkdir -p build

echo "== [语料] 校验（corpus.txt 已入库；生成器与语料必须一致 —— 防止改了生成器忘重建）=="
if command -v python3 >/dev/null 2>&1; then
    python3 gen_corpus.py --check || { echo "FAIL [语料] 与生成器不一致"; exit 1; }
else
    echo "   ⚠️ 本机无 python3 ⇒ 跳过生成器一致性校验（语料本身仍是本轮提交的实物）"
fi
[ -s corpus.txt ] || { echo "FAIL [语料] corpus.txt 缺失或为空"; exit 1; }

echo "== [truth] 生成 Go 真值（math.Float32bits / encoding/json 本尊）=="
command -v go >/dev/null 2>&1 || { echo "FAIL：本机无 go 工具链（本门真值必须来自 Go）"; exit 1; }
if (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1); then
    ./build/truth corpus.txt > build/truth.txt 2> build/truth.stderr
    NTRUTH=$(wc -l < build/truth.txt)
    echo "   真值行数：$NTRUTH"
    sed 's/^/   （Go 侧统计）/ ' build/truth.stderr
else
    echo "FAIL [truth] Go 编译失败"; tail -20 "$LOG"; exit 1
fi

# run_src <px源> → 编译并回显其输出（前 NTRUTH 行）；产物路径按 `px build` 的
#   「<源目录>/build/<名>」规则**递归查找**（源在 build/bad/ 下 ⇒ 产物在 build/bad/build/ 下）
run_src() {
    local src="$1" name bin
    name=$(basename "$src" .px)
    if ! env "$PX" build "$src" > "$LOG" 2>&1; then echo "<编译失败>"; return; fi
    bin=$(find build -type f -name "$name" -perm -u+x 2>/dev/null | head -1)
    [ -n "$bin" ] || { echo "<无产物>"; return; }
    "$bin" 2>&1 | head -n "$NTRUTH"
}

run_track() {   # $1=标签 $2=引擎（可空） $3=源文件
    local label="$1" engine="$2" src="$3" out rc
    echo "== [$label] 编译 + 运行 =="
    local -a cmd=(env)
    [ -n "$engine" ] && cmd+=("PX_BUILD_ENGINE=$engine")
    if ! "${cmd[@]}" "$PX" build "$src" > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    out=$(./build/fidelity 2>&1); rc=$?
    echo "$out" | head -n "$NTRUTH" > "build/$label.px.txt"
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; echo "$out" | tail -5; FAIL=$((FAIL+1)); return; }
    echo "$out" | grep -q "M143_ASSERT: 37P/0F" || {
        echo "FAIL [$label] 断言计数不符（期望 37P/0F）："; echo "$out" | grep "M143_ASSERT"; FAIL=$((FAIL+1)); return; }
    if diff -u build/truth.txt "build/$label.px.txt" > "build/$label.diff" 2>&1; then
        echo "PASS [$label] 与 Go 真值逐字节一致（$NTRUTH 行）"
    else
        echo "FAIL [$label] 与 Go 真值不一致："; head -30 "build/$label.diff"; FAIL=$((FAIL+1)); return
    fi
    echo "PASS [$label] 自断言 37P/0F + 退出码 0"
}

run_track "VM轨" "" fidelity.px
run_track "C轨" "c" fidelity.px

# ── ③ 负控 A：真值被改坏 ⇒ diff 必须变红 ──
echo "== [负控A] 篡改 Go 真值一行 ⇒ diff 必须变红 =="
cp build/truth.txt build/truth_bad.txt
sed -i '5s/.*/v|4|TAMPERED/' build/truth_bad.txt
if cmp -s build/truth_bad.txt build/truth.txt; then
    echo "FAIL [负控A] sed 未命中（真值没被改动）"; FAIL=$((FAIL+1))
elif diff -q build/truth_bad.txt build/VM轨.px.txt >/dev/null 2>&1; then
    echo "FAIL [负控A] 篡改真值后 diff 仍绿 —— 门在自欺"; FAIL=$((FAIL+1))
else
    echo "PASS [负控A] 篡改真值后 diff 变红（门确实在比对）"
fi

# ── ④ 负控 B：32 位文本换成 64 位 ⇒ 输出必须变化 ──
echo "== [负控B] json_num_str(x,32) → (x,64) ⇒ 输出必须变化 =="
mkdir -p build/bad
sed 's/json_num_str(x, 32)/json_num_str(x, 64)/' fidelity.px > build/bad/_bad32.px
src_bad=$(md5sum build/bad/_bad32.px | cut -d' ' -f1)
src_ok=$(md5sum fidelity.px | cut -d' ' -f1)
if [ "$src_bad" = "$src_ok" ]; then
    echo "FAIL [负控B] sed 未命中（源码未变化）"; FAIL=$((FAIL+1))
else
    bad=$(run_src build/bad/_bad32.px)
    if [ "$bad" = "$(cat build/truth.txt)" ]; then
        echo "FAIL [负控B] 换成 64 位文本后输出**没变** —— 32 位面根本没被覆盖"; FAIL=$((FAIL+1))
    elif [ "$bad" = "<编译失败>" ] || [ "$bad" = "<无产物>" ]; then
        echo "FAIL [负控B] 负控编译失败/无产物：$bad"; FAIL=$((FAIL+1))
    else
        echo "PASS [负控B] 换成 64 位文本后输出变化（32 位面确实在跑）"
    fi
fi

# ── ④b 负控 C：位模式面改坏 ⇒ 输出必须变化 ──
echo "== [负控C] float32_bits(x) → float32_bits(x / 2.0) ⇒ 输出必须变化 =="
sed 's/hex8(float32_bits(x))/hex8(float32_bits(x \/ 2.0))/' fidelity.px > build/bad/_badbits.px
if [ "$(md5sum build/bad/_badbits.px | cut -d' ' -f1)" = "$(md5sum fidelity.px | cut -d' ' -f1)" ]; then
    echo "FAIL [负控C] sed 未命中（源码未变化）"; FAIL=$((FAIL+1))
else
    badb=$(run_src build/bad/_badbits.px)
    if [ "$badb" = "$(cat build/truth.txt)" ]; then
        echo "FAIL [负控C] 改坏位模式面后输出**没变** —— 位模式面没被覆盖"; FAIL=$((FAIL+1))
    elif [ "$badb" = "<编译失败>" ] || [ "$badb" = "<无产物>" ]; then
        echo "FAIL [负控C] 负控编译失败/无产物：$badb"; FAIL=$((FAIL+1))
    else
        echo "PASS [负控C] 改坏位模式面后输出变化（位模式面确实在跑）"
    fi
fi

echo ""
if [ "$FAIL" = "0" ]; then
    echo "M143-VERIFY-OK（VM 轨 + C 轨：真值逐字节 + 37 自断言 + 3 道负控）"
    exit 0
fi
echo "M143-VERIFY-FAIL（$FAIL 项）"
exit 1
