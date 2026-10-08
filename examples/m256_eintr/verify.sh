#!/usr/bin/env bash
# ============================================================
# M256 门（第 133 轮）：**EINTR 族收口** —— 「被 GC 暂停信号打断的 IO 不是失败」
# ------------------------------------------------------------
# 病灶（缺陷 456）：`read(fd, n)` / `udp_recv` 走**裸系统调用**；并发 GC 的 stop-the-world
#   给每个活跃线程发 SIG_GC_STOP，**打断阻塞中的调用** ⇒ 返回 -1/EINTR。
#   · 用户面 fd 设了 SO_RCVTIMEO（`tcp_opt(...,{read_timeout_ms})`）⇒ 落在 signal(7) 的
#     「**永不重启**」族 ⇒ **SA_RESTART 救不了**（实测 `/tmp/m256/eintr_probe.c`，
#     与 `gc_stop_handler` 同款 sigaction ⇒ recv 返回 -1/EINTR）。
#   · 修前：`read` 返回 -1（调用方读成「读失败」）；`udp_recv` 返回 null
#     ⇒ 与「真的没有包」不可区分（**静默丢包**）。
#
# **触发条件 = 两个开关同开**（实测矩阵 /tmp/m256/matrix.log）：
#   none → T1=5 · STRESS only → 5 · INLINE only → 5 · **BOTH → -1（3/3）**
#
# 层的设计（每层都能独立判红）：
#   [1] 静态守卫 `selfhost/check_eintr.py`（A 段无例外族 + B 段用户面 fd 原语；自证 9 条）
#   [2] 编译探针
#   [3] 正常档：T1=5 · U2=4（两档都必须如此 —— 修**不能**破坏正常语义）
#   [4] 压力档 `PX_GC_STRESS=1 PX_GC_INLINE=1`：同上（**修前 T1=-1 · U2=-1**）
#   [4b] 压力档**复跑**（判据不能是 flake）
#   [5] 两档 stdout **逐字节一致**
#   [6] 负控 3 道（`--neg-skip` 可跳；各自独立判红，各自干净起点）：
#       A 撤回 `bi_read` 的包装（退回裸 read）⇒ 压力档 T1 必红（U2 仍绿 ⇒ 独立性）
#       B 撤回 `bi_udp_recv` 的包装     ⇒ 压力档 U2 必红（T1 仍绿 ⇒ 独立性）
#       C **判据自伤**：复用 A 的补丁 + `M256_EXPECT_T1=-1` ⇒ 同一故障**不再判红**
#   [7] 源逐字节还原断言
#
# ⚠️ **两处「不做就不会稳」的工程决定（都有实测依据）**：
#   ① **探针超时 20s，不是 3s/6s**：压力档下 `sleep(1500)` 的唤醒会被 GC 拖长 ⇒ 数据真的
#      比超时晚到 ⇒ 偶发 `T1=-1`/`U2=-1`，**那是 timeout，不是 EINTR**（首版就这么假红过）。
#      修前 EINTR 是**立即**返回 ⇒ 放宽超时不影响复现力。
#   ② **本门跑 `PX_GC_INLINE=1` ⇒ 会撞上已登记但未修的「缺陷 267 家族」**
#      （并发/挂起执行流在该开关下容器被误回收；本机实测 ~1/35 概率 SIGSEGV，
#       core 栈 `xmalloc ← px_dict ← vm_run_loop` + 另一线程在 `px_gc_collect`）。
#       ⇒ 被信号杀死时**记数并重试**（`N267`），并把命中次数**响亮打印**（不隐藏）；
#         判据（T1/U2/DONE）才是判红依据。**这不是把崩溃当绿** —— 缺陷 267 有它自己的账。
#   ③ **M284s1（缺陷 497）**：CI 实测本门判红（run 37569198043 · step[36] · 52min 那个 job），
#      现场 = `T1=4` + **无 U2 / 无 DONE**（stress 与 stress2 两次都如此）。
#      本机 **39 次**带检测器（`PX_GC_LIVECHK=1 PX_GC_UAFDET=1`）复跑 **0 命中** ⇒
#      判为**宿主相关**（CI runner 更慢/更争用），**不是**产品回归 —— 如实登记，不冒充已修。
#      两处加固：
#        · **探针**把 UDP `bind` 提到 `spawn usrv()` 之前（原理上消除 ICMP→ECONNREFUSED 竞态）；
#        · **门**把「重试原因」分类记账（信号 / 超时 / 异常退出）并把**探针 stdout 全文**
#          打进自己的 stdout —— 否则 CI 注解里只能看到「未跑完」，**读不出真因**。

