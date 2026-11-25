#!/bin/bash
# ============================================================
# dy-px-cache-gc.sh —— PuXian 编译缓存的**有界**回收（缺陷 866-2）
# ------------------------------------------------------------
# 为什么有它（2026-11-25 实测）：
#   /data/code/puxian/.buildcache 4.9G / 1206 条（11-24 记录 2.3G / 588 —— 已翻倍）
#   全量门一轮 +25MB，从不回收 ⇒ 磁盘单增。原来只有一次性脚本（在 /tmp，早被清），
#   缺的是「常驻 + 有上限 + 自己不静默失败」的机制。
#
# 判据（不是「删了点东西」）：
#   P1 保护项必须仍在：.rtcache/bin · .rtcache/.semmap（显式 PX_KEEP_KEY 也要在）
#   P2 上限生效：回收后 buildcache ≤ MAX_BUILD_MB、rtcache ≤ MAX_RT_MB；否则 rc=4
#   P3 正在用的键不碰：mtime 在 YOUNG_MIN 分钟内的键跳过
#   P4 只删「目录形键」（32 位 / 16 位十六进制名）；bin、.semmap 等非键项一律不动
#   N1 负控：3 个过期键 + 1 个年轻键 ⇒ 只删过期键
#   N2 负控：上限不可达 ⇒ rc=4（不许「删不动当成功」）
#
# 内部单位一律 KB（du -sm 对小于 1MB 的目录取整为 0，会把判据变哑）。
#
# 用法：dy-px-cache-gc.sh [--check] [--self-test] [--repo DIR]
# 退出码：0 成功 / 1 判据失败（保护项缺失等）/ 3 环境不成立 / 4 上限未达成
# ============================================================
set -uo pipefail

REPO=${PX_REPO:-/data/code/puxian}
MAX_BUILD_MB=${MAX_BUILD_MB:-6000}
MAX_RT_MB=${MAX_RT_MB:-5000}
YOUNG_MIN=${YOUNG_MIN:-120}
MODE=apply

while [ $# -gt 0 ]; do
  case "$1" in
    --check)     MODE=check; shift ;;
    --self-test) MODE=selftest; shift ;;
    --repo)      REPO="$2"; shift 2 ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

log()  { echo "[$(date '+%F %T')] $*"; }
dir_kb(){ du -sk "$1" 2>/dev/null | awk '{print $1}'; }
mb()   { awk -v k="$1" 'BEGIN{printf "%.1f", k/1024}'; }

# ── 自测（夹具在 /tmp，不碰真缓存）────────────────────────────
selftest() {
  local rc=0 old
  old=$(date -d '10 days ago' '+%Y%m%d%H%M' 2>/dev/null) || old=197001010000

  local T; T=$(mktemp -d /tmp/pxcachegc.XXXXXX) || return 3
  mkdir -p "$T/.rtcache/bin" "$T/.buildcache"; touch "$T/.rtcache/.semmap"
  local k
  for k in aaaa0000000000000000000000000001 bbbb0000000000000000000000000001 cccc0000000000000000000000000001; do
    mkdir -p "$T/.buildcache/$k"; dd if=/dev/zero of="$T/.buildcache/$k/o" bs=1M count=2 2>/dev/null
    touch -t "$old" "$T/.buildcache/$k" "$T/.buildcache/$k/o"
  done
  mkdir -p "$T/.buildcache/eeee0000000000000000000000000001"; dd if=/dev/zero of="$T/.buildcache/eeee0000000000000000000000000001/o" bs=1M count=2 2>/dev/null

  MAX_BUILD_MB=0 MAX_RT_MB=0 YOUNG_MIN=120 PX_KEEP_KEY= _run_gc "$T" quiet >/dev/null 2>&1
  local rc1=$?
  local left; left=$(ls -1 "$T/.buildcache" 2>/dev/null | grep -cE '^[0-9a-f]{32}$')
  if [ "$left" = 1 ] && [ -d "$T/.buildcache/eeee0000000000000000000000000001" ]; then
    echo "✅ N1 只删过期键（留年轻键 1 个）"
  else
    echo "❌ N1 期望留 1 个，实际 $left"; rc=2
  fi
  if [ -d "$T/.rtcache/bin" ] && [ -f "$T/.rtcache/.semmap" ]; then
    echo "✅ P1 保护项仍在"
  else
    echo "❌ P1 保护项被删"; rc=2
  fi
  if [ "$rc1" = 4 ]; then
    echo "✅ N2 上限不可达 ⇒ rc=4（响亮，不静默）"
  else
    echo "❌ N2 期望 rc=4，实际 rc=$rc1"; rc=2
  fi
  rm -rf "$T"

  # N3：显式保护键不许被删
  local T2; T2=$(mktemp -d /tmp/pxcachegc2.XXXXXX)
  mkdir -p "$T2/.rtcache/bin" "$T2/.buildcache"; touch "$T2/.rtcache/.semmap"
  mkdir -p "$T2/.rtcache/1111000000000001" "$T2/.rtcache/2222000000000002"
  dd if=/dev/zero of="$T2/.rtcache/1111000000000001/o" bs=1M count=2 2>/dev/null
  dd if=/dev/zero of="$T2/.rtcache/2222000000000002/o" bs=1M count=2 2>/dev/null
  touch -t "$old" "$T2/.rtcache/1111000000000001" "$T2/.rtcache/2222000000000002"
  MAX_BUILD_MB=9999 MAX_RT_MB=0 YOUNG_MIN=120 PX_KEEP_KEY=1111000000000001 _run_gc "$T2" quiet >/dev/null 2>&1
  local rc3=$?
  if [ "$rc3" = 4 ] && [ -d "$T2/.rtcache/1111000000000001" ] && [ ! -d "$T2/.rtcache/2222000000000002" ]; then
    echo "✅ N3 显式保护键保留、其余过期键回收"
  else
    echo "❌ N3 期望保 key1111 删 key2222 且 rc=4（实际 rc=$rc3）"; rc=2
  fi
  rm -rf "$T2"

  [ $rc = 0 ] && echo "SELFTEST-OK" || echo "SELFTEST-FAIL"
  return $rc
}

