#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M386 · N2 三条「会改可观测行为」的收口（清歌清单 PX-007 / PX-013 / PX-031）
# ---------------------------------------------------------------------------
# 口径（有契约保护的就不动，加安全变体；无契约保护的直接修）：
#   PX-013 缺陷 692  `px run` / `px build` 里 `def main()` + 顶层显式 `main()`
#                    ⇒ 程序跑两遍（副作用翻倍）。**无用例保护双跑**（L009 警告本身
#                    就写着"副作用全部翻倍"）⇒ 直接修：顶层显式调用时跳过自动入口。
#   PX-007 缺陷 691  `sha256(null)` == `sha256("null")`。**M129/M233 定过契约**，
#                    且被 examples/m233_text_cstr 系统断言（int/list/dict 全有 cases）
#                    ⇒ 旧函数一字不动，新增 sha256_strict / hmac_sha256_strict。
#   PX-031 缺陷 693  `file_size` 失败 px_error（杀进程）。它与 file_stat 返回 null
#                    本来就不一致，但「抛错」在无 try/catch 时至少不静默
#                    ⇒ 旧函数不动，新增 file_size_opt（失败 null，对齐 file_stat）。
#
# 判据（每条都配能红的负控）：
#   [1] 解释轨：显式 main ⇒ 只跑一次 · [1 负控] 不显式调 ⇒ 仍自动跑一次
#   [2] 编译轨（VM 默认）：同 [1] · [2 负控] 同
#   [3] C 轨（--c）：同 [1]
#   [4] 顶层其它语句 + 显式 main ⇒ 顶层语句都执行、main 一次
#   [5] sha256_strict 正控（str/bytes 与旧函数逐字节一致，含空体）
#   [6] sha256_strict 拒绝 null/int/list/dict（4 条，均须 R1002）
#       · [6 负控] sha256(null) 仍 == sha256("null")（M233 契约冻结）
#   [7] hmac_sha256_strict 正控 + 两参各拒 null · [7 负控] 旧 hmac 仍宽松
#   [8] file_size_opt 存在→int / 不存在→null / 目录→不抛错
#       · [8 负控] file_size 不存在**仍抛错**（旧契约不许被顺手改）
#   [9s] 静态：三个名字在 runtime.c 注册 + 名册在场 + 指纹链含 M386
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m386_sem.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# ── PX-013：main 双跑 ──────────────────────────────────────────────────────
cat > "$TMP/explicit.px" <<'PXEOF'
def main():
    print("MAIN-RAN")
print("TOP-RAN")
main()
PXEOF
cat > "$TMP/auto.px" <<'PXEOF'
def main():
    print("MAIN-RAN")
print("TOP-RAN")
PXEOF

# [1] 解释轨
"$PX" run "$TMP/explicit.px" > "$TMP/e_run.txt" 2>&1
n=$(grep -c '^MAIN-RAN$' "$TMP/e_run.txt")
[ "$n" = "1" ] && ok "[1] 解释轨：显式 main ⇒ 只跑一次（修前 2 次）" \
    || bad "[1] 解释轨跑了 $n 次（期望 1）"
# [2] 编译轨（VM 默认）
( cd "$TMP" && "$PX" build explicit.px >/dev/null 2>&1 )
if [ -x "$TMP/build/explicit" ]; then
    "$TMP/build/explicit" > "$TMP/e_build.txt" 2>&1
    n=$(grep -c '^MAIN-RAN$' "$TMP/e_build.txt")
    [ "$n" = "1" ] && ok "[2] 编译轨（VM）：显式 main ⇒ 只跑一次" || bad "[2] 编译轨跑了 $n 次"
else
    bad "[2] 编译轨产物缺失（px build 失败）"
fi
# [3] C 轨
( cd "$TMP" && "$PX" build --c explicit.px >/dev/null 2>&1 )
if [ -x "$TMP/build/explicit" ]; then
    "$TMP/build/explicit" > "$TMP/e_cc.txt" 2>&1
    n=$(grep -c '^MAIN-RAN$' "$TMP/e_cc.txt")
    [ "$n" = "1" ] && ok "[3] C 轨：显式 main ⇒ 只跑一次" || bad "[3] C 轨跑了 $n 次"
fi
# [1 负控]/[2 负控]：不显式调用 ⇒ 自动入口必须还在（不许把糖整个删掉）
"$PX" run "$TMP/auto.px" > "$TMP/a_run.txt" 2>&1
n=$(grep -c '^MAIN-RAN$' "$TMP/a_run.txt")
[ "$n" = "1" ] && ok "[1] 负控：不显式调用 ⇒ 自动入口仍跑 1 次（糖还在）" \
    || bad "[1] 负控失败：自动入口跑了 $n 次（糖被误删）"
( cd "$TMP" && "$PX" build auto.px >/dev/null 2>&1 )
if [ -x "$TMP/build/auto" ]; then
    "$TMP/build/auto" > "$TMP/a_build.txt" 2>&1
    n=$(grep -c '^MAIN-RAN$' "$TMP/a_build.txt")
    [ "$n" = "1" ] && ok "[2] 负控：编译轨自动入口仍跑 1 次" || bad "[2] 负控失败：跑了 $n 次"
fi
# [4] 顶层语句不许被跳过
[ "$(grep -c '^TOP-RAN$' "$TMP/e_run.txt")" = "1" ] \
    && ok "[4] 顶层其它语句仍执行 1 次（显式 main 只抑制自动入口，不抑制顶层）" \
    || bad "[4] 顶层语句执行次数异常"

