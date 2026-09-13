#!/usr/bin/env bash
# ============================================================
# examples/m65_lsp/verify.sh —— M65 LSP 里程碑验收
# ------------------------------------------------------------
# S1：双模式 jsonrpc 回环 33 PASS + os_spawn_capture 冒烟 5 PASS
# S2b：pxcheck --semantic（M115 / qg-issue 65）：语义诊断上报 + 正例不误报
#      + 编译器缺失显式上报 L000（不静默放行）
# S2：pxlsp 端到端（python3 模拟标准 LSP client 双向管道）：
#     initialize → didOpen(didChange/didSave/didClose) → publishDiagnostics
#     → shutdown → exit，17 断言全绿
# S3：completion / definition / hover 增强 e2e：
#     demo_s3.px（def/struct/enum/trait/impl/var/局部/文档注释）+ 真实文件
#     selfhost/astdump.px + 错误输入不崩，39 断言全绿
# 依赖：python3 + bootstrap/pxlsp + bootstrap/pxcheck（自举产物，随仓库提交）
# ============================================================
set -u
cd "$(dirname "$0")/../.."
HERE=examples/m65_lsp
BOOT=bootstrap
PXC=tools/pxc          # 构建入口：`px build <file>` → <dir>/build/<name>
FAIL=0

echo "== M65-S1 verify =="

# 1) 编译 jsonrpc_core 自测 + spawncap 冒烟
#    ⚠️ 必须走 tools/pxc（用户入口），**不能**用 bootstrap/pxc：
#       `bootstrap/pxc build <f>` 只把 C 源码写到 stdout、**不产出二进制**，
#       故 $HERE/build/<name> 从不生成 —— 本脚本在干净检出上一直失败，
#       却被 CI「工具自测」步的 `&&` 链吞掉（qg-issue 64）。
[ -x "$PXC" ] || { echo "FAIL: 缺 tools/pxc（构建入口不可执行）"; exit 1; }
$PXC build --no-quic $HERE/jsonrpc_selftest.px >/dev/null 2>&1 || { echo "FAIL: build jsonrpc_selftest"; exit 1; }
$PXC build --no-quic $HERE/spawncap_selftest.px >/dev/null 2>&1 || { echo "FAIL: build spawncap_selftest"; exit 1; }

# 2) 双模式运行并断言
for MODE in pxi compiled; do
  if [ "$MODE" = pxi ]; then
    JR_OUT=$($BOOT/pxi $HERE/jsonrpc_selftest.px 2>&1)
    SP_OUT=$($BOOT/pxi $HERE/spawncap_selftest.px 2>&1)
  else
    JR_OUT=$($HERE/build/jsonrpc_selftest 2>&1)
    SP_OUT=$($HERE/build/spawncap_selftest 2>&1)
  fi
  ok=1
  echo "$JR_OUT" | grep -q "33 PASS, 0 FAIL" || { echo "FAIL[$MODE]: jsonrpc_selftest 未全绿"; echo "$JR_OUT"; FAIL=1; ok=0; }
  echo "$SP_OUT" | grep -q "5 PASS, 0 FAIL" || { echo "FAIL[$MODE]: spawncap_selftest 未全绿"; echo "$SP_OUT"; FAIL=1; ok=0; }
  # 只有真的全绿才打印 PASS（原先无条件打印，门会自我粉饰）
  [ "$ok" = "1" ] && echo "PASS[$MODE]: jsonrpc_selftest + spawncap_selftest"
done

echo ""
echo "== M65-S2 verify =="

# S2：pxlsp 端到端握手 + 诊断（需自举产物 pxlsp/pxcheck 与 python3）
if [ ! -x "$BOOT/pxlsp" ] || [ ! -x "$BOOT/pxcheck" ]; then
  echo "FAIL: 缺 bootstrap/pxlsp 或 bootstrap/pxcheck（先 tools/pxc build 后 cp）"
  FAIL=1
elif ! command -v python3 >/dev/null 2>&1; then
  echo "SKIP: 无 python3（S2 client 握手验证跳过）"
