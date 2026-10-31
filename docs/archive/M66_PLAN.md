# M66_PLAN —— 自举 wsAgent runtime 原语补全 + stdlib 收编（qg-issue 01–06 全量合入）

> 状态：✅ 立项定稿（2026-09-05，决策已拍板）→ **S1/S2/S3 已提交，S4/S5 收口进行中，S6 待办**
> 官方基线：github.com/NanzhanGroup/PuXian 本地仓库 HEAD = `6df05ef`（M66-S3，工作树含 S4 lunar 未提交）
> 来源：清歌（qingge）qg-issue 01–06 + 00-README 索引（2026-09-05 更新）
> 关联：docs/M65_PLAN.md（上一里程碑，spec §12 工具链全自举收官）、docs/spec.md、docs/ROADMAP.md

## 0. 决策定稿（用户拍板 2026-09-05）

| # | 决策 | 定稿 |
|---|---|---|
| **D0** | 清歌提交时 M65 未完工、未拉取最新文档 → 以**官方仓库现状与官网计划为准**（不迁就 ISSUE 内基于旧基线/旧文档的认知，如"os_exec 官方 M23 已有"系清歌本地发布包自加、官方从未有） | ✅ 按官网计划执行 |
| **D1** | os_capture 与既有 os_spawn_capture 关系 | ✅ **A 并存**：新增 `os_capture(cmd,args)→{rc,stdout,stderr}`（stderr 分离、双管道）；`os_spawn_capture`（合并输出）保留为正式 API 不动（M65 pxmcp/pxcheck 在用，零回归） |
| **D2** | zip_unpack 密码算法 | ✅ **zipcrypto 与 AES-256 全实现**（不自限经典）：miniz 本地头解析 + zipcrypto 传统解密 + WinZip AES-256（PBKDF2-HMAC-SHA1 + AES-CTR + HMAC-SHA1 认证，AE-2） |
| **D3** | 06 T3 农历 | ✅ **纳入 M66**：新增 L1 stdlib `stdlib/lunar.px`（`std.lunar`，第 9 个标准库），内嵌 1900-2100 农历数据表 + 公历/农历互换算（纯 .px） |
| **D4** | M66 命名/编号 | ✅ **M66：自举 wsAgent runtime 原语补全 + stdlib 收编**（README 原生开发表扩 M41–M66） |

> 命名说明：M66 合入的原语（unix_connect/os_capture/os_popen/os_kill group/os_exec/zip 密码…）正是
> 清歌 ws-launcher-px / ws-core / ws-install / ws-todo 这些 **wsAgent 生态模块**被卡的原语缺口，
> 故命名"自举 wsAgent"。

## 1. qg-issue 全景（2026-09-05 重新阅读 /data/qg-issue/ 全量）

| # | 目录 | 标题 | 建议层 | 优先级 | 交付物 |
|---|---|---|---|---|---|
| 1 | 01-unix-socket-raw-connect | AF_UNIX 裸连接 `unix_connect`（http_unix 只讲 HTTP） | **L0 runtime** | P0 | ISSUE.md |
| 2 | 02-os-fs-trio | 五 native：os_exec / os_rename / os_remove_all / os_random_hex / os_file_sha256（**官方均缺失已核实**） | **L0 runtime** | P0 | ISSUE + procfs.patch + patch_osfs.py + os_fs_test.px(16断言) + exec_demo.px |
| 3 | 03-yaml-stdlib | YAML 子集解析器收编 `std.yaml` | **L1 stdlib** | P1 | ISSUE + yaml.px(421行) + yaml_test.px(35断言) |
| 4 | 04-pxml-config | PXML 原生配置格式：规范→docs/ + 解析器收编 `std.pxml` | 规范 docs/ + **L1 stdlib** | P1 | ISSUE + CONFIG-FORMAT.md + pxml.px(465行) + pxml_test.px(68断言) + dogfood/ |
| 5 | 05-lang-gaps-ws-install | ws-install 调研缺口 G1–G7 | 混合（L0 为主） | P0/P1/P2 | ISSUE.md（纯调研） |
| 6 | 06-ws-todo-survey | ws-todo 调研缺口 T1–T7 | 混合 | P0 blocker | ISSUE.md（纯调研） |

