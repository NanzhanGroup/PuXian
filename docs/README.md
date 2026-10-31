# PuXian 文档索引（docs/）

> 本文件回答三个问题：**哪个文档管什么** · **我该从哪读起** · **哪些是历史档案（可以不全信）**。
> 维护约定：新增/改名文档时同步本索引；「当前态」文档（A 区）随里程碑更新，历史档案不追溯修改。
> **M366 整理**：64 份一次性开工计划/预研已移入 [`archive/`](archive/README.md)，`docs/` 根只留当前态文档。

---

## A 区 · 当前态文档（以最新里程碑为准，改了代码就要同步）

### A1 语言与语义

| 文档 | 管什么 | 谁该读 |
|---|---|---|
| [`spec.md`](spec.md) | **语言规格说明书**（唯一权威语义来源）：词法 / 类型 / 表达式 / 语句 / 函数闭包 / 并发 / 模块 / 双模式执行 / 标准库约定 / 错误码（§11）/ 工具链接口（§12）/ 砍掉清单 / edition / Python 兼容边界 / 示例 / **§17 语义一致性收口** | 所有人；改语言语义前**先读 §17** |
| [`ERROR_CODES.md`](ERROR_CODES.md) | **错误码总表 + 逐族口径**：R1001–R1008 的触发条件与**精确文案**；专章含「同名两门（函数面 ⇄ 方法面）」§2.3.2 · 逐位置 × 错类型 §2.3.6 · **真值性表 §8** · 文本语义接口 §6.5 / §6.12–6.13 | 改内置 / 方法面 / 错误文案的人；写门对文字的人 |
| [`PUXIAN_CHEATSHEET.md`](PUXIAN_CHEATSHEET.md) | **AI 速查包**（给大模型整包喂）：易错事实表、native 名册、三轨差异与统一口径、逐里程碑「事实 NNN」条目 | 让 AI 写 `.px` 的人；排障时先搜这里 |
| [`MINI_SUBSET.md`](MINI_SUBSET.md) | **Mini 子集规范**：自举编译器只会正确编译的语言面（支持 / 明确排除 / 已知限制）。写 `selfhost/*.px` 的硬约束。**§三 的「明确排除」表在 M362 逐条复验过**（标 ✅ 者其实已支持） | 改编译器 / 解释器源码的人 |
| [`DICT_STRICT_MIGRATION.md`](DICT_STRICT_MIGRATION.md) | **严格化迁移说明**（M163–M167）：字典键、迭代快照、解包形状、`items()` —— 老代码迁移怎么改 | 存量 `.px` 维护者 |
| [`DEFECT_NUMBERING.md`](DEFECT_NUMBERING.md) | **缺陷编号纪律**（M241）：编号是全仓最常引用的「坐标」—— 指错**不会让任何门变红**（不改变行为、不改变产物） | 写 CHANGELOG / 注释 / 报告的人 |

### A2 runtime 契约（改 C 运行时前必读）

| 文档 | 管什么 |
|---|---|
| [`GC_ROOTS.md`](GC_ROOTS.md) | **GC 根面规则**：两条硬约束（容器创建后必须登记 · 登记必须紧跟创建、先于下一次分配）· 审计器判据与三条窄条件豁免 · 隔离点两类语义 |
| [`GC_TRIGGER.md`](GC_TRIGGER.md) | **GC 何时跑、页何时还**（M317）：面向运维的「我的服务为什么一直涨」回答口径 + 实测数字 |
| [`GC_MEMORY_LAYERS.md`](GC_MEMORY_LAYERS.md) | **GC 内存分层**（M318）：RSS 涨了先问**哪一层**在涨（三层模型），再决定改什么 |
| [`GC_STOP_PROTOCOL.md`](GC_STOP_PROTOCOL.md) | **暂停信号 × 互斥锁协作**（M312）：临界区屏蔽 `SIG_GC_STOP`，解除必须**在放锁之后**（反序会留下秒级停顿窗口） |
| [`GC_COVERAGE.md`](GC_COVERAGE.md) · [`GCSTRESS_LEDGER.md`](GCSTRESS_LEDGER.md) | GC 根登记**覆盖面台账**（覆盖率怎么算的）· GC 压力筛**分批台账**（哪些语料已过压力档） |
| [`LOCK_ALLOC.md`](LOCK_ALLOC.md) | **锁内可失败分配**收口（持锁时 `xmalloc` 失败会带走进程；13 → 2 的收敛过程与判据） |
| [`TABLE_PTR_CONTRACT.md`](TABLE_PTR_CONTRACT.md) | **表元素指针跨锁**契约（一类结构：取指针出锁 ⇒ 悬垂） |
| [`IO_EINTR.md`](IO_EINTR.md) | **EINTR 族**收口：一条语义、一份实现（含 GC STW 信号打断 `recv` ⇒ 误判对端关闭的真实事故） |
| [`TLS_CERT_RELOAD.md`](TLS_CERT_RELOAD.md) | 证书**热加载**与并发 TLS 握手的竞态（UAF）与修法 |

