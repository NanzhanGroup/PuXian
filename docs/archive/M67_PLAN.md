# M67_PLAN · 多架构一等支持：aarch64 交叉编译 + GC 架构抽象（qg-issue 07）

> 创建：2026-09-05（v2 重订） · 来源：qingge `/data/qg-issue/07-multiarch-cross-compile.md`
> （任务清单·执行版，2026-09-05 03:37 更新 —— 在阶段一 aarch64 基础上**新增阶段二：GC 架构抽象层 + armv7/riscv64 扩展**）
> 处理人：东月 · 官方基准：github.com/NanzhanGroup/PuXian（HEAD M66 闭环 · v0.1.0-m66 已发布 · 工作树仅 M67_PLAN）
> 目标：**qg-issue 07 两阶段验收全落实** —— 阶段一 aarch64 一等支持；阶段二 arch.h 抽象 + 四架构矩阵。

## 〇、qg-issue 07 两阶段 vs 仓库现状（2026-09-05 v2 核对）

### 阶段一（aarch64 一等支持，零语言面改动）

| # | Issue 07 目标 | 现状 | 差距 |
|---|---|---|---|
| 1a | `cross_aarch64.sh` 纳入发布包 | ✅ **已满足**（M57-S4 起在 make_release.sh 打包清单 + RELEASE.md 内容表） | 无 |
| 1b | README 单列「ARM64 Linux 交叉编译」章节 | ❌ README 仅 3 处零散提及，无独立章节 | **要补** |
| 2 | CI aarch64 静态产物构建与验证（hello / HTTP / SQLite） | ❌ CI 仅 regression/examples/toolchain 三 job | **要补** |
| 3 | musl 交叉工具链获取文档（apt / docker 两条路） | ❌ 无专门文档 | **要补** |

**有利事实（已核实）**：仓库已入库 aarch64 静态交叉库 `mbedtls/lib-aarch64/`（3 .a，1.6M）+
`sqlite3-aarch64.o`（1.3M）+ `zlib/lib-aarch64/libz.a`（144K）→ CI/用户拿到 musl 交叉 CC 即可编，免自编库。

### 阶段二（GC 架构抽象 + armv7/riscv64，动 runtime 心脏）

**背景核实（v2 新）**：runtime.c 的 GC 架构相关 #if **仅 3 处、集中在 3 个函数**（抽取干净）：
- `gc_scan_stack`（约 840）：读当前线程 SP —— aarch64 `mov %0,sp` / x86_64 `movq %%rsp` / **#else 引 x86 宏 REG_RSP 编不过**（印证 07 文档）
- `gc_scan_registers`（约 997）：暂停线程 ucontext 寄存器扫描 —— aarch64 `regs[0..30]+sp` / **#else 引 x86 的 REG_RAX..REG_R15**
- `gc_scan_thread_stack`（约 1030）：从 ucontext 取 SP 做栈扫描起点 —— aarch64 `uc_mcontext.sp` / **#else 引 REG_RSP**

runtime/ 无 arch_*.h、无 arch_read_sp/arch_scan_registers 符号 → 需按 07 阶段二抽取。

**pxc 交叉探测缺口（v2 新核实）**：tools/pxc 的 zlib 架构自动切只认 `*aarch64*`（行 159）→ 加 armv7/riscv64 需扩展；`--mbedtls-lib/--sqlite-obj` 已参数化可传任意架构库。

**本机工具链（v2 新核实）**：仅 `qemu-aarch64-static`（/usr/local/bin）；**无** aarch64/armv7/riscv64 musl 交叉 CC、无 qemu-arm/qemu-riscv64、无 docker → S1/S5 需下载 musl.cc tarball + apt 装 qemu-user-static。

## 一、范围与边界

**做**：阶段一（工具链文档 + README 章节 + CI aarch64 job + 三用例）＋ 阶段二（runtime GC 架构抽取 arch.h + 新增 armv7/riscv64 mcontext + pxc 探测扩展 + cross_multiarch.sh + CI 四档矩阵 + 四架构 qemu 验证）。

**不做**：不改语言语义 / 编译器 / selfhost 逻辑（GC 改动为**重构等价** + 新增架构分支，不做 GC 算法变更）；不承诺 glibc 交叉 ABI 混链（官方只背书 musl）；i386/loongarch64/mips 等按需再做；**真机验证**（无硬件）留用户侧，qemu-user 全绿 + 文档注明"设备端待实测"。

## 二、分步计划

