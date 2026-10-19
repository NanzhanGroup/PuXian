#!/usr/bin/env bash
# ============================================================
# examples/m313_accept_nodelay/verify.sh
# ------------------------------------------------------------
# M313（第 184 轮 · 缺陷 539）：**accept 出的 socket 没设 TCP_NODELAY**
#
# 来历（晨曦 2026-10-19 证据包 wsa-chenxi-puxian-gc-evidence-20261019.tar.gz）：
#   他们在生产给 accept 出的 socket 补了 3 行（TCP_NODELAY），实测：
#     · 补前：响应头→体 gap 40.6ms / 单连接 23.5 rps
#     · 补后：gap 0.02ms / 1042–1622 rps
#   ⇒ 他们要求并入上游（否则每次 dnf upgrade 都要手工重打，且已经漏过一次）。
#
# 根因：`tcp_connect_ex` 早在 M149 就给**出站**连接设了 NODELAY（对齐 Go net.Dial），
#   而**入站 accept 这一侧一直漏着** —— 同一条语义只做了一半（M191/M203/M230 同族）。
#
# ⭐ 判据为什么不量 40ms：Nagle + 延迟 ACK 只在**真实网络/特定对端**显形，
#   **本机回环上 ACK 是即时的**（我方实测：有无 NODELAY，回环 gap 恒 0ms）
#   ⇒ 量墙钟会得到「假绿」。所以主判据用**可判定的状态位**：
#   accept 之后 `tcp_opt(cfd, {})` 回读 `nodelay`（getsockopt 生效值）。
#
# 判据层次：
#   [1] 静态：px_io_set_nodelay 存在且设 TCP_NODELAY · px_io_accept 成功分支调用它 ·
#              runtime 里**没有**绕过 px_io_accept 的裸 accept（否则旁路漏设）· 调用点 ≥5
#   [2] 构建（裸 TCP 服务端 + 客户端）
#   [3] 动态主判据：ACCEPT-NODELAY=true 且响应可用（CLI-BODY=50）
#   [4] 时间观察（**仅信息项**）：回环 gap —— 不作为硬判据，理由见上
#   [5] 负控：忠实撤回 setsockopt + 重建 ⇒ 必须 false（判据有牙）
#   [6] 覆盖边界（如实）
#
# ⚠️ 负控会**就地改 runtime/runtime.c**（M312 同款）⇒ source gate_lock.sh + snapshot/restore + 逐字节校验。
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M313N_GATE_W:-$(mktemp -d /tmp/m313n_gate.XXXXXX)}"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

RT="$ROOT/runtime/runtime.c"
PORT=""
SRV_PID=""
cleanup() {
  [ -n "$SRV_PID" ] && kill "$SRV_PID" 2>/dev/null
  [ -f "$W/rt.snap" ] && cp -f "$W/rt.snap" "$RT" 2>/dev/null
  [ "${KEEP_W:-0}" = "1" ] || rm -rf "$W"
}
trap cleanup EXIT

echo "══ M313 · accept 侧 TCP_NODELAY（缺陷 539）══"

cp -f "$RT" "$W/rt.snap"

