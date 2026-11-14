#!/usr/bin/env bash
# ============================================================
# examples/m406_struct_field_default/verify.sh —— 结构体字段默认值 三轨对拍门（M406）
# ------------------------------------------------------------
# 为什么有这道门：
#   docs/spec.md §3.7 从第一版起就写着 `timeout: int = 30`，而 M365 起 parser 明确拒绝
#   （E2001「结构体字段默认值尚未实现」）——「文档承诺、实现拒绝」，与 M362/M365 同族。
#   M406 把这条兑现，并**同时**定死三件容易分叉的事（spec §3.7 定稿）：
#     ① 求值环境 = 模块顶层（看不到其他字段、看不到 self、看不到调用点局部）
#     ② 求值时机 = 构造时、实参全部求值之后，按字段声明顺序
#     ③ 缺字段解析顺序：位置参 → 命名参 → 默认值 → 仍缺 ⇒ 原 R2001（报文逐字节不变）
#
# 判据（每条都是「被测 vs 外部参考」，禁止实现自证）：
#   A 三轨逐字节一致     解释轨 / VM 轨 / C 轨 stdout 完全相同
#   B 与门侧独立参考     期望值按 §3.7 三条规则**手推**写定（不调实现、不跑文档表格）
#   C 错误类三轨同码同文 6 类错误 × 3 轨，归一化（去行号/前缀）后逐字比对；
#                        另加门侧谓词：码必须是 R2001/R1001 且报文含关键名
#   D 覆盖度自证         正向覆盖 T1 全默认 / T2 位置部分 / T3 命名部分 / T4 混合 /
#                        T6 全局引用 / T8 求值顺序；错误类 ≥6 种（防用例退化后门变空转）
# 负控（证明判据有鉴别力）：
#   N1 ref      期望值扰动 1 处                      ⇒ B 必须红
#   N2 okform   合法写法（命名+位置混合 / 默认引用全局函数）必须不被判红 ⇒ 判据不是恒红
#   P  前提自证 v0.2.405 的 bootstrap/pxi 在本用例上必须报 E2001「结构体字段默认值尚未实现」
#
# 用法：verify.sh [--neg-skip]     # --neg-skip 跳过负控（CI 慢机用）
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m406_sfd.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
NC_MODE="${M406_NC:-}"

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT
mkdir -p "$WORK"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 1; }

# ── 正向主用例（T1–T4 / T6 / T8）─────────────────────────────
cat > "$WORK/main.px" <<'PXEOF'
let BASE = 3

var seq = ""

def tick(n: int) -> int:
    seq = seq + str(n)
    return n

struct Config:
    timeout: int = 30
    retries: int = BASE
    name: str = "default"

struct G:
    v: int = BASE * 2

struct T:
    p: int = tick(1)
    q: int = tick(2)
    r: int = tick(3)

def main():
    let c1 = Config()
    print(str(c1.timeout) + "," + str(c1.retries) + "," + c1.name)
    let c2 = Config(5)
    print(str(c2.timeout) + "," + str(c2.retries) + "," + c2.name)
    let c3 = Config(retries: 9)
    print(str(c3.timeout) + "," + str(c3.retries) + "," + c3.name)
    let c4 = Config(5, name: "x")
    print(str(c4.timeout) + "," + str(c4.retries) + "," + c4.name)
    print(str(G().v))
    seq = ""
    let t1 = T()
    print(seq)
    print(str(t1.p) + "," + str(t1.q) + "," + str(t1.r))
    seq = ""
    let t2 = T(tick(9))
    print(seq)
    print(str(t2.p) + "," + str(t2.q) + "," + str(t2.r))
PXEOF

# ── B：门侧独立参考（按 §3.7 手推；**不调实现**）───────────────
cat > "$WORK/expect.txt" <<'EXPEOF'
30,3,default
5,3,default
30,9,default
5,3,x
6
123
1,2,3
923
9,2,3
EXPEOF
# N1：期望值扰动一处（只改第一行）⇒ B 必须判红
[ "$NC_MODE" = "ref" ] && sed -i '1s/.*/30,4,default/' "$WORK/expect.txt"

