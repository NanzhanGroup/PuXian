# PuXian 发布流程（Release SOP）

> 仓库治理 · 发布物分发（M62 起 tag 驱动全自动；M159 起含 **aarch64 并列资产**）。
> 当前版本：**`px 0.2.272`** —— ⚠ tag 命名：**`v<次段>.<里程碑>`**（M272 用户令 2026-10-05，见下「版本语义」）；
> 查最新：`git tag --sort=-v:refname | head -1`（**不写死**，写死必过期）。

## 版本语义

Tag 格式：`v<次段>.<里程碑>`，例：`v0.2.271`（= M271）。

- `<次段>`：SemVer 的 `MAJOR.MINOR`。**只在「大改进」时手工升**（兼容性变化 / 重大能力面），
  用 `packaging/make_tag.sh --minor 3`；major/minor 跃升一律由人拍板。
- **里程碑号放在 patch 段**（M272 用户令 2026-10-05「里程碑直接当 patch 号」）：
  ① 版本号自带进展；② tag 名与里程碑**一一对应**（不会出现「同一里程碑三个 tag」）；
  ③ 它是**全局轮次计数器**（跨 minor 不重置 —— `v0.3.272` 的 272 仍是 M272）。

### 版本段规则（M272 · 用户令 2026-10-05 · 方案 B）

> 「版本号采用方案 B，**里程碑直接当 patch 号**，`v0.2.171`；改版本号的规则也按你说的来
> （只在真有功能/兼容性变化时才动）……大改进了再升 `v0.3`。」

**规则**：tag = `v<次段>.<里程碑号>`。

| 项 | 规则 |
|---|---|
| **patch 段** | **= 里程碑号**（`M271` ⇒ `v0.2.271`）。全局轮次计数器，跨 minor **不重置** |
| **次段（minor）** | 只在**大改进**时手工 `--minor N` ⇒ 如 `v0.3.272` |
| **主段（major）** | 人拍板（破坏性变更） |
| **纯内部轮**（重构 / 门 / 文档 / 构建脚本） | 照打 tag（patch 段 = 里程碑号即身份），但**不因它而升次段** |

```
v0.2.269 → v0.2.270 → v0.2.271 → v0.2.272 → …      （升次段后：v0.3.<里程碑>）
旧形态（**历史冻结**，M238–M271 用过）: v0.2.0-m167 · v0.2.12-m260
```

**为什么换（三个真问题，M272 用户提名）**：
1. **同一个版本号被 150 个 tag 共用**（`v0.2.0-*` 共 150 个）⇒ 版本号毫无信息量；
2. `-m<NNN>` 在 **SemVer 里是 prerelease** ⇒ `v0.2.19-m271 < v0.2.19`，`sort -V` 与任何 semver
   库都会把它们排在**正式版之前** ⇒ **同步脚本乱的根子**（晨曦 2026-10-05 实测撞上）；
3. 「每轮 +1、到 100 进位 minor」让 `0.2.x` 一百轮耗尽、被迫升 `0.3`，而 API 可能一字未改。

**执行**：`packaging/make_tag.sh`（**不要手写 tag 名**）：

| 调用形态 | 名字行为 |
|---|---|
| `--milestone <N>`（默认路径） | `v<次段>.<N>`；次段自最近 tag 继承 |
| `--minor <N>` | 手工升次段（大改进） |
| `--name <完整名>` | 逐字照用（仍要过命名校验 = 新形态） |
| `--move`（重定向同一 tag 换提交） | 名字不变（那是同一版换个提交） |
| `--dry-run` | 只打印会得到什么名字（**只读**入口 · 无副作用） |

**同名即拒绝**（方案 B 的直接后果）：同一里程碑 = 同一个名字 ⇒ 补丁轮**必须** `--move`
（不带会 rc=4 并提示），**不要**造 `-m271s1`。

**代码侧同步**：版本号另有 **9 处字面量**（`tools/px`、`tools/pxc`、`selfhost/compiler.px`、
`selfhost/interp.px`、`tools/pxfmt.px`、`tools/pxlsp.px`、`tools/pxmcp.px` + 2 份自举基准）——
用 **`packaging/bump_version.sh <X.Y.Z>`** 一条命令改；改完**必须重烘入库件 + 冻结门重定基**
（`selfhost/*.px` 在源码链指纹里）。

