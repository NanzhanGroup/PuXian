#!/usr/bin/env bash
# ============================================================
# M194 门（第 72 轮）：native 参数**类型守卫完备性**（缺陷 222）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6（M194 建立）。
# 主问题：`R1002` 只保证"错了会说"，不保证"错了一定会被说" —— 26 个站点「形参 ≥2、
#   却只查了个数或只查了首参」，其余实参直接进 `int_val()` / `val_cstr()` / `.as.i`：
#     · `int_val` 对 float **静默截断** ⇒ `read_at(p, 1.5, 3)` == `read_at(p, 1, 3)`；
#       `.as.i`（h3 族）更糟 —— float 的**位模式**被当整数读（1.5 ⇒ 4609434218613702656）；
#     · `val_cstr` 对 int/容器 **静默串化** ⇒ `s3_get(1,2,3,4,5)` 变成对 endpoint "1" 的请求。
#   三轨**一致**（共用同一份 C native）⇒ 用户拿到的是**静默错值**，不是分叉 ——
#   所以 M190 的「三轨 rc 分叉」普查看不见它。
# 统一文案：`R1002: <函数> 的 <参数> 需要<类型>，实际是 <t>`（M179/M189 同族）。
# 判据：
#   [1] 静态：scan_errcodes.py 的 [S5] 参数守卫完备性 —— 140 个多形参守卫、缺检查 0
#   [2] 动态：12 个错例 × 三轨（解释 / VM / C）—— rc≠0 + **同码 R1002** + **同文**
#   [2b] 合法侧：8 例 rc=0（证明收口没把正常路径改坏；含 `null` = 未提供的既有写法）
#   [3] 负控 4 道（各自独立判红、源逐字节还原）：
#       A 静态 · 去掉一处 INT 检查（read_at 偏移）· B 静态 · 去掉一处 STR 检查（s3_get endpoint）
#       C 动态 · `px_arg_int` 改成不检查（⇒ 静默，同码判据失效）　D 动态 · 改码 R1002→R1007
#       ⚠️ C/D 只判 **C 轨**：解释/VM 轨跑的是预编译入库件（`bootstrap/pxi`），
#          改 runtime.c 后**必须重烘**才生效（M193 教训）—— 门不重烘，故只用 C 轨当判据。
# 用法：bash examples/m194_arg_guards/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m194_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c runtime/runtime.h runtime/runtime_ws.c runtime/runtime_h3.c runtime/runtime_quic.c)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(basename "$f")"; done; }
restore_all() { for f in "${FILES[@]}"; do [ -f "$BACK/$(basename "$f")" ] && cp -f "$BACK/$(basename "$f")" "$f"; done; return 0; }
# 纪律（M190/M191/M192/M193 教训）：每道负控前**先 restore 再 snapshot**（干净起点）；trap 只还原
snapshot
trap 'restore_all' EXIT