else
  LSP_OUT=$(python3 "$HERE/lsp_client_s2.py" "$BOOT/pxlsp" "$BOOT/pxcheck" 2>&1)
  echo "$LSP_OUT" | grep -q "ALL PASS" || { echo "FAIL: S2 client 未全绿"; echo "$LSP_OUT"; FAIL=1; }
  echo "PASS: S2 client 端到端握手 + 诊断全绿"
fi

echo ""
echo "== M65-S3 verify =="

# S3：completion/definition/hover e2e（demo + 真实文件 + 错误输入不崩）
if [ ! -x "$BOOT/pxlsp" ] || [ ! -x "$BOOT/pxcheck" ]; then
  echo "FAIL: 缺 bootstrap/pxlsp 或 bootstrap/pxcheck（S3 无法运行）"
  FAIL=1
elif ! command -v python3 >/dev/null 2>&1; then
  echo "SKIP: 无 python3（S3 client 验证跳过）"
else
  S3_OUT=$(python3 "$HERE/lsp_client_s3.py" "$BOOT/pxlsp" "$BOOT/pxcheck" "$PWD" 2>&1)
  echo "$S3_OUT" | grep -q "ALL PASS" || { echo "FAIL: S3 client 未全绿"; echo "$S3_OUT"; FAIL=1; }
  echo "PASS: S3 client completion/definition/hover 全绿"
fi

echo ""
echo "== M65-S2b verify：语义诊断（--semantic · M115 / qg-issue 65）=="

# 判据（不设暗门）：
#   ① 不带 --semantic：维持原契约（lint 层）—— 语义负例仍输出 []/rc=0
#   ② 带 --semantic：必须出现编译器诊断（E3002）且 rc=1
#   ③ 正例在 --semantic 下仍为 []/rc=0
#   ④ 编译器不可用：必须显式上报 L000（W），不静默放行
BAD=selfhost/cases_bad/codegen_b01_immutable.px
GOOD=examples/hello.px
S_SEMOK=1
if [ ! -x "$BOOT/pxcheck" ] || [ ! -x "$BOOT/pxc_vm" ]; then
  echo "SKIP: 缺 bootstrap/pxcheck 或 bootstrap/pxc_vm（语义门跳过）"
else
  OUT1=$("$BOOT/pxcheck" "$BAD" 2>&1); RC1=$?
  [ "$RC1" = "0" ] && [ "$OUT1" = "[]" ] || { echo "FAIL: 不带 --semantic 的契约被改动（rc=$RC1 out=$OUT1）"; S_SEMOK=0; }
  OUT2=$("$BOOT/pxcheck" "$BAD" --semantic 2>&1); RC2=$?
  echo "$OUT2" | grep -q '"code":"E3002"' || { echo "FAIL: --semantic 未报 E3002（out=$OUT2）"; S_SEMOK=0; }
  [ "$RC2" = "1" ] || { echo "FAIL: --semantic 语义错误下 rc=$RC2（应为 1）"; S_SEMOK=0; }
  OUT3=$("$BOOT/pxcheck" "$GOOD" --semantic 2>&1); RC3=$?
  [ "$RC3" = "0" ] && [ "$OUT3" = "[]" ] || { echo "FAIL: 正例在 --semantic 下 rc=$RC3 out=$OUT3"; S_SEMOK=0; }
  OUT4=$(PX_SEMANTIC_BIN=/nonexistent/pxc_vm "$BOOT/pxcheck" "$GOOD" --semantic 2>&1)
  echo "$OUT4" | grep -q '"code":"L000"' || { echo "FAIL: 编译器缺失时未显式上报 L000（out=$OUT4）"; S_SEMOK=0; }
  [ "$S_SEMOK" = "1" ] && echo "PASS: S2b --semantic（E3002 上报 / 正例不误报 / 编译器缺失显式上报）"
fi
[ "$S_SEMOK" = "1" ] || FAIL=1

if [ $FAIL -eq 0 ]; then
  echo ""
  echo "== M65-S1/S2/S2b/S3 verify: ALL PASS =="
  exit 0
fi
echo ""
echo "== M65-S1/S2/S2b/S3 verify: FAILED =="
exit 1
