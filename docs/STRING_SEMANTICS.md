# 字符串 / 字节 / 参数语义速查（M390 建立）

> 由来：清歌（PX-DEFECTS-20260909 一族）与晨曦（CX-01…CX-11）反复踩同一批坑，
> 而语言层此前**没有一份把它们写清**的文档。本文件是那些「文档类」条目的落点。
> 逐条对应：PX-005 / PX-006 / PX-012 / PX-016 / PX-026 / PX-033 / CX-05 / CX-06 / CX-07。

## 1 遇 NUL 不截断（M390 起，全部按字节长）

```
PX_STR 对象带 (data, len)：len() 与 s[i] 一直按 len 走；M390 起内建也与它对齐。

不截断（字节精确）：
  len  bytes_len  split  replace  contains  starts_with  ends_with  join
  trim  to_upper  to_lower  read_file  write_file  write_bytes  bytes_find
  regex_*（模式与文本都按长度）
截断（C 串语义，**只有键**）：
  dict 的键 —— 键是 char*，内嵌 NUL 无法表示（已登记，非缺陷）
```

写法建议：处理二进制一律 `bytes_*`；处理文本用字符串即可，不必再为「怕 NUL」全线退到 `bytes_*`
（旧代码注释里「字符串内建遇 NUL 一律截断」在 M390 已不成立，照它写会白付 8.5× 的 bytes_find 代价）。

## 2 正则：量词、惰性、Unicode 类

```
贪婪 / 惰性：`*` `+` `?` `{n,m}` 贪婪；后缀 `?` ⇒ 惰性（`*?` `+?` `??` `{n,m}?`）—— M390 起生效
  regex_find("a.*b","axxbxxb")  → "axxbxxb"（最长）
  regex_find("a.*?b","axxbxxb") → "axxb"   （最短）
不支持：`\p{L}` 这类 Unicode 类别（本引擎按**字节**匹配，无 Unicode 表）⇒ **显式报错**，不静默
字符类：`[[:space:]]` 等 POSIX 类可用；`\s` == [\t\n\f\r ]（不含 \v，与 Go 一致）
```

## 3 保留字（PX-026）

`pub` `fn` `def` `from` 是保留字，用作变量名直接 `E2001`。完整表以 `selfhost/pxlexer.px`
的关键字集合为准（`tools/lint_core.px` 的 BUILTIN_NAMES 是**内置函数**名，不是保留字）。

## 4 args() 与命令行（PX-012）

```
args()  **含** argv[0]（程序自身路径）—— 与 Go os.Args 同形，但**不是** Python sys.argv[1:]
⇒ 取用户实参一律 `args()[1:]`
`px run file.px a b` 与编译产物的 args() 形状**一致**（都是 [程序, a, b]）
改这条语义会破坏仓内 10+ 个工具（它们已经按 [1:] 写）⇒ 属破坏性变更，留待大版本
```

## 5 解释轨（px run）的能力边界（CX-07）

```
spawn 可用；channel / actor 在解释轨直接报「interp 不支持通道」
`px run` 比编译态慢 50–240×（X25519 标量乘实测 30s vs 0.3s）—— 用它估时会得出「不可行」的错误结论
结论：调功能用 px run，判性能/上生产用 px build
```

## 6 大常量表怎么建（CX-06）

```
不要：dict 逐条 set 建 2000+ 条表（dict 是线性数组 ⇒ O(n²)；8k 键插入 168ms、查找 292ms）
不要：循环里用 "+" 拼 2000+ 次字符串（O(n²)）
建议：编译期两个等长**有序 list 字面量** + 二分查找
      实测（晨曦）：2229 条实体表 建表 1ms / 查找 ~5µs（原 755ms 建表）
```

## 7 时间（PX-033，M390 起）

```
time_format(ts, fmt)          → **本地时区**
time_format(ts, fmt, "UTC")   → UTC
time_format(ts, fmt, null)    → UTC（显式逃生口）
now_sec / now_ms / now_ns     → 墙钟（CLOCK_REALTIME）
now_us                        → **单调钟**（CLOCK_MONOTONIC，开机以来）⇒ 不要当时间戳用
```

## 8 数值取整（PX-008，M390 起）

```
int(x)       向零截断（与 Go/C 一致，**未改**）
trunc(x)     同 int()，但名字自解释
int_floor(x) 向下取整（-7.9 → -8）
int_ceil(x)  向上取整（-0.1 → 0）
```
