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

### 3.1 基线（S0 实测已填入，2026-09-11 · 编译模式 · 本机 `dongyue`）

S0 实测命令与结果（**与 Issue 完全一致 ⇒ 复现成立**）：

| 指标 | 现状（S0 实测） | 目标 | S1~S3 后实测 |
|---|---|---|---|
| Mahesvara 放行的 17 头到达数 | **7/17**（丢 ETag/Vary/HSTS/CSP/X-Content-Type-Options/X-Frame-Options/Referrer-Policy/WWW-Authenticate/Retry-After/Accept-Ranges） | **17/17** | **17/17 ✅** |
| ma-cache 可观测头 | `Age`/`X-Cache`/`X-Cache-Key` **全丢** | 全通 | **3/3 ✅** |
| `Content-Type` 独立通道 | 通 | 不变 | 通 ✅ |
| 单头值可透传上限 | **970B**（`sweep.px` 900→1030 扫描：970 OK / 980 DROPPED） | ≥4096B 完整透传 | **4000B 完整（len=4000）✅** |
| `route()` headers | 仅 `Content-Type`（**301 的 Location 收不到**） | 与 vhost 同集合 | **5/5 ✅** |
| CRLF 防护（`/evilval`、`/evilkey`） | PASS | **仍 PASS** | **仍 PASS ✅** |

> 复现原始输出：issue 目录内 `repro.px` / `sweep.px` / `repro_route.px`（用 `tools/px build` 重建即可重跑，
> **S0 期间使用的临时构建目录已清理**，避免留残留产物）；验收测试固化为仓库资产
> `examples/m109_s0/hdr_pass.px` + `run.sh`（**27 断言**，末行 `M109-HDR ALL OK`）。

### 3.2 S0 补充核对：白名单之外还有**第二、第三处**同类违约（计划外发现）

S0 读码时在计划外多找到两处**同族缺陷**，一并纳入本里程碑（否则修一半等于没修）：

| # | 位置（M109 前行号） | 违约 |
|---|---|---|
| ⑥ | `runtime/runtime.c` `px_out11_begin`：`char head[2048]` | **头缓冲硬顶 2048B**，且 `if (off + l < sizeof(head))` 不满足时**把整个 extra 块丢弃**（不是截断）⇒ 只把 `extra[1024]` 提到 4096 会把"静默丢"从 extra 搬到 head。**必须同步提高**（S2 已改为 8192 + 只在 `\r\n` 边界截断） |
| ⑦ | `runtime/runtime.c` **脚本 handler 路径**（`.px` 输出 `__PX_RESPONSE__:{json}`）：`gz_extra[512]` | 与 vhost/route 同族：headers 里**只取 Content-Type**，其余全丢。S3 已并入同一套判定 |

> 这三处（vhost / route / 脚本 handler）+ 两处预算（extra/fextra/head）= **响应头通路的完整集合**；
> 只改 §1 表里的 ①~⑤ 会让"脚本 handler 风格"的站点仍然丢头。


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

---

## 8. 实施（S1 / S2 / S3 已全部落地）

### 8.1 S1 判定哲学改造：白名单 → **拒绝名单**

`runtime/runtime.c` 删除 `px_vhost_header_allowed()`（12 项白名单），改为：

```c
static const char* const PX_HDR_DENY[] = {
    "Content-Length", "Transfer-Encoding", "Connection", "Keep-Alive",
    "Trailer", "Upgrade", "Date", "Server", "X-Request-Id", NULL
};
int px_hdr_blocked(const char* k);     // strcasecmp，大小写不敏感
```

**默认放行**，只有拒绝名单被拦。拒绝理由逐条（对应红线 1）：

| 头 | 为什么必须由 runtime 自管 |
|---|---|
| `Content-Length` | runtime 按 `body_len` 计算；handler 可写 → 长度错乱 / HTTP 走私面 |
| `Transfer-Encoding` | 分帧方式由 runtime 决定（当前不发 chunked） |
| `Connection` / `Keep-Alive` | 逐跳头，由 runtime 的 keep-alive 决策决定 |
| `Trailer` / `Upgrade` | 逐跳/协议升级头，非 handler 语义 |
| `Date` / `Server` | 运行时保留（避免 handler 伪造 Server 指纹、与运行时 Date 重复） |
| `X-Request-Id` | runtime 已注入；handler 再写会产生**重复头**，且访问日志关联会错 |

