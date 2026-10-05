#!/usr/bin/env bash
# ============================================================
# M263 门：**「响应串味」的定向复现 + 生命周期审查 + 缺陷 466**
# ------------------------------------------------------------
# 来历：M260 在 m128 门收编后记录「C10 的 `/list?n=20000` 有概率拿到 `/inv` 形态的 body（缺陷 440）」，
#   并称「M256 源码树同样复现」⇒ 判为 pre-existing。**本轮把它查到底**：
#   [A] M257 之后我看到的两份 M256 树日志（`wt_m128_1/2.log`）里的失败**都是**
#       `/inv?kind=list tail=999999`（**门自己的判据 bug**，M260 已另修）—— **没有一份**是串味。
#   [B] 定向复现器（本门）：起全新进程 × N 轮 × 每轮 7 条断言，**逐字节**核对「答非所问」。
#       实测 **60 轮 / 420 断言 / 零异常**（另加 C9 前戏 30 轮 / 210 断言，同样零异常）。
#   [C] 静态审查：串味最可信的机制是「连接级余留字节 `pbuf` 在 **fd 复用** 时没被清掉」
#       ⇒ 查「每个**丢弃型**释放点都必须重置 `c->fd`（或走 acquire 接管）」+「关闭路径必须清
#         挂起 handler 表」。后者**当场照出缺陷 466**（空闲超时关闭路径裸 `close(fd)`）⇒ 已修。
#   [D] **去掉掩盖**：m128 的 C10 原写「疑似 440 ⇒ 重试一次 + 打一行 ℹ️」= 把真发生静默吞掉
#       ⇒ 收紧为「**零重试**」断言。
# 判据分层：
#   [1] 生命周期审查 9 条（含判定器自证 2）
#   [2] 定向复现：N 轮 CLEAN（CI 档 4 轮 · 本地默认 12 轮）
#   [3] 负控 A：**退回缺陷 466**（去掉 `http_pend_clear`）⇒ 生命周期审查 L3 必判红
#   [4] 负控 B：**构造「答非所问」**（把 `/list` 应答改成 `/inv` 形状）⇒ 复现器必判红
#   [5] 负控 C（判据自伤）：把复现器的比对改成恒真 ⇒ 负控 B 的红**必须消失**
#   [6] m128 C10「零重试」断言在位（静态）
#   [7] 覆盖边界
# 用法：bash examples/m263_resp_crosstalk/verify.sh [--neg-skip] [--rounds N]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m263_resp_crosstalk
PX=${PX:-$ROOT/tools/px}
W=/tmp/m263_gate
NEG_SKIP=0
ROUNDS=12
while [ $# -gt 0 ]; do
  case "$1" in
    --neg-skip) NEG_SKIP=1 ;;
    --rounds) shift; ROUNDS=${1:-12} ;;
    *) echo "未知参数 $1"; exit 2 ;;
  esac
  shift
done
rm -rf "$W"; mkdir -p "$W/bak"
cp runtime/runtime.c "$W/bak/runtime.c"
cp "$DIR/srv.px" "$W/bak/srv.px"
restore_all() { cp -f "$W/bak/runtime.c" runtime/runtime.c; cp -f "$W/bak/srv.px" "$DIR/srv.px"; }
trap 'restore_all; rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M263 门：「响应串味」定向复现 + pbuf/fd 生命周期审查 + 缺陷 466 =="
echo
echo "[1/7] 生命周期审查（9 条：L1–L6 + 判定器自证 X1/X2）"
if python3 "$DIR/check_lifecycle.py" --root . >"$W/lc.log" 2>&1; then
  ok "$(grep '^── ' "$W/lc.log")"
else
  bad "生命周期审查失败"; grep -E '^  ❌' "$W/lc.log" | sed 's/^/     /'
fi

echo
echo "[2/7] 定向复现：$ROUNDS 轮（每轮 7 条断言，逐字节核「答非所问」）"
if ( cd "$ROOT" && "$PX" build "$DIR/srv.px" ) >"$W/build.log" 2>&1 \
   && [ -x "$DIR/build/srv" ]; then
  ok "复现用服务端编译通过"
  if M263_W="$W" bash "$DIR/repro.sh" "$ROUNDS" >"$W/repro.log" 2>&1; then
    ok "$(grep '^M263-REPRO：' "$W/repro.log")"
  else
    bad "复现器报异常"; grep '❌' "$W/repro.log" | head -5 | sed 's/^/     /'
  fi
else
  bad "服务端编译失败"; tail -8 "$W/build.log" | sed 's/^/     /'
fi

if [ "$NEG_SKIP" = 0 ]; then
echo
echo "[3/7] 负控 A：**退回缺陷 466**（关闭路径去掉 http_pend_clear）⇒ 生命周期审查 L3 必判红"
if sed -i 's/else { http_pend_clear(fd); close(fd); }/else close(fd);/g' runtime/runtime.c \
   && grep -q 'else close(fd);' runtime/runtime.c; then
  ok "已植入缺陷 466 的原始形态"
  OUT=$(python3 "$DIR/check_lifecycle.py" --root . 2>&1); RC=$?
  if [ "$RC" = 1 ] && echo "$OUT" | grep -q 'L3'; then
    ok "生命周期审查判红且指向 L3：$(echo "$OUT" | grep '❌ L3' | cut -c1-80)"
  else
    bad "未按预期判红（rc=$RC）：$(echo "$OUT" | grep '❌' | head -2 | tr '\n' ' ')"
  fi
