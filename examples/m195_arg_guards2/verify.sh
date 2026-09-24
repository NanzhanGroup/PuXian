#!/usr/bin/env bash
# ============================================================
# M195 门（第 73 轮）：参数守卫**第二批** —— 单形参/元数上界/路径语义（缺陷 223/224/225/226）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6（M194 建立 · M195 增补 §6.5 豁免表）。
# 主问题：M194 的普查只覆盖「消息形如 `需要 (...)` 且形参 ≥2」的 26 个站点；本轮把口径
#   推广到**全部 397 个 native 函数**，抓到三类漏网：
#     ① **缺陷 224 · 单形参 INT 静默截断**（15 函数）：`int_val` 对 float 静默取整 ——
#        `range(3.5)`、`sleep(1.5)`、`chr(65.9)`、`bit_count(255.9)`、`bits_to_float32(1.5)`、
#        `clear_timer(1.5)`；onnx 族更糟：`(int)args[0].as.i` 读的是 float 的**位模式**。
#     ② **缺陷 223 · 元数上界缺失**（3 函数）：h3/quic listen 族守卫是 `nargs < 1` ⇒
#        `h3_server_listen(9000,"c","k","多余的")` 静默接受、`(9000,"c")`（缺 key）静默丢 cert。
#     ③ **缺陷 225 · 路径/数据语义**：`read_bytes(路径)`、`ws_broadcast(data)` 走 `val_cstr`
#        ⇒ `read_bytes(12345)` 会去开一个叫 "12345" 的文件（看似成功实则无意义）。
#   另修 **缺陷 226**：解释轨 `range` 的守卫**报错参数错位**（`range(0,1.5)` 报「实际是 int」
#   —— 取的是 args[0] 的类型）且与 native **同码不同文** ⇒ 两侧统一为点名出错参数。
# ⚠️⚠️ **一处分歧轮内的方向修正（值得记住）**：`sleep`/`sleep_us` 起初也按「补 px_arg_int」处理
#   ⇒ 全量门当场判红 **2 项**（m118 与 m120 的内嵌程序都写了 `sleep(0.1)`/`sleep(1.5)`）。
#   判定：**时长的 float 不是"错的类型"，而是"更精确的时长"** —— 正确的是让小数**真正生效**
#   （Python `time.sleep(0.5)` 同向），而不是报错。⇒ 新增 `px_arg_dur_ns`（接受 int|float，
#   返回纳秒），顺带干掉速查表里那条「`sleep(0.5)` 等于不睡」的老警告（静默错值）。
#   教训：**收紧类型口径前，先跑全量门看生态用法** —— 这条 3 分钟内就抓到 2 处。
# 判据：
#   [1] 静态：`scan_errcodes.py` 新增判据 ⑦（`[S6]`）—— 覆盖**全部 native 函数**，
#       397 个函数 · 未豁免缺检查 **0** · 豁免数（表内每条须在源码里找得到 + 理由非空 +
#       总数棘轮）；[S5] 仍绿。
#   [2] 动态：15 个错例 × 三轨（解释 / VM / C）—— rc≠0 + **同码 R1002** + **同文**
#   [2b] 合法侧：9 例 rc=0（含 `h3_server_listen(0)` 的**单参形态**、`sha256(1)` 的文本语义）
#   [3] 负控 4 道（各自独立判红、源逐字节还原）：
#       A 静态 · 去掉一处单形参检查（bit_count）　B 静态 · 豁免表条目在源码里找不到（改名）
#       C 静态 · 豁免总数棘轮（表上限调小）　　D 动态 · 元数上界退回 `nargs < 1`（仅 C 轨）
#       ⚠️ D 只判 C 轨：解释/VM 跑预编译入库件，改 runtime.c 后必须重烘才生效（M193 教训）。
# 用法：bash examples/m195_arg_guards2/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m195_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c runtime/runtime.h runtime/runtime_ws.c runtime/runtime_h3.c
       runtime/runtime_quic.c runtime/runtime_onnx.c selfhost/ibuiltin.px
       examples/m191_error_codes/scan_errcodes.py)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量：本轮改动必须在位 ──
for pat in 'px_arg_int(args[0], "range", "end")' \
           'px_arg_dur_ns(args[0], "sleep", "ms", 1000000.0)' \
           'px_arg_dur_ns(args[0], "sleep_us", "us", 1000.0)' \
           'px_arg_int(args[0], "chr", "码点")' \
           'px_arg_int(args[0], "bit_count", "n")' \
           'px_arg_int(args[0], "bit_length", "n")' \
           'px_arg_int(args[0], "bits_to_float32", "bits")' \
           'px_arg_int(args[0], "bits_to_float64", "bits")' \
           'px_arg_int(args[0], "clear_timer", "id")' \
           'px_arg_int(args[2], "open", "perm")' \
           'px_arg_str(args[0], "read_bytes", "路径")'; do
    grep -qF "$pat" runtime/runtime.c || { echo "❌ 前置自查失败：runtime.c 缺「$pat」" >&2; exit 2; }
