#!/usr/bin/env bash
# ============================================================
# selfhost/check_opts_valuerange.sh —— px_serve 的 opts **值域**契约（M428 · 缺陷 875）
# ------------------------------------------------------------
# 为什么有它（2026-11-21 · 六条最小复现实测，留档 /tmp/m428r/repro.sh → CHANGELOG）：
#   M427 让「拼错的键名」响亮了；但「键拼对了、值不对」当时仍然静默：
#     rate_limit {max:3, window_sec:-5} ⇒ **限流根本不启用**（6 次请求全 200；合法配置第 4 次起 429）
#     max_body_size:500 ⇒ 静默夹到 1024（启动日志 max_body=1024）
#     max_conn:0 / gzip_level:99 / http3.port:70000 ⇒ 静默忽略 / 用默认
#     rate_limit:"3/60"（类型错）⇒ 同样静默
#   最贵的一条是 rate_limit 越界：「用户以为开了保护，实际没有任何限流」。
#
# 判据（每条都要能判红）：
#   S1 形状：A 类值域校验在**第一个 opts 取值之前**（行号比较）；且 6 个关键键各有校验调用
#   S2 形状：B 类三条夹取路径（max_body_size 下限 / max_conn 上限 / whitelist 上限）都有 stderr 留痕
#   P1 负控（安全面，最值钱）：rate_limit{max:3,window_sec:-5} ⇒ rc≠0，文本含 window_sec + 区间 + 实收值
#   P2 正控：合法 rate_limit ⇒ 服务起，且**真限流**（200 200 200 429 429）
#   P2b 负控对照（N1 的前提自证）：无 rate_limit ⇒ 6 次全 200 ⇒ 429 来自限流本身，不是别处
#   P3 负控：max_conn:0 / gzip_level:99 / gzip_min_bytes:0 / http3.port:70000 逐个 ⇒ rc≠0
#   P4 正控（防过度修复）：边界合法值 max_conn:1 / gzip_level:1 / gzip_level:9 / gzip_min_bytes:1 ⇒ 服务真起来
#   P5 B 类留痕：max_body_size:500 ⇒ 服务起 + stderr 含「已夹到 1024」；whitelist 70 项 ⇒ 含「超过上限 64」
#   P6 负控自证：把 A 类 int 校验函数改成 return ⇒ P1 的信号必须消失（且补丁树必须真重建，NOCACHE）
#   P7 类型错：rate_limit 传字符串 / max_conn 传字符串 / whitelist 传字符串 ⇒ rc≠0（修前全静默）
#   P8 列表项类型：whitelist 含非字符串项 ⇒ rc≠0（修前该项被静默跳过）
#   G1 环境守卫：tools/px / cc / curl 不可用或探针构建失败 ⇒ rc=3（绝不降级成 0）
#   G2 零污染：全程在 $TMP 构建 / 打补丁，仓库 git status 行数前后一致
#
# 用法：check_opts_valuerange.sh             # 全跑（约 60~140s，含一次 runtime 重建）
#       check_opts_valuerange.sh --self-test # 只跑判据夹具（不编译，离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m428_opts.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

RT_BAK="$TMP/runtime.c.bak"
RT_PATCHED=""
cleanup() {
    if [ -n "${RT_PATCHED:-}" ] && [ -f "$RT_BAK" ]; then cp -p "$RT_BAK" "$RT" 2>/dev/null; fi
    chmod -R u+w "$TMP" 2>/dev/null
    rm -rf "$TMP"
}
trap cleanup EXIT

# ── 判据纯函数（可自测）────────────────────────────────────
err_range_ok() {  # $1=输出文本 $2=键名 ⇒ 0 表示「值域错误被响亮拒绝」的形状成立
    case "$1" in *R1002*) ;; *) return 1 ;; esac
    case "$1" in *"$2 = "*) ;; *) return 1 ;; esac      # 必须带实收值（半截报错不算）
    case "$1" in *合法区间*) ;; *) return 1 ;; esac
    case "$1" in *超出*) ;; *) return 1 ;; esac
    return 0
}
err_type_ok() {  # $1=输出文本 $2=键名 ⇒ 类型错必须说「需要…实收类型」
    case "$1" in *R1002*) ;; *) return 1 ;; esac
    case "$1" in *"$2"*) ;; *) return 1 ;; esac
    case "$1" in *实收类型*) ;; *) return 1 ;; esac
    return 0
}
warn_ok() {  # $1=stderr 文本 $2=期望片段
    case "$1" in *警告*) ;; *) return 1 ;; esac
    case "$1" in *"$2"*) ;; *) return 1 ;; esac
    return 0
}

