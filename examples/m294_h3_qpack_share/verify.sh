#!/usr/bin/env bash
# ============================================================
# examples/m294_h3_qpack_share/verify.sh
# ------------------------------------------------------------
# M294（第 171 轮）：**并发连接共享 QPACK 会话**（缺陷 507）的常设化回归门
#
# 来历：
#   M293 只在 `docs/HTTP3_STANCE.md` §二 把「**505 并发连接互相干扰**」登记为**现象**
#   （服务端把某连接的请求解成空路径 → 404、或**另一条连接的路径**、或该连接完全无响应；
#     顺序 12/12 全对、并发 N=2 即复现），并如实写明「本轮不修」。
#   本轮把它**钉到根因并修好**：
#     `bi_qs_open` 的会话占用是**无锁的「读 — 清 — 置位」三步**，其中 `memset(s,0,sizeof(*s))`
#     会把**刚占上的占用位又清回 0** ⇒ 并发连接的第二个线程在窗口里**二次认领同一个会话槽**
#     ⇒ 多条**同时存活**的活跃连接**共享同一张 QPACK 动态表** ⇒ 各自解出**别人的路径**。
#     实证（诊断时间线）：conn 13/14/15/16/17/18 **六条同时拿到 qd=13** ⇒ 六条都解出
#     `/probe/r1p2`。修法 = **占用位与数据分离**（`g_qds_used`）+ **原子交换作唯一仲裁者**。
#
# 本门的判据**只用产品自带输出**：
#   · 服务端 `[px-access]` 日志（运行时自己的访问日志）—— 重复服务路径数 / 空路径 404 / 200 数
#   · 客户端契约行（`TAG <t> OK` / `CROSSTALK` / `TIMEOUT` / `bad-status`）
#   ⚠️ **刻意不用**当时的插桩计数器（`shared` / `recycle_shared` / `open_dup`）—— 那些不在产品里，
#      门一旦依赖它们就会退化成「依赖插桩的门」，而插桩终究会被清理（M289 的教训）。
#
# ── 规模为什么是「前戏 4 + 3 轮 × 16 并发」（不是随手取的）────────────
#   ① **冷 listener 上异常率为 0**（假阴性）：必须先制造若干次建连/关闭，「已用槽被 memset
#      清回 0」那条路才走得到 ⇒ 固定前戏 4 次（实测 4 次已足够，见 [6b] 的槽位预算）。
#   ② **缺陷 508（QUIC 连接槽位耗尽，`QUIC_MAX=64`）会掩盖 507**：一个 listener 在约 10s
#      窗口内只承接 ~64 条连接，超了以后 `quic_alloc_conn` 返回 −1 且**静默**（客户端只见
#      `connect-fail`）。⇒ 本门把每个 listener 实例的连接数**压在预算内**（[3]/[4] 一个实例、
#      [5]/[6] 重启后的另一个实例），并在 **[6b] 把这个预算量出来登记**（缺陷 508 的证据）。
#
# 判据层次：
#   [1] 静态（结构 + **逆向** + 顺序 + 规模锚点）· [2] 编译 · [3] 起服务（强就绪 + 功能探针）
#   [4] **顺序对照**（变量隔离：不并发时本来就对）· [5] **并发主力**（前戏 4 + 3 轮 × 16 并发）
#   [6] 并发后串行不受污染 + 存活/fd · [6b] 槽位预算（缺陷 508 登记）· [7] 负控 A/B/C/D · [8] 覆盖边界
#
# ⚠️ 本门**会就地改仓库源码**（负控 A/C 撤回 `runtime/runtime_h3_qpack_dyn.c`）⇒
#   · 开头 source `selfhost/gate_lock.sh`（M276：与全量门/其它门互斥）
#   · `trap` 里无条件 `restore_all`（中途被杀也还原）
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="$(mktemp -d /tmp/m294_gate.XXXXXX)"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

TGT="$ROOT/runtime/runtime_h3_qpack_dyn.c"
WARM=4          # 前戏次数（[6b] 量出的槽位预算里，它占 4 —— 见文件头「规模为什么」）

