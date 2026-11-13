#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M400 · 缺陷 763 收口：门耗时基线的**输入回路**（判据自证门）
# ---------------------------------------------------------------------------
# 缺陷 763 的现场（M400 实测，不是推测）：
#   · selfhost/gate_time_baseline.tsv 上次生成 = 10-26 14:30（18 天前），233 行
#   · 门清单 274 门 ⇒ 41 门 check_gate_timeouts 的 P2 **从来判不到**（静默绿）
#   · 生成器口径写得完备（只收非 SKIP / 同门取最大 / 带来源标签），但
#     grep 全仓：**没有任何自动化调用它**，只有 docs 写着「手工跑」
#   · 每轮全量门都产出 shardN.times.tsv / quiet*.times.tsv，**从未被消费**
#   · packaging/release_preflight.sh 的计时表跑完就 rm —— 而历史上最高占用恰恰
#     只在那张被删的表里（m187 提交前门 747~755s/900s = 83~84%）
#   ⇒ 「判据的输入必须可复算」这条纪律，卡在「谁去喂」上。
#
# 本轮四条口径（每条都在下面有判据）：
#   T1 selfhost/refresh_gate_baseline.sh 自证：窗口下界 / rc=124 丢弃 / retain 只补缺 / 幂等 / 缺口 rc=1
#   T2 check_gate_timeouts.sh 自证：P6 覆盖 / P7 新鲜度 各有负控 + 正控
#   T3 真基线：形态合法 + **行数 == 门清单数**（覆盖锚点，本轮的核心数字）
#   T4 rc=124 端到端：某门只有「被砍断」的一行 ⇒ 基线保留旧值，绝不写 899
#   负控：N1 删门 ⇒ P6 必红；N2 造假「已完成」stamp ⇒ P7 必红；两者都有正控
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
TMP="${TMPDIR:-/tmp}/m400_baseline.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { pass=$((pass+1)); echo "  ✅ $*"; }
bad() { fail=$((fail+1)); echo "  ❌ $*"; }

BASE="$ROOT/selfhost/gate_time_baseline.tsv"
GEN="$ROOT/selfhost/gate_time_baseline.sh"
REF="$ROOT/selfhost/refresh_gate_baseline.sh"
CHK="$ROOT/selfhost/check_gate_timeouts.sh"
./selfhost/run_gates.sh --list 2>/dev/null | grep -v '^[[:space:]]*$' | sort -u > "$TMP/gates.txt"
NG="$(grep -c . "$TMP/gates.txt" || true)"
NB="$(grep -vc '^#' "$BASE" || true)"

# ── T1 回灌器自证 ──────────────────────────────────────────────
if "$REF" --self-test > "$TMP/t1.log" 2>&1; then
    ok "T1 回灌器自证：$(grep -o 'refresh 自测：[0-9]*/[0-9]*' "$TMP/t1.log" | awk 'END{print}')"
else
    bad "T1 回灌器自证失败（见下）"; grep '❌' "$TMP/t1.log" | head -5 | sed 's/^/      /'
fi

# ── T2 超时守卫自证（P6/P7 的牙在自证里）──────────────────────
if "$CHK" --self-test > "$TMP/t2.log" 2>&1; then
    ok "T2 超时守卫自证：$(grep -o '自证：[0-9]* 通过 / [0-9]* 失败' "$TMP/t2.log" | awk 'END{print}')"
else
    bad "T2 超时守卫自证失败"; grep '❌' "$TMP/t2.log" | head -5 | sed 's/^/      /'
fi

# ── T3 真基线：形态 + 覆盖锚点 ────────────────────────────────
if "$GEN" --check > "$TMP/t3.log" 2>&1; then
    ok "T3a 基线形态合法（$(grep -o '[0-9]* 行' "$TMP/t3.log" | head -1)）"
else
    bad "T3a 基线形态非法"; grep '❌' "$TMP/t3.log" | head -3 | sed 's/^/      /'
fi
if [ "$NG" = "$NB" ] && [ "$NG" -ge 250 ]; then
    ok "T3b 覆盖锚点：门清单 $NG 门 == 基线 $NB 行（且 ≥250，防提取器塌缩）"
else
    bad "T3b 覆盖不全：门清单 $NG / 基线 $NB（M400 前是 274/233 —— 41 门 P2 判不到）"