**CRLF 防护完整保留**（红线 2）：键/值任一含 `\r`/`\n` 一律丢弃，位于拒绝名单判定**之后**（叠加）。

### 8.2 S1/S3 统一实现：`px_hdr_append()`（新增，`runtime/runtime.c` 实现、`runtime.h` 声明）

```c
int px_hdr_append(LXValue hdrs, char* extra, int off, int extra_sz, int skip_ct, int* out_dropped);
```

**四条通路全部改用它**（保证「同一套判定」这条红线）：

| 通路 | 位置 | S1 前 | S1 后 |
|---|---|---|---|
| vhost（`px_vhost_normalize`，443） | `runtime.c` | 白名单 12 项 + `extra[1024]` | `px_hdr_append(..., skip_ct=1)` + `extra[4096]` |
| fserve / `{"file":…}` 流式 | `runtime.c` | 白名单 + `fextra[1536]` | `px_hdr_append` + `fextra[4096]` |
| **脚本 handler**（`__PX_RESPONSE__`） | `runtime.c` | 只取 Content-Type + `gz_extra[512]` | `px_hdr_append` + `gz_extra[4096]` |
| **`route()`** | `runtime_route.c` ×2 处 | 只取 Content-Type | `px_hdr_append` + `rsp_extra[4096]` |

### 8.3 S2 预算与「不静默」

| 项 | S1 前 | S1 后 | 理由 |
|---|---|---|---|
| vhost extra | `extra[1024]` | `extra[4096]` | 原 1024 使 >970B 单头**整条丢弃** |
| fserve extra | `fextra[1536]` | `fextra[4096]` | 同上 |
| 脚本 handler extra | `gz_extra[512]` | `gz_extra[4096]` | 同上 |
| route extra | `rsp_extra[512]` | `rsp_extra[4096]` | 同上 |
| **HTTP/1.1 头缓冲** | `head[2048]` | **`head[8192]`** | **关键**：2048 会使 4096 预算在此处再次静默丢弃 |
| 头缓冲溢出行为 | 整块静默丢弃 | **只在 `\r\n` 边界截断 + stderr** | 半行 = 畸形响应；宁少发几个完整头 |
| 预算超限 | **静默丢弃**（无任何信号） | `g_hdr_drop_budget` 计数 + stderr（首 10 条 + 每 1000 条限频） | 红线 7「不静默」 |

### 8.4 可观测性（并入 M108-S1c 的 `PX_SERVE_DIAG` 健康摘要）

```
[px-serve:diag] pool=… busy=… queue=… inflight=… hs(…) enter_null=… push_tmo=…
                hdr(pass=… deny=… crlf=… budget=…)
```

- `pass` 成功透传头数 · `deny` 被拒绝名单拦下 · `crlf` 被 CRLF 防护拦下 · `budget` 因预算丢弃
- `budget` 非 0 即说明**仍有头没发出去**（且已在 stderr 打了具体头名与长度）——这类"看得见的丢"取代原来的"看不见的丢"

### 8.5 回归测试同步（红线 3）

`examples/m57_s7_vhost_headers.px` **原断言 `assert not has(ks,"X-Custom")` 把本缺陷固化成了期望** ⇒
S1 已改写为**拒绝名单语义**，并新增用例：

- `/ok`：原白名单时代的头仍放行；`X-Custom` **改为必须放行**（M109 修复点）
- `/new`：ETag / Vary / HSTS / CSP / X-Content-Type-Options / Retry-After / Accept-Ranges **必须到达**
- `/deny`：9 个拒绝名单头**覆盖必须无效**（`Content-Length` 实为 runtime 自算值、`Connection` 非 `close`、`X-Request-Id` 非伪造值）；同 dict 的 `X-Keep` 照常透传
- `/evilval`、`/evilkey`：CRLF 两例**仍 PASS**
- `/ct`：`Content-Type` 仍走独立通道

新增 `examples/m109_s0/hdr_pass.px` + `run.sh`：**27 断言**的端到端验收（含 4000B 完整透传、预算溢出时同响应小头仍存活、route 301 带 Location）。

---

## 9. 实测（S0 复现 → S1~S3 修复，同机同编译模式）

