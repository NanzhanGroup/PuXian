# BUG REPORT：ws-web 多证书并发现场 SEGV 崩溃（PuXian runtime 全局表无锁竞态）

- 报告人：清歌（ws-web 维护 / 文殊系统）
- 日期：2026-09-03
- 影响模块：PuXian runtime（runtime/runtime.c）—— 非 ws-web 应用层
- 严重度：P0（生产 Web 服务器多线程场景崩溃）

## 一、现象

PuXian 版 ws-web（v0.4.1，main.px 对齐 Go 版 v0.3.0 多站点/vhost/TLS-SNI/反代）部署到
文殊生产机（ws.wsai.chat，3~4 站点多证书 + 真实并发流量）后反复崩溃：

1. `status=11/SEGV` 段错误（systemd-coredump），主线程栈落在 `px_set_global → strcmp`；
2. `lx: 内存不足` 直接 exit(1)（xmalloc mmap 失败路径打印，runtime.c:414）；
3. 优雅关闭（SIGTERM）过程中崩溃：先打印 `[px-serve] 优雅关闭完成（在途 0）`，随后 core；
4. `access.log` 追加写入失败退出（并发 append_file）。

对照：本机 qg.wsai.chat（单站点单证书、低并发）同版本二进制稳定 20+ 小时无崩溃。
→ 差异变量：**多 TLS 证书 + 并发连接 + 反复 systemd 启停（SIGTERM 优雅关闭）**。

## 二、崩溃栈（systemd-coredump）

```
Stack trace of thread 主线程:
#0 __strcmp_evex
#1 px_set_global        runtime/runtime.c:2430
#2 bi_px_serve          runtime/runtime.c:11338
#3 fn_serve_https
#4 fn_main
#5 main
Stack trace of thread accept:
#0 __libc_accept
#1 bi_http_serve
#2 spawn_thread
Stack trace of thread worker:
#0 __libc_close / __libc_write（px_error 打日志）
#1 bi_http_serve / px_pool_worker 路径
```

## 三、根因分析（高置信）

runtime 全局符号表定义（runtime.c:506-509）：

```c
#define GLOBAL_CAP 4096
static char*   g_keys[GLOBAL_CAP];
static LXValue g_vals[GLOBAL_CAP];
static int     g_len = 0;
```

`px_set_global`（2430）与 `px_get_global`（2422）遍历/写入 g_keys/g_vals/g_len，
**全程无任何锁**（无 g_gc_mu、无 g_globals_mu）：

```c
void px_set_global(const char* name, LXValue v) {
    for (int i = 0; i < g_len; i++) {
        if (strcmp(g_keys[i], name) == 0) { g_vals[i] = v; return; }   // 无锁读+写
    }
    if (g_len >= GLOBAL_CAP) px_error("全局表溢出");
    g_keys[g_len] = xstrdup(name);   // xstrdup → xmalloc → slab/mmap
    g_vals[g_len] = v;
    g_len++;                          // 非原子
}
```

并发模型（M31.4b 连接线程池）：
- 主线程 `bi_px_serve` accept → `px_pool_push`；
- 预派生 `max_conn`(默认 32) 个 `px_pool_worker` 常驻线程，各线程取连接后执行
  `px_conn_worker` → **运行 VM handler（PuXian 函数）**；
- GC 会把 g_vals 当 root 扫描（runtime.c:1045-1046 / 1116-1117）。

竞态路径：
1. **多 worker 并发执行 VM handler**：任何 handler 内对全局变量赋值 / 内建函数内部
   `px_set_global`（如 px_serve 启动写 `__px_docroot` 等三连，11411-11413）都会并发
   无锁改 g_keys/g_vals/g_len → g_len 非原子 ++、同槽覆盖、xstrdup 泄漏、越界写
   （两个线程同时通过 `g_len >= GLOBAL_CAP` 检查）→ 堆/数组损坏 → strcmp 野指针 SEGV；
2. **GC 并发扫描 g_vals 与 px_set_global 写入无互斥**：GC 可能 mark 到半写对象；
3. **优雅关闭路径**：主线程 join worker 后打印完成并返回，但 accept 线程（spawn 的
   http_serve/bi_http_serve）与主线程/GC 交互期间，关闭时再触全局表访问即崩
   —— 与"优雅关闭完成打印后立即 core"吻合；
4. `xmalloc` slab 分配器虽自带 g_slab_mu，但只保护 slab 自身一致性，不保护
   g_keys/g_vals/g_len 的逻辑一致性；slab/mmap 耗尽时打 `lx: 内存不足` exit(1)
   —— 与日志 40391 吻合。

单机低并发不崩：worker 少、GC 触发少、全局表写竞争窗口极难命中；生产高并发 + 反复
启停后必现。

## 四、修复建议（供 runtime 维护者东月参考）

1. **给全局符号表加专用锁**（如 `static pthread_mutex_t g_globals_mu`）：
   `px_set_global` / `px_get_global` / GC root 扫描段（1045/1116）读写全部持锁；
   注意 px_set_global 内 xstrdup→xmalloc 可能重入 slab 锁（参考 287 行注释先例），
   锁序需设计（globals 锁内不做慢路径 / xstrdup 前先预分配）。
2. **g_len 用原子**（`_Atomic int`）或统一在锁内修改；GLOBAL_CAP 越界前防御。
3. **GC 暂停协议与 px_set_global 互斥**：确认 px_pool_worker 执行 VM 期间是否进入
   GC 暂停（safe point）；若 VM 执行不暂停，GC 扫描必须与全局表写互斥。
4. **优雅关闭**：关闭完成后、bi_px_serve 返回前，确保所有 spawn/accept/worker 线程
   已退出且不再触碰全局表，再做清理。
5. 修复后建议用 ws-web 多 vhost + TLS-SNI + 并发压测 + 反复 SIGTERM 回归。

## 五、复现物证

- ws-web/main.px v0.4.1（本目录，多站点 vhost/TLS/反代生产版源码）
- 文殊生产崩溃 journalctl 摘录见上文第二节（systemd-coredump 多份）
- 本机 qg.wsai.chat 单站同版稳定 = 对照组