# ── 三轨执行 ────────────────────────────────────────────────
# run_lanes <prog> <pfx> → 写 $pfx.run/.vm/.c 与 $pfx.rc（rc 三元组）
run_lanes() {
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

# 错误归一化：解释轨 `运行时错误: 错误 [R1001] 3:14: 体`
#             编译两轨 `运行时错误 [main 行6]: R1001: 体`
#             解析错误 `6:21: 语法错误 E2001: 体`
norm_err() {
    sed -e 's/^运行时错误: 错误 \[\([A-Z][0-9]*\)\] [0-9]*:[0-9]*: /\1|/' \
        -e 's/^运行时错误 \[[^]]*\]: \([A-Z][0-9]*\): /\1|/' \
        -e 's/^[0-9]*:[0-9]*: 语法错误 \([A-Z][0-9]*\): /\1|/' "$1" 2>/dev/null \
        | grep -E '^[A-Z][0-9]*\|' | head -1
}

echo "═══ M406 · 结构体字段默认值 三轨对拍门 ═══"

# ── A/B：正向 ───────────────────────────────────────────────
run_lanes "$WORK/main.px" "$WORK/m"
read -r RC_RUN RC_VM RC_C < "$WORK/m.rc"
if [ "$RC_RUN" -ne 0 ] || [ "$RC_VM" -ne 0 ] || [ "$RC_C" -ne 0 ]; then
    bad "A 轨道非 0（run=$RC_RUN vm=$RC_VM c=$RC_C）：$(head -1 "$WORK/m.run.err" 2>/dev/null | cut -c1-90)"
elif cmp -s "$WORK/m.run" "$WORK/m.vm" && cmp -s "$WORK/m.run" "$WORK/m.c"; then
    ok "A 三轨逐字节一致（$(wc -l < "$WORK/m.run") 行）"
else
    bad "A 三轨不一致：run=$(tr '\n' '|' < "$WORK/m.run" | cut -c1-60) vm=$(tr '\n' '|' < "$WORK/m.vm" | cut -c1-60) c=$(tr '\n' '|' < "$WORK/m.c" | cut -c1-60)"
fi
for lane in run vm c; do
    if cmp -s "$WORK/expect.txt" "$WORK/m.$lane"; then
        ok "B-$lane 与门侧独立参考一致"
    else
        bad "B-$lane 与门侧参考不符：$(diff "$WORK/expect.txt" "$WORK/m.$lane" | head -4 | tr '\n' ' ')"
    fi
done

# ── D 覆盖度自证：用例没退化成空转 ────────────────────────────
NLINES=$(wc -l < "$WORK/expect.txt")
if [ "$NLINES" -ge 9 ]; then
    ok "D 正向覆盖 $NLINES 行（T1/T2/T3/T4/T6/T8 六类必备形态，含求值顺序两组）"
else
    bad "D 正向覆盖退化：只有 $NLINES 行（<9）⇒ 门可能已变空转"
fi

# ── C：错误类三轨同码同文 ───────────────────────────────────
#   期望只写「码 + 必须出现的关键词」；完整报文由三轨互相对拍，不抄实现文本
ERR_SPEC=(
  'e1_missing|R2001|需要 2 个字段|给出 1'
  'e2_fieldref|R1001|未定义变量|a'
  'e3_localref|R1001|未定义变量|y'
  'e4_over|R2001|需要 2 个字段|给出 3'
  'e5_unknown|R2001|没有字段|z'
  'e6_namedmiss|R2001|缺少字段|x'
)
for spec in "${ERR_SPEC[@]}"; do
    IFS='|' read -r ename ecode ekey ekey2 <<< "$spec"
    f="$WORK/$ename.px"
    case "$ename" in
      e1_missing) printf 'struct P:\n    x: int\n    y: int\n\ndef main():\n    print(P(1))\n' > "$f" ;;
      e2_fieldref) printf 'struct B:\n    a: int = 1\n    b: int = a + 1\n\ndef main():\n    print(B().b)\n' > "$f" ;;
      e3_localref) printf 'struct L:\n    v: int = y\n\ndef f() -> int:\n    let y = 7\n    return L().v\n\ndef main():\n    print(f())\n' > "$f" ;;
      e4_over)    printf 'struct P:\n    x: int = 1\n    y: int = 2\n\ndef main():\n    print(P(1, 2, 3))\n' > "$f" ;;
      e5_unknown) printf 'struct P:\n    x: int = 1\n    y: int = 2\n\ndef main():\n    print(P(z: 1))\n' > "$f" ;;
      e6_namedmiss) printf 'struct M:\n    x: int\n    y: int = 2\n\ndef main():\n    print(M(y: 1))\n' > "$f" ;;
    esac
    pfx="$WORK/$ename"
    timeout 60 "$PX" run "$f" > "$pfx.run.out" 2>"$pfx.run.err"
    rm -rf "$WORK/build"; timeout 200 "$PX" build "$f" > "$pfx.vmlog" 2>&1
    if [ -x "$WORK/build/$ename" ]; then timeout 60 "$WORK/build/$ename" > "$pfx.vm.out" 2>"$pfx.vm.err"; else : > "$pfx.vm.err"; fi
    rm -rf "$WORK/build"; timeout 300 "$PX" build --c "$f" > "$pfx.clog" 2>&1
    if [ -x "$WORK/build/$ename" ]; then timeout 60 "$WORK/build/$ename" > "$pfx.c.out" 2>"$pfx.c.err"; else : > "$pfx.c.err"; fi
    rm -rf "$WORK/build"
    r1="$(norm_err "$pfx.run.err")"; r2="$(norm_err "$pfx.vm.err")"; r3="$(norm_err "$pfx.c.err")"
    [ -z "$r2" ] && r2="$(norm_err "$pfx.vmlog")"
    [ -z "$r3" ] && r3="$(norm_err "$pfx.clog")"
    if [ -z "$r1" ] || [ -z "$r2" ] || [ -z "$r3" ]; then
        bad "C-$ename 某轨没报出可识别的错误（run='${r1}' vm='${r2}' c='${r3}'）"
        continue
    fi
    if [ "$r1" != "$r2" ] || [ "$r1" != "$r3" ]; then
        bad "C-$ename 三轨不同码同文：run='$r1' vm='$r2' c='$r3'"
        continue
    fi
    case "$r1" in
      "$ecode|"*) : ;;
      *) bad "C-$ename 错误码不符：期望 $ecode，实得 $r1"; continue ;;
    esac
    if ! printf '%s' "$r1" | grep -q "$ekey"; then
        bad "C-$ename 报文缺关键词「$ekey」：$r1"; continue
    fi
    if [ -n "$ekey2" ] && ! printf '%s' "$r1" | grep -q "$ekey2"; then
        bad "C-$ename 报文缺关键词「$ekey2」：$r1"; continue
    fi
    ok "C-$ename 三轨同码同文（$r1）"
