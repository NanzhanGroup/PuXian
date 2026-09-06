# M86_PLAN · px build 按需自动裁剪（路线甲）+ 命令正名 px（qg-issue 25）

> 创建：2026-09-06 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD M85-S3 = qg-issue 24 完成 · native **301** · stdlib 13 库 · tag v0.1.0-m85 待发）
> 来源：qg-issue **25-px-auto-prune**（M85 交付后用户连续追问 → 拍板「按 路线甲 立项 M86」）
> 用户指令（2026-09-06）：**「按 路线甲 立项 M86，那个 pxc 能不能改为 px？有没有 --help 参数？」**
> 目标：`px build <file>.px`（**裸命令）= 自动按需最小**（引用集驱动裁剪，复用 M85 模块开关链路）；`--full/--max` 逃生舱全能力；命令正名 **px**（pxc 兼容别名）；help 补齐 M85/M86 全部开关
> 性质：**L0 工具链/build 面**（selfhost 语义打点 + tools/px bash 装配器 + runtime.c 条件编译；不改语法/现有 native 语义/native 总数 301）
> 模板：**M85 模块开关链路（S1-S3 已铺）为底座，加"编译器引用集驱动"这一自动层**；乙案（默认 min 反转）经用户决策剔除
> 分批次：**S0→S3 四批**，每批独立开发 + verify + commit；S3 统一重链收口 + 全量回归

## 〇、现状侦查（2026-09-06 实测实录）

- **--help 已有**：`pxc --help` / `-h` / 无参均完整 usage、exit 0；usage 文本在 tools/pxc（bash 前端）L68 起，--version 文本 L545（`echo "pxc $SELFHOST_VER (普贤 PuXian · selfhosted $SELFHOST_MILESTONE)"`；SELFHOST_VER=0.1.0 / SELFHOST_MILESTONE=M-B9a，L61-62）。
- **help 缺口（实测）**：M85 新 flag（--min + 10× --no-*）在 usage 文本 **命中 0** → S0 补。
- **命令架构**：`/usr/bin/pxc` → symlink → `/usr/share/puxian/tools/pxc`（bash 前端 ~29KB）；内部 `PXC_BIN=$PXC_HOME/bootstrap/pxc`（真编译器 9.2MB）等变量分发各子命令后端（pxi/pxl/pxpar/pxfmt/pxlint/pxdoc/pxtest/pxbench/pxlsp/pxcheck/pxmcp），**逻辑不依赖 argv[0]**；脚本 M71-S4 `readlink -f $0` 自发现 PXC_HOME（注释明言为软链设计）→ **改名/加软链安全**。
- **改名面**：词边界 `pxc` 全仓 775 行/294 文件（74 sh 多为历史 verify，**不强制改**，pxc 兼容别名照跑）。核心必改 = tools/pxc 脚本名 + 脚本内 usage/version 文本 + packaging/puxian.spec（/usr/bin/px 双装）+ 各工具 .px"用法:"段 + README/CHEATSHEET 用户入口章节。
- **自动裁剪可行性（M86 S1 依据，四条实证）**：① M85 模块化已就绪（runtime_<mod>.c + px_register_<mod>() + #ifndef PX_NO_<MOD>，native→模块映射零成本）；② 全仓无 px_eval/按名动态调 native → 语言静态，编译期收集引用不漏；③ 自举编译器 codegen 本就要解析每个调用符号 → 语义打点零额外成本零误报，import 递归覆盖；④ M85 裁剪链路（rt 过滤/缓存 key/链接剔除）直接复用。
- **乙案（默认 min + --full）剔除理由**（已对用户说明）：自举/工具链编译自身需全量模块 → 被迫两套默认；破坏向后兼容（旧脚本裸重编 R1001）；与编译型语言"默认全能力、链接器自动裁"惯例倒置。

## 一、范围与边界

**做（S0–S3）**：
- **S0 · 命令正名 px + help 补齐**：
  - tools/pxc → **tools/px**；tools/pxc 保留兼容别名（symlink）；脚本内 usage/注释/version 文本 "pxc"→"px"（含 build/run/… 各子命令行）。
  - 各工具 .px"用法:"段同步（pxbench/pxcheck/pxdoc/pxfmt/pxlint/pxlsp/pxmcp/pxtest/routegen——侦查确认各自显示名后改）。
  - packaging/puxian.spec：装 `/usr/bin/px` + `/usr/bin/pxc → px` 软链。
  - README/CHEATSHEET 用户入口章节 "pxc"→"px"（保留 pxc 别名说明）。
  - **help 补齐 M85 开关**：usage 增 --min/--no-sqlite/--no-ws/--no-zip/--no-xml/--no-aes/--no-rsa/--no-ed25519/--no-route/--no-zlib/--no-h2 说明行（对齐 CHEATSHEET 已同步文案）。
