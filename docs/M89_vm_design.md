# M89_S2 · VM 设计定稿（显式帧 + 平坦字节码 VM）

> 日期：2026-09-08 · 作者：东月 · 基线：v0.2.0（63f1026）
> 上游：docs/M89_vm_prestudy.md（S1 详勘，只侦察未写码）
> 本文档 = S2 交付物：**设计定稿（D1–D14 决策记录）+ 字节码指令集 spec + 帧/调用/错误/闭包/GC/并发设计 + 实现批次拆分（S 微步清单）**。
> 原则（沿用 M89_PLAN §六 纪律）：每 S 微步编译 + 相关 verify 通过、不混 commit；语义以现双轨 golden 用例集为对拍基准，先锁语义再动引擎。

---

## 〇、一句话

新增**单一执行引擎**：PuXian 程序统一编译为**平坦字节码**，由 C 写的**显式帧栈 VM** 执行；
`px run`（现 pxi AST 树遍历）与 `px build`（现 codegen→C fn_*）两条轨收敛到「同一 AST → 同一字节码 → 同一 VM」。
发射器（AST→BC）用 **PuXian 自举**写；执行器（解释循环/帧/GC 根）属 **runtime 层用 C 写**（对齐 CPython：编译器/标准库用自身语言，字节码解释器 ceval.c 用 C）。

---

## 一、总体架构与决策记录（D1–D8）

### D1 · 发射器 = PuXian（selfhost/bc_*.px）；执行器 = C（runtime/vm.c）
- **发射器**：新增 `selfhost/bc_emit.px`（+ bc_util 拆分，按 500 行/文件纪律），复用现 parser/pxlexer/AST。
  输入：`cg_resolve_modules` 合并后的单一 Program（与现 codegen 完全同输入）；输出：BCModule 结构（下面 §三）。
- **执行器**：`runtime/vm.c`（C）。与 runtime.c 共享 LXValue/LXObject/GC/内置函数表，零重复值系统。
- 理由：执行器须与 runtime GC、PX_* 类型、原生桥紧密咬合，用 C 直接操作最省；发射器必须随语言自举
  （compiler.px 生态），且发射器逻辑 = 现 codegen 的「目标从 C 文本换成字节码」，纯 AST→表结构，PuXian 表达充分。
- 过渡期：现 codegen→C（fn_* 产物）**保留**至 S3-C 末尾，作为语义对拍基准与回归兜底；S3-C 后 golden 切字节码镜像。

### D2 · 统一函数对象与调用入口（px_call 零改动兼容）
- 现 `PX_FUNC` 对象 = `{name, fn, ctx}`，`px_call` 对 PX_FUNC 直调 `func.fn(args,nargs,ctx)`。
- **VM 化后**：一切 px 用户函数（顶层 def / impl 方法 / 闭包）编译为字节码函数对象 `PxVMFunc*`，
  其 `func.fn = px_vm_entry`（**唯一 C trampoline**）、`func.ctx = PxVMFunc*`。
- `px_call` **一行不改**：PX_FUNC 分支自然进入 `px_vm_entry`；`px_method`/`px_spawn`/`call_with_self`
  及全库 20+ 处调用点**零改动**（它们拿到的 `func.fn` 就是 trampoline）。
- 兼容旧产物：过渡期 PX_FUNC 分两类——`fn == px_vm_entry`（ctx=PxVMFunc）→ VM 执行；否则（旧 C 编译
  fn_* 静态函数）→ 直调 C（px_call 现逻辑天然支持，加一个指针相等判断即可）。
- **线程模型**：`px_vm_entry` 在当前线程取/建 thread-local `PxVmState` 执行（spawn/连接线程各自独立
  VM 状态，共享全局槽与 GC）——与现 px_pool_worker/px_spawn 线程模型完全同构。

### D3 · 显式帧栈（VM 内调用不回 C 递归）
- 每线程 `PxVmState { PxFrame* frames; int nframes, cap; ... }`，帧动态增长。
- `PxFrame { PxVMFunc* f; LXValue* slots; int nslots; int pc; int err_pending; LXValue err_val; }`。
- px→px 调用：**push 新帧，解释循环继续**（C 递归深度不随 px 调用链增长 → 深递归安全、帧可精确 GC 根）。
- px→native（C 内置）→ 回调 px（map/filter/sort/mutex.with/gen 回调等）：native 内经 px_call 回到
  `px_vm_entry` → **C 递归一层**（与 CPython PyEval_EvalFrame 同构）。C 栈深 = px↔C 交替边界数，通常很浅；
  深 px→px 链永远走显式帧。
