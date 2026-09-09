#!/bin/bash
# ============================================================
# M96-S3 verify.sh —— offload 名单全集 + 错误回传 + GC 根面压力（D8-①）
# ------------------------------------------------------------
#   1) 名单本地可测全集对拍（直调基准 vs 协程 offload 逐项一致）：
#      http_get / http_post / http_unix / tcp(send+recv) / udp(send+recv) /
#      dns_lookup / ws(send+recv) —— NET-ALL-OK
#   2) 错误路径：网络失败 Err Result 正常回传（NET-ERR-OK）；px_error 参数错误 →
#      外包线程捕获 → 协程恢复重抛（带源位置）→ worker 隔离 → 宿主与其它协程继续
#      （ARG-UNREACHABLE 不得出现；OK-CORO-ALIVE 必出现）
#   3) offload GC 根面压力：150 offload http_get + 1850 分配协程 + 低阈值 precise GC
#      （挂起协程 off_task args/result 入精确根面；漏标 = UAF）→ 零崩全对
#   4) s3_* 系列：已纳入名单（无外部 S3 端点，本地不实网对拍 —— 记录在案）
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px；python3（本地 tcp/udp echo）。端口 18889/18901/18902/18903。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m96_s3"
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── build daemon + 测试程序（VM 轨）"
"$PX" build --no-quic "$DIR/m96_s3_daemon.px" >/tmp/m96s3_build.log 2>&1 || { echo "FAIL build daemon"; tail -5 /tmp/m96s3_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m96_net_cmp.px"   >>/tmp/m96s3_build.log 2>&1 || { echo "FAIL build net_cmp"; tail -5 /tmp/m96s3_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m96_err.px"       >>/tmp/m96s3_build.log 2>&1 || { echo "FAIL build err"; tail -5 /tmp/m96s3_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m96_gc.px"        >>/tmp/m96s3_build.log 2>&1 || { echo "FAIL build gc"; tail -5 /tmp/m96s3_build.log; exit 1; }
chk "build(VM 轨)" 0

# 本地 python tcp/udp echo（18901/18902）—— /dev/null 重定向：fork 的 echo 服务
#   不持有 verify.sh 的 stdout 管道（否则 exec 工具 WaitDelay 等管道关闭）
ECHO_PID=0
python3 - >/dev/null 2>&1 <<'PYEOF' &
import sys, socket, threading, os
# 内嵌 echo 服务（detach 子进程，父即退避免 exec 等待）
def tcp_srv():
    s = socket.socket(); s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', 18901)); s.listen(16)
    while True:
        c, a = s.accept()
        def h(c=c):
            try:
                while True:
                    d = c.recv(65536)
                    if not d: break
                    c.sendall(d)
            except Exception: pass
            finally:
                try: c.close()
                except Exception: pass
        threading.Thread(target=h, daemon=True).start()
def udp_srv():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', 18902))
    while True:
        d, a = s.recvfrom(65536); s.sendto(d, a)
if os.fork() == 0:
    os.setsid()
    threading.Thread(target=tcp_srv, daemon=True).start()
    udp_srv()
PYEOF
sleep 0.5

SRV_PID=0
trap 'if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi; pkill -f "m96s3_echo" 2>/dev/null' EXIT
PX_SERVE_WORKERS=2 PX_CORO_WORKERS=2 "$DIR/build/m96_s3_daemon" \
    >/tmp/m96s3_srv.log 2>&1 < /dev/null &
SRV_PID=$!
sleep 1.0
# 预检服务就绪（unix socket 首次连接可能触发）
B=$(curl -s --max-time 3 "http://127.0.0.1:18889/data" 2>/dev/null)
chk "http 服务就绪" $([ -n "$B" ] && echo 0 || echo 1)
curl -s --max-time 3 --unix-socket /tmp/m96s3.sock http://localhost/hi >/dev/null 2>&1
chk "unix 服务就绪" $?

echo "── 1) 名单本地可测全集对拍（offload vs 直调）"
timeout 60 env PX_CORO_WORKERS=4 "$DIR/build/m96_net_cmp" >/tmp/m96s3_net.out 2>&1
RC=$?
grep -q "NET-ALL-OK" /tmp/m96s3_net.out
chk "net_cmp 7 项全对（rc=$RC）" $?
[ "$RC" != "0" ] && cat /tmp/m96s3_net.out

echo "── 2) 错误路径：Result 回传 + px_error 隔离"
timeout 40 env PX_CORO_WORKERS=2 "$DIR/build/m96_err" >/tmp/m96s3_err.out 2>/tmp/m96s3_err.diag
RC=$?
grep -q "NET-ERR-OK" /tmp/m96s3_err.out
chk "网络失败 Err Result 回传（NET-ERR-OK）" $?
grep -q "OK-CORO-ALIVE" /tmp/m96s3_err.out
chk "px_error 隔离后宿主/其它协程继续（OK-CORO-ALIVE）" $?
if grep -q "ARG-UNREACHABLE" /tmp/m96s3_err.out; then
    chk "参数错误协程被隔离（无 ARG-UNREACHABLE）" 1
else
    chk "参数错误协程被隔离（无 ARG-UNREACHABLE）" 0
fi
grep -q "已隔离" /tmp/m96s3_err.diag
chk "错误重抛带源位置 + worker 隔离消息" $?
echo "  （外包线程无位置版 + 协程恢复带位置版各打印一次，语义 = 直调）"

echo "── 3) offload GC 根面压力：150 http_get + 1850 分配 + 低阈值 precise"
timeout 120 env PX_CORO_WORKERS=4 PX_OFFLOAD_MAX=8 PX_GC_PRECISE=1 PX_GC_THRESHOLD=1000 \
    "$DIR/build/m96_gc" >/tmp/m96s3_gc.out 2>/tmp/m96s3_gc.diag
RC=$?
grep -q "GC-OFFLOAD-ROOT-OK" /tmp/m96s3_gc.out
chk "GC 压力零崩全对（rc=$RC）" $?
[ "$RC" != "0" ] && { echo "--- gc out ---"; cat /tmp/m96s3_gc.out; echo "--- diag tail ---"; tail -5 /tmp/m96s3_gc.diag; }

kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; SRV_PID=0

echo ""
echo "════════ m96_s3: PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
