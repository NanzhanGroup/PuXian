"""m319 客户端：握手成功后保持 <hold> 秒，然后按 mode 关闭（fin = 正常 FIN，rst = RST）。

判据不依赖客户端读到什么 —— 它只负责把连接推到「对端已关」的状态，
服务端的自旋与否由 verify.sh 用 /proc/<pid>/stat 的 utime 量出来。
"""
import base64
import os
import socket
import struct
import sys
import time

port = int(sys.argv[1])
hold = float(sys.argv[2]) if len(sys.argv) > 2 else 1.0
mode = sys.argv[3] if len(sys.argv) > 3 else "fin"

s = socket.create_connection(("127.0.0.1", port), timeout=8)
key = base64.b64encode(os.urandom(16)).decode()
req = ("GET /agent/x HTTP/1.1\r\nHost: 127.0.0.1:%d\r\nUpgrade: websocket\r\n"
       "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\n\r\n") % (port, key)
s.sendall(req.encode())
buf = b""
while b"\r\n\r\n" not in buf:
    d = s.recv(4096)
    if not d:
        break
    buf += d
head = buf.split(b"\r\n")[0]
print("101-OK" if b"101" in head else "NO101")
if b"101" not in head:
    sys.exit(2)
time.sleep(hold)
if mode == "rst":
    s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))
s.close()
