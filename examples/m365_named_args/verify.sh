#!/usr/bin/env bash
# ============================================================
# examples/m365_named_args/verify.sh —— 命名参数构造（结构体）三轨对拍门（M365）
# ------------------------------------------------------------
# 为什么有这道门：
#   docs/spec.md §3.7 从第一版起就写着 `p = Point(x: 1, y: 2)`，
#   而 parser 只吃位置参数 ⇒ 照文档写必得 `E2001 期望 ')'，实际得到 :`
#   （「文档承诺、实现拒绝」，与 M362 抓到的 MINI_SUBSET §三 同族）。
#   M365 把这条兑现，并**同时**定死两件容易分叉的事：
#     ① 求值顺序：实参按**书写顺序**求值，命名只决定装到哪个字段
#     ② 报错报文：三轨同码（R2001）同文
#
# 判据（每条都是「被测 vs 外部参考」，禁止实现自证）：
#   A 三轨逐字节一致   解释轨 / VM 轨 / C 轨 stdout 完全相同
#   B 与门侧独立参考   门用 bash 依「位置参填未占用字段 + 命名参按名填」现算期望值，
#                      不抄实现、不抄文档表格（用例表 → 程序与期望都由同一表生成）
#   C 错误报文三轨同码同文  7 类错误 × 3 轨，归一化（去行号/前缀）后逐字比对；
#                      另加门侧谓词：码必须是 R2001/E2001 且报文含**结构体名与字段名**
#   D 覆盖度自证       正向用例 ≥5 且含「全命名 / 全位置 / 混合 / 逆序」四种形态；
#                      错误类 ≥7 种（防用例退化后门变空转，恒绿）
# 负控（证明判据有鉴别力）：
#   NC1 ref      期望值扰动 1 处        ⇒ B 必须红
#   NC2 swap     程序里两个命名值互换    ⇒ B 必须红（装配错位能被抓住）
#   NC3 errtext  错误期望报文改动 1 处   ⇒ C 必须红
#
# 用法：verify.sh [--neg-skip]   # --neg-skip 跳过负控（CI 慢机用）
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m365_named_args.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
NC_MODE="${M365_NC:-}"

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT
mkdir -p "$WORK"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 1; }

# ── 用例表：token 形如 `a=1`（命名）或 `1`（位置）；字段序 a,b,c ────────
#   程序文本与期望值都从这张表生成 ⇒ 两处不可能漂移。
CASES=(
  'a=1,b=2,c="x"'          # 全命名、声明序
  'b=1,a=3,c="q"'          # 全命名、逆序
  '7,8,"z"'                # 全位置
  '7,b=9,c="w"'            # 位置 + 命名混合（位置先填 a，命名填 b/c）
  '4,c="m",b=6'            # 位置 + 命名混合、命名逆序
)
FIELDS=(a b c)

gen_case() {  # $1=case  $2=输出 px
    #   表里用 `a=1` 作「命名」标记（bash 好解析），生成程序时换成语言语法 `a: 1`
    local spec="$1" out="$2"
    local -a toks=()
    IFS=',' read -r -a toks <<< "$spec"
    local args="" tok
    for tok in "${toks[@]}"; do
        if [[ "$tok" == *=* ]]; then tok="${tok%%=*}: ${tok#*=}"; fi
        args="$args${args:+, }$tok"
    done
    {
        echo 'struct S:'
        echo '    a: int'
        echo '    b: int'
        echo '    c: str'
        echo ''
        echo 'def main():'
        if [ "$NC_MODE" = "swap" ] && [ "$(basename "$out" .px)" = "c1" ]; then
            # NC2：把两个命名值互换（期望值不变）⇒ 装配错位必须被 B 抓住
            echo '    let s = S(a: 2, b: 1, c: "x")'
        else
            printf '    let s = S(%s)\n' "$args"
        fi
        echo '    print(str(s.a) + "," + str(s.b) + "," + s.c)'
    } > "$out"
}

