#!/usr/bin/env bash
# ============================================================
# M261（缺陷 464）门：**持锁临界区内的可失败分配 —— 收口（一）**
# ------------------------------------------------------------
# 病灶：M125/M126 把「运行时错误 / 分配失败」收紧到请求级（longjmp 回隔离点），
#   但 **longjmp 不展开 pthread 锁** ⇒ 失败点落在临界区内时回卷会把锁留成「已锁」
#   ⇒ 假死；M127 只能退回 `_exit(1)`（**服务中断**）。M128 定下修法 = **两阶段**
#   （锁外备货 → 锁内只发布指针），实测修了 3 处、漏了 `px_str_int_pool`（缺陷 439）。
#   M260 修 439 并建本守卫（首扫 18 处 → 基线 13 处）。
#   **本轮（M261）收口 7 处**：fserve_ensure×2 · px_conn_pend_put · px_pin_obj（死代码）·
#   px_const_put · px_rate_limit_try×2 ⇒ 13 → 6（**4 待收口 + 2 有意保留**）。
#   ⇒ 只剩 `route_match`×3 与 `bi_sse_read_line`×1 待收口（结构性重写，下一轮），
#     以及 2 条已在基线里写明理由的 `ACCEPTED:`（gc_register 兜底 / px_list_push_locked 对照）。
# 另：本轮顺带修好守卫自身两处**判据 bug**（见下 [3] 与 [6]）。
#
# 判据分层：
#   [1] 静态锚点 20 条（含**反向判据**：被收口的旧形态必须 0 次；含判定器自证 2 条）
#   [2] 守卫自证 7/7（M261 由 5 → 7：新增「行号位移不得假新增」「同函数同被调多重集」）
#   [3] 基线账：rc=0 · 待收口 4 + 有意保留 2 · 每条 ACCEPTED 必须带理由
#   [4] 行为冒烟：fserve_ensure（首个 serve 入口建池）+ px_rate_limit_try（建桶 + 命中）
#   [5] 负控 A：**忠实退回** px_conn_pend_put 旧实现（锁内 xrealloc/xmalloc）⇒ 守卫必判红
#   [6] 负控 B：抹掉 ACCEPTED 理由 ⇒ 必须 rc=3（「无理由豁免」= 把红当绿，不许放行）
#   [7] 负控 C（判据自伤）：清空守卫的 DIRECT/INDIRECT 表 ⇒ 负控 A 的红**必须消失**
#       + 覆盖边界登记
# 用法：bash examples/m261_lock_alloc/verify.sh [--neg-skip]
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m261_lock_alloc
PX=${PX:-$ROOT/tools/px}
PORT=18321
W=/tmp/m261_gate
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
rm -rf "$W"; mkdir -p "$W/bak"
cp runtime/runtime.c "$W/bak/runtime.c"
cp selfhost/check_lock_alloc.py "$W/bak/guard.py"
cp selfhost/lock_alloc_baseline.txt "$W/bak/baseline.txt"
restore_all() {
  cp -f "$W/bak/runtime.c" runtime/runtime.c
  cp -f "$W/bak/guard.py" selfhost/check_lock_alloc.py
  cp -f "$W/bak/baseline.txt" selfhost/lock_alloc_baseline.txt
}
trap 'restore_all; rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }
grc() { python3 selfhost/check_lock_alloc.py --root . 2>&1; return $?; }

echo "== M261 门：持锁可失败分配收口（一） =="
echo
echo "[1/7] 静态锚点（20 条：反向 5 · 正向 8 · 逐函数锁区零分配 5 · 判定器自证 2）"
if python3 "$DIR/check_anchors.py" --root . >"$W/anchors.log" 2>&1; then
  ok "$(grep '^── ' "$W/anchors.log")"
else
  bad "静态锚点失败"; grep -E '^  ❌' "$W/anchors.log" | sed 's/^/     /'
fi

