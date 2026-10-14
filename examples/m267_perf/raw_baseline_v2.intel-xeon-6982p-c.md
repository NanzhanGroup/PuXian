# 性能基线 v2 · 原始数据采集与重复性自校验

- 生成时间：2026-10-13T09:55:13Z
- 口径：`docs/PERF_BASELINE_METRICS_SPEC.md`（sha256 `5d39d21c6a911a52`）
- 采集次数：3（rep1 / rep2 / rep3）
- 环境：iZj6c9bkxssh8zjro5c3j2Z · Intel(R) Xeon(R) 6982P-C · 8 核 · px 0.2.306 · gcc 11.5.0 · Rocky Linux 9.8 · 钉核 `taskset -c 3`
- 环境安静（无并发门）：**True**；并发门进程数：['0', '0', '0']

> 采集窗口无并发门进程（quiet=true）。

## 1 重复性自校验（判据：多次间最大相对偏差 ≤ 容差）

**总体：✅ 通过**　最大偏差 = 1.50%

| 指标 | kind | rep1 min(s) | rep2 min(s) | rep3 min(s) | 最大偏差 | 容差 | 判定 |
|---|---|---|---|---|---|---|---|
| fib28_vm | run | 0.5172 | 0.5176 | 0.5183 | 0.21% | 10% | ✅ |
| fib28_c | run | 0.1338 | 0.1334 | 0.1339 | 0.37% | 10% | ✅ |
| while_sum_vm | run | 0.1692 | 0.1694 | 0.1698 | 0.35% | 10% | ✅ |
| while_sum_c | run | 0.0469 | 0.0468 | 0.0467 | 0.43% | 10% | ✅ |
| mixed_vm | run | 0.2645 | 0.2647 | 0.2653 | 0.30% | 10% | ✅ |
| mixed_c | run | 0.2675 | 0.2694 | 0.2681 | 0.71% | 10% | ✅ |
| jsonwb_vm | run | 0.5505 | 0.5488 | 0.5489 | 0.31% | 10% | ✅ |
| jsonwb_c | run | 0.5622 | 0.5623 | 0.5610 | 0.23% | 10% | ✅ |
| startup_exec | startup | 0.0009 | 0.0009 | 0.0009 | 1.50% | 10% | ✅ |
| hot_build | build | 0.2590 | 0.2608 | 0.2583 | 0.97% | 15% | ✅ |

## 2 归并候选基线（adopted = 各次采集最小值）

| name | kind | unit | adopted(s) | 说明 |
|---|---|---|---|---|
| fib28_vm | run | s | 0.5172 | 纯递归 fib(28)x5 —— 纯计算最坏情形（VM 轨） |
| fib28_c | run | s | 0.1334 | 纯递归 fib(28)x5 —— C 轨（比值锚） |
| while_sum_vm | run | s | 0.1692 | 纯算术/比较/JMP 3M —— 专盯每指令固定开销漂移（VM 轨） |
| while_sum_c | run | s | 0.0467 | 纯算术 3M —— C 轨（比值锚） |
| mixed_vm | run | s | 0.2645 | dict+list+str 20 万次 —— 生产形态 IO/字典型（VM 轨） |
| mixed_c | run | s | 0.2675 | dict+list+str 20 万次 —— C 轨（比值锚） |
| jsonwb_vm | run | s | 0.5488 | JSON 2 万对象 构→串→解 —— 生产形态（VM 轨） |
| jsonwb_c | run | s | 0.561 | JSON 2 万对象 —— C 轨（比值锚） |
| startup_exec | startup | s | 0.000869 | 空程序连跑 200 次 / 200（进程启动开销） |
| hot_build | build | s | 0.2583 | px build 热缓存（空程序，含自动裁剪） |

> 候选件，未覆盖 `baseline.tsv`；人工核对后由 `verify.sh --update` 或人工固化。
