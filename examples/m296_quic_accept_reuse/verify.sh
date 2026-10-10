#!/usr/bin/env bash
# ============================================================
# examples/m296_quic_accept_reuse/verify.sh
# ------------------------------------------------------------
# M296（第 172 轮）：**一条 listener 能不能连续接手多条连接**？
#
# 来历（晨曦的现场，两轮才定性）：
#   10-10 他报「临时服务端 `quic_close(conn)` 之后 listener 端口不再监听」。
#   本机按「先量后改」复现，结果照出**两个**缺陷 —— 第二个比第一个更根本：
#
#   · 缺陷 **511**：`bi_quic_close` 无条件 `close(qc->fd)`，而 `quic_listen` + `quic_accept`
#     形态下 `qc->fd == ql->fd`（连接与 listener **共享**同一个 UDP socket，见
#     `quic_srv_new_conn` / `bi_quic_accept`）⇒ **关一条连接就把 listener 的 socket 关掉**
#     （端口不再监听；顺带让既有示例 `quic_close(c); quic_close_listener(lst)` 变成 double close）。
#     实测（修前）：`close#1` 之后 `ss -lun` 看不到该端口、`accept#2 = -1`、第二条客户端 CONNECTFAIL。
#
#   · 缺陷 **512**（更根本）：`bi_quic_accept` 收到的**第一个包**若不是本连接的有效 Initial
#     （前一条连接的 **PMTUD probe** / ACK / **CONNECTION_CLOSE** 残包、扫描流量、损坏包），
#     修前是 `return px_int(-1)` —— **整次 accept 直接判死**，而同一条 socket 缓冲区里
#     早已排队的**新客户端 Initial 永远读不到**。
#     抓包实证（端口 19935）：
#       17.241851  A(39482) > srv(19935)  UDP length **1406**   ← A 的 PMTUD probe，残留在 listener 缓冲区
#       18.499804  B(45327) > srv(19935)  UDP length 1200      ← B 的 Initial **已到达**
#       服务端 accept#2 却报 `first read_pkt rv=-232 (ERR_DROP_CONN)` 后返回 -1。
#     ⇒ 表象是「第二条连接连不上」，与 511 是**两个独立病灶**（各自的负控都能单独判红）。
#
# 修法：
#   511 → 新增 **fd 归属判据** `quic_fd_shared_with_listener()`：只有**独占** fd 才 close；
#         被活跃 listener 持有的 fd 跳过（由 `quic_close_listener` 负责）。
#         另：对**托管连接**（`owner_listener > 0`）语言层 close ⇒ **响亮拒绝**
#         （结构上消除「memset 掉连接线程正在用的 pthread_cond_t / 队列指针」这类跨线程破坏面）。
#   512 → 两个失败点（`first read_pkt` / `handshake pump`）改成**丢弃这个包、继续等下一个**
#         （deadline 保护不变），并加**限流报告**（这种丢弃不许静默，同 M295 的 quic_alloc_fail_note 口径）。
#
# 判据层次：
#   [1] 静态（结构在位 + **逆向** + 去注释器自证 + 规模锚点）
#   [2] 构建两个产物 · [3] 起服务端（**强就绪**：端口真在监听）
#   [4] 动态主判据：`close#1` 之后端口仍在监听 → `accept#2 > 0` → `RECV2` 数据正确
#   [5] 负控 A（忠实撤回 511）· B（忠实撤回 512）· C（判据自伤）
#   [6] 覆盖边界（如实）
#
# ⚠️ 本门**会就地改仓库源码**（负控 A/B 撤回 `runtime/runtime_quic.c`）⇒
#   · 开头 source `selfhost/gate_lock.sh`（M276：与全量门/其它门互斥）
#   · `trap` 里无条件 `restore_all`（中途被杀也还原）
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M296_GATE_W:-$(mktemp -d /tmp/m296_gate.XXXXXX)}"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

TGT="$ROOT/runtime/runtime_quic.c"
PXDEV="${M296_PXC:-/tmp/pxcdev}"
PORT="${M296_GATE_PORT:-19936}"

echo "══ M296 · 一条 listener 连续接手多条连接（缺陷 511 / 512）══"

