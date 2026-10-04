# GC 压力档台账 —— 「哪些语料跑过压力档 · 结论是什么 · 何时跑的」

> 建立于 **M260**（第 138 轮）。本文件说明**为什么需要它**、**它如何被维护**、以及
> **它的判据在哪**。

## 一 为什么需要（三笔欠账的汇合点）

1. **M207** 建了 `selfhost/gcstress_sweep.sh` —— 判据扎实（正常档跑两遍 + 压力档差分 +
   2+2 确认步 + `KNOWN`/`SLOW` 两张表 + 运行副作用清理），但 **425 个候选在压力档下是
   O(n²)**（每次分配都可能触发全 STW GC）⇒ **单轮数小时，从来没有完整跑过第二轮**。
2. **M212** 又加了 `--slow` 表（「慢但正确」的语料给足时间继续判），
   而 **「覆盖率」这件事始终没有账**：没人知道哪些语料跑过压力档、结论是什么、什么时候跑的。
3. **M257** 留下一条 ~1/60 的 Heisenbug（容器存储被 GC 回收后复用 ⇒ 静默堆损坏）与两句
   诚实的话：**根因未定论** · **加任何探测器/守卫后 60~240 次不复现**。
   ⇒ 缺的不是又一次排查，而是**可重复测量的装置**。

**M260 的答案**：把「跑」做成**分批 + 入账**，把「测」做成**装置 + 对照**。

## 二 台账（`selfhost/gcstress_ledger.tsv`）

格式：`路径 <TAB> 结论 <TAB> 日期 <TAB> 详情`

结论取值：

| 结论 | 含义 |
|---|---|
| `PASS` | 正常档两遍一致 **且** 压力档与正常档逐字节一致、rc 一致、无检测器标记 |
| `FAIL_OUT` / `FAIL_RC` / `FAIL_SIG` / `FAIL_DIAG` | **真信号**（输出不一致 / rc 不一致 / 被信号杀死 / 检测器响亮） |
| `STIMEOUT` | 正常档通过、压力档超时（O(n²) ⇒ 需人工定性，见 §四） |
| `NONDET` | 正常档自身两遍不一致（程序输出不确定 ⇒ 排除在判定之外，但**要定性**） |
| `SKIP_NORM` | 正常档就不通（负例 / 常驻服务 / 驱动器 / 探针 / **或真缺陷** —— 见 §五） |
| `SKIP_KNOWN` | 在 `KNOWN.tsv` 里（人工实跑定性过的「输出本身不确定」类） |
| `BUILDFAIL` | 构建失败 |

**纪律**：
* 本表**只增不减**（条目数只能增，路径不删）—— 由 `selfhost/gcstress_progress.txt` 的基线守；
* `FAIL_*` / `STIMEOUT` / `NONDET` / `BUILDFAIL` 的详情**必须带定性**
  （`缺陷NNN` 或 `假阳：<理由>`）—— 「跑出来红了但没人管」等于没跑。

## 三 工具

| 工具 | 作用 |
|---|---|
| `selfhost/gcstress_sweep.sh` | 差分筛本体（M207 建 · M212 加 `--slow`） |
| `selfhost/merge_ledger.py` | 批次结果**合并**进台账（同路径以新结果覆盖；保留「修复缺陷 NNN 后」的历史） |
| `selfhost/check_gcstress_ledger.py` | 台账判据（6 条，见下） |
| `selfhost/gcstress_progress.txt` | 「只增不减」的基线快照 |
| `selfhost/gcstress_skipnorm.tsv` | `SKIP_NORM` 的逐条定性（§五） |
| `examples/m207_gcstress/{KNOWN,SLOW}.tsv` | 两类例外的登记表 |

分批跑法（**每批可独立完成，跨轮次也行**——这正是「常态化」的意思）：

```bash
selfhost/gcstress_sweep.sh --batch K/N \
  --known examples/m207_gcstress/KNOWN.tsv \
  --slow  examples/m207_gcstress/SLOW.tsv \
  --out /tmp/batch_K.tsv --quiet
python3 selfhost/merge_ledger.py --ledger selfhost/gcstress_ledger.tsv \
  --in /tmp/batch_K.tsv --date "$(date +%F)"
```

## 四 判据（`check_gcstress_ledger.py` 第 ①–⑥ 条）

1. 格式与合法性：4 列 · 结论合法 · **路径必须存在** · 日期 `YYYY-MM-DD`；
2. `FAIL_*` / `STIMEOUT` / `NONDET` / `BUILDFAIL` **必须带定性**
   （`BUILDFAIL` 的**负例语料**除外 —— 命名即契约，见第 ⑥ 条同款模式）；
