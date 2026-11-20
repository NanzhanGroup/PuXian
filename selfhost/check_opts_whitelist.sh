#!/usr/bin/env bash
# ============================================================
# selfhost/check_opts_whitelist.sh —— px_serve 的 opts 键名白名单（M427 · 缺陷 874）
# ------------------------------------------------------------
# 为什么有它（2026-11-21 · 源码 + 最小复现实测）：
#   `px_serve(port, docroot, timeout, opts)` 的第 4 参是用户唯一的服务配置面，
#   而 runtime 对它**逐键 px_dict_get 猜测**：认得的键生效、不认得的键**静默忽略**。
#   实测（修前形状）：`{"max_body_sise": 2048}` ⇒ 不报错，服务照起，配置不生效。
#   同仓反例证明口径本来是「响亮优于静默」：
#     http_stream/ws_stream 的 opts.methods 含未知方法 ⇒ R1002
#     px_serve 的 opts.http3 在 --no-quic 产物上 ⇒ R1002（M180）
#   ⇒ 唯独「opts 的**键名**」这一层没有保护。
#
# 判据（每条都要能判红）：
#   S1 形状：runtime.c 有键表常量，且在第一个 px_dict_get(args[3], …) **之前**做校验
#   S2 形状：键表恰 14 个且与契约集合一致；未知键分支走 px_error 且文本含 R1002 + 全部键名
#   P1 负控（核心）：传 "max_body_sise" ⇒ rc≠0，输出含该键名与正确键名 "max_body_size"
#   P2 负控：传 "foo" ⇒ rc≠0
#   P3 正控（防过度修复，最关键）：14 个已知键**逐个单传** ⇒ 服务真起来（一次真实 GET 200）
#   P4 正控：{} 与不传第 4 参 ⇒ 不报错且服务起
#   P5 负控：未知键配 null 值 ⇒ 同样报（键名检查与值无关）
#   P6 负控的自证：把校验打掉（if (0 && !known)）⇒ 同一份 typo 源码必须**不再报错**（服务起来）
#        —— 证明 P1 的红来自这条校验，不是别处顺手报的
#   N1 前提自证：P3 的「服务起来」用真实 GET 200 自证（进程活着 ≠ 服务对）
#   G1 环境守卫：tools/px / cc / curl 不可用，或探针构建失败 ⇒ rc=3（绝不降级成 0）
#   G2 零污染：全程在 $TMP 构建，仓库 git status 行数前后一致（M426 缺陷 873 的教训）
#
# 用法：check_opts_whitelist.sh             # 全跑（约 80~140s，含一次 runtime 重建）
#       check_opts_whitelist.sh --self-test # 只跑判据夹具（不编译，离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m427_opts.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

# 契约键面（与 runtime.c 的键表必须逐项相等 —— S2 用 set 比较，不靠顺序）
KEYS="max_body_size body_spill max_conn rate_limit access_log alt_svc log_json log_daily gzip_level gzip_min_bytes gzip h2_demo http3 reuse_port"

RT_BAK="$TMP/runtime.c.bak"
RT_PATCHED=""
cleanup() {
    # P6 会临时改 runtime.c：**任何退出路径**都必须复原（M425 P16 的教训）
    if [ -n "${RT_PATCHED:-}" ] && [ -f "$RT_BAK" ]; then
        cp -p "$RT_BAK" "$RT" 2>/dev/null
    fi
    chmod -R u+w "$TMP" 2>/dev/null
    rm -rf "$TMP"
}
trap cleanup EXIT

# ── 判据纯函数（可自测）────────────────────────────────────
extract_keys() {  # $1=runtime.c 路径 ⇒ 一行一个键名
    sed -n '/static const char\* const px_opts_keys\[\]/,/NULL/p' "$1" 2>/dev/null \
        | grep -o '"[a-z_0-9]*"' | tr -d '"'
}

keyset_match() {  # $1=runtime.c ⇒ 0 表与契约集合相等
    local got
    got="$(extract_keys "$1" | sort | tr '\n' ' ')"
    local want; want="$(printf '%s\n' $KEYS | sort | tr '\n' ' ')"
    [ "$got" = "$want" ]
}

