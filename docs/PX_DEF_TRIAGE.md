# 第三方缺陷登记 · 逐条判定（PX-DEF triage）

> **来源**：`github.com/banshanhanfu/registry-px` 的 `docs/语言缺陷.md`（该仓库写生态库过程中
> 暴露的 PuXian 语言面问题登记，编号 `PX-DEF-001…017`）。
> **立场**：那是**对方视角的观察**，不是我们的结论。每一条都必须在本仓**三轨**
> （解释轨 `px run` / VM 轨 `px build` 默认 / C 轨 `PX_BUILD_ENGINE=c px build`）上复测，
> 才能定性为「真缺陷 / 文档缺口 / 设计如此」。本文件是**我方判定表**，随每轮里程碑更新。
> **基线**：PuXian `v0.2.0-m185`（x86_64）—— M184 那轮的表已按 `--rebake-all` 后的**新入库件**重测。
> **复现法**：探针落 `/tmp/<milestone>/probes_pxdef/*.px`，逐文件跑三轨取 `rc` / 标准输出 /
> 错误码 —— 见 `examples/m184_num_fs_strict/verify.sh` 的 `run3()` 写法。

## 一、判定分类（三档，互斥）

| 档 | 含义 | 处置 |
|---|---|---|
| **真缺陷** | 三轨**分叉**，或三轨一致但**静默给错值 / 静默丢失数据 / 文档与实现不符** | 改语言面（发射/运行期），配门 + 负控，重定基 |
| **文档缺口** | 三轨一致且语义**可自洽**，只是**没有写清楚**（速查表/规范未覆盖） | 补 `docs/PUXIAN_CHEATSHEET.md`（必要时 `docs/spec.md`），**不改行为** |
| **设计如此** | 三轨一致，且是**有意选择**（多为"响亮优于静默"） | 在速查表写明口径与绕行 |

> 判据口径（M163/M166/M179/M181/M184 一贯）：**响亮优于静默**。
> 三轨一致地"静默给错值"同样是缺陷 —— 三方库会把它当成"语言如此"而绕行，代价留给了每个用户。

## 二、判定表（2026-09-23 · m184 基线）

