# 性能基线 v2 · 原始数据采集与重复性自校验

- 生成时间：2026-10-05T05:32:50Z
- 口径：`docs/PERF_BASELINE_METRICS_SPEC.md`（sha256 `0dc65700dc7a7d79`）
- 采集次数：3（rep1 / rep2 / rep3）
- 环境：dongyue · AMD EPYC 7K62 · 8 核 · px 0.2.17 · gcc 11.5.0 · Rocky Linux 9.8 · 钉核 `taskset -c 3`
- 环境安静（无并发门）：**False**；并发门进程数：['3', '3', '4']

> 采集窗口存在并发全量门（run_gates.sh）：并发门进程数 ['3', '3', '4']，loadavg before=['1.52 1.48 1.21', '1.83 1.58 1.26', '1.58 1.59 1.29'] after=['1.83 1.58 1.26', '1.96 1.65 1.30', '1.82 1.65 1.33']。
> 抗干扰按口径 §3.1 执行（钉核 taskset -c 3 + 预热 + min-of-5）；本次 3 次重复自校验：钉核 run 类多次波动 ≤ 6.54%，未钉核 startup/build 波动 ≤ 11.00%，均在容差内。
> 如需完全静默窗口复核，可在 selfhost/run_gates.sh 结束后重跑：bash examples/m267_perf/collect_baseline_v2_driver.sh

## 1 重复性自校验（判据：多次间最大相对偏差 ≤ 容差）

**总体：✅ 通过**　最大偏差 = 11.00%

| 指标 | kind | rep1 min(s) | rep2 min(s) | rep3 min(s) | 最大偏差 | 容差 | 判定 |
|---|---|---|---|---|---|---|---|
| fib28_vm | run | 1.1056 | 1.0377 | 1.0508 | 6.54% | 10% | ✅ |
| fib28_c | run | 0.2616 | 0.2610 | 0.2628 | 0.69% | 10% | ✅ |
| while_sum_vm | run | 0.3332 | 0.3319 | 0.3329 | 0.39% | 10% | ✅ |
| while_sum_c | run | 0.0658 | 0.0670 | 0.0650 | 3.08% | 10% | ✅ |
| mixed_vm | run | 0.5035 | 0.4988 | 0.4957 | 1.57% | 10% | ✅ |
| mixed_c | run | 0.5227 | 0.5011 | 0.4984 | 4.88% | 10% | ✅ |
| jsonwb_vm | run | 1.4506 | 1.4235 | 1.4393 | 1.90% | 10% | ✅ |
| jsonwb_c | run | 1.4160 | 1.4414 | 1.4418 | 1.82% | 10% | ✅ |
| startup_exec | startup | 0.0015 | 0.0015 | 0.0015 | 5.74% | 10% | ✅ |
| hot_build | build | 0.4749 | 0.4646 | 0.5157 | 11.00% | 15% | ✅ |

## 2 归并候选基线（adopted = 各次采集最小值）

| name | kind | unit | adopted(s) | 说明 |
|---|---|---|---|---|
| fib28_vm | run | s | 1.0377 | 纯递归 fib(28)x5 —— 纯计算最坏情形（VM 轨） |
| fib28_c | run | s | 0.261 | 纯递归 fib(28)x5 —— C 轨（比值锚） |
| while_sum_vm | run | s | 0.3319 | 纯算术/比较/JMP 3M —— 专盯每指令固定开销漂移（VM 轨） |
| while_sum_c | run | s | 0.065 | 纯算术 3M —— C 轨（比值锚） |
| mixed_vm | run | s | 0.4957 | dict+list+str 20 万次 —— 生产形态 IO/字典型（VM 轨） |
| mixed_c | run | s | 0.4984 | dict+list+str 20 万次 —— C 轨（比值锚） |
| jsonwb_vm | run | s | 1.4235 | JSON 2 万对象 构→串→解 —— 生产形态（VM 轨） |
| jsonwb_c | run | s | 1.416 | JSON 2 万对象 —— C 轨（比值锚） |
| startup_exec | startup | s | 0.001463 | 空程序连跑 200 次 / 200（进程启动开销） |
| hot_build | build | s | 0.4646 | px build 热缓存（空程序，含自动裁剪） |

> 候选件，未覆盖 `baseline.tsv`；人工核对后由 `verify.sh --update` 或人工固化。
