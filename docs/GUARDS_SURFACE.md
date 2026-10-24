# 守卫扫描面体检表（M326）

> 为什么有这张表：`579` 是同一缺陷族的**第 3 次**——
> `496①`（只看已跟踪文件）→ `491`（同形，另一个守卫）→ `579`（按扩展名取面，漏掉无扩展名 git 钩子）。
> 三次都不是「判据写错了」，而是**判据的扫描面比判据本身更容易出错**。
> ⇒ 本轮把「带静态扫描面的守卫」逐个对拍：**现扫描面 ⇄ 判据真正要覆盖的集合**，
> 反例必须是**本仓可复现**的失效形状，不是假想。
>
> 口径（三条，缺一不可）：
> ① 每条结论都要有**反例**（没有反例 = 没说清它在防什么）；
> ② 「边界」**写出来**（写出来的边界是设计，没写出来的是洞）；
> ③ 放宽点必须配**负控**（放宽 = 判据变松 = 新漏报风险，见 M326 只改 1 处）。
>
> 体检日：2026-10-24（第 197 轮后，HEAD `d826e15`）。守卫清单来自 `selfhost/gates.registry.sh`。

## 一 总表（9 个守卫）

| # | 守卫 | 现扫描面（源码事实） | 判据应覆盖的集合 | 反例（本仓可复现） | 结论 |
|---|------|---------------------|-----------------|-------------------|------|
| 1 | `check_gate_registry.sh` | `gates.registry.sh` 里 `run … examples/*/verify.sh` + `ci.yml` 里 `bash examples/*/verify.sh`，各 100+ 条 | **example 门**清单 ⇄ CI 双向；例外表带理由且必须仍成立 | 新增 `examples/m999/verify.sh` 只写进本地清单 ⇒ 本门判红（`only_local` 非空） | **边界（设计）**：只对拍 example 门。非 example 门（`selfhost/*.sh` 门）本就不在 CI 显式清单里（CI 走全量档），故不同口径。**不是漏**：`--list` 233 门有规模锚点 |
| 2 | `check_gate_paths.sh` | `examples/*/verify.sh` + `examples/*/*.py` + `examples/*/*.sh` + `selfhost/*.{sh,py}` + `tools/*.{sh,py}` + `packaging/*.sh` + `packaging/chain/*.sh` + `.github/workflows/*.yml` | 一切**会在 CI 跑**的脚本里的开发机绝对路径 | M317 实测：`examples/m316_http_dup_host/negctl.py` 写死 `/data/code/puxian/runtime/runtime.c` ⇒ 分片档打桩打到主仓 ⇒ 假绿 | **已扩面（M317）**，本轮复核通过。剩余边界：`examples/*/*/*.sh`（第三层）与 `packaging/*/*.py` 不在面内 —— 实测第三层 15 个 `.sh` **全部是 `examples/*/fixtures/`**（夹具按 M226/M227 纪律不扫），`packaging/chain/*.py` 2 个、0 处开发机路径 ⇒ 当前无实例，扩面留给有真实命中时 |
| 3 | `check_pymain_guard.py` | `SCAN_DIRS` 下**所有** `.py`（`os.walk`，只跳 `SKIP_DIRS`） | 被 import 的独立 runner 必须有 `__main__` 守卫 | `three_tracks.py` 末尾模块级 `sys.exit(main())` ⇒ 一 import 就执行并退出调用方（M246 实锤） | **面完整**：`os.walk` 按目录取面，没有扩展名/glob 失配问题（`.py` 是唯一被 import 的语言） |
| 4 | `check_defect_refs.py` | `runtime/*.c` · `runtime/*.h` · `selfhost/*.px`（62 个文件） | CHANGELOG 里程碑声明的缺陷号 ⇄ **代码里**的引用 | `selfhost/*.sh` 里有 **30+ 处**「缺陷 NNN」引用，全在面外 ⇒ 只在 `.sh` 里引用的缺陷号不会被本门看见 | **边界（本轮记下，候选 M327）**：现仓方向是「声明 ⊂ 引用」，`CHANGELOG` 声明只来自 `.c/.px` 语境，故当前不假绿；但**若今后只在 `.sh/.py` 里引用**，本门会静默漏判。改面须同时回收 `--self-test` 夹具口径，本轮不动（改判据 ≠ 加例外，不在同一提交里做两件事） |
| 5 | `check_neg_residue.sh` | `selfhost runtime tools stdlib` 下 `.px .c .h .py .sh .go`；豁免表 4 条、上限 8；规模锚点 ≥200 文件 | **参与重烘的树**里的负控标记残留 | 448 实测：旧面只有 `.px/.c` ⇒ `gate_par.py` 的 `NEGCTL-M244-B` 残留逃过自检 | **面已按 448 扩过，本轮复核通过**。边界：`packaging/` 不在面内，而实测 `packaging/selftest_make_tag.sh` **含 `NEGCTL`** ⇒ 若 packaging 也进重烘面，这是**第一个要收的残留**（候选 M327，需带理由的豁免或改写该 fixture） |
| 6 | `check_emitc_hook.sh` | 真跑 `.githooks/pre-commit`；冻结集 glob 从 `emitc_freeze.sh` **同源提取**（正则只认 `selfhost/cases/*.px` 与 `examples/*/*.px` 两种形状）+ A8 断言「恰好 2 条」 | 「提交命中冻结集 `.px` 而未同交基准」必须在提交时被拦 | `A8` 就是反例：正则写窄 ⇒ 只提 1 条 ⇒ 新增 `probe.px` 不被拦（M325 实测抓到） | **面够用但边界要写死**：A8 限定 2 条 ⇒ `emitc_freeze.sh` 若新增**第三种 shape** 的 glob，A8 仍绿而钩子漏拦。结论：**加 glob 必须同步改正则 + A8 数字**（写进 `emitc_freeze.sh` 头注） |
| 7 | `check_sed_inplace.sh` | `selfhost/*.sh` · `tools/*.sh` · `packaging/*.sh` · `examples/*/verify.sh`（310 个脚本）；**排除自身** | 「就地改写**跟踪文件**」必须登记（台账双向相等 + restore 声明可核对） | 574 原件：`sed -i` 吃字符把两行 `trap` 改坏，`bash -n` 仍绿 | **边界（可接受，已写注释）**：面不含 `selfhost/*.py`、`packaging/chain/*.sh`、子目录 `.sh`。本轮实测这些面内 `sed -i` 命中 **0 处现役**（仅注释/fixture 字符串，`$CO` 变量目标也不算）⇒ 缺口暂无实例。**加面 = 加台账负担**，待有真实命中再扩 |
| 8 | `release_readiness.py` | R0 只解析**两种权威档**日志：分片档（shard1..4 + quiet）与串行档（`══ 汇总：失败 N 项` + 逐门行） | tag 前 7 条裁决 + 两档日志自洽（汇总失败数 == 逐门红数） | 558 实测：串行档被判「汇总段缺失」⇒ R0 假红；N11：汇总 2 而逐门红 0 ⇒ 必须红 | **面按实测扩过一次（558）**，自证 11/0（含 4 条负控）。边界：`run_gates.sh` 若改输出格式，R0 靠格式串识别 ⇒ **改格式必须同步改 R0 + N9–N11**（已注释） |
| 9 | `run_gates.sh` 门选择 | `run()` 内按**门名**过滤：`--list` 233 行、`--only/--skip` 精确名匹配、`_in_list` 逗号集合 | 「注册了就必须真跑；`--list` ⇄ 实际执行」守恒 | 门名拼错 ⇒ 该门被 **SKIP**（计入 SKIPPED，不判红）；只有 `run` 包裹的门才会出现在 `--list` | **边界（设计）**：`--only` 是迭代档，静默 SKIP 是它的语义；守恒由 `check_gate_registry.sh` 的规模锚点（233）+ `--list`/注册表对拍兜住。**不改成 glob**（glob 会让 typo 变成「跑了 0 个门」更难发现） |

