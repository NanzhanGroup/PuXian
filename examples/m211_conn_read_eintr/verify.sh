#!/usr/bin/env bash
# ============================================================
# M211 门（第 90 轮）：**GC STW 信号打断 recv ⇒ 误判对端关闭**（缺陷 265 收口 · 编号 284–287）
# ------------------------------------------------------------
# 病灶链（每一环都有实测证据，见 /tmp/m211/）：
#   ① `px_serve` 的读路径 = `px_conn_worker` → **`px_conn_read`**（不是 `px_recv_wait`）；
#   ② `px_conn_read` 明文分支原为 `return recv(c->fd, buf, n, 0);` —— **不重试 EINTR**；
#   ③ 并发 GC 的 STW（`gc_stop_handler`）向各线程发 SIG_GC_STOP **打断阻塞中的 recv**
#      ⇒ 返回 -1/EINTR；
#   ④ 调用方 `if (n <= 0) break;` 判定为「客户端关闭 / 空闲超时」⇒ **未进 handler 即关连接**；
#   ⑤ 客户端只见空响应 / FIN（总数 = 0）。
#   触发条件 = `PX_GC_STRESS=1 PX_GC_INLINE=1`（**两开关同开**，与缺陷 192 同族）：
#   每次分配即 inline full GC ⇒ 信号高频 ⇒ 必现；默认档 GC 延迟到安全点 ⇒ 长期未显形。
#
# 修复（`runtime/runtime.c`，四处）：
#   · 284 明文分支：EINTR 无限重试（15s 空闲语义不变 —— 超时仍返回 -1/EAGAIN）；
#   · 286 TLS 分支：`MBEDTLS_ERR_SSL_WANT_READ/WANT_WRITE` 非错误 ⇒ 重试（同族病灶的 TLS 面）；
#   · 287 `px_conn_write`：EINTR 不再当写失败（否则**响应截断**）；
#   · 285 `gc_stop_handler`：保存 / 恢复 `errno`（POSIX 要求）。
#     ⚠️ 285 经 A/B **实测不是本缺陷根因**（撤回后仍红）⇒ 如实登记为**独立小缺陷**，不冒充根因。
#
# 层的设计（每层都能独立判红）：
#   ① 静态判据（源码在位 5 条 + **反向断言** 1 条：明文分支不得再是裸 `return recv(...)`）
#   ② 动态基线（正常档）：8 连裸 TCP ⇒ 每条 total > 1000
#   ③ **动态主判据（压力档 `STRESS+INLINE`）**：同上 —— 修前 8 条**全 0**，修后全 > 1000
#   ④ 压力档与正常档的 total 序列一致
#   ⑤ 负控 3 道（`--neg-skip` 可跳；各自独立判红，且跑前 `restore_all; snapshot` 各自干净起点）：
#      A 明文分支退回裸 `return recv(...)` ⇒ ③ 必红（出现 total=0）；
#      B 撤回 TLS WANT_* 重试 ⇒ ① 静态必红；
#      C **判据自伤**：复用 A 的产物（同一补丁）+ `M211_THRESH=-1` ⇒ 同一故障**不再**判红
#        （⚠️ 首版写成 `THRESH=0` 且判据只查「是否出现 total=0」—— 那是**同义反复**：
#          `0 > 0` 为假 ⇒ 故障仍判红，而断言却报 PASS。已改为「真跑 chk_series 看它是否还红」。）
#   ⑥ 源逐字节还原断言
#
# ⚠️ 用 `./tools/px build`（产物落 `<用例目录>/build/<名>`）；`--c` 才是 C 轨，
#    默认档 = VM 轨（precise GC）—— 本缺陷正是 VM 轨默认档的病灶。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="${M211_W:-$(mktemp -d /tmp/m211_gate.XXXXXX)}"
mkdir -p "$W"
SNAP="$W/snap"; SRC="runtime/runtime.c"
snapshot() { mkdir -p "$SNAP/runtime"; cp "$SRC" "$SNAP/$SRC"; }
restore_all() { [ -f "$SNAP/$SRC" ] && cp "$SNAP/$SRC" "$SRC"; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PXBUILD="$ROOT/tools/px"
REL="examples/m211_conn_read_eintr/probe_eintr.px"
BD="$HERE/build"; BIN="$BD/probe_eintr"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS + 1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL + 1)); }

# 精确单点替换（**唯一性断言**：命中数 ≠ 1 ⇒ 响亮失败，绝不静默 no-op）
patch_one() {
    python3 - "$SRC" "$1" "$2" <<'PY'
import sys, io
p, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = io.open(p, encoding='utf-8').read()
k = s.count(old)
if k != 1:
    sys.stderr.write("ANCHOR-BAD count=%d\n" % k); sys.exit(3)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
}