# ---------- 端口卫生（M274 缺陷 480：固定端口会因上一次残留而偶发判红）----------
EPHEM_LO="$(awk '{print $1}' /proc/sys/net/ipv4/ip_local_port_range 2>/dev/null || echo 32768)"
case "$EPHEM_LO" in ''|*[!0-9]*) EPHEM_LO=32768 ;; esac
PORT_HI=$((EPHEM_LO - 3))
[ "$PORT_HI" -gt 32767 ] && PORT_HI=32767
[ "$PORT_HI" -lt 21000 ] && PORT_HI=21000
PORT=$((20000 + RANDOM % (PORT_HI - 20000)))
PORT=$((PORT / 2 * 2))            # 偶数基 ⇒ h3 = base+1 也在区间内

free_port() {   # 只清**占用该端口**的进程（按进程名清会打到别的门的服务 —— M293 教训）
  local p="$1" pid=""
  pid="$( (ss -ltnup 2>/dev/null || netstat -ltnup 2>/dev/null) | grep -E ":${p} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$pid" ] && { kill -9 "$pid" 2>/dev/null; sleep 0.3; }
  return 0
}

# ---------- 源码快照/还原（负控用）----------
BAKFILES=(); BAKBLOB=()
snapshot() { local b="$W/$(echo "$1" | tr '/' '_').bak"; cp "$1" "$b"; BAKFILES+=("$1"); BAKBLOB+=("$b"); }
restore_all() {   # $1=quiet ⇒ 不计数（trap 里用）
  local i=0 f b rc=0
  while [ "$i" -lt "${#BAKFILES[@]}" ]; do
    f="${BAKFILES[$i]}"; b="${BAKBLOB[$i]}"
    cp "$b" "$f"
    cmp -s "$f" "$b" || rc=1
    i=$((i + 1))
  done
  BAKFILES=(); BAKBLOB=()
  if [ "$rc" != "0" ] && [ "${1:-}" != "quiet" ]; then bad "源码还原失败（$TGT）"; fi
  return "$rc"
}

SRV_PID=""
cleanup_all() {
  [ -n "$SRV_PID" ] && { kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""; }
  free_port "$PORT"; free_port "$((PORT + 1))"
  restore_all quiet >/dev/null 2>&1
  rm -rf "$W"
}
trap 'cleanup_all' EXIT

# ---------- 编译（产物落派生目录：tools/px build F → dirname(F)/build/基名）----------
SRCD="$W/src"; BD="$SRCD/build"
build_cli() {
  mkdir -p "$SRCD"
  cp -f "$HERE/cli_tag.px" "$SRCD/cli_tag.px" || return 1
  ( cd "$ROOT" && ./tools/px build "$SRCD/cli_tag.px" ) >"$W/build_cli.log" 2>&1 || {
    echo "  ❌ BUILDFAIL cli_tag"; tail -5 "$W/build_cli.log" | sed 's/^/     /'; return 1; }
  return 0
}
build_srv() {
  mkdir -p "$SRCD"
  cp -f "$HERE/srv.px" "$SRCD/srv.px" || return 1
  ( cd "$ROOT" && ./tools/px build "$SRCD/srv.px" ) >"$W/build_srv.log" 2>&1 || {
    echo "  ❌ BUILDFAIL srv"; tail -5 "$W/build_srv.log" | sed 's/^/     /'; return 1; }
  return 0
}

# ---------- 起停服务（强就绪判据：运行时**绑定成功之后**打印的行）----------
SRV_LOG=""
srv_start() {   # $1 = 日志标签 → 0 = 就绪
  local tag="$1" rp="" i=0
  SRV_LOG="$W/srv_$tag.log"
  free_port "$PORT"; free_port "$((PORT + 1))"
  rm -f "$SRV_LOG"
  setsid nohup env M294_HTTP_PORT="$PORT" "$BD/srv" >"$SRV_LOG" 2>&1 </dev/null &
  disown 2>/dev/null || true     # ⚠️ 必须：否则后面裸 wait 会等这个后台作业（门永久挂住）
  while [ "$i" -lt 100 ]; do
    sleep 0.1
    grep -q "普贤应用服务器" "$SRV_LOG" 2>/dev/null && break
    i=$((i + 1))
  done
  rp="$( (ss -ltnp 2>/dev/null) | grep -E ":${PORT} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$rp" ] && SRV_PID="$rp"
  [ -n "$SRV_PID" ] && kill -0 "$SRV_PID" 2>/dev/null && return 0
  return 1
}
srv_stop() {
  [ -n "$SRV_PID" ] && { kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""; }
  free_port "$PORT"; free_port "$((PORT + 1))"
  return 0
}
run_cli() { M294_HTTP_PORT="$PORT" M294_TAG="$1" timeout -k 5 30 "$BD/cli_tag" >"$W/one.out" 2>&1; }
probe_service() { run_cli ready || true; [ "$(tail -1 "$W/one.out" 2>/dev/null)" = "TAG ready OK" ]; }