- **patch 段 = 里程碑号**（`v0.2.271` 即 M271 · M272 起）；旧形态 `v0.2.0-m167` 历史冻结。
- 发布包名：`puxian-<版本>-<m里程碑>-<sha>.tar.gz`（新形态下如 `puxian-0.2.272-m272-<sha>.tar.gz`；
  sha = tag 指向 commit 的短哈希。rpm 侧：`puxian-0.2.272-1.m272.el9.x86_64.rpm`）。
- 并列资产：`puxian-bootstrap-aarch64-<tag>.tar.gz`（**aarch64 原生工具链**，见下）。

### ⚠ tag 命名规则（硬性 · M238 用户令 2026-10-01）

**现行规则（M272 起）**：`v<次段>.<里程碑>`，例 `v0.2.272`。**没有 `-m` 后缀**。
旧形态 `v<主版本>-m<里程碑>`（`v0.2.0-m167`）为**历史冻结**：守卫仍接受（不做全量改名），
但 `make_tag.sh` **不再允许新建**。

| 场景 | 正确做法 |
|---|---|
| 正常发布 | `packaging/make_tag.sh --milestone 272 --push` ⇒ `v0.2.272` |
| **该里程碑随后又有补丁轮（s1/s2…）** | **把同一个 tag 重定向**（`--move`，见下方命令），**不要**造 `-m272s1` |
| 大改进（兼容性 / 重大能力面） | `--minor 3` ⇒ `v0.3.<里程碑>` |
| 补历史缺口 | 用**新形态**指向该里程碑的最终提交（例：M261 ⇒ `v0.2.261`） |

```bash
# 补丁轮：把同一里程碑的 tag 移到最终提交（**用工具** —— 它保留 annotated 形态与 tag 消息）
packaging/make_tag.sh --move --milestone 272
git push --force origin v0.2.272           # ⚠ 用 --force 重定向（tag 已推过）
#  ⚠ 不要 `git tag -d` 再重建 —— GitHub 会把对应 Release 转成草稿（M206 实测）。
```

**为什么这条规则不能「随意发挥」** —— 2026-10-01 实测事故（用户报障「dnf 镜像不能同步」）：

误打 `v0.2.0-m237s3`（带补丁后缀）⇒ 三段叠加，镜像**停摆两天无人知**：

1. `packaging/pxrepo_mirror.sh` 的版本序正则 `^v[0-9]+\.[0-9]+\.[0-9]+(-m[0-9]+)?$`
   —— **就是这条规则的可执行形式** —— 把它**静默过滤** ⇒ 镜像永远停在旧版；
2. `packaging/build_rpm.sh` 的 `MILESTONE="${TVER#*-}"` 取短横线之后**整段**
   ⇒ rpm 包名被污染成 `puxian-0.2.0-1.m237s3.el9.x86_64.rpm`
   ⇒ 与 gh-pages 的 `.mNNN.` **交叉校验失配**；
3. 同一刻镜像脚本的**回退分支也坏了**（GitHub API 的 `"name": "…"` 冒号后小空格没吃掉，
   叠加 `set -euo pipefail` ⇒ 命令替换非零 ⇒ **脚本静默退出、连 `die` 的 ❌ 都打不出来**）。

**三道防线**（M248 起 · 见 `packaging/README.md`）——从「事后检查」推到「事前不可写错」：

| # | 时刻 | 载体 | 作用 |
|---|---|---|---|
| ① | **创建** | **`packaging/make_tag.sh`**（**唯一入口**） | 不合规的名字**根本打不出来**（exit 3）；补丁后缀形态直接给出合规名与 `--move` 出路；`--dry-run` 是无副作用的只读入口 |
| ② | **构建** | `packaging/build_rpm.sh` · `tools/make_release.sh` | tag 派生的 MILESTONE 必须是 `^m[0-9]+$` ⇒ 否则**响亮 die**（宁可 Release 红，也不把污染包名推出去） |
| ③ | **事后** | `packaging/tag_guard.sh` **③** · `packaging/pxrepo_mirror.sh` | 未登记的违规 tag ⇒ 判红（exit 3）/ 响亮告警（`PXREPO_STRICT_TAGS=1` 时判红）；rpm 树若含 `.m<NNN>s<N>.` 形态 ⇒ **指名判红** |

