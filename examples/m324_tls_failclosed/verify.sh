#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M324 验证：TLS 客户端认证 fail-closed + IP 字面量的名字校验
#   缺陷 559（fail-open：CA 不可用 ⇒ 静默 VERIFY_NONE）
#   缺陷 560（IP 作 host ⇒ 只校链不校名）
#   缺陷 561（CA 加载失败无声）
# ---------------------------------------------------------------
# 门做四件事（全部本机回环，不连外网）：
#   ① 造三张证书：测试 CA / 叶子含 SAN IP:127.0.0.1 / 叶子只含 DNS（无 IP SAN）
#   ② 用**产品二进制**（tools/px build --full 的产物）跑四格矩阵：
#        A2「CA 正常 + 无 IP SAN 的证书」必须失败   ← 缺陷 560 的判据
#        A3「CA 全部不可用」必须失败                 ← 缺陷 559 的判据
#        A4「CA 不可用 + PX_TLS_INSECURE=1」必须成功 ← 显式降级通道仍在
#        A1「CA 正常 + 有 IP SAN」必须成功           ← 防误杀
#      A3/A4 在 **mount 命名空间内** 把三个系统 bundle 换成 /dev/null（不碰宿主 /etc）
#   ③ 负控 N1/N2（打桩 runtime.c → 重建 → 门**必须变红**）：
#        N1 让 px_tls_check_ip_name 直接 return 0 ⇒ A2 变绿（闸门没牙）
#        N2 把 helper 的 return -1 换回「静默 VERIFY_NONE」⇒ A3 变绿
#   ④ 末尾把 runtime.c 逐字节还原（sha256 对拍），并查 NEGCTL 残留
# 退出码：0 = M324-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
ROOT=$(cd ../.. && pwd)                      # 绝对路径：门内会 cd 到 ROOT 构建探针，相对路径会失效
. "$ROOT/selfhost/gate_lock.sh" || { echo "❌ 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
RT=$ROOT/runtime/runtime.c
PX=$ROOT/tools/px
FAIL=0
mkdir -p build

# 缺陷 573（M324 补 5）：本门用 sudo（root）跑，与 runner 侧普通用户门共用 /tmp。
#   固定名（/tmp/m326_gate_build.log / /tmp/pxc_gcc_err.log …）⇒ 文件属主 ≠ 当前 fsuid，
#   O_CREAT 打开既有外属主文件即 EACCES（fs.protected_regular=2 的 GitHub runner 实测）
#   ⇒ 被误报成「tools/px build 失败」。故门自身产物与工具侧日志一律落**本门独占目录**。
GATE_TMP=$(mktemp -d "${TMPDIR:-/tmp}/m326_gate.XXXXXX") || { echo "❌ mktemp -d 失败"; exit 2; }
LOG=$GATE_TMP/m326_gate_build.log
export TMPDIR_DIR_M324="$GATE_TMP"   # 仅供诊断打印，不改变外部行为

# 快照 + 还原（负控会改 runtime.c；不能用 git diff 判 —— 门跑时树本来就可能脏）
SNAP=$GATE_TMP/m326_rt_snapshot.c
cp "$RT" "$SNAP" || { echo "❌ runtime.c 快照失败"; exit 2; }
SNAP_SHA=$(sha256sum "$SNAP" | cut -d' ' -f1)
restore_rt() { [ -f "$SNAP" ] && cp "$SNAP" "$RT" 2>/dev/null; }
# 成功才清独占目录；失败留现场（/tmp/m326_gate.XXXXXX）
gate_tmp_clean() { [ "${FAIL:-0}" = 0 ] && [ -n "${GATE_TMP:-}" ] && rm -rf "$GATE_TMP"; return 0; }
# sudo 门（CI 用 root 跑）会留下 root 属主的门锁 ⇒ /tmp sticky 位让后续普通用户门删不掉，
# 整条 CI 被拖红（M324 post-tag CI 实测：m128 因 root 残留锁假红）。故把锁带走。
trap 'type stop_mocks >/dev/null 2>&1 && stop_mocks; restore_rt; type px_gate_lock_release >/dev/null 2>&1 && px_gate_lock_release' INT TERM HU; gate_tmp_clean' INT TERM HUP
trap 'type stop_mocks >/dev/null 2>&1 && stop_mocks; type px_gate_lock_release >/dev/null 2>&1 && px_gate_lock_release' EXI; gate_tmp_clean' EXIT

for _f in "$RT" "$ROOT/runtime/vm.c"; do
    if [ -f "$_f" ] && grep -q 'NEGCTL' "$_f" 2>/dev/null; then
        echo "FAIL 负控残留：$_f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
        exit 1
    fi
done

ck() {   # ck <名称> <条件>
    if [ "$2" = "0" ]; then echo "  ✅ $1"
    else echo "  ❌ $1"; FAIL=$((FAIL+1)); fi
}

# ── 找「full」rtcache（必须是含 runtime_ws.o 的那一类；缺 runtime.o 的空目录跳过）──
pick_cache() {
    local best="" m best_m=0
    for d in "$ROOT"/.rtcache/*/; do
        [ -f "$d/runtime.o" ] || continue
        [ -f "$d/runtime_ws.o" ] || continue          # --full 才有
        m=$(stat -c %Y "$d/runtime.o" 2>/dev/null || echo 0)
        if [ "$m" -gt "$best_m" ]; then best_m=$m; best="$d"; fi
    done
    [ -n "$best" ] && echo "${best%/}"
}
CACHE=$(pick_cache)
[ -n "$CACHE" ] || { echo "❌ 找不到 --full 的 rtcache（先 ./tools/px build --full examples/hello.px）"; exit 2; }
echo "── rtcache：$CACHE"

# ── ① 证书 ──
echo "── ① 造证书"
openssl req -x509 -newkey rsa:2048 -nodes -keyout build/ca.key -out build/ca.pem -days 3650 \
    -subj "/CN=M324 Test CA" -addext "basicConstraints=critical,CA:TRUE" 2>/dev/null
mk_leaf() {   # $1=名字 $2=san 内容 $3=CN
    openssl req -newkey rsa:2048 -nodes -keyout "build/$1.key" -out "build/$1.csr" -subj "/CN=$3" 2>/dev/null
    printf "subjectAltName=%s\nbasicConstraints=CA:FALSE\nextendedKeyUsage=serverAuth\n" "$2" > "build/$1.ext"
    openssl x509 -req -in "build/$1.csr" -CA build/ca.pem -CAkey build/ca.key -CAcreateserial \
        -out "build/$1.pem" -days 3650 -extfile "build/$1.ext" 2>/dev/null
}
mk_leaf ip  "IP:127.0.0.1"           "127.0.0.1"
mk_leaf dns "DNS:wrong.example.com"  "wrong.example.com"
[ -s build/ip.pem ] && [ -s build/dns.pem ] && [ -s build/ca.pem ] || { echo "❌ 证书生成失败"; exit 2; }

# ── ①.5 缺陷 573 回归判据：工具侧不得再用固定 /tmp 编译日志名 ──
#   判据为什么静态：动态方式（造一个外属主残留文件再 build）在 root 下被 CAP_DAC_OVERRIDE 放行，
#   本地复现不了；而 CI 的 root 门确实被拒 ⇒ 只有「源码里根本没有固定名」是可判定的。
echo "── ①.5 编译器 stderr 日志口径（缺陷 573）"
_NFIX=$(grep -vE '^[[:space:]]*#' "$PX" | grep -cE '/tmp/pxc_(gcc|rtcc)_err\.log|/tmp/px_refs_err\.log' || true)
ck "tools/px 不再写固定 /tmp 编译日志（命中 ${_NFIX:-0} 处，期望 0）" "$([ "${_NFIX:-0}" = 0 ] && echo 0 || echo 1)"
_NMK=$(grep -vE '^[[:space:]]*#' "$PX" | grep -c 'px_cc_err_log' || true)
ck "tools/px 走 px_cc_err_log 独占文件（命中 ${_NMK:-0} 处，期望 ≥5）" "$([ "${_NMK:-0}" -ge 5 ] && echo 0 || echo 1)"

# ── ② 产品二进制（未打桩）：构建探针 ──
echo "── ② 构建探针（产品路径：tools/px build --full）"
PROBEPX="$PWD/probe.px"
( cd "$ROOT" && "$PX" build --full "$PROBEPX" ) > "$LOG" 2>&1 || {
    # CI 红要能一眼定位：M324 post-tag CI 只 tail 5 行，结果只看到 ld 的 libc 链接警告，
    # 真正的死因被截在中间 ⇒ 白跑一轮 CI。所以把「日志规模 + 末尾 40 行 + 环境证据」一起落盘。
    echo "❌ tools/px build 失败（见 $LOG）"
    echo "     log : $(wc -l < "$LOG" 2>/dev/null || echo 0) 行 / $(stat -c %s "$LOG" 2>/dev/null || echo 0) B"
    echo "     env : uid=$(id -u)/$(id -un) gcc=$(gcc -dumpversion 2>/dev/null || echo none) cache=$(basename "$CACHE" 2>/dev/null)"
    echo "     disk: $(df -h "$ROOT" 2>/dev/null | tail -1)"
    echo "     mem : $(free -m 2>/dev/null | sed -n '2p')"
    tail -40 "$LOG" | sed 's/^/     | /'
    exit 2; }
PROBE="$PWD/build/probe"
[ -x "$PROBE" ] || { echo "❌ 探针未生成：$PROBE"; exit 2; }

# ── 服务端（两个叶子各一个；端口从 19861 起找空闲）──
free_port() { local p=$1; while ss -ltn 2>/dev/null | grep -q ":$p "; do p=$((p+1)); done; echo $p; }
P_IP=$(free_port 19861); P_DNS=$(free_port $((P_IP+1)))
# 缺陷 571：旧版 ( setsid openssl s_server ... & ) 从不回收 ⇒ 门结束后 mock 变孤儿（PPID 1）常驻占端口，
# 后续门（如 m131 硬编码 19871）撞上去就假红。改：记 PID + trap 里 stop_mocks 带走。
MOCK_PIDS=""
start_mock() {   # start_mock <端口> <证书> <私钥>
    local pid
    pid=$(setsid openssl s_server -quiet -accept "$1" -cert "$2" -key "$3" -www >/dev/null 2>&1 & echo $!)
    MOCK_PIDS="$MOCK_PIDS $pid"
}
stop_mocks() {
    local p port
    for p in $MOCK_PIDS; do kill "$p" 2>/dev/null; done
    for p in $MOCK_PIDS; do wait "$p" 2>/dev/null; done
    MOCK_PIDS=""
    # 兜底：按端口回收（setsid 万一又 fork 了一层，PID 对不上）
    for port in $P_IP $P_DNS; do
        [ -n "$port" ] || continue
        p=$(ss -ltnp 2>/dev/null | grep ":$port " | grep -o 'pid=[0-9]*' | head -1 | cut -d= -f2)
        [ -n "$p" ] && kill "$p" 2>/dev/null
    done
    return 0
}
start_mock "$P_IP"  build/ip.pem  build/ip.key
start_mock "$P_DNS" build/dns.pem build/dns.key
sleep 2

# run_probe <二进制> <端口> [env...]  → stdout
run_probe() {
    local bin=$1 port=$2; shift 2
    env "$@" PX_TLS_CA_FILE="$PWD/build/ca.pem" "$bin" "$port" 2>&1
}
# CA 全部不可用时（mount 命名空间；不碰宿主 /etc）
run_noca() {
    local bin=$1 port=$2; shift 2
    unshare -m bash -c '
        for f in /etc/pki/tls/certs/ca-bundle.crt /etc/ssl/certs/ca-certificates.crt /etc/ssl/cert.pem; do
            [ -e "$f" ] && mount --bind /dev/null "$f" 2>/dev/null
        done
        unset PX_TLS_CA_FILE
        env "${@:2}" PX_TLS_CA_FILE= "$0" "$1" 2>&1
    ' "$bin" "$port" "$@"
}

# ── ③ 四格矩阵（产品二进制）──
# ⚠️ A3/A4 需要 mount 命名空间（把系统 CA bundle 换 /dev/null）。**不可用时判红而非跳过** ——
#   否则「559 没被验证」会被写成绿（CRITERIA ③：判据必须有牙）。CI 上以 root 跑（sudo -E）。
if ! unshare -m true 2>/dev/null; then
    echo "  ❌ 前置不成立：本机 unshare -m 不可用（需 CAP_SYS_ADMIN）⇒ A3/A4 无法构造「CA 全不可用」场景"
    echo "     ⇒ 不跳过、不判绿：请以 root 运行（CI 用 sudo -E），否则缺陷 559 未被验证。"
    FAIL=$((FAIL+1))
    MNT_NS=0
else
    MNT_NS=1
fi
echo "── ③ 四格矩阵（mount ns=$MNT_NS）"
out_a1=$(run_probe "$PROBE" "$P_IP")
out_a2=$(run_probe "$PROBE" "$P_DNS")
out_a3=$(run_noca  "$PROBE" "$P_IP")
out_a4=$(run_noca  "$PROBE" "$P_IP" PX_TLS_INSECURE=1)
echo "$out_a1" | sed 's/^/    A1| /'
echo "$out_a2" | sed 's/^/    A2| /'
echo "$out_a3" | sed 's/^/    A3| /'
echo "$out_a4" | sed 's/^/    A4| /'
ck "A1 CA正常 + SAN=IP ⇒ 连接成功"                 "$(echo "$out_a1" | grep -c 'TLS_OK' | grep -q 1; echo $?)"
ck "A2 CA正常 + 无 IP SAN ⇒ 拒绝（缺陷 560 判据）"  "$(echo "$out_a2" | grep -q 'TLS_FAIL'; echo $?)"
ck "A3 CA 全部不可用 ⇒ 拒绝（缺陷 559 判据）"        "$(echo "$out_a3" | grep -q 'TLS_FAIL'; echo $?)"
ck "A4 CA不可用 + PX_TLS_INSECURE=1 ⇒ 成功（显式降级通道）" "$(echo "$out_a4" | grep -q 'TLS_OK'; echo $?)"
ck "A5 拒绝时给出 R1010/R1011 文案（缺陷 561）"      "$( { echo "$out_a2"; echo "$out_a3"; } | grep -qE 'R1010|R1011'; echo $?)"

# ── ④ 负控：打桩 → 重建探针 → 断言必须变绿（门变红）──
rebuild_with_patched_runtime() {   # $1=说明
    local RTD=$ROOT/runtime
    if ! gcc -c -O2 -pthread -I"$CACHE" -I"$RTD" -I"$RTD/mbedtls/include" -I"$RTD/third_party/miniz" \
         -o "build/runtime_patched.o" "$RT" >> "$LOG" 2>&1; then
        echo "❌ 负控：runtime.c 编译失败（见 $LOG）"; return 1
    fi
    local objs="" f
    for f in "$CACHE"/*.o; do
        [ "$(basename "$f")" = "runtime.o" ] && continue
        objs="$objs $f"
    done
    gcc -static -O2 -pthread -o "build/probe_neg" "build/probe.o" $objs "build/runtime_patched.o" \
        "$RTD/third_party/sqlite3/sqlite3.o" \
        "$RTD/mbedtls/lib/libmbedtls.a" "$RTD/mbedtls/lib/libmbedx509.a" "$RTD/mbedtls/lib/libmbedcrypto.a" \
        "$RTD/third_party/ngtcp2/lib/libngtcp2.a" "$RTD/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RTD/third_party/openssl/lib/libssl.a" "$RTD/third_party/openssl/lib/libcrypto.a" \
        "$RTD/third_party/zlib/lib/libz.a" -lm -ldl -lpthread >> "$LOG" 2>&1 || {
        echo "❌ 负控：链接失败（见 $LOG）"; return 1; }
    return 0
}

if [ "${1:-}" = "--neg-skip" ]; then
  echo "── ④ 负控：--neg-skip（CI 用）⇒ 跳过两道负控"
else
echo "── ④ 负控 N1：让 IP 名字校验直接放行"
cp "$SNAP" "$RT"
if grep -q 'static int px_tls_check_ip_name(mbedtls_ssl_context\* ssl, const char\* host) {' "$RT"; then
    python3 - "$RT" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
k = "static int px_tls_check_ip_name(mbedtls_ssl_context* ssl, const char* host) {\n"
assert s.count(k) == 1, "锚点不唯一"
s = s.replace(k, k + "    return 0;   // NEGCTL-M324-N1 关掉名字校验（负控；门必须变红）\n")
io.open(p, 'w', encoding='utf-8').write(s)
PY
    if rebuild_with_patched_runtime N1; then
        n1=$(run_probe "$PWD/build/probe_neg" "$P_DNS")
        echo "$n1" | sed 's/^/    N1| /'
        ck "N1 负控生效：关掉名字校验后 A2 变绿 ⇒ 判据有牙" "$(echo "$n1" | grep -q 'TLS_OK'; echo $?)"
    else
        ck "N1 负控可构建" "1"
    fi
else
    echo "  ❌ N1 锚点缺失（px_tls_check_ip_name 定义）"; FAIL=$((FAIL+1))
fi
cp "$SNAP" "$RT"

echo "── ④ 负控 N2：把 fail-closed 退回「静默 VERIFY_NONE」"
if grep -q 'return -1;' "$RT" && grep -q 'R1010: 无可用 CA 证书' "$RT"; then
    python3 - "$RT" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = ('    fprintf(stderr, "[px] R1010: 无可用 CA 证书 ⇒ 拒绝建立加密连接（%s）。如需临时降级，显式设置 PX_TLS_INSECURE=1。\\n",\n'
       '            site ? site : "?");\n    return -1;\n')
new = ('    /* NEGCTL-M324-N2 退回旧的 fail-open */\n'
       '    mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_NONE);\n'
       '    if (out_verify) *out_verify = 0;\n    return 0;\n')
assert s.count(old) == 1, "锚点不唯一"
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
    if rebuild_with_patched_runtime N2; then
        n2=$(run_noca "$PWD/build/probe_neg" "$P_IP")
        echo "$n2" | sed 's/^/    N2| /'
        ck "N2 负控生效：退回 fail-open 后 A3 变绿 ⇒ 判据有牙" "$(echo "$n2" | grep -q 'TLS_OK'; echo $?)"
    else
        ck "N2 负控可构建" "1"
    fi
else
    echo "  ❌ N2 锚点缺失"; FAIL=$((FAIL+1))
fi

fi   # --neg-skip 分支结束

# ── ⑤ 还原自证 ──
restore_rt
now_sha=$(sha256sum "$RT" | cut -d' ' -f1)
ck "R 还原：runtime.c 逐字节回到快照（sha256 对拍）" "$([ "$now_sha" = "$SNAP_SHA" ]; echo $?)"
ck "R 无 NEGCTL 残留"                               "$(grep -q 'NEGCTL' "$RT"; [ $? -ne 0 ]; echo $?)"

echo "════ 汇总：失败 $FAIL 项 ════"
[ "$FAIL" = "0" ] && { echo "M324-VERIFY-OK"; exit 0; }
echo "M324-VERIFY-FAIL"
exit 1
