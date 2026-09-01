# PuXian Mini 子集规范（自举编译器语言面锁定）

> 版本：M-B1（2026 实测后定稿）
> 来源：BOOTSTRAP_PLAN §3.2 清歌建议评估结论（Mini 子集规范 → 并入 M-B1）
> 目的：锁定"自举编译器支持的语言面"为最小图灵完备子集。PuXian 版编译器只需正确编译本子集（自身源码即在子集内），不必实现全部语言特性 → 编译器负担减半，且自举期间语言变更天然受限（正打风险表第一条"语法追尾"）。

## 一、锁定基线

- **语法基线：M40**（含 M39 Result/Option、M40 字符串插值；M0-M9 核心语法早已冻结）
- **自举期间语言冻结**：只准修 bug，不准加特性。任何语法变更必须走评审（BOOTSTRAP_PLAN 风险表第一条）

## 二、支持的语言特性（子集内，编译器自身源码使用）

### 2.1 词法
- 标识符、关键字、整数（含 0x 十六进制）、浮点、布尔、null
- 字符串字面量 + 转义（`\n \t \\ \" \' \$ \u{XXXX}`）+ **字符串插值 `${expr}`**（M40，表达式内仅标识符/属性/索引/二元运算）
- 运算符全套、注释 `#`、缩进块（indent/dedent）

### 2.2 语句
- `let` / `var` / `const` 变量声明（**顶层 let/var 即全局变量**，函数内可读写）
- `def` 函数定义（支持默认参数值、类型标注可选、`->` 返回类型）
- `if / elif / else`（含 if 表达式 `let x = if cond: a else: b`）
- `for x in xs`（list/dict/range/生成器/字符串）、`while`、`break`、`continue`
- `return`（含 `return Ok(v)` / `return Err(e)`）
- `import std.collections`（模块函数合并进全局）
- `struct` / `enum`（无载荷变体）/ `trait` / `impl`（扩展方法）
- 赋值：`=`、复合赋值 `+= -= *= //= %= **= &= |= ^= <<= >>=`、`d[k] = v`（**dict 新键赋值**，M-B1 已修复）
- `assert`（可选）

### 2.3 表达式
- 算术、比较、逻辑（and/or/not）、位运算、移位、幂
- 函数调用（**仅位置参数**）、属性 `.`、索引 `[]`、切片 `[a:b]`、负数索引
- 匿名函数 `fn(x) { ... }` / `fn(x): expr`（**仅无捕获**，见 §三.1）
- 列表/字典推导式 `[x for x in xs if c]`、`{k: v for ...}`
- 生成器表达式 `(x for x in xs)` + `gen_next(g)` / `list(g)` / `for-in`
- 可选链 `?.`、空合并 `??`、管道 `|>`
- **match 表达式**（唯一形式：`let r = match x: case A: ... case _: ...`）
- **Result 构造** `Ok(v)` / `Err(e)` + 解包 `?` / `!` + 方法 `is_ok/is_err/unwrap/ok/err`
- 字符串插值 `${expr}`（M40）
- 嵌套数据结构访问（`d["k"][0]["x"]`）

### 2.4 类型（运行时）
`int float str bool null list dict tuple bytes range result enum struct function`

### 2.5 编译器必需的内置函数（白名单）
| 类别 | 函数 |
|---|---|
| 字符串 | str int float len split join replace to_upper to_lower trim starts_with ends_with contains |
| 正则 | regex_match regex_find regex_search regex_find_all regex_replace regex_split |
| JSON | json_parse json_stringify json_path json_path_set |
| 文件 | read_file write_file append_file exists remove mkdir list_dir file_size read_bytes write_bytes |
| 进程 | os_spawn os_wait os_kill env |
| 容器 | range type sorted reversed map filter reduce list dict keys/values/has/get/remove/append（方法） |
| 进制 | hex_to_int int_to_hex int_to_bytes bytes_to_int |
| Result | Ok Err is_ok is_err unwrap ok err |
| 其他 | now sleep print assert exit |

