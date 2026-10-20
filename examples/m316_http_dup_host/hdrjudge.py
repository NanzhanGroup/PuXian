#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M316 门 · 判据（唯一事实源）：从「原始请求头的 hex」里判定缺陷 550 是否在场。

为什么判据只吃**线上字节**：
  缺陷 550 的形态是「同一个头名出现两次」。服务端解析后（PuXian 的 http 服务端会把
  同名头聚合成 list）就看不到「几条」了 —— 而真实受害方（nginx / Go 反代 / 云 WAF）
  看到的恰恰是**原始字节**。所以观察面必须是 hex，不是解析结果。

通用判据（对所有 case 都成立）：
  G1 每个出现的头名**恰好 1 条**（缺陷 550 的定义：Host/UA 重复即违反）
  G2 请求行形态正确（GET/POST + 路径 + HTTP/1.1）
按 case 判据：Host / User-Agent / Content-Type / Content-Length 的**值与条数**。

用法：hdrjudge.py <raw_hex> <tcp|unix>:<case> <hostport>
退出码：0 = 全通过；1 = 有判据红。
"""
import sys

EXPECT = {
    # tcp 面：内部 Host = 127.0.0.1:<port>
    "tcp:none":         ("{hostport}", "PuXian/0.1", {}),
    "tcp:host_and_ua":  ("other.example:8080", "MyClient/9.9", {}),
    "tcp:host_lower":   ("lower.example", "PuXian/0.1", {}),
    "tcp:host_empty":   ("{hostport}", "PuXian/0.1", {}),
    "tcp:ua_only":      ("{hostport}", "OnlyUA/1.0", {}),
    "tcp:post_ct":      ("{hostport}", "PuXian/0.1",
                         {"content-type": ["application/json"], "content-length": ["3"]}),
    # unix 面：内部 Host = localhost
    "unix:none":        ("localhost", "PuXian/0.1", {}),
    "unix:host_and_ua": ("u.example", "UnixCli/1.0", {}),
    "unix:host_empty":  ("localhost", "PuXian/0.1", {}),
    "unix:ua_only":     ("localhost", "UnixUA/2.0", {}),
}


def parse(raw_hex):
    data = bytes.fromhex(raw_hex)
    text = data.decode("latin-1")
    parts = text.split("\r\n\r\n", 1)
    head = parts[0]
    lines = head.split("\r\n")
    out = {}
    for ln in lines[1:]:
        if ":" not in ln:
            continue
        k, v = ln.split(":", 1)
        out.setdefault(k.strip().lower(), []).append(v.strip())
    return lines[0], out


def main():
    if len(sys.argv) < 4:
        print("HDRJUDGE-USAGE <raw_hex> <tcp|unix>:<case> <hostport>")
        return 2
    raw_hex, tag, hostport = sys.argv[1], sys.argv[2], sys.argv[3]
    if tag not in EXPECT:
        print("HDRJUDGE-BADCASE " + tag)
        return 2
    exp_host, exp_ua, extra = EXPECT[tag]
    exp_host = exp_host.replace("{hostport}", hostport)

    reqline, h = parse(raw_hex)
    fail = 0

    def ck(name, cond, got):
        nonlocal fail
        if cond:
            print("    PASS " + name)
        else:
            print("    FAIL " + name + " （实际 " + str(got) + "）")
            fail += 1

    # G2 请求行
    ck("[G2] 请求行 = <METHOD> <path> HTTP/1.1",
       len(reqline.split(" ")) == 3 and reqline.endswith("HTTP/1.1"), reqline)

    # G1 每个头名恰好 1 条（缺陷 550 的定义）
    dup = {k: len(v) for k, v in h.items() if len(v) != 1}
    ck("[G1] 所有头名恰好 1 条", dup == {}, dup)

    # Host
    hv = h.get("host", [])
    ck("[H] Host 条数 = 1", len(hv) == 1, len(hv))
    ck("[H] Host 值 = " + exp_host, hv == [exp_host], hv)

    # User-Agent
    uv = h.get("user-agent", [])
    ck("[U] User-Agent 条数 = 1", len(uv) == 1, len(uv))
    ck("[U] User-Agent 值 = " + exp_ua, uv == [exp_ua], uv)

    # 按 case 的额外头
    for k in sorted(extra.keys()):
        got = h.get(k, [])
        ck("[X] " + k + " = " + str(extra[k]), got == extra[k], got)

    print("HDRJUDGE-OK" if fail == 0 else "HDRJUDGE-FAIL %d" % fail)
    return 0 if fail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
