#!/usr/bin/env python3
# M182 门夹具：Unix 域套接字上的极简 HTTP/1.1 应答器（固定带 X-Test 头）。
# 为什么要它：本仓此前**没有** `http_unix` 成功路径的任何门（只有连接失败用例），
#   ⇒ `bi_http_unix` 的根面窗口无人覆盖。用 python3 当对端即可端到端覆盖（52 个门已用 python3）。
import os, socket, sys

path = sys.argv[1]
try:
    os.unlink(path)
except FileNotFoundError:
    pass

RESP = (b"HTTP/1.1 200 OK\r\n"
        b"X-Test: px-uds\r\n"
        b"Content-Length: 2\r\n"
        b"Connection: keep-alive\r\n"
        b"\r\n"
        b"ok")

srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
srv.bind(path)
srv.listen(64)
print("UDS-READY", flush=True)
while True:
    try:
        conn, _ = srv.accept()
    except OSError:
        break
    try:
        req = b""
        while b"\r\n\r\n" not in req:
            chunk = conn.recv(4096)
            if not chunk:
                break
            req += chunk
        if req:
            conn.sendall(RESP)
    except OSError:
        pass
    finally:
        try:
            conn.close()
        except OSError:
            pass
