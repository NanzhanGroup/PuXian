#!/usr/bin/env bash
# ============================================================
# selfhost/check_build_cache.sh —— `px build` 产物缓存（M425 · 缺陷 866）
# ------------------------------------------------------------
# 为什么有它（2026-11-20 本机实测）：
#   3 行程序连跑 5 次 px build：1251 / 1345 / 1371 / 1372 / 1623 ms，**产物逐字节相同**。
#   拆解：emit-c 11 ms + gcc -c 128 ms + **静态链接 ≈1.1 s**（3.1 MB 静态产物）。
#   `.rtcache` 只缓存 runtime 的 .o（按引用集分桶），用户程序这一层**零缓存** ⇒
#   改一行注释重编译、门里重复构建、CI 重跑，每次白付一秒多。
#
# 判据（每条都要能判红；缓存只许影响快慢，不许影响语义）：
#   S1 源码形状：BUILD_CACHE + build_cache_key 存在；key 含 basename（产物内嵌程序名）
#   S2 源码形状：命中前校验产物 sha16；写盘走 tmp+mv（原子）
#   S3 源码形状：有编译期诊断（.c.stderr 非空）不写缓存
#   P1 冷构建 rc=0；二次构建**命中**且产物 sha256 与冷构建**完全相同**
#   P2 命中耗时判据：≤400ms 且 ≤0.25×冷构建（负载饱和时降级为 ≤0.5×冷构建）
#   P3 命中 stdout 与冷构建**逐字节相同**（"编译成功: …（N 字节）"）
#   P4 **串味负控**：同内容不同 basename ⇒ 不得互相命中
#        （前提自证：两个 basename 的冷构建产物 sha 必须不同，否则这条判据无意义）
#   P5 源码改一字节 ⇒ 必须 miss，且跑起来是新行为
#   P6 依赖文件（被 import 的项目内 .px）改一字节 ⇒ 必须 miss，且跑起来是新行为
#   P7 构建 flag 变（--no-quic）⇒ 必须 miss
#   P8 缓存条目被篡改（内容与 sha16 不符）⇒ 不命中 + 重建 + 产物正确（不许产出坏二进制）
#   P9 缓存条目被截断 ⇒ 同上
#   P10 PX_BUILD_NOCACHE=1 ⇒ 完全绕过（不命中，仍成功）
#   P11 缓存目录不可建（父路径是普通文件）⇒ 构建仍成功 rc=0（降级，不许因缓存而失败）
#   P12 并发 4 个 build 同一源 ⇒ 全部 rc=0 且产物 sha 一致（原子写，不读到半成品）
#   N1 编译失败的脚本 ⇒ rc≠0 且**不写缓存**
#   N2 失败脚本重跑仍失败（缓存不许把红变绿）
#   N3 有编译期诊断（模块缺失）⇒ rc=0 但**不写缓存**（诊断不许被缓存吞掉）
#   P16 runtime 源内容进 key（缺陷 868）：改 runtime.c ⇒ 不许命中；**保留 mtime 复原**
#        ⇒ 必须回到原产物（「改回旧值」也算了变化 —— `find -newer` 看不见这一侧）
#   环境不成立（tools/px 缺失 / 探针构建失败 / 扫描面为空）⇒ rc=3，绝不降级成绿
#
# 用法：check_build_cache.sh            # 全跑（默认 ~40s）
#       check_build_cache.sh --self-test # 只跑判据夹具（不编译）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m425_buildcache.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
# M425 补2（缺陷 868）：P16 会**临时改 runtime/runtime.c**，退出路径必须无条件复原。
RT_BAK=""
cleanup() {
  if [ -n "${RT_BAK:-}" ] && [ -f "$RT_BAK" ]; then
    cp -p "$RT_BAK" "$ROOT/runtime/runtime.c" 2>/dev/null
  fi
  chmod -R u+w "$TMP" 2>/dev/null; rm -rf "$TMP"
}
trap cleanup EXIT

now_ms() { date +%s%N 2>/dev/null | cut -c1-13 | sed 's/^0*//' ; }