### 缺口语义汇总（M66 合入）
| native | 语义 | 失败行为 | 来源 |
|---|---|---|---|
| `unix_connect(path) → fd` | socket(AF_UNIX,STREAM)+connect → fd（复用 M57 read/write 收发） | px_error 或 -1 + os_errno | Issue 1 / 06 T1 |
| `os_exec(cmd, args?)` | execvp 进程替换，**成功不返回**（PID 不变，launcher 语义） | px_error 退出 | Issue 2 |
| `os_rename(old, new) → bool` | rename(2) 原子改名/覆盖 | false + os_errno | Issue 2 |
| `os_remove_all(path) → bool` | 递归删文件/目录树（符号链接 unlink）；**空串与 `/`、`//` 拒绝防删根** | false + os_errno | Issue 2 |
| `os_random_hex(n) → str` | /dev/urandom n 字节 → 2n 小写 hex（1..1024） | null | Issue 2 / 05 G7 |
| `os_file_sha256(path) → str` | 文件内容 sha256 → 64 小写 hex（mbedtls） | null | Issue 2 |
| `os_capture(cmd, args…) → {rc:int, stdout:str, stderr:str}` | 子进程**双管道**分离捕获（D1=A，与 os_spawn_capture 并存） | rc 约定同上 | 05 G1 / 06 T2 |
| `os_popen(cmd, args…) → {stdin_fd, stdout_fd}` | 双向管道（对话式进程 stdin 注入+stdout 读回） | — | 05 G4 / 06 T2 |
| `os_kill(pid, sig, group:bool)` | group=true → `kill(-pid, sig)` 清进程组 | bool | 05 G5 / 06 T2 |
| `write_file/append_file(path, content, mode?)` | fopen → `open(O_WRONLY|CREAT|TRUNC, mode)`（写 0600 密钥免 chmod） | px_error | 05 G3 |
| `zip_unpack(path, out, password?)` | 密码 zip：**zipcrypto + AES-256 双算法**（D2） | 见 D2 | 05 G2 |

## 2. 官方现状核对（实测，2026-09-05，/data/code/puxian @ ae52674）

对 `runtime/runtime.c` grep 核对（qingge 报告全部属实，且比报告更进一步）：
- `unix_connect`/`os_exec`/`os_rename`/`os_remove_all`/`os_random_hex`/`os_file_sha256`/`os_capture`/`os_popen`：**0 命中，官方缺失**；
- `os_kill` 仅 (pid,sig) 两参（decl/impl/reg 各 1 处），无 group；
- `write_file` 现为 `fopen(path,"wb")`（无 mode），`append_file` 同理 `"ab"`；
- `zip_unpack` 仅 (path,out)（miniz mz_zip_reader，注册行 5305），无密码位；
- 已有 **`os_spawn_capture(cmd,args)→[rc,out]`**（M65-S1 我方加，stdout+stderr **合并**捕获，单管道）——D1=A 保留；
- pxi 解释器：`selfhost/interp.px` 第 62 行 `names=[...]` 白名单（已有 os_spawn_capture）+ `selfhost/ibuiltin.px` dispatch（os_spawn_capture 有 if 分支转发）→ **M66 新增 native 须两处同步**（M65-S1 先例：runtime +82 行、interp.px +2、ibuiltin.px +3）；
- `tools/pxlint.px` BUILTINS 内建名单须补新 native（否则 tools/examples 用 os_* 会 lint 误报未定义）；
- **Issue 2 procfs.patch 在当前树 git apply 失败**（锚点漂移：runtime.c:219 因 M65-S1 插入 os_spawn_capture decl 失配）→ **改用 patch_osfs.py 幂等脚本，已在 M65-S5 runtime.c 副本实测 `[OK] 已插入 5 native`**（rc=0，注册行 5 处验证通过）；
- stdlib 现有 6 个（collections/edge/gfx/png/semver/webroute），文件头风格已阅（webroute.px）；`import std.edge` 先例确认顶层 def 直接可见；
- M65 回归基线：capability 双模式 253 PASS、diffcheck --all（selfhost/diffcheck.sh）、m65_lsp/m65_mcp verify ALL PASS。

## 3. 目标与范围

**目标**：qg-issue 01–06 全量合入官方仓库，把 PuXian 打造成"ws-* 系统编程可写"状态：
1. **L0 runtime**：11 项 native 补全/扩展（unix_connect + os 五件套 + os_capture + os_popen + os_kill group + write_file/append_file mode + zip_unpack password 双算法），pxi 白名单/pxlint BUILTINS 同步，重建 bootstrap/pxi；
2. **L1 stdlib**：收编第 7/8/9 个标准库 `stdlib/yaml.px`、`stdlib/pxml.px`、`stdlib/lunar.px`；PXML 规范进 `docs/PXML.md`；
3. **生态收口**：spec.md / README / CHANGELOG / ROADMAP / CI / make_release 同步；qg-issue 各 ISSUE.md 回写"已合入 M66"结论；05 §7 / 06 §7 的 ws-install / ws-todo 重写 checklist 勾除可写性缺口。

