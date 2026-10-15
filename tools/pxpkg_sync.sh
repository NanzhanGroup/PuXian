#!/usr/bin/env bash
# ============================================================
# tools/pxpkg_sync.sh —— `pxpkg sync` 客户端（W1 · docs/PXPKG_SYNC_PLAN.md §四）
# ------------------------------------------------------------
# 目的：用户**只凭一条 URL** 就能把远程 registry 同步到本地（原子 / 可验 / 可复现 / 幂等）。
#   M69 曾定「registry 目录级远程（URL 枚举/索引）不做」—— 当年缺的是分发站；
#   现在对端已就绪（soft.wsai.chat 源站 + soft.xiusoft.cn 境内边缘）⇒ 缺的只剩**客户端**。
#
# 契约实现（对齐 §四表，逐条可判）：
#   ① 入口        GET <base>/version.json ⇒ version / commit / archive / archive_sha256 / archive_bytes
#   ② 清单        ⚠ **F1（对端缺口 · 本机 13 条变体实测封口）**：version.json 声称的
#                 `files_manifest` 在分发站上**取不到** —— 任何以 `.sha256` 结尾的路径**一律 403**，
#                 `files.sha256.txt` / `SHA256SUMS` / `files.txt` / `manifest.txt` **全 404**，
#                 `sha256sums.txt` 只含「归档」一行，归档内亦无清单（只有 checksum.px）。
#                 ⇒ 逐文件清单**无旁路**；本实现因此落在**归档级校验**（= ③ 的整包口径）。
#                 待对端处置（我方推荐「清单并入归档」）后，② 的挂点在此处升级为逐文件校验。
#   ③ 增量        ⚠ **F2**：对端归档仅 ~230 KB ⇒ **整包归档**口径最简最稳；
#                 增量粒度 = **归档级**（`archive_sha256` 变 ⇒ 重下整包）。逐文件增量待 ② 解除后做。
#   ④ 原子写入    先落 **stage**（与目标**同盘** —— 跨盘 rename 会退化成「复制+删除」，不再原子）
#                 ⇒ 校验/解压全在 stage 内完成 ⇒ 最后一步才 rename 换入。
#   ⑤ 幂等短路    本地指纹（version + commit + archive_sha256）与对端一致 ⇒ 立即 rc=0，**0 下载**。
#   ⑥ 落盘即验    归档写完**立即**校验 sha256；不符即中止（报期望/实得的 12 位前缀）。
#   ⑦ 失败语义    任一步失败 ⇒ 非 0 退出，且**本地 registry 逐字节保持原样**（旧目录改名备份 + 回滚）。
#   ⑧ 缓存 key    由「真正决定行为的输入」驱动：version / commit / archive_sha256 / 归档字节数。
#
# 用法：
#   pxpkg sync [--base <URL>] [--registry <DIR>] [--dry-run] [--force] [--quiet]
#
#   环境变量：
#     PXPKG_SYNC_BASE     默认对端基址（境内边缘优先；源站可显式指定）
#     PXPKG_SYNC_DIR      默认同步根（registry 落地 <DIR>/registry）
#     PXPKG_SYNC_TIMEOUT  curl 超时秒数（默认 60）
#
# 依赖：curl + sha256sum + tar（**零新依赖** —— 与 §五 W1「纯 .px/shell」一致）
# 退出码：0=成功 / 已最新 / 预演 · 1=失败（目标未动）· 2=用法错
# ============================================================
set -u

DEFAULT_BASE="${PXPKG_SYNC_BASE:-https://soft.xiusoft.cn/puxian-registry}"
CURL_TIMEOUT="${PXPKG_SYNC_TIMEOUT:-60}"
FINGER_NAME=".pxpkg-sync.json"

BASE=""
REG=""
DRY=0
FORCE=0
QUIET=0

die()  { printf 'pxpkg sync: ERROR: %s\n' "$*" >&2; exit 1; }
usage_err() { printf 'pxpkg sync: %s\n' "$*" >&2; exit 2; }
log()  { [ "$QUIET" = "1" ] || printf 'pxpkg sync: %s\n' "$*"; }
warn() { printf 'pxpkg sync: %s\n' "$*" >&2; }

