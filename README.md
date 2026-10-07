# 普贤 PuXian（PX）

> **Python 的脸 + Rust 的类型 + Go 的并发 + C 的出身**
> 面向 AI 高效编程的自有语言，双模式运行于 Linux。

普贤（PuXian，`.px`）是一门从零实现的自有编程语言，核心定位是**让 AI（大模型）高效地编写可靠程序**：

- **语法 = Python 子集**：AI 在海量 Python 语料上训练，语法越像 Python，AI 生成的代码准确率越高；
- **类型 = Rust 风格**：渐进类型（不写类型直接跑，写类型拿性能）、枚举、模式匹配、Option/Result；
- **并发 = Go 风格**：`spawn` + `channel` + `select`，协程真并发；
- **出身 = C 编译器**：编译后端生成 C 源码，经 gcc 静态编译为**零依赖单二进制**（供应链中立、部署极简）。

一句话说明它现在到了哪一步：**编译器由自己写成 · 三轨语义一条真相 · 连「判据」本身也被守卫 · 上游生态逐字节引入并双轨回归。**

---

## ⚖️ License

![License](https://img.shields.io/badge/License-Apache--2.0-blue.svg)

PuXian 采用 **Apache License 2.0** 开源 —— 任何人可自由使用、修改、分发与商用（含闭源商用），无需授权费。完整条款见根目录 [`LICENSE`](LICENSE)。

对本项目使用者最重要的几点：

- **编译产物不受许可证约束**：你用 PuXian 写出的程序/服务，可任意闭源商用，不构成衍生作品；
- 修改并分发源码时，需保留版权声明与本 License 副本（Apache 宽松协议，不要求你的修改开源）；
- **贡献方式**：项目初期以提 issue 为主（bug 报告 / 功能建议 / 使用反馈），暂不开放代码 PR，路线由核心团队掌控；
- **Copyright © 2026 The PuXian Authors**

---

## ® Trademark

**达者同游** 为南瞻集团注册商标；**PuXian** 为达者同游团队项目名称（非注册商标）。
未经书面许可，不得将上述名称用于标识非达者同游团队官方发布的产品或服务。

---

## 🎉 当前状态

| 项 | 值 |
|---|---|
| 用户可见版本 | **`px 0.2.281`**（`./tools/px --version`；源码直跑取 `tools/px` 的 `SELFHOST_VER`，发布包含 git tag 生成的 `VERSION`） |
| 最新发布 tag | **`v0.2.285`** —— 命名 = `v0.2.<里程碑>`（M272 起：里程碑号即 patch 段） |
| 里程碑 | **M285**（143 个里程碑节 · `CHANGELOG.md` **15,859 行**） |
| 入库自举件 | **14 件**静态 ELF（`bootstrap/`，带源码链指纹门 `PXSRC-…` / `PXRT-…`） |
| 全量门 | **192 门 / 63 步**（单一注册源 `selfhost/gates.registry.sh` + 运行器 `run_gates.sh`） |
| 结构守卫 | **26 个**（`selfhost/check_*`：注册一致性 / 路径卫生 / 打桩锚点 / 判据串在场 / 前提 / 密钥 / 包架构 …） |
| native 名册 | **392 项**（单一事实源 `docs/native_index.json`，由 `tools/gen_native_table.sh` 生成，CI 防漂移） |
| 上游生态 | **122 个包**（`registry-px` @ `01f6048` · Apache-2.0 · **238 个上游用例 × 双轨**回归） |
| CI | 3 条工作流（`ci` / `release` / `tag-guard`）· 5 个 job（regression / examples / toolchain / multiarch-cross / **native-arm64 真机**） |

### 四条工程主线

> 这四条线是 M159 以来（尤其 M182–M285）的主要工作量所在 —— **不是加特性，而是把「同一件事的多个实现」收敛成一条真相。**

| 线 | 一句话 | 从哪里读 |
|---|---|---|
| **语义面 · 三轨一条真相** | 解释轨 / **VM 字节码轨（默认）** / C 文本轨对同一份源码必须给出**同一个答案**；宿主语言留空的角落逐条定规则（**响亮优于静默**） | [docs/spec.md](docs/spec.md) §17 · [docs/ERROR_CODES.md](docs/ERROR_CODES.md) |
| **内存面 · GC 根面** | VM 轨默认 precise GC（**不扫 C 栈**）⇒ 桥里只活在 C 局部的对象**必须登记**；两条硬约束 + 静态审计器 + 三个压力检测器 | [docs/GC_ROOTS.md](docs/GC_ROOTS.md) · [docs/GC_COVERAGE.md](docs/GC_COVERAGE.md) |
| **判据面 · 门自己也被守卫** | 门会「永远绿」「负控被抽掉牙」「判据串凭空捏造」⇒ 结构守卫族 + 前提自证 + 裁决器，把「门可信」变成可判定 | [docs/GATE_ASSERTIONS.md](docs/GATE_ASSERTIONS.md) · [docs/NEW_GATE_CHECKLIST.md](docs/NEW_GATE_CHECKLIST.md) · [docs/GATE_ISOLATION.md](docs/GATE_ISOLATION.md) |
| **分发面 · 一条链可判定** | 零依赖静态二进制 · 全静态可移植性门 · 包架构卫生门 · 漏打 tag 守卫 · 幂等发布步 | [docs/RELEASE_PROCESS.md](docs/RELEASE_PROCESS.md) |

### 能力一览

| 维度 | 能力 |
|---|---|
| 🏃 **三轨运行** | 解释轨（`px run`，秒起）/ **VM 字节码轨（`px build` 默认）** / C 文本轨（`px build --c`）—— 三轨行为一致，由全量门逐字节守（**语义分叉视为 bug，不是「后端差异」**） |
| 🔀 **并发** | `spawn` 真并发 · `channel` 阻塞通信 · `select` 随机就绪 · **并发 GC**（协作式 STW 安全点） · 帧协程 M:N 调度 |
| 🧹 **内存** | 编译模式：**precise GC**（VM 轨默认）+ slab 分配器（21 档 size-class）；解释轨追踪式 GC（回收循环引用）+ `gc()` 强制回收 |
| 🌐 **网络** | HTTP 客户端（TLS 1.2/1.3 · 连接池 · gzip/chunked · Unix socket）· **HTTP 服务端**（`http_serve` / `px_serve`：TLS+SNI · 优雅关闭 · per-route 限流 · 访问日志轮转 · 大 body 落盘 · **零停机换二进制** `SO_REUSEPORT`）· **HTTP/3 三栈合一**（`opts{"http3": true}`，与 HTTP/1.1 共用同一 vhost/路由/限流管道）· **WebSocket**（含 `ws_stream` 升级接管）· SSE · UDP |
| 🛡 **加密 / 文档** | AES-CBC-PKCS7 / AES-GCM · RSA · Ed25519 · XML 解析/生成 · **zip**（含 AES-256 / zipcrypto）· base64 / base32 / SHA1 / SHA256 / HMAC / PBKDF2 · **SQLite** · **零依赖 ONNX**（64 算子 + 拓扑执行器） |
| 📚 **标准库** | `stdlib/` **27 个 `.px`**，其中 **13 个公开库**（collections / semver / webroute / yaml / pxml / lunar / gfx / png / edge / cookiejar / html / multipart / smtp）—— `import std.<name>` 即用，编译/解释双模式一致 |
| 📦 **包管理** | `tools/pxpkg`（init / add / install / list / remove）· `px.pkg.lock` 可复现 + sha256 防篡改 · **多文件包**（每文件 <500 行规范与分发形态不再冲突）· `pxpkg sync`（registry 在线分发）**已立项**，见 [docs/PXPKG_SYNC_PLAN.md](docs/PXPKG_SYNC_PLAN.md) |
| 🔢 **语言能力** | 渐进类型 · 枚举 / 模式匹配 · Result/Option（`?` / `!` / `?.`）· 切片 `a[i:j:k]` · 生成器表达式（惰性）· 推导式 · 字符串插值 `${expr}` · 可选链 / 空合并 / 管道 `\|>` · 位运算 · 正则 · 锁原语 · fd / mmap 原语 · `in` / `not in` |
| 🔌 **边缘设备** | fd 原语（`open` / `ioctl` / `os_errno`，ioctl arg 三形态）· `read`/`write` 直通 · **mmap 活映射**（GC 自动 munmap）· GPIO / I2C / PWM / serial · **四架构交叉编译 + qemu 验证** |
| 🚀 **应用平台** | `px_serve` 应用服务器（Cookie/Session/基础认证 · `.px` 脚本执行 · 进程池热更新）· 路由表 + 中间件 · cron 调度 · `unix socket HTTP` 服务端 |

> 详细的语义规则、错误码与三轨口径见 [docs/spec.md](docs/spec.md)、[docs/ERROR_CODES.md](docs/ERROR_CODES.md)、[docs/PUXIAN_CHEATSHEET.md](docs/PUXIAN_CHEATSHEET.md)。

---

## 🚀 快速开始

### 准备（唯一依赖：gcc + make）

```bash
# Linux（x86_64 / aarch64 / armv7 / riscv64），确认 gcc 可用
which gcc
# 仓库自带自举工具链（bootstrap/pxc 编译器 + runtime C 运行时），克隆即用
```

### Hello World

```python
# hello.px
def main():
    let msg = "hello, 普贤\n"
    print(msg |> to_upper())
```

```bash
./tools/px run hello.px              # 脚本模式：解释执行，秒起
./tools/px build hello.px            # 编译模式：VM 字节码轨 → gcc 静态二进制
./hello/build/hello                  # 直接运行，零依赖（产物在 <目录>/build/）
```

### CLI 一览（`tools/px`）

| 命令 | 说明 |
|---|---|
| `px build <file.px>` | 编译为静态二进制（输出 `<目录>/build/<name>`）。**默认 = VM 字节码轨**；**按引用集自动裁剪**未引用模块（纯 CLI ≈ 2.7 MB，用 SQLite 自动保留 ≈ 3.8 MB，全能力 ≈ 9 MB）；解析失败自动退全量保编译成功 |
| `px build --c <file.px>` | **C 文本轨逃生舱**：走 `fn_*` → gcc 路径（纯计算热点 / 需要读生成的 C 时用） |
| `px build --full` / `--max` | **全能力编译**：跳过自动裁剪，runtime 全模块链接 |
| `px build --min <file.px>` | **显式最小化**：聚合全部 `--no-*`（quic / sqlite / ws / zip / xml / aes / rsa / ed25519 / img / route / zlib / h2） |
| `px build --target <arch>` | **高层交叉开关**：`x86_64` / `aarch64` / `armv7` / `riscv64`，一条命令折叠 `--cc` + 目标 mbedtls/sqlite/zlib 库路径 |
| `px build --print-plan` | **打印编译计划**（引擎轨 / 裁剪结果 / 链接模块 / CC / 目标架构）—— 跨架构与裁剪问题**一眼归因**（诊断优先于猜） |
| `px build --lto` | LTO 构建档（gcc `-flto` 全链） |
| `px run <file.px> [args...]` | 脚本模式执行（`--fast`：用户脚本构建缓存） |
| `px refs <file.px>` | 输出被引用的全局 / native 名集合（自动裁剪的引用采集层） |
| `px lex` / `px parse` | Token 流 / AST（调试，走 PuXian 自己的 lexer / parser） |
| `px fmt <file.px> [-w] [--check]` | 代码格式化（自举） |
| `px lint <file.px> [--json]` | 静态检查 L001–L008（自举）—— **作用域模型与语言一致**（M171） |
| `px doc` / `px test` / `px bench` | 文档生成 / 测试运行 / 基准测试（自举） |
| `px lsp` / `px mcp` | **LSP 服务器**（诊断/补全/跳转/悬停）· **MCP 服务器**（AI agent 经 MCP 调用 9 工具） |
| `pxpkg <子命令>` | **包管理器**：`init` / `add` / `install [--locked]` / `list` / `remove`（`PX_REGISTRY=<仓库>/registry`；`px.pkg.lock` 锁定版本 + sha256） |
| `px --version` / `px help` | 版本号 / 帮助 |

> **工具链全自举**：`pkg / ast / fmt / lint / test / doc / bench / lsp / mcp`（spec §12 全部 8 工具）
> 均由 PuXian 自己实现（`.px` 源码 → bootstrap 二进制 → `px` 子命令），零 Rust 依赖。
> 官方命令名为 `px`，`pxc` 为兼容别名（历史脚本照跑）。

---

## 🧭 多架构交叉编译（x86_64 / aarch64 / armv7 / riscv64；另有 OS 维度 `x86_64-windows` 骨架）

PuXian 编译产物是**零依赖静态单二进制**，天然适合树莓派 / 网关 / 边缘盒子 —— 在 x86 开发机上交叉编译，产物拷到设备即可运行。

### 1. 获取 musl 交叉工具链

```bash
# 路 A（推荐）：musl.cc 官方 tarball（免 root，解压即用）
#   aarch64 → aarch64-linux-musl-cross.tgz      ARM64（树莓派 4/5、网关、手机派）
#   armv7   → armv7l-linux-musleabihf-cross.tgz 32 位 ARM（树莓派 2/3、老工业盒子）
#   riscv64 → riscv64-linux-musl-cross.tgz      RISC-V（香山/玄铁、VisionFive2、信创）
curl -LO https://musl.cc/aarch64-linux-musl-cross.tgz
tar xzf aarch64-linux-musl-cross.tgz
export PATH=$PWD/aarch64-linux-musl-cross/bin:$PATH
```

> **为什么不用 apt 的 `gcc-aarch64-linux-gnu`？** 它是 glibc 交叉编译器：缺交叉头文件，且与仓库预置的 **musl** 静态库混链有 ABI 风险。官方只背书 musl 链路。

### 2. 一条命令交叉编译

```bash
# aarch64 库随仓库预置（runtime/mbedtls/lib-aarch64 + sqlite3-aarch64.o + zlib lib-aarch64）
./tools/px build --target aarch64 your_app.px
# 等价手工五 flag：
#   --no-quic --cc aarch64-linux-musl-gcc \
#   --mbedtls-lib runtime/mbedtls/lib-aarch64 \
#   --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o
# 产物：your_app/build/your_app —— ELF ARM aarch64 静态单二进制

# armv7 / riscv64：先用 tools/cross_multiarch.sh --arch <a> 现编目标库（CI 同款）再同法编译
```

### 3. 从源码自举（aarch64 官方通道 · M159）

```bash
# 只需 gcc —— 不需要预置 bootstrap 二进制（原生目标机，或交叉用 --cc）
selfhost/native_bootstrap.sh
selfhost/native_bootstrap.sh --cc aarch64-linux-musl-gcc --target aarch64
# 自证判据：编出的 pxc 再编 selfhost/compiler.px，与 selfhost/golden/compiler.c 逐字节一致
```

- `tools/px` **自动识别宿主架构**（选 `lib-<arch>`、非 x86_64 默认落 C 轨、自动 `--no-quic`），`--print-plan` 可打印本次编译计划用于归因；
- CI 的 **`native-arm64` 真机 job**：真机 aarch64 上自举 + 自证 + 现编 + 冒烟 + **包架构卫生门** + 打包；
- Release 附 **`puxian-bootstrap-aarch64-<tag>.tar.gz`** 并列资产（ARM 机器上免预置工具链）。

### 4. 一键验证（CI 四档矩阵同款）

```bash
bash examples/m67_aarch64/verify.sh                  # aarch64 三用例（需交叉 CC + qemu-aarch64-static）
bash examples/m67_multiarch/verify.sh                # 全四档矩阵
bash examples/m67_multiarch/verify.sh --arch aarch64 # 单档
```

> **`--target x86_64-windows`（Issue 14 W1a 骨架）**：OS 维度交叉，折叠 `cc=x86_64-w64-mingw32-gcc`
> （EPEL mingw64-gcc 或 zig wrapper）+ `lib-windows/` 库布局 + 默认 `--no-quic` ⇒ 产出 PE `.exe`。
> ⚠️ 定位是**编译链骨架**（W1a 阶段），非「Windows 平台一等支持」。
>
> **要点**：交叉编译默认配 `--no-quic`（H3/QUIC 的 ngtcp2 只有 x86_64 预编译库；边缘场景不依赖 H3，语义不受裁剪影响）；
> 非 x86_64 默认落 C 轨（VM 轨需目标架构的运行期支持）；musl 默认产出 static-pie 属正常形态；
> qemu-user 下并发 GC 模拟开销大 ⇒ 新架构 GC 由 arch 探针 + 真机验证。

---

## 🔧 判据基础设施（门）

> 这是本项目最与众不同的工程实践：**不只测产品，也测「测产品的那些东西」。**

**192 个门**分三类跑：语义门（三轨逐字节对拍）· 发射冻结门（产物指纹）· 基础设施门（重烘三连 / diffcheck 六路 / 结构守卫）。
清单在 `selfhost/gates.registry.sh`（**单一注册源** —— 此前「哪些门要跑」有三份互不相关的名单，靠人工同步 ⇒ 反复出事故），
运行器 `selfhost/run_gates.sh` 负责机制（PID 互斥 / 脏树检查 / **逐门超时** / 计时 TSV / 汇总 / `--only` / `--skip` / `--list`）。

**门自己会被咬的三个地方，都已有守卫**：

| 症状 | 守卫 | 来历 |
|---|---|---|
| 负控的打桩锚点**失配** ⇒ `sed` 静默失效 ⇒ 门照样绿而什么都没测 | `check_gate_assertions.py`（A1 锚点在位） | 全仓 355 个 `.sh`，`sed -i` 40 处，判定串 494 条 |
| 判据串**凭空捏造**（在仓库里根本不存在） | 同上（A2 判据串在场）+ `check_orchestrator.py`（O1 编排层） | 一次「红门清单为空」的假红 |
| 判据串**恒真**（如 `grep -q .`）⇒ 与输入无关 | 恒真探针（TBD · 见 ROADMAP 欠账） | 已登记 |

**结构守卫族**（26 个，`selfhost/check_*`）：门注册一致性（本地 ⇄ CI 双向）· 路径卫生（不许写死开发机绝对路径）· 里程碑↔缺陷编号一致性 · 嵌套密钥泄漏 · native 名册防漂移 · 覆盖面台账 · 锁内可失败分配 · 根契约 · shell 契约 · 包架构卫生 · 二进制可移植性（全静态 / GLIBC 基线）· 链接 flag 卫生 · 门间共享 `/tmp` 隔离 · 负控残留 · 孤儿门 · 版本字面量三方一致 · 门前提自证（P1–P4）……

**编排层入仓**（`packaging/chain/`）：`gate_verdict.sh`（**全量门裁决器** —— 三路求交才裁决，判红必须列得出红门）+ `watch_chain.sh`（通用观察器）——
把「等门按 PID · 绿推 tag / 红挂 p1」这套动作从临时脚本变成入仓、可自证、可复用的件。

**三条纪律**（血的教训换来的，写在 [docs/NEW_GATE_CHECKLIST.md](docs/NEW_GATE_CHECKLIST.md)）：

1. **门的判据要贴着被测对象的「契约」写** —— 比「整份 stdout+stderr」窄，比「看关键字」宽；
2. **负控必须各自独立判红**（改一处 ⇒ 只有对应的判据红；同时别让修复「吸收」掉旧负控的牙）；
3. **改了谁，门就要覆盖谁** —— 门写好了但**从未注册**，等于没有（M235 / M237 / M243 各撞过一次）。

---

## ♻️ 自举（Bootstrapping）

PuXian 最与众不同的地方：**它的编译器是它自己写的**。

```
selfhost/*.px（PuXian 源码）───编译───► bootstrap/pxc · pxc_vm（编译器二进制，入库）
                                          │ 编译任何 .px
                                          ▼
                                     C 源码 / 字节码 + runtime/ ──gcc──► 静态二进制

（真机/交叉 aarch64：selfhost/native_bootstrap.sh 用 gcc 现场自举 + 自证，无需预置二进制）
```

| 组件 | 说明 |
|---|---|
| `bootstrap/pxc` / `pxc_vm` | PuXian 版编译器（**C 文本轨** / **VM 字节码轨**二进位），静态 ELF，随仓库提交 |
| `bootstrap/pxi` / `pxi_vm` | PuXian 版解释器（源码头 / 字节码头） |
| `bootstrap/pxl` / `pxpar` | PuXian 版 lexer / parser（调试用） |
| `bootstrap/pxfmt` … `pxmcp` | 工具链自举件（fmt / lint / check / doc / test / bench / lsp / mcp）—— 合计 **14 件入库二进制** |
| `selfhost/*.px` | **编译器 / 解释器源码（PuXian 自己）**：**23 个 `.px`** —— `compiler.px`（CLI）→ `codegen.px` + `cg_*.px`（AST→C）与 `bc_emit.px`（AST→字节码），`interp.px` + `i*.px`（树遍历） |
| `selfhost/golden/compiler.c` | **C 轨自举基准**（**18,974 行**）：引导编译器编译自身的一次性产物 |
| `selfhost/golden/compiler.bc.dump` | **BC 轨自举基准**（**41,405 行**字节码镜像） |
| `selfhost/bootstrap_prove.sh` / `_bc.sh` | C 轨 / BC 轨自举证明：与基准**逐字节** diff |
| `selfhost/native_bootstrap.sh` | aarch64 官方通道：只需 gcc 的原生/交叉自举 + **自证** |
| `selfhost/rebake_bin.sh` | 入库件重烘 + **源码链指纹门**（`--check-all`：14 件二进制是否就是**当前源码**烘出的） |
| `selfhost/gates.registry.sh` + `run_gates.sh` | **全量门**（192 门）：清单与机制分离（M243）｜旧名 `selfhost/m116_gates.sh` 仍可用（兼容转发） |

**自举证明（三步 + 双轨自证）**：

1. 编译器 A（`bootstrap/pxc`）运行 `build compiler.px` → 生成 B.c；
2. `B.c` 与基准 `golden/compiler.c` **逐字节一致（18,974 行 0 差异）** → C 轨自举成立；
3. 强化闭环：B.c 经 gcc 编成 B 二进制 → B 再编译 compiler.px → B2.c，**A.c == B.c == B2.c 三者完全一致**；
4. **BC 轨并列自证**：编译器编译自身得到的**字节码镜像**与 `golden/compiler.bc.dump` 逐字节对拍（41,405 行 0 差异）。

> **Mini 子集（语言面锁定）**：自举期间语言被锁定为 **Mini 子集**（[docs/MINI_SUBSET.md](docs/MINI_SUBSET.md)）——
> 只准修 bug 不准加特性；PuXian 版编译器只需正确编译该子集（自身源码即在子集内）。
>
> **已知限制**：编译自己需 ≈6.5–7 min / 1.6 GB（C 运行时解释执行编译器逻辑）；编译模式 `str(float)` 大浮点 `%g` 精度（>6 位有效数字截断）；编译版无法解析含 NUL 的源码字符串。

---

## 📁 目录结构

```
├── bootstrap/              # 自举引导二进制（14 件静态 ELF：pxc/pxc_vm · pxi/pxi_vm · pxl/pxpar · 8 件工具链）
├── tools/px                # 用户入口（bash 包装，零 Rust 依赖）：build/run/lex/parse/fmt/lint/doc/test/bench/lsp/mcp/refs
├── selfhost/               # 自举工程（核心！）23 个 .px + 门与守卫
│   ├── compiler.px         #   PuXian 版完整编译器 CLI
│   ├── codegen.px + cg_*.px#   C 文本轨发射（AST → C）
│   ├── bc_emit.px          #   VM 字节码轨发射（AST → 字节码）
│   ├── interp.px + i*.px   #   解释器模块（tree-walking）
│   ├── lexer.px parser.px value.px env.px module.px   # 词法 / 语法 / 值系统 / 作用域 / 模块
│   ├── golden/             #   逐字节基准（compiler.c 18,974 行 · compiler.bc.dump 41,405 行）
│   ├── gates.registry.sh · run_gates.sh    # 全量门（192 门 · 单一注册源 + 运行器）
│   ├── check_*.sh / *.py   #   26 个结构守卫（注册 / 路径 / 锚点 / 在场 / 前提 / 密钥 / 架构 …）
│   ├── gcroot_*.py · gcstress_sweep.sh     # GC 根面审计器 + 压力筛
│   └── rebake_bin.sh · devbuild.sh         # 入库件重烘指纹门 / 开发构建
├── runtime/                # C 运行时（25 个 .c：runtime 主 + aes/xml/zip/ws/rsa/ed25519/sqlite/route/h2/h3/quic/image/onnx/ffi/zlib + coro/vm）+ mbedtls + third_party
├── stdlib/                 # 标准库 27 个 .px（13 个公开库 + L1 工具模块）
├── registry/               # 版本化库分发（135 个包 / 257 个版本目录：13 官方 + 122 第三方）
├── upstream-tests/         # 上游第三方用例（238 个 × 双轨回归 + EXPECTED.tsv 期望表）
├── examples/               # 229 个示例目录 / 120 个单文件 .px（含 192 门中的绝大多数门）
├── packaging/              # 发行打包（rpm/dnf 仓库 · make_release · tag_guard · pages · chain/）
│   └── chain/              #   全量门裁决器 + 通用观察器（M284 入仓）
├── archive/rust-compiler/  # Rust 版编译器源码归档（只读，自举前的实现，git 历史保留）
├── docs/                   # 文档（94 个 .md · 索引 docs/README.md · 规格 spec · AI 速查表 · 生态 · 路线图 …）
└── .github/workflows/      # CI：回归 + 自举证明 + 示例编译 + 四架构矩阵 + 原生 aarch64 + 发布 + Tag Guard
```

---

## 📚 文档

**先读这一张表，再决定深读哪一份**（完整索引与「哪些是历史档案」见 [docs/README.md](docs/README.md)）。

| 文档 | 说明 |
|---|---|
| [docs/README.md](docs/README.md) | **文档索引**（哪个文档管什么 · 按角色怎么读 · 哪些是历史档案） |
| [docs/spec.md](docs/spec.md) | **语言规格说明书**（唯一权威语义来源：词法/类型/表达式/语句/并发/模块/错误码/工具链接口/**§17 语义一致性收口**） |
| [docs/PUXIAN_CHEATSHEET.md](docs/PUXIAN_CHEATSHEET.md) | **AI 速查包**（整包喂大模型即可写对 `.px`：易错事实 · native 名册 · 三轨差异与统一口径） |
| [docs/MINI_SUBSET.md](docs/MINI_SUBSET.md) | **Mini 子集规范**（改编译器/解释器源码的硬约束） |
| [docs/ERROR_CODES.md](docs/ERROR_CODES.md) | **错误码总表 + 逐族口径**（含「同名两门」「真值性表」「文本语义接口」等专章） |
| [docs/GC_ROOTS.md](docs/GC_ROOTS.md) · [docs/GC_COVERAGE.md](docs/GC_COVERAGE.md) · [docs/GCSTRESS_LEDGER.md](docs/GCSTRESS_LEDGER.md) | **GC 根面**：两条硬约束 / 审计器判据与豁免 / 覆盖面台账 / 压力筛台账 |
| [docs/GATE_ASSERTIONS.md](docs/GATE_ASSERTIONS.md) · [docs/NEW_GATE_CHECKLIST.md](docs/NEW_GATE_CHECKLIST.md) · [docs/GATE_ISOLATION.md](docs/GATE_ISOLATION.md) | **判据基础设施**：门层守卫口径 · 新建门必查清单 · 门间隔离（共享 `/tmp` 白名单） |
| [docs/LOCK_ALLOC.md](docs/LOCK_ALLOC.md) · [docs/TABLE_PTR_CONTRACT.md](docs/TABLE_PTR_CONTRACT.md) · [docs/IO_EINTR.md](docs/IO_EINTR.md) | runtime 契约：锁内可失败分配 · 表元素指针跨锁 · EINTR 族收口 |
| [docs/HTTP2_DECISION.md](docs/HTTP2_DECISION.md) · [docs/HTTP3_STANCE.md](docs/HTTP3_STANCE.md) · [docs/HTTP_GZIP_NEGOTIATION.md](docs/HTTP_GZIP_NEGOTIATION.md) | **协议口径**：H2 不做（理由 + 迁移）· H3 立场与能力自证 · gzip 内容协商 |
| [docs/WS_CONN_LIFECYCLE.md](docs/WS_CONN_LIFECYCLE.md) · [docs/WS_UPGRADE.md](docs/WS_UPGRADE.md) · [docs/TLS_CERT_RELOAD.md](docs/TLS_CERT_RELOAD.md) | 连接生命周期：ws 引用计数 · 升级接管 · 证书热加载竞态 |
| [docs/DICT_STRICT_MIGRATION.md](docs/DICT_STRICT_MIGRATION.md) | **严格化迁移说明**（M163–M167：键严格 / 迭代快照 / 解包形状 / `items()`） |
| [docs/PERF_BASELINE_V2.md](docs/PERF_BASELINE_V2.md) · [docs/PERF_BASELINE_METRICS_SPEC.md](docs/PERF_BASELINE_METRICS_SPEC.md) | **性能基线 + 度量口径**（含跨平台对比的口径警告） |
| [docs/ECOSYSTEM.md](docs/ECOSYSTEM.md) | 生态总览（库定位与导出 API · 示例导航 · 消费路径 · 机器索引防漂移） |
| [docs/PX_DEF_TRIAGE.md](docs/PX_DEF_TRIAGE.md) · [docs/UPSTREAM_020_DEFECTS.md](docs/UPSTREAM_020_DEFECTS.md) | **第三方缺陷判定表**（逐条：真缺陷 / 文档缺口 / 误读）与上游 0.2.0 用例结果 |
| [docs/PX_RUN_FAST.md](docs/PX_RUN_FAST.md) · [docs/STR_CONCAT.md](docs/STR_CONCAT.md) | 性能特性口径：`px run --fast` 构建缓存（370×）· 字符串拼接的**常数 vs 复杂度**边界 |
| [docs/DEFECT_NUMBERING.md](docs/DEFECT_NUMBERING.md) | **缺陷编号纪律**（编号是全仓最常引用的「坐标」；指错不会让任何门变红） |
| [docs/ECOSYSTEM_GAPS.md](docs/ECOSYSTEM_GAPS.md) · [docs/GAP_ANALYSIS.md](docs/GAP_ANALYSIS.md) | 写库规范 checklist · 能力差距分析（边缘设备 / 2D-3D 两条线） |
| [docs/PXPKG_SYNC_PLAN.md](docs/PXPKG_SYNC_PLAN.md) | **立项**：`pxpkg sync`（registry 在线分发）—— 问题、四件工作（W1 客户端 / W2 发布物含 registry / W3 镜像 / W4 门）· 对 M69 旧决策的**重评估** |
| [docs/ROADMAP.md](docs/ROADMAP.md) | 路线图（能力基线 · 已完成主线 · 远期方向 · 语言面欠账） |
| [docs/RELEASE_PROCESS.md](docs/RELEASE_PROCESS.md) | 发布 SOP（tag 驱动全自动发布 · 发布物清单 · 漏打 tag 守卫 · 幂等步） |
| [CHANGELOG.md](CHANGELOG.md) | 变更日志（**143 个里程碑节**，含每轮的缺陷编号与验证证据） |
| [CONTRIBUTING.md](CONTRIBUTING.md) · [SECURITY.md](SECURITY.md) | 贡献指南 · 安全漏洞报告策略 |

---

## 🗓 里程碑

> 逐里程碑的完整记录（每轮的主题 / 缺陷编号 / 验证证据 / 重定基）在 [CHANGELOG.md](CHANGELOG.md)。
> 下面只保留**主线年表**。

### 一 语言内核（M0–M40，Rust 时代 · 全部 ✅）

| 阶段 | 内容 |
|---|---|
| M0–M9 | 需求/方案/规格 → 词法+语法 → 解释器 → 并发运行时 → C 代码生成 → 标准库 → AI 工具链（fmt/lint/test/bench/doc/ast）→ LSP/MCP → GC 值对象 → 包管理/模块化 |
| M10–M19 | HTTPS（TLS 1.2/1.3）→ 并发 GC → 文件随机读写+fsync → 锁原语 → sha256/xxhash → 正则 → HTTP 服务端框架 → `.px` 脚本执行机制 → 定时器 → AES/XML/zip |
| M20–M29 | C 运行时符号统一 → chunked/gzip/切片/base64/SSE → slab 分配器+追踪式 GC+WebSocket+位运算 → 网络/存储/安全收尾 → XML 生成+连接池 → 闭包循环回收+进程池+TLS 票据恢复 → `>>>`+WS 心跳+远程 registry → 服务端 TLS+Session → 路由/cron/SQLite → JSON 路径+Range+访问日志 |
| M30–M40 | 沙箱+虚拟主机+限流+连接线程池 → ws/wss 一行连接+SSE 重连+进程池热更新 → per-route 限流+TLS SNI+日志轮转+QUIC 预研 → 惰性生成器+WS 广播+事件总线 → gzip 解压+推导式 range+H2 最小服务端 → 日志增强+请求上下文+优雅关闭 → H2 over TLS+响应压缩+S3 → WS 自动重连+UDP echo → **Result/Option 唯一错误通道** → **字符串插值 `${expr}`** |

### 二 自举（M-B1 → M-B9b · 全部 ✅）

| 里程碑 | 内容 | 结果 |
|---|---|---|
| M-B1 | 能力门禁 + Mini 子集 + 对拍框架 | 能力自检 110/110 |
| M-B2 → M-B7 | lexer / parser / parser 错误恢复 / value·env·module / codegen / interp 逐个用 PuXian 重写 | 各组件对拍全过（双模式） |
| M-B8 | **自举证明** | **A.c == B.c == B2.c 逐字节一致** 🎉 |
| M-B9a | 退役 Rust 版 + 接入 CI + 引导链 | `tools/px` 全链路可用 |
| M-B9b | 第一个生产应用（dogfooding） | ✅ 已迁独立私有仓库维护 |

### 三 原生开发与平台化（M41–M158 · 全部 ✅）

| 阶段 | 主题 |
|---|---|
| M41–M45 | 类型系统欠账清零（edition/不可变/空安全/泛型）→ **显式 C 库 import（FFI）** → 文件即路由 → 语言糖（简化枚举 / `<-`）→ registry 版本化（semver + pxpkg + lock） |
| M46–M54 | **HTTP/3 / QUIC 全链路**：QUIC 传输 → H3 语义层 → QPACK（Huffman/静态表/动态表/SETTINGS/多路复用/解码器流 ack）→ **三栈合一 WebServer** + aioquic 外部互操作 → **生产化**（1-RTT resumption / 0-RTT / 连接迁移 / 流控协商） |
| M57–M61 | **边缘设备层**（fd 原语 / ioctl 三形态 / mmap 活映射 / GPIO·I2C·串口·PWM）→ 首个 dogfood daemon（pxhwmond）→ 数学与随机 → `std.edge` → zlib FFI proof + 纯语言 2D（`std.gfx` / `std.png`） |
| M62–M68 | 语言面欠账 L1–L11 → **工具链自举恢复**（fmt/lint/doc/test/bench）→ **LSP / MCP 自举收官**（spec §12 全 8 工具）→ runtime 原语补全 + stdlib 收编 → **多架构一等支持**（`arch.h` + 四档 CI 矩阵）→ pxi native 可达性根治（ffi 双表兜底） |
| M69–M82 | 生态启动（资产化 / AI 速查 / registry 拉取闭环）→ 语言缺口修复 → build 管线现代化（runtime `.o` 增量缓存 14.7s→0.94s · `--target` · MCP 第 9 工具 · 一键安装）→ AI 调试回路（逐行实时输出 / 运行期错误带源行号 / spawn 隔离 / bytes native）→ **RPM/dnf 分发闭环**（el9 → el7 全兼容 + 签名 + gh-pages 在线仓库）→ `http_serve_unix` |
| M83–M94 | **协程/调度完备性**（帧协程 M:N · 阻塞原语让出 · 抢占 · 定时器并入调度循环）+ **VM 字节码轨建设**（**M91 起 `px build` 默认 = VM 轨**） |
| M95–M103 | 服务端/客户端**全链路协程化**（http_serve / sse / route / vhost / middleware / 连接级 IDLE / 客户端 IO）+ 并发 TLS 握手修复 + `.px` 进程池协程化 |
| M104–M111 | 运行时性能与内存路径（LTO · 全局表 O(1) 名解析 · VM 全局名稳定槽位 · 字符串下标摊还 O(1) · slab + 页回收）+ 连接生命周期族 + 响应头「白名单→拒绝名单」+ 分配放大根因修复 |
| M112–M123 | **VM 轨（用户面默认轨）语义/FFI 收口** + 门判退出码 + 入库件重烘进 CI + 内置名册根治为单一事实源 + 服务进程/环境原语补全 |
| M125–M137 | runtime **内存安全三连**（分配失败降为请求级 · 信号处理器内错误不带走进程 · 隔离点回卷锁审计）+ **api-server / token-cache 全 PuXian 化** |
| M138–M148 | **Go 保真族**：正则 · `encoding/json` 逐字节复刻 · HTTP 客户端失败成因分类 + IPv6 · 大请求体内存安全 + chunked · 循环引用值的比较/渲染/JSON · float64 位模式 + NaN 语义 · `/` 完全 IEEE-754 + Go `%v` 浮点文本 |
| M149–M158 | **服务化前提族**：TCP「带超时 + 可辨别失败」· 摘要/密钥派生 + TLS 客户端族 · `tls_upgrade` · 分配率两刀 · `join` 字节口径 · 含内嵌 NUL 的字符串 · **零依赖 ONNX**（解析面 → 张量 + 64 算子 + 拓扑执行器）· 解释轨函数值 → runtime native 桥 |

### 四 M159 之后：收敛 / 生态 / 判据（全部 ✅）

> M159 起的主线不是「加特性」，而是**把「同一件事的多个实现」收敛成一条真相**，并把「怎么证明它」本身工程化。

| 波次 | 里程碑 | 主题 |
|---|---|---|
| **语义一致性收口** | M159–M169 | **aarch64 官方通道** · 闭包按引用 / 生成器捕获按值快照 · `sorted` 值比较+稳定 · 字典键严格化 · 迭代位置语义与用户索引分离 · **求值顺序 = 词法左→右** · 迭代期间改容器 ⇒ `R1003` · 解包统一（`for a,b in xs` + `items()`）· **分发可移植性**（openEuler + 全静态）· **模块体/帧的绑定归属** |
| **能力面与错误面** | M170–M181 | **GC 根面收口** · `px lint` 作用域对齐语言 · 诊断通道统一 · HTTP 反代/静态两件 · `d.get` 三轨真相 · `len(bytes)` / `os_popen` stderr · **零停机换二进制**（`SO_REUSEPORT`）· 内置面统一 · **运算族一条真相**（拆掉两颗静默坏值）· **HTTP/2 口径 + HTTP/3 能力自证** · **帧内绑定「未初始化即读」** |
| **生态引入 + 全面覆盖** | M182–M199 | native 桥登记窗口 · 数值解析严格化 · `bytes` 三面统一 · 解释轨透传完整性 · **第三方 registry-px 引入** · SHA1 族 + 安全替代 · bytes 边界族 · 方法族参数面 · **错误码面棘轮**（264 站点收口）· native 参数类型守卫 · 诊断文案 · 全内置 0 参探针 · 元数上界/时长族 · **[S10] 同一操作 × 每一个位置 × 错类型** |
| **上游全量再引入 + 运算符矩阵** | M200–M215 | `extern def` 全局发布 · **解释器件必须全能力** · base32/hmac_sha1/`sorted(key)` · 「此类型不支持X」族同码同文 · **复合赋值**三轨同一真相 · CLI 诊断通道 · **native 桥 GC 根面 20 处** · **GC 压力筛常态化** · VM 构造→登记窗口 · 审计器触发点源码派生 · 保留字作形参名 · **STW 打断 recv ⇒ 误判对端关闭** · 审计器判据诚实度 · 豁免窄条件 · **整数运算符逐值真值对拍（「三轨一致 ≠ 正确」）** |
| **逐值 / 逐型全量度量** | M216–M232 | 浮点→int 的 UB · 移位计数 UB · `and`/`or` 返回值 · **默认 PIE 工具链**（开发机恒绿、CI 必红）· `?`/`!`/`?.` 语义 · devbuild 产物指纹短路 · **`px_serve` 请求体契约** · devbuild 稳定源 · **gzip 内容协商** · H3 槽位串味 · 方法面逐位置×错类型 · **同名两门** · `in`/`not in` · tuple/result 方法面 · **索引/切片族** · **运算符 × 类型全量矩阵** · **真值性表 + 短路族** |
| **覆盖强度 + 判据基础设施一期** | M233–M248 | 文本兜底渲染器（任意值 ⇒ 其 `str()` 形态）· `bytes` 族逐类型 · **ws 连接关闭竞态 UAF** · `ws_stream` 升级接管 · 字段访问族 · tag 命名护栏 · `match`/`case` 族 · takeover 连接生命周期 · **里程碑↔缺陷编号守卫** · 表元素指针跨锁审计 · **门的单一注册源** · 门内并行 · 证书热加载竞态 · **新建门检查清单落盘** |
| **判据基础设施二期 + 编排入仓** | M249–M285 | 版本段每轮递增 · native 越界判据/覆盖面台账 · **孤儿门体检** · CI 独占 shell 契约 · 「能力存在但从没被证明可用」 · **EINTR 族收口**（一条语义、一份实现）· 容器「存储已回收」响亮自检 · 根栈收缩延迟语义 · **根栈隔离点两类语义显式分开** · GC 压力分批常态化 · 持锁可失败分配收口 · **门间隔离** · **性能基线 v2 + 全仓第一个性能判据** · `px run --fast` · 字符串拼接单次分配 · **版本口径方案 B** · 门级互斥锁 · **资源面判据**（CPU/RSS/线程/fd）· **上游 registry-px 0.2.0 全量再引入 + 凭据泄漏守卫** · **判据的「第 0 维：前提」** · **编排骨架入仓** · **门层判据守卫**（打桩锚点在位 + 判据串在场） |

> **小里程碑合流**：部分里程碑的补丁以 `sN` 后缀发布（如 `M208s1` / `M235s1` / `M237s1–s3` / `M284s1`），
> 通常是「全量门照出的连带项」或「CI 红收口」—— 它们在 CHANGELOG 里各占一节，tag 命名同样遵循 `v0.2.<里程碑>`。

---

## 🧪 示例

`examples/` 目录：**229 个示例目录 / 120 个单文件 `.px`** —— 其中绝大多数目录是**门**（三轨逐字节一致 + 负控独立判红）。

```bash
# 解释运行
./tools/px run examples/fib.px
./tools/px run examples/match.px
./tools/px run examples/m39_result.px
./tools/px run examples/m40_str_interp.px

# 编译为静态二进制
./tools/px build examples/fib.px && ./examples/build/fib
./tools/px build examples/m28_time_sqlite.px && ./examples/build/m28_time_sqlite

# 跑一个门（例：运算符全量矩阵）
bash examples/m231_op_matrix/verify.sh
```

快速上手清单：

- `hello.px` / `fib.px` / `match.px` —— 入门三件
- `concurrent.px` —— 并发（`spawn` / `channel` / `select`）
- `m39_result.px` —— Result/Option（`?` / `!`）；`m40_str_interp.px` —— 字符串插值
- `m28_time_sqlite.px` / `m28_route.px` / `m28_cron.px` —— SQLite / 路由中间件 / 定时调度
- `m29_webprod.px` / `m32_gen.px` / `m37_s3.px` —— WebServer 生产化 / 惰性生成器 / S3（SigV4）
- `m58_hwmond/` —— **dogfood 真实应用**（硬件健康守护 daemon，多文件工程）
- `m67_aarch64/` · `m67_multiarch/` —— 交叉编译与四架构矩阵
- `m159_hostarch/` … `m285_gate_assertions/` —— **语义与判据门**（回归时先跑这些）

---

## 🌱 生态

### 标准库（`stdlib/` · 27 个 `.px`，其中 13 个公开库）

| 库 | 定位 | 入口 |
|---|---|---|
| `collections` | 集合高阶操作：unique / group_by / chunk / flatten / zip_lists / sort_by | `import std.collections` |
| `semver` | 语义化版本 2.0.0：解析 / 比较 / 范围匹配（`^ ~ * x`） | `import std.semver` |
| `webroute` | 文件即路由命名：`get_healthz.px` → `GET /healthz` | `import std.webroute` |
| `yaml` / `pxml` | YAML 子集解析 / PXML 配置语言（规范见 [docs/PXML.md](docs/PXML.md)） | `import std.yaml` / `std.pxml` |
| `lunar` | 农历 1900–2100 公农历互转（含闰月） | `import std.lunar` |
| `gfx` / `png` | 纯语言 2D 画布（Bresenham / 字形 / 贴图）/ 纯语言 PNG 编码器 | `import std.gfx` / `std.png` |
| `edge` | 边缘设备：GPIO V2 / I2C / 串口 / PWM（Linux） | `import std.edge` |
| `cookiejar` | 会话 Cookie 管理（收 Set-Cookie → 按 URL 生成请求头） | `import std.cookiejar` |
| `html` | 简化 HTML5 容错解析器（正文提取 / 简单选择器） | `import std.html` |
| `multipart` / `smtp` | `multipart/form-data` 编码 / 轻量 SMTP 客户端 | `import std.multipart` / `std.smtp` |

### 包与注册表（`registry/`）

- **135 个包 / 257 个版本目录** = **13 个官方包**（与 `stdlib/` 同源镜像，`import` 名不同）+ **122 个第三方包**；
- 第三方来自 **`banshanhanfu/registry-px`** @ `01f6048`（**Apache-2.0**），**逐字节照搬**，来源/许可/补丁登记在 [`registry/THIRD_PARTY.md`](registry/THIRD_PARTY.md)，由 `tools/import_registry_px.sh` 重写并由门复核（重算 sha256 与磁盘对拍，防漂移）；
- **上游用例 238 个 × 双轨回归**（`upstream-tests/` + `EXPECTED.tsv` 期望表），逐条结果与 XFAIL 登记在 [docs/UPSTREAM_020_DEFECTS.md](docs/UPSTREAM_020_DEFECTS.md)；
- 消费路径：`import std.*`（内置）/ `pxpkg add + install`（版本锁定 + sha256）/ 直接拷源码。

> **我们对第三方缺陷的态度**：逐条**独立复核**并写进 [docs/PX_DEF_TRIAGE.md](docs/PX_DEF_TRIAGE.md)，
> 分类为「真缺陷 / 我方文档缺口 / 上游误读」—— **不盲信、也不甩锅**；能修的在下一轮收口，属文档问题的就补文档。

---

## 生态与合作

- **仓库外私有生产应用**：PuXian 的第一个真实生产用户（HTTP + SQLite 服务，dogfooding 验证），代码维护于独立私有仓库；镜像站 / 发布链等基础设施亦由 PuXian 服务承载。
- **外部 QA 驱动的修复**：多轮缺陷来自**生产流量**与第三方独立测试（含 aarch64 真机实测），修复均带最小复现与门。
- **问题反馈与贡献**：发现问题请附最小复现用例（单个 `.px` + 期望/实际输出）提交 issue；欢迎提交 PR 参与改进。

---

## 💡 Credits

PuXian 由 **达者同游团队** 开发，并使用 **wsAgent（文殊智能体）** 辅助设计与实现：

| 中文名 | 英文名 | 类型 | 角色 |
|---|---|---|---|
| 本源 | Benyuan | 人类 Human | 创始人 / 总架构师（方向与架构决策、最终验收） |
| 东月 | Dongyue | 智能体 wsAgent | 开发工程师（编码实现） |
| 清歌 | Qingge | 智能体 wsAgent | 设计师 / 质量发现（设计辅助与问题发现） |

> 本语言由 wsAgent（文殊智能体）辅助开发 —— 面向 AI 高效编程的语言，由 AI 参与编写，dogfooding 自证。团队成员明细见 [`AUTHORS.md`](AUTHORS.md)。
