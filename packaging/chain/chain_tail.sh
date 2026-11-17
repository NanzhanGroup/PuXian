#!/usr/bin/env bash
# ============================================================================
# chain_tail.sh —— 链尾接力判据【唯一实现】（M412 · 缺陷 821）
# ----------------------------------------------------------------------------
# 病史（真事，2026-11-17）：
#   M411 于 08:11 发布完成、08:32 本机 rpm 升级完毕，然后**什么都没发生**。
#   我（东月）没有立项、没有交接单、没有 park；用户 13:30 来问「M412 进度」
#   —— 那一刻 M412 压根不存在。空转 5h19m。
#
#   更糟的是当时的监控口径**主动判了绿灯**：
#   dy-ops-watch-lib-idle.sh 的 idle_unpushed_verdict 只看
#     「HEAD 是否领先 origin/main」+「NEXT-QUEUE 是否有未开工项」，
#   两者都是 0 ⇒ 打印 "OK 没活可接"。也就是说：
#     「一轮干完了、下一轮没人立项」这件事，在所有既有判据里都是**正常态**。
#
# 本脚本把这件事变成可判的：
#   最后一轮 RELEASE.done 距今 > --max-age（默认 3600s），且
#   以下接力载体**一个都没有** ⇒ STALE（rc=1）
#     ① NEXT-QUEUE.md 里还有 `- [ ]`（未开工项）
#     ② /var/log/dy-ops/pending/ 有台账（有告警/任务在等人处置）
#     ③ 有正在跑的 m<数字>* systemd 单元（链/发布/升级 —— 有人真在干活）
#     ④ /data/autodrive/CAMPAIGN-IDLE 显式收工（必须同时带 UNTIL= 与 REASON=）
#
# 为什么 ④ 要这么严：一个光秃秃的「别报我」开关，最后一定会变成永久静音
#   —— M385 的 idle-unpushed-OFF 就因为没人写 UNTIL/REASON 静默了 4h46m。
#   缺任一段 ⇒ 豁免不生效，照判 STALE（这条有负控守着）。
#
# 用法：
#   chain_tail.sh [--ad DIR] [--pending DIR] [--max-age SEC] [--now TS]
#                 [--busy N | --busy-scan] [--quiet]
#   退出码：0 = N-A / YOUNG / OK / EXEMPT（有接力或还没到点）
#           1 = STALE（发布完成太久且无人接力）
#           2 = 参数/环境错
# 输出（单行，供门与发布链 grep）：
#   CHAIN-TAIL=<V> done=<basename|none> age=<秒|none> open=<n> pending=<n> busy=<n> exempt=<0|1>
# ============================================================================
set -uo pipefail

AD=/data/autodrive
PEND=/var/log/dy-ops/pending
MAXAGE=3600
NOW=""
BUSY=""
SCAN_BUSY=1
QUIET=0

while [ $# -gt 0 ]; do
    case "$1" in
        --ad)         AD="${2:-}"; shift 2 ;;
        --pending)    PEND="${2:-}"; shift 2 ;;
        --max-age)    MAXAGE="${2:-}"; shift 2 ;;
        --now)        NOW="${2:-}"; shift 2 ;;
        --busy)       BUSY="${2:-}"; SCAN_BUSY=0; shift 2 ;;
        --busy-scan)  SCAN_BUSY=1; shift ;;
        --quiet)      QUIET=1; shift ;;
        -h|--help)    sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done

case "$MAXAGE" in ''|*[!0-9]*) echo "❌ --max-age 必须是整数：$MAXAGE" >&2; exit 2 ;; esac
[ -d "$AD" ] || { echo "❌ 目录不存在：$AD" >&2; exit 2; }

# 取「现在」：只从时钟取，不猜；测试时可用 --now 注入（自测需要确定性）
if [ -z "$NOW" ]; then
    NOW="$(date +%s)"
else
    case "$NOW" in ''|*[!0-9]*) echo "❌ --now 必须是 epoch 秒：$NOW" >&2; exit 2 ;; esac
fi

# ① 最后一轮发布完成标记（多个里程碑时取最新的）
done_file=""
done_mtime=""
for f in "$AD"/M*-RELEASE.done; do
    [ -f "$f" ] || continue
    m="$(stat -c %Y "$f" 2>/dev/null || echo 0)"
    case "$m" in ''|*[!0-9]*) m=0 ;; esac
    if [ -z "$done_mtime" ] || [ "$m" -gt "$done_mtime" ]; then
        done_mtime="$m"; done_file="$f"
    fi