err_ok() {  # $1=输出文本 ⇒ 0 表示「未知键被响亮拒绝」的形状成立
    case "$1" in
        *R1002*) ;;
        *) return 1 ;;
    esac
    case "$1" in
        *不认识的键*) ;;
        *) return 1 ;;
    esac
    case "$1" in
        *max_body_size*) ;;
        *) return 1 ;;
    esac
    # 至少列出 10 个已知键（防止「只报键名、不给出路」）
    local n=0 k
    for k in $KEYS; do
        case "$1" in *"$k"*) n=$((n+1)) ;; esac
    done
    [ "$n" -ge 10 ]
}

selftest() {
    local st=0 d t
    d="$(mktemp -d "${TMPDIR:-/tmp}/m427_st.XXXXXX")"
    echo "── 自测（判据夹具，全离线）"
    # 正控 1：真实 runtime.c 的键表必须与契约相等
    if keyset_match "$RT"; then echo "  PASS S-pos1 真实键表与契约相等"; else echo "  FAIL S-pos1 键表不匹配"; st=1; fi
    # 负控 1：删掉一个键的副本必须不匹配
    cp -f "$RT" "$d/rt1.c"
    python3 - "$d/rt1.c" <<'PY'
import sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
i=s.find('"max_body_size", "body_spill"')
assert i>0
s=s[:i]+'"body_spill"'+s[i+len('"max_body_size", "body_spill"'):]
open(p,'w',encoding='utf-8').write(s)
PY
    if keyset_match "$d/rt1.c"; then echo "  FAIL S-neg1 删键未被发现"; st=1; else echo "  PASS S-neg1 删键被检出"; fi
    # 正控 2：真实的报错形状必须被 err_ok 接受
    if err_ok '运行时错误 [<top> 行2]: R1002: px_serve opts 里不认识的键 "x"（拼错？已知键：max_body_size / body_spill / max_conn / rate_limit / access_log / alt_svc / log_json / log_daily / gzip_level / gzip_min_bytes / gzip / h2_demo / http3 / reuse_port）'
    then echo "  PASS P-pos2 报错形状被接受"; else echo "  FAIL P-pos2 报错形状被误拒"; st=1; fi
    # 负控 2/3：静默输出、以及「报了键名但没给已知键出口」
    if err_ok 'before
after'; then echo "  FAIL P-neg2 静默输出被当成报错"; st=1; else echo "  PASS P-neg2 静默输出被拒"; fi
    if err_ok 'R1002: px_serve opts 里不认识的键 "x"'; then echo "  FAIL P-neg3 无出口被接受"; st=1; else echo "  PASS P-neg3 无已知键出口被拒"; fi
    # 前提自证：夹具确实作用在真实文件上（扫描面不为空）
    t="$(extract_keys "$RT" | wc -l)"
    if [ "$t" -ge 14 ]; then echo "  PASS N1 前提自证 键表扫描面=$t"; else echo "  FAIL N1 扫描面为空($t)"; st=1; fi
    rm -rf "$d"
    [ "$st" = 0 ] && echo "SELF-TEST OK" || echo "SELF-TEST FAIL"
    return "$st"
}

if [ "${1:-}" = "--self-test" ]; then
    selftest || exit 2
    exit 0
fi

# ── G1 环境守卫 ────────────────────────────────────────────
[ -f "$PX" ] || { echo "❌ 缺 tools/px" >&2; exit 3; }
command -v curl >/dev/null 2>&1 || { echo "❌ 缺 curl（P3 需要真实 GET）" >&2; exit 3; }
command -v cc >/dev/null 2>&1 || { echo "❌ 缺 cc" >&2; exit 3; }
[ -f "$RT" ] || { echo "❌ 缺 runtime/runtime.c" >&2; exit 3; }
ST_BEFORE="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"

# ── 构建/运行助手（全部在 $TMP，零污染）────────────────────
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

RC=0; OUT=""
run_err() {  # $1=名字 ⇒ RC/OUT
    RC=0
    OUT="$( cd "$TMP" && timeout 5 "./build/$1" 2>&1 )" || RC=$?
}

free_port() {
    local p
    for p in $(seq 18320 18399); do
        ( exec 3<>"/dev/tcp/127.0.0.1/$p" ) 2>/dev/null || { echo "$p"; return 0; }
        exec 3>&- 2>/dev/null
    done
    return 1
}

