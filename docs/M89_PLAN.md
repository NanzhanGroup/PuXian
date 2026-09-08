# M89_PLAN · VM 化旗舰里程碑（AST/C 递归 → 显式帧 + 平坦字节码 VM）

> 状态：🆕 **2026-09-07 立项**（M88 A/B 收口后，按 M88_PLAN §四·A 远景裁定启动）。S0 版本升格与本文档同批执行；S1 VM 化详勘预研紧随其后。
> 关联：M88_PLAN §四·A（VM 化 = 总钥匙，native 后端排最后）、§四·A·落地建议第 2 条（VM 化 + C 类合并为独立旗舰里程碑，先详勘钉工期再拍板）。

## 〇、一句话

把 PuXian 的执行模型从「编译器把每个 px 函数生成 C 函数 fn_xxx + 运行走 px_call 直接 C 递归（interp.px 为 AST tree-walking）」升级为「编译到显式字节码 + 显式 VM 帧栈解释执行」——这是语义冻结、精确 GC、用户态协程（C 类）、native 后端的共同前提，是全项目远期路线上的**总钥匙**。

## 一、背景与裁定依据（摘自 M88_PLAN §四·A，2026-09-07 定稿）

1. 并发容量/平台不能靠 native 后端解决，靠执行模型可挂起（协程）与 OS API 层；
2. **VM 化是三合一总钥匙**，必须在 native 之前：① GC map/精确 GC 需帧显式；② 语义基准：先有 VM 作"对的实现"，native 后端对拍归因；③ 教科书全走此路（JVM / CPython 3.11 / LuaJIT = 先稳定 VM 再谈 JIT）；
3. **PuXian 现状坐实无捷径**（M88 立项期代码侦察）：interp.px 是 AST 树遍历递归（i_exec_stmt/i_eval_expr/i_eval_call 递归下降）；px 函数编译成 C 函数 fn_xxx、调用走 px_call 直接 C 递归；**无堆上显式 VM 帧栈，局部变量即 C 局部量** → VM 层协程（Lua 式帧快照挂起）目前不可行；
4. C 类协程（M:N 用户态协程）只有两条路：**甲 = VM 化后做帧协程（正路，与 VM 化合并）**；乙 = C 栈协程（= 造 Go 早期 runtime + GC 扫 N 条栈 + Windows 换 Fiber，重路）。取甲。

→ 因此 **VM 化独立立项为 M89 旗舰里程碑**；C 类协程不在 M89 主线内混做，视 S1 详勘结论（帧协程是否随 VM 设计自然可得）决定收尾批次并入或另立 M90。

## 二、范围

### 在本里程碑内（M89）
- **S0** 版本升格 0.1.0 → 0.2.0（随本立项执行：源码层 + golden 同步 + 重链 pxc/pxi + 自举证明 + 回归，见 §五 版本策略）
- **S1** VM 化可行性详勘预研（**只侦察写报告，不写码**，约半天级）——产出 `docs/M89_vm_prestudy.md`：
  - 帧栈：现有 codegen（cg_stmt/cg_expr/cg_module）如何搬 → 显式帧；函数调用链/尾调用/闭包捕获的帧表达；
  - 闭包/生成器/异常（Result Err + __prop__ ? 传播）/错误栈如何搬进 VM；
  - 字节码格式选型：栈式 vs 寄存器式；常量池/名字表；与现 C 递归语义的对拍契约怎么定；
  - 精确 GC 改造面：值表示（现原生值透传 + dict 包装 __ufn__/__struct__ 等）→ GC root/帧槽标记的量化；
  - interp.px（pxi 解释路径）与 compiler.px（pxc 编译路径）双轨如何收敛/各自改造量；
  - **工期钉到周级精度**（按 §四·A：先详勘把工期钉到周级再拍板实现批次）。
- **S2** 按 S1 详勘定稿设计 + 实现批次拆分（S 级微步，每步编译 + 相关 verify 通过，不混 commit）
- **S3** 实现
- **S4** 收口（自举证明 + 全量回归 + 重链 bootstrap + 文档同步 + tag）

### 不在本里程碑内（顺延，按 §四·A 顺序）
- C 类用户态协程 M:N（帧协程，VM 稳定后水到渠成）
- native 机器码后端（旗舰 2，VM/IR/GC map 之后）
- Windows 平台（VM + 精确 GC 后变易）

## 三、目标（里程碑级）

1. PuXian 程序执行从「px 函数 = C 函数、C 递归调用」切到「px 函数 = 字节码、显式帧栈 VM 解释」，语义逐字节可对拍（对拍基准 = 现 C 递归路径，先锁语义再动引擎）；
2. 显式帧栈为精确 GC / 帧协程铺路（S1 详勘给出精确改造面量化）；
3. VM 化全程保持自举：compiler.px/interp.px 自身仍用 PuXian 写、仍能自举证明（golden 同步）。

## 四、S 级拆分（预排，S1 详勘后修正）

| S | 内容 | 验收 |
|---|---|---|
| S0 | 版本升格 0.1.0→0.2.0（本批） | pxc/pxi/px --version = 0.2.0；自举证明 rc=0；回归全绿 |
| S1 | VM 化详勘预研（只侦察） | docs/M89_vm_prestudy.md（改造面量化 + 字节码选型 + 工期周级） |
| S2 | VM 设计定稿 + 实现拆分 | 设计文档 + S 清单 |
| S3 | VM 实现（S 微步） | 每 S 编译 + verify；语义对拍 |
| S4 | 收口 | 自举 + 全量回归 + 重链 + 文档 + tag v0.2.0-m89 |

## 五、版本策略：0.1.0 → 0.2.0（S0，2026-09-07 决策）

- **现状**：0.1.0 自 M62 起沿用至 M88b，已覆盖「Rust 退役 + 全自举工具链 + 标准库 + 并发 64 → 1 万+ 连接事件驱动」的巨变，0.1.0 严重低配；
- **升到 0.2.0 而非 1.0.0**：pre-1.0 语义化版本惯例，0.x minor 递增 = 重大架构演进（可含不兼容）；1.0.0 的语义 = 语义冻结 + 向后兼容承诺，须等 VM 化落地、语义稳定后再定（VM 化正是"语义冻结 → IR 诞生"的关口，1.0 留给它之后）；
- **落地范围（S0）**：tools/px SELFHOST_VER、compiler.px PXC_VER、interp.px PXI_VER、pxmcp.px PXMC_VER/PXC_VER_INFO、pxfmt.px banner、runtime/runtime.c server dict 版本串 → 0.2.0；golden/compiler.c 重链同步；bootstrap/pxc + pxi 重链；自举证明 + 回归；tag 从 v0.1.0-mXX → **v0.2.0-mXX**（M89 收口打 v0.2.0-m89）；
- 文档中的历史里程碑表格（README 等 v0.1.0-m70 字样）为历史记录，不动。

## 六、风险与预案

| 风险 | 预案 |
|---|---|
| VM 化体量大（数月级，§四·A 估） | S1 详勘先把工期钉到周级，按 S 微步推进，不混 commit；每步语义对拍 |
| 双轨（pxi 解释 / pxc 编译 C）改造面不一 | S1 量化两条路径，决定收敛策略（可能以 VM 为唯一执行层，pxi/pxc 共享字节码） |
| 自举证明在过渡期易碎 | 任何动 codegen 的 commit 必须同步 golden + 自举证明 rc=0（沿用 M88 收口纪律） |
| 语义漂移 | 以现 C 递归路径为对拍基准，先锁语义（golden cases）再动引擎 |
| 精确 GC 改造面过大 | S1 详勘给量化 + 分阶段（先保守扫显式帧，再精确化） |

## 附：执行状态记录（2026-09-07）

