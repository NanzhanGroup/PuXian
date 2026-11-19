#!/usr/bin/env bash
# ============================================================
# selfhost/check_pxexec_portable.sh —— px_exec 的**可移植性**（M423 · 缺陷 859）
# ------------------------------------------------------------
# 为什么有它（2026-11-19 CI run 44571916900 实测）：
#   examples/m102_s2 在 CI 上判红：「协程 px_exec 结果 3/3 对拍 slow-ok」失败。
#   真因不是协程，而是 px_exec **子进程的解释器定位**：
#     旧 px_px_bin() 只看 PX_BIN / PATH 中的 "px"。CI 只 `px build`（不安装），
#     PATH 里没有 px ⇒ execlp 失败 ⇒ px_exec **静默返回 ""**（分不清「脚本没输出」
#     与「解释器不存在」）⇒ 断言失败。本机装了 px 所以恒绿 —— 典型的「本机绿、CI 红」。
#   同族：`px --worker` 这个子命令**从来不存在**（tools/px 只认 build/run，pxi 只认脚本，
#     runtime 无 worker 入口）⇒ M25 的「进程池」从未生效：每次 px_exec 先派生 4 个必然
#     失败的 worker（每个打 2 行「未知子命令」到 stderr），再退化 fork+exec。
#
# 判据：
#   S1 源码形状：px_px_bin 必须有**自身路径推导**（/proc/self/exe → tools/px、bootstrap/pxi）
#   S2 源码形状：解释器找不到 ⇒ px_error（**响亮**，不得静默返回空串）
#   S3 源码形状：g_px_pool_size 默认 0（假池关闭，不再噪鸣）
#   P1 正控（无 px 的 PATH）：probe 放在「祖先目录里有 tools/px」的位置 ⇒ 必须成功且
#        stderr 不含「未知子命令」
#   N1 负控（祖先无 tools/px、PATH 无 px）⇒ **必须响亮报错**（rc≠0 + R1002），不得静默
#   N2 负控（PX_BIN 指向不存在）⇒ 必须 rc≠0 且 stderr 含「px_exec 执行失败」
#   前提自证：farm 里真的没有 px；fakeroot 的祖先里真的有 tools/px；扫描面非空
#
# 用法：check_pxexec_portable.sh [--self-test]
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立（不降级成 0）
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
TMP="${TMPDIR:-/tmp}/m423_pxexec.$$"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

if [ ! -x "$PX" ] || [ ! -f "$RT" ]; then
  echo "PXEXEC-PORTABLE rc=3（环境缺失：$PX 或 $RT）"; exit 3
fi

mkdir -p "$TMP/src"

# ── 探针：写一个内层脚本 → px_exec 它 → 断言有输出 ─────────────
cat > "$TMP/src/probe.px" <<'PEOF'
write_file("/tmp/m423_inner.px", "print('m423-ok')\n")
var r = px_exec("/tmp/m423_inner.px")
print("M423-PXEXEC len=" + str(len(r)))
assert(len(r) > 0, "px_exec 必须有输出（解释器可定位）")
print("M423-PXEXEC-OK")
PEOF

echo "── 构建探针（一次；px build 走 .rtcache）"
if ! ( cd "$TMP/src" && PX_BIN="$PX" "$PX" build probe.px > "$TMP/build.log" 2>&1 ); then
  echo "PXEXEC-PORTABLE rc=3（探针构建失败，见下）"; tail -5 "$TMP/build.log"; exit 3
fi
BIN="$TMP/src/build/probe"
[ -x "$BIN" ] || { echo "PXEXEC-PORTABLE rc=3（探针产物缺失）"; exit 3; }

# ── 无 px 的 PATH（保留 bash/coreutils，模拟 CI runner）──────────
FARM="$TMP/nopx"; mkdir -p "$FARM"
for d in /usr/local/bin /usr/bin /bin; do
  [ -d "$d" ] || continue
  for f in "$d"/*; do
    b=${f##*/}
    [ "$b" = "px" ] && continue
    [ -e "$FARM/$b" ] || ln -s "$f" "$FARM/$b" 2>/dev/null
  done
done
[ -x "$FARM/bash" ] || { echo "PXEXEC-PORTABLE rc=3（farm 里没有 bash）"; exit 3; }

