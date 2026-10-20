# onnxruntime 的 MatMulInteger(u8s8) kernel 偏差 —— 事实、证据、以及它对我们判据的影响

> 状态：**OURS-OK**（PuXian 与规范一致）· **UPSTREAM-BUG**（ORT 1.19.2 偏差）
> 里程碑：M314（发现并钉死根因）· M315（判据方向修正 + 最小装置入仓）
> 相关门：`examples/m315_onnx_ort_kernel/`（最小装置 · 双向登记核对）、
>        `examples/m311_real_model/`（端到端 · 主判据改为「与可信真值比」）

## 一 一句话

M311 记录的「PuXian 与 onnxruntime 相差 max 0.477 / mean 0.066」，**全部来自
onnxruntime 的 `MatMulInteger`(u8s8) kernel**；PuXian 与规范一致（实测 max 5e-6，
float32 舍入级）。「缺陷 533」**不是我们的缺陷**，原判据拿了一个会出错的实现当真值。

## 二 决定性证据（端到端 · 可复算）

把生产 int8 bge 模型里 **72 个 `MatMulInteger`** 换成等价子图
（`Cast → Sub → MatMul → Cast`，绕开 ORT 那个 kernel），其余一字不动：

| 比较 | max_abs_err | mean_abs_err | corr |
|---|---|---|---|
| onnxruntime 原始 ⇄ 改写后（可信真值） | **0.477362** | 0.066037 | 0.974709 |
| **PuXian ⇄ 可信真值** | **0.000005** | 0.000000 | 1.000000 |
| PuXian ⇄ onnxruntime 原始 | 0.477362 | 0.066037 | 0.974709 |

**第二行与第一行逐位相同** ⇒ ORT 与"它自己绕开 kernel 后的版本"的差，正是它与 PuXian 的差
⇒ 偏差 100% 在 ORT 侧。**判据怎么会被带偏**：只要把「PuXian ⇄ ORT」当等价性判据，
这个 0.477 就会被记成"我们的数值缺陷"，而 PuXian 其实一直是对的。

复算方法（需要 102MB 生产模型，所以不进门）：
```bash
# 1) 改写 72 个 MatMulInteger 为等价子图（M314 侦察脚本 /tmp/m314onnx/ 已归档为报告附件）
# 2) 分别跑原始模型与改写模型，取 last_hidden_state
PYTHONPATH=/opt/fw python3 examples/m311_real_model/ref_onnxruntime.py <model.onnx>
# 3) 与 PuXian 输出三方对拍
bash examples/m311_real_model/verify.sh          # M311_REQUIRE_ORT=1 强制数值面
```

## 三 触发条件（最小装置 · 1536 组合扫描）

扫描面：`K ∈ 1..16` × `N ∈ 1..8` × `A 是否含 >127` × `a_zp ∈ {0,114,128}` × `b_zp ∈ {0,-7}`
= **1536 个组合**，每个组合用 `M=2` 的确定性数据，真值取规范定义（逐元素整数累加）。

| 条件 | ORT 出错数 |
|---|---|
| `A`(uint8) 全部 ≤ 127 | **0 / 768** |
| `A`(uint8) 含 > 127 | **568 / 768**（74%，`K=1` 除外 —— 单元素无向量化路径） |

按 `K` / `N` / `a_zp` / `b_zp` 分组都不改变定性 ⇒ **「A 里出现 > 127 的 uint8 值」是必要条件**。

⚠️ **但不是充分条件**：同形状（K=3 · N=4）枚举 **4860 组**候选数据才找到一例稳定触发
（`mmi_k3_ortbad`：错 `[0,3]` 一个元素 · 差 660 · 跨 session 10/10 稳定）。
所以门里只对**实测确认稳定偏离**的装置做硬断言，其余标 `expect_ort_diverge: null`（只报告）。
**硬断言「A 含 >127 就必须偏离」会偶发判红** —— 会偶发判红的门等于没有门（M274 缺陷 480）。