# ---------- 服务端访问日志解析（**产品自带输出**）----------
#   [px-access] <ts> <remote> GET <path> <status> <bytes> <ms> req=<id>
#   空路径形态：`GET  404 …` ⇒ $5 不是 `/` 开头 ⇒ 计入 e404
parse_acc() {   # $1=日志 $2=路径前缀 → "dup<TAB>uniq<TAB>e404<TAB>n200"
  awk -v pre="$2" '
    /^\[px-access\]/ {
      m=$4; p=$5
      if (m != "GET") next
      if (p ~ /^\//) { if (index(p, pre) == 1) { cnt[p]++; if ($6 == 200) n200++ } }
      else { e404++ }
    }
    END {
      d=0; for (k in cnt) if (cnt[k] > 1) d++
      u=0; for (k in cnt) u++
      printf "%d\t%d\t%d\t%d\n", d+0, u+0, e404+0, n200+0
    }' "$1"
}

# ---------- 测量 ----------
M_TOT=0; M_OK=0; M_XT=0; M_TO=0; M_OT=0; M_DUP=0; M_UNIQ=0; M_E404=0; M_200=0
tally() {   # $1=tag $2=输出文件
  local L
  L="$(grep -E '^TAG ' "$2" 2>/dev/null | tail -1)"
  M_TOT=$((M_TOT + 1))
  case "$L" in
    "TAG $1 OK")  M_OK=$((M_OK + 1)) ;;
    *CROSSTALK*)  M_XT=$((M_XT + 1)) ;;
    *TIMEOUT*)    M_TO=$((M_TO + 1)) ;;
    *)            M_OT=$((M_OT + 1)) ;;
  esac
}
measure_serial() {   # $1=RUN 标签 $2=条数（**不同 RUN 的路径前缀互不包含**）
  local run="$1" n="$2" i
  M_TOT=0; M_OK=0; M_XT=0; M_TO=0; M_OT=0
  for i in $(seq "$n"); do
    M294_HTTP_PORT="$PORT" M294_TAG="${run}s-s${i}" timeout -k 5 20 "$BD/cli_tag" >"$W/${run}_s$i.out" 2>&1 || true
    tally "${run}s-s${i}" "$W/${run}_s$i.out"
  done
  read -r M_DUP M_UNIQ M_E404 M_200 <<<"$(parse_acc "$SRV_LOG" "/q294/${run}s-s")"
}
measure_conc() {   # $1=RUN 标签 $2=轮数 $3=并发
  local run="$1" rounds="$2" n="$3" r t w pids
  # ⭐ 前戏：冷 listener 上异常率为 0（**假阴性**），必须先制造若干次建连/关闭
  for w in $(seq "$WARM"); do
    M294_HTTP_PORT="$PORT" M294_TAG="${run}w-w${w}" timeout -k 5 20 "$BD/cli_tag" >"$W/${run}_w$w.out" 2>&1 || true
  done
  M_TOT=0; M_OK=0; M_XT=0; M_TO=0; M_OT=0
  for r in $(seq "$rounds"); do
    pids=""
    for t in $(seq "$n"); do
      ( M294_HTTP_PORT="$PORT" M294_TAG="${run}-r${r}p${t}" timeout -k 5 30 "$BD/cli_tag" \
          >"$W/${run}_r${r}_t${t}.out" 2>&1 ) & pids="$pids $!"
    done
    for p in $pids; do wait "$p" 2>/dev/null; done
    for t in $(seq "$n"); do tally "${run}-r${r}p${t}" "$W/${run}_r${r}_t${t}.out"; done
  done
  read -r M_DUP M_UNIQ M_E404 M_200 <<<"$(parse_acc "$SRV_LOG" "/q294/${run}-r")"
}
show_read() { echo "       [$1] 请求 $M_TOT（唯一路径 $M_UNIQ）· OK $M_OK · 串味 $M_XT · 无响应 $M_TO · 其它 $M_OT · 重复服务路径 $M_DUP · 空路径404 $M_E404 · 200 应答 $M_200"; }