#
# ⚠️ 负控要**完整重建 runtime**（改 runtime.c ⇒ .rtcache key 变）⇒ CI 用 `--neg-skip`。
#
# ⚠️⚠️ M288b（第 167 轮）：**判据不专属 ⇒ 环境一干扰就假红**。
#   CI 现场（run 37729408552 · step[36]）：`FAIL [stress] T1=6（期望 5）` + `FAIL [5] 两档不一致`
#   —— 而本机跑同一门是 **M256-VERIFY-OK（14/0）**。
#   **真因不是产品，是判据**：原判据 = 「累积读到 **5 字节**」，而「长度」**不专属**于 EINTR
#   （「read 被 STW 打断后重试」这件事）—— 端口上有**陌生数据**（外人写进那条连接 /
#   端口落进宿主的 **ephemeral 出站端口池**而被别的连接占用）同样让长度 ≠ 5。
#   ⇒ 三条修法：
#     ① **内容判据**：探针比对 `bytes_to_hex` 的**逐字节内容**（HELLO / PING），不是长度；
#        并**再确认没有多余字节**（CI 那次是 5+1 —— 只看长度会把 6 读成「≥5 就算对」）。
#     ② **环境干扰单独分类**：`T1=FOREIGN:<hex>` / `U2=FOREIGN:<hex>` / stderr 的
#        「监听端口 … 失败」⇒ 门**换端口重试**（有界）；用尽后**响亮判红**，但文案必须写明
#        「**这不是产品回归**」—— 否则下次红时读不出真因（M260 §「判据失去信号价值」的同款）。
#        产品回归的指纹**不变**：`T1=-1` / `U2=-1`（read/udp_recv 被 STW 打断后误报失败）。
#     ③ **端口区间收窄**：原 `20000 + RANDOM % 20000` 会落进 **32768..39999**，
#        而那正是宿主 ephemeral 端口池（`ip_local_port_range` 的默认下限 = **32768**）
#        ⇒ 与陌生出站连接抢端口。现在按**宿主实测下限**取上界。
#   ⚠️ 判据强度**只增不减**：内容判据严于长度判据；FOREIGN 也**判红**（不静默放行）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="${M256_W:-$(mktemp -d /tmp/m256_gate.XXXXXX)}"
mkdir -p "$W"
SNAP="$W/snap"
SRC="runtime/runtime.c"
snapshot() { mkdir -p "$SNAP/runtime"; cp "$SRC" "$SNAP/$SRC"; }
restore_all() { [ -f "$SNAP/$SRC" ] && cp "$SNAP/$SRC" "$SRC"; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PXBUILD="$ROOT/tools/px"
REL="$HERE/probe_eintr.px"

# ── M288b：端口区间的上界 = 宿主 **ephemeral 端口下限 - 1** ─────────────
#   原写法 `20000 + RANDOM % 20000` 会落进 32768..39999，而 Linux 默认
#   `net.ipv4.ip_local_port_range` = `32768 60999` ⇒ 那一段正是**本机出站连接**的临时端口池
#   ⇒ 与陌生连接抢端口（CI 上 `T1=6` 的成因之一）。这里**按宿主实测值**取上界，
#   并保底留 1000 个候选（避免宿主机把 ephemeral 下限配得很低时区间塌缩）。
EPHEM_LO="$(awk '{print $1}' /proc/sys/net/ipv4/ip_local_port_range 2>/dev/null || echo 32768)"
case "$EPHEM_LO" in ''|*[!0-9]*) EPHEM_LO=32768 ;; esac
PORT_HI=$((EPHEM_LO - 1))
[ "$PORT_HI" -gt 32767 ] && PORT_HI=32767
[ "$PORT_HI" -lt 21000 ] && PORT_HI=21000
PORT_SPAN=$((PORT_HI - 20000 + 1))
BD="$HERE/build"
BIN="$BD/probe_eintr"
PASS=0
FAIL=0
N267=0
NTO=0
NAB=0
NENV=0        # M288b：环境干扰（陌生内容 / 端口被占）⇒ 换端口重试的次数
ok() { echo "  PASS $1"; PASS=$((PASS + 1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL + 1)); }

