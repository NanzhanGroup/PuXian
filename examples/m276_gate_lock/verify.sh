#!/bin/bash
# M276 门 · **门级互斥锁**（晨曦 2026-10-05 回馈）
#
# 主题：本仓 **98 个门**会在负控里现场改源码（`restore_all` / `NEGCTL` / `run_neg` …）。
#   全量门有 PID 锁（`run_gates.sh`），但**人工单跑一扇门**与全量门并发**无覆盖** ⇒
#   负控的 snapshot/restore 互相盖掉（M191 / M213 / M221 / M223 各撞过一次：
#   假红、假绿、残留被烘进产物）。
#
# 判据
#   [1] 静态：gate_lock.sh / run_gates.sh 的关键形状（持有 + 导出 + PID 文件而非 flock）
#   [2] 守卫自扫：check_gate_lock.sh rc=0 + 规模锚点
#   [3] 行为五组：抢到 / 并发被拒 / 陈旧自愈 / 已持则跳过 / **后代不继承**（fd 泄漏回归位）
#   [4] 竞态：10 进程同时启动 ⇒ **恰好 1 个**拿到
#   [5] 与 run_gates.sh 的集成：持锁时**拒启**（对照：已持则放行 ⇒ 证明不是恒拒）
#   [6] 负控 A/B/C（各自独立判红 + 源逐字节还原）
#   [7] 覆盖边界（如实登记）
#
# CI 用 `--neg-skip`（负控 A 要现场改 gate_lock.sh 并重跑行为组）。
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh"   # M276 门级互斥（见 selfhost/gate_lock.sh）
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m276_gate_lock
W=/tmp/m276_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG=0; [ "$a" = "--harm" ] && HARM=1; done
HARM=${HARM:-0}

P=0; F=0
chk() { if eval "$2"; then echo "  ✅ $1"; P=$((P+1)); else echo "  ❌ $1"; F=$((F+1)); fi; }
hdr() { echo ""; echo "── $* ──"; }

SNAP="$W/snap"; mkdir -p "$SNAP"
snapshot() { cp -a selfhost/gate_lock.sh "$SNAP/gate_lock.sh.orig"; cp -a examples/m205_cli_channels/verify.sh "$SNAP/m205.orig"; }
restore_all() { [ -f "$SNAP/gate_lock.sh.orig" ] && cp -a "$SNAP/gate_lock.sh.orig" selfhost/gate_lock.sh
                [ -f "$SNAP/m205.orig" ] && cp -a "$SNAP/m205.orig" examples/m205_cli_channels/verify.sh; }
trap 'restore_all' EXIT
snapshot

# 一个「像门一样」的持有者（cmdline 含 verify.sh ⇒ 存活判据认它）
HOLD="$W/holder"; mkdir -p "$HOLD"
cat > "$HOLD/verify.sh" <<EOF
#!/usr/bin/env bash
. "$ROOT/selfhost/gate_lock.sh"
echo "HOLDER pid=\$\$"
sleep "\${1:-5}"
EOF
chmod +x "$HOLD/verify.sh"
LK="$W/lock"; rm -f "$LK"

echo "══ M276 门：门级互斥锁 ══"

# ── [1] 静态 ────────────────────────────────────────────────
hdr "[1] 静态：形状"
chk "[1] selfhost/gate_lock.sh 在位" "[ -f selfhost/gate_lock.sh ]"
# ⚠️ 必须**剥注释**再判 —— gate_lock.sh 的文档里大量解释「为什么不用 flock」，
#    直接 `grep -q flock` 会把解释性文字当成用法（M264 记过的「注释里的路径不算引用」同族；本轮自伤一次）。
chk "[1] 用「PID 文件」而非 flock（fd 泄漏回归位）" \
    "! { grep -vE '^[[:space:]]*#' selfhost/gate_lock.sh | grep -q 'flock'; }"
chk "[1] 存活判据含 cmdline 复核（防 PID 复用）" "grep -q '/proc/\$1/cmdline' selfhost/gate_lock.sh"
chk "[1] 拿不到锁 ⇒ rc=2（响亮，不静默）" "grep -q 'exit 2' selfhost/gate_lock.sh"
chk "[1] 成功后导出 PX_GATE_LOCK_HELD=1" "grep -q 'export PX_GATE_LOCK_HELD=1' selfhost/gate_lock.sh"
chk "[1] run_gates.sh 持有同一把锁（写 \$_PGK）" "grep -q '> \"\$_PGK\"' selfhost/run_gates.sh"
chk "[1] run_gates.sh 的锁块在 TSV 截断**之后**（m244 [5] 段依赖）" \
    "awk '/^: > \"\\\$GATE_TSV\"/{a=NR} /_PX_GL_MINE=0/{b=NR} END{exit !(a && b && b>a)}' selfhost/run_gates.sh"