# ── [1] 静态 ─────────────────────────────────────────────────
echo "── [1] 静态：设置点 / 调用点 / 无裸 accept 旁路"
# 静态：从**函数体**里判（全局 grep 会撞 tcp_connect_ex / tcp_opt 里同款调用 —— 判据要贴契约写）
SETND=$(python3 - "$RT" <<'PY'
import re, sys
src = open(sys.argv[1], encoding='utf-8').read()
m = re.search(r'static void px_io_set_nodelay\s*\([^)]*\)\s*\{', src)
if not m:
    print('NO-FUNC'); raise SystemExit
i = m.end() - 1; depth = 0; j = i
while j < len(src):
    if src[j] == '{': depth += 1
    elif src[j] == '}':
        depth -= 1
        if depth == 0: j += 1; break
    j += 1
body = src[m.end():j]
n = len(re.findall(r'setsockopt\s*\(\s*fd\s*,\s*IPPROTO_TCP\s*,\s*TCP_NODELAY', body))
print(n)
PY
)
chk "[1.1] px_io_set_nodelay 函数体里设 TCP_NODELAY" "$SETND" "1"
chk "[1.2] px_io_accept 成功分支调用 px_io_set_nodelay" "$(grep -c 'if (r >= 0) { px_io_set_nodelay(r); return r; }' "$RT")" "1"
SITES=$(grep -rn 'px_io_accept(' "$ROOT/runtime/"*.c | grep -v 'int px_io_accept' | wc -l)
if [ "$SITES" -ge 5 ]; then ok "[1.3] 共用同一 accept 入口（调用点 $SITES 处 ≥5：tcp_accept / px_serve / ws / sse / …）"; else bad "[1.3] 调用点只有 $SITES 处（<5）"; fi
BARE=$(python3 - "$ROOT" <<'PY'
import re, sys, glob, os
root = sys.argv[1]
bad = 0
for p in glob.glob(os.path.join(root, 'runtime', '*.c')):
    src = open(p, encoding='utf-8', errors='replace').read()
    # 去注释（保留字面量）
    def strip(s):
        out, i, n = [], 0, len(s)
        while i < n:
            if s.startswith('/*', i):
                j = s.find('*/', i + 2); i = n if j < 0 else j + 2
            elif s.startswith('//', i):
                j = s.find('\n', i); i = n if j < 0 else j
            elif s[i] == '"':
                out.append(s[i]); i += 1
                while i < n and s[i] != '"':
                    if s[i] == '\\': out.append(s[i]); i += 1
                    if i < n: out.append(s[i]); i += 1
                i += 1; out.append('"')
            else:
                out.append(s[i]); i += 1
        return ''.join(out)
    code = strip(src)
    m = re.search(r'int px_io_accept\s*\([^)]*\)\s*\{', code)
    if m:
        # 取函数体（按花括号配平）
        i = m.end() - 1; depth = 0; j = i
        while j < len(code):
            if code[j] == '{': depth += 1
            elif code[j] == '}':
                depth -= 1
                if depth == 0: j += 1; break
            j += 1
        code = code[:m.start()] + code[j:]
    bad += len(re.findall(r'(?<![\w.>])accept\s*\(', code))
print(bad)
PY
)
chk "[1.4] 无绕过 px_io_accept 的裸 accept（否则旁路漏设）" "$(num "$BARE")" "0"

# ── [2] 构建 ─────────────────────────────────────────────────
echo "── [2] 构建（裸 TCP 服务端 + 客户端）"
BOK=1
for f in "$HERE/srv.px" "$HERE/cli.px"; do
  if ! out=$("$ROOT/tools/px" build "$f" 2>&1); then echo "$out" | tail -3; BOK=0; fi
done
chk "[2.1] 构建成功" "$BOK" "1"
SRV="$HERE/build/srv"; CLI="$HERE/build/cli"
for b in "$SRV" "$CLI"; do [ -x "$b" ] || { bad "[2.2] 产物缺失：$b"; BOK=0; }; done
[ "$BOK" = "1" ] && ok "[2.2] 产物齐备（srv / cli）"