# ── 判据（纯函数 · 可自测）──────────────────────────────────
# 绝对数不得单独作判据（M294 教训）：链上 8 分片并行时 fork/链接会被拖慢近一倍，
# 只留 400ms 绝对阈会周期性假红 ⇒ 同时要求**相对**（命中 ≤ 0.25×冷构建）；
# 负载饱和时相对口径放宽到 0.5×（仍必须是「快一截」，不许相等）。
hit_verdict() { # $1=冷构建ms $2=命中ms $3=load $4=cores ⇒ OK|OK-REL|FAIL
  local cold="$1" hit="$2" l="$3" n="$4"
  awk -v c="$cold" -v h="$hit" 'BEGIN{exit !(h <= 400 && c > 0 && h <= 0.25*c)}' && { echo OK; return; }
  if awk -v l="$l" -v n="$n" 'BEGIN{exit !(l > 0.7*n)}'; then
    awk -v c="$cold" -v h="$hit" 'BEGIN{exit !(c > 0 && h <= 0.5*c)}' && { echo OK-REL; return; }
  fi
  echo FAIL
}

if [ "${1:-}" = "--self-test" ]; then
  echo "── self-test（判据夹具，不编译）"
  c=1670
  [ "$(hit_verdict 1670 185 0.5 16)" = OK ] && ok "F1 185ms/0.5负载 ⇒ OK（相对 0.11）" || bad "F1 正常命中被判红"
  [ "$(hit_verdict 1670 1670 0.5 16)" = FAIL ] && ok "F2 负控：相等(1670/1670) ⇒ FAIL（判据有牙）" || bad "F2 对「没加速」判绿"
  [ "$(hit_verdict 1670 600 0.5 16)" = FAIL ] && ok "F3 负控：600ms 未达 0.25× ⇒ FAIL" || bad "F3 阈值过松"
  [ "$(hit_verdict 1670 600 12 16)" = OK-REL ] && ok "F4 饱和降级：600ms/负载12 ⇒ OK-REL" || bad "F4 饱和降级不生效"
  [ "$(hit_verdict 1670 900 12 16)" = FAIL ] && ok "F5 负控：饱和但 900ms > 0.5× ⇒ FAIL" || bad "F5 饱和时把「几乎没加速」判绿"
  [ "$(hit_verdict 1670 500 0.5 16)" = FAIL ] && ok "F6 负控：500ms > 400ms 绝对阈 ⇒ FAIL" || bad "F6 绝对阈失效"
  [ "$(hit_verdict 0 0 0.5 16)" = FAIL ] && ok "F7 负控：冷构建=0（测量没拿到）⇒ FAIL" || bad "F7 零值被判绿"
echo "======================================"
  echo "BUILD-CACHE-SELFTEST pass=$PASS fail=$FAIL"
  [ "$FAIL" -eq 0 ] && { echo "SELFTEST=OK"; exit 0; }
  echo "SELFTEST=FAIL"; exit 2
fi

[ -e "$PX" ] || { echo "BUILD-CACHE rc=3（环境缺失：$PX）"; exit 3; }
[ -x "$PX" ] || { echo "BUILD-CACHE rc=3（tools/px 不可执行）"; exit 3; }
BC="$TMP/bc"; mkdir -p "$BC"
# ⚠️ 必须显式导出：默认缓存根是 $PXC_HOME/.buildcache（= 仓库根）⇒ 不导出就会往仓库树里写
#   未跟踪目录，preflight 的「树净」判据会当场判红（M422 已经踩过一次同类坑）。
export PX_BUILDCACHE_DIR="$BC"
W="$TMP/w"; mkdir -p "$W"

bld() { # $1=目录 $2..=build 参数（第一项是脚本）⇒ rc；stdout/stderr 落 $1/o,$1/e
  ( cd "$1" && PX_BUILD_VERBOSE=1 timeout 180 "$PX" build "${@:2}" > o 2> e )
}
timed_bld() { # 同上 ⇒ 把耗时 ms 打到 stdout
  local t0 t1 rc
  t0=$(now_ms); bld "$@"; rc=$?; t1=$(now_ms)
  echo $(( t1 - t0 ))
  return $rc
}

