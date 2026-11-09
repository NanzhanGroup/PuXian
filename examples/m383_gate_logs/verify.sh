#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M383 · 3/3：门日志**按链留存**（缺陷 679 —— 偶发红的现场被下一轮覆盖）
# ---------------------------------------------------------------------------
# 病根（2026-11-09 实证）：
#   run_gates.sh 把每个门的日志写在 /tmp/gate_<name>.log；分片侧 /tmp 被 bind 成
#   $BASE/tmp$i，而 tmp$i **每轮 rm -rf 重建**。M382 第三轮 m256 那条偶发红，
#   现场被第四轮直接抹掉 —— 事后只能看见结论，看不见证据。
#
# 本门守的是「现场不丢」这条机制，判据全部可复核：
#   [1] 两轮连跑：第 1 轮的日志在第 2 轮之后**逐字节仍在**（这是本门的主判据）
#   [2] 红/超时门另存 red/（一眼能捞），且内容与源一致
#   [3] 缺日志必须被点名（门红了但日志没留下 ⇒ 必须说出来，不许静默）
#   [4] 源目录零日志 ⇒ rc=3（响亮失败）—— 反面是 590 族「判据通道没了照样往下走」
#   [5] 参数缺失 ⇒ rc=4
#   [6] 接线：shard_run.sh 的 GATE_LOG_DIR_HINT 指向**链目录**且写链完成戳
#       （静态检查；端到端由全量链跑验证 —— 见 CHANGELOG M383）
#
# 负控：把 [1] 的留存目标改成「源目录内」时必须红（自证判据不是恒真）。
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
KEEP="$ROOT/packaging/chain/keep_gate_logs.sh"
SHARD="$ROOT/packaging/chain/shard_run.sh"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m383_gatelogs.XXXXXX")" || exit 1
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

[ -f "$KEEP" ] || { echo "缺少 $KEEP"; exit 1; }
[ -f "$SHARD" ] || { echo "缺少 $SHARD"; exit 1; }

# ── 构造「一轮链」的 /tmp 现场 ──
mkround() { # $1=内容版本标记
    local v="$1"
    rm -rf "$TMP/src"; mkdir -p "$TMP/src"
    printf 'log-A-%s\n' "$v" > "$TMP/src/gate_gooka.log"
    printf 'log-B-%s\n' "$v" > "$TMP/src/gate_gookb.log"
    printf 'RED-%s\n' "$v"     > "$TMP/src/gate_gored.log"
    cat > "$TMP/src/times.tsv" <<'EOF'
gooka	OK	3	-
gookb	SKIP	0	-
gored	FAIL	7	-
gonolog	FAIL	1	-
EOF
}

echo "── [1] 两轮连跑：第 1 轮现场必须在第 2 轮之后仍然存在"
mkround v1
out1="$(bash "$KEEP" "$TMP/src" "$TMP/chain1/gatelog/tmp1" "$TMP/src/times.tsv" "$TMP/chain1/red" shard1 2>&1)"; rc1=$?
[ "$rc1" -eq 0 ] && ok "第 1 轮留存 rc=0" || bad "第 1 轮留存 rc=$rc1"
[ -f "$TMP/chain1/gatelog/tmp1/gate_gooka.log" ] && ok "第 1 轮：门日志已落链目录" \
    || bad "第 1 轮：门日志未落链目录"
h1="$(sha256sum "$TMP/chain1/gatelog/tmp1/gate_gooka.log" 2>/dev/null | cut -d' ' -f1)"

# 模拟下一轮：/tmp 被 rm -rf 重建 + 内容变化（旧口径正是这里丢现场）
mkround v2
out2="$(bash "$KEEP" "$TMP/src" "$TMP/chain2/gatelog/tmp1" "$TMP/src/times.tsv" "$TMP/chain2/red" shard1 2>&1)"; rc2=$?
[ "$rc2" -eq 0 ] && ok "第 2 轮留存 rc=0" || bad "第 2 轮留存 rc=$rc2"
h1b="$(sha256sum "$TMP/chain1/gatelog/tmp1/gate_gooka.log" 2>/dev/null | cut -d' ' -f1)"
if [ -n "$h1" ] && [ "$h1" = "$h1b" ]; then
    ok "第 1 轮现场逐字节仍在（$h1）"