## 三、明确排除（编译模式不可用/未实现，写编译器必须规避）

| # | 特性 | 现状 | 替代方案 |
|---|---|---|---|
| 1 | **闭包捕获**（读/写外部变量） | 编译模式仅无捕获闭包（C 后端独立函数），捕获报"未定义变量" | 纯函数传参 / 顶层全局变量 |
| 2 | **函数内 `def` 嵌套函数** | 编译模式静默忽略（`Ok(String::new())`） | 顶层 def / 无捕获 fn 表达式 |
| 3 | **data enum 带值变体** `Circle(radius)` | 半成品：payload 恒 null，字段值不保存 | `enum 判别 + struct 载荷`（AST 节点用此组合） |
| 4 | `*args` 可变参数 | 编译模式语法错误 | 显式 list 参数 |
| 5 | 命名参数调用 `f(b: 2)` | 未实现（parser 仅位置参数） | 位置参数 / dict 参数 |
| 6 | 字符串乘法 `"ab" * 3` | 运行时错误"需要数值" | 循环拼接 |
| 7 | **match 语句形式**（case 体为 return/多语句） | case 体被当表达式，return 失效返回 null | **match 表达式** `let r = match x: ...` |
| 8 | 三元表达式 `a if b else c` | spec 明确禁止 | if 表达式 / match |
| 9 | `lambda` 关键字 | spec 明确禁止（用 fn） | `fn(x) { ... }` |
| 10 | 异常 try/throw | spec 明确不做 | **Result + `?`**（唯一错误通道） |
| 11 | 多行 fn 匿名函数 | `fn(x):\n  body` 语法错误 | 单行 fn 或块 `fn(x) { ... }` |
| 12 | 类型标注强制检查（空安全/不可变） | 未实现（纯标注，不检查） | 自举后由 PuXian 编译器自己实现 |

## 四、已知限制（不影响子集内程序，记录在案）

- 编译模式 `?` 在**顶层**（main 外）不能传播 Err（报"顶层不能传播"）——写编译器时 `?` 只在函数内使用
- `px fmt` 将插值规范化等价拼接（语义保持）
- JSON 键序：dict 序列化键序双模式可能不同（std_demo 已知波动项）

## 五、对拍契约（diffcheck.sh 已实现）

同一输入 `f.px`，Rust 版 vs PuXian 版输出必须一致：

| 阶段 | 输出 | 一致性标准 |
|---|---|---|
| lex | token 流 | 规范化后逐 token 一致（去行首空白/压缩连续空格/保留顺序与值） |
| parse | AST dump | 规范化后一致（PuXian 版输出同构 AST 文本） |
| build | C 源码 | 逐字节一致（仅去除生成头注释与行尾空白） |
| run | stdout | 逐字节一致 |

**golden 基线**：`selfhost/golden/` 已保存 s01-s08 用例的 Rust 版输出（M-B1 7 例 + M-B3 s08），PuXian 版编译器每完成一个组件即对拍验证。

**对拍命令**：
- `./selfhost/diffcheck.sh --lexer [--build]` → M-B2：PuXian lexer token 流 vs golden
- `./selfhost/diffcheck.sh --parser` → M-B3：PuXian parser AST dump vs golden

## 六、验证方法

1. **能力门禁**：`px run selfhost/capability.px` + `px build` 双模式 110 项全 PASS（M-B1 实测）
2. **对拍基线**：`selfhost/diffcheck.sh <case>.px` 逐个生成 golden（M-B1 已完成 7 例）
3. **回归**：`cargo test --release -- --test-threads=1` 205/205（M-B1 实测）
4. **自举证明**：M-B8 阶段三步 diff

## 七、自举实测发现并修复的编译器缺陷（已并入主线）

### M-B1（能力门禁暴露）

