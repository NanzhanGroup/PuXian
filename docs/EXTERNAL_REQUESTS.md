# 外部需求 / 问题收录台账

> 定位：**第三方（团队节点）提出的能力缺口与缺陷**的登记处。本仓的权威归档仍是
> `CHANGELOG.md`（每个里程碑一节）；这里只承担「收到 → 核实 → 编号 → 排期」这一段，
> 免得问题只活在某条总线消息或某个人的记忆里。
>
> 编号规则遵循 `docs/DEFECT_NUMBERING.md`：全局单调递增、不复用。
> **这里分配的编号在对应里程碑的 CHANGELOG 标题行声明之后才算入册**；
> 在此之前状态记为「拟分配」，避免出现「编号存在但没有里程碑认领」的悬空号。
>
> 证据等级：**E1** = 我方实查代码或实跑复现；**E2** = 仅报告方实测数据，我方未复核。

---

## 2026-11-26 · 晨曦（wsa-chenxi）· 为 wst-email / wst-img / wst-browser 可 PuXian 化

- 总线消息：`m18eb9ef9393978c126e8f67b`（kind=task，正文约 14.3 KB 摘要）
- 报告基线：px **0.2.444** / commit `7ffd4cfa` / synced 2026-11-25T13:04:46Z
- 我方复核基线：px **0.2.446** / commit `bbcb0eb`（本机 /data/code/puxian）
- 附带的完整需求单 14.3 KB + 8 个复现脚本**未送达**：中枢 ws-files 返回 507
  （磁盘余量 <10%；我方 2026-11-26 22:4x 实测根盘 93G/100G = **93%**，见文末运维项）

**编号分配（拟）：951–959，建议归 M447**

| 号 | 级别 | 一句话 | 证据 |
|---|---|---|---|
| 951 | P0 | 图像几何操作无内建（crop/flip/rotate/blend/round），纯 px 做像素级操作 O(n²) | E1 |
| 952 | P0 | 无字体渲染（无 stb_truetype；gfx 只有 5×7 位图 36 字符） | E1 |
| 953 | P1 | PSD：`STBI_NO_PSD` 屏蔽中；且 stb 后端只支持 v1/RGB/8-16bit，远窄于 Go 版 | E1 |
| 954 | P1 | smtp.px 注释称「STARTTLS 暂不支持」——能力早已在（M151-S1），注释过时 | E1 |
| 955 | P1 | imap 库无 TLS（只有 `tcp_connect_ex`），wst-email 默认 993 连不上 | E1 |
| 956 | P1 | mailparse 缺 MIME multipart/boundary（RFC 2047 已解） | E1 |
| 957 | P1 | png.px 只有 stored 无压缩；「zlib FFI 未注册」的注释已不成立 | E1 |
| 958 | P2 | `ws_recv` 不暴露 opcode/fin（CDP 类客户端需靠猜） | E2 |
| 959 | P2 | 文档/清单小缺陷集（bytes_slice 语义、bytes_set 语义、两处过时注释、pxe-manifest 换行、`px run --fast` import 未复验） | E1/E2 |

---

### 951 · 图像几何操作无内建（P0）

**报告断言**：runtime 只有 `img_decode` / `img_scale` / `img_encode_jpeg`；512×512 RGBA 上
逐行 slice+concat 垂直翻转 407ms/1MB、单行水平翻转 345ms/512px、逐像素 `bytes_get+bytes_set`
12426ms/16384px（解释轨）/1547ms（build 轨）；外推 4K 逐像素约 105 分钟（解释）/13 分钟（build）。

**我方核实（E1）**
- `runtime/runtime.c:14631-14633` 仅注册三个 `img_*`；全量 `img_*` 符号扫描无
  `img_crop / img_flip / img_rotate / img_blend / img_round` ⇒ **缺口成立**。
- 根因不是内建慢，是 **bytes 不可变**：`bi_bytes_set`（`runtime.c:16352`）
  每次 `xmalloc(len+1) + memcpy + 返回新对象` ⇒ 单次写 O(len)，循环写 O(n²)。**根因成立**。
- `bi_bytes_slice`（`runtime.c:16375`）确为 `[start, end)` 语义（`a`/`b` 双端钳制），
  文档与示例未写明 ⇒ 见 959。
- 耗时刻度为报告方实测，我方未逐条重跑（只核实了代码路径与复杂度）。

**处置建议**
1. 优先做**五个内建**（`img_crop / img_flip / img_rotate / img_blend / img_round`）：
   一次 C 层 memcpy，直接把 O(n²) 变 O(n)，且不影响 bytes 的不可变契约。