| 用例 | S0 复现（M109 前） | S1~S3 后 |
|---|---|---|
| `/ma` 17 头 | **7/17** | **17/17** |
| `/cache` 三头 | 0/3 | **3/3** |
| `/budget` 1200B | 整条丢失 | **len=1200 完整** |
| `sweep` 900→1030 扫描 | 970 OK / 980 DROPPED（上限 970B） | 全 OK（上限 >1030B；实测 4000B 通过） |
| `repro_route` 301 | `[Content-Length, Connection, Content-Type, X-Request-Id]`（Location 丢） | **+ Location / Cache-Control / Set-Cookie / ETag / X-Custom** |
| `hdr_pass` 27 断言 | —（新增） | **`M109-HDR ALL OK`** |
| `m57_s7` | 旧断言要求 `X-Custom` 被丢（固化缺陷） | **`M57-S7 ALL OK (M109-S1 deny-list semantics)`** |

**语义等价性说明**：本改动**有意改变**的行为 = 7/17 → 17/17、>970B → 4000B、route 仅 CT → 全集合；
**有意保持不变**的行为 = CRLF 两例拦截、`Content-Type` 独立通道、runtime 自发头（静态文件 ETag/Last-Modified/Content-Range/Alt-Svc/Accept-Ranges）逐字节不变、拒绝名单内的头（`Content-Length` 等）仍不可被 handler 覆盖。

---

## 10. 闸门（提交前）

沿用 M107/M108 口径，见 `/tmp/m109/gate.sh`：G0 规范 rtcache（清缓存重建 + md5 ≡ 仓库）·
G1/G2 双自举 `--fresh` 逐字节 · G3 `vm_ab.sh v2` · G4 `diffcheck --all` ·
G5–G8 m89_s3d/m93_s3/m96_s2/m103_s2d · G9 生态索引无漂移 · G10 p3_regex 双模式 ·
**G11 m57_s7 回归** · **G12 m109_s0 验收** · G13 pxi/pxi_vm 重链 + 冒烟 · **G14 真 serve + `curl -i` 端到端**。

---

## 11. 遗留与边界（诚实交代）

1. **`Set-Cookie` 多值受 dict 结构限制**：`headers` 是 dict ⇒ **同一头名只能有一条**，
   "多值不合并"这条红线**由结构天然满足**（不存在合并路径）。若将来需要同名多值
   （多 cookie / 多 `Vary` 分量），需把 `headers` 扩展为 list-of-pairs 或值支持数组 —— **本里程碑不做**，先记录。
2. **`Date`/`Server` 被拒是"策略"而非"冲突"**：当前 runtime 并不主动发这两个头，
   拒绝是为了避免 handler 伪造指纹与将来冲突。若业务确需自定义 `Server`，应改在**反向代理层**注入。
3. **`pxi` 下 `vhost(host, handler)` 仍报错**（既有现象，与本次改动无关）：
   `examples/m57_s7_vhost_headers.px`、`examples/m109_s0/hdr_pass.px` **均须编译模式**运行。
   S4 定夺：修 pxi 支持 handler / 或文档明示仅编译模式。
4. **外部复核未做**：Mahesvara `cache_test` 12/12 与 17 头现网到达表需**晨曦/清歌侧**在部署后跑
   （本机无 Mahesvara）——**不能算已完成**。
5. **分发**：随自动更新线下发（用户决策），本里程碑不含节点变更窗口；
   生效观测法 = "某 handler 设 `ETag` → 客户端可见"。


## 14. 二期（2026-09-11 实施完毕）：同名多值 —— 值可 `str` / `list[str]`

**立项依据**（三条硬证据，均在 S0 读码中确认）：

1. **客户端**：`runtime.c` 响应头解析逐行 `px_dict_set(*out_headers, k, px_str(v))` ⇒ 同名头**后写覆盖前写**
   （多 `Set-Cookie` 只剩最后一个）。
2. **服务端**：`px_hdr_append` 遍历 dict 时 `if (hv.type != PX_STR) continue;` ⇒ 即使 handler 传 `list`，
   **被静默跳过** —— M109-S1 刚立下「不再静默」的原则，在 list 值上又破了一次。
3. **契约厂内已声明**：`stdlib/cookiejar.px` 头注写明「本库接受 Set-Cookie 值为 str 或 list[str]」，
   但**生产者端从不产出 list** ⇒ 该分支一直是**不可达代码**。

### 14.1 改动

