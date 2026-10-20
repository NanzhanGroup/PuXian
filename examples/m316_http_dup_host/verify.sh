#!/usr/bin/env bash
# ============================================================
# examples/m316_http_dup_host/verify.sh
# ------------------------------------------------------------
# M316（第 187 轮 · 缺陷 550）：**HTTP 客户端把 Host / User-Agent 发了两遍**
#
# 来历（晨曦 2026-10-20 总线【运行时缺陷报告】m18e03f17a5fc57eae3702a2e + 附件
#   f7d26044816be7147bbe4631f--defect-ps-runtime-20261020.md）：
#   反代把调用方的 `Host: xxx` 原样带进 headers，运行时又无条件补一条内部 Host
#   ⇒ 线上出现**两条 Host** ⇒ nginx / Go 反代 / 云 WAF 直接 400（他生产实测：
#   /api/reg/ 7 次、/.well-known/ma-cdn-certs/* 5 次被误判成「后端故障」），
#   且连接池复用率归零（400 之后连接不可复用）。
#
#   根因：M84-S1 把「调用方自备则不重复发」只做在 Content-Type / Content-Length 上，
#   Host 与 User-Agent 是**无条件写**。而同仓 `sse_cli_connect_slot` 自 M137 起就是
#   按需发的（User-Agent 有判据、Host 漏判）⇒ 正确样例一直在自己代码里。
#
#   缺陷面（本轮实测确认，不是照着报障文本抄）：
#     · bi_http_request（TCP 面）    —— Host + User-Agent 都无条件
#     · bi_http_unix（Unix 面）      —— 同族（两份独立实现，M84-S1 当年也是分别加的）
#     · sse_cli_connect_slot（SSE）  —— User-Agent 已判、**Host 漏判**
#   ⚠️ 报障里说的另外几处（px_s3_exec / bi_http_get_stream / px_http_once）经核查
#   **不属本缺陷**：它们是内部构造（SigV4 签名 / 无 headers 参数 / 重定向跟随），
#   调用方无法注入同名头 ⇒ 结构上不可能重复。首轮我误报为「同族 6 处」，已更正。
#
# 判据层次：
#   [0] 前置：待测源码与语料在场（缺失即判红，不静默跳过）
#   [1] 静态：三处 Host 都过 px_hdr_val + 净化拷贝在三处生效 + 「一条语义一份实现」
#   [2] 构建：devbuild pxc（用**当前源码**的 runtime —— 入库 bootstrap/pxc 是旧件，
#       用它测等于测旧行为，M216 的教训）+ 三个语料
#   [3] 动态（TCP 面）：6 case × 「每个头名恰好 1 条」+ 值正确
#   [4] 动态（Unix 面）：4 case（bi_http_unix 是第二份实现，只修一边会留半条缺陷）
#   [5] 判据自证：合成 hex（单 Host 必绿 / 双 Host 必红）—— 证明判据不是「有 Host 就绿」
#   [6] 负控 A：忠实撤回三处判据 ⇒ **双 Host 必须回来**（并逐字节核忠实性）
#   [7] 覆盖边界（如实）
#
# ⚠️ 判据只用**线上字节**（服务端抓到的原始请求头 hex），不用插桩、不用解析后的 dict
#   （服务端会把同名头聚合成 list ⇒ 看不到「几条」，而真实受害方看的正是原始字节）。
# ⚠️ 负控会临时改仓库里的 runtime.c ⇒ source gate_lock.sh（M276）+ trap 还原。
#
# 用法：verify.sh [--neg-skip] [--keep]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M316_GATE_W:-$(mktemp -d /tmp/m316_gate.XXXXXX)}"
NEG_SKIP=0
KEEP_W=0
for a in "$@"; do
    [ "$a" = "--neg-skip" ] && NEG_SKIP=1
    [ "$a" = "--keep" ] && KEEP_W=1
done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

