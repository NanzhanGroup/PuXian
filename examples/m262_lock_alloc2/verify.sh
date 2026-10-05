#!/usr/bin/env bash
# ============================================================
# M262（缺陷 464 收尾 + 缺陷 465）门：**持锁可失败分配 —— 收口（二）**
# ------------------------------------------------------------
# 本轮把 M261 剩余的 4 处**结构性**站点收口 ⇒ 基线 **6 → 2**（只剩两条 `ACCEPTED:` 兜底）：
#   ① `route_match`（runtime_route.c）：修前**锁内** `px_dict()` + `px_root_push_keep` +
#      `px_dict_set(…, px_str(…))` ×2 ⇒ 每个请求都在临界区内做可失败分配。
#      修法 = 锁内**只读匹配** → 匹配段的 **定长快照到栈**（`PxRouteSeg.seg` 是 `char[256]`、
#      段数上限 32，零分配）→ **锁外**构造 params（`px_root_push_keep`/`px_root_pop` 成对）。
#   ② `bi_sse_read_line`：修前两处 `xmalloc(ll+1) … px_str(tmp) … xfree(tmp)` 都在
#      `g_sse_cli_mu` 临界区内。而 `tmp` **根本不需要**（`px_str_len` 自己会拷贝字节）
#      ⇒ 三层：短行进**栈缓冲**（一次堆分配都不要）· 长行走**跨轮复用**的堆缓冲且只在
#      **锁外增长** · 备货尺寸取 pending 的**容量上界**（`pend_cap+1`，容量只增）⇒ 下轮必够。
#
# 顺带**缺陷 465**（本轮由自写的路由冒烟当场照出）：M257 的容器守卫把「目标类型不符」
#   一律当作「存储已被 GC 回收并复用」。对**写入口**（`px_dict_set`/`px_list_push`）这是对的；
#   对**读入口**（`px_dict_get`）**不成立** —— 运行时代码里存在大量**合法探测式读**
#   （典型 `px_dict_get(resp, "headers")`，而 `resp` 可以是 string）⇒ 判据把「正常探测」
#   误报成「存储已失效」，且**每个请求**刷一条 CTR。
#   ⚠️ **A/B 对照**：干净 M261 树同样复现（5/5 请求）⇒ **pre-existing**，非本轮引入。
#   ⇒ 读入口默认**静默**（回到 M257 之前的语义），仅 `PX_GC_LIVECHK=1` 诊断档响亮；
#     写入口**保留**（拒写 + 响亮正是挡住「按失效内存改写 + xfree 垃圾指针」的那道闸）。
#
# 判据分层：
#   [1] 静态锚点 21 条（R* route_match · S* sse · C* 缺陷 465 · X* 判定器自证 2）
#   [2] 守卫基线账：待收口 **0** + 有意保留 2
#   [3] 行为 · 路由（默认档）：5 条路由逐字节正确 + CTR **必须为 0**（缺陷 465 默认静默）
#   [4] 行为 · 诊断档：`PX_GC_LIVECHK=1` 同程序 ⇒ CTR **必须 > 0**（检测器没丢）
#   [5] 负控 A：**忠实退回** route_match 旧实现（锁内构造 params）⇒ 守卫必判红并**指名**
#   [6] 负控 B（判据自伤）：清空守卫两张表 ⇒ 负控 A 的红**必须消失**
#   [7] 覆盖边界登记
# 用法：bash examples/m262_lock_alloc2/verify.sh [--neg-skip]
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m262_lock_alloc2
PX=${PX:-$ROOT/tools/px}
BIN=$ROOT/$DIR/build/route_smoke
W=/tmp/m262_gate
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
rm -rf "$W"; mkdir -p "$W/bak"
cp runtime/runtime_route.c "$W/bak/runtime_route.c"
cp runtime/runtime.c "$W/bak/runtime.c"
cp selfhost/check_lock_alloc.py "$W/bak/guard.py"
restore_all() { cp -f "$W/bak/runtime_route.c" runtime/runtime_route.c
                cp -f "$W/bak/runtime.c" runtime/runtime.c
                cp -f "$W/bak/guard.py" selfhost/check_lock_alloc.py; }
trap 'restore_all; rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M262 门：持锁可失败分配收口（二）+ 缺陷 465（容器守卫读入口降噪） =="
echo
echo "[1/7] 静态锚点（21 条：R* 7 · S* 8 · C* 4 · X* 2）"
if python3 "$DIR/check_anchors2.py" --root . >"$W/anchors.log" 2>&1; then
  ok "$(grep '^── ' "$W/anchors.log")"
else
  bad "静态锚点失败"; grep -E '^  ❌' "$W/anchors.log" | sed 's/^/     /'
fi

echo
echo "[2/7] 守卫基线账：待收口 0 + 有意保留 2（M261 的 6 → 2）"
OUT=$(python3 selfhost/check_lock_alloc.py --root . 2>&1); RC=$?
if [ "$RC" = 0 ] && echo "$OUT" | grep -q '待收口 0 + 有意保留 2'; then
  ok "$(echo "$OUT" | head -1)"
else
  bad "基线账不符（rc=$RC）：$OUT"
fi

