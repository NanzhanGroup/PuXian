# 性能基线 v2 · 原始数据采集与重复性自校验

- 生成时间：2026-10-13T08:50:39Z
- 口径：`docs/PERF_BASELINE_METRICS_SPEC.md`（sha256 `5d39d21c6a911a52`）
- 采集次数：3（rep1 / rep2 / rep3）
- 环境：iZj6ceod4xxg97ctkj0fb0Z · Neoverse-N2 · 8 核 · px 0.2.281 · gcc 11.5.0 · Rocky Linux 9.8 · 钉核 `taskset -c 3`
- 环境安静（无并发门）：**True**；并发门进程数：['0', '0', '0']

> 采集窗口无并发门进程（quiet=true）。

## 1 重复性自校验（判据：多次间最大相对偏差 ≤ 容差）

**总体：✅ 通过**　最大偏差 = 2.45%

| 指标 | kind | rep1 min(s) | rep2 min(s) | rep3 min(s) | 最大偏差 | 容差 | 判定 |
|---|---|---|---|---|---|---|---|
| fib28_vm | run | 0.8989 | 0.8982 | 0.8995 | 0.14% | 10% | ✅ |
| fib28_c | run | 0.2149 | 0.2144 | 0.2145 | 0.23% | 10% | ✅ |
| while_sum_vm | run | 0.2655 | 0.2659 | 0.2646 | 0.49% | 10% | ✅ |
| while_sum_c | run | 0.0385 | 0.0381 | 0.0383 | 1.05% | 10% | ✅ |
| mixed_vm | run | 0.3505 | 0.3497 | 0.3503 | 0.23% | 10% | ✅ |
| mixed_c | run | 0.3424 | 0.3422 | 0.3429 | 0.20% | 10% | ✅ |
| jsonwb_vm | run | 1.5774 | 1.5850 | 1.6015 | 1.53% | 10% | ✅ |
| jsonwb_c | run | 1.6292 | 1.5903 | 1.6185 | 2.45% | 10% | ✅ |
| startup_exec | startup | 0.0016 | 0.0016 | 0.0016 | 2.28% | 10% | ✅ |
| hot_build | build | 0.3763 | 0.3741 | 0.3730 | 0.88% | 15% | ✅ |

## 2 归并候选基线（adopted = 各次采集最小值）

| name | kind | unit | adopted(s) | 说明 |
|---|---|---|---|---|
| fib28_vm | run | s | 0.8982 | 纯递归 fib(28)x5 —— 纯计算最坏情形（VM 轨） |
| fib28_c | run | s | 0.2144 | 纯递归 fib(28)x5 —— C 轨（比值锚） |
| while_sum_vm | run | s | 0.2646 | 纯算术/比较/JMP 3M —— 专盯每指令固定开销漂移（VM 轨） |
| while_sum_c | run | s | 0.0381 | 纯算术 3M —— C 轨（比值锚） |
| mixed_vm | run | s | 0.3497 | dict+list+str 20 万次 —— 生产形态 IO/字典型（VM 轨） |
| mixed_c | run | s | 0.3422 | dict+list+str 20 万次 —— C 轨（比值锚） |
| jsonwb_vm | run | s | 1.5774 | JSON 2 万对象 构→串→解 —— 生产形态（VM 轨） |
| jsonwb_c | run | s | 1.5903 | JSON 2 万对象 —— C 轨（比值锚） |
| startup_exec | startup | s | 0.001581 | 空程序连跑 200 次 / 200（进程启动开销） |
| hot_build | build | s | 0.373 | px build 热缓存（空程序，含自动裁剪） |

> 候选件，未覆盖 `baseline.tsv`；人工核对后由 `verify.sh --update` 或人工固化。
