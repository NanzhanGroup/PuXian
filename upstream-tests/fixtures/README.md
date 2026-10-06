# upstream-tests/fixtures —— 网络用例的 mock 服务端（**本仓自建，非上游文件**）

## 为什么有这个目录

上游 `registry-px` 的**四个**用例要求**外部服务端**才能跑：

| 用例 | 期望的服务端 | 上游的做法 |
|---|---|---|
| `tests/ftp_test.px` · `tests/ftp_2_test.px` | FTP（2121 控制 / PASV 数据口） | 头注释写「真实 pyftpdlib 服务」——用 Python 临时手搭，**未入库** |
| `tests/pop3_test.px` · `tests/pop3_2_test.px` | POP3（2110） | 注释写「真实自定义 POP3 服务」——同上，**未入库** |
| `tests/oauth2_test.px` | OAuth2 令牌端点（19090，HTTP POST） | 注释写「本地 mock 端点」——同上，**未入库** |
| `tests/imap_test.px` · `tests/imap_2_test.px` | IMAP4rev1（1143；0.2.0 用 **2143**） | 注释写「本地 IMAP mock」——同上，**未入库**（M210 引入该库时发现） |

⇒ 引入这些库时，只有两条路：**SKIP（登记理由）**或**自建 fixture**。
本仓选后者，并且**不用 Python / pyftpdlib / netcat** —— 本仓的长期纪律是**不引入新依赖**，
所以 mock 服务端用**普贤自己写**（顺带把 tcp/net/http 客户端-服务端链路再压一遍）。

## 契约（与用例断言一一对应）

| fixture | 端口 | 关键契约（用例断言的就是这些） |
|---|---|---|
| `oauth2_mock.px` | 19090 | `POST /token`：`client_secret=WRONG` → 400 `{"error":"invalid_client"}`；`grant_type=client_credentials` → 200 `access_token=mock_access_token_12345` / `expires_in=3600` / `scope=read`；`grant_type=password` → `mock_access_token_12345_pw` |
| `pop3_mock.px` | 2110 | 问候 `+OK`；`PASS ok` 通过、其它 `-ERR`；`STAT` → `2 100`；`LIST` → `1 40` / `2 60`；`RETR 1` 含 `Subject: one` + `message one 普贤`（**含中文 + 点填充行**）；`RETR 2` 含 `message two`；`RETR 99` → `-ERR`；`QUIT` → `+OK` |
| `ftp_mock.px` | 2121（控制）· 2122（数据） | `220` 问候；`USER px` → 331；`PASS secret` → 230（其它 530）；`PWD` → 257；`TYPE I` → 200；`PASV` → 227（数据口 **2122** = `(127,0,0,1,8,74)`）；`LIST`/`RETR`/`STOR` → 150 + 数据 + 226；`CWD sub`/`CWD /`/`CWD ..` → 250；`QUIT` → 221 |
| `imap_mock.px` | **1143 与 2143** | 欢迎 `* OK`；`LOGIN` → `<tag> OK`；`LIST` → INBOX / Sent / Trash（**进程内可变**：CREATE/DELETE/RENAME 会改它）；`SELECT`/`EXAMINE INBOX` → `* 2 EXISTS` + `* 0 RECENT`；`FETCH 1 BODY[]` → literal `{N}`，体含 `Subject: hello` + `普贤`（UTF-8）；`SEARCH ALL` → `1 2`、`SEARCH UNSEEN` → `1`；`LOGOUT` → `<tag> OK`。⚠️ **tag 必须回显请求里的**（客户端 tag = `"A" + now_ms()%100000`，位数可变）；⚠️ literal 长度按**字节**算（`len()` 是 rune 数 —— PX-DEF-015） |

### M282 增补的命令（0.2.0 用例才用到）

上游 0.2.0 的 `ftp_2_test` / `imap_2_test` 比 0.1.0 多用了这些命令，
mock 随之扩展（**回复码由用例断言反推**，见 `registry/ftp/0.2.0/ftp.px` /
`registry/imap/0.2.0/imap.px` 的同名函数；**不是**为了通过而放宽）：

