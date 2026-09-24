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
    # M202：BASE32/HMAC-SHA1 族（与上一条同族 —— 后端算法失败的域前缀）
    r"hmac_sha1 ", r"hmac_sha1_bytes ",
    r"zip ", r"内存不足",
    # ── M193 新增（棘轮欠账第二批：网络族 / VM / 数据族）──
    r"ws_serve:",              # §3 E2 网络/传输：ws_serve 的 socket 创建/绑定/listen 失败
    r"XML 解析错误",            # §3 E4 数据/解析：与 json: 同族（XML 解析失败的 13 个站点）
    r"路由数量超出上限", "中间件数量超出上限",   # §3 E5 资源/上限
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
    # ── M193（第 71 轮）：棘轮欠账**第二批**全部收口 ⇒ 表清空 ──
    #    收口范围（143 站点 / 12 文件）：
    #      quic 28 · h3 18 · ws 16 · route 9 · sqlite 9 · xml 23 · image 7 · ffi 3 ·
    #      vm 25 · coro 1 · h3_qpack 2 · h3_qpack_dyn 2
    #    分类（§1 判据）：
    #      ① 语言层 ⇒ 补码（R1002 实参类型/形状 102 · R1005 缺形参 2 · R1004 unwrap 失败 2）
    #      ② VM 内部一致性（字节码/元数据越界，不该被用户触发）⇒ **R9001**（18 处，§2.1）
    #      ③ 库·环境族 ⇒ 只登记域前缀不改文本（18 处）：
    #         `ws_serve:`（3）· `XML 解析错误`（13）· `路由/中间件数量超出上限`（2）
    #      ④ 转发点豁免（1 处，见 FORWARD_EXEMPT）
    #    ⇒ **表为空即等价于「全仓无未收口站点」**：此后任何新增无码站点立即判门红。
}

# ── 转发点豁免（M193）：**每条必须给理由**，形态必须是 `px_error("%s"` ──
#    判据：① 表内每条都要在源码里找到且形态匹配（防"代码变了表没变"）；
#          ② 豁免总数**不得多于**登记数（棘轮）；③ 理由字段非空。
FORWARD_EXEMPT = {
    "runtime/coro.c": {
        804: "透传 px_native_call_capture 的 errbuf —— 上游 native 的错误消息已带 R 码或域前缀",
    },
}

fails = []
notes = []


def check_sites(path, kind):
    """kind = 'c'（px_error("...")） | 'px'（i_r10xx("...")）"""
    src = open(os.path.join(ROOT, path), encoding="utf-8").read().split("\n")
    n = code = dom = 0
    uncoded = []
    for ln, line in enumerate(src, 1):
        # M193：**跳过纯注释行** —— 注释里提到 `px_error("…")` 会被当成站点（假阳性：
        #   本轮 220 修复的说明注释就触发了）。判据 = 去掉前导空白后以 `//` 或 `*` 开头。
        st = line.lstrip()
        if st.startswith("//") or st.startswith("*") or st.startswith("/*"):
            continue
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
                if ln in FORWARD_EXEMPT.get(path, {}):
                    continue          # 转发点豁免（表内已给理由；另有专门判据查表⇔源码一致）
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


# ── 判据 ⑤：转发点豁免表 ⇔ 源码一致（防"代码被改动而表没跟着改"）──
EXEMPT_TOTAL = 0
for path, table in FORWARD_EXEMPT.items():
    src = open(os.path.join(ROOT, path), encoding="utf-8").read().split("\n")
    for ln, why in table.items():
        if not why.strip():
            fails.append("%s:%d 豁免条目缺理由" % (path, ln))
        if ln > len(src) or not re.search(r'px_error\(\s*"%s"', src[ln - 1]):
            fails.append("%s:%d 豁免条目在源码里找不到（或形态不是 px_error\"%s\"）" % (path, ln))
        EXEMPT_TOTAL += 1
    print("   %-30s 转发豁免 %d 处" % (path, len(table)))
