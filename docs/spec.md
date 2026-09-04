# 普贤语言规格说明书（PX Specification）

> 版本：v0.1.0-draft
> 日期：2026-08-30
> 状态：M0 里程碑交付物，供 M1（词法/语法）直接实现
> 关联文档：README.md（项目总览）、docs/ROADMAP.md（路线图）、docs/MINI_SUBSET.md（Mini 子集规范）

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

#### 3.8.1 简化枚举（type X const 一行式带值枚举，M44）
```python
type LogLevel const (Info = "info", Warn = "warn", Err = "error")
type Code const (A = 1, B = 2, OK = true)
```
- **语义**：声明一组命名常量（带值枚举），`LogLevel.Info` 求值返回绑定值 `"info"`。
- 值是常量表达式（字面量为主）；支持任意标量类型（str/int/float/bool/null）。
- **match 支持**：`case LogLevel.Info:` 按绑定值匹配（等价 `case "info":`）。
- **与无值 `enum` 并存**：`enum Kind:` 走变体名匹配（px_enum），`type X const` 走值匹配，两者不冲突。
- **实现**：编译模式值在编译期内联为 C 字面量；解释模式注册全局常量表。
- ⚠️ `type` 仍是普通标识符（`type(x)` 内置函数不受影响）；仅 `type Name const (...)` 形态被识别为枚举声明。

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

#### 5.2.1 列表追加简写 `<-`（M44）
```python
var a = []
a <- 1        # 等价 a.append(1)
a <- 2
# 支持 Var / 下标 / 成员目标
rows[0] <- 10
m["k"] <- 9
obj.items <- 5
```
- `target <- expr` ≡ `target.append(expr)`，目标是 list 时原地追加；非 list 报运行时错误。
- **语句级操作**（不是表达式运算符）：`x = a <- 4` 不支持。
- ⚠️ **与 `a < -b` 的歧义**：`<` 紧邻 `-`（无空格）构成 `<-`；比较负数请保留空格 `a < -b`（Python 风格本就推荐空格）。

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

#### 8.2.1 显式 C 库 import（M42）
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

#### 8.2.2 文件即路由（M43）
- 应用目录约定：**文件名即路由、目录即应用**（PHP 式框架形态），构建期由 `tools/routegen.px` 扫描应用目录 → 生成 `generated_routes.px`（import + `route()` 注册，由入口显式调用 `register_routes()`）。
- 文件名 → 路由规则（`std.webroute.wr_parse_file`，纯函数，相对应用根路径）：
  - `index.px` → `GET /`（默认首页）；`get.px` → `GET /`（方法前缀无 rest）
  - `get_healthz.px` → `GET /healthz`：首段（第一个 `_` 前）∈ 方法表 `{get,post,put,delete,patch,head,options,all}` → 方法前缀，其余按 `_` 拆段连接
  - `get_p_#id.px` → `GET /p/:id`：`#name` 段 → `:name` 路由参数；段 `*` → 通配
  - `post_login.px` → `POST /login`；`api/get_items.px` → `GET /api/items`（子目录前缀路由）
  - fnname = 相对路径去 `.px`、`/` → `_`、`#` 移除（`api/get_items.px` → `api_get_items`；全局唯一）
  - 无方法前缀 → GET，文件名按字面做路径（`user_profile.px` → `GET /user_profile`）
  - 非路由文件：`main`/`middleware`/`generated_routes`、任意 `_` 开头段（私有）、`static/` 目录、非 `.px`
- `middleware.px`（导出 `global_middleware(req)`，返回 null 继续 / 非 null 短路响应）→ 生成 `middleware(global_middleware)` 全局注册
- handler 顶部 20 行内 `# rate_limit: max/window_sec` 注释 → 生成 per-route 限流 opts（`route(..., {"rate_limit": {"max": N, "window_sec": M}})`）
- handler 签名约定：`def <fnname>(req, params)`（params 为路由参数 dict，runtime 已支持）
- 入口 `main.px`：`import "generated_routes"` + 显式调用 `register_routes()`（import 只合并定义不执行）→ `spawn px_serve(port, docroot)`（route 分派优先 + docroot 静态文件 + 404）
- 约束：fnname 全局唯一、method+pattern 唯一（routegen 重复检测报错）
- 与 C-FFI（8.2.1）同属构建期/运行期分离：routegen 构建期生成、运行时纯注册表分派

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

### 8.6 包管理器与版本化（M45）
> 工具：`tools/pxpkg`（PuXian 版包管理器，Rust 版 px pkg 随 M-B9a 退役后重写）。
> 定位：M26 远程 registry（URL + sha256）升级为**可复现构建**——semver 版本管理 + lockfile 锁定。

#### 8.6.1 清单 px.toml（与 M9 手写 TOML 风格兼容）
```toml
[package]
name = "myapp"
version = "0.1.0"

[dependencies]
mylib  = "^1.2.0"          # semver 范围 → registry 解析（M45 新增）
other  = "0.1.0"           # 精确版本
helper = "../local/helper.px"            # 本地路径
remote = "https://.../lib.px#sha256=HEX" # http(s) 远程（M26，保留）
```

#### 8.6.2 依赖 spec 三形态
| 形态 | 判定 | 安装 |
|---|---|---|
| semver 范围（`^1.2.0` / `~1.2` / `1.2.x` / `*` 等）| 非 URL 且路径不存在 | 从 `PX_REGISTRY` 目录解析，选**满足范围最高版本** |
| 本地路径 | `exists(spec)` | 复制到 `.px_modules/<name>/<name>.px` |
| http(s) URL | `http://` / `https://` 前缀 | `http_get` 下载（`#sha256=` 片段可选校验）|

#### 8.6.3 registry（版本源）
- 环境变量 `PX_REGISTRY` 指向目录，结构：`<name>/<version>/<name>.px`（每版本目录一个文件）。
- 版本枚举：目录名经 semver 解析过滤 → `sv_best` 选版本。

#### 8.6.4 lockfile px.pkg.lock（可复现构建契约）
```json
{"format": 1,
 "package": {"name": "myapp", "version": "0.1.0"},
 "deps": {"mylib": {"version": "1.2.5", "sha256": "<hex>", "source": "<registry路径>"}}}
```
- `pxpkg install`：解析依赖 → 安装 → **写 lock**（锁定选中的精确版本 + 内容 sha256 + source）。
- `pxpkg install --locked`：严格按 lock 校验——
  1. lock 必须存在（否则报错）；
  2. 每个依赖 lock 版本满足 px.toml spec（范围依赖校验）；
  3. 已安装内容 sha256 == lock（篡改检测）。
  全部通过才成功 → **二次安装一致、registry 变更不影响已锁定项目**（可复现构建）。
- 幂等：非 `--locked` 安装若已装且 lock 匹配则跳过。

#### 8.6.5 semver 规范（stdlib/semver.px，SemVer 2.0.0 子集）
- 格式：`major.minor.patch[-prerelease][+build]`；数字无前导零；build 不参与比较。
- 优先级：major/minor/patch 数值 → 同号段有 pre < 无 pre → pre 逐标识符（数字按数值 < 字母数字按 ASCII；数量多者大）。
- 范围语法（cargo/npm 风格子集）：
  - `^1.2.3` → `>=1.2.3 <2.0.0`；`^0.2.3` → `>=0.2.3 <0.3.0`；`^0.0.3` → `>=0.0.3 <0.0.4`
  - `~1.2.3` → `>=1.2.3 <1.3.0`；`~1.2` → `>=1.2.0 <1.3.0`；`~1` → `>=1.0.0 <2.0.0`
  - 精确 `1.2.3`；通配 `1.2.x`/`1.2.*`/`1.x`；`*`/空 → 任意
  - 预发布版本只匹配显式含 pre 的范围（本实现范围不带 pre → 一律不匹配）
- 纯函数库：`sv_parse` / `sv_cmp` / `sv_satisfies` / `sv_best`，双模式一致。

#### 8.6.6 命令
```
pxpkg init [--name NAME]        # 生成 px.toml
pxpkg add <spec> [--name ALIAS] # mylib@^1.2.0 | 路径 | URL
pxpkg install [--locked]        # 安装 + 写/校验 px.pkg.lock
pxpkg list / remove <name>
```
- `--dir <path>` 指定项目目录；`PX_REGISTRY` 指定 registry。

