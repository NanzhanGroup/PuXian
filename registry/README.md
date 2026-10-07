# PuXian Registry（官方包资产）

> M69-S3 建立 · 随仓库分发（`PX_REGISTRY=<本仓库>/registry` 即用）· 结构对齐 `tools/pxpkg`（M45）。

## 结构

```
registry/<name>/<version>/<name>.px          # 入口（恒为 <name>.px）
registry/<name>/<version>/<分包>.px          # 可选：多文件包（M187 起）
```

- `<name>` = 包名；`<version>` = 语义化版本（semver 2.0.0 子集，pxpkg 用 `sv_best` 解析范围）。
- 内容 = 纯语言库（`.px`）；安装到项目 `.px_modules/<name>/`，`import <name>` 即用（裸名 → `.px_modules` 命中；不带 `std.` 前缀）。
- **多文件包（M187 起）**：包目录下的**全部 `.px`** 随包分发，入口恒为 `<name>.px`，包内相对
  `import "子文件.px"` 原样保留。动机：官方写库规范要求**每文件 <500 行、超了拆**，
  而旧结构只认单文件 ⇒ 规范与分发形态自相冲突（qrcode 4 文件 / mysql 3 / xlsx 2）。
- lockfile `px.pkg.lock` 锁定精确版本 + sha256，`install --locked` 可复现/防篡改。
  digest 口径：**单文件包 = 入口文件 sha256**（与 M45 格式逐字节兼容）；
  **多文件包 = 各「文件名:内容」串接后 sha256**（含辅助文件 ⇒ 篡改任何一个都会被 `--locked` 拒）。

## 官方包（13 · 与 stdlib/ 同源镜像，版本 0.1.0）

| 包 | 来源 | 说明 |
|---|---|---|
| collections / semver / webroute / yaml / pxml / lunar / gfx / png / edge | `stdlib/<name>.px` | 9 个早期标准库的 registry 分发形态 |
| html / cookiejar / multipart / smtp | `stdlib/<name>.px` | M83-S5 新增 4 库（HTML 容错解析 / 会话 Cookie / multipart 上传 / SMTP 客户端），官方包 = stdlib 镜像 |

> **为什么 stdlib 还要 registry 化**：stdlib 随编译器/发布包内置（`import std.*`）；registry 形态供「按需拉取 + 版本锁定 + 可复现构建」场景（第三方项目/离线分发/自定义 registry），两者内容同源、import 名不同（`std.*` vs 裸名）。

## 第三方包（M187 引入 · **M282 上游 0.2.0 全量再引入** · 来源与许可见 `THIRD_PARTY.md`）

> 来源：`github.com/banshanhanfu/registry-px`（**Apache-2.0**，与本仓同族）@ `01f6048`。
> 纪律：**逐字节照搬**（不改上游一个字节）—— 来源/许可登记在 [`THIRD_PARTY.md`](THIRD_PARTY.md)，
> 便于与上游 sha256 直接对拍；引入器 `tools/import_registry_px.sh`（`--apply` 才写盘）。

> **逐包清单不在这里** —— 权威表是 [`THIRD_PARTY.md`](THIRD_PARTY.md)（**由引入器自动生成**，
> 逐包记版本 / 文件数 / 入口 sha256 / 三轨验证 / 补丁），并由 `examples/m187_registry_import/verify.sh`
> **重算 sha256 与磁盘对拍**防漂移。本文件只讲结构、纪律与消费路径，避免复述易腐烂的计数。
>
> 按用途粗分（**完整名单见 `THIRD_PARTY.md`**）：编码/校验（base58 / checksum / bytes_pack / xlsx / pdf …）·
> 解析/格式（csv / toml / ini / dotenv / glob / template …）· 数据结构与算法（big / decimal /
> fractions / bisect / itertools / stats …）· Web 与文本（cli / log / ansi / strcase …）·
> 安全与标识（jwt / uuid / ulid / validator / idcard / cnnum …）· 系统与资源（config / retry /
> testkit / fsnotify …）· **需外部条件**（qrcode 解释轨慢 · pg / mysql 需真实服务端 ·
> concurrent_map / workerpool 需编译模式）。
>
> ⚠️ **`passhash`**：其 `pass_verify` 用 `int(parts[1])` 直接吃畸形 hash 串，在 M184「严格解析」
> 后语义不再正确 —— 已由 **M189 加本地补丁**引入（见 `THIRD_PARTY.md` 的「补丁」列与
> `tools/patches/registry-px/`），**上游修好后撤销补丁即可**。

