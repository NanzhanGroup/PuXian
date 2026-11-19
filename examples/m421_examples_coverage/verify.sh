#!/bin/bash
# ============================================================
# m421_examples_coverage —— examples 覆盖账：谁有断言、谁只是示例、谁在裸奔
# ------------------------------------------------------------
# 缺陷 846（M421 立项 · 直接由缺陷 845 逼出来）：
#   examples/m28_route.px 是生产级 Web 路由示例，常规构建 3/3 必崩 SEGV。
#   而 292/294 门全绿——因为它**既没有 verify.sh、也没注册进 registry**，
#   全量门天然看不见它。同类风险面从未被量化：到底有多少示例是"跑了也没人验"。
#
# 判据（唯一实现在 selfhost/examples_coverage.sh）：
#   单元 = examples 下每个顶层 *.px 与每个含 *.px 的目录
#   COVERED  被已注册的 run 入口或其直属 verify.sh 引用
#   ORPHAN   有 verify.sh 但不在任何执行路径上
#   BLACKBOX 没有 verify.sh
#   豁免账 selfhost/examples_exempt.tsv：未登记 / 过期 / kind 不符 / 已覆盖仍挂（STALE）⇒ 红
#
# 门结构（每条红都必须有牙）：
#   [1] 夹具自测 10 条（正控 + 负控 + 前提自证）
#   [2] 真仓对账（账面数字 + 前提自证：registry 可读 / token 非空 / 扫描面非空）
#   [3] 真仓镜像 4 条扰动（新建黑盒 ⇒ 红；登记 ⇒ 绿；过期 ⇒ 红；已覆盖仍挂 ⇒ STALE 红）
#   [4] 环境守卫（删 registry / 空 examples ⇒ rc=3，绝不降级成绿）
# ============================================================
set -uo pipefail

. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }

HERE=$(cd "$(dirname "$0")" && pwd)
RT=$(cd "$HERE/../.." && pwd)
IMPL="$RT/selfhost/examples_coverage.sh"
pass=0; fail=0
ok()  { pass=$((pass+1)); echo "  ✅ $*"; }
bad() { fail=$((fail+1)); echo "  ❌ $*"; }

[ -f "$IMPL" ] || { echo "❌ 判据实现不存在：$IMPL"; exit 3; }

# 临时根放仓库同盘（硬链接镜像用），退出清理
GA=$(mktemp -d "${RT%/puxian}/m421_XXXXXX") || { echo "❌ mktemp 失败"; exit 3; }
trap 'rm -rf "$GA"' EXIT

# ── [1] 夹具自测 ────────────────────────────────────────────
F="$GA/fixture"
mkdir -p "$F/examples/a_good" "$F/examples/b_orphan" "$F/selfhost"
cp "$IMPL" "$F/selfhost/examples_coverage.sh"
printf 'def main():\n    print("a")\n' > "$F/examples/a_good/good.px"
printf '#!/bin/bash\nexit 0\n'          > "$F/examples/a_good/verify.sh"
printf 'def main():\n    print("b")\n' > "$F/examples/b_orphan/foo.px"
printf '#!/bin/bash\nexit 0\n'          > "$F/examples/b_orphan/verify.sh"
printf 'def main():\n    print("c")\n' > "$F/examples/c_black.px"
printf 'def main():\n    WebServer().serve(8080)\n' > "$F/examples/d_high.px"
cat > "$F/selfhost/gates.registry.sh" <<'EOF'
run a_good bash examples/a_good/verify.sh
EOF
printf '# empty\n' > "$F/selfhost/examples_exempt.tsv"
runf() { bash "$F/selfhost/examples_coverage.sh" --root "$F" "$@" 2>&1; }

OUT=$(runf --list)
cls() { echo "$OUT" | awk -F'\t' -v u="$1" '$1==u{print $2}'; }
[ "$(cls a_good)"   = COVERED ]  && ok "[1/S1a] 已注册+自带 verify ⇒ COVERED"   || bad "[1/S1a] a_good 分类=$(cls a_good)"
[ "$(cls b_orphan)" = ORPHAN ]   && ok "[1/S1b] 有 verify 但不可达 ⇒ ORPHAN"    || bad "[1/S1b] b_orphan 分类=$(cls b_orphan)"
[ "$(cls c_black.px)" = BLACKBOX ] && ok "[1/S1c] 无 verify ⇒ BLACKBOX"          || bad "[1/S1c] c_black 分类=$(cls c_black.px)"
OUT=$(runf --list)
hd=$(echo "$OUT" | awk -F'\t' '$3=="[high]"{print $1}' | tr '\n' ' ')
[ "$hd" = "d_high.px " ] && ok "[1/S2] high 只标出含 serve() 的 d_high.px" || bad "[1/S2] high 判定=$hd"

