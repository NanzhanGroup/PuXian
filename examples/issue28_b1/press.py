#!/usr/bin/env python3
# ISSUE28-B1 客户端：顺序单发 80 次测量延迟分布（判断 GC 是否落在请求热路径）
# 用法：python3 press.py <sock> <n>
import socket, sys, time, json

sock = sys.argv[1]
n = int(sys.argv[2]) if len(sys.argv) > 2 else 80
lats = []
for i in range(n):
    c = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    c.settimeout(10)
    c.connect(sock)
    req = ("GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n").encode()
    t0 = time.time()
    c.sendall(req)
    buf = b""
    while True:
        d = c.recv(65536)
        if not d:
            break
        buf += d
    dt = (time.time() - t0) * 1000
    c.close()
    lats.append(dt)
    if b"ok-b1" not in buf:
        print("BAD-RESP", i, buf[:200])
        sys.exit(2)
lats.sort()
def pct(p):
    return lats[min(len(lats) - 1, int(len(lats) * p))]
print("p50=%.1fms p95=%.1fms max=%.1fms" % (pct(0.5), pct(0.95), lats[-1]))
# 稳健性阈值：仅拦"灾难性卡死"（>2-3s）；精细 p95≤50ms 目标需观音/清歌 ws-approve 实测
if pct(0.95) < 2000 and lats[-1] < 3500:
    print("LATENCY-OK")
    sys.exit(0)
else:
    print("LATENCY-SPIKE")
    sys.exit(1)
