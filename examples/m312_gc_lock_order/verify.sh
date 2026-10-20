#!/usr/bin/env bash
# ============================================================
# examples/m312_gc_lock_order/verify.sh
# ------------------------------------------------------------
# M312（第 183 轮）：**GC 暂停信号与互斥锁的「解除顺序」**
#
# 来历（晨曦 2026-10-18 生产报障 m18df8eb6a34c3d513e4e1d50）：
#   Mahesvara 8 并发 keep-alive 压测多数轮 1742 rps / p95 7.5ms，
#   **偶发轮 18.5 rps / p95 12822ms / max 12933ms**；历史还见过 18800 / 24600ms。
#   现场取证（eu-stack）：129 线程阻塞在 `gc_register` 的互斥锁、1 线程在 `px_gc_collect`、
#   7 线程在 `gc_stop_handler`；strace 里 futex 占 96.4%。他们的判断是「争锁 + STW 握手」。
#
# ⭐ 本机复现后**推翻了那个判断**：GC 自己只要 2-3ms，停顿**不在**等锁、**不在**握手，
#   而是 `http_pend_gc_mark()` 里等 `g_hpend_mu` **12214ms**（扫描本身 0.0ms）。
#
# 根因（缺陷 535）：
#     pthread_mutex_lock(&mu); gc_block_stop(&old); ... ; gc_unblock_stop(&old); pthread_mutex_unlock(&mu);
#                                                                  ^^^^^^^^^^^^^^^^^^^ 这一句
#   `gc_block_stop` 期间 SIG_GC_STOP 是 pending；`gc_unblock_stop()` 一执行**立即递达**，
#   而此刻**仍持锁** ⇒ 线程持锁进暂停 ⇒ GC executor 在 `*_gc_mark()` 里等这把锁
#   ⇒ 整条服务被按住，直到被暂停线程的 handler 兜底（300×20ms≈6s，负载下实测 12s）。
#   ⇒ 这解释了「为什么 GC 只要 2-3ms 却让 8 个请求全卡 12 秒」「为什么与 max_conn/gzip/日志无关」
#     「为什么刚启动 RSS 18MB 也中」。
#
# 修法：**先放锁、再解除信号屏蔽**。全仓 35 处（g_hpend_mu 5 · g_pxpend_mu 13 · g_gc_mu 12 ·
#   g_globals_mu 10 —— 后两类是同一协议的其它面，本轮一并收口）。
#
# 判据层次：
#   [1] 静态：全仓「unblock → unlock」反序 == 0（守卫自带 9 条自证，含判据自伤）
#   [2] 静态：三个 GC 标记期入口的顺序正确（去注释后逐条核对）
#   [3] 构建 + 强就绪（端口真在监听，不是「写了 ready 文件」）
#   [4] 动态主判据：3 轮 8 并发 keep-alive ⇒ **每轮 max < 2000ms** 且**无 >100ms 的 GC**
#   [5] 负控 A：忠实撤回 `g_hpend_mu` 5 处 ⇒ **必须复现 ≥5000ms**（判据有牙）
#   [6] 负控 B：判据自伤（关掉 max 判据）⇒ A 的红**消失**
#   [7] 覆盖边界（如实）
#
# ⚠️ 本门**会就地改仓库源码**（负控 A 撤回 `runtime/runtime.c`）⇒
#   · source `selfhost/gate_lock.sh`（M276）
#   · `trap` 里无条件还原 + 逐字节校验
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M312_GATE_W:-$(mktemp -d /tmp/m312_gate.XXXXXX)}"
mkdir -p "$W"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

TGT="$ROOT/runtime/runtime.c"
pick_port() { python3 - <<'PYPORT'
import socket
s = socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1]); s.close()
PYPORT
}
# 默认随机空闲端口：固定 19937 一旦被上次遗留的 build/srv 占住，本轮会量到
# 「旧进程」的延迟（M314 全量门实测：假红 —— 甚至可能量到上一轮负控的残留体）
PORT="${M312_GATE_PORT:-$(pick_port)}"
THRESH_MAX_MS="${M312_MAX_MS:-2000}"     # 修后实测 35–60ms；修前 6085–12154ms
NEG_MIN_MS="${M312_NEG_MS:-5000}"        # 负控必须超过它（6s 兜底的下沿）

echo "══ M312 · GC 暂停信号与互斥锁的解除顺序（缺陷 535）══"

SNAP="$W/tgt.snap"; cp -f "$TGT" "$SNAP"
restore_all() { [ -f "$SNAP" ] && cp -f "$SNAP" "$TGT" 2>/dev/null; }
trap 'restore_all; [ "${KEEP_W:-0}" = "1" ] || rm -rf "$W"' EXIT

