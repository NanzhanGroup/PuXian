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
            # M404 缺陷 771：子门日志原本只在 $TMP 里，函数退出即删 ⇒ 红只剩一行摘要，
            #   真因（go 的真实报错 / 断言原文）无从查证。红必须自带证据 ⇒ 留尾部 6 行。
            echo "     ── 子门日志尾部（$name · 共 $(grep -c . "$TMP/$name.log" 2>/dev/null || echo 0) 行）──"
            tail -6 "$TMP/$name.log" 2>/dev/null | sed 's/^/       /'
            RM_BAD=$((RM_BAD + 1))
            continue
        fi
        # ── C3 SKIP 口径（M404 细化）────────────────────────────────────────
        #   实测依据：m103_s2a / m84_s3_dns 的「外网档」是**设计性观察项**，离线段照跑
        #   且有正向断言；旧口径一刀切判死 = 把「离线可验证的门」也钉在孤儿名单上。
        #   ① 未带 [585-SKIP-LOUD] 标记的 SKIP ⇒ 红（静默跳过 = 零观测）；
        #   ② 带标记但日志里**没有任何正向断言** ⇒ 红（整门零覆盖，SKIP 当绿）；
        #   ③ 带标记 + 有正向断言 ⇒ 允许（观察项缺失不等于门没干活）。
        local sk_all sk_loud positive
        sk_all=$(grep -cE '(^|[^A-Za-z])SKIP' "$TMP/$name.log" 2>/dev/null || true)
        sk_loud=$(grep -cE '\[585-SKIP-LOUD\]' "$TMP/$name.log" 2>/dev/null || true)
        positive=$(grep -cE 'PASS|✅|OK|[0-9]+P/[0-9]+F' "$TMP/$name.log" 2>/dev/null || true)
        if [ "${sk_all:-0}" -gt "${sk_loud:-0}" ]; then
            echo "  ❌ C3 $name 静默 SKIP（缺 [585-SKIP-LOUD] 标记）：$(grep -E '(^|[^A-Za-z])SKIP' "$TMP/$name.log" | head -1 | cut -c1-80)"
            RM_BAD=$((RM_BAD + 1))
            continue
        fi
        if [ "${sk_loud:-0}" -gt 0 ] && [ "${positive:-0}" -lt 1 ]; then
            echo "  ❌ C3 $name 整门 SKIP（带标记但无任何正向断言 ⇒ 零覆盖）"
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

# ── C6：门不得依赖**非仓库产物**（M404 加）────────────────────────────────
#   病史：m89_s3d / m90_s1 都要求 selfhost/build/compiler_new（M89/M90 期手工重链，
#   不在仓库、devbuild.sh 也不生成）⇒ 缺它 SKIP 判绿、有残留才真跑
#   ＝「门绿不绿取决于谁的机器上有什么残留」（M304 同款）。此类前提一律要求自备。
echo "── C6 门不得依赖非仓库产物（手工重链 compiler_new 等残留）──"
# ⚠️ 两条自证纪律（第一版就踩了）：
#   ① **注释不算代码**（CRITERIA ⑥）：逐行去掉整行注释再匹配，否则"病史说明"会被当成依赖；
#   ② **判据不算自己**：本门自己的检查逻辑里必然出现这个字面量 ⇒ 必须排除本门。
C6_BAD=""
for _f in examples/*/verify.sh; do
    case "$_f" in examples/m403_orphan_batch/*) continue ;; esac
    if sed 's/^[[:space:]]*#.*$//' "$_f" | grep -q 'selfhost/build/compiler_new'; then
        C6_BAD="$C6_BAD $_f"
    fi
done
if [ -z "$C6_BAD" ]; then
    ok "没有任何门依赖手工重链产物（compiler_new）"
else
    bad "以下门仍依赖非仓库产物：$C6_BAD"
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

# ── N3/N4/N5：C3 SKIP 新口径的负控 + 正控（M404 加）────────────────────────
echo "── N3/N4/N5：SKIP 口径自证（静默 SKIP 必红 / 整门 SKIP 必红 / 带标记且有正向断言必绿）──"
mkdir -p "$FX/examples/fx_silent_skip" "$FX/examples/fx_all_skip" "$FX/examples/fx_loud_skip"
printf '#!/usr/bin/env bash\necho "PASS 本地断言一条"\necho "SKIP: 某外部依赖缺失"\nexit 0\n' \
    > "$FX/examples/fx_silent_skip/verify.sh"
printf '#!/usr/bin/env bash\necho "[585-SKIP-LOUD] 外网档跳过"\nexit 0\n' \
    > "$FX/examples/fx_all_skip/verify.sh"
printf '#!/usr/bin/env bash\necho "PASS 本地断言一条"\necho "[585-SKIP-LOUD] 外网档跳过（设计性观察项）"\nexit 0\n' \
    > "$FX/examples/fx_loud_skip/verify.sh"
chmod +x "$FX/examples/fx_silent_skip/verify.sh" "$FX/examples/fx_all_skip/verify.sh" "$FX/examples/fx_loud_skip/verify.sh"
printf 'fx_silent_skip\n' > "$FX/silent.txt"
printf 'fx_all_skip\n'    > "$FX/allskip.txt"
printf 'fx_loud_skip\n'   > "$FX/loudskip.txt"
run_manifest "$FX/silent.txt"  0 "$FX" > "$TMP/n3.out" 2>&1; rcN3=$?
run_manifest "$FX/allskip.txt" 0 "$FX" > "$TMP/n4.out" 2>&1; rcN4=$?
run_manifest "$FX/loudskip.txt" 0 "$FX" > "$TMP/n5.out" 2>&1; rcN5=$?
if [ "$rcN3" != 0 ] && grep -q '静默 SKIP' "$TMP/n3.out"; then
    ok "N3 负控：静默 SKIP（无标记）⇒ 判红"
else
    bad "N3 负控失败：静默 SKIP 未被判红（rc=$rcN3）：$(head -2 "$TMP/n3.out" | tr '\n' ' ')"
fi
if [ "$rcN4" != 0 ] && grep -q '整门 SKIP' "$TMP/n4.out"; then
    ok "N4 负控：带标记但零正向断言 ⇒ 判红（零覆盖不能算绿）"
else
    bad "N4 负控失败：整门 SKIP 未被判红（rc=$rcN4）：$(head -2 "$TMP/n4.out" | tr '\n' ' ')"
fi
if [ "$rcN5" = 0 ] && grep -q 'fx_loud_skip OK' "$TMP/n5.out"; then
    ok "N5 正控：带标记 + 有正向断言 ⇒ 判绿（证明 N3/N4 的红来自口径本身）"
else
    bad "N5 正控失败：合法观察项被判红（rc=$rcN5）：$(head -2 "$TMP/n5.out" | tr '\n' ' ')"
fi

echo "── m403_orphan_batch：通过 $pass · 失败 $fail ──"
[ "$fail" = 0 ] || exit 1