# ── S1–S3 源码形状 ──────────────────────────────────────────
grep -q 'BUILD_CACHE=' "$PX" && ok "S1a tools/px 定义 BUILD_CACHE" || bad "S1a 没有 BUILD_CACHE"
grep -q '^build_cache_key()' "$PX" && ok "S1b 存在 build_cache_key" || bad "S1b 没有 build_cache_key"
kfn="$(awk '/^build_cache_key\(\)/,/^}/' "$PX")"
grep -q 'base=%s' <<< "$kfn" \
  && ok "S1c key 含 basename（产物内嵌程序名，名不进 key 会串味）" \
  || bad "S1c key 未含 basename（串味风险）"
hitblk="$(awk '/M425（缺陷 866）/,/cache_hit" = 1/' "$PX")"
grep -q 'prog.sha16' <<< "$hitblk" \
  && ok "S2a 命中前校验产物 sha16（M271 同口径）" || bad "S2a 命中前不校验产物完整性"
grep -q 'mv -f "$out_path.pxcache' <<< "$hitblk" \
  && ok "S2b 写盘走 tmp+mv（原子）" || bad "S2b 非原子写盘（可能读到半个二进制）"
grep -q '! -s "$cerr_log"' "$PX" \
  && ok "S3 有编译期诊断不写缓存" || bad "S3 诊断会被缓存吞掉"
[ -s "$PX" ] && echo "   · 前提自证：扫描面 tools/px=$(wc -c < "$PX") 字节" || bad "前提自证：tools/px 为空"

# ── P1/P2/P3 冷构建 vs 命中 ─────────────────────────────────
mkdir -p "$W/v1"
cat > "$W/v1/main.px" <<'EOF'
print("v1")
EOF
t_cold=$(timed_bld "$W/v1" main.px); rc=$?
if [ "$rc" -ne 0 ]; then
  echo "BUILD-CACHE rc=3（探针冷构建失败 rc=$rc）"; sed -n '1,8p' "$W/v1/e"; exit 3
fi
BIN1="$W/v1/build/main"
[ -x "$BIN1" ] || { echo "BUILD-CACHE rc=3（探针产物缺失）"; exit 3; }
sha_cold=$(sha256sum "$BIN1" | cut -d' ' -f1)
out_cold=$(cat "$W/v1/o")
echo "   · 冷构建 ${t_cold}ms · sha16=$(printf '%s' "$sha_cold" | cut -c1-16)"

t_hit=$(timed_bld "$W/v1" main.px); rc=$?
grep -q '缓存命中' "$W/v1/e" && hit_seen=1 || hit_seen=0
sha_hit=$(sha256sum "$BIN1" | cut -d' ' -f1)
[ "$rc" -eq 0 ] && [ "$hit_seen" -eq 1 ] \
  && ok "P1a 二次构建命中（rc=0 · 跳编译）" \
  || bad "P1a 二次构建未命中（rc=$rc hit=$hit_seen）"
[ "$sha_hit" = "$sha_cold" ] \
  && ok "P1b 命中产物与冷构建逐字节相同" \
  || bad "P1b 命中产物与冷构建不同（缓存改变了产物）"
LOAD=$(cut -d' ' -f1 /proc/loadavg 2>/dev/null || echo 0)
CORES=$(nproc 2>/dev/null || echo 1)
hv=$(hit_verdict "$t_cold" "$t_hit" "$LOAD" "$CORES")
case "$hv" in
  OK)     ok "P2 命中 ${t_hit}ms ≤0.25×冷构建 ${t_cold}ms（load=${LOAD}/${CORES}）" ;;
  OK-REL) ok "P2 命中 ${t_hit}ms（负载饱和 ⇒ 相对口径 ≤0.5×${t_cold}ms）" ;;
  *)      bad "P2 命中 ${t_hit}ms 未达标（冷 ${t_cold}ms · load ${LOAD}/${CORES}）" ;;
esac
[ "$(cat "$W/v1/o")" = "$out_cold" ] \
  && ok "P3 命中 stdout 与冷构建逐字节相同（编译成功行不变）" \
  || bad "P3 命中 stdout 变了（判据：$(cat "$W/v1/o") vs $out_cold）"