chk "[1] 规模：source 了锁的门 ≥190（实测 $(grep -l 'gate_lock.sh' examples/*/verify.sh | wc -l)）" \
    "[ $(grep -l 'gate_lock.sh' examples/*/verify.sh | wc -l) -ge 190 ]"

# ── [2] 守卫自扫 ────────────────────────────────────────────
hdr "[2] 守卫 check_gate_lock.sh 自扫"
bash selfhost/check_gate_lock.sh > "$W/guard.log" 2>&1; grc=$?
sed 's/^/  /' "$W/guard.log"
chk "[2] 守卫 rc=0" "[ $grc -eq 0 ]"
chk "[2] 报 GATE-LOCK-GUARD-OK" "grep -q 'GATE-LOCK-GUARD-OK' $W/guard.log"

# ── [3] 行为五组 ────────────────────────────────────────────
hdr "[3] 行为：抢到 / 并发被拒 / 陈旧自愈 / 已持跳过 / 后代不继承"
( unset PX_GATE_LOCK_HELD; export PX_GATE_LOCK="$LK"; . "$ROOT/selfhost/gate_lock.sh" && echo "GOT $$" ) > "$W/b1.log" 2>&1
chk "[3a] 无竞争 ⇒ 抢到" "grep -q 'GOT' $W/b1.log"

rm -f "$LK"
env -u PX_GATE_LOCK_HELD PX_GATE_LOCK="$LK" "$HOLD/verify.sh" 6 > "$W/b2a.log" 2>&1 & HP=$!
sleep 0.6
( unset PX_GATE_LOCK_HELD; export PX_GATE_LOCK="$LK"; . "$ROOT/selfhost/gate_lock.sh" && echo "BAD" ) > "$W/b2b.log" 2>&1; rc2=$?
chk "[3b] 有人持锁 ⇒ 被拒 rc=2" "[ $rc2 -eq 2 ]"
chk "[3b] 报「拒绝并发」" "grep -q '拒绝并发' $W/b2b.log"
chk "[3b] **未**拿到（无 BAD）" "! grep -q '^BAD' $W/b2b.log"
kill $HP 2>/dev/null; wait $HP 2>/dev/null

printf '999999\n' > "$LK"
( unset PX_GATE_LOCK_HELD; export PX_GATE_LOCK="$LK"; . "$ROOT/selfhost/gate_lock.sh" && echo "GOT $$" ) > "$W/b3.log" 2>&1
chk "[3c] 陈旧锁（持有者已死）⇒ 自愈抢到" "grep -q 'GOT' $W/b3.log"

sleep 20 & SP=$!
printf '%s\n' "$SP" > "$LK"
( unset PX_GATE_LOCK_HELD; export PX_GATE_LOCK="$LK"; . "$ROOT/selfhost/gate_lock.sh" && echo "GOT $$" ) > "$W/b3b.log" 2>&1
chk "[3c'] PID 被复用为**非门进程** ⇒ 仍自愈（cmdline 复核有牙）" "grep -q 'GOT' $W/b3b.log"
kill $SP 2>/dev/null; wait $SP 2>/dev/null

printf '99999999\n' > "$LK"
( export PX_GATE_LOCK="$LK" PX_GATE_LOCK_HELD=1; . "$ROOT/selfhost/gate_lock.sh" && echo "SKIP" ) > "$W/b4.log" 2>&1
chk "[3d] PX_GATE_LOCK_HELD=1 ⇒ 短路不抢" "grep -q 'SKIP' $W/b4.log"
chk "[3d] 短路时**不**动锁文件" "grep -qx 99999999 $LK"

