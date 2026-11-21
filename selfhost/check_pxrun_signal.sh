#!/usr/bin/env bash
# ============================================================
# selfhost/check_pxrun_signal.sh —— px run 的信号与进程树（M429 · 缺陷 878）
# ------------------------------------------------------------
# 为什么有它（2026-11-21 15:50 实测 · /tmp/m429）：
#   用户按示例起服务的标准写法是 `px run srv.px`（或 os_spawn("/bin/sh",["-c","exec px run …"])），
#   停服务时 kill 掉 px 的 pid。修前形状：
#     px(3251927) ──child──> pxi(3251933) LISTEN :18931
#     kill -TERM 3251927 ⇒ px 退出、**pxi 照旧 LISTEN**：
#       端口永不释放（下一次起服务 EADDRINUSE）、进程永久孤儿（PPID=1）
#   实测同类残留：M427 那批探测服务 15 个挂 7 小时、m30_tls_pool 每跑一次漏一个 18096、
#   一个 12 天 10 小时的 m312 残留 —— 都是「杀父留孙」同一形状。
#   根因不是示例作者粗心：px 的**默认解释轨**必须 fork pxi（M271 的发现性提示要在
#   子进程退出后打印耗时，`exec` 之后就没机会说话了），却从来没把信号转给子进程。
#
# 判据（每条都要能判红）：
#   S1 形状：cmd_run 里 4 个信号（INT/TERM/HUP/QUIT）都装 trap 转发 + 有 wait 循环
#            + 有 KILL 兜底；且**不得**改成 exec（那会丢 M271 提示 ⇒ 由 S2 反证）
#   S2 形状：提示条件必须排除「被信号收走」那一路（_sig = 0）
#   P1 主判据（真跑）：起 px run 服务 → 等 LISTEN → **真实 GET 200**（前提自证）→
#      kill -TERM px → 3 秒内 pxi 消失 **且** 端口释放
#   P2 正控（防过度修复）：正常脚本的退出码仍透传（失败脚本 rc=1 / 成功 rc=0），
#      stdout 只有程序输出（不含提示行）
#   P3 负控：SIGINT 同样要转发（示例里 Ctrl-C 与 kill 是两条路）
#   P4 回归：M271 提示仍工作（PX_RUN_HINT=1 强制 ⇒ 有；非 TTY 未强制 ⇒ 无）
#   P5 回归：--fast 命中缓存那条路本来就是 exec（信号直达），本轮不得把它改坏
#   P6 负控自证：把 tools/px 的 4 条 trap 打掉（仓外副本）⇒ 同一场景必须**复现孤儿**
#      且前提自证成立（打掉后服务照样起得来、GET 200）—— 证明 P1 的红来自转发，不是别处
#   G1 环境守卫：tools/px / bootstrap/pxi / curl / ss 不可用 ⇒ rc=3（绝不降级成 0）
#   G2 零污染：全程在 $TMP 里跑，仓库 git status 行数前后一致（M426 缺陷 873 的教训）
#   G3 收尾：本门自身跑完不许留监听、不许留 pxi（判据自己也守同一条契约）
#
# 用法：check_pxrun_signal.sh             # 全跑（约 30~60s）
#       check_pxrun_signal.sh --self-test # 只跑判据夹具（离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
PXI="$ROOT/bootstrap/pxi"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m429_pxrun.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

# 本门自己起的进程（任何退出路径都要收走 —— G3）
KIDS=()
PORT=""
cleanup() {
    local p
    for p in "${KIDS[@]:-}"; do
        [ -n "$p" ] && kill -KILL "$p" 2>/dev/null
    done
    chmod -R u+w "$TMP" 2>/dev/null
    rm -rf "$TMP"
}
trap cleanup EXIT

# ── 判据纯函数（可自测 · 不跑任何进程）────────────────────────
shape_traps() {   # $1=tools/px 文本 ⇒ 打印缺失项（空 = 形状成立）
    local t="$1" s
    for s in TERM INT HUP QUIT; do
        printf '%s\n' "$t" | grep -q "trap '_px_fwd $s'" || echo "缺 trap $s"
    done
    printf '%s\n' "$t" | grep -q 'while :; do' || echo "缺 wait 循环"
    printf '%s\n' "$t" | grep -q 'kill -KILL "\$_child"' || echo "缺 KILL 兜底"
    printf '%s\n' "$t" | grep -q 'wait "\$_child"' || echo "缺 wait 子进程"
}
shape_no_exec() { # $1=文本 ⇒ 0 表示没有把解释轨改成 exec（exec 会丢 M271 提示）
    ! printf '%s\n' "$1" | grep -qE '^[[:space:]]*exec "\$PXI_BIN"'
}
shape_hint_guard() { # $1=文本 ⇒ 0 表示提示条件排除了被信号收走那一路
    printf '%s\n' "$1" | grep -q '\[ "\$_sig" = 0 \].*PX_RUN_HINT_SEC'
}

