# M104 · VM 性能增强（LTO 构建档 + 执行引擎优化）实施计划

> 日期：2026-09-11 · 作者：东月 · 基线：v0.2.0-m103（commit 1d2c5c8）
> 依据：`docs/M104_native_prestudy.md`（③ native 旗舰 D0 预研）**路线 ① 裁定**
> —— native 天花板 = C 轨 ≈ 1.04~1.5x VM 且成本季度级，故 M104 转为
> **性能增强**：把杠杆打在「减少动态值层调用开销 + 降低解释循环固定开销」上。
> 定位：**纯运行时/工具链优化里程碑**（不动语言语义、不动发射器、不动 golden）。

---

## 〇、目标与非目标

**目标**
1. **LTO 构建档**：`px build` 可选 `-flto`（`PX_BUILD_LTO=1` / `--lto`），零代码改动兑现
   C 轨跨 TU 内联收益（D0 实测 1.16x），并同样惠及 VM 轨的 runtime 侧 helper 内联。
2. **VM 执行引擎优化**（默认轨，惠及全部程序）：
   - 解释循环固定开销削减（每指令 `px_srcfunc` 调用、SRCLINE 重复写、帧重载）
   - 热点指令**类型特化快路径**（INT⊕INT 直接算，绕过 `px_add/px_sub/px_lt/...` 动态分派）
   - **内联缓存**（GETG/GETF/SETF/CALLM 名解析结果按指令点缓存）
   - **分派优化**（switch → 计算跳转 / 指令融合，视实测收益取舍）
3. 全套回归守护：`vm_ab` 38 例 + `diffcheck --all` + 双自举 + m89~m103 相关 suites 零回归。

**非目标（明确不做）**
- ❌ 不动 `selfhost/bc_emit.px` / 指令集编号 / golden（超指令若做，走 **VM 侧阴影字节码**，不改发射器）
- ❌ 不做 JIT/去箱化（那是 6 个月+ 研究级项目，D0 已记录）
- ❌ 不动 native 语义对照面（C 轨继续为逃生舱，逐字节对拍契约不变）

---

## 一、实测基线（2026-09-11，本机 8 核 16G，`-O2`）

三微基准（源码在 `/tmp/m104/src`，脚本 `/tmp/m104/bench.sh`），三轮取中位：

| 负载 | 特征 | C 轨 | VM 轨 | VM/C |
|---|---|---|---|---|
| `fib26` ×5 | 递归：CALL/RET/算术/比较 | 2.717s | 2.739s | 1.01 |
| `loop_sum` 3M | 紧循环：INDEX/MOV/ADD/MOD/LT/JMP | 1.497s | 1.468s | 0.98 |
| `field_ops` 2M | 字段读写：GETF/SETF/ADD/SUB | 2.848s | 2.846s | 1.00 |

> 与 M89 基线一致（VM ≈ C）。**关键观察**：`loop_sum` 内层 12 条指令/迭代、
> 3M 迭代 ≈ 1.47s → **~41ns/指令**，远高于成熟解释器（5~10ns）→ **解释循环
> 自身有 3~4x 的对内优化空间**（此前被"M89 结论：VM≈C"掩盖：C 轨同受动态值层拖累）。

---

## 二、优化项与预期（逐项实测验收）

| # | 优化 | 手段 | 预期 | 风险 |
|---|---|---|---|---|
| O1 | 消除每指令 `px_srcfunc` | 移到 `vm_frame_push/pop`（帧切换才发生） | 5~15% | 低（语义等价） |
| O2 | SRCLINE 去重 | `fr->line` 未变则不写 `px_srcline` | 2~5% | 低 |
| O3 | 类型特化快路径 | ADD/SUB/MUL/LT/LE/GT/GE/EQ/NE/MOD/NEG/JMPT/JMPF：INT⊕INT 直算 | 10~25% | 低（逐 op 对齐 runtime 语义，兜底回落原函数） |
| O4 | 帧/slots/bc 局部缓存 | 循环内缓存 `slots`/`bc`/`name`，CALL/RET 后刷新 | 5~10% | 中（需谨慎处理帧变化） |
| O5 | 内联缓存 | GETG（全局槽 idx）、GETF/SETF（字段序号）、CALLM（方法解析） | 5~15%（真实程序） | 中（缓存失效/多态需正确性守护） |
| O6 | 分派优化 | switch → 计算跳转（GCC labels-as-values，可移植回退） | 10~20% | 中（大重构，仅在前述收益不足时启用） |
| O7 | LTO 构建档 | `tools/px` 加 `-flto` 开关（`PX_BUILD_LTO=1`/`--lto`） | C 轨 1.16x | 低 |
| O8 | 超指令融合 | VM 侧阴影字节码 peephole（不动发射器/golden） | 5~15% | 中（时间不允许则记录二期） |

**验收纪律**：每项优化后跑
1) 三微基准对比；2) `examples/m89_a2/vm_ab.sh`（38 例语义对拍）；3) `diffcheck --all`；
4) 涉及 runtime 则重跑相关 suites。**任一项回归即回退该优化**。