# ── S1/S2/S3 源码形状 ─────────────────────────────────────────
grep -q 'proc/self/exe' "$RT" && grep -q 'bootstrap/pxi' "$RT" \
  && ok "S1 px_px_bin 有自身路径推导（/proc/self/exe → tools/px · bootstrap/pxi）" \
  || bad "S1 px_px_bin 缺自身路径推导（CI 无 px 时会静默失败）"
grep -q 'px_exec 找不到 px 解释器' "$RT" \
  && ok "S2 解释器找不到 ⇒ px_error（响亮报错）" \
  || bad "S2 找不到解释器仍然静默（无 px_error）"
awk '/static int g_px_pool_size = 0;/' "$RT" | grep -q . \
  && ok "S3 假池默认关闭（g_px_pool_size = 0）" \
  || bad "S3 假池仍在默认开启（每次 px_exec 派生 4 个失败 worker）"
# 前提自证：静态判据确实读到了被测文件
[ -s "$RT" ] && echo "   · 前提自证：扫描面 runtime.c=$(wc -c < "$RT") 字节" || bad "前提自证：runtime.c 为空"

# ── P1 正控：祖先目录里有 tools/px ⇒ 无 px 的 PATH 也必须成功 ──
mkdir -p "$TMP/fakeroot/tools" "$TMP/fakeroot/examples/x/build"
ln -sf "$PX" "$TMP/fakeroot/tools/px"
cp "$BIN" "$TMP/fakeroot/examples/x/build/probe"
out=$( env -u PX_BIN PATH="$FARM" "$TMP/fakeroot/examples/x/build/probe" 2>"$TMP/p1.err" )
rc=$?
if [ $rc -eq 0 ] && printf '%s' "$out" | grep -q 'M423-PXEXEC-OK'; then
  ok "P1 无 px 的 PATH：自身路径推导生效（probe 祖先里找到 tools/px）"
else
  bad "P1 无 px 的 PATH：px_exec 仍然失败（rc=$rc）"; sed -n '1,5p' "$TMP/p1.err"
fi
if grep -q '未知子命令' "$TMP/p1.err"; then
  bad "P1b stderr 仍有「未知子命令」（假池没关）"
else
  ok "P1b stderr 无假池噪鸣"
fi
[ ! -e "$FARM/px" ] && ok "前提自证：farm 里确实没有 px" || bad "前提自证：farm 里混进了 px（正控无效）"

# ── N1 负控：祖先无 tools/px + PATH 无 px ⇒ 必须**响亮** ────────
mkdir -p "$TMP/iso/build"; cp "$BIN" "$TMP/iso/build/probe"
env -u PX_BIN PATH="$FARM" "$TMP/iso/build/probe" >"$TMP/n1.out" 2>"$TMP/n1.err"
rc=$?
if [ $rc -ne 0 ] && grep -q 'R1002: px_exec 找不到 px 解释器' "$TMP/n1.err"; then
  ok "N1 找不到解释器 ⇒ rc≠0 + R1002 响亮报错（不静默返回空串）"
else
  bad "N1 找不到解释器却没说清（rc=$rc）"; sed -n '1,5p' "$TMP/n1.err"
fi

# ── N2 负控：PX_BIN 指向不存在 ⇒ 不得静默成功 ──────────────────
env PX_BIN=/nonexistent/px PATH="$FARM" "$TMP/fakeroot/examples/x/build/probe" >"$TMP/n2.out" 2>"$TMP/n2.err"
rc=$?
if [ $rc -ne 0 ] && grep -q 'px_exec 执行失败' "$TMP/n2.err"; then
  ok "N2 PX_BIN 指向不存在 ⇒ rc≠0 + 「px_exec 执行失败」"
else
  bad "N2 PX_BIN 无效时仍然静默（rc=$rc）"; sed -n '1,5p' "$TMP/n2.err"
fi

echo "======================================"
echo "PXEXEC-PORTABLE pass=$PASS fail=$FAIL"
[ "$FAIL" -eq 0 ] || { echo "PXEXEC-PORTABLE=FAIL rc=1"; exit 1; }
echo "PXEXEC-PORTABLE=OK"
exit 0
