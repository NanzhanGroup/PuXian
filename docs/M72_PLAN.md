# M72_PLAN · AI 调试回路 + runtime bytes 增强（Issue 9/10 + Issue 13-R1）

> 创建：2026-09-06 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ a5e5731，M71 闭环 · v0.1.0-m71 已发布 · CI run 109 全绿）
> 来源：qg-issue **09-print-buffering**（print 全缓冲）+ **10-ai-native-diagnostics**（诊断 D1/D2）+ **13-puxian-backup**（GAP-BIN-1/2，R1 runtime bytes）
> 用户指令（2026-09-06）：东月给出切分方案（A：M72 = Issue 9 + 10 + 13-R1 一批 runtime，一次自举重建共享成本；13-R2 ws-backup-px → M73）→ 用户「按 A 立项 M72 开工」
> 目标：① print/println 管道/重定向下**实时可见**（Issue 9 三现象根治）② **编译产物运行时错误带源行号** + spawn 协程错误默认隔离、崩溃有现场（Issue 10 验收 1/2/3）
>       ③ runtime **bytes native**：AES-GCM bytes 版（去 utf8/NUL 限制）+ HTTP body bytes 长度感知（Issue 13-R1，ws-backup PuXian 化前置）
> 性质：**L0 runtime + selfhost 编译器，不改语言语义**（错误/日志出口增强 + 二进制安全 native 增量）→ 需重链全部 bootstrap ELF + 自举证明 + 全量回归（M68/M70 同款大回归链）

## 〇、现状侦查（2026-09-06 源码 + 实测实录，非记忆推断）

**Issue 9（print 全缓冲）—— runtime.c stdio 层**
- `bi_print`（runtime.c **L2622**）：循环 `px_print_value` printf + 末尾 `printf("\n")`，**无 fflush(stdout)**；
  `px_print_value`（**L1773**）纯 printf。stdout 重定向到管道/journald/文件 → C stdio 全缓冲（8192B）→ 攒满或 exit 才刷。
- runtime 已知此坑（L1554 注释自述），但只在 `px_error`（L1553，exit 前）与 `bi_input` 提示前补刷——**常规 print 从不刷**。
- stderr 默认无缓冲 → 错误输出本身实时（问题只在 stdout 业务日志）。

**Issue 10 D1（错误行号）—— 实测三路径现状（用 v0.1.0-m71 bootstrap 实测）**
| 路径 | 坏输入样例 | 实测输出 | 行号？ |
|---|---|---|---|
| pxi 语法错误 | `let x = [1, 2` 缺 `]` | `3:5: 语法错误 E2001: 期望 ']'，实际得到 标识符 print` | ✅ 已带（parser.px `perr` L168 = `pline():pcol()`） |
| pxi 运行时错误 | `len(null)` | `运行时错误: 错误 [R1002] 3:16: len 不支持类型 null` | ✅ 已带（i_err.px line:col） |
| **pxc build 产物运行** | `len(null)` | `运行时错误: len 不支持类型 null` | ❌ **无行号** |

