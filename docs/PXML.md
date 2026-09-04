# PXML —— PuXian 原生配置语言规范（v0.6，官方 docs 收编）

> 状态：v0.6 官方收编（M66-S3，qg-issue 04 → docs/PXML.md + stdlib/pxml.px）
> **Dogfood 闭环完成**：examples/m66_pxml/app.pxml 真实配置 + config_loader 加载器 + demo/demo_enc 全链路演示；
> 解析器 stdlib/pxml.px（import std.pxml）68 断言双模式 PASS；enc 四项定稿；**7 项开放问题全部拍板**；
> **格式定名 PXML = PuXian Markup Language**，由暂称 PAML 更名）
> 背景：token-cache-px 练手过程中，YAML 解析暴露出现有配置格式的深层问题。
> 本文回答三个问题：① YAML 是不是好的配置格式；② 现成替代品有哪些；
> ③ 要不要为 PuXian 生态自研一个原生配置格式（**v0.5 定名：PXML**）。
> v0.6 变更：**Dogfood 闭环完成**——新增 `dogfood/`（app.pxml + upstream.pxml + config_loader.px + demo.px + demo_enc.px），
> 用 PXML 描述仓库自身配置并真实加载：块/嵌套/数组/duration/include/env/ENC/引号键全特性端到端验证；
> demo.px 16 断言（双模式一致）、demo_enc.px 4 断言（ENC 加解密还原，build 产物）；
> 新增语言缺口记录：pxi 解释器缺 aes_gcm/base64/hex native 且明显慢于编译产物（见 5.6）；§5.7 新增 Dogfood 记录。
> v0.5 变更：7 项开放问题全部定稿（全称定名 **PXML = PuXian Markup Language**、裸标识符键维持现状、
> 不引入表数组、duration 单位收敛 ms/s/m/h/d、include 循环检测 v1 做且路径插值不做、
> 不要多文档/环境覆盖块、yaml→pxml 迁移器做且排后）；代码/API 更名 paml.px→pxml.px、
> paml_parse→pxml_parse；扩展名 `.paml`→`.pxml`；主密钥 env 名 `PAML_MASTER_KEY`→`PXML_MASTER_KEY`；
> §5.4 从"开放问题"转为"定稿决策"。
> v0.4 变更：**pxml.px 解析器实现**（当时名 paml.px，473 行，解释/编译双模式一致，67 断言全过）；
> 实现级决策收敛与 PuXian 语言缺口记录见 §5.6。
> v0.3 变更：**enc 四项决策全部定稿**——带名式 `ENC[...]` + kid 轮换位 + `env("PXML_MASTER_KEY")` 主密钥 + 懒解密；
> PXML 语法升为 v0.3 定稿草案（含 EBNF）；§5 重排（加密策略升为 5.3，与规格同级）。

---

## 1. 结论速览

1. **YAML 不是好的配置格式**——它是"数据序列化语言"被生态抬进配置圈的产物，规范爆炸、隐式类型是歧义温床。
2. **现成替代首选 TOML**；JSONC/HCL/KDL 各有适用面。
3. **但对 PuXian 生态，值得自研原生配置格式**：与语言"薄而直白 + 内置精选"气质吻合，且 json/xml 已内嵌 runtime（L0），自家配置语言能进 L0 就是差异化。
4. **v0.5 定名：PXML = PuXian Markup Language**（由暂称 PAML 更名；Annotative 是生造词已弃），文件扩展名 `.pxml`；解析库 `pxml.px`，入口 `pxml_parse`。
5. PXML 与 YAML 的关系是**并存而非取代**：YAML 做存量兼容导入（`yaml.px` 即桥），PXML 做新项目默认。
6. **加密立场（v0.3 定稿，v0.5 更名）**：敏感值不落盘（`env()` 引用）为第一道防线，字段级加密 `ENC[...]` 为第二道防线——**v1 直接内置**；**否决整文件加密**（伪需求，见 5.3）。enc 值用 **sops 风格带名式 `ENC[alg,kid:..,data:..,iv:..,tag:..]`**，主密钥只走 `env("PXML_MASTER_KEY")`，密文自带 kid 支持轮换，消费时懒解密。

---

## 2. YAML 为什么不是好选择

YAML 本是数据序列化语言，被 K8s/CI 生态抬进配置圈。圈内普遍"离不开又恨得牙痒"。缺点分四层：

