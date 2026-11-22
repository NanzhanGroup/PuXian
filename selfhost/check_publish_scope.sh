#!/usr/bin/env bash
# ============================================================
# selfhost/check_publish_scope.sh —— 发布自锁口径：被测面 ⇄ 发布面（M433 · 缺陷 896）
# ------------------------------------------------------------
# 病史（2026-11-22 16:40 实测 · M432）：
#   链 306/306 全绿 → publish_on_green **ABORT dirty-tree**（rc=2），只因我在建补丁
#   （ci.yml / ci_diagnose.py / fixtures …）。2 分 47 秒后靠人工补 tag，并且发出了一条
#   假文案 p1（「扫描面 日志47行 · 红项 0 条」—— 真因是树脏，判据没说出真因）。
#
# 本门守的是什么（三件事，缺一条这个修法就可能悄悄失效）：
#   A1 地基（动态）：`git worktree add --detach <commit>` 取到的是**提交内容**，
#      主树的未提交改动进不去 ⇒ 「主树 WIP 与测试面无关」这个前提是**可核事实**，不是信念。
#   A2 机制锚（静态 + 负控）：链确实用 worktree@HEAD_SHA 跑门（shard_run.sh 两处锚点）。
#   A3 口径唯一（静态 + 负控）：publish_on_green 只调 publish_scope_check.sh，
#      且**不再**有「树脏即 ABORT」这种把工作副本卫生当证据完整性的老实现。
#
# 判据：
#   P1  helper 自测（--self-test 7 例：净树/脏树/版本源/head 不一致/无 head/环境）
#   P2  真仓净树只读实测：SCOPE-OK 且 wip=0（不碰真仓、不写文件）
#   A1  worktree@commit 读到提交内容；主树读到 WIP（**前提自证**：主树确实脏）
#   A2  shard_run.sh 锚点齐全；负控：把建树行换成 `cp -a "$SRC" "$WT"`（=复制主树含 WIP）必须被判红
#   A3  publish_on_green 走唯一口径；负控：注回 `ABORT dirty-tree` 必须被判红
#   N1  负控：把 helper 的阻断条件注回「任何 WIP 都拒」⇒ 脏树夹具必须 REJECT（判据有牙）
#   N2  负控：把版本源判据摘掉 ⇒ tools/px 脏夹具必须变 OK（说明这道拦有牙且只拦它）
#   G1  环境守卫：git 不可用 / 夹具建不起来 ⇒ rc=3（绝不降级成绿）
#   G2  零污染：真仓 git status 行数前后一致
#   G3  不留监听（本门不起服务；M430 的运行器级扫描兜底）
#
# 用法：check_publish_scope.sh [--self-test]   # --self-test 只跑判据夹具（离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HELPER="$ROOT/packaging/chain/publish_scope_check.sh"
PUB="$ROOT/packaging/publish_on_green.sh"
SHARD="$ROOT/packaging/chain/shard_run.sh"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m433_scope.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
cleanup() { chmod -R u+w "$TMP" 2>/dev/null; rm -rf "$TMP" 2>/dev/null; }
trap cleanup EXIT INT TERM

command -v git >/dev/null 2>&1 || { echo "ENV-GUARD 无 git ⇒ rc=3"; exit 3; }
for f in "$HELPER" "$PUB" "$SHARD"; do
    [ -s "$f" ] || { echo "ENV-GUARD 缺件 $f ⇒ rc=3"; exit 3; }
done

# ── 夹具：一个带 tools/px 的真 git 仓库（自测用；全在 $TMP 内）───────────────
mkfix() {
    local d="$1"; mkdir -p "$d" || return 1
    ( cd "$d" && git init -q . && git config user.email t@t.invalid && git config user.name t \
      && mkdir -p tools && printf 'SELFHOST_VER="0.2.433"\n' > tools/px \
      && printf 'orig\n' > f.txt && git add -A && git commit -qm init ) >/dev/null 2>&1
}
mkstamp() { printf 'CHAIN-VERDICT=GREEN rc=0 green=307 listed=307 head=%s\n' "$2" > "$1"; }