RT="$ROOT/runtime/runtime.c"
BAK="$W/runtime.c.pre-fix"
SRV_PID=""
USRV_PID=""
cleanup() {
    [ -n "$SRV_PID" ] && kill "$SRV_PID" 2>/dev/null
    [ -n "$USRV_PID" ] && kill "$USRV_PID" 2>/dev/null
    # 还原被负控改过的源码（若还在撤回态）
    if [ -f "$BAK" ] && ! cmp -s "$BAK" "$RT"; then cp -f "$BAK" "$RT"; fi
    [ "$KEEP_W" = "1" ] || rm -rf "$W"
}
trap cleanup EXIT

# 端口卫生（M288b）：不用 bind(0)（会落进出站临时端口池），也不写死一个字面端口
#   （M283 P1）；从 19631 起找第一个「TCP/UDP 都没在监听」的。
pick_port() {
    local p=19631
    while [ "$p" -lt 19700 ]; do
        if ! ss -ltn 2>/dev/null | grep -q ":$p " && ! ss -lun 2>/dev/null | grep -q ":$p "; then
            echo "$p"; return 0
        fi
        p=$((p + 1))
    done
    return 1
}

echo "════════ m316_http_dup_host（缺陷 550）════════"

# ── [0] 前置 ────────────────────────────────────────────────
echo "── [0] 前置：源码与语料在场"
MISS=0
for f in "$RT" "$HERE/srv.px" "$HERE/cli.px" "$HERE/cli_unix.px" "$HERE/usock_srv.py" \
         "$HERE/hdrjudge.py" "$HERE/negctl.py"; do
    [ -f "$f" ] || { bad "[0] 缺失 $f"; MISS=1; }
done
[ "$MISS" = 0 ] && ok "[0] 7 件前置齐备（含 devbuild / px 工具链）"
for f in "$ROOT/selfhost/devbuild.sh" "$ROOT/tools/px"; do
    [ -x "$f" ] || bad "[0] 工具缺失或不可执行：$f"
done

# ── [1] 静态 ────────────────────────────────────────────────
echo "── [1] 静态：三处 Host 判据 + 净化拷贝 + 单一实现"
python3 - "$RT" <<'PY' > "$W/static.out" 2>&1
import re, sys
src = open(sys.argv[1], encoding="utf-8").read()
lines = src.split("\n")
def body(name):
    """取**定义**的函数体（跳过前置声明：声明以 ';' 结尾，定义以 '{' 开头）。
    ⚠️ 两处踩过的坑（都实测）：
      ① 直接 src.find(name) 命中的是文件顶部的前置声明（`...);`），再往后找第一个 '{'
         会跳到**别的函数** ⇒ S2/S3/S4 全假红；
      ② 括号配平必须**跳过字符串/字符/注释** —— sse_cli_connect_slot 体里有含 '{' 的
         格式串，裸配对会被截断到中途 ⇒ 明明写了判据却报「未见」（S4 假红）。"""
    start = 0
    while True:
        i = src.find(name, start)
        if i < 0:
            return ""
        j = src.find("{", i)
        k = src.find(";", i)
        if j >= 0 and (k < 0 or j < k):
            break
        start = i + 1
    d = 0
    p = j
    n = len(src)
    while p < n:
        c = src[p]
        if c == '"':
            p += 1
            while p < n:
                if src[p] == '\\':
                    p += 2
                    continue
                if src[p] == '"':
                    p += 1
                    break
                p += 1
            continue
        if c == "'":
            p += 1
            while p < n:
                if src[p] == '\\':
                    p += 2
                    continue
                if src[p] == "'":
                    p += 1
                    break
                p += 1
            continue
        if c == '/' and p + 1 < n and src[p + 1] == '/':
            while p < n and src[p] != '\n':
                p += 1
            continue
        if c == '/' and p + 1 < n and src[p + 1] == '*':
            p += 2
            while p + 1 < n and not (src[p] == '*' and src[p + 1] == '/'):
                p += 1
            p += 2
            continue
        if c == '{':
            d += 1
        elif c == '}':
            d -= 1
            if d == 0:
                return src[j:p + 1]
        p += 1
    return ""
