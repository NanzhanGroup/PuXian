#!/usr/bin/env bash
# ============================================================
# selfhost/check_orphan_gates.sh —— **孤儿门守卫**（M249 建立 · 缺陷 428）
# ------------------------------------------------------------
# 判据：`examples/*/verify.sh` 里每一扇门必须被**至少一个运行器**收录
#       （`selfhost/gates.registry.sh` 或 `.github/workflows/ci.yml`），
#       否则必须出现在 `selfhost/orphan_gates.txt`（**带理由**）；基线**只许缩小**。
#
# 为什么要有它（M249 实测数据）：
#   `check_gate_registry.sh`（M243）比的是 **registry ⇄ ci.yml**，
#   而「**两边都没提到**」的门**不在任何差集里** ⇒ 那半条判据**看不见**。
#   实测：179 个 verify.sh ⇄ registry 124 / ci.yml 127
#        ⇒ **55 扇门没有任何运行器跑它们**（其中 4 扇 ci.yml 有 ⇒ 真孤儿 51）。
#
# 后果（= 用户 2026-10-03 点名修的那条，就是这个形状）：
#   `m63_langfix` 的判据写着 `grep "pxc 0.1.0"`，实测早已是 `0.2.0`
#   ⇒ **判据长期恒红**；它依赖的 `/tmp/m63_mock.py` 被清理后也**无人发现**
#   ⇒ 门实际早已跑不起来。**孤儿门 = 判据腐烂的温床**（没人跑 ⇒ 没人发现 ⇒ 没人修）。
#
# 用法：./selfhost/check_orphan_gates.sh [--self-test]
# 退出码：0 绿 · 1 红 · 2 参数/环境错
# ============================================================
set -uo pipefail

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="${ORPHAN_ROOT:-$(cd "$SELF_DIR/.." && pwd)}"
cd "$ROOT" || { echo "❌ 进不去仓库根 $ROOT" >&2; exit 2; }

EX_DIR="${ORPHAN_EX_DIR:-examples}"
REG="${ORPHAN_REGISTRY:-selfhost/gates.registry.sh}"
CI="${ORPHAN_CI:-.github/workflows/ci.yml}"
BASE="${ORPHAN_BASELINE:-selfhost/orphan_gates.txt}"
# 基线规模上限 —— 写死常量（**只许缩小**：要扩充就得改这个数字，留下痕迹）
ORPHAN_MAX="${ORPHAN_MAX:-50}"

PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT

echo "=== 孤儿门守卫（M249 · 缺陷 428）==="

# ── 1 三向提取（全从源码派生，不手抄任何名单）──
find "$EX_DIR" -mindepth 2 -maxdepth 2 -name verify.sh 2>/dev/null \
    | sed 's|^\./||' | awk -F/ '{print $2}' | sort -u > "$T/all.txt"
grep -oE 'examples/[A-Za-z0-9_-]+/verify\.sh' "$REG" 2>/dev/null \
    | sed 's|examples/||; s|/verify\.sh||' | sort -u > "$T/reg.txt"
grep -oE 'examples/[A-Za-z0-9_-]+/verify\.sh' "$CI" 2>/dev/null \
    | sed 's|examples/||; s|/verify\.sh||' | sort -u > "$T/ci.txt"
cat "$T/reg.txt" "$T/ci.txt" | sort -u > "$T/covered.txt"

NA=$(wc -l < "$T/all.txt")
NR=$(wc -l < "$T/reg.txt")
NC=$(wc -l < "$T/ci.txt")
echo "  实存门 $NA · registry $NR · ci.yml $NC"

comm -23 "$T/all.txt" "$T/covered.txt" > "$T/orphan.txt"
NO=$(wc -l < "$T/orphan.txt")
echo "  孤儿门（两个运行器都没收录）= $NO"

# ── 2 规模锚点（防「提取器坏了 ⇒ 空集 ⊇ 空集」的假绿）──
[ "$NA" -ge 150 ] && ok "实存门规模下限（≥150）" || bad "实存门只 $NA 个 —— find 提取器坏了？"
[ "$NR" -ge 100 ] && ok "registry 规模下限（≥100）" || bad "registry 只 $NR 个 —— 提取器坏了？"
[ "$NC" -ge 100 ] && ok "ci.yml 规模下限（≥100）" || bad "ci.yml 只 $NC 个 —— 提取器坏了？"

# ── 3 基线表 ──
if [ ! -f "$BASE" ]; then
    bad "基线表不存在：$BASE（建它，或把孤儿清零）"
else
    grep -vE '^[[:space:]]*(#|$)' "$BASE" | cut -d'|' -f1 | sed 's/[[:space:]]*$//' | sort -u > "$T/base.txt"
    NB=$(wc -l < "$T/base.txt")

    # 3a 新增孤儿：在孤儿集、不在基线 ⇒ **判红**（债不许扩大）
    comm -23 "$T/orphan.txt" "$T/base.txt" > "$T/new.txt"
    NN=$(wc -l < "$T/new.txt")
    if [ "$NN" = 0 ]; then
        ok "无新增孤儿（$NO 个孤儿全部已登记）"
    else
        bad "**新增孤儿 $NN 个**（有 verify.sh 但没有任何运行器跑它，且不在基线）:"
        sed 's/^/       /' "$T/new.txt"
    fi

    # 3b 过期基线：在基线、不在孤儿集 ⇒ **判红**（登记必须仍然成立）
    comm -13 "$T/orphan.txt" "$T/base.txt" > "$T/stale.txt"
    NS=$(wc -l < "$T/stale.txt")
    if [ "$NS" = 0 ]; then
        ok "基线无过期条目（$NB 条 · 全部仍然成立）"
    else
        bad "基线**已过期** $NS 条（它们已被收录/已不存在 ⇒ 必须从 $BASE 移除）:"
        sed 's/^/       /' "$T/stale.txt"
    fi

    # 3c 每条必须带理由（≥8 字）
    NOREASON=0
    while IFS= read -r line; do
        case "$line" in ''|'#'*) continue ;; esac
        nm="$(printf '%s' "$line" | cut -d'|' -f1 | sed 's/[[:space:]]*$//')"
        rs="$(printf '%s' "$line" | cut -s -d'|' -f2-)"
        [ -n "$nm" ] || continue
        if [ -z "$rs" ] || [ "${#rs}" -lt 8 ]; then
            echo "       ⚠️ 缺理由/理由过短: $nm"
            NOREASON=$((NOREASON+1))
        fi
    done < <(grep -vE '^[[:space:]]*(#|$)' "$BASE")
    [ "$NOREASON" = 0 ] && ok "基线条目全部带理由（≥8 字）" || bad "$NOREASON 条基线条目缺理由"

    # 3d 基线只许缩小
    if [ "$NB" -le "$ORPHAN_MAX" ]; then
        ok "基线规模 $NB ≤ 上限 $ORPHAN_MAX（只许缩小）"
    else
        bad "基线规模 $NB > 上限 $ORPHAN_MAX —— 债在扩大（要扩充请显式改 ORPHAN_MAX 并说明）"
    fi
fi

echo
if [ "$FAIL" = 0 ]; then
    echo "══ ORPHAN-GATES-OK（通过 $PASS / 失败 0）"
    exit 0
else
    echo "══ ORPHAN-GATES-FAIL（通过 $PASS / 失败 $FAIL）"
    exit 1
fi
