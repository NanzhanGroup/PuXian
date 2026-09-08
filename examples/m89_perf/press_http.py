#!/usr/bin/env python3
# m89_perf/press_http.py —— VM vs C 轨性能基线 · HTTP 服务并发压测客户端（纯标准库）
# 用法：python3 press_http.py <port> [--threads N] [--reqs N] [--path /api/check]
# 每线程一条 TCP 长连接（HTTP/1.1 keep-alive），连续发送 GET，读完
# Content-Length 响应体后记录单请求耗时。输出 RPS / p50 / p95 / p99 / max / 错误数。
# 结果 stdout 单行 JSON（供 bench 脚本解析）。

import socket
import sys
import time
import json
import threading

def parse_args(argv):
    port = int(argv[0])
    threads = 50
    reqs = 200
    path = "/api/check"
    timeout = 30
    i = 1
    while i < len(argv):
        a = argv[i]
        if a == "--threads":
            threads = int(argv[i + 1]); i += 2
        elif a == "--reqs":
            reqs = int(argv[i + 1]); i += 2
        elif a == "--path":
            path = argv[i + 1]; i += 2
        elif a == "--timeout":
            timeout = int(argv[i + 1]); i += 2
        else:
            i += 1
    return port, threads, reqs, path, timeout

def recv_http_resp(sock):
    """读一个 HTTP/1.1 响应（支持 Content-Length），返回 (status, ms) 计时在调用方。
    返回状态行文本。"""
    buf = b""
    while b"\r\n\r\n" not in buf:
        chunk = sock.recv(65536)
        if not chunk:
            raise ConnectionError("conn closed in header")
        buf += chunk
    head, _, rest = buf.partition(b"\r\n\r\n")
    status_line = head.split(b"\r\n")[0].decode("latin1")
    clen = 0
    for line in head.split(b"\r\n"):
        if line.lower().startswith(b"content-length:"):
            clen = int(line.split(b":", 1)[1].strip())
    while len(rest) < clen:
        chunk = sock.recv(65536)
        if not chunk:
            raise ConnectionError("conn closed in body")
        rest += chunk
    return status_line

def worker(host, port, path, reqs, lat, lock, errs, start_evt, timeout):
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
    except Exception as e:
        with lock:
            errs.append("conn:%s" % e)
        return
    try:
        sock.settimeout(timeout)
        req = ("GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\n\r\n"
               % (path, host, port)).encode()
        start_evt.wait()
        for _ in range(reqs):
            t0 = time.perf_counter()
            sock.sendall(req)
            st = recv_http_resp(sock)
            lat.append(time.perf_counter() - t0)
            if " 200 " not in st:
                with lock:
                    errs.append("status:" + st)
    except Exception as e:
        with lock:
            errs.append("req:%s" % e)
    finally:
        try:
            sock.close()
        except Exception:
            pass

def main():
    port, threads, reqs, path, timeout = parse_args(sys.argv[1:])
    host = "127.0.0.1"
    lat = []
    errs = []
    lock = threading.Lock()
    start_evt = threading.Event()
    ts = []
    for _ in range(threads):
        t = threading.Thread(target=worker, args=(host, port, path, reqs, lat, lock, errs, start_evt, timeout))
        t.daemon = True
        ts.append(t)
        t.start()
    time.sleep(0.3)
    t0 = time.perf_counter()
    start_evt.set()
    for t in ts:
        t.join()
    wall = time.perf_counter() - t0
    n = len(lat)
    total = threads * reqs
    lat_s = sorted(lat)
    def pct(p):
        if not lat_s:
            return 0.0
        return lat_s[min(len(lat_s) - 1, int(len(lat_s) * p))] * 1000
    out = {
        "threads": threads, "reqs": reqs, "total": total, "ok": n, "errs": len(errs),
        "wall_s": round(wall, 3), "rps": round(n / wall, 1) if wall > 0 else 0,
        "p50_ms": round(pct(0.50), 2), "p95_ms": round(pct(0.95), 2),
        "p99_ms": round(pct(0.99), 2), "max_ms": round(lat_s[-1] * 1000, 2) if lat_s else 0,
        "err_detail": errs[:3],
    }
    print(json.dumps(out))

if __name__ == "__main__":
    main()
