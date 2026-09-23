#!/usr/bin/env bash
# ============================================================
# M191 门（第 69 轮）：诊断面「三族口径 + 码的单一真相」
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md（§1 判据 / §2.3 个数分码 / §3 域前缀闭集）。
# 本轮落地（每条都有判据）：
#   ① native 侧 **42 处语言层错误补齐 R 码**（`切片步长 0`⇒R1006、`此类型不支持切片`⇒R1002、
#      `read_at 负长度`/`udp_open 端口范围`/`负间隔`⇒R1006、`env_set name`/`json_path 非法路径`/
#      `send|recv|close 目标不是通道`/`clock.with`族⇒R1002/1005…）；域前缀族登记为**闭集**。
#   ② `assert` 失败在解释轨曾被错标 `R1003`（越界与长度 —— 语义完全不符）⇒ 两侧统一 **R2001**；
#      `panic(msg)` 两侧此前都只有裸消息 ⇒ 统一 `R2001: msg`；`assert(false,"boom")` 文案统一。
#   ③ §2.3 定档：**方法**调用的实参个数 ⇒ `R1005`，**函数/内置** ⇒ `R1002`（两侧同批对齐：
#      解释轨 14 处字符串/列表方法码 + native 12 处）；并**拆开 3 处「个数+类型」混写守卫**
#      （`random_int` / `split` / `join`）；`join` 的解释轨文案与 native 对齐。
#
# 四层判据（每层可独立判红）：
#   [1] 静态：扫 runtime/*.c + selfhost/*.px 的每个出错站点 —— 带码 or 命中域前缀闭集；
#       个数类按 §2.3 判码；「个数+数值 + 类型」混写守卫判红；**未收口棘轮**（只许减少）。
#   [2] 动态：11 个探针 × **三轨**（解释 / VM / C）：rc≠0 + **同一 R 码** + **同一正文**。
#   [3] 反例守卫：探针必须真的跑到运行期（`before` 已打印）—— 防止"编译期就报错"冒充判据。
#   [4] 负控 5 道（A/B/C/D 打静态靶、E 打动态靶），各自**独立**判红、源逐字节还原。
#
# 用法：bash examples/m191_error_codes/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m191_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
RT=runtime/runtime.c
IC=selfhost/icall.px
IB=selfhost/ibuiltin.px
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { cp -f "$RT" "$BACK/rt"; cp -f "$IC" "$BACK/ic"; cp -f "$IB" "$BACK/ib"; }
restore_all() { [ -f "$BACK/rt" ] && cp -f "$BACK/rt" "$RT"; [ -f "$BACK/ic" ] && cp -f "$BACK/ic" "$IC"; [ -f "$BACK/ib" ] && cp -f "$BACK/ib" "$IB"; }
# ⚠️ 纪律（M190 教训）：每道负控前 **先 restore 再 snapshot**，保证打桩从**干净起点**开始；
#   trap 里也 restore，中途被杀也能还原。
snapshot
# trap 只做**还原**（负控打桩必须回滚），**不删工作目录** —— 门红了要把 $W 里的三轨输出留在原地，
# 否则"门红了却读不到原因"（M168/M188 的注解教训：诊断信息必须可读）。
trap 'restore_all' EXIT

# 前置不变量：本轮的四类改动必须在位（缺一即门坏）
for pat in 'R1006: 切片步长不能为 0' 'R2001: 断言失败' 'R1002: 此类型不支持切片' 'R1005: 方法 upper 不接受参数'; do
    grep -q "$pat" "$RT" || { echo "❌ 前置自查失败：runtime.c 缺「$pat」（源码被还原/被别的门盖掉了？）" >&2; exit 2; }
done
grep -q 'i_r2001(msg, pos)' "$IB" || { echo "❌ 前置自查失败：ibuiltin.px 的 assert 未改用 R2001" >&2; exit 2; }

static_ok() { python3 "$HERE/scan_errcodes.py" --root "$ROOT" > "$W/static.log" 2>&1; }

echo "=== [1] 静态判据：带码/域前缀 · 个数分码 · 混写拆分 · 未收口棘轮 ==="
static_ok
chk "[1] 静态判据全绿" "[ $? = 0 ]"
chk "[1] runtime.c 零缺口（624 站点全部带码或域前缀）" "grep -q 'runtime/runtime.c .*无码 0' '$W/static.log'"

