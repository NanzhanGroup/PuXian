# M123 · http_serve 对端断开感知 / 请求取消原语（qg-issue 78）

> 提出方：清歌（2026-09-16）· 承接：东月 · 状态：原型已在 `feat/m123-http-conn-alive` 跑通并实测达标，
> 待与 ws-approve 侧联调 + 合并入 main。

## 一、问题（清歌实测，本机复核一致）

ws-approve 处理大载荷 `/check`（单字节成本 ≈1ms/KB）时 CPU 高企；**客户端断开后服务端仍把
请求算完**（1 并发 1MB 断连后仍烧 0.89s；4 并发 ≈3.9~4.0s）→ 自愈守卫按 CPU 判据误判"卡死"
→ 重启窗口内 token-cache fail-closed。

## 二、根因（代码级复核，runtime 0.2.0-1.m122 / 上游 `93f114d`）

1. handler 执行期**连接不在 epoll 中**：`http_conn_worker`（runtime.c:12078 起）开头
   `px_evc_acquire(fd, FSERVE_KIND_HTTP)` 只把上下文置 `ACTIVE`；新连接由 accept 直接投池
   （从不登记 epoll），IDLE 派发的连接在派发时已 `EPOLL_CTL_DEL`。
   而 `px_ev_loop` 的断开分支（runtime.c:12962）只处理**处于 IDLE 且已登记**的 fd
   （其上方的 `kind != HTTP && kind != PXSERVE → continue` 即此语义）。
   ⇒ **handler 在算期间，对端的 FIN/RST 落不到任何代码路径**。
2. M95-S2 起 VM handler 以**帧协程**跑（`px_coro_spawn_ex`，runtime.c:12338），
   协程跑在 coro worker 线程（coro.c），**与派发它的 fserve worker 不是同一线程**，
   且可跨 worker 迁移 ⇒ 单纯用线程 TLS 携带 fd 不可靠（见三·2）。
3. 结论与清歌判断一致：不是"断连处理有 bug"，而是**"处理过程不可取消、无对端可观测状态"**。

## 三、交付（原型已实现）

### 3.1 接口定稿

```c
/* 应用层：px_http_conn_alive() -> 1 / 0        （runtime native，VM/C 双轨可用）
 *   1 = 对端仍在线（未收到 FIN/RST/异常，连接上下文有效）
 *   0 = 对端已断开 / 连接失效 / 当前不在 http_serve 系 handler 上下文
 * 语义：同连接内幂等、无副作用、不消费任何数据（MSG_PEEK）；O(1)，实测 0.74µs/次。
 * 何时翻 0（**文档明确项**）：
 *   · 对端 RST / POLLERR / POLLHUP / fd 已失效 → 立即 0（无需读）；
 *   · 对端 FIN（含半关 SHUT_WR、或本端仍有未读数据）→ **下一次调用即 0**
 *     （依靠 poll(POLLRDHUP)：FIN 不被"未读缓冲"遮蔽，不必等下一次 recv）；
 *   · 非 handler 上下文（主线程/普通 spawn/回调外）→ 0。
 * ⚠️ 边界：本函数判「对端不会再发数据」，**不区分**"半关但仍等响应"的客户端
 *   （FIN 后对端仍可收包）。故 P1「跳过写响应」的判据刻意不用它。
 */
```

### 3.2 实现点（3 处，共 ~80 行）

| 文件 | 改动 |
|---|---|
| `runtime/coro.c` | `PxCoro` 增 `srv_fd`；新增 TLS `g_cur_srv_fd` + 访问器 `px_coro_srv_fd_get/set`；spawn 时**继承**调用方上下文，协程 run 时装载/退出时清（跨 worker 迁移仍正确） |
| `runtime/runtime.c` | `px_http_conn_alive_fd()`（poll+peek 双检）+ `bi_http_conn_alive()` + 注册 `http_conn_alive`；`http_conn_worker` 三条 handler 路径（协程 / 登记失败同步轨 / 逃生舱同步轨）spawn 前后装载-复位 fd |
| `runtime/runtime.c` | **P1**：`http_send_resp` 入口写侧快速失败——`poll(POLLOUT)` 报 `POLLHUP|POLLERR|POLLNVAL` → 直接收尾（判据**不含 FIN**，避免半关客户端丢响应） |
| `tools/lint_core.px` | 内置名册由 `tools/gen_builtin_list.sh` 重新派生（327 名） |

未做（有意）：`px_http_conn_abandon()` —— P1 已覆盖"跳过写响应"，abandon 只省下"handler 返回"
这一拍，收益与新增状态机不成比例；若应用侧确有需求（例如循环里深调用栈想立刻放弃）再补。

