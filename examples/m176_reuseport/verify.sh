#!/usr/bin/env bash
# ============================================================
# M176 门 · 零停机换二进制（晨曦 QA 清单 P1-5 · SO_REUSEPORT）
# ------------------------------------------------------------
# 病灶：监听套接字只设 SO_REUSEADDR（TIME_WAIT 可复用），**两个进程无法同时监听同一端口**
#   ⇒ 每次换二进制必有空窗（晨曦三节点实测 352ms / 75ms / 249ms）。
# 修法：`opts{"reuse_port": true}`（px_serve / http_serve）或 `PX_REUSE_PORT=1`（全局，
#   覆盖 sse_serve / tcp_listen 这类无 opts 的原语）；失败**响亮报错**（内核不支持不假装设上）。
#   ⚠️ 内核要求**双方都 opt-in** ⇒ 升级链里旧版也必须带此开关启动（本门把这条**测出来**）。
#
# 判据：
#   [1] 起 A（reuse=1）→ 健康；起客户端高频探测（700 次）
#   [2] 起 B（reuse=1，同端口）→ **必须并存**（都能服务）
#   [3] SIGTERM A（优雅关闭）后 B 仍服务，且**客户端 refused 必须为 0**（这才是"零停机"）
#   [4] 反向语义：在 B 监听期间起 C（reuse=0）⇒ **bind 必须失败**（证明开关不是空操作，
#       也把「双方都要 opt-in」这条内核契约变成判据）
#   [5] 负控（源码）：把 px_sock_set_reuseport 变成 no-op ⇒ [2] 必须变红（B 起不来）
#       + sha256 逐字节还原复绿
#   [6] 重试路径自证：对面 accept 后立刻 RST（零字节）时，客户端必须**重试到 3 次上界**
#       再记 bad（retried=9 / bad=3 / RC=1） —— 不依赖竞态，证明 [3] 里的重试分支不是死代码
#
# ⚠️ 2026-11-06 [3] 判据变更（用户批准「方案 A」· 见 client.px 头注释）：
#   「关闭瞬间零丢失」经 M376 四组实验证明**不可达**（TCP：唯一能停止接收新连接的原语就是
#   close 本体；shutdown(SHUT_RDWR) 对 listening socket 返回 ENOTCONN）。故契约分层：
#     · refused == 0 —— 严格不放松（没有监听空窗，这才是零停机）
#     · 零字节即断 ⇒ 允许幂等 GET 重试（每请求至多 3 次），与浏览器/curl 真实行为一致
#     · 收到响应但非 200 / refused ⇒ 不重试，直接判红
#     · 防掩盖：retried 上界 n/100（700 次 ⇒ ≤7）；实测窗口恒丢 1 条（0.14%），留 7 倍余量
# CI 用 --neg-skip（负控要重编 runtime，~1min）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
RT="$ROOT/runtime/runtime.c"
D=examples/m176_reuseport
W=/tmp/m176_gate
BAK=/tmp/m176_runtime.bak
PORT=18311
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
# M376（缺陷 662）：失败时必须**自证现场** —— 只打一行 FAIL 等于让人去翻私有 tmp
#   （M375 那次就是靠 /data/gatevm/tmp4 残存才查到 bad=1）。判据不动，只补失败时的证据输出。
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1));
        if [ -n "${CURTAG:-}" ] && [ -f "$W/sc.$CURTAG.log" ]; then
            echo "    ↳ 现场（$W/sc.$CURTAG.log 尾 20 行）："; tail -20 "$W/sc.$CURTAG.log" | sed 's/^/      /'
        fi; fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; [ -n "${APID:-}" ] && kill -TERM $APID 2>/dev/null; [ -n "${BPID:-}" ] && kill -TERM $BPID 2>/dev/null; rm -f "$BAK"' EXIT

# ⚠️ 前置不变量：门会临时改 runtime.c 再还原 —— 若上一轮被 **SIGKILL**（trap 不执行）就会留下
#   脏源码，之后整轮「全绿/全红」都是假的。故进门先自查两条锚点文本都在（缺一即判门坏 rc=2）。
#   （教训：2026-09-22 本轮第一次跑门时被外部 kill，runtime.c 留在 NC 状态 ⇒ 下一次运行
#    「主场景」全红，看起来像功能坏了，其实是门没扫干净。）
grep -q 'if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) != 0)' "$RT" || { echo "❌ 前置自查失败：runtime.c 里没有 SO_REUSEPORT 设置锚点（上一轮被 kill 留下脏源码？）" >&2; exit 2; }
grep -q 'if (px_want_reuse_port(nargs >= 4 ? args\[3\] : px_null()))' "$RT" || { echo "❌ 前置自查失败：px_serve 未接 reuse_port 开关" >&2; exit 2; }

