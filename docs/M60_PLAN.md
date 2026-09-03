# M60 规划：边缘设备 stdlib edge.px + poll/termios/us 级 5 小内置（树莓派/边缘线深化）

> 状态：**规划入库待审（2026-09）**，本源批准后按 S1–S5 执行
> 主线编号 M60（GAP/ROADMAP 候选主线排期第 2 项）
> 依据：`docs/GAP_ANALYSIS.md` §七 候选 B + §一 树莓派线缺口 #1–#5
> 流程：本规划入库 → 本源审阅（§三 待审决策 D1–D5）→ 批准 → 按子步 S1–S5 执行（每步 verify + commit）→ 里程碑闭环回填 §八
> ✅ **已批准并完成（2026-09，S1–S5 全落地）**，详见 §八回填

---

## 一、为什么是它

GAP_ANALYSIS 树莓派线真缺口按「上真板立刻会撞」排序，M60 一次收敛四个：

| GAP # | 缺口 | M60 落点 |
|---|---|---|
| #1 | GPIO 真输出/读入零封装零示例（现只有只读 chipinfo 胶水样板） | stdlib `edge.px` GPIO V2 line 请求/读写/边沿封装 |
| #2 | GPIO 边沿中断 / 多 fd 等待依赖 poll，语言层**无 fd 多路复用** | C 小内置 `fd_wait`（内部 poll） |
| #3 | 串口 UART 无 termios：设不了波特率/raw 模式 | C 小内置 `tty_config` |
| #5 | 高精度时钟缺失（`sleep` 仅 ms 整数粒度，无 us 级） | C 小内置 `sleep_us` / `now_us` |

且 M57 已铺好 fd 原语（open/ioctl/read/write/mmap 三形态）、M57-S5/M58/M59 已打通
pxi 白名单同步 + aarch64 交叉 + qemu 验证惯例——M60 是在这条成熟管道上的**深化**，
把边缘能力从「理论上通（x86 无设备 SKIP）」推到「示例即用 + PTY 纯用户态可实测 + 真板即插」。

## 二、前置实测复核（本规划事实基准，开工时点复核）

已对本仓库实际代码逐项核对（非记忆推断）：

| 项 | 实测结果 | 证据 |
|---|---|---|
| poll 语言函数 | **无**。runtime.c 中 poll 仅 3 处内部使用（10016/10035/10127，均在 `px_pool_recv_result` 子进程池读帧），未暴露为语言函数 | grep 上下文 |
| 语言层 select | 是**协程 channel 等待语法**（`spawn/chan/send/recv/select`），非 fd 多路复用 | spec.md:68/1240/1327 |
| termios | **全库零命中**（tcsetattr/tcgetattr/termios 均无） | grep |
| us 级睡眠/时钟 | `bi_sleep`（runtime.c:2669）ms 整数粒度 nanosleep；`bi_now_ms`（2718）CLOCK_REALTIME ms；内部 usleep 仅 1 处（9671，非语言函数）；**无 now_us** | 实现体 |
| open mode | 仅 `r/rb/w/wb/a/ab/rw/r+/w+`，**无 O_NONBLOCK/O_NOCTTY** | bi_open 实现体 |
| fcntl 语言函数 | **无**（`px_set_global("fcntl")` 零命中） | grep 注册区 |
| read/write（fd 版） | `read(fd,maxlen)`→bytes（EOF=空 bytes），失败 int -1 + os_errno() 可查；`write(fd,data)`→int 字节数（EINTR 内部重试） | bi_read/bi_write 实现体 |
| ioctl 三形态 | `ioctl(fd,request[,arg])`：null / int 直传 unsigned long（如 I2C_SLAVE=0x0703）/ bytes·str 就地 in-out buffer；失败 -1 + os_errno() | spec §8.17 |
| GPIO 现状 | `m57_s3_gpio.px` 仅只读 `GPIO_GET_CHIPINFO` 68B buffer；无 line request/方向读写/边沿 | examples/ |
| SPI 数据面 | `/dev/spidevX.Y` 本身支持 `read()/write()` 单次传输；配置 ioctl（SPI_IOC_WR_MODE=0x40016b01、WR_MAX_SPEED_HZ=0x40046b04 等）为 int 直传可做；但 **SPI_IOC_MESSAGE 需传含 u64 指针的 transfer 数组 → 现 ioctl 三形态无法承载「语言 bytes 地址」→ 消息型做不了** | ioctl 语义 + ABI 常量 |
| PWM | sysfs `/sys/class/pwm/pwmchipN`（export/period/duty_cycle/enable）纯文件读写，现 `open/write_file/exists` 已够 → **零新 C** | /sys 布局 |
| PTY 可测性 | M57-S3 已用 /dev/ptmx + pts 对做真内核验证 → **termios/poll/fcntl 全部可纯用户态真内核测试**（无需真板） | m57_s3 验证 |
| pxi 现状 | 白名单已含设备函数 open/close/ioctl/os_errno/read/write/mmap/munmap/mem_write/http_unix + M59 数学全集 | selfhost/interp.px 白名单行 |
| pxi import 限制 | §十三 #8：解释模式对相对路径 import + open/read 链失败（pxc run 报 io 读取失败）→ **stdlib import 主打编译模式**（同 M57/M58 策略）；pxi 仅同步 C 内置注册面 | MINI_SUBSET §十三 #8 |
| 交叉/回归基建 | M57-S4/M58-S4/M59-S4 aarch64 交叉 + qemu 自检惯例在位 | git log + examples/ |
| errno 可用性 | `os_errno()` 已注册（M57-S1）；read 失败恢复 errno 供查询 | 注册区 + bi_read |

