#!/usr/bin/env bash
# ============================================================
# M192 门（第 70 轮）：加密 / 压缩 / 归档族「错误码面」收口（棘轮基线第一批）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md（§1 判据 / §2.3 个数分码 / §3 域前缀闭集）。
# 本轮范围：aes 68 + zip 31 + rsa 11 + zlib 7 + ed25519 4 = **121 站点**全部收口。
#   ① 语言层（首参类型 / 内置实参个数 / 参数值域 / dict 形状）⇒ 补 **R1002**（96 处）
#      —— 依据 §2.3：方法调用个数 ⇒ R1005，**函数/内置** ⇒ R1002。
#   ② 后端算法失败（mbedTLS 返回非 0）⇒ 域前缀 **`aes `**（22 处）；
#      与既有 `md5 `/`sha1 `/`sha256 `/`pbkdf2_sha256 ` 同族，登记进 §3 E7。
#   ③ 资源上限 **`内存不足`**（11 处）登记进 §3 E5。
#   ④ zip 自带 **`zip `** 前缀（25 处）**不改文本**，只登记进 §3 E4 闭集。
#   ⑤ `zlib: 参数需要 str/bytes`（2 处）是**类型检查**而非环境失败 ⇒ 按 §1 去前缀改判 `R1002`
#      （§3 明令：语言层不得借域前缀逃避带码；反之，误用域前缀的也要收回来）。
# 判据：
#   [1] 静态：扫描器四查全绿 + 五个文件「无码 0」+ 未收口合计降为 143
#   [2] 动态：8 个错例 × 三轨（解释/VM/C）—— rc≠0 + **同码** + **同文** + 已进运行期
#   [2b] 合法侧：aes / zlib 往返（rc=0 且值正确）—— 证明收口没把正常路径改坏
#   [2c] 解释轨专属：zlib 2 例 —— 编译轨**默认按引用集自动裁剪模块**，而本仓没有可供
#        `import` 的 zlib stdlib（zlib 是纯 native 组，只有 `native_mod_map.txt` 三条映射）；
#        实测（M192）裸 `px build` 与 `px build --full` **均**报 `R1001: 未定义变量: 'zlib_crc32'`
#        ⇒ 属「编译轨无法启用该模块」的独立问题 ⇒ 登记为**观察项 218**；
#        该模块的错误码面由**静态判据 + 解释轨动态判据**覆盖。
#   [3] 负控 3 道：A 静态去码 · B 静态改未登记域 · C 动态改码（**各自独立**判红、源逐字节还原）
# 用法：bash examples/m192_encfamily/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m192_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime_aes.c runtime/runtime_zip.c runtime/runtime_rsa.c runtime/runtime_zlib.c runtime/runtime_ed25519.c)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(basename "$f")"; done; }
restore_all() { for f in "${FILES[@]}"; do [ -f "$BACK/$(basename "$f")" ] && cp -f "$BACK/$(basename "$f")" "$f"; done; return 0; }
# 纪律（M190/M191 教训）：每道负控前 **先 restore 再 snapshot**（干净起点）；trap 只还原、不删工作目录
snapshot
trap 'restore_all' EXIT

