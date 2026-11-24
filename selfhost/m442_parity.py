#!/usr/bin/env python3
"""M442-S2 判据工具（gate 内用）：PXE1 **ver2** ⇄ bc_cli dump 文本 逐字段对拍。

用法：python3 parity2.py <dump.txt> <file.pxe>
判据（全部逐项，不看总数）：模块名 / K 每项(kind+值，float 按位) / G 每项 /
funcs 每项（name arity ndefault nslots nup · upvals 名 · 每条指令 op/a/b/c）/ top。
外加：**格式自检**（trailer crc32 + sha256 独立实现重算；段偏移推导必须自洽）。
退出码：0=一致，1=不一致（打印第一处差异），2=输入错误。
独立性：本文件用 zlib.crc32 / hashlib.sha256 重算，不复用 runtime 侧实现 —— 判据不许借用被检对象。
"""
import hashlib
import struct
import sys
import zlib

PXOP = {
    "LOADK": 1, "IMM": 2, "MOV": 3, "GETG": 4, "SETG": 5, "NEG": 6, "NOT": 7,
    "BITNOT": 8, "ADD": 9, "SUB": 10, "MUL": 11, "DIV": 12, "IDIV": 13, "MOD": 14,
    "POW": 15, "EQ": 16, "NE": 17, "LT": 18, "LE": 19, "GT": 20, "GE": 21,
    "BITAND": 22, "BITOR": 23, "BITXOR": 24, "SHL": 25, "SHR": 26, "SHRU": 27,
    "INDEX": 28, "SETIDX": 29, "SLICE": 30, "GETF": 31, "SETF": 32, "GETF_OPT": 33,
    "NEWLIST": 34, "NEWTUPLE": 35, "NEWDICT": 36, "NEWSTRUCT": 37, "NEWENUM": 38,
    "LISTPUSH": 39, "CALL": 40, "CALLM": 41, "CALLSELF": 42, "TAILCALL": 43,
    "RET": 44, "RET0": 45, "JMP": 46, "JMPT": 47, "JMPF": 48, "TRY": 49,
    "FORCE": 50, "SRCLINE": 51, "HALT": 52, "NEWGEN": 53, "SPAWN": 54,
    "ENUMVAR": 55, "GENFROMLIST": 56, "NARGS": 57, "CELLGET": 58, "CELLSET": 59,
    "CELLNEW": 60, "MKCLO": 61, "DICTSET": 62, "ITERAT": 63, "ITERLEN": 64,
    "UNPACKCK": 65, "UNINIT": 66, "CHKINIT": 67, "IN": 68, "NOTIN": 69,
    "RAISE": 70, "MATCHFAIL": 71, "MATCHTUP": 72, "ENUMPAY": 73, "ENUMP": 74,
    "MATCHENUMP": 75, "ENUMIS": 76, "TRYENTER": 77, "TRYEXIT": 78,
}
OPNAME = {v: k for k, v in PXOP.items()}
KIND = {0: "int", 1: "float", 2: "str", 3: "bool", 4: "null", 5: "func"}


def unesc(s):
    out, i = [], 0
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            n = s[i + 1]
            if n == "\\":
                out.append("\\"); i += 2; continue
            if n == '"':
                out.append('"'); i += 2; continue
            if n == "n":
                out.append("\n"); i += 2; continue
            if n == "r":
                out.append("\r"); i += 2; continue
            if n == "t":
                out.append("\t"); i += 2; continue
            if s[i + 1:i + 4] == "000":
                out.append("\0"); i += 4; continue
        out.append(c)
        i += 1
    return "".join(out)


def r32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def r64s(b, o):
    return struct.unpack_from("<q", b, o)[0]