### 8.7 QUIC 传输级 API（M46，HTTP/3 应用验证）

`import "c/ngtcp2"`（M42 FFI 实战）：ngtcp2（C QUIC 栈）+ quictls OpenSSL 静态编译进工具链，
语言层提供 QUIC 传输级 API（对齐 udp_* 心智），本地回环端到端验证。

```
import "c/ngtcp2"

extern def quic_listen(port: int) -> int          # 服务端监听（UDP bind + 自签证书）→ listener id | -1
extern def quic_accept(listener: int, timeout_ms: int) -> int   # 阻塞等连接（QUIC 握手完成）→ conn id | -1
extern def quic_connect(ip: str, port: int, alpn: str) -> int   # 客户端连接（握手完成）→ conn id | -1
extern def quic_send(conn: int, data: str) -> int               # 发送（当前双向流）→ 写入流字节数 | -1
extern def quic_recv(conn: int, maxlen: int) -> str             # 阻塞接收 → str（""=超时/对端关闭）
extern def quic_close(conn: int) -> bool
extern def quic_close_listener(listener: int) -> bool
```

- 语义：TLS 1.3（QUIC 内）自签证书（内存生成，MVP 不校验）、单条双向流（stream 0）、阻塞事件循环。
- 双模式一致：编译（pxc build）与解释（pxi run）均走 C 桥 `bi_ffi_call`（M42 机制）。
- 验证：`examples/m46_quic_verify.sh` 回环 PASS（握手 + `hello-quic-42` → `echo:hello-quic-42`）。
- 工程说明：QUIC 栈静态编译进 pxc/pxi 产物（零依赖分发）；选型 quictls + ngtcp2 quictls 后端
  （OpenSSL 3.5 的 QUIC TLS 服务端存在集成问题，弃用并切换 quictls 3.0.9+quic）。

### 8.8 HTTP/3 语义层（M47，QPACK + HEADERS/DATA 帧 + 请求/响应对拍）

`import "c/ngtcp2"`（M46 之上）：在 QUIC 双向流上增加 HTTP/3 **语义层** ——
QPACK 头压缩（RFC 9204 无动态表子集）+ HTTP/3 帧（HEADERS=0x01 / DATA=0x00）
+ 请求/响应对拍。完整 HTTP/3（QPACK 动态表/Huffman/静态表压缩、SETTINGS 控制流、
多路复用、0-RTT/连接迁移、接入现有 HTTP 路由管道）原列为 M48+；其中 Huffman+静态表压缩
已随 §8.9（M48）落地，动态表 + SETTINGS 帧已随 §8.10（M49）落地，多路复用已随 §8.11
（M50）落地，接入现有 HTTP 路由管道已随 §8.14（M53）落地；0-RTT / 连接迁移 /
流上限协商（BLOCKED_STREAMS）已随 §8.15（M54）落地。

```
import "c/ngtcp2"

# QPACK 纯 codec（capability/测试用）
extern def h3_qenc(headers: list) -> bytes          # 编码字段段（list of [name,value]）→ bytes
extern def h3_qdec(data: bytes) -> list             # 解码字段段 → list of [name,value] | null
extern def h3_frame(type: int, payload) -> bytes    # 构造一个 H3 帧（type + varint 长度 + payload）

# HTTP/3 over QUIC（高层，复用 M46 quic_* 连接/流）
extern def h3_serve_read_request(conn: int, timeout_ms: int) -> dict|null   # {method,scheme,authority,path,headers,body}
extern def h3_serve_send_response(conn: int, status: int, headers: list, body: str) -> bool
extern def h3_client_connect(ip: str, port: int, alpn: str) -> int          # = quic_connect
extern def h3_client_send_request(conn: int, method: str, scheme: str, authority: str, path: str, headers: list, body: str) -> bool
extern def h3_client_read_response(conn: int, timeout_ms: int) -> dict|null # {status,headers,body}
```

- **QPACK MVP**：Encoded Field Section 前缀（Required Insert Count=0 + Base=0 → `00 00`），
  字段行仅 Literal Field Line with Literal Name（`001 0 0 | NameLen(3+)`），无 Huffman/静态/动态表。
- **H3 帧**：type + length 用 QUIC varint（RFC 9000 §16）；请求 = HEADERS+DATA，响应 = HEADERS(:status)+DATA。
- **边界**：MVP 约定单 DATA 帧界定消息（不依赖 FIN）；单条双向流（复用 M46 连接模型）。
- 双模式一致：编译（pxc build）与解释（pxi run）走同一 C 桥（runtime_h3.c 注册进 FFI 表）。
- 验证：`examples/m47_h3_verify.sh` 回环 PASS（QPACK 编解码 method/path/x-test 头 → 200 + echo-h3 体）；
  capability section 22（6 项：roundtrip / 伪头 / 非法输入容错），187 PASS/0 FAIL 双模式逐字节一致。

### 8.9 QPACK 完整 codec（M48，Huffman + 静态表压缩）

M47 内联 QPACK MVP 迁出为独立模块（`runtime/runtime_h3_qpack.c` + `tbl.h`），按 **RFC 9204**
升级为无动态表子集的**完整编解码**：Huffman（RFC 7541 Appendix B，QPACK 复用同表）+ QPACK
静态表（RFC 9204 Appendix A，99 项 0-98）索引压缩。仍无动态表（MaxTableCapacity=0：RIC/Base
恒 0；解码遇动态表引用 → null）。纯 codec 语言接口不变，新增 Huffman 纯 codec。

```
import "c/ngtcp2"

# M48 新增：Huffman 纯 codec（capability/互操作验证，RFC 7541 Appendix B）
extern def h3_huff(s: str) -> bytes          # Huffman 编码 → bytes（末字节补 1 padding）
extern def h3_unhuff(data: bytes) -> str    # Huffman 解码 → str | null（非法/EOS 拒绝）

# M48：hex 纯函数进 FFI 表（字节精确断言用，双模式一致）
extern def bytes_to_hex(data) -> str        # bytes/str → 小写 hex
extern def hex_to_bytes(s: str) -> bytes    # hex → bytes | null（非法返回 null）

# h3_qenc/h3_qdec 行为升级（接口不变）：
#   - Indexed Field Line（T=1 静态表全匹配 → 单字节索引，如 [:method,GET] → 00 00 D1）
#   - Literal Field Line with Name Reference（01 N=0 T=1 静态名 + 字面/Huffman 值）
#   - Literal Field Line with Literal Name（001 名/值可 Huffman）
#   - 动态表引用 / RIC≠0 / 非法 Huffman / 截断 → null（容错）
```

- **Huffman 表**：`runtime_h3_qpack_tbl.h`（自动生成，Kraft Σ2^-len≈1.0 校验 + RFC 7541
  Appendix C 官方向量逐字节一致：www.example.com→`f1e3c2e5f23a6ba0ab90f4ff`、no-cache→
  `a8eb10649cbf`、custom-key/custom-value 全命中）。
- **静态表**：QPACK 99 项（RFC 9204 Appendix A，0-based；与 HPACK 表顺序不同）。
- **工程**：QPACK 逻辑自 runtime_h3.c 迁出（净删 123 行），H3 高层请求/响应自动走新 codec；
  tools/pxc 链 runtime_h3_qpack.c；bootstrap/pxi 重建（解释模式同能力）。
- 验证：`examples/m48_qpack_verify.sh` 双模式字节精确 PASS（RFC 向量/静态索引/roundtrip/容错，
  输出逐字节一致）+ capability section 23（15 项）202 PASS/0 FAIL 双模式逐字节一致；
  `examples/m47_h3_verify.sh` 回归 PASS（QPACK 重构不回归，编译+解释）。

---

### 8.10 QPACK 动态表 + SETTINGS（M49，RFC 9204 连接级会话）

M48 无动态表 codec 升级为 **RFC 9204 全量 QPACK 会话**（`runtime/runtime_h3_qpack_dyn.c`，
自包含实现：Huffman/前缀整数/静态表原语内嵌，不依赖 M48 模块内部 static）——动态表
（容量上限 SETTINGS_QPACK_MAX_TABLE_CAPACITY、当前容量 SetCapacity、插入/驱逐/重复条目、
绝对/相对/Post-Base 索引）、编码器流指令（Set Dynamic Table Capacity / Insert with Name
Reference / Insert with Literal Name / Duplicate）、字段段完整前缀（Required Insert Count
wrap 编码 + Base/Sign/DeltaBase + Post-Base 索引）、解码器流指令（Section Acknowledgment /
Stream Cancellation / Insert Count Increment 字节级辅助），并补 HTTP/3 SETTINGS 帧
编解码（RFC 9114 §7.2.8，键 0x01=QPACK_MAX_TABLE_CAPACITY、0x07=QPACK_BLOCKED_STREAMS，
未知键跳过、重复键报错）。M48 无状态 h3_qenc/h3_qdec 行为不变（兼容）。