# ── 环境守卫（G1）────────────────────────────────────────────
env_ok() {
    [ -x "$PX" ]        || { echo "ENV: tools/px 不可执行"; return 1; }
    [ -x "$PXI" ]       || { echo "ENV: bootstrap/pxi 不可执行"; return 1; }
    command -v curl >/dev/null || { echo "ENV: 缺 curl"; return 1; }
    command -v ss   >/dev/null || { echo "ENV: 缺 ss"; return 1; }
    return 0
}

pick_port() {  # 找一个当前空闲的高位端口
    local p
    for p in $(shuf -i 21000-24000 -n 8); do
        ss -ltn 2>/dev/null | grep -q ":$p " || { echo "$p"; return 0; }
    done
    return 1
}

listen_has() { ss -ltn 2>/dev/null | grep -q ":$1 "; }
pid_of_listener() { ss -ltnp 2>/dev/null | grep ":$1 " | grep -o 'pid=[0-9]*' | head -1 | cut -d= -f2; }

# 起一个 px_serve 服务（用给定的 px 入口）⇒ 设置全局 SRV_PID
#   ⚠️ 不能用 `P="$(start_srv …)"`：命令替换会把后台任务挂到**子 shell** 名下，
#      主 shell 再 wait 就拿到 127（本轮第一版踩的假红 —— 判据假红比缺陷更该先修）。
start_srv() {   # $1=px 入口 · $2=端口(未用，留位) · $3=脚本路径
    "$1" run "$3" >"$TMP/srv.log" 2>&1 &
    SRV_PID=$!
}

wait_ready() {  # $1=端口 · $2=秒 ⇒ 0 表示 LISTEN + GET 200（前提自证：进程活着 ≠ 服务对）
    local i=0
    while [ "$i" -lt $(( $2 * 10 )) ]; do
        if listen_has "$1" && [ "$(curl -s -o /dev/null -m 2 -w '%{http_code}' "http://127.0.0.1:$1/" 2>/dev/null)" = 200 ]; then
            return 0
        fi
        i=$((i+1)); sleep 0.1
    done
    return 1
}

wait_gone() {   # $1=端口 · $2=秒 ⇒ 0 表示端口释放 **且** 监听者消失
    local i=0
    while [ "$i" -lt $(( $2 * 10 )) ]; do
        listen_has "$1" || return 0
        i=$((i+1)); sleep 0.1
    done
    return 1
}

write_srv() {   # $1=脚本路径 · $2=端口 · $3=docroot
    printf 'srv = px_serve(%s, "%s", 3000)\nprint("SRV-UP")\n' "$2" "$3" > "$1"
}

# ── 自测（离线 · 只验判据有没有牙）──────────────────────────
self_test() {
    local st=0
    local good; good="$(cat <<'FIX'
    trap '_px_fwd TERM' TERM
    trap '_px_fwd INT'  INT
    trap '_px_fwd HUP'  HUP
    trap '_px_fwd QUIT' QUIT
    while :; do
        wait "$_child"; _rc=$?
        kill -KILL "$_child" 2>/dev/null
    done
FIX
)"
    local sh; sh="$(shape_traps "$good")"
    if [ -z "$sh" ]; then ok "自测 S1 正控：完整形状被判成立"; else bad "自测 S1 正控误红：$sh"; st=1; fi

    local miss; miss="$(shape_traps "$(printf '%s\n' "$good" | grep -v 'TERM')")"
    case "$miss" in *"缺 trap TERM"*) ok "自测 S1 负控：少一条 trap ⇒ 必红" ;;
        *) bad "自测 S1 负控没牙（缺 trap TERM 未被抓到）"; st=1 ;; esac

    local empty; empty="$(shape_traps "")"
    [ -n "$empty" ] && ok "自测 S1 空文本：判红（不是空过）" || { bad "自测 S1 对空文本判成立"; st=1; }

    if shape_no_exec 'exec "$PXI_BIN" "$@" "$file"'; then
        bad "自测 S2 负控没牙：exec 仍被判成立（会丢 M271 提示）"; st=1
    else
        ok "自测 S2 负控：改成 exec ⇒ 必红"
    fi
    shape_no_exec '    "$PXI_BIN" "$@" "$file" &' && ok "自测 S2 正控：fork 形态被判成立" || { bad "自测 S2 正控误红"; st=1; }

    shape_hint_guard '    if [ "$_sig" = 0 ] && [ "$_el" -ge "${PX_RUN_HINT_SEC:-2}" ]; then' \
        && ok "自测 S2b 正控：提示条件带 _sig 守卫" || { bad "自测 S2b 正控误红"; st=1; }
    shape_hint_guard '    if [ "$_el" -ge "${PX_RUN_HINT_SEC:-2}" ]; then' \
        && { bad "自测 S2b 负控没牙：无 _sig 守卫仍判成立"; st=1; } || ok "自测 S2b 负控：缺守卫 ⇒ 必红"

    return $st
}

