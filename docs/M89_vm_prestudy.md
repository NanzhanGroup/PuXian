# M89_S1 · VM 化详勘预研报告（只侦察，未写码）

> 日期：2026-09-07 · 作者：东月 · 版本基线：v0.2.0（efbedd0，M89 S0 后）
> 目标：把「px 函数 = C 函数 + C 递归」与「pxi = AST 树遍历」两条执行轨收敛到
> 显式帧 + 平坦字节码 VM，量化改造面、定字节码选型、把工期钉到周级。
> 本报告同时服务 issue28（GC STW/堆增长/吞吐）——预研发现三条根因的代码级证据，
> 并给出「保守 GC 内先止血」与「VM 化根治」的分界。
> 方法：全部结论基于对 selfhost/（8,145 行 px）、runtime/runtime.c（14,720 行 C）、
> runtime/runtime.h、selfhost/golden/compiler.c（10,596 行、172 个 fn_）的代码侦察 +
> 实测（用 pxc/pxi 编译闭包样例验证捕获语义）。

---

## 1. 现状执行模型（两条轨，均已被代码证实）

### 1.1 编译轨（pxc，产品主路径：px build → C 二进制）

流水线：pxlexer → parser（AST）→ cg_resolve_modules（import 合并）→ codegen（AST→C 文本）→ gcc。
AST 是**共享的规范数组节点** `[tag, ...]`（`["VarDecl", ...]`、`["Add", l, r, pos]` 等），
parser/compiler(codegen)/interp 三处消费同一 AST —— 这是 VM 化最有利的既有资产。

编译产物形态（实测自 golden/compiler.c 头部 fn_peek）：

```c
static LXValue fn_xxx(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("xxx");
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();   // 参数 → C 局部
    LXValue _v2 = px_null();                            // 函数级提升局部
    LXValue px_err_1_val = px_null(); int px_err_1_proped = 0;  // ? 传播标签
    if (px_is_truthy(px_lt(px_get_global("g_pos"), px_get_global("g_len")))) { ... }
    ...
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
    return px_null();
}
```

**帧模型（编译轨）**：
- 每个 px 函数 = 一个 C 函数；局部变量 = C 局部 `_v<N>`（cg_new_var 按名登记、函数顶预置）；
- **调用 = C 递归 px_call**（`fn.as.func.fn(args,nargs,ctx)` 直接调 C 函数指针），无堆上显式帧；
- 模块顶层名字 = 运行时全局符号表 `g_keys/g_vals`，每次访问走 `px_get_global("name")`（互斥锁 g_globals_mu + 字符串哈希查表）——selfhost 编译器 199 个写出的全局名、172 个 fn_，全局访问零缓存；
- 错误：Result 为普通值（PX_RESULT）+ 每函数 `goto px_err_N` 标签做 `?` 传播；px_error = 致命（打印现场，spawn 协程内经 g_err_jmp longjmp 隔离），语言无用户可捕获异常；
- 每语句 `px_srcline(N)` 埋行号（错误现场），无内联缓存、无常量预绑定。

**闭包/捕获（编译轨，实测关键发现）**：
- 表达式 lambda `fn(x){...}` → `fn_closure_N(args,nargs,ctx)`，**ctx=NULL 从未使用**；捕获外层局部
  编译成 `px_get_global("外名")`（实测 make(10) 捕获 n → 全局表无 n → null，编译版打印 null）；
- 函数内嵌套 `def` 被 codegen **整体丢弃**（实测 fn_make_adder 内无嵌套函数注册）；
- selfhost/golden/compiler.c **0 个 fn_closure** → 自举编译器自身刻意不用 lambda/闭包；
- **结论：编译轨目前没有真正的词法闭包**（只有"捕获模块级全局"的近似）。这对精确 GC 是
  **利好**（现状无 upvalue/环境链需搬），对 VM 化是**缺口**（闭包语义若要在 VM 上成立，需在
  帧/upvalue 设计里补上，或明确维持"仅捕获模块全局"并文档化）。

### 1.2 解释轨（pxi，px run 路径）

- interp.px 驱动 + it_util/i_err/ival/icall/ibuiltin/iexpr/istmt（合计 ~3,300 行 px）= AST 树遍历递归
  （i_exec_stmt/i_eval_expr/i_call_function）；
- 值层与编译轨**不同**：原生值（int/float/str/list/dict/result）透传；用户函数/结构体/枚举/生成器/
  类型对象/内置函数用 dict 包装（`__ufn__`/`__struct__`/`__enum__`/`__typeref__`/`__gen__`/`__builtin__`）；
- 闭包经 **env 链**捕获（实测 make(10)+lambda 在 pxi 正确输出 15）——语义比编译轨"真"；
- 慢（AST dict 遍历 + 每值 Result 装箱 + env 链查名）。

