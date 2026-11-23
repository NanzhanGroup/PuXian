#!/usr/bin/env bash
. "$(dirname "$0")/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
# ============================================================
# M438 门 · px_slice 惰性偏移表（缺陷 915 · 晨曦报告）
# ------------------------------------------------------------
# 病根：px_slice 对 str **每次调用**都 xmalloc 全串 rune→byte 表 + 逐 rune 走查 ⇒
#   单次切片 O(串长) 且与切片长度无关（实测 90KB 串取 s[i:i+10] ≈170–330µs；
#   逐位置扫描 90K 次 15–30s）。而同文件 px_index 早就用 px_str_offs_get 惰性缓存
#   （MIN=1024 / MAX=16MiB）—— 同一文件两种生命周期。
# 修：优先复用缓存表（摊还 O(1)）；小串/巨串/表覆盖不到（仅畸形 UTF-8）回落原线性路径。
#
# 判据（5 组，逐组可单独变红）：
#   [1] 静态形状：缓存表复用 / 落表释放 / maxi 防低估 / 逃生口在位
#   [2] 等价性三轨：解释 · VM · C 轨跑 probes/m438_slice_equiv.px 逐字节一致
#   [3] 摊还性（C 轨）：2000 次切片 ≤ 200ms、20000 次 ≤ 1000ms
#   [4] 负控：PX_NO_STR_OFFS_SLICE=1（同一二进制只切一条路径）⇒ 2000 次 ≥ 200ms
#   [5] 前提自证：负控确实慢下来（≥5×）⇒ [4] 不是空转
# 用法：bash selfhost/check_m438_pxslice_offs.sh
# 退出码：0 = 绿 · 1 = 红 · 2 = 环境不成立
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/.."
ROOT=$(pwd)
PXI=./bootstrap/pxi
PX=./tools/px
CASE=selfhost/probes/m438_slice_equiv.px
PERF=selfhost/probes/m438_slice_perf.px
export LC_ALL=C LANG=C
for f in "$PXI" "$PX" "$CASE" "$PERF"; do
  [ -e "$f" ] || { echo "❌ ENV rc=2 缺 $f"; exit 2; }
done
W=$(mktemp -d /tmp/m438s.XXXXXX) || { echo "❌ ENV rc=2 mktemp 失败"; exit 2; }
trap 'rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M438 门 · px_slice 惰性偏移表（缺陷 915） =="

echo "[1/5] 静态形状"
SLICE_FN=$(awk '/^LXValue px_slice\(LXValue obj/,/^}/' runtime/runtime.c)
grep -q 'px_str_offs_get' <<< "$SLICE_FN" \
  && ok "px_slice 复用 px_str_offs_get（不再每次重建全表）" || bad "px_slice 未见 px_str_offs_get"
grep -q 'if (offs_owned) xfree(offs);' <<< "$SLICE_FN" \
  && ok "自建表按 offs_owned 释放（缓存表不误 free）" || bad "缺 offs_owned 释放口径"
grep -q '(int64_t)(n - 1) \* k_in' <<< "$SLICE_FN" \
  && ok "maxi 按 s+(n-1)*k_in 算（step>1 不低估）" || bad "maxi 未按 (n-1)*k_in 算（会误用覆盖不足的表）"
grep -q 'PX_NO_STR_OFFS_SLICE' runtime/runtime.c \
  && ok "逃生口 PX_NO_STR_OFFS_SLICE 在位（供 [4] 负控）" || bad "缺逃生口 PX_NO_STR_OFFS_SLICE"

echo "[2/5] 等价性三轨（解释 · VM · C）"
timeout 120 "$PXI" "$CASE" > "$W/interp.out" 2> "$W/interp.err"
rc=$?
[ "$rc" = 0 ] && ok "解释轨 rc=0" || { bad "解释轨 rc=$rc"; tail -3 "$W/interp.err" | sed 's/^/     /'; }
cp "$CASE" "$W/s.px"
if timeout 500 "$PX" build "$W/s.px" > "$W/vm.log" 2>&1 && [ -x "$W/build/s" ]; then
  timeout 120 "$W/build/s" > "$W/vm.out" 2>&1
  cmp -s "$W/interp.out" "$W/vm.out" && ok "VM 轨与解释轨逐字节一致" \
    || { bad "VM 轨与解释轨不一致"; diff "$W/interp.out" "$W/vm.out" | head -6 | sed 's/^/     /'; }
