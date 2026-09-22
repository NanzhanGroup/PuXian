#!/usr/bin/env bash
# M180 门 · HTTP/2 的口径落地 + HTTP/3 的能力自证（第 58 轮 · 晨曦 QA 清单 P0-3 / P1-4）
# ------------------------------------------------------------
# 病灶（晨曦实测，本门在本地复现）：
#   ① h2c Upgrade 请求会进 `px_h2_handle`，而那个帧循环**不接 vhost/handler 管道**，
#      只回固定演示页（`<h1>PuXian HTTP/2</h1>`）⇒ 客户端拿到 **200 + 与本站无关的内容**
#      （最坏一类：静默给错值）。
#   ② ALPN 全线只声明 http/1.1（生产站点配置）⇒ h2 不可用（口径：**不做**）。
#   ③ h2 **prior-knowledge 前导**的检测本身就是**死代码**：判据写的是
#      `strncmp(target, "PRI * HTTP/2.0", 14)`，而 `target` 是请求行切出来的第二段（只剩 `*`）
#      ⇒ 真实行为是"当成普通 h1.1 ⇒ 404"（= 晨曦实测那条）。
#   ④ `--no-quic` 构建下 `opts{"http3": true}` 被**静默忽略**（H3 不开也不报错）。
#
# 口径（docs/HTTP2_DECISION.md，本门把口径变成判据）：
#   · `Upgrade: h2c` ⇒ **忽略升级，按 HTTP/1.1 正常服务**（拿到真实 docroot 内容）；
#   · h2 **prior-knowledge 前导** ⇒ **505 + 说明文本**（明确拒绝，不装样子）；
#   · h2 演示帧层收进 `opts{"h2_demo": true}`（默认关；只给 m35/m38 这类能力演示用）；
#   · H3 是唯一现代协议路线：x86_64 默认构建**链 ngtcp2**（能力可自证），
#     非 x86_64 自动 `--no-quic` ⇒ 该构建里要 H3 必须**响亮报错**。
#
# 判据：
#   [1] 口径 A：h2c 升级请求 ⇒ `HTTP/1.1 200` + 真实内容（`STANCE-DOC-OK`）+ **无演示页**
#       + 服务端 stderr 有「忽略 h2c」一行（响亮）
#   [2] 口径 B：prior-knowledge 前导 ⇒ `HTTP/1.1 505` + 说明（含「HTTP/2 未支持」）
#   [3] 口径 C：`--no-quic` 构建 + `opts.http3` ⇒ rc≠0 + stderr 含 R1002 + 「--no-quic」
#   [4] 能力自证：要求 H3 的程序（**默认构建**）产物 `strings` 含 `ngtcp2`/`Alt-Svc`；
#       `--no-quic` 产物**不**含 + 构建 stderr 能力行写 `quic=off`
#   [5] 负控（默认跑，`--neg-skip` 跳过）：把 h2c 分支改回「进演示帧层」⇒ [1] 必须判红
# 用法：./examples/m180_h2_h3_stance/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m180_h2_h3_stance
RT="$ROOT/runtime/runtime.c"
BAK_RT=/tmp/m180_runtime.bak
W=/tmp/m180_gate
DOC=/tmp/m180_stance_docroot
rm -rf "$W"; mkdir -p "$W" "$DOC"
printf 'STANCE-DOC-OK\n' > "$DOC/index.html"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

# ── 前置不变量自查
grep -q 'g_px_h2_demo' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M180 g_px_h2_demo 锚点" >&2; exit 2; }
grep -q 'HTTP/2 未支持（PuXian 口径：不做 h2）' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M180 505 文案锚点" >&2; exit 2; }

build() {   # $1=用例 $2=额外参数 $3=日志后缀
    local n=$1 extra=${2:-} tag=${3:-}
    rm -rf "$D/build"
    # 门需要「能力行」⇒ 显式打开（默认静默，避免污染 engine_parity 的"正例 stderr 必须为空"）
    (cd "$ROOT" && PX_BUILD_FEATURES=1 ./tools/px build $extra "$D/$n.px" > "$W/$n${tag}.build.log" 2>&1) && [ -x "$D/build/$n" ]
}
port_free() { (exec 3<>/dev/tcp/127.0.0.1/"$1") 2>/dev/null && { exec 3<&-; return 1; } || return 0; }
wait_port() { local p=$1 i; for i in $(seq 1 40); do port_free "$p" && return 0; sleep 0.25; done; return 1; }
wait_listen() { local p=$1 i; for i in $(seq 1 60); do port_free "$p" || return 0; sleep 0.25; done; return 1; }
stop_srv() { kill -9 "${1:-0}" 2>/dev/null; sleep 0.4; }

