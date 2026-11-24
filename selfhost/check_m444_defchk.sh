#!/usr/bin/env bash
# ============================================================
# selfhost/check_m444_defchk.sh —— 构建前「入口级定义检查」（M444 · 缺陷 932）
# ------------------------------------------------------------
# 病史（2026-11-24 实测）：ws-todo 的兜底函数 infer_ws_root_from_argv 里写了
#   to_str(...)，而 to_str 从未定义 —— 编译成功、**运行到兜底路径才炸** R1001。
#   偏偏那条路径就是「WS_PATH 缺失」的救命索 ⇒ 兜底比没有兜底更糟。
#   病根：px 三条轨都只在运行到那一刻查名字，构建期一个字都不查。
#
# 修法：`px build` 在**入口文件**这一层做 L002（未定义标识符）前置检查，默认开。
#   为什么不塞进 sema：实测 selfhost/ 24 件模块文件 lint 出 1000+ 处 L002 假红
#   （bc_emit 112 / cg_expr 176 / ibuiltin 295 …，模块按惯例引用「主文件声明的全局」），
#   塞进编译期会把现有构建全部判红。而入口层是干净的：examples/*.px 120 个入口 L002 = 0。
#
# 判据（每条都能判红）：
#   S1 形状：tools/px 有 defchk_entry 定义；调用点在 `--pxe` 分叉**之前**（否则换个 flag 就绕过）
#   P1 正控：直读未定义名 ⇒ build rc≠0，且 stderr 点名该名字与 L002
#   P2 正控：未定义名藏在 `if x == 1:` 死分支 ⇒ rc≠0（正是「测不到的死路径」那一类）
#   P3 正控：静态轨（默认，非 --pxe）同样被拦 ⇒ rc≠0（两条轨都不能漏）
#   N1 负控（有牙的前提）：同一份 P1 源码把定义补上 ⇒ rc=0 且产物能跑、输出正确
#   N2 负控自证：PX_SKIP_DEFCHK=1 时同一份 P1 源码 ⇒ rc=0（证明 P1 的红来自这条检查）
#   N3 降级档：PX_DEFCHK=warn ⇒ rc=0 且 stderr 含「不阻断」
#   N4 无假红：真实程序 examples/fib.px ⇒ rc=0
#   N5 import 解析不到不误杀：pxlint 报 L010 警告、0 错误；px build 仍能过（工作区路径不同）
#   N6 `__` 内部名不计红（runtime 注入、不进名册）+ 前提自证「走了豁免且出声」
#   N7 examples/ 夹具树豁免 + 负控「同一份源码移出夹具树仍被拦」
#   G1 环境守卫：tools/px / bootstrap/pxlint 不可用 ⇒ rc=3（绝不降级成 0）
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"
PX="./tools/px"; LINT="./bootstrap/pxlint"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

echo "=== m444_defchk · 构建前入口级定义检查（缺陷 932）==="
[ -x "$PX" ] || { echo "❌ 环境不成立：$PX 不可执行" >&2; exit 3; }
[ -x "$LINT" ] || { echo "❌ 环境不成立：$LINT 不可执行" >&2; exit 3; }
command -v gcc >/dev/null 2>&1 || { echo "❌ 环境不成立：缺 gcc" >&2; exit 3; }

# ── 负控模式（--negctl）：把调用点打掉，P1/P2/P3 必须**全变绿**（= 本门会判红）──
#   为什么要有：本门的价值全压在「调用点真的接上了」。删掉那一行而门仍全绿 ⇒ 门是空的。
#   打桩后一定还原（trap），不允许留下脏树。
if [ "${1:-}" = "--negctl" ]; then
    echo "=== --negctl：删掉 cmd_build 里的 defchk_entry 调用 ==="
    NEGOK=0
    BAK="$(mktemp)"; cp -f "$PX" "$BAK"
    trap 'cp -f "$BAK" "$PX"; rm -f "$BAK"' EXIT
    sed -i 's@^    defchk_entry "\$file" || return 1$@    : # negctl removed@' "$PX"
    grep -q 'negctl removed' "$PX" && echo "  打桩已落地（前提自证）" || { echo "❌ 打桩没生效"; exit 1; }
    cat > /tmp/m444neg.px <<'NEGEOF'
def f():
    return to_str(1)