if [ "${1:-}" = "--self-test" ]; then
    echo "=== m429_pxrun_signal 判据自测 ==="
    if self_test; then echo "SELF-TEST OK"; exit 0; else echo "SELF-TEST FAILED"; exit 2; fi
fi

# ── 开跑 ────────────────────────────────────────────────────
echo "=== m429_pxrun_signal：px run 的信号与进程树（缺陷 878）==="
env_ok || { echo "ENV-GUARD rc=3"; exit 3; }

GIT0=0
if git -C "$ROOT" rev-parse --git-dir >/dev/null 2>&1; then
    GIT0="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"
fi

# ── S1/S2 形状 ──────────────────────────────────────────────
SRC="$(cat "$PX")"
_cmd_run="$(printf '%s\n' "$SRC" | sed -n '/^cmd_run() {/,/^}/p')"
sh_miss="$(shape_traps "$_cmd_run")"
if [ -z "$sh_miss" ]; then ok "S1 形状：4 信号转发 + wait 循环 + KILL 兜底齐备"
else bad "S1 形状缺失：$(echo "$sh_miss" | tr '\n' ' ')"; fi
shape_no_exec "$_cmd_run" && ok "S1b 形状：解释轨仍是 fork（没被改成 exec）" || bad "S1b 解释轨被改成 exec（会丢 M271 提示）"
shape_hint_guard "$_cmd_run" && ok "S2 形状：提示条件排除被信号收走那一路" || bad "S2 提示条件缺 _sig 守卫"

# ── 夹具与素材 ──────────────────────────────────────────────
mkdir -p "$TMP/site"
echo '<h1>m429</h1>' > "$TMP/site/index.html"
PORT="$(pick_port)" || { echo "ENV: 找不到空闲端口 rc=3"; exit 3; }
SRV="$TMP/srv.px"; write_srv "$SRV" "$PORT" "$TMP/site"

FAILS="$TMP/fail.px"; printf 'print("A")\nassert(1 == 2)\n' > "$FAILS"
OKS="$TMP/ok.px";    printf 'print("OUT-ONLY")\n' > "$OKS"

# ── P1 主判据 ───────────────────────────────────────────────
start_srv "$PX" "$PORT" "$SRV"; P=$SRV_PID; KIDS+=("$P")
if wait_ready "$PORT" 6; then
    ok "P1 前提自证：服务真起来了（LISTEN + 真实 GET 200）"
    XPID="$(pid_of_listener "$PORT")"
    kill -TERM "$P" 2>/dev/null
    # 先收尸再轮询：顺序反了会让 wait 撞上「已 reap ⇒ 127」（本轮第一版就踩了这个假红）。
    # 看门狗兜住「不转发就挂死」的情形：8 秒不退 ⇒ KILL，rc=137 自然判红。
    ( sleep 8; kill -KILL "$P" 2>/dev/null ) & local_wd=$!
    wait "$P" 2>/dev/null; PRC=$?
    kill "$local_wd" 2>/dev/null
    if wait_gone "$PORT" 3; then
        ok "P1 主判据：kill -TERM px ⇒ 端口已释放"
    else
        bad "P1 主判据：kill -TERM px 后端口仍被占（孤儿 pxi pid=$XPID）"
        [ -n "$XPID" ] && KIDS+=("$XPID")
    fi
    if [ -n "$XPID" ] && kill -0 "$XPID" 2>/dev/null; then
        bad "P1b pxi 仍存活（pid=$XPID）—— 杀父留孙"
    else
        ok "P1b pxi 已消失"
    fi
    case "$PRC" in
        127) bad "P1c px 退出码 127（wait 失败 ⇒ 进程树异常）" ;;
        137) bad "P1c px 8 秒未退（被看门狗 KILL：转发没生效或子进程不响应）" ;;
        *)   ok "P1c px 退出码 = 子进程退出码（$PRC）" ;;
    esac
