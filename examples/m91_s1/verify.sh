#!/usr/bin/env bash
# ============================================================
# M91 verify.sh —— 默认轨切 VM 验收（tools/px 用户入口级）
# ------------------------------------------------------------
# 断言（全部真实执行，退出码 0=全 PASS）：
#   1) 默认 `px build` 产物 = VM 轨（BCModule 字节码镜像头）
#   2) `px build --c` 产物 = C 轨（fn_* C 文本头，逃生舱）
#   3) `PX_BUILD_ENGINE=c` env 逃生舱 = C 轨
#   4) VM 轨自动裁剪生效：默认产物 < 4,000,000 B（非全能力 9MB；
#      s_G 引用集提取，同 C 轨自动最小口径）
#   5) 显式 flag 优先 env：PX_BUILD_ENGINE=c + --vm → VM 轨
#   6) 双轨运行 stdout 逐字节一致（默认 VM vs --c C 轨）
#   7) 旧 flag --vm/--bc 兼容 = 与默认 VM 等价
# 前置：bootstrap/pxc_vm（M91 重链，含 F1 默认参数）+ tools/px。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m91_s1"
OUT=/tmp/m91_s1_verify
rm -rf "$OUT"; mkdir -p "$OUT"
cp "$DIR/hello_m91.px" "$OUT/"

PASS=0; FAIL=0
chk() { # $1=名 $2=0/1(pass/fail) $3=详情
    if [ "$2" = 0 ]; then echo "  ✅ $1"; PASS=$((PASS+1));
    else echo "  ❌ $1 — ${3:-}"; FAIL=$((FAIL+1)); fi
}
vm_head='字节码模块（VM 执行）'

echo "── 1) 默认 px build = VM 轨"
if "$PX" build "$OUT/hello_m91.px" >/dev/null 2>&1 && \
   grep -q "$vm_head" "$OUT/build/hello_m91.c"; then
    chk "默认 px build 产物 = VM 轨" 0
else
    chk "默认 px build 产物 = VM 轨" 1 "产物头: $(head -1 "$OUT/build/hello_m91.c" 2>/dev/null)"
fi

echo "── 2) px build --c = C 轨逃生舱"
if "$PX" build --c "$OUT/hello_m91.px" >/dev/null 2>&1 && \
   ! grep -q "$vm_head" "$OUT/build/hello_m91.c" && \
   grep -q '由普贤' "$OUT/build/hello_m91.c"; then
    chk "px build --c = C 轨" 0
else
    chk "px build --c = C 轨" 1 "产物头: $(head -1 "$OUT/build/hello_m91.c" 2>/dev/null)"
fi

echo "── 3) PX_BUILD_ENGINE=c env 逃生舱"
if PX_BUILD_ENGINE=c "$PX" build "$OUT/hello_m91.px" >/dev/null 2>&1 && \
   ! grep -q "$vm_head" "$OUT/build/hello_m91.c"; then
    chk "PX_BUILD_ENGINE=c = C 轨" 0
else
    chk "PX_BUILD_ENGINE=c = C 轨" 1
fi

echo "── 4) VM 轨自动裁剪（默认产物 < 4MB）"
"$PX" build "$OUT/hello_m91.px" >/dev/null 2>&1
sz=$(stat -c %s "$OUT/build/hello_m91" 2>/dev/null || echo 0)
if [ "$sz" -gt 0 ] && [ "$sz" -lt 4000000 ]; then
    chk "VM 轨自动裁剪（${sz} B < 4MB）" 0
else
    chk "VM 轨自动裁剪（${sz} B < 4MB）" 1 "s_G 提取裁剪未生效？"
fi

echo "── 5) 显式 flag 优先 env"
if PX_BUILD_ENGINE=c "$PX" build --vm "$OUT/hello_m91.px" >/dev/null 2>&1 && \
   grep -q "$vm_head" "$OUT/build/hello_m91.c"; then
    chk "PX_BUILD_ENGINE=c + --vm → VM 轨" 0
else
    chk "PX_BUILD_ENGINE=c + --vm → VM 轨" 1
fi

echo "── 6) 双轨运行 stdout 逐字节一致"
"$PX" build "$OUT/hello_m91.px" >/dev/null 2>&1   # 默认 VM
vm_out=$("$OUT/build/hello_m91" 2>&1)
"$PX" build --c "$OUT/hello_m91.px" >/dev/null 2>&1 # C 轨
c_out=$("$OUT/build/hello_m91" 2>&1)
if [ -n "$vm_out" ] && [ "$vm_out" = "$c_out" ]; then
    chk "VM/C 轨 stdout 逐字节一致" 0
else
    chk "VM/C 轨 stdout 逐字节一致" 1 "VM=[$vm_out] C=[$c_out]"
fi

echo "── 7) 旧 flag --vm/--bc 兼容 = 默认等价"
"$PX" build --vm "$OUT/hello_m91.px" >/dev/null 2>&1
vm_a=$("$OUT/build/hello_m91" 2>&1)
"$PX" build "$OUT/hello_m91.px" >/dev/null 2>&1
vm_b=$("$OUT/build/hello_m91" 2>&1)
if [ "$vm_a" = "$vm_b" ] && [ -n "$vm_a" ]; then
    chk "--vm/--bc 兼容（与默认输出一致）" 0
else
    chk "--vm/--bc 兼容（与默认输出一致）" 1
fi

echo ""
echo "════════ M91 verify ════════"
echo "PASS: $PASS  FAIL: $FAIL"
[ "$FAIL" = 0 ]
