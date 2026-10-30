#!/bin/bash
# ══════════════════════════════════════════════════════════════════════
# packaging/chain/shard_run.sh —— 全量门「分片并行」调度器（M303 步3 · M317 入仓）
#
# ⚠️ 入仓说明（M317 · 纪律来自 M284 缺陷 490「编排脚本住 /tmp ⇒ 守卫扫不到」）：
#   本文件是**权威版本**；/data/gatevm/shard_run.sh 是宿主运行副本（内容曾长期只在那里）。
#   路径可覆盖：PX_SRC（默认=仓库根，从脚本位置推导）· PX_GATEVM（默认 /data/gatevm）。
#   宿主数据依赖（不是代码）：$BASE/times*.tsv · quiet_needed.txt · net_needed.txt ·
#                              $BASE/wt{1..N} · $BASE/tmp{1..N} · $BASE/shard-<ts>/
#
# 来历：M302 收尾时做的步1/步2 侦察（实测）——
#   · worktree 检出 198MB / 1.8s（.git 共享）
#   · .rtcache 10G ⇒ cp -al 硬链接副本（1.7s / 真实占用≈0，inode 共享已验证）
#   · 隔离：unshare(mount+pid+net) + bind 私有 /tmp ⇒ **锁文件天然分片**
#     （gate_lock.sh 的全局单锁在私有 /tmp 里，各分片互不可见 ⇒ 无需改产品代码）
#   · 门峰值内存实测 2.0GB（最重门）· 8 核 16G 足够 4 路
#
# 用法：
#   shard_run.sh [N]            N=分片数（M351 起**默认 = nproc/SHARD_JOBS**：16 核 ⇒ 8 片，
#                               8 核 ⇒ 4 片；也可 SHARD_N= 覆盖。旧行为：显式给 4 即 4 片）
#   shard_run.sh 4 --dry        只输出分片规划，不跑
#   shard_run.sh 4 --times F    指定耗时依据 TSV（默认用备份的上一轮全量数据）
#   shard_run.sh 4 --only a,b,c 只跑子集（M342 · 诊断用）：门名必须在 HEAD 注册清单里，否则 exit 4
#                               ——「偶发」只在并发+隔离条件下复现，串行重跑复现不出（缺陷 598）
#   ⭐ M309 阶段B（2026-10-17）起：**默认夹住门内并行度**（档A/档A' 实测口径）
#     SHARD_JOBS=2（每路 GATE_JOBS）+ 按 N 自动切 N 段连续核区间 taskset 绑核
#     ⇒ 4×2 = 8 线程恰好占满 8 核（不超订）· 实测 118 min → 87 min（1.36×）
#     覆盖：SHARD_JOBS=3 / SHARD_CPUS="0-3,4-7" ｜ **退回旧行为：SHARD_NO_CLAMP=1**
#
# 超时：GATE_TIMEOUT_DEFAULT=900（分片与 quiet 档同口径）。
#   ⭐ M305：`upstream_tests` **单独放宽到 1200s** —— 2026-10-12 宿主冷迁移后机器慢 1.4–2.3×，
#      该门实测从 525s（EPYC）涨到 **>901s** ⇒ 在 shard1 撞 900s 上限被判超时（rc=124）。
#      放宽依据是**实测**，不是“怕红”；其余门仍 900s。
#   ⭐⭐ M306（2026-10-13）：**1200 → 1800**。同一个门第三次撞线，而这次的原因**不是"再慢了一点"**：
#      · 本分发**不改 runtime** 的轮次：该门 ≈950s（M305 分片实测）⇒ 1200s 只剩 26% 余量；
#      · 本分发**改了 runtime**（M306 动了 onnx_*.c）⇒ `.rtcache` 的 key 全变 ⇒ 该门里
#        每个 build 配置都要**冷编译**一遍 ⇒ 实测 **1201s ≥ 1200s**（rc=124，差 1 秒）。
#      · 而分片的 `.rtcache` 是**每轮从主树 reflink 一份**（`cp -a --reflink=auto`，不回写）
#        ⇒ 「改了 runtime 的轮次必然冷」是**结构性**的，不是偶发。
#      ⇒ 取 1800s：对**实测最坏值**（冷档 1201s）留 **50% 余量**。
#      ⚠️ 这条超时**同时**要贴在 quiet 档上（否则它落到 900s 默认值上，比原来还紧）。
#   ⭐⭐⭐ M311（2026-10-18）：**这两处硬编码已删除** —— 知识搬回仓库。
#      为什么：上面 M305/M306 的结论**只写在宿主脚本里**，仓内既没有记录、也没有判据 ⇒
#      任何人跑**串行权威档**（`gate_verdict.sh` 裁决的就是它）都撞 900s：
#      M310 D2 段2 实测 `upstream_tests` **963s（空闲）/ 902s（分片）** ⇒ rc=124，
#      成为那一轮全量门唯一红。现在声明在 `selfhost/gates.registry.sh`：
#      `gate_timeout upstream_tests 1800`（分片/串行/D2 同源），并由
#      `selfhost/check_gate_timeouts.sh` 用**实测基线**盯裕度（≥80% 上限即判红）。
#      ⚠️ 分片 worktree 恒取自 HEAD（下方 worktree add --detach $HEAD_SHA）⇒ 注册源一定在场。
#
# 判据（步3 验收）：**并行结果 == 串行结果**（逐门 rc + 状态），且至少跑两轮。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail

