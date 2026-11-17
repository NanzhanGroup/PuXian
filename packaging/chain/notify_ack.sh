#!/usr/bin/env bash
# notify_ack.sh —— 「已通知 ≠ 已被读到」判据（M413 · 缺陷 823）
#
# 病灶（2026-11-17 M412 轮次暴露）：
#   dy-notify.sh 的 p1/esc 写完台账就调 ws-core -task-park「叫醒东月」，但 buswake 有
#   30 分钟空闲门禁（最近 30m 无 token 业务请求才唤醒）。用户在场连续对话时门禁永不满足
#   ⇒ 台账写着 ESCALATED（「已叫过」），实际没人被叫醒。历史证据：inbox.log 近 15 天
#   82 次 ESCALATE，能配对到后续了结的 12 条里有 11 条是 EXPIRE（12h TTL 到期自清），
#   不是被处理掉的。M408 三次红也全是「用户来问才知道」。
#
# 本脚本只做一件事：把「已升级 + 超时未了结 + 用户不在场」的台账挑出来，给看门狗/门用。
#   为什么不直接报 p1：BLIND 的定义就是「p1 自驱没生效」，再 p1 是同一循环（自我指涉）。
#
# 判据（三条同时成立 = BLIND）：
#   ① 台账首行 epoch 距今 ≥ BLIND_MIN（默认 90 分钟）
#   ② 末行标记是 ESCALATED（= park 成功过，即「已通知」）
#   ③ 用户不在场：token_cache 最近一次写入距今 ≥ IDLE_MIN（默认 30 分钟，与 buswake 门禁同口径）
#
# 分类（全部打印，不只打红的）：
#   blind        ①+②+③ ⇒ 自驱该醒却没醒（rc=1）
#   wait-present ①+②，但用户在场 ⇒ 门禁是设计如此，不打扰（0）
#   wait-unknown ①+②，但在场判据拿不到（无 DB / 表空）⇒ 不算事故也不算绿（0，但计数单列）
#   ok           其余（未到点 / 未升级）
#
# 输出固定含 nscan=<台账条数>（M410 判据门 R1 要求：判绿必须打印扫描面）。
# rc：0=无 BLIND · 1=有 BLIND · 3=前提不成立（台账目录不存在，不许假绿）
#
# 环境变量（自测注入用）：
#   NA_PEND_DIR  台账目录（默认 /var/log/dy-ops/pending）
#   NA_DB        token_cache db（默认 /data/app/ws/data/token_cache.db）
#   NA_NOW       当前 epoch（默认 date +%s）
#   NA_BLIND_MIN 默认 90 · NA_IDLE_MIN 默认 30 · NA_VERBOSE=1 打印每条明细

set -uo pipefail

PEND_DIR=${NA_PEND_DIR:-/var/log/dy-ops/pending}
DB=${NA_DB:-/data/app/ws/data/token_cache.db}
BLIND_MIN=${NA_BLIND_MIN:-90}
IDLE_MIN=${NA_IDLE_MIN:-30}
VERBOSE=${NA_VERBOSE:-0}
NOW=${NA_NOW:-}
[ -z "$NOW" ] && NOW=$(date +%s)
case "$NOW" in ''|*[!0-9]*) echo "PREMISE-FAIL NA_NOW 非数字：$NOW"; exit 3;; esac

# ── 前提：台账目录必须是**目录**。不存在 ⇒ 不当「0 条」，报 PREMISE-FAIL ──
if [ ! -d "$PEND_DIR" ]; then
    echo "PREMISE-FAIL pend_dir=$PEND_DIR 不是目录（不许把它当成「没有待办」）"
    exit 3
fi

# ── 用户在场判据：token_cache 最近写入 ──
tok_last=0
tok_src="none"
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
if [ "$tok_last" -le 0 ]; then
    present="unknown"
else
    if [ "$tok_age_min" -lt "$IDLE_MIN" ]; then present="yes"; else present="no"; fi
fi

nscan=0; nblind=0; npresent=0; nunknown=0; nok=0
blind_list=""

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
    case "$present" in
        yes)
            npresent=$((npresent+1))
            printf 'wait-present  %-40s age=%smin user_idle=%smin(<%s) title=%s\n' \
                "$key" "$age" "$tok_age_min" "$IDLE_MIN" "$title"
            ;;
        no)
            nblind=$((nblind+1))
            blind_list="$blind_list $key(${age}min)"
            printf 'BLIND         %-40s age=%smin user_idle=%smin title=%s\n' \
                "$key" "$age" "$tok_age_min" "$title"
            ;;
        *)
            nunknown=$((nunknown+1))
            printf 'wait-unknown  %-40s age=%smin（在场判据不可用：db=%s src=%s）title=%s\n' \
                "$key" "$age" "$DB" "$tok_src" "$title"
            ;;
    esac
done

# ── 扫描面零值守卫（M410 R2 口径）──
#   nscan 可以合法为 0（台账目录存在、确实没有待办）。但零值必须落在「已证明目录可用之后」，
#   不许是盲区：复验一次目录与可读性，任何一项失守都判 PREMISE-FAIL（宁可不判，不许假绿）。
if [ "$nscan" -eq 0 ]; then
    if [ ! -d "$PEND_DIR" ] || [ ! -r "$PEND_DIR" ]; then
        echo "PREMISE-FAIL nscan=0 但台账目录不可用（dir=$PEND_DIR）"
        exit 3
    fi
    echo "SCAN-GUARD nscan=0 且台账目录存在可读 ⇒ 真空（不是扫描盲区）"
fi

printf 'NOTIFY-ACK nscan=%s blind=%s wait_present=%s wait_unknown=%s ok=%s blind_min=%s idle_min=%s user_present=%s tok_age_min=%s\n' \
    "$nscan" "$nblind" "$npresent" "$nunknown" "$nok" "$BLIND_MIN" "$IDLE_MIN" "$present" "$tok_age_min"

if [ "$nblind" -gt 0 ]; then
    echo "BLIND-KEYS$blind_list"
    exit 1
fi
exit 0
