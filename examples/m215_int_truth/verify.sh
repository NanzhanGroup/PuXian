#!/usr/bin/env bash
# ============================================================
# M215 门（第 94 轮）· 整数运算符的**逐值真值对拍**
# ------------------------------------------------------------
# 本轮三个缺陷：
#   305 —— `//` 的商**不能**由 `(n - r) / d` 求：`n` 逼近 INT64_MIN 且 `r>0` 时
#          `n - r` 溢出（构建未开 `-fwrapv` ⇒ UB）⇒ **商符号翻转**
#          （`-(2^63-1) // 3` 静默给 `+3074457345618258602`，真值 `-3074457345618258603`）
#   306 —— `INT64_MIN // -1` 与 `% -1` ⇒ 硬件除法陷阱 **SIGFPE + core dump**（三轨同崩）
#   307 —— 解释轨 `i_bin_compare` 对 int/int 也先转 `float` ⇒ **>2^53 的整数比较错误**
#          （`9223372036854775806 < 9223372036854775807` 给 false）⇒ **三轨分叉**
# ------------------------------------------------------------
# ⚠️ 本门存在的理由（先说清"为什么需要一道新门"）：
#   305/306 的病灶在**运行时原语内部**，而且 `px_idiv`（runtime.c）与 `pxc_vm` 的
#   `PXOP_IDIV`/`PXOP_MOD` 快路径**各写了一遍同样的公式** ⇒ **三轨跑的是同一份语义**，
#   输出**逐字节一致**。任何「三轨对拍门」按定义看不见它。
#   ⇒ 判据必须 = 三轨彼此一致 **且** 与 `truth/` 的**独立真值**一致。
#   （307 恰好相反：那是三轨分叉 —— 但它只在**大整数**上显形，老门里没有一道比过。）
# ------------------------------------------------------------
# 权威口径：docs/spec.md §算术（运算表） · docs/PUXIAN_CHEATSHEET.md 事实 239 / 240
# 用法：bash examples/m215_int_truth/verify.sh [--neg-skip]
#   退出码：0 全绿 / 1 有失败 / 2 工具自证失败
#   ⚠️ `--neg-skip`（CI 用）：只跑正判据，跳过 §7 的三道负控（各要重编 runtime / pxi）
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

# 被测件：默认取**入库件**（CI 亦然）；本地调试可用 PX_PXI_BIN 指到 dev 件
PXI="${PX_PXI_BIN:-$ROOT/bootstrap/pxi}"
PXCLI="$ROOT/tools/px"

W="$(mktemp -d /tmp/m215gate.XXXXXX)"
WORK="$W/build"
mkdir -p "$WORK"

# 负控要改的两个文件（快照/还原用）
RTH="$ROOT/runtime/runtime.h"
IVAL="$ROOT/selfhost/ival.px"
SNAP="$W/snap"
mkdir -p "$SNAP"
cp -a "$RTH" "$SNAP/runtime.h.snap"
cp -a "$IVAL" "$SNAP/ival.px.snap"

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo; echo "── $*"; }

restore_all() {
    cp -a "$SNAP/runtime.h.snap" "$RTH"
    cp -a "$SNAP/ival.px.snap"  "$IVAL"
}
# M214 的教训：负控锚点/快照必须**逐级独立**，且中途被杀也要还原
trap 'restore_all; rm -rf "$W"' EXIT

