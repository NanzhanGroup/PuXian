#!/usr/bin/env bash
# ============================================================
# M90-S1/F1 verify.sh —— bc_emit 默认参数支持 验收
# ------------------------------------------------------------
# 内容：
#   1) default_args.px 行为对拍：VM 轨（compiler_new emit-c → gcc）产物
#      stdout vs 旧轨 pxi 解释 stdout 逐字节一致（F1 语义验收：
#      常量默认/多默认部分提供/默认引用前参/引用全局/别名间接调用缺参）
#   2) capability.px BC 编译门（F1 直击：此前
#      `compiler_new bc capability.px` rc=1 —— bc_emit_func_body 对带默认
#      参数函数 panic "bc_emit 默认参数未实现"；现应 rc=0）
#   3) NARGS 指令存在性：capability BC dump 含 NARGS（入口填充序列落地）
# 退出码：0=全 PASS；非 0=有失败。
# 前置：selfhost/build/compiler_new（含 F1 改动重链）+ 最新 rtcache（vm.o）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
CN="$ROOT/selfhost/build/compiler_new"
RT="$ROOT/runtime"
DIR="$ROOT/examples/m90_s1"
OUT=/tmp/m90_s1_verify
mkdir -p "$OUT"

[ -x "$CN" ] || { echo "❌ 缺 selfhost/build/compiler_new" >&2; exit 1; }
CACHE=""
for d in $(ls -dt "$ROOT"/.rtcache/*/ 2>/dev/null); do
    [ -f "$d/.complete" ] && [ -f "$d/vm.o" ] && CACHE="$d" && break
done
[ -n "$CACHE" ] || { echo "❌ 未找到含 vm.o 的 rtcache" >&2; exit 1; }
echo "── rtcache: $CACHE"

emit_and_link() { # $1=px文件 $2=输出名 → /tmp/<out>（VM 轨：compiler_new emit-c → gcc 链 rtcache）
    local src="$1" name="$2"
    ( ulimit -v 10000000; "$CN" --emit-c "$src" > "$OUT/${name}.c" 2>"$OUT/${name}.emit.log" ) || {
        echo "  ❌ emit-c 失败: $src"; tail -3 "$OUT/${name}.emit.log"; return 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" "$OUT/${name}.c" -o "$OUT/${name}.o" 2>"$OUT/${name}.cc.log" || {
        echo "  ❌ C 编译失败: $src"; tail -5 "$OUT/${name}.cc.log"; return 1; }
    local objs="" f
    for f in "$CACHE"/*.o; do objs="$objs $f"; done
    gcc -O2 -pthread -o "$OUT/${name}" "$OUT/${name}.o" $objs \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread || return 1
    return 0
}

PASS=0; FAIL=0

echo "── 1) default_args.px 行为对拍（VM 轨 vs 旧轨 pxi stdout）"
timeout 60 "$PX" run "$DIR/default_args.px" >"$OUT/da.pxi" 2>/dev/null; orc=$?
if [ $orc -ne 0 ]; then
    echo "  FAIL 旧轨 pxi 解释异常 rc=$orc"; FAIL=$((FAIL+1))
else
    emit_and_link "$DIR/default_args.px" da || FAIL=$((FAIL+1))
    timeout 60 "$OUT/da" >"$OUT/da.vm" 2>&1; vrc=$?
    if [ $vrc -eq 0 ] && diff -q "$OUT/da.pxi" "$OUT/da.vm" >/dev/null 2>&1; then
        echo "  PASS default_args 双轨 stdout 逐字节一致（$(wc -l <"$OUT/da.pxi") 行）"
        grep -q "DONE" "$OUT/da.vm" && PASS=$((PASS+1)) || { echo "  FAIL 缺 DONE"; FAIL=$((FAIL+1)); }
    else
        echo "  FAIL default_args VM 轨（rc=$vrc）"; diff "$OUT/da.pxi" "$OUT/da.vm" | head -10; FAIL=$((FAIL+1))
    fi
fi

echo "── 2) capability.px BC 编译门（F1 直击：此前 rc=1 默认参数 panic）"
timeout 300 "$CN" bc selfhost/capability.px >"$OUT/cap.bc.dump" 2>"$OUT/cap.bc.log"; crc=$?
if [ $crc -eq 0 ] && [ -s "$OUT/cap.bc.dump" ]; then
    echo "  PASS capability.px BC 编译成功（dump $(wc -l <"$OUT/cap.bc.dump") 行）"
    PASS=$((PASS+1))
else
    echo "  FAIL capability.px BC 编译（rc=$crc）"; tail -5 "$OUT/cap.bc.log"; FAIL=$((FAIL+1))
fi

echo "── 3) NARGS 入口填充落地（capability BC dump 含 NARGS 指令）"
if grep -q "NARGS" "$OUT/cap.bc.dump" 2>/dev/null; then
    echo "  PASS capability BC 含 NARGS（$(grep -c NARGS "$OUT/cap.bc.dump") 处）"
    PASS=$((PASS+1))
else
    echo "  FAIL capability BC 无 NARGS"; FAIL=$((FAIL+1))
fi

echo "── 4) cmpsem.px C轨 vs VM轨 对拍（默认参数「调用时入口求值」语义）"
# 背景：旧轨 pxi 解释器 def 定义时固化默认（Python 式）；codegen C 轨把默认
#   表达式放函数入口每调用求值（可引用前参）。VM 轨 F1 对齐 codegen（VM 化的
#   替代目标是 C 轨产物）。故本项基准 = pxc build 的 C 轨产物 stdout（非 pxi）。
if ./tools/px build "$DIR/cmpsem.px" >"$OUT/cmpsem.cbuild.log" 2>&1; then
    timeout 60 "$DIR/build/cmpsem" >"$OUT/cmpsem.c.out" 2>&1; crc=$?
    if [ $crc -ne 0 ]; then
        echo "  FAIL cmpsem C 轨产物运行 rc=$crc"; FAIL=$((FAIL+1))
    else
        emit_and_link "$DIR/cmpsem.px" cmpsem || FAIL=$((FAIL+1))
        timeout 60 "$OUT/cmpsem" >"$OUT/cmpsem.vm.out" 2>&1; vrc=$?
        if [ $vrc -eq 0 ] && diff -q "$OUT/cmpsem.c.out" "$OUT/cmpsem.vm.out" >/dev/null 2>&1; then
            echo "  PASS cmpsem C轨==VM轨 stdout 逐字节一致（$(wc -l <"$OUT/cmpsem.c.out") 行）"
            PASS=$((PASS+1))
        else
            echo "  FAIL cmpsem 双轨不一致（C rc=$crc VM rc=$vrc）"; diff "$OUT/cmpsem.c.out" "$OUT/cmpsem.vm.out" | head -10; FAIL=$((FAIL+1))
        fi
    fi
else
    echo "  FAIL cmpsem C 轨 build 失败"; FAIL=$((FAIL+1))
fi

echo "── 5) for+break/continue 死循环回归（M90-S1 附带修复）"
# t1_bc.px = capability 11 段 break/continue 最小复现。修复前：bc_emit_for 把
#   continue 回填到 jb（增量后 JMP 指令）→ body 内 continue 跳过 ADD → ctr 不
#   前进 → 死循环（capability VM 化暴露；vm_ab 38 例无 continue-for 未覆盖）。
#   修复：continue 回填到增量段起点 inc_start（先 +1 再重判，Python for 语义）。
timeout 60 "$PX" run "$DIR/t1_bc.px" >"$OUT/t1.pxi" 2>/dev/null; orc=$?
if [ $orc -eq 0 ]; then
    emit_and_link "$DIR/t1_bc.px" t1 || FAIL=$((FAIL+1))
    timeout 60 "$OUT/t1" >"$OUT/t1.vm" 2>&1; vrc=$?
    if [ $vrc -eq 0 ] && diff -q "$OUT/t1.pxi" "$OUT/t1.vm" >/dev/null 2>&1; then
        echo "  PASS for+continue 修复后双轨一致（rc=0, bc=4）"; PASS=$((PASS+1))
    else
        echo "  FAIL for+continue（VM rc=$vrc）"; diff "$OUT/t1.pxi" "$OUT/t1.vm" | head; FAIL=$((FAIL+1))
    fi
else
    echo "  FAIL for+continue pxi 基准 rc=$orc"; FAIL=$((FAIL+1))
fi

echo "════════ M90-S1/F1 verify：PASS=$PASS FAIL=$FAIL ════════"
[ $FAIL -eq 0 ]
