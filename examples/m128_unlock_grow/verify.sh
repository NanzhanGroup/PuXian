#!/usr/bin/env bash
# ============================================================
# M128（qg-issue 85）门：**扩容分配移出临界区**
# ------------------------------------------------------------
# 病灶（修复前）：px_list_push / px_dict_set / gc_register 在**持 g_gc_mu** 期间做可失败
#   分配（xrealloc / xstrdup）。M127 门已坐实：失败点落在临界区内时，回卷会留锁 ⇒ 假死，
#   故只能退回 _exit(1)（服务中断 ~3s）⇒ 这三处上「OOM ⇒ 请求级 5xx」**尚未**成立。
# 修法（runtime/runtime.c，两阶段）：锁外备货（可失败，无锁在身 ⇒ 请求级 5xx）→ 锁内只做
#   指针发布（无分配、不可失败）；备货不足则解锁重来，重试用尽退化为锁内扩容（M127 行为）。
# 判据：
#   C1..C5 五个站点注入：**500 + 进程存活 + 无锁审计行 + 报「协程隔离点内」+ [px-coro]**
#          （无锁 ⇒ 走常规隔离 ⇒ 请求级 5xx；对照 M127 门的锁内失败 ⇒ _exit(1)）；
#   C6/C7 **不变量**：子协程扩容中被隔离终止，父协程核对「对象完好 + 仍可继续追加」
#          （旧实现留下 cap 与数组长度不一致 ⇒ 下一次追加越界写；新实现备货未发布 ⇒ 零影响）；
#   C8 A/B 对照（PX_GROW_RETRY_MAX=0 走旧锁内路径 + 同尺寸注入）⇒ 进程退出 + 审计列出 g_gc_mu；
#   C9 诚实记录：默认路径下仍有锁内分配站点（rate_limit 建桶）⇒ M127 审计仍是必需的；
#   C10/C11 负控：无注入全 200（默认路径 / 兜底路径两种都测）＋ 自然退出 rc=0；
#   C12 材料完整性。
# 注 1：必须用**仓库自带** tools/px（PXC_HOME=仓库根 ⇒ 用仓库 runtime/）。
# 注 2：站点注入**一次性**且环境变量**每次读** —— 故本门用 /arm 请求在启动后打开钩子
#       （避开启动期与 HTTP 层的无关分配抢先命中），每个用例一个独立进程。
# 用法：bash examples/m128_unlock_grow/verify.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m128_unlock_grow
PX=${PX:-$ROOT/tools/px}
BIN=$ROOT/$DIR/build/unlock_grow
PORT=18199
GTHRESH=100000000          # 拉高 GC 阈值：保证存活对象数能涨过对象表初始容量 8192
ARRAY_MIN=131072           # 从 4096 槽扩到 8192 槽时数组字节数（list/vals=131072；keys 需 8192 键）
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
    timeout 20 cat <&3 ) > "$2" 2>/dev/null
  tr -d '\r' < "$2" | head -1
}
body_of() { tr -d '\r' < "$1" | tail -1; }

PID=""; LOG=""
start_bin() {   # $1=tag $2=wait(1=等 READY) [env=val ...]
  local tag="$1" need_ready="$2"; shift 2
  pkill -f "$BIN" 2>/dev/null; sleep 0.4
  LOG=/tmp/m128_${tag}.log; : > "$LOG"
  env HOLD_MS=20000 PX_GC_THRESHOLD=$GTHRESH "$@" "$BIN" >>"$LOG" 2>&1 &
  PID=$!
  local i
  if [ "$need_ready" = 1 ]; then
    for i in $(seq 1 60); do grep -q "M128 READY" "$LOG" && break; sleep 0.25; done
    grep -q "M128 READY" "$LOG" || { bad "$tag 启动失败（无 READY）"; tail -3 "$LOG" | sed 's/^/     /'; PID=""; }
  fi
}
alive()  { kill -0 "$PID" 2>/dev/null; }
wait_gone() {   # $1=秒 → 0=已退出 1=仍在
  local n=$1 i
  for i in $(seq 1 $((n*4))); do kill -0 "$PID" 2>/dev/null || return 0; sleep 0.25; done
  return 1
}
kill9() { kill -9 "$PID" 2>/dev/null; wait "$PID" 2>/dev/null; }

