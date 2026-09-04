#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# examples/m65_lsp/lsp_client_s3.py —— M65-S3 completion/definition/hover e2e
# ------------------------------------------------------------
# 模拟标准 LSP client（Content-Length 帧双向管道）驱动 bootstrap/pxlsp：
#   样本 A demo_s3.px（受控：def/struct/enum/trait/impl/var/局部/文档注释）
#     → completion（全量候选 + 前缀过滤 + kind/detail）
#     → definition（调用点 → 定义行；trait 名 → trait 行）
#     → hover（符号签名+文档注释 / 类型 / 无 doc / 内置 / 关键字）
#   样本 B selfhost/astdump.px（真实文件，无 import 链）
#     → completion 候选命中真实顶层 def；hover main 签名
#   错误输入不崩：未 didOpen uri、越界位置 → 正常响应
#   → shutdown → exit
# 退出码：全部 PASS 0；任一 FAIL 1 并打印明细。
# 用法：lsp_client_s3.py <pxlsp_bin> <pxcheck_bin> <repo_root>
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

    def recv(self, timeout=40):
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

    def req(self, msg):
        """请求-响应：等待匹配 id 的响应（跳过通知帧）。"""
        self.send(msg)
        while True:
            r = self.recv()
            if r.get("method") is None and r.get("id") == msg["id"]:
                return r


def find_top_def(src, name):
    """扫源码中顶层 'def name(' 定义行，返回 (line0, char0) 或 None。"""
    for i, ln in enumerate(src.split("\n")):
        t = ln.strip()
        if (t.startswith("def " + name + "(")
                or t.startswith("pub def " + name + "(")
                or t.startswith("extern def " + name + "(")):
            return i, ln.index(name)
    return None


def top_def_names(src):
    out = []
    for ln in src.split("\n"):
        t = ln.strip()
        if t.startswith("def ") and "(" in t:
            nm = t[4:].split("(", 1)[0].strip()
            if nm and nm not in out:
                out.append(nm)
    return out


