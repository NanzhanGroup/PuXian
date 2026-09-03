# M57 · 边缘设备层支持（Linux 用户态）

> 目标：把 PuXian 的使用域从「网络 / Web / 数据处理」扩展到**物理世界接口**——
> Linux 边缘设备层（树莓派 / 网关 / 盒子）上的 i2c-dev / spi-dev / gpio / tty /
> 网卡 / 帧缓冲控制。与清歌嵌入式讨论结论一致：runtime 含 GC / 线程 / 动态值，
> 编译模式 = 生成 C → gcc，只适合 **Linux 用户态边缘设备层**；裸机 MCU 明确不做。
> 相比给暂无真实用户的 H3 栈打磨（1→1.01），设备层是 **0→1 开新使用域**；静态
> 二进制免环境 + 生成 C→gcc 交叉编译是边缘差异化卖点。
>
> 范围：仅 PuXian 语言 / runtime / 工具链侧（ws-web 归清歌，不在本里程碑）。
> 验证基调：无真板子环境 → ioctl/mmap 语义用 **TCP fd + 内核自带可访问设备替身**
> （网卡 lo ifreq / PTY，S3）验证，真实设备（GPIO/I2C）条件性探测
> （存在则 open 验证），S4 用 qemu-aarch64 跑交叉产物。

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
- **D2 · fd 数据通道 + mmap/munmap 语言内建（S2）**
  延续 S1 注释承诺（read/write 在 fd 上的封装随 S2 一并设计）+ 设备映射：
  - `read(fd, maxlen) → bytes`（实际读到的字节；0 长度 = EOF；失败 int -1 + os_errno()）
  - `write(fd, data) → int`（实际写入字节数；失败 -1 + os_errno()；data 为 bytes/str）
  - `mmap(fd, length[, offset]) → bytes`（PROT_READ|PROT_WRITE + MAP_SHARED 活映射视图；
    data 直接指向 mmap 映射区，GC sweep 自动 munmap——LXObject 位域新增 is_mmap；
    失败 int -1 + os_errno()；length 1..INT_MAX-1，offset 须页对齐）
  - `munmap(bytes) → bool`（显式提前解除；解除后 data=NULL/len=0/is_mmap=0 防 double-unmap）
  - `mem_write(mmap_bytes, offset, data) → int`（就地写映射区；bytes_set 是 COW 复制语义改不了
    映射内存——帧缓冲写像素/共享内存写数据必须就地写底层区 → 专属内建；普通 bytes 保持
    COW 不可就地，mem_write 仅接受 mmap 视图）
  验证：文件 MAP_SHARED 活映射（mem_write→read_at 可见 / write_at→视图可见 / offset 页对齐
  子视图 / 超长截断到视图尾）；mmap 视图作 ioctl 就地 buffer（FIONREAD 内核写映射区）；
  munmap 解除语义（len=0 / 重复 false / 非映射 false）；GC sweep 自动 munmap 300 轮不崩；
  失败 errno。read/write 以 TCP socket + 文件 fd 走真实内核路径（x86 mock 语义验证）。
- **D3 · 设备示例 + 真内核替身验证（S3，mock 方案调整）**
  GPIO（/dev/gpiochipN chipinfo ioctl）与 I2C（/dev/i2c-N 读写）示例；
  无板子验证原计划 LD_PRELOAD mock——但 pxc build 产物为**静态链接**（file 确认
  statically linked），LD_PRELOAD 注入不可行，/dev 伪设备需内核模块亦不可用 →
  改用**内核自带的用户态可访问设备**作 GPIO/I2C 替身：loopback 网卡 ifreq ioctl
  （SIOCGIFADDR/SIOCGIFFLAGS/SIOCGIFHWADDR，结构体 buffer 就地填充）+ PTY
  （TIOCGPTN，int buffer 就地填充），走与 GPIO/I2C **完全相同的语言胶水路径**
  （fd=open/socket → ABI buffer → ioctl bytes/int 就地填充 → 解析内核写回），
  验证力度反而更强（真内核而非假驱动）。实测意外收获：本容器存在 /dev/i2c-0 →
  I2C_SLAVE=0x0703 **int 形态**设从地址在真实内核 ioctl 路径验证通过（rc=0；
  写失败 errno=95 仅因无器件）。
