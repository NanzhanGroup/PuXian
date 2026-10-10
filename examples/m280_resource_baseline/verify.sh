#!/usr/bin/env bash
# ============================================================
# M280 门 · **资源面判据**（第 1 批）：H3 托管 listener 的 CPU / RSS / 线程 / fd
# ------------------------------------------------------------
# 立论（本仓判据史的下一步）：
#   三轨对拍（输出）→ 期望值（值）→ 确定性（两遍逐字节）→ 结构（AST/IR）→
#   **资源（CPU / RSS / 线程 / fd）**。
#   为什么必须补这一维：**「忙自旋」「内存泄漏」「fd 泄漏」在输出面完全看不见** ——
#   输出可以逐字节正确，而进程在烧 CPU / 漏 fd。
#   `docs/HTTP3_STANCE.md` §三.2 早已登记「ERR_DRAINING 期间的读路径忙自旋尚未量化」，
#   而 M225 门只比**输出**，没有一条判据能看见它。
#
# ⚠️ 本轮最重要的一条方法论教训（首版 3 连红换来的）：
#   **资源判据必须判「收敛性」，不能判「绝对增量」** ——
#   实测 6 轮：RSS 增量为 +11.5/+11.9/+9.8/+1.6/+1.0/+1.9 MB（**趋缓**）
#     线程 35→55→75→95→82→81→90（**按需扩张至稳态**）· fd 恒定 8（无泄漏）
#     静置 15s 后 RSS 从 47384 **回落到** 42544（GC 生效）
#   ⇒ 首版「每轮增量 ≤ 2MB / 线程 ≤ 2」把**预热**误报成**泄漏**（3 连红）。
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M280_W:-/tmp/m280gate}"
NEG_SKIP="${NEG_SKIP:-0}"
mkdir -p "$W"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1（got=$2 want=$3）"; fi; }
le()  { if [ "$2" -le "$3" ]; then ok "$1（$2 ≤ $3）"; else bad "$1（$2 > 上限 $3）"; fi; }

echo "══ M280 · 资源面判据（H3 托管 listener 的 CPU/RSS/线程/fd）══"
BL="$HERE/BASELINE.tsv"

# ── [1] 静态：基线表形态与纪律 ─────────────────────────────
echo "[1] 静态：BASELINE.tsv 形态"
if [ ! -f "$BL" ]; then bad "[1] 缺 BASELINE.tsv"; else
  n=$(grep -vcE '^\s*(#|$)' "$BL" || true)
  nn=$(grep -vE '^\s*(#|$)' "$BL" | awk -F'\t' 'NF>=3 && $2 ~ /^[0-9]+$/ && length($3)>3' | wc -l)
  chk "[1a] 基线条目数" "$n" "6"
  chk "[1b] 全部「数字阈值 + 非空理由」" "$nn" "6"
  case "$(head -2 "$BL")" in *M280*) ok "[1c] 表头含来历锚点";; *) bad "[1c] 表头缺来历";; esac
  case "$(cat "$BL")" in *收敛*) ok "[1d] 含方法论锚点（收敛判据）";; *) bad "[1d] 缺收敛判据说明";; esac
fi
lim(){ grep -E "^$1\b" "$BL" | awk -F'\t' '{print $2}'; }

