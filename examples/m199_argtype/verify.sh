#!/usr/bin/env bash
# ============================================================
# M199 门（第 77 轮）：[S10]「同一操作 × **每一个位置** × **错类型**」三轨单一真相
#                        缺陷 237（三元兜底静默）+ 缺陷 238（解释轨经方法面路由）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.10（本轮建立）
#
# 为什么需要（这一面此前没人量）：
#   M190 量「实参**个数**」· M194/M195 量「**单个**实参的类型守卫」· M197 量「0 参 ⇄ arity 文案」。
#   都没量 **每个位置 × 错类型**。缺口的两种形状都在本轮实测到：
#     ① **三元兜底**：`(nargs >= 2 && args[1].type == PX_STR) ? … : " "` 把「**不是**字符串」与
#        「**缺省**」混为一谈 ⇒ 解释轨响亮、编译轨静默（`split("s", {})` → `[s]`）；
#     ② **入口不同**：解释轨经**方法面**执行 `args[0].replace(a,b)` ⇒ 位置错时给
#        `R1007 类型 dict 没有方法 'replace'`，而编译轨是**函数面** `R1002: replace 需要 3 个字符串参数`。
#
# 判据：
#   [1] 静态 [S10-a]：解析**全部已注册 native** 的逐参守卫声明 + 「三元兜底」站点清单 ⇄ 登记表对拍
#       （未登记 0 · 表漂移 0）。扫描器须收 `static` 与非 `static` 两种定义 —— 首版只收 static
#       ⇒ `ws_heartbeat` 整函数**静默漏掉**（= 假绿），本轮实测踩到。
#   [2] 动态 [S10-b]：**从声明自动生成**探针（全参类型正确、只第 P 位为毒值 `{}`），合成**一个**
#       驱动器 ⇒ **两次编译**覆盖全量；三轨（解释/VM/C）对拍 **rc + R 码 + 消息体 + stdout**，
#       **分叉必须为 0**。（一例一次 build 需 7+ 小时 —— 本驱动器把这一面变成**可全量度量**。）
#   [3] 显式期望：本轮两族缺陷的 6 条用例必须给**规范词条**（不只是"某个 R1002"）。
#   [4] 合法侧回归：LEGAL.tsv 的正常调用三轨 rc=0 且 stdout 逐字节一致。
#   [5] 负控 4 道（各自独立判红 · 源逐字节还原）：
#       A 动态（编译轨）：`split` 守卫退回三元兜底 ⇒ [2] 判红
#       B 动态（解释轨）：删 `ibuiltin.px` 的 replace 类型检查 + **只重烘 pxi** ⇒ [2] 判红
#         （专门证明「解释轨源码」也在判据内 —— 只改 .px 不重烘是**没牙**的负控，M190 踩过）
#       C 静态：登记表删一行 ⇒ 未登记判红（证明扫描器有牙）
#       D 静态：登记表加一行幽灵站点 ⇒ 表漂移判红
# 用法：bash examples/m199_argtype/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m199_gate; BACK=$W/bak; SW=$W/sweep
rm -rf "$W"; mkdir -p "$W" "$BACK" "$SW" "$SW/build"
FILES=(runtime/runtime.c runtime/runtime_ws.c runtime/runtime_quic.c selfhost/ibuiltin.px
       examples/m199_argtype/FALLBACK_ALLOW.tsv)
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
# 入库解释器**硬备份**：负控 B 会临时改写它（篡改版重烘）—— 门结束必须逐字节还原
cp -f bootstrap/pxi "$BACK/bootstrap_pxi.bin"
trap 'restore_all; cp -f "$BACK/bootstrap_pxi.bin" bootstrap/pxi 2>/dev/null' EXIT