- 验证：`bash examples/m187_registry_import/verify.sh`（引入表逐行 sha256 对拍 + **全量**包装/import +
  抽样双轨编译 + 多文件包语义 + 负控 A/B/C）。
- 验证（本轮口径）：`bash examples/m282_registry_020/verify.sh`（0.2.0 引入完整性 + 凭据守卫 +
  **ftp 0.2.0 补覆盖**）+ `bash selfhost/run_upstream_tests.sh`。
- 引入清单规模沿革（**不写死当下数字，只说口径**）：
  · M186：53 库（编译轨 50/53 · 解释轨 47/53，逐条定性见 `docs/PX_DEF_TRIAGE.md` §4.1）
  · M200：86 库 · M201：98 包 · M214：124 包 · **M282：上游 122 包 × 双版本目录 = 244 行（THIRD_PARTY.md）· registry 版本目录 257（含 13 个本仓自建包）**
- **逐版本目录**：M282 起上游每个包都有 `0.1.0` 与 `0.2.0` 两份 ⇒ `registry/<name>/<ver>/`。
  0.2.0 的引入结果与**上游用例的逐条定性**（含 12 条 `XFAIL` 上游自相矛盾）见
  **`docs/UPSTREAM_020_DEFECTS.md`**；本文件不再复述易腐烂的计数。

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

1. 新包：`registry/<name>/<version>/<name>.px`（纯函数优先，保持双模式一致；语义见 `docs/ECOSYSTEM_GAPS.md` 写库规范）；**多文件包**就把同目录的其它 `.px` 一并放进来（入口名不变）。
2. 官方 9 库升级：改 `stdlib/<name>.px` 后**同步** `registry/<name>/<newver>/<name>.px`（保留旧版本目录，供 `^` 范围解析与 lockfile 兼容）。
3. **引第三方包**：`tools/import_registry_px.sh --src <上游检出> [--exclude a,b] --apply`（逐字节照搬 + 重写 `THIRD_PARTY.md`）。
4. 校验：`examples/m69_registry/verify.sh`（stdlib 镜像闭环）+ `examples/m187_registry_import/verify.sh`（第三方引入 + 多文件包）
   + `docs/ECOSYSTEM.md §5`（stdlib 索引防漂移）。

## 远程 registry（评估结论，M69-S3）

- **最小闭环（现支持）**：registry 为**本地目录**（`PX_REGISTRY=<dir>`），随仓库分发 → git clone / 下载仓库或目录即远程可用；离线安全。
- **远程增强**：pxpkg 依赖 spec 原生支持单包 `http(s) URL#sha256`（`pxpkg add https://.../lib.px#sha256`）；registry 目录级远程（URL 枚举/索引）当前不支持——如需要，后续里程碑可给 pxpkg 加 `PX_REGISTRY_URL`（拉取远端目录清单再逐包下载），M69 明确不做。

## 一致性防漂移

`registry/<name>/0.1.0/` 与 `stdlib/` 应保持同源（改 stdlib 须同步 registry 或加版本）。CI 步骤：
```bash
for lib in collections cookiejar edge gfx html lunar multipart png pxml semver smtp webroute yaml; do
  diff -q stdlib/$lib.px registry/$lib/0.1.0/$lib.px
done
```
（M69-S5 将其并入总闸；registry 增版本后按版本目录逐一核对。）