### S0 · 版本升格 0.1.0 → 0.2.0（随立项执行）
> 完成（2026-09-07）：改源码版本串（tools/px SELFHOST_VER / compiler.px PXC_VER /
> interp.px PXI_VER / pxmcp.px PXMC_VER+PXC_VER_INFO / pxfmt.px banner / runtime/runtime.c server dict）
> → golden/compiler.c 重链同步（10596 行）→ bootstrap/pxi 重链 0.2.0（pxi --version = 0.2.0，hello 冒烟 rc=0）
> → 自举证明 rc=0（B.c==golden 10595 行 norm 逐字节）→ 回归 m82 http_serve_unix 8 项全 PASS。命令入口
> px/pxc/pxi --version 均 0.2.0；tag 基线进入 v0.2.0 时代（M89 收口打 v0.2.0-m89）。
>
> ⚠️ 发现并记录：**bootstrap/pxc 二进制内部保持 f77732f(M72) runtime 未重链**。用当前(M88) runtime 重链
> pxc 后，pxc 编 interp.px 确定性崩（"运行时错误 [rust_str_debug 行462]: 字符串索引越界: 0"；--full 全模块
> 与自动裁剪版均复现，pxc 编 compiler.px 正常、pxi(M88 runtime) 正常）→ 疑 M88 runtime 某改动与 compiler
> codegen 组合存在潜在 bug。不阻塞（用户编译路径 tools/px → bootstrap/pxc 为 f77732f-runtime 稳定版，px build
> 全链路冒烟正常），单独立项排查。故 pxc 二进制 --version 内部显示 0.1.0（自举编译器滞后一代），命令入口
> px/pxc/tools 均 0.2.0。

### S1 · VM 化详勘预研
> 完成（2026-09-07）：产出 **docs/M89_vm_prestudy.md**（只侦察未写码）。核心结论：
> ① 现状两执行轨已坐实——编译轨 px 函数=C 函数、C 递归、模块全局每次 px_get_global(锁+哈希)、
> 无堆上显式帧；解释轨 pxi=AST 树遍历、dict 包装值。共享 parser/AST = VM 化最有利资产。
> ② 关键实测：编译轨**无真词法闭包**（fn_closure ctx=NULL、捕获外层局部退化为 px_get_global 取 null、
> 函数内嵌套 def 被整体丢弃；selfhost 编译器 0 个 fn_closure）→ 精确 GC 无 upvalue 需搬，VM 闭包策略 S2 定。
> ③ 字节码选型：寄存器式 3-地址（帧槽=现 _v 编号体系，AST→BC 发射器只换目标不换 parser/AST）。
> ④ 精确 GC 面：17 类型中含引用 7 类；根从整 C 栈保守扫描换帧槽+全局槽+原生桥暂存根（runtime px_call
> 约 20+ 处，估 ≤60 处改造）；保守扫描假存活 + slab 页不还 OS = issue28 堆只涨不落代码级证据。
> ⑤ 工期钉周级：P50 ≈ 10–11 周、P90 16 周封顶（S3 分 A 骨架/B 全构造/C 自举收敛/D 精确 GC 四段可横向分包）。
> ⑥ issue28 分界：先立 issue28 B1/B2 止血批次（3–5 天，保守 GC 内触发策略+slab 归还，不依赖 VM），再进 S2 设计。

### S2 · VM 设计定稿 + 实现拆分
> 完成（2026-09-08）：产出 **docs/M89_vm_design.md**。核心定稿：
> ① 架构：发射器=PuXian 自举（selfhost/bc_emit.px，AST→BCModule），执行器=C（runtime/vm.c），
>   与现 codegen/pxi 共享同一 parser/AST；双轨收敛到「同一 AST→同一 BC→同一 VM」。
> ② 关键决策 D1–D8：统一函数对象（PX_FUNC.fn=px_vm_entry trampoline，ctx=PxVMFunc*，px_call/px_method/
>   px_spawn 零改动兼容旧 C 产物）；显式帧栈（px→px 不回 C 递归，帧=槽数组，深递归安全）；
>   8B 定长寄存器式 3-地址指令集（~45 op spec 定稿）；全局访问=固定槽数组无锁读（issue28 GIL 根治）；
>   错误传播 = TRY 就地解包/Err 即 RET（映射现 err_tag，帧无需 err 字段）；
>   短路 and/or/?? = JMPT/JMPF 分支（返回操作数值，Python 语义）；
>   闭包=真词法捕获 upvalue cell（补编译轨缺口、对齐 spec §6.2，分 P1 无捕获/P2 cell 两期）；
>   方法调用 v1 保 px_method 桥语义、S3-C 静态化优化；精确 GC 根=帧槽+全局槽+原生桥暂存根（≤60 处）。
> ③ S3 拆 A 骨架（2 周）/B 全构造（2 周）/C 自举收敛（2 周）/D 精确 GC（2 周）四级微步清单，
>   含 A/B 对拍 harness（vm_ab.sh：三实现两两对拍）、自举证明切 BC 镜像、issue28 验收门。
> ④ 下一步：S3-A 开工（A0 vm.c/bc_emit.px 骨架 → A1 常量/名字/槽 → ... 逐微步，每步编译+verify）。

### S3 · VM 实现（S3-A 骨架启动）
> A0（2026-09-08）完成：**runtime/vm.h + vm.c 骨架 + bc_emit.px 骨架**。
> - C 侧（runtime/vm.c，A0 引擎骨架）：指令集编号冻结（54 op，PXM_MAX=55，编号一经 S3 定稿即锁）；
>   显式帧栈 PxVmState/PxFrame/PxVMFunc + 帧槽数组生命周期；D2 trampoline px_vm_entry（LXFuncPtr 兼容，
>   ctx=PxVMFunc*，px_call 零改动）；解释循环最小子集已跑通（LOADK/IMM/MOV/SRCLINE/JMP/JMPT/JMPF/RET/RET0/
>   HALT），其余 op 分发默认 px_error「指令未实现」（A1 起逐批）；临时冒烟 7 例 ALL PASS（含 JMPT/JMPF 分支、
>   LOADK 走模块 K 池）；`gcc -O2 -pthread -Wall -Wextra -c` 零告警。
> - 发射器（selfhost/bc_emit.px，A0 骨架）：BCModule 目标形态（k_pool/n_pool/globals/funcs/top）+ 表构建
>   工具（bc_k_add/bc_n_add/bc_g_add/bc_new_module）+ emit 入口占位；`px parse` 通过。
> - 集成说明：vm.c 暂未入 rt_src_files（A1 首个发射切片/px run --vm 接入时连同 rt 缓存一次重建）；
>   compiler.px 的 `bc` 子命令随 A1 首个发射切片接入（golden/自举再生成同批，避免重复 3.5 分钟全量重链）。
> - 回归：hello_a0（while 求和）px build + px run 双轨一致（sum=10，rc=0），仓库可编译、工作区干净。
> - 下一步：A1 常量/名字/槽编号 —— 发射器 K/N/G/Func 收集 + LOADK/IMM/MOV/GETG/SETG/SRCLINE/RET/RET0/HALT
>   全指令接线（compiler.px `bc` 子命令 + golden 再生成）。

### S3-A · A1 常量/名字/槽编号（发射器首个真实切片）
> 完成（2026-09-08）：**发射器 K/N/G/Func 收集 + A1 指令集全接线 + vm GETG/SETG/Top 运行**。
> - **selfhost/bc_emit.px**（A0 骨架 → 首个真实发射切片）：K 池按值去重（int/float/str/bool/null）、
>   G 池全局名（源码序对齐 cg_generate）、N 池预留；**函数级槽分配对齐 _v 体系**（参数 0..arity-1 +
>   hoist 局部预占 —— 帧槽 calloc 零=PX_NULL 免初始化指令（M62-L5 对齐）—— + 临时单调分配，A1 不回收）；
>   语句/表达式 A1 子集接线：VarDecl/Assign(Var)/Return/ExprStmt/If/While/Empty + Int/Float/Str/Bool/
>   Null/Var + Unary 负字面量折叠 → LOADK/IMM/MOV/GETG/SETG/SRCLINE/RET/RET0/HALT，if/while 用
>   JMPF/JMP 相对跳转回填（off 相对下一条）；顶层=合成 top 函数（HALT）、顶层 VarDecl/Assign=SETG、
>   函数内 VarDecl=局部槽遮蔽同名全局、Assign 未声明名=hoist/全局（逐条对齐 codegen 语义）；
>   不支持 tag panic「未实现」（A2+ 逐批）；bc_dump_module 文本 dump。
> - **验证**：cases_bc/bc1.px → bc1.dump（golden，83 行）核对全对 —— K 去重（"hi" 双引用仅 1 条、
>   int16 内走 IMM 不进 K）、G 源码序 11 项、greet/double/classify/countdown 槽号/跳转 off/IMM -1
>   负常量折叠、top 全 SETG 链路 + HALT 逐条与手推一致。
> - **runtime/vm.c + vm.h**（A1 扩展）：GETG/SETG 执行（v1 经 px_get_global/px_set_global，D8
>   无锁化后置）；px_vm_run_module 完整 Top 运行（注册非 top 函数 px_func(name,px_vm_entry,
>   &funcs[i]) D2 trampoline + 跑 Top bc）。自测 examples/m89_a1/vm_selftest.c + vm_selftest_run.sh：
>   手写 PxBCModule 冒烟 **ALL PASS** —— Top SETG answer=42 后 px_get_global 可取 42；
>   px_call(read) 经 GETG 返回 42（函数注册+trampoline 通）；外部 px_set_global(7) 后 GETG v1 读到 7。
> - **工程决策（偏离 A0 预排，记录）**：新增 **bc_cli.px 独立验证壳**（import codegen.px+bc_emit.px、
>   主文件声明全套全局，main=lex/parse/resolve→emit→dump）。`compiler.px bc` 子命令 + golden 再生成
>   **推迟**到首个可运行字节码闭环（A2/A3 算术/CALL 接入时）——避免 codegen 全链重编（~6 min/次，
>   pxc M72 解释 codegen 生态）烧 2 次；届时 bc_emit 并入 compiler.px 随自举 golden 同批。vm.c 仍
>   未入 rt_src_files（同上时机，一次重建 rt 缓存）。bc_cli 编译：bootstrap/pxc build → C 存
>   selfhost/build/bc_cli.c → gcc 链接 rtcache 全 .o（build/ gitignore 不入库）。
> - 回归：compiler.px/import 链零触碰（自举证明不受影响）；vm.c 独立编译零告警；仓库可编译。
> - 下一步：A2 算术/比较（PXOP_NEG..PXOP_GE 批 + 发射器 Binary/Unary 映射 cg_binop_cname +
>   首个可计算程序端到端跑通 → compiler.px bc 接入/rt 缓存重建同批）。

