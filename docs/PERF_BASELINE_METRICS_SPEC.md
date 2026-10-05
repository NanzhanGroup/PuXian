# PERF_BASELINE_METRICS_SPEC · 性能基线 v2 指标口径（可直接照做）

> 状态：✅ 2026-10-05 建立（任务 #166/t1「基线口径梳理」）· 性能轴定调 **「守」**
> 作用：给「性能基线 v2 重定基 + 性能回归门」一把**唯一口径尺**——凡采集/固化/判红，
> 一律按本文件的**基准场景 / 采样轮次 / 统计量 / 运行环境**执行；任何与本文件不符的跑法，其数字**不得入基线**。
> 上游依据：清歌《PuXian 下一步发展方案 v1》§2.5 + P0-1、清歌《性能基线 v2》、晨曦《px run 解释轨性能》。
> 落地现状（本文件对齐的既有资产）：`docs/PERF_BASELINE_V2.md`（v2 全谱与来历）、
> `examples/m267_perf/verify.sh`（性能回归门）、`examples/m267_perf/baseline.tsv`（门用基线表）、
> `examples/m89_perf/bench_vm_vs_c.sh`（M89 旧生成器，M266 已修复）。
> **自校验**：本文件「§10 参数一致性自校验」由 `examples/m267_perf/check_spec_consistency.sh` **机器核对**；
> 若该脚本不通过（`SPEC-CONSISTENCY-OK` 未出现），本文件即视为**未生效**。

---

## 0 一句话

**性能数字只对"同机 + 同工具链 + 同负载 + 同口径"有意义**；基线的判据是**相对基线的 ×倍**，
不是绝对秒。所以本口径把"怎么测"钉死成可复制的固定动作：`taskset -c 3` 钉核 · 预热 · **多轮取 min** ·
双轨产物**身份自检** · 双轨 stdout **逐字节一致**作正确性锚点。

---

## 1 术语与对拍实体

| 术语 | 定义 |
|---|---|
| **VM 轨** | `px build <src>`（**M91 起默认**；`--vm`/`--bc` 为同义冗余 flag）。产物 = BCModule 字节码镜像 C + `vm.o`，跑在显式帧 VM 上。 |
| **C 轨** | `px build --c <src>`（逃生舱；旧 `fn_*` C 文本轨）。产物 = 机器码函数 `fn_<name>` + runtime。 |
| **对拍实体** | **同一份 `.px` 源码**的两种用户产物（VM 轨 / C 轨），源码逐字节相同。 |
| **正确性锚点** | 同一负载在 VM/C 两轨下 **stdout 逐字节一致**；不一致 ⇒ 该轮测量作废（不是性能问题，是语义问题）。 |

### 1.1 产物身份自检（**强制**，防 M266 那类「两条命令同一轨」的静默腐烂）

```bash
strings -a <bin> | grep -c '^fn_'      # C 轨 ≥ 1；VM 轨 == 0
```
- C 轨期望 `fn_*` 符号数 **≥ 1**；VM 轨期望 **== 0**（`s_G[]` 名表 + `PxBCModule`/`s_mod` 是 VM 轨特征）。
- **每一轮重定基、每一次门运行都必须做**；不做这一步的对比结论**一律无效**（M266 教训）。

---

## 2 基准场景（workload 清单）

负载源码为**最小固定形状**，输入规模写死在源码里，保证跨轮/跨机可比。
两层清单：**Tier-1 = 门内必跑（基线表 = 单一事实源）**；**Tier-2 = 全谱（重定基文档用，可选）**。

### 2.1 Tier-1 · 门内必跑（`examples/m267_perf/`，4 负载 × 双轨 + 启动 + 热构建）