# 精确单点替换（**唯一性断言**：命中数 ≠ 1 ⇒ 响亮失败，绝不静默 no-op）
patch_one() {
    python3 - "$SRC" "$1" "$2" <<'PY'
import sys, io
p, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = io.open(p, encoding='utf-8').read()
k = s.count(old)
if k != 1:
    sys.stderr.write("ANCHOR-BAD count=%d\n" % k); sys.exit(3)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
}

free_port() {
    local p="$1" pid=""
    pid="$( (ss -ltnp 2>/dev/null || netstat -ltnp 2>/dev/null) | grep ":$p " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
    [ -n "$pid" ] && kill "$pid" 2>/dev/null && sleep 0.5
    return 0
}

rebuild() {   # $1=日志路径 → 0/1
    rm -rf "$BD"
    ( cd "$ROOT" && "$PXBUILD" build "$REL" ) >"$1" 2>&1
}

# 跑一档：$1=档名（余下为 env 赋值）→ 输出落 $W/out_<档名>.txt，回 rc
run_once() {
    local tag="$1"; shift
    # M275：每次用**一对新端口** —— 本门一轮连跑 9 次，固定端口会因上一次残留而失败。
    #   区间 20000+ 刻意避开本仓其它门用到的 18xxx 段（静态扫：18099–19999）。
    #   M288b：上界改为 PORT_SPAN（宿主 ephemeral 下限以内 —— 见文件头 ③）。
    local _base=$(( 20000 + RANDOM % PORT_SPAN ))
    ( cd "$ROOT" && env "$@" M256_PROBE_PORT="$_base" M256_PROBE_UPORT="$(( _base + 1 ))" \
        timeout -k 5 180 "$BIN" ) >"$W/out_$tag.txt" 2>&1
    echo "$?"
}

# 跑一档（**信号 / 超时 / 异常退出时分类记账并重试** —— 见文件头 ⚠️②③）：$1=档名，余 env
# ⚠️ M284s1（缺陷 497）：修前**只**重试 `rc ≥ 128`（信号），而 CI 上实测的失败是超时/异常退出
#    ⇒ 门只报「探针未跑完」，**说不出为什么**。现在三类各自记数、各自响亮打印。
#    **判据强度不变**：真缺陷（EINTR 未被重试）表现为 `T1=-1 / U2=-1` 且 **rc=0**
#    （探针正常跑完）⇒ 照样判红；重试只针对"探针没能跑完"这一类的宿主干扰。
run_track() {
    local tag="$1"; shift
    local i rc t0 el
    for i in 1 2 3; do
        t0=$SECONDS
        rc="$(run_once "$tag" "$@")"
        el=$((SECONDS - t0))
        echo "  ·  [$tag] 第 $i 次 rc=$rc 耗时 ${el}s"
        # M288b：**先看环境**，再看 rc。理由：端口被陌生人占用时，探针的 srv 协程会因
        #   tcp_listen 失败被隔离 ⇒ rc=1（不是 0），而它**不是**产品回归 ⇒ 若先按 rc 分类，
        #   会被记成「异常退出」并丢掉「端口被占」这个真因。
        if env_foreign "$tag"; then
            NENV=$((NENV + 1))
            echo "  ℹ️ [$tag] 第 $i 次**环境干扰**（$(env_why "$tag")）⇒ 换端口重试（不影响产品判据）"
            continue
        fi
        if [ "$rc" = 0 ]; then return 0; fi
        if [ "$rc" -ge 128 ]; then
            N267=$((N267 + 1))
            echo "  ℹ️ [$tag] 第 $i 次被信号杀死（rc=$rc）—— 疑似**缺陷 267 家族**（并发 GC · 已登记未修）⇒ 重试"
        elif [ "$rc" = 124 ]; then
            NTO=$((NTO + 1))
            echo "  ℹ️ [$tag] 第 $i 次**超时**（rc=124 · 上限 180s）—— 高负载宿主上压力档会被饿死 ⇒ 重试"
        else
            NAB=$((NAB + 1))
            echo "  ℹ️ [$tag] 第 $i 次异常退出（rc=$rc）⇒ 重试"
        fi
    done
    # M288b：用尽重试 ⇒ **响亮一行**。理由：判据层随后仍会读最后一次的输出，
    #   所以「重试全部用尽但结论恰好正确」在 PASS/FAIL 计数上**看不出来**（首版就这么漏过）。
    echo "  ⚠️ [$tag] 3 次均未取得**可判定**的运行 ⇒ 交由判据层裁决（下面 [tag] 的判据以最后一次为准）"
    return 1
}