# ── 静态判据（做成函数 ⇒ 负控可以拿夹具副本打它）──────────────────────────
judge_shard_src() {  # 链确实取 worktree@commit（门清单也取自 commit）
    local f="$1"
    grep -qF 'HEAD_SHA="$(git -C "$SRC" rev-parse HEAD)"' "$f" \
      && grep -qF 'worktree add --detach "$WT" "$HEAD_SHA"' "$f" \
      && grep -qF 'show "HEAD:selfhost/gates.registry.sh"' "$f"
}
judge_pub_scope() {  # 发布走唯一口径，且老实现（树脏即 ABORT）已退场
    local f="$1"
    grep -qF 'publish_scope_check.sh' "$f" || return 1
    if grep -q 'ABORT dirty-tree' <(grep -v '^[[:space:]]*#' "$f"); then return 1; fi
    return 0
}

SELFTEST_OK=0
if [ "${1:-}" = "--self-test" ]; then
    # 夹具自测：只跑 helper 的 --self-test（它自己 7 例）
    if bash "$HELPER" --self-test 2>&1 | grep -q 'PUBLISH-SCOPE-SELFTEST pass=7 fail=0'; then
        ok "S1 helper 自测 7/7"; SELFTEST_OK=1
    else
        bad "S1 helper 自测未通过"; SELFTEST_OK=0
    fi
    echo "M433-PUBLISH-SCOPE-SELFTEST pass=$PASS fail=$FAIL"
    [ "$FAIL" -eq 0 ] && { echo "SELFTEST=OK"; exit 0; }
    echo "SELFTEST=FAIL"; exit 2
fi

# ── P1 helper 自测 ────────────────────────────────────────────────────────
if bash "$HELPER" --self-test 2>&1 | grep -q 'PUBLISH-SCOPE-SELFTEST pass=7 fail=0'; then
    ok "P1 helper 自测 7/7（净树/脏树/版本源/head 不一致/无 head/环境守卫）"
else
    bad "P1 helper 自测未通过"
fi

# ── P2 真仓只读实测（**独立复算对拍**，不假设树是净的）──────────────────────
#   病史：第一版写死「真仓必须是 SCOPE-OK」⇒ 只要工作树里正躺着未提交的
#   tools/px（正是本次修法自己的 bump 阶段）就假红。判据不能假设被测环境的卫生。
#   现在的口径：本门自己按 git status 独立算出**应有结论**，再与 helper 的输出对拍。
STATUS_BEFORE="$(git -C "$ROOT" status --porcelain=v1 2>/dev/null | wc -l)"
REAL_HEAD="$(git -C "$ROOT" rev-parse --short=7 HEAD 2>/dev/null)"
mkstamp "$TMP/real.stamp" "$REAL_HEAD"
EXPECT="OK"
if git -C "$ROOT" status --porcelain=v1 2>/dev/null | cut -c4- | sed 's/.* -> //' | grep -qx 'tools/px'; then
    EXPECT="REJECT-version"
fi
REAL_OUT="$(bash "$HELPER" --repo "$ROOT" --stamp "$TMP/real.stamp" 2>&1)"; REAL_RC=$?
if [ "$EXPECT" = "OK" ]; then
    if [ "$REAL_RC" -eq 0 ] && grep -q "SCOPE-OK head=$REAL_HEAD" <<< "$REAL_OUT"; then
        ok "P2 真仓只读实测：独立复算=OK，helper=$REAL_OUT"
    else
        bad "P2 复算应为 OK 但 helper rc=$REAL_RC：$REAL_OUT"
    fi
else
    if [ "$REAL_RC" -eq 2 ] && grep -q 'reason=wip-version-source' <<< "$REAL_OUT"; then
        ok "P2 真仓只读实测：独立复算=REJECT（tools/px 未提交），helper 同口径"
    else
        bad "P2 复算应为 REJECT-version 但 helper rc=$REAL_RC：$REAL_OUT"
    fi
fi