else
    bad "P1 前提不成立：服务没起来（不能凭「没起服务」判绿）"
    kill -KILL "$P" 2>/dev/null
fi

# ── P2 正控：退出码透传 + stdout 纯净 ───────────────────────
"$PX" run "$FAILS" >"$TMP/p2.out" 2>"$TMP/p2.err"; RC=$?
[ "$RC" = 1 ] && ok "P2 正控：失败脚本 rc=1 仍透传" || bad "P2 正控：失败脚本 rc=$RC（应为 1）"
if grep -q '提示：' "$TMP/p2.out" "$TMP/p2.err" 2>/dev/null; then
    bad "P2b stdout/stderr 混进提示行（非 TTY 未强制却打了提示）"
else
    ok "P2b 非 TTY 未强制 ⇒ 无提示行"
fi
OUT="$("$PX" run "$OKS" 2>/dev/null)"
[ "$OUT" = "OUT-ONLY" ] && ok "P2c stdout 只含程序输出" || bad "P2c stdout 被污染：[$OUT]"

# ── P3 负控：SIGHUP 也要转发 ────────────────────────────────
#   ⚠️ 为什么不用 SIGINT 做动态负控（本轮第一版踩过）：bash 对**后台异步命令**在启动时
#     就把 SIGINT/SIGQUIT 置为 SIG_IGN，而这种「忽略进入」的 shell 无法再 trap
#     （POSIX/bash 既有语义）⇒ kill -INT 一个后台 px 永远打不到 handler，
#     测出来的是 shell 语义而不是产品。真实 Ctrl-C 由内核发给整个前台进程组，本就都退。
#     HUP 不在这条规则里 ⇒ 用它做动态负控；INT 的转发只做 S1 形状断言。
P3PORT="$(pick_port)" || P3PORT=""
if [ -n "$P3PORT" ]; then
    P3SRV="$TMP/srv3.px"; write_srv "$P3SRV" "$P3PORT" "$TMP/site"
    start_srv "$PX" "$P3PORT" "$P3SRV"; P=$SRV_PID; KIDS+=("$P")
    if wait_ready "$P3PORT" 6; then
        kill -HUP "$P" 2>/dev/null
        ( sleep 8; kill -KILL "$P" 2>/dev/null ) & local_wd=$!
        wait "$P" 2>/dev/null; PRC3=$?
        kill "$local_wd" 2>/dev/null
        if wait_gone "$P3PORT" 3; then ok "P3 SIGHUP：端口已释放"
        else bad "P3 SIGHUP：端口仍被占（HUP 未转发）"; X3="$(pid_of_listener "$P3PORT")"; [ -n "$X3" ] && KIDS+=("$X3"); fi
        [ "$PRC3" = 137 ] && bad "P3b px 8 秒未退（看门狗 KILL：HUP 转发没生效）" \
            || ok "P3b px 及时退出（rc=$PRC3）"
    else
        bad "P3 前提不成立：服务没起来"
        kill -KILL "$P" 2>/dev/null
    fi
fi

# ── P4 M271 提示回归 ────────────────────────────────────────
SLOW="$TMP/slow.px"; printf 'sleep(1500)\nprint("S")\n' > "$SLOW"
PX_RUN_HINT=1 PX_RUN_HINT_SEC=1 "$PX" run "$SLOW" >/dev/null 2>"$TMP/p4.err"
grep -q '提示：' "$TMP/p4.err" && ok "P4 回归：PX_RUN_HINT=1 仍能打到提示" || bad "P4 回归：强制提示丢失（M271 能力被本轮改坏）"

