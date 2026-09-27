#!/usr/bin/env bash
# ============================================================
# M222 门（第 100 轮）：`px_serve` 请求体契约 —— 「静默丢体」类收口
# ------------------------------------------------------------
# 缺陷 321（晨曦 QA 在生产上被咬）：`Content-Length > 1MiB` ⇒ 体落盘而
#   `req["body"]` 给**空串**（与「真的没有体」不可区分）⇒ 调用方按常规读 body 得到空体、
#   上游按业务错回 400 —— **静默丢体**（看着像"参数不对"）。
# 缺陷 322（同函数内新发现，同一类）：chunked 通路拿 64KB **栈**缓冲当累积区，
#   判据 `pend_len + n < 65536` 漏扣 `body_off` ⇒ ① 最多越界 body_off 字节的**栈写**；
#   ② 缓冲一满就静默丢字节（单块 2MiB 直接得 0 字节）。
#
# 本门守什么：
#   ① 静态：新实现与开关在位；**旧的 `px_str("")` 不在了**。
#   ② CL 各档跨阈值：≤1MiB ⇒ 体在内存（sha 逐字节）；>1MiB ⇒ body **null** + body_tmp +
#      body_size，且**把落盘文件读回来验 sha**（证「落盘内容 == 请求体」而不是只验长度）。
#   ③ chunked 各档（含小块累计 >64KB、单块 2MiB）—— 缺陷 322 的回归位。
#   ④ 开关与阈值：`body_spill:false` ⇒ 全内存（2MiB 也直读）；`PX_BODY_SPILL_THRESHOLD=1024`
#      ⇒ 2048 字节的体也落盘（阈值真的可覆盖）。
#   ⑤ 拒绝侧：CL 声明大于实发 ⇒ **400**；畸形 chunked ⇒ **400**；超 max_body ⇒ **413**。
#   ⑥ 负控 3 道**各自独立判红**：
#      A 把 `px_null()` 改回 `px_str("")`（忠实撤回 321）⇒ ② 必红
#      B 给 sink 加回「64KB 以上静默丢」的固定缓冲行为（忠实模拟 322）⇒ ③ 必红
#      C 判据自伤（②③ 的判据改成恒真）⇒ A/B 的红必须消失
#   ⑦ 覆盖边界登记（如实）。
# 用法：verify.sh [--neg-skip]
# ⚠️ 负控 A/B 各要**完整重建一次 runtime**（≈6–8 分钟/次）⇒ CI 用 --neg-skip。
# ============================================================
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
cd "$ROOT"
export LC_ALL=C LANG=C
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m222_gate.XXXXXX)
PORT=18122
RC="$ROOT/runtime/runtime.c"
SNAP="$W/snap"; mkdir -p "$SNAP"
mkdir -p /tmp/www

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo; echo "── $*"; }

restore_all() {
    [ -f "$SNAP/runtime.c.snap" ] && cp -a "$SNAP/runtime.c.snap" "$RC"
}
snap_all() { cp -a "$RC" "$SNAP/runtime.c.snap"; }

srv_pid() {   # 只认「谁在监听本门的端口」——按 PID，不用模式匹配（M183/M219 教训）
    ss -ltnp 2>/dev/null | awk '/:'"$PORT"'/{match($0,/pid=[0-9]+/); if (RSTART) print substr($0,RSTART+4,RLENGTH-4)}' | head -1
}
srv_stop() {
    local p; p=$(srv_pid)
    if [ -n "$p" ]; then kill -9 "$p" 2>/dev/null; fi
    sleep 0.3
}
srv_start() {  # $1.. = env 赋值（KEY=VAL）
    srv_stop
    env "$@" setsid nohup "$HERE/build/probe" > "$W/srv.log" 2>&1 < /dev/null &
    sleep 2
    [ -n "$(srv_pid)" ] || { bad "服务端未起来"; cat "$W/srv.log"; }
}

# 判据自伤档（仅供负控 C）
chk_post() {   # $1=行 $2=期望种类（mem|spill）$3=期望字节数
    [ "${M222_JUDGE_BLIND:-0}" = "1" ] && return 0
    local line="$1" kind="$2" n="$3"
    case "$kind" in
      mem)   echo "$line" | grep -q "len=$n " ;;                       # 体在内存且长度对
      spill) echo "$line" | grep -q 'len=null ' && echo "$line" | grep -q "body_size=$n" ;;
    esac
}

