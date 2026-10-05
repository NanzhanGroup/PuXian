#!/usr/bin/env bash
# ============================================================
# M258 门 · 「收缩必须走延迟语义」（缺陷 460 · 与 M183 缺陷 197 同族）
# ------------------------------------------------------------
# 被证命题
#   native 桥的**深度式登记 + 出口归一**惯用法：
#       rd = px_root_depth(&rm);  box = ...; PX_KEEP(box);  px_root_restore(rd, rm);
#       return box;                       // 调用方随后自己 PX_KEEP
#   若 `px_root_restore` **立即**把 `g_px_roots_n` 收缩回 rd，则 `box` 在
#   「交回调用方 → 调用方 PX_KEEP」这个窗口里**只由 C 局部持有** ——
#   而 VM 轨产物默认 precise GC（**不扫 C 栈**）⇒ 窗口内任何一次分配都可能回收它。
#   修前 M170 给 `px_root_restore` 定的是「立即收缩」，而 M183 只把 `px_root_pop`
#   改成了延迟 ⇒ **同一个洞在另一个入口上留着**（缺陷 460）。
#
# 本门四层（层间互相独立）
#   [1] 守卫自证（check_root_contract.py --self-test · 5 判据各自判红）
#   [2] 守卫实检（本仓 runtime/ ⇒ 0 违例 + 规模锚点）
#   [3] **窗口正判据**：等价形态的「立即收缩」探针（probe_m258_iso.c，用
#       px_root_iso_mark/restore_iso —— 语义就是立即收缩）在 VM 轨默认档下
#       ⇒ **必须**丢掉容器（失效报告 / 值不对 / 信号致死）⇒ 证明窗口真实存在
#       ⚠️ 这一层不需要改任何产品代码 ⇒ **负控免费**
#   [4] **修复判据**：probe_m258.c（走 px_root_restore）在同档下 ⇒ 值必须正确、
#       无任何检测器报告、rc=0
#   [5] 负控 3 道：A 注 J1 违例 ⇒ [2] 必红 · B 用 iso 探针替身跑 [4] ⇒ 必红 ·
#       C 判据自伤（[3] 的检测恒真）⇒ 与 [4] 冲突 ⇒ 可判定
# 用法：verify.sh [--neg-skip] [--keep]
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
W="${M258_W:-/tmp/m258_gate}"; rm -rf "$W"; mkdir -p "$W"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad2(){ echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad2 "$1（期望 $3，实得 $2）"; fi; }

echo "══ [1] 守卫自证（每条判据必须能**分别**判红）════"
out="$("$ROOT/selfhost/check_root_contract.py" --self-test --tmp "$W/st" 2>&1)"; rc=$?
echo "$out" | sed 's/^/     /'
[ "$rc" = 0 ] && ok "[1] 自证 5 条全绿" || bad2 "[1] 自证失败 rc=$rc"

echo "══ [2] 守卫实检（本仓 runtime/）════"
"$ROOT/selfhost/check_root_contract.py" --root "$ROOT" > "$W/guard.txt" 2>&1; rc=$?
grep -E '^扫描：' "$W/guard.txt" | sed 's/^/     /'
grep -qE 'ROOT-CONTRACT-OK' "$W/guard.txt" && ! grep -q '^❌' "$W/guard.txt"
chk "[2] 全仓契约无违例（rc）" "$rc" 0
nf=$(grep -oP '(?<=函数 )\d+' "$W/guard.txt" | head -1)
[ -n "${nf:-}" ] && [ "$nf" -ge 900 ] && ok "[2] 规模锚点（解析函数 $nf ≥ 900）" \
    || bad2 "[2] 规模锚点异常（nf=${nf:-空}）"

echo "══ [3] 构建探针（链接 .rtcache 的 runtime 对象）════"
RT="$ROOT/runtime"
# ── M258 · 选料必须**可判定**（M223 缺陷 324 / M169 缺陷 185 的老坑）──
#   本门链接的是 `.rtcache` 里的 runtime 对象 ⇒ 若挑到**别的源码版本**烘出的那份，
#   探针测的就是**旧语义**（本次实测踩过：挑到未含缺陷 460 修复的旧目录 ⇒ [4] 假红）。
#   判据：缓存目录名 == `tools/px rtkey`（= 当前 runtime 源码的内容哈希）。
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
    echo "  ⚠️ 无可用 .rtcache ⇒ 本层 SKIP（先跑 ./tools/px build examples/hello.px）"
    echo "M258-SKIP-NOCACHE"
else
    echo "     选料：${CACHE#$ROOT/}"
    objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
    LIBS="$RT/third_party/sqlite3/sqlite3.o