| 缺陷 | 位置 | 修复 |
|---|---|---|
| dict 索引赋值新键报"没有键"（赋值前误读旧值） | interp.rs assign Index | Assign op 不预读旧值 |
| 顶层 `let` 变量生成局部 C 变量，跨函数访问报"未定义变量" | codegen.rs VarDecl | 顶层 let → `px_set_global` |
| 默认参数值被忽略（缺省参数为 null） | codegen.rs 参数绑定 | 生成默认值表达式 |
| dict.values 方法缺失 / dict.get 默认值参数缺失 | runtime.c px_method | 补齐 |
| struct 方法（impl）px_method 不支持 | runtime.c px_method | 查全局 "Type.method" 绑定 self |
| 全局表溢出（GLOBAL_CAP=256 太小） | runtime.c | 256 → 4096 |

### M-B2（lexer 自举重写暴露）

| 缺陷 | 位置 | 修复 |
|---|---|---|
| **编译版 args() 返回空列表**（不保留命令行参数，自举 lexer 无法取输入文件） | runtime.c bi_args | `px_args_init(argc, argv)` 保存 + bi_args 返回 |
| **编译版 list.pop 缺失**（缩进栈 pop 崩溃） | runtime.c px_method | PX_LIST 分支补 pop（与解释器一致） |
| **编译版字符串索引按字节**（与解释器字符语义、px_len 不一致；中文索引错位、for-in 错乱） | runtime.c px_index PX_STR | 按 UTF-8 字符索引（px_unicode_len + 字节定位） |
| **codegen 作用域：while 块内 `let` 变量块外不可见**（解释器提升、codegen 未提升 → `_v59 undeclared`） | codegen.rs | 未修（写代码时变量声明移到循环外规避；待 M 后补） |
| **解释器 split 丢弃空段**（vs Rust split 保留空段：`split("a,,b",",")` 解释器 2 段、Rust 3 段） | interp.rs split | 未修（lexer 已绕开：ctrl 表用遍历不 split）；待修 |

## 八、M-B2 新增已知限制（自举写代码必须规避）

| # | 限制 | 现象 | 规避 |
|---|---|---|---|
| 1 | **多行 list/dict 字面量不支持** | `let d = {\n...}` 报"意外的 token: 换行" | 集合字面量必须单行（长表用字符串/循环构造） |
| 2 | **编译版浮点 str() 用 `%g`**（与解释器/Rust Display 不一致） | `str(1e-7)`="1e-07"、`str(123456789.123)`="1.23457e+08" 丢精度、`str(1e20)` 错误 | 源码避免极小/极大/高精度浮点字面量；对拍用例不含此类 |
| 3 | **编译版 NUL 字符串截断**（C 字符串本质） | `"\u{0}"` 编译版变空串；对拍含 NUL 字面量的文件时输出 `""` vs `"\0"` | 源码避免 NUL 字面量（rust_str_debug 用 `c < "\u{1}"` 判断 NUL 绕开） |
| 4 | **单行 if 语句不支持**（`if x: y`） | 报"期望 换行，实际得到 return" | 一律用 if 块 |
| 5 | **函数调用参数不能跨行** | 多行调用参数报错 | 单行调用 |
| 6 | **exit(n) 不终止执行**（仅设退出码） | `exit(1)` 后代码继续跑 | 报错用 `panic` 立即终止（err 函数先 print 再 panic） |
| 7 | **编译版 AST dump 超大文件内存受限** | `build/parser 解析 >2 万行 AST 的源码` 被 kill（字符串拼接峰值内存）；解释器版可过 | 自举 codegen 消费 AST 树不 dump，不受影响；对拍用例规模内正常 |
| 8 | **大指数浮点 str() 显示**（>=1e16） | PuXian str(1.5e308) 展开为 1500...000（无指数），Rust f64 Debug 显示 1.5e308 | 源码避免 >=1e16 浮点字面量；对拍用例（s09）已规避，M-B6 处理 codegen 浮点表示 |
| 9 | **noncharacter 字符串 Debug 显示**（U+FFFE/FFFF、U+10FFFE/10FFFF 等） | Rust escape_debug 转义为 \\u{10ffff}，PuXian rust_str_debug 直接显示字符 | 源码避免 noncharacter（s09 已规避） |
| 10 | **编译版 inf 浮点常量** | codegen 生成 `px_float(inf)`，C 里 `inf` 未定义导致编译失败 | M-B6 codegen 处理（改 INFINITY）；s09 的 build golden 留空（diffcheck 容错） |

