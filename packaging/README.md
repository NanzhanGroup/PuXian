# PuXian RPM / dnf · yum 分发（M73）

让 Fedora / RHEL / Rocky / Alma / openEuler / CentOS 用户一行安装：

```bash
dnf install puxian
```

## 资产

| 文件 | 作用 |
|---|---|
| `puxian.spec` | RPM 打包定义（Version 0.1.0，Release 1.m<里程碑>） |
| `build_rpm.sh` | 全链路：tarball → rpmbuild → 包签名 → createrepo → repomd 签名 → 公钥导出 |
| `PUXIAN-GPG-KEY.asc` | 仓库公钥（正式发布时随仓库站点托管，不入库） |

## 密钥模型（正式发布）

一把「PuXian 发布专用密钥」，主/子分离：

```
主密钥（RSA-4096，仅签发子密钥/吊销）→ 离线冷存，永不进 CI
  └─ 签名子密钥（RSA-4096，2 年有效）→ 加密导出 → GitHub Actions secrets
吊销证书 → 打印两份存档（泄露应急开关）
公钥     → 随仓库站点托管（gpgkey URL）
```

### 第二步：生成正式密钥（demo 跑通后执行）

```bash
# 主密钥（交互，设置强 passphrase）
gpg --full-generate-key        # RSA 4096, Name: PuXian Release Signing
gpg --list-secret-keys --keyid-format LONG    # 记下主密钥指纹 FPR

# 签名子密钥（2 年有效）
gpg --quick-add-key <FPR> rsa4096 sign 2y

# 吊销证书
gpg --gen-revoke <FPR> --output /安全位置/puxian-revoke.asc

# CI 用：仅导出子密钥（加密导出，设独立 passphrase）
gpg --export-secret-subkeys <子密钥指纹>! | gpg --armor --symmetric -o /tmp/puxian-ci-key.asc

# 公钥托管
gpg --armor --export <FPR> > PUXIAN-GPG-KEY.asc   # 随仓库站点发布
```

GitHub Actions secrets（CI 用）：
`GPG_PRIVATE_KEY`=puxian-ci-key.asc 内容、`GPG_PASSPHRASE`、`GPG_KEY_ID`=子密钥指纹。

## 客户端 repo 文件（发布后用户配置）

```ini
[puxian]
name=PuXian Repository
baseurl=https://nanzhangroup.github.io/PuXian/rpm/$releasever/$basearch/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://nanzhangroup.github.io/PuXian/rpm/PUXIAN-GPG-KEY.asc
```

## 验证链路

dnf 安装校验链：公钥 → repomd.xml 签名（repo_gpgcheck）→ 按哈希取 rpm
→ 包签名（gpgcheck）→ 安装。两层都过才装。

## 待办（CI 入库）

- [ ] release.yml 增加 rpm job：tag 触发 → build_rpm.sh（GPG 三 secret）
      → 推 `gh-pages`（rpm/<dist>/<arch>/ + PUXIAN-GPG-KEY.asc）
- [ ] 用户安装脚本 `install-rpm.sh`（写入 repo 文件 + import 公钥）
- [ ] aarch64 仓库（交叉构建或原生 runner）