2. `bytes_mut_new / bytes_mut_put / bytes_mut_freeze`（可变通道）**单独立项**，不与本条同轮：
   它触及「bytes 不可变」这条语言级契约，影响面远大于图像，需要独立判据与一轮观察。
3. 判据：小图逐像素与非内建结果逐字节一致 + 大图（2048²）耗时有量级差 + 负控（尺寸不匹配必 Err）。

### 952 · 无字体渲染（P0）

**我方核实（E1）**
- `runtime/third_party/stb/` 只有 `stb_image.h`（v2.30）+ `stb_image_write.h`（v1.16）；
  在 runtime / stdlib / tools / third_party / bootstrap / selfhost 全源码面
  `grep stb_truetype` **0 命中** ⇒ 缺口成立。
- `stdlib/gfx.px:7` 自述「内置 5x7 位图（0-9 A-Z . - 空格 ?；小写转大写，未知转 ?）」⇒ **成立**，
  即中文、任意字号、任意字体全都做不到。

**处置建议**
- `stb_truetype`（+约 100 KB）+ `font_load / font_metrics / font_text_size / font_render(utf8, px, color)`；
  文档写明**限 .ttf/.ttc（glyf 轮廓）**，不含 OTF/CFF。字体文件不入 runtime，由调用方传路径或 bytes。
- 若要 CFF/OTF 支持须引 FreeType（体积与「静态单文件」口径冲突）⇒ 明确**不做**，写进能力边界。
- 判据：字宽/字高与 CPython PIL 同字体同字号对拍（允许 ±1px 抗锯齿差）+ 中文渲染非空 + 缺字回退可见。

### 953 · PSD 能力远窄于 Go 版（P1）

**我方核实（E1）**
- `runtime/runtime_image.c:28` `#define STBI_NO_PSD`（同处 `:27` 还有 `STBI_NO_TGA`）⇒ 当前完全不解 PSD。
- `runtime/third_party/stb/stb_image.h:6140-6170` 硬限制：`version != 1` 拒绝（PSB=2）、
  `color mode != 3` 拒绝（仅 RGB）、`bitdepth` 仅 8/16 ⇒ **删宏只得到 RGB/8-16bit PSD 子集**。
- wst-img Go 版支持 PSD+PSB / 灰度·索引·RGB·CMYK / 8·16·32bit float / 未压缩·RLE。

**处置建议**
- 第一步只删宏（一行）+ 文档写明「PSD 仅 RGB 8-16bit」；CMYK/PSB/32bit/ZIP 不做，按「能力降级」写进工具文档。
- 不要用「PSD 已支持」这种全域表述 —— 那是 953 的假绿形态（同缺陷 872 的「假文档」家族）。

### 954 · smtp STARTTLS 注释过时（P1）

**我方核实（E1）**
- `stdlib/smtp.px:11` 原文「默认明文（25/内网 587）；STARTTLS 暂不支持（语言无客户端 TLS 包装 native，见 M83_PLAN §S5 边界）」。
- `runtime/runtime.c:14316 / 14321` 已注册 `tls_connect` / `tls_upgrade`（M151-S1），
  `tls_upgrade` 走 `tls_upgrade(fd, opts)` 三段式装配 ⇒ **注释的前提已不成立**。

**处置建议**：`opts.tls = "starttls" | "implicit" | false`（默认 false 保兼容）+ `opts.tls_verify`；
同步更正 `stdlib/smtp.px` 头注释与 `M83_PLAN §S5` 的边界描述。
判据：587 STARTTLS 实连一回 + 465 隐式实连一回 + 默认 false 行为不变（防回归）。

### 955 · imap 无 TLS（P1）

**我方核实（E1）**
- `registry/imap/0.2.0/imap.px:49` 只有 `tcp_connect_ex(host, port, ...)`；
  该文件内 `tls` / `993` / `STARTTLS` **0 命中** ⇒ 成立。

**处置建议**：`imap_connect(host, port, opts?)` 增 `opts.tls`（或 `imap_connect_tls`）；
连接对象带「明文/加密」标记，`o_cmd` / `o_read_line` 按标记分流到 `tcp_*` / `tls_*`；
143 的 STARTTLS 可选（先读 CAPABILITY 广告）。
判据：993 隐式登录成功 + 143 明文不回归 + 证书校验失败必须响亮。

### 956 · mailparse 缺 MIME multipart（P1）

