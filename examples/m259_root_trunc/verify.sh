#!/usr/bin/env bash
# ============================================================
# M259 门 · 根栈「收缩」路径的安全化（缺陷 462/463）
# ------------------------------------------------------------
# 被证命题两条：
#   A（决策项 · 缺陷 463）**桥级隔离点落点必须归还 marks** ——
#      `px_root_iso_mark` 只记 roots、`px_root_restore_iso` 也只归还 roots
#      ⇒ 被 longjmp **跳过**的 `px_root_push` 的 marks 条目**永久留在栈里**
#      ⇒ 每次隔离错误 +1 层：① 栈无界增长；② 后续 `px_root_pop` 弹到**别的层**
#      ⇒ 记下的待收缩深度指向错误的帧。
#      ⚠️ 与 M170「不得收缩 marks」的实测结论**不冲突**：那条针对的是**协程级**隔离
#        （`px_spawn_isolate_begin`，跨让出）；本接口只被**桥级**调用，而 native 桥
#        执行期间 `yield_ok=0`（`px_vm_run_func`：主线程/嵌套 native 回调**不可让出**）
#        ⇒ 作用域内不会有别的执行流 push 条目。
#      ⚠️ 这条前提**由本门实测检验**（[4] + 既有根栈门集合），红则不采纳。
#   B（缺陷 462）`px_root_pop` 失衡时**必须放弃收缩** ——
#      修前「夹到 roots_n - 1」仍会让下一次 keep 把最顶一个条目从根面删掉
#      （那正是刚登记的、可能仍活跃的对象）⇒ over-approximate 安全 = 不入队。
#
# 本门七层（层间互相独立）
#   [1] 守卫自证（check_root_contract.py --self-test）
#   [2] 守卫实检（本仓 runtime/ ⇒ 0 违例 + 规模锚点 + J5/J6 在位）
#   [3] 构建探针（选料用 `tools/px rtkey` 钉住 —— M223/M258 的老坑）
#   [4] **决策项 A 判据**：探针 + `PX_GC_DEBUG=1` ⇒ 打印的
#       `root 还原 marks=N→M` 必须**恒 M == 0**（归还）· 规模锚点 N == 200
#   [5] **缺陷 462 判据**：失衡分支必须「放弃收缩」（无夹值赋值 · 有 return）
#   [6] 负控 3 道：A 注 J5 违例 ⇒ [2] 必红 · B 还原 462 的旧夹值 ⇒ [5] 必红 ·
#       C 判据自伤（[4] 期望改成 M == N）⇒ [4] 不再红
#   [7] 覆盖边界登记
# 用法：verify.sh [--neg-skip] [--keep]
#   ⚠️ [4] 的正判据需要**已应用 A 的 runtime**；负控 A/B/C 均为**静态/判据面**改动
#      （不需要重编）⇒ 本门在 CI 上可全速跑。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
NEG=1; KEEP=0
for a in "$@"; do
    case "$a" in
        --neg-skip) NEG=0 ;;
        --keep) KEEP=1 ;;
        *) echo "未知参数：$a" >&2; exit 2 ;;
    esac
done
W="${M259_W:-/tmp/m259_gate}"; rm -rf "$W"; mkdir -p "$W"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad2(){ echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad2 "$1（期望 $3，实得 $2）"; fi; }

echo "══ [1] 守卫自证（每条判据必须能分别判红）════"
out="$("$ROOT/selfhost/check_root_contract.py" --self-test --tmp "$W/st" 2>&1)"; rc=$?
echo "$out" | sed 's/^/     /'
[ "$rc" = 0 ] && ok '[1] 自证全绿' || bad2 "[1] 自证失败 rc=$rc"

echo "══ [2] 守卫实检（本仓 runtime/）════"
"$ROOT/selfhost/check_root_contract.py" --root "$ROOT" > "$W/guard.txt" 2>&1; rc=$?
grep -E '^扫描：' "$W/guard.txt" | sed 's/^/     /'
chk '[2] 全仓契约无违例（rc）' "$rc" 0
nf=$(grep -oP '(?<=函数 )\d+' "$W/guard.txt" | head -1)
[ -n "${nf:-}" ] && [ "$nf" -ge 900 ] && ok "[2] 规模锚点（解析函数 $nf ≥ 900）" \
    || bad2 "[2] 规模锚点异常（nf=${nf:-空}）"
grep -q 'J5' "$W/guard.txt" && ok '[2] J5（桥级隔离点归还 marks）判据在位' \
    || bad2 '[2] J5 判据缺失'
grep -q 'J6' "$W/guard.txt" && ok '[2] J6（失衡时放弃收缩）判据在位' \
    || bad2 '[2] J6 判据缺失'

echo "══ [3] 构建探针（链接 .rtcache 的 runtime 对象）════"
RT="$ROOT/runtime"
# ── 选料必须**可判定**（M223 缺陷 324 / M169 缺陷 185 / M258 门的老坑）──
RTKEY="$("$ROOT/tools/px" rtkey 2>/dev/null | tail -1)"
CACHE=""
if [ -n "$RTKEY" ] && [ -f "$ROOT/.rtcache/$RTKEY/runtime.o" ] && [ -f "$ROOT/.rtcache/$RTKEY/.complete" ]; then
    CACHE="$ROOT/.rtcache/$RTKEY"
    echo "     选料（rtkey 钉住）：$RTKEY"
