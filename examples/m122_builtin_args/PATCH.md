# M122 · 内建函数实参校验一族 —— 修复设计（qg-issue 77）

## 一、确认的真缺陷（复现清单实测）

### A 类：解释器「直调宿主 native」分支缺 arity 校验（29 处）
`ibuiltin.px` 里 `def i_call_builtin` 的 name 分支，形如 `return Ok(json_stringify(args[0]))`：
- **缺参**（`json_stringify()`）→ 解释轨报 `R1003: 列表索引越界: 0 (len=0)` ← **错误码与错误因**（参数缺失≠索引越界）；VM/C 轨报 `json_stringify 需要一个参数`。
- **多参**（`json_stringify(1,2)`）→ 解释轨**静默只取 args[0]，rc=0**（`json_stringify(1,2)` 输出 `1`）；VM/C 轨报错。← **两轨不一致 + 静默吞错**。

统一修补：在每分支前加 `len(args)` 精确校验（=N 或 N~M 区间），报错文案**逐字对齐 VM/C 轨**（`px_error` 文案），两轨一致。

### B 类：ibuiltin.px 有校验但文案/语义与 VM/C 轨不一致

### C 类：VM/C 轨共同缺口（争议 → 需另立）

| 项 | 现状 | 风险 | 建议 |
|---|---|---|---|
| `len` 对 `null` | 报 `R1002: len 不支持类型 null`（有码） | 语义分歧：Rust 原版 `len(null)` 报类型错 | 保持严格（不动） |
| `range` 对字符串 | C 轨 `期望整数，实际是 string`（**无码**）；解释轨 `R1002: range 参数需要整数`（有码） | C 轨无码 | **顺手补码 R1002**（文案族收口） |
| 数值函数域错误（sqrt(-1) 等） | NaN/+inf 透传 | 与编程契约不符但属预设行为（注释明言） | 不动（文档确认） |

## 二、落地范围（本轮）

1. **`selfhost/ibuiltin.px`**：A 类 **≈50 个**直调分支补 `len(args)` 校验 + 文案对齐 VM/C 轨。
2. **`runtime/runtime.c`**：
   - **400+ 处错误文案统一前缀 `R1002:`**（`px_error` 族收口：缺失/类型/取值错显式编码；`mem_write` 越界用 `R1003`）—— 消除"无码报错"与`R1002`家族不一致。
   - `range("a")`/期望整数族补 `R1002` 码（对齐解释轨）。
   - **行为收紧**：`min/max` 混型比较报 `R1002: 比较不支持`；`pow` 参数必须数字（对齐解释轨）；`sorted` 接受 tuple（对齐解释轨，消除 C 轨窄化）；`exit` 超过 1 参报 `R1002`（静默 exit(0) → 报错）。
3. 门：`examples/m122_builtin_args/verify.sh` 两轨一致性（A 组 18 例缺参 + B 组 6 例多参 + C 组合法护栏 6 例 = **30 探针 × 156 断言** 全绿）。

## 三、错误码约定（对齐 selfhost/i_err.px）

- `R1002`：类型/参数错误（个数、类型,含"需要 X 个参数"文案）
- `R1003`：索引越界（**不得**因参数缺失误报）
- `R1005`：方法实参个数不足（call_dict_method 已用）
- `R1006`：值域错误（`range step=0`）

## 四、边界

- 只动 PuXian 仓库 selfhost/ + runtime.c + examples/m122_builtin_args/。
- **不动**：不新增语言原语、不动 dict 严格语义、不动协程/退出码、不动同伴仓库。
- 改 `selfhost/*.px` ⇒ 必须 `--rebake-all`（铁律），并跑 m120 门确认无回归。