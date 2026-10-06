# 官方 registry · 第三方包来源与许可（自动生成，勿手改）

> 生成器：`tools/import_registry_px.sh`（M187 第 65 轮 · M189 补**本地补丁**通道）· 源仓库：
> `https://github.com/banshanhanfu/registry-px.git` @ `01f6048` · 许可：**Apache-2.0**（上游 LICENSE 与本仓同族）
> 纪律：包文件**逐字节照搬**（不改一个字节）；来源/许可登记在本文件，不改包内注释。
> **例外（M189）**：「补丁」列非空的包 = 在照搬之上叠了一道**本地小补丁**（`tools/patches/registry-px/<name>.patch`），
> 原因是**我方语言面收紧**导致上游代码需要跟着改（如 M184 的严格 `int()`），而上游尚未发新版；
> 此时该包的 sha256 记的是**打过补丁后**的内容（M187 门的「表 ⇔ 磁盘」对拍因此仍然有效）。
> 上游修复后撤销补丁即可。
> 复核：`bash examples/m187_registry_import/verify.sh` 会**重算**下表 sha256 与磁盘对拍（防漂移）。

| 包 | 版本 | 文件数 | 入口 sha256（前 16） | 三轨验证 | 备注 | 补丁 |
|---|---|---|---|---|---|---|
| actor | 0.1.0 | 1 | `64fadb46d79bbbca` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| actor | 0.2.0 | 1 | `4a1efe74dd48d253` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| ansi | 0.1.0 | 1 | `4b7b6b3755788cb1` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ansi | 0.2.0 | 1 | `d1935bf4416cb314` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| barcode | 0.1.0 | 1 | `a8dbb132363a7fd8` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| barcode | 0.2.0 | 1 | `023f42635592f623` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| base58 | 0.1.0 | 1 | `da046120b39c8551` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| base58 | 0.2.0 | 1 | `c444fbe9a157bbcc` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bcrypt | 0.1.0 | 1 | `80c209eeba00e825` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bcrypt | 0.2.0 | 1 | `77c206cb18161167` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bench | 0.1.0 | 1 | `d1cad36776de9bb6` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bench | 0.2.0 | 1 | `2378290543cd39a8` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| big | 0.1.0 | 1 | `6f763b75b8e6cfc6` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| big | 0.2.0 | 1 | `7b8b927e2af01973` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bisect | 0.1.0 | 1 | `d0eb8bd198614960` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bisect | 0.2.0 | 1 | `3fe1aef2536cb038` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bitset | 0.1.0 | 1 | `c42c1d2520e7bc40` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bitset | 0.2.0 | 1 | `55301eb4130d9e37` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bloom | 0.1.0 | 1 | `96fe706573eab860` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bloom | 0.2.0 | 1 | `0e93d53b65f00fac` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bson | 0.1.0 | 1 | `3cb8b0fb27748b64` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bson | 0.2.0 | 1 | `3149cb903d5abdf1` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| bytes_pack | 0.1.0 | 1 | `cbc195f8ac2ecd72` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bytes_pack | 0.2.0 | 1 | `c8a7236229e4af30` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| cache | 0.1.0 | 1 | `dc37863189c8f07e` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cache | 0.2.0 | 1 | `f01ccec26d4bd8e3` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| captcha | 0.1.0 | 1 | `7431bbdd85269758` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| captcha | 0.2.0 | 1 | `59ccff52b086b331` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| checksum | 0.1.0 | 1 | `a57ec66c4d20999d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| checksum | 0.2.0 | 1 | `a3134405285c6a00` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| cli | 0.1.0 | 1 | `c34a60c991c7c567` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cli | 0.2.0 | 1 | `104d7684b1f25336` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| cnnum | 0.1.0 | 1 | `67d04590dd785873` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cnnum | 0.2.0 | 1 | `67d04590dd785873` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| concurrent_map | 0.1.0 | 1 | `d8c06362ec5521a2` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 已存在（内容一致） |  |
| concurrent_map | 0.2.0 | 1 | `5d2b46a9ff1a6383` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 新引入 |  |
| config | 0.1.0 | 1 | `e7fdaa1f5babb015` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| config | 0.2.0 | 1 | `ed3c2443a66b5f65` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| counter | 0.1.0 | 1 | `8d0391af66e9edfe` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| counter | 0.2.0 | 1 | `c6a64dd6436c9e06` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| csv | 0.1.0 | 1 | `4f4032598220e74f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| csv | 0.2.0 | 1 | `0b12f4fe99ffb13e` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| datastruct | 0.1.0 | 1 | `de025156bdd48b8f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| datastruct | 0.2.0 | 1 | `5ad33efa19e4e121` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| datetime | 0.1.0 | 1 | `476b0beb9d487eb5` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| datetime | 0.2.0 | 1 | `8520b3774480248a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| decimal | 0.1.0 | 1 | `d8a1a8847e9f17aa` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| decimal | 0.2.0 | 1 | `d1a3c25f43b2d633` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| diff | 0.1.0 | 1 | `02d7b5030b9cb582` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| diff | 0.2.0 | 1 | `d6471087e5ab73d9` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| dirs | 0.1.0 | 1 | `12bffdf03977d865` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dirs | 0.2.0 | 1 | `0be02b866da8e5aa` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| dist | 0.1.0 | 1 | `f941374741e914b1` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dist | 0.2.0 | 1 | `a57978fbf91d011c` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| dns | 0.1.0 | 1 | `191789744a74d869` | **双轨 SKIP** · 用例要求外网 UDP 解析器 223.5.5.5:53 | 已存在（内容一致） |  |
| dns | 0.2.0 | 1 | `653dde10f125671b` | **双轨 SKIP** · 用例要求外网 UDP 解析器 223.5.5.5:53 | 新引入 |  |
| dotenv | 0.1.0 | 1 | `0082e57039871ab5` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dotenv | 0.2.0 | 1 | `305306a8e7da0d02` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| edist | 0.1.0 | 1 | `8a65ff1e6a9918fb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| edist | 0.2.0 | 1 | `8538f2078d5a7ffb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| faker | 0.1.0 | 1 | `9c8b858f9c6418ab` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| faker | 0.2.0 | 1 | `629aee9b7da777fd` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| fractions | 0.1.0 | 1 | `20df3b9e3432cdd7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| fractions | 0.2.0 | 1 | `68c83ac9e947ecad` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| fsnotify | 0.1.0 | 1 | `3a5c62a8abd757f3` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| fsnotify | 0.2.0 | 1 | `bbb5eb7431e29c1a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ftp | 0.1.0 | 1 | `e9c78d3ca066aa6d` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| ftp | 0.2.0 | 1 | `262760823457a054` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 新引入 |  |
| functools | 0.1.0 | 1 | `b10d9664fd9e7fe7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| functools | 0.2.0 | 1 | `89b49d17985f599d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| glob | 0.1.0 | 1 | `065175dad26229eb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| glob | 0.2.0 | 1 | `db80c7819c86d67f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| graph | 0.1.0 | 1 | `27ffed153d7e0ea2` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| graph | 0.2.0 | 1 | `04fcbac782fa1903` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| hant | 0.1.0 | 1 | `cd941238d8664b1c` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| hant | 0.2.0 | 1 | `29f4227324ee629f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| holidays | 0.1.0 | 1 | `1cb1a8d9e450986b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| holidays | 0.2.0 | 1 | `1e06b0831f028825` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| htmlparse | 0.1.0 | 1 | `783211a6f7a24b3b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| htmlparse | 0.2.0 | 1 | `ab5251a5dffdfd92` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| human | 0.1.0 | 1 | `9d5e7a0f623875bb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| human | 0.2.0 | 1 | `63d43c1144fde6f6` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| idcard | 0.1.0 | 1 | `748ba1d2697786e9` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| idcard | 0.2.0 | 1 | `4e2006e1ca59c7b0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| imap | 0.1.0 | 1 | `517e3844d2564a60` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| imap | 0.2.0 | 1 | `55475fd953075f51` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 新引入 |  |
| inflect | 0.1.0 | 1 | `bba51ab89d5088e9` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| inflect | 0.2.0 | 1 | `6bd146ee3ac61e85` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| ini | 0.1.0 | 1 | `611c76ceeac66690` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ini | 0.2.0 | 1 | `c6f365b5d3033ece` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| interval | 0.1.0 | 1 | `4bc579f99b649c17` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| interval | 0.2.0 | 1 | `ec4c79c2b0d4fae7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| ipaddr | 0.1.0 | 1 | `f70363154fdb84f3` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ipaddr | 0.2.0 | 1 | `83861a7ed838777d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| itertools | 0.1.0 | 1 | `37942232f7cae81a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| itertools | 0.2.0 | 1 | `8f7480d4c88e56e0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| jsonpath | 0.1.0 | 1 | `c5a502af40f33ad0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jsonpath | 0.2.0 | 1 | `eac61c204f174260` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| jwt | 0.1.0 | 1 | `446ffb7eec923f1c` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jwt | 0.2.0 | 1 | `acfdc149e2ffc13d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| log | 0.1.0 | 1 | `70f6121ee99c97c2` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| log | 0.2.0 | 1 | `44d5384a4c601443` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| luhn | 0.1.0 | 1 | `0aa8fb613af2e058` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| luhn | 0.2.0 | 1 | `797bc2bb579f2377` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| lz4 | 0.1.0 | 1 | `cae7f80b11dae58b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| lz4 | 0.2.0 | 1 | `2fce8b50be31397b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| mailparse | 0.1.0 | 1 | `20e9cdf466929eed` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mailparse | 0.2.0 | 1 | `838b766eff4d7a56` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| markdown | 0.1.0 | 1 | `bef390af44a9e049` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| markdown | 0.2.0 | 1 | `6b78667e1358066c` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| metrics | 0.1.0 | 1 | `becfbd9468200c35` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| metrics | 0.2.0 | 1 | `7e2d6c5ed1528180` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| mock | 0.1.0 | 1 | `674318e1c79cbddf` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mock | 0.2.0 | 1 | `3056dc3f8e251f2e` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| money | 0.1.0 | 1 | `b0a30a180f650257` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| money | 0.2.0 | 1 | `cbc5477110b2ade0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| mongodb | 0.1.0 | 1 | `530e364f8044bea9` | 文件完整 · 测试需**真实 mongod**（127.0.0.1:27017） | 已存在（内容一致） |  |
| mongodb | 0.2.0 | 1 | `1f3100e867df1320` | 文件完整 · 测试需**真实 mongod**（127.0.0.1:27017） | 新引入 |  |
| mqtt | 0.1.0 | 1 | `f1b6efc6e89deaf7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mqtt | 0.2.0 | 1 | `f3b50e3684fc5f11` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| msgpack | 0.1.0 | 1 | `152ea2b5b642509a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| msgpack | 0.2.0 | 1 | `815f7bcc9c18a634` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| mysql | 0.1.0 | 3 | `d9ba075ee36833dd` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 已存在（内容一致） |  |
| mysql | 0.2.0 | 3 | `95113e7e93fff83b` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 新引入 |  |
| nanoid | 0.1.0 | 1 | `2439558272e9c6b0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| nanoid | 0.2.0 | 1 | `aa59a66d907b50ea` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| natsort | 0.1.0 | 1 | `7272ac004a76b6c0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| natsort | 0.2.0 | 1 | `8721e3899df33902` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| netprobe | 0.1.0 | 1 | `a3db7ca823d35f0b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| netprobe | 0.2.0 | 1 | `ef4a009c2a7ba82d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| num2words | 0.1.0 | 1 | `a858aade1ca26bb9` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| num2words | 0.2.0 | 1 | `fea9ae9015adda42` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| oauth2 | 0.1.0 | 1 | `113ae7bde528fcdf` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| oauth2 | 0.2.0 | 1 | `49e483857717546c` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 新引入 |  |
| parser | 0.1.0 | 1 | `6fb3331f44d3df8e` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| parser | 0.2.0 | 1 | `d30c674101b0b9cd` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| passhash | 0.1.0 | 1 | `9443863a4ca80c93` | **双轨 PASS**（M189：打本地补丁后 `passhash_test` 通过；M198 起补丁已撤销） | 已存在（内容一致） |  |
| passhash | 0.2.0 | 1 | `d1d77845aa280e68` | **双轨 PASS**（M189：打本地补丁后 `passhash_test` 通过；M198 起补丁已撤销） | 新引入 |  |
| pdf | 0.1.0 | 1 | `ae31809480b0b9bc` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pdf | 0.2.0 | 1 | `684ab86f885892a7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| percent | 0.1.0 | 1 | `e17fd2702f9faf59` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| percent | 0.2.0 | 1 | `85fe5eba4d28c593` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| pg | 0.1.0 | 1 | `342b11d554472746` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 已存在（内容一致） |  |
| pg | 0.2.0 | 1 | `512e08706f3f7cf1` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 新引入 |  |
| pinyin | 0.1.0 | 1 | `6a927fdcb7829bb4` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pinyin | 0.2.0 | 1 | `b0cb385ba3eecf62` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| plex | 0.1.0 | 1 | `33160e85fc870218` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| plex | 0.2.0 | 1 | `0e8bbca8a5cb568f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| pop3 | 0.1.0 | 1 | `8dcd8a58598d7e46` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| pop3 | 0.2.0 | 1 | `59097d205c7913d1` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 新引入 |  |
| progress | 0.1.0 | 1 | `391cbd39b984e8c2` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| progress | 0.2.0 | 1 | `c92683208ca2aed3` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| properties | 0.1.0 | 1 | `e308375bb6751ca4` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| properties | 0.2.0 | 1 | `8f6e81098ae8db48` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| protobuf | 0.1.0 | 1 | `7ebc9262b316f9ee` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| protobuf | 0.2.0 | 1 | `8ec8d70e57eb1ea1` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| punycode | 0.1.0 | 1 | `abb7beb8905abc73` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| punycode | 0.2.0 | 1 | `a60b293ded41aa20` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| pwgen | 0.1.0 | 1 | `ec9eb5bcb6bf7bbe` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pwgen | 0.2.0 | 1 | `0eb24bd943e62282` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| qrcode | 0.1.0 | 4 | `40931b70d0695c20` | 编译轨 PASS · 解释轨**超时**（PX-DEF-024 性能：单码 8 掩码罚分 ≈36s） | 已存在（内容一致） |  |
| qrcode | 0.2.0 | 4 | `e1a42a8621091f40` | 编译轨 PASS · 解释轨**超时**（PX-DEF-024 性能：单码 8 掩码罚分 ≈36s） | 新引入 |  |
| quickcheck | 0.1.0 | 1 | `fa649feea3a617ae` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| quickcheck | 0.2.0 | 1 | `40a7aff4a8d80c3e` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| rate | 0.1.0 | 1 | `30e2453ba47f5d2d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| rate | 0.2.0 | 1 | `c030797488c164f8` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| readstat | 0.1.0 | 1 | `04e4c0a1dd46f773` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| readstat | 0.2.0 | 1 | `6976e1b9a8060aa9` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| redis | 0.1.0 | 1 | `c1f2458ac14523a0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| redis | 0.2.0 | 1 | `01e6dcf37bd44f0f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| retry | 0.1.0 | 1 | `a2c85cf2e11ab74a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| retry | 0.2.0 | 1 | `a385dd39fb7a78d7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| richmd | 0.1.0 | 1 | `b80eb8d483ad6da0` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| richmd | 0.2.0 | 1 | `fa3c21ef393bd3f9` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| sched | 0.1.0 | 1 | `ca80e51ceba47dc8` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sched | 0.2.0 | 1 | `66e8e868cccaa88b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| secure_random | 0.1.0 | 1 | `326131cccc074d92` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| secure_random | 0.2.0 | 1 | `41d98d73898749a3` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| seg | 0.1.0 | 1 | `24115043f503a765` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| seg | 0.2.0 | 1 | `6423cd7b301f3be5` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| semaphore | 0.1.0 | 1 | `415583a9d156e259` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| semaphore | 0.2.0 | 1 | `36db825380a78d2a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| set | 0.1.0 | 1 | `d50b18e8f7ee615a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| set | 0.2.0 | 1 | `917f6374b79d581a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| shutil | 0.1.0 | 1 | `789d5ef6fff76c49` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| shutil | 0.2.0 | 1 | `846210454344dc86` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| slug | 0.1.0 | 1 | `d32d7fecdb71e713` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| slug | 0.2.0 | 1 | `0248a12c62288357` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| snowflake | 0.1.0 | 1 | `a7c68daa30ff84a1` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| snowflake | 0.2.0 | 1 | `7c61bf639830973e` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| sqlparse | 0.1.0 | 1 | `8c8d31686907561b` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sqlparse | 0.2.0 | 1 | `52feb0cf29ec970f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| stats | 0.1.0 | 1 | `fbc0ed7688cb703d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| stats | 0.2.0 | 1 | `1301b471ab3810bb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| statx | 0.1.0 | 1 | `ec42bffcdd268d00` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| statx | 0.2.0 | 1 | `713dc23dc943e2b2` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| strcase | 0.1.0 | 1 | `0e11d536139a1758` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strcase | 0.2.0 | 1 | `a6eaafb7b250c8a5` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| strength | 0.1.0 | 1 | `4bfdf4ab98670308` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strength | 0.2.0 | 1 | `aea5d4d55d8441fb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| table | 0.1.0 | 1 | `00396d3805583157` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| table | 0.2.0 | 1 | `f6c959ccd2ccdca3` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| tar | 0.1.0 | 1 | `e822a49c9b2e7a44` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tar | 0.2.0 | 1 | `4b14ca3ff40c4a11` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| targz | 0.1.0 | 1 | `c546cd7393f75c87` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| targz | 0.2.0 | 1 | `b1b16064f5de0358` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| tdtest | 0.1.0 | 1 | `690c4c591ab473d4` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tdtest | 0.2.0 | 1 | `8eb8784a582166fb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| tempfile | 0.1.0 | 1 | `256f648fa2878927` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tempfile | 0.2.0 | 1 | `9edf42c95a97dc79` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| template | 0.1.0 | 1 | `ddd3840a91b390db` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| template | 0.2.0 | 1 | `4632d59387f00ac2` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| testkit | 0.1.0 | 1 | `55235620184a9195` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| testkit | 0.2.0 | 1 | `b3c65073503fa474` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| textwrap | 0.1.0 | 1 | `ed54bcec0ba8656d` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| textwrap | 0.2.0 | 1 | `9ecf346b88eb0af6` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| thousep | 0.1.0 | 1 | `57c0f33a58ff774f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| thousep | 0.2.0 | 1 | `aa99f7e41258e749` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| toml | 0.1.0 | 1 | `50d4bc146de0ec73` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| toml | 0.2.0 | 1 | `6c6b24e31a263820` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| totp | 0.1.0 | 1 | `e747705a6476cb87` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| totp | 0.2.0 | 1 | `8a872a8465ba07bb` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| trie | 0.1.0 | 1 | `30a321588395a6f7` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| trie | 0.2.0 | 1 | `89fa4859f80dd8ff` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| tzmini | 0.1.0 | 1 | `63cba2e6f43108ca` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tzmini | 0.2.0 | 1 | `89f92de563694652` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| ulid | 0.1.0 | 1 | `f4d1d459cecf4075` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ulid | 0.2.0 | 1 | `5e111a77c92d1130` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| units | 0.1.0 | 1 | `1d126f005f239fc8` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| units | 0.2.0 | 1 | `d4bc5e0e67d5f044` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| uuid | 0.1.0 | 1 | `fea30cda223e5070` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| uuid | 0.2.0 | 1 | `64564d50f9924c65` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| validator | 0.1.0 | 1 | `f4413bb4c03db19a` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| validator | 0.2.0 | 1 | `0d800cf03061b3ca` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| walk | 0.1.0 | 1 | `fc8a39a74d22dc3c` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| walk | 0.2.0 | 1 | `d8c41bf10ac7d327` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| workerpool | 0.1.0 | 1 | `8dcb596dd326743d` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 已存在（内容一致） |  |
| workerpool | 0.2.0 | 1 | `84b727522c513376` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 新引入 |  |
| x509 | 0.1.0 | 1 | `0fcb8715961a6555` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| x509 | 0.2.0 | 1 | `11f55acde1713313` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
| xlsx | 0.1.0 | 2 | `eb1e3a894c1dc2f4` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| xlsx | 0.2.0 | 2 | `87ecc46389910f1f` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| zlib | 0.1.0 | 1 | `3150483266a834de` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| zlib | 0.2.0 | 1 | `ab129176bd5304fe` | **双轨 PASS**（M282 上游用例回归 · 238 用例 × 双轨 —— 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 新引入 |  |
