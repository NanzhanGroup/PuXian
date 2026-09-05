# PuXian 速查包（PUXIAN_CHEATSHEET）

> **给 AI 的一句话**：把本文件 + [`docs/ECOSYSTEM.md`](ECOSYSTEM.md) 整包喂进上下文，即可写出**语法正确、native/库调用正确**的 PuXian（普贤）`.px` 程序。生成后务必用 `tools/pxc run <f>.px` 验证；编译模式 `tools/pxc build <f>.px`。
> 版本基线：M69（2026-09-05）· 双模式（编译 pxc build / 解释 pxi run）行为一致（M68 起 native 零 extern def 可达）。
> 工具链（M71，2026-09-06）：`pxc build` 已增量缓存（**二次 build ≈0.4–0.9s**）+ `--target <arch>` 交叉；`pxc mcp` 含 **build** 工具（AI 一条 MCP 写→验→交付）；安装 `tools/install.sh`（sha256 自动校验 + argv0 自发现，装完任意目录免 PX_STDLIB）。重文本/大文件处理：pxc build 编译版毫秒级 ≈ grep（ECOSYSTEM_GAPS F4 M71 更正）。

---

## 0. 三件套先记住

1. **`.px` 文件 = 程序/模块**；注释 `#`；`##` 开头为文档注释（pxc doc 生成 API 文档）。
2. **运行**：`tools/pxc run hello.px`（解释，秒起）· `tools/pxc build hello.px`（生成 C→gcc 静态二进制，`<目录>/build/hello`）。
3. **import**：`import std.collections` / `from std.collections import unique` / `import "rel/path.px"`；import 只注册定义**不执行**模块其它顶层语句——但模块顶层 **let/var/const 声明随合并导出**（M70-S3：初始化表达式在 import 方程序启动时执行一次 = 模块级状态槽）。

## 1. 语言速查

### 类型与字面量

| 类别 | 写法 |
|---|---|
| int / float / bool / null | `42` `3.14` `-5` `true` `false` `null` |
| str | `"双引号"` 支持 `${expr}` 插值、转义 `\n \t \"`；多行 `"""..."""` |
| list | `[1, 2, 3]`（**M70：括号内可换行** `[\n1, 2,\n3]`；追加 `.append(x)`） |
| dict | `{"k": "v"}`（**键限 str**） |
| bytes | `bytes("abc")` 二进制视图，配 `bytes_*` 族 |

### dict / list / str 操作（易错重点）

```px
var d = json_parse("{}")        # ⚠️ 空 dict 不能写 {}（{} 字面量 = null！）
d.set("a", 1)                   # 写键：.set(k, v)（无 d[k]=v 语法）
if d.has("a"):                  # 查键 .has(k)
    var v = d["a"]              # 读键 d[k]
var xs = []
xs.append("x")                  # list 追加 .append
print(len(xs), xs[0])           # 多参 print 空格分隔
var parts = split("a,b", ",")   # str 工具：split/join/trim/to_upper/to_lower/
                                #   contains/replace/starts_with/ends_with/切片 a[1:3]
```

### 控制流 / 函数 / 错误 / 并发

```px
# if/elif/else · for/while/break/continue · match/case
for i in range(5):
    if i % 2 == 0:
        print("even", i)

def add(a, b):                  # 函数 def（可默认参数 def f(a, b=1)）
    return a + b
var doubler = fn (x): x * 2     # 匿名函数 fn
var r = map([1, 2, 3], fn (x): x + 1)     # 高阶：map/filter/reduce/sorted

# 错误通道（唯一）：Result/Option
def div_safe(a, b):
    if b == 0:
        return Err("div by zero")
    return Ok(a / b)
var x = div_safe(4, 2)          # Ok(v)；is_ok()/is_err()/unwrap()/v? 解包
# 顶层 `?` 传播 Err/None；`!` 强制解包（Err 则 panic）

# 并发：spawn 真并发 + channel 阻塞通信 + select 随机就绪
var ch = chan(2)
spawn fn ():
    send(ch, "hi")
print(recv(ch))

# 列表推导/切片/字符串插值/管道
var sq = [x * x for x in range(5)]
var s = "val=${x}"
print("upper=" + to_upper("px"))
```

### ⚠️ 语言事实与坑（写代码前必读）