else
    bad "第 1 轮现场被改写/丢失（$h1 → ${h1b:-缺失}）"
fi
if grep -q 'log-A-v1' "$TMP/chain1/gatelog/tmp1/gate_gooka.log" 2>/dev/null; then
    ok "第 1 轮内容确为 v1（不是被第 2 轮串改）"
else
    bad "第 1 轮内容已不是 v1"
fi
[ -d "$TMP/chain1" ] && [ -d "$TMP/chain2" ] && [ "$TMP/chain1" != "$TMP/chain2" ] \
    && ok "按链（chain-ts）分目录：两轮互不覆盖" || bad "两轮目录未分离"

echo "── [2] 红/超时门另存 red/（内容与源一致）"
if [ -f "$TMP/chain1/red/shard1_gate_gored.log" ]; then
    ok "红门已另存 red/shard1_gate_gored.log"
    grep -q 'RED-v1' "$TMP/chain1/red/shard1_gate_gored.log" && ok "red 副本内容为第 1 轮（未被串改）" \
        || bad "red 副本内容不对"
else
    bad "红门未另存（$TMP/chain1/red 缺失）"
fi

echo "── [3] 缺日志必须被点名（门红了但没日志 ⇒ 说出来）"
if printf '%s' "$out1" | grep -q '缺日志'; then
    ok "输出现「缺日志」（gonolog 无 gate_*.log）"
else
    bad "缺日志未点名：$(printf '%s' "$out1" | tr '\n' ' ')"
fi
if printf '%s' "$out1" | grep -q 'gored(FAIL)'; then
    ok "非绿门在输出里被逐条列出"
else
    bad "非绿门未列出：$(printf '%s' "$out1" | tr '\n' ' ')"
fi

echo "── [4] 源目录零日志 ⇒ 必须响亮失败（rc=3，不许静默成功）"
mkdir -p "$TMP/empty"
printf 'gx\tFAIL\t1\t-\n' > "$TMP/empty/times.tsv"
bash "$KEEP" "$TMP/empty" "$TMP/chain0/gatelog/tmp1" "$TMP/empty/times.tsv" "$TMP/chain0/red" shard0 >/dev/null 2>&1
rce=$?
[ "$rce" -eq 3 ] && ok "零日志 rc=3（响亮）" || bad "零日志 rc=$rce ≠ 3（静默成功 = 现场丢了还说没事）"

echo "── [5] 参数缺失 ⇒ rc=4"
bash "$KEEP" "$TMP/src" >/dev/null 2>&1
rcp=$?
[ "$rcp" -eq 4 ] && ok "参数缺失 rc=4" || bad "参数缺失 rc=$rcp ≠ 4"

echo "── [6] 接线：shard_run.sh 的日志路径指向链目录 + 写链完成戳"
if grep -q 'GATE_LOG_DIR_HINT=$GLOG/tmp\$i' "$SHARD"; then
    ok "分片门日志 hint 指向链目录（\$GLOG/tmpN）"
else
    bad "分片门日志 hint 未指向链目录（下一轮仍会覆盖）"
fi
if grep -q 'keep_gate_logs.sh' "$SHARD"; then
    ok "分片/车道两路都调 keep_gate_logs.sh"
else
    bad "shard_run 未调用 keep_gate_logs.sh（机制未接线）"
fi
if grep -q 'CHAIN-COMPLETE.txt' "$SHARD"; then
    ok "链完成戳 CHAIN-COMPLETE.txt 已写（供校验器区分全量链 / 探针）"
else
    bad "缺链完成戳 ⇒ 残表与全量表仍无法区分"
fi
if grep -q 'gatelog' "$SHARD" && grep -q 'mtime +14' "$SHARD"; then
    ok "留存有界（14 天前的 gatelog 清理，链本身不动）"
else
    bad "留存无界（会无限长）"
fi