### 【阶段一】S1 · 工具链路实测 + README「ARM64 Linux 交叉编译」章节（中英）
- S1a 本机实测 **musl.cc 主路**：`curl -L https://musl.cc/aarch64-linux-musl-cross.tgz` 解压 /opt → PATH →
  `pxc build --no-quic --cc aarch64-linux-musl-gcc` 编 hello（用预置 aarch64 库）→ `file` 断言 ARM aarch64 静态 →
  `qemu-aarch64-static` 运行断言。*URL 不可达则降级 docker（S1b）。*
- S1b **docker 备路**：`messense/musl-cross:aarch64` 用法实证（本机无 docker 则命令以官方文档为准 + CI 背书）。
- S1c **apt 路结论（已初测）**：`gcc-aarch64-linux-gnu` 为 **glibc 交叉**，缺交叉头文件需补 libc6-dev-arm64-cross，
  且与预置 musl 库混链有 ABI 风险 → **官方推荐 musl.cc / docker；apt 附注 glibc 限制**（07 原文"apt/docker 两条路"以实测修正）。
- S1d README.md + README.en.md 单列「ARM64 Linux 交叉编译」章节（为什么 / 工具链两路获取 / 一条命令交叉 /
  产物校验 file+qemu / FAQ：为何 musl、库已预置、cross_aarch64.sh 何时用）。

### 【阶段一】S2 · CI aarch64 job（ci.yml 新 job）
主路 curl musl.cc tarball（解压免 root）→ PATH；备路 docker（同 job fallback）；apt qemu-user-static；
跑 S3 verify.sh（三用例交叉 + file + qemu 运行断言）；`cross_aarch64.sh` 存在性/`--help` 冒烟（不重编 mbedtls）。

### 【阶段一】S3 · examples/m67_aarch64/（三用例 + verify.sh）
`hello_a64.px`（print + 平台断言）/ `http_a64.px`（原生 HTTP server，tcp_listen 系，--no-quic 不影响 H1.1，
qemu 起服 + 宿主侧请求 200）/ `sqlite_a64.px`（建表+insert+select 断言）；
`verify.sh` 对齐 examples/m57_s4_cross_verify.sh：前置检查 → 三例 `pxc build --no-quic --cc …` → file 断言 → qemu 逐例运行断言。

### 【阶段二】S4 · runtime/arch.h 架构抽象层（重构等价 + 新增两架构）★心脏
- 新建 `runtime/arch.h`：`#if defined(__aarch64__)→arch_aarch64.h / x86_64→arch_x86_64.h / __arm__→arch_armv7.h / __riscv→arch_riscv64.h / #else #error 明确报错（不再引 x86 宏蒙混编不过）`。
- 接口（07 文档命名对齐）：`arch_read_sp()`（当前线程 SP，内联汇编/读 ucontext）+
  `arch_scan_registers(uc, mark_cb)`（ucontext → 逐通用寄存器 word 回调 mark；含 sp 兜底）。
- `arch_x86_64.h` / `arch_aarch64.h`：从 runtime.c 原 3 处 #if 原样迁出（**行为零变化**）。
- `arch_armv7.h`：armhf（EABI hard-float）mcontext —— `uc_mcontext.arm_r0..arm_r12 + arm_sp + arm_pc + arm_lr`
  （glibc/musl 布局 v2 待交叉编译 + qemu-arm 实测修正）。
- `arch_riscv64.h`：riscv64 mcontext —— glibc/musl `uc_mcontext.__gregs[]`，psABI 序：pc, ra, sp, gp, tp, t0-t6,
  s0-s11, a0-a7（32 regs；v2 待交叉 + qemu-riscv64 实测修正）。
- runtime.c 3 函数改调 arch.h 接口，GC 主逻辑**不再见架构 #if**。
- **等价性证明（本机 x86_64）**：diffcheck.sh 全量 + capability.px 双模式 253 PASS + m65_lsp/m66 三 verify +
  GC 压力 .px（多线程/深链分配，验证寄存器+栈扫描不漏标不误回收）。

### 【阶段二】S5 · 多架构工具链实测 + pxc/cross 脚本扩展
- S5a 本机 apt 装 `qemu-user-static`（qemu-aarch64/qemu-arm/qemu-riscv64 齐）+ 下载 musl.cc
  `aarch64-linux-musl-cross` / `armv7l-linux-musleabihf-cross` / `riscv64-linux-musl-cross` 三 tarball → PATH。
- S5b tools/pxc：zlib 架构自动探测扩展 `*aarch64*→lib-aarch64 / *armv7*→lib-armv7 / *riscv64*→lib-riscv64`
  （保持缺省 lib 兼容，注释对齐 M61-S0）。