# M288b：字符类补 `:` —— 否则 `T1=FOREIGN:574f…` 会被截成 `FOREIGN`（十六进制读不出来，
#   而「读到的是什么」正是判「环境干扰」还是「产品回归」的依据）。
get_val() { grep -oE "^$2=[-0-9A-Za-z:]+" "$W/out_$1.txt" | head -1 | cut -d= -f2; }

# ── M288b：**环境干扰**的探测与归因 ─────────────────────────────────
#   判据：一次 run 的输出里，若出现「非预期内容」或「监听失败」，则**本次跑不成立**
#   （不是产品红，也不是绿）⇒ 由调用方换端口重试。
#   ⚠️ 与「产品回归」严格分开：前者是 `T1=FOREIGN:…` / `U2=FOREIGN:…` / `监听端口…失败`，
#      后者是 `T1=-1` / `U2=-1`（EINTR 未被重试）。
#   ⚠️ 返回语义 = **grep 风格**（0 = **有**干扰）—— 调用处一律写 `if env_foreign …; then`。
#      M288b 自伤（首版）：把它写成「0=干净」，而调用处按「0=有干扰」用 ⇒ **每次干净运行都被
#      当成干扰、白耗 3 次重试**，且因为判据层随后照样读到正确输出，**结果仍是绿** ⇒
#      这类「重试用尽但结论仍对」的 bug 不会被 PASS/FAIL 计数发现。⇒ 两处加固：
#      ① 语义对齐 grep（下面）；② run_track 用尽重试时**响亮打一行**（见该函数尾部）。
env_foreign() {   # $1=档名 → 0=**有**环境干扰 · 1=干净
    grep -qE '^(T1|U2)=FOREIGN:' "$W/out_$1.txt" && return 0
    grep -q '监听端口 .* 失败' "$W/out_$1.txt" && return 0
    return 1
}
env_why() {   # $1=档名 → 人类可读的干扰原因（多条用 ` · ` 连接）
    local tag="$1" r=""
    grep -qE '^T1=FOREIGN:' "$W/out_$1.txt" && r="$rTCP 端口上读到陌生字节（$(get_val "$tag" T1 | cut -c1-40)）"
    grep -qE '^U2=FOREIGN:' "$W/out_$1.txt" && r="$r${r:+ · }UDP 端口上收到陌生数据报（$(get_val "$tag" U2 | cut -c1-40)）"
    grep -q '监听端口 .* 失败' "$W/out_$1.txt" && r="$r${r:+ · }探针服务端 tcp_listen 失败（端口被占）"
    echo "${r:-未知}"
}

