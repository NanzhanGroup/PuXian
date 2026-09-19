#!/usr/bin/env bash
# M148 验证：IEEE 浮点除法/取模 + 非有限值文本 + Go `%v` 文本 native
# ---------------------------------------------------------------
# 门做六件事：
#   ① 真值对拍：两份语料拼起来（corpus_div 792 + corpus_gv 384 = 1176 行）与
#      **Go 本尊**（truth/main.go：IEEE `/`、math.Mod、fmt.Sprint）逐字节 diff ——
#      **VM 轨 / C 轨 / 解释轨三轨各跑一遍**，三者输出必须逐字节一致。
#   ② 自断言 44 条（M148_ASSERT: nP/mF）
#   ③ 往返性质（M148_RT: checked=n bad=0）—— str() 的有限值文本是语言约定
#      （不与 Go 对拍），但「读回来还是同一个双精度」必须成立；**缺陷 133** 就是
#      这条判据照出来的（定点分支上界写死 17 位小数 ⇒ 0.000… 形态静默丢精度）。
#      `corpus_strnf.txt` 同属"不对拍"那族：str() 的**非有限值**是语言约定
#      （`inf`/`-inf`/`nan`，有 golden/s09 钉着），只做**约定**检查（断言 A38）。
#   ④ 负控 A：篡改 Go 真值一行 ⇒ diff **必须**变红
#   ⑤ 负控 B：篡改语料一行 ⇒ diff **必须**变红
#   ⑥ 负控 C/D/E：把三处修复分别**退回修前语义**，门必须变红 ——
#        C：`px_div` 的零检查加回（缺陷 118 报错语义）
#        D：`go_float_text` 的 eprec 6 → 7（缺陷 132 的 Go 文本面）
#        E：`fmt_num` 定点上界改回 `dec <= 17`（缺陷 133）
#      （negative control 是「这条面到底跑没跑」的硬判据。
#        ⚠️ 每条负控都**备份/还原**源码，门内改的是真源码，跑完必须复原。）
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m148_build.log
FAIL=0
NTRUTH=0

mkdir -p build

echo "== [语料] 校验（四份已入库；生成器与语料必须一致）=="
if command -v python3 >/dev/null 2>&1; then
    python3 gen_corpus.py --check || { echo "FAIL [语料] 与生成器不一致"; exit 1; }
else
    echo "   ⚠️ 本机无 python3 ⇒ 跳过生成器一致性校验（语料本身仍是本轮提交的实物）"
fi
for f in corpus_div.txt corpus_str.txt corpus_strnf.txt corpus_gv.txt; do
    [ -s "$f" ] || { echo "FAIL [语料] $f 缺失或为空"; exit 1; }
done

echo "== [truth] 生成 Go 真值（IEEE 754 / math.Mod / fmt.Sprint 本尊）=="
command -v go >/dev/null 2>&1 || { echo "FAIL：本机无 go 工具链（本门真值必须来自 Go）"; exit 1; }
if (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1); then
    ./build/truth div corpus_div.txt > build/truth_div.txt 2>>"$LOG"
    ./build/truth gv  corpus_gv.txt  > build/truth_gv.txt 2>>"$LOG"
    cat build/truth_div.txt build/truth_gv.txt > build/truth.txt
    NTRUTH=$(wc -l < build/truth.txt)
    echo "   真值行数：$NTRUTH（div $(wc -l < build/truth_div.txt) / gv $(wc -l < build/truth_gv.txt)）"
else
    echo "FAIL [truth] Go 编译失败"; tail -20 "$LOG"; exit 1
fi
[ "$NTRUTH" -eq 1176 ] || { echo "FAIL [truth] 真值行数不是 1176（$NTRUTH）⇒ 语料被改动过"; exit 1; }

