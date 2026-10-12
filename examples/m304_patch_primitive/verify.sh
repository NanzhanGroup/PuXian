#!/usr/bin/env bash
# ============================================================
# M304 门 · 「打桩原语」—— 把门的负控从 sed 迁移到零转义、失败响亮的原语
# ------------------------------------------------------------
# 立论：本仓 176 轮里最硬的一条纪律是「**会静默失效的门等于没有门**」（M274）。
#   而门的负控标准打桩法一直是 `sed -i 's|OLD|NEW|'`，它有两类**静默失效**：
#     ① 元字符陷阱：替换文本里的 & \ $ \1 与分隔符被 sed 解释（M302 缺陷 523 的 `\u`）
#     ② 找不到也 rc=0：负控被抽掉牙，门照样打 ✅（M285 的 A1 就是为抓它而建 —— 那是**事后扫描**）
#   ⇒ 现状：迁移前 28 门 / 52 处（旧口径）；**本轮迁 5 门 / 13 处**；
#     余量见 examples/m304_patch_primitive/migcheck.py 的 remaining_sed（去注释后按 sed -i 计数）。
#
# 判据七层（每层独立有牙）：
#   [1] 静态：原语的四条性质在源码里可证（字面量匹配 / 找不到 rc≠0 / count 强校验 / 原子写）
#   [2] 自证：原语自带 CLI 级自证（28 断言，含与 sed 的并排对照）
#   [3] 差分：原语 ⇄ wst-edit 2.1.0（**金标准**），语义类 + 落盘字节 + 权限三者全等
#   [4] 形状：sed 在这批用例上**确实会给出不同结果 / 确实会静默**（证明判据不是纸面推理）
#   [5] 负控 A/B/C + **D0–D4** 各自独立判红（A 找不到静默 · B count 不校验 · C 判据自伤 ·
#       D0 正控 / D1 登记了没迁移 / D2 迁移了没登记 / D3 规模锚点 / D4 行内字面量不误判）
#   [6] 迁移台账：已迁移门 ⇄ MIGRATED.tsv **双向**核对（漏登记 / 登记过期 都判红）
#   [7] 还原核对：与**本门开头快照**逐字节比对
#
# ⚠️ [3] 层依赖 wst-edit，只在东月机器上存在 ⇒ CI 上**响亮跳过**（不静默，M215 先例）；
#    可用 M304_REQUIRE_WEDIT=1 强制（本机全量门用）。
# [4] 层到处都有 sed ⇒ 它是 CI 上**永远有牙**的那一层。
#
# 覆盖边界见文件末。
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
  echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M304_W:-/tmp/m304_gate}"
NEG_SKIP="${NEG_SKIP:-0}"
WEDIT="${M304_WEDIT:-/data/app/ws/tools/wst-edit}"
PF="$ROOT/selfhost/patch_file.py"
DC="$HERE/diffcheck.py"
mkdir -p "$W"
# 门不给自己留产物：python 的 __pycache__ 会落进门目录（M302 曾留下过一个）
export PYTHONDONTWRITEBYTECODE=1
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1（got=$2 want=$3）"; fi; }
ge()  { if [ "$2" -ge "$3" ]; then ok "$1（$2 ≥ $3）"; else bad "$1（got=$2 want≥$3）"; fi; }

echo "══ M304：打桩原语（sed → 零转义 · 失败响亮）══"

# ── 负控快照/还原（逐字节）────────────────────────────────
python3 "$HERE/negctl.py" snapshot --root "$ROOT" --bak "$W/negbak" >/dev/null
SHA0="$(sha256sum "$PF" "$DC" | awk '{print $1}' | tr '\n' ' ')"
restore_all() { python3 "$HERE/negctl.py" restore --root "$ROOT" --bak "$W/negbak" >/dev/null 2>&1; }
trap 'restore_all' EXIT

# ── [1] 静态：四条性质 ────────────────────────────────────
echo "[1] 静态 —— 原语的四条性质（去注释后按代码行判）"
strip_c() { python3 - "$1" <<'PY'
import re,sys
src=open(sys.argv[1],encoding='utf-8').read()
src=re.sub(r'"""[\s\S]*?"""','',src)
src=re.sub(r'(?m)^\s*#.*$','',src)
print(src)
PY
}
BODY="$(strip_c "$PF")"
chk "P1 字面量匹配（不是正则）" "$(printf '%s' "$BODY" | grep -c 'text\.count(old)')" "1"
chk "P2 找不到 ⇒ rc≠0（RC_NOTFOUND 存在且被判死）" \
    "$(printf '%s' "$BODY" | grep -c 'RC_NOTFOUND')" "2"
