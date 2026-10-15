#!/usr/bin/env bash
# ============================================================
# M116 验证（qg-issue 71 D2/D4/D5/D6/D7）
# ------------------------------------------------------------
# 判据（每条都直接比可观测事实，不靠"应该没问题"）：
#   ① D4/D6 + D2 守卫写法：双轨（px run / px build）都必须全 PASS
#   ② D2 负控：**两条轨**跑 dict_missing_key.px 都必须非零退出、stderr 含 R1008
#      且出错点之后的 "after" 不得出现（两轨同口径 = 修复的核心判据）
#   ③ D6：mkdir mode 按 umask 折算的绝对权限位断言（在 ① 内）
#   ④ D5：dns_txt TCP 回退（多记录域名）—— 需外网，离线记 SKIP 不计红
#   ⑤ D7 负控：handler 出错 → **500** + **服务存活**（VM 轨与 C 轨各跑一遍）
#   ⑥ 门自检负控：把断言值改错必须变红（防门自欺）
#   ⑦ D7 端口：**区间避开 ephemeral 池 + 先确认空闲再用** —— 门自身竞态
#      （原 `20000+RANDOM%20000` 落进宿主出站端口池 32768..，偶发 Address already in use ⇒ 假红）
#   ⑧ D7 就绪轮询：**取码必须归一化**（2026-10-15 · M308 第 1 轮复跑实测）——
#      $(curl … || echo 000) 在**连接失败**时得到 **000000** ⇒ != "000" 恒真 ⇒
#      第一次轮询就 break（等于没等就绪）⇒ /bad 撞在 listen 之前 ⇒ 假红（负载越高越必红）。
#      ⇒ 改 _m116_http（失败只可能是 000）+ 真等到有响应（≤30s）+ 绑定失败换端口重试（≤3 次）。
# 用法：bash examples/m116_builtin_semantics/verify.sh
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m116_builtin_semantics
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"; [ -n "${SRV_PID:-}" ] && kill "$SRV_PID" 2>/dev/null' EXIT

pass=0; fail=0; skip=0
ok()   { echo "  ✅ $1"; pass=$((pass+1)); }
bad()  { echo "  ❌ $1"; fail=$((fail+1)); }
skips(){ echo "  ⏭  $1"; skip=$((skip+1)); }

echo "== M116 验证（PuXian 缺陷集 D2/D4/D5/D6/D7）=="

# ---------- ① D4/D6/D2 守卫：双轨 ----------
echo "[1/6] 双轨语义断言（$DIR/m116_semantics.px）"
./tools/px run "$DIR/m116_semantics.px" > "$TMP/i.log" 2>&1; rc_i=$?
if [ $rc_i -eq 0 ] && tail -1 "$TMP/i.log" | grep -q 'M116_SEMANTICS:.*0F'; then
    ok "解释轨（px run）：$(tail -1 "$TMP/i.log")"
else
    bad "解释轨失败 rc=$rc_i"; tail -12 "$TMP/i.log"
fi
./tools/px build "$DIR/m116_semantics.px" > "$TMP/b.log" 2>&1
if [ -x "$DIR/build/m116_semantics" ] &&
   ./"$DIR/build/m116_semantics" > "$TMP/c.log" 2>&1 &&
   tail -1 "$TMP/c.log" | grep -q 'M116_SEMANTICS:.*0F'; then
    ok "编译轨（px build，VM 默认轨）：$(tail -1 "$TMP/c.log")"
else
    bad "编译轨失败"; tail -12 "$TMP/c.log" 2>/dev/null; tail -5 "$TMP/b.log"
fi