selftest() {
    local st=0
    echo "── 自测（判据夹具，全离线）"
    if err_range_ok '运行时错误 [<top> 行1]: R1002: px_serve opts.rate_limit.window_sec = -5 超出合法区间 1..1000000000 秒' 'rate_limit.window_sec'
    then echo "  PASS S-pos1 真实值域报错形状被接受"; else echo "  FAIL S-pos1 误拒真实报错"; st=1; fi
    if err_range_ok 'before
after' 'window_sec'; then echo "  FAIL S-neg1 静默输出被当成报错"; st=1; else echo "  PASS S-neg1 静默输出被拒"; fi
    if err_range_ok 'R1002: px_serve opts.gzip_level 超出合法区间' 'gzip_level'; then
        echo "  FAIL S-neg2 没带实收值的半截报错被接受"; st=1
    else echo "  PASS S-neg2 半截报错（无实收值）被拒"; fi
    if err_type_ok 'R1002: px_serve opts.rate_limit 需要字典 {max, window_sec, key?, whitelist?}，实收类型 str' 'rate_limit'
    then echo "  PASS S-pos2 类型错形状被接受"; else echo "  FAIL S-pos2 误拒类型错"; st=1; fi
    if err_type_ok 'R1002: px_serve opts.x 超出合法区间' 'x'; then
        echo "  FAIL S-neg3 区间错被当成类型错"; st=1
    else echo "  PASS S-neg3 区间错不被当成类型错"; fi
    if warn_ok '[px-serve] 警告：opts.max_body_size=500 小于下限 1024，已夹到 1024' '已夹到 1024'
    then echo "  PASS S-pos3 B 类留痕形状被接受"; else echo "  FAIL S-pos3 误拒留痕"; st=1; fi
    if warn_ok '[px-serve] 警告：opts.max_body_size=500 小于下限 1024，已夹到 1024' '超过上限'
    then echo "  FAIL S-neg4 错误片段被接受"; st=1; else echo "  PASS S-neg4 无关键片段的留痕被拒"; fi
    # 前提自证：夹具确实作用在真实文件上（扫描面不为空）
    local n
    n="$(grep -c 'px_opt_expect_' "$RT" 2>/dev/null || echo 0)"
    if [ "$n" -ge 10 ]; then echo "  PASS N1 前提自证 校验调用点=$n"; else echo "  FAIL N1 扫描面为空($n)"; st=1; fi
    [ "$st" = 0 ] && echo "SELF-TEST OK" || echo "SELF-TEST FAIL"
    return "$st"
}

if [ "${1:-}" = "--self-test" ]; then
    selftest || exit 2
    exit 0
fi

# ── G1 环境守卫 ────────────────────────────────────────────
[ -f "$PX" ] || { echo "❌ 缺 tools/px" >&2; exit 3; }
[ -f "$RT" ] || { echo "❌ 缺 runtime/runtime.c" >&2; exit 3; }
command -v cc >/dev/null 2>&1 || { echo "❌ 缺 cc" >&2; exit 3; }
command -v curl >/dev/null 2>&1 || { echo "❌ 缺 curl（P2 需要真实请求）" >&2; exit 3; }
ST_BEFORE="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"

mkdir -p "$TMP/www" && printf 'ok\n' > "$TMP/www/index.html"

