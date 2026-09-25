# upstream-tests —— 上游 registry-px 真实用例（M190 引入）

> **来源**：`github.com/banshanhanfu/registry-px` @ `d4ae476d5c356b9a5b9bc495182c6722d64403af`（M190 建 · M200/M201/M210 逐次再引入）
> **许可**：Apache-2.0（与 `registry/THIRD_PARTY.md` 登记的第三方库同源、同许可）
> **纪律**：这 112 个 `.px` **逐字节照搬**上游 `tests/`（`MANIFEST.sha256` 逐文件对拍，
> 本地改动 ⇒ `selfhost/run_upstream_tests.sh` 硬失败）。**不改一个字节**。

## 为什么在官方仓库里放第三方的用例

M187/M189 起把上游库照搬进 `registry/`（首批 53 个，M210 已至 108 个）时，只做了 **import 冒烟**（能编译、能调到入口）。
冒烟证明不了「库能用」—— 真正逐库的行为断言在这里（112 个用例）：
`base58_encode(NUL+"abc") == "1ZiCa"`、`glob_match("*.log","sub/a.log") == false`、
`dotenv_parse("A=\"a # b\"")` 等等。把它们跑在**我方官方 registry** 上，
「照搬 ⇒ 可用」这一环才算闭合。

## 目录契约（为什么放在仓库根）

用例里的相对 import 写死了 `../registry/<name>/<version>/<name>.px`：

```
<repo>/upstream-tests/base58_test.px     ← 用例
              └── ../registry/base58/0.1.0/base58.px  →  <repo>/registry/base58/0.1.0/base58.px
```

⇒ 用例放在**仓库根**的 `upstream-tests/` 下，`../registry/...` 正好命中**本仓库官方 registry**。
（回归器实际是在临时工作区里 stage 一份 `tests/` + `registry/` 软链，便于负控替换 registry。）

## 怎么跑

```bash
selfhost/run_upstream_tests.sh                 # 双轨（build + interp），与 EXPECTED.tsv 对拍
selfhost/run_upstream_tests.sh --list          # 打印每个用例的期望值与理由
selfhost/run_upstream_tests.sh --track build   # 只跑编译轨
selfhost/run_upstream_tests.sh --only a,b,c  # 只跑指定用例（名字含 `_test` 后缀，见 EXPECTED.tsv 第一列）
selfhost/run_upstream_tests.sh --run-skipped   # 连期望跳过的也跑（本机有 DB/想看性能时）
```

三条硬判据（**别松**）：

1. **用例逐字节照搬** —— `sha256sum -c MANIFEST.sha256` 不过 ⇒ 直接失败；
2. **引用面完整** —— 每个 `../registry/...` 必须在被测 registry 里真实存在
   （M188 事故：stdlib 定位失败时「找不到模块」只是**警告**、程序照跑 ⇒ 用例静默降级）；
3. **期望值是数据、不是"跑出来的结果"** —— `EXPECTED.tsv` 手写登记；实际与期望不符
   （含"该跳过却跑了"以外的任何偏离）⇒ 失败。

## fixture（上游仓库**没有** fixture 脚本）

`dotenv_test` 与 `glob_test` 依赖 `/tmp` 预置，回归器自建：

| 路径 | 内容 | 谁要 |
|---|---|---|
| `/tmp/dotenv_sample.env` | `USER=test` + `QUOTED="a b"` | dotenv_test |
| `/tmp/globd/{a.log,b.txt,sub/c.log}` | 恰 3 个条目（`glob_list("*")` 断言 `len==3`；`sub/*` 断言子目录） | glob_test |

**网络用例**的 mock 服务端在 `upstream-tests/fixtures/`（**本仓自建**，用普贤写的，不引 python3/netcat）：

| fixture | 端口 | 谁要 | 备注 |
|---|---|---|---|
| `oauth2_mock.px` | 19090 | `oauth2_test` | M201 建 |
| `pop3_mock.px` | 2110 | `pop3_test` | M201 建 |
| `ftp_mock.px` | 2121（控制）/2122（数据） | `ftp_test` | M201 建（用到 `spawn` 并发） |
| `imap_mock.px` | 1143 | `imap_test` | **M210 建** |

`fixtures/` 里的文件是**本仓自己的**，不参与 `MANIFEST.sha256` 对拍、也不参与「逐字节照搬」判据。

⇒ 上游若补了 fixture 脚本，这里可改为调用它（已在其仓登记为"文档缺口"）。

## EXPECTED.tsv

字段：`<测试>\t<build 期望>\t<interp 期望>\t<理由>`，取值 `PASS` / `SKIP`。

- `SKIP` 只用于**环境不具备**或**设计性不支持**的项，每项**必须写理由**；
- 新增上游用例（或上游改了用例）时：先更 `MANIFEST.sha256`，再**显式登记**期望值 ——
  缺登记 ⇒ 回归器报 `MISMATCH`（防止"新用例默默不跑"）。

## 纳入 CI

`.github/workflows/ci.yml` 的 `toolchain` job 与本地 `selfhost/m116_gates.sh` 都跑这条门。
