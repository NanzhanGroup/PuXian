#!/usr/bin/env python3
# M316 探测装置：Unix socket 服务端，把收到的**原始请求头**按 hex 吐出来
#   为什么用 python：PuXian 没有 unix_listen 原语（只有 unix_connect），
#   而本门要看的恰恰是**线上字节**（重复 Host），不是解析后的 dict。
import os, socket, sys

path = sys.argv[1]
n = int(sys.argv[2]) if len(sys.argv) > 2 else 1
try:
    os.unlink(path)
except FileNotFoundError:
    pass
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.bind(path)
s.listen(8)
print("READY", flush=True)
for i in range(n):
    c, _ = s.accept()
    data = b""
    while b"\r\n\r\n" not in data:
        b = c.recv(4096)
        if not b:
            break
        data += b
    print("RAW %d %s" % (i, data.hex()), flush=True)
    c.sendall(b"HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK")
    c.close()
print("DONE", flush=True)
s.close()