| ID | 主题 | 三轨实测（m184） | 判定 | 状态 |
|---|---|---|---|---|
| PX-DEF-001 | 闭包捕获并**修改**外层局部，编译轨失败 | 三轨均得正确结果 | 真缺陷 | ✅ **M160 已修**（第三方文档未更新） |
| PX-DEF-002 | `int()`/`float()` 是宽容前缀解析 | 修前：三轨一致宽容（`int("12ab")`=12 / `int("abc")`=0 / `float("1.2.3")`=1.2），且 `int("")` **三轨分叉**（解释轨报错、编译轨给 0） | 真缺陷（静默错值 + 分叉） | ✅ **M184 已修**（严格解析，27 用例 × 三轨同码同文） |
| PX-DEF-003 | `Result` 无 `unwrap_err()` | 修前：三轨一致 `R1007 类型 result 没有方法 'unwrap_err'`（`ok()/err()` **早已存在**，是**查询**语义 —— 对侧给 `null`） | 真缺口（对称性 · 第三方未发现 `err()`） | ✅ **M185 已修**（新增 `unwrap_err()`，Ok 上调用响亮 `R1004`；速查表事实 213） |
| PX-DEF-004 | `"s" * n` 字符串乘法双模式不一致 | 三轨一致 `"0" * 3 = "000"` | 真缺陷 | ✅ **M179 已修**（本轮复测确认） |
| PX-DEF-005 | `mkdir()` 成功返回 `null` ⇒ `if mkdir(d):` 永远为假 | 修前：三轨一致返回 `null` | 真缺陷（静默判假） | ✅ **M184 已修**（返回 bool，见 §三） |
| PX-DEF-006 | 解释器不支持并发原语 | 设计性（`MINI_SUBSET` 排除） | 设计如此 | 速查表已写明（并发库需编译模式） |
| PX-DEF-007 | 无键 `d["k"]` 报 `R1008`（而非 null） | 三轨一致报 `R1008` | 设计如此（响亮优于静默） | 速查表已写明（先 `has()` 守卫） |
| PX-DEF-008 | `type("")` 返回 `"string"` 而非 `"str"` | 三轨一致：`string/int/float/bool/null/list/dict/tuple/bytes/result/function` | 文档缺口 | 📄 **文档已补** |
| PX-DEF-009 | `regex_match` 是**完整匹配**（非搜索） | 三轨一致：`regex_match("a@b", "xa@by")` = false；`regex_search(...)` 返回 `{start,end,match,groups}` | 文档缺口 | 📄 **文档已补** |
| PX-DEF-010 | `pbkdf2_sha256` 的 salt 按**字符串字节**处理（不 hex 解码） | 三轨一致；传 `hex_to_bytes(...)` 时与 RFC 标准逐字节相同 | 文档缺口 | 📄 **文档已补** |
| PX-DEF-011 | `sorted` 同键复合元素顺序双模式不同 | 三轨一致 `[[1,"a"],[1,"a2"]]` | 真缺陷 | ✅ **M162 已修**（值比较 + 稳定排序） |
| PX-DEF-012 | `bytes(n)` 语义是"数字转字符串字节" | 三轨一致：`bytes(10)` = 2 字节；`bytes(0)` = 1 字节（`"0"`） | 文档缺口（语义易误读） | 📄 **文档已补**（造 N 字节零缓冲用 `bytes_concat` 循环） |
| PX-DEF-014 | `list_dir` 无安全变体（不存在目录 ⇒ 进程终止） | 修前：三轨一致 `px_error` 终止 | 真缺陷 | ✅ **M184 已修**（新增 `list_dir_opt`） |
| PX-DEF-015 | `len()`/`s[i]` 按 **rune**、`s[a:b]` 切片按 **字节**（混用） | **复测否定**：三轨**全程 rune** —— `len("你好")`=2、`s[1]`=`"好"`、**`s[0:1]` = `"你"`（3 字节，完整首字符）**、`s[0:2]` = `"你好"` | **文档缺口**（对方基线 m155 时切片确按字节，其后已统一；遗留的是 `stdlib/strings.px` 注释与速查表旧措辞） | 📄 **M185 复核**：行为已一致，**不改行为**；速查表事实 52/64 与 211 写明"三面全程 rune" |
| PX-DEF-016 | `sha256` 返回 hex 串；`sha256(sha256(s))` 不是标准双哈希 | 三轨一致：`sha256("hello")` = 64 hex；hex 串再哈希 = `d7914fe5…`，原始字节双哈希 = `9595c9df…` | 文档缺口 | 📄 **文档已补** |
| PX-DEF-017 | `bytes(chr(n))` 对 `n>=128` 是 UTF-8 编码（非原始字节） | 三轨一致：`bytes(chr(185))` = 2 字节；`hex_to_bytes("b9")` = 1 字节；二者往返均可保留 0-255 | 文档缺口（str/bytes 双模型） | 📄 **文档已补** |
| **新 1**（我方新登记，内部编号 **203**） | **`bytes` 的索引/切片/迭代三面** | 修前**三轨四种行为**：切片=解释轨缺（VM/C 有）；索引=**三轨都缺**；迭代=解释轨"不可迭代"、VM/C"不支持索引" | **真缺陷**（三轨分叉 + 文档与实现不符） | ✅ **M185 已修**（`b[i]`→int / `b[a:b]`→bytes / `for x in b`→逐字节 int，三轨同码同文；速查表事实 211） |
| **新 2**（内部编号 **204**） | **同一判定规则两处实现**（解释轨 `i_str_is_int_literal`/`i_str_is_float_literal` vs 原生 `px_str_to_i64`/`px_str_to_f64`） | 两者当时接受集**恰好一致**（未爆）；但 `is_int_str(s) ⇔ int(s) 成功` 这条等式只能靠"两边都写对" | **结构性隐患**（M184 想根除的正是这类） | ✅ **M185 已收口**（删掉两份翻版，改调原生判定器 ⇒ 等式由**构造**保证） |
| **新 3**（内部编号 **205**） | **`px_to_string` 四重病灶**（容器恒 `"<object>"` / 每次调用打印 stdout / `tmpfile()` 从不 fclose（fd 泄漏）/ 调用方 `xfree` 到字符串字面量） | 实测 `join(",", [[1,2],[3,4]])` = `"<object>,<object>"` + 往 stdout 塞 4 段容器文本 + 漏 4 个 fd；解释轨 = `"[1, 2],[3, 4]"` ⇒ 分叉 | **真缺陷**（静默错值 + 三轨分叉 + 资源泄漏 + UB） | ✅ **M185 已修**（容器走 `px_fmt_value_n` 同一渲染器 + 线程局部单槽；速查表事实 214） |

> ⚠️ 编号体系说明：`PX-DEF-xxx` 是**第三方仓库**的登记号（他们最后一条是 017，无 013 —— 跳号）；
> 「新 1」是本文件**新登记**的条目（其**内部缺陷编号**为 **203**，与 `CHANGELOG.md` 的编号体系一致）。
> 两套编号并存，引用时写清是"对方登记号"还是"内部缺陷号"。
>
> `PX-DEF-013` 在对方仓库中**编号不存在**（写库时跳号）—— 如实登记，不臆造。

## 三、本轮（M184）收口的三条：改了什么、怎么证明

### PX-DEF-002 · `int()`/`float()` 严格解析
- 修前：`atoll`/`atof`（C atoi 家族）⇒ `int("12ab")`=12、`int("abc")`=0、`int("1e3")`=1、
  `int("0x10")`=0、`float("1.2.3")`=1.2、`float("x")`=0.0 —— **静默错值**；
  且 `int("")` 解释轨报错 / 编译轨给 0（**三轨分叉**）。
- 新语义：`trim`（space/`\t`/`\n`/`\r` 四种，**与既有 `trim()` 逐字符同集**）后必须**整体合法**
  ⇒ 否则 `R1002 无法将 '<原文>' 转为 int/float`。
  `int` 仅十进制；`float` 十进制 + 指数 + `inf/infinity/nan` 白名单（`0x10` 十六进制浮点显式拒绝）。
- 实现：`runtime/runtime.c` 新增 `px_num_span` / `px_str_to_i64` / `px_str_is_ieee_special` /
  `px_str_to_f64`，`bi_int`/`bi_float` 改走它们；解释轨 `selfhost/ibuiltin.px` 加**同口径前置校验**
  （`i_str_is_int_literal` / `i_str_is_float_literal`，用 `regex_match` 完整匹配语义）以便报错带 `行:列`。
