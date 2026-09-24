#!/usr/bin/env bash
# ============================================================
# M204 门（第 83 轮）：**复合赋值**在 `Index` / `Field` 目标上的三轨同一真相
#   · 缺陷 245：C 轨**静默丢弃运算符**（`m[0] += 10` 实测 `[10,2,3]`、`p.x += 10` 实测 `10`）
#   · 缺陷 246：复合赋值到**不存在的成员**时解释轨与编译两轨不同码不同文（本轮实测照出）
# ------------------------------------------------------------
# 权威口径 = docs/PUXIAN_CHEATSHEET.md 事实 233 · docs/ERROR_CODES.md §6.15
#
# 缺陷 245 的形状（**高严重度 · 静默错值**）：
#   `selfhost/cg_stmt.px` 在 `Index` / `Field` 目标上**无条件**发射
#   `px_index_set(o, i, rhs)` / `px_field_set(o, "f", rhs)` —— `op`（`+=` / `-=` / …）
#   **被静默丢弃**，等价于"直接赋值"。（`Var` 目标那一支走 `cg_assign_op_local`，是对的。）
#   实测（同一份源码）：
#     `let m=[1,2,3]; m[0] += 10`   解释/VM `[11,2,3]`  **C `[10,2,3]`**
#     `let q=[5,6];    q[1] -= 2`   解释/VM `[5,4]`     **C `[5,2]`**
#     `let d={"k":5};  d["k"] += 1` 解释/VM `{k:6}`     **C `{k:1}`**
#     `struct P: x,y ; p.x += 10`   解释/VM `11`        **C `10`**
#     `l[0] = l[0] + 100`          三轨都对 ⇒ **只坏在"复合"这一条路**
#   危害：计数器 / 累加器 / 词频统计这类代码在 C 轨**全错且难察**。
#
# 修法：op != "Assign" 时把目标对象/键**先落 `_sN` 临时**（各求值**恰一次**），
#   旧值 = `px_index(sN…, sM)` / `px_field(sN, "f")`，新值 = `<op>(旧值, rhs)`，
#   再 set；**顺序守 M165**（词法左→右：目标对象 → 键 → 右值）。
#   `_sN` 用**专用计数器** `cg_seq_new()`（不占 `cg_tmp()` 的 `_tN` 序号 ⇒
#   其余发射文本一字不变，冻结门差异面可控 —— M165 同款纪律）。
#
# 判据：
#   [1] 静态：两条复合路径都在位（且都用 `cg_seq_new()`）· 解释轨 246 的两处 R1008 守卫 ·
#            文档事实 233。
#   [2] 动态正例 2 组 × 三轨（解释 / VM / C）**逐字节一致**：13 种复合运算符 × list/dict/struct ·
#            嵌套 · **求值次数与顺序**（带副作用的 目标对象 / 键 / 右值）。
#   [3] 动态拒绝侧 3 例 × 三轨：rc≠0 · **同码** · **同文案**（缺键 / 缺字段 / 越界）。
#   [4] 负控 5 道（各自独立判红 · 源逐字节还原 · 锚点找不到即 FAIL）：
#            A Index 复合退回旧形态 ⇒ **只 C 轨**错（解释/VM 仍对）
#            B Field 复合退回旧形态 ⇒ 同上
#            C **顺序违规**（右值提前求值）⇒ pos_assign_eval 的 `a02` 变 `[v, o, k]`
#            D **重复求值**（不落临时）⇒ `a02` 变 `[o, o, k, k, v]`
#            E 解释轨 246 退回（缺失成员拿 null 硬算）⇒ rej_missing_key 判红
#   注入方式：A–D 改 `cg_stmt.px` ⇒ **重编 C 轨编译器**（devbuild → /tmp/pxcdev）后用
#     `PX_PXC_BIN` 单独跑 C 轨（改动只在 C 轨可见 ⇒ 必须这么注入，否则跑的还是入库件 = 假绿）；
#     E 改 `istmt.px` ⇒ 重编 dev 解释器（同 m202/m203 的做法）。
# 用法：bash examples/m204_assign_op/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
for a in "$@"; do case "$a" in --neg-skip) NEG=0 ;; esac; done

