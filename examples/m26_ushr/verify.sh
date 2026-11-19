#!/usr/bin/env bash
# M422 · m26_ushr 双轨门（示例即判据）
# 病史：examples/m26_ushr.px 自 M217（缺陷 309）把「负数移位计数」改成 R1003 之后，
#   示例里仍写着「-1 ⇒ 按 mod 64 折成 63 ⇒ 1」；而它**从没被任何门跑过**（M421 账上 blackbox）
#   ⇒ 示例与实现相反却无人发现。本门把它变成真跑：双轨断言 + 负数计数 R1003 负控。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="$(mktemp -d /tmp/m26_ushr.XXXXXX)"
trap 'rm -rf "$WORK"' EXIT
PASS=0; FAIL=0
ok(){ echo "  ✅ $1"; PASS=$((PASS+1)); }
bad(){ echo "  ❌ $1"; FAIL=$((FAIL+1)); }

echo "=== m26_ushr · >>> 无符号右移（双轨 + 负数计数 R1003 负控）==="
[ -x "$PX" ] || { echo "❌ 环境不成立：$PX 不可执行"; exit 3; }

# 编译到临时目录，避免与并发门抢 examples/build/
cp "$ROOT/examples/m26_ushr.px" "$WORK/m26_ushr.px"

# 轨1 解释器
ip_out=$(cd "$ROOT" && timeout --kill-after=5 120 "$PX" run "$WORK/m26_ushr.px" 2>&1); ip_rc=$?
if [ "$ip_rc" = 0 ] && printf '%s' "$ip_out" | grep -q 'M26 USHR TESTS PASSED'; then
  ok "解释轨 rc=0 且全部断言通过"
else
  bad "解释轨 rc=$ip_rc"; printf '%s\n' "$ip_out" | tail -5
fi

# 轨2 编译（VM 字节码轨）
(cd "$WORK" && timeout --kill-after=5 180 "$PX" build m26_ushr.px) > "$WORK/build.log" 2>&1; b_rc=$?
if [ "$b_rc" = 0 ] && [ -x "$WORK/build/m26_ushr" ]; then
  ok "编译轨构建成功"
  v_out=$(timeout --kill-after=5 60 "$WORK/build/m26_ushr" 2>&1); v_rc=$?
  if [ "$v_rc" = 0 ] && printf '%s' "$v_out" | grep -q 'M26 USHR TESTS PASSED'; then
    ok "编译轨运行 rc=0 且全部断言通过"
  else
    bad "编译轨运行 rc=$v_rc"; printf '%s\n' "$v_out" | tail -5
  fi
else
  bad "编译轨构建失败 rc=$b_rc"; tail -5 "$WORK/build.log"
fi

# 负控：负数移位计数必须 R1003（证明「示例已改为不写负数」不是掩盖了判据）
cat > "$WORK/neg.px" <<'EOF'
let c = -1
print(c >>> -1)
EOF
n_out=$(cd "$ROOT" && timeout --kill-after=5 60 "$PX" run "$WORK/neg.px" 2>&1); n_rc=$?
if [ "$n_rc" != 0 ] && printf '%s' "$n_out" | grep -q 'R1003'; then
  ok "负控：负数计数 ⇒ rc=$n_rc 且含 R1003"
else
  bad "负控失效：负数计数 rc=$n_rc 输出未含 R1003"; printf '%s\n' "$n_out" | tail -3
fi

# 前提自证：两条轨都真的跑到了断言（防止「什么都没跑也算绿」）
if printf '%s' "$ip_out" | grep -q 'M26 USHR TESTS PASSED' && [ -n "${v_out:-}" ]; then
  ok "前提自证：两轨均产生 PASSED 输出（非空跑）"
else
  bad "前提自证失败：至少一轨无 PASSED 输出"
fi

echo "---- m26_ushr: PASS=$PASS FAIL=$FAIL ----"
[ "$FAIL" -eq 0 ] || exit 1
echo "M26-USHR-VERDICT=OK"