# ── 前置不变量：本轮 7 处修复必须在位（缺一 ⇒ 门自身失败，不假装判红） ──
pre() { grep -qF "$1" "$2" || { echo "❌ 前置自查失败：$2 缺「$1」" >&2; exit 2; }; }
pre 'if (nargs >= 2 && args[1].type != PX_STR) px_error("R1002: split 参数需要 string");' runtime/runtime.c
pre 'px_error("R1002: exit 的 code 需要整数，实际是 %s", px_type_name(args[0]));' runtime/runtime.c
pre 'px_error("R1002: udp_open 的 port 需要整数，实际是 %s", px_type_name(args[0]));' runtime/runtime.c
pre 'px_error("R1002: tls_server 的 hostname 需要字符串，实际是 %s", px_type_name(args[2]));' runtime/runtime.c
pre 'px_error("R1002: ws_heartbeat 的 interval_ms 需要整数，实际是 %s", px_type_name(args[1]));' runtime/runtime_ws.c
pre 'px_error("R1002: quic_connect 的 session 需要字符串，实际是 %s", px_type_name(args[3]));' runtime/runtime_quic.c
pre 'type(args[0]) != "string" or type(args[1]) != "string" or type(args[2]) != "string"' selfhost/ibuiltin.px

sweep_static() { python3 "$HERE/sweep_argtype.py" --root "$ROOT" --out "$SW" > "$W/sweep.log" 2>&1; }
gen_probes()   { python3 "$HERE/gen_probes.py" --root "$ROOT" --out "$SW" > "$W/gen.log" 2>&1; }
build_drv()    {  # $1=both|c
    rm -rf "$SW/a_vm" "$SW/a_c"; mkdir -p "$SW/a_vm" "$SW/a_c"
    cp -f "$SW/drv.px" "$SW/a_vm/drv.px"; cp -f "$SW/drv.px" "$SW/a_c/drv.px"
    if [ "$1" = both ]; then
        ( cd "$SW/a_vm" && timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_vm.log" 2>&1
        rm -f "$SW/build/drv_vm"; mv -f "$SW/a_vm/build/drv" "$SW/build/drv_vm" 2>/dev/null
    fi
    ( cd "$SW/a_c" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_c.log" 2>&1
    rm -f "$SW/build/drv_c"; mv -f "$SW/a_c/build/drv" "$SW/build/drv_c" 2>/dev/null
    [ -x "$SW/build/drv_vm" ] && [ -x "$SW/build/drv_c" ]; }
tracks() { python3 "$HERE/three_tracks.py" --root "$ROOT" --sweep "$SW" --build "$SW/build" > "$W/tracks.log" 2>&1; }

echo "=== [1] 静态 [S10-a]：逐参守卫声明全量 + 三元兜底清单 ⇄ 登记表 ==="
sweep_static; rc=$?
chk "[1] 扫描器通过：未登记 0 · 表漂移 0 · 结论一致" \
    "[ \$rc = 0 ] && grep -q '未登记（必须为 0）              : 0' '$W/sweep.log' && grep -q '表漂移（必须为 0）              : 0' '$W/sweep.log' && grep -q '结论：不一致 0' '$W/sweep.log'"
chk "[1] 声明覆盖下限：有守卫函数 ≥ 240 · 守卫位置 ≥ 430（下限而非等式 —— 新增 native 自然增长）" \
    "awk -F': ' '/有逐参守卫声明的函数/{exit !(\$2>=240)}' '$W/sweep.log' && awk -F': ' '/守卫声明位置总数/{exit !(\$2>=430)}' '$W/sweep.log'"

echo "=== [2] 动态 [S10-b]：自动探针（全参类型正确 · 只第 P 位毒值）× 三轨 ==="
gen_probes; rc=$?
chk "[2] 探针生成成功（≥ 150 条）" \
    "[ \$rc = 0 ] && grep -oE '生成探针 [0-9]+' '$W/gen.log' | awk '{exit !(\$2>=150)}'"
chk "[2] 探针覆盖函数下限（≥ 100）" \
    "grep -oE '函数 [0-9]+ 个' '$W/gen.log' | awk '{exit !(\$2>=100)}'"
if build_drv both; then chk "[2] 驱动器两轨编译成功（VM + C）" "true"
else
    chk "[2] 驱动器两轨编译成功（VM + C）" "false"
    for L in b_vm b_c; do echo "     ── $L 尾 ──"; tail -4 "$W/$L.log" 2>/dev/null | sed 's/^/     /'; done
fi
tracks; rc=$?
chk "[2] 三轨对拍：**分叉 0**（rc + R 码 + 消息体 + stdout 全等）" \
    "[ \$rc = 0 ] && grep -qE '分叉 0\$' '$W/tracks.log'"

echo "=== [3] 显式期望：两族缺陷的用例必须给规范词条（三轨同码同文）==="
expect3() {  # $1=label $2=code $3=text
    python3 - "$SW/three_tracks.json" "$1" "$2" "$3" <<'PY'
import json, sys
rows = {r['label']: r for r in json.load(open(sys.argv[1]))}
lbl, code, text = sys.argv[2], sys.argv[3], sys.argv[4]
r = rows.get(lbl)
if not r: sys.exit('缺用例 ' + lbl)
for t in ('interp', 'vm', 'c'):
    if r[t]['code'] != code or r[t]['body'] != text:
        sys.exit(f"{lbl}/{t}: 期望 {code} | {text}，实际 {r[t]['code']} | {r[t]['body']}")
PY
}
chk "[3] split 第 2 参非字符串 ⇒ R1002（三轨同文）"  "expect3 split__p1 R1002 'split 参数需要 string'"
chk "[3] replace 第 0 参非字符串 ⇒ 函数面词条"        "expect3 replace__p0 R1002 'replace 需要 3 个字符串参数'"
chk "[3] replace 第 1 参非字符串 ⇒ 函数面词条"        "expect3 replace__p1 R1002 'replace 需要 3 个字符串参数'"
chk "[3] exit 的 code 非整数 ⇒ R1002"                "expect3 exit__p0 R1002 'exit 的 code 需要整数，实际是 dict'"
chk "[3] udp_open 的 port 非整数 ⇒ R1002"            "expect3 udp_open__p0 R1002 'udp_open 的 port 需要整数，实际是 dict'"
chk "[3] tls_server 的 hostname 非字符串 ⇒ R1002"    "expect3 tls_server__p2 R1002 'tls_server 的 hostname 需要字符串，实际是 dict'"

echo "=== [4] 合法侧回归：LEGAL.tsv 三轨 rc=0 且 stdout 逐字节一致 ==="
legals=$(awk -F'@@' '{print $1}' "$HERE/LEGAL.tsv" | grep -c '^ok' || true)
chk "[4] 合法用例条数 ≥ 8（下限）" "[ $legals -ge 8 ]"
chk "[4] 全部合法用例三轨 rc=0 且 stdout 一致" \
    "python3 - '$SW/three_tracks.json' <<'PY'
import json, sys
rows = {r['label']: r for r in json.load(open(sys.argv[1]))}
ok = [r for k, r in rows.items() if k.startswith('ok')]
if len(ok) < 8: sys.exit('合法用例不足: %d' % len(ok))
for r in ok:
    if any(r[t]['rc'] != 0 for t in ('interp', 'vm', 'c')): sys.exit(r['label'] + ' rc!=0')
    if len({r[t]['out'] for t in ('interp', 'vm', 'c')}) != 1: sys.exit(r['label'] + ' stdout 分叉')
print('合法侧', len(ok), '例三轨一致')
PY"

if [ "$NEG" = 1 ]; then
    echo "=== [5] 负控（各自独立判红 · 源逐字节还原）==="

    # ── B 动态（解释轨）：删 ibuiltin 的 replace 类型检查 + 只重烘 pxi ──
    restore_all; snapshot
    python3 - <<'PY'
s = open('selfhost/ibuiltin.px', encoding='utf-8').read()
old = '''        if type(args[0]) != "string" or type(args[1]) != "string" or type(args[2]) != "string":
            return Err(i_r1002("replace 需要 3 个字符串参数", pos))
'''
assert s.count(old) == 1
open('selfhost/ibuiltin.px', 'w', encoding='utf-8').write(s.replace(old, ''))
PY
    ./selfhost/rebake_bin.sh --entries=pxi > "$W/neg_b.log" 2>&1
    tracks; rc=$?
    chk "[5B] 解释轨 replace 类型检查删除（重烘 pxi）⇒ [2] 判红" \
        "[ \$rc != 0 ] && grep -q 'replace__p' '$W/tracks.log'"
    restore_all                                   # 先还原源码
    ./selfhost/rebake_bin.sh --entries=pxi >> "$W/neg_b.log" 2>&1   # 再用**正品源码**重烘回正品
    chk "[5B] 还原后重烘 pxi 与入库件逐字节一致（重烘确定性自证）" \
        "cmp -s bootstrap/pxi '$BACK/bootstrap_pxi.bin'"

    # ── A 动态（编译轨）：split 退回三元兜底 ⇒ [2] 判红 ──
    restore_all; snapshot
    python3 - <<'PY'
s = open('runtime/runtime.c', encoding='utf-8').read()
old = '    if (nargs >= 2 && args[1].type != PX_STR) px_error("R1002: split 参数需要 string");\n    const char* s = args[0].as.obj->as.str.data;\n    const char* sep = (nargs >= 2) ? args[1].as.obj->as.str.data : " ";'
new = '    const char* s = args[0].as.obj->as.str.data;\n    const char* sep = (nargs >= 2 && args[1].type == PX_STR) ? args[1].as.obj->as.str.data : " ";'
assert s.count(old) == 1
open('runtime/runtime.c', 'w', encoding='utf-8').write(s.replace(old, new))
PY
    build_drv c >/dev/null 2>&1; tracks; rc=$?
    chk "[5A] split 退回三元兜底 ⇒ [2] 判红（解释轨响亮 · 编译轨静默 = 分叉）" \
        "[ \$rc != 0 ] && grep -q 'split__p1' '$W/tracks.log'"
    restore_all

    # ── C 静态：登记表删一行 ⇒ 未登记判红 ──
    restore_all; snapshot
    cp -f "$HERE/FALLBACK_ALLOW.tsv" "$W/tbl_orig.tsv"
    grep -v 'args\[1\].type == PX_BOOL) op' "$HERE/FALLBACK_ALLOW.tsv" > "$W/tbl_c.tsv"
    cp -f "$W/tbl_c.tsv" "$HERE/FALLBACK_ALLOW.tsv"
    sweep_static; rc=$?
    chk "[5C] 登记表删一行 ⇒ 未登记判红" \
        "[ \$rc != 0 ] && grep -q '未登记（必须为 0）              : 1' '$W/sweep.log'"

    # ── D 静态：登记表加一行幽灵站点 ⇒ 表漂移判红 ──
    cp -f "$W/tbl_orig.tsv" "$HERE/FALLBACK_ALLOW.tsv"
    printf 'runtime.c\targs[9].type == PX_INT ? 1 : 0\tguard\t负控：故意不存在的站点\n' >> "$HERE/FALLBACK_ALLOW.tsv"
    sweep_static; rc=$?
    chk "[5D] 登记表加一行幽灵站点 ⇒ 表漂移判红" \
        "[ \$rc != 0 ] && grep -q '表漂移（必须为 0）              : 1' '$W/sweep.log'"
    cp -f "$W/tbl_orig.tsv" "$HERE/FALLBACK_ALLOW.tsv"
    restore_all
fi

restore_all
echo ""
echo "═══ M199 门：通过 $pass · 失败 $fail ═══"
if [ "$fail" = 0 ]; then echo "M199-VERIFY-OK"; exit 0; fi
exit 1