N=""; N_SET=0; DRY=0
# ── M351（缺陷 613）：耗时表默认值移出 /tmp ──
#   旧默认 = /tmp/gatevm/times_m302_full.tsv（10-12 手工产物，2026-10-28 实测仍在）。两处病：
#     ① 住 /tmp ⇒ 重启即被清；被清后 awk 读不到任何门 ⇒ 每门 t 退化成 60，
#        LPT **静默降级成轮转分配**（日志里一个字都不提示）—— 590 族「判据通道没了就当默认值」。
#     ② 数字比实测小 3.58×（209 门 6305s vs 10-24 实测 219 门 23473s），86 门偏差 >60s。
#   ⇒ 新口径：`times_selftuned.tsv`（times_selftune.sh --write 用上一轮实测回灌）
#             > `times_latest.tsv`（10-24 实测）> 旧表（仅兜底）
#   **选中哪张表必须打进日志**（可审计）；一张都没有 ⇒ 响亮警告而不是静默 60s。
TIMES=""
# ── M309 C1 A/B（2026-10-16）：夹住「门内并行度」的两个旋钮 ──
#   动机（实测）：分片 4 路 × GATE_JOBS 默认(=本机核数 8) = 32 线程抢 8 核
#   ⇒ 每路负载被抬高 2.5–3.3× ⇒ 净加速只有 ≈1.0–1.3×（M306 R5 实测 118 min）。
#   口径：**默认全空 = 既有行为一字不改**；只有显式设了才生效。
#     SHARD_JOBS=2   ⇒ 每路 GATE_JOBS=2（门内并行度上限）
#     SHARD_CPUS="0-1,2-3,4-5,6-7" ⇒ 第 i 路 taskset -c <第 i 段>（逗号分隔）
# ── M309 C1 阶段B 收口（2026-10-17 · 东月）：夹并行度 = **默认口径** ──
#   实测依据（A/B 两档各 87 min，老分片 M306 R5 = 118 min）：
#     · 不夹：4 路 × GATE_JOBS=8(=本机核数) = **32 线程抢 8 核** ⇒ 净加速仅 ≈1.0–1.3×
#     · 夹：4 路 × GATE_JOBS=2 + 绑核 = **8 线程不超订** ⇒ 118 → 87 min（1.36×），
#       且逐门 (状态, rc) 与串行权威档一致（shard_verdict.py 逐门对拍）
#   默认：SHARD_JOBS=2；SHARD_CPUS 未给 ⇒ 按 N 自动切 N 段连续核区间（见下）。
#   退出开关：**SHARD_NO_CLAMP=1** ⇒ 两变量都不设，完全恢复旧行为（含 SHARD_JOBS/SHARD_CPUS 也不再生效）。
SHARD_NO_CLAMP="${SHARD_NO_CLAMP:-}"
SHARD_JOBS="${SHARD_JOBS:-}"
SHARD_CPUS="${SHARD_CPUS:-}"
while [ $# -gt 0 ]; do
  case "$1" in
    --dry)   DRY=1 ;;
    --times) shift; TIMES="${1:-}" ;;
    --only)  shift; ONLY_GATES="${1:-}" ;;
    [0-9]*)  N="$1"; N_SET=1 ;;
    *) echo "未知参数: $1" >&2; exit 4 ;;
  esac
  shift
done

# ── M351（缺陷 614）：分片数默认值「按核数推导」，不再写死 4 ──
#   根因（M351 STEP 0 实测）：4 片 × SHARD_JOBS=2 = **8 线程**，是 M309 在 8 核机上设对的；
#   2026-10 换 16 核机后没人跟着改 ⇒ 起门后 loadavg ≈5.0/16（**一半 CPU 空转**），
#   墙钟 2h04–2h11 的全部优化空间就在这里。
#   口径：默认 N = nproc / SHARD_JOBS（默认 2）⇒ 16 核 → 8 片，8 核 → 4 片（**老机行为不变**）；
#        夹在 2..16；`SHARD_NO_CLAMP=1`（不夹并行度）时退回旧默认 4。
#   覆盖：位置参数 `shard_run.sh 8` 或环境变量 `SHARD_N=10`。
if [ -z "$N" ] && [ -n "${SHARD_N:-}" ]; then N="$SHARD_N"; N_SET=1; fi
if [ "$N_SET" = 0 ]; then
  if [ -n "$SHARD_NO_CLAMP" ]; then
    N=4; _N_SRC="旧默认（SHARD_NO_CLAMP=1 ⇒ 不夹门内并行度，核数不参与推导）"
  else
    _J="${SHARD_JOBS:-2}"; case "$_J" in ''|*[!0-9]*|0) _J=2 ;; esac
    _C0="$(nproc 2>/dev/null || echo 8)"
    N=$(( _C0 / _J ))
    [ "$N" -lt 2 ] && N=2; [ "$N" -gt 16 ] && N=16
    _N_SRC="按核数推导 ${_C0} 核 / ${_J} 线程 = ${N} 片"
  fi
  echo "（M351：未显式给分片数 ⇒ N=$N（$_N_SRC）· 位置参数或 SHARD_N= 可覆盖）"
fi

