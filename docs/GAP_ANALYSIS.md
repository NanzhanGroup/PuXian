# PuXian 能力差距分析（树莓派/边缘设备 · 2D/3D 游戏）

> 定位：面向两条用户线的能力差距清单与建议里程碑排期，作为 `ROADMAP.md`「远期方向」的
> 输入与单一事实源。
> 事实基准：runtime 内置函数全量清单（≈257 项 C 桥 defs 扫描）、stdlib（collections /
> semver / webroute 三文件 ~440 行）、docs/spec.md、docs/ROADMAP.md、docs/MINI_SUBSET.md
> §十三（M58 dogfood 暴露的语言欠账）、M57/M58 实测产物（examples/m57_*、examples/
> m58_hwmond/、tools/cross_aarch64.sh、bootstrap/）。
> 生成：M58 里程碑闭环 + webhook 真发闭环落地之后。

## 一、结论摘要

1. **树莓派/边缘设备：能力已开、缺「厚度 + 真板验证」**。M57 已落地底层原语（fd / mmap /
   交叉编译），缺口是 GPIO 真正控制、边沿中断、串口 termios、SPI/PWM 封装、us 级时钟、
   以及一块真板子的物理回归。
2. **2D 游戏：几乎未开**。缺图形 / 输入 / 数学基础三块；终端 ASCII 层半通、帧缓冲软件层
   「通道通但原语零」、SDL2/raylib 窗口层 0→1 未做。
3. **3D 游戏**：ROADMAP 明确不做 OpenGL/Vulkan 原生绑定；唯一现实路径 = raylib C 绑定，
   前提是 FFI 外部系统库绑定先验证。
4. **两条线的公共地基 = 数学与随机函数补齐**（sin/cos/atan2/random 等）→ 建议作为 M59
   主线，语言面最小侵入、回归可控。

## 二、树莓派/边缘设备：已具备（M57 + M58，均实测）

| 能力 | 落地证据 |
|---|---|
| fd 原语 | `open`/`close`/`ioctl`/`os_errno`；ioctl arg 三形态（int 直传 / bytes·str 就地 in/out buffer，`_IOR` 内核直接填充）——M57-S1（57bb9d7） |
| fd 数据通道 | `read`/`write`；pxhwmond 用它采真实 /proc（M58-S1，mem_total 与 MemTotal 精确对拍） |
| mmap 活映射 | `mmap`/`munmap`/`mem_write`，MAP_SHARED 帧缓冲/共享内存直访，GC 自动 munmap（is_mmap 位域）——M57-S2（f71b28e）；pxhwmond 快照 IPC 双向活映射——M58-S2 |
| 交叉编译 | aarch64 交叉 + qemu-aarch64 验证（`--no-quic` 裁剪），M57-S4 / M58-S4 双次实证（跨架构采真实 /proc 一致） |
| 设备示例 | GPIO 芯片信息查询、I2C slave 读写示例编译通过（x86 无设备自动 SKIP）——M57-S3 |
| 运行形态 | 单静态 ELF（CGO 无关）；run.sh 崩溃自愈 wrapper（kill -9 → 自动拉起）——M58-S4 |
| 基础并发 | `spawn`/`chan`/`sleep(ms)`/`set_timeout`/`signal`（M55 并发安全 hotfix 后） |

## 三、树莓派/边缘设备：真缺口（按「上真板立刻会撞」排序）