| fixture | 新增命令 | 契约 |
|---|---|---|
| `ftp_mock` | `SYST` | `215 UNIX Type: L8`（`ft_syst` 要 215，取第 2 段文本） |
| | `SIZE` / `MDTM` | `213 <n>` / `213 20260101000000`（`ft_size` 取末段转 int；`ft_mdtm` 取末段，长度 ≥14） |
| | `MKD` / `RMD` | `257 "<path>" created` / `250`（目录表 `FF_DIRS`，初值含 `/sub`） |
| | `DELE` | `250`；删后 `RETR` 必须 `550`（普贤无「删键」原语 ⇒ 重建表，同 `cm_delete` 先例） |
| | `RNFR` + `RNTO` | `350` → `250` |
| | `APPE` | `150` + 收数据 + 追加 + `226` |
| | `REST` | `350 restarting at N`；**后续 `RETR` 从该偏移开始**（用例断言 `REST 5` 后 RETR `"0123456789"` 得 `"56789"`），用后清零 |
| `imap_mock` | `EXAMINE` | 与 `SELECT` 同形（只读） |
| | `SEARCH` | `ALL` → `* SEARCH 1 2`；`UNSEEN` → `* SEARCH 1` |
| | `NOOP` / `STORE` / `COPY` / `CLOSE` | `<tag> OK …`（`STORE` 额外回一行 `* 2 FETCH (FLAGS (\Seen))`） |
| | `CREATE` / `DELETE` / `RENAME` | 改**进程内邮箱表** ⇒ `CREATE` 后 `LIST` 必须看得见 `TestBox`、`DELETE` 后必须看不见 |

> ⚠️ 已知**无法**通过的两条（我方不改 mock，见 `docs/UPSTREAM_020_DEFECTS.md`）：
> `ftp_2_test` 断言 `RETR` 内容含 `"hello ftp"`，而 `ftp_test`（0.1.0）断言同一文件
> **精确等于** `"hello from ftp 普贤\n"` ⇒ 两条用例对同一夹具**互斥**；
> `pop3_2_test` 断言 `size==114` 而同一断言的文本写 `2/100`、0.1.0 断言 `100`。
> 这两条在 `EXPECTED.tsv` 登记为 **XFAIL**（带反向判据）。

## 运行方式

**不需要手工起** —— `selfhost/run_upstream_tests.sh` 在 fixture 段自动：
① 把 `fixtures/*.px` 拷进临时工作区 → ② 用**被测工具链**（`$PX build`）编译 →
③ 后台启动 → ④ 等各自打印 `…: listening <port>`（最多 5s）→ ⑤ `EXIT` 时统一 kill。

- 只编**编译轨**：服务端是基础设施；且 ftp 用例会在**主连接仍开着**时再开一条控制连接
  （错误口令用例），需要并发 —— `spawn` **只有编译轨支持**（PX-DEF-006，设计如此）。
- 在**临时工作区**编译，不在仓库树里生成 `build/`（保持 `git status` 干净）。
- `--no-fixtures` 可整体关闭（网络用例会按各自 EXPECTED 判，通常那几条是 PASS ⇒ 会失败，
  这正是"fixture 缺了就响亮"的预期行为）。
- 若端口被占（例如手工起过），fixture 会启动失败并**打印警告**，随后用例以
  `net: 连接 … 失败 (111)` 判 FAIL —— **不静默**。

## 判据归属（别搞混）

- `MANIFEST.sha256` 只覆盖 `upstream-tests/*_test.px`（上游用例，逐字节照搬）。
  **本目录的文件是本仓自己的**，不参与该清单，也不参与「照搬」判据。
  - ⚠️ M282 实测事故：重建清单时若写成 `sha256sum *.px fixtures/*.px`，
    **我们一改自己的 mock 就会让「用例逐字节照搬」判据变红**（判据指不到真因）。
    `run_upstream_tests.sh` 因此加了**前置判据 ①b**：清单条目集 **必须恰好等于** `*_test.px` 集。
- 上游一旦自己入库了 fixture，本目录即可删除（届时用例会优先用上游的）。
