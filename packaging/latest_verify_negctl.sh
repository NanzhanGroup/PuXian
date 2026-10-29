#!/usr/bin/env bash
# ============================================================
# packaging/latest_verify_negctl.sh —— `latest_verify.sh` 的负控（M354 · 缺陷 620 第二半）
#
# 判据改动必须配负控回放（M209）。这里证的是**装置自己**：
#   n1 有 tag、没有 Release（伪造 v0.2.999）        ⇒ 必须 **BLOCKED**（修前会写 DONE/GREEN）
#   n2 有 tag、有 Release，但 latest 指向旧版本      ⇒ 必须 **STALE**（不能只报「没结论」）
#   n3 latest == 期望 tag                            ⇒ 必须 **OK**（正控：别把绿的判红）
#   n4 取不到 API/fixture（目录空）                   ⇒ 必须 **BLOCKED(api-unreadable)**，**绝不能** OK
#
# 全部离线（`latest_guard.sh --api-fixture`）⇒ 确定性、不依赖网络/发布状态。
# 用法：packaging/latest_verify_negctl.sh      （退出码 0 = 四例全落在预期）
# ============================================================
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LV="$ROOT/packaging/latest_verify.sh"
W="$(mktemp -d /tmp/lvneg.XXXXXX)"
trap 'rm -rf "$W"' EXIT
PASS=0; FAIL=0
ok(){ echo "  PASS $*"; PASS=$((PASS+1)); }
bad(){ echo "  FAIL $*"; FAIL=$((FAIL+1)); }

mkfix(){   # $1=dir $2=最高tag $3=latest tag $4=是否造 expected_release(1/0)
    local d="$1" top="$2" cur="$3" hasrel="$4"
    mkdir -p "$d"
    printf '[{"name":"%s"}]\n' "$top" > "$d/tags.json"
    printf '{"tag_name":"%s"}\n' "$cur" > "$d/latest.json"
    [ "$hasrel" = "1" ] && printf '{"id":1,"tag_name":"%s"}\n' "$top" > "$d/expected_release.json"
    return 0
}

run_case(){   # $1=名 $2=fixture $3=期望verdict ; → 设 RC / V
    local name="$1" fix="$2" want="$3" out="$W/$1.md"
    LV_FIXTURE="$fix" LV_POLL_SEC=0 LV_DEADLINE_SEC=0 \
        bash "$LV" v0.2.999 "$out" > "$W/$1.stdout" 2>&1
    RC=$?
    V="$(grep -oE '^LATEST-VERDICT=[A-Z]+' "$out" 2>/dev/null | tail -1 | cut -d= -f2)"
    V="${V:-<none>}"
    local wrc=0
    case "$want" in OK) wrc=0;; STALE) wrc=1;; *) wrc=3;; esac
    if [ "$V" = "$want" ] && [ "$RC" = "$wrc" ]; then
        ok "$name ⇒ $V（rc=$RC）"
    else
        bad "$name ⇒ 期望 $want/rc=$wrc，实测 ${V}/rc=$RC"
        sed 's/^/       | /' "$out" 2>/dev/null | tail -6
    fi
}

echo "── latest_verify 负控（M354 · 缺陷 620 第二半）"
mkfix "$W/f1" v0.2.999 v0.2.998 0;  run_case n1-blocked "$W/f1" BLOCKED
mkfix "$W/f2" v0.2.999 v0.2.998 1;  run_case n2-stale   "$W/f2" STALE
mkfix "$W/f3" v0.2.999 v0.2.999 1;  run_case n3-ok      "$W/f3" OK
mkdir -p "$W/f4";                   run_case n4-unreadable "$W/f4" BLOCKED

# n1 额外断言：BLOCKED 必须**写清下一步**（不能只说「阻塞了」）
if grep -q 'rerun-failed-jobs' "$W/n1-blocked.md" 2>/dev/null; then
    ok "n1 的 BLOCKED 带「下一步」（rerun-failed-jobs / release_reconcile）"
else
    bad "n1 的 BLOCKED 没写下一步 ⇒ 读的人不知道该怎么解"
fi
# n1 额外断言：BLOCKED **不得**同时出现 GREEN/DONE 式结论（修前的病：PENDING 当终态还写 DONE）
if grep -qE 'VERDICT=(OK|GREEN)$' "$W/n1-blocked.md" 2>/dev/null; then
    bad "n1 里同时出现 OK/GREEN ⇒ 结论自相矛盾"
else
    ok "n1 不冒充成功（无 OK/GREEN 结论行）"
fi

echo
if [ "$FAIL" -eq 0 ]; then echo "LATEST-VERIFY-NEGCTL-OK（$PASS/$((PASS+FAIL))）"; exit 0; fi
echo "LATEST-VERIFY-NEGCTL-FAIL（$PASS 通过 / $FAIL 失败）"; exit 1
