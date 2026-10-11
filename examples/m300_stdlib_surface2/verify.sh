#!/usr/bin/env bash
# ============================================================
# M300 门 · stdlib 覆盖面台账 + 第二批边界面（std.strings + std.url）
# ------------------------------------------------------------
# 立论（M250/M251 的延续）：M279 开了 stdlib 边界面的头，但**只有 21 例**，
#   而 CHANGELOG 写的「第一批取 std.collections + std.path」在实测上是**口头账**
#   （std.path 9 个导出只被碰到 1 个）。⇒ 本轮做两件事：
#     ① **覆盖面台账**（selfhost/stdlib_coverage.py）：把「有多少导出函数
#        从来没被任何门的语料碰过」从**无人能回答**变成**可判定 + 只减不增**；
#     ② **第二批全量对拍**：std.strings(16) + std.url(7) = 23 个导出 × 375 例 × 三轨。
#
# 判据四层（每层独立有牙）：
#   [A] 三轨一致 · [B] 合法侧**独立期望值**（ref.py，不同语言/不同算法）·
#   [C] **确定性**（同二进制跑两遍逐字节相同）· [D] **契约表双向**（过期/漏登记/值变化）
#   另有 [E] 驱动器未命中（NOCASE）—— 防「用例表与驱动器脱节 ⇒ 静默不执行」
#
# 覆盖边界见文件末。
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
  echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M300_W:-/tmp/m300_gate}"
NEG_SKIP="${NEG_SKIP:-0}"
mkdir -p "$W/bin" "$W/logs"
PASS=0; FAIL=0
ok()   { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1（got=$2 want=$3）"; fi; }
jq_()  { python3 -c "import json,sys;d=json.load(open('$1'));print($2)" 2>/dev/null || echo ERR; }

echo "══ M300 · stdlib 覆盖面台账 + 第二批边界面（strings + url）══"

# ── [1] 工具自证 + 生成物一致性 ──────────────────────────────
echo "[1] 工具自证 + 生成物一致性（源码派生 ⇄ 规格表 ⇄ 入库件）"
COV="$ROOT/selfhost/stdlib_coverage.py"
if [ ! -f "$COV" ]; then bad "[1] 缺 $COV"; else
  o="$(timeout 180 python3 "$COV" --self-test 2>&1 | tail -1)"
  case "$o" in *"9 通过 / 0 失败"*) ok "[1a] 覆盖面台账自证 9/0";; *) bad "[1a] 台账自证：$o";; esac

  timeout 300 python3 "$COV" --root "$ROOT" --json "$W/cov.json" > "$W/cov.txt" 2>&1
  rc=$?
  chk "[1b] 覆盖面台账真仓扫描 rc" "$rc" "0"
  n="$(jq_ "$W/cov.json" "d['functions']")"
  u="$(jq_ "$W/cov.json" "d['uncovered']")"
  b="$(jq_ "$W/cov.json" "d['baseline']")"
  cap="$(jq_ "$W/cov.json" "d['cap']")"
  echo "       模块 $(jq_ "$W/cov.json" "d['modules']") · 导出 $n · 未提及 $u · 基线 $b（cap=$cap）"
  # 自证：未提及数必须与基线条数**逐条对应**（否则台账在骗自己）
  chk "[1c] 未提及数 == 基线条数" "$u" "$b"
  if [ "$n" -ge 300 ] 2>/dev/null; then ok "[1d] 规模锚点：导出函数 $n（≥300）"; else bad "[1d] 导出函数仅 $n（<300 —— 派生器坏了？）"; fi
fi

