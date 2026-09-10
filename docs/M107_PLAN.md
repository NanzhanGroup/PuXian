# M107 计划：L0 运行时内存路径（字符串分配抖动 + GC 页回收风暴）

> 立项依据：M106-S1 采样归因（补丁后第一大户 = 分配器/页回收，占 ~87%）+ 本次 **S1 复测**（mmap/munmap 逐调用量化）。
> 基线：`main @ 9c1483e`（M106-S4）· 负载：`selfhost/build/compiler_vm bc stdlib/yaml.px`（M104/M105/M106 一贯口径）
> 状态：**S1 量化完成，S2 待开工**（本文即 S1 交付物）

---

## 1. S1 量化（真机实测，非估算）

### 1.1 基线计时（静默，3 轮，产物 md5 一致 `36a0f9d6…`）

| 轮 | real | user | sys | sys 占比 |
|---|---|---|---|---|
| 1 | 1.592s | 1.189s | 0.377s | 23.7% |
| 2 | 1.573s | 1.160s | 0.385s | 24.5% |
| 3 | 1.519s | 1.120s | 0.377s | 24.8% |

⇒ **近 1/4 墙钟花在内核态**（M106-S1 已证 syscall 时间 97% = mmap 53.94% + munmap 43.31%）。

### 1.2 系统调用逐调用计数（`strace -f -e trace=mmap,munmap,madvise,brk`，全程 83,689 行）

| 调用 | 次数 |
|---|---|
| `mmap` | **59,361** |
| `munmap` | **24,245** |
| `madvise` | **0**（全仓未使用） |
| `brk` | 82 |

**长度分布（关键）**：

| mmap 长度档 | 次数 | 合计字节 |
|---|---|---|
| **4096（单页）** | **59,306（99.9%）** | **231.7 MB** |
| 4K–64K | 24 | 0.6 MB |
| 64K–1M | 15 | 2.8 MB |
| >1M | 9 | 16.1 MB |

| munmap 长度档 | 次数 | 合计字节 |
|---|---|---|
| **4096（单页）** | **24,217（99.9%）** | **108.4 MB** |

⇒ 数量级上的全部开销是**单页 4KB 映射的建/拆**，与"大对象 mmap 兜底"（>1M 仅 9 次）无关。

### 1.3 GC 摘要（`PX_GC_DEBUG=1`，同时长）

```
collect #1..#6：标记 7263→33222 / 100000，回收 92737→66778，耗时 10~15ms
```

⇒ 6 轮 GC 合计 **≈83ms（≈5% 墙钟）**——**GC 标记本身不是瓶颈**，瓶颈在它**触发的页回收动作**。

### 1.4 与代码的对应（归因闭合）

| 观测 | 代码位置 | 说明 |
|---|---|---|
| 每 slab = **2 次 4KB mmap** | `slab_create`（runtime.c:419）`mmap(slab_bytes=4096)` + `slab_raw_alloc(slots_in_bytes)`（≤240B 亦按页取整 = 4096） | slab 体与 `in_use` 位图**各占一次独立映射** |
| 每回收 slab = **2 次 4KB munmap** | `slab_reclaim_empty`（runtime.c:625）：`slab_raw_free(s->in_use)` + `munmap(s, bytes)` | 每轮 GC 把**全部空 slab** 摘链归还 |
| **拆了又建** | `slab_reclaim_empty` 只在 GC sweep 后调用；下轮分配命中"无空槽"即 `slab_create` | 空 slab 不留缓冲 ⇒ mmap/munmap 交替冲刷 |
| **每字符串 2 次 xmalloc** | `px_str_len`（runtime.c:1701）：`xmalloc(sizeof(LXObject))` + `xmalloc(len+1)` | 短串亦如此；M106-S1 归因 `xmalloc` 28.84% 中 **95.25% 来自此处** |

数量自洽性：按"每 slab 2 次 mmap"折算 ≈ **29.6k 次 slab 创建**；按"每回收 slab 2 次 munmap"折算 ≈ **12.1k 次回收**（6 轮 GC 平均 ≈2000/轮）。

