#!/usr/bin/env python3
# M408 门内自建最小 HTTP 代理（判据用；**纯本机**、随机端口、不依赖外网/他人机器）
#   · CONNECT：按 -mode 回应 —— ok（200 后把上游字节双向搬运）/ deny（403）/ close（200 后立刻断）
#   · 明文：绝对 URL 请求行 → 连目标 → 转发（供 http 经代理的用例）
#   · 每收到一条 CONNECT，就往 -log 追加一行 "CONNECT <host:port>"（门据此**证明流量真走了代理**）
# 为什么不用现成代理/别人的机器：M405 的 C6 口径 —— 门不得依赖仓库外产物、外网和他人机器。
import argparse
import socket
import sys
import threading


def logline(path, text):
    if not path:
        return
    with open(path, "a", encoding="utf-8") as f:
        f.write(text + "\n")
        f.flush()


def pump(a, b):
    try:
        while True:
            data = a.recv(65536)
            if not data:
                break
            b.sendall(data)
    except OSError:
        pass
    finally:
        try:
            b.shutdown(socket.SHUT_WR)
        except OSError:
            pass


def handle(conn, mode, log):
    conn.settimeout(10)
    try:
        buf = b""
        while b"\r\n\r\n" not in buf:
            chunk = conn.recv(4096)
            if not chunk:
                return
            buf += chunk
            if len(buf) > 65536:
                return
        head = buf.split(b"\r\n\r\n", 1)[0].decode("latin-1")
        first = head.split("\r\n")[0]
        parts = first.split(" ")
        if len(parts) < 2:
            return
        method, target = parts[0], parts[1]

        if method.upper() == "CONNECT":
            logline(log, "CONNECT " + target)
            if mode == "deny":
                conn.sendall(b"HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n")
                return
            host, _, port = target.rpartition(":")
            if not port.isdigit():
                conn.sendall(b"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n")
                return
            if mode == "close":
                # 200 之后立刻断：模拟"隧道建立成功但上游不可用"——
                #   用来证明「CONNECT 真的发出去了」而不是"直接对代理做 TLS"
                conn.sendall(b"HTTP/1.1 200 Connection Established\r\n\r\n")
                return
            try:
                up = socket.create_connection((host, int(port)), timeout=10)
            except OSError:
                conn.sendall(b"HTTP/1.1 502 Bad Gateway\r\nContent-Length: 0\r\n\r\n")
                return
            conn.sendall(b"HTTP/1.1 200 Connection Established\r\n\r\n")
            rest = buf.split(b"\r\n\r\n", 1)[1]
            if rest:
                up.sendall(rest)
            t = threading.Thread(target=pump, args=(up, conn), daemon=True)
            t.start()
            pump(conn, up)
            up.close()
            return

        # 明文代理：请求行是绝对 URL（http://host:port/path）
        logline(log, "PLAIN " + target)
        if not target.lower().startswith("http://"):
            conn.sendall(b"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n")
            return
        rest = target[len("http://"):]
        hostport, _, path = rest.partition("/")
        path = "/" + path
        if ":" in hostport:
            host, _, port = hostport.rpartition(":")
        else:
            host, port = hostport, "80"
        try:
            up = socket.create_connection((host, int(port)), timeout=10)
        except OSError:
            conn.sendall(b"HTTP/1.1 502 Bad Gateway\r\nContent-Length: 0\r\n\r\n")
            return
        hdrs = head.split("\r\n")[1:]
        out = ["%s %s HTTP/1.1" % (method, path)] + hdrs + ["Connection: close", "", ""]
        up.sendall("\r\n".join(out).encode("latin-1"))
        pump(up, conn)
        up.close()
    except OSError:
        pass
    finally:
        try:
            conn.close()
        except OSError:
            pass


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--mode", default="ok", choices=["ok", "deny", "close"])
    ap.add_argument("--log", default="")
    a = ap.parse_args()
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", 0))
    srv.listen(16)
    print("PROXY-READY %d" % srv.getsockname()[1], flush=True)
    while True:
        try:
            c, _ = srv.accept()
        except OSError:
            return 0
        threading.Thread(target=handle, args=(c, a.mode, a.log), daemon=True).start()


if __name__ == "__main__":
    sys.exit(main())