build_pair() {
    rm -rf "$D/build"
    (cd "$ROOT" && ./tools/px build "$D/srv.px"    > "$W/srv.build.log"    2>&1) || return 1
    (cd "$ROOT" && ./tools/px build "$D/client.px" > "$W/client.build.log" 2>&1) || return 1
    [ -x "$D/build/srv" ] && [ -x "$D/build/client" ]
}
health() { (timeout 2 bash -c "exec 3<>/dev/tcp/127.0.0.1/$1") >/dev/null 2>&1 && return 0 || return 1; }
wait_health() { local i=0; while [ $i -lt 60 ]; do health "$1" && return 0; sleep 0.05; i=$((i+1)); done; return 1; }
# ⚠️ 必须套 `timeout`：对端若保持连接（keep-alive）或端口无人监听时 `cat <&3` 会**永久阻塞**
#   —— 第一次实现在负控阶段就卡在这里（gate 挂了 6 分钟没人发现）。门里的每个「会等待」的
#   动作都要有**上界**。
fetch() { timeout 4 bash -c "exec 3<>/dev/tcp/127.0.0.1/$1; printf 'GET /index.html HTTP/1.1\r\nHost: t\r\nConnection: close\r\n\r\n' >&3; cat <&3" 2>/dev/null | tail -1; }
kill_wait() { local p=$1 i=0; kill -TERM $p 2>/dev/null; while [ $i -lt 60 ]; do kill -0 $p 2>/dev/null || return 0; sleep 0.05; i=$((i+1)); done; kill -9 $p 2>/dev/null; return 1; }

run_scenario() {   # $1=标签（把结论写进 $W/sc.$1.*）
    local tag=$1
    CURTAG=$tag
    : > "$W/sc.$tag.log"
    "$D/build/srv" $PORT 1 > "$W/sc.$tag.a.log" 2>&1 & APID=$!
    if ! wait_health $PORT 2; then echo "A-NOT-READY" >> "$W/sc.$tag.log"; kill -9 $APID 2>/dev/null; return 1; fi
    echo "A-READY $(fetch $PORT)" >> "$W/sc.$tag.log"
    "$D/build/client" $PORT 700 > "$W/sc.$tag.client.out" 2>&1 & CPID=$!
    sleep 0.6
    "$D/build/srv" $PORT 1 > "$W/sc.$tag.b.log" 2>&1 & BPID=$!
    sleep 1.0
    if kill -0 $BPID 2>/dev/null; then echo "B-ALIVE $(fetch $PORT)" >> "$W/sc.$tag.log"; else echo "B-DEAD" >> "$W/sc.$tag.log"; fi
    # 反向语义：C 不开 reuse_port
    "$D/build/srv" $PORT 0 > "$W/sc.$tag.c.log" 2>&1; echo "C-RC=$?" >> "$W/sc.$tag.log"
    tail -1 "$W/sc.$tag.c.log" >> "$W/sc.$tag.log"
    kill_wait $APID; echo "A-EXITED" >> "$W/sc.$tag.log"
    echo "AFTER-A-EXIT $(fetch $PORT)" >> "$W/sc.$tag.log"
    wait $CPID 2>/dev/null; echo "CLIENT-RC=$? $(cat $W/sc.$tag.client.out)" >> "$W/sc.$tag.log"
    kill_wait $BPID
    return 0
}

echo "=== [1]-[4] 零停机换二进制动态场景"
if ! build_pair; then
    chk "构建 srv/client" "false"
else
    run_scenario main
    cat "$W/sc.main.log" | sed 's/^/     /'
    chk "A 就绪（reuse=1）" "grep -q 'A-READY PID=' $W/sc.main.log"
    chk "B 与 A **并存**于同端口（reuse=1）" "grep -q 'B-ALIVE PID=' $W/sc.main.log"
    # A 与「A 退出后仍在服务的那个」必须是**两个不同进程** ——
    #   注意：B-ALIVE 的那次 fetch 可能仍被内核哈希到 A（同 4 元组 → 同 socket），
    #   故用「A 退出后仍在服务」的 pid 作 B 的身份（那时只可能是 B）。
    chk "A 与换上的 B 是两个不同进程" "[ \"\$(grep -o 'A-READY PID=[0-9]*' $W/sc.main.log | grep -o '[0-9]*')\" != \"\$(grep -o 'AFTER-A-EXIT PID=[0-9]*' $W/sc.main.log | grep -o '[0-9]*')\" ]"
    chk "C（reuse=0）bind **失败**（双方都要 opt-in）" "grep -q 'C-RC=1' $W/sc.main.log && grep -q 'EADDRINUSE\|Address already in use\|绑定端口' $W/sc.main.log"
    chk "A 优雅退出后 B 仍服务" "grep -q 'AFTER-A-EXIT PID=' $W/sc.main.log"
    chk "**零停机**：客户端 700 次 refused=0 bad=0（零字节可重试，有界）" "grep -q 'CLIENT-RC=0 CLIENT ok=700 refused=0 bad=0 retried=' $W/sc.main.log"
