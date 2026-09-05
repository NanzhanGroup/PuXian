#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# examples/m71_mcp_build/mcp_client.py —— M71-S3 MCP build e2e
# ------------------------------------------------------------
# 模拟标准 MCP client（2024-11-05，stdio + Content-Length 帧）驱动
# `tools/pxc mcp`（经 cmd_mcp 注入 PX_BOOT/PX_PXI/PX_PXC，与真实 agent
# 路径一致）：
#   initialize → notifications/initialized → tools/list（9 工具含 build）
#   → tools/call build（file=hello_build.px）→ 断言产物文本"编译成功"
# 退出码：全 PASS 0；任一 FAIL 1。
# 用法：mcp_client.py
# ============================================================
import json
import os
import select
import subprocess
import sys
import time

FAIL = []


def check(name, cond, detail=""):
    if cond:
        print("PASS: " + name)
    else:
        FAIL.append(name)
        print("FAIL: " + name + ("  " + detail if detail else ""))


class Mcp:
    def __init__(self, proc):
        self.p = proc
        self.next_id = 1

    def send(self, msg):
        body = json.dumps(msg, ensure_ascii=False).encode("utf-8")
        frame = ("Content-Length: %d\r\n\r\n" % len(body)).encode("ascii") + body
        self.p.stdin.write(frame)
        self.p.stdin.flush()

    def recv(self, timeout=150):
        buf = b""
        deadline = time.time() + timeout
        while b"\r\n\r\n" not in buf:
            remain = deadline - time.time()
            if remain <= 0:
                raise TimeoutError("recv header timeout, got=%r" % buf[:200])
            r, _, _ = select.select([self.p.stdout], [], [], remain)
            if not r:
                raise TimeoutError("recv header timeout, got=%r" % buf[:200])
            chunk = os.read(self.p.stdout.fileno(), 65536)
            if not chunk:
                raise EOFError("EOF in header, got=%r" % buf[:200])
            buf += chunk
        head, rest = buf.split(b"\r\n\r\n", 1)
        n = 0
        for line in head.split(b"\r\n"):
            if line.lower().startswith(b"content-length:"):
                n = int(line.split(b":", 1)[1].strip())
        while len(rest) < n:
            remain = deadline - time.time()
            if remain <= 0:
                raise TimeoutError("recv body timeout")
            r, _, _ = select.select([self.p.stdout], [], [], remain)
            if not r:
                raise TimeoutError("recv body timeout")
            chunk = os.read(self.p.stdout.fileno(), 65536)
            if not chunk:
                raise EOFError("EOF in body")
            rest += chunk
        return json.loads(rest[:n].decode("utf-8"))

    def req(self, method, params=None):
        msg = {"jsonrpc": "2.0", "id": self.next_id, "method": method}
        if params is not None:
            msg["params"] = params
        self.next_id += 1
        self.send(msg)
        while True:
            r = self.recv()
            if r.get("method") is None and r.get("id") == msg["id"]:
                return r


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.abspath(os.path.join(here, "..", ".."))
    pxc = os.path.join(root, "tools", "pxc")
    src = os.path.join(here, "hello_build.px")
    out = os.path.join(here, "build", "hello_build")

    proc = subprocess.Popen([pxc, "mcp"], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            env=dict(os.environ))
    c = Mcp(proc)

    # ---------- initialize ----------
    r = c.req("initialize", {"protocolVersion": "2024-11-05",
                             "capabilities": {}, "clientInfo": {"name": "m71"}})
    check("initialize 有 result", "result" in r and "error" not in r)

    # ---------- initialized 通知 ----------
    c.send({"jsonrpc": "2.0", "method": "notifications/initialized"})

    # ---------- tools/list（9 工具含 build）----------
    r = c.req("tools/list")
    tools = r.get("result", {}).get("tools", []) if "result" in r else []
    names = [t.get("name") for t in tools]
    check("tools/list 含 build", "build" in names)
    check("tools/list 恰 9 项", len(tools) == 9)
    for expect in ["run", "fmt", "lint", "test", "bench", "doc", "ast", "version"]:
        check("tools/list 含 " + expect, expect in names)
    by_name = {t["name"]: t for t in tools}
    build_schema = by_name.get("build", {}).get("inputSchema", {})
    props = build_schema.get("properties", {})
    check("build schema 含 file/target/no_quic", "file" in props and "target" in props and "no_quic" in props)

    # ---------- tools/call: build（交付腿）----------
    r = c.req("tools/call", {"name": "build", "arguments": {"file": src}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        print("build 输出: " + txt.strip().replace("\n", " | ")[:200])
        check("build isError=false", res.get("isError") is False)
        check("build 文本含 编译成功", "编译成功" in txt)
    else:
        check("tools/call build 有 result", False, str(r))

    # ---------- tools/call: build 错误路径（坏代码 → isError）----------
    r = c.req("tools/call", {"name": "build",
                             "arguments": {"code": "def main():\n    print(1\n"}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("build 坏代码 isError=true", res.get("isError") is True)
        check("build 坏代码文本含错误", ("错误" in txt) or ("编译失败" in txt) or ("E2" in txt))
    else:
        check("tools/call build 坏代码有 result", False, str(r))

    # ---------- shutdown / exit ----------
    c.send({"jsonrpc": "2.0", "method": "shutdown"})
    c.send({"jsonrpc": "2.0", "method": "exit"})
    try:
        proc.wait(timeout=5)
    except Exception:
        proc.kill()

    if FAIL:
        print("== M71-S3 MCP build: FAILED ==")
        return 1
    print("== M71-S3 MCP build: ALL PASS（MCP build 交付闭环）==")
    return 0


if __name__ == "__main__":
    sys.exit(main())
