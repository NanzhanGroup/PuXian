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
#   [5] 负控 A：**判据灵敏度 · 必崩侧** —— 注入一个必然 SIGSEGV 的替身进程，
#       崩溃检测**必须**报「已崩」（证明判据不是恒绿）。
#   [6] 负控 B：**判据灵敏度 · 必活侧** —— 注入一个必然存活并打 M235-SURVIVED 的
#       替身，检测**必须**报「存活」（证明判据不是恒崩）。
#       两侧合起来 = **双向**，同时排除「恒绿」「恒红」两种假象。
#       ⚠️ 形态沿革（M242 缺陷 406 追查 · 实测留证 /tmp/m242/exp_layer3.log）：
#          · 原形态「忠实撤回 M235 的引用计数」在 M240 后**不再崩**（认领闸门兜住）；
#          · 改「同时撤两层」仍**不崩**（实测 0/6）；
#          · 撤**三层**（引用计数 + 认领闸门 + close 里的 shutdown）实测仅 **1/6** 崩
#            ⇒ 竞态窗口已被多层防护收窄到**不可靠复现**。
#          ⇒ 「撤回修复」**不可作判据**（假绿风险 > 判据价值，违反«门不许假红假绿»）。
#          ⇒ 「修复是否必要」改由 [1] 静态层 + [8] 分配器配对 + 文档登记覆盖。
#   [6] 覆盖边界登记
#   [8] 分配器配对（缺陷 354，M235s1）：PxConn 由 `xmalloc` 创建 ⇒ 释放必须 `xfree`
#       （裸 `free(c)` 会让 px_serve 每关一条连接就 `free(): invalid pointer` ⇒ SIGABRT）
#
# 用法：verify.sh [--neg-skip] [--runs N]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
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
run_form1_once() {   # $1=tag $2=port $3=二进制（默认 ./build/ws_uaf_repro；M242 起可注入替身）
    local tag="$1" port="$2" bin="${3:-./build/ws_uaf_repro}"
    local log="$W/f1_$tag.log"
    : > "$log"
    # ⚠️ 必须 exec：否则 $! 是**子 shell**，kill 打不到服务本体（M201 同款教训）
    ( cd "$W" && exec env M235_CERT="$W/c.pem" M235_KEY="$W/k.pem" M235_PORT="$port" \
        "$bin" >> "$log" 2>&1 ) &
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
echo "[5] 负控 A：判据灵敏度 · **必崩侧**（注入必然 SIGSEGV 的替身 ⇒ 检测必须报「已崩」）"
# ------------------------------------------------------------
# 为什么不再是「撤回修复」：见门头 [5] 的形态沿革登记（撤三层实测仅 1/6 崩 ⇒ 不可靠复现）。
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ --neg-skip：跳过负控（CI 用）"
else
    cat > "$W/dead_probe" <<'EOS'
#!/bin/bash
# 替身：必然崩溃（且不打 M235-SURVIVED）
sleep 0.4
kill -SEGV $$
EOS
    chmod +x "$W/dead_probe"
    if run_form1_once "negA" "27001" "./dead_probe"; then
        bad "负控 A：替身必然崩溃，检测却报「存活」⇒ 判据无牙"
    else
        ok "负控 A：替身必然崩溃 ⇒ 检测正确报「已崩」（判据有牙）"
    fi
fi

# ------------------------------------------------------------
echo "[6] 负控 B：判据灵敏度 · **必活侧**（必然存活的替身 ⇒ 检测必须报「存活」）"
# ------------------------------------------------------------
# 与 [5] 合起来 = 双向：排除「恒绿」（[5] 抓）与「恒红」（本条抓）两种假象。
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ --neg-skip：跳过负控（CI 用）"
else
    cat > "$W/alive_probe" <<'EOS'
#!/bin/bash
echo "M235-SURVIVED"
sleep 30
EOS
    chmod +x "$W/alive_probe"
    if run_form1_once "negB" "27002" "./alive_probe"; then
        ok "负控 B：替身必然存活 ⇒ 检测正确报「存活」（判据不是恒判崩）"
    else
        bad "负控 B：替身必然存活，检测却报「已崩」⇒ 判据恒红（无鉴别力）"
    fi
    rm -f "$W/dead_probe" "$W/alive_probe"
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
  · ⚠️ **本门的动态面只覆盖 ws 路径**：M235 的**连带改动**（http 路径
    `px_conn_close(c); xfree(c);` → `px_conn_owner_free(c)`）不在本门内 ——
    而缺陷 354（裸 `free(c)`）恰恰**只在 HTTP 路径（px_serve）显形**。
    该面由既有门 **m173 / m176 / m180** 覆盖（它们当场判红，见 M235s1 报告）。
    ⇒ 教训：**改了哪个调用点，门就必须覆盖那个调用点**。
EOF
ok "覆盖边界已登记"

# ------------------------------------------------------------
echo "[8] 分配器配对：PxConn 由 xmalloc 创建 ⇒ 释放必须 xfree（缺陷 354）"
# ------------------------------------------------------------
# 缺陷 354（M235s1）：M235 把 http 路径的 `px_conn_close(c); xfree(c);` 改写成
#   `px_conn_owner_free(c)`，而新函数体里写的是 `free(c)` —— 对象来自 `xmalloc`，必须 `xfree`。
#   **实跑后果**：px_serve 每关一条连接即 `free(): invalid pointer` ⇒ SIGABRT（rc=134）
#   ⇒ m173 / m176 / m180 三个既有门当场判红（它们覆盖的就是 HTTP 路径）。
#   历史：`xmalloc` 返回的是「跳过大小头」的指针，对它 `free()` 必然崩 —— **M201 记过一次**，
#   这是第二次 ⇒ 升格为**通用铁律**：改释放点之前，先问「这个指针是谁分配的」。
# 判据（可判定、无推断）：① 分配点确为 xmalloc；② 两个释放点全部 xfree；③ 裸 free(c) 计数 0。
# ⚠️ 判据实现纪律（M223/M226）：**不要在 destroy 与 free 之间用固定行数窗口** ——
#   首版用 `grep -A1`，而修复时在两者之间插了注释 ⇒ 窗口落到注释行 ⇒ **裸 free(c) 也数成 0**
#   （假阴）。改为：去掉注释后做**空白无关**匹配，与插几行注释无关。
read -r N_XMALLOC N_BAD N_XFREE <<EOF
$(python3 - <<'PY'
import re
s = open("runtime/runtime.c", encoding="utf-8").read()
s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
s = re.sub(r"//[^\n]*", "", s)
print(s.count("xmalloc(sizeof(PxConn))"),
      len(re.findall(r"pthread_mutex_destroy\(&c->mu\);\s*free\(c\);", s)),
      len(re.findall(r"pthread_mutex_destroy\(&c->mu\);\s*xfree\(c\);", s)))
PY
)
EOF
[ "$N_XMALLOC" -ge 1 ] \
    && ok "PxConn 分配点确为 xmalloc（$N_XMALLOC 处 —— 判据前提成立）" \
    || bad "PxConn 分配点不是 xmalloc ⇒ 本层判据前提失效（须重新推导）"
[ "$N_BAD" -eq 0 ] \
    && ok "两个对象释放点均无裸 free(c)" \
    || bad "仍有 $N_BAD 处裸 free(c) —— 对象来自 xmalloc，运行期必 abort"
[ "$N_XFREE" -ge 2 ] \
    && ok "xfree(c) 释放点 $N_XFREE 处（≥2：px_conn_release + px_conn_owner_free）" \
    || bad "xfree(c) 只 $N_XFREE 处（应为 2）⇒ 对象释放不配对"

echo
echo "结果：通过 $PASS / 失败 $FAIL"
[ "$FAIL" = "0" ] && { echo "M235-VERIFY-OK"; exit 0; } || { echo "M235-VERIFY-FAIL"; exit 1; }
