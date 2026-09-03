# M59 规划：数学与随机补齐（游戏/边缘两条线的公共地基）

> 状态：**✅ S1–S5 已全部落地并回归（2026-09）**，里程碑闭环见本文件 §八 回填。
> 主线编号 M59（M55/M56 为主线外占用编号，已核对可用）
> 依据：`docs/GAP_ANALYSIS.md` §七 候选 A（数学与随机补齐 → M59）
> 流程：本规划入库 → 本源审阅批准 → 按子步 S1–S5 执行（每步 verify + commit）→ 里程碑闭环

---

## 一、为什么是它

GAP_ANALYSIS 两条用户线（树莓派/边缘、2D/3D 游戏）的共同缺口：
- **游戏线**：圆周运动/转向/视角需要 `sin/cos/tan/atan2`，随机性需要 `random`，数值系统需要 `floor/ceil/round`；当前全部缺失。
- **边缘线**：传感器数据归一化、PID 控制、波形/统计同样需要这些函数。

且本项目 dogfood 哲学（真实用户喂 bug）下，语言面"最小侵入 + 存量能力优先"，
数学函数是**只加纯函数、不改语法/类型系统**的最小面改动，回归可控。

## 二、前置实测复核（本规划事实基准，开工时点复核）

已对本仓库实际代码逐项核对（非记忆推断）：

| 项 | 实测结果 | 证据 |
|---|---|---|
| 现有数学内置 | 仅 `abs/min/max/sum/sqrt/pow` 六枚 | `runtime/runtime.c:4848-4862` px_register_builtins |
| `sin/cos/tan/atan2` | **无**（含全 runtime*.c 扫描） | grep 零命中 |
| `floor/ceil/round/log/log10/exp` | **无**（`"exp"` 4 次命中均为 session 过期字段，非数学函数） | runtime.c:10371+ 上下文 |
| `random/rand` | **无**（examples/p0_random_io.px 实为随机文件 IO，非随机数） | grep + 文件内容 |
| libm 接入 | **已就绪**：`runtime.c:10` 已 `#include <math.h>`；`tools/pxc:175` 编译命令已带 `-lm -ldl` | 加数学内置**零链接/零依赖改动** |
| 内置实现模板 | `bi_sqrt`（runtime.c:2770）：`num_val(args[0])` → `px_float(sqrt(...))`，约 6 行 | 新增函数照此模板 |
| 解释器（pxi）内置机制 | `selfhost/ibuiltin.px` 独立分发表 `i_call_builtin`，**81 个** `if name=="..."` 分支；直调类（http_get/sha256）`Ok(同名C调用)` 即可复用 runtime 已注册函数 | ibuiltin.px 头部策略注释 + 分支清单 |
| 双模式不对称先例 | **已存在**：`sqrt` 编译模式有、解释器无（ibuiltin.px 无 sqrt 分支） | 本规划 S4 顺带补齐 |
| 浮点打印精度 | float 转字符串用 `%g`（默认 6 位有效数字）→ **断言必须用容差、不能比对打印串**（§十三 #7） | MINI_SUBSET §十三 #7 |
| 数值输入 | `num_val` 统一接受 int/float | bi_abs/bi_sqrt 均走 num_val |
| 交叉/回归基建 | M57-S4/M58-S4 已打通 aarch64 交叉 + qemu 自检流程；每里程碑 verify_sN.sh 惯例在位 | git log + examples/ |

## 三、范围决策

### 3.1 路线：内置（C libm），不建 stdlib `math.px`
理由（均基于 §二 实测）：
1. 现有数学先例 `abs/sqrt/pow` **全部走内置**，延续风格；
2. libm 已 `#include` + `-lm` 已链接 → 成本最低、零新依赖、静态二进制不变；
3. 性能：游戏主循环逐帧调用，C libm 原生 vs 纯语言手搓泰勒/查表；
4. 双模式一条路径：C 注册全集 → 解释器侧只需加直调分支（见 S4）。

### 3.2 必做清单（14 函数 + 2 常量）

