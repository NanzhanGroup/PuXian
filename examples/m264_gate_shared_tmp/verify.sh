#!/usr/bin/env bash
# ============================================================
# M264 门：**门间隔离**（共用固定 `/tmp` 路径的白名单制）
# ------------------------------------------------------------
# 病灶（M260 §八 候选 ③）：两个门若共用**同一条固定 `/tmp` 路径**，A 门的残留
#   （文件/目录/进程）会**改变 B 门的行为** ⇒ 「单独跑全绿、串起来跑却红」这类不可复现的怪现象
#   （M260 实测过一例：m157 跑完后 m260 探针 5/5 被杀，单独跑全绿）。
# 本轮处置：
#   ① 新守卫 `selfhost/check_gate_shared_tmp.sh` + 白名单 `selfhost/gate_shared_tmp.txt`
#      （**共享可以是**有意的，但必须显式登记 + 写理由；未登记⇒判红，无理由⇒rc=3，过期⇒判红）；
#   ② 把唯一一处**非有意**的共用修掉：m260 复用 m256 的探针，而探针的数据目录是**硬编码**
#      的 `/tmp/px_m256_probe` ⇒ 参数化为 `M256_PROBE_D`（m260 传自己的 `$W/probe_d`）。
# 判据分层：
#   [1] 守卫自证 4/4（未登记必红 · 登记后绿 · 过期必红 · 无理由 rc=3）
#   [2] 真仓库：0 未登记 · 0 过期 · 规模锚点（门数 · 共用名数下限）
#   [3] 白名单形态：每条带**非空理由**，条数与实测一致
#   [4] 隔离修法在位（`M256_PROBE_D` 可覆盖 + m260 传自己的目录）
#   [5] 负控 A：**临时造一个未登记共用** ⇒ 守卫必判红并指名
#   [6] 负控 B（判据自伤）：把白名单替换成「只有名字、没有理由」 ⇒ 必须 rc=3
#   [7] 覆盖边界
# 用法：bash examples/m264_gate_shared_tmp/verify.sh [--neg-skip]
# 本门**不含 .px 语料** ⇒ 不触发发射冻结门重定基。
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
G=selfhost/check_gate_shared_tmp.sh
WL=selfhost/gate_shared_tmp.txt
W=/tmp/m264_gate
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
rm -rf "$W"; mkdir -p "$W/bak"
cp "$WL" "$W/bak/wl.txt"
restore_all() { cp -f "$W/bak/wl.txt" "$WL"; }
trap 'restore_all; rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M264 门：门间隔离（共用固定 /tmp 路径 · 白名单制） =="
echo
echo "[1/7] 守卫自证（4/4）"
if bash "$G" --self-test >"$W/st.log" 2>&1 && grep -q 'GATE-SHARED-TMP-SELFTEST-OK' "$W/st.log"; then
  ok "$(grep 'self-test:' "$W/st.log")"
else
  bad "守卫自证失败"; tail -8 "$W/st.log" | sed 's/^/     /'
fi

echo
echo "[2/7] 真仓库：0 未登记 · 0 过期 · 规模锚点"
OUT=$(bash "$G" --root . 2>&1); RC=$?
if [ "$RC" = 0 ]; then ok "$OUT"; else bad "守卫判红（rc=$RC）：$(echo "$OUT" | head -3 | tr '\n' ' ')"; fi

echo
echo "[3/7] 白名单形态：每条带非空理由 · 条数与实测一致"
NENT=$(grep -vc '^#' "$WL" 2>/dev/null || true)
NENT=$(printf '%s\n' "$NENT" | tail -1)
NEMPTY=$(grep -v '^#' "$WL" | awk -F'\t' 'NF<2 || $2 ~ /^ *$/ {c++} END{print c+0}')
[ "$NENT" -ge 6 ] && ok "白名单 $NENT 条" || bad "白名单条数 $NENT（期望 ≥6）"
[ "$NEMPTY" = 0 ] && ok "0 条缺理由" || bad "$NEMPTY 条缺理由"
if bash "$G" --root . --verbose >"$W/v.log" 2>&1 && grep -qc '·' "$W/v.log"; then
  ok "共用名清单可枚举（--verbose）"
