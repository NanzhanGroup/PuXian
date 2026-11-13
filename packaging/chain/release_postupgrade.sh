#!/usr/bin/env bash
# ============================================================================
# release_postupgrade.sh —— 判绿后「自动把本机升到新版」伴生单元（M393）
#
# 病史（2026-11-12 实测）：M392 22:04 起链时手敲了 m392-postupgrade，
#   但 r1 判红它 SKIP-CHAIN-RED 退出；r2 23:50 重跑判绿 01:31 —— 这个单元**没有跟着重启**
#   ⇒ 自动 dnf 升级从未触发，06:27 才由人手工补。根因：伴生单元靠手敲，链重启就丢。
#   修法：由 launch_release_chain.sh 统一起，且本脚本**不因一次 NO-GO 就退出**——
#   STAMP 可能是上一轮的红（链已重跑），只在「STAMP 文件 mtime 晚于本单元启动」时才采信。
#
# 判据链（全部满足才升级）：
#   ① STAMP 含 CHAIN-VERDICT=GREEN 且 mtime ≥ 本进程启动时间
#   ② 镜像 version.json 的 version == v0.2.<ms>（两站都 200）
#   ③ 当前 rpm 版本 != 目标 ⇒ 才 dnf upgrade（幂等）
#   ④ 升级后自验 rpm -q 与 px --version 都等于目标
#
# 用法：release_postupgrade.sh --ms 393 [--mirror URL] [--interval 120]
#                              [--max-hours 6] [--dry-run] [--self-test]
# 退出码：0 已升级或本就最新 / 1 失败已 p1 / 2 参数错 / 3 超时
# ============================================================================
set -uo pipefail

MS=""; MIRROR="${PU_MIRROR:-https://soft.xiusoft.cn/puxian}"; INTERVAL="${PU_INTERVAL:-120}"
MAXH=6; DRY=0; SELF=0; STAMP=""
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"
LOG="${PU_LOG:-/data/autodrive/postupgrade-$(date +%Y%m%d).log}"

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)        MS="${2:-}"; shift 2 ;;
        --stamp)     STAMP="${2:-}"; shift 2 ;;
        --mirror)    MIRROR="${2:-}"; shift 2 ;;
        --interval)  INTERVAL="${2:-120}"; shift 2 ;;
        --max-hours) MAXH="${2:-6}"; shift 2 ;;
        --dry-run)   DRY=1; shift ;;
        --self-test) SELF=1; shift ;;
        -h|--help)   sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

