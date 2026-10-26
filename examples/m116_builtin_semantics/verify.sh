#!/usr/bin/env bash
# ============================================================
# M116 验证（qg-issue 71 D2/D4/D5/D6/D7）
# ------------------------------------------------------------
# 判据（每条都直接比可观测事实，不靠"应该没问题"）：
#   ① D4/D6 + D2 守卫写法：双轨（px run / px build）都必须全 PASS
#   ② D2 负控：**两条轨**跑 dict_missing_key.px 都必须非零退出、stderr 含 R1008
#      且出错点之后的 "after" 不得出现（两轨同口径 = 修复的核心判据）
#   ③ D6：mkdir mode 按 umask 折算的绝对权限位断言（在 ① 内）
#   ④ D5：dns_txt UDP 截断（TC=1）→ TCP 回退 —— 门内**自建 DNS 桩**（零外网）：
#      桩档 = 判红（UDP 面 TC=1 + 诱饵、TCP 面 3 条长 TXT；回退由桩侧计数直接观测）；
#      桩档不可用（非 root 且无 sudo -n）时才走真域名，且先做 UDP 53 + TCP 53 **双通道探针**，
#      任一不通 ⇒ SKIP 并响亮计数，**永不判红**。缺陷 584：探针通道 ≠ 被测通道。
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

# ---------- ③ D5：dns_txt UDP 截断 → TCP 回退（本地桩优先 · 缺陷 584） ----------
echo "[3/6] D5：dns_txt UDP 截断 → TCP 回退（本地 DNS 桩 · 零外网）"
# 缺陷 584 的教训：老门只用 getaddrinfo（**UDP 侧**）探网，而被测通道是 **TCP 53** ⇒
#   探针过、通道不通 ⇒ 把「环境不可用」判成「产品红」（CI run 40308948055，连带 31 个 step skipped）。
# 现在分层：
#   【桩档 · 判红】门内自建 DNS 桩（$DIR/dns_stub.c）：
#       UDP 面置 TC=1 且塞 1 条诱饵 "decoy"，TCP 面给 3 条长 TXT（RDATA 合计 >512 ⇒ 真会截断）
#       ⇒ 语义由门自己造，零外网、确定性。需要 root（绑 53）+ unshare -mn（网络/挂载命名空间
#       隔离，绝不碰宿主 /etc 与真实网卡）。CI 用 runner 的 sudo -n，本地全量门以 root 跑。
#   【外网档 · 观察项】桩档跑不了（非 root 且无 sudo -n）时才走真域名；先做 **UDP 53 + TCP 53
#       双通道探针**，任一不通 ⇒ SKIP 并**响亮计数**，永不判红 —— 判据不判它无法观测的环境。
STUB_SRC="$DIR/dns_stub.c"
STUB_BIN="$TMP/dns_stub"
D5_CLIENT_SRC="$TMP/d5.px"
D5_CLIENT="$TMP/build/d5"
D5_OK=0

# 桩在 TCP 面给出的真值 / UDP 面的诱饵（判据侧真值由桩自己吐，避免两处手抄 600 字节）
d5_sec()   { awk -v c="$1" '$0=="CASE "c{f=1;next} /^CASE /{f=0} f' "$2"; }
d5_n()     { d5_sec "$1" "$2" | sed -n 's/^n=//p' | head -1; }
d5_recs()  { d5_sec "$1" "$2" | grep '^R' || true; }

# 判据本体。**必须能被负控打红**（NC2 拿破损桩验证这一点，否则判据在自欺）
d5_judge() {   # $1=客户端输出 ⇒ 0=符合
    local out="$1"
    grep -q '^ERR ' "$out" && return 1
    [ "$(d5_n trunc.dnsstub.test "$out")" = "3" ] || return 1
    diff -q <(d5_recs trunc.dnsstub.test "$out") <("$STUB_BIN" expected) >/dev/null || return 1
    [ "$(d5_n plain.dnsstub.test "$out")" = "1" ] || return 1
    [ "$(d5_sec plain.dnsstub.test "$out" | grep -m1 '^R0=')" = "R0=plain-udp" ] || return 1
    [ "$(d5_n nx.dnsstub.test "$out")" = "0" ] || return 1
    [ "$(d5_n empty.dnsstub.test "$out")" = "0" ] || return 1
    return 0
}

# 「回退真的发生了」的直接观测：桩侧计数（而不是靠返回值看着对推断）
d5_hits() {    # $1=hits 日志
    local h="$1"
    [ "$(grep -c '^udp trunc\.dnsstub\.test$' "$h" 2>/dev/null || true)" = "1" ] || return 1
    [ "$(grep -c '^tcp trunc\.dnsstub\.test$' "$h" 2>/dev/null || true)" -ge 1 ] || return 1
    [ "$(grep -c '^udp plain\.dnsstub\.test$' "$h" 2>/dev/null || true)" = "1" ] || return 1
    [ "$(grep -c '^tcp plain\.dnsstub\.test$' "$h" 2>/dev/null || true)" = "0" ] || return 1
    return 0
}

