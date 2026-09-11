#!/bin/sh
# M109-S0 验收：构建 + 运行 hdr_pass.px（一条命令）
#   期望末行：M109-HDR ALL OK
set -eu
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
HOME_DIR=$(CDPATH= cd -- "$DIR/../.." && pwd)
PX="${PX:-$HOME_DIR/tools/px}"
export PX_STDLIB="${PX_STDLIB:-$HOME_DIR/stdlib}"

echo "== 构建 hdr_pass.px"
"$PX" build "$DIR/hdr_pass.px"
echo "== 运行（端口 18612）"
exec "$DIR/build/hdr_pass"