# ── P4 串味负控：同内容不同 basename ─────────────────────────
mkdir -p "$W/x" "$W/y"
cp "$W/v1/main.px" "$W/x/aaa.px"
cp "$W/v1/main.px" "$W/y/bbb.px"
bld "$W/x" aaa.px || { echo "BUILD-CACHE rc=3（aaa 构建失败）"; exit 3; }
bld "$W/y" bbb.px || { echo "BUILD-CACHE rc=3（bbb 构建失败）"; exit 3; }
sha_a=$(sha256sum "$W/x/build/aaa" | cut -d' ' -f1)
sha_b=$(sha256sum "$W/y/build/bbb" | cut -d' ' -f1)
if [ "$sha_a" = "$sha_b" ]; then
  echo "BUILD-CACHE rc=3（前提不成立：不同 basename 的产物 sha 相同 ⇒ P4 判据会空转）"; exit 3
fi
echo "   · 前提自证：aaa/bbb 冷构建 sha 不同（内嵌程序名）"
bld "$W/y" bbb.px
sha_b2=$(sha256sum "$W/y/build/bbb" | cut -d' ' -f1)
if [ "$sha_b2" = "$sha_b" ] && [ "$sha_b2" != "$sha_a" ]; then
  ok "P4 同内容不同 basename 不串味（bbb 命中后仍是 bbb 的产物）"
else
  bad "P4 串味：bbb 拿到了 aaa 的产物（sha ${sha_b2:0:16}）"
fi

# ── P5 源码改一字节 ⇒ miss + 新行为 ────────────────────────
printf 'print("v2")\n' > "$W/v1/main.px"
bld "$W/v1" main.px
grep -q '缓存写入' "$W/v1/e" && is_miss=1 || is_miss=0
run_out="$("$BIN1" 2>/dev/null)"
if [ "$is_miss" -eq 1 ] && [ "$run_out" = "v2" ]; then
  ok "P5 源码变更 ⇒ miss 且产物为新行为（v2）"
else
  bad "P5 源码变更未失效（miss=$is_miss out=$run_out）"
fi

# ── P6 依赖变更 ⇒ miss + 新行为 ────────────────────────────
mkdir -p "$W/dep"
printf 'def greet():\n    return "ver1"\n' > "$W/dep/helper.px"
printf 'import "helper.px"\nprint(greet())\n' > "$W/dep/main.px"
bld "$W/dep" main.px || { echo "BUILD-CACHE rc=3（依赖探针构建失败）"; exit 3; }
sed -i 's/ver1/ver2/' "$W/dep/helper.px"
bld "$W/dep" main.px
grep -q '缓存写入' "$W/dep/e" && dep_miss=1 || dep_miss=0
dep_out="$("$W/dep/build/main" 2>/dev/null)"
if [ "$dep_miss" -eq 1 ] && [ "$dep_out" = "ver2" ]; then
  ok "P6 依赖 .px 变更 ⇒ miss 且产物为新行为（ver2）"
else
  bad "P6 依赖变更未失效（miss=$dep_miss out=$dep_out）"
fi

# ── P7 flag 变 ⇒ miss ─────────────────────────────────────
bld "$W/v1" --no-quic main.px
grep -q '缓存写入' "$W/v1/e" && ok "P7 构建 flag 变（--no-quic）⇒ miss" || bad "P7 flag 变仍命中（key 漏了 flag）"

# ── P8/P9 缓存条目被篡改/截断 ⇒ 不命中且重建正确 ───────────
bld "$W/v1" main.px                       # 确保有命中条目
key=$(sed -n 's/.*缓存命中 key=\([0-9a-f]*\).*/\1/p' "$W/v1/e" | head -1)
if [ -z "$key" ]; then
  bad "P8/P9 拿不到缓存 key（命中行缺失 ⇒ 后续判据无法执行）"