print(f())
NEGEOF
    r1=0; "$PX" build --pxe /tmp/m444neg.px >/dev/null 2>&1 || r1=$?
    [ "$r1" = 0 ] && NEGOK=1
    echo "  打桩后 build rc=$r1（0 = 不再拦 ⇒ 判据有牙）"
    rm -f /tmp/m444neg.px
    [ "$NEGOK" = 1 ] && { echo "M444-DEFCHK NEG-CONTROL OK（红来自调用点，不是别处）"; exit 0; }
    echo "❌ 打桩后仍被拦 ⇒ P1/P2/P3 的红另有来源，判据无效"; exit 1
fi

T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT

# ── S1 形状：定义在、调用点在 --pxe 分叉之前 ──
n_def="$(grep -c '^defchk_entry() {' "$PX" || true)"
n_call="$(grep -c 'defchk_entry "\$file"' "$PX" || true)"
ln_call="$(grep -n 'defchk_entry "\$file"' "$PX" | head -1 | cut -d: -f1)"
ln_pxe="$(grep -n 'if \[ "\$pxe" = 1 \]' "$PX" | head -1 | cut -d: -f1)"
[ "${n_def:-0}" -ge 1 ] && ok "S1a 形状：tools/px 有 defchk_entry 定义" || bad "S1a 缺 defchk_entry 定义"
[ "${n_call:-0}" -ge 1 ] && ok "S1b 形状：cmd_build 有调用点" || bad "S1b 没有调用点（等于没接）"
if [ -n "${ln_call:-}" ] && [ -n "${ln_pxe:-}" ] && [ "$ln_call" -lt "$ln_pxe" ]; then
    ok "S1c 形状：调用行($ln_call) 在 --pxe 分叉行($ln_pxe) 之前 ⇒ 两条轨都过这一关"
else
    bad "S1c 调用点在 --pxe 分叉之后（或取不到）：--pxe 轻轻一绕就过（$ln_call vs $ln_pxe）"
fi

mk() {  # mk <file> <body...>
    printf '%s\n' "$2" > "$T/$1"; }

# ── P1/P2/P3 正控 ──
cat > "$T/p1.px" <<'EOF'
def f():
    return to_str(1)

print(f())
EOF
out="$("$PX" build --pxe "$T/p1.px" 2>&1)"; rc=$?
[ "$rc" != 0 ] && ok "P1 正控：直读未定义名 ⇒ build rc=$rc（拒构建）" || bad "P1 正控判绿：未定义名居然构建成功"
printf '%s' "$out" | grep -q 'L002' && ok "P1 报出 L002 诊断码" || bad "P1 没报 L002"
printf '%s' "$out" | grep -q "to_str" && ok "P1 点名了未定义的那个名字" || bad "P1 没点名"

cat > "$T/p2.px" <<'EOF'
def g(x):
    if x == 1:
        return to_str(2)
    return 0

print(g(0))
EOF
"$PX" build --pxe "$T/p2.px" >"$T/p2.out" 2>&1
[ $? != 0 ] && ok "P2 正控：未定义名藏在死分支（x==1 从不成立）⇒ 依然拒构建" || bad "P2 正控判绿：死分支里的未定义名漏掉了"
grep -q 'L002' "$T/p2.out" && ok "P2 前提自证：红的是 L002（不是别的错）" || bad "P2 红因不明"

"$PX" build "$T/p1.px" >"$T/p3.out" 2>&1
[ $? != 0 ] && ok "P3 正控：静态轨（默认，非 --pxe）同样被拦" || bad "P3 判绿：只拦轻轨、静态轨漏"

# ── N1 负控：补上定义必须绿，且产物真能跑 ──
cat > "$T/n1.px" <<'EOF'
def to_str(x):
    return str(x)

def f():
    return to_str(1)

print("N1-OK " + f())
EOF
if "$PX" build --pxe "$T/n1.px" >"$T/n1.build" 2>&1; then
    ok "N1 负控：补上定义 ⇒ 构建成功（检查不是恒红）"
    if "$PX" run "$T/n1.px" 2>/dev/null | grep -q '^N1-OK 1$'; then
        ok "N1 前提自证：产物/解释执行真跑到那条路径（不是空跑）"
    else
        bad "N1 产物没跑出期望输出"
    fi
else
    bad "N1 负控判红：补上定义仍被拦（检查过宽，会误杀正常程序）"
fi

# ── N2 负控自证：关掉检查 ⇒ 同一份 P1 源码必须过 ──
PX_SKIP_DEFCHK=1 "$PX" build --pxe "$T/p1.px" >"$T/n2.out" 2>&1
[ $? = 0 ] && ok "N2 负控自证：PX_SKIP_DEFCHK=1 时同一份源码构建成功 ⇒ P1 的红来自这条检查" \
           || bad "N2 前提不成立：关掉检查后仍失败（P1 的红另有来源）"