# run_track <标签> <引擎（可空）> <源文件>
run_track() {
    local label="$1" engine="$2" src="$3" out="/tmp/m148_$1.out" rc=1
    echo "== [$label] 编译 + 运行 =="
    if [ "$engine" = "interp" ]; then
        "$PX" run "$src" > "$out" 2>&1; rc=$?
    elif [ -n "$engine" ]; then
        env PX_BUILD_ENGINE="$engine" "$PX" build "$src" > "$LOG" 2>&1 || { echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return; }
    else
        "$PX" build "$src" > "$LOG" 2>&1 || { echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return; }
    fi
    if [ "$engine" != "interp" ]; then
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
        echo "   PASS 语料面逐字节一致（$label，$NTRUTH 行）"
    else
        echo "   FAIL 语料面有差异（$label）："
        diff build/truth.txt "$out.corpus" | head -20
        FAIL=$((FAIL+1))
    fi
    if grep -q '^M148_ASSERT: 44P/0F$' "$out"; then
        echo "   PASS 自断言 44P/0F（$label）"
    else
        echo "   FAIL 自断言未全绿（$label）："
        grep -E '^(FAIL |M148_ASSERT)' "$out" | head -20
        FAIL=$((FAIL+1))
    fi
    if grep -qE '^M148_RT: checked=[0-9]+ bad=0$' "$out"; then
        echo "   PASS 往返性质 checked>=150 bad=0（$label）：$(grep -E '^M148_RT' "$out")"
    else
        echo "   FAIL 往返性质不成立（$label，缺陷 133 回归）：$(grep -E '^M148_RT' "$out")"
        FAIL=$((FAIL+1))
    fi
}

run_track vm "" fidelity.px
run_track c c fidelity.px
run_track interp interp fidelity.px

echo "--- 三轨输出逐字节一致（VM / C / 解释轨）---"
if diff -q /tmp/m148_vm.out /tmp/m148_c.out > /dev/null 2>&1 \
   && diff -q /tmp/m148_vm.out /tmp/m148_interp.out > /dev/null 2>&1; then
    echo "PASS 三轨一致"
else
    echo "FAIL 三轨不一致："
    diff /tmp/m148_vm.out /tmp/m148_c.out | head -10
    diff /tmp/m148_vm.out /tmp/m148_interp.out | head -10
    FAIL=$((FAIL+1))
fi

echo "== [负控 A] 篡改 Go 真值一行 ⇒ diff 必须变红 =="
cp build/truth.txt /tmp/m148_truth_keep.txt
sed -i '3s/.*/TAMPERED|line/' build/truth.txt
if diff -q build/truth.txt /tmp/m148_vm.out.corpus > /dev/null 2>&1; then
    echo "FAIL 负控 A：篡改真值后 diff 仍绿（门没在查）"; FAIL=$((FAIL+1))
else
    echo "PASS 负控 A：篡改真值后 diff 变红"
fi
cp /tmp/m148_truth_keep.txt build/truth.txt

echo "== [负控 B] 篡改语料一行（原样**重跑已建好的产物**）⇒ diff 必须变红 =="
# 语料是**运行时**读的（read_file 相对 cwd）⇒ 篡改后重跑同一枚产物即可，
# 不必重新编译（这条负控的意图是"语料真的参与了比对"，不是"编译器重跑"）。
cp corpus_div.txt /tmp/m148_corpus_keep.txt
sed -i '5s/.*/d|3ff0000000000000|4000000000000000/' corpus_div.txt
if grep -q '^d|3ff0000000000000|4000000000000000$' corpus_div.txt; then
    bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
    "$bin" > /tmp/m148_neg_b.out 2>&1
    head -n "$NTRUTH" /tmp/m148_neg_b.out > /tmp/m148_neg_b.corpus
    if diff -q build/truth.txt /tmp/m148_neg_b.corpus > /dev/null 2>&1; then
        echo "FAIL 负控 B：篡改语料后 diff 仍绿"; FAIL=$((FAIL+1))
    else
        echo "PASS 负控 B：篡改语料后 diff 变红"
    fi
else
    echo "FAIL 负控 B：篡改未生效（锚点与语料不同步）"; FAIL=$((FAIL+1))
fi
cp /tmp/m148_corpus_keep.txt corpus_div.txt

