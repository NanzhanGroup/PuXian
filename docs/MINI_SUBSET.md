# PuXian Mini 子集规范（自举编译器语言面锁定）

> 版本：M-B1（2026 实测后定稿）
> 来源：自举前评估结论（Mini 子集规范 → 并入 M-B1）
> 目的：锁定"自举编译器支持的语言面"为最小图灵完备子集。PuXian 版编译器只需正确编译本子集（自身源码即在子集内），不必实现全部语言特性 → 编译器负担减半，且自举期间语言变更天然受限（正打风险表第一条"语法追尾"）。

## 一、锁定基线

- **语法基线：M40**（含 M39 Result/Option、M40 字符串插值；M0-M9 核心语法早已冻结）
- **自举期间语言冻结**：只准修 bug，不准加特性。任何语法变更必须走评审（防"语法追尾"风险）

## 二、支持的语言特性（子集内，编译器自身源码使用）

### 2.1 词法
- 标识符、关键字、整数（含 0x 十六进制）、浮点、布尔、null
- 字符串字面量 + 转义（`\n \t \\ \" \' \$ \u{XXXX}`）+ **字符串插值 `${expr}`**（M40，表达式内仅标识符/属性/索引/二元运算）
- 运算符全套、注释 `#`、缩进块（indent/dedent）

### 2.2 语句
- `let`（默认不可变）/ `let mut` / `var`（≡ let mut）/ `const` 变量声明（**顶层 let/var 即全局变量**，函数内可读写；M41.2 起对 let 赋值 = 编译错误 E3002）
- `def` 函数定义（支持默认参数值、类型标注可选、`->` 返回类型）
- `if / elif / else`（含 if 表达式 `let x = if cond: a else: b`）
- `for x in xs`（list/dict/range/生成器/字符串）、`while`、`break`、`continue`
- `return`（含 `return Ok(v)` / `return Err(e)`）
- `import std.collections`（模块函数合并进全局）
- `struct` / `enum`（无载荷变体）/ `trait` / `impl`（扩展方法）
- **`type X const (A = v, B = v)` 简化枚举**（M44，一行式带值枚举：`LogLevel.Info` → 绑定值；match `case LogLevel.Info:` 按值匹配；与无值 enum 并存）
- 赋值：`=`、复合赋值 `+= -= *= //= %= **= &= |= ^= <<= >>=`、`d[k] = v`（**dict 新键赋值**，M-B1 已修复）
- **`target <- expr` 列表追加简写**（M44，≡ `target.append(expr)`；Var/Field/Index 目标；语句级）
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
| 哈希/网络 | sha256 http_get（M45：lockfile 完整性校验 + 远程依赖下载）|
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
| 12 | 类型标注强制检查（空安全/不可变/泛型） | 不可变已实现（M41.2：E3002，let 默认不可变、let mut/var 可变、对 let 赋值编译报错）；空安全已实现（M41.3：E3003，显式非空类型标注赋 null 编译报错，渐进类型省略标注不查）；泛型已实现（M41.4：def first[T] 定义级泛型 + struct Box[T] + 约束 T: Trait，类型擦除渐进，运行时无泛型） | 自举后由 PuXian 编译器自己实现（M41.1-M41.4 全部完成 ✅） |

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
| **codegen 作用域：while 块内 `let` 变量块外不可见**（解释器提升、codegen 未提升 → `_v59 undeclared`） | codegen.rs | ✅ M62-L5 已修（2026-09，codegen hoist：函数内 VarDecl/For 循环变量函数顶提升 + 原位赋值，if/for/while 块外引用对齐解释器/Python 函数级语义；见 §十三.7） |
| **解释器 split 丢弃空段**（vs Rust split 保留空段：`split("a,,b",",")` 解释器 2 段、Rust 3 段） | interp.rs split | ✅ M62-L6 已修（自举 interp 重写后 split 走字符串方法 split 保留空段，编译/解释双模式实测一致；M62 补回归用例防回退，见 §十三.7） |

## 八、M-B2 新增已知限制（自举写代码必须规避）