```
import "c/ngtcp2"

# M49 新增：QPACK 连接级会话（encoder 动态表 + decoder 镜像，双端状态机）
extern def h3_qs_open(max_capacity: int) -> int    # 打开 QPACK 会话 → 句柄 | -1
extern def h3_qs_close(sess: int) -> bool
extern def h3_qs_enc(sess: int, headers: list) -> bytes   # 编码字段段（自动插动态表）
extern def h3_qs_take_enc(sess: int) -> bytes      # 取编码器流待发指令（取后清空）
extern def h3_qs_dec_ingest(sess: int, enc_stream: bytes) -> bool  # 解码端处理编码流
extern def h3_qs_dec(sess: int, section: bytes) -> list  # 解码字段段 → [[名,值]...] | null

# M49 新增：HTTP/3 SETTINGS 帧编解码（RFC 9114 §7.2.8）
extern def h3_settings_enc(pairs: list) -> bytes   # [[k:int,v:int]...] → SETTINGS 帧
extern def h3_settings_dec(frame: bytes) -> list   # SETTINGS 帧 → [[k,v]...] | null
```

- **动态表**：容量字节上限、驱逐（容量不足按 FIFO 逐旧条目）、重复条目指令、编码端
  引用跟踪（记录被引动态条目最大绝对索引 → 写 RIC）；同头复用第 2 轮不再发插入指令
  （字段段显著短于 M48 静态-only codec）。
- **SETTINGS 帧**：完整帧 type=0x04 + QUIC varint 长度 + 键值对；能力断言
  `h3_settings_enc([[1,4096],[7,100]]) == 0406015000074064`。
- **工程**：runtime.c 注册 px_register_h3_qpack_dyn、runtime.h 声明、tools/pxc
  copy_runtime + gcc 链接列表加 runtime_h3_qpack_dyn.c/.h、bootstrap/pxi 重建
  （解释模式同能力，interp codegen 内存峰值约 2.2GB）。
- 验证：`examples/m49_qpack_dyn_verify.sh` 双模式字节精确 PASS（A/B 双会话：首轮插入
  动态表、次轮同头命中动态索引指令空、压缩短于静态-only、SETTINGS roundtrip、多轮
  请求、输出逐字节一致）+ capability section 24（≥10 项）217 PASS/0 FAIL 双模式
  逐字节一致；`examples/m48_qpack_verify.sh` + `examples/m47_h3_verify.sh` 回归
  PASS（不回归）。

### 8.11 HTTP/3 多路复用（M50，QUIC 多双向流并发请求/响应）

M46–M49 是**单条双向流 + conn 级混流缓冲**（所有流数据混入一个 rbuf）。M50 升级为
**QUIC 多流并发**（RFC 9000 §2.1 bidi 流 id：client 0,4,8… / server 1,5,9…）——一条
连接上客户端逐请求开新流（每条流独立承载一个 HTTP/3 请求/响应对拍：HEADERS+DATA
帧序列），服务端按 sid 分拣处理、交错响应。旧单流 API（quic_* 与 h3_serve_read_request
等）保留"默认流"兼容语义（M46/M47 回归不破坏）。

```
import "c/ngtcp2"

# M50 新增：QUIC 多流传输 API（runtime_quic.c per-stream 缓冲）
extern def quic_open_stream(conn: int) -> int              # 本地 open 新 bidi 流 → sid | -1
extern def quic_send_stream(conn: int, sid: int, data: str) -> int   # 写指定流 → 字节数 | -1
extern def quic_recv_stream(conn: int, sid: int, maxlen: int, timeout_ms: int) -> str  # 读指定流
extern def quic_poll(conn: int, timeout_ms: int) -> int    # 等任一活跃流有数据/FIN → sid | -1

# M50 新增：HTTP/3 多路复用语义 API（runtime_h3.c per-sid 帧缓冲）
extern def h3_client_open_stream(conn: int) -> int         # 客户端 open 一条请求流
extern def h3_client_send_request_stream(conn: int, sid: int, method: str, scheme: str, authority: str, path: str, headers: list, body: str) -> bool
extern def h3_client_read_response_stream(conn: int, sid: int, timeout_ms: int) -> dict  # 按流收响应
extern def h3_serve_poll_stream(conn: int, timeout_ms: int) -> int    # 服务端等新请求流 → sid
extern def h3_serve_read_request_stream(conn: int, sid: int, timeout_ms: int) -> dict    # 按流收请求
extern def h3_serve_send_response_stream(conn: int, sid: int, status: int, headers: list, body: str) -> bool
```

- **per-stream 缓冲**：`quic_conn.streams[]`（QUIC_STREAM_MAX=16 线性槽按 sid 精确匹配），
  `quic_recv_stream_data_cb` 按 sid 入队（对端首见建槽 peer=1，FIN flag 记录）；
  `quic_any_data()` 取"有数据/FIN 的最小 sid"保证确定性；旧 quic_send/quic_recv 与
  h3_serve_read_request 等映射"默认流 = 最小活跃 sid"（M47 单流场景逐字节兼容）。
- **H3 层 per-sid 帧缓冲**：`g_h3buf[conn][slot]`（H3_STREAM_SLOTS=16，used 标志，
  修复 static 数组 sid 零值不可作空槽的坑），`h3_take_frame(conn, sid, ...)` 按流消费帧。
- **服务端模型**：accept 后循环 `serve_poll_stream → read_request_stream(sid) →
  send_response_stream(sid)`——poll 返回的每条流只被该轮处理消费，流间互不干扰。
- **客户端模型**：connect（自带 sid 0）后 `h3_client_open_stream` 逐请求开新流（4/8/12…），
  全部发出（在途并发）再按 sid 逐流收响应 → 3 流 3 响应一一对应（无串扰）。
- **验证**：`examples/m50_h3_mux_verify.sh` 双模式 PASS（3 并发请求 /alpha /beta /gamma
  各得独立响应，body=`echo-mux:<path>:stream-<sid>` 一一对应，client 输出逐字节一致）+
  capability section 25 + `examples/m47_h3_verify.sh`（旧 API 单流回归 PASS）。

### 8.12 QPACK 会话接入线上（M51，编码器/解码器/控制流走真实 QUIC 单向流）

M49 的 QPACK 动态表会话是**进程内 A/B 手递**（`h3_qs_take_enc` 取出的编码器指令手动喂对端
`h3_qs_dec_ingest`）。M51 把它**接上真实线路**：每条 HTTP/3 连接按 RFC 9114 §6.2.1 建立三条
**QUIC 单向流**（stream id 次低位=1：client 2,6,10… / server 3,7,11…）——控制流（必须是
本端**第一条** uni 流，首字节 0x00，其上是 H3 帧：SETTINGS）、QPACK 编码器流（0x02，Insert/
SetCapacity 等指令）、QPACK 解码器流（0x03，MVP 预留不自动发 ack）。字段段动态表编码跨网络
真实生效：编码器指令经真实单向流传输、对端 ingest 镜像表；编码器流与请求流是**不同 QUIC 流、
无跨流顺序保证** → 解码遇缺指令（blocked）时泵对端编码器流补齐后重试（RFC 9204 §4.2）。

```python
import "c/ngtcp2"

# M51 新增：QUIC 单向流（HTTP/3 控制/编码器/解码器流）
extern def quic_open_uni_stream(conn: int) -> int          # 本地 open uni 流 → sid | -1

# M51 新增：连接级 HTTP/3 会话（自动开 3 条 uni + SETTINGS + QPACK 动态表）
extern def h3_conn_setup(conn: int, qpack_cap: int) -> bool  # 幂等；之后请求/响应自动走动态表
extern def h3_conn_close(conn: int) -> bool                  # 释放会话（QPACK 槽回收）
extern def h3_conn_peer(conn: int) -> dict|null              # 对端状态（uni sid / SETTINGS / ingest 统计）
extern def h3_conn_stats(conn: int) -> dict|null              # 本端统计（enc_sent/表大小/阻塞恢复）
```

