#!/usr/bin/env python3
# ============================================================
# m375 门夹具 —— 「TCP 已连上但对端行为」可控的 WS 服务器
#
# 为什么需要它：缺陷 PX-BUG-20261023-01 的触发条件**不是**「连不上」，
#   而是「连上了、但对端一个字节都不发」。任何真实的 ws 服务都不会这么干，
#   所以只能造一个夹具，把「对端行为」变成可控自变量：
#
#   never    accept 后**不读不写也不关**（= 晨曦广州现场的对端）⇒ 客户端必须靠自己的超时退出
#   delay:N  accept 后等 N 秒**再**回 101（证明「超时不是过短」：N < 超时值必须成功）
#   now      立即回 101（正控：证明修复没有把正常握手一起打死）
#   refuse    accept 后立刻 close（负控邻居：证明「快速失败」与「超时」是两条不同路径）
#
# 用法：silent_srv.py <port> <mode>
#   stdout 第一行 LISTEN 供 verify.sh 判「服务起来了」，避免用 sleep 猜。
# ============================================================
import socket, sys, base64, hashlib, threading, time

GUID = b"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


def read_head(c):
    buf = b""
    c.settimeout(30)
    while b"\r\n\r\n" not in buf and len(buf) < 65536:
        chunk = c.recv(4096)
        if not chunk:
            return buf
        buf += chunk
    return buf


def ws_key(head):
    for line in head.split(b"\r\n")[1:]:
        if line.lower().startswith(b"sec-websocket-key:"):
            return line.split(b":", 1)[1].strip()
    return b""


def handle(c, mode):
    try:
        head = read_head(c)
        if mode == "refuse":
            c.close()
            return
        if mode == "never":
            # 关键：**不关**。关掉会让客户端读到 EOF 立即返回 —— 那就测不到「挂死」了。
            # 这里一直握到进程被 verify.sh 杀掉为止。
            while True:
                time.sleep(60)
        if mode.startswith("delay:"):
            time.sleep(float(mode.split(":", 1)[1]))
        key = ws_key(head)
        acc = base64.b64encode(hashlib.sha1(key + GUID).digest())
        c.sendall(b"HTTP/1.1 101 Switching Protocols\r\n"
                  b"Upgrade: websocket\r\nConnection: Upgrade\r\n"
                  b"Sec-WebSocket-Accept: " + acc + b"\r\n\r\n")
        if mode.startswith("flow:"):
            # 边界判据：握手超时**不得漏到正常收发**。
            #   flow:N = 握手成功后**再等 N 秒**才发首帧。N > 握手超时值 ⇒
            #   若实现忘了把 SO_RCVTIMEO 清零，客户端 ws_recv 会在 5s 处返回 null（假失败）。
            time.sleep(float(mode.split(":", 1)[1]))
            payload = b"hello-after-" + mode.encode()
            c.sendall(bytes([0x81, len(payload)]) + payload)
        while True:                      # 握手成功后保持连接（客户端会主动关）
            d = c.recv(4096)
            if not d:
                break
    except Exception:
        pass
    finally:
        try:
            c.close()
        except Exception:
            pass


def main():
    if len(sys.argv) < 3:
        print("usage: silent_srv.py <port> <mode>", file=sys.stderr)
        return 2
    port = int(sys.argv[1])
    mode = sys.argv[2]
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", port))
    srv.listen(16)
    print("LISTEN %d %s" % (port, mode), flush=True)
    while True:
        c, _ = srv.accept()
        threading.Thread(target=handle, args=(c, mode), daemon=True).start()


if __name__ == "__main__":
    sys.exit(main())