gen_exp() {  # $1=case  → stdout 期望值（门侧独立实现「位置填未占用字段 + 命名按名填」）
    local spec="$1"
    local -a toks=()
    IFS=',' read -r -a toks <<< "$spec"
    declare -A fill=()
    local pos=0 tok fld val
    for tok in "${toks[@]}"; do
        if [[ "$tok" == *=* ]]; then
            fld="${tok%%=*}"; val="${tok#*=}"
        else
            while [ "$pos" -lt ${#FIELDS[@]} ] && [ -n "${fill[${FIELDS[$pos]}]:-}" ]; do pos=$((pos+1)); done
            fld="${FIELDS[$pos]}"; val="$tok"; pos=$((pos+1))
        fi
        fill[$fld]="$val"
    done
    local out="" f
    for f in "${FIELDS[@]}"; do
        val="${fill[$f]:-<缺>}"
        val="${val%\"}"; val="${val#\"}"
        out="$out${out:+,}$val"
    done
    printf '%s\n' "$out"
}

# ── 三轨产出 ────────────────────────────────────────────────
run_lanes() {  # $1=prog $2=前缀  → $2.run $2.vm $2.c（stdout）· $2.rc
    local prog="$1" pfx="$2" d name
    d="$(dirname "$prog")"; name="$(basename "$prog" .px)"
    timeout 60 "$PX" run "$prog" > "$pfx.run" 2>"$pfx.run.err"; local rc_run=$?
    rm -rf "$d/build"
    timeout 200 "$PX" build "$prog" > "$pfx.vmlog" 2>&1; local rc_vm=$?
    if [ $rc_vm -eq 0 ] && [ -x "$d/build/$name" ]; then timeout 60 "$d/build/$name" > "$pfx.vm" 2>"$pfx.vm.err"; else : > "$pfx.vm"; fi
    rm -rf "$d/build"
    timeout 300 "$PX" build --c "$prog" > "$pfx.clog" 2>&1; local rc_c=$?
    if [ $rc_c -eq 0 ] && [ -x "$d/build/$name" ]; then timeout 60 "$d/build/$name" > "$pfx.c" 2>"$pfx.c.err"; else : > "$pfx.c"; fi
    rm -rf "$d/build"
    printf '%s %s %s\n' "$rc_run" "$rc_vm" "$rc_c" > "$pfx.rc"
}

# 错误报文归一化：解释轨 `运行时错误: 错误 [R2001] 6:14: 体`
#                编译两轨 `运行时错误 [main 行6]: R2001: 体`
#                解析错误 `6:21: 语法错误 E2001: 体`
norm_err() {  # $1=文件 → 首条 "码|体"
    sed -e 's/^运行时错误: 错误 \[\([A-Z][0-9]*\)\] [0-9]*:[0-9]*: /\1|/' \
        -e 's/^运行时错误 \[[^]]*\]: \([A-Z][0-9]*\): /\1|/' \
        -e 's/^[0-9]*:[0-9]*: 语法错误 \([A-Z][0-9]*\): /\1|/' "$1" 2>/dev/null \
        | grep -E '^[A-Z][0-9]*\|' | head -1
}

echo "═══ M365 · 命名参数构造（结构体）三轨对拍门 ═══"

# ── A/B：正向用例 ───────────────────────────────────────────
NCASE=${#CASES[@]}
i=0
while [ "$i" -lt "$NCASE" ]; do
    n=$((i+1)); f="$WORK/c$n.px"; pfx="$WORK/o$n"
    gen_case "${CASES[$i]}" "$f"
    exp="$(gen_exp "${CASES[$i]}")"
    [ "$NC_MODE" = "ref" ] && [ "$n" -eq 1 ] && exp="9,8,改过的期望值"
    run_lanes "$f" "$pfx"
    read -r RC_RUN RC_VM RC_C < "$pfx.rc"
    # A 三轨一致
    if [ "$RC_RUN" -ne 0 ] || [ "$RC_VM" -ne 0 ] || [ "$RC_C" -ne 0 ]; then
        bad "A$n 轨道非 0（run=$RC_RUN vm=$RC_VM c=$RC_C）用例 ${CASES[$i]}：$(head -1 "$pfx.run.err" 2>/dev/null)"
    elif cmp -s "$pfx.run" "$pfx.vm" && cmp -s "$pfx.run" "$pfx.c"; then
        ok "A$n 三轨逐字节一致（${CASES[$i]} → $(tr -d '\n' < "$pfx.run")）"
    else
        bad "A$n 三轨不一致：run=$(tr -d '\n' < "$pfx.run") vm=$(tr -d '\n' < "$pfx.vm") c=$(tr -d '\n' < "$pfx.c")"
    fi
    # B 与门侧独立参考对拍（三轨都要对，不只解释轨）
    for lane in run vm c; do
        got="$(tr -d '\n' < "$pfx.$lane")"
        if [ "$got" = "$exp" ]; then
            ok "B$n-$lane 与门侧参考一致（$exp）"
        else
            bad "B$n-$lane 与门侧参考不符：期望 $exp，实得 $got"
        fi
    done
    i=$((i+1))
done

# ── C：错误报文三轨同码同文 ─────────────────────────────────
#   期望只写「码 + 必须出现的关键词」——完整报文由三轨互相对拍，不抄实现文本
ERR_SPEC=(
  'e1_unknown|R2001|没有字段|z'
  'e2_dup|R2001|重复给出|a'
  'e3_missing|R2001|缺少字段|b'
  'e4_func|R2001|仅支持结构体构造|add'
  'e5_posafter|E2001|位置参数不能出现在命名参数之后|'
  'e6_method|R2001|仅支持结构体构造|'
  'e7_over|R2001|需要 2 个字段|3'
)
NE=${#ERR_SPEC[@]}
for spec in "${ERR_SPEC[@]}"; do
    IFS='|' read -r ename ecode ekey ekey2 <<< "$spec"
    f="$WORK/$ename.px"
    case "$ename" in
      e1_unknown) printf 'struct P:\n    a: int\n    b: int\n\ndef main():\n    let p = P(a: 1, z: 2)\n    print("ok")\n' > "$f" ;;
      e2_dup)     printf 'struct P:\n    a: int\n    b: int\n\ndef main():\n    let p = P(a: 1, a: 2)\n    print("ok")\n' > "$f" ;;
      e3_missing) printf 'struct P:\n    a: int\n    b: int\n\ndef main():\n    let p = P(a: 1)\n    print("ok")\n' > "$f" ;;
      e4_func)    printf 'def add(a: int, b: int) -> int:\n    return a + b\n\ndef main():\n    print(str(add(a: 1, b: 2)))\n' > "$f" ;;
      e5_posafter) printf 'struct P:\n    a: int\n    b: int\n\ndef main():\n    let p = P(a: 1, 2)\n    print("ok")\n' > "$f" ;;
      e6_method)  printf 'struct P:\n    a: int\n\nimpl P:\n    def get(self, k: int) -> int:\n        return self.a + k\n\ndef main():\n    let p = P(1)\n    print(str(p.get(k: 2)))\n' > "$f" ;;
      e7_over)    printf 'struct P:\n    a: int\n    b: int\n\ndef main():\n    let p = P(1, 2, 3)\n    print("ok")\n' > "$f" ;;
    esac
    pfx="$WORK/$ename"
    read -r RC_RUN RC_VM RC_C < /dev/null || true
    timeout 60 "$PX" run "$f" > "$pfx.run.out" 2>"$pfx.run.err"
    rm -rf "$WORK/build"; timeout 200 "$PX" build "$f" > "$pfx.vmlog" 2>&1
    if [ -x "$WORK/build/$ename" ]; then timeout 60 "$WORK/build/$ename" > "$pfx.vm.out" 2>"$pfx.vm.err"; else : > "$pfx.vm.err"; fi
    rm -rf "$WORK/build"; timeout 300 "$PX" build --c "$f" > "$pfx.clog" 2>&1
    if [ -x "$WORK/build/$ename" ]; then timeout 60 "$WORK/build/$ename" > "$pfx.c.out" 2>"$pfx.c.err"; else : > "$pfx.c.err"; fi
    rm -rf "$WORK/build"
    r1="$(norm_err "$pfx.run.err")"; r2="$(norm_err "$pfx.vm.err")"; r3="$(norm_err "$pfx.c.err")"
    # 编译两轨的解析错误在 build 日志里，不在运行期 stderr
    [ -z "$r2" ] && r2="$(norm_err "$pfx.vmlog")"
    [ -z "$r3" ] && r3="$(norm_err "$pfx.clog")"
    [ "$NC_MODE" = "errtext" ] && [ "$ename" = "e1_unknown" ] && r1="R2001|改过的错误报文"
    if [ -z "$r1" ]; then
        bad "C-$ename 解释轨无错误报文（应报 $ecode）"
        continue
    fi
    if [ "$r1" = "$r2" ] && [ "$r1" = "$r3" ]; then
        ok "C-$ename 三轨同码同文（$r1）"
    else
        bad "C-$ename 三轨报文不一致：run=[$r1] vm=[$r2] c=[$r3]"
    fi
    # 门侧谓词：码 + 关键词（谓词不依赖实现文本）
    code_only="${r1%%|*}"
    if [ "$code_only" = "$ecode" ] && [[ "$r1" == *"$ekey"* ]] && { [ -z "$ekey2" ] || [[ "$r1" == *"$ekey2"* ]]; }; then
        ok "C-$ename 码与关键词齐（$ecode / $ekey${ekey2:+ / $ekey2}）"
    else
        bad "C-$ename 码或关键词不符：期望 $ecode / $ekey${ekey2:+ / $ekey2}，实得 $r1"
    fi
