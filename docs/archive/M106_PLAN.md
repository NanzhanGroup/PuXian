# M106_PLAN —— 字符串下标原语 O(n)→摊还 O(1)（qg-issue 33-A 语言层主修）

> 立项：2026-09-11 · 处理人：东月 · 依据：`/data/qg-issue/33-puxian-str-index-on-n2/ISSUE.md`（清歌新建，L0 runtime 性能缺陷）
> 关系：**M105 的姊妹里程碑**（M105 打「名解析 + 系统调用」，M106 打「字符串下标」；两者共用 `compiler_vm bc stdlib/yaml.px` 真实负载口径）
> 交付：`runtime/runtime.c`（+70/−23）+ `runtime/runtime.h`（+13/−1）· tag **v0.2.0-m106**

---

## 一、立项口径

Issue 33 的现象是用户侧「词元缓存无法连接权限审批系统。」（网关审批客户端 fail-closed 统一文案），
但根因**不在连接层**（那是 Issue 31/32），而在 **`s[i]` / `len(s)` 是 O(n)/次**：

```c
// m105 runtime/runtime.c L2518（px_index 的 PX_STR 分支）
int ulen = px_unicode_len_n(obj.as.obj->as.str.data, obj.as.obj->as.str.len); // O(n) 全扫
...
while (count < i) { ... p += cl2; count++; }   // 再从 byte0 走到第 i 个 rune：O(i)
```

⇒ 任何「逐字符扫字符串」的 .px 代码天然 **O(n²)**；ws-approve 的纯 .px `json_valid`（`/check` 里被调两次）
在 64KB body 上把它放大成 **11.6s**，越过审批客户端的 2s/5s fail-closed 阈值（交点 ~22KB/35KB）。

**关键佐证（决定了做法）**：同一个 runtime 里已有正确写法 —— `px_slice` 的 str 分支
**先一次性预构建 rune→byte 偏移表（O(n)）再按表取第 i 个 rune（O(1)）**。
`px_index` 只是没用它，也没做对象级缓存。⇒ 本里程碑 = **把已验证的写法搬到 `px_index`/`px_len`/`px_slice` 上，并加惰性缓存**。

**分层**：L0 runtime（不动语言语义、不动发射器、不动 golden）；应用侧止血（Issue 33-C/D）由清歌先行，与本里程碑解耦。

---

## 二、S1 量化（先量化、再动手）

### 2.1 采样剖析（零侵入，插桩件不入库）

`/tmp/m106inst`（基线）/`/tmp/m106dy`（补丁）两套**同旗标**构建（`-O2 -g` + cuts 旗标 =
与 `.rtcache` 一致），链接 `sampler.o`（`ITIMER_PROF` 500µs 采 RIP，退出 dump PC 列表），
后处理 = `nm -n` 符号表按地址区间归因（本二进制）+ `/proc/<pid>/maps` 取 libc 基址后按偏移归因（动态库）。

负载：`compiler_vm bc stdlib/yaml.px`（与 M105 完全同口径）。

| 符号 | A 基线（HEAD runtime） | B 修复（M106-S2 runtime） |
|---|---|---|
| **样本总数（∝ CPU 时间）** | **3080** | **1663** |
| `px_index` | **1430（46.43%）** | **1（0.06%）** |
| `xmalloc` | 581（18.86%） | 542（32.59%） |
| `slab_reclaim_empty` | 424（13.77%） | 446（26.82%） |
| libc（`mmap`/`munmap`/`memcmp`…） | 437（14.19%） | 455（27.36%） |
| `vm_run_loop` | 68（2.21%） | 83（4.99%） |
| `px_gc_collect` + `gc_mark_obj` + GC 系 | 65（2.1%） | 76（4.6%） |

**决定性读数**：`3080 − 1430(px_index) = 1650` ≈ **1663（B 的总样本数）**——
即**整个差值就是 `px_index` 一项**，补丁移除的正是它，且**没有引入任何新的热点**（其余符号绝对计数持平：
xmalloc 581→542、slab_reclaim 424→446、libc 437→455，均在噪声内）。
样本计数同时给出加速比估计 **3080/1663 = 1.85x**（与 §4.1 的计时实测互相印证）。

### 2.2 libc 归因（动态库 27%，按 `/proc/<pid>/maps` 基址 + `nm -D` 偏移解析）