say() { printf '[%s] %s\n' "$(date +%F' '%T)" "$*" >> "$LOG" 2>/dev/null || true; }
alert() { say "ALERT $1 $2"; [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "$1" "$2" "$3" >/dev/null 2>&1 || true; }

mirror_ver() {  # 取镜像 version.json 的 version 字段；取不到回空
    # 自测缝：PU_MIRROR_CMD 是**一段命令**（如 printf '{"version":"0.2.999"}'），
    #   必须经 bash -c 执行；直接 "$PU_MIRROR_CMD" 会把它当成单个文件名 ⇒ 取不到版本 ⇒ 空等超时
    #   （首跑 124s 超时就是这么来的：判据没取到值，却被当成"镜像没齐"）。
    if [ -n "${PU_MIRROR_CMD:-}" ]; then bash -c "$PU_MIRROR_CMD"; return $?; fi
    timeout 20 curl -fsS "$MIRROR/version.json" 2>/dev/null \
        | tr -d ' \n' | grep -oE '"version":"[^"]+"' | head -1 | cut -d'"' -f4
}

run_pu() {
    [ -n "$MS" ] || { echo "--ms 必填" >&2; return 2; }
    local TARGET="0.2.$MS" TAG="v0.2.$MS"
    STAMP="${STAMP:-/data/autodrive/M${MS}-GATE-DONE}"
    local start; start=$(date +%s)
    local deadline=$(( start + MAXH * 3600 ))
    say "=== postupgrade start ms=$MS target=$TARGET mirror=$MIRROR dry=$DRY pid=$$ ==="

    # ① 等本轮的 GREEN
    #   不能用「mtime ≥ 本进程启动」严格比：两者都是秒级精度，同一秒起单元/写 STAMP 会判成「旧」，
    #   结果是等满 6h 都不升级（自测首跑就是这样卡死 124s 超时的）。给 300s 容差：
    #   容差内 ⇒ 认作本轮；早于容差 ⇒ 一定是上一轮遗留（不采信，这就是 M392 的漏）。
    local GRACE=${PU_STAMP_GRACE:-300}
    while :; do
        if [ -f "$STAMP" ] && [ "$(stat -c %Y "$STAMP" 2>/dev/null || echo 0)" -ge $((start - GRACE)) ] \
           && grep -q 'CHAIN-VERDICT=GREEN' "$STAMP" 2>/dev/null; then
            say "① GREEN（STAMP mtime 在容差内 ${GRACE}s）"; break
        fi        # 本轮 STAMP 含 NO-GO ⇒ 等用户/东月修完重跑，本单元不退出（这正是 M392 的漏）
        [ "$(date +%s)" -ge "$deadline" ] && {   # -ge：--max-hours 0 首轮即到点（N1 钉这条；-gt 会死循环）
            alert "m${MS}-postupgrade-timeout" "M${MS} 升级等待超时（${MAXH}h）" \
                "STAMP=$STAMP 未出现本轮 GREEN；本机未升级。"
            return 3
        }
        sleep "$INTERVAL"
    done

    # ①' M400（缺陷 763）：判绿后把这一轮的实测耗时**回灌**进基线。
    #   为什么挂在这里而不是 publish 里：publish 要求树净，回灌会产生一个 chore 提交；
    #   而这一步本来就在「发布已完成」之后，多一个提交不影响 tag（tag 打的是发布提交）。
    #   失败**不挡升级**（升级是本单元的主职）：告警 + 让下一轮 gate_timeouts 的 P7 出面。
    baseline_refresh() {
        # 路径按**脚本位置**推导，不写死开发机绝对路径（缺陷 300 / check_gate_paths 门；
        #   M400 首跑就因这一行判红）。PU_REPO 仍可显式覆盖（分片 worktree / 异地部署）。
        local here root
        here=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
        root=$(cd "$here/../.." && pwd)
        local repo="${PU_REPO:-$root}"
        local rf="$repo/selfhost/refresh_gate_baseline.sh"
        if [ ! -x "$rf" ]; then say "①' 跳过基线回灌（$rf 不存在或不可执行）"; return 0; fi
        if [ "$DRY" = "1" ]; then
            say "①' DRY-RUN 基线回灌（只报不动）"
            "$rf" --repo "$repo" --dry-run >> "$LOG" 2>&1 || say "①' DRY-RUN 回灌报错（不影响）"
            return 0
        fi
        if "$rf" --repo "$repo" --commit --push >> "$LOG" 2>&1; then
            say "①' 基线回灌 OK"
        else
            say "①' 基线回灌 FAIL（下一轮 gate_timeouts 的 P7 会当场拦下）"
            alert "m${MS}-baseline-refresh" "M${MS} 基线回灌失败" \
                "下一轮 gate_timeouts 的 P7 会判红；手工：$rf --commit --push"
        fi
        return 0
    }

    baseline_refresh     # ①' 判绿后立刻回灌（在等镜像之前做：不依赖镜像，早做早收口）

    # ② 等镜像齐
    while :; do
        local mv; mv="$(mirror_ver)"
        if [ "$mv" = "$TARGET" ] || [ "$mv" = "$TAG" ]; then say "② 镜像=$mv 已齐"; break; fi
        if [ "$(date +%s)" -ge "$deadline" ]; then
            alert "m${MS}-mirror-stale" "M${MS} 已发布但镜像仍为 ${mv:-取不到}" \
                "目标 $TARGET；本机未升级（等镜像）。mirror=$MIRROR"
            return 3
        fi
        say "② 等待镜像（现=${mv:-取不到} want=$TARGET）"
        sleep "$INTERVAL"
    done

    # ③ 幂等：已经是最新就不动
    local cur; cur="$(rpm -q --qf '%{VERSION}\n' puxian 2>/dev/null | head -1)"
    if [ "$cur" = "$TARGET" ]; then say "③ 已是 $TARGET，无需升级"; return 0; fi
    if [ "$DRY" = "1" ]; then say "③ DRY-RUN 只报不动：cur=${cur:-none} want=$TARGET"; return 0; fi

    # ③ dnf 升级 —— 两步都不能省（M393 首跑实测踩到的两个真 bug）
    #   bug-a：镜像已到 v0.2.393（synced_at 10:40:22），但本机 dnf metadata 缓存是 09:45 的
    #          ⇒ `dnf -y upgrade puxian` 输出 "Nothing to do!" ⇒ 升了个寂寞。
    #          修：升级前 dnf -q makecache --refresh。
    #   bug-b：只做一次升级、不核对结果就报成功/失败 ⇒ 缓存半新半旧时一次失败就 p1。
    #          修：升级后自验 rpm 版本，不符则刷新缓存重试（最多 2 次），仍不符才 p1。
    say "③ dnf makecache --refresh"
    dnf -q makecache --refresh >> "$LOG" 2>&1 || say "③ makecache 返回非 0（继续，下面会自验结果）"
    local try=1 nowv
    while [ "$try" -le 2 ]; do
        say "③ dnf -y upgrade puxian（第 $try 次，cur=${cur:-none} → $TARGET）"
        dnf -y upgrade puxian >> "$LOG" 2>&1
        rc_up=$?
        nowv="$(rpm -q --qf '%{VERSION}\n' puxian 2>/dev/null | head -1)"
        say "③ 第 $try 次 rc=$rc_up ⇒ rpm=${nowv:-none}"
        [ "${nowv:-}" = "$TARGET" ] && break
        [ "$try" -ge 2 ] && break
        say "③ 未达目标 ⇒ 再刷一次 metadata 重试"
        dnf -q makecache --refresh >> "$LOG" 2>&1 || true
        try=$((try+1))
    done

    # ④ 自验（不是"dnf 说成功就算成功"）—— nowv 已在 ③ 的循环里 refresh 过
    local binv
    nowv="$(rpm -q --qf '%{VERSION}\n' puxian 2>/dev/null | head -1)"
    binv="$(px --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
    say "④ rpm=$nowv px=$binv want=$TARGET"
    if [ "$nowv" != "$TARGET" ] || [ "$binv" != "$TARGET" ]; then
        alert "m${MS}-postupgrade-mismatch" "M${MS} 升级后版本不符" \
            "rpm=${nowv:-无} px=${binv:-无} want=$TARGET（dnf 说成功不等于升上了）"
        return 1
    fi
    say "===DONE 本机已升到 $TARGET==="
    return 0
}

if [ "$SELF" = 1 ]; then
    T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
    cat > "$T/notify" <<'STUB'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$STUB_ALERTS"
exit 0
STUB
    chmod +x "$T/notify"
    export STUB_ALERTS="$T/alerts.txt"; : > "$STUB_ALERTS"
    export DY_NOTIFY_BIN="$T/notify" PU_LOG="$T/pu.log" PU_INTERVAL=0
    # 自测不许碰真仓库：PU_REPO 指向不存在的目录 ⇒ ①' 走「跳过回灌」分支
    #   （既隔离，又顺带证明「缺件不挡升级」）
    export PU_REPO="$T/norepo"
    n=0; FAIL=0
    ok()  { n=$((n+1)); echo "  ✅ $*"; }
    bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }

    # P1 正控：本轮 GREEN + 镜像齐 + 已是目标版 ⇒ rc=0 且不叫人不升级
    printf 'CHAIN-VERDICT=GREEN\n' > "$T/stamp"
    PU_MIRROR_CMD="echo 0.2.999" MS=999 STAMP="$T/stamp" PU_RPM_VER=0.2.999 \
        bash "$0" --ms 999 --stamp "$T/stamp" --dry-run >/dev/null 2>&1
    rc=$?; [ "$rc" = 0 ] && ok "P1 正控：GREEN+镜像齐+dry ⇒ rc=0" || bad "P1 rc=$rc"

    # N1 负控：STAMP 是**上一轮**遗留（mtime 早于启动）⇒ 不许采信 ⇒ 超时 rc=3 并叫人
    printf 'CHAIN-VERDICT=GREEN\n' > "$T/old"
    touch -d '2020-01-01' "$T/old"
    : > "$STUB_ALERTS"
    MS=996 STAMP="$T/old" PU_MIRROR_CMD="echo 0.2.996" \
        bash "$0" --ms 996 --stamp "$T/old" --max-hours 0 >/dev/null 2>&1
    rc=$?; [ "$rc" = 3 ] && ok "N1 负控：旧 STAMP 不采信 ⇒ 超时 rc=3" || bad "N1 rc=$rc（应 3）"
    grep -q 'postupgrade-timeout' "$STUB_ALERTS" 2>/dev/null && ok "N1 报出 postupgrade-timeout" || bad "N1 没报警"

    # N2 负控：GREEN 本轮但镜像没齐 ⇒ 超时 rc=3，键 mirror-stale
    printf 'CHAIN-VERDICT=GREEN\n' > "$T/stamp2"
    : > "$STUB_ALERTS"
    MS=995 STAMP="$T/stamp2" PU_MIRROR_CMD="echo 0.2.900" \
        bash "$0" --ms 995 --stamp "$T/stamp2" --max-hours 0 >/dev/null 2>&1
    rc=$?; [ "$rc" = 3 ] && ok "N2 负控：镜像滞后 ⇒ rc=3" || bad "N2 rc=$rc（应 3）"
    grep -q 'mirror-stale' "$STUB_ALERTS" 2>/dev/null && ok "N2 报出 mirror-stale" || bad "N2 键不对"

    # N3 负控：缺 --ms ⇒ rc=2
    bash "$0" >/dev/null 2>&1; [ "$?" = 2 ] && ok "N3 负控：缺 --ms ⇒ rc=2" || bad "N3 没报错"

    # 前提自证：假 notify 真被调用
    [ -s "$STUB_ALERTS" ] && ok "前提自证：notify 真被调用（$(( $(wc -l < "$STUB_ALERTS") )) 次）" \
                         || bad "前提缺失：notify 从未被调用"

    echo "── postupgrade 自测：$((n-FAIL))/$n 通过 ──"
    [ "$FAIL" = 0 ] || exit 1
    exit 0
fi

run_pu