- 历史遗留（规则确立前）登记在 `packaging/tag_name_exempt.txt` —— **只减不增**，
  且表里出现**已不存在的 tag** 也会判红（防豁免额度永久漂着掩盖新违规）。
- 自测：`bash packaging/selftest_make_tag.sh`（**71 断言** · 覆盖三道防线 · 含负控：
  拆掉判据 ⇒ 必须真的建出/构建出不合规产物）。已注册进 `selfhost/gates.registry.sh` 与 `ci.yml`。

> ⚠ **为什么要有 ①**（2026-10-03 实测差距）：规则此前**只有事后检查** ⇒ 我可以随手
> `git tag v0.2.0-m245s1`，直到 CI / 镜像侧才发现。那次让 gh-pages 出现
> `puxian-0.2.0-1.m245s1.el9.x86_64.rpm` ⇒ 镜像的 `xcheck-rpm-tree` 判红 ⇒ **同步停摆**。
> 事后检查能**发现**，但那时的污染包名**已经在仓库里**了。

> ⚠ **不要「删 tag 再重推」**：GitHub 会把对应 Release **转成草稿**，而 `gh release view`
> 看不到草稿（M206 实测 ⇒ 空壳 release）。要改指向就用 `git push --force` 重定向。

## 一键发布（推荐）

```bash
# 推荐（M248 起）：唯一入口 —— 名字不合规会被**创建时刻**拦下
packaging/make_tag.sh --push        # 自动取 CHANGELOG 最高里程碑 → 建 tag → 推 main+tag（同一次 push）
packaging/make_tag.sh --dry-run     # 先看它打算做什么（只读、无副作用）

# 等价的手工方式（**名字必须自己保证合规**）：
git tag -a v0.2.0-m167 <该里程碑最终提交> -m "M167：摘要"
git push origin main v0.2.0-m167    # ⚠ main 与 tag **同一次** push（消 push 竞态）
```

`release.yml` 两个 job：

| Job | 产物 | 说明 |
|---|---|---|
| 主包（x86_64） | `puxian-<ver>-m<N>-<sha>.tar.gz` · `sha256sums.txt` | `tools/make_release.sh` 构建（版本自 tag 派生）+ 内置冒烟自检 |
| **aarch64 自举件（M159 建立 · M168 静态化）** | `puxian-bootstrap-aarch64-<tag>.tar.gz` + `.sha256` | 在 GitHub **原生 arm64 runner** 上 **gcc-only 从源码自举**（含**自证**：新编 pxc 编 `compiler.px` 与 `golden/compiler.c` 逐字节一致）+ 现编**全部 12 件**原生工具 + **可移植性门**（全静态 + aarch64）+ 零参数冒烟 + 打包 |

> **为什么要有 aarch64 并列资产**：主包里的 `bootstrap/*` 是 x86_64 件，在 aarch64 上直接跑会
> `Exec format error`。并列包内 `bootstrap/*` 即 aarch64 原生件，解压即可用；也可在任意 aarch64
> 机器上用 `./selfhost/native_bootstrap.sh --portable --install` 自举整套。
>
> ⚠ **包内必须全静态（M168 用户报障）**：v0.2.0-m167 及更早的 aarch64 包里 `pxc` 是**动态件**
> （宿主 ubuntu-24.04 的系统 gcc 默认动态链接）⇒ 带 `GLIBC_2.38` 需求，在 openEuler 22.03
> （glibc 2.34）上「装上了、跑不起来」。而 runner 自己就是 2.38 ⇒ 自证永远绿。
> 现在：`--portable` 强制静态 + 包前跑 `selfhost/check_bin_portability.sh --arch aarch64
> --require-static bootstrap/*`（不是全静态即 job 红）；VM 轨两件（`pxc_vm`/`pxi_vm`，
> x86_64 专属）**不随包发布**，aarch64 上 `px build` 自动走 C 轨。
>
> ⚠ **上传必须显式 `--repo <owner/repo>`**：`gh release upload` 在 cwd 不在 git 仓库内时（runner 用
> `$RUNNER_TEMP` 打包）无法推断仓库 ⇒ 会失败。已修（M159），历史 tag m159 因此只发布了主包 +
> sha256sums，并列资产自 **m162 起随包发布**。

