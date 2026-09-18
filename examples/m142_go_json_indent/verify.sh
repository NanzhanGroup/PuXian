#!/usr/bin/env bash
# M142 验证：Go `encoding/json` 的 Indent / Compact / Marshal 转义路径 / HTMLEscape 逐字节复刻
#             + `quoteChar(byte)` 全 256 取值 + 畸形 UTF-8 下的「字符定义唯一性」（缺陷 111/112）
#
# 门做五件事：
#   ① 真值对拍：88 例语料 × 4 面 + Q 面 256 行 = 696 行，与 **Go encoding/json 本尊**逐字节 diff
#      （VM 轨 + C 轨各跑一遍，断言集相同）
#   ② 自断言：幂等 / compact∘indent 一致 / 空容器不成行 / 前缀 / 错误文案与**1 基字节偏移** /
#      Latin-1 文案 / max depth / HTMLEscape / 转义路径 / 负控（M142_ASSERT: 31P/0F）
#   ③ 负控 A：篡改 Go 真值一行 ⇒ diff **必须**变红
#   ④ 负控 B：篡改 stdlib 实现一份 ⇒ 输出 **必须**变化
#   ⑤ 负控 C：篡改语料一条 ⇒ 输出 **必须**变化
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m142_build.log
FAIL=0

mkdir -p build

echo "== [truth] 生成 Go 真值（encoding/json 本尊：Indent / Compact / Marshal / HTMLEscape / quoteChar）=="
command -v go >/dev/null 2>&1 || { echo "FAIL：本机无 go 工具链（本门真值必须来自 Go）"; exit 1; }
if (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1); then
    ./build/truth corpus.txt > build/truth.txt
    echo "   真值行数：$(wc -l < build/truth.txt)（88 例 × 5 行 + Q 面 256 行）"
else
    echo "FAIL [truth] Go 编译失败"; tail -20 "$LOG"; exit 1
fi

run_track() {   # $1=标签；$2..=编译命令前缀（env [PX_BUILD_ENGINE=c]）
    local label="$1"; shift
    echo "== [$label] 编译 + 运行 =="
    if ! "$@" "$PX" build fidelity.px > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    local out rc n
    out=$(./build/fidelity 2>&1); rc=$?
    n=$(wc -l < build/truth.txt)
    echo "$out" | head -n "$n" > "build/$label.px.txt"
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; FAIL=$((FAIL+1)); return; }
    echo "$out" | grep -q "M142_ASSERT: 31P/0F" || {
        echo "FAIL [$label] 断言计数不符（期望 31P/0F）："; echo "$out" | grep "M142_ASSERT"; FAIL=$((FAIL+1)); return; }
    if diff -u build/truth.txt "build/$label.px.txt" > "build/$label.diff" 2>&1; then
        echo "PASS [$label] 与 Go 真值逐字节一致（$n 行）"
    else
        echo "FAIL [$label] 与 Go 真值不一致："; head -30 "build/$label.diff"; FAIL=$((FAIL+1)); return
    fi
    echo "PASS [$label] 自断言 31P/0F + 退出码 0"
}

run_track "VM轨" env
run_track "C轨" env PX_BUILD_ENGINE=c

# ── ③ 负控 A：真值被改坏 ⇒ diff 必须变红 ──
echo "== [负控A] 篡改 Go 真值一行 ⇒ diff 必须变红 =="
cp build/truth.txt build/truth_bad.txt
sed -i '1s/.*/CASE 0-TAMPERED/' build/truth_bad.txt
if diff -q build/truth_bad.txt "build/VM轨.px.txt" >/dev/null 2>&1; then
    echo "FAIL [负控A] 篡改真值后 diff 仍绿 —— 门在自欺"; FAIL=$((FAIL+1))
else
    echo "PASS [负控A] 篡改真值后 diff 变红"
fi

# ── ④ 负控 B：篡改 stdlib 实现 ⇒ 输出必须变化 ──
#   做法：把 stdlib 两份拷进 build/tamper/，把 `": "`（冒号后空格）改成 `":"`，
#   再让 fidelity.px 改 import 指到副本（**不碰** stdlib 原件）。
echo "== [负控B] 篡改 go_json_indent.px 的冒号间距 ⇒ 输出必须变化 =="
mkdir -p build/tamper
cp ../../stdlib/go_json_scan.px ../../stdlib/go_json_indent.px build/tamper/
sed -i 's|parts.append(" ")|0|' build/tamper/go_json_indent.px
sed -e 's|^import std.go_json_scan|import "./build/tamper/go_json_scan.px"|' \
    -e 's|^import std.go_json_indent|import "./build/tamper/go_json_indent.px"|' \
    fidelity.px > build/fidelity_tamper.px
if "$PX" build build/fidelity_tamper.px > "$LOG" 2>&1; then
    if [ -x build/fidelity_tamper ]; then
        tamper_out=$(./build/fidelity_tamper 2>&1 | head -n "$(wc -l < build/truth.txt)")
    else
        tamper_out="<无产物>"      # 没产出可执行文件：同样判红（下面比较必然不等）
    fi
    if [ "$tamper_out" = "$(cat build/truth.txt)" ]; then
        echo "FAIL [负控B] 篡改实现后输出仍与真值相同 —— 门在自欺"; FAIL=$((FAIL+1))
    else
        echo "PASS [负控B] 篡改实现后输出变化"
    fi
else
    echo "PASS [负控B] 篡改后的源码编译失败（亦判红）"
fi

# ── ⑤ 负控 C：篡改语料一条 ⇒ 输出必须变化 ──
echo "== [负控C] 篡改语料一条（第 7 行 = CASE 4 的 {\"a\":1} → {\"a\":22}）⇒ 输出必须变化 =="
cp corpus.txt build/corpus_bad.txt
sed -i '7s/.*/x7b2261223a32327d/' build/corpus_bad.txt
if cmp -s corpus.txt build/corpus_bad.txt; then
    echo "FAIL [负控C] 语料未被改动（sed 未命中）"; FAIL=$((FAIL+1))
else
    cp corpus.txt build/corpus_ok.txt
    cp build/corpus_bad.txt corpus.txt
    bad_run=$(./build/fidelity 2>&1 | head -n "$(wc -l < build/truth.txt)")
    cp build/corpus_ok.txt corpus.txt        # 立刻还原
    if cmp -s corpus.txt build/corpus_ok.txt; then
        if [ "$bad_run" = "$(cat build/truth.txt)" ]; then
            echo "FAIL [负控C] 改语料后输出仍与真值相同 —— 语料没真正进流水线"; FAIL=$((FAIL+1))
        else
            echo "PASS [负控C] 改语料后输出变化（语料确实驱动了流水线）"
        fi
    else
        echo "FAIL [负控C] 语料未正确还原"; FAIL=$((FAIL+1))
    fi
fi

echo "════════════════════════════════════"
if [ "$FAIL" = "0" ]; then echo "M142-VERIFY-OK"; exit 0; fi
echo "M142-VERIFY-FAIL（$FAIL 项）"; exit 1