print("── [S4] 转发豁免合计 %d（棘轮：不得多于登记数 1）──" % EXEMPT_TOTAL)
if EXEMPT_TOTAL > 1:
    fails.append("转发豁免条目变多：%d > 登记数 1" % EXEMPT_TOTAL)


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

# ══════════════════════════════════════════════════════════════════════
# 判据 ⑥（M194 · 第 72 轮 · 缺陷 222）：native 参数**类型守卫完备性**
#   口径见 docs/ERROR_CODES.md §6。对每个 `px_error("R1002: <fn> 需要 (<a>, <b>…)")`
#   站点：函数体里每个**被使用**的形参位置都必须有 `args[i].type` 检查。
#   用途分类（**危害递增**）：
#     CONT —— 当容器用（`.as.obj->as.{list,dict}` / `px_list_*` / `px_dict_*` / `px_bytes_*`
#             / `px_index` / `px_len`）：未检查 ⇒ 读 union 的其它字段 = **UB / 段错误**
#     INT  —— 当整数用（`int_val()` / `.as.i`）：未检查 ⇒ **float 静默截断**（M189 同族）、
#             `.as.i` 更是把 float 的**位模式**当整数读
#     NUM  —— 当数值用（`num_val()` / `math_num()`）
#     STR  —— 当字符串用（`val_cstr()` / `.as.obj->as.str.data`）：未检查 ⇒ **静默串化**
#             （`s3_get(1,2,3,4,5)` 变成对 endpoint "1" 的请求）
#   ⚠️ **豁免**（不判红）：只被 `px_type_name(args[i])` 使用的实参（仅用于错误消息自身）；
#      以及**可选实参**（形参写成 `[x]` 或 `x?`）—— 其"存在即校验"由 §6 的可选实参口径判，
#      这里只判**必填**形参。`write_file(path, content)` 的 content 走 `px_to_string`（任意
#      类型都是设计语义），因其在函数体里有 `args[1].type == PX_STR` 的**正向**分支 ⇒ 计为已检查。
ARG_GUARD_HINT = "（§6：当容器/整数/字符串使用的实参必须显式检查类型）"
_GUARD = re.compile(r'px_error\(\s*"(R\d{4})?:?\s*([A-Za-z_][A-Za-z0-9_]*)\s*需要\s*\(([^)]*)\)')
_USE = {
    "CONT": [r"\.as\.obj->as\.list", r"\.as\.obj->as\.dict", r"\.as\.obj->as\.str\.len",
             r"px_list_len\(", r"px_list_get\(", r"px_list_push\(", r"px_list_set\(",
             r"px_dict_len\(", r"px_dict_get\(", r"px_dict_set\(", r"px_dict_has\(",
             r"px_bytes_len\(", r"px_bytes_data\(", r"px_bytes_get\(",
             r"px_index\(", r"px_len\(", r"px_iter_"],
    "INT": [r"int_val\(", r"\.as\.i\b", r"px_req_int"],
    "NUM": [r"num_val\(", r"math_num\("],
    "STR": [r"val_cstr\(", r"px_val_cstr\(", r"\.as\.obj->as\.str\.data"],
}


def _func_body(lines, line_no):
    """从 line_no 向上找函数头、向下花括号配平 ⇒ 函数体行列表"""
    i = line_no - 1
    while i > 0:
        t = lines[i].rstrip()
        if t.endswith("{") and ("(" in t or t == "{"):
            break
        i -= 1
    depth = 0
    seen = False
    for j in range(i, len(lines)):
        depth += lines[j].count("{") - lines[j].count("}")
        if "{" in lines[j]:
            seen = True
        if seen and depth <= 0:
            return lines[i:j + 1]
    return lines[i:]


def _params(sig):
    out = []
    for raw in sig.split(","):
        p = raw.strip()
        if not p:
            continue
        opt = p.startswith("[")
        p = p.replace("[", "").replace("]", "")
        if p.endswith("?"):
            opt = True
            p = p[:-1]
        p = p.strip()
        for alt in p.split("|"):
            alt = alt.strip()
            if alt:
                out.append((alt, opt))
    return out


