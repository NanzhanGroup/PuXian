# Changelog

本项目的所有重要变更都会记录在此文件。
格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，
版本语义遵循 [Semantic Versioning](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### M67 · 多架构一等支持：aarch64 交叉编译 + GC 架构抽象 + armv7/riscv64（qg-issue 07，docs/M67_PLAN.md）✅

> 来源：清歌 qg-issue 07（任务清单·执行版）两阶段 —— 阶段一 aarch64 交叉编译提升为一等支持；
> 阶段二 runtime GC 架构抽象层 + armv7（armhf）/riscv64 扩展。决策：D1 armv7/riscv64 交叉库
> CI 现编 + actions/cache **不入库**（aarch64 维持仓库预置）；D2 armv7 = armhf（linux-musleabihf）；
> D3 GC 改动**重构等价**（不做算法变更）；D4 两阶段一体入 M67。实测修正：apt glibc 交叉不可背书
> （官方只支持 musl.cc / docker 两路）；riscv64 需 -no-pie；qemu-user 并发 GC 模拟限制留档。

- **M67-S1~S3 阶段一 aarch64 一等支持（commit d361b29）**：README.md / README.en.md 单列
  「ARM64 Linux 交叉编译」章节（musl.cc tarball + docker 两路获取 / 一条命令交叉 / file+qemu 校验 /
  为何不用 apt glibc）；CI 新增 aarch64 job（actions/cache musl 工具链 + 三用例 qemu 验证）；
  examples/m67_aarch64/{hello_a64,http_a64,sqlite_a64}.px + verify.sh（交叉编译 → file 断言 ELF ARM
  aarch64 → qemu 运行：hello 直跑 / HTTP qemu 起服宿主 curl 200 / SQLite CRUD）本机实测全绿。
- **M67-S4 runtime GC 架构抽象层（commit e97b377，阶段二心脏）**：runtime.c 3 处 GC 架构 #if
  （gc_scan_stack / gc_scan_registers / gc_scan_thread_stack）迁出为统一接口 `runtime/arch.h`：
  `arch_read_sp()`（当前线程 SP，内联汇编跨 glibc/musl）+ `arch_scan_registers(uc, mark_cb, ctx)`
  （暂停线程 ucontext → 逐寄存器 word 回调标记）+ `arch_uc_sp(uc)`；分架构头 `arch_x86_64.h` /
  `arch_aarch64.h`（原样迁出，行为零变化）+ 新增 **`arch_armv7.h`**（armhf：arm_r0..arm_r12 + arm_sp，
  14 word 扫描）与 **`arch_riscv64.h`**（mcontext `__gregs[32]` RISC-V psABI 序，sp=`__gregs[2]`）——
  GC 主逻辑不再见架构 #if，以后加架构只增头文件。等价性证明：x86_64 diffcheck --all + capability
  双模式 253 PASS + examples/m67_multiarch/gc_stress（并发 4 worker 深链 + stop-the-world）+ m65/m66
  verify 零回归；aarch64 交叉 verify 全绿；**armv7/riscv64 C 层探针**（SIGUSR1 ucontext → 扫 14/32
  regs + SP）musl 交叉 + qemu 实测布局通过。
- **M67-S5 工具链扩展（阶段二 #2 通道）**：tools/pxc zlib 自动探测扩三架构（aarch64/armv7/riscv64
  → lib-<arch>）+ copy_runtime 拷 arch 头 + **riscv64 自动 `-no-pie`**（musl static-pie 报
  "read-only segment has dynamic relocations" 实测修复）；`tools/cross_aarch64.sh` 泛化为
  **`tools/cross_multiarch.sh --arch aarch64|armv7|riscv64 [--outdir dir]`**（sqlite3.o + mbedtls +
  zlib 三件套独立副本交叉现编，架构校验 file 断言），cross_aarch64.sh 保留为 --arch aarch64 兼容薄包装。
- **M67-S6/S7 交叉库 + 四档矩阵**：armv7/riscv64 交叉库本机现编至 /opt/px-multiarch（不入库）；
  examples/m67_multiarch/{hello_multi,http_multi,sqlite_multi,gc_stress,gc_single}.px + verify.sh
  四档矩阵 —— **x86_64 native 全量**（含 gc_stress 并发 GC 压力 2000 + gc_single 单线程 3 万迭代）
  + **aarch64/armv7/riscv64 qemu-user** 各 hello/http/sqlite（qemu 起服宿主 curl 200，file 断言
  ARM aarch64 / ARM EABI5 / RISC-V）本机全矩阵 exit=0；CI 升级 **multiarch-cross 矩阵 job**
  （三架构并行：musl 工具链 + 现编库 actions/cache，aarch64 用仓库预置）。
- **M67-S8 生态收口**：spec §8.21 多架构段（arch.h 接口 + 四架构 + qemu 限制记录）；ROADMAP 补 M67 行 +
  当前里程碑 M67 已闭环；CHANGELOG 本条目；README/README.en 里程碑表扩 M41–M67；
  tools/make_release.sh 发布包含 cross_multiarch.sh + RELEASE.md 内容表更新。
- **验证**：四档矩阵 verify ALL PASS（x86_64 含 GC 压力）；spec/README/CHANGELOG 全绿；ci.yml
  YAML OK；diffcheck/capability/m65/m66 零回归（S4 提交时全绿）；qg-issue 07 归档 done/。

### M66 · 自举 wsAgent runtime 原语补全 + stdlib 收编（qg-issue 01–06 全量合入，docs/M66_PLAN.md）✅

> 来源：清歌（qingge）qg-issue 01–06 —— ws-core / ws-install / ws-todo 等 wsAgent 生态模块
> 被卡的 runtime 原语缺口（P0 blocker）与 stdlib 生态缺失。决策：D0 以官方仓库现状为准；
> D1 os_capture 与既有 os_spawn_capture 并存（零回归）；D2 zip 密码 zipcrypto + WinZip AES-256 全实现；
> D3 农历纳入 M66（std.lunar）；D4 命名「自举 wsAgent」。

- **M66-S1 L0 runtime 原语补全（qg-issue 01/02/05，runtime/runtime.c +386 / runtime_zip.c +255）**：
  新增 **unix_connect**（AF_UNIX 裸连接 fd，行协议驱动，http_unix 补全）；**os 五件套** os_exec
  （execvp 进程替换，launcher 语义）/ os_rename（rename(2) 原子覆盖）/ os_remove_all（递归删，空串
  与 `/`、`//` 拒绝防删根）/ os_random_hex（urandom→hex）/ os_file_sha256（mbedtls 文件哈希）；
  新增 **os_capture(cmd,args)→{rc,stdout,stderr}**（双管道分离捕获，D1 与 os_spawn_capture 并存；
  G6 which 用法示例 `os_capture("command","-v",x)`）/ **os_popen(cmd,args)→{stdin_fd,stdout_fd}**
  （双向管道 + setpgid 自成组）；**os_kill 第三参 group**（kill(-pid) 组杀，两参兼容）；
  **write_file/append_file 第三参 mode**（open + fchmod 防 umask 削权，写 0600 密钥免 chmod）；
  **zip_unpack 第三参 password**（**zipcrypto 传统 + WinZip AES-128/192/256**：extra 0x9901 探测 +
  PBKDF2-HMAC-SHA1 + AES-CTR + HMAC-SHA1 auth code，AE-1/AE-2）。pxi 白名单 + ibuiltin + pxlint
  BUILTINS 三处同步 → bootstrap/pxi、pxlint 重建（解释模式新 native 可调）；专项
  examples/m66_proc verify ALL PASS（os_fs_test 16 + os_exec 透传 + proc_test 14 + unix 行协议 +
  zipcrypto/AES-256 解包）；capability 双模式 253 PASS + diffcheck 全量 + m65 回归零影响。
- **M66-S2 L1 stdlib 收编 std.yaml（qg-issue 03，第 7 个标准库）**：stdlib/yaml.px（YAML 配置子集
  解析 yaml_parse，纯函数零 import）+ examples/m66_yaml 双模式 35 PASS 一致。
- **M66-S3 L1 stdlib 收编 std.pxml + PXML 规范（qg-issue 04，第 8 个标准库）**：stdlib/pxml.px
  （解析 + ENC 加密还原，主打编译模式）+ **docs/PXML.md 规范 v0.6**（语法/EBNF/enc 策略/决策表/
  语言缺口笔记）+ examples/m66_pxml dogfood 闭环（pxml_test 双模式 68 + demo 16 + demo_enc 4 PASS）。
- **M66-S4 L1 stdlib 收编 std.lunar（qg-issue 06 T3，第 9 个标准库，D3 拍板纳入）**：
  stdlib/lunar.px 内嵌 1900-2100 农历数据表（寿星天文历同源，逐年对拍 0 误差）+ 公历/农历互转
  （lr_solar_to_lunar / lr_lunar_to_solar / lr_md_in_year / lr_leap_month 等，纯函数双模式一致）+
  examples/m66_lunar 双模式 **36 PASS**（春节/除夕/闰二月/边界 1900-2100/往返/ws-todo lunar:8-8 落点）。
- **M66-S5 生态收口**：spec §8.20 原语补全段 + §10.3 std.yaml/pxml/lunar 行；ROADMAP 主线表补 M66 行；
  CHANGELOG 本条目；README/README.en 原生开发表扩 M41–M66 + 标准库 9 个清单同步；CI toolchain job
  工具自测并入 m66_yaml/m66_pxml/m66_lunar verify。
- **M66-S6 qg-issue 回写**：/data/qg-issue 01–06 头部状态行标「已合入 M66」；05 §7 / 06 §7 重写时机
  checklist 勾除（G1/G2/G3/Issue 2/农历全勾 → ws-install / ws-todo 主体 .px 可写，M67+ 业务里程碑另立项）。
- **验证**：m66_proc/m66_yaml/m66_pxml/m66_lunar verify ALL PASS；capability 双模式 253 PASS / 0 FAIL；
  diffcheck --all 零回归；fmt --check + lint 全仓全绿（stdlib 3 新库 lint 0/0）。

### M65 · LSP / MCP 自举（spec §12 工具链收官 + §12.1 AI agent 协议，docs/M65_PLAN.md）✅

- **M65-S1 JSON-RPC 共享底座 + runtime 补丁**：`tools/jsonrpc_core.px`（纯 defs）——
  Content-Length 帧读写（read(0) 累积缓冲，半包/粘包/坏头/坏 body 自测 33 断言双模式）+ JSON-RPC
  2.0 骨架（request→result / notification→无回 / 标准错误码 -32700/-32601）；runtime 补唯一原语
  **`os_spawn_capture(cmd,args)→[rc,output]`**（fork+execvp，stdout+stderr 合并单管道免死锁，
  exec 失败 127，编译/解释双模式 5 断言）；pxi 白名单 + ibuiltin 同步重建；pxlint BUILTINS 补裸
  read/write（M57 fd 原语漏补）；capability 双模式 253 PASS + diffcheck 全量无回归。
- **M65-S2 LSP 核心**：`tools/pxcheck.px` 独立诊断器（import parser+lint_core，lex+parse+lint →
  stdout 单行 JSON；parse/lex 错误= parser 打印后 panic 退出 1，pxlsp 按文本解析）+ `tools/pxlsp.px`
  0.1.0 —— 生命周期 initialize/initialized/shutdown/exit + didOpen/didChange(Full)/didSave/didClose +
  **publishDiagnostics**（**深度诊断子进程化**：parser 语法错误 print+panic 杀进程不可捕获 → pxcheck
  子进程隔离）；python3 模拟标准 LSP client 17 断言端到端全绿。
- **M65-S3 LSP 增强**：`tools/lsp_core.px` 符号/补全/跳转/悬停语义层（顶层 def/struct/enum/trait/
  impl/var 文本级行扫 + ## 文档注释并入 + 局部名宽松收集）+ pxlsp 0.2.0 开 completion/definition/
  hover 能力位（诚实协商）；S3 client 39 断言全绿（9 来源补全候选 + 前缀过滤 + definition 行精确 +
  hover 签名/文档 + 真实文件 selfhost/astdump.px + 错误输入不崩）。
- **M65-S4 MCP 服务器**：`tools/pxmcp.px` —— MCP 2024-11-05 stdio transport；tools/list 暴露 8 工具
  （run/fmt/lint/test/bench/doc/ast/version 带 inputSchema）+ tools/call **全部子进程执行**
  （os_spawn_capture 调 bootstrap 各二进制：崩溃隔离不污染协议 stdout + 输出可捕获）；S4 client
  41 断言端到端全绿（每工具成功回包 + 错误参数/未知工具/未知方法 isError/-32601）。
- **pxc 子命令**：`pxc lsp` / `pxc mcp`（bash exec 直通保留 fd 0/1，stdio 即协议通道）。
- **文档 / 收口**：spec §12 实现状态表 8 工具全自举 + §12.1 MCP 勾选；ROADMAP 主线表补 M62–M65 行
  （M62–64 此前仅 CHANGELOG 记录，缺行补全）+ 工具链行扩 11 子命令；README/README.en CLI 表补
  lsp/mcp + 原生开发表扩至 M41–M65；M64_PLAN M64d 状态行更新（已由 M65 承接完成）。
- **验证**：m65_lsp + m65_mcp verify ALL PASS；fmt --check + lint 全仓全绿；capability 双模式
  253 PASS / 0 FAIL；新 5 文件（jsonrpc_core/pxcheck/lsp_core/pxlsp/pxmcp）lint 0 错 0 警。

### M64 · 工具链自举恢复（fmt/lint/doc/test/bench 五项自举，docs/M64_PLAN.md）✅

- **M64-S1 keep-lexer 底座**：不碰 pxlexer（自举链零风险），派生 `tools/fmtlexer.px`
  （788 行，`g_keep` 开关 + `lex_tokens_keep` 保留注释/行结构，对齐 Rust
  `new_with_comments`）；默认模式与 pxl 逐字节一致。
- **M64-S2 `pxc fmt`**（fmt_core.px + pxfmt.px）：确定性格式化（空格规则/行结构重建/
  注释对齐/空行压缩/unified diff/幂等）；修正 Rust 版一元负号缺陷
  （`x = -1` 不再压成 `x =-1`）；格式化前后重 lex token 序列完全一致（语义等价）。
- **M64-S3 `pxc lint`**（lint_core.px + pxlint.px）：L001-L008 全移植（AST 驱动，
  复用自举 parser），`--json`/`--strict`/退出码语义；自举工具链 7 文件 dogfood 0/0。
- **M64-S4 fmt 全仓收敛**：审阅驱动修 4 缺陷（插值/数字原文保真、切片/后缀 `?` 紧贴、
  首行 def 压坏）；selfhost 21+tools 7 文件 --check 全绿；净 -318 行；
  **自举证明 B.c==golden 逐字节 + capability 双模式 253 PASS**。
- **M64-S5 `pxc doc / test / bench`**（pxdoc/pxtest/pxbench.px + pxslice.px 共享切片）：
  doc 从 `##` 注释生成 Markdown（对齐 Rust doc.rs：文件头说明并入首个定义）；
  test 运行顶层 `def test_xxx()` 无参函数，逐用例独立 pxi 子进程（语言内 os_spawn+
  os_wait 编排，非 shell）；bench 无参目标 N 次循环 × R 轮计时（now_us）。
  三工具 --version/--help + examples/m64_{doc,test,bench}/verify.sh 全绿
  （doc 18/18、test 12/12、bench 8/8）；pxlint BUILTINS 补 now_us/sleep_us。
- **README / spec §12 勾选同步**：pkg/ast/fmt/lint/test/doc/bench 七项自举已标注，
  lsp/mcp 留 M64d（按需）。

### M64 收尾 · 欠债清理（docs/M64_PLAN.md §14）✅

- **CI 质量门**：ci.yml 新增 `toolchain` job —— `pxc fmt --check`（selfhost+tools+stdlib
  39 文件收敛域）+ `pxc lint`（compiler.px 项目级主入口 + tools 11 独立文件 0/0）+
  m64_fmt/m64_lint verify；PLAN §5 承诺落地。
- **fmtlexer `${` 转义修复**：错误消息 `"字符串插值 ${ 未闭合"` 裸 `${` 会触发插值
  （错误路径求值未定义调用），改 `\${`；重建 bootstrap/pxfmt、pxdoc。
- **stdlib 6 文件 fmt 收敛**：collections/edge/gfx/png/semver/webroute 写回（净 -66 行，
  空行压缩+注释对齐，与 selfhost/tools 同规则）；capability 编译版 253 PASS 佐证无损。
- **pxlint 增强**：BUILTINS 白名单补 QUIC/H3 内建 32 名（capability L002 ×97 → 0）；
  支持 `type X const (...)`（M44 常量枚举）顶层名收集（LogLevel/Code 误报消除）；
  重建 bootstrap/pxlint；m64_lint verify 18/18 无回归。
- **L007 存量数据行 noqa**：compiler.px KEYWORDS/CTRL_ALL（653/371 字符语言强制单行）+
  capability 26 处长断言/extern 签名行尾 `# noqa` → compiler.px lint **0 错误 0 警告**、
  capability lint **0 错误**（剩 2 L001 演示形态 warning 留档 §14.3）。

### M63 · 语言面欠账修复（L8–L11 全清：pxi 网络 API / float 全精度 / pxc --version）✅

- **规划**：MINI_SUBSET §十三 欠账总结清 L8–L11；回归 examples/m63_langfix/verify.sh
  四线全绿（L8/L9/L10/L11 双模式对拍）。
- **L8 pxi 网络真实应用 API 补白名单**（interp.px + ibuiltin.px）：`http_post`/
  `http_request`/`s3_get`/`s3_put`/`s3_list`/`s3_delete` 进解释器 names 白名单 +
  `i_call_c_net` Result 透传 helper（C 网络失败 Err 不杀进程，M57 语义，双模式失败路径
  一致）；本地 mock HTTP 真请求双模式逐字节一致 + 失败 Err 透传 + 参数错报错退出。
  **http_get_stream 留档**（chunk_handler 宿主函数回调跨解释器边界，Mini 排除）。
- **L9 float→str 最短 roundtrip 全精度**（runtime.c `fmt_num`）：`%g` 6 位截断 →
  定点舒适区规则（十进制指数∈[-4,15) 内 `%.*f` 定点、区外 `%.*e` 科学，逐位 + strtod
  回读取最短 roundtrip；保持语言习惯 100000.0→"100000.0"、250.0→"250.0"、1e15→"1e+15"）；
  修复 0.1+0.2→"0.3"、1/3→"0.333333"、123456789.123→"1.23457e+08" 等精度丢失；`.0`
  补丁保留；双模式同根单点修复（m63_fp 16 断言逐字节一致）。
- **L10 编译期浮点字面量全精度**（codegen 零改动）：pxc 自举重建内嵌新 fmt_num →
  `cg_fmt_float` 的 `str(v)` 自动全精度 → C 产物 `px_float(3.141592653589793)`；
  v01_value float**（1.4142135623730951）编译/解释全 PASS → **diffcheck.sh 三处
  v01 %g 豁免移除**（差异即失败）。
- **L11 bootstrap/pxc --version**（compiler.px main 参数前置分支 + PXC_VER/PXC_MS）→
  **pxc 自举重建**（--no-quic）+ golden/compiler.c 同步（diff 仅 main +7/-1）；
  修复前 `--version` 被当文件读报错（RELEASE_PROCESS 已知边界勾除）。
- **验证**：verify.sh ALL OK；diffcheck --all/--errors 全绿（rc=0）；capability 双模式
  253/253 PASS；自举证明 B.c==golden/compiler.c；m59_math/m61_gfx/m62_langfix 回归 PASS。
- **文档**：MINI_SUBSET §七 #7/#8 勾除 + 新增 §十三.8 修复记录；spec §10.2 浮点打印
  全精度表述。

### M62 · 语言面欠账修复（L1/L5/L6/L7 + L2/L3/L4 处置）✅

- **规划**：MINI_SUBSET §十三 欠账总结（L1–L11）中清 L1–L7 可修硬欠账；
  回归 examples/m62_langfix/verify.sh（L1/L6/L7）+ verify_l5.sh（L5）双模式一致。
- **L1 浮点打印 `.0` 对齐**（commit `9acfb94`）：runtime.c `fmt_num` float 分支补 `.0`
  （整值有限 |f|<1e15 且 %g 无 `.eE`）→ `print(3.0)` 编译模式 `3`→`3.0`，与解释器 /
  Rust fmt_float 语义逐字节一致（str/插值/list/dict 内浮点全对齐）；6 位 %g 截断
  （0.1+0.2→0.3）保留为既定规避项。
- **L5 codegen 块作用域（变量提升 hoist）**：if/for/while 内 `var/let` 块外引用原编译报
  C undeclared（M-B2 留档"待 M 后补"）→ `cg_collect_hoist_vars` 统一收集 Assign 目标 +
  VarDecl + For 循环变量，函数顶 `px_null()` 预声明 + 原位赋值，对齐解释器/Python 函数级
  语义；**bootstrap/pxc 自举重建 + cases/compiler 的 C golden 全量更新**（hoist 结构变化，
  语义等价由 capability 253 PASS + 自举 B.c==golden + diffcheck --all/--errors 全绿证明）。
- **L6 split 保留空段回归**（commit `f41c529`）：自举 interp 重写后已保留空段（编译/解释
  双模式一致），补 fp_split.px 断言防回退。
- **L7 pxi bytes 族白名单补齐**（commit `ab598e0`）：interp.px names +14（bytes/bytes_len/
  bytes_get/bytes_set/bytes_slice/bytes_concat/bytes_to_str/bytes_to_hex/hex_to_bytes/
  bytes_find/bytes_base64/base64_to_bytes/base64_encode/base64_decode）+ ibuiltin 直调转发；
  bootstrap/pxi 重建；fp_bytes.px 17 断言双模式一致。
- **L2/L3/L4 处置（不改语义）**：`int(str)` 宽容前缀、`{}` 空 dict 字面量、import 模块顶层
  不执行 —— 均双模式一致的语义设计，破坏性收紧风险大于收益；文档保留警示与既有规避
  （MINI_SUBSET §十三.7），待真实需求再评估。
- **文档**：MINI_SUBSET §七 M-B2 两行待修标记 ✅、§十三 #7/§十三.4/.6 注记、新增 §十三.7
  修复记录；spec §10.2 浮点打印注记。

### 发布自动化 · tag 驱动 GitHub Release（仓库治理）

- **`.github/workflows/release.yml`（新）**：推送 tag `v*` 触发 → ubuntu-latest 上
  `tools/make_release.sh` 构建发布包（内置冒烟自检）→ `gh` CLI 创建 GitHub Release
  并上传 tarball（`GITHUB_TOKEN`，零第三方 action）；Release notes 自动含
  SHA-256 与上一 tag 起的提交列表。发布 SOP：`git tag v0.1.0-m62 &&
  git push origin v0.1.0-m62`。
- **`tools/make_release.sh` 版本来源 tag 驱动**：VER 取最近 tag 的 `vX.Y.Z` 部分
  （`v0.1.0-m62` → `0.1.0`），里程碑优先级改为 命令行参数 > tag 后缀 `m62` >
  最近提交推断，避免发布包版本与 tag 不一致。
- **`pxi --version` 补齐**：`selfhost/interp.px` 入口支持 `--version`/`-v`（无文件
  参数时输出版本退出 0，输出与 `tools/pxc --version` 对齐）；`bootstrap/pxi` 重建
  （对齐"所有程序支持 --version"规范）。

### M61 · 外部库 FFI proof（zlib）+ 纯语言 2D 游戏内圈（gfx/png/贪吃蛇）✅

- **规划**：`docs/M61_PLAN.md`（A=外部系统库绑定全链路 proof + B=纯语言 2D 内圈；无真板
  期游戏线 0→1 地基；用户选 A+B 并行）。
- **S0 zlib 两版静态 .a 入库**：`runtime/third_party/zlib/{include,lib,lib-aarch64}`，
  zlib 1.3.1 源码自编（x86_64 gcc / aarch64-linux-musl 交叉，ar 抽 crc32.o 双架构校验）；
  `tools/build_zlib.sh` 一键重建；`tools/pxc` 加 `--zlib-lib <dir>`（缺省按 `--cc` 自动
  架构探测：gcc→lib/，aarch64→lib-aarch64/，旧交叉脚本免改）+ 无条件链 libz.a（无引用
  不抽成员，向后兼容）—— commit `84fe8c8`。
- **S1 薄胶水 runtime_zlib.c**：`zlib_crc32(data)`（crc32(0,data,len)，已知值可校验）/
  `zlib_compress(data,level)`（compress2 + **uLongf\* 长度指针** cap→实际）/
  `zlib_uncompress(data)`（z_stream inflate **渐进扩容**免预知大小）；str/bytes 二进制安全、
  数据非法→null 不杀进程；注册进 FFI 表（px_register_zlib）双模式同构；pxc 链
  runtime_zlib.c；m61_zlib.px 七组断言（已知值 0x3610a686/标准 check 0xCBF43926/纯语言
  CRC32 查表 5 组互证/long roundtrip 10400→110B/NUL 安全/空串/非法流 null/level 0·6·9）
  + `nm` 实证 crc32/compress2/inflate 符号 + 4 例回归 PASS —— commit `1639c5f`。
- **S2 std.gfx + std.png（第 5/6 个 stdlib）**：gfx.px 画布 list[int] 0xRRGGBB +
  line(Bresenham)/rect/fill_rect/circle(中点)/fill_circle(弦扫描)/blit(透明跳过)/
  text(5x7 compact 字形，0-9 A-Z . - 空格，小写→大写)；png.px 纯语言 PNG 8bit RGB
  stored 编码器（CRC-32 查表 + ADLER-32 + zlib stored block + chunk 组装，零 FFI）；
  demo Mandelbrot 640x480（复数迭代 + 11 色调色板）+ 合成场景全原语；**python3 stdlib
  zlib 独立解码全校验**（chunk CRC 全过 + 像素颜色抽查全对）—— commit `171e59b`。
- **S3 raw 终端可玩贪吃蛇**（examples/m61_gfx/m61_snake.px）：w/a/s/d 控向 q 退出，
  O/#/@；try_step 纯函数 + spawn_food；交互走 tty_config(0,9600,raw)+fd_wait+read 单键
  （M60 设备组应用层 dogfood）；SNAKE_AUTO=1 无头剧本 EAT/SELF/WALL 三断言 +
  python3 PTY 真内核喂 q → QUIT；修正 tty_config 波特率 0 不支持（须 9600+ 枚举）；
  uinput/evdev stretch 留档 —— commit `2eb3060`。
- **S4 收口**：bootstrap/pxi 重建（9,293,144 B，含 runtime_zlib.c + libz.a → extern
  zlib_* 经 ffi_call C 桥双模式同能力，无需白名单/ibuiltin 分支）；m61_s4_zpxi.px pxi
  smoke（Mini 子集 int_to_bytes 构造 + crc32 守恒断言）解释==编译逐字节一致；
  m61_s4_det.px 纯整数绘制 PNG **x86 == qemu-aarch64 sha256 一致**（605c5b07…）；
  m61_s4_zpng.px **FFI compress2 直接产出标准压缩 PNG**（python 独立解码合法 +
  像素抽查）；m61_s4_impsmoke.px 探针：pxi 可解释 std.gfx 纯 list 路径、text/blit 等
  bytes 依赖仍受限；回归 hello/fib pxi + math_s1 编译 + dev_s1 --no-quic 编译 ——
  commit `ab29185`。
- **S5 文档收口**：spec §8.19（FFI 外部库约定 + gfx/png 库）+ §10 表 + MINI_SUBSET
  §十三.6（extern 双模式零成本 / pxi bytes 族缺口 / stdlib import 边界复核 / 性能
  dogfood 14-34s 每帧 → bytes 画布方向）+ ROADMAP/GAP 勾选 + CHANGELOG。
- **边界如实记录**：SDL2/raylib 真窗口结论留档（无屏，M61-PLAN §4）；QQ 富媒体发送被
  平台拒（err 40093007 下载失败），图片落盘 examples/m61_gfx/*.png 供自取；
  pxi stdlib 完整能力仍主打编译模式；性能（640x480 逐像素 list+concat 14-34s/帧）
  优化方向留档 bytes 画布。


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
