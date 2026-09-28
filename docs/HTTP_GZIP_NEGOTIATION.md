# HTTP gzip 内容协商口径（M224）

> **适用范围**：`px_serve` 的**自动压缩** —— 三处分支（`vhost(host, handler)` / `.px` 脚本响应 /
> 原生静态文件）共用同一套判定 `px_resp_gzipable()`，故本文的每一条都同时适用于这三处。
>
> **不适用**（三层互不影响，别混）：
> · native `gzip_compress()` / `gzip_uncompress()` —— 用户**显式**压缩，永远可用，不受本文任何开关影响；
> · handler 返回值里的 `{"gzip": true}`（M21）—— 语义是「**handler 自己已经压过了**」，
>   runtime 见到它会跳过自动压缩（防双重压缩 ⇒ 客户端只解一层 ⇒ 乱码），与本文的 opts 开关不是同一层；
> · `http_serve(port, handler)` —— 它**不做**自动压缩，压不压完全由 handler 返回值决定。

---

## 1 判据（RFC 9110 §12.5.3）

请求的 `Accept-Encoding` 按 `#( codings [ weight ] )` 解析，逐逗号成员形如 `token[;q=value]`：

| 情形 | 是否自动压缩 |
|---|---|
| 无 `Accept-Encoding` 头 | **否**（客户端没表态 ⇒ 默认不压，保守） |
| `gzip` / `gzip;q=1` / `gzip;q=0.5` | **是** |
| **`gzip;q=0`**（含 `q=0.` / `q=0.0` / `q=0.000`） | **否** —— q=0 = **显式拒绝**该编码 |
| `br, gzip;q=0` | **否**（同上；其余成员无关） |
| `xgzip` / `not-gzip` / `gzipx` | **否**（token **精确**匹配，不做子串） |
| **`*`** / `*;q=1` | **是** —— `*` 匹配任何**未显式列出**的编码 |
| `*;q=0` | **否** |
| `gzip;q=0, *` | **否** —— **显式项优先于通配**（RFC 明文） |
| `gzip;q=1, *;q=0` | **是**（同上） |
| `GZIP` / `Gzip`（任意大小写） | **是** —— HTTP token 大小写不敏感（§11.1） |
| `GZIP;Q=0` | **否**（参数名 `q` 同样大小写不敏感） |
| `identity;q=0` | 否（但**不**回 406，见 §4） |
| 前导/尾随空白（`"  gzip  "`） | **是**（token 两侧空白忽略） |

除 AE 之外，还要同时满足：

- **体长** `body_len >= opts.gzip_min_bytes`（默认 **1024** 字节）；
- **Content-Type** 是 `text/*`，或含 `json` / `javascript` / `xml` / `svg` / `csv`
  （**大小写不敏感** —— `application/JSON` 也压）；
- 响应体存在（`204` / `304` / 空体不压）;
- 响应头里**没有** `Content-Encoding`（handler 自压过的不重复压）。

命中时 runtime 追加 `Content-Encoding: gzip` + **`Vary: Accept-Encoding`**，并替换响应体。

---

## 2 修前 / 修后（M224 · 缺陷 326）

修前的判据是裸 `strstr(ae, "gzip")`（M53 起沿用），四个面都不合规：

| # | 面 | 修前 | 修后 |
|---|---|---|---|
| ① | q 值 | `gzip;q=0` **仍压** —— 客户端**明确拒绝**还压，违反 RFC | 不压 |
| ② | 子串 | `xgzip` / `not-gzip` **算接受** | 不算接受 |
| ③ | 通配 | `*` **不认**（该压没压，白耗带宽） | 认（除非 `*;q=0`） |
| ④ | 大小写 | `GZIP` **不认** | 认 |

另修：Content-Type 的 `json`/`xml`/`svg`/`csv` 匹配由 `strstr` 改 `strcasestr`（同一「协商口径」面）。

> ① 是**错值方向**（压了客户端拒绝的），②③④ 是**保守方向**（该压没压）。
> 两侧都修，但 ① 才是本轮的主病灶 —— 它让「客户端说不要」被无声忽略。

---

## 3 `px_serve` 的压缩相关 opts

