# 官方 registry · 第三方包来源与许可（自动生成，勿手改）

> 生成器：`tools/import_registry_px.sh`（M187 第 65 轮 · M189 补**本地补丁**通道）· 源仓库：
> `git@github.com:banshanhanfu/registry-px.git` @ `03cc339` · 许可：**Apache-2.0**（上游 LICENSE 与本仓同族）
> 纪律：包文件**逐字节照搬**（不改一个字节）；来源/许可登记在本文件，不改包内注释。
> **例外（M189）**：「补丁」列非空的包 = 在照搬之上叠了一道**本地小补丁**（`tools/patches/registry-px/<name>.patch`），
> 原因是**我方语言面收紧**导致上游代码需要跟着改（如 M184 的严格 `int()`），而上游尚未发新版；
> 此时该包的 sha256 记的是**打过补丁后**的内容（M187 门的「表 ⇔ 磁盘」对拍因此仍然有效）。
> 上游修复后撤销补丁即可。
> 复核：`bash examples/m187_registry_import/verify.sh` 会**重算**下表 sha256 与磁盘对拍（防漂移）。

| 包 | 版本 | 文件数 | 入口 sha256（前 16） | 三轨验证 | 备注 | 补丁 |
|---|---|---|---|---|---|---|
| actor | 0.1.0 | 1 | `64fadb46d79bbbca` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| actor | 0.2.0 | 1 | `4a1efe74dd48d253` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| advstruct | 0.1.0 | 1 | `4a2edfa0a18eb768` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| aescbc | 0.1.0 | 1 | `514a11c95e770aca` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| aescbc | 0.2.0 | 1 | `123904b4d6d6fe43` | **双轨 PASS**（本地 0.2.0 · 用例 `aescbc2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| ansi | 0.1.0 | 1 | `4b7b6b3755788cb1` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ansi | 0.2.0 | 1 | `d1935bf4416cb314` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| arrutil | 0.1.0 | 1 | `8078823d41be6015` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| barcode | 0.1.0 | 1 | `a8dbb132363a7fd8` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| barcode | 0.2.0 | 1 | `023f42635592f623` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| base58 | 0.1.0 | 1 | `da046120b39c8551` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| base58 | 0.2.0 | 1 | `c444fbe9a157bbcc` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bcrypt | 0.1.0 | 1 | `80c209eeba00e825` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bcrypt | 0.2.0 | 1 | `77c206cb18161167` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bench | 0.1.0 | 1 | `d1cad36776de9bb6` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bench | 0.2.0 | 1 | `2378290543cd39a8` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| big | 0.1.0 | 1 | `6f763b75b8e6cfc6` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| big | 0.2.0 | 1 | `7b8b927e2af01973` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bisect | 0.1.0 | 1 | `d0eb8bd198614960` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bisect | 0.2.0 | 1 | `3fe1aef2536cb038` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bitset | 0.1.0 | 1 | `c42c1d2520e7bc40` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bitset | 0.2.0 | 1 | `55301eb4130d9e37` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bloom | 0.1.0 | 1 | `96fe706573eab860` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bloom | 0.2.0 | 1 | `0e93d53b65f00fac` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bson | 0.1.0 | 1 | `3cb8b0fb27748b64` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bson | 0.2.0 | 1 | `3149cb903d5abdf1` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bytes_pack | 0.1.0 | 1 | `cbc195f8ac2ecd72` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| bytes_pack | 0.2.0 | 1 | `6033d782fd5e3f03` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cache | 0.1.0 | 1 | `dc37863189c8f07e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cache | 0.2.0 | 1 | `f01ccec26d4bd8e3` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| captcha | 0.1.0 | 1 | `7431bbdd85269758` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| captcha | 0.2.0 | 1 | `59ccff52b086b331` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| channel | 0.1.0 | 1 | `5676e0ac32bbcee2` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cldr | 0.1.0 | 1 | `281bd5ff4f801fc7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cli | 0.1.0 | 1 | `c34a60c991c7c567` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cli | 0.2.0 | 1 | `104d7684b1f25336` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cnnum | 0.1.0 | 1 | `67d04590dd785873` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| cnnum | 0.2.0 | 1 | `67d04590dd785873` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| colorutil | 0.1.0 | 1 | `e6304cb04a30b679` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| compact | 0.1.0 | 1 | `112d69305b7c6784` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| concurrent_map | 0.1.0 | 1 | `d8c06362ec5521a2` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 已存在（内容一致） |  |
| concurrent_map | 0.2.0 | 1 | `09a627f07ca7dc8b` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 已存在（内容一致） |  |
| config | 0.1.0 | 1 | `e7fdaa1f5babb015` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| config | 0.2.0 | 1 | `845e688316cda04a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| connpool | 0.1.0 | 1 | `2a2703720cf394de` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| counter | 0.1.0 | 1 | `8d0391af66e9edfe` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| counter | 0.2.0 | 1 | `c6a64dd6436c9e06` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| csrf | 0.1.0 | 1 | `4d84f6588161a921` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| csv | 0.1.0 | 1 | `4f4032598220e74f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| csv | 0.2.0 | 1 | `66d358fa14800c4f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ctype | 0.1.0 | 1 | `64a8edf2daf63765` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| currency | 0.1.0 | 1 | `4b52b8f35a20ffb9` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| currency | 0.2.0 | 1 | `5e858c4090a546c0` | **双轨 PASS**（本地 0.2.0 · 用例 `currency2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| datastruct | 0.1.0 | 1 | `de025156bdd48b8f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| datastruct | 0.2.0 | 1 | `5ad33efa19e4e121` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dateperiod | 0.1.0 | 1 | `fb58f1a467d7abbd` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`dateperiod.patch` |
| dateperiod | 0.2.0 | 1 | `6aa1172283e6b6a9` | **双轨 PASS**（本地 0.2.0 · 用例 `dateperiod2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| datetime | 0.1.0 | 1 | `97cf5f4cc722d69c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`datetime.patch` |
| datetime | 0.2.0 | 1 | `d367f3b196deab15` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dbal | 0.1.0 | 1 | `cc9633b61eb1ac2d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dbal | 0.2.0 | 1 | `990906f15dbd31a8` | **双轨 PASS**（本地 0.2.0 · 用例 `dbal2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| decimal | 0.1.0 | 1 | `d8a1a8847e9f17aa` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| decimal | 0.2.0 | 1 | `d1a3c25f43b2d633` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| deepmerge | 0.1.0 | 1 | `e8d1c1a1f83a8c11` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| descrypt | 0.1.0 | 1 | `c487ff3b7e125dff` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| diff | 0.1.0 | 1 | `02d7b5030b9cb582` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| diff | 0.2.0 | 1 | `32fee32f9a64a58d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dirs | 0.1.0 | 1 | `12bffdf03977d865` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dirs | 0.2.0 | 1 | `0be02b866da8e5aa` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dist | 0.1.0 | 1 | `f941374741e914b1` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dist | 0.2.0 | 1 | `8ceb057121e05cb5` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dns | 0.1.0 | 1 | `191789744a74d869` | **双轨 SKIP** · 用例要求外网 UDP 解析器 223.5.5.5:53 | 已存在（内容一致） |  |
| dns | 0.2.0 | 1 | `653dde10f125671b` | **双轨 SKIP** · 用例要求外网 UDP 解析器 223.5.5.5:53 | 已存在（内容一致） |  |
| domlite | 0.1.0 | 1 | `cf718e413f580bff` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dotenv | 0.1.0 | 1 | `0082e57039871ab5` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| dotenv | 0.2.0 | 1 | `305306a8e7da0d02` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| edist | 0.1.0 | 1 | `8a65ff1e6a9918fb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| edist | 0.2.0 | 1 | `8538f2078d5a7ffb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| errorhandler | 0.1.0 | 1 | `f4bdc6ac3cc19a1d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| eventloop | 0.1.0 | 1 | `ba4d1cef725b9f5b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| eventloop | 0.2.0 | 1 | `f57eb1efedc5e069` | **双轨 PASS**（本地 0.2.0 · 用例 `eventloop2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| exif | 0.1.0 | 1 | `9a1adf4525e994cc` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| faker | 0.1.0 | 1 | `9c8b858f9c6418ab` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| faker | 0.2.0 | 1 | `629aee9b7da777fd` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| fileinfo | 0.1.0 | 1 | `b170bbbfb975294b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| finance | 0.1.0 | 1 | `4d7f5763459e5b1e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| finance | 0.2.0 | 1 | `af30fb4721ea6917` | **双轨 PASS**（本地 0.2.0 · 用例 `finance2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| fractions | 0.1.0 | 1 | `20df3b9e3432cdd7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| fractions | 0.2.0 | 1 | `725476da978406cf` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| fsnotify | 0.1.0 | 1 | `3a5c62a8abd757f3` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| fsnotify | 0.2.0 | 1 | `81cdbdf5484f4694` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ftp | 0.1.0 | 1 | `e9c78d3ca066aa6d` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| ftp | 0.2.0 | 1 | `262760823457a054` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| functools | 0.1.0 | 1 | `b10d9664fd9e7fe7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| functools | 0.2.0 | 1 | `89b49d17985f599d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| geo | 0.1.0 | 1 | `d1fe35b04d5becb7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| gettext | 0.1.0 | 1 | `36a7935b872a56ac` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| glob | 0.1.0 | 1 | `065175dad26229eb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| glob | 0.2.0 | 1 | `db80c7819c86d67f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| graph | 0.1.0 | 1 | `27ffed153d7e0ea2` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| graph | 0.2.0 | 1 | `04fcbac782fa1903` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| hant | 0.1.0 | 1 | `cd941238d8664b1c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| hant | 0.2.0 | 1 | `b13fbb43405c496e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| hashutil | 0.1.0 | 1 | `81d1d7c54c8f3d9a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| holidays | 0.1.0 | 1 | `1cb1a8d9e450986b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| holidays | 0.2.0 | 1 | `1e06b0831f028825` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| htmlparse | 0.1.0 | 1 | `783211a6f7a24b3b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| htmlparse | 0.2.0 | 1 | `cc592bc6edc49d8e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| htmlsanitizer | 0.1.0 | 1 | `fb1baefb6179fdb4` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| htmlsanitizer | 0.2.0 | 1 | `e33b5193b32e536f` | **双轨 PASS**（本地 0.2.0 · 用例 `htmlsanitizer2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| human | 0.1.0 | 1 | `9d5e7a0f623875bb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| human | 0.2.0 | 1 | `63d43c1144fde6f6` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| idcard | 0.1.0 | 1 | `748ba1d2697786e9` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| idcard | 0.2.0 | 1 | `4e2006e1ca59c7b0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| imap | 0.1.0 | 1 | `517e3844d2564a60` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| imap | 0.2.0 | 1 | `55475fd953075f51` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| inflect | 0.1.0 | 1 | `bba51ab89d5088e9` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| inflect | 0.2.0 | 1 | `6bd146ee3ac61e85` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ini | 0.1.0 | 1 | `611c76ceeac66690` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ini | 0.2.0 | 1 | `c6f365b5d3033ece` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| interval | 0.1.0 | 1 | `4bc579f99b649c17` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| interval | 0.2.0 | 1 | `ec4c79c2b0d4fae7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ipaddr | 0.1.0 | 1 | `f70363154fdb84f3` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ipaddr | 0.2.0 | 1 | `83861a7ed838777d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| itertools | 0.1.0 | 1 | `37942232f7cae81a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| itertools | 0.2.0 | 1 | `8f7480d4c88e56e0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jsonpath | 0.1.0 | 1 | `c5a502af40f33ad0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jsonpath | 0.2.0 | 1 | `eac61c204f174260` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jsonschema | 0.1.0 | 1 | `16afc8cc82bc62a0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jwt | 0.1.0 | 1 | `446ffb7eec923f1c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| jwt | 0.2.0 | 1 | `acfdc149e2ffc13d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ldap | 0.1.0 | 1 | `6abda90b4a1ad3dc` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| log | 0.1.0 | 1 | `70f6121ee99c97c2` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| log | 0.2.0 | 1 | `44d5384a4c601443` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| luhn | 0.1.0 | 1 | `0aa8fb613af2e058` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| luhn | 0.2.0 | 1 | `797bc2bb579f2377` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| lz4 | 0.1.0 | 1 | `cae7f80b11dae58b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| lz4 | 0.2.0 | 1 | `2fce8b50be31397b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mailparse | 0.1.0 | 1 | `20e9cdf466929eed` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mailparse | 0.2.0 | 1 | `838b766eff4d7a56` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| markdown | 0.1.0 | 1 | `bef390af44a9e049` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| markdown | 0.2.0 | 1 | `6b78667e1358066c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mathx | 0.1.0 | 1 | `7c4d6b9e1fb8103b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| memcached | 0.1.0 | 1 | `2f0027e92e52a1c5` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| memcached | 0.2.0 | 1 | `0d05c2bcaa3cf8d5` | **双轨 PASS**（本地 0.2.0 · 用例 `memcached2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| metrics | 0.1.0 | 1 | `becfbd9468200c35` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| metrics | 0.2.0 | 1 | `7e2d6c5ed1528180` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| migration | 0.1.0 | 1 | `04a82af09ee3bca1` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mimebuilder | 0.1.0 | 1 | `8df413917f685ccb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mock | 0.1.0 | 1 | `2fa735aa56e54b16` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`mock.patch` |
| mock | 0.2.0 | 1 | `3cef57731933520d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`mock.patch` |
| money | 0.1.0 | 1 | `b0a30a180f650257` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| money | 0.2.0 | 1 | `cbc5477110b2ade0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mongodb | 0.1.0 | 1 | `530e364f8044bea9` | 文件完整 · 测试需**真实 mongod**（127.0.0.1:27017） | 已存在（内容一致） |  |
| mongodb | 0.2.0 | 1 | `1f3100e867df1320` | 文件完整 · 测试需**真实 mongod**（127.0.0.1:27017） | 已存在（内容一致） |  |
| mqtt | 0.1.0 | 1 | `f1b6efc6e89deaf7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mqtt | 0.2.0 | 1 | `f3b50e3684fc5f11` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| msgpack | 0.1.0 | 1 | `152ea2b5b642509a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| msgpack | 0.2.0 | 1 | `815f7bcc9c18a634` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| multisort | 0.1.0 | 1 | `5f85b6c424fe5a60` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| mysql | 0.1.0 | 3 | `b2a199c1ce323024` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 已存在（内容一致） | 补丁：`mysql.patch` |
| mysql | 0.2.0 | 3 | `8385b6011a7fe328` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 已存在（内容一致） | 补丁：`mysql.patch`,`mysql@0.2.0.patch` |
| nanoid | 0.1.0 | 1 | `2439558272e9c6b0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| nanoid | 0.2.0 | 1 | `aa59a66d907b50ea` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| natsort | 0.1.0 | 1 | `7272ac004a76b6c0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| natsort | 0.2.0 | 1 | `8721e3899df33902` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| netprobe | 0.1.0 | 1 | `a3db7ca823d35f0b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| netprobe | 0.2.0 | 1 | `ef4a009c2a7ba82d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ntp | 0.1.0 | 1 | `d40c23dd04d83af6` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| num2words | 0.1.0 | 1 | `a858aade1ca26bb9` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| num2words | 0.2.0 | 1 | `fea9ae9015adda42` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| numconv | 0.1.0 | 1 | `1c2d85b441eeec29` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| oauth2 | 0.1.0 | 1 | `113ae7bde528fcdf` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| oauth2 | 0.2.0 | 1 | `49e483857717546c` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| orm | 0.1.0 | 1 | `7747ded2a7a6937c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| orm | 0.2.0 | 1 | `8db6b0ff4e0b3ed7` | **双轨 PASS**（本地 0.2.0 · 用例 `orm2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| outbuf | 0.1.0 | 1 | `11428e28f356b655` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| parser | 0.1.0 | 1 | `6fb3331f44d3df8e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| parser | 0.2.0 | 1 | `d30c674101b0b9cd` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| passhash | 0.1.0 | 1 | `9443863a4ca80c93` | **双轨 PASS**（M189：打本地补丁后 `passhash_test` 通过；M198 起补丁已撤销） | 已存在（内容一致） |  |
| passhash | 0.2.0 | 1 | `d1d77845aa280e68` | **双轨 PASS**（M189：打本地补丁后 `passhash_test` 通过；M198 起补丁已撤销） | 已存在（内容一致） |  |
| pdf | 0.1.0 | 1 | `ae31809480b0b9bc` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pdf | 0.2.0 | 1 | `684ab86f885892a7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| percent | 0.1.0 | 1 | `e17fd2702f9faf59` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| percent | 0.2.0 | 1 | `85fe5eba4d28c593` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pg | 0.1.0 | 1 | `342b11d554472746` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 已存在（内容一致） |  |
| pg | 0.2.0 | 1 | `512e08706f3f7cf1` | 文件完整 · 测试需**真实服务端**（PG13 / MariaDB） | 已存在（内容一致） |  |
| phar | 0.1.0 | 1 | `4f4f1a2e39ae5d39` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| phonetic | 0.1.0 | 1 | `6f3e89c279060f70` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| phpser | 0.1.0 | 1 | `62d5d74a18d1d015` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pinyin | 0.1.0 | 1 | `6a927fdcb7829bb4` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pinyin | 0.2.0 | 1 | `b0cb385ba3eecf62` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| plex | 0.1.0 | 1 | `33160e85fc870218` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| plex | 0.2.0 | 1 | `0e8bbca8a5cb568f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pop3 | 0.1.0 | 1 | `8dcd8a58598d7e46` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| pop3 | 0.2.0 | 1 | `59097d205c7913d1` | **双轨 PASS**（本仓自建 mock 服务端，见 upstream-tests/fixtures/） | 已存在（内容一致） |  |
| progress | 0.1.0 | 1 | `391cbd39b984e8c2` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| progress | 0.2.0 | 1 | `c92683208ca2aed3` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| promise | 0.1.0 | 1 | `346d1bfca61220ad` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| properties | 0.1.0 | 1 | `e308375bb6751ca4` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| properties | 0.2.0 | 1 | `8f6e81098ae8db48` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| protobuf | 0.1.0 | 1 | `7ebc9262b316f9ee` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| protobuf | 0.2.0 | 1 | `359cd72974cdadde` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| punycode | 0.1.0 | 1 | `abb7beb8905abc73` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| punycode | 0.2.0 | 1 | `a60b293ded41aa20` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pwgen | 0.1.0 | 1 | `ec9eb5bcb6bf7bbe` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| pwgen | 0.2.0 | 1 | `0eb24bd943e62282` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| qrcode | 0.1.0 | 4 | `40931b70d0695c20` | 编译轨 PASS · 解释轨**超时**（PX-DEF-024 性能：单码 8 掩码罚分 ≈36s） | 已存在（内容一致） |  |
| qrcode | 0.2.0 | 4 | `e1a42a8621091f40` | 编译轨 PASS · 解释轨**超时**（PX-DEF-024 性能：单码 8 掩码罚分 ≈36s） | 已存在（内容一致） |  |
| querybuilder | 0.1.0 | 1 | `7157605cc5f85c8d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| querybuilder | 0.2.0 | 1 | `158b5276d8ee1a39` | **双轨 PASS**（本地 0.2.0 · 用例 `querybuilder2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| quickcheck | 0.1.0 | 1 | `fa649feea3a617ae` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| quickcheck | 0.2.0 | 1 | `29f6014fd06ad25a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| rate | 0.1.0 | 1 | `30e2453ba47f5d2d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| rate | 0.2.0 | 1 | `c030797488c164f8` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| readstat | 0.1.0 | 1 | `04e4c0a1dd46f773` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| readstat | 0.2.0 | 1 | `6976e1b9a8060aa9` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| redis | 0.1.0 | 1 | `c1f2458ac14523a0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| redis | 0.2.0 | 1 | `01e6dcf37bd44f0f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| retry | 0.1.0 | 1 | `a2c85cf2e11ab74a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| retry | 0.2.0 | 1 | `a385dd39fb7a78d7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| richmd | 0.1.0 | 1 | `b80eb8d483ad6da0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| richmd | 0.2.0 | 1 | `c58c36e2598bb1aa` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| rssbuilder | 0.1.0 | 1 | `6a4a3b1c2ffd9251` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sched | 0.1.0 | 1 | `ca80e51ceba47dc8` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sched | 0.2.0 | 1 | `66e8e868cccaa88b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| scraper | 0.1.0 | 1 | `e654259b97177a19` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| secure_random | 0.1.0 | 1 | `326131cccc074d92` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| secure_random | 0.2.0 | 1 | `41d98d73898749a3` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| seg | 0.1.0 | 1 | `24115043f503a765` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| seg | 0.2.0 | 1 | `6423cd7b301f3be5` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| semaphore | 0.1.0 | 1 | `415583a9d156e259` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| semaphore | 0.2.0 | 1 | `36db825380a78d2a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| session | 0.1.0 | 1 | `d161b377f68dd343` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| set | 0.1.0 | 1 | `d50b18e8f7ee615a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| set | 0.2.0 | 1 | `917f6374b79d581a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| shutil | 0.1.0 | 1 | `789d5ef6fff76c49` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| shutil | 0.2.0 | 1 | `846210454344dc86` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| simtext | 0.1.0 | 1 | `dfeeaa82ee31720c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| slug | 0.1.0 | 1 | `d32d7fecdb71e713` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| slug | 0.2.0 | 1 | `f10d66742104993c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| snapshot | 0.1.0 | 1 | `6447dad6175be5cc` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| snmp | 0.1.0 | 1 | `75146bd2b035972a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| snowflake | 0.1.0 | 1 | `a7c68daa30ff84a1` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| snowflake | 0.2.0 | 1 | `7c61bf639830973e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| soap | 0.1.0 | 1 | `02e5de207bce66ed` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| splstruct | 0.1.0 | 1 | `6c14bedbe4bb9c2f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sprintf | 0.1.0 | 1 | `96a2470e693f8158` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sprintf | 0.2.0 | 1 | `8b40cc72ec278638` | **双轨 PASS**（本地 0.2.0 · 用例 `sprintf2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| sqlparse | 0.1.0 | 1 | `8c8d31686907561b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| sqlparse | 0.2.0 | 1 | `52feb0cf29ec970f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| stats | 0.1.0 | 1 | `fbc0ed7688cb703d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| stats | 0.2.0 | 1 | `1301b471ab3810bb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| statx | 0.1.0 | 1 | `ec42bffcdd268d00` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| statx | 0.2.0 | 1 | `713dc23dc943e2b2` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strcase | 0.1.0 | 1 | `0e11d536139a1758` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strcase | 0.2.0 | 1 | `a6eaafb7b250c8a5` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strength | 0.1.0 | 1 | `4bfdf4ab98670308` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strength | 0.2.0 | 1 | `aea5d4d55d8441fb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strpad | 0.1.0 | 1 | `693c5023877705f4` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| strtotime | 0.1.0 | 1 | `f4f0d6c1cd2f90af` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`strtotime.patch` |
| strtotime | 0.2.0 | 1 | `b2b526c1c8f337dd` | **双轨 PASS**（本地 0.2.0 · 用例 `strtotime2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| table | 0.1.0 | 1 | `00396d3805583157` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| table | 0.2.0 | 1 | `f6c959ccd2ccdca3` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tar | 0.1.0 | 1 | `e822a49c9b2e7a44` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tar | 0.2.0 | 1 | `e4ed41886714d65e` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| targz | 0.1.0 | 1 | `c546cd7393f75c87` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| targz | 0.2.0 | 1 | `57ea945e74428b7c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tdtest | 0.1.0 | 1 | `690c4c591ab473d4` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tdtest | 0.2.0 | 1 | `8eb8784a582166fb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tempfile | 0.1.0 | 1 | `256f648fa2878927` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tempfile | 0.2.0 | 1 | `9edf42c95a97dc79` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| template | 0.1.0 | 1 | `ddd3840a91b390db` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| template | 0.2.0 | 1 | `4632d59387f00ac2` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| testkit | 0.1.0 | 1 | `55235620184a9195` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| testkit | 0.2.0 | 1 | `b3c65073503fa474` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| textfmt | 0.1.0 | 1 | `a8f53202bfea2c7f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| textwrap | 0.1.0 | 1 | `ed54bcec0ba8656d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| textwrap | 0.2.0 | 1 | `a9aeff4eeb3afd21` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| thousep | 0.1.0 | 1 | `57c0f33a58ff774f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| thousep | 0.2.0 | 1 | `aa99f7e41258e749` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| toml | 0.1.0 | 1 | `50d4bc146de0ec73` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| toml | 0.2.0 | 1 | `6c6b24e31a263820` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| totp | 0.1.0 | 1 | `e747705a6476cb87` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| totp | 0.2.0 | 1 | `8a872a8465ba07bb` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| trie | 0.1.0 | 1 | `30a321588395a6f7` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| trie | 0.2.0 | 1 | `89fa4859f80dd8ff` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| tzmini | 0.1.0 | 1 | `193f89a3bcf5a5cc` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`tzmini.patch` |
| tzmini | 0.2.0 | 1 | `89f92de563694652` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ulid | 0.1.0 | 1 | `f4d1d459cecf4075` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| ulid | 0.2.0 | 1 | `5e111a77c92d1130` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| units | 0.1.0 | 1 | `1d126f005f239fc8` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| units | 0.2.0 | 1 | `d4bc5e0e67d5f044` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| uuencode | 0.1.0 | 1 | `604adbd782f41ac6` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| uuencode | 0.2.0 | 1 | `23d28448fd8bbdb3` | **双轨 PASS**（本地 0.2.0 · 用例 `uuencode2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| uuid | 0.1.0 | 1 | `fea30cda223e5070` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| uuid | 0.2.0 | 1 | `64564d50f9924c65` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| validator | 0.1.0 | 1 | `f4413bb4c03db19a` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| validator | 0.2.0 | 1 | `0d800cf03061b3ca` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| vardump | 0.1.0 | 1 | `9ed14ee0262b8d9b` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| vcard | 0.1.0 | 1 | `284c78e672ac75c0` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| walk | 0.1.0 | 1 | `fc8a39a74d22dc3c` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| walk | 0.2.0 | 1 | `d8c41bf10ac7d327` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| whois | 0.1.0 | 1 | `2d4b1fe9fd4b736d` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| workerpool | 0.1.0 | 1 | `8dcb596dd326743d` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 已存在（内容一致） |  |
| workerpool | 0.2.0 | 1 | `84b727522c513376` | 编译轨 PASS · 解释轨**设计性**不支持并发（PX-DEF-006） | 已存在（内容一致） |  |
| x509 | 0.1.0 | 1 | `0fcb8715961a6555` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| x509 | 0.2.0 | 1 | `11f55acde1713313` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| xlsx | 0.1.0 | 2 | `eb1e3a894c1dc2f4` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| xlsx | 0.2.0 | 2 | `87ecc46389910f1f` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| xmlrpc | 0.1.0 | 1 | `84673070311e0c08` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| xmlstream | 0.1.0 | 1 | `6d0fc56ca49a2fd8` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| xmlstream | 0.2.0 | 1 | `9b07f7201bda6670` | **双轨 PASS**（本地 0.2.0 · 用例 `xmlstream2_test` 解释轨+编译轨全通过） | 本地修（晨曦 2026-11-09 送包，**非上游照搬**，M382） |  |
| xpath | 0.1.0 | 2 | `80311caa7fcef339` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） | 补丁：`xpath.patch` |
| zlib | 0.1.0 | 1 | `3150483266a834de` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |
| zlib | 0.2.0 | 1 | `ab129176bd5304fe` | **双轨 PASS**（上游用例回归 · 逐条结果与 XFAIL 登记见 `docs/UPSTREAM_020_DEFECTS.md`） | 已存在（内容一致） |  |