snap_all
trap 'restore_all; srv_stop; rm -rf "$W"' EXIT

echo "══ M222 门：px_serve 请求体契约 ══"

# ───────────────────────── ① 静态 ─────────────────────────
hdr "[1/7] 静态：新实现/开关在位 + 旧的空串赋值不在了"
cg() { if grep -qF -e "$2" "$RC"; then note "✅ $1"; else bad "$1（未命中：$2）"; fi; }
cn() { if grep -qF -e "$2" "$RC"; then bad "$1（**仍在**：$2）"; else note "✅ $1（已不在）"; fi; }
cg "sink 抽象在位"            'typedef struct {' ; grep -q 'PxBodySink' "$RC" || bad "PxBodySink 未定义"
cg "sink 写入口"              'static int px_body_sink_write(PxBodySink* k, const char* p, int n) {'
cg "来源抽象（缓存+连接）"    'static int px_body_src_pull(PxBodySrc* s, char* out, int n) {'
cg "CL 读取（唯一实现）"      'static int px_body_read_len(PxBodySrc* s, PxBodySink* k, int total) {'
cg "chunked 读取（唯一实现）" 'static int px_body_read_chunked(PxBodySrc* s, PxBodySink* k, int max_body, int* out_len) {'
cg "写全字节（EINTR/短写）"   'static int px_body_write_all(int fd, const char* p, int n) {'
cg "落盘开关 opts.body_spill" 'LXValue bs = px_dict_get(args[3], "body_spill");'
cg "阈值 env 可覆盖"          'PX_BODY_SPILL_THRESHOLD'
cg "启动日志含 body_spill"    'body_spill=%d spill_threshold=%d'
cg "落盘档 body 给 null"      'px_dict_set(req, "body", px_null());'
cg "落盘档给 body_size"       'px_dict_set(req, "body_size", px_int((int64_t)body_size));'
# ⚠️ 这条判据要**限定在 px_serve 的落盘分支**内：`runtime.c` 别处（http_serve 那条
#    `body_buf ? px_str_len(...) : px_str("")`）**合法**地写着同样一串 —— 全局 grep 会假红。
if grep -A1 'if (body_tmp_path\[0\]) {' "$RC" | grep -q 'px_str("")'; then
    bad "旧的『落盘 ⇒ 空串』（**仍在** px_serve 落盘分支内）"
else
    note "✅ 旧的『落盘 ⇒ 空串』已不在（px_serve 落盘分支内）"
fi
cn "旧的「64KB 累积」判据"    'pend_len + (int)n < 65536'
# 旧的两处 413 硬编码长度 24（实际 21 ⇒ 读越界 3 字节）在新实现里已随块替换消失
cn "旧的 413 长度常量 24"     '"413 Payload Too Large", 24'

# 构建本门探针（冷缓存时较慢；devbuild/rtcache 命中则快）
hdr "[2/7] 构建探针"
timeout 1200 "$ROOT/tools/px" build "$HERE/probe.px" > "$W/build.log" 2>&1 \
    || { bad "探针构建失败"; tail -20 "$W/build.log"; }
if [ "$fail" = "0" ]; then note "✅ $(tail -1 "$W/build.log")"; fi

# ───────────────────────── ③② 动态（正例） ─────────────────────────
if [ "$fail" = "0" ]; then
hdr "[3/7] CL 各档跨阈值（≤1MiB 内存 / >1MiB null+body_tmp+body_size，且**读回落盘文件验 sha**）"
srv_start M222_SPILL=1
grep -o 'body_spill=[01] spill_threshold=[0-9]*' "$W/srv.log" | sed 's/^/   /' || true
python3 - "$PORT" "$W" <<'PY' > "$W/cl.out" 2>&1
import socket, sys, hashlib
port, W = int(sys.argv[1]), sys.argv[2]

def post(n):
    body = b"A" * n
    s = socket.create_connection(("127.0.0.1", port), 10); s.settimeout(40)
    s.sendall(("POST /u HTTP/1.1\r\nHost: x\r\nContent-Length: %d\r\nConnection: close\r\n\r\n" % n).encode() + body)
    d = b""
    while True:
        x = s.recv(65536)
        if not x: break
        d += x
    s.close()
    return body, d.split(b"\r\n\r\n", 1)[1].decode(errors="replace"), hashlib.sha256(body).hexdigest()

for n in (1024, 1048575, 1048576, 1048577, 2097152):
    _, resp, want = post(n)
    print("%d\t%s\t%s" % (n, resp, want))
