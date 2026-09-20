#!/usr/bin/env bash
# ============================================================
# tools/make_release.sh —— PuXian 编译器发布包构建（仓库治理 · 发布物分发）
# ------------------------------------------------------------
# 背景：PuXian 是开源项目，但编译器本体源码（selfhost/）之外的日常使用方
#       （如外部私有应用 ws-web 的维护者）只需"用编译器开发"，无需持有源码树。
#       本脚本打一个最小可独立开发发布包：
#         编译器二进制（bootstrap/pxc pxi pxl pxpar）
#       + 工具入口（tools/px（pxc 兼容别名）+ pxpkg routegen cross_aarch64.sh cross_multiarch.sh）
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
#   ① px --version  ② hello.px 编译(静态ELF)并运行  ③ hello.px 解释运行
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

# ---- Issue 56：按 git 索引白名单复制（**不再**整目录 cp -r）----
#   为什么必须白名单：`cp -r tools` 会把 .gitignore 的 `tools/build/`（本机旧二进制
#   + sqlite3.o + openssl/lib/*.a，实测 93MB/198 件）一并打进包 ⇒ 同一 tag 在两台机器
#   打出**两个不同 sha256**（CI 资产 392 件 vs 本机 596 件）⇒ 包的哈希不能当发行身份。
#   改为「git 索引即发布物内容」：`git ls-files -s` 同时给出路径与 mode（100644/100755/
#   120000），逐件复制并**归一 mode** ⇒ 本机与 CI 检出在结构上必然一致。
copy_tracked() {
    local d="$1" rec meta mode path dst
    git ls-files -s -z -- "$d" | while IFS= read -r -d '' rec; do
        meta="${rec%%$'\t'*}"; path="${rec#*$'\t'}"
        mode="${meta%% *}"
        dst="$STAGE/$NAME/$path"
        mkdir -p "$(dirname "$dst")"
        cp -Pp -- "$path" "$dst"          # -P：保持符号链接本体（tools/pxc -> px）
        case "$mode" in
            100755) chmod 755 "$dst" ;;
            100644) chmod 644 "$dst" ;;
            120000) : ;;                  # 符号链接：-P 已复制链接本体
            *)      echo "   ⚠ 非常规 git mode $mode: $path" ;;
        esac
    done
}
git rev-parse --is-inside-work-tree >/dev/null 2>&1 \
    || { echo "❌ 不在 git 工作树内：发布物内容取自 git 索引，请在仓库内运行" >&2; exit 2; }
for _d in tools bootstrap stdlib runtime; do copy_tracked "$_d"; done
copy_tracked LICENSE
# 发布脚本自身不进发布包（依赖 git 仓库，且与"无源码树"目标冲突）
rm -f "$STAGE/$NAME/tools/make_release.sh" "$STAGE/$NAME/tools/install.sh"
# 组装自检（Issue 56 防回归）：白名单复制后，构建产物在结构上不可能出现
[ ! -e "$STAGE/$NAME/tools/build" ] \
    || { echo "❌ 发布包夹带 tools/build/（Issue 56 回归：白名单复制被绕过）" >&2; exit 1; }
echo "   组装: $(find "$STAGE/$NAME" -type f | wc -l) 件（= git 跟踪集合，已剔除 tools/build 等构建产物）"
# runtime 下的备份/杂物不进发布包
find "$STAGE/$NAME" -name '*.bak*' -delete
find "$STAGE/$NAME" -name '__pycache__' -type d -prune -exec rm -rf {} + 2>/dev/null || true