# ── N3 降级档 ──
PX_DEFCHK=warn "$PX" build --pxe "$T/p1.px" >"$T/n3.out" 2>&1
rc=$?
[ "$rc" = 0 ] && ok "N3 PX_DEFCHK=warn：不阻断（rc=0）" || bad "N3 降级档仍阻断（rc=$rc）"
grep -q '不阻断' "$T/n3.out" && ok "N3 前提自证：确实走了 warn 分支（输出含『不阻断』）" || bad "N3 没走 warn 分支"

# ── N4 无假红：真实程序 ──
"$PX" build examples/fib.px >"$T/n4.out" 2>&1
[ $? = 0 ] && ok "N4 无假红：examples/fib.px 正常构建" || bad "N4 真实程序被误拦：$(tail -2 "$T/n4.out")"

# ── N5 import 解析不到：不误杀，但必须出声 ──
out="$("$LINT" examples/m282_registry_020/ftp020_probe.px 2>&1)"
printf '%s' "$out" | grep -q 'L010' && ok "N5 前提自证：pxlint 对解析不到的 import 报 L010（不再静默级联成 L002）" \
                                  || bad "N5 pxlint 没报 L010（静默级联仍在）"
printf '%s' "$out" | grep -q 'E L002' && bad "N5 仍在级联假 L002（会把工作区式 import 的程序全判死）" \
                                     || ok "N5 负控：L002 已被剔除（0 条假红）"
"$PX" build --pxe examples/m282_registry_020/ftp020_probe.px >"$T/n5.out" 2>&1
[ $? = 0 ] && ok "N5b 不误杀：该入口仍能构建（构建期路径可能不同）" || bad "N5b 误杀了工作区式 import 的入口"

# ── N6 `__` 内部名不计红 ──
#   名册口径（gen_builtin_list.sh ①：`__` 前缀的内部名不进名册；runtime.c 明文同口径）
#   ⇒ pxlint 必然看不到 __guard_call / __iter_len。实测首轮全量门把 selfhost/interp.px
#   的 2 处 __guard_call 当未定义 ⇒ dev 解释器构建全灭（m158/m161/m169/m172/m179 连坐）。
cat > "$T/n6.px" <<'EOF'
def dead():
    return __iter_len([1, 2])

print("N6-OK")
EOF
out="$("$PX" build --pxe "$T/n6.px" 2>&1)"; rc=$?
[ "$rc" = 0 ] && ok "N6 无假红：\`__\` 内部名（runtime 注入、不进名册）不判红 ⇒ build rc=0" \
              || bad "N6 把 runtime 内部名当未定义拦了：$(printf '%s' "$out" | head -2)"
printf '%s' "$out" | grep -q '内部名' && ok "N6 前提自证：确实走了「\`__\` 豁免」分支（输出含『内部名』）" \
                                      || bad "N6 没走豁免分支（豁免没生效，或没出声 —— 静默豁免不可接受）"

# ── N7 examples/ 夹具树豁免（有牙：同一份源码移到 examples/ 之外必须被拦）──
EXF="examples/.defchk_probe_$$.px"
printf 'def f():\n    return to_str(1)\n\nprint(f())\n' > "$EXF"
trap 'rm -rf "$T"; rm -f "$EXF"' EXIT
if "$PX" build --pxe "$EXF" >"$T/n7.out" 2>&1; then
    ok "N7 夹具树豁免：examples/ 下的入口不拦（20+ 门故意编译含未定义名的程序）"
else
    bad "N7 examples/ 下仍被拦（诊断类门会全红）：$(tail -2 "$T/n7.out")"
fi
cp -f "$EXF" "$T/n7_copy.px"
if "$PX" build --pxe "$T/n7_copy.px" >"$T/n7b.out" 2>&1; then
    bad "N7 负控判绿：同一份源码在 examples/ 之外也没被拦 ⇒ 豁免扩散，检查等于空"
else
    ok "N7 负控：同一份源码放到 examples/ 之外仍被拦（豁免只对夹具树生效）"
fi
rm -f "$EXF"

# ── 规模锚点 ──
nrun=$((PASS+FAIL))
[ "$nrun" -ge 14 ] && ok "规模锚点：判据 $nrun 条（≥14）" || bad "判据只有 $nrun 条 —— 判据被静默削减？"

echo "── 合计：通过 $PASS · 失败 $FAIL ──"
if [ "$FAIL" = 0 ]; then
    echo "M444-DEFCHK OK n=$PASS"
    exit 0
fi
echo "M444-DEFCHK FAIL pass=$PASS fail=$FAIL"
exit 1