# 站点注入用例的公共断言（$1=tag $2=站点名 $3=目标路径 $4=期望 500 的 body 关键字 $5=复跑路径 $6=复跑期望 body）
site_case() {
  local tag="$1" site="$2" path="$3" fup="$4" fbody="$5"
  local H F
  if [ -z "$PID" ]; then return; fi
  req_full "/arm?site=$site&min=$ARRAY_MIN" /tmp/m128_${tag}_arm.out > /dev/null
  H=$(req_full "$path" /tmp/m128_${tag}_tgt.out)
  echo "     $path → '${H:-（无响应）}'"
  case "$H" in *" 5"*) ok "$tag 扩容分配失败 ⇒ 请求级 5xx（M128 目标）";; *) bad "$tag 期望 5xx，实得 '$H'";; esac
  grep -q "px-m128] 注入命中：站点=$site" "$LOG" && ok "$tag 注入确在站点 $site 命中（诊断行在）" || bad "$tag 未见 $site 注入命中行"
  grep -q "临界区外" "$LOG" && ok "$tag 诊断行自述落点=临界区外" || bad "$tag 诊断行缺失"
  grep -q "协程隔离点内 → 只终止本请求" "$LOG" && ok "$tag 报「协程隔离点内 ⇒ 只终止本请求」（进程继续）" || bad "$tag 未走常规隔离文案"
  grep -q "\[px-coro\] 协程运行时错误已隔离" "$LOG" && ok "$tag 有 [px-coro] 隔离行（宿主继续）" || bad "$tag 无 [px-coro] 行"
  grep -q "无法安全回滚" "$LOG" && bad "$tag 竟触发锁审计（说明失败点仍在临界区内！）" || ok "$tag **未**触发锁审计 ⇒ 失败点确在临界区外"
  alive && ok "$tag 进程存活（未 exit(1)）" || bad "$tag 进程已退出"
  [ "$(probe)" = "HTTP/1.1 200 OK" ] && ok "$tag /health 仍 200" || bad "$tag /health 非 200"
  F=$(req_full "$fup" /tmp/m128_${tag}_fup.out)
  if [ "$(body_of /tmp/m128_${tag}_fup.out)" = "$fbody" ]; then
    ok "$tag 注入已消耗 ⇒ 复跑 $fup 正常（body=$fbody）"
  else
    bad "$tag 复跑 $fup body 异常：'$(body_of /tmp/m128_${tag}_fup.out)'（期望 $fbody）"
  fi
  kill9
}

echo "== M128 验证（扩容分配移出临界区：这些站点上的分配失败 ⇒ 请求级 5xx）=="

echo
echo "[0/12] 编译 + 材料自检"
if "$PX" build "$DIR/unlock_grow.px" > /tmp/m128_build.log 2>&1 && [ -x "$BIN" ]; then
  ok "编译成功（$(stat -c%s "$BIN") 字节）"
else
  bad "编译失败"; tail -20 /tmp/m128_build.log | sed 's/^/     /'; exit 1
fi
grep -qa "px-m128" "$BIN" && ok "二进制含 M128 站点注入诊断文案（确认用的是仓库 patched runtime）" \
  || { bad "二进制不含 M128 文案 —— runtime 未重编，本门无效"; exit 1; }
grep -q "PX_GROW_RETRY_MAX" runtime/runtime.c && ok "runtime.c 含兜底开关 PX_GROW_RETRY_MAX" || bad "缺 PX_GROW_RETRY_MAX"

echo
echo "[1/12] C1 list 扩容（站点 list_grow）"
start_bin C1 1
site_case C1 list_grow "/list?n=20000" "/list?n=100" "list=100"

echo
echo "[2/12] C2 dict keys 数组扩容（站点 dict_keys_grow）"
start_bin C2 1
site_case C2 dict_keys_grow "/dict?n=20000" "/dict?n=100" "dict=100"

echo
echo "[3/12] C3 dict vals 数组扩容（站点 dict_vals_grow）"
start_bin C3 1
site_case C3 dict_vals_grow "/dict?n=20000" "/dict?n=100" "dict=100"

