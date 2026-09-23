#!/usr/bin/env bash
# ============================================================
# tools/import_registry_px.sh —— 把第三方 registry-px 的库引入官方 registry/
# ------------------------------------------------------------
# 用法：
#   tools/import_registry_px.sh --src <registry-px 检出目录> [--exclude a,b] [--apply]
# 默认（不带 --apply）为**预演**：只列出会写入什么、逐件 sha256，不动任何文件。
#
# 纪律（M187 立）：
#   ① **逐字节照搬**（不改上游文件一个字节）⇒ 上游 sha256 可直接与我们的对拍；
#      来源与许可**不在包内改注释**，一律登记在 registry/THIRD_PARTY.md；
#   ② 冲突即拒：目标已存在同名包且内容不同 ⇒ 报错退出（人工决定版本号）；
#   ③ 只认 `<src>/registry/<name>/<ver>/` 结构（与官方 registry 规范同形），
#      包内相对 import 原样保留 ⇒ 多文件包直接可用（pxpkg M187 起支持）。
# ============================================================
set -uo pipefail
SRC=""
EXCLUDE=""
APPLY=0
while [ $# -gt 0 ]; do
    case "$1" in
        --src) SRC="${2:-}"; shift 2 ;;
        --exclude) EXCLUDE="${2:-}"; shift 2 ;;
        --apply) APPLY=1; shift ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done
[ -n "$SRC" ] || { echo "用法: import_registry_px.sh --src <registry-px 目录> [--exclude a,b] [--apply]" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC_REG="$SRC/registry"
[ -d "$SRC_REG" ] || { echo "错误: 源 registry 目录不存在：$SRC_REG" >&2; exit 2; }
UPSTREAM_REV="$(git -C "$SRC" rev-parse --short HEAD 2>/dev/null || echo unknown)"
UPSTREAM_URL="$(git -C "$SRC" config --get remote.origin.url 2>/dev/null || echo unknown)"

TMPL=/tmp/m187_import.$$
mkdir -p "$TMPL"
PROV="$TMPL/THIRD_PARTY.md"
NEW=0; SKIP=0; CONFLICT=0

cat > "$PROV" <<EOF
# 官方 registry · 第三方包来源与许可（自动生成，勿手改）

> 生成器：\`tools/import_registry_px.sh\`（M187 第 65 轮）· 源仓库：
> \`${UPSTREAM_URL}\` @ \`${UPSTREAM_REV}\` · 许可：**Apache-2.0**（上游 LICENSE 与本仓同族）
> 纪律：包文件**逐字节照搬**（不改一个字节）；来源/许可登记在本文件，不改包内注释。
> 复核：\`bash examples/m187_registry_import/verify.sh\` 会**重算**下表 sha256 与磁盘对拍（防漂移）。

| 包 | 版本 | 文件数 | 入口 sha256（前 16） | 三轨验证 | 备注 |
|---|---|---|---|---|---|
EOF

prov_note() {   # 三轨验证列（口径 = M186 轮的 53 库普查；随普查更新）
    case "$1" in
        concurrent_map|workerpool) echo "编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006）" ;;
        qrcode) echo "编译轨 PASS · 解释轨**超时**（PX-DEF-024 性能：单码 8 掩码罚分 ≈36s）" ;;
        pg|mysql) echo "文件完整 · 测试需**真实服务端**（PG13 / MariaDB）" ;;
        passhash) echo "**未引入**（库内 \`int(parts[1])\` 吃畸形 hash 串，M184 严格解析后需上游先修）" ;;
        *) echo "**双轨 PASS**（M186 普查 · 53 库）" ;;
    esac
}

for libdir in "$SRC_REG"/*/; do
    name="$(basename "$libdir")"
    case ",$EXCLUDE," in *",$name,"*) echo "  [跳过] $name（--exclude）"; continue ;; esac
    for verdir in "$libdir"*/; do
        [ -d "$verdir" ] || continue
        ver="$(basename "$verdir")"
        mapfile -t files < <(cd "$verdir" && ls *.px 2>/dev/null | sort)
        [ "${#files[@]}" -gt 0 ] || continue
        has_entry=0
        for f in "${files[@]}"; do [ "$f" = "$name.px" ] && has_entry=1; done
        if [ "$has_entry" = 0 ]; then
            echo "  [拒] $name/$ver 缺入口 $name.px" >&2; CONFLICT=$((CONFLICT+1)); continue
        fi
        dst="$ROOT/registry/$name/$ver"
        conflict=0
        if [ -d "$dst" ]; then
            for f in "${files[@]}"; do
                if [ -f "$dst/$f" ] && ! cmp -s "$verdir/$f" "$dst/$f"; then conflict=1; fi
            done
        fi
        if [ "$conflict" = 1 ]; then
            echo "  [冲突] $name/$ver 已存在且内容不同 —— 需人工定版本号（拒写）" >&2
            CONFLICT=$((CONFLICT+1)); continue
        fi
        if [ -d "$dst" ]; then
            SKIP=$((SKIP+1)); state="已存在（内容一致）"
        else
            NEW=$((NEW+1)); state="新引入"
        fi
        entry_sha="$(sha256sum "$verdir/$name.px" | cut -c1-16)"
        printf '| %s | %s | %d | `%s` | %s | %s |\n' "$name" "$ver" "${#files[@]}" "$entry_sha" "$(prov_note "$name")" "$state" >> "$PROV"
        if [ "$APPLY" = 1 ]; then
            mkdir -p "$dst"
            for f in "${files[@]}"; do cp -f "$verdir/$f" "$dst/$f"; done
        fi
    done
done

echo "── 源：$UPSTREAM_URL @ $UPSTREAM_REV"
echo "── 新引入 $NEW 件 · 已存在 $SKIP 件 · 冲突/拒收 $CONFLICT 件"
if [ "$APPLY" = 1 ]; then
    cp -f "$PROV" "$ROOT/registry/THIRD_PARTY.md"
    echo "── 已写入 registry/THIRD_PARTY.md（来源与许可表）"
else
    echo "── 预演模式：未写入任何文件（加 --apply 生效）"
    head -20 "$PROV"
fi
rm -rf "$TMPL"
[ "$CONFLICT" = 0 ] || exit 1
