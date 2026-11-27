#!/usr/bin/env python3
# ============================================================
# smtp954_server.py —— 缺陷 954（M448）E1 判据用的**假 SMTP 服务端**
# ------------------------------------------------------------
# 为什么需要它：STARTTLS / implicit TLS 这两条能力**不能靠读代码证明**。
#   只有「服务端在明文阶段之后强制要求 TLS，客户端仍能发信成功」才算真接入了 TLS；
#   否则一个什么都没做的实现也能对着「宽容服务端」跑绿（假绿）。
#
# 三种模式：
#   plain     —— 全程明文（EHLO 不广告 STARTTLS）
#   advertise-fake —— 明文，但 EHLO **广告** STARTTLS；客户端发 STARTTLS 时回 454 拒绝。
#                用于负控：证明客户端在「广告了却升不上去」时**报错而不是退回明文发信**
#   （--advertise-starttls 为 plain 模式的保留开关，当前等同 plain：广告需服务端真能升级）
#   starttls  —— EHLO 广告 STARTTLS；STARTTLS 后**立即升级 TLS**；
#                升级前收到的 MAIL FROM 一律回 530（强制客户端必须先升级）
#   implicit  —— 接受连接即 TLS 握手（465 风格）；明文说 SMTP 话会被 TLS 握手打回
#
# 记录（--out JSON）：
#   {"mode":..., "tls": bool, "tls_version": "TLSv1.3"|..., "cipher":...,
#    "ehlo_after_tls": bool, "auth": bool, "from":..., "rcpt":[...],
#    "subject":...,  "data_bytes": N}
#   ⇒ 门脚本据此断言「确实升级过 TLS」「升级后重新 EHLO」「消息体真的送达」
#
# 用法：smtp954_server.py --port 15870 --mode starttls --cert c.pem --key k.pem --out r.json
# 退出：收到一封信（DATA 结束）后再收到 QUIT 即写 --out 并退出 0；--max-conns 轮数到也退出。
# ============================================================
import argparse
import json
import os
import socket
import ssl
import sys
import threading
import time

REC = {
    "mode": None,
    "tls": False,
    "tls_version": "",
    "cipher": "",
    "ehlo_after_tls": False,
    "auth": False,
    "from": "",
    "rcpt": [],
    "subject": "",
    "data_bytes": 0,
    "plain_mail_rejected": 0,
}


def log(*a):
    print("[smtp954]", *a, flush=True)


def make_ctx(cert, key):
    c = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    c.load_cert_chain(cert, key)
    return c


def rec_conn(conn):
    """TLS 升级后更新记录"""
    try:
        REC["tls_version"] = conn.version() or ""
        ci = conn.cipher()
        REC["cipher"] = ci[0] if ci else ""
    except Exception:
        pass
    REC["tls"] = True