---

## 三、子步划分

| 子步 | 内容 | 产出 |
|---|---|---|
| S1 | 立项 + 基线 + 本计划 | `docs/M104_PLAN.md` |
| S2 | O1+O2+O3（解释循环固定开销 + 类型特化） | `runtime/vm.c` 改动 + 微基准 + vm_ab 38P |
| S3 | O4+O5（帧缓存 + 内联缓存） | 同上 |
| S4 | O6/O8 按实测取舍（收益不足才上） | 同上 |
| S5 | O7 LTO 构建档（tools/px + 文档 + 验证） | `tools/px` + README/CHANGELOG |
| S6 | 收口：全量回归 + 双自举 + pxi/pxi_vm 重链 + 文档 + tag | tag `v0.2.0-m104` |

> 说明：**S2/S3/S4 改的是 `runtime/vm.c`**（VM 轨单一执行引擎），C 轨不受影响；
> 但 `pxi`（解释器）与 `pxc_vm`/`pxi_vm` 均链 runtime，故收口须重链 bootstrap 产物
> 并重跑双自举证明（BC 轨 `bootstrap_prove_bc.sh` 的 SRC_CHAIN 已含 `runtime/vm.c`）。

---

## 四、正确性守护（红线）

- **语义冻结**：所有快路径必须与对应 `px_*` 函数**逐字节同语义**（INT 溢出/负数取模/
  浮点/字符串/列表分支全部回落原函数，仅 INT⊕INT 等无歧义分支直算）。
- **vm_ab 38 例**：C 轨产物 vs VM 轨产物 stdout 逐字节一致（M89 起的权威对拍）。
- **diffcheck --all**：lexer/parser/errors/codegen/value/interp 全绿。
- **双自举证明**：C 轨（`bootstrap_prove.sh`）+ BC 轨（`bootstrap_prove_bc.sh`）rc=0。
- **并发/GC 面**：M93–M102 协程/GC 相关 suites（m89_s3d、m93~m102）零回归。

---

## 五、风险与回退

| 风险 | 应对 |
|---|---|
| 快路径语义偏差（边界） | 兜底回落原函数；vm_ab + suites 双守护；逐 op 对照 runtime 源码 |
| 内联缓存多态失效 | 缓存键含「指令点 + 类型/形状」；未命中即回落解析并覆盖缓存 |
| 计算跳转可移植性 | 用 `#if defined(__GNUC__)` 守护，非 GNU 编译器保留 switch 路径 |
| 收益不达预期 | 逐项实测，不达标即回退；LTO 档（O7）为保底独立收益 |
| 重链/自举成本高 | 收口一次性做，S2~S4 期间只跑 vm_ab + 微基准（快速迭代） |

---

## 六、验收判定

**达成** = 下列同时满足：
1. VM 轨在至少 2/3 微基准上相对基线 **≥1.25x**，且真实负载（compiler 形态 / http_json）不劣化；
2. VM 轨对 C 轨比值：纯计算 ≥1.2x（VM 更快）**或**与 LTO 档联合后整体 ≥1.3x；
3. `vm_ab` 38P/0GAP/0F、`diffcheck --all` rc=0、双自举 rc=0、m89~m103 suites 零回归；
4. `PX_BUILD_LTO=1` 可用且有实测数据入档。

**未达成** = 逐项记录实测数据与回退原因，规划二期（如 O6/O8 未做则明确转二期候选）。

---

## 七、S2–S5 实施与实测（2026-09-11 · 本机 8 核 16G；计时 = `taskset -c 3` + CPU 时间取 min）

### 7.1 交付项

| # | 优化 | 实现 | 状态 |
|---|---|---|---|
| **O1** | 消除每指令 `px_srcfunc` 跨 TU 调用 | **线程局部镜像去重**：runtime 的 `g_px_src_func` 为 `__thread` 且在 VM 轨**仅由 vm.c 写入** → 以 `__thread` 镜像指针比较，值变才调用（**精确等价**，含 native 重入/帧弹回场景） | ✅ |
| **O2** | SRCLINE 去重 | 同上（`vm_track_line`，按**全局最后写入行号**去重——非按帧内行号，故弹帧后同行号无偏差；等价于原「每 SRCLINE 无条件写」） | ✅ |
| **O3** | 类型特化快路径 | ADD/SUB/MUL/DIV/IDIV/MOD/EQ/NE/LT/LE/GT/GE/BITAND/BITOR/BITXOR/SHL/SHR/SHRU/NEG/BITNOT/NOT/JMPT/JMPF/INDEX(LIST) 的 **INT⊕INT** 分支内联构造 `LXValue`；其余类型**一律回落**原 `px_*`（逐 op 与 runtime 源码对照：错误/分配/浮点/字符串分支零改动） | ✅ |
| **O4** | 帧槽指针提到局部 | `LXValue* slots = fr->slots;` 循环顶取一次（槽数组在堆上、不随 frames 数组 realloc 移动 → 安全） | ✅ |
| **O5** | 内联缓存（GETG/GETF/SETF/CALLM） | 未实施（见 7.3 归因：真瓶颈在 runtime 侧锁/名解析，需先解决 GC 根面与锁语义） | ⏭ 二期 |
| **O6** | switch → 计算跳转（labels-as-values） | 未实施（200+ case 整体改写、回归面大，收益上限 ~10-20%） | ⏭ 二期 |
| **O7** | **LTO 构建档** | `tools/px --lto` / `PX_BUILD_LTO=1`：runtime 预编译 .o + 产物 C + 链接**全链** `-flto`；rtcache 键含 `lto` 位（与默认档隔离，互不污染） | ✅ 交付（默认关） |
| **O8** | 超指令融合（VM 侧阴影字节码 peephole） | 未实施 | ⏭ 二期 |

