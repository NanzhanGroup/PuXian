# M57 · 边缘设备层支持（Linux 用户态）

> 目标：把 PuXian 的使用域从「网络 / Web / 数据处理」扩展到**物理世界接口**——
> Linux 边缘设备层（树莓派 / 网关 / 盒子）上的 i2c-dev / spi-dev / gpio / tty /
> 网卡 / 帧缓冲控制。与清歌嵌入式讨论结论一致：runtime 含 GC / 线程 / 动态值，
> 编译模式 = 生成 C → gcc，只适合 **Linux 用户态边缘设备层**；裸机 MCU 明确不做。
> 相比给暂无真实用户的 H3 栈打磨（1→1.01），设备层是 **0→1 开新使用域**；静态
> 二进制免环境 + 生成 C→gcc 交叉编译是边缘差异化卖点。
>
> 范围：仅 PuXian 语言 / runtime / 工具链侧（ws-web 归清歌，不在本里程碑）。
> 验证基调：无真板子环境 → ioctl/mmap 语义用 **TCP fd + x86 ioctl mock** 验证，
> 真实设备（GPIO/I2C）条件性探测（存在则 open 验证），S4 用 qemu-aarch64 跑交叉产物。

## 一、现状（调研结论）

| 能力 | 现状 | 缺口（M57 待补） |
|---|---|---|
| 文件 IO | 路径式（read_file / read_at / write_at / fsync_file / truncate_file，内部 open 用完即关） | **语言面无持久 fd 句柄** → 无法持有设备文件描述符做 ioctl |
| 设备控制 | 无（grep ioctl runtime/runtime.c 零命中） | ioctl 是 Linux 用户态控外设的主入口：i2c-dev / spi-dev / gpio(老 ioctl) / tty / 网卡 全走它 |
| mmap | 仅 GC 分配器内部（slab + 大对象兜底，全 MAP_PRIVATE\|ANONYMOUS） | 未暴露语言层；无文件/设备映射（帧缓冲、共享内存、DMA） |
| 交叉编译 | 编译模式 = 生成 C → gcc；runtime 绑 mbedtls/sqlite3/openssl（平台相关） | 无交叉工具链流程；无 runtime 裁剪开关 |
| FFI | M42 `import "c/xxx"` + `extern def` + ffi_call 桥已通 | 逐库绑定够用；通用动态 FFI（dlsym）等真需求再上（明确不做） |

## 二、架构决策

- **D1 · fd 原语先于 ioctl（S1，修正清歌假设）**
  清歌方案基于「open/read/write 若 runtime 文件 IO 已覆盖，缺的只是 ioctl」——
  但事实是 runtime 文件 IO 为**路径式**，语言面**没有 open() 拿 fd** 的内建，
  即便加了 ioctl 也无 fd 可用（socket fd 除外）。故 S1 一并补：
  - `open(path[, mode]) → fd`（mode: r/w/a/rw/w+，映射 O_RDONLY/O_WRONLY|O_CREAT|O_TRUNC/
    O_APPEND/O_RDWR；失败 -1）
  - `close(fd) → bool`
  - `ioctl(fd, request[, arg]) → int`（arg 三形态：缺省/null → NULL；int → 整数值直传；
    bytes/str → **就地 in/out 缓冲区**，调用后同一对象内容被内核更新，bytes_to_int 读回）
  - `os_errno() → int`（线程局部 errno 查询；open/ioctl 失败路径可编程判断）
  四个全部走 px_set_global runtime builtin（M56 http_unix 同款），编译模式语言直接调用；
  解释模式（pxi）支持在 S5 重建时并入（interp.px 白名单 + ibuiltin.px 转发，本步不碰自举链）。
  验证：ioctl 语义在**真实 TCP fd** 上验证（FIONREAD 数值 + bytes 就地填充 5/0、
  FIONBIO 设置生效非阻塞 recv 立即返回、fd 类型不匹配/非法 fd 的 -1+errno、
  NULL/int/bytes 三形态正确传递）；open/close 设备文件通路（/dev/null）+ 失败 errno；
  真实设备（/dev/gpiochip*、/dev/i2c-*）条件性探测。request 码为 32 位 _IOC 编码
  （语言用 0x 字面量，如 FIONREAD=0x541B）。
- **D2 · mmap/munmap 语言内建（S2）**
  映射 fd（帧缓冲 /dev/fb0、共享内存、DMA 缓冲）→ bytes 视图；配合 ioctl 完成设备读写。
- **D3 · 设备示例 + mock（S3）**
  GPIO 老接口 / I2C dev 读写示例；无板子用 x86 ioctl mock（LD_PRELOAD 或 /dev 伪设备）
  验胶水语义。
- **D4 · 交叉编译工具链（S4）**
  本地 aarch64 交叉（gcc-aarch64 或 zig cc）→ qemu-aarch64 跑静态产物；
  runtime 裁剪开关（--no-ssl 等，解开 mbedtls/sqlite3/openssl 平台依赖）。
- **D5 · 收口（S5）**：pxi 重建（ioctl/open/close/os_errno + 既有 http_unix 一并进
  解释器）+ capability 双模式 + diffcheck + 自举证明 + 全量回归。
- **D6 · 文档（S6）**：spec §8.17（M57 设备层 API 文档）+ ROADMAP + CHANGELOG +
  README/README.en。

## 三、子步划分（每步可独立回归，逐步 commit）

| 子步 | 内容 | 验证 |
|---|---|---|
| S1 ✅ | D1 fd 原语（open/close/ioctl/os_errno） | commit `88e824b`；examples/m57_s1_ioctl_verify.sh 全 PASS（A open/close+errno / B TCP fd FIONREAD=5/0+就地填充、FIONBIO 生效、ENOTTY/EBADF、NULL/int/bytes 三形态 / C 真实设备条件探测）；m53_s1_h3echo 8 并发回归 PASS；capability 全量因宿主负载 killed 留 S5 |
| S2 | D2 mmap/munmap 语言内建（fd→bytes 视图） | 待定 |
| S3 | D3 GPIO / I2C 示例 + x86 ioctl mock | 待定 |
| S4 | D4 aarch64 交叉 + qemu + runtime 裁剪开关 | 待定 |
| S5 | D5 pxi 重建 + capability/diffcheck/自举/全量回归 | 待定 |
| S6 | D6 文档收尾 | 待定 |

## 四、风险与边界

- S1–S4 验证跑**编译模式**（pxc build，同 M53-S3/S4、M54-S1–S4 策略）；pxi 解释器
  对新 builtin 的支持在 S5 重建时统一并入（本里程碑不中途重链自举二进制）。
- ioctl request 码为内核 ABI 常量（跨 arch 可能不同：x86_64 与 aarch64 的
  asm-generic 大部分一致，特殊码按目标内核头文件适配，语言不内置常量表）。
- ioctl 就地填充要求语言侧 buffer 长度 ≥ 驱动写回长度（S1 用 int_to_bytes(n,4)
  造 4B buffer 验 FIONREAD；大 buffer 场景随 S2 造 bytes 工具一并解决）。
- 真实设备（GPIO/I2C）在 CI/容器不可用 → S1 条件探测（存在才 open），语义正确性
  以 TCP fd（内核真实 ioctl 路径）为准，真实设备 demo 留 S3 板子环境。
- 交叉编译需先解开 runtime 平台绑定（S4），否则 mbedtls/sqlite3/openssl 在
  aarch64 目标同样要交叉编译；S4 的 --no-ssl 裁剪开关是前提而非可选项。
- 全部新 API 保持「旧行为零变化」：未用新 builtin 的 m46–m56 脚本不受影响。