- 门：`examples/m184_num_fs_strict/` 第 ①（21 断言）②（**27 用例 × 三轨**，词条逐字相同）层。

### PX-DEF-005 · `mkdir` 返回 bool
- 修前恒 `return px_null()`；每层 `mkdir(2)` 返回值被丢弃 ⇒ "创建失败"完全无声。
- 新语义（对齐 Go `os.MkdirAll`）：成功**或**已存在且确实是目录 ⇒ `true`；
  同名**非目录**（普通文件）⇒ `false`（否则 `mkdir("/etc/passwd")` 会假真）；任一层失败 ⇒ `false`。
- 门：第 ③ 层（6 条语义断言，其中 `IF_MKDIR_TRUE` 直接盯住"`if mkdir(d):` 不再永假"）。

### PX-DEF-014 · `list_dir_opt`
- 新增 `list_dir_opt(path) → Ok(list) | Err(msg)`（对齐 `json_parse_opt` / `read_file_opt` 先例）；
  `Err` 文本 `fs: 读取目录失败 <path>: <strerror> (os error <n>)`。
- `list_dir` **保持**终止语义（响亮优于静默）——安全变体是**增量**，不是替换。
- 门：第 ④ 层（5 条语义断言 + `before` 必须先打印，证明"没被终止"是流程性的）。

## 四、下一轮（M185）计划面

1. **PX-DEF-015 + PX-DEF-200 一族**：`str` 的双模型（rune 索引 vs 字节切片）与 `bytes` 切片分叉
   —— 需先定**一条真相**（候选：索引/切片/`len` 三者统一到**rune**，字节面全部走 `bytes_*`；
   或反之统一到**字节**并给 rune 面新名字），再改三轨 + 重定基 + 全量门。
   注意这会动 `selfhost/ival.px` 的索引/切片分支与 `cg_expr` 的切片发射 ⇒ 必须重定基
   `selfhost/golden/*.c` 与发射冻结门（M162/M163/M181 的同款纪律）。
2. **PX-DEF-003（`unwrap_err`）**：`Result` 补 `unwrap_err()`（对齐 Rust `Result::unwrap_err`），
   三轨同做 — 成本低、去绕行代码。
3. **PX-DEF-012（`bytes(n)`）**：评估新增 `bytes_fill(n)` / `zeros(n)`（不破坏现有 `bytes(n)`），
   并在 lint 层对"`bytes(<int 字面量>)`"给出提示。

## 五、给上游的反馈（人工动作，需账号权限）

`registry-px/docs/语言缺陷.md` 中 **001 / 004 / 011 / 002 / 005 / 014 六条已在官方侧修复**，
建议对方更新状态列并在 CI 里加"对最新里程碑复跑"的步骤（否则登记表会长期滞后于语言演进）。
本仓的**权威状态**始终以本文件 + `CHANGELOG.md` 为准。

## 六、生态影响面（M184 使 `int()` 变严格之后的连带效应）

「语言变严格」不是零成本的 —— 它**必然**打到"依赖旧宽容行为"的库。M184 的全盘排查与处置：

### 6.1 第三方 `registry-px`：回归 33/34（修前 34/34）

| 库 | 失败形态 | 真因 | 建议修法 |
|---|---|---|---|
| `passhash` | `pass_verify` 运行期 `R1002: 无法将 'abc' 转为 int` | `registry/passhash/0.1.0/passhash.px:36` 的 `var iters = int(parts[1])` 直接吃 hash 串的迭代次数字段（**外部可构造**） | 先守卫：`if not cli_is_int_str(parts[1]): return false`；或用 `stdlib/go_strings.px::go_parse_i64(s) → Ok/Err` |

> **这不是"官方改坏了库"**：修前那个静默 `0` 会把迭代次数变成 0 —— 密码校验语义**已被破坏**，
> 只是没人报错。**响亮报错是修复，不是回退**；库需要做的是把 err 通道显式化
> （这正是 PX-DEF-002 里"严格校验函数自行实现"的由来）。

### 6.2 我方 `stdlib/`：逐处定性（全盘 grep `int(`）

| 位置 | 形态 | 定性 | 处置 |
|---|---|---|---|
| `cookiejar.px` `int(av)`（`Max-Age`） | **外部输入直达**（`Set-Cookie` 头） | **真风险**（`Max-Age=abc` ⇒ 新攻击面） | ✅ **M184 已修**：本地 `cj_is_int` 守卫 + RFC 6265 容错口径 |
| `semver.px` 三处 `int()` | 版本串分量 | **实测安全**（`sv_parse("a.b.c")` 返回 `null`；`int()` 只在已验证处调用） | 不改 |
| `pxml.px` `int(numstr)` | 扫描器自造数字文本 | 不可达 | 不改 |
| `fmtlexer.px` `int(clean)` | 源码数字字面量 | 已由前导零修复覆盖 | 不改 |

### 6.3 通用能力缺口（⇒ M185）

"这串是合法整数吗"的判断，**第三方自己写了一遍**（`cli_is_int_str`），我方 `cookiejar` 本轮也写了一遍
⇒ **语言层缺内置**。M185 计划加 `is_int_str(s)` / `is_float_str(s)`（**与严格解析同口径**，三轨同做），
再把两处本地 helper 收敛进去。**语言变严格的同时，必须给出"如何安全地做宽松解析"的官方入口。**

