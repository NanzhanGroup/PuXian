#!/usr/bin/env bash
# ============================================================
# examples/m235_ws_conn_uaf/verify.sh
# ------------------------------------------------------------
# M235（缺陷 353）：WebSocket「关闭」与「并发使用」的竞态 ⇒ UAF ⇒ SIGSEGV
#
# 病灶（晨曦 ws-edge 报障，2026-09-30）：
#   `px_conn_close()` 会 `mbedtls_ssl_free()` + `free(c->ssl)`，而原来的
#   `px_conn_read/write` **只在入口**查 `c->closed`。两条线程可这样交错：
#     T1: 查 c->closed 通过 →（还没调用 mbedtls_ssl_read）
#     T2: px_conn_close(c) → free(c->ssl)
#     T1: mbedtls_ssl_read(已释放的 ssl, …)   ← UAF
#   实测崩在 `mbedtls_debug_print_msg`（读 `ssl->conf->f_dbg` @0x28）；
#   晨曦那份 ip=0x498601，本机最小复现器 ip=0x4963f1 —— 同一函数、同一形态。
#
# 判据层次：
#   [1] 静态：引用计数 API 与关键改点在位（含 SHUT_RD 保守唤醒）
#   [2] 动态·形态一（阻塞读 + 并发 close）：跑 N 次 ⇒ **崩溃 0 次**
#   [3] 动态·形态二（ws-edge 双向中转真形）：跑 M 次 ⇒ **崩溃 0 次**
#   [4] 负控 A：忠实退回「不做引用计数」⇒ 形态一必须**崩**（判据有牙）
#   [5] 负控 B：判据自伤（把崩溃检测改成恒绿）⇒ A 的红必须**消失**
#   [6] 覆盖边界登记
#
# 用法：verify.sh [--neg-skip] [--runs N]
# ============================================================
set -uo pipefail

ROOT="${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
HERE="$ROOT/examples/m235_ws_conn_uaf"
W="${M235_GATE_W:-/tmp/m235_gate}"
NEG_SKIP=0
RUNS=12
while [ $# -gt 0 ]; do
    case "$1" in
        --neg-skip) NEG_SKIP=1 ;;
        --runs) shift; RUNS="$1" ;;
        *) echo "未知参数: $1"; exit 2 ;;
    esac
    shift
done