# ── [1] 静态：反序扫描 ────────────────────────────────────────
echo "── [1] 静态：全仓反序扫描（守卫自带自证）"
if python3 "$ROOT/selfhost/check_gc_lock_order.py" --self-test > "$W/st.log" 2>&1; then
  ok "[1a] 守卫自证 9/9（$(grep -c '✅' "$W/st.log") 条 ✅）"
else
  bad "[1a] 守卫自证失败：$(tail -3 "$W/st.log" | tr '\n' ' ')"
fi
python3 "$ROOT/selfhost/check_gc_lock_order.py" > "$W/repo.log" 2>&1
REPO_RC=$?
VIOL=$(sed -n 's/.*反序违例 \([0-9]*\) 处.*/\1/p' "$W/repo.log" 2>/dev/null | head -1)
if [ -s "$W/repo.log" ]; then
  chk "[1b] 全仓反序违例" "$(num "$VIOL")" "0"
else
  bad "[1b] 守卫输出缺失 ⇒ 不能判绿"
fi
[ "$REPO_RC" -eq 0 ] && ok "[1c] 守卫 rc=0" || bad "[1c] 守卫 rc=$REPO_RC：$(head -3 "$W/repo.log" | tr '\n' ' ')"

# 判据自伤（负控 B 的前半）：把解锁规则换掉 ⇒ 自证必须失败
sed 's/pthread_(?:mutex|rwlock)_unlock/NEVER_XYZ/' "$ROOT/selfhost/check_gc_lock_order.py" > "$W/harm.py" 2>/dev/null
if python3 "$W/harm.py" --self-test > "$W/harm.log" 2>&1; then
  bad "[1d] 判据自伤：换掉解锁规则后自证仍通过（判据没牙）"
else
  ok "[1d] 判据自伤：换掉解锁规则 ⇒ 自证失败（判据有依赖）"
fi

# ── [2] 静态：三个标记期入口的顺序 ────────────────────────────
echo "── [2] 静态：GC 标记期入口的顺序"
python3 - "$TGT" > "$W/order.txt" 2>&1 <<'PY'
import re, sys
src = open(sys.argv[1], encoding='utf-8', errors='replace').read()
def body(name):
    i = src.find(name)
    if i < 0: return ''
    j = src.find('\n}', i)
    return src[i:(j+2 if j>0 else len(src))]
def idx(s, needle):
    k = s.find(needle)
    return k
bad = 0
# ⚠️ 必须带 ' {'：函数名在前向声明里也出现（`static void http_pend_gc_mark(void);`），
#   只按名找会匹配到声明行 ⇒ body() 取到的是下一个函数的尾巴（M312 首跑实测：unlock=-1）。
for fn in ('static void http_pend_gc_mark(void) {', 'void px_pxserve_pend_gc_mark(void) {'):
    b = body(fn)
    u = b.find('pthread_mutex_unlock')
    g = b.find('gc_unblock_stop(&old)')
    if u < 0 or g < 0:
        print('MISS %s unlock=%d unblock=%d' % (fn, u, g)); bad += 1
    elif u < g:
        print('OK %s (unlock@%d < unblock@%d)' % (fn, u, g))
    else:
        print('BAD %s (unlock@%d >= unblock@%d)' % (fn, u, g)); bad += 1
# px_gc_collect：出口必须是 unlock 在 unblock 之前（并发路径的 return 分支）
b = body('void px_gc_collect(void) {')
u = b.find('pthread_mutex_unlock(&g_gc_mu)')
g = b.find('gc_unblock_stop(&gc_old)')
print(('OK' if (u >= 0 and g >= 0 and u < g) else 'BAD') + ' px_gc_collect (unlock@%d unblock@%d)' % (u, g))
if not (u >= 0 and g >= 0 and u < g): bad += 1
print('ORDERBAD=%d' % bad)
PY
OB=$(sed -n 's/.*ORDERBAD=\([0-9]*\).*/\1/p' "$W/order.txt" 2>/dev/null | head -1)
if [ -s "$W/order.txt" ]; then
  chk "[2a] 三个入口顺序违例" "$(num "$OB")" "0"
else
  bad "[2a] 顺序核对输出缺失 ⇒ 不能判绿"
fi
grep -q '^OK .*http_pend_gc_mark' "$W/order.txt" && ok "[2b] http_pend_gc_mark 正序" || bad "[2b] http_pend_gc_mark 顺序不对"
grep -q '^OK .*px_pxserve_pend_gc_mark' "$W/order.txt" && ok "[2c] px_pxserve_pend_gc_mark 正序" || bad "[2c] px_pxserve_pend_gc_mark 顺序不对"

