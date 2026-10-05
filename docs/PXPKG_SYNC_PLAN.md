# 立项：`pxpkg sync` —— registry 在线分发（M273 起）

> **令源**：用户 2026-10-05 ——「**pxpkg sync 立项**」。
> **需求输入**：晨曦（wsa-chenxi，QA）《需求输入：请给 pxpkg 加 `pxpkg sync`（registry 在线分发）》·
> 总线 `m18db271f434e05479904f400` 之后的 `chenxi-pxpkg-sync-20261005.md`（**7,507 B**）。
> **相关旧决策**：M69 曾定「registry 目录级远程（URL 枚举/索引）**明确不做**」⇒ 本文是**重评估**。

---

## 一 问题（晨曦实测，非推测）

```
$ rpm -ql puxian | grep -E 'pxpkg|registry'
/usr/share/puxian/tools/pxpkg        ✅ 包管理器在
/usr/share/puxian/tools/pxpkg.px     ✅
                                      ❌ registry/ 目录本身不在包里
```

`pxpkg` 当前只有两条取包路径：

| 路径 | 形态 | 局限 |
|---|---|---|
| `PX_REGISTRY=<目录>` | **本地目录** | 用户必须先自己有那份树（clone / 解压） |
| `pxpkg add <url>#sha256` | **单文件** | 一次一个包，无依赖闭包 |

⇒ **缺的不是「分发站」，是「客户端能消费远程目录」这个能力。**
站建得再好，用户仍要手动 clone 或解压 —— 这正是 M69 想避免的情况。

**规模**（`registry/` @ `1ace2d2a` 实测）：顶层 137 条（135 包目录 + 2 文档）· 146 文件 ·
原始 **808,112 B** · gzip 归档 **230,720 B** · 结构 `registry/<包名>/<版本>/<包名>.px`（与 pxpkg 约定一致）。

---

## 二 对端（已就绪 · 零上游改动）

晨曦已在**镜像侧**把分发包做好并境内外双活（走既有 soft-mirror，事件驱动 rsync → 广州边缘）：

```
https://soft.wsai.chat/puxian-registry/      （源站 · 香港）
https://soft.xiusoft.cn/puxian-registry/     （境内边缘 · 广州 193.112.177.58）
```

| 文件 | 说明 |
|---|---|
| `puxian-registry-0.2.12-m260-1ace2d2a.tar.gz` | 230,720 B · sha256 `1fe0567e…54c29` |
| `sha256sums.txt` | 归档 sha256 |
| `files.sha256` | **146 行逐文件 sha256**（`sha256sum` 格式，路径含 `registry/` 前缀） |
| `version.json` | 机器可读元数据（version / commit / 计数 / 哈希 / **清单引用** / **复现命令** `build.cmd`） |
| `index.html` | 用法落地页 |

**确定性打包**（同一 commit 反复打包 sha256 一致，已实测两次）：
固定 mtime（= commit 时间）· owner/group=0 · 按名排序 · `gzip -9n`。

**用户路径已实测跑通**：下载 → 解压 → `sha256sum -c files.sha256` **146/146 OK** →
`pxpkg init/add/install` 落地 `.px_modules/semver/` + `px.pkg.lock`；源站与边缘归档 sha256 一致；
发布→边缘落地 **约 32 秒**。

> 该归档可直接当作 `pxpkg sync` 的**对端样本**用于开发与自测。

---

## 三 目标 / 非目标

**目标**：用户只凭一个 URL 就能把 registry 同步到本地，**增量、原子、可验、可复现**。

**非目标**（本轮明确不做）：包签名/信任链（另立项）· 私有 registry 鉴权 · 依赖求解算法改动
（`pxpkg` 的解析逻辑不动）· 多 registry 联邦/优先级。

---

## 四 契约（**待用户拍板后固化**；下为上游拟定口径）

```
pxpkg sync [--base <url>] [--registry <dir>] [--dry-run] [--force]
```

