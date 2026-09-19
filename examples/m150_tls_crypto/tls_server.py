#!/usr/bin/env python3
# M150 门用的受控 TLS 服务端（自签证书；**本机回环**，无需外网）
#
# 行为（极简、确定）：
#   ① 握手完成即发一行 `SRV <version>\n`（**只带版本**，不带套件 —— 套件在 Go 与 mbedtls
#      之间偏好序不同，放进问候语会让两侧 recv_data 必然不同；套件改由服务端**日志**记录，
#      门再拿 PuXian 自报的套件去做"服务端确实协商过这个套件"的交叉验证）
#   ② 每收到一块数据回 `ECHO:<原样>`；收到 `bye` 回完即关
#   ③ 每个连接往日志追加一行：`CONN|n=<序号>|sni=<SNI>|ver=<版本>|cipher=<IANA 名>`
#      （SNI 与套件都是**服务端视角**，用来钉住客户端是否真的发了 SNI / 真的协商了某套件）
#   ④ 握手失败的连接记 `ERR|...`（verify=true 的负控会走到这里）
#
# 用法：python3 tls_server.py <port> <logfile>
import socket, ssl, sys, threading, os

def main():
    port = int(sys.argv[1])
    log = sys.argv[2]
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    here = os.path.dirname(os.path.abspath(__file__))
    ctx.load_cert_chain(os.path.join(here, "cert.pem"), os.path.join(here, "key.pem"))
    # ⚠️ 陷阱（本地实测踩到）：服务端侧 **拿不到** `SSLSocket.server_hostname`
    #   （那是**客户端**侧属性，服务端恒为 None）—— 服务端要读对端 SNI 只能挂
    #   `SSLContext.sni_callback`。第一版用 `tc.server_hostname` 打印，于是
    #   "客户端根本没发 SNI" 与 "服务端不会读 SNI" 两种成因看起来一模一样（都是空）。
    sni_seen = {}
    def _sni_cb(sslsock, server_name, sslctx):
        sni_seen[sslsock] = server_name or ""
    ctx.sni_callback = _sni_cb
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", port))
    s.listen(16)
    print("READY %d" % port)
    sys.stdout.flush()
    idx = [0]
    lock = threading.Lock()

    def logline(line):
        with lock:
            with open(log, "a") as f:
                f.write(line + "\n")

    def handle(c):
        try:
            tc = ctx.wrap_socket(c, server_side=True)
            with lock:
                idx[0] += 1
                n = idx[0]
            logline("CONN|n=%d|sni=%s|ver=%s|cipher=%s" %
                    (n, sni_seen.pop(tc, ""), tc.version(), tc.cipher()[0]))
            tc.sendall(("SRV %s\n" % tc.version()).encode())
            while True:
                d = tc.recv(4096)
                if not d:
                    break
                tc.sendall(b"ECHO:" + d)
                if d == b"bye":
                    break
            try:
                tc.close()
            except Exception:
                pass
        except Exception as e:
            logline("ERR|%s" % e)

    while True:
        try:
            c, _ = s.accept()
        except OSError:
            break
        threading.Thread(target=handle, args=(c,), daemon=True).start()

if __name__ == "__main__":
    main()