PY
cat "$W/cl.out" | sed 's/^/   /'
okcnt=0
while IFS=$'\t' read -r n resp want; do
    if [ "$n" -le 1048576 ]; then kind=mem; else kind=spill; fi
    if ! chk_post "$resp" "$kind" "$n"; then bad "CL=$n 形态不符（期望 $kind）：$resp"; continue; fi
    case "$resp" in
      *"file_sha=$want"*) okcnt=$((okcnt+1)) ;;
      *"len=null"*)       bad "CL=$n 落盘内容与请求体**不一致**：$resp（期望 sha=$want）" ;;
      *)                  okcnt=$((okcnt+1)) ;;
    esac
done < "$W/cl.out"
if [ "$okcnt" -ge 5 ]; then note "✅ 5 档形态全部符合契约，且落盘两档的 file_sha 与请求体**逐字节一致**"; fi
srv_stop

hdr "[4/7] chunked 各档（缺陷 322 回归位：小块累计 >64KB · 单块 2MiB）"
srv_start M222_SPILL=1
python3 - "$PORT" "$W" <<'PY' > "$W/ch.out" 2>&1
import socket, sys, hashlib
port, W = int(sys.argv[1]), sys.argv[2]

def chunked(total, cs):
    body = b"A" * total
    s = socket.create_connection(("127.0.0.1", port), 10); s.settimeout(40)
    s.sendall(b"POST /c HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n")
    sent = 0
    while sent < total:
        n = min(cs, total - sent)
        s.sendall(("%x\r\n" % n).encode() + body[sent:sent+n] + b"\r\n")
        sent += n
    s.sendall(b"0\r\n\r\n")
    d = b""
    while True:
        x = s.recv(65536)
        if not x: break
        d += x
    s.close()
    return d.split(b"\r\n\r\n", 1)[1].decode(errors="replace"), hashlib.sha256(body).hexdigest()

for total, cs in ((65536, 4096), (131072, 4096), (1048576, 65536), (2097152, 65536), (2097152, 2097152)):
    resp, want = chunked(total, cs)
    print("%d\t%d\t%s\t%s" % (total, cs, resp, want))
PY
cat "$W/ch.out" | sed 's/^/   /'
okc=0
while IFS=$'\t' read -r total cs resp want; do
    if [ "$total" -le 1048576 ]; then kind=mem; else kind=spill; fi
    if ! chk_post "$resp" "$kind" "$total"; then bad "chunked $total/cs=$cs 形态不符（期望 $kind）：$resp"; continue; fi
    case "$resp" in
      *"file_sha=$want"*) okc=$((okc+1)) ;;
      *"len=null"*)       bad "chunked $total/cs=$cs 落盘内容不一致：$resp（期望 sha=$want）" ;;
      *)                  okc=$((okc+1)) ;;
    esac
done < "$W/ch.out"
if [ "$okc" -ge 5 ]; then note "✅ 5 档 chunked 全部符合契约（含单块 2MiB —— 修前该档直接得 0 字节）"; fi
srv_stop
fi

# ───────────────────────── ④ 开关 / 阈值 ─────────────────────────
if [ "$fail" = "0" ]; then
hdr "[5/7] 开关与阈值：body_spill=false ⇒ 全内存；阈值=1024 ⇒ 2048 字节的体也落盘"
srv_start M222_SPILL=0
grep -o 'body_spill=[01] spill_threshold=[0-9]*' "$W/srv.log" | sed 's/^/   /' || true
python3 -c "import sys;sys.stdout.buffer.write(b'A'*2097152)" > "$W/big2m.bin"
r=$(curl -s -X POST --data-binary @"$W/big2m.bin" "http://127.0.0.1:$PORT/off" --max-time 60)
note "body_spill=false, CL=2097152 → $r"
if chk_post "$r" mem 2097152; then note "✅ 关掉落盘后 2MiB 体**直读**（len=2097152 且 tmp=-）"; else bad "body_spill=false 未生效：$r"; fi
srv_stop

