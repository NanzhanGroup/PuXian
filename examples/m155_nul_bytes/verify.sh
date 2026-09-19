#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M155 门：**含内嵌 NUL 的字符串**（第 37 轮 · 缺陷 148/150/151）
# ---------------------------------------------------------------
# 缺陷 148：`"\u{0}"` 字面量在三轨被静默丢弃（`len("\u{0}") == 0`）
#   —— 两层成因：① 发射层「NUL 丢弃」+ `px_str()` 按 C 串（strlen）重造；
#      ② 编译器**自身源码**里也有 `"\u{0}"` 字面量（旧编译器编出的是空串）⇒ 三轨
#        「一致地错」`_out.append("")` / `c == ""`。
# 缺陷 150：`print` / `print_err` / `px_fmt_value` 容器渲染按 strlen 截断
#   （实测 `print("A\0B")` 只写 `A`；Go 的 `fmt.Sprint` 写 3 字节）。
# 缺陷 151：`$BUILD` 里的**陈旧 runtime.{c,h} 副本**遮蔽 `-I` 解析（编译期用了旧头）。
#
# 门做六件事：
#   ① 语义：`nul_sem.px` 在**三轨**（VM 默认轨 / C 轨 / 解释轨）逐字节一致，且
#      与 `golden_sem.stdout` 一致、`FAIL` 行数为 0（真值含 Go 本尊实测的
#      sha256 / base64 / JSON 文本 —— 见语料头部注释）。
#   ② 字节精确**输出**：`nul_io.px` 的 stdout 原始字节与 `write_file` 落盘字节，
#      都必须与「Go 语义期望块」逐字节相同（`cmp`，不是人眼看）。
#   ③ 发射形状：含 NUL 的常量必须发射成 `PX_STR_LIT("\000"…)`（八进制恒 3 位），
#      不含 NUL 的常量必须仍是 `px_str("…")` —— 即「修复是**增量**」。
#   ④ 词法/语法面：`px lex` 的 token 文本仍是 `"\0"`、`px parse` 的 AST 值仍是 `"\0"`
#      （这两层**本来就没坏**，是本门的「别把好的改坏」判据）。
#   ⑤ 自举安全不变式：`selfhost/{codegen,pxlexer,it_util}.px` 的**代码**（非注释）里
#      不得再出现 `"\u{0}"` 字面量（必须用运行时构造 ⇒ 旧编译器编新源码也正确）。
#   ⑥ 负控 4 道（必须全红）：
#        A：`PX_STR_LIT` 的长度退回 `strlen` ⇒ C 轨常量截断，**语义**门红
#        B：`px_print_value` 退回 `printf("%s")` ⇒ **输出**门红
#        C：`PX_STR_LIT` 长度多算 1（`sizeof(lit)`，含结尾 NUL）⇒ **语义**门红
#        D：`vm_loadk` 忽略 K 项显式长度（旧口径）⇒ VM 轨**语义**门红
# 口径（与 m153/m154 门同）：
#   · 负控改 `runtime/`（本门另加 `runtime.h`/`vm.h` 快照）：开门先查 NEGCTL 残留、
#     开头快照、`trap` + 末尾**逐字节还原**并复跑全绿；本门持 flock（并发跑同一个门
#     = 假 diff + 权限错误，第 29 轮实锤）。
# 用法：bash examples/m155_nul_bytes/verify.sh
# 退出码：0 = M155-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
exec 9>/tmp/m155_gate.lock
if ! flock -n 9; then echo "M155-VERIFY-SKIP（已有同类门在跑）"; exit 1; fi

PX=../../tools/px
FAIL=0
mkdir -p build
say() { echo "$@"; }
chk() { # chk <描述> <实际> <期望>
  if [ "$2" = "$3" ]; then echo "  ✅ $1"; else echo "  ❌ $1 实际=[$2] 期望=[$3]"; FAIL=$((FAIL+1)); fi
}
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

# ── 快照（负控会改这些文件）+ 残留检查 ──
SNAP=/tmp/m155_snap
mkdir -p "$SNAP"
for f in runtime.c runtime.h vm.h vm.c; do cp "../../runtime/$f" "$SNAP/$f"; done
for f in runtime.c runtime.h vm.h vm.c; do
    if grep -q 'NEGCTL' "../../runtime/$f" 2>/dev/null; then
        echo "FAIL 负控残留：../../runtime/$f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"; exit 1
    fi
done
restore_rt() {
    for f in runtime.c runtime.h vm.h vm.c; do cp -f "$SNAP/$f" "../../runtime/$f" 2>/dev/null || true; done
}
trap restore_rt INT TERM HUP