# ⭐ 动态判据（**负控 B 的唯一注入点**）：M294_GATE_LOOSE=1 ⇒ 跳过硬判据
#   判据取值的依据（本门建立时实测 · 每侧 8 次独立重复 · 每次 48 请求 = 384）：
#                    修复版        修复前
#     重复服务路径>0   0/8           **8/8**   ← 主判据（100% 分离，NC-A 靠它）
#     串味>0           0/8           6/8
#     无响应>0         0/8           7/8
#     空路径404>0      0/8           3/8
#     OK/tot           ≥47/48        ≤45/48（88.8% 合计）
#   ⚠️ 「OK == tot」**不能**做硬判据：修复版 8 次里有 1 次出现 1 例 connect-fail
#      （那是**缺陷 508** 的槽位预算问题，与 507 无关）⇒ 用 95% **下限**替代。
judge_dyn() {   # $1=标签
  if [ "${M294_GATE_LOOSE:-0}" = "1" ]; then
    echo "  ℹ️ [$1] 宽松判据（M294_GATE_LOOSE=1）⇒ 跳过硬判据（用于证明红来自这几条契约）"
    return 0
  fi
  local floor=$(( (M_TOT * 95 + 99) / 100 ))
  chk "[$1] 服务端：访问日志 **重复服务路径数 == 0**（507 的直接指纹）" "$M_DUP" "0"
  chk "[$1] 客户端：串味 == 0" "$M_XT" "0"
  chk "[$1] 客户端：无响应 == 0" "$M_TO" "0"
  chk "[$1] 服务端：访问日志 **空路径 404 == 0**" "$M_E404" "0"
  if [ "$M_OK" -ge "$floor" ]; then
    ok "[$1] 客户端：拿到自己响应体的比例 $M_OK/$M_TOT（≥95% 下限 $floor）"
  else
    bad "[$1] 客户端：拿到自己响应体的比例仅 $M_OK/$M_TOT（<95% 下限 $floor）"
  fi
  [ "$M_OT" -gt 0 ] && echo "       ℹ️ [$1] 另有 $M_OT 例「其它」（connect-fail 等）—— 属缺陷 508 的槽位预算面，见 [6b]"
}

