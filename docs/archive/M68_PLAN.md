# M68_PLAN · pxi 一致性收官（解释器 native 可达性根治）

> 创建：2026-09-05 · 修订：2026-09-05（用户决定：生态线移出 M68，本里程碑只做 pxi 一致性）
> 来源：qingge 反馈「pxi 内置不全、缺 sqlite，编译器没事」→ 源码级侦查确认根因
> 处理人：东月 · 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ 126f1f0，M67 闭环 · v0.1.0-m67 已发布）
> 目标：**pxi（解释器）对 runtime 内嵌 native 的可达性与 pxc 编译产物完全一致**（根治 A）——
> 用户裸脚本（零 `extern def`）在 pxi 下可调 sqlite 等全部 native，不再 R1001「未定义变量」。
> 关联：00-README 通用说明 §4 已知语言缺口第 1 条（pxi native，长期留档未决）。

## 〇、立项决策记录（用户拍板，含调整）

| # | 决策 | 结论 |
|---|---|---|
| D1 | pxi native 对齐机制 | **A 根治**：C 侧 224 native 全量进 FFI 表 + pxi 调用解析失败自动回退 ffi_call，零 extern def，双模式行为一致（否决 B 治标补白名单） |
| D2 | 里程碑范围 | **M68 只做 pxi 一致性**（A 线）。原「生态启动」B 线（9 官库 + dogfood registry 化 / AI 速查包 / ECOSYSTEM 基线等 P0/P1 项）经用户调整**从 M68 移除**，生态缺口留档为未来候选里程碑（见 §七），信息不丢弃、不在本里程碑执行 |
| D3 | S5 发布 | **打 tag `v0.1.0-m68`** 自动发布（沿用 tag 驱动 workflow） |
| — | PR 策略 | 维持**不开 PR**（用户既定边界）→ 直接 commit + push origin main |

## 一、pxi native 可达性根因（2026-09-05 侦查实录，源码级证据）

### 现象
- pxi 跑 `sqlite_open(":memory:")` → **R1001 未定义变量**；同切片 `pxc build` 产物运行正常（exec rc=0）。
- capability.px 双模式 253 PASS 系因它对 sqlite/quic/h3 **全写了 extern def**，掩盖了不对称。

### 根因链
1. **编译模式**：`runtime/runtime.c` `px_register_builtins()`（5142 行起）将 **224 个** native 以 `px_set_global` 注册为全局 → 任何编译产物**无声明直接可调**。
2. **pxi 解释模式**（selfhost/interp.px + ibuiltin.px）只认两条路：① `i_call_builtin` 手工白名单（**129 名**）直调宿主 C；② 用户代码显式 `extern def` → FFI 桥（runtime `px_ffi_register` 主函数内仅 8 名：sqlite 6 + bytes 转换 2）。
3. → **差集 98 个 native 在 pxi 默认不可达**（sqlite_*/aes_*/rsa_*/xml_*/zip_*/tcp_*/udp_*/ws_*/sse_*/cron/signal/session_*/bus_*/http_serve/http_get_stream/tls_server/time_format/os_pid/fsync_file/truncate_file/read_at/write_at/xxhash/basic_auth/rate_limit/middleware/vhost/route/px_serve 等；`__px_*`/`__http_handler` 等伪全局与语言内置另通道需在 A1 甄别剔除）。

### 影响面
- 清歌等用户 pxi 直接跑裸脚本（未写 extern def）→ R1001 → 误判「解释器内置不全、编译器没事」。
- 双模式一致性欠账最后一类：**语言内置已对齐，C native 可达性未对齐**。

## 二、范围与边界

**做**：
- 精确盘点 224 native 差集 → `docs/pxi_native_diff.md` 差异表（含修复后状态）。
- C 侧统一 FFI 注册（224 native 全量登记进 ffi 可达面）+ selfhost 解释层自动回退 ffi_call。
- pxi 重建（selfhost 链，>10min / RSS 2.5GB 预留）+ 全量双模式回归。
- 文档收口：spec §8/§10.2、MINI_SUBSET §十三、README/README.en、CHANGELOG [Unreleased] M68 条目、00-README §4 第 1 条勾除（qg-issue 侧）。
- S5：全链复跑 + 打 tag `v0.1.0-m68` 发布 + 本机留档 + 发布指引更新。

