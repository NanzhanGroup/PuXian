#!/usr/bin/env bash
# ============================================================
# publish_scope_check.sh —— 发布自锁的**唯一口径**：被测面 ⇄ 发布面（M433 · 缺陷 896）
# ------------------------------------------------------------
# 病史（2026-11-22 16:40 实测 · M432）：
#   16:40:03 链判绿 306/306 → 交 publish_on_green → **ABORT dirty-tree**（rc=2）
#   当时脏的 5 个文件是**我同时在建的补丁**（ci.yml / ci_diagnose.py / fixtures …）。
#   17:15 才有 4 项交付 —— 中间靠人工补 tag，且顺带发了一条假文案 p1
#   （"扫描面 日志47行 · 红项 0 条"：真因是树脏，判据却没说出真因）。
#
# 为什么老口径是**错杀**（本文件存在的全部理由）：
#   链的 306 道门**从来不是在主工作树里跑的** —— 分片与 quiet 档全部在
#   `git worktree add --detach "$WT" "$HEAD_SHA"`（HEAD_SHA=git rev-parse HEAD）里跑，
#   门清单也取自 `git show HEAD:selfhost/gates.registry.sh`（shard_run.sh 第 1 节注释：
#   主树多一个未提交的门曾让分片去跑不存在的门 · 2026-10-12 教训）。
#   ⇒ **被测面 = 那一个 commit**，主树的未提交改动进不了测试面。
#   ⇒ 「主树脏就不许发」把「工作副本的卫生」当成了「证据的完整性」，是两件事。
#
# 但也**不是全都不拦**：make_tag 的版本链闸读的是 `$REPO/tools/px` 的**工作树**
#   （packaging/make_tag.sh 3.5 节 `_VERFILE="$REPO/tools/px"`）⇒ 主树 WIP 若碰
#   tools/px，版本闸看到的是 WIP 的值，tag 与源码版本号可能错配 ⇒ 这一类必须拦。
#
# 判据（唯一实现；publish_on_green 与门 m433_publish_scope 共用）：
#   SCOPE-OK       head 一致；WIP 计数并落盘（不阻断）
#   SCOPE-REJECT   stamp-head-missing / head-mismatch / wip-version-source
#
# 用法：
#   publish_scope_check.sh --repo DIR --stamp FILE [--wip-out FILE]
#   publish_scope_check.sh --self-test        # 离线夹具（临时 git 仓库，不碰真仓）
# 退出码：0 SCOPE-OK · 2 SCOPE-REJECT · 3 环境/用法错
# ============================================================
set -uo pipefail

REPO=""; STAMP=""; WIP_OUT=""; SELFTEST=0
while [ $# -gt 0 ]; do
    case "$1" in
        --repo)    REPO="${2:-}"; shift 2 ;;
        --stamp)   STAMP="${2:-}"; shift 2 ;;
        --wip-out) WIP_OUT="${2:-}"; shift 2 ;;
        --self-test) SELFTEST=1; shift ;;
        -h|--help) sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 3 ;;
    esac
done

SELF="$0"