# ── M330（2026-10-26 · 东月）：默认推导 —— 「算力 / 门内并行度 / 负载」三件事同源 ──
#   为什么改（r18 实测 · /data/gatevm/r18-N10J1-20261024-220410.log）：
#     旧推导 `step = c // n` 把**余数全塞给最后一片** —— 16 核 / N=10 实测绑成
#     「0-0,1-1,…,8-8,9-15」⇒ shard1..9 各 **1 核**，shard10 独吞 **7 核**；
#     而 LPT 又把负载平分成每片 2437s ⇒ **用 1 核实去打 7 核的活**（这是可静态核对的缺陷）。
#   ⚠️ 如实登记（2026-10-26 负控反证）：本轮先怀疑「1 核绑定 ⇒ helper 线程被饿死」
#     （依据 r18 里 m260 基线 23s → 900s、m256 116s → 900s，量级不像"慢"）。
#     **负控没撑住这个假说**：同一棵树、同一门、taskset 单核 + JOBS=1 单独跑
#     m260 = **13s 通过**（/tmp/neg-A1_m260_core0.log）；不绑核 + JOBS=2 = 72s 通过；
#     m256 不绑核 + JOBS=2 = 70s 通过。⇒ **「单核饿死」不成立，病因仍未定论**。
#     ⇒ 因此本文件里的装置改动按「可静态核对的缺陷」落地（余数均摊），
#       「不绑到少于 2 核」按**风险规避**落地（不是已证病因）。
#       r18 那 5 门 rc=124 的真因，留给下一轮「10 路并发复现」去定（待办）。
#   ⇒ SHARD_JOBS 下限 2：JOBS=1 会让「门内并行」的门退化串行（m244/m302 一族）；
#     JOBS 只影响门内并行度，与「门之间串行」正交（见 run_gates.sh 头注）。
#     （⚠️ 反例已登记：m260 单独跑时 JOBS=1/单核 **13s**、JOBS=2/不绑 **72s** ——
#       「JOBS 越大越快」不普适，别把这条当成加速依据。）
#   负载均衡口径：分片内的门是**串行链** ⇒ 墙钟 ≈ Σt_g，**不按核数除**（LPT 用纯 Σt 均衡）。
#     核数只决定「单个门内能用几个线程」，只对 JOBS>1 的门有意义 ⇒ 不拿核数当负载权重
#     （上一版把「核多 = 能扛更多」当默认，是把串行链当并行池，属判据贴错表象）。
if [ -z "$SHARD_NO_CLAMP" ]; then
  SHARD_JOBS="${SHARD_JOBS:-2}"
  SHARD_CPUS="${SHARD_CPUS:-}"
  _C="$(nproc 2>/dev/null || echo 1)"
  _BASE=$(( _C / N )); _REM=$(( _C % N ))
  if [ "$_BASE" -lt 2 ]; then
    SHARD_BIND=0
    echo "（M330：N=$N · ${_C} 核 ⇒ 每片 $(( _C / N )) 核 < 2 ⇒ **不绑核**（保守：单核绑定下 helper 线程无余量；病因未定论，见文件头 2026-10-26 负控登记）· SHARD_JOBS=$SHARD_JOBS · 退回旧行为 SHARD_NO_CLAMP=1）"
  else
    SHARD_BIND=1
    if [ -z "$SHARD_CPUS" ]; then
      SHARD_CPUS="$(python3 - "$N" "$_C" <<'PY'
import sys
n, c = int(sys.argv[1]), int(sys.argv[2])
base, rem = c // n, c % n
segs, a = [], 0
for i in range(n):
    k = base + (1 if i < rem else 0)   # 余数**按片均摊**（旧版全塞最后一片 ⇒ 1 核 ×9 + 7 核 ×1）
    segs.append("%d-%d" % (a, a + k - 1))
    a += k
print(",".join(segs))
PY
)"
    fi
    echo "（M330：N=$N · ${_C} 核 ⇒ 每片 ${_BASE}$( [ "$_REM" -gt 0 ] && printf '/%d' $(( _BASE + 1 )) ) 核 · 绑核 $SHARD_CPUS · SHARD_JOBS=$SHARD_JOBS · 退回旧行为 SHARD_NO_CLAMP=1）"
  fi
else
  SHARD_BIND=0
  echo "（SHARD_NO_CLAMP=1：不夹门内并行度 / 不绑核 —— 旧行为）"
fi
case "$N" in ''|*[!0-9]*) echo "❌ N 必须是正整数（给的是「$N」）" >&2; exit 4 ;; esac
[ "$N" -ge 1 ] && [ "$N" -le 64 ] || { echo "❌ N 必须在 1..64（给的是 $N）" >&2; exit 4; }

SRC="${PX_SRC:-$(cd "$(dirname "$0")/../.." && pwd)}"
BASE="${PX_GATEVM:-/data/gatevm}"
TS=$(date +%Y%m%d-%H%M%S)
LOG="$BASE/shard-$TS"
mkdir -p "$LOG"

# ── 1a) 耗时表解析（M351 · 缺陷 613）：显式 --times 优先；否则按「新→旧」三级回落并把选择打进日志 ──
TIMES_SRC="--times 显式指定"
if [ -z "$TIMES" ]; then
  for _cand in "$BASE/times_selftuned.tsv" "$BASE/times_latest.tsv" "$BASE/times_m302_full.tsv" /tmp/gatevm/times_m302_full.tsv; do
    if [ -s "$_cand" ]; then TIMES="$_cand"; TIMES_SRC="自动回落"; break; fi
  done