GEN="$HERE/gen.py"; REF="$HERE/ref.py"
if [ ! -f "$GEN" ] || [ ! -f "$REF" ]; then bad "[1e] 缺 gen.py / ref.py"; else
  timeout 300 python3 "$GEN" --root "$ROOT" --work "$W" > "$W/gen.txt" 2>&1
  rc=$?
  chk "[1e] 生成器 rc（规格表 ⇄ 源码派生 双向）" "$rc" "0"
  grep -q "用例" "$W/gen.txt" && sed 's/^/       /' "$W/gen.txt" | tail -1
  # 生成物漂移：重生成必须与入库件**逐字节一致**
  cmp -s "$W/drv.px" "$HERE/drv.px" && ok "[1f] drv.px 与入库件逐字节一致（无漂移）" \
      || bad "[1f] drv.px 与入库件**不一致** ⇒ 改了规格表没重新生成"
  cmp -s "$W/CASES.tsv" "$HERE/CASES.tsv" && ok "[1g] CASES.tsv 与入库件逐字节一致" \
      || bad "[1g] CASES.tsv 与入库件不一致"
  AK="$HERE/ARITY_KNOWN.tsv"
  if [ -f "$AK" ]; then
    na="$(grep -vc '^#' "$AK")"
    ne="$(grep -v '^#' "$AK" | awk -F'\t' 'NF<2 || $2==""' | wc -l)"
    if [ "$na" -ge 40 ] 2>/dev/null && [ "$ne" = "0" ]; then ok "[1i] 已登记分叉表 $na 条 · 理由全非空（只减不增由双向判据自动守）"
    else bad "[1i] 已登记分叉表形态异常（条数 $na · 空理由 $ne）"; fi
  else bad "[1i] 缺 $AK"; fi
  ncase="$(grep -vc '^#' "$HERE/CASES.tsv")"
  if [ "$ncase" -ge 300 ] 2>/dev/null; then ok "[1h] 用例规模 $ncase（≥300）"; else bad "[1h] 用例仅 $ncase（<300）"; fi
fi

# ── [2] 三轨对拍 ────────────────────────────────────────────
echo "[2] 三轨对拍（解释 / VM / C）+ 四层判据"
cp -f "$HERE/drv.px" "$W/drv.px"
timeout 1800 python3 "$HERE/three.py" --root "$ROOT" --work "$W" --cases "$HERE/CASES.tsv" \
        --model "$HERE/MODEL.tsv" --fork-allow "$HERE/ARITY_KNOWN.tsv" \
        --json "$W/three.json" > "$W/three.txt" 2>&1
trc=$?
tail -6 "$W/three.txt" | sed 's/^/     /'
chk "[2a] 三轨对拍 rc" "$trc" "0"
nfork="$(jq_ "$W/three.json" "len(d['diverged'])")"
nallow="$(jq_ "$W/three.json" "len(d['fork_allowed'])")"
echo "       三轨分叉 $nfork（其中**已登记** $nallow 条 —— 缺陷 518，见 ARITY_KNOWN.tsv）"
chk "[2b] **新分叉**（未登记 ⇒ 必须逐条定性后才能进表）" "$(jq_ "$W/three.json" "len(d['fork_new'])")" "0"
chk "[2c] 已登记分叉**过期**（修好了却没更新表）"          "$(jq_ "$W/three.json" "len(d['fork_stale'])")" "0"
chk "[2d] 合法侧期望不符"  "$(jq_ "$W/three.json" "len(d['mismatch'])")" "0"
chk "[2e] 非确定（同二进制两遍不同）" "$(jq_ "$W/three.json" "len(d['nondet'])")" "0"
chk "[2f] 超时（=死循环）" "$(jq_ "$W/three.json" "len(d['timeout'])")" "0"
chk "[2g] 契约表过期"      "$(jq_ "$W/three.json" "len(d['stale'])")" "0"
chk "[2h] 契约表漏登记"    "$(jq_ "$W/three.json" "len(d['unreg'])")" "0"
chk "[2i] 契约值变化"      "$(jq_ "$W/three.json" "len(d['changed'])")" "0"
chk "[2j] 驱动器未命中（NOCASE）" "$(jq_ "$W/three.json" "len(d['nocase'])")" "0"

# ── [2k] 门内并行（M244）：**串行档 ⇄ 并行档读数必须逐条相同** ──
echo "  ── [2k] 门内并行（M244）：GATE_JOBS=1 ⇄ GATE_JOBS=8"
for gj in 1 8; do
  GATE_JOBS=$gj timeout 300 python3 "$HERE/three.py" --root "$ROOT" --work "$W" \
      --cases "$HERE/CASES.tsv" --only '^Ls_count_of' --need 4 --json "$W/par$gj.json" >/dev/null 2>&1 \
      || bad "[2k] GATE_JOBS=$gj 档执行失败"
