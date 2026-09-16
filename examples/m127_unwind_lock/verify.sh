#!/usr/bin/env bash
# ============================================================
# M127（qg-issue 84）门：**隔离点回卷的锁安全审计**
# ------------------------------------------------------------
# 病灶（修复前）：M125/M126 把运行时错误从进程级收紧到请求级（longjmp 回隔离点），
#   但 longjmp 不展开 pthread 锁。失败点若落在临界区内（典型：gc_register 持 g_gc_mu 时
#   xrealloc→xmalloc 分配失败），回卷把锁永久留下 ⇒ 其他线程随后阻塞在 pthread_mutex_lock
#   ⇒ **进程既不服务也不退出**（假死：systemd 视为健康、监控看不到异常，比 exit(1) 更难发现）。
#   实测 gdb 现场：#0 __lll_lock_wait #1 pthread_mutex_lock #2 gc_register #3 px_str_len #4 vm_run_loop
# 修法（runtime/locktrack.h + runtime.c）：
#   · 所有运行时 pthread 锁的加/解锁同时记进「本线程持锁栈」（TLS，O(1)、无分配）；
#   · 隔离点 longjmp 之前审计：无锁在身 → 照旧 longjmp（请求级 5xx，与 M125/M126 一致）；
#     有锁在身 → 打印持锁清单并 _exit(1)（保留致命语义，systemd 3s 内拉起）⇒ 绝不留锁。
# 判据：
#   A 正控（**无锁**在身时失败）：仍是请求级 5xx、进程存活、**不**触发审计（无误伤）；
#   B 关键用例（**持 g_gc_mu** 时失败）：进程数秒内 _exit(1)，文案含「无法安全回滚」+ 持锁清单
#     （列出 g_gc_mu）+ 后果 + 处置建议 ⇒ **不假死**；
#   C 负控 PX_UNWIND_LOCK_GUARD=0：同一注入 ⇒ 进程**不退出**且 /health 探测无响应 = **假死
#     复现**，证明「假死」真实存在、且正是审计拦住了它（不是自说自话）；
#   D 无注入回归：/health、/plain、/grow 全 200（/grow 正常返回 objs=6000），无审计行，自然退出码 0；
#   E 启动期注入（无隔离点）：文案为「无隔离点 → 保留致命语义退出」+ 进程退出（M125 语义回归）；
#   F 材料完整性：locktrack.h 存在、记录逃生舱与边界。
# 注 1：必须用**仓库自带** tools/px（PXC_HOME=仓库根 ⇒ 用仓库 runtime/）；
#       /usr/bin/px 是安装版，其 runtime/ 与仓库不同步 ⇒ 会测出旧行为。
# 注 2：/grow 触发点在 **gc_register 持 g_gc_mu 时 xrealloc 扩容 g_objs 对象表**（初始容量 8192）
#       ⇒ 需先把存活对象数推过 8192（故 PX_GC_THRESHOLD 拉高、不做 GC）。
# 用法：bash examples/m127_unwind_lock/verify.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m127_unwind_lock
PX=${PX:-$ROOT/tools/px}
BIN=$ROOT/$DIR/build/unwind_lock
PORT=18099
PAYLOAD=/tmp/m127_payload.bin
GTHRESH=100000000          # 拉高 GC 阈值：保证存活对象数能涨过 g_objs 初始容量 8192
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

probe() {   # → "HTTP/1.1 200 OK" 或空（不用 curl：避免管道依赖）
  ( exec 3<>/dev/tcp/127.0.0.1/$PORT 2>/dev/null &&
    printf 'GET /health HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n' >&3 &&
    timeout 3 head -1 <&3 ) 2>/dev/null | tr -d '\r'
}
req_full() {   # $1=path $2=outfile —— 收全响应（含 body）
  : > "$2"
  ( exec 3<>/dev/tcp/127.0.0.1/$PORT 2>/dev/null &&
    printf 'GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n' "$1" >&3 &&
    timeout 8 cat <&3 ) > "$2" 2>/dev/null
  tr -d '\r' < "$2" | head -1
}

PID=""; LOG=""
start_bin() {   # $1=tag $2=wait(1=等 READY) [env=val ...]
  local tag="$1" need_ready="$2"; shift 2
  pkill -f "$BIN" 2>/dev/null; sleep 0.4
  LOG=/tmp/m127_${tag}.log; : > "$LOG"
  env HOLD_MS=20000 PX_GC_THRESHOLD=$GTHRESH "$@" "$BIN" >> "$LOG" 2>&1 &
  PID=$!
  local i
  if [ "$need_ready" = 1 ]; then
    for i in $(seq 1 60); do grep -q "M127 READY" "$LOG" && break; sleep 0.25; done
    grep -q "M127 READY" "$LOG" || { bad "$tag 启动失败（无 READY）"; PID=""; }
  fi
}
wait_gone() {   # $1=秒 → 0=已退出 1=仍在
  local n=$1 i
  for i in $(seq 1 $((n*4))); do kill -0 "$PID" 2>/dev/null || return 0; sleep 0.25; done
  return 1
}
kill9() { kill -9 "$PID" 2>/dev/null; wait "$PID" 2>/dev/null; }

