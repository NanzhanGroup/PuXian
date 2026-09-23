#!/usr/bin/env bash
# ============================================================
# M193 门（第 71 轮）：棘轮欠账**第二批** —— 143 站点「错误码面」收口
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md（§1 谁决定失败 / §2.1 码表 / §2.3 个数分码 / §3 域前缀闭集）。
# 本轮范围（12 文件 / 143 站点，M192 之后剩余的全部欠账）：
#   quic 28 · h3 18 · ws 16 · route 9 · sqlite 9 · xml 23 · image 7 · ffi 3 ·
#   vm 25 · coro 1 · h3_qpack 2 · h3_qpack_dyn 2
# 分类（§1 判据）：
#   ① 语言层（实参类型/形状）⇒ 补 **R1002**（102 处；§2.3：函数/内置 ⇒ R1002）
#   ② 调用**形状**（缺形参）⇒ **R1005**（2 处，vm CALL 参数不足；与解释轨
#      `运行时错误: 错误 [R1005] 3:8: 调用 f 缺少参数 'b'` 同码）
#   ③ `unwrap` 失败（`!` 强制解包）⇒ **R1004**（2 处；与解释轨 `unwrap 失败: Err(x)` 同码）
#   ④ VM 内部一致性（字节码/元数据越界 —— 不该被用户触发，出现即缺陷）⇒ **R9001**（18 处）
#   ⑤ 库·环境族 ⇒ **只登记域前缀、不改文本**（18 处）：
#      `ws_serve:`（3，§3 E2）· `XML 解析错误`（13，§3 E4，与 `json:` 同族）·
#      `路由/中间件数量超出上限`（2，§3 E5）
#   ⑥ 转发点豁免（1 处）：`runtime/coro.c:804` 的 `px_error("%s", errmsg)` 透传
#      `px_native_call_capture` 的 errbuf（上游已带码/域前缀）—— 表内**必须给理由**，
#      并由判据⑤查「表 ⇔ 源码一致」+ 总数棘轮。
# 判据：
#   [1] 静态：扫描器**四查 + 转发豁免查**全绿 + 12 文件「无码 0」+ 棘轮表**已清空**
#   [2] 动态：native 各族错例 × 三轨（解释/VM/C）—— rc≠0 + **同码** + **同文** + 已进运行期
#   [2b] 域前缀侧：XML 解析失败 ⇒ **不带码**（`XML 解析错误：…`），三轨同文
#   [2c] 合法侧：各族正常调用 rc=0 —— 证明收口没把正常路径改坏
#   [3] 负控 3 道（各自独立判红、源逐字节还原）：
#       A 静态去码 · B 静态改未登记域名 · C 动态改码（R1002 → R1007）
# 用法：bash examples/m193_errcodes2/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m193_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime_quic.c runtime/runtime_h3.c runtime/runtime_ws.c runtime/runtime_route.c
       runtime/runtime_sqlite.c runtime/runtime_xml.c runtime/runtime_image.c runtime/runtime_ffi.c
       runtime/vm.c runtime/coro.c runtime/runtime_h3_qpack.c runtime/runtime_h3_qpack_dyn.c)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(basename "$f")"; done; }
restore_all() { for f in "${FILES[@]}"; do [ -f "$BACK/$(basename "$f")" ] && cp -f "$BACK/$(basename "$f")" "$f"; done; return 0; }
# 纪律（M190/M191/M192 教训）：每道负控前 **先 restore 再 snapshot**（干净起点）；trap 只还原
snapshot
trap 'restore_all' EXIT