## RPM 仓库分发（el7 / el9 / openEuler）

tag 推送同时触发 `release.yml` 的 rpm 链（**GPG secrets 未配置时各 job 自动跳过**，保持绿）：

| Job | 干什么 | 产物 |
|---|---|---|
| `rpm-build-7` | centos:7 容器内**无签名**构建 el7 rpm（EOL vault 源修正 + createrepo gzip） | `pxrepo-7` |
| `rpm-build-9` | rockylinux:9 构建 el9 签名仓库 + **代签 el7** + **入库件可移植性门** + **组 openEuler 别名仓库** | `pxrepo-all` |
| `rpm-verify-7` | centos:7 + yum 3.4 双验签 → 安装 → **真编译运行** + 入库件静态性 ldd 断言 | — |
| `rpm-verify-openeuler`（M168） | `openeuler:22.03-lts` / `24.03-lts` 容器：dnf 双验签 → 装包 → **真编译运行** | — |
| `rpm-publish` | 就绪检查（7/9 + openeuler 别名 + 公钥）→ rsync 推 gh-pages（含站点根 `install-rpm.sh`/`index.html`） | gh-pages |

目录语义（M168 起**显式化**）：`rpm/7/x86_64`、`rpm/9/x86_64`（RHEL 系，repo 文件用
`$releasever/$basearch`）、`rpm/openeuler/<ver>/x86_64`（openEuler，repo 文件写**字面**目录；
元数据 gz，兼容各版本 libdnf）。

> ⚠ **这段布局有判据守着**（M254）：`packaging/selftest_pxrepo_mirror.sh` 的 **[4] rpm 树布局**段 ——
> 真实布局 fixture（4 个 dist × arch 层）必须全过 · 摘掉 `repomd.xml.asc` 必须响亮 ·
> 旧形态（缺 arch）在**同一 fixture 上必然失败** · 静态断言「拼接只许发生在 `rpm_tree_dir()`」。
> **来历**：M168 改这里时**丢了 arch 层**，而 §⑤ **没有任何门覆盖**（本文件明写了布局、实现没照做、
> 也没人守着）⇒ 裸奔 **2 天 23 小时**，直到下游（晨曦）干跑一遍才撞到 —— 而且是在
> 「已经拉完数百 MB 资产之后」才死。**改这一段之前先看门的 [4] 段。**

> ⚠ **为什么必须真跑一次 `px build`**：M168 前 el7 终验只跑 `pxc --version` —— 走的是 C 轨
> 静态件，恰好正常；而**用户面默认 VM 轨**用的 `bootstrap/pxc_vm` 当时是动态件（需 GLIBC_2.34），
> 在 el7（glibc 2.17）上一执行就崩 ⇒ 整类缺陷看不见。现在两个终验脚本都真编译 + 运行，
> 并断言入库件全静态。
>
> ⚠ **el7 的已知限制（M168 实测登记）**：`runtime/runtime.c` 需要 C11 `<stdatomic.h>`
> ⇒ **gcc ≥ 4.9**；CentOS 7 自带 4.8.5 ⇒ `px build` 在 el7 上需 SCL：
> `yum install -y centos-release-scl devtoolset-9` + `scl enable devtoolset-9 bash`
> （或 `PX_CC=<devtoolset gcc>`）。`tools/px` 在编译 runtime 前预检并给出该指引；
> el7 终验的判据是「要么真跑通，要么失败原因必须是**已登记且可执行**的那条」，否则判红。
>
> ⚠ **最终裁定（2026-09-21 · 用户指令）**：el7 的 gcc 是**用户侧**要解决的问题 —— **请自行升级 gcc 到 ≥ 4.9**
> （el7 装 SCL `devtoolset-9`，或升级发行版）。PuXian **不提供 gcc < 4.9 的回退**：`runtime` 的 C11
> `<stdatomic.h>` 是硬需求，为 4.8 加兼容层会拖慢并复杂化核心热路径。`tools/px` 的能力预检 + 指引
> 就是这条裁定的**用户出口**；终验契约不变（要么真跑通，要么失败原因是这条**已登记且可执行**的）。
> 据此，原「runtime 对 glibc 2.17 全兼容」候选**撤销**。
>
> ⚠ **失败诊断必须走 `::error::` 注解**（M168 教训）：本仓 job 日志对非管理员**不可读**
> （API 403 "Must have admin rights"），页面上也只有 step 名 —— 门红了却读不到原因等于没有门。
> 所有新加的判据步骤都要把日志尾部 `sed 's/^/::error::…/'` 打出来（公开 check-runs API 可读）。