else
  bad "VM 轨编译失败"; tail -3 "$W/vm.log" | sed 's/^/     /'
fi
rm -rf "$W/build"
if timeout 600 "$PX" build --c "$W/s.px" > "$W/c.log" 2>&1 && [ -x "$W/build/s" ]; then
  timeout 120 "$W/build/s" > "$W/c.out" 2>&1
  cmp -s "$W/interp.out" "$W/c.out" && ok "C 轨与解释轨逐字节一致" \
    || { bad "C 轨与解释轨不一致"; diff "$W/interp.out" "$W/c.out" | head -6 | sed 's/^/     /'; }
else
  bad "C 轨编译失败"; tail -3 "$W/c.log" | sed 's/^/     /'
fi
grep -q 'FAIL ' "$W/interp.out" && bad "语料自报 FAIL（等价性用例不过）" || ok "语料 17 条等价性用例全过"

echo "[3/5] 摊还性（C 轨 · 90K 串）"
cp "$PERF" "$W/p.px"
rm -rf "$W/build"
if timeout 600 "$PX" build --c "$W/p.px" > "$W/p.log" 2>&1 && [ -x "$W/build/p" ]; then
  timeout 120 "$W/build/p" > "$W/perf_fast.out" 2>&1
  t2000=$(grep '^slices=2000 ' "$W/perf_fast.out" | sed 's/.*total_ms=\([0-9]*\).*/\1/')
  t20000=$(grep '^slices=20000 ' "$W/perf_fast.out" | sed 's/.*total_ms=\([0-9]*\).*/\1/')
  echo "     实测：2000 次=${t2000:-?}ms · 20000 次=${t20000:-?}ms"
  [ -n "${t2000:-}" ] && [ "$t2000" -le 200 ] && ok "2000 次切片 ≤ 200ms（摊还 O(1)）" \
    || bad "2000 次切片 = ${t2000:-?}ms（> 200ms）"
  [ -n "${t20000:-}" ] && [ "$t20000" -le 1000 ] && ok "20000 次切片 ≤ 1000ms" \
    || bad "20000 次切片 = ${t20000:-?}ms（> 1000ms）"
else
  bad "perf 语料 C 轨编译失败"; tail -3 "$W/p.log" | sed 's/^/     /'
fi

echo "[4/5] 负控：PX_NO_STR_OFFS_SLICE=1（同一二进制，只切一条路径）"
if [ -x "$W/build/p" ]; then
  PX_NO_STR_OFFS_SLICE=1 timeout 300 "$W/build/p" > "$W/perf_slow.out" 2>&1
  s2000=$(grep '^slices=2000 ' "$W/perf_slow.out" | sed 's/.*total_ms=\([0-9]*\).*/\1/')
  echo "     负控实测：2000 次=${s2000:-?}ms"
  [ -n "${s2000:-}" ] && [ "$s2000" -ge 200 ] && ok "负控下 2000 次 ≥ 200ms（摊还判据有牙）" \
    || bad "负控下 2000 次 = ${s2000:-?}ms（< 200ms ⇒ 判据在空转）"
  # 逃生口语义不变：慢但结果必须一致
  diff <(grep -E '^slices=.*bytes=' "$W/perf_fast.out" | sed 's/total_ms=[0-9]*//') \
       <(grep -E '^slices=.*bytes=' "$W/perf_slow.out" | sed 's/total_ms=[0-9]*//') >/dev/null \
    && ok "逃生口只影响快慢、不影响结果（byte 计数一致）" || bad "逃生口改变了结果（语义被破坏）"
else
  bad "缺 C 轨产物，负控无法执行"
fi

echo "[5/5] 前提自证：负控确实慢下来（≥5×）"
if [ -n "${t2000:-}" ] && [ -n "${s2000:-}" ] && [ "${t2000:-0}" -gt 0 ]; then
  ratio=$((s2000 / (t2000 > 0 ? t2000 : 1)))
  echo "     负控/正面 = ${s2000}ms / ${t2000}ms ≈ ${ratio}×"
  [ "$ratio" -ge 5 ] && ok "负控与正面差 ${ratio}×（扰动真发生）" || bad "负控/正面仅 ${ratio}×（扰动不明显）"
else
  bad "缺耗时数据，无法自证"
fi

echo
echo "── 小计：通过 $pass · 失败 $fail ──"
[ "$fail" -gt 0 ] && { echo "❌ M438 px_slice 门失败"; exit 1; }
echo "✅ M438 px_slice 门通过"
exit 0