- **接线模型**（复用 M49 `qd_sess` 双表状态机，每端一个会话）：本端编码字段段时 QPACK
  encoder 指令（eout）flush 到本端编码器 uni 流；对端从编码器流读字节 `ingest` 进镜像表后
  解码。SETTINGS（0x01=QPACK_MAX_TABLE_CAPACITY、0x07=QPACK_BLOCKED_STREAMS，RFC 9114
  §7.2.8）从对端控制流自动解析（`h3_conn_peer` 可查）；MVP 自闭环假设两端容量同配（真实
  互操作需等对端 SETTINGS 再启动编码 → 留待后续）。
- **QPACK blocked 恢复**：`px_qd_dec` 返回 -2（RIC 超出已 ingest 指令）时，接收端按对端
  uni 布局探测（对端首条 uni = 2 或 3，步进 4）读编码器流 ingest 后重试；连接状态记录
  `blocked_cnt` 统计。
- **旧 API 兼容**：未 `h3_conn_setup` 的连接走原无状态 codec（M47–M50 行为零变化）；
  `h3_serve_poll_stream` 增强为自动消费对端 uni 流（bidi 请求流照旧返回），旧客户端不开
  uni → 行为不变。
- **运行时修复（真 bug）**：`quic_pump` 增 `want_sid` 参数——读指定流时不再因"其他流先有
  数据"而空转不 recvfrom（M50 多流读路径的潜在死等隐患，M51 对端 uni 数据先于目标流到达
  时暴露）；`QUIC_STREAM_MAX`/`H3_STREAM_SLOTS` 16→24（容纳 6 条 uni + bidi）。
- **验证**：`examples/m51_h3_qpack_wire_verify.sh` 双模式 PASS——双端 setup 后：请求 1 插
  动态表（对端编码器流字节>0）、请求 2 同头复用（编码器流**零新增**指令、字段段全动态索引）、
  SETTINGS 容量经真实控制流协商（qcap=4096）、请求/响应字段跨线还原正确、client 输出逐字节
  一致；capability section 26 + M46–M50 回归全 PASS。

---

### 8.13 QPACK 解码器流 ack 线上化（M52，RFC 9204 §4.4 闭环 + 编码表驱逐安全化）

```
# M52 新增：解码器流承载 Section Ack 的完整闭环 + Known Received Count

extern def h3_conn_stats(conn: int) -> dict|null  # 增 krc/dec_sent/dec_sects/enc_acks 字段
extern def h3_qs_krc(sess: int) -> int            # 本端编码器 Known Received Count
extern def h3_qs_enc_ric(sess: int) -> int         # 最近编码字段段 RIC（>0 对端须 ack）
extern def h3_qs_dec_ric(sess: int) -> int         # 最近解码字段段 RIC（>0 本端须发 ack）
extern def h3_qs_ack_sec(sess: int, ric: int) -> bool  # Section Ack → KRC=max(KRC,ric)
extern def h3_qs_ack_inc(sess: int, inc: int) -> bool  # Insert Count Increment → KRC+=inc
extern def h3_qs_en_len(sess: int) -> int          # 本端编码表条目
extern def h3_qs_de_len(sess: int) -> int          # 对端镜像表条目
extern def h3_qs_ins(sess: int) -> int             # 已 ingest 插入计数
```

- **双向 ack 闭环模型**（RFC 9204 §4.4/§2.1.4/§2.2.2）：
  - **接收方向**（本端解码对端字段段）：`h3_read_section` 解码成功且字段段 Required Insert
    Count > 0（含动态引用）→ 立即在**本端解码器流**发 Section Ack（wire：`1` 1-bit + Stream ID
    7-bit prefix，RFC 9204 §4.4.1 Figure 9）——本端 `dec_sents++/dec_sects++`。
  - **发送方向**（本端编码字段段）：`h3_send_fields` 经 QPACK 会话编码成功后按流登记
    outstanding（`h3conn_state.out_ack[sid]=RIC`，RFC 9204 §4.4.1 的 Section Ack 只带 stream id、
    无 RIC——编码器靠发送时登记的 RIC 还原）；`h3_consume_peer_uni` 对 peer_dec 流解析指令：
    Section Ack（`0x80` 头）→ out_ack_take(sid) → `px_qd_ack_sec(ric)`（KRC=max）；Stream
    Cancellation（`01` 头）→ 移除该流 outstanding（不推 KRC）；Insert Count Increment（`00` 头）
    → `px_qd_ack_inc(inc)`（KRC+=inc）。
  - **Known Received Count（KRC）**：本端编码器已知对端确认的插入计数。h3_conn_stats.krc 可查。
- **编码表驱逐安全化**（RFC 9204 §2.1.1/§2.2.2，修复 M51"全条目可驱逐"互操作缺陷）：编码器
  插入需腾位时**只驱逐绝对索引 < KRC 的已确认条目**；未确认引用的条目不可驱逐 → 表满腾不出时
  **放弃插入**（不写编码器流 Insert 指令，字段段走字面量/既有条目编码）——eout 与本地表严格
  同步，保证 decoder 永不引用已驱逐条目。decoder 镜像表（de）无此约束（对端编码器已自行保证）。
- **会话 RIC 跟踪**：qd_sess 记录 en_last_ric（编码）/dec_ric（解码）；语言层 h3_qs_enc_ric/
  dec_ric/krc/en_len/de_len/ins 供断言；原纯函数 h3_qs_* / 未 setup 无状态路径语义零变化。
- **验证**：`examples/m52_qpack_decack_verify.sh` 双模式 PASS——3 轮请求（x-token 每轮新值、
  编码器先插表、字段段 RIC≥1）：双端 setup 后各自解码对方字段段发 Section Ack、各自 drain
  消费对方 ack 推进 KRC（server dec_sects=3/enc_acks=3/krc=6，client dec_sects=3/enc_acks=3/
  krc=10）、请求/响应字段跨线逐字节还原、client 输出双模式一致；capability section 27（qcap=120
  驱逐受限：f1/f2 插 2 条、f3 表满无 ack 不插 en_len=2、ack_sec(2) 后 f4 驱逐 abs0 插 abs2、
  Insert Count Increment 累计）+ M46–M51 回归全 PASS。

### 8.14 HTTP/3 服务端三栈合一（M53，px_serve opts.http3 + 公共 HTTP 管道 + 外部互操作）

```
# M53 新增：HTTP/3 从"语言层 API"升级为 px_serve 服务管道成员 ——
# HTTP/1.1(TCP) + HTTP/2 + HTTP/3(QUIC/UDP) 共用同一 vhost/路由/限流/访问日志/静态/.px 管道。

extern def quic_h3_listen(port: int, cert: str, key: str) -> int   # M53-S1：QUIC raw listener（多连接托管地基；cert/key 空→自签）
extern def h3_server_listen(port: int, cert: str, key: str) -> int  # M53-S3：HTTP/3 管道托管 listener（每连接线程 → 公共请求管道）
# M53-S4：px_serve(port, docroot, timeout, {http3: true | {port?, cert?, key?}})
#   http3=true（自签）或 dict：缺省同端口（UDP/TCP 不冲突）或指定 UDP 端口起 H3 托管 —— 三栈合一
```

- **输出抽象 PxHttpOut（M53-S2，runtime.h）**：请求管道（`px_http_dispatch` /
  `px_route_try_dispatch`）只面向 PxHttpOut 写响应（respond 一次性 / begin+write+end 流式），
  不再直接 send(fd)；传输后端可插拔：HTTP/1.1 实现（包 PxConn，行为与旧路径逐字节一致）与
  HTTP/3 实现（HEADERS/DATA 经 QPACK `h3_send_fields` 编码，>700KB 自动 HEADERS+多 DATA
  分帧、流结束 `writev_stream + FIN` 显式结束）。
- **接入桥 `px_http_dispatch_h3`（M53-S3）**：H3 请求流解码为 req dict（method/path/headers/
  body/remote/version="HTTP/3"）→ 补全 query 拆分+URL 解码 / cookie / form / body gzip 解压 /
  request_id / headers[Host]=:authority（vhost 与 handler 与 HTTP/1.1 一致）→ 送公共管道
  （CORS/限流/vhost/路由/静态/.px/访问日志）。