# ── 快照 / 还原（负控用）────────────────────────────────────
SNAP="$W/tgt.snap"; cp -f "$TGT" "$SNAP"; rm -f "$TGT".neg
restore_all() { [ -f "$SNAP" ] && cp -f "$SNAP" "$TGT" 2>/dev/null; rm -f "$W"/tgt.*.bak 2>/dev/null; }
trap 'restore_all; [ "${KEEP_W:-0}" = "1" ] || rm -rf "$W"' EXIT

# 去注释（只去注释、保留字面量 —— M200 教训）
cat > "$W/cstrip.py" <<'PY'
import re, sys
s = open(sys.argv[1], encoding="utf-8", errors="replace").read()
out, i, n = [], 0, len(s)
while i < n:
    c = s[i]
    if c == '"' or c == "'":
        q = c; out.append(c); i += 1
        while i < n:
            out.append(s[i])
            if s[i] == '\\' and i + 1 < n:
                out.append(s[i+1]); i += 2; continue
            if s[i] == q: i += 1; break
            i += 1
        continue
    if c == '/' and i + 1 < n and s[i+1] == '*':
        j = s.find('*/', i + 2); i = n if j < 0 else j + 2; out.append(' '); continue
    if c == '/' and i + 1 < n and s[i+1] == '/':
        j = s.find('\n', i); i = n if j < 0 else j; continue
    out.append(c); i += 1
print(re.sub(r'[ \t]+', ' ', ''.join(out)))
PY

CODE="$W/tgt.code.c"
python3 "$W/cstrip.py" "$TGT" > "$CODE" 2>"$W/cstrip.err" || true
if [ -s "$CODE" ] && grep -q "bi_quic_accept" "$CODE"; then ok "[1a] 去注释器自证（含 bi_quic_accept 锚点）"; else bad "[1a] 去注释器不可用（见 $W/cstrip.err）"; fi

# ── [1] 静态判据 ─────────────────────────────────────────────
echo "[1] 静态：结构在位 + 逆向"
cnt(){ grep -c -- "$1" "$CODE" 2>/dev/null | head -1; }
c_def=$(num "$(cnt 'static int quic_fd_shared_with_listener(int fd)')")
chk "[1b] fd 归属判据函数（缺陷 511）" "$c_def" "1"
c_use=$(num "$(cnt 'quic_fd_shared_with_listener(qc->fd)')")
chk "[1c] bi_quic_close 用归属判据" "$c_use" "1"
# 逆向：**不得**存在无条件的 `close(qc->fd);`（修前形态）
# ⚠️ 必须**限定在 bi_quic_close 函数体内**（M222 教训：全局 grep 会假红 ——
#    `quic_migrate` 里也有一处 `close(qc->fd)`，那是**切 fd**（客户端独占 socket），
#    本就该关；它另有 `owner_listener > 0` 守卫挡住服务端形态）。
BODY="$W/close_body.c"
awk '/^static LXValue bi_quic_close\(/{f=1} f{print} f&&/^}/{exit}' "$CODE" > "$BODY"
if [ -s "$BODY" ]; then ok "[1d0] 提取到 bi_quic_close 函数体（$(wc -l < "$BODY") 行）"; else bad "[1d0] 未能提取 bi_quic_close 函数体"; fi
badclose=$(grep -nE 'close\(qc->fd\);' "$BODY" 2>/dev/null | grep -v 'quic_fd_shared_with_listener' | wc -l | tr -d ' ')
chk "[1d] bi_quic_close 内无条件 close(qc->fd) 已消除（逆向）" "$(num "$badclose")" "0"
goodclose=$(grep -cE 'quic_fd_shared_with_listener\(qc->fd\)\) close\(qc->fd\);' "$BODY" | head -1)
chk "[1d1] 归属判据式 close 在位（正向）" "$(num "$goodclose")" "1"
c_own=$(num "$(cnt 'qc->owner_listener > 0')")
if [ "$c_own" -ge 1 ]; then ok "[1e] 托管连接守卫在位（$c_own 处）"; else bad "[1e] 缺托管连接守卫"; fi
c_drop=$(num "$(cnt 'quic_accept_drop_note')")
chk "[1f] 丢弃包计数/报告（缺陷 512）" "$c_drop" "3"
c_cont=$(grep -cE 'quic_accept_drop_note\("(first read_pkt|handshake pump)"' "$CODE" | head -1)
chk "[1g] 两个丢弃点都在（first read_pkt / handshake pump）" "$(num "$c_cont")" "2"
# 逆向：修前的 `return px_int(-1)` 紧跟 read_pkt 报错 —— 不得再出现
old_ret=$(grep -c 'accept first read_pkt rv=' "$CODE" 2>/dev/null | head -1)
chk "[1h] 修前形态（read_pkt 失败即 return）已消除（逆向）" "$(num "$old_ret")" "0"
lines=$(wc -l < "$TGT")
if [ "$lines" -ge 2400 ]; then ok "[1i] 规模锚点（runtime_quic.c 行数 $lines ≥ 2400）"; else bad "[1i] 规模锚点异常（$lines）"; fi

