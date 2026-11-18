#!/usr/bin/env bash
# ============================================================================
# chain_tail.sh —— 链尾接力判据【唯一实现】（M412 · 缺陷 821 / M417 · 缺陷 836）
# ----------------------------------------------------------------------------
# 病史一（M412 实测 · 2026-11-17）：
#   M411 于 08:11 发布完成、08:32 本机 rpm 升级完毕，然后**什么都没发生**。
#   我（东月）没有立项、没有交接单、没有 park；用户 13:30 来问「M412 进度」
#   —— 那一刻 M412 压根不存在。空转 5h19m。
#   当时的监控口径主动判绿：idle-unpushed 只看「HEAD 领先 origin/main」+
#   「NEXT-QUEUE 有未开工项」，两者都 0 ⇒ 打印 "OK 没活可接"。
#
# 病史二（M417 复核 · 2026-11-18）：
#   M415 09:19 完成 → 12:05 用户来问，2h46m 零提交；而本脚本判 OK（open=10）。
#   病因：只看「棒在不在文件里」，不看「有没有人在接」。
#   ⇒ 一个把 10 条候选写进文件就再也不动的系统，在旧判据里是「有接力」。
#
# 本脚本把这件事变成可判的（两条独立的病）：
#   ① 棒丢了：最后一轮 RELEASE.done 距今 > --max-age（默认 3600s），且
#      以下接力载体**一个都没有** ⇒ STALE（rc=1）
#         · 告警/任务台账（有人在等处置，排除本 key 自身）
#         · 正在跑的 m<数字>-{chain,release,gate,postupgrade,red-guard} 单元
#         · 合规 CAMPAIGN-IDLE（必须同时带 UNTIL= 与 REASON=）
#   ② 棒在但没人接（M417 新增）：有 `- [ ]` 未开工项，却
#         · RELEASE.done 之后没有任何新 commit（taken=0）
#         · 且 NEXT-QUEUE 文件没有在宽限期内被改成 `- [~]`（taking=0）
#      ⇒ STALE-OPEN（rc=1）
#      为什么 `- [ ]` 本身不算接棒：写候选是「排队的动作」，不是「开干的动作」。
#      为什么「改标签」有宽限：改 `- [~]` 是接棒的第一秒，但必须随后产出 commit；
#        超过 --grace（默认 1800s）还没有 commit ⇒ 仍判 STALE-OPEN。
#        （这就是「只改标签不算接棒」的负控口径，门里 S4c/S4d 守着。）
#
# 为什么 CAMPAIGN-IDLE 要这么严：一个光秃秃的「别报我」开关，最后一定会变成永久静音
#   —— M385 的 idle-unpushed-OFF 就因为没人写 UNTIL/REASON 静默了 4h46m。
#
# 用法：
#   chain_tail.sh [--ad DIR] [--pending DIR] [--max-age SEC] [--now TS]
#                 [--repo DIR] [--grace SEC] [--busy N | --busy-scan] [--quiet]
#   退出码：0 = N-A / YOUNG / OK / EXEMPT（有接力或还没到点）
#           1 = STALE（棒丢了） / STALE-OPEN（棒在没人接）
#           2 = 参数/环境错
# 输出（单行，供门与发布链 grep）：
#   CHAIN-TAIL=<V> done=<basename|none> age=<秒|none> open=<n> open_do=<n> pending=<n> busy=<n> exempt=<0|1> taken=<0|1|-1> taking=<0|1> why=<字>
#   taken: 1=done 之后有新 commit · 0=没有 · -1=无法判定（无 done commit / 非 git 仓库）
#   ⚠️ taken=-1 时**不**据此判绿（不猜）；此时只靠 taking 与实际载体。
# ============================================================================
set -uo pipefail

AD=/data/autodrive
PEND=/var/log/dy-ops/pending
MAXAGE=3600
NOW=""
BUSY=""
SCAN_BUSY=1
QUIET=0
# 仓库根按**脚本位置**推导（缺陷 300：门里出现开发机绝对路径 ⇒ 本机绿 / CI 红 / 分片污染被测树）
CT_HERE="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
REPO="$(cd "$CT_HERE/../.." 2>/dev/null && pwd || echo "")"
GRACE=1800