**我方核实（E1）**
- `registry/mailparse/0.2.0/mailparse.px:150` 有 `mail_decode_rfc2047`（B/Q 与相邻编码字拼接）⇒ 报告方说的「好消息」成立。
- 同文件 `multipart` / `boundary` **0 命中**（全文 232 行）⇒ 缺主体解析成立。

**处置建议**：`mail_split_multipart(raw) -> [{headers, body, content_type, filename, is_attachment}]`
+ `mail_decode_body(part)`（base64/QP + charset，至少 utf-8/gbk/latin1）；
判据：一份真实多段邮件（含附件 + 一段 gbk）逐段对拍 + 畸形 boundary 必须 Err 不静默。

### 957 · png 无压缩；注释过时（P1）

**我方核实（E1）**
- `stdlib/png.px:2` 「stored 无压缩」、`:8` 「零新 C、零 FFI（A 线 zlib FFI 通后可选联动）」⇒ 现状成立。
- **但「解释轨 pxi FFI 未注册 zlib」已不成立**：本机实跑
  `./tools/px run examples/m61_zlib/m61_zlib.px` ⇒ A–G 全 PASS
  （compress→uncompress roundtrip：10400B → 110B），含解释轨。
  同一句过时描述也在 `registry/zlib/0.2.0/zlib.px:9`。

**处置建议**：`png_encode(..., opts.level)`，默认 **6**（当前产物比真压缩大 3–10 倍）；
`level=0` 保持现有 stored 路径（保兼容 + 留对拍基准）；同步更正两处注释。
判据：解码回像素逐字节一致 + 体积比 stored 小 3 倍以上 + level=0 产物与旧版逐字节相同。

**957 处置实况（2026-11-27，M448 · 改库不改 runtime）**
- 实现：`png_encode(w, h, pixels, opts = {})`。`opts.level` 缺省 **0 = stored**（不传 opts 时**逐字节不变**）；
  `{"level": 1..9}` ⇒ IDAT 走 `extern def zlib_compress`（runtime_zlib.c 的 compress2，两轨均已通）；
  越界值（99 / -1 / 字符串 "6"）与非 dict 的 opts 一律回退 stored；FFI 返回 null 也回退 ⇒ 永不产出坏文件。
  **默认值取 0 而不是建议里的 6**，理由（实测）：`examples/m61_gfx/verify_s4.sh` 按 sha256 对拍
  x86 ⇄ aarch64、仓库有金标 `mandelbrot.png`、emitc 冻结按闭包指纹 —— 默认改 6 会让这些判据一起漂移。
  ⇒ 「改默认」是独立议题（需同步改三类判据），本轮不开。
- E1 判据（新门 `m448_png957` = `selfhost/check_m448_png957.sh`，6 判据含负控，已注册 registry + ci.yml，基线 4s）：
  · C1 默认三参产物 sha256 = `7bee77b8…`（与 python **独立重构**的 M61-S2 口径 stored 字节相同；
    另实测：新实现 level 0 与改前实现同一图逐字节相同、`demo_mandelbrot` 产物仍等于仓库金标 `mandelbrot.png`）
  · C2 `level=6` 产物 python 独立解码（逐 chunk CRC + `zlib.decompress`）通过，像素与 stored 逐字节一致
  · C3 体积比 stored/compressed = **8.5x**（IDAT 18507 → 2181 字节；96×64 混合图：标题条 + 8 行渐变 + 棋盘精灵）
  · C4 显式 `level=0` / 99 / -1 / "6" 全部回退为 stored 且逐字节相同
  · N1 负控：把 level 分支阉掉 ⇒ 体积比回落 **1.00x**，C3 真能红
- 同步：`registry/png/0.1.0/png.px` 镜像（parity 门 13/13 绿）· emitc 冻结重定基 4 条（3 条 png 闭包 +
  1 条上一轮 smtp 注释改动遗留的 `m83_s5/smtp_test`）· 门耗时基线用 `refresh_gate_baseline.sh
  --extra-tsv` 官方回灌（**未手改 tsv**），`check_gate_timeouts.sh` 47/47 绿。
- 已知非本轮问题：`examples/m61_gfx/verify_s2.sh` 的 demo_scene 步仍是历史红
  （`set_px` 收到 float，2026-10-05 已入 `selfhost/gcstress_ledger.tsv`），与 957 无关。

### 958 · ws_recv 不带 opcode/fin（P2）

报告方建议 `ws_recv_ex -> {data, opcode, fin}`；同时**明确建议 wst-browser 保持 Go 版不迁**
（价值最低、成本最高）。我方未复核（E2）。
处置：收录不排期；若做，判据为「一次 ping/pong 与一次分片消息各自可见 opcode/fin」。

