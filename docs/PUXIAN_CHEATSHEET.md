# PuXian 速查包（PUXIAN_CHEATSHEET）

> **给 AI 的一句话**：把本文件 + [`docs/ECOSYSTEM.md`](ECOSYSTEM.md) 整包喂进上下文，即可写出**语法正确、native/库调用正确**的 PuXian（普贤）`.px` 程序。生成后务必用 `tools/px run <f>.px` 验证；编译模式 `tools/px build <f>.px`。
> 版本基线：M69（2026-09-05）· 双模式（编译 px build / 解释 pxi run）行为一致（M68 起 native 零 extern def 可达）。
> 工具链（M71，2026-09-06）：`px build` 已增量缓存（**二次 build ≈0.4–0.9s**）+ `--target <arch>` 交叉；`px mcp` 含 **build** 工具（AI 一条 MCP 写→验→交付）；安装 `tools/install.sh`（sha256 自动校验 + argv0 自发现，装完任意目录免 PX_STDLIB）。重文本/大文件处理：px build 编译版毫秒级 ≈ grep（ECOSYSTEM_GAPS F4 M71 更正）。
> M85（2026-09-06）：`px build` 模块裁剪开关集 —— `--no-sqlite`/`--no-ws`/`--no-zip`/`--no-xml`/`--no-aes`/`--no-rsa`/`--no-ed25519`/`--no-route`/`--no-zlib`/`--no-h2` 与 `--no-quic` 正交可任意组合，`--min` 聚合为最小化 profile；产物 9.0M → 2.7M；裁剪态调用缺的 native → 运行时明确报错（R1001，与裁剪编译产物一致）。
> M86（2026-09-06）：**官方命令名 `px`**（pxc 兼容别名，等价可用）；`px build` **默认按引用集自动最小**（编译期自动收集被引用 native，未引用模块自动去除；hello → ~2.7M / sqlite 用户自动保留 ~3.8M；`--full`/`--max` = 全能力逃生舱 ≈9.0M；显式 `--no-xxx`/`--min` 优先级 > 自动；解析失败自动退全量保编译成功）；`px refs <file>` 输出被引用全局/native 名集合。
> M88（2026-09-07，qg-issue 27）：**服务端并发止血** —— http_serve/http_serve_unix/sse_serve accept
> 不再每连接 spawn，接入**常驻连接池**（队满阻塞背压，服务进程不因连接数/spawn 槽满退出）；GC 线程槽
> 固定 64 → 动态上限。env `PX_SERVE_WORKERS`（池容量，默认 256，夹取 [8,4095]）/ `PX_MAX_THREADS`
> （spawn 线程上限，默认 1024，夹取 [64,4096]）。并发 100×500 压测全 200、0 失败、进程不崩。B 档（M88-B）起**空闲连接事件驱动**（Linux epoll）：http keep-alive / SSE 长连接 handler 返回后交还 IDLE 由事件循环照看，**空闲连接不占 worker**（池 8 挂 1 万 idle keep-alive / 1000 SSE 线程恒 10；SSE 空闲不超时，keep-alive 15s 超时语义保留）。env 追加 `PX_MAX_CONNS`（ConnCtx 连接登记上限，默认 16384，夹取 [1024,131072]）/ `PX_MAX_SSE_CONNS`（服务端 SSE 注册表容量，默认 4096，夹取 [64,65536]）。
> M117（2026-09-14，qg-issue 72）：**真实模块 ws-install PuXian 化暴露的三类缺陷修复** ——
> ① `read_file` 对 **st_size==0 的伪文件**（`/proc`、`/sys`）此前返回**空串**（Go `os.ReadFile` 不受影响）⇒
>    现按块读到 EOF（`/proc/sys/kernel/hostname`、`/proc/uptime` 正常）；
> ② `http_request` 的 `opts.timeout_ms` 此前**只约束收发、不约束 connect** ⇒ 对不可达对端挂到内核重传超时
>    （实测 >45s 不返回）⇒ 现连接阶段走**非阻塞 connect + poll**，且 `http_get`/`http_post` 等无 opts 入口
>    的路径统一受默认上限 **30s** 约束；
> ③ 正则字符类**不认识 POSIX 类** `[[:space:]]` ⇒ **静默不匹配**（返回 null、不报错）⇒ 现支持
>    `space/digit/alpha/alnum/upper/lower/xdigit/blank/punct/print/graph/cntrl`，**未知类名报错**。
> 另修文档漂移：`http_request` 签名（§2 曾写反为 `(method, url)`）、native 计数（311→312）。