usage() {
    sed -n '/^# 用法：/,/^# 退出码/p' "$0" | sed 's/^# \{0,1\}//'
}

while [ $# -gt 0 ]; do
    case "$1" in
        --base)      [ $# -ge 2 ] || usage_err "--base 需要参数"; BASE="$2"; shift 2 ;;
        --registry)  [ $# -ge 2 ] || usage_err "--registry 需要参数"; REG="$2"; shift 2 ;;
        --dry-run)   DRY=1; shift ;;
        --force)     FORCE=1; shift ;;
        --quiet)     QUIET=1; shift ;;
        -h|--help)   usage; exit 0 ;;
        *)           usage_err "未知参数 '$1'（--help 看用法）" ;;
    esac
done

[ -n "$BASE" ] || BASE="$DEFAULT_BASE"
BASE="${BASE%/}"
[ -n "$REG" ]  || REG="${PXPKG_SYNC_DIR:-$HOME/.pxpkg}/registry"
case "$REG" in /*) ;; *) usage_err "--registry 必须是绝对路径（现值 '$REG'）" ;; esac

command -v curl >/dev/null 2>&1      || die "缺 curl"
command -v sha256sum >/dev/null 2>&1 || die "缺 sha256sum"
command -v tar >/dev/null 2>&1       || die "缺 tar"

# ── ① 取元数据（落盘之前先取 —— 失败时目标目录**根本没被碰过**）──────────────
VJ="$(curl -fsSL --max-time "$CURL_TIMEOUT" "$BASE/version.json" 2>/dev/null)" \
    || die "取 version.json 失败：$BASE/version.json（网络 / 404 / 路径错）"

jget() {   # $1=键名 —— 只取**第一个**字符串/数字值（version.json 是单层机器可读元数据）
    printf '%s' "$VJ" | sed -n "s/.*\"$1\"[[:space:]]*:[[:space:]]*\"\([^\"]*\)\".*/\1/p" | head -1
}
jnum() {
    printf '%s' "$VJ" | sed -n "s/.*\"$1\"[[:space:]]*:[[:space:]]*\([0-9][0-9]*\).*/\1/p" | head -1
}

VER="$(jget version)"
COMMIT="$(jget commit)"
ARCH="$(jget archive)"
ASHA="$(jget archive_sha256)"
ABYT="$(jnum archive_bytes)"
[ -n "$ARCH" ] || die "version.json 缺 archive 字段（对端元数据不合契约）"
[ -n "$ASHA" ] || die "version.json 缺 archive_sha256 字段"
case "$ASHA" in ????????????????????????????????????????????????????????????????) ;;
    *) die "archive_sha256 不是 64 位十六进制（实得 '$ASHA'）" ;; esac

# ── ⑤ 幂等短路：指纹命中 ⇒ 0 下载 ─────────────────────────────────────────
prev=""
if [ -d "$REG" ] && [ -f "$REG/$FINGER_NAME" ]; then
    prev="$(sed -n 's/.*"archive_sha256"[[:space:]]*:[[:space:]]*"\([0-9a-f]\{64\}\)".*/\1/p' "$REG/$FINGER_NAME" | head -1)"
fi
UPTODATE=0
[ "$FORCE" = "0" ] && [ -n "$prev" ] && [ "$prev" = "$ASHA" ] && UPTODATE=1

if [ "$DRY" = "1" ]; then
    log "元数据：version=${VER:-?} commit=${COMMIT:0:12} archive=$ARCH bytes=${ABYT:-?} sha256=${ASHA:0:12}…"
    log "目标：$REG"
    if [ "$UPTODATE" = "1" ]; then
        log "DRY-RUN：本地指纹已命中（$prev）⇒ 同步会**立即 rc=0、0 下载**。零副作用。"
    else
        log "DRY-RUN：将下载 $BASE/$ARCH（${ABYT:-?} B）→ 落 stage → 校验 sha256 → 解压（要求含 registry/）→ 原子换入 $REG。零副作用。"
    fi
    exit 0
fi

if [ "$UPTODATE" = "1" ]; then
    log "已是最新（version=${VER:-?} sha256=${ASHA:0:12}…）⇒ 0 下载，rc=0"
    log "PX_REGISTRY=$REG"
    exit 0
