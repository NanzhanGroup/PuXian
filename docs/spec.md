# 普贤语言规格说明书（PX Specification）

> 版本：v0.1.0-draft
> 日期：2026-08-30
> 状态：M0 里程碑交付物，供 M1（词法/语法）直接实现
> 关联文档：requirements.md（需求）、plan.md（方案）

---

## 1. 总则

### 1.1 语言标识
- 名称：普贤（PuXian）
- 缩写：PX
- 文件后缀：`.px`
- 命令行入口：`px`

### 1.2 一句话定位
> **Python 的脸 + Rust 的类型 + Go 的并发 + C 的出身**
> 双模式（脚本/编译）运行于 Linux，面向 AI 高效编程。

### 1.3 设计原则
1. **显式优于隐式**——所有行为可预测，无魔法
2. **确定性优于灵活性**——同输入必同输出
3. **单一范式优于多范式**——一个代码块内一种主导范式
4. **自动管理优于手动控制**——GC、构建、依赖全自动
5. **语法锚定 Python 子集**——AI 生成成功率最大化
6. **编译期捕获错误优于运行时报错**——能静态查的绝不放运行时

### 1.4 执行模式
| 模式 | 命令 | 说明 |
|------|------|------|
| 脚本模式 | `px run file.px` | 解释执行，秒起，类型宽松 |
| 编译模式 | `px build file.px -o out` | 生成 C → gcc 静态二进制 |
| REPL | `px repl` | 交互式 |
| 工具链 | `px fmt / lint / test / doc / bench / ast / lsp` | 全内置 |

两种执行模式共享同一 AST 语义，行为一致（见 §9）。

---

## 2. 词法

### 2.1 字符集
- 源文件必须为 UTF-8 编码，无 BOM
- 仅支持 LF 换行（`\n`），CRLF 自动归一化
- 缩进必须用空格，**禁止 tab**（lexer 报错 E1003）

### 2.2 注释
| 形式 | 语法 | 用途 |
|------|------|------|
| 单行注释 | `# 注释` | 到行尾 |
| 块注释 | `#| ... |#` | 可嵌套 |
| 文档注释 | `## 注释` | 置于函数/类型上方，`px doc` 收集 |

### 2.3 标识符
- 规则：`[A-Za-z_][A-Za-z0-9_]*`，支持 Unicode（中文可用，推荐 ASCII）
- 惯例：
  - 类型：`UpperCamelCase`（Point、UserList）
  - 变量/函数：`snake_case`（user_name、get_user）
  - 常量：`UPPER_SNAKE_CASE`（MAX_SIZE）
- 保留字不可用作标识符

### 2.4 关键字（保留字）
```
let  var  const  def  fn  struct  enum  trait  impl
match  case  if  elif  else  for  while  in  return  break  continue
import  from  pub  as  spawn  chan  send  recv  select
true  false  null  self  type  capture
```

### 2.5 字面量
| 类别 | 示例 |
|------|------|
| 整数 | `123`、`-42`、`0xFF`、`0b1010`、`0o17`、`1_000_000` |
| 浮点 | `3.14`、`1e10`、`0.5` |
| 字符串 | `"..."`、`'...'`（等价）；多行 `"""..."""`；原始 `r"..."` |
| 布尔 | `true` / `false` |
| 空值 | `null`（别名 `None` 亦接受，等价；文档推荐 `null`） |
| 列表 | `[1, 2, 3]` |
| 元组 | `(1, "a", true)` |
| 字典 | `{"name": "px", "v": 1}` |

字符串转义：`\n \t \\ \" \' \u{XXXX}`。新增 `\$`（转义为字面 `$`）。

#### 字符串插值 `${expr}`（M40）
普通字符串 `"..."` 与多行字符串 `"""..."""` 内支持 `${expr}` 插值：表达式求值后经 `str()` 自动转字符串拼接（数字/布尔/null 自动转换；列表/字典等复杂类型请先手动 `str`/`json`）。

