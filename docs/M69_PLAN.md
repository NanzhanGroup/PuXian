# M69_PLAN · 生态启动（资产化 + AI 速查 + 拉取闭环）

> 创建：2026-09-05（待用户拍板后 commit 入库）· 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ 7a7e1f0，M68 闭环 · v0.1.0-m68 已发布）
> 来源：M68_PLAN §七「生态启动 · 留档」（2026-09-05 用户调整 M68 时移出，明确单立后续里程碑）
> 用户指令（2026-09-05）：GitHub 见 M68 收尾 → **立项 M69：生态线**。
> 目标：把「供给侧强、需求侧空」的缺口补上——让**人与 AI 都知道 PuXian 有什么、怎么拿、怎么写对**，
> 并打通「fetch 到 registry → pxpkg 安装 → import 双模式可用」的拉取闭环。

## 〇、生态现状（2026-09-05 侦查实录）

**供给侧（已强，勿重复造）**
- `stdlib/` 9 库：collections / semver / webroute / yaml / pxml / lunar / gfx / png / edge（L1 纯 .px，随发布包分发）。
- `tools/pxpkg` 包管理器（M45）：px.toml + 三形态依赖（semver 范围/本地路径/http URL#sha256）+ px.pkg.lock 可复现构建 + `PX_REGISTRY` 目录 registry。
- `examples/` 119 个 dogfood（m22–m67 逐里程碑沉淀：HTTP/WS/SSE/QUIC-H3/TLS/SQLite/加密/进程/硬件/农历…）。
- 发布包（make_release.sh）已含 `stdlib/` + `tools/pxc` `pxpkg` `routegen` `cross_*`；README 快速开始可用。
- **M68 刚闭环**：pxi 与编译产物 native 可达性一致（零 extern def 双模式全通）→ 生态库双模式无暗礁，是生态启动的前置（本里程碑直接受益）。

**需求侧（空，M69 要补）**
| 缺口 | 现状 | 影响 |
|---|---|---|
| 生态总览 | 无 ECOSYSTEM.md；README「生态与合作」仅 3 行；spec 无库一览 | AI/用户不知道有哪些库、干什么、怎么 import |
| AI 速查 | 无 cheatsheet；native 281 项只在 runtime.c / pxi_native_diff.md 散落 | AI 写 .px 靠猜 native 名/签名，易错 |
| 拉取闭环 | registry 仅本地目录形态；无随库分发的 registry 资产与「fetch 即用」演示 | 用户/清歌想复用官库需手工拷 stdlib，无包管理体验 |
| 写库规范 | 三条语言缺口（模块 var 函数内不可读 / 数组跨行受限 / let 不可变）+ 无分号换行约束，散在 MINI_SUBSET/qg-issue | 生态库作者踩坑；纯函数库需显式传状态等绕行 |
| 贡献通道 | qg-issue 仓库外；README 一句「issue/PR 欢迎」；PR 是否放开未定 | 第三方贡献无入口形态 |

## 一、范围与边界

**做（S1–S5）**：
- **S1 生态资产化**：stdlib 9 库 API 盘点 + examples 分类（可复用库/服务 demo/语言演示）→ `docs/ECOSYSTEM.md`（库一览：定位/导出 API 摘要/适用场景/双模式状态）+ **机器索引脚本**（从 stdlib/ 扫描生成，幂等、CI 校验防漂移）→ README「生态」段升级为库总览入口。
- **S2 AI 速查包**：`docs/PUXIAN_CHEATSHEET.md` —— native 表**由脚本从 runtime dump**（防漂移）+ 语言速查（语法/类型/控制流/import/错误通道）+ 9 库速查 + 高频模式（hello/http server/sqlite/pxpkg 三步）→ 目标：整包喂进 AI 上下文即可写对 .px。
- **S3 registry 拉取闭环**：registry 资产随库入库（`registry/<name>/<version>/<name>.px` 结构，官库 9 + 精选 dogfood 库打包）+ 端到端验证：建项目 `pxpkg init/add/install` → `import` 双模式可用；评估 pxpkg 远程 registry（PX_REGISTRY=http(s) URL）可行性与落地；交付「fetch → import」最小闭环 + 文档用例。
- **S4 写库体验评估**：三条语言缺口 + 换行/分号约束 → 每个给影响场景/绕行规范/修复成本收益评估 → `docs/ECOSYSTEM_GAPS.md`（写库规范 checklist：纯函数/显式传状态/双模式验证/fmt/lint/pxcheck）；**语义修复只评估入档，拆未来候选**（避免 M69 碰 compiler/parser）。
- **S5 总闸 + 发布**：全链回归 + 文档收口（README/README.en/spec §8.6 registry 补充/ROADMAP M69 行/CHANGELOG）+ **tag `v0.1.0-m69` 自动发布** + GitHub 产物二次冒烟 + 本机留档 + 发布指引更新。

