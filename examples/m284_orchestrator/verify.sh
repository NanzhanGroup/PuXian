#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M284 门：编排骨架入仓 + 判据串在场守卫（缺陷 490 / 495）
#
# 本门护住两件事：
#   ① `packaging/chain/`（裁决器 + 观察器）在仓库里、且可被守卫扫到；
#   ② 「判据串凭空捏造」不再可能 —— O1 判据 + 裁决器的**前提自证**双保险。
#
# 层：
#   [1] 守卫自证 5/0（4 档夹具 + 1 条反向判据）
#   [2] 真仓扫描 0 违例 + 规模下限
#   [3] 裁决器六档（红 / 矛盾×2 / 未跑完 / 超时 / 前提不成立）逐档判对
#   [4] 回归判据：**M283 那次假警报的形状** —— 真绿日志必须判 GREEN
#       + 缺陷 496：凭空串无生产者 / 未跟踪生产者可扫到（两档对照）/ 查询行不算生产者
#   [5] 负控 3 道（各自独立判红 · 源逐字节还原）
#   [6] 覆盖边界
#
# ⚠️ 纪律（本门自己遵守）：判据里不写死开发机路径（缺陷 300）· 提示文本用单引号
# ⚠️ 本门**不提供** `--neg-skip` / `NEG_SKIP`：三道负控都不重建 runtime（≈1s）
#   ⇒ 任何档位都**全跑**（比「提供开关但被忽略」更诚实 —— 缺陷 496 同族的配置名实问题）。

#    （反引号在双引号里会触发命令替换）· `local a="$1"; local b="$W/$a"` 拆两行
#    （bash 5.1 + set -u 下会取到外层变量）。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }

ROOT="${1:-$(cd "$(dirname "$0")/../.." && pwd)}"
cd "$ROOT" || exit 1
W="${M284_W:-/tmp/m284_gate}"
rm -rf "$W"; mkdir -p "$W"

PASS=0; FAIL=0
VB="packaging/chain/gate_verdict.sh"
GUARD="selfhost/check_orchestrator.py"

ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$1"; }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

printf '\n══ [1] 守卫自证（含反向判据）══\n'
ST="$(python3 "$GUARD" --self-test 2>&1)"; rc=$?
printf '%s\n' "$ST" | sed 's/^/     /'
if [ "$rc" = 0 ] && printf '%s' "$ST" | grep -q '自证：通过 5 / 失败 0'; then
    ok '守卫自证 5/0'
else
    bad "守卫自证失败（rc=$rc）"
fi

printf '\n══ [2] 真仓扫描 + 规模下限 ══\n'
SC="$(timeout 600 python3 "$GUARD" 2>&1)"; rc=$?
printf '%s\n' "$SC" | tail -3 | sed 's/^/     /'
if [ "$rc" = 0 ] && printf '%s' "$SC" | grep -q '违例 0'; then
    ok '真仓 0 违例'
else
    bad "真仓有违例（rc=$rc）"
fi
NF=$(printf '%s' "$SC" | awk '{for(i=1;i<=NF;i++) if($i=="扫描"){print $(i+1); exit}}')
NFC=${NF:-0}
if [ "$NFC" -ge 55 ]; then ok "扫描面规模（$NFC 个 .sh ≥ 55）"; else bad "扫描面过窄（$NFC < 55）"; fi
NO=$(printf '%s' "$SC" | awk '{ if (match($0, /编排脚本 [0-9]+ 个/)) {
        s=substr($0,RSTART,RLENGTH); gsub(/[^0-9]/,"",s); print s+0; exit } }')
NOC=${NO:-0}
if [ "$NOC" -ge 5 ]; then ok "含门调用的编排脚本（$NOC ≥ 5）"; else bad "编排脚本过少（$NOC < 5）"; fi
if [ -f "$VB" ] && [ -f "$GUARD" ] && [ -f packaging/chain/README.md ]; then
    ok '入仓三件齐（裁决器 / 守卫 / 文档）'
else
    bad '入仓件缺失'
fi
# 判定串规模锚点（防判据静默变窄 —— M199/M201/M226 立过「计数只做下限」）
CNT="$(timeout 600 python3 "$GUARD" --count 2>&1 | tail -1)"
printf '     %s\n' "$CNT"
NC=$(printf '%s' "$CNT" | awk '{ if (match($0, /判定串 [0-9]+ 条/)) {
        s=substr($0,RSTART,RLENGTH); gsub(/[^0-9]/,"",s); print s+0; exit } }')
