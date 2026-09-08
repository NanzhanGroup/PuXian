# M90 · 默认轨切换前奏（bc_emit 缺口清障 + 系统库 VM 化冒烟）

> 立项依据：docs/M89_PERF_BASELINE.md 决策重估 —— 默认轨切 VM 从「高成本后置
> 大决策」上调为**可行低风险独立立项**（服务/综合负载切换代价实测 ≈ 0~10%）。
> M90 = 切换的**清障与验证段**（切本身按 S0-S4 流程在清障完成后评估立项）。
> 维护：东月（dongyue）。状态随进度更新。

## 段规划

| 段 | 内容 | 状态 |
|---|---|---|
| **S1** | F1：bc_emit 默认参数支持（切换前置硬缺口）+ 附带修复 + 系统文件 VM 化验证 | ✅ 完成（本文件下方记录） |
| **S2** | F3 快查：HTTP 20 并发双轨 RPS 骤降 + ~9s 尖刺（runtime 服务层，与轨无关） | 待启动 |
| **S3** | F1 落地验证收尾：stdlib 全量 VM 编译冒烟 + capability 全量运行对拍 | 大部分已被 S1 吸收（见下） |
| **S4/收口** | 默认轨切换立项评估：golden 大迁移范围 + AB 回退开关设计 | 待 S2/S3 完成 |

## S1 完成记录（2026-09-09）

### F1 · bc_emit 默认参数（NARGS 指令 + callee 入口填充）
- **缺陷**：`bc_emit_func_body` 遇带默认参数函数直接 panic（A2+ 未实现）→
  capability.px（41KB 系统文件，含 `greet(who, punct="!")` 等）无法 VM 编译
  （`compiler bc capability.px` rc=1）→ 默认轨切换前置硬缺口。
- **修复**（对齐 codegen 语义 = callee 入口逐参 `(nargs>i)?args[i]:default`）：
  - `runtime/vm.h`：`PxFrame` 增 `nargs`（本帧实际实参数）；新指令
    `PXOP_NARGS 57`（a=dst：槽a = 本帧 nargs）`，PXM_MAX 58。
  - `runtime/vm.c`：指令名表 + `vm_frame_push` 存 nargs + 解释循环 NARGS case。
  - `selfhost/bc_emit.px`：`bc_emit_func_body` arity=必需参数数 / ndefault 计数，
    全部形参占槽；新 `bc_emit_default_fill` —— 函数入口对每个默认参数槽 i
    `NARGS→LT(pi<nargs)→JMPT 跳过 / 求默认表达式入槽`。默认值入口求值可引用
    前参/全局（与 codegen C 轨一致），经别名/px_call/闭包间接调用缺参同样兜底。
  - **向后兼容**：无默认参数函数 arity/ndefault/字节码不变（compiler.px 生态
    BC 镜像不变，除下述附带修复）；emit-c 静态 C 经 `.arity/.ndefault` 字段自动贯通。
- **验收**（examples/m90_s1/verify.sh 5 PASS）：
  1. default_args.px：VM 轨 == 旧轨 pxi 16 行 stdout 逐字节一致（常量默认/多默认
     部分提供/全局引用/别名间接缺参/返回值参与表达式）
  2. capability.px BC 编译成功（rc=1→0，dump 7414 行）—— F1 直击
  3. NARGS 落地：capability BC 含 2 处（greet/cfg 入口填充）
  4. cmpsem.px：C 轨产物 == VM 轨产物 8 行逐字节（默认**入口调用时求值**语义：
     引用前参 ref(1)=3、全局定义后改 g=500 → gdyn(1)=501；旧轨 pxi 为 def 时
     固化求值，不走 pxi 基准——VM 化替代目标是 codegen C 轨）
  5. for+break/continue 死循环回归（见下）

### 附带发现并修复 · bc_emit_for continue 跳转死循环（M89 遗留）
- **暴露**：capability.px VM 化运行卡死（CPU 100% 不退出，cap.out 停在 11 段
  break/continue 后）。最小复现 t1_bc.px 定位：**bc_emit_for 把 continue 回填到
  jb（增量**后**的 JMP 指令）→ body 内 continue 跳过 ADD → ctr 不前进 → 死循环**
  （Python for 语义须先 +1 再重判）。while 的 continue 跳 loop_start 正确（无增量）。
  vm_ab 38 例无 continue-for 未覆盖（M89 收口门盲区）。
- **修复**：continue 回填到**增量段起点 inc_start**（ADD 前），执行 ctr+=1 后经
  JMP 回 loop_start 重判。字节码仅含 continue 的 for 变化 → 无 continue 的 for
  （含 compiler.px 生态全部循环）字节不变，但 compiler.px 自身 bc_emit_for 源码
  变化 → BC 镜像/golden 更新（见回归）。
- **验收**：t1_bc.px / seg11.px（capability 11 段复现）修复后 VM 秒过、与 pxi
  逐字节一致；capability.px 全量 VM 化运行 == pxi **303 行逐字节一致（253 PASS）**
  —— 41KB 系统文件（FFI sqlite、h3/qpack、默认参数、推导式、生成器、break/
  continue 全量能力）在 VM 轨端到端跑通。已固化进 verify.sh 防回归。

### 回归（全绿）
- 双自举证明：C 轨（bootstrap_prove.sh）15058 行 ✅；BC 轨（bootstrap_prove_bc.sh）
  重放 == golden/compiler.bc.dump（30577 行 norm）✅ —— **双引擎（C/VM）编
  compiler.px 镜像逐字节一致**
- golden 更新：compiler.c（15058 行）、compiler.bc.dump（30577 行）随 compiler.px
  生态源码改动同步（bc_emit_default_fill 新增 + bc_emit_for 修复）
- vm_ab v2 收口门 **38 PASS + 0 GAP + 0 FAIL**（compiler_new/compiler_vm 以新
  runtime vm.o 重链后全量 examples 对拍）
- m89_s3d verify **9 PASS**（PxFrame.nargs 追加后并发 GC/生成器/堆回落无回归）
- diffcheck --all 全绿（C 轨 golden 全量对拍）
- 关键：runtime vm.o 重链（vm.h/vm.c 变化 → 旧 rtcache 无 PXOP_NARGS）——
  `px build --full` 触发新全量 rtcache（04ad5496）+ compiler_new/compiler_vm
  手动+prove_bc 重链新 vm.o，VM 产物方可执行 NARGS。

### 资产
- examples/m90_s1/{default_args,cmpsem,t1_bc,seg11}.px + verify.sh（5 PASS 一键）
- runtime/vm.{h,c} NARGS；selfhost/bc_emit.px 默认参数 + for 修复

## 备注
- capability VM 化端到端跑通（== pxi 303 行）已把 M90-S3「系统文件全 VM 编译
  冒烟」主体提前兑现；S3 剩余 = stdlib/*.px 全量 VM 编译清单化（F1 后应无缺
  口，例行冒烟）。
- 默认参数「pxi def 时固化 vs codegen 入口求值」语义分叉为**既有两轨分歧**
  （解释器 vs 编译器），非 VM 引入；VM 对齐 codegen（替代目标）。若语言规范
  需收敛（Python 式 def 时求值），属独立语义决策，M90 不处理。