1. **`{}` 字面量 = `null`**，不是空 dict；空 dict 用 `json_parse("{}")`。
2. **无 `d[k] = v` 赋值**；dict 写用 `.set(k, v)`、查 `.has(k)`、读 `d[k]`。
3. **表达式可跨行（M70 起）**：list/dict/调用参数/元组/索引在括号（`[` `(` `{`）内可换行（含尾部逗号），语义与单行等价；但 `=` 后、二元/一元运算符后仍**不能**换行（语句边界以换行为准，需续行用括号包裹，如 `let x = (\n  a + b\n)`）；续行缩进须与缩进栈相容（不规则缩进仍 E2002）。
4. **dict 键限定 str**；键非 str 先 `str(k)`。
5. **`let` 不可变**（重新赋值报错），要改的用 `var`。
6. **顶层 `var`/`let` = 全局状态槽（M70-S3）**：主程序与 import 模块的顶层 var/let 声明均可跨函数访问/读写（var 可写、let 只读报 E3002；import 方启动时初始化一次，同名冲突用户值优先）；写**纯函数库**仍建议显式传参（确定性优先）。
7. **import 无副作用**（不执行模块顶层函数调用/裸赋值等语句）；仅模块顶层 var/let/const **声明**随合并导出并初始化一次（M70-S3，模块级状态槽的必要初始化，非任意副作用）。
8. **编译模式全功能**（native 287 全部可调）；**解释模式（pxi）M68 后同样零 extern def 可达全部 native**——但极端底层（ffi/指针）语义以编译产物为准。
9. **stdlib 内参数名不用 `fn`**（`fn` 是匿名函数关键字），用 `f` 等。
10. 注释/字符串里长行可加 `# noqa` 供 `pxc lint` 跳过。
11. **pxi（解释器）为 Mini 子集：不支持 `spawn`/`chan` 等并发关键字** → 并发/服务端（http_serve/ws_serve 等常驻回调）程序用 `pxc build`；纯计算与客户端脚本 pxi/编译双模式皆可。

## 2. native 内置速查（287 全量见 `docs/native_index.json`，本表为常用）

### 核心 / 值
`print` `len` `range` `type` `str` `int` `float` `bool` `assert` `input` `exit` `sleep` `abs` `sqrt` `min` `max` `pow` `sorted` `reversed` `sum` `map` `filter` `reduce` `contains` `env` `args` `gc` · 数学（M59）：`sin/cos/tan/atan2/floor/ceil/round/log/log10/exp/random/random_int/random_seed` + 常量 `pi/e`
> **M72 诊断（Issue 9/10）**：`print/println` 已**逐行实时**（管道/journald 下不再攒 8KB）；`flush()` 显式刷 stdout/stderr；`print_err(...)` 输出到 **stderr**（渲染同 print）。**编译产物运行时错误带 .px 源位置**：`运行时错误 [函数 行N]: 消息`（pxi 解释器本就带 `错误 [code] 行:列`）。**spawn 协程内运行时错误默认隔离**（打印现场后宿主继续；`PX_SPAWN_ISOLATE=0` 关 → 回退原 exit 语义）。

### 文件系统
`read_file(path)` → str · `write_file(path, s[, mode])` · `append_file` · `exists` · `list_dir` · `mkdir` · `remove` · `read_at/write_at`（随机）· `file_size` · `fsync_file` · `truncate_file` · `read_bytes/write_bytes`（bytes 读写）

### JSON / 编码
`json_parse(s)` → dict/list/标量 · `json_stringify(v)` → str · `json_path(d, expr)` / `json_path_set` · `base64_encode/decode` · `int_to_hex/hex_to_int` · `bytes_to_hex/hex_to_bytes`

### bytes 二进制
`bytes(s)` `bytes_len` `bytes_get/set` `bytes_slice` `bytes_concat` `bytes_to_str` `int_to_bytes` `bytes_to_int` `bytes_base64` `bytes_find` · `bit_count` `bit_length`

### 时间 / 定时 / 调度
`now()` `now_ms()` `now_us()` `sleep(sec)` `sleep_us` `time_format(t, fmt)` `time_parse` `tz_offset` · `set_timeout(f, ms, ...)` `set_interval` `clear_timer` · `cron("分 时 日 月 周", f)`（6 字段）

### HTTP（客户端/服务端）
客户端：`http_get(url)` `http_post(url, body[, headers])` `http_request(method, url[, body, headers])` `http_get_stream` · 服务端：`http_serve(port, handler)`（每请求回调）· `px_serve(port, docroot[, tls, opts])`（静态 + .px 应用服务器，opts 可 {http3:true, max_body_size, rate_limit...}）· `px_exec`（语言内嵌 .px）· `http_unix(sock, path, ...)`（Unix socket 客户端）

### WebSocket / SSE
`ws_serve(port, onmsg)` `ws_connect(url)` `ws_send` `ws_recv` `ws_close` `ws_ping` `ws_heartbeat(conn, ms, cb)` `ws_broadcast(server, msg)` `ws_connect_auto(url, ...)` · SSE：`sse_serve(port, cb)` `sse_send` `sse_close` `sse_connect(url)` `sse_read`

