# M308_PLAN · ONNX 推理算子补齐（S4：RMSNorm / SiLU / GELU → KV cache / RoPE）

> 日期：2026-10-14 · 作者：东月 · 基线：v0.2.307（commit f0aea27）
> 令源：`ROADMAP.md` §三「已立项主线 · ONNX 内核线（S1–S5）」—— S1 ✅M303 · S2 ✅M306 ·
> S5（aarch64 第一档）✅M307 ⇒ 本里程碑落在 **S4**。
> 性质：**L0 runtime**（ONNX 执行器算子面，主要落在 `runtime/onnx_ops.c`）；
> **不动语言语义 / 不动发射器 / 不动 golden**。
> 状态：**供审稿**（`ROADMAP.md` §六 立项流程第 1 步「出 `docs/M*_PLAN.md` 规划供审」）。
> 若项目方改指 S3 或其它主题，本稿改档为备选并另出稿（见 §六）。

---

## 一、目标与非目标

**目标**

1. **算子可达性**：把 ONNX 执行器从「能跑 MatMul / 量化」补到「**能跑一个 transformer 解码步**」——
   `Gelu` / `Silu` / `RMSNormalization`（前馈 + 归一化）· `ScatterND`（KV cache 写入）· `Cos`/`Sin`（RoPE 前置）。
2. **逐值正确性**：每个算子配「手算真值 ↔ 独立纯 Python 参考 ↔ C 侧」三方对拍，容差口径写死进 spec。
3. **零依赖可审计**：全程走既有 fp32/fp16 张量路径，**不引新 dtype、不引 onnxruntime**；
   部署形态卖点（零依赖单二进制喂 ONNX，`PERF_BASELINE_V2.md` §十二.4）由此**兑现后半句**。

**非目标（明确不做）**

- ❌ 跟 onnxruntime / CUDA 拼算力（§十二.4 已定：胜负手是部署形态 / 胶水层数 / 可审计三条）。
- ❌ 把「比 onnxruntime 快」当卖点；本里程碑**不设绝对性能判据**。
- ❌ S5 基准机（2026-10-12 用户指示「先做其他」，暂缓）· ❌ S3 分块（见 §六，须指定机器）。

---

## 二、现状事实（本机核实 · 2026-10-14）

### 2.1 执行器在位

`runtime/onnx_proto.c`（protobuf 自解 · 940 行）· `onnx_tensor.c`（880）· `onnx_ops.c`（1068，
算子注册与实现）· `onnx_exec.c`（377，拓扑执行器）+ native 面 `runtime_onnx.c`
（`onnx_model_open` / `onnx_run` / `onnx_op_names` / `onnx_info` / `onnx_initializer*` / `onnx_model_close`）。

### 2.2 已注册算子 **68**（单一事实源 `onnx_ops.c` g_ops；门用 `onnx_op_count()`）

M157 时 64 + M306 量化 4。已含 `LayerNormalization` / `Erf` / `Sigmoid` / `Sqrt` / `Mul` / `ReduceMean`
（⇒ Gelu/Silu/RMSNorm 可复用）· 含 `Triu` · **不含 `Cos` / `Sin`**（已逐名核）。

### 2.3 缺口（本里程碑范围）

| 算子 | 说明 |
|---|---|
| `Gelu` | opset20；两档 `approximate`（erf 精确式 / tanh 近似式），复用既有 `Erf` |
| `Silu` | 等价 `x * Sigmoid(x)`；`Sigmoid` 已在 ⇒ **单实现**，不许"复合路径 + 算子路径"两份 |
| `RMSNormalization` | opset23；`x / sqrt(mean(x²)+eps) * scale`（默认 axis = 最后一维） |
| `ScatterND` | **KV cache 写入的唯一手段**（回读侧 `Gather` 已有） |
| `Cos` / `Sin` | **RoPE 前置**：原「RoPE 复合式零新算子」口径**不成立**（复合式所列 `Cos`/`Sin` 不在库内） |

`RoPE` 本身走**复合式**（`Cos`/`Sin` + Slice/Concat/Transpose/Mul/Add）；
com.microsoft `RotaryEmbedding` 单算子**可选**（若注册须与复合式逐值一致）。

### 2.4 已知命名缺口：`Triu` ≠ `Trilu`

注册名是 **`Triu`**，**非** ONNX 规范名 `Trilu`；而 `onnx_op_find` 是精确 `strcmp`
⇒ **真实模型里的 `Trilu` 节点匹配不上**。修法廉价（加规范名 + 保留旧名），但要**同步扩语料**
（见 2.6）。本里程碑内作为候选小缺口处理。

### 2.5 参考实现可得性

本机 **无 `onnx` / `onnxruntime` / `numpy`**（已核）⇒ 对拍**不引 ORT**，走
**纯 Python 参考 + 手算真值**（沿 `examples/m306_onnx_quant/tools/quant_ref.py` 既有模式：
另一套独立写的纯 Python，含自写半精度位运算）。日后可装 ORT 再加一路交叉对拍（非阻塞）。

### 2.6 ⚠️ 硬约束：每补一个算子必须同步扩 m157 语料

