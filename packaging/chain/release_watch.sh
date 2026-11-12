#!/usr/bin/env bash
# ============================================================================
# release_watch.sh —— PuXian 发布观察器【唯一实现 · 参数化】（M394 · 缺陷 745）
# ----------------------------------------------------------------------------
# 为什么要有它（病史，每条都真实发生过）：
#   M375…M393 每轮手写一个 m<N>-release-watch.sh，从 2650 字节长到 11702 字节，
#   每一轮的修法只在那一份里生效，下一轮复制时又丢。最直接的后果：
#     2026-11-12 01:31 M392 判绿，publish 日志写「WARN 观察器不存在：m392-release-watch.sh」
#     ⇒ 11-11 刚修好的「镜像已齐，可 dnf upgrade」通报**从来没有生成过**；
#        用户整晚不知道发布成功（那条「已发布」通知又恰好 3 次投递失败被放弃）。
#   ⇒ 本轮把它入仓、参数化，里程碑只传 --ms。
#
# 用法：release_watch.sh --ms 394 [选项]
#   --ms N            里程碑号；也接受 m394 / v0.2.394 / 0.2.394（自动归一）
#   --tag TAG         默认 v0.2.<N>
#   --ver VER         默认 0.2.<N>
#   --rpm-ms S        rpm 文件名里的里程碑段，默认 m<N>
#   --mirror-url U    默认 https://soft.xiusoft.cn/puxian
#   --log-dir DIR     默认 $DY_WATCH_AD 或 /data/autodrive
#   --body-file F     正文文件，默认 <log-dir>/m<N>_notify_body.txt
#   --rounds N        主窗口轮数（默认 40）· --main-sleep S（默认 90s）
#   --grace N         超时后复核轮数（默认 3）· --grace-sleep S（默认 180s）
#   --no-notify       只判定不通知（演练用）
#   --selftest        离线自测（不联网、不写系统日志）
# 退出码：0 OK / GRACE-OK · 1 TIMEOUT · 2 参数错
#
# 判据（全部满足才 OK）：
#   ① Release 存在且资产 >= 4        ② tarball > 60MB（缺陷 660 体积下限）
#   ③ releases/latest == TAG         ④ 镜像 version.json 的 tag == TAG
#   ⑤ 镜像 rpm el9 / el7 均 200      （④⑤ 是缺陷 663：判据必须测「节点真正依赖的端点」）
#
# 沿用的历史修法 —— 改它之前先读对应事故：
#   660  tarball 体积下限（防止半截资产被判齐）
#   663  主判据 = 镜像根，不是 gh-pages
#   698① 通知正文读文件，不硬编码上一里程碑内容
#   698③ 用户通知唯一出口（dy-notify.sh）
#   699  60min 硬超时比镜像同步真实耗时更紧（M388 差 5 秒报假警、用户通报永远没发）
#        ⇒ 超时后 GRACE 复核再下结论；窗口外齐 = GRACE-OK，照样通报，不挂 p1
#   704  令牌 401/403 ⇒ 降级匿名 + 放宽轮询；**取不到数 ≠ 资产还没出**（不许当 0）
#   707  看 rc 说话：dy-notify 非零必须记 FAIL，不许 announce 成功
#   737  呈报**新信息**：OK ⇒「镜像已齐（可 dnf upgrade）」。不再报一遍「已发布」
#        ——「已发布」已由 publish_on_green 报过，用户手机上曾因此收到两条
# ============================================================================
set -uo pipefail

