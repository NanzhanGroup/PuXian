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
> M141（2026-09-18，qg-issue 87 第 22 轮）：**墙钟纳秒 + unix 服务端 remote 对齐 Go** ——
> ① 新增 native **`now_ns()`**（CLOCK_REALTIME 的 `sec*1e9+nsec`，与 `now_sec`/`time_format`
>    同一时间轴）。此前语言里**没有墙钟纳秒**：`now_ms()`/`now_us()` 是 CLOCK_MONOTONIC
>    （自 boot 起算，测量语义）、`now_sec()` 只到秒 ⇒ Go 的 `time.Now().UnixNano()`、
>    `time.Now().Format("20060102-150405.000000000")`（纳秒目录名 / 日志时间戳）**表达不出来**；
>    拿单调钟冒充时**值与数量级都完全不同**（墙钟 ~1.8e18 vs 单调 ~1.2e11）。
> ② `http_serve_unix` 的 `req["remote"]` 由 `"unix"` 改 **`"@"`**（Go `net/http` 在 unix socket 上
>    `r.RemoteAddr` == `"@"`：未 bind 的对端 `RawSockaddrUnix.path` 为空 ⇒ Go 的 autobind 占位符；
>    Go 1.26.6 实测）。AF_INET 仍 `ip:port`，形状未变。
>    门 `examples/m141_now_ns/`（VM+C 双轨）；native 计数 330→331、内置名册 344→345。
> M145（2026-09-19，qg-issue 87 第 26 轮）：**循环引用值的比较/渲染/JSON 不再段错误** ——
> ① `==` 对齐 Go `reflect.DeepEqual`（环上「已访问对象对」再遇 ⇒ 相等）；② `str()`/`print` 按
> **路径**判定（共享而非环的对象完整渲染，环上渲染 `...`）；③ `json_stringify`/`json_stringify_go`
> 环上 `px_error`（`encountered a cycle via dict`，受控 rc=1 而非 139）；④ 解释轨 `ival.px`
> 的 `i_eq`/`i_to_str` 同批加环保护（三轨输出逐字节一致）。新 native **`object_id(v)`**（对象
> 同一性，`==` 是结构相等 ⇒ 指针语义移植/环检测必需）；native 计数 336→337、内置名册 350→351。
> 门 `examples/m145_cycle_safe/`（VM/C/解释轨 + 2 负控）。详见事实 117。
> M146（2026-09-20，qg-issue 87 第 27 轮）：**float64 位模式族 + 比较运算的 NaN 语义** ——
> ① 新增 native **`float64_bits(x)` / `bits_to_float64(u)`**（对应 M143 的 32 位一对；无它则读不出
>    float64 位模式、也**造不出** NaN/±Inf/非规格化数/−0.0）；native 计数 337→**339**、
>    内置名册 351→**353**；② 顺带根治 **缺陷 126**：`NaN != NaN` 曾为**假**、
>    `NaN <= x` / `NaN >= x` 曾为**真**（`compare_values` 三态器把 NaN 落进"相等"分支），
>    现为 IEEE754「四路皆假」且**容器**里 NaN 判不等（对齐 Go `reflect.DeepEqual`）；
>    ③ 门 `examples/m146_float64_bits/`（577 行语料 × Go 本尊逐字节 × VM/C 双轨 + 54 自断言 + 3 负控）。
>    详见事实 118 / 119。
> M148（2026-09-20，qg-issue 87 第 30 轮）：**`/` 完全 IEEE + Go `%v` 浮点文本 + `str()` 往返根治** ——
> ① **缺陷 118 根治**：`/` 是浮点除法 ⇒ **完全 IEEE-754**（`x/±0.0` → ±Inf、`0.0/0.0` → NaN、
>    溢出 → ±Inf、下溢 → ±0.0），**永不报错**；对齐 Go 的**整数**除零 panic 由 `//`（`px_idiv`）
>    与整数 `%` 守护（仍 `px_error`）。**浮点 `%` = `fmod`**（÷0 → NaN），注意它取**被除数**符号
>    （`-5.0 % 3.0` = `-2.0`），与整数 `%` 的**欧几里得**余数（`-7 % 3` = `2`）不是一回事。
> ② **新增 native `go_float_text(x[, bits])`（缺陷 132）**：Go `fmt.Sprint` 的浮点文本
>    （= `strconv.FormatFloat(f,'g',-1,bits)`）—— 最短往返、**不带尾随 `.0`**（250.0 → `"250"`）、
>    **十进制指数 ≥ 6 即科学计数**（1234567.0 → `"1.234567e+06"`）、指数**至少两位且带符号**
>    （`"1e+06"` / `"1e-05"`）；`bits=32` 走 float32 最短往返（1/3 → `"0.33333334"`）。
>    与 `str()` 的语言约定**并存不覆盖**。
> ③ **缺陷 133 根治**：`str()` 的**定点分支**上界写死 `dec <= 17`，而 `x ∈ [-4,-1]`（`0.000…`
>    形态）需要 `17-x` 位小数 ⇒ 落到**不可往返**的文本（静默丢精度：
>    `0x3f2d1ac1aeaf35e2` → `"0.00022204984938272"`）⇒ 上界改为 `17 - x`（钳 1..21）。
> ④ **`str()` 的非有限值是语言约定**：`inf` / `-inf` / **`nan`（NaN 一律 `nan`，不看符号位）**
>    —— 这一族有 `golden/s09_unicode_edge` 钉着，**不要动**；Go 的文本口径走 ②/`fmt_float_dec`。
>    NaN 的符号位归一顺带收口了一条潜伏缺陷（`cg_fmt_float` 的白名单缺 `-nan`）—— 见事实 127。
>    门 `examples/m148_ieee_div/`（1176 行语料 × Go 本尊 × **VM/C/解释轨三轨** + 44 断言 +
>    **往返性质** + **5 道负控**）。详见事实 124–127。

> M155（2026-09-20，qg-issue 87 第 37 轮）：**含内嵌 NUL 的字符串：字面量 → 常量池 → 渲染 → 输出** ——
> ① **缺陷 148 根治**（第 35 轮登记）：`"\u{0}"` 字面量此前在三轨被**静默丢弃**（`len("\u{0}") == 0`）。
>    两层成因咬在一起：**发射层**（`cg_escape_str` 明确「NUL 丢弃」+ `px_str()` 按 C 串重造）与
>    **编译器自身源码里也有 `"\u{0}"` 字面量**（旧编译器把它编成空串 ⇒ `_out.append("")`、`c == ""`
>    ⇒ 三轨「**一致地错**」）。修法：编译器源码改**运行时构造** `nul_char()`（`bytes_to_str(int_to_bytes(0,1))`
>    —— 旧编译器编新源码也正确，**自举安全**）；发射侧含 NUL 的常量改 `PX_STR_LIT(lit)`（C 轨）/
>    `PXK_STR_LIT(lit)`（VM 轨 K 表），长度由 C 的 `sizeof(lit) - 1` **编译期**给出；`vm_loadk` 用
>    K 项 `i` 字段走 `px_str_const_n`（复用指针键常量池 ⇒ LOADK 仍零分配）。**不含 NUL 的常量发射文本
>    一字不变**（257 件 `emitc_freeze` 门守着）。
> ② **缺陷 150 根治**：`print`/`print_err`/`px_fmt_value` 此前一律 `strlen` 收尾 ⇒ `print("A\0B")`
>    只写 `A`（Go 的 `fmt.Print` 写 3 字节）。新增 `px_fmt_value_n(v, &len)` 全长出长度；dict 渲染的
>    排序项改成「文本 + 长度」随行（qsort 用 `PxDisp`）；`bi_str` 的容器分支改 `px_str_len`。
> ③ **缺陷 151 根治**：`$BUILD` 里的**陈旧 runtime.{c,h} 副本**会遮蔽 `#include "runtime.h"`
>    （`#include "x.h"` 先搜**包含者目录**）⇒ 用旧头编译新 C（实测 pxfmt/pxdoc 失败：新宏不可见；
>    此前 10 件"成功"只是没发射新宏，**坑一直在**）。重烘前清掉同名副本。
> ④ **缺陷 152 根治**：`json_stringify`（语言自有变体）按 C 串走 ⇒ NUL 处**丢尾巴**，与文档化的
>    「控制字符**裸输出**」契约不符（契约说的是裸输出、不是截断）⇒ 改长度感知（NUL 也裸出）。
>    **要 Go 口径就用 `json_stringify_go`**（NUL → `\u0000`、0x01 → `\u0001`，M129 已对齐）。
> 门 `examples/m155_nul_bytes/`：三轨逐字节（41 断言 × 3 轨）+ **stdout/落盘原始字节**逐字节
>    （`cmp`，不是人眼看）+ 发射形状（`PX_STR_LIT("\000…")` 恒 3 位八进制）+ 词法/AST 面
>    + 自举安全不变式 + **4 道负控**（长度退回 strlen / print 退回 printf / 长度多算 1 / VM 忽略 K 长度）。
> M154（2026-09-20，qg-issue 87 第 36 轮）：**分配率第二刀（编译器自身热路径）+ `join` 字节口径** ——
> M153 把 runtime 侧三条"每次执行都新建对象"清掉后，插桩归因（`gc_register` × PuXian 源行）显示
> 剩下 **301 万次分配**里绝大部分在**编译器自己的循环**：① `bc_emit_c_program` 每条指令的
> **7 段 `+` 链**独占 **108.9 万（36%）**；② `rust_unescape` 逐字符 `out += …` 36.3 万；
> ③ `rust_str_debug` 19.0 万；④ `skip_comment` 18.4 万；⑤ `scan_ident_token` 的 `s += advance()` 17.6 万；
> ⑥ `g_src[g_pos]` 的**多字节 rune** 分支每次新建（中文语料里 peek/advance/ppos 合计 24 万）。
> 修法四类：**a)** 逐字符累积改「片段列表 + 一次 `join`」（`join` 是 M152 的游标 memcpy、单次分配，
> 列表元素还是 M153 池化过的单字符对象）；**b)** 新增 **多字节 rune 短串池**（2..4 字节，内容键）
> 与 **小整数 `str()` 文本池**（[-64,4096]，文本口径与 `px_fmt_value` 的 `%lld` 分支逐字节相同）；
> **c)** 常量池改**按指针哈希**（键本来就是指针 ⇒ 免掉每次求值的 strlen + 全串 FNV）；
> **d)** 比较器加**对象同一性快路径** + 钉住表去重改**哈希集合**（线性扫描在池化把表推到数千条后
> 变成 O(n²)，实测占 10.0% 采样）。**顺带根治缺陷 149**：`join` 的项与分隔符改按**显式字节长**
> 拼接 —— 旧实现 `strlen` 会在**内嵌 NUL** 处静默截断，于是 `join` 与 `+`（`px_add` 按 `str.len`）
> 对同一数据给出**不同**结果，也与 Go `strings.Join` 不一致（这正是 `out +=` 改 `join` 的**前提**）。
> 结果（token-cache 整图 73 模块 / 16936 行）：分配 **2,990,601 → 1,200,544（−59.9%）**、
> 字符串 **2,496,944 → 476,406（−81%）**、字节 619MB → 289MB、`--emit-c` **9.51s → 7.91s**（−17%）；
> 发射文本**逐字节不变**（255 件冻结门 + 本门 golden 双证）。新门 `examples/m154_alloc2/`
> （三轨语义 57 断言 · 预算硬阈值 · 输出冻结 · **3 道负控**）。详见事实 152–157。

> M153（2026-09-20，qg-issue 87 第 35 轮）：**分配率：字面量/空串/单字符池化 + 分配统计可观测** ——
> 上一轮把「发射平方级」拆掉后，`--emit-c` 仍**每次编译分配 284 万个对象**（其中 **96.4% 是字符串**、
> 513MB）。插桩 runtime 做**三级归因**（按类型 / 调用点 / **PuXian 源行**）后定位到三条"每次执行都
> 新建对象"的路径：① VM 的 `PXOP_LOADK` 每次**复制**常量（`vm_loadk` → `px_str`）；② `s[i]` 每次
> 新建一个单字符对象（Go 的 `s[i]` 是零分配字节）；③ `px_str_len(s,0)` 每次新建（Go 里空串零分配）。
> 修法：**常量池**（按**指针**缓存静态字面量；`vm_loadk` 的 `PXK_STR` 改走它）+ **空串单例** +
> **ASCII 单字符串表**（按字节值索引 256 项，零哈希）；三者都是"同内容共享**不可变**对象"，依据是
> `px_str_len` 的既有语义不变量（PX_STR 的 data 恒不可变、全仓无就地改写）⇒ 可观测语义零变化
> —— **255 个用例的 `--emit-c` 输出逐字节不变**即其证据。三者必须**永久存活** ⇒ 新增**钉住表**，
> 在两条 GC 标记路径与全局表同批标记。
> 结果：分配 **2,840,232 → 483,635（−83%）**、字符串 **−86%**（513MB → 75MB）、**GC 次数 3 → 0**；
> `--emit-c` 4000 语句 **1.94s → 0.83s**、**token-cache 整图 34.38s → 9.81s（3.5×）**。
> 另：新增观测 `PX_GC_ALLOC_STATS=1`（分配计数**完全确定** ⇒ 可做硬阈值门）、新门
> `examples/m153_alloc/`（三轨语义 + 预算 + 冻结 + 3 负控）与常驻门 `selfhost/emitc_freeze.sh`
> （255 件发射冻结）；顺带根治**缺陷 147**（重烘门"取不到判据就跳过"是退化放行暗门）。
> 详见事实 147–151。