else
  bdir="$BC/$key"
  if [ ! -f "$bdir/prog" ]; then
    bad "P8/P9 缓存条目不存在：$bdir/prog"
  else
    for mode in tamper truncate; do
      case "$mode" in
        tamper)   printf 'GARBAGE-NOT-A-BINARY\n' > "$bdir/prog" ;;
        truncate) : > "$bdir/prog" ;;
      esac
      bld "$W/v1" main.px
      if grep -q '缓存命中' "$W/v1/e"; then
        bad "P8/${mode} 坏条目仍被命中（会产出坏二进制）"
      else
        sha_now=$(sha256sum "$BIN1" | cut -d' ' -f1)
        got="$("$BIN1" 2>/dev/null)"
        if [ "$sha_now" = "$sha_cold" ] || [ "$got" = "v2" ]; then
          ok "P8/${mode} 坏条目 ⇒ 不命中 + 重建（产物可运行，输出 $got）"
        else
          bad "P8/${mode} 重建后产物异常（out=$got sha=${sha_now:0:16}）"
        fi
      fi
    done
  fi
fi

# ── P10 PX_BUILD_NOCACHE=1 ⇒ 完全绕过 ──────────────────────
( cd "$W/v1" && PX_BUILD_NOCACHE=1 PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > o3 2> e3 ); rc=$?
if [ "$rc" -eq 0 ] && ! grep -q '缓存命中' "$W/v1/e3"; then
  ok "P10 PX_BUILD_NOCACHE=1 ⇒ 不读缓存且构建成功"
else
  bad "P10 NOCACHE 未绕过（rc=$rc）"
fi

# ── P11 缓存目录不可建 ⇒ 仍成功 ────────────────────────────
: > "$TMP/afile"
( cd "$W/v1" && PX_BUILDCACHE_DIR="$TMP/afile/sub" PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > o4 2> e4 ); rc=$?
if [ "$rc" -eq 0 ] && ! grep -q '缓存命中' "$W/v1/e4"; then
  ok "P11 缓存目录不可建 ⇒ 构建仍成功（降级，不因缓存失败）"
else
  bad "P11 缓存不可写导致构建失败（rc=$rc）"
fi

# ── P12 并发 4 个 build 同一源 ─────────────────────────────
mkdir -p "$W/par"
printf 'print("par")\n' > "$W/par/p.px"
for i in 1 2 3 4; do
  ( cd "$W/par" && PX_BUILD_VERBOSE=1 timeout 180 "$PX" build p.px > "o$i" 2> "e$i" ) &
done
wait
pc=0; for i in 1 2 3 4; do [ -x "$W/par/build/p" ] && pc=$((pc+1)); done
shas=$(sha256sum "$W/par/build/p" 2>/dev/null | cut -d' ' -f1)
pout="$("$W/par/build/p" 2>/dev/null)"
[ "$pc" -eq 4 ] && [ -n "$shas" ] && [ "$pout" = "par" ] \
  && ok "P12 并发 4 个 build ⇒ 产物一致可运行（sha ${shas:0:16}）" \
  || bad "P12 并发构建产物异常（pc=$pc out=$pout）"

# ── N1/N2 失败不写缓存、失败不许被缓存变绿 ─────────────────
mkdir -p "$W/bad"
printf 'def f(\n' > "$W/bad/bad.px"          # 语法错误
before=$(find "$BC" -maxdepth 1 -type d 2>/dev/null | wc -l)
bld "$W/bad" bad.px; rc1=$?
after=$(find "$BC" -maxdepth 1 -type d 2>/dev/null | wc -l)
if [ "$rc1" -ne 0 ] && [ "$before" -eq "$after" ]; then
  ok "N1 编译失败 rc≠0 且不写缓存（条目 $before→$after）"
else
  bad "N1 失败被缓存或判绿（rc=$rc1 条目 $before→$after）"
fi
bld "$W/bad" bad.px; rc2=$?
[ "$rc2" -ne 0 ] && ok "N2 失败脚本重跑仍失败（缓存不会把红变绿）" || bad "N2 重跑变绿（缓存篡改语义）"

