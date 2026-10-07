#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M285 门：门层判据守卫（打桩锚点在位 + 判据串在场）+ 共用原语（缺陷 498）
#
# 本门护住三件事：
#   ① **打桩锚点在位**（A1）：`sed -i 's|OLD|NEW|' <目标>` 里 OLD 若已不在目标里，
#      sed **静默不生效** ⇒ 负控**失牙**、门照样 PASS 却什么都没测
#      （M230 原话「锚点失配不会报错，只会静默失效」；M161/M164/M213/M226/M227 反复撞）。
#   ② **判据串在场**（A2）从编排层推到**门层**（M284 只做了编排层，且如实登记了边界）。
#   ③ 缺陷 498：抽「grep 的 pattern」的正则**抽错了参数**（抓到行尾的消息）——
#      M285 把它抽成**共用原语** `selfhost/shellscan.py`，一条规则只留一处。
#
# 层：
#   [1] 守卫自证 6/0（4 档夹具 + 2 条反向判据）
#   [2] 共用原语在位（两个守卫都 import、且都不再自带 iter_grep_pats）
#   [3] 真仓 0 违例 + 规模锚点（防判据静默变窄）
#   [4] **判据回放**：取**真仓**的打桩点，破坏其锚点 ⇒ 必须判红**且指名目标**
#   [5] 逐类反例 + 两条「**必须不红**」的正例（反向断言 / 打桩后状态）
#   [6] 负控 3 道（各自独立判红 · 源逐字节还原）
#   [7] 覆盖边界（如实登记）
#
# ⚠️ 纪律（本门自己遵守）：判据里不写死开发机路径（缺陷 300）· 提示文本用**单引号**
#    （反引号在双引号里会触发命令替换 · 本仓已踩第 5 次）· `local a="$1"; local b="$W/$a"` 拆两行。
# ⚠️ 本门**不提供** `--neg-skip`：三道负控都不重建 runtime（改的是 Python 守卫，≈1s）
#    ⇒ 任何档位都**全跑**。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }

ROOT="${1:-$(cd "$(dirname "$0")/../.." && pwd)}"
cd "$ROOT" || exit 1
W="${M285_W:-/tmp/m285_gate}"
rm -rf "$W"; mkdir -p "$W"

PASS=0; FAIL=0
G="selfhost/check_gate_assertions.py"
SH="selfhost/shellscan.py"
OG="selfhost/check_orchestrator.py"
FIX="examples/m285_gate_assertions/fixtures"

