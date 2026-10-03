#!/usr/bin/env bash
# ============================================================
# packaging/bump_version.sh —— 把「用户可见版本号」改到指定值（M249 新增）
# ------------------------------------------------------------
# 为什么要有它（用户令 2026-10-03）：
#   「以后每次 tag 之前都改变一下 0.2.*，直到 * 变为 100 就升为 0.3.0」
#   ⇒ 版本号**每轮都动**，而它散在 7 处字面量里（2 个 shell + 5 个 .px）。
#     手改 7 处 = 早晚漏一处（M235 缺陷 355「名册同步了、门忘了注册」同族）。
#
# ⚠ 改了 `selfhost/*.px` ⇒ **必须重烘入库件 + 冻结门重定基**（本脚本会在结尾提醒）。
#   `tools/*` 是 shell，不参与烘烤指纹，零成本。
#
# 用法：packaging/bump_version.sh <新版本>       例：packaging/bump_version.sh 0.2.1
#       packaging/bump_version.sh --check        只报告当前各处取值（零副作用）
# 退出码：0 成功 · 2 参数错 · 3 校验失败（有一处没改到，已回滚）
# ============================================================
set -uo pipefail
ROOT="${BUMP_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
cd "$ROOT" || { echo "❌ 进不去仓库根 $ROOT" >&2; exit 2; }

# ── 替换策略：读出现的**旧版本号**，逐文件**全量替换** ──
#   不用行号（会漂 —— M244 教训），也不用「模式」（写法变了就失配）。
#   ⚠ 前提：旧版本号在各文件里**只出现在应改的地方** ——
#     本脚本会先打印逐文件计数并要求 ≥1，改完再校验「新版本在、旧版本无残留」。

die() { echo "❌ $*" >&2; exit "${EC:-1}"; }

peek() {  # 当前版本号：从 tools/px 的 SELFHOST_VER 取（唯一读入口）
    grep -oE '^SELFHOST_VER="[0-9]+\.[0-9]+\.[0-9]+"' tools/px 2>/dev/null \
        | head -1 | sed 's/^SELFHOST_VER="//; s/"$//'
}

NEW=""
case "${1:-}" in
    --check) NEW="" ;;
    -h|--help)
        sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
        exit 0 ;;
    '') die "缺少新版本号（例：packaging/bump_version.sh 0.2.1）" ;;
    *) NEW="$1" ;;
esac

printf '%s' "${NEW}" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$' \
    || [ -z "$NEW" ] || die "版本号格式不对：'$NEW'（要 X.Y.Z）"

CUR="$(peek)"
[ -n "$CUR" ] || die "读不到当前版本号（tools/px 里没有 SELFHOST_VER=\"X.Y.Z\"）"

echo "=== 版本号：当前 $CUR ==="
FILES="tools/px tools/pxc selfhost/compiler.px selfhost/interp.px tools/pxfmt.px tools/pxlsp.px tools/pxmcp.px"
printf '  %-24s %s\n' "文件" "含 $CUR 的次数"
for f in $FILES; do
    [ -f "$f" ] || die "缺文件：$f"
    n="$(grep -cF "$CUR" "$f" 2>/dev/null || echo 0)"
    printf '  %-24s %s\n' "$f" "$n"
    [ "$n" -ge 1 ] || die "$f 里找不到 '$CUR' —— 版本号可能已被改过，或该文件的写法变了"
done

if [ -z "$NEW" ]; then
    echo "  （--check：零副作用，结束）"
    exit 0
fi
[ "$NEW" = "$CUR" ] && { echo "  （已是 $NEW，无需改动）"; exit 0; }

# ── 备份 + 替换（用 python 逐文件精确替换，失败即整体回滚）──
BAK="$(mktemp -d)"
for f in $FILES; do mkdir -p "$BAK/$(dirname "$f")"; cp "$f" "$BAK/$f"; done

if ! python3 - "$CUR" "$NEW" $FILES <<'PY'
import sys, pathlib, os
cur, new = sys.argv[1], sys.argv[2]
# ⚠ 去重：`tools/pxc` 是 `tools/px` 的**符号链接** ⇒ 同一实体。
#   不去重的话，处理完 px 再处理 pxc 会读到**已替换**的内容
#   ⇒ 报「找不到旧版本」。（2026-10-03 实测踩到 —— 判据的「找不到」先怀疑判据自己。）
seen = set()
for fs in sys.argv[3:]:
    rp = os.path.realpath(fs)
    if rp in seen:
        print("   ↷ %s：与前面同一实体（符号链接）⇒ 跳过" % fs)
        continue
    seen.add(rp)
    p = pathlib.Path(fs)
    s = p.read_text(encoding="utf-8")
    n = s.count(cur)
    if n < 1:
        print("   ❌ %s 里没有 %s" % (fs, cur), file=sys.stderr); sys.exit(1)
    p.write_text(s.replace(cur, new), encoding="utf-8")
    print("   ✏️  %s：%d 处" % (fs, n))
PY
then
    echo "❌ 替换失败，正在回滚…" >&2
    for f in $FILES; do cp "$BAK/$f" "$f"; done
    echo "   已回滚到替换前状态" >&2
    exit 3
fi

# ── 校验：新版本各就各位、旧版本无残留 ──
BAD=0
for f in $FILES; do
    grep -qF "$NEW" "$f" || { echo "  ❌ $f 里没有新版本 $NEW" >&2; BAD=1; }
    grep -qF "$CUR" "$f" && { echo "  ❌ $f 里仍有旧版本 $CUR" >&2; BAD=1; }
done
if [ "$BAD" != 0 ]; then
    for f in $FILES; do cp "$BAK/$f" "$f"; done
    echo "❌ 校验失败 ⇒ 已回滚" >&2; exit 3
fi
rm -rf "$BAK"

echo "✅ 已把 $CUR → $NEW 写在 7 个文件里"
echo
echo "⚠️ 接下来必做（改了 selfhost/*.px ⇒ 源码链变了）："
echo "   ① 重烘入库件：  ./selfhost/rebake_bin.sh"
echo "   ② 冻定格验证：  见 CHANGELOG M249 的「重烘 + 冻结门」段"
echo "   ③ 跑门：        ./selfhost/run_gates.sh"
echo "   ④ 打 tag 时会**自动**用递增后的版本段：packaging/make_tag.sh --milestone <N>"