def check_arg_guards(path):
    lines = open(os.path.join(ROOT, path), encoding="utf-8").read().split("\n")
    nguard = nbad = 0
    for ln, line in enumerate(lines, 1):
        st = line.lstrip()
        if st.startswith("//") or st.startswith("*") or st.startswith("/*"):
            continue
        m = _GUARD.search(line)
        if not m:
            continue
        ps = _params(m.group(3))
        if len(ps) < 2:          # 单形参站点不适用本判据（首参本来就会被检查）
            continue
        nguard += 1
        body = _func_body(lines, ln)
        blob = "\n".join(body)
        ck = set(int(x.group(1)) for x in re.finditer(r"args\[(\d+)\]\.type\s*[!=]=", blob))
        ck |= set(int(x.group(2)) for x in re.finditer(r"px_val_is_\w+\(\s*args\[(\d+)\]", blob))
        for i, (nm, opt) in enumerate(ps):
            if opt or i in ck:
                continue
            u = set()
            for kind, pats in _USE.items():
                for p in pats:
                    if re.search(r"args\[%d\]" % i + p, blob) or \
                       re.search(p[:-2] + r"\(\s*args\[%d\]" % i, blob):
                        u.add(kind)
                        break
            if not u:
                continue          # 未被使用（或仅 px_type_name）⇒ 无需检查
            nbad += 1
            fails.append("%s:%d %s 的第 %d 个形参「%s」被当 %s 使用却**没有类型检查** %s"
                         % (path, ln, m.group(2), i + 1, nm, "/".join(sorted(u)), ARG_GUARD_HINT))
    return nguard, nbad


# ══════════════════════════════════════════════════════════════════════
# 判据 ⑦（M195 · 第 73 轮 · 缺陷 224/225）：**全部** native 函数的参数守卫
#   [S5] 只覆盖「消息形如 `需要 (...)` 且形参 ≥2」的站点；本判据覆盖**所有**
#   `LXValue fn(LXValue* args, int nargs, void* ctx)`（含单形参、含消息不列形参的形态）。
#   只统计**宽松接口**（不查就静默错值 / UB）：
#     INT  int_val(          —— float 静默截断
#     NUM  num_val(          —— 读 union 的 as.f
#     STR  val_cstr( / px_val_cstr(        —— int/容器静默串化
#     OBJ  .as.obj->as.* / .as.i           —— 读 union 其它字段 = UB
#   校验型接口（`px_req_*` / `px_arg_*` / `px_val_is_*` / `math_num` / `px_len` / `px_index`）
#   算「已检查」——它们本身就会响亮报错。
#   ⚠️ **豁免表**：每条必须给理由，并由三条判据强制：
#     ① 表内每条的函数必须在源码里**找得到**（防「代码改名了表没改」）；
#     ② 豁免**总数不得多于**登记数（棘轮）；③ 理由字段非空。
ARG_GUARD2_EXEMPT = {
    # ── A. **文本语义**：参数本身就是"一段文本"，任意值 ⇒ 其 str() 形态是语言约定 ──
    #    （M129 已定：`val_cstr(null)` = "null" 而非 "0.0"，与 `str(null)` 一致）
    ("runtime/runtime.c", "bi_md5"): "文本语义：哈希的输入文本（任意值可串化是语言约定，§6.5）",
    ("runtime/runtime.c", "bi_sha256"): "文本语义：哈希的输入文本",
    ("runtime/runtime.c", "bi_sha1"): "文本语义：哈希的输入文本",
    ("runtime/runtime.c", "bi_xxhash"): "文本语义：哈希的输入文本",
    ("runtime/runtime.c", "bi_base64_encode"): "文本语义：编码的输入文本",
    ("runtime/runtime.c", "bi_base64_decode"): "文本语义：解码的输入文本",
    ("runtime/runtime.c", "bi_base64_to_bytes"): "文本语义：解码的输入文本",
    # M202（第 81 轮）：base32 与 base64 族**同款"文本语义"**（编码用 bdata 故不入表，
    #   解码两件用 val_cstr ⇒ 会被 [S6] 扫到）—— 口径见 ERROR_CODES §6.8/§6.10。
    ("runtime/runtime.c", "bi_base32_decode"): "文本语义：解码的输入文本（M202 · 非法 → null，非抛错）",
    ("runtime/runtime.c", "bi_base32_to_bytes"): "文本语义：解码的输入文本（M202 · 同 base64_to_bytes）",
    ("runtime/runtime.c", "bi_hex_to_int"): "文本语义：解析的输入文本",
    ("runtime/runtime.c", "bi_hex_to_bytes"): "文本语义：解析的输入文本",
    ("runtime/runtime.c", "bi_ord"): "文本语义：取首字符码点",
    ("runtime/runtime.c", "bi_regex_valid"): "文本语义：正则表达式文本",
    ("runtime/runtime.c", "bi_regex_match"): "文本语义：正则 pattern/text 均为文本",
    ("runtime/runtime.c", "bi_regex_search"): "文本语义：正则 pattern/text 均为文本",
    ("runtime/runtime.c", "bi_regex_find"): "文本语义：正则 pattern/text 均为文本",
    ("runtime/runtime.c", "bi_regex_find_all"): "文本语义：正则 pattern/text 均为文本",
    ("runtime/runtime.c", "bi_regex_replace"): "文本语义：正则 pattern/text/repl 均为文本",
    ("runtime/runtime.c", "bi_regex_split"): "文本语义：正则 pattern/text 均为文本",
    ("runtime/runtime.c", "bi_dns_lookup"): "文本语义：域名文本",
    ("runtime/runtime.c", "bi_dns_txt"): "文本语义：域名文本",
    # ── B. **非用户入口**：由 runtime 内部以确定的 int 调用，用户拿不到它们的名字 ──
    ("runtime/runtime.c", "http_conn_worker"): "内部 worker（非用户面 builtin；调用方是 runtime 自身）",
    ("runtime/runtime.c", "sse_conn_worker"): "内部 worker（非用户面 builtin）",
    ("runtime/runtime.c", "px_conn_worker"): "内部 worker（非用户面 builtin）",
    ("runtime/runtime_ws.c", "ws_conn_worker"): "内部 worker（非用户面 builtin）",
    # ── C. **静态 helper**（不是用户面 builtin；唯一调用方已校验该实参） ──
    ("runtime/runtime.c", "sse_cli_prepare"):
        "静态 helper：唯一两个调用方 bi_sse_connect / bi_sse_connect_ex 均已校验 args[0].type == PX_STR",
}
ARG_GUARD2_TOTAL = 26          # 棘轮：豁免总数不得多于此数（M202：+base32 解码两件）

