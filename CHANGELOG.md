# Changelog

本项目的所有重要变更都会记录在此文件。
格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，
版本语义遵循 [Semantic Versioning](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### M60 · 边缘设备深化（树莓派线：5 小内置 + std.edge 第 4 个 stdlib）✅

- **规划**：`docs/M60_PLAN.md`（前置实测复核：poll 仅 runtime 内部 3 处未暴露、termios
  全库零命中、sleep 仅 ms 整数粒度、SPI_IOC_MESSAGE 因 transfer 数组含 u64 指针留档、
  GPIO V2 单线请求 592B 结构体 C offsetof 实测核对）。
- **S1 us 级时钟 + fd 控制**：`sleep_us(us)`（nanosleep，EINTR 续睡，<=0 不睡）/
  `now_us()`（**CLOCK_MONOTONIC** 微秒，测量语义与 now_ms 的 REALTIME 墙钟区分）/
  `fcntl(fd, cmd[, arg])`（标准 fcntl，O_NONBLOCK 等；失败 -1+os_errno）—— commit
  `fa91805`。
- **S2 设备组**：`tty_config(fd, baud, raw)`（tcgetattr → cfmakeraw(raw) → cfsetispeed/
  cfsetospeed → tcsetattr(TCSANOW)；baud 9600…921600，无效档终止；失败 false+errno）/
  `fd_wait(fds, timeout_ms)`（内部 poll 暴露：int/list<int> 上限 64、只监听 POLLIN、
  revents 非 0 即事件返回（含 HUP）、**超时空 list 非错误**、poll 错误 -1+errno）——
  commit `bc97b20`。
- **S3 stdlib std.edge**（第 4 个 stdlib，纯语言零新 C）：GPIO V2 line（gpio_input/
  gpio_output/gpio_input_edge/gpio_request + read/write/wait/event，592B 布局按
  linux/gpio.h offsetof 实测；OUTPUT 初始电平经 attr OUTPUT_VALUES）+ I2C（i2c_open/
  i2c_read_reg/i2c_write_reg，write-then-read 两笔事务）+ serial_open（open rw +
  tty_config raw 一站式）+ PWM sysfs（pwm_setup/enable/set_duty，open 通道写失败不杀
  进程）+ 示例（m60_serial_pty **x86 实跑 PTY 真内核串口双向 loopback**、m60_gpio/
  m60_i2c/m60_pwm 真板段 SKIP）+ dev_s3 布局常量断言单测—— commit `f5b1b03`。
- **S4 双模式同步**：pxi 白名单 +5 + ibuiltin 纯转发 5 分支（参数预检返回 Err、设备失败
  -1/false+errno 透传不包装）→ `bootstrap/pxi` 重建；dev_s4 编译/pxi/qemu-aarch64 三态
  断言全过、双模式输出逐字节一致（PTY/termios/poll/fcntl/us 时钟跨架构一致）；
  hello/fib 双模式 + m59 math_s1 解释 + m57_s1 编译回归 PASS —— commit `f7e21b5`。
- **S5 文档收口**：spec §8.18 + §10.3 std.edge 行、MINI_SUBSET §十三.5、ROADMAP M60
  勾选（候选池移除 M60）、GAP_ANALYSIS §三 #1–#5 ✅ + §七 M60 ✅（本 commit）。
- 语义要点：失败 -1/false + os_errno()（延续 M57 fd 原语，可检查不杀进程）；参数错误
  px_error 终止；fd_wait 超时返回空 list 非错误；GPIO 布局按 2024+ 内核（offsets u32
  版）书写，旧内核需按目标头调整；真板物理回归（#6）候选待硬件。
  样例与验证：`examples/m60_dev/`（dev_s1~s4.px + verify_s1~s4.sh）+ `stdlib/edge.px`。

### M59 · 数学与随机补齐（C libm 内置，14 函数 + 2 常量）✅

- **规划**：`docs/M59_PLAN.md`（前置实测复核：现有数学内置仅 abs/min/max/sum/sqrt/pow 六枚；
  sin/cos/tan/atan2、floor/ceil/round、log/log10/exp、random 全缺；libm 已 include + `-lm`
  已链接 → 零新依赖）。
- **S1 三角 + pi**：`sin`/`cos`/`tan`（弧度）/`atan2(y,x)`（先 y 后 x）+ `pi` 常量
  （本地宏 `PX_PI` 全精度）—— commit `8a85834`。
- **S2 取整/对数 + e**：`floor`/`ceil`/`round`（C99 round，.5 远离零；返回 float 与 sqrt
  一致）+ `log`(自然对数 ln)/`log10`/`exp` + `e` 常量（`PX_E` 全精度）；域错误透传 C 语义
  不终止（log(-1)→NaN、log(0)→-inf、exp(1000)→+inf）—— commit `93eb9a9`。
- **S3 随机（splitmix64）**：`random()`→float∈[0,1)（53 位尾数均匀）、`random_int(n)`→
  int∈[0,n)（n>0）、`random_seed(s)` 设种子后同 seed 同序列；确定性 64 位 PRNG 不依赖 C
  rand 的 glibc/musl 平台差异（静态二进制 + aarch64 序列可复现）；默认种子首次调用惰性
  初始化（realtime ns ^ pid）—— commit `bddd953`。
- **S4 双模式同步**：pxi 解释器白名单 +15（含补平 sqrt 编译有解释无的不对称）+ `pi`/`e`
  常量种子（读宿主全局，规避浮点字面量 6 位截断）+ `ibuiltin.px` 数学分发 5 分支与
  helper（参数预检返回 Err、域错误 NaN/inf 透传）→ `bootstrap/pxi` 重建；编译/解释/
  qemu-aarch64 三态断言全过、双模式输出逐字节一致、splitmix64 序列 x86==aarch64 逐位
  一致；hello/fib 双模式 + m57_s1/m58_s1 回归 PASS —— commit `3f7e434`。
- **S5 文档收口**：spec §10.2/§10.3、MINI_SUBSET §十三.4、ROADMAP M59 勾选、
  GAP_ANALYSIS 数学缺口勾销（本 commit）。
- 语义要点：参数个数/类型错误 → px_error 终止（编程契约，与 abs/sqrt 一致）；域错误 →
  NaN/inf 透传不终止；atan2 先 y 后 x；floor/ceil/round 返回 float 需 int() 转整数；
  pi/e 全精度常量。样例与验证：`examples/m59_math/`（math_s1~s4.px + verify_s1~s4.sh）。

### 文档 · 能力差距分析 + 候选主线排期（docs/GAP_ANALYSIS.md + ROADMAP 远期方向）

- 新增 `docs/GAP_ANALYSIS.md`：树莓派/边缘设备 与 2D/3D 游戏两条用户线的能力差距清单
  （事实基准：~257 内置函数清单 / stdlib / spec / ROADMAP / MINI_SUBSET §十三 / M57/M58
  实测产物）。
- 核心结论：树莓派线「能力已开、缺厚度与真板验证」（GPIO 真正控制、边沿中断 poll/epoll、
  串口 termios、SPI/PWM 封装、us 级时钟、真板物理回归）；2D 游戏「几乎未开」（缺图形/
  输入/数学基础——终端 ASCII 半通、帧缓冲原语零、SDL2/raylib 0→1 未做）；3D 明确不做
  原生绑定、唯一路径 = raylib C 绑定（前置 FFI 外部库绑定验证）。
- 历史任务覆盖核对：#37 → M57 S1–S4 **100% 闭环**；#36 → 主体完成（pxc 交叉产出 aarch64
  静态 ELF + qemu 验证），遗留 = 原生 aarch64 pxi 未做（如需「板子跑解释模式」）+ Mahesvara
  加分项已随拆仓移出本仓。
- ROADMAP 远期方向新增「候选主线排期」：M59 数学与随机补齐（小）→ M60 边缘 stdlib +
  设备小内置（中）→ 候选 FFI 外部库绑定验证 / 真板物理回归（需硬件）。

### M58 dogfood 闭环 · pxhwmond webhook dry-run 解禁为真发（HTTP Err 语义落地）

- **背景**：M58 pxhwmond 的 webhook 通知因 HTTP 客户端网络失败即 panic（§十三 #1/#2）
  只能 dry-run 报文落盘；HTTP/S3 网络失败 → `Err(result)` 修复（§十三.1/.2）后语言面已
  允许 daemon 内安全发起网络请求 → 本条目把 dry-run 正式解禁为真发，完成 dogfood 闭环。
- **改动（examples/m58_hwmond/）**：
  - `notify.px`：`webhook_dryrun` → `webhook_send`——`http_request(url,"POST",JSON,
    {Content-Type:application/json},{timeout_ms:3000,retries:0})` 真发；网络失败（返回
    `Err`，以 `type(resp)=="result" and resp.is_err()` 判别、`resp.err()` 取文本）→ 发送
    日志 `sent:false`+`err` 落盘、进程不死；HTTP 2xx → `sent:true`+`status`、非 2xx →
    `sent:false`+`http_status=…`。env `PXHWMON_WEBHOOK_DRYRUN_LOG` → `PXHWMON_WEBHOOK_LOG`
    （发送结果日志）。
  - `main.px`：调用点 `webhook_dryrun` → `webhook_send`；头注释同步。
  - 新增 `webhook_mock.px`：PuXian 自举的本地 webhook 接收 mock（`http_serve` handler 把
    method/path/body 落盘 JSONL），verify_s3.sh D1 真发验收用。
  - `verify_s3.sh`：D1 成功路径——阈值触发 3 轮 → mock **实收 3 条 POST /alert**（body 含
    alert）+ 发送日志 3 条 `sent:true`；D2 失败路径——webhook 指向 `127.0.0.1:1`（连接
    拒绝）→ daemon 3 轮跑完退出码 0、3 条 `sent:false`+`err`（§十三 #1 修复实证，原先
    会 panic 杀进程）；A-C 段改端口就绪探测 + client 重试（daemon 冷启动偶发 >2.5s）。
- **验证**：verify_s3.sh PASS（A-C 状态页响应头 + D1 mock 实收真发 + D2 失败不 panic）；
  verify_s1/s2/s4 回归 PASS（见提交记录）。
- 记录：MINI_SUBSET §十三.3、m58 README 边界更新（dry-run 描述 → 真发 + Err 语义闭环）。

### 语言面修复 · S3/MinIO 客户端网络失败 → Err(result)（与 HTTP 客户端同源收口）

- **问题**：`s3_put`/`s3_get`/`s3_delete`/`s3_list`（M37 引入）网络失败（协议不支持/
  建连失败/TLS/IO/连接中断）**静默返回 false/null/空 list**——无错误可见性，调用方无法
  区分「网络挂了」与「服务端拒绝」（与 HTTP 客户端 px_error 杀进程同源、但更隐蔽：
  M37 的 px_s3_exec 建连失败 `return 0` 无声无息）
- **修复（runtime/runtime.c，零新增内置函数）**：上述 4 个 S3 客户端函数**网络失败统一
  返回 `Err(result)`**（消息 `"net: ..."`），进程不终止，调用方可 `is_err()`/`?`/unwrap
  处理；**成功/应用层语义不变**：服务器正常响应时 put/delete → bool、get → string(200)
  或 null(其他)、list → keys 列表（403/404/500 等应用层状态码非网络失败，走原语义）；
  参数个数/类型错误（编程契约）仍终止。实现：`px_s3_exec` 增加错误缓冲输出
  （`px_net_fail` 填 errbuf，沿用 §十三.1 HTTP 修复的 helper）；endpoint 增加协议校验
  （非 http/https → Err，原来静默当明文 HTTP 处理）；建连失败/h_exchange 中断分别填
  `net: 连接 ... 失败` / `net: S3 请求失败: 连接中断`
- **验证**：新增 `examples/s3_neterr_result.px` 自检：连接拒绝/协议不支持 → 4 函数全
  is_err=true + 进程存活 ALIVE + 退出码 0；本地最小 S3 mock（http_serve）应用层
  200/404/204 → true/body/null/keys 语义不变（非 Err）；m37_s3.px 成功路径全链路回归
  M37_S3_OK（含 SigV4 签名 mock）；http_neterr_result.px 回归 PASS（共享 h_exchange
  helper 未破坏）；pxi 重建后解释模式同 PASS
- 记录：MINI_SUBSET §十三.2（§十三 #1/#2 属 HTTP 已修；本条目把 S3 静默失败一并收口为
  「网络错误可检查」，语言面网络 I/O 失败语义趋同）

### 语言面修复 · HTTP 客户端网络失败 → Err(result)（M58 dogfood 欠账 #1/#2 根因）

- **问题**：`http_get`/`http_post`/`http_request`/`http_unix`/`http_get_stream` 网络失败
  （解析/建连/TLS/IO/重定向/协议不支持）直接 `px_error` → 打印「运行时错误:」并 exit(1)，
  无错误返回；spawn 协程内网络调用失败会杀整个进程（MINI_SUBSET §十三 #1/#2，M58
  pxhwmond dogfood 暴露：webhook 通知只能 dry-run 落盘、不能真发网络）
- **修复（runtime/runtime.c，零新增内置函数）**：上述 5 个 HTTP 客户端函数**网络失败统一
  返回 `Err(result)`**（消息 `"net: ..."`，保留原 px_error 文案），进程不终止，调用方可
  `is_err()`/`?`/unwrap 处理；**成功返回值不变**（http_get/post → body 字符串；
  http_request/unix → dict{status,headers,body}；get_stream → bool）；参数个数/类型错误
  （编程契约）仍 px_error 终止；HTTP 应用层状态码（404/500）仍由 dict.status 返回（不算
  网络失败）。实现：底层 `px_http_once`/`px_http_request` 增加错误缓冲输出
  （`px_net_fail` 返回 NULL 信号，修复一处 NULL 未检解引用隐患）；`hparse_url` 改返回
  错误码不再终止；builtin 用 `px_net_err` 就地构造 Err
- **验证**：新增 `examples/http_neterr_result.px` 自检（连接拒绝/协议不支持/unix socket
  不存在 → is_err=true + 进程存活 ALIVE + 退出码 0）；成功路径不变（本地 HTTP 状态页
  http_get/http_post 返回 string、http_request 返回 dict status=200）；M58 verify_s1/s2/s3
  回归全 PASS；aarch64 交叉链路 verify_s4 复跑
- 连带：#2（spawn 不隔离 panic）在该场景**根除**——网络失败已不 panic；协程内其他运行时
  错误（除零等）仍不隔离（MINI_SUBSET §十三.1 部分缓解记录）

### 新增 · M58 首个 dogfood 真实应用：pxhwmond 硬件健康守护 daemon（examples/m58_hwmond，见 docs/M58_PLAN.md）

- **首个 dogfood 里程碑**：用 PuXian 写**真实边缘应用**——单静态二进制硬件健康守护 daemon，
  把 M57 能力（fd 原语数据通道 / mmap 活映射 / aarch64 交叉编译）揉进完整真实程序，
  x86 实测 + aarch64 交叉 qemu 验证（1→1.0n 验证闭环）
- **多文件 import 工程**：main/collect/shm/serve/notify 4 模块拆分（每文件 <500 行），
  `import "collect.px"` 相对路径源码模块编译模式实测可用
- **采集（M57 fd 通道主路径）**：/proc/stat CPU 差值 / meminfo / loadavg / uptime /
  net/dev（非 lo 汇总）+ 温度 hwmon/thermal 条件探测降级（缺 → `temp=na` 不崩）
- **mmap MAP_SHARED 活映射 IPC**：快照区 + 控制区 4096B 共享文件；`--dump` 外部活读、
  控制区命令通道**双向可见**（外部写 → daemon 下轮快照回显 `ctl=`，verify_s2 轮询实证）
- **手写最小 HTTP 状态页**（D4 决策：不引 px_serve docroot）：`GET /healthz` JSON +
  `GET /` HTML 表格 + 404；**显式响应头** Content-Type/Content-Length/Connection/Server
  （M57-S7「vhost 响应头丢失」教训自验，verify_s3_client http_request 逐头断言）
- **监控 + 自愈 + 通知（MONITORING/P0 落地载体）**：/healthz 心跳 + mmap 实时快照 +
  run.sh 崩溃自动重启（kill -9→137→attempt 递增）+ 阈值告警（内存/负载/温度 env）→
  告警日志 + webhook dry-run 报文落盘
- **aarch64 交叉 + qemu**：多文件 import 工程 `pxc build --no-quic` 交叉 aarch64 静态
  产物（2.5MB）→ qemu-aarch64 `--once` 采集真实 /proc（mem_total 与 MemTotal 一致，
  跨架构同源实证）；`--no-shm`/`--once` 保证自检可退出不裸奔
- **语言欠账记录（MINI_SUBSET §十三）**：http_post 失败即 panic + spawn 不隔离 panic
  （→ webhook 只能 dry-run，真实网络发送待语言补错误返回，最优先）；int() 前缀截断；
  `{}` 空 dict 不可靠（用 json_parse("{}")）；import 只合并 def 不执行模块顶层；mmap
  固定 PROT_RW 须 O_RDWR fd
- 验证：verify_s1–s4.sh 全 PASS（s1 mem 精确对拍 / s2 双向活映射 / s3 HTTP 响应头 +
  告警 / s4 崩溃自愈 + 交叉 qemu）；M57 s1/s3 回归 PASS；M58 commits：S1 `1694720` /
  S2 `c821005` / S3 `b92be61` / S4 `25aa4bb`（规划 `4000dc3`）

### 仓库治理 · Mahesvara 迁出至独立私有仓库（开源 / 私有物理隔离）

- 仓库外私有生产应用 Mahesvara（私有 webserver 系统）已从本仓库 `git rm` 并迁至**独立私有仓库**维护；
  本仓库只保留开源内容，`git grep Mahesvara`（tracked）零残留
- `.github/workflows/ci.yml` 移除 `wsweb` job（生产应用回归）；README/README.en/ROADMAP/spec/CHANGELOG/
  M53/M54/M57_PLAN/CONTRIBUTING/.gitignore/examples 中 14 文件 39 处 Mahesvara 引用一律中性化
  （「外部生产应用」表述；repro_h2_vhost/repro_tls_no_sni 标题 `[Mahesvara-blocker]`→`[dogfood-blocker]`，
  脚本自包含语义不变）
- 背景：Mahesvara 为私有系统，与开源 PuXian 同仓导致提交边界不清（曾多次直推并被 revert）；
  物理拆仓后「改 PuXian」与「推 Mahesvara」权限隔离，PuXian 侧对 Mahesvara 的参与回归 PR 流程；
  git 历史完整保留（未清史）；Mahesvara 曾以真实应用身份抓出的 H3/并发类 bug 已在 CHANGELOG M53–M56 留存

### 新增 · M57 边缘设备层支持（Linux 用户态，见 docs/M57_PLAN.md）

- **fd 原语内建（S1）**：`open(path[, mode]) → fd`（mode `r/w/a/rw/w+` → O_*）、
  `close(fd) → bool`、`ioctl(fd, request[, arg]) → int`（arg 三形态：缺省→NULL /
  int 直传 / bytes·str 就地 in/out buffer，_IOR 类内核直接填充同对象）、
  `os_errno() → int`（线程局部 errno）——清歌方案"只缺 ioctl"核查后修正：
  runtime 文件 IO 原为路径式（read_at/write_at 内部 open 用完即关），语言面无持久
  fd 句柄，故一并补 fd 原语打通「打开设备 → ioctl → 关闭」闭环
- **fd 数据通道 + mmap 活映射（S2）**：`read(fd, n) → bytes` / `write(fd, data) → int`
  （read/write(2) 直通，EINTR 重试）；`mmap(fd, len[, offset]) → bytes`（PROT_RW +
  MAP_SHARED，GC 自动 munmap，失败 -1+errno）；`munmap(bytes) → bool` 显式解除；
  `mem_write(map, offset, data) → int` 就地写映射区（bytes_set 是 COW 复制语义，
  帧缓冲/共享内存写像素必备）；LXObject 位域新增 is_mmap（px_obj_free 对 mmap
  bytes 走 munmap 而非 xfree）
- **GPIO/I2C 示例 + 真内核替身验证（S3）**：m57_s3_gpio.px（GPIO_GET_CHIPINFO_IOCTL
  buffer 解析）、m57_s3_i2c.px（I2C_SLAVE int 形态直传从地址）、m57_s3_devctl.px
  （loopback 网卡 ifreq + PTY TIOCGPTN 全真实内核硬断言）——LD_PRELOAD mock 因
  pxc 静态链接不可行，改内核自带用户态可访问设备走同胶水路径，验证力度更强
- **aarch64 交叉编译 + runtime 裁剪（S4）**：`pxc build --no-quic [--cc] [--mbedtls-lib]
  [--sqlite-obj]`；runtime PX_NO_QUIC 条件编译（7 处）+ musl 兼容 5 点（execinfo
  条件 / GC aarch64 寄存器扫描分支 / getcontext→内联汇编 SP+setjmp spill /
  close_range 循环）；tools/cross_aarch64.sh（mbedtls 3.6.2 + sqlite3 交叉入库）；
  qemu-aarch64 跑静态产物设备层 ioctl 与 x86 结果一致（asm-generic ioctl 码跨架构
  实证一致）
- **pxi 重建解释同能力（S5）**：interp.px 白名单 +10 + ibuiltin.px 纯转发分支 →
  `pxc build selfhost/interp.px` 重建 bootstrap/pxi；examples/m57_s5_pxi_smoke.px
  双模式输出一致；capability 双模式各 253 PASS 输出逐字节一致；diffcheck --all /
  --errors 全绿；自举证明 B.c==golden 逐字节一致
- 目标场景：树莓派/网关/盒子等 Linux 边缘设备层（单静态二进制免 Python 环境）；
  裸机 MCU（STM32/ESP32）因 runtime 含 GC/线程/动态值与无 OS 架构不符，明确不做；
  通用动态 FFI（dlsym）待「任意 C 库即插即用」真需求再上
- **里程碑闭环（S6 文档收尾）**：spec §8.17（边缘设备层 fd 原语 / 数据通道 / mmap 活映射 API 文档）+
  ROADMAP（主线表 M57 行 + 能力基线「边缘设备层」+ 远期 M57 段移入完成）+ README / README.en
  （特性表「🔌 边缘设备」+ 里程碑表 M57 + 示例列表 m57_s1–s5）+ 本 CHANGELOG 同步定稿；
  M57 全部代码 commit：S1 `57bb9d7` / S2 `f71b28e` / S3 `8f6e615` / S4 `bbffcd5` / S5 `62d9275`

### 修复 · M57-S7 vhost handler 自定义响应头丢失（外部生产应用 BUG_REPORT）

- 现象：`vhost(host, handler)` 返回 `dict{status,headers,body}` 时，headers 除
  `Content-Type` 外全部丢失 → 301 无 `Location`（浏览器不跳转）、`Cache-Control`
  缓存头失效、`Set-Cookie` / CORS 头均受阻
- 根因：`runtime.c px_vhost_normalize` 仅 `px_dict_get_ci("Content-Type")` 单头透传，
  未透传其余响应头（最小复现位于仓库外生产应用私库，本次以仓库内自包含复现等价验证）
- 修复：`px_vhost_normalize` 增加 `extra` 出参 + 白名单透传（`Location` /
  `Cache-Control` / `Content-Disposition` / `Content-Language` / `Set-Cookie` /
  `X-Robots-Tag` / `Access-Control-*` CORS 头），键、值任一含 CRLF 即整体丢弃（防注入），
  extra 写满安全截断（响应头缓冲 2048 兜底）；Content-Type 仍走独立通道；调用侧
  extra 缓冲 256→1024（X-Request-Id 之后追加）
- 新增回归：`examples/m57_s7_vhost_headers.px`（编译模式 ALL OK）——覆盖①白名单透传
  ②非白名单头丢弃 ③值含 CRLF 丢弃 ④键含 CRLF 不崩溃 ⑤Content-Type 独立通道；
  pxi 解释器不含 vhost builtin（Mini 子集边界），修复在 runtime C 层对编译部署形态生效
- 回归（runtime 变更全量）：pxi 重建 9,050,296B；capability 解释 + 编译双模式各
  253 PASS 输出逐字节一致；diffcheck --all / --errors 全绿；自举证明 B.c 与
  golden/compiler.c 逐字节一致（6381 行）；m31_vhost（vhost/限流/CORS）ALL OK


### 路线图 · M57 内容重定向（HTTP/3 深度生产化 → 健壮性加固 → 边缘设备层支持）

- **HTTP/3 深度生产化剩余项**（QPACK 动态表前缀 / 服务端主动迁移·immediate migration /
  深度互操作扩展）**降级为搁置**：Chrome 2021 起禁用 QPACK 动态表（容量=0）、immediate
  migration 连 ngtcp2 上游都未实现、深度互操作边际收益递减 —— 均无真实用户场景（详见
  `docs/ROADMAP.md` §三「搁置」评估注记）
- **HTTP/3 / QUIC 健壮性加固**（曾于上版设为 M57 候选主线）**同步降级为搁置**：H3 目前
  无真实用户（自签证书下浏览器退回 HTTP/1.1，不走本栈），给无人使用的栈做 fuzz / 并发
  审计价值前提不成立（1→1.01 而非 0→1）；待 H3 出现真实用户（如生产应用配真证书公网/
  浏览器实测）再捞回
- **M57 重定向为主线下一条里程碑「边缘设备层支持（Linux 用户态）」**：与清歌嵌入式讨论
  的落地结论（PuXian 只能到树莓派/网关/盒子等 Linux 边缘设备层；裸机 MCU 架构不符，
  明确不做）——相对 H3 打磨属 **0→1 开新使用域**（物理世界接口）。S1 ioctl 胶水内建
  （约 100–200 行 C，通吃 i2c/spi/gpio/tty/网卡）→ S2 mmap/munmap 设备映射 → S3
  GPIO/I2C 示例 + x86 ioctl mock 验证 → S4 aarch64 交叉编译 + qemu 验证 + runtime 裁剪
  开关 → S5 pxi 重建 + capability/diffcheck/自举全绿 → S6 文档；通用动态 FFI（dlsym）
  等「任意 C 库即插即用」真需求再上

### 新增 · M56 runtime http_unix 内建（外部生产应用配套，非主线 HTTP/3 里程碑）

- `http_unix(sock_path, url_path, method[, body[, headers]]) -> dict{status, headers, body}`
  —— Unix domain socket HTTP 客户端内建（本地服务 / LLM 网关 / 容器 daemon 调用）：
  每次新建 AF_UNIX 连接、`Connection: close` 用完即关；收发超时 180s（本地长响应）；
  带 body 且未显式带 Content-Length 时自动补（Content-Type 可被 headers 覆盖）；
  响应解析复用 h_exchange（与 http_get/http_post 同解析器）
- 引入背景：外部生产应用 LLM 接入词元缓存网关（unix socket 通道，key 零落盘）；
  由清歌提交（runtime/runtime.c +73；仓库外应用侧 llm.px / translate.px / main.px），
  编号与主线 HTTP/3 里程碑并行插队 —— 本条目为 PuXian runtime 侧文档补齐

### 新增 · M54 HTTP/3 生产化（见 docs/M54_PLAN.md）

- **TLS 1.3 会话恢复（1-RTT resumption）**：server SSL_CTX 开启 stateless session ticket；
  语言 API `quic_connect_resume(ip,port,alpn,session)` / `quic_session_save(conn)` /
  `quic_conn_resumed(conn)` —— 二次连接确为 1-RTT 恢复（resumed=true）
- **0-RTT early data**：server `SSL_CTX_set_max_early_data` + 收包路由登记客户端 Initial
  DCID（修复 0-RTT 长头包被丢）；语言 API `quic_0rtt_save` / `quic_connect_0rtt`（握手完成
  前即可 `quic_send`，tp 缺失自动降级 1-RTT）/ `quic_0rtt_rejected` /
  `quic_conn_handshake_done`；H3 0-RTT 子集 `h3_server_listen_stateless`（静态表，幂等 GET）
- **连接迁移**：`quic_migrate(conn, ip, port)` 新建 UDP fd 换源（NAT rebinding 语义）；
  `quic_conn_path` / `quic_conn_local` 观测对端/本地地址；server 完成新路径
  PATH_CHALLENGE 验证并跟随（同 conn 无重握手，echo 跨迁移续传）
- **BLOCKED_STREAMS 流上限协商**：`quic_set_max_client_streams(listener, n)` 经 transport
  params 下发对端；`quic_extend_max_streams(listener, add)` 发 MAX_STREAMS 放行；
  `quic_streams_left(conn)` 剩余配额；`quic_open_stream` 达配额返回 -206（原吞为 -1）
- bootstrap/pxi 重建：解释器同能力支持 M54 全部新内置（S5 全量回归 14 项端到端全 PASS）

### 新增 · M53 HTTP/3 三栈合一 WebServer（见 docs/M53_PLAN.md）

- `px_serve(port, docroot, timeout, {http3: true | {port?, cert?, key?}})` 单调用同时托管
  HTTP/1.1（TCP）+ HTTP/3（QUIC/UDP，缺省同端口），HTTP/1.1+HTTP/2+HTTP/3 共用同一
  vhost/路由/限流/访问日志/静态/.px 管道；Alt-Svc 自动通告；SIGTERM 优雅关闭
- 语言 API：`h3_server_listen(port, cert, key)`（HTTP/3 管道托管 listener）、
  `quic_h3_listen(port, cert, key)`（QUIC raw listener，多连接托管地基）
- QUIC 服务端多连接托管：单 fd 收包路由（DCID → 连接入包队列）+ 每连接处理线程 + PEM 证书加载
- 请求管道与传输解耦：`PxHttpOut` 输出抽象（HTTP/1.1 与 HTTP/3 共用，纯重构响应字节零变化）
- **HTTP/3 外部互操作打通**：aioquic（第三方独立实现）请求 PuXian H3 服务 200；修复 4 个
  协议级缺陷（fin-only 流活性、无 body 请求、响应头字段名小写、响应流 FIN）
- bootstrap/pxi 重建：解释器同能力支持 h3_server_listen / px_serve http3（S5 全量回归）

### 修复 · M55 并发安全（issue #2）

- 全局符号表/GC root 扫描互斥：px_set_global / px_get_global / struct 方法查找全程持锁、
  g_len 原子化、stop-the-world 持锁扫 root —— 修复并发写全局变量 + GC 竞态崩溃

## [0.1.0] - 2026

首个可公开版本：语言已自举（PuXian 编译器由 PuXian 自身编写），
工具链零 Rust 依赖（`tools/pxc` + 仓库自带自举二进制），静态二进制分发。

### 语言与工具链（自举核心）

- 语言：缩进语法（Python 脸）、渐进类型、`struct/enum/trait/impl`、
  `Option`/`Result` + `?`/`!`、`match`、推导式、生成器、闭包（无捕获面）、
  字符串插值 `${expr}`、管道 `|>`、可选链 `?.`、空合并 `??`
- 双后端：tree-walking 解释器（秒起）+ C 转译编译（`gcc` → 静态二进制），
  双模式行为一致（`selfhost/diffcheck.sh` 逐字节对拍）
- 自举闭环：`compiler.px` 编译自己 → `B.c == golden/compiler.c` 逐字节一致（三步证明）
- 工具：`tools/pxc`（build/run/lex/parse）、`tools/pxpkg`（包管理 + semver + lockfile）、
  `tools/routegen`（文件即路由生成器）
- Mini 子集规范（`docs/MINI_SUBSET.md`）：自举期间语言面锁定 + 已知限制清单

### 运行时与网络

- 内置：AES/XML/zip/gzip/正则/base64/JSON（含 JSONPath）/SQLite/协程/事件总线/沙箱
- WebServer 生产化：HTTP/1.1 服务端 + 路由/中间件/限流/访问日志/虚拟主机（SNI）/
  静态文件/Range/ETag/Session/Cookie/优雅关闭/进程池热更新
- HTTP/2：h2c + HPACK/Huffman + 多流 + over-TLS（ALPN h2）
- WebSocket / SSE（含自动重连、心跳、广播）、UDP、S3/MinIO（SigV4）
- HTTP/3 / QUIC（RFC 9114/9000/9204 预研落地）：
  - M46 QUIC 传输层（ngtcp2 + quictls 静态编译，握手/双向流/单向流）
  - M47 HTTP/3 语义层（QPACK 无动态表 codec + HEADERS/DATA 帧 + 请求/响应对拍）
  - M48 QPACK 完整 codec（RFC Huffman + 静态表 99 项字节精确）
  - M49 QPACK 动态表 + SETTINGS 控制帧（RFC 9204 全量会话）
  - M50 多路复用（单连接多双向流并发请求/响应，per-stream 缓冲）
  - M51 QPACK 会话上真实 QUIC 单向流（控制/编码器/解码器三流 + SETTINGS 协商）
  - M52 QPACK 解码器流 ack 闭环（Section Ack / KRC 推进 / 编码表驱逐安全化）

### 生态

- 仓库外生产应用：用 PuXian 写的第一个生产应用（HTTP + SQLite），dogfooding 验证（代码维护于独立私库）
- 80+ 示例程序 + 里程碑验证脚本（`examples/`）

[Unreleased]: https://github.com/NanzhanGroup/PuXian
[0.1.0]: https://github.com/NanzhanGroup/PuXian