| # | 限制 | 现象 | 规避 |
|---|---|---|---|
| 1 | **多行 list/dict 字面量** | ~~报"意外的 token: 换行"~~ **M70-S1 已修**：`[\n...]`/`{\n...}` 括号内换行容忍（parser skip_expr_ws），语义与单行等价 | 续行缩进须与缩进栈相容；`=` 后/二元运算符后不换行（需续行用括号包裹） |
| 2 | **编译版浮点 str() 用 `%g`**（与解释器/Rust Display 不一致） | `str(1e-7)`="1e-07"、`str(123456789.123)`="1.23457e+08" 丢精度、`str(1e20)` 错误 | 源码避免极小/极大/高精度浮点字面量；对拍用例不含此类 |
| 3 | **编译版 NUL 字符串截断**（C 字符串本质） | `"\u{0}"` 编译版变空串；对拍含 NUL 字面量的文件时输出 `""` vs `"\0"` | 源码避免 NUL 字面量（rust_str_debug 用 `c < "\u{1}"` 判断 NUL 绕开） |
| 4 | **单行 if 语句不支持**（`if x: y`） | 报"期望 换行，实际得到 return" | 一律用 if 块 |
| 5 | **函数调用参数跨行** | ~~多行调用参数报错~~ **M70-S1 已修**：`f(\na,\nb,\n)` 含尾部逗号全支持 | 同上（缩进栈相容） |
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
| 7 | **模块顶层 `let` 导出** | ~~import 不导出非 Const VarDecl~~ **M70-S3 已修**：import 导出顶层 let/var/const（模块级状态槽），主程序可直名读写（let 只读） | 模块顶层 var 初始化表达式于 import 方启动执行一次（保持纯值初始；可变状态建议 init 函数惰性初始化） |

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

## 十三、M58 dogfood 新增：真实应用（pxhwmond）暴露的语言行为欠账（编译模式 runtime）

> M58 用 PuXian 写首个真实边缘应用 daemon（examples/m58_hwmond，4 模块 import 工程 +
> 手写 HTTP + mmap IPC + 告警通知），dogfood 撞到的语言/runtime 行为坑。均非编译器
> 编写必需（不影响 Mini 子集），但影响**真实应用可写性**，如实记录（M58 README 边界
> 同步）。修补属后续候选（HTTP 客户端错误返回最优先）。

| # | 限制 | 现象（实测） | 规避（pxhwmond 采用） |
|---|---|---|---|
| 1 | **`http_post`/`http_request` 网络失败即 panic，无错误返回** | 连接不可达目标 → `运行时错误: net: 连接失败`，进程直接退出 | **webhook 通知只能 dry-run 落盘报文**，不真发网络（真实发送待语言补错误返回/Result）；长期服务内任何网络调用须确认失败语义 |
| 2 | **spawn 协程不隔离 panic** | 协程内 http_post 失败 → 整个进程被杀（主程序无存活机会） | 不可用 spawn 隔离风险调用；网络等可失败操作须在语言层给错误返回 |
| 3 | **`int(str)` 前缀截断，非全串校验** | `int("0.45")=0`、`int("123abc")=123`、`int("na")=0` | 数值转换前用字段白名单确认纯整数串；浮点值保持字符串或走 `float()` |
| 4 | **`{}` 空 dict 字面量不可靠** | import 合并场景 `var d = {}` 后 `d["k"]=v` 报「无法索引赋值: null」（`{}` 求值为 null） | 动态 dict 用 `json_parse("{}")` 创建；静态 dict 用完整字面量 `{"k": v}` |
| 5 | **import 合并定义 + 初始化模块顶层 var** | ~~模块级 `var` 不可见~~ **M70-S3 已修**：import 导出模块顶层 VarDecl（状态槽，初始化随主程序启动执行）；仍不执行模块其它顶层语句（函数调用/赋值等） | 模块顶层除 var/let/const/def 外不写语句；状态初始化用纯值或显式 init 函数 |
| 6 | **`mmap` 固定 PROT_READ\|PROT_WRITE** | 只读打开的 fd 上 mmap 失败（返回 -1） | mmap 一律用 O_RDWR（mode "r+"）fd |
| 7 | **`str(float)` 显示精度 `%g`** | 浮点转字符串 6 位有效数字（§十二.3 同源）。**M62-L1 已修一半**：整值浮点补 `.0` 后缀（编译/解释对齐）；**M63-L9 已全修**：float→str 最短 roundtrip 全精度（定点舒适区 1e-4≤|f|<1e15 内定点、区外科学，逐位 + strtod 回读取最短 roundtrip），见 §十三.8 | 已修复；无规避项 |
| 8 | **pxi 解释模式对真实应用 API 支持未承诺** | 解释器（Mini 子集）白名单不覆盖网络/S3 真实应用 API（相对路径 import + open/read 链早期实测失败；open/read 等已随 M57/M60 补）。**M63-L8 已补 HTTP/S3 6 名**（http_post/http_request/s3_get/s3_put/s3_list/s3_delete），见 §十三.8 | http_get_stream（chunk_handler 回调跨解释器边界）与 quic/h3/udp/serve/session/bus/cron/sse 高层 API 仍非 pxi Mini 子集（编译模式全能力）；真实网络应用可走编译模式或已补 6 名 |

