# M383 · quiet 档重归档（门基础设施 2/3）

> 结论先行：quiet 串行尾（16 门 · 实测 2428s = 40.5min）不是「门太慢」，而是**排法太粗**。
> 本档记录「为什么分两档、凭什么这么分、怎么复核」。

## 1 问题（实测）

M382 第四轮（`/data/gatevm/shard-20261109-070339/quiet.times.tsv`）：

| 门 | 秒 |
|---|---|
| m244_gate_parallel | 514 |
| m302_stdlib_surface3 | 285 |
| m291_gcrotate | 228 |
| m208_vm_c_local_roots | 205 |
| m296_quic_accept_reuse | 190 |
| m231_op_matrix | 183 |
| m183_gc_root_handover | 179 |
| m267_perf | 161 |
| m227_two_faces | 122 |
| m246_compound_assign | 121 |
| m280_resource_baseline | 61 |
| m239_match_case | 60 |
| m324_tls_failclosed | 47 |
| m120 | 29 |
| m312_gc_lock_order | 25 |
| m295_quic_slot | 18 |

合计 2428s。分片段已经压到 49min，**quiet 尾 40.5min 成了新的墙钟瓶颈**。

## 2 为什么不能「慢就挪回分片」

这些门被摘出分片的理由**不是快**，而是「分片 16 路并发下判据假红」
（见 `/data/gatevm/quiet_needed.txt` 逐条登记：性能比越阈、时延窗口被挤、
GC 停顿噪声、端口窗口被抢、负控重编窗口被挤……）。
⇒ 判据不是「耗时 ≥180s 就移档」，而是**这门对「轻并发」敏不敏感**，且要有实测。

因此分两档：

| 档 | 前提 | 跑法 | 归属来源 |
|---|---|---|---|
| 并行档 `par` | 判据是**功能性**的，不吃绝对时延/性能比，且轻并发实测绿 | K 条车道（每路 2 核 · JOBS=2 · 私有 /tmp），并发跑 | `quiet_lanes.txt`（归档器产物） |
| 独占档 `solo` | 判据前提取决于「机器空闲」：性能比 / 时延窗口 / 端口窗口 / GC 停顿 / 重编窗口 | 串行，独占机器 | 其余 quiet 门 |

**独占是白名单，不是默认**：进 `quiet_exclusive.txt` 必须写清「判据前提是什么」，
不许用「我觉得敏感」当理由（CRIT_8：注释里的理由也要能被审计）。

## 3 判据

1. **归档漂移**：`packaging/chain/quiet_archive.py --check`
   —— 用上一轮实测重算档位，与 `quiet_lanes.txt` 不一致 ⇒ rc=3。
   负控（可复现）：把某门实测耗时人为改大/改小 ⇒ `--check` 报「该进/该回」并 rc=3。
   自证：`--selftest`（慢门进并行 / 快门留独占 / 独占白名单不被慢门顶掉 / 无实测保守留独占 / 两条负控 / 装箱均分）。
2. **墙钟**：改后 quiet 尾（独占合计 + 车道墙钟下界）< 25min **且全绿**。
3. **轻并发实测**：候选门在「3 车道 × 2 核 × JOBS=2 × 私有 /tmp」下全绿 ——
   探针 `/data/autodrive/m383_quiet_lanes_probe.sh`，产物 `/data/autodrive/logs/m383_quiet_lanes_probe-<ts>/`。

## 4 数据文件（宿主，非代码）

| 文件 | 角色 |
|---|---|
| `/data/gatevm/quiet_needed.txt` | 归档来源：哪些门**摘出分片**（不变） |
| `/data/gatevm/quiet_lanes.txt` | 归档产物：`<gate> <lane>`；不在此表 = 独占档 |
| `/data/gatevm/quiet_exclusive.txt` | 独占白名单：`<gate> <判据前提/理由>` |

三个文件都在 `$PX_GATEVM`（默认 `/data/gatevm`），与 `times*.tsv` 同族
（shard_run.sh 头部「宿主数据依赖（不是代码）」一节已登记）。