# 端口清理（上次残留会让 bind 失败 ⇒ 假红）
free_port() {
    local p="$1" pid=""
    pid="$( (ss -ltnp 2>/dev/null || netstat -ltnp 2>/dev/null) | grep ":$p " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
    [ -n "$pid" ] && kill "$pid" 2>/dev/null && sleep 0.5
    return 0
}

rebuild() {   # $1=日志路径 → 0/1
    rm -rf "$BD"
    ( cd "$ROOT" && "$PXBUILD" build "$REL" ) >"$1" 2>&1
}

# ---------- 层 ① 静态判据 ----------
static_check() {
    local r=0
    grep -Fq 'if (r < 0 && errno == EINTR) continue;' "$SRC" \
        && ok "[1] 明文分支 EINTR 重试在位" || { bad "[1] 明文分支缺 EINTR 重试"; r=1; }
    grep -Fq 'if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;' "$SRC" \
        && ok "[1] TLS 分支 WANT_* 重试在位" || { bad "[1] TLS 分支缺 WANT_* 重试"; r=1; }
    grep -Fq 'if (k < 0 && errno == EINTR) continue;' "$SRC" \
        && ok "[1] px_conn_write EINTR 重试在位" || { bad "[1] px_conn_write 缺 EINTR 重试"; r=1; }
    grep -Fq 'int saved_errno = errno;' "$SRC" \
        && ok "[1] gc_stop_handler 保存 errno 在位" || { bad "[1] gc_stop_handler 缺 errno 保存"; r=1; }
    grep -Fq 'errno = saved_errno;' "$SRC" \
        && ok "[1] gc_stop_handler 恢复 errno 在位" || { bad "[1] gc_stop_handler 缺 errno 恢复"; r=1; }
    if grep -Fq 'if (!c->is_tls) return recv(c->fd, buf, n, 0);' "$SRC"; then
        bad "[1] 反向断言失败：明文分支退回裸 return recv(...)"
        r=1
    else
        ok "[1] 反向断言通过（明文分支未退回裸 recv）"
    fi
    return $r
}

echo "══ M211 门：GC STW 打断 recv ⇒ 误判对端关闭（缺陷 265 · 284–287）══"
echo "── [1] 静态判据（源码在位 5 条 + 反向 1 条）"
static_check

# ---------- 层 ② 编译探针（一次；两档共用） ----------
echo "── [2] 编译探针（./tools/px build，默认 VM 轨）"
free_port 18310
if rebuild "$W/build.log" && [ -x "$BIN" ]; then
    ok "[2] 探针编译成功 → $BIN"
else
    bad "[2] 探针编译失败 / 产物缺失"
    tail -8 "$W/build.log"
fi

# ---------- 跑一档 ----------
run_probe() {   # $1=档名  $2..=env 赋值
    local tag="$1"; shift
    local out="$W/out_$tag.txt"
    ( cd "$ROOT" && env "$@" timeout -k 5 90 "$BIN" ) >"$out" 2>&1
    echo "$?"
}

# ---------- 逐条判 total ----------
chk_series() {  # $1=档名 $2=log → 失败条数
    local tag="$1" log="$2" th v n=0
    th="${M211_THRESH:-1000}"
    for i in 1 2 3 4 5 6 7 8; do
        v="$(grep -oE "^R$i=[0-9-]+" "$log" | head -1 | cut -d= -f2)"
        if [ -z "$v" ]; then
            bad "[$tag] R$i 无输出"
            n=$((n + 1)); continue
        fi
        if [ "$v" -gt "$th" ]; then
            ok "[$tag] R$i=$v"
        else
            bad "[$tag] R$i=$v（≤$th ⇒ 响应残缺 / 对端提前关闭）"
            n=$((n + 1))
        fi
    done
    return $n
}

# ---------- 层 ③ 正常档 ----------
echo "── [3] 动态基线（正常档）"
rc_norm=$(run_probe norm)
[ "$rc_norm" = "0" ] && ok "[norm] rc=0" || bad "[norm] rc=$rc_norm"
chk_series norm "$W/out_norm.txt" || true

# ---------- 层 ④ 压力档（主判据） ----------
echo "── [4] 动态主判据（压力档 PX_GC_STRESS=1 PX_GC_INLINE=1）"
rc_str=$(run_probe str PX_GC_STRESS=1 PX_GC_INLINE=1)
[ "$rc_str" = "0" ] && ok "[str] rc=0" || bad "[str] rc=$rc_str"
chk_series str "$W/out_str.txt" || true

