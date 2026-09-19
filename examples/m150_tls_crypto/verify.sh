#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M150 验证：摘要 / 密钥派生族（md5 / md5_bytes / pbkdf2_sha256）
#            + TLS 客户端族（tls_connect / tls_send / tls_recv / tls_close）
# ---------------------------------------------------------------
# 门做六件事：
#   ① 生成语料（gen_corpus.py，确定性）→ corpus.txt（md5 / pbkdf2 / PostgreSQL md5 认证）
#   ② Go 本尊真值（truth/main.go，**只用标准库**）→ 195 行
#   ③ VM 轨 + C 轨各跑一遍 fidelity.px ⇒ 与 Go 真值**逐字节** diff
#   ④ TLS 客户端：受控 Python TLS 服务端（自签证书）+ PuXian 探针 + **Go crypto/tls 探针**
#        · 两边的"实现无关子集"逐字节一致（版本号常量 / 收发字节 / EOF / 失败可辨别）
#        · PuXian 自报的 cipher_id 交给 Go 的 `tls.CipherSuiteName(id)`，
#          必须与 PuXian 自报的 cipher **逐字节相同**（文本口径 = Go 口径）
#        · 服务端日志里必须出现 PuXian 协商的套件名 **与** 显式 servername（SNI 真的发了）
#        · verify=true 对自签证书必须失败（= libpq 的 require 与 Go 的 InsecureSkipVerify 之别）
#   ⑤ 解释轨冒烟（interp_smoke.px）：名册 + "永不杀进程"
#   ⑥ 负控 A/B/C：把三处关键语义分别退回错误语义 ⇒ 门**必须变红**
#        A：md5 只取 15 字节做 hex（长度与值都错）
#        B：pbkdf2 的 iters<1 钳到 2（而不是 1）
#        C：TLS 把 verify 缺省值改成"校验证书"（require 语义丢失 ⇒ 自签证书必红）
#
# 口径：
#   · 服务端在**本机回环**上起（自签证书随示例提交；证书用 openssl 现场校验有效性）
#     ⇒ 不依赖外网，CI 可跑。
#   · 断言里凡涉及时间只取**宽上下界**（读超时面按 >= 250ms 判）。
#   · 端口从 19761 起找第一个空闲（可用 M150_PORT_OVERRIDE 固定）。
# 用法：bash examples/m150_tls_crypto/verify.sh
# 退出码：0 = M150-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
PX=../../tools/px
RT=../../runtime/runtime.c
LOG=/tmp/m150_build.log
FAIL=0
mkdir -p build
# ⚠️ 负控会改 runtime.c：**开头快照**，末尾必须逐字节还原。
#   （不能用 `git diff` 判 —— 门跑的时候 runtime.c 本来就有本轮的未提交改动。）
cp "$RT" /tmp/m150_rt_snapshot.c

# ── 缺陷 139（第 33 轮）：负控会改写 runtime/*.c；门**被打断**时篡改态会静默留在工作区 ──
#   （语法合法、语义反向 ⇒ 编译器不报错；而本轮 runtime.c 本来就带未提交改动 ⇒ `git diff` 判不出来。）
#   两道防线：① 开门先查「负控残留标记 NEGCTL」；② 信号兜底还原快照。
for _f in ../../runtime/runtime.c ../../runtime/vm.c; do
    if [ -f "$_f" ] && grep -q 'NEGCTL' "$_f" 2>/dev/null; then
        echo "FAIL 负控残留：$_f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
        exit 1
    fi
done

restore_rt() { [ -f /tmp/m150_rt_snapshot.c ] && cp /tmp/m150_rt_snapshot.c "$RT" 2>/dev/null; }
trap restore_rt INT TERM HUP

need_gcc_note() { echo "   （编译器：$(gcc --version 2>/dev/null | head -1)）"; }

# ── 端口：找第一个空闲 ──
PORT=""
if [ -n "${M150_PORT_OVERRIDE:-}" ]; then
    PORT="$M150_PORT_OVERRIDE"
else
    for p in $(seq 19761 19820); do
        if python3 -c "