| 键 | 类型 | 默认 | 作用 |
|---|---|---|---|
| `gzip` | bool | `true` | **服务级总闸**。`false` ⇒ runtime 层**完全不压缩**，压缩策略交给应用层 |
| `gzip_min_bytes` | int ≥1 | `1024` | 触发压缩的体长下限 |
| `gzip_level` | int 1..9 | `6` | deflate 级别 |

```px
// 关掉 runtime 这层，压缩完全交给应用层（如 Mahesvara 自己那一层）
px_serve(18080, "/srv/webroot", 30000, {"gzip": false})

// 只想收紧阈值
px_serve(18080, "/srv/webroot", 30000, {"gzip_min_bytes": 4096})
```

**`opts.gzip` 的意义**：修前 runtime 这层压缩**无法关闭** —— 站点级开关（如 Mahesvara 的
`"gzip": false`）管不到它，调用方只能用 `{"gzip_min_bytes": 1073741824}`（1 GiB）把阈值抬到
永不触发，属于**语义借用**（可读性差，且行为随工具链版本变化）。
现在写 `{"gzip": false}` 即可，意图明确。

---

## 4 覆盖边界（**有意不做**，如实登记）

1. **`identity;q=0` / `*;q=0` 不回 406。** 严格按 RFC，客户端声明「什么编码都不要」（含 identity）
   时服务端应回 **406 Not Acceptable**。本实现只表现为「不压」、照常回明文 200 —— 这是
   **nginx 同款取舍**（生产上极少有客户端这么写，回 406 反而更容易把正常页面打成错误页）。
   ⇒ 若将来要合规到这一层，属于**行为变更**，需单独一轮 + 单独门。
2. **`Vary: Accept-Encoding` 只在实际压缩时发。** 若某响应「够格被压」但因客户端不支持而**没压**，
   本实现**不发** `Vary` ⇒ 与 nginx 行为一致。对**共享缓存**（CDN）而言，理论上存在
   「未压缩版被缓存、再喂给支持 gzip 的客户端」的可能。Mahesvara 侧的压缩点落在**缓存写入之后**，
   已在自己的层处理该问题；若要在 runtime 层补 `Vary`，需同时改三处调用点的头拼装（属独立变更）。
3. **没有站点级开关。** `vhost(host, handler)` 注册的站点**不能**各自开关 runtime 层的压缩 ——
   压缩配置是**进程级**的（见 §6）。要做「A 站压、B 站不压」，请用 `opts{"gzip": false}` 关掉
   runtime 层，在 handler 内自己按站点决定（可配 native `gzip_compress()`）。

---

## 5 ⚠️ 一个进程一套配置

`px_serve` 的压缩配置（以及 `max_conn` / `rate_limit` / `access_log` / `alt_svc` 等）都是
**进程级全局变量**，`px_serve()` **每次调用时先复位再应用 opts**。

⇒ **同一个进程里起两个 `px_serve`（不同端口 + 不同 opts）会互相覆盖** —— 后一个调用会改掉前一个的配置。
要跑两套配置，请**分进程**（不同的 `.px` 程序 / 不同的容器），不要靠 `spawn` 两个 `px_serve`。

（`examples/m224_gzip_negotiate/` 的两个用例正是因此拆成**两个独立程序 + 两个端口**。）

---

## 6 实现位置（供审计）

| 件 | 位置 |
|---|---|
| `px_q_is_zero` | `runtime/runtime.c`（q 值「是否等于 0」） |
| `px_ae_accepts_gzip` | 同上（`Accept-Encoding` 解析） |
| `px_resp_gzipable` | 同上（服务级开关 + 体长 + AE + Content-Type） |
| `g_px_gzip_enabled` / `g_px_gzip_min` / `g_px_gzip_level` | 同上（进程级全局） |
| opts 解析 | `px_serve` 的 `args[3]` 分支 |
| 三处调用 | `px_vhost_respond`（vhost）/ `.px` 脚本响应分支 / 原生静态分支 |
| 门 | `examples/m224_gzip_negotiate/`（21 例协商 + 8 例开关 · 双轨 + 负控 2 道） |