_SILENT = {
    "INT": [r"int_val\("],
    "NUM": [r"num_val\("],
    "STR": [r"val_cstr\(", r"px_val_cstr\("],
    "OBJ": [r"\.as\.obj->as\.str\.data", r"\.as\.obj->as\.list", r"\.as\.obj->as\.dict",
            r"\.as\.obj->as\.gen", r"\.as\.obj->as\.func", r"\.as\.i\b"],
}
_SAFE = [r"px_val_is_\w+\(\s*args\[(\d+)\]", r"px_req_\w+\(\s*args\[(\d+)\]",
         r"px_arg_\w+\(\s*args\[(\d+)\]", r"math_num\(\s*args\[(\d+)\]",
         r"px_len\(\s*args\[(\d+)\]", r"px_index\(\s*args\[(\d+)\]"]
_NATFN = re.compile(r"^(?:static\s+)?(?:LXValue|int|void|bool|const char\*)\s+(\w+)\s*"
                    r"\(LXValue\*\s*args\s*,\s*int\s*nargs")


def _nat_bodies(path):
    lines = open(os.path.join(ROOT, path), encoding="utf-8").read().split("\n")
    out, i = [], 0
    while i < len(lines):
        m = _NATFN.match(lines[i])
        if not m:
            i += 1
            continue
        depth = 0; seen = False; j = i
        for j in range(i, len(lines)):
            depth += lines[j].count("{") - lines[j].count("}")
            if "{" in lines[j]:
                seen = True
            if seen and depth <= 0:
                break
        out.append((m.group(1), i + 1, lines[i:j + 1]))
        i = j + 1
    return out