### S3-A · A2 运算批 + 顶层顺序执行（发射器→可链接 C→VM 端到端闭环打通）
> 完成（2026-09-08）：**首个可运行字节码闭环** —— BCModule 静态 C 输出 + gcc 链接 rtcache 跑 Top。
> - **runtime/vm.c**：B 表一元/二元运算全量执行（NEG/NOT/BITNOT + ADD..GE + BITAND..SHRU，
>   语义=调现 px_* C 函数，错误由 px_* 保证）；**tools/px rt_src_files 加入 vm.c/vm.h**
>   （A1 预留接入时机；rtcache 重建含 vm.o，px build 正常、hello C 产物与 pxi 输出一致回归 PASS）。
> - **selfhost/bc_emit.px**：表达式 Binary（算术/比较/位运算，两地址 ADD dst,dst,s2）与 Unary 一般化
>   （Neg/Not/BitNot → NEG/NOT/BITNOT；负字面量折叠保留）接线；新增 bc_binop_op/bc_unop_op 映射；
>   **emit-c**（bc_emit_c_program）：BCModule → 可链接 C 静态初值（PxK/N/G/bc/funcs/mod，mod 前向
>   引用）+ main 跑 px_vm_run_module（PX_BC_DUMP=1 打印非函数全局，A 阶段对拍用）。
>   **bc_cli.px 增 --emit-c**。examples/m89_a2/bc_run.sh（emit-c→gcc 链 rtcache→跑）。
> - **修复 A1 遗留 bug**：bc_emit_if 有 else/elif 时 JMPF 假值落点错（落在 then 尾 JMP 上 → else 永不
>   执行）——调整为假值落 JMP 之后（else 起点）；bc1.dump golden 回归不变。
> - **验证**：cases_bc/bc2.px（+bc2.dump golden）覆盖 Add/Sub/Mul/Div/IntDiv/Mod/Pow/比较/位/一元/
>   if-else/while；emit-c 端到端跑 bc2 顶层全局值 **16 项断言 ALL PASS**（arith=13、idiv=3、modv=1、
>   powv=256、bita=2、bito=5、bitx=4、shlv=16、negv=-7、notv=true、cmp1/cmp2/cmps、scon=abcd、
>   acc=10、res=big=if-else 真分支正确）；bc1 emit-c 跑通（GREETING/counter=42/big/pi/enabled/
>   missing/again 全对）。examples/m89_a1/vm_selftest 复跑 ALL PASS。
> - 工程决策（记录）：compiler.px bc 子命令并入仍推迟（S3-C 自举收敛统一做）；A2 以 bc_cli 独立壳 +
>   PX_BC_DUMP 顶层值断言验证（无 CALL/print，stdout 对拍留 A4 print 可用后）。A2 端到端闭环已立，
>   A3-A5 不再重编 compiler/rt 即可逐批接入。
> - 下一步：A3 控制流（短路 and/or/?? + IfExpr + break/continue；for-in 依赖迭代器/容器见 S3-B）。

### S3-A · A3 控制流批：短路 and/or/??、IfExpr、while+break/continue
> 完成（2026-09-08）：**纯发射器扩展**（VM 无需新 op —— 短路/分支全部映射到既有
> JMPT/JMPF/EQ/LOADK）。验证仍走 bc_cli emit-c 端到端（无 CALL，A4 接入 print/函数调用）。
> - **selfhost/bc_emit.px**：Binary And/Or 短路（D5：左值入 d → JMPF(and)/JMPT(or) 跳过右式，
>   d 保留左值或右值覆写——返回操作数 Python 语义）；NullCoalesce ??（EQ d,null → JMPF 非 null
>   保留，仅 null 替换）；IfExpr（JMPF→else/JMP→end 分支）；stmt Break/Continue（func.loops 循环
>   上下文栈；break=JMP end、continue=JMP cond 起点，body 发完回填）；bc_emit_while 重构支持
>   break/continue（loop ctx push/pop，breaks→end、conts→loop_start）。
> - 验证：bc1/bc2.dump golden 回归不变；cases_bc/bc3.px+dump golden 覆盖 and/or/??/IfExpr/
>   while+break+continue；emit-c 端到端 **11 断言 ALL PASS**（a1=0 a2=7 o1=9 o2=3 n1=42 n2=0
>   n3=x ie1=yes ie2=no sum=25（continue 跳 3）+flags=5（break 停））。
> - 下一步：A4 调用闭环（CALL/参数区/默认参数/RET 嵌套帧 + 发射器 Call/main 调用约定 + vm.c
>   解释循环嵌套帧改造：VM 内 px→px 压帧不回 C 递归（D3），native/旧 C 经 px_call）。

### S3-A · A4 调用闭环：CALL/嵌套帧/递归深链/main 调用约定（hello.px 三轨对拍通过）
> 完成（2026-09-08）：**函数调用闭环打通** —— px→px 调用压显式帧不回 C 递归（D3），
> native/旧 C 产物经 px_call（C 递归一层，与 CPython 同构）；混合（旧 C fn 调 VM 函数/
> VM 调旧 C fn）由 PX_FUNC.fn 指针相等判断天然兼容。
> - **runtime/vm.c + vm.h**：PxFrame 增 ret_dst（CALL 压帧 → 返回写 caller 槽；-1=顶层）；
>   vm_frame_push 带 ret_dst；RET/RET0 弹帧回传（nframes 回 base 才返回调用者）；
>   **CALL** 分派——callee 是 PX_FUNC 且 fn==px_vm_entry → 手动压帧（递归深度走 VM 帧栈，
>   深递归安全）；PX_NATIVE / 旧 C 编译产物（fn!=px_vm_entry）→ px_call 直调。参数连续区
>   槽 b+1..b+argc（发射器预留防临时碰撞）；argc<arity px_error（默认参数 S3-B）。
> - **selfhost/bc_emit.px**：抽 **bc_emit_call**（callee 槽 + 实参连续区预留 + CALL）；表达式
>   Call/Pipe（|>：左值作首参调右函数，含 f(v,...) 形态）接线；**main 调用约定**——Top 末尾
>   检测 has_main → GETG main→CALL(argc=0)→RET（Top 返回 main 结果给 run_module）；
>   emit-c driver main 按 codegen 语义转退出码（Result Err→stderr+1；Ok/Int→code）。
> - **验证**：cases_bc/bc4.px+dump golden（add 多参/fib 递归/deep 2 万层深链/main 调用），
>   emit-c 端到端 **main 退出码=63**（add(3,4)=7+fib(10)=55+deep(20000)=7-6）ALL PASS；
>   嵌套帧 CALL 手测（top→outer→inner 返回 42）PASS；bc1-4 dump golden 回归不变；
>   **examples/hello.px 三轨 stdout 逐字节一致（VM=旧 C codegen=pxi）**——main 调用 + 局部 +
>   and 短路 + if/else + print(native) + Pipe(msg |> to_upper()) 全链路真实程序跑通。
> - 决策记录：默认参数/CALLM 方法调用推迟 S3-B（容器/方法到来时随 NEWSTRUCT/px_method 桥
>   一起；A4 无容器无方法调用消费方）；for-in 依赖 len/index 见 S3-B。
> - 下一步：A5 错误（TRY/FORCE + 顶层 ? 语义 + SRCLINE 现场比对 px_error 文案）——需
>   Ok()/Err() native 构造 Result（GETG→CALL native 已可用），A4 CALL 已就绪。