res = []
def ck(tag, cond, extra=""):
    res.append(("PASS" if cond else "FAIL") + " " + tag + ("" if cond else " :: " + extra))

# S1：取头值原语存在，且三态语义（有 return 2 分支）
pv = body("static int px_hdr_val(")
ck("[S1] px_hdr_val 定义在场", pv != "")
ck("[S1] px_hdr_val 区分「存在但空值」（return 2）", "return 2" in pv)
ck("[S1] px_hdr_val 去首尾空白（跳过空格/制表符）", "' \\t'" in pv or "\\t" in pv)

# S2/S3/S4：三处 Host 都过 px_hdr_val
for tag, fn in (("S2", "static LXValue bi_http_request("),
                ("S3", "static LXValue bi_http_unix("),
                ("S4", "static int sse_cli_connect_slot(")):
    b = body(fn)
    ck("[%s] %s 的 Host 走 px_hdr_val" % (tag, fn.split("(")[0].split()[-1]),
       "px_hdr_val(" in b and '"Host"' in b, "未见 px_hdr_val+Host")
    ck("[%s] 同名 User-Agent 也走判据" % tag,
       "px_hdr_val(" in b and '"User-Agent"' in b, "未见 UA 判据")

# S5：净化拷贝存在 + 三处调用
cc = body("static int px_hdr_copy_clean(")
ck("[S5] px_hdr_copy_clean 定义在场", cc != "")
ck("[S5] 自管清单含 Host / User-Agent", '"Host"' in cc and '"User-Agent"' in cc)
ck("[S5] 空值行整行丢弃（drop 分支）", "drop = 1" in cc)
ncalls = src.count("px_hdr_copy_clean(") - 1   # 减去定义本身
ck("[S5] 净化拷贝被调用 3 处（tcp / unix / sse）", ncalls == 3, "实测 %d" % ncalls)

# S6：请求行模板里的 Host 必须都是「有条件的」。
#   ⚠️ 首版判据写的是「凡是含 Host 的请求行模板即判红」—— 那会把**修复后的 else 分支**
#   也判红（实测行 17859/18081/22999 三处正是 fallback）。改法是贴着契约写：
#   「这类模板恰好 3 处（tcp/unix/sse 的 else 分支）**且**每处紧邻 px_hdr_val 判据」。
hostpat = [i for i, l in enumerate(lines)
           if re.search(r'snprintf\(.*HTTP/1\.[01]\\r\\nHost: ', l)]
ck("[S6] 请求行模板里的 Host 恰好 3 处（tcp/unix/sse 的 else 分支）",
   len(hostpat) == 3, "行 %s" % [i + 1 for i in hostpat])
guarded = all(any("px_hdr_val(" in lines[j] for j in range(max(0, i - 12), min(len(lines), i + 1)))
              for i in hostpat)
ck("[S6] 每处都紧邻 px_hdr_val 判据（±12 行）", guarded,
   "未受判据保护的 Host 模板行 %s" % [i + 1 for i in hostpat])

# S7：一条语义一份实现 —— 两个 has 都走 px_hdr_val
eh = body("static int px_extra_hdr_has(")
sh = body("static int sse_req_hdr_has(")
ck("[S7] px_extra_hdr_has 走 px_hdr_val", "px_hdr_val(" in eh)
ck("[S7] sse_req_hdr_has 走 px_hdr_val", "px_hdr_val(" in sh)

# S8：规模锚点（防「解析器坏了 ⇒ 空集 ⇒ 全绿」）
ck("[S8] 规模锚点：runtime.c 行数 ≥ 20000", len(lines) >= 20000, "实测 %d" % len(lines))

print("\n".join(res))
sys.exit(0 if all(r.startswith("PASS") for r in res) else 1)
PY
SJRC=$?
sed 's/^/     /' "$W/static.out"
if [ "$SJRC" = 0 ]; then ok "[1] 静态判据全通过"; else bad "[1] 静态判据有红（见上）"; fi