build() {  # $1=名字 · stdin=源码
    local n="$1"
    cat > "$TMP/$n.px"
    if ! ( cd "$TMP" && "$PX" build "$n.px" >"$TMP/$n.build.log" 2>&1 ); then
        echo "  BUILD-FAIL $n: $(tail -2 "$TMP/$n.build.log" | tr '\n' ' ')"
        return 1
    fi
    [ -x "$TMP/build/$n" ] || { echo "  NO-BIN $n"; return 1; }
    return 0
}
build_nocache() {  # P6 用：补丁树必须真重建
    local n="$1"
    cat > "$TMP/$n.px"
    if ! ( cd "$TMP" && PX_BUILD_NOCACHE=1 "$PX" build "$n.px" >"$TMP/$n.build.log" 2>&1 ); then
        echo "  BUILD-FAIL $n(nocache): $(tail -2 "$TMP/$n.build.log" | tr '\n' ' ')"
        return 1
    fi
    [ -x "$TMP/build/$n" ] || { echo "  NO-BIN $n"; return 1; }
    return 0
}
RC=0; OUT=""
run_err() {  # $1=名字 ⇒ RC/OUT
    RC=0
    OUT="$( cd "$TMP" && timeout 4 "./build/$1" 2>&1 )" || RC=$?
}
free_port() {
    local p
    for p in $(seq 18440 18499); do
        ( exec 3<>"/dev/tcp/127.0.0.1/$p" ) 2>/dev/null || { echo "$p"; return 0; }
        exec 3>&- 2>/dev/null
    done
    return 1
}
# $1=二进制名 $2=端口 $3=次数 ⇒ 打印状态码序列（服务起不来时为空）
codes() {
    ( cd "$TMP" && exec "./build/$1" ) >"$TMP/$1.srv.log" 2>&1 &
    local pid=$! i c seq=""
    sleep 0.6
    for i in $(seq 1 "${3:-6}"); do
        c="$(curl -s -o /dev/null -m 1 -w '%{http_code}' "http://127.0.0.1:$2/" 2>/dev/null || true)"
        seq="$seq $c"
    done
    kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
    echo "$seq"
}
serve_ok() {  # $1=二进制名 $2=端口 ⇒ 真实 GET 200 才算「服务起来」
    local seq; seq="$(codes "$1" "$2" 1)"
    case "$seq" in *200*) return 0 ;; *) return 1 ;; esac
}

# 冒烟构建（环境不成立宁可 rc=3）
if ! printf 'print("smoke")\n' | build smoke >/dev/null 2>&1; then
    echo "❌ 探针构建失败（环境不成立）" >&2; exit 3
fi

echo "== S 源码形状 =="
L_A="$(grep -n 'M428（缺陷 875）：值域校验' "$RT" | head -1 | cut -d: -f1)"
L_GET="$(grep -n 'px_dict_get(args\[3\]' "$RT" | head -1 | cut -d: -f1)"
if [ -n "$L_A" ] && [ -n "$L_GET" ] && [ "$L_A" -lt "$L_GET" ]; then
    ok "S1a 值域校验块在第一个 opts 取值之前（L$L_A < L$L_GET）"
else
    bad "S1a 值域校验不在主路径前（块=${L_A:-?} 取值=${L_GET:-?}）"
fi
miss=""
for k in max_body_size max_conn gzip_level gzip_min_bytes rate_limit http3; do
    grep -q "px_opt_expect_.*(\"$k" "$RT" || miss="$miss $k"
done
[ -z "$miss" ] && ok "S1b 6 个关键键各有值域校验调用" || bad "S1b 缺校验的键：$miss"
w1="$(grep -c 'opts.max_body_size=%lld 小于下限 1024，已夹到 1024' "$RT")"
w2="$(grep -c 'opts.max_conn=%lld 超过上限' "$RT")"
w3="$(grep -c 'opts.rate_limit.whitelist 有 %d 项，超过上限' "$RT")"
if [ "$w1" -ge 1 ] && [ "$w2" -ge 1 ] && [ "$w3" -ge 1 ]; then
    ok "S2 B 类三条夹取路径都有 stderr 留痕"
else
    bad "S2 夹取留痕缺失（mb=$w1 conn=$w2 wl=$w3）"
fi

echo "== P1 负控（安全面）：rate_limit.window_sec 越界必须响亮 =="
P1="$(free_port)"
printf 'print("before")\npx_serve(%s, "%s", 1000, {"rate_limit": {"max": 3, "window_sec": -5}})\nprint("after")\n' "$P1" "$TMP/www" | build rl_neg >/dev/null || bad "P1 构建失败"
run_err rl_neg
if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && err_range_ok "$OUT" 'rate_limit.window_sec'; then
    ok "P1 window_sec=-5 rc=$RC 且报出键名+区间+实收值"
else
    bad "P1 未被响亮拒绝（rc=$RC out=$(printf '%s' "$OUT" | head -1 | cut -c1-100)）"
fi

echo "== P2/N1 正控：合法 rate_limit 必须**真限流** =="
P2="$(free_port)"
printf 'px_serve(%s, "%s", 1000, {"rate_limit": {"max": 3, "window_sec": 60}})\n' "$P2" "$TMP/www" | build rl_pos >/dev/null || bad "P2 构建失败"
SEQ2="$(codes rl_pos "$P2" 6)"
case "$SEQ2" in
    *"200 200 200 429 429 429"*) ok "P2 合法限流生效（$SEQ2 ）" ;;
    *) bad "P2 限流未按 max=3 生效（$SEQ2 ）" ;;