# 桩档的前提：root（绑 53）+ ip（拉起 lo）+ unshare -mn（隔离）
_M116_SUDO=""
if [ "$(id -u)" -ne 0 ] && command -v sudo >/dev/null 2>&1 && sudo -n true 2>/dev/null; then
    _M116_SUDO="sudo -n"
fi
# ip 的绝对路径：runner 普通用户的 PATH 里常常没有 /usr/sbin ⇒ 只查 PATH 会把桩档
#   **静默降级**成外网观察档（那就又回到「看着绿、实则零观测」）。这里显式兜底。
_M116_IP="$(command -v ip 2>/dev/null || true)"
for _c in /usr/sbin/ip /sbin/ip /usr/bin/ip /bin/ip; do
    [ -n "$_M116_IP" ] && break
    [ -x "$_c" ] && _M116_IP="$_c"
done
_m116_dns_ns_ok() {
    [ -n "$_M116_IP" ] || return 1
    command -v unshare >/dev/null 2>&1 || return 1
    [ "$(id -u)" -eq 0 ] || [ -n "$_M116_SUDO" ] || return 1
    $_M116_SUDO unshare -mn true 2>/dev/null || return 1
    return 0
}

cc -O2 -o "$STUB_BIN" "$STUB_SRC" > "$TMP/stub.cc.log" 2>&1
if [ ! -x "$STUB_BIN" ]; then
    bad "DNS 桩编译失败（cc $STUB_SRC）—— 桩档是 D5 的唯一判据来源"
    sed 's/^/       | /' "$TMP/stub.cc.log" | tail -8
else
    cat > "$D5_CLIENT_SRC" <<'PXEOF'
let names = ["trunc.dnsstub.test", "plain.dnsstub.test", "nx.dnsstub.test", "empty.dnsstub.test"]
for n in names:
    print("CASE " + n)
    let r = dns_txt(n)
    if type(r) == "result":
        print("ERR " + str(r))
    else:
        print("n=" + str(len(r)))
        var i = 0
        for x in r:
            print("R" + str(i) + "=" + x)
            i += 1
PXEOF
    ./tools/px build "$D5_CLIENT_SRC" > "$TMP/d5.build.log" 2>&1
    if [ ! -x "$D5_CLIENT" ]; then
        bad "D5 客户端编译失败"; sed 's/^/       | /' "$TMP/d5.build.log" | tail -6
    elif ! _m116_dns_ns_ok; then
        # ── 外网档：观察项，永不判红（判据不判它无法观测的环境）──────────────
        _m116_dns_ns="$(awk '/^[[:space:]]*nameserver/{print $2; exit}' /etc/resolv.conf 2>/dev/null)"
        if [ -z "$_m116_dns_ns" ]; then
            skips "[D5-SKIP-LOUD] 桩档不可用（非 root 且无 sudo -n）且 /etc/resolv.conf 无 nameserver ⇒ D5 本轮**零观测**"
        else
            _m116_pr="$("$STUB_BIN" probe "$_m116_dns_ns" 53 2>&1)"; _m116_prc=$?
            if [ $_m116_prc -ne 0 ]; then
                skips "[D5-SKIP-LOUD] 外网档通道不可达（$_m116_dns_ns:53）：$_m116_pr ⇒ D5 本轮**零观测**（缺陷 584：这正是以前会被误判成产品红的情形）"
            else
                cat > "$TMP/d5ext.px" <<'PXEOF'
let names = ["google.com", "cloudflare.com"]
var bad = 0
for n in names:
    let r = dns_txt(n)
    if type(r) == "result":
        print("ERR " + n + ": " + str(r))
        bad += 1
    else:
        print("OK " + n + " n=" + str(len(r)))
PXEOF
                ./tools/px build "$TMP/d5ext.px" > "$TMP/d5ext.build.log" 2>&1
                "$TMP/build/d5ext" > "$TMP/d5ext.out" 2>&1; _m116_erc=$?
                skips "[D5-SKIP-LOUD] 桩档不可用 ⇒ 仅外网观察（双通道探针已过：$_m116_pr）：$(tr '\n' ' ' < "$TMP/d5ext.out") rc=$_m116_erc（观察项不影响结论）"
            fi
        fi
        D5_OK=0
    else
        # ── 桩档：判红 ────────────────────────────────────────────────
        cat > "$TMP/d5_run.sh" <<'EOS'
