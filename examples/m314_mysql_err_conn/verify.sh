#!/usr/bin/env bash
# ============================================================
# examples/m314_mysql_err_conn/verify.sh
# ------------------------------------------------------------
# M314（第 185 轮 · 缺陷 541/542）：**mysql 驱动的错误语义分流**
#
# 来历（半山 2026-10-20 总线【缺陷提单】m18e01cc5c477865c9f35426b）：
#   实测 `SELECT no_such_col FROM juans` ⇒ `mysql err 1054 ... (connection closed)`
#   ⇒ 之后同一连接再查一律 `connection already closed`
#   ⇒ **一条写错的 SQL 就废掉整条连接**，应用必须自己重连兜底。
#
#   根因：M313 的 my_conn_bad() 对**所有** Err 一律 tcp_close + fd=-1，但两类错误语义完全不同：
#     · 服务器 ERR 包（payload 首字节 255）：包完整、seq 对齐、**连接健康**，只是本条语句失败
#     · IO 错（read/send 失败）：流已错位 ⇒ 必须废连接
#   M313 把 IO 错的处置补对了，但顺带把 ERR 包也归进了同一条路径。
#
#   同族缺口（缺陷 542）：mysql_ping / mysql_select_db 的失败路径只 return Err，
#   既不关连接也不置 fd=-1 ⇒ my_conn_check 仍判「健康」⇒ 调用方拿一根已错位的 fd 继续用。
#
# 判据层次：
#   [0] 前置：本门复用 m313 的 mock 服务端与探针语料（必须在场 —— 缺失即判红，不静默跳过）
#   [1] 静态：错误分类原语齐备（两版本）+ 前缀**单源**（格式串 ⇄ 常量对拍）+ 分流**先于** tcp_close
#   [2] 构建（mock + 两版本探针；驱动复制进 W，仓库零写入）
#   [3] 动态主判据（0.2.0）：OK1 → BAD(1054，保连接) → OK2 → FATAL(1053，关连接) → already closed
#   [4] 动态（0.1.0）：同一份代码同形态（无 ping/select_db，其余必须一致）
#   [5] 动态（ping / select_db 成功路径）
#   [6] 动态（ping 超时 ⇒ 必须关连接，不留半连接 —— 缺陷 542 的回归位）
#   [7] 负控 A：撤回 my_conn_bad 的分流 ⇒ **BAD 必须把连接关掉**（= 复现半山报障的形态）
#   [8] 负控 B：让 my_err_fatal 恒 false ⇒ **FATAL(1053) 不得关连接**（证明致命码表有牙）
#   [9] 覆盖边界（如实）
#
# ⚠️ 判据只用**产品自带输出**（驱动返回的 Err 文本 + mock 的日志行），不依赖插桩。
# ⚠️ 负控改的是 W 里的驱动副本 ⇒ 仓库零写入；仍 source gate_lock.sh（M276）。
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
SRC="$ROOT/examples/m313_mysql_driver"
W="${M314_GATE_W:-$(mktemp -d /tmp/m314_gate.XXXXXX)}"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

D020="$ROOT/registry/mysql/0.2.0/mysql.px"
D010="$ROOT/registry/mysql/0.1.0/mysql.px"
P020="$ROOT/registry/mysql/0.2.0/proto.px"
P010="$ROOT/registry/mysql/0.1.0/proto.px"
MOCK_PID=""

cleanup() {
    if [ -n "$MOCK_PID" ] && kill -0 "$MOCK_PID" 2>/dev/null; then kill "$MOCK_PID" 2>/dev/null; fi
    [ "${KEEP_W:-0}" = "1" ] || rm -rf "$W"
}
trap cleanup EXIT