| 分组 | 函数 | C 语义 | 测试要点 |
|---|---|---|---|
| 三角（弧度） | `sin(x)` `cos(x)` `tan(x)` | libm | sin(0)==0；sin(pi/2)≈1；cos(0)==1；cos(pi)≈-1；tan(0)==0 |
| | `atan2(y, x)` | libm（**先 y 后 x**） | atan2(1,1)≈π/4；atan2(0,-1)≈π（象限）；atan2(-1,-1)≈-3π/4 |
| 取整/舍入 | `floor(x)` | 向下 | floor(2.7)=2；floor(-2.1)=-3 |
| | `ceil(x)` | 向上 | ceil(2.1)=3；ceil(-2.7)=-2 |
| | `round(x)` | C99 round（.5 远离零） | round(2.5)=3；round(2.4)=2；round(-2.5)=-3 |
| 对数/指数 | `log(x)`（自然对数 ln） | libm | log(1)=0；log(e)=1（e 常量） |
| | `log10(x)` | libm | log10(1000)=3 |
| | `exp(x)` | libm | exp(0)=1；exp(1)=e（1e-12） |
| 随机 | `random()` → float ∈ [0,1) | splitmix64 | 区间；同 seed 序列可复现 |
| | `random_int(n)` → int ∈ [0,n) | splitmix64 截断 | n>0；区间 |
| | `random_seed(s)` → null | 设种子 | 重置后可复现 |
| 常量 | `pi` / `e`（float 全局） | M_PI / M_E | pi≈3.14159265358979；e≈2.718281828459045 |

**命名风格**：全小写单词/下划线，与 `now_ms/read_file/http_get` 一致；
不用 `srand/rand`（避免与 C 名混淆、语义含混），`random_seed` 显式表达"设种子"。

### 3.3 错误/边界语义（明确，写进 spec）
- **参数个数/类型错误** → `px_error` 终止进程（编程契约），与 `abs/sqrt/pow` 一致；**不做** Err(result)（网络 I/O 欠账那套语义不适用于纯数学，保持一致）。
- **数学域错误（log(-1)、sqrt(-1) 等）** → 透传 C 语义返回 **NaN**，不终止（与多数脚本语言"返回 NaN"一致，区别于 Python 抛异常；PuXian 简单哲学选 C 透传）。exp 溢出 → +inf。
- 需在 spec/MINI_SUBSET 明确记录 NaN/inf 传播；用户层暂不提供 `is_nan`（后续按需）。
- **atan2 参数顺序**：`atan2(y, x)`（先 y 后 x），文档必须写明，防踩坑。
- **floor/ceil/round 返回 float**（与 sqrt 一致，非 int）；需要整数用 `int(floor(x))`，文档注明。

### 3.4 PRNG 实现：splitmix64（自带，不用 C rand）
理由：C `rand()` 质量差且 glibc/musl 序列不同 → 跨平台不可复现；
splitmix64 为确定性 64 位 PRNG，实现约 5 行，质量良好（无短周期病态），
静态变量持有状态，默认种子取 `now_ms() ^ 时间` 混合。
**线程注意**：static 状态在多协程并发调用下序列不保证（游戏/边缘主循环单线程为主），文档注明；如后续需要再上原子/thread_local（本里程碑不做）。

### 3.5 明确不做（控制面）
- 不建 `stdlib/math.px`；不做 `asin/acos/atan/trunc/fmod/hypot/log2/degrees`（按需再加，最小集优先）；不修 `%g` 6 位打印（§十三 #7 单独留档，涉及全量 float 打印回归面大）；不加 `is_nan`。

## 四、子步划分（每步：代码 + verify 脚本 + 独立 commit）

| 子步 | 内容 | 验证（verify_sN.sh 判 PASS） |
|---|---|---|
| **S1** | C 实现三角族 + 注册 + `pi` 常量 | 编译模式：sin/cos/tan/atan2 数值断言（容差 1e-12）；pi 值与已知值一致 |
| **S2** | floor/ceil/round + log/log10/exp + `e` 常量 | 编译模式：边界断言（含负值、.5 远离零、log10(1000)==3 等） |
| **S3** | splitmix64 + random/random_int/random_seed | 编译模式：区间断言、同 seed 两次序列逐位一致、random_int 边界、pi/e 可读性 |
| **S4** | **双模式同步**：ibuiltin.px 给 14 函数加直调分发 + 顺带补 `sqrt`（消除已知不对称） | pxi 解释 smoke（数学断言同 S1-S3 抽几条）；aarch64 交叉编译 + qemu 跑 math 测试（验证 libm 静态跨架构）；既有回归抽查（hello/fib/struct/std_demo + M57/M58 verify_s1 快速项） |
| **S5** | 文档收口：spec.md §10.2 + 附录 C；MINI_SUBSET §十三.4（双模式同步记录）；CHANGELOG [Unreleased]；ROADMAP M59 勾选；GAP_ANALYSIS §七 数学缺口勾销 | git 干净；全量 verify_s1–s4 复跑 PASS |