fi

[ -e "$REG" ] && [ ! -d "$REG" ] && die "目标存在但不是目录：$REG"

# ── ④ stage 与目标**同盘**（rename 才原子）────────────────────────────────
REGP="$(dirname "$REG")"
mkdir -p "$REGP" || die "无法创建目标父目录：$REGP"
STAGE="$REGP/.pxpkg-sync.stage.$$.$(date +%s)"
rm -rf "$STAGE"
mkdir -p "$STAGE" || die "stage 创建失败：$STAGE"
cleanup() { rm -rf "$STAGE"; }
trap cleanup EXIT

log "元数据：version=${VER:-?} commit=${COMMIT:0:12} archive=$ARCH bytes=${ABYT:-?} sha256=${ASHA:0:12}…"
log "目标：$REG（当前：${prev:-无}）"

# ── ⑥ 下载 + 落盘即验（归档级 —— 见 ② F1）────────────────────────────────
curl -fsSL --max-time "$CURL_TIMEOUT" -o "$STAGE/$ARCH" "$BASE/$ARCH" \
    || die "归档下载失败：$BASE/$ARCH ⇒ 目标目录未动"
if [ -n "$ABYT" ]; then
    got_b="$(wc -c < "$STAGE/$ARCH" | tr -d ' ')"
    [ "$got_b" = "$ABYT" ] || die "归档字节数不符（期望 $ABYT 实得 $got_b）⇒ 中止，目标目录未动"
fi
got="$(sha256sum "$STAGE/$ARCH" | awk '{print $1}')"
[ "$got" = "$ASHA" ] || die "归档哈希不符（期望 ${ASHA:0:12}… 实得 ${got:0:12}…）⇒ 中止，目标目录未动"
log "归档 sha256 校验通过（${got:0:12}…）"

# 路径卫生：归档里不许有绝对路径或 `../`（防解压越界 —— 对端一旦被污染，此处是第一道闸）
tar -tzf "$STAGE/$ARCH" > "$STAGE/list.txt" 2>/dev/null || die "归档不可解析（tar -tzf 失败）⇒ 中止"
if grep -Eq '(^/|(^|/)\.\./)' "$STAGE/list.txt"; then
    die "归档含越界路径（绝对路径或 ../）⇒ 中止，目标目录未动"
fi

# ── 解压到 stage（先解压后换入：解压失败/无 registry/ ⇒ 目标原样）──────────────
tar -xzf "$STAGE/$ARCH" -C "$STAGE" || die "解压失败 ⇒ 中止，目标目录未动"
[ -d "$STAGE/registry" ] || die "归档内无 registry/ ⇒ 中止，目标目录未动"
NF="$(find "$STAGE/registry" -type f | wc -l | tr -d ' ')"
[ "$NF" -ge 1 ] || die "归档内 registry/ 为空 ⇒ 中止，目标目录未动"
rm -f "$STAGE/$ARCH" "$STAGE/list.txt"

# ── ⑧ 指纹落盘（写进 stage ⇒ 与内容同一次换入）──────────────────────────
cat > "$STAGE/registry/$FINGER_NAME" <<EOF
{"version":"${VER:-}","commit":"${COMMIT:-}","archive":"$ARCH","archive_sha256":"$ASHA","archive_bytes":${ABYT:-0},"files":$NF,"synced_at":"$(date -Iseconds)"}
EOF

# ── 换入（旧版本改名备份 ⇒ 失败回滚 ⇒ 目标**绝不**半新半旧）──────────────────
OLD="$REG.prev.$$"
if [ -d "$REG" ]; then
    mv "$REG" "$OLD" || die "旧 registry 备份失败 ⇒ 中止（目标未动）"
fi
if ! mv "$STAGE/registry" "$REG"; then
    [ -d "$OLD" ] && mv "$OLD" "$REG"
    die "换入失败 ⇒ 已回滚到原 registry"
fi
[ -d "$OLD" ] && rm -rf "$OLD"
log "同步完成：$NF 文件 → $REG（version=${VER:-?} sha256=${ASHA:0:12}…）"
log "PX_REGISTRY=$REG"
exit 0