set +e; OUT=$(runf); rc=$?
n_un=$(echo "$OUT" | grep -oE 'unlisted=[0-9]+' | head -1 | cut -d= -f2)
[ "$rc" = 1 ] && [ "$n_un" = 3 ] && ok "[1/S3] 3 条未登记 ⇒ rc=1 unlisted=3" \
  || bad "[1/S3] rc=$rc unlisted=${n_un:-?}（期望 1/3）"
# S3 前提自证：三条具体名都在输出里（证明不是空跑）
n3=$(echo "$OUT" | grep -cE 'UNLISTED   (a_good|b_orphan|c_black\.px|d_high\.px)' 2>/dev/null || true)
[ "${n3:-0}" = 3 ] && ok "[1/S3b] 前提自证：三条违例都点名（扰动真发生）" || bad "[1/S3b] 违例点名数=${n3:-0}"

printf 'b_orphan\torphan\t东月\t2099-01-01\tr\nc_black.px\tblackbox\t东月\t2099-01-01\tr\nd_high.px\tblackbox\t东月\t2099-01-01\tr\n' \
  > "$F/selfhost/examples_exempt.tsv"
set +e; OUT=$(runf); rc=$?
[ "$rc" = 0 ] && [ "$(echo "$OUT" | grep -c 'EXAMPLES-COVERAGE=OK')" = 1 ] \
  && ok "[1/S4] 三条登记 kind 正确且未过期 ⇒ rc=0 OK" || bad "[1/S4] rc=$rc"

sed -i 's/^c_black\.px\tblackbox\t东月\t2099-01-01/c_black.px\tblackbox\t东月\t2020-01-01/' "$F/selfhost/examples_exempt.tsv"
set +e; OUT=$(runf); rc=$?
{ [ "$rc" = 1 ] && grep -q 'EXPIRED    c_black.px' <<< "$OUT"; } \
  && ok "[1/S5] 到期即红（EXPIRED）" || bad "[1/S5] rc=$rc 未报 EXPIRED"
printf 'b_orphan\torphan\t东月\t2099-01-01\tr\nc_black.px\tblackbox\t东月\t2099-01-01\tr\nd_high.px\tblackbox\t东月\t2099-01-01\tr\n' \
  > "$F/selfhost/examples_exempt.tsv"

sed -i 's/^b_orphan\torphan/b_orphan\tblackbox/' "$F/selfhost/examples_exempt.tsv"
set +e; OUT=$(runf); rc=$?
{ [ "$rc" = 1 ] && grep -q 'KIND-MISMATCH b_orphan' <<< "$OUT"; } \
  && ok "[1/S6] kind 与实际不符 ⇒ 红" || bad "[1/S6] rc=$rc 未报 KIND-MISMATCH"
sed -i 's/^b_orphan\tblackbox/b_orphan\torphan/' "$F/selfhost/examples_exempt.tsv"

printf 'a_good\tblackbox\t东月\t2099-01-01\tr\n' >> "$F/selfhost/examples_exempt.tsv"
set +e; OUT=$(runf); rc=$?
{ [ "$rc" = 1 ] && grep -q 'STALE-EXEMPT a_good' <<< "$OUT"; } \
  && ok "[1/S7] 已覆盖仍挂豁免 ⇒ STALE-EXEMPT 红（防烂账）" || bad "[1/S7] rc=$rc 未报 STALE"
sed -i '$ d' "$F/selfhost/examples_exempt.tsv"

mv "$F/selfhost/gates.registry.sh" "$GA/reg.bak"
set +e; OUT=$(runf); rc=$?
{ [ "$rc" = 3 ] && ! grep -q 'EXAMPLES-COVERAGE=OK' <<< "$OUT"; } \
  && ok "[1/S8] registry 缺失 ⇒ rc=3 且不打印 OK（不降级）" || bad "[1/S8] rc=$rc"
mv "$GA/reg.bak" "$F/selfhost/gates.registry.sh"

G2="$GA/empty"; mkdir -p "$G2/examples" "$G2/selfhost"
cp "$IMPL" "$G2/selfhost/examples_coverage.sh"; printf 'run x true\n' > "$G2/selfhost/gates.registry.sh"
set +e; OUT=$(bash "$G2/selfhost/examples_coverage.sh" --root "$G2" 2>&1); rc=$?
{ [ "$rc" = 3 ] && grep -q 'EXAMPLES-COVERAGE=ENV' <<< "$OUT"; } \
  && ok "[1/S9] 扫描面为 0 ⇒ rc=3（不把空当绿）" || bad "[1/S9] rc=$rc"

