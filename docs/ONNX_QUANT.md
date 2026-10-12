# ONNX 量化算子口径（M306 · S2）

> 为什么单独一篇：量化算子是**静默出错代价最高**的一类 ——
> 权重解错 ⇒ 模型"能跑"但数值全错；量化公式写错 ⇒ **权重文件会被静默写坏**。
> 所以这里的每一条都写成**可判定**的形式，并把"我们与参考实现的差异点"摊开。

## 一 本仓支持什么

| 算子 | 语义 | 备注 |
|---|---|---|
| `DequantizeLinear` | `y = (x - x_zero_point) * x_scale` | 逐张量 / 逐轴（`axis` 缺省 1、**负值按秩归一**） |
| `QuantizeLinear` | `y = saturate(round_half_even(x / y_scale) + y_zero_point)` | 输出类型 = 零点类型（缺省 `uint8`） |
| `DynamicQuantizeLinear` | 运行时求刻度（见 §三） | 输出 3 个：`y` / `y_scale` / `y_zero_point`（后两个是**秩 0 标量**） |
| `MatMulInteger` | `Y[...,m,n] = Σ_k (A[...,m,k] - a_zp) * (B[...,k,n] - b_zp)` | 输出恒 `int32` |

**输入类型**：`DequantizeLinear` 的 `x` 收 `int8/uint8/int32`（规范 T），另收 `int16/uint16`；
`MatMulInteger` 的 `A`/`B` 同上。**多收的这两种不会引入静默错值**（公式与整型宽度无关），
而"少收"会变成一次硬失败 —— 故取超集，并在此登记。

## 二 三条统一口径（与 `QuantizeLinear` 同源）

1. **舍入 = 就近偶数**（`round_half_even`）。
   ONNX 的两份官方参考在「恰好 `.5`」上就不一致（onnxruntime 的 `std::round` 是**远离零**，
   onnx 参考实现的 `np.round` 是**就近偶数**）。真实激活里 `.5` 是测度零，
   但口径必须写死 ⇒ 本仓统一就近偶数，且两个算子**复用同一份实现**（`q_round`）。
2. **非有限值不落到整数转换**。
   `NaN` / `±Inf` 在饱和判据里被显式接住（`if (!(r >= lo)) r = lo;`）——
   直接 `(int64_t)NaN` 是 UB（C11 6.3.1.4）。
3. **刻度与零点必须是"算的时候用的那个数"**。
   `DynamicQuantizeLinear` 的输出 `y_scale` 是 **float32**，因此内部算 `y`/`y_zero_point`
   时用的是 `(float)` 化之后的同一个值 —— 否则"导出的刻度"与"用的刻度"差一点点，最难查。

## 三 DynamicQuantizeLinear 的精确语义

```
min_x = min(0, min(x))            ← **含 0**（规范要求，初值取 0 即可自然满足）
max_x = max(0, max(x))
y_scale = (max_x - min_x) / 255
if y_scale == 0 或 NaN 或 Inf:  y_scale = 1.0      ← 全常量输入（含全 0）
y_scale = (float)y_scale                            ← 后续一律用这个值
y_zero_point = clamp(round_half_even(0 - min_x / y_scale), 0, 255)
y = clamp(round_half_even(x / y_scale) + y_zero_point, 0, 255)
```

⚠️ **「含 0」这一条最容易漏**：全非负输入时若只看数据本身，`min_x` 会是正数，
刻度偏大、零点非 0 —— 而规范要求包含 0。`examples/m306_onnx_quant` 里
`q17_dynquant_nonneg`（`x=[0,127.5] ⇒ scale=0.5 · zp=0`）就是这条的守门用例。

## 四 MatMulInteger 的零点形态

| 位置 | 允许的元素数 | 含义 |
|---|---|---|
| `a_zero_point` | `1` | 逐张量 |
| | `prod(A.dims[:-1])` | **逐行**（2-D 时即 M） |
| `b_zero_point` | `1` | 逐张量 |
| | `N` | **逐列** |

其它长度**响亮报错并给出实际数字**（不猜、不夹取）。

**输出恒 `int32`**；内部用 `int64` 累加，**越界即响亮报错**（不静默回绕）。
真实模型（K ≤ 768、8 位取值范围）远不会触发，但"不会触发"不等于"可以不查"。

## 五 覆盖边界（如实登记）

- **不支持且响亮拒绝**：块量化（`block_size ≠ 0`，opset 21+）· 非饱和量化（`saturate = 0`）·
  4-bit（`int4`/`uint4`）· `QLinearConv`/`QLinearMatMul` 等**整数算术算子族**（与本族不同）。
- **`float16`/`bfloat16`**：读 / 写往返 / 特定位型（次正规、中点、最大值）已覆盖；
  ⚠️ **未覆盖 inf/NaN 的写路径** —— `onnx_t_geti` 的 `(int64_t)double` 对非有限值是 UB。
  M306 只把 **`Cast` 的浮点目标**绕开了它（M306 缺陷 529），其它算子的整型目标仍可能踩到。
- **未能与 onnxruntime 对拍**：本机**没有** onnxruntime（无 `.so`、无 python 包）。
  ⇒ 数值口径的依据是「ONNX 规范文本 + 一份独立参考实现 + 手算真值」三方，
  **不是**与 ORT 的逐位对拍。这一点在 `DynamicQuantizeLinear` 的舍入上尤其要记住（见 §二.1）。
- **真实模型**：`examples/m157_onnx_exec` 的层 ⑦ 会用本机的
  `/data/app/ws/models/embed/model.onnx`（int8 量化 bge · 102 MB · 1177 节点）做端到端；
  它依赖 `DynamicQuantizeLinear` + `MatMulInteger` + `DequantizeLinear` —— 这三个正是 M306 补的。
  （该资产**不在版本控制里** ⇒ 缺失时记 SKIP 而不是 FAIL。）

## 六 判据在哪

| 层 | 门 | 内容 |
|---|---|---|
| 三方对拍 | `examples/m306_onnx_quant` | 手算真值 ↔ 独立参考（纯 Python，自写半精度位运算）↔ C 侧，27 个模型 / 35 个输出点 |
| 行为负控 | 同上 | 14 个非法形态**每个都必须响亮报错**，且消息要「指名算子 + 给出实质原因」 |
| 算子覆盖 | `examples/m157_onnx_exec` 层 ④ | 注册表 ⇄ by_op 并集**逐条对齐**（新增算子必须真的有语料跑过） |
| 源码在位 | `examples/m306_onnx_quant` 层 ⑥ | 去注释后判：注册 / 守卫 / **单一实现**（舍入与形态解析不许有两份） |