done
if python3 -c "
import json,sys
a=json.load(open('$W/par1.json'));b=json.load(open('$W/par8.json'))
sys.exit(0 if (a['rows']==b['rows'] and a['n']==b['n']) else 1)
" 2>/dev/null; then
  ok "[2k] 串行 ⇄ 并行 **逐条读数相同**（并行不改语义）"
else
  bad "[2k] 串行/并行档读数不同 ⇒ 并行有副作用"
fi
# 负控：GATE_JOBS 非法必须**响亮**（gate_par 的口径：绝不静默回退）
GATE_JOBS=abc timeout 60 python3 "$HERE/three.py" --root "$ROOT" --work "$W" \
    --cases "$HERE/CASES.tsv" --only '^Ls_count_of_1$' --need 1 --json "$W/parbad.json" >/dev/null 2>&1
grc=$?
if [ "$grc" != "0" ]; then ok "[2k'] GATE_JOBS 非法 ⇒ 响亮失败（rc=$grc）"
else bad "[2k'] GATE_JOBS 非法竟静默通过"; fi

# ── [3] 负控 ────────────────────────────────────────────────
if [ "$NEG_SKIP" = "1" ]; then
  echo "[3] 负控（--neg-skip：CI 档跳过）"
else
  echo "[3] 负控 A/B/C（各自独立 · 只改工作区副本，不动仓库源）"
  SNAP="$W/snap"; mkdir -p "$SNAP"
  cp -f "$HERE/MODEL.tsv" "$SNAP/MODEL.tsv"; cp -f "$HERE/CASES.tsv" "$SNAP/CASES.tsv"
  cp -f "$HERE/three.py" "$SNAP/three.py"

  # NC-A：篡改契约表一条 ⇒ [D] 必须判红（契约层有牙）
  awk -F'\t' 'BEGIN{OFS="\t"} !/^#/ && !done && NF>=2 { $2="9|R9999|TAMPERED"; done=1 } {print}' \
      "$SNAP/MODEL.tsv" > "$W/MODEL.tamper.tsv"
  python3 "$HERE/three.py" --root "$ROOT" --work "$W" --cases "$HERE/CASES.tsv" \
          --model "$W/MODEL.tamper.tsv" --only '^Ls_count_of_1$' --need 1 --runs 1 \
          --json "$W/ncA.json" > "$W/ncA.txt" 2>&1
  nca="$(jq_ "$W/ncA.json" "len(d['changed'])")"
  if [ "$nca" -ge 1 ] 2>/dev/null; then ok "[3A] 篡改契约表 ⇒ 判红（$nca 条值变化）"; else bad "[3A] 篡改契约表**未判红** ⇒ 契约层无牙（$nca）"; fi

  # NC-B：篡改合法侧期望 ⇒ [B] 必须判红（期望层有牙）
  awk -F'\t' 'BEGIN{OFS="\t"} !/^#/ && !done && $2=="legal" { $4="TAMPERED"; done=1 } {print}' \
      "$SNAP/CASES.tsv" > "$W/CASES.tamper.tsv"
  python3 "$HERE/three.py" --root "$ROOT" --work "$W" --cases "$W/CASES.tamper.tsv" \
          --model "$HERE/MODEL.tsv" --only '^Ls_count_of_1$|^Ls_count_of_2$' --need 1 --runs 1 \
          --json "$W/ncB.json" > "$W/ncB.txt" 2>&1
  ncb="$(jq_ "$W/ncB.json" "len(d['mismatch'])")"
  if [ "$ncb" -ge 1 ] 2>/dev/null; then ok "[3B] 篡改期望值 ⇒ 判红（$ncb 条不符）"; else bad "[3B] 篡改期望值**未判红** ⇒ 期望层无牙（$ncb）"; fi

  # NC-C：判据自伤（把结论判据改成恒绿）⇒ 必须**不再**判红（证明上面的红来自判据）
  # ⚠️ 锚点用**结构化**匹配（括号配平），不写死整段表达式：
  #   首版写死了 `bad = (...)` 的原文 ⇒ 后来给 bad 加了两条判据就失配（0 次命中）⇒ 负控自爆。
  #   「负控锚点必须跟着代码走」—— 本仓第 N 次（M161/M164/M226/M227/M299s1）。
  python3 - "$SNAP/three.py" <<'PYEOF'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
