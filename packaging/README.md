# PuXian RPM / dnf · yum 分发（M73 el9 / M77 el7 / M168 openEuler）

让 rpm 系用户一行安装：

```bash
dnf install puxian      # EL9 · openEuler 22.03/24.03
yum install puxian      # EL7
```

**支持的组合**（M168 起由 `install-rpm.sh` 的**显式映射表**决定，不再靠 `$releasever`）：

| 发行版 | 仓库目录 | repo 文件里的 baseurl | 谁在 CI 真验收 |
|---|---|---|---|
| RHEL/Rocky/Alma/CentOS/Oracle/Scientific … el7 | `rpm/7/x86_64/` | `…/$releasever/$basearch/`（展开=7） | `rpm-verify-7`（centos:7 + yum3.4） |
| 同上 el9 | `rpm/9/x86_64/` | `…/$releasever/$basearch/`（展开=9） | `rpm-build-9`（rockylinux:9 自检） |
| **openEuler 22.03 / 24.03 LTS** | `rpm/openeuler/<ver>/x86_64/` | `…/openeuler/22.03/x86_64/`（**字面**，不用 `$releasever`） | `rpm-verify-openeuler`（真容器 dnf 安装 + 真编译运行） |
| EL 系克隆（anolis / opencloudos …） | 同上 el7/el9 | 同上 | 无（脚本会标「**未验证**」，不假装验证过） |
| el8 / el10 / Fedora / openEuler 20.03 / 非 x86_64 | —— | —— | 无（`install-rpm.sh` **明确不支持**并给出可执行的替代路线） |

> ⚠️ **为什么 openEuler 单独写目录**（2026-09-21 用户报障）：openEuler 的 `$releasever`
> 是 `22.03LTS`，而仓库树按 RHEL 语义只有 `7/9` ⇒ `baseurl` 展开必然 404；
> 旧版 `install-rpm.sh` 更是连认脸都没过（`❌ 暂支持 RHEL 系 7/9` 直接退出）。
> 现在：脚本按发行版写**字面**目录，CI 在 `openeuler/openeuler:22.03-lts` 与 `24.03-lts`
> 容器里 `dnf` 双验签 → 装包 → **真编译一个程序并运行**（只跑 `--version` 的验证
> 看不见"装得上、跑不起来"这类缺陷，M168 实测）。

## 资产

| 文件 | 作用 |
|---|---|
| `puxian.spec` | RPM 打包定义（Version 取 `%{pxver}`，由 build_rpm.sh 按 git tag 派生注入；Release `1.<里程碑>.el<dist>`；M168 起依赖含**静态 libc**：el7 硬依赖 `glibc-static`、el9/openEuler 用 `Recommends`（dnf 默认装；openEuler 无此包名、libc.a 随 `glibc-devel` 到位）） |
| `build_rpm.sh` | 全链路（el7/el8/el9 通用）：tarball → rpmbuild → 包签名 → createrepo(_c) → repomd 签名 → 公钥导出；`DIST=7/9` 决定目录与 `.el` 后缀 |
| `build_rpm_el7.sh` | centos:7 容器内执行：EOL vault 源修正 + gpg2.0(headless) + createrepo(gzip) + yum 3.4 双验签 + 真实安装验证 |
| `install-rpm.sh` | 用户侧仓库安装脚本（M168 重写：发行版显式映射 / `--dry-run` / `status` / 替代路线指引） |
| `selftest_install_rpm.sh` | **install-rpm.sh 的离线矩阵自测**（42 用例：el7/el9/oe22.03/oe24.03/oe20.03/el8/Fedora/Ubuntu/克隆 + aarch64/riscv64 + 反向判据「openEuler 输出里不得出现 `$releasever`」） |
| `verify_repo_el7.sh` | el7 终验（yum 3.4 双验签 + 安装 + **真编译运行** + 入库件静态性 ldd 断言） |
| `verify_repo_openeuler.sh` | openEuler 终验（M168 新增；dnf 双验签 + 安装 + 真编译运行 + 静态性断言） |
| `PUXIAN-GPG-KEY.asc` | 仓库公钥（正式发布时随仓库站点托管，不入库） |

## 密钥模型（正式密钥已启用）

一把「PuXian 发布专用密钥」，主/子分离：

```
主密钥（RSA-4096，仅签发子密钥/吊销）→ 离线冷存，永不进 CI
  └─ 签名子密钥（RSA-4096，2 年有效）→ 加密导出 → GitHub Actions secrets
吊销证书 → 打印两份存档（泄露应急开关）
公钥     → 随仓库站点托管（gpgkey URL）
```

