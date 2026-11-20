#!/usr/bin/env bash
# ============================================================
# selfhost/refresh_gate_baseline.sh —— 门耗时基线**回灌**器（M400 · 缺陷 763）
# ------------------------------------------------------------
# 为什么有它（M400 实测的病）：
#   每轮全量门都会产出 shardN.times.tsv / quiet*.times.tsv（实测耗时），
#   而**从来没有任何自动化把它们喂回 selfhost/gate_time_baseline.tsv**：
#     · 上次基线生成 = 10-26 14:30（18 天前）
#     · 基线 233 行 / 门清单 274 门 ⇒ 41 门 check_gate_timeouts 的 P2 根本判不到（静默绿）
#     · m187 基线写 476s，而它在提交前门里实测 755s/900s = 83%（只在被 rm 掉的 mktemp 表里）
#   ⇒ 「判据的输入必须可复算」这条纪律，卡在「谁去喂」这一步上。本脚本就是那个「谁」。
#
# 口径（与 gate_time_baseline.sh 同源，不另立一套）：
#   · 只吃**已完成**的链（stamp 目录含 CHAIN-COMPLETE.txt）—— 正在跑的链没有这个文件，
#     天然排除，避免「拿半截数据回灌」和「判据读自己」。
#   · 取最近 N 个已完成 stamp（默认 3）：**下界是窗口而不是永久最大值**，
#     这样某门被修好之后基线能真降下来（M400 前 m187 476s 永不降，就是取全历史的后果）。
#   · 当前基线用 --retain 兜底：只补「窗口内没跑到的门」，不参与取最大。
#   · 顺带吃 preflight 的计时表（packaging/release_preflight.sh 现已落盘不再 rm）。
#
# 用法：
#   selfhost/refresh_gate_baseline.sh [--stamps N] [--stamp <名>] [--measured-dir DIR]
#                                     [--baseline FILE] [--commit] [--push] [--dry-run]
#                                     [--self-test]
# 退出码：0 已刷新（或有变化但 --dry-run）/ 1 覆盖不全 / 2 用法错 / 3 无可用输入
# ============================================================
set -uo pipefail
SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
REPO_DEFAULT="$(cd "$SELF_DIR/.." && pwd)"

MDIR="/data/gatevm"
BASE="selfhost/gate_time_baseline.tsv"
REPO=""
NWIN=3
ONESTAMP=""
DO_COMMIT=0
DO_PUSH=0
DRY=0
SELF=0
EXTRA=()     # "文件<TAB>标签" —— 额外计时表（新门首次入册时的 bootstrap 实测）

say() { printf '[%s] %s\n' "$(date '+%F %T')" "$*"; }

while [ $# -gt 0 ]; do
    case "$1" in
        --measured-dir) MDIR="${2:-}"; shift 2 ;;
        --extra-tsv)    EXTRA+=("${2:-}"$'\t'"${3:-}"); shift 3 ;;
        --repo)         REPO="${2:-}"; shift 2 ;;
        --baseline)     BASE="${2:-}"; shift 2 ;;
        --stamps)       NWIN="${2:-}"; shift 2 ;;
        --stamp)        ONESTAMP="${2:-}"; shift 2 ;;
        --commit)       DO_COMMIT=1; shift ;;
        --push)         DO_PUSH=1; DO_COMMIT=1; shift ;;
        --dry-run)      DRY=1; shift ;;
        --self-test)    SELF=1; shift ;;
        -h|--help)      sed -n '2,28p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1（--help 看用法）" >&2; exit 2 ;;
    esac
done
case "$NWIN" in ''|*[!0-9]*) echo "❌ --stamps 必须是正整数" >&2; exit 2 ;; esac
[ "$NWIN" -ge 1 ] || { echo "❌ --stamps 必须 ≥1" >&2; exit 2; }

# 门清单（与 check_gate_timeouts.sh 同源：run_gates.sh --list，零副作用）
gate_list() {
    local repo="$1"
    ( cd "$repo" && ./selfhost/run_gates.sh --list 2>/dev/null ) | grep -v '^[[:space:]]*$' | sort -u
}

# 已完成 stamp（含 CHAIN-COMPLETE.txt），升序 —— 名字本身就是 shard-YYYYMMDD-HHMMSS，字典序 = 时间序
done_stamps() {
    local d="$1"
    for x in "$d"/shard-*; do
        [ -d "$x" ] || continue
        [ -f "$x/CHAIN-COMPLETE.txt" ] || continue
        basename "$x" | sed 's/^shard-//'
    done | sort
}