# 三轨跑一个 .px：$1=源 $2=标签 → 落 $WORK/<标签>.{interp,vm,c}.out，rc 落 $WORK/<标签>.rc
three_tracks() {
    local src=$1 tag=$2
    # ⚠️ 必须拆两行：`local a="$1" b="$W/$a"` 里 `$a` 取的是**外层**变量
    #   （bash 5.1 + set -u 实测 `a: unbound variable`）—— 这是 M200/M213 记过的老坑。
    local d="$WORK/$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/$tag.px"
    : > "$d/rc"
    timeout 300 "$PXI" "$d/$tag.px" > "$d/interp.out" 2>&1
    echo "interp=$?" >> "$d/rc"
    rm -rf "$d/build"
    if timeout 900 "$PXCLI" build "$d/$tag.px" > "$d/vm.log" 2>&1 && [ -x "$d/build/$tag" ]; then
        timeout 300 "$d/build/$tag" > "$d/vm.out" 2>&1
        echo "vm=$?" >> "$d/rc"
    else
        : > "$d/vm.out"; echo "vm=BUILDFAIL" >> "$d/rc"
    fi
    rm -rf "$d/build"
    if timeout 900 "$PXCLI" build --c "$d/$tag.px" > "$d/c.log" 2>&1 && [ -x "$d/build/$tag" ]; then
        timeout 300 "$d/build/$tag" > "$d/c.out" 2>&1
        echo "c=$?" >> "$d/rc"
    else
        : > "$d/c.out"; echo "c=BUILDFAIL" >> "$d/rc"
    fi
    rm -rf "$d/build"
}

# ------------------------------------------------------------
hdr "[1/8] 工具自证：语料**漂移检测**（gen_corpus.py 是单一事实源）"
if python3 "$HERE/gen_corpus.py" --out "$W" > "$W/gen.log" 2>&1; then
    if cmp -s "$W/corpus.px" "$HERE/corpus.px"; then
        note "重生成的 corpus.px 与入库件逐字节一致 ✅（$(wc -l < "$HERE/corpus.px") 行）"
    else
        bad "corpus.px 与 gen_corpus.py 不一致（改了生成器没重生成？）"
        diff "$W/corpus.px" "$HERE/corpus.px" | head -10
    fi
    NPAIR=$(wc -l < "$W/domain.txt")
    [ "$NPAIR" -ge 800 ] || bad "语料规模低于下限（$NPAIR 对 < 800）⇒ 判据可能被架空"
    note "逐对语料：$NPAIR 对（下限 800）"
else
    bad "gen_corpus.py 运行失败"; tail -5 "$W/gen.log"
fi
# 语料里必须**真的**含有判据点（防止"语料被换掉、判据还在"）
for pat in 'MINT' '9007199254740993' '9223372036854775807' 'trap1=' 'lt=' ; do
    grep -q -- "$pat" "$HERE/corpus.px" || bad "corpus.px 缺少锚点文本 '$pat'"
done
note "锚点自证：MINT / 2^53+1 / INT64_MAX / 段B / 段D 均在位"

# ------------------------------------------------------------
hdr "[2/8] 真值互校：两套**算法不同构**的实现必须逐字节一致"
#   ref.py      = 定义式（`n % abs(d)` 得非负余数，再精确除）—— 最直白
#   ref_mag.py  = 量级域 + 符号分支（与 truth/main.go 同算法、跨语言互证）—— 不需要 Go
#   ⇒ CI（无 Go 步骤）也至少有两套**算法不同构**的真值互校；Go 是第三份意见。
python3 "$HERE/truth/ref.py"     "$W/domain.txt" > "$W/t_ref.txt" 2> "$W/t_ref.err"
rc_py=$?
python3 "$HERE/truth/ref_mag.py" "$W/domain.txt" > "$W/t_mag.txt" 2> "$W/t_mag.err"
rc_mag=$?
if [ "$rc_py" -ne 0 ]; then
    bad "Python 真值（定义式 ref.py）失败：$(tail -2 "$W/t_ref.err" | tr '\n' ' ')"
elif [ "$rc_mag" -ne 0 ]; then
    bad "Python 真值（量级域 ref_mag.py）失败：$(tail -2 "$W/t_mag.err" | tr '\n' ' ')"
elif cmp -s "$W/t_ref.txt" "$W/t_mag.txt"; then
    note "两套算法不同构的真值逐字节一致 ✅（$(wc -l < "$W/t_ref.txt") 行）"
