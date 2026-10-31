# M118 —— 真实模块 `chat` PuXian 化暴露的四类缺陷（含一个流式客户端缺口）

> 台账：`qg-issue 74` · 开工 2026-09-14 · 探针来源：`NanzhanGroup/chat`（Go 8793 行 · 文殊聊天引擎库 + CLI）
> 主题：**继续拿真实模块当探针**。这一轮的四条都不是「写不出来才报错」，而是
> **写不出来 / 写错才报错 / 报错信息指向别处**，以及**一条真实通路根本无法表达**。

---

## 0. 探针与对拍

`chat` 是文殊的 LLM 交互核心：Agent 主循环 + 工具注册表（内置 10 个）+ 上下文压缩 +
三级翻篇检测 + 记忆分层（memory-service over Unix socket）+ 技能/自定义工具管理 + CLI 行编辑器。
它的 LLM 通路**全部经过词元缓存的 Unix socket**（`POST /v1/chat/completions`，`stream=true`）。

移植方式：**PuXian 版落仓库根目录**（`main.px` + 24 个模块，约 95 KB / 6000 行，全部 < 500 行），
原 Go 实现整体移入 `go/`。

对拍：`tests/parity_chat.sh`

- 起一个「假词元缓存」（`tests/mockllm.go`：Unix socket + **SSE 流式**补全）——
  这是 chat 真实依赖的那条通路，也是本轮缺陷 4 的现场
- 两侧用**同一套干净环境**跑同一串输入：`env -i PATH=… WS_PATH=<沙箱> <二进制> --quiet`
  （宿主 supervisor 环境里带着 `WS_SOCK_DIR`/`WS_TOKEN_CACHE_TOKEN`，**不清干净两侧都会去连生产 socket** ——
  第一版脚本踩过这个坑）
- 每侧独立起 mock（mock 的回合计数是有状态的，共用会让两侧打到不同回合）
- 判据：退出码 + 归一化 stdout **逐行相同** · mock 侧收到的**请求体归一化后逐字段相同**
  （归一化仅去 `created_at`/`msg_id` 两个时间/随机字段；工具 schema 比**集合**，因为 Go 侧
  `BuiltinToolDefs()` 是 map，遍历序**每次运行都不同**）

**结果：stdout 3 行逐行相同；两次请求体（2 条消息 / 4 条消息、各 10 个工具 schema）归一化后逐字段一致。**

---

## 1. 四类缺陷（本轮修复）

### D1 · 字符串插值 `${…}` 里不能出现字符串字面量

现场（移植几乎每个日志/拼接处都会写）：

```px
print("v=${d[\"k\"]}")            # 字典取值
print("c=${contains(s, \"x\")}") # 函数调用传字面量
```

两轨都报 `E1001 非法字符: '\'`，位置指向**反斜杠**，与病根（插值扫描不认转义）毫不相干。

- 因由：`scan_interp_expr` 把 `\` 交给运算符分支；而外层是双引号串时，插值里的引号**必须**转义书写。
- 修：插值扫描识别 `\"`/`\'` 转义 → 转交给新的 `scan_interp_nested_string`（定界符是 `\"`，
  内部 `\n`/`\t`/`\\` 按字面意图解释）。
- 结果：`${d["k"]}`、`${contains(s, "x")}`、`${join(",", [...])}` 两轨都可用。

### D2 · 匿名函数没有多行体的写法

```px
var f = fn (x):
    var t = x * 3
    return t
map(ys, fn (x):
    return x % 2 == 0)
```

- 语句位：`E2001 意外的 token: 换行`；**速查包 §1 自己的示例（`spawn fn ():` + 缩进体）就编译不过**。
- 调用实参位：括号内 lexer 不产 INDENT/DEDENT（spec §4.1），连缩进 token 都没有。
- 修：`parse_closure` 的 `:` 分支支持两种多行体 —— ① 换行 + **缩进块**（`parse_block`）；
  ② 括号内（无缩进 token）**按换行逐语句**解析到 `)`/`]`/`,`/`}`。
- 结果：`map/filter/sort_by` 的回调可以正常多语句；两轨一致。

### D3 · `spawn <闭包>` 把编译器内部实现抛给用户

```px
spawn fn (): print("hi")
```

VM 轨旧行为：`运行时错误 [bc_emit_stmt_inner 行1267]: bc_emit Spawn 仅支持直接函数调用: [Closure, [], null, [Call, …]]`
—— **无源位置、无建议、还泄漏内部函数名与 AST dump**（C 轨则把一句中文错误**当成 C 代码返回**）。

