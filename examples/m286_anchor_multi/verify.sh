#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M286 门：打桩锚点的**唯一性**（判据 A4 · A1 的自然强化）
#
# 为什么存在（A1 只做了一半）：
#   A1（M285）判「`sed -i 's|OLD|NEW|'` 里 OLD **在不在**目标里」——
#   但**不判 OLD 出现几次**。而 sed 是**逐行替换**（无 g）⇒ OLD 在 N 行 ⇒ **N 处都被改**。
#   门通常只想改**一处** ⇒ 多改 = 「打桩范围**超出声明意图**」。真仓实测 3 例：
#     · M200:145 改 3 处（:51 目标 + :164 `ffi_call` + :202 `px_ffi_has`）—— 后两处是**别的函数**
#     · M196:165 改 2 处（:27338 native 面 + :3816 核心 `px_gen_next`）—— 而门**声明只改原生**
#     · M263:78  改 2 处 —— 但那是**同一修复的两个分支**（缺陷 466）⇒ 「多改」**正是意图** ⇒ 豁免
#   ⚠️ 危险性：多改往往**不影响本门判据** ⇒ 长期无人发现；但它让「门声明的前提」变成假的，
#      甚至可能让**另一个**判据"更容易红" ⇒ **虚假的判据强度**。
#
# 本轮顺带修掉的一个**判据盲区**（自己撞出来的）：
#   把 m200/m196 改成**范围地址**（`/^void f/,/^}/ s|…|`）后，`SED_CALL` 正则
#   （要求 `s` 紧跟引号）**不认识地址前缀** ⇒ 这两处打桩**整条从判据视野里消失**
#   ⇒ 「修好了」是**假绿**（不是精确了，是**看不见了**）。计数 40 → 38 就是这个症状。
#   ⇒ 已扩正则（顺带多抓出 9 处旧正则漏掉的 `Ns/.*/…/` 整行替换写法）。
#
# 层：
#   [1] 守卫自证 15/0（8 档夹具 + 3 条反向判据，含 A4 的**四类形状**）
#   [2] 真仓 0 违例 + 规模锚点 + 豁免表生效
#   [3] **判据回放**：真仓**已修**的两处（范围地址）⇒ 必须**放过**；
#       且 \(\*）它们在文件里 OLD **仍然**多行（3 / 2）⇒ 证明「不红来自范围感知」而非「文件变了」
#   [4] 逐类形状 6 档（无地址多行 / 范围+1 / 范围+2 / 范围+0 / 整行替换 / 目标文件）
#   [5] 负控 3 道（各自独立判红 · 源逐字节还原）
#       NC-A `count_old` 恒 1                ⇒ 夹具多行锚点**不再**判红
#       NC-B **忽略地址**（`addr = None`）    ⇒ ⭐ 真仓 m200/m196 **必须重新判红**（有地址≠免死）
#       NC-C 判据自伤（A4 的码改成 A4X）      ⇒ NC-A/NC-B 的红**必须消失**
#   [6] 覆盖边界（如实登记）
#
# ⚠️ 纪律：提示文本用**单引号**（反引号在双引号里会触发命令替换）·
#    `local a="$1"` 与 `local b=…` 拆两行（bash5.1+set -u 会取到外层）· 不写死开发机路径（缺陷 300）
# ⚠️ 不提供 `--neg-skip`：三道负控都只改 Python 守卫（不重建 runtime，≈1s）⇒ 任何档位都全跑。
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
W="${M286_W:-/tmp/m286_gate}"
rm -rf "$W"; mkdir -p "$W"

G="selfhost/check_gate_assertions.py"
TSV="selfhost/gate_anchor_multi.tsv"
FIX="examples/m286_anchor_multi/fixtures"

PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"; }
bad() { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

printf '\n══ [1] 守卫自证（含 A4 四类形状 + 3 条反向判据）══\n'
out="$(python3 "$G" --self-test 2>&1)"; rc=$?
printf '%s\n' "$out" | sed 's/^/  | /'
chk '自证 rc' "$rc" '0'
n="$(printf '%s' "$out" | grep -c '✅')"
[ "$n" -ge 15 ] && ok "自证条目 $n ≥ 15" || bad "自证条目过少（$n < 15）"
for k in bad_a4_multi.sh bad_a4_range2.sh bad_a4_miss.sh ok_a4_range.sh ok_a4_whole.sh '反转-A4'; do
  if printf '%s' "$out" | grep -q -- "$k"; then ok "自证含 $k"; else bad "自证缺 $k"; fi
done

printf '\n══ [2] 真仓扫描 + 规模锚点 + 豁免表 ══\n'
out="$(python3 "$G" --count 2>&1)"; rc=$?
printf '%s\n' "$out" | sed 's/^/  | /'
chk '真仓 rc' "$rc" '0'
NSED="$(printf '%s' "$out" | grep -o 'sed -i [0-9]*' | head -1 | grep -o '[0-9]*')"
NSEP="$(printf '%s' "$out" | grep -o '可解析 [0-9]*' | head -1 | grep -o '[0-9]*')"
NML="$(printf '%s' "$out" | grep -o '锚点多行 [0-9]*' | head -1 | grep -o '[0-9]*')"
NWV="$(printf '%s' "$out" | grep -o '豁免 [0-9]*' | head -1 | grep -o '[0-9]*')"
[ "${NSED:-0}" -ge 45 ] && ok "sed -i 规模（$NSED ≥ 45 · 含行地址写法）" || bad "sed -i 过少（$NSED < 45）"
[ "${NSEP:-0}" -ge 18 ] && ok "目标可解析（$NSEP ≥ 18）" || bad "可解析过少（$NSEP < 18）"
chk '未豁免的多行锚点（本仓应为 0：M200/M196 已修 + M263 已豁免）' "${NML:-x}" '0'
[ "${NWV:-0}" -ge 1 ] && ok "豁免表生效（$NWV ≥ 1）" || bad "豁免表未生效"
if [ -f "$TSV" ]; then
  nh="$(grep -vc '^#' "$TSV" || true)"
  [ "${nh:-0}" -ge 1 ] && ok "豁免表条目 $nh ≥ 1" || bad "豁免表为空（过期判据会判红？）"
  grep -q 'M286' "$TSV" && ok '豁免表头注含 M286 口径' || bad '豁免表缺 M286 口径注'
else
  bad "豁免表缺失（$TSV）"
fi

printf '\n══ [3] 判据回放：真仓**已修**的两处（范围地址）⇒ 必须放过 ══\n'
for d in m200_ffi_globals m196_msg_parity; do
  python3 "$G" --file "examples/$d/verify.sh" > "$W/pf_$d.log" 2>&1; r=$?
  if [ "$r" = 0 ]; then ok "回放 $d：范围地址 ⇒ 不判红"
  else bad "回放 $d：被判红（范围感知失效？）"; sed 's/^/      /' "$W/pf_$d.log"; fi
done
# ⭐ 关键：OLD 在文件里**仍然**多行 ⇒ 「不红」只可能来自**范围感知**
n1="$(grep -c 'for (i = 0; i < g_ffi_n; i++) {' runtime/runtime_ffi.c)"
n2="$(grep -c 'px_error("R1002: gen_next 需要生成器对象")' runtime/runtime.c)"
chk 'runtime_ffi.c 里 OLD 仍 3 行（⇒ 不红来自范围，不是文件变了）' "$n1" '3'
chk 'runtime.c 里 OLD 仍 2 行（同上）' "$n2" '2'

printf '\n══ [4] 逐类形状（%s）══\n' "$FIX"
shape() {   # $1=夹具 $2=期望码（空=必须不红）
  local name="$1"
  local want="${2:-}"
  python3 "$G" --file "$FIX/$name" > "$W/sh_$name.log" 2>&1; local rc=$?
  if [ -z "$want" ]; then
    if [ "$rc" = 0 ]; then ok "$name：不判红（符合期望）"
    else bad "$name：期望不红却 rc=$rc"; sed 's/^/      /' "$W/sh_$name.log"; fi
  else
    if [ "$rc" = 1 ] && grep -q "\[$want\]" "$W/sh_$name.log"; then ok "$name：判红且码=$want"
    else bad "$name：期望码 $want，实际 rc=$rc"; sed 's/^/      /' "$W/sh_$name.log"; fi
  fi
}
shape bad_a4_multi.sh   A4
shape bad_a4_range2.sh  A4
shape bad_a4_miss.sh    A1r
shape ok_a4_range.sh
shape ok_a4_whole.sh

printf '\n══ [5] 负控 3 道（各自独立判红 · 源逐字节还原）══\n'
snap() { cp -p "$1" "$W/$(basename "$1").snap"; }
rest() { cp -p "$W/$(basename "$1").snap" "$1"; }
snap "$G"
trap 'cp -p "$W/check_gate_assertions.py.snap" "$G" 2>/dev/null || true; rm -rf "$W"' EXIT

# NC-A：count_old 恒 1 ⇒ 多行锚点（夹具）必须**不再**判红
python3 - "$G" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "    m = _SED_ADDR_RE.match(addr) if addr else None"
new = "    return 1  # NEGCTL-A\n" + old
if old not in s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
if [ $? = 0 ]; then
  python3 "$G" --file "$FIX/bad_a4_multi.sh" >"$W/nca.log" 2>&1; rca=$?
  if [ "$rca" = 0 ]; then ok 'NC-A：count_old 恒 1 ⇒ 多行锚点不再判红（红确实来自 A4）'
  else bad "NC-A：关掉后仍 rc=$rca"; sed 's/^/      /' "$W/nca.log"; fi
else
  bad 'NC-A 锚点未命中（源码已变？）'
fi
rest "$G"

# NC-B：**忽略地址** ⇒ ⭐ 真仓 m200/m196 必须**重新判红**（有地址 ≠ 免死）
python3 - "$G" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "    m = _SED_ADDR_RE.match(addr) if addr else None"
new = "    addr = None  # NEGCTL-B\n" + old
if old not in s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
if [ $? = 0 ]; then
  nbad=0
  for d in m200_ffi_globals m196_msg_parity; do
    python3 "$G" --file "examples/$d/verify.sh" > "$W/ncb_$d.log" 2>&1; r=$?
    if [ "$r" = 1 ] && grep -q '\[A4\]' "$W/ncb_$d.log"; then nbad=$((nbad+1)); fi
  done
  if [ "$nbad" = 2 ]; then ok 'NC-B：忽略地址 ⇒ 真仓两处**重新判红**（「不红」确实来自范围感知）'
  else bad "NC-B：忽略地址后只有 $nbad/2 处判红"; cat "$W/ncb_m200_ffi_globals.log" "$W/ncb_m196_msg_parity.log" 2>/dev/null | sed 's/^/      /'; fi
else
  bad 'NC-B 锚点未命中'
fi
rest "$G"

# NC-C：**判据自伤** —— 在 NC-B 的基础上把 A4 的码改掉 ⇒ 红必须消失
python3 - "$G" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "    m = _SED_ADDR_RE.match(addr) if addr else None"
new = "    addr = None  # NEGCTL-C\n" + old
if old not in s:
    print("ANCHOR-MISS"); sys.exit(9)
s = s.replace(old, new, 1)
old2 = 'bad.append(("A4", i, "打桩锚点不唯一'
new2 = 'bad.append(("A4X", i, "打桩锚点不唯一'
if old2 not in s:
    print("ANCHOR-MISS2"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(s.replace(old2, new2, 1))
PY
if [ $? = 0 ]; then
  python3 "$G" --file "examples/m200_ffi_globals/verify.sh" >"$W/ncc.log" 2>&1; rcc=$?
  # ⚠️ 判据：**`[A4]` 这个码必须不再出现**（而不是"整个 rc=0"）——
  #    因为把码改成 A4X 后**违例数仍为 1**（A4X 也是违例）⇒ 用 rc 判会**误判成"红没消失"**（首版即如此）。
  if [ "$rcc" = 1 ] && ! grep -q '\[A4\]' "$W/ncc.log"; then
    ok 'NC-C（自伤）：把 A4 的码改掉 ⇒ [A4] 不再出现（红确实由 A4 判据产生）'
  else bad "NC-C：判据自伤后仍见 [A4]（红可能来自别处）rc=$rcc"; sed 's/^/      /' "$W/ncc.log"; fi
else
  bad 'NC-C 锚点未命中'
fi
rest "$G"

if cmp -s "$G" "$W/check_gate_assertions.py.snap"; then ok '负控后源逐字节还原'; else bad '负控后源**未**还原'; fi

printf '\n══ [6] 覆盖边界（如实登记）══\n'
cat <<'TXT'
  · A4 只判**目标可解析到仓库内文件**的打桩（与 A1 同面）—— 目标是 `$W/xxx`（运行期生成）
    或**临时目录下任意路径**的**不在判据内**。实测 49 处 `sed -i` 里 **22** 处可解析。
  · 「多行」的**语义**由 sed 的**行地址**决定：
      - **无地址**      ⇒ 全文件计数（`sed` 逐行替换 ⇒ N 行就改 N 处）
      - **范围地址**    ⇒ 只数 `/re1/,/re2/` 覆盖范围内的 OLD 行数（实测 m200/m196 修好后各 1 处）
      - **单地址** `N`  ⇒ 只那一行
      - `$`            ⇒ 最后一行
  · **BRE 语义**：地址里的 `(` `)` `{` `}` `+` `?` `|` 是**字面**字符 —— 由 `bre_to_ere` 转换；
    转换失败/地址不可解析 ⇒ **回退全文件计数**（fail-safe 方向：宁可多报，不可漏报）。
  · **整行替换**（`sed -i 'Ns/.*/NEW/' <数据文件>`，OLD 是通配不是锚点）**显式排除**，不判 A1/A4：
    来源 = SED_CALL 支持行地址后多抓出的 **9 处**真实写法（m136/m138/m142/m143/m146/m147/m148）。
    ⚠️ 它们当前的目标是 `build/xxx`（不可解析）⇒ 本来就走不到判据；本排除是**防御性**的
    （防某天数据文件落进仓库 ⇒ 当场假红）。
  · **豁免**只准登记「多改无害」（同一修复的副本 / 同一逻辑）。真缺陷（改到**别的函数**、
    **别的轨道**）一律**修代码**。豁免表**双向**：漏登记判红 · **过期**（实测不再多行）判红。
  · **未做（仍开着）**：「判据串**恒真**」（`grep -q .` 那类）—— M286 侦察结论是真仓 ≈ 0 例
    （空 pattern / 纯锚 pattern 仅数例且都有正当语义；目标可解析者实测覆盖率 100% 仅 1 例且为格式判定）
    ⇒ 本轮改做**同族里真正有数据的**这条（锚点唯一性）。恒真这条**仍开着**。
TXT
ok '覆盖边界已登记（见上）'

printf '\n══ 汇总：通过 %d / 失败 %d ══\n' "$PASS" "$FAIL"
[ "$FAIL" = 0 ] && echo 'M286-VERIFY-OK' || echo 'M286-VERIFY-FAIL'
exit "$FAIL"