MS_RAW=""; TAG=""; VER=""; RPM_MS=""
MIRROR="${DY_WATCH_MIRROR:-https://soft.xiusoft.cn/puxian}"
LOGDIR="${DY_WATCH_AD:-/data/autodrive}"
BODY_FILE=""; ROUNDS="${WATCH_ROUNDS:-40}"; MAIN_SLEEP="${WATCH_MAIN_SLEEP:-90}"
GRACE="${WATCH_GRACE:-3}"; GRACE_SLEEP="${WATCH_GRACE_SLEEP:-180}"
SELFTEST=0; DO_NOTIFY=1
REPO="NanzhanGroup/PuXian"
TGZ_MIN=62914560

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)          MS_RAW="${2:-}"; shift 2 ;;
        --tag)         TAG="${2:-}"; shift 2 ;;
        --ver)         VER="${2:-}"; shift 2 ;;
        --rpm-ms)      RPM_MS="${2:-}"; shift 2 ;;
        --mirror-url)  MIRROR="${2:-}"; shift 2 ;;
        --log-dir)     LOGDIR="${2:-}"; shift 2 ;;
        --body-file)   BODY_FILE="${2:-}"; shift 2 ;;
        --rounds)      ROUNDS="${2:-}"; shift 2 ;;
        --main-sleep)  MAIN_SLEEP="${2:-}"; shift 2 ;;
        --grace)       GRACE="${2:-}"; shift 2 ;;
        --grace-sleep) GRACE_SLEEP="${2:-}"; shift 2 ;;
        --no-notify)   DO_NOTIFY=0; shift ;;
        --selftest)    SELFTEST=1; shift ;;
        -h|--help)     sed -n '2,40p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

# ── 里程碑号归一：m394 / v0.2.394 / 0.2.394 / 394 ⇒ 394 ─────────────────
norm_ms() {
    local s="${1:-}"
    s="$(printf '%s' "$s" | sed -E 's/^[[:space:]]+//; s/[[:space:]]+$//; s/^[mM]//; s/^v?[0-9]+\.[0-9]+\.//')"
    case "$s" in
        ''|*[!0-9]*) return 1 ;;
    esac
    printf '%s' "$s"
}

MS="$(norm_ms "$MS_RAW")" || {
    if [ "$SELFTEST" = 1 ]; then
        :   # 自测不需要 --ms
    else
        echo "--ms 非法（接受 394 / m394 / v0.2.394）：${MS_RAW:-空}" >&2; exit 2
    fi
}
[ -n "${MS:-}" ] || MS="0"
TAG="${TAG:-v0.2.$MS}"
VER="${VER:-0.2.$MS}"
RPM_MS="${RPM_MS:-m$MS}"
N="$MS"
LOG="$LOGDIR/m$N-release-watch.log"
BODY_FILE="${BODY_FILE:-$LOGDIR/m${N}_notify_body.txt}"
API="https://api.github.com/repos/$REPO"
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"
PAT_FILE="${DY_PAT_FILE:-/data/pat.md}"

say() { echo "[$(date +%F' '%T)] $*" >> "$LOG" 2>/dev/null || echo "[$(date +%F' '%T)] $*"; }

# ── 判据纯函数：给六个量，吐 OK / FAIL（离线可测）──────────────────────
judge() {  # judge <assets> <tgz> <latest> <mirror_tag> <el9> <el7>
    local a="${1:-0}" t="${2:-0}" l="${3:-?}" m="${4:-?}" n9="${5:-?}" n7="${6:-?}"
    [ "$a" -ge 4 ] 2>/dev/null || { echo FAIL; return; }
    [ "$t" -gt "$TGZ_MIN" ] 2>/dev/null || { echo FAIL; return; }
    [ "$l" = "$TAG" ] || { echo FAIL; return; }
    [ "$m" = "$TAG" ] || { echo FAIL; return; }
    { [ "$n9" = "200" ] && [ "$n7" = "200" ]; } || { echo FAIL; return; }
    echo OK
}

# ── 通知标题分级（缺陷 737）：OK 只报「新信息」，不重报「已发布」────────
title_for() {
    case "$1" in
        OK)       printf 'PuXian %s 镜像已齐（可 dnf upgrade）' "$TAG" ;;
        GRACE-OK) printf 'PuXian %s 资产已齐（镜像滞后）' "$TAG" ;;
        *)        printf 'PuXian %s 发布状态' "$TAG" ;;
    esac
}

