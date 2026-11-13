#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M403 · 孤儿门批量复活：把「写好了、但没有任何运行器跑它」的门接进全量门
# ---------------------------------------------------------------------------
# 病史（为什么需要这一门）：
#   M249 建立孤儿守卫时实测 179 个 verify.sh ⇄ registry 124 / ci.yml 127 ⇒ 55 扇门没人跑；
#   M251 逐条体检过一遍（46 OK / 3 SKIP / 1 FAIL），之后**再没有跑过**。
#   M401 的收获正是这条：语言缺陷（默认参数前向引用）本该由 m90_s1 挡下，而那扇门就在孤儿名单里
#   —— 「判据存在 ≠ 判据在执行」。
#
# 本轮处置（2026-11-14 05:30–05:44 一次性实测 46 扇）：
#   · 39 扇今天实测 rc=0 且**没有 SKIP** ⇒ 进 selfhost/orphan_revived.txt，由本门逐条串行跑
#   · 7 扇今天实测异常 / SKIP ⇒ 留在 selfhost/orphan_gates.txt，理由里写明实测值与处置（排 M404）
#   · m85_s1 首测 120s 超时、放宽到 420s 后 10s 通过 ⇒ 冷缓存，归入复活
#   · m66_proc rc=0 但 [6/6] 是 SKIP（缺 pyzipper）⇒ **不复活**：SKIP 当绿正是本轮要治的病
#
# 判据：
#   C1 清单每一条都必须有 examples/<名>/verify.sh（缺一件就红，不许静默跳过）
#   C2 每条必须 rc=0
#   C3 每条日志里**不许出现独立 SKIP 字样**（SKIP 不算绿 —— m66_proc 就是被这条挡住的）
#   C4 清单 ⇄ selfhost/orphan_gates.txt 不相交（同一扇门不能既「已收录」又「孤儿」）
#   C5 清单里的门不得已被 registry 收录（否则同一门跑两遍，且这里的 verdict 会掩盖真门的红）
#   N1 负控：临时夹具（一条必过 + 一条必败）⇒ 必须判红，且**指名**失败的那条
#   N2 正控：同夹具只留必过那条 ⇒ 必须判绿（证明 N1 的红确实来自「失败」而不是夹具本身）
# 环境缝（只给自证用）：M403_MANIFEST / M403_ROOT
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
MANIFEST="${M403_MANIFEST:-$ROOT/selfhost/orphan_revived.txt}"
SCAN_ROOT="${M403_ROOT:-$ROOT}"
TMP="${TMPDIR:-/tmp}/m403_batch.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# ── 跑一批清单（返回 0 = 全绿；RM_N = 通过条数，RM_BAD = 红条数）──────────────
#   ⚠️ 计数用 RM_* 而不是全局 pass/fail：自证夹具（N1/N2）要用同一函数跑**故意失败**的清单，
#      若共用全局计数，夹具的红会被算进门自己的判决里（M401 踩过的形状：夹具污染主判据）。
run_manifest() {
    local manifest="$1" quiet="${2:-1}" root="${3:-$ROOT}" n=0 f rc t0 el name
    RM_N=0; RM_BAD=0
    while IFS= read -r name; do
        case "$name" in ''|'#'*) continue ;; esac
        name="$(printf '%s' "$name" | tr -d '[:space:]')"
        [ -n "$name" ] || continue
        n=$((n + 1))
        f="$root/examples/$name/verify.sh"
        if [ ! -f "$f" ]; then
            echo "  ❌ C1 清单条目无 verify.sh：$name"
            RM_BAD=$((RM_BAD + 1))
            continue
        fi
        t0=$(date +%s)
        timeout -k 10 300 bash "$f" > "$TMP/$name.log" 2>&1
        rc=$?
        el=$(( $(date +%s) - t0 ))
        if [ "$rc" != 0 ]; then
            echo "  ❌ C2 $name 判红（rc=$rc · ${el}s · $(grep -iE 'FAIL|❌|错误' "$TMP/$name.log" | head -1 | cut -c1-90)）"
            RM_BAD=$((RM_BAD + 1))
            continue
        fi
        if grep -qE '(^|[^A-Za-z])SKIP([^A-Za-z]|$)' "$TMP/$name.log"; then
            echo "  ❌ C3 $name 打印了 SKIP（SKIP 不算绿）：$(grep -E '(^|[^A-Za-z])SKIP' "$TMP/$name.log" | head -1 | cut -c1-80)"
            RM_BAD=$((RM_BAD + 1))
            continue
        fi
        RM_N=$((RM_N + 1))
        if [ "$quiet" = 0 ]; then echo "       · $name OK ${el}s"; fi
    done < "$manifest"
    if [ "$n" = 0 ]; then echo "      · 清单为空（$manifest）"; RM_BAD=$((RM_BAD + 1)); fi
    [ "$RM_BAD" = 0 ]
}

