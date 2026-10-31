# M91_PLAN · 默认轨切 VM（px build 默认产物 = 字节码 VM 轨）

> 立项依据：docs/M90_S4_switch_eval.md（技术前置全齐）+ docs/M89_PERF_BASELINE.md
> （性能代价实测：服务/综合负载 ≈ 0~10%，纯计算热点 ~1.5x 由 C 轨逃生舱兜底）。
> M91 = 把 M89 VM 化旗舰成果**固化为用户默认路径**：`px build <f.px>` 默认产物
> 从 C 文本轨（fn_* → gcc）切到 VM 轨（BCModule 字节码镜像 C → gcc 链含 vm.o），
> C 轨退 `--c` 逃生舱。按 S0-S4 流程推进。
> 维护：东月（dongyue）。状态随进度更新。

## 〇、一句话

消除双轨发散税的第一步（默认 = VM），把 C 轨从"默认"降为"逃生舱"，为精确 GC
终极项 / 帧协程 / native 后端建立单一默认底座。

## 一、S0 决策记录

- **版本**：维持 `SELFHOST_VER=0.2.0`（M89 升格后无字节码/语言语义变化；默认引擎
  切换产物行为一致）。收口 tag `v0.2.0-m91`（RELEASE_PROCESS 约定 vX.Y.Z-mNN）。
- **范围**：tools/px（用户入口）默认引擎反转 + `--c`/env 逃生舱 + VM 轨自动裁剪
  补齐 + 验证资产 + 回归 + 重链批次 + 文档。**不在范围**：VM 唯一轨（仍保留 C 轨
  逃生舱）、精确 GC 终极项、native/JIT、diffcheck golden 迁移（diffcheck 守护的是
  C 轨 codegen 与解释器，C 轨仍是逃生舱须继续守护，见 S1 事实 3）。
- **风险预案**：默认切换若在回归中发现 VM 轨语义偏差 → 单文件回退
  `PX_BUILD_ENGINE=c`（env 逃生舱已内置），不阻塞用户。

## 二、S1 详勘事实（2026-09-09，全部实测确认）

1. **切换点**：tools/px `cmd_build`——`local ... vm=0`；`--vm|--bc` 置 vm=1；
   VM 编译器优先级 `PXC_VM_BIN > bootstrap/pxc_vm > selfhost/build/compiler_vm`，
   cmp_sub=`--emit-c`。VM 轨产物 .c = `BCModule 镜像`（`s_K/s_N/s_G/s_structs/
   s_funcs/s_mod` + `main → px_vm_run_module`），`s_G` 为 `static const char*`
   字符串表，含全部全局/native 引用名（实测 http_json.c：`handler/main/
   json_stringify/env/int/http_serve`）。
2. **自动裁剪缺口**：`if [ "$full" = 0 ] && [ "$vm" = 0 ]` → VM 轨跳自动裁剪。
   实测裸 hello：C 轨自动裁剪 2,736,992 B vs VM 轨全能力 9,033,936 B。VM 产物
   无 `px_get_global`，但 `s_G` 表可提取引用名（等价信息）→ **VM 轨自动裁剪可补**。
3. **自举守护不受影响**：diffcheck.sh / bootstrap_prove*.sh 直调 `bootstrap/pxc`
   （不走 tools/px build）→ 默认引擎切换不动自举对拍基线；C 轨 codegen golden
   继续守护逃生舱正确性。
4. **px refs 不需改**：M86-S1 `cmd_refs` 提取 C 轨 `px_get_global` 引用集；C 轨
   仍为逃生舱 → 命令继续有效（它是"源的引用集"，与默认引擎无关）。文档注明即可。
5. **VM 编译器已入库**：bootstrap/pxc_vm 存在（9,480,688 B 静态 ELF）→ 默认引擎
   前置满足；缺省时 tools/px 已有友好报错指引。
6. **内部调用面**：走 tools/px build 的内部点极少（pxmcp build 工具 + release 冒烟
   hello）；默认 VM 产物运行语义一致 → 冒烟仍过，无内部行为破坏。
7. **遗留并入**：F3-fix（de5ceef）改 runtime.c（fserve_gc_reg + 超时 tick 兜底）后
   bootstrap/pxi、pxc_vm、pxi_vm 等**未重链**（M90 收口建议项）→ M91-S4 收口并入
   重链批次，让解释器/编译器二进制也带 F3-fix 的 runtime。

## 三、S2 设计（tools/px 改动点）

### 引擎选择语义（优先级 高→低）
1. 显式 flag：`--c`（engine=c）与 `--vm|--bc`（engine=vm）——循环内后出现者胜
   （bash flag 顺序天然覆盖），帮助文案注明。
