# M109 计划：HTTP 响应头通路（handler 响应头透传族）

> 立项：2026-09-11 · 依据：`/data/qg-issue/36-puxian-vhost-response-header-whitelist/`（晨曦 QA 实测）· **用户 2026-09-11 拍板「单独立项」**（与 M107 性能、M108 连接生命周期并行）
> 范围：**Issue 36 主修**（`vhost()` handler 响应头被 `px_vhost_header_allowed()` 静默丢弃）+ **同族** `route_normalize()`（`runtime_route.c`，连 301 的 `Location` 都发不出）
> 基线：`main @ e130383` · runtime md5 `6cbede8a5457c36b72a07a4a8f488dce`
> 定位：**L0 runtime 正确性缺陷**（非性能、非连接、非崩溃：不报错、不打日志、调用方不可知）
> 状态：**S0 开工**（本文为 S0 交付物之一）

---

## 0. 为什么单独立项

Issue 36 与在办三项**无因果关系**，只是同一批「边缘 CDN 上线」中暴露的**独立面**：

| 症状 | 归属 |
|---|---|
| 单个响应头在客户端「消失」（配了但收不到、无报错） | **本里程碑（Issue 36）** |
| 请求没回来 / 偶发失败 / 第 2 次失败 | M108（Issue 31/32，连接生命周期） |
| 请求回来了但太慢 | M107（Issue 33/34/35，性能） |

合批会让 M107 的**性能验收口径**（毫秒/系统调用次数）与 M109 的**语义验收口径**（哪些头到达客户端）互相污染；且 M109 的**违约点是把既有回归测试的期望固化了缺陷**（见 §4 红线③），改动必须带着"改测试"一起审。

---

## 1. 病灶（代码级，已核对）

| # | 位置 | 行为 |
|---|---|---|
| ① | `runtime/runtime.c:15978` `px_vhost_header_allowed()` | **默认拒绝**白名单（12 项）→ 不在表内即 `return 0` |
| ② | `runtime/runtime.c:16031`（`px_vhost_normalize` 通用分支） | `if (!px_vhost_header_allowed(hk)) continue;` **静默丢弃**（无返回/无日志/无告警） |
| ③ | `runtime/runtime.c:14352`（`{"file":…}` 流式分支） | 同上，`fextra[1536]` |
| ④ | `runtime/runtime.c:14394` / `:14337` | `extra[1024]` / `fextra[1536]` 写满即**整条丢弃**（非截断），实测单头值 **>970B** 即丢 |
| ⑤ | `runtime/runtime_route.c:287-290` `route_normalize()` | headers **只取 `Content-Type`**，其余**全部无视**（含 301/302 的 `Location`） |

**证据强度**：晨曦已给**隔离复现**（编译模式）——Mahesvara P0a 明确放行的 17 个头**只有 7 个到达**、ma-cache 的 `Age`/`X-Cache`/`X-Cache-Key` 全丢、`route()` 路径 5 个头全丢。现网 CDN 命中率 68% 却无任何 HIT/MISS 标识，**险些误判"边缘没缓存"**。

**引入史**：`da85065`（M57-S7，2026-09-03）当时只为救 `Location`+`Cache-Control`；此后 M104 ma-cache（`Age`/`X-Cache`/`X-Cache-Key`）、Mahesvara P0a 安全头均未同步 → 白名单**演化成本**被低估（每加一类头都要改 C 运行时，**漏改不报错**）。

---

## 2. 切片总览

| 片 | 内容 | 预期 | 风险 | 触发回归 |
|---|---|---|---|---|
| **S0** | 复现复核：跑 `repro.px` / `sweep.px` / `repro_route.px`，固化基线（7/17、970B 阈值、route 全丢）+ 读代码行核对 | 基线与 Issue 一致 | **低** | 无（只读） |
| **S1** | **判定哲学改造（A 方案）**：白名单 → **拒绝名单**（`Content-Length`/`Transfer-Encoding`/`Connection`/`Keep-Alive`/`Date`/`Server`/`Trailer`/`Upgrade`/`X-Request-Id`）+ **保留既有 CRLF 防护**；**同步改** `examples/m57_s7_vhost_headers.px` 的固化断言 | 17/17 到达；新增头零改 runtime | **中**（改的是"默认拒绝"→"默认放行"，必须证明**没有放松 CRLF 与长度类头**） | 中 |
| **S2** | 头缓冲 `extra[1024]` / `fextra[1536]` → **4096**；超限**不再静默**（至少 stderr 一行 + 计数） | 4KB 值完整透传；丢头可见 | **低** | 轻 |
| **S3** | `route_normalize()` **复用同一套判定 + CRLF 校验**（先把 `Location` 放出来） | route 路径 301/302 可跳转、headers 全通 | **中**（另一处归一化路径，需单独冒烟） | 中 |
| **S4** | 收口：双自举 + 全量门 + 端到端（真 serve 抓响应头）+ tag `v0.2.0-m109` + 交接自动更新线 | 修复真正上生产 | 低 | — |

**排序理由**：先 S1 —— 它是**根因**（默认拒绝），一次改完全部 10 个 DROPPED 头；S2/S3 是**同类缺陷的另一半**（预算静默丢 / route 路径），改法同源、可增量验证；S4 与前三个里程碑一并随发布包下发。

---

## 3. S0：复现复核（开工第一件事）