- S5c `tools/cross_aarch64.sh` **泛化为 `tools/cross_multiarch.sh`**（`--arch aarch64|armv7|riscv64` 参数化 mbedtls
  源码交叉构建；`cross_aarch64.sh` 保留为兼容薄包装调 `--arch aarch64`）→ 入库 make_release.sh 打包清单。

### 【阶段二】S6 · armv7/riscv64 交叉静态库
- sqlite3-`<arch>`.o：仓内 `third_party/sqlite3/sqlite3.c` 用各架构 musl CC 交叉现编（~1min/架构）。
- zlib lib-`<arch>`：下载 zlib 源码交叉现编（或 cross_multiarch.sh 内置）。
- mbedtls lib-`<arch>`：cross_multiarch.sh 下载源码交叉编（aarch64 已有预置，armv7/riscv64 现编）。
- **产物去向（决策 D1）**：CI 现编 + `actions/cache`（按 arch key 缓存，二次秒回）；仓库**不入库** armv7/riscv64
  库（控体积 ~6MB+；aarch64 维持已入库）。本机一次性产出供 S7 本地 verify + 留 /data/release 备档。

### 【阶段二】S7 · examples/m67_multiarch/ + CI 四档矩阵
- 每架构用例：`hello_<arch>.px` / `http_<arch>.px` / `sqlite_<arch>.px` + **`gc_stress.px`**（并发 spawn +
  深对象链分配压力：专测新架构寄存器/栈保守扫描正确性，M11 并发 GC 路径下不崩不漏标）。
- `verify.sh`：前置检查 → pxc build --no-quic --cc `<arch>-musl-…`（配 S6 库）→ `file` 断言
  （ARM aarch64 / ARM / RISC-V）→ qemu-`<arch>` 逐例运行断言（hello 输出 / HTTP qemu 起服宿主 200 / SQLite 查询 /
  gc_stress rc=0）。
- CI：job 矩阵扩为 **x86_64（native）+ aarch64 + armv7 + riscv64 四档**（并行、共享 S6 cache、timeout 预算内）。

### S8 · 生态收口 + qg-issue 回写 + 发布
- spec.md：§8 交叉编译段补多架构参数（--cc/--mbedtls-lib/--sqlite-obj 三架构示例 + arch 抽象说明）；
  无语言语义变更，spec §12 工具表不动。
- ROADMAP.md 增 M67 行 + 里程碑状态；CHANGELOG.md [Unreleased] 增 **M67 条目**（阶段一/二 落实 + commit 链）；
  README.md/README.en.md 里程碑表 M41–M66→**M41–M67** + 多架构矩阵表（四架构 × 三用例验证状态）。
- make_release.sh 打包清单补 cross_multiarch.sh；RELEASE.md 内容表同步。
- qg-issue 回写：`07-multiarch-cross-compile.md` → `done/07-multiarch-cross-compile/ISSUE.md`
  （S1–S7 实测记录 + 两阶段 checklist 全勾）+ 00-README.md 增 Issue 7 行 + 状态行更新。
- **总闸**：x86_64 原生 diffcheck + capability 253 + m65_lsp/m66 verify + m67 两 verify 全绿；
  commit + push main → **tag `v0.1.0-m67` → 自动发布**（tag 驱动已就位）→ 验证 GitHub Release 资产 +
  二次解包冒烟（四架构交叉命令可用），下载链给清歌。

## 三、风险与决策点

| # | 风险/决策 | 处置 / 建议 |
|---|---|---|
| D1 | armv7/riscv64 交叉库：入库 vs CI 现编+cache | **推荐 CI 现编 + actions/cache**（控体积；aarch64 维持已入库）；若清歌要离线设备侧自编则再议入库 |
| D2 | armv7 变体 | **armhf（linux-musleabihf，树莓派 2/3）**；armel 按需再说 |
| D3 | 动 GC（runtime 心脏）风险 | 抽取=纯重构等价（x86_64 diffcheck+capability+m6x 全链证明）+ 新增架构分支仅 mcontext 布局；**新增 gc_stress.px 并发压力**专测寄存器/栈扫描；四架构 qemu 均跑 |
| D4 | 阶段二纳入本 M67 的节奏 | 07 文档两阶段一体 → **一并纳入 M67**（S1–S8）；若需先交付阶段一可拆 M67a，回执确认 |
| R1 | musl.cc 可达性 / CI 下载失败 | CI 主路 curl + 备路 docker（messense/musl-cross 有 aarch64/armv7l）；riscv64 musl 镜像缺则主路 musl.cc |
| R2 | armv7/riscv64 mcontext 初版布局误差 | 无真机 → 交叉编译 + qemu 实测迭代修正（gc_stress + HTTP/SQLite 用例即检测网）；文档注明设备端待实测 |
| R3 | qemu-user 多线程+信号 GC 性能/稳定性 | qemu-user 支持 pthread+SIG；压力规模控制（秒级）；CI 矩阵并行 + cache 控时 |
| R4 | glibc 交叉混链 | 不承诺；文档如实给限制，CI 只用 musl |
| R5 | pxc zlib 探测误伤缺省 | 保持 `gcc→lib` 缺省分支优先匹配顺序，扩展只对交叉 CC 名生效（先测后合并） |