- ⇒ D1 真正缺口收敛为一条：**编译产物（pxc build）运行时错误无源行号**（C runtime `px_error` 无 .px 位置信息）。pxi 解释器路径已满足。
- 编译产物 = selfhost/*.px 经 `bootstrap/pxc`（cg_*.px）生成 C + gcc 静态链接 runtime.c。行号信息**在生成时是有的**（AST 语句带 [line,col]，codegen 遍历语句时可见）——只是没打进 C。

**Issue 10 D2（崩溃现场 + spawn 隔离）**
- `px_error`（L1553）= `fprintf(stderr) + exit(1)` → 任何运行时错误**无条件带崩整个进程**（含 spawn 协程内）。
- spawn = C pthread（L6615 `spawn_thread` / L6687 `px_spawn`）：注册 GC 槽位 → `job->fn(args,nargs)` → 注销退出。`fn` 内 px_error → exit(1) 全进程死。**Mahesvara md worker 崩溃即此路径**（pxi 解释器不支持 spawn——istmt.px L110「interp 不支持 spawn（Mini 子集排除）」→ 生产 spawn 程序必然是编译产物）。
- GC 并发模型（M11 stop-the-world，L564-582）：spawn 活跃时 GC 暂停所有注册线程并保守扫栈——线程退出须走 gc_unregister_thread 注销路径（spawn_thread 已有），隔离方案必须兼容此注销协议。
- C 侧 glibc 已有 execinfo.h 条件包含（M57-S4）→ backtrace 可用（但 C 栈对 AI 定位 .px 代码价值有限，**真正的现场 = S2 的 .px 源行号 + 函数名**）。

**Issue 13 GAP-BIN-1/2（runtime_aes.c + runtime.c http）**
- runtime_aes.c（211 行）：`aes_encrypt/decrypt`（CBC）与 `aes_gcm_encrypt/decrypt`（GCM）取参全走 `vstr()`（PX_STR 且按 C 字符串，**含 \0 截断**）；解密后 **aes_is_utf8 强制校验，非 UTF-8 → null**；输出 hex 字符串。二进制明文加密不了、密文解密不出。
- runtime.c http：`bi_http_request`（L7790）body 仅收 `PX_STR`（`if (args[2].type == PX_STR)`），Content-Length 用 `strlen(body)`（L7840 区）→ **bytes 类型被忽略、\0 截断**。响应读回 str。
- **PX_BYTES 类型已存在**（runtime.h L26，M23b：带长度可含 NUL，union 复用 str 的 data/len；mmap 支持 L60）；.px 语言层 bytes 家族齐全（ibuiltin：read_bytes/write_bytes/int_to_bytes/hex_to_bytes/bytes_concat/bytes_slice/base64…）→ 缺的只是 **aes/http 两个 native 不认 bytes**。
- mbedtls 已就绪（gcm_crypt_and_tag / auth_decrypt 直接吃 data+len，无文本假设）→ 改动面小。

**重建链（M68/M70 先例，自举架构）**
- `bootstrap/pxc`（编译器 ELF）编 selfhost/*.px → C → tools/pxc build（gcc 静态链接 runtime*.c）→ ELF。
- 改 **runtime*.c** → 重链受影响 bootstrap ELF（pxi/pxc/pxfmt/pxlint/pxlsp/pxmcp/…）并**提交新 ELF**（git 跟踪）。
- 改 **selfhost/*.px**（parser/cg/interp…）→ `selfhost/bootstrap_prove.sh`（自举证明：pxc 编 compiler.px 产物 B.c 与 golden/compiler.c 逐字节一致）+ 重链全部 bootstrap ELF。
- 回归闸：diffcheck.sh（golden 对拍）+ examples/*/verify + toolchain verify（M65-M71）+ CI 六 job + native_index 防漂移（**新增 native 必同步** docs/native_index.json + ECOSYSTEM/CHEATSHEET，gen_native_table.sh 排序修复 M70-CI-FIX 已就位）。

## 一、范围与边界

**做（S1–S5）**：
- **S1 · print 即时可见（Issue 9）**：`bi_print` 行尾 `fflush(stdout)`（方案 A，行级实时，量大可接受）；新增 **`flush()`** 原语（fflush(stdout)，方案 C 最小版）；新增 **`print_err(...)`** 原语（stderr 输出，服务错误/诊断出口；为 S2 parser 错误出口统一 stderr 铺路）。纯 stdio 行为，现有 .px **无需改码**即实时。
- **S2 · 编译产物运行时错误源行号（Issue 10 D1）**：selfhost codegen（cg_stmt.px 语句遍历点）统一为每条可执行语句生成 C 行号标记（`px_srcline(<line>)`，全局 int store，开销极微）；runtime `px_error` 打印携带当前源行 + 当前函数名（cg 函数入口标记）。pxi 路径已带行号 → 验收「编译+运行错误全带行号」达成。**不动**解释器错误格式（对拍 golden 已锁定）。
- **S3 · 崩溃现场 + spawn 隔离（Issue 10 D2）**：`px_error` 现场增强（错误消息 + 源行/函数(S2) + [spawn] 线程标记 + glibc backtrace 摘要）；**spawn 协程错误默认隔离**（spawn_thread 入口设错误捕获点，协程内 px_error → 打印现场 → 走 GC 注销路径安全退出线程，宿主进程不崩）；环境变量 `PX_SPAWN_ISOLATE=0` 可关（回退 exit 行为，向后兼容）。
- **S4 · runtime bytes native（Issue 13-R1）**：GAP-BIN-1 `aes_gcm_encrypt_bytes/decrypt_bytes`（data/key/iv 收 str|bytes 均不截断；输出/输入 **bytes** = 密文||tag 原始字节；**去 aes_is_utf8**；旧 hex 文本版保留兼容）；GAP-BIN-2 http_request **body 收 PX_BYTES**（Content-Length = bytes.len 长度感知）+ http_serve 请求 body 对 bytes 的接收语义（与 ws-backup R2 需求对表，见风险）。CBC bytes 版同模式顺带（低成本防同坑）。
- **S5 · 重建链 + 全量回归 + 文档收口 + qg-issue 归档**：重链全部 bootstrap ELF + 自举证明 + diffcheck + 全量 verify；ECOSYSTEM_GAPS 增补（G5 日志可观测性/诊断 + Issue 13 bytes 增强记录）；ROADMAP/CHANGELOG/README/CHEATSHEET/native_index 同步；Issue 9/10 ISSUE.md 状态 ✅ → 归档 `done/`；Issue 13 ISSUE.md 更新 R1 ✅ M72（R2 → M73 待办，目录暂留）；tag `v0.1.0-m72` 发布。