### §十三.0 M68 根治：pxi native 可达性与编译模式一致（2026-09，docs/M68_PLAN.md）

> 上表 #8「pxi 对真实应用 API 支持未承诺」在本里程碑**根治**：解释器（pxi）不再是
> 手工白名单（interp.px `i_register_builtins` names 129 名），而是经 C 侧 `ffi_call`
> **双表兜底**（① ffi 注册表 → ② 全局 PX_NATIVE = `px_set_global` 注册的全部 runtime
> 内置 281 名）自动可达全部内置——**零 `extern def` 裸脚本**在 pxi 与编译产物行为一致。
>
> - **覆盖**：sqlite6/aes4/rsa5/xml5/zip2/tcp6/udp5/ws9/sse4/session7/ctx3/bus4/cron·
>   signal·time/http_serve·px_serve·route·vhost·middleware·rate_limit·basic_auth·
>   sandbox/fsync·read_at·write_at·truncate_file/set_timeout·set_interval·clear_timer/
>   xxhash/os_pid/now_ms/args/input/panic/gc（91 真 native）+ quic/h3 族（64，完整宿主）。
>   差异表逐名见 docs/pxi_native_diff.md。
> - **typo 语义不漂移**：真拼错名（宿主两表均未命中）→ pxi 仍 R1001「未定义变量」，
>   错误可辨、不误调（宿主哨兵 Err 载荷 `ffi_call: 未注册函数: <name>` 判定）。
> - **quic/h3 条件编译**：`--no-quic` 裁剪宿主无此全局 → 与裁剪编译产物一致不可达。
> - **capability.px 253 PASS 双模式逐字节一致、diffcheck --all 全绿、t_native 零
>   extern def 裸脚本 19 项（pxi == 编译产物）** —— 无回归证据链见 M68_PLAN §四。

### §十三.1 修复记录：HTTP 客户端网络失败 → Err(result)（#1/#2 根因）

> 表中 #1/#2 为 M58 dogfood 当时实测暴露。随后已做**语言面修复**（runtime/runtime.c，
> 见 CHANGELOG + examples/http_neterr_result.px 自检）：

| 项 | 修复后语义 |
|---|---|
| http_get / http_post | 网络失败返回 `Err(result)`（消息 `"net: ..."`），**不再终止进程**；成功仍返回 body 字符串 |
| http_request / http_unix | 网络失败返回 `Err(result)`；成功仍返回 dict{status,headers,body}；HTTP 应用层状态码（404 等）仍由 dict.status 返回（非网络失败） |
| http_get_stream | 网络失败返回 `Err(result)`；成功仍返回 bool |
| 参数个数/类型错误 | 仍 px_error 终止（编程契约，与全部 builtin 一致） |
| spawn 协程内网络调用 | 失败返回 Err 不再 panic → **不再杀整个进程**（#2 在该场景根除）；协程内其他运行时错误（除零等）仍不隔离，写代码仍须规避 |

实现要点：底层 `px_http_once`/`px_http_request` 增加错误缓冲输出（`px_net_fail`，返回
NULL 信号）；`hparse_url` 改为返回错误码（不再终止）；各 builtin 用 `px_net_err` 就地
构造 `Err("net: ...")`。零新增内置函数，纯运行时失败语义修正。

**对语言面的启示（真实用户喂 bug 的 1→1.0n 反馈）**：网络 I/O 失败路径缺失错误返回
（#1/#2）是真实应用（通知/上报类）最痛的缺口，优先于语法糖；`int()` 宽容解析 + 空
dict 字面量是隐蔽坑（静默错值/难排查），值得后续收紧或文档明示。

### §十三.2 修复记录：S3/MinIO 客户端网络失败 → Err(result)（同源收口）