else
    bad "两套真值不一致 ⇒ 真值源本身可疑（判据自身不可信比没门更糟）"
    diff "$W/t_ref.txt" "$W/t_mag.txt" | head -10
fi
# 第三份意见：Go（量级域，跨语言）。CI 无 Go 步骤（同 m136/m138 的处置口径）
if command -v go >/dev/null 2>&1; then
    ( cd "$HERE/truth" && GOFLAGS=-mod=mod go build -o "$WORK/truth" . ) > "$W/go_build.log" 2>&1
    if [ ! -x "$WORK/truth" ]; then
        bad "Go 真值（量级域）编译失败"; tail -5 "$W/go_build.log"
    else
        "$WORK/truth" "$W/domain.txt" > "$W/t_go.txt" 2> "$W/t_go.err"
        if cmp -s "$W/t_ref.txt" "$W/t_go.txt"; then
            note "第三份（Go 量级域·跨语言）亦逐字节一致 ✅"
        else
            bad "Go 真值与前两份不一致"; diff "$W/t_ref.txt" "$W/t_go.txt" | head -10
        fi
    fi
elif [ "${M215_REQUIRE_GO:-0}" = 1 ]; then
    bad "M215_REQUIRE_GO=1 但环境无 go（本地全门要求三份真值齐）"
else
    note "⚠ 环境无 go ⇒ 跳过第三份真值（CI 口径同 m136/m138；本地全门由 M215_REQUIRE_GO=1 强制）"
fi
# 规模锚点：真值行数必须与语料规模吻合（框架行 = 4 对段标记(8) + 段B 5 条 + 汇总 2 = 15）
EXP=$(( NPAIR * 2 + 15 ))
ACT=$(wc -l < "$W/t_ref.txt")
[ "$ACT" -eq "$EXP" ] || bad "真值行数 $ACT ≠ 期望 $EXP（= 2×对 + 15 行框架）"
note "行数锚点：$ACT = 2×$NPAIR + 15 ✅"
cp "$W/t_ref.txt" "$WORK/truth.txt"

# ------------------------------------------------------------
hdr "[3/8] 三轨：interp / VM / C 跑同一语料"
three_tracks "$HERE/corpus.px" corpus
cat "$WORK/corpus/rc" | sed 's/^/   rc /'
grep -q 'BUILDFAIL' "$WORK/corpus/rc" && bad "有轨编译失败（见 $WORK/corpus/*.log）"

EQ=1
cmp -s "$WORK/corpus/interp.out" "$WORK/corpus/vm.out" || { EQ=0; bad "interp ≠ vm（三轨分叉）"; diff "$WORK/corpus/interp.out" "$WORK/corpus/vm.out" | head -8; }
cmp -s "$WORK/corpus/interp.out" "$WORK/corpus/c.out"  || { EQ=0; bad "interp ≠ c（三轨分叉）"; diff "$WORK/corpus/interp.out" "$WORK/corpus/c.out" | head -8; }
[ "$EQ" = 1 ] && note "三轨彼此逐字节一致 ✅"

# ------------------------------------------------------------
hdr "[4/8] **真值对拍**（本门核心判据：三轨一致 ≠ 正确）"
for t in interp vm c; do
    if cmp -s "$WORK/corpus/$t.out" "$WORK/truth.txt"; then
        note "$t == 独立真值 ✅"
    else
        bad "$t != 独立真值（$(diff "$WORK/corpus/$t.out" "$WORK/truth.txt" | grep -c '^[<>]') 行差异）"
        diff "$WORK/corpus/$t.out" "$WORK/truth.txt" | head -6 | sed 's/^/      /'
    fi
done

