#!/usr/bin/env python3
# M191 静态判据：运行时出错站点的「码 / 域前缀」审计。
#   判据来源 = docs/ERROR_CODES.md（§1 判据、§2.3 个数分码、§3 域前缀闭集）。
# 用法：python3 scan_errcodes.py [--root <仓库根>]     退出码 0=绿 1=红
import collections, os, re, sys

ROOT = "/data/code/puxian"
for i, a in enumerate(sys.argv):
    if a == "--root" and i + 1 < len(sys.argv):
        ROOT = sys.argv[i + 1]

# ── §3 域前缀闭集（库·环境族）—— **新增域必须在这里登记**，否则判红 ──
DOMAIN = [
    r"io:", r"fs:", r"net:", r"json:", r"regex:",
    r"tcp_listen:", r"udp_open:", r"udp_serve:", r"udp_send", r"udp_recv",
    r"tls_connect:", r"tls_upgrade:", r"tls_server\(?[^)]*\)?:",
    r"http_serve:", r"http_serve_unix:", r"sse_serve:", r"px_serve:", r"vhost:",
    r"spawn:", r"cron:", r"cron[^:]*$", r"bus_subscribe:", r"bus_publish:", r"ctx_set:",
    r"定时器", r"事件总线数量超出上限", r"虚拟主机数量超出上限", r"SNI 证书数量超出上限",
    r"全局表溢出", r"句柄表已满", r"创建线程失败", r"并发线程数超出上限",
    r"os_exec", r"沙箱：", r"%s: SO_REUSEPORT",
    r"md5 ", r"sha1 ", r"sha256 ", r"pbkdf2_sha256 ", r"aes ",
    r"zip ", r"内存不足",
]
DOMAIN_RE = re.compile("^(" + "|".join(DOMAIN) + ")")

# ── §2.3 个数类：方法的判据（消息形态）──
ARITY = re.compile(r"(需要 [0-9-]+ 个参数|不接受参数|需要 [0-9-]+ 个函数参数|需要 \([^)]*\) 参数|需要 \([^)]*\) 两个参数)")
METHODISH = re.compile(r"^(方法 |list\.index|pop |is_ok|is_err|unwrap|unwrap_err|send |mutex\.with|rwlock\.with)")

# ── 未收口基线（**棘轮**）：这些文件里"尚无 R 码的语言层站点"是历史欠账，本轮不一次性收口
#    （见 docs/ERROR_CODES.md §5），但**不许变多** —— 多了即判红。
#    核心文件 `runtime/runtime.c` 与 `runtime/runtime_onnx.c` **已全额收口（基线 0）**；
#    M192 起 **加密/压缩/归档族五件（aes / zip / rsa / zlib / ed25519）也已全额收口** ——
#    其条目已从下表**删除**：删除即等价于基线 0，再往这些文件加无码站点会立即判门红。
UNCODED_BASELINE = {
    "runtime/coro.c": 1,
    "runtime/runtime_ffi.c": 3,
    "runtime/runtime_h3.c": 18,
    "runtime/runtime_h3_qpack.c": 2,
    "runtime/runtime_h3_qpack_dyn.c": 2,
    "runtime/runtime_image.c": 7,
    "runtime/runtime_quic.c": 28,
    "runtime/runtime_route.c": 9,
    "runtime/runtime_sqlite.c": 9,
    "runtime/runtime_ws.c": 16,
    "runtime/runtime_xml.c": 23,
    "runtime/vm.c": 25,
}

fails = []
notes = []