### A3 门与判据基础设施

| 文档 | 管什么 | 谁该读 |
|---|---|---|
| [`NEW_GATE_CHECKLIST.md`](NEW_GATE_CHECKLIST.md) | **新建「门」的落盘检查清单**（M247）：生成器 / 复用与 import / 性能 / **负控** / 判据 / 注册六类必查项 + 已被守卫机械化的条目 | 写门的人；**改 `examples/*/verify.sh` 前先过一遍** |
| [`GATE_ASSERTIONS.md`](GATE_ASSERTIONS.md) | **门层判据守卫口径**（M285）：① 负控**打桩锚点在位** ② 判据串**在场** ③ 三条**刻意不看** | 写门 / 改门的人 |
| [`GATE_ISOLATION.md`](GATE_ISOLATION.md) | **门间隔离**（M264）：共用固定 `/tmp` 路径的白名单制 —— 门与门之间不得互踩 | 并行跑门 / 加新门的人 |
| [`GATE_TIMEOUTS.md`](GATE_TIMEOUTS.md) | **门超时口径**（M311 · 缺陷 532）：声明写在注册源 · 三层优先级 · 配额告警 · 守卫 P1/P2/P3/P5 · 实测基线怎么刷新 | **加/改门的人** |
| [`GATE_COST_BREAKDOWN.md`](GATE_COST_BREAKDOWN.md) | **全量门成本模型**（M311 · B）：分片档实测分解 · **两个被否证的假设** · 剩余杠杆与实验协议 | 想「压缩全量门时间」的人 |
| [`GATE_FLAKINESS.md`](GATE_FLAKINESS.md) | **判据不稳定性台账**（M340 建立 → M344 起持续收口）：每条「全量门里偶发红」的**归因 + 修法 + 是否复现**。**看这张表判断「这次红是不是又抽中了」** | 看到「单跑绿、全量红」的人 |
| [`GATE_FLAKINESS_AUDIT.md`](GATE_FLAKINESS_AUDIT.md) | **时序敏感性盘点**（M341）：哪些门「拿一次观测下生死」的风险最高（**是可复核名单，不下结论**） | 同上；准备做重跑实验的人 |
| [`GATE_FLAKE_FREQ.md`](GATE_FLAKE_FREQ.md) | **窄口径门的动态重跑结果**（M342）：126 次运行 / 0 次命中，以及它**如何否掉原来的方法假设** | 想「再跑几遍就能定性」的人（先读这份） |
| [`GUARDS_SURFACE.md`](GUARDS_SURFACE.md) | **守卫扫描面体检表**（M326）：同一缺陷族的第 3 次复发不是判据错，而是**扫描面**比判据更容易错 | 写 `selfhost/check_*` 的人 |
| [`EMITC_FREEZE_DEPS.md`](EMITC_FREEZE_DEPS.md) | **发射冻结门 · 类别 B 的已知盲区**：传递依赖（改 registry 模块会让进口它的用例被判「发射回归」） | 遇到「我只改了库，怎么用例红了」的人 |

### A4 协议与连接口径