# ── [2] 构建 ────────────────────────────────────────────────
echo "[2] 构建（srv / cli）"
# ⚠️ 两个坑（都踩过）：
#   ① `tools/px build <F>` 的产物落在 **dirname(F)/build/基名** ⇒ 直接对仓库里的 .px 构建
#      会往 `examples/<门>/build/` 写（污染工作树）⇒ 一律**先复制到工作区**再构建。
#   ② 构建失败时把编译日志的尾行打进判据里（否则「构建失败」指不到真因）。
build_one(){  # $1=名 $2=引擎（c|默认）
  local f="$1" pxc="${2:-}"
  mkdir -p "$W/src"; cp -f "$HERE/$f.px" "$W/src/$f.px"
  rm -rf "$W/src/build"
  if [ -n "$pxc" ]; then
    ( cd "$W/src" && PX_PXC_BIN="$pxc" PX_BUILD_ENGINE=c timeout 300 "$ROOT/tools/px" build "$W/src/$f.px" > "$W/b_$f.log" 2>&1 )
  else
    ( cd "$W/src" && timeout 300 "$ROOT/tools/px" build "$W/src/$f.px" > "$W/b_$f.log" 2>&1 )
  fi
  [ -x "$W/src/build/$f" ] || return 1
  cp -f "$W/src/build/$f" "$W/bin_$f"; return 0
}
PXC_USE=""
if build_one srv "$PXC_USE" && build_one cli "$PXC_USE"; then ok "[2a] 两个产物构建成功（入库件）"
else bad "[2a] 构建失败（见 $W/b_*.log：$(tail -1 "$W/b_srv.log" 2>/dev/null)）"; fi

# ── [3][4] 动态主判据 ───────────────────────────────────────
# $1 = 服务端二进制 $2 = 客户端二进制 $3 = 端口 $4 = 标签前缀（诊断用）
run_case(){
  local srv="$1" cli="$2" port="$3" tag="${4:-}" spid rdy
  rm -f "$W/run_$tag.srv.log" "$W/run_$tag.a.log" "$W/run_$tag.b.log"
  M296_PORT="$port" "$srv" > "$W/run_$tag.srv.log" 2>&1 &
  spid=$!
  local i
  for i in $(seq 1 60); do grep -q "M296-PORT" "$W/run_$tag.srv.log" 2>/dev/null && break; sleep 0.25; done
  # 强就绪：端口真在监听（M280 教训：ready 字样 ≠ 绑定成功）
  local ready=no
  for i in $(seq 1 20); do ss -lun 2>/dev/null | grep -q ":$port " && { ready=yes; break; }; sleep 0.25; done
  EV_READY="$ready"
  M296_PORT="$port" M296_TAG=A "$cli" > "$W/run_$tag.a.log" 2>&1
  for i in $(seq 1 80); do grep -q "M296-READY2" "$W/run_$tag.srv.log" 2>/dev/null && break; sleep 0.25; done
  # 进程外判据：close#1 之后 listener 端口是否仍在监听
  if ss -lun 2>/dev/null | grep -q ":$port "; then EV_LISTEN_AFTER=yes; else EV_LISTEN_AFTER=no; fi
  M296_PORT="$port" M296_TAG=B "$cli" > "$W/run_$tag.b.log" 2>&1
  wait $spid 2>/dev/null
  EV_ACCEPT2="$(sed -n 's/^M296-ACCEPT2 //p' "$W/run_$tag.srv.log" | head -1)"
  EV_RECV2="$(sed -n 's/^M296-RECV2 \[\(.*\)\]$/\1/p' "$W/run_$tag.srv.log" | head -1)"
  EV_ECHO2="$(sed -n 's/^M296-CLI-B-ECHO \[\(.*\)\]$/\1/p' "$W/run_$tag.b.log" | head -1)"
  EV_ACCEPT2="$(num "$EV_ACCEPT2")"
}