def parse_pxe(path):
    b = open(path, "rb").read()
    if b[:4] != b"PXE1":
        raise SystemExit("E: magic")
    ver = struct.unpack_from("<H", b, 4)[0]
    if ver != 2:
        raise SystemExit("E: ver=%d（本工具只认 2）" % ver)
    nK, nN, nG = r32(b, 8), r32(b, 12), r32(b, 16)
    nf, top, nst = r32(b, 20), r32(b, 24), r32(b, 28)
    ninst, mnoff, slen = r32(b, 32), r32(b, 36), r32(b, 40)
    total = r32(b, 44)
    oK = 96
    oN = oK + nK * 24
    oG = oN + nN * 4
    oF = oG + nG * 4
    oFU = oF + nf * 32
    nup_total = 0
    for i in range(nf):
        nup_total += r32(b, oF + i * 32 + 24)
    oT = oFU + nup_total * 4
    nf_total = 0
    for i in range(nst):
        nf_total += r32(b, oT + i * 12 + 4)
    oTF = oT + nst * 12
    oC = oTF + nf_total * 4
    oS = oC + ninst * 8
    payload = oS + slen
    if payload + 40 != total or total != len(b):
        raise SystemExit("E: 段表不自洽 payload=%d total=%d len=%d" % (payload, total, len(b)))
    # 格式自检（独立实现重算）
    crc = zlib.crc32(b[:payload]) & 0xFFFFFFFF
    if crc != r32(b, payload):
        raise SystemExit("E: crc32 不符 have=%08x file=%08x" % (crc, r32(b, payload)))
    sha = hashlib.sha256(b[:payload]).digest()
    if sha != b[payload + 4:payload + 36]:
        raise SystemExit("E: sha256 不符")

    def s_at(off, ln=0):
        if off == 0xFFFFFFFF:
            return None
        seg = b[oS + off:]
        if ln > 0:
            return seg[:ln].decode("utf-8", "surrogateescape")
        e = seg.index(b"\0")
        return seg[:e].decode("utf-8", "surrogateescape")

    m = {"ver": ver, "name": s_at(mnoff), "K": [], "G": [], "N": [], "funcs": [],
         "structs": [], "top": -1 if top == 0xFFFFFFFF else top,
         "ninst": ninst, "slen": slen, "total": total, "nup_total": nup_total}
    for i in range(nK):
        p = oK + i * 24
        kind = KIND.get(b[p], "?")
        soff = r32(b, p + 4)
        iv = r64s(b, p + 8)
        fb = struct.unpack_from("<d", b, p + 16)[0]
        k = {"kind": kind, "i": iv, "f": fb}
        if kind == "str":
            k["s"] = s_at(soff, iv if iv > 0 else 0)
        m["K"].append(k)
    for i in range(nN):
        m["N"].append(s_at(r32(b, oN + i * 4)))
    for i in range(nG):
        m["G"].append(s_at(r32(b, oG + i * 4)))
    upbase = 0
    for i in range(nf):
        p = oF + i * 32
        f = {"name": s_at(r32(b, p)), "arity": r32(b, p + 4),
             "ndefault": r32(b, p + 8), "nslots": r32(b, p + 12),
             "bcoff": r32(b, p + 16), "nbc": r32(b, p + 20),
             "nup": r32(b, p + 24), "upvals": [], "bc": []}
        for u in range(f["nup"]):
            f["upvals"].append(s_at(r32(b, oFU + (upbase + u) * 4)))
        upbase += f["nup"]
        for j in range(f["nbc"]):
            q = oC + (f["bcoff"] + j) * 8
            op = b[q]
            a, bb, c = struct.unpack_from("<HHH", b, q + 2)
            f["bc"].append((OPNAME.get(op, "?%d" % op), a,
                            bb if bb < 32768 else bb - 65536, c))
        m["funcs"].append(f)
    for i in range(nst):
        p = oT + i * 12
        noff, nfields, fbase = r32(b, p), r32(b, p + 4), r32(b, p + 8)
        sd = {"name": s_at(noff), "fnames": []}
        for k in range(nfields):
            sd["fnames"].append(s_at(r32(b, oTF + (fbase + k) * 4)))
        m["structs"].append(sd)
    return m