W=/tmp/m204_gate; BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(selfhost/cg_stmt.px selfhost/cg_expr.px selfhost/istmt.px)
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
chkout() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; cat "$W/$3" 2>/dev/null | sed 's/^/      | /' | head -14; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

pre() { grep -qF "$1" "$2" || { echo "❌ 前置自查失败：$2 缺「$1」" >&2; exit 2; }; }
pre 'let to = cg_seq_new()' selfhost/cg_stmt.px
pre 'def cg_seq_new():' selfhost/cg_expr.px
pre 'return Err(i_r1008("结构体没有字段 '"'"'" + fname + "'"'"'", pos))' selfhost/istmt.px

norm_msg() {   # $1=输出文件 $2=R 码 → 「R 码之后、剥掉 行:列 前缀」的消息体（同 m203 口径）
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
norm_out() { sed '/^$/d' "$1" 2>/dev/null; }

PXI_BIN="$ROOT/bootstrap/pxi"
build3() {   # $1=probe 名 → $W/$1.{interp,vm,c}.{out,rc}（用**入库件**）
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
build_c_dev() {   # $1=probe 名 → $W/$1.cdev.{out,rc}（用 **/tmp/pxcdev** = 被打桩的 C 轨编译器）
    local n="$1"
    local d="$W/cd_$n"
    rm -rf "$d"; mkdir -p "$d"; cp -f "$HERE/probe/$n.px" "$d/"
    ( cd "$d" && PX_PXC_BIN=/tmp/pxcdev PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.cdev.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 120 "$d/build/$n" ) > "$W/$n.cdev.out" 2>&1; echo $? > "$W/$n.cdev.rc"
    else echo 999 > "$W/$n.cdev.rc"; { echo "(C 轨 dev 构建失败)"; tail -5 "$W/$n.cdev.build"; } > "$W/$n.cdev.out"; fi
    rm -rf "$d/build"
}
judge3() {   # $1=probe $2=期望 stdout
    local n="$1" want="$2" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" = 0 ] || return 1
        [ "$(norm_out "$W/$n.$t.out")" = "$want" ] || return 1
    done
    return 0
}
judge_rej() {   # 三轨 rc≠0 + before + 同码 + **整条消息体逐字相同**
    local n="$1" code="$2" msg="$3" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" != 0 ] || return 1
        grep -q '^before$' "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$code" "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$msg" "$W/$n.$t.out" 2>/dev/null || return 1
    done
    local a b c
    a="$(norm_msg "$W/$n.interp.out" "$code")"
    b="$(norm_msg "$W/$n.vm.out" "$code")"
    c="$(norm_msg "$W/$n.c.out" "$code")"
    [ -n "$a" ] && [ "$a" = "$b" ] && [ "$a" = "$c" ]
}
stub() { python3 "$HERE/stubs.py" "$1" "$ROOT" > "$W/stub_$1.log" 2>&1; }

echo "=== [1] 静态：两条复合路径 / cg_seq_new / 解释轨 246 守卫 / 文档 ==="
chk "[1] Index 复合路径在位（落 _sN + 旧值 + <op>）" \
    "grep -qF 'px_index_set(\" + to + \", \" + ti + \", \" + full + \")' selfhost/cg_stmt.px && grep -qF 'px_index(\" + to + \", \" + ti + \")' selfhost/cg_stmt.px"
chk "[1] Field 复合路径在位" \
    "grep -qF 'px_field_set(\" + to + \", \\\"\" + fname + \"\\\", \" + full + \")' selfhost/cg_stmt.px && grep -qF 'px_field(\" + to + \", \\\"\" + fname + \"\\\")' selfhost/cg_stmt.px"
chk "[1] 临时名走**专用计数器** cg_seq_new()（不占 _tN 序号 ⇒ 其余发射不变）" \
    "grep -qF 'def cg_seq_new():' selfhost/cg_expr.px && [ \"\$(grep -cF 'cg_seq_new()' selfhost/cg_stmt.px)\" -ge 3 ]"
chk "[1] 解释轨缺陷 246 的两处 R1008 守卫在位（缺键 / 缺字段）" \
    "grep -qF 'i_r1008(\"字典没有键 '\\''\" + iv + \"'\\''\", pos)' selfhost/istmt.px && grep -qF 'i_r1008(\"结构体没有字段 '\\''\" + fname + \"'\\''\", pos)' selfhost/istmt.px"
chk "[1] Var 目标那一支**未被改动**（cg_assign_op_local 仍在）" \
    "grep -qF 'let full = cg_assign_op_local(op, cg_load_ck(name, v), rhs)' selfhost/cg_stmt.px"
chk "[1] 速查表事实 233 在位" "grep -q '233. \*\*复合赋值' docs/PUXIAN_CHEATSHEET.md"

echo "=== [2] 动态正例 2 组 × 三轨（解释 / VM / C）逐字节一致 ==="
cat > "$W/pos_assign_op.want" <<'EOF2'
i01=[11, 2, 3]
i02=[11, 1, 3]
i03=[11, 1, 9]
i04=[5, 1, 9]
i05=[1, 1, 9]
i06=[1, 1, 9]
i07=[0, 1, 9]
i08=[9, 1, 9]
i09=[10, 1, 9]
i10=[40, 1, 9]
i11=[20, 1, 9]
d01={j: 1, k: 6}
d02={j: 7, k: 6}
f01=11
f02=1
f03=33
v01=8
v02=16
x01=[101, 2]
n01=[[1, 2], [13, 4]]
EOF2
cat > "$W/pos_assign_eval.want" <<'EOF2'
a01=[11, 2]
a02=[o, k, v]
a03={f: 6}
a04=[o]
a05=[o, o]
EOF2
for n in pos_assign_op pos_assign_eval; do
    build3 "$n"
    case "$n" in
        pos_assign_op)   d="12 种复合运算符 × list/dict/struct + 嵌套 + 显式展开对照 + Var 零回归" ;;
        pos_assign_eval) d="**求值次数与顺序**（目标对象/键各恰一次 · 词法左→右 o→k→v）" ;;
    esac
    chkout "[2] $n：$d" "judge3 $n \"\$(cat $W/$n.want)\"" "$n.interp.out"