def check_sites(path, kind):
    """kind = 'c'（px_error("...")） | 'px'（i_r10xx("...")）"""
    src = open(os.path.join(ROOT, path), encoding="utf-8").read().split("\n")
    n = code = dom = 0
    uncoded = []
    for ln, line in enumerate(src, 1):
        it = (re.finditer(r'px_error\(\s*"((?:[^"\\]|\\.)*)"', line) if kind == "c"
              else re.finditer(r'i_r(10[0-9][0-9])\(\s*"((?:[^"\\]|\\.)*)"', line))
        for m in it:
            n += 1
            if kind == "c":
                msg = m.group(1)
                rcode = msg[:5] if re.match(r"^R\d{4}", msg) else ""
            else:
                rcode = "R" + m.group(1)
                msg = m.group(2)
            if rcode:
                code += 1
            elif DOMAIN_RE.match(msg):
                dom += 1
            else:
                uncoded.append((ln, msg))
                continue
            body = re.sub(r"^R\d{4}: ", "", msg)
            if ARITY.search(body):
                want = "R1005" if METHODISH.match(body) else "R1002"
                if rcode != want:
                    fails.append("%s:%d 个数类码不符（%s，应为 %s）：%s" % (path, ln, rcode or "(无码)", want, body[:60]))
    base = UNCODED_BASELINE.get(path, 0)
    tag = "✅" if not uncoded and base == 0 else ("ℹ️" if len(uncoded) <= base else "❌")
    if len(uncoded) > base:
        fails.append("%s 未收口的无码站点 **变多**：%d > 基线 %d —— 新增站点必须补码或登记域名"
                     % (path, len(uncoded), base))
    elif uncoded:
        notes.append("%-30s 未收口无码站点 %d（基线 %d）" % (path, len(uncoded), base))
    print("   %-30s 站点 %-4d 带码 %-4d 域前缀 %-4d 无码 %-4d %s" % (path, n, code, dom, len(uncoded), tag))


def check_mixed(path):
    """§2.3：同一条守卫里**同时**断言「数值个数」与「类型」、且消息只给一个码 ⇒ 必须拆开。
    ⚠️ 边界：`if (nargs != 1 || args[0].type != PX_STR) px_error("R1002: read_file 需要一个路径参数")`
    这类**消息本身泛化**（同时覆盖个数与类型）不算违例 —— 只判「消息里写死 `需要 N 个参数`
    （数值个数）却与类型混在同一守卫」。
    """
    src = open(os.path.join(ROOT, path), encoding="utf-8").read().split("\n")
    bad = 0
    for ln, line in enumerate(src, 1):
        if "px_error(" not in line:
            continue
        if not (re.search(r"nargs\s*[!<>=]", line) and re.search(r"\.type\s*[!=]=|type\s*!=", line)):
            continue
        if not re.search(r"需要\s*[0-9-]+\s*个参数", line):
            continue
        fails.append("%s:%d 个数与类型混写在同一守卫（须拆成两条、各自给码）：%s" % (path, ln, line.strip()[:80]))
        bad += 1
    if bad:
        print("   %-30s 混写守卫 %d" % (path, bad))


CFILES = sorted(f for f in os.listdir(os.path.join(ROOT, "runtime")) if f.endswith(".c"))
print("── [S1/S2] native 站点：带码 or 域前缀闭集（+ 个数分码）──")
for f in CFILES:
    check_sites("runtime/" + f, "c")
print("── [S1/S2] 解释轨站点：同上 ──")
for f in ("selfhost/icall.px", "selfhost/ibuiltin.px", "selfhost/ival.px", "selfhost/iexpr.px", "selfhost/istmt.px"):
    check_sites(f, "px")
print("── [S3] 个数/类型混写守卫 ──")
for f in CFILES:
    check_mixed("runtime/" + f)

print("")
if notes:
    print("ℹ️ 未收口清单（**棘轮**：只许减少、不许增加；见 docs/ERROR_CODES.md §5）")
    for n in notes:
        print("   ", n)
    print("")
if fails:
    print("❌ 静态判据失败 %d 项 · 涉及 %d 个文件：" % (len(fails), len(set(f.split(":")[0] for f in fails))))
    for k, v in collections.Counter(f.split(":")[0] for f in fails).most_common():
        print("   %-30s %d" % (k, v))
    print("── 明细（前 40 条）──")
    for f in fails[:40]:
        print("   -", f)
    open("/tmp/m191_scan_fails.txt", "w", encoding="utf-8").write("\n".join(fails))
    sys.exit(1)
print("✅ 静态判据通过（带码/域前缀 · 个数分码 · 混写拆分 · 未收口棘轮 四查）")