### TCP / UDP / TLS
`tcp_listen/accept/connect/send/recv/close` · `udp_open/send/recv/close` `udp_serve(port, cb)` · TLS：`tls_server(cert, key[, hostname])`（注册后 px_serve/WS/SSE 支持 HTTPS/WSS/TLS）

### SQLite
`sqlite_open(path)` → conn · `sqlite_exec(conn, sql[, params])` · `sqlite_query(conn, sql[, params])` → list[dict] · `sqlite_close` · `sqlite_escape` · `sqlite_last_insert_rowid`

### 加密 / 哈希 / 压缩 / XML / ZIP
AES：`aes_encrypt(key, iv, data)` / `aes_decrypt`（CBC-PKCS7）· `aes_gcm_encrypt/decrypt`（hex 文本版）· **M72 bytes 版（二进制安全，含 \0/非 UTF-8，GCM 输出 密文||tag 原始 bytes 与 Go crypto/aes-gcm 互通）**：`aes_gcm_encrypt_bytes/decrypt_bytes` `aes_encrypt_bytes/decrypt_bytes` · RSA：`rsa_gen_key(bits)` `rsa_encrypt/decrypt/sign/verify` · `sha256(s)` `xxhash(s)` · 压缩/解压（zlib，M61 FFI）：`zlib_compress` `zlib_uncompress` `zlib_crc32` · XML：`xml_parse(s)` `xml_escape` `xml_unescape` `xml_build` · ZIP：`zip_pack(files_dict, out)` `zip_unpack(bytes[, password])`（M66 支持 zipcrypto/AES-256 密码）

### Web 应用平台
Session：`session_open()/session_id/get/set/del/destroy` · `basic_auth(user, pass)` · `route(method, pattern, fn)`（:id 参数 / * 通配）· `middleware(fn)` `rate_limit` `vhost` `sandbox_enter` · 上下文 `ctx_set/get/clear` · 消息总线 `bus_new/subscribe/publish/unsubscribe` · `event_bus` · `gen_next`（生成器取下一项）· `list(xs)`（生成器→list）

### 进程 / 系统（M66 五件套 + M42+）
`os_pid()` · `os_exec(cmd, args)`（替换进程）· `os_spawn(cmd[, args])` / `os_spawn_capture` · `os_wait(pid)` · `os_kill(pid[, sig])`（group 组杀）· `os_capture(cmd)`（双管道分离捕获）· `os_popen(cmd, mode)`（双向）· `os_rename` `os_remove_all`（防删根）· `os_random_hex(n)` `os_file_sha256(path)` · `unix_connect(sockpath)` · `signal(sig, fn)` · `gc()`

### fd / 边缘设备（Linux）
`open(path, flags[, mode])` `close(fd)` `read(fd, n)` `write(fd, data)` `ioctl(fd, req[, arg])` `os_errno()` · `mmap/munmap/mem_write`（活映射）· `fcntl` `tty_config` `fd_wait`（poll）· GPIO/I2C/串口/PWM 走 `import std.edge`

### QUIC / HTTP/3（完整编译含 64 项；`--no-quic` 裁剪不含）
`quic_listen/accept/connect/close/close_listener` · `quic_open_stream/open_uni_stream/send_stream/recv_stream/poll` · `h3_server_listen` `h3_serve_read_request(_stream)` `h3_client_*` · QPACK：`h3_huff/unhuff` `h3_qenc/qdec/qs_*` `h3_settings_enc/dec` `h3_conn_*` —— 生产路径推荐直接 `px_serve(..., {http3: true})`（HTTP/1.1+2+3 三栈合一）。

## 3. 标准库速查（9 库，纯语言 .px，双模式一致）

| 库 | import | 核心函数（一行式） |
|---|---|---|
| collections | `import std.collections` | `unique(xs)` 去重 · `flatten(xs)` 展平 · `group_by(xs, fn)` 分组 → dict · `chunk(xs, n)` 分块 · `zip_lists(a,b)` 拉链 · `sort_by(xs, fn)` 按键排序 |
| semver | `import std.semver` | `sv_parse("1.2.3-a.1+b5")` → {major,minor,patch,pre,build,raw} · `sv_cmp(a,b)` -1/0/1 · `sv_satisfies(ver, "^1.2.0")` bool · `sv_best(list, rng)` 最高满足版 |
| webroute | `import std.webroute` | `wr_parse_file("get_healthz.px")` → {method:"GET", pattern:"/healthz", fnname}（main.px → null） |
| yaml | `import std.yaml` | `yaml_parse(text)` → {ok:true, value:node} / {ok:false, error:"line N: ..."} |
| pxml | `import std.pxml` | `pxml_parse(text)` → {ok,value}/{ok,error}（PXML 配置语言，规范 docs/PXML.md） |
| lunar | `import std.lunar` | `lr_solar_to_lunar(y,m,d)` → {ok,year,month,day,leap} · `lr_lunar_to_solar(y,m,d,leap)` · `lr_leap_month(y)` 等（1900-2100） |
| gfx | `import std.gfx` | `canvas_create(w,h)` → [w,h,pixels] · `set_px/get_px/line/rect/fill_rect/circle/fill_circle/blit/text/text_size`（像素 0xRRGGBB） |
| png | `import std.png` | `png_encode(w, h, pixels)` → bytes（PNG stored，配 gfx 画布） |
| edge | `import std.edge` | `gpio_request/input/output/read/write/wait/event` · `i2c_open/read_reg/write_reg` · `serial_open` · `pwm_setup/enable/set_duty`（失败返回 -1/false + os_errno） |

