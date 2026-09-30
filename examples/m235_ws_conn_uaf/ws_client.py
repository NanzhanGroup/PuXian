#!/usr/bin/env python3
# ============================================================
# ws_client.py —— M235 门用 WS 客户端（纯标准库，不引第三方依赖）
#   用法：ws_client.py <wss-url> [hold_seconds] [send_payload]
#   行为：TLS 连接 → RFC6455 握手 → 发一帧掩码文本 → 保持 hold 秒 → 关闭
#   退出码：0 = 握手成功；1 = 失败
# ============================================================
import socket, ssl, base64, os, sys, struct, time

def main():
    if len(sys.argv) < 2:
        print("usage: ws_client.py <wss-url> [hold] [payload]")
        return 2
    url = sys.argv[1]
    hold = float(sys.argv[2]) if len(sys.argv) > 2 else 2.0
    payload = (sys.argv[3] if len(sys.argv) > 3 else "M235PING").encode()
    ua = sys.argv[4] if len(sys.argv) > 4 else "M235-Client/1.0"

    assert url.startswith("wss://"), "only wss:// supported"
    rest = url[6:]
    slash = rest.find("/")
    path = rest[slash:] if slash >= 0 else "/"
    hp = rest[:slash] if slash >= 0 else rest
    if ":" in hp:
        host, p = hp.rsplit(":", 1)
        port = int(p)
    else:
        host, port = hp, 443

    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE
    s = ctx.wrap_socket(socket.create_connection((host, port), timeout=10), server_hostname=host)
    s.settimeout(15)

    key = base64.b64encode(os.urandom(16)).decode()
    req = ("GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\n"
           "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
           "Sec-WebSocket-Version: 13\r\nUser-Agent: %s\r\n\r\n" % (path, host, port, key, ua))
    s.sendall(req.encode())
    buf = b""
    while b"\r\n\r\n" not in buf:
        d = s.recv(4096)
        if not d:
            print("EOF during handshake")
            return 1
        buf += d
    line = buf.split(b"\r\n")[0].decode("latin1", "replace")
    print("HANDSHAKE: %s" % line)
    if "101" not in line:
        return 1

    mask = os.urandom(4)
    n = len(payload)
    if n < 126:
        hdr = bytes([0x81, 0x80 | n])
    elif n < 65536:
        hdr = bytes([0x81, 0x80 | 126]) + struct.pack(">H", n)
    else:
        hdr = bytes([0x81, 0x80 | 127]) + struct.pack(">Q", n)
    masked = bytes(payload[i] ^ mask[i % 4] for i in range(n))
    s.sendall(hdr + mask + masked)

    t0 = time.time()
    try:
        while time.time() - t0 < hold:
            d = s.recv(4096)
            if not d:
                print("EOF after %.1fs" % (time.time() - t0))
                break
    except Exception as e:
        print("recv err: %r" % (e,))
    try:
        s.close()
    except Exception:
        pass
    return 0

if __name__ == "__main__":
    sys.exit(main())