# ── 运行装置 ─────────────────────────────────────────────────
run_srv() { # $1=port $2=log ; 设置 SRV_PID
  : > "$2"
  "$SRV" "$1" > "$2" 2>&1 &
  SRV_PID=$!
  local i
  for i in $(seq 1 200); do grep -q 'SRV-READY' "$2" 2>/dev/null && break; sleep 0.05; done
  for i in $(seq 1 100); do ss -ltn 2>/dev/null | grep -q ":$1 " && return 0; sleep 0.05; done
  return 1
}
stop_srv() { [ -n "$SRV_PID" ] && kill "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=""; }

# ── [3] 动态主判据 ───────────────────────────────────────────
echo "── [3] 动态：accept 后回读 nodelay 状态位"
PORT=$((20000 + RANDOM % 500)); SLOG="$W/srv.log"
if run_srv "$PORT" "$SLOG"; then
  ok "[3.0] 服务端就绪且端口 $PORT 在监听"
  OUT="$("$CLI" "$PORT" 2>&1)"; RC=$?
  chk "[3.1] 客户端退出码" "$RC" "0"
  echo "$OUT" | sed 's/^/     /'
  echo "$OUT" | grep -q 'CLI-BODY=50' && ok "[3.2] 连接与响应可用（体 50 字节）" || bad "[3.2] 响应不完整：$OUT"
  sleep 0.3
  ND="$(grep -m1 'ACCEPT-NODELAY=' "$SLOG" | cut -d= -f2)"
  chk "[3.3] accept 出的 socket NODELAY（getsockopt 生效值）" "${ND:-missing}" "true"
else
  bad "[3.0] 服务端未就绪（端口 $PORT）"
fi
stop_srv

# ── [4] 时间观察（仅信息项）──────────────────────────────────
echo "── [4] 时间观察（**仅信息项**，不作硬判据）"
GAP="$(echo "$OUT" | grep -m1 'GAP_MS=' | cut -d= -f2)"
echo "     ℹ️ 响应头→体 到达间隔 = ${GAP:-?} ms"
echo "     ℹ️ 本机回环 ACK 即时 ⇒ 该值通常为 0ms；40ms 的现场只有真实网络/特定对端才显形"
echo "     ℹ️ 所以主判据是 [3.3] 的状态位（可判定），不是这里 —— 不把环境差异伪装成判据"

# ── [5] 负控 ─────────────────────────────────────────────────
if [ "$NEG_SKIP" = "1" ]; then
  echo "── [5] 负控：**已按 --neg-skip 跳过**（CI 档；撤回设置要重建 runtime）"
else
  echo "── [5] 负控：忠实撤回 setsockopt + 重建 ⇒ 必须 false"
  python3 - "$RT" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '    (void)setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));  // 失败不致命（非 TCP / 受限）'
assert s.count(old) == 1, 'anchor not unique'
s = s.replace(old, '    (void)on;  // NEGCTL: 撤回 TCP_NODELAY')
open(p, 'w', encoding='utf-8').write(s)
print('     patched: 撤回 setsockopt（1 处）')
PY
  if ! cmp -s "$RT" "$W/rt.snap"; then
    ok "[5.0] 打桩确实改到了源码"
    if out=$("$ROOT/tools/px" build "$HERE/srv.px" 2>&1); then
      PORT=$((20500 + RANDOM % 400)); SLOG="$W/srv_neg.log"
      if run_srv "$PORT" "$SLOG"; then
        OUT2="$("$CLI" "$PORT" 2>&1)"; stop_srv; sleep 0.2
        ND2="$(grep -m1 'ACCEPT-NODELAY=' "$SLOG" | cut -d= -f2)"
        echo "     撤回档读数：ACCEPT-NODELAY=${ND2:-missing}"
        chk "[5.1] 撤回后必须为 false（判据有牙）" "${ND2:-missing}" "false"
      else bad "[5.2] 撤回档服务端未就绪"; fi
    else bad "[5.2] 撤回档构建失败"; fi
  else
    bad "[5.0] 打桩没有改到源码（锚点失效 —— 判据会假绿）"
  fi
  cp -f "$W/rt.snap" "$RT"
  if cmp -s "$RT" "$W/rt.snap"; then ok "[5.3] 源码已逐字节还原"; else bad "[5.3] 源码未还原"; fi
  # 还原后重建一次，让产物回到修复态（避免后续门误用打桩产物）
  "$ROOT/tools/px" build "$HERE/srv.px" >/dev/null 2>&1 || true
fi

# ── [6] 覆盖边界 ─────────────────────────────────────────────
echo "── [6] 覆盖边界（如实）"
cat <<'EOF'
  ℹ️ ① 本门断言的是 **socket 状态位**（NODELAY 生效值），不是「40ms 一定消失」——
       后者取决于网络路径与对端 ACK 策略，属环境属性，不能当判据。
  ℹ️ ② 覆盖的是 `px_io_accept` 这**一个**入口；[1.3]/[1.4] 守「所有服务端都走它、没有旁路」。
  ℹ️ ③ 已建立连接的 TLS/QUIC 路径不受影响（QUIC 走 UDP，无 Nagle 问题）。
  ℹ️ ④ 服务端到服务端（px 客户端 ⇄ px 服务端）同样受益；出站侧 M149 起已有 NODELAY。
EOF

echo
if [ "$FAIL" -eq 0 ]; then
  echo "══ M313N-VERIFY-OK · $PASS 通过 / 0 失败 ══"
  exit 0
else
  echo "══ M313N-VERIFY-FAIL · $PASS 通过 / $FAIL 失败 ══"
  exit 1
fi
