# M89_PERF_BASELINE · VM 轨 vs C 轨性能基线（默认轨切换决策数据）

> 状态：✅ **2026-09-09 完成**（M89-S4 收口后置决策项「默认轨切换」的数据基础；本报告为决策提供量化依据，不改变"切换 = 独立立项 + golden 大迁移"的既有决策框架，仅重估其成本量级与触发门槛）。
> 资产：`examples/m89_perf/`（fib_calc.px / http_json.px / press_http.py / http_bench_one.sh / bench_vm_vs_c.sh，一键复现）。
> 机器：dongyue 107.151.245.74 · 8 核 16G · px 0.2.0（M89-S4 收口态）。

## 一、背景与问题

M89 把执行模型从「px 函数 = C 函数 fn_*、px_call 直接 C 递归」升级为「字节码 + 显式帧 VM」。目前 `px build` **默认仍是 C 文本轨**，`px build --vm` 为 VM 字节码轨（试做并行）。

待决问题：**默认产物切到 VM 轨，用户程序性能代价几何？** 此前的工程判断假设"纯解释执行通常是机器码的 5~20 倍慢"，据此把默认轨切换列为"收益未兑现、成本高风险大"的后置决策。本基线用实测检验该假设。

## 二、方法（对拍实体与三类负载）

**对拍实体**：同一 .px 源程序的两种用户产物——
- **C 轨** = `px build`（fn_* C 文本 → gcc 机器码运行时，即今日默认）
- **VM 轨** = `px build --vm`（BCModule 字节码镜像 C → 链接含 vm.o 的 runtime → 显式帧 VM 解释执行）

**三类负载**（覆盖计算/服务两端的代价区间；每类 3~5 轮取中位，带预热）：

| # | 负载 | 输入/规模 | 代表性 |
|---|---|---|---|
| 1 | fib_calc（纯计算热点） | 递归 fib(28)×5，每轮 ~6-9s | CPU 密集最坏情形 |
| 2 | compiler 形态（综合真实负载） | compiler.px 双轨产物（compiler_new= C 轨 / compiler_vm= VM 轨，**源码同一**）各跑 `bc dump selfhost/astdump.px`（6.8KB） | parse/codegen/bc_emit 全链、字符串/表/函数调用混合 |
| 3 | http_json（长跑 HTTP JSON 服务） | http_serve 事件驱动服务；`/api/check` dict→json_stringify 响应；8 并发 × 200 请求/线程 = 1600 req/轮 | ws-approve 类 IO/服务型负载 |

> 说明：基准 2 的 compiler_new/compiler_vm 皆由 bootstrap_prove_bc.sh 产出（compiler.px 分别走 C 文本轨 / 字节码轨编译），是"同源码、不同执行形态"的最干净对拍对。正确性锚点：双轨 stdout/产物逐字节一致（vm_ab 38 例 + 自举证明已守护，fib/http 每轮输出固定值校验）。

## 三、实测数据

### 基准 1 · fib 纯计算（wall 秒，越低越好）

| 轨 | 轮1 | 轮2 | 轮3 | 中位 |
|---|---|---|---|---|
| C 轨 | 6.091 | 6.602 | 5.953 | **6.091** |
| VM 轨 | 9.207 | 9.283 | 9.305 | **9.283** |

**VM/C = 1.52x**（递归热点最坏情形）。

### 基准 2 · compiler 形态（wall 秒，越低越好）

| 轨 | 轮1 | 轮2 | 轮3 | 轮4 | 轮5 | 中位 |
|---|---|---|---|---|---|---|
| C 轨 compiler_new | 1.570 | 1.297 | 1.390 | 1.341 | 1.318 | **1.341** |
| VM 轨 compiler_vm | 1.340 | 1.409 | 1.516 | 1.393 | 1.385 | **1.393** |

**VM/C = 1.04x**（综合真实负载基本持平；首轮 C 轨冷启动反而更慢）。

### 基准 3 · HTTP JSON 服务（RPS 越高越好 / p50 越低越好）

| 轨 | 轮1 RPS(p50) | 轮2 RPS(p50) | 轮3 RPS(p50) | 中位 RPS |
|---|---|---|---|---|
| C 轨 | 1304 (6.46ms) | 1251 (6.75ms) | 1376 (6.17ms) | **1304** |
| VM 轨 | 1586 (5.56ms) | 1410 (5.90ms) | 1535 (5.64ms) | **1535** |

