#!/usr/bin/env bash
# ============================================================
# selfhost/gate_portscan.sh —— 门跑完「不许留监听」的机制（M430 · 缺陷 879）
# ------------------------------------------------------------
# 为什么有它（2026-11-21 19:5x 实测，修前复现）：
#   造一个「起了监听就退出」的假门，用 run_gates.sh 跑：
#       ✅ leaky_demo（1s）   汇总：失败 0 项        TSV: leaky_demo OK
#       ss: LISTEN 127.0.0.1:38769 users:(("python3",pid=855079))  PPID=1
#   ⇒ 门**判绿**、监听**留着**、进程**孤儿化**。这不是假门的问题，而是运行器
#     从来没有「门退出后看有没有留下监听」这一层。300 多道门里只有 M429 给自己
#     那一扇加了 G3 收尾判据 ⇒ 其余全是盲区。
#   代价（本月实测到的四起残留，没有一起是本门自己发现的）：
#     15 个 ./build/t 挂 7.5h 占 39711–39724 · 6 个 python3 占 18902 七天 ·
#     pxi 18096（m30_tls_pool 每跑一次漏一个）· ./build/srv 58717 跑了 12 天 10 小时
#
# 归属判据（关键设计 —— 为什么不能只比快照差集）：
#   分片档 8 个 run_gates.sh **并发**跑不同门 ⇒ 任一片的「前后快照差集」都会把
#   别片正在跑的门当成自己的泄漏。⇒ 主判据必须是**可归因**的：
#   运行器在跑门之前把 `PUXIAN_GATE_MARK=<门名>-<pid>-<随机>` 放进环境，
#   子进程（含 fork 出的 pxi / python / 编译产物）**继承**该环境；门退出后即使
#   被 init 收养（PPID=1），`/proc/<pid>/environ` 里仍然带着它 ⇒ 精确归因。
#   快照差集只作为**人工诊断**能力（`diff` 子命令），**不参与判绿**（本文件不做默认判断）。
#
# 已知盲区（如实写，不假装覆盖）：
#   · 门若用 `env -i` / `systemd-run` / `su` 起了进程 ⇒ marker 丢失 ⇒ 本机制看不见
#     （这类门目前 0 个；一旦出现，scan 会打印 `unattributed=N` 让人看见，不静默）
#   · 非 root 跑门时，别人 uid 的监听进程 environ 读不到 ⇒ 记 unattributed（响亮）
#   · 只扫 LISTEN/UNCONN（有 pid 的），不扫已建立连接 —— 判据是「端口是否被占住」
#
# 用法：
#   gate_portscan.sh scan --mark <M> --gate <名> [--grace 2] [--reap] [--quiet]
#        rc 0 干净 / 1 有泄漏（已按需回收） / 3 环境不成立（ss 不可用）
#   gate_portscan.sh snapshot [--diff <老快照>]     # 人工诊断用
#   gate_portscan.sh --self-test                    # 判据夹具（离线，不起进程）
# ============================================================
set -u

SS="${SS_BIN:-ss}"
EXEMPT="${GATE_PORTSCAN_EXEMPT:-$(cd "$(dirname "$0")" && pwd)/port_leak_exempt.tsv}"
EXEMPT_MAX="${GATE_PORTSCAN_EXEMPT_MAX:-8}"

# ── 纯函数：ss 行 → "proto<TAB>local<TAB>pids"（可自测）────────────
#   输入：ss -H -lntup 的原始行；输出：每条一个归一化记录；缺 pid 记 "-"
_parse_ss() {
    awk '
    NF >= 5 {
        proto = $1; local = $5
        # 只认 tcp/tcp6/udp/udp6（其余如 raw/packet 不属于「占端口」判据）
        if (proto !~ /^(tcp|tcp6|udp|udp6)$/) next
        pids = ""
        rest = $0
        while (match(rest, /pid=[0-9]+/)) {
            p = substr(rest, RSTART + 4, RLENGTH - 4)
            pids = (pids == "" ? p : pids "," p)
            rest = substr(rest, RSTART + RLENGTH)
        }
        if (pids == "") pids = "-"
        printf "%s\t%s\t%s\n", proto, local, pids
    }'
}