## 三、范围决策

### 3.1 路线：语言面 5 个 C 小内置 + stdlib `edge.px` 纯语言封装

理由（基于 §二 实测）：
1. **poll/termios/高精度时钟/fcntl 是内核/libc 接口，纯语言无法表达** → 必须小内置，
   延续 M57「fd 原语」最小侵入模式（C 内置每个 10~40 行）；
2. GPIO V2 请求/边沿结构体胶水、I2C 传感器读写、串口便捷、PWM sysfs 都是
   「结构体布局常量 + ioctl 编排 + read/write」的**确定性样板** → 纯语言 `stdlib/edge.px`
   封装（第 4 个 stdlib，与 collections/semver/webroute 并列），零新 C、dogfood 语言面；
3. 失败语义延续 M57：底层 ioctl/read 失败 int -1 + `os_errno()`（**可检查、不杀进程**，
   与 http/s3 网络失败 Err 化是两条并存通道，此处不混用，见 D3）。

### 3.2 必做清单

**C 小内置（5 个，S1/S2 落地，S4 全进 pxi 白名单同步）：**

| 内置 | 签名/语义 | 服务缺口 | 纯用户态可测？ |
|---|---|---|---|
| `tty_config(fd, baud, raw)` | → bool。tcgetattr →（raw=true 则 cfmakeraw 关 canonical/echo）→ cfsetispeed+cfsetospeed → tcsetattr(TCSANOW)。raw=false 仅改波特率保留原模式 | #3 串口 | ✅ /dev/ptmx（termios 真生效） |
| `fd_wait(fds, timeout_ms)` | → 就绪 fd 的 **list**（空=超时）；poll 错误返回 int -1 + os_errno()。fds 接受 int（单 fd）或 list\<int\>；timeout_ms≥0，0=立即查 | #2 GPIO 边沿/多 fd | ✅ PTY 对端写数据驱动 |
| `sleep_us(us)` | → null。us 级（nanosleep 拆分 sec+nsec），>0 | #5 1-Wire/DHT 时序 | ✅ 计时断言 |
| `now_us()` | → int。CLOCK_MONOTONIC 微秒（测宽/计时；与 now_ms 的 REALTIME 墙钟用途区别，文档写明） | #5 高精度测量 | ✅ 单调递增断言 |
| `fcntl(fd, cmd[, arg])` | → int（标准返回；失败 -1 + os_errno）。O_NONBLOCK=0x800 跨 x86/aarch64 asm-generic 一致 | 设备标准非阻塞姿势 | ✅ PTY |

**stdlib `edge.px`（纯语言，S3；V1 范围）：**
- **GPIO V2 line**：`gpio_request(chip, offset, {dir, edge, bias, debounce, consumer})` → line fd（ioctl
  GPIO_V2_LINE_REQUEST）；`gpio_read(fd)` → int 0/1（read 1B）；`gpio_write(fd, v)`（write 1B）；
  边沿事件：`fd_wait(linefd, ms)` 后 `read(fd, 16)` 解析 `gpio_v2_line_event`（含 timestamp/line/seqno）。
  uAPI 结构体布局常量（x86/aarch64 asm-generic 一致）入库。
- **I2C 传感器 helper**：`i2c_open(bus_n, addr)`（open `/dev/i2c-N` + I2C_SLAVE int 直传）；
  `i2c_read_reg(fd, reg, n)` / `i2c_write_reg(fd, reg, bytes)`（寄存器寻址传感器标准，SMBus 风格）；
  示例接 BMP280/SHT30（真板可选）。
- **串口 helper**：`serial_open(path, baud)` = open(rw) + tty_config(raw) + fd_wait/read 封装。
  注：非交互脚本非会话 leader，open 无 O_NOCTTY 不影响（文档注明；若未来交互场景需补 fcntl 加 O_NOCTTY）。