## 四、本轮（M185）收口的三条：改了什么、怎么证明

### 内部 203 · `bytes` 三面统一（对应对方 PX-DEF-015 一族 + 我方新登记）
- 修前**三轨四种行为**（见上表）；速查表事实 22/35 却写着 `b[i]` 可用 ⇒ **文档与实现不符**。
- 新语义（对齐 Python `bytes` / Go `[]byte`）：`len(b)`=字节数 · `b[i]`=**int 0..255**（负索引从尾，
  越界 `R1003 索引越界: i (len=n)`）· `b[a:b]`=**bytes**（按字节，支持负边界/步长/反向）·
  `for x in b`=逐**字节值** int。
- 实现：`runtime/runtime.c::px_index` 新增 `PX_BYTES` 分支（`px_slice` 本就支持 bytes；
  VM 轨 `PXOP_ITERAT`→`px_iter_at`→落回本函数 ⇒ 迭代一并修好）；解释轨 `selfhost/ival.px`
  的 `i_index`/`i_slice`/`i_iter` 各补分支（切片复用 `i_slice_indices` ⇒ 与 list/tuple/string 同一条下标路径）。
- 门：`examples/m185_bytes_face/` 第 ① 层（25 断言）+ 第 ④ 层（越界/负越界/非整数索引 三轨同码同文）。

### 内部 204 · 「同一规则两处实现」的收口
- 解释轨 `selfhost/ibuiltin.px` 里 `i_str_is_int_literal` / `i_str_is_float_literal` 是
  **原生谓词的翻版**（M184 之前为"报错带 `行:列`"写的）。**两处实现同一规则 ⇒ 结构性分叉隐患**。
- 本轮**删掉这两份**，`i_builtin_int`/`i_builtin_float` 改调原生 `is_int_str`/`is_float_str`
  ⇒ `is_int_str(s) ⇔ int(s) 成功` 由**构造**保证，不再依赖"两边都写对"。
- 门：第 ② 层（60 行；`E1..E6` 六条等价性定点 + `I/F` 各 21/24 个用例 × 三轨一致）。

### 内部 205 · `px_to_string` 四重病灶
- ① 容器恒返回硬编码 `"<object>"`（`join`/`write_file`/`env`/HTTP 体/`unwrap` 消息五处全中）
  ② 每次调用把值**打印到 stdout** ③ 每次调用 `tmpfile()` **从不 `fclose`**（fd 泄漏）
  ④ 调用方 `xfree` 到**字符串字面量**（UB）。详见 `CHANGELOG.md` · M185 §四。
- 修法：容器走 `px_fmt_value_n`（与 `str()`/`print` 同一渲染器）+ **线程局部单槽**；
  删 tmpfile / 删非法 `xfree`；`route_normalize` 另备 TLS 缓冲。
- 门：第 ③ 层（`A1..A4` 容器渲染 + **fd 增量 = 0** + **stdout 行数恰为 12** 证明无副作用）。

### 顺带：对方登记的 `PX-DEF-015` 被**复测否定**
- 对方基线 `m155` 时"切片按字节"确实成立；**其后某轮已统一为 rune**（本轮三轨复测：
  `s[0:1]` = `"你"`、`bytes_len(s[0:1])` = 3 ⇒ 与 `len`/`s[i]` **同轴**）。
- ⇒ 定性从"真缺陷"改判为 **文档缺口**（遗留的是 `stdlib/strings.px` 的注释与速查表旧措辞）。
  **如实记录：这条是"对方登记时对、现在已不成立"，不是我方没修。**

## 四、M186 复测（第三方 `PX-DEF-018…030` · 基线 `v0.2.0-m185`）

> 触发：`registry-px` 在 09-22/09-23 猛推 —— **库 34 → 53**（新增 T2 专项 qrcode/pdf/pg/mysql/xlsx、
> T3b/T3c 一批），缺陷登记也从 **017 涨到 030**。本轮把**全部 53 库的测试**在本机三轨上跑了一遍
> （普查脚本 `/tmp/m186/census.sh`，结果 `/tmp/m186/census.txt`），再逐条复测新登记。
> **普查口径**：编译轨 = `tools/px build`（默认 **VM 轨**）成功 + 产物可执行且 rc=0；
> 解释轨 = `tools/px run` rc=0；fixture 按对方 `tests/` 需要预置（`/tmp/dotenv_sample.env`、`/tmp/globd/…`）。

### 4.1 全量普查（53 库 · m185 入库件）

| 轨 | 通过 | 说明 |
|---|---|---|
| 编译轨（VM） | **50/53** | 3 件失败全是**需要真实服务端**的集成测试（`pg` / `mysql` / `passhash`） |
| 解释轨 | **47/53** | 6 件失败：并发 2（`concurrent_map`/`workerpool`，**解释器设计性不支持并发**）· DB 3（同上）· `qrcode` **解释轨超时**（对方 PX-DEF-024 的性能事实：单码 8 掩码罚分 ≈36s） |