| # | 负载 | 形状 | kind | 盯的是什么 |
|---|---|---|---|---|
| 1 | `fib28`（fib(28)×5） | 纯递归（CPU 密集最坏情形） | run | 纯计算热点；C 轨收益最大的场景 |
| 2 | `while_sum`（3M） | 纯算术/比较/JMP | run | **每指令/每次分配都要付的固定开销漂移**（M240–M259 那批根面/锁改动的形态） |
| 3 | `mixed`（dict+list+str 20 万次） | 容器（生产形态） | run | IO/字典型负载，与现役 .px 组件同型 |
| 4 | `jsonwb`（JSON 2 万对象 构→串→解） | JSON（生产形态） | run | 序列化/反序列化路径 |
| — | `startup_exec`（空程序连跑 200 次） | 进程启动 | startup | 解释/链接/初始化固定成本 |
| — | `hot_build`（空程序，热缓存） | 构建 | build | 迭代手感；链路/裁剪回归 |

> 源码：`examples/m267_perf/src/{fib28,while_sum,mixed,jsonwb}.px`；基线表：`examples/m267_perf/baseline.tsv`。
> **每条 run 负载必须 VM / C 成对登记**（缺一即门自检判红）。

### 2.2 Tier-2 · 全谱（`docs/PERF_BASELINE_V2.md` §五，8 负载 × 双轨）

在 Tier-1 之外**追加**：`fib26`（纯递归对照）、`loop_sum`（`range` 物化迭代）、
`textscan`（3 万行 split+contains+regex）、`stridx64k`（65 540 字符逐字符索引，M106 场景）。
Tier-2 用于重定基文档的「VM/C 全谱」，**不要求每次门运行**（门成本约束，见 §6.3）。

### 2.3 M89 三类负载的定位（**面外**，如实登记）

清歌 P0-1 提议的 M89 三类负载（fib / compiler 形态 / HTTP JSON）：
- `fib_calc.px`（md5 `4085d090734ff84bc0f08ab9773fd3aa`）已由 `bench_vm_vs_c.sh` 覆盖，属**旧生成器面**；
- **compiler 形态**依赖 `bootstrap_prove_bc.sh` 产物、**HTTP JSON** 属多核/IO 面 —— **本轮均不入门**（见 §7 覆盖边界）。

---

## 3 采样轮次与统计量（口径核心）

### 3.1 run 类负载

| 项 | 值 |
|---|---|
| 钉核 | `taskset -c 3`（有则必用；无则该轮数字降级、不得入基线） |
| 预热 | **每负载先跑 1 次**（丢弃） |
| 采样轮次 | **≥ 5 轮**（默认 `REPS=5`） |
| **统计量** | **min（最小值）** —— 抗偶发干扰最稳；不取 mean/median |
| 记录 | 每轮原始值一并留档（`min` 之外），便于复算 |

> **为什么取 min 不取中位**：本仓计时常在**全量门并行**环境下进行，干扰只会上抬耗时；
> min 是"无干扰下界"的稳健估计（M266 §9.3 实测含干扰上界 ~5%）。中位/均值会把干扰算进基线。

### 3.2 startup 类（`startup_exec`）

| 项 | 值 |
|---|---|
| 做法 | 空程序**连跑 200 次**，测总墙钟 |
| 统计量 | **总时间 / 200**（单样本均值，单位 s/次）—— 不取 min |
| 落表 | 单位 s，保留 6 位小数（如 `0.001500`） |

### 3.3 build 类（`hot_build`）

| 项 | 值 |
|---|---|
| 预热 | **2 次**热构建（丢弃） |
| 做法 | 计时 **1 次**热构建（`.rtcache` 命中） |
| 统计量 | 单次墙钟（单位 s）；**冷构建不入本口径**（见 §7） |

---

## 4 运行环境与硬件约束

**基线只对下述环境成立；换 CPU/核数/内存/OS/gcc/px 版本 ⇒ 必须 `--update` 重定基。**