```px
let name = "普贤"; let v = 2
print("hello ${name}, v${v}")   # hello 普贤, v2
print("${1 + 2 * 3}")           # 7（任意单行表达式）
print("${d["k${i}"]}")          # 嵌套插值：表达式内字符串字面量亦展开
print("cost: \${5}")            # cost: ${5}（\$ 转义为字面）
```

限定（实现为词法层展开为 `"前缀" + str(expr) + "后缀"` 拼接，解释器/编译模式语义一致）：
- 表达式须为单行；不支持 `{}` 字面量（dict/set）与注释；
- 原始字符串 `r"..."` 不支持插值（原样）；单引号字符串 `'...'` 同样支持；
- `px fmt` 会将插值源码规范化为等价拼接形式（语义保持）。

### 2.6 运算符
| 类别 | 运算符 |
|------|--------|
| 算术 | `+ - * / // % **`（`//` 整除、`**` 幂，同 Python） |
| 比较 | `== != < <= > >=` |
| 逻辑 | `and or not`（不用 `&& ||`） |
| 位运算 | `& | ^ << >> ~` |
| 赋值 | `= += -= *= /= //= %= **= &= |= ^= <<= >>=` |
| 管道 | `|>`（`x \|> f` 等价 `f(x)`） |
| 可选链 | `?.` |
| 空合并 | `??`（`a ?? b`：a 非 null 取 a，否则取 b） |
| 错误传播 | `?`（Result/Option 解包短路） |
| 类型注解 | `:` |
| 返回类型 | `->` |
| 成员访问 | `.` |
| 强制解包 | `!`（非空断言，lint 警告） |

### 2.7 分隔符
`( ) [ ] { } , : -> . ? =`；语句不用分号结尾，换行即语句结束（显式 `;` 允许作空语句）。

### 2.8 缩进
- 每级 4 空格（约定），缩进形成代码块（同 Python）
- 块内缩进必须一致，不一致报 E2002

---

## 3. 类型系统

### 3.1 基本类型
| 类型 | 说明 |
|------|------|
| `int` | 64 位有符号整数 |
| `float` | 64 位 IEEE 754 |
| `bool` | true / false |
| `str` | 不可变 UTF-8 字符串 |
| `null` | 空值类型 |

### 3.2 集合类型
| 类型 | 说明 |
|------|------|
| `list[T]` | 可变列表 |
| `array[T, N]` | 定长数组（编译期定长） |
| `map[K, V]` | 哈希表 |
| `tuple[...]` | 不可变元组 |

### 3.3 渐进类型（核心）
- 类型标注**可选**。省略时类型为 `any`。
- **脚本模式**：any 动态执行，直接跑。
- **编译模式**：省略类型做静态推断，推断不出则报错要求显式标注。
- 规则：脚本 = 宽松（any 运行），编译 = 严格（推断或显式）。

### 3.4 可空类型（空安全）
- 语法 `T?` 表示可空：`str?`、`Point?`
- 非可空 `T` 禁止赋值 `null`（编译期检查 E3003）（M41.3 已实现：显式非空类型标注赋 null 字面量编译报错；渐进类型——省略标注/TyOptional 不查）
- 访问：`a?.b?.c`（可选链，任一环节 null 则结果为 null）
- 兜底：`a ?? default`
- 强制解包：`a!`（断言非空，运行时 null 则 R1001，lint 警告）

### 3.5 Option / Result
```python
Option[T] = Some(T) | null        # None 即 null
Result[T, E] = Ok(T) | Err(E)     # E 默认 str

def safe_div(a: int, b: int) -> Result[int]:
    if b == 0:
        return Err("division by zero")
    return Ok(a / b)

def calc() -> Result[int]:
    x = safe_div(10, 2)?          # Err 则立即返回 Err，否则解包
    return Ok(x + 1)
```
- `?` 运算符：表达式后加 `?`，若为 Err/None 立即 return，否则解包。
- 使用 `?` 的函数，返回类型必须为 Result/Option（或可空），否则编译错误。