| 文档 | 管什么 |
|---|---|
| [`HTTP2_DECISION.md`](HTTP2_DECISION.md) | **HTTP/2 与 HTTP/3 的口径**（M180）：**h2 不做**（理由 + 迁移动作 + 重评估触发条件）· H3 = `opts{"http3": true}` + 自动 `Alt-Svc` |
| [`HTTP3_STANCE.md`](HTTP3_STANCE.md) | HTTP/3 立场与**能力自证**（`native_symbols()` 让「有没有某能力」可查询） |
| [`HTTP_GZIP_NEGOTIATION.md`](HTTP_GZIP_NEGOTIATION.md) | gzip **内容协商**口径（q 值 / `*` 通配 / token 大小写 / `opts{"gzip": false}` 开关） |
| [`HTTP_CLIENT_HEADERS.md`](HTTP_CLIENT_HEADERS.md) | **HTTP 客户端自管请求头口径**（M316 · 缺陷 550）：运行时补的头「调用方给了就不补、给了空值当没给」 |
| [`WS_CONN_LIFECYCLE.md`](WS_CONN_LIFECYCLE.md) · [`WS_UPGRADE.md`](WS_UPGRADE.md) | WebSocket 连接生命周期（引用计数 + 延迟释放）· HTTP 请求 → WebSocket **升级接管** |
| [`QUIC_CONN_LIFECYCLE.md`](QUIC_CONN_LIFECYCLE.md) | **QUIC listener / 连接 生命周期**（M296 · 缺陷 511/512）：listener 能否连收多连 · UDP socket 谁关（fd 归属判据）· 非法包丢弃不判死 |
| [`MYSQL_DRIVER.md`](MYSQL_DRIVER.md) | **MySQL 驱动协议口径**（M313）：握手偏移 · `CONNECT_WITH_DB=0x8`（不是 `0x800`）· **读超时语义** · IO 出错即关连接 · `caching_sha2` 只支持 fast auth |
| [`TCP_NODELAY.md`](TCP_NODELAY.md) | **服务端 socket 口径**（M313 · 缺陷 539）：accept 必须显式设 `TCP_NODELAY`（Linux 不继承）· 判据为什么用**状态位**而不是墙钟 |
| [`ONNX_QUANT.md`](ONNX_QUANT.md) | **ONNX 量化算子口径**（M306 / S2）：4 个算子的精确语义 · 三条统一口径 · 零点形态表 · 覆盖边界 |
| [`ONNX_INFER_OPS.md`](ONNX_INFER_OPS.md) | **ONNX 推理算子口径**（M308 / S4）：算子清单 + 三条口径 + 判据在哪（三条腿 + 等价腿 + 6 道负控）· **覆盖边界** |
| [`ONNX_ORT_KERNEL.md`](ONNX_ORT_KERNEL.md) | **onnxruntime 的 MatMulInteger(u8s8) kernel 偏差**（M314/M315）：我方与规范一致、上游 1.19.2–1.31.0 全档偏差；含「为什么暂时不发上游 issue」 |

### A5 性能与度量口径

| 文档 | 管什么 |
|---|---|
| [`PERF_BASELINE_V2.md`](PERF_BASELINE_V2.md) · [`PERF_BASELINE_METRICS_SPEC.md`](PERF_BASELINE_METRICS_SPEC.md) | **性能基线 v2** 与**度量口径规范**（指标定义 · 采集方式 · **跨平台对比的口径警告**） |
| [`PX_RUN_FAST.md`](PX_RUN_FAST.md) | `px run --fast` 用户脚本构建缓存（11.6 s → ~30 ms；默认 `px run` 一字节不变） |
| [`STR_CONCAT.md`](STR_CONCAT.md) | 字符串拼接 `+` / `+=` 的性能口径：**修的是常数不是复杂度**，以及复杂度为什么修不了 |

### A6 生态与分发

| 文档 | 管什么 | 谁该读 |
|---|---|---|
| [`ECOSYSTEM.md`](ECOSYSTEM.md) | **生态总览**：公开库的定位与导出 API、示例能力导航、消费路径、机器索引与防漂移 | 写库 / 用库的人 |
| [`ECOSYSTEM_GAPS.md`](ECOSYSTEM_GAPS.md) | 写库规范 checklist + 语言缺口评估 | 写库 / 报缺口的人 |
| [`PX_DEF_TRIAGE.md`](PX_DEF_TRIAGE.md) · [`UPSTREAM_020_DEFECTS.md`](UPSTREAM_020_DEFECTS.md) | **第三方缺陷判定表**（逐条：真缺陷 / 我方文档缺口 / 上游误读）· 上游 0.2.0 用例回归结果与 XFAIL 登记 | 对接第三方 / 评估生态兼容的人 |
| [`ROADMAP.md`](ROADMAP.md) | 路线图：能力基线、已完成主线、远期方向、语言面欠账 | 想了解进度与方向的人 |
| [`PXPKG_SYNC_PLAN.md`](PXPKG_SYNC_PLAN.md) | **立项文档**：`pxpkg sync`（registry 在线分发）—— 问题输入 / 四件工作 / 对旧决策的重评估 | 做包分发 / 镜像的人 |
| [`RELEASE_PROCESS.md`](RELEASE_PROCESS.md) | 发布 SOP：tag 驱动自动发布、发布物清单、漏打 tag 守卫、幂等发布步 | 发版的人 |
| [`GAP_ANALYSIS.md`](GAP_ANALYSIS.md) | 能力差距分析（边缘设备 / 2D-3D 游戏两条线的差距清单） | 排期与选型 |
| [`PXML.md`](PXML.md) | PXML 配置语言规范（`std.pxml` 背后的语言） | 用 PXML 的人 |

