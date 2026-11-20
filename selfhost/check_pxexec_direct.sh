#!/usr/bin/env bash
# ============================================================
# selfhost/check_pxexec_direct.sh —— px_exec 的**每次调用固定开销**（M424 · 缺陷 864）
# ------------------------------------------------------------
# 为什么有它（2026-11-20 实测，隔离克隆 /data/gatevm/m424recon @ 36a0581）：
#   M423 把「假池」关掉之后，每次 px_exec 都是 fork + exec 一个 **1485 行 bash 包装器**
#   （tools/px）：`tools/px --version` 786 syscalls / 7 execve = 27ms，
#   `tools/px run x.px` 41ms；而同一个发行版里的 bootstrap/pxi 直跑只要 12ms 甚至 9ms。
#   ⇒ 服务端每个 px_exec 白付 30ms 左右，而 pxi 就在旁边（`px run` 内部本来也是 exec pxi）。
#
# 判据（每条都要能判红）：
#   S1 源码形状：px_interp_bin 存在，且**仓库/安装树自身的 bootstrap/pxi 优先于 PATH 的 px**
#   S2 源码形状：直连分支真的 exec pxi（execlp(pxbin, "pxi", path, …)），CLI 分支保留 run
#   S3 源码形状：sibling 推导必须 realpath（/usr/bin/px 是 symlink，不解引用就找不到 pxi）
#   S4 源码形状：rc=2（解释器启动失败）⇒ px_error 响亮，不再静默返回 ""
#   P1 正控：px_exec 单次中位 ≤ 20ms（实测直连 9.2ms）
#   P2 判据有牙（负控形态）：同机同时刻强制走 CLI（PX_BIN=tools/px）中位 ≥ 1.5× P1
#        —— 若测量恒为常数（判据空转），两条会相等 ⇒ P2 必红
#   P3 语义等价：tools/px run 与 bootstrap/pxi 直跑 stdout/stderr/rc **逐字节相同**（≥10 个脚本）
#        前提自证：参与比对的脚本里必须 ≥8 个有非空 stdout（否则「空 == 空」也能过）
#   P4 功能：px_exec 执行的子脚本**能 import stdlib**（直连路径下模块解析仍成立）
#   N1 负控：PX_BIN 指向不存在的路径 ⇒ rc≠0 且 stderr 含 R1002（响亮，不是静默 ""）
#   环境不成立（tools/px 或 bootstrap/pxi 缺失 / 探针构建失败 / 可比对脚本 <10）⇒ rc=3，不降级成绿
#
# 用法：check_pxexec_direct.sh            # 全跑
#       check_pxexec_direct.sh --self-test # 只跑夹具（不上编译）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
PXI="$ROOT/bootstrap/pxi"
RT="$ROOT/runtime/runtime.c"
# 探针必须落在树内（M426 · 缺陷 864 补）：px_interp_bin 从 exe 目录逐级上溯 ≤6 层找
#   <dir>/bootstrap/pxi；建在 /tmp 下必找不到 ⇒ 退化到 PATH 找 px ⇒ CI 无 px ⇒ R1002 假红（本机装了 rpm 才恒绿）。
TMP="$ROOT/tmp/m424_pxexec.$$"   # $ROOT/.gitignore 已含 /tmp/ ⇒ 不脏树
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

median_of() {  # $1=输出的整段文本 ⇒ 打印 MEDIAN 数字
  printf '%s\n' "$1" | sed -n 's/^M424-MEDIAN-MS=//p' | head -1
}

# P1 判据（纯函数 · 可自测）：绝对墙钟在**负载饱和**时测的是机器不是产品
#   → 未达 20ms 且 load > 0.7×核数 时改判**相对口径**（≤0.5×CLI 才放行），并如实标注
#   ⚠️ 为什么必须有这一条：发布链 8 分片并行时 load≈11/16，fork+exec 会被拖慢一倍，
#      只留绝对阈值 ⇒ 每轮都假红（M294 的教训：绝对数不得单独作判据）。
p1_verdict() {  # $1=direct_ms $2=cli_ms $3=load $4=cores ⇒ OK|OK-REL|FAIL
  local d="$1" c="$2" l="$3" n="$4"
  if awk -v v="$d" 'BEGIN{exit !(v <= 20)}'; then echo OK; return; fi
  if awk -v l="$l" -v n="$n" 'BEGIN{exit !(l > 0.7*n)}'; then
    if awk -v d="$d" -v c="$c" 'BEGIN{exit !(c > 0 && d <= 0.5*c)}'; then echo OK-REL; else echo FAIL; fi
    return
  fi
  echo FAIL
}