# 唯一判定实现（参数化 ⇒ 自测/门可以拿夹具仓直接调它，不必复制逻辑）
scope_check() {
    local repo="$1" stamp="$2" wipout="${3:-}"
    [ -n "$repo" ] && [ -e "$repo/.git" ] || { echo "SCOPE-REJECT reason=env-no-repo repo=${repo:-}"; return 3; }
    [ -n "$stamp" ] && [ -f "$stamp" ] || { echo "SCOPE-REJECT reason=env-no-stamp stamp=${stamp:-}"; return 3; }

    local tested head lines wip_n wip_list wip_ver p
    tested="$(grep -oE 'head=[0-9a-f]{7,}' "$stamp" 2>/dev/null | tail -1 | cut -d= -f2)"
    head="$(git -C "$repo" rev-parse --short=7 HEAD 2>/dev/null)"

    lines="$(git -C "$repo" status --porcelain=v1 2>/dev/null)"
    if [ -n "$lines" ]; then
        wip_n="$(printf '%s\n' "$lines" | grep -c .)"
        wip_list="$(printf '%s\n' "$lines" | cut -c4- | sed 's/.* -> //' | paste -sd, -)"
    else
        wip_n=0; wip_list=""
    fi
    wip_ver=""
    for p in $(printf '%s\n' "${wip_list:-}" | tr ',' ' '); do
        case "$p" in
            tools/px|tools/px/*) wip_ver="$p" ;;
        esac
    done

    if [ -n "$wipout" ]; then
        { echo "# 发布时主树 WIP —— 被测面（commit）之外的未提交改动：不阻断发布，只留证"
          echo "# head=${head:-无} tested=${tested:-无} wip=$wip_n"
          if [ -n "$lines" ]; then printf '%s\n' "$lines"; else echo "（树净 · WIP=0）"; fi
        } > "$wipout" 2>/dev/null || true
    fi

    if [ -z "$tested" ]; then
        echo "SCOPE-REJECT reason=stamp-head-missing head=${head:-无} wip=$wip_n detail=stamp 里没有 head=<sha>，不知道测的是哪个 commit"
        return 2
    fi
    if [ "${head#"$tested"}" = "$head" ]; then
        echo "SCOPE-REJECT reason=head-mismatch tested=$tested head=${head:-无} wip=$wip_n detail=被门测的提交与当前 HEAD 不一致"
        return 2
    fi
    # ── 唯一阻断口径（缺陷 896 修法）：只拦「会让发布判定本身失真」的 WIP ──
    #   负控（门 N1）把这一行的条件注回 `[ "${wip_n:-0}" -gt 0 ]` ⇒ 立刻退回老口径（任何 WIP 都拒）
    if [ -n "$wip_ver" ]; then
        echo "SCOPE-REJECT reason=wip-version-source head=$head tested=$tested wip=$wip_n detail=${wip_ver} 被改：make_tag 版本链闸读工作树，会拿 WIP 的值去比 tag"
        return 2
    fi
    echo "SCOPE-OK head=${head} tested=${tested} wip=$wip_n wip-files=${wip_list:-无}"
    return 0
}

# ── 自测（离线：临时 git 仓库夹具；不碰真仓、不联网）──────────────
if [ "$SELFTEST" = "1" ]; then
    T="$(mktemp -d)"; P=0; F=0
    ok()  { echo "  PASS $1"; P=$((P+1)); }
    bad() { echo "  FAIL $1"; F=$((F+1)); }
    mkfix() {
        mkdir -p "$1" || return 1
        ( cd "$1" && git init -q . && git config user.email t@t.invalid && git config user.name t \
          && mkdir -p tools && printf 'SELFHOST_VER="0.2.433"\n' > tools/px \
          && printf 'a\n' > f.txt && git add -A && git commit -qm init ) >/dev/null 2>&1
    }
    run() { bash "$SELF" --repo "$1" --stamp "$2" ${3:+--wip-out "$3"}; }

    # S1 净树 + head 一致 ⇒ SCOPE-OK wip=0
    mkfix "$T/r1" && H1="$(git -C "$T/r1" rev-parse --short=7 HEAD)"
    printf 'CHAIN-VERDICT=GREEN rc=0 green=307 head=%s\n' "$H1" > "$T/s1"
    out="$(run "$T/r1" "$T/s1" "$T/w1")"; rc=$?
    [ $rc -eq 0 ] && grep -q 'SCOPE-OK .* wip=0' <<< "$out" && ok "S1 净树 ⇒ SCOPE-OK wip=0" || bad "S1 净树被判 $rc：$out"

    # S2 脏树（tracked 改 + 新增 untracked）+ head 一致 ⇒ **仍 SCOPE-OK**（缺陷 896 的回归判据）
    printf 'b\n' >> "$T/r1/f.txt"; printf 'new\n' > "$T/r1/u.txt"
    out="$(run "$T/r1" "$T/s1" "$T/w2")"; rc=$?
    [ $rc -eq 0 ] && grep -q 'SCOPE-OK .* wip=2' <<< "$out" && ok "S2 脏树不再阻断（wip=2 · 缺陷 896）" || bad "S2 脏树仍被判 $rc：$out"
    grep -q 'u.txt' "$T/w2" 2>/dev/null && ok "S2b WIP 明细落盘（含 untracked）" || bad "S2b WIP 明细没落盘"
    git -C "$T/r1" checkout -q -- f.txt; rm -f "$T/r1/u.txt"

    # S3 WIP 碰 tools/px ⇒ SCOPE-REJECT wip-version-source（版本闸读工作树，是真风险）
    printf 'SELFHOST_VER="0.2.999"\n' > "$T/r1/tools/px"
    out="$(run "$T/r1" "$T/s1")"; rc=$?
    [ $rc -eq 2 ] && grep -q 'reason=wip-version-source' <<< "$out" && ok "S3 tools/px 被改 ⇒ REJECT（版本闸失真）" || bad "S3 版本源 WIP 没拦：$rc $out"
    git -C "$T/r1" checkout -q -- tools/px

    # S4 head 不一致 ⇒ REJECT head-mismatch
    printf 'CHAIN-VERDICT=GREEN head=deadbee\n' > "$T/s2"
    out="$(run "$T/r1" "$T/s2")"; rc=$?
    [ $rc -eq 2 ] && grep -q 'reason=head-mismatch' <<< "$out" && ok "S4 被测提交≠HEAD ⇒ REJECT" || bad "S4 head 不一致没拦：$rc $out"

    # S5 stamp 没有 head= ⇒ REJECT stamp-head-missing（不许默认放行）
    printf 'CHAIN-VERDICT=GREEN green=307\n' > "$T/s3"
    out="$(run "$T/r1" "$T/s3")"; rc=$?
    [ $rc -eq 2 ] && grep -q 'reason=stamp-head-missing' <<< "$out" && ok "S5 无 head 字段 ⇒ REJECT（不放行）" || bad "S5 无 head 仍放行：$rc $out"

    # S6 环境不成立 ⇒ rc=3（绝不降级成 OK）
    out="$(run "$T/nope" "$T/s1")"; rc=$?
    [ $rc -eq 3 ] && grep -q 'reason=env-no-repo' <<< "$out" && ok "S6 仓不存在 ⇒ rc=3" || bad "S6 环境守卫失效：$rc $out"

    rm -rf "$T"
    echo "──────────────────────────────"
    echo "PUBLISH-SCOPE-SELFTEST pass=$P fail=$F"
    [ "$F" -eq 0 ] && { echo "SELFTEST=OK"; exit 0; }
    echo "SELFTEST=FAIL"; exit 1
fi

scope_check "$REPO" "$STAMP" "$WIP_OUT"
exit $?