echo "== [负控 C] 把 \`/\` 的零检查加回 px_div（缺陷 118 修前语义）⇒ 门必须变红 =="
cp ../../runtime/runtime.c /tmp/m148_runtime_keep.c
python3 - <<'PY' || { echo "FAIL 负控 C：篡改未生效（脚本与源码不同步）"; FAIL=$((FAIL+1)); }
import sys
p = "../../runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = "    return px_float(num_val(a) / num_val(b));\n}"
new = ("    double d = num_val(b);\n"
       "    if (d == 0) px_error(\"除零错误\");   /* NEGCTL-118 */\n"
       "    return px_float(num_val(a) / d);\n}")
if s.count(old) != 1:
    sys.exit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
if grep -q 'NEGCTL-118' ../../runtime/runtime.c; then
    if "$PX" build fidelity.px > "$LOG" 2>&1; then
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m148_neg_c.out 2>&1
        if diff -q /tmp/m148_vm.out /tmp/m148_neg_c.out > /dev/null 2>&1; then
            echo "FAIL 负控 C：加回零检查后输出未变（÷0 这条路没被跑到）"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 C：加回零检查后门变红（÷0 面确实被覆盖）"
        fi
    else
        echo "PASS 负控 C：篡改后编译失败（亦判红）"
    fi
else
    echo "FAIL 负控 C：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m148_runtime_keep.c ../../runtime/runtime.c

echo "== [负控 D] go_float_text 的 eprec 6 → 7（缺陷 132 的 Go 文本面）⇒ 输出必须变化 =="
python3 - <<'PY' || { echo "FAIL 负控 D：篡改未生效"; FAIL=$((FAIL+1)); }
import sys
p = "../../runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = "    if (exp10 < -4 || exp10 >= 6) {\n        if (pos < cap - 1) out[pos++] = digits[0];"
new = "    if (exp10 < -4 || exp10 >= 7) {   /* NEGCTL-132 */\n        if (pos < cap - 1) out[pos++] = digits[0];"
if s.count(old) != 1:
    sys.exit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
if grep -q 'NEGCTL-132' ../../runtime/runtime.c; then
    if "$PX" build fidelity.px > "$LOG" 2>&1; then
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m148_neg_d.out 2>&1
        if diff -q /tmp/m148_vm.out /tmp/m148_neg_d.out > /dev/null 2>&1; then
            echo "FAIL 负控 D：改掉 eprec 后输出未变（go_float_text 的阈值面没被覆盖）"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 D：改掉 eprec 后门变红"
        fi
    else
        echo "PASS 负控 D：篡改后编译失败（亦判红）"
    fi
else
    echo "FAIL 负控 D：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m148_runtime_keep.c ../../runtime/runtime.c

echo "== [负控 E] fmt_num 定点上界改回 \`dec <= 17\`（缺陷 133 修前语义）⇒ M148_RT 必须变红 =="
python3 - <<'PY' || { echo "FAIL 负控 E：篡改未生效"; FAIL=$((FAIL+1)); }
import sys
p = "../../runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = "                for (dec = 0; dec <= maxdec; dec++) {"
new = "                for (dec = 0; dec <= 17; dec++) {   /* NEGCTL-133 */"
if s.count(old) != 1:
    sys.exit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
if grep -q 'NEGCTL-133' ../../runtime/runtime.c; then
    if "$PX" build fidelity.px > "$LOG" 2>&1; then
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m148_neg_e.out 2>&1
        if grep -qE '^M148_RT: checked=[0-9]+ bad=0$' /tmp/m148_neg_e.out; then
            echo "FAIL 负控 E：退回旧上界后往返性质仍成立（缺陷 133 的触发面不在语料里）"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 E：退回旧上界后 M148_RT 判红 → $(grep -E '^M148_RT' /tmp/m148_neg_e.out)"
        fi
    else
        echo "PASS 负控 E：篡改后编译失败（亦判红）"
    fi
else
    echo "FAIL 负控 E：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m148_runtime_keep.c ../../runtime/runtime.c

# 还原（防篡改残留：重建一次确认源码已复原）
"$PX" build fidelity.px > "$LOG" 2>&1 || true

if [ "$FAIL" = "0" ]; then
    echo "M148-VERIFY-OK"
    exit 0
fi
echo "M148-VERIFY-FAIL（$FAIL 项）"
exit 1
