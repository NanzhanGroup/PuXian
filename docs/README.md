# PuXian 文档索引（docs/）

> 本文件回答三个问题：**哪个文档管什么** · **我该从哪读起** · **哪些是历史档案（可以不全信）**。
> 维护约定：新增/改名文档时同步本索引；「当前态」文档（下表 A 区）随里程碑更新，历史档案不追溯修改。

---

## A 区 · 当前态文档（以最新里程碑为准，改了代码就要同步）

| 文档 | 管什么 | 谁该读 |
|---|---|---|
| [`spec.md`](spec.md) | **语言规格说明书**（唯一权威语义来源）：词法 / 类型 / 表达式 / 语句 / 函数闭包 / 并发 / 模块 / 双模式执行 / 标准库约定 / 错误码（§11）/ 工具链接口（§12）/ 砍掉清单 / edition / Python 兼容边界 / 示例 / **§17 语义一致性收口** | 所有人；改语言语义前**先读 §17** |
| [`PUXIAN_CHEATSHEET.md`](PUXIAN_CHEATSHEET.md) | **AI 速查包**（给大模型整包喂）：易错事实表、native 名册、三轨差异与统一口径、逐里程碑「事实 NNN」条目 | 让 AI 写 `.px` 的人；排障时先搜这里 |
| [`MINI_SUBSET.md`](MINI_SUBSET.md) | **Mini 子集规范**：自举编译器只会正确编译的语言面（支持 / 明确排除 / 已知限制）。写 `selfhost/*.px` 的硬约束 | 改编译器/解释器源码的人 |
| [`DICT_STRICT_MIGRATION.md`](DICT_STRICT_MIGRATION.md) | **严格化迁移说明**（M163–M167）：字典键、迭代快照、解包形状、`items()` —— 老代码/permission 迁移怎么改 | 存量 `.px` 维护者 |
| [`HTTP2_DECISION.md`](HTTP2_DECISION.md) | **HTTP/2 与 HTTP/3 的口径**（M180）：**h2 不做**（理由 + 迁移动作 + 重评估触发条件）· H3 = `opts{"http3": true}` + 自动 `Alt-Svc`（x86_64 默认链 ngtcp2；`--no-quic` 下要求 H3 ⇒ 响亮报错）· `px build` 能力行 `quic=on\|off` | 部署/运维；给外部（如 Ma 侧）定文档时**直接引用** |
| [`ECOSYSTEM.md`](ECOSYSTEM.md) | **生态总览**：13 个公开库的定位与导出 API、示例能力导航、消费路径（import / pxpkg / 拷源码）、机器索引与防漂移 | 写库 / 用库的人 |
| [`ROADMAP.md`](ROADMAP.md) | 路线图：能力基线、已完成主线、远期方向、语言面欠账 | 想了解进度与方向的人 |
| [`RELEASE_PROCESS.md`](RELEASE_PROCESS.md) | 发布 SOP：tag 驱动自动发布、发布物清单（含 aarch64 并列资产）、漏打 tag 守卫 | 发版的人 |
| [`ECOSYSTEM_GAPS.md`](ECOSYSTEM_GAPS.md) | 写库规范 checklist + 语言缺口评估（含历史结论与后续处置） | 写库/报缺口的人 |
| [`GAP_ANALYSIS.md`](GAP_ANALYSIS.md) | 能力差距分析（边缘设备 / 2D-3D 游戏两条线的差距清单） | 排期与选型 |
| [`PXML.md`](PXML.md) | PXML 配置语言规范（`std.pxml` 背后的语言） | 用 PXML 的人 |

### 机器索引（不要手改，用生成器）

| 文件 | 生成器 | CI 门 |
|---|---|---|
| [`native_index.json`](native_index.json) | `tools/gen_native_table.sh`（扫 `runtime/*.c` 的 `px_set_global`） | 重跑后 `git diff --exit-code` |
| [`ecosystem_index.json`](ecosystem_index.json) | `tools/gen_ecosystem.px`（扫 `stdlib/*.px` 顶层 `def`） | 同上 |

> 改 `stdlib/` 或 runtime native 名册 ⇒ **必须重跑生成器并同步 `ECOSYSTEM.md` / `PUXIAN_CHEATSHEET.md`**，否则 CI 的「生态索引防漂移」步会红。

---

## B 区 · 按角色读

- **第一次接触 PuXian**：根 [`README.md`](../README.md)（快速开始 + 能力一览）→ `spec.md` §1–§7 → `ECOSYSTEM.md` §1（能用什么库）。
- **让 AI 写 PuXian**：把 `PUXIAN_CHEATSHEET.md` + `spec.md` §17 一起喂；速查表是为「整包投喂」写的。
- **写库 / 写生产应用**：`ECOSYSTEM.md` + `ECOSYSTEM_GAPS.md` §1 checklist + `DICT_STRICT_MIGRATION.md`。
- **改编译器 / 解释器 / runtime**：`MINI_SUBSET.md` → `spec.md` §17 → 根 `CONTRIBUTING.md`（门与重定基纪律）→ `ROADMAP.md` §五（验证体系）。
- **发版**：`RELEASE_PROCESS.md`。
- **排查「同一份源码两轨结果不同」**：先查 `spec.md` §17 与速查表「三轨差异」条目 —— 若规则已收口，那就是 bug，请带最小复现提 issue。

---

## C 区 · 历史档案（按里程碑记录，可追溯，不要当现状读）

- `M53_PLAN.md` … `M123_PLAN.md`、`M89_*`、`M90_S*`、`M95_S5_PLAN.md`、`M104_native_prestudy.md` 等 —— **各里程碑的开工计划 / 预研 / 设计文档**。
  它们的「结论」只在**当时**成立；后续里程碑可能已推翻（例：`M104_native_prestudy.md` 时代的 native 名册与裁剪口径与今天不同）。
  要现状 ⇒ 读 A 区 + `CHANGELOG.md` 对应里程碑节。
- `pxi_native_diff.md` —— M68 前后的解释器/编译轨 native 差异分析（该缺口已由 M68 根治，保留供追溯）。
- `ISSUE28_PLAN.md` —— 单个 issue 的处置计划。

> 判断某文档是否「当前态」的快捷办法：看它头部有没有「更新：<里程碑>」行；A 区文档都会随里程碑刷新头部。