> M37 的 `s3_*` 四函数与 §十三.1 HTTP 客户端同源（共享建连/h_exchange/错误语义），
> 但失败形态更隐蔽：**静默返回 false/null/空 list**（非 panic 非报错），调用方无法
> 区分「网络挂了」与「服务端拒绝」。M58 dogfood 收口 §十三.1 时一并识别，同法修复
> （runtime/runtime.c，见 CHANGELOG + examples/s3_neterr_result.px 自检）：

| 项 | 修复后语义 |
|---|---|
| s3_put / s3_delete | 网络失败返回 `Err(result)`（`"net: 连接 ... 失败"` 等），不再静默 false；应用层 200/204/404 仍返回 bool（404=幂等删除成功，语义不变） |
| s3_get | 网络失败返回 `Err(result)`；应用层 200 仍返回 body 字符串、404 等仍返回 null（非 Err） |
| s3_list | 网络失败返回 `Err(result)`；应用层 200 仍返回 keys 列表 |
| endpoint 协议 | 非 http/https → `Err("net: 不支持的协议: ...")`（原来静默当明文 HTTP 处理） |
| 参数个数/类型错误 | 仍 px_error 终止（编程契约，与全部 builtin 一致） |

实现要点：`px_s3_exec` 增加错误缓冲输出（`px_net_fail`，与 §十三.1 同一 helper）；
建连失败（明文 hconnect / TLS https_connect）与 h_exchange 连接中断分别填 errbuf 并
返回 0（失败信号）；builtin 用 `px_net_err` 就地构造 `Err`。零新增内置函数。

**意义**：至此语言面网络客户端（HTTP 5 个 + S3 4 个）网络失败全部「可检查、可恢复、
不杀进程」，真实应用可安全地在长期服务内做网络调用（webhook 通知/对象存储上报），
m58 notify.px 的 webhook dry-run 解禁为真发成为下一步 dogfood 候选。

### §十三.3 dogfood 闭环记录：pxhwmond webhook dry-run → 真发

> §十三.2 末尾留的候选已落地：m58 notify.px 用新 Err 语义把 webhook dry-run 解禁为
> 真发（HTTP 客户端 Err 语义首次进入真实 daemon 的网络路径，1→1.0n 验证闭环）。
> 改动见 CHANGELOG；此处记录语言面使用要点与验证：

- **Err 判别写法**：`http_request` 成功返回 `dict`（无 `is_err` 方法）、网络失败返回
  `result`（Err）→ 须先按类型分流：`if type(resp) == "result" and resp.is_err()` 再取
  `resp.err()`（错误文本）或 `resp.unwrap()`；直接对返回值 `["status"]` 会在失败时抛
  「无法索引: result」（M58 verify client 同型坑）。成功侧仍 `resp["status"]`。
- **超时/重试防拖**：`http_request(url, method, body, headers, {timeout_ms:3000,
  retries:0})` —— daemon 内同步网络调用给短超时 + 零重试，避免不可达目标拖住采样循环
  （默认 30s/1 次重试）。
- **发送结果落盘**：每轮发送记录 `{ts,url,alert,sent,err|status}` JSONL——成功
  （2xx）`sent:true`+status、网络失败 `sent:false`+err（`net: ...`）、非 2xx
  `sent:false`+`http_status=...`。发送日志与告警日志分离，便于追责通知链路。
- **验证（verify_s3.sh D1/D2 实证）**：本地 `http_serve` mock **实收 3 条 POST /alert**
  （body JSON 含 alert）→ 真发打通；webhook 指向连接拒绝端口 → daemon 3 轮跑完退出码
  0、3 条 `sent:false`+err（修复前此场景 px_error 杀进程，M58-S3 dry-run 即因此不能真发）。
- **测试 mock 经验**：mock 用 PuXian 自举（http_serve + handler 落盘）；bash 侧就绪判定
  不能等 mock 的 stdout（print 到文件全缓冲），改用 `/dev/tcp` 端口探测。

### §十三.4 M59 双模式同步记录：数学/随机内置进 pxi 解释器

> M59（docs/M59_PLAN.md）为编译模式新增 C libm 数学内置 14 函数 + pi/e 常量（S1–S3）。
> §二 已记录一处先于 M59 存在的**双模式不对称**：`sqrt` 编译模式有、解释器无
> （ibuiltin.px 无 sqrt 分支）。M59-S4 一并收口，并给全部 15 函数（含 sqrt）+ 2 常量做
> 解释器同步。此处记录实现要点与过程中新发现的行为差异：