esac
P2B="$(free_port)"
printf 'px_serve(%s, "%s", 1000, {"max_conn": 8})\n' "$P2B" "$TMP/www" | build rl_off >/dev/null || bad "P2b 构建失败"
SEQ2B="$(codes rl_off "$P2B" 6)"
case "$SEQ2B" in
    *"429"*) bad "P2b 对照进程也出 429 ⇒ P2 的 429 不是限流贡献的（$SEQ2B ）" ;;
    *200*) ok "P2b 对照（无 rate_limit）无 429 ⇒ 429 来自限流本身（$SEQ2B ）" ;;
    *) bad "P2b 对照进程起不来（$SEQ2B ）" ;;
esac

echo "== P3 负控：五个越界值逐个必须响亮 =="
p3_case() {  # $1=名字 $2=opts 文本 $3=期望键名
    local n="$1" o="$2" k="$3" p
    p="$(free_port)"
    printf 'px_serve(%s, "%s", 1000, {%s})\n' "$p" "$TMP/www" "$o" | build "$n" >/dev/null || { bad "P3 $n 构建失败"; return; }
    run_err "$n"
    if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && err_range_ok "$OUT" "$k"; then
        ok "P3 $n rc=$RC 响亮"
    else
        bad "P3 $n 未响亮（rc=$RC）"
    fi
}
p3_case neg_conn  '"max_conn": 0'          max_conn
p3_case neg_gzip  '"gzip_level": 99'       gzip_level
p3_case neg_gmin  '"gzip_min_bytes": 0'    gzip_min_bytes
p3_case neg_h3p   '"http3": {"port": 70000}' http3.port
p3_case neg_rlmax '"rate_limit": {"max": 0, "window_sec": 60}' rate_limit.max

echo "== P4 正控（防过度修复）：边界合法值必须照常起服务 =="
p4_case() {  # $1=名字 $2=opts 文本
    local n="$1" o="$2" p
    p="$(free_port)"
    printf 'px_serve(%s, "%s", 1000, {%s})\n' "$p" "$TMP/www" "$o" | build "$n" >/dev/null || { bad "P4 $n 构建失败"; return; }
    if serve_ok "$n" "$p"; then ok "P4 $n 服务 GET 200"; else bad "P4 $n 被误杀（$(tail -1 "$TMP/$n.srv.log" 2>/dev/null | cut -c1-90)）"; fi
}
p4_case pos_conn1  '"max_conn": 1'
p4_case pos_gl1    '"gzip_level": 1'
p4_case pos_gl9    '"gzip_level": 9'
p4_case pos_gmin1  '"gzip_min_bytes": 1'
p4_case pos_mb1024 '"max_body_size": 1024'
p4_case pos_rl1    '"rate_limit": {"max": 1, "window_sec": 1}'
# http3 正控：本二进制可能以 --no-quic 构建 ⇒ 要么真起 H3，要么 R1002 说明原因（两者都算「不静默」）
PH="$(free_port)"
printf 'px_serve(%s, "%s", 1000, {"http3": {"port": %s}})\n' "$PH" "$TMP/www" "$PH" | build pos_h3 >/dev/null || bad "P4 pos_h3 构建失败"
( cd "$TMP" && exec "./build/pos_h3" ) >"$TMP/pos_h3.srv.log" 2>&1 &
HPID=$!; sleep 0.6
kill "$HPID" 2>/dev/null; wait "$HPID" 2>/dev/null
if grep -q 'HTTP/3 listening udp/'"$PH" "$TMP/pos_h3.srv.log"; then
    ok "P4 pos_h3 边界端口 $PH 真起 H3"
elif grep -q 'no-quic' "$TMP/pos_h3.srv.log"; then
    ok "P4 pos_h3 本二进制 no-quic ⇒ R1002 说明原因（不静默）"
else
    bad "P4 pos_h3 既不启 H3 也不报错（$(tail -1 "$TMP/pos_h3.srv.log" | cut -c1-90)）"
fi

echo "== P5 B 类留痕：夹取必须可见 =="
PB="$(free_port)"
printf 'px_serve(%s, "%s", 1000, {"max_body_size": 500})\n' "$PB" "$TMP/www" | build b_mb >/dev/null || bad "P5 构建失败(mb)"
if serve_ok b_mb "$PB" && warn_ok "$(cat "$TMP/b_mb.srv.log")" '已夹到 1024'; then
    ok "P5a max_body_size=500 服务起 + stderr 留痕"
else
    bad "P5a max_body_size=500 无留痕（$(grep -c 警告 "$TMP/b_mb.srv.log" 2>/dev/null) 条警告）"
