#!/usr/bin/env bash
# ============================================================
# examples/m159_hostarch/verify.sh —— 门：**宿主机架构自适应**（R45 · 缺口 G2）
# ------------------------------------------------------------
# 背景（第三方仓库照出的缺口）：aarch64 原生跑 `px build` 此前必须手写
#   --cc/--mbedtls-lib/--sqlite-obj/--zlib-lib/--no-quic 一长串，否则链接失败
#   （mbedtls 路径写死 x86_64 布局 lib；QUIC 静态库只有 x86_64）。
# 本门证明：工具**自己知道宿主是谁**，并把决策做成了可断言输出（--print-plan）。
# 六层：
#   ① x86_64 宿主计划 = 现状（lib / libz / no_quic=0）——零回归
#   ② aarch64 宿主计划 = lib-aarch64 + sqlite3-aarch64.o + lib-aarch64/zlib + no_quic=1
#   ③ armv7 / riscv64 宿主计划同理（库布局按 arch 推导）
#   ④ 逃生舱：PX_HOST_QUIC=1 时 no_quic 保持 0（自备该架构 QUIC 库的用户不被强裁）
#   ⑤ 显式 flag 优先：--no-quic 显式给定时不打印自动提示（决策归用户）
#   ⑥ 负控：库缺失时必须**明确报错并给出获取方式**，不得静默退化到 x86_64 库
# ============================================================
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
PX="./tools/px"
W="$(mktemp -d /tmp/m159_hostarch_XXXXXX)"
trap 'rm -rf "$W"' EXIT
cat > "$W/h.px" <<'EOF'
print("hello")
EOF
FAIL=0
ok()   { echo "  ✅ $1"; }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
has()  { grep -qxF "$2" "$1" && ok "$3" || { bad "$3（期望行: $2）"; sed 's/^/      /' "$1"; }; }
no()   { grep -qF "$2" "$1" && bad "$3（不该出现: $2）" || ok "$3"; }

echo "== ① x86_64 宿主：与现状一致（显式 PX_HOST_ARCH，保证本门在任意宿主机器上结论一致）=="
PX_HOST_ARCH=x86_64 $PX build --print-plan "$W/h.px" > "$W/p_x86.txt" 2>"$W/e_x86.txt"
has "$W/p_x86.txt" "plan: host_arch=x86_64" "host_arch=x86_64"
has "$W/p_x86.txt" "plan: mbedtls_lib=$ROOT/runtime/mbedtls/lib" "mbedtls=lib"
has "$W/p_x86.txt" "plan: zlib_lib=$ROOT/runtime/third_party/zlib/lib" "zlib=lib"
has "$W/p_x86.txt" "plan: no_quic=0" "QUIC 保持开启"

echo "== ② aarch64 宿主：自动换库布局 + 自动裁 QUIC =="
PX_HOST_ARCH=aarch64 $PX build --print-plan "$W/h.px" > "$W/p_a64.txt" 2>"$W/e_a64.txt"
has "$W/p_a64.txt" "plan: host_arch=aarch64" "host_arch=aarch64"
has "$W/p_a64.txt" "plan: mbedtls_lib=$ROOT/runtime/mbedtls/lib-aarch64" "mbedtls=lib-aarch64"
has "$W/p_a64.txt" "plan: sqlite_obj=$ROOT/runtime/third_party/sqlite3/sqlite3-aarch64.o" "sqlite=sqlite3-aarch64.o"
has "$W/p_a64.txt" "plan: zlib_lib=$ROOT/runtime/third_party/zlib/lib-aarch64" "zlib=lib-aarch64"
has "$W/p_a64.txt" "plan: no_quic=1" "自动 --no-quic"
grep -q "宿主机 aarch64" "$W/e_a64.txt" && ok "提示可见（不静默）" || bad "缺少提示行"

echo "== ③ armv7 / riscv64 宿主（仓库未预置库 ⇒ 计划 或 明确报错，二者皆可；绝不用 x86_64 布局）=="
for a in armv7 riscv64; do
  PX_HOST_ARCH=$a $PX build --print-plan "$W/h.px" > "$W/p_$a.txt" 2>"$W/e_$a.txt"
  rc=$?
  if [ "$rc" = "0" ]; then
    has "$W/p_$a.txt" "plan: host_arch=$a" "host_arch=$a"
    grep -q "^plan: mbedtls_lib=.*lib-$a\$" "$W/p_$a.txt" && ok "mbedtls 目录以 lib-$a 结尾（仓库预置或 /opt/px-multiarch）" || { bad "$a：mbedtls 目录不含 lib-$a"; grep '^plan: mbedtls_lib=' "$W/p_$a.txt" | sed 's/^/      /'; }
    grep -q "^plan: zlib_lib=.*lib-$a\$" "$W/p_$a.txt" && ok "zlib 目录以 lib-$a 结尾" || bad "$a：zlib 目录不含 lib-$a"
    has "$W/p_$a.txt" "plan: no_quic=1" "$a 自动裁 QUIC"
    grep -q "plan: mbedtls_lib=$ROOT/runtime/mbedtls/lib$" "$W/p_$a.txt" && bad "$a 选了 x86_64 布局" || ok "$a 未选 x86_64 布局"
  else
    grep -q "缺预置 mbedtls 静态库" "$W/e_$a.txt" && ok "$a：库未预置 ⇒ 明确报错（rc=$rc）" || { bad "$a：报错文案不含根因"; sed 's/^/      /' "$W/e_$a.txt"; }
    grep -q "cross_multiarch.sh" "$W/e_$a.txt" && ok "$a：给出获取方式" || bad "$a：未给获取方式"
  fi
