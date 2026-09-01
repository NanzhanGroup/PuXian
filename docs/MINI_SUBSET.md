# PuXian Mini 子集规范（自举编译器语言面锁定）

> 版本：M-B1（2025 实测后定稿）
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

**golden 基线**：`selfhost/golden/` 已保存 s01-s07 用例的 Rust 版输出（M-B1 实测），PuXian 版编译器每完成一个组件即对拍验证。

## 六、验证方法

1. **能力门禁**：`px run selfhost/capability.px` + `px build` 双模式 110 项全 PASS（M-B1 实测）
2. **对拍基线**：`selfhost/diffcheck.sh <case>.px` 逐个生成 golden（M-B1 已完成 7 例）
3. **回归**：`cargo test --release -- --test-threads=1` 205/205（M-B1 实测）
4. **自举证明**：M-B8 阶段三步 diff

## 七、M-B1 实测发现并修复的编译器缺陷（已并入主线）

| 缺陷 | 位置 | 修复 |
|---|---|---|
| dict 索引赋值新键报"没有键"（赋值前误读旧值） | interp.rs assign Index | Assign op 不预读旧值 |
| 顶层 `let` 变量生成局部 C 变量，跨函数访问报"未定义变量" | codegen.rs VarDecl | 顶层 let → `px_set_global` |
| 默认参数值被忽略（缺省参数为 null） | codegen.rs 参数绑定 | 生成默认值表达式 |
| dict.values 方法缺失 / dict.get 默认值参数缺失 | runtime.c px_method | 补齐 |
| struct 方法（impl）px_method 不支持 | runtime.c px_method | 查全局 "Type.method" 绑定 self |
| 全局表溢出（GLOBAL_CAP=256 太小） | runtime.c | 256 → 4096 |