# 判定与取数分离 —— 负控 C（判据自伤）要能**只废判据、不废被测对象**
judge_listen_ok(){ if [ "${M296_FORCE_PASS:-0}" = "1" ]; then echo yes; else echo "$EV_LISTEN_AFTER"; fi; }
judge_accept2_ok(){ if [ "${M296_FORCE_PASS:-0}" = "1" ]; then echo 1; else echo "$EV_ACCEPT2"; fi; }

if [ -x "$W/bin_srv" ] && [ -x "$W/bin_cli" ]; then
  echo "[3][4] 动态：close#1 → 再 accept 第二条"
  run_case "$W/bin_srv" "$W/bin_cli" "$PORT" base
  chk "[3a] 服务端起在 :$PORT（强就绪）" "$EV_READY" "yes"
  chk "[4a] close#1 之后端口仍在监听（缺陷 511）" "$(judge_listen_ok)" "yes"
  if [ "$(judge_accept2_ok)" -gt 0 ]; then ok "[4b] accept#2 成功（缺陷 512）（$EV_ACCEPT2）"; else bad "[4b] accept#2 失败（$EV_ACCEPT2 ≤ 0）—— listener 只接了一条连接"; fi
  chk "[4c] 第二条连接的数据正确" "$EV_RECV2" "ping-B"
  chk "[4d] 第二条客户端拿到回显" "$EV_ECHO2" "echo2:ping-B"
  # 存活 / fd（顺带看有没有泄漏）
  if grep -q "M296-DONE" "$W/run_base.srv.log"; then ok "[4e] 服务端正常收尾（M296-DONE）"; else bad "[4e] 服务端未正常收尾"; fi
else
  bad "[3a] 无产物，跳过动态判据"
fi

# ── [5] 负控 ────────────────────────────────────────────────
if [ "$NEG_SKIP" = "1" ]; then
  echo "[5] 负控（--neg-skip：CI 档跳过）"
else
  echo "[5] 负控（忠实撤回 + 重建 dev 件 ⇒ 必须判红）"
  # 打桩器：把修复**忠实撤回**到修前形态（逐字节对照的是「撤回后的语义」，不是格式）
  cat > "$W/neg511.py" <<'PY'
import sys, pathlib
p = pathlib.Path(sys.argv[1]); s = p.read_text()
old = "    if (!quic_fd_shared_with_listener(qc->fd)) close(qc->fd);"
new = "    close(qc->fd);"
assert s.count(old) == 1, "锚点不唯一(511)"
p.write_text(s.replace(old, new, 1)); print("NEG511-OK")
PY
  cat > "$W/neg512.py" <<'PY'
import sys, pathlib
p = pathlib.Path(sys.argv[1]); s = p.read_text()
a = '''            quic_accept_drop_note("first read_pkt", rv, rl);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0;
            continue;'''
b = '''            fprintf(stderr, "[quic] accept first read_pkt rv=%d\\n", rv);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1);'''
assert s.count(a) == 1, "锚点不唯一(512)"
s = s.replace(a, b, 1)
c = '''            quic_accept_drop_note("handshake pump", pr2, rl);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0;
            continue;'''
