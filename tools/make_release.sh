#!/usr/bin/env bash
# ============================================================
# tools/make_release.sh —— PuXian 编译器发布包构建（仓库治理 · 发布物分发）
# ------------------------------------------------------------
# 背景：PuXian 是开源项目，但编译器本体源码（selfhost/）之外的日常使用方
#       （如外部私有应用 ws-web 的维护者）只需"用编译器开发"，无需持有源码树。
#       本脚本打一个最小可独立开发发布包：
#         编译器二进制（bootstrap/pxc pxi pxl pxpar）
#       + 工具入口（tools/pxc pxpkg routegen cross_aarch64.sh cross_multiarch.sh）
#       + 构建必需 C 依赖（runtime/ 全树，含 mbedtls / sqlite3 / miniz /
#         ngtcp2 / openssl 静态库与头文件，x86_64 + aarch64 双架构）
#       + 标准库（stdlib/，import std.* 必需）
#       + LICENSE + 自动生成的 RELEASE.md
#       **不含** selfhost/（编译器 PuXian 源码）、docs/、capability/、
#       examples/、.github/、archive/、.git —— 本地无源码库、无写通道。
# 用法：
#   tools/make_release.sh [里程碑]       里程碑缺省取最近提交里的 Mxx（如 m57）
#   tools/make_release.sh --no-check     打包后跳过冒烟自检
#   tools/make_release.sh -o <路径>      指定输出 tarball 路径
# 版本来源（M62 起 tag 驱动）：
#   VER  = 最近 tag 的 vX.Y.Z 部分（git describe --tags --abbrev=0），无 tag 默认 0.1.0
#   里程碑 = 命令行参数 > tag 后缀（v0.1.0-m62 → m62）> 最近提交消息里的 Mxx
# 典型发布流：git tag v0.1.0-m62 && tools/make_release.sh  →  puxian-0.1.0-m62-<sha>.tar.gz
# 冒烟自检（默认开，--no-check 关）：解包到临时目录后验证
#   ① pxc --version  ② hello.px 编译(静态ELF)并运行  ③ hello.px 解释运行
#   ④ import std.semver 编译（验证 stdlib 定位 PX_STDLIB）
# ============================================================
set -euo pipefail

PXC_HOME="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PXC_HOME"

MILESTONE=""
NO_CHECK=0
OUT=""
while [ $# -gt 0 ]; do
    case "$1" in
        --no-check) NO_CHECK=1; shift ;;
        -o) OUT="$2"; shift 2 ;;
        -*) echo "未知选项: $1" >&2; exit 1 ;;
        *) MILESTONE="$1"; shift ;;
    esac
done

