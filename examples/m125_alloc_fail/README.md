# M125 · 分配失败改「请求级失败」（qg-issue 81）· 验收材料

分支：`fix/m125-alloc-fail-request-scoped`（也在 `main`）
改动：`runtime/runtime.c`（+105 / −7）

## 1 病灶

三处 mmap 失败一律：

```c
if (p == MAP_FAILED) { fprintf(stderr, "lx: 内存不足\n"); exit(1); }
```

（`slab_raw_alloc` / `slab_create` / `xmalloc` 大对象）

后果 = **一个请求即可终止整个服务进程**。生产 mahesvara 同进程承载 8 个站点，
实测整站不可达约 3 s（journal：`lx: 内存不足` → `Main process exited, code=exited, status=1`
→ systemd 重启）；2026-09-16 07:17 / 09:06 两轮外部复现，`NRestarts` 累计 10 次。

更坏的一面是**归因误导**：真因往往不是内存不足，而是**尺寸算成 0 / 负数** ——
`(n + 8 + pg-1) & ~(pg-1)` 在 n 接近 SIZE_MAX 时回绕为 0，`mmap(NULL,0)` 返回 EINVAL，
却被报成「内存不足」。

## 2 修法

| 位置 | 原 | 现 |
|---|---|---|
| `slab_raw_alloc` | `exit(1)` | 返回 NULL（调用方回滚后走统一出口） |
| `slab_create` | `exit(1)` | 返回 NULL；反查数组扩容失败时**回滚已建 slab**（保持不变量） |
| `xmalloc` 大对象 | `exit(1)` | `px_alloc_fail(n, total, "xmalloc 大对象")` |
| `xcalloc` | 乘法回绕静默 | 回绕 → 按尺寸非法报错 |
| 尺寸计算 | 回绕静默 | `px_map_bytes()` 显式检出 |

统一出口 `px_alloc_fail(n, total, what)`：

```
运行时错误: 内存不足【xmalloc 大对象】申请 107374182401 字节（映射 107374186496 字节）errno=12 (Cannot allocate memory)
          处理：协程隔离点内 → 只终止本请求（HTTP 层转 5xx），进程继续服务
```

- 协程隔离点内（`g_err_jmp_set`，serve handler / spawn）→ `longjmp` **只终止本协程**
  ⇒ 沿用 M116 语义，HTTP 层转 **5xx**，进程继续服务；
- 隔离点外（启动期 / GC / 信号上下文）→ `_exit(1)`（保留致命语义，绝不静默续跑）。

**锁约束**：longjmp 不展开 pthread 锁 ⇒ 失败出口**一律先解锁再抛出**
（`xmalloc` slab 路径显式 `pthread_mutex_unlock(&g_slab_mu)`）。

## 3 门（`verify.sh`，11/11）

```sh
bash examples/m125_alloc_fail/verify.sh
```

| 用例 | 注入 | 判据 | 结果 |
|---|---|---|---|
| A | `PX_ALLOC_FAIL_MIN=4194304` | `/big`（8 MB read_file）→ 5xx；同进程 `/health`、`/small` 仍 200 | ✅ |
| B | 无（`read_file` 一个 100 GB 稀疏文件 ⇒ 一次 100 GB mmap 必被内核拒） | `/huge` → 5xx；正常 `/big` 仍 200 且长度正确 | ✅ |
| N | 阈值抬到 1 TB（等于不注入） | `/big`、`/health` 全 200（断言非恒真） | ✅ |
| 文案 | — | 旧文案 `lx: 内存不足` 不再出现；新文案含出处 + 字节数 + errno | ✅ |

## 4 回归（同批 runtime 改动全部重跑，零回归）

| 门 | 结果 |
|---|---|
| `examples/m117_realworld_defects/verify.sh` | 解释轨 9/9 + 编译轨 9/9 + 2 负控 ⇒ pass=4 fail=0 |
| `examples/m123_http_conn_alive/verify.sh` | 语义 4/4；定量 13.36s → 0.04s = **0.30%**（验收线 ≤30%） |
| `examples/m93_s2/verify.sh` | PASS=6 FAIL=0（含 500 协程 × 并发 GC、C 轨逃生舱 129 线程） |

## 5 覆盖边界（如实标注）

- 门 A 命中的是 **`xmalloc` 大对象路径**；**slab 小对象路径**的失败出口（解锁后抛出 +
  反查数组扩容回滚）由**代码审查**覆盖，未做注入验证 —— 测试钩子对所有 `xmalloc` 生效，
  但阈值必须高于**启动期最大分配 1 MB**（否则进程起不来），因此无法单独命中 slab 路径。
- 本改动**不承诺**真·OOM 一定可恢复：隔离点外仍致命；`PX_SPAWN_ISOLATE=0` 时隔离关闭，
  仍是旧致命语义（逃生舱保留）。
- 触发生产的**具体尺寸来源**尚未捕获：新文案已带 `申请/映射 字节数 + errno + 出处`，
  下次复现即可定位（这也是本次把「分类报真因」放进同一提交的原因）。

## 6 文件

| 文件 | 用途 |
|---|---|
| `alloc_fail.px` | 门程序（`FX=a|b|n` 三种模式；端口 18099） |
| `verify.sh` | 一键验证（11 项判据） |
| `README.md` | 本文件 |
