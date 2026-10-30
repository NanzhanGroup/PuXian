#!/usr/bin/env bash
# ============================================================
# examples/m362_divmod/verify.sh —— `//` / `%` 逐值对拍门（M362 · 缺陷 628）
# ------------------------------------------------------------
# 为什么有这道门（根因不是"有 bug"，是"没看守"）：
#   docs/PX_DEF_TRIAGE.md §14.4（缺陷 301）已证实：整数 `//` 与 `%` 是**欧几里得对**
#   （Rust div_euclid / rem_euclid），而 docs/spec.md 曾写「同 Python」。语义自洽、
#   三轨一致，但**没有任何门看守它** —— 只靠「实现碰巧一致 + 文档」维持。
#   风险面实在：从 Python 移植带负数的 `//`/`%`（取模索引 / 环形缓冲 / 日期偏移 /
#   `hash % n` 的负数分支）会拿到**不同的商与余数且不报错**。
#
# 判据（每条都是"被测量 vs 外部参考"，禁止实现自证）：
#   A 三轨逐字节一致   px run（解释）· px build（默认 VM 轨）· px build --c（C 轨）
#   B 与独立参考值对拍 参考值由 bash 依**欧几里得定义**现算（r∈[0,|b|)，q=(a-r)/b），
#                      不来自被测实现，也不抄文档表格
#   C 门侧重算不变量   取输出里的 (a,b,q,r) 由**门自己**判 a==q*b+r 与 r∈[0,|b|)
#                      —— 被测程序自报的任何结论列**不参与判定**（否则就是自证）
#   D 覆盖度自证       四象限各 ≥3 组 + 含 0 + 含 |a|>99（防用例退化后门变空转）
# 负控（证明判据有鉴别力，不是恒绿）：
#   NC1 参考值扰动 1 行        ⇒ B 必须红
#   NC2 输出 q 扰动 1 行       ⇒ C 必须红（门侧重算真的在算）
#   NC3 输出 r 扰动成负数      ⇒ C2（余数域）必须红
#   NC4 被测程序换 C 截断除法   ⇒ B 必须红（门能区分欧几里得 vs C 语义）
#
# 用法：verify.sh [--neg-skip]   # --neg-skip 跳过 NC4（需重编 VM/C 两轨）
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m362_divmod.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT
mkdir -p "$WORK"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 1; }

# ── 唯一用例源（.px 程序与参考值都从这里生成，防两处漂移）────
PAIRS="17:3 7:2 100:7 3:5 -7:-2 -17:-3 -100:-7 17:-3 7:-2 100:-7 -17:3 -7:2 -100:7 1:-3 -1:3 0:5 0:-5 -2:1 2:-1"
NCASE=$(echo "$PAIRS" | wc -w)

gen_prog() {  # $1=输出 .px  $2=normal|truncate
    local out="$1" mode="${2:-normal}" pxpairs="" body=""
    for pr in $PAIRS; do pxpairs="$pxpairs[${pr%%:*}, ${pr##*:}], "; done
    pxpairs="[${pxpairs%, }]"
    if [ "$mode" = "normal" ]; then
        body='        let q = a // b
        let r = a % b'
    else
        # C 截断语义：r_trunc 与被除数同号；q_trunc = (a - r_trunc) / b
        body='        let re = a % b
        var rt = re
        let ab = -b if b < 0 else b
        if a < 0 and re != 0:
            rt = re - ab
        let q = (a - rt) // b
        let r = rt'
    fi
    {
        echo 'def main():'
        printf '    let pairs = %s\n' "$pxpairs"
        echo '    for p in pairs:'
        echo '        let a = p[0]'
        echo '        let b = p[1]'
        printf '%s\n' "$body"
        echo '        print(str(a) + " " + str(b) + " " + str(q) + " " + str(r))'
    } > "$out"
}