| 项 | 基线环境（dongyue） | 备注 |
|---|---|---|
| 主机 | `dongyue` | |
| CPU | **AMD EPYC 7K62** | 与清歌同型号 ⇒ 跨机趋势互证成立 |
| 核数 | **8**（`nproc`） | |
| 内存 | **15 GiB** | |
| OS | **Rocky Linux 9.8** | |
| gcc | **11.5.0** | `gcc -dumpversion` |
| px | **0.2.17** | **仓内 `tools/px`**（非 `/usr/share` 安装件） |
| 计时器 | `date +%s.%N`（门） / `/usr/bin/time -f "%e"`（文档全谱） | 二者可互证 |
| 干扰约束 | run 类**门内串行**；重定基尽量避开全量门并行 | |

> px 版本必须与 `tools/px --version` 一致并写进基线表头；px 每升版 ⇒ 重定基。

---

## 5 基线文件格式（单一事实源）

**文件**：`examples/m267_perf/baseline.tsv`（门直接读取，**基线清单的唯一来源**）。

**表头（`#` 注释，人可读 + 换机指引，缺则门自检判红）**：

```
# M267 性能回归基线 —— 由 verify.sh --update 生成
# 生成机：<host> · <CPU model> · <nproc> 核 · gcc <ver> · <OS>
# 工具链：px <ver>
# 格式：<name> <kind:run|startup|build> <unit> <baseline秒> <说明>
# ⚠️ 绝对值只对**同机同工具链**有意义；判据是**相对变化**。刷新前须人工确认。
# ⚠️ 换机器 / 换 gcc / 换 px 版本 ⇒ 必须 --update 重定基（否则比值无意义）
# 依据：docs/PERF_BASELINE_V2.md（M266 建立）
```

**数据行**：`<name>\t<kind>\t<unit>\t<base>\t<note>`，其中
- `name`：run 类必须以 `_vm` / `_c` 结尾并**成对**出现；startup/build 为单条；
- `kind` ∈ `run | startup | build`；
- `unit`：秒 `s`；
- `base`：按 §3 统计量得到的秒数；
- `note`：形状 + 轨 + 用途（人可读）。

**下限**：run 类成对 ≥ 4 组（门自检硬约束「基线表 ≥ 4 行」「每条 run 负载 vm/c 成对」）。

---

## 6 判定规则（相对基线 ×倍）

### 6.1 比值与阈值

| 项 | 值 |
|---|---|
| 判据 | `ratio = 实测(统计量) / 基线(同名)` |
| warn | `ratio > 1.25` ⇒ 出 ⚠️，**仍绿**（响亮提示，不拦） |
| fail | `ratio > 2.00` ⇒ **判红**（拦） |
| 豁免 | 显式豁免表（带理由 + 计数），**不允许静默跳过** |

### 6.2 两轮确认（抗偶发干扰）

首轮 `ratio > fail` ⇒ **复测一次取 min**；仍 > fail 才判红。warn 不做两轮（只提示）。

### 6.3 门成本预算

门内**串行**跑 Tier-1（4 负载 × 双轨 × min-of-5 + 启动 200 + 热构建），目标 **≤ 90 s**。
⇒ 这是 Tier-2 不入门的直接原因。

---

## 7 覆盖边界（如实登记 · 不覆盖的）

| 不在口径内 | 原因 |
|---|---|
| 多核 / IO / HTTP 服务型负载（M89 `http_json` 形态） | 属多核/IO 面，本口径只管**本机单核** |
| `compiler 形态`（`bootstrap_prove_bc.sh` 产物） | 产物重、成本高，未纳入 |
| **冷构建**（清 `.rtcache`） | 会打断同机其它门/构建的缓存，代价不可控 |
| `px run`（解释轨）面 | 由 `examples/m271_run_fast/`（M271）覆盖 |
| 字符串 `+=` O(n²) | 由 `docs/STR_CONCAT.md` + M270 覆盖 |
| 跨机绝对值相减 | **禁止**；跨机只做趋势互证 |

---

## 8 复现命令（照抄即可）

