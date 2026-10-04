#!/usr/bin/env python3
# ============================================================
# m255 压测器 · 晨曦形态：WS 接管请求 ⇒ 读 ≤64B ⇒ **SO_LINGER=0 + close（RST）**
#
# 为什么用 Python 而不是 Go：本仓 CI 无 Go 步骤（口径同 m136/m138）；
#   而 SO_LINGER 在 Python 里同样可设（`examples/m123_http_conn_alive/bigdisc.py` 先例）。
#
# 用法：rst_client.py <host:port> <并发> <每协程轮数> [--upgraded]
#   --upgraded：只统计 HTTP 101 的次数（用于「会话形态自证」）
# 输出末行：CLIENT ok=<n> upgraded=<m> fail=<k> elapsed=<ms>
# ============================================================
import socket
import struct
import sys
import threading
import time
from urllib.parse import urlparse

REQ = (
    b"GET /agent/x HTTP/1.1\r\n"
    b"Host: %s\r\n"
    b"Upgrade: websocket\r\n"
    b"Connection: Upgrade\r\n"
    b"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
    b"Sec-WebSocket-Version: 13\r\n\r\n"
)


def one_session(addr, host, stats):
    """一轮 = 建链 → 发升级请求 → 读 ≤64B → SO_LINGER(0) + close（立即 RST）"""
    s = None
    try:
        s = socket.create_connection(addr, timeout=2.0)
        s.sendall(REQ % host.encode())
        s.settimeout(0.3)
        try:
            data = s.recv(64)
        except OSError:
            data = b""
        if data.startswith(b"HTTP/1.1 101"):
            stats["upgraded"] += 1
        stats["ok"] += 1
    except OSError:
        stats["fail"] += 1
    finally:
        if s is not None:
            try:
                # ★ 关键：linger{on=1, time=0} ⇒ close() 发 RST 而不是 FIN
                s.setsockopt(
                    socket.SOL_SOCKET,
                    socket.SO_LINGER,
                    struct.pack("ii", 1, 0),
                )
            except OSError:
                pass
            try:
                s.close()
            except OSError:
                pass


def main():
    if len(sys.argv) < 4:
        print("用法: rst_client.py <host:port> <并发> <每协程轮数>", file=sys.stderr)
        return 2
    parsed = urlparse("//" + sys.argv[1])
    host = parsed.hostname or "127.0.0.1"
    port = parsed.port or 80
    conc = int(sys.argv[2])
    rounds = int(sys.argv[3])

    addr = (host, port)
    stats = {"ok": 0, "upgraded": 0, "fail": 0}
    lock = threading.Lock()

    def worker():
        local = {"ok": 0, "upgraded": 0, "fail": 0}
        for _ in range(rounds):
            one_session(addr, host, local)
        with lock:
            for k in stats:
                stats[k] += local[k]

    t0 = time.time()
    threads = [threading.Thread(target=worker, daemon=True) for _ in range(conc)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    ms = int((time.time() - t0) * 1000)
    print(
        "CLIENT ok=%d upgraded=%d fail=%d elapsed=%d"
        % (stats["ok"], stats["upgraded"], stats["fail"], ms)
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