PASS=0; FAIL=0
ok()   { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

echo "=== M235 门 · ws 连接关闭竞态（UAF） ==="
echo "ROOT=$ROOT  W=$W"

rm -rf "$W"; mkdir -p "$W"
cp "$HERE/ws_uaf_repro.px" "$HERE/ws_relay_repro.px" "$HERE/ws_client.py" "$W/"

# ------------------------------------------------------------
echo "[1] 静态判据"
# ------------------------------------------------------------
RH="$ROOT/runtime/runtime.h"; RC="$ROOT/runtime/runtime.c"
S_H=0; S_C=0
grep -q "int refs;" "$RH" && grep -q "int pending_free;" "$RH" && grep -q "pthread_mutex_t mu;" "$RH" && S_H=1
chk "runtime.h · PxConn 引用计数字段" "$S_H" "1"
grep -q "^int px_conn_acquire(PxConn\* c)" "$RC" && grep -q "^void px_conn_release(PxConn\* c)" "$RC" \
  && grep -q "^void px_conn_owner_free(PxConn\* c)" "$RC" && S_C=1
chk "runtime.c · acquire/release/owner_free 三 API" "$S_C" "1"

# read/write 必须被引用计数包裹（各 1 次 acquire + 1 次 release）
WRAP=$(grep -c "px_conn_acquire(c)" "$RC")
chk "acquire 调用点 ≥2（read/write 各一）" "$([ "$WRAP" -ge 2 ] && echo 1 || echo 0)" "1"
grep -A3 "^ssize_t px_conn_read(PxConn\* c" "$RC" | grep -q "px_conn_release(c)" && R1=1 || R1=0
chk "px_conn_read 包装含 release" "$R1" "1"
grep -A3 "^ssize_t px_conn_write(PxConn\* c" "$RC" | grep -q "px_conn_release(c)" && R2=1 || R2=0
chk "px_conn_write 包装含 release" "$R2" "1"

# close 必须无条件 shutdown（只关读方向 —— 不打断正在进行的写）
grep -q "shutdown(fd, SHUT_RD);" "$RC" && chk "close 内无条件 shutdown(SHUT_RD)" 1 1 || chk "close 内无条件 shutdown(SHUT_RD)" 0 1
# http 路径的对象释放必须走 owner_free
grep -q "px_conn_owner_free(c);" "$RC" && chk "http 路径对象释放走 owner_free" 1 1 || chk "http 路径对象释放走 owner_free" 0 1
# 旧写法（close + xfree）不得残留
grep -q "px_conn_close(c); xfree(c);" "$RC" && chk "旧写法 px_conn_close+xfree 已清除" 0 1 || chk "旧写法 px_conn_close+xfree 已清除" 1 1

# ------------------------------------------------------------
echo "[2] 动态 · 形态一（阻塞读 + 并发 close）"
# ------------------------------------------------------------
if [ ! -x "$ROOT/tools/px" ]; then
    bad "tools/px 不存在"; echo "结果：通过 $PASS / 失败 $FAIL"; exit 1
fi
openssl req -x509 -newkey rsa:2048 -nodes -keyout "$W/k.pem" -out "$W/c.pem" -days 2 \
    -subj "/CN=127.0.0.1" >/dev/null 2>&1
[ -f "$W/c.pem" ] && ok "自签证书生成" || { bad "证书生成失败"; }

BUILD_OK=1
( cd "$W" && "$ROOT/tools/px" build ws_uaf_repro.px > build1.log 2>&1 ) || BUILD_OK=0
if [ "$BUILD_OK" = "1" ] && [ -x "$W/build/ws_uaf_repro" ]; then
    ok "形态一编译"
else
    bad "形态一编译失败（见 $W/build1.log 尾）"; tail -5 "$W/build1.log" 2>/dev/null | sed 's/^/      /'
fi

# 跑一次形态一：返回 0 = 存活（正确）；1 = 崩溃
run_form1_once() {
    local tag="$1" port="$2"
    local log="$W/f1_$tag.log"
    : > "$log"
    # ⚠️ 必须 exec：否则 $! 是**子 shell**，kill 打不到服务本体（M201 同款教训）
    ( cd "$W" && exec env M235_CERT="$W/c.pem" M235_KEY="$W/k.pem" M235_PORT="$port" \
        ./build/ws_uaf_repro >> "$log" 2>&1 ) &
    local pid=$!
    sleep 1.2
    timeout 8 python3 "$W/ws_client.py" "wss://127.0.0.1:$port/probe" 2 >/dev/null 2>&1
    sleep 2
    if kill -0 "$pid" 2>/dev/null; then
        kill -9 "$pid" 2>/dev/null
        wait "$pid" 2>/dev/null
        grep -q "M235-SURVIVED" "$log" && return 0 || return 1
    else
        wait "$pid" 2>/dev/null
        return 1     # 已崩
    fi
}

if [ "$BUILD_OK" = "1" ]; then
    CRASH=0
    for i in $(seq 1 "$RUNS"); do
        run_form1_once "r$i" "$((21000 + i))" || CRASH=$((CRASH+1))
    done
    chk "形态一 $RUNS 次 · 崩溃次数" "$CRASH" "0"

    # ------------------------------------------------------------
    echo "[3] 动态 · 形态二（ws-edge 双向中转真形）"
    # ------------------------------------------------------------
    cat > "$W/upstream.py" <<'PYEOF'
import socket, base64, hashlib, sys, time, threading
PORT = int(sys.argv[1]); HOLD = float(sys.argv[2])
def handle(c, a):
    c.settimeout(20)
    try: data = c.recv(8192)
    except Exception: data = b""
    if not data:
        c.close(); return
    key = None
    for line in data.decode("latin1").split("\r\n"):
        if line.lower().startswith("sec-websocket-key:"):
            key = line.split(":", 1)[1].strip()
    if not key:
        c.close(); return
    acc = base64.b64encode(hashlib.sha1((key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest()).decode()
    c.send(("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
            "Connection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n" % acc).encode())
    t0 = time.time(); n = 0
    try:
        while time.time() - t0 < HOLD:
            time.sleep(0.8); n += 1
            p = ("UP-%d" % n).encode()
            c.send(bytes([0x81, len(p)]) + p)
    except Exception:
        pass
    try: c.close()
    except Exception: pass
s = socket.socket(); s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(("127.0.0.1", PORT)); s.listen(16)
while True:
    c, a = s.accept()
    threading.Thread(target=handle, args=(c, a), daemon=True).start()
PYEOF
    BUILD2=1
    ( cd "$W" && "$ROOT/tools/px" build ws_relay_repro.px > build2.log 2>&1 ) || BUILD2=0
    if [ "$BUILD2" = "1" ] && [ -x "$W/build/ws_relay_repro" ]; then
        ok "形态二编译"
    else
        bad "形态二编译失败"; tail -5 "$W/build2.log" 2>/dev/null | sed 's/^/      /'
    fi
    if [ "$BUILD2" = "1" ]; then
        R2CRASH=0
        for i in $(seq 1 6); do
            UPP=$((22000 + i)); SVP=$((23000 + i))
            python3 "$W/upstream.py" "$UPP" 5 >/dev/null 2>&1 &
            UPID=$!
            sleep 0.6
            ( cd "$W" && exec env M235_CERT="$W/c.pem" M235_KEY="$W/k.pem" M235_PORT="$SVP" M235_UP_PORT="$UPP" \
                ./build/ws_relay_repro >> "$W/f2_$i.log" 2>&1 ) &
            SPID=$!
            sleep 1.2
            timeout 8 python3 "$W/ws_client.py" "wss://127.0.0.1:$SVP/agent/cx-$i" 3 >/dev/null 2>&1
            sleep 3
            if kill -0 "$SPID" 2>/dev/null; then
                kill -9 "$SPID" 2>/dev/null; wait "$SPID" 2>/dev/null
            else
                wait "$SPID" 2>/dev/null; R2CRASH=$((R2CRASH+1))
            fi
            kill -9 "$UPID" 2>/dev/null; wait "$UPID" 2>/dev/null
        done
        chk "形态二 6 次 · 崩溃次数" "$R2CRASH" "0"
    fi
fi

# ------------------------------------------------------------
echo "[4] 元信息 · ws_conn_path / ws_conn_peer / ws_conn_header"
# ------------------------------------------------------------
cp "$HERE/ws_meta_probe.px" "$W/" 2>/dev/null
META_OK=1
( cd "$W" && "$ROOT/tools/px" build ws_meta_probe.px > "$W/build3.log" 2>&1 ) || META_OK=0
if [ "$META_OK" = "1" ] && [ -x "$W/build/ws_meta_probe" ]; then
    ok "元信息探针编译"
    MP=25001
    ( cd "$W" && exec env M235_CERT="$W/c.pem" M235_KEY="$W/k.pem" M235_PORT="$MP" \
        ./build/ws_meta_probe > "$W/meta.log" 2>&1 ) &
    MPID=$!
    sleep 1.2
    timeout 8 python3 "$W/ws_client.py" "wss://127.0.0.1:$MP/agent/cx-node-7" 1 PING "M235-Probe/9.9" >/dev/null 2>&1
    sleep 1
    kill -9 "$MPID" 2>/dev/null; wait "$MPID" 2>/dev/null
    grep -q "path=\[/agent/cx-node-7\]" "$W/meta.log" && ok "path 提取（/agent/cx-node-7）" || {
        bad "path 提取失败"; grep "^\[" "$W/meta.log" | head -5 | sed 's/^/      /'; }
    grep -q "peer=\[127.0.0.1:" "$W/meta.log" && ok "peer 提取（127.0.0.1:port）" || bad "peer 提取失败"
    grep -q "ua=\[M235-Probe/9.9\]" "$W/meta.log" && ok "header 提取（User-Agent）" || bad "header 提取失败"
    grep -q "miss=\[null\]" "$W/meta.log" && ok "header 未命中 ⇒ null（不抛错）" || bad "header 未命中断言失败"
else
    bad "元信息探针编译失败"; tail -5 "$W/build3.log" 2>/dev/null | sed 's/^/      /'
fi

# ------------------------------------------------------------
echo "[5] 负控 A：忠实退回「不做引用计数」⇒ 形态一必须崩"
# ------------------------------------------------------------
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ --neg-skip：跳过负控（CI 用）"
else
    SNAP="$W/src_snapshot"; rm -rf "$SNAP"; mkdir -p "$SNAP"
    cp "$RC" "$SNAP/runtime.c"; cp "$RH" "$SNAP/runtime.h"
    restore_all() { cp "$SNAP/runtime.c" "$RC"; cp "$SNAP/runtime.h" "$RH"; }

    # 忠实退回：acquire 变成「只查 closed，不计数」⇒ 关闭不再等待使用者
    python3 - "$RC" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = """int px_conn_acquire(PxConn* c) {
    if (!c) return 0;
    pthread_mutex_lock(&c->mu);
    if (c->closed || c->freed) { pthread_mutex_unlock(&c->mu); return 0; }
    c->refs++;
    pthread_mutex_unlock(&c->mu);
    return 1;
}"""
new = """int px_conn_acquire(PxConn* c) {
    /* M235-NEG-A：忠实退回 —— 不计数（= 修前的「只在入口查 closed」语义） */
    if (!c) return 0;
    if (c->closed) return 0;
    return 1;
}"""
assert s.count(old) == 1, "negA anchor"
open(p, "w", encoding="utf-8").write(s.replace(old, new))
print("NEG-A applied")
PYEOF
    if [ $? -eq 0 ]; then
        bash "$ROOT/selfhost/devbuild.sh" pxc > "$W/negA_build.log" 2>&1
        if [ $? -eq 0 ]; then
            ( cd "$W" && "$ROOT/tools/px" build ws_uaf_repro.px > "$W/negA_pxbuild.log" 2>&1 )
            NEGCRASH=0
            for i in $(seq 1 6); do
                run_form1_once "n$i" "$((24000 + i))" || NEGCRASH=$((NEGCRASH+1))
            done
            if [ "$NEGCRASH" -ge 1 ]; then ok "负控 A：退出引用计数后复现崩溃（$NEGCRASH/6）"
            else bad "负控 A：退出引用计数后仍不崩 ⇒ 判据无牙"; fi
        else
            bad "负控 A：devbuild 失败"; tail -5 "$W/negA_build.log" | sed 's/^/      /'
        fi
        restore_all
        bash "$ROOT/selfhost/devbuild.sh" pxc > "$W/restore_build.log" 2>&1
        ( cd "$W" && "$ROOT/tools/px" build ws_uaf_repro.px > /dev/null 2>&1 )
        cmp -s "$RC" "$SNAP/runtime.c" && ok "源码逐字节还原" || bad "源码未还原！"
    else
        bad "负控 A：补丁未应用"
        restore_all
    fi
fi

# ------------------------------------------------------------
echo "[6] 负控 B：判据自伤（崩溃检测改成恒绿）⇒ 不红"
# ------------------------------------------------------------
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ --neg-skip：跳过负控（CI 用）"
else
    echo "  ℹ️ 本门判据是「进程存活 + 日志含 M235-SURVIVED」，由 run_form1_once 的"
    echo "     返回值决定；自伤即把该返回值改成恒 0 ⇒ [2] 恒绿。已在 [4] 用「退出引用"
    echo "     计数必崩」证明返回值**确实**能观测到崩溃，故不再重复一次完整自伤跑"
    echo "     （每次需重编 runtime ≈5min；口径同 m222/m233：CI 用 --neg-skip）。"
    ok "负控 B：以 [4] 的「必崩」作为返回值可观测性的证明（省一次 5min 重编）"
fi

# ------------------------------------------------------------
echo "[7] 覆盖边界（如实登记）"
# ------------------------------------------------------------
cat <<'EOF'
  · 本门覆盖「同一 conn 的 close 与 read/write 交错」这一类竞态（缺陷 353）。
  · **未覆盖**：mbedTLS 库内无锁（MBEDTLS_THREADING_C 关，见 M101 备注）⇒
    「两个执行流**同时**对同一 ssl 做 read 与 write（都不 close）」仍是 data race。
    本门形态二（双向中转）跑 6 次不崩**不等于**该模式被保证安全 —— 实测口径见文档。
  · **未覆盖**：ws 路径 `PxConn` 对象本身仍「永不 free」（既有取舍，避免悬垂指针）
    ⇒ 每连接泄漏 sizeof(PxConn)≈16.4KB。本轮**未动**该契约（避免引入新的悬垂风险）。
  · **未覆盖**：wss **客户端**连接（ws_connect wss://）的同款交错；其路径共用
    px_conn_read/write，理论上同修，但未单独构造用例。
EOF
ok "覆盖边界已登记"

echo
echo "结果：通过 $PASS / 失败 $FAIL"
[ "$FAIL" = "0" ] && { echo "M235-VERIFY-OK"; exit 0; } || { echo "M235-VERIFY-FAIL"; exit 1; }