if [ "${1:-}" = "--self-test" ]; then
  # 夹具：判据函数自身的行为（不依赖编译）
  echo "── self-test（判据夹具，不编译）"
  mkdir -p "$TMP"
  # F1 比例判据：直连 10ms / CLI 42ms ⇒ 必须过
  if awk 'BEGIN{exit !(42 >= 1.5*10)}'; then ok "F1 比例判据：CLI 42ms ≥ 1.5×直达 10ms ⇒ 过"; else bad "F1 比例判据误判"; fi
  # F2 负控：两条相等（判据空转形态）⇒ 必须红
  if awk 'BEGIN{exit !(10 >= 1.5*10)}'; then bad "F2 比例判据对「相等」判绿（判据无牙）"; else ok "F2 负控：相等(10/10) ⇒ 判红（判据有牙）"; fi
  # F3 阈值判据：20ms 上限
  if awk 'BEGIN{exit !(9.2 <= 20)}'; then ok "F3 阈值：直连 9.2ms ≤ 20ms ⇒ 过"; else bad "F3 阈值误判"; fi
  if awk 'BEGIN{exit !(41.7 <= 20)}'; then bad "F4 阈值对 41.7ms 判绿（无牙）"; else ok "F4 负控：CLI 41.7ms > 20ms ⇒ 判红"; fi
  # F5–F8 P1 判据（绝对阈 + 饱和降级的相对口径），每条都要能判红
  if type p1_verdict >/dev/null 2>&1; then
    [ "$(p1_verdict 9.2 41 1.0 16)" = OK ] && ok "F5 P1：9.2ms/CLI41/负载低 ⇒ OK" || bad "F5 P1 误判（低负载快路径）"
    [ "$(p1_verdict 22 26 2.0 16)" = FAIL ] && ok "F6 负控：22ms 且负载低 ⇒ FAIL（真的慢，不许降级）" || bad "F6 P1 低负载时把 22ms 判绿"
    [ "$(p1_verdict 22 45 12 16)" = OK-REL ] && ok "F7 P1 饱和降级：22ms/CLI45/负载 12-16 ⇒ OK-REL" || bad "F7 饱和降级不生效"
    [ "$(p1_verdict 22 24 12 16)" = FAIL ] && ok "F8 负控：饱和但相对也不成立（22 vs 24）⇒ FAIL" || bad "F8 饱和时把「无差别」判绿"
  else
    bad "F5–F8 p1_verdict 未定义（自测将空转）"
  fi
  echo "======================================"
  echo "PXEXEC-DIRECT-SELFTEST pass=$PASS fail=$FAIL"
  [ "$FAIL" -eq 0 ] && { echo "SELFTEST=OK"; exit 0; }
  echo "SELFTEST=FAIL"; exit 2
fi

for f in "$PX" "$PXI" "$RT"; do
  [ -e "$f" ] || { echo "PXEXEC-DIRECT rc=3（环境缺失：$f）"; exit 3; }
done
[ -x "$PX" ] && [ -x "$PXI" ] || { echo "PXEXEC-DIRECT rc=3（tools/px 或 bootstrap/pxi 不可执行）"; exit 3; }
mkdir -p "$TMP"

# ── S1–S4 源码形状 ────────────────────────────────────────────
grep -q 'static const char\* px_interp_bin' "$RT" \
  && ok "S1a 存在 px_interp_bin（直连解释器解析）" \
  || bad "S1a 没有 px_interp_bin（px_exec 仍走 bash 包装器）"
# 顺序：bootstrap/pxi 必须出现在 PATH 搜索之前
_first_pxi=$(grep -n 'bootstrap/pxi' "$RT" | head -1 | cut -d: -f1)
_first_path=$(grep -n 'getenv("PATH")' "$RT" | awk -F: -v p="${_first_pxi:-0}" '$1>p{print $1; exit}')
if [ -n "${_first_pxi:-}" ] && [ -n "${_first_path:-}" ] && [ "$_first_pxi" -lt "$_first_path" ]; then
  ok "S1b 自身树 bootstrap/pxi（行 $_first_pxi）优先于 PATH px（行 $_first_path）"