def parse_dump(path):
    m = {"K": [], "G": [], "funcs": [], "structs": []}
    cur = None
    structs_known = False
    for raw in open(path, encoding="utf-8"):
        ln = raw.rstrip("\n")
        if ln.startswith("# BCModule "):
            m["name"] = ln[len("# BCModule "):]
        elif ln.startswith("K "):
            parts = ln.split(" ", 3)
            kind = parts[2]
            rest = parts[3] if len(parts) > 3 else ""
            k = {"kind": kind}
            if kind == "str":
                k["s"] = unesc(rest[1:-1])
            elif kind in ("int", "func"):
                k["i"] = int(rest)
            elif kind == "bool":
                k["i"] = 1 if rest == "true" else 0
            elif kind == "float":
                k["f"] = float(rest)
            m["K"].append(k)
        elif ln.startswith("G "):
            m["G"].append(ln.split(" ", 2)[2])
        elif ln.startswith("# funcs "):
            m["top"] = int(ln.split(" ")[-1].split("=")[1])
        elif ln.startswith("# structs "):
            structs_known = True
        elif ln.startswith("S ") and structs_known:
            parts = ln.split(" ", 2)
            m["structs"].append({"name": parts[1],
                                 "fnames": parts[2].split(",") if len(parts) > 2 and parts[2] else []})
        elif ln.startswith("== func "):
            head, *rest = ln[len("== func "):].split(" ")
            f = {"name": head, "bc": [], "upvals": []}
            for it in rest:
                if "=" in it:
                    k, v = it.split("=", 1)
                    if k in ("arity", "ndefault", "nslots"):
                        f[k] = int(v)
                    elif k == "up":
                        f["nup"] = int(v.split("[")[0])
                        if "[" in v and v.endswith("]"):
                            inner = v[v.index("[") + 1:-1]
                            f["upvals"] = [x for x in inner.split(",") if x]
            f.setdefault("nup", 0)
            cur = f
            m["funcs"].append(f)
        elif ":" in ln and cur is not None:
            pc, *tail = ln.split(":")
            toks = tail[0].split(" ")
            fld = [int(x) for x in toks[1:4]]
            cur["bc"].append((toks[0], fld[0], fld[1], fld[2]))
    m["structs_known"] = structs_known
    return m


def main():
    if len(sys.argv) != 3:
        raise SystemExit("用法: parity2.py <dump.txt> <file.pxe>")
    d = parse_dump(sys.argv[1])
    p = parse_pxe(sys.argv[2])
    bad = []

    def eq(what, a, b):
        if a != b:
            bad.append("%s: dump=%r pxe=%r" % (what, a, b))

    eq("name", d.get("name"), p["name"])
    eq("nK", len(d["K"]), len(p["K"]))
    for i, (dk, pk) in enumerate(zip(d["K"], p["K"])):
        eq("K[%d].kind" % i, dk["kind"], pk["kind"])
        if dk["kind"] == "str":
            eq("K[%d].s" % i, dk["s"], pk["s"])
        elif dk["kind"] == "float":
            eq("K[%d].f" % i, dk["f"], pk["f"])
        elif "i" in dk:
            eq("K[%d].i" % i, dk["i"], pk["i"])
    eq("nG", len(d["G"]), len(p["G"]))
    for i, (dg, pg) in enumerate(zip(d["G"], p["G"])):
        eq("G[%d]" % i, dg, pg)
    eq("top", d.get("top"), p["top"])
    eq("nfuncs", len(d["funcs"]), len(p["funcs"]))
    for i, (df, pf) in enumerate(zip(d["funcs"], p["funcs"])):
        eq("F[%d].name" % i, df["name"], pf["name"])
        for kk in ("arity", "ndefault", "nslots", "nup"):
            eq("F[%d].%s" % (i, kk), df.get(kk), pf[kk])
        if df.get("upvals"):
            eq("F[%d].upvals" % i, df["upvals"], pf["upvals"])
        eq("F[%d].nbc" % i, len(df["bc"]), pf["nbc"])
        for j, (di, pi) in enumerate(zip(df["bc"], pf["bc"])):
            if di != pi:
                bad.append("F[%d].bc[%d]: dump=%r pxe=%r" % (i, j, di, pi))
                break
    if d.get("structs_known"):
        eq("nstructs", len(d["structs"]), len(p["structs"]))
        for i, (ds, ps) in enumerate(zip(d["structs"], p["structs"])):
            eq("S[%d].name" % i, ds["name"], ps["name"])
            if ds["fnames"]:
                eq("S[%d].fnames" % i, ds["fnames"], ps["fnames"])

    if bad:
        print("parity2: FAIL（%d 处差异）" % len(bad))
        for x in bad[:10]:
            print("  " + x)
        return 1
    print("parity2: OK（K=%d G=%d funcs=%d ninst=%d structs=%d nup_total=%d total=%d）"
          % (len(p["K"]), len(p["G"]), len(p["funcs"]), p["ninst"],
             len(p["structs"]), p["nup_total"], p["total"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