- **服务端多连接托管（M53-S1，runtime_quic.c）**：h3 listener 单 UDP fd，收包路由线程
  poll→recvfrom→短头按 DCID 路由到对应连接入包队列（长头 Initial 自动建连接）；每连接独立
  处理线程消费队列 pump ngtcp2。demo 级 quic_accept/quic_pump 路径（§8.7）原样保留。
- **证书与通告（M53-S2/S4）**：cert/key 非空走 PEM 加载（SSL_CTX_use_certificate_chain_file /
  use_PrivateKey_file），空→运行时自签兜底；http3 开启且未显式配置 alt_svc 时默认注入
  `Alt-Svc: h3=":port"`（route/middleware/静态/错误响应统一注入，M53-S2 起 route 路径也带）。
- **并发安全**：H3 连接线程（裸 pthread，不经 px_spawn）经 px_gc_thread_enter/leave 纳入
  stop-the-world GC；全局符号表/GC root 扫描互斥由 M55 保障（§7 并发）。
- **优雅关闭**：SIGTERM → 停收包路由线程 → 关托管连接 → 回收 listener，exit 0。
- **互操作修复（对拍 aioquic 独立实现暴露，RFC 9114/9000/9204）**：① fin-only 流不算活跃
  （防连接线程忙循环、优雅关闭挂死）② 无 DATA 请求（HEADERS+FIN）body 视为空（RFC 9114 §4.1）
  ③ 响应头字段名统一小写（大写触发对端 H3_MESSAGE_ERROR）④ 响应流 FIN 显式结束
  （标准客户端等待流结束，非 RST 语义）。
- **验证**：examples/m53_s3_pipe_verify.sh（同进程 px_serve + h3_server_listen 双栈，
  4 QUIC 连接并发 × 5 请求 = 20 全 PASS，H3 与 HTTP/1.1 curl 同管道输出一致）；
  examples/m53_s4_pxserve_h3_verify.sh（px_serve 三栈合一：自研 client + **aioquic 第三方
  互操作** 200/静态/404 一致 + Alt-Svc + SIGTERM 优雅关闭 + 生产应用 config http3 端到端）；
  examples/m53_s5_pxi_h3_smoke.px（pxi 重建后解释器含 h3_server_listen，id>0 PASS）；
  M46–M52 回归 + capability 双模式 253 PASS + diffcheck + 自举证明全绿。
- **边界（M54+ 方向）**：自研 MVP client 单响应帧缓冲（>700KB 多 DATA 需标准客户端全收验证）。

---

### 8.15 HTTP/3 生产化（M54，会话恢复 / 0-RTT / 连接迁移 / BLOCKED_STREAMS）

```
# M54 新增：HTTP/3 栈从"功能可用"推向"生产级传输语义"（RFC 9000/9114/9204）——
# TLS 1.3 会话恢复（1-RTT resumption）+ 0-RTT early data + 连接迁移 + 流上限协商，
# 全部以 PuXian 语言 API 落地 + 自研双端 + aioquic 互操作验证。

# —— M54-S1 · TLS1.3 会话恢复（1-RTT resumption，RFC 8446/9001）——
extern def quic_connect_resume(ip: str, port: int, alpn: str, session: str) -> int  # SSL_set_session → 1-RTT 握手
extern def quic_session_save(conn: int) -> str   # i2d_SSL_SESSION 导出含 NewSessionTicket 的 DER→hex（内部泵至多 3s 等 ticket）
extern def quic_conn_resumed(conn: int) -> bool  # SSL_session_reused（确为恢复而非全新握手）

# —— M54-S2 · 0-RTT early data（RFC 9001 §8 / 9114 §3.5 幂等 GET / 9204 §3.3.3 静态表）——
extern def quic_0rtt_save(conn: int) -> str      # TLS session DER hex + '|' + ngtcp2_conn_encode_0rtt_transport_params2
extern def quic_connect_0rtt(ip: str, port: int, alpn: str, session0rtt: str) -> int  # 握手完成前即可 quic_send
extern def quic_0rtt_rejected(conn: int) -> bool # ngtcp2_conn_tls_early_data_rejected → 0-RTT 数据按 1-RTT 重发
extern def quic_conn_handshake_done(conn: int) -> bool
extern def h3_server_listen_stateless(port: int, cert: str, key: str) -> int  # H3 无状态 0-RTT 子集（不 setup QPACK 动态表）

# —— M54-S3 · 连接迁移（client 换源 / server path 跟随，RFC 9000 §9）——
extern def quic_migrate(conn: int, local_ip: str, local_port: int) -> bool  # 新建 UDP fd 换源（被动迁移，NAT rebinding）
extern def quic_conn_path(conn: int) -> str   # 当前对端 ip:port（验证迁移生效）
extern def quic_conn_local(conn: int) -> str  # 本地源 ip:port（断言源端口变化）

# —— M54-S4 · 流上限协商（BLOCKED_STREAMS / MAX_STREAMS，RFC 9000 §19.11）——
extern def quic_set_max_client_streams(l: int, n: int) -> bool  # transport params.initial_max_streams_bidi 下发对端
extern def quic_extend_max_streams(l: int, add: int) -> bool    # → ngtcp2_conn_extend_max_streams_bidi → 发 MAX_STREAMS
extern def quic_streams_left(conn: int) -> int  # ngtcp2_conn_get_streams_bidi_left2
# quic_open_stream 达对端配额返回 -206（NGTCP2_ERR_STREAM_ID_BLOCKED），上层等待放行后重试
```

- **TLS 1.3 会话恢复（M54-S1）**：server SSL_CTX（自签/PEM 两处）开启 stateless session
  ticket（`SSL_SESS_CACHE_SERVER` + `session_id_context`，同 listener 内 ticket key 稳定）；
  client `quic_connect_resume` 载入上次 `quic_session_save` 导出的 session → 1-RTT 握手；
  `quic_conn_resumed==true` 断言确为恢复（m54_s1_resume_verify.sh）。
- **0-RTT early data（M54-S2，本里程碑最复杂一层）**：server `SSL_CTX_set_max_early_data`
  + `SSL_set_quic_early_data_enabled` 签发带 0-RTT 能力的 ticket；**收包路由修复**——托管
  server 建连接时除本端 scid 外还登记客户端 Initial 包头 DCID（vc.dcid），否则 0-RTT 长头包
  （与 Initial 同 DCID）被 router 丢弃 → early data 静默丢失；client `quic_connect_0rtt`
  session|tp 拆分 → decode_and_set_0rtt_transport_params（须在首次 write_pkt 前）→ 首次
  write_pkt 发 ClientHello（early_data 扩展 + 0-RTT key）→ 不等握手即返回，`quic_send`
  即 0-RTT 提前发送；tp 缺失/解码失败自动降级 1-RTT resume；握手完成回调检测
  `SSL_get_early_data_status==REJECTED` → `ngtcp2_conn_tls_early_data_rejected` 释放 0-RTT
  状态、后续按 1-RTT 重发。H3 0-RTT 子集：`h3_server_listen_stateless` 不 setup QPACK
  动态表（双端静态表/字面量 codec），0-RTT 请求限定幂等 GET（RFC 9114 §3.5）。
- **连接迁移（M54-S3）**：`quic_migrate` 新建 UDP fd 绑新源端口 → 切换 qc fd/local_sa →
  后续收发走新源（NAT rebinding 语义）；MVP 以被动换源完成（不调
  initiate_immediate_migration——ngtcp2 1.25.90 该路径 cwnd=0 断言崩溃），客户端直接换
  fd、ngtcp2 视角路径不变，服务器完成新路径验证并跟随（RFC 9000 §9 允许）；切换前
  pump 300ms 冲刷旧路径在途包，防服务器 remote_sa 回切死地址。连接创建四处注册
  handshake_confirmed 回调（迁移前提）；echo 服务端在 path validation 完成前 writev 失败
  时 pump 推进 PATH_CHALLENGE/RESPONSE 后重试补发（10×400ms）。
- **流上限协商（M54-S4）**：listener 级 `quic_set_max_client_streams(listener, n)` 经
  transport params.initial_max_streams_bidi 下发给对端（托管 quic_srv_new_conn + demo
  quic_accept 两路一致）；`quic_extend_max_streams(listener, add)` 遍历匹配连接调
  ngtcp2 extend → pump write_pkt 带出 MAX_STREAMS；client 达配额 `quic_open_stream` 返回
  ngtcp2 原始 -206（此前吞为 -1）；`quic_streams_left` 暴露剩余配额供断言。
