#!/usr/bin/env bash
# ============================================================
# M173 门 · HTTP 反代/静态两件（晨曦 QA 清单 P1-2 + P1-1）
# ------------------------------------------------------------
# P1-2 vhost handler 分支缺 gzip 判定：压缩只在 ① 原生静态分支、② `.px` 脚本分支存在
#      ⇒ 所有 `vhost(host, handler)` 站点（Mahesvara 全部站点）文本响应明文下发。
# P1-1 池连接复用不重设 SO_RCVTIMEO/SO_SNDTIMEO：新建连接时才设 ⇒ 先大超时建池、
#      之后小超时**不生效**（反代无法「按路径收紧超时」）。
#
# 判据（VM + C 双轨；HTTP 服务端示例含 spawn，解释轨不支持 ⇒ 与 m23c/m31 同口径）：
#   [1] 用例 A 双轨：`M173A-GZIP-OK fails=0` 且两轨 stdout 逐字节一致
#       （**必须看线上字节**：http_request 客户端自动 gunzip ⇒ 只看头验不出真压没压，
#        故用例走裸 TCP + hex 断言「头结束符后紧跟 gzip 魔数 1f8b」）
#   [2] 用例 B 双轨：`M173B-POOL-TIMEOUT-OK fails=0` 且两轨一致
#   [3] 负控 A：关掉 vhost gzip 块（`if (0)`）⇒ 用例 A 必须变红
#   [4] 负控 B：关掉复用重设块（`else if (0)`）⇒ 用例 B 必须变红
#   [5] 各自还原后复绿（逐字节还原 runtime.c 由 sha256 断言）
# CI 用 --neg-skip（负控要重编 runtime，~1min×2）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m173_http_proxy
RT="$ROOT/runtime/runtime.c"
BAK=/tmp/m173_runtime.bak
W=/tmp/m173_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; rm -f "$BAK"' EXIT

build_run() {   # $1=用例名 $2=轨(vm|c) $3=输出前缀
    local name=$1 track=$2 pfx=$3 args=""
    [ "$track" = "c" ] && args="--c"
    rm -rf "$D/build"
    if ! (cd "$ROOT" && ./tools/px build $args "$D/$name.px" > "$W/$pfx.build.log" 2>&1) || [ ! -x "$D/build/$name" ]; then
        return 99
    fi
    timeout 120 "$D/build/$name" > "$W/$pfx.out" 2>"$W/$pfx.err"
    echo $? > "$W/$pfx.rc"
    rm -rf "$D/build"
    return 0
}

echo "=== [1] 用例 A（vhost handler gzip）双轨"
build_run vhost_gzip vm A.vm; rca=$?
build_run vhost_gzip c  A.c;  rcb=$?
chk "A/VM rc=0" "[ $rca -eq 0 ] && [ \"\$(cat $W/A.vm.rc)\" = 0 ]"
chk "A/C  rc=0" "[ $rcb -eq 0 ] && [ \"\$(cat $W/A.c.rc)\" = 0 ]"
chk "A/VM M173A-GZIP-OK" "grep -q 'M173A-GZIP-OK fails=0' $W/A.vm.out"
chk "A/C  M173A-GZIP-OK" "grep -q 'M173A-GZIP-OK fails=0' $W/A.c.out"
chk "A 双轨 stdout 逐字节一致" "cmp -s $W/A.vm.out $W/A.c.out"

echo "=== [2] 用例 B（池连接复用重设超时）双轨"
build_run pool_timeout vm B.vm; rcb1=$?
build_run pool_timeout c  B.c;  rcb2=$?
chk "B/VM rc=0" "[ $rcb1 -eq 0 ] && [ \"\$(cat $W/B.vm.rc)\" = 0 ]"
chk "B/C  rc=0" "[ $rcb2 -eq 0 ] && [ \"\$(cat $W/B.c.rc)\" = 0 ]"
chk "B/VM M173B-POOL-TIMEOUT-OK" "grep -q 'M173B-POOL-TIMEOUT-OK fails=0' $W/B.vm.out"
chk "B/C  M173B-POOL-TIMEOUT-OK" "grep -q 'M173B-POOL-TIMEOUT-OK fails=0' $W/B.c.out"
chk "B 双轨 stdout 逐字节一致" "cmp -s $W/B.vm.out $W/B.c.out"

if [ $NEG -eq 1 ]; then
echo "=== [3] 负控 A：关掉 vhost gzip 块 ⇒ 用例 A 必须变红"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = 'if (vst != 204 && vst != 304 && vblen > 0 && !px_extra_has_content_encoding(extra)) {'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, 'if (0) {   // NC-A'))
PYEOF
build_run vhost_gzip vm NEG.A >/dev/null
if grep -q 'FAIL 线上有 Content-Encoding' "$W/NEG.A.out" 2>/dev/null || ! grep -q 'fails=0' "$W/NEG.A.out" 2>/dev/null; then
    chk "NC-A 判红（vhost 分支不再压缩）" "true"
else
    chk "NC-A 判红（vhost 分支不再压缩）" "false"
fi
restore_all
chk "NC-A 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
build_run vhost_gzip vm POS.A >/dev/null
chk "NC-A 还原后复绿" "grep -q 'M173A-GZIP-OK fails=0' $W/POS.A.out"

echo "=== [4] 负控 B：关掉复用重设块 ⇒ 用例 B 必须变红"
restore_all
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '} else if (slot.to_ms != timeout_ms) {'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, '} else if (0) {   // NC-B'))
PYEOF
build_run pool_timeout vm NEG.B >/dev/null
if grep -q 'FAIL 小超时下不是 dict' "$W/NEG.B.out" 2>/dev/null || ! grep -q 'fails=0' "$W/NEG.B.out" 2>/dev/null; then
    chk "NC-B 判红（复用时不再重设超时）" "true"
else
    chk "NC-B 判红（复用时不再重设超时）" "false"
fi
restore_all
chk "NC-B 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
build_run pool_timeout vm POS.B >/dev/null
chk "NC-B 还原后复绿" "grep -q 'M173B-POOL-TIMEOUT-OK fails=0' $W/POS.B.out"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M173-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