done

echo "== ④ 逃生舱 PX_HOST_QUIC=1 =="
PX_HOST_ARCH=aarch64 PX_HOST_QUIC=1 $PX build --print-plan "$W/h.px" > "$W/p_esc.txt" 2>"$W/e_esc.txt"
has "$W/p_esc.txt" "plan: no_quic=0" "自备 QUIC 库时保留 QUIC"
no "$W/e_esc.txt" "本次自动 --no-quic" "逃生舱下不打印自动裁剪提示"

echo "== ⑤ 显式 flag 优先 =="
PX_HOST_ARCH=aarch64 $PX build --print-plan --no-quic "$W/h.px" > "$W/p_exp.txt" 2>"$W/e_exp.txt"
has "$W/p_exp.txt" "plan: no_quic=1" "显式 --no-quic 生效"
no "$W/e_exp.txt" "本次自动 --no-quic" "显式给定时不再自动（决策归用户）"

echo "== ⑥ 负控：宿主库缺失须明确报错，不得静默用 x86_64 库 =="
PX_HOST_ARCH=loongarch64 $PX build "$W/h.px" > "$W/n_body.txt" 2>&1
rc=$?
[ "$rc" != "0" ] && ok "库缺失时非零退出（rc=$rc）" || bad "库缺失却返回 0"
grep -q "缺预置 mbedtls 静态库" "$W/n_body.txt" && ok "报错文案含根因" || bad "报错文案不含根因"
grep -q "cross_aarch64.sh" "$W/n_body.txt" && ok "报错给出获取方式" || bad "报错未给获取方式"
grep -qE 'mbedtls/lib/libmbedcrypto\.a|zlib/lib/libz\.a|sqlite3/sqlite3\.o' "$W/n_body.txt" && bad "疑似静默退化到 x86_64 库" || ok "未静默退化"

echo "== ⑦ 编译轨可执行性诊断（跨架构解压即用的核心 UX · **负例**）=="
FAKE="$W/fake-pxc"; printf 'not-an-elf\n' > "$FAKE"; chmod +x "$FAKE"
PX_BUILD_ENGINE=c PX_PXC_BIN="$FAKE" $PX build "$W/h.px" > "$W/n7.txt" 2>&1; rc7=$?
[ "$rc7" != "0" ] && ok "不可执行编译器 ⇒ 非零退出（rc=$rc7）" || bad "不可执行编译器却返回 0"
grep -q "无法在本机执行" "$W/n7.txt" && ok "报错点明「无法在本机执行」" || bad "报错未点明根因"
grep -q "native_bootstrap.sh" "$W/n7.txt" && ok "报错给出修复命令（native_bootstrap.sh）" || bad "报错未给修复命令"
grep -q "PX_PXC_BIN" "$W/n7.txt" && ok "报错给出逃生舱 PX_PXC_BIN" || bad "报错未给逃生舱"

echo "== ⑧ 计划可断言编译轨（plan: pxc / pxc_run）=="
PX_HOST_ARCH=aarch64 $PX build --print-plan "$W/h.px" > "$W/p8.txt" 2>&1
has "$W/p8.txt" "plan: pxc=$ROOT/bootstrap/pxc_vm" "VM 轨编译器 = 入库 pxc_vm"
grep -q "^plan: pxc_sub=--emit-c" "$W/p8.txt" && ok "计划含 pxc_sub=--emit-c" || bad "计划缺 pxc_sub"
grep -q "^plan: pxc_run=" "$W/p8.txt" && ok "计划含 pxc_run（可执行性）" || bad "计划缺 pxc_run"
PX_BUILD_ENGINE=c $PX build --print-plan "$W/h.px" > "$W/p8c.txt" 2>&1
has "$W/p8c.txt" "plan: pxc=$ROOT/bootstrap/pxc" "C 轨编译器 = 入库 pxc"
grep -q "^plan: pxc_sub=build" "$W/p8c.txt" && ok "C 轨 pxc_sub=build" || bad "C 轨 pxc_sub 错"

echo "== ⑥+ 真机回归：按**本机宿主**默认档真编译真运行（x86_64 ⇒ lib+QUIC；aarch64 ⇒ lib-aarch64+no-quic）=="
mkdir -p "$W/b" && cp "$W/h.px" "$W/b/"
$PX build "$W/b/h.px" > "$W/build.log" 2>&1 && ok "编译成功（宿主 $(uname -m)）" || { bad "编译失败"; tail -5 "$W/build.log"; }
if [ -x "$W/b/build/h" ] && [ "$("$W/b/build/h")" = "hello" ]; then ok "产物运行输出正确"; else bad "产物运行异常"; fi
echo "   （宿主不是 x86_64 时，本条即「原生 aarch64 裸 px build 直接可用」的端到端证据）"

echo ""
if [ "$FAIL" = 0 ]; then echo "M159-HOSTARCH-VERIFY-OK"; exit 0; else echo "M159-HOSTARCH-VERIFY-FAIL（$FAIL 项）"; exit 1; fi