done

if [ -z "$done_file" ]; then
    [ "$QUIET" = 1 ] || echo "CHAIN-TAIL=N-A done=none age=none open=0 pending=0 busy=0 exempt=0"
    exit 0
fi
age=$(( NOW - done_mtime ))
[ "$age" -ge 0 ] || age=0

# ② 接力载体一：NEXT-QUEUE 未开工项
open_n=0
if [ -f "$AD/NEXT-QUEUE.md" ]; then
    open_n="$(grep -c '^[[:space:]]*- \[ \]' "$AD/NEXT-QUEUE.md" 2>/dev/null || true)"
    case "$open_n" in ''|*[!0-9]*) open_n=0 ;; esac
fi

# ③ 接力载体二：告警/任务台账（有人在等处置）
#    ⚠️ 必须排除「链尾自己拉响的那条」：否则本判据的报警会把自己判成「有接力」，
#       下一次运行立刻变绿并把台账了结 ⇒ 报警/了结 30 分钟振荡一次，而人还是没立项。
#       这是「判据被自己的产物喂饱」的典型形态，故用白名单式排除（默认 *chain-tail*）。
pend_n=0
if [ -d "$PEND" ]; then
    pend_n="$(find "$PEND" -maxdepth 1 -type f ! -name "*chain-tail*" 2>/dev/null | wc -l)"
    case "$pend_n" in ''|*[!0-9]*) pend_n=0 ;; esac
fi

# ④ 接力载体三：正在跑的里程碑单元（有人真在干活）
if [ -n "$BUSY" ]; then
    busy_n="$BUSY"
    case "$busy_n" in ''|*[!0-9]*) echo "❌ --busy 必须是整数：$BUSY" >&2; exit 2 ;; esac
elif [ "$SCAN_BUSY" = 1 ] && command -v systemctl >/dev/null 2>&1; then
    # 只认「发布链相关」的里程碑单元：m<数字>-{chain,release,gate,postupgrade,red-guard}。
    #   第一版写成 m[0-9]* 通配 ⇒ 一个不相干的 m412-rebake（重烘）也会被判成「有人在接力」，
    #   而重烘/探针根本不是本轮工作被接棒。载体判据宁可窄，不许宽。
    busy_n="$(systemctl list-units --state=running --no-legend --no-pager 'm[0-9]*' 2>/dev/null \
                | awk '{print $1}' | grep -cE '^m[0-9]+-(chain|release|gate|postupgrade|red-guard)' || true)"
    case "$busy_n" in ''|*[!0-9]*) busy_n=0 ;; esac
else
    busy_n=0
fi

# ⑤ 接力载体四：显式收工（必须 UNTIL= + REASON= 两段齐全）
exempt=0
idle_file="$AD/CAMPAIGN-IDLE"
if [ -f "$idle_file" ]; then
    if grep -q 'UNTIL=' "$idle_file" 2>/dev/null && grep -q 'REASON=' "$idle_file" 2>/dev/null; then
        exempt=1
    fi
fi

verdict=OK
[ "$age" -le "$MAXAGE" ] && verdict=YOUNG
if [ "$verdict" = OK ]; then
    if [ "$exempt" = 1 ]; then
        verdict=EXEMPT
    elif [ "$open_n" -gt 0 ] || [ "$pend_n" -gt 0 ] || [ "$busy_n" -gt 0 ]; then
        verdict=OK
    else
        verdict=STALE
    fi
fi

[ "$QUIET" = 1 ] || printf 'CHAIN-TAIL=%s done=%s age=%s open=%s pending=%s busy=%s exempt=%s\n' \
    "$verdict" "$(basename "$done_file")" "$age" "$open_n" "$pend_n" "$busy_n" "$exempt"

if [ "$verdict" = STALE ]; then
    echo "   ❌ 链尾断了：$(basename "$done_file") 已 $((age/60)) 分钟，且没有任何接力载体" >&2
    echo "      （NEXT-QUEUE 未开工项 0 · pending 台账 0 · 里程碑单元 0 · 无合规 CAMPAIGN-IDLE）" >&2
    echo "      修法：① 立项下一轮并写 $AD/NEXT-QUEUE.md（至少一条 '- [ ]'）" >&2
    echo "            ② 或写 $AD/CAMPAIGN-IDLE，必须含 UNTIL=… 与 REASON=… 两行" >&2
    exit 1
fi
exit 0