# ── [2] 真仓对账 ────────────────────────────────────────────
echo "── [2] 真仓（$RT） ──"
set +e; OUT=$(bash "$IMPL" 2>&1); rc=$?
echo "$OUT" | grep -E '^账面|^前提自证' | sed 's/^/      /'
echo "$OUT" | grep -E '^EXAMPLES-COVERAGE=' | sed 's/^/      /'
nu=$(echo "$OUT" | grep -oE 'units=[0-9]+' | head -1 | cut -d= -f2)
tk=$(echo "$OUT" | grep -oE '证据 token=[0-9]+' | cut -d= -f2)
[ "${nu:-0}" -ge 300 ] && ok "[2/S10] 前提自证：真仓单元数=$nu（扫描面非空）" || bad "[2/S10] 单元数异常=${nu:-0}"
[ "${tk:-0}" -ge 50 ] && ok "[2/S11] 前提自证：覆盖证据 token=$tk 条（判据不是空转）" || bad "[2/S11] token 数=${tk:-0}"
[ "$rc" = 0 ] && ok "[2/S12] 真仓账面全登记（豁免未过期、无 STALE、无未登记）" || bad "[2/S12] 真仓 rc=$rc（有未登记/过期/不符，见上）"

# ── [3] 真仓镜像扰动（零污染原仓） ───────────────────────────
echo "── [3] 真仓镜像扰动 ──"
M="$GA/mirror"; mkdir -p "$M/selfhost"
cp -a "$RT/examples" "$M/examples" 2>/dev/null || cp -al "$RT/examples" "$M/examples" || { echo "❌ 镜像失败"; exit 3; }
cp "$RT/selfhost/gates.registry.sh" "$RT/selfhost/examples_exempt.tsv" "$IMPL" "$M/selfhost/"
base=$(bash "$M/selfhost/examples_coverage.sh" --root "$M" 2>&1 | grep -c 'EXAMPLES-COVERAGE=OK')
[ "$base" = 1 ] || { echo "❌ 镜像基线与真仓不一致（镜像不可信，后续扰动结论无意义）"; exit 3; }

mkdir -p "$M/examples/zz_probe_m421"
printf 'def main():\n    WebServer().serve(9911)\n' > "$M/examples/zz_probe_m421/foo.px"
set +e; OUT=$(bash "$M/selfhost/examples_coverage.sh" --root "$M" 2>&1); rc=$?
{ [ "$rc" = 1 ] && grep -q 'UNLISTED   zz_probe_m421' <<< "$OUT"; } \
  && ok "[3/N1] 新建无断言示例 ⇒ 判红（845 形态会被这里拦住）" || bad "[3/N1] rc=$rc 未拦住新增黑盒"
h2=$(echo "$OUT" | grep -oE 'high=[0-9]+' | cut -d= -f2)
grep -q 'UNLISTED   zz_probe_m421' <<< "$OUT" && [ "${h2:-0}" -gt 134 ] \
  && ok "[3/N1b] 前提自证：high 计数 $h2 >134（新单元真被识别为高危）" || bad "[3/N1b] high=${h2:-?}"

printf 'zz_probe_m421\tblackbox\t东月\t2099-01-01\tM421 负控\n' >> "$M/selfhost/examples_exempt.tsv"
set +e; OUT=$(bash "$M/selfhost/examples_coverage.sh" --root "$M" 2>&1); rc=$?
[ "$rc" = 0 ] && ok "[3/N2] 登记（kind 正确、未过期）⇒ 转绿" || bad "[3/N2] rc=$rc 登记后仍红"

sed -i 's/^zz_probe_m421\tblackbox\t东月\t2099-01-01/zz_probe_m421\tblackbox\t东月\t2020-01-01/' "$M/selfhost/examples_exempt.tsv"
set +e; OUT=$(bash "$M/selfhost/examples_coverage.sh" --root "$M" 2>&1); rc=$?
{ [ "$rc" = 1 ] && grep -q 'EXPIRED    zz_probe_m421' <<< "$OUT"; } \
  && ok "[3/N3] 豁免到期 ⇒ 红（欠账不能永久挂着）" || bad "[3/N3] rc=$rc"
sed -i '$ d' "$M/selfhost/examples_exempt.tsv"

cov=$(bash "$M/selfhost/examples_coverage.sh" --root "$M" --list 2>/dev/null | awk -F'\t' '$2=="COVERED"{print $1; exit}')
printf '%s\tblackbox\t东月\t2099-01-01\tM421 负控\n' "$cov" >> "$M/selfhost/examples_exempt.tsv"
set +e; OUT=$(bash "$M/selfhost/examples_coverage.sh" --root "$M" 2>&1); rc=$?
{ [ "$rc" = 1 ] && grep -q "STALE-EXEMPT $cov" <<< "$OUT"; } \
  && ok "[3/N4] 已覆盖的 $cov 仍挂豁免 ⇒ STALE 红" || bad "[3/N4] rc=$rc 未报 STALE（$cov）"

echo "M421-EXAMPLES-COVERAGE pass=$pass fail=$fail"
[ "$fail" -eq 0 ] && exit 0
exit 1
