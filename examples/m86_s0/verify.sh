#!/bin/bash
# M86-S0 (qg-issue 25) 命令正名 px + help 补齐验证
#   官方命令名 px（tools/px），pxc 为兼容别名（tools/pxc → symlink px）
#   验证：--version/--help 完整性（M85 全开关 + px 全子命令）/ 别名等价 /
#         build 零漂移（默认 9.0M + --no-quic 3.93M）/ 安装链路（spec/install.sh/make_release）双装
#   依赖：tools/px（M86-S0 版）+ bootstrap/pxc（仓库自带）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/px
PXC=../../tools/pxc
rm -rf build; mkdir -p build
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

echo "== [1/9] px --version（官方名）=="
V=$($PX --version 2>&1)
if echo "$V" | grep -qE "^px 0.1.0(-|$)"; then ok "--version → $V"; else bad "--version → $V"; fi

echo "== [2/9] px --help 含 M85 全开关 + px 全子命令 + M86 别名说明 =="
H=$($PX --help 2>&1)
C=0
for sw in --min --no-quic --no-sqlite --no-ws --no-zip --no-xml --no-aes --no-rsa --no-ed25519 --no-route --no-zlib --no-h2; do
    echo "$H" | grep -q -- "$sw" && C=$((C+1)) || echo "    缺: $sw"
done
[ "$C" = 12 ] && ok "help 含 12 个裁剪开关（--min + 11×--no-*）" || bad "help 裁剪开关 $C/12"
C=0
for sub in "px build " "px run " "px lex " "px parse " "px fmt " "px lint " "px doc " "px test " "px bench " "px lsp" "px mcp " "px --version" "px help"; do
    echo "$H" | grep -qF "$sub" && C=$((C+1)) || echo "    缺子命令: $sub"
done
[ "$C" = 13 ] && ok "help 含 px 全 13 子命令" || bad "help 子命令 $C/13"
echo "$H" | grep -q "pxc 为兼容别名" && ok "help 含 pxc 别名说明" || bad "help 缺 pxc 别名说明"

echo "== [3/9] pxc 别名等价（symlink → px）=="
if [ -L "$PXC" ] && [ "$(readlink "$PXC")" = "px" ]; then ok "tools/pxc 是 symlink → px"; else bad "tools/pxc 非 symlink"; fi
V2=$($PXC --version 2>&1)
[ "$V" = "$V2" ] && ok "pxc --version 与 px 等价" || bad "pxc --version 不等价: $V2"

echo "== [4/9] px run 解释执行 =="
O=$($PX run hello.px 2>&1)
[ "$O" = "m86s0 hello" ] && ok "px run → $O" || bad "px run → $O"

echo "== [5/9] px build --full 全能力零漂移（9.0M 基线；M86-S2 起裸 build=自动最小，全能力用 --full）=="
$PX build --full hello.px >/tmp/m86s0.log 2>&1 || { bad "--full build"; tail -3 /tmp/m86s0.log; }
SZ=$(stat -c %s build/hello 2>/dev/null || echo 0)
if [ "$SZ" -ge 8900000 ] && [ "$SZ" -le 9150000 ]; then ok "--full 体积 $SZ（9.0M 基线内）"; else bad "默认体积 $SZ 偏离基线"; fi
./build/hello >/dev/null 2>&1 && ok "默认产物运行" || bad "默认产物运行"

echo "== [6/9] px build --full --no-quic（显式 quic 裁剪 + 全能力其余，基线 3929808）=="
$PX build --full --no-quic hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
if [ "$SZ" -ge 3800000 ] && [ "$SZ" -le 4050000 ]; then ok "--full --no-quic $SZ"; else bad "--full --no-quic $SZ 期望 ~3929808"; fi

echo "== [7/9] 历史脚本兼容：tools/pxc（别名路径）build 可用 =="
$PXC build --full hello.px >/dev/null 2>&1 && ok "pxc 别名 build 成功" || bad "pxc 别名 build 失败"

echo "== [8/9] rpm spec 双装（/usr/bin/px + /usr/bin/pxc → px）=="
S=../../packaging/puxian.spec
grep -q 'ln -s %{_datadir}/puxian/tools/px %{buildroot}%{_bindir}/px' "$S" && \
grep -q 'ln -s px %{buildroot}%{_bindir}/pxc' "$S" && \
grep -q '%{_bindir}/px$' "$S" && grep -q '%{_bindir}/pxc' "$S" \
    && ok "spec 双装 px + pxc 软链" || bad "spec 双装断言失败"

echo "== [9/9] install.sh / make_release.sh 双软链与 RELEASE 模板 px 化 =="
I=../../tools/install.sh
grep -q 'ln -sf "$DEST/tools/px" "$BIN_DIR/px"' "$I" && \
grep -q 'ln -sf "$DEST/tools/pxc" "$BIN_DIR/pxc"' "$I" && ok "install.sh 双软链" || bad "install.sh 双软链"
M=../../tools/make_release.sh
grep -q '| tools/px（pxc 兼容别名） |' "$M" && grep -q './tools/px --version' "$M" && \
grep -q './tools/px build' "$M" && ok "make_release RELEASE 模板 px 化" || bad "make_release px 化"

echo
echo "M86-S0 verify: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
