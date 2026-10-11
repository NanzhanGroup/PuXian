#!/usr/bin/env bash
# ============================================================
# M302 门 · stdlib 第三批（std.semver + std.html）三轨全量对拍
# ------------------------------------------------------------
# 立论：M226（方法面）/M227（同名两门）/M230（索引切片）/M231（运算符矩阵）/
#   M232（真值性）/M279+M300（stdlib 边界面）逐个推过去，而 **stdlib 里用户
#   真正直接调用的那一层**仍有 190 个导出**从未被任何门语料碰过**（M300 台账口径）。
#   第三批取两块**用户面最直接**的：`std.semver`（版本比较/范围，被 tools/pxpkg 用）
#   与 `std.html`（转义/实体/解析，**安全相关**）。
#
# 判据四层（每层独立有牙）：
#   [A] 三轨一致（rc · R 码 · 消息体 · 程序输出）
#   [B] 合法侧**独立期望值**（ref.py：Python 按 spec 实现，**与被测不同构**）
#       + 非法侧**必须响亮**（kind=neg：R 码必须等于契约值）
#   [C] **确定性**（同二进制跑两遍逐字节相同 —— M233 的 UB 指纹判据）
#   [D] **契约表双向**（过期 / 漏登记 / 值变化）
#   另有 [E] 驱动器未命中（NOCASE）—— 防「用例表与驱动器脱节 ⇒ 静默不执行」
#
# 覆盖边界见文件末。
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
  echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M302_W:-/tmp/m302_gate}"