# ---------- [1] 静态层（单独成函数：负控 C 要**只**跑它）----------
CSTRIP=(python3 "$HERE/cstrip.py")
static_layer() {
  local s_raw s_code n
  s_raw="$(cat "$TGT")"
  s_code="$("${CSTRIP[@]}" "$TGT")"
  # S0 去注释器自证（判据自伤：注释里的标识符必须被去掉、代码里的必须保留）
  # ⚠️ 用「子进程 + stdin」而不是 importlib 加载 cstrip.py：后者会在门目录里生成
  #    `__pycache__/` ⇒ 污染仓库（全量门会判「工作树不干净」）。
  n=$(printf 'int x; /* int used; */\nint used;\n' | python3 "$HERE/cstrip.py" - | grep -c 'int used;')
  chk "S0 去注释器自证：注释里的 int used; 被去掉、代码里的保留 ⇒ 计数 1" "$(num "$n")" "1"

  n=$(num "$(printf '%s' "$s_code" | grep -c 'static char    g_qds_used\[QD_MAX_SESS\];')")
  chk "S1 占用位是**独立数组**（g_qds_used）" "$n" "1"
  n=$(num "$(printf '%s' "$s_code" | grep -c 'g_qds_used\[id - 1\]')")
  chk "S2 qd_get 读独立占用位" "$n" "1"
  n=$(num "$(printf '%s' "$s_code" | grep -c '__atomic_exchange_n(&g_qds_used\[i\], 1, __ATOMIC_SEQ_CST) == 0')")
  chk "S3 bi_qs_open 用原子交换认领" "$n" "1"
  # **逆向判据**：缺陷 507 的载体（结构体里的 used 字段）不得再出现
  n=$(num "$(printf '%s' "$s_code" | grep -cE '^[[:space:]]*int used;')")
  chk "S4（逆向）qd_sess 结构体内无 int used; 字段" "$n" "0"
  n=$(num "$(printf '%s' "$s_code" | grep -c 's->used')")
  chk "S5（逆向）全文件无 s->used" "$n" "0"
  # **顺序判据**：open 里必须「先原子占位、后清数据」
  local ln_x ln_m ln_rel ln_cm
  ln_x=$(num "$(printf '%s' "$s_code" | grep -n '__atomic_exchange_n(&g_qds_used\[i\]' | head -1 | cut -d: -f1)")
  # ⚠️ 必须在**去注释后**的文本里找 memset（「只清数据」那句话在注释里，会被 cstrip 去掉）
  #    取「原子占位那行**之后**的第一处 memset(s, 0, sizeof(*s))」= bi_qs_open 里的那一处
  ln_m=$(num "$(printf '%s' "$s_code" | awk -v s="$ln_x" 'NR > s && /memset\(s, 0, sizeof\(\*s\)\);/ {print NR; exit}')")
  if [ "$ln_x" -gt 0 ] && [ "$ln_m" -gt "$ln_x" ]; then
    ok "S6 顺序：open 里「先原子占位（行 $ln_x）、后清数据（行 $ln_m）」"
  else
    bad "S6 顺序不符（占位行 $ln_x · 清数据行 ${ln_m:-无}）—— 清数据若在占位前，占用位又会被清回 0"
  fi
  # **顺序判据**：close 里必须「先清数据、后释放占用位」
  ln_rel=$(num "$(printf '%s' "$s_code" | grep -n '__atomic_store_n(&g_qds_used\[args\[0\].as.i - 1\], 0' | head -1 | cut -d: -f1)")
  ln_cm=$(num "$(printf '%s' "$s_code" | grep -n '^    memset(s, 0, sizeof(\*s));' | head -1 | cut -d: -f1)")
  if [ "$ln_rel" -gt 0 ] && [ "$ln_cm" -gt 0 ] && [ "$ln_rel" -gt "$ln_cm" ]; then
    ok "S7 顺序：close 里「先清数据（行 $ln_cm）、后释放占用位（行 $ln_rel）」"
  else
    bad "S7 顺序不符（清数据行 ${ln_cm:-无} · 释放行 ${ln_rel:-无}）—— 先释放会让别人拿到未清空的会话"
  fi
  n=$(num "$(printf '%s' "$s_raw" | grep -c '别再往这里加')")
  chk "S8 结构体里的**警示注释**在位（「别再往这里加」）" "$n" "1"
  local nl
  nl=$(num "$(printf '%s' "$s_raw" | wc -l)")
  n=$(num "$(printf '%s' "$s_code" | grep -c 'qd_get(')")
  if [ "$nl" -ge 1000 ] && [ "$n" -ge 15 ]; then
    ok "S9 规模锚点：$nl 行（≥1000）· qd_get( 出现 $n 次（≥15）"
  else
    bad "S9 规模锚点不符（$nl 行 · qd_get( $n 次）"
  fi
}

# ============================================================
echo "===== M294 门：HTTP/3 并发连接 共享 QPACK 会话（缺陷 507 的常设化）====="
echo "端口对：http=$PORT h3=$((PORT + 1))（区间 20000..$PORT_HI，避开出站临时端口池）"

echo
echo "【1】静态判据（结构 + 逆向 + 顺序 + 规模锚点）"
static_layer

echo
echo "【2】编译门语料（srv + cli_tag）"
if build_cli && build_srv; then ok "2 件全部编译通过"; else bad "编译失败"; echo; echo "===== 失败 $FAIL 项 · 通过 $PASS 项 ====="; exit 1; fi

echo
echo "【3】起服务（强就绪判据）+ H3 监听在位 + 功能就绪探针"
if srv_start base; then ok "服务就绪（pid=$SRV_PID · 日志出现绑定成功行）"; else
  bad "服务未就绪"; sed 's/^/     /' "$SRV_LOG" 2>/dev/null | tail -5
  echo; echo "===== 失败 $FAIL 项 · 通过 $PASS 项 ====="; exit 1
