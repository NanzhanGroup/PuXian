#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M151 门用的受控服务端：**PostgreSQL 式 TLS 协商** + 二进制回显。

两种模式（同一进程，按**首个字节**区分）：
  ① 明文回显（`--echo`）：连上即把收到的**原始字节**原样回显（含 NUL / 非法 UTF-8），
     用于钉住 `tcp_send*_ex(fd, bytes)` 的二进制安全（缺陷 137）。
  ② PG 式 TSL 协商（`--pgssl`）：先读 8 字节 SSLRequest（记录其 hex）→ 回 `S` →
     **在同一个 fd 上**做 TLS 握手（自签证书）→ 之后 `READY\\n` + 二进制回显。
     用于钉住 `tls_upgrade(fd, opts)`（M151-S1）——「先建 TCP、协商、再升级」这条路径
     在 tls_connect（自建 socket）下**表达不出来**。

日志：每连接一行 `CONN|n=|phase=|sni=|ver=|reqs=<hex,...>`；握手失败记 `ERR|...`。
用法：python3 pg_tls_server.py <port> <mode> <logfile>
"""
import os, socket, ssl, sys, threading

HERE = os.path.dirname(os.path.abspath(__file__))
CERT = os.path.join(HERE, "..", "m150_tls_crypto", "cert.pem")
KEY = os.path.join(HERE, "..", "m150_tls_crypto", "key.pem")


def main():
    port, mode, logpath = int(sys.argv[1]), sys.argv[2], sys.argv[3]
    lock = threading.Lock()
    idx = [0]

    def logline(s):
        with lock:
            with open(logpath, "a") as f:
                f.write(s + "\n")

    sni_seen = {}

    def sni_cb(sslsock, name, ctx):
        sni_seen[sslsock] = name or ""

    def serve_echo(c, n):
        """明文回显：三段 recv，原样回发，收到 b'bye' 即关。"""
        reqs = []
        c.settimeout(5)
        while True:
            try:
                d = c.recv(4096)
            except socket.timeout:
                break
            if not d:
                break
            reqs.append(d.hex())
            if d == b"bye":
                break
            c.sendall(d)
        logline("CONN|n=%d|phase=echo|sni=|ver=|reqs=%s" % (n, ",".join(reqs)))

    def serve_pgssl(c, n):
        c.settimeout(8)
        hdr = b""
        while len(hdr) < 8:
            d = c.recv(8 - len(hdr))
            if not d:
                break
            hdr += d
        c.sendall(b"S")
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ctx.load_cert_chain(CERT, KEY)
        ctx.sni_callback = sni_cb
        try:
            tc = ctx.wrap_socket(c, server_side=True)
        except Exception as e:                      # 握手失败（负控会走到这里）
            logline("ERR|n=%d|req=%s|err=%s" % (n, hdr.hex(), type(e).__name__))
            c.close()
            return
        ver = tc.version()
        sni = sni_seen.get(tc, "")
        tc.sendall(b"READY\n")
        reqs = []
        while True:
            try:
                d = tc.recv(4096)
            except socket.timeout:
                break
            if not d:
                break
            reqs.append(d.hex())
            if d == b"bye":
                break
            tc.sendall(b"ECHO:" + d)
        logline("CONN|n=%d|phase=pgssl|sni=%s|ver=%s|req=%s|reqs=%s"
                % (n, sni, ver, hdr.hex(), ",".join(reqs)))
        tc.close()

    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", port))
    s.listen(8)
    print("READY %d" % port)
    sys.stdout.flush()
    while True:
        c, _ = s.accept()
        with lock:
            idx[0] += 1
            n = idx[0]
        try:
            if mode == "echo":
                serve_echo(c, n)
            else:
                serve_pgssl(c, n)
        except Exception as e:
            logline("ERR|n=%d|err=%s:%s" % (n, type(e).__name__, e))
        finally:
            try:
                c.close()
            except OSError:
                pass


if __name__ == "__main__":
    main()