echo
echo "[4/12] C4 键副本（站点 dict_key_dup；长键让键副本成为唯一大分配）"
if start_bin C4 1; then :; fi
if [ -n "$PID" ]; then
  req_full "/arm?site=dict_key_dup&min=200" /tmp/m128_C4_arm.out > /dev/null
  H=$(req_full "/dictlong?n=10" /tmp/m128_C4_tgt.out); echo "     /dictlong?n=10 → '${H:-（无响应）}'"
  case "$H" in *" 5"*) ok "C4 键副本分配失败 ⇒ 请求级 5xx";; *) bad "C4 期望 5xx，实得 '$H'";; esac
  grep -q "px-m128] 注入命中：站点=dict_key_dup" "$LOG" && ok "C4 注入命中 dict_key_dup（键副本路径）" || bad "C4 未见 dict_key_dup 命中行"
  grep -q "无法安全回滚" "$LOG" && bad "C4 竟触发锁审计" || ok "C4 未触发锁审计 ⇒ 键副本分配已在锁外"
  alive && ok "C4 进程存活" || bad "C4 进程已退出"
  [ "$(probe)" = "HTTP/1.1 200 OK" ] && ok "C4 /health 仍 200" || bad "C4 /health 非 200"
  req_full "/dict?n=10" /tmp/m128_C4_fup.out > /dev/null
  [ "$(body_of /tmp/m128_C4_fup.out)" = "dict=10" ] && ok "C4 复跑正常（dict=10）" || bad "C4 复跑异常"
  kill9
fi

echo
echo "[5/12] C5 对象表扩容（站点 gc_objs_grow）"
if start_bin C5 1; then :; fi
if [ -n "$PID" ]; then
  req_full "/arm?site=gc_objs_grow&min=$ARRAY_MIN" /tmp/m128_C5_arm.out > /dev/null
  H=$(req_full "/grown?n=9000" /tmp/m128_C5_tgt.out); echo "     /grown?n=9000 → '${H:-（无响应）}'"
  case "$H" in *" 5"*) ok "C5 对象表扩容失败 ⇒ 请求级 5xx";; *) bad "C5 期望 5xx，实得 '$H'";; esac
  grep -q "px-m128] 注入命中：站点=gc_objs_grow" "$LOG" && ok "C5 注入命中 gc_objs_grow" || bad "C5 未见命中行"
  grep -q "无法安全回滚" "$LOG" && bad "C5 竟触发锁审计" || ok "C5 未触发锁审计 ⇒ 对象表扩容已在锁外"
  alive && ok "C5 进程存活" || bad "C5 进程已退出"
  req_full "/grown?n=100" /tmp/m128_C5_fup.out > /dev/null
  [ "$(body_of /tmp/m128_C5_fup.out)" = "objs=100" ] && ok "C5 复跑正常（objs=100）⇒ 对象表在失败后仍可正确扩容" || bad "C5 复跑异常"
  # ★ 共享结构一致性：对象表是全进程共享的；失败后继续大量建对象必须全部成功（旧实现会因
  #   cap 与数组长度不一致而越界写）
  n_all=1
  for i in $(seq 1 100); do
    [ "$(probe)" = "HTTP/1.1 200 OK" ] || { n_all=0; break; }
  done
  [ "$n_all" = 1 ] && ok "C5 失败后连打 100 次请求全 200（对象表一致、服务未退化）" || bad "C5 失败后有请求失败"
  alive && ok "C5 全程进程存活" || bad "C5 最终进程已退出"
  kill9
fi

echo
echo "[6/12] C6 不变量 · list（子协程扩容中被隔离终止，父协程核对对象完好且仍可用）"
if start_bin C6 1; then :; fi
if [ -n "$PID" ]; then
  req_full "/arm?site=list_grow&min=$ARRAY_MIN" /tmp/m128_C6_arm.out > /dev/null
  H=$(req_full "/inv?kind=list" /tmp/m128_C6_tgt.out); echo "     /inv?kind=list → '$H'"
  B=$(body_of /tmp/m128_C6_tgt.out); echo "     body=$B"
  case "$H" in *" 200"*) ok "C6 父协程未受影响（200）";; *) bad "C6 期望 200，实得 '$H'";; esac
  case "$B" in *"before=4096"*) ok "C6 失败点 = 数组满（len=4096），对象 len 未被改动";; *) bad "C6 body 缺 before=4096";; esac
  case "$B" in *"first=0 last=4095"*) ok "C6 前 4096 项内容完好（未损坏）";; *) bad "C6 body 内容校验失败";; esac
  case "$B" in *"tail=999999 len=4097"*) ok "C6 失败后仍可继续追加（len=4097）⇒ cap/数组长度一致";; *) bad "C6 追加校验失败";; esac
  grep -q "\[px-coro\] 协程运行时错误已隔离" "$LOG" && ok "C6 失败确实发生在子协程（被隔离终止）" || bad "C6 无子协程隔离行"
  grep -q "无法安全回滚" "$LOG" && bad "C6 触发锁审计（异常）" || ok "C6 未触发锁审计"
  kill9