- 帧协程（M90 C 类）：显式帧 = 槽数组，挂起 = 帧拷贝存档，S1 已坐实可行，本设计为其铺路（本里程碑不做）。

### D4 · 字节码格式（寄存器式 3 地址，8B 定长）
- 指令 = `{ uint8 op; uint8 fl; uint16 a; uint16 b; uint16 c; }` = 8 字节定长。
- 操作数：`a/b/c` 视指令为**帧槽号 / 常量池 idx / 全局槽 idx / 名字表 idx / 相对跳转条数（int16）**。
- 槽号 = 现 codegen 的 `_v<N>`/`_t<N>` 同一编号体系（cg_uid 顺序）→ **发射器是现 codegen 的机械改写**：
  每个「把表达式文本拼进 C 字符串」换成「发指令 + 登记常量/名字」。
- 跳转偏移按「指令条数」相对当前 pc（`pc += 1` 每条），u16 存 int16 偏移（±32767 条足够单函数；
  超长函数由发射器拆 JMP 链兜底）。
- 常量池 K：int64/float/str 字面量（str 去重）；名字池 N：全局名/字段名/方法名/类型名/模块内名去重统一编号。

### D5 · 短路 and/or / NullCoalesce（返回操作数值，Python 语义）
- 实测现编译轨：`a and b` → `t=a; is_truthy(t) ? b : t`；`or` 对称；NullCoalesce 同族。**短路 + 返回操作数**。
- VM 映射：求值 a→t；`JMPF t,L`（and）/ `JMPT t,L`（or）跳过右式，t 保留左值；否则求值右式覆写 t。
  需要 JMPT/JMPF 为「条件跳 + 不破坏寄存器」的普通跳转即可（无专用 AND/OR 指令）。

### D6 · 闭包：真词法捕获（upvalue cell），分两期
- 现状三态：spec §6.2 定义**真捕获（读）+ `capture mut` 显式写**；pxi env 链实现真捕获（读）；
  编译轨**实现缺陷**（0 个 fn_closure、捕获外层局部退化 px_get_global 取 null、嵌套 def 被丢弃）。
- **决策**：VM 实现 spec 语义（真词法捕获读），弥补编译轨缺口，与 pxi 语义收敛。
  - 捕获机制 = **upvalue cell**（Lua 式）：函数内被嵌套 fn 引用的局部槽升级为「1 元素 cell 对象」，
    嵌套 fn 捕获 cell 引用；读=cell 解引用，写（仅 capture mut 局部）=写 cell。
  - 分期：**P1（S3-B）** 无捕获闭包（body 仅引用参数/模块全局）跑通 —— 覆盖 m25_closure_gc、自举编译器
    （0 闭包）；**P2（S3-C）** 真捕获 upvalue cell（对齐 spec + pxi env 链）。P1 前先做**捕获面扫描**
    （grep examples/selfhost cases 中「嵌套 fn 引用外层局部」用例），若现存量用例依赖捕获则 P2 提前并入 P1。
- cell 是 GC 对象（含 1 槽）→ 精确 GC 标记为根类型，S3-D 一并覆盖。

### D7 · 错误传播：Result 值 + 帧 err 传播（映射现 err_tag 模型）
- 现编译轨：每函数 `err_tag: if (err_tag_proped) return err_tag_val; return null`；`?`(Try) 在函数体内
  检测到 Err/null → 置 val+proped → goto 函数尾标签（即**就地返回**）。
- VM 映射（等价且更简）：帧字段 `err_pending/err_val`。
  - `?expr` → 求值 expr→槽 t；`OP_TRY t`：t 是 Result 且 Err → `RETURN t`（err_pending 语义内联为返回，
    因为函数尾只是转发）；t 是 null → 返回 px_null()（null 传播=返回 null，对齐现 Try null 分支）；
    否则就地解包 `t = result.value`。
  - `!expr`（ForceUnwrap）→ `OP_FORCE t`：Result Err → px_error("force unwrap Err")；null → px_error；否则解包。
  - 顶层 `?`（无函数上下文）→ 编译期/运行期 px_error（对齐现「顶层不能传播」语义）。
  - 调用方**不自动检查** Result：`?`/`!`/match 是显式消费点（与现一致，无隐式异常栈展开）。