**不做**：
- ❌ 语言语义修复（模块 var / 数组跨行 / let 不可变 / 分号换行）—— S4 只评估入档，修复拆未来里程碑（候选 M70，改 compiler/parser 需独立排期与回归）。
- ❌ 新 C 绑定（SDL/raylib 等）—— 生态有需求再立项。
- ❌ WASM 后端等 ROADMAP 远期项。
- ❌ 改 compiler/parser/语言语义；不开 PR（维持既定边界，直接 commit + push origin main）。

## 二、分步计划

### S1 · 生态资产化（ECOSYSTEM.md + 机器索引）
- 侦查：stdlib 9 库公开 API（顶层 def 清单 + 定位）、examples 119 分类清单、README/spec 既有生态表述。
- 产出 `docs/ECOSYSTEM.md`：库一览表（名称/一句定位/导出 API 摘要/适用场景/依赖/双模式）+ dogfood 资产图（examples 按里程碑/能力索引，指向可复用者）+ 消费路径（import std.x / pxpkg add / 拷源码）。
- 机器索引：`tools/gen_ecosystem.px`（或扩展 pxdoc.px）：扫描 stdlib/*.px 顶层 def → 生成机器可读索引（JSON）→ 与 ECOSYSTEM.md 同步；**CI 校验 stdlib 变更后索引一致（防漂移）**。
- README「生态」段升级：库总览表 + 指向 ECOSYSTEM.md + 快速用库示例。

### S2 · AI 速查包（PUXIAN_CHEATSHEET.md）
- native dump 脚本：从 runtime.c `px_register_builtins` 注册表 + quic/h3 条件注册表提取全量 native 名 → 按能力域分类（IO/文件/网络/HTTP/WS/SSE/QUIC-H3/TLS/加密/压缩/编码/JSON/XML/DB-SQLite/系统/进程/时间/时钟/GC/杂项）→ 生成速查 native 表（**防漂移：脚本入库 + CI/发布校验**）。
- 语言速查段：词法/类型/控制流/函数/闭包/import/错误通道（Result + ?）/并发（spawn/chan）/Mini 子集边界。
- 库速查段：9 库核心 API 一行式；高频模式段：hello/HTTP 服务/SQLite/pxpkg 建项目三步/FFI extern def。
- 验证：取速查包喂 AI 写 3 个代表性 .px（http server/sqlite/yaml 解析）→ 一次写对率目标 ≥2/3（自测）。

### S3 · registry 拉取闭环
- 侦查 pxpkg 现有实现（tools/pxpkg.px 354 行）：registry 目录解析/URL 下载/lockfile/幂等；评估远程 registry（PX_REGISTRY=http(s) 目录枚举）可行性。
- 构建 `registry/` 资产：9 官库 + 精选 dogfood 库（如 m58_hwmond 分解为可复用模块者）→ `<name>/<version>/<name>.px` 结构入库，附发布说明（随包带 registry/ 或文档指引）。
- 端到端验证：临时项目 `PX_REGISTRY=<repo>/registry`（或远程 URL）→ `pxpkg init` + `pxpkg add semver@^0.1.0` 类 + `pxpkg install` → `import std.semver` 编译/解释双模式可用 + lockfile 生成/校验。
- 若远程 registry 落地为 GitHub Pages/raw URL：补 spec §8.6.3 说明 + README 用例；否则文档化「registry 随仓库分发 + 本地挂载」最小闭环。

### S4 · 写库体验评估（ECOSYSTEM_GAPS.md）
- 四条已知缺口逐一评估：影响场景（写什么库时踩到）/绕行规范（显式传状态、单行数组、let 改 var 等）/修复成本与收益/修复触及面（compiler/parser 哪个文件）。
- 产出 `docs/ECOSYSTEM_GAPS.md`：写库规范 checklist（可跑 `pxcheck` + 双模式 + fmt + lint）+ 各缺口「评估结论：建议修复/暂缓 + 理由」→ 语义修复候选拆 M70（记录，不执行）。
- 顺带把 00-README §4 已知缺口段与新报告对齐（仓库外 qg-issue 侧）。

### S5 · 总闸 + 文档 + 发布
- 回归总闸：stdlib 9 库双模式全过（m66 verify 复用）；pxpkg 端到端（S3 场景重跑）；机器索引/速查生成脚本幂等 + 与运行时一致（native 计数 vs runtime 实际注册数）；examples 抽跑 m58/m65/m67 不回归；全仓 fmt --check + lint 0/0；ci.yml/release.yml YAML；make_release.sh bash -n。
- 文档收口：README/README.en（生态段 + 库表 + 速查入口）、spec（§8.6 registry 若扩展）、ROADMAP（M69 行 + 状态）、CHANGELOG [Unreleased] M69 条目。
- **打 tag `v0.1.0-m69` → push → tag 驱动 workflow 自动发布** → GitHub 产物二次解包冒烟（沿 M68 先例）→ 本机留档 + 发布指引更新至 m69。
- 不开 PR。

## 三、回归总闸（S5 全链复跑）
- stdlib 9 库双模式（编译产物 + pxi run）全 PASS —— 生态库可用性底线。
- capability 双模式一致（M68 后基线不回归）；diffcheck --all 全绿。
- S3 pxpkg 端到端：init/add/install/list/remove + lockfile + import 双模式。
- 生成物一致性：ECOSYSTEM 索引 == stdlib 实扫；CHEATSHEET native 表 == runtime 注册数（脚本重跑 diff 空）。
- 全仓 fmt --check / lint 0/0 / ci+release YAML / make_release.sh bash -n。
- push 后 CI 六 job 全绿。

## 四、风险与预案
| 风险 | 影响 | 预案 |
|---|---|---|
| stdlib API 盘点工作量大/过时 | ECOSYSTEM 表失真 | 机器索引脚本从源码扫顶层 def 生成，人写定位一句；脚本入库 CI 防漂移 |
| native dump 与 runtime 注册源脱节 | 速查表缺项 | dump 脚本直读 runtime.c 注册段（单一事实源），发布前计数对账 |
| pxpkg 远程 registry 评估不过（网络/结构） | S3 闭环打折 | 退「registry/ 随仓库分发 + 本地挂载」最小闭环，文档化远程用法；不阻塞其他 S |
| 语言缺口评估滑向语义修复 | 范围蔓延碰 compiler | S4 铁律「只评估入档」，修复候选拆 M70 记录不执行 |
| 机器索引/速查脚本自身成新维护负担 | 工具噪音 | 脚本小而纯（只扫顶层 def/注册表），并入 CI 校验即自举维护 |
| 发布包不含 docs/（make_release 现状） | 用户拿不到 ECOSYSTEM/速查 | Release 说明附链接 + repo README 即入口；评估 docs 精编件进包（可选） |

## 五、验收清单（里程碑完成判据）
- [ ] `docs/ECOSYSTEM.md`：9 库一览（定位/API 摘要/场景）+ dogfood 资产图 + 消费路径；机器索引脚本入库且 CI 校验防漂移
- [ ] `docs/PUXIAN_CHEATSHEET.md`：native 表由脚本从 runtime dump（与注册数对账一致）+ 语言速查 + 库速查 + 高频模式；整包喂 AI 自测 ≥2/3 一次写对
- [ ] `registry/` 资产随库入库（9 官库 + 精选 dogfood）；pxpkg init/add/install → import 双模式端到端跑通 + lockfile 可复现
- [ ] `docs/ECOSYSTEM_GAPS.md`：四条缺口评估入档 + 写库规范 checklist；语义修复拆 M70 候选记录（未在 M69 执行）
- [ ] README/README.en 生态段升级、spec §8.6 补充（如需）、ROADMAP M69 行、CHANGELOG M69 条目收口
- [ ] 全链回归绿（§三）+ tag `v0.1.0-m69` 发布成功 + GitHub 产物冒烟 + 本机留档 + 发布指引更新
- [ ] 未开 PR；compiler/parser/语言语义零改动

## 六、待用户拍板（2026-09-05 提交）
| # | 决策点 | 推荐 |
|---|---|---|
| D1 | 生态文档（ECOSYSTEM/速查/registry）是否精编进发布包（make_release 现不含 docs/） | 暂不进包，repo README 为入口（评估后可选）；Release 说明附链接 |
| D2 | registry/ 随库入库 + 本地挂载最小闭环（远程 URL 评估通过才扩） | 采纳最小闭环，远程注册评估为加分项 |
| D3 | S4 语言缺口只评估入档、修复拆 M70 | 采纳（M69 不碰 compiler） |
| D4 | S5 打 tag `v0.1.0-m69` 自动发布 | 默认打（沿用 tag 驱动 workflow） |
| D5 | 是否排 ws-todo 场次执行（M68 模式：12:00 起三段） | 待用户定（可今日下午/晚间排，或即时开做） |
| D6 | 需求/贡献通道（README issue 通道已在；PR/Discussions 是否放开） | 维持现状（issue 通道），放开与否另议 |

## 七、执行进度（实时记录 · 东月）
> 2026-09-05 立项起草（工作树，未 commit）。待用户确认 D1–D6 后 commit 入库并开工。