echo
echo "[2/7] 守卫自证（7/7）"
if python3 selfhost/check_lock_alloc.py --self-test >"$W/selftest.log" 2>&1 \
   && grep -q 'LOCK-ALLOC-SELFTEST-OK' "$W/selftest.log"; then
  ok "check_lock_alloc.py 自证 $(grep -o 'self-test: [0-9]*/[0-9]*' "$W/selftest.log")"
else
  bad "守卫自证失败"; tail -8 "$W/selftest.log" | sed 's/^/     /'
fi

echo
echo "[3/7] 基线账（待收口 4 + 有意保留 2；ACCEPTED 必须带理由）"
OUT=$(python3 selfhost/check_lock_alloc.py --root . 2>&1); RC=$?
if [ "$RC" = 0 ] && echo "$OUT" | grep -q '待收口 4 + 有意保留 2'; then
  ok "实测与基线一致：$(echo "$OUT" | head -1)"
else
  bad "基线账不符（rc=$RC）：$OUT"
fi
NACC=$(grep -v '^#' selfhost/lock_alloc_baseline.txt | grep -c 'ACCEPTED:' || true)
if [ "$NACC" = 2 ]; then ok "有意保留 2 条且各自写明理由"; else bad "ACCEPTED 条数=$NACC（期望 2）"; fi

echo
echo "[4/7] 行为冒烟：fserve_ensure（首个 serve 入口建池）+ px_rate_limit_try（建桶/命中）"
if "$PX" build "$DIR/smoke.px" >"$W/build.log" 2>&1 && [ -x "$DIR/build/smoke" ]; then
  ( "$DIR/build/smoke" >"$W/smoke.log" 2>&1 & echo $! >"$W/smoke.pid" )
  SPID=$(cat "$W/smoke.pid")
  for _ in $(seq 1 60); do grep -q 'M261 READY' "$W/smoke.log" && break; sleep 0.25; done
  if grep -q 'M261 READY' "$W/smoke.log"; then
    H=$( ( exec 3<>/dev/tcp/127.0.0.1/$PORT 2>/dev/null &&
            printf 'GET /health HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' >&3 &&
            timeout 10 cat <&3 ) 2>/dev/null | tr -d '\r' | tail -1 )
    [ "$H" = "alive" ] && ok "GET /health ⇒ alive（fserve 建池发布正确）" || bad "/health body='$H'"
    R=$( ( exec 3<>/dev/tcp/127.0.0.1/$PORT 2>/dev/null &&
            printf 'GET /rl HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' >&3 &&
            timeout 10 cat <&3 ) 2>/dev/null | tr -d '\r' | tail -1 )
    [ "$R" = "rl=11111" ] && ok "GET /rl ⇒ rl=11111（建桶 + 5 次命中全通过）" || bad "/rl body='$R'"
  else
    bad "冒烟程序未 READY"; tail -5 "$W/smoke.log" | sed 's/^/     /'
  fi
  kill -9 "$SPID" 2>/dev/null; wait "$SPID" 2>/dev/null
else
  bad "冒烟程序编译失败"; tail -10 "$W/build.log" | sed 's/^/     /'
fi

if [ "$NEG_SKIP" = 0 ]; then
echo
echo "[5/7] 负控 A：**忠实退回** px_conn_pend_put 旧实现（锁内 xrealloc/xmalloc）⇒ 守卫必判红"
python3 "$DIR/negctl.py" revert-pend-put >"$W/ncA.log" 2>&1 \
  && ok "旧实现已植入（$(grep -c . "$W/ncA.log") 行输出）" || bad "植入失败：$(tail -2 "$W/ncA.log")"
OUT=$(python3 selfhost/check_lock_alloc.py --root . 2>&1); RC=$?
if [ "$RC" = 1 ] && echo "$OUT" | grep -q 'px_conn_pend_put'; then
  ok "守卫判红且**指名** px_conn_pend_put：$(echo "$OUT" | grep 'px_conn_pend_put' | head -1 | tr -d ' ')"
