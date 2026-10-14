# 性能基线 v2 · 原始数据采集与重复性自校验

- 生成时间：2026-10-13T09:55:08Z
- 口径：`docs/PERF_BASELINE_METRICS_SPEC.md`（sha256 `5d39d21c6a911a52`）
- 采集次数：3（rep1 / rep2 / rep3）
- 环境：iZj6chsaojnjk8brz2q8cfZ · Intel(R) Xeon(R) Gold 6462C · 8 核 · px 0.2.306 · gcc 11.5.0 · Rocky Linux 9.8 · 钉核 `taskset -c 3`
- 环境安静（无并发门）：**True**；并发门进程数：['0', '0', '0']

> 采集窗口无并发门进程（quiet=true）。

## 1 重复性自校验（判据：多次间最大相对偏差 ≤ 容差）

**总体：✅ 通过**　最大偏差 = 2.19%

| 指标 | kind | rep1 min(s) | rep2 min(s) | rep3 min(s) | 最大偏差 | 容差 | 判定 |
|---|---|---|---|---|---|---|---|
| fib28_vm | run | 0.4851 | 0.4839 | 0.4849 | 0.25% | 10% | ✅ |
| fib28_c | run | 0.1319 | 0.1319 | 0.1319 | 0.00% | 10% | ✅ |
| while_sum_vm | run | 0.1552 | 0.1552 | 0.1553 | 0.06% | 10% | ✅ |
| while_sum_c | run | 0.0449 | 0.0456 | 0.0458 | 2.00% | 10% | ✅ |
| mixed_vm | run | 0.2595 | 0.2591 | 0.2596 | 0.19% | 10% | ✅ |
| mixed_c | run | 0.2634 | 0.2636 | 0.2636 | 0.08% | 10% | ✅ |
| jsonwb_vm | run | 0.5732 | 0.5821 | 0.5829 | 1.69% | 10% | ✅ |
| jsonwb_c | run | 0.5894 | 0.5918 | 0.5930 | 0.61% | 10% | ✅ |
| startup_exec | startup | 0.0008 | 0.0008 | 0.0008 | 2.19% | 10% | ✅ |
| hot_build | build | 0.2472 | 0.2457 | 0.2446 | 1.06% | 15% | ✅ |

## 2 归并候选基线（adopted = 各次采集最小值）

| name | kind | unit | adopted(s) | 说明 |
|---|---|---|---|---|
| fib28_vm | run | s | 0.4839 | 纯递归 fib(28)x5 —— 纯计算最坏情形（VM 轨） |
| fib28_c | run | s | 0.1319 | 纯递归 fib(28)x5 —— C 轨（比值锚） |
| while_sum_vm | run | s | 0.1552 | 纯算术/比较/JMP 3M —— 专盯每指令固定开销漂移（VM 轨） |
| while_sum_c | run | s | 0.0449 | 纯算术 3M —— C 轨（比值锚） |
| mixed_vm | run | s | 0.2591 | dict+list+str 20 万次 —— 生产形态 IO/字典型（VM 轨） |
| mixed_c | run | s | 0.2634 | dict+list+str 20 万次 —— C 轨（比值锚） |
| jsonwb_vm | run | s | 0.5732 | JSON 2 万对象 构→串→解 —— 生产形态（VM 轨） |
| jsonwb_c | run | s | 0.5894 | JSON 2 万对象 —— C 轨（比值锚） |
| startup_exec | startup | s | 0.000775 | 空程序连跑 200 次 / 200（进程启动开销） |
| hot_build | build | s | 0.2446 | px build 热缓存（空程序，含自动裁剪） |

> 候选件，未覆盖 `baseline.tsv`；人工核对后由 `verify.sh --update` 或人工固化。
