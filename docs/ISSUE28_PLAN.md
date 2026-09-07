# ISSUE28_PLAN · GC STW 尖刺 + 堆不回收 + 并发吞吐止血批次（qg-issue 28）

> 状态：🆕 **2026-09-07 立项开工**（M89-S1 VM 详勘预研后，按预研结论"先止血、M89 内根治"的接力路径）。
> 关联：qg-issue 28（GC stop-the-world 周期尖刺 + 堆内存不回收 + 吞吐 GIL 效应，仍阻塞 ws-approve .px 化 #47）。
> 处理人：东月（dy.wsai.chat）。官方基准：github.com/NanzhanGroup/PuXian（HEAD 0.2.0 / M89-S1-12020c9）。

## 〇、一句话

M88-B 修好「万级空闲连接线程恒定」，但 m88b 实测 **单发 p95=331ms/max=520ms 周期尖刺、堆 40MB→1.1GB 只涨不落、500 并发 p50=5.2s（GIL 串行化）** 仍阻塞 #47 上线。本批次在**保守 GC 框架内**止血（B1 GC 延迟到安全点削峰、B2 slab 空页归还 OS、B3 全局表互斥锁→读写锁破 GIL），M89（VM 化精确 GC）为最终根治（分代/增量/精确根），本批次代码与结论直接喂给 M89。

## 一、根因（M89-S1 代码级坐实 + m88b 实测）

| 层 | 现象 | 代码证据 |
|---|---|---|
| GC STW | 每次对象数越阈值即在分配热路径内联全量 STW 回收；停顿随 worker 数/栈深增长，周期 ~11 请求一刺 | `gc_register` need→`px_gc_collect()` 内联；STW 逐线程信号暂停 + 保守扫栈 + 全表 sweep |
| 堆不回收 | 请求对象释放回 slab 空闲链表，但整块 slab 从不 munmap → RSS 只涨不落 | slab 无"全空页归还 OS"路径（无 trim/madvise/munmap） |
| 吞吐 GIL | px_get/set_global 全走一把互斥锁 g_globals_mu，500 并发全局访问串行化 | M55：全局符号表 `pthread_mutex_t g_globals_mu` |

## 二、S 级拆分（每步独立 commit + 编译 + 回归）

| S | 内容 | 验收 |
|---|---|---|
| B3 | 全局符号表锁 M55 互斥锁→**读写锁**（px_get_global/px_global_native/struct 方法查找=读锁并发；px_set_global/GC 根扫描=写锁独占；锁序 g_gc_mu→g_globals_mu 不变） | 编译过；m82 8 项 PASS；并发 spawn get/set 压测无损坏/不崩；GC 根扫描语义不变 |
| B2 | **slab 空页归还 OS**：GC sweep 后摘除并 munmap 完全空闲的非头 slab（每 class 保留头 slab 作分配缓冲防抖动），反查数组同步移除 | 编译过；GC 后 RSS 回落可测（压测后回落基线±20%）；m82 PASS |
| B1 | **GC 延迟到安全点**：多线程服务模式对象越阈值不再内联 STW（置 pending），改在请求间安全点回收（fserve/px_pool worker 循环顶）+ 硬上限兜底内联；单线程 CLI/解释模式保持原内联零回归；GC 摘要打印耗时便于观测 | 编译过；服务模式在途请求不吃 GC 停顿（可测）；单线程/pxi 行为不变；m82/m83_s6 PASS |
| S4 | 收口：全量回归 + 自举证明 + 重链 bootstrap + 文档同步（CHANGELOG/spec/qg-issue 28 状态）+ 记录 M89 吸收项 | 回归全绿 |

## 三、验证标准（issue 28 §7 目标 + 本环境可达子集）

- 单发 p95 ≤ 50ms / max ≤ 100ms 无周期尖刺；500 并发 p50 ≤ 200ms 0 超时 —— **全量标准在观音/清歌 ws-approve 隔离环境跑**（本环境以回归 + 专项可测项为准）；
- 内存：压测后回落（B2 空页归还）可测；
- 回归总闸：m82 + m83_s6 + m23a + pxi 冒烟 + 自举证明 rc=0。

## 四、不在本批次（M89 吸收项）

- 分代/增量 GC、精确根（M89 VM 化 D 段）；并发标记并行化（可入 M89 或独立小批）；
- 默认 PX_SERVE_WORKERS=256 空载烧 CPU（issue28 §3.5）：运维即设 PX_SERVE_WORKERS=8~32，代码默认值调优并入 M89 批次评估；
- 全局表 O(n) 线性扫描改哈希（随 VM 化名字表/槽位直读一并设计）。
