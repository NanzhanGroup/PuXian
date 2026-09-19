#!/usr/bin/env bash
# M147 验证：定点小数文本 native `fmt_float_dec(x[, dec])`
# ---------------------------------------------------------------
# 门做四件事：
#   ① 真值对拍：corpus.txt（<dec>|v|<十进制> / <dec>|b|<16位十六进制> 两类）逐行与
#      **Go 本尊**（strconv.FormatFloat(x,'f',dec,64)）逐字节 diff
#      （VM 轨 + C 轨各跑一遍，输出必须逐字节一致）
#   ② 自断言：绝对期望值 38 条（M147_ASSERT: nP/mF）——
#      **并列取偶**（0.125→"0.12"、2.625→"2.62"、3.5/4.5→"4"）/
#      NaN·±Inf 的 Go 文本（不是 libc 的 nan/inf）/ ±0.0 的符号 /
#      dec 缺省 6 · dec 钳位 / 与 json_num_str（最短往返）不混 /
#      **缺陷 128**：一元负号的符号位（VM 轨修前丢 -0.0）
#   ③ 负控 A：篡改 Go 真值一行 ⇒ diff **必须**变红
#   ④ 负控 B：把取偶语义改成"半向上"（native 里改 snprintf 精度再自行 +0.5）⇒ 输出必须变化
#      负控 C：把 dec 参数忽略（固定 2 位）⇒ 输出必须变化
#      （negative control 是"这条面到底跑没跑"的硬判据 —— 门可以因夹具坏而**假绿**）
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m147_build.log
FAIL=0

# ── 缺陷 139（第 33 轮）：负控会改写 runtime/*.c；门**被打断**时篡改态会静默留在工作区 ──
#   （语法合法、语义反向 ⇒ 编译器不报错；而本轮 runtime.c 本来就带未提交改动 ⇒ `git diff` 判不出来。）
#   两道防线：① 开门先查「负控残留标记 NEGCTL」；② 信号兜底还原快照。
for _f in ../../runtime/runtime.c ../../runtime/vm.c; do
    if [ -f "$_f" ] && grep -q 'NEGCTL' "$_f" 2>/dev/null; then
        echo "FAIL 负控残留：$_f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
        exit 1
    fi
done

cp ../../runtime/runtime.c /tmp/m147_runtime_keep.c
cp ../../runtime/vm.c      /tmp/m147_vm_keep.c
restore_rt() { cp /tmp/m147_runtime_keep.c ../../runtime/runtime.c 2>/dev/null; cp /tmp/m147_vm_keep.c ../../runtime/vm.c 2>/dev/null; }
trap restore_rt INT TERM HUP
NTRUTH=0

mkdir -p build

echo "== [语料] 校验（corpus.txt 已入库；生成器与语料必须一致）=="
if command -v python3 >/dev/null 2>&1; then
    python3 gen_corpus.py --check || { echo "FAIL [语料] 与生成器不一致"; exit 1; }
else
    echo "   ⚠️ 本机无 python3 ⇒ 跳过生成器一致性校验（语料本身仍是本轮提交的实物）"
fi
[ -s corpus.txt ] || { echo "FAIL [语料] corpus.txt 缺失或为空"; exit 1; }

echo "== [truth] 生成 Go 真值（strconv.FormatFloat 本尊）=="
command -v go >/dev/null 2>&1 || { echo "FAIL：本机无 go 工具链（本门真值必须来自 Go）"; exit 1; }
if (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1); then
    ./build/truth corpus.txt > build/truth.txt 2> build/truth.stderr
    NTRUTH=$(wc -l < build/truth.txt)
    echo "   真值行数：$NTRUTH"
    sed 's/^/   （Go 侧统计）/ ' build/truth.stderr
else
    echo "FAIL [truth] Go 编译失败"; tail -20 "$LOG"; exit 1
fi
[ "$NTRUTH" -ge 600 ] || { echo "FAIL [truth] 真值行数过少（$NTRUTH）"; exit 1; }

# run_track <标签> <引擎（可空）> <源文件>
run_track() {
    local label="$1" engine="$2" src="$3" out="/tmp/m147_$1.out" rc=1
    echo "== [$label] 编译 + 运行 =="
    if [ -n "$engine" ]; then
        env PX_BUILD_ENGINE="$engine" "$PX" build "$src" > "$LOG" 2>&1 || { echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return; }
    else
        "$PX" build "$src" > "$LOG" 2>&1 || { echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return; }
    fi
    local bin
    bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
    [ -n "$bin" ] || { echo "FAIL [$label] 无产物"; FAIL=$((FAIL+1)); return; }
    "$bin" > "$out" 2>&1; rc=$?
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
    if grep -q '^M147_ASSERT: 38P/0F$' "$out"; then
        echo "   PASS 自断言 38P/0F（$label）"
    else
        echo "   FAIL 自断言未全绿（$label）："
        grep -E '^(FAIL |M147_ASSERT)' "$out" | head -20
        FAIL=$((FAIL+1))
    fi
}

run_track vm "" fidelity.px
run_track c c fidelity.px

echo "--- VM / C 两轨输出逐字节一致 ---"
if diff -q /tmp/m147_vm.out /tmp/m147_c.out > /dev/null 2>&1; then
    echo "PASS 两轨一致"
else
    echo "FAIL 两轨不一致："; diff /tmp/m147_vm.out /tmp/m147_c.out | head -20; FAIL=$((FAIL+1))