# ── [2][3] 动态：6 轮采样 + 收敛判据 ───────────────────────
SVC="$W/srv"; CLG="$W/cli_grace"; CLA="$W/cli_abrupt"
# M296：**语料更新也要触发重建** —— 修前只判「产物存在」⇒ 改了 srv.px/cli_*.px 后
#   门仍跑**旧产物**（等于「我改了门语料，但门测的还是老东西」）。
if [ ! -x "$SVC" ] || [ ! -x "$CLG" ] || [ ! -x "$CLA" ] \
   || [ "$HERE/srv.px" -nt "$SVC" ] || [ "$HERE/cli_grace.px" -nt "$CLG" ] || [ "$HERE/cli_abrupt.px" -nt "$CLA" ]; then
  echo "[2] 构建三个产物（首次 ≈60s）"
  mkdir -p "$W"
  cp -f "$HERE"/*.px "$W"/ 2>/dev/null
  for f in srv cli_grace cli_abrupt; do
    ( cd "$W" && timeout 300 "$ROOT/tools/px" build "$f.px" > "$W/b_$f.log" 2>&1 && cp -f "$W/build/$f" "$W/$f" ) || true
  done
fi
if [ ! -x "$SVC" ] || [ ! -x "$CLG" ] || [ ! -x "$CLA" ]; then
  bad "[2] 缺产物（$SVC / $CLG / $CLA）—— 构建失败，见 $W/b_*.log"
else
  PORT="${M280_PORT:-19698}"
  RDY="$W/ready.$$"; rm -f "$RDY"
  # ⚠️ 用独特进程名：`$!` 在本脚本上下文里拿不到真 PID（实测 proc=NO），
  #    而 `pgrep -f` 会匹配到自己的命令行 ⇒ 一律用 `pgrep -x <独特名>`。
  SVC2="$W/m280svc"; cp -f "$SVC" "$SVC2"
  ( M280_HTTP_PORT="$PORT" M280_READY="$RDY" exec "$SVC2" > "$W/srv.log" 2>&1 ) &
  for i in $(seq 1 40); do [ -f "$RDY" ] && break; sleep 0.5; done
  # ⚠️ M280 教训：srv.px **先写 ready 再 px_serve** ⇒ 端口被占时 ready 已写而绑定失败
  #    ⇒ 「就绪」必须**验证真的在监听**（否则测试在假前提下跑，读出一片 0）。
  for i in $(seq 1 20); do ss -ltn 2>/dev/null | grep -q ":$PORT " && break; sleep 0.5; done
  if ! ss -ltn 2>/dev/null | grep -q ":$PORT "; then
    bad "[2a] 服务端未真正监听 :$PORT（ready 文件存在但绑定失败，见 $W/srv.log）"
  fi
  SPID="$(ps -eo pid=,comm= 2>/dev/null | awk '$2=="m280svc"{print $1; exit}')"
  export SPID
  for i in $(seq 1 40); do [ -f "$RDY" ] && break; sleep 0.5; done
  if [ ! -f "$RDY" ]; then bad "[2] 服务端未就绪"; else
    ok "[2a] 服务端就绪（pid=$SPID port=$PORT）"
    _pid(){ echo "${1:-$SPID}"; }
    rss(){ awk '/VmRSS/{print $2}' "/proc/$(_pid "${1:-}")/status" 2>/dev/null | head -1 | tr -d ' ' || echo 0; }
    thr(){ awk '/Threads/{print $2}' "/proc/$(_pid "${1:-}")/status" 2>/dev/null | head -1 | tr -d ' ' || echo 0; }
    fdn(){ ls "/proc/$(_pid "${1:-}")/fd" 2>/dev/null | wc -l; }
    tick(){ awk '{print $14+$15}' "/proc/$(_pid "${1:-}")/stat" 2>/dev/null || echo 0; }

    R0=$(rss); T0=$(thr); F0=$(fdn)
    declare -a CR CH CF RQ
    ROUNDS="${M280_ROUNDS:-9}"
    # M283（判据的「第 0 维：前提」）：每轮**记录成功请求数** ——
    #   若某轮请求大量失败，RSS 曲线本就**不可比**（测的不是同一件事），门必须**响亮**说出来，
    #   而不是把「请求没跑起来」当成「内存泄漏」。
    round(){  # $1=序号
      local t0 t1 i ok=0
      t0=$(tick)
      for i in $(seq 1 10); do
        M280_HTTP_PORT="$PORT" timeout 20 "$CLA" >/dev/null 2>&1 && ok=$((ok+1))
        M280_HTTP_PORT="$PORT" timeout 20 "$CLG" >/dev/null 2>&1 && ok=$((ok+1))
      done
      sleep 3
      t1=$(tick)
      CR[$1]=$((t1-t0)); CH[$1]=$(rss); CF[$1]=$(fdn); RQ[$1]=$ok
      echo "     轮$1: cpu+${CR[$1]} rss=${CH[$1]} thr=$(thr) fd=${CF[$1]} ok=$ok/20"
    }
    echo "[2] $ROUNDS 轮采样（每轮 220 次请求 + 3s 静置）"
    CH[0]=$R0; CF[0]=$F0
    for r in $(seq 1 "$ROUNDS"); do round "$r"; done

    # ── v3（M296）：**均值**代替单点 —— 单点方差极大（CI 两次实测末轮增量 11968 ⇄ 2272，5×）
    inc(){ echo $(( CH[$1] - CH[$(( $1 - 1 ))] )); }
    avg3(){ echo $(( ($1 + $2 + $3) / 3 )); }
    d1=$(inc 1)
    f_avg=$(avg3 "$(inc 1)" "$(inc 2)" "$(inc 3)")
    l_avg=$(avg3 "$(inc $((ROUNDS-2)))" "$(inc $((ROUNDS-1)))" "$(inc $ROUNDS)")
    peak=0; for r in $(seq 0 "$ROUNDS"); do [ "${CH[$r]}" -gt "$peak" ] && peak=${CH[$r]}; done
    T3=$(thr); F6=$(fdn)
    L_CPU=$(lim cpu_tick_per_round); L_FIRST=$(lim rss_first_round_kb)
    L_RATIO=$(lim rss_converge_ratio); L_THR=$(lim thread_after_warmup); L_FD=$(lim fd_grow_total)
    L_PEAK=$(lim rss_peak_max_kb)

    echo "[3] 判据（收敛性 = 后段均值 vs 前段均值；不是单点绝对增量）"
    cmax=0; for r in $(seq 1 "$ROUNDS"); do [ "${CR[$r]}" -gt "$cmax" ] && cmax=${CR[$r]}; done
    le "[3a] CPU 每轮峰值 tick" "$cmax" "$L_CPU"
    # [3b] 只抓「一上来就失控」——绝对增量跨分配器不可比（见 BASELINE.tsv 头注的 CI/本机对照）
    le "[3b] 首轮 RSS 预热增量(KB)" "$d1" "$L_FIRST"
    lim6=$(( f_avg * L_RATIO / 100 ))
    if [ "$l_avg" -le "$lim6" ]; then
      ok "[3c] RSS 收敛（后 3 轮均值 +$l_avg ≤ 前 3 轮均值 $f_avg 的 ${L_RATIO}% = $lim6）"
    else
      bad "[3c] RSS 未收敛（后 3 轮均值 +$l_avg > $lim6 = 前 3 轮均值 $f_avg 的 ${L_RATIO}%）—— 疑似泄漏"
    fi
    le "[3f] RSS 峰值(KB)" "$peak" "$L_PEAK"
    dthr=$(( $(thr) - T3 ))
    if [ "$dthr" -le "$L_THR" ]; then ok "[3d] 预热后线程增量（$dthr ≤ $L_THR）"
    else bad "[3d] 预热后线程仍增长（+$dthr）—— 疑似线程泄漏"; fi
    dfd=$(( F6 - F0 ))
    if [ "$dfd" -le "$L_FD" ]; then ok "[3e] fd 总增量（$dfd ≤ $L_FD）"
    else bad "[3e] fd 泄漏（+$dfd）"; fi

    # [3g] **可回收**（正向证据）：静置后 RSS 应当回落 —— 证明「活着的对象确实被回收了」。
    #   ⚠️ 分配器不归还内存时「不回落」是**合法**的（BASELINE.tsv 头注已登记）⇒ 这是**登记项**，
    #   不单独判红；它与 [3c] 一起构成「不是泄漏」的证据。
    sleep 15
    idle=$(rss)
    if [ "$idle" -lt "$peak" ]; then
      ok "[3g] 可回收（静置 15s：峰值 $peak → $idle，回落 $(( peak - idle ))KB）"
    else
      echo "     ℹ️ [3g] 静置 15s 未回落（峰值 $peak → $idle）—— 分配器保留属正常；以 [3c] 的收敛性为准"
    fi

    # [3h] **前提自证**（M283）：曲线只有在「请求真的跑起来」时才有意义。
    weak=0
    for r in $(seq 1 "$ROUNDS"); do [ "${RQ[$r]}" -lt 16 ] && weak=$((weak+1)); done
    if [ "$weak" = "0" ]; then
      ok "[3h] 前提自证：每轮成功请求 ≥16/20（$(for r in 1 2 3; do printf '%s ' "${RQ[$r]}"; done)…）"
    else
      bad "[3h] 有 $weak 轮成功请求 <16/20 —— **RSS 曲线不可比**（测的不是同一件事）：$(for r in $(seq 1 "$ROUNDS"); do printf '%s ' "${RQ[$r]}"; done)"
    fi

    echo "[4] 负控：注入忙自旋 ⇒ CPU 判据必须判红"
    if [ "$NEG_SKIP" = "1" ]; then echo "     （--neg-skip：CI 档跳过）"
    else
      pkill -x m280svc 2>/dev/null; sleep 1; rm -f "$RDY"
      ( M280_HTTP_PORT="$PORT" M280_READY="$RDY" M280_SPIN=1 exec "$SVC2" > "$W/srv_spin.log" 2>&1 ) &
      for i in $(seq 1 40); do [ -f "$RDY" ] && break; sleep 0.5; done
      SPID2="$(ps -eo pid=,comm= 2>/dev/null | awk '$2=="m280svc"{print $1; exit}')"
      export SPID2
      for i in $(seq 1 40); do [ -f "$RDY" ] && break; sleep 0.5; done
      t0=$(awk '{print $14+$15}' "/proc/$SPID2/stat" 2>/dev/null || echo 0)
      sleep 5
      t1=$(awk '{print $14+$15}' "/proc/$SPID2/stat" 2>/dev/null || echo 0)
      if [ $((t1-t0)) -gt "$L_CPU" ]; then ok "[4a] 忙自旋被抓住（+$((t1-t0)) > $L_CPU）"
      else bad "[4a] 负控失效：忙自旋未被 CPU 判据抓住（+$((t1-t0))）"; fi
      pkill -x m280svc 2>/dev/null
    fi
    pkill -x m280svc 2>/dev/null; sleep 1; rm -f "$RDY"
  fi
fi

echo "[5] 覆盖边界（如实）"
cat <<'TXT'
     · 只覆盖 px_serve 的 **H3 托管 listener**（QUIC/H3 路径）；HTTP/1.1 明文路径不在面内。
     · 判据一律是**增量 + 收敛性**（不是绝对值）：基线随机器/内核变化，判「是否收敛」才可判定。
     · 「忙自旋」判据 = CPU tick 增量（HZ=100 ⇒ 1 tick ≈ 1% 单核·秒），采样窗口 5s。
     · RSS 看 VmRSS；allocator 不归还内存时「不回落」可能掩盖真泄漏 ⇒ 本门只做**筛查**。
     · 线程/fd 只数**个数**，不判身份/类型。
     · **未覆盖**：长时间（>1h）稳态漂移；多 listener 并发；TLS 路径的资源面。
TXT

echo ""
echo "══ 汇总：$PASS 通过 / $FAIL 失败 ══"
[ "$FAIL" = "0" ] && { echo "M280-VERIFY-OK"; exit 0; } || { echo "M280-VERIFY-FAIL"; exit 1; }