GitHub Actions secrets（已配置）：`GPG_PRIVATE_KEY` / `GPG_PASSPHRASE` / `GPG_KEY_ID`。

> 密钥轮换/重建时，重新生成子密钥并按上文导出为 **明文 PRIVATE KEY BLOCK**
> （`gpg --export-secret-subkeys <子指纹>!` 不加 `--symmetric` 包裹；CI 的
> `--import` 只认 `PGP PRIVATE KEY BLOCK`，不认 `PGP MESSAGE` 加密信封），
> 再更新三个 secrets 即可。

## 用户安装

```bash
curl -fsSL -o install-rpm.sh https://soft.xiusoft.cn/puxian/install-rpm.sh   # 国内镜像（默认）
#   上游兜底同文件： https://nanzhangroup.github.io/PuXian/install-rpm.sh
bash install-rpm.sh status      # 只报「本机是什么 / 会去哪个仓库」（免 root，先看后装）
sudo bash install-rpm.sh        # 写入 repo + 导入公钥（自动识别 dnf/yum 与发行版）
sudo dnf install puxian         # EL9 / openEuler 一行安装（el7 用 yum install puxian）
sudo dnf upgrade puxian         # 里程碑升级自动拉新（el7 yum update）
```

脚本也随仓库提供：`packaging/install-rpm.sh`；支持 `install`/`remove`/`status`、
`--dry-run`、`--base`、`--os-release`（自测注入）、`--repo-file`、`--arch`。

**仓库双活**（同一发布产物在两处的等价副本，GPG 签名链两处相同、不绑定域名）：

| 仓库 | baseurl | 说明 |
|---|---|---|
| 国内镜像（默认） | `https://soft.xiusoft.cn/puxian/rpm/…` | 脚本内置 3s 探测，通过即用 |
| 上游 GitHub Pages | `https://nanzhangroup.github.io/PuXian/rpm/…` | 探测失败自动回退；亦可 `PUXIAN_RPM_BASE=` 强制指定 |

（el7 → `rpm/7/x86_64/`，el9 → `rpm/9/x86_64/`，**openEuler → `rpm/openeuler/<ver>/x86_64/`**；
gpgkey 统一在 `rpm/PUXIAN-GPG-KEY.asc`）。
国内镜像另托管发布 tarball：`https://soft.xiusoft.cn/puxian/releases/`。

## 国内镜像站点（soft.xiusoft.cn/puxian）

上游 GitHub Pages 国内实测 TTFB ≈ 0.30s，国内镜像 ≈ 0.07s（同节点同文件，约 4 倍）。
镜像由**边缘侧同步器**从 GitHub 拉取后落静态站点，再由既有的 soft-mirror 分发到各节点：

```
GitHub（tag + gh-pages/rpm + Release 资产）
   ↓  packaging/pxrepo_mirror.sh（幂等 · 单调 · 原子 · 可回滚）
<站点源目录>/puxian/                      ← 落地点（--dest）
   ↓  soft-mirror（rsync -a --delete-delay，本机 + 广州节点）
https://soft.xiusoft.cn/puxian/           ← install-rpm.sh / rpm/ / releases/ / version.json
```

| 物料 | 说明 |
|---|---|
| `packaging/pxrepo_mirror.sh` | 同步器参考实现（<300 行，仅依赖 git/curl/rsync/rpm/tar；`--dry-run` 空跑） |
| `packaging/systemd/pxrepo-sync.{service,timer}` | 单元模板（30min 周期 + 开机 2min 后即跑；失败由下一轮承担） |
| `/puxian/version.json` | 镜像自证（version/tag/synced_at/tarball sha256）——既是 `install-rpm.sh` 的 3s 探测探针，也是监控与用户对账口径 |

**同步契约**：① 版本序取自远端 tag（非 API `latest`）② 与 gh-pages `rpm/` 树内实际
`.mNNN` 交叉校验，不一致 ⇒ 中止、保持上一版；③ 比现役旧 ⇒ 拒绝（`PXREPO_ALLOW_REGRESSION=<理由>`
人工放行）；④ tarball sha256 == `sha256sums.txt`、每包 `rpm -Kv`、`repomd.xml.asc` 全过才落盘；
⑤ 发布顺序 **新 rpm（不删旧）→ repodata → 删旧 rpm**，消除「元数据与包不匹配」窗口；
⑥ 站点根文件（`index.html` / `install-rpm.sh`）按**内容指纹**判定 —— **版本相同也要刷新**，
   否则「只改落地页、版本不变」的更新会被幂等短路永久挡住（2026-09-16 实测踩到）。