NCC=${NC:-0}
if [ "$NCC" -ge 15 ]; then ok "判定串规模锚点（$NCC ≥ 15）"; else bad "判定串过少（$NCC < 15）"; fi

printf '\n══ [3] 裁决器六档 ══\n'
mk() { printf '%b' "$2" > "$W/$1"; }
mk red.log      '✅ a（1s）\n❌ b（rc=1, 2s）\n══ 汇总：失败 1 项（总耗时 3s）══\n'
mk contra.log   '✅ a（1s）\n══ 汇总：失败 1 项（总耗时 3s）══\n'
mk contra2.log  '❌ b（rc=1, 2s）\n══ 汇总：失败 0 项（总耗时 3s）══\n'
mk nosum.log    '✅ a（1s）\n'
mk tmo.log      '⏱  c（**超时**：900s ≥ 上限 900s，rc=124）\n══ 汇总：失败 1 项（总耗时 900s）══\n'
printf 'hello\n' > "$W/badrunner.sh"

vcheck() {   # $1=名 $2=日志 $3=期望VERDICT $4=期望rc $5…=额外参数
    local nm="$1"; local lg="$2"; local ev="$3"; local erc="$4"; shift 4
    local out; local rc
    out="$(bash "$VB" "$lg" --quiet "$@" 2>&1)"; rc=$?
    if [ "$rc" = "$erc" ] && printf '%s' "$out" | grep -q "^VERDICT=$ev$"; then
        ok "裁决/$nm ⇒ $ev (rc=$erc)"
    else
        bad "裁决/$nm：期望 $ev/$erc，实际 $(printf '%s' "$out" | head -1)/$rc"
    fi
}
CANON="selfhost/run_gates.sh"
vcheck 'red'      "$W/red.log"     RED          1 --runner "$CANON"
vcheck 'contra'   "$W/contra.log"  INCONSISTENT 2 --runner "$CANON"
vcheck 'contra2'  "$W/contra2.log" INCONSISTENT 2 --runner "$CANON"
vcheck 'nosum'    "$W/nosum.log"   INCONSISTENT 2 --runner "$CANON"
vcheck 'timeout'  "$W/tmo.log"     RED          1 --runner "$CANON"
vcheck 'premise'  "$W/red.log"     PREMISE-FAIL 3 --runner "$W/badrunner.sh"

printf '\n══ [4] 回归：M283 假警报的形状 ══\n'
# 写一份与 M283 真日志**同形**的绿日志（190 门 ✅ · 汇总 0 项 · 无失败行）
{ i=0; while [ "$i" -lt 190 ]; do echo "✅ gate_$i（1s）"; i=$((i+1)); done
  echo ''; echo '══ 汇总：失败 0 项（总耗时 5782s）══'; } > "$W/m283like.log"
out="$(bash "$VB" "$W/m283like.log" --runner "$CANON" --quiet 2>&1)"
if printf '%s' "$out" | grep -q '^VERDICT=GREEN$'; then
    ok 'M283 同形绿日志 ⇒ GREEN（旧 watcher 在此判红）'
else
    bad "M283 同形绿日志未判绿：$(printf '%s' "$out" | head -1)"
fi
# 并证明**那个凭空串**确实"无生产者"（否则 O1 判据的立论不成立）。
# ⚠️ 必须用守卫自己的探针，**不能**用裸 `git grep`：
#   裸 git grep 会把 CHANGELOG / 注释里的**引用**当成"存在" —— v1→v2 就是这么踩的
#   （我的取证文本自己把那个串变成了"在仓库里"，自证当场从 5/0 掉到 4/1）。
# ⚠️ M284s1（缺陷 496）：这个串在**本文件里也必须运行时拼装** —— 写成字面量的话，
#   **这一行自己就成了它的「生产者」**（实测：本门就是被这行坑的，5/0 → 4/1）。
FAKE_A='双路'; FAKE_B='判据一致'; FAKE="${FAKE_A}${FAKE_B}"
if python3 "$GUARD" --probe "$FAKE" >/dev/null 2>&1; then
    bad '凭空串竟有生产者 —— O1 的立论不成立'