# ---------- ② D2 负控：两轨同口径 ----------
echo "[2/6] D2 负控：缺键下标两轨都必须以 R1008 失败"
d2_probe() {   # $1=标签 $2...=命令
    local label="$1"; shift
    local out="$TMP/d2.$label.log"
    "$@" > "$out" 2>&1; local rc=$?
    if [ $rc -eq 0 ]; then bad "$label：缺键竟成功退出（静默 null 回归）"; return; fi
    if ! grep -q 'R1008' "$out"; then bad "$label：退出码非零但无 R1008（判据不符）"; sed -n 1,3p "$out"; return; fi
    if ! grep -q "字典没有键 'nope'" "$out"; then bad "$label：R1008 文案缺键名"; sed -n 1,3p "$out"; return; fi
    if grep -q '^after$' "$out"; then bad "$label：出错后仍继续执行（不是硬错误）"; return; fi
    ok "$label：rc=$rc · $(grep -m1 R1008 "$out" | sed 's/^ *//')"
}
d2_probe "解释轨" ./tools/px run "$DIR/dict_missing_key.px"
./tools/px build "$DIR/dict_missing_key.px" > "$TMP/d2.build.log" 2>&1
d2_probe "编译轨(VM)" ./"$DIR/build/dict_missing_key"
./tools/pxc build "$DIR/dict_missing_key.px" > "$TMP/d2.buildc.log" 2>&1
if [ -x "$DIR/build/dict_missing_key" ]; then
    cp -f "$DIR/build/dict_missing_key" "$TMP/d2c.bin" 2>/dev/null
fi
# C 轨另编一份（tools/pxc 与 tools/px 输出同名同目录，故用 PX_BUILD_ENGINE 隔离）
PX_BUILD_ENGINE=c ./tools/px build "$DIR/dict_missing_key.px" > "$TMP/d2.buildc2.log" 2>&1
if [ -x "$DIR/build/dict_missing_key" ]; then
    cp -f "$DIR/build/dict_missing_key" "$TMP/d2c.bin"
    d2_probe "编译轨(C)" "$TMP/d2c.bin"
fi

# ---------- ③ D5：dns_txt TCP 回退（需外网） ----------
echo "[3/6] D5：dns_txt UDP 截断 → TCP 回退（多记录域名）"
cat > "$TMP/d5.px" <<'PXEOF'
# 先探网：getaddrinfo 路径（dns_lookup）失败 ⇒ 环境无外网 ⇒ 退出码 2（门记 SKIP）
let probe = dns_lookup("google.com")
if type(probe) == "result":
    print("NET-DOWN: " + str(probe))
    exit(2)
let names = ["google.com", "cloudflare.com"]
var bad = 0
for n in names:
    let r = dns_txt(n)
    if type(r) == "result":
        print("ERR " + n + ": " + str(r))
        bad += 1
    else:
        print("OK " + n + " n=" + str(len(r)))
if bad > 0:
    exit(1)
PXEOF
./tools/px build "$TMP/d5.px" > "$TMP/d5.build.log" 2>&1
"$TMP/build/d5" > "$TMP/d5.log" 2>&1; d5rc=$?
if [ $d5rc -eq 0 ]; then
    ok "TCP 回退生效：$(tr '\n' ' ' < "$TMP/d5.log")"
elif [ $d5rc -eq 2 ]; then
    skips "环境无外网（dns_lookup 已失败），跳过：$(tr '\n' ' ' < "$TMP/d5.log")"
else
    bad "dns_txt 多记录域名失败（网通则必红）：$(tr '\n' ' ' < "$TMP/d5.log")"
fi

# ---------- ④ D7：handler 出错 → 500 + 服务存活 ----------
echo "[4/6] D7 负控：handler 运行时错误 → 500（且服务不死）"
if ! command -v curl >/dev/null 2>&1; then
    skips "无 curl，跳过 D7 端到端"