---

## 0. 三件套先记住

1. **`.px` 文件 = 程序/模块**；注释 `#`；`##` 开头为文档注释（px doc 生成 API 文档）。
2. **运行**：`tools/px run hello.px`（解释，秒起）· `tools/px build hello.px`（生成 C→gcc 静态二进制，`<目录>/build/hello`）。
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
# ⚠️ M115 实测更正：通道收发是**方法**写法 `ch.send(v)` / `ch.recv()`；
#    速查包旧版写的 `send(ch, v)` / `recv(ch)` **不存在**（直接报 E2001 意外的 token: send）。
var ch = chan(2)
spawn fn ():
    ch.send("hi")
print(ch.recv())

# 列表推导/切片/字符串插值/管道
var sq = [x * x for x in range(5)]
var s = "val=${x}"
print("upper=" + to_upper("px"))
```

### ⚠️ 语言事实与坑（写代码前必读）

1. **`{}` 字面量 = `null`**，不是空 dict；空 dict 用 `json_parse("{}")`。
2. **无 `d[k] = v` 赋值**；dict 写用 `.set(k, v)`、查 `.has(k)`、读 `d[k]`。
3. **表达式可跨行（M70 起；M116 修好"闭合行缩进"）**：list/dict/调用参数/元组/索引在括号（`[` `(` `{`）内可换行（含尾部逗号），语义与单行等价；**括号内续行完全不受缩进栈约束** —— 闭合括号比语句更深（`return [1,\n            2]`）或顶格（`2]` 对齐第 0 列）都合法（M116 前：续行更深会在行尾多弹一级缩进，**把外层代码块提前结束**，报错行号还指向下一行行首）。但 `=` 后、二元/一元运算符后仍**不能**换行（语句边界以换行为准，需续行用括号包裹，如 `let x = (\n  a + b\n)`）。
4. **dict 键限定 str**；键非 str 先 `str(k)`。
5. **`let` 不可变**（重新赋值报错），要改的用 `var`。
6. **顶层 `var`/`let` = 全局状态槽（M70-S3）**：主程序与 import 模块的顶层 var/let 声明均可跨函数访问/读写（var 可写、let 只读报 E3002；import 方启动时初始化一次，同名冲突用户值优先）；写**纯函数库**仍建议显式传参（确定性优先）。
7. **import 无副作用**（不执行模块顶层函数调用/裸赋值等语句）；仅模块顶层 var/let/const **声明**随合并导出并初始化一次（M70-S3，模块级状态槽的必要初始化，非任意副作用）。
8. **编译模式全功能**（native 287 全部可调）；**解释模式（pxi）M68 后同样零 extern def 可达全部 native**——但极端底层（ffi/指针）语义以编译产物为准。
9. **stdlib 内参数名不用 `fn`**（`fn` 是匿名函数关键字），用 `f` 等。
10. 注释/字符串里长行可加 `# noqa` 供 `px lint` 跳过。
11. **pxi（解释器）为 Mini 子集：不支持 `spawn`/`chan` 等并发关键字** → 并发/服务端（http_serve/ws_serve 等常驻回调）程序用 `px build`；纯计算与客户端脚本 pxi/编译双模式皆可。
12. **pxi 的脚本参数必须在最后一个**（`selfhost/interp.px`：`let f = a[len(a) - 1]`）：直接调 `bootstrap/pxi` 要写 `pxi [数据文件...] script.px`；**写反会把数据文件当源码编译**，报 `未定义变量: 'xxx'` / `读取文件失败`，与真实病因（参数顺序）毫不相干。`tools/px run`（M106-S4 起）已自动把脚本置于末尾，`px run script.px [args...]` 可放心用。
13. **性能红线（写热循环前必读，M107-S0 实测）**：
    - **全局名访问 ≈40~50 ns/次**（顶层 `var`/`let`、native、`def` 皆然）。百万级热循环把**循环体内用到的全局名**（含循环上界、累加器、被调函数）先取到 `var` 局部：VM 轨 1MB 实测 **195 ms → 105 ms（1.9×）**（C 轨 97 ms → 20 ms）。小循环无感，大循环建议照做。
    - `s[i]` / `len(s)` / `for ch in s` 对 `str` 自 **M106** 起为**摊还 O(1)**（修复前 O(n²)：64KB 逐字符扫 4.74s→0.03s）。但**大文本逐字符**仍优先 `bytes(s)` 视图 + `bytes_get` / `b[i]`（bytes 下标本就是 O(1)，且无 rune 解码开销）。
    - **`{n,}` 无上限量词正则有 O(n²) 风险**（64KB 命中输入 8.8 s；固定 `{20}` / 纯字面 0 ms）——M107-S1 修。此前对**大文本**做密钥/敏感串扫描，先用**字面前缀预筛**（如 `contains(s, "sk-")` 为假即跳过该正则）。
14. **保留字坑（M115 实测，ws-ddns PuXian 化当场踩）**：`self` / `pub` / `send` / `recv` / `capture` / `trait` / `impl` / `match` … 都是**关键字**，
    不能当变量名（`var self = …` / `var pub = …` → `E2001 期望变量名，实际得到 self|pub`）。写端口/系统代码时 `self` 是高频命名，请改用 `exe` / `this` / `srv` 等。
15. **`args` 必须写成 `args()`**（M115 修）：此前解释轨**未注册**该内置名 ⇒ `px run` 下裸写 `args` 报 `R1001 未定义变量: 'args'`（仅编译产物可用）；
    且两轨形状不一致（解释轨给的是宿主 argv `[<path>/pxi, 用户参…, 脚本]`）。现已规范化：`px run s.px a b` 与编译产物同为 **`[程序, a, b]`**。
16. **`env_set` 的遮蔽坑（M115）**：解释器内置分发层 `selfhost/ibuiltin.px` 与编译器内部的 `selfhost/env.px`（变量环境）**同处一个编译单元**，
    而 `env.px` 里有同名 PuXian 函数 `def env_set(env, name, value)` ⇒ 在 ibuiltin 里直接写 `env_set(a, b)` 会**静默绑到内部辅助函数**上
    （报 `运行时错误 [env_set 行40]: 期望整数，实际是 string`，行列完全指不到病根）。新增宿主 native 若与 `selfhost/*.px` 内部函数同名，分发层**必须走 `ffi_call("名字", [...])`** 按名调用。
17. **`def main()` 是自动入口，别再手写 `main()`（M115 实测）**：定义了 `def main()` 时，**程序会在所有顶层语句之后自动调用它一次**（编译轨/解释轨一致）。
    若在文件末尾再显式写一行 `main()`，**整个程序会跑两遍**（实测：服务端日志重复、签名器会 spawn 两个守护进程、写文件写两次）。
    仓库内 `tools/gen_ecosystem.px`、`examples/repro_h2_vhost.px` 曾踩此坑（M115 修正）。
    **M116 起 `px lint` 会报 `L009` 告警**（只告警不改语义），见到就删掉那行。
18. **dict 缺键：两条轨都报 R1008（M116 统一）**：`d["缺的键"]` / `d.缺的字段` 在**编译轨与解释轨都**是运行时错误
    `R1008 字典没有键 'xxx'`（进程以非零码退出）。M116 前编译轨**静默返回 null**、解释轨才报错 ——
    实测后果：`str(r["role"])`（键缺失）在编译轨下得到字符串 `"null"` 并**被写进数据库**，是"静默数据损坏"。
    正确写法是**先守卫**：`if d.has("k"): ... d["k"] ...`；`{}` 的字面量是 `null` 不是空 dict（见第 1 条）。
19. **服务端 handler 出错 → 500（M116）**：`px_serve` / `http_serve` 的 handler / middleware 内抛运行时错误时，
    客户端收到 **`500 Internal Server Error`**（body 说明现场在服务端 stderr），**服务继续可用**。
    M116 前：VM 轨（协程化 handler）客户端收到 **`204 No Content`**（= 成功语义，客户端与监控全部误判、现场只在 stderr）；
    C 轨/同步轨更糟 —— 错误直接打穿到进程级，**整台服务器退出**（后续请求 Connection refused）。
    写 handler 请把"可能失败"的路径显式 `return {"status": 500, "body": ...}`，别指望它自己变成 500。

20. **`/` 是浮点除（int/int → float）**：`7/2 = 3.5`、`6/3 = 2.0`，`type(7/2) == "float"`。
    从 Go/Java/C 移植时**照抄 `a / b` 会静默算出小数**（实测：ws-install 的八进制权限位转换、
    人类可读大小格式化当场算错）。整除写 `int(a / b)`（向零截断）或 `floor`。
    另：超过 2^53 的整数经 `/ 1` 会丢精度（`9007199254740993/1` → `9.007199254740992e+15`）。
21. **`join(sep, list)` 分隔符在前**，与 `split(s, sep)` 相反（M117 实测：写错时错误信息是
    "join 分隔符需要 string"，指不到病根）。速查包此前未标签名 —— 本表已补。
22. **`env(name)` 变量不存在返回 `null`**（Go `os.Getenv` 返回 `""`）：`str(null) == "null"`，
    直接拼进配置就是**静默污染**；请先判 `null` 再取值。
23. **`str(bytes)` 返回占位符 `"<bytes N>"`**（不是内容、也不报错）—— bytes→str 用 `bytes_to_str`；
    `len(bytes)` 不支持（用 `bytes_len`）；二进制写盘用 `write_bytes`（无 mode 参数，权限位需 `write_file` 或系统 `chmod`）。
24. **`read_file` 支持伪文件（M117 修）**：`st_size==0` 的 `/proc`、`/sys` 文件此前读出**空串**
    （`/proc/sys/kernel/hostname` 读不到主机名）；现按块读到 EOF。补充：`read_file` 读不存在路径是**报错**，不是返回空串。
25. **HTTP 客户端返回类型不是统一的**（M117 起连接阶段也受超时约束）：
    - `http_get(url)` 成功 → **str**（body），失败 → **Result(Err)**；`http_request(...)` 成功 → **dict**，失败 → **Result(Err)**；
    - 因此**不能**直接 `.is_err()`（成功时抛 `R1007 dict/string 没有方法 is_err`），正确写法是先 `if type(r) == "result":` 再判；
    - `opts.timeout_ms`（默认 30000）**含连接阶段**（M117 前只管收发，对不可达对端会挂到内核重传超时）。
26. **正则支持 POSIX 字符类（M117 起）**：`[[:space:]]` / `[[:digit:]]` / `[[:alpha:]]` / `[[:alnum:]]` /
    `[[:upper:]]` / `[[:lower:]]` / `[[:xdigit:]]` / `[[:blank:]]` / `[[:punct:]]` / `[[:print:]]` /
    `[[:graph:]]` / `[[:cntrl:]]`。**M117 前**这些写法**静默不匹配**（`regex_search` 返回 `null`，不报错）；
    现未知类名（`[[:nope:]]`）**直接报错**。注意 `\s` 系列一直是支持的。

## 2. native 内置速查（312 全量见 `docs/native_index.json`，本表为常用）

### 核心 / 值
`print` `len` `range` `type` `str` `int` `float` `bool` `assert` `input` `exit` `sleep` `abs` `sqrt` `min` `max` `pow` `sorted` `reversed` `sum` `map` `filter` `reduce` `contains` `env`（⚠️ **变量不存在返回 `null`**，不是 `""` —— `str(null)` 会得到 `"null"`，取值请先判 null） `args()`（**调用式**：`px run s.px a b` 与编译产物同形 `[程序, a, b]`——M115 修；见 §1.1 事实清单）
· 易错：`join(sep, list)` 分隔符在前（见下）· `/` 是**浮点除**（见 §1.1 事实 20）`gc` · 数学（M59）：`sin/cos/tan/atan2/floor/ceil/round/log/log10/exp/random/random_int/random_seed` + 常量 `pi/e`
> **M72 诊断（Issue 9/10）**：`print/println` 已**逐行实时**（管道/journald 下不再攒 8KB）；`flush()` 显式刷 stdout/stderr；`print_err(...)` 输出到 **stderr**（渲染同 print）。**编译产物运行时错误带 .px 源位置**：`运行时错误 [函数 行N]: 消息`（pxi 解释器本就带 `错误 [code] 行:列`）。**spawn 协程内运行时错误默认隔离**（打印现场后宿主继续；`PX_SPAWN_ISOLATE=0` 关 → 回退原 exit 语义）。

### 文件系统
`read_file(path)` → str · `write_file(path, s[, mode])` · `append_file` · `exists` · `list_dir` · `mkdir(path[, mode])`（M116：mode 作用于**所有新建层级**，缺省 0755，最终仍受 umask 约束 —— `mkdir(dir, 0o700)` 才能表达"放私钥的目录"）· `remove` · `read_at/write_at`（随机）· `file_size` · `fsync_file` · `truncate_file` · `read_bytes/write_bytes`（bytes 读写）

### JSON / 编码
`json_parse(s)` → dict/list/标量 · `json_stringify(v)` → str · `json_path(d, expr)` / `json_path_set` · `base64_encode/decode` · `int_to_hex/hex_to_int` · `bytes_to_hex/hex_to_bytes`

### bytes 二进制
`bytes(s)`（⚠️ **`str(bytes)` 得到的是占位符 `"<bytes N>"`，不是内容**，也不报错 —— bytes→str 必须用 `bytes_to_str(b)`；`len(bytes)` 不支持，用 `bytes_len(b)`）`bytes_len` `bytes_get/set` `bytes_slice` `bytes_concat` `bytes_to_str` `int_to_bytes` `bytes_to_int` `bytes_base64` `bytes_find` · `bit_count` `bit_length`

### 时间 / 定时 / 调度
`now()`（**本地时间字符串** `YYYY-MM-DD HH:MM:SS`）`now_ms()` `now_us()` `now_sec()`（M115：Unix 秒，配 `time_format`）`sleep(sec)` `sleep_us` `time_format(t, fmt)` `time_parse` `tz_offset` · `set_timeout(f, ms, ...)` `set_interval` `clear_timer` · `cron("分 时 日 月 周", f)`（6 字段）

### HTTP（客户端/服务端）
客户端：`http_get(url)` `http_post(url, body[, headers])` ·
**`http_request(url, method[, body[, headers[, opts]]])`**（⚠️ **url 在前**；opts = `{timeout_ms, retries, proxy}`，
timeout_ms **含连接阶段**，默认 30000；`http_get`/`http_post` 无 opts 入口时同样受这 30s 上限约束）· `http_get_stream` · 服务端：`http_serve(port, handler)`（TCP 每请求回调）· `http_serve_unix(sock_path, handler)`（**Unix socket 服务端**，M82；自动清残留 + 0600）· `px_serve(port, docroot[, tls, opts])`（静态 + .px 应用服务器，opts 可 {http3:true, max_body_size, rate_limit...}）· `px_exec`（语言内嵌 .px）· `http_unix(sock, path, ...)`（Unix socket 客户端，M56）
> **M116：handler/middleware 内抛运行时错误 → 客户端收 `500`（含说明 body），服务继续可用**（此前 VM 轨收 204 静默成功、C 轨整台服务器退出，见 §1.1 事实 19）。

### WebSocket / SSE
`ws_serve(port, onmsg)` `ws_connect(url)` `ws_send` `ws_recv` `ws_close` `ws_ping` `ws_heartbeat(conn, ms, cb)` `ws_broadcast(server, msg)` `ws_connect_auto(url, ...)` · SSE：`sse_serve(port, cb)` `sse_send` `sse_close` `sse_connect(url)` `sse_read` · **M83-S6 同端口流式（http_serve/http_serve_unix）**：`http_stream(path, on_connect)` 把同端口某 path 注册为流式 SSE（on_connect(req) 内 `sse_send(req["conn"], chunk)` 逐块推、可 `sse_send` dict {event,data,id,retry}，on_connect 返回自动关闭；普通 JSON handler 同端口共存，流式路由优先；明文 HTTP/HTTP-over-unix，px_serve 面暂不接入）

### TCP / UDP / TLS
`tcp_listen/accept/connect/send/recv/close` · `udp_open/send/recv/close` `udp_serve(port, cb)` · TLS：`tls_server(cert, key[, hostname])`（注册后 px_serve/WS/SSE 支持 HTTPS/WSS/TLS）

### DNS（域名解析）
`dns_lookup(domain)` → list[str]（**M84-S3**，A+AAAA 全量返回，getaddrinfo；顺序即解析器返回序）——失败（NXDOMAIN/超时/无地址记录）返回 **Err("dns: ...")**，调用方可 `is_err()`/`?` 判定（区别于空 list）。守护域名解析（bs-safeip resolve_ips 类）不再依赖 getent 外部命令代偿。
`dns_txt(domain)` → list[str]（**M103-S2a**，手写 DNS TXT 查询，**M116 起 UDP 截断自动回退 TCP**）——多记录/长 TXT 域名（`google.com` 17 条、`cloudflare.com` 28 条）此前因 TC 截断直接 Err，现按 RFC 1035 §4.2.2 自动 TCP 重发。**语义注意**：无 TXT 记录 **与 NXDOMAIN 都返回空 list（非报错）** —— 与 Go `net.LookupTXT`（NXDOMAIN 报错）**不同**，移植 Go 代码时按"空 = 无记录"处理。

### SQLite
`sqlite_open(path)` → conn · `sqlite_exec(conn, sql[, params])` · `sqlite_query(conn, sql[, params])` → list[dict] · `sqlite_close` · `sqlite_escape` · `sqlite_last_insert_rowid`

### 加密 / 哈希 / 压缩 / XML / ZIP
AES：`aes_encrypt(key, iv, data)` / `aes_decrypt`（CBC-PKCS7）· `aes_gcm_encrypt/decrypt`（hex 文本版）· ⚠️ **bytes 版参数序是 `(data, key, iv)`，数据在前**，返回 `密文||tag`（nonce 要调用方自己拼，才能与 Go `aead.Seal(nonce, nonce, pt, nil)` 互通）· **M72 bytes 版（二进制安全，含 \0/非 UTF-8，GCM 输出 密文||tag 原始 bytes 与 Go crypto/aes-gcm 互通）**：`aes_gcm_encrypt_bytes/decrypt_bytes` `aes_encrypt_bytes/decrypt_bytes` · **M83-S2 ECB（PKCS7 无 IV，微信网关媒体 AES-128-ECB）：hex 版与 openssl enc -aes-128-ecb 逐字节一致，bytes 版供二进制媒体** `aes_encrypt_ecb/decrypt_ecb` `aes_encrypt_ecb_bytes/decrypt_ecb_bytes` · **M83-S2 gzip 通用**：`gzip_compress(bytes)`→bytes（标准 gzip 容器，与系统 gzip/Go compress/gzip 互通）`gzip_uncompress(gz)`→bytes|null · RSA：`rsa_gen_key(bits)` `rsa_encrypt/decrypt/sign/verify` · **M83-S4 标准签名（PKCS1v15-SHA256 + DigestInfo，与 Go `rsa.SignPKCS1v15`/openssl 互通；PEM 入参 `pk_parse` 自动 PKCS8/PKCS1/SPKI；msg str\|bytes 二进制安全、超长自动 sha256 无长度限制；不支持加密 PEM）**：`rsa_sign_pkcs1v15_sha256(pem_priv, msg)`→sig_hex `rsa_verify_pkcs1v15_sha256(pem_pub, msg, sig_hex)`→bool · **M83-S3 ed25519（RFC8032，与 Go crypto/ed25519 互通，确定性签名同 seed 同 msg 逐字节一致；PEM 收 Go x509 PKCS8/SPKI）**：`ed25519_sign(priv, msg)`→sig_hex（priv 收 hex seed32/sk64 或 PKCS8 PEM；msg 收 str|bytes）`ed25519_verify(pub, msg, sig)`→bool（pub 收 hex 或 SPKI PEM）· **M84-S2 HMAC-SHA256（RFC4231 官方向量 / 腾讯云 TC3 / SigV4 / webhook / JWT HS256；key/msg 均 str\|bytes 二进制安全可含 NUL，key>64B 自动先哈希）**：`hmac_sha256(key, msg)`→hex · `sha256(s)`（M84-S2 增强：收 str\|bytes 含 NUL 全哈希，不再 strlen 截断）`xxhash(s)` · 压缩/解压（zlib，M61 FFI）：`zlib_compress` `zlib_uncompress` `zlib_crc32` · XML：`xml_parse(s)` `xml_escape` `xml_unescape` `xml_build` · ZIP：`zip_pack(files_dict, out)` `zip_unpack(bytes[, password])`（M66 支持 zipcrypto/AES-256 密码）

### Web 应用平台
Session：`session_open()/session_id/get/set/del/destroy` · `basic_auth(user, pass)` · `route(method, pattern, fn)`（:id 参数 / * 通配）· `middleware(fn)` `rate_limit` `vhost` `sandbox_enter` · 上下文 `ctx_set/get/clear` · 消息总线 `bus_new/subscribe/publish/unsubscribe` · `event_bus` · `gen_next`（生成器取下一项）· `list(xs)`（生成器→list）

### 进程 / 系统（M66 五件套 + M42+）
`os_pid()` · `os_exec(cmd, args)`（替换进程）· `os_spawn(cmd, args[, group|opts])` / `os_spawn_capture`（group=true 子进程 setpgid 自成组，M83-S2）· `os_wait(pid)` · `os_kill(pid[, sig[, group]])`（group 组杀）· `os_capture(cmd)`（双管道分离捕获）· `os_popen(cmd, mode)`（双向）· `os_rename` `os_remove_all`（防删根）· `os_random_hex(n)` `os_file_sha256(path)` · `unix_connect(sockpath)` · `signal(sig, fn)` · `gc()`
> **M115 服务进程/环境补全**（ws-center / ws-ddns PuXian 化实测缺口）：`env_set(name, value)` / `env_unset(name)`（进程环境**可写**，子进程可继承）· `os_self_path()`（当前可执行文件绝对路径，守护化/自升级用）· `isatty(fd)`（TTY 判定，交互式提示只在终端弹）· `now_sec()`（Unix 秒，与 `time_format` 同轴 —— `now()` 是**本地时间字符串**，喂 `time_format` 会类型报错）。
> **`os_spawn` opts dict（M115）**：`os_spawn(cmd, args, {group:true, setsid:true, stdout:"/log/x.log", stderr:"…", stdin:false, cwd:"/tmp", env:{"K":"V"}})` —— 守护化不再需要 `/bin/sh -c "setsid … >>log 2>&1 &"`（旧法拿不到真实 pid）。`stdin:false` = `/dev/null`；`opts.env` **不污染父进程**；第 3 参传 bool 时语义不变（零回归）。

### fd / 边缘设备（Linux）
`open(path, flags[, mode])` `close(fd)` `read(fd, n)` `write(fd, data)` `ioctl(fd, req[, arg])` `os_errno()` · `mmap/munmap/mem_write`（活映射）· `fcntl` `tty_config` `fd_wait`（poll）· GPIO/I2C/串口/PWM 走 `import std.edge`

### QUIC / HTTP/3（完整编译含 64 项；`--no-quic` 裁剪不含）
`quic_listen/accept/connect/close/close_listener` · `quic_open_stream/open_uni_stream/send_stream/recv_stream/poll` · `h3_server_listen` `h3_serve_read_request(_stream)` `h3_client_*` · QPACK：`h3_huff/unhuff` `h3_qenc/qdec/qs_*` `h3_settings_enc/dec` `h3_conn_*` —— 生产路径推荐直接 `px_serve(..., {http3: true})`（HTTP/1.1+2+3 三栈合一）。

## 3. 标准库速查（13 库，纯语言 .px，双模式一致）

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
| html | `import std.html` | `html_parse(text)` → DOM（容错，坏标签自动纠正）· `html_text(dom)` 剥标签正文（script/style 跳过）· `html_query(dom, "p.a#id")` 简单选择 → [node] · `html_children/attr/tag` · `html_escape` |
| cookiejar | `import std.cookiejar` | `cj_new()` → jar · `cj_update(jar, resp_headers)` 解析 Set-Cookie · `cj_header(jar, url)` → "n=v; n2=v2"（domain/path/secure 匹配，会话保持）· `cj_clean` / `cj_len` |
| multipart | `import std.multipart` | `mp_encode(fields, files)` → {body: bytes, content_type: "multipart/form-data; boundary=…", len}（files 值 {filename, data: str\|bytes, type}）· `mp_boundary` |
| smtp | `import std.smtp` | `smtp_send(host, port, from, to, msg, opts?)` → bool（msg {subject, text\|html}；opts {user, password, helo} AUTH LOGIN）· `smtp_try` → {ok, err} 诊断 |

> 完整 API 文档：`tools/px doc stdlib/<name>.px`；用法示例见 `docs/ECOSYSTEM.md §2`。

## 4. 高频模式（可直接抄）

### 4.1 HTTP 服务端（http_serve 回调式 · 编译模式运行）

```px
# ⚠️ http_serve 常驻服务 + spawn → 用 `px build`（pxi Mini 子集无 spawn）
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

**Unix domain socket 服务端变体（M82 http_serve_unix）**：同 handler 契约、同解析管道，
仅监听面换成本地 sock 文件（本地 HTTP-over-unix 服务端，如 ws-approve serve / token-cache 网关）：

```px
# ⚠️ 编译模式（pxi Mini 子集无 spawn）
spawn http_serve_unix("/tmp/approve.sock", handler)   # 自动清理残留 sock + chmod 0600
# 客户端即 http_unix(sock_path, path, method, ...)（M56）
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

- **native 清单**（312，单一事实源 = runtime 注册表）：`bash tools/gen_native_table.sh` → `docs/native_index.json`；CI 重跑 diff 防漂移。**本表计数必须 == count**（现 312）。
- **stdlib 索引**：`tools/px run tools/gen_ecosystem.px` → `docs/ecosystem_index.json`。
- 规范：`docs/spec.md`（§8 模块/import、§9 双模式、§12 AI 协议）· `docs/MINI_SUBSET.md`（子集边界）· 缺口与写库规范：`docs/ECOSYSTEM_GAPS.md`。
