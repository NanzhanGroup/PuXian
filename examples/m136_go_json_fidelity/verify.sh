#!/usr/bin/env bash
# M136 验证：JSON 往返 / str() 的「Go 逐字节保真」（qg-issue 87 缺陷 92–95 根治 + 复发防线）
#
#   缺陷 92（runtime）：`str(s)` 对含内嵌 NUL 的字符串按 C 串重造 ⇒ 首个 0x00 处静默截断
#   缺陷 93（runtime）：`json_parse` 整数溢出用 strtoll 夹钳值（LLONG_MAX）当结果
#   缺陷 94（runtime）：JSON 字符串解码 `\u0000` 后同样被 C 串重造 ⇒ 值被截断
#   缺陷 95（runtime）：`jgo_format_float` 用 `%.*f` 扫"最短"⇒ 大整数值给**精确值**
#                      而不是 Go 的**最短往返数字**展开（1.23e19 差 168）
#
# 本门四件事：
#   ① 真值对拍：truth/（**Go encoding/json 本尊**）与 fidelity.px 的 corpus 逐行 diff（两轨都跑）
#   ② 自断言：str()/bytes NUL 保真 + JSON 四类边界（M136_ASSERT 计数）
#   ③ 负控 A：把 truth 的一行改错 ⇒ diff **必须**变红（证明 diff 不是白跑）
#   ④ 负控 B：把 fidelity.px 的某条 corpus 改一个字节 ⇒ diff 必须变红
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m136_build.log
FAIL=0

mkdir -p build

echo "== [truth] 生成 Go 真值（encoding/json 本尊） =="
command -v go >/dev/null 2>&1 || { echo "SKIP：本机无 go 工具链（真值对拍跳过，仅跑自断言）"; TRUTH=""; }
TRUTH_GEN=0
if command -v go >/dev/null 2>&1; then
    if (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1); then
        TRUTH_GEN=1
        ./build/truth > build/truth.txt
        echo "   真值行数：$(wc -l < build/truth.txt)"
    else
        echo "FAIL [truth] Go 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1))
    fi
fi

run_track() {   # $1=标签；$2..=编译命令前缀（env [PX_BUILD_ENGINE=c]）
    local label="$1"; shift
    echo "== [$label] 编译 =="
    if ! "$@" "$PX" build fidelity.px > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    echo "== [$label] 运行 =="
    local out rc
    out=$(./build/fidelity 2>&1); rc=$?
    echo "$out" | sed 's/^/     /'
    echo "$out" | grep -q "M136_ASSERT: 24P/0F" || {
        echo "FAIL [$label] 断言计数不符（期望 24P/0F）"; FAIL=$((FAIL+1)); return; }
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; FAIL=$((FAIL+1)); return; }
    # ① 真值对拍：取输出前 N 行（corpus 行）与 Go 真值 diff
    if [ "$TRUTH_GEN" = "1" ]; then
        local n
        n=$(wc -l < build/truth.txt)
        echo "$out" | head -n "$n" > "build/$label.px.txt"
        if diff -u build/truth.txt "build/$label.px.txt" > "build/$label.diff" 2>&1; then
            echo "PASS [$label] 与 Go 真值逐字节一致（$n 行）"
        else
            echo "FAIL [$label] 与 Go 真值不一致："; head -30 "build/$label.diff"; FAIL=$((FAIL+1))
        fi
    fi
    echo "PASS [$label] 自断言 + 退出码"
}

run_track "VM轨" env
run_track "C轨" env PX_BUILD_ENGINE=c

# ── ③ 负控 A：真值被改坏 ⇒ diff 必须变红 ──
if [ "$TRUTH_GEN" = "1" ]; then
    echo "== [负控A] 篡改 Go 真值一行 ⇒ diff 必须变红 =="
    cp build/truth.txt build/truth_bad.txt
    sed -i '1s/.*/{"s":"TAMPERED"}/' build/truth_bad.txt
    if diff -q build/truth_bad.txt build/VM轨.px.txt >/dev/null 2>&1; then
        echo "FAIL [负控A] 篡改后 diff 仍绿 —— 门在自欺"; FAIL=$((FAIL+1))
    else
        echo "PASS [负控A] 篡改后 diff 变红"
    fi
fi

# ── ④ 负控 B：PuXian 侧 corpus 被改坏 ⇒ diff 必须变红 ──
echo "== [负控B] 篡改 fidelity.px 一条 corpus ⇒ 输出必须变化 =="
sed 's/{"f":1e21}/{"f":1e2}/' fidelity.px > build/fidelity_bad.px
if "$PX" build build/fidelity_bad.px > "$LOG" 2>&1; then
    if [ -x build/fidelity_bad ]; then
        ./build/fidelity_bad > build/bad.txt 2>&1
    else
        cp build/fidelity build/fidelity_bad 2>/dev/null || true
        ./build/fidelity_bad > build/bad.txt 2>&1
    fi
    if diff -q build/bad.txt build/VM轨.px.txt >/dev/null 2>&1; then
        echo "FAIL [负控B] 篡改 corpus 后输出仍与基准相同 —— 门在自欺"; FAIL=$((FAIL+1))
    else
        echo "PASS [负控B] 篡改 corpus 后输出变化"
    fi
else
    echo "PASS [负控B] 篡改后的源码编译失败（亦判红）"
fi

echo "════════════════════════════════════"
if [ "$FAIL" = "0" ]; then echo "M136-VERIFY-OK"; exit 0; fi
echo "M136-VERIFY-FAIL（$FAIL 项）"; exit 1