> M152（2026-09-20，qg-issue 87 第 34 轮）：**整图发射的平方级爆炸 + GC 的「字节盲区」** ——
> token-cache 整图（73 模块 / 17k 行 / 913 函数）在**用户面默认轨** `tools/px build` 上跑不完：
> 实测 3:37 时 RSS **8.1GB**、零输出（再跑到 4:45 涨到 8.35GB，把整机拖进 swap，同机 gcc 卡 D 态）。
> 这正是 2026-09-19 那 9 次**全机 OOM**（QQ 网关被 systemd stop+restart ⇒ 闪断）的枪。
> 三段实测定位：① 语料放大曲线 2000/4000/8000/16000 语句 ⇒ 3.0s/9.9s/32.5s/**114.2s**、
> RSS 125MB/450MB/1769MB/**7196MB**（时间 ~N^1.7、**内存 ~N²**）；② `perf`：**70% 时间在 GC**
> （`gc_mark_obj` 44.6% + `px_gc_collect` 25.7%），`vm_run_loop` 只 2.9%；③ `PX_GC_DEBUG=1`：
> 单次编译 **339 次 GC**（后 200+ 次「回收 0 个」）、累计分配 ≈ **3400 万对象**。根因两条线：
> **发射线** `out += chunk`（`px_add` 每次整串重分配 + `px_str_len` 再深拷）平方级 + `join` 自身
> 也是 `strcat` 平方级、还漏一个总长缓冲；**回收线** `g_gc_trigger_bytes` 是**死代码**
> ⇒ GC 只认对象数（`str`/`bytes` 的载荷**已**计入 `est`，只是阈值永不启用）。修法：块表 + 一次
> `join`（`bc_emit_c_program` 61 处 + `cg_generate` 三处）· `join` 游标 memcpy + 归还缓冲 ·
> `GC_TRIGGER_BYTES_DEFAULT=512MiB`（`PX_GC_TRIGGER_BYTES` 覆盖）· `GC_THRESHOLD_DEFAULT`
> 10 万 → **100 万对象**。**硬判据**：改前/改后对 **253 个 `.px`** 取 `--emit-c` sha256 **逐条相同**；
> 语料曲线 func 1000：19.61s/3620MB → **9.32s/133MB**（内存 27×↓）；整图真身：**跑不完 → 153.44s /
> 236MB / rc=0**。详见事实 141–143。
> ↳ 同轮**顺带照出并根治缺陷 146**：字节阈值一打开，m144 门的 VM 轨就稳定报
> `net: http_unix 请求失败: 连接关闭 (4)`（4 = **EINTR**）—— **阈值本身没坏**，是它把
> 「`recv`/`send` 不重试 EINTR ⇒ stop-the-world GC 的暂停信号把在途请求打成网络错误」这条
> 潜伏路径照了出来（`PX_GC_TRIGGER_BYTES=0` ⇒ 全绿，最小差分定死）。修后同一门在默认阈值下
> `PASS=115 FAIL=0`。详见事实 146。


> M151（2026-09-20，qg-issue 87 第 33 轮）：**`tls_upgrade`（同一 fd 上升级 TLS）+ 三处根治** ——
> PostgreSQL 驱动的线上行为比 Redis 多一道**协商**：lib/pq 在 `sslmode != disable` 时先在**明文**上发
> 8 字节 `SSLRequest`（`int32(8) + int32(80877103)`）→ 读 1 字节 `'S'` → **再在同一个 fd 上**做 TLS
> 握手。而 `tls_connect`（M150）**自己建 socket** ⇒ 这条路**表达不出来**（"先明文发协商包"与
> "从握手开始"不在同一个 socket 生命周期里）。新增 **`tls_upgrade(fd[, opts])`**：返回 dict 与
> `tls_connect` **同形**，`opts = {"verify"（**缺省 false**）, "servername", "host", "read_timeout_ms"}`；
> **所有权**：成功 ⇒ 句柄表**接管 fd**（此后**不要再 `tcp_close`**）；失败 ⇒ fd **仍归调用方**。
> 与 `tls_connect` 共用 100% 的"配置 + 握手"代码（`px_tls_session_alloc` / `_config` / `_handshake`
> 三段式）—— 不给"两份实现必然漂移"留机会。
> 三处根治：**缺陷 137** `tcp_send` / `tcp_send_ex` 不认 `bytes` ⇒ 把 `str(bytes)` 的占位符
> `"<bytes N>"` 发上线（**载荷一个字节都没发，却返回成功**）—— 二进制协议静默损坏，对齐 `sse_write`；
> **缺陷 138** 解释轨的"模块缺失"诊断本身把进程打挂（`cg_pwarn` 未定义 —— `cg_module.px` 被两条轨以
> 不同 import 闭包加载）⇒ 诊断在 `cg_module.px` 内**自足**（`cgm_perr` / `cgm_pwarn`），且 **`px run`
> 不再合并 stderr**（与 `pxc` 同口径：诊断走 stderr、不污染数据流）；**缺陷 139** 负控门被打断 ⇒
> `runtime/*.c` 会**静默**留在**篡改态**（本轮开工实测 md5 与门内快照不一致 —— 差一步就带着它上库）⇒
> `NEGCTL` 残留标记 + 开门预检 + 全门预检 + 信号兜底。
> 门 `examples/m151_pg_tls_bytes/`（① 二进制安全 ② `tls_upgrade` 与 **Go `crypto/tls`** 同一服务端对拍
> ③ 解释轨诊断 ④ **3 道负控**）⇒ `M151-VERIFY-OK`；不依赖外网。详见事实 **137–140**。

> M150（2026-09-20，qg-issue 87 第 32 轮）：**摘要/密钥派生族 + TLS 客户端族** —— 移植
> PostgreSQL 驱动（lib/pq v1.12.3）时撞到的两块硬缺口：
> **① 认证面**：`AuthenticationMD5Password` 的应答是一条**嵌套 MD5**
> （`"md5" + hex(md5(hex(md5(password||user)) || salt))`），而语言里只有 `sha256`/`hmac_sha256`；
> `SCRAM-SHA-256`（PG 14+ 服务端默认认证）的 `SaltedPassword = Hi(pw, salt, i)` 就是
> **PBKDF2-HMAC-SHA256**，手搓只能用 `hmac_sha256`（返回 **hex 文本**）做每轮 hex↔bytes 往返。
> 新增 `md5(data)`（32 位小写 hex）/ `md5_bytes(data)`（16 字节）/
> `pbkdf2_sha256(password, salt, iterations, dklen)`（**原始字节**；`iters<1→1`、
> `dklen<1→32`、`dklen>4096→4096`）。
> **② 客户端 TLS**：语言此前**只有服务端 TLS**（`tls_server`），客户端 TLS 只存在于
> `http_get`/`http_post`/`s3_*` 的**内部**（`https_connect_t`）⇒ 任何"自己的协议跑在 TLS 之上"
> 的客户端都写不出来。而 lib/pq 的 `sslmode` **缺省即 `require`**（`ssl.go` 的 `mode == ""`
> 分支），require 的语义是 `InsecureSkipVerify = true`（**加密但不校验证书**）—— 没有这一族，
> PG 客户端只能靠 `sslmode=disable` "装得像"。新增四个 native（与 `tcp_*_ex` 逐条同构，
> 一律返回结果 dict、**永不杀进程**）：
>   · `tls_connect(host, port[, opts])` → `{ok, id, fd, peer, version, version_num, cipher, cipher_id, verify, stage, errno, err}`；
>     `opts = {"timeout_ms": int, "verify": bool（**缺省 false** = libpq 的 require 语义）, "servername": str, "read_timeout_ms": int}`；
>     `version_num` = **Go `tls.VersionTLS13` 口径**（0x0304）；`cipher_id` = IANA 套件号
>     （= Go 的 `tls.CipherSuite` 常量值）；`cipher` = **Go `tls.CipherSuiteName` 口径**
>     （`TLS_AES_256_GCM_SHA384`，不是 mbedtls 的 `TLS1-3-AES-256-GCM-SHA384`）；
>     `stage ∈ ""|"resolve"|"socket"|"connect"|"tls"`。
>   · `tls_send(id, data)` → `{ok, n, err}`（失败时 `n` = 已写出字节数）。
>   · `tls_recv(id, maxlen)` → `{ok, data, n, eof, timeout, err}`（`close_notify`/FIN ⇒ `eof=true`，**非错误**）。
>   · `tls_close(id)` → bool（幂等；重复关闭返回 false）。
> 门 `examples/m150_tls_crypto/`：`corpus.txt` **148 条**（md5 47 + pbkdf2 81 + PG md5 认证 20）
> ⇒ 与 **Go 本尊**（crypto/md5 + crypto/hmac·sha256 手写 PBKDF2）**195 行逐字节**对拍 ×
> **VM+C 双轨** + 自断言 **26 条** + 解释轨冒烟 11 条；TLS 侧用**受控 Python TLS 服务端**
> （自签证书）把 PuXian 与 **Go `crypto/tls`** 放**同一个服务端**上跑同一套动作（实现无关子集
> 逐字节一致 + `tls.CipherSuiteName(px_cipher_id) == px_cipher` + 服务端 `sni_callback`
> 确实收到显式 `servername` + IP 字面量**不发** SNI）+ **3 道负控**（md5 只取 15 字节 /
> pbkdf2 钳位改 2 / TLS 缺省改校验证书 ⇒ 全判红）。
> M149（2026-09-20，qg-issue 87 第 31 轮）：**TCP「带超时 + 可辨别失败」族** —— 旧 `tcp_*` 的**失败面**
> 表达不出 Go 的 `net`（连接失败**杀进程**、无连接超时、`tcp_recv` 把 EOF/超时/出错**都返回 ""**），
> 于是「网络是常态故障源」的客户端（Redis / PostgreSQL / 任意带 deadline 的协议）**无法移植**。
> 新增四个 native（与旧 `tcp_*` **并存不覆盖**，一律返回结果 dict、**永不杀进程**）：
>   · `tcp_connect_ex(host, port[, opts])` → `{ok, fd, addr, peer, stage, errno, err}`；
>     `opts = {"timeout_ms": int, "nodelay": bool}`；`stage ∈ ""|"resolve"|"socket"|"connect"`；
>     **默认 setsockopt(TCP_NODELAY,1)**（Go `net.Dial` 对 TCP 默认开启）；成功时 `peer` = Go
>     `RemoteAddr()` 形态（`ip:port`），`addr` = 数字地址（IPv6 带方括号，与失败路径同形）。
>   · `tcp_opt(fd, opts)` → `{ok, nodelay, keepalive, read_timeout_ms, write_timeout_ms, errno, err}`：
>     **改完回读（getsockopt）并返回生效值** ⇒ 超时配置**可编程验证**（否则只能"相信"）；
>     缺键=不改；`*_timeout_ms = 0` = **无限**（`SO_RCVTIMEO`/`SO_SNDTIMEO` 清零）。
>   · `tcp_recv_ex(fd, maxlen)` → `{ok, data, n, eof, timeout, errno, err}`：`n==0 ⇒ eof=true`
>     （对端 FIN，**非错误**）；`EAGAIN/EWOULDBLOCK ⇒ timeout=true` + `err="i/o timeout"`
>     （Go `os.ErrDeadlineExceeded` 文案）。**EOF / 超时 / 出错三者可辨别**。
>   · `tcp_send_ex(fd, data)` → `{ok, n, timeout, errno, err}`：循环写完（EINTR 续写），
>     失败时 `n` = **已写出**字节数（对齐 Go `Conn.Write` 的 `n`）。
> 门 `examples/m149_tcp_deadline/`：**受控服务端**（独立进程，按首行 mode 扮演沉默/问候即关/
> 分段/大包/收取/立即关）⇒ VM+C 双轨各 **53 断言**（连接超时 300ms 被真正遵守、黑洞地址 errno=110、
> refused=111、解析失败 stage=resolve、`nodelay` 默认 true、读超时 ≥250ms 且 errno=EAGAIN、
> EOF≠超时、分段无帧边界、100000 字节收发、对端关闭后写 EPIPE）+ 解释轨冒烟 9 断言 +
> **3 道负控**（去掉连接超时 / 关掉 NODELAY 默认 / 把 EOF 报成非 EOF ⇒ 全判红）。详见事实 128–130。


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
var d = {}                      # 空 dict：M129 起可直写（此前 {} 是 null，旧写法 json_parse("{}") 仍可用）
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

1. **`{}` 字面量 = 空 dict（M129 起；qg-issue 87 缺陷 15）** —— 此前求值为 `null`（导致「空字典字面量」在语言里没有写法，只能 `json_parse("{}")`）。旧写法仍可用、行为一致；空 dict 与空 list 一样是**假值**。
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
    **M128 补充**：`from` 同样是保留字（`var from = ""` → `E2001 期望变量名，实际得到 from`；api-server 移植 `handlers_session.go` 的变量名 `from` 当场踩到，改名 `src`）。
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
    正确写法是**先守卫**：`if d.has("k"): ... d["k"] ...`（`{}` 自 M129 起是空 dict，见第 1 条）。
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

27. **字符串插值里可以写转义引号了（M118）**：`print("v=\${d[\"k\"]}")`、`print("\${contains(s, \"x\")}")` 两轨都合法。
    M118 前报 `E1001 非法字符: '\'`（位置指向反斜杠，与病根"插值扫描不认转义"无关）。
    注意：插值内的嵌套字符串仍以 `\"` 为定界符书写。
28. **匿名函数支持多行体（M118）**：`var f = fn (x):` + 换行缩进块；**调用实参位**同样支持
    （`map(ys, fn (x):` + 换行语句体，括号内 lexer 不产 INDENT/DEDENT，parser 按换行解析到 `)`/`]`/`,`）。
    M118 前这两处都报 `E2001 意外的 token: 换行`——**速查包 §1 自己的示例就编译不过**。
29. **`spawn` 只支持直接函数调用（`spawn f(args)`）**；`spawn fn (): …` / `spawn <闭包>` 现在是
    带源位置的 `E2011`（含两种可用写法）。M118 前 VM 轨把**编译器内部函数名 + AST dump** 抛给用户，
    C 轨甚至把错误文案当 C 代码返回。
30. **`int_to_hex(n, width)` 是「取低 width×4 位」，不是「转十六进制」**：`int_to_hex(12345678901, 1) == "5"`、
    `int_to_hex(255, 4) == "00ff"`。移植 Go 的 `%x` 请用 `int_to_hex(n, 16)` 再去前导零。
31. **Result 没有「取错误值」原语**：只有 `is_ok/is_err/unwrap`（`unwrap` 在 Err 上 panic）。
    移植 Go 的 `(val, err)` 双返回时只能从 `str(Ok(v)) == "Ok(v)"` / `str(Err(e)) == "Err(e)"` 剥壳。
    （原生缺口：建议补 `unwrap_err`/`expect`。）
32. **`fn` 也是保留字**：`var fn = json_parse("{}")` → `E2001 期望变量名，实际得到 fn`（改用 `fndef` 等）。
33. **括号内可跨行续行（M119，Go/Python 语义）**：`(` `[` `{` 内，长表达式可在**运算符处**换行，
    以下都合法 —— 运算符放行尾：`print("a" +` ⏎ `"b")`；运算符放行首：`print("a"` ⏎ `+ "b")`；
    多行字典**不需要尾逗号**：`{"k": "v"` ⏎ `}`。
    M119 前：行尾 `+` → `E2001 意外的 token: 换行`；行首 `+` → `E2001 期望 ')'，实际得到 +`
    ⇒ 长表达式只能挤成一行（**编译器自身源码亦被迫如此**，supervisor 加一行日志就撞上）。
    边界：**括号外行尾**运算符自 M129 起同样续行（见第 43 条，Go 自动分号插入语义），
    **括号外行首**运算符仍报错；括号内**行首**的一元运算符
    （`-` `~` `not`）不参与续行（与「新语句以一元运算符开头」歧义）—— 把运算符写到行尾即可。

34. **缺键 = 运行时错误 `R1008`（M116 起三轨统一；M120 起配套完善，口径=「保持严格」）**：
    `d["缺失键"]` 不返回 `null`，直接报错杀进程（非协程路径 rc=1）。**可选键必须显式守卫**：
    `if st.has("k"): … st["k"] …`（嵌套逐层守卫）。`d.get(k, 默认)` 的默认值**只覆盖「键不存在」**，
    键在而值为 null 仍返回 null。（`{}` 自 M129 起是空 dict，见第 1 条 —— 此前是 null。）
    迁移指引（逐条错误码表 + 存量扫法 + ws-core 案例）：**`docs/DICT_STRICT_MIGRATION.md`**。
35. **索引/字段/方法族的错误码三轨一致（M120）**：dict 缺键 `R1008: 字典没有键 'k'` · 结构体缺字段
    `R1008` · 其它类型缺字段/无方法 `R1007: 类型 <t> 没有方法 'm'` · 索引越界 `R1003` ·
    键/参数非字符串 `R1002: 方法 has 参数 1 需要 string`。
    修前编译轨这批错误**不带错误码**、且文案用「对象」而非「类型」（按码匹配的工具会漏）。
36. **协程内运行时错误不再静默（M120）**：隔离语义保留（宿主继续；handler 出错给客户端 500），但
    ① 隔离时打印 `[px-coro] 协程运行时错误已隔离（第 N 次…）`；② **进程正常退出时返回 1**
    （修前 rc=0 = 假成功：顶层工作全在协程里的服务/守护崩了也报「成功」）；
    ③ `PX_SPAWN_ISOLATE=0`（立即终止语义）修前**在 VM/px-coro 轨无效**，M120 起两条隔离路径都生效。
37. **编译轨实参类型不校验会段错误（M120 修 dict 五方法）**：`d.has(1)` 修前编译轨
    **SIGSEGV rc=139**、解释轨友好报 `R1002`；现编译轨同样 `R1002`。**同族**（`startswith` 等内建）
    尚未收口 —— 键/参数来自外部数据（`json_parse`）时先用 `type()` 判型。

38. **JSON 序列化有两个原语，别拿错（M129，qg-issue 87 缺陷 16）**：
    - `json_stringify(v)` —— **插入序**、不做 HTML 转义、控制字符**裸输出**（可产出非法 JSON）、浮点 `%g`（6 位有效数字，**会丢精度**）。
    - `json_stringify_go(v)` —— **对齐 Go `encoding/json`**：dict 键**递归按字节序排序**、`<` `>` `&` → `\u003c \u003e \u0026`、
      控制字符 → `\u00XX`、U+2028/9 → `\u2028/9`、非法 UTF-8 → `\ufffd`、浮点走**最短往返**、
      `bytes` 按 Go `[]byte` 出 **base64**。
      可选第二参：`json_stringify_go(v, {"sort_keys": false, "escape_html": false})` ——
      Go 侧 **map 排序 / 结构体保声明序**，而 PuXian 里二者都是 dict，故排序必须可关：
      遇到"map 里嵌结构体"的响应（如 `{"agents":[ExternalAgent…],"ok":true}`）用
      `{"sort_keys": false}` 并**手工按字母序 set 外层键**。
    凡"移植 Go 代码"或"要输出给人/机器对齐的 JSON"（响应体、落盘配置文件）一律用 `json_stringify_go`；
    `json_stringify` 保留给"内部紧凑编码、不关心键序"的场景。**两者不可混用比较**。
39. **`type()` 对字符串返回 `"string"`，不是 `"str"`**（M129 实测）：`type("a") == "string"`、
    `type([1]) == "list"`、`type(json_parse("{}")) == "dict"`、`type(1) == "int"`、`type(null) == "null"`、
    `type(true) == "bool"`。与 native 报错文案里的 `str` **不一致** —— 写类型分派时用错会**恒 false**
    （不报错，静默走错分支）。
40. **`sqlite_query` 的 dict 键 = SQLite 返回的**列名**（M129，qg-issue 87 缺陷 20）**：
    `SELECT COALESCE(base_url,'') FROM ...` 的键是字符串 `"COALESCE(base_url,'')"`，
    `r["base_url"]` 取不到（返回前先 `r.keys()` 看一眼最省事）。
    移植 Go 的 `rows.Scan`（**按位置**取值）时**必须写 `AS` 别名**，否则字段恒空 —— 且写库路径会把空值写回去（静默清库）。
41. **闭包捕获：轨间语义分叉（M129，qg-issue 87 缺陷 21）**：
    - **字节码 VM 轨（`px build` 默认）**：闭包为 M89-S3 **P1「无捕获」**版 —— 闭包体里引用**外层函数的参数/局部**
      会退化成全局名查找 ⇒ 运行期 `未定义变量` 或取到 null。**只能捕获模块级全局**。
    - **C 轨（`px build --c`）**：M129 起**真词法捕获**（upvalue cell，按引用）。
    - **解释器轨（`px run`）**：env 链，一直是真捕获。
    ⇒ 写**需要捕获外层局部**的回调（`with_db(cfg, fn(db): …)` 这类）时，字节码轨会报"未定义变量"。
    规避写法（P2 落地前）：把事务体**内联展开**，或把所需值作为**显式参数**传入；
    同时注意 `for` 循环里创建的闭包在解释轨是**每次迭代新绑定**。
42. **`os_capture`/`os_spawn_capture` 的 opts**（M129，缺陷 7）：`{"stdin_data": "…"|bytes(), "stdin": "/path", "cwd": "/tmp", "env": {...}, "group": true, "timeout_ms": 3000}`；
    `os_capture` → `{rc, stdout, stderr}`（分离），`os_spawn_capture` → `[rc, output]`（合并）。
    ⚠️ `os_popen` 的 stderr **未重定向**（会漏进宿主 stderr，缺陷 14 待修）。


43. **「行尾运算符续行」（M129，qg-issue 87 缺陷 33）**：Go 风格的长表达式**可以**这样写 ——
    ```px
    var s = "a" +
            "b"
    var ok = x == "ab" and
             y == 3
    ```
    规则 = Go 的自动分号插入：**上一 token 是双目运算符**（`+ - * / // % ** == != < > <= >=
    and or & | ^ << >> >>> ?? |> = += …` 以及 `.` `?.` `=>` `,`）时，换行**不是**语句分隔符，
    且下一行的缩进**不参与缩进栈**（等价括号内续行）。
    ⚠️ **仍是错误**的写法：把运算符放到**下一行行首**（`a` 换行 `+ b`）—— Go 也不接受；
    `not` 是一元前缀，不在续行表内；`:` 是块头终止符，**绝不**参与续行（否则吞掉所有缩进块）。
44. **`json_parse_opt(s) → Ok(v) | Err(msg)`（M129，缺陷 38）**：**解析外部 JSON 必须用它**。
    裸 `json_parse` 遇到畸形输入会 `px_error` —— 在 `http_serve` 里表现为 **500**
    （而 Go 的 `json.Unmarshal` 只是返回 error，由调用方按零值处理，这是 HTTP handler 的常态）。
    失败**不打印、不退出、不污染 stderr**；不支持嵌套调用（捕获点只有一个）。
    配套 stdlib 便捷函数：`px/util.px::json_parse_safe(s)`（失败返回 `null`）。
45. **`hex_to_bytes("")` 返回零长度 `bytes`（M129，缺陷 34）**：与 Go `hex.DecodeString("")` 一致。
    此前返回 `null` ⇒ `bytes_len` 直接 R1002 杀进程。顺带登记**宽容差异**（缺陷 36）：
    本实现会先剥掉空白字符再解码，Go 对空白**报错**。
46. **`null` 字符串化为 `"null"`（M129，缺陷 35）**：`str(null)` 与 `val_cstr(null)`
    （`sha256`/`base64_encode`/`hex_to_bytes` 等 native 的入参转换）现已一致。
    此前 `sha256(null)` = `sha256("0.0")` —— **静默错误结果**。仍建议：**别给 native 传 null**。
47. **`def main(): …` 会被运行时自动调用（缺陷 37）**：**不要**在文件末尾再写 `main()` ——
    会**静默执行两遍**（Go 移植里 `func main(){…}` + 顶层 `main()` 是高频写法，务必删掉后者）。
48. **`bytes` 可以直接当 HTTP 响应体（M129）**：`http_serve` 的 handler 返回
    `{"status":…, "body": <bytes>, "headers":…}`，`runtime_route.c` 支持 `PX_BYTES`。
    ⚠️ **M129 二次修订（缺陷 49）**：`http_serve(port, fn)` 这条最常用的路径走的是
    `runtime.c::px_http_build_response`，而它**原先没有 `PX_BYTES` 分支** ⇒ 返回 bytes 体时
    **响应体恒为空**（`Content-Length: 0`），状态码/其它头一切正常、**无任何报错**。
    同族另外两处（`runtime_route.c::route_normalize`、`runtime.c::px_vhost_normalize`）本来就有。
    现已补齐（顶层 + dict 两处），并加最小复现断言。**凡二进制响应必跑一次端到端 curl 自查**。
    返回**二进制**（zip/图片/音视频）**必须**用 `bytes`：走 `str` 会因「str 不能承载内嵌 NUL」
    在第一个 `0x00` 处**静默截断**。
49. **stdlib `url.px`（M129，缺陷 32）**：`url_query_escape` / `url_values_encode`（键字节序排序）/
    `url_query_unescape` / `url_parse`（`path` 为**解码后**值，`+` 不当空格）。native 表里
    **没有**任何 URL 编码原语，签名类代码务必用本模块而非手搓。
50. **`fn` 是保留字（缺陷 4 补充）**：`fn` 不能当变量名/循环变量/参数名（`for fn in …` → E2001）。
    另注意 `t`、`s` 这类短名极易**遮蔽**项目里的同名函数（如 i18n 的 `t()`）—— 遮蔽后调用会 500。

51. **`int(str)` 是近似 `strtoll`，不报错（M130，qg-issue 87 缺陷 60）**：
    `int("e") == 0`、`int("a") == 0`、`int("") == 0`、**`int("12ab") == 12`** ——
    解析到第一个非数字字符即停，**无数字前缀返回 0 且不报错**。
    ⚠️ 与 Go `strconv.Atoi`（报错）**不同**；且**不要**用 `int(c) - int("0")` 做字符→数字：
    十六进制逐位转换的惯用写法 `int(c) - int("a") + 10` 会把 `e/f/d` **一律算成 10**
    （应 14/15/13）= 静默错值（`stdlib/yaml_lex.px::yl_hexv` 曾因此把 YAML `\uXXXX`/`\xNN`
    解错，只要含字母 a-f）。**正确姿势 = 查表**：`index_of("0123456789abcdef", to_lower(c))`。
    （速查包旧文与若干移植注释曾把 `int()` 记为"畸形输入报运行时错误"—— 与实测相反，已更正。）

52. **文件锁 / 权限 / `open` 原始 flags（M130，缺陷 56/57/58）**：
    - `flock(fd, op)` → `0` 成功 / `-1` + `os_errno()` 失败；
      op = `LOCK_SH=1 / LOCK_EX=2 / LOCK_NB=4 / LOCK_UN=8`（可按位或，如 `2|4`）。
      **BSD 锁语义**：进程退出或关闭 fd 即释放 ⇒ 无 stale 锁文件（PID 锁的正确基础）。
      对照：`fcntl(fd, cmd[, arg])` 的 arg **只收 int/bool**，**传不了 `struct flock*`**，
      POSIX 记录锁（`F_SETLK`）**表达不了** ⇒ 要锁文件用 `flock`。
    - `chmod(path, mode)` → bool（= Go `os.Chmod`）；失败 `false` + `os_errno()`。
    - `open(path, flags_int[, perm])`：**原始 flags** 形态（新增）。
      `open(p, 0o100|2, 0o644)` 即 Go `os.OpenFile(p, O_CREATE|O_RDWR, 0644)`（**不截断**）——
      字符串形态（`r/w/a/rw/w+`）表达不了"有则开、无则建、不截断"（`w+` 带 `O_TRUNC`），
      也表达不了 `O_EXCL`。字符串形态**行为零变化**。
      ⚠️ 第三参只在第二参是 int 时可用。
## 2. native 内置速查（352 全量见 `docs/native_index.json`，本表为常用）

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
`now()`（**本地时间字符串** `YYYY-MM-DD HH:MM:SS`）`now_ms()` `now_us()` `now_sec()`（M115：Unix 秒，配 `time_format`）**`now_ns()`**（M141：**墙钟** Unix **纳秒** = `sec*1e9+nsec`；⚠️ `now_ms`/`now_us` 是 **CLOCK_MONOTONIC**（自 boot 起算），**`now_ns` 才是墙钟** —— 移植 Go 的 `time.Now().UnixNano()` / `Format("...000000000")` 必须用它，拿 `now_us()*1000` 冒充得到的是完全不同的值与数量级）**`sleep(ms)`**（⚠️ M128 实测更正：参数是**毫秒**不是秒！runtime `bi_sleep` 按 ms 换算；`sleep(1)`=1ms、`sleep(1500)`=1500ms；且 `int_val` **截断小数** ⇒ `sleep(0.5)` 等于不睡。移植 Go `time.Sleep(30*time.Second)` 若写成 `sleep(30)` 会少睡 1000 倍）`sleep_us`（微秒） `time_format(t, fmt)` `time_parse` `tz_offset` · `set_timeout(f, ms, ...)` `set_interval` `clear_timer` · `cron("分 时 日 月 周", f)`（6 字段）

### HTTP（客户端/服务端）
客户端：`http_get(url)` `http_post(url, body[, headers])` ·
**`http_request(url, method[, body[, headers[, opts]]])`**（⚠️ **url 在前**；opts = `{timeout_ms, retries, proxy}`，
timeout_ms **含连接阶段**，默认 30000；`http_get`/`http_post` 无 opts 入口时同样受这 30s 上限约束）· **`sse_connect(url[, opts])`（M118 起支持 Unix socket + POST）**：`opts = {reconnect_ms, sock, method, body, headers, content_type}` —— `sock` 非空则走 AF_UNIX（`url` 只作请求路径），配 `method`/`body`/`headers` 即可表达「POST + JSON 体 + 鉴权头的 SSE 流」（LLM 补全的标准形状；M118 前只有 GET/TCP，这条通路无法表达）· `http_get_stream` · 服务端：`http_serve(port, handler)`（TCP 每请求回调）· `http_serve_unix(sock_path, handler)`（**Unix socket 服务端**，M82；自动清残留 + 0600）· `px_serve(port, docroot[, tls, opts])`（静态 + .px 应用服务器，opts 可 {http3:true, max_body_size, rate_limit...}）· `px_exec`（语言内嵌 .px）· `http_unix(sock, path, ...)`（Unix socket 客户端，M56）
> **M116：handler/middleware 内抛运行时错误 → 客户端收 `500`（含说明 body），服务继续可用**（此前 VM 轨收 204 静默成功、C 轨整台服务器退出，见 §1.1 事实 19）。

### WebSocket / SSE
`ws_serve(port, onmsg)` `ws_connect(url)` `ws_send` `ws_recv` `ws_close` `ws_ping` `ws_heartbeat(conn, ms, cb)` `ws_broadcast(server, msg)` `ws_connect_auto(url, ...)` · SSE：`sse_serve(port, cb)` `sse_send` `sse_close` `sse_connect(url)` `sse_read` · **M83-S6 同端口流式（http_serve/http_serve_unix）**：`http_stream(path, on_connect)` 把同端口某 path 注册为流式 SSE（on_connect(req) 内 `sse_send(req["conn"], chunk)` 逐块推、可 `sse_send` dict {event,data,id,retry}，on_connect 返回自动关闭；普通 JSON handler 同端口共存，流式路由优先；明文 HTTP/HTTP-over-unix，px_serve 面暂不接入）

### TCP / UDP / TLS
`tcp_listen/accept/connect/send/recv/close` · **`tcp_connect_ex` `tcp_opt` `tcp_recv_ex` `tcp_send_ex`（M149：带超时 + 失败不杀进程 + EOF/超时可辨别；新代码优先用这一族）** · `udp_open/send/recv/close` `udp_serve(port, cb)` · TLS：`tls_server(cert, key[, hostname])`（注册后 px_serve/WS/SSE 支持 HTTPS/WSS/TLS）

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
`os_pid()` · `os_exec(cmd, args)`（替换进程）· `os_spawn(cmd, args[, group|opts])` / `os_spawn_capture(cmd, args[, opts])`（group=true 子进程 setpgid 自成组，M83-S2）· `os_wait(pid)` · `os_kill(pid[, sig[, group]])`（group 组杀）· `os_capture(cmd, args[, opts])`（双管道**分离**捕获 stdout/stderr）· `os_popen(cmd, args)`（双向）· `os_rename` `os_remove_all`（防删根）· `os_random_hex(n)` `os_file_sha256(path)` · `unix_connect(sockpath)` · `signal(sig, fn)` · `gc()`
> **M115 服务进程/环境补全**（ws-center / ws-ddns PuXian 化实测缺口）：`env_set(name, value)` / `env_unset(name)`（进程环境**可写**，子进程可继承）· `os_self_path()`（当前可执行文件绝对路径，守护化/自升级用）· `isatty(fd)`（TTY 判定，交互式提示只在终端弹）· `now_sec()`（Unix 秒，与 `time_format` 同轴 —— `now()` 是**本地时间字符串**，喂 `time_format` 会类型报错）。
> **`os_spawn` opts dict（M115）**：`os_spawn(cmd, args, {"group":true, "setsid":true, "stdout":"/log/x.log", "stderr":"…", "stdin":false, "cwd":"/tmp", "env":{"K":"V"}})`（⚠️ M128 实测更正：键**必须带引号** —— `{k: v}` 的键是**表达式**，`{group:true}` 会当成变量 `group` 求值 ⇒ `R1001 未定义变量`；非字符串键 ⇒ `R1002 字典键必须是字符串`。旧文档示例是无引号写法，照抄必踩。另：`stdin` 只接受**路径**，不能直接喂字符串数据） —— 守护化不再需要 `/bin/sh -c "setsid … >>log 2>&1 &"`（旧法拿不到真实 pid）。`stdin:false` = `/dev/null`；`opts.env` **不污染父进程**；第 3 参传 bool 时语义不变（零回归）。
>
> **`os_capture` / `os_spawn_capture` opts dict（M129）**：`{"stdin_data": "…"|bytes(), "stdin": "/path", "cwd": "/tmp", "env": {"K":"V"}, "group": true, "timeout_ms": 3000}`
> —— 补齐 Go 侧最常见的三种子进程形状（原先语言里各缺一角，`qg-issue 87` 缺陷 7）：
> - **`"stdin_data"`：把内存里的数据喂给子进程 stdin**（建管道写入，写完即关 = EOF；空串也建管道 ⇒ 子进程立刻 EOF）。这是 `cmd.Stdin = bytes.NewReader(s)` 的等价物 —— 旧文档只有 `stdin` **路径**，无法表达"参数走 stdin"的工具协议。与大输出并发时用 **poll 双工**驱动（既写 stdin 又读 stdout），**不会互锁**。
> - **`"timeout_ms"`：超时 SIGKILL 整组**（需配 `"group": true`；工具常自己再 fork，只杀父会留孤儿）。超时后 rc = **137**（=128+SIGKILL），并**继续读尽残余输出**（不是丢弃）。
> - `"env"` 与 `environ` **合并**（同名覆盖、其余继承）⇒ 等价 Go `os.Environ()` + `append`；`"cwd"` 同 `os_spawn`。
> - 区别只在输出形状：`os_capture` → `{rc, stdout, stderr}`（分离，= Go `cmd.Output()`）；`os_spawn_capture` → `[rc, output]`（**合并**，= Go `cmd.CombinedOutput()`）。
> - ⚠️ `os_popen` 的 stderr **未重定向**（会落进宿主进程的 stderr）—— 要"只要 stdout"请用 `os_capture`（已知缺陷，见 `qg-issue 87` 缺陷 14）。
>
> **HTTP 状态行 reason（M129 补全）**：runtime 的状态码→reason 表已**全表对齐 Go `net/http`**（含 307 `Temporary Redirect`、308、4xx/5xx 全量）。此前只有 22 项且未知码回退 `"OK"` ⇒ 307 会被写成 `HTTP/1.1 307 OK`。未登记码现按 Go 的字面值输出 `status code NNN`（**不再伪装成 OK**）。

### fd / 边缘设备（Linux）
`open(path, flags[, perm])`（M130：第二参给 **int** 即原始 flags，如 `64|2` = `O_CREAT|O_RDWR`；字符串形态 `r/w/a/rw/w+` 不变）`close(fd)` `read(fd, n)` `write(fd, data)` `ioctl(fd, req[, arg])` `os_errno()` · **`flock(fd, op)`（M130）** · **`chmod(path, mode)`（M130）** · `mmap/munmap/mem_write`（活映射）· `fcntl` `tty_config` `fd_wait`（poll）· GPIO/I2C/串口/PWM 走 `import std.edge`

### QUIC / HTTP/3（完整编译含 64 项；`--no-quic` 裁剪不含）
`quic_listen/accept/connect/close/close_listener` · `quic_open_stream/open_uni_stream/send_stream/recv_stream/poll` · `h3_server_listen` `h3_serve_read_request(_stream)` `h3_client_*` · QPACK：`h3_huff/unhuff` `h3_qenc/qdec/qs_*` `h3_settings_enc/dec` `h3_conn_*` —— 生产路径推荐直接 `px_serve(..., {http3: true})`（HTTP/1.1+2+3 三栈合一）。

## 3. 标准库速查（27 库，纯语言 .px，双模式一致；下表为常用）

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
| go_json 族 | `import std.go_json_indent` | **Go `encoding/json` 保真族**（M142）：`go_json_widen_numbers(v)`（int→float 的 interface{} 语义）· `go_json_indent(src, prefix, indent)` → str\|null（`json.Indent` 逐字节）· `go_json_indent_ex` → {ok,out,err,offset} · `go_json_compact(src, escape)` → str\|null（`json.Compact` / `Marshal` 转义路径）· `go_json_htmescape(src)` → str · scanner 层 `gjs_new/gjs_step/gjs_eof/gjs_byte0/gjs_hexd/gjs_quote_char` |

> 完整 API 文档：`tools/px doc stdlib/<name>.px`；用法示例见 `docs/ECOSYSTEM.md §2`。

## 4. 高频模式（可直接抄）

### 4.1 HTTP 服务端（http_serve 回调式 · 编译模式运行）

```px
# ⚠️ http_serve 常驻服务 + spawn → 用 `px build`（pxi Mini 子集无 spawn）
def handler(req):
    # req: {method, path, query, headers, body, form, files, file_fields, ...}（dict）
    #   form       —— 非文件段的「字段名 → 值」
    #   files      —— 文件段的「**文件名** → 内容」（历史语义，保持不变）
    #   file_fields—— 文件段的「**文件名 → 字段名**」（M129 新增，qg-issue 87 缺陷 10）
    #     ⇒ Go `r.FormFile("avatar")`（按**字段名**取件）的等价物：
    #       `for fn in req["files"]: if req["file_fields"][fn] == "avatar": data = req["files"][fn]`
    #       此前 files 只以文件名为键，多文件且字段名不同时**无法区分**（单文件只能"取唯一项"近似）。
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

- **native 清单**（345，单一事实源 = runtime 注册表）：`bash tools/gen_native_table.sh` → `docs/native_index.json`；CI 重跑 diff 防漂移。**本表计数必须 == count**（现 345）。
- **stdlib 索引**：`tools/px run tools/gen_ecosystem.px` → `docs/ecosystem_index.json`。
- 规范：`docs/spec.md`（§8 模块/import、§9 双模式、§12 AI 协议）· `docs/MINI_SUBSET.md`（子集边界）· 缺口与写库规范：`docs/ECOSYSTEM_GAPS.md`。

---

## 移植 Go 代码的**静默漂移**清单（M129 第 7 轮补齐，逐条实测）

51. **`/` 是浮点除法，`//` 才是整除（缺陷 42）**：`16640 / 3600` → `4.622222222222222`（float），
    `16640 // 3600` → `4`。Go 的 `int/int` 是整除 ⇒ 移植 `estTokens := totalBytes / 2`、
    `h := h * maxSize / w`、`sx := x * srcW / newW` 这类表达式**必须写 `//`**。
    症状：结果变 float ⇒ `str()` 打出 `"8.0"`（不是 `"8"`）、比较/切片下标全歪，**不报错**。
52. **`len(s)` / `s[i:j]` / `s[i]` 是「字符（rune）」语义，Go 是「字节」（缺陷 45）**：
    `len("中文ab") == 4`（Go 8）、`"中x"` 的 `index_of(x) == 1`（Go 3）、`s[0:3] == "中文a"`。
    移植 `s[:n]` 截断、`len(s) > N` 长度校验、`strings.Index` 后切片时，**凡涉及非 ASCII 必分叉**。
    ⇒ 用 `stdlib/go_strings.px`：`byte_len` / `go_slice` / `go_prefix` / `go_truncate` / `go_index`。
53. **`bytes == bytes` 按内容比较，但解释器（`px run`）曾恒 false（缺陷 43）**：解释器 `i_eq` 是
    白名单实现、`bytes` 落到末尾 `return false`；编译轨按 memcmp。已修（两轨一致）。
    历史影响：所有 `px run` 下的自检脚本里 `bytes == bytes` 恒否 —— 静默跳过断言分支。
54. **字符串→数字转换对畸形输入是「运行时错误」（缺陷 47）**：`int("12ab")` 杀进程（http_serve 下 500），
    而 Go 的 `strconv.Atoi` 只返回 err。同理 `hex_to_bytes("zz")`。⇒ 先判 `is_hex(s)`，
    或用 `stdlib/go_strings.px::go_parse_i64(s) → Ok/Err`。
55. **`is_dir(path)`（缺陷 44）**：Go 的 `fi.IsDir()` 等价物。`exists("/tmp")` 对目录返回 **true**
    （与文件不可区分）、`file_size("/tmp")` 返回目录项大小。`read_file_opt(dir)` 虽报 EISDIR，
    但 EACCES 同形 ⇒ 分不出。
56. **`stdlib/time_go.px`（缺陷 41）**：Go 参考布局（`time.RFC3339` / `"2006-01-02 15:04:05"` …）。
    ⚠️ 别用 strftime 手搓 RFC3339：`time_format(..., "%z")` 给的是 `+0800`（**无冒号**）、
    `%Z` 给 `UTC`，而 Go 是 `+08:00` 且**偏移为 0 时输出 `Z`**。用 `go_rfc3339(ts)` / `go_format(ts, layout)`。
57. **`img_scale` 的滤波（缺陷 39）**：默认双线性；`{"filter":"nearest"}` 才是 Go 手写缩放
    （`x*srcW/newW` 整数除法 + 取样）的等价物。实测 6/6 组尺寸 RGBA 逐字节一致。
58. **multipart 文件内容：`req["file_bytes"]` 才是二进制安全的（缺陷 50）**：
    `req["files"]` 的值是 **str**，**遇首个 `\x00` 静默截断**（实测 8 字节 PNG 魔数+NUL → 7~8 字节，
    `type()` 仍是 "string"）。凡二进制上传（图片/附件/S3）一律用 `file_bytes`；
    `file_fields` 给「文件名 → 字段名」（缺陷 10），二者配合才等价 Go 的 `r.FormFile("name")`。
59. **`json_stringify_go` 的 opts 是 TLS（缺陷 51）**：`{"sort_keys": false}` 曾经是**进程全局**，
    在 `http_serve`（多线程）+ `spawn` 并发下会互相污染（响应键序偶发错乱、无报错）。现已 `__thread`。
    写代码时仍建议：**同一线程内序列化前不要依赖跨调用的 opts 状态**。
60. **「未注册路径」在 Go 是 404（先于认证），不是 401（缺陷 48）**：`http.ServeMux` 无匹配即
    `404 page not found`，任何认证包装都没机会跑。移植时若把认证判定内联进 dispatch，
    未注册路径会落到认证分支 ⇒ 状态码与 Go 相反。
61. **`tz_offset(tz)` 现在接受 `"local"`（缺陷 40）**：与 `time_format(ts, fmt, "local")` 同轴
    （此前 `tz_offset("local")` 返回 `null`，一个入口有一个没有）。
62. **保留字清单再补**：`fn`（缺陷 4）、`from`（缺陷 12）、**`pub`**（第 7 轮实测：
    `def f(pub):` → `E2001 意外的 token: pub`）。参数名/循环变量避开这三个。
63. **不支持单行 `if cond: stmt`（第 10 轮实测 · 缺陷 63）**：Python 习惯的
    `if n == 2: return "x"` 会被解析器拒（`E2001 期望 换行，实际得到 return`）——
    `:` 后**必须换行 + 缩进块**。同一函数里混写（有的单行有的多行）时，报错位置会落在
    **入口文件**的某个无关行号上（本例报 `main.px:100`，真凶在 `token_cache.px`），**看行号会误导**。
64. **`index_of`/`last_index_of` 是「字节」下标，而 `s[i:j]` 是「rune」切片（第 10 轮实测）**：
    这是缺陷 45 的**组合陷阱** —— 串里只要有中文，`s[last_index_of(s,"(") : ...]` 取到的是错位片段
    （不报错、静默取错值）。要定位/切分含多字节字符的串，**一律用 `split`**（rune 安全）。
65. **`http_stream(path, cb)` 只在 GET 上接管（第 10 轮实测）**：runtime 的分流条件是
    `if (strcmp(method, "GET") == 0)` 才查流式路由表 ⇒ **POST 的 SSE 端点无法走流式接管**
    （Go 的 `handleStream` 正是 POST）。POST 只能走普通 handler 一次性返回，**失去逐块 flush**。
66. **`http.Error` 会给响应补 `X-Content-Type-Options: nosniff`（第 10 轮实测）**：
    移植 `http.Error(w, msg, code)` 时除了 `text/plain; charset=utf-8` + 尾缀 `\n`，
    还要**同时补 nosniff 头**，否则逐字节不符（404/405 一律如此）。
67. **Go 里「裸 `json.NewEncoder(w).Encode(...)`、从不 Set Content-Type」的端点，
    Content-Type 是**嗅探**出来的 `text/plain; charset=utf-8`（第 10 轮实测）**：
    `handleToolsExec` / `handleTaskNotify` 即此类。移植时**不要顺手补 `application/json`**
    （用 `json_res_sniffed`）；判据很简单：**Go 源里有没有 `w.Header().Set("Content-Type", ...)`**。
68. **map 键序不是「看着像字母序」就算**：`server` 块里 `tls` < `token_cache`
    （'l' < 'o'，第 10 轮实测照出/修正）；写死插入序时**务必按键的 ASCII 逐字比较**，
    别凭语感（`token_cache` 会被误以为在 `tls` 前面）。
69. **`http_stream` 现在支持 POST / 自定义响应头 / 手写响应头（第 11 轮 · 缺陷 65 已修）**：
    第三参 opts（全部可选）—— `{"methods": ["POST", ...]}`（默认 `["GET"]`，**按方法掩码**命中，
    未列出的方法落普通 dispatch）、`{"headers": {...}}`（按名大小写不敏感**覆写**默认 SSE 三头，
    未消费的按原序追加）、`{"manual": true}`（接管时**不写任何响应头**）。
    配套两个 native：**`sse_start(conn, status, headers)`**（语言层自写响应头；headers 里若声明
    `Transfer-Encoding: chunked`，本连接后续写出自动分块，收尾自动补 `0\r\n\r\n`）、
    **`sse_write(conn, data)`**（**原文**写出，不做 SSE 分帧；`sse_send` 会按 SSE 规则加 `data: ` 前缀）。
    ⚠️ 为什么需要 manual：Go 的 handler 是「**先校验、后 Set 头**」——400/405 必须在响应头发出**之前**
    返回普通 HTTP 响应；auto 模式会先写 200 + SSE 头，之后就回不了头了。
70. **`sse_read_line(conn)` —— 上游 SSE 的**逐行原文**读取（第 11 轮新增）**：返回下一行
    （不含行尾 `\n`；行尾 `\r` 一并去掉），EOF 返回 `null`。语义 = Go 的 `bufio.Scanner` 循环
    （`line := scanner.Text(); fmt.Fprintf(w, "%s\n", line); flusher.Flush()`）。
    **透传上游 SSE 时不要用 `sse_read`**：它把事件解析成 dict 再重新分帧，会多出 `event:` 行、
    打乱字段序、丢掉注释行 —— 关心语义用 `sse_read`，关心字节用 `sse_read_line`。
    本函数**不做**断线重连（透传语义下重连会把内容重发一遍）。
71. **SSE 客户端原先**没有**解码 chunked**（第 11 轮实测 · 缺陷 69 已修）**：上游用
    `Transfer-Encoding: chunked`（Go 的 SSE handler「写头 + 边写边 Flush」必然如此）时，
    分块帧会原样进缓冲 —— `sse_read` 的 `\n\r\n` 事件分隔**恰好**把块长行当成"无冒号行"丢掉，
    于是**只在块边界与事件边界重合时侥幸可用**；块边界落在 data 行中间时内容被块长行污染，
    且终结块处会多出空事件（实测：4 块 → 多 2 个空事件）。现在 `sse_read` / `sse_read_line`
    共用**唯一**解码点（RFC 7230 §4.1），三种切法（1 字节/块、13 字节/块、不分块）输出逐字节一致。
72. **`sse_close` / `sse_send` / `sse_write` 的 conn 是两套 id（第 11 轮实测 · 缺陷 70 已修）**：
    服务端连接（`sse_serve` / `http_stream` 接管）与客户端连接（`sse_connect`）**原先各自从 1 自增**，
    命名空间重合；而 `sse_close(conn)` **先查服务端表、再查客户端表** ⇒ 同进程里
    「`sse_close(上游客户端 conn)`」在 id 撞上**下游服务端 conn** 时**关掉的是自己的下游连接**
    （症状：响应缺收尾块、连接提前关，且只在 id 相同时复现，极难定位）。
    现已把客户端 id 起点移到 `1<<40`（不相交区间）。**同进程同时持两类连接时务必注意这条**。
73. **`file_stat(path)` —— 文件元信息（第 12 轮 · 缺陷 71 已修）**：返回
    `{size, mtime, mtime_ns, is_dir, mode} | null`（**失败返回 null，不杀进程**；跟随符号链接）。
    ⚠️ 此前只有 `file_size`（失败**px_error 杀进程**）与 `is_dir`，**没有 mtime** ——
    Go `os.Stat()` 的「先看存在性、再看年龄」这个常态写法（缓存/快照过期判定）表达不出。
74. **`go_errno_string(errno)` —— errno → 文案（第 12 轮 · 缺陷 72 已修）**：返回 **Go**
    `syscall.Errno.Error()` 的文案（**小写**、表驱动、表外回落 `errno N`）。
    ⚠️ **不要用 libc `strerror`**：glibc 给 `"No such file or directory"`，Go 给
    `"no such file or directory"` —— 移植 `*PathError.Error()`（`stat <p>: <errno>`）必然分叉。
    表由 `tools/gen_go_errno_table.go` 从 Go 自身导出（勿手抄）。
75. **`ord(s)` / `chr(n)` —— 码点 ↔ 字符（第 12 轮 · 缺陷 73 已修 · 落实缺陷 29）**：
    `ord` 取**首字符**码点（空串 0；非法 UTF-8 按 Go `utf8.DecodeRune` 给 `0xFFFD`）；
    `chr` 非法码点（<0 / 代理区 / >0x10FFFF）→ `U+FFFD`。**项目内不应再手写 UTF-8 解码**。
76. **`regex_valid(pattern) → bool` —— 只编译不匹配（第 12 轮 · 缺陷 74 已修）**：
    ⚠️ native 表里的 `regex_match/regex_find/...` 对坏正则一律 **`px_error`（杀进程）**，
    Go 侧常态的「探测合法性」（`if _, err := regexp.Compile(p); err != nil`）**表达不出**。
    探测用 `regex_valid`，别用 `regex_match(p, "")` 去试。
77. **`url.Parse` 的文案面（第 12 轮 · 缺陷 75 已修，见 `token-cache/px/go_urlerr.px`）**：
    ① `Parse` = 「先切 `#frag` → `parse(主体)` → `setFragment(frag)`」，两段失败时
    **`parse "<串>"` 里引用的串不同**（前者是去片段的主体，后者是完整原始串）；
    ② host 段的 `%XX` 只允许**解码后非 ASCII**（`%25` 例外）⇒ `http://a%41.com/` 报错、
    `http://x/%41` 合法（path 段只查十六进制）；
    ③ **端口取最后一个冒号**（Go ≤1.25；Go 1.26 的 GODEBUG `urlstrictcolons` 按模块
    `go` 版本门控 —— 本仓库 `go 1.25.0` ⇒ 用旧行为）；
    ④ query/fragment/userinfo **不做转义校验**（`?q=%zz` 合法）。
78. **顶层 `def` 没有模块命名空间：跨文件重名 = 静默覆盖（第 12 轮 · 缺陷 77 已修+加门）**：
    ⚠️ 多文件合入同一作用域，**后 import 的赢**，且**无任何告警**。实测事故：新增的
    `stats_inc` 撞了 `client.px` 里带守卫的同名函数 ⇒ `/stats` 的 `sets` 行为随 import 顺序变化。
    **写新文件前先跑 `python3 px/tools/symcheck.py main.px`**（门已纳入常驻集）。
79. **`open` 的三参形态两轨同形（第 12 轮 · 缺陷 78 已修）**：`open(path, flags_int[, perm])`
    在**解释轨**同样可用（`px run` 与编译产物行为一致；此前解释轨只收 1–2 参而直接 `R1002`）。
    另记：**解释器不支持 `mutex`（Mini 子集排除，缺陷 79 未修）** ⇒ 像 token-cache 这种
    "启动即建 mutex"的程序**跑不了 `px run`**，验证请用编译轨。

80. **`http_unix` 的第 6 参 `opts.timeout_ms` —— 总时限（第 13 轮 · 缺陷 80 已修）**：
    `http_unix(sock, path, method[, body[, headers[, opts]]]])`，`opts = {"timeout_ms": N}`。
    `N > 0` 时**连接阶段**也受该时限（非阻塞 connect + poll），收发用 `SO_RCVTIMEO/SNDTIMEO`。
    ⚠️ 修前 RCVTIMEO **固定 180s** ⇒ 对"accept 了但不回包"的对端要挂 180s；
    Go 的 `http.Client{Timeout}`（总时限）在这条通路上**表达不出**（不是慢，是语义缺失）。
    ⚠️ 边界：这是"**每次 IO** 的上限"，Go 是"**整个请求**的上限" —— 对端匀速滴数据时二者不同。
    门：`examples/m133_http_unix_timeout/`（VM+C 双轨 · 含"不同 timeout 必须成比例"的负控）。
81. **网络原语失败后的 `os_errno()` 有了语义（第 13 轮 · 缺陷 81 已修）**：
    `http_unix` / `http_request` 失败时把**成因**留在 errno 上（修前被之后的 `close()` 覆盖）：
    `0` = 对端干净关闭（EOF）· `11`(EAGAIN) = 读超时 · `111`(ECONNREFUSED) = 连不上 ·
    `2`(ENOENT) = 路径不存在 · `104`/`32` = 对端 reset / broken pipe · `90` = 响应头过大。
    用途：区分「连不上」（可重试）与「超时/对端断开」（重试会放大对端 CPU）——
    token-cache 审批重试策略的分类重试**正是**靠它（`px/approval_retry.px`）。
82. **import 关系必须与实际依赖一致：跨文件的"顺手用"会在**单模块导入**时炸（第 13 轮 · 缺陷 82 已修）**：
    PuXian 顶层符号**无命名空间**（见第 78 条），于是"某个文件定义了 X"就够——只要**有人**
    把它 import 进来。`approval_cfg.px::expand_ws_path` 用了 `expand_env`（定义在 `config.px`），
    但 `approval_cfg.px` 只 import 了 `config_fields.px`；主程序 main.px 恰好 import 了 config.px
    ⇒ 一直"看起来正常"，**单独 import 审批子系统**（探针）时报 `未定义变量: expand_env`。
    修法：把 `expand_env` 迁到双方都 import 的 `config_fields.px`。
    ⇒ **写探针/子模块是检验隐式依赖的最省事手段**（探针自带那份 import 闭包）。
83. **`http_request` 成功时返回 `dict`，失败才返回 `Err(result)`（第 13 轮 · 缺陷 83 已修）**：
    ⚠️ 直接 `var r = http_request(...); if r.is_err():` 会在**成功**路径抛
    `R1007 类型 dict 没有方法 'is_err'` **把进程打挂**。必须先 `if type(r) == "result":` 再判。
    实测事故：告警上报**成功**时（ws-alertd 可达）把 token-cache 打挂 —— 越正常越先死。
    同类语义：`http_get`/`http_post`（成功 → str）、`/set`/`ssl` 等"成功值 + Err 失败"的 native。
84. **`\b` / `\B` 零宽断言：Go RE2 有，PuXian 正则引擎没有（第 13 轮 · 未修 · 已登记）**：
    `regex_valid("\\bsecret\\b") == false`（`rp_parse_escape` 的 default 分支 → "未知转义"）。
    影响面：**凡用 `regex_valid` 做"能编译就用、不能编译就整体禁用"的项目侧判定**，两侧会分叉。
    token-cache 本地快判：规则快照的 `secret_patterns` 里只要有一个 `\b…\b`，PuXian 就会把
    **整个本地快判禁用**（Go 保持启用）⇒ 全部改走远程（**fail-safe 方向**，但行为不同）。
    门：`px/tools/diff_approval.py::fastpath_wordboundary`（断言"分叉形状正确"而非假装全绿）。
    ⟹ 根治要在引擎里加零宽断言节点（含 M107/M110 前缀预筛路径的适配），留单独一轮。
85. **就地改写脚本必须断言"结束锚点已找到"（第 13 轮 · 我自己踩的坑）**：
    `j = s.find(marker); s2 = s[:i] + s[j+1:]` —— 当 `find` 返回 **-1** 时 `s[j+1:]` 就是
    `s[0:]`（整份文件），结果是**前半段 + 整份原文**（配置类文件会静默变成两份，
    编译期表现为顶层符号重名）。修法：`if j < 0: raise`；改完**必须**跑符号查重门
    （`symcheck.py` —— 这次正是它抓到的）。
86. ★ **「是否为堆对象」**绝不能是 switch 白名单 —— 漏项 = GC 误回收 = 悬垂（第 15 轮 · 缺陷 86 已修，P0）**：
    `runtime.c::px_value_is_obj()` 决定 **GC 根标记**（全局表 / 帧槽 / 容器递归 / TLS 根栈全走它）。
    它原来是 `switch (v.type)` 白名单，**漏了 `PX_MUTEX` / `PX_RWLOCK` / `PX_GEN`** ⇒
    这三种值放进 `list/dict/struct/tuple/gen/chan`，**或**作为全局/帧槽的**直接值**时都不被标记
    ⇒ 对象被 sweep 回收（悬垂）。
    实测症状（token-cache 生产链路，**最凶的一类：静默永久挂死**）：同一进程内第 6 个
    「走审批判定」的请求起，连接**被 accept 但 handler 永不执行**、客户端收 0 字节，此后全部挂住；
    `fds/线程数`都不变、无任何日志。gdb 栈：`__lll_lock_wait ← pthread_mutex_lock ← px_mutex_lock
    ← vm_run_loop` —— `pthread_mutex_lock` 阻塞在**已回收内存的 `__lock`** 上（`__lock=2` 但
    无持有者、无唤醒源）。同一把锁在 `px_coro_mutex_wait`/`px_mutex_try_lock` 上也一样。
    修法：改成 `static const bool g_type_is_obj[PX_TYPE_MAX]` **位置表**（逐项表态）
    ＋ `_Static_assert(sizeof(...) == PX_TYPE_MAX)` ⇒ **LXType 新增成员而不表态 = 编译失败**。
    同族历史缺陷 43（解释器 `i_eq` 白名单漏 `bytes` ⇒ `bytes == bytes` 恒 false）——
    ⇒ **纪律：凡"按 LXType 分类"的函数一律用位置表 + 尺寸断言，不用 switch 白名单。**
    回归门：`examples/m134_gc_obj_roots/`（17 断言 × VM/C 双轨 + 静态门 + 两个负控）。
87. **测"对象会不会被误回收"：把它放进容器/全局/帧，然后 `gc()`（第 15 轮）**：
    `gc()` 是 naitive，**强制整轮 mark+sweep**（不等阈值），比"造 10 万垃圾逼 GC"确定性高得多：
    ```
    var g = []
    g.append(mutex())
    var fmu = rwlock()        # 帧槽也要试
    gc()
    g[0].lock(); g[0].unlock()   # 悬垂 → 立即 abort 并打印 obj/type
    ```
    runtime 侧配套：`px_dbg_obj_check()`（`px_mutex_lock/try_lock/unlock`、
    `px_rwlock_*`、`px_coro_mutex_wait` 入口）——标签是锁但对象头 type 不是 ⇒ 打印
    `[FATAL-obj] 位置 obj=0x… type=…（期望 …）` 后 abort。**把"静默永久挂死"变成"立刻可见的崩溃"**。
88. **"卡死"类问题的 gdb 取证套路（第 15 轮实战，10 分钟定位）**：
    ```
    gdb -p <pid> -batch -ex 'thread apply all bt 14' > bt.txt
    # ① 按帧签名聚合（几百个线程秒看出谁是异常）：把每线程的 `in <fn>` 拼成签名计数
    # ② 只看异常线程：`thread N` + `frame 2` + `info registers rdi` → 取被锁对象地址
    # ③ 读对象内存：`x/6wx <addr>` → `__lock=2` + 其余字段是垃圾 ⇒ 悬垂/被复用；
    #    若 `__owner` 是真 TID，再按 TID 找持有者线程（才是真互斥等待）
    ```
    判别要点：**阻塞在 `pthread_mutex_lock` ≠ 等锁**；`futex` 的 `__lock` 词被写坏时，
    它是"等在垃圾上"，任何超时/重试都不会救回来 —— 这类必须先证明"对象还活着"。

89. **`str(x)` 对**含内嵌 NUL 的字符串**曾静默截断（第 16 轮 · 缺陷 92，已修）**：
    `PX_STR` 本身是**长度感知**的（`px_str_len` 带显式 len，`len()`/切片/JSON 转义都按长度走），
    只有 `str()` 这一跳走的是 `px_fmt_value()` → `char*` → 在首个 `0x00` 处断掉。
    ```
    var b = bytes("AB"); b = bytes_concat(b, int_to_bytes(0,1)); b = bytes_concat(b, bytes("CD"))
    byte_len(bytes_to_str(b))     # 5 ✓（本来就对）
    byte_len(str(bytes_to_str(b))) # 修前 2 ✗ / 修后 5 ✓
    ```
    ⇒ **纪律：二进制体不要经 `str()` 中转**；必须转就把长度一起带走（`bytes` / `px_str_len`）。
    HTTP 响应体若用 `str` 承载，运行时按 `.len` 写出（`px_http_build_response`），NUL 安全。
90. **JSON 解析的三个「Go 保真」坑（第 16 轮 · 缺陷 93/94/96，已修/已补）**：
    · **整数溢出**：`strtoll` 溢出返回 `LLONG_MAX` 且 `end` 越过全部数字 ⇒ 旧实现当"整数解析成功"，
      `12345678901234567890` 静默变成 `9223372036854775807`。修法：`errno==ERANGE` 时回落 `strtod`。
    · **`\u0000` 截断**：JSON 字符串解码出的 0x00 在重造时被 C 串截断（同 89）⇒ 解析侧改为长度感知。
    · **Go 的 `interface{}` 把数字全变 float64**：`json.Unmarshal` 到 `map[string]interface{}` 后
      Marshal，`9223372036854775807` → `9223372036854776000`、`9007199254740993` → `9007199254740992`。
      PuXian 的 `json_parse` **保留整数**（语言语义）⇒ 需要 Go 那一侧语义时必须显式加宽：
      ```px
      import std.go_json
      json_stringify_go(go_json_widen_numbers(json_parse(text)))
      ```
91. **浮点转字符串的「最短往返」要用数字串，不是精度扫描（第 16 轮 · 缺陷 95，已修）**：
    对 `1.2345678901234567e19`，`%.0f` 给的是**精确值** `12345678901234567168`，
    而 Go 的 `strconv.FormatFloat(f,'f',-1,64)` 是**最短往返数字**再按 'f' 展开 ⇒ `12345678901234567000`（差 168）。
    正确做法：先用 `%.*e` 递增精度扫出**最短数字串**（首个能 `strtod` 回读相等者），再按
    Go 的 `|x| ∈ [1e-6, 1e21)` ⇒ 'f' 否则 'e' 规则渲染该数字串（'e' 还要做 `e-0d` → `e-d` 收敛）。
    回归门：`examples/m136_go_json_fidelity/`（23 行语料与 **Go encoding/json 本尊**逐字节 diff + 24 断言 × VM/C 双轨 + 双负控）。
92. **HTTP 头的「多值」与「原始查询串」（第 16 轮 · 缺陷 88/90，已修）**：
    · 响应头 dict 的值可以是 **list**（同名多头，如 `Set-Cookie`）—— 修前 `px_http_build_response`
      与 `http_request` 的请求头拼接都把非 `str` 值 `continue` 掉 ⇒ **整条头静默消失**（透传上游时必踩）。
    · 服务端请求 dict 现在提供 **`req["raw_query"]`**（= Go `r.URL.RawQuery`，**不解码**）；
      旧字段 `req["query"]` 是**整体解码后**的串 ⇒ `?key=a%26b` 会被解码成 `key=a&b`，再按 `&` 切分**必然切错**。
      取值一律 `url_parse_query(req["raw_query"])`（`stdlib/url.px`，含 `+`→空格与非法转义的 Go 语义）。
    · 新增 `url_path_escape`（= Go `url.PathEscape`：`,` `;` `/` `?` `!` `'` `(` `)` `*` 都要转义）。
93. **`http_request` 的透明 gzip 解码可关（第 16 轮 · 缺陷 89，新增）**：
    `http_request(url, method, body, headers, {"decode_gzip": false})` ——
    Go 的 `Transport` **只在调用方未自带 `Accept-Encoding` 时**才自己加头并透明解压；
    调用方自带时 Go 原样透传压缩体。要复刻后者（如媒体/文件代理逐头逐体透传）就必须关掉本开关。
94. **`pass` 不是关键字 ⇒ 现已**编译期报错**（第 17 轮发现 · 第 19 轮根治 · 缺陷 101）**：
    `if cond:` 下写 `pass` 会被解析成"求值标识符 `pass`"的**表达式语句** ⇒ `px build` 通过、
    `px lint` 报 `E L002: 未定义变量: 'pass'`（**编译期唯一的线索**）、跑到那条分支才炸。
    空块请用合法空语句 **`0`**（实测：lint 0 错 0 警、编译通过、分支真空转）。
    实测现场：token-cache `llm_resp.px` 7 处 + `llm_msgs.px` 1 处写了 `pass`，上游 JSON 里
    出现任何 `null`（`"created": null` / `"tool_calls": null` / `"function": null` …）即整进程死。
    **第 19 轮根治（E2012）**：解析器对**裸标识符语句**直接报编译错误
    `E2012 裸标识符语句没有效果：<名>（调用需要括号 ()；若想要空语句请写 0）`，
    恰好叫 `pass` 的另给一句提示。规则只在**语句位置**生效（match/select 的 case 体是值，不受影响），
    裸字符串字面量语句仍允许（不是标识符）。
    · 为何不把 `pass` 关键字化：本仓库确有 `var pass = 0`（pass = 通过计数）作变量名，
      关键字化会**静默改变**这些程序的含义。
    · **同族的"沉默"**：`examples/*.px` 里 228 处 `assert <expr>`（无括号）此前被解析成**两条语句**
      （`assert` 空转 + 表达式无效果）⇒ 这些断言**从未执行**；改成 `assert(…)` 后立刻查出 3 处
      失效期望（`hex_to_bytes` 返 bytes 不是 str / base64 语料注释与实解不符 / `zh[::-1]` 少一个字）
      与 **1 处真 bug**（见 100）。门：`examples/m139_bare_ident_stmt/`。
    ⇒ **改了 .px 先跑 `px lint <file>`**：缺 import、缺定义（删函数漏删引用）仍由 lint 兜（L002）。
95. **`sse_connect_ex(url, opts)` —— SSE 客户端"失败可分类"版（第 17 轮 · 缺陷 97，新增 native）**：
    `→ dict{ok, conn, status, ctype, retry_after, body, errno, stage}`；`stage` = 1 参数 / 2 连接 /
    3 发送 / 4 读头 / 5 状态码 / 6 CT 不符 / 7 槽位耗尽。`opts={method, body, headers, sock,
    content_type, require_ct, timeout_ms, reconnect_ms}`。
    · `require_ct: false`（**ex 模式默认**）⇒ 200 即接通，**不管 Content-Type**
      （Go 的 `forwardStream` 就只看行）；`sse_connect` 保持旧语义（要求 `text/event-stream`）。
    · 非 200 时 `status`/`ctype`/`retry_after`/**`body`（已解码 chunked）** 都有值 ⇒
      可以像 Go 那样分流"可重试（429/5xx/408 → 换候选）"与"不可重试（400 → 原样报错）"。
    · `timeout_ms` ⇒ `SO_RCVTIMEO/SNDTIMEO`（**每次 IO** 上限；0=不设=永久阻塞，旧行为）。
    · `req_headers` 里已给的同名默认头（UA/Accept/Connection/Cache-Control）**不再重复发**。
96. **真流式代理的两个"必须照抄 Go"的点（第 17 轮）**：
    ① **分帧边界 = Flush 点，不是 write 点** —— 逐行 `sse_write` 会得到"每行一块"，
       与 Go 不同；正确做法是非 `data: ` 行**只攒不写**、到 `data: ` 行把攒下的字节
       **一起**写出，**流末**再刷一次（Go 在 handler 返回时刷 bufio）。
    ② 上游响应**收齐再给**与**边到边给**在"字节内容"上完全一样 ⇒ **必须测时序**
       （记首字节到达时刻 TTFB）：本机实测 Go 0.002s / PuXian 0.005s（上游 1.5s 分两段），
       收齐再给的话 TTFB 必然 ≥1.5s。门见 `px/tools/diff_stream_inc.py`。
97. **正则引擎的「Go 保真」（第 19 轮 · 缺陷 84 根治 + 一族静默错值，必记）**：
    · **字符类内的转义表照 Go 实测**：修前 `[\D]` `[\W]` `[\S]` 被当**字面量** D/W/S、
      `[\b]` 被当字面量 b（Go 一律报 `invalid escape sequence`）—— 静默错值。
    · 新增 `\b` `\B`（**ASCII** 词边界：`\w` 一侧、`\W/\A/\z` 一侧）· `\A` `\z`（文本首尾，
      等价 `^` `$`）· `\Q…\E`（字面引用）· `\xHH` / `\x{H…}` / 八进制 `\0`..`\777`。
    · **八进制入口条件是 Go 的**：首位 `0` 一律合法；首位 `1`-`7` **必须**后面还有一位八进制
      （`\77` 合法 = 0o77，`\7` 非法 —— Go 视单字符 `\1`..`\9` 为不支持的反向引用）；
      `8`/`9` 非法。字面量转义 = **除 ASCII 字母/数字与非 ASCII 外皆可**（含空格与控制字符）。
    · `\s` == `[\t\n\f\r ]`（**不含 `\v`**），而 POSIX `[[:space:]]` == `[\t\n\v\f\r ]`（**含**）——
      两处此前都写成"含 \v"⇒ 已按 Go 拆开。
    · **零宽断言上的量词**：`\b{2} \b+ \b{3,} \B{2} ^{2} $+` ≡ **断言本身**；`\b{0} \b* \b? $*` ≡
      **恒匹配（零宽）**（旧实现：BFS 的"end 必须前进"去重守卫把 level ≥ 1 全丢 ⇒ `\b{2}` 永不匹配）。
    · **空匹配迭代规则**（`regex_find_all` / `regex_replace`，照 Go `allMatches` / Rust `find_iter`）：
      非空匹配 ⇒ 下一个搜索位置 = 尾部；空匹配 ⇒ 位置 +1，**且若其起点恰为前一次匹配的尾部则丢弃**
      （`a*` 在 "baab" 上：Go `[[0,0],[1,3],[4,4]]`；旧实现多产出 `[3,3]`）。
    · **`(?:…)` 非捕获组**：旧引擎把 `?:` 当**字面量** ⇒ `(?:api[_-]?key|apikey…)`（= 现网
      `secret_patterns` 的写法）**永不匹配且不报错**；其余 `(?…)` 形式（命名组/内联标志/环视）现在
      **显式报错**，绝不静默降级（宁可不匹配也不静默错值）。
    · **已知边界（引擎按字节，Go 按 rune）**：非 ASCII 文本上位置/条数/匹配片段都会不同
      （`\b*` 在 "中文abc中文" 上：本引擎 16、Go 8）；`regex_split` 只在**非空**匹配处分割
      （Go 的空匹配也分割）；无 Unicode 表 ⇒ `\p…` 报错（Go 合法）；类内多字节码点（`[\x{4e2d}]`）
      报错（字节类表达不了）。
    · 门：`examples/m138_regex_go_parity/` —— **775 行**与 Go regexp 本尊逐字节 diff（穷举 0x20–0x7e
      的 `\c` 与 `[\c]`、含 15 组文本 × 40 模式 × find/find_all/search+groups/全匹配、替换/分割），
      VM+C 双轨 + 自断言（含上述每条边界）+ 双负控。
98. **`px_gzip_decompress`：**输出缓冲不够不是错误**（第 19 轮 · 缺陷 107，已修）**：
    miniz 在 `MZ_FINISH` 下满载即返 `MZ_BUF_ERROR`（且入口 `avail_in == 0` 时**无法续跑**）⇒
    必须**换更大缓冲重来**（容量翻倍，均摊 O(n)），不能当失败。旧实现按 `inlen*3 + 4096` 一次定容、
    把"非 OK/非 STREAM_END"一律当失败 ⇒ 返回 NULL ⇒ 调用方（`h_exchange`）**静默保留压缩体**：
    686 字节的 gzip（解压 327680 字节，压缩比 477×）经 `http_request` 得到的是**682 字节 gzip 流**而不是文本。
    实证：`examples/m24_http_adv` 那条 `assert(len(r.body) == len(big))` 修前必红、修后必绿
    （而它此前是**死断言** —— 见 94）。负控：截断/非 gzip/空 仍返回 null（语义不变）。
99. **门自己写错了也是缺陷（第 19 轮 · 复现三次）**：
    · 逗号运算符式的 `run_px() { env "$@" "$PX" build; }` ⇒ 文件名落到了命令**前面**，
      5 个"正例"全报 `env: 'x.px': No such file or directory` —— 而**负例**照样"通过"（因为报错）。
      教训：负例不能只看"有没有报错"，要 **grep 具体诊断片段**（否则门在自欺）。
    · 一次 `pkill -f <脚本名>` 又把自己那一行命令行匹配上了（第 10 轮同款）⇒ 用 `ps|grep "[x]"` 形态。
100. **HTTP 客户端连接失败的成因分类（第 21 轮 · 缺陷 108，M140）**：`http_request` / `http_get` /
    `http_post` / `s3_*` 的底层连接函数是同一个 `px_tcp_connect_timeout`。修前它的失败路径是
    `freeaddrinfo(res); close(fd); return -1;` —— **`close()` 会覆盖 errno**，于是函数对
    「域名解析失败 / socket 建不出来 / connect 被拒 / 网络不可达 / 无路由 / 被丢包超时」一律只回 -1，
    语言层只剩一句 `net: 连接 <host>:<port> 失败`，**成因不可编程获取**。
    现行为（失败文案即分类，`out_stage`/`out_errno`/`out_addr` 位参回给调用方）：

    | 阶段 | 文案 | `out_errno` |
    |---|---|---|
    | 解析（getaddrinfo） | `net: 解析主机失败 <host> (eai=<码>)` | `EAI_*`（负码，如 `-2` = EAI_NONAME） |
    | socket(2) | `net: 创建 socket 失败 <地址>:<端口> (<errno>)` | errno |
    | connect(2) | `net: 连接 <地址>:<端口> 失败 (<errno>)` | errno（超时用 `ETIMEDOUT=110`） |
    | TLS 握手 | `net: TLS 握手失败 (<mbedtls 码>)` | mbedtls 返回码 |

    · 同轮把 `hints.ai_family` 由 `AF_INET` 改成 **`AF_UNSPEC`**（Go 的 `dial tcp` 是双栈）⇒
      「仅 AAAA 的上游」能连上了；返回前逐个地址尝试，**顺序 = getaddrinfo 返回序，全部失败时
      返回第一个地址的成因**（Go 的 `dialParallel`/`dialSerial` 也是给首选地址的错 ——
      Go 1.26 实测：`localhost:P` 双栈皆拒时 Go 打 `dial tcp [::1]:P: connect: connection refused`）。
    · 失败文案里的地址是**数字地址**（Go 亦然：给解析结果而非主机名；IPv6 加方括号）。
      故 `http://localhost:1/` 的 Err 是 `... 连接 [::1]:1 失败 (111)` 而**不是** `localhost:1`。
    · **URL 里的 IPv6 字面量**（`http://[::1]:8080/`）此前解析不出来（`hparse_url` 把 `[` 当主机名
      起点、端口解析成 `atoi(":1]")` = 0）。现在 `hparse_url` / `px_http_once` /
      `sse_cli_connect_slot` 三处都按方括号取主机（host 保留括号：Host 头与 Go 的 dial 文案都用
      带括号形式），去括号后才交给 `getaddrinfo`。
    · **`sse_connect_ex` 顺带对齐**：明文连接从「`gethostbyname`（IPv4-only）+ **无 connect 超时**
      的阻塞 connect」改为同一个 `px_tcp_connect_timeout`（获得双栈 + 连接期超时 + 同一套 errno）。
      它的 `stage` 枚举（1 参数/2 连接/3 发送/4 读头/5 状态码/6 CT/7 槽满）**新增 `8` = 域名解析**
      —— `px_tcp_connect_timeout` 自己的 stage 码（1/2/3/4）**不能**直接透传（它的 `3` 是 connect，
      而 sse 的 `3` 是发送）。
    · 门：`examples/m140_http_conn_errno/`（VM+C 双轨 9 断言 ×2 + 逐字节一致；不依赖任何外部服务：
      `127.0.0.1:1` 必拒、`.invalid` 必不解析、`2001:db8::/32` 无路由时必 `ENETUNREACH`）。
      项目侧门：`diff_llm_go.py` 的 `up-refused` / `up-dns` / `up-v6-noroute`（含流式），
      **负控实测**：把分类关掉 ⇒ `up-dns`/`up-v6-noroute` 判红。
    · 已知边界：`stage >= 3`（发送/读头）时 Go 的 `*url.Error` 含**本端地址**
      （`write tcp 127.0.0.1:41234->…`），本实现取不到（仍按 dial 形状输出）；`TLS 握手失败`
      只有 mbedtls 码，Go 的证书/握手文案依赖 x509 细节 ⇒ 消费方**原样透传**，不再谎称 refused。
101. **语言里没有「墙钟纳秒」（第 22 轮 · 缺陷 109，M141 新增 native `now_ns`）**：三个时间原语各有其轴 ——
    `now()` = 本地时间**字符串**、`now_sec()` = 墙钟**秒**、`now_ms()`/`now_us()` = **CLOCK_MONOTONIC**
    （自 boot 起算，**测量语义**）。所以「纳秒时间戳」在语言里**没有来源**。移植 Go 的
    `time.Now().UnixNano()` / `time.Now().Format("20060102-150405.000000000")`（token-cache 的
    ContextDebug 目录名 / `CacheLogEntry.Timestamp`）时若拿 `now_us()*1000` 冒充 ⇒ **值与数量级都不同**
    （墙钟 ~1.8e18 vs 单调 ~1.2e11；门内负控断言量级差 > 1e11）。**判断口径**：要「与 `time_format`
    同轴、可与墙钟互换」就用 `now_ns()`；要「测流逝（不受系统时间调整影响）」才用 `now_ms`/`now_us`。
102. **unix socket 服务端的 `req["remote"]` 是 `"@"` 而不是 `"unix"`（第 22 轮 · 缺陷 110，M141）**：
    Go `net/http` 在 AF_UNIX 上 `r.RemoteAddr` == `"@"`（**未 bind 的客户端**对端地址为空 ⇒
    `syscall.RawSockaddrUnix.path` 空 ⇒ Go 的 autobind 占位符；Go 1.26.6 实测）；本运行时原给 `"unix"`
    ⇒ 移植 Go 代码时「来源地址」字段（ContextDebug `up.md`、audit 日志）无法对齐。现 AF_UNIX → `"@"`，
    AF_INET 仍 `ip:port`。⚠️ 判据要**三面**齐：形态独占（无 `:`、长度 < 40）+ TCP 面**没被误伤**
    （服务端看到的是**客户端源端口**而非监听端口）+ 负控（`!= "unix"`）。

103. **`len(s)` / `s[i]` / `s[a:b]` 的「字符」定义必须唯一（第 23 轮 · 缺陷 111/112，M142 根治）**：
    修前运行时里有**三套**判定 —— `len()` 数「非连续字节」、`px_index` 按前导字节的**声称**长度走查、
    `px_slice` 走 1 字节再跳续字节。**合法 UTF-8 下三者同值**（所以长期没人发现），**畸形 UTF-8 下互相矛盾**：
    `len("A\x80B") == 2`（孤立续字节被 len 忽略）却 `s[1]` 能取到 0x80；`len("{\xe4}") == 2`（尾部截断的
    前导字节**吞掉**后面的 `}`）⇒ `for i in range(len(s))` **访问不到全部字节**，逐字符扫描器（JSON/HTTP 解析）
    在畸形输入上索引错位。现统一为 `px_utf8_step(p, rem)`：**合法序列 → 其长度；ASCII / 孤立续字节 /
    尾部截断 / 后继不是续字节 → 1**。⇒ `len(s) == 走查步数`、`offs[len] == bytes_len`（无字节被吞），
    合法 UTF-8 **零行为变化**。缺陷 112（同轮）：单字符提取原按「声称长度」`memcpy` ⇒ **越读 `str.len` 尾界**
    （把相邻内存的垃圾当字符返回）；现在长度由构造保证 ≤ 剩余字节，越读不可能。
    判据：`len(bytes_to_str(int_to_bytes(0x80,1,"little"))) == 1`、`len("{\xe4}") == 3`（= 字节数），
    且**双轨一致**（VM+C；门 `examples/m142_go_json_indent/` 的 N 段断言）。
104. **`json.Indent` 不是「格式化器」——它是在原文上按 scanner 分类**插入/删除**（第 23 轮 · M142）**：
    ① 字符串内字节**原样搬运**（`1.50` 不变成 `1.5`、`\u4e2d` 不变成 `中`、非法 UTF-8 也不重新编码）；
    ② 空对象/空数组**不成行**（Go 的 `needIndent` 延迟缩进语义；`{}` → `{}` 而不是 `{\n}`）；
    ③ 顶层值**之后**的空白：Indent 保留、Compact 丢弃；最大嵌套深度（10000）属 **scanner** 行为，
    Compact/Indent 同源。⇒ 复刻必须**连 scanner 一起复刻**（`std/go_json_scan.px`），不能自己写美化器。
    ⚠️ **Go 的语义陷阱**：`json.Compact` 返回的 `*SyntaxError.Offset` **恒为 0**（`appendCompact` 从不递增
    `scan.bytes`，实测 12 例全 0），而 `Indent` 给真实的 **1 基字节偏移** —— 两者**不是一个口径**，别照抄。
    现成 API：`go_json_indent(src, prefix, indent)` → str|null（错则调用方回退原文，正是 token-cache
    `ContextDebug.prettyJSON` 的用法）· `go_json_indent_ex` → {ok,out,err,offset} · `go_json_compact(src, escape)`
    （escape=true = `Marshal` 的 `<`/`>`/`&`/U+2028/29 转义路径）· `go_json_htmescape(src)`。
    门：`examples/m142_go_json_indent/`（88 例 × 4 面 + `quoteChar` **全 256 取值** = 696 行与 Go 本尊逐字节 diff
    + 31 断言 × 双轨 + 三道负控）。
105. **源码里不能写 `\xNN`；Unicode 转义是 `\u{XXXX}`（第 23 轮实测）**：`"\x80"` 报
    `词法错误 E1005: 非法转义序列 \x`；`"\u2028"` 报 `E1005: Unicode 转义须为 \u{XXXX} 形式`。要造含
    **非 ASCII 单字节**的串（测错误文案、构造二进制头）必须
    `bytes_to_str(int_to_bytes(b, 1, "little"))`（`bytes(list)` **不是**字节缓冲 —— 它是 `str(list)`！）。
106. **同一文件用两种 import 写法引入 ⇒ C 轨重复定义（第 23 轮 · 新的「双轨不一致」面）**：
    `stdlib/go_json_indent.px` 原写 `import "./go_json_scan.px"`，而门里又 `import std.go_json_scan`
    ⇒ C 轨把两边都发射 ⇒ `error: redefinition of 'fn_gjs_step'`（**编译期暴露，好**）；
    **VM 轨按顶层符号名覆盖、静默通过**（坏）。修法：**一律用规范路径**（`std.<name>`，stdlib 之间互
    import 也照此，与 yaml/yamlx 同惯例）。
107. **门自己写错（第 23 轮 · 又一次）**：M142 首跑 6 条断言红，**全部是我把期望值写错**，不是实现错 ——
    `[1,]` 在 Go 的 scanner 里是**语法错误**（`invalid character ']' looking for beginning of value`，
    尾随逗号**不**被接受：逗号后进的是 `stateBeginValue` 而不是 `stateBeginValueOrEmpty`）；
    `["中文中文"]x` 的偏移是 **17**（1 基**字节**）；0x80 的上下文是 `looking for beginning of value`
    而不是 `after array element`。⇒ 纪律：**期望值只能从真值产**（本门最终形态：696 行真值 diff 为准，
    断言只钉"不变量 + 已实测常量"）。

108. **float32 值族：语言里只有 float64，float32 必须 `f32()` 逐步收口（第 24 轮 · M143 · 缺陷 115）**：
    原语三件套 —— `float32(x)`（舍入到最近的 float32，IEEE754 round-to-nearest-even，以 float64
    **精确**承载）· `float32_bits(x)`（`math.Float32bits(float32(x))` 的 uint32，int 承载）·
    `bits_to_float32(u)`（`math.Float32frombits(uint32(u))`，只取低 32 位）。
    **纪律**：float32 的 + - * / sqrt，其**精确结果**在 float64 里可精确表示（53 ≥ 2\*24+2 位）
    ⇒ `f32(a op b)` 与 Go 的 float32 直接算**逐位相同**（无二次舍入），不必为每种运算造原语；
    但**循环累加必须每步收口**：`f32(acc + b)` 而非「float64 累加、最后窄化一次」——
    实证 `1.0f 每次 +1e-8f × 1000` ⇒ 每步收口停在 `3f800000`(1.0)，float64 累加后窄化 = `3f800054`(1.00001)。
    ⚠️ 另两处实测：`float32(1e40)` = `+Inf`（位模式 7f800000，**不报错**）；`int → float32`
    必须走 C 的直接转换（先 double 再 float 会**二次舍入**）。
109. **Go `encoding/json` 的浮点数文本按**位宽**分两套（第 24 轮 · M143 · 缺陷 117）**：
    `strconv.AppendFloat(f, 'f'|'e', -1, bits)` —— `bits=64` 给 float64 最短往返，
    `bits=32` 给 **float32 最短往返**（`1/3` → `0.33333334` 而非 `0.3333333333333333`；
    `0.1` → `0.1`）。科学计数阈值也按位宽比较（`float32(abs) < 1e-6 || >= 1e21`）。
    口岸值（Go 1.26.6 实测）：`1e-7`→`"1e-7"`（`e-0d`→`e-d` **收敛**）· `1e-6`→`"0.000001"`
    （**阈值不含**）· `1e20`→`"100000000000000000000"`（定点）· `1e21`→`"1e+21"`（`+0d` **不**收敛）·
    `-0.0`→`"-0"`（Go **不**把负零写成 0；注意源码字面量 `-0.0` 会被常量折叠成 +0，真值须用
    `math.Copysign(0,-1)` / `-1.0*0.0` 造）· `MaxFloat32`→`"3.4028235e+38"` · 最小次正规→`"1e-45"`。
    现成 API：`json_num_str(x[, bits])`（非有限值 → **null**，对应 Go 的 `UnsupportedValueError`；
    `json_stringify_go(v[,opts])` 已按 bits=64 走同一条渲染路径）。**注意**：语言自有的
    `json_stringify` 仍走语言习惯（`1e-07`、`-0`）—— 需要 Go 兼容时**必须**用 `json_stringify_go` /
    `json_num_str`；两者不是同一个口径，这是**已登记边界**（不是缺陷）。
    ⚠️ **已登记边界**：`str(某 float32 值)` 走的是 **float64** 最短往返（运行时只有 float64
    值，无从得知"它原本是 float32"）⇒ 与 Go 的 `fmt.Sprint(float32)`（32 位 'g' 最短）**不同**；
    需要 Go 文本时用 `json_num_str(x, 32)`。
110. **浮点除零：Go 是良定义（±Inf / NaN），PuXian 目前 `px_error` 杀进程（缺陷 118，未修）**：
    `px_div` 对**任何**零除数（含 `0.0`）都报「除零错误」⇒ Go 里合法的 `x/0.0`（Inf）、
    `0.0/0.0`（NaN）、向量归一化的退化分支若写成除法就会**打挂进程**。本轮的浮点门语料
    **刻意不含零除数**（避免把语义分歧混进保真门），该分歧按独立里程碑跟踪（改语义要配
    双轨负控门 + 全仓回归）。判据：`float(1.0)/float(0.0)` 在 Go 给 `+Inf`，在此报错。
111. **float32/float64 混用表达式必须逐项核对 Go 源码的「窄化位置」（第 24 轮 · 缺陷 119）**：
    `embedding_types.go::CosineSimilarity` 写的是 `dot += float64(a[i] * b[i])` ——
    **乘积先在 float32 里算**再拓宽累加；而 `vector_util.go::DotProduct` 写的是
    `sum += float64(a[i]) * float64(b[i])` —— **乘积在 float64 里算**。同一个文件里两种写法并存，
    「看起来等价」，在**恰好是 float32 精确值**的语料（0.5/2.0/1024.5）上**完全同值**
    （第 23 轮的 G 段因此全绿），换 0.1 / 1÷3 / 1e-8 语料后当场分叉
    （实测 `CosineSimilarity` Go 给 `0`，float64 乘积版给 `3.0000000590866564e-21`）。
    ⇒ 纪律：**语料里必须混入 float32 不能精确表示的值**（0.1 / 1÷3 / 1e-8 / 次正规），
    否则窄化面等于没测。
112. **`append_file_opt(path, content[, mode])`（第 24 轮 · 缺陷 115 收口）**：`append_file` 的
    **Result 版** —— `Ok(null) | Err("io: 追加写入失败 …: No such file or directory (os error 2)")`。
    Go 侧 `os.OpenFile(..., O_APPEND|O_CREATE|O_WRONLY, 0644)` 的 err 通道是「只记日志、不阻塞主流程」
    （token-cache LogWriter），而 `append_file` 失败即杀进程 ⇒ 磁盘满/权限不足会**整服务死**。
    与 `write_file_opt` 同款纪律：**不** fchmod（Go 的 perm 只在**创建**时生效）·
    `+ append_file_opt` 也进了解释轨名册（转发层 + names 两处同步，见第 22 轮教训）。

113. **HTTP 服务端（`http_serve` / `http_serve_unix`）的请求体两条通路**（第 25 轮 · 缺陷 121–124）：
    `Content-Length`（定长）与 **`Transfer-Encoding: chunked`**（分块）**都支持**，handler 都从
    `req["body"]` 取**解码后**的体。上限默认 256MB（`PX_HTTP_BODY_MAX` 可改），超限回 **413**
    并关闭连接（两条通路同语义）。⚠️ 此前 chunked **静默丢体**（handler 拿到空串却回 200）——
    客户端发 chunked 体时必须走本修正后的 runtime。**边界**：`req["headers"]` 保留
    `Transfer-Encoding` 原文（Go 的 `net/http` 会把它从 `r.Header` 删掉放进 `r.TransferEncoding`
    —— 这一条是**已登记分叉**）。
114. **HTTP 客户端的大请求体**：`http_request` / `http_unix` / `http_post` / `s3_put` 的体
    **不再 memcpy 进栈缓冲**（修前 >16KB / >3.9KB / >4KB 直接**段错误**，而服务端却收到完整请求
    —— 典型「看起来成功」的内存安全 bug）。除 `http_post`（`px_http_once`，HTTP/1.0 短连接）外
    都支持**任意大小**体（体由 `h_exchange` 独立发送，`Content-Length` 按字节长度计算）。
115. **`http_unix(sock, path, method, body, headers, {"chunked": true})`**：用
    `Transfer-Encoding: chunked` 发送体（**不发** Content-Length），分帧 **32768/块**
    —— 与 Go `net/http`（未知长度 Reader ⇒ `io.Copy` 32KB 缓冲）**逐字节同形**。
    `opts` 与 `timeout_ms` 同字典共存（`{"chunked": true, "timeout_ms": 2000}` 合法）。
116. **HTTP 管道化 / 连接级余留缓冲**（第 25 轮 · 缺陷 124）：同一 TCP 段里读进来的**下一请求**
    字节不再被丢弃（存 `PxConnCtx.pbuf`，跨 worker 调用 / handler 协程续写存续），
    管道化的第二个请求**照常应答**；余留字节在响应后若存在，本 worker **不再交还 IDLE**
    （否则字节随 worker 栈一起丢）。判据不能用 `fd 可读`：那些字节早在用户态缓冲里。
117. **循环引用值的「比较 / 渲染 / JSON」三层语义（第 26 轮 · M145 · 缺陷 125）**：M145 前
     `var g = {}; g.set("me", g)` 之后 `g == g` / `str(g)` / `json_stringify(g)` **一律段错误**
     （三条通路各自无界递归 ⇒ C 栈溢出；VM/C 双轨同源，实测 rc=139）。现语义：
     · **`==`**：环上「已访问对象对」再次出现 ⇒ 视为**相等**（对齐 Go `reflect.DeepEqual`）；
       非环的**共享**子对象不受影响（`{"p":a,"q":a}` 与 `{"p":{…},"q":{…}}` 照常按内容比）。
     · **`str()` / `print()`**：**路径**上的对象再次出现 ⇒ 渲染 `...`
       （`str(g)` == `{me: ..., n: 1}`）；**共享而非环**的必须完整渲染 —— 所以用**路径栈**
       而不是累积集合（后者会把 `{"p":a,"q":a}` 的第二处误渲染成 `...`，改坏既有合法输出）。
     · **`json_stringify` / `json_stringify_go`**：环上 **`px_error`**
       `json: unsupported value: encountered a cycle via dict`（Go `json.Marshal` 同族文案）
       ⇒ 受控退出 rc=1（**不是** 139）。解释轨（`px run`）同三条契约（`ival.px` 同步修）。
     · 新 native **`object_id(v)`**：对象**不透明**标识（同一性）。堆对象 → 地址（存活期内恒定，
       mark-sweep 非移动 GC）；非堆值（int/float/bool/null）→ 0。用途：解释轨环检测、
       以及**移植带指针语义的 Go 代码**（`child == leaf` 这类判断 —— PuXian 的 `==` 是结构相等，
       做不到「是不是同一个对象」）。
     门：`examples/m145_cycle_safe/`（三轨 VM/C/解释轨输出逐字节一致 + 环上 JSON 受控报错 + 2 负控）。
118. **float64 位模式族：`float64_bits(x)` / `bits_to_float64(u)`（第 27 轮 · M146）**：M143 补了 32 位的
     一对（`float32_bits` / `bits_to_float32`），**64 位的对应项一直缺失** —— 后果不是"少个糖"：
     · **读不出** float64 的位模式 ⇒ 跨语言/跨机器对拍 float64（SS / 半径 / 置信度 / 距离 /
       `time.Duration`）时没有**无损指纹**，只能靠"最短往返文本实现一致"这个弱前提；
       M143 的 `json_num_str(x, 64)` 是替代品，但它要先约定文本格式。
     · **造不出** NaN / ±Inf / **非规格化数** / −0.0 —— 这四类**没有十进制字面量**：
       `bits_to_float64(0x7FF0000000000000)` 才有 +Inf，`bits_to_float64(1)` 才有 4.9e-324。
       移植任何"按位构造浮点"的 Go 代码（`math.Float64frombits`）时这是**唯一入口**。
     语义与 Go 一一对应（`math.Float64bits` / `Float64frombits`）：返回的 uint64 以 **int64
     二进制补码**承载 ⇒ ≥2^63 的位模式是**负值**（= Go `int64(math.Float64bits(f))` 的再解释口径），
     取十六进制文本用 `int_to_hex(v, 16)`（"取低 64 位"，对负值同样给正确的补码文本）。
     `bits_to_float64` 只取**低 64 位**（负值 = 回绕）。
     ⚠️ **`-0.0` 字面量是 +0.0**（与 Go 一致）：Go 的常量是无符号零，`var a = -0.0` 后
     `math.Signbit(a)` 为 **false**；PuXian 同。要 −0.0 只能 `bits_to_float64(0x8000000000000000)`
     或 `float("-0.0")`（`strtod` 保号）。同理 **±0 的 `==` 相等**，只有位模式能区分。
     门：`examples/m146_float64_bits/`（577 行语料 = 十进制口岸 + 整数 + 512 个伪随机 64 位模式
     + 21 个位模式口岸，与 Go 本尊逐字节 diff × VM/C 双轨 + 54 条自断言 + 3 道负控）。
119. **比较运算的 NaN 语义：IEEE754「四路皆假」（第 27 轮 · 缺陷 126，M146 根治）**：这是 M146
     补出 `bits_to_float64` 后**当轮就被门照出**的真缺陷（此前语言里造不出 NaN ⇒ 通路不可达）：
     · 修前：`NaN != NaN` → **假**、`NaN <= 1.0` → **真**、`NaN >= 1.0` → **真**
       （`NaN < x` / `NaN > x` / `NaN == x` 恰好是对的）。
     · 根因：`compare_values` 是**三态**比较器（-1/0/1），数值分支是
       `x < y ? -1 : (x > y ? 1 : 0)` —— NaN 两个判断都假 ⇒ **落进 else 被当成"相等"**；
       而 `px_ne` / `px_le` / `px_ge` 都借道它。
     · 修法：`px_eq/ne/lt/le/gt/ge` 对**数值**直接走 C 的浮点比较（IEEE 原生正确），
       非数值仍走 `compare_values` ⇒ 容器 / 字符串 / 枚举的**全序**（`sorted` / `sort_by` /
       `min` / `max`）语义**零变化**；`compare_values_raw` 的数值分支里 NaN 返回**非零**
       （三态器只能以"非零"表达"不可比"）⇒ `[NaN] == [NaN]` 为**假**，
       与 Go `reflect.DeepEqual`（`v1.Float() == v2.Float()`）一致。
     · ⚠️ **INT-INT 必须保留整数比较**（不绕 double）：否则 `9007199254740993 == 9007199254740992`
       会因丢精度误判为真。修后全表：`==`假 `!=`真 `<`假 `<=`假 `>`假 `>=`假（6 条自断言钉住）。
     · 解释轨（`px run`）的 `Ne` 走 `not i_eq`，本就对；修 runtime 后两轨一致。

120. **定点小数文本 `fmt_float_dec(x[, dec])`（第 29 轮 · M147 · 缺陷 127）**：Go 的
     `fmt.Sprintf("%.*f", dec, x)` 口径。**dec 缺省 6**（= Go `%f` 的默认精度）；`dec<0` 钳 0、
     `dec>64` 钳 64（⚠️ **不是** `strconv.FormatFloat` 的 `-1`="最短往返" 语义 —— 那用
     `json_num_str` / `str()`）。非有限值给 Go 文本 **`NaN` / `+Inf` / `-Inf`**（libc 给 `nan`/`inf`
     ⇒ 必须显式分支）。其余交 libc `snprintf("%.*f")`：与 Go 一样按**二进制精确值**做正确舍入、
     **并列取偶**。
     · 判据（反例即断言）：`fmt_float_dec(0.125, 2)` = `"0.12"`（不是 `"0.13"`）、`2.625` → `"2.62"`、
       `fmt_float_dec(3.5, 0)` = `"4"`、`fmt_float_dec(4.5, 0)` = `"4"`、`-0.5 @0` → `"-0"`、
       `1e20 @2` → `"100000000000000000000.00"`（精确整数展开，不是 `1e+20`）。
     · **别再用 `int(x*10^d + 0.5)` 手搓**（四舍五入-半向上）：只要语料里出现可精确表示的 `.x5`
       就**必然**与 Go 分叉。回归门：`examples/m147_fmt_dec/`。
     · 与另两个浮点文本族**不混**：`str()`（语言最短往返，带 `.0` 补丁）· `json_num_str(x[,bits])`
       （`encoding/json` 的浮点文本）。要哪个口径就用哪个，别互相"近似"。
121. **`-0.0` 的符号位在三段管线**上都可能丢**（第 29 轮 · 缺陷 128/129/130，M147 根治）**：
     症状只有一条 —— VM 轨 `float64_bits(-0.0)` 给 `0000000000000000` 而 C 轨/解释轨给
     `8000000000000000`（双轨不一致）。成因三层，各自独立、**修完上一层门还是红的**：
       ① **负字面量折叠**写成 `0 - v`（IEEE `0 - 0` = **+0.0**）⇒ 必须翻**符号位**
          （`bits_to_float64(float64_bits(v) ^ (0 - 9223372036854775807 - 1))`）；
       ② **常量池去重**按 `e["f"] == f` 比较 ⇒ `-0.0 == 0.0` 为真、负零被并到正零 ⇒ 浮点常量
          一律**按位模式**比（顺带：NaN 的 `==` 恒假 ⇒ 按 `==` 比会每次新增常量）；
       ③ **C 源码里的字面量**：把 `"-0.0"` 的 `.0` 去掉得 `"-0"`，而 C 的 `-0` 是**整型 0**
          ⇒ `-0` 时**保留 `.0`**（C 按浮点常量解析才得到负零）。
     ⇒ 教训：**"改一处就好"在这条管线上不成立**；断言要写"三轨逐位一致"，别只看数值。
122. **`type()` 对字符串返回 `"string"`（不是 `"str"`）；三轨一致**（第 29 轮实测）：写移植代码时
     若按别处习惯写 `type(v) != "str"`，它**永远为真** ⇒ 正常输入被判"类型不符"而静默跳过
     （本轮就是这么让 7 条 miss 记录全被丢掉的，症状是"语料非空但计数为 0"）。
     容器/标量名：`"dict"` `"list"` `"int"` `"float"` `"bool"` `"string"` `"null"` `"bytes"` `"function"`。
123. **没有行续接符**（第 29 轮实测）：行尾 `\` 折行会报 **`E1001 非法字符: '\'`**。
     长表达式只能**拆成临时变量**分段赋值，或写在**括号内**（`print(...)` 的实参表里可换行）。
     同族提醒：字符串里**没有 `\\` 转义**（写不出"反斜杠字符"的字面量）；`\"` / `\n` / `\r` / `\t` 都有。

124. **`/` 是浮点除法 ⇒ 完全 IEEE-754，永不报错；`//` 与整数 `%` 才守除零**（第 30 轮 · M148 · 缺陷 118）：
     —— Go 的浮点换算是良定义的：`x/0.0` = ±Inf、`0.0/0.0` = NaN、`math.Mod(x,0)` = NaN。
     修前 PuXian 的 `/` 对零除数**一律 `px_error` 杀进程** ⇒ 移植 Go 的比值/余弦/成功率/均值
     都得先手写零判断，**表达不出**「÷0 → Inf/NaN」这条路。
     · **规则**：`/` **整数操作数也走浮点除法**（`5/2` = `2.5`、`1/0` = `+Inf`）——
       因为 `/` 的定义就是浮点除法；要整数除**商**用 `//`，要整数余数用 `%`。
     · `//`（`px_idiv`）与**整数** `%`（`px_mod` 整数分支）**除零仍然 `px_error`** —— 对齐
       Go 的整数除零 panic（`integer divide by zero`）。浮点 `//` 无 Go 对应物 ⇒ 保持 fail-fast。
     · **浮点 `%` = `fmod`（IEEE）**：`x % 0.0` → NaN、`±Inf % y` → NaN；余数取**被除数**符号
       （`-5.0 % 3.0` = `-2.0`）—— 与**整数** `%` 的**欧几里得**余数（`-7 % 3` = `2`，Rust
       `rem_euclid`）**不是一回事**，别把两者当同一个算子。
     · 三轨（runtime `px_div`/`px_mod`、VM `PXOP_DIV`/`PXOP_MOD`、解释轨 `i_bin_numeric`）同批改。
     · **`str()` 的非有限值不受影响**：仍是语言约定的 `inf` / `-inf` / `nan`（有 golden/s09 钉着）。
       本轮一度改成 Go 的 `+Inf`/`-Inf`/`NaN`，被三道 golden 拦下并暴露出".0 补丁只查小写
       `i`/`n`"的通路不一致 ⇒ 回滚。**要 Go 的文本用 `go_float_text` / `fmt_float_dec`。**
     回归门：`examples/m148_ieee_div/`（`corpus_div.txt` 792 行含 ÷0、0/0、±Inf、NaN、非规格化、
     上下溢 × Go 本尊逐字节；负控 C = 把零检查加回去 ⇒ 门必须红）。
125. **要 Go 打印浮点，用 `go_float_text(x[, bits])`，别指望 `str()`**（第 30 轮 · M148 · 缺陷 132）：
     `str()` 的浮点文本是**语言自身约定**：定点舒适区是十进制指数 `x ∈ [-4,15)`、整值补 `.0`
     （`250.0` → `"250.0"`、`1e15` → `"1e+15"`）。而 Go 的 `fmt.Sprint`（`%v`）是另一套：
     · **不带尾随 `.0`**：`250.0` → `"250"`、`100000.0` → `"100000"`；
     · **指数 ≥ 6 就转科学计数**：`1234567.0` → `"1.234567e+06"`（`999999.0` 仍是 `"999999"`）；
     · 科学计数的指数**至少两位且必带符号**：`"1e+06"` / `"1e-05"`（**不是** JSON 的 `"1e-5"`）；
     · `bits=32` 走 float32 最短往返：`go_float_text(1.0/3.0, 32)` = `"0.33333334"`。
     ⇒ 移植 Go 的日志/文本输出（`fmt.Sprintf("%v", f)`）时用这个 native；**两族并存、互不覆盖**
     （改 `str()` 是**破坏性**变更，会动到全仓 golden —— 不做）。
     四个浮点文本族的分工：`str()` **语言约定**（有限值定点区 `[-4,15)`+补 `.0`；
     **非有限值是 `inf`/`-inf`/`nan`**）· `json_num_str(x[,bits])` `encoding/json` ·
     `fmt_float_dec(x[,dec])` Go `%.Nf` · `go_float_text(x[,bits])` Go `%v`。
     ⚠️ **别把 `str()` 当成 Go 的 `%v`**：`str(250.0)` = `"250.0"` 而 Go 给 `"250"`——
     移植 Go 文案时前者会**逐字节不同**（本轮就是这么发现这条口径的）。
126. **`str()` 必须能往返 —— 定点分支的位数上界要按有效位数算**（第 30 轮 · M148 · 缺陷 133）：
     症状：`str(0x3f2d1ac1aeaf35e2 的值)` 给 `"0.00022204984938272"`，而 `float()` 读回来**已不是**
     原值（Go/Python 给 `"0.00022204984938271806"`）—— **静默丢精度**，比报错更糟。
     成因：定点分支的候选精度写死 `for (dec = 0; dec <= 17; dec++)`；定点文本的"位数"是**小数点后**
     的位数，而 `x ∈ [-4,-1]` 的 `0.000…` 形态需要 `17 - x` 位才够 17 位有效数字 ⇒ 循环跑到底
     仍未回读相等，`num_buf` 就留下了最后一个候选。
     修法：上界改 `17 - x`（钳到 1..21）。**判据**（写进 m148 门）：对每个有限值断言
     `float(str(f)) == f`（`M148_RT: checked=189 bad=0`）；负控 E = 退回 `dec <= 17` ⇒ 必红。
     ⇒ 教训：**"最短往返"这类性质要写成可执行的断言**，光靠"实现注释里写了 roundtrip"不算数。

127. **NaN 的文本**归一为 `nan`（第 30 轮 · M148 · 缺陷 134 潜伏项收口）：
     · 原先 `str(±NaN)` 走 libc `%g`：glibc 对**负 NaN** 给 **`-nan`**（符号位来自位模式），
       而 `0.0/0.0` 在 x86 上**就是负 NaN** ⇒ `str()` 的文本**依赖平台与运算**：
       同一个 NaN 可能印成 `nan` 或 `-nan`。
     · **更糟的是编译器侧**：`selfhost/codegen.px::cg_fmt_float` 的**非有限值白名单**只列了
       `"inf" / "-inf" / "nan"` ⇒ 真出现 `"-nan"` 时**漏过**该分支，继续走去掉 `.0` 的逻辑
       （与缺陷 130 同族的手法）⇒ 可能产出错误字面量。
     · 修法：**从源头归一**（`fmt_num` 里 `isnan(f)` → `"nan"`，丢符号位），
       并给 `cg_fmt_float` 的白名单补 `"-nan"` 作**纵深防御**（`str()` 已不会再产出它）。
     · 约定（记牢）：**`inf` / `-inf` / `nan`** 是 `str()` 对非有限值的文本；`+inf`/`-inf`
       由 `%g` 给出（各 libc 一致），NaN 一律 `nan`（**不看符号位**）。

128. **要用 TCP 就得用 `tcp_*_ex` 族 —— 旧 `tcp_*` 的失败面会杀进程、也没有超时**（第 31 轮 · M149）：
     · 旧接口三个硬伤：`tcp_connect` 失败 `px_error` **杀进程**、**无连接超时**（SYN 被丢就永久挂起）、
       `tcp_recv` 把 **EOF / 读超时 / 出错** 一律返回 `""`（三者不可分辨）；`tcp_send` 出错同样杀进程
       且拿不到"已写出多少字节"。
     · Go 的对应物：`net.DialTimeout` / `SetReadDeadline` / `Conn.Write` 的 `(n, err)` ⇒
       **没有这一族，"网络是常态故障源"的客户端（Redis/PostgreSQL/任何带 deadline 的协议）就移植不了**。
     · 用法（拿结果 dict，**永不杀进程**）：
       `d = tcp_connect_ex(h, p, {"timeout_ms": 300})` → `d["ok"]` / `d["fd"]` / `d["stage"]` / `d["errno"]`；
       `tcp_opt(d["fd"], {"read_timeout_ms": 300, "write_timeout_ms": 300})` → **回读生效值**；
       `r = tcp_recv_ex(fd, 65536)` → `r["eof"]`（对端关）/ `r["timeout"]`（读超时）/ `r["errno"]`（其它错误）三者可分辨。
     · **`nodelay` 默认 true**是刻意的：Go `net.Dial` 对 TCP 连接**默认开** `TCP_NODELAY`，
       不设就让小请求撞 Nagle + 延迟 ACK（request/response 形态最吃亏）。
     · 边界：`getaddrinfo` 阶段**不受 `timeout_ms` 约束**（libc 解析无异步取消入口）。
     门：`examples/m149_tcp_deadline/`（VM+C 双轨 53 断言 × 2 + 解释轨 9 断言 + 3 道负控）。

129. **`tcp_opt` 的"回读"不是装饰**（第 31 轮 · M149）：超时若只"设"不回读，测试就只能断言
     "调用没报错" —— 那等于什么都没验。`tcp_opt(fd, {...})` 改完立刻 `getsockopt` 并返回**生效值**
     （`nodelay` / `keepalive` / `read_timeout_ms` / `write_timeout_ms`），于是门可以断言
     `400ms 设进去、400ms 读回来`；`0` 的语义 = **无限**（timeval 清零）。

130. **"对端关闭后写"的 errno 是 `EPIPE(32)` 或 `ECONNRESET(104)`，不是"发送失败"这一句话**
     （第 31 轮 · M149）：首个写往往**成功**（进本地发送缓冲），要对端 RST 回来后的**第二次**写才失败
     ⇒ 断言要写成"循环写到失败为止"（门内实测第 1 次就拿到 `EPIPE`）。
     另：运行时**已忽略 `SIGPIPE`**（M88-S2），所以这条路径是**拿到 errno**而不是进程被信号打死。

131. **`mbedtls_ssl_conf_read_timeout` 必须配 `mbedtls_net_recv_timeout` 才生效**（第 32 轮 · M150）：
     `mbedtls_ssl_set_bio(ssl, net, send, recv, recv_timeout)` 的**第 5 个参数**是 `f_recv_timeout`；
     传 `NULL` 时 mbedtls 退回**阻塞** `mbedtls_net_recv`，读超时**完全不生效**（表现为 `tls_recv`
     永久挂起，而不是报错）。⇒ `read_timeout_ms > 0` 时必须换上 `mbedtls_net_recv_timeout`。
     到期返回 `MBEDTLS_ERR_SSL_TIMEOUT`，本族归成 `timeout=true` + `err="i/o timeout"`。
132. **服务端读不到 `SSLSocket.server_hostname` —— 那是客户端属性**（第 32 轮 · M150 的**夹具**教训）：
     Python 侧服务端要拿到对端 SNI 只能挂 `SSLContext.sni_callback`。第一版用 `server_hostname`
     打印，于是"客户端根本没发 SNI"与"服务端不会读 SNI"两种成因**看起来一模一样**（都是空）
     ⇒ 差点把运行时判成 SNI 没生效。**夹具自己也会撒谎，先证夹具。**
133. **`bytes_set(b, i, v)` 是写时复制、返回新 bytes，不是原地改**（第 32 轮 · M150）：
     要"逐字节改一个缓冲"必须写成 `b = bytes_set(b, i, v)`。同一族的 `bytes_get`/`bytes_concat`
     同理。用 `hmac_sha256`（返回 hex 文本）手搓 PBKDF2 时，`hex_to_bytes(...)` 往返是必须的语法。
134. **Go 的 `hex.EncodeToString(...)` 结果再入哈希时是 32 字节 ASCII**（第 32 轮 · M150 的**门**教训）：
     PostgreSQL md5 认证链是 `md5( hex(md5(pw||user)) || salt )` —— 内层摘要以 **hex 文本**（32 字节）
     参与外层哈希，**不是** 16 字节原始摘要。门的第一版写成 `hex_to_bytes(h1)`（= 16 字节），
     pgauth 20 行整段与 Go 分叉（**门当场照出**）。注意 `hex_to_bytes(md5(x))` 与 `md5_bytes(x)`
     是**同一串 16 字节**，拿它们互比是恒真的假断言。
135. **IP 字面量不发 SNI**（第 32 轮 · M150）：`tls_connect` 的 `servername` 缺省用 `host`，但
     `host` 是 IPv4/IPv6 字面量（含 `[::1]`）时 **不发 SNI** —— 与 RFC 6066 及 Go
     `crypto/tls` 的 `hostnameInSNI` 同口径。要强制发就给 `servername`。
136. **TLS 套件的"名字"与"数字"两套口径都要给**（第 32 轮 · M150）：`cipher_id` 是 IANA 号
     （Go 的 `tls.CipherSuite` 常量），`cipher` 是 Go `tls.CipherSuiteName` 的**文本**；
     mbedtls 自己给的是 `TLS1-3-AES-256-GCM-SHA384` 这种**带横线**的 IANA 名 —— 运行时做了
     `-`→`_` 与 `TLS1_3_`→`TLS_` 两步映射（对 mbedtls 支持的全部套件都成立），
     **不是**有损近似。但**不要**拿套件名去跨实现比：Go 与 mbedtls 的**偏好序**不同
     （本机实测 Go 选 `0x1301`，mbedtls 选 `0x1302`）—— 比 `version_num` / 收发字节 / EOF 这些
     实现无关量，套件名只与**同一 id 的 Go 文本**比。
       要 Go 的口径（`+Inf` / `-Inf` / `NaN`）用 `go_float_text` / `fmt_float_dec`。

137. **`tcp_send` / `tcp_send_ex` 只认 str 时，二进制载荷会静默变成占位符**（第 33 轮 · M151 · 缺陷 137）：
     `args[1]` 只判 `PX_STR`，其余一律走 `px_to_string()` —— 而 `str(bytes)` 是占位符 `<bytes N>`
     ⇒ `tcp_send_ex(fd, b)` 把 10 字节的 `"<bytes 7>"` 发出去，**载荷一个字节都没发**，而且**返回成功**。
     同族 `sse_write` 早就两种都认 ⇒ **同一 runtime 里两种口径就是漂移源**。
     门内实证：受控回显服务端收到 `3c6f626a6563743e`（= `"<object>"`）而不是 `410042ffc3285a`。
     教训：**`px_to_string()` 对 bytes 是给日志看的占位符，永远不要让它碰线协议。**

138. **同一文件被两条轨以不同 import 闭包加载时，「靠邻居提供」的依赖会在另一条轨变成未定义变量**
     （第 33 轮 · M151 · 缺陷 138）：`cg_perr` / `cg_pwarn` 定义在 `codegen.px`，而 `cg_module.px`
     **同时**被 `interp.px` 直接 import（解释轨里没有 `codegen.px`）⇒ 解释轨走到「找不到模块」这条
     **诊断**路径时抛 `运行时错误 [cg_stdlib_dir 行203]: 未定义变量: cg_pwarn` —— **诊断本身成了崩溃源**
     （`px run` 一个 `import "./nope.px"` 的文件退出码 1，用户看到的是内部符号名，而不是「找不到模块」）。
     修法：在 `cg_module.px` **本文件内**实现 `cgm_perr` / `cgm_pwarn`（文案与 `codegen.px` 逐字节相同），
     调用点改走 `cgm_*`。**诊断代码必须与被诊断对象同自足性。**
     ↳ 同族纪律：**`px run` 不再 `2>&1` 合并 stderr**（改成与 `pxc` 同口径，Issue 45：诊断走 stderr、
       不污染数据流）—— 否则「诊断走了 stderr」这件事**断言不出来**（会被混进 stdout 里看起来一样）。

139. **负控门被打断 ⇒ `runtime/*.c` 会「静默」留在篡改态**（第 33 轮 · M151 · 缺陷 139）：
     负控靠"改字面量 / 改条件再重建"来验红，改动**语法合法、语义反向** ⇒ 编译器不报错、`pxc` 照常构建；
     而门自己带着未提交改动时，`git diff` 也判不出来（本轮开工实测：`runtime/runtime.c` 与门内快照
     **md5 不一致**，差异正是 M150 负控 C 的 `MBEDTLS_SSL_VERIFY_REQUIRED` 篡改体
     —— **差一步就带着篡改态提交上库**）。三道防线：
     · **残留标记 + 开门预检**：负控写入 `/* NEGCTL-<门>-<项> */` 注释；门开头查 `runtime/{runtime,vm}.c`
       是否含 `NEGCTL` ⇒ 命中立刻退出（"上一轮门被中断？先还原再跑"）；**还原之后再查一次**。
     · **全门预检**：`m116_gates.sh` / `m117_gates.sh` 开头同样 `grep -l 'NEGCTL' runtime/*.c` ⇒
       不让**整轮全门**建在脏源上（否则一轮几十个门全白跑，还看不出为什么）。
     · **信号兜底**：`cp runtime.c <快照>` + `trap restore_rt INT TERM HUP`。
       ⚠️ **局限（本地实测）**：bash 的 trap 在**当前前台命令结束之后**才执行（`kill -TERM` 后文件是在
       `sleep` 跑完那一刻才被还原），对 **SIGKILL 无效** ⇒ 真正的兜底是"标记 + 预检"。
     教训：**能"自动还原"的东西，也要能"自动发现没还原"。**

140. **`tls_upgrade(fd, opts)` 的所有权是「成功即接管」**（第 33 轮 · M151）：在**已连接**的 fd 上做 TLS
     握手（PG 的 `SSLRequest` 协商必需 —— `tls_connect` 自己建 socket，表达不出「先明文发 8 字节、
     再在同一 fd 上升级」）。成功 ⇒ 句柄表**接管 fd**（此后 **不要再 `tcp_close`**，否则 mbedtls 的 BIO
     会读到已关闭的 fd）；失败 ⇒ fd **仍归调用方**（可继续明文用或关闭）。返回 dict 与 `tls_connect`
     **同形**，且两者共用 100% 的「配置 + 握手」代码（`px_tls_session_alloc` / `_config` / `_handshake`
     三段式）—— 不给「两份实现必然漂移」留机会。

141. **字符串 `out += chunk` 是平方级，且「少量大对象 + 海量字节」会绕过 GC 阈值**（第 34 轮 · M152 · 缺陷 141/142）：
     `px_add` 对 str 是「xmalloc 新缓冲 + 两段 memcpy + `px_str_len` **再深拷一次**」⇒ 逐段拼文本
     （编译器整图发射！）时间与分配都是 **O(块数 × 文本长)** 平方级；而回收只由**对象数**阈值触发
     （`g_gc_trigger_bytes` 是**死代码**：全仓无赋值点、无环境变量）⇒ 拼文本这种"块多、对象少"的相位
     永远打不到阈值，垃圾不回收。实测：16000 语句 → 114s / **7.2GB**（2000 语句只要 3.0s / 125MB）；
     `perf`：**70% 时间在 GC**（`gc_mark_obj` 44.6% + `px_gc_collect` 25.7%），`vm_run_loop` 只 2.9%。
     修法：① 发射改**块表 + 一次 `join`**（`bc_emit_c_program` 61 处 + `cg_generate` 三处）；
     ② `bi_join` 自身改**游标 memcpy**（旧实现每项 `strcat` 从头上重扫）+ 归还拼装缓冲
     （旧 `px_str(out)` 是复制 ⇒ **整段泄漏**）；③ 补 `GC_TRIGGER_BYTES_DEFAULT=512MiB`
     （`PX_GC_TRIGGER_BYTES` 覆盖，<=0 关闭）。**对象数阈值管不住字节。**
     ↳ 硬判据：改前/改后对 **253 个 `.px`** 取 `--emit-c` 输出 sha256 ⇒ **逐条相同**。

142. **GC 阈值调小 = 白跑**（第 34 轮 · M152 · 缺陷 143）：`GC_THRESHOLD_DEFAULT` 原为 **10 万对象**
     ≈ 8MB 堆 —— 对 16GB 机器荒谬地小。GC 次数 = 分配量 / 阈值，而每次 GC 的代价是 **O(全堆)**
     ⇒ 阈值越小、白跑越多（实测 4000 语句语料：`PX_GC_DEBUG=1` 显示 **339 次 GC**，后 200+ 次
     「回收 0 个」；10 万 ⇒ 10.45s/97MB，**100 万 ⇒ 2.03s/292MB**）。内存下界交给**字节**阈值
     （见 141），故对象阈值放大到 100 万。两个旋钮一起才是「时间 ↔ 内存」的显式取舍。

143. **解释轨/VM 轨的"每语句 8500 次分配"是下一层瓶颈**（第 34 轮 · M152 登记 · 缺陷 145 候选）：
     4000 语句的整图编译累计分配 ≈ **3400 万对象**（339 次 GC × ~10 万）。本轮把"跑不完"变成
     "2.5 分钟出产物"，但**没有**收敛分配率本身。候选排查方向：`px_srcline` 上下文对象、dict 键串、
     AST/字节码的保留期、K/N 池的重复建串。**先量出"谁在分配"，再谈"少分配"。**

144. **重烘会「静默链上别的 runtime」**（第 33 轮 · M151 发布流程中实锤 · 缺陷 144）：
     `rebake_bin.sh` 的 `check_cache` 挑缓存目录的口径是"**目录 mtime 最新**"（M114-S3 治的是"挑 .o
     最多的"），而 **缓存命中不刷新目录 mtime** ⇒ 当前源码对应的 `rt_key` 目录若不是"最后写入的那个"，
     就链上**另一份 runtime**。实锤：以 M151 源码重烘的 `bootstrap/pxc` 里
     `strings | grep -c PX_GC_TRIGGER_BYTES` = **1**（该串只存在于**未提交的 M152** `runtime.c`），
     而 `PXSRC-…` 指纹 / `--check` / `--check-vm` **全绿**。三道修法：
     ① `tools/px rtcache` 直接输出**当前源码 rt_key** 的目录（不再猜 mtime），`check_cache` 用它；
     ② 缓存目录放一枚 `__rtfp.o`（`PXRT-<key>` 常量，随 `$CACHE/*.o` 自动链进产物；旧缓存就地补写）；
     ③ 三道门**读回产物里的 `PXRT-…` 与当前 rt_key 比对** ⇒「链对没有」可自动发现。
     教训：**能"自动挑选"的东西，也要能"自动核对挑对没有"** —— 把"挑 .o 最多的"改成"挑最新写入的"
     只是**换了一种猜法**；正确判据是"按当前源码算出的 key"。
     ↳ 同族陷阱（本轮实测）：`"$CACHE"*.o` 的 glob 在 `$CACHE` **无尾斜杠**时会拼成 `dir*.o` ⇒ 不展开
       （`ld: cannot find …*.o`）；而 gcc **链接失败会删掉输出文件** ⇒ `bootstrap/pxc` 被删（有
       `/tmp/pxc.bak-*` 兜底可恢复）。"选目录"和"拼路径"是两处都必须自洽的契约。

146. **`recv`/`send` 必须重试 EINTR —— 否则一次 GC 就能把在途请求打成"连接关闭 (4)"**（第 34 轮 · M152）：
     多线程/协程模式下 stop-the-world GC 会向工作线程投递**暂停信号**（`SIG_GC_STOP`）⇒ 阻塞在
     socket 上的线程被中断，系统调用返回 `-1/EINTR(4)`。`conn_recv` 的**明文分支**是裸 `recv()`，
     `sock_send_all` / `bi_tcp_recv` 也没有重试 ⇒ 旧实现把 EINTR 当"连接关闭"。
     **实锤**：m144 门 VM 轨在大请求体 + 字节阈值触发 GC 时稳定报
     `net: http_unix 请求失败: 连接关闭 (4)`（4 = EINTR）；`PX_GC_TRIGGER_BYTES=0` 则全绿
     —— **阈值本身没坏，是它把这条潜伏路径照出来了**。修法：四处加 `EINTR` 重试（对齐既有
     `px_tcp_recv_ex`）。教训：**"谁的错"要用最小差分定死**（关掉阈值 ⇒ 绿；打开 ⇒ 红），
     否则会把"GC 暴露的 I/O 缺陷"误判成"GC 自己的缺陷"而回滚掉正确的修复。


147. **字面量/空串/单字符都是「每次执行都新建对象」——分配率的第一大头**（第 35 轮 · M153 · 缺陷 145）：
     数字（4000 语句的整图编译）：总计 **2,840,232 次分配**，其中**字符串 2,739,131（96.4% / 513MB）**、
     列表 100,447、其余 < 700。热点调用点 `px_add`/VM-ADD **2,101,250（74%）**、`px_index` 270,038。
     微探针（200k 次迭代，基线 378）：`let s = ""` **+1/次**；`s[i%10]` **+1/次**；`c != ""` **+2/次**；
     `c >= "0" and c <= "9"` 各 **+1/次**；`"a" + str(i) + ", " + …`（5 段链）**+11/次**；
     `join("", parts)` **+7/次**（比链式省，但不是零）。三条根因与修法：
     · **VM 的 `PXOP_LOADK` 每次复制常量**（`vm_loadk` → `px_str(k->s)`）⇒ 新增 `px_str_const()`：
       按**指针**缓存（**仅限地址恒定的静态字面量** —— BC 镜像 K 表 payload；传栈/堆缓冲会取到旧值，
       接口注释里是红线）；`vm_loadk` 的 `PXK_STR` 改走它 ⇒ 每个字面量全进程只物化一次。
     · **`s[i]` 每次新建单字符对象** ⇒ `px_index` 的字符串分支在 `clen == 1` 时走 **256 项字节表**
       （零哈希零探测）；**多字节 rune 不走表**（回落普通分配）。
     · **`px_str_len(s, len<=0)` 每次新建** ⇒ **空串单例**。
     依据：`px_str_len` 的语义不变量「PX_STR 的 data 恒不可变，全仓无任何就地改写 `as.str` 的站点」
     ⇒ 同内容共享对象**不改变**相等/序/长度/索引任何语义（Go 的字面量合并同理）。
     **必须永久存活**（池里留悬垂指针比不池化更糟）⇒ 新增**钉住表** `g_pinned`，在**两条** GC 标记
     路径（并发 / 单线程）与全局表同批标记。效果：分配 **−83%**、字符串 **−86%**、**GC 3 次 → 0 次**；
     4000 语句 `--emit-c` **1.94s → 0.83s**、**token-cache 整图 34.38s → 9.81s**。
     边界：`object_id()` 对**同内容的短串/字面量**从此返回同一 id（契约只对容器/结构断言"不同对象
     不同 id"）；**C 轨（`px_str`）的字面量仍每次新建** —— 要走常量池得改发射器（改生成的 C 文本）；
     常量池**按指针**缓存 ⇒ **红线**：只允许地址恒定的静态字面量，传栈/堆临时缓冲会取到旧值；
     "物化 → 钉住"之间存在极短窗口，与既有单槽 `g_tmp_root` 的保护窗口同级（单线程/协程路径无影响）

148. **`PX_GC_ALLOC_STATS=1`：分配计数是「确定量」，可做硬阈值门**（第 35 轮 · M153）：
     `[px-alloc-stats] total=… bytes=…MB` + 按类型计数打到 stderr（进程退出时）。为什么必须有：
     本轮判据本身就是"分配率"，**没有这个数字就只能看时间** —— 而时间是受负载影响的伪判据；
     分配计数**完全确定**（同源码 + 同实现 ⇒ 同数字）⇒ 可以写死阈值（`examples/m153_alloc/`
     用 `total ≤ 800000`，实测 276；关闭池化的负控 1,200,263 ⇒ 判红）。

149. **`--emit-c` 输出冻结门（255 件）**（第 35 轮 · M153 建立）：`selfhost/emitc_freeze.sh --check`
     对 `selfhost/cases/*.px` + `examples/*/*.px` 逐个算 `--emit-c` 的 sha256，与
     `selfhost/golden/emitc_freeze.sha` 逐行比。为什么必须有：`--check` / `bootstrap_prove` 判的都是
     **compiler.px 自己**的产物 ⇒ "改了 runtime/优化了发射路径，却顺手**动摇了别的程序**的发射结果"
     这一类回归**没有任何门看得见**（第 34 轮起靠临时脚本手查）。**纯性能/runtime 改动 ⇒ 本门必须全绿**；
     有意改发射 ⇒ `--freeze` 重定基并在同一提交里说明原因。

150. **门里「取不到判据就跳过」是退化放行暗门**（第 35 轮 · 缺陷 147）：`rebake_bin.sh` 的 runtime
     缓存指纹门（M152 建，防缺陷 144）在"取不到当前 rt_key"时 `return 0` ⇒ 实测踩中：`runtime.c` 有
     **编译错误** ⇒ `tools/px rtcache` 失败 ⇒ 门打印"⚠️ 取不到当前 rt_key ⇒ 跳过 runtime 缓存核对"，
     **却仍输出"✅ 全件源码链一致 · runtime 链错 0 件"** —— 而当时入库件其实链的是**旧 cache 里的
     runtime**（正是 M152 要防的事，只是换了个入口）。修法：① 新增 `tools/px rtkey`（**只算 key 不编译**）；
     ② 门的"取不到"从**跳过**改为**判红**。教训：**门红不了，就不是门**；"判据取不到"与"判据不满足"
     在门里必须是**同一个出口**（都是红）。

151. **`"\u{0}"` 在字符串字面量里被静默丢弃**（第 35 轮登记 · **第 37 轮 M155 根治**，见事实 158–162）：
     实测 `len("\u{0}") == 0`、`"\u{0}a"` 长度为 1 且内容为 `a` —— **三轨一致**（VM / C / 解释轨输出
     相同），且 M153 改动前后一致 ⇒ 与池化**无关**。根因方向：K 表/M 表 payload 是 C 字符串 ⇒ 含 NUL
     的字面量在 `strlen` 语义下丢失。**要用 NUL 字符目前只能靠 `bytes()` 或运行期构造。**

152. **缺陷 149（本轮根治）：`join` 按 `strlen` 拼接 ⇒ 内嵌 NUL 处静默截断**（第 36 轮 · M154）：
     `bi_join` 旧实现 `sep_len = strlen(sep)` + 逐项 `px_to_string` 后 `strlen(ts)`，而 `+`
     （`px_add` 的 PX_STR/PX_STR 分支）与 Go `strings.Join` 都按**显式字节长**拼接 ⇒ 对含 NUL 的数据
     `join("", [s]) != s`（实测 `["A\0B"]` join 出 1 字节，`"A\0B" + ""` 得 3 字节；含 NUL 的**分隔符**
     同样被截）。**这不是纯风格问题**：本轮把编译器里 45 处 `out += x` 改成 `join`，若口径不同就是
     **行为变更**（改之前先修口径 = 让改写等价）。修法：`bi_join_item()` 统一取 (data, str.len)，
     仅非串类型回落 `px_to_string`（数字/布尔/null 无 NUL）。门里用
     `bytes_to_str(hex_to_bytes("410042"))` 造 NUL 串断言 `join` 与 `+` **逐字节一致**（负控 C：
     退回 `strlen` ⇒ 语义门 9 条 FAIL 必红）。
     **附口径备忘**：`len(s)` 是 **rune 数**（字符数）；字节数走 `bytes_len(bytes(s))`。

153. **小整数 `str()` 文本池**（第 36 轮 · M154）：`str(int)` 的常见入参是**下标/计数/操作数**
     （发射器每条指令 4 次），而 `px_fmt_value → xmalloc → px_str` 每次一条新对象（Go 的
     `strconv.Itoa` 同样每次分配）。新增 `px_str_int_pool()`：`[-64, 4096]` 区间按值索引、
     文本与 `px_fmt_value_raw` 的 `snprintf("%lld")` 分支**同一个格式串**（逐字节相同）、
     条目对象 `px_pin_obj` 钉住；域外走原路。**池化的是对象，字节语义零变化**；
     可见性边界与 M153 同款（同值 `str()` 返回同一对象 ⇒ `object_id` 相同）。

154. **多字节 rune 短串池**（第 36 轮 · M154）：M153 的单字节表只覆盖 ASCII，`s[i]` 落在**多字节
     rune**（中文/拉丁扩展/补充平面）时仍每次新建 —— 实测中文语料里 `peek`/`advance`/`ppos` 三处
     `g_src[g_pos]` 合计 **24 万次分配**。新增 `px_rune_pool(buf, clen)`：**2..4 字节**内容键（4 字节
     打包 + 长度）、4096 槽直接映射 + 4 探针、**不换出**（用尽即回落普通分配）；命中/未命中都返回
     可用对象（并发竞态下两线程可能各得一个，值语义不受影响）。**UTF-8 的 rune 最长 4 字节** ⇒
     该区间覆盖全部合法 rune，`\u{1d11e}`（4 字节补充平面）在门里已断言。

155. **常量池要按「指针」哈希，比较器要有「同一性」快路径**（第 36 轮 · M154）：常量池的键本来就是
     指针（`key == s`，且契约要求地址恒定），但哈希按**内容**做（每次求值一次 strlen + 全串 FNV）——
     `perf` 里 `px_str_const` 3.8% + `__strcmp` 3.3%。改指针乘性散列后查找**与内容无关**（桶分布还更好：
     同一内容不同地址的字面量不再挤一条探测链）。同理，池化之后「同一对象」是高频情形 ⇒
     `px_eq`/`px_ne` 的串类分支与 `compare_values_raw` 顶部加**同一性短路**（同对象 ⇒ 0/相等；
     长度不同 ⇒ 不等；其余才 memcmp）—— 这既是性能，也是把 M153/M154 的池化**收益兑现**的一步。

156. **钉住表的去重是线性扫描 ⇒ 池化把 O(n) 推成 O(n²)**（第 36 轮 · M154）：`px_pin_obj` 原本每次
     线性扫全表去重。M153/M154 的池化把钉住表从「几百」推到「数千」（字面量 + 256 单字符 + 4096 rune
     + 4161 小整数文本）⇒ 每次 pin 数千次比较。实测 `px_pin_obj` 占整图编译 **10.0% 采样**（榜第二，
     仅次于解释循环）。修法：开放寻址指针集合（16K 槽、128KB、惰性分配），集合满/探针用尽时**退化为
     直接追加**（数组里重复一项、标记同一对象两次无害）。改后该符号退出 `perf` 榜。
     **教训：任何"每次追加前全表查重"的结构，都要问一句"这张表会被谁推到多大"。**

157. **分配预算门的阈值必须贴着实测（1.2×），否则负控会漏网**（第 36 轮 · M154）：本轮首次开门
     实测基线 401,270、阈值取 65 万（1.6×）⇒ **负控 A（关掉 rune 池，+20 万 = 601,266）漏网**，
     门自己报了"未判红"。分配计数是**确定量**（同源码 + 同实现 ⇒ 同数字），所以阈值贴到
     1.2×（48 万）是安全的，且能拦住任一处退回（关 rune 池 601,266 / 关整数池 1,000,270）。
     **教训：给「确定量」留 1.6 倍余量，等于自己把门的判别力砍掉一半。**

158. **`"\u{0}"` 的两层成因与根治（第 37 轮 · M155 · 缺陷 148）**：① **发射层** —— `cg_escape_str`
     的 NUL 分支写的是 `_out.append("")`（M-B8 为与 Rust 版逐字节一致而立，那时 C 串按 strlen 重造，
     NUL 本就表达不出来），且常量一律 `px_str("…")`（C 串）⇒ 首个 0x00 即断。② **编译器自身源码**
     里的 `"\u{0}"`（`codegen.px`/`pxlexer.px`/`it_util.px` 共 5 处）也被**旧编译器**编成空串 ⇒
     `rust_unescape` 的 `_out.append("")`、`char_debug` 的 `c == ""` —— **三轨一致地错**。修法：
     （a）源码改**运行时构造** `nul_char()`（`bytes_to_str(int_to_bytes(0, 1))`）—— 不依赖被修的行为，
     **旧编译器编新源码也正确**（自举安全的必要动作）；（b）发射：含 NUL 的常量 → `PX_STR_LIT(lit)` /
     `PXK_STR_LIT(lit)`，长度 = `(int)sizeof(lit) - 1`（**编译期**算，发射器不需要字节长函数）；
     （c）`vm_loadk`：K 项 `i > 0` ⇒ `px_str_const_n(s, i)`（指针键常量池复用 ⇒ 热路径零分配）。
     **不含 NUL 的常量发射文本一字不变**（`i == 0` 仍走 `px_str_const`）。
159. **`\000` 必须写满三位八进制（第 37 轮 · M155）**：C 的八进制转义**最多吃 3 位** —— `"a\01"` 是
     `0x01` 而不是 `NUL` + `'1'`（实测语料 `"\u{0}1"` 正是这条的判据）。发射器恒写三位 ⇒ 后随数字
     **必停**。负控 C 用「长度多算 1」守长度判据，语料里 `lit.octal_amb.*` 三条守这条。
160. **`print` / 渲染族的长度口径（第 37 轮 · M155 · 缺陷 150）**：`px_fmt_value(v)` 只回 `char*`，
     调用方一律 `strlen`/`printf("%s")` ⇒ 内嵌 NUL 处截断（`print("A\0B")` 只写 `A`）。现分
     `px_fmt_value_n(v, &len)`（全长）与 `px_fmt_value`（兼容包装）；容器渲染里 **dict 的排序项**
     必须「文本 + 长度」随行（qsort 的比较器用 `PxDisp`，排序键仍是 `strcmp` ⇒ 口径不变）；
     `bi_str` 的非串分支用 `px_str_len(s, n)`。**判据要用 `cmp` 比原始字节**，别用 `grep` 看。
161. **`json_stringify` 与 `json_stringify_go` 是两条口径（第 37 轮 · M155 · 缺陷 152）**：
     `json_stringify_go` = **Go 保真**（NUL → `\u0000`、0x01 → `\u0001`、`<>&` → `\u003c…`、键排序、
     float 最短往返）；`json_stringify` = 语言自有变体（**控制字符裸输出**，可产出非法 JSON）。
     M155 前 `json_stringify` 还会在首个 NUL **丢掉整条尾巴**（`jout_escape` 走 C 串 + 尾串 `px_str`），
     与「裸输出」契约不符 ⇒ 已改长度感知（`jout_append_n` + `px_str_len(o.buf, o.len)`）。
     **要移植 Go 代码、要给别人解析的 JSON，一律用 `_go`** —— 本轮我的第一版语料就用错了函数
     （断言直接红），这类「选错函数」的错误在门里表现为**语义断言失败**而不是报错。
162. **`$BUILD` 里的陈旧 runtime 副本会遮蔽 `#include "runtime.h"`（第 37 轮 · M155 · 缺陷 151）**：
     `#include "x.h"` 的搜索顺序是「**包含者所在目录** → `-I` 列表」，而 `selfhost/build/` 里躺着
     一份 2025-09-05 的 `runtime.{c,h}` 副本（某次以该目录为输出的构建留下的）⇒ **盖过**
     `-I"$CACHE"`（按 rt_key 现生成的当前 runtime 源）⇒ 编译用的是**旧头**。实测症状：新发射的
     `PX_STR_LIT` 在生成的 C 里"未知"（pxfmt/pxdoc 直接编译失败）；此前 10 件"成功"只是因为它们
     的闭包里没有含 NUL 的字面量 ——**坑一直在，只是从未被照出来**。根治：重烘前删除同名副本
     （`shadow_clean`）。与缺陷 144（链错 runtime）同族：**「用哪一份运行时」必须可判定、可发现**。