### 2.1 规范复杂度爆炸，配置只用到 20%
- 完整 YAML 1.2 规范 80+ 页：锚点/别名/合并键/多行块/多文档/标签/流式集合……配置几乎全用不上，却逼每个解析器实现全部。
- 解析器实现差异大：YAML 1.1 vs 1.2 行为不同，各家库行为不一致（同一文件不同解析器结果可能不同）。
- 安全隐患：反序列化炸弹 / 别名递归（billion laughs 同类攻击面）。

### 2.2 隐式类型转换 = 歧义温床（最坑）
- **Norway 问题**：YAML 1.1 中 `no` → false，国家代码 `country: NO` 被解析成布尔；`yes/on/off` 同理。
- `version: 1.0` 存进去变浮点数；`1:30` 变 90 秒（六十进制）；版本号/时间串到处踩坑，必须加引号防身。
- 类型靠"猜"：`3.0.0` 是字符串、`1.0` 是数字、`1e3` 是数字……规则微妙到人记不住。

### 2.3 缩进语义对工具不友好
- diff/merge 冲突极易破坏结构（复制粘贴一段就破格）。
- tab/空格混用报错晦涩。
- 机器生成/修改困难（没有括号/块边界兜底）。

### 2.4 复用与校验能力弱
- 无原生 include/继承（各家自己拼文件 = 本仓库 config 分片那套 workaround）。
- 无 schema 标准（要另上 JSON Schema / CUE）。
- `#` 注释规则绕：URL 里的 `#` 必须引号，行内注释位置讲究。

---

## 3. 现成替代品盘点

| 格式 | 强 | 弱 | 适合 |
|---|---|---|---|
| **TOML** | 为配置而生：显式类型、无歧义、规范小、解析器简单 | 深层嵌套/复杂数组表达笨重 | 中浅层配置，**最推荐的现成替代** |
| **JSON5/JSONC** | 类型显式、机器友好、有注释 | 手写啰嗦（引号多） | 工具配置/机器生成 |
| **HCL** | 表达式/函数/块结构，HashiCorp 生态证明 | 学习曲线、偏重 | 基础设施即代码 |
| **KDL** | 新、简洁、显式类型、规范小 | 生态未成熟、人写略陌生 | 值得盯的明日之星 |
| YAML 子集 + schema | 兼容存量 | 治标不治本 | 存量迁移过渡 |

> 结论：**如果只是给现有项目选格式 → 用 TOML，不折腾自研。**
> 下面的 PXML 是"为 PuXian 语言生态"而做，不是为单一项目。

---

## 4. 为什么 PuXian 值得有自己的配置格式

1. **气质吻合**：PuXian 薄核心主张 "能力内嵌 runtime，不靠包生态"，json/xml 已 L0 内嵌。若配置语言也能 L0 原生解析、零依赖、开箱即用，是自然延伸。
2. **可绕开 YAML 全部先天缺陷**：显式类型（消灭 Norway）、无缩进语义（消灭 tab 地狱）、原生注释/块注释、原生 include/环境变量插值、报错带行号。
3. **Dogfood 价值**：编译器、网关、所有自家服务统一用它，边用边磨，反过来锤炼语言。
4. **差异化**：主流语言没有"官方配置语言"先例（TOML 是第三方），这是 PuXian 品牌记忆点。
5. **安全卖点**：自家生态坚持"配置里无明文密钥"（env 引用 + 字段级 enc），本身就是可对外讲的差异点。

---

## 5. PXML 规格（v0.5 定稿）

### 5.1 设计原则
- **一切类型显式，拒绝魔法**：不搞 `yes/no/on/off` 隐式布尔，不搞六十进制。
- **无缩进语义**：块用花括号 `{}`，数组用方括号 `[]`，消灭 tab 地狱与破格。
- **语法面小到"一个下午能实现完 C 解析器"**：为将来 L0 内嵌留余地。
- **原生 include 与 env 插值**：解决分片 workaround 与密钥落盘问题。
- **敏感值不落盘**：默认用 `env("...")` 引用环境变量；**实在要落盘/进仓库的敏感字段，用字段级加密 `ENC[...]`**（v1 内置）。
- **解析器永不碰密钥**：`ENC[...]` 对解析器只是"带标记的字符串"，识别保留、不解密；解密只发生在消费方运行时。
- **报错带行号**：解析错误输出 `line N: ...`。

### 5.2 语法（v0.5 定稿）

#### 5.2.1 示例