def check_arg_guards2(path):
    """→ (扫描的 native 函数数, 未豁免的缺检查站点数)"""
    nfn = nbad = 0
    for name, ln, body in _nat_bodies(path):
        nfn += 1
        blob = "\n".join(body)
        ck = set(int(x.group(1)) for x in re.finditer(r"args\[(\d+)\]\.type\s*[!=]=", blob))
        for pat in _SAFE:
            for x in re.finditer(pat, blob):
                ck.add(int(x.group(1)))
        bad = []
        for i in range(0, 10):
            if i in ck:
                continue
            u = set()
            for kind, pats in _SILENT.items():
                for pp in pats:
                    if re.search(r"args\[%d\]" % i + pp, blob) or \
                       re.search(pp[:-2] + r"\(\s*args\[%d\]" % i, blob):
                        u.add(kind)
                        break
            if u:
                bad.append((i, "/".join(sorted(u))))
        if not bad:
            continue
        why = ARG_GUARD2_EXEMPT.get((path, name))
        if why:
            continue                     # 已登记豁免（表内理由由下面单独查）
        nbad += 1
        fails.append("%s:%d %s 的实参 %s 被按类型使用却没有类型检查"
                     "（§6；确属设计请登记 ARG_GUARD2_EXEMPT 并给理由）"
                     % (path, ln, name, " ".join("[%d]%s" % (i, u) for i, u in bad)))
    return nfn, nbad


# ── 豁免表 ⇔ 源码一致 + 理由非空 + 总数棘轮 ──
_ex_all = {}
for _f in sorted(f for f in os.listdir(os.path.join(ROOT, "runtime")) if f.endswith(".c")):
    _ex_all["runtime/" + _f] = set(n for n, _l, _b in _nat_bodies("runtime/" + _f))
_ok_ex = 0
for (_f, _fn), _why in ARG_GUARD2_EXEMPT.items():
    if not _why.strip():
        fails.append("ARG_GUARD2_EXEMPT 的 %s:%s 缺理由" % (_f, _fn))
    if _fn not in _ex_all.get(_f, set()):
        fails.append("ARG_GUARD2_EXEMPT 的 %s:%s 在源码里**找不到**（改名了？表没跟着改？）" % (_f, _fn))
    else:
        _ok_ex += 1
if _ok_ex > ARG_GUARD2_TOTAL:
    fails.append("ARG_GUARD2_EXEMPT 条目变多：%d > 登记数 %d" % (_ok_ex, ARG_GUARD2_TOTAL))

print("── [S6] 全部 native 函数参数守卫 + 豁免表（M195）──")
_n1 = _n2 = 0
for f in sorted(x for x in os.listdir(os.path.join(ROOT, "runtime")) if x.endswith(".c")):
    a, b = check_arg_guards2("runtime/" + f)
    _n1 += a
    _n2 += b
    if b:
        print("   %-30s native 函数 %-4d 未豁免缺检查 %-3d ❌" % (f, a, b))
print("── [S6] native 函数合计 %d · 未豁免缺检查 **%d** · 豁免 %d/%d ──"
      % (_n1, _n2, _ok_ex, ARG_GUARD2_TOTAL))
if _n1 == 0:
    fails.append("[S6] 判据未生效：native 函数 0 个（扫描器/源码形态变了？）")

print("── [S5] native 参数类型守卫完备性（M194 §6）──")
_g = _b = 0
for f in CFILES:
    a, b = check_arg_guards("runtime/" + f)
    _g += a
    _b += b
    if a:
        print("   %-30s 多形参守卫 %-4d 缺检查 %-3d %s" % (f, a, b, "✅" if b == 0 else "❌"))
if _g == 0:
    fails.append("[S5] 判据未生效：多形参守卫 0 处（扫描器/源码形态变了？）")
print("── [S5] 多形参守卫合计 %d · 缺类型检查 **%d** ──" % (_g, _b))

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
print("✅ 静态判据通过（带码/域前缀 · 个数分码 · 混写拆分 · 未收口棘轮 · 转发豁免 · **参数守卫完备性** 六查）")