fi
# 前提自证：基线里每个门名都真在门清单里（反向包含，防「凑数补行」）
comm -13 "$TMP/gates.txt" <(awk -F'\t' '!/^#/{print $1}' "$BASE" | sort -u) > "$TMP/extra.txt"
[ ! -s "$TMP/extra.txt" ] && ok "T3c 前提自证：基线没有门清单以外的行（不是凑数）" \
                          || bad "T3c 基线含非门清单行：$(head -3 "$TMP/extra.txt" | tr '\n' ' ')"

# ── 负控用的夹具基底：真基线 + 「门清单里有、基线里还没行」的占位行 ─────────
#   为什么要这一步：本门自己也新增了门 —— **首次入册时它自己的基线行还不存在**。
#   若负控直接拿真基线当「期望绿」的对照，就会因为这个缺口先把绿判成红（首跑实测：
#   N1b/N2b/N2c/N3 四条全假红）。T3b 该判的是**真**覆盖，负控该判的是**判据本身**，两者分开。
cp "$BASE" "$TMP/full.tsv"
comm -23 "$TMP/gates.txt" <(awk -F'\t' '!/^#/{print $1}' "$BASE" | sort -u) > "$TMP/norow.txt"
while IFS= read -r g; do
    [ -n "$g" ] || continue
    printf '%s\t1\tOK\tsynthetic\n' "$g" >> "$TMP/full.tsv"
done < "$TMP/norow.txt"
[ -s "$TMP/norow.txt" ] && echo "  ℹ️  基线暂缺 $(grep -c . "$TMP/norow.txt") 门口（首次入册/待回灌）：$(head -3 "$TMP/norow.txt" | tr '\n' ' ')"

# ── T4 rc=124 端到端：砍断值不许进基线 ───────────────────────
#   夹具：旧基线 gX=100（retain 兜底），输入只有一行 gX FAIL 899 rc=124
printf '# old\ngX\t100\tOK\tseed\n' > "$TMP/old.tsv"
printf 'gX\tFAIL\t899\t124\n' > "$TMP/in.tsv"
if "$GEN" --from "$TMP/in.tsv" --label to124 --retain "$TMP/old.tsv" --out "$TMP/o1.tsv" > "$TMP/t4.log" 2>&1; then
    v="$(awk -F'\t' '$1=="gX"{print $2}' "$TMP/o1.tsv")"
    [ "$v" = 100 ] && ok "T4 rc=124 被丢：gX=100（retain 旧值，不是砍断值 899）" \
                   || bad "T4 gX=$v（应 100；899 说明砍断值被当成耗时）"
    # 负控：把丢弃关掉 ⇒ 必须变成 899（证明 T4 的红/绿来自这条排除，而不是碰巧）
    "$GEN" --from "$TMP/in.tsv" --label to124 --keep-timeout-rows --out "$TMP/o2.tsv" > /dev/null 2>&1
    v2="$(awk -F'\t' '$1=="gX"{print $2}' "$TMP/o2.tsv")"
    [ "$v2" = 899 ] && ok "T4b 负控：--keep-timeout-rows ⇒ gX=899（排除确实是起作用的那一步）" \
                    || bad "T4b 负控 gX=$v2（应 899）"
else
    bad "T4 生成器 rc≠0"; sed -n '1,3p' "$TMP/t4.log" | sed 's/^/      /'
fi

# ── T5 幂等：同输入两次生成，**数据行**逐字节相同 ──────────────
#   文件头含「生成时间」是设计（可追溯），所以只对数据行比对
#   ⚠️ 先证「两个文件都真生成了」—— 否则 cmp 两个缺失文件会**假绿**（首跑实测踩到）
"$GEN" --from "$TMP/in.tsv" --label idem --retain "$TMP/old.tsv" --out "$TMP/i1.tsv" > /dev/null 2>&1
sleep 1
"$GEN" --from "$TMP/in.tsv" --label idem --retain "$TMP/old.tsv" --out "$TMP/i2.tsv" > /dev/null 2>&1
if [ ! -s "$TMP/i1.tsv" ] || [ ! -s "$TMP/i2.tsv" ]; then
    bad "T5 前提缺失：生成物不存在（i1=$( [ -s "$TMP/i1.tsv" ] && echo 有 || echo 无) i2=$( [ -s "$TMP/i2.tsv" ] && echo 有 || echo 无)）"
elif cmp -s <(grep -v '^#' "$TMP/i1.tsv") <(grep -v '^#' "$TMP/i2.tsv"); then
    ok "T5 幂等：数据行逐字节相同（跨秒仍一致）"
else
    bad "T5 非幂等"
