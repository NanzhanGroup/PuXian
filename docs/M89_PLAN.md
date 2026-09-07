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
