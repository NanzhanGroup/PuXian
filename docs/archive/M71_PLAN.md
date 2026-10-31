# M71_PLAN · 构建管线现代化 + AI 交付一条龙（B1–B4 + MCP build + F4 更正）

> 创建：2026-09-06 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ 7464990，M70 闭环 · v0.1.0-m70 已发布 · CI run 108 全绿）
> 来源：qg-issue 12-build-pipeline（B1 runtime 预编译 .o / B2 --target / B3 per-arch Release / B4 一键安装）
>       + qg-issue 11-perf-correction（F4 文档更正）+ 用户点名 **MCP build 工具**（补齐「写→验→交付」最后一条腿）
> 用户指令（2026-09-06）：东月提出切分方案（M71 = build 管线 + AI 交付，M72 顺延 Issue 9/10）→ 用户「好的，按你的方案立项 M71，开工」
> 目标：① `pxc build` 亚秒级（runtime .o 增量缓存，等价 Go build cache）② `pxc build --target <arch>` 一条命令折叠 5 flag（对齐 GOOS/GOARCH）
>       ③ MCP 第 9 工具 `build`（pxmcp，AI agent 一条 MCP 完成写→验→**交付编译产物**）④ 发布/安装体验补齐（sha256sums + install.sh + argv0 自发现）
>       ⑤ F4「重文本用 C/shell」结论更正（慢的是 pxi 解释器，编译版毫秒级）。**纯工具链/分发/文档面，零语言语义改动**（不动 selfhost 语言源码 → 回归轻）。

## 〇、现状侦查（2026-09-06 源码实录，非记忆推断）

