#!/usr/bin/env bash
# ============================================================
# M205 门（第 83 轮）：CLI 工具的**诊断通道**统一（缺陷 186 的 tools 面收尾）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §7（M205 建立）。
# 主问题（缺陷 186 家族）：M172 把**语言运行期**诊断统一到了 stderr，但**工具链**没跟上 ——
#   `tools/*.px` 的失败诊断仍写 stdout（`错误: 无法读取文件 …` / `pxpkg 错误: …`，
#   以及**参数错时打印的用法**），而 `tools/px`（shell）与 `tools/pxpkg`（bash 侧）**早已**是
#   `>&2` + 非零退出 ⇒ 同一套 CLI 里两种口径。后果：`cmd 2>/dev/null` 会把失败诊断一起吞掉、
#   `cmd >out 2>err` 的 err 是空的 ⇒ 脚本/CI **看不见失败原因**（M193/M195 两次红都因此更难读）。
# 判据（通道 = 消费方）：
#   · **诊断**（错误/警告/参数错提示/参数错时的用法）⇒ **stderr**；失败路径 **rc≠0**
#   · **产品**（成功路径的结果、结构化 JSON、`--help`/`--version` 文本）⇒ **stdout**
# 分层：
#   [0] 前置自证：8 个入库件存在且**含本轮改动**（锚点字符串）—— 防"拿旧件跑出假绿"（M204 教训）
#   [1] 静态 [S12]：scan_cli_channels.py 的 A/B/C/D/E 五条（含**反向判据** E：产品仍在 stdout）
#   [2] 动态：8 个 CLI × 三类 —— err 16 例（stdout 空 + stderr 非空 + rc≠0）
#                            · help 8 例（stdout 非空 + stderr 空 + rc=0）
#                            · ok   4 例（stderr 空 + rc=0 + 产品在 stdout）
#   [3] 解释轨面：pxpkg（bash+px）/ routegen / print_err 探针 —— 通道在**解释轨**同样成立
#   [4] 负控 3 道（各自独立判红、源逐字节还原）：
#       A 静态 · pxfmt 的 `print_err("错误: …")` 退回 `print(`            ⇒ [1] 判红
#       B 动态 · pxfmt 的 `usage(true)` 退回 `usage(false)` + **现场重编** ⇒ [2] 判红（参数错走 stdout）
#       C 动态 · tools/pxpkg 的 `*)` 退回 `help|--help|-h|*)`            ⇒ [3] 判红（未知命令 rc=0）
# 用法：bash examples/m205_cli_channels/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m205_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(tools/pxfmt.px tools/pxtest.px tools/pxbench.px tools/pxdoc.px tools/pxlint.px
       tools/pxcheck.px tools/pxlsp.px tools/pxmcp.px tools/pxpkg.px tools/routegen.px
       tools/px tools/pxpkg)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(basename "$f")"; done; }
restore_all() { for f in "${FILES[@]}"; do [ -f "$BACK/$(basename "$f")" ] && cp -f "$BACK/$(basename "$f")" "$f"; done; return 0; }
# 纪律（M190/M191/M196/M203 教训）：每道负控前**先 restore 再 snapshot**（各自独立、干净起点）
snapshot
trap 'restore_all' EXIT

echo "══════════ M205 · CLI 诊断通道统一（缺陷 186 tools 面）══════════"

# ── [0] 前置自证 ──
echo "── [0] 前置自证（入库件 + 本轮改动锚点）──"
for b in pxfmt pxlint pxtest pxbench pxdoc pxcheck pxlsp pxmcp; do
    chk "[0] bootstrap/$b 存在且可执行" "[ -x bootstrap/$b ]"
done
for pair in "pxfmt:pxfmt: 未知选项" "pxlint:pxlint: 未知选项" "pxtest:pxtest: 未知选项" \
            "pxdoc:pxdoc: 未知选项" "pxlsp:pxlsp: 未知选项" "pxmcp:pxmcp: 未知选项" \
            "pxcheck:pxcheck: 未知选项" "pxbench:pxbench: 未知选项"; do
    b="${pair%%:*}"; s="${pair#*:}"
    chk "[0] bootstrap/$b 含本轮锚点（$s）" "grep -qaF '$s' 'bootstrap/$b'"
done
chk "[0] tools/pxpkg 含本轮 bash 侧锚点" "grep -qF \"pxpkg: 未知命令\" tools/pxpkg"

# ── [1] 静态 ──
echo "── [1] 静态 [S12]（scan_cli_channels.py）──"
if python3 "$HERE/scan_cli_channels.py" "$ROOT" > "$W/scan.log" 2>&1; then
    chk "[1] [S12] A/B/C/D/E 全绿" "true"
else
    chk "[1] [S12] A/B/C/D/E 全绿" "false"
fi
sed -n '2,20p' "$W/scan.log" | sed 's/^/     /'