done

# ── D：覆盖度自证（防空转）──────────────────────────────────
[ "$NCASE" -ge 5 ] && ok "D1 正向用例 $NCASE 组（≥5）" || bad "D1 正向用例只有 $NCASE 组"
[ "$NE" -ge 7 ] && ok "D2 错误类 $NE 种（≥7）" || bad "D2 错误类只有 $NE 种"
ALL_CASES="$(printf '%s\n' "${CASES[@]}")"
if grep -qxF 'a=1,b=2,c="x"' <<< "$ALL_CASES" && grep -qxF '7,8,"z"' <<< "$ALL_CASES" \
   && grep -q '^7,b=9' <<< "$ALL_CASES" && grep -q '^b=1,a=3' <<< "$ALL_CASES"; then
    ok "D3 四种形态齐（全命名 / 全位置 / 混合 / 逆序）"
else
    bad "D3 用例形态不全（防「用例被改空门变恒绿」）"
fi

# ── 负控：门能红 ────────────────────────────────────────────
if [ "$NEG_SKIP" -eq 0 ] && [ -z "$NC_MODE" ]; then
    for pair in "ref:B" "swap:B" "errtext:C"; do
        m="${pair%%:*}"; judge="${pair##*:}"
        out="$(M365_NC="$m" bash "$0" --neg-skip 2>&1 || true)"
        if grep -q "❌ $judge" <<< "$out"; then
            ok "NC-$m 扰动后 $judge 判据如实红"
        else
            bad "NC-$m 扰动后 $judge 没红（判据无鉴别力）"
        fi
    done
else
    note "负控跳过（--neg-skip 或负控子进程）"
fi

echo "───────────────────────────────────────────────"
echo "M365 门结果：✅ $PASS · ❌ $FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