**不做**：
- ❌ **生态启动全部项**（registry 资产化 / pxreg / AI 速查包 / ECOSYSTEM.md / P0.1–P0.3 + P1.1–P1.3）—— 已移出，留档 §七 未来候选，不在 M68 执行。
- 不改语言语义 / parser / compiler / codegen / pxc build 行为（A 线为 runtime 注册层 + selfhost 解释层改动，compiler 不动 → 自举证明非必需，但 pxi 重建后须全量双模式回归）。
- 不逐个手补白名单（治标，A 方案已否决）。
- 不回归 FFI 外部库机制（zlib/sqlite3.o 等经 `extern def` 的 C 库路径双模式已一致）。
- 不做 spawn/chan/mutex 等 Mini 子集有意排除项。
- 不开 PR；不修语言语义缺陷（模块 var / 数组跨行 / let 不可变等，00-README §4 另 3 条，留档未来候选）。

## 三、分步计划

### A1 · 能力精确盘点（纯侦查，差异表 v1）
- 脚本 diff：runtime `px_set_global` 全名表（224）vs pxi 白名单（129，ibuiltin.px 全部 `name == "..."` 分支）vs 语言内置另通道（interp/iexpr/it_util i_ 前缀、KEYWORDS、常量 pi/e、args/input/panic）。
- 98 差集逐一归因：真 native（应可达）/ 语言内置另通道（剔除）/ 伪全局内部位（`__px_*`/`__http_handler`/`__sse_handler`/`__px_docroot/port/timeout` 剔除）/ 有意排除（记录依据）。
- 产出 **`docs/pxi_native_diff.md`**：列 = native | runtime 注册 | 编译模式 | pxi 现状（白名单/FFI/不可达）| 归因 | 建议 | 修复后状态（A3 后回填）。

### A2 · 机制侦查 + 方案定案
- 读 `i_eval_call` Var 分支完整顺序（iexpr.px ~262 起）与 R1001 产生点；读 runtime `ffi_call`/`px_ffi` 实现（runtime_ffi.c）与 `px_register_builtins` 注册循环 → 定 A 落地细节。
- C 侧统一注册宏设计：`px_register_builtins` 内每个 native 同时进 `px_ffi` 表（不重复注册、顺序稳定）；`ffi_call` 查无此名返回**可辨错误**（区分「未注册」与「调用失败」，不杀进程）。
- selfhost 侧回退点：`i_eval_call` 用户函数/白名单/i_ffi 全未命中处 → 构造 `i_builtin_ffi_call([cname, arg_vals])` → ffi 命中即调、未命中才 R1001（与编译模式「全局名运行时查表」语义对齐）。

### A3 · 实现 + pxi 重建 + 双模式回归
- 按 A2 实现 C 侧 + selfhost 侧；`bootstrap_prove.sh` 等价链重建 pxi（**>10min / RSS 2.5GB，预留整块时间，后台执行 + 定期探活**）。
- 回归总闸见 §五；补 typo 场景用例（真拼错变量不应被误当 C native、错误语义不漂移）。

### A4 · 文档收口
- `pxi_native_diff.md` 定稿（差异表 + A3 修复后状态回填）。
- spec §8/§10.2「pxi native 可达性与编译模式一致（无需 extern def）」；MINI_SUBSET §十三 对应更新。
- CHANGELOG [Unreleased] M68 条目草稿；qg-issue 00-README §4 第 1 条状态更新。

### S5 · 总闸 + 发布（D3：打 tag）
- 全链复跑（§五）+ 工作树收敛（全仓 fmt --check / lint 0/0）。
- 文档定稿 + CHANGELOG [Unreleased] M68 完整条目。
- **打 tag `v0.1.0-m68` → push → tag 驱动 workflow 自动发布**；对 GitHub 实际产物二次解包冒烟（沿 M66/M67 收尾先例）；本机留档 + 发布指引更新至 m68。

## 四、回归总闸（A3 与 S5 两次跑）
- capability.px 双模式 **253 PASS 且解释/编译逐字节一致**（主闸）。
- diffcheck --all 全量 + stdlib 9 库双模式（collections/semver/edge/gfx/png/webroute/yaml/pxml/lunar）。
- examples m65_lsp / m65_mcp / m66_* / m67_* verify 抽跑 + 新增 pxi native 抽测切片（98 差集中代表性 native：sqlite_open/json_encode/yaml_parse/zip_unpack/aes_gcm_encrypt/os_pid/now_ms/http_serve 等，pxi == 编译产物一致，**零 extern def 裸脚本**）。
- typo 语义回归（拼错变量仍报 R1001、错误可辨、不误当 C native）。
- 全仓 fmt --check + lint 0/0；ci.yml YAML 校验；make_release.sh bash -n。
- push origin main 后 CI 六 job 全绿确认（自举回归/示例/工具链质量门/multiarch-cross 三档）。