构建冒烟自检（任一失败即 workflow 失败、不产生 Release）：① `pxc --version` ② hello 编译（静态 ELF）+ 运行
③ hello 解释运行 ④ `import std.collections` 编译（stdlib 定位）⑤ 包外目录 + `PX_STDLIB` env 编译。

## 本地手工打包（无 GitHub / 预检）

```bash
tools/make_release.sh            # 里程碑取最近 tag 或最近提交里的 Mxx
tools/make_release.sh --no-check # 跳过冒烟自检
tools/make_release.sh -o /tmp/x.tar.gz
tools/install.sh --prefix ~/.local   # 从包/仓库一键安装（sha256 校验 + PX_STDLIB 自动注入）
```

## 守卫：漏打 tag 不再静默（qg-issue 86）

**发布只由 tag 驱动** ⇒ 推 `main` **不会**发布任何东西，两件事必须都做。
2026-09-17 实况：M126 / M127 / M128 连续三个里程碑只推 main、一个 tag 都没打，
Releases 页最新仍是 `v0.2.0-m125`，而 CI 全绿 —— 没有任何信号。
为此有 `packaging/tag_guard.sh`：

```bash
packaging/tag_guard.sh                  # 检查 HEAD
packaging/tag_guard.sh --ref <提交>      # 检查指定提交
packaging/tag_guard.sh --grace-min 180  # 刚推上来的提交允许窗口期（push 触发用；见下"窗口为什么是 180"）
```

**判据（唯一）**：被检查提交的 `CHANGELOG.md` **标题行**里最大的里程碑号 `M<NNN>`，
必须存在一个 **可达的** `v*-m<NNN>` tag（容忍前导零；打在旁支上的 tag 不算）。
缺 ⇒ `rc=1` 并打印「里程碑名册 → tag」对照与可直接复制的补打命令；
参数/环境错误（无 CHANGELOG、标题行无里程碑、ref 不存在）⇒ `rc=2`。

只看**标题行**是有意的：正文常出现「⇒ 建议 M129」这类**未来**里程碑，会把判据带偏。

**自动化**：

- `ci.yml` 的「发布侧守卫自测」步跑 `packaging/selftest_tag_guard.sh`（22 断言，守脚本自身）。
- `.github/workflows/tag-guard.yml`：每日 **09:17 CST** 复查 main（主检测路径，0 窗口）
  ＋ `push(main)` 窗口期检查（`--grace-min 180`）＋ 可手动 `workflow_dispatch`。
  失败会把守卫输出写进 run 摘要。

**窗口为什么是 180 分钟（M215 收尾 · 2026-09-26 实测事故）**：
本仓的**正常轮次**是「先 commit（全量门 `m116_gates.sh` 拒绝脏树）→ 跑 m116 全量门
（实测 **~70 分钟**）→ 再 push」⇒ commit 与 push 之间**天然隔着 80+ 分钟**。
原窗口是 45 分钟 ⇒ **必然被突破、push 触发的 run 必然误红**（当日实测：16:45 commit →
18:08 push，年龄 **83 分钟**）。而那道红条只说「缺发布 tag」，**读不出真因是「窗口太小」**
—— 按本仓纪律「门的红必须能读出真因」，两处一起改：
① 窗口 45 → **180**（覆盖「提交→跑全门→推送」）；② 守卫**无条件打印「距今年龄」**信息行，
判红时 run 摘要里能直接读到「年龄 vs grace」。
⚠️ 真正的强制路径始终是**每日定时**（`GRACE_MIN=0`）—— 窗口只服务于"别在正常发布节奏里误报"。

**看到这道门红怎么办**：先问「这个里程碑要不要发」。

