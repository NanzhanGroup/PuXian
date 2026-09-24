#!/usr/bin/env bash
# ============================================================
# M203 门（第 82 轮）：**「此类型不支持X / 不可迭代」族的三轨同码同文**（缺陷 243-a/b/c）
# ------------------------------------------------------------
# 权威口径 = docs/PUXIAN_CHEATSHEET.md 事实 232 · docs/ERROR_CODES.md §6.14
#
# 病（同一族四个站点，M191 只收口了「切片」那一个 —— 本轮补上其余三个）：
#   | 站点 | 解释轨（修前） | 编译两轨（修前） |
#   |---|---|---|
#   | 切片 | `此类型不支持切片: <t>` ✅ M191 | `此类型不支持切片: <t>` |
#   | **索引** | `此类型不支持索引`（**缺类型**） | `此类型不支持索引: <t>` |
#   | **迭代** | `此类型不可迭代`（缺类型） | **`len 不支持类型 <t>`** ← 借了用户没写过的 `len` |
#   | **索引赋值** | `索引赋值目标不支持`（**整个词条都不同**） | `此类型不支持索引赋值: <t>` |
#   三轨两/三个答案 ⇒ 按「码 + 文案」做匹配或断言的一方必然漏。
#
# 统一口径（三轨一条真相）：
#   · 不可索引：`R1002 此类型不支持索引: <t>`
#   · 不可迭代：`R1002 此类型不可迭代: <t>`（**编译两轨不再借 `px_len`** —— 新增
#     `px_iter_prepare`，三轨共用同一函数；VM 轨经内部全局 `__iter_len` 调用它）
#   · 不可索引赋值：`R1002 此类型不支持索引赋值: <t>`
#   为什么 `此类型不可迭代` 不叫「此类型不支持索引」：迭代与索引在 M164 已明确分开
#   （`px_iter_at` ⇄ `px_index`）——拿索引的词条去描述迭代，正是 M164 想根治的那类混淆。
#
# 判据：
#   [1] 静态：四个站点到位（三处解释轨 + `px_iter_prepare` 实现/声明/注册）·
#            **两条发射路径都接了新入口**（C 轨 `px_iter_prepare(` · VM 轨 `__iter_len`）·
#            `__` 内部名**不进公开名册**（tools/gen_native_table.sh 的过滤 + 名册成员核对）。
#   [2] 动态正例 2 组 × 三轨（解释 / VM / C）**逐字节一致**（迭代 7 种可迭代类型 ·
#            推导式两条发射点 · 函数内/闭包内/嵌套/空迭代 · 索引 5 型 · 索引赋值 3 型）。
#   [3] 动态拒绝侧 8 例 × 三轨：rc≠0 · **同码** · **同文案** · 前置标记在。
#   [4] 负控 5 道（各自独立判红 · 源逐字节还原 · 锚点找不到即 FAIL）：
#            A/B/C 三个解释轨站点各自退回旧文案（**重编 dev 解释器**后跑）　
#            D `px_iter_prepare` 的文案退回「借 len」⇒ **VM 与 C 两轨都要红**（= 两条发射路径真接了）
#            E `px_iter_prepare` 对 int 放行（静默 0 次迭代）⇒ 响亮退回静默
# 用法：bash examples/m203_iter_index_msg/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
for a in "$@"; do case "$a" in --neg-skip) NEG=0 ;; esac; done

W=/tmp/m203_gate; BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c runtime/runtime.h selfhost/ival.px selfhost/istmt.px)
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
chkout() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; cat "$W/$3" 2>/dev/null | sed 's/^/      | /' | head -12; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

pre() { grep -qF "$1" "$2" || { echo "❌ 前置自查失败：$2 缺「$1」" >&2; exit 2; }; }
pre 'return Err(i_r1002("此类型不支持索引: " + t, pos))' selfhost/ival.px
pre 'return Err(i_r1002("此类型不可迭代: " + t, pos))' selfhost/ival.px
pre 'return Err(i_r1002("此类型不支持索引赋值: " + t, pos))' selfhost/istmt.px
pre 'int px_iter_prepare(LXValue v) {' runtime/runtime.c
pre 'px_set_global("__iter_len", px_native("__iter_len", bi_iter_len));' runtime/runtime.c

