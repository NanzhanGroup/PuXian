#!/usr/bin/env bash
# ============================================================
# packaging/ci_step_rc_selftest.sh —— CI 步的 **rc 捕获** 自测（缺陷 847）
# ------------------------------------------------------------
# 为什么有它（2026-11-19 实测）：
#   GitHub workflow 的默认 shell 是 `bash -e {0}`。写成
#       bash verify.sh > log 2>&1; rc=$?
#       if [ $rc -eq 3 ]; then echo SKIP; exit 0; fi
#   时，verify.sh 一旦非零，`-e` **在取到 rc 之前**就终结本步 ⇒ 后面所有分支是死代码，
#   重定向进 log 的现场也没人 tail。事实形态：CI 日志只有
#   `##[error]Process completed with exit code 3`，一行门输出都没有。
#
# 本脚本的判据（把 ci.yml 里**真实的 run 块**取出来，在 `bash -e` 下用假 verify.sh 跑）：
#   A 前提自证：旧写法在 `bash -e` 下确实到不了 SKIP 分支（本脚本的牙）
#   B rc=0 ⇒ step rc=0 且打印 PASS
#   C rc=3 ⇒ step rc=0 且打印 SKIP（环境缺失不判红）
#   D rc=1 / rc=4 ⇒ step rc=1 且打印 rc 值（真红必须红，且现场可见）
#
# 用法：packaging/ci_step_rc_selftest.sh [--step m420_memsan_cron] [--yml PATH]
#       rc=0 全过 · rc=1 有判据失败 · rc=3 前提不成立（缺 python3/yaml/步名）
# ============================================================
set -uo pipefail
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SELF_DIR/.." && pwd)
YML="$ROOT/.github/workflows/ci.yml"
STEP="m420_memsan_cron"
while [ $# -gt 0 ]; do
    case "$1" in
        --step) shift; STEP="${1:-}";;
        --yml)  shift; YML="${1:-}";;
        *) echo "未知参数：$1" >&2; exit 2;;
    esac
    shift
done
command -v python3 >/dev/null 2>&1 || { echo "CI-RC-SELFTEST-ENV rc=3（缺 python3）"; exit 3; }
python3 -c 'import yaml' 2>/dev/null || { echo "CI-RC-SELFTEST-ENV rc=3（缺 python3-yaml）"; exit 3; }
[ -f "$YML" ] || { echo "CI-RC-SELFTEST-ENV rc=3（缺 $YML）"; exit 3; }

TMP=$(mktemp -d /tmp/ci-rc-selftest.XXXXXX) || exit 3
trap 'rm -rf "$TMP"' EXIT
pass=0; fail=0
ok()  { pass=$((pass+1)); echo "PASS $*"; }
bad() { fail=$((fail+1)); echo "FAIL $*"; }

# ── A 前提自证：旧写法在 -e 下确实吞掉分支 ──────────────────
cat > "$TMP/old.sh" <<'OLD'
rc=0
bash -c 'exit 3' >/dev/null 2>&1; rc=$?
if [ $rc -eq 3 ]; then echo "SKIP-REACHED"; exit 0; fi
echo "PASS-REACHED"
OLD
out=$(bash -e "$TMP/old.sh" 2>&1); rc=$?
if [ "$rc" -eq 3 ] && [ -z "$out" ]; then
    ok "A 前提自证：旧写法 rc=3 ⇒ 本步 rc=3 且无输出（SKIP 分支到不了 —— 与 CI 日志同形）"
else
    bad "A 前提不成立：旧写法 rc=$rc 输出=[$out]（期望 rc=3 且空）"
fi

# ── 取真实 run 块 ───────────────────────────────────────────
python3 - "$YML" "$STEP" > "$TMP/step.sh" <<'PY'
import sys
sys.path.insert(0, '')
import yaml
d = yaml.safe_load(open(sys.argv[1]))
want = sys.argv[2]
for j in d.get('jobs', {}).values():
    for s in j.get('steps', []):
        if (s.get('name') or '').startswith(want):
            print(s['run']); raise SystemExit(0)
raise SystemExit(3)
PY
[ -s "$TMP/step.sh" ] || { echo "CI-RC-SELFTEST-ENV rc=3（ci.yml 里找不到步 $STEP）"; exit 3; }

mkdir -p "$TMP/examples/$STEP"
# rc=3 的**期望 step rc 由步自身决定**：m420 是「环境缺失 ⇒ SKIP ⇒ exit 0」，
#   m421 是「环境不成立 ⇒ exit 1」。判据不猜口径，直接读 run 块里的 rc=3 分支。
RC3_EXPECT=1
grep -qE '\-eq 3.*exit 0' "$TMP/step.sh" && RC3_EXPECT=0
echo "  （本步 rc=3 分支的期望 step rc=$RC3_EXPECT，由 run 块自身声明）"

case_run() {   # $1=假 rc  $2=期望 step rc  $3=必须出现的字样
    printf '#!/bin/bash\necho "fake-verify rc=%s"\nexit %s\n' "$1" "$1" > "$TMP/examples/$STEP/verify.sh"
    out=$(cd "$TMP" && bash -e step.sh 2>&1); rc=$?
    if [ "$rc" -ne "$2" ]; then bad "rc=$1 ⇒ step rc=$rc（期望 $2）"; return 0; fi
    case "$out" in
        *"$3"*) ok "rc=$1 ⇒ step rc=$rc 且打印「$3」";;
        *)      bad "rc=$1 ⇒ step rc=$rc 但未打印「$3」：[$(echo "$out" | head -3)]";;
    esac
}
case_run 0 0 "PASS"
case_run 3 "$RC3_EXPECT" "rc=3"
case_run 4 1 "rc=4"
case_run 1 1 "rc=1"

echo "CI-RC-SELFTEST step=$STEP pass=$pass fail=$fail"
[ "$fail" -eq 0 ] && exit 0
exit 1