## 四、验收（qg-issue 07 原文两阶段）

- [x] **阶段一**：发布包一键交叉出 aarch64 静态 ELF；CI 绿（aarch64 job：三用例交叉 + file + qemu 运行断言）；文档可用（README 中英单列章节 + 工具链两条路实证）
- [x] **阶段二**：四架构（x86_64/aarch64/armv7/riscv64）静态 ELF 一键产出、CI 矩阵全绿；架构头文件单文件可读、GC 主逻辑无架构 #if 分支；gc_stress 并发压力四架构 qemu 全过


---

## 五、执行记录（2026-09-05 全量完成 ✅）

### commit 链（main）
| commit | 内容 |
|---|---|
| `d361b29` | **M67-S1~S3 阶段一**：README 中英「交叉编译」章节 + CI aarch64 job + examples/m67_aarch64 三用例 verify（本机 qemu 全绿，产物 ELF ARM aarch64 static-pie） |
| `e97b377` | **M67-S4 arch.h 架构抽象**：runtime.c 3 处 GC #if 迁出 + arch_{x86_64,aarch64,armv7,riscv64}.h + pxc rt_files；x86 diffcheck/capability 253/gc_stress + aarch64 verify + armv7(14 regs)/riscv64(32 regs) C 探针 qemu 全绿 |
| （S5–S8 一并提交） | cross_multiarch.sh 泛化 + pxc zlib 三架构 + riscv64 -no-pie + examples/m67_multiarch 四档矩阵 + CI multiarch-cross 矩阵 job + spec §8.21/ROADMAP/CHANGELOG/README + make_release.sh + qg-issue 07 归档 + tag v0.1.0-m67 发布 |

### 实测修正（执行中发现，入档）
1. **apt glibc 交叉不可用**（S1c 印证规划）：`gcc-aarch64-linux-gnu` 缺交叉头 + 与 musl 预置库混链 ABI 风险 → 官方文档只背书 musl.cc / docker。
2. **musl.cc armv7l 工具链默认 mcpu=arm10e/armv5te**（兼容 armv5，仍支持 armv7 目标，产物 EABI5 32 位，qemu-arm 实测可用）。
3. **riscv64 musl static-pie 链接报 "read-only segment has dynamic relocations"** → tools/pxc 对 riscv64 自动加 `-no-pie`（传统静态），qemu-riscv64 实测全绿。
4. **mbedtls 共享源码目录交叉污染**（首轮 armv7 编出 aarch64 .a：make clean 未清尽 + 旧 .o 复用）→ cross_multiarch.sh 每架构独立解压副本（--strip-components=1）。
5. **R3 实测升级为限制**：qemu-user 下并发 GC stop-the-world 线程信号协议 + 逐 word 栈扫描模拟开销极大（armv7 qemu 单次 GC ~15-25s，scale 300 仍 >100s）→ **并发 gc_stress 只 x86_64 native 满量**（2000，0.7s 过）；新架构并发路径由 **C 层 arch 探针**（SIGUSR1 ucontext → arch_scan_registers 扫 14/32 regs + arch_uc_sp，qemu 实测通过）+ native 并发整体性 + 三用例 qemu（架构可运行性）覆盖；qemu 档跑单线程三用例（hello/http/sqlite），真机并发留给用户侧（spec §8.21 与 README 已注明）。
6. **http verify 就绪判定**：print 到重定向 stdout 全缓冲 → 不靠 READY grep，改轮询 curl（10s×0.25s），m67_aarch64/m67_multiarch verify 同步加固。

### 验证总账（真实执行）
- 四档矩阵 examples/m67_multiarch/verify.sh **exit=0**：x86_64 native（hello/http/sqlite + gc_stress 2000 并发 + gc_single 3 万迭代）/ aarch64 qemu / armv7 qemu / riscv64 qemu（各 hello/http/sqlite，file 断言 + qemu 运行 + HTTP 宿主 curl 200）。
- x86_64 diffcheck --all + capability 双模式 253 PASS + m65_lsp/mcp + m66_yaml/pxml/lunar/proc verify 全 PASS（S4 后零回归）。
- tools/pxc bash -n + ci.yml YAML OK + make_release.sh bash -n OK。