fi
if [ -z "$TIMES" ] || [ ! -s "$TIMES" ]; then
  echo "⚠️ 没有可用的耗时表（找过 $BASE/times_{selftuned,latest,m302_full}.tsv）⇒ **每门按 60s 估计**，LPT 退化为轮转分配"
  TIMES=/dev/null; TIMES_SRC="无表（退化：每门 60s）"
fi
echo "耗时表：$TIMES（$TIMES_SRC）"

echo "═══ 分片调度：N=$N · 日志 $LOG ═══"

# ── 1) 门清单（**取自 HEAD**，而不是主树）──
# ⚠️ 为什么不能用主树的 `run_gates.sh --list`（2026-10-12 实测）：主树可能有**别处的未提交改动**
#    —— 当时主树多了个新门 `selfwake_guard`（另一个会话在建），而分片 worktree 检出的是 HEAD、
#    没有那个门 ⇒ 分片会去跑一个**不存在的门**。分片跑的是 HEAD，清单就必须与 HEAD 同源。
#    `--list` 读的是 `gates.registry.sh` 的 `run <name>` 行 ⇒ 直接从 HEAD 提取即可。
git -C "$SRC" show "HEAD:selfhost/gates.registry.sh" | sed -n 's/^run \([A-Za-z0-9_]*\) .*/\1/p' > "$LOG/gates.txt"

# ── 1b) M342：`--only` 子集（诊断用 · 缺陷 598 的处置）──
#   为什么加：M342 实测 21 个「单发取样 + 驾驭活进程」的门**串行重跑 63 轮全绿**，
#   而 6 个已定性偶发**全部只在 4 分片并发下出现**（同树单跑必绿）。
#   ⇒ 复现偶发必须在**同样的并发+隔离条件**下跑子集，否则观测条件 ≠ 现象发生条件（与缺陷 589 同族）。
#   ⚠️ 前提自证：请求的每个门名必须**存在于** HEAD 注册清单，否则响亮报错退出
#      （否则「少跑了几个门」会被读成「这轮全绿」——正是 M336 空跑必红要防的那类假绿）。
if [ -n "${ONLY_GATES:-}" ]; then
  : > "$LOG/gates.all.txt"; cp "$LOG/gates.txt" "$LOG/gates.all.txt"
  MISSING=""
  : > "$LOG/gates.txt"
  for g in $(printf '%s' "$ONLY_GATES" | tr ',' ' '); do
    if grep -qx "$g" "$LOG/gates.all.txt"; then
      echo "$g" >> "$LOG/gates.txt"
    else
      MISSING="$MISSING $g"
    fi
  done
  if [ -n "$MISSING" ]; then
    echo "❌ --only 里有不在 HEAD 注册清单里的门：$MISSING" >&2
    echo "   （拒绝降级成「跑剩下的」：那会把少跑当成全绿）" >&2
    exit 4
  fi
  echo "⭐ --only 子集模式：$(wc -l < "$LOG/gates.txt") / $(wc -l < "$LOG/gates.all.txt") 门"
fi
NG=$(wc -l < "$LOG/gates.txt")
echo "门数: $NG（清单取自 HEAD:$(git -C "$SRC" rev-parse --short HEAD)）"

# ── 2) 耗时依据 ──
: > "$LOG/plan.tsv"
MISS=0
while read -r g; do
  [ -z "$g" ] && continue
  t=$(awk -F'\t' -v G="$g" '$1==G{print $3}' "$TIMES" 2>/dev/null | head -1)
  [ -z "$t" ] && { t=60; MISS=$((MISS+1)); }   # 未见过的门给保守估计
  printf '%s\t%s\n' "$t" "$g" >> "$LOG/plan.tsv"
done < "$LOG/gates.txt"
# M351：缺表门数必须打出来 —— 旧版静默按 60s 处理，缺得多就等于表没用（跟"没有表"一样）。
echo "耗时表未覆盖的门：$MISS / $NG（按 60s 保守估计）"
[ "$MISS" -gt 20 ] && echo "⚠️ 缺表门 $MISS 个 ⇒ 本表区分度不足，均衡全靠缺省 60s"

# ── 3) LPT 分片（最贵的先排）──
python3 - "$N" "$LOG/plan.tsv" "$LOG" <<'PY'
import sys
N=int(sys.argv[1]); plan=sys.argv[2]; out=sys.argv[3]
rows=[]
for ln in open(plan, encoding='utf-8'):
    p=ln.rstrip('\n').split('\t')
    if len(p)==2:
        try: rows.append((int(p[0]), p[1]))
        except ValueError: pass
rows.sort(reverse=True)
bins=[[] for _ in range(N)]; load=[0]*N
for t,g in rows:
    i=load.index(min(load)); bins[i].append(g); load[i]+=t
for i,b in enumerate(bins):
    open('%s/shard%d.txt'%(out,i+1),'w',encoding='utf-8').write('\n'.join(b)+'\n')