import socket,sys
s=socket.socket(); s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
try:
    s.bind(('127.0.0.1', $p))
except OSError:
    sys.exit(1)
s.close()
" 2>/dev/null; then PORT=$p; break; fi
    done
fi
[ -n "$PORT" ] || { echo "FAIL 找不到空闲端口（19761-19820）"; exit 1; }
echo "== [配置] 回环端口 $PORT =="
need_gcc_note

# ── ① 语料 ──
echo "== [①] 生成语料 =="
python3 gen_corpus.py || { echo "FAIL 语料生成失败"; exit 1; }
N_CORPUS=$(grep -vc '^#' corpus.txt)
echo "   corpus.txt: $N_CORPUS 条"
if [ "$N_CORPUS" -lt 120 ]; then echo "FAIL 语料条数 $N_CORPUS < 120"; exit 1; fi

# ── ② Go 真值 ──
echo "== [②] Go 本尊真值（crypto/md5 + crypto/hmac·sha256 手写 PBKDF2）=="
# 口径与 m142/m148 一致：go.mod 钉 go 1.21（CI 预装 Go 版本不确定），**先 build 再跑**
# （`go run` 每次都要走一遍构建/缓存查找，且 `--csname` 那条路会被反复调用）。
if ! ( cd truth && GOFLAGS=-mod=mod go build -o ../build/truth . > "$LOG" 2>&1 ); then
    echo "FAIL Go 真值编译失败"; tail -12 "$LOG" | sed 's/^/   /'; exit 1
fi
if ! ./build/truth corpus.txt > /tmp/m150_truth.out 2>/tmp/m150_truth.err; then
    echo "FAIL Go 真值运行失败"; tail -5 /tmp/m150_truth.err | sed 's/^/   /'; exit 1
fi
if ! ( cd truth_tls && GOFLAGS=-mod=mod go build -o ../build/truth_tls . > "$LOG" 2>&1 ); then
    echo "FAIL Go TLS 探针编译失败"; tail -12 "$LOG" | sed 's/^/   /'; exit 1
fi
N_TRUTH=$(wc -l < /tmp/m150_truth.out)
echo "   真值 $N_TRUTH 行"
[ "$N_TRUTH" -ge 190 ] || { echo "FAIL 真值行数 $N_TRUTH < 190"; exit 1; }

# ── ③ VM / C 双轨 ──
SRV_PID=""
stop_server() {
    if [ -n "$SRV_PID" ]; then
        kill -TERM "$SRV_PID" 2>/dev/null || true
        sleep 0.2; kill -KILL "$SRV_PID" 2>/dev/null || true
        wait "$SRV_PID" 2>/dev/null || true
        SRV_PID=""
    fi
}
trap 'stop_server' EXIT
start_server() {
    rm -f /tmp/m150_srv.log
    setsid nohup python3 tls_server.py "$PORT" /tmp/m150_srv.log > /tmp/m150_srv.out 2>&1 &
    SRV_PID=$!
    local i=0
    while [ $i -lt 50 ]; do
        if grep -q '^READY' /tmp/m150_srv.out 2>/dev/null; then return 0; fi
        sleep 0.1; i=$((i + 1))
    done
    echo "FAIL TLS 服务端未就绪"; cat /tmp/m150_srv.out 2>/dev/null | sed 's/^/   /'; return 1
}

build_one() {
    if [ "$2" = "c" ]; then
        env PX_BUILD_ENGINE=c "$PX" build "$1" > "$LOG" 2>&1
    else
        "$PX" build "$1" > "$LOG" 2>&1
    fi
}