### 7.2 实测

**A. VM 轨（默认轨；M104 = O1+O2+O3+O4 全开 vs 基线 v0.2.0-m103）**

| 负载 | 基线 | M104 | 提速 |
|---|---|---|---|
| `while_sum` 3M（纯分派：算术/比较/JMP，无容器无调用） | 0.311s | 0.290s | **1.07x** |
| `fib26` ×5（递归：CALL/RET/比较） | 2.757s | 2.503s | **1.10x** |
| `field_while` 2M（字段读写 GETF/SETF） | 2.095s | 1.985s | **1.06x** |
| `loop_sum` 3M（`for i in range(3M)`，含 range 物化） | 1.414s | 1.388s | 1.02x |
| **真实负载**：`compiler_vm bc stdlib/yaml.px`（VM 驱动编译器编 592 行 yaml.px） | 5.046s | 4.871s | **1.04x**（产物 dump 逐字节一致 ✅） |

**B. C 轨 + LTO 档（负载相关，实测双向）**

| 负载 | 默认档 | `--lto` | 比值 |
|---|---|---|---|
| `while_sum`（紧算术循环） | 0.086s | 0.062s | **1.39x** |
| `fib26`（深递归） | 2.212s | 2.515s | **0.88x（反向）** |

> LTO 的正负收益都是「跨 TU 内联」的副作用：循环体能吃掉 `px_add/px_mod` 等 helper 的调用开销（大赚），深递归则因内联膨胀伤 I-cache/寄存器分配（亏）。故 `--lto` 交付为**可选档、默认关**，`--help` 写明适用场景（循环密集型纯计算程序）。

### 7.3 裁定：性能目标**未达成**（按 §六 判据）——进展、归因与二期已入档

§六 判据要求「VM 轨在 2/3 微基准上 ≥1.25x」；实测 1.02~1.10x（真实负载 1.04x）→ **未达标**，按 §六 转为「逐项记录实测与回退原因 + 规划二期」。

**归因（与 `docs/M104_native_prestudy.md` 的 D0 判据互相印证）**：本运行时的成本主体是 **动态值层**，不是**分派层**。
- O1/O2/O4 砍掉的是解释循环固定开销（跨 TU 调用 / 重复写 / 间接寻址），合计仅 **~2–4%**；
- **唯一有效的是 O3 类型特化**（绕开 `px_add/px_lt/...` 动态分派）——递归/算术密集的 `fib26` 拿到 1.10x，占全部收益的大头；
- 负载一旦进入容器/字段/生成器路径（`loop_sum` 的 range 物化、`field_while` 的 `px_field` 线性 `strcmp` + `px_field_set` 的 `pthread_mutex_lock`+`sigprocmask`），类型特化就无能为力——**这些才是真瓶颈**（`loop_sum` 比同构的 `while_sum` 慢 4.6 倍，差异全在 `range(...)` 物化与容器索引，而非算术）。

**结论**：M104 从实验侧**证伪了「优化解释循环分派可拿到 1.2~1.5x」的假设**，并把「成本在动态值层」从 D0 的纸面论证升级为实测证据。真正的 1.2x+ 收益只能在**去箱化 / 类型反馈**（LuaJIT 形态，D0 已判 6 个月+ 研究级）或 **runtime 侧锁/名解析路径重构**上取得。

**二期候选（按收益/风险排序）**
1. **去箱化 / 类型反馈执行**（消掉 16B 带标签值 + 动态分派成本本身）——真解法，研究级；
2. **O5 内联缓存（高收益 · 中高风险）**：`px_field/px_field_set` 每次线性 `strcmp` 名解析，`px_field_set` 还每次 `pthread_mutex_lock(g_gc_mu)` + `sigprocmask`；`px_get_global` 每次 `rwlock` + `sigprocmask` + 线性 `strcmp` 扫描。按指令点缓存「结构类型 → 字段序号 / 全局槽」，可一次省掉 strcmp 与部分锁路径；前置是理清 GC 根面与锁语义（不能简单绕过）；
3. **O6 计算跳转**：收益上限 ~10–20%，但需整体改写 `vm_run_loop` 的 200+ case（`break` → `NEXT()`），回归面大、可移植性需 `__GNUC__` 分支；
4. **O8 超指令融合**：VM 侧阴影字节码 peephole（不动发射器/golden），收益 ~5–15%。