print('分片负载(s):', load)
# 墙钟模型（M330）：分片内的门是串行链 ⇒ 墙钟下界 = max(每片 Σt)，**不按核数除**。
#   旧版把「核数」当吞吐倍数会报出比实际乐观 1.6–2× 的数字（N=10 r18 报 40.6min / 实测 51min）。
print('串行总量 %d s (%.1f min) ⇒ 墙钟下界 %.1f min（单门下界 %d s）'
      % (sum(load), sum(load)/60.0, max(load)/60.0, max(rows)[0] if rows else 0))
print('说明：上界受「quiet 串行尾」支配，最终墙钟 = 本值 + quiet 档（见 --dry 末尾）')
PY

# ── 3b) 出网门：pin 到 shard1（唯一**带宿主网络**的分片）──
#   为什么需要（2026-10-12 首航实测）：`--net` 隔离让需要 UDP 出网的用例失败
#     （`upstream_tests` 的 dns 用例：`udp_send 失败 (errno=101)`）—— 那不是缺陷，是隔离的代价。
#   为什么只放开**一个**分片：分片之间**靠 netns 互不可见**才不会撞端口（P1 登记的 10 个端口）。
#     只让 shard1 看宿主网络 ⇒ 它只需与**宿主自身服务**不冲突（门都用自用端口，实测无冲突），
#     而 shard2..N 之间仍完全隔离 ⇒ 并行性不受影响。
# ── M364 ①：出网登记表**入仓** ⇒ 读「仓内 ∪ 宿主」合并（2026-10-30 第 237 轮净检） ──
#   病灶：此前登记表**只**在宿主 $BASE/net_needed.txt（宿主目录，非仓库），
#     而 gates.registry.sh 里只有一句注释（不是机器可读元数据）⇒ 换机 / 重建 gatevm 后
#     表就没了 ⇒ M363 那两红（出网门被塞进隔离片、8 次探测全 000）**静默复现**，
#     且 check_registry_external.sh 只提示、不判红，没有任何门拦得住。
#   修法：仓内 `selfhost/net_needed.txt` 为权威副本 + 宿主表仍兼容（合并去重，python 侧 set 去重）。
#   清单同源原则：优先取 HEAD（分片跑的就是 HEAD）；HEAD 无此表则取工作树（首次提交前的自测）。
NET_EFF="$LOG/net_needed.effective.txt"
: > "$NET_EFF"
if git -C "$SRC" cat-file -e "HEAD:selfhost/net_needed.txt" 2>/dev/null; then
  git -C "$SRC" show "HEAD:selfhost/net_needed.txt" >> "$NET_EFF"
  echo "（M364 出网表：HEAD:selfhost/net_needed.txt$( [ -s "$BASE/net_needed.txt" ] && echo ' ∪ 宿主 net_needed.txt' )）"
elif [ -s "$SRC/selfhost/net_needed.txt" ]; then
  cat "$SRC/selfhost/net_needed.txt" >> "$NET_EFF"
  echo "（M364 出网表：工作树 selfhost/net_needed.txt（HEAD 尚无）$( [ -s "$BASE/net_needed.txt" ] && echo ' ∪ 宿主 net_needed.txt' )）"
fi
if [ -s "$BASE/net_needed.txt" ]; then
  cat "$BASE/net_needed.txt" >> "$NET_EFF"
fi

if [ -s "$NET_EFF" ]; then
  python3 - "$LOG" "$N" "$NET_EFF" <<'PY'
import sys, os
log, n, nn = sys.argv[1], int(sys.argv[2]), sys.argv[3]
want = set(l.split('#')[0].strip() for l in open(nn, encoding='utf-8') if l.split('#')[0].strip())  # M309 阶段B：剥行内注释
bins = []
for i in range(1, n + 1):
    p = os.path.join(log, 'shard%d.txt' % i)
    bins.append([l.strip() for l in open(p, encoding='utf-8') if l.strip()])
# M342：只对**本次真的要跑**的门生效 —— 否则 `--only` 子集会被 net_needed 把 upstream_tests 塞回来
#       （2026-10-27 实测：`--only m245,m150 --dry` 打出 shard1 有 2 门，实为上游门被追加）
present = set(g for b in bins for g in b)
want = set(g for g in want if g in present)
moved = []
for b in bins[1:]:
    for g in list(b):
        if g in want:
            b.remove(g); moved.append(g)
for g in sorted(want):
    if g not in bins[0]:
        bins[0].append(g); moved.append(g)
for i, b in enumerate(bins, 1):
    open(os.path.join(log, 'shard%d.txt' % i), 'w', encoding='utf-8').write('\n'.join(b) + '\n')
# 报「**最终**在 shard1 的出网门」（不是"移动了几个" —— 首版因此误报「（无）」，
# 而实际上 upstream_tests 本来就被装进了 shard1 ⇒ 只是没"移动"，不是没生效）
final = sorted(g for g in bins[0] if g in want)
print('出网门在 shard1 的有 %d 个：%s' % (len(final), ', '.join(final) or '（无）'))
missing = sorted(want - set(final))
if missing:
    print('⚠️ 有出网门没落到 shard1（隔离档下会失败）：%s' % ', '.join(missing))
PY
fi