echo "== M127 验证（隔离点回卷的锁安全审计：绝不留锁 ⇒ 绝不假死）=="

echo "[0/6] 准备载荷 + 编译"
head -c 8388609 /dev/zero | tr '\0' 'A' > "$PAYLOAD"
if "$PX" build "$DIR/unwind_lock.px" > /tmp/m127_build.log 2>&1 && [ -x "$BIN" ]; then
  ok "编译成功（$(stat -c%s "$BIN") 字节）"
else
  bad "编译失败"; tail -20 /tmp/m127_build.log | sed 's/^/     /'; exit 1
fi
if grep -qa "无法安全回滚" "$BIN"; then   # grep -a：strings 默认只出 7-bit ASCII
  ok "二进制含 M127 审计文案（确认用的是仓库 patched runtime）"
else
  bad "二进制不含 M127 审计文案 —— runtime 未重编，本门无效"; exit 1
fi

echo
echo "[1/6] A 正控：**无锁在身**时注入分配失败（PX_ALLOC_FAIL_MIN）"
start_bin A 1 PX_ALLOC_FAIL_MIN=8388610
if [ -n "$PID" ]; then
  H=$(req_full /plain /tmp/m127_A_plain.out); echo "     /plain → '$H'"
  case "$H" in *" 5"*) ok "A 大分配失败转成请求级 5xx（进程未死）";; *) bad "A /plain 异常：'$H'";; esac
  [ "$(probe)" = "HTTP/1.1 200 OK" ] && ok "A /health 仍 200（服务未中断）" || bad "A /health 非 200"
  grep -q "无法安全回滚" "$LOG" && bad "A 无锁场景竟触发锁审计（误伤！）" || ok "A 未触发锁审计（无锁场景零影响）"
  grep -q "px-coro" "$LOG" && ok "A 走的是常规 longjmp 隔离（[px-coro] 行）" || bad "A 未见常规隔离行"
  if wait_gone 30; then
    wait "$PID" 2>/dev/null; RC=$?
    grep -q "M127 MAIN-EXIT-NORMAL" "$LOG" && [ "$RC" = 0 ] \
      && ok "A 主循环跑到自然退出、退出码 0（隔离未影响宿主）" || bad "A 退出异常 rc=$RC"
  else bad "A 未自然退出（假死）"; kill9; fi
fi

echo
echo "[2/6] B 关键：**持 g_gc_mu** 时注入失败 ⇒ 必须 _exit(1)（绝不留锁假死）"
# M128 说明：默认路径上 px_list_push/px_dict_set/gc_register 的扩容分配已移到**临界区之外**
#   （这正是 M128 的成果，见 examples/m128_unlock_grow）⇒ 本用例需显式把扩容压回**锁内**
#   （PX_GROW_RETRY_MAX=0 = 兜底路径，与 M127 及以前的行为逐字一致）才能验收审计闸本身。
start_bin B 1 PX_GROW_RETRY_MAX=0 PX_ALLOC_FAIL_IN_LOCK=g_gc_mu
if [ -n "$PID" ]; then
  R=$(req_full /grow /tmp/m127_B_grow.out); echo "     /grow → '${R:-（无响应：进程已退出）}'"
  if wait_gone 10; then ok "B 进程在 10s 内退出（未假死）"; else bad "B 进程 10s 后仍存活 = 假死"; kill9; fi
  grep -q "PX_ALLOC_FAIL_IN_LOCK 注入" "$LOG" && ok "B 注入确实在临界区内触发（同一条失败路径）" || bad "B 未触发临界区内注入"
  grep -q "无法安全回滚" "$LOG" && ok "B 打印了「无法安全回滚」审计行" || bad "B 无审计行"
  grep -q "g_gc_mu" "$LOG" && ok "B 持锁清单列出了 g_gc_mu（可定位）" || bad "B 清单未列出 g_gc_mu"
  grep -q "进程既不服务也不退出" "$LOG" && ok "B 文案说明了假死后果" || bad "B 文案缺后果说明"
  grep -q "_exit(1)" "$LOG" && ok "B 文案说明了保留致命语义（_exit(1)）" || bad "B 文案未说明处置"
  grep -q "处理建议" "$LOG" && ok "B 给出了处理建议（移出临界区）" || bad "B 无处理建议"
  grep -q "px-coro" "$LOG" && bad "B 竟走了 longjmp（审计未生效）" || ok "B 未走 longjmp（审计已拦截）"