**明确不做（边界）**：
- 不写 ws-install / ws-todo 业务本体（其重写为 M67+ 业务里程碑，待本里程碑合入后按 checklist 另立项）；
- 06 T4 date_add / T5 CLI flag / T7 rune 截断：ws-todo 重写时 .px 手写绕行项（官方已评估 P2/P3，不进 M66）；
- 06 T6 时区 IANA：文档注明即可；
- Issue 1 可选增强 `unix_listen`：不做，保持最小；
- 05 G6 which：不单独造原语，作为 `os_capture("command","-v",x)` 标准用法示例进文档（随 G1 消失）；
- 不动 parser/compiler/codegen/lint 语义 → **无需自举证明 B.c==golden**（compiler 未动）。

## 4. 里程碑与验收（S 步，平铺编号）

### M66-S1：L0 runtime 原语补全（一次 pxi 重建，避免重复 10min+ 长编译）
**合入内容（按区手工 + 脚本，全程对 M65-S5 runtime.c 副本演练后再落工作树）**：
1. 进程/文件组（Issue 2 五件套 + 05 G3/G5）：
   - `patch_osfs.py runtime/runtime.c`（已实测 [OK]，幂等：已含跳过、锚点异常报错不写）插入 os_exec/os_rename/os_remove_all/os_random_hex/os_file_sha256；
   - `os_kill(pid,sig,group:bool)`：第三参 true → `kill(-pid,sig)`（保留两参旧调用兼容）；
   - `write_file`/`append_file` 第三参 mode：fopen → `open(path, flags, mode)`（不传保持 0666&umask 现状）。
2. 网络/管道/zip 组（Issue 1 + 05 G1/G2/G4）：
   - `unix_connect(path)→fd`：仿 tcp_connect sockaddr 构建 + M57 fd 风格；
   - `os_capture(cmd,args)→{rc,stdout,stderr}`（D1=A：双管道分离捕获新实现）；
   - `os_popen(cmd,args)→{stdin_fd,stdout_fd}`（pipe×2 + fork/exec，双向）；
   - `zip_unpack(path,out,password?)`：第三参可选；**zipcrypto + WinZip AES-256 双算法**（D2：读本地文件头，密码 zip 条目走自定义解密路径，非密码 zip 走 miniz 原路径）。
3. pxi 白名单同步：`selfhost/interp.px` names 数组 + `selfhost/ibuiltin.px` dispatch（os_* 系转发分支）；`tools/pxlint.px` BUILTINS 补新 native 名。
4. 重建：`pxc build selfhost/interp.px`（>10min、RSS ~2.5GB，background + 日志）+ 覆盖 bootstrap/pxi；pxlint 重建。
**专项验证（真实执行）**：
- qingge 交付 `os_fs_test.px`（16 断言）编译模式 PASS；`exec_demo.px` os_exec 进程替换（PID 不变、/bin/echo 输出透传）；
- 新增 `examples/m66_proc/`：os_capture（stdout/stderr 分离断言）、os_popen（echo 回显双向）、os_kill group、write_file mode（0600 stat 验证）、unix_connect（本地 unix sock 行协议实测 + 失败路径 os_errno）、zip_unpack 密码（zipcrypto 与 AES-256 各构造密码 zip 解包断言）双模式；
- **回归**：capability 双模式 253 PASS/0 FAIL + diffcheck --all rc=0（selfhost/diffcheck.sh）+ m65_lsp/m65_mcp verify 复跑 ALL PASS。

### M66-S2：L1 stdlib 收编 std.yaml（Issue 3）
- `yaml.px` → `stdlib/yaml.px`：文件头按 stdlib 风格改写（用途/支持子集/不支持项/入口 yaml_parse 说明，参照 webroute.px）；**纯函数无 import 依赖，内容零改动优先**（若因模块级 var 不可读等语言坑需适配则最小化）；
- `yaml_test.px` → `examples/m66_yaml/yaml_test.px` + verify.sh（双模式）；
- 验收：pxi run + pxc build 双模式各 **35 PASS** 且输出一致；fmt --check + lint 全绿；capability 253 无回归。

