# PuXian Registry（官方包资产）

> M69-S3 建立 · 随仓库分发（`PX_REGISTRY=<本仓库>/registry` 即用）· 结构对齐 `tools/pxpkg`（M45）。

## 结构

```
registry/<name>/<version>/<name>.px
```

- `<name>` = 包名；`<version>` = 语义化版本（semver 2.0.0 子集，pxpkg 用 `sv_best` 解析范围）。
- 内容 = 单文件纯语言库（`.px`）；安装到项目 `.px_modules/<name>/<name>.px`，`import <name>` 即用（裸名 → `.px_modules` 命中；不带 `std.` 前缀）。
- lockfile `px.pkg.lock` 锁定精确版本 + sha256，`install --locked` 可复现/防篡改。

## 官方包（9 · 与 stdlib/ 同源镜像，版本 0.1.0）

| 包 | 来源 | 说明 |
|---|---|---|
| collections / semver / webroute / yaml / pxml / lunar / gfx / png / edge | `stdlib/<name>.px` | 9 个标准库的 registry 分发形态（官方包 = stdlib 镜像，同一事实源） |

> **为什么 stdlib 还要 registry 化**：stdlib 随编译器/发布包内置（`import std.*`）；registry 形态供「按需拉取 + 版本锁定 + 可复现构建」场景（第三方项目/离线分发/自定义 registry），两者内容同源、import 名不同（`std.*` vs 裸名）。

## 用法

```bash
# 官方 registry：仓库根 = registry 目录
export PX_REGISTRY=/path/to/PuXian/registry

mkdir app && cd app
pxpkg init --name myapp
pxpkg add semver@^0.1.0        # registry 解析 ^0.1.0 → 0.1.0
pxpkg add yaml@0.1.0           # 精确版本
pxpkg install                  # → .px_modules/ + px.pkg.lock
# main.px: import semver / import yaml → pxi run / pxc build 双模式可用
```

端到端验证：`bash examples/m69_registry/verify.sh`（init/add/install → import 双模式 → --locked 可复现）。

## 发布新版本 / 新包

1. 新包：`registry/<name>/<version>/<name>.px`（纯函数优先，保持双模式一致；语义见 `docs/ECOSYSTEM_GAPS.md` 写库规范）。
2. 官方 9 库升级：改 `stdlib/<name>.px` 后**同步** `registry/<name>/<newver>/<name>.px`（保留旧版本目录，供 `^` 范围解析与 lockfile 兼容）。
3. 校验：`examples/m69_registry/verify.sh` + `docs/ECOSYSTEM.md §5`（stdlib 索引防漂移）。

## 远程 registry（评估结论，M69-S3）

- **最小闭环（现支持）**：registry 为**本地目录**（`PX_REGISTRY=<dir>`），随仓库分发 → git clone / 下载仓库或目录即远程可用；离线安全。
- **远程增强**：pxpkg 依赖 spec 原生支持单包 `http(s) URL#sha256`（`pxpkg add https://.../lib.px#sha256`）；registry 目录级远程（URL 枚举/索引）当前不支持——如需要，后续里程碑可给 pxpkg 加 `PX_REGISTRY_URL`（拉取远端目录清单再逐包下载），M69 明确不做。

## 一致性防漂移

`registry/<name>/0.1.0/` 与 `stdlib/` 应保持同源（改 stdlib 须同步 registry 或加版本）。CI 步骤：
```bash
for lib in collections edge gfx lunar png pxml semver webroute yaml; do
  diff -q stdlib/$lib.px registry/$lib/0.1.0/$lib.px
done
```
（M69-S5 将其并入总闸；registry 增版本后按版本目录逐一核对。）