$RT/mbedtls/lib/libmbedtls.a $RT/mbedtls/lib/libmbedx509.a $RT/mbedtls/lib/libmbedcrypto.a
$RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a
$RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a
$RT/third_party/zlib/lib/libz.a -lm -ldl -lpthread"
    buildprobe() {   # $1=源文件基名 → $W/$1
        gcc -c -O2 -I"$CACHE" -I"$RT" "$HERE/$1.c" -o "$W/$1.o" >"$W/$1.cc.log" 2>&1 || return 1
        gcc -static -O2 -pthread -o "$W/$1" "$W/$1.o" $objs $LIBS >"$W/$1.link.log" 2>&1 || return 1
        return 0
    }
    bp_ok=1
    buildprobe probe_m258     || { bad2 "[3] probe_m258 构建失败"; tail -5 "$W/probe_m258.cc.log" "$W/probe_m258.link.log" | sed 's/^/     /'; bp_ok=0; }
    buildprobe probe_m258_iso || { bad2 "[3] probe_m258_iso 构建失败"; tail -5 "$W/probe_m258_iso.cc.log" "$W/probe_m258_iso.link.log" | sed 's/^/     /'; bp_ok=0; }
    [ "$bp_ok" = 1 ] && ok "[3] 两个探针构建成功"

    # 共同的判据：把「输出」与「检测器标记」分开看
    MARK='\[M257-CTR\]|\[M257-ROOT\]|\[M258-|已回收|LIVECHK|UAFDET|Segmentation|core dumped'
    runprobe() {   # $1=探针名  $2=tag
        env PX_GC_STRESS=1 PX_GC_LIVECHK=1 PX_GC_UAFDET=1 \
            timeout -k 5 180 "$W/$1" >"$W/$2.out" 2>"$W/$2.err"
        echo $? > "$W/$2.rc"
    }

    if [ "$bp_ok" = 1 ]; then
        echo "══ [3] 窗口正判据（立即收缩形态 ⇒ 容器必丢）════"
        runprobe probe_m258_iso iso
        irc=$(cat "$W/iso.rc")
        ilen=$(grep -oP '(?<=^LEN=)\S+' "$W/iso.out" || true)
        imark=$(grep -cE "$MARK" "$W/iso.out" "$W/iso.err" 2>/dev/null | awk -F: '{s+=$2} END{print s+0}')
        echo "     iso: rc=$irc LEN=${ilen:-无} 检测器标记=$imark"
        if [ "$irc" -ge 128 ] || [ "$imark" -gt 0 ] || [ "${ilen:-x}" != "4" ]; then
            ok "[3] 立即收缩形态**确实**丢掉容器（窗口真实存在）"
        else
            bad2 "[3] 立即收缩形态竟未丢容器 ⇒ 探针不敏感（判据无牙）"
        fi

        echo "══ [4] 修复判据（px_root_restore ⇒ 延迟收缩）════"
        runprobe probe_m258 fix
        frc=$(cat "$W/fix.rc")
        flen=$(grep -oP '(?<=^LEN=)\S+' "$W/fix.out" || true)
        fsum=$(grep -oP '(?<=^SUM=)\S+' "$W/fix.out" || true)
        fmark=$(grep -cE "$MARK" "$W/fix.out" "$W/fix.err" 2>/dev/null | awk -F: '{s+=$2} END{print s+0}')
        tailend=$(grep -c 'M258-PROBE-END' "$W/fix.out" || true)
        echo "     fix: rc=$frc LEN=${flen:-无} SUM=${fsum:-无} 标记=$fmark 收尾行=$tailend"
        chk "[4] 退出码" "$frc" 0
        chk "[4] LEN" "${flen:-无}" 4
        chk "[4] SUM" "${fsum:-无}" 66
        chk "[4] 检测器标记数" "$fmark" 0
        chk "[4] 程序跑到结尾" "$tailend" 1
    fi

    echo "══ [5] 负控 3 道（各自独立判红）════"
    if [ "$NEG" = 0 ]; then
        echo "  ⏭  --neg-skip（CI 档）"
    else
        # NC-A：往 runtime.c 注一个 J1 违例（把延迟行退回立即收缩）⇒ 守卫必红
        SRC="$ROOT/runtime/runtime.c"; cp "$SRC" "$W/runtime.c.pre"
        if python3 - "$SRC" <<'PY'
import re, sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
n = s.count("g_px_trunc_pending = roots_depth;")
if n != 1:
    print("锚点不唯一：%d" % n); sys.exit(1)
s = s.replace("g_px_trunc_pending = roots_depth;", "g_px_roots_n = roots_depth;")
open(p, 'w', encoding='utf-8').write(s)
PY
        then
            "$ROOT/selfhost/check_root_contract.py" --root "$ROOT" >"$W/ncA.txt" 2>&1; arc=$?
            if [ "$arc" = 1 ] && grep -q '\[J1\]' "$W/ncA.txt"; then
                ok "[5A] J1 违约注入 ⇒ 守卫判红"
            else
                bad2 "[5A] J1 违约注入却未判红（rc=$arc）"
            fi
        else
            bad2 "[5A] 锚点替换失败（GAP1）"
        fi
        cp "$W/runtime.c.pre" "$SRC"
        cmp -s "$W/runtime.c.pre" "$SRC" && ok "[5A] 源逐字节还原" || bad2 "[5A] 源未还原"

        # NC-B：用 iso 探针替身跑 [4] ⇒ 必须红（证明 [4] 有牙）
        if [ "$bp_ok" = 1 ]; then
            ilen2=$(grep -oP '(?<=^LEN=)\S+' "$W/iso.out" || true)
            if [ "${ilen2:-无}" = 4 ] && [ "$(cat "$W/iso.rc")" = 0 ]; then
                bad2 "[5B] 替身探针竟与修复版同结果 ⇒ [4] 无牙"
            else
                ok "[5B] 替身（立即收缩）不满足 [4] 判据 ⇒ [4] 有牙"
            fi
        fi

        # NC-C：判据自伤 —— 让 [3] 的「检测」恒真 ⇒ 修复版也会被 [3] 判红
        #       （判据不区分两档 ⇒ 红不再来自被测差异）
        if [ "$bp_ok" = 1 ]; then
            if [ "$(cat "$W/fix.rc")" -ge 128 ] || grep -qE "$MARK" "$W/fix.err"; then
                bad2 "[5C] 修复版竟满足 [3] 判据 ⇒ 两档不可区分"
            else
                ok "[5C] 两档可区分（[3] 的红来自被测差异，非判据噪声）"
            fi
        fi
    fi
fi

echo
echo "══ 汇总：通过 $PASS · 失败 $FAIL ══"
[ "$KEEP" = 1 ] || true
if [ "$FAIL" = 0 ]; then echo "M258-VERIFY-OK"; exit 0; fi
echo "M258-VERIFY-FAIL"; exit 1
