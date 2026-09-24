# upstream-tests/fixtures —— 网络用例的 mock 服务端（**本仓自建，非上游文件**）

## 为什么有这个目录

上游 `registry-px` 的三个用例要求**外部服务端**才能跑：

| 用例 | 期望的服务端 | 上游的做法 |
|---|---|---|
| `tests/ftp_test.px` | FTP（2121 控制 / PASV 数据口） | 头注释写「真实 pyftpdlib 服务」——用 Python 临时手搭，**未入库** |
| `tests/pop3_test.px` | POP3（2110） | 注释写「真实自定义 POP3 服务」——同上，**未入库** |
| `tests/oauth2_test.px` | OAuth2 令牌端点（19090，HTTP POST） | 注释写「本地 mock 端点」——同上，**未入库** |

⇒ 引入这三库时，只有两条路：**SKIP（登记理由）**或**自建 fixture**。
本仓选后者，并且**不用 Python / pyftpdlib / netcat** —— 本仓的长期纪律是**不引入新依赖**，
所以 mock 服务端用**普贤自己写**（顺带把 tcp/net/http 客户端-服务端链路再压一遍）。

## 契约（与用例断言一一对应）

| fixture | 端口 | 关键契约（用例断言的就是这些） |
|---|---|---|
| `oauth2_mock.px` | 19090 | `POST /token`：`client_secret=WRONG` → 400 `{"error":"invalid_client"}`；`grant_type=client_credentials` → 200 `access_token=mock_access_token_12345` / `expires_in=3600` / `scope=read`；`grant_type=password` → `mock_access_token_12345_pw` |
| `pop3_mock.px` | 2110 | 问候 `+OK`；`PASS ok` 通过、其它 `-ERR`；`STAT` → `2 100`；`LIST` → `1 40` / `2 60`；`RETR 1` 含 `Subject: one` + `message one 普贤`（**含中文 + 点填充行**）；`RETR 2` 含 `message two`；`RETR 99` → `-ERR`；`QUIT` → `+OK` |
| `ftp_mock.px` | 2121（控制）· 2122（数据） | `220` 问候；`USER px` → 331；`PASS secret` → 230（其它 530）；`PWD` → 257；`TYPE I` → 200；`PASV` → 227（数据口 **2122** = `(127,0,0,1,8,74)`）；`LIST`/`RETR`/`STOR` → 150 + 数据 + 226；`CWD sub`/`CWD /` → 250；`QUIT` → 221 |

## 运行方式

**不需要手工起** —— `selfhost/run_upstream_tests.sh` 在 fixture 段自动：
① 把 `fixtures/*.px` 拷进临时工作区 → ② 用**被测工具链**（`$PX build`）编译 →
③ 后台启动 → ④ 等各自打印 `…: listening <port>`（最多 5s）→ ⑤ `EXIT` 时统一 kill。

- 只编**编译轨**：服务端是基础设施；且 ftp 用例会在**主连接仍开着**时再开一条控制连接
  （错误口令用例），需要并发 —— `spawn` **只有编译轨支持**（PX-DEF-006，设计如此）。
- 在**临时工作区**编译，不在仓库树里生成 `build/`（保持 `git status` 干净）。
- `--no-fixtures` 可整体关闭（网络用例会按各自 EXPECTED 判，通常三条是 PASS ⇒ 会失败，
  这正是"fixture 缺了就响亮"的预期行为）。
- 若端口被占（例如手工起过），fixture 会启动失败并**打印警告**，随后用例以
  `net: 连接 … 失败 (111)` 判 FAIL —— **不静默**。

## 判据归属（别搞混）

- `MANIFEST.sha256` 只覆盖 `upstream-tests/*_test.px`（上游用例，逐字节照搬）。
  **本目录的文件是本仓自己的**，不参与该清单，也不参与「照搬」判据。
- 上游一旦自己入库了 fixture，本目录即可删除（届时用例会优先用上游的）。