- **白名单注册（interp.px）**：`i_register_builtins` 的 `names` 列表 +15（sqrt/sin/cos/
  tan/atan2/floor/ceil/round/log/log10/exp/random/random_int/random_seed）。常量 pi/e 不能走
  `it_builtin`（函数形态），单独 `env_define(g_globals, "pi", pi)` —— **直接读宿主全局**
  （编译模式 runtime 已用本地宏 PX_PI/PX_E 注册全精度），规避「高精度浮点字面量被编译期
  截断 ~6 位」的坑（math_s1 实测：字面量 3.14159265358979 编译后只剩 ~6 位有效数字，
  断言必须用表达式互证/低精度字面量 + 容差）。
- **分发（ibuiltin.px）**：数学 5 分支（一元 10 函数合一、atan2、random、random_int、
  random_seed）+ 预检 helper——参数个数/类型错返回 `Err`（解释器不杀进程，区别于编译模式
  px_error 终止）；参数合法后**直调同名 runtime C builtin**（pxi 宿主已注册全集）。域错误
  （log(-1)→NaN 等）由 C libm 透传，与编译模式一致、不返回 Err。
- **验证**：math_s4.px 编译/解释/qemu-aarch64 三态断言全过；双模式输出逐字节一致；
  splitmix64 序列 x86==aarch64 逐位一致（纯整数 PRNG + 确定性种子，跨平台可复现）。
- **新发现的既有差异（先于 M59，非本次引入）**：编译模式 `%g` 打印整值浮点为 `"3"`、
  解释器 i_to_str 为 `"3.0"`（`float(3)`、`2.0*2.0` 均可复现）——通用浮点打印不对称
  （与 §十三 #7 同源，%g 6 位问题）；M59 验证文件规避：整值浮点只断言不打印，双模式
  对拍仅用 int/非整值浮点/字符串输出。**M62-L1 已修（2026-09）**：runtime fmt_num
  对齐 ival.px i_fmt_float——整值有限 |f|<1e15 且无 .eE → 补 `.0`，双模式逐字节一致
  （fp_floatfmt 回归；6 位 %g 截断保留为既定规避项）。

### §十三.5 M60 双模式同步记录：边缘设备 5 内置进 pxi 解释器

> M60（docs/M60_PLAN.md）为编译模式新增 5 个 C 小内置（S1：sleep_us/now_us/fcntl；
> S2：tty_config/fd_wait）+ stdlib/edge.px（S3，纯语言封装，import std.edge）。S4 做
> 解释器同步（白名单 +5 + ibuiltin 纯转发 5 分支 → bootstrap/pxi 重建）。要点与新发现：

- **白名单（interp.px）**：`names` 列表 +5（sleep_us/now_us/fcntl/tty_config/fd_wait），
  与 M57 设备原语同区（open/close/ioctl/os_errno/read/write/mmap 已在 §十三 M57-S5 区）。
- **分发（ibuiltin.px）**：5 分支全部「参数预检返回 Err（解释器不杀进程）+ 成功后直调
  同名 runtime C builtin」；设备失败 -1/false + os_errno() 透传不包装（与 M57 fd 原语
  语义一致，非 Err 化——与 HTTP/S3 网络失败 Err 化是两条并存通道）。
- **开工实测修正（ioctl arg 语义边界）**：`TIOCSPTLCK`（_IOW 写 int）驱动要**有效写指针**，
  ioctl arg 传 int 0（→NULL）得 EFAULT errno=14——须传 bytes/int_to_bytes buffer；
  而 `I2C_SLAVE`（_IOW 但驱动读值可 NULL？不，同为 _IOW 实为读指针场景差异）——
  教训：_IOWR/_IOW 类 ioctl 一律用 bytes buffer 形态最稳，int 直传仅用于「驱动把值当
  地址/值」场景（如 I2C_SLAVE 从地址值）。M57 §8.17 已述三种形态，本记录补充边界。
- **验证**：dev_s4.px 编译/pxi/qemu-aarch64 三态断言全过、双模式输出逐字节一致
  （PTY/termios/poll/fcntl/us 时钟跨架构一致，qemu 用户态 syscall 透传真内核）；edge.px
  GPIO V2 布局单测 dev_s3（592B 结构 C offsetof 对照）x86 可跑；stdlib import（std.edge）
  主打编译模式（§十三 #8 pxi 相对 import 限制不变）。