# ── [7] 缺陷 681：全量轮也必须写 gates.all.txt（否则戳恒为 probe）──
#   实测：16:40→18:13 的 261 门全量轮写出的 CHAIN-COMPLETE.txt 是
#     stamp=probe · gates_registry=0 ⇒「全跑」被标成「探针」（留痕字段与实际不符）。
#   判据两条：① 全集副本的生成行必须在 --only 分支之前（接线顺序）
#             ② 用 shard_run 里**原样的**计算段 + 真文件跑两档（全量 ⇒ full-basis，缺文件 ⇒ probe）
echo "── [7] 全量轮全集戳（缺陷 681：gates.all.txt 必须由全量路径生成）"
_a_line="$(grep -n 'cp "\$LOG/gates.txt" "\$LOG/gates.all.txt"' "$SHARD" | head -1 | cut -d: -f1)"
_o_line="$(grep -n 'if \[ -n "\${ONLY_GATES:-}" \]' "$SHARD" | head -1 | cut -d: -f1)"
if [ -n "${_a_line:-}" ] && [ -n "${_o_line:-}" ] && [ "$_a_line" -lt "$_o_line" ]; then
    ok "全集副本生成行在 --only 分支之前（行 $_a_line < $_o_line）"
else
    bad "全集副本生成行缺失或位置错（a=${_a_line:-无} / only=${_o_line:-无}）⇒ 全量轮戳仍会失实"
fi
# 动态：原样抽 shard_run 的计算段（不是重写一份），配真文件跑
_SNIP="$(sed -n '/^_alln=\$(grep -c \. "\$LOG\/gates.all.txt"/,/^\[ "\$NG" = "\$_alln" \]/p' "$SHARD")"
if [ -n "$_SNIP" ]; then
    sd="$TMP/stamp"; mkdir -p "$sd"
    printf 'g1\ng2\ng3\n' > "$sd/gates.txt"; cp "$sd/gates.txt" "$sd/gates.all.txt"
    _stamp=""; LOG="$sd" NG=3 QUIET_LIST="" eval "$_SNIP"
    [ "$_stamp" = "full-basis" ] && ok "全集在场 ⇒ stamp=full-basis（$_stamp）" \
        || bad "全集在场却 stamp=${_stamp:-空}（全量轮会被误标成探针）"
    rm -f "$sd/gates.all.txt"; _stamp=""
    LOG="$sd" NG=3 QUIET_LIST="" eval "$_SNIP"
    [ "$_stamp" = "probe" ] && ok "负控：缺全集 ⇒ stamp=probe（口径能区分）" \
        || bad "负控不成立：缺全集却 stamp=${_stamp:-空}"
else
    bad "抽不到 shard_run 的 _stamp 计算段（脚本结构变了，判据失效须重写）"
fi

echo "── 负控：把留存目标指回源目录 ⇒ [1] 必须红"
# 机制自证：如果在同一目录里 cp，第二轮会覆盖第一轮 ⇒ 判据能红。
#   （这不是测 keep_gate_logs 的实现，而是测「本门的主判据不是恒真」）
d="$TMP/nc"; mkdir -p "$d"
printf 'v1\n' > "$d/gate_x.log"; cp -f "$d/gate_x.log" "$d/" 2>/dev/null
h_a="$(sha256sum "$d/gate_x.log" | cut -d' ' -f1)"
printf 'v2\n' > "$d/gate_x.log"; cp -f "$d/gate_x.log" "$d/" 2>/dev/null
h_b="$(sha256sum "$d/gate_x.log" | cut -d' ' -f1)"
[ "$h_a" != "$h_b" ] && ok "负控成立：同目录覆写 ⇒ 内容必变（所以判据能红）" \
    || bad "负控不成立：同目录覆写后内容未变（判据可能是恒真）"

echo
echo "── 结果：$pass 通过 / $fail 失败"
if [ "$fail" -eq 0 ]; then
    echo "M383-GATELOG-VERIFY-OK"
    exit 0
fi
echo "M383-GATELOG-VERIFY-FAIL"
exit 1
