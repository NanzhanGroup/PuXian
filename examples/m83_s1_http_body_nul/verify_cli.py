#!/usr/bin/env python3
# m83_s1 verify 的 HTTP 客户端助手：TCP(unix) POST 二进制 body（可含 NUL），自断言返回
# 用例：
#   probe-tcp  <port>   POST b"A\x00BC" /probe → 期望 "len=4,after_nul=true"
#   probe-unix <sock>   同上走 AF_UNIX
#   echo-tcp   <port>   POST b"A\x00BC" /echo → 期望逐字节回显 b"A\x00BC"（含 NUL）
#   echo-unix  <sock>   同上走 AF_UNIX
#   len-tcp    <port> <binfile> <expect>   POST 文件 /len → 期望 "len=<expect>"
#   len-unix   <sock>  <binfile> <expect>  同上走 AF_UNIX
import sys, socket, urllib.request

NUL_BODY = b"A\x00BC"

def tcp_post(port, path, body):
    req = urllib.request.Request(
        "http://127.0.0.1:%s%s" % (port, path), data=body,
        headers={"Content-Type": "application/octet-stream"}, method="POST")
    with urllib.request.urlopen(req, timeout=20) as r:
        return r.status, r.read()

def unix_post(sock, path, body):
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect(sock)
    head = ("POST %s HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/octet-stream\r\n"
            "Content-Length: %d\r\nConnection: close\r\n\r\n" % (path, len(body))).encode()
    s.sendall(head + body)
    data = b""
    while True:
        b = s.recv(65536)
        if not b:
            break
        data += b
    s.close()
    h, _, body2 = data.partition(b"\r\n\r\n")
    return int(h.split(b" ")[1]), body2

def expect(cond, msg):
    if not cond:
        print("FAIL " + msg)
        sys.exit(1)
    print("PASS " + msg)

def main():
    cmd = sys.argv[1]
    if cmd == "probe-tcp":
        st, b = tcp_post(sys.argv[2], "/probe", NUL_BODY)
        expect(b == b"len=4,after_nul=true", "probe-tcp: %s" % b.decode())
    elif cmd == "probe-unix":
        st, b = unix_post(sys.argv[2], "/probe", NUL_BODY)
        expect(b == b"len=4,after_nul=true", "probe-unix: %s" % b.decode())
    elif cmd == "echo-tcp":
        st, b = tcp_post(sys.argv[2], "/echo", NUL_BODY)
        expect(b == NUL_BODY, "echo-tcp: 回显 %r（期望 %r）" % (b, NUL_BODY))
    elif cmd == "echo-unix":
        st, b = unix_post(sys.argv[2], "/echo", NUL_BODY)
        expect(b == NUL_BODY, "echo-unix: 回显 %r（期望 %r）" % (b, NUL_BODY))
    elif cmd == "len-tcp":
        body = open(sys.argv[3], "rb").read()
        st, b = tcp_post(sys.argv[2], "/len", body)
        expect(b == b"len=" + sys.argv[4].encode(), "len-tcp: %s（期望 len=%s，%d B）" % (b.decode(), sys.argv[4], len(body)))
    elif cmd == "len-unix":
        body = open(sys.argv[3], "rb").read()
        st, b = unix_post(sys.argv[2], "/len", body)
        expect(b == b"len=" + sys.argv[4].encode(), "len-unix: %s（期望 len=%s，%d B）" % (b.decode(), sys.argv[4], len(body)))
    else:
        print("unknown cmd " + cmd)
        sys.exit(2)

if __name__ == "__main__":
    main()
