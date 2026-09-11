#!/bin/bash
# M109 二期验收：handler 响应头「同名多值」通路（str / list[str]）
#   期望：hdr_multi.px 末行 M109-HDRMULTI ALL OK，且 stderr 上有「不静默」告警
set -eu
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
HOME_DIR=$(CDPATH= cd -- "$DIR/../.." && pwd)
PX="${PX:-$HOME_DIR/tools/px}"
export PX_STDLIB="${PX_STDLIB:-$HOME_DIR/stdlib}"
LOG=/tmp/m109_hdrmulti.log

echo "== 构建 hdr_multi.px"
"$PX" build "$DIR/hdr_multi.px"

echo "== 运行（端口 18613）"
set +e
"$DIR/build/hdr_multi" >"$LOG" 2>&1
RC=$?
set -e
cat "$LOG"
echo "   （服务端退出码 $RC）"

echo "== 校验"
FAIL=0
if grep -q 'M109-HDRMULTI ALL OK' "$LOG"; then echo "  PASS 全部断言通过"; else echo "  FAIL 断言未全过"; FAIL=1; fi
if grep -q '响应头值类型不支持' "$LOG"; then echo "  PASS 不静默：值类型不支持已告警"; else echo "  FAIL 未观测到 badtype 告警"; FAIL=1; fi
if grep -q 'list 元素非字符串' "$LOG"; then echo "  PASS 不静默：list 元素非 str 已告警"; else echo "  FAIL 未观测到 list 元素告警"; FAIL=1; fi

if [ "$FAIL" = "0" ]; then
    echo "════════ m109_headers_multi: PASS ════════"
    exit 0
fi
echo "════════ m109_headers_multi: FAIL ════════"
exit 1