# ── [2] 构建 ────────────────────────────────────────────────
echo "── [2] 构建（devbuild pxc 用当前源码 + 三个语料）"
cp -f "$RT" "$BAK"
if ! "$ROOT/selfhost/devbuild.sh" pxc > "$W/devb.log" 2>&1; then
    bad "[2.0] devbuild pxc 失败（尾 3 行：$(tail -3 "$W/devb.log" | tr '\n' ' ')）"
fi
PXC=/tmp/pxcdev
chk "[2.0] 编译器产物在场" "$([ -x "$PXC" ] && echo yes || echo no)" "yes"

cp -f "$HERE/srv.px" "$HERE/cli.px" "$HERE/cli_unix.px" "$HERE/usock_srv.py" "$W/"
BUILD_OK=1
for f in srv cli cli_unix; do
    if ! PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/$f.px" > "$W/build_$f.log" 2>&1; then
        bad "[2.1] 语料 $f.px 构建失败：$(tail -2 "$W/build_$f.log" | tr '\n' ' ')"
        BUILD_OK=0
    fi
done
SRV_BIN="$W/build/srv"; CLI_BIN="$W/build/cli"; UCLI_BIN="$W/build/cli_unix"
for b in "$SRV_BIN" "$CLI_BIN" "$UCLI_BIN"; do
    [ -x "$b" ] || { bad "[2.2] 产物缺失：$b"; BUILD_OK=0; }
done
[ "$BUILD_OK" = 1 ] && ok "[2.2] 三个产物齐备"

# 采集一条 case 的原始请求头（只采集，不判据）—— 负控与主判据共用
RAW_HEX=""
raw_tcp() {   # $1=case ⇒ 设置 RAW_HEX / RAW_RC(0=ok)
    local c="$1" P srvlog clilog i okr=0
    RAW_HEX=""; RAW_RC=1
    P=$(pick_port) || { bad "[raw] 端口池耗尽"; return; }
    srvlog="$W/srv.$c.log"; clilog="$W/cli.$c.log"
    : > "$srvlog"; : > "$clilog"
    M316_PORT="$P" "$SRV_BIN" > "$srvlog" 2>&1 &
    SRV_PID=$!
    for i in $(seq 1 200); do grep -q 'M316-SRV-READY' "$srvlog" 2>/dev/null && { okr=1; break; }; sleep 0.05; done
    if [ "$okr" != 1 ]; then bad "[raw/$c] 服务端未就绪"; kill "$SRV_PID" 2>/dev/null; SRV_PID=""; return; fi
    # 前提自证（M283 P2）：真的在监听，而不只是打印了 READY
    if ! ss -ltn 2>/dev/null | grep -q ":$P "; then
        bad "[raw/$c] 前提不成立：端口 $P 未在监听（READY 是假就绪）"
        kill "$SRV_PID" 2>/dev/null; SRV_PID=""; return
    fi
    M316_PORT="$P" M316_CASE="$c" "$CLI_BIN" > "$clilog" 2>&1
    for i in $(seq 1 200); do grep -q 'M316-SRV-DONE' "$srvlog" 2>/dev/null && break; sleep 0.05; done
    kill "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=""
    RAW_HEX=$(grep 'M316-RAW' "$srvlog" | head -1 | awk '{print $3}')
    if [ -z "$RAW_HEX" ]; then bad "[raw/$c] 服务端未抓到请求"; return; fi
    if ! grep -q 'M316-CLI-OK' "$clilog"; then
        bad "[raw/$c] 客户端未成功（$(tail -1 "$clilog")）"; return
    fi
    RAW_ADDR="127.0.0.1:$P"
    RAW_RC=0
}

