#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
fake_up.py — M137 示例用的可控上游桩（裸 socket，逐字节控制分帧与时序）

行为**只由 URL 路径首段**决定（与"谁在问"无关）：
  /ok       200  CT: text/event-stream  chunked  2 事件 + [DONE]
  /slow     200  CT: text/event-stream  chunked  事件1 → **sleep 1.2s** → 事件2 + [DONE]
            （用于证明 sse_read_line 是"边到边读"而不是"收齐再给"）
  /nocat    200  CT: application/json   Content-Length  （Go 不检查 CT，须仍能连通）
  /e404     404  CT: application/json   Content-Length  {"error":"not found"}
  /e500chunk 500 CT: application/json   **chunked** + Retry-After: 3   {"error":"boom"}
  /e429     429  CT: application/json   Content-Length + Retry-After: 7
  /echo     400  CT: text/plain         body = 收到的原始请求（供探针核对发上去的头）
  /hang     收到请求后**不回任何字节**（配合 opts.timeout_ms 验证读头超时）

用法： fake_up.py <port> <unix_sock_path|-> <log>
"""
import json
import os
import socket
import sys
import threading
import time


def split_req(buf):
    i = buf.find(b"\r\n\r\n")
    if i < 0:
        return buf, b""
    return buf[:i + 4], buf[i + 4:]


def read_request(conn):
    buf = b""
    conn.settimeout(5)
    while b"\r\n\r\n" not in buf:
        try:
            d = conn.recv(65536)
        except socket.timeout:
            return buf, b""
        if not d:
            return buf, b""
        buf += d
    head, rest = split_req(buf)
    clen = 0
    for line in head.split(b"\r\n"):
        if line.lower().startswith(b"content-length:"):
            clen = int(line.split(b":")[1].strip() or 0)
    while len(rest) < clen:
        try:
            d = conn.recv(65536)
        except socket.timeout:
            break
        if not d:
            break
        rest += d
    return head, rest


def send_chunks(conn, pieces, delay=0.0):
    for p in pieces:
        conn.sendall(b"%x\r\n" % len(p) + p + b"\r\n")
        if delay:
            time.sleep(delay)
    conn.sendall(b"0\r\n\r\n")


def handle(conn, log):
    head, body = read_request(conn)
    path = b""
    if head:
        parts = head.split(b"\r\n")[0].split(b" ")
        path = parts[1] if len(parts) > 1 else b""
    p = path.decode("latin1")
    log.write("REQ %s\n" % p)
    log.flush()
    if p.startswith("/ok"):
        conn.sendall(b"HTTP/1.1 200 OK\r\n"
                     b"Content-Type: text/event-stream\r\n"
                     b"Cache-Control: no-cache\r\n"
                     b"Transfer-Encoding: chunked\r\n\r\n")
        send_chunks(conn, [b'data: {"a":1}\n\n', b'data: [DONE]\n\n'])
    elif p.startswith("/slow"):
        conn.sendall(b"HTTP/1.1 200 OK\r\n"
                     b"Content-Type: text/event-stream\r\n"
                     b"Transfer-Encoding: chunked\r\n\r\n")
        send_chunks(conn, [b'data: {"n":1}\n\n'])
        # ⚠️ 这里**不能**发终结块：下一块要等 1.2s 后才到
        time.sleep(1.2)
        conn.sendall(b'%x\r\n' % len(b'data: {"n":2}\n\n') + b'data: {"n":2}\n\n' + b"\r\n")
        conn.sendall(b'%x\r\n' % len(b"data: [DONE]\n\n") + b"data: [DONE]\n\n" + b"\r\n")
        conn.sendall(b"0\r\n\r\n")
    elif p.startswith("/nocat"):
        b = b'{"x":1}'
        conn.sendall(b"HTTP/1.1 200 OK\r\n"
                     b"Content-Type: application/json\r\n"
                     b"Content-Length: %d\r\n\r\n" % len(b) + b)
    elif p.startswith("/e404"):
        b = b'{"error":"not found"}'
        conn.sendall(b"HTTP/1.1 404 Not Found\r\n"
                     b"Content-Type: application/json\r\n"
                     b"Content-Length: %d\r\n\r\n" % len(b) + b)
    elif p.startswith("/e500chunk"):
        conn.sendall(b"HTTP/1.1 500 Internal Server Error\r\n"
                     b"Content-Type: application/json\r\n"
                     b"Retry-After: 3\r\n"
                     b"Transfer-Encoding: chunked\r\n\r\n")
        send_chunks(conn, [b'{"err', b'or":"boom"}'])
    elif p.startswith("/e429"):
        b = b'{"error":"rate"}'
        conn.sendall(b"HTTP/1.1 429 Too Many Requests\r\n"
                     b"Content-Type: application/json\r\n"
                     b"Retry-After: 7\r\n"
                     b"Content-Length: %d\r\n\r\n" % len(b) + b)
    elif p.startswith("/echo"):
        conn.sendall(b"HTTP/1.1 400 Bad Request\r\n"
                     b"Content-Type: text/plain\r\n"
                     b"Content-Length: %d\r\n\r\n" % len(head) + head)
    elif p.startswith("/hang"):
        time.sleep(10)
    else:
        conn.sendall(b"HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n")
    try:
        conn.close()
    except OSError:
        pass


def serve(sock, log):
    while True:
        try:
            c, _ = sock.accept()
        except OSError:
            return
        threading.Thread(target=handle, args=(c, log), daemon=True).start()


def main():
    port = int(sys.argv[1])
    addr = sys.argv[2]
    log = open(sys.argv[3], "a", buffering=1)
    if addr != "-" and os.path.exists(addr):
        os.unlink(addr)
    if addr == "-":
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(("127.0.0.1", port))
    else:
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.bind(addr)
    s.listen(64)
    log.write("READY\n")
    serve(s, log)


if __name__ == "__main__":
    main()