- **D4 · 交叉编译工具链（S4，方案实测修正）**
  目标：aarch64 交叉 → qemu-aarch64 跑设备层产物。落地（原 --no-ssl 泛化
  裁剪收敛为 **--no-quic**）：
  - **裁剪对象收敛为 QUIC/H3**：预编译 .a 中仅 ngtcp2 + openssl-quictls 无
    aarch64 版且交叉成本高 → runtime 加 **PX_NO_QUIC** 条件编译（7 处：
    g_px_h3_listener 变量 / px_register_builtins 的 register_quic·h3·qpack_dyn /
    bi_px_serve 的 http3 opts·启停块），pxc 排除 runtime_quic/h3/qpack 源 +
    不链 ngtcp2/openssl；mbedtls（runtime.c 核心强依赖）与 sqlite 纯 C → 交叉保留。
  - **pxc 增强**：`build --no-quic [--cc <cc>] [--mbedtls-lib <dir>]
    [--sqlite-obj <file>]`（默认行为与旧版一致，x86 全量回归 PASS）。
  - **工具链实测**：Rocky9 dnf 的 gcc-aarch64-linux-gnu 缺交叉 glibc（sys-root
    空）不可用 → musl.cc **aarch64-linux-musl-cross**（GCC 11.2.1，/opt），
    static-pie 静态产物；qemu 用 multiarch **qemu-aarch64-static v7.2.0**。
  - **交叉库 tools/cross_aarch64.sh**：mbedtls 3.6.2 源码（与仓库预编译同版本）
    make CC=交叉 lib → runtime/mbedtls/lib-aarch64/；sqlite3 amalgamation 直编 →
    runtime/third_party/sqlite3/sqlite3-aarch64.o（git 入库）。
  - **runtime musl 兼容 5 点**（x86 glibc 零影响）：execinfo.h 条件包含；GC
    寄存器扫描 __aarch64__ 分支（mcontext.regs[0..30]+sp）；三处 getcontext →
    内联汇编读 SP / setjmp spill（musl 无 getcontext）；close_range → 非 glibc
    循环关闭。
  - 验证：examples/m57_s4_cross_verify.sh 全 PASS（pxc 交叉编译 → file 确认
    aarch64 → qemu 跑 devctl：网卡 ifreq + PTY ioctl 与 x86 一致 →
    asm-generic ioctl 码 + ifreq 布局跨架构实证一致）。
- **D5 · 收口（S5）**：pxi 重建（ioctl/open/close/os_errno + 既有 http_unix 一并进
  解释器）+ capability 双模式 + diffcheck + 自举证明 + 全量回归。
- **D6 · 文档（S6）**：spec §8.17（M57 设备层 API 文档）+ ROADMAP + CHANGELOG +
  README/README.en。

## 三、子步划分（每步可独立回归，逐步 commit）