```pxml
# PXML v0.5 示例（定稿）

server {
    port = 8080            # 行尾注释：数字就是数字，没有 yes/no 把戏
    host = "127.0.0.1"     # 字符串必须引号（单双皆可），多行字符串用 """..."""
    enabled = true         # 布尔只有 true/false 字面量，别无分号
    ttl = 720h             # duration 字面量原生（单位收敛 ms/s/m/h/d）
    tags = ["llm", "cache",]
    limits = { max = 100, min = 1 }
}

upstream = include("llm.pxml")   # 原生 include，相对当前文件
api_key  = env("LLM_API_KEY")    # 环境变量显式取，不落盘（第一道防线）

# 字段级加密（第二道防线）：v1 内置；结构可读、可提交 git
db_password = ENC[aes256gcm, kid:v1, data:q0X9t2Z..., iv:7a1c4e..., tag:9c4ef2...]
```

#### 5.2.2 EBNF（定稿）

```ebnf
document    ::= statement*
statement   ::= assignment | block
assignment  ::= key "=" value (eol | eof)
block       ::= key "{" statement* "}"
value       ::= literal | array | dict | call | enc

literal     ::= string | number | bool | duration | null
string      ::= '"' text '"' | "'" text "'" | '"""' text '"""'
number      ::= int | float
bool        ::= "true" | "false"
null        ::= "null"
duration    ::= number unit            (* unit = ms | s | m | h | d  v0.5 收敛 *)
array       ::= "[" (value ("," value)*)? ","? "]"
dict        ::= "{" (key "=" value ("," | eol))* "}"
call        ::= ident "(" (string ("," string)*)? ")"   (* 现仅 include / env *)
enc         ::= "ENC[" encfield ("," encfield)* "]"
encfield    ::= "alg:" alg | "kid:" ident | "data:" b64 | "iv:" b64 | "tag:" b64

key         ::= ident | string
ident       ::= [A-Za-z_][A-Za-z0-9_]*
alg         ::= [A-Za-z0-9_-]+          (* v1 只承诺 aes256gcm *)
b64         ::= [A-Za-z0-9+/=]+
comment     ::= "#" ... eol             (* 整行或行尾；字符串与 ENC 内部 # 不剥离 *)
```

要点与 v0.5 定稿决策：
- **块 `{}` 与 dict `{}` 同形**：`key { ... }` 等价于 `key = { ... }`（块内语句以换行/逗号分隔）。
- 顶层仅允许 `assignment`（`key = value`）；嵌套（dict/block）内允许再嵌套 block 与 dict。
- **裸标识符键维持现状**：`[A-Za-z_][A-Za-z0-9_]*` 可用裸键，其余必须引号——不放开，省引号且无 YAML 键乱象。
- **不引入表数组**（TOML `[[servers]]` 是"无嵌套括号"的补丁式复杂度）：需要同类重复块时用 dict 数组兜底，如 `servers = [ { name="a" }, { name="b" } ]`。
- **duration 单位收敛 ms/s/m/h/d**：ns/us 属程序级精度、配置里是噪音，不再支持（解析器报"未知单位"）；`1.5s` 这类小数保留。
- **include 循环检测 v1 就要做**（消费方展开时维护"已加载文件栈"，成本低防炸）；**路径插值不做**（表达式拼路径超出配置语言职责，用环境变量/外部拼路径解决）。
- **不要多文档（`---`）、不要环境覆盖块**：dev/prod 差异靠"基础 pxml + 覆盖 pxml + include 拼装 + env 引用"。
- `call` 现仅 `include("path")` 与 `env("NAME")`，均返回字符串语义（消费方展开）。
- `ENC[...]` 被解析为**单一字符串字面量**（带标记），parser 不做任何解密/内容校验。

### 5.3 加密策略：字段级 enc（v0.3 定稿，v0.5 更名）

#### 先拆概念：两种"配置文件加密"，方向天差地别
**A. 整文件加密 —— 否决（伪需求）**

| 坑 | 后果 |
|---|---|
| 鸡生蛋 | 解密要口令/密钥，这口令放哪？还是得 env 或交互，等于没省事还多一层 |
| git diff 报废 | 加密后整文件是乱码，code review 没法做，历史不可追溯 |
| 改一个字段全重来 | 轮换密钥 = 全部重新加密 |
| 明文泄漏面不减 | 运行时日志、备份、core dump、调试打印照样可能泄漏解密后的明文 |
| 无头重启 | 服务器重启谁输口令？部署脚本被迫复杂化 |