done

echo "=== [3] 动态拒绝侧 3 例 × 三轨（rc≠0 · 同码 · **同文案**）==="
cat > "$W/rej.tsv" <<'EOF2'
rej_missing_key|R1008|字典没有键 'k'
rej_field_absent|R1008|结构体没有字段 'z'
rej_index_oob|R1003|索引越界: 5 (len=1)
EOF2
while IFS='|' read -r n code msg; do
    [ -n "$n" ] || continue
    build3 "$n"
    #   ⚠️ 实参用**双引号**：消息里本身可能含单引号（`结构体没有字段 'z'`）⇒ 单引号会打破引用
    #      （首版即如此：门报「同文案」失败，而三轨其实完全一致 —— 错误信息指不到真因）。
    chkout "[3] $n：三轨 rc≠0 · 同码 $code · 同文案「$msg」" "judge_rej $n \"$code\" \"$msg\"" "$n.interp.out"
done < "$W/rej.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立判红 · 源逐字节还原 · 打桩失败即 FAIL）==="

    # A–D：改 cg_stmt.px ⇒ **重编 C 轨编译器**（改动只在 C 轨可见 ⇒ 必须这么注入）
    for L in A B C D; do
        restore_all; snapshot
        if stub "$L"; then
            if timeout 900 bash selfhost/devbuild.sh > "$W/neg$L.build" 2>&1 && [ -x /tmp/pxcdev ]; then
                build_c_dev pos_assign_op >/dev/null 2>&1
                build_c_dev pos_assign_eval >/dev/null 2>&1
                case "$L" in
                A) chk "[4A] Index 复合退回旧形态 ⇒ **C 轨**与期望不符（op 被丢弃）" \
                       "! { [ \"\$(cat $W/pos_assign_op.cdev.rc)\" = 0 ] && [ \"\$(norm_out $W/pos_assign_op.cdev.out)\" = \"\$(cat $W/pos_assign_op.want)\" ]; }"
                   chk "[4A] 且**只** C 轨错：解释轨仍与期望一致（独立牙）" \
                       "[ \"\$(norm_out $W/pos_assign_op.interp.out)\" = \"\$(cat $W/pos_assign_op.want)\" ]"
                   chk "[4A] 失败可读：C 轨输出里 i01 不是 [11, 2, 3]" "! grep -q 'i01=\[11, 2, 3\]' '$W/pos_assign_op.cdev.out'" ;;
                B) chk "[4B] Field 复合退回旧形态 ⇒ **C 轨**与期望不符" \
                       "! { [ \"\$(cat $W/pos_assign_op.cdev.rc)\" = 0 ] && [ \"\$(norm_out $W/pos_assign_op.cdev.out)\" = \"\$(cat $W/pos_assign_op.want)\" ]; }"
                   chk "[4B] 且**只** C 轨错：解释轨仍与期望一致" \
                       "[ \"\$(norm_out $W/pos_assign_op.interp.out)\" = \"\$(cat $W/pos_assign_op.want)\" ]" ;;
                C) chk "[4C] 顺序违规（右值提前求值）⇒ pos_assign_eval 的 C 轨判红" \
                       "! { [ \"\$(cat $W/pos_assign_eval.cdev.rc)\" = 0 ] && [ \"\$(norm_out $W/pos_assign_eval.cdev.out)\" = \"\$(cat $W/pos_assign_eval.want)\" ]; }"
                   chk "[4C] 失败可读：顺序变成 v 在前（实测 a02 非 [o, k, v]）" "! grep -q 'a02=\[o, k, v\]' '$W/pos_assign_eval.cdev.out'" ;;
                D) chk "[4D] 重复求值（不落临时）⇒ pos_assign_eval 的 C 轨判红" \
                       "! { [ \"\$(cat $W/pos_assign_eval.cdev.rc)\" = 0 ] && [ \"\$(norm_out $W/pos_assign_eval.cdev.out)\" = \"\$(cat $W/pos_assign_eval.want)\" ]; }"
                   chk "[4D] 失败可读：目标对象被求值两次（实测 a02 非 [o, k, v]）" "! grep -q 'a02=\[o, k, v\]' '$W/pos_assign_eval.cdev.out'" ;;
                esac
            else
                echo "  FAIL [4$L] dev C 轨编译器构建失败"; tail -3 "$W/neg$L.build" | sed 's/^/      | /'; fail=$((fail+1))
            fi
        else
            echo "  FAIL [4$L] 打桩失败"; cat "$W/stub_$L.log" | sed 's/^/      | /'; fail=$((fail+1))
        fi
    done

    # E：改 istmt.px ⇒ 重编 dev 解释器
    restore_all; snapshot
    if stub E; then
        if timeout 900 bash selfhost/devbuild.sh pxi > "$W/negE.build" 2>&1 && [ -x /tmp/pxidev ]; then
            cp -f /tmp/pxidev "$W/pxidev_negE"
            ( cd "$W/b_rej_missing_key" && timeout 120 "$W/pxidev_negE" rej_missing_key.px ) > "$W/rej_missing_key.interp.out" 2>&1
            echo $? > "$W/rej_missing_key.interp.rc"
            chk "[4E] 解释轨 246 退回 ⇒ rej_missing_key 判红（缺键不再报 R1008）" "! judge_rej rej_missing_key R1008 字典没有键"
            chk "[4E] 失败可读：解释轨回到「无法相加: null + int」" "grep -qF '无法相加' '$W/rej_missing_key.interp.out'"
        else
            echo "  FAIL [4E] dev 解释器构建失败"; tail -3 "$W/negE.build" | sed 's/^/      | /'; fail=$((fail+1))
        fi
    else
        echo "  FAIL [4E] 打桩失败"; cat "$W/stub_E.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    restore_all
fi

echo "── 结果 ──"
echo "  通过 $pass · 失败 $fail"
[ "$fail" = 0 ] && { echo "M204-ASSIGN-OP-VERIFY-OK"; exit 0; }
echo "M204-ASSIGN-OP-VERIFY-FAIL"; exit 1