PXI_BIN="$ROOT/bootstrap/pxi"
build3() {
    local n="$1"
    local d="$W/b_$n"
    rm -rf "$d"; mkdir -p "$d"; cp -f "$HERE/probe/$n.px" "$d/"
    ( cd "$d" && timeout 120 "$PXI_BIN" "$n.px" ) > "$W/$n.interp.out" 2>&1; echo $? > "$W/$n.interp.rc"
    ( cd "$d" && timeout 900 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.vm.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 120 "$d/build/$n" ) > "$W/$n.vm.out" 2>&1; echo $? > "$W/$n.vm.rc"
    else echo 999 > "$W/$n.vm.rc"; { echo "(VM 轨构建失败)"; tail -5 "$W/$n.vm.build"; } > "$W/$n.vm.out"; fi
    rm -rf "$d/build"
    ( cd "$d" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.c.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 120 "$d/build/$n" ) > "$W/$n.c.out" 2>&1; echo $? > "$W/$n.c.rc"
    else echo 999 > "$W/$n.c.rc"; { echo "(C 轨构建失败)"; tail -5 "$W/$n.c.build"; } > "$W/$n.c.out"; fi
    rm -rf "$d/build"
}
norm_out() { sed '/^$/d' "$1" 2>/dev/null; }
judge3() {
    local n="$1" want="$2" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" = 0 ] || return 1
        [ "$(norm_out "$W/$n.$t.out")" = "$want" ] || return 1
    done
    return 0
}
norm_msg() {   # $1=输出文件 $2=R 码 → 「R 码之后、剥掉 行:列 前缀」的消息体（第一条）
    #   ⚠️ 三轨的错误行**格式不同**：解释轨 `… 错误 [R1002] 3:1: <msg>`（方括号，行:列在码后）·
    #      编译两轨 `… [行3]: R1002: <msg>`。用一个 sed 同时切两种形态会**必有一边为空**
    #      （本门第一版即如此 ⇒ 8 条全假红）。这里显式归一化。
    python3 - "$1" "$2" <<'PYEOF'
import re, sys
txt = open(sys.argv[1], encoding='utf-8', errors='replace').read()
code = sys.argv[2]
for ln in txt.splitlines():
    if code not in ln:
        continue
    rest = ln.split(code, 1)[1]
    rest = rest.lstrip(']: ').strip()
    rest = re.sub(r'^\d+:\d+:\s*', '', rest)
    print(rest)
    break
PYEOF
}
judge_rej() {   # 三轨 rc≠0 + 前置标记 + 同码 + **同文案**（逐轨逐条都要在）
    local n="$1" code="$2" msg="$3" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" != 0 ] || return 1
        grep -q '^before$' "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$code" "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$msg" "$W/$n.$t.out" 2>/dev/null || return 1
    done
    # 三轨文案必须**逐字相同**（不只是都含子串）—— 取「错误行去掉前缀」后比较
    #   三轨前缀格式不同 ⇒ 归一化后**逐字比较整条消息体**（不是「都含子串」）
    local a b c
    a="$(norm_msg "$W/$n.interp.out" "$code")"
    b="$(norm_msg "$W/$n.vm.out" "$code")"
    c="$(norm_msg "$W/$n.c.out" "$code")"
    [ -n "$a" ] && [ "$a" = "$b" ] && [ "$a" = "$c" ]
}
stub() { python3 "$HERE/stubs.py" "$1" "$ROOT" > "$W/stub_$1.log" 2>&1; }

echo "=== [1] 静态：四个站点 / 两条发射路径都接了新入口 ==="
chk "[1] 解释轨三处文案（索引 / 迭代 / 索引赋值）都带**实际类型**" \
    "grep -qF 'i_r1002(\"此类型不支持索引: \" + t, pos)' selfhost/ival.px && grep -qF 'i_r1002(\"此类型不可迭代: \" + t, pos)' selfhost/ival.px && grep -qF 'i_r1002(\"此类型不支持索引赋值: \" + t, pos)' selfhost/istmt.px"