chk "P3 count 强校验（actual != want 即判死）" \
    "$(printf '%s' "$BODY" | grep -c 'actual != want')" "1"
chk "P4 原子写（临时文件 + os.replace）" \
    "$(printf '%s' "$BODY" | grep -c 'os\.replace(tmp, path)')" "1"
chk "P5 权限/属主保留" "$(printf '%s' "$BODY" | grep -c 'os\.chmod(tmp')" "1"
# 逆向判据：源码里**不得**出现正则/分隔符式替换（那正是 sed 的形状）
chk "P6 逆向：无正则替换（re.sub）" "$(printf '%s' "$BODY" | grep -c 're\.sub')" "0"

# ── [2] 自证 ─────────────────────────────────────────────
echo "[2] 自证 —— 原语 CLI 级 28 断言（含与 sed 并排对照）"
OUT="$(PF="$PF" W="$W/selftest" bash "$HERE/selftest.sh" 2>&1)"; rc=$?
LINE="$(printf '%s\n' "$OUT" | grep -E '^SELFTEST ' | tail -1)"
chk "自证 rc=0" "$rc" "0"
case "$LINE" in
  'SELFTEST OK · 通过 28 / 失败 0') ok "自证全部通过（$LINE）" ;;
  *) bad "自证未全绿（$LINE）" ;;
esac

# ── [3] 差分（金标准）─────────────────────────────────────
echo "[3] 差分 —— patch_file ⇄ wst-edit 2.1.0（语义类 + 字节 + 权限）"
NCASE="$(python3 -c "import sys;sys.path.insert(0,'$HERE');from cases import CASES;print(len(CASES))")"
NSED="$(python3 -c "import sys;sys.path.insert(0,'$HERE');from cases import SED_APPLICABLE;print(len(SED_APPLICABLE))")"
ge "规模锚点：用例表条数（防用例表被挪空）" "$NCASE" "12"
if [ -x "$WEDIT" ]; then
  D="$(timeout 180 python3 "$DC" --patch "$PF" --wedit "$WEDIT" --work "$W/diff" 2>&1)"; rc=$?
  DL="$(printf '%s\n' "$D" | grep -E '^DIFF  n=' | tail -1)"
  chk "差分层 rc=0" "$rc" "0"
  chk "差分：12/12 全等" "$DL" "DIFF  n=$NCASE pass=$NCASE fail=0"
  echo "     $DL"
else
  if [ "${M304_REQUIRE_WEDIT:-0}" = "1" ]; then
    bad "本机无 wst-edit（$WEDIT）而 M304_REQUIRE_WEDIT=1 ⇒ 差分层必须执行"
  else
    echo "  ⏭ DIFF-SKIPPED：本机无 wst-edit（$WEDIT）—— 差分层未执行（CI 常态）"
  fi
  D="$(timeout 180 python3 "$DC" --patch "$PF" --wedit - --work "$W/diff" 2>&1)"
fi

# ── [4] 形状层：sed 确实会错 / 确实会静默 ─────────────────
echo "[4] 形状 —— sed 在这批形状上的实测（判据不是纸面推理）"
SDL="$(printf '%s\n' "$D" | grep -E '^SED   applicable=' | tail -1)"
chk "sed 对照覆盖条数（= 单处替换用例数）" "$(printf '%s' "$SDL" | sed -n 's/.*applicable=\([0-9]*\).*/\1/p')" "$NSED"
ge "sed 给出**不同结果**的用例数" "$(printf '%s' "$SDL" | sed -n 's/.*differ=\([0-9]*\).*/\1/p')" "6"
ge "sed **无命中却 rc=0**（静默失效）的用例数" "$(printf '%s' "$SDL" | sed -n 's/.*silent=\([0-9]*\).*/\1/p')" "1"
echo "     $SDL"