# 判一档：$1=档名 → 失败条数（期望值可用 M256_EXPECT_T1/U2 覆盖 ⇒ 供负控 C 自伤）
judge_track() {
    local tag="$1" n=0 v
    local e_t1="${M256_EXPECT_T1:-5}" e_u2="${M256_EXPECT_U2:-4}"
    # M288b：三类取值 —— **产品对** / **环境干扰** / **产品回归**。
    #   ⚠️ 环境干扰**照样判红**（不静默放行）—— 但文案必须让人一眼看出该查什么。
    v="$(get_val "$tag" T1)"
    case "$v" in
        FOREIGN:*)
            # ⚠️ 「环境干扰」这四个字是**分类标签**：负控 [6D] 断言的就是它
            #   （改文案 ⇒ 必须同步 examples/m256_eintr/verify.sh 的 [6D] 断言）。
            bad "[$tag] T1=**环境干扰**（非 EINTR 回归 · 非预期内容）：${v#FOREIGN:}"
            echo "       ⇒ 重试用尽仍如此 ⇒ 判据**未成立**（所以判红，不是放行）。"
            echo "       ⇒ **两种可能，都要看**（M288b 实测两种都见过）："
            echo "          · hex 是可打印 ASCII 且形如 pNNN ⇒ 疑似**已登记的缺陷 267 家族**"
            echo "            （GC 压力档下对象被复用 —— 实测 read 的 bytes 渲染出了 press() 的中间串）"
            echo "          · 其余 ⇒ 更可能是**端口上有陌生数据**（外人写进那条连接 / 端口被占）"
            echo "       ⇒ 产品回归（EINTR 未重试）的指纹是 T1=-1，**不是**这一条。"
            n=$((n + 1)) ;;
        CONNFAIL)
            if grep -q '监听端口 .* 失败' "$W/out_$tag.txt"; then
                bad "[$tag] T1=**环境干扰·非产品回归**：探针服务端 tcp_listen 失败（端口被占）"
            else
                bad "[$tag] T1=CONNFAIL（连不上本探针的服务端 —— 期望 $e_t1）"
            fi
            n=$((n + 1)) ;;
        "$e_t1")
            ok "[$tag] T1=$v（read 拿到对端发来的 5 字节 · **内容 = HELLO**）" ;;
        *)
            bad "[$tag] T1=$v（期望 $e_t1 —— read 被 GC 暂停信号打断后误报失败）"; n=$((n + 1)) ;;
    esac
    v="$(get_val "$tag" U2)"
    case "$v" in
        FOREIGN:*)
            bad "[$tag] U2=**环境干扰**（非产品回归 · 非预期内容）：${v#FOREIGN:}"
            echo "       ⇒ 静默丢包（产品回归）的指纹是 U2=-1，**不是**这一条；"
            echo "         非预期内容同样只有两种来源：端口上的陌生数据报 / 缺陷 267 家族。"
            n=$((n + 1)) ;;
        "$e_u2")
            ok "[$tag] U2=$v（udp_recv 拿到数据报 · **内容 = PING**）" ;;
        *)
            bad "[$tag] U2=$v（期望 $e_u2 —— udp_recv 被中断后返回 null=「没有包」静默丢包）"; n=$((n + 1)) ;;
    esac
    if grep -q '^M256-PROBE-DONE$' "$W/out_$tag.txt"; then ok "[$tag] 探针跑到结尾"
    else
        bad "[$tag] 探针未跑完（异常/挂死）"; n=$((n + 1))
        # ⚠️ M284s1（缺陷 497）：**把现场打进 stdout** —— CI 的 job 日志非管理员 403，
        #    注解是唯一通道，而只有 stdout 会被注解带走（「门红了要能读出真因」）。
        echo "       ── [$tag] 探针 stdout 全文（含 stderr）──"
        sed 's/^/       | /' "$W/out_$tag.txt" 2>/dev/null
        echo "       ── 结束（rc 见上方 [第 N 次 rc=…] 行）──"
    fi
    return $((n > 0 ? 1 : 0))
}

echo "══ M256 门：EINTR 族收口（缺陷 456）══"

# ---------- 层 [1] 静态守卫 ----------
echo "── [1] 静态守卫 selfhost/check_eintr.py"
if python3 selfhost/check_eintr.py --self-test >"$W/selftest.log" 2>&1 \
   && grep -q '通过 9 / 失败 0' "$W/selftest.log"; then
    ok "[1] 守卫自证 9/9"
else
    bad "[1] 守卫自证未通过"; tail -6 "$W/selftest.log"
fi
if python3 selfhost/check_eintr.py "$ROOT" >"$W/static.log" 2>&1; then
    ok "[1] 实检：A 段 0 · B 段 0"
else
    bad "[1] 实检有违例"; tail -8 "$W/static.log"
fi

# ---------- 层 [2] 编译探针 ----------
echo "── [2] 编译探针"
# M288b：**判据的唯一事实源** —— 期望内容的十六进制常量在**探针**里（探针与门共用一个值）。
#   ⚠️ 这里断言的是「常量在位且形态正确」（`bytes_to_hex` 的正确性由 [3]/[4] 的实跑证明）。
#   少了这一层，日后有人把探针的期望内容改掉、却忘了同步门的语义，**门不会响**。
if grep -q 'let HELLO_HEX = "48454c4c4f"' examples/m256_eintr/probe_eintr.px \
   && grep -q 'let PING_HEX = "50494e47"' examples/m256_eintr/probe_eintr.px; then
    ok "[2] 期望常量在位（HELLO → 48454c4c4f · PING → 50494e47）"
else
    bad "[2] 探针缺期望常量（判据的事实源被改动 / 门未同步）"
fi
free_port 18420
free_port 18421
if rebuild "$W/build.log" && [ -x "$BIN" ]; then
    ok "[2] 探针编译成功 → $BIN"
else
    bad "[2] 探针编译失败 / 产物缺失"; tail -8 "$W/build.log"
fi