# ── P5 --fast 路径（本来 exec，信号直达）──────────────────
if [ "${PX_RUN_NOCACHE:-0}" != 1 ]; then
    P5PORT="$(pick_port)" || P5PORT=""
    if [ -n "$P5PORT" ]; then
        P5SRV="$TMP/srv5.px"; write_srv "$P5SRV" "$P5PORT" "$TMP/site"
        # 先热一次缓存（首次编译 ~1s），再起服务
        timeout 20 "$PX" run --fast "$OKS" >/dev/null 2>&1
        "$PX" run --fast "$P5SRV" >"$TMP/srv5.log" 2>&1 &
        P=$!; KIDS+=("$P")
        if wait_ready "$P5PORT" 25; then
            kill -TERM "$P" 2>/dev/null
            if wait_gone "$P5PORT" 3; then ok "P5 --fast：信号直达产物，端口释放"
            else bad "P5 --fast：端口仍被占"; X5="$(pid_of_listener "$P5PORT")"; [ -n "$X5" ] && KIDS+=("$X5"); fi
            wait "$P" 2>/dev/null
        else
            bad "P5 前提不成立：--fast 没起来（不许当绿）"
            kill -KILL "$P" 2>/dev/null
        fi
    fi
fi

# ── P6 负控自证：打掉转发 ⇒ 必须复现孤儿 ────────────────────
NEGROOT="$TMP/negroot"
mkdir -p "$NEGROOT/tools"
for d in bootstrap runtime stdlib; do
    [ -d "$ROOT/$d" ] && ln -s "$ROOT/$d" "$NEGROOT/$d"
done
cp "$ROOT/VERSION" "$NEGROOT/VERSION" 2>/dev/null || true
# 只打掉「转发」这一件事：4 条 trap 变 no-op，其余原样
sed "s/^    trap '_px_fwd \(TERM\|INT\|HUP\|QUIT\)' \([A-Z]*\)$/    : # patched \2/" "$PX" > "$NEGROOT/tools/px"
chmod +x "$NEGROOT/tools/px"
if [ -z "$(shape_traps "$(sed -n '/^cmd_run() {/,/^}/p' "$NEGROOT/tools/px")" | grep '缺 trap')" ]; then
    bad "P6 负控构造失败：补丁没打掉 trap（判据会空转）"
else
    P6PORT="$(pick_port)" || P6PORT=""
    if [ -n "$P6PORT" ]; then
        P6SRV="$TMP/srv6.px"; write_srv "$P6SRV" "$P6PORT" "$TMP/site"
        "$NEGROOT/tools/px" run "$P6SRV" >"$TMP/p6.log" 2>&1 &
        P=$!; KIDS+=("$P")
        if wait_ready "$P6PORT" 8; then
            ok "P6 前提自证：打掉 trap 后服务照样起得来（GET 200）"
            X6="$(pid_of_listener "$P6PORT")"
            kill -TERM "$P" 2>/dev/null
            sleep 2
            if listen_has "$P6PORT"; then
                ok "P6 负控：打掉转发 ⇒ 孤儿复现（证明 P1 的红来自这条转发）"
            else
                bad "P6 负控没牙：打掉转发后端口仍被释放（P1 可能靠别的东西过）"
            fi
            [ -n "$X6" ] && { KIDS+=("$X6"); kill -KILL "$X6" 2>/dev/null; }
            wait "$P" 2>/dev/null
        else
            bad "P6 前提不成立：打掉 trap 后服务起不来（负控无效，不能判绿）"
            kill -KILL "$P" 2>/dev/null
        fi
    fi
fi

# ── G2 零污染 ──────────────────────────────────────────────
if git -C "$ROOT" rev-parse --git-dir >/dev/null 2>&1; then
    GIT1="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"
    [ "$GIT0" = "$GIT1" ] && ok "G2 零污染：git status 行数 $GIT0 → $GIT1" \
        || bad "G2 零污染被破坏：git status $GIT0 → $GIT1"
fi

# ── G3 本门收尾（判据自己也守同一条契约）──────────────────
LEFT=0
for p in "$PORT" "${P3PORT:-}" "${P5PORT:-}" "${P6PORT:-}"; do
    [ -n "$p" ] && listen_has "$p" && LEFT=$((LEFT+1))
done
[ "$LEFT" = 0 ] && ok "G3 收尾：本门没留任何监听" || bad "G3 收尾：仍有 $LEFT 个端口被占"

echo
echo "=== 汇总：PASS $PASS / FAIL $FAIL ==="
[ "$FAIL" = 0 ] || exit 1
exit 0
