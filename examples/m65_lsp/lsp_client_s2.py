#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# examples/m65_lsp/lsp_client_s2.py —— M65-S2 LSP 握手 + 诊断 e2e
# ------------------------------------------------------------
# 模拟标准 LSP client（Content-Length 帧双向管道，python3 subprocess），
# 驱动 bootstrap/pxlsp（PX_PXCHECK 指向 bootstrap/pxcheck）：
#   initialize → initialized → didOpen(带 lint) → didChange(干净)
#   → didChange(语法错误) → didClose → shutdown → exit
# 断言 publishDiagnostics 逐项命中（0-based 行/列 + severity/code）。
# 退出码：全部 PASS 0；任一 FAIL 1 并打印明细。
# 用法：lsp_client_s2.py <pxlsp_bin> <pxcheck_bin>
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


class Lsp:
    def __init__(self, proc):
        self.p = proc

    def send(self, msg):
        body = json.dumps(msg, ensure_ascii=False).encode("utf-8")
        frame = ("Content-Length: %d\r\n\r\n" % len(body)).encode("ascii") + body
        self.p.stdin.write(frame)
        self.p.stdin.flush()

    def recv(self, timeout=30):
        """读一帧，返回 dict。超时/EOF 抛异常。"""
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


def main():
    if len(sys.argv) < 3:
        print("用法: lsp_client_s2.py <pxlsp> <pxcheck>")
        return 1
    pxlsp, pxcheck = sys.argv[1], sys.argv[2]
    env = dict(os.environ)
    env["PX_PXCHECK"] = pxcheck
    proc = subprocess.Popen([pxlsp], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            env=env)
    c = Lsp(proc)

    URI = "file:///data/code/puxian/examples/m65_lsp/demo_s2.px"

    # 1) initialize
    c.send({"jsonrpc": "2.0", "id": 1, "method": "initialize",
            "params": {"processId": None, "rootUri": None,
                       "capabilities": {}}})
    r = c.recv()
    check("initialize 响应 id=1", r.get("id") == 1)
    caps = r.get("result", {}).get("capabilities", {})
    ts = caps.get("textDocumentSync", {})
    check("capabilities.textDocumentSync.change == 1(Full)",
          ts.get("change") == 1)
    check("capabilities.textDocumentSync.openClose",
          ts.get("openClose") is True)
    check("serverInfo.name == pxlsp",
          r.get("result", {}).get("serverInfo", {}).get("name") == "pxlsp")

    # 2) initialized（通知）
    c.send({"jsonrpc": "2.0", "method": "initialized", "params": {}})

    # 3) didOpen 带 lint 问题的文档
    lint_src = "def demo():\n    let a = 1\n    print(a)\n    print(undefined_var)\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen",
            "params": {"textDocument": {"uri": URI, "languageId": "px",
                                        "version": 1, "text": lint_src}}})
    r = c.recv()
    check("didOpen → publishDiagnostics", r.get("method") == "textDocument/publishDiagnostics")
    diags = r.get("params", {}).get("diagnostics", [])
    check("didOpen uri 回显", r.get("params", {}).get("uri") == URI)
    codes = [d.get("code") for d in diags]
    check("lint: 含 L002(未定义变量)", "L002" in codes,
          "codes=%s" % codes)
    l002 = [d for d in diags if d.get("code") == "L002"]
    if l002:
        d0 = l002[0]
        line = d0["range"]["start"]["line"]
        check("L002 0-based 行=3", line == 3, "line=%s" % line)
        check("L002 severity=1(Error)", d0.get("severity") == 1,
              "severity=%s" % d0.get("severity"))

    # 4) didChange 干净文本 → 空诊断
    clean_src = "def demo():\n    let a = 1\n    print(a)\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didChange",
            "params": {"textDocument": {"uri": URI},
                       "contentChanges": [{"text": clean_src}]}})
    r = c.recv()
    diags = r.get("params", {}).get("diagnostics", [])
    check("didChange 干净 → 空诊断", diags == [], "diags=%s" % diags)

    # 5) didChange 语法错误 → Error 诊断
    err_src = "def broken(:\n    return 1\n"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didChange",
            "params": {"textDocument": {"uri": URI},
                       "contentChanges": [{"text": err_src}]}})
    r = c.recv()
    diags = r.get("params", {}).get("diagnostics", [])
    check("语法错误 → 有诊断", len(diags) >= 1, "diags=%s" % diags)
    if diags:
        d0 = diags[0]
        check("语法错误 severity=1", d0.get("severity") == 1)
        check("语法错误有 code", isinstance(d0.get("code"), str) and d0["code"] != "")

    # 6) didClose → 空诊断
    c.send({"jsonrpc": "2.0", "method": "textDocument/didClose",
            "params": {"textDocument": {"uri": URI}}})
    r = c.recv()
    diags = r.get("params", {}).get("diagnostics", [])
    check("didClose → 空诊断", diags == [])

    # 7) shutdown → result null
    c.send({"jsonrpc": "2.0", "id": 2, "method": "shutdown", "params": {}})
    r = c.recv()
    check("shutdown id=2", r.get("id") == 2)
    check("shutdown result null", r.get("result") is None and "error" not in r)

    # 8) exit → 服务器退出
    c.send({"jsonrpc": "2.0", "method": "exit", "params": {}})
    try:
        proc.wait(timeout=15)
        check("exit 后服务器退出 rc=0", proc.returncode == 0,
              "rc=%s" % proc.returncode)
    except subprocess.TimeoutExpired:
        check("exit 后服务器退出 rc=0", False, "timeout")
        proc.kill()

    if FAIL:
        print("== S2 client: %d FAIL ==" % len(FAIL))
        for f in FAIL:
            print("  - " + f)
        return 1
    print("== S2 client: ALL PASS ==")
    return 0


if __name__ == "__main__":
    sys.exit(main())