### 959 · 文档 / 清单小缺陷集（P2）

| 子项 | 核实 |
|---|---|
| `bytes_slice(b, start, end)` 是 `[start,end)`，不是 start+长度 | E1（`runtime.c:16375` 实现） |
| `bytes_set` 返回**新对象**、b 本身不变 | E1（`runtime.c:16352` malloc+memcpy） |
| `stdlib/smtp.px:11` STARTTLS 过时 | E1（见 954） |
| `registry/zlib/0.2.0/zlib.px:10` + `stdlib/png.px:8` 解释轨 FFI 过时 | E1（见 957 实测） |
| `pxe-manifest.json` 的 `px_version` 带尾部换行（`trim()` 不剥 `\n`） | **E2 未复现**（2026-11-27）：全盘 `find / -name pxe-manifest.json` 0 命中，仓库内也无 `px_version` 生成点 ⇒ 无产物可验。待报告方给生成命令 |
| `px run --fast` 是否仍不解析 import（0.2.373 的 R1001） | **E1 已复现，形态比记录更细**（2026-11-27）：字符串形 `import "lib1.px"` ⇒ `px build` / `--fast` 均 rc=0 正确（M378 的修有效）；**点号形本地模块** `import lib1` / `import util.helper` ⇒ 解释轨 OK，`px build` rc=1 `E L002: 未定义变量: 'lib1'`，`--fast` 同红且**不回落解释轨**（rc=1）。同时 `px deps main.px` 能正确解出 `lib1.px`（deps 2）⇒ **依赖闭包解析与编译期模块解析口径不一致** |

处置：随 M447 一并改文档（零风险）；`pxe-manifest` 与 `--fast` 两条先补复现再定性，
**不要凭记录直接改**。

**959 处置实况（2026-11-27，纯文档/注释，零运行时代码）**
- 已改：`docs/PUXIAN_CHEATSHEET.md` §bytes（`bytes_slice` 半开区间 `[start,end)` 第三参是结束下标；
  `bytes_set` COW 返回新对象、逐字节改写 O(n²)）· `stdlib/smtp.px` 头注释（STARTTLS 能力已由 M151-S1
  的 `tls_connect`/`tls_upgrade` 就位，本库未接入，指 954）· `stdlib/png.px:8`（zlib FFI 早已通，
  解释/编译两轨均可 `import "c/zlib"`）· `registry/zlib/0.2.0/zlib.px:9`（PX-DEF-035 已修，两轨可用）。
- 复现（E1，本机）：`./tools/px run examples/m61_zlib/m61_zlib.px` ⇒ A–G 全 PASS（解释轨含内）；
  `grep -n tls_connect runtime/runtime.c` ⇒ 14316 注册、18822 有句柄表。
- `--fast` 点号形本地 import 的硬失败**未修**（属编译轨模块解析，非文档）：留作 M448 候选缺陷。

---

## 与 M447 的关系

- 951–959 为**拟分配编号**，M447 的 CHANGELOG 标题行写「（登记 951–959）」后入册。
- 建议实做次序（按「阻塞第三方 / 改动半径」排）：
  `959 文档（半天）→ 957 png 压缩（改库不改 runtime）→ 954 smtp 注释+opts → 953 PSD 删宏
   → 956 mailparse multipart → 955 imap TLS → 952 字体 → 951 图像内建`
- 951 / 952 是**唯二动 runtime** 的两条，按纪律各自单独一轮，不同轮混改。
- 本仓当前 CHANGELOG 最大号为 **M446**；**947 / 948 / 949 / 950 四条编号只存在于提交信息与
  `/data/gatevm/oneoff-m447/NOTES-949-950.md`，CHANGELOG 无对应条目** —— 与 951–959 一并在
  M447 回填，否则这四条在权威归档里是空坐标。

## 附：报告方提到的环境侧问题（非本仓）

- **ws-files 507**：根盘 2026-11-26 22:4x 实测 `93G/100G = 93%`（可用 7.9G）。
  占用大头：`/data/gatevm` 71–75G（wt1–wt8 各 5.3–6.8G + m442s3-exp 5.9G + tmp2–tmp8 各约 2G，
  均为第 4/5 轮全量门的分片工作树，当前**无门在跑**）、`.buildcache` 5.9G、`.rtcache` 4.3G。
  这批是运行时产物（`shard_run.sh` 会自建），清理属破坏性操作，**须用户点头后执行**。

---

*收录：东月 · 2026-11-26 22:5x · 基线 0.2.446 / bbcb0eb*
