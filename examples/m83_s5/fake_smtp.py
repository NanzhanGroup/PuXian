#!/usr/bin/env python3
# M83-S5 假 SMTP 服务器（本地回环收信）—— 记录对话到 out/<n>.txt，便于断言
# 支持：EHLO/HELO、AUTH LOGIN（任意凭据）、MAIL FROM、RCPT TO、DATA、QUIT、NOOP
# 用法：python3 fake_smtp.py <port> <outdir>
import socket
import sys
import os
import threading

def handle(conn, addr, outdir, counter):
    f = conn.makefile("rb")
    w = conn.makefile("wb")
    counter[0] += 1
    n = counter[0]
    def send(line):
        w.write((line + "\r\n").encode())
        w.flush()
    send("220 fake.px ESMTP ready")
    mail_from = None
    rcpts = []
    in_data = False
    data_lines = []
    cur = None
    while True:
        line = f.readline()
        if not line:
            break
        text = line.decode("utf-8", "replace").rstrip("\r\n")
        if in_data:
            if text == ".":
                in_data = False
                # 写邮件文件
                os.makedirs(outdir, exist_ok=True)
                raw = "\n".join(data_lines)
                with open(os.path.join(outdir, "mail_%d.txt" % n), "w", encoding="utf-8") as fo:
                    fo.write("MAIL_FROM: %s\n" % (mail_from or ""))
                    fo.write("RCPT_TO: %s\n" % (", ".join(rcpts)))
                    fo.write(raw)
                data_lines = []
                send("250 2.0.0 OK queued")
            else:
                data_lines.append(text)
            continue
        up = text.upper()
        if up.startswith("EHLO") or up.startswith("HELO"):
            send("250-fake.px")
            send("250-AUTH LOGIN PLAIN")
            send("250 SIZE 10485760")
        elif up.startswith("AUTH LOGIN"):
            send("334 VXNlcm5hbWU6")          # "Username:"
            user = f.readline().decode().rstrip("\r\n")
            send("334 UGFzc3dvcmQ6")          # "Password:"
            pw = f.readline().decode().rstrip("\r\n")
            data_lines.append("AUTHUSER=%s AUTHPASS=%s" % (user, pw))
            send("235 2.7.0 Authentication successful")
        elif up.startswith("MAIL FROM"):
            mail_from = text.split(":", 1)[1].strip() if ":" in text else ""
            send("250 2.1.0 OK")
        elif up.startswith("RCPT TO"):
            rcpts.append(text.split(":", 1)[1].strip() if ":" in text else "")
            send("250 2.1.5 OK")
        elif up.startswith("DATA"):
            in_data = True
            send("354 End data with <CR><LF>.<CR><LF>")
        elif up.startswith("QUIT"):
            send("221 2.0.0 Bye")
            break
        elif up.startswith("NOOP"):
            send("250 2.0.0 OK")
        else:
            send("250 2.0.0 OK")
    try:
        f.close()
    except Exception:
        pass
    try:
        conn.close()
    except Exception:
        pass

def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 2525
    outdir = sys.argv[2] if len(sys.argv) > 2 else "out"
    counter = [0]
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", port))
    srv.listen(16)
    print("fake-smtp listening on %d" % port, flush=True)
    while True:
        conn, addr = srv.accept()
        t = threading.Thread(target=handle, args=(conn, addr, outdir, counter), daemon=True)
        t.start()

if __name__ == "__main__":
    main()