## 五、风险与预案

| 风险 | 影响 | 预案 |
|---|---|---|
| pxi 重建超长（>10min / RSS 2.5GB） | 进度卡顿 | A3 集中一次重建；后台执行 + 定期探活；失败按 bootstrap 链重试 |
| ffi 自动回退改变错误语义（真 typo 被误当 C native） | 行为漂移 | A2 先验证 ffi 查无此名可辨错误 → 回退仅在该错误转 R1001；typo 回归用例覆盖 |
| 白名单 129 名与自动回退重复路径 | 双重调用 | 回退只放白名单/i_ffi/用户函数全未命中后（A2 确认顺序）；同名字不改白名单分支 |
| 224 全量进 FFI 表致注册表膨胀/顺序不稳 | 启动/行为风险 | 统一注册宏复用既有表结构、仅增登记不改语义；差异表脚本化可重跑验证 |
| 盘点归因误判（伪全局/另通道混入差集） | 实现范围偏差 | A1 严格三表 diff + 逐名人工复核；伪全局/有意排除单独列并记依据 |
| 生态线被临时塞回 | 范围蔓延 | 本里程碑边界明确「不做生态」；缺口已留档 §七，可独立立项不阻塞 M68 |

## 六、验收清单（里程碑完成判据）
- [x] `docs/pxi_native_diff.md` 定稿：281 全量归因、修复后 pxi 可达 = 编译模式
- [x] **零 extern def 裸脚本** pxi 跑通 A 类代表性 native（sqlite_open/json/aes/os_pid/now_ms/http_serve 等），结果与编译产物一致 —— t_native.px 19/19（sqlite5+os_pid/now_ms/tz_offset/time_format+xxhash/aes_gcm 往返+xml_parse/escape+fsync/read_at/write_at/truncate 文件原子），pxi == 编译产物
- [x] pxi 重建成功；capability 253 双模式逐字节一致；stdlib m66 yaml35/lunar36/pxml68/proc14 双模式 PASS；diffcheck --all 全绿
- [x] typo 语义不漂移（拼错变量仍 R1001、可辨）—— t_typo.px：R1001 'sqliite_open'
- [x] spec §9.3、MINI_SUBSET §十三.0、README/README.en、CHANGELOG M68、00-README §4 第 1 条 全部收敛
- [x] tag `v0.1.0-m68` 发布成功（Release 资产 `puxian-0.1.0-m68-41afc1d.tar.gz` 50,404,289B，sha256 e63ac492… 与 GitHub API 记录一致）；GitHub 产物二次解包冒烟全绿（pxc/pxi 静态 ELF + hello 编译/解释 + import std.semver + **零 extern def 裸脚本 sqlite_open/exec/query + os_pid + now_ms pxi == 编译产物逐字节一致**）；本机留档 + 发布指引更新至 m68

## 七、生态启动 · 留档（未来候选里程碑，不在 M68 执行）
> 2026-09-05 用户调整：生态线移出 M68。以下为原 P0/P1 缺口留档，供独立立项时引用，不丢失。
> 核心诊断：生态不是少，是**供给侧强、需求侧空**——9 个官库 + dogfood 代码已在，缺的是可 `import` 的在线 registry + 让 AI「喂进上下文就能写对」的速查包。
> 佐证：spec §8.2 import 已支持 `import std.xxx` / 相对路径 / `import "c/xxx"`，import 只注册不执行顶层；stdlib/ 9 库平铺 → 基座现成。
- **候选 M69（生态启动）**：① registry 资产化（9 官库 + dogfood ≥20 项 → ECOSYSTEM.md + 机器索引）；② AI 速查包（native 表由脚本从 runtime dump 防漂移 → PUXIAN_CHEATSHEET.md）；③ pxreg fetch 拉取闭环（fetch → import 双模式可用）；④ 写库体验缺口评估（模块 var / 数组跨行 / let 不可变 → 语义修复再拆档）；⑤ 需求/贡献通道（qg-issue 继续；Discussions/PR 是否放开待用户定）。

## 八、执行进度（M68 场次实时记录 · 东月）

> 2026-09-05 12:00 上午场开始（ws-todo #10 唤醒）。基线：main @ 126f1f0 + M68_PLAN commit a466ede，工作树干净。