### M66-S3：L1 stdlib 收编 std.pxml + PXML 规范 docs/（Issue 4）
- `pxml.px` → `stdlib/pxml.px`（加密能力 aes_gcm/base64/hex 依赖 build native → 文件头注明**主打编译模式**，参照 std.gfx 先例）；dogfood/ 迁 `examples/m66_pxml/`（+ verify.sh）；
- PXML 规范：CONFIG-FORMAT.md 主体 → `docs/PXML.md`（规范 v0.6：语法+EBNF/enc 策略/决策表/语言缺口笔记 §5.6/Dogfood 记录 §5.7 保留）；
- 验收：pxi run + pxc build 双模式各 **68 PASS**；dogfood demo 16 断言 + demo_enc 4 断言（PXML_MASTER_KEY env，不落盘）；fmt/lint 全绿。

### M66-S4：L1 stdlib 收编 std.lunar + 农历算法（06 T3，D3 拍板）
- 新建 `stdlib/lunar.px`（`std.lunar`）：**内嵌 1900-2100 农历数据表** + 纯 .px 换算算法；
- 功能面：`lunar_date(ts) → {year,month,day,leap,is_leap_month,gz_year,gz_month,gz_day}`（公历→农历）、
  `lunar_to_solar(y,m,d,leap) → ts`（农历→公历，含闰月参数）、`lunar_md_in_year(m,d,leap?,year) → ts`（ws-todo `lunar:8-8` 场景：某公历年该农历月日落在哪一天）；
- 数据表来源：1900-2100 农历（闰月 + 每月大小）公开数据表（对齐 6tail/lunar-go 同源数据 1900-2100，逐年核对），以紧凑字串/数组形式内嵌；
- 测试 `examples/m66_lunar/`：与公历已知锚点对拍（2026 春节=2026-02-17、2024 春节=2024-02-10 等 + 闰月样例 + lunar:8-8 于 2026 公历日）+ verify.sh 双模式；
- 验收：双模式断言全 PASS；fmt/lint 全绿；spec/ROADMAP/README 生态同步（此步并入 S5 生态收口）。

### M66-S5：生态收口（文档 + CI + 发布）
- `docs/spec.md`：网络/进程/文件原语表补行（unix_connect、os_exec/rename/remove_all/random_hex/file_sha256、os_capture/os_popen、os_kill group、write_file/append_file mode、zip_unpack password 双算法说明）；05 G6 which 用法示例；
- `CHANGELOG.md` [Unreleased] M66 完整条目；`docs/ROADMAP.md`：主线表补 M66 行、当前里程碑"M66 已闭环"；
- `README.md`/`README.en.md`：原生开发表 M41–M65 扩至 M41–M66（中英一致）、stdlib 列表加 yaml/pxml/lunar；
- `.github/workflows/ci.yml`：toolchain job lint 清单补新文件；新增 m66_proc/m66_yaml/m66_pxml/m66_lunar verify 冒烟；
- `tools/make_release.sh`：⑥ 自检清单同步；
- 验收：全仓 fmt --check + lint 全绿；ci.yml YAML 校验 + make_release bash -n；grep 文档无残留错标。

### M66-S6：qg-issue 回写 + 端到端验收
- `/data/qg-issue/` 各 ISSUE.md 头部状态行回写：01/02 →「✅ 已合入 M66-S1（commit xxx）」，03/04 →「✅ 已收编 std.yaml/std.pxml（commit xxx）」，05 →「✅ G1–G7 已合入 M66（G6 随 G1 用法示例覆盖）」，06 →「✅ T1/T2 已合入 M66-S1、T3 农历已收编 std.lunar（M66-S4）→ ws-todo 主体可写」，00-README 索引状态列同步；
- 05 §7 / 06 §7 重写时机 checklist 勾除（G1/G2/G3/Issue2/私有补丁确认 全勾 → ws-install 可写；unix_connect/os_capture/os_popen/os_kill group/农历 全勾 → ws-todo 主体可写）；
- 端到端验收总闸：全仓 fmt/lint 全绿 + capability 双模式 253 PASS + diffcheck --all + m65_lsp/m65_mcp/m66_* 全 verify PASS + pxi 冒烟（新 native 解释模式可用）+ git log 干净 + push main。

## 5. 风险与对策