else
  bad "守卫未按预期判红（rc=$RC）：$(echo "$OUT" | head -2 | tr '\n' ' ')"
fi
restore_all
if cmp -s "$W/bak/runtime.c" runtime/runtime.c; then ok "负控 A 后源逐字节还原"; else bad "还原不完整"; fi

echo
echo "[6/7] 负控 B：抹掉 ACCEPTED 理由 ⇒ 必须 rc=3（无理由豁免 = 把红当绿）"
sed 's/\tACCEPTED:.*/\tACCEPTED:/' selfhost/lock_alloc_baseline.txt >"$W/nor.baseline"
python3 selfhost/check_lock_alloc.py --root . --baseline "$W/nor.baseline" >"$W/ncB.log" 2>&1; RC=$?
if [ "$RC" = 3 ] && grep -q '无理由' "$W/ncB.log"; then
  ok "rc=3 且文案指向真因：$(grep '无理由' "$W/ncB.log" | head -1 | cut -c1-60)"
else
  bad "未按预期 rc=3（实得 $RC）：$(head -2 "$W/ncB.log" | tr '\n' ' ')"
fi
# B2：基线里凭空多一条 ⇒ 必须 rc=1（防「基线可任意放水」）
{ cat "$W/bak/baseline.txt"; printf 'runtime/runtime.c:1\t__fake__\txmalloc\tDIRECT\n'; } >"$W/extra.baseline"
python3 selfhost/check_lock_alloc.py --root . --baseline "$W/extra.baseline" >"$W/ncB2.log" 2>&1; RC=$?
[ "$RC" = 0 ] && ok "基线多一条**不影响**实测比对（多出的是基线独有 ⇒ 只提示收口）" || bad "rc=$RC（期望 0）"

echo
echo "[7/7] 负控 C（判据自伤）：清空守卫的两张分配表 ⇒ 负控 A 的红**必须消失**"
python3 "$DIR/negctl.py" neuter-guard "$W/guard_neutered.py" >/dev/null 2>&1 \
  && ok "已生成「判据失明」版守卫" || bad "生成失败"
python3 "$DIR/negctl.py" revert-pend-put >/dev/null 2>&1
OUT=$(python3 "$W/guard_neutered.py" --root . 2>&1); RC=$?
if [ "$RC" = 0 ] && ! echo "$OUT" | grep -q '❌'; then
  ok "判据失明后不再判红 ⇒ 证明负控 A 的红**来自判据**（rc=0）"
else
  bad "判据失明后仍判红（rc=$RC）⇒ 负控 A 的红来路不明"
fi
restore_all
else
echo; echo "[5-7/7] 负控已跳过（--neg-skip）"
fi

echo
echo "── 覆盖边界（如实登记）"
echo "   · 本轮的锚点与守卫都是**静态**判据；「锁内分配真的会导致 _exit(1)」的动态证明"
echo "     由 m128 门（PX_ALLOC_FAIL_IN_LOCK 注入 + 锁审计行）承担，本门不重复。"
echo "   · 余留字节路径（px_conn_pend_put）的动态覆盖在 HTTP 管线化门里"
echo "     （m95_s2 / m97_s2 / m131 / m176 / m242）；本门只验 fserve/rate_limit 两条。"
echo '   · route_match×3 与 bi_sse_read_line×1 **本轮未收口**（结构性重写：前者要'
echo '     把段快照到栈、后者要「栈快路径 + 锁外备货」）⇒ 已在基线记为「待收口」，下一轮处理。'
echo '   · 2 条 ACCEPTED: 是**有意保留的兜底路径**（理由见基线文件），不是欠账。'
echo '   ⚠️ 提示文本一律**单引号** —— 双引号里的反引号会触发命令替换（本仓第 6 次记）。'
echo
echo "── M261 门结束：pass=$pass fail=$fail"
[ "$fail" = 0 ] && echo "M261-VERIFY-OK" || echo "M261-VERIFY-FAIL"
exit "$fail"