| 符号 | 占比 |
|---|---|
| `__munmap`（0x108dd0，syscall 落在 0x108ddb） | **10.86%** |
| `__mmap`（0x108d10，syscall 落在 0x108d36） | **8.57%** |
| `__memcmp_avx2_movbe`（0x15a95d… 族，非动态符号表的本地实现） | **~6.06%** |
| `malloc`/`free`/`pthread_*` 等 | 各 <1% |

与 `strace` 旁证完全一致：`mmap` **53.94%** / `munmap` **43.31%**（占系统调用时间）。

### 2.3 调用者归因（v2 采样器：RIP + 一帧调用者 `[RBP+8]`）

补丁版一次运行（1460 样本）：RIP 分布 = libc 30.75% · `xmalloc` 28.84% · `slab_reclaim_empty` 25.82% ·
`vm_run_loop` 6.16% · `px_gc_collect` 1.51% · `xfree` 1.37%。

| 热点 | 主要调用者 | 占该热点 |
|---|---|---|
| `xmalloc` | **`px_str_len`** | **95.25%**（401/421） |
| `slab_reclaim_empty` | **`px_gc_collect`** | **100%**（377/377） |
| `xfree` | `px_gc_collect` 60% / `px_obj_free` 25% / `px_add` 10% | — |

⇒ **下一站（M107 候选）方向明确**：①字符串分配抖动（`px_str_len` 一手贡献 95% 的 `xmalloc`）；
②GC 页回收风暴（`slab_reclaim_empty` 100% 来自 `px_gc_collect`，对上 libc `munmap`/`mmap` 19.4%、
对下系统调用时间 97%）。二者都比继续抠解释循环分派更有价值（M104 已证伪该路线 ≥1.25x）。

---

## 三、S2 实施（`runtime/runtime.c` +70/−23、`runtime/runtime.h` +13/−1）

### 3.1 改动

1. `LXObject` 的 `str` 子结构挂两个**惰性缓存**：`rune_len`（−1 = 未算）、`rune_offs`（+ `offs_cnt`）：
   ```c
   struct { char* data; int len; int rune_len; int offs_cnt; int* rune_offs; } str;
   ```
2. 新增 `px_utf8_clen()`（与 `px_index` 原判定式逐字相同）、
   `px_str_rune_len()`（惰性 rune 计数）、`px_str_offs_get()`（惰性 rune→byte 偏移表）。
3. 接入点：`px_index`（O(1) 取起始偏移）、`px_len`（O(1) 取 rune 数）、`px_slice`（str 分支 len）；
   `px_obj_free`（PX_STR/PX_BYTES 释放表）、`px_str_len`/`px_bytes_len`/`bi_mmap`（初值）、
   `bi_munmap`（复位缓存）。
4. `PX_STR_OFFS_MIN = 1024`：**只有字节长度 ≥1KB 的串才建表**（表 = 4B×步数），小串仍走原线性走查（不付分配代价）。

### 3.2 语义红线（逐条对齐改动前，全部有对拍证据）

| # | 红线 | 保证方式 |
|---|---|---|
| 1 | `len(s)` 值不变 | `rune_len` 存的就是 `px_unicode_len_n` 的原结果，**不是**建表步数（畸形 UTF-8 下二者不等） |
| 2 | `s[i]` 取到的字节位置不变 | 偏移表按 `px_index` 原线性走查**完全相同的步进规则**建（前导字节判长） |
| 3 | 越界/畸形 UTF-8 行为不变 | `i ≥ 表步数`（仅畸形 UTF-8 可达）**回落原线性走查**，连越界读行为都保持原样 |
| 4 | 单字符结果构造不变 | 仍按 `px_utf8_clen(c0)` 判长构造（内嵌 NUL / M89-S3-C1 语义不动） |
| 5 | 负索引不变 | `i += ulen` 仍用同一 `ulen` |
| 6 | str 不可变 ⇒ 缓存天然无失效问题 | px 字符串拼接产生新对象；唯一可变路径 `bi_munmap` 显式复位 |
| 7 | GC 不受影响 | 缓存字段为**纯数据**（`int`/`int*`），`gc_mark_obj` 的 PX_STR/PX_BYTES 走 default 不扫 union；`LXValue` 尺寸不变（`obj` 仍为指针）⇒ 帧槽扫描/精确 GC 路径逐字不变 |
| 8 | 并发安全 | 只读路径无锁；建表路径用 `xmalloc` + `__atomic_*`（先放数据后发布指针，竞态时他线程已建的胜出、本次分配丢弃） |
| 9 | `for ch in s` 一并受益（两轨均已核实） | C 轨 `cg_stmt.px` 生成 `for(i;i<px_len(x);i++) x[i]`；VM 轨 `bc_emit.px` 生成 `n=len(its)` + 每元素 `INDEX` ⇒ 两轨都只走 `px_len`/`px_index` |

