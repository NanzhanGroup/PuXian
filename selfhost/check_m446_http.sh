#!/usr/bin/env bash
# ============================================================
# check_m446_http.sh —— 请求/响应框架的 RFC 硬约束（M446 · 缺陷 907/908 同族）
# ------------------------------------------------------------
# 为什么有它（2026-11-25 实测，修前）：
#   ① GET /index.html HTTP/1.1 **不带 Host** ⇒ 200（RFC 9112 §3.2 要求 400）
#      且在 http_serve 与 px_serve **两条轨**上都要守（修前只修一条仍会漏）
#   ② handler 返回 null（status=204）⇒ 仍写 "Content-Length: 0"（RFC 9110 §15.3.5 禁止）
#
# 判据：
#   C1 HTTP/1.1 缺 Host ⇒ 400（px_serve 轨）
#   C2 HTTP/1.0 缺 Host ⇒ 不要求（仍 200；防「一刀切」把 1.0 打死）
#   C3 204 ⇒ 响应头无 Content-Length 且无体
#   C4 200 的 Content-Length 与体**完好**（防修 204 时误伤正常响应）
#   N1 负控：把 C1 请求补上 Host ⇒ 必须 200（证明红的来源是「缺 Host」而不是别的）
#   G1 环境守卫：缺 tools/pxc 或 python3 ⇒ rc=3
#
# 用法：check_m446_http.sh [--repo DIR]
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) ROOT="$2"; shift 2 ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

[ -x "$ROOT/tools/pxc" ] || { echo "ENV: 缺 tools/pxc"; exit 3; }
command -v python3 >/dev/null 2>&1 || { echo "ENV: 缺 python3"; exit 3; }

TMP=$(mktemp -d /tmp/m446http.XXXXXX) || exit 3
PORT=$(python3 -c 'import socket;s=socket.socket();s.bind(("127.0.0.1",0));print(s.getsockname()[1]);s.close()')
trap '[ -f "$TMP/srv.pid" ] && kill "$(cat "$TMP/srv.pid")" 2>/dev/null; rm -rf "$TMP"' EXIT

ok_n=0; bad_n=0
ok()  { echo "✅ $*"; ok_n=$((ok_n+1)); }
bad() { echo "❌ $*"; bad_n=$((bad_n+1)); }

mkdir -p "$TMP/doc"
printf 'OK' > "$TMP/doc/index.html"

cat > "$TMP/srv.px" <<PX
PORT = $PORT
D = "$TMP/doc"
def n204(req, params):
    return null
route("GET", "/null", n204)
def serve():
    px_serve(PORT, D, 6000)
spawn serve()
sleep(25000)
PX

if ! "$ROOT/tools/pxc" build "$TMP/srv.px" > "$TMP/build.log" 2>&1; then
  bad "构建失败"; tail -6 "$TMP/build.log" | sed 's/^/     /'
  echo "M446-HTTP-OK pass=$ok_n fail=$bad_n"; exit 1
fi

setsid nohup "$TMP/build/srv" > "$TMP/srv.out" 2>&1 &
echo $! > "$TMP/srv.pid"
ready=0
for _ in $(seq 1 60); do
  if ss -lnt 2>/dev/null | grep -q ":$PORT "; then ready=1; break; fi
  sleep 0.2
done
if [ "$ready" != 1 ]; then
  bad "服务未起监听（端口 $PORT）"; head -5 "$TMP/srv.out" | sed 's/^/     /'
  echo "M446-HTTP-OK pass=$ok_n fail=$bad_n"; exit 1
fi

python3 - "$PORT" > "$TMP/res.txt" 2>&1 <<'PY'
import socket, sys
port = int(sys.argv[1])

def req(payload):
    s = socket.create_connection(("127.0.0.1", port), timeout=5)
    s.settimeout(3)
    s.sendall(payload)
    d = b""
    try:
        while True:
            c = s.recv(4096)
            if not c:
                break
            d += c
    except Exception:
        pass
    s.close()
    return d

def line(d):
    return d.split(b"\r\n")[0]

# C1
d1 = req(b"GET /index.html HTTP/1.1\r\nConnection: close\r\n\r\n")
print("C1", "OK" if b"400" in line(d1) else "FAIL", repr(line(d1)))
# C2
d2 = req(b"GET /index.html HTTP/1.0\r\n\r\n")
print("C2", "OK" if b"200" in line(d2) else "FAIL", repr(line(d2)))
# C3
d3 = req(b"GET /null HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
head3 = d3.split(b"\r\n\r\n")[0]
body3 = d3.split(b"\r\n\r\n", 1)[-1] if b"\r\n\r\n" in d3 else b"?"
ok3 = b"204" in line(d3) and b"Content-Length" not in head3 and body3 == b""
print("C3", "OK" if ok3 else "FAIL", repr(head3[:120]))
# C4
d4 = req(b"GET /index.html HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
head4 = d4.split(b"\r\n\r\n")[0]
body4 = d4.split(b"\r\n\r\n", 1)[-1]
ok4 = b"200" in line(d4) and b"Content-Length: 2" in head4 and body4 == b"OK"
print("C4", "OK" if ok4 else "FAIL", repr(head4[:120]))
# N1：同一路径 + Host ⇒ 200（证明 C1 的红来自「缺 Host」）
d5 = req(b"GET /index.html HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
print("N1", "OK" if b"200" in line(d5) else "FAIL", repr(line(d5)))
PY

while read -r tag verdict _rest; do
  case "$tag" in
    C1) [ "$verdict" = OK ] && ok "C1 HTTP/1.1 缺 Host ⇒ 400" || bad "C1 缺 Host 未拒（$verdict）" ;;
    C2) [ "$verdict" = OK ] && ok "C2 HTTP/1.0 缺 Host ⇒ 仍 200" || bad "C2 HTTP/1.0 被误拒（$verdict）" ;;
    C3) [ "$verdict" = OK ] && ok "C3 204 无 Content-Length 且无体" || bad "C3 204 仍带 CL/体（$verdict）" ;;
    C4) [ "$verdict" = OK ] && ok "C4 200 的 CL 与体完好" || bad "C4 200 被误伤（$verdict）" ;;
    N1) [ "$verdict" = OK ] && ok "N1 负控：带 Host ⇒ 200（红的来源是缺 Host）" || bad "N1 负控失败：带 Host 也不 200" ;;
  esac
done < "$TMP/res.txt"

echo "M446-HTTP-OK pass=$ok_n fail=$bad_n"
[ "$bad_n" = 0 ] && exit 0 || exit 1