# ── [5] 负控（--neg 档）───────────────────────────────────
echo "[5] 负控（各自独立判红）"
if [ "$NEG_SKIP" = "1" ]; then
  echo "  ⏭ NEG-SKIP：CI 档跳过负控（负控要重建 + 重跑，成本高）"
else
  neg_diff_fail() {   # 跑差分层，返回 DIFF 的 fail 计数
    timeout 180 python3 "$DC" --patch "$PF" --wedit "$WEDIT" --work "$W/diff_neg" 2>&1 \
      | grep -E '^DIFF  n=' | tail -1 | sed -n 's/.*fail=\([0-9]*\).*/\1/p'
  }
  if [ ! -x "$WEDIT" ]; then
    echo "  ⏭ 负控 A/B 需要 wst-edit（差分层）；本机无 ⇒ 只跑负控 C 的等价检查"
  else
    # A：找不到静默成功 ⇒ C7 必须判出差异
    python3 "$HERE/negctl.py" A --root "$ROOT" --bak "$W/negbak" >/dev/null || bad "负控 A 注入失败"
    fa="$(neg_diff_fail)"
    ge "负控 A（not-found 静默）⇒ 差分必红" "${fa:-0}" "1"
    restore_all
    # B：count 不校验 ⇒ C8 必须判出差异
    python3 "$HERE/negctl.py" B --root "$ROOT" --bak "$W/negbak" >/dev/null || bad "负控 B 注入失败"
    fb="$(neg_diff_fail)"
    ge "负控 B（count 不校验）⇒ 差分必红" "${fb:-0}" "1"
    restore_all
    # C：判据自伤（A + 比较恒真）⇒ A 的红必须消失
    python3 "$HERE/negctl.py" A --root "$ROOT" --bak "$W/negbak" >/dev/null
    python3 "$HERE/negctl.py" C --root "$ROOT" --bak "$W/negbak" >/dev/null
    fc="$(neg_diff_fail)"
    chk "负控 C（判据自伤）⇒ A 的红消失（证明红来自判据）" "${fc:-x}" "0"
    restore_all
  fi

  # D：迁移台账层（**纯夹具** · 不需要 wst-edit）—— 双向都要有牙
  #    为什么用夹具而不是改真仓库的门：本层的被测对象是「台账 ⇄ 门」的一致性，
  #    夹具能精确构造「登记了没迁移」「迁移了没登记」两种形态，且**不动工作树**。
  FIX="$W/migfix"; rm -rf "$FIX"; mkdir -p "$FIX/examples/mok"
  printf '#!/usr/bin/env bash\n. ../../selfhost/gate_patch.sh\ngp_patch f a b\n' > "$FIX/examples/mok/verify.sh"
  printf 'mok\t0\tok\n' > "$FIX/led.tsv"
  python3 "$HERE/migcheck.py" --root "$FIX" --ledger "$FIX/led.tsv" --min-gates 1 >/dev/null 2>&1
  chk "负控 D0（正控：台账 ⇄ 门一致）⇒ rc=0" "$?" "0"
  mkdir -p "$FIX/examples/mbad"
  printf '#!/usr/bin/env bash\nsed -i s/a/b/ f\n' > "$FIX/examples/mbad/verify.sh"
  printf 'mok\t0\tok\nmbad\t0\tx\n' > "$FIX/led.tsv"
  python3 "$HERE/migcheck.py" --root "$FIX" --ledger "$FIX/led.tsv" --min-gates 1 >/dev/null 2>&1
  chk "负控 D1（台账登记了未迁移的门）⇒ 必红" "$?" "1"
  mkdir -p "$FIX/examples/mok2"
  printf '#!/usr/bin/env bash\n. ../../selfhost/gate_patch.sh\ngp_patch f a b\n' > "$FIX/examples/mok2/verify.sh"
  printf 'mok\t0\tok\n' > "$FIX/led.tsv"
  python3 "$HERE/migcheck.py" --root "$FIX" --ledger "$FIX/led.tsv" --min-gates 1 >/dev/null 2>&1
  chk "负控 D2（已迁移但漏登记）⇒ 必红" "$?" "1"
  python3 "$HERE/migcheck.py" --root "$FIX" --ledger "$FIX/led.tsv" --min-gates 99 >/dev/null 2>&1
  chk "负控 D3（扫描数 < 规模锚点）⇒ 必红" "$?" "1"
  # D4：**判据精确性** —— 门里「生成夹具的行」含 gp_patch 字面量，但**不在命令位置** ⇒ 不得误判「已迁移」
  #     （M304 首跑实测：按「出现即算」的判据把**本门自己**判成「漏登记」= 假阳）
  #     ⚠️ 夹具必须**另起目录**：D1/D2 已往 $FIX 里造了 mbad / mok2，留着会把 D4 变成假红
  #        （首跑实测：mok2 有 gp_patch 却不在 D4 的台账里 ⇒ C 违例 ⇒ got=1 而 want=0）
  FIX4="$W/migfix4"; rm -rf "$FIX4"; mkdir -p "$FIX4/examples/mok" "$FIX4/examples/mgen"
  printf '#!/usr/bin/env bash\n. ../../selfhost/gate_patch.sh\ngp_patch f a b\n' > "$FIX4/examples/mok/verify.sh"
  printf '#!/usr/bin/env bash\necho "gp_patch f a b" > out\n' > "$FIX4/examples/mgen/verify.sh"
  printf 'mok\t0\tok\n' > "$FIX4/led.tsv"
  python3 "$HERE/migcheck.py" --root "$FIX4" --ledger "$FIX4/led.tsv" --min-gates 1 >/dev/null 2>&1
  chk "负控 D4（行内的 gp_patch 字面量）⇒ 不得误判为已迁移" "$?" "0"