### 3.3 数据面代价与**已知边界**（如实记录，供二期决策）

- `LXObject.str` 子结构 **+8 字节** ⇒ 每个 str/bytes 对象多 8 字节（`LXValue` 不变，GC 帧扫描不受影响）；
- ≥1KB 的串**被索引**时多一次 `xmalloc`（表 4B×步数，随对象回收）——
  实测真实负载 `xmalloc` 绝对样本 **581→542，未增加**（表只在真正 `s[i]` 时建，`len(s)` 不建）；
- ✅ **边界已收口（见 §六）**：表大小 = 4×串字节数，原**无上限**。已加 `PX_STR_OFFS_MAX = 16 MiB`
  护栏 —— 超过上界的串不建表、回落原线性走查（与改动前同行为），仅保留 `rune_len` 惰性计数。
  对 1MB 串 = 4MB 表（无碍）；数百 MB 巨串（正常路径应走 `bytes` 视图）不再放大 4×内存。

---

## 六、S3 内存上界护栏（2026-09-11）

### 6.1 动机

§3.3 记录的已知边界：偏移表 ≈ `sizeof(int)×(字节数+1)` ≈ **4× 串长**，且原实现**无上限**。
台账/日志类巨串（数百 MB）若被逐字符索引，会放大 4× 内存（20 MiB 串 → 80 MiB 表）。

### 6.2 改动（`runtime/runtime.c`、`runtime/runtime.h`）

- 新增 `PX_STR_OFFS_MAX = 16 MiB`（`#ifndef` 包裹，可 `-DPX_STR_OFFS_MAX=…` 覆盖做边界试验）；
  `PX_STR_OFFS_MIN` 同样包 `#ifndef`（便于等价性试验）；
- `px_str_offs_get()` 增一行上界早退：`if (n > PX_STR_OFFS_MAX) return NULL;`
  ⇒ 返回 NULL 即走 `px_index` 的**原线性走查**（该路径自 M105 起即存在，且是所有 <1KB 串的既有路径）；
- `runtime.h` 注释同步（建表条件改为 `PX_STR_OFFS_MIN ≤ 字节长度 ≤ PX_STR_OFFS_MAX`）。

**语义**：超上界串的行为 = **M105 及更早**（无表时代）逐字节同行为；`rune_len` 惰性计数仍在（O(1) 空间）。

### 6.3 对照实验（护栏的正确性主证据）

同一输入、仅上界不同两个构建：

| 构建 | `PX_STR_OFFS_MAX` | 20 MiB 串路径 |
|---|---|---|
| **A** | 16 MiB（护栏生效） | 不建表 → 线性走查 |
| **B** | 1 GiB（护栏关闭） | 建 80 MiB 表 → 表查 |

输入：20 MiB ASCII · 20 MiB CJK · 1 MiB · 5 KB；每输入打印 `len(s)` + 13 个下标（含负索引）的字节序列。

- **输出 A ≡ B 逐字节相同**（4/4 输入）⇒ 回落路径与建表路径在真实巨串上**等价**；
- **峰值 RSS（`getrusage` MaxRSS）**：

| 输入 | A（护栏 on） | B（护栏 off） | Δ |
|---|---|---|---|
| 20 MiB ASCII | **43.1 MB** | **123.0 MB** | **+79.9 MB ≈ 4×20 MiB** |
| 20 MiB CJK | **43.1 MB** | 72.4 MB | +28.9 MB ≈ 4×7.23M runes（未触页不计入 RSS） |
| 1 MiB ASCII | 9.1 MB | 8.9 MB | ~0（两者都建 4 MB 表） |
| 5 KB CJK | 3.8 MB | 3.9 MB | ~0 |

⇒ 护栏**确实拦住**了巨串的 4× 放大，且**不改变任何输出**。

### 6.4 回归

全量闸门（bootstrap C/BC 双轨 · `vm_ab.sh v2` · `diffcheck --all` · m89_s3d / m93_s3 / m96_s2 / m103_s2d）
在含护栏的 runtime 上重跑，清单与 §5.2 同款。

---

## 四、实测

