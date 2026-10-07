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
BD="$HERE/build"
BIN="$BD/probe_eintr"
PASS=0
FAIL=0
N267=0
NTO=0
NAB=0
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
    local _base=$(( 20000 + RANDOM % 20000 ))
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
    return 1
}

get_val() { grep -oE "^$2=[-0-9A-Za-z]+" "$W/out_$1.txt" | head -1 | cut -d= -f2; }

# 判一档：$1=档名 → 失败条数（期望值可用 M256_EXPECT_T1/U2 覆盖 ⇒ 供负控 C 自伤）
judge_track() {
    local tag="$1" n=0 v
    local e_t1="${M256_EXPECT_T1:-5}" e_u2="${M256_EXPECT_U2:-4}"
    v="$(get_val "$tag" T1)"
    if [ "$v" = "$e_t1" ]; then ok "[$tag] T1=$v（read 拿到对端发来的 5 字节）"
    else bad "[$tag] T1=$v（期望 $e_t1 —— read 被 GC 暂停信号打断后误报失败）"; n=$((n + 1)); fi
    v="$(get_val "$tag" U2)"
    if [ "$v" = "$e_u2" ]; then ok "[$tag] U2=$v（udp_recv 拿到数据报）"
    else bad "[$tag] U2=$v（期望 $e_u2 —— udp_recv 被中断后返回 null=「没有包」静默丢包）"; n=$((n + 1)); fi
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
if [ "$FAIL" -eq 0 ]; then
    echo "M256-VERIFY-OK（通过 $PASS / 失败 $FAIL）"
    exit 0
fi
echo "M256-VERIFY-FAIL（通过 $PASS / 失败 $FAIL）"
exit 1