fi

echo
echo "[3/6] C 负控：同一注入 + PX_UNWIND_LOCK_GUARD=0 ⇒ **必须复现假死**"
start_bin C 1 PX_GROW_RETRY_MAX=0 PX_ALLOC_FAIL_IN_LOCK=g_gc_mu PX_UNWIND_LOCK_GUARD=0
if [ -n "$PID" ]; then
  R=$(req_full /grow /tmp/m127_C_grow.out); echo "     /grow → '${R:-（无响应）}'"
  sleep 2
  kill -0 "$PID" 2>/dev/null && ok "C 进程未退出（假死：进程还在，systemd 视为健康）" || bad "C 进程退出了（负控无效，需查明）"
  H=$(probe); echo "     /health → '${H:-（无响应/超时）}'"
  [ -z "$H" ] && ok "C /health 无响应 = 留锁假死复现（正是 M127 要消灭的状态）" || bad "C /health 仍 200（未复现假死：$H）"
  grep -q "PX_ALLOC_FAIL_IN_LOCK 注入" "$LOG" && ok "C 与 B 踩中同一失败点（对照有效）" || bad "C 未触同一失败点（对照无效）"
  grep -q "无法安全回滚" "$LOG" && bad "C 逃生舱下仍打审计行（逃生舱未生效）" || ok "C 逃生舱生效：无条件 longjmp"
  kill9
fi

echo
echo "[4/6] D 无注入回归：正常服务零行为变化"
start_bin D 1
if [ -n "$PID" ]; then
  [ "$(probe)" = "HTTP/1.1 200 OK" ] && ok "D /health 200" || bad "D /health 非 200"
  P=$(req_full /plain /tmp/m127_D_plain.out); case "$P" in *" 200"*) ok "D /plain 200（无注入时大文件读取正常）";; *) bad "D /plain 异常：'$P'";; esac
  G=$(req_full /grow /tmp/m127_D_grow.out);   case "$G" in *" 200"*) ok "D /grow 200（对象表扩容路径正常）";; *) bad "D /grow 异常：'$G'";; esac
  grep -q "objs=6000" /tmp/m127_D_grow.out && ok "D /grow body = objs=6000（扩容完整跑完）" || bad "D /grow 未正常完成"
  grep -q "无法安全回滚" "$LOG" && bad "D 正常路径出现审计行（假阳性！）" || ok "D 无审计行（零假阳性）"
  grep -q "运行时错误" "$LOG" && bad "D 正常路径出现运行时错误行" || ok "D 无运行时错误行"
  if wait_gone 30; then
    wait "$PID" 2>/dev/null; RC=$?
    [ "$RC" = 0 ] && grep -q "M127 MAIN-EXIT-NORMAL" "$LOG" && ok "D 自然退出码 0（零回归）" || bad "D 退出异常 rc=$RC"
  else bad "D 未自然退出（假死）"; kill9; fi
fi

echo
echo "[5/6] E 启动期注入（无隔离点）⇒ 保留致命语义 + 文案区分（M125 语义回归）"
start_bin E 0 PX_ALLOC_FAIL_MIN=1024
if [ -n "$PID" ]; then
  if wait_gone 8; then ok "E 启动期失败即退出（未假死）"; else bad "E 启动期失败却未退出"; kill9; fi
  grep -q "无隔离点 → 保留致命语义退出" "$LOG" && ok "E 文案区分「无隔离点」" || bad "E 文案未区分隔离点"
  grep -q "无法安全回滚" "$LOG" && bad "E 无隔离点场景误报锁审计" || ok "E 未误报锁审计"
fi

echo
echo "[6/6] F 材料完整性"
[ -f "$ROOT/runtime/locktrack.h" ] && ok "F runtime/locktrack.h 存在" || bad "F 缺 locktrack.h"
grep -q "PX_UNWIND_LOCK_GUARD" "$ROOT/runtime/locktrack.h" && ok "F 头文件记录了逃生舱" || bad "F 头文件未记逃生舱"
grep -q "边界（如实" "$ROOT/runtime/locktrack.h" && ok "F 头文件记录了边界（cond_wait / 覆盖范围 / 计数平衡）" || bad "F 头文件未记边界"
for tu in runtime.c coro.c runtime_ws.c runtime_route.c runtime_sqlite.c runtime_quic.c; do
  grep -q 'include "locktrack.h"' "$ROOT/runtime/$tu" || bad "F $tu 未接入 locktrack.h"
done
ok "F 6 个持锁 TU 均已接入 locktrack.h（runtime.c / coro.c / ws / route / sqlite / quic）"

pkill -f "$BIN" 2>/dev/null
echo
echo "==================== M127 门结果：pass=$pass fail=$fail ===================="
[ "$fail" = 0 ] || exit 1