rm -f "$LK"
mkdir -p "$W/leak"; cat > "$W/leak/verify.sh" <<EOF
#!/usr/bin/env bash
. "$ROOT/selfhost/gate_lock.sh"
setsid sleep 15 </dev/null >/dev/null 2>&1 &
echo DONE
EOF
chmod +x "$W/leak/verify.sh"
env -u PX_GATE_LOCK_HELD PX_GATE_LOCK="$LK" "$W/leak/verify.sh" > "$W/b5a.log" 2>&1
sleep 0.3
( unset PX_GATE_LOCK_HELD; export PX_GATE_LOCK="$LK"; . "$ROOT/selfhost/gate_lock.sh" && echo "GOT $$" ) > "$W/b5b.log" 2>&1
chk "[3e] 持有者退出、**后台子进程仍活** ⇒ 下一扇门照样能拿（无 fd 泄漏）" "grep -q 'GOT' $W/b5b.log"
ps -eo pid,args | grep 'sleep 15' | grep -v grep | awk '{print $1}' | xargs -r kill 2>/dev/null

# ── [4] 竞态 ────────────────────────────────────────────────
hdr "[4] 竞态：10 进程同时启动 ⇒ 恰好 1 个"
rm -f "$LK"; : > "$W/race.out"
for i in $(seq 1 10); do
    env -u PX_GATE_LOCK_HELD PX_GATE_LOCK="$LK" "$HOLD/verify.sh" 0.4 >> "$W/race.out" 2>&1 &
done
wait
nw=$(grep -c 'HOLDER' "$W/race.out" || true); nw=${nw:-0}
chk "[4] 恰好 1 个拿到（实测 $nw）" "[ $nw -eq 1 ]"

# ── [5] 与 run_gates.sh 集成 ────────────────────────────────
hdr "[5] run_gates.sh：持锁时拒启；已持则放行"
rm -f "$LK"
printf '%s\n' "$$" > "$LK"      # 本门就是持锁者
env -u PX_GATE_LOCK_HELD PX_GATE_LOCK="$LK" PX_GATE_PIDLOCK="$W/pidlock" GATE_TSV="$W/tsv1" \
    ./selfhost/run_gates.sh --only __m276_none__ --allow-dirty > "$W/rg1.log" 2>&1; r1=$?
chk "[5a] 有门单跑着 ⇒ 全量门**拒启** rc≠0" "[ $r1 -ne 0 ]"
chk "[5a] 报「有一扇门单跑着」" "grep -q '有一扇门单跑着' $W/rg1.log"
env PX_GATE_LOCK_HELD=1 PX_GATE_LOCK="$LK" PX_GATE_PIDLOCK="$W/pidlock" GATE_TSV="$W/tsv2" \
    ./selfhost/run_gates.sh --only __m276_none__ --allow-dirty > "$W/rg2.log" 2>&1; r2=$?
chk "[5b] 对照：已持锁（env 继承）⇒ 放行 rc=0（证明 5a 不是恒拒）" "[ $r2 -eq 0 ]"
rm -f "$LK"

# ── [6] 负控 ────────────────────────────────────────────────
if [ "$NEG" = 1 ]; then
    hdr "[6a] 负控 A：把 gate_lock.sh 变空转（不看锁）⇒ [3b] 必须红"
    python3 - "$ROOT" <<'PY'
import sys, pathlib
p = pathlib.Path(sys.argv[1]) / "selfhost/gate_lock.sh"
s = p.read_text()
old = 'if [ "${PX_GATE_LOCK_HELD:-0}" = "1" ]; then\n    return 0 2>/dev/null || exit 0\nfi'
assert s.count(old) == 1, f"锚点 {s.count(old)}"
open(p, "w").write(s.replace(old, old + '\nreturn 0 2>/dev/null || exit 0   # NEGCTL-A', 1))
PY
    rm -f "$LK"
    env -u PX_GATE_LOCK_HELD PX_GATE_LOCK="$LK" "$HOLD/verify.sh" 5 > "$W/na_h.log" 2>&1 & HPA=$!
    sleep 0.6
    ( unset PX_GATE_LOCK_HELD; export PX_GATE_LOCK="$LK"; . "$ROOT/selfhost/gate_lock.sh" && echo "BAD" ) > "$W/na.log" 2>&1; rca=$?
    if [ "$HARM" = 1 ]; then
        chk "[6a] --harm：空转档下**不**被拒（证明红来自 [3b] 那条断言本身）" "[ $rca -eq 0 ]"
    else
        chk "[6a] A：空转 ⇒ 第二扇门**未被拒** rc=$rca（必须非 2）" "[ $rca -ne 2 ]"
        chk "[6a] A：[3b] 等价断言当场判红" "grep -q '^BAD' $W/na.log"
    fi
    kill $HPA 2>/dev/null; wait $HPA 2>/dev/null
    restore_all
    chk "[6a] gate_lock.sh 逐字节还原" "cmp -s $SNAP/gate_lock.sh.orig selfhost/gate_lock.sh"

    hdr "[6b] 负控 B：抽掉一个门的 source 行 ⇒ 守卫 J1 必须红"
    python3 - "$ROOT" <<'PY'
