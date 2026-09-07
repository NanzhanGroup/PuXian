#!/usr/bin/env python3
# B-S2 验证：N 空闲 keep-alive 长连接 + 突发全 200（qg-issue 27 B 类）
# 核心判定：池容量 8（PX_SERVE_WORKERS=8）却能挂 200+ 空闲连接且首请求/突发全 200
#           = 空闲连接交还 IDLE（事件循环照看）不占 worker；A 类 8 worker 只能同时喂 8 连接。
# 另验证：① 线程数不随连接数涨（nlwp ≈ 池+常数）② 15s 空闲超时语义保留（16s 后服务端关闭空闲连接）
import socket, sys, time, os, re

PORT = 18099
N_IDLE = int(sys.argv[1]) if len(sys.argv) > 1 else 200
BURST = int(sys.argv[2]) if len(sys.argv) > 2 else 50
HOST = "127.0.0.1"

def recv_http(sock, timeout=30):
    """读一个 HTTP 响应（含 body），返回 (status_line, body)"""
    sock.settimeout(timeout)
    data = b""
    while b"\r\n\r\n" not in data:
        chunk = sock.recv(65536)
        if not chunk:
            break
        data += chunk
    if b"\r\n\r\n" not in data:
        return None, data
    head, _, rest = data.partition(b"\r\n\r\n")
    status = head.split(b"\r\n")[0].decode(errors="replace")
    clen = 0
    for line in head.split(b"\r\n"):
        if line.lower().startswith(b"content-length:"):
            clen = int(line.split(b":")[1].strip())
    body = rest
    while len(body) < clen:
        chunk = sock.recv(65536)
        if not chunk:
            break
        body += chunk
    return status, body[:clen]

# 1) 建立 N 个 keep-alive 连接，各发首请求并保持空闲
conns = []
ok_first = 0
for i in range(N_IDLE):
    try:
        s = socket.create_connection((HOST, PORT), timeout=10)
        req = b"GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
        s.sendall(req)
        st, body = recv_http(s, timeout=30)
        if st and "200" in st and body == b"ok-bs2":
            ok_first += 1
        conns.append(s)  # 无论首请求成败都挂住观察（失败多半是连接被关）
    except Exception as e:
        # 连接失败也记录（区分 A 类排队超时）
        conns.append(None)

print(f"[1] 首请求: {ok_first}/{N_IDLE} 返回 200 (池容量=8)")

# 2) 线程数：不随连接数涨（空闲连接事件循环照看）
time.sleep(1)
try:
    with open("/proc/self/status") as f:
        pass
except Exception:
    pass
threads_line = ""
for pid_path in os.listdir("/proc"):
    if not pid_path.isdigit():
        continue
    try:
        with open(f"/proc/{pid_path}/cmdline", "rb") as f:
            cmd = f.read().replace(b"\x00", b" ").decode(errors="replace")
        if "bs2_daemon" in cmd:
            with open(f"/proc/{pid_path}/status") as f:
                for line in f:
                    if line.startswith("Threads:"):
                        threads_line = f"pid={pid_path} " + line.strip()
            break
    except Exception:
        continue
print(f"[2] 服务线程数: {threads_line}   (空闲连接={N_IDLE})")

# 3) 突发：挑 BURST 个空闲连接发第二个请求 → 全 200
ok_burst = 0
alive = [s for s in conns if s is not None]
for i, s in enumerate(alive[:BURST]):
    try:
        req = b"GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
        s.sendall(req)
        st, body = recv_http(s, timeout=10)
        if st and "200" in st and body == b"ok-bs2":
            ok_burst += 1
    except Exception:
        pass
print(f"[3] 突发请求(空闲连接复用): {ok_burst}/{min(BURST, len(alive))} 返回 200")

# 4) 15s 空闲超时语义保留：挂 1 个空闲连接 16s → 服务端应关闭（recv 返回空）
if conns and conns[0] is not None:
    s = conns[0]
    try:
        s.settimeout(20)
        # 排空任何残留后纯空闲等待
        time.sleep(16)
        s.sendall(b"GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n")
        data = s.recv(65536)
        if data == b"":
            print("[4] 15s 空闲超时语义: PASS（服务端 16s 后关闭空闲连接）")
        else:
            print(f"[4] 15s 空闲超时语义: 连接仍存活(收到 {len(data)}B) —— 检查事件循环 tick")
    except socket.timeout:
        print("[4] 15s 空闲超时语义: 读超时（连接仍挂）")
    except Exception:
        print("[4] 15s 空闲超时语义: 连接已关闭（PASS 变体）")
else:
    print("[4] 跳过（无存活连接）")

for s in conns:
    try:
        s.close()
    except Exception:
        pass
print("DONE")