### S3-A · A5 错误传播：TRY/FORCE + 现场追踪（S3-A 骨架段收口）
> 完成（2026-09-08）：**错误传播接线 + VM 错误现场对齐 px_error 文案**（D7）。
> - **runtime/vm.c**：PXOP_TRY（?）——Result-Err/null → 就地返回（RET 语义回传 caller
>   槽；顶层帧 → 返回 run_module → driver 报错退出）；Ok → 就地解包覆写槽。PXOP_FORCE
>   （!）——Err/null → px_error（"force unwrap Err: <载荷>" / "force unwrap null"）；Ok 解包。
>   **现场同步**：解释循环每帧迭代 px_srcfunc(函数名)、SRCLINE 指令 px_srcline(行) →
>   runtime px_error 文案格式 "[函数 行N]: ..."（M72-S2 对齐，实测
>   "运行时错误 [main 行3]: force unwrap Err: boom"）。
> - **selfhost/bc_emit.px**：表达式 Try（?）→ TRY、ForceUnwrap（!）→ FORCE（求值入 d →
>   TRY/FORCE d）。顶层 ? 语义 = Err/null 经 RET 到 run_module → driver 报错退出（对齐
>   codegen "顶层不能传播" 报错意图；文案差异记录）。
> - **验证**：cases_bc/bc5.px+dump（Err("除零") 经 use_div? → main? 两跳就地传播）→ emit-c
>   退出码 1 + stderr "错误: 除零" PASS；bc6.px+dump（get_val()! = 7 FORCE Ok 解包 +
>   ret_null()? null 就地传播 → main RET null → exit 0；若 null 未传播则 exit 8 区分）PASS；
>   FORCE Err 现场 "运行时错误 [main 行3]: force unwrap Err: boom" PASS；bc1-4 dump golden
>   回归不变；bc2/3/4 verify + hello 三轨复跑全绿。
> - **S3-A 段收口**：A0 骨架 → A1 常量/槽 → A2 运算+顶层闭环 → A3 控制流 → A4 调用闭环 →
>   A5 错误传播 全部完成；hello.px 三轨（VM=旧 C=pxi）stdout 逐字节一致；fib.px 需容器
>   （list/dict/range/推导式）→ S3-B。A 段门（cases s01-s07+hello/fib）中 fib 随 S3-B 补齐。
> - 下一步：S3-B 全构造（容器 NEWLIST/NEWTUPLE/NEWDICT/INDEX/SETF/字段 GETF/SETF、方法
>   CALLM、for-in、推导式、闭包 P1、生成器/并发桥）。

### S3-B · B1 容器/字段/方法桥/For 迭代（S3-B 首切）
> 完成（2026-09-08）：**C 表容器/字段 op 全接线 + 发射器字面量/索引/切片/字段/方法/For**。
> - **runtime/vm.c**：NEWLIST/NEWTUPLE/NEWDICT（自连续槽拷贝建容器；dict 仅 str 键入池对齐
>   codegen）、INDEX/SETIDX/SLICE、GETF/SETF/GETF_OPT（px_field/px_field_set，dict 字段=
>   px_field→dict_get）、CALLM（方法桥 px_method：a=dst,b=obj,c=N 名,fl=argc，args=槽 b+1..
>   b+argc）。NEWSTRUCT/NEWENUM 待 B2 类型元数据表。
> - **selfhost/bc_emit.px**：① 指令升 5 元组 [op,fl,a,b,c]（fl 预留，CALLM 用 argc；dump/C
>   输出跳过 fl → bc1-6 golden 文本不变，实测回归一致）；② 表达式 List/Tuple/Dict 字面量 →
>   NEWLIST/NEWTUPLE/NEWDICT、Index→INDEX、Slice→SLICE（缺省界置 null）、Field→GETF、
>   OptionalField→GETF_OPT、方法调用 obj.m(args)（callee Field）→ CALLM；③ Assign 目标
>   扩 Var/Index/Field + 复合赋值（读旧值→运算→写回）+ `a <- rhs` Append（→ append 方法桥）；
>   ④ For 迭代展开（对齐 cg：CALL len → 计数器 → INDEX 取元素 → body → 计数+1；break/
>   continue ctx 回填，continue=跳回增量后重判 cond）；⑤ hoist 收集补 For 循环变量 + For
>   body 递归。Field 的 enum/const enum 折叠、struct 构造调用 → B2 类型元数据表接入。
> - **验证**：cases_bc/bc7.px+dump golden（list/tuple/dict 字面量、索引读写、切片、str 索引、
>   dict 字段 GETF、CALLM list.push/str.to_upper、For 迭代 dict、局部/全局复合赋值）——
>   **bc7_verify.sh 13 断言全 PASS**；**m89_b1_parity.px 容器双轨对拍（VM=旧 C codegen）
>   stdout 逐字节一致 PASS**（total=112/sl=[99,3]/d.a/d.b/s=HI/len=5）；bc1-6 dump golden
>   不变 + bc2-6 verify + hello 三轨复跑全绿。
> - 决策/记录：bc_run.sh 自动探测 rtcache 改「按 mtime 最新含 vm.o」（避免命中 vm.c 改动前
>   旧 vm.o）；VM 帧槽尚未入 GC 根面（保守扫描面外，A/B 测试规模安全；S3-D 段切精确根）；
>   bc_emit 不做 M41.2 不可变检查等静态语义校验（语义校验走 compiler/codegen，VM 侧收口时对齐）。
> - 下一步：B2 构造/类型（NEWSTRUCT/NEWENUM + struct/enum/const enum 类型元数据表 +
>   impl 方法注册 "Type.method" → G 槽）。

### S3-B · B2 构造/类型：NEWSTRUCT/NEWENUM + struct/enum/const enum/impl 方法注册
> 完成（2026-09-08）：**类型系统三件套（struct/enum/const enum）在 VM 上跑通 + impl 方法
> 注册为全局 "Type.method"（px_method struct 桥按名转发）**。
> - **runtime/vm.h**：PxBCModule 增 struct 类型元数据表（PxStructDef {name, fnames, nfields}，
>   NEWSTRUCT 运行时字段名来源）；vm.c 实现 **NEWSTRUCT**（a=dst, b=structs 元数据 idx,
>   c=字段值连续槽基址 → px_struct，同 codegen 构造语义）与 **NEWENUM**（a=dst, b=N 类型名,
>   c=N 变体名 → px_enum）；px_vm_run_module 注册全局函数跳过 name 以 '<' 开头的闭包
>   （B4 起 closure 不注册全局，LOADK PXK_FUNC 直接引用）。
> - **selfhost/bc_emit.px**：① 类型元数据收集 —— StructDef → structs（字段声明序）、
>   EnumDef → enums（变体表）、TypeConst 递归收集（顶层+函数体+嵌套块，对齐 cg_collect_consts，
>   值=表达式，访问处内联发射而非编译期常量）、impl 方法列表按 "Type.method" 字典序
>   （对齐 codegen M-B6 确定性排序）；② Field 编译期折叠 —— const enum（LogLevel.Info →
>   值表达式内联）与 enum 变体（Color.Red → NEWENUM，obj 不求值）优先于 GETF；③ 构造
>   Call/Constructor（callee Var 为类型名）→ struct/enum 构造；④ impl 方法以注册名
>   "Type.method" 作为普通函数（self=首参）发射并入 funcs（px_vm_run_module 统一注册）；
>   ⑤ emit-c 输出 s_structs 元数据表 + s_mod .structs/.nstructs；⑥ top_stmts 过滤补
>   TypeConst、bc_emit_stmt 补 TypeConst 跳过。
> - **验证**：cases_bc/bc8.px+dump golden —— **bc8_verify.sh 11 断言全 PASS**（struct 构造
>   NEWSTRUCT/字段读 GETF/字段写 SETF/impl 方法 Point.sum CALLM→px_method struct 桥（self
>   绑定）/enum 变体 NEWENUM + px_eq enum 相等比较/TypeConst 折叠 LogLevel.Error=3）；
>   **bc8.px 旧轨 pxi 同跑 rc=0 语义一致**；bc1-7 dump golden 全不变 + bc7_verify 复跑全绿
>   （B1 无回归）。
> - 记录/决策：NEWSTRUCT 用 structs 元数据下标（非 N 名）作 b 操作数——字段名表须随模块
>   静态生成；enum 构造仅支持 Color(Variant)/Color("Variant")（payload enum 未覆盖，遇之
>   panic 未实现，不静默错）；const enum 值=表达式内联（对齐 codegen，非编译期常量折叠）；
>   TypeConst 语法为括号形式 `type X const (A = 1, ...)`（非缩进块，parser M44 语法）。
> - 下一步：B3 推导式/match/生成器（ListComp/GenExp/DictComp → 循环展开或 NEWGEN；
>   match/select 模式匹配展开；cases s04/s13 + m34_gen_lazy/m25 系列）。