| # | 缺口 | 现状证据 | 影响 | 解决路径 | 状态 |
|---|---|---|---|---|---|
| 1 | GPIO 真正输出/读入 | 原仅只读 `GPIO_GET_CHIPINFO`；未做 line request / 读写 / 边沿事件 | LED/按键/继电器做不了 | ioctl 语法已够，缺封装与示例 | ✅ **M60 已落地**：`std.edge` GPIO V2 line（gpio_input/output/input_edge + read/write），592B 结构体 C offsetof 单测；真板段示例 m60_gpio |
| 2 | GPIO 边沿中断（poll/epoll） | 原 runtime 内部 poll 未暴露为语言函数 | 按键中断、脉冲计数、编码器全卡死 | 语言暴露 poll | ✅ **M60 已落地**：`fd_wait(fds, ms)`（POLLIN，revents 非 0 即事件含 HUP）；GPIO 边沿经 gpio_wait/gpio_event |
| 3 | 串口 UART：无 termios | 原全库无 termios/tcsetattr/B9600 | `/dev/ttyS0` 设不了波特率/raw；GPS/蓝牙/RS485 全废 | termios 封装 | ✅ **M60 已落地**：`tty_config(fd, baud, raw)` + `serial_open`；m60_serial_pty **x86 实跑 PTY 真内核串口 loopback** |
| 4 | SPI / PWM 便捷封装 | 原无 spidev/pwmchip 示例；PWM 只能手写 sysfs | ioctl 结构体含指针、手写痛苦，无 stdlib | edge.px 封装 | ✅ **M60 已落地（PWM 全）**：`pwm_setup/enable/set_duty` sysfs + m60_pwm 示例；⚠️ SPI_IOC_MESSAGE（transfer 数组含 u64 指针）仍留档（§3.4），spidev read/write 半通 |
| 5 | 高精度时钟（us 级） | 原 `sleep` 仅 ms 整数粒度 | 1-Wire、DHT11/DS18B20 时序 bit-bang 做不了 | us 级内置 | ✅ **M60 已落地**：`sleep_us(us)` + `now_us()`（CLOCK_MONOTONIC 微秒） |
| 6 | 真板物理验证缺位 | GPIO/I2C/PWM 示例「留板子环境验证」（x86 SKIP）；qemu 也验不了真实设备 | 所有边缘能力停在「理论上通」，无真实器件回归 | 真板物理回归 | ⏳ 候选（需树莓派硬件，m60_gpio/i2c/pwm 即插即用） |
| 7 | pxi 解释器设备 API（§十三 #8） | `pxc run` 对相对 import + open 链失败 | 板子上只能跑**编译模式**静态二进制（可行，但解释器快排用不了） | 语言面小修/评估（同 http_get 类 Err 修复节奏） |

**缺口共性**：缺的不是语法，而是 **3 个小内置（poll/epoll + termios）+ 1 个 edge stdlib
封装 + 1 块真板子**。

## 四、2D 游戏

### 已具备的游戏底座

float64（IEEE 754）、list/dict/闭包/struct、多文件 import、`spawn`/`chan`、`sleep(ms)`/
`now_ms`/`set_timeout`、mmap、单静态二进制、可交叉编译。**画像素的最低通道已通**
（/dev/fb0 mmap，M57 帧缓冲示例路径）。

### 分层可行性

| 目标层 | 可行性 | 缺什么 |
|---|---|---|
| 终端 ASCII 游戏（蛇/俄罗斯方块/Roguelike） | 🟡 半通 | `input` 是**行缓冲**读，无 raw-mode/单键即时响应（termios 缺）；无 `random`；HUD 浮点显示受 %g 6 位精度影响（§十三 #7） |
| 帧缓冲软件 2D（/dev/fb0 像素） | 🟡 通道通、原语零 | 画点/线/圆/矩形/BMP 精灵/字体渲染全要纯语言手写（stdlib `gfx.px` 候选）；键盘事件封装（/dev/input/event* 可读但解析无示例）；无双缓冲/vsync 封装 |
| SDL2 / raylib 窗口游戏 | 🔴 0→1 未做 | FFI（M42）至今只绑过**内部 C 库**（ngtcp2/mbedtls 等），无「外部系统库」绑定先例：.a/.so 链接流程、pkg-config、事件循环桥均未验证 |

### 数学欠账（做旋转/随机游戏立刻撞）—— ✅ M59 已全部补齐（2026-09）

| 类别 | 函数 |
|---|---|
| ✅ 已有 | `abs`/`min`/`max`/`sqrt`/`pow`/`sum`；float64 运算在位 |
| ✅ M59 已补（原必撞） | `sin`/`cos`/`tan`/`atan2`（弧度）；`random`/`random_int`/`random_seed`（splitmix64，确定性可复现） |
| ✅ M59 已补（原次要） | `floor`/`ceil`/`round`/`log`/`log10`/`exp` + `pi`/`e` 全精度常量 |

> 落地：C libm 内置 + splitmix64（M59 S1–S3）；pxi 解释器双模式同步 + aarch64 交叉
> qemu 验证（M59 S4）；examples/m59_math/ + ROADMAP/CHANGELOG/MINI_SUBSET §十三.4。
> 注：`min`/`max` 为双参内置（非 std.math 包形态）。stdlib 目前仅 collections / semver /
> webroute 三文件共 ~440 行——**生态库极薄**仍是两条线的共同底色。

## 五、3D 游戏：一句话

ROADMAP 明确不做 OpenGL/Vulkan 原生绑定（工程量与价值不成比例）。唯一现实路径 =
**raylib C 绑定**：语言只写游戏逻辑、渲染交给 raylib——前提是 FFI 外部库绑定先做通
（见 §七 候选 A）。纯语言软件光栅化理论可行但性能无望，不建议。