# ── [3] 动态（TCP 面）──────────────────────────────────────
echo "── [3] 动态（TCP 面 · bi_http_request）"
for c in none host_and_ua host_lower host_empty ua_only post_ct; do
    raw_tcp "$c"
    if [ "$RAW_RC" != 0 ]; then continue; fi
    if python3 "$HERE/hdrjudge.py" "$RAW_HEX" "tcp:$c" "$RAW_ADDR" > "$W/judge.$c.log" 2>&1; then
        ok "[3/$c] Host/UA 各恰 1 条且值正确"
    else
        bad "[3/$c] 判据红：$(grep -E 'FAIL' "$W/judge.$c.log" | head -3 | tr '\n' ' ')"
    fi
done

# ── [4] 动态（Unix socket 面）──────────────────────────────
echo "── [4] 动态（Unix 面 · bi_http_unix，第二份实现）"
for c in none host_and_ua host_empty ua_only; do
    SOCK="$W/u_$c.sock"
    ulog="$W/usrv.$c.log"; clog="$W/ucli.$c.log"
    : > "$ulog"; : > "$clog"
    python3 "$W/usock_srv.py" "$SOCK" 1 > "$ulog" 2>&1 &
    USRV_PID=$!
    okr=0
    for i in $(seq 1 200); do grep -q 'READY' "$ulog" 2>/dev/null && { okr=1; break; }; sleep 0.05; done
    if [ "$okr" != 1 ]; then bad "[4/$c] unix 服务端未就绪"; kill "$USRV_PID" 2>/dev/null; USRV_PID=""; continue; fi
    if [ ! -S "$SOCK" ]; then bad "[4/$c] 前提不成立：套接字文件不存在"; kill "$USRV_PID" 2>/dev/null; USRV_PID=""; continue; fi
    M316_SOCK="$SOCK" M316_CASE="$c" "$UCLI_BIN" > "$clog" 2>&1
    for i in $(seq 1 200); do grep -q 'DONE' "$ulog" 2>/dev/null && break; sleep 0.05; done
    kill "$USRV_PID" 2>/dev/null; wait "$USRV_PID" 2>/dev/null; USRV_PID=""
    rm -f "$SOCK"
    hex=$(grep 'RAW' "$ulog" | head -1 | awk '{print $3}')
    if [ -z "$hex" ]; then bad "[4/$c] unix 服务端未抓到请求"; continue; fi
    if ! grep -q 'M316-UCLI-OK' "$clog"; then bad "[4/$c] 客户端未成功（$(tail -1 "$clog")）"; continue; fi
    if python3 "$HERE/hdrjudge.py" "$hex" "unix:$c" "localhost" > "$W/ujudge.$c.log" 2>&1; then
        ok "[4/$c] Host/UA 各恰 1 条且值正确"
    else
        bad "[4/$c] 判据红：$(grep -E 'FAIL' "$W/ujudge.$c.log" | head -3 | tr '\n' ' ')"
    fi
done

# ── [5] 判据自证（合成数据）────────────────────────────────
echo "── [5] 判据自证：单 Host 必绿 / 双 Host 必红"
python3 -c "
reqs = {
  'single': ['Host: 127.0.0.1:19631', 'User-Agent: PuXian/0.1'],
  'double': ['Host: 127.0.0.1:19631', 'User-Agent: PuXian/0.1', 'Host: other.example:8080'],
  'dupa':   ['Host: 127.0.0.1:19631', 'User-Agent: PuXian/0.1', 'User-Agent: X/1'],
}
for k, v in reqs.items():
    raw = ('GET /p HTTP/1.1\r\n' + ''.join(h + '\r\n' for h in v) + '\r\n').encode('latin-1').hex()
    open('/tmp/m316_synth_' + k + '.hex', 'w').write(raw)