## 九、M-B5 新增已知限制（value/env/module 实测暴露，自举写代码必须规避）

| # | 限制 | 现象 | 规避 |
|---|---|---|---|
| 1 | **`{}` 字面量是空块=null，不是空 dict** | `type({})`=\"null\"；`let d = {}` 后 `d[\"k\"]=v` 报\"索引赋值目标不支持\"（对 null 赋值） | 空 dict 用 `{\"_\": 0}` 创建后 `remove(\"_\")`；dict 字面量必须有至少一个键值对 |
| 2 | **共享引用 dict 比较死循环**（解释器） | `let b = a; a == b`（同一 dict 引用）无限递归被杀（Rust HashMap PartialEq 自引用）；独立构造的 dict 比较正常 | 禁止比较共享引用的 dict（env 的 parent 链不要直接 `==` 比较，用行为验证）；测试脚本规避 |
| 3 | **编译模式类型对象不可用**（TypeRef） | `type(Color)` / 把 enum/struct 类型名作为值传参 → 编译版报\"未定义变量: Color\"（codegen 不注册类型名到全局，仅内联 px_enum/px_struct 构造） | enum/struct 类型名只用于构造（`Color.Red`、`Point(1,2)`），不作值传递；对拍用例跳过 type 对象断言 |
| 4 | **编译模式 range() 物化为 list** | `str(range(3))` 编译版 \"[0, 1, 2]\" vs 解释器 \"range(0, 3, 1)\" | range 只用于 `for` 迭代，不打印/转字符串；对拍用例不跨模式对比 range 的 str |
| 5 | **编译模式闭包显示名带序号** | `str(fn(x){x})` 编译版 \"\<fn \<closure2\>\>\" vs 解释器 \"\<fn \<closure\>\>\" | 不依赖闭包显示名（语义不受影响） |
| 6 | **dict 键必须字符串**（语言级） | 整数键 `d[1]` 编译/解释均按字符串键 \"1\" 处理（`keys()` 返回 [\"1\"]） | 显式 `str(k)` 作键 |
| 7 | **模块顶层 `let` 不导出**（import 只导出 def/struct/enum/trait/impl/const） | `import` 后访问模块的顶层 `let` 变量 → 未定义 | 模块导出常量用 `const` 关键字；函数/类型定义自动导出 |

## 十、M-B6 新增：codegen 自举重写完成（AST → C 源码逐字节对拍）

- **交付**：`selfhost/codegen.px`（主流程/函数生成）+ `cg_stmt.px`（语句）+ `cg_expr.px`（表达式/推导式/闭包）+ `cg_module.px`（import 模块解析），复用 `parser.px`/`pxlexer.px` 的 lex/parse。
- **对拍**：`./diffcheck.sh --codegen`（run 模式）**12/12 用例 C 源码逐字节一致**（s01-s09 + v01-v03，含 import/trait+impl/闭包/生成器/推导式/Result+?/match/插值/unicode 边界/浮点大数）；`--codegen --build`（编译版 codegen.px 本身）**11/12**，v01 差异为已知浮点精度限制（见下表 #2）。
- **对齐 Rust codegen.rs 的关键点**：
  - impl 方法按 `"类型.方法"` 字典序输出（M-B6 起 Rust 版也排序，HashMap 迭代顺序跨进程不稳定 → 确定性输出；`codegen.rs` 已改并回归 205/205）。
  - uid 分配顺序逐点对齐（_tN/_vN/px_err_N/闭包 closure_id）。
  - 推导式嵌套循环组装、match 模式条件、Try/?/ForceUnwrap/IfExpr 的 statement-expression 格式逐字节对齐。
  - 浮点输出对齐 Rust f64 Display：`2.0→"2"`、`1e15→"1000000000000000"`（`cg_fmt_float` + `cg_expand_sci` 处理编译版 %g 指数格式）。