### S3-B · B3a 推导式/生成器/闭包基建（ListComp/DictComp/GenExp/Closure/Block）
> 完成（2026-09-08）：**闭包/块表达式基建 + 三类推导式在 VM 跑通（bc9 ALL PASS）**。
> - **runtime/vm.h/vm.c**：PxK 增 PXK_FUNC（i=funcs 下标，LOADK 物化为 PX_FUNC(px_vm_entry,
>   &funcs[i])——非全局注册函数的引用）；vm_loadk 增 mod 参数；实现 **NEWGEN**（a=dst,b=seq
>   槽,c=2 连续槽基址[transform(PX_FUNC),filter(PX_FUNC|null)] → px_gen_lazy，对齐 codegen
>   单 for GenExp：elt 恒为 transform 闭包）。px_vm_run_module 跳过 '<' 名全局注册（B2 已加）。
> - **selfhost/bc_emit.px**：① lambda 基建 bc_emit_push_lambda（无捕获单参/多参闭包
>   "<closureN>" 入 funcs 返回下标）+ K 池 func 登记（dump/emit-c 输出 func 常量）；② Closure
>   表达式 fn(params) body → LOADK PXK_FUNC；③ **Block 块表达式**（{ stmts }：顺序执行，
>   值=最后 ExprStmt，块内 VarDecl/Assign 就地分配槽——无块级作用域对齐 codegen/M62-L5）；
>   ④ 推导式嵌套循环展开 bc_emit_comp（ListComp push / DictComp rv.set(k,v)，多子句递归，
>   迭代变量重绑定 smap 保存/恢复——comp 变量不泄漏，对齐 cg_comp_collect/restore；cond
>   JMPF 过滤）；⑤ GenExp 单 for 单变量 → NEWGEN（seq + transform 闭包 fn(x){elt} + filter
>   闭包 fn(x){cond} 或 null；多 for/多变量 panic 未实现）；⑥ bc_emit_methodcall_slot 修正
>   （obj 引用先 MOV 到新 cslot，实参区全新预留——dictcomp set 暴露原实现 obj 槽后排布与
>   既有活跃槽重叠覆写）。
> - **验证**：cases_bc/bc9.px+dump golden —— **bc9_verify.sh 12 断言全 PASS**（ListComp 平方
>   列表 len/元素、ListComp+cond 偶数过滤、DictComp {k: k+"!"} str 键、GenExp (x*10 for..) 惰性
>   NEWGEN + for-in 迭代物化 transform 闭包、Closure fn(a){a+5}(Block) LOADK PXK_FUNC 调用）；
>   bc9 旧轨 pxi 同跑 rc=0 语义一致；bc1-8 dump golden 全不变（无闭包/推导式 case 零影响）。
> - 记录：comp 迭代变量多变量子句（解构 for）未实现（panic）；GenExp 仅单 for 单变量
>   （多 for 物化路径未接）；dictcomp/comp dict.set 键假定 str（对齐 NEWDICT 仅 str 键入池）；
>   Closure 为无捕获 P1（body 引用宿主局部 → 按全局处理报未定义，与 codegen fn_closureN 同构；
>   真捕获 upvalue cell = P2 留 S3-C）。修复过程中发现并解决 bc_emit_expr 内 let si(Index 分支)
>   与 var si(Block 分支) 同名触发 M41.2 E3002（循环变量改名 bi2）。