# ── 真回收（$1=repo $2=quiet）────────────────────────────────
_run_gc() {
  local R="$1" Q="${2:-}"
  local B="$R/.buildcache" RT="$R/.rtcache"
  [ -d "$B" ] || { echo "ENV: 缺 $B"; return 3; }
  [ -d "$RT" ] || { echo "ENV: 缺 $RT"; return 3; }
  local keep="${PX_KEEP_KEY:-}"

  local bkb rkb; bkb=$(dir_kb "$B"); rkb=$(dir_kb "$RT")
  [ -z "$Q" ] && log "before: buildcache=$(mb "$bkb")MB rtcache=$(mb "$rkb")MB keep=${keep:-none}"

  local max_b=$(( MAX_BUILD_MB * 1024 )) max_r=$(( MAX_RT_MB * 1024 ))
  local removed=0 freed=0 kb

  if [ "$MODE" != check ] && [ "$bkb" -gt "$max_b" ]; then
    while IFS= read -r d; do
      [ "$bkb" -le "$max_b" ] && break
      kb=$(dir_kb "$d"); rm -rf "$d" && { bkb=$((bkb - kb)); removed=$((removed+1)); freed=$((freed+kb)); }
    done < <(find "$B" -maxdepth 1 -type d -regextype posix-extended -regex '.*/[0-9a-f]{32}$' \
             -mmin +"$YOUNG_MIN" -printf '%T@ %p\n' 2>/dev/null | sort -n | cut -d' ' -f2-)
  fi

  if [ "$MODE" != check ] && [ "$rkb" -gt "$max_r" ]; then
    while IFS= read -r d; do
      [ "$rkb" -le "$max_r" ] && break
      [ -n "$keep" ] && [ "$(basename "$d")" = "$keep" ] && continue
      kb=$(dir_kb "$d"); rm -rf "$d" && { rkb=$((rkb - kb)); removed=$((removed+1)); freed=$((freed+kb)); }
    done < <(find "$RT" -maxdepth 1 -type d -regextype posix-extended -regex '.*/[0-9a-f]{16}$' \
             -mmin +"$YOUNG_MIN" -printf '%T@ %p\n' 2>/dev/null | sort -n | cut -d' ' -f2-)
  fi

  local rc=0
  [ -d "$RT/bin" ]    || { echo "P1-FAIL: .rtcache/bin 缺失"; rc=1; }
  [ -f "$RT/.semmap" ]|| { echo "P1-FAIL: .rtcache/.semmap 缺失"; rc=1; }
  if [ -n "$keep" ] && [ ! -d "$RT/$keep" ]; then echo "P1-FAIL: 显式保护键 $keep 被删"; rc=1; fi

  bkb=$(dir_kb "$B"); rkb=$(dir_kb "$RT")
  if [ "$MODE" != check ]; then
    [ "$bkb" -gt "$max_b" ] && { echo "P2-FAIL: buildcache $(mb "$bkb")MB > ${MAX_BUILD_MB}MB（可删键不足，需人工）"; rc=4; }
    [ "$rkb" -gt "$max_r" ] && { echo "P2-FAIL: rtcache $(mb "$rkb")MB > ${MAX_RT_MB}MB（可删键不足，需人工）"; rc=4; }
  fi
  [ -z "$Q" ] && log "after: buildcache=$(mb "$bkb")MB rtcache=$(mb "$rkb")MB removed=$removed freed=$(mb "$freed")MB rc=$rc"
  return $rc
}

case "$MODE" in
  selftest) selftest; exit $? ;;
  check)    _run_gc "$REPO"; exit $? ;;
  apply)    _run_gc "$REPO"; exit $? ;;
esac