- px_error 致命错误、spawn 内 longjmp 隔离、px_srcfunc/px_srcline 现场追踪：VM 指令每行插 `SRCLINE line`
  （现 cg 每语句 px_srcline 对应），函数入口帧记录 name（px_srcfunc 语义）。

### D8 · 模块/编译单元 = BCModule（单 prog 单单元）
- import 由 cg_resolve_modules 编译期合并 → 单一 Program → 单一 BCModule（与现 codegen 同输入同粒度）。
- `BCModule`：
  - `K` 常量池（去重）
  - `G` 全局槽声明表：cg_globals 全部名字 → 槽 idx（含函数名、顶层 var、impl 方法名 "Type.method"）
  - `N` 名字池（字段名/方法名/类型名/枚举名 等运行时名字，与 K 分离或合一）
  - `Func` 函数表（每项 = PxVMFunc：name、arity、默认参数数、bc 数组、常量池引用、upvalue 描述）
  - `Struct`/`Enum` 元数据表（类型名 → 字段名表/变体表；与现 cg_collect_types 输出对齐）
  - `Top` 顶层代码 bc（主模块：注册全局 + 顶层语句 + main 调用）
  - `NativeRefs` 外部名表（源码中 Var 解析不到「局部/声明全局」的名字 = native/builtin/FFI 名 → 启动期绑定）
- **全局访问去锁化（issue28 根因 2 的 VM 根治）**：
  - 声明全局（函数/顶层 var）→ 启动期把每个名字绑定到**固定槽**（槽数组一次性分配）：
    函数名槽 = PX_FUNC 对象；var 槽 = 初值；此后 `GETG gidx` = 槽数组下标（**无锁无哈希**，读路径 GIL 消失）。
  - native/builtin/FFI 名 → 启动期经 px_global_native/FFI 查表解析为 PX_NATIVE 对象填入固定槽（一次绑定，
    只读）。解析失败保持现 R1001 语义（未定义变量报错时机一致）。
  - 顶层 var 的 `SETG`：写槽 + 同步写 g_keys/g_vals（兼容 C 桥按名查询如 px_method 的 "Type.method" 转发、
    px_get_global 只读面）。SETG 低频（仅可变全局赋值）→ 双写成本可忽略。
  - 现 g_globals 表与 M88-B3 读写锁**保留**：服务 C 桥按名查询 / 旧 C 产物 / px_method 方法转发（原生方法名）。
- 过渡期双视图一致：VM 启动逐名执行等价 `px_set_global(name, 槽初值)` 注册（旧 C 产物同款），确保
  px_get_global/px_method 见到的名字集与现模型完全一致 → **对拍不因视图差异漂移**。

---

## 二、字节码指令集 spec（v0.2.0-m89 草案）

