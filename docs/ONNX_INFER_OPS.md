# ONNX 推理算子口径（M308 · S4：Gelu / Silu / RMSNormalization / ScatterND / Cos / Sin + `Trilu` 别名）

> 为什么单独一篇：M157 把执行器做到「能跑 MatMul」、M306 补上量化面之后，
> **跑不了一个 transformer 解码步** —— 缺 GELU/SiLU/RMSNorm（前馈 + 归一化）、
> KV cache 写入（`ScatterND`）、RoPE 前置（`Cos`/`Sin`）；且 ONNX **规范名 `Trilu`**
> 与本仓历史注册名 `Triu` 不一致，而算子查找走**精确 `strcmp`**。
> 本篇把「补了什么 / 语义是什么 / 与规范的差异在哪 / 判据在哪 / **没覆盖什么**」摊开 ——
> 让「能跑」这件事**可判定**，而不是靠一次演示。
>
> 姊妹篇：`docs/ONNX_QUANT.md`（M306 · 量化面）。

## 一 本仓支持什么（M308 新增 6 算子 + 1 别名）

| 算子 | 语义 | 备注 |
|---|---|---|
| `Gelu` | `0.5·x·(1+erf(x/√2))`（缺省）· `0.5·x·(1+tanh(√(2/π)·(x+0.044715·x³)))`（`approximate="tanh"`） | 两档**必须可判异**（门 ③c 单独守）；非法 `approximate` 响亮拒绝 |
| `Silu` | `x·σ(x)` | 按 x 符号分支避免 `exp(-x)` 溢出；**单一实现**（不许复合路径与算子路径各写一份） |
| `RMSNormalization` | `y = x / sqrt(mean_tail(x²)+ε) · scale` | `axis` 起的**尾部维**归一化（opset23 口径，同 `LayerNormalization`）；`scale` 形状 = `X.shape[axis:]`（允许标量）；`ε` 缺省 `1e-5`，负值响亮拒绝 |
| `ScatterND` | `out = data; out[indices[u]] = updates[u]` | `indices` 末维 `k ≤ rank(data)`；越界 / `updates` 元素数不符 / dtype 不符**一律响亮报错**（不静默截断） |
| `Cos` / `Sin` | 一元逐元素 | RoPE 前置；走既有 fp32/fp16 张量路径，**不引新 dtype** |
| `Trilu`（**别名**） | 与历史名 `Triu` **同一个实现** | ONNX 规范名是 `Trilu`；**两名并存**（旧名不许删 —— 既有 m157 语料与已过检产物依赖它） |

## 二 三条口径

1. **两档必须可判异**（`Gelu`）。把 erf 档与 tanh 档写成同一式之后，模型照样「能跑」、
   形状照样对、三方对拍照样有一份手算真值 —— **只要那份真值也是按同一式算的，三方一致地错**
   （M156 §38.4 的原话）。⇒ 门里单独一条腿（`tools/gelu_tiers.py`）只判
   「两档**逐元素最小间距 > 1e-5**」，不参与任何其他比较。
2. **数值容差写死**：
   - fp32 dump 的舍入在 ~3e-7 量级 ⇒ 对拍阈值 **1e-6**（留 3× 余量）；
   - `Gelu` 两档各自对闭式的阈值 **2e-6**（留 6×）；
   - KV cache 的「增量 ⇄ 全长」等价性是**搬运不是计算** ⇒ **严格相等，无容差**。
3. **响亮优于静默**（同 M163 / M166 家族）：`ScatterND` 的越界 / 元素数 / `k > rank` / dtype，
   `RMSNormalization` 的 `axis` 越界 / `scale` 长度，`Gelu` 的非法 `approximate` ——
   **全部走算子层响亮错误**，消息里**指名算子**并给出实质原因（门 ④ 逐条断言「报错来自它自己那个算子」）。

## 三 `Trilu` 别名（一个真实的命名缺口）