strip_comments() { sed -e 's/[[:space:]]*#.*$//' "$1"; }
port_new() { echo $((20000 + RANDOM % 500)); }
# 抽函数体（到下一个 def / 注释行 / 文件尾）—— 判据要看「函数内」而不是「文件里第一次出现」
fn_body() {   # $1=file $2=函数名
    awk -v n="$2" '
        index($0, "def " n "(") == 1 { f=1; print; next }
        f && (/^def / || /^#/) { f=0 }
        f { print }
    ' "$1"
}

echo "══ M314 · mysql 驱动：服务器 ERR 包 ⇄ IO 错 的分流（缺陷 541/542）══"

# ── [0] 前置：复用语料必须在场（M285：判据要在场，缺了要响亮）────────
echo "── [0] 前置：复用 m313 的 mock / 探针语料"
MISS0=0
for f in mock_mysql.px probe.px; do
    if [ -f "$SRC/$f" ]; then ok "[0.1] 语料在场：examples/m313_mysql_driver/$f"; else bad "[0.1] 语料缺失：$SRC/$f"; MISS0=1; fi
done
[ "$MISS0" = "1" ] && { echo "══ 前置缺失，后续无意义 ══"; echo "══ M314-VERIFY-FAIL · $PASS 通过 / $FAIL 失败 ══"; exit 1; }
# 语料必须已含 M314 的新模式（否则说明用到了旧副本）
grep -q 'errcase' "$SRC/mock_mysql.px" && ok "[0.2] mock 支持 errcase 模式" || bad "[0.2] mock 缺 errcase 模式（语料过旧？）"
grep -q 'PROBE-ERR-KEEP-OK' "$SRC/probe.px" && ok "[0.3] probe 含 M314 判据标记" || bad "[0.3] probe 缺 M314 判据标记"

# ── [1] 静态 ─────────────────────────────────────────────────
echo "── [1] 静态：分类原语齐备 · 前缀单源 · 分流先于关闭（去注释后判）"
for f in "$P020" "$P010"; do
    tag="$(basename "$(dirname "$f")")"
    code="$W/code_proto_$tag.txt"; strip_comments "$f" > "$code"
    chk "[1.1] $tag 分类原语齐备（is_server / code / fatal）" \
        "$(grep -cE '^def my_err_(is_server|code|fatal)\(' "$code")" "3"
    chk "[1.2] $tag 前缀常量存在且唯一" "$(grep -c '^var MY_ERR_PREFIX = "mysql err "' "$code")" "1"
    # 前缀必须**声明在使用之前**（避免缺陷 193「先读后声明」）
    LP=$(grep -n '^var MY_ERR_PREFIX' "$code" | cut -d: -f1)
    LU=$(grep -n 'MY_ERR_PREFIX + str(code)' "$code" | cut -d: -f1)
    if [ -n "$LP" ] && [ -n "$LU" ] && [ "$LP" -lt "$LU" ]; then ok "[1.3] $tag 前缀声明先于使用（$LP < $LU）"; else bad "[1.3] $tag 前缀声明顺序不对（LP=$LP LU=$LU）"; fi
    # 前缀单源：不得再出现硬编码的 "mysql err "
    chk "[1.4] $tag 无硬编码 \"mysql err \"（只走常量）" "$(grep -c '"mysql err "' "$code")" "1"
    # 致命码表 6 个码（按**出现次数**数，不是按行数 —— 一行里就有 4 个）
    NFAT=$(grep -oE 'code == (1053|1927|3169|4031|2006|2013)' "$code" | wc -l)
    chk "[1.5] $tag 致命码表含 6 个码" "$NFAT" "6"
done
cmp -s "$P010" "$P020" && ok "[1.6] 两版本 proto.px 逐字节相同（上游本来同源）" || bad "[1.6] 两版本 proto.px 不一致"

for f in "$D020" "$D010"; do
    tag="$(basename "$(dirname "$f")")"
    code="$W/code_mysql_$tag.txt"; strip_comments "$f" > "$code"
    chk "[1.7] $tag my_conn_bad 存在" "$(grep -c '^def my_conn_bad(conn, what):' "$code")" "1"
    chk "[1.7b] $tag 分流判据在位（is_server && !fatal ⇒ 直接 Err）" \
        "$(grep -c 'if my_err_is_server(what) and not my_err_fatal(my_err_code(what)):' "$code")" "1"
    # 分流必须**先于** tcp_close（顺序错 = 修了等于没修）
    # ⚠️ 必须在 **my_conn_bad 的函数体内**比较 —— 文件里第一个 tcp_close 在 mysql_connect 里
    fn_body "$code" my_conn_bad > "$W/body_connbad_$tag.txt"
    LSPLIT=$(grep -n 'if my_err_is_server(what) and not my_err_fatal' "$W/body_connbad_$tag.txt" | cut -d: -f1)
    LCLOSE=$(grep -n 'tcp_close(fd)' "$W/body_connbad_$tag.txt" | head -1 | cut -d: -f1)
    if [ -n "$LSPLIT" ] && [ -n "$LCLOSE" ] && [ "$LSPLIT" -lt "$LCLOSE" ]; then ok "[1.8] $tag 分流先于 tcp_close（$LSPLIT < $LCLOSE）"; else bad "[1.8] $tag 顺序不对（split=$LSPLIT close=$LCLOSE）"; fi
done
# 0.2.0 的 ping / select_db 必须走 my_conn_bad（缺陷 542）
code020="$W/code_mysql_0.2.0.txt"
chk "[1.9] 0.2.0 ping/select_db 的 send 失败走 my_conn_bad" \
    "$(grep -cE 'return my_conn_bad\(conn, "(ping|select_db) send: "' "$code020")" "2"
chk "[1.10] 0.2.0 ping/select_db 的 read 失败走 my_conn_bad" \
    "$(( $(fn_body "$code020" mysql_ping | grep -c 'return my_conn_bad(conn, r.err())') + $(fn_body "$code020" mysql_select_db | grep -c 'return my_conn_bad(conn, r.err())') ))" "2"
chk "[1.11] 0.2.0 ping/select_db 都有 my_conn_check 入口检查" \
    "$(( $(fn_body "$code020" mysql_ping | grep -c 'var ck = my_conn_check(conn)') + $(fn_body "$code020" mysql_select_db | grep -c 'var ck = my_conn_check(conn)') ))" "2"
L=$(wc -l < "$D020"); M=$(wc -l < "$D010")
if [ "$L" -ge 470 ] && [ "$M" -ge 420 ]; then ok "[1.12] 规模锚点（0.2.0=$L 行 · 0.1.0=$M 行）"; else bad "[1.12] 规模锚点：0.2.0=$L / 0.1.0=$M"; fi

# ── [2] 构建 ─────────────────────────────────────────────────
echo "── [2] 构建（mock + 两版本探针；驱动复制进 W，仓库零写入）"
mk_driver_copy() { mkdir -p "$2" && cp -f "$1"/*.px "$2"/ 2>/dev/null; }
mk_driver_copy "$ROOT/registry/mysql/0.2.0" "$W/drv020"
mk_driver_copy "$ROOT/registry/mysql/0.1.0" "$W/drv010"
cp -f "$SRC/mock_mysql.px" "$W/mock_mysql.px"
sed "s|\.\./\.\./registry/mysql/0\.2\.0/mysql\.px|$W/drv020/mysql.px|" "$SRC/probe.px" > "$W/probe020.px"
sed "s|\.\./\.\./registry/mysql/0\.2\.0/mysql\.px|$W/drv010/mysql.px|" "$SRC/probe.px" > "$W/probe010.px"
BUILD_OK=1
for f in "$W/mock_mysql.px" "$W/probe020.px" "$W/probe010.px"; do
    if ! out=$("$ROOT/tools/px" build "$f" 2>&1); then echo "$out" | tail -3; BUILD_OK=0; fi
done
chk "[2.1] 三个产物构建成功" "$BUILD_OK" "1"
MOCK="$W/build/mock_mysql"; P020B="$W/build/probe020"; P010B="$W/build/probe010"
for b in "$MOCK" "$P020B" "$P010B"; do [ -x "$b" ] || { bad "[2.2] 产物缺失：$b"; BUILD_OK=0; }; done
[ "$BUILD_OK" = "1" ] && ok "[2.2] 产物齐备"

run_mock() {   # $1=port $2=mode $3=log
    : > "$3"
    "$MOCK" "$1" "$2" > "$3" 2>&1 &
    MOCK_PID=$!
    local i
    for i in $(seq 1 200); do grep -q 'MOCK-READY' "$3" 2>/dev/null && break; sleep 0.05; done
    for i in $(seq 1 100); do ss -ltn 2>/dev/null | grep -q ":$1 " && return 0; sleep 0.05; done
    return 1
}
stop_mock() { [ -n "$MOCK_PID" ] && kill "$MOCK_PID" 2>/dev/null; wait "$MOCK_PID" 2>/dev/null; MOCK_PID=""; }

# 共用判据：跑一次 errcase 探针并断言四件事
run_errcase() {   # $1=probe 产物 $2=标签 ⇒ 设置 ERRCASE_OK
    local pb="$1" tag="$2" P LOG OUT
    P=$(port_new); LOG="$W/mock_errcase_$tag.log"
    ERRCASE_OK=0
    if ! run_mock "$P" errcase "$LOG"; then bad "[$tag] mock 未就绪（端口 $P）"; return; fi
    OUT="$(timeout 25 "$pb" "$P" errcase 2>&1)"; RC=$?
    echo "$OUT" | sed 's/^/     /'
    local badn=0
    [ "$RC" = "0" ] || { bad "[$tag] 探针退出码 $RC"; badn=1; }
    # ① 服务器 ERR 包 ⇒ **不**关连接
    echo "$OUT" | grep -q 'PROBE-ERR-BAD .*mysql err 1054:.* closed=false' \
        && ok "[$tag] 1054 只让本条语句失败（closed=false）" || { bad "[$tag] 1054 仍把连接关掉了"; badn=1; }
    # ② 连接真的还能用（不是「没关但也没法用」）
    echo "$OUT" | grep -q 'PROBE-ERR-OK2 affected=2' \
        && ok "[$tag] 出错后同一连接继续可用（SELECT ok2 affected=2）" || { bad "[$tag] 出错后连接不可用"; badn=1; }
    # ③ 致命码 1053 ⇒ 关连接
    #   ⚠️ 实际文本是 `msg=mysql: mysql err 1053: …`（my_conn_bad 叠了 `mysql: ` 前缀）
    echo "$OUT" | grep -q 'PROBE-ERR-FATAL .*mysql err 1053:.* closed=true' \
        && ok "[$tag] 致命码 1053 关连接（closed=true）" || { bad "[$tag] 致命码没有关连接"; badn=1; }
    # ④ 关掉之后再查 ⇒ 响亮 already closed
    echo "$OUT" | grep -q 'PROBE-ERR-AFTER msg=mysql: connection already closed' \
        && ok "[$tag] 关闭后复用 ⇒ already closed" || { bad "[$tag] 关闭后仍可复用（半连接）"; badn=1; }
    echo "$OUT" | grep -q 'PROBE-ERR-KEEP-OK' && ok "[$tag] 总判据 PROBE-ERR-KEEP-OK" || { bad "[$tag] 总判据未通过"; badn=1; }
    [ "$badn" = "0" ] && ERRCASE_OK=1
    stop_mock
}

# ── [3] 动态（0.2.0）────────────────────────────────────────
echo "── [3] 动态：0.2.0（ERR 包保连接 · 致命码废连接）"
run_errcase "$P020B" "3"

# ── [4] 动态（0.1.0）────────────────────────────────────────
echo "── [4] 动态：0.1.0（同一份代码 ⇒ 同形态）"
run_errcase "$P010B" "4"

# ── [5] ping / select_db 成功路径 ────────────────────────────
echo "── [5] 动态：ping / select_db 成功路径"
P=$(port_new); LOG="$W/mock_ping.log"
if run_mock "$P" ok "$LOG"; then
    OUT="$(timeout 25 "$P020B" "$P" ping 2>&1)"; RC=$?
    echo "$OUT" | sed 's/^/     /'
    chk "[5.1] 退出码" "$RC" "0"
    echo "$OUT" | grep -q 'PROBE-PING-OK' && ok "[5.2] PROBE-PING-OK" || bad "[5.2] ping/select_db 未通过：$(echo "$OUT" | tail -1)"
    grep -q 'MOCK-PING-OK' "$LOG" && ok "[5.3] 服务端收到 COM_PING" || bad "[5.3] 服务端未收到 COM_PING"
    grep -q 'MOCK-SELECTDB db=pxtest' "$LOG" && ok "[5.4] 服务端收到 COM_INIT_DB(pxtest)" || bad "[5.4] 未收到 COM_INIT_DB"
else
    bad "[5.0] mock 未就绪（端口 $P）"
fi
stop_mock

# ── [6] ping 超时 ⇒ 关连接（缺陷 542 回归位）──────────────────
echo "── [6] 动态：心跳超时 ⇒ 关连接（不留半连接）"
P=$(port_new); LOG="$W/mock_pingto.log"
if run_mock "$P" silent_after "$LOG"; then
    OUT="$(timeout 25 "$P020B" "$P" pingtimeout 2>&1)"; RC=$?
    echo "$OUT" | sed 's/^/     /'
    chk "[6.1] 退出码（已关连接 ⇒ 0）" "$RC" "0"
    echo "$OUT" | grep -q 'PROBE-PINGTIMEOUT-ERR .* closed=true' && ok "[6.2] 超时后连接被关闭（closed=true）" || bad "[6.2] 超时后没关连接"
    echo "$OUT" | grep -q 'PROBE-PING-AFTER msg=mysql: connection already closed' && ok "[6.3] 二次 ping ⇒ already closed" || bad "[6.3] 半连接态仍在"
    echo "$OUT" | grep -q 'PROBE-PING-NO-HALF-OPEN' && ok "[6.4] PROBE-PING-NO-HALF-OPEN" || bad "[6.4] 总判据未通过"
else
    bad "[6.0] mock 未就绪（端口 $P）"
fi
stop_mock

# ── [7][8] 负控（改 W 里的驱动副本，仓库零写入）─────────────
if [ "$NEG_SKIP" = "1" ]; then
    echo "── [7][8] 负控：**已按 --neg-skip 跳过**（CI 档）"
else
    neg_run() {   # $1=name  ⇒ 打印探针输出；结果放 NEG_OUT / NEG_OK
        local name="$1"
        local d="$W/drv_neg_$name"
        mk_driver_copy "$ROOT/registry/mysql/0.2.0" "$d"
        python3 - "$d/mysql.px" "$d/proto.px" "$name" <<'PY'
import sys
mp, pp, name = sys.argv[1], sys.argv[2], sys.argv[3]
m = open(mp, encoding='utf-8').read()
p = open(pp, encoding='utf-8').read()
if name == 'nosplit':
    old = "    if my_err_is_server(what) and not my_err_fatal(my_err_code(what)):\n        return Err(what)\n"
    assert m.count(old) == 1, "nosplit 锚点不唯一"
    m = m.replace(old, "")
elif name == 'nofatal':
    n = 0
    for ln in ("if code == 1053 or code == 1927 or code == 3169 or code == 4031:\n        return true",
               "if code == 2006 or code == 2013:\n        return true"):
        assert p.count(ln) == 1, "nofatal 锚点不唯一: " + ln
        p = p.replace(ln, ln.replace("return true", "return false")); n += 1
    assert n == 2
else:
    raise SystemExit("unknown neg " + name)
open(mp, 'w', encoding='utf-8').write(m)
open(pp, 'w', encoding='utf-8').write(p)
print("NEG-PATCHED " + name)
PY
        local pb="$W/negprobe_$name.px"
        sed "s|\.\./\.\./registry/mysql/0\.2\.0/mysql\.px|$d/mysql.px|" "$SRC/probe.px" > "$pb"
        "$ROOT/tools/px" build "$pb" >/dev/null 2>&1 || { NEG_OUT="BUILD-FAIL"; NEG_OK=0; return; }
        local P2 LOG2
        P2=$(port_new); LOG2="$W/mock_neg_$name.log"
        if ! run_mock "$P2" errcase "$LOG2"; then NEG_OUT="MOCK-FAIL"; NEG_OK=0; return; fi
        NEG_OUT="$(timeout 25 "$W/build/negprobe_$name" "$P2" errcase 2>&1)"
        stop_mock
    }

    echo "── [7] 负控 A：撤回 my_conn_bad 的分流 ⇒ BAD(1054) 必须**把连接关掉**"
    neg_run nosplit
    echo "$NEG_OUT" | sed 's/^/     /'
    if echo "$NEG_OUT" | grep -q 'PROBE-ERR-BAD .*closed=true'; then
        ok "[7.1] 撤回分流后 1054 关连接（判据有牙：复现了半山报障的形态）"
    else
        bad "[7.1] 撤回分流后 1054 仍未关连接（判据没牙 / 补丁没生效）"
    fi
    echo "$NEG_OUT" | grep -q 'PROBE-ERR-KEEP-OK' && bad "[7.2] 撤回后仍报 KEEP-OK（判据失效）" || ok "[7.2] 撤回后总判据不再通过"

    echo "── [8] 负控 B：致命码表失效 ⇒ FATAL(1053) 必须**不再关连接**"
    neg_run nofatal
    echo "$NEG_OUT" | sed 's/^/     /'
    if echo "$NEG_OUT" | grep -q 'PROBE-ERR-FATAL .*closed=false'; then
        ok "[8.1] 致命码表失效后 1053 不关连接（判据有牙）"
    else
        bad "[8.1] 致命码表失效后 1053 仍关连接（判据没牙）"
    fi
    echo "$NEG_OUT" | grep -q 'PROBE-ERR-KEEP-OK' && bad "[8.2] 撤回后仍报 KEEP-OK（判据失效）" || ok "[8.2] 撤回后总判据不再通过"
fi

# ── [9] 覆盖边界 ─────────────────────────────────────────────
echo "── [9] 覆盖边界（如实）"
echo "  ℹ️ ① 服务器 ERR 包的**分类**只由 proto.px 的固定前缀识别；上游若改 my_err_message 的格式串，"
echo "       本门 [1.2][1.4] 与 [1.1] 会判红（前缀与格式必须同源）。"
echo "  ℹ️ ② 致命码表按 MySQL 5.7/8.0 文档选 6 个；**未实测**真实服务器上的 4031/3169（需对应版本 + 触发条件）。"
echo "  ℹ️ ③ 结果集（列定义 + 行）与 COM_STMT_* 仍不在本门动态面内（同 m313 的边界）。"
echo "  ℹ️ ④ 「ERR 包之后连接是否被服务端主动断开」取决于服务器/中间件；本门只判**客户端**是否自行关闭。"
echo "  ℹ️ ⑤ 事务语义（begin/commit/rollback 走 mysql_query ⇒ 同一路径）未单独设用例。"

echo
if [ "$FAIL" = "0" ]; then
    echo "══ M314-VERIFY-OK · $PASS 通过 / 0 失败 ══"
    exit 0
else
    echo "══ M314-VERIFY-FAIL · $PASS 通过 / $FAIL 失败 ══"
    exit 1
fi