fi
grep -q "HTTP/3 listening udp/$((PORT + 1))" "$SRV_LOG" 2>/dev/null && ok "H3 监听在位（udp/$((PORT + 1))）" || bad "H3 监听缺失"
probe_service && ok "功能就绪探针：一次真实 H3 请求得到自己的响应体（不只等日志）" || bad "功能就绪探针失败"

echo
echo "【4】顺序对照（变量隔离：**不并发**时本来就对）"
measure_serial S1 12
show_read S1
chk "[S1] 顺序 12 条全部拿到自己的响应体" "$M_OK" "12"
chk "[S1] 顺序 12 条 → 访问日志 12 条唯一路径（无重复服务）" "$M_UNIQ" "12"
chk "[S1] 顺序下 重复服务路径数 == 0" "$M_DUP" "0"

echo
echo "【5】并发主力（前戏 $WARM + 3 轮 × 16 并发 · 每连接**全局唯一**路径）"
echo "      ⚠️ 重启 listener：缺陷 508 的槽位预算（一个实例 ~64 条连接，见 [6b]）"
srv_stop
if srv_start main; then
  ok "listener 重启就绪（槽位预算重置）"
  probe_service && ok "重启后 功能就绪探针" || bad "重启后 功能就绪探针失败"
  measure_conc C1 3 16
  show_read C1
  judge_dyn C1
else
  bad "并发主力：服务未就绪"
fi

echo
echo "【6】并发后：串行不受污染 + 服务存活 + fd 未泄漏"
measure_serial S2 3
chk "[S2] 并发测量后 串行 3/3 不受污染（这才是「listener 被拖垮」的判据）" "$M_OK" "3"
if kill -0 "$SRV_PID" 2>/dev/null; then ok "并发测量后服务仍存活（同一 listener）"; else bad "并发测量后服务已死"; fi
BASE_FD="$(num "$(ls "/proc/$SRV_PID/fd" 2>/dev/null | wc -l)")"
[ "$BASE_FD" -ge 6 ] && ok "[S2] listener fd 数 $BASE_FD（≥6 · 未塌缩）" || bad "[S2] listener fd 数异常（$BASE_FD）"

echo
echo "【6b】槽位预算（**缺陷 508 已修**的登记 —— 也是本门规模取值的依据）"
echo "      为什么量它：508 曾**掩盖** 507（槽位满了以后客户端只见 connect-fail，看不到串味/重复路径）"
echo "      M295 前：上限 64 ⇒ 90 次串行必在第 65 次左右触顶（本层曾据此判红）· M295 后：256 ⇒ 应全部成功"
srv_stop
if srv_start slot; then
  NSUC=0; FF=""; T0=$(date +%s)
  for n in $(seq 1 90); do
    M294_HTTP_PORT="$PORT" M294_TAG="slot$n" timeout -k 5 8 "$BD/cli_tag" >"$W/slot.out" 2>&1 || true
    if [ "$(tail -1 "$W/slot.out")" = "TAG slot$n OK" ]; then NSUC=$((NSUC + 1)); else FF="$n"; break; fi
  done
  EL=$(( $(date +%s) - T0 ))
  echo "       串行建连/关闭：连续成功 $NSUC 次 · 首次失败 $FF（耗时 ${EL}s）"
  # M295s1：缺陷 508 已修 ⇒ 上限从 64 提到 256（PX_QUIC_CONN_MAX）。
  #   本层的判据随之**反向**：90 次串行**不得**触及上限（触顶 = 上限被降回 / 本层前提变了）。
  #   上限的**精确值**由 examples/m295_quic_slot 的 [6] 段看守（串行 264 ⇒ 首次失败 @256）；
  #   本门只保证自己的规模远小于上限、不污染读数。
  if [ -z "$FF" ]; then
    ok "90 次串行**未触及上限**（与 PX_QUIC_CONN_MAX=256 一致 ⇒ 缺陷 508 已修）"
    echo "       ℹ️ 上限的精确值由 examples/m295_quic_slot 的 [6] 段看守；本门只保证规模不越界"
  else
    bad "第 $FF 次触顶（连续成功 $NSUC）⇒ 上限已不再 ≥ 90 ⇒ 请更新 [6b] 与 docs/HTTP3_STANCE.md"
  fi