# ── 期望块（Go 本尊语义）┐
# Go 参考：fmt.Println("L\x00R") 写 4c 00 52 0a；fmt.Println("a\x00b\x00") 写 61 00 62 00 0a；
#          fmt.Println("A\x00B") 写 41 00 42 0a；fmt.Println([]string{"A\x00B"}) 写 5b 41 00 42 5d 0a；
#          os.WriteFile(p, "A\x00B\x00x\x00") 写 41 00 42 00 78 00
printf 'L\000R\n' > build/exp_io_1.bin
printf 'a\000b\000\n' > build/exp_io_2.bin
printf 'A\000B\n' > build/exp_io_3.bin
printf '[A\000B]\n' > build/exp_io_4.bin
cat build/exp_io_1.bin build/exp_io_2.bin build/exp_io_3.bin build/exp_io_4.bin > build/exp_io_stdout.bin
printf 'M155-IO-DONE\n' >> build/exp_io_stdout.bin
printf 'A\000B\000x\000' > build/exp_transcript.bin

say "── ①+② 三轨构建与对拍 ——"
for track in vm c interp; do
    case "$track" in
      vm)     ENGINE="--vm" ; RUN="" ;;
      c)      ENGINE="--c"  ; RUN="" ;;
      interp) ENGINE=""     ; RUN="interp" ;;
    esac
    if [ "$track" = "interp" ]; then
        rm -f nul_bytes_out.bin
        "$PX" run nul_sem.px  > "build/sem.$track.out" 2> "build/sem.$track.err" || bad "$track: nul_sem 运行失败"
        "$PX" run nul_io.px   > "build/io.$track.out"  2> "build/io.$track.err"  || bad "$track: nul_io 运行失败"
    else
        rm -f build/nul_sem build/nul_io build/nul_bytes_out.bin
        "$PX" build $ENGINE nul_sem.px > "build/build_sem.$track.log" 2>&1 || bad "$track: nul_sem 编译失败"
        "$PX" build $ENGINE nul_io.px  > "build/build_io.$track.log"  2>&1 || bad "$track: nul_io 编译失败"
        ./build/nul_sem > "build/sem.$track.out" 2> "build/sem.$track.err" || bad "$track: nul_sem 退出码非 0"
        rm -f nul_bytes_out.bin
        ./build/nul_io  > "build/io.$track.out"  2> "build/io.$track.err"  || bad "$track: nul_io 退出码非 0"
        cp -f nul_bytes_out.bin "build/transcript.$track.bin" 2>/dev/null || bad "$track: 未生成 transcript"
        rm -f nul_bytes_out.bin
    fi
    # 语义轨：与 golden 逐字节 + FAIL 计数为 0
    if cmp -s "build/sem.$track.out" golden_sem.stdout; then chk "$track 语义 vs golden（逐字节）" same same
    else bad "$track: 语义输出与 golden 不一致（diff 见 build/sem.$track.out）"; fi
    nf=$(grep -c '^FAIL ' "build/sem.$track.out" || true)
    chk "$track 语义 FAIL 行数" "$nf" "0"
    # 输出轨：stdout 原始字节 + transcript 文件字节
    if cmp -s "build/io.$track.out" build/exp_io_stdout.bin; then chk "$track stdout 原始字节（含 NUL）" same same
    else bad "$track: stdout 字节不符（期望 exp_io_stdout.bin，实际 build/io.$track.out）"; fi
done
# 解释轨的 transcript：`px run` 的 cwd 是示例目录，文件就落在示例目录
if [ -f nul_bytes_out.bin ]; then mv -f nul_bytes_out.bin build/transcript.interp.bin; fi
for track in vm c interp; do
    if cmp -s "build/transcript.$track.bin" build/exp_transcript.bin; then chk "$track 落盘字节（write_file 含 NUL）" same same
    else bad "$track: 落盘字节不符（build/transcript.$track.bin）"; fi
done
# 三轨互相逐字节一致（语义 + 输出）
for t in c interp; do
    if cmp -s build/sem.vm.out "build/sem.$t.out" && cmp -s build/io.vm.out "build/io.$t.out"; then
        chk "三轨互拍 vm vs $t" same same
    else bad "三轨不一致：vm vs $t"; fi
done

say "── ③ 发射形状（修复必须是增量）——"
"$PX" build --c nul_sem.px > build/build_emit.log 2>&1 || bad "发射形状：构建失败"
emitc=build/nul_sem.c
if grep -q 'PX_STR_LIT("\\0001")' "$emitc" && grep -q 'PX_STR_LIT("a\\000b")' "$emitc"; then
    chk "含 NUL 常量 = PX_STR_LIT 且八进制恒 3 位" yes yes