### 1.3 双轨现状小结

| 维度 | 编译轨（pxc→C） | 解释轨（pxi 树遍历） |
|---|---|---|
| 值表示 | LXValue + 原生类型直通 | dict 包装用户对象 |
| 局部变量 | C 局部（_v） | env 链 dict 查名 |
| 调用 | C 递归 px_call | px 层递归 i_call_* |
| 闭包捕获 | 仅模块全局（近似） | env 链真捕获 |
| 错误 | Result + goto 标签 / px_error 致命 | Result Err + __prop__ 传播 |
| 一致性 | golden 对拍逐字节 | MINI_SUBSET 对拍逐字节 |

两条轨语义已各自对拍锁定 → **VM 化 = 换同一台引擎执行同一 AST→字节码，语义基准 = 现两条轨的
golden 用例集**。

---

## 2. 改造面量化

### 2.1 AST/语言构造盘点（字节码发射器必须覆盖的指令面）

编译轨 cg_expr 处理的表达式节点 tag（实测 grep 全集）：

```
Add And Binary BitAnd BitOr BitXor Block Bool Call Closure Constructor Dict DictComp Div Eq
ExprStmt Field Float ForceUnwrap Ge GenExp Gt IfExpr Index Int IntDiv Le List ListComp Lt Match Mod Mul
Ne Neg Not Null NullCoalesce OptionalField Or PatBinding PatConstructor PatLiteral PatTuple PatWildcard
Pipe Pow Shl Shr ShrU Slice Str Sub Try Tuple Unary Var
```

语句面（istmt/cg_stmt + parser 产出的顶层/函数体节点）：VarDecl / Assign / ExprStmt / If / For /
While / Return / Break / Continue / Def / Struct / Enum / Impl / Trait / Import / Match / Spawn /
Chan send·recv·select / Type 简化枚举 / 常量声明等（以 parser.px 75 个 def + cg_gen_stmt 分支为准）。