### 4.1 真实负载 A/B（`compiler_vm bc stdlib/yaml.px`，`taskset -c 3`，5 轮取 min CPU = user+sys）

两二进制**同旗标、同非 runtime 源文件**（已逐项 md5 核验：`vm.c`/`runtime_ffi.c`/`runtime_route.c`/`coro.c`
与 HEAD 完全一致，`runtime.c/h` 为唯一差异），机器静默（load ≈0.86）：

| 会话 | A 基线（HEAD runtime） | B 修复（M106-S2 runtime） | 比值 |
|---|---|---|---|
| 第 1 轮 | **2.800s**（2.850/2.800/2.850/2.970/2.930） | **1.520s**（1.520/1.690/1.660/1.570/1.740） | **1.84x** |
| 第 1 轮复测 | **2.860s**（2.860/2.910/2.930/3.040/3.020） | **1.510s**（1.590/1.600/1.510/1.630/1.600） | **1.89x** |
| 第 2 轮 | **2.870s** | **1.610s** | 1.78x |
| 第 2 轮复测 | **2.730s** | **1.510s** | 1.81x |

⇒ **加速比 1.78 ~ 1.89x（≈1.85x）**，与 §2.1 采样计数比 **3080/1663 = 1.85x** 一致（两条独立证据互证）。

> 如实记录：门跑批（bootstrap 编译）期间测得的一组数（2.47/2.59s → 1.20s = 2.06~2.16x）**不可复现**，
> 属并发争用下的失真值，**不采用**；以静默态 4 组数据为准。

### 4.2 语言层微基准（对口 ISSUE §2.1 / §7 判据 1）

`bench.px`：`mode=index`（只考 `px_index`）/ `mode=ilen`（考 `px_index` + `px_len`，即 `for ch in s` 的 codegen 形态）。
CPU 秒（user+sys，单轮，静默态）：

| 规模 | index·before | index·after | ilen·before | ilen·after |
|---|---|---|---|---|
| 4 KB | 0.000 | 0.000 | 0.000 | 0.000 |
| 8 KB | 0.080 | **0.000** | 0.130 | **0.000** |
| 16 KB | 0.290 | **0.000** | 0.510 | **0.010** |
| 32 KB | 1.180 | **0.010** | 2.080 | **0.010** |
| 64 KB | **4.740** | **0.030** | **8.250** | **0.040** |

- before：规模 ×2 → 耗时 **×4**（0.08→0.29→1.18→4.74）= **标准 O(n²)**；
- after：**平坦**（≥1KB 建表后摊还 O(1)），64KB **0.03s** ⇒ 相对 before **158x（index）/ 206x（ilen）**；
- 与 ISSUE §2.1（64KB→5419ms）同量级（本机同源构建 4740ms）⇒ 病灶复现一致，修复后判据 1 达成。

### 4.3 等价性对拍（本里程碑的正确性主证据）

`eq.px` 对每个输入逐字节打印 `len(s)` + 每个 `s[i]` 的字节序列（走 `bytes_get` 数字输出，NUL/非法 UTF-8 都不被打印层截断），
**before 二进制 vs after 二进制输出逐字节 diff = 相同**（`diff` 无输出）：

- 输入 16 个：`ascii` / `cjk` / `emoji` / `long_ascii(4KB)` / `long_cjk(6.8KB)` / `long_crlf(4.1KB)` /
  **6 类畸形 UTF-8**（`mal1`–`mal6`）/ `mal_long(2KB 畸形)` / `mix` / `nul_head` / `nul_mid`
  —— 覆盖 ASCII/CJK/emoji/CRLF/NUL 头中/**建表路径（≥1KB）**/**回落路径（畸形长串）**；
- 另加负索引（`s[-1]` / `s[-n]`）与越界用例：`oob_pos`（i=len）/ `oob_neg`（i 仍为负）/ `oob_long`（8192 字节串上越界）
  ⇒ 报错文案逐字节一致（`字符串索引越界: 8192`）。

---

## 五、S5 收口（2026-09-11 · 本机 8 核 16G）

### 5.1 交付清单

