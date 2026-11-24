#!/usr/bin/env python3
"""M442-S2 负控：对 .pxe 做定点畸变（必要时重算 trailer），断言 loader 的拒收码。

用法：python3 negctl.py <pxvm> <file.pxe>
每条负控都打印 PASS/FAIL；任一 FAIL ⇒ 退出码 1。
判据纪律：**每条负控都必须让判据变红**；只在「改完能加载」时才重算校验和
（否则只是被 V6 挡住，V3/V4/V5 等于没测 —— M442-S1 踩过这条）。
"""
import hashlib
import struct
import subprocess
import sys
import zlib

HDR, TRAILER = 96, 40


def r32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def w32(b, o, v):
    struct.pack_into("<I", b, o, v)


def refix(b):
    """重算 crate trailer（crc32 + sha256(payload)），模拟「篡改者把校验和也改了」。"""
    payload = len(b) - TRAILER
    w32(b, payload, zlib.crc32(b[:payload]) & 0xFFFFFFFF)
    b[payload + 4:payload + 36] = hashlib.sha256(b[:payload]).digest()
    return b


def layout(b):
    nK, nN, nG = r32(b, 8), r32(b, 12), r32(b, 16)
    nf, nst, ninst = r32(b, 20), r32(b, 28), r32(b, 32)
    oK = HDR
    oN = oK + nK * 24
    oG = oN + nN * 4
    oF = oG + nG * 4
    oFU = oF + nf * 32
    nup = sum(r32(b, oF + i * 32 + 24) for i in range(nf))
    oT = oFU + nup * 4
    nft = sum(r32(b, oT + i * 12 + 4) for i in range(nst))
    oTF = oT + nst * 12
    oC = oTF + nft * 4
    oS = oC + ninst * 8
    return dict(nK=nK, nN=nN, nG=nG, nf=nf, nst=nst, ninst=ninst,
                oK=oK, oN=oN, oG=oG, oF=oF, oFU=oFU, oT=oT, oTF=oTF, oC=oC, oS=oS)


def run(pxvm, path):
    r = subprocess.run([pxvm, path], capture_output=True, text=True)
    return r.returncode


def main():
    pxvm, path = sys.argv[1], sys.argv[2]
    src = open(path, "rb").read()
    L = layout(src)
    fails = []

    def case(name, data, want_rc, refix_after=False):
        b = bytearray(data)
        if refix_after:
            b = refix(b)
        tmp = "/tmp/negctl_case.pxe"
        open(tmp, "wb").write(b)
        rc = run(pxvm, tmp)
        ok = (rc == want_rc) if isinstance(want_rc, int) else (rc in want_rc)
        print("%-52s want=%-7s got=%-3d %s" % (name, want_rc, rc, "PASS" if ok else "FAIL"))
        if not ok:
            fails.append(name)

    # N0 正控：原文件必须能加载（rc=0 或有程序退出码）
    case("N0 正控（未改动）", src, 0)

    # N1 破一字节（不动校验和）⇒ V6
    b = bytearray(src)
    b[L["oS"] + 1] ^= 0xFF
    case("N1 池内翻一字节（不修校验和）", b, 6)

    # N2 版本号 3（修校验和）⇒ V2
    b = bytearray(src)
    struct.pack_into("<H", b, 4, 3)
    case("N2 ver=3（修校验和）", b, 2, refix_after=True)

    # N3 total 字段 +1（修校验和）⇒ V3
    b = bytearray(src)
    w32(b, 44, r32(b, 44) + 1)
    case("N3 total+1（修校验和）", b, 3, refix_after=True)

    # N4 跳转越界（修校验和）⇒ V4 —— 安全边界：pc 不许跳出函数
    b = bytearray(src)
    hit = None
    for i in range(L["nf"]):
        p = L["oF"] + i * 32
        bcoff, nbc = r32(b, p + 16), r32(b, p + 20)
        for j in range(nbc):
            q = L["oC"] + (bcoff + j) * 8
            if b[q] in (46, 47, 48, 77):     # JMP/JMPT/JMPF/TRYENTER
                struct.pack_into("<H", b, q + 4, 0x7FFF)
                hit = (i, j, b[q])
                break
        if hit:
            break
    if hit is None:
        print("%-52s %s" % ("N4 跳转越界", "SKIP（本样例无跳转指令 —— 判据会空跑！）"))
        fails.append("N4 前提自证失败")
    else:
        print("   └ 前提自证：样例确有跳转指令 F[%d].bc[%d] op=%d" % hit)
        case("N4 跳转目标 0x7FFF 越界（修校验和）", b, 4, refix_after=True)

    # N5 LOADK 的 k 越界（修校验和）⇒ V4
    b = bytearray(src)
    hit = None
    for i in range(L["nf"]):
        p = L["oF"] + i * 32
        bcoff, nbc = r32(b, p + 16), r32(b, p + 20)
        for j in range(nbc):
            q = L["oC"] + (bcoff + j) * 8
            if b[q] == 1:                    # LOADK
                struct.pack_into("<H", b, q + 4, L["nK"] + 5)
                hit = (i, j)
                break
        if hit:
            break
    if hit is None:
        print("%-52s %s" % ("N5 LOADK 越界", "SKIP（无 LOADK）"))
        fails.append("N5 前提自证失败")
    else:
        case("N5 LOADK k=nK+5 越界（修校验和）", b, 4, refix_after=True)

    # N6 modname_off 越界（修校验和）⇒ V5
    b = bytearray(src)
    w32(b, 36, 0x7FFFFFF0)
    case("N6 modname_off 越界（修校验和）", b, 5, refix_after=True)

    # N7 截断 1 字节（不动校验和）⇒ V3 或 V6
    case("N7 截尾 1 字节", src[:-1], (3, 6))

    # N8 magic 破坏 ⇒ 3（ELAYOUT，magic 即 V1）
    b = bytearray(src)
    b[0] = ord("X")
    case("N8 magic 破坏", b, 3)

    # N9 复原：原文件仍可加载（负控没污染文件系统状态）
    case("N9 复原（原文件）", src, 0)

    print()
    if fails:
        print("负控 FAIL: " + ", ".join(fails))
        return 1
    print("负控全绿")
    return 0


if __name__ == "__main__":
    sys.exit(main())