### 3.3 实测（本机 dongyue，合成负载：handler 内 1200 片 × ~11ms 纯 CPU，每片查一次 alive）

| 场景 | 现状（不查 alive） | 用 `http_conn_alive()` | 归一化 |
|---|---|---|---|
| 1 并发 1MB，客户端发完即断 | 服务端总 CPU **13.40s** | **0.03s** | 0.22% |
| 4 并发 1MB，同上 | **56.26s** | **0.10s** | 0.18% |
| 正常请求（对端在线，完整处理） | 13.12s | 13.09s | 1200 次检查 ≈0.9ms，噪声内 |
| 客户端 RST 断开 | 13.38s | 0.02s | — |

- 感知延迟 = 检查周期：本原型片长 11ms（日志 `ALIVE0 at slice=1` 即首片即感知）；
  按 ws-approve「每 KB 查一次（≈1ms）」粒度，感知 ≈1ms，远优于验收线 200ms。
- 原语成本实测定标（本机，AF_UNIX 流式）：`poll(POLLIN|POLLRDHUP,0)` 0.366µs/次，
  加 `recv(MSG_PEEK|MSG_DONTWAIT)` 合计 **0.738µs/次** → 每 KB 查一次 = 载荷成本的 ~0.07%。
- 语义回归（全绿）：在线空闲=1 · 有在途未读数据=1（且 RDHUP 可穿透未读数据）·
  对端 FIN（带未读数据）=0 · 对端 FIN（无残留）=0 · fd 已失效=0 · 非 handler 上下文=0 ·
  keep-alive 连续两次请求=1,1。

### 3.4 回归

- `examples/m82_http_serve_unix/verify.sh`：8 项全绿（含进程内自检、断连容忍、TCP/UNIX 双跑）。
- `selfhost/builtin_list_check.sh`：4 项全绿（名册随 runtime 注册表派生）。
- VM 轨（默认）与 C 轨（`--no-quic`）编译均通过。

## 四、验收对照（清歌 §五）

| 验收项 | 要求 | 本机结果 |
|---|---|---|
| 1 | 断连后总 CPU ≤ 完整处理 30% | **0.22% / 0.18%** ✅ |
| 2 | 1 并发 1MB 断连后 < 0.3s | 0.03s（合成负载基线 13.4s）✅ |
| 3 | 4 并发 1MB 合计 < 1.5s | 0.10s（基线 56.26s）✅ |
| 4 | 33 组断连矩阵不回归 / 正常请求（含 SSE）不变 | m82 门全绿；SSE 路径未触碰（另见下） |

> 注：本机无 ws-approve（`/data/app/ws/core/` 下无该二进制），上表为 runtime 侧**等价复现器**
> （`/tmp/m123/bigdisc.py` 形态对齐清歌脚本）的实测；**最终验收须在清歌侧对 ws-approve 原样跑
> `requirements/bigdisc.py`（不改一行业务代码）**。

## 五、遗留 / 风险

1. **半关语义**：`http_conn_alive()` 对 `shutdown(SHUT_WR)` 但等响应的客户端返回 0。
   应用侧若用"活着才继续算"，这类客户端会拿不到响应（ws-approve 场景不出现：客户端断开=放弃）。
   文档必须写明（已在 §3.1）。若将来需要区分，可加 `http_conn_writable()`（`SO_ERROR` 判写侧）。
2. **SSE 流式**：`http_stream` 接管后的连接走 `sse_send`（自带 EPIPE 处理），本次未接 P0/P1；
   如需"半截写"治理另开一条。
3. **平台面**：epoll 分支与 `POLLRDHUP` 为 Linux 语义；非 Linux（macOS/BSD）下 poll 的
   `POLLRDHUP` 不可用 → 该平台原语退化为"仅 FIN/ERR"（`MSG_PEEK` 路径仍有效，行为不劣化）。
4. **P2 现状更正**：单请求体积上限**已存在**——`PX_HTTP_BODY_MAX`（默认 256MB，超限在读 body
   前直接 413，见 runtime.c:12189 起）。缺的只是"按 serve 粒度传参"（`http_serve(..., opts)`），
   评估后可做，但并非空白。
5. **P3 现状更正**：P3(b) 的"并发/队列信号量"已有雏形——`PX_SERVE_WORKERS` 有界池 +
   `fserve_push` 队满背压 + `PX_MAX_CONNS` 登记上限。P3(a)"单请求 CPU/时间预算中断 handler"
   需要从 VM 里 longjmp 出 handler（M94-S2 的抢占只让出、不中断），风险高于收益，暂不做。
