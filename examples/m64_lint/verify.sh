#!/usr/bin/env bash
# M64-S3 lint 自测验证
set -u
cd "$(dirname "$0")/../.." || exit 1
PXL="$PWD/bootstrap/pxlint"
pass=0; fail=0
chk() { # name cond
    if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi
}
# 注：L004 空块在 PuXian 语法中不可表达（parser 强制块非空，空 def/if/for 均
# parse 报错），故自测不覆盖 L004；实现保留防御（对齐 Rust lint.rs）。
echo "[1] bad.px 命中 L001/L002/L003/L005/L006/L008"
out=$("$PXL" examples/m64_lint/bad.px 2>&1); rc=$?
chk "rc=1（有 Error）" "[ $rc -eq 1 ]"
for code in L001 L002 L003 L005 L006 L008; do
    chk "$code 命中" "echo \"\$out\" | grep -q '$code'"
done
chk "L001 含 unused_var" "echo \"\$out\" | grep 'L001' | grep -q 'unused_var'"
chk "L002 含 no_such_thing" "echo \"\$out\" | grep 'L002' | grep -q 'no_such_thing'"
chk "L005 含 duplicate" "echo \"\$out\" | grep 'L005' | grep -q 'duplicate'"
chk "L006 含 BadFunc" "echo \"\$out\" | grep 'L006' | grep -q 'BadFunc'"
echo "[2] longline.px L007/L008"
out2=$("$PXL" examples/m64_lint/longline.px 2>&1)
chk "L007 命中" "echo \"\$out2\" | grep -q 'L007'"
chk "L008 命中" "echo \"\$out2\" | grep -q 'L008'"
echo "[3] clean.px 零告警"
out3=$("$PXL" examples/m64_lint/clean.px 2>&1); rc3=$?
chk "rc=0" "[ $rc3 -eq 0 ]"
chk "无诊断行" "! echo \"\$out3\" | grep -qE ': (W|E) L00'"
echo "[4] --json 机器可读"
out4=$("$PXL" --json examples/m64_lint/bad.px 2>&1)
chk "JSON 含 code L002" "echo \"\$out4\" | grep -q '\"code\":\"L002\"'"
chk "JSON error 级别" "echo \"\$out4\" | grep -q '\"level\":\"error\"'"
echo "[5] clean.px --strict rc=0（无警告）"
"$PXL" --strict examples/m64_lint/clean.px >/dev/null 2>&1; rcs=$?
chk "clean --strict rc=0" "[ $rcs -eq 0 ]"
echo
echo "结果: $pass 通过 / $fail 失败"
[ $fail -eq 0 ] || exit 1