def handle(conn, addr, args, ctx):
    f = conn.makefile("rwb")
    helo_after_tls = False

    def send(line):
        f.write((line + "\r\n").encode())
        f.flush()

    def readline():
        ln = f.readline()
        if not ln:
            return None
        return ln.decode("utf-8", "replace").rstrip("\r\n")

    tls_now = False
    try:
        if args.mode == "implicit":
            conn = ctx.wrap_socket(conn, server_side=True)
            rec_conn(conn)
            f = conn.makefile("rwb")
            tls_now = True

        send("220 smtp954 fake ESMTP" + (" (implicit tls)" if tls_now else ""))

        in_data = False
        data_buf = []
        while True:
            ln = readline()
            if ln is None:
                break
            if in_data:
                if ln == ".":
                    in_data = False
                    raw = "\r\n".join(data_buf)
                    REC["data_bytes"] = len(raw.encode())
                    for hl in data_buf:
                        if hl.lower().startswith("subject:"):
                            REC["subject"] = hl.split(":", 1)[1].strip()
                    REC["rcpt"] = REC["rcpt"]
                    send("250 2.0.0 queued")
                    log("message accepted", REC["data_bytes"], "bytes; tls=", REC["tls"])
                else:
                    data_buf.append(ln[1:] if ln.startswith("..") else ln)
                continue

            up = ln.upper()
            if up.startswith("EHLO") or up.startswith("HELO"):
                ext = []
                if args.mode in ("starttls", "advertise-fake") and not tls_now:
                    ext.append("250-STARTTLS")
                    ext.append("250-AUTH LOGIN")
                    ext.append("250 SIZE 10485760")
                else:
                    ext.append("250-AUTH LOGIN")
                    ext.append("250 SIZE 10485760")
                if up.startswith("EHLO"):
                    send("250-smtp954")
                    for e in ext[:-1]:
                        send(e)
                    send(ext[-1])
                else:
                    send("250 smtp954")
                if tls_now:
                    REC["ehlo_after_tls"] = True
                    helo_after_tls = True
            elif up.startswith("STARTTLS"):
                if args.mode == "advertise-fake":
                    # 负控用：广告了但升不上去（真实世界的中间件/配置错就是这样）
                    send("454 4.7.0 TLS not available right now")
                    continue
                if args.mode != "starttls" or tls_now:
                    send("502 5.5.1 STARTTLS not available")
                    continue
                send("220 2.0.0 Ready to start TLS")
                conn = ctx.wrap_socket(conn, server_side=True)
                rec_conn(conn)
                f = conn.makefile("rwb")
                tls_now = True
                log("upgraded to TLS:", REC["tls_version"])
            elif up.startswith("AUTH LOGIN"):
                send("334 VXNlcm5hbWU6")
                readline()
                send("334 UGFzc3dvcmQ6")
                readline()
                REC["auth"] = True
                send("235 2.7.0 Authentication successful")
            elif up.startswith("MAIL FROM"):
                if args.mode == "starttls" and not tls_now:
                    REC["plain_mail_rejected"] += 1
                    send("530 5.7.0 Must issue a STARTTLS command first")
                    continue
                REC["from"] = ln.split(":", 1)[1].strip()
                send("250 2.1.0 Ok")
            elif up.startswith("RCPT TO"):
                REC["rcpt"].append(ln.split(":", 1)[1].strip())
                send("250 2.1.5 Ok")
            elif up == "DATA":
                if args.mode == "starttls" and not tls_now:
                    REC["plain_mail_rejected"] += 1
                    send("530 5.7.0 Must issue a STARTTLS command first")
                    continue
                in_data = True
                data_buf = []
                send("354 End data with <CR><LF>.<CR><LF>")
            elif up == "QUIT":
                send("221 2.0.0 Bye")
                break
            elif up == "RSET":
                send("250 2.0.0 Ok")
            else:
                send("502 5.5.2 Command not implemented")
    except Exception as e:
        log("conn error:", e)
    finally:
        try:
            f.close()
        except Exception:
            pass
        try:
            conn.close()
        except Exception:
            pass


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, required=True)
    ap.add_argument("--mode", choices=["plain", "starttls", "implicit", "advertise-fake"], required=True)
    ap.add_argument("--cert", required=True)
    ap.add_argument("--key", required=True)
    ap.add_argument("--out", default="")
    ap.add_argument("--advertise-starttls", action="store_true")
    ap.add_argument("--max-conns", type=int, default=1)
    ap.add_argument("--idle-timeout", type=float, default=30.0)
    args = ap.parse_args()

    REC["mode"] = args.mode
    # --advertise-starttls 只影响 plain 模式的广告文案（用于「广告了但不支持」的对拍）
    ctx = make_ctx(args.cert, args.key)

    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", args.port))
    srv.listen(8)
    srv.settimeout(args.idle_timeout)
    # --port 0 ⇒ 内核分配空闲端口（8 分片并发跑门时不撞端口）；实际端口由 READY 行报出
    real_port = srv.getsockname()[1]
    print("READY %d %s" % (real_port, args.mode), flush=True)

    served = 0
    try:
        while served < args.max_conns:
            try:
                conn, addr = srv.accept()
            except socket.timeout:
                break
            conn.settimeout(args.idle_timeout)
            t = threading.Thread(target=handle, args=(conn, addr, args, ctx), daemon=True)
            t.start()
            t.join()
            served += 1
    finally:
        try:
            srv.close()
        except Exception:
            pass
        if args.out:
            with open(args.out, "w") as fh:
                json.dump(REC, fh, ensure_ascii=False)
            log("record ->", args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