run_track() {
    local t="$1" out="/tmp/m150_$1.out" tout="/tmp/m150_$1.tls.out" rc=0
    echo "== [③/$t 轨] 编译 fidelity.px + tls_probe.px =="
    build_one fidelity.px "$t"  || { echo "FAIL [$t] fidelity 编译失败"; tail -12 "$LOG" | sed 's/^/   /'; FAIL=$((FAIL+1)); return; }
    build_one tls_probe.px "$t" || { echo "FAIL [$t] tls_probe 编译失败"; tail -12 "$LOG" | sed 's/^/   /'; FAIL=$((FAIL+1)); return; }

    timeout 120 ./build/fidelity > "$out" 2>&1; rc=$?
    if [ "$rc" != "0" ]; then echo "FAIL [$t] fidelity rc=$rc"; tail -15 "$out" | sed 's/^/   /'; FAIL=$((FAIL+1)); return; fi
    # 语料段（去掉 PASS/FAIL/M150_ 行）必须与 Go 真值逐字节一致
    awk '!/^(PASS|FAIL|M150_)/' "$out" > "/tmp/m150_$1.corpus"
    if diff -q /tmp/m150_truth.out "/tmp/m150_$1.corpus" > /dev/null; then
        echo "   PASS [$t] 语料段 $N_TRUTH 行与 Go 真值逐字节一致"
    else
        echo "FAIL [$t] 语料段与 Go 真值有差异（前 8 行）："
        diff /tmp/m150_truth.out "/tmp/m150_$1.corpus" 2>/dev/null | head -8 | sed 's/^/   /'
        FAIL=$((FAIL+1)); return
    fi
    local np nf
    np=$(grep -c '^PASS ' "$out" || true); nf=$(grep -c '^FAIL ' "$out" || true)
    echo "   断言：$np PASS / $nf FAIL"
    if ! grep -qE '^M150_ASSERT: [0-9]+P/0F$' "$out"; then
        echo "FAIL [$t] 断言未全绿："
        grep -E '^(FAIL |M150_ASSERT)' "$out" | sed 's/^/   /'
        FAIL=$((FAIL+1)); return
    fi
    if [ "$np" -lt 24 ]; then echo "FAIL [$t] 通过项数 $np < 24（防门空转）"; FAIL=$((FAIL+1)); return; fi

    # TLS 探针（同一服务端）
    timeout 60 env M150_TLS_PORT="$PORT" ./build/tls_probe > "$tout" 2>&1; rc=$?
    if [ "$rc" != "0" ] || ! grep -qE '^M150_TLS: [1-9][0-9]*P/0F/0S$' "$tout"; then
        echo "FAIL [$t] TLS 探针未全绿（rc=$rc）："
        grep -E '^(FAIL |M150_TLS)' "$tout" | sed 's/^/   /'
        FAIL=$((FAIL+1)); return
    fi
    local tn
    tn=$(grep -c '^TLS|' "$tout" || true)
    echo "   PASS [$t] TLS 探针 $(grep -E '^M150_TLS' "$tout")（$tn 行 TLS| 输出）"
    if [ "$tn" -lt 14 ]; then echo "FAIL [$t] TLS 输出行数 $tn < 14"; FAIL=$((FAIL+1)); return; fi
    echo "   PASS [$t] M150_ASSERT $(grep -E '^M150_ASSERT' "$out")"
}

start_server || FAIL=$((FAIL+1))
[ "$FAIL" = "0" ] && { run_track vm; run_track c; }
stop_server

# ── ③b 两轨逐字节一致 ──
echo "== [③b] VM / C 两轨输出逐字节一致 =="
if [ -f /tmp/m150_vm.out ] && [ -f /tmp/m150_c.out ] && diff -q /tmp/m150_vm.out /tmp/m150_c.out > /dev/null; then
    echo "   PASS fidelity 输出两轨一致"
else
    echo "FAIL fidelity 两轨不一致："; diff /tmp/m150_vm.out /tmp/m150_c.out 2>/dev/null | head -8 | sed 's/^/   /'; FAIL=$((FAIL+1))
fi
if [ -f /tmp/m150_vm.tls.out ] && [ -f /tmp/m150_c.tls.out ] \
   && diff -q /tmp/m150_vm.tls.out /tmp/m150_c.tls.out > /dev/null; then
    echo "   PASS TLS 探针输出两轨一致"
else
    echo "FAIL TLS 探针两轨不一致："; diff /tmp/m150_vm.tls.out /tmp/m150_c.tls.out 2>/dev/null | head -8 | sed 's/^/   /'; FAIL=$((FAIL+1))