fi
WL="$(for i in $(seq 1 70); do printf '"10.1.%d.%d",' $((i / 250)) $((i % 250 + 1)); done)"
WL="${WL%,}"
PW="$(free_port)"
printf 'px_serve(%s, "%s", 1000, {"rate_limit": {"max": 5, "window_sec": 60, "whitelist": [%s]}})\n' "$PW" "$TMP/www" "$WL" | build b_wl >/dev/null || bad "P5 构建失败(wl)"
if serve_ok b_wl "$PW" && warn_ok "$(cat "$TMP/b_wl.srv.log")" '超过上限 64'; then
    ok "P5b whitelist 70 项 ⇒ 截断 + stderr 留痕"
else
    bad "P5b whitelist 超限无留痕"
fi

echo "== P7/P8 类型错：修前全静默，现在必须响亮 =="
p7_case() {  # $1=名字 $2=opts 文本 $3=期望键名
    local n="$1" o="$2" k="$3" p
    p="$(free_port)"
    printf 'px_serve(%s, "%s", 1000, {%s})\n' "$p" "$TMP/www" "$o" | build "$n" >/dev/null || { bad "P7 $n 构建失败"; return; }
    run_err "$n"
    if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && err_type_ok "$OUT" "$k"; then
        ok "$n rc=$RC 类型错响亮"
    else
        bad "$n 类型错未响亮（rc=$RC）"
    fi
}
p7_case ty_rl   '"rate_limit": "3/60"'                          rate_limit
p7_case ty_conn '"max_conn": "8"'                               max_conn
p7_case ty_wl   '"rate_limit": {"max": 5, "window_sec": 60, "whitelist": "127.0.0.1"}' rate_limit.whitelist
p7_case ty_item '"rate_limit": {"max": 5, "window_sec": 60, "whitelist": ["127.0.0.1", 42]}' rate_limit.whitelist

echo "== P6 负控自证：把 A 类 int 校验打掉，P1 的信号必须消失 =="
cp -p "$RT" "$RT_BAK"
RT_PATCHED=1
if python3 - "$RT" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
anchor = 'static void px_opt_expect_int(const char* key, LXValue v, long long lo, long long hi, const char* range) {'
assert s.count(anchor) == 1, "锚点计数=%d" % s.count(anchor)
open(p, 'w', encoding='utf-8').write(s.replace(anchor, anchor + '\n    if (1) return;   /* M428 gate P6：临时打掉 */', 1))
PY
then
    if ! grep -q 'M428 gate P6：临时打掉' "$RT"; then bad "P6a 前提不成立：补丁未落到文件上"; fi
    p="$(free_port)"
    printf 'px_serve(%s, "%s", 1000, {"rate_limit": {"max": 3, "window_sec": -5}})\nprint("started")\n' "$p" "$TMP/www" | build_nocache patched >/dev/null || bad "P6 补丁树构建失败"
    if grep -q '缓存命中' "$TMP/patched.build.log" 2>/dev/null; then
        bad "P6a 前提不成立：补丁树命中了构建缓存"
    else
        ok "P6a 前提自证：补丁树真重建（NOCACHE）"
    fi
    RC=0; OUT="$( cd "$TMP" && timeout 3 "./build/patched" 2>&1 )" || RC=$?
    if [ "$RC" -eq 124 ] && ! err_range_ok "$OUT" 'rate_limit.window_sec'; then
        ok "P6 打掉校验后同一份源码不再报错（rc=124 阻塞）⇒ P1 的红来自这条校验"
    else
        bad "P6 打掉校验后仍报错（rc=$RC）⇒ P1 可能走了旁路"
    fi
else
    bad "P6 打补丁失败（px_opt_expect_int 锚点不唯一）"
fi
cp -p "$RT_BAK" "$RT"; RT_PATCHED=""
if [ "$(sha256sum "$RT" | cut -d' ' -f1)" = "$(sha256sum "$RT_BAK" | cut -d' ' -f1)" ]; then
    ok "P6b runtime.c 已原样复原（sha256 相同）"
else
    bad "P6b runtime.c 复原失败"; exit 3
fi

echo "== G2 零污染 =="
ST_AFTER="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"
if [ "$ST_BEFORE" = "$ST_AFTER" ]; then
    ok "G2 仓库状态行数不变（$ST_BEFORE ⇒ $ST_AFTER）"
else
    bad "G2 仓库被污染（$ST_BEFORE ⇒ $ST_AFTER）"
fi

echo "── 小计：PASS $PASS / FAIL $FAIL"
[ "$FAIL" = 0 ] || exit 1
echo "OPTS-VALUERANGE OK"
exit 0
