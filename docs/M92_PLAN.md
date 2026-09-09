# M92_PLAN · 精确 GC 终极项（退役整栈保守扫描 + 原生桥根登记）

> 状态：🚧 **进行中（S2c 完成）**。S1 设计定稿 + S2a precise 框架 + S2b 语言核心层
> 批量登记已合入（2017101/a55a13c）；S2c 登记批次 2（服务/IO 桥）已合入（本 commit）：
>   http_conn_worker（http_serve/http_serve_unix 主 worker：headers/req/form/resp 迭代
>   作用域登记——崩点修复，precise 压测实锤 http_conn_worker→px_dict_set→UAF）、
>   sse_conn_worker（headers/req）、sse_parse_event_c（累积 dict）、bi_http_request/
>   bi_http_unix（客户端响应 dict）、px_conn_worker（px_serve 主 worker headers/req/form）、
>   route_match/px_route_try_dispatch（params + 中间件/handler 返回值）、px_http_dispatch
>   vhost handler 段。**附带修复**：px_root_push/pop/keep 加 SIG_GC_STOP 屏蔽（根栈操作
>   非原子 → GC handler 快照半态 → 并发随机 UAF；与 list/dict 结构修改同模式）。
>   已知残余：PX_GC_INLINE=1 强化模式 precise 服务长跑 ~2 万请求级稀有崩溃（崩 GC 标记
>   hash set 坏指针；INLINE 强制内联 GC 至任意分配点 → executor 极稀有交错；真实服务
>   deferrable 安全点 GC 无窗口——无 INLINE precise 19.2 万请求稳定、conservative 15 万
>   稳定），列 issue 专项，不阻塞（见 §五 S2c 验收记录）。
>   剩余 S2d（bc_emit 产物插桩 px_gc_set_precise）→ S3 收口。
> 上游：M89_PLAN §S3-D 后置决策、docs/M89_vm_design.md §六（精确 GC 设计）。
> 基线：v0.2.0（M91 收口 3bbbee4）。

## 〇、一句话

把 GC 根面从「全局槽 + 暂存根 + **整 C 栈保守扫描**（逐 word 哈希查）」换成「全局槽 +
VM 帧槽（跨线程已有）+ **native 桥登记根**（TLS 根栈）+ 暂存根」——退役保守栈扫描，
消除 STW 与线程栈大小/深度的耦合（issue28 STW 尖刺的 VM 根治），为帧协程/native/GC
演进提供单一精确根面。cell 标记（后置决策③）随闭包 P2 真捕获立项（现无 cell 对象，
见 §四）。

## 一、触发条件核对（M91 后全部满足）

| 后置条件 | 状态 |
|---|---|
| 默认轨切 VM（M91） | ✅ px build 默认产物 = BCModule 字节码 VM 轨 |
| 引导二进制重链吸收最新 runtime（候选①批次） | ✅ bootstrap/pxi、pxi_vm 已重链（含 F3-fix） |
| 旧 C 轨降级为逃生舱（--c） | ✅ 保留，本里程碑定义其 GC 模式（见 §三 D2） |

## 二、D0 侦察量化（2026-09-10，dongyue）

- **保守扫描现状**（runtime/runtime.c）：根3（本线程栈 gc_scan_stack + 寄存器）+ 根4
  （暂停线程栈 gc_scan_thread_stack + 寄存器）——整活跃栈区 [RSP, 栈底) 逐 8B word
  哈希查对象表。STW 耗时与栈深/线程数线性相关（服务端 256 worker 全停后逐条扫栈）。
- **VM 帧槽精确根已在**（S3-D-1）：单线程 px_vm_gc_mark + 并发 px_vm_gc_mark_state
  （暂停线程 ti->vm_state 跨线程标帧槽）——精确根面基础设施已具备。
- **登记改造面**：runtime 全量 413 个 bi_/px_* 桥；其中**含跨 GC 点调用**
  （px_call/px_method/px_spawn/call_with_self/px_gc_poll/px_gc_collect）的文件分布：
  runtime.c **76**、vm.c 7、runtime_ws.c 5、runtime_route.c 2、runtime_ffi.c 1，
  其余（加密/xml/zip/zlib/sqlite/rsa/quic/h3/h2/ed25519）0。真正需登记 = 「持有
  跨 GC 点局部 LXValue 引用」的桥，按函数去重估 **60~100 处**（与 M89 预估 ≤60 同量级，
  机械性强，风险在漏登记 → use-after-free，须 precise 模式全量回归 + 低阈值压力暴露）。
