#!/usr/bin/env bash
# ============================================================
# M224 门 · gzip 内容协商口径（缺陷 326 · 晨曦 QA P1-2 续）
# ------------------------------------------------------------
# 病灶：`px_resp_gzipable` 的 Accept-Encoding 判据是裸 `strstr(ae, "gzip")`
#   （M53 起），四个面都不合规 ——
#     ① 不解析 q 值 ⇒ `gzip;q=0`（客户端**显式拒绝**）仍被压缩；
#     ② 子串误命中 ⇒ `xgzip` / `not-gzip` 也算接受；
#     ③ 不认 `*` 通配（RFC 9110 §12.5.3：匹配任何未显式列出的编码）；
#     ④ token 比较大小写敏感 ⇒ `GZIP` 不命中（HTTP token 大小写不敏感 §11.1）。
#   同族：Content-Type 的 json/xml/svg/csv 匹配也是大小写敏感的 `strstr`。
#   另：runtime 这层压缩**无法关闭** ⇒ 新增 `opts{"gzip": false}`（晨曦诉求②）。
# 三处调用（vhost / .px 脚本 / 原生静态）共用 `px_resp_gzipable` ⇒ 一处修三处同愈。
#
# 判据：
#   [1] 用例 A（21 例协商）双轨：`M224A-NEGOTIATE-OK fails=0` 且两轨 stdout 逐字节一致
#   [2] 用例 B（服务级开关）双轨：`M224B-SWITCH-OK fails=0` 且两轨一致
#   [3] 静态判据：新判据函数在位 + 旧 `strstr` 判据**已不存在**（防回退）
#   [4] 负控 A：把 AE 判据退回 `strstr` ⇒ 用例 A 必须变红
#   [5] 负控 B：抽掉服务级开关判据 ⇒ 用例 B 必须变红
#   [6] 各自还原后复绿（逐字节还原由 sha256 断言）
# CI 用 --neg-skip（负控要重编 runtime，~20s×4）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m224_gzip_negotiate
RT="$ROOT/runtime/runtime.c"
BAK=/tmp/m224_runtime.bak
W=/tmp/m224_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; rm -f "$BAK"' EXIT

build_run() {   # $1=用例名 $2=轨(vm|c) $3=输出前缀
    local name="$1"
    local track="$2"
    local pfx="$3"
    local args=""
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

echo "=== [1] 用例 A（gzip 内容协商 21 例）双轨"
build_run gzneg vm A.vm; rca=$?
build_run gzneg c  A.c;  rcb=$?
chk "A/VM rc=0" "[ $rca -eq 0 ] && [ \"\$(cat $W/A.vm.rc)\" = 0 ]"
chk "A/C  rc=0" "[ $rcb -eq 0 ] && [ \"\$(cat $W/A.c.rc)\" = 0 ]"
chk "A/VM M224A-NEGOTIATE-OK" "grep -q 'M224A-NEGOTIATE-OK fails=0' $W/A.vm.out"
chk "A/C  M224A-NEGOTIATE-OK" "grep -q 'M224A-NEGOTIATE-OK fails=0' $W/A.c.out"
chk "A 双轨 stdout 逐字节一致" "cmp -s $W/A.vm.out $W/A.c.out"

echo "=== [2] 用例 B（opts{gzip:false} 服务级开关）双轨"
build_run gzswitch vm B.vm; rcb1=$?
build_run gzswitch c  B.c;  rcb2=$?
chk "B/VM rc=0" "[ $rcb1 -eq 0 ] && [ \"\$(cat $W/B.vm.rc)\" = 0 ]"
chk "B/C  rc=0" "[ $rcb2 -eq 0 ] && [ \"\$(cat $W/B.c.rc)\" = 0 ]"
chk "B/VM M224B-SWITCH-OK" "grep -q 'M224B-SWITCH-OK fails=0' $W/B.vm.out"
chk "B/C  M224B-SWITCH-OK" "grep -q 'M224B-SWITCH-OK fails=0' $W/B.c.out"
chk "B 双轨 stdout 逐字节一致" "cmp -s $W/B.vm.out $W/B.c.out"

echo "=== [3] 静态判据（新判据在位 + 旧判据已消失 ⇒ 防回退）"
chk "新判据 px_ae_accepts_gzip 在位" "grep -q 'static int px_ae_accepts_gzip' $RT"
chk "新判据 px_q_is_zero 在位"        "grep -q 'static int px_q_is_zero' $RT"
chk "服务级开关 g_px_gzip_enabled 在位" "grep -q 'int g_px_gzip_enabled = 1;' $RT"
chk "opts 解析 gzip 键在位"           "grep -q 'px_dict_get(args\[3\], \"gzip\")' $RT"
chk "旧裸 strstr 判据已消失"          "! grep -q 'strstr(ae.as.obj->as.str.data, \"gzip\")' $RT"
chk "Content-Type 匹配已改 strcasestr" "grep -q 'strcasestr(ct, \"json\")' $RT"
chk "三处调用共用同一判定（调用点数=3）" "[ \$(grep -c 'px_resp_gzipable(&' $RT) -eq 3 ]"

if [ $NEG -eq 1 ]; then
echo "=== [4] 负控 A：AE 判据退回裸 strstr ⇒ 用例 A 必须变红"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = 'if (!px_ae_accepts_gzip(ae.as.obj->as.str.data)) return 0;'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(
    s.replace(old, 'if (!strstr(ae.as.obj->as.str.data, "gzip")) return 0;   // NC-A'))
PYEOF
build_run gzneg vm NEG.A >/dev/null
if ! grep -q 'fails=0' "$W/NEG.A.out" 2>/dev/null; then
    chk "NC-A 判红（q=0 / 子串误命中复现）" "true"
else
    chk "NC-A 判红（q=0 / 子串误命中复现）" "false"
fi
restore_all
chk "NC-A 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
build_run gzneg vm POS.A >/dev/null
chk "NC-A 还原后复绿" "grep -q 'M224A-NEGOTIATE-OK fails=0' $W/POS.A.out"

echo "=== [5] 负控 B：抽掉服务级开关判据 ⇒ 用例 B 必须变红"
restore_all
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '    if (!g_px_gzip_enabled) return 0;'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(
    s.replace(old, '    if (0 && !g_px_gzip_enabled) return 0;   // NC-B'))
PYEOF
build_run gzswitch vm NEG.B >/dev/null
if ! grep -q 'fails=0' "$W/NEG.B.out" 2>/dev/null; then
    chk "NC-B 判红（开关被架空）" "true"
else
    chk "NC-B 判红（开关被架空）" "false"
fi
restore_all
chk "NC-B 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
build_run gzswitch vm POS.B >/dev/null
chk "NC-B 还原后复绿" "grep -q 'M224B-SWITCH-OK fails=0' $W/POS.B.out"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M224-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
