#!/usr/bin/env bash
# ============================================================
# M201 门（第 80 轮）：**解释器件必须是「全能力」件** —— 缺陷 241（= 第三方 PX-DEF-035 复现）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.13（能力面查询原语）
#
# 为什么需要（实测取证，不是推理）：
#   第三方登记 PX-DEF-035：官方 **aarch64 引导包**里 `import "c/zlib"` + `extern def zlib_*`
#   编译轨全通、解释轨 `Err(ffi_call: 未注册函数: zlib_compress)`。
#   M200 我在 **x86_64 仓库内态**上复核，得出"他们方向记反了"的结论 —— **那个结论错了**：
#   本轮把**已发布**的 m197 / m200 aarch64 包下载解包逐件复核：
#     · `bootstrap/pxc` 有 **19** 个 `zlib_*` 名字；
#     · `bootstrap/pxi` 有 **0** 个 —— 且族级差异不止 zlib：aes / rsa / ed25519 / sqlite /
#       xml / zip / ws 整族都缺。
#   根因：发布链用 `tools/px build <interp.px>`（**按引用集自动裁剪**）现编解释器；
#   而解释器是**泛化分派**（native 名在运行期按字符串查表）⇒ 裁剪器"看不见"这些名字
#   ⇒ 把整族模块从解释器里裁掉。仓库内 x86_64 入库件不受影响（`devbuild.sh` /
#   `rebake_bin.sh` 链的是**全量** runtime 对象）⇒ 缺陷**只在发布链**、只在非 x86_64 包上被看见。
#
# 对策（两件，各有一句可执行的判据）：
#   ① `native_symbols()` 原语（runtime）—— 把"本构建有没有某能力"从**靠猜**变成**可查询**
#     （第三方登记里点名要的"FFI 可用符号清单查询原语"）。
#   ② 发布链对解释器件改用 `--full`，且构建口径**只有一处**（`selfhost/build_native_tools.sh`）
#     —— 原先 ci.yml 与 release.yml 各写一遍 12 件清单，正是漂移的温床。
#
# 判据：
#   [1] 期望集（**源码派生** + 自证）：`runtime/*.c` 的 `px_set_global("X", px_native(` 与
#       `px_ffi_register("X"` 名并集；按本架构能力过滤（`--print-plan` 的 `no_quic` ⇒ 去 quic/h3）；
#       自证：名字数 ≥ M201_EXP_MIN（默认 300）+ 每个"可裁模块"家族至少 1 名（防退化假绿）。
#   [2] 被测件**实跑**：`<interp> dump_symbols.px` 报出的集合必须**包含**整个期望集；
#       缺失名逐条打印（≤20 条）。⚠️ "包含"判据必须配 [3B] 的空输出负控（否则空集假绿）。
#   [3] 动态**真调用**：`zlib_crc32` / `zlib_compress`↔`zlib_uncompress` / `bytes_to_hex`↔
#       `hex_to_bytes` 在解释轨与编译轨**逐字节一致**（名字存在 ≠ 已注册 ≠ 可调用；M200 就是后者）。
#   [4] 负控（各自独立判红）：
#       A 用一个**真被裁过**的产物当被测件（`--no-zlib` 现编）⇒ [2] 必红且指名 `zlib_*`
#       B 用**空输出**探针 ⇒ [2] 必红（防"空集 ⊇ 任意集"的假绿）
#       C 把期望集下限抬到不可达（`M201_EXP_MIN=99999`）⇒ [1] 必红（证明下限判据不是摆设）
# 用法：bash examples/m201_interp_ffi/verify.sh [--neg-skip] [--interp <件>]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
INTERP="$ROOT/bootstrap/pxi"
EXP_MIN="${M201_EXP_MIN:-300}"
while [ $# -gt 0 ]; do
    case "$1" in
        --neg-skip) NEG=0; shift ;;
        --interp) INTERP="$2"; shift 2 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done

W="${M201_FFI_W:-/tmp/m201_ffi_gate}"   # 可覆盖：**[4C]/[4D] 是自调用**，内层会把工作区 rm -rf
                                        #   ⇒ 内层必须换一个目录，否则外层日志被自己的内层删掉
rm -rf "$W"; mkdir -p "$W"
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
chkout() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; head -12 "$W/$3" 2>/dev/null | sed 's/^/      | /'; fail=$((fail+1)); fi; }

[ -x "$INTERP" ] || { echo "❌ 前置自查失败：被测解释器件不可执行：$INTERP" >&2; exit 2; }
[ -f "$HERE/probe/dump_symbols.px" ] || { echo "❌ 前置自查失败：缺 dump_symbols.px" >&2; exit 2; }

# ---- 本架构能力：no_quic ⇒ quic/h3 族**合法**缺席（不该要求）----
NO_QUIC=0
grep -q '^plan: no_quic=1' /tmp/.m201_plan 2>/dev/null && NO_QUIC=1
if command -v timeout >/dev/null; then
    timeout 60 "$ROOT/tools/px" build --print-plan "$HERE/probe/dump_empty.px" > "$W/plan.txt" 2>&1 || true
    grep -q '^plan: no_quic=1' "$W/plan.txt" && NO_QUIC=1