### S3-B · B3b match 模式匹配（enum 变体/literal/wildcard/guard）
> 完成（2026-09-08）：**match 表达式在 VM 跑通（bc10 ALL PASS）**。
> - **runtime**：新增 px_enum_variant（enum→px_str(variant)、非 enum→null——对齐 cg
>   subject.type==PX_ENUM && strcmp 短路，非 enum 不报错）；vm.h 增 PXOP_ENUMVAR（55，
>   PXM_MAX→56：a=dst,b=obj 槽）+ vm.c 实现。
> - **selfhost/bc_emit.px**：Match 表达式 → if-elif 链（d 初=subject，命中 arm body 覆写 d
>   并跳 end，全不命中返回 subject——对齐 cg Match）；模式条件 bc_match_cond：PatConstructor/
>   大写 PatBinding → ENUMVAR(subject) == "变体名"（str EQ）；PatLiteral → EQ(subject, 字面量
>   表达式，含 Field enum 折叠）；PatWildcard/小写 PatBinding/空 PatTuple → 恒真（null 无跳转）；
>   guard（arm[2]）与 pattern cond 组合（均命中才进 body）。
> - **验证**：cases_bc/bc10.px+dump golden —— **bc10_verify.sh 6 断言全 PASS**（def 内 match
>   enum 变体 Red/Green + wildcard 兜底、顶层 match literal 3/1 + wildcard）；bc10 pxi 同跑
>   rc=0 语义一致；bc1-9 dump golden 全不变零回归。
> - 记录：PatBinding 小写（绑定语义）恒真不绑定变量（对齐 codegen 现状，P2/后续可补）；
>   guard 支持已加（codegen 侧忽略 guard 是缺口，VM 侧先行正确实现，记录差异待收敛）。
> - 下一步：B4 闭包 P1 收口（m25_closure_gc 对拍 + 捕获面扫描）→ B5 并发/IO 桥
>   （spawn/chan/send/recv/select/ffi 构造）。

### S3-B · B4 闭包 P1 收口（无捕获闭包 + m25_closure_gc 对拍）
> 完成（2026-09-08）：**闭包基建（B3a 落地）经真实用例对拍收口 P1**。
> - Closure 表达式 → LOADK PXK_FUNC 引用 funcs 内 "<closureN>"（px_vm_entry trampoline，
>   px_call 零改动）；Block body 支持 return 语句（RET 提前返回，尾 RET0 死代码无害）。
> - **验证**：**examples/m25_closure_gc.px 在 VM 跑通** —— bc_run.sh emit-c → gcc → 运行：
>   "M25 CLOSURE GC TESTS PASSED"（rc=0）—— 覆盖闭包自引用循环（let g=null; g=fn()..）、
>   for i in range(300) 大批闭包垃圾 + gc() 强回收、活跃闭包 k()==42 未被误回收、assert 断言。
> - 捕获面扫描结论（P2 时机）：m25/自举编译器路径 0 捕获；examples 闭包用例（match.px 的
>   fn(x){x*2}、m25、m34 gen_lazy transform/filter）均为无捕获或单参 lambda —— **P1 覆盖现
>   存量用例，真捕获（upvalue cell）P2 按计划留 S3-C**（spec §6.2 语义为终目标）。

### S3-B · B5 并发/原语桥（spawn/chan/send/recv/select + 关键 bug 修复）
> 完成（2026-09-08）：**VM 并发桥打通（bc11/bc12 ALL PASS）**，并修复一个**跨轨 runtime bug**。
> - **runtime**：新增 **px_spawn_ctx(fn, ctx, args, nargs)**（spawn_thread 透传 job->ctx；
>   px_spawn=ctx NULL 包装）；**px_spawn_name 透传 PX_FUNC 的 ctx** —— 修复致命缺陷：
>   原 px_spawn_name 只传 fn 丢 ctx，VM PX_FUNC(px_vm_entry, ctx=PxVMFunc*) 起线程后
>   px_vm_entry ctx=NULL 直接返回 → spawn 的 worker 静默不执行 → 主线程 chan.recv 死等
>   （codegen 旧 C 产物 ctx 无关不暴露；VM 首暴露，跨轨 runtime 层修复）。新增 px_enum_variant
>   （B3b）。VM 启动注册 5 个原语 native：chan(cap)/mutex()/rwlock()（构造）、spawn(fname,args..)
>   （px_spawn_name 入口）、chan_try_recv(ch)（select 用；命中=收到值、未命中=null）。
> - **selfhost/bc_emit.px**：语句 tag —— ChanDecl（chan() 构造）、Send ch,v（ch.send）、Recv ch
>   （ch.recv 丢弃）、Spawn f(args)（→ spawn("f",args..)）、Select（顺序 try_recv 每 arm +
>   bind 绑定命中值 + body；全未命中 → else（若有）否则忙等重试对齐 cg retry；null 消息边缘
>   误判记录）。chan/mutex/rwlock 构造表达式 = 普通 Call（GETG 注册 native）。
> - **验证**：bc11.px（spawn 2 worker + chan(4) + send/recv，main 收两值求和）——
>   **bc11_verify.sh PASS（"sum: 6"）**；bc12.px（worker spawn + select ch.recv 绑定 x）
>   —— **bc12_verify.sh PASS（"sel: 6" + "done"）**；bc1-10 dump golden 全不变零回归。
>   旧轨 px build bc11 运行对照 stdout（codegen chan/spawn 支持）——见提交记录。
> - 记录：pxi（Mini 子集）不支持通道（R1002），bc11/bc12 对拍走旧 C codegen 轨而非 pxi；
>   select 无 else 忙等（高 CPU，对齐 cg retry）；真 null 消息经 chan_try_recv 判未命中
>   （边缘局限记录）；select 随机化公平未做（顺序尝试，语义选择任一就绪 arm 仍成立）；
>   mutex/rwlock/ffi_call/http_serve 语句构造已由 Call 桥覆盖（chan_try_recv 等 native 已注册），
>   专项用例验证随 S3-B 门（vm_ab v2 + examples 并发对拍）推进。
> - **S3-B 段完成状态**：B1（容器/字段/方法/For）✓ B2（构造/类型）✓ B3（推导式/match/
>   生成器）✓ B4（闭包 P1）✓ B5（并发/原语桥）✓ —— 微步全落地；**里程碑门**（cases 全量 +
>   examples 全量 VM vs 旧轨 stdout 一致 vm_ab.sh v2）待 S3-C 收口时补全量 harness 回归
>   （本段以 bc1-12 自建用例 + 代表性对拍为验证基准，见 §七 S3-B 门）。

### S3-C · C1 自举：runtime 内嵌 NUL 修复 + 编译器全量 BCModule 发射打通（2026-09-08 启动）
> C1 目标（docs/M89_vm_design.md §七 S3-C）：compiler.px（+bc_emit 自身）在 VM 上编译自身
> → 字节码镜像；与现 C 递归产物编译结果对拍一致。
> - **第一步（本批）—— 前置阻塞清除 + 发射完备性证明**：
>   - runtime 内嵌 NUL 字符串 bug 修复（见 CHANGELOG M89-S3-C1）：px_index/px_slice 越界
>     检查与单字符结果构造的 strlen 截断 = S0 记录 "pxc 重链后编 interp.px 崩 rust_str_debug
>     行462" 的真凶；修复后 bc_cli 处理 pxlexer.px/compiler.px exit 0。
>   - **bc_emit 对编译器全量发射成功**：compiler.px 全 import 链（codegen/parser/pxlexer/
>     cg_stmt/cg_expr/cg_module/bc_emit，~4700 行合并源码）→ BCModule dump 21456 行 exit 0，
>     173 funcs / 226 globals / 809 K —— 语言覆盖达自举完备，零未实现 panic。
>   - 基线入库：selfhost/golden/compiler.bc.dump（382KB，重复运行逐字节一致）。
>   - 回归：examples/m89_c1/nul_str_verify.sh PASS + bc4/8/11 + hello 三轨 + B1 parity 全绿。
> - 记录：bc_emit 对 compiler.px 发射耗时 ~6 min（C 递归引擎解释 4700 行编译器源码的固有
>   成本；后续 VM 镜像重放会更慢，性能优化点：全局/K 池线性扫描可 hash 化——留待 C1 对拍
>   通过后按需做）；codegen 对字面量 NUL 的 C 串截断是编译轨固有局限（运行期串字节安全，
>   不受影响）。
> - 下一步（C1 对拍）：bc_cli --emit-c bc_cli.px → gcc → VM 驱动；跑 compiler.px 重放
>   BCModule dump，与 golden/compiler.bc.dump 逐字节对拍（VM 编译器编译自身的第一证明）。

### S3-C · C1 自举 — OOM 阻塞记录（2026-09-08 08:0x，网关连带重启）
> 事件：C1 自举验证中 bc_cli（VM）全量执行 compiler.bc 两次 OOM（dmesg 07:55 rss 6.7GB、
> 08:04 rss 7.1GB，机器 7.8GB）→ 内核杀 bc_cli → 该进程在 ws-supervisor.service cgroup 内
> → systemd 判 unit oom-kill → 整个 ws-supervisor 重启（QQ 网关/memory-service 连带，08:06 恢复）。
> 技术归因：runtime 为保守标记-清除 GC（M8/M11，根=线程栈/寄存器+全局表+g_tmp_root）；
> **VM 帧槽（PxFrame.slots）在堆上，不在 GC 根集**（vm.c 头注自认"S3-D 前不纳入 GC 根面"）
> → VM 大规模执行 compiler.px 全链（对象百万级）时活跃对象对 GC 不可见/栈残留保守误标，
> 内存只增不减 → OOM。= 设计预告的 S3-D「帧槽根切换」技术债被 C1 提前引爆。
> 决策（记录）：C1 剩余"VM 跑 compiler.bc 自举证明"前置 = **VM 帧槽 GC 根止血（S3-D 切片）**：
>   runtime.c 增外部根注册（px_gc_add_roots，单线程/并发根扫描统一纳入）+ vm.c 把 PxVmState
>   活跃帧槽区间注册为根 → 全量重链 + bc1-12/hello/compiler dump 回归 → 再跑 C1 自举证明。
> 防护（立即生效）：此后一切 VM/编译任务命令行前缀 `ulimit -v 2500000`（2.5GB 快速失败，
>   不再全局 OOM 连带网关）；根治后才放开。

### S3-C · C1 自举 — OOM 解除 + 自举证明 PASS（2026-09-08 本机 dongyue 重放）
> S3-D 止血（96a8e4b，帧槽 GC 根）落地后于本机复跑 C1 自举对拍 → **PASS**。
> - **流程（对齐 3dfbb04 记录下一步）**：
>   ① 旧轨复核：bc_cli dump compiler.px（~6 min）== golden/compiler.bc.dump 逐字节一致
>     （本机工具链与权威基线一致确认）；旧轨 bc1-12 dump golden 全量回归全绿。
>   ② bc_cli --emit-c bc_cli.px → bc_cli_vm.c（990,651 B，VM 字节码镜像静态 C）→ gcc 链
>     rtcache（含 vm.o，S3-D）→ /tmp/bc_cli_vm（VM 驱动版编译器；冒烟 bc1 dump==golden）。
>   ③ VM 驱动版跑 compiler.px 重放 → dump **21,456 行 / 382,650 B，与 golden/compiler.bc.dump
>     逐字节一致（diff 空）** —— VM 编译器编译自身第一证明 PASS（rc=0）。
> - **S3-D 止血实证**：VM 重放 compiler.px 全链全程 rss ≤ ~0.5 GB（帧槽入 GC 根后内存受控，
>   对比 OOM 记录 7.1GB 泄漏态），ulimit -v 10GB 内完成、无泄漏。
> - **内存记录（新发现）**：emit-c bc_cli.px 走**旧 C 引擎**（非 VM），虚拟内存高水位 >6GB
>   （slab 页不还 OS + emit-c 全链大对象/大 out 字符串，issue28 技术债）→ ulimit 2.5GB/6GB
>   均被 malloc 拒杀，**≥9.5GB 才完成**（本机 16G，非物理 OOM）。VM 路径则受控。
> - 本机环境注：缺 glibc static（-static 链接报 cannot find -lm/-lc）→ 本地验证链接去 -static。
> - 下一步：C2 golden 切换（golden/compiler.c → BCModule 镜像；自举证明规则更新）+ S3-C 收口
>   门（vm_ab.sh v2 examples 全量 VM vs 旧轨 stdout 对拍）。

### S3-C · C2 golden 切换前奏：compiler.px 主链路并入 bc_emit（2026-09-08）
> 完成（C2-1，dongyue）：VM 化自举收敛 A1 决策"bc_emit 并入 compiler.px 随自举 golden
> 同批"落地 —— 编译器正主（compiler.px/pxc 源码）获得 BC 发射能力，不再依赖姊妹壳 bc_cli。
> - **selfhost/compiler.px**：import bc_emit.px + 声明 g_bcm + bc_basename + main 支持
>   `bc`（BCModule dump）/`--emit-c`（emit-c 静态 C）子命令；默认 C 文本（cg_generate）
>   路径零改动 → px build / pxc 旧用法完全兼容。
> - **golden 双轨同步**：compiler.px 源码有意变更 → ① golden/compiler.c = pxc 编
>   compiler.px 新产物（14935 行，并入 bc_emit 生态后增大）② golden/compiler.bc.dump =
>   compiler.px 含 bc_emit 的新字节码镜像（30315 行，原 21456 → 含发射器自身）。
> - **证明链**：bootstrap_prove rc=0（B.c==golden/compiler.c 逐字节）；compiler_new
>   （= 新 compiler.px 的 C 引擎编译产物）bc bc1-12 dump == golden 全绿；VM 编译器
>   （compiler_vm = 新 compiler.px emit-c → gcc 链 vm.o）重放 compiler.px → dump 30315
>   行与 golden/compiler.bc.dump 逐字节一致 —— C 引擎轨与 VM 轨双轨一致。
> - **回归**：hello 三轨（pxi / C 轨 / VM 轨 emit-c→gcc→run）stdout 逐字节一致；
>   compiler.px parse/lint 零告警；bc_emit 逻辑未动 → cases_bc dump 全不变。
> - 记录：bc_cli.px 姊妹壳暂留（C2 收口退役，届时 bc_run.sh 切 compiler.px bc）；
>   bootstrap/pxc 二进制滞后一代未重链（沿用 M89_PLAN S0 决策，C2 后段 pxc VM 化统一
>   重链）；compiler.bc.dump 权威基线随 compiler.px 含 bc_emit 变为"含发射器自身的
>   编译器镜像"（VM 自举重放 == golden 实证）。
> - 下一步（C2 主体）：自举证明规则更新（compiler.bc.dump 为字节码权威对拍基准，
>   bootstrap_prove.sh 支持 BC 轨）；bootstrap/pxc/pxi 重链 VM 版（pxc 产物 = BCModule
>   内嵌 C + VM 启动 main，px build 默认切 BC）；S3-C 收口门 vm_ab.sh v2（examples
>   全量 VM vs 旧轨 stdout 对拍）。

### S3-C · C2-2 golden 同步修复 + BC 轨自举证明脚本（2026-09-08，dongyue）
> 完成（C2-2a + C2-2b）：C2 主体第 1 项（自举证明规则更新）落地，并修复 C2-1 提交的
> golden 与源码不同步问题。
> - **C2-2a golden 双轨同步修复**：发现 C2-1 提交(90f2a29) 的 compiler.px（21:44 最后
>   微调 bc flag → --emit-c，对齐 bc_cli）晚于 golden 生成（21:36），HEAD 中 compiler.px
>   (--emit-c) 与 golden(--bc) 不同步 → 自举证明实际失败（B.c vs golden 差 1 处 flag
>   判定行）。修复 = 用当前 compiler.px 重生成双轨权威基线：golden/compiler.c = pxc 编
>   compiler.px（14935 行，--emit-c 版）→ bootstrap_prove rc=0；golden/compiler.bc.dump =
>   compiler_new bc compiler.px（30315→30314 行，K 池含 --emit-c）→ VM 编译器
>   （compiler_vm = 新 compiler.px emit-c → gcc 链 vm.o）重放 == golden 逐字节一致；
>   bc1-12 dump golden 回归全绿。commit d01e2a7。
> - **C2-2b bootstrap_prove_bc.sh（BC 轨自举证明规则落地）**：自举证明基准从 C 文本单轨
>   升级为双轨（C 轨 compiler.c 由 bootstrap_prove.sh 守护；BC 轨 compiler.bc.dump 由
>   bootstrap_prove_bc.sh 守护）。BC 轨全链：pxc build compiler.px → compiler_new.c →
>   gcc 链 rtcache → compiler_new（C 引擎版）→ --emit-c compiler.px → compiler_vm.c →
>   gcc 链 → compiler_vm（VM 驱动版）→ bc compiler.px 重放 → 对拍
>   golden/compiler.bc.dump（带缓存失效判断 + --fresh 强制全链）。缓存态全链 rc=0，
>   VM 重放 dump 30314 行 == golden 逐字节一致。commit e1bf26d。
> - 下一步（C2 主体剩余）：bootstrap/pxc/pxi 重链 VM 版（pxc 产物 = BCModule 内嵌 C +
>   VM 启动 main，px build 默认切 BC）；S3-C 收口门 vm_ab.sh v2（examples 全量 VM vs
>   旧轨 stdout 对拍）。

### S3-C · C2-4 推导式 3 GAP 补齐 + VM op GENFROMLIST —— 收口门 GAP→0（2026-09-08，dongyue）
> 完成（C2-4）：补齐 vm_ab.sh v2 首跑记录的 3 GAP（m30_comp 推导式多变量/
> m32_gen 多 for GenExp/m34_gen_lazy 物化）→ S3-C 收口门全绿 **19 PASS + 0 GAP
> + 0 FAIL**（VM 与旧轨 pxi stdout 逐字节一致）。
> - runtime 新增 op **PXOP_GENFROMLIST 56**（a=dst, b=list 槽 → px_gen_from_list
>   物化包 gen；对齐 codegen px_gen_from_list/pxi it_gen）。指令集纯追加（55 op
>   编号语义零改动），PXM_MAX 57。
> - bc_emit.px：bc_emit_comp 多变量子句支持（逐字段 INDEX 解包，越界 null 对齐
>   pxi i_bind_comp_vars；单变量分配序保持既有 → bc1-12 dump golden 全回归不变）；
>   bc_emit_genexp 物化路径（非单 for 单变量 → NEWLIST + 嵌套循环 + GENFROMLIST）。
> - golden 双轨同步：compiler.c（14987 行）→ C 轨自举 rc=0；compiler.bc.dump
>   （30446 行，diff 仅 bc_emit_comp 自身字节区）→ VM compiler_vm 重放 == golden
>   （BC 轨自举 rc=0）。
> - 下一步（C2 主体剩余）：bootstrap/pxc/pxi VM 化重链（pxc 产物 = BCModule
>   内嵌 C + VM main，px build 默认切 BC）—— 建议分支试做 + 重生成 BC golden
>   对比，避免动主干 diffcheck（s01-s15 .c 形态）。

### S3-C · C2-5 px build --vm —— VM 轨产物产品化（分支试做，2026-09-09，dongyue）
> 完成（分支 feat/m89-c2-vmtoolchain 切片 1）：C2 主体最大项（pxc/pxi VM 化 +
> px build 切 BC）的第 1 切片 —— 把 VM 轨产物正式纳入用户工具链入口，与旧轨
> **并存过渡**（默认不变，显式 --vm/--bc 走 VM 轨）。
> - **tools/px 增 `px build --vm/--bc <file>`**：编译器从 bootstrap/pxc（fn_* C
>   文本轨）切到 compiler_vm（compiler.px 的字节码镜像 + VM 驱动，M89-S3-C2
>   正主），子命令 --emit-c → 产物 = bc_emit.px emit-c **BCModule 字节码镜像 C**
>   → gcc -static 链 runtime（rt_src_files 已含 vm.h/vm.c，cache 自动含 vm.o）
>   → **静态 ELF，程序跑显式帧 VM**。PXC_VM_BIN 环境变量可覆盖 VM 编译器路径
>   （为切片 2 pxc_vm 静态重链留口，CI/部署可指向 bootstrap/pxc_vm）。
> - VM 产物无 px_get_global 引用可提取 → 自动裁剪（M86-S2）对 VM 轨跳过，默认
>   全能力链接（≈9.0M 基线，旧轨 --full 语义）；显式 --no-xxx 裁剪仍可组合。
> - **环境债修复**：本机 Rocky Linux 9.8 缺 glibc-static（gcc -static 全链路
>   不可用，px build 本机跑不通 —— 旧 .rtcache 均为晨曦环境产物）→ 启用 CRB
>   仓库 + dnf install glibc-static → gcc -static 复活，**px build 旧轨本机回归
>   可用**（hello 2.7M 静态产物编译成功）。
> - 验证（px build --vm 静态产物 stdout == pxi 逐字节一致）：hello（基础）、
>   m34_gen_lazy（GenExp 物化 GENFROMLIST）、m30_comp（推导式多变量）、fib
>   （递归）、m39_gc（GC）五用例全 PASS；产物 file 确认 statically linked。
> - 下一步（切片 2）：compiler_vm 静态重链落位 bootstrap/pxc_vm（gcc -static
>   现可链）→ px build --vm 默认指 pxc_vm；pxi VM 化（interp.px 增 bc 发射 +
>   golden 同步）评估；分支验证通过后再谈默认轨切换（需整体规划 diffcheck/
>   capability 的 fn_* .c 形态 golden 迁移）。

### S3-C · C2-5b bootstrap/pxc_vm —— pxc VM 化静态重链（切片 2，分支试做）
> 完成（分支 feat/m89-c2-vmtoolchain 切片 2）：pxc 的 VM 化重链原型落地 ——
> **compiler_vm 静态重链落位 bootstrap/pxc_vm**（9.3M 静态 ELF，与 pxc/pxi 同为
> 引导二进制入库）。px build --vm 编译器默认路径升级：PXC_VM_BIN 环境变量 >
> bootstrap/pxc_vm（静态，clone 即用）> selfhost/build/compiler_vm（dev 自举
> 缓存）。gcc -static 本机可用（C2-5 已装 glibc-static）是重链前置。
> - 链法：selfhost/build/compiler_vm.c（compiler.px 字节码镜像静态 C）→ gcc
>   -static 链 rtcache（含 vm.o）→ bootstrap/pxc_vm。--version = pxc 0.2.0。
> - **自举一致实证**：pxc_vm bc compiler.px 重放（约 6 分钟，RSS ~0.76GB 内存
>   可控）→ dump 30446 行 == golden/compiler.bc.dump **逐字节一致** —— 静态
>   VM 版编译器具备权威编译能力，与动态 compiler_vm 完全等价。
> - 验证：px build --vm（PXC_VM_BIN=./bootstrap/pxc_vm）fib 静态产物 == pxi
>   逐字节一致；pxc_vm bc hello == compiler_vm bc hello 一致。
> - 下一步（切片 3 评估）：pxi VM 化（interp.px import bc_emit + emit-c →
>   pxi_vm 静态 ELF，解释器自身跑 VM）—— compiler_vm --emit-c interp.px 可行
>   性实验先行；分支验证通过后再谈默认轨切换（diffcheck/capability fn_* .c
>   golden 迁移需整体规划）。

### S3-C · C2-5c bootstrap/pxi_vm —— pxi VM 化原型（切片 3，分支试做）
> 完成（分支 feat/m89-c2-vmtoolchain 切片 3）：pxi 的 VM 化原型成立 ——
> compiler_vm --emit-c interp.px（143 行装配壳 + parser/env/cg_module/it_util/
> i_err/ival/icall/ibuiltin/iexpr/istmt 全解释链）→ 930KB BCModule 静态 C →
> gcc -static 链 → **bootstrap/pxi_vm**（9.3M 静态 ELF，解释器自身跑在显式帧
> VM 上）。发射层面证明 bc_emit 已覆盖解释器全链语法构造（无缺口）。
> - --version = pxi 0.2.0；hello/m22_bitwise_data/p8_slice_base64/struct 与
>   bootstrap/pxi 逐字节一致（VM 化解释器语义不变，行为对齐）。
> - 入库（引导二进制，与 pxi 同待遇）。默认轨未切：pxi_vm 供 PXI_BIN/PX_PXI
>   环境变量覆盖式实验（diffcheck/pxtest 等整体切 VM 版属默认轨切换，待分支
>   验证通过后整体规划）。tools/px 未加 px run --vm（pxi_vm 与 pxi 同语义，
>   解释执行无独立价值；pxc_vm 的 --vm 才是产物形态切换）。
> - 至此分支试做三大切片齐：px build --vm（产物 VM 化产品化）+ pxc_vm（编译器
>   自身 VM 化静态重链）+ pxi_vm（解释器自身 VM 化）—— C2 主体的技术可行性
>   全部实证；剩余 = 默认轨切换的 golden 大迁移（diffcheck s01-s15 .c / capa-
>   bility / 全量回归），需在主干整体规划后执行。

### S3-C · C2 收口：三大切片合入 main + 主干全量回归绿（2026-09-09，dongyue）

> 完成：分支 feat/m89-c2-vmtoolchain（C2-5/5b/5c 三大切片）**ff 合入 main**
> （8ebaacd）并 push GitHub main —— px build --vm + bootstrap/pxc_vm +
> bootstrap/pxi_vm 全部落主干，工作区干净。合入后主干全量回归复跑全绿：
> - **bootstrap_prove.sh**（C 轨自举）✅ 14986 行 == golden/compiler.c
> - **bootstrap_prove_bc.sh**（BC 轨自举）✅ compiler_vm 重放 dump 30445 行
>   == golden/compiler.bc.dump 逐字节一致
> - **vm_ab.sh v2**（S3-C 收口门）✅ 19 PASS + 0 GAP + 0 FAIL
> - **diffcheck.sh --all**（lex/parse/codegen/run vs golden）✅ 全量对拍通过
> - 本地分支已删，远程 feat/m89-c2-vmtoolchain 保留可追溯
> - **S3-C 段（C1 自举 + C2 golden 双轨同步/BC 轨自举证明/推导式 GAP 补齐/
>   收口门 GAP→0/三大 VM 化切片）技术目标全部达成**。

### S3-C · 决策记录：默认轨切换不在 S3-C 段做（2026-09-09，dongyue）

> 剩余最大项「px build 默认产物切 BC + diffcheck/capability/全量回归 golden
> 从 fn_* C 文本形态迁 BCModule 形态」**不在 S3-C 段执行**，理由（工程判断）：
> 1. **收益未到兑现点**：VM 轨当前是解释执行（无 JIT/native），把 px 默认
>    产物切到 BCModule 会让所有用户程序从「C 文本编译产物直接机器码执行」
>    退为「VM 解释执行」——性能倒退；VM 化的价值（显式帧 → 精确 GC/帧协程/
>    native 后端）尚未在本里程碑兑现，默认切换只增成本不增收益。
> 2. **成本高风险大**：diffcheck s01-s15 的 .c golden、capability、全量回归
>    全部依赖 fn_* C 文本形态；golden 大迁移 = 重生成全部基准 + 改造断言 +
>    主干验证体系整体动手术，不可逆、易碎，需独立规划与稳定期。
> 3. **技术可行性已实证**：C2 三切片证明 VM 化全链路（产物/编译器/解释器）
>    与旧轨逐字节一致；工程化铺开（默认切换 + golden 迁移）应作为 M89 后段
>    （GC/协程/native 前置需求明确时）或独立里程碑的专项，而非 S3-C 收口
>    的必要条件。
> → **S3-C 段正式收口**；默认轨切换挂起为后置决策项（触发条件：native 后端
>   立项 / 精确 GC 需要 VM 为唯一执行轨 / 用户拍板性能换架构统一）。

### S3-C · vm_ab 收口门扩展：19 例 → 确定性 examples 全量（2026-09-09，dongyue）

> S3-C 收口门定义本义 = examples 全量 VM vs 旧轨 stdout 对拍（M89_PLAN
> C2 记录原文）。当前 vm_ab.sh v2 清单 19 例为精选确定性子集；收口后
> 下一步将清单扩展到 examples 中全部「确定性本地输出」（无网络/无外部
> 服务/无不可控时间输出）用例，逐批补 bc_emit 缺口 → 收口门覆盖最大化。
