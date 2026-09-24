#!/usr/bin/env bash
# ============================================================
# M202 门（第 81 轮）：**加密/排序扩展面 + 非字符串实参渲染别名**
#   · 第三方 PX-DEF-032 base32 · PX-DEF-033 hmac_sha1 · PX-DEF-034 sorted key
#   · 本轮由 [2] 的探针**现场照出来**的缺陷 244（`bdata`/`val_cstr` 共享静态缓冲）
# ------------------------------------------------------------
# 权威口径 = docs/PUXIAN_CHEATSHEET.md 事实 229 / 230 / 231
#
# 为什么需要（三条 PX-DEF 都是"三轨一致地缺"）：
#   · PX-DEF-032 无 base32 native —— OTP 秘钥的**标准文本格式**就是 base32
#     （RFC 4648 §6 / RFC 4226 §3.1 的 `secret`）⇒ 官方 `registry/totp` 只能**纯 .px 自实现**。
#   · PX-DEF-033 无 hmac_sha1 —— RFC 4226/6238（HOTP/TOTP）的**默认算法**，
#     语言只有 hmac_sha256 ⇒ totp 只能用 `sha1_bytes` 手工拼 64 字节块 + ipad/opad。
#   · PX-DEF-034 `sorted` 只收 1 个参数（无 key）⇒ 自然排序只能靠 `[键, 下标, 原值]` 绕行。
#
# 口径决定（写下来，后面别翻案）：
#   ① base32 与 base64 族**逐项对齐**：`base32_encode` / `base32_decode`（非法→null）/
#      `bytes_base32` / `base32_to_bytes`；解码**宽松面逐条写明**（大小写不敏感 · 填充可省 ·
#      忽略 ASCII 空白；其余一律 null）。**同一个陷阱照旧**：`base32_decode` 回 **str**，
#      二进制一律走 `base32_to_bytes`。
#   ② `hmac_sha1` → 40 字符小写 hex；`hmac_sha1_bytes` → 20 字节。
#      验收用 **RFC 2202 §3 全部 7 条向量**（不是"跑一遍抄下来"）。
#   ③ `sorted(xs[, key_fn])`：**取键恰一次/元素、按原始顺序**；比较仍走 `compare_values`
#      ⇒ M162 的「值比较 + 稳定排序」**不变**；**明确不做 comparator**（任意 int 比较器会让
#      "稳定排序"失去可判定性，与 M162 立的"同一比较器下输出唯一"直接冲突）。
#   ④ **缺陷 244**：`bdata`/`val_cstr` 对非 str/bytes 实参曾共用一处 `static char tmp[64]`
#      ⇒ `hmac_sha256(123, 456)` 实测 == `hmac(456, 456)`（key 被 msg 顶掉），同族
#      `hmac_sha1` / `pbkdf2_sha256` / `regex_*` 全中 —— **安全原语静默错值**；
#      且它是进程级 static ⇒ 多线程并发取值互踩。修法 = **每线程 8 槽轮转环**。
#
# 判据：
#   [1] 静态：注册点 · 两侧 arity/key 守卫文案**逐字一致**（[S8] 同源）· 解释轨仍走
#            `i_cmp_values` · 事实 229/230/231 · native 计数 ≥387 且 6 新名在册 · 名册已重跑 ·
#            **缺陷 244 的修法在位**（`px_tmp_slot` + 两处调用点不再有共享 static）。
#   [2] 动态正例 5 组 × 三轨（解释 / VM / C）**逐字节一致**（期望值**独立算得**：
#            RFC 向量来自 RFC，其余来自 Python 的 base64/hmac/pbkdf2 与"键长程序化"）。
#   [3] 动态拒绝侧 6 例 × 三轨：rc≠0 · **同码** · **同文案** · 前置标记在。
#   [4] 负控 5 道（各自独立判红 · 源逐字节还原 · 锚点找不到即 FAIL 而不是假绿）：
#            A 原生忽略 key ⇒ sorted_key 红（b32/hmac 仍绿）　B 解释轨忽略 key（**重编 dev 解释器**）
#            C base32 去掉有效位掩码 ⇒ **仅** b32_rt 红　D 稳定性条件 `>= 0` ⇒ **仅** sorted_stable 红
#            E `bdata` 退回共享 static ⇒ **仅** hmac1_pos 红（缺陷 244 复现）
# 用法：bash examples/m202_crypto_ext/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
for a in "$@"; do case "$a" in --neg-skip) NEG=0 ;; esac; done