fi

# ── ④ TLS 交叉对拍 ──
# ⚠️ 这里必须**重新起服务端**：run_track 之后已 stop_server，而 Go 探针还要连同一个服务端
#   （第一版忘了这一步 ⇒ Go 探针连不上、`exit status 1`，看起来像"Go 侧失败"）。
echo "== [④] TLS：PuXian ↔ Go crypto/tls ↔ 服务端三方对齐 =="
# ⚠️ start_server 会 rm 掉服务端日志；而 SNI / 套件的判据来自**PuXian 探针那一轮**的连接
#   ⇒ 重启前先把日志留一份（第一版没留，SNI 判据读到的是空日志，假红）。
[ -f /tmp/m150_srv.log ] && cp /tmp/m150_srv.log /tmp/m150_srv_probe.log
start_server || FAIL=$((FAIL+1))
PX_TLS=/tmp/m150_vm.tls.out
SRVLOG=/tmp/m150_srv_probe.log
[ -f "$SRVLOG" ] || SRVLOG=/tmp/m150_srv.log
if [ -f "$PX_TLS" ]; then
    # 实现无关子集（键名归一：PuXian 的 peer_port ↔ Go 的 insecure_peer_port）
    sub() { grep -E '^TLS\|(insecure_ok|insecure_version_num|peer_port|insecure_peer_port|send_ok|recv_ok|recv_data|recv_n|after_bye_n|eof|close_ok|verify_ok)\|' "$1" \
            | sed -e 's/^TLS|peer_port|/TLS|insecure_peer_port|/' -e 's/^TLS|insecure_peer_port|/TLS|peer_port|/' ; }
    if ./build/truth_tls "127.0.0.1:$PORT" > /tmp/m150_tls_go.out 2>/tmp/m150_tls_go.err; then
        sub "$PX_TLS" > /tmp/m150_px.sub
        sub /tmp/m150_tls_go.out > /tmp/m150_go.sub
        if diff -q /tmp/m150_px.sub /tmp/m150_go.sub > /dev/null; then
            echo "   PASS PuXian 与 Go crypto/tls 在**同一服务端**上：实现无关子集逐字节一致（$(wc -l < /tmp/m150_px.sub) 行）"
        else
            echo "FAIL PuXian 与 Go 的 TLS 子集不一致："
            diff /tmp/m150_px.sub /tmp/m150_go.sub | head -10 | sed 's/^/   /'
            FAIL=$((FAIL+1))
        fi
        # Go 侧也要在 verify=true 上失败（语义对齐）
        grep -q '^TLS|verify_ok|false$' /tmp/m150_tls_go.out \
            && echo "   PASS Go 侧 verify=true 同样失败（两侧对自签证书的判定一致）" \
            || { echo "FAIL Go 侧 verify=true 竟然成功"; FAIL=$((FAIL+1)); }
    else
        echo "FAIL Go TLS 探针运行失败"; tail -5 /tmp/m150_tls_go.err | sed 's/^/   /'; FAIL=$((FAIL+1))
    fi
    # 套件文本口径 = Go 口径
    CID=$(awk -F'|' '/^TLS\|cipher_id\|/{print $3}' "$PX_TLS")
    CNAME=$(awk -F'|' '/^TLS\|cipher\|/{print $3}' "$PX_TLS")
    if [ -n "$CID" ] && [ -n "$CNAME" ]; then
        GONAME=$( ./build/truth --csname "$CID" 2>/dev/null )
        if [ "$GONAME" = "$CNAME" ]; then
            echo "   PASS cipher 文本口径 = Go（tls.CipherSuiteName($CID) = $CNAME）"
        else
            echo "FAIL cipher 文本口径与 Go 不一致：Go=[$GONAME] PuXian=[$CNAME]（id=$CID）"
            FAIL=$((FAIL+1))
        fi
    else
        echo "FAIL 未取到 PuXian 的 cipher/cipher_id"; FAIL=$((FAIL+1))
    fi
    # 服务端视角：套件名 + SNI
    if [ -f "$SRVLOG" ]; then
        if grep -q "cipher=$CNAME" "$SRVLOG"; then
            echo "   PASS 服务端日志里出现了 PuXian 协商的套件（$CNAME）"
        else
            echo "FAIL 服务端日志里没有 $CNAME："; sed 's/^/   /' "$SRVLOG" | head -6; FAIL=$((FAIL+1))
        fi
        if grep -q "sni=example.test" "$SRVLOG"; then
            echo "   PASS 显式 servername 真的作为 SNI 发出（服务端 sni_callback 收到 example.test）"
        else
            echo "FAIL 服务端未收到 SNI=example.test（servername 没生效）"; FAIL=$((FAIL+1))
        fi
        if grep -q "sni=127.0.0.1" "$SRVLOG"; then
            echo "FAIL IP 字面量**不该**发 SNI（RFC 6066），但服务端收到了 sni=127.0.0.1"; FAIL=$((FAIL+1))
        else
            echo "   PASS IP 字面量不发 SNI（与 RFC 6066 / Go hostnameInSNI 同口径）"
        fi
    else
        echo "FAIL 服务端日志缺失"; FAIL=$((FAIL+1))
    fi