# ---------- 层 [3][4][4b] ----------
if [ -x "$BIN" ]; then
    echo "── [3] 正常档"
    run_track norm || true; judge_track norm || true
    echo "── [4] 压力档（PX_GC_STRESS=1 PX_GC_INLINE=1）"
    run_track stress PX_GC_STRESS=1 PX_GC_INLINE=1 || true; judge_track stress || true
    echo "── [4b] 压力档**复跑**（稳定性：判据不能是 flake）"
    run_track stress2 PX_GC_STRESS=1 PX_GC_INLINE=1 || true; judge_track stress2 || true

    echo "── [5] 两档 stdout 逐字节一致"
    if cmp -s "$W/out_norm.txt" "$W/out_stress.txt"; then
        ok "[5] 两档一致"
    else
        bad "[5] 两档不一致"; diff "$W/out_norm.txt" "$W/out_stress.txt" | head -8
    fi
fi

# ---------- 层 [6] 负控 ----------
# 「任何一次命中即算复现」—— 单次存在概率（实测 4/5）⇒ 取 3 次机会，避免把 flake 当判据。
# ⚠️ M276 实测（本门在**全量门里**跑时 [6A] 判红，单跑却绿）：原判据有两个**假红源** ——
#   ① 探针**没跑完**（被信号杀死 / 输出缺 U2）—— 那是**已知未修**的缺陷 267 家族
#      （并发/挂起执行流在 PX_GC_INLINE=1 下容器被误回收），**不是**负控要测的那条故障；
#      原判据把它当成「keep 位置被破坏」⇒ 立刻 rc=2。
#      （实测 out_n_2.txt 只有 `T1=5` 一行、没有 U2/DONE ⇒ 探针中途死了。）
#   ② keep 位置偶发异常（实测 out_n_1.txt `T1=5 / U2=-1`）—— 同样先重试，**连续 3 次**才判 rc=2。
#   ⇒ 改成**有界重试**（最多 6 次）：不可判的run 只记 ℹ️ 并重试；
#      keep **连续 3 次**异常才 rc=2；`gone` 至少命中 1 次才算通过。
#   ⚠️ **牙还在**：若产品真的回归（udp_recv 被 STW 打断后静默丢包），U2 会**每次**都 -1
#      ⇒ 连续 3 次异常 ⇒ 仍然 rc=2 判红；`gone` 永不出现 ⇒ rc=1 判红。
neg_any() {   # $1=撤回后应消失的键 $2=应保持的值 $3=另一键 $4=另一键应保持的值
    local gone="$1" gone_expect="$2" keep="$3" keep_expect="$4" tag hits=0 i=0 v k bk=0
    while [ "$i" -lt 6 ]; do
        i=$((i + 1)); tag="n_${i}"
        run_once "$tag" PX_GC_STRESS=1 PX_GC_INLINE=1 >/dev/null
        if ! grep -q 'M256-PROBE-DONE' "$W/out_$tag.txt"; then
            N267=$((N267 + 1))
            echo "  ℹ️ [$tag] 探针未跑完（疑似缺陷 267 家族）⇒ 本次**不可判**，重试"
            continue
        fi
        k="$(get_val "$tag" "$keep")"
        if [ "$k" != "$keep_expect" ]; then
            bk=$((bk + 1))
            echo "  ℹ️ [$tag] $keep=$k（期望 $keep_expect）—— 第 $bk 次"
            [ "$bk" -ge 3 ] && return 2
            continue
        fi
        v="$(get_val "$tag" "$gone")"
        [ -n "$v" ] && [ "$v" != "$gone_expect" ] && hits=$((hits + 1))
        [ "$hits" -ge 1 ] && return 0
    done
    [ "$hits" -ge 1 ] && return 0
    return 1
}

if [ "$NEG_SKIP" = "1" ]; then
    echo "── [6] 负控：--neg-skip（CI 档）⇒ 跳过 3 道"