- **验证**：examples/m54_s1_resume_verify.sh（二次连接 resumed=true + echo）、
  m54_s2_0rtt_verify.sh（transport 0-RTT：connect_0rtt 返回 handshake_done=false + 立即
  send 成功 + echo 正确；H3 0-RTT 静态表 GET → 200）、m54_s3_migrate_verify.sh（M1 echo
  基准 → migrate 换源（源端口变化断言）→ M2 echo 从新源收到，同 conn 无重握手）、
  m54_s4_streams_verify.sh（上限 2 → 开流 4 成功、开流 8 -206 → extend +4 → 重试成功）；
  S5 全量回归：pxi 重建 + capability 双模式 253 PASS + diffcheck --all/--errors rc=0 +
  自举证明 B.c==golden + m46–m54 端到端 14 项全 PASS。
- **边界（M55+ 方向）**：0-RTT + QPACK 动态表前缀（RFC 9204 深度语义）、服务端主动
  迁移、immediate migration（ngtcp2 断言待上游修复）、0-RTT 第三方互操作（aioquic 控制
  面弱）留待后续。

---

### 8.16 本地 HTTP 客户端内建 http_unix（M56，Unix domain socket）

```
# M56 新增：http_unix(socket_path, url_path, method[, body[, headers]]) → dict{status, headers, body}
# Unix domain socket 上的 HTTP 客户端（本地服务 / LLM 网关 / 容器 daemon 调用），
# 与 http_get/http_post（TCP/TLS）互补；响应解析复用 h_exchange（同一 HTTP/1.1 解析器）。
http_unix("/tmp/llm.sock", "/v1/chat/completions", "POST",
          '{"messages":[]}', {"Content-Type": "application/json"})   # → {status, headers, body}
```

- **签名**：`http_unix(socket_path: str, url_path: str, method: str[, body: str[, headers: dict]]) -> dict`。
  `url_path` 为请求路径（含 query，不含 scheme/host）；`method` 任意（GET/POST/...）；
  `headers` 为 `dict[str,str]`，逐行附加 `k: v\r\n`。
- **连接语义**：每次新建 AF_UNIX SOCK_STREAM 连接，`Connection: close` 用完即关（不池化，
  本地低频调用足够）；收发超时放宽至 180s（本地网关可能长响应，如 LLM 长文本生成）。
- **请求组装**：`Host: localhost` + `User-Agent: PuXian/0.1`；带 body 且 headers 未显式含
  Content-Length 时自动补 `Content-Type: application/x-www-form-urlencoded` + Content-Length
  （显式传 headers 可覆盖，JSON 等场景自设 Content-Type）。
- **返回 / 错误**：`{status: int, headers: dict, body: str}`；连接失败 panic（带 errno），
  对端关闭等请求失败 panic `net: http_unix 请求失败`。
- **背景与验证**：M56 由外部生产应用 LLM 接入词元缓存网关（unix socket 通道、key 零落盘）配套引入
  （runtime.c 内建，仓库外应用 .px dogfood 调用）；pxi 重建后解释器同能力支持；无独立示例，
  随 capability/diffcheck 双模式回归保障。

---

### 8.17 边缘设备层：fd 原语 + 数据通道 + mmap 活映射（M57，Linux 用户态设备接入）

```python
# M57-S1 新增：fd 原语（ioctl/read/write 不依赖 import，request 码为内核 ABI 常量，语言不内置常量表）
#   open(path[, mode]) → int fd（mode r/w/a/rw/w+ → O_*；失败 int -1 + os_errno()）
#   close(fd) → bool
#   ioctl(fd, request[, arg]) → int（原始返回值；失败 -1 + os_errno()）
#     arg 缺省/null → NULL（无数据 ioctl）
#     arg int       → 整值按 unsigned long 直传（整数型 ioctl，如 I2C_SLAVE 传从地址）
#     arg bytes/str → 就地 in/out 缓冲区（_IOR/_IOWR 类 ioctl 内核直接读写该内存，同一变量读回）
#   os_errno() → int（最近一次系统调用失败 errno，线程局部）
# M57-S2 新增：fd 数据通道 + mmap 设备映射
#   read(fd, maxlen) → bytes（read(2) 直通，实际读到的字节；0 长度=EOF；失败 int -1 + os_errno()）
#   write(fd, data)  → int（write(2) 直通，EINTR 重试；失败 -1 + os_errno()；data bytes/str）
#   mmap(fd, length[, offset]) → bytes（PROT_READ|PROT_WRITE + MAP_SHARED 活映射视图，
#     GC 自动回收时 munmap；失败 int -1 + os_errno()；length 1..INT_MAX-1，offset 须页对齐）
#   munmap(bytes) → bool（显式提前解除；非映射/已解除返回 false）
#   mem_write(mmap_bytes, offset, data) → int（就地写映射视图 [offset..]，超长截断到视图尾）

fd = open("/dev/i2c-1", "rw")            # 打开设备
ioctl(fd, 0x0703, 0x48)                  # I2C_SLAVE：int 形态直传从地址（0x0703 为 _IOC 32 位码）
write(fd, b"\x00")                       # 写器件寄存器命令
data = read(fd, 2)                       # 读器件数据
close(fd)

fb = open("/dev/fb0", "rw")
info = int_to_bytes(0, 160)              # 160B 就地 buffer
ioctl(fb, 0x4600, info)                  # FBIOGET_VSCREENINFO：内核填充结构（_IOR 就地写回）
view = mmap(fb, 128 * 64 * 4, 0)         # MAP_SHARED 活映射（帧缓冲像素区）
mem_write(view, 0, pixel_bytes)          # 就地写像素（bytes_set 是 COW 复制语义，改不了映射区）
munmap(view)
close(fb)
```

- **背景与定位（M57，Linux 用户态设备层）**：PuXian 在嵌入式方向只能到 **Linux 边缘设备层**
  （树莓派 / 网关 / 盒子——runtime 含 GC/线程/动态值，裸机 MCU 无 OS + 架构不符，明确不做）。
  此前文件 IO 为路径式（read_at/write_at 内部 open 用完即关），语言面无**持久 fd 句柄**；
  设备接入需要「打开设备 → ioctl 配置 → read/write 数据 / mmap 大块直访 → 关闭」的 fd 闭环，
  故 M57 一并补齐 fd 原语（open/close/ioctl/os_errno，清歌方案核查修正——原方案默认已有 fd 句柄）。
- **open/close/ioctl/os_errno（S1）**：mode 语义 `r`=O_RDONLY（默认）/`w`=O_WRONLY|O_CREAT|O_TRUNC/
  `a`=O_WRONLY|O_CREAT|O_APPEND/`rw`,`r+`=O_RDWR/`w+`=O_RDWR|O_CREAT|O_TRUNC；设备文件典型 `r`
  （只读查询）/`rw`（读写控制）。ioctl arg 三形态见上（int 直传 / bytes·str 就地 buffer / null），
  `_IOR` 类调用后同一对象内容被内核填充，用 `bytes_to_int`/`cstr_at`/`int_from_bytes` 解析。
  request 为 32 位 `_IOC` 码（方向位在最高 2 位可 >2^31，语言用 0x 字面量/十进制均可）。
- **read/write（S2）**：read(2)/write(2) 直通——设备文件顺序读写/收发的通用入口（与 socket 无关）；
  read 返回实际读到的字节（0 长度=EOF 空 bytes，与失败 int -1 类型区分）；write EINTR 自动重试。
- **mmap/munmap/mem_write（S2）**：`mmap(fd,len[,offset])` 建 **PROT_READ|PROT_WRITE + MAP_SHARED 活映射**
  （帧缓冲 /dev/fb0、共享内存、DMA 缓冲 → bytes 视图，配合 ioctl 成「配置 + 大数据块直接内存访问」双通道；
  ioctl 也可把 mmap 视图作就地 buffer——视图是活内存，内核写入立即可读）。
  **生命周期关键**：mmap bytes 的 LXObject 位域 `is_mmap=1`、data 指向映射区而非 xmalloc 堆块 →
  GC sweep 走 munmap 而非 xfree（构造先 gc_register 再置位，防 sweep 误回收）；`munmap` 显式提前解除后
  置 data=NULL/len=0/is_mmap=0（防 double-unmap，重复/非映射返回 false）；
  `mem_write(map, offset, data)` 就地写映射视图（普通 bytes 的 bytes_set 是 COW 复制语义，改不了映射内存；
  帧缓冲写像素/共享内存写数据必须就地写底层映射区）。
