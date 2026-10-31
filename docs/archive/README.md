# docs/archive/ —— 历史档案（不要当现状读）

本目录收纳**按里程碑一次性使用的开工计划 / 预研 / 评估文档**（M366 整理）。
它们记录的是**当时**的判断与实验协议，后续里程碑可能已推翻其结论。

## 读法与替代

| 你想要 | 该读 |
|---|---|
| 某项能力**现在**是什么行为 | `../spec.md` + `../PUXIAN_CHEATSHEET.md` + `../README.md`（A 区当前态文档） |
| 某一轮**为什么**这么改、代价多少、被否证了什么 | 仓库根 `CHANGELOG.md` 对应里程碑节（**唯一权威归档**） |
| 缺陷的编号与定性 | `../PX_DEF_TRIAGE.md` · `../DEFECT_NUMBERING.md` |

## 收录内容

- `M53_PLAN.md` … `M123_PLAN.md` —— 早期里程碑开工计划（逐轮一个）
- `M89_vm_design.md` / `M89_vm_prestudy.md` / `M89_PERF_BASELINE.md` / `M90_S2_F3.md` /
  `M90_S4_switch_eval.md` / `M95_S5_PLAN.md` / `M104_native_prestudy.md` —— 设计预研与阶段评估
- `M308_PLAN.md` … `M311_PLAN.md` —— ONNX 线与门成本线计划
- `M353_QUIET_PREMISE.md` / `M355_PIPE_SWEEP.md` —— 门抖动治理期的专项分析
- `ISSUE28_PLAN.md` —— 早期 issue 专项计划
- `pxi_native_diff.md` —— M68 前后的解释器 / 编译轨 native 差异表（该缺口已由 M68 根治，保留供追溯）

> 引用约定：全仓对它们的引用已统一改为 `docs/archive/<名>`（M366）。
> 新增一次性计划文档时**直接写进本目录**，不要堆在 `docs/` 根部 —— 根部只放「当前态」文档。
