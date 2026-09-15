#!/usr/bin/env python3
# m123 断连放大复现器（对齐 ws-approve/requirements/bigdisc.py 的形态）
#   判据：客户端「完整送达请求后立即断开（可选 RST）」→ 统计服务端该请求**总 CPU**
#   （utime+stime），直到服务端回到空闲（CPU 连续 0.5s 不增长）。
# 用法：bigdisc.py <unix_sock> <pid> <route> <KB> <并发> [rst]
import socket, sys, time, os, threading

sock_path, pid, route, kb, conc = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4]), int(sys.argv[5])
rst = len(sys.argv) > 6 and sys.argv[6] == "rst"
body = b"x" * (kb * 1024)
req = ("POST %s HTTP/1.1\r\nHost: px\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n"
       % (route, len(body))).encode() + body
HZ = os.sysconf("SC_CLK_TCK")

def cpu(pid):
    with open("/proc/%d/stat" % pid) as f:
        p = f.read().split()
    return (int(p[13]) + int(p[14])) / HZ

def client(i):
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.connect(sock_path)
    if rst:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, b"\x01\x00\x00\x00\x00\x00\x00\x00")
    s.sendall(req)          # 完整请求送达（服务端已进入 handler）
    s.close()               # 立即断开，不读响应

def wait_idle(pid, quiet=0.5, limit=30.0):
    t0, last, stable = time.time(), cpu(pid), 0.0
    while time.time() - t0 < limit:
        time.sleep(0.1)
        c = cpu(pid)
        if c - last < 0.005:
            stable += 0.1
            if stable >= quiet: return
        else:
            stable = 0.0
        last = c

wait_idle(pid)                       # 等基线空闲
c0 = cpu(pid)
ts = [threading.Thread(target=client, args=(i,)) for i in range(conc)]
for t in ts: t.start()
for t in ts: t.join()
t_disc = time.time()
wait_idle(pid)                       # 等到服务端真正空闲
c1 = cpu(pid)
print("route=%-5s KB=%-5d 并发=%-2d rst=%-5s  服务端总 CPU = %6.2fs  （客户端于发送完毕即断开）"
      % (route, kb, conc, rst, c1 - c0))
print("CPU_SECONDS %.3f" % (c1 - c0))
