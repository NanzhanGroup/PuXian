#!/usr/bin/env bash
# notify_ack.sh —— 「已通知 ≠ 已被读到」判据（M413 缺陷 823 · M414 缺陷 824 修口径）
#
# 病灶（2026-11-17 M412 轮次暴露）：
#   dy-notify.sh 的 p1/esc 写完台账就调 ws-core -task-park「叫醒东月」，但 buswake 有
#   30 分钟空闲门禁（最近 30m 无 token 业务请求才唤醒）。用户在场连续对话时门禁永不满足
#   ⇒ 台账写着 ESCALATED（「已叫过」），实际没人被叫醒。历史证据：inbox.log 近 15 天
#   82 次 ESCALATE，能配对到后续了结的 12 条里有 11 条是 EXPIRE（12h TTL 到期自清）。
#
# M414 修的是什么（2026-11-17 21:37 那条 p0 误报，用户被假警报叫醒）：
#   M413 判据只看「账龄 ≥90min ∧ 末行 ESCALATED ∧ 用户不在场」就判 BLIND，
#   于是「发布链还在跑、park 探测条件本来就是假」也被当成「叫不醒」。
#   而 BLIND 的语义是「**该醒**而没醒」，「该醒」的权威定义就是 park 探测条件本身。
#   ⇒ 本版先求 probe，再看在途发布单元（二级兜底）：
#        probe=false            ⇒ WAITING（该醒的前提不成立）
#        probe=true  + 用户在场   ⇒ wait-present（门禁设计如此）
#        probe=true  + 用户不在场 ⇒ BLIND
#        probe=true  + 在场未知   ⇒ wait-unknown（不猜）
#        无 park 记录 + 有在途发布单元 ⇒ WAITING（等工序 ≠ 叫不醒）
#        无 park 记录 + 无在途单元     ⇒ wait-unknown（不猜，不报 p0）
#   另加数值兜底：blind_min 动态下界 = max(NA_BLIND_MIN, NA_WAIT_FLOOR + 最长在途发布单元时长)
#
# 输出固定含 nscan=<台账条数>，且把四类等待单独计数（不许并进 ok，也不许并进 blind）。
# rc：0=无 BLIND · 1=有 BLIND · 3=前提不成立（台账目录不可用 / 参数非法，不许假绿）
#
# 环境变量（自测注入用）：
#   NA_PEND_DIR    台账目录（默认 /var/log/dy-ops/pending）
#   NA_DB          token_cache db（默认 /data/app/ws/data/token_cache.db）
#   NA_PARKWHEN    park 条件文件（默认 <WS_PATH>/data/handoff/.parkwhen.json）
#   NA_UNITS_FILE  在途单元清单（每行「单元名<TAB>已运行分钟」；给了就不查 systemctl）
#   NA_UNIT_RE     认作「发布机械段」的单元名正则
#   NA_NOW · NA_BLIND_MIN(90) · NA_WAIT_FLOOR(30) · NA_IDLE_MIN(30) · NA_VERBOSE=1

set -uo pipefail

WS=${WS_PATH:-/data/app/ws}
PEND_DIR=${NA_PEND_DIR:-/var/log/dy-ops/pending}
DB=${NA_DB:-$WS/data/token_cache.db}
PARKWHEN=${NA_PARKWHEN:-$WS/data/handoff/.parkwhen.json}
UNITS_FILE=${NA_UNITS_FILE:-}
UNIT_RE=${NA_UNIT_RE:-^m[0-9]+-(chain|release|gate|postupgrade|red-guard)(\.service)?$}
BLIND_MIN=${NA_BLIND_MIN:-90}
WAIT_FLOOR=${NA_WAIT_FLOOR:-30}
IDLE_MIN=${NA_IDLE_MIN:-30}
VERBOSE=${NA_VERBOSE:-0}
NOW=${NA_NOW:-}
[ -z "$NOW" ] && NOW=$(date +%s)
case "$NOW" in ''|*[!0-9]*) echo "PREMISE-FAIL NA_NOW 非数字：$NOW"; exit 3;; esac
for v in "$BLIND_MIN" "$WAIT_FLOOR" "$IDLE_MIN"; do
    case "$v" in ''|*[!0-9]*) echo "PREMISE-FAIL 阈值非数字：$v"; exit 3;; esac
done

# ── 前提：台账目录必须是**目录**。不存在 ⇒ 不当「0 条」，报 PREMISE-FAIL ──
if [ ! -d "$PEND_DIR" ]; then
    echo "PREMISE-FAIL pend_dir=$PEND_DIR 不是目录（不许把它当成「没有待办」）"
    exit 3
fi