else
    ok '凭空串 无生产者（O1 立论成立）'
fi
# ── 缺陷 496 的**两档对照**（各自证明一条修正有牙）──
#   ① 未跟踪面：造一个**未跟踪**的真生产者 ⇒ 默认档必须在场；`ORCH_TRACKED_ONLY=1`
#      （修前口径）必须凭空。
#   ② 查询行：同一探针，换成**只把串当参数交给查询工具**（`--probe 'X'`）⇒ 必须仍**凭空**。
#   ⚠️ 骨架一律**运行时拼装**；⚠️ 临时目录跑完**必删**（工作树卫生：M254 负控残留守卫）。
UD="$ROOT/.m284s1_untracked"
mkdir -p "$UD"
printf '#!/usr/bin/env bash\necho %s\n' "$FAKE" > "$UD/prod.sh"
if python3 "$GUARD" --probe "$FAKE" >/dev/null 2>&1; then
    ok '496① 未跟踪的生产者被扫到（默认档 ⇒ 在场）'
else
    bad '496① 未跟踪的生产者未被扫到（扫描面仍只看已跟踪）'
fi
if ORCH_TRACKED_ONLY=1 python3 "$GUARD" --probe "$FAKE" >/dev/null 2>&1; then
    bad '496① 对照档也判在场 —— 说明未跟踪面根本没参与判定（判据没牙）'
else
    ok '496① 对照：只看已跟踪 ⇒ 凭空（⇒ 未跟踪修正有牙）'
fi
printf '#!/usr/bin/env bash\npython3 selfhost/check_orchestrator.py --probe %s\n' "$FAKE" > "$UD/qq.sh"
rm -f "$UD/prod.sh"
if python3 "$GUARD" --probe "$FAKE" >/dev/null 2>&1; then
    bad '496③ 只把串交给查询工具的行被当成了生产者'
else
    ok '496③ 查询工具调用行不算生产者（QUERY_RE 有牙）'
fi
rm -rf "$UD"
if python3 "$GUARD" --probe "$FAKE" >/dev/null 2>&1; then
    bad '删掉临时生产者后仍判在场（有残留？）'
else
    ok '删掉后回到凭空（未跟踪面无残留）'
fi
# ── 静态判据：两处修正的**代码形态**在位（防日后被"顺手改回去"）──
U1="$(grep -c -e '--untracked' "$GUARD" || true)"
U2="$(grep -c -e 'QUERY_RE.search' "$GUARD" || true)"
if [ "${U1:-0}" -ge 1 ]; then ok "守卫用 --untracked（$U1 处）"; else bad '守卫未用 --untracked（缺陷 496① 回归）'; fi
if [ "${U2:-0}" -ge 1 ]; then ok "QUERY_RE 真正生效（$U2 处）"; else bad 'QUERY_RE 只定义没使用（缺陷 496③ 回归）'; fi
# 对照判据：同一探针对一个**真在场**的骨架必须判"在场"（否则探针是恒假的口径）
if python3 "$GUARD" --probe '汇总' >/dev/null 2>&1; then
    ok '对照：骨架 汇总 有生产者（探针非常假）'
else
    bad '对照失败：骨架 汇总 竟报凭空 —— 探针口径恒假'
fi

printf '\n══ [5] 负控 3 道（各自独立判红 · 源逐字节还原）══\n'
snap() { cp -p "$1" "$W/$(basename "$1").snap"; }
rest() { cp -p "$W/$(basename "$1").snap" "$1"; }
snap "$VB"; snap "$GUARD"
trap 'rest() { cp -p "$W/$(basename "$1").snap" "$1" 2>/dev/null || true; }; rest "'"$VB"'"; rest "'"$GUARD"'"; rm -rf "'"$W"'"' EXIT

# NC-A：裁决器的**前提自证**改成恒真 ⇒ premise 档不再 rc=3
python3 - "$VB" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
new = s.replace("        grep -qF -e \"$_tok\" \"$RUNNER\" || _missing=\"$_missing $_tok\"",
                "        true || _missing=\"$_missing $_tok\"")
