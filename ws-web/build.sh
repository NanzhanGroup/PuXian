#!/usr/bin/env bash
# ============================================================
# ws-web 一键构建（编译为静态二进制，无需 Rust 工具链）
# ============================================================
set -e
cd "$(dirname "$0")/.."
./tools/pxc build ws-web/main.px
mv -f ws-web/build/main ws-web/build/ws-web 2>/dev/null || true
echo "✅ 产物: ws-web/build/ws-web （$(du -h ws-web/build/ws-web | cut -f1)）"