| # | 契约点 | 说明 |
|---|---|---|
| 1 | **入口** | `GET <base>/version.json` ⇒ `version` / `commit` / `files_manifest` |
| 2 | **清单** | `GET <base>/<files_manifest>` ⇒ `sha256sum` 格式，**路径含 `registry/` 前缀** |
| 3 | **增量** | 与本地清单比对，只下差异文件；**体量小时允许整包归档**（230 KB，一次拿下更简单） |
| 4 | **原子写入** | 临时目录 → `rename`；**任一步失败 ⇒ 本地 registry 保持原样**（不留半残） |
| 5 | **幂等短路** | `version` + `commit` + 清单指纹相同 ⇒ **立即退出 0**，不重复下载 |
| 6 | **落盘即验** | 每个文件写完**立即**校验 sha256，不符即中止 |
| 7 | **失败语义** | 非 0 退出；**绝不**把本地 registry 改成「一半新一半旧」 |
| 8 | **缓存 key** | 与 `.rtcache` 同思路：由「**真正决定行为的输入**」驱动（含工具链/格式版本），版本变即失效 |

**默认 `--base`**：`https://soft.xiusoft.cn/puxian-registry/`（境内边缘优先，源站兜底；
与 `install-rpm.sh` 的 `MIRROR_ROOT → UPSTREAM_ROOT` 同款双通道思路）。

---

## 五 上游侧要做的三件事

| W | 内容 | 备注 |
|---|---|---|
| **W1** | **`pxpkg sync` 客户端**（三～四节契约） | 主体工作；纯 `.px`/shell，无新依赖 |
| **W2** | **把 `registry/` 纳入发布物**（晨曦方案 A） | ⚠ **需用户拍板**：会改变 tarball/rpm 的内容清单（体积 +230 KB 压缩后） |
| **W3** | **`pxrepo_mirror.sh` 的 registry 支持** | 晨曦实测陷阱：发布段是 `rsync -a --delete --exclude=/rpm/` ⇒ `DEST` 下**其它内容会被删**。若希望 URL 是 `…/puxian/registry/`（更直觉），需加 `--exclude=/registry/` 或把 registry 纳入 STAGING。**这是上游文件，晨曦不便擅改（对）** |

**W4（门）**：离线夹具（本地 `file://` 或用晨曦的归档做对端样本）× 全契约点 × 负控。

---

## 六 判据（六条硬线 · 立项目标不是「写完了」而是「可判定」）

1. **原子性**：注入「下载中途失败」⇒ 本地 registry **逐字节等于同步前**（快照比对）。
2. **幂等**：连跑两次 ⇒ 第二次 **0 下载**（且 rc=0）。
3. **落盘即验**：篡改对端清单里任一文件的 sha256 ⇒ **必须**中止且指名文件。
4. **失败语义**：对端 404 / 网络中断 / 清单缺失 ⇒ 非 0 且本地不变。
5. **可复现**：同一 `commit` 的归档 sha256 稳定（对端已自证；我方**复算**一次）。
6. **不带病分发**：`--dry-run` **零副作用**（只打印将发生什么）。

---

## 七 边界与风险

- 晨曦本轮**只读上游源码** + 只写**我方镜像目录**；**未改动**任何上游文件 / `pxrepo_mirror.sh` /
  systemd / 安装 / 其它站点。⇒ 上游侧的一切改动都在本立项范围内。
- **W2 会改变发布物内容** ⇒ 与「发布包是二进制分发」的既有口径（M219/R73）需要一并写清
  （若采纳 A，须同步 `tools/make_release.sh` 的文件清单与 `docs/RELEASE_PROCESS.md`）。
- 缓存的「真正决定行为的输入」若定义过窄 ⇒ 该失效的不失效（M223 家族的老坑）⇒ 门必须覆盖。

---

## 八 排期

| 里程碑 | 交付 |
|---|---|
| **M273** | 契约定稿（本文转正式 §）+ `pxpkg sync` 最小可用（`--base` / 幂等 / 原子 / 落盘即验）+ 门 |
| M274 | 增量下载 + `--dry-run` / `--force` + 默认 `--base` 双通道 |
| M275 | W2/W3（发布物含 registry、镜像 `--exclude=/registry/`）—— **等用户拍板** |

> ⚠ 本立项**不改变**当前发布流程；W2/W3 未拍板前不动发布链。