echo "=== [2] 动态判据：11 探针 × 三轨（同码 + 同文）==="
norm() {  # $1=输出文件 → "CODE|正文"
    # ⚠️ 必须先**筛出错误行**再归一：程序自己的输出（`before`）也在同一份 stdout+stderr 里，
    #    首版直接 `head -1` ⇒ 永远取到 `before` ⇒ 11 条动态判据全假红（本轮实测）。
    grep -E '^运行时错误' "$1" 2>/dev/null | head -1 \
      | sed -E -e 's/^运行时错误: 错误 \[(R[0-9]{4})\] [0-9]+:[0-9]+: /\1|/' \
               -e 's/^运行时错误 \[[^]]*行[0-9]+\]: (R[0-9]{4}): /\1|/'
}
run3() {  # run3 <用例名>
    local b="$1" d="$W/d_$b"
    rm -rf "$d"; mkdir -p "$d"
    cp "$HERE/probe/$b.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$b.interp.out" 2>&1
    echo $? > "$W/$b.interp.rc"
    rm -rf "$d/build"
    timeout 180 ./tools/px build "$d/$b.px" > "$W/$b.vm.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.vm.out" 2>&1; echo $? > "$W/$b.vm.rc"
    else echo 999 > "$W/$b.vm.rc"; tail -3 "$W/$b.vm.build.log" > "$W/$b.vm.out"; fi
    rm -rf "$d/build"
    PX_BUILD_ENGINE=c timeout 180 ./tools/px build "$d/$b.px" > "$W/$b.c.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.c.out" 2>&1; echo $? > "$W/$b.c.rc"
    else echo 999 > "$W/$b.c.rc"; tail -3 "$W/$b.c.build.log" > "$W/$b.c.out"; fi
    rm -rf "$d/build"
}
judge_case() {  # judge_case <用例名> <期望码> <期望正文> —— 0=通过
    local b="$1" ec="$2" eb="$3" t ok=1
    for t in interp vm c; do
        [ "$(cat "$W/$b.$t.rc")" != 0 ] || ok=0                        # 三轨 rc≠0
        [ "$(norm "$W/$b.$t.out")" = "$ec|$eb" ] || ok=0               # 同码 + 同文 + 等于期望
        grep -q '^before$' "$W/$b.$t.out" || ok=0                      # 运行期（before 已打印）
    done
    [ $ok = 1 ]
}
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[2] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立；源逐字节还原）==="
    nc_static() {   # nc_static <名> <判据命令> <打桩命令>
        local name="$1" judge="$2" patch="$3"
        restore_all
        eval "$patch" >/dev/null 2>&1 || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        if ( eval "$judge" ); then echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1));
        else echo "  PASS 负控 $name：静态判据已判红（符合预期）"; pass=$((pass+1)); fi
        restore_all
    }
    PAT1='s/px_error("R1006: 切片步长不能为 0")/px_error("切片步长不能为 0")/'
    PAT2='s|px_error("io: 读取文件失败 %s", path)|px_error("fileio: 读取文件失败 %s", path)|'
    PAT3='s/i_r1005("方法 upper 不接受参数", pos)/i_r1002("方法 upper 不接受参数", pos)/'
    PAT4='s/if (nargs != 2) px_error("R1002: join 需要 2 个参数（分隔符, 列表）");/if (nargs != 2 || args[0].type != PX_STR) px_error("R1002: join 需要 2 个参数（分隔符, 列表）");/'
    nc_static "A 去掉 R 码（语言层无码）" "static_ok" "sed -i '$PAT1' '$RT'"
    nc_static "B 改域名（未登记域）" "static_ok" "sed -i '$PAT2' '$RT'"
    nc_static "C 方法个数码回退 R1002" "static_ok" "sed -i '$PAT3' '$IC'"
    nc_static "D 撤销混写拆分（join）" "static_ok" "sed -i '$PAT4' '$RT'"
    # 负控 E 打**动态**靶：runtime 的切片步长码去掉 ⇒ 三轨词条不再同码
    restore_all
    sed -i "$PAT1" "$RT"
    rm -rf "$W/nc"; mkdir -p "$W/nc"
    cp "$HERE/probe/d1_slice_step0.px" "$W/nc/"
    ( cd "$W/nc" && timeout 60 "$ROOT/bootstrap/pxi" d1_slice_step0.px ) > "$W/nce.i.out" 2>&1
    rm -rf "$W/nc/build"; ( cd "$W/nc" && timeout 180 "$ROOT/tools/px" build d1_slice_step0.px ) > "$W/nce.b.log" 2>&1
    if [ -x "$W/nc/build/d1_slice_step0" ]; then ( cd "$W/nc" && timeout 60 ./build/d1_slice_step0 ) > "$W/nce.v.out" 2>&1; fi
    if grep -q '^R1006|切片步长不能为 0$' <(norm "$W/nce.v.out"); then
        echo "  FAIL 负控 E：动态判据仍为绿（负控没有牙）"; fail=$((fail+1))
    else
        echo "  PASS 负控 E：动态判据已判红（VM 轨词条不含 R1006）"; pass=$((pass+1))
    fi
    restore_all
fi

echo ""
echo "结果: $pass 通过 / $fail 失败"
[ $fail = 0 ] && echo "M191-VERIFY-OK"
[ $fail = 0 ] || exit 1
