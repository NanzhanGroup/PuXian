#!/usr/bin/env bash
# ============================================================
# packaging/latest_guard.sh —— `releases/latest` 必须等于「最高版本 tag」（缺陷 587/588 · M335）
#
# 为什么有它（2026-10-26 实测事故）：
#   `tools/install.sh` 取的是 `api.github.com/repos/.../releases/latest`。
#   GitHub 的 latest **按 Release 的创建时间排，不看版本号**！
#   ⇒ 那天先补 v0.2.330、再补 v0.2.325/326/327，latest 一度变成 **v0.2.327**
#   ⇒ 一键安装会给用户装**旧树**，而当时 release_reconcile 报 OK
#     （它只看「有没有 Release / 资产齐不齐」，不看 latest 指向谁）。
#   这与「tag 事件丢失导致资产没出来」同型、且**更难发现**：页面看着很新，装出来是旧的。
#
# 它做什么：
#   ① 取 git tag 列表 → 最高的「纯数字版本号」`^v<主>.<次>.<修>$` = 期望的 latest；
#     （带后缀的开发 tag 如 v0.2.0-mNN 不进要求集：本就不该有可下载版本）
#   ② 取 `releases/latest` 实到的 tag；
#   ③ 不等 ⇒ 判红（rc=1）；`--fix` 时先 PATCH make_latest=true 再**重新读一遍**自证。
#
# 用法：
#   latest_guard.sh [--repo OWNER/NAME] [--fix] [--json] [--api-fixture DIR]
#   环境：GH_TOKEN（**只走环境变量**，不落盘、不进仓库 [SECURITY/P0]）
#   退出码：0 = 一致（或已修好并自证）· 1 = 不一致/修复失败 · 2 = 用法或环境错
#
# 覆盖边界（如实登记）：
#   · 判据面只到「latest 指向哪个 tag」；latest 指向的 Release 资产是否齐全由
#     check_release_assets.sh / release_reconcile.sh 负责（各管一段，不重叠）
#   · 仓库一个 Release 都没有 ⇒ 拿不到 latest ⇒ **响亮降级**（rc=0 + stderr 告警），不判红也不静默
#   · --api-fixture 模式只判「发现」不判「修复」（不联网 ⇒ 不能 PATCH）
# ============================================================
set -uo pipefail

REPO=${GH_REPO:-NanzhanGroup/PuXian}
FIX=0; JSON=0; FIXTURE=""
API=${GH_API:-https://api.github.com}

usage(){ sed -n '2,32p' "$0" | sed 's/^# \{0,1\}//'; }

while [ $# -gt 0 ]; do
  case "$1" in
    --repo)        REPO="$2"; shift 2;;
    --fix)         FIX=1; shift;;
    --json)        JSON=1; shift;;
    --api-fixture) FIXTURE="$2"; shift 2;;
    -h|--help)     usage; exit 0;;
    *) echo "未知选项：$1" >&2; usage >&2; exit 2;;
  esac
done

if [ -z "$FIXTURE" ] && [ -z "${GH_TOKEN:-}" ]; then
  echo "❌ 未设置 GH_TOKEN（令牌只走环境变量），也未给 --api-fixture" >&2
  exit 2
fi

W=$(mktemp -d); trap 'rm -rf "$W"' EXIT

api_get(){   # $1=path $2=outfile  ⇒ 0 成功
  curl -sS -H "Authorization: Bearer ${GH_TOKEN:-}" -H "Accept: application/vnd.github+json" \
       "$API/repos/$REPO$1" -o "$2" 2>/dev/null || return 1
  grep -q '\[' <<< "$(head -c1 "$2" 2>/dev/null)" && return 0
  grep -q '"tag_name"\|"id"' "$2" 2>/dev/null && return 0
  return 1
}

# ---- ① git tag 列表（两页足够：仓库单页 100 个，最新版本都在第一页）----
TAG_FILES=""
for pg in 1 2; do
    f="$W/tags$pg.json"
    if [ -n "$FIXTURE" ]; then
        src="$FIXTURE/tags.json"; [ "$pg" = 2 ] && src="$FIXTURE/tags2.json"
        [ -f "$src" ] && { cp "$src" "$f"; TAG_FILES="$TAG_FILES $f"; }
    elif api_get "/tags?per_page=100&page=$pg" "$f"; then
        TAG_FILES="$TAG_FILES $f"
    fi
done
if [ -z "$TAG_FILES" ]; then
    echo "❌ 拿不到 git tag 列表 ⇒ 无法确定「最高版本 tag」" >&2
    exit 2
fi

# ---- ② releases/latest（404 = 一个 Release 都没有 ⇒ 降级，不判红）----
LATEST_FILE=""
if [ -n "$FIXTURE" ]; then
    [ -f "$FIXTURE/latest.json" ] && LATEST_FILE="$FIXTURE/latest.json"
else
    api_get "/releases/latest" "$W/latest.json" && LATEST_FILE="$W/latest.json"
fi

EXPECTED=$(python3 - "$TAG_FILES" <<'PY'
import json, re, sys
best = None
def key(v):
    return tuple(int(x) for x in v[1:].split('.'))
for f in sys.argv[1].split():
    try: d = json.load(open(f))
    except Exception: continue
    if not isinstance(d, list): continue
    for it in d:
        n = (it or {}).get('name') or ''
        if re.match(r'^v\d+\.\d+\.\d+$', n):
            if best is None or key(n) > key(best): best = n
print(best or '')
PY
)