# 前置不变量：本轮四类改动必须在位（缺一即门坏 / 源码被还原）
for pat in 'R1002: quic_listen 需要 (port: int)' 'R1002: h3_qenc 需要 (headers: list)' \
           'R1002: ws_send 需要 (conn, data) 参数' 'R1002: ffi_call 需要 (name, args_list) 参数' \
           'R1002: spawn 需要函数名' 'R1005: VM %s:%d CALL' 'R1004: force unwrap' \
           'R9001: VM %s:%d GETG 全局越界' 'R1002: %s' ; do
    grep -qF "$pat" runtime/*.c || { echo "❌ 前置自查失败：runtime 缺「$pat」（源码被还原/被别的门盖掉？）" >&2; exit 2; }
done
for pat in 'ws_serve: 绑定端口' 'XML 解析错误：缺少根元素' '路由数量超出上限' '中间件数量超出上限'; do
    grep -qF "$pat" runtime/*.c || { echo "❌ 前置自查失败：runtime 缺「$pat」（域前缀登记项）" >&2; exit 2; }
done

static_ok() { python3 "$ROOT/examples/m191_error_codes/scan_errcodes.py" --root "$ROOT" > "$W/static.log" 2>&1; }

echo "=== [1] 静态判据：带码/域前缀 · 个数分码 · 混写拆分 · 未收口棘轮 · 转发豁免 ==="
static_ok
chk "[1] 扫描器五查全绿" "[ \$? = 0 ]"
chk "[1] 前置：static_ok 真跑过（日志非空）" "[ -s '$W/static.log' ]"
for f in runtime_quic runtime_h3 runtime_ws runtime_route runtime_sqlite runtime_xml \
         runtime_image runtime_ffi vm coro runtime_h3_qpack runtime_h3_qpack_dyn; do
    chk "[1] ${f}.c 无码 0（本批已收口）" "grep -qE '${f}\\.c .*无码 0' '$W/static.log'"
done
chk "[1] 棘轮表已清空 ⇒ 无「未收口」行" "! grep -q '未收口无码站点' '$W/static.log'"
chk "[1] 转发豁免合计 1（coro.c:804）" "grep -q 'runtime/coro.c .*转发豁免 1 处' '$W/static.log'"

echo "=== [2] 动态判据：native 各族错例 × 三轨（同码 + 同文）==="
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
judge_case() {  # judge_case <名> <期望码> <期望正文> —— 0=通过
    local b="$1" ec="$2" eb="$3" t ok=1
    for t in interp vm c; do
        [ "$(cat "$W/$b.$t.rc")" != 0 ] || ok=0
        [ "$(norm "$W/$b.$t.out")" = "$ec|$eb" ] || ok=0
        grep -q '^before$' "$W/$b.$t.out" || ok=0
    done
    [ $ok = 1 ]
}
if [ -f "$HERE/probe/EXPECT.tsv" ]; then
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[2] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"
fi

echo "=== [2b] 域前缀侧：XML 解析失败 ⇒ 不带码（§3 E4），三轨同文 ==="
if [ -f "$HERE/probe/EXPECT_DOMAIN.tsv" ]; then
while IFS=$'\t' read -r b pat; do
    [ -n "$b" ] || continue
    run3 "$b"
    for t in interp vm c; do
        chk "[2b] $b ($t)：rc≠0 且无 R 码、含「$pat」" \
            "[ \"\$(cat '$W/$b.$t.rc')\" != 0 ] && grep -q '$pat' '$W/$b.$t.out' && ! norm '$W/$b.$t.out' | grep -q '^R'"
    done
done < "$HERE/probe/EXPECT_DOMAIN.tsv"
fi

echo "=== [2c] 合法侧：收口不得改坏正常路径 ==="
if [ -f "$HERE/probe/EXPECT_OK.tsv" ]; then
while IFS=$'\t' read -r b want; do
    [ -n "$b" ] || continue
    run3 "$b"
    for t in interp vm c; do
        chk "[2c] $b ($t)：rc=0 且输出含「$want」" \
            "[ \"\$(cat '$W/$b.$t.rc')\" = 0 ] && grep -q '$want' '$W/$b.$t.out'"
    done
done < "$HERE/probe/EXPECT_OK.tsv"
fi

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立；源逐字节还原）==="
    nc_static() {   # nc_static <名> <判据命令> <打桩命令>
        local name="$1" judge="$2" patch="$3"
        restore_all; snapshot
        eval "$patch" >/dev/null 2>&1 || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        if ( eval "$judge" ); then echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1));
        else echo "  PASS 负控 $name：静态判据已判红（符合预期）"; pass=$((pass+1)); fi
        restore_all
    }
    # A：去掉一个 R1002 ⇒ 语言层无码 ⇒ 闭集判红
    PAT_A='s/px_error("R1002: quic_listen 需要 (port: int)")/px_error("quic_listen 需要 (port: int)")/'
    # B：把已登记域 `XML 解析错误` 改成未登记域 `xml_syntax` ⇒ §3 闭集判红
    PAT_B='s/XML 解析错误：缺少根元素/xml_syntax: 缺少根元素/'
    # C：动态靶 —— R1002 → R1007 ⇒ 「同码」判据必须由绿转红
    PAT_C='s/px_error("R1002: h3_qenc 需要 (headers: list)")/px_error("R1007: h3_qenc 需要 (headers: list)")/'
    nc_static "A 去掉 R 码（语言层无码 ⇒ 棘轮/闭集判红）" "static_ok" "sed -i '$PAT_A' runtime/runtime_quic.c"
    nc_static "B 改未登记域名（xml_syntax 未登记 §3）" "static_ok" "sed -i '$PAT_B' runtime/runtime_xml.c"
    if [ -f "$HERE/probe/EXPECT.tsv" ] && grep -q '^e2_h3_qenc' "$HERE/probe/EXPECT.tsv"; then
        restore_all; snapshot
        sed -i "$PAT_C" runtime/runtime_h3.c
        run3 e2_h3_qenc
        if judge_case 'e2_h3_qenc' 'R1002' 'h3_qenc 需要 (headers: list)'; then
            echo "  FAIL 负控 C：动态判据仍为绿（负控没有牙）"; fail=$((fail+1))
        else
            echo "  PASS 负控 C：动态判据已判红（码被改成 R1007 ⇒ 同码判据失效）"; pass=$((pass+1))
        fi
        restore_all
    fi
fi

echo ""
echo "结果: $pass 通过 / $fail 失败"
[ $fail = 0 ] && echo "M193-VERIFY-OK"
[ $fail = 0 ] || exit 1