### 3.6 泛型（✅ 已实现 M41.4：定义级泛型，类型擦除渐进）
```python
def first[T](items: list[T]) -> T:
    return items[0]

struct Box[T]:
    value: T

# 泛型约束
trait Comparable:
    def cmp(self, other: Self) -> int

def max_of[T: Comparable](a: T, b: T) -> T:
    ...
```

### 3.7 结构体
```python
struct Point:
    x: int
    y: int

# 字段默认值
struct Config:
    timeout: int = 30
    retries: int = 3

# 方法（self 显式）
impl Point:
    def dist(self) -> float:
        return (self.x * self.x + self.y * self.y) ** 0.5

# 构造（命名参数）
p = Point(x: 1, y: 2)
```

### 3.8 枚举
```python
enum Color:
    Red
    Green
    Blue

# 携带数据（data enum）
enum Shape:
    Circle(radius: float)
    Rect(w: float, h: float)
```

### 3.9 trait（接口）
- 只支持方法签名（v0.1 不做关联类型、默认方法）
- 用于泛型约束与鸭子类型收拢

### 3.10 类型别名
```python
type ID = int
type UserList = list[User]
```

### 3.11 明确不做
- ❌ 联合类型（用 enum / Result 表达）
- ❌ 函数重载（用泛型表达）
- ❌ 继承（用 trait + 组合表达）

---

## 4. 表达式

支持的表达式种类（按优先级从高到低）：
1. 字面量、名字引用
2. 后缀：调用 `f(x)`、下标 `a[i]`、切片 `a[1:3]`、成员 `a.b`、可选链 `a?.b`、强制解包 `a!`
3. 一元：`-x`、`not x`、`~x`
4. 幂 `**`
5. 乘除 `* / // %`
6. 加减 `+ -`
7. 移位 `<< >>`
8. 位与 `&`，位异或 `^`，位或 `|`
9. 比较 `== != < <= > >=`
10. 逻辑 `and`、`or`
11. 空合并 `??`
12. 管道 `|>`
13. 赋值 `=` 及复合赋值（语句语境）
14. 列表推导 `[expr for x in items if cond]`（同 Python）
15. 字典推导 `{k: v for ...}`（v0.1 可选）
16. if 表达式 `if cond: a else: b`（表达式形式）
17. 匿名函数 `fn(x: int) -> int { x * 2 }` 或单行 `fn(x): x * 2`

**禁止**：
- ❌ 三元表达式 `a if b else c`（二义性，用 if 表达式或 match）
- ❌ 隐式类型转换（int→float 允许，其余显式转换 `int(x)`/`str(x)`）
- ❌ 嵌套复杂运算符链混用（lint 建议拆行）

---

## 5. 语句

### 5.1 变量声明
```python
let x = 1              # 不可变（默认）
let mut y = 2          # 可变
var z = 3              # var ≡ let mut（简写）
const MAX = 100        # 编译期常量
let n: int = 5         # 显式类型
let (a, b) = pair      # 解构
```
- 默认不可变；对不可变变量赋值 = 编译错误 E3002。（M41.2 已实现：`let` 默认不可变、`let mut`/`var` 可变，编译模式对 `let` 变量赋值报 E3002）
- 未初始化变量不可用（编译期检查）。

### 5.2 赋值
- `x = expr`、复合赋值 `x += 1` 等
- 仅 `let mut` / `var` 可赋值

### 5.3 控制流
```python
if cond:
    ...
elif cond2:
    ...
else:
    ...

for x in items:
    ...
for i in range(10):
    ...
while cond:
    ...
break / continue
```

### 5.4 match（表达式 + 语句）
```python
match value:
    case Pattern:
        ...
    case Pattern if guard:     # 守卫条件
        ...
    case _:                    # 通配
        ...

# 作为表达式返回值
let desc = match color:
    case Red: "warm"
    case _: "other"
```
模式种类：字面量、枚举变体、解构、通配符 `_`。
要求：match 必须穷尽（有 `case _` 或覆盖所有枚举变体），否则编译错误 E3006。