chk "[1] runtime 有 px_iter_prepare（实现 + 声明 + 注册）且文案带类型" \
    "grep -qF 'int px_iter_prepare(LXValue v) {' runtime/runtime.c && grep -qF 'int px_iter_prepare(LXValue v);' runtime/runtime.h && grep -qF 'px_error(\"R1002: 此类型不可迭代: %s\", px_type_name(v));' runtime/runtime.c"
chk "[1] C 轨**两条发射点**都走新入口（for 循环 + 推导式）" \
    "[ \"\$(grep -cF 'px_iter_prepare(' selfhost/cg_stmt.px)\" -ge 1 ] && [ \"\$(grep -cF 'px_iter_prepare(' selfhost/cg_expr.px)\" -ge 1 ]"
chk "[1] VM 轨迭代长度改走内部全局 __iter_len（不再借 len）" \
    "[ \"\$(grep -cF 'bc_g_add(g_bcm[\"globals\"], \"__iter_len\")' selfhost/bc_emit.px)\" -eq 2 ] && [ \"\$(grep -cF 'bc_g_add(g_bcm[\"globals\"], \"len\")' selfhost/bc_emit.px)\" -eq 0 ]"
chk "[1] __iter_len 是**内部名**：生成器过滤 __ 前缀 + 不在公开索引里" \
    "grep -q \"grep -v '^__'\" tools/gen_native_table.sh && ! python3 -c 'import json,sys;n=json.load(open(\"docs/native_index.json\"))[\"names\"];sys.exit(0 if \"__iter_len\" in n else 1)'"
chk "[1] 速查表事实 232 在位" \
    "grep -q '232. \*\*「此类型不支持' docs/PUXIAN_CHEATSHEET.md"

echo "=== [2] 动态正例 2 组 × 三轨（解释 / VM / C）逐字节一致 ==="
cat > "$W/pos_iter_index.want" <<'EOF2'
i01=123
i02=45
i03=中文
i04=65,66,
i05=ba
i06=24
i07=012
i08=[1, 2, 3]
i09={a: 1, b: 1}
i10=[a, b]
x01=2
x02=7
x03=c
x04=65
x05=9
a01=[1, 5, 3]
a02={k: 2}
EOF2
cat > "$W/pos_iter_ctx.want" <<'EOF2'
f01=6
f02=9
f03=[2, 3]
f04=[1, 2, 3]
f05=0
f06=[1x, 1y, 2x, 2y]
EOF2
for n in pos_iter_index pos_iter_ctx; do
    build3 "$n"
    chkout "[2] $n：三轨逐字节一致（零回归面）" "judge3 $n \"\$(cat $W/$n.want)\"" "$n.interp.out"
done

echo "=== [3] 动态拒绝侧 8 例 × 三轨（rc≠0 · 同码 · **同文案**）==="
cat > "$W/rej.tsv" <<'EOF'
rej_iter_int|R1002|此类型不可迭代: int
rej_iter_bool|R1002|此类型不可迭代: bool
rej_iter_fn|R1002|此类型不可迭代: native
rej_index_int|R1002|此类型不支持索引: int
rej_index_float|R1002|此类型不支持索引: float
rej_index_bool|R1002|此类型不支持索引: bool
rej_assign_int|R1002|此类型不支持索引赋值: int
rej_assign_bool|R1002|此类型不支持索引赋值: bool
EOF
while IFS='|' read -r n code msg; do
    [ -n "$n" ] || continue
    build3 "$n"
    chkout "[3] $n：三轨 rc≠0 · 同码 $code · 同文案「$msg」" "judge_rej $n '$code' '$msg'" "$n.interp.out"