i = s.index('    bad = ')
j = s.index('(', i)
depth, k = 0, j
while k < len(s):
    if s[k] == '(':
        depth += 1
    elif s[k] == ')':
        depth -= 1
        if depth == 0:
            break
    k += 1
assert depth == 0, 'NC-C 锚点：括号未配平'
out = s[:i] + '    bad = 0' + s[k+1:]
assert out != s, 'NC-C 锚点命中但没改动'
io.open(p, 'w', encoding='utf-8').write(out)
print('NC-C: bad 判据已置 0（%d 字符 → 9 字符）' % (k + 1 - i))
PYEOF
  python3 "$SNAP/three.py" --root "$ROOT" --work "$W" --cases "$HERE/CASES.tsv" \
          --model "$W/MODEL.tamper.tsv" --only '^Ls_count_of_1$' --need 1 --runs 1 \
          --json "$W/ncC.json" > "$W/ncC.txt" 2>&1
  ncc="$(jq_ "$W/ncC.json" "len(d['changed'])")"
  if [ "$ncc" -ge 1 ] 2>/dev/null; then
    ok "[3C] 判据自伤后**仍能看到**篡改（$ncc 条）—— 说明红来自**判据**而不是读数"
  else
    bad "[3C] 判据自伤后连读数都没了 ⇒ 上面的红来源不明"
  fi
  # 还原保真
  cmp -s "$SNAP/MODEL.tsv" "$HERE/MODEL.tsv" && cmp -s "$SNAP/CASES.tsv" "$HERE/CASES.tsv" \
      && ok "[3D] 工作区副本未被改动（源逐字节保真）" || bad "[3D] 源被改动"
fi

# ── [4] 覆盖边界 ────────────────────────────────────────────
echo "[4] 覆盖边界（如实登记）"
cat <<'EOF'
  · 台账口径是「**语料面提及率**」（函数名以 `name(` 形态出现在 examples/** 里），
    **不是**「被断言过」。它只回答「有没有人碰过它」；判据强度是各门自己的事。
  · 面只取 **std.strings(16) + std.url(7)** 的**公开**导出；`_` 私有函数不入面
    （但 `url_parse`/`url_parse_query` 的字段断言覆盖了它们的效果）。
  · 毒值面 = null/true/7/"7"/[]/{} 六种 × **逐位置** ∪ 错 arity（少 1 / 多 1）。
  · 未知面（**未覆盖，如实登记**）：`time_go`(19) · `html`(26) · `yaml*`(51) ·
    `pxml`(21) · `smtp`(14) · `cookiejar`(17) … —— 台账里逐条可查（cap=只减不增）。
  · 合法侧期望值由 `ref.py`（Python 复刻 Go 语义）独立给出；**不由三轨自证**。
    三轨各自与它比对 ⇒ 「三轨一致地错」也能被发现（M215 的立论）。
  · 不覆盖：`bytes` 逐字节语义（M234 另有门）· 大宽度下 pad/repeat 的**复杂度**
    （O(n²) 属 M270 已登记的语言特性，本门只判语义）· 解释轨 `panic` 的前缀差异
    （缺陷 186 族：只比 R 码 + 消息体，不比整行）。
EOF
ok "覆盖边界已登记"

echo
echo "══ 汇总：通过 $PASS / 失败 $FAIL（SKIP 0）══"
[ "$FAIL" = "0" ] && { echo "M300-VERIFY-OK"; exit 0; } || { echo "M300-VERIFY-FAIL"; exit 1; }