> ⚠️ `--work` 暂存目录**必须位于 `--dest` 之外**，否则会被 soft-mirror 一并分发出去。

**落地页链接规范（2026-09-16 修 · 静默错链）**：站点 `…/puxian` **无尾斜杠时不 301 补斜杠**，
落地页会被直接 200 返回 —— 此时**裸相对链接被解析到站点根**：`version.json` → `/version.json`
（该路径**存在**，但内容是**文殊 App 的 `{"version":"1.7.141"}`**，用户点「镜像自证」会拿到
**别人的版本号**，不报错）、`rpm/PUXIAN-GPG-KEY.asc` → `/rpm/…`（**404**）。
⇒ `packaging/pages/index.html` 的**站内链接必须根相对**（`/puxian/…`）或绝对 URL，
由 `packaging/selftest_pages_links.sh` 离线把关（禁止裸相对；含负控）。

## CI 发布（release.yml，tag `v*` 推送触发，如 v0.2.0 / v0.1.0-mXX）

构建与发布分离（避免并发 push gh-pages 冲突）：

| job | 环境 | 说明 |
|---|---|---|
| `rpm-build-9` | container `rockylinux:9` | build_rpm.sh（DIST=9）+ dnf file:// 双验签 → artifact `pxrepo-9` |
| `rpm-build-7` | ubuntu + `docker run centos:7` | build_rpm_el7.sh：EOL vault 源 → gpg2.0 headless 签名 → createrepo(gzip) → yum file:// 双验签 + 真实安装 `pxc --version` → artifact `pxrepo-7` |
| `rpm-publish` | ubuntu | download 两 artifact 合并 rpm/{7,9} → rsync 推 `gh-pages/rpm/` |

- 容器内 centos:7 无法跑 node 系 actions（glibc 2.17 < node20 需求），故 el7
  构建走 ubuntu runner + docker run（runner 自带 docker，无 dind）
- el7 的 gpg1/gpg2.0 无 `--pinentry-mode loopback` → 统一用 `--passphrase-file`
  headless 签名；`__gpg` 自动指向 gpg2（el7 无 /usr/bin/gpg）
- el7 元数据由 python `createrepo`（gzip）组包，el9 由 `createrepo_c`（zstd）
  组包 —— 恰好匹配 yum 3.4 / dnf 各自支持的压缩格式
- 仓库只保留最新版；旧版 rpm 由 GitHub Release 资产留存
- GPG secrets 未配置时各 job 自动跳过（绿），配置后下一个 tag 即自动发布

## 验证链路

dnf/yum 安装校验链：公钥 → repomd.xml 签名（repo_gpgcheck）→ 按哈希取 rpm
→ 包签名（gpgcheck）→ 安装。两层都过才装。

## 待办

- [x] secrets 正式密钥启用（导入步 + headless 签名验证通过）
- [x] el7 签名仓库 + yum 真实验证（M77）
- [x] **openEuler 22.03 / 24.03 支持**（M168：认脸 + 别名仓库 + 容器终验）
- [ ] el8 / el10 目录铺开（release.yml matrix 加 dist=8/10 即可，结构已支持）
- [ ] aarch64 仓库（rpm 侧；当前 aarch64 走**官方引导包** `puxian-bootstrap-aarch64-<tag>.tar.gz`）
- [ ] Fedora（打包链与 EL 不同，未验证）

## M168：openEuler 支持 + 二进制可移植性（2026-09-21 用户报障）

用户实测报出两条（都不是"某个功能不好用"，而是**分发链缺了一块却没有任何门看得见**）：

1. **官方脚本不认 openEuler**：`install-rpm.sh` 的放行名单写死 `rhel|rocky|almalinux|centos|ol`
   且只允许 7/9 版本，其它发行版一律 `❌ 暂支持 RHEL 系 7/9` 退出；仓库目录也只有
   `7/x86_64`、`9/x86_64`，而 openEuler 的 `$releasever` 是 `22.03LTS` ⇒ 即便放行也必 404。
   修法 = **显式映射表**（脚本定目录，不把解释权交给 `$releasever`）+ 新增
   `rpm/openeuler/{22.03,24.03}/x86_64`（**同一批已签名 rpm** —— openEuler 22.03 的
   glibc 2.34 == el9 ABI 基线；元数据用 **gz**，避免各版本 libdnf 对 zstd 支持不一）
   + `selftest_install_rpm.sh`（**离线矩阵 42 用例**，含反向判据）+ **真容器终验**
   `rpm-verify-openeuler`（`openeuler:22.03-lts` / `24.03-lts`：dnf 双验签 → 装包 →
   **真编译运行**）。不支持组合现在会打印支持矩阵 + **可执行**的替代路线
   （tarball / aarch64 引导包，URL 从 `version.json` 取，精确到版本号）。