# ── 用户在场判据：token_cache 最近写入 ──
tok_last=0; tok_src="none"
if [ -f "$DB" ]; then
    if command -v sqlite3 >/dev/null 2>&1; then
        tok_last=$(sqlite3 "$DB" "select coalesce(max(updated_at),0) from token_cache;" 2>/dev/null || echo 0)
        tok_src="sqlite"
    else
        tok_src="no-sqlite3"
    fi
fi
case "$tok_last" in ''|*[!0-9]*) tok_last=0;; esac
tok_age_min=$(( (NOW - tok_last) / 60 ))
if [ "$tok_last" -le 0 ]; then present="unknown"
elif [ "$tok_age_min" -lt "$IDLE_MIN" ]; then present="yes"
else present="no"; fi

# ── 在途发布机械段（二级兜底 + 动态下界）──
#   输出每行「unit<TAB>minutes」。NA_UNITS_FILE 给了就用它（自测确定性），否则查 systemctl。
list_units() {
    if [ -n "$UNITS_FILE" ]; then
        # 用 -e 而不是 -f：自测会传 /dev/null（空清单），它是字符设备不是普通文件
        [ -e "$UNITS_FILE" ] || { echo "PREMISE-FAIL NA_UNITS_FILE 不存在：$UNITS_FILE" >&2; return 2; }
        cat "$UNITS_FILE" 2>/dev/null
        return 0
    fi
    command -v systemctl >/dev/null 2>&1 || return 0
    local now_mono u mon
    now_mono=$(awk '{printf "%d", $1*1000000}' /proc/uptime 2>/dev/null || echo 0)
    systemctl list-units --state=active --no-legend --plain --no-pager 2>/dev/null |
    awk '{print $1}' | while read -r u; do
        [[ "$u" =~ $UNIT_RE ]] || continue
        mon=$(systemctl show -p ActiveEnterTimestampMonotonic --value "$u" 2>/dev/null)
        case "$mon" in ''|*[!0-9]*) mon=0;; esac
        if [ "$mon" -gt 0 ] && [ "$now_mono" -gt "$mon" ]; then
            printf '%s\t%d\n' "$u" $(( (now_mono - mon) / 60000000 ))
        else
            printf '%s\t%s\n' "$u" "?"
        fi
    done
}

units_out=$(list_units 2>/dev/null) || { echo "PREMISE-FAIL 在途单元清单不可用"; exit 3; }
unit_max=0; unit_max_name=""; unit_n=0
while IFS=$'\t' read -r un um; do
    [ -n "${un:-}" ] || continue
    unit_n=$((unit_n+1))
    case "$um" in ''|*[!0-9]*) continue;; esac
    if [ "$um" -gt "$unit_max" ]; then unit_max=$um; unit_max_name=$un; fi
done <<< "$units_out"

# 动态下界：等工序的时间不该算进「叫不醒」的账龄
eff_min=$BLIND_MIN
if [ $(( WAIT_FLOOR + unit_max )) -gt "$eff_min" ]; then eff_min=$(( WAIT_FLOOR + unit_max )); fi

# ── park 探测条件的权威读取 ──
#   打印 cmds（\x1f 分隔），rc：0=有记录 · 1=无该任务记录 · 2=文件不可解析
probe_cmds() { # probe_cmds <task-key>
    [ -f "$PARKWHEN" ] || return 1
    command -v python3 >/dev/null 2>&1 || return 2
    python3 - "$PARKWHEN" "$1" 2>/dev/null <<'PY'
import json, sys
p, key = sys.argv[1], sys.argv[2]
try:
    d = json.load(open(p))
except Exception:
    sys.exit(2)
if not isinstance(d, dict):
    sys.exit(2)
e = d.get(key)
if not isinstance(e, dict):
    sys.exit(1)
cmds = e.get('cmds')
if not isinstance(cmds, list) or not cmds:
    cmds = [e['cmd']] if e.get('cmd') else []
cmds = [c for c in cmds if isinstance(c, str) and c.strip()]
if not cmds:
    sys.exit(1)
print('\x1f'.join(cmds))
PY
}

# probe 求值：全部 cmds 退出 0 ⇒ true（与 ws-core park 语义一致）
probe_eval() { # probe_eval <key> -> 打印 "true|false|absent|unparsable<TAB>cmd摘要"
    local key=$1 raw rc c all=1 shown=""
    raw=$(probe_cmds "dy-ops-$key"); rc=$?
    [ "$rc" = 1 ] && { raw=$(probe_cmds "$key"); rc=$?; }
    case "$rc" in
        1) printf 'absent\t\n'; return 0;;
        2) printf 'unparsable\tparkwhen=%s\n' "$PARKWHEN"; return 0;;
    esac
    shown=$(printf '%s' "$raw" | tr '\037' ' ' | cut -c1-90)
    local IFS=$'\037'
    for c in $raw; do
        timeout 10 bash -c "$c" >/dev/null 2>&1 || all=0
    done
    unset IFS
    printf '%s\t%s\n' "$([ "$all" = 1 ] && echo true || echo false)" "$shown"
}