else
  bad "[6b] 服务未就绪"
fi
srv_stop

# ============================================================
if [ "$NEG_SKIP" = "1" ]; then
  echo
  echo "【7】负控：--neg-skip 跳过"
else
  echo
  echo "【7】负控 A/B：**忠实撤回**缺陷 507 的修复 ⇒ 动态判据必须判红（各自独立）"
  SHA_FIXED="$(sha256sum "$BD/srv" | cut -d' ' -f1)"

  echo "  ── NC-D：打桩器自证（锚点唯一性 + 撤回忠实性）"
  if python3 "$HERE/negctl.py" --selftest "$TGT" >"$W/ncD.out" 2>&1; then
    ok "NC-D negctl.py --selftest：$(num "$(grep -c '✅' "$W/ncD.out")") 条自证全绿（含 2 道判据自伤）"
  else
    bad "NC-D selftest 失败"; sed 's/^/     /' "$W/ncD.out" | tail -12
  fi

  echo "  ── NC-A：撤回修复 → **重建** → 动态判据必须判红"
  echo "      （⚠️ 下面出现的 ❌ 是**预期红** —— 它们证明判据有牙；已从失败计数里扣除）"
  restore_all            # ⚠️ 每道负控**各自独立、干净起点**（M284 的教训：restore 会清空备份表）
  snapshot "$TGT"
  if python3 "$HERE/negctl.py" --revert "$TGT" >"$W/ncA_patch.log" 2>&1; then
    ok "NC-A 补丁应用成功（5 处锚点各命中 1 次）"
  else
    bad "NC-A 补丁失败（锚点不唯一/过期）"; cat "$W/ncA_patch.log"
  fi
  if build_srv; then ok "NC-A 撤回档重建成功"; else bad "NC-A 撤回档重建失败"; fi
  SHA_PREFIX="$(sha256sum "$BD/srv" | cut -d' ' -f1)"
  # ⭐ 前提自证（M289 的教训）：**先证明这次改动不是空操作**
  if [ "$SHA_PREFIX" != "$SHA_FIXED" ]; then
    ok "NC-A 前提自证：撤回档产物与修复版**不同**（${SHA_FIXED:0:12}… ≠ ${SHA_PREFIX:0:12}…）⇒ 撤回有实际效果"
  else
    bad "NC-A 前提自证失败：撤回档产物与修复版**逐字节相同** ⇒ 撤回是空操作（负控无意义）"
  fi
  NBEFORE=$FAIL
  if srv_start ncA; then
    measure_conc A1 3 16
    show_read A1
    judge_dyn A1
  else
    bad "NC-A 服务未就绪"
  fi
  # ⚠️ **预期的红要从失败计数里扣掉**（否则负控本身会让门判红 —— 这正是 M278「会偶发判红
  #    的门等于没有门」的镜像：负控的红必须被「预期化」，只剩**结论**进计数）。
  NCN=$((FAIL - NBEFORE)); FAIL=$NBEFORE
  if [ "$NCN" -ge 1 ]; then
    ok "NC-A 撤回修复后**动态判据判红**（$NCN 项预期红，已从失败计数扣除）⇒ 判据有牙"
  else
    bad "NC-A 撤回修复后判据**未判红** ⇒ 判据无牙（或撤回无效）"
  fi

  echo "  ── NC-B：判据自伤（同一个撤回档二进制 + 宽松开关）⇒ 必须**不再**判红"
  NBEFORE2=$FAIL
  srv_stop
  # ⚠️ 换**新的 listener 实例**（槽位预算干净）：否则 A1 已经把 ~64 条预算用掉，
  #    宽松档的读数会被**缺陷 508** 污染（实测第一次跑成 OK 7/48 ⇒ 看着像"判据坏了"）。
  if srv_start ncB; then
    M294_GATE_LOOSE=1 measure_conc A2 3 16
    M294_GATE_LOOSE=1 judge_dyn A2
    NCN2=$((FAIL - NBEFORE2)); FAIL=$NBEFORE2
    if [ "$NCN2" -eq 0 ]; then
      ok "NC-B 宽松判据下**不再判红**（读数：OK $M_OK/$M_TOT · 串味 $M_XT · 重复路径 $M_DUP）⇒ NC-A 的红来自那 5 条契约"
    else
      bad "NC-B 宽松判据下仍判红（新增 $NCN2 项）⇒ NC-A 的红来源不明"
    fi
  else
    bad "NC-B 前置失败（服务未就绪）"
  fi
  srv_stop

  echo "  ── NC-C：**只改结构、不重建** ⇒ [1] 静态层必须独立判红（证明静态/动态各有各的牙）"
  restore_all
  snapshot "$TGT"
  if python3 "$HERE/negctl.py" --revert-struct-only "$TGT" >"$W/ncC_patch.log" 2>&1; then
    ok "NC-C 补丁应用成功（只撤回结构体那一处）"
  else
    bad "NC-C 补丁失败"; cat "$W/ncC_patch.log"
  fi
  NBEFORE3=$FAIL
  echo "      （⚠️ 下面静态层的 ❌ 同样是**预期红**）"
  static_layer
  NCN3=$((FAIL - NBEFORE3)); FAIL=$NBEFORE3
  if [ "$NCN3" -ge 1 ]; then
    ok "NC-C 只改结构 ⇒ 静态层判红（$NCN3 项预期红，已从失败计数扣除）⇒ 静态判据有独立的牙"
  else
    bad "NC-C 只改结构 ⇒ 静态层**未**判红 ⇒ 静态判据形同虚设"
  fi

  echo "  ── 还原 + 保真自证（还原后重建产物必须与修复版**逐字节相同**）"
  restore_all
  if build_srv; then
    SHA_BACK="$(sha256sum "$BD/srv" | cut -d' ' -f1)"
    if [ "$SHA_BACK" = "$SHA_FIXED" ]; then
      ok "还原后重建产物与修复版**逐字节相同**（${SHA_BACK:0:12}…）⇒ 还原保真、无残留"
    else
      bad "还原后产物与修复版不同（${SHA_BACK:0:12}… ≠ ${SHA_FIXED:0:12}…）⇒ 源码未还原干净"
    fi
  else
    bad "还原后重建失败"
  fi
