# M61 规划：外部库 FFI proof（zlib）+ 纯语言 2D 游戏内圈（gfx）

> 状态：待审（D1–D5 拍板后开工）
> 背景：无树莓派真板 → GAP/ROADMAP 候选池排除「真板物理回归」，剩余两项均为游戏线铺路。用户选定 **A+B 并行一期**：
> A = FFI 外部系统库绑定验证（游戏窗口线 0→1 的机制地基）；B = 纯语言 2D 游戏内圈（零 FFI/零硬件，产出可见图片与可玩 demo，最大程度 dogfood M59+M60）。

---

## §1 事实基准（本轮复核，非记忆推断）

### A 线（FFI）
| 项 | 实测结论 | 证据 |
|---|---|---|
| M42 FFI 机制 | ✅ 已通：`import "c/xxx"` + `extern def` + FFI 注册表（runtime_ffi.c，M42）/ quic（runtime_quic.c，M46），语言 extern def 按名字查 **C 侧手写注册表** | runtime.c:5307-5320；spec §M42/§M46 |
| 已绑库 | **全为内部 third_party**（ngtcp2/openssl-quictls/mbedtls），pxc 链接行硬编码拼 .a 路径 | tools/pxc:99-173 |
| 外部系统库先例 | **零**：无 pkg-config 接入、无 `-lz` 通用机制、无「第三方 .a 入库/交叉」约定 | — |
| pxc 形态 | **bash 脚本**（非二进制），默认 `cc="gcc"`（x86 静态）；`--cc` 可换交叉链 | tools/pxc:106-112 |
| 交叉链 | aarch64-linux-musl 全套含 ar/ranlib/strip（M57/M58 复用） | tools/cross_aarch64.sh |
| zlib x86 | pkg-config YES + /usr/include/zlib.h 在；**静态 libz.a 未确认/需自备**；aarch64 musl 无现成 → 源码自编 | 本轮环境 |
| bytes 层 | 全套就绪（read_bytes/write_bytes/int_to_bytes/bytes_to_int/bytes_len/get/slice/concat），语言↔C 指针缓冲语义可承载 | runtime.c:5349-5361、5731+ |

> **机制关键认知**：extern def 并非自动 dlsym 任意 C 符号，而是查 **C 侧胶水注册表**。
> 故「绑定外部库」= ① 写薄 C 胶水注册（指针/长度签名函数）→ ② 外部静态 .a 进 pxc 链接行 → ③ 入库约定。
> 这三点正是 GAP「FFI 只绑过内部库」的真缺口，A 线逐一打通。

### B 线（2D 内圈）
| 地基 | 状态 |
|---|---|
| 数学全套（sin/cos/tan/atan2/floor/ceil/round/log/log10/exp/random*/pi/e） | ✅ M59 |
| us 级时钟 + fd 多路 + raw 终端（sleep_us/now_us/fd_wait/tty_config/fcntl） | ✅ M60 |
| fd 原语（open/read/write/ioctl/mmap/mem_write） | ✅ M57 |
| bytes 全套（含二进制文件 write_bytes、就地解析） | ✅ 见上 |
| stdlib 生态 | ✅ collections/semver/webroute/edge（4 个），gfx 为第 5 个 |

---

## §2 方案设计

### A 线：外部系统库绑定 proof（zlib）
- **目标**：以 zlib（真实世界最普适 C 库之一）打通「外部 .a 入库 → C 薄胶水注册 → pxc 链接 → 语言调用 → 跨架构」全链路，回答「FFI 外部库机制通不通」，并为后续任意外部库（sqlite3/raylib 等）立模板。
- **绑定三函数**（签名逐步加难度，正好验证指针/长度传参语义）：
  - `crc32(crc, buf)`：纯入参指针+长度，无歧义（已知值可校验）
  - `compress2(dst, src, level)` / `uncompress(dst, src)`：含 **uLongf\* 长度指针 in/out** —— 经 bytes 就地缓冲语义承载，是 FFI 指针传参的典型压力位
- **示例 m61_zlib.px**：crc32("hello") 已知值校验（0x3610a686）；一段文本 compress → uncompress 还原逐字节一致 + 压缩率展示；**纯语言 CRC32 查表实现互证**（dogfood）。
- **入库布局（草案，S0 定稿）**：`third_party/zlib/{src, lib-x86_64, lib-aarch64}/`，zlib 1.3.x 源码自编两版静态 .a：
  - x86：`CC=gcc AR=ar RANLIB=ranlib ./configure --static && make`（glibc）
  - aarch64：`CC=aarch64-linux-musl-gcc AR=aarch64-linux-musl-ar RANLIB=aarch64-linux-musl-ranlib ./configure --static && make`（纯 musl 静态，与工具链同哲学）
  - 源码/产物入库或仅产物+构建脚本入库（S0 定，倾向产物+脚本，免每次联网）