# ---- 2a. 机器可读版本号（M87：px --version 读取；源码仓无此文件 → px 显示语义版 0.1.0）----
echo "${TVER:-${VER}-${MILESTONE}}" > "$STAGE/$NAME/VERSION"

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
| tools/px（pxc 兼容别名） | 工具链入口（build/run/lex/parse/fmt/lint/doc/test/bench/lsp/mcp/--version/help） |
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
- **aarch64 / riscv64 原生用户注意（M159）**：本包 \`bootstrap/*\` 是 **x86_64 件**（在 arm 上直接
  \`Exec format error\`）⇒ 请改用并列发布的 \`puxian-bootstrap-aarch64-<tag>.tar.gz\`
  （原生 arm64 runner 从源码现编 + 自证）；或在本机自举整套：
  \`./selfhost/native_bootstrap.sh --install\`（仅需 gcc，源码见仓库）
- 本包 \`tools/px\` 已做**宿主架构自适应**：非 x86_64 宿主自动取 \`runtime/mbedtls/lib-<arch>\`、
  \`sqlite3-<arch>.o\`、\`zlib/lib-<arch>\` 并自动 \`--no-quic\`（该平台无预编译 ngtcp2/openssl）；
  \`px build --print-plan\` 可打印解析后的构建计划（含编译轨与可执行性）

## 快速开始
\`\`\`bash
tar xzf ${NAME}.tar.gz && cd ${NAME}
./tools/px --version                      # 版本确认
./tools/px run hello.px                   # 解释运行
./tools/px build hello.px                 # 编译静态 ELF → hello/build/hello
./hello/build/hello                        # 直接运行
\`\`\`

> **stdlib 定位**：在解压目录（或子目录）内开发，\`import std.*\` 自动命中
> （候选路径含 \`./stdlib\`、\`../stdlib\`）。若在包外任意目录调用 px（pxc 别名等价），请先
> \`export PX_STDLIB=<解压目录>/stdlib\`。

## 常用选项
- \`px build --no-quic <app.px>\`：裁剪 QUIC/H3（去掉 ngtcp2/openssl 链接，
  runtime 侧 -DPX_NO_QUIC），产物更小、无第三方 QUIC 依赖（嵌入式/边缘设备场景）。
- \`px build --cc aarch64-linux-musl-gcc --mbedtls-lib runtime/mbedtls/lib-aarch64
  --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o --no-quic <app.px>\`：
  aarch64 交叉编译（本包已含目标架构 mbedtls/sqlite3/zlib 静态库，直接可编）。
- armv7 / riscv64 交叉：先 \`tools/cross_multiarch.sh --arch armv7 --outdir <dir>\`（或
  \`--arch riscv64\`）现编目标库（本包已含脚本），再 \`px build --cc <arch>-linux-musl-gcc
  --mbedtls-lib <dir>/mbedtls/lib-<arch> --sqlite-obj <dir>/sqlite3/sqlite3-<arch>.o --no-quic <app.px>\`；
  riscv64 自动加 -no-pie（M67）。
- \`px help\`：完整用法。

## 能力面（本包随附）
fd 原语与 mmap 活映射（M57 边缘设备层）、HTTP/1.1·2·3 + QUIC、WebSocket、
vhost 多站点、路由、SQLite、AES/RSA/XML/ZIP、正则、内置 json/xml 解析、
import std.* 标准库等。编译模式（build）覆盖全部能力；解释器 pxi 为 Mini 子集
（不含 vhost/quic 等重型 builtin，日常脚本与算法调试够用）。
EOF

# ---- 3. 打 tar.gz（Issue 56：位级可复现 —— 固定排序/属主/mtime）----
#   确定性四要素：① 文件集合（git 索引）② 权限（git mode）③ 属主（固定 0:0）
#   ④ mtime（固定为本次 commit 时间）⇒ 同一 commit **在任意机器、任意时刻**打包，
#   sha256 相同 ⇒ 「包的哈希」可以正式作为发行身份（这是 Issue 56 的验收目标）。
#   注：这是**一次性的资产字节变更**（旧资产属主是 runner / mtime 是打包时刻），
#       不影响解包与 tools/install.sh 的 sha256sums.txt 校验。
TAR_EPOCH="$(git log -1 --format=%ct 2>/dev/null || echo 0)"
# ---- 兼容探测（qg-issue 68）：--sort=name 是 GNU tar 1.28+ 才有的选项 ----
#   实况 2026-09-14：release.yml 的 rpm-build-7 job 在 centos:7 容器里构建，该镜像自带
#   tar 1.26 ⇒ `--sort=name` 属未知长选项，glibc argp 直接以 **64**（EX_USAGE）退出
#   ⇒ make_release.sh 起 `set -e` 一并退出 64 ⇒ el7 RPM 轨整条红。
#   故此处逐项探测、按能力组装；不支持的选项**不加**，但**必须显式告警**（不静默降级）。
tar_supports() {                        # $1=待测选项；在空目录上试建一个最小包
    tar -C "$TAR_PROBE" "$1" -cf "$TAR_PROBE/p.tgz" f >/dev/null 2>&1
}
TAR_PROBE="$(mktemp -d)"; : > "$TAR_PROBE/f"
TAR_OPTS=(--owner=0 --group=0 --numeric-owner)
if tar_supports "--mtime=@0"; then
    TAR_OPTS+=(--mtime="@${TAR_EPOCH}")          # ④ mtime 固定为本次 commit 时间
else
    echo "   ⚠ 本机 tar 不支持 --mtime（$(tar --version 2>/dev/null | head -1)）：包内 mtime 非固定值" >&2
fi
if tar_supports "--sort=name"; then
    TAR_OPTS+=(--sort=name)                      # ① 文件顺序按名字固定
else
    echo "   ⚠ 本机 tar 不支持 --sort=name（$(tar --version 2>/dev/null | head -1)）：" >&2
    echo "     文件顺序改由 readdir 决定 ⇒ 该 tar 下打出的包 sha256 不保证跨机器一致。" >&2
    echo "     影响面：仅 el7 RPM 轨的 rpm 源 tarball（对外发布件走 el9 的现代 tar，仍位级可复现）。" >&2
fi
rm -rf "$TAR_PROBE"
tar -C "$STAGE" "${TAR_OPTS[@]}" -czf "$PKG" "$NAME"
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
V="$(./tools/px --version 2>&1)" || { echo "  ❌ px --version 失败"; fail=1; }
echo "  ① px --version → $V"

# ② hello 编译 + 运行
# 注意：编译/解释两模式的输出全局函数均为 print（println 非 builtin）
# 自检统一用 --no-quic（更小更快、无第三方 QUIC 依赖；全量链接路径单独手工验证）
cat > hello.px <<'PX'
def main():
    print("hello from release pkg\n")
PX
./tools/px build --no-quic hello.px >/dev/null 2>&1 || { echo "  ❌ build hello 失败"; fail=1; }
if [ -x build/hello ]; then
    SZ2="$(stat -c %s build/hello)"
    O="$(./build/hello 2>&1)"
    echo "  ② build hello → 静态ELF ${SZ2}B, 运行输出: $O"
    [ "$O" = "hello from release pkg" ] || { echo "  ❌ 运行输出不符"; fail=1; }
else
    echo "  ❌ build 产物缺失"; fail=1
fi

# ③ 解释运行
O2="$(./tools/px run hello.px 2>&1)"
echo "  ③ px run hello → $O2"
[ "$O2" = "hello from release pkg" ] || { echo "  ❌ 解释运行输出不符"; fail=1; }

# ④ import std.collections 编译（stdlib 定位：包根 ./stdlib 相对命中）
#    只 import + 固定输出——验证"import 解析 + stdlib 定位 + 模块可编译"，
#    不赌 std API 语义；模块本体编译通过即证明 stdlib 随包可用。
cat > usesem.px <<'PX'
import std.collections
def main():
    print("stdlib import ok\n")
PX
if ./tools/px build --no-quic usesem.px >/dev/null 2>&1 && [ -x build/usesem ]; then
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
if "$CHK/$NAME/tools/px" build --no-quic proj.px >/dev/null 2>&1 && [ -x build/proj ]; then
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
