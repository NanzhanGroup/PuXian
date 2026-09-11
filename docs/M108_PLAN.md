# M108 计划：服务端连接生命周期族（px_serve 连接槽泄漏/池自愈）

> 立项：2026-09-11（用户拍板「M108 同时立项」）
> **修订：2026-09-11 · 东月自查更正** —— 原计划把 Issue 31/32 当作「待修」是**错的**：
>   **M97**（`f4b805f`，tag `v0.2.0-m97`，2026-09-10）已同时修掉 31（客户端 `h_exchange` 协议感知）
>   与 32（服务端 `px_send_all` 全量写 + IDLE 前置），并附专用对拍套件；
>   **M99**（`d0d1f60`，tag `v0.2.0-m99`）又把 IDLE 交还 + 事件循环补到 `px_serve`。
>   ⇒ M108 主体改为 **Issue 37**（px_serve 连接槽泄漏 → 443 假死）。详见
>   `/data/qg-issue/37-puxian-serve-conn-slot-leak/归并裁定-东月-20260911.md`
> 依据：`/data/qg-issue/37-.../ISSUE.md` + `EVIDENCE.md` + `probe.sh`（晨曦 QA 现场取证）
> 基线：`main @ cfea869`（M107 收口）· 分层 **L0 runtime**（网络/连接池层），业务 `.px` 无需改动
> 状态：**S0 开工**（本文为 S0 交付物之一）

---

## 0. 范围（修订后）

| 项 | 状态 | 处置 |
|---|---|---|
| **31** 客户端 HTTP/1.0 keep-alive 误判 → 奇偶失败 | ✅ **M97-S2 已修**（`m97_s2`：旧 runtime 复现 ok=15/30） | 归档 → `done/`；仅需清歌现网复测登记 |
| **32** 服务端响应悬挂（裸 send / IDLE 不等 POLLOUT） | ✅ **M97-S3 已修**（`m97_s3`：Go 单连接 100 请求含 2MB 全零悬挂） | 同上 |
| **37** `px_serve` 连接槽泄漏 → 443 假死（80 正常） | 🆕 **本里程碑主修** | S0–S4 |
| 37 §7-E / §9 可用性看门狗（带 SNI 探 443、CLOSE-WAIT 阈值、能反向拉起 Ma 的独立 unit） | **非 runtime**（业务/systemd 侧） | **单列**移交，不入 M108 |

---

## 1. 前提复核：37 §3.1 的结构性论断对当前 main **已不成立**

37 称「`px_serve`（443）未获得 M88-B-S2 同等改造：worker 独占整条连接、无 IDLE 交还、无事件循环」。源码取证：

| 位置 | 现状（current main） |
|---|---|
| `runtime/runtime.c:16051-16072` | 段2 尾 `px_pxserve_idle_after_resp(conn,fd)` → **连接交 IDLE、worker 释放**（**M99**） |
| `runtime/runtime.c:12036` / `12108` | 事件循环 detect 可读 → `px_pool_push(fd)` 投回续处理 |
| `runtime/runtime.c:15202-15214` | **M98-S2a** 连接堆化 + 挂起注册表（PxPend），非「每连接常驻线程阻塞」 |
| `runtime/runtime.c:11152-11154` | **M97-S3** 响应全量入内核后才交还 IDLE |

**行号交叉验证**：37 引用的 `14036/14159/14236/14269` 与现盘同物 `16527/16650/16729/16720` **相差约 2.5 千行** ⇒ 其取证对应**旧 revision**。

### ⇒ S0 第一件事（先取证再动手）：**定版本**

取晨曦 Mahesvara **0.11.2 实际编入的 runtime 版本串 / md5**（`pxc` rtcache 哈希）：
- **pre-M97/M99** ⇒ 37 病灶对部署件成立，主修 = 升级（自动更新线）+ §3 S1 加固；
- **含 M97/M99** ⇒ 存在**另一次泄漏**，按 §2 复现定位。

---

## 2. 证据自洽性缺口与三条新候选

37 §4.2 的候选 A/B 与**它自己的观测**矛盾（wchan 直方图无任何 socket 读等待、294 线程 futex 空闲）
⇒ 新增候选（S0 优先证实/证伪）：

- **C1 · fd 复用串扰**：`PxPend` `active`/`stage` 未清 → 重入 `px_conn_worker` 直接返回，
  fd 既无 worker 读也不在 epoll 集 → ClientHello 滞留 Recv-Q（吻合实测 1525~1794B）。