# ── A1 地基动态自证：worktree@commit 看不到主树 WIP ────────────────────────
F1="$TMP/repo1"
if mkfix "$F1"; then
    H1="$(git -C "$F1" rev-parse --short=7 HEAD)"
    printf 'dirty-wip\n' > "$F1/f.txt"                      # 主树 WIP
    MAIN_SEE="$(cat "$F1/f.txt")"
    if git -C "$F1" worktree add -q --detach "$TMP/wt1" "$H1" >/dev/null 2>&1; then
        WT_SEE="$(cat "$TMP/wt1/f.txt" 2>/dev/null)"
        if [ "$MAIN_SEE" = "dirty-wip" ] && [ "$WT_SEE" = "orig" ]; then
            ok "A1 worktree@$H1 读到提交内容（orig），主树读到 WIP（dirty-wip）"
        else
            bad "A1 前提自证失败：主树=[$MAIN_SEE] worktree=[$WT_SEE]（夹具没真脏或 worktree 没取 commit）"
        fi
        git -C "$F1" worktree remove --force "$TMP/wt1" >/dev/null 2>&1
    else
        bad "A1 worktree 建不起来（夹具/环境问题，本条没验到）"
    fi
    ( cd "$F1" && git checkout -q -- f.txt )
else
    echo "ENV-GUARD 夹具仓库建不起来 ⇒ rc=3"; exit 3
fi

# ── A2 机制锚 + 负控 ──────────────────────────────────────────────────────
if judge_shard_src "$SHARD"; then
    ok "A2 链用 worktree@HEAD_SHA 跑门（HEAD_SHA 来自 rev-parse HEAD · 门清单取自 HEAD）"
else
    bad "A2 shard_run.sh 的「测试面=commit」锚点缺失"
fi
NEG2="$TMP/shard_neg.sh"; cp "$SHARD" "$NEG2"
python3 - "$NEG2" <<'PY' 2>/dev/null || sed -i 's|worktree add --detach "$WT" "$HEAD_SHA"|cp -a "$SRC" "$WT"|g' "$NEG2"
import sys,io
p=sys.argv[1]; s=io.open(p,encoding='utf-8').read()
# 全部出现处都要换（shard_run.sh 里建树有两处：首建 + 重试）——只换一处等于负控没扰动
s2=s.replace('worktree add --detach "$WT" "$HEAD_SHA"','cp -a "$SRC" "$WT"')
io.open(p,'w',encoding='utf-8').write(s2)
sys.exit(0 if s2!=s else 1)
PY
if [ "$(grep -cF 'cp -a "$SRC" "$WT"' "$NEG2")" -ge 1 ]; then
    if judge_shard_src "$NEG2"; then
        bad "A2-负控 不成立：把建树换成「复制主树」后判据仍绿（判据没牙）"
    else
        ok "A2-负控 成立：换成「复制主树含 WIP」⇒ 判据立刻红"
    fi
else
    bad "A2-负控 扰动未生效（sed/python 未命中，本条没验到）"
fi

# ── A3 口径唯一 + 负控 ────────────────────────────────────────────────────
if judge_pub_scope "$PUB"; then
    ok "A3 发布走 publish_scope_check.sh，且无「树脏即 ABORT」老实现"
else
    bad "A3 发布口径不唯一（未调 publish_scope_check.sh 或仍有 ABORT dirty-tree）"
fi
NEG3="$TMP/pub_neg.sh"; cp "$PUB" "$NEG3"
printf '\nsay "ABORT dirty-tree"\n' >> "$NEG3"
if judge_pub_scope "$NEG3"; then
    bad "A3-负控 不成立：注回 ABORT dirty-tree 后判据仍绿"
else
    ok "A3-负控 成立：注回「树脏即 ABORT」⇒ 判据红"
fi