> 完整 API 文档：`tools/pxc doc stdlib/<name>.px`；用法示例见 `docs/ECOSYSTEM.md §2`。

## 4. 高频模式（可直接抄）

### 4.1 HTTP 服务端（http_serve 回调式 · 编译模式运行）

```px
# ⚠️ http_serve 常驻服务 + spawn → 用 `pxc build`（pxi Mini 子集无 spawn）
def handler(req):
    # req: {method, path, query, headers, body, form, files, ...}（dict）
    if req["path"] == "/" and req["method"] == "GET":
        return "hello"                        # str → 200 text/plain body
    if req["path"] == "/json":
        return {"status": 200, "body": "{}",
                "headers": {"Content-Type": "application/json"}}  # dict → 自定义
    if req["path"] == "/gone":
        return 404                            # int → 状态码（空 body）
    return {"status": 404, "body": "not found"}
spawn http_serve(18080, handler)
sleep(300)   # 等服务线程完成 bind
```

### 4.2 HTTP 客户端 + JSON

```px
var r = http_request("http://127.0.0.1:18080/json", "GET")  # → dict {status, headers, body}
print(r["status"])                                          # 200
var d = json_parse(r["body"])                               # body 为 str
var b = http_get("http://127.0.0.1:18080/")                 # → body str（网络失败返回 Err(result)，可 is_err()/? 处理）
var r2 = http_post("http://127.0.0.1:18080/", "payload")    # → body str
```

### 4.3 SQLite（CRUD）

```px
var db = sqlite_open("/tmp/app.db")
sqlite_exec(db, "CREATE TABLE IF NOT EXISTS t(id INTEGER PRIMARY KEY, name TEXT)")
sqlite_exec(db, "INSERT INTO t(name) VALUES(?)", ["pu"])
var rows = sqlite_query(db, "SELECT * FROM t")
for row in rows:
    print(row["name"])
sqlite_close(db)
```

### 4.4 文件读写

```px
write_file("/tmp/out.txt", "line1\nline2\n")
var s = read_file("/tmp/out.txt")
for ln in split(s, "\n"):
    if len(ln) > 0:
        print(ln)
```

### 4.5 px_serve 应用平台 + .px 脚本（文件即路由）

```px
px_serve(18085, "/srv/webroot")   # 静态 + .px 应用（get_healthz.px → GET /healthz）
```

### 4.6 WebSocket 服务端

```px
ws_serve(19090, fn (conn, msg):
    print("recv:", msg)
    ws_send(conn, "echo:" + msg)
)
```

### 4.7 pxpkg 建项目三步（registry 拉取，M45/M69）

```bash
pxpkg init myapp && cd myapp
pxpkg add semver@^0.1.0           # registry/<name>/<version>/<name>.px
pxpkg install                      # 生成 px.px.lock（可复现）
# 然后 import std.semver 直接可用
```

### 4.8 FFI：extern def 调用 C（M42，编译模式）

```px
import "c/sqlite3"
extern def sqlite3_open_v2(path: str, db: ptr, flags: int, z: ptr) -> int
# 编译模式全功能；pxi 亦经 FFI 桥支持已注册 C 函数
```

### 4.9 定时 + cron

```px
set_interval(fn (): print("tick"), 1000)
cron("*/5 * * * *", fn (): print("every 5 min"))   # 6 字段
set_timeout(fn (): print("once after 2s"), 2000)
```

## 5. 防漂移与源

- **native 清单**（287，单一事实源 = runtime 注册表）：`bash tools/gen_native_table.sh` → `docs/native_index.json`；CI 重跑 diff 防漂移。**本表计数必须 == count**（现 287）。
- **stdlib 索引**：`tools/pxc run tools/gen_ecosystem.px` → `docs/ecosystem_index.json`。
- 规范：`docs/spec.md`（§8 模块/import、§9 双模式、§12 AI 协议）· `docs/MINI_SUBSET.md`（子集边界）· 缺口与写库规范：`docs/ECOSYSTEM_GAPS.md`。