- **C2 · IDLE 注册/移交丢失**：`px_evc_idle_put_fd` / `px_pxserve_inflight_data`（TLS `roff<rlen` 判定）
  使连接落入「既不在 worker 也不在 epoll」的黑洞态。
- **C3 · 半开连接未被 tick 回收**：`px_evc_detach` / `px_pxpend_close` 某分支漏 close（= C1 的直接机制）。

**必抓证据**：① 故障态逐线程 `wchan`/`stat` 直方图；② `Listen` socket `Recv-Q` 二次确认；③ CLOSE-WAIT 增量 + 连接 `ino`。

---

## 3. 切片

| 片 | 内容 | 风险 | 触发回归 |
|---|---|---|---|
| **S0** | 定版本 + 搭**半开连接压力台**（`shutdown(SHUT_WR)` / 客户端 RST / TLS 握手中断）+ 收敛 §2 缺口 | 低 | 无（只读 + 新套件） |
| **S1** | **池自愈（止血，先上）**：37 §7-**B**（池 worker `PX_POOL_*_MAX_REQ` 滚动重建 + `g_pool_count` 持续满/worker 全忙超阈值的泄漏检测事件）+ **C**（`px_pool_push` 与 worker **condvar 拆分**，消除「两谓词共用一条 condvar」）+ **D**（优雅关闭超时：残留 fd `shutdown/close` + 未归 worker 诊断）⇒ 直接兑现 **V5 缺陷可见性** 与「91s → **<2s**」 | **低** | 轻 |
| **S2** | **泄漏点修复**（依 S0 定位：C1/C2/C3） | 中 | 中 |
| **S3** | **容量解耦**（V2：并发 > `max_conn` 不再硬顶；按 S0 结论定「IDLE 不计容量」或有界排队 + 超时拒绝） | 中 | 中 |
| **S4** | 收口：双自举 + 全量门 + tag `v0.2.0-m108` + 自动更新线交接 + 运维看门狗移交单 | 低 | — |

**排序理由**：S1 是**与根因无关的止血与自愈**（不依赖 S0 定位，风险最低），能立刻消掉
「无自愈、无告警、停机 91s」三个运维致命项；S2 才是根因修复，依赖 S0 的现场证据。

---

## 4. 验收标准

沿用 37 §8 的 V1–V5，**新增 V6**：

| # | 验证项 | 判据 |
|---|---|---|
| V1 | 泄漏修复 | 半开连接压力下 `ss -tan state close-wait` **不单调增长**，压力结束后回落至 0 |
| V2 | 容量不再硬顶 | 并发 443 连接 > 32 时全部正常完成 |
| V3 | 优雅关闭 | 同 V1 条件下 `systemctl stop` **< 2s** 返回，日志出现 `优雅关闭完成（在途 0）` |
| V4 | 无回归 | 现网 7 域名（wsai.chat / www / soft / mta-sts / px.dzty.club / ma.xiusoft.cn / kbs.xiusoft.cn）全 200 |
| V5 | 缺陷可见性 | 假死发生时进程**自身**输出可观测信号（不依赖外部看门狗） |
| **V6** | 压力稳定 | 半开/RST/握手中断压力 **30 分钟零假死**，线程数与 RSS 平稳 |

---

## 5. 语义红线

1. 不改 TLS 语义与证书校验行为（含多租户 vhost SNI 判定）。
2. 不降级 keep-alive 语义：HTTP/1.1 复用行为与 M97/M99 后**逐字节一致**。
3. 关闭路径必须仍走 `px_pxpend_close` / `px_evc_detach` 统一出口，**不得新增独立 close**。
4. 池自愈滚动重建**只在空闲 worker** 上执行，不得丢弃在途请求。
5. 优雅关闭必须仍打印 `[px-serve] 优雅关闭完成（在途 N）`（保留既有可观测契约）。
6. fd 复用防护（`active=0` 置位顺序、`ino` 校验）**不得放宽**。
7. `px_pool_push` 的 condvar 拆分**不得改变**队列语义（仍是 `PX_POOL_MAX` 有界环）。
8. S1 的泄漏检测**只记事件 + 可选受控重启**，默认不得自动重启（避免掩盖根因）。

---

## 6. 交接项（不由本仓执行）