| 风险 | 对策 |
|---|---|
| `pxc build interp.px` 极慢（>10min、RSS 2.5GB，M65-S1 实测） | 全部 runtime 改动**一次**重建；background + 日志；期间不并行其它重活 |
| Issue 2 procfs.patch 锚点漂移 git apply 失败 | 已实测 patch_osfs.py 幂等脚本 [OK]；锚点异常会报错不写文件（安全） |
| 新 native 在 pxi 解释模式缺失/慢 → std.yaml/pxml/lunar 双模式不一致 | stdlib 文件头注明加密能力主打编译模式（std.gfx 先例）；专项测试双模式复跑 |
| zip 双算法实现复杂（zipcrypto 传统 + AES-256 PBKDF2） | miniz 已带 crc32/deflate/inflate；PBKDF2-HMAC-SHA1/AES-CTR 需 mbedtls 或手写（runtime 已有 mbedtls sha256 + aes 先例）；先小样本验证再落 |
| 农历数据表正确性 | 对齐公开权威数据（1900-2100 逐年），多个已知锚点对拍（春节/闰月/腊月三十） |
| 语言坑（无 `;`/控制流换行/数组跨行受限/let 不可变/模块级 var 函数内不可读） | 收编前 run+build 双模式复跑；必要时最小适配 |
| 新增 native 引发 lint 误报 | S1 同步 pxlint BUILTINS + 重建 pxlint（M65-S1 先例） |

## 6. 验证矩阵（总闸）

| 验证 | 命令/方式 | 期望 |
|---|---|---|
| Issue 2 专项 | `pxc build os_fs_test.px && ./build/os_fs_test`、exec_demo | 16 PASS / PID 不变 + 透传 |
| 新原语专项 | examples/m66_proc 双模式 | 全 PASS（含 unix/capture/popen/kill/mode/zip 双算法） |
| stdlib yaml | examples/m66_yaml verify.sh | 双模式 35 PASS 一致 |
| stdlib pxml | examples/m66_pxml verify.sh（含 enc env） | 双模式 68 + demo 16 + enc 4 PASS |
| stdlib lunar | examples/m66_lunar verify.sh | 双模式锚点断言全 PASS |
| 语言回归 | capability.px pxi + pxc build | 各 253 PASS/0 FAIL |
| 对拍回归 | selfhost/diffcheck.sh --all | rc=0 |
| M65 回归 | examples/m65_lsp、m65_mcp verify.sh | ALL PASS |
| 仓库收敛 | fmt --check + lint（compiler.px 项目级 + tools 全文件） | 全绿 0 错 0 警 |
| 冒烟 | 新 native `pxc run` 一行调用 | 解释模式可用 |

## 7. S1 执行记录（2026-09-05 真实执行，m66_proc verify ALL PASS）

- **runtime/runtime.c**：Issue 2 五件套经 patch_osfs.py 幂等插入（os_exec/os_rename/os_remove_all/os_random_hex/os_file_sha256，decl 224-228 / impl 5561+ / reg 5392-5396）；os_kill 第三参 group（kill(-pid) 组杀，两参旧调用兼容）；write_file/append_file 第三参 mode（open+O_CREAT+mode，显式 fchmod 防 umask 削权限）；新增 unix_connect/os_capture（双管道 poll 分离 stdout/stderr）/os_popen（双向管道，子进程 setpgid 自成组→os_kill group 可组杀）。
- **runtime/runtime_zip.c**：zip_unpack 增加第三参 password，支持 **zipcrypto 传统（PKWARE CRC32 keys 流）+ WinZip AES-128/192/256（AE-1/AE-2：extra 0x9901 探测 + PBKDF2-HMAC-SHA1 派生 + AES-CTR 解密 + HMAC-SHA1 auth code 校验）**；AES extra 真实 method 探测。
- **pxi 同步**：selfhost/interp.px names +16（os 五件套/unix_connect/os_capture/os_popen），ibuiltin.px dispatch 补转发（os_kill 支持 2-3 参）；pxlint/pxcheck/lsp_core BUILTINS 补 open/close/ioctl/os_errno（M57 fd 原语漏补）+ M66 新 native。
- **重建**：bootstrap/pxi 全量重建（quic 符号在，9.32MB，pxi --version 0.1.0）；bootstrap/pxlint 全量重建（9.17MB，含新 BUILTINS）。pxi 解释模式新 native 冒烟 PASS（os_capture/os_random_hex/os_file_sha256 解释可调）。
- **专项验证（examples/m66_proc verify.sh）**：os_fs_test 16 PASS；exec_demo os_exec 进程替换透传；proc_test 14 PASS（write_file mode 0600/os_capture 分离 rc/os_popen 双向/os_kill group 组杀/os_kill 两参兼容/unix_connect 失败-1+errno）；unix_test 行协议（python unix sock daemon pong 回显）PASS；zipcrypto 密码 zip 解包 PASS；**WinZip AES-256 密码 zip（pyzipper 生成）解包 PASS**。
- **回归**：capability 双模式各 253 PASS/0 FAIL；diffcheck --all 全量对拍全绿；m65_lsp/m65_mcp verify ALL PASS；新文件 fmt/lint 全绿（tools 全绿）。
- **S1 收尾**：已 git commit `b01d7ea`（runtime 六文件 + pxi/pxlint 重建 + m66_proc examples + PLAN）。