### M-B6 新增已知限制（自举写代码必须规避）

| # | 限制 | 现象 | 规避 |
|---|---|---|---|
| 1 | **模块不导出 `main` 函数**（M-B6 修正：Rust module.rs + PuXian cg_module.px 一致行为） | import 一个含 `def main()` 的模块时，模块的 main 不再合并进主程序（避免编译模式 fn_main 重定义冲突） | 模块（库文件）不定义 main；入口 main 只写在主程序 |
| 2 | **编译模式 `str(float)` 用 C `%g`（6 位有效数字）** | 高精度浮点字面量（如 `1.4142135623730951`）在编译版 codegen（--build）输出截断为 `1.41421`，与解释器/rust Display 不一致；`--codegen`（run）模式无此问题 | codegen 用例避免 >6 位有效数字的浮点字面量；`--codegen --build` 对拍跳过 v01（已知差异） |
| 3 | **编译模式 `str(float)` 指数格式 `%g`（如 `1e+15`）** | `cg_expand_sci` 已在 codegen 层展开为定点（对齐 Rust Display），但运行时 `print(1e15)` 编译版仍输出 `1e+15` | codegen 生成 C 源码无此问题；运行时打印大浮点走解释器或规避 |

## 十一、M-B7 新增：interp 自举重写完成（tree-walking 解释器，run 层对拍）

- **交付**：`selfhost/interp.px`（主入口/内置注册）+ `it_util.px`（工具/值包装）+ `i_err.px`（错误）+ `ival.px`（值层）+ `icall.px`（调用/方法）+ `ibuiltin.px`（内置分发）+ `iexpr.px`（表达式/推导式/match）+ `istmt.px`（语句/赋值）。复用 `parser.px`/`pxlexer.px`/`cg_module.px`。
- **对拍**：`./diffcheck.sh --interp [--build]` —— s01-s07/s09 **stdout 逐字节一致**（run + build 双模式 8/8），v01-v03 **全 PASS**（259/39/26）；`--build` 验证 interp.px 本身在 Mini 子集内（可被 codegen 编译）。
- **值表示**：原生值（int/float/str/bool/null/list/dict/tuple/result/range/bytes）直接透传（PuXian 语义已与 Rust 对齐）；用户函数/结构体/枚举/类型对象/生成器/内置函数用 dict 包装（`__ufn__`/`__struct__`/`__enum__`/`__typeref__`/`__gen__`/`__builtin__`）。
- **错误传播**：eval 返回 `Result`，`Err({"__err__":...})` 真实错误 / `Err({"__prop__":v})` `?` 传播；仅在 `i_call_function` 边界把 `__prop__` 转为正常返回值（对齐 M39 propagate 语义）。
- **语言增强（自举必需）**：新增 **`tuple(list/tuple) → tuple`** 内置函数（Rust builtin.rs + C runtime bi_tuple 双模式）——PuXian 原本无法动态构造 tuple 值（`tuple([1,2])` 报"未定义变量"），自举 interp 求值 Tuple 字面量/切片必须。已双模式对齐（`tuple([1,2,3])` → `(1, 2, 3)`）。
- **修复的 Rust/C 端缺陷**：无（本轮纯 PuXian 侧）。
- **interp 已知限制（写代码规避）**：