1. 清歌/晨曦：现网复测 **31/32**（proxy_forward 连续请求零 502；token-cache 长连 10 分钟口径零失败）→ 归档登记。
2. 晨曦：**Mahesvara 0.11.2 编入的 runtime 版本串/md5**（S0 定版本前提）。
3. 晨曦：故障态逐线程 `wchan` 直方图（37 §10.2 清单），或用 S0 压力台复现取数。
4. **运维侧单列**：可用性看门狗（带 SNI 探 443 + 内容校验 + CLOSE-WAIT 阈值 + 能反向拉起 Ma 的独立 unit）+ `ma-sec status` PID 字段缺陷（`tools/ma-sec.px:174`）。

---

# 实施与实测（S0–S4 收口，2026-09-11）

## 7. S0 定版本 + 复现（结论：**根因不是"槽泄漏"，是"握手持全局锁做无超时阻塞读"**）

### 7.1 与 issue 37 候选 A/B 的差异（关键纠正）

issue 37 §2.2 把现场「294/300 线程 `wchan=futex`」读作 *"worker 不是卡在处理中，而是没活干"*，
据此排除了候选 A（accept 反压）与候选 B（泄漏连接霸占 worker）。**这个读法有误**：
`futex` 既包含「在 condvar 上等活」的 worker，**也包含「堵在 `pthread_mutex_lock(&g_srv_hs_mu)` 上」的 worker**。
真实图景是 **1 个线程卡在 socket 读（`wait_woken`）+ 其余 worker 全堵在全局握手锁上**，
两者外观都接近「空闲」。这也解释了为何"没有任何 socket 读等待"这一否定证据不成立——直方图把极少数
的 `wait_woken` 漏掉/误归类了。

### 7.2 本地复现（`examples/m108_s0/`，同一 runtime，一条命令）

```
基线（无 stall）           : curl 200 @64ms
制造 1 个半完成握手连接     : bash /dev/tcp 连上后只发 5 字节 TLS Record 头（声称 512B）后静默
故障态探测                 : curl 000 / 6.00s（超时）  ← 整端口假死
openssl s_client           : 停在 "Connecting to 127.0.0.1"（无 ServerHello）
ss -tan                    : 服务端 CLOSE-WAIT 且 Recv-Q=518 / 324（= 未读 ClientHello）
wchan 直方图               : 39 futex_do_wait / 1 wait_woken / 1 inet_csk_accept / 1 ep_poll
```
⇒ 与现场（`294 futex / 2 inet_csk_accept`、CLOSE-WAIT 单调增长、Recv-Q 最高 1794B、80 正常 443 全死）**同形**。

### 7.3 定版本（交接项，非本仓可完成）

37 §3.1 引用的行号（14036/14159/14236/14269）与现盘同物（16527+/16731+）相差约 2.5k 行 ⇒ 其取证对应**旧 revision**。
但 **S0 复现在当前 main 上依然成立**，故「部署件是否含 M97/M99」**不再是本修的前置条件**：
根因在 M101 引入的全局握手串行锁 + 无超时阻塞 I/O，与 M97/M99 是否存在无关。
（仍需晨曦提供 Mahesvara 0.11.2 编入的 runtime 版本串/md5 以便登记影响面。）

## 8. 修复与缺陷清单（S1/S2/S3）

见 `CHANGELOG.md`「M108」条目（含每条修复的动机、做法与实测）。要点：

| 类别 | 缺陷 | 严重度 |
|---|---|---|
| **根因** | `mbedtls_ssl_handshake()` 在持 `g_srv_hs_mu` 期间做**无超时阻塞 recv** → 1 个卡住连接拖死整个 TLS 端口（生产 2h41m 全站不可用） | 致命 |
| 自愈 | 无任何握手/连接超时；只能重启 | 致命 |
| 可见性 | 假死时进程零信号（无日志、无告警、无计数） | 高 |
| 关闭 | `pthread_join` 无上限 → systemd 90s SIGKILL（现场 91s、无完成行） | 高 |
| 泄漏 | `px_conn_worker` 槽获取失败**直接 return**（fd 无 worker、无 epoll、无 close → 永久 CLOSE-WAIT） | 中 |
| 容量 | `px_pool_push` 队列满时**无限 cond_wait**，且调用方是 accept 线程/事件循环线程 | 中 |
| 并发原语 | pusher 与 worker 共用一条 condvar（两个谓词） | 低 |

## 9. 前后对照（同一压力台，`examples/m108_s0/hs_stall_repro.sh`）