3. **只增不减**：条目数 ≥ 基线，且基线里的路径不得消失；
4. 规模锚点；
5. **未覆盖清单可见**（不判红 —— 判红靠第 ③ 条）；
6. **`SKIP_NORM` 必须被定性** —— 见 §五。

## 五 `SKIP_NORM` 的两种含义（M260 的实测教训）

`gcstress_sweep.sh` 对「正常档就不通」的语料记 `SKIP_NORM` 并跳过压力档。
这在多数情况下是对的，**但 `examples/m128_unlock_grow/unlock_grow.px` 的 rc=1 不是设计，
而是缺陷 439**（持 `g_intstr_mu` 做可失败分配 ⇒ 分配失败 ⇒ 隔离审计发现持锁 ⇒ `_exit(1)`）。

⇒ **「正常档不通」有两种：设计如此，与坏掉了。台账必须能区分。**

判据：
* 路径匹配 `SKIP_AUTO_RE`（命名即契约：`err_*` / `e1_*` / `error` / `bad` / `neg_*` /
  `daemon` / `server` / `srv` / `serve` / `_diag` / `_strict` / `_bounds` / `_uaf` /
  `_probe` / `_mutate` / `cases.px` / `driver` / `tiny` / `mini.px`）⇒ 自动合法；
* 其余**必须**在 `selfhost/gcstress_skipnorm.tsv` 里被逐条定性（类别 `合法` 或 `欠账`）。
  `欠账` 类的条目会以「⚠️ 已登记欠账」打印，且**数量只减不增**。

## 六 M260 的实测结果（首次完整一轮）

| 结论 | 条数 |
|---|---|
| PASS | 215 |
| SKIP_NORM | 176 |
| STIMEOUT | 12 |
| NONDET | 11 |
| BUILDFAIL | 7 |
| SKIP_KNOWN | 2 |
| **FAIL_\*** | **0** |

对照 **M207**（2026-09-25）的基线：`PASS 184 · SKIP_NORM 162 · STIMEOUT 12 · NONDET 10 ·
BUILDFAIL 7 · SKIP_KNOWN 1 · **FAIL 5**`。

⇒ **FAIL 5 → 0**。逐条追溯：

| M207 的 FAIL | M260 结论 | 追溯 |
|---|---|---|
| `m117_realworld_defects` | PASS（进 `KNOWN.tsv`） | 假阳：打印**实测用时**（701/702ms 相对 700ms 阈值） |
| `m89_s3d/vm_spawn_smoke` | PASS（进 `KNOWN.tsv`） | 假阳：并发**到达顺序**不确定 |
| `m173_http_proxy/vhost_gzip` | **PASS** | **缺陷 265 已修**（M211：`px_conn_read` 的 EINTR 重试） |
| `m93_s3/coro_gc_block` | **PASS** | 缺陷 267 家族 —— 见 §七 |
| `m88_s3/s1b_gc_stress` | **PASS**（`--slow` 400s） | 同上（实测 O(n²) 慢，输出正确） |
| `m96_s3/m96_gc` | **PASS** | 同上 |

## 七 缺陷 267 家族 / M257 Heisenbug：**装置 + 对照，而不是结论**

**装置**（`examples/m260_gc_repro/`）：同一探针 × N 次 + 三条判据
（被信号杀死 / 输出缺 `DONE` / **`[M257-CTR]` 守卫命中**），并支持 `M260_WT=<另一棵树>` 做对照。

实测：

| 组 | 源码 | 次数 | 结果 |
|---|---|---|---|
| A | 当前 HEAD · `m93_s3/coro_gc_block` | 20 | **20/20 通过** |
| B | 当前 HEAD · `m256_eintr/probe_eintr`（M257 的原现场，记录 ~1/60） | 60 | **60/60 通过**，`[M257-CTR]` 命中 **0** |
| C | **M256 的源码树**（`git worktree` 到 `957dd64`）· 同一探针 | 60 | **60/60 通过** |

⇒ **诚实的结论**：
* 当前 HEAD **0 复现**；
* **但对照组（M256 树）同样 0 复现** ⇒ **不能把「消失」归因于 M258/M259 的修复**；
* 更可能是 M257 记录的那个 Heisenbug 本身**依赖当时的环境/时序条件**
  （M257 自己也记录「加任何探测器/守卫后 60~240 次不复现」）；
* ⇒ **该缺陷的复现条件仍未掌握**。本轮的贡献是：**装置在手**
  （下次任何一次复现，都能立刻被三条判据抓住并留下日志），而不是「已修」。