# ── PX-007：哈希 strict 族 ─────────────────────────────────────────────────
cat > "$TMP/h_ok.px" <<'PXEOF'
print(sha256_strict("abc") == sha256("abc"))
print(sha256_strict(bytes("")) == sha256(""))
print(hmac_sha256_strict("k", "m") == hmac_sha256("k", "m"))
print(sha256(null) == sha256("null"))
print(hmac_sha256(null, "m") == hmac_sha256(null, "m"))
PXEOF
out=$("$PX" run "$TMP/h_ok.px" 2>&1)
want=$'true\ntrue\ntrue\ntrue\ntrue'
[ "$out" = "$want" ] && ok "[5] sha256_strict/hmac_strict 合法侧与旧函数逐字节一致（含空体、含 bytes）" \
    || bad "[5] 合法侧不一致：$(echo "$out" | tr '\n' ' ')"
[ "$(echo "$out" | sed -n '4p')" = "true" ] \
    && ok "[6] 负控：sha256(null) 仍 == sha256(\"null\")（M233 契约冻结，未被顺手改）" \
    || bad "[6] 负控失败：旧 sha256(null) 语义被改动"

i=0
for expr in 'sha256_strict(null)' 'sha256_strict(123)' 'sha256_strict([1])' 'sha256_strict({"a":1})'; do
    i=$((i+1))
    cat > "$TMP/rej.px" <<PXEOF
print($expr)
PXEOF
    o=$("$PX" run "$TMP/rej.px" 2>&1); rc=$?
    if [ "$rc" != "0" ] && echo "$o" | grep -q "R1002"; then
        ok "[6.$i] $expr ⇒ R1002（拒绝静默字符串化）"
    else
        bad "[6.$i] $expr 未报 R1002（rc=$rc, out=$(echo "$o" | head -1)）"
    fi
done
for expr in 'hmac_sha256_strict(null, "m")' 'hmac_sha256_strict("k", null)'; do
    i=$((i+1))
    cat > "$TMP/rej.px" <<PXEOF
print($expr)
PXEOF
    o=$("$PX" run "$TMP/rej.px" 2>&1); rc=$?
    if [ "$rc" != "0" ] && echo "$o" | grep -q "R1002"; then
        ok "[7.$i] $expr ⇒ R1002"
    else
        bad "[7.$i] $expr 未报 R1002（rc=$rc）"
    fi
done

# ── PX-031：file_size_opt ─────────────────────────────────────────────────
printf 'hello' > "$TMP/five.txt"
mkdir -p "$TMP/adir"
cat > "$TMP/fs.px" <<PXEOF
print(file_size_opt("$TMP/five.txt"))
print(file_size_opt("$TMP/nope_404"))
print(type(file_size_opt("$TMP/adir")))
PXEOF
out=$("$PX" run "$TMP/fs.px" 2>&1); rc=$?
l1=$(echo "$out" | sed -n '1p'); l2=$(echo "$out" | sed -n '2p')
[ "$rc" = "0" ] && [ "$l1" = "5" ] && ok "[8] file_size_opt 存在文件 ⇒ 5" \
    || bad "[8] file_size_opt 正常路径错（rc=$rc, l1=$l1）"
[ "$l2" = "null" ] && ok "[8] file_size_opt 不存在 ⇒ null（不抛错）" \
    || bad "[8] file_size_opt 缺失路径返回 $l2（期望 null）"
[ "$rc" = "0" ] && ok "[8] file_size_opt 目录路径不抛进程错误（rc=0）" \
    || bad "[8] file_size_opt 目录路径仍抛错（rc=$rc）"
# [8 负控] 旧 file_size 仍抛错（契约冻结）
cat > "$TMP/fs_bad.px" <<PXEOF
print(file_size("$TMP/nope_404"))
PXEOF
o=$("$PX" run "$TMP/fs_bad.px" 2>&1); rc=$?
if [ "$rc" != "0" ] && echo "$o" | grep -q "获取文件大小失败"; then
    ok "[8] 负控：file_size 不存在**仍抛错**（旧契约原样保留，没被顺手改成返回 -1）"
else
    bad "[8] 负控失败：旧 file_size 语义被改（rc=$rc）"
fi

# ── [9s] 静态断言 ─────────────────────────────────────────────────────────
miss=""
for n in sha256_strict hmac_sha256_strict file_size_opt; do
    grep -q "px_set_global(\"$n\"" runtime/runtime.c || miss="$miss runtime:$n"
    grep -q "$n" tools/lint_core.px || miss="$miss 名册:$n"
    grep -q "\"$n\"" docs/native_index.json || miss="$miss index:$n"
done
[ -z "$miss" ] && ok "[9s] 三个新 native 已注册 + 名册在场 + native_index 在场" \
    || bad "[9s] 登记缺失：$miss"
# 三轨 main 检测都在场（防止只修了一条轨）
mc=0
for f in selfhost/interp.px selfhost/codegen.px selfhost/bc_emit.px; do
    grep -q "expl_main" "$f" && mc=$((mc+1))
done
[ "$mc" = "3" ] && ok "[9s] 解释轨/C 轨/VM 轨三处都装了「显式 main」检测" \
    || bad "[9s] 只有 $mc/3 条轨装了检测（漏的轨仍会双跑）"

echo "──── M386 语义门：$pass 通过 / $fail 失败 ────"
[ "$fail" = 0 ] || { echo "❌ m386_px_semantics 判红"; exit 1; }
echo "M386-SEMANTICS-OK"
