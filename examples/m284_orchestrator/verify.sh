#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M284 门：编排骨架入仓 + 判据串在场守卫（缺陷 490 / 495）
#
# 本门护住两件事：
#   ① `packaging/chain/`（裁决器 + 观察器）在仓库里、且可被守卫扫到；
#   ② 「判据串凭空捏造」不再可能 —— O1 判据 + 裁决器的**前提自证**双保险。
#
# 层：
#   [1] 守卫自证 10/0（4 档夹具 + 1 条反向判据 + M326 的 5 条扫描面三态/负控）
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

# M288s1（2026-10-08 CI 实测）：**位置参数不能当 ROOT** —— CI 会传 `--neg-skip`，
#   而 `cd "--neg-skip"` 会让 cd 去解析选项 ⇒ `cd: --: invalid option` ⇒ 门在 CI 上
#   **0s 判红、且一个字都没输出**（真因完全读不出来）。
#   ⇒ ROOT 走环境变量（`ROOT=`），argv[1] 仅在**不是开关**时按旧约定接受。
ROOT="${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
case "${1:-}" in
    ""|--*) ;;
    *) ROOT="$1" ;;
esac
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
if [ "$rc" = 0 ] && grep -q '自证：通过 10 / 失败 0' <<< "$ST"; then
    # M326：自证从 5 条扩到 10 条（+S1..S5 扫描面三态/双负控）。数字是**锚点**，
    # 少一条就判红 —— 防「自证被悄悄删掉几条而总数看着还像那么回事」。
    ok '守卫自证 10/0'
else
    bad "守卫自证失败（rc=$rc）"
fi

printf '\n══ [2] 真仓扫描 + 规模下限 ══\n'
SC="$(timeout 600 python3 "$GUARD" 2>&1)"; rc=$?
printf '%s\n' "$SC" | tail -3 | sed 's/^/     /'
if [ "$rc" = 0 ] && grep -q '违例 0' <<< "$SC"; then
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
    if [ "$rc" = "$erc" ] && grep -q "^VERDICT=$ev$" <<< "$out"; then
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
if grep -q '^VERDICT=GREEN$' <<< "$out"; then
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

printf '\n══ [4b] 缺陷 579：O1 生产者面（无扩展名钩子）══\n'
# 579 的形状（r16 唯一的红）：O1 把 `.githooks/pre-commit` 里那句**真 printf**
#   判成「凭空判据串」。真因不在判据，而在**扫描面**：旧实现用 git grep 的
#   pathspec `-- '*.sh' '*.py' …` 按**扩展名**取面，而 git 钩子必须叫 `pre-commit`
#   （**无扩展名**）⇒ 整份文件被排除。这是同一缺陷族第 3 次（496① / 491 / 579）。
# 下面每条都要能**独立判红**，且前提先自证（CRITERIA ③/④）：
#   ① 真生产者被扫到（r16 那条红不再复现）
#   ② 两档覆盖：无扩展名 + shebang 的前缀生产者「造前凭空 / 造后在场 / 删后凭空」
#   ③ 认**输出**不认**字面**：真生产者改成注释 ⇒ 必须回到凭空（有牙）
#   ④ 放宽面后仍会判红：垃圾串照样凭空（不是「什么都能找到」）
#   ⑤ 静态形态：扩展名 pathspec 白名单已移除、is_code_file 在位（防日后被改窄）
# ⚠️ 骨架一律**运行时拼装**：写成字面量的话，本文件自己就成了它的生产者
#   （M284s1 的教训：自证当场 5/0 → 4/1）。
P='发射'; Q='冻结集'; S579="${P}${Q}"
if python3 "$GUARD" --probe "$S579" >/dev/null 2>&1; then
    ok '579① 真生产者（.githooks/pre-commit 的 printf 行）被扫到'
else
    bad '579① 真生产者未被扫到 —— 扫描面仍按扩展名取面（缺陷 579 未修）'
fi
F1='无扩展名'; F2='生产者对照'; S2="${F1}${F2}"
D579="$ROOT/.m325_579"
rm -rf "$D579"
if python3 "$GUARD" --probe "$S2" >/dev/null 2>&1; then
    bad '579② 前提不成立：对照串在造之前就已在场（负控会是空操作）'