# ── 纯函数：environ 文件里是否有「整行等于 key=value」（可自测）──────
#   为什么不用 grep 子串：marker A 是 marker A1 的前缀 ⇒ substring 会误认归属
_mark_hit_file() {   # $1=environ 文件 $2=mark
    [ -r "$1" ] || return 2
    tr '\0' '\n' < "$1" 2>/dev/null | grep -qxF "PUXIAN_GATE_MARK=$2"
}
_mark_hit() { _mark_hit_file "/proc/$1/environ" "$2"; }

# ── 扫一遍：输出 "proto<TAB>local<TAB>pid<TAB>归属"（own/dirty/unknown）──
#   归属 own = environ 命中本次 marker；unknown = 有 pid 但 environ 读不到
_scan() {   # $1=mark
    local mark="$1" proto local pids p r own
    "$SS" -H -lntup 2>/dev/null | _parse_ss | while IFS=$'\t' read -r proto local pids; do
        [ "$pids" = "-" ] && continue          # 无主 socket（内核/其它命名空间）不归属
        own="dirty"
        for p in ${pids//,/ }; do
            _mark_hit "$p" "$mark"; r=$?
            if [ "$r" = 0 ]; then own="own"; break; fi
            if [ "$r" = 2 ]; then own="unknown"; fi
        done
        printf '%s\t%s\t%s\t%s\n' "$proto" "$local" "$pids" "$own"
    done
}

# ── 豁免账：只减不增 + 理由 ≥8 字 + 到期日必填（格式错 rc=2）──────
_check_exempt() {
    local n=0 bad=0 today line gate reason due
    today="$(date +%Y-%m-%d)"
    [ -f "$EXEMPT" ] || return 0
    while IFS=$'\t' read -r gate reason due; do
        case "$gate" in ''|'#'*) continue ;; esac
        n=$((n+1))
        [ "${#reason}" -ge 8 ] || { echo "❌ 豁免缺理由（≥8 字）：$gate"; bad=$((bad+1)); }
        case "$due" in
            [0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]) ;;
            *) echo "❌ 豁免缺到期日（YYYY-MM-DD）：$gate"; bad=$((bad+1)); continue ;;
        esac
        if [ "$due" \< "$today" ]; then echo "❌ 豁免已过期：$gate（$due）"; bad=$((bad+1)); fi
    done < "$EXEMPT"
    if [ "$n" -gt "$EXEMPT_MAX" ]; then
        echo "❌ 豁免账 $n 条 > 上限 $EXEMPT_MAX（只减不增：加豁免等于放掉判据）"
        bad=$((bad+1))
    fi
    [ "$bad" = 0 ]
}

_is_exempt() {   # $1=门名
    [ -f "$EXEMPT" ] || return 1
    awk -F'\t' -v g="$1" '$1==g && $1 !~ /^#/ {found=1} END{exit !found}' "$EXEMPT"
}

# ── 回收：只杀 environ 命中本次 marker 的 pid（pid 复用前二次核对）────
_reap() {   # $1=mark $2..=pids
    local mark="$1"; shift
    local p killed=0 i
    for p in "$@"; do
        case "$p" in ''|*[!0-9]*) continue ;; esac
        _mark_hit "$p" "$mark" || continue      # 归属再核一次（防 pid 复用误杀）
        kill -TERM "$p" 2>/dev/null && killed=$((killed+1))
    done
    [ "$killed" -gt 0 ] || return 0
    for i in 1 2 3 4 5 6; do
        local alive=0
        for p in "$@"; do
            _mark_hit "$p" "$mark" && kill -0 "$p" 2>/dev/null && alive=1
        done
        [ "$alive" = 0 ] && return 0
        sleep 0.5
    done
    for p in "$@"; do
        _mark_hit "$p" "$mark" && kill -KILL "$p" 2>/dev/null
    done
    return 0
}