else
    echo "FAIL 缺 PuXian TLS 输出"; FAIL=$((FAIL+1))
fi
stop_server

# ── ⑤ 解释轨冒烟（三条通路：解释器 run / VM / C）──
echo "== [⑤] 解释轨冒烟 interp_smoke.px =="
for mode in run vm c; do
    out="/tmp/m150_smoke_$mode.out"; rc=1
    case "$mode" in
        run) "$PX" run interp_smoke.px > "$out" 2>&1; rc=$? ;;
        vm)  build_one interp_smoke.px "" >/dev/null 2>&1 && { ./build/interp_smoke > "$out" 2>&1; rc=$?; } ;;
        c)   build_one interp_smoke.px c  >/dev/null 2>&1 && { ./build/interp_smoke > "$out" 2>&1; rc=$?; } ;;
    esac
    if [ "$rc" != "0" ] || ! grep -qE '^M150_SMOKE: [0-9]+P/0F$' "$out"; then
        echo "FAIL 解释轨冒烟（$mode）：rc=$rc"; tail -10 "$out" | sed 's/^/   /'; FAIL=$((FAIL+1))
    else
        echo "   PASS 冒烟 $mode：$(grep -E '^M150_SMOKE' "$out")"
    fi
done

# ── ⑥ 负控：篡改 runtime.c → 重建 → 门必须变红 ──
negctl() {
    local tag="$1" old="$2" new="$3" why="$4"
    # 缺陷 139：负控必须留下**可检测的标记** —— 门被打断时靠它识别残留篡改态
    new="$new  /* NEGCTL-150-${tag} */"
    echo "== [⑥ 负控 $tag] 篡改 runtime.c ⇒ 门必须变红（$why）=="
    cp "$RT" /tmp/m150_runtime_keep.c
    local pok=1
    OLD="$old" NEW="$new" python3 - <<'PY' || pok=0
import os
p = "../../runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old, new = os.environ["OLD"], os.environ["NEW"]
if s.count(old) != 1:
    raise SystemExit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
    if [ "$pok" != "1" ]; then
        echo "FAIL 负控 $tag：篡改未生效（锚点与源码不同步）"; FAIL=$((FAIL+1)); cp /tmp/m150_runtime_keep.c "$RT"; return
    fi
    local verdict="green"
    # ⚠️ TLS 负控需要服务端在场；且判据必须要求**至少 1 个 PASS** ——
    #   第一版把 `^M150_TLS: [0-9]+P/0F/0S$` 当"绿"，于是"根本没连上（0P/0F/0S）"
    #   被误判成"绿" ⇒ 负控 C 假绿（"没跑"必须与"跑绿了"分开）。
    start_server >/dev/null 2>&1
    if build_one fidelity.px "" >/dev/null 2>&1; then
        local o="/tmp/m150_neg_$tag.out"
        timeout 120 ./build/fidelity > "$o" 2>&1 || true
        awk '!/^(PASS|FAIL|M150_)/' "$o" > "/tmp/m150_neg_$tag.corpus"
        if ! diff -q /tmp/m150_truth.out "/tmp/m150_neg_$tag.corpus" > /dev/null; then verdict="red-corpus"; fi
        if ! grep -qE '^M150_ASSERT: [0-9]+P/0F$' "$o"; then verdict="red-assert"; fi
        # TLS 负控（C）额外判据：探针必须红
        if ! build_one tls_probe.px "" >/dev/null 2>&1; then
            verdict="red-build"
        else
            local to="/tmp/m150_neg_${tag}_tls.out"
            timeout 60 env M150_TLS_PORT="$PORT" ./build/tls_probe > "$to" 2>&1 || true
            if ! grep -qE '^M150_TLS: [1-9][0-9]*P/0F/0S$' "$to"; then verdict="red-tls"; fi
        fi
    else
        verdict="red-build"
    fi
    stop_server
    cp /tmp/m150_runtime_keep.c "$RT"
    if grep -q 'NEGCTL' "$RT"; then echo "FAIL 负控 $tag：runtime.c 还原后仍有 NEGCTL 标记"; FAIL=$((FAIL+1)); fi
    if [ "$verdict" = "green" ]; then
        echo "FAIL 负控 $tag 未判红（门没有真的在跑这条面）"
        FAIL=$((FAIL+1))
    else
        echo "   PASS 负控 $tag 判红（$verdict）"
    fi
}