# ── 前置不变量：本轮改动必须在位（缺一即门坏 / 源码被别的门盖回去）──
for pat in 'int64_t px_arg_int(LXValue v, const char* fn, const char* pname)' \
           'const char* px_arg_str(LXValue v, const char* fn, const char* pname)' \
           'px_arg_int(args[1], "read_at", "偏移")' \
           'px_arg_int(args[0], "int_to_hex", "n")' \
           'px_arg_int(args[2], "bytes_set", "value")' \
           'px_arg_str(args[0], "s3_get", "endpoint")' \
           'px_arg_str(args[0], "write_bytes", "路径")' \
           'px_arg_str(args[1], "http_request", "method")' \
           'px_arg_str(args[0], "tls_connect", "host")' \
           'px_arg_int(args[1], "h3_serve_read_request", "timeout_ms")' \
           'px_arg_str(args[2], "int_to_bytes", "endian")' \
           'http_request 的 body 需要字符串或 bytes' \
           'px_serve 的 opts 需要字典' \
           'ws_serve 的 opts 需要字典' \
           'time_format 的 ts 需要整数'; do
    grep -qF "$pat" runtime/*.c || { echo "❌ 前置自查失败：runtime 缺「$pat」（源码被还原/被别的门盖掉？）" >&2; exit 2; }
done
grep -qF 'px_arg_int(LXValue v, const char* fn, const char* pname);' runtime/runtime.h \
    || { echo "❌ 前置自查失败：runtime.h 缺 px_arg_* 声明（各模块不可见）" >&2; exit 2; }

norm() {   # $1=输出文件 → "CODE|正文"
    grep -E '^运行时错误' "$1" 2>/dev/null | head -1 \
      | sed -E -e 's/^运行时错误: 错误 \[(R[0-9]{4})\] [0-9]+:[0-9]+: /\1|/' \
               -e 's/^运行时错误 \[[^]]*行[0-9]+\]: (R[0-9]{4}): /\1|/'
}

run3() {   # run3 <用例名>
    local b="$1" d="$W/d_$b"
    rm -rf "$d"; mkdir -p "$d"
    cp "$HERE/probe/$b.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$b.interp.out" 2>&1; echo $? > "$W/$b.interp.rc"
    rm -rf "$d/build"
    timeout 200 ./tools/px build "$d/$b.px" > "$W/$b.vm.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.vm.out" 2>&1; echo $? > "$W/$b.vm.rc"
    else echo 999 > "$W/$b.vm.rc"; tail -3 "$W/$b.vm.build.log" > "$W/$b.vm.out"; fi
    rm -rf "$d/build"
    PX_BUILD_ENGINE=c timeout 260 ./tools/px build "$d/$b.px" > "$W/$b.c.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.c.out" 2>&1; echo $? > "$W/$b.c.rc"
    else echo 999 > "$W/$b.c.rc"; tail -3 "$W/$b.c.build.log" > "$W/$b.c.out"; fi
    rm -rf "$d/build"
}
run1c() {  # 只跑 C 轨（负控 C/D 用 —— runtime.c 改动只有 C 轨会立刻生效）
    local b="$1" d="$W/nc_$b"
    rm -rf "$d"; mkdir -p "$d"; cp "$HERE/probe/$b.px" "$d/"
    PX_BUILD_ENGINE=c timeout 260 ./tools/px build "$d/$b.px" > "$W/nc_$b.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/nc_$b.c.out" 2>&1; echo $? > "$W/nc_$b.c.rc"
    else echo 999 > "$W/nc_$b.c.rc"; tail -3 "$W/nc_$b.build.log" > "$W/nc_$b.c.out"; fi
    rm -rf "$d/build"
}
judge_case() {  # judge_case <名> <期望码> <期望正文> —— 0=通过
    local b="$1" ec="$2" eb="$3" t ok=1
    for t in interp vm c; do
        [ "$(cat "$W/$b.$t.rc")" != 0 ] || ok=0
        [ "$(norm "$W/$b.$t.out")" = "$ec|$eb" ] || { ok=0; [ -n "${VG:-}" ] && echo "    [$t] got: $(norm "$W/$b.$t.out")"; }
        grep -q '^before$' "$W/$b.$t.out" || ok=0    # 必须已进运行期（防编译期报错冒充）
    done
    [ $ok = 1 ]
}

echo "=== [1] 静态判据：参数类型守卫完备性（§6）==="
static_ok() { python3 "$ROOT/examples/m191_error_codes/scan_errcodes.py" --root "$ROOT" > "$W/static.log" 2>&1; }
static_ok; rc=$?
chk "[1] 扫描器六查全绿" "[ \$rc = 0 ]"
chk "[1] 判据 ⑥ 真跑过（日志有 [S5] 段）" "grep -q '\[S5\] native 参数类型守卫完备性' '$W/static.log'"
# ⚠️ M195（第 73 轮）后此数由 140 → **137**：h3/quic listen 族的**元数守卫消息**改成
#   「需要 (port) 或 (port, cert, key) 参数」（同时表达 1 参/3 参两种合法形态）——
#   扫描器只取第一个 `)` 之前的形参 ⇒ 这些站点不再计入「多形参守卫」。
#   站点本身仍在（由 [S6] 全文覆盖），只是**计数口径**变了 ⇒ 判据跟着更新，不是放宽。
chk "[1] 多形参守卫合计 137、缺类型检查 0" "grep -q '多形参守卫合计 137 · 缺类型检查 \*\*0\*\*' '$W/static.log'"
chk "[1] 无任何文件残留「缺检查」非零" "! grep -qE '缺检查 [1-9]' '$W/static.log'"

echo "=== [2] 动态判据：12 个错例 × 三轨（同码 + 同文）==="
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[2] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"

echo "=== [2b] 合法侧：收口不得改坏正常路径（含 null = 未提供）==="
while IFS=$'\t' read -r b want; do
    [ -n "$b" ] || continue
    run3 "$b"
    for t in interp vm c; do
        chk "[2b] $b ($t)：rc=0 且输出含「$want」" \
            "[ \"\$(cat '$W/$b.$t.rc')\" = 0 ] && grep -q '$want' '$W/$b.$t.out'"
    done
done < "$HERE/probe/EXPECT_OK.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立判红 · 源逐字节还原）==="

    # A 静态：去掉一处 INT 检查（read_at 偏移）⇒ [S5] 必须判红
    restore_all; snapshot
    sed -i 's|px_arg_int(args\[1\], "read_at", "偏移")|int_val(args[1])|' runtime/runtime.c
    static_ok
    chk "[3A] 去掉 read_at 偏移的检查 ⇒ [S5] 判红" "[ \$? != 0 ] && grep -q 'read_at' '$W/static.log' && grep -qE '缺检查 [1-9]' '$W/static.log'"

    # B 静态：去掉一处 STR 检查（s3_get endpoint）⇒ [S5] 必须判红
    restore_all; snapshot
    sed -i 's|px_arg_str(args\[0\], "s3_get", "endpoint")|val_cstr(args[0])|' runtime/runtime.c
    static_ok
    chk "[3B] 去掉 s3_get endpoint 的检查 ⇒ [S5] 判红" "[ \$? != 0 ] && grep -q 's3_get' '$W/static.log' && grep -qE '缺检查 [1-9]' '$W/static.log'"

    # C 动态：`px_arg_int` 退回**旧的 `int_val` 语义**（float 静默截断）⇒ e1 必须**静默**
    #   （rc=0 且读到的仍是 offset 1 ⇒ "ong"），于是 [2] 的「rc≠0 + 同码」判据由绿转红。
    #   ⚠️ 不能只写 `return v.as.i;` —— 那是**位模式**语义（偏移变成天文数字 ⇒ 报 io: 随机读失败，
    #      仍然响亮，负控就"没有牙"了）。必须复刻 `int_val` 的**截断**语义才是真正的静默复发。
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
p = sys.argv[1] + "/runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = """int64_t px_arg_int(LXValue v, const char* fn, const char* pname) {
    if (v.type != PX_INT)
        px_error("R1002: %s 的 %s 需要整数，实际是 %s", fn, pname, px_type_name(v));
    return v.as.i;
}"""
new = """int64_t px_arg_int(LXValue v, const char* fn, const char* pname) {
    (void)fn; (void)pname;
    /* 负控 C：退回 int_val 的**截断**语义（float 静默取整） */
    if (v.type == PX_INT) return v.as.i;
    if (v.type == PX_FLOAT) return (int64_t)v.as.f;
    return 0;
}"""
assert s.count(old) == 1, "负控 C 锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PYEOF
    run1c e1_read_at_float
    chk "[3C] px_arg_int 退回截断语义 ⇒ e1 C 轨静默（rc=0 且判据失效）" \
        "[ \"\$(cat '$W/nc_e1_read_at_float.c.rc')\" = 0 ] && grep -q 'ong' '$W/nc_e1_read_at_float.c.out'"

    # D 动态：改码 R1002 → R1007 ⇒ 仍然响亮但**码不符** ⇒ 同码判据必须失效
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
p = sys.argv[1] + "/runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = """        px_error("R1002: %s 的 %s 需要整数，实际是 %s", fn, pname, px_type_name(v));"""
new = """        px_error("R1007: %s 的 %s 需要整数，实际是 %s", fn, pname, px_type_name(v));"""
assert s.count(old) == 1, "负控 D 锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PYEOF
    run1c e1_read_at_float
    chk "[3D] 改码 R1002→R1007 ⇒ 同码判据（\$ec|…）失效" \
        "[ \"\$(cat '$W/nc_e1_read_at_float.c.rc')\" != 0 ] && [ \"\$(norm '$W/nc_e1_read_at_float.c.out')\" != 'R1002|read_at 的 偏移 需要整数，实际是 float' ] && grep -q 'R1007' '$W/nc_e1_read_at_float.c.out'"

    restore_all
fi

echo ""
echo "── 汇总：通过 $pass · 失败 $fail ──"
if [ "$fail" = 0 ]; then echo "M194-VERIFY-OK"; exit 0; else echo "M194-VERIFY-FAIL"; exit 1; fi