else bad "含 NUL 常量未按 PX_STR_LIT(\\000 三位) 发射"; fi
if grep -q 'px_str("A")' "$emitc"; then chk "不含 NUL 的常量仍是 px_str" yes yes
else bad "不含 NUL 的常量发射形状被改动（应保持 px_str）"; fi
#   `px_str("")` 本身是**合法**的（语料里有 `join("", …)`、`!= ""` 等**真空串**字面量）——
#   要判的不是它的有无，而是「含 NUL 的常量有没有被退化成空常量」：`PX_STR_LIT("")` 为 0 且
#   含 NUL 的常量**确实走了新路径**（计数 ≥ 8：语料里含 NUL 的字面量共 8 条以上）。
nlit=$(grep -c 'PX_STR_LIT(' "$emitc" || true)
chk "PX_STR_LIT 计数 ≥ 8（含 NUL 常量走新路径）" "$([ "$nlit" -ge 8 ] && echo yes || echo no)" "yes"
if grep -q 'PX_STR_LIT("")' "$emitc"; then bad "出现 PX_STR_LIT(\"\")（含 NUL 的常量退化成空常量）"; else chk "无空常量回归（PX_STR_LIT(\"\") 为 0）" no no; fi
if grep -q 'PXK_STR_LIT' build/nul_sem.c; then bad "C 轨发射里出现了 K 表宏（轨串了？）"; else chk "C 轨不含 K 表宏" no no; fi
"$PX" build --vm nul_sem.px > build/build_emitvm.log 2>&1 || bad "发射形状(VM)：构建失败"
if grep -q 'PXK_STR_LIT("a\\000b")' build/nul_sem.c; then chk "VM 轨 K 表含 NUL 常量 = PXK_STR_LIT" yes yes
else bad "VM 轨 K 表未按 PXK_STR_LIT 发射"; fi

say "── ④ 词法/语法面（本来就没坏，别改坏）——"
printf 'let a = "\\u{0}a"\n' > build/lex_probe.px
"$PX" lex build/lex_probe.px > build/lex_probe.tokens 2>&1 || bad "lex 失败"
if grep -q '\\0a' build/lex_probe.tokens; then chk "lex token 文本保留 \\0" yes yes
else bad "lex token 文本未见 \\0（转义/调试表示被改坏？）"; fi
"$PX" parse build/lex_probe.px > build/lex_probe.ast 2>&1 || bad "parse 失败"
if grep -q 'value: "\\0a"' build/lex_probe.ast; then chk "parse AST 值保留 \\0" yes yes
else bad "parse AST 值未见 \\0"; fi

say "── ⑤ 自举安全不变式：编译器源码代码里不得再有 \\u{0} 字面量 ——"
n_lit=$(grep -h '\\u{0}' ../../selfhost/codegen.px ../../selfhost/pxlexer.px ../../selfhost/it_util.px 2>/dev/null | grep -v '^[[:space:]]*#' | grep -c . || true)
chk "codegen/pxlexer/it_util 的代码行中 \\u{0} 字面量数" "$n_lit" "0"

say "── ⑥ 负控 4 道（必须全红）——"
run_ctl() {  # run_ctl <名> <脚本> <期望红的口径>
    local name="$1" script="$2" what="$3"
    python3 "$script" >/dev/null 2>&1 || { bad "$name: 打补丁失败"; restore_rt; return; }
    local red=0
    case "$what" in
      sem)
        rm -f build/nul_sem build/nul_bytes_out.bin
        "$PX" build --c nul_sem.px > build/negc.log 2>&1 && ./build/nul_sem > build/negc.out 2>/dev/null
        if [ -f build/negc.out ] && ! cmp -s build/negc.out golden_sem.stdout; then red=1; fi ;;
      out)
        rm -f build/nul_io build/nul_bytes_out.bin
        "$PX" build --c nul_io.px > build/negc.log 2>&1 && ./build/nul_io > build/negc.out 2>/dev/null
        if [ -f build/negc.out ] && ! cmp -s build/negc.out build/exp_io_stdout.bin; then red=1; fi ;;
      vmsem)
        rm -f build/nul_sem build/nul_bytes_out.bin
        "$PX" build --vm nul_sem.px > build/negc.log 2>&1 && ./build/nul_sem > build/negc.out 2>/dev/null
        if [ -f build/negc.out ] && ! cmp -s build/negc.out golden_sem.stdout; then red=1; fi ;;
    esac
    restore_rt
    if [ "$red" = "1" ]; then echo "  ✅ 负控 $name 判红"; else echo "  ❌ 负控 $name **未判红**（负控失效 ⇒ 门无判别力）"; FAIL=$((FAIL+1)); fi
}
run_ctl A negctl_a.py sem
run_ctl B negctl_b.py out
run_ctl C negctl_c.py sem
run_ctl D negctl_d.py vmsem

say "── 还原后复跑（必须全绿）——"
rm -f build/nul_sem build/nul_io nul_bytes_out.bin
"$PX" build --vm nul_sem.px > build/recheck.log 2>&1 && ./build/nul_sem > build/recheck.out 2>/dev/null
if cmp -s build/recheck.out golden_sem.stdout; then chk "还原后 VM 轨复跑全绿" same same; else bad "还原后 VM 轨复跑不一致"; fi
for f in runtime.c runtime.h vm.h vm.c; do
    if cmp -s "../../runtime/$f" "$SNAP/$f"; then chk "runtime/$f 逐字节还原" same same; else bad "runtime/$f 未还原"; fi
done

echo
if [ "$FAIL" = "0" ]; then echo "M155-VERIFY-OK"; exit 0; else echo "M155-VERIFY-FAIL（$FAIL 项）"; exit 1; fi