对应量级（selfhost 自身，最能代表真实负载的"中等复杂"px 程序）：
- selfhost/*.px 合计 8,145 行、~329 个函数 def（parser 75 / pxlexer 36 / capability 21 / ibuiltin 18 / astdump 16 ...）；
- 编译成 C：compiler.c 10,596 行、**172 个 fn_**、199 个模块全局名。

### 2.2 值表示与 GC 根面（精确 GC 改造量）

runtime.h：`LXValue = {LXType, union{bool,int64,double,LXObject*}}`；
LXObject 17 个类型中**含子引用、需递归标记**的：PX_LIST(items)、PX_DICT(vals 及 keys 均为对象化键
外的 char*）、PX_STRUCT_inst(fvals)、PX_TUPLE(items)、PX_GEN(list/seq/transform/filter)、
PX_RESULT(value)、PX_CHAN(buf)；**叶子**：STR/BYTES/FUNC/NATIVE/ENUM/MUTEX/RWLOCK（当前 func 无 ctx 引用，
ENUM 仅名字字符串）。

现状 GC（runtime.c 560–1270 行，证据级）：
- **STW 全线程暂停**：SIGRTMIN+2 实时信号逐个暂停已注册 worker，epoch 自旋同步；
- **保守整栈扫描**：gc_scan_stack 对每线程 C 栈 SP→栈顶逐 word，`(w&7)==0 && 在 g_objs 哈希` 即当指针标记
  —— 无类型信息，**会误留垃圾**（栈上陈旧 word 命中对象 → 假存活）；worker 栈越大、线程越多，单次
  STW 越长（issue28 实测 W32 比 W8 更慢同源）；
- 触发：全局对象表 g_objs 对象数 ≥ 阈值（默认 100,000，PX_GC_THRESHOLD 可配），每轮后阈值翻倍
  （`g_gc_threshold = g_obj_count * 2`）——堆头寸最多 2× 存活量，collect 周期与分配速率耦合；
- 分配器：mmap + 21 档 size-class slab（16B–16KB），超 16KB 直接 mmap；**slab 页全空不归还 OS**
  （无 madvise/munmap 回收路径）→ RSS 只涨不落（issue28 根因 3 的直接代码证据之一）；
- 根集合：全局表（持 g_globals_mu 扫）+ 各线程栈/寄存器/tmp_root；
- 保守误留 + slab 不还 + 阈值翻倍头寸 = issue28「40MB→380MB→1.1GB 不回吐」三层代码级解释。

**精确 GC 改造面量化**：把根从「整 C 栈逐 word」换成「VM 帧槽数组 + 全局槽数组 + 原生桥暂存根」后：
- 标记器不再需要类型不可知扫描；每 LXValue 槽自带 type 判别（与现 gc_mark_obj 的 DFS 一致，只是根更精）；
- 需人工登记的**原生桥暂存根**：runtime 全库 grep `px_call(` 约 20+ 处调用点在持有 LXValue 临时量时
  跨 px_call（map/filter/reduce 回调、gen transform/filter、sort、方法转发 call_with_self、mutex.with、
  spawn 参数等）→ 这些 C 函数内的临时 LXValue 需 px_push_root/px_pop（或并入现 g_tmp_root 机制扩展），
  估计 ≤ 60 处改造；
- 栈扫描架构（arch.h read_sp + pthread_getattr_np + setjmp spill）**可整体退役**，Windows 移植随之变易。

### 2.3 双轨收敛路径

两条轨共享 parser/lexer/AST → 新增一层 **AST→字节码（bc_*.px）**，字节码由**单一 VM** 执行：
- `px run`（现 pxi 树遍历）→ 解释器 = 同一 VM 直接跑字节码（不再 parse 后逐节点树走）；istmt/iexpr/
  ival/icall/i_err/it_util 树遍历栈（~3,300 行）可退役（过渡期保留兜底）；
- `px build`（现 codegen→C fn_*）→ 产物改为「C 文件内嵌字节码表 + 极简 main 启动 VM」或
  「独立字节码镜像 + 小 launcher」，gcc 管线不变，**原生内置函数（~140 内置 + 方法分派）仍是 C 桥**，
  不进 VM（对齐 CPython 内置层定位）；
- 自举 compiler.px 自身在 VM 上编译自己 → golden 从 fn_* C 文本对拍切换为字节码镜像对拍（自举证明
  改字节码逐字节相等，规则沿用）。

### 2.4 生成器/异常/协程在 VM 中的形态（预判）

- 生成器：现编译轨 = C 层 px_gen_lazy/物化（gen 对象持 seq + transform/filter 闭包，由 gen_next 在 C 里
  逐项回调，**不需 px 帧挂起**）→ VM 下保持同样结构（闭包变字节码函数即可），**无帧快照需求**；
- 异常/错误：Result 本就是普通值 + `?` 编译为指令序列（Err → 函数级错误处理/返回），px_error 仍是
  致命原生（spawn 内 longjmp 隔离）——VM 不需要用户异常栈展开（语言无 try/catch），错误栈保持
  srcfunc/srcline 埋点；
- C 类协程（M90）：显式帧 = 槽数组，挂起 = 帧拷贝存档 → 帧协程在 VM 上**自然可得**（M88_PLAN §四·A
  裁定甲路线成立的前提由本报告坐实）；而现 C 递归模型完全不可行（无堆上帧）。

---

## 3. 字节码格式选型（建议：寄存器式 3-地址，理由充分）

| 维度 | 栈式（CPython 风格） | 寄存器式 3-地址（Lua/推荐） |
|---|---|---|
| 帧槽映射 | 操作数在栈顶，局部仍需 slot | 帧 = LXValue 槽数组，指令直接 [dst,a,b] |
| 与现 codegen 迁移 | 需重排（cg 现产 C 表达式树，天然树→寄存器） | cg 每表达式已产出临时 _v 语义 → 直接映射槽，**迁移最省** |
| 精确 GC 标记 | 栈顶区 + 局部 slot 两处 | 帧槽数组一体扫描 |
| 帧协程快照 | 栈+局部 | 拷贝槽数组即可 |
| 指令密度/性能 | 指令多、字节小 | 指令少、每指令 4B 编码，解释循环简单 |
| 已知陷阱 | — | 需防寄存器分配碎片（现 cg_tmp 已天然编号，风险低） |

理由：现 cg 的局部/临时本身就是「按名编号的 C 局部 _v<N>」→ 寄存器 VM 的槽号 = 同一编号体系，
AST→BC 发射器只是把「打印 C 代码」换成「发指令 + 常量/名字入池」，**parser/AST/名字解析零改动**。
常量池存 int/float/str 字面量；全局名表 name→idx，初始化后只读（GC 期外无锁）→ 顺带根治 M55
全局互斥的读路径串行（issue28 根因 2 在 VM 下消失）。

---

## 4. issue28 关联结论（预研的直接产出）

三根因代码级定位 + 修复分界：

| issue28 根因 | 代码证据（本报告） | 保守 GC 内止血（独立批次，3–5 天） | VM 化根治（M89） |
|---|---|---|---|
| STW 尖刺（~11 请求一刺，W 越多越久） | 信号全线程暂停 + 每线程整栈逐 word 扫描 | B1：触发策略改「请求间隙/空闲后台」+ 单次 STW 上限保护 + 阈值封顶；扫描降频 | 根=帧槽精确标记，无整栈扫描，STW 时间与栈大小解耦 |
| 堆只涨不落（40MB→1.1GB） | 保守扫描假存活 + slab 页不归还 OS + 阈值翻倍头寸 | B2：slab 全空页 munmap/madvise 归还 + 字节阈值触发启用 + GC 后压缩 | 精确标记消灭假存活 → 不回收垃圾真正可回收 |
| 吞吐 GIL（500 并发 p50=5.2s） | 每全局访问 px_get_global 锁+哈希 | 读多写少 → 评估读写锁/只读快照（收益有限、需防 GC 交互） | 全局=槽数组，初始化后只读无锁 |

**分界建议**：issue28 B1/B2 是 3–5 天、不依赖 VM、立刻缓解线上（#47 保持 Go 版期间可灰度）；
M55 锁细化为可选尾项（评估后并入或跳过，VM 会根治）。M89 主体（VM）是根治但数月级。
→ 执行顺序建议：**先 issue28 B1/B2 批次 → 再 M89 S2 设计**（S2 可与 B1/B2 并行开展）。

---

## 5. 工期钉到周级

假设：单人全时、S 微步纪律（每步编译 + 相关 verify + 不混 commit）、沿用现 codegen 语义基准对拍。
风险缓冲已含在区间（P50=基准）。

| 阶段 | 内容 | 周 |
|---|---|---|
| S2 设计 | 字节码指令集 spec + 帧布局 + 常量/名字池 + A/B 对拍 harness（现 C 递归 vs VM）+ S 清单 | 1.5–2 |
| S3-A 骨架 | bc 发射器覆盖核心 stmt/expr（~60 expr tag 全走一遍、语句 15 类）；VM 主循环 + 帧 + 内置函数桥 + Result/? 传播；golden cases A/B | 2 |
| S3-B 全构造 | 模式匹配/match、推导式、生成器、struct/enum/trait/impl 方法分派、FFI、chan/spawn 并发语义；examples+cases 全量 A/B | 2 |
| S3-C 自举收敛 | compiler.px 在 VM 上编译自身（对拍与现 C 递归一致）→ golden 切换字节码镜像；pxi 树遍历退役（interp golden 在 VM 上跑通）；构建/发布管线切 BC 产物 | 2 |
| S3-D 精确 GC | 帧槽/全局槽/原生桥暂存根（~60 处）；退役整栈保守扫描（保留信号 STW 或第一步增量）；按 issue28 验证标准压测（单发 p95≤50ms、500 并发 p50≤200ms、堆回落） | 2 |
| S4 收口 | 自举证明（字节码镜像）+ 全量回归 + 重链 bootstrap/pxc/pxi + 文档 + tag v0.2.0-m89 | 0.5–1 |
| **合计** | | **P50 ≈ 10–11 周；P10 ≈ 9 周；P90 ≈ 15–16 周** |

结论：VM 化 = **季度级（约 10–12 周核心，P90 16 周封顶）**，与 M88_PLAN §四·A「数月级」预判吻合；
非一人之力可按 S3-A/B/C/D 四段横向分包（四段边界清晰、各自对拍自洽）。

---

## 6. 风险与预案

| 风险 | 证据/影响 | 预案 |
|---|---|---|
| 编译轨无真词法闭包（ctx=NULL） | 实测 | VM 设计明确 upvalue 策略：方案①帧引用（真闭包，补语义但需查现语义基准是否依赖）；方案②维持"仅模块全局捕获"并文档化——S2 定稿前先用 golden cases 探测闭包用例覆盖面 |
| 原生桥 ~60 处暂存根遗漏 → GC 误清 | runtime grep px_call 20+ 处 | 精确 GC 阶段先跑 spawn/回调/生成器专项压测 + GC 调试开关（g_gc_debug=1）核对标记数 |
| 自举过渡期 golden 易碎 | 现 golden 是 fn_* C 文本 | 任何动 codegen 的 commit 同步 golden + 自举证明；C 递归路径保留到 S3-C 末尾再删 |
| 语义漂移 | 双轨各有对拍 | A/B harness 先锁现两条轨的 golden 用例集，VM 只做"第三种实现必须同时过两组对拍" |
| 字节码/常量池膨胀 | — | 每指令 4B 定长 + 常量去重；产物 C 内嵌数组与现 fn_* 体积同级（compiler.c 10,596 行 ≈ 内嵌表量级可比） |
| interp/pxi 退役连带回归面 | ~3,300 行 px | 过渡期保留树遍历器作兜底，VM 在 interp golden 全绿后再删 |

---

## 7. S1 结论（一句话）

VM 化改造面已量化、字节码选型有据（寄存器式 3-地址 + 帧槽=现 _v 编号，parser/AST 零改动）、
工期钉到 **P50 10–11 周 / P90 16 周**；同时坐实 issue28 三根因的代码级证据与「保守 GC 止血（3–5 天）
先于 VM 根治」的分界。**下一步 = 先立项 issue28 B1/B2 止血批次，再进 M89-S2 设计。**