nscan=0; nblind=0; npresent=0; nunknown=0; nprobe=0; nunitexc=0; nok=0
blind_list=""; waiting_list=""

for f in "$PEND_DIR"/*; do
    [ -e "$f" ] || continue
    nscan=$((nscan+1))
    key=$(basename "$f")
    first=$(head -1 "$f" 2>/dev/null | cut -f1)
    case "$first" in ''|*[!0-9]*) first=0;; esac
    age=$(( (NOW - first) / 60 ))
    lastmark=$(tail -1 "$f" 2>/dev/null | cut -f1)
    title=$(head -1 "$f" 2>/dev/null | cut -f2-)

    if [ "$lastmark" != "ESCALATED" ]; then
        nok=$((nok+1))
        [ "$VERBOSE" = "1" ] && printf 'ok            %-40s age=%smin last=%s\n' "$key" "$age" "${lastmark:-none}"
        continue
    fi
    if [ "$age" -lt "$BLIND_MIN" ]; then
        nok=$((nok+1))
        [ "$VERBOSE" = "1" ] && printf 'ok            %-40s age=%smin last=ESCALATED(<%smin)\n' "$key" "$age" "$BLIND_MIN"
        continue
    fi

    IFS=$'\t' read -r pv pcmd <<< "$(probe_eval "$key")"
    case "$pv" in
        false)
            nprobe=$((nprobe+1))
            waiting_list="$waiting_list $key(probe=false)"
            printf 'WAITING-PROBE %-40s age=%smin probe=false cmd=%s\n' "$key" "$age" "$pcmd"
            ;;
        true)
            case "$present" in
                yes)
                    npresent=$((npresent+1))
                    printf 'wait-present  %-40s age=%smin user_idle=%smin(<%s) title=%s\n' \
                        "$key" "$age" "$tok_age_min" "$IDLE_MIN" "$title"
                    ;;
                unknown)
                    nunknown=$((nunknown+1))
                    printf 'wait-unknown  %-40s age=%smin（在场判据不可用：db=%s src=%s）title=%s\n' \
                        "$key" "$age" "$DB" "$tok_src" "$title"
                    ;;
                *)
                    if [ "$age" -lt "$eff_min" ]; then
                        nunitexc=$((nunitexc+1))
                        waiting_list="$waiting_list $key(unit=$unit_max_name)"
                        printf 'WAITING-UNIT  %-40s age=%smin unit=%s run=%smin threshold=%smin\n' \
                            "$key" "$age" "${unit_max_name:-?}" "$unit_max" "$eff_min"
                    else
                        nblind=$((nblind+1))
                        blind_list="$blind_list $key(${age}min)"
                        printf 'BLIND         %-40s age=%smin user_idle=%smin probe=true title=%s\n' \
                            "$key" "$age" "$tok_age_min" "$title"
                    fi
                    ;;
            esac
            ;;
        absent|unparsable)
            if [ "$unit_n" -gt 0 ]; then
                nunitexc=$((nunitexc+1))
                waiting_list="$waiting_list $key(unit=$unit_max_name,probe=$pv)"
                printf 'WAITING-UNIT  %-40s age=%smin unit=%s run=%smin probe=%s title=%s\n' \
                    "$key" "$age" "$unit_max_name" "$unit_max" "$pv" "$title"
            else
                nunknown=$((nunknown+1))
                printf 'wait-unknown  %-40s age=%smin（无 park 记录 / 不可解析：%s）title=%s\n' \
                    "$key" "$age" "$pv" "$title"
            fi
            ;;
    esac
done

# ── 扫描面零值守卫（M410 R2 口径）：零值必须落在「已证明目录可用之后」──
if [ "$nscan" -eq 0 ]; then
    if [ ! -d "$PEND_DIR" ] || [ ! -r "$PEND_DIR" ]; then
        echo "PREMISE-FAIL nscan=0 但台账目录不可用（dir=$PEND_DIR）"
        exit 3
    fi
    echo "SCAN-GUARD nscan=0 且台账目录存在可读 ⇒ 真空（不是扫描盲区）"
fi

printf 'NOTIFY-ACK nscan=%s blind=%s wait_present=%s wait_unknown=%s wait_probe=%s wait_unit=%s ok=%s blind_min=%s base_min=%s unit_n=%s unit_max=%s unit=%s idle_min=%s user_present=%s tok_age_min=%s\n' \
    "$nscan" "$nblind" "$npresent" "$nunknown" "$nprobe" "$nunitexc" "$nok" \
    "$eff_min" "$BLIND_MIN" "$unit_n" "$unit_max" "${unit_max_name:-none}" "$IDLE_MIN" "$present" "$tok_age_min"

if [ "$nblind" -gt 0 ]; then
    echo "BLIND-KEYS$blind_list"
    exit 1
fi
exit 0