- 要发 ⇒ 按上面「一键发布」补 tag（`git tag -a v<版本>-m<NNN> <提交>` 并 push）。
- 不发（纯文档/CI 提交）⇒ 显式放行并与 CHANGELOG 说明：
  `TAG_GUARD_ALLOW_MISSING='<理由>' bash packaging/tag_guard.sh --ref <提交>`。

**边界（如实）**：① 只要求**最高**里程碑有 tag，中间里程碑缺 tag 只提示（发布包含全部提交）；
② 不校验版本段该不该升主版本（**人工判断** —— patch 段已由 `make_tag.sh` 机械递增，
   minor/major 的跃升仍由人拍板：这正是 2026-10-03 用户点名「暂不升 0.3.0、下次用 0.2.1」的那个决定）；③ 只认 `-m<NNN>` 结尾的 tag；
④ GitHub 在仓库 **60 天无活动后停用定时工作流**（不告警）⇒ 长期静默时需手动 dispatch。

**历史缺口（如实登记 · 2026-09-22 复核）**：`M171`–`M177` 无 tag（第 56/57 轮被中断、没走发布步），
`M178`–`M180` 由 `v0.2.0-m180` 一次覆盖 —— 守卫只要求**最高**里程碑有 tag，且**发布包含全部提交**。
更早的缺口同样存在且从未回填：`M53`–`M65`、`M71`、`M85`、`M90`、`M113`、`M126`–`M136`。
⇒ 判据是「**最新 tag 即最新版**」。若某个历史里程碑需要**可安装版本**，只能人工逐个补 tag
（每个 tag = 一次完整 Release，实测 10–19 分钟，且 release.yml 的 `concurrency` 串行化 ⇒ 不可并发）。

**每轮收尾纪律（2026-09-22 补）**：推 `main` 之前先跑一次 `packaging/tag_guard.sh --ref HEAD`；
红了就当场处置（补 tag，或 `TAG_GUARD_ALLOW_MISSING='<理由>'` 留痕），**不要留给定时任务** ——
定时任务是兜底，不是流程。定时任务有 `GRACE_MIN=0`，push 触发有 **180 分钟** grace ⇒
「推 main 后**三小时**内不打 tag」会在 push 触发的复查里被跳过（但**次日定时**必红）；
⚠️ 反过来说：push 触发的绿**不能**当作「发布侧没问题」，**只有定时那次才算数**。

**推送次序（M245 补 · 2026-10-03 实测事故 · ⚠️ 确定会复发）**：`main` 与 `tag` **必须同一次 push 推出去**：

```bash
git push origin main v0.2.0-mNNN                        # ✅ 一次推两个 ref：run 的快照里两者都在
git push origin main && git push origin v0.2.0-mNNN     # ❌ 竞态（下面就是这个形状）
```

为什么：`tag-guard.yml` 由 `push(main)` 触发 ⇒ run **数秒内**创建、job 立即起、`actions/checkout`
（`fetch-depth: 0`，本仓实测耗时 **3.5 分钟**）随即开始抓 refs。若 tag 是在原 push **之后**才推的，
checkout 的快照里就没有它 ⇒ 守卫判「缺 tag」判红，**而远端其实已经有了**。
实测（run **#192** · job `111057186097`）：main push **22:34:23Z** → job **22:34:25Z** 起 →
checkout fetch **22:34:27Z** 开始 → tag **22:34:26Z** 才创建并紧随推送 ⇒ 竞态成立。

⚠️ **触发条件 = 提交年龄 > grace(180 min)**（守卫不跳过、当场判）。本仓「commit（全量门拒绝脏树）
→ 跑全量门 **100+ 分钟** → 再 push」的正常节奏**正好越过**这条线（实测该提交年龄 3h44m）
⇒ **这是确定会复发的竞态**，不是偶发。

守卫侧已加**补取兜底**（判「缺」之前 `git fetch --tags` 一次并复判；`TAG_GUARD_NO_FETCH=1` 可关）
+ 自测 **K 段**（两份干净快照 + 负控「禁补取必红」）。
⇒ 但**次序仍照上面写**：兜底是安全网，不是「随便推」的借口。

## 发布前核对清单