### 5.5 函数定义
```python
def name(params) -> RetType:
    ...
```
- 参数：必选、默认值 `b: int = 10`、命名参数调用 `f(b: 2, a: 1)`、`*args`
- 无返回类型标注时：有 `return expr` 则推断，否则视为返回 null
- 函数末尾无 return 默认返回 null

### 5.6 return
- `return expr` 或 `return`（返回 null）

### 5.7 错误传播 `?`
见 §3.5。`?` 只能用于返回 Result/Option/可空类型的函数内。

### 5.8 异常
**明确不做** try/catch/throw（v0.1+）。错误处理唯一通道：Result + `?`。

---

## 6. 函数与闭包

### 6.1 具名函数
```python
def add(a: int, b: int) -> int:
    return a + b
```

### 6.2 匿名函数（fn）
```python
double = fn(x: int) -> int { x * 2 }
items.map(fn(x): x * 2)
```
- 闭包可捕获外部变量（读）
- 捕获写：需外部变量为 `let mut`，并在 fn 定义处显式 `capture mut x`（防止隐式副作用，AI 可预测）

### 6.3 高阶函数
list 标准库提供 `map / filter / reduce / each / find / sort`。

### 6.4 递归
支持直接/间接递归。尾递归优化：函数加 `@tailrec` 标注，编译器保证栈安全（脚本模式同样生效）。

---

## 7. 并发

### 7.1 协程
```python
spawn task(args)       # 启动协程，返回 handle
handle.join()          # 等待完成
```

### 7.2 channel
```python
ch = chan[int]()       # 无缓冲 channel
ch = chan[int](10)     # 有缓冲（容量 10）
ch.send(42)
val = ch.recv()
```
- `send` 阻塞直到对方接收；`recv` 阻塞直到有数据
- 关闭：`ch.close()`，recv 返回 `Result[int]`（closed 时 Err）
- 支持泛型 channel：`chan[T]`

### 7.3 select（多路等待）
```python
select:
    case v = ch1.recv():
        ...
    case ch2.send(1):
        ...
    case _:
        ...              # 默认分支（非阻塞）
```

### 7.4 锁
v0.1 不提供锁原语，用 channel 实现互斥（Go 哲学）。`std.sync.Mutex` 后续加入标准库。

### 7.5 调度
- M:N 协程调度，运行时用 C 实现（runtime.c，M-B9a Rust 版已退役）
- 目标：10 万级协程
- 协程栈：动态增长（起始 8KB）

---

## 8. 模块与包

### 8.1 模块
- 一个 `.px` 文件 = 一个模块
- 目录 = 包（含 `mod.px` 或与目录同名文件）
- 程序入口：含 `main()` 的文件

### 8.2 import
```python
import std.io
import std.net.http
from std.collections import HashMap
import "path/to/file.px"      # 相对路径导入（脚本模式）
import "c/sqlite3"            # M42：显式 C 库 import（FFI）
```

#### 8.2.1 显式 C 库 import（M42，对应"清歌的建议 P2-7"）
- `import "c/xxx"`：声明导入 C 库（路径以 `c/` 开头、不以 `.px` 结尾）→ 不加载 .px 文件，函数由 `extern def` 声明。
- `extern def name(params) -> ret`：C 函数声明（无 body、单行、返回类型用 `->` 与普通 def 一致）。
- 调用：extern 函数像普通函数一样调用，统一经 **FFI C 桥 `ffi_call(name, args_list)`**（runtime_ffi.c 注册表）——编译模式与解释模式行为一致。
- 类型映射（MVP）：`int`↔i64、`float`↔f64、`str`↔char*（UTF-8）、`bytes`↔(ptr,len)、`ptr`/句柄↔int 自增 id、list/dict 直接透传（绑定函数为 LXValue 接口）。
- 参数签名编译期校验（个数不符 → 编译错误 E3004）。
- 示例：
  ```python
  import "c/sqlite3"
  extern def sqlite_open(path: str) -> int
  extern def sqlite_exec(db: int, sql: str) -> int
  extern def sqlite_query(db: int, sql: str) -> list

  def main():
      let db = sqlite_open(":memory:")
      sqlite_exec(db, "CREATE TABLE t (id INT)")
      print(sqlite_query(db, "SELECT COUNT(*) c FROM t")[0]["c"])
  ```