while [ $# -gt 0 ]; do
    case "$1" in
        --ad)         AD="${2:-}"; shift 2 ;;
        --pending)    PEND="${2:-}"; shift 2 ;;
        --max-age)    MAXAGE="${2:-}"; shift 2 ;;
        --now)        NOW="${2:-}"; shift 2 ;;
        --repo)       REPO="${2:-}"; shift 2 ;;
        --grace)      GRACE="${2:-}"; shift 2 ;;
        --busy)       BUSY="${2:-}"; SCAN_BUSY=0; shift 2 ;;
        --busy-scan)  SCAN_BUSY=1; shift ;;
        --quiet)      QUIET=1; shift ;;
        -h|--help)    sed -n '2,52p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done

case "$MAXAGE" in ''|*[!0-9]*) echo "❌ --max-age 必须是整数：$MAXAGE" >&2; exit 2 ;; esac
case "$GRACE"  in ''|*[!0-9]*) echo "❌ --grace 必须是整数：$GRACE" >&2; exit 2 ;; esac
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
    [ "$QUIET" = 1 ] || echo "CHAIN-TAIL=N-A done=none age=none open=0 open_do=0 pending=0 busy=0 exempt=0 taken=0 taking=0 why=none"
    exit 0
fi
age=$(( NOW - done_mtime ))
[ "$age" -ge 0 ] || age=0

# ② 接力载体一：NEXT-QUEUE 未开工项 / 进行中项 / 文件动过的时刻
open_n=0
open_do=0
qfile_mtime=0
if [ -f "$AD/NEXT-QUEUE.md" ]; then
    open_n="$(grep -c '^[[:space:]]*- \[ \]' "$AD/NEXT-QUEUE.md" 2>/dev/null || true)"
    case "$open_n" in ''|*[!0-9]*) open_n=0 ;; esac
    open_do="$(grep -c '^[[:space:]]*- \[~\]' "$AD/NEXT-QUEUE.md" 2>/dev/null || true)"
    case "$open_do" in ''|*[!0-9]*) open_do=0 ;; esac
    qfile_mtime="$(stat -c %Y "$AD/NEXT-QUEUE.md" 2>/dev/null || echo 0)"
    case "$qfile_mtime" in ''|*[!0-9]*) qfile_mtime=0 ;; esac
fi

# ③ 接力载体二：告警/任务台账（有人在等处置）
#    ⚠️ 必须排除「链尾自己拉响的那条」：否则本判据的报警会把自己判成「有接力」，
#       下一次运行立刻变绿并把台账了结 ⇒ 报警/了结 30 分钟振荡一次，而人还是没立项。
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

# ⑤ 接棒迹象（M417 新增，与载体分开算：载体是「有棒」，这里才是「有人接」）
#    taken：RELEASE.done 里的 commit 之后是否有新提交
#    taking：NEXT-QUEUE 里已有 `- [~]` 且文件在宽限期内被改过（接棒的第一秒）
taken=0
taking=0
why=none
done_commit="$(sed -n 's/.*[Cc]ommit=\([0-9a-f]\{7,40\}\).*/\1/p' "$done_file" 2>/dev/null | head -1)"
# 缺陷 838（M417 复核实测）：旧判据用 `[ -d "$REPO/.git" ]` 认仓库 ——
#   分片/worktree 里 .git 是**普通文件**（gitdir: ...），该判据恒假 ⇒ taken 恒 -1
#   ⇒ 只要 NEXT-QUEUE 有 `- [ ]`／`- [~]` 且宽限期已过，本判据必然 STALE-OPEN 误红，
#   与代码无关（2026-11-18 M417 链 15:14→17:00 跑 106min，shard8 里 taking 过期 5149s 判红）。
#   改用 git 自身判仓库（worktree 同样返回成功），只在 git 真不可用时才 taken=-1。
if [ -n "$done_commit" ] && command -v git >/dev/null 2>&1 && git -C "$REPO" rev-parse --git-dir >/dev/null 2>&1; then
    n_after="$(git -C "$REPO" rev-list --count "$done_commit..HEAD" 2>/dev/null || echo "")"
    case "$n_after" in
        ''|*[!0-9]*) taken=-1; why=unknown-commit ;;
        0)           taken=0;  why=no-new-commit ;;
        *)           taken=1;  why="commit+$n_after" ;;
    esac