逐条：
| 测试 | 编译轨 | 解释轨 | 定性 |
|---|---|---|---|
| `concurrent_map` / `workerpool` | PASS | FAIL | **设计如此**：`pxi` Mini 子集无 spawn/chan（PX-DEF-006） |
| `pg` / `mysql` | BUILT-RUNFAIL | FAIL | 需要**真实 PG13 / MariaDB**（对方凭真实库自证；本机无服务 ⇒ 属**环境缺口**，非语言问题） |
| `passhash` | BUILT-RUNFAIL | FAIL | **M184 记录的后果**（`int()` 严格化 ⇒ 库内 `int(parts[1])` 直接吃畸形 hash 串）—— 已登记，待对方按新语义修 |
| `qrcode` | PASS | FAIL | 解释器**性能**（PX-DEF-024），非语义缺陷 |
| 其余 **47** 件 | PASS | PASS | 双轨通过（含 base58 / checksum / tar / glob / dotenv / bytes_pack 等二进制与解析类） |

### 4.2 新登记逐条判定（018–030）

| 对方号 | 主题 | 三轨实测（m185） | 判定 | 处置 |
|---|---|---|---|---|
| PX-DEF-018 | `int_to_bytes` 缺参：解释轨报晦涩 `R1003 索引越界` | 解释轨 `R1003: 索引越界: 1 (len=1)` vs 编译轨 `R1002: int_to_bytes 需要 (n, size[, endian[, signed]]) 参数` | **真缺陷**（诊断分叉） | ✅ **M186 修**（内部 **208**） |
| PX-DEF-019 | `bytes_to_int` 的 `endian` 参数解释轨被忽略 | 解释轨 `258`（大端）vs 编译轨 `513`（小端） | **真缺陷**（**静默错值**） | ✅ **M186 修**（内部 **206**） |
| PX-DEF-020 | `int_to_bytes` 的 `signed=true` + 负值解释轨异常 | 解释轨 `302e30`（"0.0"）vs 编译轨 `fffb` | **真缺陷**（**静默错值**） | ✅ **M186 修**（内部 **207**） |
| PX-DEF-021 | `zip_pack` / `zip_unpack` 解释轨缺失（`R1001 未定义变量`） | **复测否定**：三轨**都**解析到该 native；两参调用 `zip_pack(dict, path)` 端到端三轨输出**逐字节一致**（pack=true / unpack=2 / 读回内容相同） | **不复现** | 📄 记录（对方基线 m182，其后已可用；且对方的调用形态是**一参版**，实际签名是 2 参） |
| PX-DEF-022 | list 切片是浅拷贝（内层共享） | 三轨一致（与 Python/Go 一致：`var b = a` 是引用、`a[:]` 是浅拷贝） | **设计如此** | 📄 速查表写明（需深拷贝手写循环） |
| PX-DEF-023 | `Err` 嵌套字符串化冗长（`Err(a: Err(b))`） | 三轨一致 | **设计如此**（展示形态） | 📄 速查表写明 |
| PX-DEF-024 | 解释器循环性能极差（QR 罚分 ≈36s vs 编译 <1s） | 本次普查**实测复现**（`qrcode_test` 解释轨 300s 超时） | **设计性**（解释器非性能轨） | 📄 已写明（重循环/大矩阵用编译产物验收） |
| PX-DEF-025 | 字符串无 `.find()` 方法（子串定位） | 三轨一致：`"abc".find("b")` ⇒ `R1007 类型 string 没有方法 'find'`；`std.strings.index_of` 可用 | **真缺口**（方法面） | ⏳ **候选 M188**（与 027 同族：方法面收口，需三轨 + 重定基） |
| PX-DEF-026 | 无 `sha1` native（MySQL `mysql_native_password` 必需） | 三轨一致 `R1001 未定义变量: 'sha1'`（**一致的缺口**，非分叉） | **真缺口** | ⏳ **候选 M187**（补 `sha1`/`sha1_bytes`；对侧已用纯 .px SHA1 自证，非阻塞） |
| PX-DEF-027 | 字符串方法面窄（无 `strip`/`has_prefix`/`has_suffix`） | 三轨一致 `R1007`；函数式替代（`trim`/`starts_with`/`ends_with`）齐全 | **真缺口**（同 025 族） | ⏳ **候选 M188**（与 025 合并做） |
| PX-DEF-028 | C 风格 `for (;;)` 不支持 | 三轨一致 `E1001 非法字符: ';'` | **设计如此** | 📄 速查表写明（无限循环写 `while true:`） |
| PX-DEF-029 | `bytes_get` 越界返回 `null`（不报错） | 三轨一致返回 `null`；后续比较会报 `R1002 无法比较: null < int` | **文档缺口** | 📄 速查表写明（解析前 `off >= bytes_len(b)` 守卫） |
| PX-DEF-030 | `bytes_to_int` 读 `0xFFFFFFFF` 返回**无符号** 4294967295 | 三轨一致（`signed=true` 才给 -1） | **文档缺口** | 📄 速查表写明（wire protocol 判负长度用 `signed=true` 或 `> 2147483647`） |

> **判定口径**：`PX-DEF-021` 属「对方登记时对、现在已不成立」；`018/019/020` 三条是**同一根因**的三种表现
> （解释轨转发 native 时**只透传前 1~3 个实参**）—— 一条修法收三条，见 `CHANGELOG.md` · M186。

### 4.3 M186 收口的三条（内部 206/207/208）

| 内部号 | 对方号 | 修前 | 修后 |
|---|---|---|---|
| **206** | PX-DEF-019 | `bytes_to_int(0102, "little")` 解释轨 **258** | 三轨 **513** |
| **207** | PX-DEF-020 | `int_to_bytes(-5, 2, "big", true)` 解释轨 **`302e30`（"0.0"）** | 三轨 **`fffb`** |
| **208** | PX-DEF-018 | `int_to_bytes(1)` 解释轨 **`R1003 索引越界`** | 三轨 **`R1002 int_to_bytes 需要 (n, size[, endian[, signed]]) 参数`** |

