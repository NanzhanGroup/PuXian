# ws-web —— PuXian 写的第一个生产应用（M-B9b）

> 自举闭环的最后一环：用 PuXian 写 PuXian 生态的第一个生产应用。
> 编译器已自举（M-B8），Rust 版已退役（M-B9a），本应用全程使用 `tools/pxc` 工具链，**无需 Rust**。

## 快速开始

```bash
# 1. 编译为静态二进制（生产 + 开发主路径，支持 sqlite/http/ws 全能力）
./build.sh && ./build/ws-web

# 2. 冒烟测试（编译模式：起临时服务 → 请求端点 → 断言）
./tools/pxc build ws-web/tests/smoke.px && ./ws-web/tests/build/smoke

# 3. 解释执行（仅 Mini 子集，sqlite/http 等生产内置不可用——见下）
./run.sh
```

> ⚠️ **解释器（pxi）仅支持 Mini 子集内置白名单**（MINI_SUBSET §2.5）：`sqlite_* / http_* / ws_*` 等生产内置**不在解释器**中，只在**编译模式**（pxc build → C → runtime.c）可用。
> **ws-web 开发/运行请走编译模式（build.sh）**；run.sh 仅用于纯逻辑快速验证。

## 端点

| 方法 | 路径 | 说明 |
|---|---|---|
| GET | `/healthz` | 健康检查 `{"ok": true, "uptime": ms}` |
| GET | `/api/time` | 服务器时间戳 `{"ts": ms}` |
| GET | `/api/items` | 列出全部记录（SQLite）`{"items": [...]}` |
| POST | `/api/items` | 新增记录，body=`{"name": "..."}` |

## 技术栈（PuXian 内置，零外部依赖）

- **HTTP 服务**：`http_serve(port, handler)`，handler(req) 返回 `{status, body, headers, file, gzip...}`
- **HTTP 客户端**：`http_request(url, method, body?, headers?)` → `{status, headers, body}`（keep-alive 连接池）
- **WebSocket**：`ws_serve / ws_connect / ws_send / ws_broadcast / ws_recv`（生产需要可加）
- **SSE**：`sse_serve`（生产需要可加）
- **路由**：`route()` 函数式路由（M28，路由表非空时 px_serve 走路由优先）
- **SQLite**：`sqlite_open / sqlite_exec / sqlite_query / sqlite_escape / sqlite_last_insert_rowid`
- **JSON**：`json_parse / json_stringify / json_path / json_path_set`
- **协程**：`spawn fn(...)` 起线程；`sleep(ms)` 休眠
- **静态文件**：`px_serve(port, docroot, timeout)`（见 examples/m29_webprod.px）

## 写 PuXian 必须规避的坑（Mini 子集，自举编译器锁定）

> 完整规范见 `docs/MINI_SUBSET.md`（§三 明确排除 + §八~§十二 已知限制）。以下是最容易踩的：

| # | 别写 | 原因 | 替代 |
|---|---|---|---|
| 1 | 匿名 fn 捕获外部变量 | 编译模式仅**无捕获闭包**，捕获报"未定义变量" | 顶层 def 函数 + 全局变量（顶层 var 函数内可读写） |
| 2 | 函数内 `def` 嵌套函数 | 编译模式**静默忽略**（返回空） | 顶层 def |
| 3 | `match` 语句形式（case 体多语句/return） | case 体当表达式，return 失效返回 null | **match 表达式** `let r = match x: case ...: ...` |
| 4 | `try / throw` 异常 | spec 明确不做 | **Result + `?`**：`return Err("...")` + `f()?` |
| 5 | 字符串乘法 `"ab" * 3` | 运行时错误"需要数值" | 循环拼接 |
| 6 | 命名参数 `f(b: 2)`、`*args` | parser 仅位置参数 | 位置参数 / 显式 list |
| 7 | data enum 带值变体 `Circle(r)` | payload 恒 null | `enum 判别 + struct 载荷` |
| 8 | 三元 `a if b else c`、`lambda` 关键字 | spec 明确禁止 | if 表达式 / fn |
| 9 | 多行 fn 匿名 `fn(x):` 换行 | 语法错误 | 单行 `fn(x): expr` 或块 `fn(x) { ... }` |
| 10 | `?` 在顶层（main 外）传播 Err | 报"顶层不能传播" | `?` 只在函数内用 |

## 语言能力速查（常用且可用）

- 字符串插值：`"hi ${name} ${a + b}"`（表达式内仅标识符/属性/索引/二元运算）
- 推导式：`[x for x in xs if c]`、`{k: v for ...}`；生成器 `(x for x in xs)` + `list(g)`
- 可选链 `?.`、空合并 `??`、管道 `|>`
- dict 新键赋值 `d["k"] = v`；切片 `a[1:3]`、负数索引 `a[-1]`
- Result：`Ok(v)` / `Err(e)` / `is_ok` / `is_err` / `unwrap` / `ok` / `err`
- 集合：`sorted / reversed / map / filter / reduce / unique / group_by`（stdlib/collections.px）
- 文件：`read_file / write_file / append_file / exists / mkdir / remove / list_dir`
- 内置注册表：见 MINI_SUBSET §2.5 白名单

## 目录结构

```
ws-web/
├── main.px          # 入口：HTTP 服务（骨架，可扩展）
├── build.sh         # 一键构建静态二进制
├── run.sh           # 本地解释运行
└── tests/
    └── smoke.px     # 冒烟测试（healthz / items / 404）
```

## 反馈与贡献（语言缺陷上报）

遇到语言 bug 或特性缺失：

1. 写**最小复现用例**（单个 .px 文件 + 期望输出/实际输出）
2. 提交 GitHub issue，label `ws-web`（ws-web 相关）或 `compiler`（语言/编译器缺陷）
3. 维护者复现确认后修复 → 跑完整回归（--all + 自举证明）→ 随下个提交发布

## 版本

- 工具链：`./tools/pxc --version` → `pxc 0.1.0 (普贤 PuXian · selfhosted M-B9a)`