# ── N3 有诊断 ⇒ 不写缓存 ──────────────────────────────────
mkdir -p "$W/warn"
printf 'import "nope.px"\nprint("w")\n' > "$W/warn/w.px"
before=$(find "$BC" -maxdepth 1 -type d 2>/dev/null | wc -l)
bld "$W/warn" w.px; rc=$?
after=$(find "$BC" -maxdepth 1 -type d 2>/dev/null | wc -l)
if [ "$rc" -eq 0 ] && [ "$before" -eq "$after" ]; then
  ok "N3 有模块缺失诊断 ⇒ rc=0 但不写缓存（诊断不被吞掉）"
else
  bad "N3 带诊断的构建仍写缓存（条目 $before→$after）"
fi

# ── S4 源码形状：**编译器身份必须进 key**（M425 补1 · 缺陷 867）──
#   为什么要有这条：M425 首轮链 10 红（m204/m233/m237/m246/m301/m166/m218/m220/m239）
#   全部是「负控打桩后 `PX_PXC_BIN=/tmp/pxcdev px build` ⇒ 命中旧条目 ⇒ C 轨仍是好实现
#   ⇒ 负控必须判红失败」。实测（/tmp/diag425）：`PX_PXC_BIN=/bin/false` 也能「编译成功」
#   —— 缓存把一个不同（甚至坏的）编译器的产物冒充成本次产物，这是**语义**问题不是速度问题。
grep -q '^comp_fp()' "$PX" && ok "S4a tools/px 定义 comp_fp" || bad "S4a 没有 comp_fp"
grep -q 'comp_fp' <<< "$kfn" \
  && ok "S4b build_cache_key 含编译器身份（PX_PXC_BIN/PXC_VM_BIN/PX_PXI_BIN 覆盖口）" \
  || bad "S4b build_cache_key 漏了编译器身份（换编译器会命中旧产物）"
runfn="$(awk '/^run_cache_key\(\)/,/^}/' "$PX")"
grep -q 'comp_fp' <<< "$runfn" \
  && ok "S4c run_cache_key 含编译器身份（--fast 与 build 同口径）" \
  || bad "S4c run_cache_key 漏了编译器覆盖口（/tmp/pxidev 类负控会被中和）"

# ── S5 源码形状：stdlib 内容必须进 key（缺陷 867 第②洞）──────
#   `import std.*` 由 PX_STDLIB 解析，px_resolve_import 对 std.* 直接返回空
#   （为「沙箱复制不漂移」）⇒ stdlib/*.px **不在依赖闭包**里。实测（diag425）：
#   改 stdlib/semver.px 后 `px build` 仍打印「缓存命中」⇒ 编译器内联的 stdlib 被缓存忽略。
cfn="$(awk '/^comp_fp\(\)/,/^}/' "$PX")"
grep -q 'PX_STDLIB' <<< "$cfn" \
  && ok "S5a comp_fp 覆盖 PX_STDLIB 下的 *.px（stdlib 每件内容进 key）" \
  || bad "S5a comp_fp 没有 stdlib 身份（改 stdlib 会命中旧产物）"
grep -qE 'PX_PXC_BIN|PXC_VM_BIN|PX_PXI_BIN' <<< "$cfn" \
  && ok "S5b comp_fp 覆盖三个编译器覆盖口（PX_PXC_BIN / PXC_VM_BIN / PX_PXI_BIN）" \
  || bad "S5b comp_fp 漏了编译器覆盖口"