**B. 字段级加密 —— 采用（sops 风格）**
只加密 `api_key`、`db_password` 这类真正敏感的值，其余保持明文可读、可 diff、可 review（Mozilla sops 已验证此路）。

#### 根本原则：Secrets 不该进配置文件
```
env()  → 运行时从环境取（不落盘）        ← 首选，覆盖 90% 场景
ENC[]  → 落盘但字段级加密（可提交 git）  ← 兜底，结构可读
整文件加密                             ← 否决
```

#### v0.3 拍板定稿的四项决策
1. **值文法：sops 风格带名式 `ENC[...]`**
   ```pxml
   db_password = ENC[aes256gcm, kid:v1, data:<b64密文>, iv:<b64>, tag:<b64>]
   ```
   - 字段带名 → 后续扩展（加盐/加 AAD/换算法）**不改文法**，解析器与旧密文兼容。
   - `alg` 为必填首字段，v1 只承诺 `aes256gcm`（PuXian runtime 已有 AES 内建）。
2. **主密钥来源：v1 只支持 `env("PXML_MASTER_KEY")`**
   - 密钥永不落盘；KMS / 密钥文件往后放（v2+）。
3. **kid（密钥标识）：v1 就带**
   - 密文自描述"用哪把钥"，实现"换主钥不重加密全部"的基础（新 kid 追加，旧 kid 保留解密）。
4. **解密时机：懒解密**
   - 消费方用到该字段时才解密，减少明文在内存的驻留窗口。

#### 分层铁律
`ENC[...]` 的**识别**（parser）与**解密**（consumer runtime）必须分离：
- parser 看到 `ENC[` 就整体识别为一个标记字符串 —— 纯函数、零密钥依赖、可测试；
- 解密发生在消费方加载配置时：`kid` 选钥 → `env("PXML_MASTER_KEY")` 取主密钥 → 解密 `data`（校验 `tag`/`iv`）。

#### 加密分代（v1 直接带，不做"先无后有"）
| 版本 | 能力 | 状态 |
|---|---|---|
| **v1（当前定稿）** | `env()` 引用 + 字段级 `ENC[aes256gcm,kid,data,iv,tag]`、懒解密、主密钥 env | 语法已定稿，pxml.px 已实现（enc 识别） |
| v2+ | KMS / 密钥文件、更多算法位、加解密审计 | 视需要 |

### 5.4 定稿决策（v0.5：7 项开放问题全部拍板，不再开放）

| # | 问题 | v0.5 定稿 |
|---|---|---|
| 1 | 全称定名 | **PXML = PuXian Markup Language**（弃 Annotative 暂称）；扩展名 `.pxml`；解析库 `pxml.px` / 入口 `pxml_parse`；主密钥 env `PXML_MASTER_KEY` |
| 2 | 裸标识符键 | **维持现状**：`[A-Za-z_][A-Za-z0-9_]*` 可裸键，其余必须引号 |
| 3 | 表数组 | **不引入**：同类重复块用 dict 数组兜底 `servers = [ {…}, {…} ]` |
| 4 | duration 单位 | **收敛 ms/s/m/h/d**（ns/us 移除，解析器报未知单位） |
| 5 | include | **循环检测 v1 做**（消费方文件栈）；**路径插值不做**（环境变量/外部拼路径解决） |
| 6 | 多文档 / 环境覆盖块 | **都不要**：dev/prod 差异靠"基础 pxml + 覆盖 pxml + include + env"拼装 |
| 7 | yaml → pxml 迁移器 | **做，排后**（不阻塞 v1）：yaml.px 现成，等 PXML dogfood 跑稳后写 `yaml2pxml.px` |

> 已定稿（更早）：enc 值文法（带名式 ENC）、主密钥来源（env）、kid 轮换位（v1 带）、解密时机（懒）——见 5.3。