else
  bad "S1b 顺序错：PATH 的 px 仍先于本树 pxi（版本错配风险）"
fi
grep -q 'execlp(pxbin, "pxi", path' "$RT" \
  && ok "S2 直连分支 exec pxi" \
  || bad "S2 直连分支未 exec pxi"
grep -q 'execlp(pxbin, "px", "run", path' "$RT" \
  && ok "S2b CLI 兜底分支保留 px run" \
  || bad "S2b CLI 兜底分支丢失"
awk '/px_sibling_pxi/,/^}/' "$RT" | grep -q 'realpath' \
  && ok "S3 sibling 推导 realpath（/usr/bin/px 是 symlink）" \
  || bad "S3 sibling 推导未 realpath（找不到 /usr/share/puxian/bootstrap/pxi）"
grep -q 'R1002: px_exec 无法启动 px 解释器' "$RT" \
  && ok "S4 rc=2 解释器启动失败 ⇒ 响亮报错" \
  || bad "S4 rc=2 仍静默返回空串"
[ -s "$RT" ] && echo "   · 前提自证：扫描面 runtime.c=$(wc -c < "$RT") 字节" || bad "前提自证：runtime.c 为空"

# ── 探针：px_exec 15 次，脚本内自计时（now_ns）────────────────
mkdir -p "$TMP/probe"
cat > "$TMP/probe/inner.px" <<'PEOF'
print("inner-ok")
PEOF
cat > "$TMP/probe/probe.px" <<'PEOF'
write_file("/tmp/m424_probe_inner.px", "print(\"inner-ok\")\n")
var times = []
for i in range(15):
    let t0 = now_ns()
    let r = px_exec("/tmp/m424_probe_inner.px")
    let t1 = now_ns()
    assert(len(r) > 0, "px_exec 必须有输出（解释器可定位）")
    times.push((t1 - t0) / 1000000)
times.sort()
print("M424-MEDIAN-MS=" + str(times[7]))
PEOF
echo "── 构建探针（px build；首次会重编 runtime）"
if ! ( cd "$TMP/probe" && "$PX" build probe.px > "$TMP/build.log" 2>&1 ); then
  echo "PXEXEC-DIRECT rc=3（探针构建失败）"; tail -5 "$TMP/build.log"; exit 3
fi
BIN="$TMP/probe/build/probe"
[ -x "$BIN" ] || { echo "PXEXEC-DIRECT rc=3（探针产物缺失）"; exit 3; }


out_direct=$( env -u PX_BIN "$BIN" 2>"$TMP/direct.err" ); rc_direct=$?
out_cli=$( env PX_BIN="$PX" "$BIN" 2>"$TMP/cli.err" ); rc_cli=$?
md=$(median_of "$out_direct"); mc=$(median_of "$out_cli")
LOAD=$(cut -d' ' -f1 /proc/loadavg 2>/dev/null || echo 0)
CORES=$(nproc 2>/dev/null || echo 1)
echo "   · 机器前提：load=${LOAD} / cores=${CORES}（P1 绝对阈 20ms；饱和时改判相对口径）"

if [ $rc_direct -eq 0 ] && [ -n "$md" ]; then
  pv=$(p1_verdict "$md" "${mc:-0}" "${LOAD:-0}" "${CORES:-1}")
  case "$pv" in
    OK)     ok "P1 px_exec 单次中位 ${md}ms ≤ 20ms（直连 pxi）" ;;
    OK-REL) ok "P1 px_exec 单次中位 ${md}ms（负载 ${LOAD}/${CORES} 饱和 ⇒ 相对口径 ≤0.5×CLI ${mc}ms）" ;;
    *)      bad "P1 px_exec 单次中位 ${md}ms 未达标（CLI ${mc:-?}ms · load ${LOAD}/${CORES}）" ;;
  esac
else
  bad "P1 探针未跑通（rc=$rc_direct）"; sed -n '1,5p' "$TMP/direct.err"
fi
if [ -n "$md" ] && [ -n "$mc" ]; then
  if awk -v c="$mc" -v d="$md" 'BEGIN{exit !(c >= 1.5*d)}'; then
    ok "P2 伪证（判据有牙）：强制 CLI ${mc}ms ≥ 1.5×直连 ${md}ms"
  else
    bad "P2 CLI ${mc}ms < 1.5×直连 ${md}ms —— 测量空转（两路没差别）"
  fi
