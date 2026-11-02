# M372 立项 · data enum 载荷（三轨构造 + match 解构）

> 令源：用户 2026-11-02 08:20「门的问题解决了就开始产品面，自主立项，自主决策，持续推进」。
> 前置：M371（装置面收口）已 GREEN 247/247，tag `v0.2.371` 已推。
> 本文是**实施前的语义定稿**。未实现的部分一律照实标注，不写「已支持」。

## 一 现状（M372 开工前实测，HEAD 2482584+）

| 面 | 状态 |
|---|---|
| 语法 | ✅ 已收（M367-S1）。`Circle(radius: float)` → `EnumField{name, ty, pos}`；旧写法 `Circle(float)` 继续接受 |
| 解释轨 | `Shape.Circle(1.5)` → `R1005 枚举变体 Shape.Circle 不接受参数` |
| VM / C 轨 | 同码同文（M366 缺陷 634 收口） |
| spec §3.8 | 明写「载荷未实现，请用 struct + 可空字段」 |
| 值模型 | 解释轨 `it_enum(t, v, payload)` **已有 payload 槽**（恒 null）；runtime 的 `enum_inst` **没有** payload 字段 |

⇒ 唯一「文档写了、三轨都没兑现」的产品面特性。选它做 M372。

## 二 语义定稿（用户可见契约）

### 2.1 声明

```python
enum Shape:
    Circle(radius: float)     # 具名字段
    Rect(w: float, h: float)
    Px(x: int, y: int)        # 旧写法：字段名缺省
    Dot                        # 无载荷变体
```

- **具名字段与旧写法（只有类型）并存**，与 parser 当前行为一致。
- 旧写法字段的**内部名 = 位置序号** `"0"`、`"1"`…（不暴露给用户；解构按位置绑定）。
- **旧写法保留历史行为（兼容 M367 门 D 用例）**：`enum Vec: V(float)` 的 `Vec.V` 仍可**无载荷取值**
  （payload = null），`Vec.V(1.5)` 也能构造载荷。判据在 `it_enum_pos_style`：字段名恰为 `0..N-1`
  ⇒ 位置式（标识符不能以数字开头，用户写不出这种字段名）。**具名字段变体则必须给载荷**。
- 字段类型只作**文档/静态检查**（本仓动态类型为主），本期不做运行期类型强校验 —— 与 `struct` 同口径。

### 2.2 构造

```python
let a = Shape.Circle(1.5)                    # 位置实参，按声明序
let b = Shape.Rect(w: 2.0, h: 3.0)           # 命名实参（复用 argmap.px，与 struct 同规则）
let c = Shape.Rect(2.0, h: 3.0)              # 混合：位置只填未被命名占用的字段
let d = Shape.Dot                             # 无载荷变体：不带括号
```

判据（三轨同码同文）：

| 情形 | 结果 |
|---|---|
| 有字段变体写 `Shape.Circle`（无括号） | `R1005: 枚举变体 Shape.Circle 需要 1 个字段，给出 0；请写 Shape.Circle(...)` |
| 无字段变体写 `Shape.Dot()` | `R1005: 枚举变体 Shape.Dot 不接受参数，给出 0` |
| 个数不符 | `R2001: 枚举变体 Shape.Rect 需要 2 个字段，给出 1` |
| 名字不存在 | `R2001: 枚举变体 Shape.Rect 没有字段「x」` |
| 参数重复 | `R2001: 枚举变体 Shape.Rect 字段「w」重复给出` |
| 缺字段（命名参数） | `R2001: 枚举变体 Shape.Rect 缺少字段「h」` |
| 位置实参出现在命名实参之后 | `E2001`（parser，与 struct 同规则） |
| 变体不存在 | `R1008: 枚举 Shape 没有变体 'Nope'`（不变） |

> 个数/名字类用 **R2001**（与结构体构造同一族、同一份 `argmap_fields` 实现）；
> **只有**「载荷放不下」（无括号取值、无字段变体给实参）用 R1005。

> 为什么无括号的 `Shape.Circle` 报错而不是造「载荷全 null」的值：静默 null 是本仓反复踩过的坑
> （M116/M120/M163）。**要空载荷就声明无字段变体**。

### 2.3 解构（match）

```python
match s:
    case Shape.Circle(r):
        print(r)
    case Shape.Rect(w, h):
        print(w, h)
    case Shape.Dot:
        print("dot")
```

- 子模式个数必须等于字段个数（多/少都**不匹配**，落到下一个 arm；与现有 PatTuple 语义一致）。
- 子模式按**声明序**绑定（位置），支持嵌套模式/字面量/`_`。
- 不带子模式的有字段变体（`case Shape.Circle:`）**匹配**该变体（不绑定），不报错 —— 与「只问变体」的直觉一致。
- 现有 PatConstructor 在 C/VM 轨对「带子模式」恒假（M239 注释：data enum 未支持）⇒ 本期**真绑定**。

