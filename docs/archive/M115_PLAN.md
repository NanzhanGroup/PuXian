# M115_PLAN · 服务进程/环境原语补全（ws-center / ws-ddns PuXian 化实测缺口）

> 来源：`github.com/NanzhanGroup/ws-center`（2717 行 Go · 中心管理服务）与
> `github.com/NanzhanGroup/ws-ddns`（1141 行 Go · DNS 授权签名系统）的 **PuXian 化开发**。
> 目标不是"移植成功"，而是**在真实开发中把 PuXian 的缺口顶出来并当场补掉**（不许绕过）。
> 本文件只记"被真实开发卡住"的原语，不记"顺手好用"的愿望。

## 一、缺口（四条，均**无现成代偿**）

| # | Go 侧用法 | PuXian 现状 | 后果 |
|---|---|---|---|
| ① | `os.Setenv(k, v)`（`loadEnvFile`：配置文件 → 进程环境） | 只有读 `env(name)`，**无写** | 配置无法进进程环境，子进程（守护化自举/外部命令）**继承不到**；只能改成全局 dict，**语义漂移**（不是 Go 语义） |
| ② | daemonize 三件套：`os.Executable()` + `SysProcAttr{Setsid:true}` + `cmd.Stdout/Stderr → 日志文件` | `os_spawn` 只有 `(cmd, args[, group])`；无 setsid、无 stdio 重定向、无子进程 env；**无"我是谁"** | 只能 `/bin/sh -c "setsid … >>log 2>&1 &"`：引入 **shell 依赖**（边缘/容器无 sh 即废）、守护进程**真实 pid 拿不到**（pidfile 语义失效）、`--stop` 无法精确停机 |
| ③ | `os.Stdin.Stat().Mode()&os.ModeCharDevice`（交互式选择授权范围，**仅 TTY 弹**） | 无 TTY 判定原语（`tty_config` 不能代偿——它会**设置**波特率，非查询） | 交互选择无法表达，只能永远走非交互分支 |
| ④ | `time.Now().Unix()`（签名协议 `wsddns:v1:<unix秒>:<sig>` 的核心字段） | `now()` 返回**本地时间字符串**，`now_sec` 不存在；`time_format(now(), …)` 直接**类型报错** | 只能 `now_ms()//1000` 代偿（可行但反直觉；语言缺"当前 Unix 秒"这一最基本的服务端原语） |

## 二、本步交付（5 个 native + 1 个扩展）

| 原语 | 语义 |
|---|---|
| `env_set(name, value) → bool` | `setenv(…, 1)`；name 空/含 `=` → px_error；失败 false |
| `env_unset(name) → bool` | `unsetenv`；同上校验 |
| `os_self_path() → str\|null` | `/proc/self/exe` 绝对路径（重新 exec 自身/守护化/自升级必需） |
| `isatty(fd) → bool` | `isatty(3)`；管道/重定向/后台 → false |
| `now_sec() → int` | Unix 秒（UTC 基准，与 `time_format`/`time_parse`/`tz_offset` 同一时间轴） |
| **`os_spawn(cmd, args[, opts])` 扩展** | 第 3 参由 `bool group` 扩为 **bool \| opts dict**：`{group, setsid, stdout, stderr, stdin, cwd, env}`。旧 bool 语义**零回归** |

设计要点（实现约束，不是口味）：
- **`env` 在 fork 之前**备成 `char** envp`（父侧 `calloc`/`strdup`），子进程只做 AS-safe 调用
  （`setsid`/`setpgid`/`chdir`/`open`/`dup2`/`execvpe`）—— **fork 后**不调 `setenv`（会摸 malloc 锁，
  多线程下可能死锁）；`execvpe` 保留 `PATH` 查找语义。
- `stdin: false` → `/dev/null`（守护化必需：不占终端、`read` 立即 EOF）；
- **父侧同调** `setpgid`（qg-issue 69 已有约定，本步沿用）；
- 解释器轨同步：`selfhost/interp.px` 名册 + `selfhost/ibuiltin.px` 分发（**三处同步**是 M66 起的既有铁律）。

## 三、门的口径

| 门 | 判据 |
|---|---|
| `examples/m115_proc_env.px`（双模式） | 9 组断言：env 读写 / **子进程继承** / opts.env 覆盖且**不污染父进程** / cwd+stdout / stdin=false / **setsid 后 `ps -o sid= -p PID` == PID** / `os_self_path` 存在 / isatty 类型与非法 fd / now_sec 与 now_ms//1000 一致 |
| `examples/m115_proc_env.sh`（verify） | 两种模式都跑 + 负控（**故意断言错的值 → 必须红**，防门自欺） |
| 名册门 | `bash selfhost/builtin_list_check.sh`（新名必须同时进 runtime / interp / lint 名册） |
| 表漂移门 | `bash tools/gen_native_table.sh` → 306 → **311** |
| 回归 | `diffcheck.sh --all/--errors` · `capability` 双模式 · `bootstrap_prove{,_bc}` · `engine_parity` · `rebake_bin.sh --check-all` |

## 四、明确**不做**（留作后续，避免范围蔓延）

- **`px` 解析器续行缩进缺陷**（本步实测发现，见 `docs/ECOSYSTEM_GAPS.md`）：`return {\n …}` /
  `f(1,\n 2)` 这类**闭合括号比语句更深缩进**的写法（人写代码的常态）会报
  `E2001 意外的 token: 去缩进`，且**行列指向无辜的下一行**。与 M70-S1「括号内换行/缩进 token 被
  parser 忽略」的规范描述不一致 ⇒ 属**真缺陷**，但修在 lexer/parser（token 流/golden 面广），
  与本步（runtime native）不耦合，**单独立项**。
- 速查包 `send(ch, x)`/`recv(ch)` 写法（实际语言为 `ch.send(x)`/`ch.recv()`）：**本步顺手改正**
  （纯文档，零代码风险）。