| # | 限制 | 现象 | 规避 |
|---|---|---|---|
| 1 | 并发语句（spawn/send/recv/select/chan/mutex/rwlock）interp 不支持 | 报"interp 不支持 X（Mini 子集排除）" | Mini 子集不含并发；自举源码不用 |
| 2 | 内置函数运行时失败（read_file 权限错等）在 interp 内终止（PuXian 无异常捕获） | 错误不可被用户代码 `?` 传播 | read_file 预检查 exists()；对拍用例不含失败路径 |
| 3 | 编译模式 `str(float)` 用 `%g`（6 位有效数字） | `--interp --build` 对拍 v01 的 float** 精度差异 | 同 §十.2；对拍跳过 v01 build |
| 4 | 用户 dict 含保留键（`__struct__` 等 `__` 前缀）会被误判为包装值 | 类型/渲染按包装值处理 | 用户 dict 键避免 `__` 前缀 |
| 5 | 闭包显示名 `<fn <closure>>`（解释器模式） | 与编译模式 `<fn <closureN>>` 不同 | 不依赖闭包显示名（同 §九.5） |

## 十二、M-B8 新增：自举证明完成（PuXian 版编译器编译自己，两步 diff 一致）

- **交付**：`selfhost/compiler.px`（PuXian 版完整编译器 CLI：`import codegen.px` + 主文件声明 25 个全局状态 + `main()`，完整流水线 read→lex→parse→resolve(import)→generate→C 源码输出 stdout）+ `selfhost/bootstrap_prove.sh`（自举证明脚本）。
- **自举证明（经典三步）**：`./selfhost/bootstrap_prove.sh`
  1. Rust 版 `px build compiler.px` → 编译器 A（`build/compiler`，3.87MB 静态二进制，17s）——同时证明 compiler.px 全链在 Mini 子集内；
  2. 编译器 A 运行 `build compiler.px` → B.c（C 产物，约 348KB/6003 行，需 ~3.5min）；
  3. **A.c（Rust 版产物）与 B.c（A 产物）norm_c 后逐字节一致（6002 行 0 差异）→ 自举成立** 🎉
- **强化（M-B9 前置）**：B.c 用 gcc 编译成 B 二进制（`build/compiler_B`），B 再编译 compiler.px → B2.c，与 B.c 一致（完全自举闭环）。
- **NUL 处理统一（自举逼出的修复）**：Rust codegen.rs `escape_str` + PuXian `cg_escape_str` 对 NUL 一律**丢弃**（编译版 C 运行时字符串按 strlen 截断，无法表达 NUL；统一为丢弃使含 NUL 字面量的源码双模式行为一致、C 产物逐字节对齐）。旧行为：Rust 版把 NUL 原样写入 C 源码（`px_str("<NUL>")`），编译版截断成 `px_str("")` → 自举 diff 3 处差异。修复后消除。
- **回归**：`--codegen` 解释模式对拍通过（s02_str 验证 cg_escape_str 改动无破坏）；`cargo test` 205/205。

### M-B8 新增已知限制（自举写代码必须规避）

| # | 限制 | 现象 | 规避 |
|---|---|---|---|
| 1 | **编译版编译器（A/B 二进制）无法正确解析含 NUL 的源码字符串**（C 运行时字符串 NUL 截断，M-B2 §八.3 的延伸） | A 编译含 `"\u{0}"` 字面量的源码时，AST 里 NUL 变空串 | 编译器源码本身含 `"\u{0}"` 字面量（rust_unescape/scan_escape/char_debug）——Rust 版可解析但编译版截断，**自举产物 diff 由 escape_str 双端丢弃 NUL 统一**；写编译器源码避免依赖 NUL 运行时语义 |
| 2 | **编译版编译器编译自己需 ~3.5min / 1.6GB 内存**（C 运行时解释执行 PuXian 编译器逻辑，解析 ~120KB 源码 + import 链） | `build/compiler build compiler.px` 耗时 3-4 分钟、峰值内存 1.6GB | bootstrap_prove.sh 已设 900s 超时；CI/日常不重复跑（产物缓存） |