### 2.4 值语义

- **相等**：`(类型名, 变体名, 载荷)` 全等。⚠️ 解释轨 `i_eq` 已经比 payload，runtime 的
  `compare_values`（PX_ENUM 分支）**只比 variant** ⇒ 本期必须补齐，否则载荷一落地三轨立刻分叉。
- **排序**：仍只按变体名（同变体现状），载荷不参与 —— 与解释轨 `i_cmp` 一致，不改。
- **打印**：`Shape.Circle({0: 2.0})` —— **含载荷**。
  解释轨 `i_to_str` 从 M237 起就预留了这个格式（`Type.Variant(payload)`，payload 为 null 时退化回
  `Type.Variant`）⇒ **以它为准**，S3 让 runtime 的 `px_fmt_value`（PX_ENUM 分支）对齐，
  而不是反过来删掉解释轨的既有行为。
- **repr/序列化**：不承诺。
- **可变性**：载荷只读。`s.radius` 值级字段访问**本期不做**（用 match 解构）；
  `Shape.Circle.radius` 之类仍报 R1007。留着是为了不把「enum 值 = 结构体」这个错误直觉固化。

## 三 实现方案（值模型 = dict 载荷，最小改 runtime 面）

选 dict 而不是「按字段名平铺进 enum_inst」：dict 已有 GC 标记、`px_field` 读、等值比较，三轨都能复用，
runtime 只需加**一个字段 + 两个桥函数**。

### 3.1 runtime（改 ⇒ 必须重烘）

- `runtime.h`：`enum_inst` 增 `LXValue payload;`；声明
  `LXValue px_enum_payload(const char*, const char*, LXValue)`、
  `LXValue px_enum_payload_of(LXValue)`。
- `runtime.c`：
  - `px_enum()` 初始化 `payload = px_null()`（不给载荷的既有调用点语义不变）；
  - 新增两个桥函数（`px_enum_payload_of` 非 enum → null）；
  - GC 标记：`PX_ENUM` 分支补 `mark(payload)`；
  - `compare_values` 的 `PX_ENUM` 分支：先比 type_name+variant，再 `compare_values(payload, payload)`；
  - `px_field(PX_ENUM)` 保持 R1007（不暴露字段名），除非 payload 为 dict 且键命中 ⇒ **不**开这个口子（见 2.4）。

### 3.0 parser（S2 已改）

- `parse_pattern` 的 `名字.成员` 分支：成员名后跟 `(` ⇒ `PatConstructor(成员名, 子模式)`；
  不跟 `(` ⇒ 保持 `PatLiteral(Field)`（`type X const` 常量组按值匹配，enum 变体按变体名匹配）。
  **修前** `case Shape.Circle(r):` 直接 `E2001 期望 ':'，实际得到 (` —— data enum 连 match 都写不出来。
- 载荷 dict 的键统一为**位置序号** `"0"…"N-1"`（字段名只在构造时做命名参数映射），
  这样旧写法 `Circle(float)` 与具名写法共用同一表示，三轨不必各带一张字段名表。

### 3.2 解释轨（selfhost · S2 已完成）

- `interp.px` 新增 `g_enum_fields`（`enum → {variant → [字段名...]}`），`istmt.px` 在 `i_exec_enum` 填。
  （`g_enums` 的「变体名列表」保留，避免动 20+ 处 `contains` 调用。）
- `argmap.px` 抽出 `argmap_fields(field_names, args, owner, kind)`，`argmap_struct` 转调 ——
  报文里的「结构体」可换成「枚举变体」，装配算法仍是同一份。
- `iexpr.px`：
  - `i_enum_build`：按书写顺序求值（`NamedArg` 取内层表达式）→ `argmap_fields` 定序 → 装 dict；
  - `i_enum_ctor`（`Color("Red")` 形态）：有字段变体 ⇒ R1005「需要 N 个字段，请用 X.Y(...) 构造」；
  - 类型名成员调用（`Shape.Circle(...)`）：变体存在 ⇒ `i_enum_build`，否则 R1008；
  - `i_match_pattern` 的 `PatConstructor`：payload 按 `"0"…` 解构，子模式个数不等 ⇒ 本 arm 不匹配；
  - `PatLiteral` 的 `Field` 若指向 enum 变体 ⇒ 只比类型+变体（载荷不参与）。
- `ival.px`：`i_field` 类型名分支，有字段变体 ⇒ R1005（无括号不许造值）。

### 3.2.1 S2 实测（2026-11-02 · /tmp/pxidev）