### §十三.6 M61 双模式同步记录：外部库 extern（zlib）+ stdlib import 边界复核

> M61（docs/M61_PLAN.md）A 线把外部系统库（zlib）以 extern def 接入 FFI（runtime_zlib.c，
> 3 函数注册进 px_ffi_register 表，pxc 无条件链 libz.a）；B 线新增 stdlib/gfx.px 与
> stdlib/png.px（第 5/6 个 stdlib）。S4 复核双模式边界，要点与新发现：

- **extern def 双模式成本为零（与 builtin 不同）**：zlib_* 是 extern def → 编译模式走
  codegen 的 ffi_call 桥、解释模式走 `i_builtin_ffi_call`（同一 C 桥 bi_ffi_call，查 C 侧
  注册表）—— **无需改 interp.px 白名单、无需 ibuiltin.px 分支**；只要 `bootstrap/pxi`
  重建时链入 runtime_zlib.c + libz.a（px_register_builtins 无条件注册）即双模式同能力。
  验证：m61_s4_zpxi.px 编译 == pxi 解释输出逐字节一致。
- **pxi Mini 子集 bytes 构造缺口（如实记录）**：interp.px `names` 白名单含
  read_bytes/write_bytes/int_to_bytes/bytes_to_int，但**不含** `bytes`（str→bytes 构造）、
  `bytes_len`/`bytes_get`/`bytes_concat`/`bytes_slice`/`bytes_to_hex`/`hex_to_bytes`/
  `base64_to_bytes` 等 bytes 族 → pxi 侧文本 bytes 只能 int_to_bytes 大端构造；m61_s4_zpxi
  用「roundtrip 后 crc32 守恒」断言替代逐字节比较（编译模式已做字节级验证，双模式行为
  由同一 C 桥保证）。
- **pxi 点分 stdlib import 边界复核（修正 §十三 #8 的绝对化表述）**：探针 impsmoke.px
  `import std.gfx` + canvas_create/set_px/get_px（纯 list/整数路径）在 pxi **可解释通过**
  —— 说明 §十三 #8 的「相对/点分 import 失败」不是全量封锁；**但** std.gfx 的
  text()/blit() 与 std.png 全部编码路径依赖上述 pxi 未同步的 bytes 族 builtin → 运行期报
  「未定义变量: bytes」→ **stdlib 完整能力仍主打编译模式**，pxi 覆盖 C 内置注册面 +
  简单纯 list 库路径。**M62-L7 已补齐（2026-09）**：interp.px 白名单 +14 bytes 族
  （bytes/bytes_len/bytes_get/bytes_set/bytes_slice/bytes_concat/bytes_to_str/bytes_to_hex/
  hex_to_bytes/bytes_find/bytes_base64/base64_to_bytes/base64_encode/base64_decode）+
  ibuiltin 直调转发 → pxi 与编译模式同能力（fp_bytes 17 断言双模式一致；见 §十三.7）。
- **性能 dogfood（编译模式，非 pxi）**：640x480 单帧 PNG 生成 mandelbrot ~34s /
  scene ~14.5s —— 瓶颈为逐像素 list 存储 + bytes 逐字节 concat 拷贝（std.png 行内
  640 次 concat/行 × 480 行 + adler/crc 逐字节 3 遍）。正确性已 python zlib 独立解码
  全验；**画布/编码器优化方向 = bytes 三字节每像素 + 批量行缓冲**（M61-PLAN D5 预案，
  性能按需再评估，不阻塞游戏线正确性）。

### §十三.7 M62 语言面欠账修复记录（L1/L5/L6/L7 落地 + L2/L3/L4 处置）

> M62 把 §七 M-B2 遗留 + §十三 系列中标「待修/待 M 后补/留档按需」的语言面欠账清掉一批
> （用户侧清单 L1–L7；L1=%g 浮点 .0、L5=块作用域、L6=split、L7=pxi bytes 族为锚点）。
> 均双模式实测验证（examples/m62_langfix/verify.sh + verify_l5.sh）。L2/L3/L4 属语义设计，另行处置：