# ---- 版本来源（tag 驱动，M62）----
# VER 优先级：最近 tag vX.Y.Z[-mxx] > 默认 0.1.0（避免包版本与 tag 不一致）
VER="0.1.0"
TAG="$(git describe --tags --abbrev=0 2>/dev/null || true)"
if [ -n "$TAG" ]; then
    TVER="${TAG#v}"                 # v0.1.0-m62 → 0.1.0-m62
    TAG_VER="${TVER%%-*}"           # → 0.1.0
    [ -n "$TAG_VER" ] && VER="$TAG_VER"
    if [ -z "$MILESTONE" ] && [ "$TVER" != "${TVER%%-*}" ]; then
        # 未显式指定里程碑且 tag 带 -mxx 后缀 → tag 是发布决策，优先于 commit 推断
        MILESTONE="$(echo "${TVER#*-}" | tr 'A-Z' 'a-z')"
    fi
    echo "   版本源: tag $TAG"
fi
if [ -z "$MILESTONE" ]; then
    MILESTONE="$(git log -1 --pretty=%s | grep -o 'M[0-9][0-9]*' | head -1 || true)"
    [ -n "$MILESTONE" ] || MILESTONE="dev"
    MILESTONE="$(echo "$MILESTONE" | tr 'A-Z' 'a-z')"
fi
SHA="$(git rev-parse --short HEAD)"
NAME="puxian-${VER}-${MILESTONE}-${SHA}"
STAGE="/tmp/${NAME}.stage"
PKG="${OUT:-/tmp/${NAME}.tar.gz}"

echo "== 发布包构建: ${NAME} =="
echo "   里程碑: $MILESTONE | commit: $SHA | 输出: $PKG"

# ---- 1. staging 组装 ----
rm -rf "$STAGE"
mkdir -p "$STAGE/$NAME"

cp -r tools        "$STAGE/$NAME/tools"
cp -r bootstrap    "$STAGE/$NAME/bootstrap"
cp -r stdlib       "$STAGE/$NAME/stdlib"
cp -r runtime      "$STAGE/$NAME/runtime"
cp    LICENSE      "$STAGE/$NAME/LICENSE"
# 发布脚本自身不进发布包（依赖 git 仓库，且与"无源码树"目标冲突）
rm -f "$STAGE/$NAME/tools/make_release.sh" "$STAGE/$NAME/tools/install.sh"
# runtime 下的备份/杂物不进发布包
find "$STAGE/$NAME" -name '*.bak*' -delete
find "$STAGE/$NAME" -name '__pycache__' -type d -prune -exec rm -rf {} + 2>/dev/null || true

# ---- 2. 生成 RELEASE.md ----
cat > "$STAGE/$NAME/RELEASE.md" <<EOF
# PuXian ${VER}（${MILESTONE} · ${SHA}）

普贤语言编译器**发布包**（最小可独立开发集）。本包只含编译/运行所需的二进制与
构建依赖，**不含编译器本体 PuXian 源码**（selfhost/）与 git 仓库——仅供"使用
PuXian 开发应用"，不提供源码改动/推送通道。源码见开源仓库
\`github.com/NanzhanGroup/PuXian\`（改动请走 issue / PR）。

## 内容
| 路径 | 说明 |
|---|---|
| tools/pxc | 工具链入口（build/run/lex/parse/fmt/lint/doc/test/bench/lsp/mcp/--version/help） |
| tools/pxpkg | 包管理器（M45 registry） |
| tools/routegen / cross_aarch64.sh / cross_multiarch.sh | 路由生成 / aarch64 交叉库构建 / 多架构（aarch64·armv7·riscv64）交叉库构建（可选，M67） |
| bootstrap/pxc pxi pxl pxpar | 自举编译器 / 解释器 / lexer / parser 二进制 |
| bootstrap/pxfmt pxlint pxdoc pxtest pxbench pxlsp pxmcp pxcheck | 自举工具链（fmt/lint/doc/test/bench/lsp/mcp/diagnostics） |
| runtime/ | 构建必需 C 依赖（runtime*.c/h + mbedtls + miniz + sqlite3 + ngtcp2 + openssl，x86_64 与 aarch64 库均在） |
| stdlib/ | 标准库（import std.* 必需） |
| RELEASE.md / LICENSE | 本说明 / Apache-2.0 |

**不含**：selfhost/（编译器源码）、docs/、examples/、capability/、.git。

## 环境要求
- Linux x86_64；\`gcc\` + \`make\`（构建默认全量含 QUIC/H3，需链接 ngtcp2/openssl 静态库，
  本包已带；机器无需装这两库）
- 交叉编译 aarch64：\`aarch64-linux-musl-gcc\`（见下）

## 快速开始
\`\`\`bash
tar xzf ${NAME}.tar.gz && cd ${NAME}
./tools/pxc --version                      # 版本确认
./tools/pxc run hello.px                   # 解释运行
./tools/pxc build hello.px                 # 编译静态 ELF → hello/build/hello
./hello/build/hello                        # 直接运行
\`\`\`

> **stdlib 定位**：在解压目录（或子目录）内开发，\`import std.*\` 自动命中
> （候选路径含 \`./stdlib\`、\`../stdlib\`）。若在包外任意目录调用 pxc，请先
> \`export PX_STDLIB=<解压目录>/stdlib\`。

## 常用选项
- \`pxc build --no-quic <app.px>\`：裁剪 QUIC/H3（去掉 ngtcp2/openssl 链接，
  runtime 侧 -DPX_NO_QUIC），产物更小、无第三方 QUIC 依赖（嵌入式/边缘设备场景）。
- \`pxc build --cc aarch64-linux-musl-gcc --mbedtls-lib runtime/mbedtls/lib-aarch64
  --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o --no-quic <app.px>\`：
  aarch64 交叉编译（本包已含目标架构 mbedtls/sqlite3/zlib 静态库，直接可编）。
- armv7 / riscv64 交叉：先 \`tools/cross_multiarch.sh --arch armv7 --outdir <dir>\`（或
  \`--arch riscv64\`）现编目标库（本包已含脚本），再 \`pxc build --cc <arch>-linux-musl-gcc
  --mbedtls-lib <dir>/mbedtls/lib-<arch> --sqlite-obj <dir>/sqlite3/sqlite3-<arch>.o --no-quic <app.px>\`；
  riscv64 自动加 -no-pie（M67）。
- \`pxc help\`：完整用法。

## 能力面（本包随附）
fd 原语与 mmap 活映射（M57 边缘设备层）、HTTP/1.1·2·3 + QUIC、WebSocket、
vhost 多站点、路由、SQLite、AES/RSA/XML/ZIP、正则、内置 json/xml 解析、
import std.* 标准库等。编译模式（build）覆盖全部能力；解释器 pxi 为 Mini 子集
（不含 vhost/quic 等重型 builtin，日常脚本与算法调试够用）。
EOF

# ---- 3. 打 tar.gz ----
tar -C "$STAGE" -czf "$PKG" "$NAME"
SZ="$(stat -c %s "$PKG")"
echo "   ✅ tarball: $PKG （$SZ 字节）"
# M71-S4：sha256sums.txt 与 tarball 同目录（tools/install.sh 一键安装校验用）
( cd "$(dirname "$PKG")" && sha256sum "$(basename "$PKG")" > sha256sums.txt )
echo "   ✅ sha256sums: $(dirname "$PKG")/sha256sums.txt"
rm -rf "$STAGE"

# ---- 4. 冒烟自检 ----
if [ "$NO_CHECK" = "1" ]; then
    echo "  （--no-check：跳过冒烟自检）"
    exit 0
fi

CHK="/tmp/${NAME}.check"
rm -rf "$CHK"
mkdir -p "$CHK"
tar -C "$CHK" -xzf "$PKG"
cd "$CHK/$NAME"

echo "== 冒烟自检 =="
fail=0

# ① 版本
V="$(./tools/pxc --version 2>&1)" || { echo "  ❌ pxc --version 失败"; fail=1; }
echo "  ① pxc --version → $V"

# ② hello 编译 + 运行
# 注意：编译/解释两模式的输出全局函数均为 print（println 非 builtin）
# 自检统一用 --no-quic（更小更快、无第三方 QUIC 依赖；全量链接路径单独手工验证）
cat > hello.px <<'PX'
def main():
    print("hello from release pkg\n")
PX
./tools/pxc build --no-quic hello.px >/dev/null 2>&1 || { echo "  ❌ build hello 失败"; fail=1; }
if [ -x build/hello ]; then
    SZ2="$(stat -c %s build/hello)"
    O="$(./build/hello 2>&1)"
    echo "  ② build hello → 静态ELF ${SZ2}B, 运行输出: $O"
    [ "$O" = "hello from release pkg" ] || { echo "  ❌ 运行输出不符"; fail=1; }
else
    echo "  ❌ build 产物缺失"; fail=1
fi

# ③ 解释运行
O2="$(./tools/pxc run hello.px 2>&1)"
echo "  ③ pxc run hello → $O2"
[ "$O2" = "hello from release pkg" ] || { echo "  ❌ 解释运行输出不符"; fail=1; }

# ④ import std.collections 编译（stdlib 定位：包根 ./stdlib 相对命中）
#    只 import + 固定输出——验证"import 解析 + stdlib 定位 + 模块可编译"，
#    不赌 std API 语义；模块本体编译通过即证明 stdlib 随包可用。
cat > usesem.px <<'PX'
import std.collections
def main():
    print("stdlib import ok\n")
PX
if ./tools/pxc build --no-quic usesem.px >/dev/null 2>&1 && [ -x build/usesem ]; then
    O3="$(./build/usesem 2>&1)"
    echo "  ④ import std.collections → $O3"
    [ "$O3" = "stdlib import ok" ] || { echo "  ❌ 输出不符"; fail=1; }
else
    echo "  ❌ import std.* 编译失败（stdlib 未定位）"; fail=1
fi

# ⑤ 包外任意目录调用 + PX_STDLIB env（模拟用户项目目录在包外）
mkdir -p /tmp/pxuser && cd /tmp/pxuser
cat > proj.px <<'PX'
import std.webroute
def main():
    print("webroute import ok\n")
PX
export PX_STDLIB="$CHK/$NAME/stdlib"
if "$CHK/$NAME/tools/pxc" build --no-quic proj.px >/dev/null 2>&1 && [ -x build/proj ]; then
    O4="$(./build/proj 2>&1)"
    echo "  ⑤ 包外+PX_STDLIB(webroute) → $O4"
    [ "$O4" = "webroute import ok" ] || { echo "  ❌ 输出不符"; fail=1; }
else
    echo "  ❌ 包外 PX_STDLIB 编译失败"; fail=1
fi

# ⑥ M64/M65 工具链自举版存在性（pxfmt/pxlint/pxdoc/pxtest/pxbench + pxlsp/pxmcp/pxcheck）
TLS=""
# 注意：⑤ 之后 cwd 已切到 /tmp/pxuser，此处必须用绝对路径（$CHK/$NAME）
BCHK="$CHK/$NAME/bootstrap"
for t in pxfmt pxlint pxdoc pxtest pxbench pxlsp pxmcp pxcheck; do
    if [ -x "$BCHK/$t" ] && "$BCHK/$t" --version >/dev/null 2>&1; then
        TLS="$TLS $t"
    else
        echo "  ❌ bootstrap/$t --version 失败"; fail=1
    fi
done
echo "  ⑥ 工具链自举版 →$TLS"

cd / && rm -rf /tmp/pxuser "$CHK"
echo "== 自检结果: $([ "$fail" = 1 ] && echo '❌ FAIL' || echo '✅ ALL OK') =="
echo "发布包就绪: $PKG"
exit "$fail"