2. **aarch64 官方引导包"装得上、跑不起来"**：包内 `bootstrap/pxc` 是**动态件**（宿主
   ubuntu-24.04-arm 的系统 gcc 默认动态链接）⇒ 带 `GLIBC_2.38` 需求，而 openEuler 22.03
   只有 glibc 2.34。根因不在打包，而在**发布链没有人在看"这枚二进制需要什么"**：
   CI 自证只问"runner 上能不能跑"，而 runner 自己就是 2.38 ⇒ 永远绿。
   修法 = ① `selfhost/native_bootstrap.sh --portable`（原生档默认也 `-static`，
   且**不是全静态就判红**）；② 新门 `selfhost/check_bin_portability.sh`
   （`readelf -d` 判全静态 / `objdump -T` 取最高 `GLIBC_x.y` 与基线比较 / 架构断言 /
   `--require-static`；**自带 4 道负控的自证**）；③ 发布 job 现编**全部 12 件**原生工具
   （不再只 4 件 —— 否则包里混着 x86_64 的 pxfmt/pxlint，用户一跑就是 Exec format error）
   并剔除 VM 轨两件（x86_64 专属）后跑门。

**顺带查出、同一族的第三个坑**：`bootstrap/pxc_vm`（**用户面默认 VM 轨**）也是动态件
（需 `GLIBC_2.34`）⇒ 在 **el7（glibc 2.17）** 上 `px build` 一执行就崩；而 el7 验证脚本
当时只跑 `pxc --version`（走 C 轨静态件，恰好正常）⇒ **整类缺陷不可见**。
修法：`rebake_bin.sh` 把 pxc_vm 的链接口径改为 `static`（**14 件全静态**，`--check-all`
14/14、`--check-vm` 字节码镜像逐字节一致）+ `tools/px` 把「VM 轨可用性」判据从
「架构不符」放宽为「**本机执行不了**」（含 glibc 不够；`PX_STRICT_VM=1` 可强制响亮失败）
+ `verify_repo_el7.sh` 补**真编译运行**与 **ldd 静态性断言** + rpm `Requires` 补静态 libc
（el7 硬依赖 / el9·openEuler 弱依赖，见上表）。

## M78/M80：el7 支持与 RPM 签名密钥轮换（重要）
- **架构（M78）**：el7(rpm4.11/gpg2.0) 原生容器内 headless 签名不可靠（无 `--pinentry-mode
  loopback`，`__gpg_sign_cmd` 注入在 rpm4.11 不生效 → Bad passphrase）。改为：
  centos:7 容器**无签名构建**（`SKIP_SIGN=1`）→ el9(rockylinux:9, rpm4.16) **代签**
  el7 rpm（createrepo_c gzip，兼容 yum3.4）→ 发布单 artifact `pxrepo-all` →
  `rpm-verify-7` job 用 centos:7 + yum 3.4 对**正式签名仓库**双验签 + 安装终验。
- **密钥轮换（M80）**：旧 CI 密钥是「主密钥占位 + 签名子密钥」形态；el7 rpm4.11 的
  rpmdb 只导入主钥、不认子密钥签发的包（`NOKEY key ID 542a1694`）。已轮换为**专用主密钥**
  （RSA4096、无子密钥），el7/el9 包均以主密钥签发，rpm4.11 yum3.4 / dnf 全兼容：
  - 指纹：`334536AC7B3E161ABE1F53753851AD992A61D264`
  - GitHub secrets：`GPG_PRIVATE_KEY`（明文 armor 主私钥）/ `GPG_PASSPHRASE` /
    `GPG_KEY_ID`（上指纹）已同步更新
  - 本机备份（root 600）：`/data/puxian-rpm-release-sec.asc`（私钥）、
    `/data/.puxian-rpm-pass`（口令，勿外泄）
- [x] el7 签名仓库 + yum 3.4 真实验签安装（rpm-verify-7 job 终验通过，M80）