CURRENT=""
if [ -n "$LATEST_FILE" ]; then
    CURRENT=$(python3 -c 'import json,sys; d=json.load(open(sys.argv[1])) or {}; print(d.get("tag_name") or "")' "$LATEST_FILE" 2>/dev/null || true)
fi

emit(){   # $1=verdict $2=note
    if [ "$JSON" = 1 ]; then
        printf '{"repo":"%s","expected":"%s","latest":"%s","verdict":"%s","fixed":%s}\n' \
            "$REPO" "$EXPECTED" "$CURRENT" "$1" "${FIXED:-false}"
    fi
}

# JSON 模式：人读文案一律走 stderr，stdout 只留一个 JSON（否则调用方解析不到）
say(){ if [ "$JSON" = 1 ]; then echo "$@" >&2; else echo "$@"; fi; }

if [ -z "$EXPECTED" ]; then
    echo "⚠️ 仓库里没有「纯数字版本号」的 tag ⇒ 无期望值，跳过（能力降级）" >&2
    emit SKIP
    exit 0
fi

if [ -z "$CURRENT" ]; then
    echo "⚠️ 拿不到 releases/latest（仓库可能一个 Release 都没有）⇒ 跳过，不判红也不静默" >&2
    emit NO-LATEST
    exit 0
fi

if [ "$CURRENT" = "$EXPECTED" ]; then
    say "✅ releases/latest = $CURRENT（= 最高版本 tag $EXPECTED）"
    emit OK
    exit 0
fi

# ---- ③ 不一致：先分清「期望版本还没有 Release（发布中/未发布）」与「有 Release 但 latest 指向别处」----
# 为什么要分（M339 实测踩到）：tag 刚推上去、Release 还在构建时，latest 必然还指向上一个版本。
#   那不是缺陷（发布链自己会在 assets-verify 末尾把 latest 纠正过来），判红只会制造假红。
#   真正的缺陷形态是：**期望版本的 Release 已经存在**，latest 却指着旧版本 ⇒ install.sh 装旧树。
EXPECTED_HAS_RELEASE=0
if [ -n "$FIXTURE" ]; then
    [ -f "$FIXTURE/expected_release.json" ] && EXPECTED_HAS_RELEASE=1
else
    _code=$(curl -sS -o "$W/exp.json" -w '%{http_code}' -H "Authorization: Bearer ${GH_TOKEN:-}" \
         -H "Accept: application/vnd.github+json" "$API/repos/$REPO/releases/tags/$EXPECTED" 2>/dev/null) || _code=""
    [ "$_code" = "200" ] && EXPECTED_HAS_RELEASE=1
fi

if [ "$EXPECTED_HAS_RELEASE" = 0 ]; then
    echo "ℹ️ PENDING：$EXPECTED 还没有 Release（可能在发布中）⇒ latest 仍指向 $CURRENT 是预期的；本判据不判红" >&2
    echo "   （「该有发布却没有」由 release_reconcile.sh 负责，两者不重叠）" >&2
    emit PENDING
    exit 0
fi

echo "❌ LATEST-STALE: releases/latest = $CURRENT ≠ 最高版本 tag $EXPECTED" >&2
echo "   ⇒ tools/install.sh 取 latest ⇒ 用户会装到 $CURRENT 的树（旧树）" >&2

if [ "$FIX" = 1 ] && [ -z "$FIXTURE" ]; then
    # 只把**期望的**那个 Release 标成 latest（幂等：重复执行结果相同）
    rid=$(curl -sS -H "Authorization: Bearer ${GH_TOKEN:-}" -H "Accept: application/vnd.github+json" \
              "$API/repos/$REPO/releases/tags/$EXPECTED" 2>/dev/null \
          | python3 -c 'import json,sys; print((json.load(sys.stdin) or {}).get("id") or "")' 2>/dev/null || true)
    if [ -z "$rid" ]; then
        echo "❌ 修复失败：找不到 $EXPECTED 的 Release（先补发布再谈 latest）" >&2
        emit STALE-NO-RELEASE; exit 1
    fi
    code=$(curl -sS -o "$W/patch.json" -w '%{http_code}' -X PATCH \
                -H "Authorization: Bearer ${GH_TOKEN:-}" -H "Accept: application/vnd.github+json" \
                -d '{"latest":true}' "$API/repos/$REPO/releases/$rid")
    case "$code" in
      200) ;;
      *) echo "❌ 修复失败：PATCH make_latest 返回 HTTP $code" >&2; emit STALE-PATCH-FAIL; exit 1;;
    esac
    # **重新读一遍**（不信任 PATCH 回执 —— 与 GATE_8「复核已发布的包别只看仓库内件」同口径）
    api_get "/releases/latest" "$W/latest2.json" || { echo "❌ 修复后无法回读 latest" >&2; exit 1; }
    now=$(python3 -c 'import json,sys; print((json.load(open(sys.argv[1])) or {}).get("tag_name") or "")' "$W/latest2.json" 2>/dev/null || true)
    if [ "$now" = "$EXPECTED" ]; then
        FIXED=true
        say "✅ 已修复并自证：releases/latest = $now"
        emit OK
        exit 0
    fi
    echo "❌ 修复后仍不一致：latest = $now ≠ $EXPECTED" >&2
    emit STALE-AFTER-FIX; exit 1
fi

if [ "$FIX" = 1 ] && [ -n "$FIXTURE" ]; then
    echo "ℹ️ fixture 模式不执行修复（不联网）" >&2
fi
emit STALE
exit 1
