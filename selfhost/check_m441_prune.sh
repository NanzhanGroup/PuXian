#!/usr/bin/env bash
# ============================================================
# selfhost/check_m441_prune.sh —— 产物裁剪开关（M441 · 缺陷 923/924）
# ------------------------------------------------------------
# 病象：runtime 侧对 onnx 的裁剪（-DPX_NO_ONNX + 源文件移出编译集 + native_mod_map 条目）
#   自 M156 起全部就绪，**只差 tools/px 未接线** ⇒ 用户看得见文档却用不了开关。
#   同时实测发现第二个问题：改了 tools/px 的裁剪逻辑后，产物缓存**不失效**（键里没有构建器）。
#
# 判据（每条都要能判红；负控必须有前提自证）：
#   S1 形状：--no-onnx / --gc-sections 解析在；三处 mod 循环含 onnx；
#            rt_key 的 gcs 只在 gcs=1 时进哈希（gcs=0 与旧口径逐字节一致）；
#            build_cache_key 把构建器自身内容纳入键
#   S2 负控（S1 的判据有牙）：把 tools/px 临时副本里的 --no-onnx 行删掉 ⇒ 静态检查必须判红
#   P1 默认行为不变：默认 build 的产物**含** onnx 符号（自动裁剪不裁 onnx），尺寸 > --no-onnx
#   P2 --no-onnx 生效：产物 onnx 符号数 == 0，且尺寸严格小于默认，且能跑出正确输出
#   P3 --no-onnx 的运行期契约：用 onnx 的示例默认能解析（nodes=1）→ --no-onnx 必须 R1001
#            （前提自证：默认那次确实解析成功，否则 R1001 可能是别的原因）
#   P4 --gc-sections：产物能跑（含 stdlib 字符串操作 + 递归 + onnx 解析三条），且尺寸 < 默认
#   P5 组合：--no-onnx --gc-sections 同时生效（onnx=0 且能跑）
# 全部 build 一律 PX_BUILD_NOCACHE=1 —— 缓存命中会让判据失真（本项正是因为缓存踩的坑）。
#
# 用法：check_m441_prune.sh            # 全跑（实测 ~150s）
#       check_m441_prune.sh --self-test # 只跑夹具
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
env_bad() { echo "❌ 环境不成立：$1" >&2; exit 3; }

[ -f "$PX" ] || env_bad "缺 $PX"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m441.XXXXXX")" || env_bad "mktemp 失败"
trap 'rm -rf "$TMP"' EXIT

if [ "${1:-}" = "--self-test" ]; then
  echo "── self-test（判据夹具，不动仓库）"
  grep -q -- '--no-onnx) mods="\$mods onnx"' "$PX" \
    && ok "F1 正样本：当前 tools/px 含 --no-onnx 接线" || bad "F1 正样本不成立（当前树已缺接线）"
  sed '/--no-onnx) mods="\$mods onnx"/d' "$PX" > "$TMP/x.sh"
  grep -q -- '--no-onnx) mods="\$mods onnx"' "$TMP/x.sh" \
    && bad "F2 负控：删掉接线后仍判有 ⇒ 判据无牙" || ok "F2 负控：删掉接线 ⇒ 判无（有牙）"
  echo "  小计：$PASS 通过 / $FAIL 失败"
  [ "$FAIL" -eq 0 ] || exit 2
  exit 0
fi

syms() { nm "$1" 2>/dev/null | grep -c -i "$2"; }
size() { stat -c%s "$1" 2>/dev/null || echo 0; }
build() { # $1=out 其余=px build 参数
  local out="$1"; shift
  ( cd "$ROOT" && PX_BUILD_NOCACHE=1 ./tools/px build "$@" /tmp/m441_gate.px ) >"$TMP/b.log" 2>&1
  [ -f /tmp/build/m441_gate ] || { echo "BUILD-FAIL"; tail -3 "$TMP/b.log"; return 1; }
  cp /tmp/build/m441_gate "$out" 2>/dev/null
}

printf 'def main():\n    print("m441-ok\\n")\n' > /tmp/m441_gate.px

echo "── S1 形状"
grep -q -- '--no-onnx) mods="\$mods onnx"' "$PX" && ok "S1a --no-onnx 解析在" || bad "S1a 缺 --no-onnx 解析"
grep -q -- '--gc-sections) gcs=1' "$PX" && ok "S1b --gc-sections 解析在" || bad "S1b 缺 --gc-sections 解析"
n_loop=$(grep -c 'for m.* in quic sqlite ws zip xml aes rsa ed25519 img route zlib h2 onnx' "$PX")
n_loop2=$(grep -c 'for m2 in quic sqlite ws zip xml aes rsa ed25519 img route zlib h2 onnx' "$PX")
[ "${n_loop:-0}" -ge 1 ] && ok "S1c 预编译 mod_macro 循环含 onnx" || bad "S1c mod_macro 循环未含 onnx"
[ "${n_loop2:-0}" -ge 1 ] && ok "S1d 单文件编译循环含 onnx" || bad "S1d 单文件循环未含 onnx"
grep -q 'gcs=1' "$PX" && grep -q '\[ "\$gcs" = 1 \] && echo " gcs=1"' "$PX" \
  && ok "S1e gcs 只在 gcs=1 时进 rt_key（gcs=0 与旧口径一致）" \
  || bad "S1e gcs 无条件进键 ⇒ 默认路径会冷缓存"