# 前置不变量：本轮的四类改动必须在位（缺一即门坏）
for pat in 'R1002: aes_encrypt 需要 3 个参数' 'aes 密钥设置失败' 'R1002: 参数需要 str/bytes' \
           'R1002: ed25519_keygen 不需要参数' 'R1002: zip_pack 第一个参数须为'; do
    grep -q "$pat" runtime/*.c || { echo "❌ 前置自查失败：runtime 缺「$pat」（源码被还原/被别的门盖掉？）" >&2; exit 2; }
done

static_ok() { python3 "$ROOT/examples/m191_error_codes/scan_errcodes.py" --root "$ROOT" > "$W/static.log" 2>&1; }

echo "=== [1] 静态判据：带码/域前缀 · 个数分码 · 混写拆分 · 未收口棘轮 ==="
static_ok
chk "[1] 扫描器四查全绿" "[ $? = 0 ]"
for f in aes zip rsa zlib ed25519; do
    chk "[1] runtime_${f}.c 无码 0（本族已收口）" "grep -q 'runtime_${f}\.c .*无码 0' '$W/static.log'"
done
# M193（第 71 轮）起：棘轮欠账第二批把剩余 143 站点全部收口 ⇒ 扫描器**不再打印**「未收口」行。
#   （本判据原为「合计已降到 143」；M193 后 143 → 0 ⇒ 判据随之更新为「不再有未收口行」。
#     ⚠️ 这是**第五次**「旧门判据被后续里程碑改动作废」—— 见 CHANGELOG §纪律。）
chk "[1] 未收口清单已**清零**（M193 全量收口后：扫描器不再有「未收口」行）" "! grep -q '未收口无码站点' '$W/static.log'"

echo "=== [2] 动态判据：10 个错例 × 三轨（同码 + 同文）==="
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
    timeout 180 ./tools/px build "$d/$b.px" > "$W/$b.vm.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.vm.out" 2>&1; echo $? > "$W/$b.vm.rc"
    else echo 999 > "$W/$b.vm.rc"; tail -3 "$W/$b.vm.build.log" > "$W/$b.vm.out"; fi
    rm -rf "$d/build"
    PX_BUILD_ENGINE=c timeout 240 ./tools/px build "$d/$b.px" > "$W/$b.c.build.log" 2>&1
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
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[2] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"

echo "=== [2b] 合法侧：收口不得改坏正常路径 ==="
while IFS=$'\t' read -r b want; do
    [ -n "$b" ] || continue
    run3 "$b"
    for t in interp vm c; do
        chk "[2b] $b ($t)：rc=0 且输出含「$want」" \
            "[ \"\$(cat '$W/$b.$t.rc')\" = 0 ] && grep -q '$want' '$W/$b.$t.out'"
    done
done < "$HERE/probe/EXPECT_OK.tsv"

echo "=== [2c] 解释轨专属：zlib（编译轨按引用集裁剪，见门头 / 观察项 218）==="
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    d="$W/i_$b"; rm -rf "$d"; mkdir -p "$d"; cp "$HERE/probe/$b.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$b.interp.out" 2>&1
    echo $? > "$W/$b.interp.rc"
    chk "[2c] $b：解释轨 rc≠0 + $ec + 同文「$eb」" \
        "[ \"\$(cat '$W/$b.interp.rc')\" != 0 ] && [ \"\$(norm '$W/$b.interp.out')\" = '$ec|$eb' ] && grep -q '^before$' '$W/$b.interp.out'"
done < "$HERE/probe/EXPECT_INTERP.tsv"
while IFS=$'\t' read -r b want; do
    [ -n "$b" ] || continue
    d="$W/io_$b"; rm -rf "$d"; mkdir -p "$d"; cp "$HERE/probe/$b.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$b.interp.ok.out" 2>&1
    echo $? > "$W/$b.interp.ok.rc"
    chk "[2c] $b：解释轨 rc=0 且输出含「$want」（zlib 往返）" \
        "[ \"\$(cat '$W/$b.interp.ok.rc')\" = 0 ] && grep -q '$want' '$W/$b.interp.ok.out'"
done < "$HERE/probe/EXPECT_OK_INTERP.tsv"

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
    PAT_A='s/px_error("R1002: aes_encrypt 需要 3 个参数/px_error("aes_encrypt 需要 3 个参数/'
    PAT_B='s/px_error("aes 密钥设置失败")/px_error("crypto 密钥设置失败")/'
    PAT_C='s/px_error("R1002: aes_encrypt 需要 3 个参数: (data, key, iv)")/px_error("R1007: aes_encrypt 需要 3 个参数: (data, key, iv)")/'
    nc_static "A 去掉 R 码（语言层无码 ⇒ 棘轮/闭集判红）" "static_ok" "sed -i '$PAT_A' runtime/runtime_aes.c"
    nc_static "B 改未登记域名（crypto 未登记 §3）" "static_ok" "sed -i '$PAT_B' runtime/runtime_aes.c"
    # 负控 C 打**动态**靶：R1002 → R1007 ⇒ 「同码」判据必须由绿转红（编译轨现编 runtime.c，改动必生效）
    restore_all; snapshot
    sed -i "$PAT_C" runtime/runtime_aes.c
    run3 e2_aes_arity
    if judge_case 'e2_aes_arity' 'R1002' 'aes_encrypt 需要 3 个参数: (data, key, iv)'; then
        echo "  FAIL 负控 C：动态判据仍为绿（负控没有牙）"; fail=$((fail+1))
    else
        echo "  PASS 负控 C：动态判据已判红（码被改成 R1007 ⇒ 同码判据失效）"; pass=$((pass+1))
    fi
    restore_all
fi

echo ""
echo "结果: $pass 通过 / $fail 失败"
[ $fail = 0 ] && echo "M192-VERIFY-OK"
[ $fail = 0 ] || exit 1