done
grep -qF 'px_arg_str(args[0], "ws_broadcast", "data")' runtime/runtime_ws.c \
    || { echo "❌ 前置自查失败：runtime_ws.c 缺 ws_broadcast 的 data 检查" >&2; exit 2; }
for f in h3_server_listen h3_server_listen_stateless; do
    grep -qF "if (nargs != 1 && nargs != 3)" runtime/runtime_h3.c \
        || { echo "❌ 前置自查失败：runtime_h3.c 缺元数上界（$f）" >&2; exit 2; }
done
grep -qF 'if (nargs != 1 && nargs != 3)' runtime/runtime_quic.c \
    || { echo "❌ 前置自查失败：runtime_quic.c 缺元数上界（quic_h3_listen）" >&2; exit 2; }
grep -qF 'px_arg_int(args[0], "onnx_info", "id")' runtime/runtime_onnx.c \
    || { echo "❌ 前置自查失败：runtime_onnx.c 缺 onnx_info 的 id 检查" >&2; exit 2; }
grep -qF 'return Err(i_r1002("range 的 end 需要整数，实际是 " + i_type_name(end), pos))' selfhost/ibuiltin.px \
    || { echo "❌ 前置自查失败：ibuiltin.px 的 range 守卫未对齐（缺陷 226）" >&2; exit 2; }

norm() {   # $1=输出文件 → "CODE|正文"
    grep -E '^运行时错误' "$1" 2>/dev/null | head -1 \
      | sed -E -e 's/^运行时错误: 错误 \[(R[0-9]{4})\] [0-9]+:[0-9]+: /\1|/' \
               -e 's/^运行时错误 \[[^]]*行[0-9]+\]: (R[0-9]{4}): /\1|/'
}

