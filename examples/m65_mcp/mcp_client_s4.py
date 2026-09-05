#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# examples/m65_mcp/mcp_client_s4.py —— M65-S4 MCP 服务器 e2e 验收
# ------------------------------------------------------------
# 模拟标准 MCP client（2024-11-05，stdio + Content-Length 帧）驱动
# bootstrap/pxmcp：
#   initialize → notifications/initialized → tools/list（8 工具 + schema）
#   → tools/call 每工具各一次成功回包 + 错误参数回 isError
#   → shutdown → exit
# 工具执行真实落盘验证：run/fmt/lint/ast 用 code（写临时文件），
# test/bench/doc 用真实样本 examples/m65_mcp/demo_mcp.px（file 参数）。
# 退出码：全部 PASS 0；任一 FAIL 1 并打印明细。
# 用法：mcp_client_s4.py <pxmcp_bin> <bootstrap_dir> <demo_file>
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

    def recv(self, timeout=90):
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
        """request → 等匹配 id 的响应（跳过通知帧）。"""
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
    if len(sys.argv) < 4:
        print("用法: mcp_client_s4.py <pxmcp> <bootstrap_dir> <demo_file>")
        return 1
    pxmcp, boot, demo = sys.argv[1], sys.argv[2], sys.argv[3]
    env = dict(os.environ)
    env["PX_BOOT"] = boot          # pxmcp spawn 各工具二进制（pxi/pxfmt/pxlint/...）
    env["PX_PXI"] = os.path.join(boot, "pxi")  # test/bench 子进程需解释器
    proc = subprocess.Popen([pxmcp], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            env=env)
    c = Mcp(proc)

    # ---------- initialize ----------
    r = c.req("initialize", {"protocolVersion": "2024-11-05",
                             "capabilities": {}, "clientInfo": {"name": "test"}})
    check("initialize 有 result", "result" in r and "error" not in r)
    if "result" in r:
        res = r["result"]
        check("initialize protocolVersion", res.get("protocolVersion") == "2024-11-05")
        caps = res.get("capabilities", {})
        check("initialize capabilities.tools", isinstance(caps.get("tools"), dict))
        tc = caps.get("tools", {})
        check("initialize tools.listChanged=false", tc.get("listChanged") is False)
        si = res.get("serverInfo", {})
        check("initialize serverInfo.name=px-mcp", si.get("name") == "px-mcp")

    # ---------- initialized 通知（无响应即可，客户端不 recv）----------
    c.send({"jsonrpc": "2.0", "method": "notifications/initialized"})

    # ---------- tools/list ----------
    r = c.req("tools/list")
    check("tools/list 有 result", "result" in r and "error" not in r)
    tools = []
    if "result" in r:
        tools = r["result"].get("tools", [])
        names = [t.get("name") for t in tools]
        for expect in ["run", "fmt", "lint", "test", "bench", "doc", "ast", "version", "build"]:
            check("tools/list 含 " + expect, expect in names)
        check("tools/list 恰 9 项（M71 扩展 build）", len(tools) == 9)
        # schema 结构抽查
        for t in tools:
            sch = t.get("inputSchema", {})
            if sch.get("type") != "object" or not isinstance(sch.get("properties"), dict):
                check("tools/list schema " + str(t.get("name")), False)
                break
        else:
            check("tools/list 每项 inputSchema 结构正确", True)
        # required 抽查：test 必填 file；version 无必填
        by_name = {t["name"]: t for t in tools}
        req_test = by_name["test"]["inputSchema"].get("required", [])
        check("tools/list test.required=[file]", req_test == ["file"])
        req_ver = by_name["version"]["inputSchema"].get("required", [])
        check("tools/list version 无必填", req_ver == [])

    # ---------- tools/call: version ----------
    r = c.req("tools/call", {"name": "version", "arguments": {}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("version isError=false", res.get("isError") is False)
        check("version 文本含 pxc", "pxc 0.1.0" in txt)
    else:
        check("version 有 result", False)

    # ---------- tools/call: run（code）----------
    code_run = 'def main():\n    print("hi-mcp-run")\n'
    r = c.req("tools/call", {"name": "run", "arguments": {"code": code_run}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("run isError=false", res.get("isError") is False)
        check("run 输出含 hi-mcp-run", "hi-mcp-run" in txt)
    else:
        check("run 有 result", False)

    # ---------- tools/call: fmt（code：待格式化）----------
    code_fmt = "def f(a,b):\n return a+b\n"
    r = c.req("tools/call", {"name": "fmt", "arguments": {"code": code_fmt}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("fmt isError=false", res.get("isError") is False)
        check("fmt 输出已格式化", "def f(a, b):" in txt and "    return a + b" in txt)
    else:
        check("fmt 有 result", False)

    # ---------- tools/call: lint（file：干净样本 → rc0）----------
    r = c.req("tools/call", {"name": "lint", "arguments": {"file": demo}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("lint 干净样本 isError=false", res.get("isError") is False)
        check("lint 输出 0 错误", "0 错误" in txt)
    else:
        check("lint 有 result", False)

    # ---------- tools/call: test（file 样本）----------
    r = c.req("tools/call", {"name": "test", "arguments": {"file": demo}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("test isError=false", res.get("isError") is False)
        check("test 结果 2/2 通过", "2/2 通过" in txt)
    else:
        check("test 有 result", False)

    # ---------- tools/call: bench（file 样本 + func work：bench 约定无参）----------
    r = c.req("tools/call", {"name": "bench",
                             "arguments": {"file": demo, "func": "work"}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("bench isError=false", res.get("isError") is False)
        check("bench 输出含 汇总", "汇总" in txt)
    else:
        check("bench 有 result", False)

    # ---------- tools/call: doc（file 样本）----------
    r = c.req("tools/call", {"name": "doc", "arguments": {"file": demo}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("doc isError=false", res.get("isError") is False)
        check("doc 输出含 add 标题", "add" in txt and "# " in txt)
    else:
        check("doc 有 result", False)

    # ---------- tools/call: ast（code）----------
    code_ast = "def main():\n    return 0\n"
    r = c.req("tools/call", {"name": "ast", "arguments": {"code": code_ast}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("ast isError=false", res.get("isError") is False)
        check("ast 输出含 Program", "Program" in txt)
    else:
        check("ast 有 result", False)

    # ---------- 错误输入 ----------
    r = c.req("tools/call", {"name": "lint", "arguments": {}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("lint 缺参 isError=true", res.get("isError") is True)
        check("lint 缺参错误消息", "缺少参数" in txt)
    else:
        check("lint 缺参有 result", False)
    r = c.req("tools/call", {"name": "nosuch", "arguments": {}})
    if "result" in r:
        res = r["result"]
        txt = res["content"][0]["text"] if res.get("content") else ""
        check("未知工具 isError=true", res.get("isError") is True)
        check("未知工具错误消息", "未知工具" in txt)
    else:
        check("未知工具有 result", False)
    # 未知方法（非工具）→ JSON-RPC -32601 error
    r = c.req("bogus/method", {})
    check("未知方法 JSON-RPC error -32601",
          r.get("error", {}).get("code") == -32601)

    # ---------- shutdown → exit ----------
    r = c.req("shutdown")
    check("shutdown result 空对象", r.get("result") == {})
    c.send({"jsonrpc": "2.0", "method": "exit"})
    rc = proc.wait(timeout=20)
    check("exit 后进程 rc=0", rc == 0)

    err = proc.stderr.read().decode("utf-8", "replace")
    if err.strip():
        print("stderr: " + err[:500])
    print("")
    if FAIL:
        print("MCP-S4: %d FAIL" % len(FAIL))
        return 1
    print("MCP-S4: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