```bash
cd /data/qg-issue/36-puxian-vhost-response-header-whitelist
px build repro.px      && ./build/repro        # A/B/C 三段（17 头生存表 / ma-cache 三头 / 970B 阈值）
px build sweep.px      && ./build/sweep        # 值长度 900→1030 扫描
px build repro_route.px && ./build/repro_route # route 路径 5 头全丢
```

- **必须编译模式**（`px build`）：pxi 下 `vhost(host, def_fn)` 报"第二参数需要 docroot 字符串或 handler 函数"——**这是第二个已记录现象**，本里程碑**不改变**其结论，但需在 S4 决定是"修 pxi 支持 handler"还是"文档明示仅编译模式可用"（先记录，不阻塞）。
- S0 产出：基线数字入本文件 §3.1 + `docs/M109_PLAN.md` 附原始输出路径。

### 3.1 基线（待 S0 实测填入）

| 指标 | 现状 | 目标 |
|---|---|---|
| Mahesvara 放行的 17 头到达数 | **7/17** | **17/17** |
| ma-cache 可观测头 | `Age`/`X-Cache`/`X-Cache-Key` 全丢 | 全通 |
| `Content-Type` 独立通道 | 通 | 不变 |
| 单头值可透传上限 | **970B**（超则整条丢） | ≥4096B 完整透传 |
| `route()` headers | 仅 `Content-Type` | 与 vhost 同集合 |
| CRLF 防护（`/evilval`、`/evilkey`） | PASS | **仍 PASS** |

---

## 4. 语义红线（S1~S3 实施前逐条核对）

1. **长度类头必须由 runtime 自管**：`Content-Length` / `Transfer-Encoding` **不得**被 handler 覆盖（否则长度错乱、HTTP 走私面）。`Connection`/`Keep-Alive`/`Date`/`Server`/`Trailer`/`Upgrade`/`X-Request-Id` 同列为 runtime 自管 → **显式拒绝**。
2. **CRLF 防护不回归**：既有键/值 `\r`/`\n` 校验（`memchr`）**代码不删、语义不放宽**；拒绝名单是**叠加**在 CRLF 校验**之后**的一道，不是替代。
3. **既有回归期望须同步**：`examples/m57_s7_vhost_headers.px` 的 `assert not has(ks,"X-Custom")`（"非白名单头不得透传"）**本身把本缺陷固化成期望** → 改 A 后须改为**拒绝名单语义**（如断言 `X-Transfer-Encoding` 被拒、`X-Custom` 放行），并保留 `/evilval`、`/evilkey` 两例通过。
4. **头名大小写不敏感**：`strcasecmp` 语义保持；**头顺序保持** handler 插入顺序；`Set-Cookie` 多值**不合并**、不重复。
5. **runtime 自发的头不受影响**：静态文件路径的 `ETag`/`Last-Modified`/`Content-Range`/`Alt-Svc`/`Accept-Ranges` **不经** handler 白名单，改动前后逐字节相同。
6. **两条路径一致**：`px_serve`（vhost）与 `route()` 归一化后**同一套判定**；`pxi`/编译轨行为一致性不因本次改动而变差（pxi 的 handler 限制属既有现象，另记）。
7. **不静默**：任何"丢弃/超预算"都必须留下**可观测信号**（stderr 一行 + 计数），否则等于把缺陷从"白名单丢"换成"预算丢"。
8. **文档写明双层过滤**：内层 runtime **不再收紧**，应用层（Mahesvara `resp_header_pass`）自控 —— 否则两层取交，问题以另一种形式复现。

---

## 5. 验收门

- **功能（S1/S3）**：§3.1 表全部达标（17/17、route headers 同集合、`Content-Length` 不可被覆盖、CRLF 两例仍 PASS）
- **预算（S2）**：单头值 4000B 完整透传；超限场景 stderr 可见
- **回归**：双自举逐字节（C 轨 `B.c == golden/compiler.c`；BC 轨 `dump == golden/compiler.bc.dump`）· `vm_ab.sh v2` 38P/0GAP/0F · `diffcheck --all` rc=0 · m89_s3d/m93_s3/m96_s2/m103_s2d 全绿 · 生态索引（13 libs / 306 natives）无漂移
- **端到端**：真起 `px_serve` + vhost handler，用 `curl -i` 抓**实际响应头**（不看单测自报），逐条比对 17 头
- **外部**：Mahesvara `cache_test`（12/12）—— 由清歌/晨曦侧复核
- **静态链接/版本**：`file` + `ldd` + `px --version` 核对

---

## 6. 分发交接（沿用用户 2026-09-11 决策：**走自动更新线，不手工推节点**）

M109 与 M106/M107/M108 一并**随发布包下发**；本里程碑**不含**节点变更窗口。S4 只需确认发布包含新 runtime，并给出"更新后何时生效"的观测法（以"某 handler 设 `ETag` → 客户端可见"为验收）。

---

## 7. 不纳入本里程碑

| 项 | 去向 | 理由 |
|---|---|---|
| `pxi` 下 `vhost(host, handler)` 报错（§3 注） | **先记录，S4 定夺** | 与响应头通路无因果；先确认是否另有 issue |
| Issue 31/32（连接生命周期） | M108 | 症状易混（同一 fail-closed 文案），根因不同 |
| Issue 33/34/35（性能） | M107 | 口径不同（耗时 vs 语义） |
| Mahesvara 侧 `resp_header_pass` 收紧/放宽 | 业务侧（清歌） | 本里程碑只保证"runtime 不再单方面作废应用层意图"，应用层策略自控 |