done

# ── N2 判据自证：合法写法必须不被判红（判据不是恒红）──────────
cat > "$WORK/okform.px" <<'PXEOF'
let MUL = 4

def scale(n: int) -> int:
    return n * MUL

struct S:
    a: int = scale(2)
    b: int = 7

def main():
    let s1 = S()
    let s2 = S(1, b: 5)
    print(str(s1.a) + "," + str(s1.b) + "," + str(s2.a) + "," + str(s2.b))
PXEOF
run_lanes "$WORK/okform.px" "$WORK/ok"
read -r RC_RUN RC_VM RC_C < "$WORK/ok.rc"
if [ "$RC_RUN" -eq 0 ] && [ "$RC_VM" -eq 0 ] && [ "$RC_C" -eq 0 ] \
   && [ "$(tr -d '\n' < "$WORK/ok.run")" = "8,7,1,5" ] \
   && cmp -s "$WORK/ok.run" "$WORK/ok.vm" && cmp -s "$WORK/ok.run" "$WORK/ok.c"; then
    ok "N2 判据自证：合法写法（默认引用全局函数 · 命名+位置混合）三轨皆绿且值正确（8,7,1,5）"
else
    bad "N2 判据自证失败：合法写法被误判（rc=$RC_RUN/$RC_VM/$RC_C 输出=$(tr '\n' '|' < "$WORK/ok.run" | cut -c1-50)）"
fi

# ── N1 判据自证：期望值扰动必须让 B 判红 ──────────────────────
if [ "$NEG_SKIP" = 0 ]; then
    if [ "$NC_MODE" = "ref" ]; then
        if cmp -s "$WORK/expect.txt" "$WORK/m.run"; then
            bad "N1 判据自证失败：期望值被扰动后 B 仍判绿 ⇒ B 无鉴别力"
        else
            ok "N1 判据自证：期望值扰动 1 处 ⇒ B 判红（判据有鉴别力）"
        fi
    else
        note "N1 负控需 M406_NC=ref 重跑（门内自检由 registry 的负控钩子触发）"
    fi
fi

# ── P 前提自证：v0.2.405 上该语法必须被拒（证明门不是空转）────
P_TAG="${PX_SFD_BASE_TAG:-v0.2.405}"
if git -C "$ROOT" cat-file -e "$P_TAG:bootstrap/pxi" 2>/dev/null; then
    git -C "$ROOT" show "$P_TAG:bootstrap/pxi" > "$WORK/pxi_old" 2>/dev/null
    chmod +x "$WORK/pxi_old"
    out_old="$("$WORK/pxi_old" "$WORK/main.px" 2>&1)"
    if printf '%s' "$out_old" | grep -q '结构体字段默认值尚未实现'; then
        ok "P 前提自证：$P_TAG 报 E2001「结构体字段默认值尚未实现」（旧口径 ⇒ 本轮的改动可见）"
    else
        bad "P 前提自证：$P_TAG 未按旧口径拒绝（$(printf '%s' "$out_old" | head -1 | cut -c1-80)）"
    fi
else
    note "P 前提自证跳过（本地无标签 $P_TAG 的 bootstrap/pxi）"
fi

echo "── m406_struct_field_default：通过 $PASS · 失败 $FAIL"
[ "$FAIL" = 0 ] || exit 1