- 修法：`selfhost/ibuiltin.px` 两处转发**全量透传 + 参数个数前置校验**（错误走 `i_r1002` ⇒ 与 C 层同码同文）；
  解释轨的 `signed`/`endian`/多余实参从此**不再被静默丢弃**。
- 门：`examples/m186_native_passthrough/`（33 行值语义 × 三轨逐字节一致 + 拒绝侧 4 例 × 三轨同码同文 +
  负控 A/B/C 各自独立判红 —— 解释轨改动用 **dev 解释器**，与安装件隔离）。

## 五、M188 收口（第 66 轮 · 第三方 `PX-DEF-026/016/012/017/025/027`）

> 复现基线：`v0.2.0-m187` 入库件；方法：逐条在**解释轨 / VM 轨 / C 轨**上跑最小用例（`/tmp/m188/probe.px`）。

| 对方号 | 主题 | 三轨实测（修前） | 判定 | 处置 |
|---|---|---|---|---|
| PX-DEF-026 | 无 `sha1` | 三轨一致 `R1001 未定义变量: 'sha1'` | **真缺口**（一致的缺口，非分叉） | ✅ **M188 修** |
| PX-DEF-016 | `sha256` 只给 hex（双哈希陷阱） | 三轨一致：`sha256(sha256("abc"))` = `d7914fe5…` | **文档/构造器缺口** | ✅ **M188 补 `sha256_bytes`** |
| PX-DEF-012 | `bytes(10)` 非 10 字节 | 三轨一致：`bytes_len(bytes(10))` = 2 | **文档/构造器缺口** | ✅ **M188 补 `bytes_zeros`** |
| PX-DEF-017 | `chr(n≥128)` 非原始字节 | 三轨一致：`bytes_to_hex(bytes(chr(185)))` = `c2b9` | **文档/构造器缺口** | ✅ **M188 补 `byte`** |
| PX-DEF-025 | 无 `.find()` | 三轨一致 `R1007 类型 string 没有方法 'find'` | **真缺口**（方法面） | ✅ **M188 修** |
| PX-DEF-027 | 无 `.strip()`/`.has_prefix()`/`.has_suffix()` | 三轨一致 `R1007` | **真缺口**（方法面） | ✅ **M188 修** |

- **统一口径（本轮新增，速查表事实 218/219）**：
  · `sha1(data)` → 40 字符小写 hex · `sha1_bytes(data)` → 20 字节（族口径与 sha256/md5 一致）；
  · `sha256_bytes(data)` → 32 字节（**"哈希的哈希"必须用它**，否则落在 hex 文本上）；
  · `bytes_zeros(n)` → n 个 `0x00`（`bytes(10)` 是 "10" 的 2 字节）· `byte(n)` → 原样单字节（n∈0..255）；
  · `s.find(sub)` 返回 **rune** 下标（`"中文abc".find("abc")` = 2）· `-1` 不存在 · `0` 空子串；
    `.strip`/`.has_prefix`/`.has_suffix` 分别直通 `trim`/`starts_with`/`ends_with`（**只加路由**）。
- 门：`examples/m188_std_face/`（4 层 + 4 负控，`M188-VERIFY-OK`）。

### 5.1 仍开放（下一批候选，按严重+紧急排序）

| 内部号 | 对方号 | 主题 | 状态 |
|---|---|---|---|
| — | PX-DEF-024 | 解释器循环性能（QR 罚分 ≈36s） | 设计性（解释器非性能轨），已在速查表写明 |
| — | PX-DEF-029 | `bytes_get` 越界返回 `null`（不报错） | ❌ **复测改判为真缺陷** ⇒ ✅ **M189 修**（内部 **211**，见 §七） |
| — | PX-DEF-030 | `bytes_to_int` 默认**无符号**读取 | **文档缺口**（`signed` 形参**一直存在**，传 `true` 即得 -1） |
| — | PX-DEF-022/023/028 | 浅拷贝 / Err 嵌套展示 / 无 C 风格 `for(;;)` | **设计如此**（M189 复测确认三轨一致，见 §七.2） |
| — | PX-DEF-024 | 解释器循环性能（QR 罚分 ≈36s） | 设计性（解释器非性能轨） |
| — | PX-DEF-021 | `zip_pack`/`zip_unpack` 解释轨缺失 | ❌ **M186 复测否定**（解释轨已可用，端到端逐字节一致） |
| 内部 | `passhash` 库 | M184 严格解析后 `int(parts[1])` 语义不再正确 | ✅ **M189 已引入**（本地补丁 1 行，见 §七.4） |

## 七、M189（第 67 轮）：PX-DEF-018…030 **全量判定** + bytes 边界族收口

### 7.1 判定表（2026-09-23 · m188 入库件上三轨复测）

