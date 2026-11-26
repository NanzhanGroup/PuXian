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
  # C3c（缺陷 944）：部署副本住 /usr/local/sbin ⇒ 按脚本位置推导得到 /usr/local，
  # 而 /usr/local 不是仓 ⇒ 旧口径「什么都没找到也不报」= 静默空转。
  # 因此字节一致的部署**必须**由单元提供 PX_REPO，否则部署了等于没部署。
  #
  # M446 补7（2026-11-26 · 缺陷 947）：**bus 不可用 ≠ 机制缺失**。
  #   分片用 unshare --mount --pid --fork --mount-proc，在里面 systemctl 报
  #   「Failed to connect to bus: No data available」⇒ 旧口径把「看不见」读成
  #   「缺 timer / 缺 PX_REPO」⇒ 假红（m446_cache_bounded 复跑仍红 1s 的真因）。
  #   修：先探 bus；bus 不可用 ⇒ 退到**文件级判据**（unit/drop-in/timers.target.wants），
  #   并在输出里明写「降级」——降级本身要看得见，不能洗成普通绿。
  UNIT=dy-px-cache-gc.service
  TIMER=dy-px-cache-gc.timer
  BUS_OK=0
  # 探 bus 用 show，**不要用 is-system-running**：宿主 degraded 时它 rc=1（本机现值就是
  # degraded）⇒ 会把「bus 好用」误判成「不可用」，那是同一家族的假红。
  systemctl show --property=Version --value >/dev/null 2>&1 && BUS_OK=1
  UDIR=/etc/systemd/system

  if [ "$BUS_OK" = 1 ]; then
    if systemctl cat "$UNIT" >/dev/null 2>&1; then
      if systemctl cat "$UNIT" 2>/dev/null | grep -qE '^Environment=.*PX_REPO=[^ ]'; then
        ok "C3c 单元已给 PX_REPO（部署副本不会推导成 /usr/local）"
      else
        bad "C3c $UNIT 缺 PX_REPO ⇒ 部署副本 REPO 推导成 /usr/local 并**静默空转**（缺陷 944）"
      fi
    else
      ok "C3c 跳过：本机无 $UNIT"
    fi
  else
    # 文件级：主 unit 或任一 drop-in 给出 PX_REPO 即算（drop-in 文件名不固定，按内容找）
    if [ -f "$UDIR/$UNIT" ]; then
      if grep -rqsE '^[[:space:]]*Environment=.*PX_REPO=[^ ]' "$UDIR/$UNIT" "$UDIR/$UNIT.d" 2>/dev/null; then
        ok "C3c 文件级判据（bus 不可用 ⇒ 降级）：$UNIT 或其 drop-in 已给 PX_REPO"
      else
        bad "C3c 文件级判据：$UNIT 在场但无 PX_REPO ⇒ 部署副本会静默空转（缺陷 944）"
      fi
    else
      ok "C3c 跳过：本机无 $UNIT（bus 不可用，文件级判据同样找不到）"
    fi
  fi

  if [ "$BUS_OK" = 1 ]; then
    if systemctl cat "$TIMER" >/dev/null 2>&1; then
      if systemctl is-enabled "$TIMER" >/dev/null 2>&1; then
        ok "C3b 定时器已安装且 enabled"
      else
        bad "C3b 定时器存在但未 enabled（等于不会跑）"
      fi
    else
      bad "C3b 缺 $TIMER（机制没有驱动力）"
    fi
  else
    # 文件级：timer 文件在场 **且** 挂在 timers.target.wants（否则等于不启用）
    if [ -f "$UDIR/$TIMER" ]; then
      if [ -L "$UDIR/timers.target.wants/$TIMER" ] || [ -f "$UDIR/timers.target.wants/$TIMER" ]; then
        ok "C3b 文件级判据（bus 不可用 ⇒ 降级）：$TIMER 在场且已挂 timers.target.wants"
      else
        bad "C3b 文件级判据：$TIMER 在场但未 enabled（缺 timers.target.wants 链接 ⇒ 不会跑）"
      fi
    else
      bad "C3b 缺 $TIMER（机制没有驱动力）"
    fi
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

# 末行口径（缺陷 506 家族）：有红时**不许**以 OK 收尾 —— 否则「末行判红」正则与 rc
# 会给出互相矛盾的信号，后来者无法判断是门绿了还是被兜底拦了。
if [ "$bad_n" = 0 ]; then
  echo "CACHE-BOUNDED-OK pass=$ok_n fail=0"
  exit 0
else
  echo "CACHE-BOUNDED-RED pass=$ok_n fail=$bad_n"
  exit 1
fi
