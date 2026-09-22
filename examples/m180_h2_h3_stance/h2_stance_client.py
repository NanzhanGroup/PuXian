#!/usr/bin/env python3
# M180 门用客户端：验证 h2 的口径（不做 h2）
#   A: `Upgrade: h2c` 的 h1.1 请求 ⇒ **按 HTTP/1.1 正常服务**（真实 docroot 内容，不是演示页）
#   B: h2 **prior-knowledge 前导**（`PRI * HTTP/2.0`）⇒ **505 + 说明文本**
import socket, sys, time

port = int(sys.argv[1])
fail = []


def recv_all(s, wait=0.8):
    s.settimeout(2.0)
    data = b""
    t0 = time.time()
    try:
        while True:
            c = s.recv(4096)
            if not c:
                break
            data += c
            if time.time() - t0 > wait:
                break
    except Exception:
        pass
    return data


# ── A：h2c 升级请求（应被忽略，按 h1.1 服务）
try:
    s = socket.create_connection(("127.0.0.1", port), timeout=5)
    s.sendall(
        b"GET /index.html HTTP/1.1\r\nHost: localhost\r\n"
        b"Connection: Upgrade\r\nUpgrade: h2c\r\n"
        b"HTTP2-Settings: AAMAAABkAAQCAAAAAAIAAAAA\r\n\r\n"
    )
    data = recv_all(s)
    s.close()
    head = data.split(b"\r\n", 1)[0].decode("latin1")
    a_ok = (
        head.startswith("HTTP/1.1 200")
        and b"STANCE-DOC-OK" in data
        and b"PuXian HTTP/2" not in data
        and b"101" not in head.encode()
    )
    print("A_h2c_upgrade: %s  head=%s" % ("OK" if a_ok else "FAIL", head))
    if not a_ok:
        fail.append("A")
        print("   body[:160]=%r" % data[:160])
except Exception as e:
    print("A_h2c_upgrade: FAIL 异常 %s" % e)
    fail.append("A")

# ── B：h2 prior-knowledge 前导（应被明确拒绝 505）
try:
    s = socket.create_connection(("127.0.0.1", port), timeout=5)
    s.sendall(b"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n")
    data = recv_all(s)
    s.close()
    head = data.split(b"\r\n", 1)[0].decode("latin1")
    b_ok = head.startswith("HTTP/1.1 505") and "HTTP/2 未支持".encode() in data
    print("B_prior_knowledge: %s  head=%s" % ("OK" if b_ok else "FAIL", head))
    if not b_ok:
        fail.append("B")
        print("   body[:160]=%r" % data[:160])
except Exception as e:
    print("B_prior_knowledge: FAIL 异常 %s" % e)
    fail.append("B")

print("M180-H2STANCE-OK" if not fail else ("M180-H2STANCE-FAIL " + ",".join(fail)))
sys.exit(0)
