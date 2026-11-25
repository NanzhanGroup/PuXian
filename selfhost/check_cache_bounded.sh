#!/usr/bin/env bash
# ============================================================
# check_cache_bounded.sh —— 编译缓存回收机制必须**存在且有界**（缺陷 866-2）
# ------------------------------------------------------------
# 为什么有它：
#   /data/code/puxian/.buildcache 曾从 2.3G/588 条涨到 4.9G/1206 条（全量门一轮 +25MB），
#   从来没人管；原来只有 /tmp 里的一次性脚本（早被清）。⇒「有界」必须是一条门，
#   不是「某人某天想起来删一下」。
#
# 判据：
#   C1 仓库内 selfhost/px_cache_gc.sh 存在、语法可过（权威副本在仓库里，CI 也能跑）
#   C2 --self-test 全绿（N1 只删过期键 / P1 保护项 / N2 上限不可达 rc=4 / N3 显式保护键）
#   C3 部署面（仅本机）：/usr/local/sbin/dy-px-cache-gc.sh 与仓库内逐字节一致（防漂移）
#      + dy-px-cache-gc.timer 存在 —— 不存在时记 SKIP（CI 无 systemd 单元，不判红）
#   N1 负控：把「保护年轻键」的 -mmin +YOUNG_MIN 改成 +0 ⇒ 自测必须变红（判据有牙）
#   G1 缺脚本 ⇒ rc=3（不降级成 0）
#
# 用法：check_cache_bounded.sh [--repo DIR]
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) ROOT="$2"; shift 2 ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

GC="$ROOT/selfhost/px_cache_gc.sh"
ok_n=0; bad_n=0
ok()  { echo "✅ $*"; ok_n=$((ok_n+1)); }
bad() { echo "❌ $*"; bad_n=$((bad_n+1)); }

[ -f "$GC" ] || { echo "ENV: 缺 $GC"; exit 3; }
bash -n "$GC" || { echo "ENV: $GC 语法错误"; exit 3; }
ok "C1 回收脚本在场且语法可过（$(wc -l < "$GC") 行）"

OUT=$(bash "$GC" --self-test 2>&1); RC=$?
if [ "$RC" = 0 ] && printf '%s' "$OUT" | grep -q 'SELFTEST-OK'; then
  n=$(printf '%s\n' "$OUT" | grep -c '^✅')
  ok "C2 自测全绿（$n 项）"
else
  bad "C2 自测失败（rc=$RC）"; printf '%s\n' "$OUT" | sed 's/^/     /'
fi

DEPLOY=/usr/local/sbin/dy-px-cache-gc.sh
if [ -f "$DEPLOY" ]; then
  if cmp -s "$DEPLOY" "$GC"; then
    ok "C3 部署副本与仓库内一致"
  else
    bad "C3 部署副本已漂移（$DEPLOY ≠ selfhost/px_cache_gc.sh）⇒ 以仓库为准重新部署"
  fi
  if systemctl cat dy-px-cache-gc.timer >/dev/null 2>&1; then
    if systemctl is-enabled dy-px-cache-gc.timer >/dev/null 2>&1; then
      ok "C3b 定时器已安装且 enabled"
    else
      bad "C3b 定时器存在但未 enabled（等于不会跑）"
    fi
  else
    bad "C3b 缺 dy-px-cache-gc.timer（机制没有驱动力）"
  fi
else
  echo "⏭  C3 跳过：本机未部署（CI 环境正常）"
fi

# N1：破坏「保护年轻键」⇒ 自测必须红
TMP=$(mktemp -d /tmp/cachebnd.XXXXXX) || exit 3
trap 'rm -rf "$TMP"' EXIT
sed 's/-mmin +"\$YOUNG_MIN"/-mmin +0/g' "$GC" > "$TMP/gc_neg.sh"
if cmp -s "$TMP/gc_neg.sh" "$GC"; then
  bad "N1 负控未生效（替换没命中，判据无法自证）"
else
  if bash "$TMP/gc_neg.sh" --self-test >/dev/null 2>&1; then
    bad "N1 负控：去掉年轻键保护后自测仍绿 ⇒ 判据没牙"
  else
    ok "N1 负控：去掉年轻键保护 ⇒ 自测转红"
  fi
fi

echo "CACHE-BOUNDED-OK pass=$ok_n fail=$bad_n"
[ "$bad_n" = 0 ] && exit 0 || exit 1
