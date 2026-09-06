#!/usr/bin/env python3
# M84-S1 (Issue 23) http_request/http_unix CT/CL 解耦验证 —— raw 请求回环服务
#   同时监听 TCP 与 Unix socket，每连接收一个 HTTP 请求（头到 \r\n\r\n + 按 Content-Length
#   读足 body），把请求原文落盘为 <outdir>/req_<seq>.txt（按连接完成顺序编号），回 200 close。
#   服务端按 CL 成功读足 body 并回 200 ⇒ 证明客户端 Content-Length 已正确补齐（含 body 完整送达）。
# 用法：echo_server.py --tcp-port 18107 --unix-sock /tmp/m84s1_echo.sock --outdir /tmp/m84s1_reqs
import argparse, socket, threading, os, sys

_seq = 0
_lock = threading.Lock()

def recv_full(conn):
    data = b""
    conn.settimeout(8)
    while b"\r\n\r\n" not in data:
        chunk = conn.recv(4096)
        if not chunk:
            break
        data += chunk
        if len(data) > 131072:
            break
    if b"\r\n\r\n" not in data:
        return data, 0
    head, _, rest = data.partition(b"\r\n\r\n")
    clen = 0
    for line in head.split(b"\r\n"):
        if line.lower().startswith(b"content-length:"):
            try:
                clen = int(line.split(b":", 1)[1].strip() or b"0")
            except ValueError:
                clen = 0
    body = rest
    while len(body) < clen:
        chunk = conn.recv(4096)
        if not chunk:
            break
        body += chunk
    return head + b"\r\n\r\n" + body, clen

def handle(conn, outdir):
    global _seq
    try:
        raw, clen = recv_full(conn)
        conn.sendall(b"HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n")
        with _lock:
            _seq += 1
            n = _seq
        path = os.path.join(outdir, "req_%d.txt" % n)
        with open(path, "wb") as f:
            f.write(raw)
    except Exception as e:
        sys.stderr.write("handle err: %s\n" % e)
    finally:
        try:
            conn.close()
        except Exception:
            pass

def serve_tcp(port, outdir):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", port))
    s.listen(16)
    while True:
        c, _ = s.accept()
        threading.Thread(target=handle, args=(c, outdir), daemon=True).start()

def serve_unix(path, outdir):
    try:
        os.unlink(path)
    except OSError:
        pass
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.bind(path)
    s.listen(16)
    while True:
        c, _ = s.accept()
        threading.Thread(target=handle, args=(c, outdir), daemon=True).start()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tcp-port", type=int, default=18107)
    ap.add_argument("--unix-sock", default="/tmp/m84s1_echo.sock")
    ap.add_argument("--outdir", default="/tmp/m84s1_reqs")
    a = ap.parse_args()
    os.makedirs(a.outdir, exist_ok=True)
    t = threading.Thread(target=serve_tcp, args=(a.tcp_port, a.outdir), daemon=True)
    u = threading.Thread(target=serve_unix, args=(a.unix_sock, a.outdir), daemon=True)
    t.start()
    u.start()
    sys.stdout.write("echo_server ready tcp=%d unix=%s out=%s\n" % (a.tcp_port, a.unix_sock, a.outdir))
    sys.stdout.flush()
    threading.Event().wait()

if __name__ == "__main__":
    main()