# ── 自测：离线，不联网、不写真实日志、不通知 ─────────────────────────────
if [ "$SELFTEST" = 1 ]; then
    # 自测期间日志落临时文件，绝不污染真实台账
    LOG="$(mktemp -u /tmp/rw-selftest.XXXXXX).log"
    pass=0; fail=0
    chk() {  # chk <名字> <期望> <实得>
        if [ "$2" = "$3" ]; then pass=$((pass+1)); printf 'ok   %-34s => %s\n' "$1" "$3"
        else fail=$((fail+1)); printf 'FAIL %-34s 期望 %s 实得 %s\n' "$1" "$2" "$3"; fi
    }
    # 前提自证：断言机制本身必须能失败（否则下面全绿没有意义）
    chk_proof() {
        local b="$fail"
        chk "P0 前提自证（故意错）" OK FAIL >/dev/null
        if [ "$fail" -eq $((b+1)) ]; then
            pass=$((pass+1)); printf 'ok   %-34s => 断言机制确实能失败\n' "P0 前提自证"
        else
            printf 'FAIL %-34s 故意错未被计入 fail\n' "P0 前提自证"
        fi
        fail="$b"
    }
    chk_proof

    chk "C1 全齐"              OK   "$(judge 4 73943186 "$TAG" "$TAG" 200 200)"
    chk "C2 镜像未到"          FAIL "$(judge 4 73943186 "$TAG" v0.2.1 200 200)"
    chk "C3 el9=404"           FAIL "$(judge 4 73943186 "$TAG" "$TAG" 404 200)"
    chk "C4 资产不足(3)"       FAIL "$(judge 3 73943186 "$TAG" "$TAG" 200 200)"
    chk "C5 tarball 太小"      FAIL "$(judge 4 1000 "$TAG" "$TAG" 200 200)"
    chk "N1 空串不当 0（704）" FAIL "$(judge '' '' "$TAG" "$TAG" 200 200)"

    chk "N2 归一 m394"         394  "$(norm_ms m394)"
    chk "N2 归一 v0.2.394"     394  "$(norm_ms v0.2.394)"
    chk "N2 归一 0.2.394"      394  "$(norm_ms 0.2.394)"
    chk "N3 归一拒非法 m39x"   "rc=1" "$(norm_ms m39x >/dev/null 2>&1 && echo rc=0 || echo rc=1)"

    t_ok="$(title_for OK)"; t_gr="$(title_for GRACE-OK)"
    case "$t_ok" in
        *已发布*) chk "N4 OK 标题不许含已发布(737)" 0 1 ;;
        *)        chk "N4 OK 标题不许含已发布(737)" 0 0 ;;
    esac
    [ "$t_ok" != "$t_gr" ] && chk "N5 OK/GRACE 标题必须区分" 0 0 || chk "N5 OK/GRACE 标题必须区分" 0 1

    echo "SELFTEST pass=$pass fail=$fail"
    rm -f "$LOG" 2>/dev/null || true
    [ "$fail" = 0 ]
    exit $?
fi

[ "$N" != "0" ] || { echo "--ms 必填" >&2; exit 2; }