| 对方号 | 主题 | 三轨实测 | 判定 | 本轮状态 |
|---|---|---|---|---|
| PX-DEF-018 | `int_to_bytes(1)` 缺参解释轨崩 | 三轨 `R1002: int_to_bytes 需要 (n, size[, endian[, signed]]) 参数` | 真缺陷 | ✅ **M186 已修** |
| PX-DEF-019 | `bytes_to_int` 的 endian 解释轨被忽略 | 三轨 `bytes_to_int(0102,"little")` = 513 | 真缺陷 | ✅ **M186 已修** |
| PX-DEF-020 | `int_to_bytes(-5,2,"big",true)` 解释轨给 null | 三轨 = `fffb` | 真缺陷 | ✅ **M186 已修** |
| PX-DEF-021 | `zip_pack`/`zip_unpack` 解释轨缺失 | 三轨均可调用（**原样签名** `(files, out_path)`） | ❌ 复测否定 | ✅ M186 复核 |
| PX-DEF-022 | list 切片是浅拷贝、无 `deepcopy` | 三轨一致：`P1=99 P2=77`（切片浅、变量名=引用） | **设计如此**（与 Python/Go 同） | 📄 速查表写明 |
| PX-DEF-023 | `str(Err(...))` 嵌套冗长 | 三轨一致 `Err(a: Err(b))` | **设计如此**（展示形态） | 📄 速查表写明 |
| PX-DEF-024 | 解释器循环性能差 | 设计性（非语义） | **设计如此** | 📄 速查表写明 |
| PX-DEF-028 | 不支持 C 风格 `for (;;)` | 三轨一致 `E1001 非法字符: ';'` | **设计如此**（语法风格） | 📄 速查表写明 |
| **PX-DEF-029** | **`bytes_get` 越界返回 `null`** | 修前：`bytes_get(b,5)`→`null`，而 `b[5]`/`bytes_set(b,5,v)` 均 `R1003 索引越界` | **真缺陷**（同族两套答案 + null 下流成晦涩错误） | ✅ **M189 修**（内部 **211**） |
| PX-DEF-030 | `bytes_to_int` 读 `0xffffffff` 给 4294967295 | **`signed=true` 即得 -1**（`bytes_to_int(b,"big",true)` = -1；`"fffb"` → -5） | **文档缺口**（形参存在，对方未用） | 📄 速查表写明 |

> 结论：**018…030 共 13 条**里，**真缺陷 4（018/019/020/029）已全修**、**复测否定 1（021）**、
> **文档缺口 1（030）**、**设计如此 4（022/023/024/028）**。至此**对方登记的全部 001…030（除跳号 013）
> 均已逐条判定并处置**：真缺陷 **9 条**（001/004/011/002/005/014/026/025/027 + 018/019/020/029）
> 全部修完或补上安全替代，其余为文档/设计类。

### 7.2 本轮修的三条（内部 211/212 一族，同一处代码的三个面）

| 内部号 | 形状 | 修前 | 修后 |
|---|---|---|---|
| **211**（= PX-DEF-029） | `bytes_get` 越界 | `null`（静默）—— 而 `b[i]`/`bytes_set`/`list[i]`/`str[i]` **全都响亮** | `R1003: 索引越界: i (len=n)`（三轨同码同文） |
| **212-a** | `bytes_get(b,"x")` 文案 | 「期望整数，实际是 string」（走 `int_val`），而 `b["x"]` 报「**索引**必须是整数…」 | 索引校验改走 `px_req_int_idx` ⇒ 与索引面**同一入口同一文案** |
| **212-b** | `bytes_get(b, 1.5)` | **静默截断**返回 `b[1]`（`int_val` 里 `(int64_t)` 强转） | `R1002: 索引必须是整数，实际是 float` |
| 同族收尾 | `bytes_set` 越界文案 | 「R1003: bytes_set 下标越界」（同族**第三套**文案） | 统一为 `R1003: 索引越界: i (len=n)` |

**实现**：`runtime/runtime.c` 的 `bi_bytes_get`/`bi_bytes_set`（越界 → `px_error`；索引 → `px_req_int_idx`）；
`selfhost/ibuiltin.px` 加**同口径前置校验**（只为报错带 `行:列`，与 M184 的 `int()` 同款做法）。
**未动** `bytes_slice`（clamp 是**有意**语义，与 `b[a:b]` 同轴，三轨一致 —— 已进门的正判据 A15/A16）。
**门**：`examples/m189_bytes_bounds/`（4 层 + **4 道负控各自判红**，`M189-VERIFY-OK`）。

> ⚠️ 本轮**顺带发现**：`bytes_get(b, 1.5)` 的静默截断**不是**对方登记的条目 ——
> 它是在"给同族补判据"时被门的设计逼出来的（写 `float` 索引这一条）。
> **教训：修一族问题时，判据要覆盖"同一操作的每一个入口 × 每一种坏输入"，而不是只覆盖被报障的那一格。**

### 7.3 PX-DEF-030 的确切口径（补文档，不改行为）

```px
bytes_to_int(hex_to_bytes("ffffffff"))                # 4294967295（无符号，默认）
bytes_to_int(hex_to_bytes("ffffffff"), "big", true)   # -1（有符号 = 协议里的"负长度/NULL 标记"）
bytes_to_int(hex_to_bytes("fffb"), "big", true)       # -5
bytes_to_int(hex_to_bytes(""), "big", true)           # null（长度 0 或 >8 ⇒ null，不变）
```
⇒ 二进制协议里判 NULL/负长度**必须显式传 `signed=true`**，不要用 `> 2147483647` 绕。

### 7.4 `passhash` 的引入（M189 收尾：本地补丁，可撤销）