## 8. S2 执行记录（commit `17f00d5`，m66_yaml verify ALL PASS）

- `stdlib/yaml.px` 收编（403 行，纯函数零 import，内容来自 qg-issue 03 yaml.px + 文件头按 stdlib 风格改写：
  用途/支持子集/不支持项/入口 yaml_parse 说明）；`examples/m66_yaml/yaml_test.px`（99 行 35 断言）+ verify.sh 双模式。
- 验证：pxi run + pxc build --no-quic 双模式各 **35 PASS** 且输出一致；fmt --check + lint 0/0；
  capability 253 无回归（stdlib 收编零 runtime 改动）。

## 9. S3 执行记录（commit `6df05ef`，m66_pxml verify ALL PASS）

- `stdlib/pxml.px` 收编（444 行，加密能力 aes_gcm/base64/hex 依赖编译模式 native → 文件头注明主打编译模式）；
  规范主体 CONFIG-FORMAT.md → `docs/PXML.md`（v0.6：语法 EBNF/enc 策略/决策表/语言缺口笔记 §5.6/Dogfood §5.7）；
  dogfood 迁 `examples/m66_pxml/`（app.pxml/upstream.pxml/config_loader.px/demo.px/demo_enc.px）+ verify.sh。
- 验证：pxml_test 双模式各 **68 PASS**；demo 16 PASS + demo_enc 4 PASS（PXML_MASTER_KEY env 不落盘）；
  fmt/lint 全绿。

## 10. S4 执行记录（stdlib.lunar，待提交）

- `stdlib/lunar.px` 新建（185 行，第 9 个标准库）：**内嵌 1900-2101 每年正月初一偏移表（202 项）+ 1900-2100
  农历年表 5 位 hex（201 项，闰月+大小月）**，源自寿星天文历（sxtwl）同源数据逐年对拍 0 误差；
  入口 `lr_solar_to_lunar(y,m,d)` / `lr_lunar_to_solar(y,m,d,leap)` / `lr_md_in_year(m,d,solar_year)`
  （ws-todo lunar:8-8 场景）/ `lr_leap_month/lr_year_days/lr_month_days/lr_leap_days`；纯函数双模式一致。
- `examples/m66_lunar/lunar_test.px`：**36 断言双模式全 PASS**（2024/2026/2023 春节、2023 闰二月锚点、
  边界 1900-01-31 / 2100-12-31、1996 八月十五、2033 春节、5 组往返互转、闰月号/闰月天数/年天数、
  lunar:M-D 落点 2026 八月初八=2026-09-18 等）+ verify.sh（双模式 + fmt/lint）。
- 验证：解释模式 36 PASS/0 FAIL；编译模式 36 PASS/0 FAIL；stdlib/lunar.px + lunar_test.px lint 0/0、fmt 全绿。

## 11. S5 执行记录（生态收口，待提交）

- spec.md §8.20 原语补全段（unix_connect/os 五件套/os_capture/os_popen/os_kill group/write_file mode/
  zip_unpack 密码双算法 + G6 which 用法示例）+ §10.3 标准库表补 std.yaml/std.pxml/std.lunar 三行；
- CHANGELOG [Unreleased] M66 完整条目；ROADMAP 主线表补 M66 行 + 当前里程碑更新；
- README/README.en：原生开发表 M41–M66 + 📚 标准库 9 个清单 + 目录树 stdlib 注释同步；
- ci.yml toolchain job 工具自测并入 m66_yaml/m66_pxml/m66_lunar verify；make_release.sh 无新二进制免改；
- 全仓 fmt --check + lint 全绿；ci.yml YAML 校验 + make_release bash -n。