- **S1 · 编译器语义打点收集引用集**：
  - selfhost codegen/语义解析处：收集"被引用 native 名集合"，导出（stdout 或文件）供装配器读取；import 递归（含 stdlib 各库依赖链）。
  - 打点走**旁路**（不改变 codegen 主路径语义）；解析失败/异常源码 → 标记"不可裁剪"，装配器退全量并提示（保编译成功不背锅）。
  - verify：多文件 import / 条件分支引用 / stdlib 调用链 / 异常源码 四类用例的引用集正确性。
- **S2 · 自动折裁剪集 + 逃生舱**：
  - `px build <file>.px`（无裁剪 flag）= 引用集 → "未引用模块集" → 自动折 `--no-xxx` 集合 → 复用 M85 链路（rt 过滤 + 缓存 key 纳开关 + 链接剔除）。
  - `--full` / `--max` = 强制全能力（逃生舱，跳过自动裁剪）；`--min` 保留 = 显式手动全裁。
  - **优先级定稿**：显式裁剪 flag（--no-xxx/--min/--full）> 自动折集；自动只补未显式声明的模块（S1 侦查依赖矩阵后定稿，verify 断言）。
  - verify：hello（print 核心集）裸 build ≈ 自动最小；--full ≈ 9.0M；引用 sqlite 的程序裸 build 不裁 sqlite 可运行；纯 HTTP 服务端裸 build ≈ --no-quic 量级。
- **S3 · 收口**：全能力重链 bootstrap/pxi + 自举证明 + 回归总闸（m82 + m83_s1-s6 + m84_s1-s3 + m85_s1-s2 + m86_s0-s2）+ 文档同步 + CHANGELOG + qg-issue 25 归档 done/ + 00-README 更新（**tag v0.1.0-m86 待用户令**，规划默认推 GitHub release + dnf 仓库 m86）。

**不做（边界）**：
- ⏸️ 乙案（默认 min）不做；方案 C 自动 --gc-sections 仍二期候选不承诺。
- 不改 compiler/parser/语言语义/现有 native 签名；默认裸 build 产物行为变化仅限"未引用模块不再链入"（语义等价 + R1001 可判定兜底）。
- bootstrap/pxi/pxc/pxl 等内部二进制名不改（pxc 仍是编译器内部名，用户入口为 px）；始终全能力构建发布物。
- 历史 examples verify 脚本不批量改写 pxc→px（别名照跑）。

## 二、分批次执行计划

| 批 | 内容 | verify（examples/） | 预计 commit |
|---|---|---|---|
| **S0** | 侦查各工具 usage 显示名 → tools/pxc 改名 tools/px + pxc 兼容别名 → 脚本内 usage/version 文本 px 化 → 各工具 .px"用法:"段同步 → spec 双装 → README/CHEATSHEET 入口更新 → **help 补齐 M85 开关** | m86_s0：`px --help` 含 M85 全开关 & px 全子命令；`px --version`；`pxc --help` 等价；px build/run 冒烟；rpm 双装符号断言 | M86-S0 |
| **S1** | selfhost 语义打点收集引用集（旁路导出）+ import 递归 + 解析异常退全量标记；装配器接收引用集接口 | m86_s1：引用集正确性四类用例（多文件 import/条件分支/stdlib 链/异常退全量）+ 引用集内容断言 | M86-S1 |
| **S2** | 自动折裁剪集（未引用→--no-xxx 自动补）+ --full/--max 逃生舱 + 优先级定稿 + 缓存 key 与自动集整合 | m86_s2：裸 build hello ≈ 自动最小（体积断言 < 3.5M）；--full ≈ 9.0M 全量；引用 sqlite 程序裸 build 可运行；纯 HTTP 服务端裸 build ≈ --no-quic 量级；显式 flag 优先断言 | M86-S2 |
| **S3** | 收口：全能力重链 bootstrap/pxi + 自举证明 + 回归总闸（m82+m83_s1-6+m84_s1-3+m85_s1-2+m86_s0-2）+ 文档 + qg-issue 25 归档 done/ + CHANGELOG + tag v0.1.0-m86（待令） | 全量回归 PASS + 双模式抽查 + fmt/lint 0 + worktree 干净 + CI 绿 | M86-S3 |

## 三、验收清单（S3 收口总闸）
- [ ] `px --help/-h/无参` 完整 usage（含 M85 --min/--no-* + M86 --full/--max/自动裁剪说明）；`px --version`；`pxc` 别名等价；rpm 装 px+pxc 软链
- [ ] 裸 `px build` 自动最小（hello ≈ 2.7~3.5M 量级）；`--full` ≈ 9.0M；引用即保留（sqlite 程序裸 build 可运行）；解析异常退全量提示
- [ ] 引用集收集器四类用例 PASS；显式 flag 优先级 > 自动 断言通过
- [ ] 回归：m82 + m83_s1-s6 + m84_s1-s3 + m85_s1-s2 全绿（默认全能力路径零漂移）；自举证明 rc=0；native 301 不变
- [ ] 文档同步（README/CHEATSHEET/spec/pxi_native_diff/help）；qg-issue 25 归档 done/；00-README 更新
- [ ] tag v0.1.0-m86 推 GitHub（待用户令）；CI completed success；fmt/lint 0 错；worktree 干净

