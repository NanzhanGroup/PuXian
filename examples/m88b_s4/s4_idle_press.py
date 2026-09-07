#!/usr/bin/env python3
# B-S4 量级验证：N 空闲 keep-alive 连接同时挂载，进程 OS 线程数 ≈ 池容量 + 常数（不随连接数涨）
#   —— M88_PLAN §五 B 类验收①：1 万 idle keep-alive 连接建立，线程数不随连接数线性涨。
# 约束：http keep-alive 空闲 15s 超时（事件循环 tick 关）→ 须在 15s 窗口内完成建立 + 计数，
#       用并发建立加速（默认 N=10000，16 并发 × 625）。
# 用法：python3 s4_idle_press.py [N] [并发]
import socket, sys, time, os, resource
from concurrent.futures import ThreadPoolExecutor

resource.setrlimit(resource.RLIMIT_NOFILE, (65535, 65535))
PORT = 18099
N = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
CONC = int(sys.argv[2]) if len(sys.argv) > 2 else 16
HOST = "127.0.0.1"

def server_threads():
    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            with open(f"/proc/{pid}/comm") as f:
                if f.read().strip() == "s4_daemon":
                    with open(f"/proc/{pid}/status") as f:
                        for line in f:
                            if line.startswith("Threads:"):
                                return int(line.split()[1])
        except Exception:
            continue
    return -1

def one(i):
    """建 1 条 keep-alive 连接：首请求 + 立即第二请求（突发复用，均须 200）→ 保持空闲返回 socket。
    第二请求发生在本连接刚交还 IDLE 后立刻可读 → 事件循环 detect + 派发回池 = 突发复用窗口内实证。
    失败返回 None。"""
    try:
        s = socket.create_connection((HOST, PORT), timeout=15)
        s.settimeout(15)
        # 首请求
        s.sendall(b"GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n")
        if read_resp_ok(s) != 1:
            s.close()
            return None
        # 立即第二请求（空闲连接复用/突发）
        s.sendall(b"GET /health HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n")
        if read_resp_ok(s) != 1:
            s.close()
            return None
        return s
    except Exception:
        try:
            s.close()
        except Exception:
            pass
        return None

def read_resp_ok(s):
    """读一个 HTTP 响应（头 + Content-Length body），200 返回 1，否则 0。"""
    try:
        data = b""
        while b"\r\n\r\n" not in data:
            ch = s.recv(65536)
            if not ch:
                return 0
            data += ch
        head, _, rest = data.partition(b"\r\n\r\n")
        if b"200" not in head.split(b"\r\n")[0]:
            return 0
        clen = 0
        for line in head.split(b"\r\n"):
            if line.lower().startswith(b"content-length:"):
                clen = int(line.split(b":")[1].strip())
        body = rest
        while len(body) < clen:
            ch = s.recv(65536)
            if not ch:
                break
            body += ch
        return 1 if body[:clen] == b"ok-s4" else 0
    except Exception:
        return 0

t0 = time.time()
socks = []
with ThreadPoolExecutor(max_workers=CONC) as ex:
    for s in ex.map(one, range(N)):
        socks.append(s)
t1 = time.time()
ok = sum(1 for s in socks if s is not None)
print(f"[1] 建立+首请求+突发复用均200: {ok}/{N}  用时 {t1-t0:.1f}s", flush=True)
print(f"    (http keep-alive 空闲 15s 由事件循环 tick 关闭=设计语义；超窗连接被服务端正常关闭)", flush=True)

th = server_threads()
print(f"[2] 服务线程数: {th}  (挂 {ok} 空闲连接; 池=8 预期≈10，不随连接数涨)", flush=True)

# 保持 2s 后再数一次（确认线程稳定不涨）
time.sleep(2)
th2 = server_threads()
print(f"[3] 2s 后线程数: {th2}", flush=True)

for s in socks:
    try:
        s.close()
    except Exception:
        pass

pass_ok = (ok >= 9000) and (th == th2 and 0 < th <= 64)
print(f"[4] 判定: ok={ok}(>=9000) 线程恒定={th == th2}({th})")
print("B-S4 万级空闲连接验证 " + ("PASS" if pass_ok else "FAIL"))
sys.exit(0 if pass_ok else 1)