### 机器索引（不要手改，用生成器）

| 文件 | 生成器 | CI 门 |
|---|---|---|
| [`native_index.json`](native_index.json) | `tools/gen_native_table.sh`（扫 `runtime/*.c` 的 `px_set_global`） | 重跑后 `git diff --exit-code`；另由 `check_native_coverage.py` 对覆盖面台账 |
| [`ecosystem_index.json`](ecosystem_index.json) | `tools/gen_ecosystem.px`（扫 `stdlib/*.px` 顶层 `def`） | 同上 |

> 改 `stdlib/` 或 runtime native 名册 ⇒ **必须重跑生成器并同步 `ECOSYSTEM.md` / `PUXIAN_CHEATSHEET.md`**，
> 否则 CI 的「派生索引防漂移」步会红（M235 缺陷 355 的教训：名册同步了、**门忘了注册** ⇒ 用户可见面回归）。

---

## B 区 · 按角色读

- **第一次接触 PuXian**：根 [`README.md`](../README.md)（快速开始 + 能力一览）→ `spec.md` §1–§7 → `ECOSYSTEM.md`。
- **让 AI 写 PuXian**：把 `PUXIAN_CHEATSHEET.md` + `spec.md` §17 一起喂；速查表是为「整包投喂」写的。
- **写库 / 写生产应用**：`ECOSYSTEM.md` + `ECOSYSTEM_GAPS.md` §1 checklist + `DICT_STRICT_MIGRATION.md`。
- **改编译器 / 解释器 / runtime**：`MINI_SUBSET.md` → `spec.md` §17 → 根 `CONTRIBUTING.md` → **`NEW_GATE_CHECKLIST.md`** →（改 C 运行时）`GC_ROOTS.md` → `GC_STOP_PROTOCOL.md` → `ROADMAP.md`。
- **写门 / 改门**：`NEW_GATE_CHECKLIST.md` → `GATE_ASSERTIONS.md` → `GATE_ISOLATION.md` → `GATE_TIMEOUTS.md`。
- **发版**：`RELEASE_PROCESS.md`（含 tag 命名规则与漏打 tag 守卫）。
- **对接第三方生态**：`ECOSYSTEM.md` → `PX_DEF_TRIAGE.md` → `UPSTREAM_020_DEFECTS.md`。
- **排查「同一份源码两轨结果不同」**：先查 `spec.md` §17 与速查表「三轨差异」条目 —— 若规则已收口，那就是 bug，请带最小复现提 issue。
- **排查「门为什么绿/红得莫名其妙」**：`GATE_ASSERTIONS.md`（判据自身失效的三种形状）→ `GATE_ISOLATION.md` → **`GATE_FLAKINESS.md`（这次红是不是已知的偶发）**。
- **排查「服务内存一直涨」**：`GC_MEMORY_LAYERS.md`（先分层）→ `GC_TRIGGER.md`（何时跑/何时还）→ `GC_ROOTS.md`（根登记漏了？）。

---

## C 区 · 历史档案

**全部在 [`archive/`](archive/README.md)**（M366 从 `docs/` 根部迁入，共 64 份）：
各里程碑的**开工计划 / 预研 / 阶段评估**（`M53_PLAN.md` … `M123_PLAN.md`、`M308_PLAN.md`…`M311_PLAN.md`、
`M89_vm_*`、`M90_S*`、`M95_S5_PLAN.md`、`M104_native_prestudy.md`、`M353_QUIET_PREMISE.md`、`M355_PIPE_SWEEP.md`、`ISSUE28_PLAN.md`）。

> 它们的「结论」只在**当时**成立；后续里程碑可能已推翻（例：`M104_native` 时代的 native 名册与今天不同）。
> 要现状 ⇒ 读 A 区 + `CHANGELOG.md` 对应里程碑节（**CHANGELOG 是唯一权威归档**）。

> 判断某文档是否「当前态」的快捷办法：看它头部有没有「更新：<里程碑>」行；A 区文档都会随里程碑刷新头部。
> 本索引的 A 区覆盖 `docs/` 根部全部非历史档案；新增一次性计划文档请直接写进 `archive/`。