- **C 轨逃生舱产物**（fn_*）：局部 = C 栈 LXValue（_vN 局部 + 栈上数组），依赖保守扫栈
  作根（s01 golden 样例实证：fn_main 内 _v4/_v5 + (LXValue[]){...} 栈临时数组）。
- **cell（后置③）**：bc_emit.px PxVMFunc.upvals 恒 NULL（P2 真捕获未实现）；codegen
  fn_closure 仅为命名计数（selfhost 0 真捕获）→ **无 cell 对象可标**，本里程碑登记为
  「随闭包 P2 立项」，不空转实现。

## 三、设计定稿（D1–D4）

### D1 · precise/conservative 双模式 GC
- runtime 增 `g_gc_precise`（默认 0 = conservative，行为零变化，逃生舱/C 轨全回归安全）。
- **precise 模式（g_gc_precise=1）**：GC 根 = 全局槽 + VM 帧槽（跨线程）+ TLS 登记根栈 +
  暂存根；**跳过 gc_scan_stack / gc_scan_registers / gc_scan_thread_stack**。
- 模式由产物决定：VM 轨产物（bc_emit 生成 .c 的 main）调 `px_gc_set_precise(1)`；C 轨
  逃生舱产物不调 → conservative。产物二选一（px build 一次只出一种轨）→ 无同二进制
  双轨混合，模式判定天然干净。
- env `PX_GC_PRECISE=1`（仅 debug/回归驱动，文档注明危险：C 轨产物 + precise 会漏根）。
- STW 信号式暂停协议保留（不变量不动），只缩扫描面。

### D2 · C 轨逃生舱（--c）处置
- 逃生舱产物保持 conservative（不插 px_gc_set_precise）→ fn_* C 栈局部继续受保守扫栈保护，
  逃生舱语义零变化。精确化收益只对默认 VM 轨兑现；逃生舱为纯计算热点逃生路径，保守扫栈
  残留不影响主线 STW（默认轨已退役）。逃生舱彻底精确化 = codegen 产物根登记改造
  （golden 大迁移范畴），列后置（native 后端立项时一并）。

### D3 · native 桥根登记 API（TLS 根栈）
```c
void px_gc_set_precise(int precise);      // 1=precise / 0=conservative
void px_root_push(void);                  // 作用域开始（native 桥入口）
void px_root_pop(void);                   // 作用域结束（所有出口前；与 push 配对）
void px_root_keep(const LXValue* v);      // 登记局部引用（跨可能触发 GC 的调用前）
#define PX_KEEP(v) px_root_keep(&(v))     // LXValue 局部变量用法
```
- 实现：每线程 LXValue 动态栈 + 帧标记栈；GC precise 时本线程/暂停线程登记栈按
  px_gc_mark_slots 精确标记（值类型自带判别；冗余登记无害，漏登记致命）。
- 用法纪律（写进注释）：桥内「新建 LXValue 局部 → 在其后任何可能触发 GC 的调用
  （px_call/px_method/px_spawn/分配/收发/扩容）前」PX_KEEP；args 指向对象无需登记
  （调用者帧槽/上层根已保护）；helper 短生命周期无跨调用无需登记。
- 登记改造分文件批次（每批 precise 回归 + PX_GC_THRESHOLD 低阈值压力验证后合入）：
  S2b runtime.c 核心（px_call/px_method/call_with_self/px_spawn/px_chan_*/字典/列表桥）、
  S2c vm.c（CALL native 侧 abuf 已是帧槽拷贝、主要查 bi_vm_*）、S2d 网络/IO 桥
  （runtime_ws/route/ffi + runtime.c 服务池）。

### D4 · cell 标记（后置③归并）
- 现无 PX_CELL 类型/无 upvalue cell 对象（bc_emit upvals=NULL、codegen 0 真捕获）→ 不新增
  类型。文档记录：P2 闭包真捕获立项时 cell = 含 1 槽的 GC 对象，标记路径复用 PX_STRUCT
  单槽模式（gc_mark_obj 增分支即可），届时随闭包里程碑一并落地验证。

## 四、范围

- **在本里程碑**：precise 模式框架 + native 桥登记（runtime.c 全量 + 网络/IO 桥）+ VM 轨
  产物插桩 + 回归/压力验证 + 文档 + tag。
