#!/usr/bin/env python3
# M288 门 · 原始 WS 客户端（服务端面用）——把「升级请求与首帧是否同段」变成可控变量
#   together —— 升级请求 + 首帧**一次 sendall**（真实客户端常见：不等 101 就发）
#   after    —— 收到 101 之后再发首帧（对照：修前也必须正常 ⇒ 证明差异确由「同段」引起）
import socket, sys, base64, os, time

def mask_frame(payload: bytes) -> bytes:
    m = b"\x11\x22\x33\x44"
    p = bytes(b ^ m[i % 4] for i, b in enumerate(payload))
    n = len(payload)
    assert n < 126
    return bytes([0x81, 0x80 | n]) + m + p

def main():
    port = int(sys.argv[1])
    mode = sys.argv[2]
    key = base64.b64encode(os.urandom(16))
    req = (b"GET /x HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
           b"Upgrade: websocket\r\nConnection: Upgrade\r\n"
           b"Sec-WebSocket-Key: " + key + b"\r\nSec-WebSocket-Version: 13\r\n\r\n") % port
    s = socket.socket()
    s.settimeout(10)
    s.connect(("127.0.0.1", port))
    fr = mask_frame(b"E")
    if mode == "together":
        s.sendall(req + fr)
        print("sent=%d(req=%d+frame=%d) ONE sendall" % (len(req) + len(fr), len(req), len(fr)), flush=True)
    else:
        s.sendall(req)
        print("sent=%d(req only)" % len(req), flush=True)
    buf = b""
    while b"\r\n\r\n" not in buf:
        blk = s.recv(4096)
        if not blk:
            break
        buf += blk
    head, _, rest = buf.partition(b"\r\n\r\n")
    print("UPGRADED = %s" % (b"101" in head.split(b"\r\n")[0]), flush=True)
    if mode != "together":
        s.sendall(fr)
    got = rest
    deadline = time.time() + 4
    while b"ack" not in got and time.time() < deadline:
        try:
            blk = s.recv(4096)
        except Exception:
            break
        if not blk:
            break
        got += blk
    print("ACKEQ = %s" % (b"ack" in got), flush=True)
    s.close()
    print("cli-done", flush=True)

main()
