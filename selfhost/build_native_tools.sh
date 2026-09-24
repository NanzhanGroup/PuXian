#!/usr/bin/env bash
# ============================================================
# build_native_tools.sh —— 原生（宿主架构）**12 件工具的唯一构建入口**（M201 建立）
# ------------------------------------------------------------
# 为什么有它（M201 缺陷 241 = 第三方 PX-DEF-035 的根因）：
#   原先 `ci.yml`（native-arm64 job）与 `release.yml`（aarch64 job）**各写一遍**
#   「12 件清单 + `PX_BUILD_ENGINE=c tools/px build`」的循环 —— 两处必然漂移，
#   而漂移的代价已经实测到：发布链用**自动裁剪**（`tools/px build` 默认按引用集剔模块）
#   现编**解释器**，而解释器是**泛化分派**（native 名在运行期按字符串查表）⇒
#   裁剪器"看不见"这些名字 ⇒ 把 zlib / sqlite / xml / aes / rsa / ed25519 / zip / ws
#   **整族**从 `bootstrap/pxi` 里裁掉（实测已发布 aarch64 包：`pxc` 有 19 个 `zlib_*`，
#   `pxi` **0 个**）。后果 = 官方 zlib 包在解释轨**完全不可用**，而 CI 只问"能不能跑
#   hello"⇒ 一直绿。
#
# 口径（唯一一处，别再复制到 YAML 里）：
#   · **泛化分派件必须 `--full`** —— 它把用户程序当数据来解释/执行，能力面 = 整语言；
#     目前只有 `pxi`（解释器）属于这一类。
#   · 其余件按引用集**自动裁剪**（那是**特性**：二进制更小；它们的输入不是用户程序）。
#   · `pxc` 由 `selfhost/native_bootstrap.sh` 产出（gcc-only 自举 + 自证），本脚本不碰它。
#
# 用法：
#   selfhost/build_native_tools.sh [--dir <安装目录>（默认 bootstrap）] [--no-check]
#                                  [--jobs N]
# 退出码：0=全部装好（默认还跑了能力面门）；1=失败
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

DIR="bootstrap"
DO_CHECK=1
JOBS="${JOBS:-$( (nproc 2>/dev/null || echo 4) )}"
while [ $# -gt 0 ]; do
    case "$1" in
        --dir) DIR="$2"; shift 2 ;;
        --no-check) DO_CHECK=0; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done
mkdir -p "$DIR"

# 件清单：`名字|入口源|额外 flag`（**flag 只给泛化分派件**，见上）
TOOLS="pxi|selfhost/interp.px|--full
pxl|selfhost/lexer.px|
pxpar|selfhost/parser.px|
pxfmt|tools/pxfmt.px|
pxbench|tools/pxbench.px|
pxcheck|tools/pxcheck.px|
pxdoc|tools/pxdoc.px|
pxlint|tools/pxlint.px|
pxlsp|tools/pxlsp.px|
pxmcp|tools/pxmcp.px|
pxtest|tools/pxtest.px|"

fail=0
echo "══ 原生工具构建（宿主 $(uname -m) · 安装到 $DIR/ · 解释器 --full）══"
while IFS='|' read -r name src flags; do
    [ -n "$name" ] || continue
    log="/tmp/m201_bnt_$name.log"
    # shellcheck disable=SC2086
    if ! ( cd "$ROOT" && PX_BUILD_ENGINE=c tools/px build $flags "$src" ) > "$log" 2>&1; then
        echo "❌ $name：现编失败（$src${flags:+ $flags}）"; tail -15 "$log" | sed 's/^/     /'; fail=1; continue
    fi
    out="$(dirname "$src")/build/$(basename "$src" .px)"
    if [ ! -x "$out" ]; then echo "❌ $name：缺产物 $out"; tail -5 "$log" | sed 's/^/     /'; fail=1; continue; fi
    cp -f "$out" "$DIR/$name"
    echo "✅ $DIR/$name  $(stat -c %s "$DIR/$name" 2>/dev/null || stat -f %z "$DIR/$name") 字节${flags:+ （$flags）}"
done <<< "$TOOLS"

# VM 轨两件是 x86_64 侧的 --emit-c 产物：非 x86_64 包内不该出现（否则用户拿到 Exec format error）
case "$(uname -m)" in
    x86_64) : ;;
    *) rm -f "$DIR/pxc_vm" "$DIR/pxi_vm"; echo "── 非 x86_64：已移除 VM 轨两件（不随包发布）" ;;
esac

if [ "$fail" != 0 ]; then echo "❌ 构建失败"; exit 1; fi
echo "✅ 12 件（含 pxc 之外的 11 件）已装入 $DIR/"

if [ "$DO_CHECK" = 1 ]; then
    echo "── 能力面门（解释器件必须全能力）—— examples/m201_interp_ffi/verify.sh"
    if bash "$ROOT/examples/m201_interp_ffi/verify.sh" --interp "$ROOT/$DIR/pxi" > /tmp/m201_bnt_check.log 2>&1; then
        tail -6 /tmp/m201_bnt_check.log
        echo "✅ 能力面门通过"
    else
        echo "::error::解释器件能力面门判红（PX-DEF-035 复发的现场）"
        sed 's/^/::error::ffi /' /tmp/m201_bnt_check.log
        exit 1
    fi
fi
exit 0