**pxc build 当前实现（tools/pxc cmd_build :137-241）**
- 流程：`bootstrap/pxc build file > build/<base>.c` → `copy_runtime`（把 runtime*.c/h + miniz + sqlite3.o/.c + ngtcp2/openssl .a/include 复制到 build/ 目录）→ **单条 gcc 全量现编现链**：`cc -static -O2 -pthread -o out base.c <12+ runtime*.c> sqlite3.o miniz*.c libmbedtls.a ... libz.a -lm -ldl`。
- 每次 build 都全量编译 runtime*.c（仅 sqlite3.o 预编译）；hello 也要 gcc 现编 runtime.c(572KB)+aes/xml/zip/ws/rsa/route/h2/ffi/zlib(+quic/h3/qpack 全量时)。
- copy_runtime 仅被 cmd_build 引用（tools/pxc:184 唯一调用点）→ 可安全改造。
- 三方静态库已 git 预置（同 sqlite3.o 模式）：runtime/mbedtls/lib{,-aarch64}/*.a、runtime/third_party/{ngtcp2,openssl}/*.a、zlib lib{,-aarch64}/libz.a、sqlite3{sqlite3.o,sqlite3-aarch64.o}。
- 交叉 5 flag 现状（RELEASE.md 原文）：`pxc build --cc aarch64-linux-musl-gcc --mbedtls-lib runtime/mbedtls/lib-aarch64 --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o --no-quic <file>`。
- riscv64 特殊：-no-pie（case "$cc" 判断）；交叉默认 --no-quic（M57-S4：ngtcp2/openssl 仅 x86_64 预置）。

**MCP 现状（tools/pxmcp.px，M65-S4 自举）**
- 8 工具（run/fmt/lint/test/bench/doc/ast/version），全部 os_spawn_capture(bootstrap/<bin>) 子进程隔离；
- **无 build 工具** → agent 经 MCP 拿不到编译产物（交付腿断链）；
- pxmcp.px 是 PuXian 源码（import jsonrpc_core.px），改后需 tools/pxc build tools/pxmcp.px 重建并 **cp 到 bootstrap/pxmcp 提交**（bootstrap 12 ELF 全部 git 跟踪）；
- pxmcp 统一入口环境变量：pxc mcp 注入 PX_BOOT（bootstrap 绝对路径）、PX_PXI；build 需再注入 PX_PXC（tools/pxc 绝对路径）或由 PX_BOOT 推导（dirname(PX_BOOT)/tools/pxc）。

**分发现状**
- make_release.sh：打 x86_64 单包（含 bootstrap ELF + tools + runtime 全树双架构库 + stdlib），内置 6 项冒烟（pxc --version / build hello / run / import std / 包外 PX_STDLIB / 工具链 --version）；tarball 无 sha256sums.txt 资产（GitHub 上传单资产）。
- tools/pxc 定位：PXC_HOME="$(cd "$(dirname "$0")/..")" —— 经软链调用（/usr/local/bin/pxc → 包/tools/pxc）会**错位**；stdlib 定位靠 cwd 相对（./stdlib ../stdlib）或手动 PX_STDLIB（cg_module.px:24 读 env）→ 包外任意目录不可用。
- bootstrap ELF 在 git → CI 直接跑仓库内二进制，回归 job/examples/toolchain/multiarch 全覆盖 build 路径。

**Issue 11（F4）事实（ECOSYSTEM_GAPS §3 表格行）**
- F4 现记「纯普贤逐行扫大文件慢（解释器字符串密集循环）…工具选型：重文本/大文件用 C 或 shell」——归因对象错（慢的是 pxi tree-walking 解释器，**pxc build 编译版扫 13578 行 runtime.c 仅 0.055s ≈ grep**），M69-S2 绕行快照被误当结论，需更正。

## 一、范围与边界

**做（S1–S6）**：
- **S1 · runtime .o 增量缓存（B1）**：pxc build 新增 rt-obj 缓存层（`$PXC_HOME/.rtcache/<key>/`，key = runtime 源+头内容哈希 × cc × no_quic），命中 → 只编 base.c + 链接预编译 .o（亚秒级）；未命中 → 一次性全编入缓存。等价 Go build cache。三方库/sqlite3.o 维持 git 预置模式不动。**取舍记录**：不做 runtime*.o 全量 git 跟踪（572KB runtime.c 等频繁变更 → 二进制漂移风险 + 仓库体积），本地缓存 + 发布包预热（见 S4）已覆盖 B1 验收「二次 build 不再现编」。
- **S2 · `pxc build --target <arch>`（B2）**：高层开关折叠 5 flag——arch ∈ {x86_64, aarch64, armv7, riscv64}；自动映射 cc（PATH 探测 <arch> 交叉 gcc）/ mbedtls-lib / sqlite-obj / zlib-lib / 默认 --no-quic（交叉）/ riscv64 -no-pie；显式 flag 覆盖 target 默认。cc 缺失 → 友好报错（指引 musl.cc / zig / cross_multiarch.sh），不联网自动下载（不可预期）。**取舍记录**：zig cc 自动 fallback 不内嵌（复合命令进 cc 变量与现有 eval 字符串架构冲突大），文档指引「zig cc wrapper 入 PATH 即可用 --target」。
- **S3 · MCP `build` 工具（第 9 工具，交付腿补齐）**：pxmcp 加 build——params {file(必填) / code / target(可选) / no_quic(可选)}，os_spawn_capture(PX_PXC, [build, flags…, file]) 调 tools/pxc（与 CLI 完全同语义，零重复实现）；返回产物路径文本；description 写明写盘副作用（源文件同目录 build/）。tools/pxc cmd_mcp 注入 PX_PXC。重建 bootstrap/pxmcp 提交。
- **S4 · 发布/安装体验（B3 收敛 + B4）**：make_release 多资产 + sha256sums.txt；新 tools/install.sh（curl|sh 一键装：uname -m 选资产 → 验 sha256 → 解压 → 软链 → PX_STDLIB 自动）；tools/pxc argv0 自发现（readlink -f 解析软链真实根）+ stdlib 自动注入（包根有 stdlib 且 PX_STDLIB 未设 → export）。**B3 per-arch native 工具链包降级按需**（bootstrap 12 二进制 × 3 架构交叉编 + qemu 冒烟回归面过大；--target（S2）已让 x86_64 包单侧交叉出任意架构产物，native 工具链等真实 aarch64 开发机需求，ROADMAP 按需记录）。
- **S5 · F4 更正 + 文档收口（Issue 11）**：ECOSYSTEM_GAPS F4 行更正（归因 pxi 解释器，编译版毫秒级≈grep）；CHEATSHEET/ROADMAP/CHANGELOG/README(.en)/RELEASE_PROCESS 同步；pxc usage 头注释同步（cache/--target/MCP 9 工具）。
- **S6 · 发布**：tag `v0.1.0-m71` → CI 自动发布（make_release 内置冒烟覆盖新 build 路径）+ 本机留档 + 发布指引更新。

**不做**：
- ❌ 语言语义改动（selfhost/*.px 零改动 → golden 零漂移、diffcheck 无需重跑）。
- ❌ Issue 9（print 全缓冲）/ Issue 10（AI 诊断）—— 已定 M72「AI 调试回路」（动 runtime.c → 需 selfhost 重建，范围分离）。
- ❌ B3 native 工具链矩阵、zig 内嵌 fallback、字节码 VM（Issue 12 远期正道）、macOS/Windows。
- ❌ 不开 PR（维持既定边界，直接 commit + push origin main）。

## 二、分步计划

### S1 · runtime .o 增量缓存（tools/pxc）
- 新函数 `rt_key()`：按固定序对参与编译的 runtime 源（runtime*.c/h + arch*.h + miniz*.c，quic 与否决定集合）逐文件 sha256sum 聚合 + cc 名 + no_quic 标志 → sha256 前 16 位。key 变化（改 runtime 源码/换 cc/切 quic）→ 自动重建缓存。
- 新函数 `rt_ensure()`：cache_dir=$PXC_HOME/.rtcache/$key；逐 .o 存在性检查，缺失 → mkdir + 逐 .c 编译入缓存（flags 照抄现状 gcc 命令的 defines/includes：-O2 -static -pthread [-DPX_NO_QUIC] -I…）。
- 新函数 `rt_link()`：编译 base.c 为 base.o（-I runtime 各 include 绝对路径）→ 链接 `base.o <cache>/*.o sqlite3.o mbedtls.a… zlib.a -lm -ldl`（第三方 .a 直接引用 RT_DIR 绝对路径，免复制）。
- cmd_build 重构：保留 bootstrap/pxc build 生成 base.c → 删 copy_runtime 调用 → rt_ensure + 编 base.o + rt_link。build/ 目录不再复制 runtime 全树（只留 .c/.o/ELF）。
- .gitignore 加 `.rtcache/`。
- 验证：quic 全量 / --no-quic / hello / std import / sqlite 程序 × 首次（miss）与二次（hit）计时对比；file 静态链；运行输出与现状一致；tools/pxc 交叉路径（--cc）同样走缓存（key 含 cc）。

### S2 · --target（tools/pxc cmd_build）
- 参数加 `--target <arch>`；arch→默认映射（cc/mbedtls/sqlite/zlib/no_quic/pie）assoc 数组；显式 --cc/--mbedtls-lib/--sqlite-obj/--zlib-lib/--no-quic 覆盖；`--cc` 与 `--target` 同给时 cc 用显式（其余仍 target 默认）。
- cc 探测：command -v <arch>-gcc 族；缺失 → 报错含指引（musl.cc / zig wrapper / tools/cross_multiarch.sh 现编库）。
- usage + 头注释同步。验证：--target x86_64 == 现状；本机无交叉 cc → 友好报错；尝试下载 musl.cc aarch64 工具链做真交叉验证（网络通则可做，不通则以 CI multiarch 矩阵为验收）。

### S3 · MCP build（tools/pxmcp.px + tools/pxc + 重建）
- tools_meta() 加 9th tool：build（desc 注明写盘副作用 + 产物路径约定；params: file false / code false / target false / no_quic false）。
- mcp_call_tool 加 build 分支：mcp_resolve(arg) 取路径（file/code 临时文件都支持）→ 组 args（--no-quic/--target 透传）→ spawn 执行器路径（env PX_PXC 优先，缺省 dirname(PX_BOOT)/tools/pxc）→ text=rc 输出（含「编译成功: <path>」）；rc!=0 → isError。
- tools/pxc cmd_mcp：注入 PX_PXC="$PXC_HOME/tools/pxc"；usage「8 工具」→「9 工具 build」。
- 重建：tools/pxc build tools/pxmcp.px → cp tools/build/pxmcp bootstrap/pxmcp；fmt/lint 过（pxmcp.px 在 CI toolchain 收敛域）。
- e2e：仿 examples/m65_mcp/mcp_client_s4.py 写 m71 verify（initialize → tools/list 断言 9 工具 → tools/call build hello 临时工程 → 产物存在 + 可运行）。

### S4 · 发布/安装（make_release.sh + tools/install.sh + tools/pxc）
- make_release.sh：产物收集段生成 `sha256sums.txt`（与 tarball 同目录）+ release.yml 上传多资产（tarball + sha256sums.txt）；RELEASE.md 更新安装段（install.sh 用法 + --target 交叉示例）。
- tools/install.sh（新，进发布包）：`curl -fsSL …/install.sh | sh` —— uname -m（x86_64|aarch64）→ 从 GitHub latest release 拉 puxian-*.tar.gz + sha256sums.txt → 验 sha256 → 解压 ~/.puxian/<ver> → 软链 ~/.local/bin/{pxc,pxi…}（或 /usr/local/bin，按权限降级）→ 打印验证命令。
- tools/pxc argv0 自发现：`SELF="$(readlink -f "$0" 2>/dev/null || echo "$0")"` → PXC_HOME 基于 SELF 真实位置；stdlib 自动注入：若 PX_STDLIB 未设且 "$PXC_HOME/stdlib" 存在 → export PX_STDLIB（run/build/… 全部子命令生效）。注意 readlink -f 在 macOS 不可用 → fallback（Linux 定位不变）。
- 验证：软链 pxc 到 /tmp/bin 从任意 cwd 调 build/run（stdlib import 命中）；install.sh 本地 dry-run（mock 下载源为本地 tarball）。

### S5 · F4 更正 + 文档收口
- ECOSYSTEM_GAPS §3：F4 行「工具选型」更正为「归因更正：慢的是 **pxi 解释器**（tree-walking）；**pxc build 编译版毫秒级（0.055s/13578 行 ≈ grep 14×）**，重文本编译版可胜任」+ 处理列标 M71 更正；文件头「更新」行加注。
- PUXIAN_CHEATSHEET：核对是否有 F4/工具选型引用，同步更正；ROADMAP：主线表 M71 行 + 能力基线工具链行（mcp 8→9 工具 / build cache / --target / install.sh）；CHANGELOG [Unreleased] M71 条目（S1–S6 提交汇总）；README(.en) 能力表工具链行 + 交叉编译章节（--target 用法）；RELEASE_PROCESS.md 发布 SOP 更新（多资产）。
- tools/pxc usage/头注释（S2 已改）+ pxmcp.px 头注释（9 工具）。

### S6 · 发布
- 全链绿 → commit 收口 → tag v0.1.0-m71 → push → CI release 自动发（make_release 冒烟 = 新 build 路径验收）。
- GitHub 产物二次冒烟（sha256 三方核对 + 解包 build/run/import std + --target 帮助）。
- 本机留档（/data/release）+ 发布指引更新至 m71。

## 三、回归总闸（判据）
1. CI 六 job 全绿：regression（自举 + diffcheck 全量——M71 不动语言应零漂移）+ examples + toolchain（fmt/lint/生态防漂移/M65-M67 verify）+ multiarch 三架构交叉（**覆盖 S1 缓存 + S2 --target 交叉路径**）。
2. build 三态实测：quic 全量 / --no-quic / std import+sqlite 程序，产物 file=statically linked + 运行输出与 M70 一致；二次 build 亚秒（日志显示走 .o 链接不现编 runtime）。
3. MCP e2e：tools/list 9 工具 + tools/call build 出静态 ELF（examples/m71_mcp_build verify）。
4. make_release.sh 内置 6 项冒烟全过 + sha256sums.txt 生成；install.sh dry-run 装出可用 pxc。
5. fmt/lint 0 错；ci/release YAML + 新增 sh 脚本 bash -n；CHANGELOG/ROADMAP/README 收口 commit。

## 四、验收方法（Issue 12 六节 + B4 对齐）
1. **B1**：同一机器二次 `pxc build hello.px` 不再现编 runtime（strace 或耗时对比：二次 < 1s，首次 miss 与原状同级）；改 runtime.c 后自动重建缓存。
2. **B2**：`pxc build --target aarch64 hello.px` 一条命令产出 aarch64 静态 ELF（CI multiarch qemu 可运行）；--target x86_64 与现状一致。
3. **MCP build**：MCP client 调 build → 返回产物路径 → 文件存在且 file=static ELF（AI 交付闭环）。
4. **B3/B4**：Release 每 tag 附 tarball + sha256sums.txt；新机器 install.sh 后 pxc 任意 cwd 可用（import std.* 命中，免 PX_STDLIB）。
5. **F4**：ECOSYSTEM_GAPS 文档更正提交可见。