## 二 本轮唯一改动：579 的已知洞收口（`check_orchestrator.py`）

- 改点：`is_code_file()` 原口径「有扩展名 ∈ CODE_EXT，或无扩展名 + 首两字节 `#!`」，
  漏掉 **无扩展名且无 shebang** 的构建文件（`Makefile` / `Dockerfile` 形态）。
- 新口径：`NO_EXT_CODE_NAMES = (Makefile, makefile, GNUmakefile, Dockerfile, Containerfile)` —— **文件名白名单**。
  为什么不是「无扩展名一律算代码」：那样 `LICENSE` / `README` / `VERSION` 都会进生产者面，
  判据串只要在文档里被**引用**一次就算「在场」⇒ O1 重新失去牙（495 的形状）。
  白名单是有限的、可机器核对的集合；加名字 = 口径变更，须同步自证。
- 三态自证（新增 `surface_self_test()`，5 条断言）：
  - `S1` 造前凭空（临时仓只有 `README`）—— 防「判据恒真」；
  - `S2` 造后在场（`Makefile` 里有真生产者）—— 防「白名单没生效」；
  - `S3` 删后凭空 —— 防「缓存/目录残留冒充命中」；
  - `S4` **负控**：`README` 含同串但不是代码 ⇒ 仍凭空（防一刀切放宽）；
  - `S5` **负控**：`Dockerfile` 形态也必须认（防「只加一个名字」的半修）。
- A/B 证据：把 `is_code_file()` 里新增的两行删掉跑自证 ⇒ `S2/S5` 当场判红（8/2），
  证明确实是**这条规则**让判据变绿，而不是别的路径碰巧命中。
- 代价：本仓当前**没有**这两种形态的文件 ⇒ 扫描面文件数不变（86 个 `.sh` / 62 个源码文件），
  只补规则 + 自证，不动 `runtime/*.c`（无需重烘 14 件）。

## 三 结论与后续

- 本轮体检 9 个守卫，分档：**已完整 / 已扩面 4**（#2 #3 #5 #8）、**边界已写明、不改 4**（#1 #6 #7 #9）、
  **候选 2**（#4 `check_defect_refs` 面窄、#5 的 `packaging/` 残留）—— 候选都带**具体文件名**，不是「以后再说」。
- 本轮**代码改动只有 1 处**：`check_orchestrator.py` 的 `is_code_file()`（579 收口，见第二节）。
  其余 8 个守卫只做复核/记录 —— **体检不顺手放宽判据**（放宽 = 新漏报，M325 一节的教训）。
- 判据：一个扫描面守卫算「体检过」，必须同时给出 ①现面 ②应覆盖集 ③反例 ④结论；
  缺 ③ 的行视为未体检（本表无此行）。
- 后续（M327 候选，按代价排序）：
  1. `packaging/selftest_make_tag.sh` 的 `NEGCTL`：决定「改写 fixture」还是「带理由豁免」——**先定重烘面边界**，再决定加不加 `packaging` 到 DIRS；
  2. `check_defect_refs.py` 扩 `selfhost/*.sh` 面：需先算「现仓会新增多少判定」，避免一次把门判红；
  3. `emitc_freeze.sh` 的 glob 形状：若新增第三形状，正则与 A8 数字**同一提交**内改。