# ── 3c) 低负载门：从分片里**摘出去**（分片跑完后单独串行跑；见 quiet_needed.txt）──
QUIET_LIST=""
if [ -s "$BASE/quiet_needed.txt" ]; then
  # ── M309 阶段B · 行内注释剥离（实测缺陷修复）──
  #   档A' 日志实测：quiet_needed.txt 里 m208 那行带了**行内注释**，而旧解析只剥「# 开头」的整行
  #   ⇒ 门名被拼成「m208_vm_c_local_roots#追加证据：宿主重负载（dnf-makecache）下亦假红（M307r4）」
  #   这种**脏门名**传进 --only（A' 的 /tmp/gatevm/m309ab_a2.log 第 17 行可复现）。
  #   本仓纪律：数据文件照写，解析器必须健壮 ⇒ 统一「先剥行内注释、再分列」。
  QUIET_LIST="$(sed -e 's/#.*//' "$BASE/quiet_needed.txt" | tr -d ' \t' | grep -v '^$' | paste -sd,)"
  # M342：--only 子集模式下，quiet 档也必须收敛到子集内 ——
  #   否则「跑 21 门」会悄悄多跑 10 个低负载门（多跑不算错，但会让「这轮跑了什么」说不清）。
  if [ -n "${ONLY_GATES:-}" ]; then
    _q="$(printf '%s' "$QUIET_LIST" | tr ',' '\n' | grep -Fx -f "$LOG/gates.txt" | paste -sd,)"
    QUIET_LIST="${_q:-}"
    echo "  （--only 子集：quiet 档收敛为「${QUIET_LIST:-无}」）"
  fi
  python3 - "$LOG" "$N" "$BASE/quiet_needed.txt" <<'PY'
import sys, os
log, n, qf = sys.argv[1], int(sys.argv[2]), sys.argv[3]
want = set(l.split('#')[0].strip() for l in open(qf, encoding='utf-8') if l.split('#')[0].strip())  # M309 阶段B：剥行内注释
gone = []
for i in range(1, n + 1):
    p = os.path.join(log, 'shard%d.txt' % i)
    b = [l.strip() for l in open(p, encoding='utf-8') if l.strip()]
    gone += [g for g in b if g in want]
    open(p, 'w', encoding='utf-8').write('\n'.join([g for g in b if g not in want]) + '\n')
print('低负载门已摘出分片（改串行后跑）：%s' % (', '.join(sorted(set(gone))) or '（无）'))
PY
fi

if [ "$DRY" = 1 ]; then
  echo "── 各分片门数 / 核数 ──"
  for i in $(seq 1 "$N"); do
    _cpus=""
    if [ -n "$SHARD_CPUS" ]; then _cpus="$(printf '%s' "$SHARD_CPUS" | cut -d, -f"$i")"; fi
    if [ -n "$_cpus" ]; then _cn="$(( $(echo "$_cpus" | awk -F- '{print ($2==""?1:$2-$1+1)}') ))"; else _cn="≈$(printf '%.1f' "$(echo "$(nproc) $N" | awk '{print $1/$2}')")"; fi
    # M342：`wc -l` 会把「空分片的那个换行」数成 1 门 ⇒ 实打实测到「shard3: 1 门」而实际 0 门（假读数）。
    #       改成数**非空行**。
    echo "  shard$i: $(grep -c . "$LOG/shard$i.txt") 门 · 核 ${_cpus:-不绑}（${_cn}）· JOBS=$SHARD_JOBS"
  done
  if [ -n "$QUIET_LIST" ]; then
    _qt="$(awk -F'\t' -v L="$QUIET_LIST" 'BEGIN{n=split(L,a,",");for(i=1;i<=n;i++)w[a[i]]=1} $1 in w {s+=$3} END{printf "%d", s}' "$TIMES" 2>/dev/null)"
    echo "  quiet 串行尾：$(printf '%s' "$QUIET_LIST" | awk -F, '{print NF}') 门 · 约 $(( ${_qt:-0} / 60 )) min（与分片串行，必须算进墙钟）"
  fi
  echo "（--dry：不启动）"; exit 0
fi