### 5.5 实现路线建议
1. ~~yaml.px 已成~~ → **pxml.px 已实现**（v0.4 名 paml.px，v0.5 更名 pxml.px；466 行，68 断言双模式 PASS）；语言基础已到手，可 dogfood 使用。
2. **enc 与解析器解耦**（已实现）：`pxml.px` 只做"识别 `ENC[...]` 标记并原样保留"（纯函数、零密钥、可测试），加解密单独成消费方库（`dogfood/config_loader.px` 已落地，复用 runtime AES），可独立测试。
3. **先定 stdlib 边界**：PXML 若进 L0，需 C runtime 实现（C 解析器，一个下午量级），届时与 PuXian 核心仓库协作。
4. **Dogfood 已起步**（v0.6 完成）：`dogfood/` 已用 PXML 描述仓库自身配置并真实加载（见 §5.7），跑稳后可外溢给其他服务。
5. **yaml → pxml 迁移器（yaml2pxml.px）排后做**（v0.5 拍板）：递归转换 yaml.px 的解析结果为 pxml 文本。

### 5.6 实现记录（v0.4 落地，v0.5/0.6 更新）
代码见 `pxml.px`（<500 行约束内）；测试 `test/pxml_test.px`（68 断言，`pxc run` 解释 + `pxc build` 编译双模式一致，产物静态 ELF）。v0.5 起文件/API 由 paml.px / paml_parse 更名，duration 单位收敛。

**实现级决策（v0.4 收敛，v0.5 更新）**
- `duration` 字面量（`720h`/`1.5s`）换算为**纳秒 int**（`1d = 86400e9` ns）——与 Go `time.Duration` 语义对齐；解析器纯换算、不校验正负。v0.5 起仅支持 ms/s/m/h/d。
- `env("X")` / `include("p")` 解析为 **call 节点** `{"__call":"env","args":["X"]}`（双下划线保留键，避免与业务键碰撞）；parser 不执行、不做 IO。
- **键重复报错**（非后者覆盖）：易发现拼写/合并错误。
- 字符串**不支持 `\uXXXX` 转义**（直接写 UTF-8，省 codepoint 转换代码）；支持 `\n \t \r \" \\ \/`、单引号 `''`、三引号跨行原样。
- 无缩进语义**已验证**：空格/tab/缩进随意，不产生语法含义；裸 `http://` 遇 `:` 即报错并提示用 `=`（YAML 走 yaml.px）。

**PuXian 语言缺口（写 stdlib/库的人的笔记，首批"缺口补丁清单"素材）**
1. **模块级 `var` 在函数内不可读**（R1001 未定义变量）——函数只能访问局部变量，模块级共享状态不可行；库必须**纯函数显式传状态**（yaml.px/pxml.px 均为"传 idx/pos 返回状态"风格）。
2. **不支持 `;` 语句分隔**（词法 E1001 非法字符）；单行 `if x: return ...` 编译报错 E2001（期望换行），**控制流语句体必须换行**；单行 `if x: 表达式/赋值` 部分场景解释模式可跑但编译模式不保证，保守全部拆多行。
3. 数组/容器字面量**跨行受限**（`let a = [\n ...` 报"意外的 token: 换行"）——多元素逐行 `append`。
4. `let` 默认不可变（编译模式 E3002），需改值用 `var`/`let mut`——解释模式宽容、编译模式严格，**务必双模式都验**。
5. **`pxc run`（pxi 解释器）缺 aes_gcm/base64/hex 等 native**（build 产物才有）；且解释器明显慢于编译产物（demo.px：解释 ≈50s vs 编译 <1s）——**加密类库与耗时演示请以 `pxc build` 产物验证为准**。这是 Dogfood 实测暴露的差异（记录于 v0.6）。
6. 字符串**没有 `find` 方法**（取字段报错），解析分隔请用 `split`；`join` 参数顺序为 **(分隔符, list)**（与直觉相反，Dogfood 实测踩坑）。

### 5.7 Dogfood 闭环（v0.6 完成）

**目的**：PXML 语法定稿 + 解析器就绪后，用"自己的配置"做端到端闭环，验证格式在真实场景可用，而不是只跑单测。

**落地**：`dogfood/` 目录，全链路加载验证：

| 文件 | 作用 |
|---|---|
| `app.pxml` | 主配置：app/server/llm/cache/database 块 + duration + 数组 + 引号键 + include + env(带默认值) + ENC |
| `upstream.pxml` | include 分片：嵌套数组/dict、duration（被 app.pxml include） |
| `config_loader.px` | **消费方加载器**：read_file → pxml_parse → 递归展开 env/include → ENC 懒解密（分层铁律落地） |
| `demo.px` | 加载验证：16 断言（块取值、720h 纳秒、include 展开、env 默认值、引号键、ENC 保留） |
| `demo_enc.px` | ENC 加解密验证：runtime AES 生成标准 ENC 密文 → 写入临时配置 → 加载解密还原（4 断言） |