2. `PX_BUILD_ENGINE` env：`c` → C 轨；`vm` → VM 轨；其它值忽略并警告（退默认）。
3. 默认：`vm`（M91 切换点）。

### cmd_build 改动
- 初始化：`engine="${PX_BUILD_ENGINE:-vm}"`；解析 `--c`→engine=c、`--vm|--bc`→engine=vm。
- 编译器选择分支：`engine=vm` → 原 vm=1 逻辑（PXC_VM_BIN>bootstrap/pxc_vm>dev
  compiler_vm + `--emit-c` + 缺省报错）；`engine=c` → 原默认（bootstrap/pxc build）。
- 自动裁剪：条件放宽为 `full=0`（C 轨与 VM 轨都做）；提取分轨：
  - C 轨：grep `px_get_global("name")`（现状不变）
  - VM 轨：解析产物 .c 的 `s_G[]` 块内 `"name"` 字符串 → 同名集合
  - 后续 map 反推/补裁逻辑两轨共用（现状已共用）。
- usage/头部注释同步（默认轨说明、--c/--vm、PX_BUILD_ENGINE）。

### 验证资产（examples/m91_s1/）
`verify.sh` 断言（全部真实执行）：
1. 默认 `px build hello` → 产物 .c 含 `字节码模块（VM 执行）` 特征头（VM 轨）
2. `px build --c hello` → 产物 .c 为 C 文本轨（无该头、含 fn_* 特征）
3. `PX_BUILD_ENGINE=c px build hello` → C 轨（env 逃生舱生效）
4. VM 轨自动裁剪：默认 hello 产物 < 4,000,000 B（非 9MB 全能力）
5. 双轨运行 stdout 逐字节一致
6. 显式 flag 覆盖 env：`PX_BUILD_ENGINE=c px build --vm hello` → VM 轨

### 收口回归门
- tools/px 冒烟：默认/--c/env/flag 组合 + hello/ffi(sqlite+json+http) 双轨对拍
- selfhost 门：diffcheck --all + bootstrap_prove.sh + bootstrap_prove_bc.sh
- VM 语义门：vm_ab（38 例）+ m90_s1(5) + m89_s3d(9) + m82 unix + m83_s6
- capability：解释/编译 253 PASS（C 轨逃生舱仍可用）
- 重链批次：bootstrap/pxi、pxc_vm、pxi_vm 等全模块重链（吸收 F3-fix runtime）+
  重链后复跑 diffcheck/vm_ab 抽查
- 文档：CHANGELOG / README / spec.md usage 段 / M90_S4_switch_eval 状态标注

## 四、S 级拆分与状态

| 段 | 内容 | 状态 |
|---|---|---|
| S0/S1 | 立项 + 详勘（本篇） | ✅ |
| S2 | 设计定稿（本篇 §三） | ✅ |
| S3a | tools/px 引擎反转 + --c/env + usage | ✅（fca0cee） |
| S3b | VM 轨自动裁剪（s_G 提取） | ✅（fca0cee） |
| S3c | examples/m91_s1/verify.sh + 冒烟 | ✅（7 PASS） |
| S4 | 收口：全回归 + 文档 + tag v0.2.0-m91 | ⏳ 回归全绿，待 push/tag |

## S4 收口记录（2026-09-09）

- 回归全绿：m91_s1 7 / m90_s1 5 / m89_s3d 9 / vm_ab v2 **38 PASS 0 GAP 0 FAIL**
  / diffcheck --all / m82 unix（issue28 场景默认 VM --no-quic）全 PASS / m83_s6
  SSE 全 PASS / 双自举证明（C 轨 B.c==golden 15058 行 + BC 轨 VM 重放==bc.dump）。
- **附带修复**：examples/m89_a2/vm_run.sh cache 选择——"最新含 vm.o rtcache"在 M91
  默认自动裁剪后会捡到裁剪态 cache（缺 aes/xml/zip .o）→ p7 等 2 例报"未定义变量"
  伪 GAP；改为优先全能力 cache（含全部模块 .o），无则回退最新并警告。修复后
  vm_ab 38 PASS 0 GAP（与 M90-S1 基线一致，证实原 2 GAP 为伪缺口非 VM 轨缺陷）。
- 文档：CHANGELOG M91 条目 + README/spec.md build 描述 + M90_S4_switch_eval 状态。
- 后续批次（标注未并入）：bootstrap/pxi/pxi_vm 重链吸收 F3-fix runtime（M91_PLAN
  S1 事实 7，收口评估为独立批次更稳——基座大二进制变更 + 需全量重验，收益集中于
  解释器跑 http_serve 长压的非主用例）。