gen_ref() {  # $1=输出参考表（纯 bash 算术，不含 // 或 % 语义依赖）
    local out="$1" a b absb r q
    : > "$out"
    for pr in $PAIRS; do
        a="${pr%%:*}"; b="${pr##*:}"
        absb=$(( b < 0 ? -b : b ))
        r=$(( a % absb )); [ "$r" -lt 0 ] && r=$(( r + absb ))
        q=$(( (a - r) / b ))
        printf '%s %s %s %s\n' "$a" "$b" "$q" "$r" >> "$out"
    done
}

# ── 三轨产出 ────────────────────────────────────────────────
run_lanes() {  # $1=prog $2=前缀  → $2.run / .vm / .c / .rc
    local prog="$1" pfx="$2" d name
    d="$(dirname "$prog")"; name="$(basename "$prog" .px)"
    timeout 60 "$PX" run "$prog" > "$pfx.run" 2>"$pfx.run.err"; local rc_run=$?
    rm -rf "$d/build"
    timeout 200 "$PX" build "$prog" > "$pfx.vmlog" 2>&1; local rc_vm=$?
    if [ $rc_vm -eq 0 ] && [ -x "$d/build/$name" ]; then timeout 60 "$d/build/$name" > "$pfx.vm" 2>"$pfx.vm.err"; else : > "$pfx.vm"; fi
    rm -rf "$d/build"
    timeout 300 "$PX" build --c "$prog" > "$pfx.clog" 2>&1; local rc_c=$?
    if [ $rc_c -eq 0 ] && [ -x "$d/build/$name" ]; then timeout 60 "$d/build/$name" > "$pfx.c" 2>"$pfx.c.err"; else : > "$pfx.c"; fi
    printf '%s %s %s\n' "$rc_run" "$rc_vm" "$rc_c" > "$pfx.rc"
}

# 门侧重算：只读 (a b q r)，不读被测程序自报的任何结论列
check_semantics() {
    # 两条判据**各自独立**判定（不能写成 if/else —— 否则 r 越界会先命中 a!=q*b+r，
    # C2 永远没机会发言，NC3 就红不了，等于余数域这条判据是摆设）
    awk '{ a=$1+0; b=$2+0; q=$3+0; r=$4+0; ab=(b<0?-b:b)
           if (a != q*b + r)          print "C "NR": a!=q*b+r ["$0"]"
           if (r < 0 || r >= ab)      print "C2 "NR": r out of [0,|b|) ["$0"]" }' "$1"
}

echo "═══ M362 · \`//\` / \`%\` 逐值对拍门 ═══"

gen_prog "$WORK/p.px"
gen_ref  "$WORK/ref.txt"
run_lanes "$WORK/p.px" "$WORK/o"
read -r RC_RUN RC_VM RC_C < "$WORK/o.rc"
note "用例 $NCASE 组 · 轨道退出码：解释=$RC_RUN VM=$RC_VM C=$RC_C"

# ── A 三轨逐字节一致 ────────────────────────────────────────
if [ "$RC_RUN" -ne 0 ]; then bad "A0 解释轨非 0（rc=$RC_RUN）：$(head -2 "$WORK/o.run.err" | tr '\n' ' ')"
else ok "A0 解释轨 rc=0"; fi
[ -s "$WORK/o.run" ] && ok "A1 解释轨有输出（$(wc -l < "$WORK/o.run") 行）" || bad "A1 解释轨无输出"
for lane in vm c; do
    if [ "$lane" = "vm" ]; then rc=$RC_VM; else rc=$RC_C; fi
    if [ "$rc" -ne 0 ]; then bad "A2-$lane 轨道非 0（rc=$rc）：$(tail -2 "$WORK/o.$lane""log" 2>/dev/null | tr '\n' ' ')"
    elif cmp -s "$WORK/o.run" "$WORK/o.$lane"; then ok "A2-$lane 与解释轨逐字节一致"
    else bad "A2-$lane 与解释轨不一致：$(diff "$WORK/o.run" "$WORK/o.$lane" 2>/dev/null | grep '^[<>]' | head -4 | tr '\n' ' ')"
    fi
done

# ── B 与独立参考值对拍 ─────────────────────────────────────
if cmp -s "$WORK/ref.txt" "$WORK/o.run"; then
    ok "B  $NCASE/$NCASE 与欧几里得参考值逐字节一致"