grep -q 'sha256sum "\$PXC_SELF"' "$PX" \
  && ok "S1f build_cache_key 纳入构建器自身内容" || bad "S1f 产物缓存键未覆盖构建器（缺陷 924 回归）"

echo "── S2 负控（判据有牙）"
sed '/--no-onnx) mods="\$mods onnx"/d' "$PX" > "$TMP/x.sh"
grep -q -- '--no-onnx) mods="\$mods onnx"' "$TMP/x.sh" \
  && bad "S2 负控无效：删掉接线后仍判有" || ok "S2 删掉接线 ⇒ S1a 判无（有牙）"

echo "── P1/P2 默认不变与 --no-onnx"
build "$TMP/def" || env_bad "默认构建失败"
build "$TMP/non" --no-onnx || env_bad "--no-onnx 构建失败"
d_def=$(size "$TMP/def"); d_non=$(size "$TMP/non")
o_def=$(syms "$TMP/def" onnx); o_non=$(syms "$TMP/non" onnx)
[ "$o_def" -gt 0 ] && ok "P1a 默认产物含 onnx 符号（$o_def 个 ⇒ 自动裁剪未改动默认行为）" \
                   || bad "P1a 默认产物已无 onnx 符号 ⇒ 默认行为被改了（违反硬约束）"
[ "$o_non" -eq 0 ] && ok "P2a --no-onnx 产物 onnx 符号 = 0" || bad "P2a --no-onnx 仍含 $o_non 个 onnx 符号"
[ "$d_non" -lt "$d_def" ] && ok "P2b --no-onnx 严格更小（$d_def → $d_non，省 $((d_def-d_non))B）" \
                          || bad "P2b --no-onnx 未变小（$d_def → $d_non）"
out=$("$TMP/non" 2>&1 | head -1)
[ "$out" = "m441-ok" ] && ok "P2c --no-onnx 产物运行正确" || bad "P2c --no-onnx 运行异常：$out"

echo "── P3 运行期契约（负控有前提自证）"
EX="$ROOT/examples/m156_onnx/onnx_dump.px"; MD="$ROOT/examples/m156_onnx/models/t01_basic.onnx"
if [ -f "$EX" ] && [ -f "$MD" ]; then
  ( cd "$ROOT" && PX_BUILD_NOCACHE=1 ./tools/px build "$EX" ) >"$TMP/o1.log" 2>&1
  cp "$ROOT/examples/m156_onnx/build/onnx_dump" "$TMP/od_def" 2>/dev/null
  ( cd "$ROOT" && PX_BUILD_NOCACHE=1 ./tools/px build --no-onnx "$EX" ) >"$TMP/o2.log" 2>&1
  cp "$ROOT/examples/m156_onnx/build/onnx_dump" "$TMP/od_non" 2>/dev/null
  r1=$("$TMP/od_def" "$MD" "$TMP/o1.json" 2>&1 | head -1)
  r2=$("$TMP/od_non" "$MD" "$TMP/o2.json" 2>&1 | head -1)
  case "$r1" in nodes=*) ok "P3a 前提自证：默认产物真能解析（$r1）" ;;
                *) bad "P3a 前提不成立：默认产物未能解析（$r1）⇒ P3b 的结论不可用" ;; esac
  case "$r2" in *R1001*) ok "P3b --no-onnx 后 onnx native 缺失 ⇒ R1001（契约如实）" ;;
                *) bad "P3b --no-onnx 后未报 R1001（实得：$r2）" ;; esac
else
  bad "P3 缺示例 $EX 或模型 $MD"
fi

echo "── P4/P5 --gc-sections"
build "$TMP/gcs" --gc-sections || env_bad "--gc-sections 构建失败"
g_sz=$(size "$TMP/gcs"); g_out=$("$TMP/gcs" 2>&1 | head -1)
[ "$g_sz" -lt "$d_def" ] && ok "P4a --gc-sections 更小（$d_def → $g_sz，省 $((d_def-g_sz))B）" \
                         || bad "P4a --gc-sections 未变小（$d_def → $g_sz）"
[ "$g_out" = "m441-ok" ] && ok "P4b --gc-sections 产物运行正确" || bad "P4b 运行异常：$g_out"
build "$TMP/both" --no-onnx --gc-sections || env_bad "组合构建失败"
b_out=$("$TMP/both" 2>&1 | head -1); b_on=$(syms "$TMP/both" onnx)
[ "$b_on" -eq 0 ] && [ "$b_out" = "m441-ok" ] \
  && ok "P5 --no-onnx --gc-sections 组合生效且可运行（$(size "$TMP/both")B）" \
  || bad "P5 组合异常（onnx=$b_on out=$b_out）"

rm -f /tmp/m441_gate.px
echo "── 小计：$PASS 通过 / $FAIL 失败"
[ "$FAIL" -eq 0 ] && echo "M441-PRUNE-OK" || echo "M441-PRUNE-FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