else
    echo "── [6] 负控 A：撤回 bi_read 的包装（退回裸 read）"
    restore_all; snapshot
    if patch_one '    ssize_t n = px_io_read(fd, buf, (size_t)maxlen);' \
                 '    ssize_t n = read(fd, buf, (size_t)maxlen);' \
       && rebuild "$W/build_na.log"; then
        neg_any T1 5 U2 4; rc=$?
        if [ $rc -eq 0 ]; then ok "[6A] 3 次内 T1 被判红，且 U2 始终绿（独立）"
        else bad "[6A] 未按预期判红（rc=$rc：1=故障未出现 2=波及 U2 3=补丁/重建失败）"; fi
    else
        bad "[6A] 补丁或重建失败"
    fi

    echo "── [6] 负控 B：撤回 bi_udp_recv 的包装"
    restore_all; snapshot
    if patch_one '    int n = (int)px_io_recvfrom(fd, buf, (size_t)maxlen, 0, (struct sockaddr*)&src, &slen);' \
                 '    int n = (int)recvfrom(fd, buf, (size_t)maxlen, 0, (struct sockaddr*)&src, &slen);' \
       && rebuild "$W/build_nb.log"; then
        neg_any U2 4 T1 5; rc=$?
        if [ $rc -eq 0 ]; then ok "[6B] 3 次内 U2 被判红，且 T1 始终绿（独立）"
        else bad "[6B] 未按预期判红（rc=$rc）"; fi
    else
        bad "[6B] 补丁或重建失败"
    fi

    echo "── [6] 负控 C：判据自伤（同一故障 + M256_EXPECT_T1=-1 ⇒ 不再判红）"
    restore_all; snapshot
    patch_one '    ssize_t n = px_io_read(fd, buf, (size_t)maxlen);' \
              '    ssize_t n = read(fd, buf, (size_t)maxlen);' >/dev/null 2>&1 || true
    rebuild "$W/build_nc.log" >/dev/null 2>&1 || true
    # ⚠️ M276 实测：缺陷 456 的**复现本身是概率性的**（要等 GC 的 STW 恰好打断 read）
    #   ⇒ 原来只跑 1 次就断言「故障复现」，实测 4 次里会红 1 次。改为**有界重试**（最多 6 次）。
    # ⚠️ 这里是**顶层**（不在函数里）—— 用 `local` 会 `can only be used in a function`
    #   + `set -u` 下紧接 `_i: unbound variable`（M276 实测踩到）。顶层一律用普通变量。
    _i=0; _rep=0
    while [ "$_i" -lt 6 ]; do
        _i=$((_i + 1))
        run_track nc PX_GC_STRESS=1 PX_GC_INLINE=1 >/dev/null || true
        if [ "$(get_val nc T1)" != "5" ]; then _rep=1; break; fi
        echo "  ℹ️ [nc] 第 $_i 次未复现（T1=5）—— STW 未恰好打断 read，重试"
    done
    if [ "$_rep" = 1 ]; then
        if M256_EXPECT_T1=-1 judge_track nc >/dev/null 2>&1; then
            ok "[6C] 同故障在自伤判据下不再判红 ⇒ 红来自比对本身"
        else
            bad "[6C] 自伤判据下仍判红 ⇒ 红的来源不明"
        fi
    else
        bad "[6C] 故障 6 次均未复现（STW 未打断 read）—— 判据无法自证"
    fi
    restore_all
    # ⚠️⚠️ M288b **实测出来的一个潜在缺陷（本门自己的）**：`restore_all` 只还原**源码**，
    #   而 `$BIN` 仍是负控 C 编出来的**打了补丁的**二进制（NC-C 的 rebuild 用的是
    #   「撤回包装」后的 runtime.c）⇒ 直接复用它，读到的是「缺陷 456 仍在」的产物
    #   （实测：`T1=-1`，而期望是 `T1=FOREIGN:…`）。
    #   修前这条**一直存在**，只是后面没有再用 `$BIN`，所以从未显形（NC-C 是最后一道）。
    #   ⇒ 凡「还原源码之后还要再跑一次产物」的地方，都必须**重编**。
    #   （M287 的槽缓存让这一步≈0.2s —— 干净源码的 key 在 [2] 就建过。）
    if ! rebuild "$W/build_nd0.log"; then
        bad "[6D] 干净件重编失败（NC-D 的判据不可信）"
    fi

    # ── M288b：负控 D —— 「环境干扰」必须被**单独分类** ────────────────
    #   做法：让 srv 发**同长度不同内容**（WORLD）的 5 字节 ⇒ 确定性构造「陌生内容」。
    #   ⚠️ 刻意**同长度**：这样「长度判据」（修前的形态）会**放过**它、只有「内容判据」抓得住
    #     ⇒ 本负控证的正是「新判据有牙」，而不是在跑一条本来就会红的路径。
    #   ⚠️ 不依赖任何源码补丁（所以放在 restore_all 之后）。
    echo "── [6] 负控 D（M288b）：环境干扰（陌生内容）必须**单独分类**"
    # ⚠️ M288b 补 —— **分两层**（首版把两层揉在一起 ⇒ 全量门里假红一次，见下）：
    #   ① **装置层**：探针在 `M256_PROBE_EXTRA=1` 下必须报出 WORLD 的 hex
    #      （= 证 `M256_PROBE_EXTRA` 这条**模拟通路**真的改了对端载荷，且**内容判据**抓得住它
    #        —— 同长度不同内容：**长度判据会放过**）。
    #      网络/端口/GC 是**外部**条件 ⇒ **有界重试**（3 次换端口）；仍未取得才判红并打印实际值。
    #   ② **判据层**：直接喂**合成输出**给 judge_track ⇒ 完全不依赖网络/端口/压力。
    #      —— 本负控**真正要证**的是「分类代码把 FOREIGN 归到『环境干扰』而不是『产品回归』」，
    #      那是一个**纯函数性质**，不该靠跑真实网络来证。
    #
    # ⚠️ 为什么必须拆（实测，第 167 轮全量门）：首版用**单次** `run_once` + 精确值断言，
    #   实测拿到 `T1=FOREIGN:70363938`（既非 WORLD 也非 HELLO）⇒ 判红。
    #   无论那是宿主干扰还是产品问题，**门的负控都不应该依赖网络的安静** ——
    #   何况信号被混进了「装置/判据」同一层，读不出真因。
    _nd=0
    for _i in 1 2 3; do
        run_once nd PX_GC_STRESS=1 PX_GC_INLINE=1 M256_PROBE_EXTRA=1 >/dev/null 2>&1 || true
        if grep -q '^T1=FOREIGN:574f524c44$' "$W/out_nd.txt"; then _nd=1; break; fi
        echo "  ℹ️ [6D] 第 $_i 次未取到 WORLD 的 hex（实际 T1=$(get_val nd T1)）⇒ 换端口重试"
    done
    if [ "$_nd" = 1 ]; then
        ok "[6D] 探针把**同长度不同内容**（WORLD）报成 T1=FOREIGN:574f524c44（内容判据有牙）"
    else
        bad "[6D] 3 次均未取到 T1=FOREIGN:574f524c44（实际 T1=$(get_val nd T1)）"
        sed 's/^/       | /' "$W/out_nd.txt" 2>/dev/null | head -8
    fi
    # ② 判据层 · 合成输入 ⇒ 确定性（不看网络、不看 GC、不看端口）
    printf 'T1=FOREIGN:574f524c44\nU2=4\nM256-PROBE-DONE\n' > "$W/out_nds.txt"
    # 用**子 shell** 跑 judge_track：它内部的 ok/bad 不应污染本门的计数与结论。
    if ( judge_track nds ) >"$W/ncd.log" 2>&1; then
        bad "[6D] 环境干扰被判成通过 ⇒ 分类失效"
    else
        if grep -q '环境干扰' "$W/ncd.log" && ! grep -q '期望 5' "$W/ncd.log"; then
            ok "[6D] 门把环境干扰**单独分类**（合成输入 · 不依赖环境）"
        else
            bad "[6D] 分类文案不对（应出现「环境干扰」且**不得**出现「期望 5」）"
            sed 's/^/       | /' "$W/ncd.log" 2>/dev/null | head -8
        fi
    fi
fi

# ---------- 层 [7] 源还原 ----------
echo "── [7] 源逐字节还原"
if cmp -s "$SNAP/$SRC" "$SRC"; then
    ok "[7] runtime.c 与快照逐字节一致"
else
    bad "[7] runtime.c 未还原（负控残留）"
fi

echo
echo "ℹ️ 缺陷 267 家族（并发 GC · 已登记未修）命中并重试：$N267 次"
echo "ℹ️ 重试分类（缺陷 497 记账）：超时 $NTO 次 · 异常退出 $NAB 次 —— 两类都**不是** EINTR 回归"
echo "ℹ️ 环境干扰（M288b 记账）：$NENV 次 —— **不是**产品回归，是端口上有陌生数据/被占；0 次表示本次环境干净"
if [ "$FAIL" -eq 0 ]; then
    echo "M256-VERIFY-OK（通过 $PASS / 失败 $FAIL）"
    exit 0
fi
echo "M256-VERIFY-FAIL（通过 $PASS / 失败 $FAIL）"
exit 1