ok()  { PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"; }
bad() { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

printf '\n══ [1] 守卫自证（含反向判据）══\n'
out="$(python3 "$G" --self-test 2>&1)"; rc=$?
printf '%s\n' "$out" | sed 's/^/  | /'
chk '自证 rc' "$rc" '0'
n="$(printf '%s' "$out" | grep -c '✅')"
if [ "$n" -ge 6 ]; then ok "自证条目 $n ≥ 6"; else bad "自证条目过少（$n < 6）"; fi

printf '\n══ [2] 共用原语（一条规则只许一处实现）══\n'
for f in "$G" "$OG"; do
  if grep -q 'from shellscan import' "$f"; then ok "$f 复用 shellscan"; else bad "$f 未复用 shellscan"; fi
  if grep -q '^def iter_grep_pats' "$f"; then bad "$f 仍自带 iter_grep_pats（分叉隐患）"; else ok "$f 不自带 iter_grep_pats"; fi
done
if grep -q 'def mask_prose' "$SH"; then ok 'mask_prose 在共用原语里'; else bad 'mask_prose 缺失'; fi

printf '\n══ [3] 真仓扫描 + 规模锚点 ══\n'
out="$(python3 "$G" --count 2>&1)"; rc=$?
printf '%s\n' "$out" | sed 's/^/  | /'
chk '真仓 rc' "$rc" '0'
NF="$(printf '%s' "$out" | grep -o '扫描 [0-9]* 个' | grep -o '[0-9]*')"
NG2="$(printf '%s' "$out" | grep -o '判定串 [0-9]*' | head -1 | grep -o '[0-9]*')"
NSED="$(printf '%s' "$out" | grep -o 'sed -i [0-9]*' | head -1 | grep -o '[0-9]*')"
NSEP="$(printf '%s' "$out" | grep -o '可解析 [0-9]*' | head -1 | grep -o '[0-9]*')"
NTA="$(printf '%s' "$out" | grep -o '目标 [0-9]*' | head -1 | grep -o '[0-9]*')"
[ "${NF:-0}" -ge 300 ] && ok "扫描面规模（$NF 个 .sh ≥ 300）" || bad "扫描面过窄（$NF < 300）"
[ "${NSED:-0}" -ge 20 ] && ok "sed -i 规模（$NSED ≥ 20）" || bad "sed -i 过少（$NSED < 20）"
[ "${NSEP:-0}" -ge 10 ] && ok "sed 目标可解析（$NSEP ≥ 10）" || bad "sed 目标可解析过少（$NSEP < 10）"
[ "${NG2:-0}" -ge 300 ] && ok "判定串规模（$NG2 ≥ 300）" || bad "判定串过少（$NG2 < 300）"
[ "${NTA:-0}" -ge 10 ] && ok "判定串目标可解析（$NTA ≥ 10）" || bad "目标可解析过少（$NTA < 10）"

printf '\n══ [4] 判据回放：真仓打桩点 + 破坏锚点 ⇒ 必须判红且指名 ══\n'
SRC='examples/m201_bytes_dec/verify.sh'
LN="$(grep -n 'NEGCTL-A' "$SRC" | head -1 | cut -d: -f1)"
if [ -n "$LN" ]; then
  sed -n "${LN}p" "$SRC" > "$W/replay_ok.sh"
  sed 's/bytes_to_dec/bytes_to_dec_GONE/g' "$W/replay_ok.sh" > "$W/replay_bad.sh"
  if python3 "$G" --file "$W/replay_ok.sh" >"$W/r_ok.log" 2>&1; then
    ok '回放对照：锚点在位 ⇒ 不判红'
  else
    bad '回放对照：锚点在位却被判红（判据过严？）'
    sed 's/^/      /' "$W/r_ok.log"
  fi
  python3 "$G" --file "$W/replay_bad.sh" >"$W/r_bad.log" 2>&1; rcb=$?
  chk '破坏锚点后 rc' "$rcb" '1'
  if grep -q '\[A1\]' "$W/r_bad.log" && grep -q 'runtime/runtime.c' "$W/r_bad.log"; then
    ok '破坏锚点 ⇒ A1 判红**且指名目标**'
  else
    bad '破坏锚点未被 A1 指名判红'
    sed 's/^/      /' "$W/r_bad.log"
  fi
else
  bad '回放源锚点未命中（m201 打桩行已变？）'
fi

printf '\n══ [5] 逐类反例 + 两条「必须不红」的正例 ══\n'
pk() {  # $1=名 $2=脚本路径 $3=期望码（空=必须不红）
  local name="$1" path="$2" want="${3:-}"
  python3 "$G" --file "$path" >"$W/c_$name.log" 2>&1; local rc=$?
  if [ -z "$want" ]; then
    if [ "$rc" = 0 ]; then ok "$name：不判红（符合期望）"; else
      bad "$name：期望不红却 rc=$rc"; sed 's/^/      /' "$W/c_$name.log"; fi
  else
    if [ "$rc" = 1 ] && grep -q "\[$want\]" "$W/c_$name.log"; then ok "$name：判红且码=$want"; else
      bad "$name：期望码 $want，实际 rc=$rc"; sed 's/^/      /' "$W/c_$name.log"; fi
  fi
}
printf '#!/usr/bin/env bash\nsed -i %s|PXOP_NOPE_XYZ_285|PXOP_X|%s runtime/vm.h\n' "'s" "'" > "$W/c_a1.sh"
pk 'A1-打桩锚点不在位' "$W/c_a1.sh" 'A1'
printf '#!/usr/bin/env bash\ngrep -q %s运行时黑盒标记%s runtime/vm.h || { bad %s缺锚点%s; exit 2; }\n' "'" "'" '"' '"' > "$W/c_a2f.sh"
pk 'A2-不在目标文件里' "$W/c_a2f.sh" 'A2'
python3 - "$W/c_a2p.sh" <<'PYP'
import io, sys
# ⚠️ **码点拼装**：本门源码里不得出现这个串，否则 `findable` 会把它当"生产者"
#    ⇒ 反例当场假绿（实测）。与守卫 FIXTURES 同款做法。
s = "".join(chr(c) for c in (0x865A, 0x6784, 0x5224, 0x636E, 0x4E32, 0x7532, 0x4E59, 0x4E19))
io.open(sys.argv[1], "w", encoding="utf-8").write(
    "#!/usr/bin/env bash\nif echo x | grep -q '%s'; then echo g; fi\n" % s)
PYP
pk 'A2-凭空判据串' "$W/c_a2p.sh" 'A2'
printf '#!/usr/bin/env bash\ngrep -q %s运行时黑盒标记%s runtime/vm.h && bad %s旧措辞仍在%s\n' "'" "'" '"' '"' > "$W/c_neg.sh"
pk '反向断言（期望缺席）⇒ 不红' "$W/c_neg.sh"
printf '#!/usr/bin/env bash\nsed -i %s|PXOP_ITERLEN|PXOP_ITERLEN_X|%s runtime/vm.h\ngrep -q PXOP_ITERLEN_X runtime/vm.h || bad %s打桩未生效%s\n' "'s" "'" '"' '"' > "$W/c_post.sh"
pk '打桩后状态 ⇒ 不红' "$W/c_post.sh"

printf '\n══ [6] 负控 3 道（各自独立判红 · 源逐字节还原）══\n'
snap() { cp -p "$1" "$W/$(basename "$1").snap"; }
rest() { cp -p "$W/$(basename "$1").snap" "$1"; }
snap "$G"
trap 'cp -p "$W/check_gate_assertions.py.snap" "$G" 2>/dev/null || true; rm -rf "$W"' EXIT

# NC-A：A1 的在场判定恒真 ⇒ 回放点必须**不再**判红
python3 - "$G" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "            if pat_present(old, read(tgt)):"
new = "            if True:  # NEGCTL-A"
if old not in s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
if [ $? = 0 ]; then
  python3 "$G" --file "$W/replay_bad.sh" >"$W/nca.log" 2>&1; rca=$?
  if [ "$rca" = 0 ]; then ok 'NC-A：关掉 A1 ⇒ 破坏锚点不再判红（红确实来自 A1）'; else
    bad "NC-A：关掉 A1 后仍 rc=$rca"; sed 's/^/      /' "$W/nca.log"; fi
else
  bad 'NC-A 锚点未命中（源码已变？）'
fi
rest "$G"

# NC-B：A2 的在场判定恒真 ⇒ 凭空反例必须**不再**判红
python3 - "$G" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "            ok, key = criteria_verdict(pat)"
new = "            ok, key = True, 'NEGCTL-B'"
if old not in s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
if [ $? = 0 ]; then
  python3 "$G" --file "$W/c_a2p.sh" >"$W/ncb.log" 2>&1; rcb2=$?
  if [ "$rcb2" = 0 ]; then ok 'NC-B：关掉 A2 ⇒ 凭空反例不再判红（红确实来自 A2）'; else
    bad "NC-B：关掉 A2 后仍 rc=$rcb2"; sed 's/^/      /' "$W/ncb.log"; fi
else
  bad 'NC-B 锚点未命中'
fi
rest "$G"

# NC-C：**反向**（判据自伤）——把 findable 恒假 ⇒ 真仓必须**报出大量**违例
#       ⇒ 证明「0 违例」不是判据瞎了，而是真的没有违背者
python3 - "$G" <<'PY'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "    ok = git_grep_hit(ROOT, s, paths, extra_excl="
new = "    ok = False or git_grep_hit(ROOT, s, paths, extra_excl="
if old not in s:
    print("ANCHOR-MISS"); sys.exit(9)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1).replace(
    "    CACHE[s] = ok", "    CACHE[s] = False", 1))