else
    ok '579② 前提自证：对照串造之前凭空'
    mkdir -p "$D579"
    printf '#!/usr/bin/env bash\necho %s\n' "$S2" > "$D579/prod_noext"
    if python3 "$GUARD" --probe "$S2" >/dev/null 2>&1; then
        ok '579② 无扩展名 + shebang 的生产者被扫到（面真覆盖到了）'
    else
        bad '579② 无扩展名 + shebang 的生产者仍被漏掉'
    fi
    rm -rf "$D579"
    if python3 "$GUARD" --probe "$S2" >/dev/null 2>&1; then
        bad '579② 删掉后仍判在场（面内残留 / 判据恒真）'
    else
        ok '579② 删掉后回到凭空（两档对照成立）'
    fi
fi
rm -rf "$D579"
HK='.githooks/pre-commit'
cp -p "$ROOT/$HK" "$W/hk.snap"
# 把真生产者那一行改成注释（**不是删掉**：删掉会同时改变「在场」的两个原因）
python3 - "$ROOT/$HK" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
anchor = '⛔ 提交被拦下：'
i = s.find(anchor)
if i < 0:
    print('ANCHOR-MISS'); sys.exit(9)
ls = s.rfind('\n', 0, i) + 1
io.open(p, 'w', encoding='utf-8').write(s[:ls] + '# ' + s[ls:])
PY
if [ $? = 0 ]; then
    if python3 "$GUARD" --probe "$S579" >/dev/null 2>&1; then
        bad '579③ 生产者改成注释后仍判在场 —— 认的是字面在场，不是输出'
    else
        ok '579③ 生产者改成注释 ⇒ 回到凭空（认输出，有牙）'
    fi
    cp -p "$W/hk.snap" "$ROOT/$HK"
    if python3 "$GUARD" --probe "$S579" >/dev/null 2>&1; then
        ok '579③ 还原后回到在场（对照的另一档）'
    else
        bad '579③ 还原后仍凭空（还原没到位）'
    fi
else
    cp -p "$W/hk.snap" "$ROOT/$HK"
    bad '579③ 锚点未命中（生产者行已变？锚点必须同步）'
fi
if cmp -s "$ROOT/$HK" "$W/hk.snap"; then ok '579 钩子源逐字节还原'; else bad '579 钩子源未还原'; fi
G1='犇鑫骉'; G2='鑫犇'; SG="${G1}${G2}"
if python3 "$GUARD" --probe "$SG" >/dev/null 2>&1; then
    bad '579④ 放宽面后垃圾串也被判在场（面放太宽 ⇒ 判据失去牙）'
else
    ok '579④ 放宽面后垃圾串仍凭空（判据仍有牙）'
fi
SP="$(grep -c 'is_code_file' "$GUARD" || true)"
if [ "${SP:-0}" -ge 2 ]; then
    ok "579⑤ is_code_file 在位（$SP 处：定义 + 调用）"
else
    bad "579⑤ is_code_file 缺失或未接线（${SP:-0} 处）"
fi
if grep -q 'paths = \["\*"' "$GUARD"; then
    bad '579⑤ 扩展名 pathspec 白名单仍在（扫描面可能又被改窄）'
else
    ok '579⑤ 扩展名 pathspec 白名单已移除'
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
  · 579（M325 补 2）把生产者面从「扩展名白名单」改成 `is_code_file()`：有扩展名 ⇒ 查白名单；
    **无扩展名** ⇒ 首两字节是 `#!`。
  · M326 收口 579 的已知洞：**无扩展名 + 无 shebang** 的构建文件按**文件名白名单**认
    （`Makefile` / `makefile` / `GNUmakefile` / `Dockerfile` / `Containerfile`）。
    为什么是白名单而不是「无扩展名一律算」：后者会把 `LICENSE`/`README`/`VERSION` 算进代码面，
    判据串在文档里被引用一次就「在场」⇒ O1 重新失去牙（495 的形状）。
    新边界（同样如实写出来）：白名单**之外**的无扩展名代码（如 `Justfile`）仍不在面内 ——
    加名字 = 口径变更，必须同一提交里改 `NO_EXT_CODE_NAMES` 与 S5 自证。
  · 579 只改「生产者搜索面」，**不改变** O1 的立论（兜里那条判据仍要求 CJK + 长度 ≥ 4）。
    面放宽的代价是**漏报**变多（宁可漏报不可假红），所以 ④ 专门反向验「垃圾串仍凭空」。
TXT
ok '覆盖边界已登记（见上）'

printf '\n══ 汇总：通过 %d / 失败 %d ══\n' "$PASS" "$FAIL"
[ "$FAIL" = 0 ] && { echo 'M284-VERIFY-OK'; rm -rf "$W"; exit 0; }
echo 'M284-VERIFY-FAIL'
exit 1