# ── 4) 准备分片环境（worktree + .rtcache reflink 副本 + 私有 tmp + 私有 devbuild 槽）──
# ⚠️ worktree **不能放 /tmp 下** —— 私有 /tmp 一 bind 就把它遮蔽了（步2 实测）
# ⚠️ .rtcache 用 **reflink（COW）** 而不是硬链接：实测本机 / 是 XFS reflink=1
#    （cp -a --reflink=auto 全量 10G 耗时 6.4s · 0 空间 · 改副本不影响源，已实证）。
#    硬链接的两个隐患：① 若某 key 目录「存在但缺 .complete」，rt_cache_compile 会
#    `cp src $dir/` 与 `cc -o $dir/x.o` **就地截断**同名 inode ⇒ 源 + 全部分片一起污染；
#    ② 无法界定「谁写脏了谁」。reflink 从根上消掉这类风险，代价只有几秒。
# ⚠️ devbuild 多槽（/tmp/devbuild_slots）也要**每分片一份**：回填是
#    `cp -a $out $slot/$name` 后写 key ⇒ 多分片同填一个槽时读者可能拿到半个二进制。
#    默认路径在私有 /tmp 里 ⇒ 不预置就是**空槽**（每片冷建）。
HEAD_SHA="$(git -C "$SRC" rev-parse HEAD)"
for i in $(seq 1 "$N"); do
  WT="$BASE/wt$i"
  # 每次重建 worktree：消掉「上一轮残留 vs 这一轮 HEAD」这类不可见差异
  # （步2 实测：检出 198MB / 1.8s，代价可忽略）
  git -C "$SRC" worktree remove --force "$WT" >/dev/null 2>&1
  rm -rf "$WT"
  echo "── 建 worktree wt$i @ ${HEAD_SHA:0:8} ──"
  git -C "$SRC" worktree add --detach "$WT" "$HEAD_SHA" >/dev/null 2>&1 || { echo "❌ wt$i 建树失败"; exit 3; }
  cp -a --reflink=auto "$SRC/.rtcache" "$WT/.rtcache" || { echo "❌ wt$i .rtcache 副本失败"; exit 3; }
  # ⚠️ 私有 /tmp **每次清空重建**（不能只删 devbuild_slots）——
  #   实测（2026-10-12）：上一轮被 `kill -KILL` 打断时，分片里的门锁文件
  #   （`/tmp/m116_gates.lock`）留了下来，里面记的是**分片 PID 命名空间里的低号 PID**（4），
  #   而 `kill -0 4` 在新命名空间里仍然算"活着" ⇒ 之后每一轮分片都报
  #   「已有 m116 全门在跑（PID 4）…拒绝启动」⇒ **整轮空跑**（211 门一个没跑，汇总却只报
  #   「合计 绿 3」（quiet 档那 3 个）—— 见下面 7) 新增的"空跑必红"判据）。
  rm -rf "$BASE/tmp$i"
  mkdir -p "$BASE/tmp$i"
  if [ -d /tmp/devbuild_slots ]; then
    cp -a --reflink=auto /tmp/devbuild_slots "$BASE/tmp$i/devbuild_slots" 2>/dev/null \
      || echo "  ⚠ wt$i：devbuild 槽副本失败（仍可跑，会冷建）"
  fi
done

# ── 5) 启动 ──
PIDS=()
SKIPPED=0
for i in $(seq 1 "$N"); do
  ONLY=$(paste -sd, "$LOG/shard$i.txt")
  # ── M352（缺陷 615）：**0 门的分片不启动** ──
  #   实测（2026-10-28 V5 负控）：`--only` 子集只给 4 门、N=8 ⇒ 后 4 片 shard$i.txt 为空
  #   ⇒ 命令行拼成 `run_gates.sh --only ''`，而 run_gates.sh 把**空值当「不过滤」**
  #   ⇒ 空片各跑了 30–34 门全量门（shard5..8.log 绿 31/31/32/34），白烧 4 份全量。
  #   判据口径：0 门就是 0 门，**不启动**比「启动后跑全量」安全；rc 记 0、日志写明原因。
  if [ -z "${ONLY//[[:space:]]/}" ]; then
    echo "  ⏭  shard$i：0 门（不启动 —— 空 --only 会被当成「不过滤」跑全量 · 缺陷 615）"
    echo "shard$i：0 门（跳过 · 缺陷 615）" > "$LOG/shard$i.log"
    echo 0 > "$LOG/shard$i.rc"
    SKIPPED=$((SKIPPED+1))
    continue
  fi
  # ── M309 C1 A/B：每路的 GATE_JOBS / 绑核（空 = 保持既有行为）──
  JOBS_ENV=""; [ -n "$SHARD_JOBS" ] && JOBS_ENV="GATE_JOBS=$SHARD_JOBS "
  TSET=""
  if [ -n "$SHARD_CPUS" ]; then
    _cpus=$(printf '%s' "$SHARD_CPUS" | cut -d, -f"$i")
    [ -n "$_cpus" ] && TSET="taskset -c $_cpus "
  fi
  # ⚠️ shard1 = **唯一带宿主网络**的分片（跑需要出网的门，见 3b）。
  #    其余分片 `--net` 隔离 ⇒ 彼此端口互不可见 —— 这才是分片之间能并行的前提。
  NS="--mount --pid --net --fork --mount-proc"
  [ "$i" = 1 ] && NS="--mount --pid --fork --mount-proc"
  (
    unshare $NS bash -c "
      mount --bind $BASE/tmp$i /tmp
      ip link set lo up 2>/dev/null
      cd $BASE/wt$i || exit 9
      GATE_TSV=/tmp/times.tsv GATE_TIMEOUT_DEFAULT=900 \
      GATE_LOG_DIR_HINT=$BASE/tmp$i \
      ${JOBS_ENV}${TSET}bash selfhost/run_gates.sh --only '$ONLY'
    " > "$LOG/shard$i.log" 2>&1
    echo "$?" > "$LOG/shard$i.rc"
  ) &
  PIDS+=($!)
  if [ "$i" = 1 ]; then NSN="· 宿主网络"; else NSN="· net 隔离"; fi
  echo "  shard$i 启动 pid=${PIDS[-1]}（$(grep -c . "$LOG/shard$i.txt") 门 · $NSN）"
done

# ── 6) 等待 ──
echo "── 等待 ${#PIDS[@]} 个分片（0 门的片不启动${SKIPPED:+，跳过 $SKIPPED 片}）──"
for p in "${PIDS[@]}"; do wait "$p"; done
echo "── 全部分片结束 ──"