cmd_scan() {
    local mark="" gate="" grace=2 reap=0 quiet=0
    while [ $# -gt 0 ]; do
        case "$1" in
            --mark)  shift; mark="${1:-}" ;;
            --gate)  shift; gate="${1:-}" ;;
            --grace) shift; grace="${1:-2}" ;;
            --reap)  reap=1 ;;
            --quiet) quiet=1 ;;
            *) echo "❌ scan 未知参数：$1" >&2; return 2 ;;
        esac
        shift
    done
    [ -n "$mark" ] || { echo "❌ scan 需要 --mark" >&2; return 2; }
    command -v "$SS" >/dev/null 2>&1 || { echo "SCAN-UNAVAIL: 没有 $SS" ; return 3; }
    _check_exempt || return 2
    if [ -n "$gate" ] && _is_exempt "$gate"; then
        [ "$quiet" = 1 ] || echo "EXEMPT $gate"
        return 0
    fi

    local deadline=$(( $(date +%s) + grace )) rows own leaked=0 unatt=0 unk=0
    while :; do
        rows="$(_scan "$mark")"
        leaked="$(printf '%s\n' "$rows" | awk -F'\t' '$4=="own"' | wc -l)"
        unk="$(printf '%s\n' "$rows" | awk -F'\t' '$4=="unknown"' | wc -l)"
        [ "$leaked" -gt 0 ] || break
        [ "$(date +%s)" -ge "$deadline" ] && break
        sleep 0.5      # 宽限：门自己没等优雅关闭时给它一点时间
    done

    if [ "$leaked" -gt 0 ]; then
        echo "PORT-LEAK gate=${gate:-?} listeners=$leaked marker=1"
        printf '%s\n' "$rows" | awk -F'\t' '$4=="own" {printf "   %s %s pid=%s\n", $1, $2, $3}'
        if [ "$reap" = 1 ]; then
            local allp
            allp="$(printf '%s\n' "$rows" | awk -F'\t' '$4=="own" {print $3}' | tr ',' '\n' | sort -u | tr '\n' ' ')"
            _reap "$mark" $allp
            local left
            left="$(_scan "$mark" | awk -F'\t' '$4=="own"' | wc -l)"
            [ "$left" = 0 ] && echo "   REAPED 全部回收（marker 归属）" \
                            || echo "   ⚠️ 仍剩 $left 个未回收 —— 需人工介入"
        fi
        return 1
    fi
    [ "$unk" -gt 0 ] && [ "$quiet" = 0 ] && echo "SCAN-UNATTRIBUTED n=$unk（environ 读不到：非本 uid 或已清环境）"
    return 0
}

cmd_snapshot() {
    local diff=""
    while [ $# -gt 0 ]; do
        case "$1" in --diff) shift; diff="${1:-}" ;; *) return 2 ;; esac
        shift
    done
    local now; now="$(mktemp)"
    "$SS" -H -lntup 2>/dev/null | _parse_ss | awk -F'\t' '{print $1":"$2}' | sort -u > "$now"
    if [ -n "$diff" ]; then
        echo "== 新增监听（差集，人工诊断用 —— **不参与判绿**：分片并发下会误归因）=="
        comm -13 <(sort -u "$diff") "$now" 2>/dev/null || cat "$now"
    else
        cat "$now"
    fi
    rm -f "$now"
    return 0
}