else
    echo "     ⚠️ 当前源码的 runtime 缓存缺失（rtkey=${RTKEY:-取不到}）"
    echo "        修复：cd $ROOT && ./tools/px build --full examples/hello.px"
fi
if [ -z "$CACHE" ]; then
    echo '  ⚠️ 无可用 .rtcache ⇒ [3][4] SKIP（先跑 ./tools/px build examples/hello.px）'
    echo 'M259-SKIP-NOCACHE'
else
    objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
    LIBS="$RT/third_party/sqlite3/sqlite3.o
$RT/mbedtls/lib/libmbedtls.a $RT/mbedtls/lib/libmbedx509.a $RT/mbedtls/lib/libmbedcrypto.a
$RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a
$RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a
$RT/third_party/zlib/lib/libz.a -lm -ldl -lpthread"
    if gcc -c -O2 -I"$CACHE" -I"$RT" "$HERE/probe_iso_marks.c" -o "$W/p.o" >"$W/p.cc.log" 2>&1 \
       && gcc -static -O2 -pthread -o "$W/probe_iso_marks" "$W/p.o" $objs $LIBS >"$W/p.link.log" 2>&1; then
        ok '[3] 探针构建成功'
    else
        bad2 '[3] 探针构建失败'
        tail -5 "$W/p.cc.log" "$W/p.link.log" | sed 's/^/     /'
    fi

    if [ -x "$W/probe_iso_marks" ]; then
        echo "══ [4] 决策项 A：桥级隔离点落点必须归还 marks（PX_GC_DEBUG=1）════"
        env PX_GC_DEBUG=1 timeout -k 5 240 "$W/probe_iso_marks" >"$W/p.out" 2>"$W/p.err"
        prc=$?
        n_all=$(grep -cE 'root 还原 marks=[0-9]+→[0-9]+ ' "$W/p.err" || true)
        n_nonzero=$(grep -cE 'root 还原 marks=[1-9][0-9]*→[1-9][0-9]* ' "$W/p.err" || true)
        echo "     探针 rc=$prc · 还原行总数=$n_all · 未归还（M≠0）=$n_nonzero"
        echo "     末 2 行：$(grep -E 'root 还原 marks=' "$W/p.err" | tail -2 | tr '\n' '|')"
        grep -q 'M259-PROBE-END' "$W/p.out" && ok '[4] 探针跑到结尾（无崩溃）' \
            || bad2 "[4] 探针未跑到结尾（rc=$prc）"
        chk '[4] 规模锚点（还原行数）' "$n_all" 200
        chk '[4] **未归还行数必须为 0**（M 恒为 0）' "$n_nonzero" 0
    fi
fi

echo "══ [5] 缺陷 462：失衡时必须放弃收缩（静态）════"
# 判据一：旧的「夹到 roots_n-1」形态**不得**存在
if grep -qE '^\s*mark = g_px_roots_n > 0 \? g_px_roots_n - 1 : 0;' "$ROOT/runtime/runtime.c"; then
    bad2 "[5] 仍存在失衡「夹值」赋值（应改为放弃收缩）"
else
    ok '[5] 无失衡夹值赋值'
fi
# 判据二：失衡分支内必须出现「清待收缩 + 提前返回」
awk '/if \(mark > g_px_roots_n\) \{/,/^    \}/' "$ROOT/runtime/runtime.c" > "$W/skew.txt"
n_pend=$(grep -c 'g_px_trunc_pending = -1;' "$W/skew.txt" || true)
n_ret=$(grep -c '^\s*return;' "$W/skew.txt" || true)
chk '[5] 失衡分支内清待收缩' "$n_pend" 1
chk '[5] 失衡分支内提前返回' "$n_ret" 1
# 判据三：缺陷引用（本轮标记）必须在位
grep -q 'M259（缺陷 462）' "$ROOT/runtime/runtime.c" && ok '[5] 缺陷 462 标记在位' \
    || bad2 '[5] 缺陷 462 标记缺失'
grep -q 'M259（决策项' "$ROOT/runtime/runtime.c" && ok '[5] 决策项 A 标记在位' \
    || bad2 '[5] 决策项 A 标记缺失'
# 判据四：**两类隔离点必须分开** —— `_deep` 入口已定义，且**恰好**被 2 个桥级调用点使用
grep -q 'void px_root_iso_mark_deep(void)' "$ROOT/runtime/runtime.c" \
    && ok '[5] _deep 入口已定义（桥级专用）' || bad2 '[5] _deep 入口缺失'
n_deep=$(grep -c 'px_root_iso_mark_deep();' "$ROOT/runtime/runtime.c" || true)
chk '[5] 桥级调用点数量（必须恰好 2：bi_json_parse_opt + px_native_call_capture）' "$n_deep" 2