**不做**：
- ❌ Issue 13-R2 ws-backup-px 业务重写 → **M73**（纯 .px 业务，不碰 runtime，等 R1 落地后开工）。
- ❌ 语言语义/parser 语法改动、断点/单步/watch 等人类调试器（违背 AI 优先定位）。
- ❌ pxi 解释器错误格式改动（golden 锁定）；解释器加速（字节码 VM，远期正道）。
- ❌ 字节码 VM、spawn 子进程化、.px 层 try/catch（Result/Option 唯一错误通道哲学不变）。
- ❌ 不开 PR（维持既定边界，直接 commit + push origin main）。

## 二、分步计划

### S1 · print 即时可见（runtime.c stdio，Issue 9）
- `bi_print`（L2622）末尾加 `fflush(stdout)`。
- 新 `bi_flush`（1 参 0 → `fflush(stdout)`，注册全局 `flush`）；新 `bi_print_err`（同 bi_print 但输出 stderr，注册全局 `print_err`）。
- **pxi/selfhost 侧**：ibuiltin.px `i_register_builtins` 名单加 `flush`/`print_err`（pxi 解释器可用；编译产物直接 px_set_global 即可）。
- 注册点：runtime.c `px_register_*`（L5169 print 邻近）加三行；native_index.json 同步（+2 native：flush/print_err——print_err 算 native）。
- 重建：重链 bootstrap/pxi（验证 .px 层 flush/print_err 可用）+ 任一编译产物。
- 验证：`for i in 1..1000 print(i)` 管道下 `tail -f` 实时逐行（修复前攒 8KB）；kill -9 末行不丢；flush/print_err 输出定向正确；现有 verify 零漂移（print 行尾 flush 不改输出内容）。

### S2 · 编译产物运行时错误源行号（selfhost cg + runtime px_error，Issue 10 D1）
- selfhost codegen：找 cg_stmt.px 生成函数体的**语句遍历点**，统一在每个可执行语句前 emit `px_srcline(<stmt line>);`（一处插入覆盖全语句面；expr 不插——语句级定位足够）。函数入口另 emit `px_srcfunc("<函数名>", <line>);`（或并入）。
- runtime.c：全局 `int g_px_src_line; const char* g_px_src_func;` + `px_srcline/px_srcfunc` 赋值函数；`px_error` 打印：有行号 → `运行时错误 [<func>@行<line>]: <msg>`；无（初始化期）→ 原样。print_err 输出错误（stderr 已有）。
- 注意：cg 产物变化 → golden/compiler.c 同步更新（自举证明新基准）+ 全量 bootstrap ELF 重链。
- 验证：编译产物 `len(null)` 坏程序 → `运行时错误 [main@行3]: len 不支持类型 null`（LLM 一次定位）；语法错误（pxi）保持现状行号；examples verify 全绿（正常路径零输出变化）。