import sys, pathlib
p = pathlib.Path(sys.argv[1]) / "examples/m205_cli_channels/verify.sh"
lines = p.read_text().split("\n")
out = [l for l in lines if "gate_lock.sh" not in l]
assert len(out) == len(lines) - 1, "应有且仅有 1 行"
p.write_text("\n".join(out))
PY
    bash selfhost/check_gate_lock.sh > "$W/nb.log" 2>&1; rcb=$?
    if [ "$HARM" = 1 ]; then
        chk "[6b] --harm：判据自伤 ⇒ 该红**消失**" "[ $rcb -eq 0 ]"
    else
        chk "[6b] B：守卫判红 rc=1" "[ $rcb -eq 1 ]"
        chk "[6b] B：指名 m205_cli_channels" "grep -q 'm205_cli_channels' $W/nb.log"
    fi
    restore_all
    chk "[6b] m205 门逐字节还原" "cmp -s $SNAP/m205.orig examples/m205_cli_channels/verify.sh"

    hdr "[6c] 负控 C：判据自伤（守卫 --harm + 同一夹具）⇒ B 的红必须消失"
    python3 - "$ROOT" <<'PY'
import sys, pathlib
p = pathlib.Path(sys.argv[1]) / "examples/m205_cli_channels/verify.sh"
lines = p.read_text().split("\n")
out = [l for l in lines if "gate_lock.sh" not in l]
p.write_text("\n".join(out))
PY
    bash selfhost/check_gate_lock.sh --harm > "$W/nc.log" 2>&1; rcc=$?
    chk "[6c] C：--harm 下 rc=0（红来自 J1 那条判据本身）" "[ $rcc -eq 0 ]"
    restore_all
    chk "[6c] m205 门逐字节还原（第二次）" "cmp -s $SNAP/m205.orig examples/m205_cli_channels/verify.sh"
else
    hdr "[6] 负控：--neg-skip（CI 模式）"
fi

# ── 收尾：源必须与进门时一致 ────────────────────────────────
hdr "[7] 覆盖边界（如实登记）"
cat <<'EOF'
  · 锁是**全局单把**（不按仓库根分键）⇒ 两个 git worktree 会互相串行（**有意**：分键要
    runner/gate 两处各算一遍，算不一致 = 静默失效；宁可过度串行）。
  · 「持有者是否在跑门」靠 `kill -0` + `/proc/<pid>/cmdline` 正则 ⇒ **非 Linux 平台**退回只看存活
    （PID 复用理论上会误判为「被占」；Linux 上是精确的）。
  · 本门**只**覆盖 `examples/*/verify.sh`；`selfhost/check_*.sh` 那批守卫不改源码，未纳面。
  · `run_gates.sh` 的锁块在**脏树检查之后** ⇒ 脏树时不会走到锁（那条路径本来就拒启）。
  · [5] 段必须给内层 run_gates 一个**独立的 PID 锁路径**（`PX_GATE_PIDLOCK`）：
    否则在**全量门里**跑时，内层会先撞上外层那把锁 ⇒ [5a]/[5b] 必红（M276 实测踩到）。
  · 竞态判据（[4]）是**概率性**的：10 进程同启 100 次才等价于穷举；本门跑 1 次。
EOF
restore_all
chk "[F] 源与进门时逐字节一致（自证未被负控污染）" \
    "cmp -s $SNAP/gate_lock.sh.orig selfhost/gate_lock.sh && cmp -s $SNAP/m205.orig examples/m205_cli_channels/verify.sh"

echo ""
echo "══════════════════════════════════════════"
echo "  M276 门结果：通过 $P · 失败 $F"
echo "══════════════════════════════════════════"
[ "$F" -eq 0 ] && echo "M276-VERIFY-OK" || { echo "M276-VERIFY-FAIL"; exit 1; }