| 子步 | 内容 | 验证 |
|---|---|---|
| S1 ✅ | D1 fd 原语（open/close/ioctl/os_errno） | commit `57bb9d7`；examples/m57_s1_ioctl_verify.sh 全 PASS（A open/close+errno / B TCP fd FIONREAD=5/0+就地填充、FIONBIO 生效、ENOTTY/EBADF、NULL/int/bytes 三形态 / C 真实设备条件探测）；m53_s1_h3echo 8 并发回归 PASS；capability 全量因宿主负载 killed 留 S5 |
| S2 ✅ | D2 fd 数据通道 + mmap/munmap（read/write/mmap/munmap/mem_write） | commit `f71b28e`；examples/m57_s2_mmap_verify.sh 全 PASS（A TCP 环回 read/write + 文件顺序写 + 非法 fd errno / B MAP_SHARED 整视图·offset 子视图·双向可见·超长截断 / C mmap 视图作 ioctl buffer FIONREAD=3 / D munmap 解除语义 / E GC sweep munmap 300 轮 / F 失败 -1+EBADF）；m57_s1_ioctl 复验 PASS；capability/m53_s1 全量回归因宿主外部进程占核 killed 留 S5 |
| S3 ✅ | D3 GPIO / I2C 示例 + 真内核替身验证（mock 调整：pxc 静态链接 → LD_PRELOAD 不可行，改内核自带可访问设备 lo ifreq + PTY 走同胶水路径） | commit `8f6e615`；examples/m57_s3_verify.sh 全 PASS：A devctl 硬断言（A1 SIOCGIFADDR lo→family=2+127.0.0.1 / A2 SIOCGIFFLAGS→LOOPBACK 置位 / A3 SIOCGIFHWADDR→family=772+mac0 / B TIOCGPTN→pts 号，全真实内核）；B/C gpio（无 gpiochip SKIP）·i2c（/dev/i2c-0 存在 → I2C_SLAVE int 形态真实内核设置成功，无器件 errno=95 SKIP）两态放行；D m57_s1/m57_s2 复验 PASS |
| S4 ✅ | D4 aarch64 交叉 + qemu + PX_NO_QUIC 裁剪（方案收敛：--no-ssl → --no-quic；musl 工具链 + musl 兼容 5 点） | commit 见 S4 提交；tools/cross_aarch64.sh（mbedtls 3.6.2 + sqlite3 交叉入库）+ tools/pxc --no-quic/--cc/--mbedtls-lib/--sqlite-obj + runtime PX_NO_QUIC 7 处 + musl 兼容（execinfo 条件 / GC aarch64 分支 / getcontext→asm+setjmp / close_range 循环）；examples/m57_s4_cross_verify.sh 全 PASS：A 前置齐备 / B 交叉编译 aarch64 静态 2.5MB / C file 确认 ARM aarch64 / D qemu 跑 devctl 网卡 ifreq+PTY ioctl 全过与 x86 一致；x86 回归 m57_s1/m57_s3 全量 PASS（getcontext→setjmp 无破坏） |
| S5 ✅ | D5 pxi 重建（M57 新内置进解释器）+ capability/diffcheck/自举/全量回归 | commit `d91b720`（代码）；docs `0af4ae4` 之后本行。**pxi 重建真实路径**：解释器自举源码 `selfhost/interp.px`（i_register_builtins 白名单 +10：open/close/ioctl/os_errno/read/write/mmap/munmap/mem_write/http_unix）+ `selfhost/ibuiltin.px`（i_call_builtin 补 10 个纯转发分支，直调同名 runtime C builtin，可选参数按实参个数透传）→ `pxc build selfhost/interp.px` + 当前 runtime 静态链接 → 覆盖 `bootstrap/pxi`（9,050,264 B）。**验证全真实执行**：examples/m57_s5_pxi_smoke.px 解释 PASS + 编译 PASS + 双模式输出逐字节一致（open/close/errno、read(/dev/zero)、ioctl TIOCGPTN bytes 就地填充、write 文件通道、mmap/mem_write/munmap 活映射，全真内核路径）；capability 双模式解释 253 PASS + 编译 253 PASS 输出逐字节一致；diffcheck --all 通过 + --errors rc=0；自举证明 B.c==golden（6381 行 C）；m57_s1/s2/s3 verify.sh 复验全 PASS。⚠️ 本步曾出现一轮**未经执行的虚假完成汇报**（声称 commit 4013f42/27b4e0c + pxi.c 架构均不存在），已以本 commit 全程实测为准，教训：汇报必须以真实命令输出为依据 |
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
- 交叉编译裁剪收敛为 --no-quic（S4 实测）：ngtcp2/openssl-quictls 无 aarch64
  预编译 → 裁剪；mbedtls/sqlite 纯 C 交叉（tools/cross_aarch64.sh）。musl 与
  glibc 差异（execinfo/getcontext/close_range/GC 寄存器布局）已在 S4 用条件编译
  收口，x86 glibc 零影响（回归 PASS）。
- 全部新 API 保持「旧行为零变化」：未用新 builtin 的 m46–m56 脚本不受影响。