else
    taken=-1; why=unknown-commit
fi
qstate=none
if [ "$open_do" -gt 0 ] && [ "$qfile_mtime" -gt "$done_mtime" ]; then
    q_age=$(( NOW - qfile_mtime ))
    [ "$q_age" -ge 0 ] || q_age=0
    if [ "$q_age" -le "$GRACE" ]; then
        taking=1
        qstate="taking-${q_age}s"
    else
        qstate="taking-expired-${q_age}s"
    fi
fi
# why 同时承载两条线索（commit 判定 + NEXT-QUEUE 动过的时刻），用逗号分隔
#   —— 只留一条会把另一条吞掉（S4d 首跑就红在这：why 已被 unknown-commit 占用，
#      taking-expired 写不进去，门读不到「标签过期」这个事实）。
if [ "$qstate" != none ]; then
    if [ "$why" = none ]; then why="$qstate"; else why="$why,$qstate"; fi
fi

# ⑥ 接力载体四：显式收工（必须 UNTIL= + REASON= 两段齐全）
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
    elif [ "$busy_n" -gt 0 ] || [ "$pend_n" -gt 0 ]; then
        verdict=OK
    elif [ "$open_n" -gt 0 ] || [ "$open_do" -gt 0 ]; then
        # 有棒：区分「有人接」与「没人接」
        if [ "$taken" = 1 ] || [ "$taking" = 1 ]; then
            verdict=OK
        else
            verdict=STALE-OPEN
        fi
    else
        verdict=STALE
    fi
fi

[ "$QUIET" = 1 ] || printf 'CHAIN-TAIL=%s done=%s age=%s open=%s open_do=%s pending=%s busy=%s exempt=%s taken=%s taking=%s why=%s\n' \
    "$verdict" "$(basename "$done_file")" "$age" "$open_n" "$open_do" "$pend_n" "$busy_n" "$exempt" "$taken" "$taking" "$why"

if [ "$verdict" = STALE ]; then
    echo "   ❌ 链尾断了：$(basename "$done_file") 已 $((age/60)) 分钟，且没有任何接力载体" >&2
    echo "      （NEXT-QUEUE 未开工项 0 · pending 台账 0 · 里程碑单元 0 · 无合规 CAMPAIGN-IDLE）" >&2
    echo "      修法：① 立项下一轮并写 $AD/NEXT-QUEUE.md（至少一条 '- [ ]'）" >&2
    echo "            ② 或写 $AD/CAMPAIGN-IDLE，必须含 UNTIL=… 与 REASON=… 两行" >&2
    exit 1
fi

if [ "$verdict" = STALE-OPEN ]; then
    echo "   ❌ 有棒没人接：$(basename "$done_file") 已 $((age/60)) 分钟，NEXT-QUEUE 有 $open_n 条未开工项，但" >&2
    echo "      既无 $((GRACE/60)) 分钟内标过的 '- [~]'（taking=0），也无可判定的新提交（taken=$taken，why=$why）" >&2
    echo "      修法：把那一条改成 '- [~]' **并立刻产出提交**（只改标签超过 $((GRACE/60)) 分钟不产出提交，仍判红）" >&2
    [ "$taken" = -1 ] && echo "      注意：taken=-1 表示拿不到 done 的 commit 或 $REPO 不是 git 仓库 ⇒ 本判据不据此判绿" >&2
    exit 1
fi
exit 0
