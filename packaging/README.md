# PuXian RPM / dnf · yum 分发（M73 el9 / M77 el7）

让 RHEL 系（Rocky / Alma / CentOS / CentOS Stream / Oracle）用户一行安装：

```bash
dnf install puxian      # EL9
yum install puxian      # EL7
```

## 资产

| 文件 | 作用 |
|---|---|
| `puxian.spec` | RPM 打包定义（Version 0.1.0，Release 1.m<里程碑>.el<dist>） |
| `build_rpm.sh` | 全链路（el7/el8/el9 通用）：tarball → rpmbuild → 包签名 → createrepo(_c) → repomd 签名 → 公钥导出；`DIST=7/9` 决定目录与 `.el` 后缀 |
| `build_rpm_el7.sh` | centos:7 容器内执行：EOL vault 源修正 + gpg2.0(headless) + createrepo(gzip) + yum 3.4 双验签 + 真实安装验证 |
| `install-rpm.sh` | 用户侧仓库安装脚本（自动探测 dnf/yum 与 el7/el9） |
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
curl -fsSL -o install-rpm.sh https://nanzhangroup.github.io/PuXian/install-rpm.sh
sudo bash install-rpm.sh          # 写入 repo + 导入公钥（自动识别 dnf/yum、el7/el9）
sudo dnf install puxian           # EL9 一行安装（el7 用 yum install puxian）
sudo dnf upgrade puxian           # 里程碑升级自动拉新（el7 yum update）
```

脚本也随仓库提供：`packaging/install-rpm.sh`；支持 `install`/`remove` 参数。
仓库 URL：`https://nanzhangroup.github.io/PuXian/rpm/$releasever/$basearch/`
（el7 → `rpm/7/x86_64/`，el9 → `rpm/9/x86_64/`；gpgkey 在 `rpm/PUXIAN-GPG-KEY.asc`）。

## CI 发布（release.yml，tag `v0.1.0-mXX` 推送触发）

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
- [ ] el8 目录铺开（release.yml matrix 加 dist=8 即可，结构已支持）
- [ ] aarch64 仓库（交叉构建或原生 runner）
- [ ] Fedora / openEuler 目录铺开（$releasever 语义不同，需独立 dist 目录 + 测试）