---

## 2. 归因结论（M107 打什么）

1. **页回收风暴**（第一优先）：`slab_reclaim_empty` 每轮 GC 全量 `munmap` 空 slab，随后分配立刻 `mmap` 重建 ⇒ 24k 拆 + 59k 建 = **83.5k 次单页系统调用/负载**。
2. **分配粒度不经济**：slab 体与位图分两次映射 ⇒ 系统调用次数**先天 ×2**。
3. **字符串分配抖动**：每个字符串固定 2 次 `xmalloc`；短串（占比极高）本可内联。

---

## 3. 切片与候选（按"收益/风险"排序）

| 切片 | 内容 | 预期 | 风险 |
|---|---|---|---|
| **S2** | **slab 映射合并 + 空 slab 缓存/滞回**：①`in_use` 位图移入 slab 尾部，同一次 mmap（系统调用次数减半）；②空 slab 不立即归还，保留每 class N 个（水位上限）或改 `madvise(MADV_DONTNEED)`（保映射、还物理页、免重建零页）；③GC 只在水位超限时回收 | mmap/munmap 次数降 1~2 个数量级；sys 0.38s 的部分回收 | 内存驻留上升（需水位可配 + 默认保守） |
| **S3** | **字符串 SSO / 合并分配**：①`LXObject` + 数据一次分配（2 次 xmalloc → 1）；②≤16B 短串内联进对象（不额外分配）；③竞技场化短命串 | 直击 `xmalloc` 28.84%（95.25% ← `px_str_len`） | **`as.str.data` 指针语义**：须确认无站点假设"data 指向独立分配"（xfree 路径 / FFI 传指针 / bytes 复用 union） |
| **S4** | 大对象 mmap 兜底路径评估（阈值/池化）与 `madvise` 策略统一 | 兜底路径 9 次/负载——**低优先** | 低 |

> 与 M104 §七 的 O5/O6/O8（分派层）**互斥**：M104 已证伪"抠分派可得 1.2~1.5x"，本里程碑只在**分配器/页回收**层动手。

---

## 4. 语义红线（实施前逐条核对，S2 起）

1. `xmalloc` 返回槽**必须零初始化**（`gc_mark` 等字段依赖）——合并映射/内联后形式可变，**语义不可变**。
2. 槽地址 → slab 反查（`slab_find_locked` / `g_xfree_hint` 边界计算 / `g_slab_ranges`）在**映射布局改变后必须同步**；`slab_verify` 断言同步。
3. 精确 GC（M92）根面：`LXObject` 尺寸/union 布局变更需复核**所有** `sizeof(LXObject)` 处与 VM 帧槽扫描。
4. 并发路径：`g_slab_mu` 临界区不新增重入（`slab_raw_alloc` 不得改走 `xmalloc`）；`slab_reclaim_empty` 的"持 `g_gc_mu` + STW"前提保持。
5. `PX_GC_INLINE` / `PX_GC_THRESHOLD` / `PX_GC_DEBUG` 既有开关语义不变；新增水位开关（如 `PX_SLAB_KEEP`）默认值 = 保守（可对拍还原 S2 前行为）。
6. 跨轨一致：C 轨 `fn_*` 与 VM 轨（`vm.c`）共用 runtime ⇒ 改动**两轨同时**生效，须双轨冒烟。

---

## 5. 验收门（沿用 M106 口径）

- 双自举逐字节（C 轨 `compiler.c` / BC 轨 `compiler.bc.dump`）
- `vm_ab.sh v2` 38P/0GAP/0F · `diffcheck --all` rc=0 · m89_s3d / m93_s3 / m96_s2 / m103_s2d 全绿
- pxi / pxi_vm 重链 + hello 四方一致 + 生态索引无漂移
- **性能判据**：真实负载 real 降幅 ≥10% 且 sys 时间降幅 ≥40%（S2）；mmap+munmap 次数降 ≥90%（S2 硬指标）
- 产物 dump 与 M106 基线 md5 相同（语义零漂移）