# ── 6b) 低负载门：此时机器已空闲 ⇒ 串行跑（见 quiet_needed.txt）──
if [ -n "$QUIET_LIST" ]; then
  echo "── 低负载门串行跑（$QUIET_LIST）──"
  # 用分片 worktree（与分片同一份代码）；先恢复被负控改过的受管目录，避免 run_gates 拒脏树
  git -C "$BASE/wt1" checkout -q -- selfhost runtime tools 2>/dev/null || true
  ( cd "$BASE/wt1" && GATE_TSV="$LOG/quiet.times.tsv" GATE_TIMEOUT_DEFAULT=900 \
      bash selfhost/run_gates.sh --only "$QUIET_LIST" ) > "$LOG/quiet.log" 2>&1
  echo "  quiet rc=$?"
fi

# ── 7) 汇总 ──
echo ""
echo "═══ 分片汇总 ═══"
TOT_OK=0; TOT_FAIL=0; TOT_SKIP=0
for i in $(seq 1 "$N"); do
  # ⚠️ 红必须数**两种**：`❌`（判红）**与** `⏱`（超时）—— `run_gates.sh` 的超时不打 ❌
  #    但同样 FAIL++（M284 缺陷 495 同源；2026-10-12 M305 首轮实测：shard1 自报「绿 45 红 0」
  #    而裁决器报 rc=124 ⇒ 超时被算成了绿）。
  ok=$(grep -cE '^✅ [a-zA-Z0-9_]+（' "$LOG/shard$i.log" 2>/dev/null)
  bad=$(grep -cE '^(❌|⏱)[[:space:]]+[a-zA-Z0-9_]+（' "$LOG/shard$i.log" 2>/dev/null)
  rc=$(cat "$LOG/shard$i.rc" 2>/dev/null || echo '?')
  echo "  shard$i: 绿 $ok · 红 $bad · rc=$rc"
  TOT_OK=$((TOT_OK+ok)); TOT_FAIL=$((TOT_FAIL+bad))
  cp "$BASE/tmp$i/times.tsv" "$LOG/shard$i.times.tsv" 2>/dev/null || true
done
if [ -n "$QUIET_LIST" ]; then
  qok=$(grep -cE '^✅ [a-zA-Z0-9_]+（' "$LOG/quiet.log" 2>/dev/null)
  qbad=$(grep -cE '^(❌|⏱)[[:space:]]+[a-zA-Z0-9_]+（' "$LOG/quiet.log" 2>/dev/null)
  echo "  quiet : 绿 $qok · 红 $qbad（低负载档）"
  TOT_OK=$((TOT_OK+qok)); TOT_FAIL=$((TOT_FAIL+qbad))
fi
echo "  合计：绿 $TOT_OK · 红 $TOT_FAIL（门清单 $NG）"
# ── 7b) 「空跑必红」：分片一个门都没跑出来 ⇒ 整轮**没有意义**，必须响亮 ──
#   实测（2026-10-12 11:38）：分片全部因残留锁被拒（rc=1 / 0 门），而汇总只打
#   「合计：绿 3」（那是 quiet 档那 3 个）—— 不细看会以为"绿了"。判据：分片侧合计必须 > 0。
SH_OK=0
for i in $(seq 1 "$N"); do
  # ── M352（缺陷 616）：**不能写 `$(grep -c … || echo 0)`** ──
  #   `grep -c` 无匹配时**自身已输出 0 且 rc=1** ⇒ `|| echo 0` 再补一个 ⇒ 子命令两行输出
  #   ⇒ 算术式变成 `SH_OK + 0\n0` ⇒ `syntax error in expression (error token is "0")`。
  #   实测（2026-10-28 V5 负控）：shard2 有 0 绿 1 红（m267 判红）⇒ 立刻命中；
  #   而这条算术之后紧跟的就是「空跑必红」裁决（SH_OK -eq 0 ⇒ exit 6）
  #   ⇒ 判据通道在出错态下继续求值 = 590 族「通道没了照样往下走」。
  #   触发条件平凡：**任何一片全红**（真灾难）就会命中。
  _n=$(grep -cE '^✅ [a-zA-Z0-9_]+（' "$LOG/shard$i.log" 2>/dev/null) || _n=0
  case "$_n" in ''|*[!0-9]*) _n=0 ;; esac
  SH_OK=$((SH_OK + _n))
done
if [ "$SH_OK" -eq 0 ]; then
  echo "  ❌ 空跑：$N 个分片一个门都没跑出来（看 $LOG/shard1.log —— 多半是残留锁/建树失败）"
  exit 6
fi
# ── M355（缺陷 623）：有红 ⇒ **退出码必须非 0** ──
#   旧末尾只打一行「❌ 有红」，进程 rc 恒 0 ⇒ 调用方 `rc=$?` 失真。
#   实测（2026-10-29 12:00 第二遍全量门）：红 2、`shard_rc=0`（日志 /data/gatevm/chain-m354-20261029-103355-2699083/shard_rc 内容为 0），
#   全靠链上 release_guard 解析「合计：绿 x · 红 y」那一行才兜住 —— 这是「判据第二通道」，
#   任何直接调 shard_run 的人（脚本、人、将来的 CI）都会把红读成绿。
#   专用码约定：0=全绿 · 1=有红（本行）· 6=空跑（上方）· 其它=自身故障。
if [ "$TOT_FAIL" != 0 ]; then
  echo "  ❌ 有红 ⇒ 逐条看 $LOG/shard*.log"
  echo "日志目录：$LOG"
  exit 1
fi
echo "日志目录：$LOG"
exit 0