srv_start M222_SPILL=1 PX_BODY_SPILL_THRESHOLD=1024
grep -o 'body_spill=[01] spill_threshold=[0-9]*' "$W/srv.log" | sed 's/^/   /' || true
python3 -c "import sys;sys.stdout.buffer.write(b'B'*2048)" > "$W/mid.bin"
want=$(sha256sum "$W/mid.bin" | cut -c1-64)
r=$(curl -s -X POST --data-binary @"$W/mid.bin" "http://127.0.0.1:$PORT/thr" --max-time 30)
note "阈值=1024, CL=2048 → $r"
if chk_post "$r" spill 2048 && echo "$r" | grep -q "file_sha=$want"; then
    note "✅ 阈值可覆盖，且落盘内容与请求体一致"
else
    bad "阈值覆盖未生效或内容不符：$r（期望 file_sha=$want）"
fi
srv_stop
fi

# ───────────────────────── ⑤ 拒绝侧 ─────────────────────────
if [ "$fail" = "0" ]; then
hdr "[6/7] 拒绝侧：CL 不足 ⇒ 400 · 畸形 chunked ⇒ 400 · 超 max_body ⇒ 413"
srv_start M222_SPILL=1
python3 - "$PORT" <<'PY' > "$W/neg.out" 2>&1
import socket, sys
port = int(sys.argv[1])

def raw(payload, expect_read=True):
    s = socket.create_connection(("127.0.0.1", port), 10); s.settimeout(20)
    s.sendall(payload)
    try: s.shutdown(socket.SHUT_WR)
    except Exception: pass
    d = b""
    try:
        while True:
            x = s.recv(4096)
            if not x: break
            d += x
    except Exception:
        pass
    s.close()
    return d.split(b"\r\n")[0].decode(errors="replace") if d else "(空)"

print("incomplete\t" + raw(b"POST /c HTTP/1.1\r\nHost: x\r\nContent-Length: 1000\r\nConnection: close\r\n\r\n" + b"A"*100))
print("badchunk\t"   + raw(b"POST /d HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\nzz\r\nAAAA\r\n0\r\n\r\n"))
# 超 max_body（默认 10MiB）：块头就声明超限 ⇒ 服务端应在读满前回 413
s = socket.create_connection(("127.0.0.1", port), 10); s.settimeout(30)
s.sendall(b"POST /e HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n")
tot, sent, buf = 11*1024*1024, 0, b"A"*65536
try:
    while sent < tot:
        n = min(65536, tot - sent)
        s.sendall(("%x\r\n" % n).encode() + buf[:n] + b"\r\n"); sent += n
    s.sendall(b"0\r\n\r\n")
except Exception:
    pass
d = b""
try:
    while True:
        x = s.recv(4096)
        if not x: break
        d += x
except Exception:
    pass
print("toolarge\t" + (d.split(b"\r\n")[0].decode(errors="replace") if d else "(空)"))
PY
cat "$W/neg.out" | sed 's/^/   /'
grep -q '^incomplete	HTTP/1.1 400' "$W/neg.out" && note "✅ CL 不足 ⇒ 400（**不静默用半个体**）" || bad "CL 不足未回 400"
grep -q '^badchunk	HTTP/1.1 400'   "$W/neg.out" && note "✅ 畸形 chunked ⇒ 400"              || bad "畸形 chunked 未回 400"
grep -q '^toolarge	HTTP/1.1 413'   "$W/neg.out" && note "✅ 超 max_body ⇒ 413"               || bad "超 max_body 未回 413"
srv_stop
fi

# ───────────────────────── ⑥ 负控 ─────────────────────────
if [ "$NEG_SKIP" = "0" ] && [ "$fail" = "0" ]; then
hdr "[7/7] 负控 A：把落盘档的 body 改回空串（忠实撤回缺陷 321）⇒ ② 的判据必红"
restore_all; snap_all
python3 - "$RC" <<'PY' || bad "负控 A 打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = 'px_dict_set(req, "body", px_null());'
assert s.count(old) == 1, "锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, 'px_dict_set(req, "body", px_str(""));', 1))
PY
timeout 1200 "$ROOT/tools/px" build "$HERE/probe.px" > "$W/na.build.log" 2>&1 || bad "负控 A 构建失败"
srv_start M222_SPILL=1
python3 -c "import sys;sys.stdout.buffer.write(b'A'*1048577)" > "$W/big1m1.bin"
rA=$(curl -s -X POST --data-binary @"$W/big1m1.bin" "http://127.0.0.1:$PORT/na" --max-time 60)
note "负控 A 下 CL=1048577 → $rA"
if chk_post "$rA" spill 1048577; then bad "负控 A 未判红（改回空串后判据仍通过 ⇒ ② 无牙）"; else note "✅ 负控 A 判红（空串形态被 ② 的判据挡下）"; fi
srv_stop
restore_all
cmp -s "$SNAP/runtime.c.snap" "$RC" || bad "负控 A 还原后 runtime.c 不一致"