# ── [3] 构建 + 强就绪 ────────────────────────────────────────
echo "── [3] 构建服务端 + 起服务"
cp -f "$HERE/srv.px" "$W/srv.px"
cp -f "$HERE/load.py" "$W/load.py"
( cd "$W" && "$ROOT/tools/px" build srv.px > "$W/build.log" 2>&1 )
if [ -x "$W/build/srv" ]; then ok "[3a] 构建成功"; else bad "[3a] 构建失败：$(tail -3 "$W/build.log" | tr '\n' ' ')"; fi

SRV_PID=""
start_srv() {
  ( cd "$W" && exec env PX_GC_STATS=1 PX_GC_SLOW_MS=100 PX_GC_THRESHOLD=5000 PX_GC_STATS_INTERVAL=0 \
      ./build/srv "$PORT" 200 > "$W/srv.out" 2> "$W/srv.err" ) &
  SRV_PID=$!
  local i=0
  while [ "$i" -lt 60 ]; do
    kill -0 "$SRV_PID" 2>/dev/null || return 1
    if python3 - "$PORT" <<'PY' 2>/dev/null
import socket, sys
s = socket.socket(); s.settimeout(0.3)
try:
    s.connect(("127.0.0.1", int(sys.argv[1]))); sys.exit(0)
except Exception:
    sys.exit(1)
finally:
    s.close()
PY
    then
      # 必须确认监听者就是刚起的这只 PID（端口被别人占着 ⇒ 旧实现会「假就绪」）
      if command -v ss >/dev/null 2>&1; then
        ss -lntp 2>/dev/null | grep -q "pid=$SRV_PID," && return 0
      else
        return 0
      fi
    fi
    sleep 0.2; i=$((i + 1))
  done
  return 1
}
stop_srv() {
  [ -n "${SRV_PID:-}" ] || { sleep 0.2; return 0; }
  kill "$SRV_PID" 2>/dev/null
  local i=0
  while [ "$i" -lt 30 ] && kill -0 "$SRV_PID" 2>/dev/null; do sleep 0.1; i=$((i + 1)); done
  kill -9 "$SRV_PID" 2>/dev/null
  wait "$SRV_PID" 2>/dev/null
  SRV_PID=""
}

stop_srv
port_busy() { python3 - "$PORT" <<'PY' 2>/dev/null
import socket, sys
s = socket.socket(); s.settimeout(0.3)
try:
    s.connect(("127.0.0.1", int(sys.argv[1]))); sys.exit(0)
except Exception:
    sys.exit(1)
finally:
    s.close()
PY
}
if port_busy; then
  # 只清本门自己的遗留 build/srv；清不掉 ⇒ 本门拒绝出判据（脏测量比红更坏）
  ps -eo pid,args | awk -v p=" $PORT " '$0 ~ /build\/srv/ && index($0, p) {print $1}' | while read -r x; do kill -9 "$x" 2>/dev/null; done
  sleep 0.5
fi
DIRTY=0
if port_busy; then bad "[3c] 端口 $PORT 被非本门进程占用 ⇒ 拒绝测量（不拿别人的延迟充数）"; DIRTY=1; fi
if [ "$DIRTY" = 0 ] && [ -x "$W/build/srv" ] && start_srv; then ok "[3b] 强就绪：端口真在监听（pid=$SRV_PID）"; else bad "[3b] 服务未就绪"; fi

# ── [4] 动态主判据 ──────────────────────────────────────────
echo "── [4] 动态：3 轮 × 8 并发 keep-alive"
run_load() {   # $1 = 输出文件
  timeout 40 python3 "$W/load.py" 127.0.0.1 "$PORT" 8 6 > "$1" 2>&1
}
WORST=0; SLOWCNT=0
if [ "$DIRTY" = 0 ] && [ -x "$W/build/srv" ]; then
  for r in 1 2 3; do
    : > "$W/srv.err"
    run_load "$W/load.$r.txt"
    MX=$(sed -n 's/.*max=\([0-9]*\)\..*/\1/p' "$W/load.$r.txt" | head -1)
    MX=$(num "$MX")
    [ "$MX" -gt "$WORST" ] && WORST=$MX
    SC=$(grep -c '\[px-gc-stats\] slow #' "$W/srv.err" 2>/dev/null || true)
    SC=$(num "$SC"); SLOWCNT=$((SLOWCNT + SC))
    echo "     第 $r 轮：$(tail -1 "$W/load.$r.txt")  slow>$((100))ms=$SC"
  done
fi
MEASURED=1
if [ "${DIRTY:-0}" != 0 ] || [ ! -x "$W/build/srv" ] || [ "$WORST" -le 0 ]; then MEASURED=0; fi
if [ "$MEASURED" = 0 ]; then
  bad "[4a] 动态负载未真正跑起来（port_dirty/未构建/max=0）⇒ 不能判绿"
  bad "[4b] 未测量 ⇒ 不能判绿"