echo "── C1/C2/C3 逐条实跑（清单 $(grep -vc '^#' "$MANIFEST" 2>/dev/null || echo 0) 条）──"
if run_manifest "$MANIFEST" 0 "$ROOT"; then
    ok "清单内全部门 rc=0 且无 SKIP（$RM_N 条）"
else
    bad "清单内有 $RM_BAD 条红（逐条见上）"
fi

# ── C4：清单 ⇄ 孤儿基线不相交 ──────────────────────────────────────────────
echo "── C4 清单与孤儿基线不相交 ──"
if [ -f "$ROOT/selfhost/orphan_gates.txt" ] && [ -f "$MANIFEST" ]; then
    inter="$(comm -12 <(grep -v '^#' "$MANIFEST" | tr -d '[:space:]' | grep . | sort -u) \
                      <(grep -v '^#' "$ROOT/selfhost/orphan_gates.txt" | cut -d'|' -f1 | tr -d '[:space:]' | grep . | sort -u) | head -5)"
    if [ -z "$inter" ]; then
        ok "无交集（同一扇门不会既已收录又孤儿）"
    else
        bad "清单与孤儿基线有交集：$(echo "$inter" | tr '\n' ' ')"
    fi
else
    bad "C4 缺清单或孤儿基线文件（无法判）"
fi

# ── C5：清单里的门不得已被 registry 收录 ──────────────────────────────────
echo "── C5 清单里的门不得已被 registry 收录（防同一门跑两遍）──"
if [ -f "$ROOT/selfhost/gates.registry.sh" ] && [ -f "$MANIFEST" ]; then
    dup=""
    while IFS= read -r name; do
        case "$name" in ''|'#'*) continue ;; esac
        name="$(printf '%s' "$name" | tr -d '[:space:]')"
        [ -n "$name" ] || continue
        if grep -qE "^run ${name} " "$ROOT/selfhost/gates.registry.sh"; then dup="$dup $name"; fi
    done < "$MANIFEST"
    if [ -z "$dup" ]; then
        ok "清单里没有任何门已被 registry 单独收录"
    else
        bad "以下门同时被 registry 收录（应从这里移出）：$dup"
    fi
else
    bad "C5 缺清单或 registry（无法判）"
fi

# ── N1/N2 判据自证（临时夹具：一条必过 + 一条必败）──────────────────────────
echo "── N1/N2 判据自证（临时夹具）──"
FX="$TMP/fx"; mkdir -p "$FX/examples/fx_ok" "$FX/examples/fx_bad"
printf '#!/usr/bin/env bash\necho "FX-OK"\nexit 0\n'  > "$FX/examples/fx_ok/verify.sh"
printf '#!/usr/bin/env bash\necho "FX-BAD-原因"\nexit 3\n' > "$FX/examples/fx_bad/verify.sh"
chmod +x "$FX/examples/fx_ok/verify.sh" "$FX/examples/fx_bad/verify.sh"
printf 'fx_ok\nfx_bad\n' > "$FX/both.txt"
printf 'fx_ok\n'       > "$FX/only_ok.txt"

#   输出落文件而不是 $(...) 捕获：函数体内跑子进程 + 读清单，走命令替换时
#   偶发拿不到函数输出（实测过），落文件最稳，也便于失败时把原文打出来。
run_manifest "$FX/both.txt"    0 "$FX" > "$TMP/n1.out" 2>&1; rcN1=$?
run_manifest "$FX/only_ok.txt" 0 "$FX" > "$TMP/n2.out" 2>&1; rcN2=$?
if [ "$rcN1" != 0 ] && grep -q 'fx_bad' "$TMP/n1.out"; then
    ok "N1 负控：必败夹具被判红且指名 fx_bad"
else
    bad "N1 负控失败：必败夹具没被判红/没指名（rc=$rcN1）：$(head -2 "$TMP/n1.out" | tr '\n' ' ')"
fi
if [ "$rcN2" = 0 ] && grep -q 'fx_ok OK' "$TMP/n2.out"; then
    ok "N2 正控：只留必过夹具 ⇒ 判绿（证明 N1 的红来自失败本身）"
else
    bad "N2 正控失败：必过夹具被判红（rc=$rcN2）：$(head -2 "$TMP/n2.out" | tr '\n' ' ')"
fi

echo "── m403_orphan_batch：通过 $pass · 失败 $fail ──"
[ "$fail" = 0 ] || exit 1
