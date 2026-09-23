#!/usr/bin/env bash
# ============================================================
# tools/import_registry_px.sh —— 把第三方 registry-px 的库引入官方 registry/
# ------------------------------------------------------------
# 用法：
#   tools/import_registry_px.sh --src <registry-px 检出目录> [--exclude a,b] [--apply] [--update]
# 默认（不带 --apply）为**预演**：只列出会写入什么、逐件 sha256，不动任何文件。
#
# M198 补：`--update`（**就地更新**）—— 上游在**同一个版本目录**里改了内容时的唯一正规通道
#   （实测 2026-09-23：上游 db5f210 → 7da3397e 就是「同版本就地改」：mysql 删 sha1.px 加 stmt.px、
#   pg/cli/passhash 各改若干行）。默认行为仍是**冲突即拒**（人工决定），显式 `--update` 才写：
#     · 内容不同的文件 ⇒ 覆盖（表里状态记「**就地更新**」）；
#     · **上游已删除**的文件 ⇒ 从本地包目录删除（多文件包：M187 起包内全部 .px 随包分发）；
#     · **上游新增**的文件 ⇒ 补入（并把「本地缺文件」也算作"需更新"，不再静默算作一致）。
#
# 纪律（M187 立）：
#   ① **逐字节照搬**（不改上游文件一个字节）⇒ 上游 sha256 可直接与我们的对拍；
#      来源与许可**不在包内改注释**，一律登记在 registry/THIRD_PARTY.md；
#   ② 冲突即拒：目标已存在同名包且内容不同 ⇒ 报错退出（人工决定版本号）；
#   ③ 只认 `<src>/registry/<name>/<ver>/` 结构（与官方 registry 规范同形），
#      包内相对 import 原样保留 ⇒ 多文件包直接可用（pxpkg M187 起支持）。
#
# M189 补：**本地补丁**通道（keep 纪律①的可审计例外）
#   `tools/patches/registry-px/<name>.patch`（unified diff，路径相对包目录 ⇒ `patch -p0 -d <pkg>`）
#   在「照搬之后、写盘之前」应用到**暂存副本**上：
#     · 无补丁的包 ⇒ 与上游逐字节相同（纪律①不变）；
#     · 有补丁的包 ⇒ THIRD_PARTY.md 的「补丁」列写明文件名，sha 记的是**打过补丁后**的内容，
#       便于 M187 门继续做「表 ⇔ 磁盘」防漂移对拍；
#     · 补丁只用于「上游尚未修、但缺陷根因在我方语言面变更」的场景（如 M184 的严格 int()），
#       上游修复后**撤销补丁**即可（表里会少一列内容）。
# ============================================================
set -uo pipefail
SRC=""
EXCLUDE=""
APPLY=0
UPDATE=0
while [ $# -gt 0 ]; do
    case "$1" in
        --src) SRC="${2:-}"; shift 2 ;;
        --exclude) EXCLUDE="${2:-}"; shift 2 ;;
        --apply) APPLY=1; shift ;;
        --update) UPDATE=1; shift ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done