- [ ] `main` 已含待发代码并推送（工作区干净）
- [ ] **入库件已按当前源码重烘**：`./selfhost/rebake_bin.sh && ./selfhost/rebake_bin.sh --check-all`（14/14 指纹一致）
- [ ] 双轨自举证明本地位跑过（`bootstrap_prove.sh` + `bootstrap_prove_bc.sh`）
- [ ] 本地先跑一次 `tools/make_release.sh` 确认冒烟全 PASS（避免 workflow 白跑）
- [ ] 本地全量门无红：`./selfhost/run_gates.sh`（本地含负控；旧名 `m116_gates.sh` 仍可用 = 兼容转发）
      · 迭代期可用 `--only <门名表>` / `--fail-fast` 快速验回归（**发布前必须跑全量**）
- [ ] CHANGELOG 已记录本版变更（含每轮缺陷编号与验证证据）
- [ ] 打 tag 前 `git log --oneline <上一tag>..HEAD` 确认入版范围符合预期
- [ ] 打完 tag 后：**CI / Release / Tag Guard 三个 run 全绿**，且 Release 资产齐全
      （主包 + `sha256sums.txt` + **aarch64 并列包** + 其 `.sha256`）
- [ ] **推送次序**：`git push origin main v0.2.0-mNNN`（**一次推两个 ref**）——
      分批推会与 `push(main)` 触发的 checkout 竞态（M245 补 · run #192 实测）
- [ ] **轮末复核**：`packaging/tag_guard.sh --ref HEAD` ⇒ `rc=0`（红即按「守卫」节处置，别留给定时任务）

## 零停机升级（M176 · SO_REUSEPORT）

> 面向**用 PuXian 跑服务**的人（`px_serve` / `http_serve` 的生产栈）。
> 门：`examples/m176_reuseport/`（`M176-VERIFY-OK` · 真机时序 + 负控）。

**一句话**：让新旧进程**同时**监听同一端口，再让旧进程优雅排空 —— 监听套接字没有一刻为空。

```px
# 服务端启动时开一次（之后每次升级都受益）
px_serve(PORT, DOCROOT, 10000, {"reuse_port": true})
# http_serve 同键；sse_serve / tcp_listen 没有 opts 参数 ⇒ 用环境变量：
#   PX_REUSE_PORT=1 ./myserver
```

升级三步（**顺序不能颠倒**）：

```bash
# ① 起新进程（同端口；此刻新旧同时在监听，内核按 4 元组哈希分流）
setsid nohup /path/new/myserver > /var/log/myserver.new.log 2>&1 &
NEWPID=$!
# ② 健康门（TCP + 真请求，别只看进程在不在）
for i in $(seq 1 50); do curl -fsS http://127.0.0.1:$PORT/ >/dev/null && break; sleep 0.1; done
# ③ 旧进程优雅退出（M27 起：停 accept + 等在途请求完成）
kill -TERM $OLDPID
```

⚠️ **两个前提**（不满足就不是零停机）：

1. **内核要求双方都 opt-in** —— 升级链里**旧版也必须带 `reuse_port` 启动**。
   若旧版没开，新进程 `bind` 会拿到 `EADDRINUSE`（错误文案会带 `Address already in use`）——
   这时只能退回「重启式升级」（有一个短暂空窗，用 ①②③ 的顺序仍能把空窗压到最小：
   先探活新进程、再切流量、最后停旧进程）。
2. **在途请求要能收尾** —— handler 里别用「无上界」的阻塞调用（`SO_RCVTIMEO`/超时要显式设），
   否则 `kill -TERM` 之后旧进程会一直等在途请求，排空时间不可控。

## 已知边界

- 发布包不含 `selfhost/` 源码（编译器 PuXian 源码）与 git 仓库——仅供「使用 PuXian 开发应用」；
  源码改动走 GitHub issue / PR。aarch64 并列包同样只含工具链与 runtime/stdlib，不含 selfhost 源码
  （需要自举整套时用 `selfhost/native_bootstrap.sh`，它需要仓库源码）。
- `bootstrap/pxi --version` 已支持；`bootstrap/pxc` **不带** `--version` 参数解析（会当文件名），
  正式入口一律走 `tools/px`（官方名，M86-S0）/ `tools/pxc`（兼容别名）。
- 交叉架构发布物只做 **aarch64**（GitHub 原生 arm64 runner 可得）；armv7 / riscv64 走
  `tools/cross_multiarch.sh` 本地现编 + qemu 验证（CI 四档矩阵覆盖）。