d = '''            fprintf(stderr, "[quic] accept pump rv=%d\\n", pr2);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1);'''
assert s.count(c) == 1, "锚点不唯一(512b)"
p.write_text(s.replace(c, d, 1)); print("NEG512-OK")
PY
  # 重建 dev 件（M287 多槽缓存：撤回态此前建过 ⇒ 通常秒级命中）
  rebuild_dev(){
    ( cd "$ROOT" && timeout 600 "$ROOT/selfhost/devbuild.sh" pxc > "$W/devbuild.log" 2>&1 )
  }
  neg_case(){  # $1=标签 $2=桩脚本
    restore_all
    python3 "$2" "$TGT" > "$W/neg_$1.stamp" 2>&1 || { bad "[5-$1] 打桩失败（见 $W/neg_$1.stamp）"; return 1; }
    rebuild_dev || true
    [ -x "$PXDEV" ] || { bad "[5-$1] dev 件重建失败"; return 1; }
    build_one srv "$PXDEV" >/dev/null 2>&1; build_one cli "$PXDEV" >/dev/null 2>&1
    if [ ! -x "$W/bin_srv" ]; then bad "[5-$1] 负控档构建失败"; return 1; fi
    run_case "$W/bin_srv" "$W/bin_cli" "$((PORT + 1))" "$1"
    restore_all
    rebuild_dev >/dev/null 2>&1 || true
  }
  # A：撤回 511 ⇒ [4a] 必须判红
  if neg_case neg511 "$W/neg511.py"; then
    NEG_A_READ="$EV_LISTEN_AFTER"
    if [ "$EV_LISTEN_AFTER" = "no" ]; then ok "[5-A] 撤回 511 ⇒ 端口消失（判据有牙）"
    else bad "[5-A] 撤回 511 后端口仍在监听（判据没牙）"; fi
  fi
  # B：撤回 512 ⇒ [4b] 必须判红
  if neg_case neg512 "$W/neg512.py"; then
    if [ "$EV_ACCEPT2" -le 0 ]; then ok "[5-B] 撤回 512 ⇒ accept#2 失败（判据有牙）"
    else bad "[5-B] 撤回 512 后 accept#2 仍成功（判据没牙）"; fi
  fi
  # C：判据自伤 —— 取**负控 A 的实测读数**（端口确已消失），再开 M296_FORCE_PASS
  #    ⇒ 判据必须**不再报红**（证明上面那条红来自判据本身，而不是别的东西顺带产生的）
  echo "  [5-C] 判据自伤：同一份负控 A 读数下，废掉判据 ⇒ 红必须消失"
  if [ "${NEG_A_READ:-}" = "no" ]; then
    M296_FORCE_PASS=1
    if [ "$(judge_listen_ok)" = "yes" ]; then ok "[5-C] 自伤生效（负控 A 的 no 被 M296_FORCE_PASS 抹成 yes ⇒ 红消失）"
    else bad "[5-C] 自伤无效（判据没被废掉）"; fi
    M296_FORCE_PASS=0
  else
    bad "[5-C] 缺少负控 A 的读数（NEG_A_READ=${NEG_A_READ:-空}）"
  fi
  restore_all
fi

# ── [6] 覆盖边界 ────────────────────────────────────────────
echo "[6] 覆盖边界（如实）"
cat <<'TXT'
     · 只覆盖 `quic_listen` + `quic_accept`（**手写服务端**）形态；`px_serve` / `quic_h3_listen`
       的**托管**路径不在面内（托管连接的 fd 共享同源，511 的判据对它同样生效，但其
       「语言层 close」在现版本不可达 —— 本次只加了守卫，未做端到端观测）。
     · 第二条连接是**串行**接的（A 关掉之后才起 B）。**并发**多条同时在线不在本门面内
       （那是 M293/M294 的门：`m293_h3_robustness` / `m294_h3_qpack_share`）。
     · 缺陷 512 修的是「首包无效就丢弃」，**未**度量「噪声包洪泛下的 accept 延迟」
       （deadline 保护不变，但每丢一个包会短暂分配一个 conn 槽再释放）。
     · `quic_migrate`（客户端迁移）路径的同族 fd 处理**未在本门覆盖**（缺陷 504 待修）。
TXT

echo ""
echo "══ 汇总：$PASS 通过 / $FAIL 失败 ══"
[ "$FAIL" = "0" ] && { echo "M296-VERIFY-OK"; exit 0; } || { echo "M296-VERIFY-FAIL"; exit 1; }