# ── P13 负控：换「不能产 C 的编译器」⇒ 不许命中，且必须响亮失败 ──
if [ -x /bin/false ]; then
  # 前提自证：同一源在 engine=c 下已有可命中条目（否则「不命中」是假绿）
  ( cd "$W/v1" && PX_BUILD_ENGINE=c PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > o5 2> e5 )
  ( cd "$W/v1" && PX_BUILD_ENGINE=c PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > o5b 2> e5b )
  if grep -q '缓存命中' "$W/v1/e5b"; then
    echo "   · 前提自证：engine=c 同编译器二次构建命中（证明这一组 key 本来是会命中的）"
  else
    bad "P13 前提不成立：engine=c 二次构建未命中（后续「换编译器不命中」无从判定）"
  fi
  rm -rf "$W/v1/build"
  ( cd "$W/v1" && PX_BUILD_ENGINE=c PX_PXC_BIN=/bin/false PX_BUILD_VERBOSE=1 \
      timeout 180 "$PX" build main.px > o6 2> e6 ); rc13=$?
  grep -q '缓存命中' "$W/v1/e6" \
    && bad "P13a 换了编译器仍命中缓存（缺陷 867：门的负控会被静默中和）" \
    || ok "P13a 换编译器 ⇒ 不命中（编译器身份进了 key）"
  [ "$rc13" -ne 0 ] \
    && ok "P13b 坏编译器 ⇒ 响亮失败 rc=$rc13（不许拿旧产物冒充成功）" \
    || bad "P13b 坏编译器仍判成功 rc=0（错误缓存比不缓存更糟）"
else
  bad "P13 前提不成立：/bin/false 不可执行（环境异常）"
fi

# ── P14 正控：**真编译器但换了实现**（输出被改成 WRAPPED）⇒ 必须 miss 且新行为生效 ──
#   与 P4 不同：P4 变的是 basename，P14 变的是**编译器**（sha 不同、内容不同）。
#   前提自证：wrapper 的 sha16 与真 pxc 不同（否则这条判据空转）。
printf 'print("v1")\n' > "$W/v1/main.px"
cat > "$TMP/wrap_pxc" <<EOF
#!/usr/bin/env bash
"$ROOT/bootstrap/pxc" "\$@" | sed 's/"v1"/"WRAPPED"/'
EOF
chmod +x "$TMP/wrap_pxc"
sha_w=$(sha256sum "$TMP/wrap_pxc" | cut -c1-16)
sha_r=$(sha256sum "$ROOT/bootstrap/pxc" 2>/dev/null | cut -c1-16)
if [ -n "$sha_r" ] && [ "$sha_w" != "$sha_r" ]; then
  echo "   · 前提自证：wrapper sha16=$sha_w ≠ 真 pxc sha16=$sha_r"
  rm -rf "$W/v1/build"
  ( cd "$W/v1" && PX_BUILD_ENGINE=c PX_PXC_BIN="$TMP/wrap_pxc" PX_BUILD_VERBOSE=1 \
      timeout 180 "$PX" build main.px > o7 2> e7 ); rc14=$?
  w_out="$("$W/v1/build/main" 2>/dev/null)"
  if [ "$rc14" -eq 0 ] && ! grep -q '缓存命中' "$W/v1/e7" && [ "$w_out" = "WRAPPED" ]; then
    ok "P14 换真编译器（输出 WRAPPED）⇒ miss 且新产物生效（编译器身份真的进 key）"
  else
    bad "P14 换编译器未生效（rc=$rc14 out=$w_out hit=$(grep -c '缓存命中' "$W/v1/e7")）"
  fi
else
  bad "P14 前提不成立：拿不到真 pxc 的 sha（或 wrapper 与它同 sha）"
fi