# ── [2] 动态（编译件 × 8 CLI × 三类）──
echo "── [2] 动态（编译件 · err / help / ok）──"
run_case() {   # $1=标签 $2=kind $3..=命令
    local tag="$1" kind="$2"; shift 2
    "$@" >"$W/$tag.out" 2>"$W/$tag.err"; local rc=$?
    local so se
    so=$(wc -c < "$W/$tag.out"); se=$(wc -c < "$W/$tag.err")
    case "$kind" in
        err)  chk "[2] $tag：stdout 空 + stderr 非空 + rc≠0" "[ $so -eq 0 ] && [ $se -gt 0 ] && [ $rc -ne 0 ]" ;;
        help) chk "[2] $tag：stdout 非空 + stderr 空 + rc=0" "[ $so -gt 0 ] && [ $se -eq 0 ] && [ $rc -eq 0 ]" ;;
        ok)   chk "[2] $tag：stderr 空 + rc=0 + 产品在 stdout" "[ $se -eq 0 ] && [ $rc -eq 0 ] && [ $so -gt 0 ]" ;;
    esac
    if [ "$kind" = err ] && { [ "$so" -ne 0 ] || [ "$se" -eq 0 ] || [ "$rc" -eq 0 ]; }; then
        echo "      out: $(head -c 120 "$W/$tag.out" | tr '\n' '|')"
        echo "      err: $(head -c 120 "$W/$tag.err" | tr '\n' '|')  rc=$rc"
    fi
}
B=bootstrap
run_case pxfmt-missing   err  $B/pxfmt /nonexistent_m205.px
run_case pxfmt-noargs    err  $B/pxfmt
run_case pxfmt-badopt    err  $B/pxfmt --bogus_m205 x.px
run_case pxlint-missing  err  $B/pxlint /nonexistent_m205.px
run_case pxlint-noargs   err  $B/pxlint
run_case pxlint-badopt   err  $B/pxlint --bogus_m205
run_case pxtest-missing  err  $B/pxtest /nonexistent_m205.px
run_case pxtest-noargs   err  $B/pxtest
run_case pxbench-missing err  $B/pxbench /nonexistent_m205.px f
run_case pxbench-nofn    err  $B/pxbench tools/pxfmt.px nosuchfn_m205
run_case pxdoc-missing   err  $B/pxdoc /nonexistent_m205.px
run_case pxdoc-badopt    err  $B/pxdoc --bogus_m205
run_case pxcheck-missing err  $B/pxcheck /nonexistent_m205.px
run_case pxcheck-badopt  err  $B/pxcheck --bogus_m205 x.px
run_case pxlsp-badopt    err  $B/pxlsp --bogus_m205
run_case pxmcp-badopt    err  $B/pxmcp --bogus_m205
run_case pxfmt-help      help $B/pxfmt --help
run_case pxlint-help     help $B/pxlint --help
run_case pxtest-help     help $B/pxtest --help
run_case pxbench-help    help $B/pxbench --help
run_case pxdoc-help      help $B/pxdoc --help
run_case pxcheck-help    help $B/pxcheck --help
run_case pxlsp-help      help $B/pxlsp --help
run_case pxmcp-help      help $B/pxmcp --help
run_case pxfmt-ok        ok   $B/pxfmt --check tools/routegen.px
run_case pxlint-ok       ok   $B/pxlint tools/routegen.px
run_case pxdoc-ok        ok   $B/pxdoc tools/routegen.px
run_case pxcheck-ok      ok   $B/pxcheck tools/routegen.px

# ── [3] 解释轨面 ──
echo "── [3] 解释轨面（pxpkg / routegen / print_err 探针）──"
tools/pxpkg --bogus_m205 >"$W/pxpkg.out" 2>"$W/pxpkg.err"; rc=$?
chk "[3] pxpkg 未知命令：stdout 空 + stderr 非空 + rc≠0" \
    "[ $(wc -c < "$W/pxpkg.out") -eq 0 ] && [ $(wc -c < "$W/pxpkg.err") -gt 0 ] && [ $rc -ne 0 ]"
tools/pxpkg --help >"$W/pxpkgh.out" 2>"$W/pxpkgh.err"; rc=$?
chk "[3] pxpkg --help：stdout 非空 + stderr 空 + rc=0" \
    "[ $(wc -c < "$W/pxpkgh.out") -gt 0 ] && [ $(wc -c < "$W/pxpkgh.err") -eq 0 ] && [ $rc -eq 0 ]"
env -u PX_APP_DIR ./bootstrap/pxi run tools/routegen.px >"$W/rg.out" 2>"$W/rg.err"; rc=$?
chk "[3] routegen 缺 PX_APP_DIR：stdout 空 + stderr 非空 + rc≠0" \
    "[ $(wc -c < "$W/rg.out") -eq 0 ] && [ $(wc -c < "$W/rg.err") -gt 0 ] && [ $rc -ne 0 ]"