else
    mkdir -p /tmp/m116_docroot
    # ── 端口选择：**两道防线**（修门自身竞态，判据不变）──────────────────
    #   2026-10-14 · M307 第 5 轮实测：原 `$((20000 + RANDOM % 20000))` 裸选偶发撞占用端口
    #   ⇒ `px_serve: 绑定端口 … 失败：Address already in use` ⇒ 服务起不来 ⇒ curl 拿 000 记红。
    #   证据：同一二进制 + 同一环境变量连跑 100 次 ⇒ 98 绿 / 2 红（两红皆此因）；单跑 5/5 绿。
    #   ① **区间避开宿主 ephemeral 出站端口池**（M288b 同款）：`ip_local_port_range` 实测
    #      下界 = 32768 ⇒ 原区间 32768..39999 会与**陌生出站连接**抢端口（本机实测占用者：
    #      v2ray:33847 / ls-server:37079）。上界改为 min(32767, 下界-1)，保底 ≥21000。
    #   ② **先确认空闲再用**（无 LISTEN、无 TIME_WAIT）⇒ 兜住残余 TOCTOU。
    #   ⇒ 这是**门自身的竞态**，不是产品回归（与 registry/mock「启动改轮询就绪」同族）。
    _m116_ephem_lo="$(awk '{print $1}' /proc/sys/net/ipv4/ip_local_port_range 2>/dev/null || echo 32768)"
    case "$_m116_ephem_lo" in ''|*[!0-9]*) _m116_ephem_lo=32768 ;; esac
    _m116_port_hi=$(( _m116_ephem_lo - 1 ))
    [ "$_m116_port_hi" -gt 32767 ] && _m116_port_hi=32767
    [ "$_m116_port_hi" -lt 21000 ] && _m116_port_hi=21000
    _m116_port_span=$(( _m116_port_hi - 20000 + 1 ))
    _m116_port_free() {  # $1=port ⇒ 0=空闲（无 LISTEN、无 TIME_WAIT）
        ss -H -ltn "sport = :$1" 2>/dev/null | grep -q . && return 1
        ss -H -tan "sport = :$1" 2>/dev/null | grep -q . && return 1
        return 0
    }
    _m116_pick_port() {
        local p n=0
        while [ "$n" -lt 50 ]; do
            p=$((20000 + RANDOM % _m116_port_span))
            _m116_port_free "$p" && { printf '%s\n' "$p"; return 0; }
            n=$((n+1))
        done
        printf '%s\n' "$((20000 + RANDOM % _m116_port_span))"
    }
    # ── HTTP 取码（**必须归一化**：连接失败只能有一种写法 `000`）─────────────
    #   2026-10-15 · M308 第 1 轮静默复跑实测真因（**门自身缺陷**，非产品回归）：
    #   原写法 $(curl … -w '%{http_code}' … || echo 000) 在**连接失败**时，
    #   curl 自己已把 000 打到 stdout 且退出码非零 ⇒ || echo 000 再补一个 ⇒
    #   捕获值 = **000000**（本机实测复现）。就绪轮询的 [ "$_pre" != "000" ] 因此
    #   **恒真** ⇒ 第一次轮询（此刻服务还没 listen）就 break ⇒ 等于**没等就绪** ⇒
    #   /bad 撞在 bind/listen 之前 ⇒ 拿 000000 ⇒ 判红。负载越高越必红（本轮 loadavg 6~9）。
    #   ⇒ 取码统一走 _m116_http；判据（code=500 / alive≠000）一字不改。
    _m116_http() {   # $1=超时秒 $2=url [$3=body 落盘] ⇒ 打印 http_code（失败=000）
        local c
        if [ -n "${3:-}" ]; then
            c=$(curl -s -o "$3" -w '%{http_code}' --max-time "$1" "$2" 2>/dev/null)
        else
            c=$(curl -s -o /dev/null -w '%{http_code}' --max-time "$1" "$2" 2>/dev/null)
        fi
        case "$c" in ''|*[!0-9]*) c=000 ;; esac
        printf '%s' "$c"
    }
    d7_case() {   # $1=标签 $2=产物路径
        local label="$1" bin="$2" attempt=1
        local port pid _i _pre code body alive
        while :; do
            port=$(_m116_pick_port)
            M116_PORT=$port "$bin" > "$TMP/d7.$label.log" 2>&1 &
            pid=$!
            # 轮询就绪（**真等**）：进程活着就一直等到有响应，最长 60×0.5s = 30s；
            #   进程已退（如绑定失败）立即停等，交给下面的断言 / 端口重试处置。
            _pre=000
            for _i in $(seq 1 60); do
                kill -0 "$pid" 2>/dev/null || break
                _pre=$(_m116_http 2 "http://127.0.0.1:$port/nonexist")
                [ "$_pre" != "000" ] && break
                sleep 0.5
            done
            # 端口竞态重试（**只兜门自身的资源竞态**，判据一字不改）：
            #   进程起不来 **且** 日志指认绑定失败 ⇒ 换端口再来（至多 3 次）。
            if [ "$_pre" = "000" ] && ! kill -0 "$pid" 2>/dev/null \
               && grep -qE '绑定端口|Address already in use' "$TMP/d7.$label.log" 2>/dev/null; then
                kill $pid 2>/dev/null; wait $pid 2>/dev/null
                if [ "$attempt" -lt 3 ]; then
                    attempt=$((attempt+1))
                    echo "  ↻ $label：端口 $port 绑定失败 ⇒ 换端口重试（第 $attempt 次）"
                    continue
                fi
            fi
            code=$(_m116_http 5 "http://127.0.0.1:$port/bad" "$TMP/d7.body")
            body=$(head -c 60 "$TMP/d7.body" 2>/dev/null | tr -d '\n')
            alive=$(_m116_http 5 "http://127.0.0.1:$port/nonexist")
            kill $pid 2>/dev/null; wait $pid 2>/dev/null
            break
        done
        if [ "$code" != "500" ]; then bad "$label：出错 handler 返回 $code（应为 500）"; sed 's/^/       | /' "$TMP/d7.$label.log" 2>/dev/null | tail -6; return; fi
        if [ "$alive" = "000" ]; then bad "$label：服务在 handler 出错后死亡（应继续可用）"; return; fi
        ok "$label：500 + 服务存活（后续请求 $alive）· body=\"$body\""
    }
    ./tools/px build "$DIR/serve_err.px" > "$TMP/d7.build.log" 2>&1
    if [ -x "$DIR/build/serve_err" ]; then
        d7_case "VM轨" "$DIR/build/serve_err"
    else
        bad "VM 轨 serve_err 编译失败"; tail -5 "$TMP/d7.build.log"
    fi
    PX_BUILD_ENGINE=c ./tools/px build "$DIR/serve_err.px" > "$TMP/d7.buildc.log" 2>&1
    if [ -x "$DIR/build/serve_err" ]; then
        cp -f "$DIR/build/serve_err" "$TMP/serve_err_c"
        d7_case "C轨" "$TMP/serve_err_c"
    else
        bad "C 轨 serve_err 编译失败"; tail -5 "$TMP/d7.buildc.log"
    fi
