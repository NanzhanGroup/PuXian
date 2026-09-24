# PuXian 发布流程（Release SOP）

> 仓库治理 · 发布物分发（M62 起 tag 驱动全自动；M159 起含 **aarch64 并列资产**）。
> 当前版本：**`px 0.2.0`**（最新 tag：`v0.2.0-m202`）。

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

**历史缺口（如实登记 · 2026-09-22 复核）**：`M171`–`M177` 无 tag（第 56/57 轮被中断、没走发布步），
`M178`–`M180` 由 `v0.2.0-m180` 一次覆盖 —— 守卫只要求**最高**里程碑有 tag，且**发布包含全部提交**。
更早的缺口同样存在且从未回填：`M53`–`M65`、`M71`、`M85`、`M90`、`M113`、`M126`–`M136`。
⇒ 判据是「**最新 tag 即最新版**」。若某个历史里程碑需要**可安装版本**，只能人工逐个补 tag
（每个 tag = 一次完整 Release，实测 10–19 分钟，且 release.yml 的 `concurrency` 串行化 ⇒ 不可并发）。

**每轮收尾纪律（2026-09-22 补）**：推 `main` 之前先跑一次 `packaging/tag_guard.sh --ref HEAD`；
红了就当场处置（补 tag，或 `TAG_GUARD_ALLOW_MISSING='<理由>'` 留痕），**不要留给定时任务** ——
定时任务是兜底，不是流程。定时任务有 `GRACE_MIN=0`，push 触发只有 45 分钟 grace ⇒
「推 main 后一小时内不打 tag」必然在下一次复查变红。

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