- **PWM sysfs helper**：`pwm_setup(chip, ch, period_ns, duty_ns)` + `pwm_enable(chip, ch, on)`——
  export/period/duty_cycle/enable 纯 write_file（零新 C）。

**示例（examples/m60_*.px，沿用 M57「x86 实跑 / 真板 SKIP」双通道）：**
- `m60_serial_pty.px`：**x86 实跑**——open /dev/ptmx → tty_config 从端 → 主端写 → fd_wait → read → 断言（串口全链路真内核验证）；
- `m60_gpio.px`：真板树莓派 GPIO V2 读按键 + 边沿事件计数（x86 无 gpiochip SKIP，沿用 M57 探测模式）；
- `m60_i2c.px`：真板 I2C 总线扫描 + 读传感器（x86 无器件 SKIP）；
- `m60_pwm.px`：真板 PWM LED 呼吸（x86 无 pwmchip SKIP）。

### 3.3 错误/边界语义（明确，写进 spec）
- **小内置参数个数/类型错误** → `px_error` 终止（编程契约，延续 M57/M59）；
- **设备失败（可预期）** → 与 M57 fd 原语一致：返回 int -1 / false + `os_errno()` 查询，**不杀进程**；
- **fd_wait 超时不是错误**：空 list（多路）/ 与就绪同通道；
- **now_us/sleep_us**：now_us 用 CLOCK_MONOTONIC（测量语义，不受墙钟调整影响），文档与 now_ms(REALTIME) 用途区分写清；
- **errno 全局性**：edge.px helper 失败即查 os_errno，勿跨调用缓存（单线程主循环内安全）。

### 3.4 明确不做（控制面）
- **SPI_IOC_MESSAGE**（transfer 数组含 u64 指针，现 ioctl 无法承载「语言 bytes 地址」）——spidev
  read/write 半双工替代够 V1；全双工+CS 控制需新机制，留档按需；
- **termios 精细逐字段**（ixon/opost/VMIN/VTIME 等）不暴露——tty_config 只 raw+baud 覆盖串口 95% 场景；
- **GPIO V1 legacy uAPI / sysfs export** 不推（新内核 v2 uAPI 为准）；
- **/dev/input 事件解析、/dev/fb0 图形原语** → 属游戏线（M 后候选），M60 只留 fd 数据通道；
- **pxi 相对 import 修复**（§十三 #8）→ 非本里程碑；stdlib import 主打编译模式，pxi 仅同步 C 内置注册面；
- **真板物理回归**（需树莓派硬件）→ 硬件到位前全部真板段示例 SKIP 通道 + qemu 交叉验证；真板回归单独立项（GAP §七）。

### 3.5 待审决策（D1–D5，批准前定夺）
- **D1 `fd_wait` 签名**：推荐单函数 `fd_wait(fds, ms) → list<ready>`（int/list 双收，统一）；
  备选双函数 `fd_wait(fd,ms)→int 三态` + `fd_wait_all(fds,ms)→list`。
- **D2 `fcntl` 是否保留**：保留=5 内置（通用 fd 控制，边缘样板标准姿势）；可砍=4 内置（fd_wait 就绪后 read 实际不阻塞）。
- **D3 `edge.px` 失败语义**：推荐透传 int -1 + os_errno（与 M57 fd 层一致，薄封装）；备选 Ok/Err 包装（M58 http 风格，封装变厚）。
- **D4 GPIO 只做 V2 uAPI**（不做 V1/sysfs），认可？
- **D5 示例集范围**：PTY 串口 loopback（x86 实跑）+ gpio/i2c/pwm（真板 SKIP），是否再加 DHT11 us 时序真板示例（dogfood sleep_us）？

## 四、子步划分（每步：代码 + verify 脚本 + 独立 commit）

| 子步 | 内容 | 验证 |
|---|---|---|
| **S1** | C 小内置·时钟组 + fcntl：`sleep_us` / `now_us` / `fcntl` + 注册 | verify_s1：计时断言、单调递增、PTY 非阻塞三态 |
| **S2** | C 小内置·设备组：`tty_config` / `fd_wait` + 注册 | verify_s2：PTY termios raw 真生效（无 \n 回读 1B）、波特率、fd_wait 就绪/超时/错误三态 |
| **S3** | stdlib `edge.px`（GPIO V2 / I2C / 串口 / PWM helper）+ examples m60_* + verify | verify_s3：PTY 串口 loopback x86 实跑；gpio/i2c/pwm 真板 SKIP 通道；库函数单测（布局常量断言） |
| **S4** | pxi 白名单 +5 + ibuiltin 转发 + pxi 重建 + **aarch64 交叉 qemu 冒烟**（PTY/termios/poll/fcntl 跨架构一致）+ 回归 | verify_s4：编译/解释/qemu-aarch64 三态；m57/m58/m59 关键回归 |
| **S5** | 文档收口：spec §8.18、MINI_SUBSET §十三.5、ROADMAP/GAP 勾选、CHANGELOG | verify_s1–s4 复跑 + git 干净 + PLAN 回填 §八 |