- **L1 编译/解释浮点打印不对称（%g 整值 .0）— ✅ 已修**（commit 9acfb94）：
  runtime.c `fmt_num` float 分支对齐 selfhost/ival.px `i_fmt_float`——整值且有限且
  |f| < 1e15 且 %g 输出无 `.eE/inf/nan` → 补 `.0`。修复前 `print(3.0)` 编译 `3` /
  解释 `3.0`；修复后双模式逐字节一致（含 str/插值/list/dict 内浮点）。**6 位 %g 截断
  （0.1+0.2→0.3）是既定规避项，不在本次爆炸面**（全精度打印留档按需）。
- **L2 `int(str)` 前缀截断（§十三 #3）— 判定为语义设计，不做破坏性收紧**：双模式
  一致（都 atoll/宽容）；仓库代码有依赖（int("42 ")、int("0x") 场景）。处置：文档
  保留警示；如需严格解析可后续加 `int_strict`/全串校验开关（待需求）。
- **L3 `{}` 空 dict 字面量（§十三 #4）— 判定为语法语义（`{}` 历来是"空块"=null），
  不改 parser**：全仓依赖 `{}`→null 的规避已文档化（§九.1）；改成空 dict 需动
  parser/codegen 且破坏 selfhost 既有写法，风险大于收益。空 dict 用
  `json_parse("{}")` 或 `{"_": 0}`+remove（既有规避）。
- **L4 import 只合并函数不执行模块顶层（§十三 #5）— ✅ M70-S3 已修**：import 导出
  非 Const 顶层 VarDecl（cg_module cg_is_definition/cg_def_name 放宽），模块级状态槽
  随合并由主程序启动初始化（px_set_global 注册）；import 仍不执行模块其它顶层语句
  （保持无副作用原则）。语义定案：模块顶层 var 初始化执行一次、`const`/纯函数仍是库
  推荐形态，可变状态走显式 init 函数模式。
- **L5 块作用域不对称（if/for/while 内声明块外引用编译失败）— ✅ 已修**（codegen
  hoist）：selfhost/codegen.px `cg_collect_assign_vars` → `cg_collect_hoist_vars`
  （Assign 目标 + VarDecl + For 循环变量统一收集），函数顶 `px_null()` 预声明 + 原位
  赋值；cg_stmt.px VarDecl/For 分支改为复用已 hoist 变量（顶层代码无 hoist 保持就地
  声明）。修复后 `while/if/for` 块内 `var/let` 块外引用编译通过，与解释器/Python
  函数级语义一致（fp_block.px 验证）。**连带产物变化**：codegen 输出的 C 源码结构
  变化（变量声明提前）→ cases/compiler 的 C golden 全量更新（hoist 结构，语义等价由
  capability 253 PASS + 自举收敛 + 全量 diffcheck 证明）。bootstrap/pxc 自举重建。
- **L6 split 保留空段 — ✅ 确认已修 + 回归防回退**（commit f41c529）：M-B2 记录的旧
  Rust 解释器问题；自举 interp（interp.px/ibuiltin.px）重写后 split 走字符串方法
  split 保留空段（编译/解释双模式实测 3 段一致）。fp_split.px 断言中/尾/前空段。
- **L7 pxi bytes 族白名单 — ✅ 已补齐**（commit ab598e0）：interp.px names +14 +
  ibuiltin.px 直调转发（pxi 宿主 C native 全集直调；bytes_slice 按 1-3 参透传、
  bytes_concat 两两折叠等价 C 变参）。fp_bytes.px 17 断言双模式一致。
  注：解释器参数/类型错由 C 层终止（编程契约，与编译模式一致；区别于 io 可恢复
  错误走 Err）。
### §十三.8 M63 语言面欠账修复记录（L8–L11 全清：pxi 网络 API / float 全精度 / pxc --version）

> M63 清掉欠账表 L8–L11 全部四项（用户侧清单；L8=pxi 真实 API 深化、L9=%g 全精度打印、
> L10=编译期浮点字面量截断、L11=bootstrap/pxc --version）。均双模式实测验证
> （examples/m63_langfix/verify.sh）。与 M62 同主线：以「编译/解释双模式一致」为验收标准。