fi

# ── N1 P6 负控：删一门 ⇒ 必红且指名；正控：原样 ⇒ 绿 ─────────
awk -F'\t' '!/^#/ && $1!="check_defect_refs"{print}' "$TMP/full.tsv" > "$TMP/gap.tsv"
"$CHK" --baseline "$TMP/gap.tsv" --list-file "$TMP/gates.txt" --no-fresh > "$TMP/n1.log" 2>&1
rc=$?
if [ "$rc" = 1 ] && grep -qE '基线缺 [0-9]+ /' "$TMP/n1.log" && grep -q '· check_defect_refs' "$TMP/n1.log"; then
    ok "N1 负控：删掉 check_defect_refs ⇒ P6 判红且指名"
else
    bad "N1 负控失败 rc=$rc：$(grep '❌' "$TMP/n1.log" | head -2 | tr '\n' ' ')"
fi
# 前提自证：扰动真发生了（少了一行）
[ "$(grep -vc '^#' "$TMP/gap.tsv")" = "$(( $(grep -vc '^#' "$TMP/full.tsv") - 1 ))" ] && ok "N1 前提自证：夹具确实少了一行" \
                                                    || bad "N1 前提自证失败（夹具没少行，判据无效）"
"$CHK" --baseline "$TMP/full.tsv" --list-file "$TMP/gates.txt" --no-fresh > "$TMP/n1b.log" 2>&1
[ "$?" = 0 ] && ok "N1b 正控：完整基线 ⇒ P6 绿（红确实来自缺失）" || bad "N1b 正控失败：$(grep '❌' "$TMP/n1b.log" | head -1)"

# ── N2 P7 负控：造一个更新的「已完成」链 ⇒ 必红；正控：拿掉 ⇒ 绿 ─
mkdir -p "$TMP/md/shard-20991231-000000"
printf 'chain_ts=20991231-000000\nok=1\n' > "$TMP/md/shard-20991231-000000/CHAIN-COMPLETE.txt"
printf 'gX\tOK\t5\t-\n' > "$TMP/md/shard-20991231-000000/shard1.times.tsv"
"$CHK" --baseline "$TMP/full.tsv" --list-file "$TMP/gates.txt" --measured-dir "$TMP/md" > "$TMP/n2.log" 2>&1
rc=$?
if [ "$rc" = 1 ] && grep -q '未回灌' "$TMP/n2.log" && grep -q '20991231-000000' "$TMP/n2.log"; then
    ok "N2 负控：有更新的已完成链未回灌 ⇒ P7 判红且指名链号"
else
    bad "N2 负控失败 rc=$rc：$(grep '❌' "$TMP/n2.log" | head -2 | tr '\n' ' ')"
fi
# 负控的负控：只有目录、**没有** CHAIN-COMPLETE.txt（= 正在跑的链）⇒ 不许判红
rm -f "$TMP/md/shard-20991231-000000/CHAIN-COMPLETE.txt"
"$CHK" --baseline "$TMP/full.tsv" --list-file "$TMP/gates.txt" --measured-dir "$TMP/md" > "$TMP/n2b.log" 2>&1
[ "$?" = 0 ] && ok "N2b 未完成的链不计入（否则判据会读自己）" || bad "N2b 未完成链被计入 ⇒ P7 会自激"
rm -rf "$TMP/md"
"$CHK" --baseline "$TMP/full.tsv" --list-file "$TMP/gates.txt" --measured-dir "$TMP/md" > "$TMP/n2c.log" 2>&1
[ "$?" = 0 ] && ok "N2c 正控：目录不存在 ⇒ P7 只 ℹ️ 不判红（异机/CI 可用）" || bad "N2c 目录缺失时判红"

# ── N3 P7 与 P6 互不顶替：只关 P7，P6 仍要能红 ────────────────
"$CHK" --baseline "$TMP/gap.tsv" --list-file "$TMP/gates.txt" --no-fresh > "$TMP/n3.log" 2>&1
[ "$?" = 1 ] && grep -qE '基线缺 [0-9]+ /' "$TMP/n3.log" && ok "N3 判据隔离：--no-fresh 不影响 P6" \
                                                    || bad "N3 关掉 P7 后 P6 也没红（判据互相顶替）"

echo
echo "════════════════════════════════════"
echo "m400_baseline_feedback: pass=$pass fail=$fail（门清单 $NG / 基线 $NB）"
[ "$fail" -eq 0 ] || exit 1
exit 0