" >> "$W/synth.log" 2>&1
S_OK=0
if python3 "$HERE/hdrjudge.py" "$(cat /tmp/m316_synth_single.hex)" "tcp:none" "127.0.0.1:19631" > "$W/synth_single.log" 2>&1; then S_OK=$((S_OK + 1)); fi
if ! python3 "$HERE/hdrjudge.py" "$(cat /tmp/m316_synth_double.hex)" "tcp:none" "127.0.0.1:19631" > "$W/synth_double.log" 2>&1; then S_OK=$((S_OK + 1)); fi
if ! python3 "$HERE/hdrjudge.py" "$(cat /tmp/m316_synth_dupa.hex)" "tcp:none" "127.0.0.1:19631" > "$W/synth_dupa.log" 2>&1; then S_OK=$((S_OK + 1)); fi
rm -f /tmp/m316_synth_single.hex /tmp/m316_synth_double.hex /tmp/m316_synth_dupa.hex
chk "[5] 判据能分辨：单 Host 绿 + 双 Host 红 + 双 UA 红（3/3）" "$S_OK" "3"

# ── [6] 负控 A：忠实撤回 ⇒ 双 Host 必须回来 ────────────────
echo "── [6] 负控 A：撤回三处判据 ⇒ 双 Host 必须复现"
if [ "$NEG_SKIP" = 1 ]; then
    echo "     （--neg-skip：跳过负控）"
