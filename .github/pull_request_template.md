## 变更概述

请简要说明本次 PR 做了什么、为什么。

## 验证清单（提交前勾选）

- [ ] `./selfhost/diffcheck.sh --all` 全绿（或注明未跑原因）
- [ ] `./selfhost/diffcheck.sh --errors` 全绿
- [ ] 能力自检双模式 PASS（capability.px 编译 + 解释）
- [ ] 自举证明 `bootstrap_prove.sh` 通过（或注明：未触及 compiler.px，已跳过）
- [ ] 涉及新功能时已补对应用例（`selfhost/cases/` 或 `examples/`）并更新 golden
- [ ] 涉及语言/运行时变更时已同步 `docs/spec.md` / `docs/MINI_SUBSET.md`
- [ ] 提交信息符合仓库风格（`M## 标题` + 要点列表）

## 相关 Issue

Closes #（如有）