```
Shape.Circle({0: 2.0})
Shape.Rect({0: 3.0, 1: 4.0})
12.0 / 12.0 / 15.0 / 16 / 0        # area(): Circle/Rect 命名/Rect 混合/Px/Dot
true / false / true                 # 相等：载荷参与
px 7,9                              # match 解构
any circle                          # case Shape.Circle: 匹配任意载荷
v=2.0                               # 嵌套字面量模式
```
错误路径 7 条逐条核对（R1005 两条 · R2001 三条 · E2001 一条 · 多参 R2001 一条）。

⚠️ **S2 结束时 `Shape.Circle(1.5)` 只有解释轨能跑**；编译两轨仍报 R1005（原样）。这是明确的分阶段状态：
**不许打 tag、不许写「已实现」**，CHANGELOG 与提交信息必须写明「编译轨未落地」。

### 3.3 VM 轨

- `vm.h`：新增 `PXOP_ENUMPAY`（`a=dst, b=obj 槽`，取 payload；非 enum → null）+ `PXOP_ENUMNEWP`
  （`a=dst, b=类型名 N idx, c=变体 N idx, fl=载荷槽`）。不动 `NEWENUM` 语义（旧 BC 兼容）。
- `vm.c`：两指令实现；`PXOP_ENUMVAR` 不变。
- `bc_emit.px`：
  - `g_bcm["enums"]` 升级为 `{变体名: [字段名...]}`（`bc_enum_has` 改判键存在）；
  - 构造 ⇒ `NEWDICT`（"0"…）或复用 argmap 展开 ⇒ `ENUMNEWP`；
  - `bc_match_cond` 的 `PatConstructor` 带子模式：`ENUMPAY` + `GETF("i")` + 递归子模式（不再恒假）。

### 3.4 C 轨

- `codegen.px`：`cg_enums` 升级为 `{变体名: [字段名...]}`。
- `cg_expr.px`：构造发 `px_enum_payload("T","V", <dict>)`；`PatConstructor` 子模式发
  `px_field(px_enum_payload_of(subject), "0")` 递归匹配。

### 3.5 spec

- §3.8 状态行从「载荷未实现」改为「✅ 已实现（M372）」+ 2.2/2.3 契约；删「请用 struct + 可空字段」。

## 四 门（新门 `m372_data_enum`）

`examples/m372_data_enum/verify.sh`，按 NEW_GATE_CHECKLIST 六格：

1. **正控**：三轨各跑同一份 `main.px`（构造/命名实参/混合/多字段/无载荷/match 解构/嵌套模式/相等/打印），
   逐字节对拍 stdout + rc。
2. **值语义格**：`Shape.Circle(1.0) == Shape.Circle(1.0)` 真、`!= Shape.Circle(2.0)` 真
   （**这格专门盯 2.4 的编译轨 compare_values 缺口**）。
3. **错误格**：2.2 表里 6 条错，三轨同码同文（含「无括号有字段变体」「Dot()」两条新口径）。
4. **负控**：把 runtime 的 `px_enum_payload` 改成忽略载荷 ⇒ 值语义格必红；
   把 match 子模式绑定改成恒假 ⇒ 解构格必红。
5. **前提自证**：`grep` 确认被测二进制确实是本次构建（产物指纹），不是旧 bootstrap。
6. **门自伤检查**：新门脚本本身先过 `pkg_pipe_grep` + `m283_gate_premise`（M371 教训）。

## 五 分步与代价

| 步 | 内容 | 代价 |
|---|---|---|
| S1 | 本文定稿 + 交接 | 已完成 |
| S2 | runtime 承载 + 解释轨 + 门（先覆盖解释轨，编译轨仍报 R1005） | 2–3h |
| S3 | VM 轨 + C 轨 + 三轨对拍门转正 | 3–4h |
| S4 | 重烘 14 件 + 全量门（248 门）+ tag v0.2.372 | 重烘 ~15min + 门 ~95min |

⚠️ S2 结束时 `Shape.Circle(1.5)` 会「解释轨能跑、编译两轨报未实现」—— **这是明确的分阶段状态，
不许打 tag、不许写「已实现」**；S2 的提交信息与 CHANGELOG 必须写明「编译轨未落地」。
S3 完成前 CI 全量门必须仍绿（新门在 S2 阶段只跑解释轨，标注 `M372-S2`）。

## 六 不做（本期明确排除）

- 载荷字段的**值级字段访问**（`s.radius`）—— 容易把 enum 误解成 struct。
- 打印带载荷 / JSON 序列化。
- 字段默认值（`Circle(radius: float = 1.0)`）—— 连 struct 都还没有（M365 登记），不在此处先开。
- 载荷类型运行期强校验。