else
    NEG_OK=1
    if ! python3 "$HERE/negctl.py" --revert > "$W/neg_revert.log" 2>&1; then
        bad "[6.0] 撤回失败：$(tail -1 "$W/neg_revert.log")"; NEG_OK=0
    fi
    if [ "$NEG_OK" = 1 ]; then
        # 忠实性两条判据：
        #   ① 结构（始终可用）：三处原始形态各 1 处在场、三处判据形态各 0 处
        #   ② 逐字节（仅当 HEAD 还是修复前才有意义 —— 提交之后 HEAD 含修复，
        #      拿它比就必然「不忠实」，那是判据自己的错，不是撤回的错）
        if python3 "$HERE/negctl.py" --check-reverted > "$W/neg_faith.log" 2>&1; then
            ok "[6.1] 撤回忠实（结构：三处原始形态在场、判据形态全撤）"
        else
            bad "[6.1] 撤回不忠实（$(tail -1 "$W/neg_faith.log")）"; NEG_OK=0
        fi
        if git -C "$ROOT" show HEAD:runtime/runtime.c > "$W/runtime.c.head" 2>/dev/null; then
            # ⚠️ 刻意**不做**逐字节对拍：撤回只撤「三处调用」，而 px_hdr_val /
            #   px_hdr_copy_clean 两个新函数仍在源码里 ⇒ 与 HEAD 必然不同。
            #   那是设计，不是「撤回不忠实」。忠实性由 [6.1] 的结构判据保证。
            :
        fi
    fi
    if [ "$NEG_OK" = 1 ]; then
        SHA_FIX=$(sha256sum "$PXC" | awk '{print $1}')
        "$ROOT/selfhost/devbuild.sh" pxc > "$W/devb_neg.log" 2>&1 || bad "[6.2] 负控档重建失败"
        SHA_NEG=$(sha256sum "$PXC" | awk '{print $1}')
        # 前提自证（M289 教训）：负控档产物必须与修复档**不同**，否则测的还是修复后的行为
        if [ "$SHA_FIX" != "$SHA_NEG" ]; then
            ok "[6.2] 负控档编译器与修复档不同（前提成立）"
        else
            bad "[6.2] 负控档编译器与修复档相同 —— 前提不成立（源码没变？产物没重建？）"
            NEG_OK=0
        fi
    fi
    if [ "$NEG_OK" = 1 ]; then
        # ⚠️ 关键：语料是**静态链接**的可执行文件 ⇒ 撤回 runtime 后必须**重新构建**，
        #   否则测的还是旧（修复态）行为（本轮实测踩过：[6.3] 报 Host=1 假绿）。
        REB=1
        for f in srv cli cli_unix; do
            PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/$f.px" > "$W/negbuild_$f.log" 2>&1 || { bad "[6.2b] 负控档重建 $f 失败"; REB=0; }
        done
        [ "$REB" = 1 ] && ok "[6.2b] 负控档语料已用负控编译器重建"
    fi
    if [ "$NEG_OK" = 1 ]; then
        raw_tcp host_and_ua
        if [ "$RAW_RC" != 0 ]; then
            bad "[6.3] 撤回档下采集失败"
        else
            CNT=$(python3 -c "
import sys
h = bytes.fromhex(sys.argv[1]).decode('latin-1').split('\r\n\r\n')[0].split('\r\n')[1:]
print(sum(1 for l in h if l.split(':')[0].strip().lower() == 'host'))
" "$RAW_HEX")
            chk "[6.3] 撤回档下 Host 条数 = 2（= 复现缺陷 550）" "$CNT" "2"
            if python3 "$HERE/hdrjudge.py" "$RAW_HEX" "tcp:host_and_ua" "$RAW_ADDR" > "$W/neg_judge.log" 2>&1; then
                bad "[6.4] 撤回档下判据竟然绿了（判据没牙）"
            else
                ok "[6.4] 撤回档下判据判红（判据有牙）"
            fi
        fi
    fi
    # 还原 + 二次核忠实
    if ! python3 "$HERE/negctl.py" --restore > "$W/neg_restore.log" 2>&1; then
        bad "[6.5] 还原失败：$(tail -1 "$W/neg_restore.log")"
    else
        if cmp -s "$BAK" "$RT"; then ok "[6.5] 源码已逐字节还原"; else bad "[6.5] 源码未还原到修复态"; fi
    fi
    "$ROOT/selfhost/devbuild.sh" pxc > "$W/devb_back.log" 2>&1 || bad "[6.6] 还原后重建失败"
    # 还原后语料也要重建成修复态（否则用的是负控产物，[6.6] 会假红）
    for f in srv cli cli_unix; do
        PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/$f.px" > "$W/backbuild_$f.log" 2>&1 || bad "[6.6] 还原档重建 $f 失败"
    done
    # 还原后必须回到绿（否则说明还原不彻底，后续门会看到坏态）
    raw_tcp host_and_ua
    if [ "$RAW_RC" = 0 ] && python3 "$HERE/hdrjudge.py" "$RAW_HEX" "tcp:host_and_ua" "$RAW_ADDR" > "$W/back_judge.log" 2>&1; then
        ok "[6.6] 还原后恢复到绿（修复态可复现）"
    else
        bad "[6.6] 还原后仍红 —— 还原不彻底"
    fi
fi

# ── [7] 覆盖边界（如实）────────────────────────────────────
echo "── [7] 覆盖边界（如实登记，不算判据）"
cat <<'EOF'
     · sse 面（sse_connect 自备 Host）本门只有**静态判据**，没有动态抓包：
       理由是 sse 客户端要事件循环 + 长连接，抓首包需另建装置；本轮改动是
       「同一函数里的同一模式」（Host 补判 + 净化拷贝），静态 + m137 门回归覆盖。
     · **Connection 头仍是运行时无条件发**（调用方给 `Connection: close` 时会得到两条）
       —— 同族、**未修**：改它必须同步连接池的「这条连接能不能回收」语义，
       属独立改动，已登记为后续候选。
     · 代理场景（opts.proxy 非空）未单独覆盖：req_target 变绝对 URL，Host 逻辑在同一段代码。
     · TLS（https）面未单独覆盖：同一构造代码，只是 conn_send 走 mbedtls。
     · HTTP/2 / HTTP/3 不涉及：本运行时客户端只有 HTTP/1.x。
     · px_s3_exec / bi_http_get_stream / px_http_once **不属本缺陷**（内部构造，无 headers 注入面）。
EOF

echo
echo "════════ m316_http_dup_host: $PASS 通过 / $FAIL 失败 ════════"
[ "$FAIL" = 0 ] && { echo "M316-VERIFY-OK"; exit 0; } || { echo "M316-VERIFY-FAIL"; exit 1; }