def main():
    if len(sys.argv) < 4:
        print("用法: lsp_client_s3.py <pxlsp> <pxcheck> <repo_root>")
        return 1
    pxlsp, pxcheck, repo = sys.argv[1], sys.argv[2], sys.argv[3]
    env = dict(os.environ)
    env["PX_PXCHECK"] = pxcheck
    proc = subprocess.Popen([pxlsp], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            env=env)
    c = Lsp(proc)

    def hover_at(uri, src, line0, char0):
        rr = c.req({"jsonrpc": "2.0", "id": 99, "method": "textDocument/hover",
                    "params": {"textDocument": {"uri": uri},
                               "position": {"line": line0, "character": char0}}})
        res = rr.get("result")
        return (res or {}).get("contents", {}).get("value", "")

    # ---------- 样本 A：demo_s3.px（受控符号集合） ----------
    demo = open(os.path.join(repo, "examples/m65_lsp/demo_s3.px"),
                encoding="utf-8").read()
    DURI = "file:///data/code/puxian/examples/m65_lsp/demo_s3.px"

    r = c.req({"jsonrpc": "2.0", "id": 1, "method": "initialize",
               "params": {"processId": None, "rootUri": None,
                          "capabilities": {}}})
    check("initialize id=1", r.get("id") == 1)
    caps = r.get("result", {}).get("capabilities", {})
    check("capabilities.completionProvider", caps.get("completionProvider") is True)
    check("capabilities.definitionProvider", caps.get("definitionProvider") is True)
    check("capabilities.hoverProvider", caps.get("hoverProvider") is True)
    c.send({"jsonrpc": "2.0", "method": "initialized", "params": {}})

    # didOpen demo：样本语法干净 → 诊断 []
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen",
            "params": {"textDocument": {"uri": DURI, "languageId": "px",
                                        "version": 1, "text": demo}}})
    r = c.recv()
    check("didOpen → publishDiagnostics(干净 [])",
          r.get("method") == "textDocument/publishDiagnostics"
          and r.get("params", {}).get("diagnostics", []) == [],
          "diags=%s" % r.get("params", {}).get("diagnostics", [])[:2])

    # ---------- completion：全量候选命中各来源 ----------
    r = c.req({"jsonrpc": "2.0", "id": 2, "method": "textDocument/completion",
               "params": {"textDocument": {"uri": DURI},
                          "position": {"line": 0, "character": 0}}})
    items = r.get("result", {}).get("items", [])
    labels = [it.get("label") for it in items]
    check("completion 全量含关键字 let", "let" in labels)
    check("completion 全量含内置 print", "print" in labels)
    check("completion 全量含 std each", "each" in labels)
    check("completion 全量含顶层 def calc_add", "calc_add" in labels)
    check("completion 全量含顶层 struct Point", "Point" in labels)
    check("completion 全量含顶层 enum Color", "Color" in labels)
    check("completion 全量含顶层 trait Speaker", "Speaker" in labels)
    check("completion 全量含顶层 var g_ready", "g_ready" in labels)
    check("completion 全量含局部 let local_var", "local_var" in labels)
    # calc_add 存在 kind=Function/detail=签名 的候选（另有一份 locals 宽松候选属正常）
    ca = [it for it in items if it.get("label") == "calc_add"]
    check("calc_add 候选含 Function 签名项",
          any(it.get("kind") == 3 and it.get("detail") == "def calc_add(a, b) -> int"
              for it in ca),
          "details=%s" % [it.get("detail") for it in ca])
    pt = [it for it in items if it.get("label") == "Point"][0]
    check("Point kind=7(Class)", pt.get("kind") == 7)
    spk = [it for it in items if it.get("label") == "Speaker"][0]
    check("Speaker kind=7(Class)", spk.get("kind") == 7)
    lv = [it for it in items if it.get("label") == "local_var"][0]
    check("local_var kind=6(Variable)", lv.get("kind") == 6)

    # ---------- completion：前缀过滤（光标在 "loc" 词尾） ----------
    lv_line = next(i for i, ln in enumerate(demo.split("\n"))
                   if "local_var = 42" in ln)
    lv_char = demo.split("\n")[lv_line].index("local_var") + 3
    r = c.req({"jsonrpc": "2.0", "id": 3, "method": "textDocument/completion",
               "params": {"textDocument": {"uri": DURI},
                          "position": {"line": lv_line, "character": lv_char}}})
    items = r.get("result", {}).get("items", [])
    check("completion 前缀过滤 loc → 仅 local_var",
          len(items) == 1 and items[0].get("label") == "local_var",
          "items=%s" % [it.get("label") for it in items])

    # ---------- definition：调用点 → 定义行 ----------
    call_line = next(i for i, ln in enumerate(demo.split("\n"))
                     if "calc_add(local_var, item)" in ln)
    call_char = demo.split("\n")[call_line].index("calc_add") + 1
    r = c.req({"jsonrpc": "2.0", "id": 4, "method": "textDocument/definition",
               "params": {"textDocument": {"uri": DURI},
                          "position": {"line": call_line, "character": call_char}}})
    res = r.get("result")
    exp = find_top_def(demo, "calc_add")
    check("definition 命中 def calc_add 行",
          res is not None and res.get("uri") == DURI
          and res["range"]["start"]["line"] == exp[0]
          and res["range"]["start"]["character"] == exp[1],
          "res=%s exp=%s" % (res, exp))

    # ---------- definition：impl 行引用 trait 名 → trait 定义行 ----------
    impl_line = next(i for i, ln in enumerate(demo.split("\n"))
                     if ln.strip().startswith("impl Speaker for Point"))
    tr_line = next(i for i, ln in enumerate(demo.split("\n"))
                   if ln.strip().startswith("trait Speaker"))
    r = c.req({"jsonrpc": "2.0", "id": 5, "method": "textDocument/definition",
               "params": {"textDocument": {"uri": DURI},
                          "position": {"line": impl_line,
                                       "character": demo.split("\n")[impl_line].index("Speaker") + 1}}})
    res = r.get("result")
    check("definition Speaker(impl 引用) → trait 行",
          res is not None and res["range"]["start"]["line"] == tr_line,
          "res=%s tr_line=%s" % (res, tr_line))

    # ---------- hover ----------
    dl, dc = find_top_def(demo, "calc_add")
    hv = hover_at(DURI, demo, dl, dc + 1)
    check("hover calc_add 含签名", "**calc_add**" in hv
          and "def calc_add(a, b) -> int" in hv, "hv=%r" % hv[:160])
    check("hover calc_add 含文档注释", "计算两数和" in hv and "文档注释验证 hover" in hv,
          "hv=%r" % hv[:200])
    sp = next(i for i, ln in enumerate(demo.split("\n"))
              if ln.strip().startswith("struct Point"))
    hv = hover_at(DURI, demo, sp, demo.split("\n")[sp].index("Point") + 1)
    check("hover Point 含类型", "**Point**" in hv and "struct Point" in hv,
          "hv=%r" % hv[:120])
    ep = next(i for i, ln in enumerate(demo.split("\n"))
              if ln.strip().startswith("enum Color"))
    hv = hover_at(DURI, demo, ep, demo.split("\n")[ep].index("Color") + 1)
    check("hover Color（无 doc）不崩", "**Color**" in hv and "enum Color" in hv,
          "hv=%r" % hv[:120])
    hv = hover_at(DURI, demo, tr_line, demo.split("\n")[tr_line].index("Speaker") + 1)
    check("hover trait Speaker 含文档注释",
          "**Speaker**" in hv and "trait Speaker" in hv and "说话能力 trait" in hv,
          "hv=%r" % hv[:160])
    pp = next(i for i, ln in enumerate(demo.split("\n")) if "print(item)" in ln)
    hv = hover_at(DURI, demo, pp, demo.split("\n")[pp].index("print") + 1)
    check("hover 内置 print", "**print**" in hv and "内置函数" in hv, "hv=%r" % hv[:120])
    kp = next(i for i, ln in enumerate(demo.split("\n")) if "let local_var = 42" in ln)
    hv = hover_at(DURI, demo, kp, demo.split("\n")[kp].index("let") + 1)
    check("hover 关键字 let", "**let**" in hv and "关键字" in hv, "hv=%r" % hv[:120])

    # ---------- 样本 B：selfhost/astdump.px（真实文件，无 import 链） ----------
    ast = open(os.path.join(repo, "selfhost/astdump.px"), encoding="utf-8").read()
    AURI = "file:///data/code/puxian/selfhost/astdump.px"
    c.send({"jsonrpc": "2.0", "method": "textDocument/didOpen",
            "params": {"textDocument": {"uri": AURI, "languageId": "px",
                                        "version": 1, "text": ast}}})
    r = c.recv()
    check("astdump.px didOpen → 诊断帧",
          r.get("method") == "textDocument/publishDiagnostics")
    r = c.req({"jsonrpc": "2.0", "id": 6, "method": "textDocument/completion",
               "params": {"textDocument": {"uri": AURI},
                          "position": {"line": 0, "character": 0}}})
    labels = [it.get("label") for it in r.get("result", {}).get("items", [])]
    names = top_def_names(ast)
    check("astdump.px 真实 def 名入候选 (%s)" % names[0],
          names and names[0] in labels and names[1] in labels,
          "labels 含 %s" % names[:3])
    check("astdump.px 补全含真实 def pad", "pad" in labels)
    # hover 真实 def pad（无 doc 不崩）→ 签名 def pad(n)
    pl = next(i for i, ln in enumerate(ast.split("\n"))
              if ln.strip().startswith("def pad("))
    hv = hover_at(AURI, ast, pl, ast.split("\n")[pl].index("pad") + 1)
    check("astdump.px hover pad 含签名", "def pad(n)" in hv, "hv=%r" % hv[:120])
    # definition：pad( 在 dump_node 内被调用 → 跳 pad 定义行
    ref = next(i for i, ln in enumerate(ast.split("\n"))
               if "pad(" in ln and not ln.strip().startswith("def pad("))
    r = c.req({"jsonrpc": "2.0", "id": 7, "method": "textDocument/definition",
               "params": {"textDocument": {"uri": AURI},
                          "position": {"line": ref,
                                       "character": ast.split("\n")[ref].index("pad") + 1}}})
    res = r.get("result")
    check("astdump.px definition pad → 定义行",
          res is not None and res["range"]["start"]["line"] == pl,
          "res=%s pl=%s" % (res, pl))

    # ---------- 错误输入不崩 ----------
    for meth in ("textDocument/completion", "textDocument/definition",
                 "textDocument/hover"):
        rr = c.req({"jsonrpc": "2.0", "id": 8, "method": meth,
                    "params": {"textDocument": {"uri": "file:///nope.px"},
                               "position": {"line": 0, "character": 0}}})
        check("未知文档 %s 正常响应" % meth.split("/")[-1], "id" in rr and "error" not in rr,
              "r=%s" % rr)
    rr = c.req({"jsonrpc": "2.0", "id": 9, "method": "textDocument/hover",
                "params": {"textDocument": {"uri": DURI},
                           "position": {"line": 99999, "character": 99999}}})
    check("越界位置 hover result null", rr.get("result") is None, "r=%s" % rr)

    # ---------- shutdown / exit ----------
    r = c.req({"jsonrpc": "2.0", "id": 10, "method": "shutdown", "params": {}})
    check("shutdown result null", r.get("result") is None)
    c.send({"jsonrpc": "2.0", "method": "exit", "params": {}})
    try:
        proc.wait(timeout=15)
        check("exit 后服务器退出 rc=0", proc.returncode == 0,
              "rc=%s" % proc.returncode)
    except subprocess.TimeoutExpired:
        check("exit 后服务器退出 rc=0", False, "timeout")
        proc.kill()
    err = proc.stderr.read().decode("utf-8", "replace")
    if "运行时错误" in err or "panic" in err:
        check("服务器 stderr 无运行时错误", False, err[:300])
    else:
        check("服务器 stderr 无运行时错误", True)

    if FAIL:
        print("== S3 client: %d FAIL ==" % len(FAIL))
        for f in FAIL:
            print("  - " + f)
        return 1
    print("== S3 client: ALL PASS ==")
    return 0


if __name__ == "__main__":
    sys.exit(main())