# ── GitHub API 取数（缺陷 704）：令牌失效绝不静默 ────────────────────────
AUTH_EV="${DY_WATCH_AUTH_EV:-/tmp/m${N}-release-watch.auth.events}"
auth_ev() { printf '%s %s\n' "$(date +%T)" "$1" >> "$AUTH_EV" 2>/dev/null || true; }
auth_state() {
    local inv tok an
    inv=$(grep -c 'token-invalid' "$AUTH_EV" 2>/dev/null || true); inv=${inv:-0}
    tok=$(grep -c 'token-ok' "$AUTH_EV" 2>/dev/null || true);      tok=${tok:-0}
    an=$(grep -c 'anon-ok' "$AUTH_EV" 2>/dev/null || true);        an=${an:-0}
    if [ "$inv" -gt 0 ]; then echo "PAT-INVALID(token401/403)×$inv·anon-ok×$an"
    elif [ "$tok" -gt 0 ]; then echo "token-ok×$tok"
    else echo "no-token(anon-ok×$an)"; fi
}
PAT="$(tr -d ' \t\r\n' < "$PAT_FILE" 2>/dev/null || true)"
gh_get() {  # $1=API 路径 ⇒ stdout=响应体，rc=0 成功
    local body code
    if [ -n "${PAT:-}" ]; then
        body=$(curl -sS -m 25 -H "Authorization: Bearer $PAT" -H "Accept: application/vnd.github+json" \
                    -w $'\n%{http_code}' "https://api.github.com$1" 2>/dev/null)
        code=$(printf '%s' "$body" | tail -1); body=$(printf '%s' "$body" | sed '$d')
        case "$code" in
            200) auth_ev token-ok; printf '%s' "$body"; return 0 ;;
            401|403) auth_ev "token-invalid http=$code"
                     say "WARN AUTH-INVALID http=$code $1 —— 令牌失效（不是「资产未出」），改匿名重取"
                     PAT="" ;;
            *) auth_ev "token-http-$code" ;;
        esac
    fi
    body=$(curl -sS -m 25 -H "Accept: application/vnd.github+json" -w $'\n%{http_code}' \
                "https://api.github.com$1" 2>/dev/null)
    code=$(printf '%s' "$body" | tail -1); body=$(printf '%s' "$body" | sed '$d')
    if [ "$code" = "200" ]; then
        auth_ev anon-ok; printf '%s' "$body"; return 0
    fi
    auth_ev "anon-http-$code"
    say "WARN ANON-FAIL http=$code $1（两侧都取不到数 ⇒ 本次采样无效，不得当资产=0）"
    return 1
}

probe() {  # 一行六个值：assets tgz latest mtag el9 el7
    local rel latest assets tgz mtag el9 el7
    rel=$(gh_get "/repos/$REPO/releases/tags/$TAG") || rel=""
    latest=$(gh_get "/repos/$REPO/releases/latest" | grep -o '"tag_name": *"[^"]*"' | head -1 | sed 's/.*"\(v[^"]*\)"/\1/')
    assets=$(printf '%s' "$rel" | grep -c '"browser_download_url"')
    tgz=$(printf '%s' "$rel" | grep -o '"size": *[0-9]*' | head -1 | grep -o '[0-9]*')
    mtag=$(curl -s -m 20 "$MIRROR/version.json" | grep -o '"tag": *"[^"]*"' | head -1 | sed 's/.*"\(v[^"]*\)"/\1/')
    el9=$(curl -s -o /dev/null -m 25 -w '%{http_code}' -I "$MIRROR/rpm/9/x86_64/puxian-$VER-1.$RPM_MS.el9.x86_64.rpm")
    el7=$(curl -s -o /dev/null -m 25 -w '%{http_code}' -I "$MIRROR/rpm/7/x86_64/puxian-$VER-1.$RPM_MS.el7.x86_64.rpm")
    printf '%s %s %s %s %s %s\n' "${assets:-0}" "${tgz:-0}" "${latest:-?}" "${mtag:-?}" "${el9:-?}" "${el7:-?}"
}

notify_good() {  # $1=OK|GRACE-OK  $2=已等秒数
    local how="$1" waited="$2" body rc=0 title
    if [ -r "$BODY_FILE" ]; then body="$(cat "$BODY_FILE")"
    else body="PuXian $TAG 已发布，详见仓库 CHANGELOG。"; fi
    title="$(title_for "$how")"
    local tail_txt
    if [ "$how" = "OK" ]; then
        tail_txt="tag $TAG · Release 资产齐全 · 镜像 el7/el9 双 200（dnf upgrade puxian 即可）· 齐活用时 $((waited/60))min"
    else
        tail_txt="tag $TAG · 资产齐全，镜像尚未同步完成（可先 dnf upgrade，稍后自动跟上）· 用时 $((waited/60))min"
    fi
    if [ "$DO_NOTIFY" != 1 ]; then say "NOTIFY-SKIP how=$how（--no-notify）"; echo "WATCH-$how" >> "$LOG"; return 0; fi
    if DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" user "$title" "$body
$tail_txt" >> "$LOG" 2>&1; then
        say "USER-NOTIFIED how=$how"
    else
        rc=$?
        say "WARN USER-NOTIFY-FAIL rc=$rc（dy-notify 非零：已落 pending-user 由 esc 重投，此处不算送达）"
    fi
    echo "WATCH-$how" >> "$LOG"
}