cat > "$W/perr.px" <<'PEOF'
def main():
    print("产品行")
    print_err("诊断行")
PEOF
./bootstrap/pxi run "$W/perr.px" >"$W/perr.out" 2>"$W/perr.err"; rc=$?
chk "[3] print_err 解释轨：产品→stdout、诊断→stderr" \
    "[ $rc -eq 0 ] && grep -q '产品行' '$W/perr.out' && ! grep -q '诊断行' '$W/perr.out' && grep -q '诊断行' '$W/perr.err'"

# ── [4] 负控 ──
if [ "$NEG" = 1 ]; then
    echo "── [4] 负控（3 道 · 各自独立判红 · 源逐字节还原）──"

    # A 静态：pxfmt 的一处诊断退回 print( ⇒ [1] 必须判红
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
p = sys.argv[1] + "/tools/pxfmt.px"
s = open(p, encoding="utf-8").read()
old = '        print_err("错误: 无法读取文件 " + file)'
new = '        print("错误: 无法读取文件 " + file)'
assert s.count(old) == 1, "负控 A 锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PYEOF
    if python3 "$HERE/scan_cli_channels.py" "$ROOT" > "$W/ncA.log" 2>&1; then ncA=0; else ncA=1; fi
    chk "[4A] pxfmt 诊断退回 stdout ⇒ [S12] 判红（且指到该站点）" \
        "[ $ncA -ne 0 ] && grep -q 'tools/pxfmt.px' '$W/ncA.log'"
    sed -n '/❌/p' "$W/ncA.log" | head -2 | sed 's/^/      /'
    restore_all

    # B 动态：pxfmt 的参数错用法退回 stdout（usage(false)）+ **现场重编** ⇒ [2] 必须判红
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
p = sys.argv[1] + "/tools/pxfmt.px"
s = open(p, encoding="utf-8").read()
old = "        usage(true)\n        exit(2)"
new = "        usage(false)\n        exit(2)"
assert s.count(old) == 1, "负控 B 锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PYEOF
    rm -rf tools/build/pxfmt
    if ./tools/pxc build --no-quic tools/pxfmt.px > "$W/ncB.build" 2>&1 && [ -x tools/build/pxfmt ]; then
        tools/build/pxfmt >"$W/ncB.out" 2>"$W/ncB.err"; rc=$?
        chk "[4B] 参数错用法退回 stdout ⇒ 通道判据失效（stdout 非空）" \
            "[ $rc -ne 0 ] && [ $(wc -c < "$W/ncB.out") -gt 0 ]"
    else
        echo "      ⚠️ 负控 B 现场重编失败（判红 · 不得静默跳过）："; tail -3 "$W/ncB.build" | sed 's/^/        /'
        chk "[4B] 参数错用法退回 stdout ⇒ 通道判据失效（stdout 非空）" "false"
    fi
    restore_all

    # C 动态：tools/pxpkg 未知命令退回 stdout+rc=0 ⇒ [3] 必须判红
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
p = sys.argv[1] + "/tools/pxpkg"
s = open(p, encoding="utf-8").read()
old = "    help|--help|-h)\n        cat <<'EOF'\n"
new = "    help|--help|-h|*)\n        cat <<'EOF'\n"
assert s.count(old) == 1, "负控 C 锚点不唯一"
s = s.replace(old, new)
i = s.index("    *)\n        # M205")
j = s.index("        ;;\n", i) + len("        ;;\n")
s = s[:i] + s[j:]
open(p, "w", encoding="utf-8").write(s)
PYEOF
    tools/pxpkg --bogus_m205 >"$W/ncC.out" 2>"$W/ncC.err"; rc=$?
    chk "[4C] pxpkg 未知命令退回 stdout+rc=0 ⇒ [3] 判据失效" \
        "[ $rc -eq 0 ] && [ $(wc -c < "$W/ncC.out") -gt 0 ] && [ $(wc -c < "$W/ncC.err") -eq 0 ]"
    restore_all

    # 收尾：源必须逐字节还原（与门开头快照 cmp —— 不能拿 git status 判：本轮改动本就未提交）
    restore_ok=1
    for f in "${FILES[@]}"; do cmp -s "$f" "$BACK/$(basename "$f")" || restore_ok=0; done
    chk "[4D] 负控后源码逐字节还原（与门开头快照 cmp 一致）" "[ $restore_ok = 1 ]"
fi

rm -rf tools/build/pxfmt tools/build/pxfmt.o tools/build/pxfmt.c 2>/dev/null

echo ""
echo "── 汇总：通过 $pass · 失败 $fail ──"
if [ "$fail" = 0 ]; then
    echo "M205-VERIFY-OK"
    exit 0
else
    echo "M205-VERIFY-FAIL"
    exit 1
fi
