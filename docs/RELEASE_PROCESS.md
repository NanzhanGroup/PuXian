# PuXian 发布流程（Release SOP）

> 仓库治理 · 发布物分发（M62 起 tag 驱动全自动；M159 起含 **aarch64 并列资产**）。
> 当前版本：**`px 0.2.0`**（最新 tag 例：`v0.2.0-m167`）。

## 版本语义

Tag 格式：`v<主版本>-m<里程碑>`，例：`v0.2.0-m167`。

- `<主版本>`：SemVer（当前 `0.2.0`），随语言/生态阶段推进。
- `-m<里程碑>`：发布序号，对应最近入版的 M 里程碑（如 m167 含 M164–M167 的语义收口）。
- 发布包名：`puxian-<主版本>-<里程碑>-<sha>.tar.gz`（sha = tag 指向 commit 的短哈希）。
- 并列资产：`puxian-bootstrap-aarch64-<tag>.tar.gz`（**aarch64 原生工具链**，见下）。

## 一键发布（推荐）

```bash
git tag v0.2.0-m167         # 版本决策：tag 指向当前 HEAD（main 应已含待发代码）
git push origin v0.2.0-m167 # 触发 .github/workflows/release.yml
```

`release.yml` 两个 job：

| Job | 产物 | 说明 |
|---|---|---|
| 主包（x86_64） | `puxian-<ver>-m<N>-<sha>.tar.gz` · `sha256sums.txt` | `tools/make_release.sh` 构建（版本自 tag 派生）+ 内置冒烟自检 |
| **aarch64 自举件（M159）** | `puxian-bootstrap-aarch64-<tag>.tar.gz` + `.sha256` | 在 GitHub **原生 arm64 runner** 上 **gcc-only 从源码自举**（含**自证**：新编 pxc 编 `compiler.px` 与 `golden/compiler.c` 逐字节一致）+ 现编核心工具 + 零参数冒烟 + 打包 |

> **为什么要有 aarch64 并列资产**：主包里的 `bootstrap/*` 是 x86_64 件，在 aarch64 上直接跑会
> `Exec format error`。并列包内 `bootstrap/*` 即 aarch64 原生件，解压即可用；也可在任意 aarch64
> 机器上用 `./selfhost/native_bootstrap.sh --install` 自举整套。
>
> ⚠ **上传必须显式 `--repo <owner/repo>`**：`gh release upload` 在 cwd 不在 git 仓库内时（runner 用
> `$RUNNER_TEMP` 打包）无法推断仓库 ⇒ 会失败。已修（M159），历史 tag m159 因此只发布了主包 +
> sha256sums，并列资产自 **m162 起随包发布**。

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
packaging/tag_guard.sh --grace-min 45   # 刚推上来的提交允许窗口期（push 触发用）
```

**判据（唯一）**：被检查提交的 `CHANGELOG.md` **标题行**里最大的里程碑号 `M<NNN>`，
必须存在一个 **可达的** `v*-m<NNN>` tag（容忍前导零；打在旁支上的 tag 不算）。
缺 ⇒ `rc=1` 并打印「里程碑名册 → tag」对照与可直接复制的补打命令；
参数/环境错误（无 CHANGELOG、标题行无里程碑、ref 不存在）⇒ `rc=2`。

只看**标题行**是有意的：正文常出现「⇒ 建议 M129」这类**未来**里程碑，会把判据带偏。

**自动化**：

- `ci.yml` 的「发布侧守卫自测」步跑 `packaging/selftest_tag_guard.sh`（22 断言，守脚本自身）。
- `.github/workflows/tag-guard.yml`：每日 **09:17 CST** 复查 main（主检测路径，0 窗口）
  ＋ `push(main)` 窗口期检查（`--grace-min 45`）＋ 可手动 `workflow_dispatch`。
  失败会把守卫输出写进 run 摘要。

**看到这道门红怎么办**：先问「这个里程碑要不要发」。

- 要发 ⇒ 按上面「一键发布」补 tag（`git tag -a v<版本>-m<NNN> <提交>` 并 push）。
- 不发（纯文档/CI 提交）⇒ 显式放行并与 CHANGELOG 说明：
  `TAG_GUARD_ALLOW_MISSING='<理由>' bash packaging/tag_guard.sh --ref <提交>`。

**边界（如实）**：① 只要求**最高**里程碑有 tag，中间里程碑缺 tag 只提示（发布包含全部提交）；
② 不校验版本段该不该升主版本（人工判断）；③ 只认 `-m<NNN>` 结尾的 tag；
④ GitHub 在仓库 **60 天无活动后停用定时工作流**（不告警）⇒ 长期静默时需手动 dispatch。

## 发布前核对清单

- [ ] `main` 已含待发代码并推送（工作区干净）
- [ ] **入库件已按当前源码重烘**：`./selfhost/rebake_bin.sh && ./selfhost/rebake_bin.sh --check-all`（14/14 指纹一致）
- [ ] 双轨自举证明本地位跑过（`bootstrap_prove.sh` + `bootstrap_prove_bc.sh`）
- [ ] 本地先跑一次 `tools/make_release.sh` 确认冒烟全 PASS（避免 workflow 白跑）
- [ ] 本地全量门无红：`./selfhost/m116_gates.sh`（本地含负控）
- [ ] CHANGELOG 已记录本版变更（含每轮缺陷编号与验证证据）
- [ ] 打 tag 前 `git log --oneline <上一tag>..HEAD` 确认入版范围符合预期
- [ ] 打完 tag 后：**CI / Release / Tag Guard 三个 run 全绿**，且 Release 资产齐全
      （主包 + `sha256sums.txt` + **aarch64 并列包** + 其 `.sha256`）

## 已知边界

- 发布包不含 `selfhost/` 源码（编译器 PuXian 源码）与 git 仓库——仅供「使用 PuXian 开发应用」；
  源码改动走 GitHub issue / PR。aarch64 并列包同样只含工具链与 runtime/stdlib，不含 selfhost 源码
  （需要自举整套时用 `selfhost/native_bootstrap.sh`，它需要仓库源码）。
- `bootstrap/pxi --version` 已支持；`bootstrap/pxc` **不带** `--version` 参数解析（会当文件名），
  正式入口一律走 `tools/px`（官方名，M86-S0）/ `tools/pxc`（兼容别名）。
- 交叉架构发布物只做 **aarch64**（GitHub 原生 arm64 runner 可得）；armv7 / riscv64 走
  `tools/cross_multiarch.sh` 本地现编 + qemu 验证（CI 四档矩阵覆盖）。
