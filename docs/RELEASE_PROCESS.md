# PuXian 发布流程（Release SOP）

> 仓库治理 · 发布物分发（M62 起 tag 驱动全自动）。

## 版本语义

Tag 格式：`v<主版本>-m<里程碑>`，例：`v0.1.0-m62`。

- `<主版本>`：SemVer（当前 0.1.0），随语言/生态阶段推进。
- `-m<里程碑>`：发布序号（对应最近入版的 M 里程碑，如 m62 含 M57–M61 与发布自动化）。
- 发布包名：`puxian-<主版本>-<里程碑>-<sha>.tar.gz`（sha 为 tag 指向 commit 的短哈希）。

## 一键发布（推荐）

```bash
git tag v0.1.0-m62          # 版本决策：tag 指向当前 HEAD（main 应已含待发代码）
git push origin v0.1.0-m62  # 触发 GitHub Actions .github/workflows/release.yml
```

workflow 自动完成：

1. `tools/make_release.sh` 构建发布包（版本自 tag 派生，见脚本"版本来源"段）；
   内置解包冒烟自检：① `pxc --version` ② hello 编译(静态 ELF)+运行
   ③ hello 解释运行 ④ `import std.collections` 编译（stdlib 定位）
   ⑤ 包外目录 + `PX_STDLIB` env 编译。任一失败 → workflow 失败，不产生 Release。
2. `gh` CLI 创建 GitHub Release（`GITHUB_TOKEN`，零第三方 action），
   notes 含 SHA-256 与上一 tag 起的提交列表；tarball 作为 Release 资产上传。

## 本地手工打包（无 GitHub / 预检）

```bash
tools/make_release.sh            # 里程碑取最近 tag 或最近提交里的 Mxx
tools/make_release.sh --no-check # 跳过冒烟自检
tools/make_release.sh -o /tmp/x.tar.gz
```

## 发布前核对清单

- [ ] `main` 已含待发代码并推送（工作区干净）
- [ ] 本地先跑一次 `tools/make_release.sh` 确认冒烟全 PASS（避免 workflow 白跑）
- [ ] CHANGELOG 已记录本版变更
- [ ] 打 tag 前 `git log --oneline <上一tag>..HEAD` 确认入版范围符合预期

## 已知边界

- `bootstrap/pxi --version` 已支持（M62）；`bootstrap/pxc --version` 仍把参数当文件
  （编译器本体入口未加参数解析），正式入口一律走 `tools/pxc`（bash 包装，
  已支持 `--version`/`help`）。
- 发布包不含 selfhost/ 源码（编译器 PuXian 源码）与 git 仓库——仅供"使用
  PuXian 开发应用"，源码改动走 GitHub issue / PR。