```bash
PX=/data/code/puxian/tools/px
W=/tmp/perf_spec; rm -rf "$W"; mkdir -p "$W"
# ① 产物身份自检（强制）
for n in fib28 while_sum mixed jsonwb; do
  mkdir -p "$W/vm_$n" "$W/c_$n"
  cp examples/m267_perf/src/$n.px "$W/vm_$n/"; cp examples/m267_perf/src/$n.px "$W/c_$n/"
  ( cd "$W/vm_$n" && "$PX" build     $n.px >/dev/null )   # VM 轨（默认）
  ( cd "$W/c_$n"  && "$PX" build --c $n.px >/dev/null )   # C 轨
  echo "$n VM fn_*=$(strings -a "$W/vm_$n/build/$n" | grep -c '^fn_') (期望0) \
        C fn_*=$(strings -a "$W/c_$n/build/$n"  | grep -c '^fn_') (期望≥1)"
done
# ② 正确性锚点（双轨 stdout 逐字节一致）+ 计时（min of 5）
bench() { local r=5; local -a a=(); for i in $(seq 1 $r); do
  local s e; s=$(date +%s.%N); taskset -c 3 "$@" >/dev/null 2>&1; e=$(date +%s.%N)
  a+=("$(echo "$e $s" | awk '{printf "%.4f",$1-$2}')"); done
  printf '%s\n' "${a[@]}" | sort -n | head -1; }
for n in fib28 while_sum mixed jsonwb; do
  ov=$(taskset -c 3 "$W/vm_$n/build/$n"); oc=$(taskset -c 3 "$W/c_$n/build/$n")
  [ "$ov" = "$oc" ] && echo "$n stdout一致 ✅($ov)" || echo "$n stdout不一致 ❌"
  echo "  $n vm(min5)=$(bench "$W/vm_$n/build/$n")  c(min5)=$(bench "$W/c_$n/build/$n")"
done
# ③ 固化 / 判红（门内一步到位）
bash examples/m267_perf/verify.sh           # 跑门（相对基线判红）
bash examples/m267_perf/verify.sh --update  # 重定基（人工确认后提交）
bash examples/m267_perf/verify.sh --neg     # 负控（4 道各自独立判红）
```

---

## 9 与既有资产的关系（谁抄谁）

| 资产 | 角色 | 与口径的关系 |
|---|---|---|
| **本文件** | **口径唯一尺** | — |
| `docs/PERF_BASELINE_V2.md` | v2 全谱与来历 | 数字层；口径服从本文件 |
| `examples/m267_perf/verify.sh` | 门实现 | **本口径的机器执行体** |
| `examples/m267_perf/baseline.tsv` | 门用基线表 | 本口径 §5 格式 |
| `examples/m89_perf/bench_vm_vs_c.sh` | 旧生成器 | M266 已修（`--vm`→`--c` + 身份自检）；遗留 `median`/无 `taskset` 见 §10.3 |

---

## 10 参数一致性自校验（本任务内机器核对）

由 `examples/m267_perf/check_spec_consistency.sh` 提取本文件的**机器可读参数块**，
与 `verify.sh` / `baseline.tsv` / 实机环境逐项比对；通过输出 `SPEC-CONSISTENCY-OK`。

### 10.1 声明参数（机器可读块）

```
# >>> SPEC-PARAMS >>>
pin=taskset -c 3
run_reps=5
run_stat=min
startup_runs=200
startup_stat=mean
build_warmups=2
build_stat=single
warn_ratio=1.25
fail_ratio=2.00
confirm_rounds=2
gate_loads=fib28 while_sum mixed jsonwb
full_loads=fib28 fib26 while_sum loop_sum mixed jsonwb textscan stridx64k
identity_cmd=strings -a BIN | grep -c '^fn_'
identity_c_ge=1
identity_vm_eq=0
baseline_file=examples/m267_perf/baseline.tsv
baseline_format=name kind:run|startup|build unit seconds note
gate_script=examples/m267_perf/verify.sh
gate_registry_line=run m267_perf bash examples/m267_perf/verify.sh
env_cpu=AMD EPYC 7K62
env_cores=8
env_mem_gib=15
env_os=Rocky Linux 9.8
env_gcc=11.5.0
env_px=0.2.17
# <<< SPEC-PARAMS <<<
```