fi

if [ $NEG -eq 1 ]; then
echo "=== [5] 负控：px_sock_set_reuseport 变 no-op ⇒ B 必须起不来"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''    int one = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) != 0)
        px_error("%s: SO_REUSEPORT 设置失败（内核不支持？）：%s", what, strerror(errno));'''
assert s.count(old) == 1, s.count(old)
new = '''    (void)fd; (void)what;   // NC: SO_REUSEPORT 不再设置'''
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
build_pair >/dev/null 2>&1
run_scenario neg
chk "NC 判红（B 起不来）" "grep -q 'B-DEAD' $W/sc.neg.log"
restore_all
chk "NC 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
build_pair >/dev/null 2>&1
run_scenario pos
chk "NC 还原后复绿" "grep -q 'B-ALIVE PID=' $W/sc.pos.log && grep -q 'CLIENT-RC=0 CLIENT ok=700 refused=0 bad=0 retried=' $W/sc.pos.log"
fi

echo "=== [6] 重试路径自证（对面 accept 即 RST ⇒ 零字节 ⇒ 必须重试到上界再记 bad）"
# 目的：不让「零字节重试」变成没人跑过的死代码。低负载下竞态可能一次都不触发（retried=0），
#   那样 [3] 就只是「恰好没撞上窗口」，证明不了重试分支还在。
# 判据：n=3 ⇒ retried=9（每请求 3 次）、bad=3、ok=0、RC=1 —— 既证明重试生效，也证明**有界**。
if [ -x "$D/build/client" ]; then
    RPORT=18399
    python3 - "$RPORT" > "$W/sc.rst.log" 2>&1 <<'PYEOF' & RPID=$!
import os, socket, struct, sys, time
port = int(sys.argv[1])
# 可注入启动延迟（只给负控/复现用）：模拟满负载下解释器启动慢于固定等待
_d = float(os.environ.get("M176_FIXTURE_DELAY_MS", "0")) / 1000.0
if _d > 0:
    time.sleep(_d)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(("127.0.0.1", port)); s.listen(64)
# 只服务有限个连接（客户端最多 3 请求 × 3 次 = 9 次连接），避免测试吊住
for _ in range(40):
    try:
        c, _a = s.accept()
    except OSError:
        break
    # SO_LINGER{1,0} ⇒ close 发 RST，客户端一个字节都收不到（正是我们要构造的「零字节即断」）
    c.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))
    c.close()
s.close()
PYEOF
    # 就绪判据（缺陷 675）：固定 `sleep 0.3` 在 8 分片满负载下**不够** —— 夹具尚未 listen 时
    #   客户端第 1 条请求直接记 refused（M381 全量门实证：refused=1 bad=2 retried=6 ⇒ 假红）。
    #   改为有界探活：连得上即证明 listen 已就绪；15s 内连不上就判「夹具未就绪」并 rc=2，
    #   绝不让判据在夹具没起来的情况下说谎。探活那一条会占掉夹具 40 个 accept 中的 1 个（余量充足）。
    RDY=0
    for _ in $(seq 1 300); do
        if timeout 1 bash -c "exec 3<>/dev/tcp/127.0.0.1/$RPORT" 2>/dev/null; then RDY=1; break; fi
        sleep 0.05
    done
    if [ "$RDY" != 1 ]; then echo "❌ [6] 夹具未就绪（127.0.0.1:$RPORT 15s 内不可连）"; kill -TERM $RPID 2>/dev/null; exit 2; fi
    timeout 30 "$D/build/client" $RPORT 3 > "$W/sc.rst.client.out" 2>&1; RRC=$?
    kill -TERM $RPID 2>/dev/null; wait $RPID 2>/dev/null
    echo "     RST 夹具: RC=$RRC $(cat $W/sc.rst.client.out)" | sed 's/^ *//' >&2
    chk "重试路径生效且**有界**（3 请求 ⇒ retried=9 bad=3 ok=0）" \
        "[ \"$RRC\" = \"1\" ] && grep -q 'CLIENT ok=0 refused=0 bad=3 retried=9' $W/sc.rst.client.out"
else
    chk "重试路径自证（需要 client 构建产物）" "false"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M176-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
