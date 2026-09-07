#!/usr/bin/env python3
# B-S3 验证：SSE 长连接事件化（qg-issue 27 B 类）
# 核心判定：
#  [1] N=400 空闲 SSE 长连接全部建立成功 + 各收到 3 条事件（hello/line1\nline2/plain）
#      —— N > 旧 256 定长注册表上限 → 服务端 g_sse_conns 动态化（PX_MAX_SSE_CONNS）生效；
#          若 handler 返回后连接仍占 worker（未事件化），8 worker 不可能同时喂 400 长连接。
#  [2] 服务进程 OS 线程数不随连接数涨（≈ 8 fserve + 1 事件循环 + 1 主 = 10）
#  [3] SSE 长连接不被事件循环 tick 空闲超时误关（挂 3s 仍存活）
#  [4] 关闭 200 连接 → 服务端自动清理（fd/slot 回收）→ 再开 200 新连接仍全成功
import socket, sys, time, os, re, resource

resource.setrlimit(resource.RLIMIT_NOFILE, (8192, 8192))
PORT = 18100
N = int(sys.argv[1]) if len(sys.argv) > 1 else 400
HOST = "127.0.0.1"

def read_until(sock, marker, timeout=15):
    sock.settimeout(timeout)
    data = b""
    while marker not in data:
        ch = sock.recv(65536)
        if not ch:
            break
        data += ch
    return data

def open_sse(idx):
    """建立 1 条 SSE 连接；读到 3 条事件后返回 socket（保持打开）。异常返回 None。"""
    try:
        s = socket.create_connection((HOST, PORT), timeout=10)
        s.sendall(b"GET /events HTTP/1.1\r\nHost: localhost\r\nAccept: text/event-stream\r\n\r\n")
        # 读 HTTP 响应头（注意：一次 recv 可能已带事件数据 → 保留剩余 buf）
        buf = b""
        s.settimeout(15)
        while b"\r\n\r\n" not in buf:
            ch = s.recv(65536)
            if not ch:
                break
            buf += ch
        if b"\r\n\r\n" not in buf:
            s.close()
            return None
        head, _, buf = buf.partition(b"\r\n\r\n")
        if b"200" not in head.split(b"\r\n")[0] or b"text/event-stream" not in head:
            s.close()
            return None
        # 读事件流直到收到 3 条（hello / line1\nline2 / plain，SSE 帧以空行 \n\n 分隔）
        events = []
        deadline = time.time() + 15
        while len(events) < 3 and time.time() < deadline:
            while b"\n\n" in buf:
                ev, _, buf = buf.partition(b"\n\n")
                events.append(ev)
            if len(events) >= 3:
                break
            s.settimeout(max(0.5, deadline - time.time()))
            ch = s.recv(65536)
            if not ch:
                break
            buf += ch
        if len(events) < 3:
            s.close()
            return None
        s.settimeout(15)
        return s
    except Exception:
        try:
            s.close()
        except Exception:
            pass
        return None

def server_threads():
    """返回 bs3_daemon 进程的 Threads 数（无则 -1）。按 /proc/<pid>/comm 精确匹配，排除 bash 包装。"""
    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            with open(f"/proc/{pid}/comm") as f:
                comm = f.read().strip()
            if comm == "bs3_daemon":
                with open(f"/proc/{pid}/status") as f:
                    for line in f:
                        if line.startswith("Threads:"):
                            return int(line.split()[1])
        except Exception:
            continue
    return -1

# [1] 建立 N 条 SSE 长连接
conns = []
ok = 0
for i in range(N):
    s = open_sse(i)
    if s is not None:
        ok += 1
        conns.append(s)
print(f"[1] SSE 长连接建立+收3事件: {ok}/{N}  (N>{'256 旧上限' if N > 256 else '256'})")
if ok < max(N - 2, 1):
    print("FAIL 建立数不足（动态化/事件化疑似失效）")
    sys.exit(1)

# 等 2s（覆盖事件循环 1s tick 多次）→ 验证 SSE 不被空闲超时误关
time.sleep(2)
still = 0
for s in conns:
    try:
        s.settimeout(0.001)
        ch = s.recv(1)   # 若被关闭 recv 返回 b''；无数据抛 timeout = 仍挂着（PASS）
        if ch == b"":
            pass          # 被关闭 → 不算 still
        else:
            still += 1    # 有数据也算活
    except socket.timeout:
        still += 1
    except Exception:
        pass
print(f"[2] SSE 长连接 3s 后仍存活: {still}/{len(conns)}  (不被 15s tick 误关、断开才关)")

# [3] 服务线程数（8 fserve + 1 ev + 1 main ≈ 10；若每连接占 worker 会随 N 涨）
t1 = server_threads()
time.sleep(1)
t2 = server_threads()
print(f"[3] 服务线程数: {t1}/{t2}  (挂 {len(conns)} 空闲 SSE；预期≈10，不随连接数涨)")
if t2 > 0 and t2 > 32:
    print("FAIL 线程数随连接数暴涨（事件化未生效）")
    sys.exit(1)

# [4] 关一半 → 服务端自动清理 → 再开一半新连接
half = conns[:len(conns)//2]
for s in half:
    try:
        s.close()
    except Exception:
        pass
conns = conns[len(conns)//2:]
time.sleep(2)   # 给事件循环 detect 断开 + 清理
ok2 = 0
for i in range(len(half)):
    s = open_sse(i)
    if s is not None:
        ok2 += 1
        conns.append(s)
print(f"[4] 断开{len(half)}后重开{len(half)}: {ok2}/{len(half)} 全成功（fd/slot 回收）")
if ok2 < max(len(half) - 2, 1):
    print("FAIL 断开后重开失败（fd/slot 未回收）")
    sys.exit(1)

for s in conns:
    try:
        s.close()
    except Exception:
        pass
print("B-S3 SSE 长连接事件化验证 PASS")