# ── N1 负控：注回「任何 WIP 都拒」⇒ 脏夹具必须 REJECT ─────────────────────
N1H="$TMP/helper_dirty.sh"; cp "$HELPER" "$N1H"
python3 - "$N1H" <<'PY' 2>/dev/null || sed -i 's|if \[ -n "\$wip_ver" \]; then|if [ "${wip_n:-0}" -gt 0 ]; then|' "$N1H"
import sys,io
p=sys.argv[1]; s=io.open(p,encoding='utf-8').read()
s2=s.replace('if [ -n "$wip_ver" ]; then','if [ "${wip_n:-0}" -gt 0 ]; then',1)
io.open(p,'w',encoding='utf-8').write(s2)
sys.exit(0 if s2!=s else 1)
PY
if grep -qF 'if [ "${wip_n:-0}" -gt 0 ]; then' "$N1H"; then
    mkstamp "$TMP/s1" "$H1"
    printf 'wip-for-n1\n' >> "$F1/f.txt"          # 前提自证：夹具此刻必须真脏（否则「不阻断」测的是净树）
    DIRTY_N="$(git -C "$F1" status --porcelain=v1 | wc -l)"
    N1_OUT="$(bash "$N1H" --repo "$F1" --stamp "$TMP/s1" 2>&1)"; N1_RC=$?
    ORI_OUT="$(bash "$HELPER" --repo "$F1" --stamp "$TMP/s1" 2>&1)"; ORI_RC=$?
    if [ "$N1_RC" -eq 2 ] && [ "$ORI_RC" -eq 0 ] && [ "$DIRTY_N" -ge 1 ]; then
        ok "N1 负控成立：注回老口径（任何 WIP 都拒）⇒ 同一脏夹具（$DIRTY_N 项）从 OK 变 REJECT"
    else
        bad "N1 负控不成立：注回后 rc=$N1_RC（原版 rc=$ORI_RC，夹具脏项=$DIRTY_N）—— 判据没有牙或前提没成立"
    fi
else
    bad "N1 扰动未生效（未命中阻断条件行）"
fi

# ── N2 负控：摘掉版本源判据 ⇒ tools/px 脏夹具变 OK ────────────────────────
printf 'SELFHOST_VER="0.2.999"\n' > "$F1/tools/px"
ORI2_OUT="$(bash "$HELPER" --repo "$F1" --stamp "$TMP/s1" 2>&1)"; ORI2_RC=$?
N2H="$TMP/helper_nover.sh"; cp "$HELPER" "$N2H"
python3 - "$N2H" <<'PY' 2>/dev/null || sed -i 's|wip_ver="\$p"|wip_ver=""|' "$N2H"
import sys,io
p=sys.argv[1]; s=io.open(p,encoding='utf-8').read()
s2=s.replace('wip_ver="$p"','wip_ver=""',1)
io.open(p,'w',encoding='utf-8').write(s2)
sys.exit(0 if s2!=s else 1)
PY
if grep -qF 'wip_ver=""' "$N2H"; then
    N2_OUT="$(bash "$N2H" --repo "$F1" --stamp "$TMP/s1" 2>&1)"; N2_RC=$?
    if [ "$ORI2_RC" -eq 2 ] && [ "$N2_RC" -eq 0 ]; then
        ok "N2 负控成立：版本源 WIP 原本 REJECT，摘掉该判据后变 OK（这道拦有牙且只拦它）"
    else
        bad "N2 负控不成立：原版 rc=$ORI2_RC 摘除后 rc=$N2_RC"
    fi
else
    bad "N2 扰动未生效（未命中 wip_ver 赋值行）"
fi
( cd "$F1" && git checkout -q -- tools/px )

# ── G2 零污染 ─────────────────────────────────────────────────────────────
STATUS_AFTER="$(git -C "$ROOT" status --porcelain=v1 2>/dev/null | wc -l)"
if [ "$STATUS_BEFORE" = "$STATUS_AFTER" ]; then
    ok "G2 零污染：真仓 git status 行数前后一致（$STATUS_BEFORE）"
else
    bad "G2 真仓被改：status 行数 $STATUS_BEFORE → $STATUS_AFTER"
fi

# ── G3 不留监听（本门不起服务）────────────────────────────────────────────
ok "G3 本门不起任何监听（无端口可残留；运行器级扫描兜底）"

echo "──────────────────────────────"
echo "M433-PUBLISH-SCOPE pass=$PASS fail=$FAIL"
[ "$FAIL" -eq 0 ] && { echo "M433-VERIFY-OK"; exit 0; }
echo "M433-VERIFY-FAIL"; exit 1