fi

echo
echo "[7/12] C7 不变量 · dict（同上；另核对已有键的值完好）"
if start_bin C7 1; then :; fi
if [ -n "$PID" ]; then
  req_full "/arm?site=dict_vals_grow&min=$ARRAY_MIN" /tmp/m128_C7_arm.out > /dev/null
  H=$(req_full "/inv?kind=dict" /tmp/m128_C7_tgt.out); echo "     /inv?kind=dict → '$H'"
  B=$(body_of /tmp/m128_C7_tgt.out); echo "     body=$B"
  case "$H" in *" 200"*) ok "C7 父协程未受影响（200）";; *) bad "C7 期望 200，实得 '$H'";; esac
  case "$B" in *"before=4096"*) ok "C7 失败点 = 数组满（len=4096），dict len 未被改动";; *) bad "C7 body 缺 before=4096";; esac
  case "$B" in *"v0=v0 vlast=v4095"*) ok "C7 首位/末位键值完好（未损坏）";; *) bad "C7 键值校验失败";; esac
  case "$B" in *"tail=ok len=4097"*) ok "C7 失败后仍可插入新键（len=4097）⇒ cap/数组长度一致";; *) bad "C7 插入校验失败";; esac
  kill9
fi

echo
echo "[8/12] C8 A/B 对照：旧路径（PX_GROW_RETRY_MAX=0，锁内扩容）+ 同尺寸注入 ⇒ 仍致命"
if start_bin C8 1 PX_GROW_RETRY_MAX=0 PX_ALLOC_FAIL_MIN=2097152; then :; fi
if [ -n "$PID" ]; then
  [ "$(probe)" = "HTTP/1.1 200 OK" ] && ok "C8 启动正常（旧路径无注入时服务可用）" || bad "C8 启动后 /health 非 200"
  H=$(req_full "/list?n=70000" /tmp/m128_C8_tgt.out); echo "     /list?n=70000 → '${H:-（无响应）}'"
  if wait_gone 15; then ok "C8 同尺寸注入在旧路径上 ⇒ 进程退出（与 C1 的 5xx 形成对照）"; else bad "C8 进程未退出（异常）"; kill9; fi
  grep -q "无法安全回滚" "$LOG" && ok "C8 触发锁审计（证明失败点在临界区内）" || bad "C8 无审计行"
  grep -q "g_gc_mu" "$LOG" && ok "C8 持锁清单列出 &g_gc_mu（可定位）" || bad "C8 清单未列 g_gc_mu"
  grep -q "_exit(1)" "$LOG" && ok "C8 文案说明保留致命语义 _exit(1)" || bad "C8 文案缺处置"
fi

echo
echo "[9/12] C9 诚实记录：默认路径下仍有锁内分配站点（rate_limit 建桶）⇒ M127 审计仍是必需的"
if start_bin C9 1 PX_ALLOC_FAIL_IN_LOCK=g_rate_mu; then :; fi
if [ -n "$PID" ]; then
  H=$(req_full "/rates" /tmp/m128_C9_tgt.out); echo "     /rates → '${H:-（无响应）}'"
  grep -q "px-locktrack] 注入命中：本线程持锁栈深度=1，匹配=&g_rate_mu" "$LOG" \
    && ok "C9 rate_limit 建桶确在 g_rate_mu 临界区内分配（M129 候选：px_rate_limit_try）" \
    || bad "C9 未见 g_rate_mu 锁内注入命中行"
  if wait_gone 10; then ok "C9 该站点失败 ⇒ 锁审计介入并退出（未假死）"; else bad "C9 进程未退出"; kill9; fi
  grep -q "g_rate_mu" "$LOG" && ok "C9 审计清单列出 &g_rate_mu" || bad "C9 审计清单缺 g_rate_mu"
fi