else
  bad "P2 缺测量值（direct='$md' cli='$mc'）"
fi

# ── P3 语义等价：px run vs pxi 直跑（逐字节）──────────────────
CAND="examples/hello.px examples/fib.px examples/struct.px examples/match.px
      examples/m26_ushr.px examples/m39_err_main.px examples/m25_closure_gc.px
      examples/p0_random_io.px examples/p2_crypto_hash.px examples/m60_gpio.px"
same=0; diff=0; nonempty=0; n=0
for f in $CAND; do
  [ -f "$ROOT/$f" ] || continue
  n=$((n+1))
  ( cd "$ROOT" && timeout 30 "$PX" run "$f" > "$TMP/a.out" 2> "$TMP/a.err" ); ra=$?
  ( cd "$ROOT" && timeout 30 "$PXI" "$f" > "$TMP/b.out" 2> "$TMP/b.err" ); rb=$?
  [ -s "$TMP/b.out" ] && nonempty=$((nonempty+1))
  if [ "$ra" -eq "$rb" ] && cmp -s "$TMP/a.out" "$TMP/b.out" && cmp -s "$TMP/a.err" "$TMP/b.err"; then
    same=$((same+1))
  else
    diff=$((diff+1)); echo "   · 差异：$f（rc $ra/$rb）"
  fi
done
[ "$n" -ge 10 ] || { echo "PXEXEC-DIRECT rc=3（可比对脚本只有 $n 个，<10）"; exit 3; }
[ "$nonempty" -ge 8 ] || { echo "PXEXEC-DIRECT rc=3（非空 stdout 只有 $nonempty 个，判据会「空==空」空转）"; exit 3; }
[ "$diff" -eq 0 ] \
  && ok "P3 $same/$n 脚本 stdout/stderr/rc 逐字节相同（非空 $nonempty 个）" \
  || bad "P3 直连与 px run 不等价：$diff 个脚本有差异"

# ── P4 直连路径下 stdlib import 仍成立 ────────────────────────
mkdir -p "$TMP/std"
cat > "$TMP/std/child.px" <<'PEOF'
import std.collections
let u = unique([1, 1, 2, 3, 3])
print("stdlib-ok uniq=" + str(len(u)))
PEOF
cat > "$TMP/std/probe2.px" <<'PEOF'
let r = px_exec("/tmp/m424_probe_std.px")
print("M424-STDLIB-CHILD len=" + str(len(r)))
assert(len(r) > 0, "stdlib 子脚本必须有输出")
print(r.strip())
PEOF
cp "$TMP/std/child.px" /tmp/m424_probe_std.px
if ( cd "$TMP/std" && "$PX" build probe2.px > "$TMP/build2.log" 2>&1 ) \
   && env -u PX_BIN "$TMP/std/build/probe2" > "$TMP/p4.out" 2> "$TMP/p4.err" \
   && grep -q 'stdlib-ok uniq=3' "$TMP/p4.out"; then
  ok "P4 px_exec 直连路径下 stdlib import 成立（unique([1,1,2,3,3]) ⇒ 3）"
else
  bad "P4 直连路径下 stdlib 子脚本失败"; sed -n '1,6p' "$TMP/p4.out" "$TMP/p4.err" 2>/dev/null
fi

# ── N1 负控：PX_BIN 指向不存在 ⇒ 响亮报错 ────────────────────
env PX_BIN=/nonexistent/px "$BIN" > "$TMP/n1.out" 2> "$TMP/n1.err"; rc=$?
if [ $rc -ne 0 ] && grep -q 'R1002: px_exec 无法启动 px 解释器' "$TMP/n1.err"; then
  ok "N1 PX_BIN 无效 ⇒ rc≠0 + R1002（不静默返回空串）"
else
  bad "N1 PX_BIN 无效却没说清（rc=$rc）"; sed -n '1,5p' "$TMP/n1.err"
fi

echo "======================================"
echo "PXEXEC-DIRECT pass=$PASS fail=$FAIL"
[ "$FAIL" -eq 0 ] || { echo "PXEXEC-DIRECT=FAIL rc=1"; exit 1; }
echo "PXEXEC-DIRECT=OK"
exit 0