ONNX 规范名是 `Trilu`；本仓历史注册名是 `Triu`，而 `onnx_op_find` 走**精确 `strcmp`**
⇒ 只注册历史名时，**真实模型**里的 `Trilu` 节点会报「未实现的算子」。
⇒ 两条都注册、指向**同一个实现**；`examples/m157_onnx_exec` 语料同步补 `Trilu` 节点
（§二.0-A 硬约束：「注册表 == 语料执行集合」，注册而未执行 ⇒ m157 门判红）。
实测注册表 **75** 个算子**全部**在语料里执行过。

## 四 判据在哪（`examples/m308_onnx_ops/verify.sh` · 九层）

| 层 | 内容 |
|---|---|
| ① | 语料幂等（5 正例 + 7 行为负控）+ **真值覆盖**（悬空真值 0 · 有真值的模型都逐输出齐备） |
| ② | 语言侧驱动 `onnx_exec.px` 编译 |
| ③a | 三方对拍：手算真值 ↔ 独立参考（`../m157_onnx_exec/tools/exec_ref.py`）↔ C 侧 |
| ③b | **复算腿**：`tools/ops_ref.py` 把 Silu/Cos/Sin/RMSNorm/ScatterND 在**另一份实现**里从零重算（**刻意不复用 ③a**） |
| ③c | **分档腿**：`tools/gelu_tiers.py` 判「erf 档 ⇄ tanh 档逐元素可判异」 |
| ③d | **等价腿**：`tools/kv_equiv.py` 判「KV cache 增量 ⇄ 全长**逐值等价**」+「写入真落在目标槽、其余槽未动」 |
| ④ | 7 条行为负控：每条必须响亮报错，且报错来自**它自己那个算子**（`cases.json` 逐条登记 `op`） |
| ⑤ | 算子覆盖：6 算子 + `Trilu` 规范名都**真的执行过**（`by_op` 并集） |
| ⑥ | 静态（**去注释后**判）：注册 / 两档分叉 / 越界守卫 / 单一实现 |
| ⑦ | **源码负控 6 道**，每道打在不同判据层：A 分档腿 · B 行为负控 · C `Trilu` 运行时 · E 复算腿 · F 等价腿 · D 判据自伤（＋逐字节还原） |
| ⑧ | 覆盖边界（如实登记，见 §五） |

## 五 覆盖边界（如实登记 —— 别让「没覆盖」看起来像「覆盖了」）

- **未覆盖**：
  ① `Gelu` 的 fp16 输入路径（本门语料只走 fp32；fp16 读写往返由 m306 门覆盖，但**两条腿没串起来**）；
  ② com.microsoft `RotaryEmbedding` 单算子（M308-PLAN §二 的**可选项**）—— **未注册、未覆盖**；
  ③ KV cache 的**多步增量**与 `past_len` 语义（本轮取**单槽写入**的等价性判据）；
  ④ `onnx_t_geti` 对非有限浮点的整型目标仍是 UB（M306 已登记，未修）。
- **不跑真实模型端到端**：那是 m157 门 `M157_REAL=1` 的事 —— 它依赖**环境资产**（`/data/app/ws/models/embed/model.onnx`），不是仓库资产。

## 六 attention 辅助（M308-PLAN §二.S2）落地到什么程度

| 项 | 状态 | 判据 |
|---|---|---|
| **KV cache 增量解码等价性** | ✅ | `e16_kv_decode`：预分配 cache 上 `ScatterND` 写第 2 槽 ⇄ `Concat` 全长，**逐值等价**（③d 直接判这条**关系**，并附「写入真的发生」） |
| **RoPE 复合式** | ✅ | `e17_rope`：`Cos`/`Sin` + `Gather`（rotate_half 置换）+ 广播乘加，**7 节点**跑通，**全部中间量**对手算真值 |
| 多步增量 / `past_len` 语义入 spec | ⏳ 未做 | 移交下一轮 |
| **性能门接入**（`examples/m267_perf/`） | ⏳ 未做 | M308 §一 明确「**不设绝对性能判据**」⇒ 接入登记为**移交下一轮** |

## 七 怎么自己跑

```bash
cd examples/m308_onnx_ops
bash verify.sh              # 全档（含 6 道负控 · 约 4 分钟）
bash verify.sh --neg-skip   # 只跑正判据（约 30 秒 · CI 用这档）
```