**演示命令**（仓库根执行）：
```bash
pxc build dogfood/demo.px && ./dogfood/build/demo        # 16 断言 PASS（编译模式）
pxc run dogfood/demo.px                                  # 解释模式也可跑（约 50s，慢）
PXML_MASTER_KEY=0123456789abcdef0123456789abcdef pxc build dogfood/demo_enc.px
PXML_MASTER_KEY=0123456789abcdef0123456789abcdef ./dogfood/build/demo_enc   # 4 断言 PASS
```

**分层铁律在 loader 中的实现要点**：
- `config_loader.px` 里 `lx_enc_parse` 只拆 `ENC[...]` 结构字段（alg/kid/data/iv/tag），不做解密；
- `lx_enc_decrypt` 才是消费方解密（`aes_gcm_decrypt` + `PXML_MASTER_KEY`），**不传密钥 / 非 aes256gcm / tag 校验失败一律返回原标记字符串**（懒解密安全兜底）；
- include 用**已加载路径栈**做循环检测（v0.5 定稿项 #5 落地）；
- `env("X","默认")` 支持第二参数默认值（demo 中 `base_url` 演示）。

**Dogfood 结论**：
1. 语法面覆盖真实需求够用（块/嵌套/数组/duration/include/env/ENC/引号键全落地，无需新增特性）；
2. 解析器（pxml.px）与消费方（config_loader.px）职责分离清晰，ENC 懒解密路径闭环可用；
3. 暴露 2 个新语言缺口（见 5.6 #5 #6：pxi 缺 native + 慢、string 无 find / join 参数序），后续反馈给 PuXian 主仓；
4. 下一步候选：把仓库构建/工具配置切 PXML；写 yaml2pxml 迁移器（v0.5 拍板排后项）；stdlib 收编讨论。

---

## 6. 相关仓库现状

| 仓库 | 内容 | 状态 |
|---|---|---|
| `NanzhanGroup/token-cache-px` | yaml.px（存量兼容桥）+ pxml.px + dogfood/（PXML 自举配置与加载器）+ docs（本文件） | 本文所在 |
| `NanzhanGroup/PuXian` | 语言本体（含 stdlib/方向文档） | 收编 std.yaml / 未来 PXML 的归宿 |

---

## 7. 变更记录
- v0.6：**Dogfood 闭环完成**——新增 `dogfood/`（app.pxml + upstream.pxml + config_loader.px + demo.px + demo_enc.px），仓库自身配置用 PXML 描述并真实加载：demo.px 16 断言双模式 PASS、demo_enc.px 4 断言（ENC 生成→写入→解密还原，build 产物）；新增 §5.7；语言缺口扩至 6 条（pxi 解释器缺 aes/base64/hex native 且慢、string 无 find / join 参数序）；§5.5 路线与 §6 现状更新；README 全面转 PXML + Dogfood。
- v0.5：7 项开放问题全部拍板定稿；格式更名 **PAML → PXML**（PuXian Markup Language，弃 Annotative 暂称），扩展名 `.pxml`、文件/API 更名 pxml.px/pxml_parse、主密钥 env 更名 `PXML_MASTER_KEY`；duration 单位收敛 ms/s/m/h/d（ns/us 报错）；§5.4 转为定稿决策表；路线加 yaml2pxml 迁移器（排后）。
- v0.4：pxml.px 解析器实现（当时名 paml.px，466 行，68 断言双模式 PASS）；新增 §5.6 实现记录（duration 纳秒、call 节点、重复键报错、无 \u 转义、无缩进语义验证 + PuXian 语言缺口）；§5.5 路线更新。
- v0.3：enc 四项决策定稿（带名式 `ENC[alg,kid,data,iv,tag]` + env 主密钥 + kid 轮换位 v1 内置 + 懒解密）；5.2 升为"语法定稿草案"并补 EBNF；加密策略升为 5.3；5.4 待定问题收敛为 7 项（enc 相关移出）；5.5 路线更新（yaml.px 已成，可动工 paml.px）。
- v0.2：新增加密策略——明确否决整文件加密、采用字段级 `enc:`（sops 风格）、v1 直接内置；5.1 设计原则与实现路线同步更新；增补 enc 相关开放问题。
- v0.1：初稿（YAML 分析 + 候选盘点 + PXML 草案），格式名由 pconf 改为 paml（v0.5 起更名 PXML）。