- **验证**：x86 `file` 确认静态链接 → qemu-aarch64 跑同示例逐字节一致（复用 M57/M58 流程）。

### B 线：stdlib gfx.px（纯语言 2D 游戏内圈）
- **画布**：`list[int]` 每像素打包 `0xRRGGBB` + 宽高（正确优先；性能 dogfood 后若需再评估 bytes 三字节/像素，D5）。
- **原语**（全部 Bresenham/中点圆手写纯语言）：`px/line/rect/fill_rect/circle/fill_circle/blit`（精灵=小 list 或 read_bytes 载 PPM）+ `text(x,y,str,color)`（内置 5x7 ASCII 位图字形 dict）。
- **PNG 编码器 png.px（第 6 个 stdlib 候选，D3）**：纯语言写 PNG（8bit RGB、逐行 filter=0、CRC32 查表、**zlib stored block 无压缩**）→ `write_bytes` 落盘 → **send_file 给用户眼见图片**（B 线核心可见性）。A 线 zlib 通后可选联动 upgrade 为 FFI compress2 真压缩（见 §4）。
- **demo1（必达）**：Mandelbrot 640x480 静帧 PNG —— 确定性、无输入依赖、dogfood M59 float 全套（sin 无关但复数迭代=浮点压力）、视觉直接。
- **demo2（必达）**：合成场景静帧（线/圆/矩形/填充/多精灵/文字「PuXian 2D」）—— 全原语覆盖验证。
- **输入线（dogfood M60）**：`tty_config(0)` raw + `fd_wait([0])` 单键读取 → 终端**可玩** demo（贪吃蛇/反应测速；真终端可跑，本会话 TERM=dumb 仅验证可运行逻辑）。uinput/evdev 键盘解析为 stretch（D4）。

---

## §3 子步计划（每步独立 commit + 独立验证）

| 子步 | 内容 | 验证 |
|---|---|---|
| **S0** | A 地基：third_party/zlib 自编两版静态 .a 入库 + 构建脚本；pxc 链接分支初探 | 两版 `file`/ar t 确认 |
| **S1** | A 主体：runtime_zlib.c 薄胶水（crc32/compress2/uncompress 注册）+ pxc 链接 + m61_zlib.px | x86 静态跑通 + 纯语言 CRC32 互证 |
| **S2** | B 核心：stdlib gfx.px（画布+原语+字形）+ png.px stored 编码器 + demo1 Mandelbrot + demo2 合成场景 | PNG 落盘 → send_file 眼见 |
| **S3** | B 输入：raw 终端单键 demo（可玩）；stretch：uinput evdev 解析 | 真终端可玩 + 逻辑自检 |
| **S4** | A+B 收口：aarch64 交叉全量（zlib 示例 + gfx PNG 落盘一致）+ pxi 双模式（stdlib gfx 解释器跑，pxi 相对 import 限制如实记录）；（可选联动）PNG 走 FFI compress2 | 跨架构一致 |
| **S5** | 文档：spec §8.x（FFI 外部库约定 + gfx/png 库）、MINI_SUBSET 新发现、stdlib 表 +、ROADMAP/GAP 勾选、CHANGELOG、PLAN 回填 | 全量复跑 PASS |

---

## §4 明确不做（本轮，留档按需）
- **SDL2/raylib 真窗口**：无显示器/真板，只写结论性评估留档（依赖 apt glibc 动态 or 大体积静态交叉 + 仅 dummy 离屏冒烟，收益/成本比低）——A 线 stretch 押后（D1）。
- PNG 压缩优化/调色板/渐进（stored 起步；A 线 zlib 联动为可选加分）。
- 动画循环/碰撞物理框架/声音（demo 层自写够用）。
- /dev/fb0 实时动画（无头不可见，留给真板 m60_gpio 系列即插即用验收）。

---

## §5 待拍板（D1–D5）
- **D1**：A 线 stretch = SDL2/raylib「结论留档、本轮不实装」→ 认可？
- **D2**：zlib 自编两版静态 .a 入库 third_party（产物+构建脚本）→ 认可？（替代 apt 半套/动态链接，保持纯静态哲学）
- **D3**：PNG 编码器独立 stdlib png.px（stored 起步，A 线通后可选 FFI upgrade）→ 认可？
- **D4**：B 线输入：raw 终端可玩 demo（必达）+ uinput evdev（stretch）→ 认可？
- **D5**：画布用 list[int]（正确优先，性能 dogfood 后再定 bytes）→ 认可？

> 回复「批准开工」或逐条给 D1–D5 意见即开动 S0。