- 新增 C 库 = runtime 绑定文件 + `px_ffi_register` + 语言层 extern 声明（三处）。

### 8.3 可见性
- 默认私有（模块内可见）
- `pub` 导出：`pub def`、`pub struct`、`pub enum`、`pub trait`
- 下划线前缀 `_name` 表示内部实现

### 8.4 导入语义（与 Python 的关键差异）
- **import 不执行顶层语句**，只注册定义（无 import 副作用）
- 顶层语句仅在作为入口文件运行时执行
- 目的：确定性优先，杜绝 Python 式 import 副作用坑

### 8.5 依赖
- v0.1 无第三方依赖，标准库单仓库 `std.*` 全内置
- 编译产物零外部依赖（静态二进制）

---

## 9. 双模式执行

### 9.1 脚本模式
- `px run file.px [args...]`
- 解释执行（AST tree-walking / 字节码 VM，M2 定）
- 类型宽松：省略标注按 any 动态执行
- 用途：开发迭代、测试、工具脚本

### 9.2 编译模式
- `px build file.px -o out`
- 流水线：源码 → AST → 类型检查/推断 → C 代码生成 → `gcc -static` → 二进制
- 类型严格：全部静态确定
- 产物：静态链接、零运行时依赖、可复制部署

### 9.3 一致性保证
- 同一 AST 语义，两模式行为一致
- 差异仅限性能与静态检查时机：
  - 脚本模式：any 类型运行时检查（慢但行为相同）
  - 编译模式：编译期检查 + 推断（快，等价运行时检查前置）
- 测试套件必须在双模式下跑同一用例

### 9.4 入口
```python
def main(args: list[str]) -> int:
    ...
    return 0
```
- `main` 可省略参数；返回 int 作为进程退出码（省略返回 0）

---

## 10. 标准库约定

### 10.1 命名
- 全小写下划线 `snake_case`
- 缩写不转大写：`std.io`、`std.net.http`
- 常量 `UPPER_SNAKE_CASE`

### 10.2 内置函数（builtin）
```python
print(*args)          # 输出到 stdout（空格分隔，自动换行）
len(x)                # 长度
range(n) / range(a, b, step)
type(x)               # 类型名
str(x) int(x) float(x) bool(x)   # 显式转换
input(prompt)         # 读一行（可选）
assert(cond, msg)     # 断言，失败 panic
panic(msg)            # 致命错误，退出码 1
```

### 10.3 核心模块（v0.1 范围）
| 模块 | 内容 |
|------|------|
| `std.io` | print、read_file、write_file、read_stdin |
| `std.fs` | 文件系统：list、mkdir、remove、exists |
| `std.net` | http client/server、tcp（基础） |
| `std.json` | parse / stringify |
| `std.time` | now、sleep、duration |
| `std.string` | split、join、trim、upper、lower、contains、replace |
| `std.math` | abs、sqrt、min、max、pow |
| `std.collections` | list/map 扩展操作 |
| `std.os` | env、args、exit |
| `std.process` | run 子进程（捕获输出） |

### 10.4 错误分类
- 可能失败的函数返回 `Result[T, E]`，E 为 `str` 或标准错误类型
- 错误消息格式：`模块名: 描述`（如 `io: file not found: /x`）
- 标准错误带错误码：`E_IO_NOT_FOUND`、`E_JSON_PARSE` 等

---

## 11. 错误码体系

### 11.1 编译错误
| 区间 | 类别 |
|------|------|
| E1xxx | 词法错误 |
| E2xxx | 语法错误 |
| E3xxx | 类型错误 |
| E4xxx | 语义错误 |

