#!/usr/bin/env bash
# M138 验证：正则的「Go 保真」门（qg-issue 87 缺陷 84 根治 + 复发防线）
#
#   缺陷 84（语言/runtime）：引擎不认识 `\b` / `\B`（及 `\A` / `\z` / `\x…` / 八进制 / `\Q…\E`），
#     且**字符类内的转义处理是错的**：`[\D]` `[\W]` `[\S]` 被当字面量 D/W/S（静默错值）、
#     `[\b]` 被当字面量 b。现场后果：任何含 `\b…\b` 的 `secret_patterns` 直接让
#     token-cache 的本地快判**整体禁用**（Go 侧启用）⇒ 两侧行为分叉。
#   同批照出的还有：`\s` 含 `\v`（Go 的 `\s` **不含** `\v`，而 POSIX `[[:space:]]` 含），
#     以及 find_all / replace 的「相邻空匹配」迭代规则与 Go 不同。
#
# 本门四件事：
#   ① 真值对拍：truth/（**Go regexp 本尊**）与 parity.px 的输出逐字节 diff（VM+C 两轨都跑）
#   ② 自断言：esc() 自身 + **已登记边界**（M138_SELF_OK 计数）
#   ③ 负控 A：把真值改一个字节 ⇒ diff **必须**变红（证明 diff 不是白跑）
#   ④ 负控 B：把 parity.px 的语料改一处 ⇒ 输出必须变化
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m138_build.log
FAIL=0

mkdir -p build

echo "== [truth] 生成 Go 真值（regexp 本尊） =="
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

run_track() {   # $1=标签；$2..=编译命令前缀
    local label="$1"; shift
    echo "== [$label] 编译 =="
    if ! "$@" "$PX" build parity.px > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    echo "== [$label] 运行 =="
    local out rc
    out=$(./build/parity 2>&1); rc=$?
    echo "$out" | tail -2 | sed 's/^/     /'
    echo "$out" | grep -q "M138_SELF_OK" || {
        echo "FAIL [$label] 自断言未通过（期望 M138_SELF_OK）"; FAIL=$((FAIL+1)); return; }
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; FAIL=$((FAIL+1)); return; }
    if [ "$TRUTH_GEN" = "1" ]; then
        local n
        n=$(wc -l < build/truth.txt)
        echo "$out" | head -n "$n" > "build/$label.px.txt"
        if diff -u build/truth.txt "build/$label.px.txt" > "build/$label.diff" 2>&1; then
            echo "PASS [$label] 与 Go 真值逐字节一致（$n 行）"
        else
            echo "FAIL [$label] 与 Go 真值不一致："; head -40 "build/$label.diff"; FAIL=$((FAIL+1))
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
    sed -i '1s/.*/f|TAMPERED|abc|0|-|-|-|-/' build/truth_bad.txt
    if diff -q build/truth_bad.txt build/VM轨.px.txt >/dev/null 2>&1; then
        echo "FAIL [负控A] 篡改后 diff 仍绿 —— 门在自欺"; FAIL=$((FAIL+1))
    else
        echo "PASS [负控A] 篡改后 diff 变红"
    fi
fi

# ── ④ 负控 B：parity.px 语料被改坏 ⇒ 输出必须变化 ──
echo "== [负控B] 篡改 parity.px 一条语料 ⇒ 输出必须变化 =="
sed 's|pats <- "\\\\b\\*"|pats <- "\\\\b\\*\\*"|' parity.px > build/parity_bad.px
if "$PX" build build/parity_bad.px > "$LOG" 2>&1; then
    if [ -x build/parity_bad ]; then
        ./build/parity_bad > build/bad.txt 2>&1
        if diff -q build/bad.txt build/VM轨.px.txt >/dev/null 2>&1; then
            echo "FAIL [负控B] 篡改语料后输出仍与基准相同 —— 门在自欺"; FAIL=$((FAIL+1))
        else
            echo "PASS [负控B] 篡改语料后输出变化"
        fi
    else
        echo "PASS [负控B] 篡改后的源码未产出二进制（亦判红）"
    fi
else
    echo "PASS [负控B] 篡改后的源码编译失败（亦判红）"
fi

echo "════════════════════════════════════"
if [ "$FAIL" = "0" ]; then echo "M138-VERIFY-OK"; exit 0; fi
echo "M138-VERIFY-FAIL（$FAIL 项）"; exit 1