- **设备示例（S3）**：m57_s3_gpio.px（/dev/gpiochipN `GPIO_GET_CHIPINFO_IOCTL` 68B buffer 就地填充
  name[32]/label[32]/lines 解析，只读查询）；m57_s3_i2c.px（/dev/i2c-N `I2C_SLAVE` int 形态直传从地址 +
  write 寄存器命令/read 器件数据）；m57_s3_devctl.px **真内核替身硬断言**（loopback 网卡 ifreq：
  SIOCGIFADDR→family=2+127.0.0.1、SIOCGIFFLAGS→LOOPBACK、SIOCGIFHWADDR→family=772 + PTY `TIOCGPTN`）
  ——LD_PRELOAD mock 因 pxc 产物静态链接不可行，改内核自带用户态可访问设备走**同一胶水路径**，验证力度更强。
- **交叉编译与裁剪（S4）**：`pxc build --no-quic [--cc <交叉CC>] [--mbedtls-lib <dir>] [--sqlite-obj <file>]`
  —— ngtcp2/openssl-quictls 无 aarch64 预编译且交叉成本高 → 裁剪（PX_NO_QUIC 条件编译 7 处）；
  mbedtls/sqlite 纯 C 交叉保留（tools/cross_aarch64.sh，mbedtls 3.6.2 + sqlite3 交叉入库）；
  musl 兼容 5 点（execinfo 条件包含 / GC `__aarch64__` 寄存器扫描分支 / getcontext→内联汇编 SP+setjmp
  spill / close_range→循环关闭）；qemu-aarch64 静态产物设备层 ioctl 与 x86 结果一致
  （asm-generic ioctl 码 + ifreq 布局**跨架构实证一致**）。
- **pxi 解释同能力（S5）**：解释器自举源码 `selfhost/interp.px` 内置白名单 +10（open/close/ioctl/os_errno/
  read/write/mmap/munmap/mem_write/http_unix）+ `selfhost/ibuiltin.px` `i_call_builtin` 补 10 个纯转发分支
  （直调同名 runtime C builtin，语义与编译模式天然一致；可选参数按实参个数透传对齐 C 签名）→
  `pxc build selfhost/interp.px` 重建 bootstrap/pxi。
- **验证**：examples/m57_s1_ioctl_verify.sh / m57_s2_mmap_verify.sh / m57_s3_verify.sh /
  m57_s4_cross_verify.sh / m57_s5_pxi_smoke.px 双模式全 PASS（真内核路径：TCP fd/文件/lo ifreq/PTY/mmap
  活映射双向可见/GC 自动 munmap 300 轮）；capability 双模式各 253 PASS 输出逐字节一致；
  diffcheck --all/--errors 全绿；自举证明 B.c==golden 逐字节一致（6381 行 C）。
- **边界**：ioctl request 码为内核 ABI 常量，x86_64 与 aarch64 的 asm-generic 大部分一致，特殊码按目标
  内核头适配（语言不内置常量表）；真实设备（GPIO/I2C）在 CI/容器不可用 → 条件探测 + 真内核替身验证
  （语义正确性以 TCP fd/lo ifreq/PTY 真实内核路径为准）；裸机 MCU（STM32/ESP32）明确不做；
  通用动态 FFI（dlsym）等「任意 C 库即插即用」真需求再上。

### 8.18 边缘设备深化：us 级时钟 + fd 控制 + 设备组小内置 + std.edge（M60，树莓派线深化）

> 在 §8.17 fd 原语之上收敛 GAP 树莓派线 #1–#5（GPIO 真控制/边沿中断/串口 termios/
> us 级时钟），M60（docs/M60_PLAN.md）。新增 5 个 C 小内置（Linux 用户态边缘设备）：
> `sleep_us(us)` → null（us 级 nanosleep，EINTR 续睡，`<=0` 不睡）；`now_us()` → int
> （**CLOCK_MONOTONIC** 微秒，测量/计时语义，与 `now_ms` 的 REALTIME 墙钟用途区分，
> 两者数值起点不同不可比）；`fcntl(fd, cmd[, arg])` → int（标准 fcntl，O_NONBLOCK 等）；
> `tty_config(fd, baud, raw)` → bool（tcgetattr → raw=true 则 cfmakeraw 关 canonical/echo →
> cfsetispeed+cfsetospeed → tcsetattr(TCSANOW)，baud 常规档 9600…921600）；`fd_wait(fds,
> timeout_ms)` → list<就绪 fd>（内部 poll；fds 收 int/list<int> 上限 64，仅监听 POLLIN，
> revents 非 0 即事件返回——POLLHUP/ERR 由随后 read 判 EOF/-1；**超时空 list 非错误**；
> poll 系统错误 int -1 + os_errno；EINTR 自动续等）。失败语义延续 M57：-1/false +
> os_errno()（不杀进程）；参数个数/类型错 → px_error 终止（编程契约）。
>
> **stdlib `std.edge`（第 4 个 stdlib，纯语言封装，零新 C）**：GPIO V2 line uAPI
> （`gpio_input`/`gpio_output`/`gpio_input_edge`/`gpio_request` + `gpio_read`/`gpio_write`/
> `gpio_wait`/`gpio_event`，单线请求 592B 结构体布局按 linux/gpio.h 实测；边沿事件 24B
> 解析 timestamp/id/offset/seqno）、I2C（`i2c_open(bus,addr)` + `i2c_read_reg`/
> `i2c_write_reg`，write-then-read 两笔事务、无重复起始位）、串口（`serial_open(path,
> baud)` = open rw + tty_config raw 一站式）、PWM sysfs（`pwm_setup`/`pwm_enable`/
> `pwm_set_duty`，open 通道写失败不杀进程）。示例：m60_serial_pty.px（**x86 实跑 PTY
> 真内核串口 loopback**）、m60_gpio/i2c/pwm.px（真板段，x86 无设备 SKIP 通道）；
> dev_s3.px 布局常量断言（C offsetof 对照）。pxi 解释器白名单 +5 双模式同步 +
> aarch64 交叉 qemu 三态一致。边界：SPI_IOC_MESSAGE（transfer 数组含 u64 指针）现
> ioctl 无法承载语言 bytes 地址 → spidev read/write 半通、全双工留档；GPIO V2 结构体
> 布局按 2024+ 内核（offsets u32 版），旧内核需按目标头调整；真板物理回归单独立项。

---

### 8.19 外部系统库 FFI（zlib proof）+ 纯语言 2D 游戏内圈（M61，游戏线 0→1 地基）

> M61（docs/M61_PLAN.md）双线收口 GAP 候选池（无真板）：A = **FFI 外部系统库绑定**
> proof（游戏窗口线 0→1 机制地基），B = **纯语言 2D 内圈**（零 FFI/零硬件、产出可见
> 图片与可玩 demo，最大程度 dogfood M59+M60）。

**A 线 · FFI 外部库机制（真缺口 = 只绑过内部库）**

> 机制认知（M42/M46 先例）：`extern def` 并非自动 dlsym 任意 C 符号，而是查 **C 侧手写
> 注册表**（px_ffi_register）。故「绑定外部库」= ① 写薄 C 胶水注册 → ② 外部静态 .a 进
> pxc 链接行 → ③ 入库约定。M61 以 zlib（最普适 C 库）打通全链路并立模板：

- **外部 .a 两版入库**：`runtime/third_party/zlib/{include, lib, lib-aarch64}`，zlib 1.3.1
  源码自编（x86_64 gcc / aarch64-linux-musl 交叉，`tools/build_zlib.sh` 一键重建 + ar 抽
  crc32.o 架构校验）；保持纯静态哲学，零动态依赖。
- **pxc 链接分支**：`--zlib-lib <dir>` 显式指定，缺省按 `--cc` **自动架构探测**（gcc →
  `lib/`，aarch64-* → `lib-aarch64/`，旧交叉脚本免改）；libz.a 无条件链（~150KB，无 zlib
  引用时不抽成员，向后兼容）。