fi

# ---------- ⑤ 门自检负控 ----------
echo "[5/6] 负控：断言值改错必须变红"
sed 's/== \["xy", "z"\]/== ["xy", "NOPE"]/' "$DIR/m116_semantics.px" > "$TMP/neg.px"
if ./tools/px run "$TMP/neg.px" > "$TMP/neg.log" 2>&1; then
    bad "负控未生效：故意错值仍然通过（门在自欺）"
else
    ok "负控生效（故意错值 → 红）"
fi

# ---------- ⑥ 反例自检：D4 修复前的"函数体被截断"必须不复现 ----------
echo "[6/6] D4 反例自检：多行字面量之后的本函数语句仍属本函数"
cat > "$TMP/d4tail.px" <<'PXEOF'
def f():
    var xs = [
        1,
        2
    ]
    xs.append(3)
    return xs
def g():
    return {"k": 1,
            "j": 2}
let a = f()
let b = g()
print("A=" + str(a) + " B=" + str(b["j"]))
PXEOF
./tools/px run "$TMP/d4tail.px" > "$TMP/d4tail.log" 2>&1
if grep -q 'A=\[1, 2, 3\] B=2' "$TMP/d4tail.log"; then
    ok "续行后函数体完整：$(tail -1 "$TMP/d4tail.log")"
else
    bad "续行破坏函数体（D4 回归）"; tail -5 "$TMP/d4tail.log"
fi

# ---------- 清理 ----------
rm -rf "$DIR/build" /tmp/build/d5 "$DIR/../build/m116_semantics" 2>/dev/null

echo ""
echo "M116_VERIFY: ${pass}P/${fail}F/${skip}S"
[ "$fail" -eq 0 ] || exit 1
exit 0
