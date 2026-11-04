# M374 · 仓库瘦身：bootstrap/ 从历史剥离（2026-11-04）

## 一 为什么做

缺陷 658 的表象是 CI 检出 20–45min 顶超时（Tag Guard 20min、Release el7 45min 被 cancel），
根因是**仓库体积**：GitHub 报 10.04 GB，本地 `.git` 17 GB。

体积的唯一成因是 `bootstrap/`：**把编译产物当源码提交**。
278 轮里程碑、每轮重烘 14 件（131 MB），累积成历史里 **2409 个 blob / 未压缩 21.4 GB**。
其余目录都不是问题（`examples/` 历史 blob 3412 个仅 12.6 MB；`gh-pages` 236 MB）。

## 二 做了什么

```
git filter-repo --path bootstrap/ --invert-paths --force
```

| 项 | 之前 | 之后 |
|---|---|---|
| 本地 `.git` | 17 GB | **89 MB** |
| 历史里 bootstrap blob | 2409 | **14**（仅 HEAD 一份）|
| 可达提交 | 1151 | 1041（22 个「只改 bootstrap」的提交变空被剔除）|
| tag | v0.2.371–374（旧 sha）| v0.2.371–374（新 sha，同名重建）|
| GitHub 标称 size | 10.04 GB | 见 §五（服务端 GC 前不变）|

**HEAD 仍保留 `bootstrap/` 一份**：14 件是自举链起点与打包输入
（`release.yml` aarch64 包、`ci.yml` 全件源码链门、`portability_shipped`、`tools/make_release.sh` 冒烟
都直接读它）。从 HEAD 一并删掉会让构建、CI 与 251 门全部失效。

## 三 旧 sha → 新 sha（关键项）

完整映射：`docs/SHA_MAP_M374.tsv`（1062 条，filter-repo 原始 commit-map）。

| 旧 | 新 | 说明 |
|---|---|---|
| `138b194055e8ddec3eb828e84ada337482de2f42` | `bb4e0ccba82556b73feec704320debeb013a43d0` | tag v0.2.374 指向 |
| `3b7b629023dea4ca3747fd628ae92fd82def5ab9` | `bb166c46fa11779bd4150c6544d1d4e4c8fb9cd6` | tag v0.2.372 指向 |
| `0f4fbed6630d0d99927b43e8d3688c5c0ae9fc8c` | `0197d5b5c90c4b9510653ede659bd72412cc632d` | 重写前 HEAD（M374 三）|
| `e369d5281de03431e6e09a438d93dbc03e97cfe7` | `8468dc6262bce4907084bc39e3f7f114b911d9f8` | M374 收账 |

后续新增：`213dca7`（M374 四，恢复 bootstrap + 本文档）。

## 四 影响（不可逆部分已封存）

- **所有历史 commit sha 变更**，CHANGELOG 与 docs 里旧 sha 成为纯文本，不再可点。
  查旧 sha 一律先看 `docs/SHA_MAP_M374.tsv`。
- 已被删除的旧分支：`feat/pxpkg-sync-w1`、`m308-s4-perfgate`（两者均 0 独有，内容全在 main）。
  `m315-ci-fix` 的 2 个未合并提交已导出 patch 存 `/data/autodrive/m374slim/m315-ci-fix-patches/`。
- stash `M373-diag2` 已导出 patch 后 drop。
- 已发出的下载包文件名仍带旧 sha（如 `puxian-0.2.374-138b1940.tar.gz`），属既定资产名，不影响安装。

## 五 GitHub 侧体积

force push 后，GitHub 的 `size` 字段**不会立刻变小**：
旧对象变为 unreachable，但需服务端 GC 才释放。
`api.github.com/repos/NanzhanGroup/PuXian` 的 `size` 在 10.04 GB 停留属正常。
彻底释放需向 GitHub Support 提交「purge unreachable objects」请求，或等待其不定期 GC。

## 六 回滚

`/data/autodrive/m374slim/git-backup-hardlinks`（17 GB，重写前 `.git` 的硬链接副本）
保留在仓库之外，需要时可整体恢复。确认无回滚需求后删除该目录即释放磁盘。

## 七 后续（独立任务）

`bootstrap/` 仍会在每次重烘后增长。根治做法是**不入库**：
`.gitignore` 它，CI 与本地改为「检出后 `rebake_bin.sh --rebake-all` 现场重建」，
并加一门校验「bootstrap 未入库」。本轮未做，避免与本题变更混在一起。

—— 东月 · 2026-11-04