# ---------- 层 ⑤ 两档序列一致 ----------
echo "── [5] 两档 total 序列一致性"
sn() { grep -oE '^R[1-8]=[0-9-]+' "$1" | tr '\n' ' '; }
A="$(sn "$W/out_norm.txt")"; B="$(sn "$W/out_str.txt")"
[ -n "$A" ] && [ "$A" = "$B" ] && ok "[5] 两档序列一致：$A" || bad "[5] 序列不一致 norm=[$A] str=[$B]"

# ---------- 层 ⑥ 负控 ----------
PATCH_A_OLD='        for (;;) {
            ssize_t r = recv(c->fd, buf, n, 0);
            if (r < 0 && errno == EINTR) continue;
            return r;
        }'
PATCH_A_NEW='        return recv(c->fd, buf, n, 0);'

if [ "$NEG_SKIP" = "1" ]; then
    echo "── [6] 负控（--neg-skip 跳过）"
else
    echo "── [6] 负控 3 道（各自独立）"

    # 负控 A：明文分支退回裸 return recv(...) ⇒ 压力档必红
    restore_all; snapshot
    if patch_one "$PATCH_A_OLD" "$PATCH_A_NEW" >"$W/pA.log" 2>&1; then
        free_port 18310
        rebuild "$W/build_A.log"
        run_probe negA PX_GC_STRESS=1 PX_GC_INLINE=1 >/dev/null
        if grep -qE '^R[1-8]=0$' "$W/out_negA.txt" 2>/dev/null; then
            ok "[A] 撤回 EINTR 重试 ⇒ 压力档判红（出现 total=0）"
        else
            bad "[A] 撤回 EINTR 重试后**未**判红（负控失去牙）：$(sn "$W/out_negA.txt")"
        fi
    else
        bad "[A] 负控补丁未生效：$(cat "$W/pA.log")"
    fi

    # 负控 B：撤回 TLS WANT_* 重试 ⇒ 静态必红
    restore_all; snapshot
    if patch_one '    int ret;
    for (;;) {   // M211（缺陷 265 同族）：WANT_READ/WANT_WRITE 非错误 —— 必须重试
        ret = mbedtls_ssl_read((mbedtls_ssl_context*)c->ssl, c->rbuf, (size_t)sizeof(c->rbuf));
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        break;
    }' '    int ret = mbedtls_ssl_read((mbedtls_ssl_context*)c->ssl, c->rbuf, (size_t)sizeof(c->rbuf));' >"$W/pB.log" 2>&1; then
        if grep -Fq 'if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;' "$SRC"; then
            bad "[B] 撤回 TLS 重试后静态判据仍通过（负控失去牙）"
        else
            ok "[B] 撤回 TLS 重试 ⇒ 静态判据判红"
        fi
    else
        bad "[B] 负控补丁未生效：$(cat "$W/pB.log")"
    fi

    # 负控 C：判据自伤 —— 复用 A 的产物（同一补丁在位）+ THRESH=-1 ⇒ 同一故障**不再**判红
    #   判据必须**真跑 chk_series**（不能只看「是否出现 total=0」—— 那是同义反复）
    if [ -x "$BIN" ] && grep -qE '^R[1-8]=0$' "$W/out_negA.txt" 2>/dev/null; then
        ( cd "$ROOT" && M211_THRESH=-1 PX_GC_STRESS=1 PX_GC_INLINE=1 timeout -k 5 90 "$BIN" ) >"$W/out_negC.txt" 2>&1
        echo "  （负控 C 明细：阈值 -1 下逐条判定）"
        if M211_THRESH=-1 chk_series negC "$W/out_negC.txt"; then
            ok "[C] 故障仍在（A 已证）而阈值放宽后不再判红 ⇒ 红确实来自阈值判据"
        else
            bad "[C] 判据自伤：阈值放宽后 chk_series 仍报失败"
        fi
    else
        bad "[C] 前置不满足（A 未判红或产物缺失）"
    fi

    restore_all; snapshot
fi

# ---------- 层 ⑦ 还原断言 ----------
echo "── [7] 源逐字节还原"
restore_all
if diff -q "$SNAP/$SRC" "$SRC" >/dev/null 2>&1; then
    ok "[7] runtime.c 已逐字节还原"
else
    bad "[7] runtime.c 未还原"
fi

echo "══ M211 门：$PASS 通过 / $FAIL 失败 ══"
if [ "$FAIL" -eq 0 ]; then echo "M211-VERIFY-OK"; exit 0; fi
exit 1