### S3 · 崩溃现场 + spawn 隔离（runtime.c 错误边界，Issue 10 D2）
- 错误边界：线程局部 `jmp_buf g_px_errjmp; volatile int g_px_errjmp_set;`。`px_error` 改造：打印现场（含 S2 行号/函数 + 当前线程标识 spawn/main）→ 若 `g_px_errjmp_set` 且非主线程 → `longjmp`；否则 `exit(1)`。
- spawn_thread（L6623）：注册 GC 槽位后、调 `fn` 前 `setjmp` 捕获点；`longjmp` 回来 → 打印 `[px-spawn] 协程崩溃: <现场>` 标记 → **走原注销路径**（持锁 g_gc_mu 减活跃 + gc_unregister_thread）退出线程。主线程 spawn 入口也要考虑（spawn 里再 spawn？只给非主线程设边界）。
- 可配置：读 env `PX_SPAWN_ISOLATE`（默认 1=隔离；0=px_error 原 exit 语义，向后兼容）。
- 风险（PLAN 显式记录）：longjmp 不展开 pthread 锁——若崩溃点在 mutex/rwlock 临界区（mutex.with 内裸错误），锁遗留 → 其它线程等锁卡死。缓解：文档明确「临界区内用 Result 收敛错误，运行时错误（bug）隔离不保证锁安全」+ 默认隔离对**无锁持有时**完全安全（绝大多数崩溃场景）。执行期实测 Mutex 场景，若冲突不可解 → 降级为「仅现场增强 + PX_SPAWN_ISOLATE 文档化」并在 PLAN 附录记录（验收 2 以隔离为主路径，如实汇报边界）。
- 验证：spawn 协程 `len(null)` → 宿主继续运行（打印 [px-spawn] 崩溃现场 + 后续 main 代码执行）；非 spawn 顶层错误仍 exit 带现场；PX_SPAWN_ISOLATE=0 回退原行为。

### S4 · runtime bytes native（runtime_aes.c + runtime.c http，Issue 13-R1）
- runtime_aes.c：新增取参助手 `vbytes()`（str|bytes 均收，返回 data+len，不截断 NUL）：
  - `aes_gcm_encrypt_bytes(data, key, iv) → bytes`：mbedtls_gcm_crypt_and_tag，输出 **bytes** = 密文||tag(16) 原始字节；
  - `aes_gcm_decrypt_bytes(ct, key, iv) → bytes|null`：mbedtls_gcm_auth_decrypt，tag 失败 null；**去掉 aes_is_utf8**；
  - 旧 `aes_gcm_encrypt/decrypt`（hex 文本版）与 CBC `aes_encrypt/decrypt` **原样保留**（兼容既有调用方）；
  - 顺带 CBC bytes 版 `aes_encrypt_bytes/decrypt_bytes`（同模式去 utf8，防同坑）——按体量评估是否并入本 S（建议并入，同一批回归）。
  - 注册：runtime.c 或 aes 注册区 px_set_global 加 2(+2) native；native_index.json + ECOSYSTEM/CHEATSHEET 同步。
- runtime.c http（GAP-BIN-2）：
  - `bi_http_request`（L7790）：body 收 PX_BYTES（+现有 PX_STR）；Content-Length 用 bytes.len（`px_http_request` 签名 body 已带 `out_len`？核实后把 strlen 换长度感知发送）；px_http_request（L8795）body 发送用 len 感知。
  - http_serve 请求 body 二进制接收语义：执行期与 ws-backup R2 对表（见风险 R7），优先保证「密文 body 能被 serve 收到且字节不变」。
- 验证（Issue 13 R1 验收）：PuXian bytes GCM 加密**任意二进制**（含 \0/非 UTF-8，如 runtime.c 头 8KB）→ Go ref（/data/code/ws-backup/ref-go crypto.go）解密**逐字节一致**；反向亦然（需 Go helper 或对拍脚本，S4 期建 examples/m72_* verify + ref 对照）；http_request bytes body 上传 → 本地 http_serve 接收字节一致。