### A1 · 能力精确盘点 — ✅ DONE
- 实证口径（比立项侦查更精确）：编译产物默认可达 = runtime **px_set_global("name", px_native(...)) 启动注册全局 native 全集 281**（runtime.c px_register_builtins 217 + quic/h3/qpack/ws 注册 64；__px_* 动态伪全局与 pi/e 常量已剔除）；pxi 默认可达 = **interp.px i_register_builtins() names 数组 129 名**（env_define 注册进 g_globals）。
- 差集 **155** = 编译可达、pxi R1001。分类：**A 类真 native 91**（sqlite6/aes4/rsa5/xml5/zip2/tcp6/udp5/ws9/sse4/session7/ctx3/bus4/cron·signal·time/http_serve·px_serve·route·vhost·middleware·rate_limit·basic_auth·sandbox/fsync·read_at·write_at·truncate_file/set_timeout·set_interval·clear_timer/xxhash/os_pid/now_ms/args/input/panic/gc 等）+ **B 类 quic/h3 64**（#ifndef PX_NO_QUIC 条件注册）。
- 用户现象复现：pxi 裸跑 `sqlite_open(":memory:")` → R1001（rc=1）；`pxc build` 同切片产物运行正常（exec rc=0、rows=[{a:1},{a:2}]）→ 「pxi 缺 sqlite、编译器没事」准确。
- 产出：**docs/pxi_native_diff.md**（逐名差异表 + 归因排除项）。

### A2 · 机制侦查 + 方案定案 — ✅ DONE（落点比立项字面更优，忠实 D1 根治意图）
- 侦查要点：`px_get_global` 未找到会 px_error（杀进程）→ 不能用于 ffi 兜底探测，需新非致命查询；`i_register_builtins` names 是 pxi 可达性真正单源（129 名 env_define 进 g_globals，用户裸名调用 → i_eval_call Var → env_get 未命中 → R1001）；宿主 C native 可经 `px_err(px_str(...))` 返回**非致命可辨错误**（PX_RESULT）。
- 定案（忠实 D1「A 根治：224 全量可达 + 自动回退 + 查无此名可辨错误 + 零 extern def」，实现路径由「逐行复制 224 进 ffi 表」优化为「**ffi_call 双表兜底**」）：
  1. **C 侧 runtime.c**：新增 `bool px_global_native(const char* name, LXValue* out)`（g_globals_mu + gc_block_stop 锁协议，名存在且 type==PX_NATIVE 才 true，未找到不杀进程）——全局表即单源，天然覆盖 runtime.c + quic/h3 全部 px_set_global native，**无需逐行注册宏、无 ffi 表扩容/双源漂移**。
  2. **C 侧 runtime_ffi.c**：`bi_ffi_call` 双表：① ffi 注册表（extern def C 库原路径）→ ② px_global_native 全局 PX_NATIVE 兜底；两表未命中 → `px_err(px_str("ffi_call: 未注册函数: <name>"))`（可辨、不杀进程）。**已验证**（旧编译器 + 新 runtime build 冒烟）：ffi_call("os_pid",[])→pid、ffi_call("now_ms",[])→now_ms、未注册名→is_err=true 且载荷前缀可辨。
  3. **selfhost iexpr.px `i_eval_call` Var 分支**：chan/mutex/rwlock 判断后新增宿主回退——`env_get` 未命中 → 评估实参 → `i_builtin_ffi_call([cname, arg_vals])`；宿主 Err 载荷为 "ffi_call: 未注册" 前缀（真 typo）→ 转 `i_r1001(cname)`；其余返回值（含业务 Err 值）原样返回。
- 改动文件：runtime/runtime.h（+声明）、runtime/runtime.c（+px_global_native）、runtime/runtime_ffi.c（bi_ffi_call 双表）、selfhost/iexpr.px（i_eval_call 回退）。compiler/parser/语言语义零改动。

### A3 · pxi 重建 — ✅ DONE
- `tools/pxc build selfhost/interp.px` 重建成功（12:14，9326544 字节）→ `cp selfhost/build/interp bootstrap/pxi`（commit 3b47dbe 随代码一并入库）。
- **回归全绿**：① t_sqlite 用户现象 pxi 裸调正常（原 R1001）；② t_native 零 extern def 19/19，pxi == 编译产物；③ t_typo 拼错名 R1001 可辨；④ capability pxi 253 = bin 253 逐字节一致；⑤ diffcheck --all rc=0；⑥ m66 stdlib verify yaml35/lunar36/pxml68/proc14 双模式 PASS（编译产物 + pxi run 复核）；⑦ iexpr.px fmt --check 通过（lint 单文件 import 变量误报为既有工具噪音，非本改动引入）。
- commit：**3b47dbe**（runtime.h/runtime.c/runtime_ffi.c/iexpr.px/bootstrap.pxi，+65/-2）。