echo
echo "[3/7] 行为 · 路由（默认档）：5 条路由逐字节 + CTR 必须为 0（缺陷 465）"
if ( cd "$ROOT" && "$PX" build "$DIR/route_smoke.px" ) >"$W/build.log" 2>&1 && [ -x "$BIN" ]; then
  ok "route_smoke 编译通过"
  timeout 90 "$BIN" >"$W/def.out" 2>&1; RC=$?
  if [ "$RC" = 0 ] && grep -q '^M262-ROUTE-SMOKE-DONE$' "$W/def.out"; then
    ok "默认档 rc=0 且跑到结尾"
  else
    bad "默认档异常 rc=$RC"; tail -5 "$W/def.out" | sed 's/^/     /'
  fi
  while IFS='=' read -r k v; do
    if grep -qxF "$k=$v" "$W/def.out"; then ok "路由 $k = $v"; else bad "路由 $k 期望 $v"; fi
  done <<'EXP'
route one: id=42
route two: a=x,b=y
route wild: w=a/b/c
route mix: k=K,w=a/b
route lit: v=v9
EXP
  N=$(grep -c 'M257-CTR' "$W/def.out" || true)
  if [ "$N" = 0 ]; then ok "默认档 CTR 计数 = 0（读入口降噪生效 · 缺陷 465）"
  else bad "默认档仍有 $N 条 M257-CTR（降噪未生效）"; fi
else
  bad "route_smoke 编译失败"; tail -10 "$W/build.log" | sed 's/^/     /'
fi

echo
echo "[4/7] 行为 · 诊断档：PX_GC_LIVECHK=1 ⇒ CTR 必须 > 0（检测器没丢）"
if [ -x "$BIN" ]; then
  PX_GC_LIVECHK=1 timeout 90 "$BIN" >"$W/live.out" 2>&1; RC=$?
  N=$(grep -c 'M257-CTR' "$W/live.out" || true)
  if [ "$N" -gt 0 ]; then ok "诊断档 CTR 计数 = $N（探测式读仍被如实报告）"
  else bad "诊断档 CTR=0 ⇒ 检测器被误删（rc=$RC）"; fi
  grep -qxF 'route one: id=42' "$W/live.out" && ok "诊断档路由结果不变（诊断只加噪音、不改语义）" \
    || bad "诊断档路由结果被改变"
else
  bad "无二进制可跑诊断档"
fi

if [ "$NEG_SKIP" = 0 ]; then
echo
echo "[5/7] 负控 A：**忠实退回** route_match 旧实现（锁内构造 params）⇒ 守卫必判红并指名"
python3 "$DIR/negctl2.py" revert-route-match >"$W/ncA.log" 2>&1 \
  && ok "$(cat "$W/ncA.log")" || bad "植入失败：$(tail -2 "$W/ncA.log")"
OUT=$(python3 selfhost/check_lock_alloc.py --root . 2>&1); RC=$?
if [ "$RC" = 1 ] && echo "$OUT" | grep -q 'route_match'; then
  ok "守卫判红且**指名** route_match（$(echo "$OUT" | grep -c 'route_match') 条）"
else
  bad "守卫未按预期判红（rc=$RC）：$(echo "$OUT" | head -2 | tr '\n' ' ')"
fi
restore_all
if cmp -s "$W/bak/runtime_route.c" runtime/runtime_route.c \
   && cmp -s "$W/bak/runtime.c" runtime/runtime.c; then ok "负控 A 后源逐字节还原"
else bad "还原不完整"; fi

echo
echo "[6/7] 负控 B（判据自伤）：清空守卫两张表 ⇒ 负控 A 的红**必须消失**"
python3 "$DIR/negctl2.py" neuter-guard "$W/g_neut.py" >/dev/null 2>&1 \
  && ok "已生成「判据失明」版守卫" || bad "生成失败"
python3 "$DIR/negctl2.py" revert-route-match >/dev/null 2>&1
OUT=$(python3 "$W/g_neut.py" --root . 2>&1); RC=$?
if [ "$RC" = 0 ] && ! echo "$OUT" | grep -q '❌'; then
  ok "判据失明后不再判红 ⇒ 证明负控 A 的红**来自判据**（rc=0）"
else
  bad "判据失明后仍判红（rc=$RC）⇒ 负控 A 的红来路不明"
fi
restore_all
else
echo; echo "[5-6/7] 负控已跳过（--neg-skip）"
fi

echo
echo "[7/7] 覆盖边界（如实登记）"
echo '   · 本门判据：静态锚点 21 条 + 守卫基线账 + 路由行为（默认/诊断两档）。'
echo '   · SSE 触发路径（bi_sse_read_line）的**动态**覆盖不在本门 —— 它需要真流式上游，'
echo '     由既有门承担：examples/m137_sse_connect_ex（sse_connect_ex + 行读取，含 Python 桩）'
echo '     与 examples/m131_http_stream_post。本门只守它「锁内零分配」的**结构契约**。'
echo '   · 缺陷 465 的 A/B 取证（干净 M261 树 5/5 CTR）在本轮报告与 CHANGELOG 里留证；'
echo '     本门把结论固化为「默认档 0 / 诊断档 >0」两条判据。'
echo '   · 余留 2 处 `ACCEPTED:` 是**有意保留的兜底路径**（gc_register 兜底 / px_list_push_locked 对照）。'
echo
echo "── M262 门结束：pass=$pass fail=$fail"
[ "$fail" = 0 ] && echo "M262-VERIFY-OK" || echo "M262-VERIFY-FAIL"
exit "$fail"