NEG_SKIP="${NEG_SKIP:-0}"
mkdir -p "$W/bin" "$W/logs"
PASS=0; FAIL=0
ok()   { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1（got=$2 want=$3）"; fi; }

SEM="$ROOT/stdlib/semver.px"
HTM="$ROOT/stdlib/html.px"
RSEM="$ROOT/registry/semver/0.1.0/semver.px"
RHTM="$ROOT/registry/html/0.1.0/html.px"

echo "══ M302：stdlib 第三批（semver + html）══"

# ── 快照（负控要改源码 ⇒ 逐字节还原）────────────────────────
SNAP="$W/snap"; mkdir -p "$SNAP"
for f in "$SEM" "$HTM"; do cp -f "$f" "$SNAP/$(basename "$f")"; done
SUM0="$(sha256sum "$SEM" "$HTM" | awk '{print $1}' | tr '\n' ' ')"
restore_all() { cp -f "$SNAP/semver.px" "$SEM"; cp -f "$SNAP/html.px" "$HTM"; }
trap 'restore_all' EXIT

# ── [1] 静态层 ──────────────────────────────────────────────
echo "[1] 静态层（修复在位 + 派生 + 规模 + 锚点）"
# 1a 五处修复的在位断言（逐条对应缺陷 520–526）
declare -a FIX=(
  'lo_mi < 0|缺陷 520：^ 缺段/通配的钳制（防拼出 "0.0.-1"）'
  'if op != ""|缺陷 521：^ / ~ 单独出现 ⇒ 非法范围'
  'panic("sv_cmp: 版本号非法: " + str(a))|缺陷 522：sv_cmp 非法输入响亮'
  'if len(parts) > 3|缺陷 526：范围段数 > 3 ⇒ 非法'
  'if len(pre) == 0|缺陷 525：prerelease/build 空段 ⇒ 非法'
  "elif c == \"'\":|缺陷 524：html_escape 转单引号"
  'u{00a0}|缺陷 523：&nbsp; ⇒ U+00A0'
)
for it in "${FIX[@]}"; do
  pat="${it%%|*}"; desc="${it##*|}"
  n="$(grep -cF "$pat" "$SEM" "$HTM" 2>/dev/null | awk -F: '{s+=$2} END{print s+0}')"
  if [ "${n:-0}" -ge 1 ]; then ok "[1a] $desc"; else bad "[1a] 缺修复：$desc（模式 '$pat'）"; fi
done
# 1b 派生完整性：函数清单从源码派生（不手抄）+ 规模下限
NFN="$(grep -hcE '^def [A-Za-z_]' "$SEM" "$HTM" | awk '{s+=$1} END{print s+0}')"
if [ "$NFN" -ge 30 ]; then ok "[1b] 派生函数数 $NFN（≥30）"; else bad "[1b] 派生函数数 $NFN < 30"; fi
# 1c 契约表规模（SIGS.tsv 的签名条数与 arity 和）
NSIG="$(grep -vcE '^#|^$' "$HERE/SIGS.tsv" 2>/dev/null || echo 0)"
if [ "$NSIG" -ge 28 ]; then ok "[1c] 签名表 $NSIG 条（≥28）"; else bad "[1c] 签名表 $NSIG < 28"; fi
# 1d 负控锚点在位（每处撤回都要能定位到**唯一**一行）
for pat in '        if lo_mi < 0:' '        panic("sv_cmp: 版本号非法: " + str(a))' "        elif c == \"'\":"; do
  n="$(grep -cF "$pat" "$SEM" "$HTM" 2>/dev/null | awk -F: '{s+=$2} END{print s+0}')"
  if [ "${n:-0}" != "1" ]; then bad "[1d] 负控锚点命中 $n 次（应为 1）: $pat"; fi
done
[ "${FAIL:-0}" = "0" ] && ok "[1d] 负控锚点唯一性"

# ── [2] 生成 + 三轨对拍 ─────────────────────────────────────
echo "[2] 生成（防漂移）+ 三轨对拍"
python3 "$HERE/gen.py" --work "$W" > "$W/logs/gen.txt" 2>&1
grc=$?
if [ "$grc" != "0" ]; then bad "[2a] gen.py 失败（rc=$grc）"; tail -3 "$W/logs/gen.txt" | sed 's/^/     /';
else ok "[2a] gen.py 生成 $(grep -vc '^#' "$W/CASES.tsv") 例"; fi
# 2b 防漂移：入库 CASES.tsv 必须与现生成**逐字节一致**（M282 缺陷 491 的教训：扫描面别漏在提交外）
if [ -f "$HERE/CASES.tsv" ]; then
  if cmp -s "$HERE/CASES.tsv" "$W/CASES.tsv"; then ok "[2b] 入库用例表与现生成一致"
  else bad "[2b] 用例表漂移（入库 ≠ 现生成）—— 请 cp \$W/CASES.tsv 入库"; fi
else bad "[2b] 缺入库 CASES.tsv"; fi

GATE_JOBS="${GATE_JOBS:-16}" timeout 1800 python3 "$HERE/three.py" --root "$ROOT" --work "$W" \
    --cases "$W/CASES.tsv" --model "$HERE/MODEL.tsv" --json "$W/three.json" > "$W/logs/three.txt" 2>&1
trc=$?
tail -8 "$W/logs/three.txt" | sed 's/^/     /'
if [ "$trc" = "0" ]; then ok "[2c] 三轨对拍全绿（A/B/C/D/E）"
else bad "[2c] 三轨对拍判红（rc=$trc）"; fi

# ── [2d] 非法侧：响亮 **且指名** ────────────────────────────
echo "[2d] 非法输入必须响亮且**指名**"
out="$(cd "$ROOT" && ./bootstrap/pxi N0 "$W/drv.px" 2>&1 || true)"
case "$out" in *"R2001"*"sv_cmp: 版本号非法"*) ok "[2d] sv_cmp 非法 ⇒ R2001 且指名（含 'sv_cmp: 版本号非法'）";;
  *) bad "[2d] sv_cmp 非法输入的报错不合契约：$(echo "$out" | head -1)";; esac
