# M90-S4 · 默认轨切 VM 立项评估（收口）

> **状态（2026-09-09，dongyue）**：本评估建议的默认轨切换已按 M91 立项执行并收口
> （docs/M91_PLAN.md + CHANGELOG）：`px build` 默认 = VM 字节码轨（compiler_vm
> --emit-c → BCModule 镜像），`--c`/`PX_BUILD_ENGINE=c` = C 轨逃生舱；VM 轨自动
> 裁剪补齐（s_G 引用集提取，hello 9.03MB→2.74MB 无体积回归）；pxc_vm 静态重链
> 入库（含 F1 默认参数）；m91_s1/m90_s1/m89_s3d/vm_ab 38/diffcheck/双自举/m82/
> m83_s6 回归全绿。

> 完成（2026-09-09，dongyue）：M90 清障与验证段收口。本文 = 默认轨切换
> （px build 默认产物 C 文本 → BCModule 字节码镜像，运行于显式帧 VM）的
> **立项评估报告**：前置就绪度、风险、迁移范围、AB 回退、排期建议。

## 1. 切换定义

- **现状**：`px build <f.px>` 默认走 C 轨（pxc：AST→C 文本 fn_* → gcc 静态）；
  VM 轨经 `px build --vm`（compiler_vm：AST→BCModule → 静态 C 初值 + VM main）。
- **切换后**：默认产物 = VM 轨（字节码镜像）；旧 C 轨退**逃生舱**（`--c` 显式 /
  env 开关），用于纯计算热点程序（perf 数据：fib 1.52x）直至 native/JIT 后端。
- **本质**：永久消除双轨发散税（双编译器/双 bootstrap/38 例对拍/双 golden 终身
  维护），解锁纯精确 GC 终极项 + 帧协程 + native 后端的单一稳定底座。

## 2. 前置就绪度（M90 已验证，全绿）

| 前置 | 状态 | 证据 |
|---|---|---|
| F1 bc_emit 默认参数（capability 41KB 无法 VM 编的唯一缺口） | ✅ | capability.px `bc` rc=1→0；default_args 双轨一致 |
| 系统库全 VM 编译 | ✅ | stdlib 13/13 VM 编译通过；capability（import sqlite3/webroute/semver/collections）全量 VM 运行 == pxi 303 行逐字节（253 PASS） |
| VM 语义对齐 | ✅ | vm_ab 38 PASS 0 GAP（确定性 examples 全集）；diffcheck --all；双自举证明（C/BC golden） |
| 性能代价量化 | ✅ | docs/M89_PERF_BASELINE.md：fib 1.52x / compiler 形态 1.04x / HTTP 服务 1.00x（8 并发短测） |
| continue-for 死循环（M89 遗留） | ✅ | M90-S1 附带修复 + 回归资产（m90_s1/verify.sh 项 5） |

## 3. 切换前剩余风险与处置建议

1. **F3 服务层 bug（docs/M90_S2_F3.md）**：keep-alive >15s 压测崩溃（与轨无关，
   C/VM 都中）。服务型负载是默认轨主场景 → **建议切换前先落 F3-fix**（超时
   close 前二次确认兜底 + 事件循环漏报深查），否则切换后服务负载体验不升反险。
2. **非确定性 examples 面**：vm_ab 覆盖 38 确定性例；其余（server/网络/verify
   专项）切换后行为取决于服务层/专项 verify，须在切换稳定期按专项 verify.sh
   抽测（如 m53/m57/m88 各专项）。
3. **纯计算热点用户**：~50% 性能代价 → C 轨逃生舱文档 + native/JIT 路线图兜底。

## 4. golden 大迁移范围（切换里程碑主体工作）

| 面 | 内容 | 动作 |
|---|---|---|
| diffcheck golden | lex/parse/codegen/run 全量 .stdout/.c | 基准切换 VM 轨后重生成 + 逐字节复核（复用既有 norm） |
| capability golden | 能力自检输出 | VM 轨 == pxi 已证（303 行），固化 golden |
| 双自举镜像 | golden/compiler.c（C 轨 15058）+ compiler.bc.dump（BC 轨 30577） | 切换后 C 轨镜像降级为逃生舱基准（保留，不再主守护）；BC 镜像继续主守护 |
| bootstrap 二进制 | pxc（C 轨编译器）/ pxc_vm / pxi / pxi_vm | 切换后 pxc_vm 为默认编译器；pxc 留逃生舱；重链全量 rtcache |
| examples/build 产物 | 各例 C 轨 .c/.o | 随默认切 VM 自动（build 产物 gitignore，无迁移成本） |

## 5. AB 回退开关设计

- 编译期：`px build`（默认 VM）+ `px build --c`（C 轨逃生舱）——**产物运行期
  无差别**（都静态 C 可执行），AB 在 build 命令层，零运行时成本。
- env 逃生舱：`PX_BUILD_ENGINE=c|vm` 供脚本/CI 一次性回退（M86-S2 同款 env
  覆盖心智：PXC_VM_BIN 已有先例）。
- 稳定期门：全量 golden 迁移 + vm_ab + 专项 verify + issue28 类服务压测 + 自举
  证明双轨在切换后连续跑 3 次全绿 → 视为切稳。

## 6. 排期建议（F3-fix 之后启动，S0-S4 完整流程）

- **阶段 0 立项**：本篇为评估底稿；切本身独立里程碑（如 M91），S0 详勘 golden
  迁移工具链。
- **建议次序**：① F3-fix（服务稳定性，独立小批）→ ② 默认轨切换里程碑（golden
  迁移 + 逃生舱 + AB env + 稳定期门）→ ③ VM 唯一轨后：退役整栈保守扫描 + 原生
  桥 ≤60 登记 + cell 标记（纯精确 GC 终极项，M89-S3-D 归档后置批次）。
- **暂缓项**：native/JIT 后端（纯计算热点 ~50% 逃生舱可兜，非切换阻塞）。

## 7. 结论

默认轨切换**技术前置已齐**（F1/stdlib/语义对拍/perf 数据均绿），唯一建议先行项
= F3-fix（服务层稳定性，与轨无关但决定服务型负载体验）。切换本身 = 低风险工程
（产物语义不变、AB 在 build 层、golden 迁移机制已有），可按 S0-S4 立项推进，
无技术 blocker。