- **L8 pxi 网络真实应用 API 补白名单 — ✅ 已补齐 6 名**（interp.px + ibuiltin.px）：
  `http_post(url,body)`、`http_request(url,method[,body[,headers[,opts]]])`、
  `s3_get(endpoint,bucket,key,ak,sk)`、`s3_put(...)`、`s3_list(...)`、`s3_delete(...)`。
  机制：interp.px `i_register_builtins` names 表 +6；ibuiltin.px 新增 `i_call_c_net(v)`
  helper（Result 透传：C 网络失败返回 Err("net: ...") result 不杀进程，M57 语义）+
  各分支参数个数校验（错 → Err(i_r1002(...))，解释器不杀进程，区别于编译模式 px_error）。
  验证：本地 mock HTTP（:18080）真请求 http_post/http_request/http_get + 失败路径
  Err 透传 —— m63_net.px 编译/解释双模式输出逐字节一致；m63_net_err.px 4 断言；
  s3_put 参数错 → 报错退出 + 消息。bootstrap/pxi 重建。
  **http_get_stream 留档不入白名单**：C 侧 `bi_http_get_stream` 要求 chunk_handler 为宿主
  PX_FUNC/PX_NATIVE 并跨边界回调（M24 流式下载），解释器函数是解释器闭包值，跨 FFI
  回调不成立 → Mini 子集排除（M63_PLAN 原列 7 名 → 实落 6 名的差异即此，文档记录）。
  quic/h3/udp/serve/session/bus/cron/sse 等编译模式高层 API 维持非 Mini 排除（§十三 #8 边界）。
- **L9 float→str 最短 roundtrip 全精度 — ✅ 已修**（runtime.c `fmt_num` float 分支）：
  原 `%g` 6 位截断（0.1+0.2→"0.3"、1/3→"0.333333"、123456789.123→"1.23457e+08"）→
  **定点/科学按语言 %g 舒适区自动选择**：十进制指数 x∈[-4,15)（1e-4≤|f|<1e15）内 `%.*f`
  定点、区外 `%.*e` 科学，逐位递增 + strtod 回读取**首个 roundtrip 成功者**（最短；位数
  单调，IEEE754 17 位内必达）。规则与既有习惯一致：100000.0→"100000.0"、250.0→"250.0"、
  123456789.123→定点、1e15→"1e+15"、0.0001→定点、1e-5→科学（对齐 C++ to_chars 定点界；
  实现 v2 曾全扫 %.g 取最短字符，会把 100000.0/250.0 误显科学，v3 修正为舒适区规则）。
  M62-L1 `.0` 补丁保留。双模式同根（pxi 宿主 str() 同一 fmt_num）→ 单点修复。
  m63_fp.px 16 断言（roundtrip + 精确文本 + .0/-0.0 + 风格边界值）双模式逐字节一致。
- **L10 编译期浮点字面量截断 — ✅ 已修（零 codegen 源码改动）**：根因是 codegen
  `cg_fmt_float` 用 `str(v)`（px 宿主 str = fmt_num %g 6 位）生成 C 常量 → 高精度字面量
  3.14159265358979323846 编译后只剩 3.14159。pxc 自举重建（内嵌新 roundtrip fmt_num）后
  `str(v)` 自动全精度 → C 产物 `px_float(3.141592653589793)`（m63_prec.px 6 断言 + build
  中间 C grep 实证）。v01_value float**（期望 1.4142135623730951）编译/解释全 PASS →
  **diffcheck.sh 三处 v01 %g 豁免移除**（s09 的 250.0 打印亦随 L9 恢复 golden 一致）。
- **L11 bootstrap/pxc --version — ✅ 已修（自举重建）**：compiler.px main 入口参数前置分支
  （len(args)==2 && args[1] in --version/-v → 输出版本退出 0）+ `PXC_VER="0.1.0"` /
  `PXC_MS="M-B9a"` 常量（照 interp.px PXI_VER 模式，对齐 tools/pxc 头注释）→
  **bootstrap/pxc 自举重建**（tools/pxc build --no-quic selfhost/compiler.px，4144832 B，
  与历史构建一致）+ golden/compiler.c 同步更新（diff 仅 main --version 分支 +7/-1 +
  UID 顺延，无浮点字面量变化——compiler.px 自身无高精度浮点字面量）。
  修复前 `bootstrap/pxc --version` 把参数当文件读（报「io: 读取文件失败」，RELEASE_PROCESS
  已知边界）；修复后 `pxc 0.1.0 (普贤 PuXian · selfhosted M-B9a)` 与 pxi/tools 对齐。
- **验证**：examples/m63_langfix/verify.sh ALL OK（L8/L9/L10/L11 全绿）；diffcheck
  --all/--errors 全绿；capability 双模式 253/253 PASS；自举证明 B.c==golden/compiler.c；
  m59_math/m61_gfx/m62_langfix 历史回归复跑 PASS。