### 为什么生产模型必然受影响
`a_zero_point` 来自 `DynamicQuantizeLinear`，其输出域是 **0..255**（含 >127 与 ≤127 两者）
⇒ 72 个 `MatMulInteger` 里只要有一个触发，误差就会经后续 12 层放大。实测端到端确实受影响
（见第二节）。

## 四 这不代表我们的 ONNX 面已经完美

「最大的一块偏差不是我们的」≠「我们的都对」。**尚未用可信真值复核的算子**：

- `Softmax`（数值稳定性路径 · 是否需要减 max）
- `LayerNorm` / `RMSNormalization`（方差口径 · eps 位置）
- `Erf`（多项式近似的系数与分段）
- `Cast`（float↔int 的舍入方向与饱和）
- `DequantizeLinear`（fp16/bf16 刻度 · 逐轴）

⇒ 建议做成下一轮的「逐算子与规范对拍」面（**真值全部取规范定义，不取任何执行器**）。

## 五 判据设计的三条教训（比结论本身更值钱）

1. **第三方实现只能是"独立参考"，不能是"真值"。**
   参考实现也会错；把参考当真值，就会把正确的实现记成缺陷。
   正确做法：真值取**规范定义**（可手算、可复算），第三方实现用来做**交叉核对**。
2. **双向登记，别让"已知偏离"变成永久豁免。**
   `examples/m315_onnx_ort_kernel/devices/index.json` 的 `expect_ort_diverge` 字段是**双向**的：
   `true` 的装置哪天不偏离了 ⇒ 判红逼更新文档。否则 ORT 修好了我们还挂着一行"已知问题"。
3. **概率性的现象不许做硬断言。**
   ORT 74% 触发 ⇒ 任何「必须偏离」的硬断言都有 26% 概率假红。
   门里改成「实测稳定触发的装置断言偏离 + 对照组断言一致 + 其余只报告」。

## 六 给上游的报告要点（ORT issue 草稿）

- 版本：onnxruntime **1.19.2**（CPU EP，`GraphOptimizationLevel.ORT_DISABLE_ALL` 与默认档**都复现**）
- 算子：`MatMulInteger`，`A: uint8[2,K]` · `B: int8[K,4]` · `a_zero_point: uint8` · `b_zero_point: int8`
- 触发：`A` 含 > 127 的值（`K=1` 不触发）
- 最小复现：`examples/m315_onnx_ort_kernel/devices/mmi_k3_ortbad.onnx`（149 B）
  · 输入 `examples/m315_onnx_ort_kernel/devices/mmi_k3_ortbad.in.txt`
  · 期望 `...ref.txt`（规范真值，可由 `gen_devices.py::ref_manual` 独立复算）
  · 实测：`[0,3]` 应为 `-9728`，ORT 给 `-9068`（差 660）
- 影响：int8 量化模型的 `DynamicQuantizeLinear → MatMulInteger` 主路径（左侧激活必然含 >127）
- 我们这一侧的对照：PuXian（同一规范定义的独立实现）+ `onnx.ReferenceEvaluator` + numpy 整数域
  **三者一致** ⇒ 期望值可信

## 七 相关文件

| 路径 | 作用 |
|---|---|
| `examples/m315_onnx_ort_kernel/` | 最小装置 + 双向登记核对门（7 装置 · 14 判据） |
| `examples/m311_real_model/` | 端到端门（主判据已改为「与可信真值比」） |
| `examples/m311_real_model/truth_bge_fixed.npy` | 端到端可信真值（12416 B · 入仓生成物） |
| `examples/m311_real_model/baseline.tsv` | 辅判据（对 ORT 原始）的漂移基线 |
| `/data/reports/puxian-m314-recon-onnx-ort-kernel-20261020.md` | 侦察报告（含扫描细节） |