self_test() {
    local P=0 F=0 t
    ok(){ echo "  PASS $1"; P=$((P+1)); }
    bad(){ echo "  FAIL $1"; F=$((F+1)); }
    t="$(mktemp -d)"; trap 'rm -rf "$t"' RETURN

    local o
    o="$(printf '%s\n' 'tcp LISTEN 0 4096 127.0.0.1:33847 0.0.0.0:* users:(("v2ray",pid=941,fd=4)) ' | _parse_ss)"
    [ "$o" = "$(printf 'tcp\t127.0.0.1:33847\t941')" ] && ok "S1 tcp LISTEN 解析（含 pid）" || bad "S1 tcp 解析：$o"

    o="$(printf '%s\n' 'udp UNCONN 0 0 127.0.0.1:18902 0.0.0.0:* users:(("python3",pid=2908905,fd=5))' | _parse_ss)"
    [ "$o" = "$(printf 'udp\t127.0.0.1:18902\t2908905')" ] && ok "S2 udp UNCONN 解析（缺陷 879 的 18902 形状）" || bad "S2 udp 解析：$o"

    o="$(printf '%s\n' 'tcp LISTEN 0 128 [::1]:8080 [::]:* users:(("a",pid=7,fd=3),("b",pid=8,fd=4))' | _parse_ss)"
    [ "$o" = "$(printf 'tcp\t[::1]:8080\t7,8')" ] && ok "S3 ipv6 + 多 pid（共享 socket 两个持有者）" || bad "S3 ipv6/多 pid：$o"

    o="$(printf '%s\n' 'tcp LISTEN 0 128 0.0.0.0:9000 0.0.0.0:*' | _parse_ss)"
    [ "$o" = "$(printf 'tcp\t0.0.0.0:9000\t-')" ] && ok "S4 无 pid ⇒ 记 -（不归属，不许当本门泄漏）" || bad "S4 无 pid：$o"

    o="$(printf '%s\n' 'raw UNCONN 0 0 *:1 *:* users:(("x",pid=5,fd=1))' 'tcp LISTEN 0 1 127.0.0.1:2 0.0.0.0:* users:(("y",pid=6,fd=2))' | _parse_ss | wc -l)"
    [ "$o" = 1 ] && ok "S5 只认 tcp/udp（raw 不计入占端口判据）" || bad "S5 协议过滤：$o 行"

    printf 'A=1\0PUXIAN_GATE_MARK=m430-g-99\0B=2\0' > "$t/e1"
    printf 'PUXIAN_GATE_MARK=m430-g-990\0' > "$t/e2"
    _mark_hit_file "$t/e1" "m430-g-99" && ok "S6 environ 精确命中 marker" || bad "S6 marker 命中失败"
    _mark_hit_file "$t/e2" "m430-g-99" && bad "S7 前缀被误认归属（990 命中 99）" || ok "S7 前缀不命中（整行相等语义）"
    _mark_hit_file "$t/不存在" "m430-g-99"; [ "$?" = 2 ] && ok "S8 environ 读不到 ⇒ rc=2（unknown 而非 dirty）" || bad "S8 读不到时返回码错"

    cat > "$t/ex.tsv" <<'EOS'
leaky_shim	M427 时期的最小复现脚本起了探测服务没 trap（已修）	2026-12-31
EOS
    # ⚠️ 夹具必须覆盖 **EXEMPT 本身**（脚本启动时的路径变量），
    #    而不是 env 名 GATE_PORTSCAN_EXEMPT —— 后者只在脚本顶层读一次，
    #    函数调用前缀改它**无效**（第一版 S10–S13 四条就是这么假的：读的是真表，
    #    所以「坏表」全都通过 ⇒ 判据没牙。修法：EXEMPT=<文件> _check_exempt）。
    EXEMPT="$t/ex.tsv" EXEMPT_MAX=8 _check_exempt >/dev/null 2>&1 \
        && ok "S9 豁免表合法格式通过" || bad "S9 合法豁免被拒"
    printf 'g1\t短\t2026-12-31\n' > "$t/ex2.tsv"
    EXEMPT="$t/ex2.tsv" _check_exempt >/dev/null 2>&1 && bad "S10 理由过短被放过" || ok "S10 理由 <8 字判错"
    printf 'g1\t理由够长的理由\t2020-01-01\n' > "$t/ex3.tsv"
    EXEMPT="$t/ex3.tsv" _check_exempt >/dev/null 2>&1 && bad "S11 过期豁免被放过" || ok "S11 过期豁免判错"
    printf 'g1\t理由够长的理由\n' > "$t/ex4.tsv"
    EXEMPT="$t/ex4.tsv" _check_exempt >/dev/null 2>&1 && bad "S12 缺到期日被放过" || ok "S12 缺到期日判错"

    # S13 上限：超过 EXEMPT_MAX ⇒ 判错（只减不增）
    { echo "# 头"; i=0; while [ $i -lt 3 ]; do printf 'g%d\t理由够长的理由够长\t2026-12-31\n' "$i"; i=$((i+1)); done; } > "$t/ex5.tsv"
    EXEMPT="$t/ex5.tsv" EXEMPT_MAX=2 _check_exempt >/dev/null 2>&1 \
        && bad "S13 超上限被放过" || ok "S13 豁免超上限判错"

    echo "  --- 自测：PASS $P / FAIL $F ---"
    [ "$F" = 0 ]
}

case "${1:-}" in
    scan)        shift; cmd_scan "$@" ;;
    snapshot)    shift; cmd_snapshot "$@" ;;
    --self-test) self_test ;;
    *) sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; exit 2 ;;
esac