else
  bad "植入失败"
fi
restore_all
cmp -s "$W/bak/runtime.c" runtime/runtime.c && ok "负控 A 后源逐字节还原" || bad "还原不完整"

echo
echo "[4/7] 负控 B：**构造「答非所问」**（/list 的应答改成 /inv 形状）⇒ 复现器必判红"
if sed -i 's|return "list=" + str(len(l))|return "inv first=" + str(0) + " n=" + str(len(l))|' "$DIR/srv.px" \
   && grep -q 'inv first=' "$DIR/srv.px"; then
  ok "已把 /list 的应答构造成 /inv 形状"
  if ( cd "$ROOT" && "$PX" build "$DIR/srv.px" ) >"$W/b2.log" 2>&1; then
    OUT=$(M263_W="$W" bash "$DIR/repro.sh" 2 >"$W/reproB.log" 2>&1); RC=$?
    if [ "$RC" = 1 ] && grep -q '答非所问\|串味\|❌' "$W/reproB.log"; then
      ok "复现器判红（$(grep -m1 '❌' "$W/reproB.log" | cut -c1-70)）"
    else
      bad "复现器未判红（rc=$RC）"; tail -4 "$W/reproB.log" | sed 's/^/     /'
    fi
  else
    bad "负控 B 的服务端编译失败"; tail -6 "$W/b2.log" | sed 's/^/     /'
  fi
else
  bad "构造失败"
fi
restore_all
cmp -s "$W/bak/srv.px" "$DIR/srv.px" && ok "负控 B 后语料逐字节还原" || bad "语料还原不完整"

echo
echo "[5/7] 负控 C（判据自伤）：把复现器的比对改成恒真 ⇒ 负控 B 的红**必须消失**"
# ⚠️ 门自身 bug（本轮踩）：复现器有**两处**独立异常判据（逐字节比对 + 互斥形状），
  #   只中性化第一处 ⇒ 第二处仍报 ⇒ 假红。⇒ 收敛到**唯一出口** `flag_anom` 后整行中性化。
  sed 's/^flag_anom() {.*$/flag_anom() { :; }/' "$DIR/repro.sh" > "$W/repro_neut.sh"
if ! cmp -s "$DIR/repro.sh" "$W/repro_neut.sh"; then
  ok "已生成「比对恒真」版复现器"
  sed -i 's|return "list=" + str(len(l))|return "inv first=" + str(0) + " n=" + str(len(l))|' "$DIR/srv.px"
  ( cd "$ROOT" && "$PX" build "$DIR/srv.px" ) >"$W/b3.log" 2>&1 || true
  # ⚠️ 门自身 bug（本轮踩）：`repro.sh` 用 `cd "$(dirname "$0")/../.."` 定位仓库根，
  #   而失明版副本落在 `$W` ⇒ `$ROOT` 算错 ⇒ 找不到二进制 ⇒ **rc=2**（不是「判据失明」）。
  #   ⇒ 一律把**绝对**二进制路径当第二个参数传进去（`repro.sh` 支持）。
  OUT=$(M263_W="$W" bash "$W/repro_neut.sh" 2 "$ROOT/$DIR/build/srv" >"$W/reproC.log" 2>&1); RC=$?
  if [ "$RC" = 0 ]; then
    ok "判据失明后不再判红 ⇒ 证明负控 B 的红**来自判据**（rc=0）"
  else
    bad "判据失明后仍判红（rc=$RC）⇒ 负控 B 的红来路不明"
  fi
  restore_all
  ( cd "$ROOT" && "$PX" build "$DIR/srv.px" ) >/dev/null 2>&1 || true
else
  bad "未能生成失明版（锚点失配）"
fi
else
echo; echo "[3-5/7] 负控已跳过（--neg-skip）"
fi

echo
echo "[6/7] m128 C10「**零重试**」断言在位（去掉掩盖 · 静态）"
if grep -q 'C10 零重试' examples/m128_unlock_grow/verify.sh \
   && grep -q 'M263：\*\*去掉掩盖\*\*' examples/m128_unlock_grow/verify.sh; then
  ok "m128 C10 已把「疑似 440 ⇒ 重试」改为「零重试否则判红」"
else
  bad "m128 C10 的去掩盖断言不在位"
fi
if grep -q '缺陷 440 · 已登记未修' examples/m128_unlock_grow/verify.sh; then
  bad "m128 里仍留着「缺陷 440 已登记未修」的旧措辞"
else
  ok "旧措辞已清（不再把未复现项当成待修缺陷）"
fi

echo
echo "[7/7] 覆盖边界（如实登记）"
echo '   · 复现器用的是**自建**服务端（px_serve + 路由 + 重分配 + 协程），形状对齐 m128 现场，'
echo '     但**不复刻** m128 的注入前戏；后者另有一条 30 轮 / 210 断言的记录（详见 CHANGELOG）。'
echo '   · 「串味」这一类故障**没有可判定的一般判据**（它是竞态的外观）⇒ 本门给的是一条'
echo '     **可重复的观测窗口** + **生命周期静态审查**；两者都清不等于数学上不可能。'
echo '   · 缺陷 466 的**可复现性**未做（需要「挂起项残留 + fd 复用」的同时发生）；本门给的是'
echo '     静态判据 + 负控（退回原形态必判红）。'
echo
echo "── M263 门结束：pass=$pass fail=$fail"
[ "$fail" = 0 ] && echo "M263-VERIFY-OK" || echo "M263-VERIFY-FAIL"
exit "$fail"