run3() {
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
run1c() {
    local b="$1" d="$W/nc_$b"
    rm -rf "$d"; mkdir -p "$d"; cp "$HERE/probe/$b.px" "$d/"
    PX_BUILD_ENGINE=c timeout 260 ./tools/px build "$d/$b.px" > "$W/nc_$b.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/nc_$b.c.out" 2>&1; echo $? > "$W/nc_$b.c.rc"
    else echo 999 > "$W/nc_$b.c.rc"; tail -3 "$W/nc_$b.build.log" > "$W/nc_$b.c.out"; fi
    rm -rf "$d/build"
}
judge_case() {
    local b="$1" ec="$2" eb="$3" t ok=1
    for t in interp vm c; do
        [ "$(cat "$W/$b.$t.rc")" != 0 ] || ok=0
        [ "$(norm "$W/$b.$t.out")" = "$ec|$eb" ] || ok=0
        grep -q '^before$' "$W/$b.$t.out" || ok=0
    done
    [ $ok = 1 ]
}

echo "=== [1] 静态判据：全部 native 函数参数守卫 + 豁免表（§6）==="
static_ok() { python3 "$ROOT/examples/m191_error_codes/scan_errcodes.py" --root "$ROOT" > "$W/static.log" 2>&1; }
static_ok; rc=$?
chk "[1] 扫描器全绿（含 [S6]）" "[ \$rc = 0 ]"
chk "[1] [S6] 真跑过" "grep -q '\[S6\] 全部 native 函数参数守卫' '$W/static.log'"
# M201：计数判据改成**下限**（M197 纪律：新增 native 不该让门红 —— 新增 native 时门不该为"计数变了"而红）。
#   同时把 [S6] 行的**实际值**打出来并**拆成两个变量**再判 —— 免得在 chk 的 eval 串里跟
#   `**` / `$` 的转义打架（首版就是这么假红过一次：`awk '{exit !($1 >= 397)}'` 被 eval 打回
#   `backslash not last character on line`，而报出的却是"计数不符"，**指不到真因**）。
S6_LINE="$(grep -E '\[S6\] native 函数合计' "$W/static.log" | head -1)"
N_NAT="$(printf '%s' "$S6_LINE" | grep -oE '合计 [0-9]+' | grep -oE '[0-9]+' | head -1)"
N_WAIV="$(printf '%s' "$S6_LINE" | grep -oE '豁免 [0-9]+/[0-9]+' | head -1)"
N_ZERO="$(printf '%s' "$S6_LINE" | grep -c '未豁免缺检查 \*\*0\*\*')"
echo "   [S6] native 函数合计 = ${N_NAT:-空} · 未豁免缺检查为 0 的行数 = ${N_ZERO:-0} · ${N_WAIV:-无豁免字段}"
#   ⚠️ M202 修：原判据把豁免数写成**等式** `豁免 24/24` ⇒ 本轮 +2 条（base32 解码族）即假红。
#     M197 纪律「计数只做下限」在这里同样适用：改成「**分子 == 分母**（表与源码一致）
#     且 分子 ≥ 24（规模不缩水）」——新增**有理由、走棘轮**的豁免不该让门红。
WAIV_OK="$(printf '%s' "${N_WAIV:-豁免 0/0}" | awk -F'[ /]' '{exit !($2 == $3 && $2 >= 24)}' && echo yes || echo no)"
chk "[1] native 函数 ≥397（**下限** · M201 实测 399）· 未豁免缺检查 0 · 豁免数=分子/分母且 ≥24" \
    "[ ${N_NAT:-0} -ge 397 ] && [ \"$WAIV_OK\" = yes ] && [ ${N_ZERO:-0} -ge 1 ]"
chk "[1] [S5] 仍绿（多形参守卫 **≥137** · 缺检查 0）" \
    "grep -q '多形参守卫合计 .* · 缺类型检查 \*\*0\*\*' '$W/static.log' && grep -oE '多形参守卫合计 [0-9]+' '$W/static.log' | awk '{exit !(\$2 >= 137)}'"

echo "=== [2] 动态判据：15 个错例 × 三轨（同码 + 同文）==="
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

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立判红 · 源逐字节还原）==="

    # A 静态：[S6] 不再看见单形参检查
    restore_all; snapshot
    sed -i 's|px_arg_int(args\[0\], "bit_count", "n")|int_val(args[0])|' runtime/runtime.c
    static_ok
    chk "[3A] 去掉 bit_count 的检查 ⇒ [S6] 判红" "[ \$? != 0 ] && grep -q 'bi_bit_count' '$W/static.log'"

    # B 静态：豁免表条目在源码里找不到（改名）
    restore_all; snapshot
    sed -i 's|^static LXValue bi_xxhash(LXValue\* args|static LXValue bi_xxhash_renamed(LXValue* args|' runtime/runtime.c
    static_ok
    chk "[3B] 豁免表条目在源码里找不到（bi_xxhash 改名）⇒ 判红" \
        "[ \$? != 0 ] && grep -q '在源码里\*\*找不到\*\*' '$W/static.log'"

    # C 静态：豁免总数棘轮（把上限调小）
    restore_all; snapshot
    #   ⚠️ M202 修：原 sed 锚点是 `ARG_GUARD2_TOTAL = 24`（硬编码旧值）⇒ 数值一改**打桩失效**、
    #     门报「棘轮未判红」却指不到真因。改成**版本无关**：把上限压到 1（表里只要有 ≥1 条就红）。
    sed -i -E 's|^ARG_GUARD2_TOTAL = [0-9]+|ARG_GUARD2_TOTAL = 1|' examples/m191_error_codes/scan_errcodes.py
    grep -q '^ARG_GUARD2_TOTAL = 1' examples/m191_error_codes/scan_errcodes.py || { echo "  FAIL [3C] 打桩失败（锚点未命中）"; fail=$((fail+1)); }
    static_ok
    chk "[3C] 豁免上限压到 1 ⇒ 棘轮判红" "[ \$? != 0 ] && grep -q 'ARG_GUARD2_EXEMPT 条目变多' '$W/static.log'"

    # D 动态：元数上界退回 `nargs < 1`（仅 C 轨）
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
for fn in ("h3_server_listen_stateless", "h3_server_listen"):
    p = sys.argv[1] + "/runtime/runtime_h3.c"
    s = open(p, encoding="utf-8").read()
    old = ('    if (nargs != 1 && nargs != 3)\n'
           '        px_error("R1002: %s 需要 (port) 或 (port, cert, key) 参数");' % fn)
    new = ('    if (nargs < 1)\n'
           '        px_error("R1002: %s 需要 (port) 或 (port, cert, key) 参数");' % fn)
    assert s.count(old) == 1, "负控 D 锚点不唯一（%s）" % fn
    open(p, "w", encoding="utf-8").write(s.replace(old, new))
PYEOF
    run1c f1_h3_4args
    chk "[3D] 元数上界退回 nargs<1 ⇒ f1 C 轨静默（rc≠1 且判据失效）" \
        "[ \"\$(cat '$W/nc_f1_h3_4args.c.rc')\" != 1 ]"

    restore_all
fi

echo ""
echo "── 汇总：通过 $pass · 失败 $fail ──"
if [ "$fail" = 0 ]; then echo "M195-VERIFY-OK"; exit 0; else echo "M195-VERIFY-FAIL"; exit 1; fi