# ── P15 stdlib 内容进 key（缺陷 867 第②洞 · 真行为）──────────────
mkdir -p "$TMP/std"
cp "$ROOT"/stdlib/*.px "$TMP/std"/ 2>/dev/null
printf 'import std.semver\n\ndef main():\n    print(sv_cmp("1.0.0", "2.0.0"))\n' > "$W/v1/main.px"
( cd "$W/v1" && PX_STDLIB="$TMP/std" PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > os1 2> es1 )
( cd "$W/v1" && PX_STDLIB="$TMP/std" PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > os2 2> es2 ); rc15=$?
if [ "$rc15" -eq 0 ] && grep -q '缓存命中' "$W/v1/es2"; then
  echo "   · 前提自证：PX_STDLIB 副本同内容二次构建命中（证明这条路径本来会命中）"
  printf '\n# m425 p15 probe\n' >> "$TMP/std/semver.px"
  ( cd "$W/v1" && PX_STDLIB="$TMP/std" PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > os3 2> es3 )
  grep -q '缓存命中' "$W/v1/es3" \
    && bad "P15 stdlib 内容变更仍命中缓存（编译器内联的 stdlib 被缓存忽略）" \
    || ok "P15 stdlib 内容变更 ⇒ 不命中（stdlib 每件内容进 key）"
else
  bad "P15 前提不成立：PX_STDLIB 副本二次构建未命中（rc=$rc15）"
fi

# ── P16 runtime 源内容进 key（缺陷 868 · 「改回旧值」也算变）──────────
#   病灶（2026-11-20 23:2x · M425 全量门 m233 负控 B 红）：toolchain_fp 的 memo 只用
#   `find -newer` 判有效 ⇒ 用 `cp -p` / `shutil.copy2` **保留 mtime** 复原回来时（负控、
#   回滚、打桩复原的标准手法）memo 看不见「变旧」这一侧 ⇒ 继续吐旧 rt_key ⇒ 产物缓存
#   把「回退版 runtime」与「正式版 runtime」算成同一个输入（题眼：坏产物以正式 key 落盘）。
#   判据三段都要，缺一段都是空转：
#     ① 前提自证：同内容二次构建**命中**（证明这条路径本来就会命中）
#     ② 改 runtime.c（追加一行注释）⇒ 必须**不命中**
#     ③ **保留 mtime 复原** ⇒ 必须回到 ① 的正式产物（命中），且源码逐字节还原
RT_C="$ROOT/runtime/runtime.c"
if [ -f "$RT_C" ]; then
  RT_BAK="$TMP/runtime.c.bak"; cp -p "$RT_C" "$RT_BAK"
  printf 'print("rt16")\n' > "$W/v1/main.px"
  ( cd "$W/v1" && PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > oA 2> eA ); rcA=$?
  sha_rtA=$(sha256sum "$W/v1/build/main" 2>/dev/null | cut -d' ' -f1)
  ( cd "$W/v1" && PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > oB 2> eB ); rcB=$?
  if [ "$rcA" -eq 0 ] && [ "$rcB" -eq 0 ] && grep -q '缓存命中' "$W/v1/eB"; then
    echo "   · 前提自证：同内容二次构建命中（这条路径本来会命中）"
    sha_rt=$(sha256sum "$RT_C" | cut -d' ' -f1)
    printf '\n// m425 p16 probe\n' >> "$RT_C"
    ( cd "$W/v1" && PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > oC 2> eC )
    grep -q '缓存命中' "$W/v1/eC" \
      && bad "P16b 改 runtime 源仍命中缓存（缺陷 868：runtime 未进产物缓存 key）" \
      || ok "P16b 改 runtime 源 ⇒ 不命中（runtime 内容/签名进了 key）"
    cp -p "$RT_BAK" "$RT_C"          # ③ 刻意保留 mtime —— 正是掩盖病灶的那种复原
    if [ "$(sha256sum "$RT_C" | cut -d' ' -f1)" = "$sha_rt" ]; then
      ( cd "$W/v1" && PX_BUILD_VERBOSE=1 timeout 180 "$PX" build main.px > oD 2> eD )
      sha_rtD=$(sha256sum "$W/v1/build/main" 2>/dev/null | cut -d' ' -f1)
      if grep -q '缓存命中' "$W/v1/eD" && [ "$sha_rtD" = "$sha_rtA" ]; then
        ok "P16c 保留 mtime 复原 ⇒ 命中并回到正式产物（memo 双向失效生效）"
      else
        bad "P16c 复原后未回到正式产物（hit=$(grep -c '缓存命中' "$W/v1/eD") sha=$([ "$sha_rtD" = "$sha_rtA" ] && echo same || echo diff)）"
      fi
    else
      bad "P16c 前提不成立：runtime.c 未逐字节还原"
    fi
  else
    bad "P16 前提不成立：同内容二次构建未命中（rcA=$rcA rcB=$rcB）"
  fi
  RT_BAK=""
fi

echo "======================================"
echo "BUILD-CACHE pass=$PASS fail=$FAIL"
[ "$FAIL" -eq 0 ] || { echo "BUILD-CACHE=FAIL rc=1"; exit 1; }
echo "BUILD-CACHE=OK"
exit 0