#!/bin/bash
# 在 unshare -mn 内跑：只碰本命名空间的 lo 与 /etc/resolv.conf（不碰宿主）
set -u
mode="$1"; out="$2"; hits="$3"
"$IP_BIN" link set lo up 2>/dev/null || { echo "NS_FAIL=lo" > "$out"; exit 90; }
mount --make-rprivate / 2>/dev/null || true
printf 'nameserver 127.0.0.1\n' > "$RESOLV_TMP" 2>/dev/null || { echo "NS_FAIL=resolv-write" > "$out"; exit 91; }
mount --bind "$RESOLV_TMP" /etc/resolv.conf 2>/dev/null || { echo "NS_FAIL=resolv-bind" > "$out"; exit 92; }
grep -q 'nameserver 127.0.0.1' /etc/resolv.conf 2>/dev/null || { echo "NS_FAIL=resolv-visible" > "$out"; exit 93; }
: > "$hits"
"$STUB_BIN" serve 53 "$hits" "$mode" 2> "$STUB_ERR" &
sp=$!
for _i in $(seq 1 50); do grep -q STUB_READY "$STUB_ERR" 2>/dev/null && break; sleep 0.1; done
"$D5_CLIENT" > "$out" 2>&1
rc=$?
kill "$sp" 2>/dev/null; wait "$sp" 2>/dev/null
exit $rc
EOS
        _m116_ns_run() {   # $1=mode $2=输出 $3=hits
            $_M116_SUDO env STUB_BIN="$STUB_BIN" D5_CLIENT="$D5_CLIENT" IP_BIN="$_M116_IP" \
                RESOLV_TMP="$TMP/d5.resolv" STUB_ERR="$TMP/d5.stub.err" \
                unshare -mn bash "$TMP/d5_run.sh" "$1" "$2" "$3"
        }
        _m116_ns_run normal "$TMP/d5.normal.out" "$TMP/d5.hits"; _m116_rc=$?
        case $_m116_rc in
            90|91|92|93)
                bad "桩档命名空间构造失败（$(head -1 "$TMP/d5.normal.out" 2>/dev/null)）—— 桩档是必红判据，不因构造失败判绿"
                D5_OK=0 ;;
            *)
                if d5_judge "$TMP/d5.normal.out"; then
                    ok "桩档：TC=1（UDP）⇒ 回退 TCP ⇒ 3 条长 TXT 逐字节等于桩的 TCP 面（多段 character-string 已合并）"
                    D5_OK=1
                else
                    bad "桩档：dns_txt 在「UDP 截断 + TCP 可达」下结果不符（产品缺陷判据）"
                    sed 's/^/       | /' "$TMP/d5.normal.out" | head -12
                    D5_OK=0
                fi ;;
        esac
        if [ "$D5_OK" = "1" ]; then
            if d5_hits "$TMP/d5.hits"; then
                ok "回退真的发生（桩侧直读）：udp trunc ×1 → tcp trunc ×1；plain 只走 UDP（非无脑 TCP）"
            else
                bad "回退未按预期发生（桩侧计数）：$(tr '\n' ' ' < "$TMP/d5.hits")"
            fi
            # NC1：判别子存在 —— 若诱饵与真值相同，「有没有回退」在返回值上不可分 ⇒ 判据无牙
            if [ "$("$STUB_BIN" decoy)" != "$("$STUB_BIN" expected | head -1)" ] && \
               [ "$("$STUB_BIN" decoy | grep -c '^R')" != "$("$STUB_BIN" expected | grep -c '^R')" ]; then
                ok "NC1 判别子存在：UDP 诱饵（1 条 decoy）≠ TCP 真值（3 条长记录）⇒ 不回退者必然对不上"
            else
                bad "NC1 判别子失效：诱饵与真值不可分 ⇒ 判据没有牙"
            fi
            # NC2：判据负控 —— TCP 面故意破损（accept 后立即关），判据**必须**红
            _m116_ns_run tcp-broken "$TMP/d5.broken.out" "$TMP/d5.broken.hits" >/dev/null 2>&1
            if d5_judge "$TMP/d5.broken.out"; then
                bad "NC2 判据自欺：TCP 面破损（accept 后立即关）时判据竟仍判绿"
            else
                ok "NC2 判据有牙：TCP 面破损 ⇒ 客户端 $(grep -m1 '^ERR ' "$TMP/d5.broken.out" | cut -c1-60) ⇒ 判据判红"
            fi
            # NC3：症状定性 —— 通道不可达的可观测症状就是 CI 红原文那句 tcp connect() failed
            _m116_ns_run udp-only "$TMP/d5.noudp.out" "$TMP/d5.noudp.hits" >/dev/null 2>&1
            if grep -q 'tcp connect() failed' "$TMP/d5.noudp.out"; then
                ok "NC3 症状定性：TCP 53 不可达 ⇒ 「dns: tcp connect() failed」（与 CI run 40308948055 红原文同症 ⇒ 该文案属环境类，非产品错）"
            else
                bad "NC3 症状不符：TCP 不可达时未见 tcp connect() failed：$(head -3 "$TMP/d5.noudp.out" | tr '\n' ' ')"
            fi
        fi
    fi
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
