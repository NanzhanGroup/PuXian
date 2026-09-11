# Changelog

本项目的所有重要变更都会记录在此文件。
格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，
版本语义遵循 [Semantic Versioning](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### M107-S2 · VM 全局名访问「稳定指针槽位记忆」（qg-issue 35 残余 · 每次访问 30ns → 18ns，−40%）

> 依据 `docs/M107_PLAN.md` §2.4。M105-S2 已把 `px_get_global` 的 O(g_len) 线性扫描改成 O(1) 哈希，
> 残余成本落在**每次**访问都要重算的 `strlen + FNV-1a + 开放寻址探测 + strcmp`。
> - **做法**：VM 轨 `PXOP_GETG` 传入的名字来自 `BCModule` 全局名表 `m->G[b]`——**编译期常量字符串指针，
>   进程生命周期内地址不变** ⇒ 以「名字指针」为键做一级记忆（TLS 直接映射 128 项）：首次按名查找并
>   记忆槽位号，其后 O(1) 命中，只保留 `g_globals_mu` 读锁内的取值拷贝。
> - **不失效论证**：全局表 `name→槽位` 映射**只增不改**（`px_set_global` 未命中才 `xstrdup` 追加，
>   命中只覆写 `g_vals[gi]`；表永不删除/移动/压缩）；`PxBCModule` 无堆构造/释放路径 ⇒ 记忆项恒有效，
>   同名重定义仍走同一槽位 ⇒ 语义与逐次按名查找**逐字一致**。
> - **安全边界**：该记忆**仅**对地址稳定的名字开放（新 API `px_global_resolve_stable`）。`spawn` 的运行时
>   函数名、总线 topic 键等可能传入会被 GC 回收的堆字符串，继续走 `px_get_global` 按名查找（防地址复用误命中）。
> - **不改 codegen / BC 格式 / golden**——效果在 VM 层拿到，避免动 `cg_*.px` 触发 golden 重生成与自举链。
> - **受控 A/B**（`taskset -c 3`，交错 7 轮取最好，同一 S3 runtime 只差 S2）：
>   `gl.px`（1 个全局名/迭代）A **133ms → 117ms**（B 局部 101ms 不变）⇒ 每次访问 **32ns → 16ns（−50%）**；
>   `gl3.px` 形态 A（3 个全局名/迭代）A **171ms → 150ms** ⇒ **23.7ns → 15.3ns（−35%）**；
>   命中率实证 1,048,583/9 与 4,194,317/13（≈100%）。残余 ≈8~10ns 全在读锁（量测探针对照），无锁 seqlock 列入二期。
> - ⚠️ **实测纠错**：首版槽位散列先右移对齐（`ptr>>4`/`ptr>>3`），而 `.rodata` 相邻短名只差低几位
>   （`G_b`@0x5af053 与 `G_n`@0x5af057 仅差 4 字节）⇒ 恒撞同槽、命中率 50%、`gl3` 形态 A 反退化到 224ms。
>   改「**裸指针乘黄金常数取高位**」后命中率 ≈100%。**教训：以指针为键的散列不可先右移对齐。**
> - `bootstrap/pxi` / `bootstrap/pxi_vm` 重链吸收本次 runtime/VM 变更。

### M107-S3 · 内存路径：slab 分配器复用 + 页回收收口（qg-issue 34 内存路径 · 真负载 5.6× / mmap+munmap −97.8%）

> 依据 `docs/M107_PLAN.md` §3.5，病灶由 M106-S1 的**调用者归因**钉死（`xmalloc` 采样 28.84% 中 **95.25% ← `px_str_len`**；
> `slab_reclaim_empty` 25.82% 中 **100% ← `px_gc_collect`**）+ `strace -f` 逐调用计数（真负载 mmap 59,369 / munmap 24,113，
> 其中 **99.9% 是 4KB 单页**）。
> - **S3a 位图内联**：`in_use` 位图由「独立 `slab_raw_alloc`（第二次 mmap）」改为**内联在 slab 映射尾部**
>   （`[Slab 头][槽区][位图]` 同属一次 mmap）⇒ 每 slab 建 1 次 / 拆 1 次。
> - **S3b 空槽复用不变量**：`g_slab_heads[ci]` 链语义强化为「链上恰好是该 class 中**尚有空槽**的 slab，
>   链首即分配来源」；分配取满即摘链首、释放使 slab 由「满」变「有空槽」即头插回链（均 O(1)）。
>   原实现只从链首分配 ⇒ **非头 slab 的空槽永不复用**（slab 只增不减、内存放大）。
> - **S3c 空 slab 滞留水位**（`SLAB_FREE_STREAK=4`）：连续 4 轮 GC 观测到完全空闲才 `munmap`（原每轮全量归还 ⇒ 拆了又建）。
> - **S3d 字符串合并分配**：`px_str_len` 由 **2 次 xmalloc**（`LXObject` + `data`）改为 **1 次**（`data` 内联在对象块后）。
> - **S3e 单 slab 最小 16KB**：原按「≥4 槽」定页数，小 class 只映射 1 页 ⇒「每 slab 一次 mmap」固定成本被放大。
> - **关键修复（消 O(N²)）**：`slab_reclaim_empty` 原对**每个**待归还 slab 走链摘除（O(链长)）⇒ O(归还数 × 链长)；
>   真负载采样 top PC 正落在该内层循环（**77% 采样**）。改为「过滤 + **整链重建**」（O(N)）。
> - **实测（真负载 `compiler_vm bc stdlib/yaml.px`，静默，3 轮取最好）**：real **1.554s → 0.280s（5.6×）**；
>   mmap **59,369 → 1,827**、munmap **24,113 → 37**（合计 **−97.8%**）；峰值 RSS **144.7 → 38.5 MB（−73%）**；
>   产物 **md5 逐字节一致**。旁证：分配器压力/边界用例 **2.29s → 0.48s（4.8×）**、32 例语料逐字节一致、**0 条 SLAB 告警**。
> - `bootstrap/pxi` / `bootstrap/pxi_vm` 重链吸收 M107-S3 runtime。
> - 计划、量化表与两次**实测纠错**（首版整链搜索退化 ~10s、次版单 hint 退化 ~14.5s）见 `docs/M107_PLAN.md` §3.5。

### M106 · 字符串下标原语 O(n)→摊还 O(1)（qg-issue 33-A 语言层主修 · 真实负载 1.78~1.89x）

> 依据 `/data/qg-issue/33-puxian-str-index-on-n2/ISSUE.md`（清歌新建，L0 runtime 性能缺陷）。Issue 33 的用户可见现象是
> 审批链路的 fail-closed 统一文案，但根因**不在连接**（那是 Issue 31/32）而在 **`s[i]` / `len(s)` 为 O(n)/次**：
> `px_index` 的 PX_STR 分支先全串扫一遍拿 rune 数、再从 byte0 走到第 i 个 rune ⇒ 逐字符扫字符串天然 **O(n²)**；
> ws-approve 的纯 .px `json_valid`（`/check` 调两次）在 64KB body 上放大成 11.6s，越过 2s/5s fail-closed 阈值。
> 计划、量化表与实测见 `docs/M106_PLAN.md`。
> - **S1 量化（零侵入采样剖析 + 调用者归因，插桩件不入库）**——真实负载 `compiler_vm bc stdlib/yaml.px`：
>   - 基线 **3080 样本**，`px_index` 独占 **46.43%**；补丁后 **1663 样本**，`px_index` **0.06%**，
>     且 `3080 − 1430 = 1650 ≈ 1663` ⇒ **差值恰好是这一项，零新热点**（其余符号绝对计数持平）；
>   - 采样计数比 **3080/1663 = 1.85x**，与计时实测 1.78~1.89x **互证**；
>   - 补丁后第一大户转为分配器/页回收（~87%），调用者归因（v2 采样器采 `[RBP+8]`）：
>     `xmalloc` **95.25% ← `px_str_len`**、`slab_reclaim_empty` **100% ← `px_gc_collect`**
>     （对上 libc `munmap`/`mmap` 19.4%、`strace` 系统调用时间 mmap 53.94% / munmap 43.31%）
>     ⇒ **M107 候选方向：①字符串分配抖动 ②GC 页回收风暴**（优于继续抠解释循环分派，M104 已证伪）。
> - **S2 实施**（`runtime/runtime.c` +70/−23、`runtime/runtime.h` +13/−1）：`LXObject.str` 加两个惰性缓存
>   `rune_len`（惰性 rune 计数）+ `rune_offs`/`offs_cnt`（惰性 rune→byte 偏移表，`PX_STR_OFFS_MIN=1024`，
>   仅字节长度 ≥1KB 的串建表），接入 `px_index`/`px_len`/`px_slice`——
>   覆盖两种 `for ch in s` 形态（C 轨 `cg_stmt.px` 生成 `for(i;i<px_len(x);i++) x[i]`；
>   VM 轨 `bc_emit.px` 生成 `n=len(its)` + 每元素 `INDEX`）；`px_obj_free`/`px_str_len`/`px_bytes_len`/
>   `bi_mmap`/`bi_munmap` 配平。**语义红线 9 条**逐条对齐改动前：`rune_len` 存的是原 `px_unicode_len_n` 结果
>   （**不是**建表步数，畸形 UTF-8 下二者不等）、偏移表与旧线性走查**同步进规则**、`i ≥ 表步数` **回落原走查**
>   （连越界读行为都保留）、单字符仍按前导字节判长构造、负索引/内嵌 NUL/M89-S3-C1 语义不动、
>   str 不可变 ⇒ 无失效逻辑、缓存字段为纯数据（`gc_mark_obj` 不扫 union；`LXValue` 尺寸不变 ⇒ 精确 GC 路径逐字不变）、
>   建表路径 `__atomic_*` 先数据后发布指针（竞态时他线程已建的胜出）。
>   - 实测（`taskset -c 3`，5 轮取 min CPU=user+sys；两二进制同旗标、同非 runtime 源文件（逐项 md5 核验），
>     仅 `runtime.c/h` 不同；机器静默 load≈0.86）：**基线 2.730~2.870s → 修复 1.510~1.610s = 1.78~1.89x**；
>   - 语言层微基准（对口 ISSUE §2.1/§7 判据 1）：`s[i]` 逐字符扫 64KB **4.740s → 0.030s（158x）**、
>     `while i < len(s)` 形态 **8.250s → 0.040s（206x）**；修复前规模 ×2 耗时 ×4（0.08→0.29→1.18→4.74 = 标准 O(n²)），
>     修复后**平坦**（≥1KB 建表后摊还 O(1)）；
>   - 等价性对拍：16 个输入（ASCII/CJK/emoji/CRLF/6 类畸形 UTF-8/2KB 长畸形/NUL 头/NUL 中/混合，
>     覆盖建表路径与回落路径）的 `len(s)` + **每个 `s[i]` 的字节序列** before vs after **逐字节相同**；
>     负索引（`s[-1]`/`s[-n]`）+ 3 个越界用例报错文案一致（含 `字符串索引越界: 8192`）。
>   - 代价与已知边界（如实记录）：`LXObject.str` **+8 字节/串**（`LXValue` 不变）；
>     偏移表 4B×字节、随对象回收；上界护栏见下 **S3**。
> - **S3 内存上界护栏（`PX_STR_OFFS_MAX`）**：偏移表 ≈ `sizeof(int)×(字节数+1)` ≈ 4× 串长，巨串（数百 MB）
>   反复取 `s[i]` 会 4× 放大内存。加 `PX_STR_OFFS_MAX = 16 MiB`（`#ifndef` 可 `-D` 覆盖，便于边界试验）：
>   超过上界的串**不建表**，回落 `px_index` 原线性走查（与 M105 及更早逐字节同行为），
>   仅保留 `rune_len` 惰性计数（O(1) 空间）。
>   - **对照实验**（同一输入、仅上界不同：A = 16 MiB 护栏生效 vs B = 1 GiB 护栏关闭）：
>     20 MiB ASCII / 20 MiB CJK / 1 MiB / 5 KB 四输入，`len(s)` + 13 个下标（含负索引）
>     的字节序列 **A ≡ B 逐字节相同** ⇒ 回落路径与建表路径在真实巨串上等价；
>   - **峰值 RSS（实测）**：20 MiB ASCII **43.1 MB(A) vs 123.0 MB(B) = +79.9 MB ≈ 4×20 MiB**；
>     20 MiB CJK **43.1 vs 72.4 MB = +28.9 MB ≈ 4×7.23M runes**（未触页不计入 RSS）⇒ 护栏确实拦住 4× 放大。
>   - **收口**：pxi（C 轨）/ pxi_vm（VM 轨）重链吸收含护栏 runtime；`vm_ab` v2 **38P/0GAP/0F**、
>     `diffcheck --all` **rc=0**、m89_s3d **9P/0F**、m93_s3 **6P/0F**、m96_s2 **8P/0F**、m103_s2d **rc=0**；
>     tag **`v0.2.0-m106s3`**。
> - **S5 收口（双自举 + 重链 + 全量回归）**：
>   - **双自举**：C 轨 `bootstrap_prove.sh --fresh` → B.c == `golden/compiler.c`（**15060 行**）逐字节一致；
>     BC 轨 `bootstrap_prove_bc.sh --fresh` → 重放 dump == `golden/compiler.bc.dump`（**30581 行**）逐字节一致；
>   - **`bootstrap/pxi` / `bootstrap/pxi_vm` 重链吸收 M106 runtime**（先判轨再重链）；
>   - **回归**（套件自报 rtcache = 补丁版运行时缓存目录，确认跑在新 runtime 上）：`vm_ab.sh v2`
>     **38 PASS / 0 GAP / 0 FAIL** · `diffcheck --all` **rc=0**（s01–s15 与 golden 一致；value/interp 全量通过）·
>     m89_s3d **PASS=9/FAIL=0** · m93_s3 **PASS=6/FAIL=0** · m96_s2 **PASS=8/FAIL=0** · m103_s2d **rc=0** ·
>     生态索引防漂移（`gen_ecosystem.px` + `gen_native_table.sh` → `git diff --exit-code`）**无漂移** ·
>     冒烟：C 轨/VM 轨 `hello` 产物 + `pxi`/`pxi_vm` 解释**四方逐字节一致**。
>   - tag **`v0.2.0-m106`**。


### M105 · 运行时热点路径重构（S1 量化 + S2 全局表 O(1) 名解析 + S3 GC 同步瘦身 + S5 收口）

> 依据 `docs/M104_PLAN.md` §7.3 归因（「真瓶颈在 runtime 侧的名解析与锁路径，而非解释循环分派」），
> M105 承接该结论**先量化、再动手**。计划、量化表与实测见 `docs/M105_PLAN.md`。
> - **S1 量化（零侵入，插桩件不入库）**——真实负载 `compiler_vm bc stdlib/yaml.px`：
>   - 动态指令 **5,302,670** 条，其中 `GETG` **723,980（13.7%）**、`SRCLINE` 869,191、`CALL` 312,181；
>   - `LD_PRELOAD` 计数：`strcmp` 调用 **262,956,746** 次，其中 **95.0%（249,747,716）来自全局表线性扫描**
>     （`px_get_global` 240,083,442 = 91.3% + `px_set_global` 22,601,530 = 8.6%）→
>     **平均每次 GETG 探测 331.6 项**（`g_len≈537`，平均扫过全表 62%）；
>   - 单元成本：`rwlock rdlock+unlock` **12ns**（锁不是问题）、`pthread_sigmask` 屏蔽+还原对
>     **343~372ns**（2 次系统调用）、`px_get_global` 全表均值 **2.08µs**；
>   - **数据否决原计划 S3「字段/字典名解析缓存」**（`field_while` 全程仅 18M strcmp ≈ 0.1s），
>     真实支配项是 `px_field_set`/`px_list_push`/`px_dict_set` 每操作一次的
>     `pthread_mutex_lock(g_gc_mu)` + `sigprocmask` 对（`field_while`/`loop_sum` 的 sys 时间 ≈ user 时间）。
> - **S2 全局表 O(1) 名解析**（`runtime/runtime.c`，+67/-30）：64 位名哈希开放寻址索引
>   （`GHASH_CAP = 2×GLOBAL_CAP`，负载 ≤0.5），`px_get_global`/`px_global_native`/`px_set_global`/
>   `px_method`(struct 方法) 四处线性扫描改为哈希查找（命中时 1 次 strcmp 兜底）；**锁语义、GC 根面
>   （根扫描仍线性遍历 `g_vals`）、插入点唯一性、溢出与错误文案全部不变**。
>   - 实测：`compiler_vm bc stdlib/yaml.px` **5.080s → 3.600s = 1.41x**（5 轮 min CPU，产物 dump 逐字节一致）；
>     `strcmp` 调用数 **262,956,746 → 1,047,279（-99.6%）**。
>   - 验证：`vm_ab` v2 **38P/0GAP/0F** · `diffcheck --all` **rc=0** · m89_s3d **9P/0F** · m93_s3 **6P/0F** ·
>     m96_s2 **8P/0F** · m103_s2d **rc=0** · C/VM 双轨 `hello`/`fib` 一致。
> - **S3 GC 同步瘦身 ——「软屏蔽 + 延迟暂停」**（`runtime/runtime.c`，+68/-4）：
>   `gc_block_stop/unblock_stop` 每对 = 2 次 `rt_sigprocmask`（实测 343~372ns），真实负载中 4,384,444 次
>   （S2 后第一大户）。改动：① 临界区不再真屏蔽 `SIG_GC_STOP`，改置本线程 TLS 标志 `g_gc_crit`；
>   ② 暂停信号处理器**首行**查该标志，非 0 即**延迟暂停**（立刻返回：不保存 ucontext / 不上报 paused），
>   交 executor 既有重发循环（200us + 5s 兜底）稍后重试；③ 判定用 **thread-local 层栈**
>   （`g_gcs_depth/g_gcs_skip`，仅最外层决策，内层直接返回）⇒ 嵌套重入判定唯一，防「临界区内
>   spawn → 谓词翻转」；④ 谓词 `g_active_threads == 0` 与 executor 发送闸门同判据 ⇒ 单线程下是
>   可证明的空操作，读值陈旧也**只影响性能不影响安全**（处理器只延迟暂停）；⑤ `pthread_atfork`
>   复位层栈（`#ifndef _WIN32`）；⑥ 真屏蔽分支原样保留 ⇒ **并发模式零行为变化**（实测 9 轮并发
>   GC `deferred=0`）。
>   - 实测（`taskset -c 3`，5 轮 min CPU，真实负载 `compiler_vm bc stdlib/yaml.px`，同会话交替）：
>     baseline(M104) **5.35~5.40s** · S2 **3.69~3.94s** · h2(裸跳过·不安全上限) **2.78~2.98s** ·
>     **S3 2.84~2.99s** ⇒ **S3 vs S2 = 1.30~1.33x**、**vs M104 基线 = 1.88~1.90x**，
>     且**距「无任何保护」上限仅 2.2%**（保护成本≈0）；`pthread_sigmask` 调用 **4,384,444 → 0**，
>     `strcmp`/`mutex`/`rwlock` 计数逐项不变；产物 dump 与 baseline/S2 **md5 相同**。
>   - 验证：m89_s3d **9P/0F**（并发 GC 压测×3）· m93_s3 **6P/0F** · m96_s2 **8P/0F** ·
>     m98_s2 **7P/0F** · m99_s2 **8P/0F** · m103_s2d **rc=0** · vm_ab v2 **38P/0GAP/0F** ·
>     diffcheck --all **rc=0**（全部经 rtcache/产物核对确认跑在 S3 runtime 上）；
>     **对抗性验证**：/tmp 专用变体强制并发下也走软屏蔽 ⇒ 延迟暂停真被触发
>     （deferred 0→2→24→26 / 0→10→95）且并发 GC 压测 **3/3 PASSED**（零崩零 UAF）。
> - **S5 收口（双自举 + 重链 + 全量回归）**：
>   - **双自举证明**：C 轨 `bootstrap_prove.sh --fresh` → B.c == `golden/compiler.c`（**15060 行**）逐字节一致；
>     BC 轨 `bootstrap_prove_bc.sh --fresh`（新链 `compiler_vm`，runtime = M105 S2+S3）→ 重放 dump ==
>     `golden/compiler.bc.dump`（**30581 行**）逐字节一致；
>   - **`bootstrap/pxi` / `bootstrap/pxi_vm` 重链吸收 M105 runtime**：pxi（C 轨，`fn_*`）**9,626,368 → 9,626,960 B**、
>     pxi_vm（VM 轨，`s_G/s_K/s_bc_*`）**9,460,200 → 9,460,784 B**；均 statically linked、`--version` = 0.2.0，
>     双轨 `hello` 产物输出 + pxi/pxi_vm 解释输出四方逐字节一致（全能力 `--full` 构建，模块符号集不变）；
>   - **回归**：`vm_ab.sh v2` **38 PASS / 0 GAP / 0 FAIL** · `diffcheck --all` **rc=0** · m89_s3d **9P/0F** ·
>     m93_s3 **6P/0F** · m96_s2 **8P/0F** · m103_s2d **rc=0** · 生态索引防漂移（`gen_ecosystem` +
>     `gen_native_table` 后 `git diff --exit-code`）**无漂移**；
>   - **M105 累计收益**（真实负载 `compiler_vm bc stdlib/yaml.px`）：**5.35~5.40s → 2.84~2.99s = 1.88~1.90x**
>     （S2 1.41x × S3 1.30~1.33x），距「无任何保护」理论上限仅 **2.2%**；
>   - **遗留（二期候选）**：S4「容器/字段写路径 `pthread_mutex_lock(g_gc_mu)` 粒度」复评——S3 已消掉该路径
>     sigmask 一半，`mutex_lock` 仍在（**2,932,655 次/负载**，S3 后占比需重测再定）；与 M104 §七 的
>     O5 内联缓存 / O6 计算跳转 / O8 超指令合并排序；
>   - tag **`v0.2.0-m105`**。

### M104 · VM 性能增强（LTO 构建档 + 执行引擎优化）

> 依据 `docs/M104_native_prestudy.md`（③ native 旗舰 D0 预研）**路线 ① 裁定**：native 天花板 = C 轨
> ≈1.04~1.5x VM 且成本 18~25 周，故 M104 转为**纯运行时/工具链性能增强**（不动语言语义、不动发射器、
> 不动 golden）。计划与完整实测见 `docs/M104_PLAN.md`（§七）。
> - **执行引擎优化（默认 VM 轨；`runtime/vm.c`）**
>   - **O3 类型特化快路径**（主要收益）：`ADD/SUB/MUL/DIV/IDIV/MOD/EQ/NE/LT/LE/GT/GE/BITAND/BITOR/
>     BITXOR/SHL/SHR/SHRU/NEG/BITNOT/NOT/JMPT/JMPF/INDEX` 的 **INT⊗INT** 分支在解释循环内联构造
>     `LXValue`（逐 op 与 runtime 源码同语义）；其余类型/浮点/分配/错误分支**一律回落**原 `px_*`。
>   - **O1/O2 源位置追踪镜像去重**：runtime 的 `g_px_src_func/g_px_src_line` 均为 `__thread` 且在 VM 轨
>     **仅由 vm.c 写入** → 以 `__thread` 镜像指针比较，值变才跨 TU 调用（**与 M104 前「每指令无条件
>     px_srcfunc / 每 SRCLINE 无条件 px_srcline」逐字节等价**，含 native 重入、帧弹回同行号等场景；
>     不用 push/pop 版本，避免「弹帧后 tracker 驻留 callee 行号」一类偏差）。
>   - **O4**：帧槽数组指针提到循环局部（槽数组在堆上、不随 frames 数组 realloc 移动 → native 重入亦安全）。
> - **O7 LTO 构建档**：`px build --lto` / `PX_BUILD_LTO=1`（runtime 预编译 .o + 产物 C + 链接**全链**
>   `-flto`；rtcache 键含 `lto` 位，与默认档互不污染）。**默认关**（收益负载相关，见下）。
> - **实测**（本机 8 核 16G；`taskset -c 3` + CPU 时间取 min，7 轮）
>   - VM 轨（O1–O4 全开 vs 基线 v0.2.0-m103）：`while_sum` **1.07x** / `fib26` **1.10x** /
>     `field_while` **1.06x** / `loop_sum` 1.02x / **真实负载** `compiler_vm bc stdlib/yaml.px` **1.04x**
>     （产物 dump 逐字节一致）。
>   - C 轨 LTO 档：`while_sum`（紧循环）**1.39x** / `fib26`（深递归）**0.88x**——跨 TU 内联的双向副作用，
>     故交付为可选档、默认关（`--help` 写明适用场景）。
> - **判定：性能目标未达成**（计划 §六 判据「VM 轨在 2/3 微基准上 ≥1.25x」）。实验**证伪**了「优化解释
>   循环分派可拿到 1.2~1.5x」的假设，与 D0 判据互为印证：**成本主体是动态值层，不是分派层**——
>   O1/O2/O4（循环固定开销）合计仅 ~2–4%，O3（绕开动态分派）拿走大部分收益；负载一旦进入容器/字段
>   路径（`px_field` 线性 strcmp + `px_field_set` 的 mutex/sigprocmask、`px_get_global` 的 rwlock+扫描）
>   类型特化即失效。**O5 内联缓存 / O6 计算跳转 / O8 超指令 → 二期候选**（归因与优先级入档 §七）。
> - **验证**：`vm_ab` v2 **38P/0GAP/0F** + `diffcheck --all` rc=0 + 双自举证明（C 轨 B.c 15060 行逐字节
>   一致 + BC 轨 compiler.bc.dump 重放逐字节一致）+ `pxi` 9,622,128→**9,626,368B** / `pxi_vm`
>   9,455,952→**9,460,200B** 重链吸收 M104 runtime（双轨 hello 一致、statically linked）。

### CI 修复 · fmt 门收敛（selfhost/bc_emit.px + stdlib/yaml.px）

> GitHub Actions #211（M103-S3 收口 run）toolchain job `fmt --check selfhost/*.px`
> 报 `selfhost/bc_emit.px` 格式不符。根因：两文件增量写入时绕过格式门、累积至
> #211 暴露——bc_emit.px（M89-S3-C2 并入 compiler.px 后从未收敛，老账）+
> stdlib/yaml.px（M103-S2c 新增 yaml_stringify 段）。
> - **修复**：bootstrap/pxfmt -w 规范化两文件。`git diff -w` 为空 → 纯空行/行内
>   尾注释对齐变化，**零代码语义变化**（bc_emit.px -55 行空行 -14 行尾注释对齐、
>   yaml.px -11 行空行）。
> - **基准同步**：编译器源码行号平移 → 产物调试行号字段同步，双 golden 更新
>   （compiler.c 1453 处 px_srcline 值、compiler.bc.dump 1453 处 SRCLINE 值，
>   非行号差异 0 处）+ 重链 bootstrap/pxc_vm（源码↔二进制对应）。
> - **生态索引同步**（#211 第二处红，fmt 步骤过后才暴露）：M103-S2c 扩展
>   stdlib/yaml.px 后未重跑 `tools/gen_ecosystem.px` → docs/ecosystem_index.json
>   的 yaml `lines` 仍为 404（滞后）→ 重生成同步为 592；native_index.json 无漂移。
> - **验证**：C 轨自举 rc=0 + BC 轨自举全链重建 rc=0 + vm_ab v2 **38P/0GAP/0F** +
>   diffcheck --all rc=0 + m66_yaml 35P/0F + m103_s2c 46P/0F + pxlint
>   selfhost/compiler.px 0 错 0 警 + fmt --check 全收敛域（selfhost+tools+stdlib）绿。
> - **pxc_vm 保持入库版**：本次仅源码行号平移（无功能变化），VM 编译器产物语义
>   零影响（vm_ab/diffcheck 全绿）；重链方式与 M92 常量级不同，不做不可控变更。
> - **防复发**：改动/新增 .px 先 `px fmt -w` 再入库；收口流程前置 fmt 收敛域检查
>   + 生态/native 索引重生成（CI toolchain job 两道门）。

### M103 · 清歌 Issue 29/30 语言层缺口收官（ws-ddns / api-server PuXian 化阻塞清零）

> M103 = 清歌 Issue 29（29-puxian-ddns-gaps：dns TXT + ed25519_keygen）+ Issue 30
> （30-puxian-api-server-gaps：YAML 序列化 + 图片 JPEG）全量收官——qg-issue 29/30
> 归档 done，ws-ddns .px 纯 native 移植与 api-server 头像/配置写回缺口清零。
> 规划 docs/M103_PLAN.md。性质：**L0 runtime（native 301 → 306（+5））+ L1 stdlib**。
> 基线 v0.2.0-m102（8377953）。commit 链：01eed4d（S1 立项 + D0 侦察）+ 9216c71
> （S2a）+ ae707ff（S2b）+ b6d262e（S2c）+ 996f4b4（S2d）+ 本收口（S3）。
> - **S2a · dns_txt(domain) → list[str]**（Issue 29 GAP-DNS-TXT-1，native 302）：
>   getaddrinfo 只 A/AAAA 结构上不可能返回 TXT；libc res_query 在 musl（交叉
>   aarch64/armv7/riscv64）仅 A/AAAA 桩、mingw 无 → **手写 DNS UDP wire 查询**
>   （resolv.conf nameserver → QTYPE=16 报文 → SO_RCVTIMEO 3s → 解析响应，label/
>   压缩指针 dns_skip_name、多段 character-string 合并、TC 截断/格式错 Err 可判定）。
>   语义对齐 Go net.LookupTXT：**无 TXT/NXDOMAIN → 空 list（非报错）**——授权 TXT
>   可能未配置，调用方按无记录处理。examples/m103_s2a 9P/0F（qq.com SPF + .invalid/
>   localhost 空 list + 空域/坏 label Err + dns_lookup A/AAAA 零回归）+ Go
>   net.LookupTXT 对拍 CMP-EQUAL + m84_s3_dns 回归全绿。
> - **S2b · ed25519_keygen() → dict{pk_hex,sk_hex,pk_pem,sk_pem}**（Issue 29
>   GAP-ED25519-2，native 303）：tweetnacl randombytes seed32 → crypto_sign_seed_keypair
>   展开 sk64=seed||pub；PKCS8/SPKI DER（RFC 8410，SPKI 44B=30 2A 内容 42、PKCS8 48B
>   嵌套 OCTET 04 22 04 20）**与 Go crypto/ed25519 + x509 逐字节互通**；e_b64enc/
>   e_pem 行 64 PEM；runtime.h 声明 + PX_NO_ED25519 注册 + native_mod_map。修 S2b
>   SPKI 数组 42→44 越界（2B 栈写 + 编码丢尾）bug。examples/m103_s2b 14P/0F + Go
>   字节级互通三断言 PASS（SPKI pub==pk_hex / PKCS8 seed 派生 pub==pk_hex / Go 验 px
>   签）+ m83_s3_ed25519 sign/verify 零回归。
> - **S2c · std.yaml 补 yaml_stringify(value) → str**（Issue 30 GAP-YAML-SER，
>   stdlib/yaml.px 纯 .px 扩展，native 不变）：M66-S2 parse 的写回对称——api-server
>   models.go/config.go 写 token-cache-llm.yaml / references_*.yaml「反序列化→改字段→
>   序列化→落盘」闭环。yl_needs_quote 歧义判定（空/数字/null·bool 字面量/含 : # " '
>   换行制表反斜杠/首字符 YAML 特殊符 → 双引号转义，与 parse yl_unescape_dq 集对齐）、
>   嵌套缩进 2、seq-map 首键 "- " 行内 + child_col 对齐 parse yl_parse_seq、空容器
>   {} / []（Go/标准语义；px parse 流式集合不支持记录）。examples/m103_s2c 46P/0F
>   （api-server 形状 roundtrip deep-equal + 特殊字符串矩阵 27 项 + 标量保型 int 大数/
>   float + Go yaml.v3 写→px 读互认）+ Go yaml.v3 读 px 产出互认 + m66_yaml 现有
>   parse 双模式 35P 零回归。
> - **S2d · img_decode / img_scale / img_encode_jpeg**（Issue 30 GAP-IMG，native
>   306）：stb_image v2.30 + stb_image_write v1.16（public domain，sha256 归档）入库
>   runtime/third_party/stb/；runtime_image.c 单 TU（STB_IMAGE_IMPLEMENTATION，裁
>   PIC/PNM/HDR/TGA/PSD/PVR/PKM 留 png/jpeg/webp）；img_decode(data:bytes|str)→
>   {w,h,pixels:bytes RGBA w*h*4}（强制 4 通道 + 1 亿像素上限）；img_scale 双线性
>   中心对齐等比缩放（≤512，avatar 语义）；img_encode_jpeg RGBA→RGB +
>   stbi_write_jpg_to_func 内存缓冲（调用局部 JpgCtx 无全局态线程安全，q 1-100 clamp）。
>   独立 **img 裁剪模块 --no-img**（mod_srcs/rt_src_files/5 处 for 循环/mod_macro
>   PX_NO_IMG/runtime.c 注册/native_mod_map，--full img 增量 ~100KB）。examples/
>   m103_s2d 21P/0F（scene.png 640×480 decode + 等比 512×384 + JPEG q70 魔数 FFD8/
>   FFD9 + decode back + Go jpeg 输入 decode 200×150 + 小图不缩放 + 畸形 Err）+
>   Go image.Decode 验 px JPEG 合法 512×384 + 体积 13209B vs Go ref 13177B 同量级。
> - **收口（S3）**：native_index 306 + CHEATSHEET 306 同步；回归总闸 vm_ab 38P/
>   0GAP/0F + diffcheck --all rc=0 + 双自举证明（C 轨 B.c 15060 行一致 + BC 轨
>   compiler.bc.dump 逐字节）+ pxi/pxi_vm 重链（吸收 M103 runtime）+ ROADMAP M83–M103
>   （连续 20 里程碑）+ qg-issue 29/30 归档 done + CHANGELOG/M103_PLAN 完成
>   + tag **v0.2.0-m103**。
> - 意义：ws-ddns（Issue 29）local -genkey / server 授权 TXT 校验与 api-server
>   （Issue 30）头像 JPEG / YAML 配置写回四个语言层缺口全部 native/stdlib 落地，
>   两模块纯 PuXian 移植阻塞清零；二期候选续：URL 直达 .px 管道 defer / h2 生产化 /
>   px_serve 事件化连接剩余面 / 远景 ③ native 机器码旗舰。
### M102 · .px 子进程池协程化（语言层 px_exec offload）+ h2/URL 侦察收口

> M102 = 二期候选 B（.px 子进程池协程化）+ C（h2 handler 协程化）侦察驱动里程碑
> （候选排序第 4/5 项）：**先 D0 侦察、成立则实施、不成立就地关闭记录**。commit 链：
> 7edb71f（S1 立项 + D0 侦察定稿）+ 3b02cf9（S2a D1 实施）+ a6e7993（S2b 收口决策）。
> 规划 docs/M102_PLAN.md。性质：**L0 runtime**（语言层 .px 执行并发模型）。
> - **D0 侦察结论**：
>   - **候选 C（h2 handler 协程化）→ ❌ 关闭记录**：runtime_h2.c 为最小 h2c 帧循环
>     （M35：h2c Upgrade + prior knowledge + HPACK + 固定 echo 响应），**不调用 VM
>     handler、不经 px_http_dispatch 公共管道** → 无"handler"可协程化；且 M-B9b 已
>     ALPN 固定 http/1.1（vhost handler 仅 http/1.1 生效，h2 帧循环绕过 vhost 生产
>     不可用）→ h2 生产化（含 handler 管道接入）需完整 h2 服务端另立大里程碑。
>   - **候选 B（.px 子进程池协程化）→ ✅ 成立**：.px 执行 = M25 常驻 `px --worker`
>     子进程池（PHP-FPM 风格）；父进程 px_pool_recv_result 用 poll+read 同步阻塞等
%   - 实现范围收敛：D1（语言层 px_exec offload）实施；D2（URL 直达 .px 请求管道
%     defer）评估为需「offload executor C 闭包任务 + PxPend kind 扩展 + 管道拆段」
%     ≈独立里程碑 → 关闭记录二期；h2 关闭记录。
> - **D1（语言层 px_exec offload）✅**：`px_exec`（bi_px_exec → px_pool_run 阻塞等
>   px --worker 结果帧）纳入 `px_native_offload_kind` 名单 —— 协程 ctx（route/vhost/
>   middleware VM handler、spawn 帧协程）内调 px_exec 自动外包执行线程池（M96 β
>   路线），阻塞等子进程在外包线程，worker 释放取下一协程。px_exec 内仅 native 直调
>   （json_stringify 等）不回调用户 VM → 满足 D5 名单约束；px_pool 全局池 g_px_pool_mu
>   保护，外包线程并发安全（与多连接线程直调并发语义一致）。逃生舱（主线程/非协程
>   ctx）→ 直调零变化。
> - **验证（examples/m102_s2 verify 5P/0F）**：PX_CORO_WORKERS=1 极限 —— 单次
>   px_exec（sleep400）once≈500ms；3 协程并发 px_exec conc≈505ms（≈单次，外包并行
>   铁证；若同步阻塞卡 1 coro worker 需 ~3×≈1.5s）+ 结果 3/3 对拍 slow-ok。回归：
>   m96_s2 8P/0F（offload 名单扩展不破坏）+ m32_hot_reload（px_exec 语义）+ m96_s3
>   9P + m93_s2/s3 + m94_s2/s3 + m95_s2/s4 + m97_s2/s3 + m98_s2 + m99_s2 + m100 +
>   m101_s2 + m89_s3d 全绿；vm_ab 38P/0GAP/0F + diffcheck --all rc=0 + 双自举证明
>   （C 轨 + BC 轨）+ pxi/pxi_vm 重链（吸收 M102 runtime，双轨 hello 一致）
>   + tag v0.2.0-m102。
> - 二期候选续：URL 直达 .px 管道 defer（offload C 闭包执行器 + PxPend kind 扩展）；
%   h2 生产化接入公共管道；清歌 Issue 29/30（dns_txt/ed25519_keygen/图片/yaml 写）。

### M101 · px_serve 并发 TLS 握手缺陷修复（qg 二期候选 A）

> M101 = 二期候选排序第 A 项（已复现真实缺陷：M99-S3 记档「px_serve 并发 TLS 握手
> TLS1.3 CertificateVerify 签名错 + TLS1.2 大并发 EOF，M98 runtime 复现，先于 M99」）。
> 规划 docs/M101_PLAN.md。性质：**L0 runtime bugfix**（服务端 TLS 多 worker 并发握手
> 正确性——mbedtls 3.6.2 预编译库未编线程支持下的共享竞态根治）。
> - **根因（D0 侦察 + 复现二分）**：px_serve g_pool 多 worker 并发在各自连接上执行
>   mbedtls 服务端握手，而 runtime/mbedtls 3.6.2 库 **MBEDTLS_THREADING_C 关（config
>   2100/2111/3630 全注释）→ 库内无任何互斥**；握手中跨线程**共享可变对象**：全局
>   私钥 g_srv_key（RSA CRT 签名写 ctx）/ SNI key / 全局 session cache g_srv_tls_cache
>   （无锁链表）。examples/m101_s2 复现（v0.2.0-m100 runtime，barrier 48 并发新建 TLS
>   连接全握手）：**RSA-TLS1.3 ok=2 fail=142**（'invalid signature by the server
>   certificate: crypto/rsa: verification error' = RSA 私钥并发签名竞争实锤）、
>   **RSA-TLS1.2 144 全 EOF**、EC(P-256)-TLS1.3 ok=140 fail=4（EOF/reset = session
>   cache 无锁竞争）。修复前**每次请求都新建连接全握手**（Connection: close）。
> - **修复（S2，消除握手中跨线程共享可变写，不做全局性能新瓶颈）**：
>   · **per-连接 RSA 私钥 clone**（px_pk_clone_rsa：mbedtls 3.6.2 无 mbedtls_pk_copy，
>     legacy PK 启用（USE_PSA_CRYPTO 关）→ pk_info_from_type(PK_RSA)+pk_setup+
>     mbedtls_rsa_copy 深拷贝出独立 rsa ctx）→ c->own_pk（默认）/ c->own_pk_sni（SNI
>     命中，px_sni_cb p_ctx=NULL→PxConn* 后锁内 clone）→ 签名写各自 ctx（主因根治）。
>   · **session cache 加锁包装**（px_srv_cache_get/set 自定义 get/set 包 g_srv_cache_mu，
>     3.6.2 cache 回调 4 参签名 (void*,id,len,session)）——次因根治（EC 残余 4 失败归因）。
>   · **全局握手串行锁** g_srv_hs_mu 包整个 px_conn_tls_handshake：clone+cache 锁后
>     RSA-TLS1.3 仍残余 ~1% MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED(-110) + 偶发堆损坏
>     （串行 300 次零失败 = 并发特有；TLS1.3 服务端存在无法枚举的深层共享）→ 串行锁
>     根治（锁序 hs_mu→tls_mu 无死锁；keep-alive 连接握手仅一次不受影响；clone+cache
>     锁保留双保险——未来换开 threading 的 mbedtls 可去掉串行锁仍正确）。
>   · **px_conn_close 泄漏修复**：释放条件 is_tls → owned && ssl（握手失败路径 is_tls
>     未置 1 原实现跳过释放 ssl/conf/drbg/entropy/own_pk → 每失败连接泄漏，并发失败
>     高频时严重）；close_notify 仅握手完成后发。
> - **验证（S2 全绿 + 收口全绿）**：examples/m101_s2 verify.sh 三轮并发全 0 失败
>   （RSA-TLS1.3 / RSA-TLS1.2 / EC-TLS1.3 各 144/144；修复前 RSA-TLS1.3 142 失败、
>   TLS1.2 144 EOF）+ 独立 6 轮×144=864 连发 0 失败 0 崩溃 + SNI 单连（CN=localhost /
>   CN=sni.local 证书正确切换）+ SNI 并发 30/30 + m99_s2 verify_tls 8P/0F（M99 TLS
>   场景不受损）+ 回归 suites（m82/m83_s6/m89_s3d/m93_s2/s3/m94_s2/s3/m95_s2/s4/
>   m96_s2/s3/m97_s2/s3/m98_s2/m99_s2/m100）全绿 + vm_ab 38P/0GAP/0F + diffcheck
>   --all rc=0 + 双自举证明 + pxi/pxi_vm 重链 + tag v0.2.0-m101。

### M100 · middleware 链协程化（px_serve route 管道内链状态机 defer）

> M100 = 二期候选第 2 项（middleware 链协程化）正式立项（M98_PLAN §二 ⚠️ 遗留清单第 4 项；
> M99 收口后二期排序第 2 项）。commit 链：2c202ec（S1 立项 + D0 侦察 + 设计定稿）+
> 6d65d23（S2 实现 + examples/m100 验证套件 11P/0F）+ 本 commit（S3 收口 + tag v0.2.0-m100）。
> 规划 docs/M100_PLAN.md。上游：M98（px_serve route/vhost handler 拆段协程化，
> PxPend 注册表 + px_pxserve_defer + 段2 续处理骨架直接复用）；M99（连接级事件化 IDLE）。
> 性质：**L0 runtime**（px_serve route 管道 middleware 链并发模型层）。
> - **背景**：M98 只拆了「middleware 链之后的 route handler」——链本身仍在 g_pool worker
>   内同步逐个 px_call（runtime_route.c px_route_try_dispatch for 循环）。链上任一 VM
>   middleware 内 chan/sleep 长业务（与 handler 同构）→ 阻塞 worker；多慢 middleware
>   请求把 max_conn 池占满 → 快路径饿死。middleware 链与 handler 不同：多段串联 + 每段
>   返回值决定链去向（null 继续 / 非 null 短路 / 全 null 进 handler）→ 不能平移单点 defer。
> - **S2（链状态机 defer 实现）**：
>   · **PxPend kind 扩 2/3** + mw 字段：2 = middleware 链 defer 运行中（链状态机）、
>     3 = middleware 短路完成（段2）；mw_i/mw_n/mw_chain[32]（链快照）/mw_handler/
>     mw_params 入 GC 根（px_pxserve_pend_gc_mark 补标，逐段 spawn 函数值跨协程保活）。
>   · **px_pxserve_mw_defer**（runtime.c）：登记 kind=2 + 链快照（持调用方 mws 快照拷入，
>     不依赖 g_middlewares 运行期一致性）+ handler/params 入根 → spawn 首段（middleware[0]）。
>   · **px_serve_mw_done**（链推进回调，coro worker）：段为 middleware（mw_i<mw_n）→
>     ret==null 推进下一段（mw_i++，仍 <mw_n → spawn mw_chain[mw_i] 继续链；==mw_n →
>     链全 null 通过 → spawn handler 段 [req, params]）；ret!=null → 短路（resp + kind=3 +
>     stage=2 → 投回 g_pool 段2）；段为 handler（mw_i==mw_n）→ resp + kind=0（= route
>     handler 完成语义 px_route_respond）。spawn 出锁后做（锁序 g_pxpend_mu 不嵌套）。
>   · **px_route_try_dispatch 分流**（runtime_route.c）：链非空且每段 middleware + handler
>     全 VM（PX_FUNC 且 fn==px_vm_entry）且 async_ok → 链 defer（return 2，worker 释放）；
>     含 C 闭包 middleware 段 / 非 VM handler / async_ok=0 / 无协程内核 → **原同步链零变化**
>     （M98 行为逐字节一致）。
>   · **短路 respond 公共化**：px_route_mw_short_respond（normalize + respond +
>     (middleware) 访问日志）——同步短路与协程段2（kind=3）共用，文案逐字一致。
>   · **px_conn_worker 段2** 加 skind==3 分支（middleware 短路续处理）。
> - **验证（examples/m100 11P/0F + 回归 26 套）**：并发 20×/slow-mw（middleware sleep 600ms
>   让出）wall=0.60s（同步占 2 worker 串行需 ~6s+——核心铁证）+ 慢 middleware 在途
>   30×/fast 不饿死（0.01s）+ 短路 401/403 + 多段链 null 推进 + keep-alive 5 请求 +
>   线程峰值 12≤20 + (middleware) 日志 + 优雅关闭干净退出；px_serve 管道回归
>   （m28_route/m29_webprod/m31_vhost/m33_route_rate_limit/m57_s7_vhost_headers/
>   m43_webapp（10P/0F STDOUT==GOLDEN，含 middleware.px 短路）/m98_s2/m99_s2）+ 协程内核
>   suites（m82/m83_s6/m89_s3d/m93_s2/s3/m94_s2/s3/m95_s2/s4/m96_s2/s3/m97_s2/s3）
>   全绿；vm_ab + diffcheck --all + 双自举证明 + pxi/pxi_vm 重链，tag v0.2.0-m100。
> - **D0 记录**：库内 middleware 全为 def VM 函数（纯计算记录/校验/短路），无长业务真实
>   用例 → 本里程碑 = 能力补齐（与 handler 同级让出），消除「middleware 写长业务占死
>   worker」架构缺口；含 C 闭包链保持同步（边界，语义与 M98 一致）。

### M99 · px_serve 连接级事件化 IDLE（keep-alive 空闲不占 g_pool worker）

> M99 = 原 M95 二期候选第 1 项（px_serve 连接级事件化 IDLE）正式立项（M98 收口后，
> qg-issue 29/30/31/32 等已闭环或排期）。commit 链：68a589b（S1 立项 + D0 侦察 +
> 方案）+ 09caa6a（S2 实现 + examples/m99_s2 验证门）+ 965bbd0（S3 回归面）+ 本
> commit（S4 收口 + tag v0.2.0-m99）。规划 docs/M99_PLAN.md。
> - **背景**：M98 已把 px_serve 的 route/vhost VM handler 拆段协程化（长业务占协程不占
>   线程），但 **keep-alive 空闲连接仍每连接一个 g_pool 线程阻塞在下一请求 recv 直到
>   连接关闭**（SO_RCVTIMEO 15s）→ max_conn 预派生池被空闲连接占死，超出 worker 数的
>   并发连接悬挂排队。M95 已给 http_serve 系做连接级事件化（IDLE 交事件循环照看），
>   px_serve 缺同款 → 并发模型未到终点。
> - **S2（连接级事件化 IDLE 实现）**：px_serve 连接接入既有全局事件循环内核
>   （px_evc/px_ev_loop，http/sse 同源）：
>   · FSERVE_KIND_PXSERVE=2 新连接类型（事件循环 detect 可读 → 投回 px_serve 的 g_pool
>     （px_pool_push）；tick 15s 空闲超时/对端断开 close 均走 px_pxpend_close —— 清
>     PxConn/TLS 会话/inflight 计数，不可裸 close）。
>   · 交 IDLE 变体 px_evc_idle_put_fd(nonblock=0)：**fd 保持阻塞**（px_serve 读侧走
>     SO_RCVTIMEO recv + TLS mbedtls，切非阻塞会 recv EAGAIN 误判断开/引入 TLS WANT；
>     epoll 对阻塞 fd 照常报可读，投回时数据在途 → recv 立即返回零 EAGAIN）；
>     http_serve 用 wrapper(nonblock=1) 行为零变化。
>   · 在途判定 px_pxserve_inflight_data：TLS 连接含 **PxConn rbuf 缓冲残留探测**
>     （mbedtls_ssl_read 一次读整 record 未消费完 roff<rlen → 算在途；否则交 IDLE 后
>     缓冲中下一请求对 epoll 不可见 → 永不处理悬挂至 15s 超时）；明文 poll fd。
>   · px_conn_worker 每 job px_evc_acquire 登记 ACTIVE + req_done/段2 响应尾
>     px_pxserve_idle_after_resp 交 IDLE 释放 worker；px_pxpend_close 前置 px_evc_detach
>     （防事件循环照看已关 fd / fd 复用串扰）。
>   · 优雅关闭补 px_pxserve_ev_close_all（accept 退出 + 池 join 后清全部 PXSERVE 登记
>     连接 → g_px_inflight 归零，干净退出，不再等 15s tick/5s 兜底）。
> - **验证（全绿）**：examples/m99_s2 verify.sh（明文）8P/0F —— **max_conn=4 并发 40
>   keep-alive ×/fast 全成功 wall=0.01s**（事件化前 4 worker 被空闲占死 → 36 悬挂，核心
>   铁证）+ 线程峰值 14≤20（40 空闲连接 0 占 worker）+ 并发 40×/slow（sleep300 让出 +
>   IDLE）0.30s + 空闲 16.5s 后 8/8 连接被 15s tick 回收 + 优雅关闭 0.1s 在途 0；
>   verify_tls.sh（TLS）8P/0F —— 顺序建 20 TLS keep-alive 全成功（0.97s，max_conn=2）
>   + 线程峰值 12≤18 + TLS 空闲后 20 conns 并发续请求 20/20 wall=0.00s（IDLE 唤醒 +
   mbedtls rbuf 探测续服务不悬挂）+ 优雅关闭干净。
> - **收口回归**：diffcheck --all rc=0 + vm_ab 37P/1GAP(环境)/0F + 双自举证明（C 轨
>   bootstrap_prove rc=0 + BC 轨 compiler_vm 重放 dump 与 golden 逐字节一致）+ 里程碑
>   suites m95_s2/s4、m97_s2/s3、m82、m83_s6、M98_s2 全绿（px_evc 事件循环改动面：
>   http/sse 交 IDLE/派发路径零回归）。
> - **重链**：bootstrap/pxi_vm（VM 轨，9,342,968 → 9,343,016B）与 bootstrap/pxi
>   （C 轨，9,513,200 → 9,513,256B）--full 吸收 M99 runtime（FSERVE_KIND_PXSERVE /
>   px_pxserve_idle_after_resp / px_pxserve_ev_close_all / px_evc_idle_put_fd 链入）；
>   双轨 hello stdout 逐字节一致。compiler_new/vm 未重链（M99 runtime 改动在 px_serve
>   连接网络层，compiler 执行路径不触；--fresh 可重建）。
> - **⚠️ 发现既有缺陷（先于 M99，M98 runtime 复现，qg 二期候选）**：px_serve **并发 TLS
>   握手**缺陷 —— TLS1.3 下 CertificateVerify「crypto/rsa: verification error」（Go
>   客户端 InsecureSkipVerify 不豁免 CertificateVerify 验签）+ TLS1.2 下大并发部分 EOF。
>   用 M98 runtime 复现同失败（更严重）→ 确证非 M99 引入（M99 不触握手路径）。M99 TLS
>   验证据此顺序建连聚焦「已建 TLS 连接空闲事件化」；并发握手健壮性另立里程碑（疑似
>   mbedtls 全局 session cache 并发竞态 / RSA-PSS CertificateVerify 边界）。另记既有
>   限制：px_serve HTTP/1.1 pipelining 同缓冲残余无 pbuf 续接（http_serve 有）→ 二期。

### M98 · px_serve route/vhost handler 协程化（D8-② 收官）

> M98 = 原 M95-S5（px_serve route/vhost handler 协程化）正式立项（M97 期间 qg-issue
> 31/32 插入顺延编号）。commit 链：1d3168b（S1 立项 + D0 复核 + 范围决策）+ fdc2160
> （S2a route 拆段）+ 448c0d3（S2b vhost 拆段 + null 回退）+ 本 commit（S3 收口 +
> tag v0.2.0-m98）。规划 docs/M98_PLAN.md；早期侦察快照 docs/M95_S5_PLAN.md。
> - **背景**：px_serve（M31.4b g_pool）每连接一个常驻线程跑 px_conn_worker 阻塞
>   keep-alive 到连接关闭（PxConn 为栈对象）→ route/vhost VM handler 长业务
>   （sleep/chan/spawn）占死 worker → 慢请求数 ≥ worker 数即饿死全系统。D0 复核
>   确认不可直接平移 http/sse 的 http_pend（px_serve handler 调用点深埋
>   px_http_dispatch + PxConn 栈对象跨 worker 释放即丢）→ 需连接堆化注册表 +
>   管道拆段内核，独立里程碑量级。
> - **S2a（连接堆化注册表 + route handler 拆段内核）**：PxPend 连接注册表
>   （fd→堆 PxConn*，TLS 会话/读缓冲跨 g_pool worker 存活；独立锁 g_pxpend_mu +
>   SIG_GC_STOP 屏蔽 + fd 复用 active=0 防串扰）；px_conn_worker 连接槽取/建
>   （新建锁外握手）+ 续处理入口 + DEFER 释放 worker；px_http_dispatch 返回码化
>   （0=同步完成/1=已拆段）+ async_ok 开关（H3/其余传 0 → 原同步路径逐字节零变化）；
>   route 命中 VM handler → px_pxserve_defer（stage1 + req 入 GC 根 +
>   px_coro_spawn_ex(handler,[req,params],done,fd)）→ worker 释放；done（coro
>   worker）stage2 + px_pool_push 投回；段2 px_route_respond（async/sync 响应语义
>   逐字节一致）+ 访问日志；GC 标记期 px_pxserve_pend_gc_mark 补标挂起 req/resp
>   （precise 必须，漏标=UAF）。
> - **S2b（vhost VM handler 拆段 + null 回退续管道）**：px_pxserve_defer 泛化
>   （hargs/nargs + kind=0 route/1 vhost + vroot）；px_http_dispatch 增 skip_pre：
>   CORS/限流/vhost 段1 包 `if(!skip_pre)`（vhost null 回退续管道重入不双计）；
>   vhost handler 命中 VM → defer(kind=1) 拆段；段2 kind 分派 —— route →
>   px_route_respond；vhost resp 非 null → px_vhost_respond（公共函数，M57-S7 白名单
>   响应头透传，无访问日志=vhost 历史语义）；resp null → store vroot + 重入
>   px_http_dispatch(skip_pre=1) 续 route+静态/.px（续管道内 route 再拆段 → 再释放
>   等二次投回）。未命中 VM / H3 / 原生 handler → 同步路径零变化。
> - **验证（全绿）**：examples/m98_s2 verify.sh —— 并发 20×/slow（max_conn=2）
>   wall≈1.2s（同步占线程≈12s）+ fast 30 不饿死（wall≈40ms）+ keep-alive 顺序
>   2×/slow（dials=1 续处理）+ /p/:id 路径参数跨 defer + /big 1.5MB body tmp defer
>   期可读 + /alloc ×10 precise GC 挂起表根 + **vhost 并发 10×/vh-slow
>   （wall=807ms，同步≈4s）+ vhost null 回退 3×/index.txt（dials=1）+ Content-Type
>   透传（段2 normalize）** + 访问日志对拍 + 线程峰值 11≤18。
> - **收口回归**：px_serve 核心套件 m28_route / m29_webprod / m31_vhost /
>   m33_route_rate_limit / m57_s7_vhost_headers + m53_s4 HTTP/1.1 共享管道全绿
>   （H3/QUIC 部分依赖 ngtcp2+aioquic 外部库，本环境缺 → HTTP/1.1 管道回归覆盖）；
>   里程碑 suites m82/m83_s6/m89_s3d/m93_s2/s3/m94_s2/s3/m95_s2/s4/m96_s2/s3/
>   m97_s2（重跑 3/0）/m97_s3 全绿 + vm_ab 38P/0GAP/0F + diffcheck --all rc=0 +
>   双自举证明（BCModule dump 30582 行 + B.c 15060 行均与 golden 逐字节一致）。
> - **重链**：bootstrap/pxi（C 轨解释器，9,504,592 → 9,513,200B）与 bootstrap/pxi_vm
>   （VM 轨，9,334,360 → 9,342,968B）吸收 M98 runtime（px_pxserve_defer/px_vhost_respond/
>   px_pxserve_pend_gc_mark 链入）；双轨 hello stdout 逐字节一致。compiler_new/vm 未
>   重链（M98 runtime 改动在 px_serve 网络层，compiler 执行路径不触网络；--fresh 可重建）。
> - **二期候选（另立里程碑）**：px_serve 连接级事件化 IDLE（keep-alive 空闲不占
>   g_pool 线程）、h2 连接 handler 协程化、.px 子进程池协程化、middleware 链协程化、
>   px_serve CORS/限流 middleware 链拆段。

### M97 · 连接复用生命周期缺陷修复（qg-issue 31 客户端 + 32 服务端）

> M97 = 清歌（qingge）新 issue 31/32 立项；原 M97（px_serve route/vhost handler
> 协程化，M95_S5_PLAN.md）**顺延为 M98**。commit 链：b6587a3（S1 立项 + D0 侦察 +
> 设计定稿）+ 本 commit（S2 F31 + S3 F32 + S4 收口 + tag v0.2.0-m97）。
> - **Issue 31（客户端 http_request 连接池）实锤三处**：h_exchange 响应行版本被
>   `sscanf(buf,"HTTP/%*s %d")` 直接跳过 → keep_alive 判定只看 `Connection: close`
>   不判协议版本 → HTTP/1.0 上游（无 Connection 头 + Content-Length、发完即关）的
>   死连接被误判可复用回池；默认 retries=1 → 池连接复用失败路径无自动新建重连 →
>   **奇偶失败**（Mahesvara 反代 1/3/5 成、2/4/6 败实测吻合）。
> - **F31 修复**：h_exchange 解析响应协议版本 → keep_alive **协议感知**（RFC 7230
>   §6.3：HTTP/1.1 默认复用 / HTTP/1.0 默认关、仅显式 `Connection: keep-alive`
>   复用）；bi_http_request 池死连接复用失败 → 丢弃并**自动新建重发一次**（不消耗
>   attempt，首次 IO 失败 = 连接已死无半响应污染，重发安全）。验证：HTTP/1.0 模拟
>   上游连续 30 次请求零失败且 accept==30（不回池）；旧 runtime（pxi 对拍）精确
>   复现 ok=15/30 奇偶失败 → bug 实锤 + 修复双闭合。
> - **Issue 32（服务端 http_serve_unix keep-alive 长连）实锤三处**：响应写为单次裸
>   send 不检查返回值（非阻塞 fd EAGAIN/部分写 → 响应截断）；事件循环 IDLE 只注册
>   EPOLLIN|EPOLLRDHUP|EPOLLET 不等 POLLOUT → 部分写交还 IDLE 后剩余响应**永不写出**
>   → 悬挂至 15s 空闲超时 close（token-cache 网关 1s fail-closed 误报根因）；
>   M95-S2 handler 挂起期连接 ACTIVE 不照看不交还（记录项）。
> - **F32 修复**：新增 `px_send_all` 非阻塞全量写（循环 send，EAGAIN/EWOULDBLOCK →
>   poll(POLLOUT, 15s) 续写，EINTR 重试，EPIPE/ERR/超时 → -1）；http_send_resp
>   普通响应 + file 流式 + 404 与 http_conn_worker 413 全部替换；响应字节**全量入
>   内核后才交还 IDLE**；写失败 → px_evc_close 收尾不留半写连接。验证：Go
>   http.Transport keep-alive 单连接复用（conns==1）100 请求（/big 2MB 大响应与
>   /ok 交替）100/100 零悬挂零丢零截断（wall 0.72s）+ 服务端 audit==100 无丢对拍闭合。
> - **回归**：新增 examples/m97_s2（4 PASS）+ m97_s3（6 PASS）+ m89_s3d 9 + m93_s2/s3
>   12 + m94_s2/s3 8 + m95_s2 14 + m95_s4 10 + m96_s2 8 + m96_s3 9 + m82/m83_s6 +
>   vm_ab 38P/0GAP/0F + diffcheck --all ✅ + 双自举证明 + pxi/pxi_vm 重链吸收 M97
>   runtime + tag v0.2.0-m97。

### M96 · 客户端网络 IO 协程化（D8-①：阻塞 native offload 执行器，慢上游不卡 worker）

> M96 = M93_PLAN §D8 ①（http_request/tcp/udp/ws/s3/dns 同步阻塞桥 → 协程上下文感知）
> + M94_PLAN §五 编排（M96 = D8-① 客户端网络 IO 协程化）。
> commit 链：84bff6d（S1 立项定稿 + 路线拍板）+ 8a8add4（S2 执行器核心）+ d3802de
> （S3 名单全集 + 错误回传重构）+ 本 commit（S4 收口 + tag v0.2.0-m96）。
> - **D0 侦察**：客户端网络 native 全为 C 层阻塞全协议（http_get 阻塞 recv 循环 /
>   https mbedtls 阻塞 IO / s3 连接池 h_exchange / ws runtime_ws / tcp-udp 阻塞
>   syscall / dns getaddrinfo）；M93-M95 让出覆盖的真实死角 —— M94 抢占只在 VM
>   指令边界、绝不在 native C 内部，M95 handler 协程化后 handler 内调慢上游
>   http_get 即冻结整 worker（慢上游并发 ≥ worker 数饿死全系统）。
> - **路线决策（β）**：α（epoll 状态机化重写全协议 + TLS 非阻塞）= 数里程碑不可达；
>   β（阻塞 native 外包执行线程池，Go cgo/LockOSThread 范式）—— C 代码零改、语义
>   零变、一刀切覆盖全部阻塞 native。px 网络栈在 C 层 = Go 的 cgo 边界等价物 → β
>   是正确工程选择；α 记二期候选（协议整体 VM 化后自然可得纯正 epoll 形态）。
> - **S2 offload 执行器核心**：PxOffTask + 外包线程池（按需创建 / 空闲回收 /
>   PX_OFFLOAD_MAX 上限默认 max(8, CPU×2)）；vm.c CALL native 预检扩展
>   `px_native_offload_kind` 命中名单 → 打包 args 投递 → 协程登记 offload 等待让出
>   → 外包线程执行完写任务槽唤醒 → px_vm_resume 恢复写 dst + pc 前进（**不回退
>   重试**，语义 = M93 sleep 预写 dst 让出模式推广）；主线程/逃生舱/嵌套回调
>   （yield_ok=0）直调零变化。实证：PX_CORO_WORKERS=1 + 慢 http(500ms) + 快
>   sleep(60ms) 协程 → sleep 66/71ms 完成（worker 未卡）；offload 与直调逐字节一致；
>   线程池收敛 + 空闲回收（7→3）。
> - **S3 名单全集 + 错误回传**：名单扩至 17 项（http_get/post/unix + tcp/udp/dns +
>   s3_* 4 项 + ws_* 4 项；排除 http_get_stream —— chunk 回调用户 VM，违反纯网络
>   约束）；**实锤修复编译器级坑** = setjmp 包装 helper 的"longjmp 回返回 0 → 调用者
>   分支"在 gcc -O1/-O2 不可靠（最小复现：返回 0 被误判真 → 死循环）→ 重构为
>   runtime.c `px_native_call_capture`（setjmp 函数内消化，调用者只见普通一次返回）；
>   错误回传实证：外包线程 px_error → 协程恢复重抛带源位置 → worker 隔离 → 宿主
>   继续（语义与直调一致）。
> - **S4 收口（本 commit）**：回归中发现并修复 M93 协程化潜伏真实缺陷 —— 帧协程
>   多 worker 并发 println 到同一 stdout 行内多次 printf 非原子 → 行交错合并
>   （m93_s2 门1 偶发 R=63，10 次复跑 2 次触发）→ runtime 加 g_print_mu 整行原子锁
>   （bi_print/print_err；GC 标记不涉此锁简单互斥即可，print 低频 I/O 开销可忽略）
>   → 修复后 coro_print 10/10 全 64 稳定。
> - **验证门全绿**：m96_s2 套件 **8 PASS**（不卡 worker 实证 + 4 offload/直调逐字节
>   一致 + 线程池上限收敛 7 空闲回收 3 + 逃生舱 --c 直调不变）+ m96_s3 套件 **9 PASS**
>   （名单 7 项本地可测全集对拍 + Err Result 回传 + px_error 隔离带源位置 + 150
>   offload × 1850 分配低阈值 precise GC 零崩）+ 回归（vm_ab 38 PASS 0 GAP 0 FAIL +
>   m89_s3d 9 + m93_s2 6 + m93_s3 6 + m94_s2/s3 8 + m95_s2 12 + m95_s4 10 + m82 +
>   m83_s6 全绿 + diffcheck --all ✅）+ 双自举证明（compiler_vm 重放 compiler.px 与
>   golden/compiler.bc.dump 逐字节一致）+ bootstrap/pxi_vm 重链吸收 M96 runtime
>   （9,329,448 → 9,334,320B，hello 与 pxi stdout 一致）+ bootstrap/pxi 重链吸收
>   M96 runtime（9,495,360 → 9,504,560B，--c 轨解释器，M91 重链批次惯例）。详见
>   docs/M96_PLAN.md。
> - ⚠️ 二期边界（M96_PLAN §五/D8 二期候选）：α 纯正 epoll 网络栈（协议整体 VM 化后）；
>   文件 IO / popen 子进程等待等其余阻塞 native offload 候选；M97 = px_serve
>   route/vhost handler 协程化（M95_S5_PLAN.md，暂缓）。

### M95 · 服务端 handler 协程化（D8-② http_serve 系：活跃请求占协程不占 fserve worker）

> M95 = M94_PLAN §五 编排 D8 二期第 ② 条（http_serve/http_serve_unix handler 协程化）。
> commit 链：a6d6056（S1 立项定稿 + S2 实现闭环）+ 本 commit（S3 收口 + tag v0.2.0-m95）。
> - **S2 handler 协程化**：http_serve/http_serve_unix 的活跃请求 handler 从「fserve
>   pthread worker 内同步 px_call（handler 长业务占线程）」→「handler 帧协程」。
>   http_conn_worker 拆段：读+解析（段1）→ handler.fn==px_vm_entry（VM）→
>   http_pend_put(stage=1) + `px_coro_spawn_ex` 帧协程 → **worker 释放**去取下一 job；
>   协程完成回调 http_handler_done（coro worker 线程）写 resp（stage=2）+ fserve_push
>   投回续处理 → fserve worker 重入循环顶 take → http_send_resp（段2 响应写 +
>   keep-alive，与同步路径共用，逐字节语义一致）。逃生舱（PX_NATIVE / C 轨 handler）
>   → 原同步直调零变化。
> - **机制扩展**：coro.c 完成回调（PxCoro.done_cb/done_ud + px_coro_spawn_ex，px_coro_
>   spawn = NULL 壳）；vm.c/h PxVmState.ret_val（跑完存顶层返回值供回调取 + GC 补标）；
>   http pending 表（fd 索引，req/resp 为 GC 根，gc 标记期补标 —— precise 漏标=UAF）；
>   px_evc_close 清 pending 项（fd 复用防串扰）；PX_SERVE_WORKERS 下限 8→2（协程承载
>   长业务后显式小池可用，默认仍 256）。
> - **S2 排障两案**：① verify 门5 假 FAIL = verify.sh 统计 bug（`echo` 未重定向致
>   daemon 无尾换行响应体粘连、grep 行锚定失效）→ 修 printf '\n' >>out + 逐文件
>   grep -l；② **真实死锁** = http_pend_gc_mark 在 GC 标记期持 g_conn_mu，与不屏蔽
>   SIG_GC_STOP 的既有 g_conn_mu 临界区（M88-B px_evc_*）形成 STW 死锁面（GC 等锁
>   vs 持锁线程被信号暂停）→ precise 低阈值 GC 压力下 daemon 卡死（health 不分配仍
>   响应、分配型请求全挂）→ pending 表改**独立锁 g_hpend_mu** + 全部临界区屏蔽
>   SIG_GC_STOP（对齐 M92 根栈原子性模式）。
> - **验证门全绿**：m95_s2 套件 **12 PASS**（功能路由 + fserve=2 × 30 并发 sleep
>   handler wall 0.25s（占协程实证，线程模型需 ≥3s）+ 20 并发期间线程收敛 9 + /chan
>   让出 + precise 低阈值 GC 压力 16/16 + close/HEAD 逐字节）+ 协程内核回归 m93_s2/s3
>   12 + m94_s2/s3 8 + **vm_ab 38 PASS 0 GAP 0 FAIL** + m89_s3d 9 + m82/m83_s6 ✅ +
>   diffcheck --all ✅；bootstrap/pxi_vm 重链吸收 M95 runtime（9,329,128 → 9,329,448B，
>   hello 与 pxi stdout 一致）。详见 docs/M95_PLAN.md。
> - ⚠️ 二期边界（M95_PLAN §五）：sse_serve（需先做「协程局部连接上下文」替代 __thread
>   g_cur_conn）、px_serve（route/vhost 管道 handler 调用点深埋 runtime_route）；完成后
>   评估 fserve/pool 默认 worker 数下调（256 → CPU 级）。

### M95-S4 · sse_serve handler 协程化（D8-② 二期：SSE 活跃 handler 占协程不占 fserve worker）

> M95-S4 = M95_PLAN §五 二期编排第 S4 条（sse_serve handler 协程化）。commit：本 commit。
> - **sse_conn_worker 拆段**：TLS 握手+读请求+解析+注册 conn id+发 SSE 响应头（段1）
>   → handler.fn==px_vm_entry（VM）→ 注册表项 stage=1 + `px_coro_spawn_ex` 帧协程 →
>   **worker 释放**去取下一 job（handler 内 chan/sleep/spawn 让出占协程不占 fserve
>   worker）；协程完成回调 sse_handler_done（coro worker 线程）置 stage=2 +
>   fserve_push(fd, SSE) 投回续处理 → fserve worker 重入 sse_conn_worker 入口
>   （注册表项 stage==2 take）→ **sse_conn_hold**（段2 收尾 = 原 step8/9：明文连接交还
>   IDLE 事件循环照看 / TLS 阻塞保持读 + 注册清理，同步路径与续处理共用，逐字节语义
>   一致）。逃生舱（PX_NATIVE handler）→ 原同步直调 + sse_conn_hold 零变化。
> - **D0 关键结论**：g_cur_conn 全库**无读取消费者**（仅赋值 + extern + 注释，历史
>   遗留；sse_send/sse_close 按 conn id 查 g_sse_conns 注册表，不依赖 TLS 线程局部）
>   → 无需「协程局部连接上下文」迁移机制；SSE 一连接一 handler（无 keep-alive 循环）
>   → pending 仅需注册表项 stage 标志（无独立 req/resp GC 根面需求 —— req 由协程
>   args 保活，SSE handler 返回后不写 HTTP 响应）。
> - **验证门全绿**：examples/m95_s4/verify.sh **10 PASS**（功能路由 /health /sleep
>   /chan + **fserve=2 × 20 并发 /sleep(0.6s) wall 0.65s**（占协程实证，线程模型 2
>   worker 需 ≥6s）+ 20 并发期间线程收敛 8 + /chan spawn+chan.recv 让出 + precise
>   低阈值 GC 压力 12/12 + 客户端断连不崩服务健康）+ TLS SSE 冒烟（tls_server +
>   sse_serve handler 内 sleep 两段推送 + sse_close，curl -k 收流正确；handler 返回
>   连接保持路径断连后服务健康重连正常）。回归全绿：m83_s6 9（SSE legacy + http_stream
>   takeover）+ m95_s2 12 + m89_s3d 9 + **vm_ab 38 PASS 0 GAP 0 FAIL** + m93_s2/s3 12
>   + m94_s2/s3 8 + diffcheck interp ✅。详见 docs/M95_PLAN.md §五。
> - ⚠️ 继续边界：M95-S5（px_serve route/vhost handler 协程化，g_pool 收敛到 coro）。

### M94 · 协程调度完备性（D8 二期-A：抢占 + 定时器并入调度循环 + 逃生舱边界决策）

> M94 = M93_PLAN §D8 二期第 ③④ 条落地（①② 网络 IO / handler 协程化编排 M95/M96）。
> commit 链：391ea8e（S1 立项 + 设计 D1-D3 + D0 侦察）+ 30d94c6（S2 抢占）
> + b2f9426（S3 定时器合并）+ 本 commit（S4 收口 + tag v0.2.0-m94）。
> - **S2 抢占式时间片**（补 M93 真实缺口：纯计算不阻塞协程独占 worker 至进程退出，
>   WORKERS=1 时死循环可饿死其它 spawn）：PxCoro.run_begin_us + vm_run_loop 每 4096
>   条指令 weak 查 px_coro_preempt_check（本次运行 ≥ PX_CORO_QUANTUM_US 默认 5ms）
>   → return 2（新返回码：抢占让出）。抢占点仅解释循环指令边界（绝不在 native C
>   内部 / 让出登记临界区内）→ 协程未登记任何等待 = 无唤醒源 → worker 直接放回
>   就绪队尾 FIFO 轮转（无 lost-wakeup / 双执行竞态；区别于阻塞让出 return 1）。
>   PX_CORO_QUANTUM_US=0 关闭（回归 M93 无抢占语义逃生阀）。验证：WORKERS=1 死循环
>   ×2 vs 有限×8 公平轮转 PASS（M93 无抢占必饿死）；quantum=0 饿死重现（逃生阀
>   实证）；200us 小量子正确；coro_many 1000/1000 完成无吞吐回归。
> - **S3 定时器并入调度循环**（退役独立 timer 线程）：g_sleepers 移入 g_coro_mu 单锁
>   （登记/摘取/就绪队列同锁 → 消除旧双锁 g_timer_mu→g_coro_mu 锁序）；worker 取
>   协程前置摘到期 sleeper 入就绪（唤醒规则同 px_coro_wake：RUNNING→wake_pending
>   由让出 worker 自入队 / BLOCKED→直接入队）；就绪空且有未来 sleeper →
>   cond_timedwait 到最近到期（绝对时间自醒重摘）；sleep 登记成链表新头才 signal
>   （非头则 worker 已 timedwait ≤ 该 due，免打扰）。收益：线程数 -1（WORKERS=4 +
>   20 sleeper = 5 线程，旧版含 timer=6）；并发 sleep 精度保持（40 并发 wall≈max 121ms）。
> - **D8-④ 逃生舱边界决策**（文档收口，不改 codegen）：逃生舱（--c fn_* C 递归产物）
>   保持 pthread spawn —— fn_* 无 VM 显式帧状态，帧协程在其上不可行（M89 prestudy
>   §2.4 定论）；逃生舱产物内 spawn **VM 函数目标早已协程化**（M93-S2 分派按目标
>   函数 fn==px_vm_entry，与调用者轨无关）；逃生舱整体 VM 化/精确化降级为 codegen
>   golden 大迁移立项候选。M93_PLAN D8 状态更新（③④ 完成，①②→M95/M96 编排）。
> - **验证门全绿**：m94_s2 verify 4 PASS + m94_s3 verify 4 PASS + m93_s2 6 + m93_s3 6
>   + m89_s3d 9 + **vm_ab 38 PASS 0 GAP 0 FAIL** + diffcheck --all ✅ + m82 + m83_s6 ✅；
>   双自举证明：compiler_new/compiler_vm 全链重链（含 M94 runtime）后 compiler_vm
>   重放 compiler.px dump 30582 行与 golden **逐字节一致**；bootstrap/pxi_vm 重链
>   吸收新调度器（9,329,184 → 9,329,128B，hello 一致）。详见 docs/M94_PLAN.md。
> - ⚠️ 后续编排（D8 二期 ①②）：**M95** 服务端 handler 协程化（http_serve/sse/
>   px_serve 活跃请求占协程不占 fserve worker）；**M96** 客户端网络 IO 协程化
>   （http/tcp/udp/ws/s3/dns/fd_wait 阻塞桥 → 事件登记让出）。work-stealing 后置。

### M93-S2/S3 · 帧协程 M:N 内核 + 阻塞原语让出（chan/mutex/rwlock/sleep 协程化）

> M93（候选③，M88 C 类旗舰）落地：`px_spawn` VM 函数从每 spawn 一个 pthread →
> **用户态帧协程 M:N**（worker 池 = min(CPU,8)，协程 = 独立 PxVmState，切换只换
> 指针，零 ucontext/零汇编）。commit 链：64034a5（S1 立项+设计 D1-D8）+ 695651a
> （S2 协程内核）+ 本 commit 04a11a7（S3 阻塞让出，路线 B）。
> - **S2 内核**：协程 = 独立 PxVmState（帧栈全堆）→ spawn 目标判 fn==px_vm_entry
>   即协程化；8 worker 常驻 + FIFO 就绪队列；线程数从 spawn 数收敛到 worker 数
>   （spawn 128 → 9 线程）；GC 根面 = 协程表（args + 帧槽精确标记，BLOCKED 无 C
>   栈 = 纯帧槽根，M92 直接受益）；逃生舱 fn_* 仍 pthread（语义零变化）。
> - **S3 阻塞让出（路线 B：解释循环 C 层自动包装，不动编译器/bc_emit）**：
>   chan send/recv、mutex lock、rwlock r/wlock 在协程 ctx 走 try 变体 → 失败登记
>   对象等待链表（cw_*）让出（worker 不阻塞）；sleep/sleep_us 登记单后台 timer
>   线程让出（并发 40 睡 wall≈max 121ms 非累加 3.2s）；with/with_read/with_write
>   展开 = 压 fn 帧 + 帧弹自动解锁（fn 中途让出锁随帧保留 + GC 保活锁对象）。
>   非协程 ctx（主线程/嵌套 native 回调/逃生舱）→ 原 pthread 语义零变化。
> - **修复两并发 bug**：① 让出窗口双执行/UAF（登记到 worker 让出间唤醒并行
>   resume + 让出 worker 读协程 UAF）→ 让出标志改运行函数返回码 + wake_pending
>   延迟调度状态机（state 收敛 g_coro_mu，ASAN 12 轮零报）；② timer 线程摘到期
>   后仍 cond_wait 漏唤醒 → batch 非空不 wait。
> - **验证门全绿**：examples/m93_s3/verify.sh **6 PASS**（mutex 4000/chan 乒乓
>   8×300/并发 sleep/with 展开/让出×GC 低阈值/逃生舱对拍）；m93_s2 6 + m89_s3d 9
>   + vm_ab 31P/7GAP(已知)/0F + diffcheck --all ✅ + m82 + m83_s6 ✅；万级 spawn
>   10000 协程 3.26s；5000 协程×分配×GC 37s 零崩。详见 docs/M93_PLAN.md。
> - ⚠️ 已知边界（D8 二期）：网络 IO（http/tcp/ws）epoll 协程化、服务端 handler
>   协程化、抢占/work-stealing、逃生舱内 spawn 协程化 —— 后置。

### M92 · 精确 GC 终极项收口（tag v0.2.0-m92）

> M92 完整交付：VM 主执行轨退役整栈保守扫描 → 精确根面（全局槽 + VM 帧槽 +
> native 桥登记根栈），双模式由产物轨自动判定（VM 轨默认 precise；C 轨逃生舱
> conservative）。commit 链：2017101（S1/S2a 框架）+ a55a13c（S2b 语言核心层）
> + a323505（S2c 服务/IO 层 + 根栈原子性）+ 9d91f24（S2d 插桩）+ 本收口。
> 详见 docs/M92_PLAN.md。tag **v0.2.0-m92**。

### M92-S2d · VM 轨产物默认 precise 插桩（bc_emit）+ 自举重建 + 双 golden 更新（2026-09-09，dongyue）

> M92 S2d 完成：bc_emit.px 产物 main 插 `px_gc_set_precise(1)` → **默认 `px build`
> VM 产物自动进入 precise 精确根面**（退役保守栈扫描；C 轨逃生舱产物不插 →
> conservative，逃生舱语义零变化）。这是 M89_S3-D 后置决策三件套的最终落地——
> 默认轨（VM）成为精确 GC 主执行轨。
> - **插桩**（selfhost/bc_emit.px）：`main()` 里 `px_register_builtins()` 后加
>   `px_gc_set_precise(1);`（含注释）。产物 C 实测含该调用（precise_stress.c
>   grep 命中）。
> - **自举全链重建**：compiler.px（import bc_emit）→ compiler_new（C 引擎编译器）→
>   compiler_vm（VM 驱动编译器，含新 bc_emit）→ 部署 bootstrap/pxc_vm（9,334,096 →
>   2,566,248B 基线变化见二进制）。此后 `px build` 产物默认 precise。
> - **双 golden 更新**：golden/compiler.c 15058 → **15061 行**、golden/compiler.bc.dump
>   30578 → **30582 行**（双引擎 C/VM 编 compiler.px 镜像逐字节一致）。
> - **验证（默认 precise 下全量回归绿）**：vm_ab v2 **38 PASS 0 GAP 0 FAIL**（38 例
>   逐字节对拍，precise 根面无漏登记 UAF）；m89_s3d **9 PASS**（VM 并发 GC ×3 +
>   堆回落 ×3 波）；m82 verify 8 PASS（http_serve_unix）；m83_s6 verify 全 PASS
>   （SSE/http_stream/断连）；diffcheck --all 24 用例全 ✅；双自举证明
>   bootstrap_prove.sh rc=0 + bootstrap_prove_bc.sh rc=0（新 golden 逐字节）；http_json
>   默认 precise（thr2000 deferrable 真实服务模式）8-12 并发 5×6000=30000 请求 0 错
>   RPS 1270-1535（与 conservative 持平，无性能回归）；px_serve+route 默认 precise
>   200 并发全 200。
> - 备注：M92_PLAN 状态/验收同步；残余 issue（PX_GC_INLINE=1 强化模式稀有 GC 标记
>   崩溃）记录于 S2c 段，真实服务 deferrable 模式无窗口。

### M92-S2c · precise GC native 桥登记批次 2（服务/IO 层）+ 根栈并发原子性修复（2026-09-09，dongyue）

> M92（精确 GC 终极项，docs/M92_PLAN.md）S2c 完成。S2a 框架 + S2b 语言核心层
> 登记（2017101/a55a13c）后，本批把服务/IO 主路径 native 桥的跨 GC 点裸局部全部
> 登记进 precise 根面，并用 precise 低阈值 + 服务压测验证。
> - **服务端主 worker 登记**（runtime/runtime.c）：
>   - `http_conn_worker`（http_serve/http_serve_unix 共享）：headers/req/form/resp
>     请求迭代作用域登记（px_root_push/keep/pop 严格配对，覆盖全部出口含 413/
>     stream 接管/file 流/keep-alive 关闭/idle 交还）——precise 压测实锤崩点
>     （http_conn_worker→px_dict_set→UAF，56 请求即崩 → 登记后 14400 请求 0 错）；
>   - `sse_conn_worker`（SSE 服务）+ `sse_parse_event_c`（SSE 事件文本累积 dict）；
>   - `px_conn_worker`（px_serve 主 worker：headers/req/form 迭代登记）；
>   - `px_http_dispatch` vhost handler 返回值段（normalize/respond 期间保护）；
> - **客户端桥登记**：`bi_http_request`/`bi_http_unix` 响应 dict 构造（h_exchange
>   填充的 headers + d 跨 px_dict_set/px_str_len 分配）；
> - **route 层登记**（runtime/runtime_route.c）：`route_match` 匹配 params 构造 +
>   `px_route_try_dispatch` params/中间件/route handler 返回值（跨 normalize/send）；
> - **附带修复 · 根栈并发原子性**：`px_root_push/pop/keep` 加 SIG_GC_STOP 屏蔽
>   （gc_block_stop/gc_unblock_stop）——根栈 n++ 非原子，若 GC 暂停信号落在中途，
>   handler 快照 ti->root_n 读到半态 → 刚登记局部漏根被误回收（并发随机 UAF 根因），
>   与既有 list/dict 结构修改关键区同模式；
> - **审查确认无需登记**：vm.c bi_vm_*（chan/mutex/spawn/try_recv 无跨 GC 裸局部）、
>   runtime_ws.c ws_conn_worker（handler 全局表保护 + arg 为 int）、bi_ffi_call（查表直调）；
> - **验证**：precise 低阈值（thr500-1500 + PX_GC_INLINE=1 强化）http_serve 8 并发
>   压测 6×2400 + 16 并发 3×4800 全 0 错；m82 verify 8 PASS；m83_s6 verify 全 PASS；
>   px_serve+route+middleware 并发 499/500（新增 examples/m92_precise/s2c_pxserve.px）；
>   m31_vhost precise 自检 ALL OK；conservative 零回归 m89_s3d 9 PASS + vm_ab v2
>   38 PASS/0GAP/0FAIL + precise_stress1/2 逐字节一致；真实服务模式（deferrable
>   安全点 GC）precise 19.2 万请求稳定。
> - **已知残余（列 issue）**：PX_GC_INLINE=1 强化模式 precise 长跑 ~2 万请求稀有
>   崩溃（GC 标记 hash set 坏指针；INLINE 强制内联 GC 至任意分配点的极稀有交错，
>   诊断版 xrealloc 加检查即可改变时序规避）。真实服务 deferrable 模式无窗口（GC
>   只在请求间安全点执行）。INLINE 为压测强化手段，非生产路径；记录待专项深查。
> - 新增：examples/m92_precise/s2c_pxserve.px（px_serve+route precise 冒烟压测）；
>   M92_PLAN 状态/验收/残余更新。

### 重链批次 · bootstrap/pxi + pxi_vm 吸收 F3-fix runtime（2026-09-09，dongyue）

> M91_PLAN S1 事实 7 标注的后续批次（基座大二进制变更留独立批次）。F3-fix
> （de5ceef，http_serve keep-alive 长压卡顿根治：fserve_gc_reg 空闲注销 GC 槽 +
> 超时 tick poll(0) 二次确认兜底）改 runtime/runtime.c 后，解释器引导二进制
> 未重链 → 本次全能力重链让解释器也带 F3-fix runtime。
> - **bootstrap/pxi**（C 轨解释器，9,480,688→9,480,960 B）：`px build --c --full
>   selfhost/interp.px`（C 轨逃生舱全能力）→ cp。pxi 0.2.0、hello stdout 与旧
>   逐字节一致、strings 确认 fserve_gc_reg/tmo_save/tmo_close 链入。
> - **bootstrap/pxi_vm**（VM 轨解释器，9,314,512→9,314,792 B）：`px build --full
>   selfhost/interp.px`（M91 默认 VM 引擎 = pxc_vm --emit-c BCModule 镜像）→ cp。
>   pxi 0.2.0、hello stdout 与旧一致、pxi == pxi_vm stdout 逐字节一致。
> - **回归抽查**（对齐 M89-S4 重链惯例）：diffcheck --all rc=0（新 pxi 全量对拍
>   golden）+ vm_ab.sh v2 **38 PASS 0 GAP 0 FAIL**。
> - 备注：pxc_vm 已在 M91-S3 重链（9,338,504B 含 F1+PXOP_NARGS）不需重复；pxc
>   （C 轨编译器）按 M89 决策保持不重链（编译只产文本不依赖 runtime 版本）。

### M91 · 默认轨切 VM（px build 默认产物 = 字节码 VM 轨 + C 轨逃生舱）

> 完成（2026-09-09，dongyue）：M91（立项依据 docs/M90_S4_switch_eval.md + perf 基线）
> 按 S0-S4 收口，详见 docs/M91_PLAN.md。commit fca0cee（S3）+ 收口提交。
> - **tools/px 引擎反转**：`px build` 默认 = VM 轨（compiler_vm --emit-c → BCModule
>   字节码镜像 C → 链 runtime 含 vm.o → 显式帧 VM 执行）；新增 `--c`（C 轨逃生舱
>   fn_* 文本轨，纯计算热点 ≈1.5x 用）；`PX_BUILD_ENGINE=c|vm` env 逃生舱（脚本/CI
>   一次性回退；显式 flag > env > 默认 vm；非法 env 警告退默认）；`--vm/--bc` 保留
>   兼容（= 新默认）。VM 编译器优先 PXC_VM_BIN > bootstrap/pxc_vm > dev compiler_vm。
> - **VM 轨自动裁剪补齐**（M89-S3-C2 试做缺口）：VM 产物无 px_get_global → 提取
>   BCModule 镜像 s_G 字符串表（全部全局/native 引用名，等价信息）→ 与 C 轨共用
>   native_mod_map 反推。实测裸 hello：VM 全能力 9,033,936B → 裁剪 2,737,200B
>   （≈ C 轨 2,736,992B，无体积回归）；capability 8,953,320B 自动保留 sqlite，
>   运行 253 PASS/0 FAIL。
> - **bootstrap/pxc_vm 静态重链**（M91 硬前置）：旧入库版（M89-S4 9,334,096B）无 F1
>   默认参数 → capability emit-c 报 "bc_emit 默认参数未实现"；基于含 PXOP_NARGS 的
>   compiler_vm.c（M90-S1 重链产物）gcc -static 链全能力 rtcache 重链 9,338,504B →
>   emit-c capability rc=0、--version = pxc 0.2.0。
> - **vm_ab cache 选择修复**（M91 暴露的测试基建 bug）：vm_run.sh 原选"最新含 vm.o
>   rtcache"，M91 默认自动裁剪后最新 cache 常为裁剪态 → 引用 aes/xml/zip native 的
>   用例（p7 等 2 例）报"未定义变量"伪缺口；改为优先全能力 cache（含全部模块 .o），
>   无则回退最新并警告 → vm_ab v2 恢复 38 PASS 0 GAP 0 FAIL。
> - **回归全绿**：m91_s1 verify 7 PASS（默认=VM 轨 / --c=C 轨 / env=C 轨 / VM 自动
>   裁剪 <4MB / flag 优先 env / 双轨 stdout 逐字节一致 / --vm 兼容）+ m90_s1 5 PASS
>   + m89_s3d 9 PASS + vm_ab v2 38 PASS 0 GAP 0 FAIL + diffcheck --all + m82 unix
>   全 PASS（issue28 场景默认 VM --no-quic）+ m83_s6 SSE 全 PASS + 双自举证明
>   （C 轨 B.c==golden 15058 行 + BC 轨 VM 重放 == golden/compiler.bc.dump）。
> - 说明：diffcheck/prove* 直调 bootstrap/pxc 不经 tools/px → 自举守护基线不受默认
>   切换影响；px refs 仍提取 C 轨引用集（C 轨保留为逃生舱，命令继续有效）。
> - 后续批次（收口标注，未并入 M91）：bootstrap/pxi/pxi_vm 重链吸收 F3-fix runtime
>   （解释器非 http_serve 长压主用例，基座大二进制变更收益低风险高，留独立批次）。

### F3-fix · http_serve keep-alive 长压卡顿修复（GC×fserve 池交互根因 + 超时 tick 兜底）

> 完成（2026-09-09，dongyue）：M90-S2 F3 的修复落地。**根因实证修正**：误杀活跃连接的
> 真根因不是 epoll 漏报，而是 **M11 并发 GC（STW）与事件化 fserve 池交互**——事件化后
> 256 fserve_worker 绝大多数空闲 cond_wait，GC 每次 STW 暂停全部注册线程的协商开销 +
> 被暂停 worker 卡 cond_wait 锁窗口 → 事件循环 fserve_push 长时间阻塞 → 连接请求 15s
> 无人处理。实证链：ET/LT 均漏报排除 epoll 语义 → SAVE 插桩 idle_ev_cnt=0 但高频 wait
> 日志每批含该 fd → gdb 抓拍事件循环卡 fserve_push 锁 + 254 worker 被 GC 暂停自旋 →
> **禁 GC 后 RPS 218→1400、卡顿归零（实锤）**。
> - **治本**：`fserve_gc_reg()` —— fserve_worker 空闲（cond_wait）注销 GC 槽，取 job
>   处理前注册（空闲 worker 无 px 对象 → 不参与 GC 暂停安全）；GC 只暂停在岗 worker。
> - **兜底**：事件循环空闲超时 tick close 前 `poll(0)` 二次确认，活跃连接（有在途数据）
>   救回投回 fserve 池，杜绝任何路径下被 15s tick 误杀。
> - **验证**（GC 默认开启 8×3000 长压）：RPS 216→**1288**、ok 24000/24000、max 18s→262ms、
>   tmo_save 9~20→0。回归：m89_s3d 9 PASS、m90_s1 5 PASS、m82 unix 全 PASS（issue28 场景）、
>   m83_s6 SSE/http_stream 全 PASS、vm_ab 38 例 FAIL 0、短压 RPS 1473。
> - 诊断：PX_EV_DIAG=1 门控计数器（idle_put/detect/tmo_close/tmo_save + SAVE 明细），
>   生产默认关零开销。详见 docs/M90_S2_F3.md §6。

### M90-S2/S3/S4 · F3 快查 + stdlib 全 VM 冒烟 + 默认轨切换立项评估（M90 收官）

> 完成（2026-09-09，dongyue）：M90（默认轨切换前奏）S2-S4 收官（S1 见下一条目）。
> - **S2 F3 快查（docs/M90_S2_F3.md）**：HTTP keep-alive 高并发崩溃判定真 bug ——
>   **15s keep-alive 空闲超时误杀活跃连接**（M88-B-S2 事件化遗留，与 VM/C 轨无关）。
>   关键证据：崩溃阈值 = 压测时长跨 ~15s（M89 短测 8 并发 12.3s <15s 未触发被掩盖；
>   长压 8/20 并发均复现：RPS 249-301、ok 率 26-65%、max 16.6-17.7s 尖刺、Connection
>   reset；原版产物干净环境复现排除改动/噪声）。已试修复回退（扫描限频无效；worker
>   3ms 乒乓短窗 20 并发 RPS 631/尖刺消但 8 并发退化）。**修复列独立批次 F3-fix**
>   （超时 close 前 poll 二次确认兜底 + 事件循环漏报深查）。
> - **S3 stdlib 全 VM 编译冒烟**：stdlib 13/13 `compiler_new bc` 通过（F1 后无缺口；
>   capability 41KB 全量 VM 运行 == pxi 已在 S1 兑现）。
> - **S4 默认轨切换立项评估（docs/M90_S4_switch_eval.md）**：技术前置全齐（F1/
>   stdlib/语义对拍 38 例/perf 代价量化 0-10%）；golden 大迁移范围 + AB 回退开关
>   （`px build` 默认 VM + `--c` 逃生舱 + PX_BUILD_ENGINE env）设计完成；建议
>   F3-fix 先行后按 S0-S4 独立立项（如 M91）推进切换，无技术 blocker。
> - M90 收官状态：S1 代码 + S2 报告 + S3 冒烟 + S4 评估全绿；F3-fix 与默认轨切换
>   立项为下一批次。

### M90-S1 · bc_emit 默认参数支持（F1）+ for/continue 死循环修复 + capability VM 化跑通

> 完成（2026-09-09，dongyue）：M90（默认轨切换前奏：bc_emit 缺口清障）S1 收官，
> 详见 docs/M90_PLAN.md。默认轨切换前置硬缺口 F1 落地，并连带暴露/修复 M89 遗留
> continue-for 死循环、实现 capability.px（41KB 系统文件）全量 VM 化运行。
> - **F1 bc_emit 默认参数**：新指令 PXOP_NARGS 57（槽=本帧实际实参数，PxFrame
>   增 nargs）+ bc_emit_func_body arity=必需数/ndefault 计数 + 函数入口
>   bc_emit_default_fill 填充序列（NARGS→LT(pi<nargs)→JMPT 跳过/求默认表达式入
>   槽，对齐 codegen callee 侧入口 `(nargs>i)?args[i]:default` 语义，经别名/闭包
>   间接调用缺参同样兜底；默认值可引用前参/全局）。此前 `compiler bc
>   capability.px` rc=1（panic "bc_emit 默认参数未实现"）→ 现 rc=0。
> - **附带修复 bc_emit_for continue 死循环（M89 遗留，vm_ab 38 例盲区）**：continue
>   原回填到增量后 JMP → body 内 continue 跳过 ctr+=1 → 死循环（capability VM 化
>   卡死暴露，t1_bc.px 最小复现）；改回填到增量段起点（Python for：先 +1 再重判）。
> - **capability.px 全量 VM 化**：41KB 系统能力自检（253 PASS，含 FFI sqlite/
>   h3-qpack/默认参数/推导式/生成器/break-continue）emit-c → VM 运行 == pxi
>   **303 行 stdout 逐字节一致**。
> - **双自举 golden 更新**：compiler.c 15058 行 / compiler.bc.dump 30577 行（随
>   compiler.px 生态源码改动；双引擎 C/VM 编 compiler.px 镜像逐字节一致）。
> - **回归全绿**：m90_s1 verify 5 PASS（default_args 双轨一致/capability BC 编译/
>   NARGS 落地/cmpsem C轨==VM轨入口求值语义/continue 回归）+ vm_ab 38 PASS +
>   m89_s3d 9 PASS + diffcheck --all + prove.sh/prove_bc.sh 双自举证明。
> - **关键操作**：runtime vm.h/vm.c 变更 → 旧 rtcache 无 PXOP_NARGS → `px build
>   --full` 建新全量 rtcache + compiler_new/compiler_vm 重链新 vm.o。
> - 备注：默认参数「pxi def 时固化 vs codegen 入口求值」为既有两轨分歧（非 VM
>   引入，VM 对齐 codegen 替代目标）；pxi 不支持默认引用前参。

### M89-后置决策 · VM vs C 轨性能基线（默认轨切换数据底座）

> 完成（2026-09-09，dongyue）：M89-S4 收口后置决策项「默认轨切 VM」的量化数据
> 基础（docs/M89_PERF_BASELINE.md + examples/m89_perf/ 一键复现资产）。
> - **三类负载实测（同源双轨产物对拍，3-5 轮取中位）**：fib 纯计算热点 VM/C =
>   **1.52x**；compiler 形态综合真实负载（compiler.px 双轨产物跑同一 bc dump）=
>   **1.04x**；HTTP JSON 服务 8 并发 RPS = **1.00x（持平）**。
> - **结论：此前"VM 解释 = 机器码 5~20x 慢"假设被实测推翻**（根因：px C 轨产物
>   每步值操作仍走 runtime 动态值层，函数级机器码占比小，VM dispatch 只是叠加小层）。
>   服务/IO 与综合负载切换代价 ≈ 0~10%，纯计算热点 ~50%（可 C 轨逃生舱 / native 兜底）。
>   默认轨切换从"高成本后置大决策"重估为**可行低风险项**（仍须独立里程碑：
>   bc_emit 默认参数缺口 F1 + golden/capability 大迁移 + 稳定期/AB 回退）。
> - 附带发现归档：F1 bc_emit 默认参数未实现（capability.px bc rc=1）；F2 bc dump
>   大文件慢（compiler.px 全链 >10min；emit-c 4-5min vs pxc 3.5min）；F3 HTTP 20
>   并发双轨 RPS 均骤降且 max ~9s 尖刺（runtime 服务层/GC 退化，与轨无关）；
>   F4 server 空转 spin。均记入 M90 候选。

### M89-S4 · VM 化旗舰里程碑收口（tag v0.2.0-m89）

> 完成（2026-09-09，dongyue）：M89（AST/C 递归 → 显式帧 + 平坦字节码 VM）S0→S4
> 全段收官。S4 收口门 = 自举证明 + 全量回归 + 重链 bootstrap + 文档同步 + tag。
> - **自举证明双轨成立**：C 轨 14986 行 == golden/compiler.c；BC 轨 compiler_vm
>   重放 30445 行 == golden/compiler.bc.dump（逐字节）—— 编译器源码↔二进制↔字节码
>   镜像三方一致。
> - **全量回归全绿**：diffcheck --all rc=0 + vm_ab v2 收口门 **38 PASS 0 GAP 0 FAIL**
>   （examples 确定性全集 VM vs 旧轨逐字节一致）+ m89_s3d verify 9 PASS + hello
>   三轨一致。
> - **重链 bootstrap（吸收 M89 runtime 改动）**：pxi / pxc_vm / pxi_vm 三引导二进制
>   全模块重链（含 S3-D 精确 GC 根面修复），重链后 diffcheck + vm_ab 复跑全绿。
> - 文档同步：M89_PLAN + CHANGELOG；决策记录（默认轨切 VM 后置批次）已归档。
> - **M89 里程碑成果一览**：55→57 op 显式帧 VM + 自举 BCModule 发射器 + 双轨对拍
>   收口门 38 例 + 三大 VM 化切片（px build --vm / pxc_vm / pxi_vm）+ 精确 GC 根面
>   完备性（跨线程帧根 + PX_GEN 标记）。**VM 化为 native 后端 / 帧协程 / 语义冻结
>   铺平道路（总钥匙）**。

### M89-S3-D3/4 · issue28 验收复核结论 + VM 并发 GC×生成器混合收口（精确 GC 根面完备性第 2 批）

> 完成（2026-09-09，dongyue）：S3-D 段收口补强 + issue28 定位修正。
> - **issue28 验收结论修正**：全量延迟验收（单发 p95≤50ms / 500 并发 p50≤200ms /
>   堆回落）已由**清歌 0.2.0 隔离实测复核通过**（`/data/qg-issue/28-…/VERIFY-20260908-
>   0.2.0-qingge.md`：单发 p50=1ms/p95≤6ms、>100ms=0；500 并发 p50=42~55ms 0 失败；
>   1500 并发 0 失败；空载 RSS 17~30MB）→ **ws-approve .px 版无需重编码可替换 Go 版
>   上线**（core/ws-approve + tools/wst-approve 已替换）。即 issue28 由 B1/B2/B3
>   **在无 VM 化时已解决**，S3-D 不再承担 issue28 验收门；D1（并发 VM 帧根）/D2
>   （PX_GEN 标记）定位明确为「VM 为唯一执行轨」的精确 GC 正确性前置，VM 化对
>   issue28 只吸收 §9.3 记录的残余低频偶发停顿（~0.4~0.6% >100ms，sweep 线性），
>   属锦上添花非解决路径。
> - **D4 组合最坏窗口压测**（examples/m89_s3d/vm_conc_gen_stress.px）：6 spawn
>   worker 并发跑 VM × 惰性生成器（部分消费 10/15 项跨 ~10 轮并发 GC 后全量消费
>   剩余，校验游标/transform/filter 子对象跨 GC 完好）+ 容器/字符串垃圾波 ——
>   D1（跨线程帧根）+ D2a（PX_GEN 递归标记）叠加验证；期望单 worker 435（420+
>   15×GKC=1）、K=6 总 2610。verify.sh 增第 5 项 ×3。
> - **verify.sh 9 PASS + 0 FAIL**：原 1-4 项（冒烟/并发 GC ×3/gen 护栏/VM 堆回落
>   RSS 6084→7156KB <40MB）回归不变 + 混合压测 ×3 PASS。
> - **S3-D 段收口**：段内可推进工作全部完成；剩余（退役整栈保守扫描、原生桥 ≤60
>   处逐一登记、cell 标记、issue28 残余停顿吸收）与「px build 默认切 BC + golden
>   大迁移」同锁 **默认轨切 VM 后置批次**，是 M89-S4 收口前最大待决项。
> - 文档：`/data/qg-issue/00-README.md` Issue 28 状态同步为「✅ 已复核通过（2026-
>   09-08 清歌 0.2.0）」。

### M89-S3-D1/2 · VM 并发帧根 + PX_GEN 标记（精确 GC 根面完备性第 1 批）

> 完成（2026-09-09，dongyue）：S3-D（精确 GC）本批按 R4 双根过渡（保守扫栈 +
> VM 帧槽精确根并存；退役整栈保守扫描/原生桥 ≤60 处登记随默认轨切 VM 后置）推进
> GC **正确性/根面完备性**，坐实并修复三缺口中的两个：
> - **D1 并发 GC 跨线程 VM 帧根**：旧实现只标本线程 VM 帧（96a8e4b 单线程止血），
>   并发 GC 停其它线程后其堆上帧槽不可见 → 活跃对象被误回收 use-after-free。
>   机制：GCThreadInfo.vm_state + 暂停处理器（目标线程）读自身 TLS VM 状态 +
>   executor 对每暂停线程 px_vm_gc_mark_state 跨线程精确标帧槽 + 补标 executor
>   自身 VM 帧；vm_frame_push 改「先初始化后发布 nframes」暂停安全不变量 + 帧
>   数组扩容 malloc+拷贝+发布（免 realloc 悬垂窗口）。
> - **D2 PX_GEN 子对象漏标**：gc_mark_obj 增 PX_GEN 分支（list/seq/transform/
>   filter 四值递归）——两轨通用防御（保守扫栈掩盖下作防回归护栏 + 纯精确前置）。
> - **A/B 实证**（examples/m89_s3d/vm_conc_gc_stress.px，8 spawn worker VM ×
>   PX_GC_THRESHOLD=20000）：修复前 runtime **3/3 segfault**、修复后 **3/3 PASS**
>   （旧 C 轨并发对照组 3/3 PASS → 崩溃确系 VM 帧根缺口）。修复中自查并修正弱
>   符号命名不一致（px_vm_gc_cur_state/px_vm_cur_state）导致跨线程标记被静默禁用。
> - 测试资产 examples/m89_s3d/：vm_spawn_smoke / vm_conc_gc_stress / gen_gc_stress
>   / verify.sh（冒烟 + 并发 GC ×3 + gen 护栏 + VM 轨堆回落 3 波采样）= 6 PASS。
> - 回归：vm_ab v2 收口门（full 模块 rtcache）**38 PASS + 0 GAP + 0 FAIL**；
>   px build / pxi 旧轨对照绿。M89_PLAN S3-D 段记录 + 后置项（退役保守扫描等）。

### M89-S3-C2-6 · vm_ab 收口门扩展：确定性 examples 19→38 全量对拍全绿

> 完成（2026-09-09，dongyue）：S3-C 收口门覆盖从 19 例精选扩到**确定性
> examples 全量 38 例**，对拍 **38 PASS + 0 GAP + 0 FAIL**（VM 轨 stdout 与
> 旧轨 pxi 逐字节一致）——bc_emit/VM 覆盖面无已知缺口。
> - **确定性判别自动化**（/tmp/probe_det.sh 方法固化进脚本注释）：119 顶层
>   examples 逐个 pxi 解释双跑，rc=0 且 stdout 逐字节一致 = 确定性用例；
>   排除 server 监听型/网络依赖/不可控时间/随机输出。38 例入选（2026-09-09
>   探测固化）。
> - **新增 19 例**：http_neterr_result / https_demo / m23d_rsa /
>   m29_jsonpath_web / m31_sandbox / m46_quic_smoke / m48_qpack_verify /
>   m49_qpack_dyn_verify / m53_s5_pxi_h3_smoke / m54_s5_pxi_quic_smoke /
>   m57_s3_gpio / m57_s3_i2c / m57_s5_pxi_smoke / m60_gpio / m60_i2c /
>   m60_pwm / m60_serial_pty / p7_aes_xml_zip / s3_neterr_fail。原 19 例全保留。
> - 覆盖类型显著扩展：QUIC/H3 smoke 与 qpack verify、GPIO/I2C/PWM/串口
>   设备 API、AES+XML+ZIP、JSONPath、RSA、sandbox、确定性网络错误路径 ——
>   VM 发射器对上述语法/库调用面全部与旧轨一致。
> - vm_ab.sh 顺带修参数解析：`v2` 子命令被误当过滤词导致全量跑空（0 用例），
>   改为 v2 作版本子命令 shift 忽略，兼容 `vm_ab.sh [v2] [<substr>]`。

### M89-S3-C2-5c · bootstrap/pxi_vm —— pxi VM 化原型（切片 3，分支试做）

> 完成（2026-09-09，dongyue）：compiler_vm --emit-c interp.px（装配壳 + 全
> 解释链 parser/env/cg_module/it_util/i_err/ival/icall/ibuiltin/iexpr/istmt）
> → BCModule 静态 C → gcc -static 链 → bootstrap/pxi_vm（9.3M 静态 ELF，
> **解释器自身跑在显式帧 VM 上**）。发射层面证明 bc_emit 已覆盖解释器全链
> 语法构造。--version = pxi 0.2.0；hello / m22_bitwise_data / p8_slice_base64
> / struct 与 bootstrap/pxi 逐字节一致。默认轨未切（pxi_vm 供 PXI_BIN/PX_PXI
> 覆盖式实验）。

### M89-S3-C2-5b · bootstrap/pxc_vm —— pxc VM 化静态重链（切片 2，分支试做）

> 完成（2026-09-09，dongyue）：compiler_vm 静态重链落位 bootstrap/pxc_vm
> （9.3M 静态 ELF，引导二进制入库）→ px build --vm 编译器默认路径升级为
> PXC_VM_BIN > bootstrap/pxc_vm > selfhost/build/compiler_vm。
> - 自举一致实证：pxc_vm bc compiler.px 重放（~6 分钟，RSS ~0.76GB）dump
>   30446 行 == golden/compiler.bc.dump 逐字节一致 —— 静态 VM 版编译器具备
>   权威编译能力（= pxc 的 VM 化重链技术成立）。
> - 验证：px build --vm（PXC_VM_BIN=pxc_vm）fib == pxi；pxc_vm bc hello ==
>   compiler_vm bc hello。

### M89-S3-C2-5 · px build --vm —— VM 轨产物产品化（分支 feat/m89-c2-vmtoolchain 切片 1）

\u003e 完成（2026-09-09，dongyue）：C2 主体最大项第 1 切片 —— tools/px 增 `--vm/--bc`
\u003e 开关，VM 轨产物正式纳入用户工具链入口，与旧 fn_* C 文本轨**并存过渡**。
\u003e - `px build --vm \u003cfile\u003e`：编译器切 compiler_vm（compiler.px 字节码镜像 + VM），
\u003e   --emit-c → BCModule 字节码镜像 C → gcc -static 链 runtime（含 vm.o）→ 静态
\u003e   ELF 跑显式帧 VM。PXC_VM_BIN 环境变量可覆盖 VM 编译器路径（切片 2 留口）。
\u003e - VM 产物无 px_get_global 引用 → 跳过 M86-S2 自动裁剪，默认全能力链接。
\u003e - 环境债：Rocky 9 缺 glibc-static（px build 本机不可用）→ 启 CRB + dnf install
\u003e   glibc-static → gcc -static 复活，px build 旧轨本机回归可用。
\u003e - 验证：hello / m34_gen_lazy（GENFROMLIST 物化）/ m30_comp（推导式多变量）/
\u003e   fib（递归）/ m39_gc（GC）五用例 px build --vm 静态产物 stdout == pxi 逐字节一致。

### M89-S3-C2-4 · 3 GAP 补齐（推导式多变量子句 + GenExp 物化路径）+ VM op GENFROMLIST —— S3-C 收口门 GAP→0

> 完成（2026-09-08，dongyue）：补齐 bc_emit 推导式已知缺口（vm_ab.sh v2 首跑
> 记录的 3 GAP：m30_comp/m32_gen/m34_gen_lazy）→ 收口门全绿 **19 PASS + 0 GAP
> + 0 FAIL**（VM 与旧轨 pxi stdout 逐字节一致）。
> - **runtime/vm.h + vm.c**：新 op **PXOP_GENFROMLIST 56**（a=dst, b=list 槽 →
>   px_gen_from_list：物化 GenExp 收集完 list 后包成 generator，对齐 codegen
>   px_gen_from_list / pxi it_gen 语义）；PXM_MAX 56→57；op 名表 + 分发 case。
>   指令集按名字扩展（既有 55 op 编号/语义零改动，纯追加）。
> - **selfhost/bc_emit.px**：① bc_emit_comp 支持**多变量子句**（for k, v in ...：
>   进入子句前每变量分配槽 + 保存旧绑定，循环体内迭代元素 INDEX 到 item 槽后
>   逐字段 px_index(item,i) 解包，越界 null —— 对齐 pxi i_bind_comp_vars /
>   cg bind；单变量路径分配序与既有实现一致，**bc1-12 dump golden 全回归不变**）
>   ② bc_emit_genexp 增**物化路径**：非「单 for 单变量」GenExp（多 for 笛卡尔积 /
>   多变量解包）→ NEWLIST 收集 + bc_emit_comp 嵌套循环展开 + GENFROMLIST 包 gen
>   （对齐 codegen cg GenExp 物化分支）；单 for 单变量仍走 NEWGEN 惰性不回归。
> - **golden 双轨同步**（bc_emit.px 被 compiler.px import → 编译器生态变化）：
>   golden/compiler.c = pxc 编 compiler.px 新产物（14987 行）→ bootstrap_prove
>   rc=0（C 轨自举成立）；golden/compiler.bc.dump = compiler_new bc compiler.px
>   新镜像（30446 行，diff 仅 bc_emit_comp 函数自身字节区）→ compiler_vm 重放
>   == golden 逐字节一致（BC 轨自举成立）。
> - 回归：bc1-12 dump golden 全绿；m30_comp（ALL PASSED）/m32_gen（m32_gen_ok）
>   /m34_gen_lazy（M34_GEN_LAZY_OK）VM 轨通过；vm_ab.sh v2 全量 19 PASS。
> - 下一步：C2 主体剩余 —— bootstrap/pxc/pxi VM 化重链 + px build 默认切 BC
>   产物（pxc 产物 = BCModule 内嵌 C + VM main）；建议分支试做 + 重生成 BC golden。

### M89-S3-C2-3b · vm_ab.sh v2 —— S3-C 收口门骨架（examples 确定性用例 VM vs 旧轨 stdout 对拍）

> 完成（2026-09-08，dongyue，commit b5bddb4）：S3-C 收口门落地首版 —— 对拍
> examples 本地确定性用例（旧轨 pxi vs VM 轨 vm_run.sh emit-c→gcc→run）
> stdout 逐字节一致 = PASS。
> - 首跑：19 用例 → **16 PASS + 3 GAP + 0 FAIL**。PASS 覆盖算术/字符串/字节/
>   闭包/GC/Result/单 for 推导式/regex/base64/struct/工具链 —— VM 与旧轨语义零
>   漂移实证。3 GAP = m30_comp/m32_gen/m34_gen_lazy（bc_emit 已知缺口：推导式
>   多变量子句/GenExp 多 for 物化路径，B3a 记录'多 for 物化路径待接入'），分类
>   记录不阻塞（FAIL>0 才退出码非零）。
> - 清单可传参过滤（vm_ab.sh <substr>）+ --list 列清单。
> - 下一步：补齐 bc_emit 推导式缺口 → 重跑收敛（GAP→0）→ pxc/pxi VM 化。

### M89-S3-C2-3 · vm_run.sh —— VM 轨运行器（compiler.px 正主 emit-c → gcc → 运行）

> 完成（2026-09-08，dongyue，commit a908d69）：bc_cli 姊妹壳退役第一步 —— VM
> 发射切 compiler.px 正主（selfhost/build/compiler_new = pxc 编 compiler.px 的
> C 引擎产物，含 bc/--emit-c 子命令）。
> - 流程：compiler_new --emit-c <file> → 静态 BCModule C → gcc 链 rtcache
>   （含 vm.o）→ 运行；退出码透传。
> - 验证：hello/fib/m22/m25/m26/m39/m40/match/p2/p8/struct/toolchain_demo 均
>   VM==pxi 逐字节一致；m34_gen_lazy 暴露 bc_emit 多 for GenExp 缺口（记录待补）；
>   std_demo 差异为时间相关输出（非语义差异）。

### M89-S3-C2-2 · golden 同步修复 + BC 轨自举证明脚本（自举证明规则更新落地）

> 完成（2026-09-08，dongyue）：C2 主体第 1 项（自举证明规则更新）落地，修复 C2-1
> 提交的 golden 与源码不同步问题。
> - **C2-2a golden 双轨同步修复**（commit d01e2a7）：C2-1 提交(90f2a29) 中 compiler.px
>   （21:44 最后微调 bc flag → --emit-c，对齐 bc_cli）晚于 golden 生成（21:36），HEAD 的
>   compiler.px(--emit-c) 与 golden(--bc) 不同步 → 自举证明实际失败。修复 = 重生成双轨
>   权威基线：golden/compiler.c = pxc 编 compiler.px（14935 行）→ bootstrap_prove rc=0；
>   golden/compiler.bc.dump = compiler_new bc compiler.px（30315→30314 行，含 --emit-c）
>   → VM 编译器重放 == golden 逐字节一致；bc1-12 dump golden 回归全绿。
> - **C2-2b bootstrap_prove_bc.sh**（commit e1bf26d）：自举证明升级为双轨（C 轨 compiler.c
>   由 bootstrap_prove.sh 守护；BC 轨 compiler.bc.dump 由 bootstrap_prove_bc.sh 守护）。
>   脚本全链：pxc build compiler.px → compiler_new.c → gcc 链 → compiler_new → --emit-c
>   compiler.px → compiler_vm.c → gcc 链 → compiler_vm → bc compiler.px 重放 → 对拍
>   golden/compiler.bc.dump（缓存失效判断 + --fresh）。缓存态全链 rc=0，VM 重放
>   30314 行 == golden 逐字节一致。
> - 下一步（C2 主体剩余）：bootstrap/pxc/pxi 重链 VM 版（px build 默认切 BC 产物）；
>   S3-C 收口门 vm_ab.sh v2（examples 全量 VM vs 旧轨 stdout 对拍）。

### M89-S3-C2 · golden 切换前奏：compiler.px 主链路并入 bc_emit（编译器正主 BC 发射能力）

> 完成（2026-09-08）：VM 化自举收敛进入 C2 第一步 —— 把 bc 发射能力从姊妹壳
> bc_cli.px 并入 compiler.px 主链路（M89_PLAN A1 决策"届时 bc_emit 并入 compiler.px
> 随自举 golden 同批"落地）。compiler.px 现支持 `bc`/`--emit-c` 子命令（默认仍 C
> 文本旧轨，px build 零回归）。
> - **selfhost/compiler.px**：import bc_emit.px + 声明 g_bcm + bc_basename + main
>   分支（参数含 `bc` → BCModule dump；含 `--emit-c` → emit-c 静态 C）。默认 C 文本
>   （cg_generate）路径不动。
> - **golden 双轨同步**（compiler.px 源码变化 → 权威产物再生成）：golden/compiler.c
>   = pxc 编 compiler.px 新产物（14935 行，原 10596 → 因并入 bc_emit 生态）；自举
>   证明 B.c==golden/compiler.c 逐字节 PASS（bootstrap_prove.sh rc=0）。
> - **VM 自举重放证明（C2 核心前置）**：compiler_new（= 新 compiler.px 的 C 引擎
>   产物）emit-c compiler.px → gcc 链 rtcache（含 vm.o）→ compiler_vm（VM 驱动版
>   编译器）；compiler_vm bc compiler.px → dump 30315 行 / 541,219 B 与
>   golden/compiler.bc.dump 逐字节一致（编译器字节码镜像在 VM 上编译自身，C 引擎
>   产物与 VM 产物双轨一致）。
> - **回归**：bc1-12 dump golden 全绿（bc_emit 逻辑未动）；hello C 轨产物与旧 pxc
>   逐字节零回归；hello VM（emit-c→gcc→run）输出 "HELLO, 普贤" 与 pxi/C 轨一致；
>   compiler.px parse/lint 零告警。
> - 记录：bc_cli.px 姊妹壳暂留（后续 C2 收口退役）；compiler.bc.dump 权威基线随
>   compiler.px 含 bc_emit 增大为含发射器自身的镜像（30315 行，重复运行逐字节一致）。
> - 下一步：C2 golden 切换 —— 自举证明规则更新（compiler.bc.dump 为字节码权威对拍
>   基准）；bootstrap/pxc/pxi 重链 VM 版（px build 产物 = BCModule 内嵌 C + VM 启动）；
>   vm_ab.sh v2 examples 全量 VM vs 旧轨 stdout 对拍。

### M89-S3-B5 · VM 化：并发/原语桥（spawn/chan/send/recv/select + px_spawn_ctx 修复）

> 完成（2026-09-08）：VM 并发桥打通（bc11/bc12 ALL PASS），修复一个跨轨 runtime bug。
> - **runtime**：新增 px_spawn_ctx(fn, ctx, args, nargs)（spawn_thread 透传 ctx；px_spawn 为
>   ctx=NULL 包装）；px_spawn_name 透传 PX_FUNC.ctx —— 修复致命缺陷：原只传 fn 丢 ctx，VM
>   PX_FUNC(px_vm_entry, ctx=PxVMFunc*) 起线程后 px_vm_entry ctx=NULL 直接返回 → spawn worker
>   静默不执行 → 主线程 chan.recv 死等（旧 C 产物 ctx 无关不暴露；VM 首暴露，runtime 层修复）。
> - **selfhost/bc_emit.px**：语句 ChanDecl/Send/Recv/Spawn/Select；VM 启动注册原语 native
>   chan(cap)/mutex()/rwlock()/spawn(fname,..)/chan_try_recv(ch)。
> - **验证**：bc11.px（spawn 2 worker + chan + send/recv 求和）bc11_verify PASS（"sum: 6"）；
>   bc12.px（spawn + select 绑定 recv）bc12_verify PASS（"sel: 6"+"done"）；bc1-10 dump 全不变。
> - 记录：pxi Mini 子集不支持通道（R1002）→ 对拍走旧 C codegen 轨；select 无 else 忙等对齐
>   cg retry、null 消息判未命中边缘局限、随机化公平未做；mutex/rwlock/ffi/http 构造已由
>   Call 桥覆盖，专项用例验证随 S3-B 门（vm_ab v2 + examples 并发对拍）推进。

### M89-S3-B4 · VM 化：闭包 P1 收口（m25_closure_gc 真实用例对拍 PASS）

> 完成（2026-09-08）：闭包基建（B3a 落地：Closure→LOADK PXK_FUNC、Block、lambda 入 funcs）
> 经真实用例收口 **P1（无捕获闭包）**。
> - **验证**：examples/m25_closure_gc.px 在 VM（bc_run.sh emit-c→gcc→运行）输出
>   "M25 CLOSURE GC TESTS PASSED"（rc=0）—— 覆盖闭包自引用循环垃圾 + gc() 强回收、
>   for range(300) 批量、活跃闭包不被误回收（k()==42）、assert。
> - 捕获面扫描：存量闭包用例（m25/match.px 的 fn(x){x*2}/m34 gen_lazy transform/filter）
>   均无捕获或单参 lambda → P1 覆盖现存量；真捕获 upvalue cell = P2 按计划留 S3-C。

### M89-S3-B3b · VM 化：match 模式匹配（enum 变体/literal/wildcard/guard）

> 完成（2026-09-08）：VM 化（M89）S3-B 推进，**match 表达式在 VM 跑通**。
> - **runtime**：新增 px_enum_variant（enum→px_str(variant)，非 enum→null，对齐 cg
>   type 短路）；vm.h 增 PXOP_ENUMVAR（55）+ vm.c 实现。
> - **selfhost/bc_emit.px**：Match 表达式 → if-elif 链（d 初=subject，命中 arm 覆写并跳 end，
>   全不命中返回 subject）；模式条件 PatConstructor/大写 PatBinding → ENUMVAR(subject)==变体名、
>   PatLiteral → EQ(subject, 字面量)、PatWildcard/小写 Binding → 恒真；guard 与 pattern cond
>   组合（VM 先行正确实现 guard，codegen 侧忽略 guard 记录差异待收敛）。
> - **验证**：bc10.px+dump golden，bc10_verify.sh 6 断言全 PASS（def 内 match enum 变体 +
>   wildcard、顶层 match literal）；bc10 pxi 同跑 rc=0 语义一致；bc1-9 dump golden 全不变。
> - 记录：小写 PatBinding 绑定语义未实现（对齐 codegen 现状，P2 补）。

### M89-S3-B3a · VM 化：推导式/生成器/闭包基建（ListComp/DictComp/GenExp/Closure/Block）

> 完成（2026-09-08）：VM 化（M89）S3-B 推进，**三类推导式 + 闭包/块表达式在 VM 跑通**。
> - **runtime/vm.h/vm.c**：PxK 增 PXK_FUNC（i=funcs 下标，LOADK 物化为 PX_FUNC(px_vm_entry,
>   &funcs[i])，闭包/lambda 引用）；vm_loadk 增 mod 参数；实现 NEWGEN（seq + transform/filter
>   闭包 → px_gen_lazy）。
> - **selfhost/bc_emit.px**：lambda 基建（无捕获闭包 "<closureN>" 入 funcs）；Closure 表达式 →
>   LOADK PXK_FUNC；Block 块表达式（值=最后 ExprStmt）；推导式嵌套循环展开（ListComp push /
>   DictComp rv.set，迭代变量重绑定保存/恢复，cond JMPF 过滤）；GenExp 单 for → NEWGEN 惰性
>   （transform/filter 闭包）；methodcall_slot 修正（obj 先 MOV 到新 cslot，实参区全新预留，
>   修复 dictcomp set 参数区与活跃槽重叠覆写）。
> - **验证**：bc9.px+dump golden，bc9_verify.sh 12 断言全 PASS（ListComp/cond 过滤/DictComp/
>   GenExp 惰性 + for-in 迭代/Closure(Block) 调用）；bc9 pxi 同跑 rc=0 语义一致；bc1-8 dump
>   golden 全不变零回归。
> - 记录：comp 多变量子句、GenExp 多 for 未实现（panic 不静默错）；Closure 为无捕获 P1（真
>   捕获 upvalue cell = P2 留 S3-C）；bc_emit_expr let si 与 var si 同名 E3002 已解决。

### M89-S3-B2 · VM 化：构造/类型 —— NEWSTRUCT/NEWENUM + struct/enum/const enum/impl 方法

> 完成（2026-09-08）：VM 化（M89）S3-B 全构造段推进，**类型系统三件套在 VM 上跑通 + impl
> 方法注册全局 "Type.method"**。
> - **runtime/vm.h/vm.c**：PxBCModule 增 struct 元数据表（PxStructDef）；实现 NEWSTRUCT
>   （structs 元数据 idx + 字段值连续槽 → px_struct）与 NEWENUM（N 池类型名/变体名 → px_enum）；
>   px_vm_run_module 注册全局函数跳过 '<' 开头闭包名（B4 closure 走 LOADK PXK_FUNC）。
> - **selfhost/bc_emit.px**：类型元数据收集（StructDef structs / EnumDef enums / TypeConst 递归
>   consts / impl 方法按 "Type.method" 字典序）；Field 折叠 const enum（值表达式内联）与 enum
>   变体（NEWENUM）；Call/Constructor 类型名 → struct/enum 构造；impl 方法以 "Type.method"
>   普通函数发射入 funcs；emit-c 输出 s_structs 元数据表。
> - **验证**：bc8.px+dump golden，bc8_verify.sh 11 断言全 PASS（struct 构造/字段读写/impl 方法
>   Point.sum CALLM→px_method 桥（self 绑定）/enum 变体 NEWENUM + px_eq enum 比较/TypeConst
>   折叠）；bc8 旧轨 pxi 同跑 rc=0 语义一致；bc1-7 dump golden 全不变 + bc7_verify 复跑全绿。

### M89-S3-B1 · VM 化：容器/字段/方法桥/For 迭代（S3-B 首切）

> 完成（2026-09-08）：VM 化旗舰（M89）进入 **S3-B 全构造段**，B1 落地。
> - **runtime/vm.c**：C 表容器/字段 op 全接线 —— NEWLIST/NEWTUPLE/NEWDICT（自连续槽拷贝建容器，
>   dict 仅字符串键入池对齐 codegen）、INDEX/SETIDX/SLICE、GETF/SETF/GETF_OPT（px_field/
>   px_field_set；dict 字段 = px_field→dict_get）、CALLM 方法桥（px_method：a=dst,b=obj,c=N 名,
>   fl=argc，实参=槽 b+1..）。NEWSTRUCT/NEWENUM 待 B2 类型元数据表。
> - **selfhost/bc_emit.px**：① 指令升 5 元组 [op,fl,a,b,c]（fl 预留，CALLM 存 argc；dump/C 输出
>   跳过 fl → bc1-6 golden 文本不变实测回归一致）；② 表达式 List/Tuple/Dict 字面量 → NEWLIST/
>   NEWTUPLE/NEWDICT、Index→INDEX、Slice→SLICE、Field→GETF、OptionalField→GETF_OPT、方法调用
>   obj.m(args)（callee Field）→ CALLM；③ Assign 目标扩 Var/Index/Field + 复合赋值 + `a <- rhs`
>   Append；④ For 迭代展开（对齐 cg：CALL len → 计数器 → INDEX 取元素 → body → 计数+1；
>   break/continue ctx 回填）；⑤ hoist 补 For 循环变量。enum/const enum 折叠、struct 构造 → B2。
> - **验证**：bc7.px+dump golden（容器字面量/索引读写/切片/dict 字段/CALLM 方法/For 迭代/复合赋值）
>   13 断言全 PASS；**m89_b1_parity.px 容器双轨对拍（VM=旧 C codegen）stdout 逐字节一致**；
>   bc1-6 dump golden 不变 + bc2-6 verify + hello 三轨复跑全绿。
> - 记录：bc_run.sh 自动探测 rtcache 改按 mtime 最新（避免命中 vm.c 改动前旧 vm.o）；VM 帧槽尚未入
>   GC 根面（A/B 测试规模安全，S3-D 段切精确根）；bc_emit 暂不做 M41.2 等静态语义校验（compiler 侧负责）。

### M89-S3-A1 · VM 化实现启动：发射器首个真实切片 + VM GETG/SETG/Top 运行

> 完成（2026-09-08）：VM 化旗舰（M89）进入 **S3-A 实码阶段**，A1 落地
> （A0 骨架见 docs/M89_vm_design.md）。
> - **selfhost/bc_emit.px**（AST→BCModule 发射器首个真实切片）：K 池按值去重、G 池全局名源码序收集、
>   N 池预留；函数级槽分配对齐现 codegen `_v` 编号体系（参数 0..arity-1 + hoist 局部预占（帧槽零值
>   即 PX_NULL 免初始化）+ 临时槽）；A1 子集接线 VarDecl/Assign(Var)/Return/If/While/Empty +
>   字面量（含 Unary 负字面量折叠）→ LOADK/IMM/MOV/GETG/SETG/SRCLINE/RET/RET0/HALT；if/while 用
>   JMPF/JMP 相对跳转回填；顶层合成 top 函数；逐条对齐 codegen 语义（顶层=SETG、函数内 VarDecl 局部
>   遮蔽、Assign 未声明名=hoist/全局）。**cases_bc/bc1.px → bc1.dump golden 核对全对**（K 去重/G 源码
>   序/槽号/跳转/负常量）。
> - **runtime/vm.c + vm.h**：GETG/SETG 执行（v1 经 px_get_global/px_set_global，D8 无锁化后置）；
>   px_vm_run_module 完整 Top 运行（注册非 top 函数 D2 trampoline + 跑 Top bc）。
>   examples/m89_a1/vm_selftest **ALL PASS**（Top SETG=42 可读、px_call(read) GETG=42、
>   GETG v1 读外部写回 7）。
> - 新增 bc_cli.px 验证壳（A 阶段独立 dump 入口）；compiler.px `bc` 子命令 + golden 再生成 + vm.c 入
>   rt_src_files 推迟到首个可运行字节码闭环（A2/A3）一次做（避免 codegen 全链 ~6min 重编烧多次）。
> - ⚠️ 记录：pxi 解释器不支持 import codegen 生态（i_run_program 解释合并 prog 触发 rust_str_debug
>   越界）——compiler 生态工具一律编译版运行（bc_cli 亦如此），解释验证仅限单文件。

### M89-S1/S2 · VM 化详勘预研 + 设计定稿（docs/M89_vm_prestudy.md · M89_vm_design.md）

> 完成（2026-09-07~08）：VM 化旗舰（M89）**详勘（S1，只侦察未写码）** 与 **设计定稿（S2）**。
> - S1 结论：双执行轨坐实（编译轨 px 函数=C 函数+C 递归、无显式帧；pxi=AST 树遍历），共享
>   parser/AST=VM 化最有利资产；实测编译轨**无真词法闭包**（捕获外层局部退化 px_get_global 取 null、
>   selfhost 编译器 0 个 fn_closure）；字节码选型=寄存器式 3-地址（帧槽=现 _v 编号，AST→BC 只换目标）；
>   精确 GC 改造面量化（根：帧槽+全局槽+原生桥 ≤60 处，退役整栈保守扫描）；issue28 三根因代码级坐实；
>   工期钉 **P50≈10–11 周 / P90 16 周**（S3-A 骨架/B 全构造/C 自举收敛/D 精确 GC 可横向分包）。
> - S2 设计定稿（D1–D8）：发射器=PuXian 自举（bc_emit.px）、执行器=C（runtime/vm.c）；统一函数对象
>   （PX_FUNC.fn=px_vm_entry trampoline、ctx=PxVMFunc*，px_call/px_method/px_spawn 零改动兼容旧 C 产物）；
>   显式帧栈（px→px 不回 C 递归，深递归安全）；8B 定长寄存器式 3-地址指令集（~45 op spec）；
>   全局=固定槽数组无锁读（issue28 GIL 根治）；错误 TRY 就地解包/Err 即 RET；短路 and/or/?? = JMPT/JMPF；
>   闭包=真词法捕获 upvalue cell（补编译轨缺口，分 P1/P2）；方法调用 v1 桥接 px_method、S3-C 静态化；
>   精确 GC 根=帧槽+全局槽+原生桥。S3 拆四级微步清单 + A/B 对拍 harness（vm_ab.sh 三实现两两对拍）+ BC 镜像自举证明。
> - ⚠️ 记录：M89 主体（VM 实现 S3-A..D）为季度级大工程，issue28 止血（B1/B2/B3）已先行落地（见下条），
>   根治（分代/精确 GC + 全局槽）随 M89-S3-D 吸收。

### Issue28 止血批次 B1-B3（qg-issue 28 · docs/ISSUE28_PLAN.md）

> 完成（2026-09-07）：对 M88-B 后仍阻塞 ws-approve .px 化（#47）的 **GC STW 周期尖刺 +
> 堆只涨不落 + 并发吞吐 GIL** 三瓶颈做保守 GC 框架内止血（最终根治随 M89 VM 化精确 GC）。
> - **B3**：全局符号表 M55 互斥锁 → **读写锁**——px_get_global / px_global_native / struct 方法查找
>   读锁并发（破 GIL 串行化），px_set_global / GC 根扫描写锁独占；锁序 g_gc_mu → g_globals_mu 不变。
> - **B2**：**slab 空页归还 OS**（GC sweep 后摘除并 munmap 完全空闲非头 slab，每 class 保留头防抖动）+
>   **字符串拼接中间缓冲泄漏修复**（px_add "+" / px_mul "×n" 中间缓冲 xmalloc 未 xfree —— 堆 40MB→1.1GB
>   不回吐的直接根因之一：每拼接泄漏 1 缓冲；修复前 3 轮 40 万垃圾波 RSS 逐轮 +130MB，修复后回落基线 +15MB）。
> - **B1**：**GC 延迟到请求间安全点**（服务模式越阈值置 g_gc_pending，fserve/px_pool worker 空闲
>   px_gc_poll 回收，硬上限 阈值×4 内联兜底；单线程 CLI/解释保持原内联零回归）+ **sweep 削峰**
>   （sweep 免逐趟 sigprocmask + xfree 局部性 hint：单次 STW 250-500ms → ~100-150ms）+
>   GC 摘要打印耗时；`PX_GC_INLINE=1` 可对拍还原 B1 前行为。
> - 验证：examples/issue28_b1 A/B（200 请求：B1 前每 ~4 请求 1 刺 p95 数百 ms → B1 后 200 请求仅
>   2 次 GC、p95≈10ms、无灾难卡死）；examples/issue28_b2 RSS 回落专项 PASS；回归 m82（8 项）/m83_s6/
>   m83_s1-s5/m84_s1/m85_s1-s2/m86_s0-s2 PASS。
> - ⚠️ 残余（M89 吸收）：单次 STW 仍 ~100-300ms（sweep 线性于对象数）；issue28 §7 全量验收
>   （单发 p95≤50ms / max≤100ms / 500 并发 p50≤200ms）需观音/清歌 ws-approve 隔离实测。

### M89 立项 + S0 · 版本升格 0.1.0 → 0.2.0（docs/M89_PLAN.md · VM 化旗舰里程碑启动）

> 完成（2026-09-07）：**M89 立项**（VM 化 = AST/C 递归 → 显式帧 + 平坦字节码 VM，
> 总钥匙；S0 版本升格 → S1 VM 详勘预研 → S2 设计 → S3 实现 → S4 收口）。**版本号 0.1.0 → 0.2.0**：
> 0.1.0 自 M62 沿用至 M88b，已覆盖「Rust 退役 + 全自举工具链 + 并发 64 → 1 万+ 连接事件驱动」巨变，
> 严重低配；升 0.2.0 而非 1.0.0（pre-1.0 语义化版本演进，1.0.0 留给 VM 化落地、语义冻结之后）。
> - 落地：tools/px SELFHOST_VER / compiler.px PXC_VER / interp.px PXI_VER / pxmcp.px / pxfmt.px /
>   runtime/runtime.c server dict → 0.2.0；golden/compiler.c 重链同步（10596 行）；**bootstrap/pxi 重链
>   0.2.0**（HEAD pxc 编 interp.px + 全 runtime；pxi --version = 0.2.0，hello 冒烟 rc=0）；
>   命令入口 px/pxc/pxi --version 均 0.2.0；px build 全链路冒烟 rc=0。
> - 验证：**自举证明 rc=0**（B.c==golden/compiler.c 10595 行 norm 逐字节）；回归 m82 http_serve_unix
>   8 项全 PASS；fmt/lint 干净。
> - ⚠️ 发现并记录：**bootstrap/pxc 二进制内部保持 f77732f(M72) runtime 未重链**——用当前(M88) runtime
>   重链 pxc 后，pxc 编 interp.px 确定性崩（"字符串索引越界: 0"，--full 全模块与自动裁剪版均复现；
>   pxc 编 compiler.px 正常、pxi(M88 runtime) 正常）→ 疑 M88 runtime 某改动与 compiler codegen 组合存在
>   潜在 bug，**单独立项排查**（不阻塞：用户编译路径 tools/px → bootstrap/pxc 为 f77732f-runtime 稳定版）。
>   故 pxc 二进制 --version 内部显示 0.1.0（自举编译器滞后一代）。
> - tag 基线进入 v0.2.0 时代（M89 收口打 v0.2.0-m89）。qg-issue 27 的 B 档已闭环；C 类协程按 §四·A
>   顺序（VM 化 → 协程）随 M89 详勘推进。

### M88-B-S4 · 收口：事件驱动量级压测 + 回归总闸 + tag v0.1.0-m88b（qg-issue 27 B 类）

> 完成（2026-09-07，commit M88-B-S4）：**B 类（事件驱动半同步/半异步）收口**。
> - **万级空闲连接量级实证**（examples/m88b_s4/，PX_SERVE_WORKERS=8）：**10000 空闲 keep-alive
>   同时挂载（首请求+窗口内突发复用均 200，9720/10000——其余为 http keep-alive 空闲 15s 设计语义下
>   超窗连接被服务端正常关闭），进程 OS 线程数恒 10**（≈ 池容量 + 事件循环 + 主，不随连接数涨）。
> - **SSE 长连接量级**（examples/m88b_bs3/）：1000 SSE 长连接全建立收事件、3s 后 1000/1000 存活
>   （SSE 空闲不超时）、线程恒 10、断开 500 重开 500 全成功。
> - **短连接吞吐 ≥ A 类基线**：http_serve_unix 并发 100×500 = 50000/50000 全 200、0 失败、0 err、
>   进程不崩（69.6s，与 A 类基线 63s 同量级）。
> - **回归总闸全绿**：m82 + m83_s1-s6 + m84_s1-s3 + m85_s1-s2 + m86_s0-s2 全部 rc=0（m83_s5/s6
>   收尾 `kill 0` 退出码已随 af75807 修复归 0，不再人工豁免）；m23a SSE+WS PASS；**自举证明 rc=0**
>   （B.c==golden/compiler.c 10595 行，M88 未动 compiler.px/golden）；native **301 不变**（B 类五提交
>   零触碰 px_native 注册面）。fmt/lint 0 错；worktree 干净。
> - **重链 bootstrap/pxi**（--full，9,462,024→9,467,264 B；B 类 runtime——事件驱动内核/http 交还/SSE
>   事件化/注册表动态化——全能力入解释器宿主；strings 含 PX_MAX_CONNS/PX_MAX_SSE_CONNS/PX_SERVE_WORKERS
>   实证；pxi 冒烟 rc=0）。
> - 文档同步（spec §8.22 连接语义 → 池化+事件驱动、CHEATSHEET M88 行、M88_PLAN §七）。**tag v0.1.0-m88b**
>   已 push。qg-issue 27 的 B 档（连接 1 万+、空闲不占线程）落地；C 类（用户态协程 M:N）按
>   M88_PLAN §四·A 顺序（VM 化 → 协程）另行立项。

### M88-B-S3 · SSE 长连接事件循环化 + 服务端注册表动态化（qg-issue 27 B 类）

> 完成（2026-09-07，commit 30cfbfa）：`sse_conn_worker` handler 返回后，**明文 SSE 长连接交还 IDLE
> 事件循环**（Linux epoll 照看：对端断开/异常 → 统一关闭清理；**SSE 空闲不超时**，区别于 http keep-alive
> 15s tick），worker 返回释放取新 job = **SSE 长连接不再占线程**；TLS / 非 Linux / fd 超限自动降级原
> 阻塞保持路径（功能不降）。服务端 SSE 注册表 `g_sse_conns` 256 定长 → **动态容量表**（env `PX_MAX_SSE_CONNS`
> 默认 4096 夹取 [64,65536]）。事件循环按连接类型分流；`sse_close` 按 IDLE/ACTIVE 分流关闭、`sse_send`
> 写失败统一清理（新增 `px_evc_detach`/`px_evc_is_idle`）；http_stream/stream_takeover 共享动态注册表。
> 验证：bs3 专项 **1000 SSE 长连接全建立收事件、3s 后 1000/1000 存活（不被 15s tick 误关）、服务线程恒
> 10、断开 500 重开 500 全成功**；回归 m23a SSE+WS、m82 http_serve_unix、m83_s6 同端口流式 SSE 全绿。

### 回归脚本卫生修复 · m83_s5/s6 verify.sh 收尾 trap `kill 0`（M84-S4 起记录不修，今根治 · 不占里程碑号）

> 完成（2026-09-07）：`examples/m83_s5/verify.sh`、`examples/m83_s6/verify.sh` 收尾 `trap ... EXIT`
> 内为裸 `kill $SRV_PID [$SMTP_PID]`。当服务段正常结束、PID 变量已置 0 后脚本 exit，trap 触发 →
> `kill 0`（bash/POSIX：**pid=0 的语义是向当前进程组广播 SIGTERM**，不是"杀 PID 0"）→ 脚本连同自身
> 进程组被信号终止 → 内容断言全 PASS 但退出码非 0。此现象自 **M84-S4** 起每轮收口都在 CHANGELOG
> 记录为"不修"（内容全过、仅退出码失真，总闸靠人工豁免）。本次给 trap 内 kill 加 `[ "$PID" -gt 0 ]`
> 守卫根治：M84/M85/M86/M88 四个里程碑累积的"退出码需人工豁免"噪音消除，回归总闸退出码恢复可信。
> 验证：m83_s5、m83_s6 重跑，内容断言全 PASS 且 **EXIT=0**。纯测试脚本卫生修复，不碰产品代码、
> 不动断言语义、不改被测行为，不占里程碑号、不打 tag。

### M88-A 立项 · runtime 并发模型演进 A/B/C（qg-issue 27）

> 立项（2026-09-07）：qg-issue 27（清歌 ws-approve .px 化 #47 压测/线上事故：http_serve_unix 高并发崩溃）
> → runtime `px_spawn` 受 GC 线程表固定 64 槽限制，槽满 `px_error`→`exit(1)`；http_serve/sse_serve
> accept 每连接 spawn → 64 并发即崩。M88 = A（线程池止血，已开工）→ B（事件驱动）→ C（用户态协程 M:N）
> 三档完整演进。规划见 docs/M88_PLAN.md。

### M88-S1 · GC 线程槽动态化（qg-issue 27）

> 完成（2026-09-07，commit c530612）：GC 线程表固定 64 槽 → 动态容量（env `PX_MAX_THREADS`，默认 1024，
> 夹取 [64,4096]），gc_init_env 一次性按上限分配稳定表（无 realloc 指针移动，规避信号处理器/GC 遍历竞态）。
> 17 处遍历/3 处报错文案改动态。验证：200 并发 spawn 全过（旧 64 崩）、200 线程并发 GC 冒烟稳定。

### M88-S2 · 函数式 serve 连接线程池 + 服务端健壮性三修复（qg-issue 27）

> 完成（2026-09-07）：http_serve/http_serve_unix/sse_serve accept 从"每连接 px_spawn"改为投递
> **函数式 serve 常驻连接池**（fserve：accept → (fd,kind) 环形队列，worker 调 http_conn_worker/sse_conn_worker；
> 队满阻塞背压，**服务进程永不因 spawn/槽满 exit**）。env `PX_SERVE_WORKERS`（默认 256）。
> 附带修复两个"进程悄然消失"根因：① 忽略 SIGPIPE（http_conn_worker 裸 send 到已断开连接默认杀进程）；
> ② bi_sleep EINTR 续睡（nanosleep 不在 SA_RESTART 清单，主线程 sleep 被 GC 信号打断提前返回 → main 结束）。
> 回归：m82 http_serve_unix 专项 + m23a SSE+WS + 并发压测（100×500 全 200 0 失败，进程不崩）。

### M88-S3 · 收口：重链 pxi + 自举证明 + 回归总闸 + 文档/issue 归档（qg-issue 27）

> 完成（2026-09-07，commit M88-S3）：**全能力重链 bootstrap/pxi**（`tools/pxc build --full
> selfhost/interp.px`，9,457,456→9,462,024 B；解释器宿主内嵌 M88 runtime → `pxi` 解释路径同样
> 获得线程槽动态化/连接池/SIGPIPE·sleep 健壮性修复；strings 含 PX_SERVE_WORKERS/PX_MAX_THREADS 实证）。
> 自举证明 **rc=0**（bootstrap/pxc 编 compiler.px == golden/compiler.c 10595 行逐字节 —— M88 未动
> compiler.px/golden/pxc，runtime 改动对编译器零影响实证）；native **301 不变**（未动 px_native 注册面）。
> 回归总闸：m82 + m83_s1-s4 + m84_s1-s3 + m85_s1-s2 + m86_s0-s2 **干净全绿**；m83_s5/s6 内容断言
> 全 PASS（multipart/SMTP 回环、同端口流式 SSE/断连不崩）—— 二者 verify.sh 收尾 `trap ... EXIT` 内
> `kill $SRV_PID` 在 PID 已置 0 时退化为 `kill 0`（进程组自杀）致退出码非 0，系 **M84-S4 起记录不修**的
> 历史边界（非 M88 回归）。M88 专项抽验：s1_spawn_200（200 并发 spawn 全 done）+ http_serve_unix
> 并发 100×500 = **50000/50000 全 200、0 失败、0 err、进程不崩**（53s）。px fmt/lint 质量门 0 错。
> 文档同步（spec §8.22 连接语义改池化 + env PX_SERVE_WORKERS/PX_MAX_THREADS、CHEATSHEET M88 行、
> M88_PLAN 状态、examples/m88_s3/press_unix.go 注释 # → // 修正）。qg-issue 27 归档 done/。tag v0.1.0-m88。

### px --version 对齐 go/python 单版本号 + 发布号 VERSION（补丁 · 不占里程碑号 · 随下次发版携带）

> 完成（2026-09-06，commit 39e01bb，小改动不立项不单独发版）：`px --version` 从 `px 0.1.0 (普贤 PuXian · selfhosted M-B9a)` 简化为**单版本号一行**（对齐 go `go1.26.6 linux/amd64` / python `3.9.25` 心智，应 m86 发版后用户追问简化）。
> - **用户可见版本 = 发布号**：发布包由 make_release.sh 从 git tag 生成 `VERSION`（如 `0.1.0-m86`）→ `px --version` 输出 `px 0.1.0-m86`（一眼可判新旧、与 dnf 包名 `0.1.0-1.m86` 对应）；源码仓直跑无 VERSION → 输出语义版 `px 0.1.0`。
> - **M-B9a 归位内部**：自举内核血统标识不再出现在用户可见输出（保留 tools/px 注释 + compiler.px/interp.px 常量，供内部诊断；bootstrap 二进制版本串不动 → 自举/对拍零影响，无需重跑自举证明）。
> - **机制（最小改动）**：tools/px 新增 RELEASE_VER（读 `$PXC_HOME/VERSION`，缺失退 SELFHOST_VER，--version 与 usage 头部统一）；make_release.sh 打包生成 `VERSION`（`${TVER:-${VER}-${MILESTONE}}`）；packaging/puxian.spec %install 收录 VERSION。
> - **验证**：源码直跑 `px 0.1.0` / 模拟发布包 `px 0.1.0-m86` / usage 头部同步 ✅；examples/m86_s0/verify.sh + CONTRIBUTING.md 断言同步新格式（兼容源码/发布两态）。
> - 已发布 v0.1.0-m86 rpm 不含 VERSION（本改动在其后）→ 已装机 `px --version` 待下一发布自动带发布号。

### M86 立项 · px build 按需自动裁剪（路线甲）+ 命令正名 px（qg-issue 25）

> 立项（2026-09-06）：M85 交付后用户连续追问「能否按需自动裁剪（不必手动 --min）？或反转默认 min + --full？」「pxc 能否改为 px？有没有 --help？」→
> 三问实测：① `--help` 已有（--help/-h/无参均完整 usage、exit 0），但 M85 新 flag（--min + 10× --no-*）未同步进 usage 文本（grep 命中 0，文档缺口）；
> ② pxc→px 可行（/usr/bin/pxc = symlink → tools/pxc bash 前端；M71-S4 readlink 自发现、逻辑不依赖 argv[0]；改名面=脚本名 + usage/version 文本 + spec + 文档，pxc 留兼容别名）；
> ③ 用户拍板**路线甲（自动裁剪）**，乙案（默认 min）剔除（自举/工具链需全量、破坏向后兼容、与编译型惯例倒置）。
> M86 = **S0**（命令正名 px + help 补齐 M85 开关 + pxc 兼容别名 + spec 双装 + 文档入口更新）→ **S1**（selfhost 语义打点收集「被引用 native 集」，旁路导出不扰 codegen；import 递归；解析异常退全量）→ **S2**（裸 `px build` 自动折 `--no-xxx` 裁剪集复用 M85 链路 + `--full/--max` 逃生舱 + 显式 flag 优先级 > 自动）→ **S3**（收口：重链 + 自举证明 + 回归总闸 + qg-issue 25 归档 + tag v0.1.0-m86 待令）。
> 不改语言语法/现有 native 语义；native 总数 **301** 不变；bootstrap 发布物全能力。执行规划 `docs/M86_PLAN.md`（S0→S3）。

### M86-S0 · 命令正名 px + help 补齐 + 安装链路双装（qg-issue 25）

> 完成（2026-09-06，commit M86-S0）：**官方命令名 `pxc` → `px`**（pxc 保留兼容别名）。
> - **改名落盘**：`tools/pxc` → `tools/px`（bash 前端 git mv），`tools/pxc` = symlink → `px`（软链入库，历史脚本/文档路径照跑）；脚本内 usage/version/错误提示/注释 px 化（`bootstrap/pxc` 编译器真名保留不误伤）；`cmd_mcp` 注入 `PX_PXC=$PXC_HOME/tools/px`。
> - **安装链路双装**：`packaging/puxian.spec` 装 `/usr/bin/px` + `/usr/bin/pxc → px`；`tools/install.sh` 双软链 `bin/px` + `bin/pxc`；`tools/make_release.sh` RELEASE 模板与冒烟自检 px 化。
> - **help 补齐**：`px --help/-h/无参` usage 含 px 全 13 子命令 + M85 裁剪开关（--min + 10× --no-*）+ M86-S0 pxc 别名说明；`px --version` → `px 0.1.0 (普贤 PuXian · selfhosted M-B9a)`，pxc 别名输出等价。
> - **文档入口同步**：README.md / README.en.md / docs/PUXIAN_CHEATSHEET.md / docs/spec.md / docs/MINI_SUBSET.md 用户命令与 tools/px 路径 px 化（pxc 别名说明保留；历史 M*_PLAN/ROADMAP 原样）。
> - **零漂移实证**：examples/m86_s0 verify **14/14 PASS** —— 默认 build **9,010,184 B**（9.0M 基线不变）/ `--no-quic` 3,929,808 B（M85 基线不变）/ pxc 别名 build 等价 / spec+install.sh+make_release 双装断言全过。
> - `.px` 工具（pxfmt/pxlint/pxdoc/pxtest/pxbench/pxlsp/pxmcp）usage 显示名保留各自自举二进制真名（px 子命令错误提示已在 tools/px 层 px 化，直接调用场景名实相符，侦查后不改为宜）。

### M86-S1 · 引用集收集 `px refs` + native_mod_map（qg-issue 25）

> 完成（2026-09-06，commit M86-S1）：**自动裁剪的"引用采集"层落地**。
> - **runtime/native_mod_map.txt**（112 个可裁 native 名=模块）：从 runtime.c px_register_builtins 各 `#ifndef PX_NO_<MOD>` 宏块 + 模块文件注册点提取（sqlite/ws/zip/xml/aes/rsa/ed25519/route/zlib/h2/quic）；生成器 `tools/gen_native_map.sh`（awk+grep 零依赖，`LC_ALL=C sort` 与入库 diff 一致可重生成校验）。
> - **`px refs <file.px>`**（tools/px 新子命令，usage 同步）：pxc build 编译 C 产物 → 提取 `px_get_global("name")` 引用名集合（每行一名，排序去重）。**实现选择（实践修正）**：M86_PLAN 原方案"编译器内打点收集"经实测否决——打点版自举 pxc 编 compiler.px 自身触发 runtime 字符串越界（自举不收敛风险），回滚零改动方案；C 产物静态提取更优：提取的是**最终代码生成结果**（import 递归展开后全量、零漏报零误报、条件分支静态覆盖），且**零编译器/零 golden 改动、零自举风险**（bootstrap/pxc 与 golden 均未动）。
> - **失败退全量**：解析/编译失败 → px refs 非 0 + 提示"自动裁剪退全量"（保编译成功不背锅）。
> - **verify examples/m86_s1 19/19 PASS**：hello 核心集 / 条件分支静态收集（if 内 ws_connect）/ import 递归（多文件 aes + stdlib 库内部 len/zip_lists）/ 解析异常退全量 / 9 模块代表 native 全命中。

### M86-S2 · 自动按需裁剪：裸 `px build` = 引用集最小 + `--full/--max` 逃生舱（qg-issue 25）

> 完成（2026-09-06，commit M86-S2）：**用户目标「必须 --min？不如默认自动 / 想全面用 --full」落地**。
> - **cmd_build 自动判定**：无 `--full/--max` 时，从**已生成的 C 产物**（主编译一次性 codegen）提取 px_get_global 引用集 → native_mod_map 反推被引用模块 → 未引用模块自动补裁（`--no-xxx` 复用 M85 cuts/宏/缓存 key 隔离链路，零额外 codegen）。
> - **优先级**：显式 flag（--no-xxx / --min / --no-quic / --target 物理约束）> 自动；自动只补"未显式声明 + 引用集未命中"模块。`--full/--max` = 全能力逃生舱（可组合显式 --no-xxx = 显式裁剪其余全能力）。
> - **实测**：裸 `px build hello` **2,713,472 B**（自动最小 = M85 --min 档；hello 只引 core → 11 模块全裁）/ `--full` **9,010,184 B**（精确全能力基线）/ `--max` 同 / sqlite_dep 裸 **3,759,248 B**（引用 sqlite → 保留，运行 `sqlite_open(":memory:")=1` 无 R1001）/ `--no-sqlite` 显式覆盖自动保留 → R1001 未定义 sqlite_open / allmod（引 9 模块）裸 9,001,304 B（保留被引用模块，route/h2 未引用裁掉）。
> - **语义变更**：M85"默认不传 flag = 全能力零漂移" → M86-S2 起**默认裸 build = 自动最小**（M86 立项目标）；全能力需 `--full/--max`。受影响回归适配：m86_s0 verify 默认断言改 `--full`（改名零漂移验证意图不变）。
> - **verify examples/m86_s2 11/11 PASS**（裸最小/--full/--max/sqlite 保留/显式优先/allmod 保留/map 与生成器 diff 一致）。

### M86-S3 · 收口：回归总闸 + 文档同步 + qg-issue 25 归档（qg-issue 25）

> 完成（2026-09-06，commit M86-S3）：**回归总闸 15 批全绿**（m82_http_serve_unix + m83_s1–s6 + m84_s1–s3 + m85_s1–s2 + m86_s0–s2）——m85_s1/s2、m86_s0 适配 M86-S2 新默认语义（M85 显式裁剪档统一 `--full` 前缀保原意；m85_s1 9/9、m85_s2 8/8、m86_s0 14/14、m86_s1 19/19、m86_s2 11/11）。
> - **自举证明 rc=0**（bootstrap_prove：B.c == golden/compiler.c 10595 行逐字节一致）——S1 选型「C 产物静态提取（零编译器/golden 改动）」的零自举影响实证；native **301** 不变。
> - **文档同步**：README.md / README.en.md CLI 表（裸 build 自动最小 + --full/--max + px refs）、CHEATSHEET M86 摘要、spec.md M86 规格段、pxi_native_diff px 化；px fmt 新增样例全过、bash -n 语法过。
> - **qg-issue 25 归档 `done/25-px-auto-prune/`**；00-README 更新（Issue 1–25 全部合入/归档，仅 14-W1b ⏸️ + 13-R2 自持）。
> - tag v0.1.0-m86 **待用户令**（规划默认推 GitHub release + dnf 仓库 m86）。

### M85 立项 · pxc build 编译产物按需裁剪（qg-issue 24 · 方案 B 细粒度模块开关）

> 立项（2026-09-06）：用户提问「编译产物都在 9M 以上，能去掉未使用模块吗」→ 真机实测 hello.px
> 默认 build **9,010,184 B** / `--no-quic` **3,929,808 B**（−56%）→ 源码级解剖定位根因：tools/pxc
> （bash 装配器）无条件全链 runtime 全部 .o + sqlite3.o + mbedtls 三 .a + libz.a，且 runtime.c
> `px_register_builtins()`（L5313）集中注册使 ld 无法丢弃未用 .o → 新建 qg-issue **24** 并立项
> **M85 = 方案 B 细粒度模块开关**：把 PX_NO_QUIC（M57-S4）单开关模式泛化为模块开关集
> （--no-sqlite/--no-ws/--no-tls/--no-zip/--no-xml/--no-aes/--no-rsa/--no-ed25519/--no-route 等，
> 与 --no-quic 正交可组合）+ runtime.c 各模块注册段 #ifndef PX_NO_\<MOD\> 包裹 + tools/pxc
> 链接/缓存 key 按开关集裁剪；目标产物 9.0M→**2~4M**，默认全能力 build 行为零漂移；
> 方案 C（--gc-sections 自动死代码裁剪）二期评估不承诺。执行规划 `docs/M85_PLAN.md`（S1→S3）。
> 不改语言语法/现有 native 语义；native 总数 **301** 不变。

### M85-S1 · 模块裁剪开关集落地：runtime.c 宏包裹 + tools/pxc 细粒度裁剪（qg-issue 24）

> 实施（2026-09-06）：S1 首步依赖矩阵侦查定稿（修正规划假设）→ runtime.c 14 处宏包裹 + tools/pxc 重构。
> - **侦查修正**：`--no-tls` 从开关集**剔除**——mbedtls 非仅 TLS 面，runtime.c 主文件 104 处直接调用（http/https/wss/tls_server 核心）+ aes/rsa/zip/ws(sha1) 均以 mbedtls 为底座，恒链不可独立裁；miniz 被 runtime.c(gzip 内联核心)+zip 共用恒链；tls_server/session/basic_auth 属 runtime.c 内联（M27）不随 --no-ws 裁。sqlite 最净（runtime.c 对 sqlite3 API 零直接调用）。
> - **runtime.c 宏包裹**（零漂移，默认无 -D 全能力路径编译通过）：sqlite（注册 px_set_global+ffi_register 两段）/ ws / zip / xml / aes / rsa（两段）/ ed25519 / route（注册段 + http dispatch 调用段）/ zlib / h2（TLS ALPN + h2c 两调用段）共 **10 模块 14 处** `#ifndef PX_NO_<MOD>`。
> - **tools/pxc 重构**：新增 `--no-sqlite/--no-ws/--no-zip/--no-xml/--no-aes/--no-rsa/--no-ed25519/--no-route/--no-zlib/--no-h2`（与 --no-quic 正交可组合）；rt_src_files/rt_key/rt_cache_compile/rt_ensure 改以 **cuts 集合**驱动——被裁模块源不复制不编译、`-DPX_NO_<MOD>` 宏自动注入、**缓存 key 纳入 cuts**（不同裁剪组合缓存隔离，实测 4 组合各独立 .rtcache 无串用）、链接去 sqlite3.o / libz.a（按开关）。
> - **verify（examples/m85_s1，PASS=9 FAIL=0）**：默认 build **9,010,184 B**（M84 基线逐字节零漂移）/ `--no-quic` **3,929,808 B** / `--no-quic --no-sqlite` **2,884,072 B** / 全裁（--no-quic+10 模块）**2,713,472 B**（9.0M→2.7M，**−70%**）；裁剪态缺 native 调用 → 运行时明确报错（未定义变量，非崩溃，R1001 叙事）；全裁态 http_get 等核心 HTTP native 保留可用。

### M85-S2 · --min profile + target 折叠叠加 + 文档同步（qg-issue 24）

> 实施（2026-09-06）：S1 基础上补 profile 与组合验证。
> - **`--min` profile 预设**：聚合 `--no-quic` + sqlite/ws/zip/xml/aes/rsa/ed25519/route/zlib/h2 全裁，
>   一条命令出最小化产物 **2,713,472 B**（9.0M → 2.7M，−70%）；与手写全裁组合逐字节同体积（等价性断言）。
> - **target 折叠 + 裁剪叠加**：`--target x86_64 --no-sqlite` 路径验证（quic 全能力 + 去 sqlite，7,968,768 < 9.0M 基线）；
>   aarch64/armv7/riscv64 真机交叉组合由 CI 覆盖（本机无 musl 交叉工具链）。
> - **文档同步**：README（CLI 表 `pxc build --min` 行）、PUXIAN_CHEATSHEET（工具链行 M85 裁剪说明）、
>   pxi_native_diff（M85-S1 更新段：模块裁剪宿主缺 native → R1001 口径，native 计数以全能力 301 为准）、
>   spec.md（M57-S4 裁剪叙事处补 M85 模块开关泛化注记）。
> - verify（examples/m85_s2，PASS=8 FAIL=0）：--min 体积/运行、--min==全裁、target+裁剪、缺 native R1001、核心 http 保留。

### M85-S3 · 收口：全能力重链 pxi + 自举证明 + 回归总闸 + qg-issue 24 归档（qg-issue 24）

> 收口（2026-09-06）：runtime.c 宏包裹文本变更 → 按 M84-S4 惯例全能力重链 **bootstrap/pxi**。
> - **重链 + 零漂移实证**：tools/pxc build selfhost/interp.px → 产物 **9,457,456 B 与 m84 入库 bootstrap/pxi 同字节**（仅 .note.gnu.build-id 20B 差异，gcc 链接非确定性）——宏包裹在无 -D 全能力路径对 pxi **零语义影响**的最强证明；功能冒烟（hello/sha256/sqlite/dns_lookup 解释模式）全过；已 cp 覆盖 bootstrap/pxi 提交（build-id 更新记录）。
> - **自举证明**：selfhost/bootstrap_prove.sh rc=0（B.c == golden/compiler.c 10595 行逐字节；compiler.px 未动）。
> - **回归总闸 12 批内容全 PASS**：m82_http_serve_unix + m83_s1–s6 + m84_s1–s3 + m85_s1–s2 —— m83_s5/s6 内容断言全 PASS，仅 verify.sh 收尾 trap 在工具进程管理下 signal 边界（同 M84-S4 记录，CI 环境正常，不修）。
> - **qg-issue 24 归档 `done/`**（00-README 更新：Issue 1–24 全部合入/归档，仅 14-W1b ⏸️ + 13-R2 自持）；docs/M85_PLAN.md 全程记录。
> - tag v0.1.0-m85 **待用户令**（规划默认推）。

### M84-S4 · 收口：重链 bootstrap/pxi + 全量回归 + qg-issue 21/22/23 归档（tag v0.1.0-m84）

> 收口（2026-09-06）：M83-S2…M84-S3 的 runtime 变更（AES-ECB/gzip 暴露/ed25519/RSA-PKCS1v15/http_stream/CT 修复/hmac_sha256/dns_lookup）此前均未重链解释器 → 本批重链 **bootstrap/pxi**（9,425,360 → 9,457,456 字节，git 提交新 ELF）——pxi 解释模式现可调 M83/M84 全部新 native。
> - **重链 + 自举证明**：tools/pxc build selfhost/interp.px → 覆盖 bootstrap/pxi；bootstrap_prove.sh rc=0（compiler.px 未动，B.c == golden 10595 行逐字节）。pxc 等编译器类 ELF 源码未动无需重链（产物每次编译现链最新 runtime）。
> - **双模式抽查**：pxi run == pxc build —— hmac_sha256 RFC4231 TC2（5bdcc146…）与 dns_lookup localhost（[::1, 127.0.0.1]）解释/编译逐字节一致。
> - **全量回归总闸全绿**：m82（http_serve_unix）+ m83_s1–s6 + m84_s1–s3 每批独立 verify PASS —— m83_s5 multipart 上传卸 Content-Length hack 仍过（CT 修复后 runtime 自动补 CL）、m83_s6 SSE 同端口零回归、m84_s1 单 CT 6 场景断言、m84_s2 RFC4231/Go/TC3 对拍、m84_s3 公网 www.qq.com 与 Go net.LookupIP 交集≥1。
> - **native 301**（gen_native_table.sh 幂等，native_index/CHEATSHEET 已同步）；qg-issue **21/22/23 归档 `done/`**（00-README 更新：Issue 1–23 全部合入/归档，仅 14-W1b ⏸️ 挂起 + 13-R2 业务自持）；docs/M84_PLAN.md 全程记录。
> - 附带记录：m83_s5/s6 verify.sh 收尾 trap 在工具进程管理下 signal terminated（内容断言全 PASS，CI 环境正常），属既有脚本边界，不修。

### M84-S2 · hmac_sha256 native + sha256 bytes/含 NUL 增强（qg-issue 21，GAP-HMAC-1）

> 立项（2026-09-06）：qg-issue 21 —— bs-safeip 腾讯云 API 3.0 需 4 级 HMAC-SHA256 链（TC3），全仓无
> hmac native；sha256 仅 ASCII（bi_sha256 用 strlen → 含 NUL/bytes 载荷截断）。mbedtls 层已有内部
> HMAC 实现（M37 S3/SigV4 同源 px_hmac_sha256）→ 纯暴露成本。native 299→**300**（+1 hmac_sha256）。
> - **`hmac_sha256(key, msg)`→hex**：key/msg 均收 str|bytes 二进制安全可含 NUL（数值自动字符串化）；
>   key>64B 自动先 sha256（标准 HMAC 块处理）。腾讯云 TC3 / AWS SigV4 / webhook 验签 / JWT HS256 解锁。
> - **`sha256` 原地增强**（+0 native，输出语义不变）：bdata/blen 替代 val_cstr+strlen——bytes 与含 NUL
>   载荷全哈希（旧截断 bug 修复）；纯 ASCII str 输出与旧版逐字节一致零回归。
> - verify（examples/m84_s2_hmac，全绿）：RFC4231 TC1-4/6 固定向量（TC1/TC2 官方期望硬编码 + Go
>   crypto/hmac 全量对拍）、文本/二进制 key×文本/二进制 msg（含 NUL）Go 互通、sha256(binmsg 含 NUL)==
>   Go sha256.Sum256、腾讯云 TC3 4 级链（kDate→kService→kSigning→signature）端到端对拍 Go、篡改敏感性。
> - 回归：m83_s3（ed25519）/ m83_s4（RSA）/ m84_s1（http CT）全绿。

### M84-S3 · dns_lookup 原生域名解析（qg-issue 22，GAP-DNS-1）

> 立项（2026-09-06）：全仓无 getaddrinfo/A 记录查询 native —— 守护类模块（bs-safeip util.px
> resolve_ips 每轮解析守护域名）只能 `getent ahostsv4` 外部命令文本切分（脆弱、依赖 glibc 命令）。
> 底层 glibc getaddrinfo 现成 → 纯暴露成本。native 300→**301**（+1 dns_lookup）。
> - **`dns_lookup(domain)`→list[str]**：getaddrinfo（AF_UNSPEC+SOCK_STREAM，与 hconnect 同口径）返回
>   A+AAAA **全量地址**（顺序即解析器返回序）；解析失败（NXDOMAIN/超时/空域名/无地址记录）返回
>   **Err("dns: <host>: <原因>")**——调用方可 `is_err()`/`?` 判定，与「解析成功但空」天然区分。
> - verify（examples/m84_s3_dns，全绿）：localhost 确定性断言（含 127.0.0.1，/etc/hosts 语义）、
>   .invalid NXDOMAIN/空域名 → Err 可判定、公网域 www.qq.com（A+AAAA）与 Go net.LookupIP 期望交集
>   ≥1（DNS 轮询容忍，非精确相等）且返回 IPv6、重复调用集合稳定；断网自动跳过公网项仅本地断言。
> - 回归：m82 / m83_s3 / m83_s4 / m84_s1 / m84_s2 全绿。

### M84-S1 · http_request/http_unix 双 Content-Type bug 修复（qg-issue 23）

> 立项（2026-09-06）：http_request 显式带 Content-Type 且无 Content-Length 时，默认 urlencoded CT 照加
> → 双 Content-Type，腾讯云等严服务端拒收。根因：默认头补充判定挂在 Content-Length 上而非 Content-Type。
> - **CT/CL 判定解耦**（runtime.c bi_http_request + bi_http_unix）：已带 Content-Type 不再追加默认 CT；
>   默认表单头仅完全未指定 CT 时补；CL 仅缺失时补。新增 px_extra_hdr_has 行首匹配（避免 strcasestr 误伤
>   X-Content-Type 等含子串头）。GET 无 body 无 CT/CL 不变。
> - verify（examples/m84_s1_http_ct，全绿）：6 场景单头断言（无头默认表单回归 / json 显式 CT 单头+自动
>   CL / multipart 显式 CL 保留 / GET / X-Custom 相邻不干扰 / http_unix 同步）。
> - M83-S5 multipart 上传的 Content-Length 显式 hack 卸除（修复后 runtime 自动补齐）。native 保持 299。

### M83-S6 · http_stream 同端口流式 SSE + M83 全量收口（qg-issue 19，GAP-SRV-SSE；tag v0.1.0-m83）✅

> 立项（2026-09-06）：M83-S6 = Issue 19 —— http_serve/sse_serve 两路径不相通：http_serve handler 一次性应答、
> sse_serve 独立端口专用不接受 http 路由。api-server OpenAI 兼容 /stream（同一端口 JSON+SSE 共存）无优雅绕过。
> 形态决策 B：**http_stream(path, on_connect) 流式路由注册**——http 服务面与 sse_send/sse_close 通道复用，
> 改动面最小（http_serve 与 http_serve_unix 共享 http_conn_worker → 一处改两入口通）。native 298→**299**（+1）。
> 收口：qg-issue 16/17/18/19/20 全部合入（W1b Windows 平台分流保持挂起），tag v0.1.0-m83。

- **S6-1 新增 native `http_stream(path, on_connect)`**（runtime.c）：http_serve/http_serve_unix 同端口流式路由表
  （GET + path 精确匹配，流式路由优先于普通 handler）。命中 → 连接包装 PxConn 注册进 g_sse_conns（与 sse_serve
  同注册表/锁）→ req 注入 conn id → 写 SSE 响应头（text/event-stream + Connection: close）→ 调 on_connect(req)；
  on_connect 内 `sse_send(req["conn"], chunk)` 逐块推送（每块即写即刷），可 `sse_send` dict {event,data,id,retry}；
  可提前 `sse_close(conn)`；on_connect 返回后 runtime 自动注销 + 关闭连接（curl 自然 EOF）。
  handler fn 存全局表（GC 根）经 `__stream_fn_<i>` 取回；全局表 g_stream_mu/g_stream_routes 前置定义于
  http_conn_worker（避免 static 使用前未定义）。限制：明文 HTTP / HTTP-over-unix（http_conn_worker 面）；
  px_serve（应用平台独立 worker）暂不接入（文档注明，后续扩展）。
- **S6-2 验证（examples/m83_s6 verify 5 步全绿）**：同端口 /json（普通 JSON handler 返回 json-ok）与 /stream
  （3 chunk 间隔 100ms + event done + data: [DONE]）共存；TCP 与 Unix socket 双入口 curl -N 均收到完整 SSE 帧
  序列 + Content-Type: text/event-stream + 自然 EOF（on_connect 返回即关闭）；sse_serve 独立端口旧行为回归
  （legacy-1/2 帧）；客户端中途断开（--max-time 0.3）后服务端 health 仍 ok（不崩）。
- **S6b 收口**：native_index 298→299（gen_native_table.sh 重跑）+ CHEATSHEET（SSE native 面 + 计数）+ CHANGELOG +
  M83_PLAN 标记 + qg-issue 16-20 归档 done/ + 自举证明（bootstrap_prove.sh rc=0，产物与基准逐字节一致）+
  回归总闸 m82 + m83_s1-s5 全绿。

### M83-S5 · stdlib 四库 std.html / cookiejar / multipart / smtp（qg-issue 20-L1）✅

> 立项（2026-09-06）：M83-S5 = Issue 20-L1 —— HTML5 / cookie jar / multipart 生成 / SMTP 四个纯 .px 标准库
> （L1 随发布包分发；零 native 改动）。stdlib 9 → **13 库**，registry 官方包同步 13 个。
> ⚠️ 验证中实测并记录 **Issue 23**（http_request 显式 Content-Type 仍追加默认 urlencoded → multipart 上传须
> 带 Content-Length 绕过，见 m83_s5_client 注释）——真 bug 修复留 qg-issue 23 单独处理（M83 边界不动 runtime 语义）。

- **S5-1 `std.html`**（stdlib/html.px + registry/html/0.1.0）：简化 HTML5 容错解析——`html_parse(text)→DOM`
  （坏标签自动纠正/隐式闭合/孤立结束标签忽略/script·style raw text 不进正文/实体最小解码 &amp;&lt;&gt;&quot;&#39;&nbsp;/
  裸 `<` 容错）、`html_text(node)` 剥标签取可见正文、`html_query(root, sel)` tag[.class][#id] 简单选择、
  `html_children/html_attr/html_tag/html_escape`。DOM 节点统一 {tag, attrs, text, children}（#root/#text/#comment）。
- **S5-2 `std.cookiejar`**（stdlib/cookiejar.px）：会话 Cookie——`cj_new()→jar`、`cj_update(jar, resp_headers)`
  解析 Set-Cookie（domain/path/max-age/secure/httponly/samesite，Max-Age=0 立删、Max-Age 换算绝对到期秒）、
  `cj_header(jar, url)` 按 domain（含 ".example.com" 后缀）·path 前缀·secure(https) 匹配生成 "n=v; n2=v2"、
  `cj_len/cj_clean`。language 限制（http_request 同名响应头覆盖 → 多 Set-Cookie 仅最后一个）文档化。
- **S5-3 `std.multipart`**（stdlib/multipart.px）：`mp_encode(fields, files)→{body: bytes, content_type, len}`
  （自动 boundary=`os_random_hex`，files 值 {filename, data: str|bytes, type}，字段 int/float/bool 自动转 str）。
- **S5-4 `std.smtp`**（stdlib/smtp.px）：轻量 SMTP 客户端——`smtp_send(host, port, from, to, msg, opts?)→bool`
  + `smtp_try→{ok, err}` 诊断（EHLO/MAIL FROM/RCPT TO/DATA/QUIT 全流程 + 可选 AUTH LOGIN base64；DATA 行首点
  转义按 \n 归一行再转；tcp_send 循环发满；消息构造 From/To/Subject/Date/MIME-Version/Content-Type）。
  STARTTLS 暂不支持（语言无客户端 TLS 包装 native，文档注明）。
- **S5-5 验证（examples/m83_s5 verify 6 步全绿）**：libs_test 双模式（pxi run + pxc build 输出逐字节一致）；
  multipart 上传回环——px mp_encode body ↔ px http_serve（px_parse_multipart）解析字段/文件/二进制逐字节保真
  （Content-Length 显式带以绕过 Issue 23）+ **curl -F 对照**（同一服务端解析出同 token/me.png/内容 → 证明
  与标准 multipart 互通）；SMTP 真发信回环——本地 fake_smtp.py（python）收信断言 Subject/From/To/正文/行首点
  转义 "..keep this line"/AUTH LOGIN 凭据 base64。
- stdlib 全库 fmt/lint 0 错 0 警告；registry 4 库镜像 + registry/README（9→13 官方包）；gen_ecosystem.px LIBS
  白名单 9→13 重跑 ecosystem_index.json（13 libs）；ECOSYSTEM/CHEATSHEET 标准库表 9→13。

### M83-S4 · RSA PKCS1v15-SHA256 标准签名 + PEM 入参 native（qg-issue 18，GAP-RSA-1）✅

> 立项（2026-09-06）：M83-S4 = Issue 18 —— `rsa_sign/rsa_verify` 裸 type1（MBEDTLS_MD_NONE 无 DigestInfo）
> + 只收 hex 模数/指数，不满足 ws-pay 商户签名 / agentmail DKIM rsa-sha256 硬契约（微信/支付宝服务端验签
> 要求标准 PKCS#1 v1.5-SHA256 + PEM）。mbedtls `pk_parse_key`/`pk_parse_public_key`/`pk_sign`/`pk_verify`
> 现成 API 齐备（源码级核实）→ 走现成 API，零自研 DER。native 296→**298**（+2）。纯新增，零删除；
> 旧 `rsa_sign/rsa_verify` 裸模式原样保留，Linux 基线 m82 全绿保持。

- **S4-1 新增 native**：`rsa_sign_pkcs1v15_sha256(pem_priv, msg)→sig_hex|null`（pem_priv 收 PEM 文本，
  `pk_parse_key` 自动识别 PKCS8 `BEGIN PRIVATE KEY` / PKCS1 `BEGIN RSA PRIVATE KEY`；内部 sha256 + DigestInfo
  + PKCS1v15 = 标准 PKCS#1 v1.5-SHA256；msg 收 str|bytes 二进制安全、超长 msg 自动 sha256 无长度限制；
  **不支持加密 PEM**，私钥按 [SECURITY] 走环境变量明文 PEM 传递）+ `rsa_verify_pkcs1v15_sha256(pem_pub, msg,
  sig_hex)→bool`（pem_pub 收 SPKI `PUBLIC KEY` / PKCS1 `RSA PUBLIC KEY`，`pk_parse_public_key` 自动识别）。
- **S4-2 验证（examples/m83_s4，与 Go + openssl 双向互通全绿）**：Go `rsa.GenerateKey(2048)` → 导出
  PKCS8/PKCS1 私钥 PEM + SPKI/PKCS1 公钥 PEM → px 用私钥 PEM（**PKCS8 与 PKCS1 签出同一签名**，确定性）
  签文本+二进制 msg → Go `rsa.VerifyPKCS1v15(sha256)` true、**openssl dgst -sha256 -verify Verified OK**
  （第三方独立证明输出为标准 PKCS1v15-SHA256）；px 验 Go 签（SPKI/PKCS1 公钥 × 文本/二进制）true；
  2000B 超长 msg 签+验过；反例矩阵（篡改 msg / 坏 sig / 公钥当私钥签 null / 私钥当公钥验 false /
  非 RSA key null / sig 非 hex false）全过。
- 回归：m83_s1/s2/s3 + m82 全绿；native_index/CHEATSHEET/M83_PLAN/CHANGELOG 同步。

### M83-S3 · ed25519 签名/验签 native（qg-issue 17，GAP-ED25519-1）✅

> 立项（2026-09-06）：M83 分六批实现 qg-issue 除挂起 W1b 外全量，S3 = Issue 17 —— 签名族第一个
> 硬缺口（api-server /v1/family 节点互信 PEM 验签 + ws-ddns 更新双向签名均需要 ed25519，无优雅绕过：
> openssl 子进程丑且不自洽）。**issue 描述「mbedtls ed25519 已就绪只差暴露」不成立**（源码级核实：
> 预编译 mbedtls 库不含 ed25519 符号/头，仓库无 mbedtls 源码可重编）→ 用户授权东月定实现：
> **DJB tweetnacl-20140427（public domain 参考实现）**。native 294→**296**（+2）。纯新增，
> 零删除；Linux 基线 m82 全绿保持。

- **S3-1 引入 tweetnacl（GAP-ED25519-1）**：`runtime/tweetnacl.c` + `runtime/tweetnacl.h` 逐字节取自
  https://tweetnacl.cr.yp.to/20140427/（public domain；sha256 归档于 runtime_ed25519.c 文件头）。
  仅 tweetnacl.c 内 crypto_sign_keypair 后新增 **1 处** `crypto_sign_seed_keypair`（RFC8032
  seed→sk64=seed||pub，对齐官方 NaCl 同名 API；Go x509 PKCS8 导出即 32B seed，无此扩展无法用 Go
  的 PEM 私钥直签）。wrapper 独立 `runtime/runtime_ed25519.c`（本地 helper + DER 小解析）。
- **S3-2 语言层 native**：`ed25519_sign(priv, msg)→sig_hex|null`（priv 收 hex seed32 / hex sk64 /
  PKCS8 PEM，seed 自动展开；msg 收 str|bytes 二进制安全；RFC8032 **确定性签名**；内容非法 null）+
  `ed25519_verify(pub, msg, sig)→bool`（pub 收 hex / SPKI PEM；坏入参 false）。PEM/DER：base64 解码
  + 通用 TLV 走查（短/长格式长度均支持），PKCS8（OID 1.3.101.112）嵌套/直存/完整 sk 三种 OCTET
  STRING 形态兼容，SPKI BIT STRING 校验 unused-bits=0。
- **S3-3 验证 examples/m83_s3_ed25519/**（**与 Go crypto/ed25519 双向互通对拍全绿**）：Go 生成
  keypair → 导出 PKCS8 PEM / SPKI PEM / seed / sk64 / msg（文本 + 二进制含 NUL）→ ① px 用 PEM 与
  hex 公钥验 Go 签（文本+二进制）全 true ② px 用 PKCS8 PEM / seed / sk64 **三路签出的签名与 Go 签
  逐字节一致**（确定性 + 格式等价双证明）③ Go 反向验 px 签（文本+二进制）均 true ④ 反例矩阵：
  篡改 msg / 错公钥 / 坏 sig / 长度错 / 公钥当私钥签 null / 私钥当公钥验 false。回归：m83_s2 全绿
  + m82 unix serve 8 项 + m83_s1 6 组 + rsa（m23d）/p7 aes-xml-zip 双模式全绿。
- **S3-4 工具链/文档**：tools/pxc rt_src_files + rt_cache_compile 增 tweetnacl.c/.h +
  runtime_ed25519.c（进缓存 key，改源码自动重建）；native_index 294→296；CHEATSHEET / M83_PLAN 同步。

### M83-S2 · AES-ECB + gzip 语言层通用暴露 + os_spawn setpgid（qg-issue 20-L0，GAP-AES-1 / GAP-ARC-1 / GAP-PGID-1）✅

> 立项（2026-09-06）：M83 分六批实现 qg-issue 除挂起 W1b 外全量，S2 = Issue 20 **L0 三件**——
> AES-ECB（微信网关媒体全链路 AES-128-ECB，iLink 协议硬规定、无优雅绕过）、gzip 语言层通用
> 压缩/解压（runtime 内部 px_gzip_* 早已实现只差暴露，zlib 已链）、os_spawn 独立进程组
> （supervisor 停服需 kill(-pgid) 连孙进程一起清）。native 288→**294**（+6）。纯新增/参数增强，
> 零删除；Linux 基线 m82 全绿保持。

- **S2-1 AES-ECB 四形态（GAP-AES-1）**：runtime_aes.c 新增 `aes_encrypt_ecb(data,key)→hex` /
  `aes_decrypt_ecb(hex,key)→str|null`（PKCS7、key 16/24/32→128/192/256、无 IV）+ bytes 版
  `aes_encrypt_ecb_bytes` / `aes_decrypt_ecb_bytes`（微信媒体为**任意二进制**，hex 版 utf8 校验
  会拒非 UTF-8 → bytes 版无 utf8 校验，对齐 aes_gcm_*_bytes 家族）；mbedtls_aes_crypt_ecb
  逐 16B 块实现（aes_ecb_crypt 内部循环），与 Go crypto/aes NewCipher / openssl enc -aes-128-ecb
  PKCS7 逐字节互通。
- **S2-2 gzip 语言层暴露（GAP-ARC-1）**：注册 `gzip_compress(data)→bytes` /
  `gzip_uncompress(gz)→bytes|null`（包 runtime.c 既存 px_gzip_compress/decompress，标准 gzip 容器
  1F 8B 头 + raw deflate + CRC32 + ISIZE，与系统 gzip / Go compress/gzip 互通）；输入兼容
  str|bytes（bdata/blen 含 NUL 不截断），解压失败（非 gzip/截断/损坏）返回 null。
- **S2-3 os_spawn 独立进程组（GAP-PGID-1）**：os_spawn 增可选第 3 参 `group:bool`（默认 false
  保持现状）→ fork 后子进程 `setpgid(0,0)` 自成组；配合既有 `os_kill(pid, sig, true)` 组杀
  （kill(-pid)），supervisor 停服可连孙进程/daemonize 残留一起清。
- **S2-4 验证 examples/m83_s2_archive_gaps/**：s2_verify.px + verify.sh 全绿——① AES-128-ECB hex
  与 openssl enc -aes-128-ecb -K 逐字节一致（同 PKCS7）② AES-ECB-256 bytes round-trip（任意
  二进制含 NUL）③ 空串→PKCS7 整块 16B 边界 ④ px gzip → 系统 gzip -dc 解回原文 ⑤ 系统 gzip -9
  → px gzip_uncompress 解回 ⑥ gzip 二进制（含 NUL）双向互通 ⑦ os_spawn group=true →
  os_kill(pid,9,true) 组杀成功（rc=137=128+9）⑧ smoke：hex/bytes round-trip + gzip round-trip
  全断言。回归：m82_http_serve_unix 8 项 + m83_s1 6 组 + rsa/bytes/p7 双模式全绿。

### M83-S1 · 服务端 body 动态化（64KB→可配+413）+ len()/contains() NUL 一致化（qg-issue 16，GAP-SRV-2 / GAP-STR-1-B1）✅

> 立项（2026-09-06）：M83 分六批实现 qg-issue 除挂起 W1b 外全量，S1 = Issue 16（http_serve 服务端
> 64KB 固定 body 缓冲 + 语言层 str 对 NUL 截断）。ws-backup receiver 实测缺口：64KB 超限**静默截断
> 不报错**（handler 拿残缺 body = 静默数据损坏）；str 内部带 len 但 len()/contains() 按 C strlen 语义
> 对含 NUL 二进制提前截断。本批只动 L0 runtime，不改语言语法；Linux 基线 m82 全绿保持。

- **S1-1 服务端 body 动态缓冲 + 413（GAP-SRV-2）**：http_conn_worker 第 4 段固定 `char body_buf[65536]`
  栈缓冲 → `xmalloc` 动态跟随 content_length（上限默认 256MB、`PX_HTTP_BODY_MAX` 环境变量可配，
  超限返回 `413 Payload Too Large` 并关连接，不再静默截断）；与客户端 http_request 动态读（M72-S4）
  对称。http_serve 与 http_serve_unix（M82）**共享同一 worker → 一处改两入口通**。
- **S1-2 body 缓冲生命周期**：req.body 经 px_str_len 深拷贝、multipart/form 解析同步完成 → handler
  同步返回后 `xfree(body_buf)`（防 keep-alive 长连接累积）；无 body 时 body_buf=NULL → req.body 给空串
  （避免 px_str_len(NULL,0) 的 memcpy UB）。
- **S1-3 len() 尊重 str.len（GAP-STR-1-B1）**：新增 `px_unicode_len_n(s, n)`（字节边界 UTF-8 字符计数，
  px_unicode_len 改由其包装、strlen 边界——旧调用零回归）；px_len 的 PX_STR 分支改按 `as.str.len` 计数，
  内嵌 NUL 不再截断（文本 str.len==strlen → 字符数语义不变）。
- **S1-4 contains() 字节 memmem（GAP-STR-1-B1）**：新增 `px_memmem(hay,hl,ned,nl)` 字节级子串查找
  （可含 NUL，纯文本与 strstr 等价），bi_contains 字符串分支改 memmem 按 str.len 边界。
- **S1-5 验证 examples/m83_s1_http_body_nul/**：serve_daemon.px（TCP + unix 双 serve）+ verify_cli.py
  （urllib/AF_UNIX 裸 socket POST 二进制 body）+ verify.sh 6 组全绿——① 1MB 全 ASCII POST /len →
  len=1048576（旧 64KB 截 65535）② 含 NUL body `A\0BC` /probe → `len=4,after_nul=true`（旧 len=1、
  contains 跨 NUL 命中 false）③ NUL body /echo 原样回显逐字节一致（响应 str.len 保真）④ unix 入口
  同测（1MB+NUL 全过）⑤ 小 body 无回归 ⑥ PX_HTTP_BODY_MAX=102400 + 200KB POST → 413。
- **S1-6 回归**：examples/m82_http_serve_unix verify 8 项全绿（unix serve 原行为不破坏）。native 288
  （本批无新增函数）。runtime.c 六处补丁 diff 66 行，Linux POSIX 路径零删除、仅动态化/加边界。

### W1a · Windows 交叉编译链骨架（qg-issue 14，Issue 14 W1 第一阶段）✅

> 立项（2026-09-06 用户拍板）：Issue 14 Windows 移植按里程碑推进，W1a = 编译链
> 骨架先行（零风险、不碰 runtime.c），W1b 平台分流/hello.exe 等后续里程碑待令。
> 范围：`pxc build --target x86_64-windows` 参数折叠 + 第三方静态库 windows 交叉
> 入库 + 工具链接通验证；不改语言语义、不改 runtime 平台代码。

- **W1a-S1 --target x86_64-windows OS 维度折叠（commit 待填）**：tools/pxc
  target_defaults 新增 `x86_64-windows`（对齐 M71-S2 注释 GOOS/GOARCH 双维心智）——
  cc=x86_64-w64-mingw32-gcc（PATH 探测：EPEL mingw64-gcc 或 zig wrapper 均可）+
  库布局 lib-windows/（对齐 lib-&lt;arch&gt; 约定）+ **默认 --no-quic**（QUIC/ngtcp2+
  openssl 交叉 Windows 裁剪，与边缘设备同线）；未知 target 提示同步补该值。
- **W1a-S2 第三方静态库 windows 交叉入库（zig cc -target x86_64-windows-gnu）**：
  sqlite3-windows.o（Intel amd64 COFF，1795 symbols，仓库预置）+
  runtime/mbedtls/lib-windows/{libmbedtls,libmbedx509,libmbedcrypto}.a +
  runtime/third_party/zlib/lib-windows/libz.a；file 断言 COFF amd64 通过。
- **W1a-S3 工具链接通验证**：`pxc build --target x86_64-windows` 全链走通
  （target 折叠 → cc 探测 → 静态库完整性 → PXC_BIN AST→C → runtime 预编译入口），
  稳定停在 runtime.c POSIX 头编译错误（预期，W1b 平台分流消化）；Linux 主链
  x86_64 构建不受影响（cache key 按 cc 隔离），CI 不回归。

### M82 · unix socket HTTP 服务端 native http_serve_unix（qg-issue 15，GAP-SRV-1）✅

> 立项（2026-09-06 用户拍板 A1：新原生，不与 TCP http_serve 混淆）：ws-approve .px 化
> serve 前置缺口——PuXian 只有 unix socket HTTP **客户端**（http_unix/unix_connect，
> M56/M66）无**服务端**，而 token-cache 等主客户端是 fail-closed 直连 unix socket HTTP。
> 方案 A1：L0 runtime 新原生 `http_serve_unix(sock_path, handler)`，复用 http_conn_worker
> 同一 HTTP 解析/路由/keep-alive 管道，仅监听面 TCP → AF_UNIX。不改语言语义。
> 回归：runtime.c（新增 native + worker remote 判族兼容）→ pxc build 编译验证 +
> examples/m82_http_serve_unix verify 8 项全绿 + native_index 防漂移（287→288）。

- **M82-S1 http_serve_unix（qg-issue 15 / GAP-SRV-1，commit 待填）**：新原生
  `http_serve_unix(sock_path, handler)`——AF_UNIX 服务端三差异点：启动自动 **unlink 残留
  sock 文件**（异常退出遗留 → bind EADDRINUSE）；bind 后 sock **chmod 0600**（审批/令牌
  本地敏感数据）；accept 循环**错误容忍**（EINTR 重试 + EMFILE 短暂让出）。worker remote
  段 getpeername 改 sockaddr_storage **判族兼容**：AF_UNIX → `"unix"`（原 sockaddr_in 对
  AF_UNIX 会读错字节），TCP http_serve 行为不变。native 287→288。
- **M82-S2 示例 + 回归收口**：examples/m82_http_serve_unix/（serve_daemon.px + verify.sh）
  8 项全绿——进程内自检（http_unix → http_serve_unix /health·POST echo·remote=unix）+
  curl --unix-socket 冒烟 + 残留 sock 清理 + 权限 0600 + 客户端断连容忍 + **TCP http_serve
  同 handler 双跑对拍**；spec §8.22 + CHEATSHEET（native 287→288 + §4.1 unix 变体）+
  ECOSYSTEM + native_index.json（tools/gen_native_table.sh 重生成）同步。

### M72 · AI 调试回路 + runtime bytes 增强（docs/M72_PLAN.md）✅

> 立项（2026-09-06 用户指令，方案 A）：qg-issue **9**（print stdout 全缓冲——服务日志运行中不可见/崩溃前丢失）+ **10**（AI 诊断 D1 编译产物错误无行号 / D2 spawn 协程错误不隔离）+ **13-R1**（ws-backup PuXian 化前置：AES/HTTP 二进制硬缺口 GAP-BIN-1/2）**一批 runtime 立项**，一次自举重建共享成本；13-R2（ws-backup-px 业务）拆 M73。L0 runtime + 编译器诊断层，**不改语言语义**。
> 回归：runtime.c/h + runtime_aes.c + selfhost（cg_stmt/codegen/interp/ibuiltin）改动 → 重链 bootstrap pxc/pxi + golden/compiler.c 与 codegen golden 重生成 + 自举证明 + diffcheck + examples/toolchain verify + native_index 防漂移。

- **M72-S1 print 即时可见（Issue 9，commit 759fc4c）**：`bi_print` 行尾 `fflush(stdout)` → print/println 管道/journald/文件下**逐行实时**（不再攒 8KB、崩溃前不丢行），现有 .px **零改码**；新原生 **`flush()`**（显式刷 stdout/stderr）+ **`print_err(...)`**（渲染同 print 输出 stderr）；pxi ibuiltin 名单+dispatch 同步；native 281→283；实测 200 行间隔输出管道下 2s 已见 110 行。
- **M72-S2 编译产物运行时错误源行号（Issue 10 D1，commit f77732f）**：cg_stmt 语句入口 wrapper 每条可执行语句（含 if/while/for 嵌套块）前插 `px_srcline(<源行>)` + codegen 函数入口插 `px_srcfunc(<函数名>)`；runtime `px_error` 打印 **`运行时错误 [函数 行N]: msg`**（pxi 解释器路径本就带行号不变）→「AI 不需要断点」等价物①：报错即定位源行；重链 bootstrap/pxc；实测嵌套 while 内 `len(null)` → `[foo 行5]`。
- **M72-S3 spawn 协程错误隔离 + 崩溃现场（Issue 10 D2，commit 24a6e95）**：px_error 打印现场后若在 spawn 协程（setjmp 捕获点）→ longjmp 隔离、走 GC 注销路径安全退线程，**宿主继续**；主线程/无捕获点保持 exit(1) 带现场；`PX_SPAWN_ISOLATE=0` 关 → 回退原 exit 语义向后兼容；实测 worker 内错误 → `[worker 行3]` 现场 + `[px-spawn] 已隔离`，宿主全跑 rc=0。
- **M72-S4 runtime bytes native（Issue 13 R1，commit 39a4d6c）**：**GAP-BIN-1** `aes_gcm_encrypt_bytes/decrypt_bytes`（str|bytes 取参含 \0 不截断；去 aes_is_utf8 限制——二进制明文可解出；输出 密文||tag 原始 bytes 与 Go crypto/aes-gcm 字节兼容）+ CBC `aes_encrypt_bytes/decrypt_bytes` 顺带同坑修复；**GAP-BIN-2** `http_request` body 收 str|bytes **长度感知**（Content-Length=bytes.len；body 独立发送顺带修复 16KB req 缓冲大 payload 溢出；http_unix 保持原样）；native 283→287；实测 runtime.c 577953B 二进制 GCM/CBC roundtrip + HTTP POST bytes 上传 sha256 逐字节一致。
- **M72-S5 重建链 + 回归 + 文档收口**：golden/compiler.c（自举基准）与 codegen golden（s 系列 .c，S2 插桩后 C 产物变化）重生成；pxi 重链；自举证明 + diffcheck --all + examples/toolchain verify 全绿；ECOSYSTEM_GAPS §6（M72 已落地能力 D1-D4/B1-B2）；CHEATSHEET native 计数 281→287 + M72 诊断/bytes 注记；ROADMAP/README(.en)/qg-issue 同步；Issue 9/10 归档 done；Issue 13 R1 ✅（R2 → M73）。

### M71 · build 管线现代化 + AI 交付一条龙（docs/M71_PLAN.md）✅

- **M71 立项（commit ae81443）**：四条合一 —— qg-issue 12（B1 build 无缓存全量 gcc / B2 交叉 5 flag 人肉串 / B3 Release 仅 x86_64 / B4 安装靠 PX_STDLIB）+ qg-issue 11（F4 结论过时）+ 用户点名 MCP `build` 第 9 工具（补「写→验→交付」闭环）；Issue 9/10（runtime 诊断，需 selfhost 重建）拆 **M72 候选**。
- **M71-S1 build 增量缓存（commit 528b072）**：`tools/pxc build` 对 runtime 源预编译 `.o` 缓存（等价 Go build cache）——key = runtime源×cc×no_quic；命中只编 base.c + 链接：**二次 build quic 14.7s→0.94s / no-quic 11.6s→0.41s**；改 runtime 源 / 换 cc / 切 quic 自动重建（key 失效），零语义风险（产物同前全量路径）。
- **M71-S2 `--target` 交叉开关（commit bd23dba）**：`pxc build --target <arch>` 对齐 GOOS/GOARCH 心智 —— 自动折叠 cc/mbedtls-lib/sqlite-obj/zlib-lib/`--no-quic` + riscv64 `-no-pie`；显式 flag 优先；目标 cc 缺失给友好指引（含安装建议）；x86_64 缺省等价现状，不回归。
- **M71-S3 MCP 第 9 工具 `build`（commit c7c045c）**：pxmcp `tools_meta`+`mcp_call_tool` 加 `build`（参数 file/code/target/no_quic → spawn `tools/pxc build`，返回产物路径 + 非零 rc 即 isError）→ **AI agent 一条 MCP 完成写→验→交付（编译产物）闭环**；examples/m71_mcp_build verify（list 9 工具 + build 产物静态可运行 + 坏代码 isError）+ m65 verify 8→9 全绿。
- **M71-S4 分发安装（commit e0b19d5）**：Release 资产附 **`sha256sums.txt`**（make_release.sh 与 tarball 同目录生成 + release.yml 上传/Notes 提及）；**`tools/install.sh`** 一键安装（`--prefix`/PX_PREFIX/root→`/usr/local` 非 root→`~/.local`；sha256 强校验失败中止；aarch64 提示用 `--target` 交叉）；`tools/pxc` **argv0 自发现**（readlink 解析软链→真实包根）+ **PX_STDLIB 自动注入**（包根有 stdlib 且未显式设置时）→ **安装后任意目录免环境变量**；实测 `--prefix /tmp/pxinst` 安装 OK + 软链任意目录含 `import std.io` 程序运行 OK（B4 达成）。
- **M71-S5 文档收口**：ECOSYSTEM_GAPS.md **F4 归因更正**（Issue 11：慢的是 pxi 解释器非「纯普贤」——实测 **pxc build 编译版扫 13578 行 runtime.c 仅 0.055s ≈ grep 14×**，工具选型更正为「重文本/大文件编译版毫秒级可胜任；真重负载走 pxc build 产物或等字节码 VM」）+ 顶部更新注记；ROADMAP/README(.en) M71 行 + CHEATSHEET 工具链注记。
### M70 · 语言缺口修复：表达式跨行 + 模块顶层状态（docs/M70_PLAN.md）✅

> 立项（2026-09-05 用户指令）：M69-S4 `docs/ECOSYSTEM_GAPS.md §4` 拆出的两条语言缺口——
> **G2 表达式跨行**（写库第一痛点，AI 生成最常见语法错）+ **G1 模块顶层状态**（写有状态模块/配置中心受阻）。
> 决策：A→B 顺序执行；G3 `let` 不可变（有意设计保持）、G4 分号（不做）；范围铁律 = 只动语言面
> （parser/cg_module 语义），不开 PR；直接开工不排 ws-todo。
> 回归：语言面修复全套（diffcheck --all/--errors + capability 双模式 253 逐字节 + 全能力重建
> bootstrap 链 + 自举证明 B.c==A.c + stdlib/m62-m64/m69_registry/m70_langfix verify + fmt/lint + YAML/bash -n）。

- **M70-S1 表达式跨行（commit c4571b7）**：parser 括号上下文换行容忍——新增 `skip_expr_ws()`，
  在 `parse_call_args`（含尾部逗号）/ `parse_list_or_comp`（含 listcomp for/if 分行）/ `parse_paren_or_tuple`
  / `parse_postfix` 索引切片四处括号上下文支持多行，`brace_looks_like_dict` 跨行 dict 判定修正；
  **不动 lexer token 流 → 现有 golden 零漂移**；新增 `s15_multiline.px` + 四类 golden +
  cases_bad `parse_b11/b12`（括号未闭合仍报错）。新规则：换行仅限括号（`[` `(` `{`）内、语句边界
  仍以换行为准（`=` 后/二元运算符后不换行，续行用括号包裹）、续行缩进须与缩进栈相容（不规则仍 E2002）。
- **M70-S2 fmt 多行收口（commit 3cf0b6d）**：fmt_core 为 token 流重排架构 → **多行天然支持，零代码改动**；
  `examples/m70_langfix/verify_fmt_multiline.sh`（① 多行结构保留+规范化 ② 幂等 ③ 重 lex 语义等价
  ④ fmt 输出 pxi 往返一致 ⑤ --check）全过；单行格式零回归。
- **M70-S3 模块顶层状态（commit 66af554）**：实证——主程序同文件顶层 let/var 跨函数**早已支持**
  （codegen `px_get/set_global`），G1 真缺口 = **import 不导出非 Const 顶层 VarDecl**（实测 R1001 未定义）。
  修复 `cg_module.px`：`cg_is_definition`/`cg_def_name` 的 VarDecl 分支由「仅 Const」放宽为全部
  （let/var/const）→ 模块级状态槽随 import 合并、主程序顶层 VarDecl 初始化注册全局（双模式共用
  `cg_resolve_modules` 一处修复双模式生效）。**新语义/约束**：① 模块顶层 var/let 初始化表达式在
  import 方程序启动时执行一次（import 有「合并初始化」副作用——库作者保持纯值/惰性 init）；② 主程序与
  模块同名顶层声明 → 用户值覆盖模块默认；③ 模块 let 仍不可变（跨文件赋值编译期 E3002）。验证：
  `v04_module_state.px` 9 断言（模块函数读写/主程序直名读写/let 只读/无副作用）pxi+pxc 双模式全过 +
  自举证明通过（golden/compiler.c 重生成 6992 行 B.c==A.c）+ diffcheck --all/--errors 全绿。
- **M70-S4 回归总闸（commit 见本里程碑收口）**：**全能力重建 bootstrap/pxi pxc pxpar**（不带 --no-quic，
  与 M68/M69 发布物对齐——quic/h3 解释能力保持，capability quic 段不丢）；capability 双模式
  **253 PASS 逐字节一致**；diffcheck --all + --errors 全绿；stdlib/m62_langfix/m63_langfix/m64_fmt/
  m64_lint/m69_registry/m70_langfix verify 全过；fmt --check + lint 0 错；ci/release YAML +
  make_release.sh bash -n。
- **M70-S5 文档收口 + S6 发布（commit/tag v0.1.0-m70）**：spec §4.1（表达式跨行规则）/§5.1（顶层
  let/var 全局状态槽语义）/§8.4（import 导出边界）新增修订 + MINI_SUBSET §四/§八/§九/§十三 限制更新
  （G1/G2 从限制改「已支持+规则」）+ ECOSYSTEM_GAPS（G1/G2 标记 M70 已修、保留历史评估）+
  CHEATSHEET §0/§1 同步 + ROADMAP M70 行 + README(.en) 里程碑行；tag `v0.1.0-m70` → push →
  CI 自动发布 + GitHub 产物二次冒烟（sha256 三方一致 + 跨行/模块状态双模式冒烟）+ 本机留档 +
  发布指引更新至 m70。

### M69 · 生态启动：资产化 + AI 速查 + registry 拉取闭环（docs/M69_PLAN.md）✅

> 立项（2026-09-05 用户指令）：GitHub 见 M68 收尾 → 立项 M69 生态线。承接 M68_PLAN §七留档。
> 决策：D1 精编件不进发布包（repo README 为入口 + Release 附链接）；D2 registry 最小闭环（随库入库 + 本地挂载，远程评估）；
> D3 语言缺口只评估入档（修复拆 M70）；D4 tag v0.1.0-m69 自动发布；D5 不排 ws-todo 直接开工；D6 维持 issue 通道不开 PR。
> 范围铁律：不改 compiler/parser/语言语义。

- **M69-S1 生态资产化（commit 25d5add）**：`docs/ECOSYSTEM.md`（9 库一览：定位/导出 API/适用场景 +
  快速用库实测示例 + 119 dogfood 能力导航 + 里程碑主线 + 消费路径 + 写库健康速查）；
  `tools/gen_ecosystem.px` 机器索引（扫 stdlib 顶层 def/头部注释 → `docs/ecosystem_index.json`，幂等）；
  ci.yml 生态索引防漂移步骤（重跑 + `git diff --exit-code`）；README 生态段升级（9 库表 + 入口文档）；
  **修复 stdlib collections.group_by 历史 bug（M5 起：`result = {}` —— {} 字面量 = null + 无 `d[k]=v` 赋值）**
  → `json_parse("{}")` + `.set` API，双模式实测 `{"1":[1,3,5],"0":[2,4,6]}`（8/9 库抽测双模式逐字节一致，edge 需真板）。
- **M69-S2 AI 速查包（commit 56d4c4d）**：`docs/PUXIAN_CHEATSHEET.md`（语言速查 + 11 条易错事实 +
  native 按域速查 + 9 库速查 + 9 个高频模式模板）；`tools/gen_native_table.sh`（runtime*.c `px_set_global`
  注册表 → `docs/native_index.json` **281 全量**，单一事实源 = runtime）；ci.yml native 清单防漂移；
  **AI 自测 3/3 一次写对**（http server 编译模式 / sqlite / yaml，模板实测输出入档）；双模式差异点入档
  （pxi Mini 子集无 spawn/chan → 常驻服务端程序须 `pxc build`）。
- **M69-S3 registry 拉取闭环（commit e1aed36）**：`registry/` 随库入库（9 官库镜像 `<name>/0.1.0/<name>.px` +
  README：结构/用法/发布流程/远程评估/一致性防漂移）；`examples/m69_registry/verify.sh` **11 断言全过**
  （init/add 3 库 → install → lockfile 锁定 0.1.0 → main.px `import semver/yaml/collections` 裸名命中
  `.px_modules`，pxi run + pxc build 双模式一致 → `--locked` 可复现 + registry 不可用仍复现）；
  spec §8.6.3 官方 registry + 远程评估结论（目录随 git clone 分发 / 单包 http URL#sha256；目录级 URL 枚举不做）。
- **M69-S4 写库评估（commit b6079ed）**：`docs/ECOSYSTEM_GAPS.md`（写库规范 checklist 13 条 + 语言缺口
  G1-G4 评估入档 + M69 实证 F1-F5 + M70 候选记录）；结论：G2 表达式跨行（修复候选，写库第一痛点）、
  G1 模块顶层状态（修复候选）、G3 let 不可变（有意设计保持）、G4 分号（不做）；**M70 候选 A/B 记录在案，
  M69 不碰 compiler/parser**。
- **M69-S5 总闸 + 发布（本 commit）**：capability 双模式 **253 PASS 逐字节一致**；diffcheck --all 全绿；
  stdlib 9 库双模式（S1 抽测 + m66 verify）；m45_pkgdemo + m69_registry verify 全过；fmt --check + lint 0/0
  （gen_ecosystem.px 纳入 tools lint 域）；ci/release YAML + make_release.sh bash -n；生态/native 索引重跑
  diff 空（幂等）；**tag `v0.1.0-m69` → push → tag 驱动 workflow 自动发布** + GitHub 产物二次解包冒烟 +
  发布指引更新（RELEASE_PROCESS.md）。

### M68 · pxi 一致性收官：解释器 native 可达性根治（docs/M68_PLAN.md）✅

> 来源：清歌反馈「pxi 内置不全、缺 sqlite，编译器没事」→ 源码级侦查确认根因：编译产物
> 默认可达 = runtime `px_set_global` 全局 native **281 名**，pxi 只认 interp.px
> `i_register_builtins` 白名单 **129 名** → 差集 155（sqlite/aes/rsa/xml/zip/tcp/udp/ws/
> sse/cron/session/bus/http_serve/os_pid/now_ms…）在 pxi 裸脚本（零 extern def）R1001。
> 决策：D1 A 根治（否决 B 补白名单）；D2 里程碑只做 pxi 一致性（生态线移出留档）；
> D3 打 tag v0.1.0-m68 发布；不开 PR。

- **M68-S1 差异表定稿（commit d71a82c）**：`docs/pxi_native_diff.md` 逐名归因（A 类真
  native 91 + quic/h3 条件编译 64；`__px_*` 伪全局 / pi·e 常量 / chan·spawn 语言构造 /
  dict·flatten·unique 解释器内部 均已剔除记依据）。
- **M68-S2 根治实现（commit 3b47dbe，+65/-2 行）**：C 侧 `ffi_call` **双表兜底**——① ffi
  注册表（extern def C 库原路径）→ ② 新增 `px_global_native()` 查全局 PX_NATIVE 表
  （px_set_global 注册的全部内置，编译产物裸名调用同源单源，无逐行注册宏/无表扩容/无
  双源漂移）；两表未命中返回**可辨 Err** `ffi_call: 未注册函数: <name>`（不杀进程，原
  静默 null）。pxi `i_eval_call` Var 分支 env 未命中 → **自动回退** `ffi_call(cname, args)`，
  宿主哨兵 Err（真拼错名）转 R1001，业务返回值/Err 原样透传。pxi 重建（bootstrap/pxi）。
- **回归全绿**：capability 双模式 **253 PASS 逐字节一致**；t_native 零 extern def 裸脚本
  **19/19（pxi == 编译产物）**；用户现象 t_sqlite pxi 裸调正常（原 R1001 修复）；t_typo
  拼错名仍 R1001 可辨；diffcheck --all 全量通过；m66 stdlib verify yaml35/lunar36/
  pxml68/proc14 双模式 PASS；fmt --check 通过；ci.yml YAML + make_release.sh bash -n 通过。
- **文档收敛**：spec §9.3 + MINI_SUBSET §十三.0 + README/README.en 已知限制（pxi 白名单
  条目更新为 M68 根治后一致）+ CHANGELOG 本条 + qg-issue 00-README §4 第 1 条勾除。

### M67 · 多架构一等支持：aarch64 交叉编译 + GC 架构抽象 + armv7/riscv64（qg-issue 07，docs/M67_PLAN.md）✅

> 来源：清歌 qg-issue 07（任务清单·执行版）两阶段 —— 阶段一 aarch64 交叉编译提升为一等支持；
> 阶段二 runtime GC 架构抽象层 + armv7（armhf）/riscv64 扩展。决策：D1 armv7/riscv64 交叉库
> CI 现编 + actions/cache **不入库**（aarch64 维持仓库预置）；D2 armv7 = armhf（linux-musleabihf）；
> D3 GC 改动**重构等价**（不做算法变更）；D4 两阶段一体入 M67。实测修正：apt glibc 交叉不可背书
> （官方只支持 musl.cc / docker 两路）；riscv64 需 -no-pie；qemu-user 并发 GC 模拟限制留档。

- **M67-S1~S3 阶段一 aarch64 一等支持（commit d361b29）**：README.md / README.en.md 单列
  「ARM64 Linux 交叉编译」章节（musl.cc tarball + docker 两路获取 / 一条命令交叉 / file+qemu 校验 /
  为何不用 apt glibc）；CI 新增 aarch64 job（actions/cache musl 工具链 + 三用例 qemu 验证）；
  examples/m67_aarch64/{hello_a64,http_a64,sqlite_a64}.px + verify.sh（交叉编译 → file 断言 ELF ARM
  aarch64 → qemu 运行：hello 直跑 / HTTP qemu 起服宿主 curl 200 / SQLite CRUD）本机实测全绿。
- **M67-S4 runtime GC 架构抽象层（commit e97b377，阶段二心脏）**：runtime.c 3 处 GC 架构 #if
  （gc_scan_stack / gc_scan_registers / gc_scan_thread_stack）迁出为统一接口 `runtime/arch.h`：
  `arch_read_sp()`（当前线程 SP，内联汇编跨 glibc/musl）+ `arch_scan_registers(uc, mark_cb, ctx)`
  （暂停线程 ucontext → 逐寄存器 word 回调标记）+ `arch_uc_sp(uc)`；分架构头 `arch_x86_64.h` /
  `arch_aarch64.h`（原样迁出，行为零变化）+ 新增 **`arch_armv7.h`**（armhf：arm_r0..arm_r12 + arm_sp，
  14 word 扫描）与 **`arch_riscv64.h`**（mcontext `__gregs[32]` RISC-V psABI 序，sp=`__gregs[2]`）——
  GC 主逻辑不再见架构 #if，以后加架构只增头文件。等价性证明：x86_64 diffcheck --all + capability
  双模式 253 PASS + examples/m67_multiarch/gc_stress（并发 4 worker 深链 + stop-the-world）+ m65/m66
  verify 零回归；aarch64 交叉 verify 全绿；**armv7/riscv64 C 层探针**（SIGUSR1 ucontext → 扫 14/32
  regs + SP）musl 交叉 + qemu 实测布局通过。
- **M67-S5 工具链扩展（阶段二 #2 通道）**：tools/pxc zlib 自动探测扩三架构（aarch64/armv7/riscv64
  → lib-<arch>）+ copy_runtime 拷 arch 头 + **riscv64 自动 `-no-pie`**（musl static-pie 报
  "read-only segment has dynamic relocations" 实测修复）；`tools/cross_aarch64.sh` 泛化为
  **`tools/cross_multiarch.sh --arch aarch64|armv7|riscv64 [--outdir dir]`**（sqlite3.o + mbedtls +
  zlib 三件套独立副本交叉现编，架构校验 file 断言），cross_aarch64.sh 保留为 --arch aarch64 兼容薄包装。
- **M67-S6/S7 交叉库 + 四档矩阵**：armv7/riscv64 交叉库本机现编至 /opt/px-multiarch（不入库）；
  examples/m67_multiarch/{hello_multi,http_multi,sqlite_multi,gc_stress,gc_single}.px + verify.sh
  四档矩阵 —— **x86_64 native 全量**（含 gc_stress 并发 GC 压力 2000 + gc_single 单线程 3 万迭代）
  + **aarch64/armv7/riscv64 qemu-user** 各 hello/http/sqlite（qemu 起服宿主 curl 200，file 断言
  ARM aarch64 / ARM EABI5 / RISC-V）本机全矩阵 exit=0；CI 升级 **multiarch-cross 矩阵 job**
  （三架构并行：musl 工具链 + 现编库 actions/cache，aarch64 用仓库预置）。
- **M67-S8 生态收口**：spec §8.21 多架构段（arch.h 接口 + 四架构 + qemu 限制记录）；ROADMAP 补 M67 行 +
  当前里程碑 M67 已闭环；CHANGELOG 本条目；README/README.en 里程碑表扩 M41–M67；
  tools/make_release.sh 发布包含 cross_multiarch.sh + RELEASE.md 内容表更新。
- **验证**：四档矩阵 verify ALL PASS（x86_64 含 GC 压力）；spec/README/CHANGELOG 全绿；ci.yml
  YAML OK；diffcheck/capability/m65/m66 零回归（S4 提交时全绿）；qg-issue 07 归档 done/。
- **M67-CIfix CI 修复（commit 24e1065，多架构矩阵收绿）**：multiarch-cross 三档工具链双源 ——
  ①修复 CIfix4 ci.yml heredoc 缩进错误（顶格破坏 YAML 块标量 → workflow 未启动，run95 直接失败根因）；
  ②musl.cc 路 1 改 2 次快速尝试（GitHub runner 侧到 musl.cc 持续不可达），失败自动切路 2
  zig cc fallback（ziglang.org 0.16.0 x86_64 55MB 单文件 = 三架构 musl 交叉 clang，生成
  `<triple>-gcc/ar/ranlib` wrapper 对齐 musl.cc 布局，后续 PATH/脚本零改动复用）；
  ③zig 独立 actions/cache + 现编库 cache key 分工具链源。GitHub CI 六 job 全绿：aarch64（zig cc
  混链仓库预置 gcc-musl 库 + qemu 三用例）、armv7/riscv64（zig 现编交叉库 + qemu 三用例）全 PASS；
  本机同步实测 zig 路三架构 verify exit=0。
- **M67-CIfix6 cache 联动修复（commit 69ee30e）**：修复 CIfix5 引入的二次回归（run97 三档红）——
  根因一：cache-musl 命中时 cache-zig 被 if 跳过 → `/opt/zig` 未恢复 → wrapper `exec /opt/zig/zig`
  找不到立即失败；根因二：cache-libs key 依赖被跳过的 dl 步骤输出 TOOLCHAIN（空→'cached'），
  与首轮存的 '-zig' 不匹配致每轮重复现编。修复：cache-zig 去掉 if **始终 restore**（保证 wrapper
  依赖的 zig 本体在场）；cache-libs key 固定 `px-libs-<arch>-v2`（与工具链来源解耦，musl ABI 稳定
  可混链）。**CI run 98 六 job 全绿**（aarch64/armv7/riscv64 三档 qemu 验证 + 回归 + 质量门），
  cache 全 warm 后 multiarch 三档仅需 qemu 安装 + verify 秒级完成。

### M66 · 自举 wsAgent runtime 原语补全 + stdlib 收编（qg-issue 01–06 全量合入，docs/M66_PLAN.md）✅

> 来源：清歌（qingge）qg-issue 01–06 —— ws-core / ws-install / ws-todo 等 wsAgent 生态模块
> 被卡的 runtime 原语缺口（P0 blocker）与 stdlib 生态缺失。决策：D0 以官方仓库现状为准；
> D1 os_capture 与既有 os_spawn_capture 并存（零回归）；D2 zip 密码 zipcrypto + WinZip AES-256 全实现；
> D3 农历纳入 M66（std.lunar）；D4 命名「自举 wsAgent」。

- **M66-S1 L0 runtime 原语补全（qg-issue 01/02/05，runtime/runtime.c +386 / runtime_zip.c +255）**：
  新增 **unix_connect**（AF_UNIX 裸连接 fd，行协议驱动，http_unix 补全）；**os 五件套** os_exec
  （execvp 进程替换，launcher 语义）/ os_rename（rename(2) 原子覆盖）/ os_remove_all（递归删，空串
  与 `/`、`//` 拒绝防删根）/ os_random_hex（urandom→hex）/ os_file_sha256（mbedtls 文件哈希）；
  新增 **os_capture(cmd,args)→{rc,stdout,stderr}**（双管道分离捕获，D1 与 os_spawn_capture 并存；
  G6 which 用法示例 `os_capture("command","-v",x)`）/ **os_popen(cmd,args)→{stdin_fd,stdout_fd}**
  （双向管道 + setpgid 自成组）；**os_kill 第三参 group**（kill(-pid) 组杀，两参兼容）；
  **write_file/append_file 第三参 mode**（open + fchmod 防 umask 削权，写 0600 密钥免 chmod）；
  **zip_unpack 第三参 password**（**zipcrypto 传统 + WinZip AES-128/192/256**：extra 0x9901 探测 +
  PBKDF2-HMAC-SHA1 + AES-CTR + HMAC-SHA1 auth code，AE-1/AE-2）。pxi 白名单 + ibuiltin + pxlint
  BUILTINS 三处同步 → bootstrap/pxi、pxlint 重建（解释模式新 native 可调）；专项
  examples/m66_proc verify ALL PASS（os_fs_test 16 + os_exec 透传 + proc_test 14 + unix 行协议 +
  zipcrypto/AES-256 解包）；capability 双模式 253 PASS + diffcheck 全量 + m65 回归零影响。
- **M66-S2 L1 stdlib 收编 std.yaml（qg-issue 03，第 7 个标准库）**：stdlib/yaml.px（YAML 配置子集
  解析 yaml_parse，纯函数零 import）+ examples/m66_yaml 双模式 35 PASS 一致。
- **M66-S3 L1 stdlib 收编 std.pxml + PXML 规范（qg-issue 04，第 8 个标准库）**：stdlib/pxml.px
  （解析 + ENC 加密还原，主打编译模式）+ **docs/PXML.md 规范 v0.6**（语法/EBNF/enc 策略/决策表/
  语言缺口笔记）+ examples/m66_pxml dogfood 闭环（pxml_test 双模式 68 + demo 16 + demo_enc 4 PASS）。
- **M66-S4 L1 stdlib 收编 std.lunar（qg-issue 06 T3，第 9 个标准库，D3 拍板纳入）**：
  stdlib/lunar.px 内嵌 1900-2100 农历数据表（寿星天文历同源，逐年对拍 0 误差）+ 公历/农历互转
  （lr_solar_to_lunar / lr_lunar_to_solar / lr_md_in_year / lr_leap_month 等，纯函数双模式一致）+
  examples/m66_lunar 双模式 **36 PASS**（春节/除夕/闰二月/边界 1900-2100/往返/ws-todo lunar:8-8 落点）。
- **M66-S5 生态收口**：spec §8.20 原语补全段 + §10.3 std.yaml/pxml/lunar 行；ROADMAP 主线表补 M66 行；
  CHANGELOG 本条目；README/README.en 原生开发表扩 M41–M66 + 标准库 9 个清单同步；CI toolchain job
  工具自测并入 m66_yaml/m66_pxml/m66_lunar verify。
- **M66-S6 qg-issue 回写**：/data/qg-issue 01–06 头部状态行标「已合入 M66」；05 §7 / 06 §7 重写时机
  checklist 勾除（G1/G2/G3/Issue 2/农历全勾 → ws-install / ws-todo 主体 .px 可写，M67+ 业务里程碑另立项）。
- **验证**：m66_proc/m66_yaml/m66_pxml/m66_lunar verify ALL PASS；capability 双模式 253 PASS / 0 FAIL；
  diffcheck --all 零回归；fmt --check + lint 全仓全绿（stdlib 3 新库 lint 0/0）。

### M65 · LSP / MCP 自举（spec §12 工具链收官 + §12.1 AI agent 协议，docs/M65_PLAN.md）✅

- **M65-S1 JSON-RPC 共享底座 + runtime 补丁**：`tools/jsonrpc_core.px`（纯 defs）——
  Content-Length 帧读写（read(0) 累积缓冲，半包/粘包/坏头/坏 body 自测 33 断言双模式）+ JSON-RPC
  2.0 骨架（request→result / notification→无回 / 标准错误码 -32700/-32601）；runtime 补唯一原语
  **`os_spawn_capture(cmd,args)→[rc,output]`**（fork+execvp，stdout+stderr 合并单管道免死锁，
  exec 失败 127，编译/解释双模式 5 断言）；pxi 白名单 + ibuiltin 同步重建；pxlint BUILTINS 补裸
  read/write（M57 fd 原语漏补）；capability 双模式 253 PASS + diffcheck 全量无回归。
- **M65-S2 LSP 核心**：`tools/pxcheck.px` 独立诊断器（import parser+lint_core，lex+parse+lint →
  stdout 单行 JSON；parse/lex 错误= parser 打印后 panic 退出 1，pxlsp 按文本解析）+ `tools/pxlsp.px`
  0.1.0 —— 生命周期 initialize/initialized/shutdown/exit + didOpen/didChange(Full)/didSave/didClose +
  **publishDiagnostics**（**深度诊断子进程化**：parser 语法错误 print+panic 杀进程不可捕获 → pxcheck
  子进程隔离）；python3 模拟标准 LSP client 17 断言端到端全绿。
- **M65-S3 LSP 增强**：`tools/lsp_core.px` 符号/补全/跳转/悬停语义层（顶层 def/struct/enum/trait/
  impl/var 文本级行扫 + ## 文档注释并入 + 局部名宽松收集）+ pxlsp 0.2.0 开 completion/definition/
  hover 能力位（诚实协商）；S3 client 39 断言全绿（9 来源补全候选 + 前缀过滤 + definition 行精确 +
  hover 签名/文档 + 真实文件 selfhost/astdump.px + 错误输入不崩）。
- **M65-S4 MCP 服务器**：`tools/pxmcp.px` —— MCP 2024-11-05 stdio transport；tools/list 暴露 8 工具
  （run/fmt/lint/test/bench/doc/ast/version 带 inputSchema）+ tools/call **全部子进程执行**
  （os_spawn_capture 调 bootstrap 各二进制：崩溃隔离不污染协议 stdout + 输出可捕获）；S4 client
  41 断言端到端全绿（每工具成功回包 + 错误参数/未知工具/未知方法 isError/-32601）。
- **pxc 子命令**：`pxc lsp` / `pxc mcp`（bash exec 直通保留 fd 0/1，stdio 即协议通道）。
- **文档 / 收口**：spec §12 实现状态表 8 工具全自举 + §12.1 MCP 勾选；ROADMAP 主线表补 M62–M65 行
  （M62–64 此前仅 CHANGELOG 记录，缺行补全）+ 工具链行扩 11 子命令；README/README.en CLI 表补
  lsp/mcp + 原生开发表扩至 M41–M65；M64_PLAN M64d 状态行更新（已由 M65 承接完成）。
- **验证**：m65_lsp + m65_mcp verify ALL PASS；fmt --check + lint 全仓全绿；capability 双模式
  253 PASS / 0 FAIL；新 5 文件（jsonrpc_core/pxcheck/lsp_core/pxlsp/pxmcp）lint 0 错 0 警。

### M64 · 工具链自举恢复（fmt/lint/doc/test/bench 五项自举，docs/M64_PLAN.md）✅

- **M64-S1 keep-lexer 底座**：不碰 pxlexer（自举链零风险），派生 `tools/fmtlexer.px`
  （788 行，`g_keep` 开关 + `lex_tokens_keep` 保留注释/行结构，对齐 Rust
  `new_with_comments`）；默认模式与 pxl 逐字节一致。
- **M64-S2 `pxc fmt`**（fmt_core.px + pxfmt.px）：确定性格式化（空格规则/行结构重建/
  注释对齐/空行压缩/unified diff/幂等）；修正 Rust 版一元负号缺陷
  （`x = -1` 不再压成 `x =-1`）；格式化前后重 lex token 序列完全一致（语义等价）。
- **M64-S3 `pxc lint`**（lint_core.px + pxlint.px）：L001-L008 全移植（AST 驱动，
  复用自举 parser），`--json`/`--strict`/退出码语义；自举工具链 7 文件 dogfood 0/0。
- **M64-S4 fmt 全仓收敛**：审阅驱动修 4 缺陷（插值/数字原文保真、切片/后缀 `?` 紧贴、
  首行 def 压坏）；selfhost 21+tools 7 文件 --check 全绿；净 -318 行；
  **自举证明 B.c==golden 逐字节 + capability 双模式 253 PASS**。
- **M64-S5 `pxc doc / test / bench`**（pxdoc/pxtest/pxbench.px + pxslice.px 共享切片）：
  doc 从 `##` 注释生成 Markdown（对齐 Rust doc.rs：文件头说明并入首个定义）；
  test 运行顶层 `def test_xxx()` 无参函数，逐用例独立 pxi 子进程（语言内 os_spawn+
  os_wait 编排，非 shell）；bench 无参目标 N 次循环 × R 轮计时（now_us）。
  三工具 --version/--help + examples/m64_{doc,test,bench}/verify.sh 全绿
  （doc 18/18、test 12/12、bench 8/8）；pxlint BUILTINS 补 now_us/sleep_us。
- **README / spec §12 勾选同步**：pkg/ast/fmt/lint/test/doc/bench 七项自举已标注，
  lsp/mcp 留 M64d（按需）。

### M64 收尾 · 欠债清理（docs/M64_PLAN.md §14）✅

- **CI 质量门**：ci.yml 新增 `toolchain` job —— `pxc fmt --check`（selfhost+tools+stdlib
  39 文件收敛域）+ `pxc lint`（compiler.px 项目级主入口 + tools 11 独立文件 0/0）+
  m64_fmt/m64_lint verify；PLAN §5 承诺落地。
- **fmtlexer `${` 转义修复**：错误消息 `"字符串插值 ${ 未闭合"` 裸 `${` 会触发插值
  （错误路径求值未定义调用），改 `\${`；重建 bootstrap/pxfmt、pxdoc。
- **stdlib 6 文件 fmt 收敛**：collections/edge/gfx/png/semver/webroute 写回（净 -66 行，
  空行压缩+注释对齐，与 selfhost/tools 同规则）；capability 编译版 253 PASS 佐证无损。
- **pxlint 增强**：BUILTINS 白名单补 QUIC/H3 内建 32 名（capability L002 ×97 → 0）；
  支持 `type X const (...)`（M44 常量枚举）顶层名收集（LogLevel/Code 误报消除）；
  重建 bootstrap/pxlint；m64_lint verify 18/18 无回归。
- **L007 存量数据行 noqa**：compiler.px KEYWORDS/CTRL_ALL（653/371 字符语言强制单行）+
  capability 26 处长断言/extern 签名行尾 `# noqa` → compiler.px lint **0 错误 0 警告**、
  capability lint **0 错误**（剩 2 L001 演示形态 warning 留档 §14.3）。

### M63 · 语言面欠账修复（L8–L11 全清：pxi 网络 API / float 全精度 / pxc --version）✅

- **规划**：MINI_SUBSET §十三 欠账总结清 L8–L11；回归 examples/m63_langfix/verify.sh
  四线全绿（L8/L9/L10/L11 双模式对拍）。
- **L8 pxi 网络真实应用 API 补白名单**（interp.px + ibuiltin.px）：`http_post`/
  `http_request`/`s3_get`/`s3_put`/`s3_list`/`s3_delete` 进解释器 names 白名单 +
  `i_call_c_net` Result 透传 helper（C 网络失败 Err 不杀进程，M57 语义，双模式失败路径
  一致）；本地 mock HTTP 真请求双模式逐字节一致 + 失败 Err 透传 + 参数错报错退出。
  **http_get_stream 留档**（chunk_handler 宿主函数回调跨解释器边界，Mini 排除）。
- **L9 float→str 最短 roundtrip 全精度**（runtime.c `fmt_num`）：`%g` 6 位截断 →
  定点舒适区规则（十进制指数∈[-4,15) 内 `%.*f` 定点、区外 `%.*e` 科学，逐位 + strtod
  回读取最短 roundtrip；保持语言习惯 100000.0→"100000.0"、250.0→"250.0"、1e15→"1e+15"）；
  修复 0.1+0.2→"0.3"、1/3→"0.333333"、123456789.123→"1.23457e+08" 等精度丢失；`.0`
  补丁保留；双模式同根单点修复（m63_fp 16 断言逐字节一致）。
- **L10 编译期浮点字面量全精度**（codegen 零改动）：pxc 自举重建内嵌新 fmt_num →
  `cg_fmt_float` 的 `str(v)` 自动全精度 → C 产物 `px_float(3.141592653589793)`；
  v01_value float**（1.4142135623730951）编译/解释全 PASS → **diffcheck.sh 三处
  v01 %g 豁免移除**（差异即失败）。
- **L11 bootstrap/pxc --version**（compiler.px main 参数前置分支 + PXC_VER/PXC_MS）→
  **pxc 自举重建**（--no-quic）+ golden/compiler.c 同步（diff 仅 main +7/-1）；
  修复前 `--version` 被当文件读报错（RELEASE_PROCESS 已知边界勾除）。
- **验证**：verify.sh ALL OK；diffcheck --all/--errors 全绿（rc=0）；capability 双模式
  253/253 PASS；自举证明 B.c==golden/compiler.c；m59_math/m61_gfx/m62_langfix 回归 PASS。
- **文档**：MINI_SUBSET §七 #7/#8 勾除 + 新增 §十三.8 修复记录；spec §10.2 浮点打印
  全精度表述。

### M62 · 语言面欠账修复（L1/L5/L6/L7 + L2/L3/L4 处置）✅

- **规划**：MINI_SUBSET §十三 欠账总结（L1–L11）中清 L1–L7 可修硬欠账；
  回归 examples/m62_langfix/verify.sh（L1/L6/L7）+ verify_l5.sh（L5）双模式一致。
- **L1 浮点打印 `.0` 对齐**（commit `9acfb94`）：runtime.c `fmt_num` float 分支补 `.0`
  （整值有限 |f|<1e15 且 %g 无 `.eE`）→ `print(3.0)` 编译模式 `3`→`3.0`，与解释器 /
  Rust fmt_float 语义逐字节一致（str/插值/list/dict 内浮点全对齐）；6 位 %g 截断
  （0.1+0.2→0.3）保留为既定规避项。
- **L5 codegen 块作用域（变量提升 hoist）**：if/for/while 内 `var/let` 块外引用原编译报
  C undeclared（M-B2 留档"待 M 后补"）→ `cg_collect_hoist_vars` 统一收集 Assign 目标 +
  VarDecl + For 循环变量，函数顶 `px_null()` 预声明 + 原位赋值，对齐解释器/Python 函数级
  语义；**bootstrap/pxc 自举重建 + cases/compiler 的 C golden 全量更新**（hoist 结构变化，
  语义等价由 capability 253 PASS + 自举 B.c==golden + diffcheck --all/--errors 全绿证明）。
- **L6 split 保留空段回归**（commit `f41c529`）：自举 interp 重写后已保留空段（编译/解释
  双模式一致），补 fp_split.px 断言防回退。
- **L7 pxi bytes 族白名单补齐**（commit `ab598e0`）：interp.px names +14（bytes/bytes_len/
  bytes_get/bytes_set/bytes_slice/bytes_concat/bytes_to_str/bytes_to_hex/hex_to_bytes/
  bytes_find/bytes_base64/base64_to_bytes/base64_encode/base64_decode）+ ibuiltin 直调转发；
  bootstrap/pxi 重建；fp_bytes.px 17 断言双模式一致。
- **L2/L3/L4 处置（不改语义）**：`int(str)` 宽容前缀、`{}` 空 dict 字面量、import 模块顶层
  不执行 —— 均双模式一致的语义设计，破坏性收紧风险大于收益；文档保留警示与既有规避
  （MINI_SUBSET §十三.7），待真实需求再评估。
- **文档**：MINI_SUBSET §七 M-B2 两行待修标记 ✅、§十三 #7/§十三.4/.6 注记、新增 §十三.7
  修复记录；spec §10.2 浮点打印注记。

### 发布自动化 · tag 驱动 GitHub Release（仓库治理）

- **`.github/workflows/release.yml`（新）**：推送 tag `v*` 触发 → ubuntu-latest 上
  `tools/make_release.sh` 构建发布包（内置冒烟自检）→ `gh` CLI 创建 GitHub Release
  并上传 tarball（`GITHUB_TOKEN`，零第三方 action）；Release notes 自动含
  SHA-256 与上一 tag 起的提交列表。发布 SOP：`git tag v0.1.0-m62 &&
  git push origin v0.1.0-m62`。
- **`tools/make_release.sh` 版本来源 tag 驱动**：VER 取最近 tag 的 `vX.Y.Z` 部分
  （`v0.1.0-m62` → `0.1.0`），里程碑优先级改为 命令行参数 > tag 后缀 `m62` >
  最近提交推断，避免发布包版本与 tag 不一致。
- **`pxi --version` 补齐**：`selfhost/interp.px` 入口支持 `--version`/`-v`（无文件
  参数时输出版本退出 0，输出与 `tools/pxc --version` 对齐）；`bootstrap/pxi` 重建
  （对齐"所有程序支持 --version"规范）。

### M61 · 外部库 FFI proof（zlib）+ 纯语言 2D 游戏内圈（gfx/png/贪吃蛇）✅

- **规划**：`docs/M61_PLAN.md`（A=外部系统库绑定全链路 proof + B=纯语言 2D 内圈；无真板
  期游戏线 0→1 地基；用户选 A+B 并行）。
- **S0 zlib 两版静态 .a 入库**：`runtime/third_party/zlib/{include,lib,lib-aarch64}`，
  zlib 1.3.1 源码自编（x86_64 gcc / aarch64-linux-musl 交叉，ar 抽 crc32.o 双架构校验）；
  `tools/build_zlib.sh` 一键重建；`tools/pxc` 加 `--zlib-lib <dir>`（缺省按 `--cc` 自动
  架构探测：gcc→lib/，aarch64→lib-aarch64/，旧交叉脚本免改）+ 无条件链 libz.a（无引用
  不抽成员，向后兼容）—— commit `84fe8c8`。
- **S1 薄胶水 runtime_zlib.c**：`zlib_crc32(data)`（crc32(0,data,len)，已知值可校验）/
  `zlib_compress(data,level)`（compress2 + **uLongf\* 长度指针** cap→实际）/
  `zlib_uncompress(data)`（z_stream inflate **渐进扩容**免预知大小）；str/bytes 二进制安全、
  数据非法→null 不杀进程；注册进 FFI 表（px_register_zlib）双模式同构；pxc 链
  runtime_zlib.c；m61_zlib.px 七组断言（已知值 0x3610a686/标准 check 0xCBF43926/纯语言
  CRC32 查表 5 组互证/long roundtrip 10400→110B/NUL 安全/空串/非法流 null/level 0·6·9）
  + `nm` 实证 crc32/compress2/inflate 符号 + 4 例回归 PASS —— commit `1639c5f`。
- **S2 std.gfx + std.png（第 5/6 个 stdlib）**：gfx.px 画布 list[int] 0xRRGGBB +
  line(Bresenham)/rect/fill_rect/circle(中点)/fill_circle(弦扫描)/blit(透明跳过)/
  text(5x7 compact 字形，0-9 A-Z . - 空格，小写→大写)；png.px 纯语言 PNG 8bit RGB
  stored 编码器（CRC-32 查表 + ADLER-32 + zlib stored block + chunk 组装，零 FFI）；
  demo Mandelbrot 640x480（复数迭代 + 11 色调色板）+ 合成场景全原语；**python3 stdlib
  zlib 独立解码全校验**（chunk CRC 全过 + 像素颜色抽查全对）—— commit `171e59b`。
- **S3 raw 终端可玩贪吃蛇**（examples/m61_gfx/m61_snake.px）：w/a/s/d 控向 q 退出，
  O/#/@；try_step 纯函数 + spawn_food；交互走 tty_config(0,9600,raw)+fd_wait+read 单键
  （M60 设备组应用层 dogfood）；SNAKE_AUTO=1 无头剧本 EAT/SELF/WALL 三断言 +
  python3 PTY 真内核喂 q → QUIT；修正 tty_config 波特率 0 不支持（须 9600+ 枚举）；
  uinput/evdev stretch 留档 —— commit `2eb3060`。
- **S4 收口**：bootstrap/pxi 重建（9,293,144 B，含 runtime_zlib.c + libz.a → extern
  zlib_* 经 ffi_call C 桥双模式同能力，无需白名单/ibuiltin 分支）；m61_s4_zpxi.px pxi
  smoke（Mini 子集 int_to_bytes 构造 + crc32 守恒断言）解释==编译逐字节一致；
  m61_s4_det.px 纯整数绘制 PNG **x86 == qemu-aarch64 sha256 一致**（605c5b07…）；
  m61_s4_zpng.px **FFI compress2 直接产出标准压缩 PNG**（python 独立解码合法 +
  像素抽查）；m61_s4_impsmoke.px 探针：pxi 可解释 std.gfx 纯 list 路径、text/blit 等
  bytes 依赖仍受限；回归 hello/fib pxi + math_s1 编译 + dev_s1 --no-quic 编译 ——
  commit `ab29185`。
- **S5 文档收口**：spec §8.19（FFI 外部库约定 + gfx/png 库）+ §10 表 + MINI_SUBSET
  §十三.6（extern 双模式零成本 / pxi bytes 族缺口 / stdlib import 边界复核 / 性能
  dogfood 14-34s 每帧 → bytes 画布方向）+ ROADMAP/GAP 勾选 + CHANGELOG。
- **边界如实记录**：SDL2/raylib 真窗口结论留档（无屏，M61-PLAN §4）；QQ 富媒体发送被
  平台拒（err 40093007 下载失败），图片落盘 examples/m61_gfx/*.png 供自取；
  pxi stdlib 完整能力仍主打编译模式；性能（640x480 逐像素 list+concat 14-34s/帧）
  优化方向留档 bytes 画布。


- **规划**：`docs/M60_PLAN.md`（前置实测复核：poll 仅 runtime 内部 3 处未暴露、termios
  全库零命中、sleep 仅 ms 整数粒度、SPI_IOC_MESSAGE 因 transfer 数组含 u64 指针留档、
  GPIO V2 单线请求 592B 结构体 C offsetof 实测核对）。
- **S1 us 级时钟 + fd 控制**：`sleep_us(us)`（nanosleep，EINTR 续睡，<=0 不睡）/
  `now_us()`（**CLOCK_MONOTONIC** 微秒，测量语义与 now_ms 的 REALTIME 墙钟区分）/
  `fcntl(fd, cmd[, arg])`（标准 fcntl，O_NONBLOCK 等；失败 -1+os_errno）—— commit
  `fa91805`。
- **S2 设备组**：`tty_config(fd, baud, raw)`（tcgetattr → cfmakeraw(raw) → cfsetispeed/
  cfsetospeed → tcsetattr(TCSANOW)；baud 9600…921600，无效档终止；失败 false+errno）/
  `fd_wait(fds, timeout_ms)`（内部 poll 暴露：int/list<int> 上限 64、只监听 POLLIN、
  revents 非 0 即事件返回（含 HUP）、**超时空 list 非错误**、poll 错误 -1+errno）——
  commit `bc97b20`。
- **S3 stdlib std.edge**（第 4 个 stdlib，纯语言零新 C）：GPIO V2 line（gpio_input/
  gpio_output/gpio_input_edge/gpio_request + read/write/wait/event，592B 布局按
  linux/gpio.h offsetof 实测；OUTPUT 初始电平经 attr OUTPUT_VALUES）+ I2C（i2c_open/
  i2c_read_reg/i2c_write_reg，write-then-read 两笔事务）+ serial_open（open rw +
  tty_config raw 一站式）+ PWM sysfs（pwm_setup/enable/set_duty，open 通道写失败不杀
  进程）+ 示例（m60_serial_pty **x86 实跑 PTY 真内核串口双向 loopback**、m60_gpio/
  m60_i2c/m60_pwm 真板段 SKIP）+ dev_s3 布局常量断言单测—— commit `f5b1b03`。
- **S4 双模式同步**：pxi 白名单 +5 + ibuiltin 纯转发 5 分支（参数预检返回 Err、设备失败
  -1/false+errno 透传不包装）→ `bootstrap/pxi` 重建；dev_s4 编译/pxi/qemu-aarch64 三态
  断言全过、双模式输出逐字节一致（PTY/termios/poll/fcntl/us 时钟跨架构一致）；
  hello/fib 双模式 + m59 math_s1 解释 + m57_s1 编译回归 PASS —— commit `f7e21b5`。
- **S5 文档收口**：spec §8.18 + §10.3 std.edge 行、MINI_SUBSET §十三.5、ROADMAP M60
  勾选（候选池移除 M60）、GAP_ANALYSIS §三 #1–#5 ✅ + §七 M60 ✅（本 commit）。
- 语义要点：失败 -1/false + os_errno()（延续 M57 fd 原语，可检查不杀进程）；参数错误
  px_error 终止；fd_wait 超时返回空 list 非错误；GPIO 布局按 2024+ 内核（offsets u32
  版）书写，旧内核需按目标头调整；真板物理回归（#6）候选待硬件。
  样例与验证：`examples/m60_dev/`（dev_s1~s4.px + verify_s1~s4.sh）+ `stdlib/edge.px`。

### M59 · 数学与随机补齐（C libm 内置，14 函数 + 2 常量）✅

- **规划**：`docs/M59_PLAN.md`（前置实测复核：现有数学内置仅 abs/min/max/sum/sqrt/pow 六枚；
  sin/cos/tan/atan2、floor/ceil/round、log/log10/exp、random 全缺；libm 已 include + `-lm`
  已链接 → 零新依赖）。
- **S1 三角 + pi**：`sin`/`cos`/`tan`（弧度）/`atan2(y,x)`（先 y 后 x）+ `pi` 常量
  （本地宏 `PX_PI` 全精度）—— commit `8a85834`。
- **S2 取整/对数 + e**：`floor`/`ceil`/`round`（C99 round，.5 远离零；返回 float 与 sqrt
  一致）+ `log`(自然对数 ln)/`log10`/`exp` + `e` 常量（`PX_E` 全精度）；域错误透传 C 语义
  不终止（log(-1)→NaN、log(0)→-inf、exp(1000)→+inf）—— commit `93eb9a9`。
- **S3 随机（splitmix64）**：`random()`→float∈[0,1)（53 位尾数均匀）、`random_int(n)`→
  int∈[0,n)（n>0）、`random_seed(s)` 设种子后同 seed 同序列；确定性 64 位 PRNG 不依赖 C
  rand 的 glibc/musl 平台差异（静态二进制 + aarch64 序列可复现）；默认种子首次调用惰性
  初始化（realtime ns ^ pid）—— commit `bddd953`。
- **S4 双模式同步**：pxi 解释器白名单 +15（含补平 sqrt 编译有解释无的不对称）+ `pi`/`e`
  常量种子（读宿主全局，规避浮点字面量 6 位截断）+ `ibuiltin.px` 数学分发 5 分支与
  helper（参数预检返回 Err、域错误 NaN/inf 透传）→ `bootstrap/pxi` 重建；编译/解释/
  qemu-aarch64 三态断言全过、双模式输出逐字节一致、splitmix64 序列 x86==aarch64 逐位
  一致；hello/fib 双模式 + m57_s1/m58_s1 回归 PASS —— commit `3f7e434`。
- **S5 文档收口**：spec §10.2/§10.3、MINI_SUBSET §十三.4、ROADMAP M59 勾选、
  GAP_ANALYSIS 数学缺口勾销（本 commit）。
- 语义要点：参数个数/类型错误 → px_error 终止（编程契约，与 abs/sqrt 一致）；域错误 →
  NaN/inf 透传不终止；atan2 先 y 后 x；floor/ceil/round 返回 float 需 int() 转整数；
  pi/e 全精度常量。样例与验证：`examples/m59_math/`（math_s1~s4.px + verify_s1~s4.sh）。

### 文档 · 能力差距分析 + 候选主线排期（docs/GAP_ANALYSIS.md + ROADMAP 远期方向）

- 新增 `docs/GAP_ANALYSIS.md`：树莓派/边缘设备 与 2D/3D 游戏两条用户线的能力差距清单
  （事实基准：~257 内置函数清单 / stdlib / spec / ROADMAP / MINI_SUBSET §十三 / M57/M58
  实测产物）。
- 核心结论：树莓派线「能力已开、缺厚度与真板验证」（GPIO 真正控制、边沿中断 poll/epoll、
  串口 termios、SPI/PWM 封装、us 级时钟、真板物理回归）；2D 游戏「几乎未开」（缺图形/
  输入/数学基础——终端 ASCII 半通、帧缓冲原语零、SDL2/raylib 0→1 未做）；3D 明确不做
  原生绑定、唯一路径 = raylib C 绑定（前置 FFI 外部库绑定验证）。
- 历史任务覆盖核对：#37 → M57 S1–S4 **100% 闭环**；#36 → 主体完成（pxc 交叉产出 aarch64
  静态 ELF + qemu 验证），遗留 = 原生 aarch64 pxi 未做（如需「板子跑解释模式」）+ Mahesvara
  加分项已随拆仓移出本仓。
- ROADMAP 远期方向新增「候选主线排期」：M59 数学与随机补齐（小）→ M60 边缘 stdlib +
  设备小内置（中）→ 候选 FFI 外部库绑定验证 / 真板物理回归（需硬件）。

### M58 dogfood 闭环 · pxhwmond webhook dry-run 解禁为真发（HTTP Err 语义落地）

- **背景**：M58 pxhwmond 的 webhook 通知因 HTTP 客户端网络失败即 panic（§十三 #1/#2）
  只能 dry-run 报文落盘；HTTP/S3 网络失败 → `Err(result)` 修复（§十三.1/.2）后语言面已
  允许 daemon 内安全发起网络请求 → 本条目把 dry-run 正式解禁为真发，完成 dogfood 闭环。
- **改动（examples/m58_hwmond/）**：
  - `notify.px`：`webhook_dryrun` → `webhook_send`——`http_request(url,"POST",JSON,
    {Content-Type:application/json},{timeout_ms:3000,retries:0})` 真发；网络失败（返回
    `Err`，以 `type(resp)=="result" and resp.is_err()` 判别、`resp.err()` 取文本）→ 发送
    日志 `sent:false`+`err` 落盘、进程不死；HTTP 2xx → `sent:true`+`status`、非 2xx →
    `sent:false`+`http_status=…`。env `PXHWMON_WEBHOOK_DRYRUN_LOG` → `PXHWMON_WEBHOOK_LOG`
    （发送结果日志）。
  - `main.px`：调用点 `webhook_dryrun` → `webhook_send`；头注释同步。
  - 新增 `webhook_mock.px`：PuXian 自举的本地 webhook 接收 mock（`http_serve` handler 把
    method/path/body 落盘 JSONL），verify_s3.sh D1 真发验收用。
  - `verify_s3.sh`：D1 成功路径——阈值触发 3 轮 → mock **实收 3 条 POST /alert**（body 含
    alert）+ 发送日志 3 条 `sent:true`；D2 失败路径——webhook 指向 `127.0.0.1:1`（连接
    拒绝）→ daemon 3 轮跑完退出码 0、3 条 `sent:false`+`err`（§十三 #1 修复实证，原先
    会 panic 杀进程）；A-C 段改端口就绪探测 + client 重试（daemon 冷启动偶发 >2.5s）。
- **验证**：verify_s3.sh PASS（A-C 状态页响应头 + D1 mock 实收真发 + D2 失败不 panic）；
  verify_s1/s2/s4 回归 PASS（见提交记录）。
- 记录：MINI_SUBSET §十三.3、m58 README 边界更新（dry-run 描述 → 真发 + Err 语义闭环）。

### 语言面修复 · S3/MinIO 客户端网络失败 → Err(result)（与 HTTP 客户端同源收口）

- **问题**：`s3_put`/`s3_get`/`s3_delete`/`s3_list`（M37 引入）网络失败（协议不支持/
  建连失败/TLS/IO/连接中断）**静默返回 false/null/空 list**——无错误可见性，调用方无法
  区分「网络挂了」与「服务端拒绝」（与 HTTP 客户端 px_error 杀进程同源、但更隐蔽：
  M37 的 px_s3_exec 建连失败 `return 0` 无声无息）
- **修复（runtime/runtime.c，零新增内置函数）**：上述 4 个 S3 客户端函数**网络失败统一
  返回 `Err(result)`**（消息 `"net: ..."`），进程不终止，调用方可 `is_err()`/`?`/unwrap
  处理；**成功/应用层语义不变**：服务器正常响应时 put/delete → bool、get → string(200)
  或 null(其他)、list → keys 列表（403/404/500 等应用层状态码非网络失败，走原语义）；
  参数个数/类型错误（编程契约）仍终止。实现：`px_s3_exec` 增加错误缓冲输出
  （`px_net_fail` 填 errbuf，沿用 §十三.1 HTTP 修复的 helper）；endpoint 增加协议校验
  （非 http/https → Err，原来静默当明文 HTTP 处理）；建连失败/h_exchange 中断分别填
  `net: 连接 ... 失败` / `net: S3 请求失败: 连接中断`
- **验证**：新增 `examples/s3_neterr_result.px` 自检：连接拒绝/协议不支持 → 4 函数全
  is_err=true + 进程存活 ALIVE + 退出码 0；本地最小 S3 mock（http_serve）应用层
  200/404/204 → true/body/null/keys 语义不变（非 Err）；m37_s3.px 成功路径全链路回归
  M37_S3_OK（含 SigV4 签名 mock）；http_neterr_result.px 回归 PASS（共享 h_exchange
  helper 未破坏）；pxi 重建后解释模式同 PASS
- 记录：MINI_SUBSET §十三.2（§十三 #1/#2 属 HTTP 已修；本条目把 S3 静默失败一并收口为
  「网络错误可检查」，语言面网络 I/O 失败语义趋同）

### 语言面修复 · HTTP 客户端网络失败 → Err(result)（M58 dogfood 欠账 #1/#2 根因）

- **问题**：`http_get`/`http_post`/`http_request`/`http_unix`/`http_get_stream` 网络失败
  （解析/建连/TLS/IO/重定向/协议不支持）直接 `px_error` → 打印「运行时错误:」并 exit(1)，
  无错误返回；spawn 协程内网络调用失败会杀整个进程（MINI_SUBSET §十三 #1/#2，M58
  pxhwmond dogfood 暴露：webhook 通知只能 dry-run 落盘、不能真发网络）
- **修复（runtime/runtime.c，零新增内置函数）**：上述 5 个 HTTP 客户端函数**网络失败统一
  返回 `Err(result)`**（消息 `"net: ..."`，保留原 px_error 文案），进程不终止，调用方可
  `is_err()`/`?`/unwrap 处理；**成功返回值不变**（http_get/post → body 字符串；
  http_request/unix → dict{status,headers,body}；get_stream → bool）；参数个数/类型错误
  （编程契约）仍 px_error 终止；HTTP 应用层状态码（404/500）仍由 dict.status 返回（不算
  网络失败）。实现：底层 `px_http_once`/`px_http_request` 增加错误缓冲输出
  （`px_net_fail` 返回 NULL 信号，修复一处 NULL 未检解引用隐患）；`hparse_url` 改返回
  错误码不再终止；builtin 用 `px_net_err` 就地构造 Err
- **验证**：新增 `examples/http_neterr_result.px` 自检（连接拒绝/协议不支持/unix socket
  不存在 → is_err=true + 进程存活 ALIVE + 退出码 0）；成功路径不变（本地 HTTP 状态页
  http_get/http_post 返回 string、http_request 返回 dict status=200）；M58 verify_s1/s2/s3
  回归全 PASS；aarch64 交叉链路 verify_s4 复跑
- 连带：#2（spawn 不隔离 panic）在该场景**根除**——网络失败已不 panic；协程内其他运行时
  错误（除零等）仍不隔离（MINI_SUBSET §十三.1 部分缓解记录）

### 新增 · M58 首个 dogfood 真实应用：pxhwmond 硬件健康守护 daemon（examples/m58_hwmond，见 docs/M58_PLAN.md）

- **首个 dogfood 里程碑**：用 PuXian 写**真实边缘应用**——单静态二进制硬件健康守护 daemon，
  把 M57 能力（fd 原语数据通道 / mmap 活映射 / aarch64 交叉编译）揉进完整真实程序，
  x86 实测 + aarch64 交叉 qemu 验证（1→1.0n 验证闭环）
- **多文件 import 工程**：main/collect/shm/serve/notify 4 模块拆分（每文件 <500 行），
  `import "collect.px"` 相对路径源码模块编译模式实测可用
- **采集（M57 fd 通道主路径）**：/proc/stat CPU 差值 / meminfo / loadavg / uptime /
  net/dev（非 lo 汇总）+ 温度 hwmon/thermal 条件探测降级（缺 → `temp=na` 不崩）
- **mmap MAP_SHARED 活映射 IPC**：快照区 + 控制区 4096B 共享文件；`--dump` 外部活读、
  控制区命令通道**双向可见**（外部写 → daemon 下轮快照回显 `ctl=`，verify_s2 轮询实证）
- **手写最小 HTTP 状态页**（D4 决策：不引 px_serve docroot）：`GET /healthz` JSON +
  `GET /` HTML 表格 + 404；**显式响应头** Content-Type/Content-Length/Connection/Server
  （M57-S7「vhost 响应头丢失」教训自验，verify_s3_client http_request 逐头断言）
- **监控 + 自愈 + 通知（MONITORING/P0 落地载体）**：/healthz 心跳 + mmap 实时快照 +
  run.sh 崩溃自动重启（kill -9→137→attempt 递增）+ 阈值告警（内存/负载/温度 env）→
  告警日志 + webhook dry-run 报文落盘
- **aarch64 交叉 + qemu**：多文件 import 工程 `pxc build --no-quic` 交叉 aarch64 静态
  产物（2.5MB）→ qemu-aarch64 `--once` 采集真实 /proc（mem_total 与 MemTotal 一致，
  跨架构同源实证）；`--no-shm`/`--once` 保证自检可退出不裸奔
- **语言欠账记录（MINI_SUBSET §十三）**：http_post 失败即 panic + spawn 不隔离 panic
  （→ webhook 只能 dry-run，真实网络发送待语言补错误返回，最优先）；int() 前缀截断；
  `{}` 空 dict 不可靠（用 json_parse("{}")）；import 只合并 def 不执行模块顶层；mmap
  固定 PROT_RW 须 O_RDWR fd
- 验证：verify_s1–s4.sh 全 PASS（s1 mem 精确对拍 / s2 双向活映射 / s3 HTTP 响应头 +
  告警 / s4 崩溃自愈 + 交叉 qemu）；M57 s1/s3 回归 PASS；M58 commits：S1 `1694720` /
  S2 `c821005` / S3 `b92be61` / S4 `25aa4bb`（规划 `4000dc3`）

### 仓库治理 · Mahesvara 迁出至独立私有仓库（开源 / 私有物理隔离）

- 仓库外私有生产应用 Mahesvara（私有 webserver 系统）已从本仓库 `git rm` 并迁至**独立私有仓库**维护；
  本仓库只保留开源内容，`git grep Mahesvara`（tracked）零残留
- `.github/workflows/ci.yml` 移除 `wsweb` job（生产应用回归）；README/README.en/ROADMAP/spec/CHANGELOG/
  M53/M54/M57_PLAN/CONTRIBUTING/.gitignore/examples 中 14 文件 39 处 Mahesvara 引用一律中性化
  （「外部生产应用」表述；repro_h2_vhost/repro_tls_no_sni 标题 `[Mahesvara-blocker]`→`[dogfood-blocker]`，
  脚本自包含语义不变）
- 背景：Mahesvara 为私有系统，与开源 PuXian 同仓导致提交边界不清（曾多次直推并被 revert）；
  物理拆仓后「改 PuXian」与「推 Mahesvara」权限隔离，PuXian 侧对 Mahesvara 的参与回归 PR 流程；
  git 历史完整保留（未清史）；Mahesvara 曾以真实应用身份抓出的 H3/并发类 bug 已在 CHANGELOG M53–M56 留存

### 新增 · M57 边缘设备层支持（Linux 用户态，见 docs/M57_PLAN.md）

- **fd 原语内建（S1）**：`open(path[, mode]) → fd`（mode `r/w/a/rw/w+` → O_*）、
  `close(fd) → bool`、`ioctl(fd, request[, arg]) → int`（arg 三形态：缺省→NULL /
  int 直传 / bytes·str 就地 in/out buffer，_IOR 类内核直接填充同对象）、
  `os_errno() → int`（线程局部 errno）——清歌方案"只缺 ioctl"核查后修正：
  runtime 文件 IO 原为路径式（read_at/write_at 内部 open 用完即关），语言面无持久
  fd 句柄，故一并补 fd 原语打通「打开设备 → ioctl → 关闭」闭环
- **fd 数据通道 + mmap 活映射（S2）**：`read(fd, n) → bytes` / `write(fd, data) → int`
  （read/write(2) 直通，EINTR 重试）；`mmap(fd, len[, offset]) → bytes`（PROT_RW +
  MAP_SHARED，GC 自动 munmap，失败 -1+errno）；`munmap(bytes) → bool` 显式解除；
  `mem_write(map, offset, data) → int` 就地写映射区（bytes_set 是 COW 复制语义，
  帧缓冲/共享内存写像素必备）；LXObject 位域新增 is_mmap（px_obj_free 对 mmap
  bytes 走 munmap 而非 xfree）
- **GPIO/I2C 示例 + 真内核替身验证（S3）**：m57_s3_gpio.px（GPIO_GET_CHIPINFO_IOCTL
  buffer 解析）、m57_s3_i2c.px（I2C_SLAVE int 形态直传从地址）、m57_s3_devctl.px
  （loopback 网卡 ifreq + PTY TIOCGPTN 全真实内核硬断言）——LD_PRELOAD mock 因
  pxc 静态链接不可行，改内核自带用户态可访问设备走同胶水路径，验证力度更强
- **aarch64 交叉编译 + runtime 裁剪（S4）**：`pxc build --no-quic [--cc] [--mbedtls-lib]
  [--sqlite-obj]`；runtime PX_NO_QUIC 条件编译（7 处）+ musl 兼容 5 点（execinfo
  条件 / GC aarch64 寄存器扫描分支 / getcontext→内联汇编 SP+setjmp spill /
  close_range 循环）；tools/cross_aarch64.sh（mbedtls 3.6.2 + sqlite3 交叉入库）；
  qemu-aarch64 跑静态产物设备层 ioctl 与 x86 结果一致（asm-generic ioctl 码跨架构
  实证一致）
- **pxi 重建解释同能力（S5）**：interp.px 白名单 +10 + ibuiltin.px 纯转发分支 →
  `pxc build selfhost/interp.px` 重建 bootstrap/pxi；examples/m57_s5_pxi_smoke.px
  双模式输出一致；capability 双模式各 253 PASS 输出逐字节一致；diffcheck --all /
  --errors 全绿；自举证明 B.c==golden 逐字节一致
- 目标场景：树莓派/网关/盒子等 Linux 边缘设备层（单静态二进制免 Python 环境）；
  裸机 MCU（STM32/ESP32）因 runtime 含 GC/线程/动态值与无 OS 架构不符，明确不做；
  通用动态 FFI（dlsym）待「任意 C 库即插即用」真需求再上
- **里程碑闭环（S6 文档收尾）**：spec §8.17（边缘设备层 fd 原语 / 数据通道 / mmap 活映射 API 文档）+
  ROADMAP（主线表 M57 行 + 能力基线「边缘设备层」+ 远期 M57 段移入完成）+ README / README.en
  （特性表「🔌 边缘设备」+ 里程碑表 M57 + 示例列表 m57_s1–s5）+ 本 CHANGELOG 同步定稿；
  M57 全部代码 commit：S1 `57bb9d7` / S2 `f71b28e` / S3 `8f6e615` / S4 `bbffcd5` / S5 `62d9275`

### 修复 · M57-S7 vhost handler 自定义响应头丢失（外部生产应用 BUG_REPORT）

- 现象：`vhost(host, handler)` 返回 `dict{status,headers,body}` 时，headers 除
  `Content-Type` 外全部丢失 → 301 无 `Location`（浏览器不跳转）、`Cache-Control`
  缓存头失效、`Set-Cookie` / CORS 头均受阻
- 根因：`runtime.c px_vhost_normalize` 仅 `px_dict_get_ci("Content-Type")` 单头透传，
  未透传其余响应头（最小复现位于仓库外生产应用私库，本次以仓库内自包含复现等价验证）
- 修复：`px_vhost_normalize` 增加 `extra` 出参 + 白名单透传（`Location` /
  `Cache-Control` / `Content-Disposition` / `Content-Language` / `Set-Cookie` /
  `X-Robots-Tag` / `Access-Control-*` CORS 头），键、值任一含 CRLF 即整体丢弃（防注入），
  extra 写满安全截断（响应头缓冲 2048 兜底）；Content-Type 仍走独立通道；调用侧
  extra 缓冲 256→1024（X-Request-Id 之后追加）
- 新增回归：`examples/m57_s7_vhost_headers.px`（编译模式 ALL OK）——覆盖①白名单透传
  ②非白名单头丢弃 ③值含 CRLF 丢弃 ④键含 CRLF 不崩溃 ⑤Content-Type 独立通道；
  pxi 解释器不含 vhost builtin（Mini 子集边界），修复在 runtime C 层对编译部署形态生效
- 回归（runtime 变更全量）：pxi 重建 9,050,296B；capability 解释 + 编译双模式各
  253 PASS 输出逐字节一致；diffcheck --all / --errors 全绿；自举证明 B.c 与
  golden/compiler.c 逐字节一致（6381 行）；m31_vhost（vhost/限流/CORS）ALL OK


### 路线图 · M57 内容重定向（HTTP/3 深度生产化 → 健壮性加固 → 边缘设备层支持）

- **HTTP/3 深度生产化剩余项**（QPACK 动态表前缀 / 服务端主动迁移·immediate migration /
  深度互操作扩展）**降级为搁置**：Chrome 2021 起禁用 QPACK 动态表（容量=0）、immediate
  migration 连 ngtcp2 上游都未实现、深度互操作边际收益递减 —— 均无真实用户场景（详见
  `docs/ROADMAP.md` §三「搁置」评估注记）
- **HTTP/3 / QUIC 健壮性加固**（曾于上版设为 M57 候选主线）**同步降级为搁置**：H3 目前
  无真实用户（自签证书下浏览器退回 HTTP/1.1，不走本栈），给无人使用的栈做 fuzz / 并发
  审计价值前提不成立（1→1.01 而非 0→1）；待 H3 出现真实用户（如生产应用配真证书公网/
  浏览器实测）再捞回
- **M57 重定向为主线下一条里程碑「边缘设备层支持（Linux 用户态）」**：与清歌嵌入式讨论
  的落地结论（PuXian 只能到树莓派/网关/盒子等 Linux 边缘设备层；裸机 MCU 架构不符，
  明确不做）——相对 H3 打磨属 **0→1 开新使用域**（物理世界接口）。S1 ioctl 胶水内建
  （约 100–200 行 C，通吃 i2c/spi/gpio/tty/网卡）→ S2 mmap/munmap 设备映射 → S3
  GPIO/I2C 示例 + x86 ioctl mock 验证 → S4 aarch64 交叉编译 + qemu 验证 + runtime 裁剪
  开关 → S5 pxi 重建 + capability/diffcheck/自举全绿 → S6 文档；通用动态 FFI（dlsym）
  等「任意 C 库即插即用」真需求再上

### 新增 · M56 runtime http_unix 内建（外部生产应用配套，非主线 HTTP/3 里程碑）

- `http_unix(sock_path, url_path, method[, body[, headers]]) -> dict{status, headers, body}`
  —— Unix domain socket HTTP 客户端内建（本地服务 / LLM 网关 / 容器 daemon 调用）：
  每次新建 AF_UNIX 连接、`Connection: close` 用完即关；收发超时 180s（本地长响应）；
  带 body 且未显式带 Content-Length 时自动补（Content-Type 可被 headers 覆盖）；
  响应解析复用 h_exchange（与 http_get/http_post 同解析器）
- 引入背景：外部生产应用 LLM 接入词元缓存网关（unix socket 通道，key 零落盘）；
  由清歌提交（runtime/runtime.c +73；仓库外应用侧 llm.px / translate.px / main.px），
  编号与主线 HTTP/3 里程碑并行插队 —— 本条目为 PuXian runtime 侧文档补齐

### 新增 · M54 HTTP/3 生产化（见 docs/M54_PLAN.md）

- **TLS 1.3 会话恢复（1-RTT resumption）**：server SSL_CTX 开启 stateless session ticket；
  语言 API `quic_connect_resume(ip,port,alpn,session)` / `quic_session_save(conn)` /
  `quic_conn_resumed(conn)` —— 二次连接确为 1-RTT 恢复（resumed=true）
- **0-RTT early data**：server `SSL_CTX_set_max_early_data` + 收包路由登记客户端 Initial
  DCID（修复 0-RTT 长头包被丢）；语言 API `quic_0rtt_save` / `quic_connect_0rtt`（握手完成
  前即可 `quic_send`，tp 缺失自动降级 1-RTT）/ `quic_0rtt_rejected` /
  `quic_conn_handshake_done`；H3 0-RTT 子集 `h3_server_listen_stateless`（静态表，幂等 GET）
- **连接迁移**：`quic_migrate(conn, ip, port)` 新建 UDP fd 换源（NAT rebinding 语义）；
  `quic_conn_path` / `quic_conn_local` 观测对端/本地地址；server 完成新路径
  PATH_CHALLENGE 验证并跟随（同 conn 无重握手，echo 跨迁移续传）
- **BLOCKED_STREAMS 流上限协商**：`quic_set_max_client_streams(listener, n)` 经 transport
  params 下发对端；`quic_extend_max_streams(listener, add)` 发 MAX_STREAMS 放行；
  `quic_streams_left(conn)` 剩余配额；`quic_open_stream` 达配额返回 -206（原吞为 -1）
- bootstrap/pxi 重建：解释器同能力支持 M54 全部新内置（S5 全量回归 14 项端到端全 PASS）

### 新增 · M53 HTTP/3 三栈合一 WebServer（见 docs/M53_PLAN.md）

- `px_serve(port, docroot, timeout, {http3: true | {port?, cert?, key?}})` 单调用同时托管
  HTTP/1.1（TCP）+ HTTP/3（QUIC/UDP，缺省同端口），HTTP/1.1+HTTP/2+HTTP/3 共用同一
  vhost/路由/限流/访问日志/静态/.px 管道；Alt-Svc 自动通告；SIGTERM 优雅关闭
- 语言 API：`h3_server_listen(port, cert, key)`（HTTP/3 管道托管 listener）、
  `quic_h3_listen(port, cert, key)`（QUIC raw listener，多连接托管地基）
- QUIC 服务端多连接托管：单 fd 收包路由（DCID → 连接入包队列）+ 每连接处理线程 + PEM 证书加载
- 请求管道与传输解耦：`PxHttpOut` 输出抽象（HTTP/1.1 与 HTTP/3 共用，纯重构响应字节零变化）
- **HTTP/3 外部互操作打通**：aioquic（第三方独立实现）请求 PuXian H3 服务 200；修复 4 个
  协议级缺陷（fin-only 流活性、无 body 请求、响应头字段名小写、响应流 FIN）
- bootstrap/pxi 重建：解释器同能力支持 h3_server_listen / px_serve http3（S5 全量回归）

### 修复 · M55 并发安全（issue #2）

- 全局符号表/GC root 扫描互斥：px_set_global / px_get_global / struct 方法查找全程持锁、
  g_len 原子化、stop-the-world 持锁扫 root —— 修复并发写全局变量 + GC 竞态崩溃

## [0.1.0] - 2026

首个可公开版本：语言已自举（PuXian 编译器由 PuXian 自身编写），
工具链零 Rust 依赖（`tools/pxc` + 仓库自带自举二进制），静态二进制分发。

### 语言与工具链（自举核心）

- 语言：缩进语法（Python 脸）、渐进类型、`struct/enum/trait/impl`、
  `Option`/`Result` + `?`/`!`、`match`、推导式、生成器、闭包（无捕获面）、
  字符串插值 `${expr}`、管道 `|>`、可选链 `?.`、空合并 `??`
- 双后端：tree-walking 解释器（秒起）+ C 转译编译（`gcc` → 静态二进制），
  双模式行为一致（`selfhost/diffcheck.sh` 逐字节对拍）
- 自举闭环：`compiler.px` 编译自己 → `B.c == golden/compiler.c` 逐字节一致（三步证明）
- 工具：`tools/pxc`（build/run/lex/parse）、`tools/pxpkg`（包管理 + semver + lockfile）、
  `tools/routegen`（文件即路由生成器）
- Mini 子集规范（`docs/MINI_SUBSET.md`）：自举期间语言面锁定 + 已知限制清单

### 运行时与网络

- 内置：AES/XML/zip/gzip/正则/base64/JSON（含 JSONPath）/SQLite/协程/事件总线/沙箱
- WebServer 生产化：HTTP/1.1 服务端 + 路由/中间件/限流/访问日志/虚拟主机（SNI）/
  静态文件/Range/ETag/Session/Cookie/优雅关闭/进程池热更新
- HTTP/2：h2c + HPACK/Huffman + 多流 + over-TLS（ALPN h2）
- WebSocket / SSE（含自动重连、心跳、广播）、UDP、S3/MinIO（SigV4）
- HTTP/3 / QUIC（RFC 9114/9000/9204 预研落地）：
  - M46 QUIC 传输层（ngtcp2 + quictls 静态编译，握手/双向流/单向流）
  - M47 HTTP/3 语义层（QPACK 无动态表 codec + HEADERS/DATA 帧 + 请求/响应对拍）
  - M48 QPACK 完整 codec（RFC Huffman + 静态表 99 项字节精确）
  - M49 QPACK 动态表 + SETTINGS 控制帧（RFC 9204 全量会话）
  - M50 多路复用（单连接多双向流并发请求/响应，per-stream 缓冲）
  - M51 QPACK 会话上真实 QUIC 单向流（控制/编码器/解码器三流 + SETTINGS 协商）
  - M52 QPACK 解码器流 ack 闭环（Section Ack / KRC 推进 / 编码表驱逐安全化）

### 生态

- 仓库外生产应用：用 PuXian 写的第一个生产应用（HTTP + SQLite），dogfooding 验证（代码维护于独立私库）
- 80+ 示例程序 + 里程碑验证脚本（`examples/`）

[Unreleased]: https://github.com/NanzhanGroup/PuXian
[0.1.0]: https://github.com/NanzhanGroup/PuXian

### M89-S3A-A0 · VM 骨架（runtime/vm.h+vm.c + selfhost/bc_emit.px）

> 完成（2026-09-08）：S3-A 骨架第一步 —— 单一执行引擎（显式帧 + 平坦字节码 VM）C 侧骨架 + 发射器骨架。
> - **runtime/vm.h**：指令集编号冻结（54 op：A 加载/槽 5 · B 一元/二元 22 · C 容器/字段 11 · D 调用/返回 6 ·
>   E 控制流/错误 7 · F 生成器/并发 2；PXM_MAX=55）；8B 定长指令 PxInst{op,fl,a,b,c}；常量池 PxK；
>   PxVMFunc（name/arity/ndefault/nslots/bc/mod，upvals 预留 P2）与 PxBCModule（K/N/G/funcs/top）；
>   显式帧 PxFrame/PxVmState；API（px_op_name/px_vm_state/px_vm_entry/px_vm_run_func/px_vm_run_module/
>   px_vm_new_func）。跳转 off 语义=相对「下一条」，发射器按 (目标-(pc+1)) 计算。
> - **runtime/vm.c**：__thread PxVmState；帧栈 push/pop（槽数组 calloc，RET 弹帧释放）；D2 trampoline
>   px_vm_entry（ctx=PxVMFunc*，px_call/px_method/px_spawn 零改动兼容）；解释循环最小子集已跑通
>   （LOADK/IMM/MOV/SRCLINE/JMP/JMPT/JMPF/RET/RET0/HALT），其余 op 分发 px_error「指令未实现」；
>   `gcc -O2 -pthread -Wall -Wextra -c` 零告警；临时冒烟 7 例 ALL PASS（IMM→RET 常量、RET0→null、
>   JMPF 分支真假两路、JMPT 跳过、LOADK 走模块 K 池）。
> - **selfhost/bc_emit.px**：发射器骨架（PuXian，D1 自举）—— BCModule 目标形态 + 表构建工具
>   （bc_k_add/bc_n_add/bc_g_add/bc_new_module）+ emit 入口占位（stmt/expr 逐 tag A1 起）；`px parse` 通过。
> - 集成：vm.c 暂未入 rt_src_files / compiler.px `bc` 子命令随 A1 首个发射切片接入（golden 再生成同批，
>   避免重复 3.5 分钟全量重链）；回归 hello（while 求和）px build + px run 双轨一致 rc=0。

### M89-S3-A2 · 运算批 + 顶层顺序执行：发射器→可链接 C→VM 端到端闭环打通

> 完成（2026-09-08）：VM 化 S3-A 进入**首个可运行字节码闭环**（A1 只是 dump 验证；本步起
> BCModule 可静态输出为 C、gcc 链接 runtime（含 vm.o）真正跑 Top，顶层值可对拍断言）。
> - **runtime/vm.c**：B 表一元/二元运算全量执行（NEG/NOT/BITNOT、ADD..GE、BITAND..SHRU，调现
>   px_* C 函数，语义=旧轨）；**vm.c/vm.h 入 tools/px rt_src_files**（rtcache 重建含 vm.o）。
> - **selfhost/bc_emit.px**：Binary 算术/比较/位运算批（两地址）+ Unary 一般化接线；新增
>   bc_binop_op/bc_unop_op；**emit-c（bc_emit_c_program）**= BCModule→可链接 C 静态初值
>   （PxK/N/G/bc/funcs/mod 前向引用）+ main 跑 px_vm_run_module（PX_BC_DUMP 环境变量打印非函数
>   全局，A 阶段对拍）；bc_cli.px 增 `--emit-c`。
> - **修复 A1 遗留 bug**：bc_emit_if 有 else/elif 时 JMPF 假跳落点错（落 then 尾 JMP → else 永不
>   执行）；修正为假值落 JMP 之后（else 起点）。bc1.dump golden 回归不变。
> - **验证**：cases_bc/bc2.px+dump golden（Add/Sub/Mul/IntDiv/Mod/Pow/比较/位/一元/if-else/while）；
>   examples/m89_a2/bc_run.sh + bc2_verify.sh 端到端 **16 断言 ALL PASS**；bc1.px emit-c 跑通；
>   vm_selftest(A1) 复跑 ALL PASS；px build hello C 产物与 pxi 输出一致（rtcache 重建无回归）。

### M89-S3-A3 · 控制流批：短路 and/or/??、IfExpr、while+break/continue（纯发射器）

> 完成（2026-09-08）：短路与分支全部映射到既有 JMPT/JMPF/EQ/LOADK —— VM 无新增 op。
> - bc_emit.px：Binary And/Or 短路（D5 返回操作数）；NullCoalesce（EQ d,null→JMPF 仅 null 替换）；
>   IfExpr（JMPF→else / JMP→end）；Break/Continue（func.loops 循环上下文栈，break→end、
>   continue→cond 起点，body 发完统一回填）；bc_emit_while 重构支持 break/continue。
> - 验证：bc1/bc2 dump golden 回归不变；bc3.px dump golden + emit-c 端到端 11 断言 ALL PASS
>   （and/or 返回左/右操作数、?? 仅 null 替换、IfExpr 双支、while continue 跳 3 + break 停 8 →
>   sum=25 / flags=5）。

### M89-S3-A4 · 调用闭环：CALL/嵌套帧/递归深链/main 调用约定（hello.px 三轨对拍）

> 完成（2026-09-08）：**函数调用闭环打通** —— px→px 调用压显式帧不回 C 递归（D3 深链
> 安全），native/旧 C 产物经 px_call（C 递归一层）；PX_FUNC.fn==px_vm_entry 判断天然兼容
> 混合调用。
> - runtime/vm.c+vm.h：PxFrame 增 ret_dst（CALL 返回写 caller 槽）；RET/RET0 弹帧回传；
>   CALL 分派（VM 函数手动压帧 / native·旧 C px_call），参数连续区槽排布。
> - bc_emit.px：bc_emit_call 抽取 + Call/Pipe 表达式接线 + main 调用约定（Top 末尾 GETG
>   main→CALL→RET，run_module 返回 main 结果）+ emit-c driver 退出码转换（对齐 codegen）。
> - 验证：bc4.px（add 多参/fib(10)=55 递归/deep 2 万层深链/main 返回）main 退出码=63
>   ALL PASS；嵌套帧 CALL 手测 PASS；**examples/hello.px 三轨 stdout 逐字节一致
>   （VM=旧 C=pxi）**——真实程序（main/局部/and 短路/if-else/print native/Pipe to_upper）
>   在 VM 上跑通；bc1-4 dump golden 回归不变。
> - 默认参数/CALLM 方法调用/for-in 依赖容器 → S3-B（记录在案）。

### M89-S3-A5 · 错误传播：TRY/FORCE + 错误现场追踪（S3-A 骨架段收口）

> 完成（2026-09-08）：VM 化 S3-A 全段（A0 骨架→A5 错误）收口。
> - runtime/vm.c：TRY（?）——Err/null 就地返回传播（RET 语义回传，顶层帧→run_module→
>   driver 报错退出）/Ok 解包覆写；FORCE（!）——Err/null px_error / Ok 解包；解释循环
>   逐帧 px_srcfunc + SRCLINE 逐条 px_srcline → runtime px_error 文案格式对齐 M72-S2
>   （实测 "运行时错误 [main 行3]: force unwrap Err: boom"）。
> - bc_emit.px：表达式 Try→TRY、ForceUnwrap→FORCE。
> - 验证：bc5（Err 两跳 ? 传播 → exit 1 + stderr 除零）、bc6（FORCE Ok + null ? 传播 →
>   exit 0）ALL PASS；bc1-4 dump golden 回归；bc2/3/4 verify + hello 三轨复跑全绿。
> - S3-A 收口：A0-A5 完成，hello.px VM=旧C=pxi 三轨逐字节一致；fib.px 依赖容器 → S3-B。

### M89-S3-C1 · VM 化自举收敛起步：runtime 内嵌 NUL 字符串修复 + 编译器全量 BCModule 发射打通

> 完成（2026-09-08）：C1 自举第一道阻塞清除 + 编译器字节码镜像基线建立。
> - **根因**：M83-S1（GAP-STR-1-B1）修 len() 尊重 str.len 字节边界，但 px_index/px_slice
>   仍用 strlen 版 px_unicode_len（内嵌 NUL 截断）→ 含 \u{0} 的串 len()=N 却 s[0] 判越界；
>   且字符串索引单字符结果用 px_str（strlen）构造，取到 NUL 时截断成空串。该 bug = S0 记录的
>   "pxc 重链后编 interp.px 确定性崩 rust_str_debug 行462" 真凶 —— 自举编译器 lex 含 "\u{0}"
>   字面量的源码（pxlexer.px 等）时 rust_str_debug 遍历 NUL 串崩 "字符串索引越界: 0"。
> - **修复**（runtime/runtime.c 3 处）：px_index 越界检查改 px_unicode_len_n(data, str.len)；
>   单字符结果改 px_str_len(buf, clen)；px_slice str len 改 px_unicode_len_n(data, str.len)。
>   修复后 bc_cli 处理 pxlexer.px/compiler.px（原确定性崩）exit 0。
> - **编译器全量 BCModule 发射打通（C1 核心前置）**：bc_emit 对 compiler.px（+codegen/
>   parser/pxlexer/cg_*/bc_emit 全 import 链 ~4700 行合并源码）完整发射成功 —— 173 funcs /
>   226 globals / 809 K 常量，dump 21456 行 exit 0 零未实现 panic → bc_emit 语言覆盖已达
>   编译器自举完备（修复前连 resolve 阶段都到不了）。
> - **基线**：golden/compiler.bc.dump（382KB，编译器 BCModule dump 权威基线，重复运行
>   diff 逐字节一致——发射确定性实证）。
> - **回归**：新增 examples/m89_c1/nul_str_regress.px（运行时构造 NUL 串：len/索引/切片/
>   反步长切片/遍历/拼接/负索引/中文不受影响）nul_str_verify.sh PASS；bc4/bc8/bc11 verify +
>   hello 三轨 + B1 parity 复跑全绿（runtime 改动零回归）。
> - 记录：编译轨 codegen 对字面量 NUL（\u{0}）在 C 源码层截断（C 串 "\0" 终止）——运行期
>   内存串字节安全不受影响（lexer 解码路径已由本修复覆盖）；to_upper/to_lower/trim 等原生
>   仍 strlen 截断 NUL（不在自举路径，留待后续 GAP-STR 收口）。
> - 下一步：C1 镜像对拍 —— bc_cli 自身 emit-c 成 VM 驱动，跑 compiler.px 重放 dump 与
>   golden/compiler.bc.dump 逐字节对拍（VM 上编译器编译自身）。

### M89-S3-C1 · 自举证明 PASS：VM 编译器编译自身，BCModule dump 与 golden 逐字节一致

> 完成（2026-09-08，dongyue 重放）：S3-D（96a8e4b 帧槽 GC 根止血）落地后，C1 自举对拍通过。
> - **证明链**：① 旧轨 bc_cli dump compiler.px == golden/compiler.bc.dump（本机基线复核一致）；
>   ② bc_cli --emit-c bc_cli.px → bc_cli_vm.c（990,651 B）→ gcc 链 rtcache（含 vm.o，S3-D）
>   → bc_cli_vm（VM 驱动版编译器）；③ bc_cli_vm compiler.px 重放 → dump 21,456 行 / 382,650 B
>   **与 golden/compiler.bc.dump 逐字节一致（diff 空，rc=0）** —— 编译器字节码镜像在 VM 上
>   编译自身，与旧 C 递归引擎产物完全一致，VM 语义等价第一证明。
> - **S3-D 止血实证**：VM 重放 compiler.px 全链 rss ≤ ~0.5 GB（对比 OOM 记录 7.1GB 泄漏态），
>   10GB ulimit 内完成无泄漏 —— VM 帧槽入 GC 根面修复有效。
> - **内存记录**：emit-c bc_cli.px 走旧 C 引擎（非 VM），虚拟内存高水位 >6GB（slab 不还 OS +
>   全链大对象，issue28 技术债）→ 本地验证需 ulimit ≥9.5GB；VM 路径不受影响。
> - **回归**：旧轨 bc1-12 dump golden 全绿；VM 冒烟 bc1/bc2 dump 一致；hello/旧轨一致。
> - 环境注：dongyue 缺 glibc static → 本地链接去 -static（仅开发验证，发布仍静态）。
> - 下一步：C2 golden 切换（compiler.c → BCModule 镜像 + 自举证明规则更新）+ S3-C 收口门
>   （vm_ab.sh v2 examples 全量 VM vs 旧轨 stdout 对拍）。

### M89-S3-C2 收口 · 三大切片合入 main + 主干全量回归绿（2026-09-09，dongyue）

> 分支 feat/m89-c2-vmtoolchain（C2-5 px build --vm / C2-5b pxc_vm / C2-5c
> pxi_vm）ff 合入 main（8ebaacd）并 push GitHub main。合入后主干全量回归
> 复跑全绿：C 轨自举（14986 行 == golden/compiler.c）+ BC 轨自举（compiler_vm
> 重放 dump 30445 行 == golden/compiler.bc.dump）+ vm_ab.sh v2 收口门
> （19 PASS + 0 GAP + 0 FAIL）+ diffcheck.sh --all 全量对拍 —— **S3-C 段
> 技术目标全部达成并收口**。
>
> 决策记录：默认轨切换（px build 默认产物切 BC + diffcheck/capability golden
> 大迁移）不在 S3-C 段做——VM 轨当前解释执行无 JIT/native，默认切换性能倒退
> 且动主干验证体系（fn_* .c golden 全量重生成），收益未到兑现点；技术可行性
> 已由三切片实证，工程化铺开挂起为后置决策项（详见 docs/M89_PLAN.md）。