编码：`{op u8, fl u8, a u16, b u16, c u16}`，8B 定长。`pc` 单位为条。约定操作数缩写：`s`=槽号，`k`=K 池 idx，
`g`=G 槽 idx，`n`=N 名字 idx，`off`=相对跳转条数(int16，目标 pc' = pc + off)。

### A. 加载 / 槽
| op | a | b | c | 语义 |
|---|---|---|---|---|
| `LOADK` | s | k | - | 槽s = K[k]（int/float/str/bool/null 统一池） |
| `IMM` | s | imm16 | - | 槽s = int(imm16)（小整数快捷，发射器对 |n|≤32767 用） |
| `MOV` | s | s2 | - | 槽s = 槽s2 |
| `GETG` | s | g | - | 槽s = 全局槽 g（无锁数组读） |
| `SETG` | g | s | - | 全局槽 g = 槽s（顶层 var：同步 g_globals） |

### B. 一元/二元运算（dst=槽s，a/b=槽s2/s3；语义=调对应 px_* 现 C 函数）
| op | a | b | c | 语义 |
|---|---|---|---|---|
| `NEG` `NOT` `BITNOT` | s | s2 | - | px_neg/px_not/px_bitnot |
| `ADD` `SUB` `MUL` `DIV` `IDIV` `MOD` `POW` | s | s2 | s3 | px_add...px_pow |
| `EQ` `NE` `LT` `LE` `GT` `GE` | s | s2 | s3 | px_eq...px_ge |
| `BITAND` `BITOR` `BITXOR` `SHL` `SHR` `SHRU` | s | s2 | s3 | px_bitand...px_ushr |

> and/or/?? 无专用指令（短路语义用分支，D5）；错误/越界由 px_* 内 px_error 保证（语义与现一致）。

### C. 容器 / 字段
| op | a | b | c | 语义 |
|---|---|---|---|---|
| `INDEX` | s | s2 | s3 | 槽s = px_index(槽s2, 槽s3) |
| `SETIDX` | s | s2 | s3 | px_index_set(槽s2, 槽s3, 槽s)；结果=槽s（赋值表达式值） |
| `SLICE` | s | s2 | s3 | px_slice(槽s2, 槽s3[s], 槽s3+1[e], 槽s3+2[step])（s3=3 连续槽基址，缺省已置 px_null） |
| `GETF` | s | s2 | n | 槽s = px_field(槽s2, N[n])（含 enum 变体/const enum 提前量：发射器把编译期可折叠的 Field 直接发 LOADK/常量） |
| `SETF` | s2 | n | s | px_field_set(槽s2, N[n], 槽s)；结果=槽s |
| `GETF_OPT` | s | s2 | n | OptionalField：槽s2 null→px_null 否则 px_field |
| `NEWLIST` | s | s3 | n | 新建 list，从槽 s3 连续 n 项拷贝 |
| `NEWTUPLE` | s | s3 | n | 同上 tuple |
| `NEWDICT` | s | s3 | n | 从 s3 起 n 组 [k,v] 建 dict（n=项数，槽占用 2n） |
| `NEWSTRUCT` | s | t | s3 | 建类型 t 的 struct 实例，字段值自槽 s3 连续 nfields 个（t=N 类型名 idx；字段表查模块元数据） |
| `NEWENUM` | s | t | v | 建类型 t 变体 v（t/v=N idx 或模块枚举元数据 idx） |
| `LISTPUSH` | s2 | s | - | px_list_push(槽s2, 槽s)（结果可忽略，留槽 s2） |

### D. 调用 / 返回
| op | a | b | c | 语义 |
|---|---|---|---|---|
| `CALL` | s | s2 | argc | 调 槽s2 函数；参数 = 槽 s2+1..s2+argc 连续区（发射器在 CALL 前 MOV 排布）；返回写槽 s |
| `CALLM` | s | s2 | n | 方法调用 obj.method：obj=槽s2，方法名 N[n]；等价现 px_method(obj,N[n],args...)；args 区同 CALL |
| `CALLSELF` | s | s2 | argc | 以 self=槽s2 首参调 槽s（现 call_with_self 语义；list.map 等 C 回调转发用，S3-C 可静态化） |
| `TAILCALL` | s2 | argc | - | 尾调用：复用当前帧（S3-C 优化项，`@tailrec` spec §6.4 语义出口） |
| `RET` | s | - | - | 返回槽 s 值（含 err_pending 时返回 err_val，见 D7） |
| `RET0` | - | - | - | 返回 px_null()（函数自然落尾） |

### E. 控制流 / 错误
| op | a | b | c | 语义 |
|---|---|---|---|---|
| `JMP` | - | off | - | pc += off |
| `JMPT` | s | off | - | 槽s 真值 → pc += off |
| `JMPF` | s | off | - | 槽s 假值 → pc += off |
| `TRY` | s | - | - | D7：槽s 为 Result-Err → RET s；null → RET0；否则 s=解包值 |
| `FORCE` | s | - | - | D7：Err/null → px_error；否则 s=解包值 |
| `SRCLINE` | - | line | - | 记录行号到帧（px_srcline 语义；函数入口帧含名） |
| `HALT` | - | - | - | 顶层代码结束 / main 返回 |

### F. 生成器 / 并发 / 桥（保留现 C 语义，多数映射到 px_* 既有函数）
- 生成器：`NEWGEN s,s2,s3,s4` 多操作数 → 拆 2 条（先 MOV 排布 4 槽：seq/transform/filter）用 `NEWGEN s, base`
  （现 px_gen_lazy 物化/惰性）。gen_next 消费等走内置函数桥（PX_GEN 已是对象，gen 的 transform/filter 槽存
  PX_FUNC=字节码闭包，gen_next C 内 px_call 回 VM——D3 回调路径）。
- `SPAWN s, s2, argc`：现 px_spawn_name 语义（新线程跑 槽s2 函数，参数连续区）→ runtime 线程 + 该线程
  thread-local VM 执行（D2）。`CHAN`/`SEND`/`RECV`/`SELECT`/mutex/rwlock：编译为对 chan/mutex 原语的
  方法/函数调用（现 px_method 通道分支 + 内置），VM 不重造，走 C 桥。
- FFI/原生调用：native 函数对象已在槽（D8 NativeRefs 绑定），`CALL` 统一（PX_NATIVE → C 直调）。

> **指令数估计**：上表 ≈ 45 条 op。发射器覆盖 expr tag ~60 个 + stmt ~15 类（S1 §2.1 清单），全部映射到上表
> 指令序列（含推导式/match 展开为分支+循环指令，映射现 cg_comp_body/cg_gen_select 的控制流）。

---

## 三、帧布局与调用约定（细化）

```
PxFrame {
  PxVMFunc* f;        // 函数元数据（bc/K 池引用/name/arity/upvalue 描述）
  LXValue*  slots;    // 槽数组：slots[0..arity-1] 参数；其后局部(hoist)+临时
  int       nslots;   // 槽容量（发射器算好 = 参数 + cg 在该函数内 uid 峰值）
  int       pc;       // 返回时恢复
  uint8     err_pending; LXValue err_val;   // D7 错误传播（TRY 就地返回，帧内可省——见下）
  PxFrame*  prev;     // 帧链（栈式数组内 prev 索引）
}
```
- **参数传递**：CALL 指令取 caller 帧 s2+1..s2+argc 连续槽 → push callee 帧时整段拷入
  callee.slots[0..argc)。arity 默认参数：callee 侧 slots 预填默认值，不足 argc 时覆盖（对齐现
  `(nargs>i)?args[i]:default`）。
- **err_pending 是否需要帧字段**：D7 采用「TRY 检测到 Err 即 RET 传播」，则函数尾无需 goto 标签，
  err 不驻留帧 → **帧无需 err 字段**（RET 即返回 err 值）。与现 err_tag 模型语义等价（函数尾仅转发），
  帧更简。顶层 ? 在 Top bc 里 TRY 遇 Err → px_error。
- **slots 生命周期与精确 GC（S3-D）**：活动帧 = 线程根；`RET` 时帧弹出（槽不再扫描）。slot 值类型自带判别
  → 精确标记：slot.type 含引用（PX_STR/LIST/DICT/...）→ mark。**无整栈逐 word 保守扫描**（退役 arch.h read_sp）。
- **调用深度保护**：nframes 达上限（如 1M 槽/16MB 栈预算）→ px_error("栈溢出")（现 C 递归靠系统栈，
  VM 显式帧给可控上限，深递归不再 segfault）。

---

## 四、A/B 对拍 harness（S3-A 首步交付）

目标：VM 是「第三种实现」，必须同时过**两条现轨的用例集对拍**。
- 用例集：`selfhost/cases/`（20 个 s*/v*，含 MINI_SUBSET golden）+ `examples/` 全量 + `selfhost/golden/*.ast`（AST 级驱动）。
- 三实现两两对拍：`px build`（C 递归产物，旧）/ `pxi`（树遍历）/ `pxv`（VM，新增入口 `tools/px run --vm` 或独立 pxc-v）。
- 对拍内容：stdout 逐字节 + 退出码 + （服务类）行为等价；错误现场（px_error 文案）比对 diffcheck 既有机制。
- harness 脚本 `selfhost/vm_ab.sh`：对每用例跑三实现，diff 输出，汇总 PASS/FAIL 清单。
- 自举证明（S3-C）：compiler.px 在 VM 上编译自身 → 字节码镜像与 golden 逐字节一致（自举证明规则从
  C 文本对拍切 BC 镜像对拍，M89_PLAN §六 纪律沿用）。

---

## 五、与工具链/发布集成（S3-C 落地）

- `px run file.px`：lex→parse→resolve→**bc 发射**→内存 BCModule→VM 执行（pxi 树遍历退役前保留兜底开关）。
- `px build file.px`：产物 = **C 文件内嵌 BCModule（字节码数组 + K/N/G/Func 表 + 启动 main）**，gcc 管线
  不变（M86-S2 引用裁剪逻辑按函数/全局引用图切 BCModule 子集，机制复用）。启动 main = 载入 BCModule →
  px_vm_entry 跑 Top bc。产物体积与现 fn_* 同级（8B×指令数 + 表）。
- `bootstrap/pxc`/`pxi` 重链为 VM 版；golden/compiler.c 换 golden/compiler.bc（或 C 内嵌表）。
- 旧 C 递归引擎（fn_* codegen + runtime px_call 直调）在 S3-C 末尾保留代码路径但退出默认管线，S4 评估删除。

---

## 六、精确 GC 设计（S3-D，吸收 issue28 根治验收）

- 根集合改为：**每线程活动 VM 帧槽数组 + 全局槽数组 + C 桥暂存根（原生桥）+ g_globals 兼容表（保留）**。
- 原生桥暂存根：S1 估 ≤60 处（px_call 20+ 调用点持有临时 LXValue 跨回调）。机制沿用 g_tmp_root/px_push_root
  扩展或引入 RAII 式 push/pop 宏；**S3-D 先跑 spawn/回调/生成器专项压测 + g_gc_debug=1 核对标记数**。
- cell（D6）/PxVMFunc/BCModule 常驻对象：BCModule 生命周期 = 进程级（程序主体，不回收）；cell 与普通对象同管。
- STW：保留信号式全线程暂停（现机制），但扫描面从「整 C 栈逐 word」缩为「帧槽精确标记」→ 暂停与线程栈
  大小/深度解耦（issue28 STW 尖刺的 VM 根治）；堆归还（slab 空页）复用 issue28-B2 已落地机制。
- 验收（对齐 issue28 §7 + M89_PLAN）：单发 p95≤50ms、500 并发 p50≤200ms、堆回落基线。分代/增量/并发标记
  列为后续（若 D 段达验收则不追加入口，避免 M89 膨胀）。

---

## 七、批次拆分（S3 微步清单，每步 = commit + 编译 + verify）

> 微步纪律：每步结束仓库可编译、相关 verify 绿；发射器/VM 改动同步 golden/自举证明（M89_PLAN §六）。

### S3-A 骨架（发射器 + VM 最小闭环，≈2 周）
- A0 基建：`runtime/vm.h/vm.c` 骨架（PxVmState/PxFrame/指令分发循环 + px_vm_entry + PxVMFunc 对象分配）；
  `selfhost/bc_emit.px` 骨架 + `compiler.px` 加 `bc` 子命令（AST→BCModule 序列化预览 dump）。
- A1 常量池/名字池/槽编号：发射器收集 K/N/G/Func 表；LOADK/IMM/MOV/GETG/SETG/SRCLINE/RET/RET0/HALT。
- A2 运算 + 字面量 + 顶层顺序执行：A 表/B 表指令全量；`examples/fib.px`/hello 在 VM 上 stdout 与旧轨一致。
- A3 控制流：JMP/JMPT/JMPF + if/while/for/break/continue + 短路 and/or/??（D5）+ IfExpr/Block。
- A4 调用闭环：CALL/参数连续区排布/默认参数/RET；递归（fib/阶乘）深链压栈安全；px_method 桥 CALLM 原型。
- A5 错误：TRY/FORCE + 顶层 ? 语义 + 现场（SRCLINE/函数名）比对现 px_error 文案。
- **里程碑门**：cases s01–s07 + examples/hello/fib 在 VM 与旧轨 stdout 逐字节一致（vm_ab.sh v1）。

### S3-B 全构造（≈2 周）
- B1 容器/字段：INDEX/SETIDX/SLICE/GETF/SETF/GETF_OPT + list/tuple/dict/str 全操作；cases s03/s09/s10。
- B2 构造/类型：NEWLIST/NEWTUPLE/NEWDICT/NEWSTRUCT/NEWENUM/LISTPUSH + struct/enum/const enum/类型元数据表；
  cases s05/s08/s11；impl 方法注册 "Type.method" → G 槽（D8）。
- B3 推导式/match/生成器：ListComp/GenExp/DictComp → NEWGEN + 分支循环展开（映射 cg_comp_*）；match/select
  模式匹配展开（映射 cg_gen_select/pattern_cond）；cases s04/s13 + m34_gen_lazy/m25 系列。
- B4 闭包 P1（无捕获）：Closure tag → PxVMFunc + 嵌套 fn 全局化；m25_closure_gc 通过；**捕获面扫描**定 P2 时机。
- B5 并发/IO 桥：spawn/chan/select/mutex/rwlock/FFI/http_serve 等在 VM 上跑通（C 桥语义不变）；
  examples/concurrent*/http_serve_unix/m83 用例 VM 对拍。
- **里程碑门**：cases 全量 + examples 全量 VM vs 旧轨 stdout 一致（vm_ab.sh v2，服务类行为断言）。

### S3-C 自举收敛（≈2 周）
- C1 自举：compiler.px（+bc_emit 自身）在 VM 上编译自身 → 字节码镜像；与现 C 递归产物编译结果对拍一致。
- C2 golden 切换：golden/compiler.c → BCModule 镜像（字节码逐字节对拍）；自举证明规则更新；bootstrap/pxc/pxi
  重链 VM 版；构建/发布管线切 BC 产物（tools/px build 默认）。
- C3 闭包 P2：upvalue cell 真捕获（D6）；px run（原 pxi 路径）切 VM；pxi 树遍历保留兜底开关，interp golden 在 VM 跑通。
- C4 方法分派优化（可选）：模块内 struct 方法静态化 CALLM→CALL 快路径（免 px_method 锁哈希）；
  TAILCALL/@tailrec。
- **里程碑门**：自举证明 rc=0（BC 镜像）；pxi 退役（树遍历代码保留兜底）；全量回归绿。

### S3-D 精确 GC（≈2 周）
- D1 根切换：帧槽 + 全局槽 + 原生桥暂存根标记；退役整栈保守扫描（保留信号 STW 或降频）。
- D2 原生桥根登记（≤60 处）+ cell/PxVMFunc 标记。
- D3 issue28 验收压测：单发 p95≤50ms / 500 并发 p50≤200ms / 堆回落；g_gc_debug 核对标记数；spawn/回调/生成器专项。
- **里程碑门**：issue28 §7 验收全过。

### S4 收口（≈0.5–1 周）
自举证明（BC 镜像）+ 全量回归（vm_ab v2 三实现两两对拍）+ 重链 bootstrap + 文档（spec/README/CHANGELOG/M89_PLAN）
+ tag v0.2.0-m89。

---

## 八、风险与决策记录（S2 新增）

| # | 风险 | 决策/预案 |
|---|---|---|
| R1 | 发射器工作量 ≈ codegen 全量重写 | 槽号=现 _v 编号 → 机械改写；A0–A5 先覆盖最小闭环再铺全；codegen 保留作基准直到 S3-C |
| R2 | 方法调用走 px_method 锁哈希（性能） | v1 保语义走桥；S3-C C4 静态化优化；issue28 GIL 主因（模块全局）已由 GETG 槽根治 |
| R3 | 闭包 P2（cell）滑期 | P1 先行（0 闭包自举不受阻）；捕获面扫描决定 P2 前置/后置；spec 语义为最终目标 |
| R4 | 旧 C 产物 + VM 产物并存期 GC 根面不一 | 过渡期统一按「栈扫描保守 + 帧槽」双根（VM 帧槽也进扫描），S3-D 才切纯精确；D2 的 trampoline 判别保证 px_call 分派正确 |
| R5 | 字节码/表膨胀 | 8B 定长 + K/N 去重 + G 槽复用现 cg_globals；产物体积与 fn_* 同级（S1 §6 预算） |
| R6 | pxi 退役连带回归（~3300 行） | 保留兜底开关到 S4；interp golden 先迁 VM 跑通再删 |

---

## 附：S2 执行状态
> 状态：✅ **完成（2026-09-08）**：本设计文档定稿（D1–D8 决策 + 指令集 spec + 帧/错误/闭包/GC 设计 +
> S3-A..D 微步清单 + 风险记录）。待 M89_PLAN §四 S3 启动。

---

<!-- 本文件由东月维护，属 M89 里程碑文档族：M89_PLAN.md（总纲）→ M89_vm_prestudy.md（S1 详勘）→ 本文件（S2 设计） -->