refresh() {   # $1=repo ; 结果写全局 RESULT_TXT
    local repo="$1"
    local bpath="$repo/$BASE"
    [ -f "$bpath" ] || { echo "❌ 基线不存在：$bpath" >&2; return 3; }
    [ -d "$MDIR" ] || { echo "❌ --measured-dir 不存在：$MDIR" >&2; return 3; }

    local -a picks=()
    if [ -n "$ONESTAMP" ]; then
        picks=("$ONESTAMP")
        [ -f "$MDIR/shard-$ONESTAMP/CHAIN-COMPLETE.txt" ] || { echo "❌ stamp 未完成或不存在：$MDIR/shard-$ONESTAMP" >&2; return 3; }
    else
        mapfile -t ALL < <(done_stamps "$MDIR")
        local n=${#ALL[@]}
        [ "$n" -ge 1 ] || { echo "❌ $MDIR 下没有任何已完成的链（缺 CHAIN-COMPLETE.txt）" >&2; return 3; }
        local from=$(( n - NWIN )); [ "$from" -lt 0 ] && from=0
        picks=("${ALL[@]:$from}")
    fi
    say "回灌窗口：${#picks[@]} 个已完成 stamp（$(printf '%s ' "${picks[@]}")）"

    local -a SRC=()
    local cnt=0
    for st in "${picks[@]}"; do
        local d="$MDIR/shard-$st"
        local got=0
        for f in "$d"/*.times.tsv; do
            [ -f "$f" ] || continue
            SRC+=(--from "$f" --label "chain-$st"); cnt=$((cnt+1)); got=$((got+1))
        done
        [ "$got" -gt 0 ] || say "⚠️ $st 没有 times.tsv（跳过）"
    done
    [ "$cnt" -gt 0 ] || { echo "❌ 窗口内没有任何 times.tsv" >&2; return 3; }

    # preflight 计时表：只吃「不早于窗口最老 stamp」的（老表是别的机器状态，别混进来）
    local oldest="${picks[0]}"
    local pf=0
    for f in "$MDIR"/preflight-*.times.tsv; do
        [ -f "$f" ] || continue
        local ps; ps="$(basename "$f" | sed -E 's/^preflight-([0-9]{8}-[0-9]{6})\.times\.tsv$/\1/')"
        case "$ps" in [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]-[0-9][0-9][0-9][0-9][0-9][0-9]) ;; *) continue ;; esac
        [ "$ps" \< "$oldest" ] && continue
        SRC+=(--from "$f" --label "preflight-$ps"); cnt=$((cnt+1)); pf=$((pf+1))
    done
    [ "$pf" -gt 0 ] && say "含 preflight 计时表 $pf 份（提交前门 11 门，同一套 900s 配额）"

    # 额外计时表（新门 bootstrap）：--extra-tsv <文件> <标签>
    local ex
    for ex in "${EXTRA[@]:-}"; do
        [ -n "$ex" ] || continue
        local ef="${ex%%$'\t'*}" el="${ex##*$'\t'}"
        if [ -f "$ef" ] && [ -n "$el" ]; then
            SRC+=(--from "$ef" --label "$el"); cnt=$((cnt+1)); say "额外计时表：$el = $ef"
        else
            echo "❌ --extra-tsv 参数不完整（文件=$ef 标签=${el:-空}）" >&2; return 3
        fi
    done

    local out="$bpath"
    if [ "$DRY" = 1 ]; then out="$(mktemp /tmp/btest.XXXXXX)"; fi
    local rc
    ( cd "$repo" && ./selfhost/gate_time_baseline.sh "${SRC[@]}" --retain "$bpath" --out "$out" ) >/tmp/gtb_run.log 2>&1
    rc=$?
    tail -2 /tmp/gtb_run.log | sed 's/^/    /'
    [ "$rc" = 0 ] || { echo "❌ 生成器 rc=$rc（日志 /tmp/gtb_run.log）" >&2; [ "$DRY" = 1 ] && rm -f "$out"; return 3; }

    # ── 先剪掉「不在门清单里」的行 ──
    #   为什么：输入里可能有残余（stub 计时表、已删门的旧行）。基线必须 ⊆ 门清单，
    #   否则 P4 天天报残留、P6 对真门判不准。剪枝在这里做一次，比在判据里做可靠。
    local glist0
    glist0="$(gate_list "$repo")"
    awk -F'\t' -v OFS='\t' '
        NR==FNR { keep[$1]=1; next }
        /^#/ || $0=="" { print; next }
        ($1 in keep) { print }
    ' <(printf '%s\n' "$glist0") "$out" > "$out.keep" && mv "$out.keep" "$out"

    # ── 覆盖自查（P6 同口径；在这里先判，免得等到门里才发现）──
    local glist gmiss=0
    glist="$glist0"
    local ng nb
    ng="$(printf '%s\n' "$glist" | grep -c . || true)"
    nb="$(grep -vc '^#' "$out" || true)"
    while IFS= read -r g; do
        [ -n "$g" ] || continue
        awk -F'\t' -v g="$g" '$1==g{f=1} END{exit !f}' "$out" \
            || { echo "  ⚠️ 基线仍缺门：$g" >&2; gmiss=$((gmiss+1)); }
    done <<< "$glist"
    say "覆盖：门清单 $ng 门 · 基线 $nb 行 · 缺 $gmiss"

    if [ "$DRY" = 1 ]; then
        say "DRY-RUN：新基线在 $out（未写回），缺 $gmiss"
        rm -f "$out"
        [ "$gmiss" = 0 ] || return 1
        return 0
    fi

    if cmp -s "$out" <(git -C "$repo" show HEAD:"$BASE" 2>/dev/null) 2>/dev/null; then
        say "REFRESH-RESULT=UNCHANGED（与 HEAD 逐字节相同，无需提交）"
    else
        say "REFRESH-RESULT=CHANGED（$BASE 已更新）"
    fi

    [ "$gmiss" = 0 ] || { say "REFRESH-COVERAGE=GAP(缺 $gmiss) —— 不提交，先补覆盖"; return 1; }
    say "REFRESH-COVERAGE=OK"

    if [ "$DO_COMMIT" = 1 ]; then
        # ── 冻树标记等待（M423 · 缺陷 865）──────────────────────────────────
        #   病史（2026-11-20 14:28:50 实测）：M423 判绿 → 本脚本立刻回灌，而链要到
        #   publish 完（make_tag + push，实测 45s）才在 EXIT trap 里摘 <git-dir>/chain-freeze；
        #   .githooks/pre-commit 见标即拦（缺陷 848）⇒ REFRESH-COMMIT=FAIL(commit)。
        #   这是结构性的：判绿后「立刻」回灌，必然落在冻结窗口里。
        #   ⇒ 提交前有界等待标记消失（默认 900s，5s 一探）；超时则响亮失败，不猜、不硬闯。
        local gd freeze fwait="${PU_FREEZE_WAIT:-900}" waited=0
        gd="$(git -C "$repo" rev-parse --absolute-git-dir 2>/dev/null)"
        freeze="${gd:-$repo/.git}/chain-freeze"
        if [ -f "$freeze" ]; then
            say "REFRESH-FREEZE=WAIT(链未退，冻树标记在 $freeze；最多等 ${fwait}s)"
            while [ -f "$freeze" ] && [ "$waited" -lt "$fwait" ]; do
                sleep 5; waited=$((waited + 5))
            done
            if [ -f "$freeze" ]; then
                say "REFRESH-COMMIT=FAIL(freeze 等了 ${waited}s 仍未摘 · $freeze · 链可能仍在跑；本次不提交，链退出后重跑本脚本即可)"
                return 1
            fi
            say "REFRESH-FREEZE=OK(等了 ${waited}s，冻树标记已摘)"
        fi
        # 树必须干净（除本文件）：回灌是机械段，不该和在写的东西混在一个提交里
        local dirty
        dirty="$(git -C "$repo" status --porcelain | grep -v " ${BASE}$" | head -5)"
        if [ -n "$dirty" ]; then
            say "REFRESH-COMMIT=SKIP(工作树有其它改动，不代你提交)"
            printf '%s\n' "$dirty" | sed 's/^/    /'
            return 0
        fi
        if git -C "$repo" diff --quiet -- "$BASE"; then
            say "REFRESH-COMMIT=SKIP(基线无变化)"
            return 0
        fi
        local msg="chore(基线)：回灌 $(printf '%s' "${picks[-1]}") 的门耗时实测（缺陷 763）"
        git -C "$repo" add "$BASE" || { say "REFRESH-COMMIT=FAIL(git add)"; return 1; }
        printf '%s\n' "$msg" > /tmp/gtb_commit.txt
        git -C "$repo" commit -F /tmp/gtb_commit.txt >/tmp/gtb_commit.log 2>&1 \
            || { say "REFRESH-COMMIT=FAIL(commit，见 /tmp/gtb_commit.log)"; return 1; }
        say "REFRESH-COMMIT=OK $(git -C "$repo" rev-parse --short HEAD)"
        if [ "$DO_PUSH" = 1 ]; then
            # M417（缺陷 836 实测）：M416 那轮 14:30:49 commit OK、14:31:29 push FAIL（40s 超时），
            #   台账只写「基线回灌失败」⇒ 看上去像「覆盖不全」，实际只是推不动。
            #   而失败后留下的 pending 台账，又和「真失败」混在一起，无法分辨。
            #   改为：3 次退避重试（1s/3s），失败文案必须带 attempts + 已提交 sha + 错误摘要。
            local attempt=0 max=3 rc_push=1 pmsg="" sha
            sha="$(git -C "$repo" rev-parse --short HEAD 2>/dev/null || echo unknown)"
            while [ "$attempt" -lt "$max" ]; do
                attempt=$((attempt + 1))
                if git -C "$repo" push origin HEAD:main >/tmp/gtb_push.log 2>&1; then
                    rc_push=0; break
                fi
                pmsg="$(tail -3 /tmp/gtb_push.log 2>/dev/null | tr '\n' ' ' | cut -c1-160)"
                # 非快进/被拒 = 推不上去（远端有新提交），退避重试不会有帮助 ⇒ 立刻停，交人裁决
                if grep -qiE 'rejected|non-fast-forward|fetch first' /tmp/gtb_push.log 2>/dev/null; then
                    pmsg="非快进（远端有新提交，需人工 rebase）：$pmsg"
                    break
                fi
                case "$attempt" in 1) sleep 1 ;; 2) sleep 3 ;; esac
            done
            if [ "$rc_push" = 0 ]; then
                say "REFRESH-PUSH=OK attempts=$attempt"
            else
                say "REFRESH-PUSH=FAIL(attempts=$attempt · 已提交 $sha 未推送 · main 领先 origin；err=$pmsg；log=/tmp/gtb_push.log)"
                return 1
            fi
        fi
    fi
    return 0
}

# ---------- 自证（离线夹具，不碰真仓库/真 gatevm）----------
# ⚠️ 用全局 ST_T 而不是 local：trap 在函数返回后触发，local 变量那时已被 set -u 判未绑定
#    （首版实测：跑完最后一行报「T: unbound variable」）
selftest() {
    ST_T="$(mktemp -d /tmp/m400rf.XXXXXX)" || exit 3
    local T="$ST_T"
    [ -z "${SELFTEST_KEEP:-}" ] && trap 'rm -rf "$ST_T"' EXIT
    local n=0 FAIL=0
    ok()  { n=$((n+1)); echo "  ✅ $*"; }
    bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }

    local R="$T/repo"; mkdir -p "$R/selfhost" "$T/md"
    cp "$SELF_DIR/gate_time_baseline.sh" "$R/selfhost/"
    chmod +x "$R/selfhost/gate_time_baseline.sh"
    # 假门清单：stub run_gates.sh（真 run_gates.sh --list 要读整仓 registry，自证不该依赖它）
    cat > "$R/selfhost/run_gates.sh" <<'EOS'
#!/usr/bin/env bash
[ "${1:-}" = "--list" ] && { printf 'g1\ng2\ng3\ng4\n'; exit 0; }
exit 2
EOS
    chmod +x "$R/selfhost/run_gates.sh"
    # 旧基线：g1=900 是**全历史**最大值（修复前），窗口里应该看不到它
    printf '# old\n# 门名\t秒\t状态\t来源\ng1\t900\tOK\tr24-contended-N10\ng2\t50\tOK\tr24-contended-N10\ng3\t40\tOK\tr24-contended-N10\n' \
        > "$R/selfhost/gate_time_baseline.tsv"

    mkstamp() {  # mkstamp <stamp> <g1秒> <g4秒> [124]
        local d="$T/md/shard-$1"
        mkdir -p "$d"
        printf 'chain_ts=%s\nok=4\nfail=0\n' "$1" > "$d/CHAIN-COMPLETE.txt"
        { printf 'g1\tOK\t%s\t-\n' "$2"; printf 'g2\tOK\t60\t-\n'
          printf 'g3\tOK\t40\t-\n'; printf 'g4\tOK\t%s\t-\n' "$3"; } > "$d/shard1.times.tsv"
        [ "${4:-}" = 124 ] && printf 'g2\tFAIL\t899\t124\n' >> "$d/shard1.times.tsv"
        return 0
    }
    # 6 个已完成 stamp：g1 = 950 900 700 | 500 100 100（窗口 3 ⇒ 只看后三个）
    mkstamp 20260101-000000 950 30
    mkstamp 20260102-000000 900 30
    mkstamp 20260103-000000 700 30
    mkstamp 20260104-000000 500 30
    mkstamp 20260105-000000 100 30
    mkstamp 20260106-000000 100 30 124     # 本轮 g1 已修好（100）；附带一行 rc=124

    MDIR="$T/md" bash "$0" --repo "$R" --measured-dir "$T/md" \
        --baseline selfhost/gate_time_baseline.tsv --stamps 3 >"$T/t1.log" 2>&1
    local rc1=$?
    local v1 v2 v4
    v1="$(awk -F'\t' '$1=="g1"{print $2}' "$R/selfhost/gate_time_baseline.tsv")"
    v2="$(awk -F'\t' '$1=="g2"{print $2}' "$R/selfhost/gate_time_baseline.tsv")"
    v4="$(awk -F'\t' '$1=="g4"{print $2}' "$R/selfhost/gate_time_baseline.tsv")"

    # T0 前提自证：回灌真跑了（rc=0 且有覆盖结论行）—— 没有这步，后面全是空判
    if [ "$rc1" = 0 ] && grep -q 'REFRESH-COVERAGE=OK' "$T/t1.log"; then
        ok "T0 前提：回灌真跑了且覆盖 OK"
    else
        bad "T0 前提不成立：rc=$rc1（见 $T/t1.log）"; sed -n '1,6p' "$T/t1.log" | sed 's/^/      /'
    fi

    # T1 窗口下界：g1 = 500（窗口内最大值）；950/900/700 在窗外，不许参与
    [ "$v1" = 500 ] && ok "T1 窗口下界生效：g1=500（窗外 950/900/700 未参与取最大）" \
                    || bad "T1 g1=$v1（应 500；=900 说明窗外值在参与，=950 说明读了全历史）"

    # T2 rc=124 不许当耗时（缺陷 763 正面判据）：g2 仍是 60，不是砍断值 899
    [ "$v2" = 60 ] && ok "T2 rc=124 被丢：g2=60（砍断值 899 未进基线）" \
                   || bad "T2 g2=$v2（应 60；=899 说明 124 行被当成耗时）"

    # T3 retain 只补缺：g4 旧基线没有，窗口实测 30 ⇒ 补进来
    [ "$v4" = 30 ] && ok "T3 覆盖补齐：g4=30（旧基线无此行，窗口实测补上）" || bad "T3 g4=$v4"

    # T4 幂等：同输入二次回灌，**数据行 + 除「生成时间」外的注释行**逐字节相同
    #   ⚠️ 不能用整文件 cmp：文件头含「生成时间」，两次运行几乎必然跨秒 ⇒ flaky。
    #      2026-11-13 22:2x 实测：同一脚本 r2 判绿、重跑就红（唯一差异是 `# 生成时间：` 那行）。
    #      幂等的语义是「同样输入不产出不同数据」，不是「文件头时间戳冻结」。
    cp "$R/selfhost/gate_time_baseline.tsv" "$T/b1.tsv"
    MDIR="$T/md" bash "$0" --repo "$R" --measured-dir "$T/md" \
        --baseline selfhost/gate_time_baseline.tsv --stamps 3 >"$T/t4.log" 2>&1
    local d4 rows4
    d4="$(diff <(grep -v '^# 生成时间：' "$T/b1.tsv" 2>/dev/null) \
               <(grep -v '^# 生成时间：' "$R/selfhost/gate_time_baseline.tsv" 2>/dev/null) | head -4)"
    rows4="$(grep -vc '^#' "$R/selfhost/gate_time_baseline.tsv" 2>/dev/null || echo 0)"
    # 先证前提（两个文件都在、且有数据行）—— 否则 cmp/diff 两个空文件会假绿
    if [ -s "$T/b1.tsv" ] && [ -s "$R/selfhost/gate_time_baseline.tsv" ] && [ "$rows4" -gt 0 ] && [ -z "$d4" ]; then
        ok "T4 幂等：二次回灌数据行逐字节相同（忽略生成时间行；$rows4 行）"
    else
        bad "T4 非幂等（行数 $rows4）：$(printf '%s' "$d4" | tr '\n' ' ')"
    fi
    # T4b 负控：把上面这条 diff 判据套在「被人改了一行数据」的副本上 ⇒ 必须非空
    #   （防「忽略生成时间」被误写成「什么都忽略」= 判据恒真）
    cp "$T/b1.tsv" "$T/b4mut.tsv"
    awk -F'\t' -v OFS='\t' '!/^#/ && !done { $2 = $2 + 1; done = 1 } { print }' \
        "$T/b4mut.tsv" > "$T/b4mut2.tsv" && mv "$T/b4mut2.tsv" "$T/b4mut.tsv"
    if [ -n "$(diff <(grep -v '^# 生成时间：' "$T/b4mut.tsv") <(grep -v '^# 生成时间：' "$T/b1.tsv") | head -2)" ]; then
        ok "T4b 负控：数据行被改动 ⇒ 同一条 diff 判据仍能报出差异（T4 不是恒真）"
    else
        bad "T4b 判据恒真：改了一行数据仍判相同 ⇒ T4 的绿没有意义"
    fi

    # T5 覆盖缺口 ⇒ rc=1 且指名（另起一套干净夹具，不污染 T1 的结论）
    local R2="$T/repo2" M2="$T/md2"
    mkdir -p "$R2/selfhost" "$M2/shard-20260201-000000"
    cp "$SELF_DIR/gate_time_baseline.sh" "$R2/selfhost/"
    cp "$R/selfhost/run_gates.sh" "$R2/selfhost/"
    chmod +x "$R2/selfhost/"*.sh
    printf 'chain_ts=20260201-000000\nok=3\n' > "$M2/shard-20260201-000000/CHAIN-COMPLETE.txt"
    { printf 'g1\tOK\t10\t-\n'; printf 'g2\tOK\t20\t-\n'; printf 'g3\tOK\t30\t-\n'; } \
        > "$M2/shard-20260201-000000/shard1.times.tsv"
    printf '# x\ng1\t10\tOK\tseed\ng2\t20\tOK\tseed\ng3\t30\tOK\tseed\n' \
        > "$R2/selfhost/gate_time_baseline.tsv"
    MDIR="$M2" bash "$0" --repo "$R2" --measured-dir "$M2" \
        --baseline selfhost/gate_time_baseline.tsv --stamps 3 >"$T/t5.log" 2>&1
    local rc5=$?
    if [ "$rc5" = 1 ] && grep -q '基线仍缺门：g4' "$T/t5.log"; then
        ok "T5 覆盖缺口 ⇒ rc=1 且指名 g4"
    else
        bad "T5 覆盖缺口没报对：rc=$rc5 / $(grep -c '缺门：g4' "$T/t5.log") 次指名"
    fi
    # T5b 正控：把 g4 的实测补进同一套夹具 ⇒ 必须 rc=0（证明 T5 的红来自「缺」而非恒红）
    printf 'g4\tOK\t40\t-\n' >> "$M2/shard-20260201-000000/shard1.times.tsv"
    MDIR="$M2" bash "$0" --repo "$R2" --measured-dir "$M2" \
        --baseline selfhost/gate_time_baseline.tsv --stamps 3 >"$T/t5b.log" 2>&1
    local rc5b=$?
    [ "$rc5b" = 0 ] && ok "T5b 正控：补齐 g4 ⇒ rc=0（T5 的红来自缺口）" || bad "T5b 正控 rc=$rc5b"

    # T6 无已完成链 ⇒ rc=3（不许把「没数据」当「刷新成功」）
    mkdir -p "$T/md3/shard-20260301-000000"   # 只有目录，没有 CHAIN-COMPLETE.txt
    printf 'g1\tOK\t10\t-\n' > "$T/md3/shard-20260301-000000/shard1.times.tsv"
    MDIR="$T/md3" bash "$0" --repo "$R2" --measured-dir "$T/md3" \
        --baseline selfhost/gate_time_baseline.tsv >"$T/t6.log" 2>&1
    [ "$?" = 3 ] && ok "T6 未完成的链不算数据 ⇒ rc=3" || bad "T6 未完成链没报 rc=3"

    # ── T7/T8：push 重试（M417 · 缺陷 836）────────────────────────────────
    #   病史：M416 那轮 commit OK、push 40s 超时 ⇒ 台账只写「基线回灌失败」，
    #         看着像覆盖不全；而且一次网络抖动就留下人肉待办。
    #   这里用 git 包装器注入「前两次失败」，验证重试真的存在（不是写在注释里）。
    local GITREAL; GITREAL="$(command -v git)"
    if [ -z "$GITREAL" ]; then
        bad "T7/T8 跳过前提不成立：找不到真 git"
    else
        local BARE="$T/origin.git" CNTF="$T/pushcnt" GB="$T/bin_git"
        # fixture repo 原本只是「目录 + 文件」，从未 git init（旧用例不测 --commit）。
        #   测 push 就必须先让它成为真仓库，并把现状提交掉（否则「树脏不代提交」的守卫
        #   会把整条路径 SKIP 掉，T7 将变成「什么都没测」而假绿）。
        git -C "$R" init -q >/dev/null 2>&1
        git -C "$R" config user.email t@example.invalid >/dev/null 2>&1
        git -C "$R" config user.name t >/dev/null 2>&1
        git -C "$R" add -A >/dev/null 2>&1
        git -C "$R" commit -qm "fixture init（m417 selftest）" >/dev/null 2>&1
        git init -q --bare "$BARE" >/dev/null 2>&1
        git -C "$R" remote remove origin >/dev/null 2>&1 || true
        git -C "$R" remote add origin "$BARE" >/dev/null 2>&1
        git -C "$R" push -q origin HEAD:main >/dev/null 2>&1 || true
        mkdir -p "$GB"
        cat > "$GB/git" <<STUBG
#!/usr/bin/env bash
args=("\$@")
is_push=0
for a in "\$@"; do [ "\$a" = "push" ] && is_push=1; done
if [ "\$is_push" = 1 ]; then
    c=\$(cat "$CNTF" 2>/dev/null || echo 0); c=\$((c+1)); echo "\$c" > "$CNTF"
    if [ "\$c" -le "\${STUB_FAIL_N:-2}" ]; then
        echo "fatal: unable to access 'https://example.invalid/': Operation timed out" >&2
        exit 128
    fi
fi
exec "$GITREAL" "\${args[@]}"
STUBG
        chmod +x "$GB/git"
        # T7 正控：前 2 次失败、第 3 次成功 ⇒ rc=0 且 attempts=3
        : > "$CNTF"; mkstamp 20260107-000000 120 30
        PATH="$GB:$PATH" STUB_FAIL_N=2 MDIR="$T/md" bash "$0" --repo "$R" \
            --measured-dir "$T/md" --baseline selfhost/gate_time_baseline.tsv --stamps 3 --push \
            >"$T/t7.log" 2>&1
        local rc7=$?
        n=$((n+1))
        if [ "$rc7" = 0 ] && grep -q 'REFRESH-PUSH=OK attempts=3' "$T/t7.log"; then
            ok "T7 正控：push 前两次失败 ⇒ 重试第 3 次成功（attempts=3）"
        else
            bad "T7 rc=$rc7（应 0）；$(grep 'REFRESH-PUSH=' "$T/t7.log" | tail -1)"
        fi
        # T7b 前提自证：桩真被调了 3 次（证明上面那条走的不是「一次就成功」）
        local cnt7; cnt7="$(cat "$CNTF" 2>/dev/null || echo 0)"
        n=$((n+1))
        [ "$cnt7" = 3 ] && ok "T7b 前提自证：桩 git 的 push 调用计数 = 3" \
                        || bad "T7b 桩只被调 $cnt7 次（应 3 ⇒ 重试逻辑没跑）"
        # T8 负控：三次全败 ⇒ rc=1，且文案必须点名「已提交 sha 未推送」（不许再说「覆盖不全」）
        : > "$CNTF"; mkstamp 20260108-000000 130 30
        PATH="$GB:$PATH" STUB_FAIL_N=9 MDIR="$T/md" bash "$0" --repo "$R" \
            --measured-dir "$T/md" --baseline selfhost/gate_time_baseline.tsv --stamps 3 --push \
            >"$T/t8.log" 2>&1
        local rc8=$?
        n=$((n+1))
        if [ "$rc8" = 1 ] && grep -q 'REFRESH-PUSH=FAIL(attempts=3' "$T/t8.log" \
           && grep -q '未推送' "$T/t8.log"; then
            ok "T8 负控：三次全败 ⇒ rc=1，文案点名「已提交未推送」"
        else
            bad "T8 rc=$rc8（应 1）；$(grep 'REFRESH-PUSH=' "$T/t8.log" | tail -1)"
        fi
        # T9 负控：非快进（rejected）⇒ 不空转重试，attempts=1 立刻交人
        : > "$CNTF"; mkstamp 20260109-000000 140 30
        cat > "$GB/git" <<STUBG2
#!/usr/bin/env bash
args=("\$@")
is_push=0
for a in "\$@"; do [ "\$a" = "push" ] && is_push=1; done
if [ "\$is_push" = 1 ]; then
    c=\$(cat "$CNTF" 2>/dev/null || echo 0); c=\$((c+1)); echo "\$c" > "$CNTF"
    echo "! [rejected]        main -> main (non-fast-forward)" >&2
    exit 1
fi
exec "$GITREAL" "\${args[@]}"
STUBG2
        chmod +x "$GB/git"
        PATH="$GB:$PATH" MDIR="$T/md" bash "$0" --repo "$R" \
            --measured-dir "$T/md" --baseline selfhost/gate_time_baseline.tsv --stamps 3 --push \
            >"$T/t9.log" 2>&1
        local rc9=$? cnt9; cnt9="$(cat "$CNTF" 2>/dev/null || echo 0)"
        n=$((n+1))
        if [ "$rc9" = 1 ] && [ "$cnt9" = 1 ] && grep -q '非快进' "$T/t9.log"; then
            ok "T9 负控：非快进 ⇒ 不重试（attempts=1）并标注需人工 rebase"
        else
            bad "T9 rc=$rc9 cnt=$cnt9（应 1/1）；$(grep 'REFRESH-PUSH=' "$T/t9.log" | tail -1)"
        fi
    fi

    # ── T10/T10b：冻树标记等待（M423 · 缺陷 865）────────────────────────────
    #   病史：2026-11-20 14:28:50 M423 判绿后回灌 REFRESH-COMMIT=FAIL，真因是
    #         链要到 publish 完（make_tag+push ~45s）才摘 <git-dir>/chain-freeze，
    #         .githooks/pre-commit 见标即拦（缺陷 848）⇒ 回灌提交必撞冻结窗口。
    local R3="$T/repo3" M3="$T/md3b"
    mkdir -p "$R3/selfhost" "$M3/shard-20260401-000000"
    cp "$SELF_DIR/gate_time_baseline.sh" "$R3/selfhost/"
    cp "$R/selfhost/run_gates.sh" "$R3/selfhost/"
    chmod +x "$R3/selfhost/"*.sh
    printf 'chain_ts=20260401-000000\nok=4\n' > "$M3/shard-20260401-000000/CHAIN-COMPLETE.txt"
    { printf 'g1\tOK\t11\t-\n'; printf 'g2\tOK\t20\t-\n'
      printf 'g3\tOK\t30\t-\n'; printf 'g4\tOK\t40\t-\n'; } \
        > "$M3/shard-20260401-000000/shard1.times.tsv"
    printf '# x\ng1\t10\tOK\tseed\ng2\t20\tOK\tseed\ng3\t30\tOK\tseed\ng4\t40\tOK\tseed\n' \
        > "$R3/selfhost/gate_time_baseline.tsv"
    git -C "$R3" init -q >/dev/null 2>&1
    git -C "$R3" config user.email t@example.invalid >/dev/null 2>&1
    git -C "$R3" config user.name t >/dev/null 2>&1
    git -C "$R3" add -A >/dev/null 2>&1
    git -C "$R3" commit -qm "fixture3（m423 selftest）" >/dev/null 2>&1
    local F3 h0 h10
    F3="$(git -C "$R3" rev-parse --absolute-git-dir 2>/dev/null)/chain-freeze"
    h0="$(git -C "$R3" rev-parse HEAD)"
    # T10 正控：冻树标记 2s 后被摘 ⇒ 等完再提交，必须成功且 HEAD 前进
    printf 'milestone=M998\n' > "$F3"
    ( sleep 2; rm -f "$F3" ) &
    PU_FREEZE_WAIT=60 MDIR="$M3" bash "$0" --repo "$R3" --measured-dir "$M3" \
        --baseline selfhost/gate_time_baseline.tsv --stamps 1 --commit >"$T/t10.log" 2>&1
    local rc10=$?
    wait
    h10="$(git -C "$R3" rev-parse HEAD)"
    n=$((n+1))
    if [ "$rc10" = 0 ] && grep -q 'REFRESH-FREEZE=OK' "$T/t10.log" \
       && grep -q 'REFRESH-COMMIT=OK' "$T/t10.log" && [ "$h0" != "$h10" ]; then
        ok "T10 正控：冻树标记 2s 后摘 ⇒ 等完再提交（REFRESH-FREEZE=OK · HEAD 前进）"
    else
        bad "T10 rc=$rc10 head_moved=$([ "$h0" != "$h10" ] && echo yes || echo no)；$(grep -E 'REFRESH-(FREEZE|COMMIT)=' "$T/t10.log" | tr '\n' ' ')"
    fi
    # T10b 负控：冻树标记一直不摘 ⇒ 到点 rc=1 且**不提交**（不许静默跳过、不许硬闯钩子）
    printf 'milestone=M997\n' > "$F3"
    local hb; hb="$(git -C "$R3" rev-parse HEAD)"
    PU_FREEZE_WAIT=3 MDIR="$M3" bash "$0" --repo "$R3" --measured-dir "$M3" \
        --baseline selfhost/gate_time_baseline.tsv --stamps 1 --commit >"$T/t10b.log" 2>&1
    local rc10b=$?
    n=$((n+1))
    if [ "$rc10b" = 1 ] && grep -q 'REFRESH-FREEZE=WAIT' "$T/t10b.log" \
       && grep -q 'FAIL(freeze' "$T/t10b.log" \
       && [ "$(git -C "$R3" rev-parse HEAD)" = "$hb" ]; then
        ok "T10b 负控：冻树标记不摘 ⇒ rc=1 且未提交（不静默跳过、不硬闯钩子）"
    else
        bad "T10b rc=$rc10b head_moved=$([ "$(git -C "$R3" rev-parse HEAD)" = "$hb" ] && echo no || echo yes)；$(grep -E 'REFRESH-(FREEZE|COMMIT)=' "$T/t10b.log" | tr '\n' ' ')"
    fi
    rm -f "$F3"

    echo "── refresh 自测：$((n-FAIL))/$n 通过 ──"
    [ "$FAIL" = 0 ] || return 1
    return 0
}

if [ "$SELF" = 1 ]; then selftest; exit $?; fi

# 默认在仓库根跑（SELF_DIR/..），BASE 是相对仓库的路径
REPO="${REPO:-$REPO_DEFAULT}"
cd "$REPO" || exit 2
refresh "$REPO"
