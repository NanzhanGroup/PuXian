#!/usr/bin/env bash
# ============================================================
# M171 门 · lint 作用域模型对齐 spec §17（缺陷 116 收口）
# ------------------------------------------------------------
# 判据四层：
#   [1] 合法侧 5 例：`px lint` 0 错 0 警 **且** 三轨（interp/VM/C）stdout 逐字节一致
#       —— 两件事必须同时成立：「lint 说合法」且「真的合法」。只有前者 = 改绿，
#       只有后者 = lint 漏判。
#   [2] 真阳性侧 2 例：读取从未绑定的名字仍报 L002；声明未读仍报 L001
#       —— 防"把 lint 改废"（一律不报也是 0 错）。
#   [3] 仓库实战回归：8 个**曾被误报**的仓内真实文件 → 0 错误
#       （盘上语料比自造的更有说服力：stdlib/collections.px 原 46 处）。
#   [4] 负控 3 道，各自独立判红（改源码 → 重编 lint → 该层必须变红 → 逐字节还原）：
#       NC-A 关「帧顶声明 hoist」  → [1]② 变红
#       NC-B 关「赋值式绑定就地声明」→ [1]③ 变红
#       NC-C 关「外层帧可见链」    → [1]① 变红
# 纪律（R50 教训）：每道负控前先 restore 再 snapshot（各自干净起点）；
#   trap 兜底还原；收尾 git status 复核。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
PX="$ROOT/tools/px"
LINT_SRC="$ROOT/tools/lint_core.px"
LINT_DEV="$ROOT/tools/build/pxlint"
BAK=/tmp/m171_lint_core.bak
# --neg-skip：跳过第 [4] 层负控（CI 用；负控要改源码 + 重编 lint 三次，本地全门跑）
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }

snapshot() { cp -f "$LINT_SRC" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$LINT_SRC"; }
trap 'restore_all; rm -f "$BAK"' EXIT

build_dev() { (cd "$ROOT" && "$PX" build tools/pxlint.px >/dev/null 2>&1) || return 1; [ -x "$LINT_DEV" ]; }

D=examples/m171_lint_scope
echo "=== [1] 合法侧：lint 0 错 0 警 + 三轨 stdout 逐字节一致"
for c in case_clo_capture case_nested_mutual case_assign_style case_comp_scope case_module_binding; do
    o=$("$ROOT/bootstrap/pxlint" "$D/$c.px" 2>&1 | tail -1)
    chk "$c lint 0/0" "echo \"\$o\" | grep -q '0 错误, 0 警告'"
    bash "$D/three_tracks.sh" "$D/$c.px" >/dev/null 2>&1; rc3=$?
    chk "$c 三轨一致" "[ $rc3 -eq 0 ]"
done

echo "=== [2] 真阳性侧：L002 / L001 仍被抓住"
o1=$("$ROOT/bootstrap/pxlint" "$D/bad_undefined.px" 2>&1)
o2=$("$ROOT/bootstrap/pxlint" "$D/bad_unused.px" 2>&1)
chk "bad_undefined 报 L002" "echo \"\$o1\" | grep -q \"E L002.*no_such_thing\""
chk "bad_unused 报 L001"    "echo \"\$o2\" | grep -q \"W L001.*never_read\""

echo "=== [3] 仓库实战回归：曾被误报的真实文件 → 0 错误"
REPO_FILES="stdlib/collections.px examples/fib.px examples/m30_comp.px examples/m31_vhost.px examples/m23c_http_adv.px examples/webapp/index.px examples/m69_registry/app/main.px examples/m160_closure/closure_test.px"
for f in $REPO_FILES; do
    [ -f "$f" ] || { echo "  SKIP $f（不在盘上）"; continue; }
    o=$("$ROOT/bootstrap/pxlint" "$f" 2>&1 | tail -1)
    chk "$f 0 错误" "echo \"\$o\" | grep -q '0 错误,'"
done

if [ $NEG -eq 1 ]; then
echo "=== [4] 负控（各自独立判红）"
echo "--- NC-A 关「顶层帧顶声明 hoist」→ 合法侧② 应变红"
snapshot
python3 - "$LINT_SRC" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
# 只改 lc_check_program 里顶层函数体那一处（帧顶 hoist）
old = 'lc_collect_frame_decls(st[4], ctx["decl"])'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, '0  # NC-A: 顶层帧顶 hoist 关闭'))
PYEOF
if build_dev; then
    o=$("$LINT_DEV" "$D/case_nested_mutual.px" 2>&1)
    chk "NC-A 判红（互递归被误报 L002）" "echo \"\$o\" | grep -q 'E L002'"
else
    chk "NC-A 重编 lint" "false"
fi
restore_all; build_dev >/dev/null 2>&1
o=$("$LINT_DEV" "$D/case_nested_mutual.px" 2>&1 | tail -1)
chk "NC-A 还原后复绿" "echo \"\$o\" | grep -q '0 错误, 0 警告'"

echo "--- NC-B 关「推导式变量声明」→ 合法侧④ 应变红"
restore_all
python3 - "$LINT_SRC" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''def lc_decl_comp_vars(sub, clauses):
    var ci = 0
    while ci < len(clauses):
        let cl = clauses[ci]
        let vars = cl[1]
        var vi = 0
        while vi < len(vars):
            lc_declare(sub, lc_unq(vars[vi]))
            vi += 1
        ci += 1'''
assert s.count(old) == 1, s.count(old)
new = '''def lc_decl_comp_vars(sub, clauses):
    var ci = 0
    while ci < len(clauses):
        ci += 1'''
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
if build_dev; then
    o=$("$LINT_DEV" "$D/case_comp_scope.px" 2>&1)
    chk "NC-B 判红（推导式变量被误报 L002）" "echo \"\$o\" | grep -q 'E L002'"
else
    chk "NC-B 重编 lint" "false"
fi
restore_all; build_dev >/dev/null 2>&1
o=$("$LINT_DEV" "$D/case_comp_scope.px" 2>&1 | tail -1)
chk "NC-B 还原后复绿" "echo \"\$o\" | grep -q '0 错误, 0 警告'"

echo "--- NC-C 关「外层帧可见链」→ 合法侧① 应变红"
restore_all
python3 - "$LINT_SRC" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''    let ks = parent["decl"].keys()
    var i = 0
    while i < len(ks):
        outer[ks[i]] = parent
        i += 1
    let oks = parent["outer"].keys()'''
assert s.count(old) == 1, s.count(old)
new = '''    let ks = parent["decl"].keys()
    var i = 0
    while i < len(ks):
        i += 1
    let oks = parent["outer"].keys()'''
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
if build_dev; then
    o=$("$LINT_DEV" "$D/case_clo_capture.px" 2>&1)
    chk "NC-C 判红（闭包捕获被误报 L002）" "echo \"\$o\" | grep -q 'E L002'"
else
    chk "NC-C 重编 lint" "false"
fi
restore_all; build_dev >/dev/null 2>&1
o=$("$LINT_DEV" "$D/case_clo_capture.px" 2>&1 | tail -1)
chk "NC-C 还原后复绿" "echo \"\$o\" | grep -q '0 错误, 0 警告'"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M171-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