## 六、历史任务覆盖核对（#36 / #37 → M57/M58）

| 清单项 | 状态 | 覆盖证据 | 遗留 |
|---|---|---|---|
| #37 通用 Linux 设备绑定（ioctl/mmap extern） | ✅ **100% 闭环**（= M57 S1–S4） | 57bb9d7（open/close/ioctl/os_errno）、f71b28e（read/write/mmap/munmap/mem_write + GC 自动 munmap）、8f6e615（纯用户态先行——真内核替身 lo ifreq / PTY / i2c，比假驱动更强）、bbffcd5（qemu-aarch64 联动验证）；M58-S4 跨架构采真实 /proc 一致 | 无 |
| #36 交叉编译验证（arm64/aarch64 静态二进制） | ⚠️ **主体完成，2 处偏差** | pxc 前端 `--cc aarch64-linux-musl-gcc` 交叉产出 **aarch64 静态 ELF**（M57-S4 devctl 2.5MB static-pie、M58-S4 pxhwmond 多文件工程）+ qemu-aarch64 验证编译运行；tools/cross_aarch64.sh + runtime/mbedtls/lib-aarch64/ 入库 | ① pxi/pxl/pxpar **自身从未交叉成 aarch64**（file 实测四者均 x86-64）——解释器/调试工具不产出目标代码，「交叉产出」语义不成立；若本意是「板子上跑解释模式」则未做（可补：`pxc build --no-quic --cc aarch64-linux-musl-gcc selfhost/interp.px` → qemu smoke）② 加分项「ws-web 交叉 arm64 冒烟 200」无执行记录，且 ws-web 已拆独立私有仓（44637c0），本仓无此代码可验，只能走 ws-web 侧 |

## 七、建议里程碑排期（供主线立项）

| 序 | 里程碑 | 内容 | 规模 / 前置 | 依据 |
|---|---|---|---|---|
| **M59** ✅ | 数学与随机补齐 | **已完成（2026-09，M59）**：`sin`/`cos`/`tan`/`atan2` + `floor`/`ceil`/`round`/`log`/`log10`/`exp` + `random`/`random_int`/`random_seed` + `pi`/`e`（C libm 内置 + splitmix64；pxi 双模式同步 + aarch64 验证）—— 见 ROADMAP 上表 + CHANGELOG + examples/m59_math | 小；语言面最小侵入（已落地） | 游戏+边缘两条线的公共地基（本表「数学欠账」一项已勾销） |
| **M60** ✅ | 边缘 stdlib + 设备小内置 | **已完成（2026-09，M60）**：5 个 C 小内置（`sleep_us`/`now_us`/`fcntl`/`tty_config`/`fd_wait`）+ 第 4 个 stdlib `std.edge`（GPIO V2 line / I2C / serial_open / PWM sysfs）+ 示例（PTY 串口 x86 实跑 / 真板 SKIP 通道）+ pxi 白名单 +5 双模式同步 + aarch64 交叉 qemu 三态一致 —— 见 ROADMAP 上表 + CHANGELOG + examples/m60_dev + stdlib/edge.px | 中；树莓派线 §三 #1–#5 已收敛（#6 真板回归仍候选，需硬件） | GPIO 控制、边沿中断、UART、us 级时钟（§三 #1–#5） |
| 候选 A | FFI 外部库绑定验证 | SDL2 最小窗口或 raylib hello：.a/.so 链接 + pkg-config + 事件循环桥 | 中–大；游戏窗口线 0→1 前提（现 FFI 仅绑内部 C 库、无外部先例） | |
| 候选 B | 真板物理回归 | 树莓派 + LED/按键/温湿度：GPIO/I2C/串口/温度示例真跑 | 需硬件；一切边缘能力的最终裁决 | |

> 备注：里程碑开工按 M58 模式：先出 `docs/M*_PLAN.md` 规划供审 → 审批后按子步落地 +
> verify 回归 + 文档收口。当前（M60 已闭环）候选池：FFI 外部库绑定验证（游戏窗口线
> 0→1 前提，见 §四/§五）与真板物理回归（需硬件，见 §三 #6）。

## 八、与既有文档的关系

- `ROADMAP.md`：本文件是其「远期方向」候选主线排期的明细依据（ROADMAP 只放汇总表）。
- `MINI_SUBSET.md` §十三：M58 dogfood 暴露的语言欠账（其中 §十三.1/.2 HTTP/S3 网络失败→
  Err 已修复；#1/#2 修复实证见 examples/http_neterr_result.px、s3_neterr_result.px、
  m58_hwmond verify_s3.sh D2）。