out2="$(cd "$ROOT" && ./bootstrap/pxi "Nr1" "$W/drv.px" 2>&1 || true)"
case "$out2" in "false"*) ok "[2d] 非法范围（^ 单独）⇒ satisfies 返回 false";;
  *) bad "[2d] 非法范围口径不符：$(echo "$out2" | head -1)";; esac

# ── [3] 负控 ────────────────────────────────────────────────
echo "[3] 负控（撤回修复 ⇒ 必须判红；判据自伤 ⇒ 红必须消失）"
NEG_ONLY='^(S3_0|S4_0|N0|Nr1|E5|D7)$'   # 每处修复各有一条**专属**落点用例
if [ "$NEG_SKIP" = "1" ]; then
  ok "[3] 负控已按 --neg-skip 跳过（CI 档；本地全量门会跑）"
else
  # 3A 撤回 5 处（逐处独立：每处撤回后该处对应的判据必须红）
  neg_case() {   # $1=描述  $2=sed 脚本  $3=目标文件
    restore_all
    sed -i "$2" "$3"
    # 前提自证（M292 的教训）：**先证明打桩生效**，否则「没判红」的原因是「打桩没生效」而非「负控无牙」
    if cmp -s "$3" "$SNAP/$(basename "$3")"; then
      bad "[3A] 撤回「$1」**打桩未生效**（源码未被改动）—— 前提不成立，不做负控结论"; return
    fi
    GATE_JOBS=4 timeout 300 python3 "$HERE/three.py" --root "$ROOT" --work "$W" \
        --cases "$W/CASES.tsv" --only "$NEG_ONLY" --need 6 --json "$W/logs/neg.json" \
        > "$W/logs/neg.txt" 2>&1
    nrc=$?
    if [ "$nrc" != "0" ]; then ok "[3A] 撤回「$1」⇒ 判红 ✓"
    else bad "[3A] 撤回「$1」后**没有**判红（负控无牙）"; fi
  }
  neg_case_py() {   # $1=描述  $2=目标文件：把 ^ 分支的 else 两行**忠实换回**旧文本
    restore_all
    python3 - "$2" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
old = '        else:\n            lo = "0.0.0"\n            hi = "1.0.0"'
new = '        else:\n            lo = "0.0." + str(pa)\n            hi = "0.0." + str(pa + 1)'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
    GATE_JOBS=4 timeout 300 python3 "$HERE/three.py" --root "$ROOT" --work "$W" \
        --cases "$W/CASES.tsv" --only "$NEG_ONLY" --need 6 --json "$W/logs/neg.json" \
        > "$W/logs/neg_520.txt" 2>&1
    nrc=$?
    if [ "$nrc" != "0" ]; then ok "[3A] 撤回「$1」⇒ 判红 ✓"
    else bad "[3A] 撤回「$1」后**没有**判红（负控无牙）"; fi
  }
  neg_case_py "缺陷 520（^ 缺段拼出 0.0.-1）" "$SEM"
  neg_case "缺陷 521（^ 单独非法）"   's|            return null|            return {"lo": "0.0.0", "hi": "999999999.0.0", "pre": ""}|' "$SEM"
  neg_case "缺陷 522（sv_cmp 静默）"  's|        panic("sv_cmp: 版本号非法: " + str(a))|        a = "0.0.0"|'  "$SEM"
  neg_case "缺陷 524（不转单引号）"   's|            out += "&#39;"|            out += ""|'                  "$HTM"
  neg_case "缺陷 523（nbsp 错成空格）" 's|    e.set("nbsp", "\\u{00a0}")|    e.set("nbsp", " ")|'            "$HTM"
  restore_all
  # 3B 判据自伤：把 [B] 比对恒真 ⇒ 上面那些红必须消失（证明红确实来自比对，而不是别处）
  cp -f "$HERE/three.py" "$W/logs/three.bak.py"
  python3 - "$HERE/three.py" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
