#!/usr/bin/env bash
# M287s2 · 「失败确认步 / FLAKE」自证（11 判据 · 4 组）
#   手法：把 run_upstream_tests.sh 放进假树，用**假 tools/px** 控制「第几次调用失败」，
#         从而在**真实的判据代码**上端到端验证 FLAKE 分支（不是另写一份逻辑）。
#   ⚠️ 首版踩：把模式判据写成 `[ "$1" = first-fail ]` —— 而 $1 是子命令（`build`）
#      ⇒ 假件**从不失败** ⇒ ①③ 假绿。⇒ 模式必须**烘焙进**生成的脚本。
set -u
REAL="$(cd "$(dirname "$(readlink -f "$0")")/.." && pwd)"
T="${T:-/tmp/m287s2_fake}"
n=0; tot=0
ok()  { n=$((n+1)); echo "  ✅ $*"; }
bad() { echo "  ❌ $*"; }
jud() { tot=$((tot+1)); if [ "$1" = 1 ]; then ok "$2"; else bad "$2"; fi; }

setup() {   # $1 = first-fail | always-fail | normal
    rm -rf "$T"; mkdir -p "$T/selfhost" "$T/tools"
    ln -s "$REAL/registry"       "$T/registry"
    ln -s "$REAL/upstream-tests" "$T/upstream-tests"
    cp "$REAL/selfhost/run_upstream_tests.sh" "$T/selfhost/"
    case "$1" in
        # ⚠️ 判据必须锚在**第一次 `build`**（harness 启动时会先跑一次 `px --version` 取版本号，
        #   首版锚「第一次调用」⇒ 被 --version 吃掉 ⇒ ①③ 假绿（本轮实测）。
        first-fail)  GUARD="if [ \"\$1\" = build ] && [ ! -f $T/seen ]; then touch $T/seen; echo FAKE-FIRST-BUILD-FAIL; exit 1; fi" ;;
        always-fail) GUARD='echo FAKE-ALWAYS-FAIL; exit 1' ;;
        *)           GUARD=':' ;;
    esac
    {
        echo '#!/usr/bin/env bash'
        echo "C=$T/count"
        echo 'k=$(cat "$C" 2>/dev/null || echo 0); k=$((k+1)); echo $k > "$C"'
        printf '%s\n' "$GUARD"
        echo "exec $REAL/tools/px \"\$@\""
    } > "$T/tools/px"
    chmod +x "$T/tools/px"
}

run() {     # $1=mode [额外参数…]  → stdout: rc
    local mode="$1"; shift
    setup "$mode"
    timeout -k 20 600 bash "$T/selfhost/run_upstream_tests.sh" \
        --only base58_test --track build --no-fixtures \
        --json "$T/out.json" "$@" > "$T/run.log" 2>&1
    echo $?
}

echo "== ① 首跑失败 / 确认步通过 ⇒ FLAKE（不判红，rc=0）=="
rc=$(run first-fail)
tail -3 "$T/run.log" | sed 's/^/     /'
jud "$([ "$rc" = 0 ] && echo 1 || echo 0)" "rc=0（抖动不判非零）· 实得 $rc"
jud "$(grep -q '抖动：首跑失败' "$T/run.log" && echo 1 || echo 0)" "输出**响亮**报抖动（告警行）"
jud "$(grep -q '"flake": 1' "$T/out.json" && echo 1 || echo 0)" "--json flake=1（实得：$(grep -o '"flake": [0-9]*' "$T/out.json"))"
# ⚠️ 设计：抖动**单独成桶**（不重复计入 pass），故 pass+flake 守恒
jud "$(python3 -c "
import json
d=json.load(open('$T/out.json'))
print(1 if (d['pass']==0 and d['flake']==1 and d['pass']+d['flake']==1) else 0)
" 2>/dev/null || echo 0)" "计数守恒：抖动单独成桶（pass=0/flake=1，不重复计）"

echo "== ② 两次都失败（真缺陷）⇒ 仍是 FAIL（**不被掩盖**）=="
rc=$(run always-fail)
jud "$([ "$rc" != 0 ] && echo 1 || echo 0)" "rc≠0（真失败仍判非零）· 实得 $rc"
jud "$(grep -q '"fail": 1' "$T/out.json" && echo 1 || echo 0)" "--json fail=1（未记为 flake）"
jud "$(grep -q '首跑失败 ⇒ 确认步' "$T/run.log" && echo 1 || echo 0)" "确认步**真的跑了**（留痕）"

echo "== ③ --no-confirm：关掉确认步 ⇒ 首跑失败即 FAIL =="
rc=$(run first-fail --no-confirm)
jud "$([ "$rc" != 0 ] && echo 1 || echo 0)" "rc≠0 · 实得 $rc"
jud "$([ "$(grep -c '确认步' "$T/run.log")" = 0 ] && echo 1 || echo 0)" "确认步未触发"

echo "== ④ 正常路径无回归 ⇒ flake=0 =="
rc=$(run normal)
jud "$([ "$rc" = 0 ] && echo 1 || echo 0)" "rc=0 · 实得 $rc"
jud "$(grep -q '"flake": 0' "$T/out.json" && echo 1 || echo 0)" "flake=0（正常路径不误报）"

echo
echo "── 自证：$n/$tot ──"
[ "$n" = "$tot" ] && { echo "M287S2-CONFIRM-SELFTEST-OK"; exit 0; }
echo "M287S2-CONFIRM-SELFTEST-FAIL"; exit 1
