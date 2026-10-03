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

    # ── 3e 体检台账（M251 · 缺陷 428 的「下半条」）─────────────────────
    # M249 挡住了「新增孤儿」，但基线理由统一是「存量」——
    #   「这扇门现在还是绿的吗？它依赖的东西还在吗？」**无人回答**。
    #   m63 已证明「孤儿门 = 判据腐烂的温床」⇒ 体检必须留下**可核对**的记录。
    AUDIT="${ORPHAN_AUDIT:-selfhost/orphan_audit.tsv}"
    if [ ! -f "$AUDIT" ]; then
        bad "体检台账不存在：$AUDIT（每个孤儿门都要有一条实测记录）"
    else
        awk -F'\t' '!/^#/ && NF>=5 {print $1}' "$AUDIT" | sed 's/[[:space:]]*$//' | sort -u > "$T/audit.txt"
        NX=$(wc -l < "$T/audit.txt")
        echo "  体检台账 $NX 条"
        comm -23 "$T/base.txt" "$T/audit.txt" > "$T/a_miss.txt"
        comm -13 "$T/base.txt" "$T/audit.txt" > "$T/a_stale.txt"
        NM=$(wc -l < "$T/a_miss.txt"); NX2=$(wc -l < "$T/a_stale.txt")
        [ "$NM" = 0 ] && ok "台账无漏记（$NB 个孤儿门全部有实测记录）" \
                      || { bad "台账**漏记 $NM 个**（孤儿门没有实测记录）:"; sed 's/^/       /' "$T/a_miss.txt"; }
        [ "$NX2" = 0 ] && ok "台账无过期条目" \
                       || { bad "台账**过期 $NX2 条**（不在基线里 ⇒ 删它）:"; sed 's/^/       /' "$T/a_stale.txt"; }

        # 3e-2 分类合法 + 非 OK 条目必须带定性处置
        BADCLS=0; BADNOTE=0
        : > "$T/a_bad.txt"
        while IFS=$'\t' read -r g deps rc sec cls note; do
            case "$g" in ''|'#'*) continue ;; esac
            case "$cls" in
                OK|SKIP|FAIL|TIMEOUT) ;;
                *) echo "       ⚠️ 非法分类 '$cls'（门 $g）"; BADCLS=$((BADCLS+1)) ;;
            esac
            case "$cls" in
                SKIP|FAIL|TIMEOUT)
                    case "$note" in
                        *已修*|*已登记*|*环境*|*设计性*) ;;
                        *) echo "       ⚠️ $g 分类 $cls 但无处置（note 需含 已修/已登记/环境/设计性）"; BADNOTE=$((BADNOTE+1)) ;;
                    esac ;;
            esac
        done < "$AUDIT"
        [ "$BADCLS" = 0 ] && ok "台账分类字段全部合法" || bad "$BADCLS 条分类非法"
        [ "$BADNOTE" = 0 ] && ok "非 OK 条目全部带定性处置" || bad "$BADNOTE 条缺处置文本"

        # 3e-3 规模锚点（防「空台账 ⊇ 空基线」的假绿）
        [ "$NX" -ge 40 ] && ok "台账规模下限（≥40，实测 $NX）" || bad "台账只 $NX 条 —— 体检没跑？"
    fi

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
