#!/usr/bin/env python3
# M288 门 · 「最小 WS 服务端」——把「101 与首帧是否同段到达」变成**可控变量**
#
# 为什么本门必须用 python 裸 socket 对端（而不用 .px 对端）：
#   缺陷的形状是「头与首帧落在**同一次读**里」。而「两次 send 会不会被内核合成一段」
#   不由应用层决定（Nagle / 发送队列 / 接收方调度）—— .px 层的 ws_send 无法**保证**
#   「同段」或「不同段」。裸 socket 的 `sendall(101 + frame)` 才能真正固定这个变量。
#   ⇒ 本门因此**与负载无关**（实测：负载档 7.5% 复现 ⇒ 本夹具 100% 复现）。
import socket, sys, hashlib, base64, time, struct

GUID = b"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

def text_frame(payload: bytes) -> bytes:
    n = len(payload)
    if n < 126:
        return bytes([0x81, n]) + payload
    if n <= 0xFFFF:
        return bytes([0x81, 126]) + struct.pack(">H", n) + payload
    return bytes([0x81, 127]) + struct.pack(">Q", n) + payload

def main():
    port = int(sys.argv[1])
    mode = sys.argv[2] if len(sys.argv) > 2 else "combined"
    payload = b"A" if mode != "big" else (b"B" * 3000)
    frame = text_frame(payload)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", port))
    s.listen(8)
    print("listening %d mode=%s" % (port, mode), flush=True)
    s.settimeout(15)
    try:
        c, _ = s.accept()
    except Exception:
        print("accept-timeout", flush=True)
        s.close(); return
    c.settimeout(10)
    data = b""
    while b"\r\n\r\n" not in data:
        blk = c.recv(4096)
        if not blk:
            break
        data += blk
    key = None
    for line in data.split(b"\r\n"):
        if line.lower().startswith(b"sec-websocket-key:"):
            key = line.split(b":", 1)[1].strip()
    if not key:
        print("NO-KEY", flush=True)
        c.close(); s.close(); return
    acc = base64.b64encode(hashlib.sha1(key + GUID).digest())
    resp = (b"HTTP/1.1 101 Switching Protocols\r\n"
            b"Upgrade: websocket\r\n"
            b"Connection: Upgrade\r\n"
            b"Sec-WebSocket-Accept: " + acc + b"\r\n\r\n")
    if mode == "split":          # 101 单独一段；首帧 0.2s 后
        c.sendall(resp)
        print("first-send=%d(101 only)" % len(resp), flush=True)
        time.sleep(0.2)
        c.sendall(frame)
    elif mode == "partial":      # 101 + 首帧的**第 1 字节**同段（半帧）
        c.sendall(resp + frame[:1])
        print("first-send=%d(101+frame[0:1])" % (len(resp) + 1), flush=True)
        time.sleep(0.2)
        c.sendall(frame[1:])
    else:                        # combined / big：101 + 整帧**一次 sendall**
        c.sendall(resp + frame)
        print("first-send=%d(101=%d+frame=%d)" % (len(resp) + len(frame), len(resp), len(frame)), flush=True)
    c.settimeout(3.0)
    try:
        while True:
            if not c.recv(4096):
                break
    except Exception:
        pass
    c.close(); s.close()
    print("mock-done", flush=True)

main()