else
    bad "B  与参考值不符：$(diff "$WORK/ref.txt" "$WORK/o.run" 2>/dev/null | grep '^[<>]' | head -6 | tr '\n' ' ')"
fi

# ── C 门侧重算不变量（不读程序自报）──────────────────────────
cres=$(check_semantics "$WORK/o.run")
[ -z "$(printf '%s' "$cres" | grep '^C ')" ] && ok "C  门侧重算 a==q*b+r 全过（$NCASE 行）" || bad "C  $(printf '%s' "$cres" | tr '\n' '|')"
[ -z "$(printf '%s' "$cres" | grep '^C2')" ] && ok "C2 余数 ∈ [0,|b|) 全过（欧几里得性质，非 floor / C 语义）" || bad "C2 $(printf '%s' "$cres" | tr '\n' '|')"

# ── D 覆盖度自证 ───────────────────────────────────────────
if awk '{ a=$1+0; b=$2+0
          if (a>0 && b>0) o++; if (a<0 && b<0) n++
          if (a>0 && b<0) pn++; if (a<0 && b>0) np++
          if (a==0) z++; if (a>99 || a<-99) big++ }
     END{ exit !(o>=3 && n>=3 && pn>=3 && np>=3 && z>=1 && big>=2) }' "$WORK/o.run"; then
    ok "D  覆盖度：四象限各 ≥3 · 含 0 · 含 |a|>99"
else bad "D  用例覆盖度不足（象限/边界退化）"; fi

# ── NC1 参考值扰动 ⇒ B 必须红 ──────────────────────────────
awk 'NR==3 { $3 = $3 + 1; print; next } { print }' "$WORK/ref.txt" > "$WORK/ref.bad"
if cmp -s "$WORK/ref.bad" "$WORK/o.run"; then bad "NC1 扰动参考值仍判一致 ⇒ B 无鉴别力"
else ok "NC1 参考值扰动 1 行 ⇒ B 判红"; fi

# ── NC2 输出 q 扰动 ⇒ C 必须红 ─────────────────────────────
awk 'NR==3 { $3 = $3 + 1; print; next } { print }' "$WORK/o.run" > "$WORK/o.qbad"
if [ -n "$(check_semantics "$WORK/o.qbad" | grep '^C ')" ]; then ok "NC2 q 扰动 1 行 ⇒ 门侧重算判红"
else bad "NC2 q 扰动后 C 仍绿 ⇒ C 没真在算"; fi

# ── NC3 输出 r 扰动成负数 ⇒ C2 必须红 ──────────────────────
awk 'NR==3 { $4 = -1; print; next } { print }' "$WORK/o.run" > "$WORK/o.rbad"
if [ -n "$(check_semantics "$WORK/o.rbad" | grep '^C2')" ]; then ok "NC3 r 扰动成 -1 ⇒ C2 判红（余数域判据有效）"
else bad "NC3 r 越界仍绿 ⇒ C2 无鉴别力"; fi

# ── NC4 C 截断除法 ⇒ B 必须红 ──────────────────────────────
if [ "$NEG_SKIP" = "1" ]; then
    note "NC4 已跳过（--neg-skip）"
else
    gen_prog "$WORK/p_nc4.px" truncate
    timeout 60 "$PX" run "$WORK/p_nc4.px" > "$WORK/n4.run" 2>&1
    if cmp -s "$WORK/ref.txt" "$WORK/n4.run"; then
        bad "NC4 截断除法与欧几里得结果相同 ⇒ 门区分不了两种语义"
    else
        ok "NC4 截断除法被 B 判红（$(diff "$WORK/ref.txt" "$WORK/n4.run" 2>/dev/null | grep -c '^[<>]') 行差异）⇒ 门能区分欧几里得 vs C 语义"
    fi
fi

echo "───────────────────────────────"
echo "M362-DIVMOD: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] && { echo "M362-DIVMOD-OK"; exit 0; } || { echo "M362-DIVMOD-FAIL"; exit 1; }