- **薄胶水 runtime_zlib.c**（注册 3 函数，进 FFI 表，双模式统一 bi_ffi_call C 桥）：
  - `zlib_crc32(data)` → int：crc32(0,data,len)，已知值可校验；
  - `zlib_compress(data, level)` → bytes：compress2 + **uLongf\* 长度指针**（cap→实际），
    语言侧免预分配；
  - `zlib_uncompress(data)` → bytes：z_stream inflate **渐进扩容**（免预知解压大小）。
  - str/bytes 均走 union data/len（二进制安全含 NUL）；数据非法 → 返回 null 不杀进程
    （与 M57 设备层哲学一致），参数/类型错误 → px_error 终止。
- **语言面**：`import "c/zlib"` + `extern def zlib_*(...)`，双模式一致。
- **验证**：m61_zlib.px —— crc32("hello")=0x3610a686 / 标准 check 0xCBF43926 / **纯语言
  CRC32 查表 5 组互证**（真实调用 libz.a 的旁证）/ 长文本 roundtrip 10400→110B /
  NUL 安全 / 空串 / 非法流 null / level 0·6·9；`nm` 实证产物含 crc32/compress2/inflate
  符号；**aarch64 交叉 + qemu 输出与 x86 diff 逐字节一致**（aarch64 libz.a 生效）。

**B 线 · 纯语言 2D 内圈（第 5/6 个 stdlib）**

- **std.gfx**（stdlib/gfx.px，画布 = `[w,h,pixels]`，pixels 每像素 0xRRGGBB）：
  `canvas_create/w/h/pixels`、`set_px`/`get_px`（边界裁剪）、`line`（Bresenham 整数）、
  `rect`/`fill_rect`、`circle`（中点 8 对称）/`fill_circle`（sqrt 弦扫描）、
  `blit`（spr=[sw,sh,colors]，0=透明跳过）、`text`/`text_size`（内置 5x7 位图，
  0-9 A-Z . - 空格，小写自动转大写、未知→?，compact 编码 bit(x+5y)）。零新 C 零 FFI。
- **std.png**（stdlib/png.px，纯语言 PNG 编码器 8bit RGB stored 无压缩）：
  签名 + IHDR/IDAT/IEND chunk + 纯语言 CRC-32 查表 + ADLER-32 + zlib stored block
  （CMF 0x78 01，≤65535/块，BFINAL 正确）。零 FFI、可移植（A 线联动压缩见下）。
- **示例**：demo_mandelbrot.px（640x480 复数迭代 + 11 色调色板）、demo_scene.px
  （全原语合成 + 8x8 透明棋盘 blit + 3 行 5x7 文字）；**m61_snake.px raw 终端可玩贪吃蛇**
  （w/a/s/d 控向 q 退出，O/#/@；交互走 tty_config raw + fd_wait + read 单键 = M60 设备组
  dogfood；SNAKE_AUTO=1 无头确定性剧本 EAT/SELF/WALL 三断言）。
- **跨架构确定性**：m61_s4_det.px 纯整数绘制（无浮点）PNG，x86 == qemu-aarch64
  **sha256 逐字节一致**（gfx 整数原语 + PNG stored 编码器跨架构确定）。
- **FFI 压缩联动（A+B 交汇）**：m61_s4_zpng.px 用 extern zlib_compress(level6) 直接产出
  压缩 IDAT → 标准压缩 PNG（python zlib 独立解码 chunk CRC 全过 + 像素抽查）。

**验证**：python3 stdlib zlib 独立解码所有产出 PNG —— chunk 遍历 + 每 chunk CRC32 全校验
+ IDAT 解压 + filter=0 行结构 + 像素颜色抽查（scene 太阳黄/底/边框蓝；mandelbrot M 集内部
黑/外部有色/标题白字）；pxi（重建后）可解释 zlib extern（双模式一致）；pxi 可 import
std.gfx 走纯 list 路径（text/blit 依赖 pxi 未同步 builtin 仍受限，stdlib 完整能力主打编译
模式，MINI_SUBSET §十三.6）。QQ 富媒体发送通道被平台拒（40093007）→ 图片落盘供自取。

**性能 dogfood 教训（留档）**：640x480 单帧 PNG —— mandelbrot ~34s / scene ~14.5s
（编译模式逐像素 list 存储 + bytes concat 拷贝为主）→ B 线后续画布优化方向 = bytes
三字节/像素 + 批量 blit（M61-PLAN D5 预案，正确优先已验证、性能按需再评估）。

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

> **M59 数学/随机内置（2026-09，C libm，docs/M59_PLAN.md）**：`sin(x)`/`cos(x)`/`tan(x)`
> （弧度）、`atan2(y,x)`（**先 y 后 x**）→ float；`floor(x)`/`ceil(x)`/`round(x)`
> （C99 .5 远离零，**返回 float**，需整数用 `int()`）→ float；`log(x)`（自然对数 ln）/
> `log10(x)`/`exp(x)`；`random()` → float∈[0,1)、`random_int(n)` → int∈[0,n)（n>0）、
> `random_seed(s)`（splitmix64 确定性，同 seed 同序列）；常量 `pi`/`e`（全精度 float）。
> 错误语义：参数个数/类型错误 → 终止（编程契约，同 abs/sqrt）；**域错误透传 C 不终止**
> （log(-1)→NaN、log(0)→-inf、exp(1000)→+inf）。返回的 float 一律按 IEEE754 全精度参与
> 运算；打印走**最短 roundtrip 全精度**（M63-L9：十进制指数∈[-4,15) 定点舒适区内
> `%.*f` 定点、区外 `%.*e` 科学，逐位 + strtod 回读取最短 roundtrip——0.1+0.2→
> "0.30000000000000004"、1/3→
> "0.3333333333333333"、1e15→"1e+15"、250.0→"250.0"；替代早期 `%g` 6 位截断）；
> **M62-L1（2026-09）起整值浮点打印补 `.0` 后缀**、M63-L9 起小数全精度
> （编译/解释双模式同根 fmt_num，见 MINI_SUBSET §十三.7/§十三.8）。

### 10.3 核心模块（v0.1 范围）
| 模块 | 内容 |
|------|------|
| `std.io` | print、read_file、write_file、read_stdin |
| `std.fs` | 文件系统：list、mkdir、remove、exists |
| `std.net` | http client/server、tcp（基础） |
| `std.json` | parse / stringify |
| `std.time` | now、sleep、duration |
| `std.string` | split、join、trim、upper、lower、contains、replace |
| `std.math` | abs、sqrt、min、max、pow（M59 起另有全局数学内置：sin/cos/tan/atan2、floor/ceil/round、log/log10/exp、random 族、pi/e，见 §10.2 注） |
| `std.collections` | list/map 扩展操作 |
| `std.edge` | 边缘设备（M60）：GPIO V2 line 控制/边沿、I2C 寄存器读写、串口 raw（serial_open）、PWM sysfs —— 见 §8.18 |
| `std.gfx` | 纯语言 2D 画布（M61）：line/rect/circle/fill_*/blit/text（5x7 字形）+ 精灵 —— 见 §8.19 |
| `std.png` | 纯语言 PNG 编码器（M61）：8bit RGB stored 无压缩，落盘可见图 —— 见 §8.19 |
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

**实现状态（M64/M65 工具链全自举）**：上表 `fmt`（M64a）/`lint`（M64b）/`test`/`doc`/
`bench`（M64c）/`lsp`（M65-S2/S3）/`mcp`（M65-S4）均已由 PuXian 自举实现
（`.px` 源码 → `bootstrap/px*` → `tools/pxc` 子命令：`pxc fmt / lint / doc / test /
bench / lsp / mcp`）；`ast` 已有 `astdump.px`（JSON/Debug AST），`pkg` 已有
`pxpkg`（M45）。**spec §12 表内 8 工具全部自举完成**。Rust 版全套留档
`archive/rust-compiler/`（只读）。

### 12.1 面向 AI agent 的协议
- 优先支持 **MCP**（Model Context Protocol），AI agent 可直接调用 `px` 工具链
- **已实现（M65-S4）**：`pxc mcp` —— MCP 2024-11-05 stdio transport，tools/list
  暴露 8 工具（run/fmt/lint/test/bench/doc/ast/version，带 inputSchema），
  tools/call 子进程执行（崩溃隔离不污染协议通道）。
- 错误信息机器可读（JSON 模式 `px build --json` / `pxc lint --json`），AI 可直接解析修复

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