常用错误码：
```
E1001 无法识别的字符
E1002 字符串未闭合
E1003 tab 混用缩进
E2001 语法错误
E2002 缩进错误
E3001 类型不匹配
E3002 对不可变变量赋值
E3003 null 赋给非可空类型
E3004 函数缺少 return
E3005 未定义的名字
E3006 match 未穷尽
E3007 泛型约束不满足
E4001 重复定义
E4002 不可达代码
E4003 ? 用于非 Result/Option 函数
```

### 11.2 运行时错误
```
R1001 空值强制解包失败（a! 为 null）
R1002 下标越界
R1003 除零
R1004 类型转换失败
R1005 channel 已关闭
R2001 panic 调用
R2002 协程崩溃（未捕获错误）
R2003 栈溢出
```

### 11.3 错误输出格式（对标 Rust，AI 可消化）
```
error[E3001]: type mismatch: expected int, got str
  --> src/main.px:12:5
   |
12 | let x: int = "abc"
   |               ^^^^^ expected int
   = help: use int("abc") to convert, or change annotation to str
```
每条错误必带：错误码、消息、位置（文件:行:列）、帮助建议。

---

## 12. AI 工具链接口

| 工具 | 命令 | 说明 |
|------|------|------|
| 格式化 | `px fmt` | 确定性格式化（同输入必同输出） |
| 静态检查 | `px lint` | 含"AI 代码"风格检查（禁二义性构造等） |
| 测试 | `px test` | 运行 `@test` 标注的函数 |
| 文档 | `px doc` | 从 `##` 注释生成文档 |
| 基准 | `px bench` | 基准测试，输出对比 |
| AST 输出 | `px ast file.px` | JSON 输出 AST，供 AI 工具分析/重构 |
| LSP | `px lsp` | 诊断/补全/跳转/悬停，编辑器与 agent 集成 |
| 可复现构建 | `px build --locked` | 锁定构建参数，产物带版本哈希 |

### 12.1 面向 AI agent 的协议
- 优先支持 **MCP**（Model Context Protocol），AI agent 可直接调用 `px` 工具链
- 错误信息机器可读（JSON 模式 `px build --json`），AI 可直接解析修复

---

## 13. 明确不做（砍掉清单）

| # | 功能 | 理由 |
|---|------|------|
| 1 | 宏系统 / 元编程 | AI 本身是代码生成器，语言内再套生成器无意义且幻觉率高 |
| 2 | 包管理器 / 第三方依赖树 | 标准库全内置 + 单二进制零依赖，pip/npm 依赖地狱消失 |
| 3 | 借用检查 / 手动内存管理 | AI 在 borrow checker 上循环是最大效率杀手，GC 默认 |
| 4 | 手动线程 / 锁原语 | channel 优先，线程抽象对 AI 负担重 |
| 5 | 交互式断点调试器（gdb 式） | AI 需要结构化错误 + trace + 修复建议，不需要单步 |
| 6 | 类继承体系 | 继承层次是 AI 难推理的 spaghetti，trait + 组合替代 |
| 7 | 异常 try/catch/throw | 与 Result 双轨制让 AI 困惑，只留 Result + ? |
| 8 | 三元表达式 a if b else c | 二义性，用 if 表达式 / match |
| 9 | 隐式类型转换 | AI 易写错，显式转换 |
| 10 | 运算符重载 | 魔法行为不可预测，v0.1 不做（后续 trait 化可选） |
| 11 | 复杂正则表达式 | 简单字符串方法 + glob 足够，std.regex v0.2 可选 |
| 12 | 属性 / 魔法方法（__xxx__） | 隐式行为违背"显式优于隐式" |
| 13 | 动态 monkey-patching | 破坏确定性 |
| 14 | 多范式自由混用 | 单一范式，禁止花式混搭 |
| 15 | CGO/FFI | ✅ **M42 已实现**（`import "c/xxx"` + `extern def` + FFI 注册表，双模式一致） |
| 16 | 构建脚本（Makefile/CMake） | `px build` 一条命令，无需构建系统 |
| 17 | set 类型 | v0.1 用 list/map 组合（可后续加） |
| 18 | 联合类型 / 函数重载 | enum / 泛型替代 |