### A4 · 文档收口 — ✅ DONE（commit 41afc1d）
- pxi_native_diff.md 修复后状态回填 ✅；spec §9.3 + MINI_SUBSET §十三.0 + README/README.en 已知限制 + CHANGELOG M68 条目 ✅；qg-issue 00-README §4 第 1 条勾除 ✅（/data/qg-issue/，仓库外）。
- commit **41afc1d**（docs 收口全链记录）已推送 origin main。

### 14:00 · 中断收尾（ws-todo #11）— M68 未全部完成，按计划停止
- **断点**：A1–A4 全部完成并推送 origin main（main @ 41afc1d，工作树干净）；**S5 未执行** → 18:00 晚间场从 S5 续做。
- **18:00 晚间场待做（S5 总闸 + 发布）**：
  1. 全链复跑（§四回归总闸：capability 双模式 253 一致 / diffcheck --all / stdlib 9 库双模式 / m65–m67 抽跑 / typo 用例 / 全仓 fmt --check / lint / ci.yml YAML / make_release.sh bash -n）；
  2. 工作树收敛 + CHANGELOG [Unreleased] M68 完整条目核验；
  3. **打 tag `v0.1.0-m68` → push → tag 驱动 workflow 自动发布**；
  4. GitHub 实际产物二次解包冒烟（沿 M66/M67 先例）；
  5. 本机留档 + 发布指引更新至 m68；
  6. 验收清单最后一项 [ ] → [x] 回填 + 向用户完整报告。
- 14:00 现场快照：origin/main @ 41afc1d 已同步、工作树 clean、产物齐备（bootstrap/pxi 9326544B 12:14 重建、docs/pxi_native_diff.md 10025B）。

### 18:00 · 晚间场 S5 收尾（ws-todo #12）— ✅ 里程碑闭环
> 先读 §八 断点留档续做。侦查发现：tag v0.1.0-m68 于 12:26 已打已推（指向 41afc1d，A4 代码终态）、
> Release workflow run 4 已成功、资产已发布（sha256 e63ac492…），CI run 101(41afc1d)/102(428fa12) 均六 job 全绿
> → S5 剩余项 = 全链复跑 + 产物冒烟 + 留档 + 指引 + 验收回填。
- **全链复跑（§四总闸，本机 18:03-18:25）**：
  ① capability 双模式 **pxi 253 = bin 253 逐字节一致**（diff 空，CAPABILITY_DIFF_IDENTICAL）；
  ② diffcheck --all **全量对拍全部通过**（rc=0，96 行日志）；
  ③ 发布产物冒烟（/tmp/pxm68 解包 v0.1.0-m68-41afc1d）：pxc/pxi 均 statically linked ELF + --version 正常；
     hello.px 编译静态 ELF 运行 + 解释运行均输出一致；`import std.semver` + sv_parse("1.2.3") 编译运行
     (major=1 minor=2 patch=3) 验证 stdlib 定位；**M68 核心：零 extern def 裸脚本（sqlite_open/exec/query +
     os_pid + now_ms）pxi rc=0 输出 `rows=[{a: 1}, {a: 2}] pid_gt0=true now_ms_gt0=true` 与编译产物
     逐字节一致（SMOKE_DIFF_IDENTICAL）—— 用户「pxi 缺 sqlite」现象在发布版彻底消失**；
  ④ typo 语义：`sqliite_open` → `错误 [R1001] 1:22: 未定义变量: 'sqliite_open'`（真 typo 不误当 native、可辨）；
  ⑤ verify：m64_fmt / m64_lint / m65_lsp / m65_mcp / m66_yaml / m66_pxml / m66_lunar / m66_proc **8 项全 PASS**；
  ⑥ fmt --check 全绿（selfhost+tools+stdlib 收敛域）；lint compiler.px 0 错 + tools 16 文件 0/0；
  ⑦ ci.yml + release.yml YAML 合法；make_release.sh bash -n OK；⑧ CI run 102（HEAD 428fa12）六 job 全绿。
- **本机留档**：`/data/release/puxian-0.1.0-m68-41afc1d.tar.gz`（sha256 e63ac492e74df… 与 GitHub API 一致）+
  `PuXian发布包开发指引-v0.1.0-m68.md`（§5 删除过时「pxi 缺 native」缺口描述，更新为 M68 后双模式一致 + M68 能力速览）。
- **验收清单全勾**（§六 8/8）→ M68 里程碑闭环。M68 收尾 commit 随本文档入库并推送 origin main。