## 四、风险与预案
| 风险 | 预案 |
|---|---|
| 引用集收集器漏 native（复杂分支/宏式遗漏）→ 裁剪态 R1001 | 收集器静态全量遍历 AST 调用点；解析异常/不确定 → 退全量兜底；verify"引用即保留"断言 + 全量回归默认路径零漂移 |
| 自动折集与显式 flag 冲突 | 显式 > 自动（自动只补未声明模块）；help 文档化；verify 断言 |
| px 改名牵连 rpm/CI/文档/历史脚本 | pxc 兼容别名长期保留；spec 双装；历史 verify 不批量改（别名跑） |
| 打点影响编译器性能/正确性 | 旁路导出不扰 codegen 主路径；S3 全量回归 + 自举证明双保险 |
| 缓存 key 未纳自动集 → 裁剪/全量互串 | key 复用 M85 扩展签名（cuts 已纳开关），自动集并入 cuts；verify 断言切换后体积变化 |
| help 文本膨胀（~150 行 + 新开关） | 分组排版（build/裁剪 profile/工具/环境/别名）；单屏可扫 |

---

## 附：执行状态记录（2026-09-06 立项）

| 批 | 状态 | 说明 |
|---|---|---|
| S0 | ✅ done | **命令正名 px**：tools/pxc → tools/px（git mv）+ tools/pxc = symlink px 入库；usage/version/错误提示/注释 px 化（bootstrap/pxc 真名保留）；cmd_mcp PX_PXC→tools/px；spec/install.sh 双软链 /usr/bin/px + pxc→px；make_release RELEASE 模板 px 化；README/en/CHEATSHEET/spec/MINI_SUBSET 入口 px 化；help 补齐 M85 全开关 + M86 pxc 别名说明。verify examples/m86_s0 **14/14 PASS**（默认 9010184 / --no-quic 3929808 零漂移；pxc 别名 build 等价）。commit M86-S0 |
| S1 | ✅ done | **引用采集层（实现修正）**：runtime/native_mod_map.txt（112 native 名=模块，生成器 tools/gen_native_map.sh 零依赖可重生成）；`px refs <file>` 子命令（pxc build C 产物提取 px_get_global 名，import 递归全量；失败非0退全量）。**实践否决原"编译器打点"方案**：打点版 pxc 编 compiler.px 自举触发 runtime 字符串越界（自举不收敛）→ 回滚，改 C 产物静态提取（零编译器/golden 改动、零自举风险、零漏报）。verify examples/m86_s1 **19/19 PASS**。commit M86-S1 |
| S2 | ✅ done | **自动按需裁剪**：cmd_build 加 auto 判定——无 --full/--max 时从已生成 C 产物提取引用集 → native_mod_map 反推被引用模块 → 未引用模块自动补裁（零额外 codegen，复用 M85 cuts/宏/cache-key 隔离链路）；`--full/--max` 逃生舱全能力；显式 flag（--no-xxx/--min/--no-quic/target 物理约束）> 自动，自动只补未声明模块。实测：裸 hello **2,713,472**（自动最小）/ `--full` **9,010,184** / sqlite_dep 裸 3,759,248 保留可运行 / --no-sqlite 显式覆盖 → R1001 / allmod（9 模块）裸 9,001,304 保留。verify examples/m86_s2 **11/11 PASS**。commit M86-S2 |
| S3 | ✅ done | **收口**：回归总闸 **15 批全绿**（m82 + m83_s1-s6 + m84_s1-s3 + m85_s1-s2 + m86_s0-s2；m85/m86 语义适配——M86-S2 起默认裸 build=自动最小，M85 显式裁剪档统一 `--full` 前缀保原意）；自举证明 **rc=0**（compiler.px/golden 未动 → S1 C 产物方案零自举影响实证）；px fmt 新增样例全过；bash -n 语法过；文档同步（README/en CLI 表 M86-S2 说明、CHEATSHEET M86 摘要、spec.md M86 规格段、pxi_native_diff px 化）；qg-issue 25 归档 `done/25-px-auto-prune/` + 00-README（Issue 1–25 全清）。commit M86-S3；**tag v0.1.0-m86 待用户令** |

> 立项 commit：M86 立项落盘（qg-issue 25 ISSUE.md + docs/M86_PLAN.md + 00-README + CHANGELOG）
