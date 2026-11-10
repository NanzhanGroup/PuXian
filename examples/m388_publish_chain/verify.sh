#!/usr/bin/env bash
# ============================================================================
# m388_publish_chain —— 发布链「静默/重复」判据（M388）
#
# 缺陷来源（全部 2026-11-10 实测）：
#   698-② 通知正文把 tag sha 打成字面 $TAGSHA（用户手机看到的就是这串字符）
#   698-③ make_tag 的 hook 发一条 + publish 脚本再发一条 ⇒ 一次发布两条通知
#   703    发布脚本抢不到锁 exit 0 静默 ⇒ 孤儿锁时永远不发布且无人知道
#   （706 在仓库外：idle 判据把 release-watch 当实活 —— 由 dy-ops-watch --selftest 守）
#
# 判据 G1..G5 + 26 条子自测。负控 N1/N2 已在 selftest 内部。
# ============================================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
PUB="$ROOT/packaging/publish_on_green.sh"
BODY="$ROOT/packaging/publish_notify_body.sh"
FAIL=0
ok()  { printf 'OK   %-3s %s\n' "$1" "$2"; }
no()  { printf 'FAIL %-3s %s\n' "$1" "$2"; FAIL=$((FAIL+1)); }

# ── 前提自证 P1：被测件存在且非空（否则后面的"没有坏模式"全是假绿） ───────
for f in "$PUB" "$BODY"; do
    if [ -s "$f" ]; then ok P1 "被测件存在且有内容：${f#$ROOT/}（$(wc -l < "$f") 行）"
    else no P1 "被测件缺失或为空：$f"; fi
done
[ "$FAIL" -eq 0 ] || { echo "M388-VERIFY-FAIL 被测件不齐"; exit 1; }

# ── G1 通知正文不许再把 sha 写成字面量（698-②） ───────────────────────────
# 只查非注释行（CRITERIA ⑥：注释里的代码也算代码；但这里是注释在**描述缺陷**，
# 不是实现。判据必须落在实现面上，否则会把「写清楚病根」判成「又犯了这个病」。）
if grep -v '^[[:space:]]*#' "$BODY" 2>/dev/null | grep -q 'TAGSHA'; then
    no G1 "publish_notify_body.sh 实现里出现 TAGSHA —— sha 必须只从参数 \$2 来"
else
    ok G1 "通知正文只从参数取 sha（实现面无 TAGSHA）"
fi
if grep -q '\\\$TAGSHA' "$PUB" 2>/dev/null; then
    no G1b "publish_on_green.sh 里仍有转义后的 \$TAGSHA 字面量"
else
    ok G1b "publish_on_green.sh 无 \$TAGSHA 字面量"
fi

# ── G2 双发禁用：hook 置空必须在 make_tag 调用之前（698-③） ───────────────
LN_HOOK=$(grep -n 'PX_RELEASE_NOTIFY_HOOK=/bin/true' "$PUB" | head -1 | cut -d: -f1)
LN_TAG=$(grep -n 'packaging/make_tag.sh' "$PUB" | head -1 | cut -d: -f1)
if [ -n "$LN_HOOK" ] && [ -n "$LN_TAG" ] && [ "$LN_HOOK" -lt "$LN_TAG" ]; then
    ok G2 "make_tag 之前已置 hook=/bin/true（发布链单出口，不会双发）"
else
    no G2 "hook 置空缺失或在 make_tag 之后（hook=${LN_HOOK:-无} tag=${LN_TAG:-无}）"
fi

# ── G3 子进程不继承锁 fd（703 的根因） ───────────────────────────────────
BAD_SLEEP=$(grep -n 'sleep [0-9]' "$PUB" | grep -v '9>&-' | grep -v '^\s*#' | wc -l)
if [ "$BAD_SLEEP" -eq 0 ]; then
    ok G3 "所有 sleep 都带 9>&-（子进程不继承锁 fd）"
else
    no G3 "有 $BAD_SLEEP 处 sleep 没带 9>&-：$(grep -n 'sleep [0-9]' "$PUB" | grep -v '9>&-' | head -3 | tr '\n' ' ')"
fi

# ── G4 孤儿判定必须排除自身（不自欺：自己 exec 9> 打开锁会被 fuser 列出） ──
if grep -q '"\$_p" = "\$\$"' "$PUB"; then
    ok G4 "孤儿判定排除自身 pid（否则永远判「有人正常持锁」）"
else
    no G4 "孤儿判定未排除自身 pid ⇒ 孤儿锁永远发现不了"
fi

# ── G5 抢锁失败必须区分「同脚本在跑」与「孤儿」 ──────────────────────────
if grep -q 'WARN-LOCK-ORPHAN' "$PUB" && grep -q 'exit 3' "$PUB"; then
    ok G5 "抢锁失败区分孤儿（WARN-LOCK-ORPHAN + exit 3）"
else
    no G5 "抢锁失败仍是单一 exit 0（静默失效点）"
fi

# ── 子自测：26 条（含 C1..C10 / N1..N2 / P1..P2） ─────────────────────────
ST="$HERE/selftest.sh"
if [ -x "$ST" ]; then
    OUT="$(bash "$ST" 2>&1)"; SRC=$?
    if printf '%s' "$OUT" | grep -q 'PUBLISH-SELFTEST-OK' && [ "$SRC" -eq 0 ]; then
        ok G6 "$(printf '%s' "$OUT" | grep 'PUBLISH-SELFTEST-OK' | tail -1)"
    else
        no G6 "子自测未通过（rc=$SRC）：$(printf '%s' "$OUT" | grep 'FAIL' | head -5 | tr '\n' ' ')"
    fi
else
    no G6 "子自测缺失：$ST"
fi

echo
if [ "$FAIL" -eq 0 ]; then echo "M388-VERIFY-OK"; exit 0; fi
echo "M388-VERIFY-FAIL 失败 $FAIL 项"; exit 1