## § 本地自有包（**本仓维护 · 不由上游覆盖**）

> 判定口径：`registry/` 下有、**上游 `registry-px` 同版本目录没有** ⇒ 本地自有。
> 生成器每次重算 sha256 与文件数（与上游表同形），故 m187 的「表 ⇔ 磁盘」对拍对它们同样有效。
> ⚠️ 不要因为「上游没有」就当缺失删掉 —— 它们是本仓的包，只是上游不收录。

| 包 | 版本 | 文件数 | 入口 sha256（前 16） | 三轨验证 | 备注 | 补丁 |
|---|---|---|---|---|---|---|
| checksum | 0.1.0 | 1 | `a57ec66c4d20999d` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| checksum | 0.2.0 | 1 | `a3134405285c6a00` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| collections | 0.1.0 | 1 | `5f985a49e1265622` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| collections | 0.2.0 | 1 | `65cfa284bba831d2` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| cookiejar | 0.1.0 | 1 | `d8799d0c9156ab10` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| cookiejar | 0.2.0 | 1 | `69feab5b9f7f6420` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| edge | 0.1.0 | 1 | `46a2966fb6a92151` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| gfx | 0.1.0 | 1 | `b9be78feb906d539` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| html | 0.1.0 | 1 | `4f71970be9189dc3` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| lunar | 0.1.0 | 1 | `88ff9fc9fdc0587f` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| multipart | 0.1.0 | 1 | `2bca043d82b9e2b9` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| png | 0.1.0 | 1 | `d3518b69f434dc47` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| pxml | 0.1.0 | 1 | `0a0d8c67cf30fa73` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| pxml | 0.2.0 | 1 | `22d14f005799ce94` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| semver | 0.1.0 | 1 | `40cc1850e0e8938f` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| smtp | 0.1.0 | 1 | `e85f0766b9d37d52` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| webroute | 0.1.0 | 1 | `81b6823bf17210a0` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| yaml | 0.1.0 | 1 | `0f535d1e3c256888` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
| yaml | 0.2.0 | 1 | `5fdafafd85c38c19` | —（本仓自研 · 无上游用例） | **本地自有**（上游无此包） |  |