s = s.replace("if kind == 'legal' and want != 'LOCK' and r['vm'][3] != want:",
              "if False and kind == 'legal' and want != 'LOCK' and r['vm'][3] != want:")
s = s.replace("if kind == 'neg' and want != 'LOCK' and r['vm'][1] != want:",
              "if False and kind == 'neg' and want != 'LOCK' and r['vm'][1] != want:")
open(p, 'w', encoding='utf-8').write(s)
PYEOF
  sed -i 's|            return null|            return {"lo": "0.0.0", "hi": "999999999.0.0", "pre": ""}|' "$SEM"
  GATE_JOBS=4 timeout 300 python3 "$HERE/three.py" --root "$ROOT" --work "$W" \
      --cases "$W/CASES.tsv" --only "$NEG_ONLY" --need 6 --json "$W/logs/negB.json" > "$W/logs/negB.txt" 2>&1
  brc=$?
  cp -f "$W/logs/three.bak.py" "$HERE/three.py"
  restore_all
  if [ "$brc" = "0" ]; then ok "[3B] 判据自伤 ⇒ 红消失（红的来源确为比对）"
  else bad "[3B] 判据自伤后仍判红（rc=$brc）—— 红的来源可疑"; fi
  # 3C 还原保真
  SUM1="$(sha256sum "$SEM" "$HTM" | awk '{print $1}' | tr '\n' ' ')"
  if [ "$SUM1" = "$SUM0" ]; then ok "[3C] 源码逐字节还原（sha256 一致）"
  else bad "[3C] 源码未还原：$SUM0 ≠ $SUM1"; fi
  # 3D three.py 也还原
  if cmp -s "$HERE/three.py" "$W/logs/three.bak.py"; then ok "[3D] three.py 逐字节还原"
  else bad "[3D] three.py 未还原"; fi
fi

# ── [4] registry 副本同步 ───────────────────────────────────
echo "[4] registry 分发副本与 stdlib 同步（同一语义只该有一份实现）"
cmp -s "$SEM" "$RSEM" && ok "[4a] registry/semver 与 stdlib/semver 一致" || bad "[4a] registry/semver 漂移"
cmp -s "$HTM" "$RHTM" && ok "[4b] registry/html 与 stdlib/html 一致" || bad "[4b] registry/html 漂移"

# ── [5] 覆盖边界 ────────────────────────────────────────────
echo "[5] 覆盖边界（如实登记）"
cat <<'EOF'
  · 面只取 **std.semver(9) + std.html(26)** 的导出；其它 25 个 stdlib 模块未覆盖
    （M300 台账 selfhost/stdlib_coverage.py 逐条可查，cap 只减不增）。
  · 合法侧期望值由 `ref.py` **独立**给出（Python 按 semver.org §9/§11 + node-semver 范围展开；
    转义/实体按 M302 定稿契约）—— **不由三轨自证**。
  · ⚠️ **arity 用例只比 (rc, R 码)，不比消息体**：M301（缺陷 518）已定稿
    「三轨 arity 文案**有意**不同」⇒ 本层**不覆盖 arity 文案的三轨一致性**。
  · 实体表**只解 6 个命名实体**（amp/lt/gt/quot/apos/nbsp），数字实体与其余命名实体
    **原样保留** —— 这是**文档化的能力缺口**（本门按契约判，缺口另行登记）。
  · 范围语法**不支持** `||` 复合、`>=1.2.3 <2.0.0` 空格复合、prerelease 限定 ——
    **文档化子集**（同上）。
  · 解释轨 `panic` 的前缀差异（缺陷 186 族）不在本门面内：只比 R 码 + 消息体。
EOF
ok "覆盖边界已登记"

echo
echo "══ 汇总：通过 $PASS / 失败 $FAIL ══"
[ "$FAIL" = "0" ] && { echo "M302-VERIFY-OK"; exit 0; } || { echo "M302-VERIFY-FAIL"; exit 1; }
