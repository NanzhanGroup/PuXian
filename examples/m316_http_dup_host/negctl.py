#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M316 门 · 负控打桩器（正向 ⇄ 撤回，逐字节可逆）

纪律（M285 A4）：每次替换断言**恰 1 处**命中；命中数不符即拒绝写入。
纪律（M294）：负控必须**忠实** —— 撤回后的源码要与 M316 之前**逐字节**一致
  （由 --verify-faithful 对着备份比对）。
"""
import sys
import os

# ⚠️ M317 修（缺陷 300 同族 · 门自身绝对路径）：
#   此处原先写死 `/data/code/puxian/runtime/runtime.c` ⇒ 在**分片 worktree** 里跑门时，
#   打桩改的是**主仓库**源码，而不是被测的那棵树。实测后果（M317 分片档）：
#     ① devbuild 用的是 worktree（源码没变）⇒ 命中缓存 ⇒ `[6.2]` 报「负控档与修复档相同」；
#     ② 打桩期间**主仓库源码被改成撤回态** ⇒ 同窗口其他分片的门读到被污染的源码。
#   修法 = 由本文件位置上溯两级推导仓根（门在 `<root>/examples/m316_.../`）⇒ 天然指向被测树；
#   仍允许 `M316_ROOT` 显式覆盖（门里传 $ROOT）。
_HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.environ.get("M316_ROOT") or os.path.abspath(os.path.join(_HERE, "..", ".."))
P = os.path.join(ROOT, "runtime", "runtime.c")

# ① bi_http_request 的「按需发」块
FIX_HDR = r'''    int rlen;
    {
        char hv[512];
        if (px_hdr_val(extra_headers, "Host", hv, sizeof(hv)) == 1)
            rlen = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\n", method, req_target);
        else
            rlen = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\nHost: %s:%d\r\n", method, req_target, host, port);
    }
    if (px_hdr_val(extra_headers, "User-Agent", NULL, 0) != 1)
        rlen += snprintf(req + rlen, sizeof(req) - rlen, "User-Agent: PuXian/0.1\r\n");
    rlen += snprintf(req + rlen, sizeof(req) - rlen, "Connection: keep-alive\r\n");'''

OLD_HDR = r'''    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: PuXian/0.1\r\nConnection: keep-alive\r\n",
        method, req_target, host, port);'''

# ② bi_http_unix 的「按需发」块
FIX_UNIX = r'''    int rlen;
    {
        char hv[512];
        if (px_hdr_val(extra_headers, "Host", hv, sizeof(hv)) == 1)
            rlen = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\n", method, url_path);
        else
            rlen = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\nHost: localhost\r\n", method, url_path);
    }
    if (px_hdr_val(extra_headers, "User-Agent", NULL, 0) != 1)
        rlen += snprintf(req + rlen, sizeof(req) - rlen, "User-Agent: PuXian/0.1\r\n");
    rlen += snprintf(req + rlen, sizeof(req) - rlen, "Connection: close\r\n");'''

OLD_UNIX = r'''    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.1\r\nHost: localhost\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n",
        method, url_path);'''

# ③ sse 的 Host 判据
FIX_SSE = r'''    int rl;
    {
        char hval[512];
        if (px_hdr_val(g_sse_clients[slot].req_headers, "Host", hval, sizeof(hval)) == 1)
            rl = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\n", method, path);
        else
            rl = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\nHost: %s\r\n", method, path, hosthdr);
    }'''

OLD_SSE = r'''    int rl = snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\nHost: %s\r\n", method, path, hosthdr);'''

PAIRS = [("http_request", FIX_HDR, OLD_HDR),
         ("http_unix", FIX_UNIX, OLD_UNIX),
         ("sse", FIX_SSE, OLD_SSE)]


def swap(pairs, tag):
    src = open(P, encoding="utf-8").read()
    for name, a, b in pairs:
        n = src.count(a)
        if n != 1:
            sys.exit("[FATAL] %s/%s：命中 %d 次（期望 1）—— 拒绝写入" % (tag, name, n))
        src = src.replace(a, b, 1)
    open(P, "w", encoding="utf-8").write(src)
    print("NEGCTL-OK " + tag)


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else ""
    if mode == "--revert":
        swap([(n, a, b) for n, a, b in PAIRS], "revert")
    elif mode == "--restore":
        swap([(n, b, a) for n, a, b in PAIRS], "restore")
    elif mode == "--check-reverted":
        # 忠实性判据（不依赖 git、不依赖开发机残留 —— M199 纪律）：
        #   ① 三处「原始形态」必须各在场 1 次
        #   ② 三处「判据形态」必须各 0 次（px_hdr_val 的定义/其它调用不算 —— 它们不在这些块里）
        src = open(P, encoding="utf-8").read()
        bad = []
        for name, fix, old in PAIRS:
            n_old = src.count(old)
            n_fix = src.count(fix)
            if n_old != 1:
                bad.append("%s 原始形态 %d 处（期望 1）" % (name, n_old))
            if n_fix != 0:
                bad.append("%s 判据形态仍存 %d 处（期望 0）" % (name, n_fix))
        if bad:
            print("NEGCTL-UNFAITHFUL " + " | ".join(bad))
            sys.exit(1)
        print("NEGCTL-FAITHFUL-STRUCT")
    elif mode == "--verify-faithful":
        # 与备份比对：撤回后必须与 M316 之前逐字节一致
        import hashlib
        cur = hashlib.sha256(open(P, "rb").read()).hexdigest()
        ref = hashlib.sha256(open(sys.argv[2], "rb").read()).hexdigest()
        print("NEGCTL-FAITHFUL" if cur == ref else "NEGCTL-UNFAITHFUL")
        sys.exit(0 if cur == ref else 1)
    else:
        print("用法：negctl.py --revert|--restore|--verify-faithful <备份路径>")
        sys.exit(2)