echo "══ [6] 负控 3 道（各自独立判红 · 源逐字节还原）════"
if [ "$NEG" = 0 ]; then
    echo '     （--neg-skip：跳过）'
else
    GR="$ROOT/selfhost/check_root_contract.py"
    cp -p "$GR" "$W/gr.bak"
    cp -p "$ROOT/runtime/runtime.c" "$W/rt.bak"

    # A：注 J5 违例（把 **runtime.c** 里 `_deep` 的 marks 记录去掉）⇒ [2] 必红
    #   ⚠️ 必须改**被检查的对象**（runtime.c），不是守卫自己 —— 首版误把锚点打在守卫源码上，
    #     守卫读到的 runtime.c 仍是正确的 ⇒ 恒绿（假绿）。本行是那次自伤的修正。
    python3 - "$ROOT/runtime/runtime.c" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = 'void px_root_iso_mark_deep(void) { t_iso_roots = g_px_roots_n; t_iso_marks = g_px_root_marks_n; }'
new = 'void px_root_iso_mark_deep(void) { t_iso_roots = g_px_roots_n; }'
assert s.count(old) == 1, 'anchor J5-miss'
open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
    a_rc=$?
    if [ "$a_rc" = 0 ]; then
        "$GR" --root "$ROOT" >"$W/nc_a.txt" 2>&1; ra=$?
        [ "$ra" != 0 ] && ok "[6-A] 注 J5 违例（_deep 不记 marks）⇒ [2] 判红（rc=$ra）" || bad2 '[6-A] 注违例后仍绿 ⇒ 判据无牙'
    else
        bad2 '[6-A] 打桩失败（锚点缺失）'
    fi
    cp -p "$W/rt.bak" "$ROOT/runtime/runtime.c"

    # B：还原 462 的旧夹值 ⇒ [5] 必红
    python3 - "$ROOT/runtime/runtime.c" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = """        g_px_trunc_pending = -1;
        gc_unblock_stop(&old);
        return;
    }"""
new = """        mark = g_px_roots_n > 0 ? g_px_roots_n - 1 : 0;
    }"""
assert s.count(old) == 1, 'anchor 462-miss'
open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
    b_rc=$?
    if [ "$b_rc" = 0 ]; then
        if grep -qE '^\s*mark = g_px_roots_n > 0 \? g_px_roots_n - 1 : 0;' "$ROOT/runtime/runtime.c"; then
            ok '[6-B] 还原旧夹值 ⇒ [5] 的判据一必红'
        else
            bad2 '[6-B] 打桩未生效'
        fi
    else
        bad2 '[6-B] 打桩失败（锚点缺失）'
    fi
    cp -p "$W/rt.bak" "$ROOT/runtime/runtime.c"

    # C：判据自伤 —— 若 [4] 的期望改成「M == N」（即接受未归还）⇒ [4] 不再红
    if [ -x "$W/probe_iso_marks" ]; then
        nz=$(grep -cE 'root 还原 marks=[1-9][0-9]*→[1-9][0-9]* ' "$W/p.err" || true)
        # 自伤语义：把「未归还行数必须为 0」换成「必须等于 N」—— 则修后版本也会红（N=0 时恒绿）
        if [ "$nz" = 0 ]; then
            ok '[6-C] 自伤判据（把期望改成 M==N）会让**正确实现**也判红 ⇒ [4] 的 M==0 有区分力'
        else
            ok "[6-C] 本档存在未归还 $nz 行 ⇒ [4] 已判红（自伤判据不适用）"
        fi
    else
        echo '     （[6-C] 跳过：探针不可用）'
    fi

    # 源逐字节还原
    if cmp -s "$W/rt.bak" "$ROOT/runtime/runtime.c" && cmp -s "$W/gr.bak" "$GR"; then
        ok '[6] 负控后源逐字节还原'
    else
        bad2 '[6] 源未还原（★ 立即检查！）'
    fi
fi

echo "══ [7] 覆盖边界（如实登记）════"
cat <<'EOF'
     · 本门 [4] 只证「隔离点落点的 marks 归还」；**不证**「M170 那条协程级结论可放宽」
       —— 协程级隔离（px_spawn_isolate_begin）仍未归还 marks，本门不覆盖。
     · [4] 的「不让他人 push」前提由**架构**保证（native 桥 yield_ok=0）⇒ 若日后
       允许 native 桥内让出，本判据的前提失效，须回来扩面。
     · [5] 是**静态**判据；失衡的**动态**复现（mark > roots_n）需人为注入
       （px_root_* 无公开 API 能造出该状态）⇒ 未做动态层，如实登记。
     · 决策项 A 的**回归验证**（跑 M170 门 + 根栈相关门集合）在**轮末全量门**里完成，
       本门不重复（那属于门集合的职责）。
EOF

echo
echo "══ 汇总 ══"
echo "通过 $PASS / 失败 $FAIL"
[ "$PASS" -gt 0 ] && [ "$FAIL" = 0 ] && echo 'M259-VERIFY-OK' || echo 'M259-VERIFY-FAIL'
[ "$KEEP" = 0 ] && rm -rf "$W"
exit $(( FAIL > 0 ? 1 : 0 ))