`examples/m157_onnx_exec/verify.sh` ④ 用 `opnames.px` 取注册表，再扫全部语料 `by_op` 并集，
断言 **`declared == used`**：注册而未执行 ⇒ 红（`从未执行`）；执行而未注册 ⇒ 亦红。
⇒ 本里程碑每往 `g_ops` 追加一个算子，**必须同步让它在 m157 语料里真的跑一遍**，
否则「ONNX 四门零回归」会从"零回归"变成"**必红**"。

---

## 三、子步（按序落地，每步 verify + 落盘）

**S1 · 推理算子实现**（`runtime/onnx_ops.c`；同步扩 m157 语料）
- `Gelu` 两档属性**必须都在场且可判等/可判异**（负控：把两档写成同一式 ⇒ 门必须红）。
- `Silu` 单实现；`RMSNormalization` 属性 `epsilon` 默认 1e-5、axis 照 opset23；`Cos`/`Sin` 一元逐元素。
- `Triu` 命名缺口（2.4）一并收口（旧名保留，避免破坏既有语料）。

**S2 · attention 辅助（KV cache + RoPE）**
- `ScatterND`：逐位置真值 + **越界 / 形状不符必须响亮报错**。
- **KV cache 布局**写成可进 spec 的约定（`[B, H, T, D]` 追加式 + `past_len` 语义）；
  硬判据 = **增量解码（逐步 ScatterND + Concat）与一次性喂全长逐值等价**。
- RoPE：先复合式跑通为**基线**，再评估是否注册 `RotaryEmbedding`（可选）。

**S3 · 数值对拍（三方）**
- 手算真值：每算子 ≥3 形态（epsilon 边界 / 全零 / 负值 / fp16 刻度）。
- 纯 Python 参考：`examples/m308_onnx_ops/tools/ops_ref.py`（**独立第三份实现**，不与 C 侧共享代码）。
- 容差口径**写死进 spec**：erf 精确式 ≤1e-6(fp32) / ≤1e-3(fp16)；tanh 近似式与精确式之差
  **只登记不判红**（两式本不同）。

**S4 · 性能门接入**（`examples/m267_perf/` + `tools/perf_gate/`）
- 新用例 `onnx_gelu` / `onnx_rmsnorm` / `onnx_rope_kv`（**生成器入库、模型文件不入库**，沿 §12.5 三条决定）。
- **必须同步加下限判据**（§12.5：不加则「坏模型空转」会以"性能大幅提升"面目过门）；新用例挂 NC-E 式负控。
- CI 先 **warn-only**（清歌 `perf_gate v1.1`，阈值 10%），本机 5 次取 min。

**S5 · 文档收口 + 发布**
- `spec.md` ONNX 算子节（新增算子 + epsilon/axis 语义 + 容差口径）· `PERF_BASELINE_V2.md` §十二 表增 S4 行 ·
  `ROADMAP.md` §三 ONNX 线 S4 → ✅ · `CHANGELOG.md` M308 节 · `docs/README.md` · tag `v0.2.308`。

---

## 四、交付物

| 类 | 物 |
|---|---|
| 代码 | `runtime/onnx_ops.c`（Gelu / Silu / RMSNormalization / ScatterND / Cos / Sin）· 视情 `onnx_tensor.c`（KV cache 布局助手） |
| 门 | `examples/m308_onnx_ops/verify.sh`（正判据分层 + **每道负控各自独立判红**）· **`examples/m157_onnx_exec/` 语料同步扩至覆盖新算子**（§2.6）· `examples/m267_perf/` S4 用例 · `selfhost/gates.registry.sh` 注册（与 `ci.yml` 双向一致） |
| 文档 | 本文件 · spec / PERF_BASELINE_V2 / ROADMAP / CHANGELOG / docs/README |

---

## 五、判据红线

三轨一致 `diffcheck --all` rc=0 · `vm_ab` 38P/0GAP/0F · capability 双模式 ·
**ONNX 四门零回归（m156 / m157 / m250 / m306）** · 双自举逐字节 · 全量门 0 红 ·
入库件重烘（`rebake_bin.sh`）+ `--check-all` 指纹复核（改 runtime 必需）。

---

## 六、备选：S3（MatMul 分块 + B 预转置）—— **须先指定机器**

- **前置硬条件：指定机器名。** 机器证据（`PERF_BASELINE_V2.md` §13.4 / §14.4）512³÷1024³ 跨页崩塌比：
  EPYC 7K62 **3.75×** · 5 号机 1.78× · Xeon E5-2682v4 1.34× · Neoverse-N2 **1.00×** · Gold 6462C **0.79×**
  ⇒ 现象与机型强相关；**无崩塌机型上 S3 收益 ≈ 0**。
- 步序：S1 分块 + B 预转置（数值等价优先，容差对拍）→ S2 等价回归 → S3 指定机 512³/1024³ 崩塌比对 +
  收益量化 → **若崩塌比 <1.5× 即判「不值得做」并如实归档**（沿用 M104「量化表否定的子项立即降级」纪律）。

---

## 七、待确认（唯一外部依赖）

1. **M308 主题确认** —— 本稿按 **S4** 立案（机器无关、判据本机可得、属已立项 ONNX 线内子步）。
   若要改指 **S3**，请**同时给机器名**；若要改指其它主题，一句话即可，本稿改档。
2. 其余子步（S1–S5）**不需要额外外部输入**。