W=/tmp/m202_gate; BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c selfhost/ibuiltin.px)
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
chkout() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; cat "$W/$3" 2>/dev/null | sed 's/^/      | /' | head -12; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量（缺了说明门自己过期）──
pre() { grep -qF "$1" "$2" || { echo "❌ 前置自查失败：$2 缺「$1」" >&2; exit 2; }; }
pre 'px_set_global("base32_encode", px_native("base32_encode", bi_base32_encode));' runtime/runtime.c
pre 'px_set_global("base32_decode", px_native("base32_decode", bi_base32_decode));' runtime/runtime.c
pre 'px_set_global("base32_to_bytes", px_native("base32_to_bytes", bi_base32_to_bytes));' runtime/runtime.c
pre 'px_set_global("bytes_base32", px_native("bytes_base32", bi_bytes_base32));' runtime/runtime.c
pre 'px_set_global("hmac_sha1", px_native("hmac_sha1", bi_hmac_sha1));' runtime/runtime.c
pre 'px_set_global("hmac_sha1_bytes", px_native("hmac_sha1_bytes", bi_hmac_sha1_bytes));' runtime/runtime.c
pre 'px_error("R1002: sorted 需要 (list[, key_fn]) 参数");' runtime/runtime.c
pre 'i_r1002("sorted 需要 (list[, key_fn]) 参数", pos)' selfhost/ibuiltin.px
pre 'static char* px_tmp_slot(void) {' runtime/runtime.c
for f in "$HERE"/probe/*.px; do [ -f "$f" ] || { echo "❌ 前置自查失败：probe 目录为空" >&2; exit 2; }; done

# 解释轨探针跑法：**被测解释器可覆盖**（负控 B 用重编的 dev 件）
PXI_BIN="$ROOT/bootstrap/pxi"
build3() {   # $1=probe 名 → $W/$1.{interp,vm,c}.{out,rc}
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
judge3() {   # $1=probe  $2=期望 stdout
    local n="$1" want="$2" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" = 0 ] || return 1
        [ "$(norm_out "$W/$n.$t.out")" = "$want" ] || return 1
    done
    return 0
}
judge_rej() {   # $1=probe $2=R 码 $3=消息子串
    local n="$1" code="$2" msg="$3" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" != 0 ] || return 1
        grep -q '^before$' "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$code" "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$msg" "$W/$n.$t.out" 2>/dev/null || return 1
    done
    return 0
}
stub() { python3 "$HERE/stubs.py" "$1" "$ROOT" > "$W/stub_$1.log" 2>&1; }

echo "=== [1] 静态：注册点 / 两侧文案同源 / 缺陷 244 修法在位 / 文档 / 计数 ==="
chk "[1] 6 个新 native 全在 runtime 注册（base32×4 + hmac_sha1×2）" \
    "[ \"\$(grep -cE 'px_set_global\(\"(base32_encode|base32_decode|base32_to_bytes|bytes_base32|hmac_sha1|hmac_sha1_bytes)\"' runtime/runtime.c)\" -eq 6 ]"
chk "[1] sorted arity 文案**两侧逐字一致**（[S8] 同源）" \
    "grep -qF 'R1002: sorted 需要 (list[, key_fn]) 参数' runtime/runtime.c && grep -qF 'i_r1002(\"sorted 需要 (list[, key_fn]) 参数\", pos)' selfhost/ibuiltin.px"
chk "[1] key 位类型守卫文案两侧一致" \
    "grep -qF 'R1002: sorted 的第 2 个参数需要函数，实际是 %s' runtime/runtime.c && grep -qF 'sorted 的第 2 个参数需要函数，实际是 \" + kt' selfhost/ibuiltin.px"
chk "[1] 解释轨仍走**同一条**值比较器（i_cmp_values，M162 单一真相未被绕过）" \
    "grep -q 'i_cmp_values(keys\[idx\[j\]\], keys\[idx\[j + 1\]\])' selfhost/ibuiltin.px"
chk "[1] 缺陷 244 修法在位：每线程轮转环 + 两个取值点都改用它" \
    "grep -q 'static char\* px_tmp_slot(void)' runtime/runtime.c && [ \"\$(grep -c 'char\* tmp = px_tmp_slot();' runtime/runtime.c)\" -eq 2 ]"
chk "[1] 缺陷 244 旧形态已消失（val_cstr/bdata 内不再有共享 static char tmp）" \
    "! grep -qE 'static char tmp\[64\];' runtime/runtime.c"
chk "[1] 速查表事实 229/230/231 已在位" \
    "grep -q '229. \*\*.base32_encode' docs/PUXIAN_CHEATSHEET.md && grep -q '230. \*\*.hmac_sha1' docs/PUXIAN_CHEATSHEET.md && grep -q '231. \*\*.sorted(xs\[, key_fn\])' docs/PUXIAN_CHEATSHEET.md"
N_NAT="$(python3 -c 'import json;print(json.load(open("docs/native_index.json"))["count"])')"
chk "[1] native 计数 ≥ 387（下限 · 现 $N_NAT）且 6 个新名全在册" \
    "[ \"$N_NAT\" -ge 387 ] && python3 -c 'import json;n=set(json.load(open(\"docs/native_index.json\"))[\"names\"]);import sys;sys.exit(0 if {\"base32_encode\",\"base32_decode\",\"base32_to_bytes\",\"bytes_base32\",\"hmac_sha1\",\"hmac_sha1_bytes\"}<=n else 1)'"
chk "[1] 内置名册（tools/lint_core.px）已重跑 ⇒ 6 个新名在内（否则 lint 误报 L002）" \
    "python3 '$HERE/check_names.py' '$ROOT'"

echo "=== [2] 动态正例 5 组 × 三轨（解释 / VM / C）逐字节一致 ==="
cat > "$W/b32_pos.want" <<'EOF'
e01=
e02=MY======
e03=MZXQ====
e04=MZXW6===
e05=MZXW6YQ=
e06=MZXW6YTB
e07=MZXW6YTBOI======
e08=GEZDG===
e09=MFRGG===
e10=AD7RA===
EOF
cat > "$W/b32_rt.want" <<'EOF'
d01=f
d02=fo
d03=foo
d04=foob
d05=fooba
d06=foobar
d07=
d08=foo
d09=foobar
d10=foobar
d11=null
d12=null
d13=null
d14=null
d15=000102
d16=00ff00
r0=/
r1=01/AE======
r2=0108/AEEA====
r3=01080f/AEEA6===
r4=01080f16/AEEA6FQ=
r5=01080f161d/AEEA6FQ5
r6=01080f161d24/AEEA6FQ5EQ======
r7=01080f161d242b/AEEA6FQ5EQVQ====
r8=01080f161d242b32/AEEA6FQ5EQVTE===
r9=01080f161d242b3239/AEEA6FQ5EQVTEOI=
r10=01080f161d242b323940/AEEA6FQ5EQVTEOKA
r11=01080f161d242b32394047/AEEA6FQ5EQVTEOKAI4======
r12=01080f161d242b323940474e/AEEA6FQ5EQVTEOKAI5HA====
EOF
cat > "$W/hmac1_pos.want" <<'EOF'
h01=b617318655057264e28bc0b6fb378c8ef146be00
h02=effcdf6ae5eb2fa2d27416d5f184df9c259a7c79
h03=125d7342b9ac11cd91a39af48aa17b4f63f175d3
h04=4c9007f4026250c6bc8414f9bf50c86c2d7235da
h05=4c1a03424b55e07fe7f27be1d58bb9324a9a5a04
h06=aa4ae5e15272d00e95705637ce8a3b55ed402112
h07=e8e99d0f45237d786d6bbaa7965c7808bbff1a91
h08=de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9
h09=de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9
h10=ef914b4bdc872dace16f5043ac3f4ce9604c13ee
h11=20
h12=40
n01=6ab97ba270783282de3e9ee84cb352ae1c40396c
n02=6ab97ba270783282de3e9ee84cb352ae1c40396c
n03=d46bcbca58fe719a5d8df73175ec453ae6a7778e3e84a64ca87a63ec7c02b41f
n04=d46bcbca58fe719a5d8df73175ec453ae6a7778e3e84a64ca87a63ec7c02b41f
n05=e488a2dc6b8bf24b
n06=e488a2dc6b8bf24b
n07=false
n08=false
n09={end: 2, groups: [null, null, null, null, null, null, null, null, null], match: 12, start: 0}
n10={end: 2, groups: [null, null, null, null, null, null, null, null, null], match: 12, start: 0}
n11=12
n12=12
EOF
cat > "$W/sorted_key.want" <<'EOF'
k01=[3, 2, 1]
k02=3/[3, 1, 2]
k03=[3, 2, 1]
k04=[1, 2, 3]
k05=[a, cc, bbb]
k06=[x2, x1, x10]
k07=[200, 30, 4]
k08=[[1, a], [2, b]]
k09=[a, bb, ccc]
k10=[] [7]
k11=[3, 1, 2]/[3, 2, 1]
k12=[3, 2, 1]
k13=[6, 4, 2]
k14=[a, b, c]
EOF
cat > "$W/sorted_stable.want" <<'EOF'
s01=[0.5, 1.0, 1]
s02=[0.5, 1, 1.0]
s03=[[a, 1], [a, 2], [a, 3], [b, 0]]
s04=[[0, x], [1, z], [1, y]]
s05=[5, 4, 3, 2, 1]
s06=[2, 9, 10]
s07=[[1, a], [1, a2]]
s08=[0.5, 1.0, 1]
s09=[0.5, 1, 1.0]
s10=[true, 1, a]
s11=[1, [2]]
s12=[a, b, c]
s13=[1, 2, 3]
EOF

for n in b32_pos b32_rt hmac1_pos sorted_key sorted_stable; do
    build3 "$n"
    case "$n" in
        b32_pos)      d="RFC 4648 §10 全 7 向量 + 串化 + bytes_base32" ;;
        b32_rt)       d="宽松面（无填充/小写/空白）+ 非法→null + 二进制安全 + 0..12 字节往返" ;;
        hmac1_pos)    d="RFC 2202 §3 全 7 向量 + _bytes 形态 + NUL + **非字符串实参同值（缺陷 244 面）**" ;;
        sorted_key)   d="取键恰一次/按原序 + 常见用法 + 不改原值 + 可迭代实参统一" ;;
        sorted_stable) d="键相等保持原序 + **无 key 零回归**（M162 值比较/稳定未被破坏）" ;;
    esac
    chkout "[2] $n：$d" "judge3 $n \"\$(cat $W/$n.want)\"" "$n.interp.out"
done

echo "=== [3] 动态拒绝侧 6 例 × 三轨（rc≠0 · 同码 · 同文案）==="
cat > "$W/rej.tsv" <<'EOF'
rej_sorted_arity|R1002|sorted 需要 (list[, key_fn]) 参数
rej_sorted_keytype|R1002|sorted 的第 2 个参数需要函数，实际是 int
rej_sorted_arg0|R1002|sorted 参数需要 list/tuple/生成器/字符串，实际是 int
rej_b32_arity|R1002|base32_encode 需要一个参数
rej_hmac1_arity|R1002|hmac_sha1 需要 (key, msg) 参数
rej_keyerr|R1003|索引越界: 1 (len=1)
EOF
while IFS='|' read -r n code msg; do
    [ -n "$n" ] || continue
    build3 "$n"
    chkout "[3] $n：三轨 rc≠0 · 同码 $code · 同文案" "judge_rej $n '$code' '$msg'" "$n.interp.out"
done < "$W/rej.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立判红 · 源逐字节还原 · 打桩失败即 FAIL）==="
    # A：原生忽略 key ⇒ sorted_key 红，base32 面仍绿（独立牙）
    restore_all; snapshot
    if stub A; then
        build3 sorted_key >/dev/null 2>&1; build3 b32_pos >/dev/null 2>&1
        chk "[4A] 原生忽略 key ⇒ sorted_key 判红" "! judge3 sorted_key \"\$(cat $W/sorted_key.want)\""
        chk "[4A] 且**只**红这一面：b32_pos 仍绿（独立牙）" "judge3 b32_pos \"\$(cat $W/b32_pos.want)\""
    else
        echo "  FAIL [4A] 打桩失败"; cat "$W/stub_A.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    # B：解释轨忽略 key —— **必须重编 dev 解释器**（改源码不重编 = 跑的还是入库件 = 假绿）
    restore_all; snapshot
    if stub B; then
        if timeout 900 bash selfhost/devbuild.sh pxi > "$W/negB.build" 2>&1 && [ -x /tmp/pxidev ]; then
            cp -f /tmp/pxidev "$W/pxidev_negB"
            ( cd "$W/b_sorted_key" && timeout 120 "$W/pxidev_negB" sorted_key.px ) > "$W/sorted_key.interp.out" 2>&1
            echo $? > "$W/sorted_key.interp.rc"
            chk "[4B] 解释轨忽略 key（**重编 dev 解释器**后跑）⇒ sorted_key 判红" "! judge3 sorted_key \"\$(cat $W/sorted_key.want)\""
            chk "[4B] 失败原因是**键没生效**（输出读得到）" "! grep -q 'k01=\[3, 2, 1\]' '$W/sorted_key.interp.out'"
        else
            echo "  FAIL [4B] dev 解释器构建失败"; tail -3 "$W/negB.build" | sed 's/^/      | /'; fail=$((fail+1))
        fi
    else
        echo "  FAIL [4B] 打桩失败"; cat "$W/stub_B.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    PXI_BIN="$ROOT/bootstrap/pxi"
    # C：base32 解码去掉有效位掩码 ⇒ 仅 b32_rt 红
    restore_all; snapshot
    if stub C; then
        build3 b32_rt >/dev/null 2>&1; build3 b32_pos >/dev/null 2>&1
        chk "[4C] 去掉长度/填充合法性校验 ⇒ b32_rt 判红（非法输入不再 null）" "! judge3 b32_rt \"\$(cat $W/b32_rt.want)\""
        chk "[4C] 且**只**红解码面：b32_pos（纯编码）仍绿" "judge3 b32_pos \"\$(cat $W/b32_pos.want)\""
    else
        echo "  FAIL [4C] 打桩失败"; cat "$W/stub_C.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    # D：稳定性条件 `>= 0` ⇒ 仅 sorted_stable 红
    restore_all; snapshot
    if stub D; then
        build3 sorted_stable >/dev/null 2>&1; build3 b32_rt >/dev/null 2>&1
        chk "[4D] 稳定性条件改 \`>= 0\` ⇒ sorted_stable 判红（键相等不再保持原序）" "! judge3 sorted_stable \"\$(cat $W/sorted_stable.want)\""
        #   ⚠️ 独立性用 b32_rt 而不是 sorted_key：sorted_key 的 k06 本身就**依赖稳定性**
        #      （等键保持原序）⇒ 拿它做「无关面」是门自己的判据错（M202 实测假红一次）。
        chk "[4D] 且**只**红排序面：b32_rt 仍绿" "judge3 b32_rt \"\$(cat $W/b32_rt.want)\""
    else
        echo "  FAIL [4D] 打桩失败"; cat "$W/stub_D.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    # E：`bdata` 退回共享 static ⇒ 缺陷 244 复现（仅 hmac1_pos 红）
    restore_all; snapshot
    if stub E; then
        build3 hmac1_pos >/dev/null 2>&1; build3 b32_rt >/dev/null 2>&1
        chk "[4E] bdata 退回共享 static ⇒ hmac1_pos 判红（缺陷 244 复现）" "! judge3 hmac1_pos \"\$(cat $W/hmac1_pos.want)\""
        chk "[4E] 且**只**红这一面：b32_rt 仍绿" "judge3 b32_rt \"\$(cat $W/b32_rt.want)\""
        chk "[4E] 复现形态可读（key 被 msg 顶掉 ⇒ n01==n03 的值域）" "grep -q 'n01=f5d5a2a27c73f3e892e6352d8cf6f7a802d7b08d' '$W/hmac1_pos.vm.out'"
    else
        echo "  FAIL [4E] 打桩失败"; cat "$W/stub_E.log" | sed 's/^/      | /'; fail=$((fail+1))
    fi
    restore_all
fi

echo "── 结果 ──"
echo "  通过 $pass · 失败 $fail"
[ "$fail" = 0 ] && { echo "M202-CRYPTO-EXT-VERIFY-OK"; exit 0; }
echo "M202-CRYPTO-EXT-VERIFY-FAIL"; exit 1