## 五、回归矩阵

| 验证对象 | S1 | S2 | S3 | S4 | S5 |
|---|---|---|---|---|---|
| 新 verify_sN | ✅ | ✅ | ✅ | ✅ | ✅ 复跑 |
| hello.px / fib.px | ✅ | ✅ | ✅ | ✅ | ✅ |
| m57_s1/s2/s3（fd/mmap/devctl） | | | ✅ | ✅ | ✅ |
| m58_s1/s2/s3/s4（采集/mmap/HTTP+告警/自愈交叉） | | | | ✅ | ✅ |
| m59 数学 verify_s1–s3 | | | | ✅ | ✅ |
| aarch64 交叉 qemu | | | | ✅ | ✅ |

## 六、里程碑闭环判据
- 5 小内置全注册 + pxi 白名单同步 + 文档（spec/MINI_SUBSET/ROADMAP/GAP/CHANGELOG）落齐；
- `stdlib/edge.px` 入库；每类设备（GPIO/I2C/串口/PWM）至少一个示例：x86 可跑段实跑 PASS、真板段 SKIP 通道通畅；
- 全量 verify_s1–s4 + 关键历史回归 + aarch64 交叉冒烟 PASS；git 干净、commit 齐全。

## 七、风险与备注
- **GPIO V2 uAPI 结构体对齐**：含 64 位字段（flags/offsets/event 结构含 timestamp u64），edge.px 布局常量需
  按 asm-generic 精确书写；x86/aarch64 布局一致（M57-S4 已实证 ifreq/ioctl 码跨架构一致），但 S3 要逐字段核对
  （可先用本地 struct 布局常识对照内核头核对，真板到位后实测兜底）。
- **errno 全局单线程语义**：协程并发共享 fd/交错查询 errno 有竞态 → edge 用例单线程主循环为主，文档警告。
- **fd_wait 就绪后 read 不阻塞**（poll 保证）→ 串口/GPIO 读安全；但仍建议严格「fd_wait 再 read」顺序，防竞态。
- **pxi 白名单行已很长**（~100 函数），S4 追加时保持单行可读性（必要时换行策略随代码评审定）。
- **规模估计**：C 内置 5 个 ~120 行；edge.px ~350–450 行（GPIO V2 结构体为主体）；示例 + verify 4–6 个；
  M60 总量中等偏大，子步化后每步可控（延续 M57/M58 节奏）。

## 八、执行回填（✅ S1–S5 全落地，2026-09）

> 本源批准开工（D1–D5 按推荐默认：D1 单函数 fd_wait 收 int/list、D2 保留 fcntl=5 内置、
> D3 edge.px 透传 -1+os_errno、D4 GPIO 只做 V2 uAPI、D5 示例集按默认范围），全流程
> verify_s1–s4 全 PASS + git 干净。各子步 commit：
> - **S1** `fa91805`：sleep_us/now_us/fcntl 三内置 + dev_s1（计时/单调/PTY 非阻塞三态/EAGAIN）
> - **S2** `bc97b20`：tty_config/fd_wait + dev_s2（PTY termios raw 真生效/波特率/fd_wait
>   就绪·超时·HUP·单int与list双形态/空集合）
> - **S3** `f5b1b03`：stdlib/edge.px（246 行）+ m60_serial_pty（x86 实跑 PTY 串口双向
>   loopback）/m60_gpio/i2c/pwm（真板 SKIP）+ dev_s3 布局单测（GPIO V2 592B 结构按 C
>   offsetof 实测核对）
> - **S4** `f7e21b5`：pxi 白名单 +5 + ibuiltin 转发 5 分支 + bootstrap/pxi 重建 + dev_s4
>   编译/pxi/qemu-aarch64 三态一致 + hello/fib/m59/m57 回归
> - **S5** 本文档回填 + spec §8.18/§10.3 + MINI_SUBSET §十三.5 + ROADMAP/GAP 勾选 + CHANGELOG
>
> **开工实测新发现（已记录 MINI_SUBSET §十三.5）**：TIOCSPTLCK（_IOW 写 int）ioctl arg
> 须 bytes/int_to_bytes buffer（int 0/NULL → EFAULT errno=14）——_IOWR/_IOW 类一律
> bytes buffer 最稳。**范围外留档**：SPI_IOC_MESSAGE 全双工（transfer 含 u64 指针）、
> GPIO 旧内核 ABI（≤6.x offsets u64 版）、真板物理回归（#6，待硬件）。