serve_ok() {  # $1=二进制名 $2=端口 ⇒ 真实 GET 200 才算「服务起来」
    ( cd "$TMP" && exec "./build/$1" ) >"$TMP/$1.srv.log" 2>&1 &
    local pid=$! i code=""
    for i in 1 2 3 4 5 6 7 8 9 10; do
        code="$(curl -s -o /dev/null -m 1 -w '%{http_code}' "http://127.0.0.1:$2/" 2>/dev/null || true)"
        [ "$code" = "200" ] && break
        sleep 0.2
    done
    kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
    [ "$code" = "200" ]
}

# 站点根（native 静态路径）
mkdir -p "$TMP/www" && printf 'ok\n' > "$TMP/www/index.html"

# 冒烟构建（环境不成立宁可 rc=3）
if ! printf 'print("smoke")\n' | build smoke >/dev/null 2>&1; then
    echo "❌ 探针构建失败（环境不成立）" >&2; exit 3
fi

echo "== S 源码形状 =="
if grep -q 'static const char\* const px_opts_keys\[\]' "$RT"; then
    ok "S1a 键表常量存在"
else
    bad "S1a 缺键表常量"
fi
L_TABLE="$(grep -n 'static const char\* const px_opts_keys\[\]' "$RT" | head -1 | cut -d: -f1)"
L_FIRST_GET="$(grep -n 'px_dict_get(args\[3\]' "$RT" | head -1 | cut -d: -f1)"
if [ -n "$L_TABLE" ] && [ -n "$L_FIRST_GET" ] && [ "$L_TABLE" -lt "$L_FIRST_GET" ]; then
    ok "S1b 校验在第一个 opts 取值之前（表 L$L_TABLE < 取值 L$L_FIRST_GET）"
else
    bad "S1b 校验不在主路径前（表=${L_TABLE:-?} 取值=${L_FIRST_GET:-?}）"
fi
if keyset_match "$RT"; then
    ok "S2a 键表恰 14 键且与契约集合相等"
else
    bad "S2a 键表与契约不相等（实际：$(extract_keys "$RT" | tr '\n' ' ')）"
fi
if grep -q 'R1002: px_serve opts 里不认识的键' "$RT" && grep -q 'px_opts_rl_keys' "$RT"; then
    ok "S2b 未知键分支走 px_error(R1002) 且 rate_limit 子字典同办"
else
    bad "S2b 未知键分支/子字典校验缺失"
fi

echo "== P1/P2/P5 负控：拼错与未知键必须响亮 =="
P1V="$(free_port)"; printf 'print("before")\npx_serve(%s, "%s", 1000, {"max_body_sise": 2048})\nprint("after")\n' "$P1V" "$TMP/www" | build typo1 >/dev/null || bad "P1 构建失败"
run_err typo1
if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && err_ok "$OUT"; then
    ok "P1 拼错键 rc=$RC 且报出键名与正确键名"
else
    bad "P1 拼错键未响亮（rc=$RC out=$(printf '%s' "$OUT" | head -2 | tr '\n' '|')）"
fi
P2V="$(free_port)"; printf 'px_serve(%s, "%s", 1000, {"foo": 1})\n' "$P2V" "$TMP/www" | build typo2 >/dev/null || bad "P2 构建失败"
run_err typo2
if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && err_ok "$OUT"; then
    ok "P2 未知键 foo rc=$RC"
else
    bad "P2 未知键 foo 未响亮（rc=$RC）"
fi
P5V="$(free_port)"; printf 'px_serve(%s, "%s", 1000, {"max_body_sise": null})\n' "$P5V" "$TMP/www" | build typo5 >/dev/null || bad "P5 构建失败"
run_err typo5
if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ] && err_ok "$OUT"; then
    ok "P5 未知键配 null 值同样报（键名检查与值无关）"
else
    bad "P5 未知键配 null 未响亮（rc=$RC）"
fi

