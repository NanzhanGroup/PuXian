#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M312 压测器：8 并发 keep-alive，量 rps / p50 / p95 / max。

用法：load.py <host> <port> <并发> <秒数>
输出一行契约行，便于门判定：
  M312-LOAD conc=8 secs=20 req=... rps=... p50=...ms p95=...ms max=...ms err=...
"""
import socket, sys, time, threading, statistics

host = sys.argv[1]
port = int(sys.argv[2])
conc = int(sys.argv[3])
secs = float(sys.argv[4])

lat = []
err = [0]
lock = threading.Lock()
stop_at = time.time() + secs

REQ = b"GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\n\r\n"

def worker():
    global lat
    local = []
    e = 0
    try:
        s = socket.create_connection((host, port), timeout=30)
        s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        buf = b""
        while time.time() < stop_at:
            t0 = time.time()
            try:
                s.sendall(REQ)
                # 读到 complete response（Content-Length 或 chunked 结束）
                while True:
                    if b"\r\n\r\n" in buf:
                        head, rest = buf.split(b"\r\n\r\n", 1)
                        hl = head.decode("latin1")
                        cl = 0
                        for line in hl.split("\r\n"):
                            if line.lower().startswith("content-length:"):
                                cl = int(line.split(":", 1)[1].strip())
                        if len(rest) >= cl:
                            buf = rest[cl:]
                            break
                    d = s.recv(65536)
                    if not d:
                        raise IOError("eof")
                    buf += d
            except Exception:
                e += 1
                try:
                    s.close()
                except Exception:
                    pass
                try:
                    s = socket.create_connection((host, port), timeout=30)
                    s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                    buf = b""
                except Exception:
                    break
                continue
            local.append((time.time() - t0) * 1000.0)
        s.close()
    except Exception:
        e += 1
    with lock:
        lat.extend(local)
        err[0] += e

ths = [threading.Thread(target=worker, daemon=True) for _ in range(conc)]
t0 = time.time()
for t in ths: t.start()
for t in ths: t.join()
el = time.time() - t0

lat.sort()
def q(p):
    if not lat: return 0.0
    i = min(len(lat) - 1, int(len(lat) * p))
    return lat[i]

print("M312-LOAD conc=%d secs=%.0f req=%d rps=%.1f p50=%.2fms p95=%.2fms max=%.2fms err=%d"
      % (conc, el, len(lat), len(lat) / el if el > 0 else 0, q(0.50), q(0.95), q(1.0), err[0]))