fi

echo "== [负控 A] 篡改 Go 真值一行 ⇒ diff 必须变红 =="
cp build/truth.txt /tmp/m147_truth_keep.txt
sed -i '3s/.*/TAMPERED|line/' build/truth.txt
if diff -q build/truth.txt /tmp/m147_vm.out.corpus > /dev/null 2>&1; then
    echo "FAIL 负控 A：篡改真值后 diff 仍绿（门没在查）"; FAIL=$((FAIL+1))
else
    echo "PASS 负控 A：篡改真值后 diff 变红"
fi
cp /tmp/m147_truth_keep.txt build/truth.txt

echo "== [负控 B] 把\"取偶\"改成\"半向上\"（native 精度 +1 再自行进位）⇒ 输出必须变化 =="
cp ../../runtime/runtime.c /tmp/m147_runtime_keep.c
python3 - <<'PY' || { echo "FAIL 负控 B：篡改未生效（脚本与源码不同步）"; FAIL=$((FAIL+1)); }
import sys
p = "../../runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = '    snprintf(buf, sizeof(buf), "%.*f", dec, d);\n    return px_str(buf);'
new = ('    snprintf(buf, sizeof(buf), "%.*f", dec, d);\n'
       '    /* NEGCTL: 半向上（非取偶）—— 只在本负控里生效 */\n'
       '    { char up[512]; int nd = dec + 1; snprintf(up, sizeof(up), "%.*f", nd, d);\n'
       '      if (dec > 0) { up[strlen(up) - 1] = \'\\0\'; }\n'
       '      snprintf(buf, sizeof(buf), "%s", up); }\n'
       '    return px_str(buf);')
if s.count(old) != 1:
    sys.exit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
if grep -q 'NEGCTL' ../../runtime/runtime.c; then
    if "$PX" build fidelity.px > "$LOG" 2>&1; then
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m147_neg_b.out 2>&1
        if diff -q /tmp/m147_vm.out /tmp/m147_neg_b.out > /dev/null 2>&1; then
            echo "FAIL 负控 B：改掉取偶语义后输出未变（该面可能没被跑到）"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 B：改掉取偶语义后输出变化"
        fi
    else
        echo "PASS 负控 B：篡改后编译失败（亦判红）"
    fi
else
    echo "FAIL 负控 B：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m147_runtime_keep.c ../../runtime/runtime.c

echo "== [负控 C] 忽略 dec 参数（固定 2 位）⇒ 输出必须变化 =="
cp fidelity.px /tmp/m147_fx_keep.px
sed -i 's/print(p\[0\] + "|" + kind + "|" + txt + "|" + fmt_float_dec(f, dec))/print(p[0] + "|" + kind + "|" + txt + "|" + fmt_float_dec(f, 2))/' fidelity.px
if grep -q 'fmt_float_dec(f, 2))' fidelity.px; then
    if "$PX" build fidelity.px > "$LOG" 2>&1; then
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m147_neg_c.out 2>&1
        if diff -q /tmp/m147_vm.out /tmp/m147_neg_c.out > /dev/null 2>&1; then
            echo "FAIL 负控 C：忽略 dec 后输出未变"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 C：忽略 dec 后输出变化"
        fi
    else
        echo "PASS 负控 C：篡改后编译失败（亦判红）"
    fi
else
    echo "FAIL 负控 C：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m147_fx_keep.px fidelity.px

echo "== [负控 D] 把 VM 轨的浮点一元负号改成 `0 - v`（缺陷 128 的语义）⇒ VM 轨断言必须变红 =="
cp ../../runtime/vm.c /tmp/m147_vmc_keep.c
python3 - <<'PY' || { echo "FAIL 负控 D：篡改未生效"; FAIL=$((FAIL+1)); }
import sys
p = "../../runtime/vm.c"
s = open(p, encoding="utf-8").read()
old = "            else if (x.type == PX_FLOAT) slots[in.a] = vm_float(-x.as.f);"
new = "            else if (x.type == PX_FLOAT) slots[in.a] = vm_float(0.0 - x.as.f);   /* NEGCTL-128 */"
if s.count(old) != 1:
    sys.exit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
if grep -q 'NEGCTL-128' ../../runtime/vm.c; then
    if "$PX" build fidelity.px > "$LOG" 2>&1; then
        bin=$(find build -type f -name "fidelity" -perm -u+x 2>/dev/null | head -1)
        "$bin" > /tmp/m147_neg_d.out 2>&1
        if diff -q /tmp/m147_vm.out /tmp/m147_neg_d.out > /dev/null 2>&1; then
            echo "FAIL 负控 D：改掉 VM 一元负号后输出未变（缺陷 128 没被断言覆盖）"; FAIL=$((FAIL+1))
        else
            echo "PASS 负控 D：改掉 VM 一元负号后输出变化（缺陷 128 已被断言钉住）"
        fi
    else
        echo "PASS 负控 D：篡改后编译失败（亦判红）"
    fi
else
    echo "FAIL 负控 D：篡改未生效"; FAIL=$((FAIL+1))
fi
cp /tmp/m147_vmc_keep.c ../../runtime/vm.c

# 还原（防篡改残留）
"$PX" build fidelity.px > "$LOG" 2>&1 || true

if [ "$FAIL" = "0" ]; then
    echo "M147-VERIFY-OK"
    exit 0
fi
echo "M147-VERIFY-FAIL（$FAIL 项）"
exit 1