fi
echo "── 本架构能力：no_quic=$NO_QUIC（1 ⇒ quic/h3 族合法缺席，从期望集里剔除）"

# ---- [1] 期望集（源码派生）----
build_expected() {   # → $W/expected.txt（排序去重）
    { grep -hoE 'px_set_global\("[^"]+", *px_native\(' "$ROOT"/runtime/*.c | sed 's/px_set_global("//; s/", *px_native(//'
      grep -hoE 'px_ffi_register\("[^"]+"' "$ROOT"/runtime/*.c | sed 's/px_ffi_register("//; s/"//'
    } | LC_ALL=C sort -u | { if [ "$NO_QUIC" = 1 ]; then grep -vE '^(quic_|h3_)'; else cat; fi; } > "$W/expected.txt"
    # 自测钩子（只给 [4C]/[4D] 的**自调用负控**用，正常跑不设）：人为抽掉一个锚点
    if [ -n "${M201_DROP_ANCHOR:-}" ]; then
        grep -vx "$M201_DROP_ANCHOR" "$W/expected.txt" > "$W/e2.txt"; mv "$W/e2.txt" "$W/expected.txt"
    fi
}
build_expected
N_EXP="$(wc -l < "$W/expected.txt")"
echo "=== [1] 期望集：源码派生 $N_EXP 名（下限 $EXP_MIN）==="
chk "[1] 期望集规模 ≥ $EXP_MIN（源码派生，防退化）" "[ $N_EXP -ge $EXP_MIN ]"
FAMS="zlib_compress sqlite_open aes_encrypt rsa_sign ed25519_sign zip_pack ws_serve xml_parse onnx_op_names img_decode route md5 sha256_bytes base64_encode"
miss_fam=0
for f in $FAMS; do
    grep -qx "$f" "$W/expected.txt" || { echo "      ✗ 家族代表缺席（判据锚点失效）：$f"; miss_fam=$((miss_fam+1)); }
done
chk "[1] 14 个家族代表都在期望集内（锚点自证）" "[ $miss_fam = 0 ]"

# ── **设计性豁免**（解释器 = Mini 子集：并发原语仅编译轨，见 docs/MINI_SUBSET.md §边界表）──
#   逐条给理由；判据见 [1b]/[1c]/[1d] —— 豁免是"有据可查的清单"，不是"缺了就写进豁免"。
WAIVED="spawn chan chan_try_recv select_try mutex rwlock"
#   · spawn / chan / chan_try_recv / select_try：协程与通道（Mini 子集不含并发）
#   · mutex / rwlock：同步原语（同上）
w_bad=0
for x in $WAIVED; do grep -qx "$x" "$W/expected.txt" || { echo "      ✗ 豁免项不在期望集里（豁免过期）：$x"; w_bad=$((w_bad+1)); }; done
chk "[1b] 6 条豁免都在期望集内（豁免未过期）" "[ $w_bad = 0 ]"
chk "[1c] 豁免规模 ≤ 8（不许悄悄长大）" "[ $(echo $WAIVED | wc -w) -le 8 ]"
w_doc=0
for x in $WAIVED; do grep -q "$x" "$ROOT/docs/MINI_SUBSET.md" || { echo "      ✗ 豁免项在 docs/MINI_SUBSET.md 里查不到依据：$x"; w_doc=$((w_doc+1)); }; done
chk "[1d] 每条豁免都能在 docs/MINI_SUBSET.md 找到依据（有据可查）" "[ $w_doc = 0 ]"

# ---- [2] 被测件实跑（能力面）----
dump_of() {   # $1=被测件  → 报告名写入 $W/report_<tag>.txt
    local bin="$1" tag="$2"
    ( cd "$W" && timeout 120 "$bin" "$HERE/probe/dump_symbols.px" ) > "$W/raw_$tag.txt" 2>&1
    LC_ALL=C sort -u "$W/raw_$tag.txt" | grep -vE '^$' > "$W/report_$tag.txt"
}
dump_of "$INTERP" interp
N_REP="$(wc -l < "$W/report_interp.txt")"
# 比较面 = 期望集 **减去设计性豁免**
cp -f "$W/expected.txt" "$W/required.txt"
for x in $WAIVED; do grep -vx "$x" "$W/required.txt" > "$W/req2.txt"; mv "$W/req2.txt" "$W/required.txt"; done
comm -23 "$W/required.txt" "$W/report_interp.txt" > "$W/missing_interp.txt"
N_MISS="$(wc -l < "$W/missing_interp.txt")"
N_REQ="$(wc -l < "$W/required.txt")"
echo "── 被测件：$INTERP（自报 native 名 $N_REP 个）· 必需面 $N_REQ 名（期望 $N_EXP − 豁免 $(echo $WAIVED | wc -w)）"
[ "$N_MISS" != 0 ] && { echo "── 缺失（前 20）："; head -20 "$W/missing_interp.txt" | sed 's/^/      - /'; }
chk "[2] 被测解释器件**包含**整个必需面（缺 $N_MISS 名）" "[ $N_MISS = 0 ]"
chk "[2] 被测件自报规模非退化（≥ $EXP_MIN）" "[ $N_REP -ge $EXP_MIN ]"

# ---- [3] 动态真调用（解释轨 ⇄ 编译轨逐字节一致）----
d="$W/callp"; rm -rf "$d"; mkdir -p "$d"; cp -f "$HERE/probe/zlib_roundtrip.px" "$d/"
( cd "$d" && timeout 60 "$INTERP" zlib_roundtrip.px ) > "$W/call.interp.out" 2>&1; echo $? > "$W/call.interp.rc"
( cd "$d" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build zlib_roundtrip.px ) > "$W/call.build.log" 2>&1
if [ -x "$d/build/zlib_roundtrip" ]; then ( cd "$d" && timeout 60 "$d/build/zlib_roundtrip" ) > "$W/call.c.out" 2>&1; echo $? > "$W/call.c.rc"
else echo 999 > "$W/call.c.rc"; tail -3 "$W/call.build.log" > "$W/call.c.out"; fi
rm -rf "$d/build"
WANT_CALL='crc=891568578
back=hello 普贤
hex=a1b2'
chkout "[3] 解释轨真调用（zlib 往返 + hex 往返）rc=0 且输出逐字节一致" \
    "[ \"\$(cat '$W/call.interp.rc')\" = 0 ] && [ \"\$(sed '/^$/d' '$W/call.interp.out')\" = \"\$WANT_CALL\" ]" "call.interp.out"
chkout "[3] 编译轨同探针同输出（三轨一致面）" \
    "[ \"\$(cat '$W/call.c.rc')\" = 0 ] && [ \"\$(sed '/^$/d' '$W/call.c.out')\" = \"\$WANT_CALL\" ]" "call.c.out"

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立判红）==="
    # A：真被裁过的产物（--no-zlib）当被测件 ⇒ [2] 必红且指名 zlib
    d="$W/trim"; rm -rf "$d"; mkdir -p "$d"; cp -f "$HERE/probe/dump_symbols.px" "$d/"
    ( cd "$d" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build --no-zlib dump_symbols.px ) > "$W/trim.log" 2>&1
    if [ -x "$d/build/dump_symbols" ]; then
        dump_of "$d/build/dump_symbols" trim
        comm -23 "$W/required.txt" "$W/report_trim.txt" > "$W/missing_trim.txt"
        chk "[4A] 被裁产物（--no-zlib）⇒ 能力面判红" "[ \"\$(wc -l < '$W/missing_trim.txt')\" != 0 ]"
        chk "[4A] 且缺失里**指名** zlib_compress（诊断可执行）" "grep -qx 'zlib_compress' '$W/missing_trim.txt'"
    else
        echo "  FAIL [4A] 打桩失败：--no-zlib 产物未产出"; fail=$((fail+1))
    fi
    rm -rf "$d/build"
    # B：空输出探针 ⇒ 包含判据必须红（防空集假绿）
    ( cd "$W" && timeout 60 "$INTERP" "$HERE/probe/dump_empty.px" ) > "$W/raw_empty.txt" 2>&1
    LC_ALL=C sort -u "$W/raw_empty.txt" | grep -vE '^$' > "$W/report_empty.txt"
    chk "[4B] 空输出 ⇒ 判据必红（防「空集包含任意集」假绿）" \
        "[ \"\$(comm -23 '$W/required.txt' '$W/report_empty.txt' | wc -l)\" != 0 ]"
    # C：把期望集下限抬到不可达 ⇒ **自调用**必红（证明下限判据不是摆设）
    ( cd "$ROOT" && M201_FFI_W="$W/negc_ws" M201_EXP_MIN=99999 bash "$HERE/verify.sh" --neg-skip ) > "$W/negc.log" 2>&1; rc_c=$?
    chk "[4C] 下限抬到 99999 ⇒ 自调用判红（下限判据有效）" "[ \$rc_c = 1 ] && grep -q 'FAIL \[1\] 期望集规模' '$W/negc.log'"
    # D：抽掉一个家族锚点 ⇒ **自调用**必红（证明锚点自证有效）
    ( cd "$ROOT" && M201_FFI_W="$W/negd_ws" M201_DROP_ANCHOR=zlib_compress bash "$HERE/verify.sh" --neg-skip ) > "$W/negd.log" 2>&1; rc_d=$?
    chk "[4D] 抽掉锚点 zlib_compress ⇒ 自调用判红（锚点自证有效）" \
        "[ \$rc_d = 1 ] && grep -q '家族代表缺席（判据锚点失效）：zlib_compress' '$W/negd.log'"
fi

echo "── 结果 ──"
echo "  通过 $pass · 失败 $fail"
[ "$fail" = 0 ] && { echo "M201-INTERP-FFI-VERIFY-OK"; exit 0; }
echo "M201-INTERP-FFI-VERIFY-FAIL"; exit 1