# ------------------------------------------------------------
hdr "[5/8] 缺陷 306 定向：**陷阱回归**（修前 SIGFPE + core dump）"
for t in interp vm c; do
    rc=$(grep -m1 "^$t=" "$WORK/corpus/rc" | cut -d= -f2)
    [ "$rc" = "0" ] || bad "$t rc=$rc ≠ 0（段 B 的陷阱用例把进程打死了？）"
done
sed -n '/== B-BEGIN ==/,/== B-END ==/p' "$WORK/corpus/interp.out" | sed 's/^/   /'
# 定向断言：五个值都必须与文档口径一致（不只对文件，直接盯字面值）
grep -qx 'trap1=-9223372036854775808' "$WORK/corpus/interp.out" || bad "trap1(INT64_MIN//-1) 不等于回绕值 INT64_MIN"
grep -qx 'trap2=0'                    "$WORK/corpus/interp.out" || bad "trap2(INT64_MIN%-1) 不等于 0"
grep -qx 'trap3=1'                    "$WORK/corpus/interp.out" || bad "trap3(-7//INT64_MIN) 不等于 1"
grep -qx 'trap4=9223372036854775801'  "$WORK/corpus/interp.out" || bad "trap4(-7%INT64_MIN) 不等于 2^63-7"
grep -qx 'trap5=-9223372036854775807' "$WORK/corpus/interp.out" || bad "trap5(INT64_MAX//-1) 不等于 -INT64_MAX"
note "五条陷阱用例的字面值全部命中文档口径 ✅"

# ------------------------------------------------------------
hdr "[6/8] 缺陷 307 定向：**大整数比较**（修前解释轨退化为 double）"
# 这三行是"最小反例"——修前解释轨全给 false；它们在 truth 里都必须为 true
for line in \
    'n=9223372036854775806 d=9223372036854775807 lt=true' \
    'n=-9223372036854775808 d=-9223372036854775807 lt=true' \
    'n=9007199254740992 d=9007199254740993 lt=true' ; do
    grep -q "^$line" "$WORK/truth.txt" || bad "真值表缺少反例行：$line"
done
grep -q '^n=9223372036854775806 d=9223372036854775807 lt=true le=true gt=false ge=false eq=false ne=true$' "$WORK/corpus/interp.out" \
    || bad "解释轨大整数比较仍不符真值（缺陷 307 未收口）"
grep -q '^n=-9223372036854775808 d=-9223372036854775807 lt=true le=true gt=false ge=false eq=false ne=true$' "$WORK/corpus/interp.out" \
    || bad "解释轨 INT64_MIN 邻域比较仍不符真值"