**文件布局**（每文件 <500 行约束天然满足，改动全为增量）：
- `runtime/runtime.c`：bi_* 实现 + px_register_builtins 注册（S1–S3）
- `selfhost/ibuiltin.px`：解释器分发分支（S4）
- `examples/m59_math/`：`math_s1.px`~`math_s3.px` + `verify_s1.sh`~`verify_s4.sh`（S1–S4，每个 verify 自带就绪探测与退出码断言）
- `docs/spec.md` / `docs/MINI_SUBSET.md` / `CHANGELOG.md` / `docs/ROADMAP.md` / `docs/GAP_ANALYSIS.md`（S5）

## 五、回归矩阵

| 维度 | 手段 |
|---|---|
| 数值正确性 | 编译模式断言 vs 已知数学值（容差 1e-12，避开 %g 6 位打印问题） |
| 双模式一致 | S4：同一批断言在 pxi 解释模式复跑；ibuiltin.px 新增分支为直调类 → 语义与编译一致 |
| 跨架构 | aarch64-linux-musl 交叉 + qemu 跑 math 测试（libm 静态可用性 + 结果与 x86 一致） |
| 既有功能 | hello/fib/struct/std_demo + M57/M58 verify_s1 快速回归（新增注册不破坏既有 builtin） |
| 随机确定性 | 同 seed 序列逐位一致（不强依赖时间）；分布宽松断言（N 次均值近 0.5 ± 0.1） |
| 文档一致性 | spec 内置清单 ↔ ibuiltin.px 分支 ↔ runtime 注册三方对齐检查（S5 收口时 grep 三处同名） |

## 六、里程碑闭环判据
1. S1–S5 verify 全 PASS（含 aarch64 交叉 + pxi 双模式）；
2. 三方对齐：runtime 注册 / ibuiltin 分发 / spec 文档中 14 函数 + 2 常量同名齐全；
3. git 干净、逐步 commit 可追溯；ROADMAP M59 状态由"规划"推进为"完成"。

## 七、风险与备注
- **风险极低**：纯增量内置，不动语法/GC/类型；libm 已在链路上；最大回归面是"新全局名与用户代码冲突"（pi/e 为新增名，存量代码无占用，grep 确认）。
- **遗留（有意）**：%g 6 位打印精度（§十三 #7）；解释器对设备类 builtin（s3_* 等）白名单缺失（§十三 #8）；均非本里程碑范围，已在 GAP/欠账清单留档。
- 规划时点复核已做；**S1 开工前不再重复扫描**（本规划 §二 即为最新事实，正文已含行号证据）。

---

## 八、执行回填（S1–S5 ✅）

| 子步 | commit | 结果 |
|---|---|---|
| S1 三角+pi | `8a85834` | 编译模式 sin/cos/tan/atan2 数值断言（1e-12 容差）+ pi；verify_s1 PASS |
| S2 取整/对数+e | `93eb9a9` | floor/ceil/round/log/log10/exp + e；域错误 NaN/inf 透传实证；verify_s2 PASS |
| S3 随机 | `bddd953` | splitmix64：random/random_int/random_seed；同 seed 序列逐位复现；verify_s3 PASS |
| S4 双模式同步+aarch64 | `3f7e434` | pxi 白名单 +15 + pi/e 常量种子 + sqrt 缺口补平 + bootstrap/pxi 重建；编译/解释/qemu-aarch64 三态 PASS、双模式逐字节一致、splitmix x86==aarch64；hello/fib + m57_s1/m58_s1 回归 PASS |
| S5 文档收口 | `cc2dd3f`（+本回填） | spec §10.2/§10.3、MINI_SUBSET §十三.4、ROADMAP M59 勾选、GAP_ANALYSIS 数学缺口勾销、CHANGELOG [Unreleased] |

全量 verify_s1–s4 复跑 PASS；git 干净。新发现留档：编译 `%g` 整值浮点打印 "3" vs 解释器
"3.0"（先于 M59 存在，MINI_SUBSET §十三.4）。