| 片 | 位置 | 做法 |
|---|---|---|
| **S1 服务端生产者** | `runtime/runtime.c` `px_hdr_append` | 值支持 `str` 或 `list[str]`：list **逐元素展开**，每元素各做一次 CRLF 防护与预算计数；**键级判定（拒绝名单 / skip_ct）在元素循环之外** ⇒ list 不能绕过拒绝名单；非 `str`/非 list 的值、以及 list 内的非 str 元素：**计数 + 限频告警**（新增计数 `g_hdr_drop_val`，并入 `PX_SERVE_DIAG` 的 `hdr(...badval=N)`） |
| **S2 客户端生产者** | `runtime.c` 响应头解析 | 同名头（**大小写不敏感**，RFC 7230）：首次仍写入 `str`（**单值路径逐字节不变**）；第二次起升级为 `list[str]`，键名沿用**首次出现的拼写**。实现上先以 `strcasecmp` 定位既有键，再以该键的**副本**走 `px_dict_set` 原地替换（避免把 dict 内部指针传入 setter 引发的 realloc 风险） |
| **S3 语义红线** | — | `Content-Length` / `Transfer-Encoding` / `Connection` 等仍由 runtime 自管（键级拒绝，list 不能绕过）；输出总量仍受 `extra` 预算约束（元素级计数） |
| **S4 契约同步** | `stdlib/cookiejar.px` 头注 | 原「⚠️ 语言层限制：同名头会覆盖」→ 改为「✅ 同名多值：从第二次起聚合为 `list[str]`；list 分支即正常路径」 |

### 14.2 验收（`examples/m109_headers_multi/`，26 断言全绿）

`bash examples/m109_headers_multi/run.sh` → `M109-HDRMULTI ALL OK`：

| 段 | 覆盖 | 关键断言 |
|---|---|---|
| A | vhost `list[str]` 值 | `X-Multi` 三值全部到达且为 `list` len=3 |
| B | 两个 `Set-Cookie` | 客户端 `list` len=2，两条值都在 |
| C | **向后兼容** | 单值头类型仍是 `string`（`m23c_http_adv` 断言口径不变） |
| D | 大小写不同同名头（`X-Dup` / `x-dup`） | 聚合成 `list` len=2，**不产生第二个冗余键** |
| E | 拒绝名单不可用 list 绕过 | `Content-Length=["1","2"]` 无效（仍为 runtime 计算值），同 dict 其他头照常 |
| F | list 元素级 CRLF | 坏元素丢弃、好元素存活；**只剩一条时仍是 `str`**（不无谓升级） |
| G | 值类型不支持（`int`） | **不再静默**：stderr 告警 + 计数，同 dict 其他头照常 |
| H | list 内非 str 元素 | 跳过并告警，同 list 的 str 元素照常 |
| I | `route()` 通路 | 同样支持 list（单值仍 str） |
| J | **`std.cookiejar` 契约** | 两条 `Set-Cookie` → jar 收录 **2 条**（此前不可达分支现为正常路径） |

`run.sh` 另校验 stderr 上确实出现两类告警（证明「不静默」是**可观测**的，不只是注释）。

### 14.3 全量闸门（`/tmp/m109s2/gate.sh`，`ALL_DONE`）

F1~F4 功能回归全过（本二期 / M109-S1 `hdr_pass` 26 断言 / `m57_s7` / `m23c`）；
G1 C 轨自举 rc=0（与自身逐字节一致）；G2 BC 轨自举 rc=0（dump 与 `golden/compiler.bc.dump` **30581 行**一致）；
G3 `vm_ab.sh v2` **38 PASS / 0 GAP / 0 FAIL**；G4 `diffcheck --all` rc=0；G5 `m89_s3d` 9P0F；
G6 `m93_s3` 6P0F；G7 `m96_s2` 8P0F；G8 `m103_s2d` rc=0；G9 生态索引**无漂移**（13 libs / **306** natives）；
G10 `p3_regex` 双模式 ALL PASSED。

`runtime.c` md5（本次改动后）= `296c53e44065d312213ad55608939acc`。

### 14.4 边界（诚实交代）

- `headers` 仍是 dict ⇒ **同一个键**才聚合；若要表达「同名多值的顺序与跨键顺序」，需要 list-of-pairs 形态（本里程碑不做）。
- `Date` / `Server` 被拒仍属**策略**（runtime 自管），不是遗漏。
- 外部复核（晨曦/清歌侧）不变：Mahesvara `cache_test` 12/12 与现网 17 头到达表；本次新增的 list 通路建议在观音双 serve 对拍时一并覆盖。