echo "== P3/N1 正控：14 个已知键逐个单传，服务必须真起来 =="
val_of() {
    case "$1" in
        max_body_size) echo "2048" ;;
        body_spill) echo "true" ;;
        max_conn) echo "8" ;;
        rate_limit) echo '{"max": 50, "window_sec": 60, "key": "ip", "whitelist": ["127.0.0.1"]}' ;;
        access_log) echo "\"$TMP/www/access.log\"" ;;
        alt_svc) echo '"h3=443"' ;;
        log_json) echo "true" ;;
        log_daily) echo "true" ;;
        gzip_level) echo "6" ;;
        gzip_min_bytes) echo "512" ;;
        gzip) echo "true" ;;
        h2_demo) echo "false" ;;
        http3) echo "false" ;;
        reuse_port) echo "true" ;;
    esac
}
i=0
for k in $KEYS; do
    i=$((i+1))
    v="$(val_of "$k")"; p="$(free_port)"
    if [ -z "$p" ]; then bad "P3 $k 无空闲端口"; continue; fi
    if ! printf 'px_serve(%s, "%s", 1000, {%s: %s})\nprint("served")\n' "$p" "$TMP/www" "\"$k\"" "$v" | build "key_$k" >/dev/null; then
        bad "P3 $k 构建失败"; continue
    fi
    if serve_ok "key_$k" "$p"; then
        ok "P3/$i $k 单传 ⇒ 服务 GET 200"
    else
        bad "P3/$i $k 被误杀（srv.log: $(tail -1 "$TMP/key_$k.srv.log" 2>/dev/null | cut -c1-80)）"
    fi
done

echo "== P4 正控：{} 与不传第 4 参 =="
p="$(free_port)"; printf 'px_serve(%s, "%s", 1000, {})\n' "$p" "$TMP/www" | build empty >/dev/null || bad "P4 构建失败(空字典)"
serve_ok empty "$p" && ok "P4a {} 起服务" || bad "P4a {} 起不来"
p="$(free_port)"; printf 'px_serve(%s, "%s", 1000)\n' "$p" "$TMP/www" | build noopt >/dev/null || bad "P4 构建失败(无 opts)"
serve_ok noopt "$p" && ok "P4b 不传第 4 参起服务" || bad "P4b 不传第 4 参起不来"

echo "== P6 负控自证：把校验打掉，P1 的信号必须消失 =="
cp -p "$RT" "$RT_BAK"
RT_PATCHED=1
# ⚠ 锚点必须**唯一且落在 px_serve 内**：runtime.c 里另有一处 `if (!known)`
#   （4530 · 枚举变体 R1008）。第一版用裸 `if (!known)` 替换 ⇒ 打到了枚举那处，
#   于是「补丁树仍报错」被误判成 P1 旁路（2026-11-21 实测踩到 —— 判据的假红先怀疑判据自己）。
if python3 - "$RT" <<'PY'
import sys
p=sys.argv[1]
s=open(p,encoding='utf-8').read()
anchor='if (!known)\n                px_error("R1002: px_serve opts'
assert s.count(anchor)==1, "锚点不唯一：%d" % s.count(anchor)
open(p,'w',encoding='utf-8').write(s.replace(anchor,'if (0 && !known)\n                px_error("R1002: px_serve opts',1))
PY
then
    if ! grep -q 'if (0 && !known)' "$RT"; then bad "P6a 前提不成立：补丁未落到文件上"; fi
    p="$(free_port)"
    printf 'px_serve(%s, "%s", 1000, {"max_body_sise": 2048})\nprint("started")\n' "$p" "$TMP/www" | build patched >/dev/null || bad "P6 补丁树构建失败"
    # 前提自证：补丁树必须**真重建**（命中缓存 ⇒ 跑的还是没补丁的产物，判据空转）
    if grep -q '缓存命中' "$TMP/patched.build.log" 2>/dev/null; then
        bad "P6a 前提不成立：补丁树构建命中了缓存"
    else
        ok "P6a 前提自证：补丁树真重建（未命中构建缓存）"
    fi
    RC=0; OUT="$( cd "$TMP" && timeout 3 "./build/patched" 2>&1 )" || RC=$?
    if [ "$RC" -eq 124 ] && ! err_ok "$OUT"; then
        ok "P6 校验打掉后 typo 不再报错（rc=124 阻塞）⇒ P1 的红来自这条校验"
    else
        bad "P6 校验打掉后仍报错（rc=$RC）⇒ P1 不是踩这条路径"
    fi
else
    bad "P6 打补丁失败（px_serve 锚点不唯一或找不到）"
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
echo "OPTS-WHITELIST OK"
exit 0