negctl A \
'    char hex[33];
    bytes_to_hex(digest, 16, hex);
    return px_str(hex);' \
'    char hex[33];
    bytes_to_hex(digest, 15, hex);
    return px_str(hex);' \
'把 md5 的十六进制输出从 16 字节改成 15 字节（长度与值都错）'

negctl B \
'    if (iters < 1) iters = 1;' \
'    if (iters < 1) iters = 2;' \
'把 pbkdf2 的 iters<1 钳位从 1 改成 2（A9/A10 必红）'

negctl C \
'    } else {
        // libpq 的 require / Go 的 InsecureSkipVerify=true：加密但不验证身份
        mbedtls_ssl_conf_authmode(&s->conf, MBEDTLS_SSL_VERIFY_NONE);
    }' \
'    } else {
        px_ensure_cacert();
        mbedtls_ssl_conf_authmode(&s->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&s->conf, &g_cacert, NULL);
    }' \
'把 TLS 的 verify 缺省值改成"校验证书"（libpq 的 require 语义丢失 ⇒ 自签证书必红）'

# 篡改恢复后必须重新回到绿（否则说明"负控红"不是篡改导致的）
echo "== [⑥+] 负控恢复后回归（必须重新全绿）=="
if build_one fidelity.px "" >/dev/null 2>&1 && timeout 120 ./build/fidelity > /tmp/m150_post.out 2>&1 \
   && grep -qE '^M150_ASSERT: [0-9]+P/0F$' /tmp/m150_post.out \
   && diff -q /tmp/m150_truth.out <(awk '!/^(PASS|FAIL|M150_)/' /tmp/m150_post.out) > /dev/null; then
    echo "   PASS runtime.c 恢复后 fidelity 重新全绿"
else
    echo "FAIL 恢复后仍不绿（git 状态可能已被污染）"; FAIL=$((FAIL+1))
fi
if diff -q /tmp/m150_rt_snapshot.c "$RT" > /dev/null; then
    echo "   PASS runtime/runtime.c 与门开始时的快照逐字节一致（负控已完全还原）"
else
    echo "FAIL runtime/runtime.c 与快照不一致（负控未完全还原）"; FAIL=$((FAIL+1))
fi

echo "════════════════════════════════════════════"
if [ "$FAIL" = "0" ]; then
    echo "M150-VERIFY-OK"
    exit 0
fi
echo "M150-VERIFY-FAIL：失败 $FAIL 项"
exit 1