| 项 | 结果 |
|---|---|
| **C 轨自举证明** | `selfhost/bootstrap_prove.sh --fresh` → B.c == `golden/compiler.c`（**15060 行**）**逐字节一致** ✅ |
| **BC 轨自举证明** | `selfhost/bootstrap_prove_bc.sh --fresh` → `compiler_vm`（新链，runtime = M106）重放 dump == `golden/compiler.bc.dump`（**30581 行**）**逐字节一致** ✅ |
| **pxi 重链**（C 轨） | `tools/pxc build --c --full selfhost/interp.px` → **9,626,960 B**（与 M105 版**同尺寸**——`px_index` 位移 +0x2b0，被节对齐吸收；**内容已变**：md5 `240a04608f07…` → `cb3b4d8f95c7…`；**二进制内实证**：新 px_index 反汇编含 `callq <xmalloc>`（偏移表构建）+ `callq <xfree>`（竞态丢弃），M105 版两者皆无）✅ |
| **pxi_vm 重链**（VM 轨） | `tools/pxc build --full selfhost/interp.px` → **9,460,784 B**（同尺寸同理；md5 `4b2140fd22…` → `910c116970…`；px_index 同证含 `callq <xmalloc>` + `callq <xfree>`）✅ |
| 轨别核实 | pxi：`fn_*`=**229** / `s_bc_*`=**0** ⇒ **C 轨**；pxi_vm：`fn_*`=0 / `s_bc_*`=**230** ⇒ **VM 轨**；两者均 statically linked x86-64 ELF、`--version` = `pxi 0.2.0` ✅ |
| 生态索引 | `./bootstrap/pxi tools/gen_ecosystem.px`（13 libs）+ `bash tools/gen_native_table.sh`（306 natives）→ `git diff --exit-code` **无漂移** ✅ |
| 冒烟 | C 轨/VM 轨 `hello` 产物 + `pxi`/`pxi_vm` 解释 = **四方输出逐字节一致**（`HELLO, 普贤`）✅ |
| tag | **`v0.2.0-m106`** |

### 5.2 回归（全部跑在 M106 runtime 上；套件自报 `rtcache: /data/code/puxian/.rtcache/c95f3c2348de18a1/` = 补丁版 runtime 缓存）

| 门 | 结果 |
|---|---|
| `examples/m89_a2/vm_ab.sh v2`（38 例 VM vs 旧轨对拍） | **38 PASS / 0 GAP / 0 FAIL** ✅ |
| `selfhost/diffcheck.sh --all`（lex/parse/codegen/run + value/interp 全量对拍） | **rc=0**「全量对拍全部通过」✅（s01–s15 与 golden 一致；v01_value 259 PASS、v02_env 39、v03_module 26、v04_module_state 9） |
| `examples/m89_s3d/verify.sh`（并发 GC ×3 / 生成器 / VM 堆回落 / 混合压测） | **PASS=9 FAIL=0** ✅ |
| `examples/m93_s3/verify.sh`（帧协程 chan/mutex/rwlock/sleep + 并发 GC + C 轨逃生舱） | **PASS=6 FAIL=0** ✅ |
| `examples/m96_s2/verify.sh`（daemon / 线程池上限收敛+空闲回收 / 逃生舱 / VM 重建） | **PASS=8 FAIL=0** ✅ |
| `examples/m103_s2d/verify.sh`（img 链路 + Go 对拍） | **rc=0**（512x384，px 13209B vs go_ref 13177B 同量级）✅ |
| 冒烟：C 轨/VM 轨 `hello` 产物 + `pxi`/`pxi_vm` 解释 | **四方输出逐字节一致** ✅ |
| 生态索引防漂移（CI 同款：`gen_ecosystem.px` + `gen_native_table.sh` → `git diff --exit-code`） | **无漂移** ✅ |

### 5.3 里程碑结论

1. **目标达成**：Issue 33-A（语言层主修）落地为**逐字符扫描 O(n²) → O(n)**，
   真实负载 **1.78~1.89x**、语言层微基准 **64KB 158~206x**，且 `px_index` 从 46.43% 采样占比降到 0.06%。
2. **「先量化」再次决定性**：S1 采样 + 调用者归因**确认病灶即 `px_index`**（占 46%），
   修复后**差值恰好 = 该项**（3080−1430≈1663），**零新热点**；同一次量化还顺带产出了 M107 的方向（§2.3）。
3. **代价与边界**：`LXObject` +8B/串、≥1KB 串被索引时 4B×字节的表（随对象回收）；
   **表无上限**是已知边界，建议二期补 `PX_STR_OFFS_MAX`（§3.3）。
4. **回归零缺口**：双自举逐字节 + 38 例 VM 对拍 + diffcheck 全量 + 4 个套件全绿 + 生态索引无漂移。