# ── 开跑前验一次令牌：坏令牌不许装成「资产未出」（704）───────────────
: > "$AUTH_EV" 2>/dev/null || true
gh_get "/repos/$REPO/releases/tags/$TAG" >/dev/null 2>&1 || true
AUTH_NOW="$(auth_state)"
say "AUTH-PROBE state=$AUTH_NOW"
case "$AUTH_NOW" in
  PAT-INVALID*)
    say "WARN $PAT_FILE 已失效（GitHub 401/403）⇒ 降级匿名；匿名限流 60/h，自动放宽轮询"
    if [ -z "${WATCH_ROUNDS:-}" ] && [ -z "${WATCH_MAIN_SLEEP:-}" ]; then
      ROUNDS=12; MAIN_SLEEP=300
      say "ADAPT rounds=$ROUNDS gap=${MAIN_SLEEP}s（匿名档 ≈60min 窗口 · API 调用 ≤30 次/h）"
    fi ;;
esac

say "=== watch start ms=$N tag=$TAG mirror=$MIRROR —— 判据含镜像根(663)·GRACE 复核(699)·auth=$AUTH_NOW ==="
start=$(date +%s)
a=""; t=""; l=""; m=""; n9=""; n7=""
for phase in main grace; do
    if [ "$phase" = main ]; then cnt="$ROUNDS"; gap="$MAIN_SLEEP"; else cnt="$GRACE"; gap="$GRACE_SLEEP"; fi
    i=1
    while [ "$i" -le "$cnt" ]; do
        read -r a t l m n9 n7 <<< "$(probe)"
        say "$phase=$phase try=$i assets=$a tgz=$t latest=$l mirror_tag=$m el9=$n9 el7=$n7"
        if [ "$(judge "$a" "$t" "$l" "$m" "$n9" "$n7")" = OK ]; then
            waited=$(( $(date +%s) - start ))
            if [ "$phase" = main ]; then
                say "VERDICT=OK assets=$a tgz=$t latest=$l mirror_tag=$m el9=$n9 el7=$n7 waited=$((waited/60))min"
                notify_good OK "$waited"; exit 0
            else
                say "VERDICT=GRACE-OK assets=$a tgz=$t latest=$l mirror_tag=$m el9=$n9 el7=$n7 waited=$((waited/60))min（699：窗口外几秒级的滞后不挂 p1）"
                notify_good GRACE-OK "$waited"; exit 0
            fi
        fi
        i=$((i+1))
        [ "$i" -le "$cnt" ] && sleep "$gap"
    done
done
waited=$(( $(date +%s) - start ))
say "VERDICT=TIMEOUT assets=${a:-?} tgz=${t:-0} latest=${l:-?} mirror_tag=${m:-?} el9=${n9:-?} el7=${n7:-?} waited=$((waited/60))min（主窗口 ${ROUNDS} 轮 + 复核 ${GRACE} 轮均不齐）"
if [ "$DO_NOTIFY" = 1 ]; then
    DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "m${N}-release-timeout" "M${N} 发布超时" \
        "$TAG 主窗口 ${ROUNDS} 轮（${MAIN_SLEEP}s/轮）+ 复核 ${GRACE} 轮（${GRACE_SLEEP}s/轮）均未齐
assets=${a:-?} tgz=${t:-0} latest=${l:-?} mirror_tag=${m:-?} el9=${n9:-?} el7=${n7:-?}
证据 $LOG" >> "$LOG" 2>&1 || true
fi
echo "WATCH-TIMEOUT" >> "$LOG"
exit 1