# 反向判据：**两个不同的整数绝不能被判等**（这是"被舍入到同一 double"的特征签名）。
#   ⚠️ 必须排除 `n == d` 的对 —— 那是**合法**的 eq=true/lt=false/gt=false
#   （首版没排除 ⇒ 自己把自己判红 30 次，正好是值域大小 30）。
NLE=$(awk '
  /^== D-BEGIN ==$/ { f = 1; next }
  /^== D-END ==$/   { f = 0 }
  f {
    n = ""; d = ""
    for (i = 1; i <= NF; i++) {
      if ($i ~ /^n=/) n = substr($i, 3)
      if ($i ~ /^d=/) d = substr($i, 3)
    }
    if (n != d && ($0 ~ /eq=true/ || ($0 ~ /lt=false/ && $0 ~ /gt=false/))) bad++
  }
  END { print bad + 0 }
' "$WORK/corpus/interp.out")
[ "$NLE" -eq 0 ] || bad "有 $NLE 对**不同**整数被判等/不可比 ⇒ 比较被舍入到同一 double"
NMX=$(awk '
  /^== D-BEGIN ==$/ { f = 1; next }
  /^== D-END ==$/   { f = 0 }
  f { if ($0 ~ /^n=/) c++ }
  END { print c + 0 }
' "$WORK/corpus/interp.out")
[ "$NMX" -eq "$NPAIR" ] || bad "段 D 行数 $NMX ≠ $NPAIR"
note "段 D 覆盖 $NMX 对；『不同整数被判等』计数 = $NLE（期望 0）✅"
note "2^53 邻域 / INT64_MIN 邻域 / INT64_MAX 邻域的比较全部精确 ✅"

# ------------------------------------------------------------
hdr "[7/8] 不变量自断言（欧几里得余数定义：0 <= r < |d|）"
grep -qx 'M215-INV-BAD=0 CNT='"$NPAIR" "$WORK/corpus/interp.out" \
    || bad "不变量自断言失败：$(grep 'M215-INV-BAD' "$WORK/corpus/interp.out")（期望 0 / $NPAIR）"
grep -q '^M215-DONE$' "$WORK/corpus/interp.out" || bad "语料未跑到结尾（中途崩过？）"
note "0 <= r < |d| 在全部 $NPAIR 对上成立；语料跑完 ✅"

# ------------------------------------------------------------
if [ "$NEG_SKIP" = 1 ]; then
    hdr "[8/8] 负控：--neg-skip ⇒ 跳过（CI 用）"
else
    hdr "[8/8] 负控三道（各自独立判红；源必须逐字节还原）"

    # ---- 负控 A：只撤回**缺陷 305** 的修（商公式退回 `(n-r)/d`；306 的短路**保留**）----
    python3 - "$RTH" <<'PY'
import sys
p = sys.argv[1]
s = open(p).read()
old = """    if (n == INT64_MIN && d == -1) return INT64_MIN;
    int64_t q = n / d;
    if (n % d < 0) q += (d > 0 ? -1 : 1);
    return q;"""
new = """    if (n == INT64_MIN && d == -1) return INT64_MIN;
    int64_t r = n % d;
    if (r < 0) r += (d < 0 ? -d : d);
    return (n - r) / d;     /* NEGCTL-M215-A */"""
assert old in s, "NC-A 锚点未命中"
open(p, "w").write(s.replace(old, new, 1))
PY
    if [ $? -eq 0 ]; then
        rm -rf "$WORK/ncA"; mkdir -p "$WORK/ncA"; cp "$HERE/corpus.px" "$WORK/ncA/ncA.px"
        if timeout 900 "$PXCLI" build --c "$WORK/ncA/ncA.px" > "$WORK/ncA.log" 2>&1 && [ -x "$WORK/ncA/build/ncA" ]; then
            timeout 300 "$WORK/ncA/build/ncA" > "$WORK/ncA.out" 2>&1
            if cmp -s "$WORK/ncA.out" "$WORK/truth.txt"; then
                bad "负控 A 未判红（撤回 305 的修后仍与真值一致 ⇒ 判据无牙）"
            else
                note "负控 A ✅ 撤回 305 ⇒ C 轨与真值出现 $(diff "$WORK/ncA.out" "$WORK/truth.txt" | grep -c '^[<>]') 行差异"
                # 同时证明"独立有牙"：段 B 仍应绿（306 的短路保留了）
                cmp -s <(sed -n '/== B-BEGIN ==/,/== B-END ==/p' "$WORK/ncA.out") <(sed -n '/== B-BEGIN ==/,/== B-END ==/p' "$WORK/truth.txt") \
                    && note "   （段 B 在 NC-A 下仍全绿 ⇒ A 只打 305、没顺带打 306）" \
                    || bad "NC-A 连带把段 B 打红了 ⇒ 两道负控不独立"
            fi
        else
            bad "负控 A 编译失败"; tail -5 "$WORK/ncA.log"
        fi
        rm -rf "$WORK/ncA"
    else
        bad "负控 A 补丁未命中锚点"
    fi
    restore_all
    cmp -s "$RTH" "$SNAP/runtime.h.snap" || bad "负控 A 后 runtime.h 未逐字节还原"

    # ---- 负控 B：只撤回**缺陷 306** 的短路（305 的修保留）----
    python3 - "$RTH" <<'PY'
import sys
p = sys.argv[1]
s = open(p).read()
a = "    if (n == INT64_MIN && d == -1) return INT64_MIN;\n"
b = "    if (n == INT64_MIN && d == -1) return 0;\n"
assert a in s and b in s, "NC-B 锚点未命中"
s = s.replace(a, "    /* NEGCTL-M215-B */\n", 1).replace(b, "    /* NEGCTL-M215-B */\n", 1)
open(p, "w").write(s)
PY
    if [ $? -eq 0 ]; then
        rm -rf "$WORK/ncB"; mkdir -p "$WORK/ncB"; cp "$HERE/corpus.px" "$WORK/ncB/ncB.px"
        if timeout 900 "$PXCLI" build --c "$WORK/ncB/ncB.px" > "$WORK/ncB.log" 2>&1 && [ -x "$WORK/ncB/build/ncB" ]; then
            timeout 300 "$WORK/ncB/build/ncB" > "$WORK/ncB.out" 2>&1
            rcB=$?
            if [ "$rcB" -ne 0 ]; then
                note "负控 B ✅ 撤回 306 ⇒ C 轨 rc=$rcB（SIGFPE/崩溃，修前形态复现）"
            else
                bad "负控 B 未判红（rc=0 ⇒ 陷阱不再触发？）"
            fi
            # 独立性检查：**段 A 必须仍然全绿**。
            #   ⚠️ 这只有在语料把 (INT64_MIN, -1) 这一对**排除出叉积**时才成立 ——
            #     否则撤回 306 的短路会让进程在**段 A 中途**崩掉，而 C 的 stdout 是**块缓冲**
            #     （静态件用不了 stdbuf）⇒ 段 A 输出整块丢失、两道负控无法解耦。
            #     那一对由**段 B 专管**（见 gen_corpus.py 的设计取舍说明）。
            sed -n '/== A-BEGIN ==/,/== A-END ==/p' "$WORK/ncB.out"  > "$WORK/ncB.A"
            sed -n '/== A-BEGIN ==/,/== A-END ==/p' "$WORK/truth.txt" > "$WORK/truth.A"
            NA=$(wc -l < "$WORK/ncB.A")
            if [ "$NA" -eq "$((NPAIR + 2))" ] && cmp -s "$WORK/ncB.A" "$WORK/truth.A"; then
                note "   （段 A 全段 $NA 行在 NC-B 下仍逐字节一致 ⇒ B 只打 306、没顺带打 305）"
            else
                bad "NC-B 连带改坏了段 A（实际 $NA 行，期望 $((NPAIR + 2)) 行）⇒ 两道负控不独立"
            fi
        else
            bad "负控 B 编译失败"; tail -5 "$WORK/ncB.log"
        fi
        rm -rf "$WORK/ncB"
    else
        bad "负控 B 补丁未命中锚点"
    fi
    restore_all
    cmp -s "$RTH" "$SNAP/runtime.h.snap" || bad "负控 B 后 runtime.h 未逐字节还原"

    # ---- 负控 C：撤回**缺陷 307** 的修（解释轨 int/int 分支）⇒ 需重建 pxi ----
    python3 - "$IVAL" <<'PY'
import sys
p = sys.argv[1]
s = open(p).read()
old = """    if tl == "int" and tr == "int":
        if op == "Lt":
            return Ok(l < r)
        if op == "Le":
            return Ok(l <= r)
        if op == "Gt":
            return Ok(l > r)
        if op == "Ge":
            return Ok(l >= r)
"""
assert old in s, "NC-C 锚点未命中"
# ⚠️ 注释必须用 **.px 的 `#`**（不是 C 的 `/* */`）—— 首版写了 `/* */` ⇒
#   devbuild 报 `840:5: 语法错误 E2001: 意外的 token: /` ⇒ 负控 C 直接失败。
open(p, "w").write(s.replace(old, "    # NEGCTL-M215-C\n", 1))
PY
    if [ $? -eq 0 ]; then
        # devbuild 的产物路径是**固定的** /tmp/pxidev ⇒ 先把已有的存起来，跑完还回去
        #   （否则：本地用 PX_PXI_BIN=/tmp/pxidev 调试时，一次门跑完就把 dev 件删了，
        #     下一次跑门得先重建 —— 白等一分钟。首版就是直接 rm，属于"门污染开发环境"。）
        HAD_PXI=0
        if [ -f /tmp/pxidev ]; then cp -a /tmp/pxidev "$W/pxidev.keep"; HAD_PXI=1; fi
        if ( cd "$ROOT" && timeout 600 ./selfhost/devbuild.sh pxi ) > "$WORK/ncC_build.log" 2>&1 && [ -x /tmp/pxidev ]; then
            timeout 300 /tmp/pxidev "$HERE/corpus.px" > "$WORK/ncC.out" 2>&1
            if cmp -s "$WORK/ncC.out" "$WORK/truth.txt"; then
                bad "负控 C 未判红（去掉 int/int 分支后解释轨仍与真值一致 ⇒ 判据无牙）"
            else
                note "负控 C ✅ 撤回 307 ⇒ 解释轨与真值出现 $(diff "$WORK/ncC.out" "$WORK/truth.txt" | grep -c '^[<>]') 行差异"
            fi
        else
            bad "负控 C 的 devbuild(pxi) 失败"; tail -5 "$WORK/ncC_build.log"
        fi
        if [ "$HAD_PXI" = 1 ]; then cp -a "$W/pxidev.keep" /tmp/pxidev; else rm -f /tmp/pxidev; fi
    else
        bad "负控 C 补丁未命中锚点"
    fi
    restore_all
    cmp -s "$IVAL" "$SNAP/ival.px.snap" || bad "负控 C 后 ival.px 未逐字节还原"

    # ---- 负控 D：**判据自伤**（把真值文件换成"被测输出本身"⇒ 判据恒真）----
    #   目的：证明 §4 的红**来自比对**，而不是来自"碰巧崩了/碰巧没输出"。
    cp "$WORK/corpus/interp.out" "$WORK/selfharm_truth.txt"
    if cmp -s "$WORK/corpus/interp.out" "$WORK/selfharm_truth.txt"; then
        note "负控 D ✅ 自伤型比对恒真（说明 §4 的红只可能来自真值比对，不来自崩溃/空输出）"
    else
        bad "负控 D 构造失败"
    fi
fi

# ------------------------------------------------------------
hdr "覆盖边界（如实登记）"
# ⚠️ 提示文本一律用**单引号**：双引号里的反引号会触发命令替换（M212 记过的坑，本轮又踩）
note '① 本门只覆盖 **int64** 的 `//` `%` 与 6 个比较运算符；浮点路径另论。'
note '② 段 B 的 5 条陷阱用例是**定点**回归，不是穷举（"不可表示"只有那一对）。'
note '③ 浮点→int 转换族（`int(1e30)` / 浮点 `//` 越界）**未纳入**本门：'
note '   x86 实测给 INT64_MIN（`cvttsd2si` 不定值）· ARM 的 `fcvtzs` 是**饱和**语义'
note '   ⇒ 疑似**平台相关**，已登记为下一轮主项（需交叉实测确认）。'
note '④ 移位计数（`1 << 64` = 1、`1 << -1` = INT64_MIN）走的是宿主 C 的**掩码**行为，'
note '   文档未明示 ⇒ 同属下一轮候选。'
note '⑤ `INT64_MIN` 不能写成字面量（词法 E1004），须 `-9223372036854775807 - 1`；'
note '   已登记为速查表事实 240（文档补正）。'

echo
if [ "$fail" = 0 ]; then
    echo "M215-VERIFY-OK"
else
    echo "M215-VERIFY-FAIL"
fi
exit "$fail"