**设计原则一句话**：
> 传统语言优化"人类写代码的体验"，普贤优化"AI 生成准确率 + 人类审阅可读性 + 部署确定性"。

---

## 14. 版本化与演进（edition）

- 语法演进走 edition：`px 2026`、`px 2028`……
- 源码首行可声明：`# px 2026`
- 默认 edition = 当前 px 工具链最新稳定版
- 新语法特性只进新 edition；旧 edition 语法永久兼容
- 标准库 API 只做向后兼容增强，不 breaking

---

## 15. 语法与 Python 的兼容边界

### 15.1 兼容（AI 直接照 Python 写）
缩进、注释、字符串、列表/字典/元组字面量、`for/while/if/elif/else`、`def`、默认参数、关键字参数、列表推导、切片、`break/continue`、`and/or/not`、`//`、`**`。

### 15.2 扩展（PX 独有，AI 需提示后使用）
类型标注、`struct/enum/trait/impl`、`match/case`、协程 `spawn/chan/select`、管道 `|>`、可选链 `?.`、空合并 `??`、错误传播 `?`、可空类型 `T?`、`let/let mut`。

### 15.3 不兼容（AI 需避免）
`class`/继承、异常 `try/except/raise`、装饰器、`lambda`（用 `fn`）、动态添加属性、`set` 字面量、`import` 副作用执行。

### 15.4 目标
> 提示"用普贤语言写代码，它像 Python 但类型更严格"时，AI 一次生成通过率 > 90%。

---

## 16. 示例程序

### 16.1 Hello World
```python
def main():
    print("Hello, 普贤!")
```

### 16.2 斐波那契
```python
def fib(n: int) -> int:
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)

def main(args: list[str]) -> int:
    n = int(args[0]) if len(args) > 0 else 10
    for i in range(n):
        print(i, fib(i))
    return 0
```

### 16.3 结构体 + match
```python
enum Shape:
    Circle(radius: float)
    Rect(w: float, h: float)

def area(s: Shape) -> float:
    match s:
        case Circle(radius: r): return 3.14159 * r * r
        case Rect(w: w, h: h): return w * h

def main():
    shapes = [Circle(radius: 1.0), Rect(w: 2.0, h: 3.0)]
    for s in shapes:
        print(area(s))
```

### 16.4 并发（协程 + channel）
```python
def worker(id: int, ch: chan[int]):
    ch.send(id * 100)

def main():
    ch = chan[int](4)
    for i in range(4):
        spawn worker(i, ch)
    total = 0
    for i in range(4):
        total += ch.recv()
    print("total:", total)    # 600
```

### 16.5 Result + ? 错误传播
```python
import std.fs

def read_config(path: str) -> Result[str]:
    return std.fs.read_file(path)

def main() -> Result[int]:
    content = read_config("/etc/px.conf")?
    print(content)
    return Ok(0)
```

---

## 附录 A：关键字速查
```
let var const          # 变量声明（不可变/可变/常量）
def fn                 # 具名函数 / 匿名函数
struct enum trait impl # 类型系统
match case             # 模式匹配
if elif else for while in return break continue  # 控制流
import from pub as     # 模块
spawn chan send recv select  # 并发
true false null self   # 字面量/特殊
type                   # 类型别名
capture                # 闭包捕获声明
```

## 附录 B：运算符优先级（高 → 低）
```
后缀: () [] . ?. ! 调用/下标/成员/解包
一元: - not ~
** 
* / // %
+ -
<< >>
&
^
|
比较: == != < <= > >=
and
or
??
|>
= += -= ...
```

## 附录 C：标准库 API 草案（v0.1 简要）
见 §10.3；详细签名在 M5 里程碑输出 `stdlib_api.md`。

---

*本文档为 M0 交付物。变更须走文档修订记录；与实现冲突时以本文档为准，冲突处提 issue 修订。*
