# MySQL 驱动（`registry/mysql`）口径与踩坑

> 适用版本：`registry/mysql/0.1.0` 与 `0.2.0`（两份**同一份代码**，行号一致 —— 修一处必须修两处）。
> 本文是 M313（第 184 轮）的产物：半山 2026-10-19 生产报障后的定稿口径。

## 一 为什么写这份文档

`mysql_connect()` 连 MySQL 5.7 时**永久阻塞**（不返回、不报错、不超时），服务器错误日志里同时出现
`Bad handshake`。根因是**三处独立的协议层错误叠加**，而它们**只在真实握手时才会显形** ——
驱动此前**没有任何动态门**（上游用例要真实 MariaDB，CI 上恒 SKIP；「三轨对拍」也看不见，
因为同一份 `.px` 在三轨跑的是**同一份语义**）。

⇒ 本文记下**正确的协议口径**，并指向新门 `examples/m313_mysql_driver/`（用普贤自己写的 mock 服务端）。

## 二 三处缺陷（缺陷 536 / 537 / 538）

### 536 · 握手偏移多跳一字节

```px
var ver = my_cstr_at(hs, soff)     # 返回 [字符串, 终止符**之后**的偏移]
soff = ver[1] + 1                  # ❌ 再 +1 ⇒ connection id / nonce / capability 全段错位
soff = ver[1]                      # ✅
```

症状：`nonce` 只读到 8 字节（应 20）· capability 读成 `0x15C12DFF`（应 `0xC203FFFF`）
⇒ 认证 token 算错 ⇒ 服务器判 `Bad handshake`。

**教训（可推广）**：「返回偏移」的函数必须**明确是「指向终止符」还是「终止符之后」** ——
两种约定都常见（`strchr` 返回指针 vs `strtok` 之后的位置），混一次就整段错位。

### 537 · CONNECT_WITH_DB 用错常量

| 名字 | 值 |
|---|---|
| `CLIENT_CONNECT_WITH_DB` | **0x00000008** |
| `CLIENT_IGNORE_SPACE` | `0x00000800` ← 修前误用 |

修前：标志位没置，payload 末尾却塞了库名 ⇒ 服务器按「多余字节」处理 ⇒ 同样 `Bad handshake`。
两处必须一起改：置位处（`cap41 = … | 0x00000008`）与判据处（`if cap41 & 0x00000008`）。

**并且**：只在**真的给了库名**时才置位（`want_db`）—— 空库名不该发 `\0`。

### 538 · 读无界（永久阻塞）

`my_recv_n()` → `tcp_recv_ex()` 修前**没有超时**：协议不匹配 / 网络抖动 / 服务端半开连接
⇒ 调用线程**永久挂住**（放进 Web 服务就是一个 worker 静默死掉）。

修法（与 `registry/netprobe` 同一口径）：

```px
tcp_opt(fd, {"read_timeout_ms": rtmo, "write_timeout_ms": wtmo})
```

| 参数 | 默认 | 说明 |
|---|---|---|
| `timeout_ms` | **5000** | 连接超时，**同时作为读写超时上限**（本轮起） |
| `read_timeout_ms` | = `timeout_ms` | 单独覆盖读超时；**0 = 无限**（恢复旧行为） |
| `write_timeout_ms` | = `timeout_ms` | 同上（写方向） |

⚠️ **语义变化（据实登记）**：修前读**永远不超时**；修后默认 5s 超时。
长查询（大聚合 / 大结果集）请显式传 `{"read_timeout_ms": 60000}`。
超时是**响亮**的（`Err("… i/o timeout")`），而挂死是**静默**的 —— 这是本仓「响亮优于静默」的一贯取舍。

### 附 · IO 出错即关连接（缺陷 540 · 不留半连接态）

超时 / EOF 之后协议**已错位**（读到半个包）。修前驱动只返回 `Err` 而把 fd 留在 `conn` 里
⇒ 调用方再拿这根 fd 发命令，只会**读到错位数据**（比报错更难查）。

现行口径（与 Go `database/sql` 一致）：

| 场景 | 行为 |
|---|---|
| 任何 IO 错误（send/recv/超时/EOF） | **关闭 fd** + `conn["fd"] = -1`，Err 末尾带 `(connection closed)` |
| 在已关闭的 conn 上再操作 | 立即 `Err("mysql: connection already closed")`（不碰 fd） |
| `mysql_close(已关闭)` | `Ok(true)`（**防 double close 命中别人复用的 fd 号**） |


### 附 · 形参与文档一致（同轮修）

文档写 `mysql_connect(host, port, user, password[, dbname][, opts])`，而实现是 6 个必填参数
（`R1005 调用 f 缺少参数 'e'`）⇒ 本轮补默认值：`dbname = ""`、`opts = null`。

## 三 认证能力边界（**不是缺陷，是未实现**）

- **`mysql_native_password`**：完整支持（`SHA1(pw) XOR SHA1(nonce + SHA1(SHA1(pw)))`）。
- **`caching_sha2_password`**：只支持 **fast auth**。**full auth 需要 TLS/RSA 交换口令**
  ⇒ 源码里直接返回 `Err`。⇒ **MySQL 8 + 非 TLS + caching_sha2 账号会踩**，请改用 TLS 或 native 账号。

## 四 门（`examples/m313_mysql_driver/`）

用**普贤自己写的 mock MySQL 服务端**（`mock_mysql.px`，不引入任何外部依赖）：

| 层 | 判据 |
|---|---|
| [1] | 静态：两版本三处修复齐备 + 读超时已设 + 形参默认值 + 规模锚点 + 两版本形态一致 |
| [3] | 0.2.0 端到端：握手 + **20 字节 auth token 独立复算对拍** + `USE` + `SELECT` ⇒ `affected=7` |
| [4] | 0.1.0 同形态（同一份代码 ⇒ 必须一起修） |
| [5] | 读超时有界：服务端只 accept 不发字节 ⇒ 有界返回 `i/o timeout`（实测 501ms，上限判 5000ms） |
| [6] | 负控 A：撤回 `soff = ver[1]` ⇒ 必须 `Err`（原因指到 token 错位） |
| [7] | 负控 B：撤回能力位 ⇒ 必须 `Err`（原因指到 `IGNORE_SPACE`） |
| [8] | 负控 C：去掉 `tcp_opt` ⇒ 超时档必须**挂住**（`timeout 5` 判 rc=124） |

⚠️ 服务端是**严格校验**的：能力位、用户名、库名、插件名、token、**尾部有无多余字节**逐项检查，
任一不符就回 `ERR` 并**写明是哪一项** —— 失败信息必须指到真因。

## 五 覆盖边界（如实）

- 结果集（列定义 + 行）只由上游 `mysql_test.px` 覆盖（需真实 MariaDB，CI 上 SKIP）⇒ 不在本门面内。
- 参数化语句（`COM_STMT_*`）同前。
- mock 复刻的是**协议形态**，不依赖具体服务端版本（现场是 5.7.44）。

## 六 相关建议（未做，登记）

- **sqlite3 FTS5**：半山实测默认 `sqlite3.o` 未编 FTS5（`no such module: fts5`），
  自编加 `-DSQLITE_ENABLE_FTS5` 后正常。本仓 `sqlite3.o` 是**预置三架构构建输入**
  （x86-64 / aarch64 / Windows COFF），改宏 = 三个二进制重编 + 体积与架构卫生复核
  ⇒ 不属本轮（驱动缺陷）范围，登记为后续候选。