### S5 · 重建链 + 全量回归 + 文档 + 归档
- 全量重建：selfhost/bootstrap_prove.sh（--fresh，覆盖 S2 cg 改动自举证明）→ 重链全部 bootstrap ELF（pxi/pxc/pxfmt/pxlint/pxlsp/pxmcp/pxcheck/pxbench/pxslice/routegen/pxdoc/pxtest 依实际依赖）→ 提交新 ELF。
- 回归总闸（见三）。
- 文档：ECOSYSTEM_GAPS 增补 G5（日志可观测性：print 实时/flush/print_err/错误行号/spawn 隔离；Issue 13 bytes 增强记录）；PUXIAN_CHEATSHEET（flush/print_err/aes_gcm_*_bytes/http bytes body + 诊断注记）；ROADMAP 主线 M72 行 + 能力基线；CHANGELOG [Unreleased] M72 条目；README(.en) 能力/日志章节；native_index 防漂移（S4 已同步）；tools/pxc usage 头注释（如有诊断 env 说明）。
- qg-issue：09/10 ISSUE.md 头部状态改 ✅（M72）→ mv done/；13 ISSUE.md 更新（R1 ✅ M72 细节 + R2 仍 M73 待办）目录暂留根（R2 未完）；00-README 总览表/历史段同步。
- 发布：tag v0.1.0-m72 → CI 自动发布（make_release 冒烟覆盖）+ 本机留档 + 发布指引更新。

## 三、回归总闸（判据）
1. CI 六 job 全绿：regression（自举证明 + diffcheck 全量）+ examples + toolchain（fmt/lint/**生态+原生索引防漂移**/M65-M71 verify）+ multiarch 三架构交叉。
2. S1：管道下 print 实时逐行 + kill -9 末行不丢 + flush/print_err 定向正确；现有输出 golden 零漂移。
3. S2：编译产物运行时错误带源行号 `[func@行N]`；pxi 语法/运行错误行号现状保持；正常路径输出零变化。
4. S3：spawn 协程 `len(null)` → 宿主存活 + [px-spawn] 崩溃现场；PX_SPAWN_ISOLATE=0 回退 exit；顶层错误 exit 带现场。
5. S4：bytes GCM ↔ Go ref 双向逐字节一致（examples/m72_* verify + ref 对拍）；http bytes body 上传接收字节一致；旧 hex 文本 aes 兼容零回归。
6. fmt/lint 0 错；bash -n 通过；CHANGELOG/ROADMAP/README/ECOSYSTEM/CHEATSHEET/native_index/qg-issue 收口 commit。

## 四、验收方法（对齐各 ISSUE 五节）
1. **Issue 9 ①**：任意 .px `1..1000 print` 管道/supervisor 托管 → 修复后运行中实时逐行可见（tail -f / journalctl -f），程序不退出也可见。**②** kill -9 前末行不丢。**③** Mahesvara（编译产物，supervisor 托管）watch 构建日志实时可见（S1 后编译产物重发即生效）。
2. **Issue 10 ①**：编译产物坏输入（`len(null)`）→ stderr 给出 `[函数@源行]` 现场 → LLM 一次定位。**②** spawn 协程运行时错误不再无条件带走进程（默认隔离，宿主继续 + 现场打印）。**③** 编译错误（pxi 语法，已有行号）+ 运行错误（pxi 已有 / 编译产物 S2 新增）全部带行号。
3. **Issue 13 R1**：PuXian `aes_gcm_encrypt_bytes` 加密任意二进制 → Go ref 解密逐字节一致（反向亦然）；`http_request` bytes body 上传接收一致。（R2 ws-backup-px 属 M73，本里程碑只交付 R1 能力。）

## 五、风险与取舍
- **S3 longjmp 与持锁**：隔离默认开启但崩溃点若持 pthread 锁 → 锁遗留（文档限制 + 实测 Mutex 场景；不可解则降级隔离为可选项，如实汇报边界——见 S3 风险段）。
- **S2 cg 插桩影响面**：cg 产物变化 → golden/compiler.c 新基准 + 全部 bootstrap ELF 重链 + diffcheck 全量（自举回归链成本高但一次共享，覆盖 S1-S4 所有 runtime/selfhost 改动）。
- **S4 bytes 语义边界**：.px str 可含 NUL（px_str_len data+len），但 native 取参 vstr 截断——bytes 新 native 一律 vbytes 长度感知；http_serve 二进制 body 暴露形态（str 含 NUL vs bytes）执行期与 ws-backup R2 需求对表定，避免为未用形态过度设计。
- **native_index 防漂移**：S1 flush/print_err + S4 aes_gcm_*_bytes/http bytes 相关 → 新增 native 全量同步索引（gen_native_table.sh 字节序已修，M70-CI-FIX 保证 CI 不假红）。
- 里程碑边界：M72 交付「运行时能力 + 诊断」，ws-backup 生产 PuXian 化在 M73（R1 已为其铺路）；Issue 13 目录 M72 不归档（R2 未完成）。