fi

echo
echo "覆盖边界（如实登记）："
echo "  · 只覆盖 **托管 listener** 路径（px_serve + opts.http3）—— 即晨曦生产在用的形态；"
echo "    裸 quic_listen + quic_accept 的 demo 路径不在面内（两路径的会话分配路径不同）。"
echo "  · 判据**只用产品自带输出**（服务端 [px-access] 日志 + 客户端契约行）；"
echo "    刻意不用当时的插桩计数器（shared / recycle_shared / open_dup）—— 它们不在产品里。"
echo "  · 规模（前戏 4 + 3 轮 × 16）由 [6b] 量出的**槽位预算**决定，不是随手取的："
echo "    冷 listener 上异常率为 0（假阴性）⇒ 必须有前戏；而 508 的上限（M295 前 64 / 现 256）决定规模上限（本门取 16 并发 × 3 轮 + 前戏 4 = 52 ≪ 256）。"
echo "  · 不覆盖：QUIC 连接槽位耗尽本身（**缺陷 508** —— M295 已修，[6b] 只**登记**「90 次不触顶」；
    上限精确值与「满了要可观测」由 examples/m295_quic_slot 看守）；"
echo "    连接迁移（**缺陷 504**，由 examples/m293_h3_robustness 的 MIGRATE_KNOWN.tsv 登记）；"
echo "    0-RTT 接受率统计；多节点/集群形态；QUIC 层内部状态（只看客户端契约与访问日志）。"
echo "  · 负控 A 需**重建 runtime**（暖缓存 ≈15s / 冷缓存数分钟）⇒ CI 走 --neg-skip，完整档留本地全量门。"
echo "  · 负控 A 的撤回是**逐字节忠实**的（negctl.py --selftest 的 S3 用 sha256 证明）；"
echo "    该文件日后若因别的原因演进，S3 会转为 ℹ️（只做结构判据），不会假红。"
echo
echo "===== 失败 $FAIL 项 · 通过 $PASS 项 ====="
# ⚠️ 退出码必须反映 FAIL（M293 缺陷 506：运行器只看 rc，门内 ❌ 会被静默吞掉）
if [ "$FAIL" -eq 0 ]; then
  echo "M294-VERIFY-OK"
  exit 0
fi
exit 1