- **不在本里程碑（后置）**：C 轨逃生舱产物精确化（codegen 改造 + golden 迁移，随 native
  立项）；cell 对象实现（随闭包 P2）；分代/增量 GC（保留入口，D 段验收达标则不追加入口）。

## 五、S 级拆分（每 S = commit + 编译 + 验证绿，不混）

| S | 内容 | 验收 |
|---|---|---|
| S1 | 设计定稿（本文档）+ D0 量化 | 本文档 + 侦察数据 |
| S2a | precise 框架：runtime 双模式 + TLS 登记根栈 API + GC 根面接线（并发/单线程）+ 冒烟 | 默认模式全回归不变（diffcheck/vm_ab 抽查）；PX_GC_PRECISE=1 最小用例通过 |
| S2b | 登记批次 1：runtime.c 语言核心桥（累积构造/递归构造/json helper，19 函数 + 3 helper） | ✅ 完成（a55a13c）：precise 低阈值（thr=400/800）stress1+stress2 与 conservative 逐字节一致；conservative 零回归 m89_s3d 9PASS + vm_ab 38PASS/0GAP/0FAIL。压力集 examples/m92_precise/precise_stress2.px（dict keys/values、json roundtrip、json_path_set 深拷贝链、list 拼接/切片、sorted/reversed、split/join、list(str)、regex find_all/search/split、list_dir）；SEG3 json_path_set 暴露漏登记 → 补 json_path_set_at 整函数 |
| S2c | 登记批次 2：vm.c bi_vm_* + runtime_ws/route/ffi + runtime.c 服务池/IO 桥 | ✅ 完成（本 commit）：服务主路径登记（http_conn_worker/sse_conn_worker/sse_parse_event_c/bi_http_request/bi_http_unix/px_conn_worker/route_match/px_route_try_dispatch/px_http_dispatch vhost 段）；vm.c bi_vm_*/ws 主路径/ffi 审查确认无跨 GC 裸局部（无需登记）。验收（precise 低阈值/INLINE 强化）：
  - http_serve JSON 服务 8 并发压测 14400/14400 请求 0 错（登记崩点修复前 56 请求即崩）；
  - thr500/16 并发 14400 请求 0 错；m82 verify 8 PASS（http_serve_unix+http_unix 客户端路径）；m83_s6 verify 全 PASS（SSE/http_stream/断连）；
  - px_serve+route+middleware 并发压测 499/500（examples/m92_precise/s2c_pxserve.px 新增）；m31_vhost precise 自检 ALL OK（vhost handler 段）；
  - conservative 零回归：m89_s3d 9 PASS + vm_ab v2 38 PASS/0GAP/0FAIL + precise_stress1/2 逐字节一致；
  - 真实服务模式（无 PX_GC_INLINE，deferrable 安全点 GC）precise 19.2 万请求稳定；PX_GC_INLINE=1 强化模式 ~2 万请求稀有崩溃（GC 标记 hash set 坏指针，已记录 issue，见状态头）；
  - 附带修复：px_root_push/pop/keep SIG_GC_STOP 屏蔽（并发根栈快照原子性）。 |
| S2d | VM 轨产物插桩：bc_emit.px 产物 main 加 px_gc_set_precise(1)（C 轨 codegen 不加） | 默认 px build 产物 precise 运行全量回归绿 |
| S3 | 收口：precise 全量回归（vm_ab/diffcheck/m89_s3d/m82/m83 + 低阈值压力）+ 重链 bootstrap + 文档 + tag | 全绿 + tag v0.2.0-m92 |

## 六、风险与预案

| 风险 | 预案 |
|---|---|
| 漏登记 native 桥局部 → UAF（难调试） | PX_GC_THRESHOLD 低阈值（500~20000）放大 GC 窗口；precise 全量回归 + 并发/生成器专项压力；g_gc_debug 核对标记数 |
| precise 下 helper 短局部也被回收（误判短生命周期） | 保守登记（宁冗余无害）；px_root_keep 判定 is_obj 精确；低阈值压测覆盖 |
| 逃生舱/旧 C 产物误开 precise | 插桩只在 VM 产物 main（bc_emit 生成）；env 文档注明仅 debug |
| 并发路径暂停线程登记栈快照不一致 | 暂停自旋期线程无代码执行 → TLS 根栈不变；handler 存 ti->roots/root_n（同 tmp_root 模式） |
| 回归面大 | 默认 conservative 零行为变化先行；precise 逐批文件开启 + 每批验证后合入 |
