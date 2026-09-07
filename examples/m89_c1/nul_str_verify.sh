#!/usr/bin/env bash
# ============================================================
# M89-S3-C1: nul_str_verify.sh —— 内嵌 NUL 字符串 runtime 修复回归
# ------------------------------------------------------------
# 修复背景（runtime.c）：M83-S1 修 len() 尊重 str.len 字节边界，但 px_index/px_slice
#   仍用 strlen 版 px_unicode_len（内嵌 NUL 截断）→ len()=N 的含 NUL 串 s[0] 判越界；
#   且单字符结果 px_str 构造在取到 NUL 时截断成空串。导致自举编译器 lex 含 "\u{0}"
#   字面量的源码（pxlexer.px 等）确定性崩 "字符串索引越界: 0"（C1 自举前置阻塞）。
# 退出码：0=PASS；非 0=FAIL。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
./tools/px build examples/m89_c1/nul_str_regress.px >/tmp/nul_verify_build.log 2>&1 || { echo "❌ 编译失败"; cat /tmp/nul_verify_build.log; exit 1; }
./examples/m89_c1/build/nul_str_regress
rc=$?
if [ $rc -eq 0 ]; then
    echo "== C1 内嵌 NUL 字符串 runtime 回归 PASS =="
else
    echo "❌ C1 内嵌 NUL 字符串 runtime 回归 FAIL（exit=$rc）"
fi
exit $rc