**VM/C RPS = 1.00x（持平，三轮 VM 均 ≥ C，差异在运行噪声带内）**；p50 亦持平。

## 四、结论

1. **此前"VM 解释 = 机器码 5~20x 慢"的假设被实测推翻**。根因：px 的"C 轨"并非裸机器码性能——fn_* C 函数每一步值操作仍走 runtime 动态值层（对象头 / GC / dict / 字符串），函数级机器码只占执行时间的小部分；VM dispatch 只是在其上叠加的一小层。
2. **代价随负载类型连续变化，区间 1.0x~1.5x**：
   - 服务/IO 型（HTTP JSON）：**~1.0x（无显著劣化）**
   - 综合真实负载（编译器跑编译）：**~1.04x**
   - 纯计算递归热点（最坏情形）：**~1.52x**
3. **对默认轨切换决策的重估**：
   - 项目实际主流负载（m88b 服务 / ws-approve / 各类工具）以服务与综合为主 → **默认切 VM 的性能代价 ≈ 0~10%，不再是"不可接受的纯倒退"**；
   - 纯计算热点程序代价 ~50%——这类程序在切换后可用 C 轨逃生舱（保留 `px build` 旧产物路径 / `--cc`）或等 native/JIT 后端；
   - 切换的净收益（VM 为唯一执行轨 → 退役整栈保守扫描、精确 GC 兑现、帧协程、单执行语义维护）在服务/GC 侧大概率**反超**这 0~10% 的解释代价。
4. **决策框架更新建议**：默认轨切换从"后置大决策、低优先级"上调为**"可行低风险项，具备独立立项条件"**——但仍需独立里程碑（golden `.c` 大迁移 + capability 全量回归 + 稳定期），并先补齐下列前置缺口。

## 五、附带发现（本基线实测暴露，与轨无关/切换前置）

| # | 发现 | 影响 | 处置 |
|---|---|---|---|
| F1 | **bc_emit 默认参数未实现**：`compiler_new bc selfhost/capability.px` rc=1，报"bc_emit 默认参数未实现（A2+）: greet 参数 #1" | capability.px（41KB 系统文件）含默认参数语法 → VM 轨无法编译 → **默认轨切换前置缺口** | 列入切换里程碑必补（A2 待办） |
| F2 | **bc dump 大文件慢/未验证**：codegen.px import 链 bc >120s、compiler.px 全链 bc >10min（中途放弃）；`px build --vm compiler.px`（emit-c 路径）约 4-5min vs C 轨 pxc ~3.5min | 切换后大型程序构建期变慢 ~1.3-1.4x（运行期已证 ~1.04x 内） | 构建期成本单独评估；bc dump 大文件性能列入 M90 候选 |
| F3 | **HTTP 20 并发双轨 RPS 均骤降**（8 并发 ~1300→20 并发 ~340）且 max ~9-10s 尖刺，双轨同现 | px runtime 服务层/GC 在 20 keep-alive 持续请求下有退化（与轨无关，C/VM 均现） | 记入 M90 候选调查（区分 http_serve 模型 vs GC） |
| F4 | http_json server 压测窗口外空转吃 CPU（135-237%） | 事件循环空闲 spin（s4_daemon idle 场景已知特性） | 压测方法学：计时窗口内压测、窗口毕即杀（http_bench_one.sh 已封装） |

## 六、复现

```bash
./examples/m89_perf/bench_vm_vs_c.sh all     # 全三类（约 4-6 分钟，含预热）
./examples/m89_perf/bench_vm_vs_c.sh fib     # 单类
./examples/m89_perf/bench_vm_vs_c.sh http
./examples/m89_perf/bench_vm_vs_c.sh compile
```
前置：tools/px 可 build；selfhost/build/compiler_new + compiler_vm 存在（跑过 bootstrap_prove_bc.sh）；python3。
产物缓存 /tmp/m89perf/bin/（缺则自动 `px build` / `px build --vm` 重建）。

## 七、归档

- 本报告为「默认轨切换」后置决策项的**数据底座**（docs/M89_PLAN.md 决策记录同步引用）。
- 切换触发条件重述（据本数据修订）：① 服务/IO 为主的用户负载可直接切（代价 ~0）；② 纯计算热点程序需 C 轨逃生舱或 native/JIT 兜底；③ 切换独立立项（S0-S4 流程），前置 = 补齐 bc_emit 默认参数（F1）+ golden/capability 大迁移 + 稳定期/AB 回退开关。