if new == s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(new)
PY
if [ $? = 0 ]; then
    out="$(bash "$VB" "$W/red.log" --quiet --runner "$W/badrunner.sh" 2>&1)"; rc=$?
    if [ "$rc" != 3 ]; then ok "NC-A：抽掉前提自证 ⇒ 不再 rc=3（rc=$rc）"; else bad 'NC-A：前提自证仍有牙？'; fi
else
    bad 'NC-A 锚点未命中（源码已变？）'
fi
rest "$VB"

# NC-B：O1 改成恒真（判据自伤）⇒ 真仓/夹具不再判红
python3 - "$GUARD" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
new = s.replace("    if not (CJK.search(pat) and len(pat) >= 4):",
                "    if True:")
if new == s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(new)
PY
if [ $? = 0 ]; then
    out="$(python3 "$GUARD" --file examples/m284_orchestrator/fixtures/bad_o1.sh 2>&1)"; rc=$?
    if [ "$rc" = 0 ]; then ok 'NC-B：O1 恒真 ⇒ bad_o1 不再判红（判据自伤证明）'; else bad 'NC-B：判据自伤后仍红？'; fi
else
    bad 'NC-B 锚点未命中'
fi
rest "$GUARD"

# NC-C：去掉 O3 ⇒ bad_o3 不再判红
python3 - "$GUARD" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
new = s.replace("        for t in P1_RE.findall(line):\n            p1_topics.add((t, i))",
                "        for t in []:\n            p1_topics.add((t, i))")
if new == s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(new)
PY
if [ $? = 0 ]; then
    out="$(python3 "$GUARD" --file examples/m284_orchestrator/fixtures/bad_o3.sh 2>&1)"; rc=$?
    if [ "$rc" = 0 ]; then ok 'NC-C：去 O3 ⇒ bad_o3 不再判红'; else bad 'NC-C：去 O3 后仍红？'; fi
else
    bad 'NC-C 锚点未命中'
fi
rest "$GUARD"

# 源逐字节还原
r1=0; r2=0
cmp -s "$VB" "$W/$(basename "$VB").snap" || r1=1
cmp -s "$GUARD" "$W/$(basename "$GUARD").snap" || r2=1
if [ "$r1" = 0 ] && [ "$r2" = 0 ]; then ok '负控后源逐字节还原'; else bad '源未还原'; fi

printf '\n══ [6] 覆盖边界 ══\n'
cat <<'TXT'
  · O1 只覆盖 **中文** 判定串（英文/符号串如 PASS/OK/^✅ 不判 —— 它们可能来自第三方工具，
    全仓在场性不成立）；纯 ASCII 判据串的凭空风险**未覆盖**（下一轮候选）。
  · O2 只认 `run_gates.sh` / `m11{6,7}_gates.sh` 三个门运行器名；**改名要同步**（由
    `check_gate_registry` 与本条注释共同提醒）。
  · O3 按 `-` 前缀配对（`m284-red` → `m284`）；**跨文件**的了结（A 挂 B 了结）未覆盖。
  · 裁决器只解析 `run_gates.sh` 的现行输出格式；格式改了会被**前提自证**挡住（rc=3），
    但那时**判断已交回人工** —— 这不是自动修复。
  · 本门动态面**不覆盖真实推送**（`--dry` 档只到 make_tag 之前的判定）。
  · 缺陷 496（M284s1）只收口「**生产者搜索**」这一侧的扫描面与查询行规则；
    `check_no_secrets.sh` 的同类修正（缺陷 491）在 M282 已做 —— 但**别的守卫**是否
    还有「只看已跟踪」的扫描面，**未逐个体检**（下一轮候选）。
  · 496 的 `ORCH_TRACKED_ONLY` **只用于 A/B**；正常路径不得设置（本门已两档对照证明有牙）。
  · v3 试过「生产者必须含输出关键词」—— **实测被否**（凭空 0 → 7 全误伤：`.px` 字面量
    赋值 + 别的脚本 echo 出的中文短语）⇒ 已撤回，并把判据注释与实现改成一致。
TXT
ok '覆盖边界已登记（见上）'

printf '\n══ 汇总：通过 %d / 失败 %d ══\n' "$PASS" "$FAIL"
[ "$FAIL" = 0 ] && { echo 'M284-VERIFY-OK'; rm -rf "$W"; exit 0; }
echo 'M284-VERIFY-FAIL'
exit 1