PY
if [ $? = 0 ]; then
  out="$(python3 "$G" --count 2>&1)"; rc3=$?
  nv="$(printf '%s' "$out" | grep -o '违例 [0-9]*' | grep -o '[0-9]*')"
  if [ "${nv:-0}" -ge 50 ]; then ok "NC-C（反向）：findable 恒假 ⇒ 真仓报出 $nv 条违例（兜底面有牙）"; else
    bad "NC-C：findable 恒假却只报 ${nv:-0} 条（兜底面可能失效）"; fi
else
  bad 'NC-C 锚点未命中'
fi
rest "$G"

if cmp -s "$G" "$W/check_gate_assertions.py.snap"; then ok '负控后源逐字节还原'; else bad '负控后源**未**还原'; fi

printf '\n══ [7] 覆盖边界（如实登记）══\n'
cat <<'TXT'
  · A1 只判**目标可解析到仓库内文件**的打桩（实测 40 处 sed -i 里 22 处可解析）——
    目标是 `$W/xxx`（运行期生成）或 `/tmp/...` 的**不在判据内**（那是「改内存里的产物」，
    没有静态在场可言）。
  · A1 的「在场」是**多层引用展开后的任一形态命中**（双引号层 ×2 · BRE→字面 · BRE→ERE）
    ⇒ 方向是**放宽**（fail-safe）。代价：极少数「形态碰巧命中」的失配会漏报。
  · A2 只覆盖**中文（CJK）判定串**；纯 ASCII 判据串（`grep -q PASS`）**未覆盖**
    （与 M284 的 O1 同口径 —— 它们可能来自第三方工具，全仓在场性不成立）。
  · A2 的兜底面**刻意排除** `*.md`/CHANGELOG（免疫自指，M284 v2 的教训）⇒
    「生产者在文档里」的判据串会走**目标在场**那条（目标可解析时）而**不是**兜底。
  · A2 跳过**反向断言**（`if grep…; then bad` / `&& VAR=1` / `|| true` / `!`）与
    **同文件打桩后的状态**——两者都是真仓实测存在的合法形状；代价是这两类里的
    真「凭空串」不判（**知道自己在哪儿不看**，好过假红）。
  · `mask_prose` 对「引号里装的是命令」（`chk "说明" "grep -q …"`，会被 eval）**保留**，
    判据是「引号内容以 grep/sed/! 开头」⇒ 形状变了要同步（本门 [5] 有正例护着）。
  · 本门**不判**「判据串是否**恒真**」（如 `grep -q .` 恒匹配）—— 那是另一条判据（下一轮候选）。
TXT
ok '覆盖边界已登记（见上）'

printf '\n══ 汇总：通过 %d / 失败 %d ══\n' "$PASS" "$FAIL"
[ "$FAIL" = 0 ] && echo 'M285-VERIFY-OK' || echo 'M285-VERIFY-FAIL'
exit "$([ "$FAIL" = 0 ] && echo 0 || echo 1)"