- 修：两轨统一改为带源位置的语义错误：
  `错误: 3:5: 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): … 然后 spawn work()）`
- 说明：`spawn f(args)` 一直是可用的（C/VM 两轨都支持）；本轮修的是**诊断**，不是加能力。

### D4 · SSE 客户端只有 GET + TCP：chat 的 LLM 通路无法表达

chat 的每一次 LLM 调用都是 **Unix socket 上的 `POST /v1/chat/completions`（SSE 流式）**。而旧版原语：

| 原语 | 能力 | 为什么不够 |
|---|---|---|
| `sse_connect(url)` | 仅 `http(s)://`、**仅 GET**、无 headers/body | 连 POST 都发不出，更别说 unix socket |
| `http_unix` / `http_request` | 全缓冲 | 拿不到逐块到达的 token（流式语义丢失） |
| `http_get_stream(url, cb)` | 仅 GET、仅 TCP | 同上 |

- 修：`sse_connect(url[, opts])`，`opts = {reconnect_ms, sock, method, body, headers, content_type}`：
  - `sock` 非空 → **AF_UNIX** 连接，`url` 只作请求路径；
  - `method`/`body`/`headers` → POST + JSON 体 + 鉴权头；
  - 头序列化复用既有 `px_hdr_append`（str/list[str] 多值、CRLF 防护、预算控制）；
  - 请求体/附加头随 slot 存活（**重连保持**），slot 复用/关闭/断连时统一释放；
  - 第 2 参传 int 时行为不变（**零回归**，负控已覆盖）。
- 结果：chat 的 LLM 通路在 PuXian 侧逐块可用（实测 mock 每 40 ms 一个 token，
  客户端在 1/41/82/122 ms 分别收到 → **真流式**，不是攒完一次给）。

### 附带：本轮踩到但**没改**语言的点（如实记录）

- **`fn` 是关键字**：`var fn = …` → `E2001 期望变量名，实际得到 fn`。写移植代码时极易踩
  （M115 已经记过 `self`/`pub`；本轮又踩 `fn`）。错误信息本身**清晰**，故未改；
  速查包 §1.1 事实清单补一条。
- **`int_to_hex(n, width)` 是「取低 width×4 位」**，不是「转成十六进制」：`int_to_hex(12345678901, 1) == "5"`。
  移植 Go 的 `%x` 必须 `int_to_hex(n, 16)` 再去前导零（本轮在 `util.px` 里封了 `to_hex`）。
- **Result 没有「取错误值」原语**：只有 `is_ok/is_err/unwrap`（`unwrap` 在 Err 上 panic），
  移植 Go 的 `(val, err)` 双返回只能 `str(Ok(v))="Ok(v)"` / `str(Err(e))="Err(e)"` 剥壳
  （本轮 `res_text`）。**这条是原生缺口，建议后续补 `unwrap_err`/`expect`**。
- **`http_stream` + `http_serve_unix`（服务端 SSE over unix）本轮未打通**：连上后在 curl 与
  PuXian 客户端两侧都拿不到响应（socket 文件在、连接被拒/无响应），**未定性**——
  chat 侧不需要它（真词元缓存是独立进程），故本轮不阻塞；已作为**待复核项**登记，
  门里的 SSE 用例改为「一次性 SSE 响应体」以自包含（客户端语义不变）。

---

## 2. 门

- `examples/m118_realworld_defects/verify.sh`（**本轮专项，8 项全绿**）：D1/D2 两轨各 1 项 ·
  D3 诊断 2 项 · D4 unix+POST+SSE 1 项 · 负控 2 项（坏 socket/旧 URL 形式仍优雅失败 ·
  M117 的未知 POSIX 类名仍报错，防回退）
- 既有全门：`selfhost/m117_gates.sh`（重烘门 / 逐字节门 / 引擎对拍 / 示例套件）
- 入库件：`./selfhost/rebake_bin.sh --rebake-all` **14 件全绿**（含新增源码链指纹）

## 3. 边界

- 本轮**未新增 native**（`sse_connect` 是既有 native 的选项扩展）→ native 计数仍 312，
  `docs/native_index.json` 无漂移。
- D1/D2/D3 改的是 `selfhost/{pxlexer,parser,bc_emit,cg_stmt}.px`；D4 改的是 `runtime/runtime.c`。
  两处都走既有重烘链条（`PXSRC` 源码链指纹门可判来源）。
- 已知未做：`unwrap_err` 类原语（记在 §1 附带）、`http_stream` over unix（待复核）、
  服务端 SSE 的增量路径未在本轮门里覆盖。