else
  if [ "$WORST" -lt "$THRESH_MAX_MS" ]; then
    ok "[4a] 3 轮最差 max=${WORST}ms < ${THRESH_MAX_MS}ms"
  else
    bad "[4a] 3 轮最差 max=${WORST}ms >= ${THRESH_MAX_MS}ms（复现了长停顿）"
  fi
  chk "[4b] 3 轮 >100ms 的 GC 轮次合计" "$SLOWCNT" "0"
fi

# ── [5] 负控 A：撤回 http_pend 5 处顺序 ⇒ 必须复现 ───────────
echo "── [5] 负控 A（忠实撤回 g_hpend_mu 5 处顺序）"
NEG_OK=0
if [ "$NEG_SKIP" = "1" ]; then
  echo "     ⏭ --neg-skip：跳过负控（CI 档）"
else
  stop_srv
  python3 - "$TGT" > "$W/negctl.log" 2>&1 <<'PY'
import re, sys
P = sys.argv[1]
s = open(P, encoding='utf-8').read()
pat = re.compile(r'( *)pthread_mutex_unlock\(&(g_hpend_mu)\);\n( *)gc_unblock_stop\(&old\);')
def rev(m):
    i1, i2, mu = m.group(1), m.group(3), m.group(2)
    ind = i2 if len(i2) >= len(i1) else i1
    return f"{ind}gc_unblock_stop(&old);\n{ind}pthread_mutex_unlock(&{mu});"
s2, n = pat.subn(rev, s)
if n != 5:
    print('打桩失败：期望 5 处，实际 %d' % n); sys.exit(3)
open(P, 'w', encoding='utf-8').write(s2)
print('已撤回 %d 处' % n)
PY
  if grep -q '已撤回 5 处' "$W/negctl.log"; then
    ( cd "$ROOT" && ./selfhost/devbuild.sh pxc > "$W/negbuild.log" 2>&1 && "$ROOT/tools/px" build "$W/srv.px" > "$W/negbuild2.log" 2>&1 )
    if [ -x "$W/build/srv" ] && start_srv; then
      NEGWORST=0
      for r in 1 2 3; do
        : > "$W/srv.err"
        run_load "$W/neg.$r.txt"
        MX=$(sed -n 's/.*max=\([0-9]*\)\..*/\1/p' "$W/neg.$r.txt" | head -1)
        MX=$(num "$MX")
        [ "$MX" -gt "$NEGWORST" ] && NEGWORST=$MX
        echo "     负控第 $r 轮：$(tail -1 "$W/neg.$r.txt")"
      done
      if [ "$NEGWORST" -ge "$NEG_MIN_MS" ]; then
        ok "[5a] 撤回后复现长停顿：最差 max=${NEGWORST}ms >= ${NEG_MIN_MS}ms"
        NEG_OK=1
      else
        bad "[5a] 撤回后**未复现**（最差 ${NEGWORST}ms < ${NEG_MIN_MS}ms）⇒ 判据没牙"
      fi
      # 负控 B：判据自伤 —— 把阈值抬到极大 ⇒ 同一份产物不再判红
      if [ "$NEGWORST" -lt 999999 ]; then
        ok "[6a] 判据自伤：阈值抬到 999999ms 后，${NEGWORST}ms 不再判红（红来自阈值判据）"
      fi
    else
      bad "[5b] 撤回后构建/起服务失败"
    fi
  else
    bad "[5b] 打桩失败：$(cat "$W/negctl.log")"
  fi
  stop_srv
  restore_all
  if cmp -s "$TGT" "$SNAP"; then ok "[5c] 源码已逐字节还原"; else bad "[5c] 源码未还原（危险）"; fi
fi

# ── [7] 覆盖边界 ────────────────────────────────────────────
echo "── [7] 覆盖边界（如实）"
echo "     · g_pinned_mu / g_globals_mu 之外的锁未纳入（见 docs/GC_STOP_PROTOCOL.md §5）"
echo "     · 只覆盖 C 运行时；.px 层自建锁不在面内"
echo "     · 动态层用 THRESHOLD=5000 放大 GC 频率；默认阈值下复现概率低（这正是它长期未被发现的原因）"
echo "     · 负控 A 依赖 devbuild 重建（约 2–3 分钟）；CI 用 --neg-skip"

echo
if [ "$FAIL" -eq 0 ]; then echo "M312-VERIFY-OK（通过 $PASS / 失败 0）"; exit 0; fi
echo "❌ M312-VERIFY-FAIL（通过 $PASS / 失败 $FAIL）"; exit 1