# ── [1][2] 服务器（默认口径）
echo "=== [1][2] 口径 A/B：h2c 升级被忽略（按 h1.1 服务）· prior-knowledge ⇒ 505"
build srv_stance "" "" || { echo "❌ 服务器构建失败（$W/srv_stance.build.log）" >&2; tail -5 "$W/srv_stance.build.log" >&2; exit 2; }
CP=18290
wait_port $CP || { echo "❌ 端口 $CP 被占" >&2; exit 2; }
PX_STANCE_PORT=$CP "$D/build/srv_stance" > "$W/srv.log" 2>&1 &
SRV_PID=$!
trap 'stop_srv ${SRV_PID:-0}; rm -rf "$D/build"' EXIT
wait_listen $CP || { echo "❌ 服务器未监听 $CP（日志见 $W/srv.log）" >&2; cat "$W/srv.log" >&2; exit 2; }
python3 "$D/h2_stance_client.py" $CP > "$W/client.out" 2>&1
sed 's/^/   /' "$W/client.out"
chk "A h2c 升级被忽略、按 h1.1 服务真实内容" "grep -q 'A_h2c_upgrade: OK' $W/client.out"
chk "B prior-knowledge ⇒ 505 + 说明" "grep -q 'B_prior_knowledge: OK' $W/client.out"
chk "A/B 汇总标记" "grep -q 'M180-H2STANCE-OK' $W/client.out"
chk "服务端 stderr 有「忽略 h2c」说明（响亮）" "grep -q '忽略 h2c 升级请求' $W/srv.log"
stop_srv $SRV_PID
wait_port $CP || echo "   (提示：端口 $CP 仍在监听)"
rm -rf "$D/build"

echo "=== [3] 口径 C：--no-quic 构建 + opts.http3 ⇒ 响亮报错（修前静默）"
build noquic_http3 "--no-quic" ".noquic" || { echo "❌ --no-quic 构建失败" >&2; exit 2; }
PX_STANCE_PORT=18291 "$D/build/noquic_http3" > "$W/noquic.out" 2> "$W/noquic.err"; rc=$?
chk "C rc≠0" "[ $rc -ne 0 ]"
chk "C stdout 无诊断（"before" 或已被 abort 吞掉）" "o=\$(cat $W/noquic.out); [ -z \"\$o\" ] || [ \"\$o\" = before ]"
chk "C stdout 不含错误码（诊断不混进产物通道）" "! grep -q 'R1002' $W/noquic.out"
chk "C stderr 含 R1002 + --no-quic 指引" "grep -q 'R1002' $W/noquic.err && grep -q -- '--no-quic' $W/noquic.err && grep -q 'opts.http3' $W/noquic.err"
chk "C 构建 stderr 能力行写 quic=off" "grep -q 'quic=off' $W/noquic_http3.noquic.build.log"
rm -rf "$D/build"

echo "=== [4] 能力自证：要求 H3 的程序在默认构建里链 ngtcp2；--no-quic 构建不链"
build noquic_http3 "" ".def" || { echo "❌ 默认构建失败" >&2; exit 2; }
DEF_BIN="$D/build/noquic_http3"
chk "默认构建产物含 ngtcp2（H3 能力在）" "strings -a $DEF_BIN | grep -q ngtcp2"
chk "默认构建产物含 Alt-Svc（H3 通告）" "strings -a $DEF_BIN | grep -q Alt-Svc"
chk "默认构建 stderr 能力行写 quic=on" "grep -q 'quic=on' $W/noquic_http3.def.build.log"
chk "默认构建产物**可执行**且能起 H3（跑一下看 listener 行）" "timeout 8 env PX_STANCE_PORT=18293 $DEF_BIN > $W/h3run.out 2> $W/h3run.err; grep -q 'HTTP/3 listening udp/18293' $W/h3run.err"
rm -rf "$D/build"
build noquic_http3 "--no-quic" ".noquic2" || exit 2
chk "--no-quic 产物**不含** ngtcp2" "! strings -a $D/build/noquic_http3 | grep -q ngtcp2"
rm -rf "$D/build"

if [ $NEG -eq 1 ]; then
echo "=== [5] 负控：h2c 分支改回「进演示帧层」⇒ [1] 必须判红（重现演示页）"
cp -f "$RT" "$BAK_RT"
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o = '#ifndef PX_NO_H2\n                if (g_px_h2_demo) {'
assert s.count(o) == 1, s.count(o)
s = s.replace(o, '#ifndef PX_NO_H2\n                if (1) {   // NC：默认进演示帧层（M180 修前的行为）')
open(p, 'w', encoding='utf-8').write(s)
PYEOF
build srv_stance "--full" ".nc" || echo "  (负控构建失败)"
NP=18292
wait_port $NP || echo "  (端口 $NP 被占)"
PX_STANCE_PORT=$NP "$D/build/srv_stance" > "$W/srv_nc.log" 2>&1 &
NC_PID=$!
wait_listen $NP || echo "  (负控服务器未监听)"
python3 "$D/h2_stance_client.py" $NP > "$W/client_nc.out" 2>&1
stop_srv $NC_PID
rm -rf "$D/build"
if grep -q 'A_h2c_upgrade: OK' "$W/client_nc.out"; then
    chk "NC 判红（A 仍 OK ⇒ 门没钉住口径）" "false"
else
    chk "NC 判红（A 失败 = 客户端又拿到演示页）" "grep -q 'A_h2c_upgrade: FAIL' $W/client_nc.out"
fi
cp -f "$BAK_RT" "$RT"; rm -f "$BAK_RT"
chk "NC 还原逐字节（runtime.c）" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M180-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