M184 的严格 `int()` 使 `pass_verify` 在**畸形存储串**上从「静默 0 后误判」变成「抛 R1002」，
而上游测试明确期望 `false` ⇒ 需要一行守卫。处置：
- 上游仍是 `db5f210`（未发新版），故以**本地补丁**形式引入，**不作永久分叉**：
  `tools/patches/registry-px/passhash.patch`（`+2` 行：`if not is_int_str(parts[1]): return false`）；
- 引入器 `tools/import_registry_px.sh` 新增补丁通道（**暂存 → 打补丁 → 算 sha → 写盘**），
  `registry/THIRD_PARTY.md` 新增「补丁」列；**表里的 sha 记的是打过补丁后的内容** ⇒
  M187 门的「表 ⇔ 磁盘」防漂移对拍**继续有效**；
- 除 `passhash` 外 **52 包仍与上游逐字节一致**（本轮实测复核）。
- 上游修复后：删补丁文件 → 重跑引入器即可归零（表里那一列自动变空）。

### 7.5 仍开放（下一批候选）

| 内部号 | 主题 | 为什么还没做 |
|---|---|---|
| **186** | **诊断通道/措辞三轨不同**（解释轨 `错误 [R1003] <行>:<列>: msg` 写 **stdout**；编译轨 `运行时错误 [<fn> 行N]: R1003: msg` 写 **stderr**） | 影响面大（改解释器错误出口 + 编译轨前缀），且**门已经在按"提取正文比对"绕行** ⇒ 需单独一轮 |
| — | 逐包**真实用例**纳入 CI（现在只做 import 冒烟） | 依赖上游 `tests/` 的 fixture（`dotenv`/`glob` 需 `/tmp` 预置文件） |
| — | `Result`/`Option` 方法**文案族**审计 | 低危，可在 186 那轮一并扫 |

---

## 八、M200（第 78 轮）：上游**全量再引入**（53 → 86 库）+ PX-DEF-035 的方向更正

### 8.1 引入结果（上游 `1a7d844`）

| 项 | M198 时 | M200 |
|---|---|---|
| 上游库 | 53 | **86**（**33 新库** + **8 就地更新**：base58/bytes_pack/checksum/datetime/fractions/ini/parser/strcase） |
| `upstream-tests/` 用例 | 55 | **88**（+33，逐字节照搬 · `MANIFEST.sha256` 对拍） |
| `EXPECTED.tsv` 登记 | 61 行 | **88 条**（新增 33：**31 PASS** + **2 SKIP**） |
| 双轨回归实测 | 99 通过 / 0 失败 / 11 跳过 | **161 通过 / 0 失败 / 15 跳过** |
| `registry/` 总数 | 53（全为上游） | **99** = 86（上游）+ **13 本仓自建**（edge/gfx/lunar/pxml/semver/yaml/multipart/cookiejar/html/png/webroute/smtp/collections） |

新增的 2 条 SKIP **各有独立理由**（登记进 `EXPECTED.tsv`，非"跑不过就跳过"）：
`mqtt_test` 需真实 MQTT broker `127.0.0.1:1883` · `redis_test` 需真实 Redis `127.0.0.1:6379`（与 mysql/pg 同族）。
另：`walk_test` 需 `/tmp/wk_src` fixture（7 项目录树）⇒ 已按 dotenv/glob 的先例**补进
`selfhost/run_upstream_tests.sh` 的 fixture 段**（不补就是"库缺陷"误报到上游头上）。

### 8.2 **PX-DEF-035 方向更正**（本轮实测）

| 对方登记 | 实测（M200） |
|---|---|
| `zlib.px`：「**编译轨可用**；解释轨 pxi FFI 表未注册 → 返回 Err，见 PX-DEF-035」 | **方向相反**：**解释轨可用**（`print(zlib_crc32(bytes("abc")))` → `891568578`），**编译轨 `R1001 未定义变量: 'zlib_compress'`** |

根因（内部 **缺陷 240**）：`extern def`（C-FFI 桥）的名字在**解释轨**有运行期兜底
（`selfhost/iexpr.px` 遇未知名/FFI 名 → `i_builtin_ffi_call` → C 侧 `ffi_call` 的**双表**），
而**编译轨**把它编译成 **GETG**（extern def 名即全局名），运行期**从未**发布 ⇒ 表里没有 ⇒ R1001。
实证：`px build` 默认档与 `px build --full` **皆然** ⇒ 与"按引用集自动裁剪"无关，是**发布缺失**；
`px_ffi_register` 的 **92** 个名字里，**89** 个同时也走 `px_set_global`，**仅 3 个只靠 FFI 表**
（`zlib_crc32` / `zlib_compress` / `zlib_uncompress`）—— **这 3 个正是缺陷面**。

修法：`px_ffi_publish_globals()`（`runtime/runtime_ffi.c`）在 `px_register_builtins` 的**建表窗口末尾**
（`g_gc_frozen = 1` 期间，见 M170 缺陷 189）把整表逐条 `px_set_global(name, px_native(name, fn))`。
⇒ 三轨同一条真相；官方 `registry/zlib` 在编译产物里**可直接用**（`upstream-tests/zlib_test` 双轨转 PASS）。

**判定**：PX-DEF-035 **是真缺陷（内部 240）并被修复**；对方对**方向**的判断有误
（这会误导其用户"别用编译轨"，而实际是"必须用编译轨"）。