else
  bad "共用名清单不可枚举"
fi

echo
echo "[4/7] 隔离修法在位（M256_PROBE_D 可覆盖 + m260 传自己的目录）"
grep -q 'env("M256_PROBE_D")' examples/m256_eintr/probe_eintr.px \
  && ok "probe_eintr.px：数据目录可由 M256_PROBE_D 覆盖" \
  || bad "probe_eintr.px 未参数化"
grep -q 'M256_PROBE_D="\$W/probe_d"' examples/m260_gc_repro/verify.sh \
  && ok "m260：运行时传入自己的探针目录" \
  || bad "m260 未传 M256_PROBE_D"
grep -q 'mkdir -p "\$W/probe_d"' examples/m260_gc_repro/verify.sh \
  && ok "m260：用自己的 mkdir 目标（不再共用固定路径）" \
  || bad "m260 的 mkdir 目标未改"

if [ "$NEG_SKIP" = 0 ]; then
echo
echo "[5/7] 负控 A：临时造一个**未登记共用** ⇒ 守卫必判红并指名"
T="$W/fake"
mkdir -p "$T/examples/zz1" "$T/examples/zz2" "$T/selfhost"
printf '/tmp/m264_fake_shared\n' > "$T/examples/zz1/verify.sh"
printf '/tmp/m264_fake_shared\n' > "$T/examples/zz2/verify.sh"
for _i in $(seq 1 150); do mkdir -p "$T/examples/g$_i"; done
cp "$WL" "$T/selfhost/gate_shared_tmp.txt"
OUT=$(bash "$G" --root "$T" 2>&1); RC=$?
if [ "$RC" = 1 ] && echo "$OUT" | grep -q 'm264_fake_shared'; then
  ok "守卫判红并指名：$(echo "$OUT" | grep '未登记' | head -1 | cut -c1-70)"
else
  bad "未按预期判红（rc=$RC）：$(echo "$OUT" | head -2 | tr '\n' ' ')"
fi

echo
echo "[6/7] 负控 B（判据自伤）：白名单「只有名字、没有理由」⇒ 必须 rc=3"
sed 's/\t.*$//' "$WL" > "$WL.tmp" && mv "$WL.tmp" "$WL"
OUT=$(bash "$G" --root . 2>&1); RC=$?
if [ "$RC" = 3 ] && echo "$OUT" | grep -q '无理由'; then
  ok "rc=3 且文案指向真因（无理由豁免 = 把红当绿）"
else
  bad "未按预期 rc=3（实得 $RC）：$(echo "$OUT" | head -2 | tr '\n' ' ')"
fi
restore_all
cmp -s "$W/bak/wl.txt" "$WL" && ok "白名单逐字节还原" || bad "白名单还原不完整"
else
echo; echo "[5-6/7] 负控已跳过（--neg-skip）"
fi

echo
echo "[7/7] 覆盖边界（如实登记）"
echo '   · 本门判据是**静态**（扫 examples/*/verify.sh 与 examples/*/*.px|.sh 的 /tmp 顶层名）。'
echo '   · 剥离器**逐行**判引号 ⇒ 跨行引号串里的路径仍会计入（已知边界，见守卫头注）：'
echo '     处置 = 门里**不复述旧路径字面量**。'
echo '   · 「共用**端口**」不在本门判据内（例如 m260 固定的 18420/18421 是**有意**且已在它的'
echo '     覆盖边界里写「不可并行跑」）—— 若要判据化，需要另一套「端口清单」白名单（下一轮候选）。'
echo '   · 白名单里 6 条共用名**都是有意共享**（开发构件缓存 / devbuild 指纹 / tools 默认输出目录 /'
echo '     ONNX 外部模型），理由逐条写在 selfhost/gate_shared_tmp.txt。'
echo
echo "── M264 门结束：pass=$pass fail=$fail"
[ "$fail" = 0 ] && echo "M264-VERIFY-OK" || echo "M264-VERIFY-FAIL"
exit "$fail"
