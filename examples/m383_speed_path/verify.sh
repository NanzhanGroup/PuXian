#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M383 · `px run --fast` 热态加速：工具链指纹缓存 + 主脚本 AST 单次解析
# ---------------------------------------------------------------------------
# 病根（2026-11-09 实测本机，`px run --fast hello.px` 热态中位数 845 ms）：
#   px --version 自调用 38 ms + px rtkey 自调用（30 次 fork sha256sum）461 ms
#   + sha256sum pxc_vm 73 ms + sha256sum pxi 98 ms = 670 ms（79%）—— 全是**常量**；
#   加上 run_subset_ok 与 px_dep_closure 对同一主脚本各解析一次 AST（26 ms × 2）。
#
# 本门守的是「那条快路径没被写坏」：
#   [1] 快路径 key == 强制重算(TTL=0) key      —— 缓存不许改变 key（语义契约）
#   [2] 篡改指纹缓存 ⇒ key 必须变              —— **负控**：证明缓存真被读；
#       若实现退化成「每次重算」，这条会红（这正是最容易被悄悄改坏的方向）
#   [3] TTL=0 ⇒ stamp 必须更新                 —— 兜底路径在位
#   [4] execve 次数 ≤ 40（strace 在时）        —— 旧实现 149；防 spawn 面回退
#   [5] 大 AST（>32768 字符）key == TTL=0      —— 不 export 分支正确
#
# 隔离：全程用临时 PX_RUNCACHE_DIR，不碰仓库 .runcache。
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMPC="${TMPDIR:-/tmp}/m383_speed_cache.$$"
rm -rf "$TMPC"; mkdir -p "$TMPC"
trap 'rm -rf "$TMPC"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

keyof() { # $1=脚本 ⇒ 16 位 key（空 = 未取得，例如回落解释轨）
    PX_RUNCACHE_DIR="$TMPC" PX_RUN_VERBOSE=1 "$PX" run --fast "$1" 2>&1 \
        | sed -n 's/.*key=\([a-f0-9]\{16\}\).*/\1/p' | head -1
}

S="$ROOT/examples/hello.px"
[ -f "$S" ] || { echo "缺少 $S"; exit 1; }

echo "── [1] 快路径 key == 强制重算 key（缓存不许改变 key）"
K0="$(keyof "$S")"
if [ -n "$K0" ]; then ok "取得 key=$K0"; else bad "未取得 key（--fast 是否回落了？）"; fi
K1="$(PX_TOOLCHAIN_FP_TTL=0 keyof "$S")"
if [ -n "$K0" ] && [ "$K0" = "$K1" ]; then
    ok "快路径 == 强制重算（$K0）"
else
    bad "key 不一致 fast=$K0 slow=$K1"
fi

echo "── [2] 负控：篡改指纹缓存 ⇒ key 必须变（证明缓存真被读）"
FP="$TMPC/.toolchain.fp"
if [ -f "$FP" ]; then
    cp -p "$FP" "$FP.bak"
    # 只改内容、保留 #SIZES 行与 mtime 语义 ⇒ 仍是「快路径可命中」的状态
    awk 'NR==3{print "deadbeefdeadbeef"; next} {print}' "$FP.bak" > "$FP"
    K2="$(keyof "$S")"
    if [ -n "$K2" ] && [ "$K0" != "$K2" ]; then
        ok "篡改后 key 变化（$K0 → $K2）⇒ 缓存确实被使用"
    else
        bad "篡改指纹后 key 未变（K0=$K0 K2=$K2）⇒ 快路径退化成每次重算"
    fi
    cp -p "$FP.bak" "$FP"
    K3="$(keyof "$S")"
    if [ "$K0" = "$K3" ]; then ok "恢复后 key 回到原值（$K3）"; else bad "恢复后 key 未复原（$K3）"; fi
else
    bad "未生成指纹缓存文件 $FP"
fi

echo "── [3] TTL=0 ⇒ stamp 必须更新（兜底路径在位）"
ST="$TMPC/.toolchain.stamp"
M0="$(stat -c %Y "$ST" 2>/dev/null || echo 0)"
sleep 1
PX_TOOLCHAIN_FP_TTL=0 keyof "$S" >/dev/null 2>&1
M1="$(stat -c %Y "$ST" 2>/dev/null || echo 0)"
if [ "$M1" -gt "$M0" ]; then
    ok "TTL=0 触发重算（stamp $M0 → $M1）"
else
    bad "TTL=0 未触发重算（stamp=$M1）⇒ 兜底失效"
fi

echo "── [4] execve 次数上界（旧实现 149；修复后实测 29）"
if command -v strace >/dev/null 2>&1; then
    N="$(strace -f -c -e trace=execve env PX_RUNCACHE_DIR="$TMPC" "$PX" run --fast "$S" \
         2>&1 >/dev/null | awk '/[[:space:]]execve$/{print $4; exit}')"
    if [ -n "${N:-}" ] && [ "$N" -le 40 ]; then
        ok "execve=$N ≤ 40"
    else
        bad "execve=${N:-未取得} > 40 或未取得 ⇒ spawn 面回退"
    fi
else
    echo "  ⏭  SKIP：无 strace，无法断言 spawn 数（不影响其它判据）"
fi

echo "── [5] 大 AST 脚本（>32768 字符，不 export 分支）key 一致"
BIG="$ROOT/examples/p8_slice_base64.px"
if [ -f "$BIG" ]; then
    KB="$(keyof "$BIG")"
    KB2="$(PX_TOOLCHAIN_FP_TTL=0 keyof "$BIG")"
    if [ -n "$KB" ] && [ "$KB" = "$KB2" ]; then
        ok "大 AST key 一致（$KB）"
    else
        bad "大 AST key 不一致 fast=$KB slow=$KB2"
    fi
else
    bad "缺少 $BIG"
fi

echo
echo "── 结果：$pass 通过 / $fail 失败"
if [ "$fail" -eq 0 ]; then
    echo "M383-VERIFY-OK"
    exit 0
fi
echo "M383-VERIFY-FAIL"
exit 1