### 10.2 核对结果（2026-10-05 · 本任务实跑）

| 声明项 | 本文件 | `verify.sh` 实现 | 实机/基线表 | 判定 |
|---|---|---|---|---|
| 钉核 | `taskset -c 3` | `PIN="taskset -c 3"`（`command -v taskset` 守卫） | `taskset` 存在 | ✅ |
| run 轮次 | 5 | `REPS="${M267_REPS:-5}"` | — | ✅ |
| run 统计量 | min | `time_run`: `… \| sort -n \| head -1` | baseline.tsv 采 min | ✅ |
| 启动次数 | 200 | `for _ in $(seq 200)`，`/200` | `startup_exec=0.001500` | ✅ |
| 热构建预热 | 2 | `for _ in 1 2; do …; done` 后计时 1 次 | `hot_build=0.540` | ✅ |
| warn | 1.25 | `WARN_R="${M267_WARN:-1.25}"` | — | ✅ |
| fail | 2.00 | `FAIL_R="${M267_FAIL:-2.00}"` | — | ✅ |
| 两轮确认 | 2 | 首轮超 fail ⇒ `measure_one` 复测取 min | — | ✅ |
| 门内负载 | 4 | `LOADS="fib28 while_sum mixed jsonwb"` | baseline.tsv 4×双轨+2 | ✅ |
| 身份自检 | C≥1 / VM=0 | `check_identity`: `grep -c '^fn_'` | — | ✅ |
| 基线格式 | §5 | `read_baseline` 按 5 列解析 | 表头 4 行 | ✅ |
| 门注册 | 1 行 | `selfhost/gates.registry.sh:1001` | — | ✅ |
| 环境 | §4 | 基线表头写 机/CPU/gcc/OS | 实机一致 | ✅ |

### 10.3 已发现的口径不一致（**如实登记**，由后续任务处置）

| # | 位置 | 现象 | 与口径冲突 | 处置建议 |
|---|---|---|---|---|
| I1 | `examples/m89_perf/bench_vm_vs_c.sh` `median3()` | fib 取**3 轮中位** | §3.1 要求 **min** | 旧生成器仅作历史/决策数据；**不得作为门基线**。若要复用，改 min。 |
| I2 | `examples/m89_perf/bench_vm_vs_c.sh` | 计时**未钉核**（无 `taskset`） | §3.1 要求钉核 | 同上；门内（m267）已钉核，不受影响。 |
| I3 | `examples/m89_perf/bench_vm_vs_c.sh` `median3` | fib 仅 **3 轮** | §3.1 要求 ≥5 | 同上。 |
| I4 | `docs/PERF_BASELINE_V2.md` §2.3 | 措辞「5–7 轮」（清歌 fib 5 / 其余 7） | §3.1 钉 **5**（门值） | 文档全谱可 >5；**门基线**统一 5。已在 §3.1 说明。 |
| I5 | `tools/pxbench.px` | 旧通用基准工具（`count`/`repeat`，默认 3 轮），非本口径 | 独立工具，不参与门 | 保留；不纳入本口径。 |

> 以上均为**登记**而非本轮修复项（本轮只产出口径与自校验）；I1–I3 属旧生成器面，
> 门内实现（`verify.sh`）与口径**完全一致**，故不影响 v2 门基线可信度。

---

## 11 自校验结论

- 本文件 §10.1 声明参数与 `examples/m267_perf/verify.sh`、`examples/m267_perf/baseline.tsv`、
  `selfhost/gates.registry.sh` 及实机环境**逐项一致**（§10.2 全绿）。
- 唯一不一致项集中在**旧生成器** `bench_vm_vs_c.sh`（§10.3 I1–I3），已显式登记隔离，**不入门基线**。
- ⇒ 本口径**可直接照做**：`t2` 采集 → `t3` 固化 `baseline.tsv` → `t4/t5` 按 §6 判定。