done < "$W/rej.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立判红 · 源逐字节还原 · 打桩失败即 FAIL）==="
    neg_interp() {   # $1=桩号 $2=要红的 probe $3=想绿的 probe（独立牙）
        restore_all; snapshot
        if stub "$1"; then
            if timeout 900 bash selfhost/devbuild.sh pxi > "$W/neg$1.build" 2>&1 && [ -x /tmp/pxidev ]; then
                cp -f /tmp/pxidev "$W/pxidev_neg$1"
                for n in "$2" "$3"; do
                    ( cd "$W/b_$n" && timeout 120 "$W/pxidev_neg$1" "$n.px" ) > "$W/$n.interp.out" 2>&1
                    echo $? > "$W/$n.interp.rc"
                done
                chk "[4$1] 解释轨退回旧文案 ⇒ $2 判红" "! judge_rej $2 R1002 此类型"
                chk "[4$1] 且**只**红这一面：$3 仍绿" "judge_rej $3 R1002 此类型"
            else
                echo "  FAIL [4$1] dev 解释器构建失败"; tail -3 "$W/neg$1.build" | sed 's/^/      | /'; fail=$((fail+1))
            fi
        else
            echo "  FAIL [4$1] 打桩失败"; cat "$W/stub_$1.log" | sed 's/^/      | /'; fail=$((fail+1))
        fi
    }
    neg_interp A rej_index_int rej_iter_int
    neg_interp B rej_iter_int rej_index_int
    neg_interp C rej_assign_int rej_index_int
    PXI_BIN="$ROOT/bootstrap/pxi"

    # D：runtime 的迭代入口文案退回「借 len」⇒ **VM 与 C 两轨都要红**（= 两条发射路径真接了）
    restore_all; snapshot
    if stub D; then
        build3 rej_iter_int >/dev/null 2>&1
        chk "[4D] 迭代入口文案退回「借 len」⇒ rej_iter_int 判红" "! judge_rej rej_iter_int R1002 此类型"
        chk "[4D] **VM 轨**读到了被改的文案（⇒ VM 轨真走 px_iter_prepare）" "grep -qF 'len 不支持类型 int' '$W/rej_iter_int.vm.out'"
        chk "[4D] **C 轨**也读到了（⇒ C 轨真走 px_iter_prepare）" "grep -qF 'len 不支持类型 int' '$W/rej_iter_int.c.out'"
    else
        echo "  FAIL [4D] 打桩失败"; cat "$W/stub_D.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    # E：对 int 放行（静默 0 次迭代）⇒ 响亮退回静默
    restore_all; snapshot
    if stub E; then
        build3 rej_iter_int >/dev/null 2>&1
        #   ⚠️ 本桩只打 `px_iter_prepare`（**编译两轨**用的入口）；解释轨有自己的 `i_iter`
        #      ⇒ 断言只能要求 **VM/C 静默**（首版写成 `judge3`（三轨全绿）⇒ 门自己假红一次）。
        chk "[4E] 迭代入口对 int 放行 ⇒ **VM 轨**静默（rc=0 且只输出 before）" \
            "[ \"\$(cat '$W/rej_iter_int.vm.rc')\" = 0 ] && [ \"\$(norm_out '$W/rej_iter_int.vm.out')\" = before ]"
        chk "[4E] 同桩 ⇒ **C 轨**也静默（响亮退回静默）" \
            "[ \"\$(cat '$W/rej_iter_int.c.rc')\" = 0 ] && [ \"\$(norm_out '$W/rej_iter_int.c.out')\" = before ]"
        chk "[4E] 解释轨**不受**该桩影响（它走自己的 i_iter，仍响亮）" \
            "[ \"\$(cat '$W/rej_iter_int.interp.rc')\" != 0 ] && grep -qF '此类型不可迭代: int' '$W/rej_iter_int.interp.out'"
    else
        echo "  FAIL [4E] 打桩失败"; cat "$W/stub_E.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    restore_all
fi

echo "── 结果 ──"
echo "  通过 $pass · 失败 $fail"
[ "$fail" = 0 ] && { echo "M203-ITER-MSG-VERIFY-OK"; exit 0; }
echo "M203-ITER-MSG-VERIFY-FAIL"; exit 1