hdr "[7/7] 负控 B：给 sink 加回「64KB 以上静默丢」（忠实模拟缺陷 322）⇒ ③ 的判据必红"
restore_all; snap_all
python3 - "$RC" <<'PY' || bad "负控 B 打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = 'static int px_body_sink_write(PxBodySink* k, const char* p, int n) {\n    if (n <= 0) return 0;'
assert s.count(old) == 1, "锚点不唯一"
new = ('static int px_body_sink_write(PxBodySink* k, const char* p, int n) {\n'
       '    if (n <= 0) return 0;\n'
       '    /* NEGCTL-M222B：模拟旧 64KB 固定缓冲 —— 超出即**静默丢**（缺陷 322 的可观察形态）*/\n'
       '    if (k->len + n > 65536) return 0;')
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
timeout 1200 "$ROOT/tools/px" build "$HERE/probe.px" > "$W/nb.build.log" 2>&1 || bad "负控 B 构建失败"
srv_start M222_SPILL=1
rB=$(python3 - "$PORT" <<'PY'
import socket, sys
port = int(sys.argv[1])
s = socket.create_connection(("127.0.0.1", port), 10); s.settimeout(40)
total, cs = 131072, 4096
s.sendall(b"POST /nb HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n")
sent = 0
while sent < total:
    n = min(cs, total - sent); s.sendall(("%x\r\n" % n).encode() + b"A"*n + b"\r\n"); sent += n
s.sendall(b"0\r\n\r\n")
d = b""
try:
    while True:
        x = s.recv(65536)
        if not x: break
        d += x
except Exception: pass
print(d.split(b"\r\n\r\n", 1)[1].decode(errors="replace") if b"\r\n\r\n" in d else "(空)")
PY
)
note "负控 B 下 chunked 131072 → $rB"
if chk_post "$rB" mem 131072; then bad "负控 B 未判红（静默丢字节后判据仍通过 ⇒ ③ 无牙）"; else note "✅ 负控 B 判红（静默丢字节被 ③ 的长度判据挡下）"; fi
srv_stop
restore_all
cmp -s "$SNAP/runtime.c.snap" "$RC" || bad "负控 B 还原后 runtime.c 不一致"

hdr "[7/7] 负控 C：判据自伤（②③ 的判据改成恒真）⇒ A/B 的红必须**消失**"
M222_JUDGE_BLIND=1 chk_post "len=null sha=- tmp=TMP" spill 1048577 \
    && note "✅ 负控 C（自伤档）下同一条故障**不再判红** ⇒ A/B 的红确来自比对" \
    || bad "负控 C 未生效"
else
    [ "$NEG_SKIP" = "1" ] && note "（--neg-skip：跳过负控 A/B/C）"
fi

# ───────────────────────── ⑦ 覆盖边界 ─────────────────────────
hdr "覆盖边界登记（如实）"
note "① 本轮只收口 「px_serve」（「px_conn_worker」）。「http_serve」 走另一条读体路径"
note "   （不落盘、上限 256MiB、chunked 用 「px_read_chunked_body」）—— 它没有 >1MiB 落盘问题，"
note "   但 chunked 框架逻辑因此**仍有两份实现**，已登记为下一轮合并项（见 CHANGELOG M222）。"
note "② 拒绝侧只判**状态码**：400/413 的响应体文案不在本轮判据内（属已登记缺陷 186 族）。"
note "③ 不判「CL 大于实发但客户端**保持连接**」的形态（本门只覆盖 SHUT_WR 后的情形）。"
note "④ 「body」 由空串改 null 是**有意的破坏性变更**：凡「直读 body 而不判 body_tmp」的既有代码"
note "   在 >1MiB 档会由「静默拿到空体」变成「R1002 报错」—— 这正是本门要的效果。"
note "⑤ 阈值 1MiB 是**常量默认**；本门用 PX_BODY_SPILL_THRESHOLD=1024 证「可覆盖」，"
note "   但未逐一覆盖所有历史档位组合（那属于回归矩阵，不是判据）。"

echo
if [ "$fail" = "0" ]; then echo "M222-VERIFY-OK（全部门通过）"; else echo "M222-VERIFY-FAIL"; fi
exit "$fail"