echo
echo "[10/12] C10 负控 · 无注入（默认路径全功能 + 自然退出）"
if start_bin C10 1; then :; fi
if [ -n "$PID" ]; then
  for P in "/health:alive" "/list?n=20000:list=20000" "/dict?n=20000:dict=20000" "/grown?n=9000:objs=9000" "/dictlong?n=10:dictlong=10"; do
    PTH=${P%%:*}; EXP=${P##*:}
    H=$(req_full "$PTH" /tmp/m128_C10.out); B=$(body_of /tmp/m128_C10.out)
    if [ "$B" = "$EXP" ]; then ok "C10 $PTH → 200/$EXP"; else bad "C10 $PTH body='$B'（期望 $EXP；状态 '$H'）"; fi
  done
  H=$(req_full "/inv?kind=list" /tmp/m128_C10b.out); B=$(body_of /tmp/m128_C10b.out)
  # 无注入时子协程不会被终止 ⇒ 轮询观察到的 len 是**任意**中间值（非 4096），故此处只校验
  #   「首项/末项与末尾追加」的一致性，不校验具体 len。
  case "$B" in *"first=0 last="*"tail=999999"*) ok "C10 /inv?kind=list 正常（首末项与追加尾部一致）";; *) bad "C10 /inv list body='$B'";; esac
  H=$(req_full "/inv?kind=dict" /tmp/m128_C10c.out); B=$(body_of /tmp/m128_C10c.out)
  case "$B" in *"v0=v0"*"tail=ok"*) ok "C10 /inv?kind=dict 正常（首键值与追加键一致）";; *) bad "C10 /inv dict body='$B'";; esac
  grep -q "px-m128]" "$LOG" && bad "C10 无注入却出现 M128 注入行" || ok "C10 无注入时零注入行"
  grep -q "无法安全回滚\|运行时错误" "$LOG" && bad "C10 出现错误/审计行" || ok "C10 无错误行、无审计行"
  if wait_gone 40; then
    wait "$PID" 2>/dev/null; RC=$?
    grep -q "M128 MAIN-EXIT-NORMAL" "$LOG" && [ "$RC" = 0 ] && ok "C10 自然退出、退出码 0" || bad "C10 退出异常 rc=$RC"
  else bad "C10 未自然退出"; kill9; fi
fi

echo
echo "[11/12] C11 负控 · 兜底路径（PX_GROW_RETRY_MAX=0）功能等价、无注入"
if start_bin C11 1 PX_GROW_RETRY_MAX=0; then :; fi
if [ -n "$PID" ]; then
  for P in "/list?n=20000:list=20000" "/dict?n=20000:dict=20000" "/grown?n=9000:objs=9000"; do
    PTH=${P%%:*}; EXP=${P##*:}
    req_full "$PTH" /tmp/m128_C11.out > /dev/null; B=$(body_of /tmp/m128_C11.out)
    if [ "$B" = "$EXP" ]; then ok "C11 兜底路径 $PTH → $EXP"; else bad "C11 $PTH body='$B'（期望 $EXP）"; fi
  done
  grep -q "无法安全回滚\|px-m128]" "$LOG" && bad "C11 出现审计/注入行" || ok "C11 兜底路径无审计、无注入"
  kill9
fi

echo
echo "[12/12] C12 材料完整性"
grep -q "m128_alloc" runtime/runtime.c && ok "runtime.c 含 m128_alloc（阶段 A 分配出口）" || bad "缺 m128_alloc"
for S in list_grow dict_keys_grow dict_vals_grow dict_key_dup gc_objs_grow; do
  grep -q "\"$S\"" runtime/runtime.c && ok "runtime.c 含站点标签 $S" || bad "缺站点标签 $S"
done
grep -q "px_list_push_locked\|px_dict_set_locked" runtime/runtime.c && ok "保留锁内兜底路径（M127 行为）" || bad "缺锁内兜底路径"
[ -f "$DIR/README.md" ] && ok "验收材料 README.md 在" || bad "缺 README.md"
[ -f "$DIR/unlock_grow.px" ] && ok "验收器 unlock_grow.px 在" || bad "缺 unlock_grow.px"
grep -q "PX_GROW_RETRY_MAX" "$DIR/README.md" && ok "README 记录兜底开关与边界" || bad "README 未记录兜底开关"
grep -q "PX_ALLOC_FAIL_SITE" "$DIR/README.md" && ok "README 记录站点注入钩子" || bad "README 未记录站点注入"

echo
echo "== M128 门结束：pass=$pass fail=$fail =="
[ "$fail" = 0 ] || exit 1