fi

# ── 收尾：还原核对 ────────────────────────────────────────
# ── [6] 迁移台账：sed 打桩 → gp_patch（M304 起 · **逐门推进**）────
# 台账是**双向**的（migcheck.py 核对）：漏登记 / 登记过期 都判红。
# 为什么要有这层：[1]-[5] 只证明「原语是对的」，本层证明「它**真被用上了**」——
#   否则工具写成艺术品，而门的负控还在用 sed，等于没改（M304 的立论）。
echo "[6] 迁移台账 —— 已迁移门 ⇄ MIGRATED.tsv（双向核对 + 规模锚点）"
MIGL="$HERE/MIGRATED.tsv"
MIG="$(python3 "$HERE/migcheck.py" --root "$ROOT" --ledger "$MIGL" 2>&1)"; rcm=$?
chk "迁移台账一致（A/B/C 双向 + D 规模锚点）" "$rcm" "0"
printf '%s\n' "$MIG" | while IFS= read -r _l; do printf '     %s\n' "$_l"; done

# ── 收尾：还原核对 ────────────────────────────────────────
echo "[7] 还原核对"
RF="$(python3 "$HERE/negctl.py" restore --root "$ROOT" --bak "$W/negbak" 2>&1 | tail -1)"
case "$RF" in *RESTORE-OK*) ok "$RF" ;; *) bad "$RF" ;; esac
# ⚠️ 判据必须与**本门开头取下的快照**比，不能拿 git 比 HEAD ——
#    工作树本来就是未提交状态（本轮是新文件），比 HEAD 会恒判「未还原」（M214 的教训）。
SHA1="$(sha256sum "$PF" "$DC" | awk '{print $1}' | tr '\n' ' ')"
chk "还原后两件目标 == 本门开头快照" "$SHA1" "$SHA0"

echo
echo "════════════ M304 verify：PASS=$PASS FAIL=$FAIL ════════════"
[ "$FAIL" -eq 0 ] && echo "M304-VERIFY-OK"
[ "$FAIL" -eq 0 ]

# ── 覆盖边界（如实）──────────────────────────────────────
# ① 差分层（[3]）只在**有 wst-edit 的机器**上执行 —— CI 上为 SKIP；要点是 [4] 层
#    （sed 形状）在 CI 上永远有牙，两者互补而非替代。
# ② 本轮**只换 5 个门**做样板（runtime_*.c 那几个最危险的）；全量 28 门 / 52 处
#    在后续轮次迁移 —— 迁移进度由 M285/M286 守卫的计数给出，不由本门断言。
# ③ 原语只覆盖「文本替换」这一族。sed 的**行操作**（按行号删/插/替换）不在本门面内
#    （wst-edit 有 delete_line/insert/replace_line，本原语暂无 —— 故那类站点不迁移）。
# ④ 原语不做 UTF-16/GBK 等非 UTF-8 文件的落盘（会以 rc=4 响亮拒绝，不静默转换）。