[ -n "$SRC" ] || { echo "用法: import_registry_px.sh --src <registry-px 目录> [--exclude a,b] [--apply]" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC_REG="$SRC/registry"
PATCHDIR="$ROOT/tools/patches/registry-px"
[ -d "$SRC_REG" ] || { echo "错误: 源 registry 目录不存在：$SRC_REG" >&2; exit 2; }
UPSTREAM_REV="$(git -C "$SRC" rev-parse --short HEAD 2>/dev/null || echo unknown)"
UPSTREAM_URL="$(git -C "$SRC" config --get remote.origin.url 2>/dev/null || echo unknown)"

TMPL=/tmp/m187_import.$$
mkdir -p "$TMPL"
PROV="$TMPL/THIRD_PARTY.md"
NEW=0; SKIP=0; CONFLICT=0; PATCHED=0; UPD=0; REMOVED=0

cat > "$PROV" <<EOF
# 官方 registry · 第三方包来源与许可（自动生成，勿手改）

> 生成器：\`tools/import_registry_px.sh\`（M187 第 65 轮 · M189 补**本地补丁**通道）· 源仓库：
> \`${UPSTREAM_URL}\` @ \`${UPSTREAM_REV}\` · 许可：**Apache-2.0**（上游 LICENSE 与本仓同族）
> 纪律：包文件**逐字节照搬**（不改一个字节）；来源/许可登记在本文件，不改包内注释。
> **例外（M189）**：「补丁」列非空的包 = 在照搬之上叠了一道**本地小补丁**（\`tools/patches/registry-px/<name>.patch\`），
> 原因是**我方语言面收紧**导致上游代码需要跟着改（如 M184 的严格 \`int()\`），而上游尚未发新版；
> 此时该包的 sha256 记的是**打过补丁后**的内容（M187 门的「表 ⇔ 磁盘」对拍因此仍然有效）。
> 上游修复后撤销补丁即可。
> 复核：\`bash examples/m187_registry_import/verify.sh\` 会**重算**下表 sha256 与磁盘对拍（防漂移）。

| 包 | 版本 | 文件数 | 入口 sha256（前 16） | 三轨验证 | 备注 | 补丁 |
|---|---|---|---|---|---|---|
EOF

prov_note() {   # 三轨验证列（口径 = M186 轮的 53 库普查；随普查更新）
    case "$1" in
        concurrent_map|workerpool) echo "编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006）" ;;
        qrcode) echo "编译轨 PASS · 解释轨**超时**（PX-DEF-024 性能：单码 8 掩码罚分 ≈36s）" ;;
        pg|mysql) echo "文件完整 · 测试需**真实服务端**（PG13 / MariaDB）" ;;
        passhash) echo "**双轨 PASS**（M189：打本地补丁后 \`passhash_test\` 通过）" ;;
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
        # ── 暂存区：照搬 → 打补丁 → 算 sha（保证「表里的 sha = 磁盘上真实内容」）──
        stage="$TMPL/stage/$name/$ver"
        rm -rf "$stage"; mkdir -p "$stage"
        for f in "${files[@]}"; do cp -f "$verdir/$f" "$stage/$f"; done
        patch_name=""
        if [ -f "$PATCHDIR/$name.patch" ]; then
            if ! ( cd "$stage" && patch -p0 -s --no-backup-if-mismatch < "$PATCHDIR/$name.patch" ); then
                echo "  [补丁失败] $name/$ver：$PATCHDIR/$name.patch 未能应用（锚点漂移？）" >&2
                CONFLICT=$((CONFLICT+1)); continue
            fi
            patch_name="$name.patch"
            PATCHED=$((PATCHED+1))
        fi
        dst="$ROOT/registry/$name/$ver"
        conflict=0
        missing=""
        if [ -d "$dst" ]; then
            for f in "${files[@]}"; do
                if [ ! -f "$dst/$f" ]; then conflict=1; missing="$missing $f"; fi
                if [ -f "$dst/$f" ] && ! cmp -s "$stage/$f" "$dst/$f"; then conflict=1; fi
            done
            [ -n "$missing" ] && echo "  [注意] $name/$ver 本地缺文件（上游有）：$missing" 
            if [ "$conflict" = 1 ] && [ -n "$patch_name" ]; then
                # 已有目录内容不同：若差异正好等于「上游 + 本补丁」则不算冲突
                same=1
                for f in "${files[@]}"; do cmp -s "$stage/$f" "$dst/$f" || same=0; done
                [ "$same" = 1 ] && conflict=0
            fi
        fi
        do_update=0
        if [ "$conflict" = 1 ] && [ "$UPDATE" = 1 ]; then
            do_update=1
        elif [ "$conflict" = 1 ]; then
            echo "  [冲突] $name/$ver 已存在且内容不同 —— 需人工定版本号（拒写；要就地更新用 --update）" >&2
            CONFLICT=$((CONFLICT+1)); continue
        fi
        if [ -d "$dst" ]; then
            if [ "$do_update" = 1 ]; then
                UPD=$((UPD+1)); state="**就地更新**（上游同版本目录变更）"
                for oldf in $(cd "$dst" && ls *.px 2>/dev/null); do
                    keep=0
                    for f in "${files[@]}"; do [ "$f" = "$oldf" ] && keep=1; done
                    if [ "$keep" = 0 ]; then
                        echo "  [删除] $name/$ver/$oldf（上游已移除）"
                        [ "$APPLY" = 1 ] && rm -f "$dst/$oldf"
                        REMOVED=$((REMOVED+1))
                    fi
                done
            else
                SKIP=$((SKIP+1)); state="已存在（内容一致）"
            fi
        else
            NEW=$((NEW+1)); state="新引入"
        fi
        entry_sha="$(sha256sum "$stage/$name.px" | cut -c1-16)"
        extra=""
        [ -n "$patch_name" ] && extra="补丁：\`$patch_name\`"
        printf '| %s | %s | %d | `%s` | %s | %s | %s |\n' \
            "$name" "$ver" "${#files[@]}" "$entry_sha" "$(prov_note "$name")" "$state" "$extra" >> "$PROV"
        if [ "$APPLY" = 1 ]; then
            mkdir -p "$dst"
            for f in "${files[@]}"; do cp -f "$stage/$f" "$dst/$f"; done
        fi
    done
done

echo "── 源：$UPSTREAM_URL @ $UPSTREAM_REV"
echo "── 新引入 $NEW 件 · 已存在 $SKIP 件 · **就地更新 $UPD 件**（上游已删 $REMOVED 个文件）· 冲突/拒收 $CONFLICT 件 · 其中带本地补丁 $PATCHED 件"
if [ "$APPLY" = 1 ]; then
    cp -f "$PROV" "$ROOT/registry/THIRD_PARTY.md"
    echo "── 已写入 registry/THIRD_PARTY.md（来源与许可表）"
else
    echo "── 预演模式：未写入任何文件（加 --apply 生效）"
    head -22 "$PROV"
fi
rm -rf "$TMPL"
[ "$CONFLICT" = 0 ] || exit 1
