#!/bin/bash
# M62 语言面欠账 L1/L6/L7 验证（双模式一致）
#   L1 %g 浮点 .0 对齐（runtime fmt_num）→ fp_floatfmt.px
#   L6 split 保留空段（回归防回退）        → fp_split.px
#   L7 pxi bytes 族白名单补齐              → fp_bytes.px
# 运行：bash verify.sh（需 bootstrap/pxi 为含 L7 白名单的新版；pxc build 用新 runtime）
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
FAIL=0
mkdir -p build

for f in fp_floatfmt fp_split fp_bytes; do
  echo "== $f =="
  if ! $PX run "$f.px" > "/tmp/m62_${f}.run" 2>&1; then
    echo "  ❌ 解释模式运行失败"; tail -3 "/tmp/m62_${f}.run"; FAIL=1; continue
  fi
  if ! $PX build --no-quic "$f.px" > "/tmp/m62_${f}.buildlog" 2>&1; then
    echo "  ❌ 编译失败"; tail -4 "/tmp/m62_${f}.buildlog"; FAIL=1; continue
  fi
  if ! ./build/$f > "/tmp/m62_${f}.bin" 2>&1; then
    echo "  ❌ 编译产物运行失败"; FAIL=1
  fi
  echo "  解释: $(tail -1 /tmp/m62_${f}.run)"
  echo "  编译: $(tail -1 /tmp/m62_${f}.bin)"
  if ! diff -q "/tmp/m62_${f}.run" "/tmp/m62_${f}.bin" > /dev/null; then
    echo "  ❌ 双模式输出不一致:"; diff "/tmp/m62_${f}.run" "/tmp/m62_${f}.bin"; FAIL=1
  fi
  grep -q " OK$" "/tmp/m62_${f}.run" || { echo "  ❌ 断言未全过"; FAIL=1; }
done

if [ "$FAIL" -eq 0 ]; then
  echo "M62 L1/L6/L7 全 PASS"
else
  echo "M62 存在失败"
  exit 1
fi