`SRV_BIN` 指向同一份源码分别构建的 `pre_m108`（修复前）与 `post_m108`（修复后）二进制：

| 判据 | 修复前 pre_m108 | 修复后 post_m108 |
|---|---|---|
| 基线 | 200 | 200 |
| **3 个半完成握手后探测** | **000（假死）** | **200** |
| 服务端 CLOSE-WAIT | 未归零（9s 后仍 1） | **1s 内归零** |
| 优雅关闭 | **8s 内未退出**（join 永久阻塞） | **513ms** + 完成行（另两轮 24ms / 507ms） |
| 可观测信号（V5） | 无 | `[px-serve:STALL] tls-handshake-timeout … 停滞=2002ms hs_tmo=1` |
| 并发 64（>`max_conn` 32） | 64/64 | 64/64 |
| **合计** | **PASS=3 FAIL=4** | **PASS=7 FAIL=0（M108_REPRO_OK）** |

滚动重建（S1d）：`PX_POOL_MAX_REQ=2` → **72/72 请求全成功 · 98 次重建 · 优雅关闭正常 · 完成行存在**。

## 10. 语义红线自查（计划 §5 八条）

① TLS/SNI/证书行为不变（`_locked` 配置阶段逐行保留，仅把状态机推进移出锁外分步执行）；
② keep-alive 复用语义不变（15s 空闲时限不变，仅改为可被 `g_px_stop` 打断的等待）；
③ 关闭路径仍统一走 `px_pxpend_close` / `px_evc_detach`，**未新增独立 close**（`shutdown(SHUT_RDWR)` 只唤醒读等待，不关 fd）；
④ 滚动重建只在刚处理完 job、未持有连接的 worker 上发生；
⑤ 保留 `[px-serve] 优雅关闭完成（在途 N）`；
⑥ fd 复用防护未放宽（新增 `out_fd_closed` 出参是**加强**：避免双重 close）；
⑦ 队列仍是 `PX_POOL_MAX` 有界环（只把"无限等待"改为"有界等待 + 超时拒绝"）；
⑧ 泄漏/停滞只计数告警，**默认不自动重启**。

## 11. 验收（issue 37 §8 V1–V6）状态

| # | 判据 | 状态 |
|---|---|---|
| V1 | 半开压力下服务端 CLOSE-WAIT 不单调增长、压力后归零 | ✅ 本地压力台实测（1s 内归零） |
| V2 | 并发 443 > 32 全部正常完成 | ✅ 本地 64/64 |
| V3 | 优雅关闭 < 2s + 完成行 | ✅ 24~513ms + 完成行 |
| V4 | 现网 7 域名全 200 | ⏳ **待晨曦/清歌现网复测**（需部署件） |
| V5 | 假死时进程自身可观测 | ✅ `[px-serve:STALL]` + `PX_SERVE_DIAG` 健康行 |
| V6 | 半开/RST/握手中断压力 30 分钟零假死 | ⏳ 本地压力台已覆盖半开/残缺握手；**30 分钟长跑与 RST 注入待补**（见 §12） |

## 12. 遗留与交接

1. **V6 长跑 / RST 注入**：本地台子当前覆盖「半开 + 残缺 ClientHello + 关闭期卡住握手」；
   RST 注入与 30 分钟长跑建议在晨曦侧用真实 CDN 形态跑（本机仅 8 核，长跑与编译任务互相干扰）。
2. **定版本**：晨曦提供 Mahesvara 0.11.2 编入的 runtime 版本串/md5（登记影响面，非修复前置）。
3. **V4 现网复测**：部署后 7 域名 + 回源；M106/M107/M108 一律**走自动更新线**，不手工推节点（用户决定）。
4. **运维侧单列**（仍不入 M108）：带 SNI 探 443 + 内容校验 + CLOSE-WAIT 阈值 + 能反向拉起 Ma 的独立看门狗 unit；
   `ma-sec status` PID 字段缺陷（`tools/ma-sec.px:174`）。**本次故障 2h41m 无人被告知**，该件优先级不低。
5. **二期候选**：`PX_POOL_MAX_REQ` 默认值是否放开（现默认 0=关闭，仅保留能力）；
   M101 全局握手串行锁是否可用「mbedtls 开启 `MBEDTLS_THREADING_C` 重新预编译」替换（可去串行锁，但需重建多架构预编译库）。

