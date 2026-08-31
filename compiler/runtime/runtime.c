// 普贤 (PuXian) C 运行时库 — runtime.c
// M4 编译模式：动态值系统 + 内置函数
// M8：值对象自动释放（保守标记-清除 GC）
// M11：并发 GC（spawn 活跃时 stop-the-world 全量回收 + 多线程栈/寄存器保守扫描）
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <poll.h>
#include <stdatomic.h>
#include <execinfo.h>
#include "miniz.h"   // M21 gzip 压缩/解压（raw deflate + gzip 容器，M19 zip 同源）

// M10 HTTPS：mbedtls 静态库（compiler/runtime/mbedtls/）
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cache.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/sha256.h"

// M27 P0：服务端 TLS + WebServer 生产化
// - tls_server(cert, key)：注册进程级服务端 TLS（px_serve/sse_serve/ws_serve 自动 TLS）
// - g_cur_conn：当前线程处理中的连接（px_px_send 等旧 fd 接口自动转发 TLS 写）
// - g_px_stop / g_px_listen_fd：优雅关闭（SIGINT/SIGTERM → 停止 accept）
// - g_px_max_body：请求体大小上限（px_serve opts.max_body_size）
__thread PxConn* g_cur_conn = NULL;
static volatile sig_atomic_t g_px_stop = 0;
static volatile sig_atomic_t g_px_listen_fd = -1;
static int g_px_max_body = 10 * 1024 * 1024;
volatile int g_px_inflight = 0;  // px_serve 在途请求数（优雅关闭等待归零；runtime_ws.c 共享）
static long long g_px_body_seq = 0;       // body 落盘临时文件序号
static mbedtls_x509_crt g_srv_cert;
static mbedtls_pk_context g_srv_key;
static int g_srv_tls_ready = 0;
static pthread_mutex_t g_srv_tls_mu = PTHREAD_MUTEX_INITIALIZER;
// M30：服务端 https 连接池——TLS 会话缓存（Session ID + 票据），新连接快速恢复握手
static mbedtls_ssl_cache_context g_srv_tls_cache;
static int g_srv_tls_cache_init = 0;
static LXValue bi_tls_server(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_open(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_id(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_set(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_del(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_destroy(LXValue* args, int nargs, void* ctx);
static LXValue bi_basic_auth(LXValue* args, int nargs, void* ctx);

// 前向声明：xmalloc/xfree 在 gc_block_stop 定义之前使用（M11 自由链表分配器）
static void gc_block_stop(sigset_t* old);
static void gc_unblock_stop(const sigset_t* old);

// std.net（M5.2/M10）前向声明（定义在文件尾部，注册函数在前部使用）
static LXValue bi_tcp_listen(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_accept(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_connect(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_send(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_recv(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_close(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_post(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_serve(LXValue* args, int nargs, void* ctx);
// M23c P1：HTTP 生产化（http_request 连接池 / http_get_stream 流式下载）
static LXValue bi_http_request(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_get_stream(LXValue* args, int nargs, void* ctx);
// M17 .px 脚本执行机制（应用平台）
static LXValue bi_px_exec(LXValue* args, int nargs, void* ctx);
static LXValue bi_px_serve(LXValue* args, int nargs, void* ctx);
// M18 后台定时任务 / 定时器原语
static LXValue bi_set_timeout(LXValue* args, int nargs, void* ctx);
static LXValue bi_set_interval(LXValue* args, int nargs, void* ctx);
static LXValue bi_clear_timer(LXValue* args, int nargs, void* ctx);
// M28 P1：时间时区 + cron（定义在文件尾部 M28 区块）
LXValue bi_time_format(LXValue* args, int nargs, void* ctx);
LXValue bi_time_parse(LXValue* args, int nargs, void* ctx);
LXValue bi_tz_offset(LXValue* args, int nargs, void* ctx);
LXValue bi_cron(LXValue* args, int nargs, void* ctx);
// M21 P1：SSE 服务端（LLM 流式推送 / 实时通知）
static LXValue bi_sse_serve(LXValue* args, int nargs, void* ctx);
static LXValue bi_sse_send(LXValue* args, int nargs, void* ctx);
static LXValue bi_sse_close(LXValue* args, int nargs, void* ctx);
// M23 P1：SSE 客户端（流式消费 / 事件订阅）
static LXValue bi_sse_connect(LXValue* args, int nargs, void* ctx);
static LXValue bi_sse_read(LXValue* args, int nargs, void* ctx);
// M22 P1：强制垃圾回收（gc()）
static LXValue bi_gc(LXValue* args, int nargs, void* ctx);
// M23 P1：进程/信号（os_pid/os_spawn/os_wait/os_kill/signal）
static LXValue bi_os_pid(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_spawn(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_wait(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_kill(LXValue* args, int nargs, void* ctx);
static LXValue bi_signal(LXValue* args, int nargs, void* ctx);
// M23b P1：二进制安全字节串（bytes 类型）
static LXValue bi_bytes(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_len(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_set(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_slice(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_concat(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_to_str(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_base64(LXValue* args, int nargs, void* ctx);
static LXValue bi_base64_to_bytes(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_find(LXValue* args, int nargs, void* ctx);
static LXValue bi_read_bytes(LXValue* args, int nargs, void* ctx);
static LXValue bi_write_bytes(LXValue* args, int nargs, void* ctx);
// M30 P1：字节序可控整数↔bytes（pxdb 存储基石）
static LXValue bi_int_to_bytes(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_to_int(LXValue* args, int nargs, void* ctx);

// M23b 字节辅助（字符串/字节串统一 data+len；供 base64/hex 等前置函数使用）
static const char* bdata(LXValue v);
static int blen(LXValue v);

// M10 HTTPS 内部辅助
static char* px_http_request(const char* url, const char* method, const char* body, int* out_len);
static int px_https_request(const char* host, int port, const char* req, char** out, int* out_len);
// M24 https 连接池：TLS 会话复用。CA 证书缓存定义在此（M10 区 px_ensure_cacert 使用，前向声明）。
static pthread_mutex_t g_cacert_mu = PTHREAD_MUTEX_INITIALIZER;
static mbedtls_x509_crt g_cacert;
static int g_cacert_loaded = 0;
static void px_ensure_cacert(void);
// M21 gzip / chunked 辅助（px_http_request 客户端解码用，定义在后方）
static char* px_gzip_decompress(const char* in, int inlen, int* outlen);
static char* px_chunked_decode(const char* in, int inlen, int* outlen);

// ==================== 内存分配（M22：size-class slab 子分配器 + 大对象 mmap 兜底） ====================
// M11 并发 GC：sweep 会释放对象，而其他线程可能正在 malloc/free 中被 GC 信号挂起
// （持有 glibc 堆锁）→ GC 主线程 free 会死锁。因此对象与子分配全部改用
// mmap + 自管 slab（无 glibc 堆锁）：信号挂起在 mmap/munmap 中不持有堆锁。
// M22 优化（解决 M11.3 已知限制①"mmap 每对象一页 ≈4KB/对象 内存放大"）：
//   小对象（≤16KB）按 size-class 从 slab 槽位分配（16/24/32/48/64/96/128/192/256/
//   384/512/768/1024/1536/2048/3072/4096/6144/8192/12288/16384），一页多槽共享；
//   超过最大 class 回落 mmap（每分配一映射，带大小头）。
//   M11.3 曾试"无锁 slab"失败（空闲栈 next 被覆盖 = 无锁竞态）→ M22 改用互斥锁保护，
//   持锁期间屏蔽 SIG_GC_STOP（gc_block_stop，与 list/dict 结构修改函数同一模式）：
//   持锁线程不会被 GC 暂停 → GC 主线程（sweep 时 xfree）不会等待被暂停线程持有的锁。
//   对象表 g_objs 与 GC 逻辑不变，slab 仅是底层内存提供者；槽位复用只发生在 sweep
//   （stop-the-world，无并发分配）之后，杜绝 use-after-free。

#define PX_PAGE 4096
#define SLAB_MAX_CLASS 16384          // 超过此大小 → mmap 兜底
#define SLAB_CLASS_COUNT 21
static const size_t slab_classes[SLAB_CLASS_COUNT] = {
    16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768,
    1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384
};

typedef struct Slab {
    struct Slab* next;      // 同 class 链表
    void* base;             // mmap 映射起点（含头部）
    size_t class_size;      // 槽大小（= class 值）
    size_t slot_count;      // 槽总数
    size_t free_count;      // 空闲槽数（0 → 不可分配，需新 slab）
    void* free_head;        // 空闲链表头（槽内首 word 存 next，NULL 结束）
    unsigned char* in_use;  // 调试：槽占用位图（1=已分配），检测双重分配/释放
} Slab;

static pthread_mutex_t g_slab_mu = PTHREAD_MUTEX_INITIALIZER;
static Slab* g_slab_heads[SLAB_CLASS_COUNT] = {0};
// 地址 → slab 反查（xfree/xrealloc 定位）：按 base 升序，二分查找
static Slab** g_slab_ranges = NULL;
static size_t g_slab_range_count = 0;
static size_t g_slab_range_cap = 0;

// 反查数组的裸分配：slab_create 在持 g_slab_mu 期间调用，不能走 xmalloc（会重入锁）
static void* slab_raw_alloc(size_t n) {
    size_t pg = PX_PAGE;
    size_t total = (n + sizeof(size_t) + pg - 1) & ~(size_t)(pg - 1);
    void* p = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { fprintf(stderr, "lx: 内存不足\n"); exit(1); }
    *(size_t*)p = total;
    return (char*)p + sizeof(size_t);
}
static void slab_raw_free(void* p) {
    if (!p) return;
    size_t total = *(size_t*)((char*)p - sizeof(size_t));
    munmap((char*)p - sizeof(size_t), total);
}

static int slab_class_index(size_t n) {
    for (int i = 0; i < SLAB_CLASS_COUNT; i++) {
        if (n <= slab_classes[i]) return i;
    }
    return -1;  // 大对象
}

static int slab_cmp(const void* a, const void* b) {
    const Slab* sa = *(const Slab* const*)a;
    const Slab* sb = *(const Slab* const*)b;
    if (sa->base < sb->base) return -1;
    if (sa->base > sb->base) return 1;
    return 0;
}

// 创建新 slab（调用方须持 g_slab_mu）：映射可容纳 ≥4 槽的页数，初始化空闲链表
static Slab* slab_create(size_t class_size, int class_idx) {
    size_t slot_count = (4 * class_size + PX_PAGE - 1) / PX_PAGE;  // 至少 4 槽的页数
    size_t pages = slot_count < 1 ? 1 : slot_count;
    size_t slab_bytes = pages * PX_PAGE;
    // 头部对齐：Slab 结构体放映射起点，槽区紧随其后（8 字节对齐）
    size_t header = (sizeof(Slab) + 7) & ~(size_t)7;
    size_t slots_in_bytes = (slab_bytes - header) / class_size;
    if (slots_in_bytes < 1) slots_in_bytes = 1;
    void* p = mmap(NULL, slab_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { fprintf(stderr, "lx: 内存不足\n"); exit(1); }
    Slab* s = (Slab*)p;
    s->next = g_slab_heads[class_idx];
    s->base = p;
    s->class_size = class_size;
    s->slot_count = slots_in_bytes;
    s->free_count = slots_in_bytes;
    s->free_head = NULL;
    s->in_use = (unsigned char*)slab_raw_alloc(slots_in_bytes);
    memset(s->in_use, 0, slots_in_bytes);
    char* slots = (char*)p + header;
    // 空闲链表：从后往前串（槽内首 word 存 next）
    void* head = NULL;
    for (size_t i = slots_in_bytes; i > 0; i--) {
        void* slot = slots + (i - 1) * class_size;
        *(void**)slot = head;
        head = slot;
    }
    s->free_head = head;
    g_slab_heads[class_idx] = s;
    // 插入反查数组（保持按 base 升序）
    if (g_slab_range_count >= g_slab_range_cap) {
        size_t ncap = g_slab_range_cap ? g_slab_range_cap * 2 : 64;
        Slab** nr = (Slab**)slab_raw_alloc(ncap * sizeof(Slab*));
        if (g_slab_ranges) { memcpy(nr, g_slab_ranges, g_slab_range_count * sizeof(Slab*)); slab_raw_free(g_slab_ranges); }
        g_slab_ranges = nr;
        g_slab_range_cap = ncap;
    }
    size_t pos = 0;
    while (pos < g_slab_range_count && g_slab_ranges[pos]->base < p) pos++;
    memmove(&g_slab_ranges[pos + 1], &g_slab_ranges[pos], (g_slab_range_count - pos) * sizeof(Slab*));
    g_slab_ranges[pos] = s;
    g_slab_range_count++;
    return s;
}

// 指针 → slab（二分；返回 NULL 表示 mmap 大对象）
// 注意：g_slab_ranges 在 slab_create（持 g_slab_mu）中 memmove 维护；
// 外部调用（xrealloc→xalloc_cap）必须经 slab_find（持锁）读取，防撕裂读竞态。
static Slab* slab_find_locked(const void* p) {
    if (g_slab_range_count == 0) return NULL;
    // 防御：检测 ranges 数组损坏（并发竞态/越界写），避免返回垃圾 Slab* 导致 xfree 误判
    if (g_slab_range_count > g_slab_range_cap) {
        fprintf(stderr, "SLAB CORRUPT: count=%zu cap=%zu\n", g_slab_range_count, g_slab_range_cap);
        abort();
    }
    size_t lo = 0, hi = g_slab_range_count;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        Slab* s = g_slab_ranges[mid];
        if ((uintptr_t)s < 0x10000 || ((uintptr_t)s & 7) != 0 ||
            s->base != (void*)s || s->class_size < 16 || s->class_size > SLAB_MAX_CLASS ||
            s->slot_count == 0 || s->slot_count > 1024) {
            fprintf(stderr, "SLAB CORRUPT: ranges[%zu]=%p count=%zu cap=%zu p=%p base=%p cs=%zu slots=%zu\n",
                    mid, (void*)s, g_slab_range_count, g_slab_range_cap, p,
                    s->base, s->class_size, s->slot_count);
            abort();
        }
        if (p < s->base) hi = mid;
        else {
            size_t header = (sizeof(Slab) + 7) & ~(size_t)7;
            void* end = (char*)s->base + ((header + s->slot_count * s->class_size + PX_PAGE - 1) & ~(size_t)(PX_PAGE - 1));
            if ((const char*)p < (const char*)end) return s;
            lo = mid + 1;
        }
    }
    return NULL;
}

// 线程安全版（供 xalloc_cap 等未持 g_slab_mu 的调用方）
static Slab* slab_find(const void* p) {
    sigset_t old;
    gc_block_stop(&old);
    pthread_mutex_lock(&g_slab_mu);
    Slab* s = slab_find_locked(p);
    pthread_mutex_unlock(&g_slab_mu);
    gc_unblock_stop(&old);
    return s;
}

static void* xmalloc(size_t n) {
    if (n <= 0) n = 1;
    int ci = slab_class_index(n);
    if (ci < 0) {  // 大对象：mmap 每分配一映射（带大小头，行为同 M11）
        size_t pg = PX_PAGE;
        size_t total = (n + sizeof(size_t) + pg - 1) & ~(size_t)(pg - 1);
        void* p = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED) { fprintf(stderr, "lx: 内存不足\n"); exit(1); }
        *(size_t*)p = total;
        return (char*)p + sizeof(size_t);
    }
    size_t cs = slab_classes[ci];
    sigset_t old;
    gc_block_stop(&old);
    pthread_mutex_lock(&g_slab_mu);
    Slab* s = g_slab_heads[ci];
    if (!s || s->free_count == 0) s = slab_create(cs, ci);
    void* slot = s->free_head;
    size_t header = (sizeof(Slab) + 7) & ~(size_t)7;
    size_t idx = ((const char*)slot - ((const char*)s->base + header)) / cs;
    if (s->in_use[idx]) { fprintf(stderr, "SLAB BUG: double-alloc slot %zu class %zu\n", idx, cs); abort(); }
    s->in_use[idx] = 1;
    s->free_head = *(void**)slot;
    s->free_count--;
    pthread_mutex_unlock(&g_slab_mu);
    gc_unblock_stop(&old);
    memset(slot, 0, cs);   // 清零：gc_mark 等字段依赖零初始化
    return slot;
}

static void xfree(void* p) {
    if (!p) return;
    sigset_t old;
    gc_block_stop(&old);
    pthread_mutex_lock(&g_slab_mu);
    Slab* s = slab_find_locked(p);
    if (s) {
        size_t header = (sizeof(Slab) + 7) & ~(size_t)7;
        size_t off = (const char*)p - ((const char*)s->base + header);
        size_t idx = off / s->class_size;
        int aligned = (off % s->class_size == 0);
        if (!aligned || idx >= s->slot_count) {
            fprintf(stderr, "SLAB BUG: bad-free p=%p slab=%p class=%zu off=%zu idx=%zu aligned=%d\n",
                    p, s->base, s->class_size, off, idx, aligned);
            abort();
        }
        if (!s->in_use[idx]) {
            // 槽已空闲（double-free）：幂等忽略，不重复入链（防空闲链表环 → 双重分配）
            pthread_mutex_unlock(&g_slab_mu);
            gc_unblock_stop(&old);
            return;
        }
        s->in_use[idx] = 0;
        *(void**)p = s->free_head;
        s->free_head = p;
        s->free_count++;
        pthread_mutex_unlock(&g_slab_mu);
        gc_unblock_stop(&old);
        return;
    }
    pthread_mutex_unlock(&g_slab_mu);
    gc_unblock_stop(&old);
    size_t total = *(size_t*)((char*)p - sizeof(size_t));
    munmap((char*)p - sizeof(size_t), total);
}

// 分配容量（xrealloc 用：slab → class 大小；mmap → 记录大小）
static size_t xalloc_cap(const void* p) {
    Slab* s = slab_find(p);
    if (s) return s->class_size;
    return *(size_t*)((char*)p - sizeof(size_t)) - sizeof(size_t);
}

static void* xrealloc(void* p, size_t n) {
    if (!p) return xmalloc(n);
    if (n <= 0) n = 1;
    size_t cap = xalloc_cap(p);
    if (n <= cap) return p;   // 容量足够，不缩小
    void* np = xmalloc(n);
    memcpy(np, p, cap < n ? cap : n);
    xfree(p);
    return np;
}

static void* xcalloc(size_t n, size_t sz) {
    void* p = xmalloc(n * sz);
    memset(p, 0, n * sz);
    return p;
}

static char* xstrdup(const char* s) {
    size_t n = strlen(s);
    char* p = xmalloc(n + 1);
    memcpy(p, s, n + 1);
    return p;
}

// ==================== 全局表（定义前移：GC 标记根集合使用） ====================

#define GLOBAL_CAP 256
static char* g_keys[GLOBAL_CAP];
static LXValue g_vals[GLOBAL_CAP];
static int g_len = 0;

// ==================== GC（M8：保守标记-清除，值对象自动释放） ====================
// 所有 LXObject 注册到全局对象表 g_objs。分配累计超阈值 → gc_collect()：
//   1) mark：根 = 全局表 + 暂存根（刚创建对象）+ 当前线程栈保守扫描
//   2) sweep：未标记对象释放子分配 + 本体，从表移除
// 并发保护（M11）：spawn 线程活跃时 stop-the-world——GC 主线程向所有活跃线程
// 发送 SIG_GC_STOP 实时信号，线程在信号处理器中保存上下文（寄存器）+ 暂存根后
// 自旋等待 g_gc_resume；GC 扫描全局表 + 所有线程栈/寄存器后清扫，再恢复线程。
// 保守扫描只认对象本体地址（8 字节对齐），误标仅推迟回收（安全），漏标由暂存根
// + 全局表 + 栈扫描综合兜底。

#define GC_THRESHOLD_DEFAULT 100000   // 对象数触发阈值（可被 PX_GC_THRESHOLD 覆盖）
#define GC_HASH_MIN_CAP 4096          // 对象地址哈希集合初始容量

static pthread_mutex_t g_gc_mu = PTHREAD_MUTEX_INITIALIZER;
static LXObject** g_objs = NULL;
static int g_obj_count = 0;
static int g_obj_cap = 0;
static long long g_alloc_bytes = 0;
static long long g_gc_trigger_bytes = 0;  // 0 = 未启用字节阈值
static int g_gc_threshold = GC_THRESHOLD_DEFAULT;
static int g_gc_env_inited = 0;
static int g_gc_debug = 0;
static int g_active_threads = 0;   // spawn 活跃线程数（>0 时进入并发 GC 路径）

// M11 并发 GC：线程注册表 + 暂停协议
#define MAX_SPAWN_THREADS 64
#define SIG_GC_STOP (SIGRTMIN + 2)   // 实时信号：暂停线程（可排队，不与用户信号冲突）
typedef struct {
    pthread_t tid;
    int in_use;          // 槽位占用
    int paused;          // 该线程已暂停（在信号处理器中等待恢复）
    int is_main;         // 主线程槽位（退出时不注销）
    int epoch;           // 暂停所属 GC 轮次（用于区分"本轮真暂停"与"堆积信号短暂暂停"）
    ucontext_t uc;       // 暂停时保存的上下文（寄存器）
    LXObject* tmp_root;  // 暂停时该线程的暂存根（__thread 跨线程不可读，由处理器保存）
} GCThreadInfo;
static GCThreadInfo g_threads[MAX_SPAWN_THREADS];
static int g_paused_count = 0;      // 已暂停线程数（调试用；控制流以 paused 标志 + epoch 为准）
static volatile int g_gc_resume = 0;// （保留字段，控制流以 epoch 为准）
static volatile int g_gc_epoch = 0; // GC 轮次号：每轮开始/结束各 ++，handler 等待其变化
static volatile int g_gc_stop_in_progress = 0; // 本轮 GC 是否在进行中（handler 用其区分过期堆积信号）
static volatile pthread_t g_gc_executor = 0;   // 当前 GC 主线程（执行 px_gc_collect 的线程）；handler 用它自检防自打断
static int g_gc_runs = 0;
static int g_gc_freed = 0;
static int g_gc_skips = 0;
static long long g_gc_marked = 0;   // 调试：最近一轮 GC 标记数
static __thread LXObject* g_tmp_root = NULL;  // 暂存根：保护刚创建对象（构造函数内触发 GC）

// 开放寻址哈希集合（对象地址快速查询，供保守栈扫描）
typedef struct {
    uintptr_t* slots;
    size_t cap;
    size_t count;
} GCHash;

static void gc_hash_init(GCHash* h, size_t cap) {
    size_t c = GC_HASH_MIN_CAP;
    while (c < cap) c <<= 1;
    h->slots = xcalloc(c, sizeof(uintptr_t));
    h->cap = c;
    h->count = 0;
}

static void gc_hash_insert(GCHash* h, uintptr_t addr) {
    if ((h->count + 1) * 10 >= h->cap * 7) {  // load factor 0.7 扩容
        size_t ncap = h->cap << 1;
        uintptr_t* ns = xcalloc(ncap, sizeof(uintptr_t));
        for (size_t i = 0; i < h->cap; i++) {
            uintptr_t a = h->slots[i];
            if (!a) continue;
            size_t j = (size_t)(a ^ (a >> 16)) & (ncap - 1);
            while (ns[j]) j = (j + 1) & (ncap - 1);
            ns[j] = a;
        }
        xfree(h->slots);
        h->slots = ns;
        h->cap = ncap;
    }
    size_t j = (size_t)(addr ^ (addr >> 16)) & (h->cap - 1);
    for (size_t probes = 0; probes <= h->cap; probes++) {
        if (!h->slots[j]) { h->slots[j] = addr; h->count++; return; }
        j = (j + 1) & (h->cap - 1);
    }
    // 防御：表满（理论不可达），丢弃该插入
}

static bool gc_hash_has(GCHash* h, uintptr_t addr) {
    size_t j = (size_t)(addr ^ (addr >> 16)) & (h->cap - 1);
    // 防御：探测上限 cap+1 次，防止表损坏/满载时线性探测死循环导致 GC 卡死
    for (size_t probes = 0; probes <= h->cap; probes++) {
        uintptr_t s = h->slots[j];
        if (!s) return false;
        if (s == addr) return true;
        j = (j + 1) & (h->cap - 1);
    }
    return false;
}

static void gc_hash_free(GCHash* h) { xfree(h->slots); h->slots = NULL; }

static void gc_install_handler(void);
static void gc_ensure_main_registered(void);
static void gc_init_env(void) {
    const char* d = getenv("PX_GC_DEBUG");
    if (d && d[0] == '1') g_gc_debug = 1;
    const char* t = getenv("PX_GC_THRESHOLD");
    if (t && atoi(t) > 0) g_gc_threshold = atoi(t);
    gc_install_handler();
    gc_ensure_main_registered();
    g_gc_env_inited = 1;
}

static void gc_debug(const char* fmt, ...) {
    if (!g_gc_debug) return;
    // 注意：必须用 write(2) 直写 fd，不能经 stdio（fprintf）——
    // GC 主线程持 g_gc_mu 时若 fprintf，而某线程在 printf 中被 GC 信号挂起
    // （持有 stdio 内部锁），会死锁。write 是原子 syscall，无 stdio 锁。
    char buf[512];
    int n = snprintf(buf, sizeof(buf), "[px-gc] ");
    va_list ap;
    va_start(ap, fmt);
    n += vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
    va_end(ap);
    if (n < 0) n = 0;
    if (n > (int)sizeof(buf) - 2) n = (int)sizeof(buf) - 2;
    buf[n++] = '\n';
    (void)write(2, buf, (size_t)n);
}

static bool px_value_is_obj(LXValue v) {
    switch (v.type) {
        case PX_STR:
        case PX_BYTES:
        case PX_LIST:
        case PX_DICT:
        case PX_FUNC:
        case PX_NATIVE:
        case PX_STRUCT:
        case PX_ENUM:
        case PX_TUPLE:
        case PX_CHAN:
            return true;
        default:
            return false;
    }
}

// 释放对象内部子分配 + 对象本体（sweep 阶段调用）
static void px_obj_free(LXObject* o) {
    switch (o->type) {
        case PX_STR: xfree(o->as.str.data); break;
        case PX_BYTES: xfree(o->as.str.data); break;
        case PX_LIST: xfree(o->as.list.items); break;
        case PX_DICT:
            for (int i = 0; i < o->as.dict.len; i++) xfree(o->as.dict.keys[i]);
            xfree(o->as.dict.keys);
            xfree(o->as.dict.vals);
            break;
        case PX_FUNC: xfree(o->as.func.name); break;
        case PX_NATIVE: xfree(o->as.native.name); break;
        case PX_STRUCT:
            xfree(o->as.struct_inst.type_name);
            for (int i = 0; i < o->as.struct_inst.nfields; i++) xfree(o->as.struct_inst.fnames[i]);
            xfree(o->as.struct_inst.fnames);
            xfree(o->as.struct_inst.fvals);
            break;
        case PX_ENUM:
            xfree(o->as.enum_inst.type_name);
            xfree(o->as.enum_inst.variant);
            break;
        case PX_TUPLE: xfree(o->as.tuple.items); break;
        case PX_CHAN:
            xfree(o->as.chan.buf);
            pthread_mutex_destroy(&o->as.chan.mu);
            pthread_cond_destroy(&o->as.chan.cv_send);
            pthread_cond_destroy(&o->as.chan.cv_recv);
            break;
        case PX_MUTEX:
            pthread_mutex_destroy(&o->as.mutex.mu);
            pthread_cond_destroy(&o->as.mutex.cv);
            break;
        case PX_RWLOCK:
            pthread_mutex_destroy(&o->as.rwlock.mu);
            pthread_cond_destroy(&o->as.rwlock.cv);
            break;
        default: break;
    }
    xfree(o);
}

// 标记单个对象及其可达子对象（显式栈 DFS，避免深链递归栈溢出）
// M11：每个出栈对象先用 gc_hash_has 校验是否在对象表（set）——并发数据竞争
// 可能使对象图出现损坏指针（如 list.items 指向已释放/半构造区域），
// 校验后跳过垃圾指针，避免 DFS 无限遍历损坏对象图导致 GC 卡死。
static void gc_mark_obj(GCHash* set, LXObject* o) {
    if (!o) return;
    LXObject** stack = xmalloc(sizeof(LXObject*) * 1024);
    int cap = 1024, sp = 0;
#define PUSH_OBJ(x) do { if (sp >= cap) { cap *= 2; stack = xrealloc(stack, sizeof(LXObject*) * cap); } stack[sp++] = (x); } while (0)
    PUSH_OBJ(o);
    while (sp > 0) {
        LXObject* cur = stack[--sp];
        if (!gc_hash_has(set, (uintptr_t)cur)) continue;   // 垃圾指针 → 跳过
        if (cur->gc_mark) continue;
        cur->gc_mark = 1;
        g_gc_marked++;
        switch (cur->type) {
            case PX_LIST: {
                LXValue* items = cur->as.list.items;
                for (int i = 0; i < cur->as.list.len; i++) {
                    if (px_value_is_obj(items[i]) && items[i].as.obj) PUSH_OBJ(items[i].as.obj);
                }
                break;
            }
            case PX_DICT: {
                LXValue* vals = cur->as.dict.vals;
                for (int i = 0; i < cur->as.dict.len; i++) {
                    if (px_value_is_obj(vals[i]) && vals[i].as.obj) PUSH_OBJ(vals[i].as.obj);
                }
                break;
            }
            case PX_STRUCT: {
                LXValue* fvals = cur->as.struct_inst.fvals;
                for (int i = 0; i < cur->as.struct_inst.nfields; i++) {
                    if (px_value_is_obj(fvals[i]) && fvals[i].as.obj) PUSH_OBJ(fvals[i].as.obj);
                }
                break;
            }
            case PX_TUPLE: {
                LXValue* items = cur->as.tuple.items;
                for (int i = 0; i < cur->as.tuple.len; i++) {
                    if (px_value_is_obj(items[i]) && items[i].as.obj) PUSH_OBJ(items[i].as.obj);
                }
                break;
            }
            case PX_CHAN: {
                // M11：并发下无锁保守扫描（chan.buf 元素为单 word 原子读写，误标仅推迟回收）
                LXValue* buf = cur->as.chan.buf;
                int phys = cur->as.chan.cap > 0 ? cur->as.chan.cap : 1;
                for (int i = 0; i < phys; i++) {
                    if (px_value_is_obj(buf[i]) && buf[i].as.obj) PUSH_OBJ(buf[i].as.obj);
                }
                break;
            }
            default: break;  // STR / FUNC / NATIVE / ENUM 无子对象
        }
    }
    xfree(stack);
#undef PUSH_OBJ
}

// 当前线程栈保守扫描：把栈上"看起来像对象地址"的 word 标记为根。
// 只扫描活跃帧 [当前 RSP, 栈底)——整栈（默认 8MB）逐 word 哈希查找太慢
// （每次 GC 多线程 × 百万 word），且未使用栈区无有效指针。
static void gc_scan_stack(GCHash* set) {
    pthread_attr_t attr;
    void* stackaddr = NULL;
    size_t stacksize = 0;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        pthread_attr_getstack(&attr, &stackaddr, &stacksize);
        pthread_attr_destroy(&attr);
    }
    if (!stackaddr || stacksize == 0) return;
    ucontext_t uc;
    getcontext(&uc);
    uintptr_t rsp = (uintptr_t)uc.uc_mcontext.gregs[REG_RSP];
    uintptr_t start = rsp & ~(uintptr_t)7;
    uintptr_t end = (uintptr_t)stackaddr + stacksize;
    if (g_gc_debug) { char dbg[160]; int dn = snprintf(dbg, sizeof(dbg), "[scan-self] rsp=%lx start=%lx end=%lx range=%lu\n", rsp, start, end, (unsigned long)(end - start)); (void)write(2, dbg, (size_t)dn); }
    for (uintptr_t p = start; p + sizeof(uintptr_t) <= end; p += sizeof(uintptr_t)) {
        uintptr_t w = *(uintptr_t*)p;
        if ((w & 7) == 0 && gc_hash_has(set, w)) {
            gc_mark_obj(set, (LXObject*)w);
        }
    }
}

// 保存/恢复通用寄存器到栈：-O2 下局部指针可能仅在寄存器中，
// 保守扫描必须把寄存器也纳入根集合，否则深链/长循环内分配易误回收。
// 实现：getcontext() 把全部通用寄存器（含 caller-saved rax/rcx/rdx/rsi/rdi/r8-r11
// 与 callee-saved rbx/rbp/r12-r15）写入 ucontext_t；该结构位于本函数栈帧，
// 被 gc_scan_stack 保守扫描覆盖。
// （ucontext.h 已在文件头部 include 区统一引入）

// ==================== M11 并发 GC：线程暂停协议 ====================
// 设计：GC 需要 stop-the-world 时，向所有已注册活跃线程发送 SIG_GC_STOP 实时信号。
// 线程在信号处理器中（async-signal-safe，仅内存读写 + sched_yield）：
//   保存 ucontext（寄存器）+ 本线程暂存根 → 计数 paused → 自旋等待 g_gc_resume → 恢复。
// GC 主线程（触发 GC 的线程）不暂停自己，扫描：全局表 + 自己栈/寄存器 + 所有暂停
// 线程的寄存器/栈/暂存根，然后设置 g_gc_resume=1 唤醒全部线程。

// 信号处理器：暂停当前线程直到 GC 完成
static void gc_stop_handler(int sig, siginfo_t* si, void* ctx) {
    (void)sig; (void)si;
    pthread_t me = pthread_self();
    // M11 修复⑤：若我是当前 GC 执行者（正在跑 px_gc_collect），忽略暂停信号——
    // 否则延迟信号在本轮 GC 执行中投递，handler 自旋等 epoch，而 epoch 只有
    // 本线程自己能推进 → 死锁（依赖 5 秒兜底才恢复，每轮 GC 卡 5 秒）。
    if (g_gc_executor && pthread_equal(g_gc_executor, me)) return;
    GCThreadInfo* ti = NULL;
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (g_threads[i].in_use && pthread_equal(g_threads[i].tid, me)) { ti = &g_threads[i]; break; }
    }
    if (!ti) return;  // 理论不会：未注册线程收到暂停信号
    // 关键：区分"有效暂停请求"与"过期堆积信号"。
    // 若当前没有 GC 在进行（g_gc_stop_in_progress==0），说明这是上一轮排队、
    // 延迟到现在才处理的信号——直接忽略返回，避免 my_epoch 捕获当前 epoch 后
    // 自旋等待一个永远不会到来的"本轮结束"（死锁）。
    if (!g_gc_stop_in_progress) {
        if (g_gc_debug) { char dbg[96]; int dn = snprintf(dbg, sizeof(dbg), "[stop-expired] tid=%lx\n", (unsigned long)me); (void)write(2, dbg, (size_t)dn); }
        return;
    }
    // M22 修复：堆积实时信号重入保护——若本线程已在本轮暂停（首次信号已保存用户态上下文
    // 并自旋），后续堆积信号（gc_block_stop 阻塞期间 GC 重发累积）直接忽略返回，
    // 禁止重入覆盖 ti->uc（否则寄存器扫描拿到的是信号处理器自旋状态 → 丢用户态寄存器
    // → 活跃对象漏标被误回收 → use-after-free，即并发 GC 偶发崩溃根因）。
    if (ti->paused && ti->epoch == g_gc_epoch) return;
    if (g_gc_debug) {
        char dbg[128];
        int dn = snprintf(dbg, sizeof(dbg), "[stop] tid=%lx\n", (unsigned long)me);
        (void)write(2, dbg, (size_t)dn);
    }
    ti->uc = *(const ucontext_t*)ctx;
    ti->tmp_root = g_tmp_root;
    ti->epoch = g_gc_epoch;   // 记录暂停所属轮次
    ti->paused = 1;
    __sync_fetch_and_add(&g_paused_count, 1);
    __sync_synchronize();
    // 自旋等待本轮 GC 结束：条件 = 本轮仍在进行（stop_in_progress）且 epoch 未变。
    // 若本轮已结束（stop 已清除，信号为延迟/堆积）→ 立即退出，绝不空等——
    // 否则会自旋等待一个永远不会到来的"本轮结束"（偶发死锁）。
    // 兜底：自旋超上限（约 5 秒）强制恢复，宁可漏回收也绝不卡死。
    int my_epoch = g_gc_epoch;
    int hspins = 0;
    while (g_gc_stop_in_progress && g_gc_epoch == my_epoch) {
        sched_yield();
        if (++hspins > 5000000) break;
    }
    __sync_synchronize();
    ti->paused = 0;
    __sync_fetch_and_add(&g_paused_count, -1);
}

static void gc_install_handler(void) {
    static int installed = 0;
    if (installed) return;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = gc_stop_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;  // SA_RESTART：被信号打断的系统调用自动重启
    sigemptyset(&sa.sa_mask);
    sigaction(SIG_GC_STOP, &sa, NULL);
    installed = 1;
}

// 结构修改关键区信号屏蔽：防止线程在 realloc/写元素中途被 GC 信号挂起，
// 导致 GC 标记读到半更新状态（旧 items 指针已 free / 容量未同步等）。
// 仅屏蔽 SIG_GC_STOP，不影响其他信号；chan.buf 单 word 原子写无需屏蔽。
static void gc_block_stop(sigset_t* old) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIG_GC_STOP);
    pthread_sigmask(SIG_BLOCK, &set, old);
}
static void gc_unblock_stop(const sigset_t* old) {
    pthread_sigmask(SIG_SETMASK, old, NULL);
}

// 线程槽位注册/注销（调用方须持 g_gc_mu；信号处理器只读不写注册表）
static GCThreadInfo* gc_find_thread(pthread_t tid) {
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (g_threads[i].in_use && pthread_equal(g_threads[i].tid, tid)) return &g_threads[i];
    }
    return NULL;
}

static int gc_register_thread(pthread_t tid, int is_main) {
    if (gc_find_thread(tid)) return 0;  // 已注册
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (!g_threads[i].in_use) {
            g_threads[i].tid = tid;
            g_threads[i].in_use = 1;
            g_threads[i].paused = 0;
            g_threads[i].is_main = is_main;
            g_threads[i].epoch = 0;
            memset(&g_threads[i].uc, 0, sizeof(g_threads[i].uc));
            g_threads[i].tmp_root = NULL;
            return 0;
        }
    }
    return -1;  // 槽位满
}

static void gc_unregister_thread(pthread_t tid) {
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (g_threads[i].in_use && !g_threads[i].is_main && pthread_equal(g_threads[i].tid, tid)) {
            g_threads[i].in_use = 0;
            g_threads[i].paused = 0;
            g_threads[i].epoch = 0;
            return;
        }
    }
}

// 确保主线程已注册（首次 GC 初始化时调用；此时通常仍为主线程）
static void gc_ensure_main_registered(void) {
    if (gc_find_thread(pthread_self())) return;
    gc_register_thread(pthread_self(), 1);
}

// 扫描单个暂停线程的通用寄存器（x86_64）
static void gc_scan_registers(GCHash* set, ucontext_t* uc) {
    greg_t* regs = uc->uc_mcontext.gregs;
    static const int reg_ids[] = {
        REG_RAX, REG_RBX, REG_RCX, REG_RDX, REG_RSI, REG_RDI,
        REG_RBP, REG_R8, REG_R9, REG_R10, REG_R11, REG_R12,
        REG_R13, REG_R14, REG_R15
    };
    for (unsigned i = 0; i < sizeof(reg_ids)/sizeof(reg_ids[0]); i++) {
        uintptr_t w = (uintptr_t)regs[reg_ids[i]];
        if ((w & 7) == 0 && gc_hash_has(set, w)) gc_mark_obj(set, (LXObject*)w);
    }
}

// 扫描单个暂停线程的栈（pthread_getattr_np 获取边界；只扫活跃帧 [RSP, 栈底)）
static void gc_scan_thread_stack(GCHash* set, pthread_t tid, ucontext_t* uc) {
    pthread_attr_t attr;
    void* stackaddr = NULL;
    size_t stacksize = 0;
    if (pthread_getattr_np(tid, &attr) == 0) {
        pthread_attr_getstack(&attr, &stackaddr, &stacksize);
        pthread_attr_destroy(&attr);
    }
    if (!stackaddr || stacksize == 0) return;
    uintptr_t rsp = (uintptr_t)uc->uc_mcontext.gregs[REG_RSP];
    uintptr_t start = rsp & ~(uintptr_t)7;
    uintptr_t end = (uintptr_t)stackaddr + stacksize;
    for (uintptr_t p = start; p + sizeof(uintptr_t) <= end; p += sizeof(uintptr_t)) {
        uintptr_t w = *(uintptr_t*)p;
        if ((w & 7) == 0 && gc_hash_has(set, w)) gc_mark_obj(set, (LXObject*)w);
    }
}

// 主回收入口：mark + sweep（M11：spawn 活跃时 stop-the-world）
void px_gc_collect(void) {
    // M11 修复④：GC 执行期间屏蔽自己的 SIG_GC_STOP——防止上一轮"延迟信号"
    // 在本轮 GC 执行中投递（handler 会自旋等 epoch，而 epoch 只有本线程能推进
    // → 卡死/5 秒空转）。先拿锁再屏蔽：等锁期间不屏蔽（可被其他 GC 正常暂停），
    // 持锁后屏蔽（防自打断）。出口统一 gc_unblock_stop。
    sigset_t gc_old;
    pthread_mutex_lock(&g_gc_mu);
    gc_block_stop(&gc_old);
    g_gc_executor = pthread_self();   // 标记我是 GC 执行者（handler 自检防自打断）
    if (!g_gc_env_inited) gc_init_env();
    if (g_obj_count == 0) {
        pthread_mutex_unlock(&g_gc_mu);
        g_gc_executor = 0;
        gc_unblock_stop(&gc_old);
        return;
    }

    if (g_active_threads > 0) {
        // ===== M11 并发路径：stop-the-world =====
        pthread_t me = pthread_self();
        if (g_gc_debug) {
            char dbg[512]; int dn = 0;
            dn += snprintf(dbg+dn, sizeof(dbg)-dn, "[gc] me=%lx active=%d\n", (unsigned long)me, g_active_threads);
            for (int i = 0; i < MAX_SPAWN_THREADS; i++)
                if (g_threads[i].in_use)
                    dn += snprintf(dbg+dn, sizeof(dbg)-dn, "  [%d] tid=%lx main=%d paused=%d\n", i, (unsigned long)g_threads[i].tid, g_threads[i].is_main, g_threads[i].paused);
            (void)write(2, dbg, (size_t)dn);
        }
        // 本轮 GC 开始（handler 捕获 my_epoch 后自旋等待 epoch 变化）
        g_gc_epoch++;
        g_gc_stop_in_progress = 1;   // 标记进行中（handler 据此区分过期堆积信号）
        __sync_synchronize();
        // 1) 向所有已注册、非自身、已创建完成的线程发送暂停信号（只发一次）
        for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
            GCThreadInfo* ti = &g_threads[i];
            if (!ti->in_use || pthread_equal(ti->tid, me)) continue;
            if ((uintptr_t)ti->tid == 0) continue;  // 创建中：还没运行普贤代码，无需暂停
            pthread_kill(ti->tid, SIG_GC_STOP);     // ESRCH（线程恰好退出）忽略
        }
        // 2) 等待所有"仍存活且已注册"的线程**本轮**暂停。
        //    判定"本轮真暂停"：ti->paused==1 且 ti->epoch==当前 epoch。
        //    堆积信号（stop 已清除）会让 paused 短暂置 1 后立即返回，但 epoch 是旧值
        //    → 不算本轮暂停 → 重发信号直到真正本轮暂停。已退出线程（ESRCH）忽略。
        int spins = 0;
        for (;;) {
            int remain = 0;
            __sync_synchronize();
            for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
                GCThreadInfo* ti = &g_threads[i];
                if (!ti->in_use || pthread_equal(ti->tid, me) || (uintptr_t)ti->tid == 0) continue;
                if (ti->paused && ti->epoch == g_gc_epoch) continue;   // 本轮已真暂停
                int rc = pthread_kill(ti->tid, SIG_GC_STOP);
                if (g_gc_debug) { char dbg[96]; int dn = snprintf(dbg, sizeof(dbg), "[kill] tid=%lx rc=%d\n", (unsigned long)ti->tid, rc); (void)write(2, dbg, (size_t)dn); }
                if (rc == 0 || (rc != 0 && errno == EAGAIN)) remain++;
                /* ESRCH：线程已退出，忽略 */
            }
            if (remain == 0) break;
            if (++spins > 5000000) {   // 兜底：约 5 秒未全部暂停 → 降级跳过本轮（绝不卡死）
                g_gc_skips++;
                g_gc_epoch++;
                g_gc_stop_in_progress = 0;
                __sync_synchronize();
                while (g_paused_count > 0) sched_yield();
                pthread_mutex_unlock(&g_gc_mu);
                g_gc_executor = 0;
                gc_unblock_stop(&gc_old);
                return;
            }
            // M22：重发间隔加小延时，避免对长时间屏蔽信号的线程狂轰信号（实时信号排队 →
            // 解除屏蔽时堆积触发 → 信号处理器重入覆盖 ucontext，丢用户态寄存器）。
            struct timespec ts = {0, 200000};   // 200us
            nanosleep(&ts, NULL);
            sched_yield();
        }
        // 3) 标记
        GCHash set;
        gc_hash_init(&set, (size_t)g_obj_count * 2);
        for (int i = 0; i < g_obj_count; i++) gc_hash_insert(&set, (uintptr_t)g_objs[i]);
        g_gc_marked = 0;
        if (g_gc_debug) (void)write(2, "[mk] hash\n", 10);
        // 根1：全局表
        for (int i = 0; i < g_len; i++) {
            if (px_value_is_obj(g_vals[i]) && g_vals[i].as.obj) gc_mark_obj(&set, g_vals[i].as.obj);
        }
        if (g_gc_debug) (void)write(2, "[mk] globals\n", 13);
        // 根2：本线程（GC 执行者）暂存根
        if (g_tmp_root) gc_mark_obj(&set, g_tmp_root);
        // 根3：本线程栈 + 寄存器（getcontext 写入栈上 ucontext，一并扫描）
        ucontext_t uc;
        getcontext(&uc);
        gc_scan_stack(&set);
        if (g_gc_debug) (void)write(2, "[mk] self-stack\n", 15);
        // 根4：所有本轮暂停线程：寄存器 + 栈 + 暂存根
        for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
            GCThreadInfo* ti = &g_threads[i];
            if (!ti->in_use || !ti->paused || ti->epoch != g_gc_epoch || pthread_equal(ti->tid, me)) continue;
            gc_scan_registers(&set, &ti->uc);
            gc_scan_thread_stack(&set, ti->tid, &ti->uc);
            if (ti->tmp_root) gc_mark_obj(&set, ti->tmp_root);
            if (g_gc_debug) { char dbg[64]; int dn = snprintf(dbg, sizeof(dbg), "[mk] scanned tid=%lx\n", (unsigned long)ti->tid); (void)write(2, dbg, (size_t)dn); }
        }
        // 4) sweep
        if (g_gc_debug) { char dbg[96]; int dn = snprintf(dbg, sizeof(dbg), "[mk] sweep count=%d\n", g_obj_count); (void)write(2, dbg, (size_t)dn); }
        int freed = 0, w = 0;
        for (int i = 0; i < g_obj_count; i++) {
            LXObject* o = g_objs[i];
            if (!gc_hash_has(&set, (uintptr_t)o)) continue;   // 防御：损坏条目，丢弃
            if (o->gc_mark) {
                o->gc_mark = 0;
                g_objs[w++] = o;
            } else {
                px_obj_free(o);
                freed++;
            }
        }
        g_obj_count = w;
        g_alloc_bytes = 0;
        g_gc_freed += freed;
        g_gc_runs++;
        g_tmp_root = NULL;
        if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
        gc_debug("collect #%d(并发): 标记 %lld/%d 回收 %d 存活 %d 线程 %d", g_gc_runs, g_gc_marked, g_obj_count + freed, freed, g_obj_count, g_paused_count);
        if (g_gc_debug) (void)write(2, "[mk] after-collect\n", 19);
        gc_hash_free(&set);
        // 5) 本轮结束：epoch++ 唤醒所有暂停线程；清除进行中标志；等待其全部恢复
        g_gc_epoch++;
        g_gc_stop_in_progress = 0;
        __sync_synchronize();
        if (g_gc_debug) (void)write(2, "[mk] resume-wait\n", 17);
        int wspins = 0, wstable = 0;
        for (;;) {
            int any_paused = 0;
            __sync_synchronize();
            for (int i = 0; i < MAX_SPAWN_THREADS; i++)
                if (g_threads[i].in_use && g_threads[i].paused) { any_paused = 1; break; }
            if (!any_paused) { if (++wstable >= 2) break; }
            else wstable = 0;
            if (++wspins > 5000000) break;   // 兜底：约 5 秒未全部恢复 → 强制继续（绝不卡死）
            sched_yield();
        }
        if (g_gc_debug) (void)write(2, "[mk] resume-done\n", 17);
        pthread_mutex_unlock(&g_gc_mu);
        g_gc_executor = 0;
        gc_unblock_stop(&gc_old);
        return;
    }

    // ===== 单线程快路径（无活跃 spawn 线程）=====
    GCHash set;
    gc_hash_init(&set, (size_t)g_obj_count * 2);
    for (int i = 0; i < g_obj_count; i++) gc_hash_insert(&set, (uintptr_t)g_objs[i]);
    g_gc_marked = 0;
    for (int i = 0; i < g_len; i++) {
        if (px_value_is_obj(g_vals[i]) && g_vals[i].as.obj) gc_mark_obj(&set, g_vals[i].as.obj);
    }
    if (g_tmp_root) gc_mark_obj(&set, g_tmp_root);
    ucontext_t uc;
    getcontext(&uc);
    gc_scan_stack(&set);
    int freed = 0, w = 0;
    for (int i = 0; i < g_obj_count; i++) {
        LXObject* o = g_objs[i];
        if (!gc_hash_has(&set, (uintptr_t)o)) continue;   // 防御：损坏条目，丢弃
        if (o->gc_mark) {
            o->gc_mark = 0;
            g_objs[w++] = o;
        } else {
            px_obj_free(o);
            freed++;
        }
    }
    g_obj_count = w;
    g_alloc_bytes = 0;
    g_gc_freed += freed;
    g_gc_runs++;
    g_tmp_root = NULL;
    if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
    gc_debug("collect #%d: 标记 %lld/%d 回收 %d 存活 %d 跳过 %d", g_gc_runs, g_gc_marked, g_obj_count + freed, freed, g_obj_count, g_gc_skips);
    gc_hash_free(&set);
    pthread_mutex_unlock(&g_gc_mu);
    g_gc_executor = 0;
    gc_unblock_stop(&gc_old);
}

// 注册对象（构造时调用）。est = 估算占用字节（触发字节阈值用，当前主用对象数阈值）。
static void gc_register(LXObject* o, long long est) {
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    o->gc_mark = 0;   // 关键：xmalloc 未清零，gc_mark 垃圾值=1 会导致 DFS 跳过该节点（子对象漏标）
    if (g_obj_count >= g_obj_cap) {
        int ncap = g_obj_cap ? g_obj_cap * 2 : 8192;
        g_objs = xrealloc(g_objs, sizeof(LXObject*) * ncap);
        g_obj_cap = ncap;
    }
    g_objs[g_obj_count++] = o;
    g_alloc_bytes += est;
    g_tmp_root = o;  // 保护刚创建对象
    int need = (g_obj_count >= g_gc_threshold) ||
               (g_gc_trigger_bytes && g_alloc_bytes >= g_gc_trigger_bytes);
    pthread_mutex_unlock(&g_gc_mu);
    if (need) px_gc_collect();
}

int px_gc_stats(int* live, int* total) {
    pthread_mutex_lock(&g_gc_mu);
    if (live) *live = g_obj_count;
    if (total) *total = g_gc_freed;
    int runs = g_gc_runs;
    pthread_mutex_unlock(&g_gc_mu);
    return runs;
}

// 调试辅助：对象是否仍注册在对象表（未回收）
int px_gc_contains(LXObject* o) {
    pthread_mutex_lock(&g_gc_mu);
    int found = 0;
    for (int i = 0; i < g_obj_count; i++) {
        if (g_objs[i] == o) { found = 1; break; }
    }
    pthread_mutex_unlock(&g_gc_mu);
    return found;
}

// ==================== 值构造 ====================

LXValue px_null(void) { LXValue v; v.type = PX_NULL; v.as.i = 0; return v; }
LXValue px_bool(bool b) { LXValue v; v.type = PX_BOOL; v.as.b = b; return v; }
LXValue px_int(int64_t i) { LXValue v; v.type = PX_INT; v.as.i = i; return v; }
LXValue px_float(double f) { LXValue v; v.type = PX_FLOAT; v.as.f = f; return v; }

LXValue px_str_len(const char* s, int len) {
    LXValue v; v.type = PX_STR;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_STR;
    char* d = xmalloc(len + 1);
    memcpy(d, s, len); d[len] = 0;
    o->as.str.data = d; o->as.str.len = len;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + len + 1);
    return v;
}

LXValue px_str(const char* s) { return px_str_len(s, (int)strlen(s)); }

// M23b：二进制安全字节串构造（复制 len 字节，可含 NUL；union 复用 str data/len）
LXValue px_bytes_len(const void* data, int len) {
    LXValue v; v.type = PX_BYTES;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_BYTES;
    char* d = xmalloc(len + 1);
    if (len > 0 && data) memcpy(d, data, (size_t)len);
    d[len] = 0;
    o->as.str.data = d; o->as.str.len = len;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + len + 1);
    return v;
}

LXValue px_list(int cap) {
    LXValue v; v.type = PX_LIST;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_LIST;
    o->as.list.items = xmalloc(sizeof(LXValue) * (cap > 0 ? cap : 8));
    o->as.list.len = 0; o->as.list.cap = cap > 0 ? cap : 8;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + (size_t)o->as.list.cap * sizeof(LXValue));
    return v;
}

LXValue px_list_n(LXValue* items, int n) {
    LXValue v = px_list(n);
    for (int i = 0; i < n; i++) px_list_push(v, items[i]);
    return v;
}

LXValue px_dict(void) {
    LXValue v; v.type = PX_DICT;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_DICT;
    o->as.dict.keys = xmalloc(sizeof(char*) * 8);
    o->as.dict.vals = xmalloc(sizeof(LXValue) * 8);
    o->as.dict.len = 0; o->as.dict.cap = 8;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + 8 * (sizeof(char*) + sizeof(LXValue)));
    return v;
}

LXValue px_func(const char* name, LXFuncPtr fn, void* ctx) {
    LXValue v; v.type = PX_FUNC;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_FUNC;
    o->as.func.name = xstrdup(name); o->as.func.fn = fn; o->as.func.ctx = ctx;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + strlen(name) + 1);
    return v;
}

LXValue px_native(const char* name, LXFuncPtr fn) {
    LXValue v; v.type = PX_NATIVE;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_NATIVE;
    o->as.native.name = xstrdup(name); o->as.native.fn = fn;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + strlen(name) + 1);
    return v;
}

LXValue px_struct(const char* type_name, char** fnames, LXValue* fvals, int nfields) {
    LXValue v; v.type = PX_STRUCT;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_STRUCT;
    o->as.struct_inst.type_name = xstrdup(type_name);
    o->as.struct_inst.fnames = xmalloc(sizeof(char*) * (nfields ? nfields : 1));
    o->as.struct_inst.fvals = xmalloc(sizeof(LXValue) * (nfields ? nfields : 1));
    long long est = sizeof(LXObject) + strlen(type_name) + 1;
    for (int i = 0; i < nfields; i++) {
        o->as.struct_inst.fnames[i] = xstrdup(fnames[i]);
        o->as.struct_inst.fvals[i] = fvals[i];
        est += strlen(fnames[i]) + 1 + sizeof(LXValue);
    }
    o->as.struct_inst.nfields = nfields;
    v.as.obj = o;
    gc_register(o, est);
    return v;
}

LXValue px_enum(const char* type_name, const char* variant) {
    LXValue v; v.type = PX_ENUM;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_ENUM;
    o->as.enum_inst.type_name = xstrdup(type_name);
    o->as.enum_inst.variant = xstrdup(variant);
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + strlen(type_name) + strlen(variant) + 2);
    return v;
}

LXValue px_tuple(LXValue* items, int len) {
    LXValue v; v.type = PX_TUPLE;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_TUPLE;
    o->as.tuple.items = xmalloc(sizeof(LXValue) * (len ? len : 1));
    for (int i = 0; i < len; i++) o->as.tuple.items[i] = items[i];
    o->as.tuple.len = len;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + (size_t)(len ? len : 1) * sizeof(LXValue));
    return v;
}

// ==================== 类型判断 ====================

bool px_is_null(LXValue v) { return v.type == PX_NULL; }

bool px_is_truthy(LXValue v) {
    switch (v.type) {
        case PX_NULL: return false;
        case PX_BOOL: return v.as.b;
        case PX_INT: return v.as.i != 0;
        case PX_FLOAT: return v.as.f != 0.0;
        case PX_STR: return v.as.obj->as.str.len > 0;
        case PX_BYTES: return v.as.obj->as.str.len > 0;
        case PX_LIST: return v.as.obj->as.list.len > 0;
        case PX_DICT: return v.as.obj->as.dict.len > 0;
        default: return true;
    }
}

const char* px_type_name(LXValue v) {
    switch (v.type) {
        case PX_NULL: return "null";
        case PX_BOOL: return "bool";
        case PX_INT: return "int";
        case PX_FLOAT: return "float";
        case PX_STR: return "string";
        case PX_BYTES: return "bytes";
        case PX_LIST: return "list";
        case PX_DICT: return "dict";
        case PX_FUNC: return "function";
        case PX_NATIVE: return "native";
        case PX_STRUCT: return "struct";
        case PX_ENUM: return "enum";
        case PX_TUPLE: return "tuple";
        case PX_CHAN: return "chan";
        case PX_MUTEX: return "mutex";
        case PX_RWLOCK: return "rwlock";
    }
    return "unknown";
}

// ==================== 错误 ====================

void px_error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "运行时错误: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

// ==================== 字符串工具 ====================

int px_unicode_len(const char* s) {
    int n = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
        if ((*p & 0xC0) != 0x80) n++;  // 非连续字节 = 新字符
    }
    return n;
}

// 简单数字转字符串（int/float）
static char num_buf[64];
static const char* fmt_num(LXValue v) {
    if (v.type == PX_INT) {
        snprintf(num_buf, sizeof(num_buf), "%lld", (long long)v.as.i);
    } else {
        snprintf(num_buf, sizeof(num_buf), "%g", v.as.f);
    }
    return num_buf;
}

// ==================== 输出 ====================

static char* escape_str(const char* s, int len) {
    char* out = xmalloc(len * 4 + 1);
    int j = 0;
    for (int i = 0; i < len; i++) {
        unsigned char c = s[i];
        switch (c) {
            case '\n': out[j++] = '\\'; out[j++] = 'n'; break;
            case '\t': out[j++] = '\\'; out[j++] = 't'; break;
            case '\r': out[j++] = '\\'; out[j++] = 'r'; break;
            case '"': out[j++] = '\\'; out[j++] = '"'; break;
            case '\\': out[j++] = '\\'; out[j++] = '\\'; break;
            default:
                if (c < 0x20) { snprintf(out + j, 5, "\\x%02x", c); j += 4; }
                else out[j++] = (char)c;
        }
    }
    out[j] = 0;
    return out;
}

void px_print_value(LXValue v, bool newline) {
    switch (v.type) {
        case PX_NULL: printf("null"); break;
        case PX_BOOL: printf(v.as.b ? "true" : "false"); break;
        case PX_INT: printf("%lld", (long long)v.as.i); break;
        case PX_FLOAT: printf("%g", v.as.f); break;
        case PX_STR: fwrite(v.as.obj->as.str.data, 1, v.as.obj->as.str.len, stdout); break;
        case PX_BYTES: printf("<bytes %d>", v.as.obj->as.str.len); break;
        case PX_LIST: {
            printf("[");
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.list.len; i++) {
                if (i) printf(", ");
                px_print_value(o->as.list.items[i], false);
            }
            printf("]");
            break;
        }
        case PX_TUPLE: {
            printf("(");
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.tuple.len; i++) {
                if (i) printf(", ");
                px_print_value(o->as.tuple.items[i], false);
            }
            printf(")");
            break;
        }
        case PX_DICT: {
            printf("{");
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.dict.len; i++) {
                if (i) printf(", ");
                printf("\"%s\": ", o->as.dict.keys[i]);
                px_print_value(o->as.dict.vals[i], false);
            }
            printf("}");
            break;
        }
        case PX_FUNC: printf("<fn %s>", v.as.obj->as.func.name); break;
        case PX_NATIVE: printf("<native %s>", v.as.obj->as.native.name); break;
        case PX_STRUCT: {
            LXObject* o = v.as.obj;
            printf("%s(", o->as.struct_inst.type_name);
            for (int i = 0; i < o->as.struct_inst.nfields; i++) {
                if (i) printf(", ");
                printf("%s=", o->as.struct_inst.fnames[i]);
                px_print_value(o->as.struct_inst.fvals[i], false);
            }
            printf(")");
            break;
        }
        case PX_ENUM: printf("%s.%s", v.as.obj->as.enum_inst.type_name, v.as.obj->as.enum_inst.variant); break;
        default: printf("?"); break;
    }
    if (newline) printf("\n");
}

char* px_to_string(LXValue v) {
    static char* buf = NULL;
    static int cap = 0;
    // 简化：针对 int/float 直接用 num_buf，字符串用转义缓冲
    if (v.type == PX_INT || v.type == PX_FLOAT) return (char*)fmt_num(v);
    if (v.type == PX_BOOL) return v.as.b ? (char*)"true" : (char*)"false";
    if (v.type == PX_NULL) return (char*)"null";
    if (v.type == PX_STR) return (char*)v.as.obj->as.str.data;
    // 其他类型：写临时文件流
    FILE* tmp = tmpfile();
    if (tmp) { px_print_value(v, false); fflush(tmp); }
    return (char*)"<object>";
}

// ==================== 运算 ====================

static double num_val(LXValue v) {
    return v.type == PX_INT ? (double)v.as.i : v.as.f;
}
static int64_t int_val(LXValue v) {
    if (v.type == PX_INT) return v.as.i;
    if (v.type == PX_FLOAT) return (int64_t)v.as.f;
    px_error("期望整数，实际是 %s", px_type_name(v));
    return 0;
}

LXValue px_add(LXValue a, LXValue b) {
    if (a.type == PX_STR && b.type == PX_STR) {
        int la = a.as.obj->as.str.len, lb = b.as.obj->as.str.len;
        char* d = xmalloc(la + lb + 1);
        memcpy(d, a.as.obj->as.str.data, la);
        memcpy(d + la, b.as.obj->as.str.data, lb);
        d[la + lb] = 0;
        return px_str_len(d, la + lb);
    }
    if (a.type == PX_INT && b.type == PX_INT) return px_int(a.as.i + b.as.i);
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) return px_float(num_val(a) + num_val(b));
    if (a.type == PX_LIST && b.type == PX_LIST) {
        LXValue r = px_list(a.as.obj->as.list.len + b.as.obj->as.list.len);
        LXObject* ro = r.as.obj; LXObject* ao = a.as.obj; LXObject* bo = b.as.obj;
        for (int i = 0; i < ao->as.list.len; i++) px_list_push(r, ao->as.list.items[i]);
        for (int i = 0; i < bo->as.list.len; i++) px_list_push(r, bo->as.list.items[i]);
        (void)ro;
        return r;
    }
    px_error("无法相加: %s + %s", px_type_name(a), px_type_name(b));
    return px_null();
}

LXValue px_sub(LXValue a, LXValue b) {
    if (a.type == PX_INT && b.type == PX_INT) return px_int(a.as.i - b.as.i);
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) return px_float(num_val(a) - num_val(b));
    px_error("无法相减: %s - %s", px_type_name(a), px_type_name(b));
    return px_null();
}

LXValue px_mul(LXValue a, LXValue b) {
    if (a.type == PX_INT && b.type == PX_INT) return px_int(a.as.i * b.as.i);
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) return px_float(num_val(a) * num_val(b));
    if (a.type == PX_STR && b.type == PX_INT) {
        int n = (int)b.as.i;
        int len = a.as.obj->as.str.len;
        char* d = xmalloc(len * n + 1);
        for (int i = 0; i < n; i++) memcpy(d + i * len, a.as.obj->as.str.data, len);
        d[len * n] = 0;
        return px_str_len(d, len * n);
    }
    px_error("无法相乘: %s * %s", px_type_name(a), px_type_name(b));
    return px_null();
}

LXValue px_div(LXValue a, LXValue b) {
    double d = num_val(b);
    if (d == 0) px_error("除零错误");
    return px_float(num_val(a) / d);
}

LXValue px_idiv(LXValue a, LXValue b) {
    int64_t d = int_val(b);
    if (d == 0) px_error("除零错误");
    return px_int(int_val(a) / d);
}

LXValue px_mod(LXValue a, LXValue b) {
    int64_t d = int_val(b);
    if (d == 0) px_error("取模除零错误");
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) return px_float(fmod(num_val(a), num_val(b)));
    return px_int(int_val(a) % d);
}

LXValue px_pow(LXValue a, LXValue b) {
    if (a.type == PX_INT && b.type == PX_INT && b.as.i >= 0) {
        int64_t r = 1;
        for (int64_t i = 0; i < b.as.i; i++) r *= a.as.i;
        return px_int(r);
    }
    return px_float(pow(num_val(a), num_val(b)));
}

LXValue px_neg(LXValue a) {
    if (a.type == PX_INT) return px_int(-a.as.i);
    if (a.type == PX_FLOAT) return px_float(-a.as.f);
    px_error("无法取负: -%s", px_type_name(a));
    return px_null();
}

LXValue px_not(LXValue a) { return px_bool(!px_is_truthy(a)); }
LXValue px_bitnot(LXValue a) { return px_int(~int_val(a)); }
LXValue px_bitand(LXValue a, LXValue b) { return px_int(int_val(a) & int_val(b)); }
LXValue px_bitor(LXValue a, LXValue b) { return px_int(int_val(a) | int_val(b)); }
LXValue px_bitxor(LXValue a, LXValue b) { return px_int(int_val(a) ^ int_val(b)); }
LXValue px_shl(LXValue a, LXValue b) { return px_int(int_val(a) << int_val(b)); }
LXValue px_shr(LXValue a, LXValue b) { return px_int(int_val(a) >> int_val(b)); }
LXValue px_ushr(LXValue a, LXValue b) {
    // 无符号（逻辑）右移：按 uint64 解释后右移，再转回 int64。
    // 移位量对 64 取模（与解释器 wrapping_shr 一致；负移位量按无符号取模）。
    uint64_t v = (uint64_t)int_val(a);
    uint64_t sh = (uint64_t)int_val(b) & 63u;
    return px_int((int64_t)(v >> sh));
}

static int compare_values(LXValue a, LXValue b) {
    if (a.type == PX_INT && b.type == PX_INT) {
        return a.as.i < b.as.i ? -1 : (a.as.i > b.as.i ? 1 : 0);
    }
    if ((a.type == PX_INT || a.type == PX_FLOAT) && (b.type == PX_INT || b.type == PX_FLOAT)) {
        double x = num_val(a), y = num_val(b);
        return x < y ? -1 : (x > y ? 1 : 0);
    }
    if (a.type == PX_STR && b.type == PX_STR) {
        int la = a.as.obj->as.str.len, lb = b.as.obj->as.str.len;
        int m = la < lb ? la : lb;
        int c = memcmp(a.as.obj->as.str.data, b.as.obj->as.str.data, m);
        if (c != 0) return c < 0 ? -1 : 1;
        return la < lb ? -1 : (la > lb ? 1 : 0);
    }
    if (a.type == PX_BYTES && b.type == PX_BYTES) {
        int la = a.as.obj->as.str.len, lb = b.as.obj->as.str.len;
        int m = la < lb ? la : lb;
        int c = memcmp(a.as.obj->as.str.data, b.as.obj->as.str.data, m);
        if (c != 0) return c < 0 ? -1 : 1;
        return la < lb ? -1 : (la > lb ? 1 : 0);
    }
    if (a.type == PX_BOOL && b.type == PX_BOOL) {
        return a.as.b == b.as.b ? 0 : (a.as.b ? 1 : -1);
    }
    if (a.type == PX_ENUM && b.type == PX_ENUM) {
        return strcmp(a.as.obj->as.enum_inst.variant, b.as.obj->as.enum_inst.variant);
    }
    if ((a.type == PX_LIST || a.type == PX_TUPLE) && (b.type == PX_LIST || b.type == PX_TUPLE)) {
        int na = (a.type == PX_LIST) ? a.as.obj->as.list.len : a.as.obj->as.tuple.len;
        int nb = (b.type == PX_LIST) ? b.as.obj->as.list.len : b.as.obj->as.tuple.len;
        int m = na < nb ? na : nb;
        for (int i = 0; i < m; i++) {
            LXValue x = (a.type == PX_LIST) ? a.as.obj->as.list.items[i] : a.as.obj->as.tuple.items[i];
            LXValue y = (b.type == PX_LIST) ? b.as.obj->as.list.items[i] : b.as.obj->as.tuple.items[i];
            int c = compare_values(x, y);
            if (c != 0) return c;
        }
        return na < nb ? -1 : (na > nb ? 1 : 0);
    }
    if (a.type == PX_NULL && b.type == PX_NULL) return 0;
    // 默认按类型名比较，保证可比性
    return strcmp(px_type_name(a), px_type_name(b));
}

LXValue px_eq(LXValue a, LXValue b) {
    // 数值跨类型相等：1 == 1.0
    if ((a.type == PX_INT || a.type == PX_FLOAT) && (b.type == PX_INT || b.type == PX_FLOAT))
        return px_bool(num_val(a) == num_val(b));
    return px_bool(compare_values(a, b) == 0);
}
LXValue px_ne(LXValue a, LXValue b) { return px_bool(compare_values(a, b) != 0); }
LXValue px_lt(LXValue a, LXValue b) { return px_bool(compare_values(a, b) < 0); }
LXValue px_le(LXValue a, LXValue b) { return px_bool(compare_values(a, b) <= 0); }
LXValue px_gt(LXValue a, LXValue b) { return px_bool(compare_values(a, b) > 0); }
LXValue px_ge(LXValue a, LXValue b) { return px_bool(compare_values(a, b) >= 0); }

LXValue px_and(LXValue a, LXValue b) {
    return px_is_truthy(a) ? b : a;  // 短路由 codegen 保证
}
LXValue px_or(LXValue a, LXValue b) {
    return px_is_truthy(a) ? a : b;
}

// ==================== 容器操作 ====================

LXValue px_index(LXValue obj, LXValue idx) {
    if (obj.type == PX_LIST) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.list.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("列表索引越界: %d (len=%d)", i, len);
        return obj.as.obj->as.list.items[i];
    }
    if (obj.type == PX_TUPLE) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.tuple.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("元组索引越界: %d", i);
        return obj.as.obj->as.tuple.items[i];
    }
    if (obj.type == PX_STR) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.str.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("字符串索引越界: %d", i);
        // 返回单字符（按字节；中文需要字节切片，MVP 简化）
        char buf[8] = {0};
        buf[0] = obj.as.obj->as.str.data[i];
        return px_str(buf);
    }
    if (obj.type == PX_DICT) {
        if (idx.type == PX_STR) {
            return px_dict_get(obj, idx.as.obj->as.str.data);
        }
        px_error("字典索引需要字符串键");
    }
    px_error("无法索引: %s", px_type_name(obj));
    return px_null();
}

// ==================== M21/M24 切片 a[start:end] / a[start:end:step] ====================
// start/end/step 为 PX_NULL 表示省略；负索引从尾部算；越界 clamp；step<0 反向，step=0 报错。
// str 按 UTF-8 字符切（与解释器字符语义一致，中文正常）；list/tuple/bytes 取元素返回新对象。

// M24：切片边界调整（Python slice_adjust 语义，与解释器 Rust adjust 逐字节一致）
// v<0 先 +len；再按步长方向 clamp：step>0 → [0,len]，step<0 → [-1,len-1]
static int64_t px_slice_adjust(int64_t v, int len, int64_t step) {
    if (v < 0) v += len;
    if (step > 0) {
        if (v < 0) v = 0;
        if (v > len) v = len;
    } else {
        if (v < -1) v = -1;
        if (v > len - 1) v = len - 1;
    }
    return v;
}

LXValue px_slice(LXValue obj, LXValue start, LXValue end, LXValue step) {
    // Python slice.indices(len) 语义（与解释器 interp.rs slice_indices 逐字节一致）：
    //   step 缺省=1，step=0 报错；step>0 时 start 缺省 0、end 缺省 len；
    //   step<0 时 start 缺省 len-1、end 缺省 -1（取到索引 0 含）；
    //   负边界 +len 后按步长方向 clamp。
    int s_missing = start.type == PX_NULL;
    int e_missing = end.type == PX_NULL;
    int k_missing = step.type == PX_NULL;
    int64_t k_in = k_missing ? 1 : int_val(step);
    if (k_in == 0) px_error("切片步长不能为 0");

    int len, kind = 0; // 0=list 1=tuple 2=str 3=bytes
    if (obj.type == PX_LIST) { kind = 0; len = obj.as.obj->as.list.len; }
    else if (obj.type == PX_TUPLE) { kind = 1; len = obj.as.obj->as.tuple.len; }
    else if (obj.type == PX_STR) { kind = 2; len = px_unicode_len(obj.as.obj->as.str.data); }
    else if (obj.type == PX_BYTES) { kind = 3; len = obj.as.obj->as.str.len; }
    else { px_error("无法切片: %s", px_type_name(obj)); return px_null(); }

    int64_t s = s_missing ? (k_in < 0 ? len - 1 : 0) : px_slice_adjust(int_val(start), len, k_in);
    int64_t e = e_missing ? (k_in < 0 ? -1 : len) : px_slice_adjust(int_val(end), len, k_in);

    // 元素个数
    int n = 0;
    if (k_in > 0) { for (int64_t i = s; i < e; i += k_in) n++; }
    else { for (int64_t i = s; i > e; i += k_in) n++; }

    if (kind == 0) { // list
        LXValue r = px_list(n);
        if (k_in > 0) { for (int64_t i = s; i < e; i += k_in) px_list_push(r, obj.as.obj->as.list.items[(int)i]); }
        else { for (int64_t i = s; i > e; i += k_in) px_list_push(r, obj.as.obj->as.list.items[(int)i]); }
        return r;
    }
    if (kind == 1) { // tuple
        LXValue* items = xmalloc(sizeof(LXValue) * (size_t)(n > 0 ? n : 1));
        int j = 0;
        if (k_in > 0) { for (int64_t i = s; i < e; i += k_in) items[j++] = obj.as.obj->as.tuple.items[(int)i]; }
        else { for (int64_t i = s; i > e; i += k_in) items[j++] = obj.as.obj->as.tuple.items[(int)i]; }
        LXValue r = px_tuple(items, j);
        xfree(items);
        return r;
    }
    if (kind == 3) { // bytes（按字节）
        const char* data = obj.as.obj->as.str.data;
        char* out = xmalloc((size_t)(n + 1));
        int j = 0;
        if (k_in > 0) { for (int64_t i = s; i < e; i += k_in) out[j++] = data[(int)i]; }
        else { for (int64_t i = s; i > e; i += k_in) out[j++] = data[(int)i]; }
        LXValue r = px_bytes_len(out, j);
        xfree(out);
        return r;
    }
    // str：按 UTF-8 字符收集（预构建字符字节偏移表）
    const char* data = obj.as.obj->as.str.data;
    int blen = obj.as.obj->as.str.len;
    int* offs = xmalloc(sizeof(int) * (size_t)(len + 1));
    int boff = 0;
    offs[0] = 0;
    for (int c = 0; c < len; c++) {
        boff++;
        while (boff < blen && ((unsigned char)data[boff] & 0xC0) == 0x80) boff++;
        offs[c + 1] = boff;
    }
    int total = 0;
    if (k_in > 0) { for (int64_t i = s; i < e; i += k_in) total += offs[(int)i + 1] - offs[(int)i]; }
    else { for (int64_t i = s; i > e; i += k_in) total += offs[(int)i + 1] - offs[(int)i]; }
    char* out = xmalloc((size_t)(total + 1));
    int oi = 0;
    if (k_in > 0) {
        for (int64_t i = s; i < e; i += k_in) {
            int cl = offs[(int)i + 1] - offs[(int)i];
            memcpy(out + oi, data + offs[(int)i], (size_t)cl);
            oi += cl;
        }
    } else {
        for (int64_t i = s; i > e; i += k_in) {
            int cl = offs[(int)i + 1] - offs[(int)i];
            memcpy(out + oi, data + offs[(int)i], (size_t)cl);
            oi += cl;
        }
    }
    out[oi] = 0;
    LXValue r = px_str_len(out, oi);
    xfree(out);
    xfree(offs);
    return r;
}

void px_index_set(LXValue obj, LXValue idx, LXValue val) {
    if (obj.type == PX_LIST) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.list.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("列表索引越界: %d", i);
        // M11：与 GC 互斥（见 px_list_push 注释）。注意：必须先拿锁再屏蔽信号——
        // 等锁期间不能屏蔽 SIG_GC_STOP，否则 GC 无法暂停该线程（信号 pending），
        // 导致 stop-the-world 空转/降级/漏扫描。
        sigset_t old;
        pthread_mutex_lock(&g_gc_mu);
        gc_block_stop(&old);
        obj.as.obj->as.list.items[i] = val;
        gc_unblock_stop(&old);
        pthread_mutex_unlock(&g_gc_mu);
        return;
    }
    if (obj.type == PX_DICT) {
        if (idx.type == PX_STR) {
            px_dict_set(obj, idx.as.obj->as.str.data, val);   // 内部已互斥
            return;
        }
        px_error("字典索引需要字符串键");
    }
    px_error("无法索引赋值: %s", px_type_name(obj));
}

LXValue px_field(LXValue obj, const char* name) {
    if (obj.type == PX_STRUCT) {
        LXObject* o = obj.as.obj;
        for (int i = 0; i < o->as.struct_inst.nfields; i++) {
            if (strcmp(o->as.struct_inst.fnames[i], name) == 0) return o->as.struct_inst.fvals[i];
        }
        px_error("结构体 %s 没有字段 %s", o->as.struct_inst.type_name, name);
    }
    if (obj.type == PX_DICT) return px_dict_get(obj, name);
    px_error("无法取字段: %s.%s", px_type_name(obj), name);
    return px_null();
}

void px_field_set(LXValue obj, const char* name, LXValue val) {
    if (obj.type == PX_STRUCT) {
        LXObject* o = obj.as.obj;
        for (int i = 0; i < o->as.struct_inst.nfields; i++) {
            if (strcmp(o->as.struct_inst.fnames[i], name) == 0) {
                // M11：与 GC 互斥（见 px_list_push 注释）
                sigset_t old;
                pthread_mutex_lock(&g_gc_mu);
                gc_block_stop(&old);
                o->as.struct_inst.fvals[i] = val;
                gc_unblock_stop(&old);
                pthread_mutex_unlock(&g_gc_mu);
                return;
            }
        }
        px_error("结构体 %s 没有字段 %s", o->as.struct_inst.type_name, name);
    }
    px_error("无法字段赋值: %s.%s", px_type_name(obj), name);
}

void px_list_push(LXValue list, LXValue val) {
    LXObject* o = list.as.obj;
    // M11：对象结构修改与 GC 标记/清扫通过 g_gc_mu 互斥（消除数据竞争）。
    // 必须先拿锁再屏蔽信号：等锁期间若屏蔽 SIG_GC_STOP，GC 无法暂停本线程
    // （信号 pending），导致 stop-the-world 空转、GC 降级、栈漏扫描（use-after-free）。
    // 持锁后屏蔽：持锁期间 GC 主线程在等锁（不会发信号），不会被挂起。
    sigset_t old;
    pthread_mutex_lock(&g_gc_mu);
    gc_block_stop(&old);
    if (o->as.list.len >= o->as.list.cap) {
        o->as.list.cap *= 2;
        o->as.list.items = xrealloc(o->as.list.items, sizeof(LXValue) * o->as.list.cap);
    }
    o->as.list.items[o->as.list.len++] = val;
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_gc_mu);
}

void px_dict_set(LXValue dict, const char* key, LXValue val) {
    LXObject* o = dict.as.obj;
    // M11：与 GC 通过 g_gc_mu 互斥（见 px_list_push 注释）。先拿锁再屏蔽信号。
    sigset_t old;
    pthread_mutex_lock(&g_gc_mu);
    gc_block_stop(&old);
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], key) == 0) {
            o->as.dict.vals[i] = val;
            gc_unblock_stop(&old);
            pthread_mutex_unlock(&g_gc_mu);
            return;
        }
    }
    if (o->as.dict.len >= o->as.dict.cap) {
        o->as.dict.cap *= 2;
        o->as.dict.keys = xrealloc(o->as.dict.keys, sizeof(char*) * o->as.dict.cap);
        o->as.dict.vals = xrealloc(o->as.dict.vals, sizeof(LXValue) * o->as.dict.cap);
    }
    o->as.dict.keys[o->as.dict.len] = xstrdup(key);
    o->as.dict.vals[o->as.dict.len] = val;
    o->as.dict.len++;
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_gc_mu);
}

LXValue px_dict_get(LXValue dict, const char* key) {
    LXObject* o = dict.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], key) == 0) return o->as.dict.vals[i];
    }
    return px_null();
}

bool px_dict_has(LXValue dict, const char* key) {
    LXObject* o = dict.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], key) == 0) return true;
    }
    return false;
}

int px_len(LXValue v) {
    switch (v.type) {
        case PX_STR: return px_unicode_len(v.as.obj->as.str.data);
        case PX_LIST: return v.as.obj->as.list.len;
        case PX_DICT: return v.as.obj->as.dict.len;
        case PX_TUPLE: return v.as.obj->as.tuple.len;
        default: px_error("len 不支持类型 %s", px_type_name(v)); return 0;
    }
}

// ==================== 调用 ====================

LXValue px_call(LXValue fn, LXValue* args, int nargs) {
    if (fn.type == PX_FUNC) return fn.as.obj->as.func.fn(args, nargs, fn.as.obj->as.func.ctx);
    if (fn.type == PX_NATIVE) return fn.as.obj->as.native.fn(args, nargs, NULL);
    px_error("无法调用非函数: %s", px_type_name(fn));
    return px_null();
}

// 以 self 为第一参数调用全局函数（字符串方法转发）
static LXValue call_with_self(const char* fn, LXValue self, LXValue* args, int nargs) {
    LXValue* a = xmalloc(sizeof(LXValue) * (nargs + 1));
    a[0] = self;
    for (int i = 0; i < nargs; i++) a[i+1] = args[i];
    LXValue r = px_call(px_get_global(fn), a, nargs + 1);
    xfree(a);
    return r;
}

LXValue px_method(LXValue obj, const char* name, LXValue* args, int nargs) {
    // 通道方法
    if (obj.type == PX_CHAN) {
        if (strcmp(name, "send") == 0) {
            if (nargs != 1) px_error("send 需要 1 个参数");
            return px_chan_send(obj, args[0]);
        }
        if (strcmp(name, "recv") == 0) return px_chan_recv(obj);
        if (strcmp(name, "close") == 0) { px_chan_close(obj); return px_null(); }
    }
    // 互斥锁方法（M13）
    if (obj.type == PX_MUTEX) {
        if (strcmp(name, "lock") == 0) return px_mutex_lock(obj);
        if (strcmp(name, "unlock") == 0) return px_mutex_unlock(obj);
        if (strcmp(name, "try_lock") == 0) return px_mutex_try_lock(obj);
        if (strcmp(name, "with") == 0) {
            if (nargs != 1) px_error("mutex.with 需要 1 个函数参数");
            px_mutex_lock(obj);
            LXValue r = px_call(args[0], NULL, 0);
            px_mutex_unlock(obj);
            return r;
        }
    }
    // 读写锁方法（M13）
    if (obj.type == PX_RWLOCK) {
        if (strcmp(name, "rlock") == 0) return px_rwlock_rlock(obj);
        if (strcmp(name, "runlock") == 0) return px_rwlock_runlock(obj);
        if (strcmp(name, "wlock") == 0) return px_rwlock_wlock(obj);
        if (strcmp(name, "wunlock") == 0) return px_rwlock_wunlock(obj);
        if (strcmp(name, "try_rlock") == 0) return px_rwlock_try_rlock(obj);
        if (strcmp(name, "try_wlock") == 0) return px_rwlock_try_wlock(obj);
        if (strcmp(name, "with_read") == 0) {
            if (nargs != 1) px_error("rwlock.with_read 需要 1 个函数参数");
            px_rwlock_rlock(obj);
            LXValue r = px_call(args[0], NULL, 0);
            px_rwlock_runlock(obj);
            return r;
        }
        if (strcmp(name, "with_write") == 0) {
            if (nargs != 1) px_error("rwlock.with_write 需要 1 个函数参数");
            px_rwlock_wlock(obj);
            LXValue r = px_call(args[0], NULL, 0);
            px_rwlock_wunlock(obj);
            return r;
        }
    }
    // 字符串方法
    if (obj.type == PX_STR) {
        if (strcmp(name, "upper") == 0 || strcmp(name, "to_upper") == 0) {
            return call_with_self("to_upper", obj, args, nargs);
        }
        if (strcmp(name, "lower") == 0 || strcmp(name, "to_lower") == 0) {
            return call_with_self("to_lower", obj, args, nargs);
        }
        if (strcmp(name, "len") == 0) return px_int(px_len(obj));
        if (strcmp(name, "trim") == 0) return call_with_self("trim", obj, args, nargs);
        if (strcmp(name, "split") == 0) return call_with_self("split", obj, args, nargs);
        if (strcmp(name, "contains") == 0) return call_with_self("contains", obj, args, nargs);
        if (strcmp(name, "replace") == 0) return call_with_self("replace", obj, args, nargs);
        if (strcmp(name, "starts_with") == 0) return call_with_self("starts_with", obj, args, nargs);
        if (strcmp(name, "ends_with") == 0) return call_with_self("ends_with", obj, args, nargs);
    }
    if (obj.type == PX_LIST) {
        if (strcmp(name, "append") == 0) {
            if (nargs != 1) px_error("append 需要 1 个参数");
            px_list_push(obj, args[0]);
            return px_null();
        }
        if (strcmp(name, "len") == 0) return px_int(px_len(obj));
        if (strcmp(name, "push") == 0) { px_list_push(obj, args[0]); return px_null(); }
        if (strcmp(name, "contains") == 0) {
            if (nargs < 1) px_error("contains 需要 1 个参数");
            LXObject* o = obj.as.obj;
            for (int i = 0; i < o->as.list.len; i++) {
                if (px_eq(o->as.list.items[i], args[0]).as.b) return px_bool(true);
            }
            return px_bool(false);
        }
        if (strcmp(name, "join") == 0) return call_with_self("join", args[0], &obj, 1);
    }
    if (obj.type == PX_DICT) {
        if (strcmp(name, "get") == 0) {
            if (nargs < 1) px_error("get 需要 1 个参数");
            return px_dict_get(obj, args[0].as.obj->as.str.data);
        }
        if (strcmp(name, "set") == 0) {
            if (nargs < 2) px_error("set 需要 2 个参数");
            px_dict_set(obj, args[0].as.obj->as.str.data, args[1]);
            return px_null();
        }
        if (strcmp(name, "len") == 0) return px_int(px_len(obj));
        if (strcmp(name, "has") == 0 || strcmp(name, "contains") == 0) {
            if (nargs < 1) px_error("has 需要 1 个参数");
            return px_bool(px_dict_has(obj, args[0].as.obj->as.str.data));
        }
        if (strcmp(name, "keys") == 0) {
            LXObject* o = obj.as.obj;
            LXValue r = px_list(0);
            for (int i = 0; i < o->as.dict.len; i++) px_list_push(r, px_str(o->as.dict.keys[i]));
            return r;
        }
        if (strcmp(name, "remove") == 0) {
            if (nargs < 1) px_error("remove 需要 1 个参数");
            LXValue v = px_dict_get(obj, args[0].as.obj->as.str.data);
            px_dict_set(obj, args[0].as.obj->as.str.data, px_null()); // 简化：置 null 表示删除
            return v;
        }
    }
    px_error("对象 %s 没有方法 %s", px_type_name(obj), name);
    return px_null();
}

// ==================== 全局表 ====================

LXValue px_get_global(const char* name) {
    for (int i = 0; i < g_len; i++) {
        if (strcmp(g_keys[i], name) == 0) return g_vals[i];
    }
    px_error("未定义变量: %s", name);
    return px_null();
}

void px_set_global(const char* name, LXValue v) {
    for (int i = 0; i < g_len; i++) {
        if (strcmp(g_keys[i], name) == 0) { g_vals[i] = v; return; }
    }
    if (g_len >= GLOBAL_CAP) px_error("全局表溢出");
    g_keys[g_len] = xstrdup(name);
    g_vals[g_len] = v;
    g_len++;
}

// ==================== 内置函数 ====================

static LXValue bi_print(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    for (int i = 0; i < nargs; i++) {
        if (i) printf(" ");
        px_print_value(args[i], false);
    }
    printf("\n");
    return px_null();
}

static LXValue bi_len(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("len 需要一个参数");
    return px_int(px_len(args[0]));
}

static LXValue bi_range(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    int64_t start = 0, end = 0, step = 1;
    if (nargs == 1) { end = int_val(args[0]); }
    else if (nargs == 2) { start = int_val(args[0]); end = int_val(args[1]); }
    else if (nargs == 3) { start = int_val(args[0]); end = int_val(args[1]); step = int_val(args[2]); }
    else px_error("range 需要 1-3 个参数");
    if (step == 0) px_error("range step 不能为 0");
    LXValue r = px_list(0);
    if (step > 0) for (int64_t i = start; i < end; i += step) px_list_push(r, px_int(i));
    else for (int64_t i = start; i > end; i += step) px_list_push(r, px_int(i));
    return r;
}

static LXValue bi_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("type 需要一个参数");
    return px_str(px_type_name(args[0]));
}

static LXValue bi_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("str 需要一个参数");
    // 字符串原样返回，数字转字符串
    if (args[0].type == PX_STR) return args[0];
    if (args[0].type == PX_INT || args[0].type == PX_FLOAT) return px_str(fmt_num(args[0]));
    if (args[0].type == PX_BOOL) return px_str(args[0].as.b ? "true" : "false");
    if (args[0].type == PX_NULL) return px_str("null");
    px_error("str 不支持类型 %s", px_type_name(args[0]));
    return px_null();
}

static LXValue bi_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("int 需要一个参数");
    LXValue a = args[0];
    if (a.type == PX_INT) return a;
    if (a.type == PX_FLOAT) return px_int((int64_t)a.as.f);
    if (a.type == PX_BOOL) return px_int(a.as.b ? 1 : 0);
    if (a.type == PX_STR) { return px_int(atoll(a.as.obj->as.str.data)); }
    px_error("int 不支持类型 %s", px_type_name(a));
    return px_null();
}

static LXValue bi_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("float 需要一个参数");
    LXValue a = args[0];
    if (a.type == PX_FLOAT) return a;
    if (a.type == PX_INT) return px_float((double)a.as.i);
    if (a.type == PX_STR) return px_float(atof(a.as.obj->as.str.data));
    px_error("float 不支持类型 %s", px_type_name(a));
    return px_null();
}

static LXValue bi_bool(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bool 需要一个参数");
    return px_bool(px_is_truthy(args[0]));
}

static LXValue bi_assert(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) px_error("assert 需要 1-2 个参数");
    if (!px_is_truthy(args[0])) {
        if (nargs >= 2 && args[1].type == PX_STR) px_error("断言失败: %s", args[1].as.obj->as.str.data);
        else px_error("断言失败");
    }
    return px_null();
}

static LXValue bi_panic(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs >= 1 && args[0].type == PX_STR) px_error("%s", args[0].as.obj->as.str.data);
    px_error("panic");
    return px_null();
}

static LXValue bi_sleep(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) px_error("sleep 需要 1 个参数");
    int64_t ms = int_val(args[0]);
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
    return px_null();
}

static LXValue bi_to_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("to_upper 需要一个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    int len = (int)strlen(s);
    char* d = xmalloc(len + 1);
    for (int i = 0; i < len; i++) {
        char c = s[i];
        d[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    d[len] = 0;
    return px_str(d);
}

static LXValue bi_to_lower(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("to_lower 需要一个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    int len = (int)strlen(s);
    char* d = xmalloc(len + 1);
    for (int i = 0; i < len; i++) {
        char c = s[i];
        d[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    }
    d[len] = 0;
    return px_str(d);
}

static LXValue bi_trim(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("trim 需要一个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\n' || s[len-1] == '\r')) len--;
    return px_str_len(s, len);
}

static LXValue bi_now_ms(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return px_int((int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000L);
}

static LXValue bi_abs(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("abs 需要一个参数");
    LXValue a = args[0];
    if (a.type == PX_INT) return px_int(a.as.i < 0 ? -a.as.i : a.as.i);
    if (a.type == PX_FLOAT) return px_float(fabs(a.as.f));
    px_error("abs 不支持类型 %s", px_type_name(a));
    return px_null();
}

static LXValue bi_min(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) px_error("min 需要至少 1 个参数");
    LXValue m = args[0];
    for (int i = 1; i < nargs; i++) {
        LXValue c = px_lt(args[i], m);
        if (c.as.b) m = args[i];
    }
    return m;
}

static LXValue bi_max(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) px_error("max 需要至少 1 个参数");
    LXValue m = args[0];
    for (int i = 1; i < nargs; i++) {
        LXValue c = px_gt(args[i], m);
        if (c.as.b) m = args[i];
    }
    return m;
}

static LXValue bi_sum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sum 需要 1 个参数");
    LXValue v = args[0];
    if (v.type != PX_LIST) px_error("sum 需要一个列表");
    LXObject* o = v.as.obj;
    LXValue r = px_int(0);
    for (int i = 0; i < o->as.list.len; i++) r = px_add(r, o->as.list.items[i]);
    return r;
}

static LXValue bi_sqrt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sqrt 需要一个参数");
    return px_float(sqrt(num_val(args[0])));
}

// ==================== M5 标准库内置函数 ====================

static LXValue bi_input(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs >= 1 && args[0].type == PX_STR) {
        fwrite(args[0].as.obj->as.str.data, 1, args[0].as.obj->as.str.len, stdout);
        fflush(stdout);
    }
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) return px_str("");
    int len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) len--;
    return px_str_len(buf, len);
}

static LXValue bi_exit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    int code = (nargs >= 1 && args[0].type == PX_INT) ? (int)args[0].as.i : 0;
    exit(code);
    return px_null();
}

// split(s, sep) -> [str]；sep 为空按空白切分
static LXValue bi_split(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2 || args[0].type != PX_STR) px_error("split 需要 1-2 个参数（字符串, [分隔符]）");
    const char* s = args[0].as.obj->as.str.data;
    const char* sep = (nargs >= 2 && args[1].type == PX_STR) ? args[1].as.obj->as.str.data : " ";
    int sep_len = (int)strlen(sep);
    LXValue r = px_list(0);
    if (sep_len == 0) {
        // 按空白切分
        const char* p = s;
        while (*p) {
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            if (!*p) break;
            const char* start = p;
            while (*p && !(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
            px_list_push(r, px_str_len(start, (int)(p - start)));
        }
        return r;
    }
    const char* p = s;
    while (1) {
        const char* hit = strstr(p, sep);
        if (!hit) {
            px_list_push(r, px_str(p));
            break;
        }
        px_list_push(r, px_str_len(p, (int)(hit - p)));
        p = hit + sep_len;
    }
    return r;
}

// join(sep, list) -> str
static LXValue bi_join(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("join 需要 2 个参数（分隔符, 列表）");
    const char* sep = args[0].as.obj->as.str.data;
    int sep_len = (int)strlen(sep);
    if (args[1].type != PX_LIST && args[1].type != PX_TUPLE) px_error("join 第二参数需要 list/tuple");
    LXObject* o = args[1].as.obj;
    int n = (args[1].type == PX_LIST) ? o->as.list.len : o->as.tuple.len;
    // 先计算总长
    size_t total = 1;
    for (int i = 0; i < n; i++) {
        LXValue item = (args[1].type == PX_LIST) ? o->as.list.items[i] : o->as.tuple.items[i];
        char* ts = px_to_string(item);
        total += strlen(ts) + (i ? sep_len : 0);
    }
    char* out = xmalloc(total);
    out[0] = 0;
    for (int i = 0; i < n; i++) {
        if (i) strncat(out, sep, sep_len);
        LXValue item = (args[1].type == PX_LIST) ? o->as.list.items[i] : o->as.tuple.items[i];
        char* ts = px_to_string(item);
        strcat(out, ts);
    }
    return px_str(out);
}

// contains(容器, 元素) -> bool（字符串/列表）
static LXValue bi_contains(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("contains 需要 2 个参数");
    if (args[0].type == PX_STR) {
        if (args[1].type != PX_STR) return px_bool(false);
        return px_bool(strstr(args[0].as.obj->as.str.data, args[1].as.obj->as.str.data) != NULL);
    }
    if (args[0].type == PX_LIST) {
        LXObject* o = args[0].as.obj;
        for (int i = 0; i < o->as.list.len; i++) {
            if (px_eq(o->as.list.items[i], args[1]).as.b) return px_bool(true);
        }
        return px_bool(false);
    }
    px_error("contains 不支持类型 %s", px_type_name(args[0]));
    return px_null();
}

// replace(s, old, new) -> str
static LXValue bi_replace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_STR || args[1].type != PX_STR || args[2].type != PX_STR)
        px_error("replace 需要 3 个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    const char* old = args[1].as.obj->as.str.data;
    const char* nw = args[2].as.obj->as.str.data;
    int old_len = (int)strlen(old);
    if (old_len == 0) return args[0];
    size_t cap = strlen(s) * 2 + strlen(nw) * (strlen(s) / old_len + 1) + 16;
    char* out = xmalloc(cap);
    out[0] = 0;
    const char* p = s;
    while (1) {
        const char* hit = strstr(p, old);
        if (!hit) {
            strcat(out, p);
            break;
        }
        strncat(out, p, hit - p);
        strcat(out, nw);
        p = hit + old_len;
    }
    return px_str(out);
}

static LXValue bi_starts_with(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_STR) px_error("starts_with 需要 2 个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    const char* p = args[1].as.obj->as.str.data;
    size_t lp = strlen(p);
    return px_bool(strncmp(s, p, lp) == 0);
}

static LXValue bi_ends_with(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_STR) px_error("ends_with 需要 2 个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    const char* p = args[1].as.obj->as.str.data;
    size_t ls = strlen(s), lp = strlen(p);
    if (lp > ls) return px_bool(false);
    return px_bool(memcmp(s + ls - lp, p, lp) == 0);
}

static LXValue bi_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("pow 需要 2 个参数");
    if (args[0].type == PX_INT && args[1].type == PX_INT && args[1].as.i >= 0) {
        int64_t r = 1;
        for (int64_t i = 0; i < args[1].as.i; i++) r *= args[0].as.i;
        return px_int(r);
    }
    return px_float(pow(num_val(args[0]), num_val(args[1])));
}

// sorted(list) -> list（按 compare_values 排序，冒泡）
static LXValue bi_sorted(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_LIST) px_error("sorted 需要一个列表");
    LXObject* o = args[0].as.obj;
    LXValue r = px_list(o->as.list.len);
    for (int i = 0; i < o->as.list.len; i++) px_list_push(r, o->as.list.items[i]);
    LXObject* ro = r.as.obj;
    for (int i = 0; i < ro->as.list.len; i++) {
        for (int j = i + 1; j < ro->as.list.len; j++) {
            if (compare_values(ro->as.list.items[j], ro->as.list.items[i]) < 0) {
                LXValue t = ro->as.list.items[i];
                ro->as.list.items[i] = ro->as.list.items[j];
                ro->as.list.items[j] = t;
            }
        }
    }
    return r;
}

static LXValue bi_reversed(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("reversed 需要一个参数");
    if (args[0].type == PX_LIST) {
        LXObject* o = args[0].as.obj;
        LXValue r = px_list(o->as.list.len);
        for (int i = o->as.list.len - 1; i >= 0; i--) px_list_push(r, o->as.list.items[i]);
        return r;
    }
    if (args[0].type == PX_TUPLE) {
        LXObject* o = args[0].as.obj;
        LXValue r = px_list(o->as.tuple.len);
        for (int i = o->as.tuple.len - 1; i >= 0; i--) px_list_push(r, o->as.tuple.items[i]);
        return r;
    }
    if (args[0].type == PX_STR) {
        const char* s = args[0].as.obj->as.str.data;
        int len = (int)strlen(s);
        char* d = xmalloc(len + 1);
        for (int i = 0; i < len; i++) d[i] = s[len - 1 - i];
        d[len] = 0;
        return px_str(d);
    }
    px_error("reversed 不支持类型 %s", px_type_name(args[0]));
    return px_null();
}

// ---- std.io / std.fs ----

static LXValue bi_read_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("read_file 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    FILE* f = fopen(path, "rb");
    if (!f) px_error("io: 读取文件失败 %s", path);
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = xmalloc(sz + 1);
    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = 0;
    fclose(f);
    return px_str_len(buf, (int)rd);
}

static LXValue bi_write_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("write_file 需要 (路径, 内容)");
    const char* path = args[0].as.obj->as.str.data;
    const char* content;
    int clen;
    if (args[1].type == PX_STR) { content = args[1].as.obj->as.str.data; clen = args[1].as.obj->as.str.len; }
    else { content = px_to_string(args[1]); clen = (int)strlen(content); }
    FILE* f = fopen(path, "wb");
    if (!f) px_error("io: 写入文件失败 %s", path);
    fwrite(content, 1, clen, f);
    fclose(f);
    return px_null();
}

static LXValue bi_append_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("append_file 需要 (路径, 内容)");
    const char* path = args[0].as.obj->as.str.data;
    const char* content;
    int clen;
    if (args[1].type == PX_STR) { content = args[1].as.obj->as.str.data; clen = args[1].as.obj->as.str.len; }
    else { content = px_to_string(args[1]); clen = (int)strlen(content); }
    FILE* f = fopen(path, "ab");
    if (!f) px_error("io: 追加写入失败 %s", path);
    fwrite(content, 1, clen, f);
    fclose(f);
    return px_null();
}

// ---- M12 P0：文件随机读写 + fsync（WAL / 增量日志基石）----
// read_at(path, offset, length) → 字符串：从 offset 偏移读 length 字节
// （offset 超出 EOF 返回空串；读不足 length 返回实际读到的字节）
static LXValue bi_read_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_STR) px_error("read_at 需要 (路径, 偏移, 长度)");
    const char* path = args[0].as.obj->as.str.data;
    int64_t offset = int_val(args[1]);
    int length = (int)int_val(args[2]);
    if (length < 0) px_error("read_at 长度不能为负");
    if (length == 0) return px_str("");
    int fd = open(path, O_RDONLY);
    if (fd < 0) px_error("io: 打开文件失败 %s: %s", path, strerror(errno));
    char* buf = xmalloc((size_t)length);
    ssize_t rd = pread(fd, buf, (size_t)length, (off_t)offset);
    close(fd);
    if (rd < 0) { xfree(buf); px_error("io: 随机读失败 %s: %s", path, strerror(errno)); }
    LXValue r = px_str_len(buf, (int)rd);
    xfree(buf);
    return r;
}

// write_at(path, offset, content) → 实际写入字节数
// 文件不存在自动创建；offset 超过 EOF 时中间为空洞（读回 0），WAL 增量写友好
static LXValue bi_write_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_STR) px_error("write_at 需要 (路径, 偏移, 内容)");
    const char* path = args[0].as.obj->as.str.data;
    int64_t offset = int_val(args[1]);
    const char* content;
    int clen;
    if (args[2].type == PX_STR) { content = args[2].as.obj->as.str.data; clen = args[2].as.obj->as.str.len; }
    else { content = px_to_string(args[2]); clen = (int)strlen(content); }
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) px_error("io: 打开文件失败 %s: %s", path, strerror(errno));
    ssize_t wr = pwrite(fd, content, (size_t)clen, (off_t)offset);
    close(fd);
    if (wr < 0) px_error("io: 随机写失败 %s: %s", path, strerror(errno));
    return px_int((int64_t)wr);
}

// file_size(path) → int：文件字节数
static LXValue bi_file_size(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("file_size 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    struct stat st;
    if (stat(path, &st) != 0) px_error("io: 获取文件大小失败 %s: %s", path, strerror(errno));
    return px_int((int64_t)st.st_size);
}

// fsync_file(path) → null：将文件数据刷入磁盘（WAL 落盘保证）
static LXValue bi_fsync_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("fsync_file 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    int fd = open(path, O_RDWR);
    if (fd < 0) fd = open(path, O_RDONLY); // 只读权限文件也允许 fsync（Linux/POSIX）
    if (fd < 0) px_error("io: 打开文件失败 %s: %s", path, strerror(errno));
    int rc = fsync(fd);
    close(fd);
    if (rc != 0) px_error("io: fsync 失败 %s: %s", path, strerror(errno));
    return px_null();
}

// truncate_file(path, size) → null：截断/扩展文件到指定大小（日志轮转、预分配）
static LXValue bi_truncate_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("truncate_file 需要 (路径, 大小)");
    const char* path = args[0].as.obj->as.str.data;
    int64_t size = int_val(args[1]);
    if (truncate(path, (off_t)size) != 0) px_error("io: 截断文件失败 %s: %s", path, strerror(errno));
    return px_null();
}

// ==================== M14 P1：crypto 哈希（签名校验 / 缓存 key / 数据指纹） ====================

// 取任意值的字符串表示（与解释器 to_string 一致：str 原样，其余 str(v)）
static const char* val_cstr(LXValue v) {
    if (v.type == PX_STR) return v.as.obj->as.str.data;
    static char tmp[64];
    snprintf(tmp, sizeof(tmp), "%s", fmt_num(v));
    return tmp;
}

// 跨模块版本（runtime_ws.c 等外部模块用；val_cstr 为 static 不可见）
const char* px_val_cstr(LXValue v) { return val_cstr(v); }

static void bytes_to_hex(const unsigned char* in, size_t len, char* out) {
    static const char HEX[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[i * 2] = HEX[in[i] >> 4];
        out[i * 2 + 1] = HEX[in[i] & 0x0F];
    }
    out[len * 2] = '\0';
}

// ==================== M21 base64（RFC 4648 标准，带 padding） ====================

static const char B64_TBL[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

// base64_encode(data) → str（RFC 4648 标准，带 padding；非字符串自动字符串化）
static LXValue bi_base64_encode(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("base64_encode 需要一个参数");
    const char* data = bdata(args[0]);
    int len = blen(args[0]);
    int olen = ((len + 2) / 3) * 4;
    char* out = xmalloc((size_t)olen + 1);
    int oi = 0, i = 0;
    while (i + 3 <= len) {
        unsigned n = ((unsigned char)data[i] << 16) | ((unsigned char)data[i+1] << 8) | (unsigned char)data[i+2];
        out[oi++] = B64_TBL[(n >> 18) & 63];
        out[oi++] = B64_TBL[(n >> 12) & 63];
        out[oi++] = B64_TBL[(n >> 6) & 63];
        out[oi++] = B64_TBL[n & 63];
        i += 3;
    }
    int rem = len - i;
    if (rem == 1) {
        unsigned n = (unsigned char)data[i] << 16;
        out[oi++] = B64_TBL[(n >> 18) & 63];
        out[oi++] = B64_TBL[(n >> 12) & 63];
        out[oi++] = '='; out[oi++] = '=';
    } else if (rem == 2) {
        unsigned n = ((unsigned char)data[i] << 16) | ((unsigned char)data[i+1] << 8);
        out[oi++] = B64_TBL[(n >> 18) & 63];
        out[oi++] = B64_TBL[(n >> 12) & 63];
        out[oi++] = B64_TBL[(n >> 6) & 63];
        out[oi++] = '=';
    }
    out[oi] = 0;
    LXValue r = px_str_len(out, oi);
    xfree(out);
    return r;
}

// base64_decode(b64) → str 或 null（非法输入返回 null，不抛错）
static LXValue bi_base64_decode(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("base64_decode 需要一个参数");
    const char* s = val_cstr(args[0]);
    int n = (int)strlen(s);
    int pad = 0;
    while (n > 0 && s[n-1] == '=') { pad++; n--; }
    if (pad > 2 || (n % 4) == 1) return px_null();
    int cap = (n / 4) * 3 + 3;
    char* out = xmalloc((size_t)cap + 1);
    int oi = 0, i = 0, q[4], qi = 0;
    while (i < n) {
        int v = b64_val(s[i]);
        if (v < 0) { xfree(out); return px_null(); }
        q[qi++] = v;
        if (qi == 4) {
            unsigned vv = ((unsigned)q[0] << 18) | ((unsigned)q[1] << 12) | ((unsigned)q[2] << 6) | (unsigned)q[3];
            out[oi++] = (char)((vv >> 16) & 0xFF);
            out[oi++] = (char)((vv >> 8) & 0xFF);
            out[oi++] = (char)(vv & 0xFF);
            qi = 0;
        }
        i++;
    }
    if (qi == 2) {
        unsigned vv = ((unsigned)q[0] << 18) | ((unsigned)q[1] << 12);
        out[oi++] = (char)((vv >> 16) & 0xFF);
    } else if (qi == 3) {
        unsigned vv = ((unsigned)q[0] << 18) | ((unsigned)q[1] << 12) | ((unsigned)q[2] << 6);
        out[oi++] = (char)((vv >> 16) & 0xFF);
        out[oi++] = (char)((vv >> 8) & 0xFF);
    } else if (qi != 0) {
        xfree(out); return px_null();
    }
    out[oi] = 0;
    LXValue r = px_str_len(out, oi);
    xfree(out);
    return r;
}

// ==================== M22 P1：位运算 / 二进制数据视图 ====================
// int_to_hex(n, width) → str（固定宽度小写 hex，负数按补码取低 4*width 位）
static LXValue bi_int_to_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("int_to_hex 需要 (n, width) 参数");
    int64_t n = int_val(args[0]);
    int64_t w = int_val(args[1]);
    if (w < 1 || w > 16) px_error("int_to_hex 的 width 必须在 1..16");
    uint64_t mask = (w >= 16) ? ~0ULL : ((1ULL << (4 * (int)w)) - 1);
    uint64_t v = (uint64_t)n & mask;
    char out[40];
    snprintf(out, sizeof(out), "%0*llx", (int)w, (unsigned long long)v);
    return px_str(out);
}

// hex_to_int(hex) → int 或 null（非法 → null；允许空白）
static LXValue bi_hex_to_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("hex_to_int 需要一个参数");
    const char* s = val_cstr(args[0]);
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    if (*s == '\0') return px_null();
    char* end = NULL;
    errno = 0;
    long long v = strtoll(s, &end, 16);
    while (end && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end++;
    if (errno != 0 || end == s || (end && *end != '\0')) return px_null();
    return px_int((int64_t)v);
}

// bytes_to_hex(data) → str（字节 → 小写 hex；非字符串自动字符串化）
static LXValue bi_bytes_to_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bytes_to_hex 需要一个参数");
    const char* data = bdata(args[0]);
    int len = blen(args[0]);
    char* out = xmalloc((size_t)len * 2 + 1);
    bytes_to_hex((const unsigned char*)data, (size_t)len, out);
    LXValue r = px_str(out);
    xfree(out);
    return r;
}

// hex_to_bytes(hex) → bytes 或 null（hex → 原始字节；非法/奇数长度 → null）
static LXValue bi_hex_to_bytes(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("hex_to_bytes 需要一个参数");
    const char* s = val_cstr(args[0]);
    size_t n = strlen(s);
    char* clean = xmalloc(n + 1);
    size_t m = 0;
    for (size_t i = 0; i < n; i++) {
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r') clean[m++] = s[i];
    }
    clean[m] = 0;
    if (m % 2 != 0 || m == 0) { xfree(clean); return px_null(); }
    size_t olen = m / 2;
    char* out = xmalloc(olen + 1);
    for (size_t i = 0; i < m; i += 2) {
        int hi = -1, lo = -1;
        char c = clean[i];
        if (c >= '0' && c <= '9') hi = c - '0';
        else if (c >= 'a' && c <= 'f') hi = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') hi = c - 'A' + 10;
        c = clean[i + 1];
        if (c >= '0' && c <= '9') lo = c - '0';
        else if (c >= 'a' && c <= 'f') lo = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') lo = c - 'A' + 10;
        if (hi < 0 || lo < 0) { xfree(clean); xfree(out); return px_null(); }
        out[i / 2] = (char)((hi << 4) | lo);
    }
    out[olen] = 0;
    xfree(clean);
    LXValue r = px_bytes_len(out, (int)olen);
    xfree(out);
    return r;
}

// bit_count(n) → int（popcount：二进制中 1 的个数）
static LXValue bi_bit_count(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bit_count 需要一个参数");
    uint64_t v = (uint64_t)int_val(args[0]);
    int c = 0;
    while (v) { v &= v - 1; c++; }
    return px_int(c);
}

// bit_length(n) → int（二进制位数；n<=0 → 0）
static LXValue bi_bit_length(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bit_length 需要一个参数");
    int64_t n = int_val(args[0]);
    if (n <= 0) return px_int(0);
    int bits = 0;
    uint64_t v = (uint64_t)n;
    while (v) { bits++; v >>= 1; }
    return px_int(bits);
}

// sha256(data) → 64 字符小写 hex 字符串（mbedtls 实现，与解释器一致）
static LXValue bi_sha256(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sha256 需要一个参数");
    const char* data = val_cstr(args[0]);
    unsigned char digest[32];
    if (mbedtls_sha256((const unsigned char*)data, strlen(data), digest, 0) != 0)
        px_error("sha256 计算失败");
    char hex[65];
    bytes_to_hex(digest, 32, hex);
    return px_str(hex);
}

// ---- XXH64（xxHash, seed=0）----
#define XXH_P1 0x9E3779B185EBCA87ULL
#define XXH_P2 0xC2B2AE3D27D4EB4FULL
#define XXH_P3 0x165667B19E3779F9ULL
#define XXH_P4 0x85EBCA77C2B2AE63ULL
#define XXH_P5 0x27D4EB2F165667C5ULL

static uint64_t xxh_rotl(uint64_t x, int r) { return (x << r) | (x >> (64 - r)); }

static uint64_t xxh_round(uint64_t acc, uint64_t input) {
    acc += input * XXH_P2;
    acc = xxh_rotl(acc, 31);
    acc *= XXH_P1;
    return acc;
}

static uint64_t xxh_merge(uint64_t acc, uint64_t val) {
    acc ^= xxh_round(0, val);
    acc = acc * XXH_P1 + XXH_P4;
    return acc;
}

static uint64_t rd64(const unsigned char* p) {
    uint64_t v = 0;
    for (int i = 7; i >= 0; i--) v = (v << 8) | p[i];
    return v;
}

static uint64_t xxh64(const unsigned char* data, size_t len) {
    size_t p = 0;
    uint64_t h;
    if (len >= 32) {
        uint64_t v1 = XXH_P1 + XXH_P2, v2 = XXH_P2, v3 = 0, v4 = 0 - XXH_P1;
        while (p + 32 <= len) {
            v1 = xxh_round(v1, rd64(data + p));
            v2 = xxh_round(v2, rd64(data + p + 8));
            v3 = xxh_round(v3, rd64(data + p + 16));
            v4 = xxh_round(v4, rd64(data + p + 24));
            p += 32;
        }
        h = xxh_rotl(v1, 1) + xxh_rotl(v2, 7) + xxh_rotl(v3, 12) + xxh_rotl(v4, 18);
        h = xxh_merge(h, v1);
        h = xxh_merge(h, v2);
        h = xxh_merge(h, v3);
        h = xxh_merge(h, v4);
    } else {
        h = XXH_P5;
    }
    h += len;
    while (p + 8 <= len) {
        uint64_t k = xxh_round(0, rd64(data + p));
        h ^= k;
        h = xxh_rotl(h, 27) * XXH_P1 + XXH_P4;
        p += 8;
    }
    if (p + 4 <= len) {
        uint64_t k = 0;
        for (int i = 3; i >= 0; i--) k = (k << 8) | data[p + i];
        h ^= k * XXH_P1;
        h = xxh_rotl(h, 23) * XXH_P2 + XXH_P3;
        p += 4;
    }
    while (p < len) {
        h ^= (uint64_t)data[p] * XXH_P5;
        h = xxh_rotl(h, 11) * XXH_P1;
        p++;
    }
    h ^= h >> 33;
    h *= XXH_P2;
    h ^= h >> 29;
    h *= XXH_P3;
    h ^= h >> 32;
    return h;
}

// xxhash(data) → int（XXH64, seed=0；高速指纹/取模分片）
static LXValue bi_xxhash(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("xxhash 需要一个参数");
    const char* data = val_cstr(args[0]);
    return px_int((int64_t)xxh64((const unsigned char*)data, strlen(data)));
}


// ==================== M15 P1：正则表达式（与解释器 regex.rs 同一回溯算法） ====================
// 候选列表法：rmatch 返回节点从 pos 起的所有 (end, groups) 候选，贪心优先（次数多在前）。
// 支持：字面量/./字符类/\d\w\s(及取反)/量词 * + ? {n,m}/锚点 ^$/(捕获组 9 个)/交替 |/转义

enum { RN_CHAR = 0, RN_ANY, RN_CLASS, RN_SEQ, RN_ALT, RN_REP, RN_GROUP, RN_START, RN_END };
#define RG_N 10

typedef struct RNode {
    int type;
    unsigned char ch;        // RN_CHAR
    unsigned char* cls_lo;   // RN_CLASS
    unsigned char* cls_hi;
    int ncls;
    int neg;
    struct RNode** kids;     // RN_SEQ / RN_ALT
    int nkids;
    struct RNode* child;     // RN_REP / RN_GROUP
    int min, max;            // RN_REP
    int gidx;                // RN_GROUP
} RNode;

typedef struct { int end; int64_t groups[RG_N]; } RCand;
typedef struct { RCand* items; int len, cap; } RCandList;

static RCandList rcand_new(void) { RCandList l = {0, 0, 0}; return l; }
static void rcand_add(RCandList* l, int end, const int64_t* g) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->items = xrealloc(l->items, sizeof(RCand) * l->cap); }
    l->items[l->len].end = end;
    if (g) memcpy(l->items[l->len].groups, g, sizeof(int64_t) * RG_N);
    l->len++;
}
static void rcand_free(RCandList* l) { if (l->items) { xfree(l->items); l->items = NULL; } l->len = l->cap = 0; }
static void rcand_extend(RCandList* dst, RCandList* src) {
    for (int i = 0; i < src->len; i++) rcand_add(dst, src->items[i].end, src->items[i].groups);
}

// ---- 解析 ----
typedef struct { const unsigned char* b; int len; int pos; int groups; char err[160]; } RParser;

static int rp_peek(RParser* p) { return p->pos < p->len ? p->b[p->pos] : -1; }
static RNode* rp_new(int type) { RNode* n = xmalloc(sizeof(RNode)); memset(n, 0, sizeof(RNode)); n->type = type; return n; }
static void rp_add_kid(RNode* n, RNode* k) {
    n->kids = xrealloc(n->kids, sizeof(RNode*) * (n->nkids + 1));
    n->kids[n->nkids++] = k;
}
static void rp_free(RNode* n) {
    if (!n) return;
    if (n->kids) { for (int i = 0; i < n->nkids; i++) rp_free(n->kids[i]); xfree(n->kids); }
    if (n->child) rp_free(n->child);
    if (n->cls_lo) xfree(n->cls_lo);
    if (n->cls_hi) xfree(n->cls_hi);
    xfree(n);
}

static RNode* rp_parse_alt(RParser* p);
static RNode* rp_parse_seq(RParser* p);
static RNode* rp_parse_repeat(RParser* p);
static RNode* rp_parse_atom(RParser* p);
static RNode* rp_parse_class(RParser* p);
static RNode* rp_parse_escape(RParser* p);
static int rp_parse_class_elem(RParser* p, unsigned char* lo, unsigned char* hi);

static int rp_parse_class_elem(RParser* p, unsigned char* lo, unsigned char* hi) {
    int c = rp_peek(p);
    if (c < 0) { snprintf(p->err, sizeof(p->err), "字符类提前结束"); return -1; }
    if (c == '\\') {
        p->pos++;
        int e = rp_peek(p);
        if (e < 0) { snprintf(p->err, sizeof(p->err), "转义提前结束"); return -1; }
        p->pos++;
        switch (e) {
            case 'd': lo[0] = '0'; hi[0] = '9'; return 1;
            case 'w': lo[0]='0';hi[0]='9'; lo[1]='A';hi[1]='Z'; lo[2]='a';hi[2]='z'; lo[3]='_';hi[3]='_'; return 4;
            case 's': lo[0]=' ';hi[0]=' '; lo[1]='\t';hi[1]='\t'; lo[2]='\n';hi[2]='\n'; lo[3]='\r';hi[3]='\r'; lo[4]=0x0b;hi[4]=0x0b; lo[5]=0x0c;hi[5]=0x0c; return 6;
            default: {
                unsigned char ch = (unsigned char)e;
                switch (e) { case 'n': ch = '\n'; break; case 't': ch = '\t'; break; case 'r': ch = '\r'; break; case '0': ch = 0; break; case 'f': ch = 0x0c; break; case 'v': ch = 0x0b; break; }
                lo[0] = ch; hi[0] = ch; return 1;
            }
        }
    }
    p->pos++;
    lo[0] = (unsigned char)c; hi[0] = (unsigned char)c;
    return 1;
}

static RNode* rp_parse_class(RParser* p) {
    p->pos++; // '['
    int neg = 0;
    if (rp_peek(p) == '^') { neg = 1; p->pos++; }
    unsigned char lo[256], hi[256];
    int ncls = 0, first = 1;
    for (;;) {
        int c = rp_peek(p);
        if (c < 0) { snprintf(p->err, sizeof(p->err), "字符类缺少 ]"); return NULL; }
        if (c == ']' && !first) { p->pos++; break; }
        first = 0;
        unsigned char elo[10], ehi[10];
        int n = rp_parse_class_elem(p, elo, ehi);
        if (n < 0) return NULL;
        if (n == 1 && rp_peek(p) == '-' && p->pos + 1 < p->len && p->b[p->pos + 1] != ']') {
            p->pos++; // '-'
            unsigned char elo2[10], ehi2[10];
            int n2 = rp_parse_class_elem(p, elo2, ehi2);
            if (n2 < 0) return NULL;
            if (n2 != 1) { snprintf(p->err, sizeof(p->err), "字符范围右端不能是转义类"); return NULL; }
            if (ehi2[0] < elo[0]) { snprintf(p->err, sizeof(p->err), "字符范围 hi < lo"); return NULL; }
            if (ncls < 256) { lo[ncls] = elo[0]; hi[ncls] = ehi2[0]; ncls++; }
        } else {
            for (int i = 0; i < n && ncls < 256; i++) { lo[ncls] = elo[i]; hi[ncls] = ehi[i]; ncls++; }
        }
    }
    if (ncls == 0) { snprintf(p->err, sizeof(p->err), "空字符类"); return NULL; }
    RNode* nd = rp_new(RN_CLASS);
    nd->cls_lo = xmalloc(ncls);
    nd->cls_hi = xmalloc(ncls);
    memcpy(nd->cls_lo, lo, ncls);
    memcpy(nd->cls_hi, hi, ncls);
    nd->ncls = ncls;
    nd->neg = neg;
    return nd;
}

static RNode* rp_parse_escape(RParser* p) {
    p->pos++; // '\\'
    int e = rp_peek(p);
    if (e < 0) { snprintf(p->err, sizeof(p->err), "转义字符缺失"); return NULL; }
    p->pos++;
    switch (e) {
        case 'd': case 'D': case 'w': case 'W': case 's': case 'S': {
            RNode* n = rp_new(RN_CLASS);
            unsigned char lo[10], hi[10];
            int cnt = 0, neg = 0;
            switch (e) {
                case 'd': lo[0]='0'; hi[0]='9'; cnt = 1; break;
                case 'D': lo[0]='0'; hi[0]='9'; cnt = 1; neg = 1; break;
                case 'w': lo[0]='0';hi[0]='9'; lo[1]='A';hi[1]='Z'; lo[2]='a';hi[2]='z'; lo[3]='_';hi[3]='_'; cnt = 4; break;
                case 'W': lo[0]='0';hi[0]='9'; lo[1]='A';hi[1]='Z'; lo[2]='a';hi[2]='z'; lo[3]='_';hi[3]='_'; cnt = 4; neg = 1; break;
                case 's': lo[0]=' ';hi[0]=' '; lo[1]='\t';hi[1]='\t'; lo[2]='\n';hi[2]='\n'; lo[3]='\r';hi[3]='\r'; lo[4]=0x0b;hi[4]=0x0b; lo[5]=0x0c;hi[5]=0x0c; cnt = 6; break;
                case 'S': lo[0]=' ';hi[0]=' '; lo[1]='\t';hi[1]='\t'; lo[2]='\n';hi[2]='\n'; lo[3]='\r';hi[3]='\r'; lo[4]=0x0b;hi[4]=0x0b; lo[5]=0x0c;hi[5]=0x0c; cnt = 6; neg = 1; break;
                default: break;
            }
            n->cls_lo = xmalloc(cnt);
            n->cls_hi = xmalloc(cnt);
            memcpy(n->cls_lo, lo, cnt);
            memcpy(n->cls_hi, hi, cnt);
            n->ncls = cnt;
            n->neg = neg;
            return n;
        }
        case 'n': { RNode* n = rp_new(RN_CHAR); n->ch = '\n'; return n; }
        case 't': { RNode* n = rp_new(RN_CHAR); n->ch = '\t'; return n; }
        case 'r': { RNode* n = rp_new(RN_CHAR); n->ch = '\r'; return n; }
        case '0': { RNode* n = rp_new(RN_CHAR); n->ch = 0; return n; }
        case 'f': { RNode* n = rp_new(RN_CHAR); n->ch = 0x0c; return n; }
        case 'v': { RNode* n = rp_new(RN_CHAR); n->ch = 0x0b; return n; }
        case '.': case '*': case '+': case '?': case '(': case ')': case '[': case ']':
        case '{': case '}': case '|': case '^': case '$': case '\\': case '/': {
            RNode* n = rp_new(RN_CHAR); n->ch = (unsigned char)e; return n;
        }
        default: snprintf(p->err, sizeof(p->err), "未知转义 \\%c", e); return NULL;
    }
}

static RNode* rp_parse_atom(RParser* p) {
    int c = rp_peek(p);
    if (c < 0) { snprintf(p->err, sizeof(p->err), "意外的结尾"); return NULL; }
    switch (c) {
        case '(': {
            p->pos++;
            RNode* node = rp_parse_alt(p);
            if (!node) return NULL;
            if (rp_peek(p) != ')') { rp_free(node); snprintf(p->err, sizeof(p->err), "缺少 )"); return NULL; }
            p->pos++;
            if (p->groups >= 9) { rp_free(node); snprintf(p->err, sizeof(p->err), "捕获组最多 9 个"); return NULL; }
            p->groups++;
            RNode* g = rp_new(RN_GROUP);
            g->child = node;
            g->gidx = p->groups;
            return g;
        }
        case '[': return rp_parse_class(p);
        case '.': p->pos++; return rp_new(RN_ANY);
        case '^': p->pos++; return rp_new(RN_START);
        case '$': p->pos++; return rp_new(RN_END);
        case '\\': return rp_parse_escape(p);
        case ')': snprintf(p->err, sizeof(p->err), "意外的 )"); return NULL;
        default: p->pos++; { RNode* n = rp_new(RN_CHAR); n->ch = (unsigned char)c; return n; }
    }
}

static RNode* rp_parse_repeat(RParser* p) {
    RNode* atom = rp_parse_atom(p);
    if (!atom) return NULL;
    int c = rp_peek(p);
    if (c == '*') { p->pos++; RNode* r = rp_new(RN_REP); r->child = atom; r->min = 0; r->max = -1; return r; }
    if (c == '+') { p->pos++; RNode* r = rp_new(RN_REP); r->child = atom; r->min = 1; r->max = -1; return r; }
    if (c == '?') { p->pos++; RNode* r = rp_new(RN_REP); r->child = atom; r->min = 0; r->max = 1; return r; }
    if (c == '{') {
        int save = p->pos;
        p->pos++;
        int min = 0, got = 0;
        while (rp_peek(p) >= '0' && rp_peek(p) <= '9') { min = min * 10 + (rp_peek(p) - '0'); p->pos++; got = 1; }
        if (!got) { p->pos = save; return atom; }
        int max = -1;
        c = rp_peek(p);
        if (c == '}') { p->pos++; max = min; }
        else if (c == ',') {
            p->pos++;
            if (rp_peek(p) == '}') { p->pos++; max = -1; }
            else {
                int m2 = 0, got2 = 0;
                while (rp_peek(p) >= '0' && rp_peek(p) <= '9') { m2 = m2 * 10 + (rp_peek(p) - '0'); p->pos++; got2 = 1; }
                if (!got2 || rp_peek(p) != '}') { p->pos = save; return atom; }
                p->pos++;
                max = m2;
            }
        } else { p->pos = save; return atom; }
        if (max != -1 && max < min) { snprintf(p->err, sizeof(p->err), "{n,m} 中 m 不能小于 n"); rp_free(atom); return NULL; }
        RNode* r = rp_new(RN_REP);
        r->child = atom;
        r->min = min;
        r->max = max;
        return r;
    }
    return atom;
}

static RNode* rp_parse_seq(RParser* p) {
    RNode* n = rp_new(RN_SEQ);
    for (;;) {
        int c = rp_peek(p);
        if (c < 0 || c == '|' || c == ')') break;
        RNode* r = rp_parse_repeat(p);
        if (!r) { rp_free(n); return NULL; }
        rp_add_kid(n, r);
    }
    if (n->nkids == 1) { RNode* k = n->kids[0]; xfree(n->kids); xfree(n); return k; }
    return n;
}

static RNode* rp_parse_alt(RParser* p) {
    RNode* n = rp_new(RN_ALT);
    RNode* s = rp_parse_seq(p);
    if (!s) { rp_free(n); return NULL; }
    rp_add_kid(n, s);
    while (rp_peek(p) == '|') {
        p->pos++;
        s = rp_parse_seq(p);
        if (!s) { rp_free(n); return NULL; }
        rp_add_kid(n, s);
    }
    if (n->nkids == 1) { RNode* k = n->kids[0]; xfree(n->kids); xfree(n); return k; }
    return n;
}

static RNode* rcompile(const char* pat, char* errbuf, int errsz) {
    RParser p;
    memset(&p, 0, sizeof(p));
    p.b = (const unsigned char*)pat;
    p.len = (int)strlen(pat);
    RNode* root = rp_parse_alt(&p);
    if (root && p.pos != p.len) {
        snprintf(errbuf, errsz, "正则语法错误: 位置 %d 处意外的字符", p.pos);
        rp_free(root);
        return NULL;
    }
    if (!root && errbuf) snprintf(errbuf, errsz, "%s", p.err);
    return root;
}

// ---- 匹配（候选列表回溯，与 Rust 端同序） ----
static RCandList rmatch(RNode* n, const unsigned char* text, int len, int pos, const int64_t* groups);

static RCandList rmatch(RNode* n, const unsigned char* text, int len, int pos, const int64_t* groups) {
    RCandList out = rcand_new();
    switch (n->type) {
        case RN_CHAR:
            if (pos < len && text[pos] == n->ch) rcand_add(&out, pos + 1, groups);
            break;
        case RN_ANY:
            if (pos < len && text[pos] != '\n') rcand_add(&out, pos + 1, groups);
            break;
        case RN_CLASS:
            if (pos < len) {
                unsigned char ch = text[pos];
                int hit = 0;
                for (int i = 0; i < n->ncls; i++) if (ch >= n->cls_lo[i] && ch <= n->cls_hi[i]) { hit = 1; break; }
                if (hit != n->neg) rcand_add(&out, pos + 1, groups);
            }
            break;
        case RN_START:
            if (pos == 0) rcand_add(&out, pos, groups);
            break;
        case RN_END:
            if (pos == len) rcand_add(&out, pos, groups);
            break;
        case RN_ALT:
            for (int i = 0; i < n->nkids; i++) {
                RCandList sub = rmatch(n->kids[i], text, len, pos, groups);
                rcand_extend(&out, &sub);
                rcand_free(&sub);
            }
            break;
        case RN_GROUP: {
            int64_t g[RG_N];
            memcpy(g, groups, sizeof(g));
            g[n->gidx] = ((int64_t)pos << 32) | (unsigned)pos;
            RCandList sub = rmatch(n->child, text, len, pos, g);
            for (int i = 0; i < sub.len; i++) {
                int64_t gg[RG_N];
                memcpy(gg, sub.items[i].groups, sizeof(gg));
                gg[n->gidx] = ((int64_t)pos << 32) | (unsigned)sub.items[i].end;
                rcand_add(&out, sub.items[i].end, gg);
            }
            rcand_free(&sub);
            break;
        }
        case RN_REP: {
            // BFS：all 记录 (end, groups, level)；贪心优先 = level 降序
            typedef struct { int end; int64_t groups[RG_N]; int level; } RRepCand;
            RRepCand* all = NULL;
            int all_len = 0, all_cap = 0;
            RRepCand* frontier = NULL;
            int flen = 0, fcap = 0;
            // level 0
            all = xrealloc(all, sizeof(RRepCand) * (all_len + 1));
            all[all_len].end = pos; memcpy(all[all_len].groups, groups, sizeof(int64_t) * RG_N); all[all_len].level = 0; all_len++;
            frontier = xrealloc(frontier, sizeof(RRepCand) * (flen + 1));
            frontier[flen].end = pos; memcpy(frontier[flen].groups, groups, sizeof(int64_t) * RG_N); flen++;
            int level = 0;
            for (;;) {
                if (n->max >= 0 && level >= n->max) break;
                RRepCand* next = NULL;
                int nlen = 0;
                for (int fi = 0; fi < flen; fi++) {
                    RCandList sub = rmatch(n->child, text, len, frontier[fi].end, frontier[fi].groups);
                    for (int si = 0; si < sub.len; si++) {
                        if (sub.items[si].end > frontier[fi].end) {
                            int dup = 0;
                            for (int ni = 0; ni < nlen; ni++) {
                                if (next[ni].end == sub.items[si].end &&
                                    memcmp(next[ni].groups, sub.items[si].groups, sizeof(int64_t) * RG_N) == 0) { dup = 1; break; }
                            }
                            if (!dup) {
                                next = xrealloc(next, sizeof(RRepCand) * (nlen + 1));
                                next[nlen].end = sub.items[si].end;
                                memcpy(next[nlen].groups, sub.items[si].groups, sizeof(int64_t) * RG_N);
                                nlen++;
                            }
                        }
                    }
                    rcand_free(&sub);
                }
                if (nlen == 0) { if (next) xfree(next); break; }
                level++;
                for (int i = 0; i < nlen; i++) {
                    all = xrealloc(all, sizeof(RRepCand) * (all_len + 1));
                    all[all_len].end = next[i].end;
                    memcpy(all[all_len].groups, next[i].groups, sizeof(int64_t) * RG_N);
                    all[all_len].level = level;
                    all_len++;
                }
                if (frontier) xfree(frontier);
                frontier = next;
                flen = nlen;
            }
            // 插入排序：level 降序（贪心优先，稳定）
            for (int i = 1; i < all_len; i++) {
                RRepCand key = all[i];
                int j = i - 1;
                while (j >= 0 && all[j].level < key.level) { all[j + 1] = all[j]; j--; }
                all[j + 1] = key;
            }
            for (int i = 0; i < all_len; i++) {
                if (all[i].level >= n->min) rcand_add(&out, all[i].end, all[i].groups);
            }
            if (all) xfree(all);
            if (frontier) xfree(frontier);
            break;
        }
        case RN_SEQ: {
            RCandList cur = rcand_new();
            rcand_add(&cur, pos, groups);
            for (int i = 0; i < n->nkids; i++) {
                RCandList next = rcand_new();
                for (int j = 0; j < cur.len; j++) {
                    RCandList sub = rmatch(n->kids[i], text, len, cur.items[j].end, cur.items[j].groups);
                    rcand_extend(&next, &sub);
                    rcand_free(&sub);
                }
                rcand_free(&cur);
                cur = next;
                if (cur.len == 0) break;
            }
            return cur;
        }
    }
    return out;
}

// ---- 顶层搜索 ----
static int rsearch_from(RNode* root, const unsigned char* text, int len, int start, int* out_s, int* out_e, int64_t* out_g) {
    int64_t g0[RG_N];
    for (int i = 0; i < RG_N; i++) g0[i] = -1;
    for (int s = start; s <= len; s++) {
        RCandList l = rmatch(root, text, len, s, g0);
        if (l.len > 0) {
            *out_s = s;
            *out_e = l.items[0].end;
            memcpy(out_g, l.items[0].groups, sizeof(int64_t) * RG_N);
            out_g[0] = ((int64_t)s << 32) | (unsigned)(*out_e);
            rcand_free(&l);
            return 1;
        }
        rcand_free(&l);
    }
    return 0;
}

static int rfullmatch(RNode* root, const unsigned char* text, int len, int64_t* out_g) {
    int64_t g0[RG_N];
    for (int i = 0; i < RG_N; i++) g0[i] = -1;
    RCandList l = rmatch(root, text, len, 0, g0);
    int found = 0;
    for (int i = 0; i < l.len; i++) {
        if (l.items[i].end == len) {
            memcpy(out_g, l.items[i].groups, sizeof(int64_t) * RG_N);
            out_g[0] = 0; // start=0
            out_g[0] = ((int64_t)0 << 32) | (unsigned)len;
            found = 1;
            break;
        }
    }
    rcand_free(&l);
    return found;
}

// ---- 字符串构建 ----
typedef struct { char* data; int len, cap; } RStrBuf;
static void rsb_append(RStrBuf* b, const char* s, int n) {
    if (n <= 0) return;
    if (b->len + n > b->cap) {
        int nc = b->cap ? b->cap * 2 : 64;
        while (nc < b->len + n) nc *= 2;
        b->data = xrealloc(b->data, nc);
        b->cap = nc;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
}

static void r_expand_repl(const char* repl, const int64_t* g, const unsigned char* text, int tlen, RStrBuf* out) {
    int rl = (int)strlen(repl);
    for (int i = 0; i < rl; i++) {
        if (repl[i] == '$' && i + 1 < rl) {
            char c = repl[i + 1];
            if (c == '$') { rsb_append(out, "$", 1); i++; continue; }
            if (c >= '0' && c <= '9') {
                int idx = c - '0';
                if (idx < RG_N && g[idx] != -1) {
                    int s = (int)(g[idx] >> 32);
                    int e = (int)(g[idx] & 0xffffffff);
                    if (s >= 0 && s <= e && e <= tlen) rsb_append(out, (const char*)text + s, e - s);
                }
                i++;
                continue;
            }
        }
        rsb_append(out, repl + i, 1);
    }
}

static void r_replace(RNode* root, const unsigned char* text, int len, const char* repl, RStrBuf* out) {
    int pos = 0;
    while (pos <= len) {
        int s, e;
        int64_t g[RG_N];
        if (!rsearch_from(root, text, len, pos, &s, &e, g)) break;
        rsb_append(out, (const char*)text + pos, s - pos);
        g[0] = ((int64_t)s << 32) | (unsigned)e;
        r_expand_repl(repl, g, text, len, out);
        pos = (e == s) ? s + 1 : e;
        if (e == s && pos <= len) rsb_append(out, (const char*)text + s, 1);
    }
    rsb_append(out, (const char*)text + pos, len - pos);
}

static void r_split(RNode* root, const unsigned char* text, int len, LXValue list) {
    int pos = 0;
    while (pos <= len) {
        int s = -1, e = -1;
        for (int start = pos; start <= len; start++) {
            int64_t g0[RG_N];
            for (int i = 0; i < RG_N; i++) g0[i] = -1;
            RCandList l = rmatch(root, text, len, start, g0);
            if (l.len > 0 && l.items[0].end > start) { s = start; e = l.items[0].end; rcand_free(&l); break; }
            rcand_free(&l);
        }
        if (s < 0) break;
        px_list_push(list, px_str_len((const char*)text + pos, s - pos));
        pos = e;
    }
    px_list_push(list, px_str_len((const char*)text + pos, len - pos));
}

// ---- 内置函数 ----
static LXValue bi_regex_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("regex_find 需要 2 个参数: (pattern, text)");
    const char* pat = val_cstr(args[0]);
    const char* text = val_cstr(args[1]);
    int tlen = (int)strlen(text);
    char err[160];
    err[0] = 0;
    RNode* root = rcompile(pat, err, sizeof(err));
    if (!root) px_error("regex: %s", err);
    int s, e;
    int64_t g[RG_N];
    int found = rsearch_from(root, (const unsigned char*)text, tlen, 0, &s, &e, g);
    rp_free(root);
    if (found) return px_str_len(text + s, e - s);
    return px_null();
}

static LXValue bi_regex_match(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("regex_match 需要 2 个参数: (pattern, text)");
    const char* pat = val_cstr(args[0]);
    const char* text = val_cstr(args[1]);
    char err[160];
    err[0] = 0;
    RNode* root = rcompile(pat, err, sizeof(err));
    if (!root) px_error("regex: %s", err);
    int64_t g[RG_N];
    int found = rfullmatch(root, (const unsigned char*)text, (int)strlen(text), g);
    rp_free(root);
    return px_bool(found != 0);
}

static LXValue bi_regex_search(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("regex_search 需要 2 个参数: (pattern, text)");
    const char* pat = val_cstr(args[0]);
    const char* text = val_cstr(args[1]);
    int tlen = (int)strlen(text);
    char err[160];
    err[0] = 0;
    RNode* root = rcompile(pat, err, sizeof(err));
    if (!root) px_error("regex: %s", err);
    int s, e;
    int64_t g[RG_N];
    int found = rsearch_from(root, (const unsigned char*)text, tlen, 0, &s, &e, g);
    rp_free(root);
    if (!found) return px_null();
    LXValue d = px_dict();
    px_dict_set(d, "match", px_str_len(text + s, e - s));
    px_dict_set(d, "start", px_int(s));
    px_dict_set(d, "end", px_int(e));
    LXValue gl = px_list(0);
    for (int i = 1; i < RG_N; i++) {
        if (g[i] != -1) {
            int gs = (int)(g[i] >> 32), ge = (int)(g[i] & 0xffffffff);
            px_list_push(gl, px_str_len(text + gs, ge - gs));
        } else {
            px_list_push(gl, px_null());
        }
    }
    px_dict_set(d, "groups", gl);
    return d;
}

static LXValue bi_regex_find_all(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("regex_find_all 需要 2 个参数: (pattern, text)");
    const char* pat = val_cstr(args[0]);
    const char* text = val_cstr(args[1]);
    int tlen = (int)strlen(text);
    char err[160];
    err[0] = 0;
    RNode* root = rcompile(pat, err, sizeof(err));
    if (!root) px_error("regex: %s", err);
    LXValue r = px_list(0);
    int pos = 0;
    while (pos <= tlen) {
        int s, e;
        int64_t g[RG_N];
        if (!rsearch_from(root, (const unsigned char*)text, tlen, pos, &s, &e, g)) break;
        px_list_push(r, px_str_len(text + s, e - s));
        pos = (e == s) ? s + 1 : e;
    }
    rp_free(root);
    return r;
}

static LXValue bi_regex_replace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("regex_replace 需要 3 个参数: (pattern, text, repl)");
    const char* pat = val_cstr(args[0]);
    const char* text = val_cstr(args[1]);
    const char* repl = val_cstr(args[2]);
    int tlen = (int)strlen(text);
    char err[160];
    err[0] = 0;
    RNode* root = rcompile(pat, err, sizeof(err));
    if (!root) px_error("regex: %s", err);
    RStrBuf out = {0, 0, 0};
    r_replace(root, (const unsigned char*)text, tlen, repl, &out);
    rp_free(root);
    LXValue v = px_str_len(out.data ? out.data : "", out.len);
    if (out.data) xfree(out.data);
    return v;
}

static LXValue bi_regex_split(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("regex_split 需要 2 个参数: (pattern, text)");
    const char* pat = val_cstr(args[0]);
    const char* text = val_cstr(args[1]);
    int tlen = (int)strlen(text);
    char err[160];
    err[0] = 0;
    RNode* root = rcompile(pat, err, sizeof(err));
    if (!root) px_error("regex: %s", err);
    LXValue r = px_list(0);
    r_split(root, (const unsigned char*)text, tlen, r);
    rp_free(root);
    return r;
}

static LXValue bi_exists(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("exists 需要一个路径参数");
    struct stat st;
    return px_bool(stat(args[0].as.obj->as.str.data, &st) == 0);
}

static LXValue bi_list_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("list_dir 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    DIR* d = opendir(path);
    if (!d) px_error("fs: 读取目录失败 %s", path);
    LXValue r = px_list(0);
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        px_list_push(r, px_str(e->d_name));
    }
    closedir(d);
    // 简单排序
    LXObject* ro = r.as.obj;
    for (int i = 0; i < ro->as.list.len; i++) {
        for (int j = i + 1; j < ro->as.list.len; j++) {
            if (compare_values(ro->as.list.items[j], ro->as.list.items[i]) < 0) {
                LXValue t = ro->as.list.items[i];
                ro->as.list.items[i] = ro->as.list.items[j];
                ro->as.list.items[j] = t;
            }
        }
    }
    return r;
}

static LXValue bi_mkdir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("mkdir 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    // 递归创建
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    int len = (int)strlen(tmp);
    for (int i = 1; i < len; i++) {
        if (tmp[i] == '/') {
            tmp[i] = 0;
            mkdir(tmp, 0755);
            tmp[i] = '/';
        }
    }
    mkdir(tmp, 0755);
    return px_null();
}

static LXValue bi_remove(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("remove 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    if (remove(path) != 0 && rmdir(path) != 0) px_error("fs: 删除失败 %s", path);
    return px_null();
}

// ---- std.json ----

typedef struct {
    const char* p;
} JsonCtx;

static void json_ws(JsonCtx* j) {
    while (*j->p == ' ' || *j->p == '\t' || *j->p == '\n' || *j->p == '\r') j->p++;
}
static char* json_str_raw(JsonCtx* j) {
    // 前置：已消费 '"'；返回 malloc 字符串（已解码）
    char* out = xmalloc(strlen(j->p) + 1);
    int k = 0;
    while (*j->p && *j->p != '"') {
        if (*j->p == '\\') {
            j->p++;
            switch (*j->p) {
                case 'n': out[k++] = '\n'; j->p++; break;
                case 't': out[k++] = '\t'; j->p++; break;
                case 'r': out[k++] = '\r'; j->p++; break;
                case 'b': out[k++] = '\b'; j->p++; break;
                case 'f': out[k++] = '\f'; j->p++; break;
                case '"': out[k++] = '"'; j->p++; break;
                case '\\': out[k++] = '\\'; j->p++; break;
                case '/': out[k++] = '/'; j->p++; break;
                case 'u': {
                    char hex[5] = {0};
                    for (int i = 0; i < 4 && j->p[1+i]; i++) hex[i] = j->p[1+i];
                    j->p += 5;
                    unsigned code = (unsigned)strtoul(hex, NULL, 16);
                    if (code < 0x80) out[k++] = (char)code;
                    else if (code < 0x800) {
                        out[k++] = (char)(0xC0 | (code >> 6));
                        out[k++] = (char)(0x80 | (code & 0x3F));
                    } else {
                        out[k++] = (char)(0xE0 | (code >> 12));
                        out[k++] = (char)(0x80 | ((code >> 6) & 0x3F));
                        out[k++] = (char)(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default: out[k++] = *j->p; j->p++; break;
            }
        } else {
            out[k++] = *j->p;
            j->p++;
        }
    }
    if (*j->p == '"') j->p++;
    out[k] = 0;
    return out;
}

static LXValue json_parse_value(JsonCtx* j);

static LXValue json_parse_value(JsonCtx* j) {
    json_ws(j);
    if (*j->p == '{') {
        j->p++;
        LXValue d = px_dict();
        json_ws(j);
        if (*j->p == '}') { j->p++; return d; }
        while (1) {
            json_ws(j);
            if (*j->p != '"') px_error("json: 期望对象键");
            j->p++;
            char* key = json_str_raw(j);
            json_ws(j);
            if (*j->p != ':') px_error("json: 期望 ':'");
            j->p++;
            LXValue v = json_parse_value(j);
            px_dict_set(d, key, v);
            xfree(key);
            json_ws(j);
            if (*j->p == ',') { j->p++; continue; }
            if (*j->p == '}') { j->p++; break; }
            px_error("json: 对象解析失败");
        }
        return d;
    }
    if (*j->p == '[') {
        j->p++;
        LXValue a = px_list(0);
        json_ws(j);
        if (*j->p == ']') { j->p++; return a; }
        while (1) {
            LXValue v = json_parse_value(j);
            px_list_push(a, v);
            json_ws(j);
            if (*j->p == ',') { j->p++; continue; }
            if (*j->p == ']') { j->p++; break; }
            px_error("json: 数组解析失败");
        }
        return a;
    }
    if (*j->p == '"') {
        j->p++;
        char* s = json_str_raw(j);
        LXValue r = px_str(s);
        xfree(s);
        return r;
    }
    if (strncmp(j->p, "true", 4) == 0) { j->p += 4; return px_bool(true); }
    if (strncmp(j->p, "false", 5) == 0) { j->p += 5; return px_bool(false); }
    if (strncmp(j->p, "null", 4) == 0) { j->p += 4; return px_null(); }
    // 数字
    char* end;
    long long iv = strtoll(j->p, &end, 10);
    if (end != j->p && (*end == 0 || *end == ',' || *end == '}' || *end == ']' || *end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        j->p = end;
        return px_int(iv);
    }
    double dv = strtod(j->p, &end);
    if (end != j->p) {
        j->p = end;
        return px_float(dv);
    }
    px_error("json: 无法解析");
    return px_null();
}

static LXValue bi_json_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("json_parse 需要一个字符串参数");
    JsonCtx j = { args[0].as.obj->as.str.data };
    LXValue r = json_parse_value(&j);
    return r;
}

// Value -> JSON 字符串（写入动态缓冲）
typedef struct { char* buf; int len, cap; } JOut;
static void jout_append(JOut* o, const char* s) {
    int l = (int)strlen(s);
    if (o->len + l + 1 > o->cap) {
        o->cap = o->cap * 2 + l + 16;
        o->buf = xrealloc(o->buf, o->cap);
    }
    memcpy(o->buf + o->len, s, l);
    o->len += l;
    o->buf[o->len] = 0;
}
static void jout_escape(JOut* o, const char* s) {
    jout_append(o, "\"");
    for (const char* p = s; *p; p++) {
        char c = *p;
        switch (c) {
            case '"': jout_append(o, "\\\""); break;
            case '\\': jout_append(o, "\\\\"); break;
            case '\n': jout_append(o, "\\n"); break;
            case '\r': jout_append(o, "\\r"); break;
            case '\t': jout_append(o, "\\t"); break;
            default: {
                char tmp[2] = { c, 0 };
                jout_append(o, tmp);
            }
        }
    }
    jout_append(o, "\"");
}
static void json_stringify_value(JOut* o, LXValue v) {
    switch (v.type) {
        case PX_NULL: jout_append(o, "null"); break;
        case PX_BOOL: jout_append(o, v.as.b ? "true" : "false"); break;
        case PX_INT: {
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "%lld", (long long)v.as.i);
            jout_append(o, tmp);
            break;
        }
        case PX_FLOAT: {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "%g", v.as.f);
            jout_append(o, tmp);
            break;
        }
        case PX_STR: jout_escape(o, v.as.obj->as.str.data); break;
        case PX_LIST: {
            jout_append(o, "[");
            LXObject* ob = v.as.obj;
            for (int i = 0; i < ob->as.list.len; i++) {
                if (i) jout_append(o, ",");
                json_stringify_value(o, ob->as.list.items[i]);
            }
            jout_append(o, "]");
            break;
        }
        case PX_TUPLE: {
            jout_append(o, "[");
            LXObject* ob = v.as.obj;
            for (int i = 0; i < ob->as.tuple.len; i++) {
                if (i) jout_append(o, ",");
                json_stringify_value(o, ob->as.tuple.items[i]);
            }
            jout_append(o, "]");
            break;
        }
        case PX_DICT: {
            jout_append(o, "{");
            LXObject* ob = v.as.obj;
            for (int i = 0; i < ob->as.dict.len; i++) {
                if (i) jout_append(o, ",");
                jout_escape(o, ob->as.dict.keys[i]);
                jout_append(o, ":");
                json_stringify_value(o, ob->as.dict.vals[i]);
            }
            jout_append(o, "}");
            break;
        }
        default: px_error("json: 无法序列化类型 %s", px_type_name(v));
    }
}
static LXValue bi_json_stringify(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("json_stringify 需要一个参数");
    JOut o = { NULL, 0, 0 };
    o.buf = xmalloc(64); o.cap = 64; o.buf[0] = 0;
    json_stringify_value(&o, args[0]);
    LXValue r = px_str(o.buf);
    xfree(o.buf);
    return r;
}

// ---- M29：JSON 路径运算符（JSONB 基石）----
// json_path(json_or_str, "$.a[0].b") → 按路径取（.key ["key"] [n] 负索引；取不到 → null）
// json_path_set(json_or_str, path, value) → 返回更新后的新值（路径不存在自动创建）

// 段类型：字符串键 / 数组索引
typedef struct { int is_idx; long long idx; char* key; } JPathSeg;

// 解析路径 → 段数组（返回段数；失败返回 -1）
static int json_path_parse(const char* path, JPathSeg* segs, int max) {
    int n = 0;
    const char* p = path;
    if (*p == '$') p++;
    while (*p) {
        if (n >= max) return -1;
        if (*p == '.') {
            p++;
            if (*p == '[') {
                // .["key"] 或 .[n]
                p++;
                if (*p == '"') {
                    p++;
                    const char* s = p;
                    while (*p && *p != '"') p++;
                    if (!*p) return -1;
                    char* key = xmalloc((size_t)(p - s) + 1);
                    memcpy(key, s, (size_t)(p - s)); key[p - s] = 0;
                    p++; // "
                    if (*p != ']') { xfree(key); return -1; }
                    p++; // ]
                    segs[n].is_idx = 0; segs[n].key = key; n++;
                } else {
                    char* end;
                    long long v = strtoll(p, &end, 10);
                    if (end == p) return -1;
                    if (*end != ']') return -1;
                    segs[n].is_idx = 1; segs[n].idx = v; segs[n].key = NULL; n++;
                    p = end + 1;
                }
            } else {
                const char* s = p;
                while (*p && *p != '.' && *p != '[') p++;
                if (p == s) return -1;
                char* key = xmalloc((size_t)(p - s) + 1);
                memcpy(key, s, (size_t)(p - s)); key[p - s] = 0;
                segs[n].is_idx = 0; segs[n].key = key; n++;
            }
        } else if (*p == '[') {
            p++;
            if (*p == '"') {
                p++;
                const char* s = p;
                while (*p && *p != '"') p++;
                if (!*p) return -1;
                char* key = xmalloc((size_t)(p - s) + 1);
                memcpy(key, s, (size_t)(p - s)); key[p - s] = 0;
                p++;
                if (*p != ']') { xfree(key); return -1; }
                p++;
                segs[n].is_idx = 0; segs[n].key = key; n++;
            } else {
                char* end;
                long long v = strtoll(p, &end, 10);
                if (end == p) return -1;
                if (*end != ']') return -1;
                segs[n].is_idx = 1; segs[n].idx = v; segs[n].key = NULL; n++;
                p = end + 1;
            }
        } else {
            // 无前缀：a.b[0]
            const char* s = p;
            while (*p && *p != '.' && *p != '[') p++;
            if (p == s) return -1;
            char* key = xmalloc((size_t)(p - s) + 1);
            memcpy(key, s, (size_t)(p - s)); key[p - s] = 0;
            segs[n].is_idx = 0; segs[n].key = key; n++;
        }
    }
    return n;
}

static void json_path_segs_free(JPathSeg* segs, int n) {
    for (int i = 0; i < n; i++) if (!segs[i].is_idx && segs[i].key) xfree(segs[i].key);
}

// 按路径取（不 deep copy，返回内部引用）
static LXValue json_path_walk(LXValue cur, JPathSeg* segs, int n) {
    for (int i = 0; i < n; i++) {
        if (segs[i].is_idx) {
            if (cur.type != PX_LIST) return px_null();
            LXObject* o = cur.as.obj;
            long long idx = segs[i].idx;
            if (idx < 0) idx += o->as.list.len;
            if (idx < 0 || idx >= o->as.list.len) return px_null();
            cur = o->as.list.items[(int)idx];
        } else {
            if (cur.type != PX_DICT) return px_null();
            cur = px_dict_get(cur, segs[i].key);
            if (cur.type == PX_NULL) return px_null();
        }
    }
    return cur;
}

// 深拷贝（JSON 可序列化部分）
static LXValue json_value_copy(LXValue v) {
    switch (v.type) {
        case PX_NULL: case PX_BOOL: case PX_INT: case PX_FLOAT: case PX_STR: case PX_BYTES:
            return v;
        case PX_LIST: {
            LXValue r = px_list(0);
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.list.len; i++) px_list_push(r, json_value_copy(o->as.list.items[i]));
            return r;
        }
        case PX_TUPLE: {
            LXObject* o = v.as.obj;
            LXValue* items = xmalloc(sizeof(LXValue) * (size_t)(o->as.tuple.len > 0 ? o->as.tuple.len : 1));
            for (int i = 0; i < o->as.tuple.len; i++) items[i] = json_value_copy(o->as.tuple.items[i]);
            LXValue r = px_tuple(items, o->as.tuple.len);
            xfree(items);
            return r;
        }
        case PX_DICT: {
            LXValue r = px_dict();
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.dict.len; i++) {
                px_dict_set(r, o->as.dict.keys[i], json_value_copy(o->as.dict.vals[i]));
            }
            return r;
        }
        default: return v;
    }
}

// 递归设值：在 base 的 segs[0..] 处写入 new_val，返回新对象
static LXValue json_path_set_at(LXValue base, JPathSeg* segs, int n, LXValue new_val) {
    if (n == 0) return new_val;
    if (segs[0].is_idx) {
        long long idx = segs[0].idx;
        LXValue lst = (base.type == PX_LIST) ? base : px_list(0);
        LXObject* o = lst.as.obj;
        int len = o->as.list.len;
        long long real = (idx < 0) ? len + idx : idx;
        // 深拷贝现有元素
        LXValue* items = xmalloc(sizeof(LXValue) * (size_t)(len > 0 ? len : 1));
        for (int i = 0; i < len; i++) items[i] = json_value_copy(o->as.list.items[i]);
        LXValue r = px_list(len);
        for (int i = 0; i < len; i++) px_list_push(r, items[i]);
        xfree(items);
        LXObject* ro = r.as.obj;
        if (real < 0) {
            // 负索引越界 → 插入 0
            LXValue child = json_path_set_at(px_null(), segs + 1, n - 1, new_val);
            // 在 0 处插入：新 list 重建
            LXValue* ni = xmalloc(sizeof(LXValue) * (size_t)(ro->as.list.len + 1));
            ni[0] = child;
            for (int i = 0; i < ro->as.list.len; i++) ni[i + 1] = ro->as.list.items[i];
            LXValue rr = px_list(ro->as.list.len + 1);
            for (int i = 0; i < ro->as.list.len + 1; i++) px_list_push(rr, ni[i]);
            xfree(ni);
            return rr;
        } else if (real < ro->as.list.len) {
            // 原位设值
            LXValue child = json_path_set_at(ro->as.list.items[(int)real], segs + 1, n - 1, new_val);
            ro->as.list.items[(int)real] = child;
            return r;
        } else {
            // 越界扩展：null 填充 + 追加
            while (ro->as.list.len < real) px_list_push(r, px_null());
            LXValue child = json_path_set_at(px_null(), segs + 1, n - 1, new_val);
            px_list_push(r, child);
            return r;
        }
    } else {
        // dict 字段
        LXValue d = (base.type == PX_DICT) ? base : px_dict();
        LXObject* o = d.as.obj;
        LXValue r = px_dict();
        for (int i = 0; i < o->as.dict.len; i++) {
            px_dict_set(r, o->as.dict.keys[i], json_value_copy(o->as.dict.vals[i]));
        }
        LXValue child = px_dict_get(d, segs[0].key);
        LXValue nv = (child.type == PX_NULL)
            ? json_path_set_at(px_null(), segs + 1, n - 1, new_val)
            : json_path_set_at(child, segs + 1, n - 1, new_val);
        px_dict_set(r, segs[0].key, nv);
        return r;
    }
}

static LXValue bi_json_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("json_path 需要 (json, path) 参数");
    // 输入归一化：str → 解析
    LXValue v = args[0];
    if (v.type == PX_STR) {
        JsonCtx j = { v.as.obj->as.str.data };
        v = json_parse_value(&j);
    }
    if (args[1].type != PX_STR) px_error("json_path 的 path 需要字符串");
    JPathSeg segs[64];
    int n = json_path_parse(args[1].as.obj->as.str.data, segs, 64);
    if (n < 0) { json_path_segs_free(segs, 0); px_error("json_path: 非法路径"); }
    LXValue r = json_path_walk(v, segs, n);
    json_path_segs_free(segs, n);
    return r;
}

static LXValue bi_json_path_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("json_path_set 需要 (json, path, value) 参数");
    LXValue v = args[0];
    if (v.type == PX_STR) {
        JsonCtx j = { v.as.obj->as.str.data };
        v = json_parse_value(&j);
    }
    if (args[1].type != PX_STR) px_error("json_path_set 的 path 需要字符串");
    JPathSeg segs[64];
    int n = json_path_parse(args[1].as.obj->as.str.data, segs, 64);
    if (n < 0) { json_path_segs_free(segs, 0); px_error("json_path_set: 非法路径"); }
    LXValue r;
    if (n == 0) {
        r = json_value_copy(args[2]);
    } else {
        r = json_path_set_at(json_value_copy(v), segs, n, args[2]);
    }
    json_path_segs_free(segs, n);
    return r;
}

// ---- std.time ----

static LXValue bi_now(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
             tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return px_str(buf);
}

// ---- std.os ----

static LXValue bi_env(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("env 需要一个变量名");
    const char* v = getenv(args[0].as.obj->as.str.data);
    return v ? px_str(v) : px_null();
}

static LXValue bi_args(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    // 编译版不保留原始 argv，返回空列表（脚本版返回 args）
    return px_list(0);
}

// ---- std.collections（高阶函数） ----

static LXValue bi_map(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_LIST) px_error("map 需要 (list, fn)");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue r = px_list(0);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue res = px_call(fn, &item, 1);
        px_list_push(r, res);
    }
    return r;
}

static LXValue bi_filter(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_LIST) px_error("filter 需要 (list, fn)");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue r = px_list(0);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue res = px_call(fn, &item, 1);
        if (px_is_truthy(res)) px_list_push(r, item);
    }
    return r;
}

static LXValue bi_reduce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3 || args[0].type != PX_LIST) px_error("reduce 需要 (list, fn, [init])");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue acc = (nargs >= 3) ? args[2] : px_int(0);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue pair[2] = { acc, item };
        acc = px_call(fn, pair, 2);
    }
    return acc;
}

void px_register_builtins(void) {
    px_set_global("print", px_native("print", bi_print));
    px_set_global("len", px_native("len", bi_len));
    px_set_global("range", px_native("range", bi_range));
    px_set_global("type", px_native("type", bi_type));
    px_set_global("str", px_native("str", bi_str));
    px_set_global("int", px_native("int", bi_int));
    px_set_global("float", px_native("float", bi_float));
    px_set_global("bool", px_native("bool", bi_bool));
    px_set_global("assert", px_native("assert", bi_assert));
    px_set_global("panic", px_native("panic", bi_panic));
    px_set_global("sleep", px_native("sleep", bi_sleep));
    px_set_global("to_upper", px_native("to_upper", bi_to_upper));
    px_set_global("to_lower", px_native("to_lower", bi_to_lower));
    px_set_global("trim", px_native("trim", bi_trim));
    px_set_global("now_ms", px_native("now_ms", bi_now_ms));
    px_set_global("abs", px_native("abs", bi_abs));
    px_set_global("min", px_native("min", bi_min));
    px_set_global("max", px_native("max", bi_max));
    px_set_global("sum", px_native("sum", bi_sum));
    px_set_global("sqrt", px_native("sqrt", bi_sqrt));
    // M5 标准库
    px_set_global("input", px_native("input", bi_input));
    px_set_global("exit", px_native("exit", bi_exit));
    px_set_global("split", px_native("split", bi_split));
    px_set_global("join", px_native("join", bi_join));
    px_set_global("contains", px_native("contains", bi_contains));
    px_set_global("replace", px_native("replace", bi_replace));
    px_set_global("starts_with", px_native("starts_with", bi_starts_with));
    px_set_global("ends_with", px_native("ends_with", bi_ends_with));
    px_set_global("pow", px_native("pow", bi_pow));
    px_set_global("sorted", px_native("sorted", bi_sorted));
    px_set_global("reversed", px_native("reversed", bi_reversed));
    px_set_global("read_file", px_native("read_file", bi_read_file));
    px_set_global("write_file", px_native("write_file", bi_write_file));
    px_set_global("append_file", px_native("append_file", bi_append_file));
    // M12 P0：文件随机读写 + fsync（WAL / 增量日志基石）
    px_set_global("read_at", px_native("read_at", bi_read_at));
    px_set_global("write_at", px_native("write_at", bi_write_at));
    px_set_global("file_size", px_native("file_size", bi_file_size));
    px_set_global("fsync_file", px_native("fsync_file", bi_fsync_file));
    px_set_global("truncate_file", px_native("truncate_file", bi_truncate_file));
    // M14 P1：crypto 哈希
    px_set_global("sha256", px_native("sha256", bi_sha256));
    px_set_global("xxhash", px_native("xxhash", bi_xxhash));
    // M15 P1：正则表达式（文本解析 / 日志分析 / 参数抽取）
    px_set_global("regex_find", px_native("regex_find", bi_regex_find));
    px_set_global("regex_match", px_native("regex_match", bi_regex_match));
    px_set_global("regex_search", px_native("regex_search", bi_regex_search));
    px_set_global("regex_find_all", px_native("regex_find_all", bi_regex_find_all));
    px_set_global("regex_replace", px_native("regex_replace", bi_regex_replace));
    px_set_global("regex_split", px_native("regex_split", bi_regex_split));
    px_set_global("exists", px_native("exists", bi_exists));
    px_set_global("list_dir", px_native("list_dir", bi_list_dir));
    px_set_global("mkdir", px_native("mkdir", bi_mkdir));
    px_set_global("remove", px_native("remove", bi_remove));
    px_set_global("json_parse", px_native("json_parse", bi_json_parse));
    px_set_global("json_stringify", px_native("json_stringify", bi_json_stringify));
    // M29：JSON 路径运算符（JSONB 基石）
    px_set_global("json_path", px_native("json_path", bi_json_path));
    px_set_global("json_path_set", px_native("json_path_set", bi_json_path_set));
    px_set_global("now", px_native("now", bi_now));
    px_set_global("env", px_native("env", bi_env));
    px_set_global("args", px_native("args", bi_args));
    px_set_global("map", px_native("map", bi_map));
    px_set_global("filter", px_native("filter", bi_filter));
    px_set_global("reduce", px_native("reduce", bi_reduce));
    // std.net（M5.2）
    px_set_global("tcp_listen", px_native("tcp_listen", bi_tcp_listen));
    px_set_global("tcp_accept", px_native("tcp_accept", bi_tcp_accept));
    px_set_global("tcp_connect", px_native("tcp_connect", bi_tcp_connect));
    px_set_global("tcp_send", px_native("tcp_send", bi_tcp_send));
    px_set_global("tcp_recv", px_native("tcp_recv", bi_tcp_recv));
    px_set_global("tcp_close", px_native("tcp_close", bi_tcp_close));
    px_set_global("http_get", px_native("http_get", bi_http_get));
    px_set_global("http_post", px_native("http_post", bi_http_post));
    px_set_global("http_serve", px_native("http_serve", bi_http_serve));
    // M23c P1：HTTP 生产化（http_request 连接池 / http_get_stream 流式下载）
    px_set_global("http_request", px_native("http_request", bi_http_request));
    px_set_global("http_get_stream", px_native("http_get_stream", bi_http_get_stream));
    // M17 .px 脚本执行机制
    px_set_global("px_exec", px_native("px_exec", bi_px_exec));
    px_set_global("px_serve", px_native("px_serve", bi_px_serve));
    // M18 后台定时任务 / 定时器原语
    px_set_global("set_timeout", px_native("set_timeout", bi_set_timeout));
    px_set_global("set_interval", px_native("set_interval", bi_set_interval));
    px_set_global("clear_timer", px_native("clear_timer", bi_clear_timer));
    // M19 P1：AES 加密（企微回调加解密 / 数据落盘加密 / Cookie 签名）
    px_set_global("aes_encrypt", px_native("aes_encrypt", bi_aes_encrypt));
    px_set_global("aes_decrypt", px_native("aes_decrypt", bi_aes_decrypt));
    px_set_global("aes_gcm_encrypt", px_native("aes_gcm_encrypt", bi_aes_gcm_encrypt));
    px_set_global("aes_gcm_decrypt", px_native("aes_gcm_decrypt", bi_aes_gcm_decrypt));
    // M19 P1：XML 解析（企微回调 Encrypt 报文 / 配置文件 / 文档）
    px_set_global("xml_parse", px_native("xml_parse", bi_xml_parse));
    px_set_global("xml_escape", px_native("xml_escape", bi_xml_escape));
    px_set_global("xml_unescape", px_native("xml_unescape", bi_xml_unescape));
    px_set_global("xml_build", px_native("xml_build", bi_xml_build));
    // M19 P1：zip 打包/解压（docx/xlsx/pptx 是 zip+xml，文档工具基石）
    px_set_global("zip_pack", px_native("zip_pack", bi_zip_pack));
    px_set_global("zip_unpack", px_native("zip_unpack", bi_zip_unpack));
    // M21 P1：base64 编解码
    px_set_global("base64_encode", px_native("base64_encode", bi_base64_encode));
    px_set_global("base64_decode", px_native("base64_decode", bi_base64_decode));
    // M21 P1：SSE 服务端（LLM 流式推送 / 实时通知）
    px_set_global("sse_serve", px_native("sse_serve", bi_sse_serve));
    px_set_global("sse_send", px_native("sse_send", bi_sse_send));
    px_set_global("sse_close", px_native("sse_close", bi_sse_close));
    // M23 P1：SSE 客户端（流式消费 / 事件订阅）
    px_set_global("sse_connect", px_native("sse_connect", bi_sse_connect));
    px_set_global("sse_read", px_native("sse_read", bi_sse_read));
    // M22 P1：位运算 / 二进制数据视图（存储引擎序列化基石）
    px_set_global("int_to_hex", px_native("int_to_hex", bi_int_to_hex));
    px_set_global("hex_to_int", px_native("hex_to_int", bi_hex_to_int));
    px_set_global("bytes_to_hex", px_native("bytes_to_hex", bi_bytes_to_hex));
    px_set_global("hex_to_bytes", px_native("hex_to_bytes", bi_hex_to_bytes));
    px_set_global("bit_count", px_native("bit_count", bi_bit_count));
    px_set_global("bit_length", px_native("bit_length", bi_bit_length));
    // M22 P1：WebSocket（RFC 6455，微信/QQ/飞书长连接 / LLM 流式 / 实时推送）
    px_set_global("ws_serve", px_native("ws_serve", bi_ws_serve));
    px_set_global("ws_connect", px_native("ws_connect", bi_ws_connect));
    px_set_global("ws_send", px_native("ws_send", bi_ws_send));
    px_set_global("ws_recv", px_native("ws_recv", bi_ws_recv));
    px_set_global("ws_close", px_native("ws_close", bi_ws_close));
    px_set_global("ws_ping", px_native("ws_ping", bi_ws_ping));
    px_set_global("ws_heartbeat", px_native("ws_heartbeat", bi_ws_heartbeat));
    // M27 P0：WebServer 生产化四件套（服务端 TLS / Session / 基础认证）
    px_set_global("tls_server", px_native("tls_server", bi_tls_server));
    px_set_global("session_open", px_native("session_open", bi_session_open));
    px_set_global("session_id", px_native("session_id", bi_session_id));
    px_set_global("session_get", px_native("session_get", bi_session_get));
    px_set_global("session_set", px_native("session_set", bi_session_set));
    px_set_global("session_del", px_native("session_del", bi_session_del));
    px_set_global("session_destroy", px_native("session_destroy", bi_session_destroy));
    px_set_global("basic_auth", px_native("basic_auth", bi_basic_auth));
    // M28 P1：路由表 + 中间件（runtime_route.c）
    px_set_global("route", px_native("route", bi_route));
    px_set_global("middleware", px_native("middleware", bi_middleware));
    // M28 P1：SQLite 绑定（runtime_sqlite.c）
    px_set_global("sqlite_open", px_native("sqlite_open", bi_sqlite_open));
    px_set_global("sqlite_exec", px_native("sqlite_exec", bi_sqlite_exec));
    px_set_global("sqlite_query", px_native("sqlite_query", bi_sqlite_query));
    px_set_global("sqlite_close", px_native("sqlite_close", bi_sqlite_close));
    px_set_global("sqlite_escape", px_native("sqlite_escape", bi_sqlite_escape));
    px_set_global("sqlite_last_insert_rowid", px_native("sqlite_last_insert_rowid", bi_sqlite_last_insert_rowid));
    // M28 P1：时间 / 时区
    px_set_global("time_format", px_native("time_format", bi_time_format));
    px_set_global("time_parse", px_native("time_parse", bi_time_parse));
    px_set_global("tz_offset", px_native("tz_offset", bi_tz_offset));
    // M28 P1：cron 定时调度
    px_set_global("cron", px_native("cron", bi_cron));
    // M22 P1：强制垃圾回收（解释器追踪式 GC / 编译模式保守标记-清除）
    px_set_global("gc", px_native("gc", bi_gc));
    // M23 P1：进程/信号（文殊场景收尾：外部工具编排、守护进程、优雅停机）
    px_set_global("os_pid", px_native("os_pid", bi_os_pid));
    px_set_global("os_spawn", px_native("os_spawn", bi_os_spawn));
    px_set_global("os_wait", px_native("os_wait", bi_os_wait));
    px_set_global("os_kill", px_native("os_kill", bi_os_kill));
    px_set_global("signal", px_native("signal", bi_signal));
    // M23d P1：RSA（PKCS#1 v1.5，密钥/密文/签名均 hex；实现 runtime_rsa.c）
    px_set_global("rsa_gen_key", px_native("rsa_gen_key", bi_rsa_gen_key));
    px_set_global("rsa_encrypt", px_native("rsa_encrypt", bi_rsa_encrypt));
    px_set_global("rsa_decrypt", px_native("rsa_decrypt", bi_rsa_decrypt));
    px_set_global("rsa_sign", px_native("rsa_sign", bi_rsa_sign));
    px_set_global("rsa_verify", px_native("rsa_verify", bi_rsa_verify));
    // M23b P1：二进制安全字节串（bytes 类型；带长度，可含 NUL）
    px_set_global("bytes", px_native("bytes", bi_bytes));
    px_set_global("bytes_len", px_native("bytes_len", bi_bytes_len));
    px_set_global("bytes_get", px_native("bytes_get", bi_bytes_get));
    px_set_global("bytes_set", px_native("bytes_set", bi_bytes_set));
    px_set_global("bytes_slice", px_native("bytes_slice", bi_bytes_slice));
    px_set_global("bytes_concat", px_native("bytes_concat", bi_bytes_concat));
    px_set_global("bytes_to_str", px_native("bytes_to_str", bi_bytes_to_str));
    px_set_global("bytes_base64", px_native("bytes_base64", bi_bytes_base64));
    px_set_global("base64_to_bytes", px_native("base64_to_bytes", bi_base64_to_bytes));
    px_set_global("bytes_find", px_native("bytes_find", bi_bytes_find));
    px_set_global("read_bytes", px_native("read_bytes", bi_read_bytes));
    px_set_global("write_bytes", px_native("write_bytes", bi_write_bytes));
    px_set_global("int_to_bytes", px_native("int_to_bytes", bi_int_to_bytes));
    px_set_global("bytes_to_int", px_native("bytes_to_int", bi_bytes_to_int));
}

// gc() → int：强制运行一次垃圾回收（与解释器 gc() 双模式一致）
static LXValue bi_gc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("gc 不需要参数");
    px_gc_collect();
    return px_int(1);
}

// ==================== M23 进程 / 信号（编译模式，与解释器 builtin.rs 双模式一致） ====================
// os_pid() → int 当前进程 PID
// os_spawn(cmd, args) → int pid | null（fork+execvp 启动，不等待）
// os_wait(pid) → int 退出码（正常=exit code；信号终止=128+sig；失败=-1）
// os_kill(pid, sig) → bool
// signal(sig, handler) → bool（self-pipe：信号处理器写 1 字节到管道，专用线程读管道
//                         → 调用注册的普贤 handler(sig)；handler 存入全局表防 GC 回收）

static LXValue bi_os_pid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("os_pid 不需要参数");
    return px_int((int64_t)getpid());
}

static LXValue bi_os_spawn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_LIST)
        px_error("os_spawn 需要 (cmd, args) 参数");
    const char* cmd = args[0].as.obj->as.str.data;
    LXObject* list = args[1].as.obj;
    int argc = list->as.list.len;
    char** argv = (char**)calloc((size_t)argc + 2, sizeof(char*));
    argv[0] = strdup(cmd);
    for (int i = 0; i < argc; i++) {
        LXValue v = list->as.list.items[i];
        if (v.type != PX_STR) {
            for (int j = 0; j <= i; j++) free(argv[j]);
            free(argv);
            px_error("os_spawn 的 args 必须是字符串列表");
        }
        argv[i + 1] = strdup(v.as.obj->as.str.data);
    }
    argv[argc + 1] = NULL;
    pid_t pid = fork();
    if (pid < 0) {
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    if (pid == 0) {
        // 子进程：execvp（argv[0]=cmd）
        execvp(cmd, argv);
        _exit(127);
    }
    for (int i = 0; i <= argc; i++) free(argv[i]);
    free(argv);
    return px_int((int64_t)pid);
}

static LXValue bi_os_wait(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("os_wait 需要 (pid) 参数");
    pid_t pid = (pid_t)args[0].as.i;
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return px_int(-1);
    if (WIFEXITED(status)) return px_int((int64_t)WEXITSTATUS(status));
    if (WIFSIGNALED(status)) return px_int(128 + (int64_t)WTERMSIG(status));
    return px_int(-1);
}

static LXValue bi_os_kill(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("os_kill 需要 (pid, sig) 参数");
    pid_t pid = (pid_t)args[0].as.i;
    int sig = (int)args[1].as.i;
    return px_bool(kill(pid, sig) == 0);
}

// ---- signal：self-pipe + 专用分发线程 ----
#define MAX_SIG_HANDLERS 64
static int g_sig_pipe[2] = {-1, -1};
static pthread_t g_sig_thread;
static volatile sig_atomic_t g_sig_thread_started = 0;
static pthread_mutex_t g_sig_mu = PTHREAD_MUTEX_INITIALIZER;
static struct { int sig; LXValue handler; } g_sig_handlers[MAX_SIG_HANDLERS];
static int g_sig_handler_count = 0;

static void sig_bridge(int sig) {
    unsigned char b = (unsigned char)sig;
    if (g_sig_pipe[1] >= 0) {
        ssize_t r = write(g_sig_pipe[1], &b, 1);
        (void)r;
    }
}

static void* sig_dispatch_thread(void* arg) {
    (void)arg;
    unsigned char buf[64];
    for (;;) {
        ssize_t n = read(g_sig_pipe[0], buf, sizeof(buf));
        if (n <= 0) continue;
        for (ssize_t i = 0; i < n; i++) {
            int sig = buf[i];
            pthread_mutex_lock(&g_sig_mu);
            LXValue h = px_null();
            for (int j = 0; j < g_sig_handler_count; j++) {
                if (g_sig_handlers[j].sig == sig) { h = g_sig_handlers[j].handler; break; }
            }
            pthread_mutex_unlock(&g_sig_mu);
            if (h.type == PX_FUNC || h.type == PX_NATIVE) {
                LXValue arg = px_int(sig);
                px_call(h, &arg, 1);
            }
        }
    }
    return NULL;
}

static LXValue bi_signal(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("signal 需要 (sig, handler) 参数");
    int sig = (int)args[0].as.i;
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE)
        px_error("signal 的 handler 必须是函数");
    // 首次：创建管道 + 启动分发线程
    if (!g_sig_thread_started) {
        if (pipe(g_sig_pipe) != 0) return px_bool(false);
        pthread_create(&g_sig_thread, NULL, sig_dispatch_thread, NULL);
        g_sig_thread_started = 1;
    }
    // 注册普贤 handler（存全局表 + 全局表键防 GC 回收）
    pthread_mutex_lock(&g_sig_mu);
    int found = 0;
    for (int j = 0; j < g_sig_handler_count; j++) {
        if (g_sig_handlers[j].sig == sig) {
            g_sig_handlers[j].handler = handler;
            found = 1;
            break;
        }
    }
    if (!found && g_sig_handler_count < MAX_SIG_HANDLERS) {
        g_sig_handlers[g_sig_handler_count].sig = sig;
        g_sig_handlers[g_sig_handler_count].handler = handler;
        g_sig_handler_count++;
    }
    pthread_mutex_unlock(&g_sig_mu);
    // 存入全局表防 GC 回收（handler 是用户函数对象，需在 GC 根中）
    char key[64];
    snprintf(key, sizeof(key), "__sig_handler_%d", sig);
    px_set_global(key, handler);
    // 注册 C 信号处理器
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_bridge;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(sig, &sa, NULL);
    return px_bool(true);
}

// ==================== M23b 二进制安全字节串 ====================
// 字符串/字节串统一取 data+len（二进制安全，可含 NUL；PX_STR 与 PX_BYTES 均可；
// 数值自动字符串化——与解释器 bytes_of 的 to_string 语义一致）
static const char* bdata(LXValue v) {
    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.data;
    static char tmp[64];
    snprintf(tmp, sizeof(tmp), "%s", fmt_num(v));
    return tmp;
}
static int blen(LXValue v) {
    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.len;
    return (int)strlen(bdata(v));
}

// bytes(s) → bytes（字符串/字节串 UTF-8 字节原样）
static LXValue bi_bytes(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bytes 需要一个参数");
    return px_bytes_len(bdata(args[0]), blen(args[0]));
}

// bytes_len(b) → int
static LXValue bi_bytes_len(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bytes_len 需要一个参数");
    if (args[0].type != PX_BYTES) px_error("bytes_len 需要 bytes，实际是 %s", px_type_name(args[0]));
    return px_int(args[0].as.obj->as.str.len);
}

// bytes_get(b, i) → int|null（负索引支持；越界 → null）
static LXValue bi_bytes_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("bytes_get 需要 (bytes, index) 参数");
    if (args[0].type != PX_BYTES) px_error("bytes_get 需要 bytes，实际是 %s", px_type_name(args[0]));
    int64_t i = int_val(args[1]);
    int len = args[0].as.obj->as.str.len;
    int64_t idx = i;
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) return px_null();
    return px_int((unsigned char)args[0].as.obj->as.str.data[idx]);
}

// bytes_set(b, i, v) → bytes（函数式：返回修改后的新 bytes，原对象不变）
static LXValue bi_bytes_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("bytes_set 需要 (bytes, index, value) 参数");
    if (args[0].type != PX_BYTES) px_error("bytes_set 需要 bytes，实际是 %s", px_type_name(args[0]));
    int64_t i = int_val(args[1]);
    int64_t v = int_val(args[2]);
    if (v < 0 || v > 255) px_error("bytes_set 的值必须在 0..255");
    int len = args[0].as.obj->as.str.len;
    int64_t idx = i;
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) px_error("bytes_set 下标越界");
    const char* src = args[0].as.obj->as.str.data;
    char* d = xmalloc((size_t)len + 1);
    memcpy(d, src, (size_t)len);
    d[len] = 0;
    d[idx] = (char)(unsigned char)v;
    LXValue r = px_bytes_len(d, len);
    xfree(d);
    return r;
}

// bytes_slice(b, start, end) → bytes（start/end 传 null 表示省略；负索引/越界 clamp）
static LXValue bi_bytes_slice(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 3) px_error("bytes_slice 需要 (bytes[, start[, end]]) 参数");
    if (args[0].type != PX_BYTES) px_error("bytes_slice 需要 bytes，实际是 %s", px_type_name(args[0]));
    int len = args[0].as.obj->as.str.len;
    int64_t a, b, sa, sb;
    bool has_s, has_e;
    if (nargs >= 2 && args[1].type != PX_NULL) {
        sa = int_val(args[1]); has_s = true;
    } else { sa = 0; has_s = false; }
    if (nargs >= 3 && args[2].type != PX_NULL) {
        sb = int_val(args[2]); has_e = true;
    } else { sb = len; has_e = false; }
    a = has_s ? (sa < 0 ? (sa + len > 0 ? sa + len : 0) : (sa < len ? sa : len)) : 0;
    b = has_e ? (sb < 0 ? (sb + len > 0 ? sb + len : 0) : (sb < len ? sb : len)) : len;
    if (a > b) a = b = a;
    return px_bytes_len(args[0].as.obj->as.str.data + a, (int)(b - a));
}

// bytes_concat(a, b, ...) → bytes（Str/Bytes 混合均可，UTF-8 字节原样）
static LXValue bi_bytes_concat(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) px_error("bytes_concat 至少需要一个参数");
    int total = 0;
    for (int i = 0; i < nargs; i++) total += blen(args[i]);
    char* d = xmalloc((size_t)total + 1);
    int off = 0;
    for (int i = 0; i < nargs; i++) {
        memcpy(d + off, bdata(args[i]), (size_t)blen(args[i]));
        off += blen(args[i]);
    }
    d[total] = 0;
    LXValue r = px_bytes_len(d, total);
    xfree(d);
    return r;
}

// bytes_to_str(b) → str（UTF-8 lossy）
static LXValue bi_bytes_to_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bytes_to_str 需要一个参数");
    if (args[0].type != PX_BYTES) px_error("bytes_to_str 需要 bytes，实际是 %s", px_type_name(args[0]));
    // 字节 → UTF-8 字符串（C 端不做 lossy 替换，直接按字节复制；与解释器 lossy 对合法 UTF-8 一致）
    const char* d = args[0].as.obj->as.str.data;
    int len = args[0].as.obj->as.str.len;
    return px_str_len(d, len);
}

// bytes_base64(b) → base64 str（字节安全编码）
static LXValue bi_bytes_base64(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("bytes_base64 需要一个参数");
    const char* data = bdata(args[0]);
    int len = blen(args[0]);
    int olen = ((len + 2) / 3) * 4;
    char* out = xmalloc((size_t)olen + 1);
    int oi = 0, i = 0;
    while (i + 3 <= len) {
        unsigned n = ((unsigned char)data[i] << 16) | ((unsigned char)data[i+1] << 8) | (unsigned char)data[i+2];
        out[oi++] = B64_TBL[(n >> 18) & 63];
        out[oi++] = B64_TBL[(n >> 12) & 63];
        out[oi++] = B64_TBL[(n >> 6) & 63];
        out[oi++] = B64_TBL[n & 63];
        i += 3;
    }
    int rem = len - i;
    if (rem == 1) {
        unsigned n = (unsigned char)data[i] << 16;
        out[oi++] = B64_TBL[(n >> 18) & 63];
        out[oi++] = B64_TBL[(n >> 12) & 63];
        out[oi++] = '='; out[oi++] = '=';
    } else if (rem == 2) {
        unsigned n = ((unsigned char)data[i] << 16) | ((unsigned char)data[i+1] << 8);
        out[oi++] = B64_TBL[(n >> 18) & 63];
        out[oi++] = B64_TBL[(n >> 12) & 63];
        out[oi++] = B64_TBL[(n >> 6) & 63];
        out[oi++] = '=';
    }
    out[oi] = 0;
    LXValue r = px_str_len(out, oi);
    xfree(out);
    return r;
}

// base64_to_bytes(s) → bytes|null（严格解码：非法 → null）
static LXValue bi_base64_to_bytes(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("base64_to_bytes 需要一个参数");
    const char* s = val_cstr(args[0]);
    int n = (int)strlen(s);
    int pad = 0;
    while (n > 0 && s[n-1] == '=') { pad++; n--; }
    if (pad > 2 || (n % 4) == 1) return px_null();
    int cap = (n / 4) * 3 + 3;
    char* out = xmalloc((size_t)cap + 1);
    int oi = 0, i = 0, q[4], qi = 0;
    while (i < n) {
        int v = b64_val(s[i]);
        if (v < 0) { xfree(out); return px_null(); }
        q[qi++] = v;
        if (qi == 4) {
            unsigned vv = ((unsigned)q[0] << 18) | ((unsigned)q[1] << 12) | ((unsigned)q[2] << 6) | (unsigned)q[3];
            out[oi++] = (char)((vv >> 16) & 0xFF);
            out[oi++] = (char)((vv >> 8) & 0xFF);
            out[oi++] = (char)(vv & 0xFF);
            qi = 0;
        }
        i++;
    }
    if (qi == 2) {
        unsigned vv = ((unsigned)q[0] << 18) | ((unsigned)q[1] << 12);
        out[oi++] = (char)((vv >> 16) & 0xFF);
    } else if (qi == 3) {
        unsigned vv = ((unsigned)q[0] << 18) | ((unsigned)q[1] << 12) | ((unsigned)q[2] << 6);
        out[oi++] = (char)((vv >> 16) & 0xFF);
        out[oi++] = (char)((vv >> 8) & 0xFF);
    } else if (qi != 0) {
        xfree(out); return px_null();
    }
    out[oi] = 0;
    LXValue r = px_bytes_len(out, oi);
    xfree(out);
    return r;
}

// bytes_find(b, sub) → int|null（子串字节下标；sub 可为 bytes 或 str）
static LXValue bi_bytes_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("bytes_find 需要 (bytes, sub) 参数");
    if (args[0].type != PX_BYTES) px_error("bytes_find 需要 bytes，实际是 %s", px_type_name(args[0]));
    const char* b = args[0].as.obj->as.str.data;
    int bl = args[0].as.obj->as.str.len;
    const char* sub = bdata(args[1]);
    int sl = blen(args[1]);
    if (sl == 0) return px_int(0);
    if (sl > bl) return px_null();
    for (int i = 0; i + sl <= bl; i++) {
        if (memcmp(b + i, sub, (size_t)sl) == 0) return px_int(i);
    }
    return px_null();
}

// read_bytes(path) → bytes（二进制安全读取）
static LXValue bi_read_bytes(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("read_bytes 需要一个路径参数");
    const char* p = val_cstr(args[0]);
    FILE* f = fopen(p, "rb");
    if (!f) px_error("io: 读取文件失败 %s", p);
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return px_bytes_len("", 0); }
    char* d = xmalloc((size_t)sz + 1);
    size_t rd = fread(d, 1, (size_t)sz, f);
    fclose(f);
    d[rd] = 0;
    LXValue r = px_bytes_len(d, (int)rd);
    xfree(d);
    return r;
}

// write_bytes(path, b) → bool（二进制安全写入；b 可为 bytes 或 str）
static LXValue bi_write_bytes(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("write_bytes 需要 (路径, bytes) 参数");
    const char* p = val_cstr(args[0]);
    const char* d = bdata(args[1]);
    int len = blen(args[1]);
    FILE* f = fopen(p, "wb");
    if (!f) px_error("io: 写入文件失败 %s", p);
    size_t wr = fwrite(d, 1, (size_t)len, f);
    fclose(f);
    return px_bool(wr == (size_t)len);
}

// ==================== M30 P1：字节序可控整数↔bytes（pxdb 存储基石） ====================
// int_to_bytes(n, size[, endian[, signed]]) → bytes|null
//   size 1..8；endian "big"/"little"（"be"/"le" 也接受，默认 big）
//   signed=false（默认）：范围 [0, 2^(8s)-1]；signed=true：[-2^(8s-1), 2^(8s-1)-1]
//   负数以补码编码；越界返回 null
static LXValue bi_int_to_bytes(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 4) px_error("int_to_bytes 需要 (n, size[, endian[, signed]]) 参数");
    if (args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("int_to_bytes 的 n/size 需要 int");
    int64_t n = args[0].as.i;
    int size = (int)args[1].as.i;
    int big = 1;
    if (nargs >= 3) {
        const char* e = val_cstr(args[2]);
        if (!strcasecmp(e, "little") || !strcasecmp(e, "le")) big = 0;
        else if (!strcasecmp(e, "big") || !strcasecmp(e, "be")) big = 1;
        else px_error("int_to_bytes 的 endian 需为 big/little");
    }
    int signed_ = 0;
    if (nargs >= 4) {
        if (args[3].type != PX_BOOL) px_error("int_to_bytes 的 signed 需为 bool");
        signed_ = args[3].as.b ? 1 : 0;
    }
    if (size < 1 || size > 8) px_error("int_to_bytes 的 size 必须在 1..8");
    // 范围检查
    if (signed_) {
        if (size < 8) {
            int64_t lo = -(int64_t)1 << (8 * size - 1);
            int64_t hi = ((int64_t)1 << (8 * size - 1)) - 1;
            if (n < lo || n > hi) return px_null();
        }
        // size == 8：i64 全范围合法
    } else {
        if (n < 0) return px_null();
        if (size < 8) {
            uint64_t hi = ((uint64_t)1 << (8 * size)) - 1;
            if ((uint64_t)n > hi) return px_null();
        }
    }
    uint64_t v = (uint64_t)n;
    if (size < 8) v &= ((uint64_t)1 << (8 * size)) - 1; // 负数补码截断
    unsigned char buf[8];
    for (int i = 0; i < size; i++) {
        int shift = big ? (size - 1 - i) * 8 : i * 8;
        buf[i] = (unsigned char)((v >> shift) & 0xFF);
    }
    return px_bytes_len(buf, size);
}

// bytes_to_int(b[, endian[, signed]]) → int|null（长度 1..8；非法长度返回 null）
static LXValue bi_bytes_to_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 3) px_error("bytes_to_int 需要 (bytes[, endian[, signed]]) 参数");
    if (args[0].type != PX_BYTES) px_error("bytes_to_int 需要 bytes，实际是 %s", px_type_name(args[0]));
    int len = args[0].as.obj->as.str.len;
    const unsigned char* data = (const unsigned char*)args[0].as.obj->as.str.data;
    int big = 1;
    if (nargs >= 2) {
        const char* e = val_cstr(args[1]);
        if (!strcasecmp(e, "little") || !strcasecmp(e, "le")) big = 0;
        else if (!strcasecmp(e, "big") || !strcasecmp(e, "be")) big = 1;
        else px_error("bytes_to_int 的 endian 需为 big/little");
    }
    int signed_ = 0;
    if (nargs >= 3) {
        if (args[2].type != PX_BOOL) px_error("bytes_to_int 的 signed 需为 bool");
        signed_ = args[2].as.b ? 1 : 0;
    }
    if (len < 1 || len > 8) return px_null();
    uint64_t v = 0;
    if (big) {
        for (int i = 0; i < len; i++) v = (v << 8) | data[i];
    } else {
        for (int i = 0; i < len; i++) v |= (uint64_t)data[i] << (8 * i);
    }
    if (signed_) {
        int hi = big ? 0 : len - 1;
        if (data[hi] & 0x80) {
            if (len < 8) v |= ~(((uint64_t)1 << (8 * len)) - 1); // 符号扩展
            return px_int((int64_t)v);
        }
    }
    return px_int((int64_t)v);
}

// ==================== 并发原语（M4.2） ====================

// 全局 select 唤醒：任何 chan 操作后广播，select 循环醒来重试
static pthread_mutex_t g_sel_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_sel_cv = PTHREAD_COND_INITIALIZER;

void px_select_signal(void) {
    pthread_mutex_lock(&g_sel_mu);
    pthread_cond_broadcast(&g_sel_cv);
    pthread_mutex_unlock(&g_sel_mu);
}

void px_select_wait(void) {
    pthread_mutex_lock(&g_sel_mu);
    pthread_cond_wait(&g_sel_cv, &g_sel_mu);
    pthread_mutex_unlock(&g_sel_mu);
}

bool px_is_chan(LXValue v) { return v.type == PX_CHAN; }
bool px_is_mutex(LXValue v) { return v.type == PX_MUTEX; }
bool px_is_rwlock(LXValue v) { return v.type == PX_RWLOCK; }

// ==================== 锁原语（M13：mutex / rwlock） ====================
// 与解释器语义对齐：pthread_mutex + condvar 实现真正阻塞（非忙等）
// rwlock 写优先：writer_waiting > 0 时阻塞新读者，防止读饿死写

static LXObject* px_mutex_obj(LXValue m, const char* op) {
    if (m.type != PX_MUTEX) px_error("%s: 目标不是互斥锁（%s）", op, px_type_name(m));
    return m.as.obj;
}

LXValue px_mutex_create(void) {
    LXObject* o = xcalloc(1, sizeof(LXObject));
    o->type = PX_MUTEX;
    o->as.mutex.locked = 0;
    pthread_mutex_init(&o->as.mutex.mu, NULL);
    pthread_cond_init(&o->as.mutex.cv, NULL);
    LXValue v; v.type = PX_MUTEX; v.as.obj = o;
    gc_register(o, sizeof(LXObject));
    return v;
}

LXValue px_mutex_lock(LXValue m) {
    LXObject* o = px_mutex_obj(m, "lock");
    pthread_mutex_lock(&o->as.mutex.mu);
    while (o->as.mutex.locked) pthread_cond_wait(&o->as.mutex.cv, &o->as.mutex.mu);
    o->as.mutex.locked = 1;
    pthread_mutex_unlock(&o->as.mutex.mu);
    return px_null();
}

LXValue px_mutex_try_lock(LXValue m) {
    LXObject* o = px_mutex_obj(m, "try_lock");
    pthread_mutex_lock(&o->as.mutex.mu);
    int ok = 0;
    if (!o->as.mutex.locked) { o->as.mutex.locked = 1; ok = 1; }
    pthread_mutex_unlock(&o->as.mutex.mu);
    return px_bool(ok);
}

LXValue px_mutex_unlock(LXValue m) {
    LXObject* o = px_mutex_obj(m, "unlock");
    pthread_mutex_lock(&o->as.mutex.mu);
    o->as.mutex.locked = 0;
    pthread_cond_signal(&o->as.mutex.cv);
    pthread_mutex_unlock(&o->as.mutex.mu);
    return px_null();
}

static LXObject* px_rwlock_obj(LXValue m, const char* op) {
    if (m.type != PX_RWLOCK) px_error("%s: 目标不是读写锁（%s）", op, px_type_name(m));
    return m.as.obj;
}

LXValue px_rwlock_create(void) {
    LXObject* o = xcalloc(1, sizeof(LXObject));
    o->type = PX_RWLOCK;
    o->as.rwlock.readers = 0;
    o->as.rwlock.writer = 0;
    o->as.rwlock.writer_waiting = 0;
    pthread_mutex_init(&o->as.rwlock.mu, NULL);
    pthread_cond_init(&o->as.rwlock.cv, NULL);
    LXValue v; v.type = PX_RWLOCK; v.as.obj = o;
    gc_register(o, sizeof(LXObject));
    return v;
}

LXValue px_rwlock_rlock(LXValue m) {
    LXObject* o = px_rwlock_obj(m, "rlock");
    pthread_mutex_lock(&o->as.rwlock.mu);
    while (o->as.rwlock.writer || o->as.rwlock.writer_waiting > 0)
        pthread_cond_wait(&o->as.rwlock.cv, &o->as.rwlock.mu);
    o->as.rwlock.readers++;
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return px_null();
}

LXValue px_rwlock_try_rlock(LXValue m) {
    LXObject* o = px_rwlock_obj(m, "try_rlock");
    pthread_mutex_lock(&o->as.rwlock.mu);
    int ok = 0;
    if (!o->as.rwlock.writer && o->as.rwlock.writer_waiting == 0) {
        o->as.rwlock.readers++;
        ok = 1;
    }
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return px_bool(ok);
}

LXValue px_rwlock_runlock(LXValue m) {
    LXObject* o = px_rwlock_obj(m, "runlock");
    pthread_mutex_lock(&o->as.rwlock.mu);
    if (o->as.rwlock.readers > 0) o->as.rwlock.readers--;
    if (o->as.rwlock.readers == 0) pthread_cond_broadcast(&o->as.rwlock.cv);
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return px_null();
}

LXValue px_rwlock_wlock(LXValue m) {
    LXObject* o = px_rwlock_obj(m, "wlock");
    pthread_mutex_lock(&o->as.rwlock.mu);
    o->as.rwlock.writer_waiting++;
    while (o->as.rwlock.writer || o->as.rwlock.readers > 0)
        pthread_cond_wait(&o->as.rwlock.cv, &o->as.rwlock.mu);
    o->as.rwlock.writer_waiting--;
    o->as.rwlock.writer = 1;
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return px_null();
}

LXValue px_rwlock_try_wlock(LXValue m) {
    LXObject* o = px_rwlock_obj(m, "try_wlock");
    pthread_mutex_lock(&o->as.rwlock.mu);
    int ok = 0;
    if (!o->as.rwlock.writer && o->as.rwlock.readers == 0) {
        o->as.rwlock.writer = 1;
        ok = 1;
    }
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return px_bool(ok);
}

LXValue px_rwlock_wunlock(LXValue m) {
    LXObject* o = px_rwlock_obj(m, "wunlock");
    pthread_mutex_lock(&o->as.rwlock.mu);
    o->as.rwlock.writer = 0;
    pthread_cond_broadcast(&o->as.rwlock.cv);
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return px_null();
}

LXValue px_chan_create(int cap) {
    LXObject* o = xcalloc(1, sizeof(LXObject));
    o->type = PX_CHAN;
    o->as.chan.cap = cap;
    o->as.chan.len = 0;
    o->as.chan.head = 0;
    o->as.chan.closed = 0;
    o->as.chan.recv_waiting = 0;
    int phys = cap > 0 ? cap : 1;  // 无缓冲也保留 1 个交付槽
    o->as.chan.buf = xcalloc(phys, sizeof(LXValue));
    pthread_mutex_init(&o->as.chan.mu, NULL);
    pthread_cond_init(&o->as.chan.cv_send, NULL);
    pthread_cond_init(&o->as.chan.cv_recv, NULL);
    LXValue v; v.type = PX_CHAN; v.as.obj = o;
    gc_register(o, sizeof(LXObject) + (size_t)phys * sizeof(LXValue));
    return v;
}

LXValue px_chan_send(LXValue ch, LXValue val) {
    if (ch.type != PX_CHAN) px_error("send: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    pthread_mutex_lock(&o->as.chan.mu);
    while (1) {
        if (o->as.chan.closed) {
            pthread_mutex_unlock(&o->as.chan.mu);
            px_error("R1011: 向已关闭的通道发送");
        }
        if (o->as.chan.cap == 0) {
            // 无缓冲：等待接收者就绪
            if (o->as.chan.recv_waiting > 0) {
                // M22 修复：写 buf 元素（LXValue 多字节非原子）期间屏蔽 GC 暂停信号，
                // 防 GC 扫描 chan 读到半写入值 → 活跃对象漏标被误回收
                sigset_t old;
                gc_block_stop(&old);
                o->as.chan.buf[0] = val;
                o->as.chan.len = 1;
                gc_unblock_stop(&old);
                pthread_cond_signal(&o->as.chan.cv_recv);
                pthread_mutex_unlock(&o->as.chan.mu);
                px_select_signal();
                return val;
            }
            pthread_cond_wait(&o->as.chan.cv_send, &o->as.chan.mu);
        } else {
            // 有缓冲：满则等待
            if (o->as.chan.len < o->as.chan.cap) {
                int tail = (o->as.chan.head + o->as.chan.len) % o->as.chan.cap;
                sigset_t old;
                gc_block_stop(&old);
                o->as.chan.buf[tail] = val;
                o->as.chan.len++;
                gc_unblock_stop(&old);
                pthread_cond_signal(&o->as.chan.cv_recv);
                pthread_mutex_unlock(&o->as.chan.mu);
                px_select_signal();
                return val;
            }
            pthread_cond_wait(&o->as.chan.cv_send, &o->as.chan.mu);
        }
    }
}

LXValue px_chan_recv(LXValue ch) {
    if (ch.type != PX_CHAN) px_error("recv: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    pthread_mutex_lock(&o->as.chan.mu);
    while (1) {
        if (o->as.chan.len > 0) {
            LXValue v = o->as.chan.buf[o->as.chan.head];
            if (o->as.chan.cap == 0) {
                // 无缓冲：清空交付槽，通知等待的发送者
                o->as.chan.len = 0;
                if (o->as.chan.recv_waiting > 0) o->as.chan.recv_waiting--;
                pthread_cond_signal(&o->as.chan.cv_send);
            } else {
                o->as.chan.head = (o->as.chan.head + 1) % o->as.chan.cap;
                o->as.chan.len--;
                pthread_cond_signal(&o->as.chan.cv_send);
            }
            pthread_mutex_unlock(&o->as.chan.mu);
            px_select_signal();
            return v;
        }
        if (o->as.chan.closed) {
            pthread_mutex_unlock(&o->as.chan.mu);
            px_error("R1011: 从已关闭且为空的通道接收");
        }
        if (o->as.chan.cap == 0) o->as.chan.recv_waiting++;
        pthread_cond_wait(&o->as.chan.cv_recv, &o->as.chan.mu);
        if (o->as.chan.cap == 0 && o->as.chan.len == 0 && !o->as.chan.closed) {
            // 被唤醒但值被别的接收者取走（竞争），继续等待
        }
    }
}

bool px_chan_try_recv(LXValue ch, LXValue* out) {
    if (ch.type != PX_CHAN) px_error("recv: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    bool ok = false;
    pthread_mutex_lock(&o->as.chan.mu);
    if (o->as.chan.len > 0) {
        *out = o->as.chan.buf[o->as.chan.head];
        if (o->as.chan.cap == 0) {
            o->as.chan.len = 0;
            if (o->as.chan.recv_waiting > 0) o->as.chan.recv_waiting--;
        } else {
            o->as.chan.head = (o->as.chan.head + 1) % o->as.chan.cap;
            o->as.chan.len--;
        }
        pthread_cond_signal(&o->as.chan.cv_send);
        ok = true;
    }
    pthread_mutex_unlock(&o->as.chan.mu);
    if (ok) px_select_signal();
    return ok;
}

void px_chan_close(LXValue ch) {
    if (ch.type != PX_CHAN) px_error("close: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    pthread_mutex_lock(&o->as.chan.mu);
    o->as.chan.closed = 1;
    pthread_cond_broadcast(&o->as.chan.cv_send);
    pthread_cond_broadcast(&o->as.chan.cv_recv);
    pthread_mutex_unlock(&o->as.chan.mu);
    px_select_signal();
}

// ==================== spawn（pthread） ====================

typedef struct {
    LXFuncPtr fn;
    LXValue* args;
    int nargs;
} SpawnJob;

static void* spawn_thread(void* p) {
    SpawnJob* job = (SpawnJob*)p;
    // M11 修复①（创建窗口）：新线程自注册——槽位已由 px_spawn 预留（in_use=1, tid=0）。
    // 必须在调用 fn（分配/持有对象）之前把真实 tid 写入槽位；否则 GC 会因
    // "tid==0 创建中"跳过本线程，而它已在运行普贤代码 → 其栈上对象被 sweep 误回收
    // （use-after-free，即此前偶发 SIGSEGV 的根因）。
    pthread_mutex_lock(&g_gc_mu);
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (g_threads[i].in_use && (uintptr_t)g_threads[i].tid == 0) {
            g_threads[i].tid = pthread_self();
            break;
        }
    }
    pthread_mutex_unlock(&g_gc_mu);
    job->fn(job->args, job->nargs, NULL);
    // M11 修复②（退出窗口）：先持锁注销（活跃计数减一 + 槽位清空），再释放 job 内存。
    // 保证"仍持有普贤对象"的阶段始终在注册表内被 GC 暂停/扫描；注销后本线程不再被
    // 扫描，但只做 xfree（不创建/使用普贤对象），安全。原实现先 xfree 再等锁注销，
    // 存在"线程已退出但槽位未清"窗口：GC 对半退出线程 pthread_kill 返回 ESRCH 被忽略
    // → 漏暂停，或 pthread_getattr_np 读到已回收的栈 → SIGSEGV。
    pthread_mutex_lock(&g_gc_mu);
    g_active_threads--;
    gc_unregister_thread(pthread_self());
    pthread_mutex_unlock(&g_gc_mu);
    xfree(job->args);
    xfree(job);
    return NULL;
}

void px_spawn(LXFuncPtr fn, LXValue* args, int nargs) {
    pthread_mutex_lock(&g_gc_mu);   // M8：创建前先标记活跃（防止主线程 GC 误判）
    if (!g_gc_env_inited) gc_init_env();
    g_active_threads++;
    // M11：同一临界区预留线程槽位（tid=0 表示创建中，GC 视为无需暂停）
    int slot = -1;
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (!g_threads[i].in_use) { slot = i; break; }
    }
    if (slot >= 0) {
        g_threads[slot].tid = (pthread_t)0;
        g_threads[slot].in_use = 1;
        g_threads[slot].paused = 0;
        g_threads[slot].is_main = 0;
        g_threads[slot].epoch = 0;
        g_threads[slot].tmp_root = NULL;
    } else {
        // M11 修复③：槽位满时拒绝创建（而非让未注册线程裸奔——它不会被暂停/扫描，
        // 栈上对象会被误回收）。g_active_threads 已 ++，回滚后报错。
        g_active_threads--;
    }
    pthread_mutex_unlock(&g_gc_mu);
    if (slot < 0) px_error("spawn: 并发线程数超出上限 %d", MAX_SPAWN_THREADS);
    SpawnJob* job = xmalloc(sizeof(SpawnJob));
    job->fn = fn;
    job->nargs = nargs;
    job->args = xmalloc(sizeof(LXValue) * (nargs > 0 ? nargs : 1));
    if (nargs > 0) memcpy(job->args, args, sizeof(LXValue) * nargs);
    pthread_t t;
    if (pthread_create(&t, NULL, spawn_thread, job) != 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_active_threads--;
        if (slot >= 0) g_threads[slot].in_use = 0;
        pthread_mutex_unlock(&g_gc_mu);
        px_error("spawn: 创建线程失败");
    }
    if (slot >= 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_threads[slot].tid = t;
        pthread_mutex_unlock(&g_gc_mu);
    }
    pthread_detach(t);
}

void px_spawn_name(const char* fname, LXValue* args, int nargs) {
    LXValue fn = px_get_global(fname);
    if (fn.type == PX_FUNC) {
        px_spawn(fn.as.obj->as.func.fn, args, nargs);
    } else if (fn.type == PX_NATIVE) {
        px_spawn(fn.as.obj->as.native.fn, args, nargs);
    } else {
        px_error("spawn: 未找到函数 %s", fname);
    }
}

// ==================== M18 后台定时任务 / 定时器原语 ====================
// set_timeout(fn, ms, ...args)：一次性定时器；set_interval(fn, ms, ...args)：周期定时器
// clear_timer(id)：取消（返回是否取消成功）。
// 每个定时器一个 pthread（注册进 GC 槽位）：回调执行期间可被并发 GC 暂停/扫描；
// 线程栈上持有 fn/args 副本 → 回调函数与参数对象保持可达，不会被 sweep 误回收。
// 固定节奏：sleep 在循环顶部，回调执行耗时不计入间隔（不堆积）。

#define MAX_TIMERS 128
typedef struct {
    int64_t id;      // >0 有效；0 = 空槽
    int active;      // 1=生效中；clear_timer 置 0（取消标记）
} TimerSlot;
static TimerSlot g_timers[MAX_TIMERS];
static pthread_mutex_t g_timer_mu = PTHREAD_MUTEX_INITIALIZER;
static int64_t g_next_timer_id = 0;

typedef struct {
    int64_t id;
    int periodic;    // 1=interval，0=timeout
    int64_t ms;
    LXValue fn;      // 回调函数值
    LXValue* args;   // 调用参数（堆；线程栈上做副本保证 GC 可达）
    int nargs;
} TimerJob;

static void timer_sleep_ms(int64_t ms) {
    struct timespec req, rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000L;
    while (req.tv_sec > 0 || req.tv_nsec > 0) {
        if (nanosleep(&req, &rem) == 0) break;
        req = rem;   // EINTR（含 GC 暂停信号）→ 继续睡剩余时间
    }
}

// 查询定时器是否仍生效（1=继续，0=被取消/槽已释放）
static int timer_still_active(int64_t id) {
    pthread_mutex_lock(&g_timer_mu);
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (g_timers[i].id == id) {
            int a = g_timers[i].active;
            pthread_mutex_unlock(&g_timer_mu);
            return a;
        }
    }
    pthread_mutex_unlock(&g_timer_mu);
    return 0;
}

// 释放定时器槽位（timeout 执行完毕 / 定时器线程退出）
static void timer_release_slot(int64_t id) {
    pthread_mutex_lock(&g_timer_mu);
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (g_timers[i].id == id) {
            g_timers[i].active = 0;
            g_timers[i].id = 0;
            break;
        }
    }
    pthread_mutex_unlock(&g_timer_mu);
}

static void* timer_thread(void* p) {
    TimerJob* job = (TimerJob*)p;
    // M11：自注册真实 tid 到 GC 槽位（槽位已由 timer_create 预留 in_use=1, tid=0）
    pthread_mutex_lock(&g_gc_mu);
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (g_threads[i].in_use && (uintptr_t)g_threads[i].tid == 0) {
            g_threads[i].tid = pthread_self();
            break;
        }
    }
    pthread_mutex_unlock(&g_gc_mu);

    // 栈上持有 fn/args 副本：GC 保守扫描本线程栈时回调对象保持可达
    LXValue fn = job->fn;
    LXValue args_stack[16];
    int nargs = job->nargs < 16 ? job->nargs : 16;
    for (int i = 0; i < nargs; i++) args_stack[i] = job->args[i];
    int64_t id = job->id;
    int periodic = job->periodic;

    do {
        timer_sleep_ms(job->ms);
        if (!timer_still_active(id)) break;   // 取消检查（每次 tick 前）
        LXValue r = px_call(fn, args_stack, nargs);
        (void)r;
    } while (periodic);

    timer_release_slot(id);

    // 注销：先持锁再释放 job（与 spawn_thread 相同的退出窗口处理）
    pthread_mutex_lock(&g_gc_mu);
    g_active_threads--;
    gc_unregister_thread(pthread_self());
    pthread_mutex_unlock(&g_gc_mu);
    xfree(job->args);
    xfree(job);
    return NULL;
}

// 创建定时器：periodic=1 周期 / 0 一次性；返回定时器 id
static int64_t px_timer_create(int periodic, LXValue fn, LXValue* args, int nargs, int64_t ms) {
    pthread_mutex_lock(&g_timer_mu);
    int slot = -1;
    for (int i = 0; i < MAX_TIMERS; i++) if (g_timers[i].id == 0) { slot = i; break; }
    if (slot < 0) {
        pthread_mutex_unlock(&g_timer_mu);
        px_error("定时器数量超出上限 %d", MAX_TIMERS);
    }
    int64_t id = ++g_next_timer_id;
    g_timers[slot].id = id;
    g_timers[slot].active = 1;
    pthread_mutex_unlock(&g_timer_mu);

    // GC 槽位预留（同 px_spawn：同一临界区内 g_active_threads++ + 预留槽位）
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    g_active_threads++;
    int gslot = -1;
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) if (!g_threads[i].in_use) { gslot = i; break; }
    if (gslot >= 0) {
        g_threads[gslot].tid = (pthread_t)0;
        g_threads[gslot].in_use = 1;
        g_threads[gslot].paused = 0;
        g_threads[gslot].is_main = 0;
        g_threads[gslot].epoch = 0;
        g_threads[gslot].tmp_root = NULL;
    } else {
        g_active_threads--;   // 槽位满回滚
    }
    pthread_mutex_unlock(&g_gc_mu);
    if (gslot < 0) px_error("定时器: 并发线程数超出上限 %d", MAX_SPAWN_THREADS);

    TimerJob* job = xmalloc(sizeof(TimerJob));
    job->id = id;
    job->periodic = periodic;
    job->ms = ms;
    job->fn = fn;
    job->nargs = nargs;
    job->args = xmalloc(sizeof(LXValue) * (nargs > 0 ? nargs : 1));
    if (nargs > 0) memcpy(job->args, args, sizeof(LXValue) * nargs);

    pthread_t t;
    if (pthread_create(&t, NULL, timer_thread, job) != 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_active_threads--;
        if (gslot >= 0) g_threads[gslot].in_use = 0;
        pthread_mutex_unlock(&g_gc_mu);
        pthread_mutex_lock(&g_timer_mu);
        g_timers[slot].id = 0;
        pthread_mutex_unlock(&g_timer_mu);
        px_error("定时器: 创建线程失败");
    }
    if (gslot >= 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_threads[gslot].tid = t;
        pthread_mutex_unlock(&g_gc_mu);
    }
    pthread_detach(t);
    return id;
}

static LXValue bi_set_timeout(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2) px_error("set_timeout 需要 (fn, ms[, ...args]) 参数");
    if (args[0].type != PX_FUNC && args[0].type != PX_NATIVE)
        px_error("set_timeout: 第一个参数必须是函数");
    int64_t ms = int_val(args[1]);
    if (ms < 0) px_error("set_timeout: 间隔不能为负数");
    int64_t id = px_timer_create(0, args[0], args + 2, nargs - 2, ms);
    return px_int(id);
}

static LXValue bi_set_interval(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2) px_error("set_interval 需要 (fn, ms[, ...args]) 参数");
    if (args[0].type != PX_FUNC && args[0].type != PX_NATIVE)
        px_error("set_interval: 第一个参数必须是函数");
    int64_t ms = int_val(args[1]);
    if (ms < 0) px_error("set_interval: 间隔不能为负数");
    int64_t id = px_timer_create(1, args[0], args + 2, nargs - 2, ms);
    return px_int(id);
}

static LXValue bi_clear_timer(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("clear_timer 需要 1 个参数");
    int64_t id = int_val(args[0]);
    pthread_mutex_lock(&g_timer_mu);
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (g_timers[i].id == id && g_timers[i].active) {
            g_timers[i].active = 0;
            pthread_mutex_unlock(&g_timer_mu);
            return px_bool(1);
        }
    }
    pthread_mutex_unlock(&g_timer_mu);
    return px_bool(0);
}

// ==================== std.net（M5.2）：TCP + HTTP 客户端 ====================

static int sock_send_all(int fd, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        int n = (int)send(fd, data + sent, len - sent, 0);
        if (n <= 0) return -1;
        sent += n;
    }
    return sent;
}

static LXValue bi_tcp_listen(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("tcp_listen 需要 (port) 参数");
    int port = (int)args[0].as.i;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) px_error("net: 创建 socket 失败");
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        px_error("net: 监听端口 %d 失败", port);
    }
    if (listen(fd, 16) < 0) {
        close(fd);
        px_error("net: listen 失败");
    }
    return px_int(fd);
}

static LXValue bi_tcp_accept(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("tcp_accept 需要 (listener_id) 参数");
    int lfd = (int)args[0].as.i;
    struct sockaddr_in cli;
    socklen_t cli_len = sizeof(cli);
    int cfd = accept(lfd, (struct sockaddr*)&cli, &cli_len);
    if (cfd < 0) px_error("net: accept 失败");
    return px_int(cfd);
}

static LXValue bi_tcp_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_INT) px_error("tcp_connect 需要 (host, port) 参数");
    const char* host = args[0].as.obj->as.str.data;
    int port = (int)args[1].as.i;
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res) px_error("net: 解析主机失败 %s", host);
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) { freeaddrinfo(res); px_error("net: 创建 socket 失败"); }
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        int e = errno;
        freeaddrinfo(res);
        close(fd);
        px_error("net: 连接 %s:%d 失败 (%d)", host, port, e);
    }
    freeaddrinfo(res);
    return px_int(fd);
}

static LXValue bi_tcp_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("tcp_send 需要 (conn_id, data) 参数");
    int fd = (int)args[0].as.i;
    const char* data;
    int len;
    if (args[1].type == PX_STR) { data = args[1].as.obj->as.str.data; len = args[1].as.obj->as.str.len; }
    else { data = px_to_string(args[1]); len = (int)strlen(data); }
    int n = sock_send_all(fd, data, len);
    if (n < 0) px_error("net: 发送失败");
    return px_int(n);
}

static LXValue bi_tcp_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT) px_error("tcp_recv 需要 (conn_id, maxlen) 参数");
    int fd = (int)args[0].as.i;
    int maxlen = (int)args[1].as.i;
    if (maxlen <= 0) maxlen = 1;
    char* buf = xmalloc(maxlen + 1);
    int n = (int)recv(fd, buf, maxlen, 0);
    if (n <= 0) { xfree(buf); return px_str(""); }
    buf[n] = 0;
    LXValue r = px_str_len(buf, n);
    xfree(buf);
    return r;
}

static LXValue bi_tcp_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("tcp_close 需要 (id) 参数");
    close((int)args[0].as.i);
    return px_null();
}

// ==================== M23c/M24 HTTP keep-alive 连接池 + 客户端（双模式：与解释器 builtin.rs 一致） ====================
// http_request(url, method, body?, headers?) → dict{status, headers, body}
// http_get_stream(url, chunk_handler) → bool（流式下载分块回调）
// M24：https 也池化——已握手的 mbedtls TLS 会话（含 conf/ctr_drbg/entropy 生命周期）随槽位缓存复用。
#define HTTP_POOL_MAX_HOSTS 32
#define HTTP_POOL_PER_HOST 4

// TLS 会话（建立连接即完成握手；可跨请求复用）
typedef struct {
    mbedtls_net_context net;         // TCP fd
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    char skey[256];                  // M25：host:port（关闭时保存会话票据用）
} HttpsSession;

// ==================== M25 TLS 会话票据恢复（RFC 5077 session ticket） ====================
// 连接池复用的是"活着"的 TLS 连接；连接关闭（服务器断开/池满淘汰/请求失败）后，
// 下次同 host 新建连接若走完整握手，多 1~2 个 RTT。M25 保存每个 host:port 最近一次
// 协商的 mbedtls_ssl_session（含 session ticket），新建连接时 set_session 尝试恢复握手
// （服务器不支持/票据过期 → mbedtls 自动回退完整握手，安全性不变）。
// Rust 端（解释器）：全局共享 Arc<ClientConfig>（rustls 会话存储随 Arc 共享）→ 同样恢复。

#define HTTPS_SAVED_MAX 32
typedef struct {
    char key[256];
    mbedtls_ssl_session session;
    int has;
} HSavedTls;
static HSavedTls g_saved_tls[HTTPS_SAVED_MAX];
static pthread_mutex_t g_saved_tls_mu = PTHREAD_MUTEX_INITIALIZER;

// M25 诊断（定义在下方；此处前置声明供 saved_tls_store 使用）
static void px_tls_debug(const char* fmt, ...);

// mbedtls 3.6 的 mbedtls_ssl_session_copy 为内部导出符号（未在公共头声明）；
// 此处按源码原型补前置声明（libmbedtls.a 已含该符号），避免隐式声明告警。
int mbedtls_ssl_session_copy(mbedtls_ssl_session* dst, const mbedtls_ssl_session* src);

// 保存某 host 的会话（拷贝语义；覆盖旧值）
static void saved_tls_store(const char* key, const mbedtls_ssl_session* s) {
    pthread_mutex_lock(&g_saved_tls_mu);
    int slot = -1;
    for (int i = 0; i < HTTPS_SAVED_MAX; i++) {
        if (g_saved_tls[i].has && strcmp(g_saved_tls[i].key, key) == 0) { slot = i; break; }
        if (slot < 0 && !g_saved_tls[i].has) slot = i;
    }
    if (slot < 0) { pthread_mutex_unlock(&g_saved_tls_mu); return; }
    if (!g_saved_tls[slot].has) {
        mbedtls_ssl_session_init(&g_saved_tls[slot].session);
        strncpy(g_saved_tls[slot].key, key, 255);
        g_saved_tls[slot].key[255] = 0;
    } else {
        mbedtls_ssl_session_free(&g_saved_tls[slot].session);
        mbedtls_ssl_session_init(&g_saved_tls[slot].session);
    }
    if (mbedtls_ssl_session_copy(&g_saved_tls[slot].session, s) == 0) {
        g_saved_tls[slot].has = 1;
        px_tls_debug("tls %s 已保存会话票据", key);
    }
    pthread_mutex_unlock(&g_saved_tls_mu);
}// 取出某 host 的会话（拷贝到 out；无则返回 -1）
static int saved_tls_take(const char* key, mbedtls_ssl_session* out) {
    pthread_mutex_lock(&g_saved_tls_mu);
    int rc = -1;
    for (int i = 0; i < HTTPS_SAVED_MAX; i++) {
        if (g_saved_tls[i].has && strcmp(g_saved_tls[i].key, key) == 0) {
            if (mbedtls_ssl_session_copy(out, &g_saved_tls[i].session) == 0) rc = 0;
            break;
        }
    }
    pthread_mutex_unlock(&g_saved_tls_mu);
    return rc;
}

// M25 诊断：PX_TLS_DEBUG=1 时打印 TLS 会话票据行为（不影响正常输出）
static void px_tls_debug(const char* fmt, ...) {
    if (!getenv("PX_TLS_DEBUG")) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

// 建立 HTTPS 连接（TCP + TLS 握手完成；尝试会话票据恢复）；失败返回 NULL
static HttpsSession* https_connect(const char* host, int port) {
    HttpsSession* s = (HttpsSession*)xmalloc(sizeof(HttpsSession));
    memset(s, 0, sizeof(*s));
    mbedtls_net_init(&s->net);
    mbedtls_ssl_init(&s->ssl);
    mbedtls_ssl_config_init(&s->conf);
    mbedtls_ctr_drbg_init(&s->ctr_drbg);
    mbedtls_entropy_init(&s->entropy);
    const char* pers = "px_https";
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    int ret;
    if ((ret = mbedtls_ctr_drbg_seed(&s->ctr_drbg, mbedtls_entropy_func, &s->entropy,
                                     (const unsigned char*)pers, strlen(pers))) != 0) goto fail;
    if ((ret = mbedtls_net_connect(&s->net, host, portstr, MBEDTLS_NET_PROTO_TCP)) != 0) goto fail;
    px_ensure_cacert();
    if ((ret = mbedtls_ssl_config_defaults(&s->conf, MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) goto fail;
    mbedtls_ssl_conf_authmode(&s->conf, g_cacert_loaded ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(&s->conf, &g_cacert, NULL);
    mbedtls_ssl_conf_rng(&s->conf, mbedtls_ctr_drbg_random, &s->ctr_drbg);
    mbedtls_ssl_conf_min_version(&s->conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
    // M25：显式开启会话票据（TLS 1.2 静态票据；TLS 1.3 客户端自动接受 NewSessionTicket）
    mbedtls_ssl_conf_session_tickets(&s->conf, MBEDTLS_SSL_SESSION_TICKETS_ENABLED);
    if ((ret = mbedtls_ssl_setup(&s->ssl, &s->conf)) != 0) goto fail;
    mbedtls_ssl_set_hostname(&s->ssl, host);
    mbedtls_ssl_set_bio(&s->ssl, &s->net, mbedtls_net_send, mbedtls_net_recv, NULL);
    // M25：尝试用保存的会话票据恢复握手（服务器拒绝则自动完整握手）
    char skey[300];
    snprintf(skey, sizeof(skey), "%s:%d", host, port);
    snprintf(s->skey, sizeof(s->skey), "%s", skey);
    mbedtls_ssl_session saved;
    mbedtls_ssl_session_init(&saved);
    if (saved_tls_take(skey, &saved) == 0) {
        mbedtls_ssl_set_session(&s->ssl, &saved);
        px_tls_debug("tls %s 恢复握手（已存票据）", skey);
    }
    mbedtls_ssl_session_free(&saved);
    int guard = 0;
    while ((ret = mbedtls_ssl_handshake(&s->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) goto fail;
        if (++guard > 40) goto fail;
    }
    // M25：握手完成即保存本次会话（供下次连接恢复；TLS 1.3 票据在 close 时再补存）
    {
        mbedtls_ssl_session fresh;
        mbedtls_ssl_session_init(&fresh);
        if (mbedtls_ssl_get_session(&s->ssl, &fresh) == 0) {
            saved_tls_store(skey, &fresh);
        }
        mbedtls_ssl_session_free(&fresh);
    }
    return s;
fail:
    mbedtls_net_free(&s->net);
    mbedtls_ssl_free(&s->ssl);
    mbedtls_ssl_config_free(&s->conf);
    mbedtls_ctr_drbg_free(&s->ctr_drbg);
    mbedtls_entropy_free(&s->entropy);
    xfree(s);
    return NULL;
}

static void https_close(HttpsSession* s) {
    if (!s) return;
    // M25：关闭前补存会话（TLS 1.3 NewSessionTicket 常在首轮读时到达）
    if (s->skey[0]) {
        mbedtls_ssl_session cur;
        mbedtls_ssl_session_init(&cur);
        if (mbedtls_ssl_get_session(&s->ssl, &cur) == 0) {
            saved_tls_store(s->skey, &cur);
        }
        mbedtls_ssl_session_free(&cur);
    }
    mbedtls_net_free(&s->net);
    mbedtls_ssl_free(&s->ssl);
    mbedtls_ssl_config_free(&s->conf);
    mbedtls_ctr_drbg_free(&s->ctr_drbg);
    mbedtls_entropy_free(&s->entropy);
    xfree(s);
}

// 统一发送（tls 非空走 mbedtls，否则走 fd send）
static int conn_send(HttpsSession* tls, int fd, const char* data, int len) {
    if (!tls) return sock_send_all(fd, data, len);
    int sent = 0;
    while (sent < len) {
        int w = mbedtls_ssl_write(&tls->ssl, (const unsigned char*)data + sent, (size_t)(len - sent));
        if (w == MBEDTLS_ERR_SSL_WANT_WRITE || w == MBEDTLS_ERR_SSL_WANT_READ) continue;
        if (w < 0) return -1;
        sent += w;
    }
    return sent;
}
static int conn_recv(HttpsSession* tls, int fd, char* buf, int len) {
    if (!tls) return (int)recv(fd, buf, (size_t)len, 0);
    for (;;) {
        int n = mbedtls_ssl_read(&tls->ssl, (unsigned char*)buf, (size_t)len);
        if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        return n;
    }
}

// 池槽位：明文 fd 或 TLS 会话
typedef struct {
    int is_tls;
    int fd;
    HttpsSession* tls;
} HPoolSlot;
static pthread_mutex_t g_hpool_mu = PTHREAD_MUTEX_INITIALIZER;
static struct { char key[256]; int n; HPoolSlot slots[HTTP_POOL_PER_HOST]; } g_hpool[HTTP_POOL_MAX_HOSTS];

static int hpool_take(const char* key, HPoolSlot* out) {
    pthread_mutex_lock(&g_hpool_mu);
    for (int i = 0; i < HTTP_POOL_MAX_HOSTS; i++) {
        if (g_hpool[i].n > 0 && strcmp(g_hpool[i].key, key) == 0) {
            *out = g_hpool[i].slots[--g_hpool[i].n];
            pthread_mutex_unlock(&g_hpool_mu);
            return 0;
        }
    }
    pthread_mutex_unlock(&g_hpool_mu);
    return -1;
}
static void hpool_put(const char* key, HPoolSlot s) {
    pthread_mutex_lock(&g_hpool_mu);
    for (int i = 0; i < HTTP_POOL_MAX_HOSTS; i++) {
        if (strcmp(g_hpool[i].key, key) == 0 || g_hpool[i].n == 0) {
            if (g_hpool[i].n == 0) { strncpy(g_hpool[i].key, key, 255); g_hpool[i].key[255] = 0; }
            if (g_hpool[i].n < HTTP_POOL_PER_HOST) {
                g_hpool[i].slots[g_hpool[i].n++] = s;
            } else {
                if (s.tls) https_close(s.tls);
                close(s.fd);
            }
            pthread_mutex_unlock(&g_hpool_mu);
            return;
        }
    }
    pthread_mutex_unlock(&g_hpool_mu);
    if (s.tls) https_close(s.tls);
    close(s.fd);
}

// 解析 http(s) URL：is_https / host / port / path
static void hparse_url(const char* url, int* is_https, char* host, int host_cap, int* port, const char** path) {
    *is_https = 0;
    const char* rest = url;
    if (strncmp(url, "https://", 8) == 0) { *is_https = 1; rest = url + 8; }
    else if (strncmp(url, "http://", 7) == 0) { rest = url + 7; }
    else px_error("net: 不支持的协议: %s", url);
    int hl = 0;
    while (rest[hl] && rest[hl] != '/' && rest[hl] != ':' && hl < host_cap - 1) { host[hl] = rest[hl]; hl++; }
    host[hl] = 0;
    *port = *is_https ? 443 : 80;
    const char* p = rest + hl;
    if (*p == ':') {
        *port = atoi(p + 1);
        const char* q = p + 1;
        while (*q && *q != '/') q++;
        p = q;
    }
    *path = (*p == '/') ? p : "/";
}

// 建立 TCP 连接（域名解析），返回 fd；失败返回 -1
static int hconnect(const char* host, int port) {
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res) return -1;
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) { freeaddrinfo(res); return -1; }
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) { freeaddrinfo(res); close(fd); return -1; }
    freeaddrinfo(res);
    return fd;
}

// 在已建立连接上完成一次 HTTP 往返（keep-alive 安全：按 Content-Length/chunked 精确读）
// 支持明文 fd 与 TLS 会话（M24 https 连接池）。成功返回 0，body malloc；失败返回 -1。
static int h_exchange(HPoolSlot* slot, const char* req, int rlen,
                      int* out_status, LXValue* out_headers,
                      char** out_body, int* out_body_len, int* out_keep_alive) {
    int fd = slot->fd;
    HttpsSession* tls = slot->is_tls ? slot->tls : NULL;
    if (conn_send(tls, fd, req, rlen) < 0) return -1;
    // 读响应头
    int cap = 16384, len = 0;
    char* buf = xmalloc(cap);
    int header_end = -1;
    for (;;) {
        if (len + 4096 > cap) { cap *= 2; buf = xrealloc(buf, cap); }
        int n = conn_recv(tls, fd, buf + len, 4096);
        if (n <= 0) { xfree(buf); return -1; }
        len += n;
        buf[len] = 0;
        char* sep = strstr(buf, "\r\n\r\n");
        if (sep) { header_end = (int)(sep - buf); break; }
        if (len > 65536) { xfree(buf); return -1; }
    }
    // 解析状态码 + 头部
    int status = 0;
    sscanf(buf, "HTTP/%*s %d", &status);
    *out_headers = px_dict();
    int chunked = 0, gzip = 0, keep_alive = 1;
    int content_length = -1;
    char* hline = buf;
    char* hend = buf + header_end;
    while (hline < hend) {
        char* eol = strstr(hline, "\r\n");
        if (!eol || eol > hend) break;
        int linelen = (int)(eol - hline);
        char line[4096];
        int cl = linelen < 4095 ? linelen : 4095;
        memcpy(line, hline, (size_t)cl);
        line[cl] = 0;
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            char* k = line;
            char* v = colon + 1;
            while (*v == ' ') v++;
            char* ve = v + strlen(v);
            while (ve > v && (ve[-1] == ' ' || ve[-1] == '\r')) ve--;
            *ve = 0;
            if (strcasecmp(k, "Content-Length") == 0) content_length = atoi(v);
            if (strcasecmp(k, "Transfer-Encoding") == 0 && strstr(v, "chunked")) chunked = 1;
            if (strcasecmp(k, "Content-Encoding") == 0 && strstr(v, "gzip")) gzip = 1;
            if (strcasecmp(k, "Connection") == 0 && strcasecmp(v, "close") == 0) keep_alive = 0;
            px_dict_set(*out_headers, k, px_str(v));
        }
        hline = eol + 2;
    }
    // 读 body
    char* body_buf = NULL;
    int body_len = 0;
    int have = len - (header_end + 4);
    if (chunked) {
        // 继续读直到 "0\r\n\r\n"
        int bl = have > 0 ? have : 0;
        if (bl > 0) { body_buf = xmalloc(bl); memcpy(body_buf, buf + header_end + 4, (size_t)bl); }
        for (;;) {
            if (body_buf && bl >= 5 && memcmp(body_buf + bl - 5, "0\r\n\r\n", 5) == 0) break;
            body_buf = xrealloc(body_buf, bl + 4096);
            int n = conn_recv(tls, fd, body_buf + bl, 4096);
            if (n <= 0) break;
            bl += n;
        }
        char* dec = px_chunked_decode(body_buf, bl, &body_len);
        if (dec) { xfree(body_buf); body_buf = dec; }
    } else if (content_length >= 0) {
        int copied = have > 0 ? (have < content_length ? have : content_length) : 0;
        body_buf = xmalloc((size_t)content_length + 1);
        if (copied > 0) memcpy(body_buf, buf + header_end + 4, (size_t)copied);
        while (copied < content_length) {
            int n = conn_recv(tls, fd, body_buf + copied, (size_t)(content_length - copied));
            if (n <= 0) break;
            copied += n;
        }
        body_len = copied;
        body_buf[body_len] = 0;
    } else {
        // 无长度：读到 EOF（连接将关闭）
        int bl = have > 0 ? have : 0;
        if (bl > 0) { body_buf = xmalloc(bl); memcpy(body_buf, buf + header_end + 4, (size_t)bl); }
        for (;;) {
            body_buf = xrealloc(body_buf, bl + 4096);
            int n = conn_recv(tls, fd, body_buf + bl, 4096);
            if (n <= 0) break;
            bl += n;
        }
        body_len = bl;
        if (body_buf) body_buf[body_len] = 0;
        keep_alive = 0; // 读到 EOF，连接不可复用
    }
    if (gzip) {
        char* dec = px_gzip_decompress(body_buf, body_len, &body_len);
        if (dec) { xfree(body_buf); body_buf = dec; }
    }
    if (body_buf) body_buf[body_len] = 0;
    *out_status = status;  // M27 修复：M23c 遗留——状态码解析后未回传（http_request 一直返回 0）
    *out_body = body_buf;
    *out_body_len = body_len;
    *out_keep_alive = keep_alive;
    xfree(buf);
    return 0;
}

// http_request(url, method, body?, headers?) → dict{status, headers, body}
static LXValue bi_http_request(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 4) px_error("http_request 需要 (url, method[, body[, headers]]) 参数");
    const char* url = val_cstr(args[0]);
    const char* method = val_cstr(args[1]);
    const char* body = NULL;
    if (nargs >= 3 && args[2].type == PX_STR) body = args[2].as.obj->as.str.data;
    char extra_headers[4096] = {0};
    if (nargs >= 4 && args[3].type == PX_DICT) {
        LXObject* ho = args[3].as.obj;
        int off = 0;
        for (int i = 0; i < ho->as.dict.len && off < 4095; i++) {
            if (ho->as.dict.vals[i].type != PX_STR) continue;
            off += snprintf(extra_headers + off, 4096 - (size_t)off, "%s: %s\r\n",
                            ho->as.dict.keys[i], ho->as.dict.vals[i].as.obj->as.str.data);
        }
    }
    int is_https = 0;
    char host[256];
    int port = 80;
    const char* path = "/";
    hparse_url(url, &is_https, host, sizeof(host), &port, &path);
    char key[300];
    snprintf(key, sizeof(key), "%s:%d", host, port);
    char req[16384];
    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: PuXian/0.1\r\nConnection: keep-alive\r\n",
        method, path, host, port);
    if (extra_headers[0]) { memcpy(req + rlen, extra_headers, strlen(extra_headers)); rlen += (int)strlen(extra_headers); }
    if (body) {
        if (!strcasestr(extra_headers, "Content-Length")) {
            rlen += snprintf(req + rlen, sizeof(req) - rlen,
                "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n", (int)strlen(body));
        }
    }
    rlen += snprintf(req + rlen, sizeof(req) - rlen, "\r\n");
    if (body) { memcpy(req + rlen, body, strlen(body)); rlen += (int)strlen(body); }
    for (int attempt = 0; attempt < 2; attempt++) {
        HPoolSlot slot;
        if (hpool_take(key, &slot) != 0) {
            // 池中无空闲连接：新建（http 明文 TCP；https TLS 握手）
            slot.is_tls = is_https;
            slot.fd = -1;
            slot.tls = NULL;
            if (is_https) {
                slot.tls = https_connect(host, port);
                slot.fd = slot.tls ? slot.tls->net.fd : -1;
            } else {
                slot.fd = hconnect(host, port);
            }
            if (slot.fd < 0) {
                if (attempt == 0) continue;
                px_error("net: 连接 %s:%d 失败", host, port);
            }
        }
        int status = 0, body_len = 0, keep_alive = 1;
        LXValue headers = px_null();
        char* resp_body = NULL;
        if (h_exchange(&slot, req, rlen, &status, &headers, &resp_body, &body_len, &keep_alive) == 0) {
            if (keep_alive) hpool_put(key, slot);
            else { if (slot.tls) https_close(slot.tls); close(slot.fd); }
            LXValue d = px_dict();
            px_dict_set(d, "status", px_int(status));
            px_dict_set(d, "headers", headers);
            px_dict_set(d, "body", resp_body ? px_str_len(resp_body, body_len) : px_str(""));
            if (resp_body) xfree(resp_body);
            return d;
        }
        if (slot.tls) https_close(slot.tls);
        close(slot.fd);
        if (attempt == 0) continue; // 池连接失效重试
        px_error("net: http_request 失败: 连接关闭");
    }
    return px_null();
}

// ==================== M24：流式 gzip 解压器（http_get_stream 边收边解） ====================
// 基于 miniz mz_inflate（raw deflate，-15），增量喂入压缩字节，解压输出累积到内部缓冲。
// 状态机：0=gzip 头解析 → 1=deflate → 2=尾部(CRC32+ISIZE 8 字节) → 3=完成。
typedef struct {
    mz_stream s;
    int state;        // 0 头 / 1 deflate / 2 尾 / 3 done
    int init;         // mz_inflateInit2 已调用
    int err;
    unsigned char hdr[4096];  // 头累积缓冲
    int hdr_len;
    int tail;         // 尾部已跳过字节
    unsigned char* out;
    int out_len, out_cap;
} GzStream;

static void gz_stream_init(GzStream* g) { memset(g, 0, sizeof(*g)); }
static void gz_stream_free(GzStream* g) {
    if (g->init) mz_inflateEnd(&g->s);
    if (g->out) xfree(g->out);
    g->out = NULL;
    g->out_len = g->out_cap = 0;
}

// 取走全部解压输出（缓冲清零，调用者 xfree）
static unsigned char* gz_stream_take(GzStream* g, int* len) {
    unsigned char* r = g->out;
    *len = g->out_len;
    g->out = NULL;
    g->out_len = g->out_cap = 0;
    return r;
}

static void gz_append(GzStream* g, const unsigned char* data, int n) {
    if (g->out_len + n > g->out_cap) {
        int nc = g->out_cap ? g->out_cap : 4096;
        while (nc < g->out_len + n) nc *= 2;
        g->out = (unsigned char*)xrealloc(g->out, (size_t)nc);
        g->out_cap = nc;
    }
    memcpy(g->out + g->out_len, data, (size_t)n);
    g->out_len += n;
}

// 从 hdr 缓冲计算 gzip 头总长度（含扩展字段）；数据不足返回 -1
static int gz_header_size(const unsigned char* b, int n) {
    if (n < 10) return -1;
    if (b[0] != 0x1F || b[1] != 0x8B || b[2] != 8) return -1;
    int total = 10, pos = 10;
    unsigned char flg = b[3];
    if (flg & 4) {
        if (pos + 2 > n) return -1;
        int xlen = b[pos] | (b[pos + 1] << 8);
        total += 2 + xlen; pos += 2 + xlen;
    }
    if (flg & 8) {
        int i = pos;
        while (i < n && b[i]) i++;
        if (i >= n) return -1;
        total += i - pos + 1; pos = i + 1;
    }
    if (flg & 16) {
        int i = pos;
        while (i < n && b[i]) i++;
        if (i >= n) return -1;
        total += i - pos + 1; pos = i + 1;
    }
    if (flg & 2) total += 2;
    return total;
}

// 喂输入压缩字节；解压输出累积（调用者 gz_stream_take 取走）。返回 0 正常 / -1 损坏。
static int gz_stream_feed(GzStream* g, const unsigned char* in, int inlen) {
    if (g->err) return -1;
    int ipos = 0;
    while (ipos < inlen) {
        if (g->state == 0) {
            // 累积头部直到完整
            int room = (int)sizeof(g->hdr) - g->hdr_len;
            int take = inlen - ipos;
            if (take > room) take = room;
            memcpy(g->hdr + g->hdr_len, in + ipos, (size_t)take);
            g->hdr_len += take;
            ipos += take;
            int hsize = gz_header_size(g->hdr, g->hdr_len);
            if (hsize < 0) {
                if (g->hdr_len >= (int)sizeof(g->hdr)) { g->err = 1; return -1; }
                continue; // 头还不完整，等更多数据
            }
            if (hsize > g->hdr_len) continue; // 扩展字段还不完整
            // 头完整：把头部之后的数据留出（回退给 deflate 处理）
            int extra = g->hdr_len - hsize;
            if (extra > 0) {
                memmove(g->hdr, g->hdr + hsize, (size_t)extra);
                g->hdr_len = extra;
            } else {
                g->hdr_len = 0;
            }
            if (mz_inflateInit2(&g->s, -15) != MZ_OK) { g->err = 1; return -1; }
            g->init = 1;
            g->state = 1;
            if (extra > 0) {
                // 头部后紧跟的数据：递归喂（用 hdr 缓冲中的数据）
                int r = gz_stream_feed(g, g->hdr, g->hdr_len);
                g->hdr_len = 0;
                if (r < 0) return -1;
            }
            continue;
        }
        if (g->state == 1) {
            g->s.next_in = (unsigned char*)(in + ipos);
            g->s.avail_in = (mz_ulong)(inlen - ipos);
            for (;;) {
                unsigned char tmp[65536];
                g->s.next_out = tmp;
                g->s.avail_out = sizeof(tmp);
                size_t before_in = g->s.avail_in;
                int r = mz_inflate(&g->s, MZ_NO_FLUSH);
                int produced = (int)sizeof(tmp) - (int)g->s.avail_out;
                if (produced > 0) gz_append(g, tmp, produced);
                if (r == MZ_STREAM_END) {
                    ipos = inlen - (int)g->s.avail_in;
                    g->state = 2;
                    break;
                }
                if (r != MZ_OK) { g->err = 1; return -1; }
                if (g->s.avail_in == 0) { ipos = inlen; break; }      // 输入耗尽等更多
                if (produced == 0) { g->err = 1; return -1; }          // 无进展 = 损坏
                (void)before_in;
            }
            continue;
        }
        if (g->state == 2) {
            int need = 8 - g->tail;
            int take = inlen - ipos;
            if (take > need) take = need;
            g->tail += take;
            ipos += take;
            if (g->tail >= 8) g->state = 3;
            continue;
        }
        break; // state==3 done
    }
    return 0;
}
// http_get_stream(url, chunk_handler) → bool：流式下载，分块调 handler(块文本)
// M24：支持 https（TLS 连接）+ gzip 流式解压（边收边解，不等完整下载）
static LXValue bi_http_get_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("http_get_stream 需要 (url, chunk_handler) 参数");
    const char* url = val_cstr(args[0]);
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("http_get_stream 的 chunk_handler 必须是函数");
    int is_https = 0;
    char host[256];
    int port = 80;
    const char* path = "/";
    hparse_url(url, &is_https, host, sizeof(host), &port, &path);
    HPoolSlot slot;
    slot.is_tls = is_https;
    slot.fd = -1;
    slot.tls = NULL;
    if (is_https) { slot.tls = https_connect(host, port); slot.fd = slot.tls ? slot.tls->net.fd : -1; }
    else slot.fd = hconnect(host, port);
    if (slot.fd < 0) px_error("net: 连接 %s:%d 失败", host, port);
    int fd = slot.fd;
    HttpsSession* tls = slot.is_tls ? slot.tls : NULL;
    char req[8192];
    int rlen = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n\r\n",
        path, host, port);
    if (conn_send(tls, fd, req, rlen) < 0) { if (tls) https_close(tls); else close(fd); px_error("net: 发送请求失败"); }
    // 读响应头
    int cap = 16384, len = 0;
    char* buf = xmalloc(cap);
    int header_end = -1;
    for (;;) {
        if (len + 4096 > cap) { cap *= 2; buf = xrealloc(buf, cap); }
        int n = conn_recv(tls, fd, buf + len, 4096);
        if (n <= 0) { xfree(buf); if (tls) https_close(tls); else close(fd); px_error("net: 读取响应失败"); }
        len += n;
        buf[len] = 0;
        char* sep = strstr(buf, "\r\n\r\n");
        if (sep) { header_end = (int)(sep - buf); break; }
        if (len > 65536) { xfree(buf); if (tls) https_close(tls); else close(fd); px_error("net: 响应头超过 64KB"); }
    }
    int chunked = 0, gzip = 0, content_length = -1;
    char* hline = buf;
    while (hline < buf + header_end) {
        char* eol = strstr(hline, "\r\n");
        if (!eol || eol > buf + header_end) break;
        char line[4096];
        int cl = (int)(eol - hline);
        if (cl > 4095) cl = 4095;
        memcpy(line, hline, (size_t)cl);
        line[cl] = 0;
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            char* v = colon + 1;
            while (*v == ' ') v++;
            if (strcasecmp(line, "Content-Length") == 0) content_length = atoi(v);
            if (strcasecmp(line, "Transfer-Encoding") == 0 && strstr(v, "chunked")) chunked = 1;
            if (strcasecmp(line, "Content-Encoding") == 0 && strstr(v, "gzip")) gzip = 1;
        }
        hline = eol + 2;
    }
    // 流式读 body 并分块回调
    bool complete = true;
    int pending_off = header_end + 4;
    int pending_len = len - pending_off;
    char* pending = NULL;
    if (pending_len > 0) { pending = xmalloc(pending_len); memcpy(pending, buf + pending_off, (size_t)pending_len); }
    // M24：gzip 流式解压器
    GzStream gz;
    int gz_active = gzip ? 1 : 0;
    if (gz_active) gz_stream_init(&gz);
    char* obuf = xmalloc(65536);
    int olen = 0;

    // 输出缓冲满 64KB → 回调 handler
#define STREAM_FLUSH() do { \
        if (olen > 0) { \
            LXValue arg = px_str_len(obuf, olen); \
            LXValue rv = px_call(handler, &arg, 1); \
            if (rv.type == PX_BOOL && !rv.as.b) { complete = false; } \
            olen = 0; \
        } \
    } while (0)
    // 喂 gzip 解压器 → 解压输出累积到 obuf 并 flush
#define STREAM_FEED_GZ(data, n) do { \
        if (gz_stream_feed(&gz, (const unsigned char*)(data), (n)) < 0) { complete = false; goto stream_done; } \
        if (gz.out_len > 0) { \
            int tlen = 0; \
            unsigned char* t = gz_stream_take(&gz, &tlen); \
            for (int ti = 0; ti < tlen; ti++) { \
                obuf[olen++] = (char)t[ti]; \
                if (olen >= 65536) STREAM_FLUSH(); \
            } \
            xfree(t); \
        } \
    } while (0)

    if (chunked) {
        // chunked：按 chunk 边界切块；gzip 时块喂给解压器
        for (;;) {
            int ci = -1;
            for (int i = 0; i + 1 < pending_len; i++) {
                if (pending[i] == '\r' && pending[i + 1] == '\n') { ci = i; break; }
            }
            if (ci >= 0) {
                char sz[32];
                int sl = ci < 31 ? ci : 31;
                memcpy(sz, pending, (size_t)sl);
                sz[sl] = 0;
                int csize = (int)strtol(sz, NULL, 16);
                int cstart = ci + 2;
                if (csize == 0) { complete = true; break; }
                if (pending_len >= cstart + csize + 2) {
                    int bl = csize;
                    if (gz_active) {
                        STREAM_FEED_GZ(pending + cstart, bl);
                        if (!complete) goto stream_done;
                    } else {
                        LXValue arg = px_str_len(pending + cstart, bl);
                        LXValue rv = px_call(handler, &arg, 1);
                        if (rv.type == PX_BOOL && !rv.as.b) { complete = false; goto stream_done; }
                    }
                    int rest = pending_len - (cstart + csize + 2);
                    memmove(pending, pending + cstart + csize + 2, (size_t)rest);
                    pending_len = rest;
                    continue;
                }
            }
            // 数据不完整：继续读
            pending = xrealloc(pending, pending_len + 4096);
            int n = conn_recv(tls, fd, pending + pending_len, 4096);
            if (n <= 0) break;
            pending_len += n;
        }
    } else {
        // Content-Length / EOF：边读边处理（64KB 块）
        int have_cl = content_length >= 0;
        // 先处理已有 pending
        int off = 0;
        while (off < pending_len && complete) {
            int bl = pending_len - off;
            if (bl > 65536) bl = 65536;
            if (gz_active) {
                STREAM_FEED_GZ(pending + off, bl);
            } else {
                LXValue arg = px_str_len(pending + off, bl);
                LXValue rv = px_call(handler, &arg, 1);
                if (rv.type == PX_BOOL && !rv.as.b) { complete = false; break; }
            }
            off += bl;
        }
        // 继续读网络
        while (complete) {
            if (have_cl && pending_len >= content_length) break;
            char tmp[65536];
            int want = (int)sizeof(tmp);
            if (have_cl && content_length - pending_len < want) want = content_length - pending_len;
            int n = conn_recv(tls, fd, tmp, want);
            if (n <= 0) break;
            pending_len += n;
            if (gz_active) {
                STREAM_FEED_GZ(tmp, n);
            } else {
                LXValue arg = px_str_len(tmp, n);
                LXValue rv = px_call(handler, &arg, 1);
                if (rv.type == PX_BOOL && !rv.as.b) { complete = false; break; }
            }
        }
    }
    // 收尾：gzip 剩余输出（正常路径；错误路径经 goto stream_done 跳过）
    if (gz_active) {
        STREAM_FLUSH();
    }
stream_done:
    if (gz_active) { gz_stream_free(&gz); }
    if (obuf) xfree(obuf);
    if (pending) xfree(pending);
    if (buf) xfree(buf);
    if (tls) https_close(tls); else close(fd);
    return px_bool(complete);
}

// ==================== M10 HTTP / HTTPS 客户端 ====================
// 统一 http/https GET/POST：px_http_request(url, method, body) → malloc 响应体
// 自动跟随重定向（最多 5 次）；https 走 mbedtls（静态链接，保持静态二进制）

// mbedtls 全局 CA 证书缓存（定义在文件顶部声明区，M24 https 连接池与 M10 px_https_request 共用）

static void px_ensure_cacert(void) {
    pthread_mutex_lock(&g_cacert_mu);
    if (!g_cacert_loaded) {
        mbedtls_x509_crt_init(&g_cacert);
        // RHEL 系与 Debian 系常见 CA bundle 路径
        const char* paths[] = {
            "/etc/pki/tls/certs/ca-bundle.crt",
            "/etc/ssl/certs/ca-certificates.crt",
            "/etc/ssl/cert.pem",
            NULL
        };
        for (int i = 0; paths[i]; i++) {
            if (mbedtls_x509_crt_parse_file(&g_cacert, paths[i]) == 0) {
                g_cacert_loaded = 1;
                break;
            }
        }
    }
    // M26：PX_TLS_CA_FILE 环境变量追加信任的 CA（企业内网自签证书场景；PEM 或 DER）
    {
        const char* extra = getenv("PX_TLS_CA_FILE");
        if (extra && *extra) {
            static int extra_loaded = 0;
            if (!extra_loaded) {
                extra_loaded = 1;
                if (mbedtls_x509_crt_parse_file(&g_cacert, extra) == 0) {
                    g_cacert_loaded = 1;
                } else {
                    fprintf(stderr, "[px] PX_TLS_CA_FILE 加载失败: %s\n", extra);
                }
            }
        }
    }
    pthread_mutex_unlock(&g_cacert_mu);
}

// mbedtls HTTPS 请求：返回 malloc 响应（含响应头+体），*out_len 输出长度；0=成功
static int px_https_request(const char* host, int port, const char* req, char** out, int* out_len) {
    mbedtls_net_context server_fd;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    int ret;
    const char* pers = "px_https";

    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
        (const unsigned char*)pers, strlen(pers));
    if (ret != 0) { ret = -1001; goto cleanup; }
    ret = mbedtls_net_connect(&server_fd, host, portstr, MBEDTLS_NET_PROTO_TCP);
    if (ret != 0) { ret = -1002; goto cleanup; }

    px_ensure_cacert();
    ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) { ret = -1003; goto cleanup; }
    mbedtls_ssl_conf_authmode(&conf, g_cacert_loaded ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(&conf, &g_cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_min_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);

    ret = mbedtls_ssl_setup(&ssl, &conf);
    if (ret != 0) { ret = -1004; goto cleanup; }
    mbedtls_ssl_set_hostname(&ssl, host);
    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    int guard = 0;
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ret = -1005;
            goto cleanup;
        }
        if (++guard > 40) { ret = -1006; goto cleanup; }
    }

    // 发送请求
    int len = (int)strlen(req);
    const char* rq = req;
    while (len > 0) {
        int w = mbedtls_ssl_write(&ssl, (const unsigned char*)rq, len);
        if (w == MBEDTLS_ERR_SSL_WANT_WRITE || w == MBEDTLS_ERR_SSL_WANT_READ) continue;
        if (w < 0) { ret = -1007; goto cleanup; }
        len -= w;
        rq += w;
    }
    // 读响应
    int cap = 4096, total = 0;
    char* buf = xmalloc(cap);
    for (;;) {
        if (total + 4096 > cap) { cap *= 2; buf = xrealloc(buf, cap); }
        int n = mbedtls_ssl_read(&ssl, (unsigned char*)buf + total, 4096);
        if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (n <= 0) break;
        total += n;
    }
    buf[total] = 0;
    *out = buf;
    *out_len = total;
    ret = 0;
cleanup:
    mbedtls_net_free(&server_fd);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return ret;
}

// 单次 HTTP 往返：返回 malloc 响应（响应头+体），解析状态码与 Location
static char* px_http_once(const char* url, const char* method, const char* body,
    int* out_len, int* out_status, char* loc, int loc_cap) {
    int is_https = 0;
    const char* rest;
    if (strncmp(url, "https://", 8) == 0) { is_https = 1; rest = url + 8; }
    else if (strncmp(url, "http://", 7) == 0) { rest = url + 7; }
    else { px_error("net: 不支持的协议: %s", url); return NULL; }

    char host[256];
    int hostlen = 0;
    while (rest[hostlen] && rest[hostlen] != '/' && rest[hostlen] != ':' && hostlen < 255) {
        host[hostlen] = rest[hostlen];
        hostlen++;
    }
    host[hostlen] = 0;
    if (hostlen == 0) { px_error("net: 主机名为空"); return NULL; }
    int port = is_https ? 443 : 80;
    const char* p = rest + hostlen;
    if (*p == ':') {
        port = atoi(p + 1);
        if (port <= 0 || port > 65535) { px_error("net: 端口非法"); return NULL; }
        const char* q = p + 1;
        while (*q && *q != '/') q++;
        p = q;
    }
    const char* path = (*p == '/') ? p : "/";

    // 构建请求
    char req[4096];
    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n",
        method, path, host, port);
    if (body) {
        rlen += snprintf(req + rlen, sizeof(req) - rlen,
            "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n",
            (int)strlen(body));
    }
    rlen += snprintf(req + rlen, sizeof(req) - rlen, "\r\n");
    if (body) {
        memcpy(req + rlen, body, strlen(body));
        rlen += (int)strlen(body);
    }

    char* resp = NULL;
    int resp_len = 0;
    if (is_https) {
        int r = px_https_request(host, port, req, &resp, &resp_len);
        if (r != 0) { px_error("net: HTTPS 请求失败 (%d) %s", r, host); return NULL; }
    } else {
        // 明文 http
        struct addrinfo hints, *res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        char portstr[16];
        snprintf(portstr, sizeof(portstr), "%d", port);
        if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res) { px_error("net: 解析主机失败 %s", host); return NULL; }
        int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0) { freeaddrinfo(res); px_error("net: 创建 socket 失败"); return NULL; }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            freeaddrinfo(res);
            close(fd);
            px_error("net: 连接 %s:%d 失败", host, port);
            return NULL;
        }
        freeaddrinfo(res);
        sock_send_all(fd, req, rlen);
        int cap = 4096, len = 0;
        resp = xmalloc(cap);
        for (;;) {
            if (len + 4096 > cap) { cap *= 2; resp = xrealloc(resp, cap); }
            int n = (int)recv(fd, resp + len, 4096, 0);
            if (n <= 0) break;
            len += n;
        }
        close(fd);
        resp[len] = 0;
        resp_len = len;
    }

    // 解析状态码与 Location
    int status = 0;
    loc[0] = 0;
    if (resp_len > 0) {
        sscanf(resp, "HTTP/%*s %d", &status);
        char* hdr_end = strstr(resp, "\r\n\r\n");
        char* scan_end = hdr_end ? hdr_end : resp + resp_len;
        char* line = resp;
        while (line && line < scan_end) {
            if (strncasecmp(line, "Location:", 9) == 0) {
                char* v = line + 9;
                while (*v == ' ' || *v == '\t') v++;
                int vl = 0;
                while (v[vl] && v[vl] != '\r' && v[vl] != '\n' && vl < loc_cap - 1) {
                    loc[vl] = v[vl];
                    vl++;
                }
                loc[vl] = 0;
                break;
            }
            char* nl = strchr(line, '\n');
            if (!nl || nl + 1 >= scan_end) break;
            line = nl + 1;
        }
    }

    *out_len = resp_len;
    *out_status = status;
    return resp;
}

// 统一 HTTP 请求：自动跟随重定向（最多 5 次），返回 malloc 响应体
static char* px_http_request(const char* url, const char* method, const char* body, int* out_len) {
    char cur[2048];
    snprintf(cur, sizeof(cur), "%s", url);
    for (int i = 0; i < 5; i++) {
        char loc[1024];
        int status = 0;
        int len = 0;
        char* resp = px_http_once(cur, method, body, &len, &status, loc, sizeof(loc));
        if (status >= 300 && status < 400 && loc[0]) {
            char next[2048];
            if (strncmp(loc, "http://", 7) == 0 || strncmp(loc, "https://", 8) == 0) {
                snprintf(next, sizeof(next), "%s", loc);
            } else if (loc[0] == '/') {
                const char* s = strstr(cur, "://");
                if (!s) { xfree(resp); px_error("net: 非法 URL"); return NULL; }
                const char* hp = s + 3;
                const char* hp_end = strchr(hp, '/');
                int hplen = hp_end ? (int)(hp_end - hp) : (int)strlen(hp);
                snprintf(next, sizeof(next), "%.*s://%.*s%s", (int)(s - cur), cur, hplen, hp, loc);
            } else {
                const char* s = strstr(cur, "://");
                const char* hp = s + 3;
                const char* hp_end = strchr(hp, '/');
                int hplen = hp_end ? (int)(hp_end - hp) : (int)strlen(hp);
                const char* dir = hp_end ? hp_end : "/";
                const char* dslash = strrchr(dir, '/');
                int dirlen = dslash ? (int)(dslash - dir + 1) : (int)strlen(dir);
                snprintf(next, sizeof(next), "%.*s://%.*s%.*s%s", (int)(s - cur), cur, hplen, hp, dirlen, dir, loc);
            }
            xfree(resp);
            snprintf(cur, sizeof(cur), "%s", next);
            continue;
        }
        // 分离响应体
        char* body_start = resp;
        char* sep = strstr(resp, "\r\n\r\n");
        char* head_end = sep ? sep : resp + len;
        if (sep) body_start = sep + 4;
        int body_len = (int)(resp + len - body_start);
        // M21：chunked / gzip 自动解码（客户端完整性）
        char* body_buf = xmalloc(body_len + 1);
        memcpy(body_buf, body_start, body_len);
        body_buf[body_len] = 0;
        int chunked = 0, gzip = 0;
        char* hp = resp;
        while (hp < head_end) {
            char* eol = strstr(hp, "\r\n");
            char* e = eol && eol < head_end ? eol : head_end;
            int llen = (int)(e - hp);
            if (llen > 0) {
                char line[1024];
                int cl = llen < 1023 ? llen : 1023;
                memcpy(line, hp, (size_t)cl);
                line[cl] = 0;
                if (strncasecmp(line, "Transfer-Encoding:", 18) == 0 && strcasestr(line, "chunked")) chunked = 1;
                if (strncasecmp(line, "Content-Encoding:", 17) == 0 && strcasestr(line, "gzip")) gzip = 1;
            }
            if (!eol || eol >= head_end) break;
            hp = eol + 2;
        }
        int final_len = body_len;
        char* final_buf = body_buf;
        if (chunked) {
            char* dec = px_chunked_decode(body_buf, body_len, &final_len);
            if (dec) {
                xfree(body_buf);
                body_buf = NULL;   // 防止清理阶段重复释放（M22 修复：原代码此处悬垂指针导致 double-free）
                final_buf = dec;
            }
        }
        if (gzip) {
            char* dec = px_gzip_decompress(final_buf, final_len, &final_len);
            if (dec) { if (final_buf != body_buf) xfree(final_buf); final_buf = dec; }
        }
        char* r = xmalloc(final_len + 1);
        memcpy(r, final_buf, (size_t)final_len);
        r[final_len] = 0;
        if (final_buf != body_buf) xfree(final_buf);
        if (body_buf) xfree(body_buf);
        xfree(resp);
        *out_len = final_len;
        return r;
    }
    px_error("net: 重定向次数过多（>5）");
    return NULL;
}

// http_get(url) → 响应体（支持 http/https，自动跟随重定向）
static LXValue bi_http_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("http_get 需要 (url) 参数");
    const char* url = args[0].as.obj->as.str.data;
    int len = 0;
    char* body = px_http_request(url, "GET", NULL, &len);
    LXValue r = px_str_len(body, len);
    xfree(body);
    return r;
}

// http_post(url, body) → 响应体（支持 http/https，自动跟随重定向）
static LXValue bi_http_post(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_STR) px_error("http_post 需要 (url, body) 参数");
    const char* url = args[0].as.obj->as.str.data;
    const char* body = args[1].as.obj->as.str.data;
    int len = 0;
    char* resp = px_http_request(url, "POST", body, &len);
    LXValue r = px_str_len(resp, len);
    xfree(resp);
    return r;
}

// ==================== M16 HTTP 服务端（编译模式，与并发 GC 兼容） ====================
// http_serve(port, handler)：socket 监听 + accept 循环，每连接 px_spawn 一个处理线程。
// 连接线程经 px_spawn 注册进 GC 槽位 → 并发 GC 会暂停/扫描其栈，安全。

static int px_http_hexv(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// URL 解码（+ → 空格，%XX → 字节）；返回 xmalloc 缓冲，调用者 xfree
static char* px_url_decode(const char* s) {
    int n = (int)strlen(s);
    char* out = xmalloc(n + 1);
    int oi = 0;
    for (int i = 0; i < n; i++) {
        if (s[i] == '+') {
            out[oi++] = ' ';
        } else if (s[i] == '%' && i + 2 < n) {
            int h = px_http_hexv(s[i + 1]), l = px_http_hexv(s[i + 2]);
            if (h >= 0 && l >= 0) {
                out[oi++] = (char)(h * 16 + l);
                i += 2;
            } else {
                out[oi++] = s[i];
            }
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = 0;
    return out;
}

// 大小写不敏感查找 dict 键（HTTP 头名不区分大小写）
LXValue px_dict_get_ci(LXValue d, const char* key) {
    if (d.type != PX_DICT) return px_null();
    LXObject* o = d.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcasecmp(o->as.dict.keys[i], key) == 0) return o->as.dict.vals[i];
    }
    return px_null();
}

// 从 multipart Content-Type 提取 boundary（xmalloc，调用者 xfree）
static char* px_mime_boundary(const char* ct) {
    const char* p = strstr(ct, "boundary=");
    if (!p) return NULL;
    p += 9;
    while (*p == ' ') p++;
    char* out = xmalloc(256);
    int i = 0;
    if (*p == '"') {
        p++;
        while (*p && *p != '"' && i < 255) out[i++] = *p++;
    } else {
        while (*p && *p != ';' && *p != ' ' && *p != '\r' && *p != '\n' && i < 255) out[i++] = *p++;
    }
    out[i] = 0;
    return out;
}

// 从 Content-Disposition 行提取 name="..." / filename="..."（xmalloc，调用者 xfree）
static char* px_mime_attr(const char* line, const char* key) {
    char needle[64];
    snprintf(needle, sizeof(needle), "%s\"", key);
    char* p = strstr(line, needle);
    if (!p) return NULL;
    p += strlen(needle);
    char* e = strchr(p, '"');
    if (!e) return NULL;
    int len = (int)(e - p);
    char* out = xmalloc((size_t)len + 1);
    memcpy(out, p, (size_t)len);
    out[len] = 0;
    return out;
}

// multipart/form-data 解析：设置 req["form"]（普通字段）与 req["files"]（filename -> 内容）
static void px_parse_multipart(LXValue req, const char* body, int body_len, const char* boundary) {
    LXValue form = px_dict();
    LXValue files = px_dict();
    char delim[512];
    snprintf(delim, sizeof(delim), "--%s", boundary);
    int dlen = (int)strlen(delim);
    const char* p = body;
    const char* end = body + body_len;
    while (p < end) {
        if (p + 2 <= end && p[0] == '\r' && p[1] == '\n') p += 2;
        if ((size_t)(end - p) < (size_t)dlen) break;
        if (memcmp(p, delim, (size_t)dlen) != 0) break;
        p += dlen;
        if (p + 2 <= end && p[0] == '-' && p[1] == '-') break;  // 收尾 boundary
        if (p + 2 <= end && p[0] == '\r' && p[1] == '\n') p += 2;
        const char* hs = p;
        const char* sep = NULL;
        for (const char* s = p; s + 4 <= end; s++) {
            if (s[0] == '\r' && s[1] == '\n' && s[2] == '\r' && s[3] == '\n') { sep = s; break; }
        }
        if (!sep) break;
        const char* cs = sep + 4;
        const char* ce = cs;
        for (const char* s = cs; s + dlen + 2 <= end; s++) {
            if (s[0] == '\r' && s[1] == '\n' && memcmp(s + 2, delim, (size_t)dlen) == 0) { ce = s; break; }
        }
        if (ce == cs) ce = end;
        char name[256] = {0}, filename[512] = {0};
        char* head = xmalloc((size_t)(sep - hs) + 1);
        memcpy(head, hs, (size_t)(sep - hs));
        head[sep - hs] = 0;
        char* save = NULL;
        char* line = strtok_r(head, "\r\n", &save);
        while (line) {
            if (strncasecmp(line, "Content-Disposition:", 20) == 0) {
                char* n = px_mime_attr(line, "name=");
                if (n) { snprintf(name, sizeof(name), "%s", n); xfree(n); }
                char* f = px_mime_attr(line, "filename=");
                if (f) { snprintf(filename, sizeof(filename), "%s", f); xfree(f); }
            }
            line = strtok_r(NULL, "\r\n", &save);
        }
        xfree(head);
        int clen = (int)(ce - cs);
        if (clen > 0 && cs[clen - 1] == '\n') clen--;
        if (clen > 0 && cs[clen - 1] == '\r') clen--;
        if (filename[0]) {
            px_dict_set(files, filename, px_str_len(cs, clen));
        } else if (name[0]) {
            px_dict_set(form, name, px_str_len(cs, clen));
        }
        p = ce;
    }
    px_dict_set(req, "form", form);
    px_dict_set(req, "files", files);
}

static const char* px_http_status_reason(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "OK";
    }
}

// 构造 HTTP 响应报文（xmalloc，调用者 xfree）
// ==================== M21 gzip / chunked（HTTP 客户端 + 服务端） ====================

// gzip 压缩：10 字节头 + raw deflate + CRC32 + ISIZE。返回 xmalloc，调用者 xfree；失败返回 NULL。
static char* px_gzip_compress(const char* in, int inlen, int* outlen) {
    mz_stream s;
    memset(&s, 0, sizeof(s));
    if (mz_deflateInit2(&s, 6, MZ_DEFLATED, -15, 8, MZ_DEFAULT_STRATEGY) != MZ_OK) return NULL;
    int bound = (int)mz_compressBound((mz_ulong)inlen);
    int cap = bound + 18;
    char* out = xmalloc((size_t)cap);
    int off = 0;
    out[off++] = 0x1F; out[off++] = 0x8B; out[off++] = 8; out[off++] = 0;
    out[off++] = 0; out[off++] = 0; out[off++] = 0; out[off++] = 0;
    out[off++] = 0; out[off++] = 0;  // MTIME=0, XFL=0, OS=0
    s.next_in = (const unsigned char*)in;
    s.avail_in = (mz_ulong)inlen;
    s.next_out = (unsigned char*)out + off;
    s.avail_out = (mz_ulong)bound;
    int r = mz_deflate(&s, MZ_FINISH);
    mz_deflateEnd(&s);
    if (r != MZ_STREAM_END) { xfree(out); return NULL; }
    off += (int)s.total_out;
    mz_ulong crc = mz_crc32(0, (const unsigned char*)in, (mz_ulong)inlen);
    out[off++] = (char)(crc & 0xFF);
    out[off++] = (char)((crc >> 8) & 0xFF);
    out[off++] = (char)((crc >> 16) & 0xFF);
    out[off++] = (char)((crc >> 24) & 0xFF);
    unsigned isize = (unsigned)inlen;
    out[off++] = (char)(isize & 0xFF);
    out[off++] = (char)((isize >> 8) & 0xFF);
    out[off++] = (char)((isize >> 16) & 0xFF);
    out[off++] = (char)((isize >> 24) & 0xFF);
    *outlen = off;
    return out;
}

// gzip 解压（支持 FLG 头扩展 FEXTRA/FNAME/FCOMMENT/FHCRC）。返回 xmalloc，失败返回 NULL。
static char* px_gzip_decompress(const char* in, int inlen, int* outlen) {
    if (inlen < 18 || (unsigned char)in[0] != 0x1F || (unsigned char)in[1] != 0x8B) return NULL;
    unsigned char flg = (unsigned char)in[3];
    int hdr = 10;
    if (flg & 4) { int xl = (unsigned char)in[hdr] | ((unsigned char)in[hdr+1] << 8); hdr += 2 + xl; }
    if (flg & 8) { while (hdr < inlen && in[hdr]) hdr++; hdr++; }
    if (flg & 16) { while (hdr < inlen && in[hdr]) hdr++; hdr++; }
    if (flg & 2) hdr += 2;
    if (hdr + 8 > inlen) return NULL;
    int clen = inlen - hdr - 8;
    mz_stream s;
    memset(&s, 0, sizeof(s));
    if (mz_inflateInit2(&s, -15) != MZ_OK) return NULL;
    int cap = inlen * 3 + 4096;
    char* out = xmalloc((size_t)cap);
    s.next_in = (const unsigned char*)in + hdr;
    s.avail_in = (mz_ulong)clen;
    s.next_out = (unsigned char*)out;
    s.avail_out = (mz_ulong)cap;
    int r;
    for (;;) {
        r = mz_inflate(&s, MZ_FINISH);
        if (r == MZ_STREAM_END) break;
        if (r != MZ_OK) { mz_inflateEnd(&s); xfree(out); return NULL; }
        int used = (int)(s.next_out - (unsigned char*)out);
        int nc = cap * 2;
        char* nout = xrealloc(out, (size_t)nc);
        s.next_out = (unsigned char*)nout + used;
        s.avail_out = (mz_ulong)(nc - used);
        out = nout;
        cap = nc;
    }
    mz_inflateEnd(&s);
    *outlen = (int)s.total_out;
    return out;
}

// chunked 传输编码。返回 xmalloc，调用者 xfree。
static char* px_chunked_encode(const char* in, int inlen, int* outlen) {
    if (inlen <= 0) {
        char* out = xmalloc(8);
        memcpy(out, "0\r\n\r\n", 5);
        *outlen = 5;
        return out;
    }
    int cap = inlen + inlen / 16 + 64;
    char* out = xmalloc((size_t)cap);
    int oi = 0, i = 0;
    while (i < inlen) {
        int chunk = inlen - i;
        if (chunk > 4096) chunk = 4096;
        oi += snprintf(out + oi, (size_t)(cap - oi), "%x\r\n", chunk);
        memcpy(out + oi, in + i, (size_t)chunk);
        oi += chunk;
        memcpy(out + oi, "\r\n", 2);
        oi += 2;
        i += chunk;
    }
    memcpy(out + oi, "0\r\n\r\n", 5);
    oi += 5;
    *outlen = oi;
    return out;
}

// chunked 解码。返回 xmalloc，调用者 xfree；失败返回 NULL。
static char* px_chunked_decode(const char* in, int inlen, int* outlen) {
    int cap = inlen + 1;
    char* out = xmalloc((size_t)cap);
    int oi = 0, i = 0;
    while (i < inlen) {
        int start = i;
        while (i < inlen && in[i] != '\r') i++;
        if (i >= inlen) break;
        char size_line[64];
        int sl = i - start;
        if (sl > 63) sl = 63;
        memcpy(size_line, in + start, (size_t)sl);
        size_line[sl] = 0;
        i += 2;  // \r\n
        char* semi = strchr(size_line, ';');
        if (semi) *semi = 0;
        char* endp = NULL;
        long size = strtol(size_line, &endp, 16);
        if (!endp || *endp != 0 || size < 0) break;
        if (size == 0) break;
        if (i + size > inlen) break;
        if (oi + size >= cap) {
            while (oi + size >= cap) cap *= 2;
            out = xrealloc(out, (size_t)cap);
        }
        memcpy(out + oi, in + i, (size_t)size);
        oi += (int)size;
        i += (int)size;
        if (i + 2 <= inlen && in[i] == '\r' && in[i+1] == '\n') i += 2;
        else break;
    }
    out[oi] = 0;
    *outlen = oi;
    return out;
}

static char* px_http_build_response(LXValue v, int* out_len, int* keep_alive_out) {
    int status = 200;
    const char* body = "";
    int body_len = 0;
    char extra_headers[4096] = {0};
    int gzip = 0, chunked = 0;
    int keep_alive = 1; // M23c：HTTP/1.1 默认 keep-alive；dict "keep_alive": false 强制关闭
    // （M21：gzip/chunked 标志解析同上）
    if (v.type == PX_DICT) {
        LXValue st = px_dict_get(v, "status");
        if (st.type == PX_INT) status = (int)st.as.i;
        LXValue ka = px_dict_get(v, "keep_alive");
        if (ka.type == PX_BOOL && !ka.as.b) keep_alive = 0;
        LXValue b = px_dict_get(v, "body");
        if (b.type == PX_STR) {
            body = b.as.obj->as.str.data;
            body_len = b.as.obj->as.str.len;
        }
        LXValue h = px_dict_get(v, "headers");
        if (h.type == PX_DICT) {
            LXObject* ho = h.as.obj;
            int off = 0;
            for (int i = 0; i < ho->as.dict.len && off < (int)sizeof(extra_headers) - 64; i++) {
                if (ho->as.dict.vals[i].type != PX_STR) continue;
                off += snprintf(extra_headers + off, sizeof(extra_headers) - (size_t)off,
                                "%s: %s\r\n", ho->as.dict.keys[i], ho->as.dict.vals[i].as.obj->as.str.data);
            }
        }
        // M21：gzip / chunked 标志
        LXValue gz = px_dict_get(v, "gzip");
        if (gz.type == PX_BOOL && gz.as.b) gzip = 1;
        LXValue ch = px_dict_get(v, "chunked");
        if (ch.type == PX_BOOL && ch.as.b) chunked = 1;
    } else if (v.type == PX_STR) {
        body = v.as.obj->as.str.data;
        body_len = v.as.obj->as.str.len;
    } else if (v.type == PX_INT) {
        status = (int)v.as.i;
    } else if (v.type == PX_NULL) {
        status = 204;
    }
    // body 预处理：gzip 压缩 / chunked 编码
    char* owned = NULL;
    if (gzip) {
        char* gz = px_gzip_compress(body, body_len, &body_len);
        if (gz) { owned = gz; body = gz; }
    }
    if (chunked) {
        char* chd = px_chunked_encode(body, body_len, &body_len);
        if (chd) { if (owned) xfree(owned); owned = chd; body = chd; }
    }
    const char* reason = px_http_status_reason(status);
    int has_ct = strstr(extra_headers, "Content-Type") != NULL || strstr(extra_headers, "content-type") != NULL;
    char* out = xmalloc(8192 + body_len);
    int off = 0;
    off += snprintf(out + off, 8192 + body_len - off, "HTTP/1.1 %d %s\r\n", status, reason);
    if (!chunked) {
        off += snprintf(out + off, 8192 + body_len - off, "Content-Length: %d\r\n", body_len);
    }
    off += snprintf(out + off, 8192 + body_len - off, keep_alive ? "Connection: keep-alive\r\n" : "Connection: close\r\n");
    if (keep_alive_out) *keep_alive_out = keep_alive;
    if (extra_headers[0]) {
        int l = (int)strlen(extra_headers);
        memcpy(out + off, extra_headers, (size_t)l);
        off += l;
    }
    if (gzip) {
        off += snprintf(out + off, 8192 + body_len - off, "Content-Encoding: gzip\r\n");
    }
    if (chunked) {
        off += snprintf(out + off, 8192 + body_len - off, "Transfer-Encoding: chunked\r\n");
    }
    if (!has_ct) {
        off += snprintf(out + off, 8192 + body_len - off, "Content-Type: text/plain; charset=utf-8\r\n");
    }
    memcpy(out + off, "\r\n", 2);
    off += 2;
    if (body_len > 0) {
        memcpy(out + off, body, (size_t)body_len);
        off += body_len;
    }
    out[off] = 0;
    if (owned) xfree(owned);
    if (out_len) *out_len = off;
    return out;
}

// 连接处理线程（px_spawn 注册）：args[0] = fd
// ==================== M23c HTTP 服务端 keep-alive（双模式：与解释器 builtin.rs 一致） ====================
static const char* px_file_content_type(const char* path);
// 同一连接循环处理多个请求：HTTP/1.1 默认 keep-alive；客户端 Connection: close、
// handler 返回 keep_alive:false、或空闲超时(15s) → 关闭。handler 返回 dict 支持
// "file": path（流式文件响应，大文件不占内存）。
static LXValue http_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // keep-alive 空闲读超时 15s
    struct timeval tv; tv.tv_sec = 15; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    for (;;) {
        // 1. 读请求头（直到 \r\n\r\n，上限 64KB；EOF/超时 → 关闭）
        char buf[65536];
        int len = 0;
        int header_end = -1;
        while (len < (int)sizeof(buf) - 1) {
            ssize_t n = recv(fd, buf + len, (size_t)((int)sizeof(buf) - 1 - len), 0);
            if (n == 0) { close(fd); return px_null(); }          // 对端关闭
            if (n < 0) {                                           // 超时/错误
                if (errno == EAGAIN || errno == EWOULDBLOCK) { close(fd); return px_null(); }
                close(fd); return px_null();
            }
            len += (int)n;
            buf[len] = 0;
            char* sep = strstr(buf, "\r\n\r\n");
            if (sep) { header_end = (int)(sep - buf); break; }
        }
        if (header_end < 0 || len == 0) { close(fd); return px_null(); }

        // 2. 解析请求行：METHOD SP target SP version
        char* head = buf;
        char* sp1 = strchr(head, ' ');
        if (!sp1) { close(fd); return px_null(); }
        *sp1 = 0;
        char* method = head;
        char* target = sp1 + 1;
        char* sp2 = strchr(target, ' ');
        char version[16] = "HTTP/1.1";
        if (sp2) {
            *sp2 = 0;
            const char* ver = sp2 + 1;
            int vlen = 0;
            while (ver[vlen] && ver[vlen] != '\r' && ver[vlen] != '\n' && vlen < 15) vlen++;
            memcpy(version, ver, (size_t)vlen);
            version[vlen] = 0;
        }
        char path[2048] = {0}, query[2048] = {0};
        char* q = strchr(target, '?');
        char* dec;
        if (q) {
            *q = 0;
            dec = px_url_decode(target);
            snprintf(path, sizeof(path), "%s", dec ? dec : target);
            xfree(dec);
            dec = px_url_decode(q + 1);
            snprintf(query, sizeof(query), "%s", dec ? dec : q + 1);
            xfree(dec);
        } else {
            dec = px_url_decode(target);
            snprintf(path, sizeof(path), "%s", dec ? dec : target);
            xfree(dec);
        }

        // 3. 头部 + Content-Length + Connection
        LXValue headers = px_dict();
        int content_length = 0;
        int client_close = 0; // Connection: close
        char* hline = sp2 ? sp2 + 1 : target + strlen(target);
        char* nl0 = strchr(hline, '\n');
        hline = nl0 ? nl0 + 1 : head + len;
        while (hline && *hline && *hline != '\r' && *hline != '\n') {
            char* eol = strstr(hline, "\r\n");
            if (!eol) eol = strchr(hline, '\n');
            int linelen = eol ? (int)(eol - hline) : (int)strlen(hline);
            char line[4096];
            int cl = linelen < 4095 ? linelen : 4095;
            memcpy(line, hline, (size_t)cl);
            line[cl] = 0;
            char* colon = strchr(line, ':');
            if (colon) {
                *colon = 0;
                char* k = line;
                char* v = colon + 1;
                while (*v == ' ') v++;
                char* ve = v + strlen(v);
                while (ve > v && (ve[-1] == ' ' || ve[-1] == '\r')) ve--;
                *ve = 0;
                if (strcasecmp(k, "Content-Length") == 0) content_length = atoi(v);
                if (strcasecmp(k, "Connection") == 0 && strcasecmp(v, "close") == 0) client_close = 1;
                px_dict_set(headers, k, px_str(v));
            }
            hline = eol ? eol + 2 : hline + strlen(hline);
        }

        // 4. 读 body（Content-Length）
        char body_buf[65536] = {0};
        int body_len = 0;
        if (content_length > 0) {
            int body_off = header_end + 4;
            int have = len - body_off;
            if (have > 0) {
                int take = have < content_length ? have : content_length;
                if (take > (int)sizeof(body_buf) - 1) take = (int)sizeof(body_buf) - 1;
                memcpy(body_buf, buf + body_off, (size_t)take);
                body_len = take;
            }
            while (body_len < content_length && body_len < (int)sizeof(body_buf) - 1) {
                ssize_t n = recv(fd, body_buf + body_len, (size_t)((int)sizeof(body_buf) - 1 - body_len), 0);
                if (n <= 0) break;
                body_len += (int)n;
            }
            if (body_len > content_length) body_len = content_length;
            body_buf[body_len] = 0;
        }

        // 5. 构造请求 dict
        LXValue req = px_dict();
        px_dict_set(req, "method", px_str(method));
        px_dict_set(req, "target", px_str(target));
        px_dict_set(req, "path", px_str(path));
        px_dict_set(req, "query", px_str(query));
        px_dict_set(req, "version", px_str(version));
        px_dict_set(req, "headers", headers);
        px_dict_set(req, "body", px_str_len(body_buf, body_len));
        LXValue form = px_dict();
        {
            struct sockaddr_in raddr;
            socklen_t rl = sizeof(raddr);
            if (getpeername(fd, (struct sockaddr*)&raddr, &rl) == 0) {
                char rbuf[64];
                snprintf(rbuf, sizeof(rbuf), "%s:%d", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
                px_dict_set(req, "remote", px_str(rbuf));
            } else {
                px_dict_set(req, "remote", px_str(""));
            }
        }
        LXValue ct_v = px_dict_get_ci(headers, "Content-Type");
        const char* ct = (ct_v.type == PX_STR) ? ct_v.as.obj->as.str.data : "";
        if (body_len > 0) {
            if (strstr(ct, "multipart/form-data")) {
                char* boundary = px_mime_boundary(ct);
                if (boundary) {
                    px_parse_multipart(req, body_buf, body_len, boundary);
                    xfree(boundary);
                }
            } else if (strstr(ct, "application/x-www-form-urlencoded")) {
                char* fcopy = xmalloc((size_t)body_len + 1);
                memcpy(fcopy, body_buf, (size_t)body_len);
                fcopy[body_len] = 0;
                char* save = NULL;
                char* pair = strtok_r(fcopy, "&", &save);
                while (pair) {
                    char* eq = strchr(pair, '=');
                    if (eq) {
                        *eq = 0;
                        char* kv = px_url_decode(pair);
                        char* vv = px_url_decode(eq + 1);
                        px_dict_set(form, kv, px_str(vv));
                        xfree(kv); xfree(vv);
                    } else {
                        char* kv = px_url_decode(pair);
                        px_dict_set(form, kv, px_str(""));
                        xfree(kv);
                    }
                    pair = strtok_r(NULL, "&", &save);
                }
                xfree(fcopy);
                px_dict_set(req, "form", form);
            }
        }

        // 6. 调 handler
        LXValue handler = px_get_global("__http_handler");
        LXValue resp = px_null();
        if (handler.type == PX_FUNC || handler.type == PX_NATIVE) {
            resp = px_call(handler, &req, 1);
        }

        // 7. 响应：file 流式（Connection: close，发送后关闭）或普通（keep-alive 判定）
        LXValue file_v = (resp.type == PX_DICT) ? px_dict_get(resp, "file") : px_null();
        if (file_v.type == PX_STR) {
            // 流式文件响应
            const char* fpath = file_v.as.obj->as.str.data;
            FILE* f = fopen(fpath, "rb");
            if (f) {
                fseek(f, 0, SEEK_END);
                long fsz = ftell(f);
                fseek(f, 0, SEEK_SET);
                const char* ct2 = px_file_content_type(fpath);
                char hdr[1024];
                int hl = snprintf(hdr, sizeof(hdr),
                                  "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: %s\r\nConnection: close\r\n\r\n",
                                  fsz, ct2);
                if (hl > 0) send(fd, hdr, (size_t)hl, 0);
                char fbuf[65536];
                size_t rd;
                while ((rd = fread(fbuf, 1, sizeof(fbuf), f)) > 0) {
                    size_t off = 0;
                    while (off < rd) {
                        ssize_t w = send(fd, fbuf + off, rd - off, 0);
                        if (w <= 0) { off = rd; break; }
                        off += (size_t)w;
                    }
                }
                fclose(f);
            } else {
                const char* notfound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                send(fd, notfound, (int)strlen(notfound), 0);
            }
            close(fd);
            return px_null();
        }
        int out_len = 0;
        int resp_keep_alive = 1;
        char* out = px_http_build_response(resp, &out_len, &resp_keep_alive);
        if (out) {
            if (strcmp(method, "HEAD") == 0) {
                char* sep = strstr(out, "\r\n\r\n");
                if (sep) out_len = (int)(sep - out) + 4;
            }
            if (out_len > 0) send(fd, out, out_len, 0);
            xfree(out);
        }
        // 8. keep-alive 判定
        if (client_close || !resp_keep_alive) {
            close(fd);
            return px_null();
        }
    }
    close(fd);
    return px_null();
}

// 简单 Content-Type 推断（按扩展名）
static const char* px_file_content_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcasecmp(ext, ".html") == 0 || strcasecmp(ext, ".htm") == 0) return "text/html; charset=utf-8";
    if (strcasecmp(ext, ".css") == 0) return "text/css; charset=utf-8";
    if (strcasecmp(ext, ".js") == 0) return "application/javascript; charset=utf-8";
    if (strcasecmp(ext, ".json") == 0) return "application/json; charset=utf-8";
    if (strcasecmp(ext, ".png") == 0) return "image/png";
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcasecmp(ext, ".gif") == 0) return "image/gif";
    if (strcasecmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcasecmp(ext, ".txt") == 0) return "text/plain; charset=utf-8";
    if (strcasecmp(ext, ".xml") == 0) return "application/xml; charset=utf-8";
    if (strcasecmp(ext, ".zip") == 0) return "application/zip";
    if (strcasecmp(ext, ".pdf") == 0) return "application/pdf";
    return "application/octet-stream";
}


// http_serve(port, handler)：阻塞 accept 循环（Go 风格），每连接 px_spawn 处理
static LXValue bi_http_serve(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("http_serve 需要 (port, handler) 参数");
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("http_serve 的 handler 必须是函数");
    // handler 存入全局表（GC 扫描根），连接线程经全局表取回
    px_set_global("__http_handler", handler);
    int port = (int)args[0].as.i;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) px_error("http_serve: socket 创建失败");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        px_error("http_serve: 绑定端口 %d 失败", port);
    }
    if (listen(sfd, 128) < 0) {
        close(sfd);
        px_error("http_serve: listen 失败");
    }
    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        LXValue arg = px_int(cfd);
        px_spawn(http_conn_worker, &arg, 1);
    }
    return px_null(); // 不可达
}

// ==================== M21 SSE 服务端（编译模式，与并发 GC 兼容） ====================
// sse_serve(port, handler)：accept 循环，每连接 px_spawn 处理线程。
// 连接线程：解析请求 → 发 SSE 响应头 → 注册连接(conn id) → 调 handler（req 注入 conn）
//           → handler 返回后保持连接（recv 阻塞），直到 sse_close(conn)（shutdown 唤醒）
//           或对端断开（recv 返回 0/错误）。
// sse_send(conn, data)：注册表 + 锁，任意线程可推送；写失败自动清理。
// sse_close(conn)：shutdown 唤醒连接线程，清理注册。

#define MAX_SSE_CONNS 256
static pthread_mutex_t g_sse_mu = PTHREAD_MUTEX_INITIALIZER;
static struct { int fd; int64_t id; int active; PxConn* conn; } g_sse_conns[MAX_SSE_CONNS];
static int64_t g_sse_next_id = 1;

static int sse_find(int64_t id) {
    for (int i = 0; i < MAX_SSE_CONNS; i++) {
        if (g_sse_conns[i].active && g_sse_conns[i].id == id) return i;
    }
    return -1;
}

static int sse_alloc_slot(void) {
    for (int i = 0; i < MAX_SSE_CONNS; i++) {
        if (!g_sse_conns[i].active) return i;
    }
    return -1;
}

// ==================== M23 SSE 客户端（编译模式，与解释器 builtin.rs 双模式一致） ====================
// sse_connect(url) → int conn | null（http:// 与 https://；GET 握手校验 200 + text/event-stream）
// sse_read(conn) → dict{event,data,id,retry} | null（阻塞读一条事件；断开 → null）
// sse_close(conn) → bool（同时处理服务端/客户端注册表）
// 客户端注册表：fd 读端 + pending 字节缓冲（已读未解析），锁保护；https 会话存 HttpsSession*。

#define MAX_SSE_CLIENTS 256
static pthread_mutex_t g_sse_cli_mu = PTHREAD_MUTEX_INITIALIZER;
static struct {
    int fd;
    int64_t id;
    int active;
    HttpsSession* tls;        // 非空 = https（mbedtls 会话；fd 为底层 TCP）
    unsigned char* pending;   // 已读未解析缓冲
    int pend_len;
    int pend_cap;
} g_sse_clients[MAX_SSE_CLIENTS];
static int64_t g_sse_cli_next_id = 1;

static int sse_cli_find(int64_t id) {
    for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
        if (g_sse_clients[i].active && g_sse_clients[i].id == id) return i;
    }
    return -1;
}

static int sse_cli_alloc_slot(void) {
    for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
        if (!g_sse_clients[i].active) return i;
    }
    return -1;
}


// SSE 帧编码：str → `data: xxx\n\n`；dict → event/data/id/retry。返回 xmalloc，调用者 xfree。
static char* sse_frame_c(LXValue data) {
    char* out = xmalloc(8192);
    int off = 0;
    if (data.type == PX_STR) {
        const char* s = data.as.obj->as.str.data;
        int n = data.as.obj->as.str.len;
        int start = 0;
        for (int i = 0; i <= n; i++) {
            if (i == n || s[i] == '\n') {
                off += snprintf(out + off, 8192 - off, "data: %.*s\n", i - start, s + start);
                start = i + 1;
            }
        }
        off += snprintf(out + off, 8192 - off, "\n");
    } else if (data.type == PX_DICT) {
        LXObject* o = data.as.obj;
        for (int i = 0; i < o->as.dict.len; i++) {
            const char* k = o->as.dict.keys[i];
            LXValue v = o->as.dict.vals[i];
            if (strcmp(k, "id") == 0 && v.type == PX_STR) {
                off += snprintf(out + off, 8192 - off, "id: %s\n", v.as.obj->as.str.data);
            } else if (strcmp(k, "event") == 0 && v.type == PX_STR) {
                off += snprintf(out + off, 8192 - off, "event: %s\n", v.as.obj->as.str.data);
            } else if (strcmp(k, "retry") == 0 && v.type == PX_INT) {
                off += snprintf(out + off, 8192 - off, "retry: %lld\n", (long long)v.as.i);
            } else if (strcmp(k, "data") == 0) {
                if (v.type == PX_STR) {
                    const char* s = v.as.obj->as.str.data;
                    int n = v.as.obj->as.str.len;
                    int start = 0;
                    for (int j = 0; j <= n; j++) {
                        if (j == n || s[j] == '\n') {
                            off += snprintf(out + off, 8192 - off, "data: %.*s\n", j - start, s + start);
                            start = j + 1;
                        }
                    }
                } else if (v.type != PX_NULL) {
                    off += snprintf(out + off, 8192 - off, "data: %s\n", val_cstr(v));
                }
            }
        }
        off += snprintf(out + off, 8192 - off, "\n");
    } else {
        off += snprintf(out + off, 8192 - off, "data: %s\n\n", val_cstr(data));
    }
    out[off] = 0;
    return out;
}

// SSE 连接线程（px_spawn 注册进 GC 槽位）：args[0] = fd
static LXValue sse_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // M27：TLS 握手（若 tls_server 注册）→ PxConn 统一读写（堆分配共享给 sse_send）
    PxConn* c = xmalloc(sizeof(PxConn));
    if (px_conn_init(c, fd) != 0) { xfree(c); return px_null(); }
    g_cur_conn = c;
    __sync_fetch_and_add(&g_px_inflight, 1);

    // 1. 读请求头（直到 \r\n\r\n，上限 64KB）
    char buf[65536];
    int len = 0;
    int header_end = -1;
    while (len < (int)sizeof(buf) - 1) {
        ssize_t n = px_conn_read(c, buf + len, (size_t)((int)sizeof(buf) - 1 - len));
        if (n <= 0) break;
        len += (int)n;
        buf[len] = 0;
        char* sep = strstr(buf, "\r\n\r\n");
        if (sep) { header_end = (int)(sep - buf); break; }
    }
    if (header_end < 0 || len == 0) {
        px_conn_close(c);  // 对象保留（closed 标记），避免并发 ws/sse 使用悬垂指针
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
        return px_null();
    }

    // 2. 解析请求行：METHOD SP target SP version
    char* head = buf;
    char* sp1 = strchr(head, ' ');
    if (!sp1) { close(fd); return px_null(); }
    *sp1 = 0;
    char* method = head;
    char* target = sp1 + 1;
    char* sp2 = strchr(target, ' ');
    char version[16] = "HTTP/1.1";
    if (sp2) {
        *sp2 = 0;
        const char* ver = sp2 + 1;
        int vlen = 0;
        while (ver[vlen] && ver[vlen] != '\r' && ver[vlen] != '\n' && vlen < 15) vlen++;
        memcpy(version, ver, (size_t)vlen);
        version[vlen] = 0;
    }
    char path[2048] = {0}, query[2048] = {0};
    char* q = strchr(target, '?');
    char* dec;
    if (q) {
        *q = 0;
        dec = px_url_decode(target);
        snprintf(path, sizeof(path), "%s", dec ? dec : target);
        xfree(dec);
        dec = px_url_decode(q + 1);
        snprintf(query, sizeof(query), "%s", dec ? dec : q + 1);
        xfree(dec);
    } else {
        dec = px_url_decode(target);
        snprintf(path, sizeof(path), "%s", dec ? dec : target);
        xfree(dec);
    }

    // 3. 头部
    LXValue headers = px_dict();
    char* hline = sp2 ? sp2 + 1 : target + strlen(target);
    char* nl0 = strchr(hline, '\n');
    hline = nl0 ? nl0 + 1 : head + len;
    while (hline && *hline && *hline != '\r' && *hline != '\n') {
        char* eol = strstr(hline, "\r\n");
        if (!eol) eol = strchr(hline, '\n');
        int linelen = eol ? (int)(eol - hline) : (int)strlen(hline);
        char line[4096];
        int cl = linelen < 4095 ? linelen : 4095;
        memcpy(line, hline, (size_t)cl);
        line[cl] = 0;
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            char* k = line;
            char* v = colon + 1;
            while (*v == ' ') v++;
            char* ve = v + strlen(v);
            while (ve > v && (ve[-1] == ' ' || ve[-1] == '\r')) ve--;
            *ve = 0;
            px_dict_set(headers, k, px_str(v));
        }
        hline = eol ? eol + 2 : hline + strlen(hline);
    }

    // 4. 构造请求 dict（SSE 无 body，简化）
    LXValue req = px_dict();
    px_dict_set(req, "method", px_str(method));
    px_dict_set(req, "target", px_str(target));
    px_dict_set(req, "path", px_str(path));
    px_dict_set(req, "query", px_str(query));
    px_dict_set(req, "version", px_str(version));
    px_dict_set(req, "headers", headers);
    px_dict_set(req, "body", px_str(""));
    px_dict_set(req, "form", px_dict());
    {
        struct sockaddr_in raddr;
        socklen_t rl = sizeof(raddr);
        if (getpeername(fd, (struct sockaddr*)&raddr, &rl) == 0) {
            char rbuf[64];
            snprintf(rbuf, sizeof(rbuf), "%s:%d", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
            px_dict_set(req, "remote", px_str(rbuf));
        } else {
            px_dict_set(req, "remote", px_str(""));
        }
    }

    // 5. 分配 conn id + 注册（防 fd 复用：注册后才接受 sse_send）
    pthread_mutex_lock(&g_sse_mu);
    int64_t conn = g_sse_next_id++;
    int slot = sse_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_sse_mu);
        close(fd);
        return px_null();
    }
    g_sse_conns[slot].fd = fd;
    g_sse_conns[slot].id = conn;
    g_sse_conns[slot].active = 1;
    g_sse_conns[slot].conn = c;
    pthread_mutex_unlock(&g_sse_mu);
    px_dict_set(req, "conn", px_int(conn));

    // 6. 发 SSE 响应头（连接保持，直到 sse_close / 对端断开）
    const char* hdr = "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream; charset=utf-8\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n";
    px_conn_write(c, hdr, strlen(hdr));

    // 7. 调 handler（req 注入 conn；handler 内/后台线程可 sse_send）
    LXValue handler = px_get_global("__sse_handler");
    if (handler.type == PX_FUNC || handler.type == PX_NATIVE) {
        px_call(handler, &req, 1);
    }

    // 8. 保持连接：read 阻塞直到 sse_close（shutdown 唤醒）或对端断开
    char rb[64];
    while (px_conn_read(c, rb, sizeof(rb)) > 0) {}

    // 9. 清理注册 + 关闭（只在仍注册时 close，避免与 sse_close 重复关闭）
    int closed = 0;
    pthread_mutex_lock(&g_sse_mu);
    for (int i = 0; i < MAX_SSE_CONNS; i++) {
        if (g_sse_conns[i].active && g_sse_conns[i].fd == fd) {
            g_sse_conns[i].active = 0;
            g_sse_conns[i].fd = -1;
            closed = 1;
        }
    }
    pthread_mutex_unlock(&g_sse_mu);
    if (closed) {
        // 置空共享 conn 指针（sse_send 已不可再写该连接）
        pthread_mutex_lock(&g_sse_mu);
        for (int j = 0; j < MAX_SSE_CONNS; j++) {
            if (g_sse_conns[j].conn == c) g_sse_conns[j].conn = NULL;
        }
        pthread_mutex_unlock(&g_sse_mu);
        px_conn_close(c);  // 对象保留（closed 标记），避免并发 ws/sse 使用悬垂指针
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
    }
    return px_null();
}

// sse_serve(port, handler)：阻塞 accept 循环
static LXValue bi_sse_serve(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("sse_serve 需要 (port, handler) 参数");
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("sse_serve 的 handler 必须是函数");
    // handler 存入全局表（GC 扫描根），连接线程经全局表取回
    px_set_global("__sse_handler", handler);
    int port = (int)args[0].as.i;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) px_error("sse_serve: socket 创建失败");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        px_error("sse_serve: 绑定端口 %d 失败", port);
    }
    if (listen(sfd, 128) < 0) {
        close(sfd);
        px_error("sse_serve: listen 失败");
    }
    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        LXValue arg = px_int(cfd);
        px_spawn(sse_conn_worker, &arg, 1);
    }
    return px_null(); // 不可达
}

// sse_send(conn, data) → bool（连接不存在/写失败返回 false）
static LXValue bi_sse_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("sse_send 需要 (conn, data) 参数");
    int64_t conn = args[0].as.i;
    char* frame = sse_frame_c(args[1]);
    pthread_mutex_lock(&g_sse_mu);
    int idx = sse_find(conn);
    if (idx < 0) {
        pthread_mutex_unlock(&g_sse_mu);
        xfree(frame);
        return px_bool(false);
    }
    PxConn* pc = g_sse_conns[idx].conn;
    if (!pc) {
        pthread_mutex_unlock(&g_sse_mu);
        xfree(frame);
        return px_bool(false);
    }
    int fd = g_sse_conns[idx].fd;
    ssize_t w = px_conn_write(pc, frame, strlen(frame));
    if (w < 0) {
        g_sse_conns[idx].active = 0;
        g_sse_conns[idx].fd = -1;
        g_sse_conns[idx].conn = NULL;
        px_conn_close(pc);  // 对象保留
        pthread_mutex_unlock(&g_sse_mu);
        xfree(frame);
        return px_bool(false);
    }
    pthread_mutex_unlock(&g_sse_mu);
    xfree(frame);
    return px_bool(true);
}

// sse_close(conn) → bool（服务端连接 shutdown 唤醒；客户端连接直接关闭）
static LXValue bi_sse_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("sse_close 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    // 服务端连接
    pthread_mutex_lock(&g_sse_mu);
    int idx = sse_find(conn);
    if (idx < 0) {
        pthread_mutex_unlock(&g_sse_mu);
        // 客户端连接
        pthread_mutex_lock(&g_sse_cli_mu);
        int cidx = sse_cli_find(conn);
        if (cidx < 0) {
            pthread_mutex_unlock(&g_sse_cli_mu);
            return px_bool(false);
        }
        int cfd = g_sse_clients[cidx].fd;
        HttpsSession* ctl = g_sse_clients[cidx].tls;
        g_sse_clients[cidx].active = 0;
        g_sse_clients[cidx].tls = NULL;
        if (g_sse_clients[cidx].pending) xfree(g_sse_clients[cidx].pending);
        g_sse_clients[cidx].pending = NULL;
        g_sse_clients[cidx].pend_len = g_sse_clients[cidx].pend_cap = 0;
        pthread_mutex_unlock(&g_sse_cli_mu);
        shutdown(cfd, SHUT_RDWR);
        if (ctl) https_close(ctl); else close(cfd);
        return px_bool(true);
    }
    int fd = g_sse_conns[idx].fd;
    g_sse_conns[idx].active = 0;
    g_sse_conns[idx].fd = -1;
    shutdown(fd, SHUT_RDWR);
    close(fd);
    pthread_mutex_unlock(&g_sse_mu);
    return px_bool(true);
}

// 解析 SSE 事件文本（field: value 行）为 dict
static LXValue sse_parse_event_c(const char* text, int len) {
    LXValue d = px_dict();
    px_dict_set(d, "event", px_str("message"));
    char* data_buf = xmalloc(len + 1);
    int data_len = 0;
    char* id_buf = xmalloc(len + 1);
    int has_id = 0;
    char* event_buf = xmalloc(len + 1);
    int has_event = 0;
    int has_retry = 0;
    long long retry = 0;
    int start = 0;
    for (int i = 0; i <= len; i++) {
        if (i == len || text[i] == '\n') {
            int ll = i - start;
            if (ll > 0 && text[start + ll - 1] == '\r') ll--;
            const char* line = text + start;
            int colon = -1;
            for (int j = 0; j < ll; j++) {
                if (line[j] == ':') { colon = j; break; }
            }
            if (colon >= 0) {
                const char* value = line + colon + 1;
                int vlen = ll - colon - 1;
                if (vlen > 0 && *value == ' ') { value++; vlen--; }
                if (colon == 5 && strncmp(line, "event", 5) == 0) {
                    memcpy(event_buf, value, vlen);
                    event_buf[vlen] = 0;
                    has_event = 1;
                } else if (colon == 4 && strncmp(line, "data", 4) == 0) {
                    if (data_len > 0) data_buf[data_len++] = '\n';
                    memcpy(data_buf + data_len, value, vlen);
                    data_len += vlen;
                } else if (colon == 2 && strncmp(line, "id", 2) == 0) {
                    memcpy(id_buf, value, vlen);
                    id_buf[vlen] = 0;
                    has_id = 1;
                } else if (colon == 5 && strncmp(line, "retry", 5) == 0) {
                    char tmp[64];
                    int tl = vlen < 63 ? vlen : 63;
                    memcpy(tmp, value, tl);
                    tmp[tl] = 0;
                    retry = atoll(tmp);
                    has_retry = 1;
                }
            }
            start = i + 1;
        }
    }
    if (has_event) px_dict_set(d, "event", px_str(event_buf));
    if (has_id) px_dict_set(d, "id", px_str(id_buf));
    if (has_retry) px_dict_set(d, "retry", px_int(retry));
    if (data_len > 0) px_dict_set(d, "data", px_str_len(data_buf, data_len));
    else px_dict_set(d, "data", px_str(""));
    xfree(data_buf);
    xfree(id_buf);
    xfree(event_buf);
    return d;
}

// sse_connect(url) → conn id | null
static LXValue bi_sse_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("sse_connect 需要 (url) 参数");
    const char* url = args[0].as.obj->as.str.data;
    int is_https = 0;
    const char* rest;
    if (strncmp(url, "https://", 8) == 0) {
        is_https = 1;
        rest = url + 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        rest = url + 7;
    } else {
        return px_null(); // 仅支持 http:// 与 https://
    }
    char host[256];
    int port = is_https ? 443 : 80;
    const char* path = "/";
    const char* slash = strchr(rest, '/');
    int hl;
    if (slash) {
        hl = (int)(slash - rest);
        path = slash;
    } else {
        hl = (int)strlen(rest);
    }
    if (hl <= 0 || hl >= (int)sizeof(host)) return px_null();
    memcpy(host, rest, hl);
    host[hl] = 0;
    char* colon = strchr(host, ':');
    if (colon) {
        *colon = 0;
        port = atoi(colon + 1);
        if (port <= 0) return px_null();
    }
    int fd = -1;
    HttpsSession* tls = NULL;
    if (is_https) {
        tls = https_connect(host, port);
        if (!tls) return px_null();
        fd = tls->net.fd;
    } else {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return px_null();
        struct hostent* he = gethostbyname(host);
        if (!he) { close(fd); return px_null(); }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)port);
        memcpy(&addr.sin_addr, he->h_addr, (size_t)he->h_length);
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return px_null(); }
    }
    char req[8192];
    char hosthdr[512];
    if (colon) snprintf(hosthdr, sizeof(hosthdr), "%s:%d", host, port);
    else snprintf(hosthdr, sizeof(hosthdr), "%s", host);
    int rl = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: PuXian/0.1\r\nAccept: text/event-stream\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n",
        path, hosthdr);
    if (rl <= 0 || conn_send(tls, fd, req, rl) < 0) {
        if (tls) https_close(tls); else close(fd);
        return px_null();
    }
    // 读响应头（直到 \r\n\r\n，上限 64KB）
    unsigned char hbuf[65536];
    int hn = 0;
    int header_end = -1;
    while (hn < (int)sizeof(hbuf)) {
        int n = conn_recv(tls, fd, (char*)(hbuf + hn), (int)sizeof(hbuf) - hn);
        if (n <= 0) break;
        hn += n;
        for (int i = 0; i + 3 < hn; i++) {
            if (hbuf[i] == '\r' && hbuf[i+1] == '\n' && hbuf[i+2] == '\r' && hbuf[i+3] == '\n') {
                header_end = i + 4;
                break;
            }
        }
        if (header_end >= 0) break;
    }
    if (header_end < 0) {
        if (tls) https_close(tls); else close(fd);
        return px_null();
    }
    char* hstr = xmalloc((size_t)header_end + 1);
    memcpy(hstr, hbuf, (size_t)header_end);
    hstr[header_end] = 0;
    int status = 0;
    char* sp = strchr(hstr, ' ');
    if (sp) status = atoi(sp + 1);
    int ct_ok = 0;
    char* ctp = strstr(hstr, "Content-Type:");
    if (!ctp) ctp = strstr(hstr, "content-type:");
    if (ctp) {
        ctp += 14;
        while (*ctp == ' ') ctp++;
        if (strstr(ctp, "text/event-stream")) ct_ok = 1;
    }
    xfree(hstr);
    if (status != 200 || !ct_ok) {
        if (tls) https_close(tls); else close(fd);
        return px_null();
    }
    // 注册（剩余字节进 pending）
    pthread_mutex_lock(&g_sse_cli_mu);
    int slot = sse_cli_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_sse_cli_mu);
        if (tls) https_close(tls); else close(fd);
        return px_null();
    }
    int64_t id = g_sse_cli_next_id++;
    int remain = hn - header_end;
    g_sse_clients[slot].fd = fd;
    g_sse_clients[slot].tls = tls;
    g_sse_clients[slot].id = id;
    g_sse_clients[slot].active = 1;
    if (remain > 0) {
        g_sse_clients[slot].pend_cap = remain + 64;
        g_sse_clients[slot].pending = xmalloc((size_t)g_sse_clients[slot].pend_cap);
        memcpy(g_sse_clients[slot].pending, hbuf + header_end, (size_t)remain);
        g_sse_clients[slot].pend_len = remain;
    } else {
        g_sse_clients[slot].pending = NULL;
        g_sse_clients[slot].pend_len = g_sse_clients[slot].pend_cap = 0;
    }
    pthread_mutex_unlock(&g_sse_cli_mu);
    return px_int(id);
}

// sse_read(conn) → 事件 dict | null（阻塞读一条；断开 → null）
static LXValue bi_sse_read(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("sse_read 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    int fd = -1;
    int idx = -1;
    pthread_mutex_lock(&g_sse_cli_mu);
    idx = sse_cli_find(conn);
    if (idx < 0) { pthread_mutex_unlock(&g_sse_cli_mu); return px_null(); }
    fd = g_sse_clients[idx].fd;
    pthread_mutex_unlock(&g_sse_cli_mu);
    for (;;) {
        // 尝试从 pending 取完整事件（\n\n 或 \r\n\r\n）
        pthread_mutex_lock(&g_sse_cli_mu);
        if (g_sse_clients[idx].active && g_sse_clients[idx].pending && g_sse_clients[idx].pend_len > 0) {
            unsigned char* p = g_sse_clients[idx].pending;
            int n = g_sse_clients[idx].pend_len;
            int found = -1, sep = 0;
            for (int i = 0; i < n; i++) {
                if (p[i] == '\n' && i + 1 < n && p[i+1] == '\n') { found = i; sep = 2; break; }
                if (p[i] == '\n' && i + 2 < n && p[i+1] == '\r' && p[i+2] == '\n') { found = i; sep = 3; break; }
            }
            if (found >= 0) {
                LXValue ev = sse_parse_event_c((const char*)p, found);
                int newlen = n - (found + sep);
                memmove(p, p + found + sep, (size_t)newlen);
                g_sse_clients[idx].pend_len = newlen;
                pthread_mutex_unlock(&g_sse_cli_mu);
                return ev;
            }
        }
        pthread_mutex_unlock(&g_sse_cli_mu);
        // 阻塞读更多（https 走 mbedtls）
        unsigned char tmp[4096];
        HttpsSession* tls = NULL;
        pthread_mutex_lock(&g_sse_cli_mu);
        if (g_sse_clients[idx].active) tls = g_sse_clients[idx].tls;
        pthread_mutex_unlock(&g_sse_cli_mu);
        int n = conn_recv(tls, fd, (char*)tmp, (int)sizeof(tmp));
        if (n <= 0) {
            pthread_mutex_lock(&g_sse_cli_mu);
            HttpsSession* t2 = NULL;
            if (g_sse_clients[idx].active) {
                g_sse_clients[idx].active = 0;
                if (g_sse_clients[idx].pending) xfree(g_sse_clients[idx].pending);
                g_sse_clients[idx].pending = NULL;
                g_sse_clients[idx].pend_len = g_sse_clients[idx].pend_cap = 0;
                t2 = g_sse_clients[idx].tls;
                g_sse_clients[idx].tls = NULL;
            }
            pthread_mutex_unlock(&g_sse_cli_mu);
            if (t2) https_close(t2); else close(fd);
            return px_null();
        }
        pthread_mutex_lock(&g_sse_cli_mu);
        if (!g_sse_clients[idx].active) {
            pthread_mutex_unlock(&g_sse_cli_mu);
            return px_null();
        }
        if (g_sse_clients[idx].pend_len + n > g_sse_clients[idx].pend_cap) {
            int ncap = g_sse_clients[idx].pend_cap ? g_sse_clients[idx].pend_cap * 2 : (n + 64);
            if (ncap < g_sse_clients[idx].pend_len + n) ncap = g_sse_clients[idx].pend_len + n + 64;
            unsigned char* np = xmalloc((size_t)ncap);
            if (g_sse_clients[idx].pend_len > 0)
                memcpy(np, g_sse_clients[idx].pending, (size_t)g_sse_clients[idx].pend_len);
            if (g_sse_clients[idx].pending) xfree(g_sse_clients[idx].pending);
            g_sse_clients[idx].pending = np;
            g_sse_clients[idx].pend_cap = ncap;
        }
        memcpy(g_sse_clients[idx].pending + g_sse_clients[idx].pend_len, tmp, (size_t)n);
        g_sse_clients[idx].pend_len += n;
        pthread_mutex_unlock(&g_sse_cli_mu);
    }
}

// ==================== M17 .px 脚本执行机制（编译模式） ====================
// 与解释器模式（Rust web.rs）行为一致：静态文件 + .px 脚本执行（进程池雏形，
// fork+exec `px run`，子进程隔离 + 超时 kill，天然满足安全需求）。
// 双模式可跑同一套 .px 应用（PX_INIT_GLOBALS 环境变量传递 REQUEST/GET/POST/SERVER）。

// 单调时钟毫秒（超时计算）
static long long px_mono_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// MIME 类型表（静态文件）
static const char* px_mime_type(const char* path) {
    const char* dot = strrchr(path, '.');
    if (!dot) return "application/octet-stream";
    if (strcasecmp(dot, ".html") == 0 || strcasecmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
    if (strcasecmp(dot, ".css") == 0) return "text/css; charset=utf-8";
    if (strcasecmp(dot, ".js") == 0 || strcasecmp(dot, ".mjs") == 0) return "application/javascript; charset=utf-8";
    if (strcasecmp(dot, ".json") == 0) return "application/json; charset=utf-8";
    if (strcasecmp(dot, ".txt") == 0 || strcasecmp(dot, ".md") == 0) return "text/plain; charset=utf-8";
    if (strcasecmp(dot, ".xml") == 0 || strcasecmp(dot, ".svg") == 0) return "text/xml; charset=utf-8";
    if (strcasecmp(dot, ".csv") == 0) return "text/csv; charset=utf-8";
    if (strcasecmp(dot, ".png") == 0) return "image/png";
    if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcasecmp(dot, ".gif") == 0) return "image/gif";
    if (strcasecmp(dot, ".webp") == 0) return "image/webp";
    if (strcasecmp(dot, ".ico") == 0) return "image/x-icon";
    if (strcasecmp(dot, ".bmp") == 0) return "image/bmp";
    if (strcasecmp(dot, ".pdf") == 0) return "application/pdf";
    if (strcasecmp(dot, ".woff") == 0) return "font/woff";
    if (strcasecmp(dot, ".woff2") == 0) return "font/woff2";
    if (strcasecmp(dot, ".ttf") == 0) return "font/ttf";
    if (strcasecmp(dot, ".wasm") == 0) return "application/wasm";
    if (strcasecmp(dot, ".zip") == 0) return "application/zip";
    if (strcasecmp(dot, ".mp3") == 0) return "audio/mpeg";
    if (strcasecmp(dot, ".mp4") == 0) return "video/mp4";
    if (strcasecmp(dot, ".webm") == 0) return "video/webm";
    return "application/octet-stream";
}

// px 解释器二进制定位：PX_BIN 环境变量优先，否则 PATH 中的 "px"
static const char* px_px_bin(void) {
    const char* b = getenv("PX_BIN");
    return (b && *b) ? b : "px";
}

// ==================== M25 .px 进程池（PHP-FPM 风格） ====================
// M17 的每请求 fork+exec `px run` 开销（fork + exec + 解释器启动）较大；
// M25 预派生 PX_POOL_SIZE 个 `px --worker` 解释器进程常驻复用：
//   父进程把任务帧（4 字节大端长度 + path\0env_json\0dump\0timeout）写入空闲
//   worker stdin；worker 执行目标脚本，把结果帧（exit_code\0output）写回 stdout。
// 超时 → SIGKILL + 补位；worker 崩溃 → 补位重试，最终回退旧 fork+exec 路径保底。
#define PX_POOL_SIZE 4

// 兜底路径前置声明（定义在本节之后）
static int px_run_px_child(const char* path, const char* env_json, int dump_response,
                           int timeout_ms, char** out, int* out_len, int* exit_code);

typedef struct {
    pid_t pid;
    int in_fd;    // 父→worker（worker stdin）
    int out_fd;   // worker→父（worker stdout）
    int alive;
    int busy;
} PXWorker;

static PXWorker g_px_pool[PX_POOL_SIZE];
static pthread_mutex_t g_px_pool_mu = PTHREAD_MUTEX_INITIALIZER;
static int g_px_pool_ready = 0;

// 派生一个 `px --worker` 进程（返回值 0 成功）
static int px_pool_spawn(PXWorker* w) {
    int in_pipe[2] = {-1, -1}, out_pipe[2] = {-1, -1};
    if (pipe(in_pipe) != 0 || pipe(out_pipe) != 0) {
        if (in_pipe[0] >= 0) { close(in_pipe[0]); close(in_pipe[1]); }
        if (out_pipe[0] >= 0) { close(out_pipe[0]); close(out_pipe[1]); }
        return -1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(in_pipe[0]); close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);
        return -1;
    }
    if (pid == 0) {
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        close(in_pipe[0]); close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);
        // 关闭继承的其他 fd（监听 socket、连接 fd、其他 worker 管道）：
        // 防止 worker 持有监听端口 / 管道写端导致 EOF 误判（M25 进程池）
        close_range(3, ~0U, 0);
        execlp(px_px_bin(), "px", "--worker", (char*)NULL);
        dprintf(STDERR_FILENO, "px: 找不到 px 解释器（设置 PX_BIN 或加入 PATH）\n");
        _exit(127);
    }
    close(in_pipe[0]);
    close(out_pipe[1]);
    w->pid = pid;
    w->in_fd = in_pipe[1];
    w->out_fd = out_pipe[0];
    w->alive = 1;
    w->busy = 0;
    return 0;
}

// 惰性初始化池（首次 px_pool_run 时）
static void px_pool_init_lazy(void) {
    pthread_mutex_lock(&g_px_pool_mu);
    if (!g_px_pool_ready) {
        for (int i = 0; i < PX_POOL_SIZE; i++) {
            g_px_pool[i].alive = 0;
            g_px_pool[i].busy = 0;
            if (px_pool_spawn(&g_px_pool[i]) != 0) g_px_pool[i].alive = 0;
        }
        g_px_pool_ready = 1;
    }
    pthread_mutex_unlock(&g_px_pool_mu);
}

// 取空闲 worker 下标（无空闲 -1）
static int px_pool_take(void) {
    pthread_mutex_lock(&g_px_pool_mu);
    int idx = -1;
    for (int i = 0; i < PX_POOL_SIZE; i++) {
        if (g_px_pool[i].alive && !g_px_pool[i].busy) {
            g_px_pool[i].busy = 1;
            idx = i;
            break;
        }
    }
    pthread_mutex_unlock(&g_px_pool_mu);
    return idx;
}

static void px_pool_release(int idx) {
    pthread_mutex_lock(&g_px_pool_mu);
    if (idx >= 0 && idx < PX_POOL_SIZE) g_px_pool[idx].busy = 0;
    pthread_mutex_unlock(&g_px_pool_mu);
}

// 杀掉并补位 worker（调用方此后不得再引用该下标）
static void px_pool_respawn(int idx) {
    pthread_mutex_lock(&g_px_pool_mu);
    if (idx < 0 || idx >= PX_POOL_SIZE) { pthread_mutex_unlock(&g_px_pool_mu); return; }
    PXWorker w = g_px_pool[idx];
    g_px_pool[idx].alive = 0;
    g_px_pool[idx].busy = 0;
    if (w.pid > 0) kill(w.pid, SIGKILL);
    if (w.in_fd > 0) close(w.in_fd);
    if (w.out_fd > 0) close(w.out_fd);
    if (px_pool_spawn(&g_px_pool[idx]) != 0) g_px_pool[idx].alive = 0;
    pthread_mutex_unlock(&g_px_pool_mu);
}

// 管道/文件 fd 全量写（进程池任务帧用；sock_send_all 的 send() 不适用于管道）
static int px_write_all(int fd, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, data + sent, (size_t)(len - sent));
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        sent += (int)n;
    }
    return sent;
}

// 写任务帧（4 字节大端长度 + path\0env_json\0dump\0timeout_ms）
static int px_pool_send_task(int idx, const char* path, const char* env_json,
                             int dump_response, int timeout_ms) {
    char payload[65536];
    int plen = snprintf(payload, sizeof(payload), "%s%c%s%c%c%c%d",
                        path, 0,
                        env_json ? env_json : "", 0,
                        dump_response ? '1' : '0', 0,
                        timeout_ms);
    if (plen < 0 || plen >= (int)sizeof(payload)) return -1;
    PXWorker* w = &g_px_pool[idx];
    uint32_t len = (uint32_t)plen;
    uint8_t hdr[4] = { (uint8_t)(len >> 24), (uint8_t)(len >> 16),
                       (uint8_t)(len >> 8), (uint8_t)len };
    // 管道 fd 不能用 send()（ENOTSOCK），用 write 循环
    if (px_write_all(w->in_fd, (const char*)hdr, 4) < 0) return -1;
    if (px_write_all(w->in_fd, payload, plen) < 0) return -1;
    return 0;
}

// 读结果帧（带总超时）。返回 0=成功, 1=超时, 2=失败。
// 成功时 *out 指向 malloc 缓冲（调用方 xfree），*out_len 为输出字节数，*exit_code 为退出码。
static int px_pool_recv_result(int idx, char** out, int* out_len, int* exit_code, int timeout_ms) {
    PXWorker* w = &g_px_pool[idx];
    long long t0 = px_mono_ms();
    uint8_t hdr[4];
    int got = 0;
    while (got < 4) {
        if (timeout_ms > 0) {
            long long now = px_mono_ms();
            int rem = timeout_ms - (int)(now - t0);
            if (rem <= 0) return 1;
            struct pollfd pp = { w->out_fd, POLLIN, 0 };
            int r = poll(&pp, 1, rem);
            if (r == 0) return 1;
            if (r < 0) { if (errno == EINTR) continue; return 2; }
        }
        ssize_t n = read(w->out_fd, hdr + got, 4 - got);
        if (n <= 0) return 2;
        got += (int)n;
    }
    uint32_t len = ((uint32_t)hdr[0] << 24) | ((uint32_t)hdr[1] << 16) |
                   ((uint32_t)hdr[2] << 8) | hdr[3];
    if (len == 0 || len > 128u * 1024u * 1024u) return 2;
    char* buf = xmalloc((size_t)len + 1);
    int total = 0;
    while (total < (int)len) {
        if (timeout_ms > 0) {
            long long now = px_mono_ms();
            int rem = timeout_ms - (int)(now - t0);
            if (rem <= 0) { xfree(buf); return 1; }
            struct pollfd pp = { w->out_fd, POLLIN, 0 };
            int r = poll(&pp, 1, rem);
            if (r == 0) { xfree(buf); return 1; }
            if (r < 0) { if (errno == EINTR) continue; xfree(buf); return 2; }
        }
        ssize_t n = read(w->out_fd, buf + total, (size_t)(len - total));
        if (n <= 0) { xfree(buf); return 2; }
        total += (int)n;
    }
    buf[total] = 0;
    char* sep = memchr(buf, 0, (size_t)total);
    if (!sep) { xfree(buf); return 2; }
    int prefix_len = (int)(sep - buf) + 1;
    if (exit_code) *exit_code = atoi(buf);
    int outlen = total - prefix_len;
    memmove(buf, sep + 1, (size_t)outlen);
    buf[outlen] = 0;
    *out = buf;
    *out_len = outlen;
    return 0;
}

// 进程池执行 .px 脚本（px_serve / px_exec 统一入口；取代每请求 fork+exec）：
// 返回 0=完成, 1=超时, 2=失败。无空闲 worker / 崩溃重试后回退 px_run_px_child 保底。
static int px_pool_run(const char* path, const char* env_json, int dump_response,
                       int timeout_ms, char** out, int* out_len, int* exit_code) {
    px_pool_init_lazy();
    for (int attempt = 0; attempt < 3; attempt++) {
        int idx = px_pool_take();
        if (idx < 0) {
            return px_run_px_child(path, env_json, dump_response, timeout_ms, out, out_len, exit_code);
        }
        if (px_pool_send_task(idx, path, env_json, dump_response, timeout_ms) != 0) {
            px_pool_respawn(idx);
            if (attempt >= 2) {
                return px_run_px_child(path, env_json, dump_response, timeout_ms, out, out_len, exit_code);
            }
            continue;
        }
        int rc = px_pool_recv_result(idx, out, out_len, exit_code, timeout_ms);
        if (rc == 0) {
            px_pool_release(idx);
            return 0;
        }
        px_pool_respawn(idx);
        if (rc == 1) return 1;  // 超时：脚本卡死，回退也超时，直接报超时
        if (attempt >= 2) {
            return px_run_px_child(path, env_json, dump_response, timeout_ms, out, out_len, exit_code);
        }
    }
    return 2;
}

// 子进程执行 `px run <path>` 并捕获 stdout（进程池满/兜底路径：隔离 + 超时 kill）
// env_json：PX_INIT_GLOBALS 环境变量（JSON dict，解释器注入全局变量）
// dump_response：1 时设置 PX_DUMP_RESPONSE=1（px_serve 用：脚本 RESPONSE 序列化到 stdout 尾部）
// timeout_ms<=0 无限等待。返回 0=完成, 1=超时, 2=启动失败。
static int px_run_px_child(const char* path, const char* env_json, int dump_response,
                           int timeout_ms, char** out, int* out_len, int* exit_code) {
    int pfd[2];
    if (pipe(pfd) != 0) return 2;
    pid_t pid = fork();
    if (pid < 0) {
        close(pfd[0]); close(pfd[1]);
        return 2;
    }
    if (pid == 0) {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        if (env_json && *env_json) setenv("PX_INIT_GLOBALS", env_json, 1);
        else unsetenv("PX_INIT_GLOBALS");
        if (dump_response) setenv("PX_DUMP_RESPONSE", "1", 1);
        else unsetenv("PX_DUMP_RESPONSE");
        execlp(px_px_bin(), "px", "run", path, (char*)NULL);
        dprintf(STDERR_FILENO, "px: 找不到 px 解释器（设置 PX_BIN 或加入 PATH）\n");
        _exit(127);
    }
    close(pfd[1]);
    int cap = 65536;
    char* buf = xmalloc((size_t)cap);
    int total = 0;
    int timedout = 0;
    long long t0 = px_mono_ms();
    for (;;) {
        if (timeout_ms > 0) {
            long long now = px_mono_ms();
            int rem = timeout_ms - (int)(now - t0);
            if (rem <= 0) { timedout = 1; break; }
            struct pollfd pp = { pfd[0], POLLIN, 0 };
            int r = poll(&pp, 1, rem);
            if (r == 0) { timedout = 1; break; }
            if (r < 0) { if (errno == EINTR) continue; break; }
        }
        ssize_t n = read(pfd[0], buf + total, (size_t)(cap - 1 - total));
        if (n <= 0) break;
        total += (int)n;
        if (total >= cap - 1) break;
    }
    if (timedout) kill(pid, SIGKILL);
    close(pfd[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    buf[total] = 0;
    *out = buf;
    *out_len = total;
    if (exit_code) {
        if (WIFEXITED(status)) *exit_code = WEXITSTATUS(status);
        else *exit_code = 1;
    }
    return timedout ? 1 : (WIFEXITED(status) ? 0 : 2);
}

// urlencoded → dict（GET 查询串 / POST 表单共用）
static LXValue px_parse_urlenc(const char* body) {
    LXValue d = px_dict();
    char* copy = xmalloc(strlen(body) + 1);
    strcpy(copy, body);
    char* save = NULL;
    char* pair = strtok_r(copy, "&", &save);
    while (pair) {
        char* eq = strchr(pair, '=');
        if (eq) {
            *eq = 0;
            char* k = px_url_decode(pair);
            char* v = px_url_decode(eq + 1);
            px_dict_set(d, k, px_str(v));
            xfree(k); xfree(v);
        } else {
            char* k = px_url_decode(pair);
            px_dict_set(d, k, px_str(""));
            xfree(k);
        }
        pair = strtok_r(NULL, "&", &save);
    }
    xfree(copy);
    return d;
}

// 发送 HTTP 响应（HEAD 只发响应头）
// ==================== M27 P0：PxConn 连接抽象（明文/TLS 统一） ====================
// 服务端 TLS：accept 后 px_conn_init 做 mbedtls 服务端握手（若 tls_server 已注册）。
static int px_conn_tls_handshake(PxConn* c) {
    mbedtls_ssl_context* ssl = (mbedtls_ssl_context*)c->ssl;
    mbedtls_ssl_config* conf = (mbedtls_ssl_config*)c->conf;
    mbedtls_ctr_drbg_context* drbg = (mbedtls_ctr_drbg_context*)c->ctr_drbg;
    mbedtls_entropy_context* ent = (mbedtls_entropy_context*)c->entropy;
    const char* pers = "px_server";
    if (mbedtls_ctr_drbg_seed(drbg, mbedtls_entropy_func, ent,
                              (const unsigned char*)pers, strlen(pers)) != 0) return -1;
    if (mbedtls_ssl_config_defaults(conf, MBEDTLS_SSL_IS_SERVER,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) return -1;
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, drbg);
    pthread_mutex_lock(&g_srv_tls_mu);
    int oc = mbedtls_ssl_conf_own_cert(conf, &g_srv_cert, &g_srv_key);
    pthread_mutex_unlock(&g_srv_tls_mu);
    if (oc != 0) return -1;
    // M30：服务端 https 连接池——全局 TLS 会话缓存共享给所有连接（Session ID 恢复）
    if (g_srv_tls_cache_init) {
        mbedtls_ssl_conf_session_cache(conf, &g_srv_tls_cache,
                                       mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
    }
    // TLS 1.2 会话票据（与客户端 M25 票据恢复对偶）
    mbedtls_ssl_conf_session_tickets(conf, MBEDTLS_SSL_SESSION_TICKETS_ENABLED);
    if (mbedtls_ssl_setup(ssl, conf) != 0) return -1;
    mbedtls_ssl_set_bio(ssl, &c->fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    int ret;
    while ((ret = mbedtls_ssl_handshake(ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) return -1;
    }
    return 0;
}

// 初始化连接（fd 上 TLS 握手若已注册服务端证书；失败返回 -1，连接应关闭）
int px_conn_init(PxConn* c, int fd) {
    memset(c, 0, sizeof(*c));
    c->fd = fd;
    c->is_tls = 0;
    c->rlen = 0; c->roff = 0;
    if (!g_srv_tls_ready) return 0; // 明文
    c->ssl = malloc(sizeof(mbedtls_ssl_context));
    c->conf = malloc(sizeof(mbedtls_ssl_config));
    c->ctr_drbg = malloc(sizeof(mbedtls_ctr_drbg_context));
    c->entropy = malloc(sizeof(mbedtls_entropy_context));
    if (!c->ssl || !c->conf || !c->ctr_drbg || !c->entropy) {
        close(fd);
        c->fd = -1;
        return -1;
    }
    mbedtls_ssl_init((mbedtls_ssl_context*)c->ssl);
    mbedtls_ssl_config_init((mbedtls_ssl_config*)c->conf);
    mbedtls_ctr_drbg_init((mbedtls_ctr_drbg_context*)c->ctr_drbg);
    mbedtls_entropy_init((mbedtls_entropy_context*)c->entropy);
    if (px_conn_tls_handshake(c) != 0) {
        px_conn_close(c);
        return -1;
    }
    c->is_tls = 1;
    return 0;
}

// 读：TLS 带缓冲（SSL_read 一次多读；已缓冲数据先出）
ssize_t px_conn_read(PxConn* c, void* buf, size_t n) {
    if (c->closed) return -1;
    if (!c->is_tls) return recv(c->fd, buf, n, 0);
    if (c->roff < c->rlen) {
        size_t avail = (size_t)(c->rlen - c->roff);
        size_t take = avail < n ? avail : n;
        memcpy(buf, c->rbuf + c->roff, take);
        c->roff += (int)take;
        return (ssize_t)take;
    }
    c->rlen = 0; c->roff = 0;
    int ret = mbedtls_ssl_read((mbedtls_ssl_context*)c->ssl, c->rbuf, (size_t)sizeof(c->rbuf));
    if (ret <= 0) return ret; // 0=EOF, <0=错误
    c->rlen = ret;
    size_t take = (size_t)ret < n ? (size_t)ret : n;
    memcpy(buf, c->rbuf, take);
    c->roff = (int)take;
    return (ssize_t)take;
}

ssize_t px_conn_write(PxConn* c, const void* buf, size_t n) {
    if (c->closed) return -1;
    if (!c->is_tls) return send(c->fd, buf, n, MSG_NOSIGNAL);
    size_t off = 0;
    while (off < n) {
        int ret = mbedtls_ssl_write((mbedtls_ssl_context*)c->ssl,
                                    (const unsigned char*)buf + off, n - off);
        if (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
        if (ret <= 0) return -1;
        off += (size_t)ret;
    }
    return (ssize_t)off;
}

void px_conn_close(PxConn* c) {
    if (!c) return;
    if (c->closed) return;  // 幂等：已关闭
    c->closed = 1;
    if (c->is_tls) {
        mbedtls_ssl_close_notify((mbedtls_ssl_context*)c->ssl);
        mbedtls_ssl_free((mbedtls_ssl_context*)c->ssl);
        mbedtls_ssl_config_free((mbedtls_ssl_config*)c->conf);
        mbedtls_ctr_drbg_free((mbedtls_ctr_drbg_context*)c->ctr_drbg);
        mbedtls_entropy_free((mbedtls_entropy_context*)c->entropy);
        free(c->ssl); free(c->conf); free(c->ctr_drbg); free(c->entropy);
        c->ssl = c->conf = c->ctr_drbg = c->entropy = NULL;
    }
    if (c->fd >= 0) { close(c->fd); c->fd = -1; }
    c->is_tls = 0;
}

// tls_server(cert, key)：注册服务端 TLS（cert/key 为 PEM 路径或 PEM 内容）→ bool
static LXValue bi_tls_server(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_STR) {
        px_error("tls_server 需要 (cert_pem, key_pem) 参数");
    }
    const char* cert = args[0].as.obj->as.str.data;
    const char* key = args[1].as.obj->as.str.data;
    pthread_mutex_lock(&g_srv_tls_mu);
    mbedtls_x509_crt_init(&g_srv_cert);
    mbedtls_pk_init(&g_srv_key);
    int rc1 = strstr(cert, "-----BEGIN")
        ? mbedtls_x509_crt_parse(&g_srv_cert, (const unsigned char*)cert, strlen(cert) + 1)
        : mbedtls_x509_crt_parse_file(&g_srv_cert, cert);
    int rc2 = strstr(key, "-----BEGIN")
        ? mbedtls_pk_parse_key(&g_srv_key, (const unsigned char*)key, strlen(key) + 1, NULL, 0, NULL, NULL)
        : mbedtls_pk_parse_keyfile(&g_srv_key, key, NULL, NULL, NULL);
    if (rc1 != 0 || rc2 != 0) {
        char eb[256];
        mbedtls_strerror(rc1 != 0 ? rc1 : rc2, eb, sizeof(eb));
        pthread_mutex_unlock(&g_srv_tls_mu);
        px_error("tls_server: 证书/私钥解析失败: %s", eb);
    }
    // M30：初始化服务端 TLS 会话缓存（连接池）
    if (!g_srv_tls_cache_init) {
        mbedtls_ssl_cache_init(&g_srv_tls_cache);
        mbedtls_ssl_cache_set_max_entries(&g_srv_tls_cache, 128);
        mbedtls_ssl_cache_set_timeout(&g_srv_tls_cache, 86400);
        g_srv_tls_cache_init = 1;
    }
    g_srv_tls_ready = 1;
    pthread_mutex_unlock(&g_srv_tls_mu);
    return px_bool(true);
}

// ==================== M27 P0：优雅关闭（SIGINT/SIGTERM） ====================
static void px_sigstop_handler(int sig) {
    (void)sig;
    g_px_stop = 1;
    int fd = (int)g_px_listen_fd;
    if (fd >= 0) shutdown(fd, SHUT_RDWR);
}

// ==================== M27 P0：Cookie / Session（跨请求共享，文件存储） ====================
#define PX_SESSION_TTL 7200
#define PX_SESSION_NAME "pxsid"

// 解析 Cookie 头 "a=1; b=2" → dict；返回 px_dict
static LXValue px_parse_cookie(const char* header) {
    LXValue d = px_dict();
    if (!header) return d;
    const char* p = header;
    while (*p) {
        while (*p == ' ' || *p == ';') p++;
        const char* eq = strchr(p, '=');
        if (!eq) break;
        char k[256]; int kl = (int)(eq - p);
        if (kl > 255) kl = 255;
        memcpy(k, p, (size_t)kl); k[kl] = 0;
        const char* v = eq + 1;
        while (*v == ' ') v++;
        const char* semi = strchr(v, ';');
        int vl = semi ? (int)(semi - v) : (int)strlen(v);
        while (vl > 0 && (v[vl-1] == ' ' || v[vl-1] == '\r')) vl--;
        if (vl > 0 && v[0] == '"' && v[vl-1] == '"') { v++; vl -= 2; }
        char vbuf[1024]; if (vl > 1023) vl = 1023;
        memcpy(vbuf, v, (size_t)vl); vbuf[vl] = 0;
        px_dict_set(d, k, px_str(vbuf));
        p = semi ? semi + 1 : v + strlen(v);
    }
    return d;
}

// 从 headers dict 取指定头（大小写不敏感）→ str 或 null
static LXValue px_header_get(const LXValue* headers, const char* name) {
    if (headers->type != PX_DICT) return px_null();
    LXObject* o = headers->as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcasecmp(o->as.dict.keys[i], name) == 0) return o->as.dict.vals[i];
    }
    return px_null();
}

// 读整个文件（≤16MB）→ 1 成功 / 0 失败
static int px_read_whole_file(const char* path, char** out, int* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz > 16 * 1024 * 1024) { fclose(f); return 0; }
    char* buf = xmalloc((size_t)sz + 1);
    size_t got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[got] = 0;
    *out = buf;
    *out_len = (int)got;
    return 1;
}

// session 存储：文件 /tmp/px_sessions/<sid>.json（worker 进程间共享，flock 并发安全）
static const char* px_session_dir(void) {
    static char dir[512];
    if (!dir[0]) {
        const char* e = getenv("PX_SESSION_DIR");
        snprintf(dir, sizeof(dir), "%s", e && *e ? e : "/tmp/px_sessions");
        mkdir(dir, 0700);
    }
    return dir;
}

static void px_session_path(char* out, size_t n, const char* sid) {
    snprintf(out, n, "%s/%s.json", px_session_dir(), sid);
}

// 读 session 文件 → dict{data, exp} 或 null
static LXValue px_session_read(const char* sid) {
    char path[1024];
    px_session_path(path, sizeof(path), sid);
    char* data = NULL; int len = 0;
    if (!px_read_whole_file(path, &data, &len)) return px_null();
    LXValue v = px_str_len(data, len);
    xfree(data);
    LXValue j = px_call(px_get_global("json_parse"), &v, 1);
    if (j.type != PX_DICT) return px_null();
    return j;
}

static int px_session_valid(const LXValue* sess, long long now) {
    if (sess->type != PX_DICT) return 0;
    LXValue exp = px_dict_get(*sess, "exp");
    return exp.type == PX_INT && exp.as.i > now;
}

// 写 session 文件（原子：tmp + rename；flock 串行）
static void px_session_write(const char* sid, const LXValue* sess) {
    char path[1024], tmp[1080];
    px_session_path(path, sizeof(path), sid);
    snprintf(tmp, sizeof(tmp), "%s.tmp.%d", path, (int)getpid());
    LXValue j = px_call(px_get_global("json_stringify"), (LXValue*)sess, 1);
    if (j.type != PX_STR) return;
    FILE* f = fopen(tmp, "wb");
    if (!f) return;
    int fl = j.as.obj->as.str.len;
    if (fwrite(j.as.obj->as.str.data, 1, (size_t)fl, f) != (size_t)fl) {
        fclose(f); unlink(tmp); return;
    }
    fclose(f);
    rename(tmp, path);
}

// 清理过期 session（px_serve 启动时）
static void px_session_sweep(void) {
    DIR* d = opendir(px_session_dir());
    if (!d) return;
    long long now = (long long)time(NULL);
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        char sid[512]; strncpy(sid, e->d_name, sizeof(sid) - 8);
        char* dot = strstr(sid, ".json");
        if (!dot) continue;
        *dot = 0;
        LXValue sess = px_session_read(sid);
        if (!px_session_valid(&sess, now)) {
            char path[1024]; px_session_path(path, sizeof(path), sid);
            unlink(path);
        }
    }
    closedir(d);
}

// 生成新 session id（时间 + pid + 计数器 hex）
static void px_new_session_id(char* out, size_t n) {
    static long long seq = 0;
    long long s = __sync_fetch_and_add(&seq, 1);
    snprintf(out, n, "%llx%llx%llx",
             (unsigned long long)time(NULL), (unsigned long long)getpid(), (unsigned long long)s);
}

// thread-local：当前请求 session id / 待注入 Set-Cookie / 基础认证失败 realm
static __thread char g_cur_sid[256];
static __thread int g_cur_sid_set = 0;
static __thread char g_session_cookie[512];
static __thread int g_session_cookie_set = 0;
static __thread char g_auth_realm[128];
static __thread int g_auth_realm_set = 0;

// 重置请求线程局部状态（每请求结束调用）
static void px_reset_request_state(void) {
    g_cur_sid_set = 0;
    g_cur_sid[0] = 0;
    g_session_cookie_set = 0;
    g_session_cookie[0] = 0;
    g_auth_realm_set = 0;
    g_auth_realm[0] = 0;
}

// session_open()：读 REQUEST.cookie[pxsid] 复用/新建；新会话 Set-Cookie 记录待注入
// （worker 内 session 数据存文件，跨 worker 共享；Set-Cookie 经 PX_DUMP_RESPONSE 回传父进程）
static LXValue bi_session_open(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("session_open 不需要参数");
    LXValue req = px_get_global("REQUEST");
    const char* sid = "";
    if (req.type == PX_DICT) {
        LXValue ck = px_dict_get(req, "cookie");
        if (ck.type == PX_DICT) {
            LXValue v = px_dict_get(ck, PX_SESSION_NAME);
            if (v.type == PX_STR) sid = v.as.obj->as.str.data;
        }
    }
    long long now = (long long)time(NULL);
    if (sid && *sid && strlen(sid) < 256) {
        LXValue sess = px_session_read(sid);
        if (px_session_valid(&sess, now)) {
            // 续期 + 复用
            LXValue exp = px_int(now + PX_SESSION_TTL);
            px_dict_set(sess, "exp", exp);
            px_session_write(sid, &sess);
            strncpy(g_cur_sid, sid, sizeof(g_cur_sid) - 1);
            g_cur_sid_set = 1;
            return px_str(sid);
        }
    }
    char nid[256];
    px_new_session_id(nid, sizeof(nid));
    LXValue data = px_dict();
    LXValue sess = px_dict();
    px_dict_set(sess, "data", data);
    px_dict_set(sess, "exp", px_int(now + PX_SESSION_TTL));
    px_session_write(nid, &sess);
    snprintf(g_session_cookie, sizeof(g_session_cookie),
             "%s=%s; Path=/; HttpOnly", PX_SESSION_NAME, nid);
    g_session_cookie_set = 1;
    strncpy(g_cur_sid, nid, sizeof(g_cur_sid) - 1);
    g_cur_sid_set = 1;
    return px_str(nid);
}

// 当前 session data dict（未 open / 已过期 → null）
static LXValue px_cur_session_data(void) {
    if (!g_cur_sid_set) return px_null();
    long long now = (long long)time(NULL);
    LXValue sess = px_session_read(g_cur_sid);
    if (!px_session_valid(&sess, now)) return px_null();
    return px_dict_get(sess, "data");
}

static LXValue bi_session_id(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    if (g_cur_sid_set && *g_cur_sid) return px_str(g_cur_sid);
    return px_null();
}

static LXValue bi_session_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("session_get 需要 (key) 参数");
    LXValue data = px_cur_session_data();
    if (data.type != PX_DICT) return px_null();
    return px_dict_get(data, args[0].as.obj->as.str.data);
}

static LXValue bi_session_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("session_set 需要 (key, value) 参数");
    if (!g_cur_sid_set) return px_bool(false);
    LXValue sess = px_session_read(g_cur_sid);
    long long now = (long long)time(NULL);
    if (!px_session_valid(&sess, now)) {
        sess = px_dict();
        px_dict_set(sess, "data", px_dict());
        px_dict_set(sess, "exp", px_int(now + PX_SESSION_TTL));
    }
    LXValue data = px_dict_get(sess, "data");
    if (data.type != PX_DICT) { data = px_dict(); px_dict_set(sess, "data", data); }
    px_dict_set(data, args[0].as.obj->as.str.data, args[1]);
    px_session_write(g_cur_sid, &sess);
    return px_bool(true);
}

static LXValue bi_session_del(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("session_del 需要 (key) 参数");
    LXValue data = px_cur_session_data();
    if (data.type != PX_DICT) return px_bool(false);
    LXValue sess = px_session_read(g_cur_sid);
    LXValue nd = px_dict();
    LXObject* o = data.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], args[0].as.obj->as.str.data) != 0)
            px_dict_set(nd, o->as.dict.keys[i], o->as.dict.vals[i]);
    }
    px_dict_set(sess, "data", nd);
    px_session_write(g_cur_sid, &sess);
    return px_bool(true);
}

static LXValue bi_session_destroy(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    if (!g_cur_sid_set) return px_bool(false);
    char path[1024];
    px_session_path(path, sizeof(path), g_cur_sid);
    int rc = unlink(path) == 0;
    g_cur_sid_set = 0;
    g_cur_sid[0] = 0;
    return px_bool(rc);
}

// basic_auth(user, pass)：校验 Authorization: Basic；失败记录 realm（响应 401 注入）
static LXValue bi_basic_auth(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_STR) {
        px_error("basic_auth 需要 (user, pass) 参数");
    }
    LXValue req = px_get_global("REQUEST");
    LXValue hdr = px_null();
    if (req.type == PX_DICT) {
        LXValue headers = px_dict_get(req, "headers");
        if (headers.type == PX_DICT) hdr = px_header_get(&headers, "Authorization");
    }
    if (hdr.type == PX_STR) {
        const char* h = hdr.as.obj->as.str.data;
        if (strncasecmp(h, "Basic ", 6) == 0) {
            // base64 解码 user:pass
            LXValue b64 = px_str(h + 6);
            LXValue decoded = px_call(px_get_global("base64_decode"), &b64, 1);
            if (decoded.type == PX_STR) {
                char expect[512];
                snprintf(expect, sizeof(expect), "%s:%s",
                         args[0].as.obj->as.str.data, args[1].as.obj->as.str.data);
                if (strcmp(decoded.as.obj->as.str.data, expect) == 0) return px_bool(true);
            }
        }
    }
    strncpy(g_auth_realm, "px", sizeof(g_auth_realm) - 1);
    g_auth_realm_set = 1;
    return px_bool(false);
}

static void px_px_send_ex(int fd, int status, const char* ct, const char* body, int body_len,
                          int head_only, int keep_alive, const char* extra_headers) {
    const char* reason = px_http_status_reason(status);
    char head[2048];
    int off = snprintf(head, sizeof(head),
                       "HTTP/1.1 %d %s\r\nContent-Length: %d\r\nConnection: %s\r\n",
                       status, reason, body_len, keep_alive ? "keep-alive" : "close");
    if (ct && *ct) off += snprintf(head + off, sizeof(head) - (size_t)off, "Content-Type: %s\r\n", ct);
    if (extra_headers && *extra_headers) {
        int l = (int)strlen(extra_headers);
        if (off + l < (int)sizeof(head)) { memcpy(head + off, extra_headers, (size_t)l); off += l; }
    }
    off += snprintf(head + off, sizeof(head) - (size_t)off, "\r\n");
    if (g_cur_conn && g_cur_conn->is_tls) {
        px_conn_write(g_cur_conn, head, (size_t)off);
        if (!head_only && body_len > 0) px_conn_write(g_cur_conn, body, (size_t)body_len);
    } else {
        send(fd, head, off, 0);
        if (!head_only && body_len > 0) send(fd, body, body_len, 0);
    }
}

// 兼容旧签名（Connection: close，无额外头）
static void px_px_send(int fd, int status, const char* ct, const char* body, int body_len, int head_only) {
    px_px_send_ex(fd, status, ct, body, body_len, head_only, 0, NULL);
}

// 请求头是否 Connection: close（keep-alive 判定）
static int px_req_wants_close(LXValue* headers) {
    LXValue cv = px_header_get(headers, "Connection");
    if (cv.type == PX_STR) {
        const char* v = cv.as.obj->as.str.data;
        while (*v == ' ') v++;
        if (strncasecmp(v, "close", 5) == 0) return 1;
    }
    return 0;
}

// 生成请求 ID：px-<unixms>-<seq>
static void px_new_req_id(char* out, size_t n) {
    static long long seq = 0;
    long long s = __sync_fetch_and_add(&seq, 1);
    snprintf(out, n, "px-%lld-%lld", (long long)time(NULL) * 1000, s);
}

// HTTP date（RFC 7231 IMF-fixdate）：gmtime_r → "Sun, 06 Nov 1994 08:49:37 GMT"
static void px_http_date(time_t ts, char* out, size_t n) {
    struct tm tmv;
    gmtime_r(&ts, &tmv);
    static const char* wd[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static const char* mo[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    snprintf(out, n, "%s, %02d %s %04d %02d:%02d:%02d GMT",
             wd[tmv.tm_wday], tmv.tm_mday, mo[tmv.tm_mon],
             tmv.tm_year + 1900, tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
}

// 解析 Range: bytes=start-end（单段）→ 返回 1 + 设置 start/end；不支持返回 0
static int px_parse_range(const char* r, long long size, long long* start, long long* end) {
    while (*r == ' ') r++;
    if (strncasecmp(r, "bytes=", 6) != 0) return 0;
    r += 6;
    const char* dash = strchr(r, '-');
    if (!dash) return 0;
    if (dash == r) {
        // suffix: bytes=-N
        long long n = atoll(dash + 1);
        if (n <= 0 || size <= 0) return 0;
        *start = size - n; if (*start < 0) *start = 0;
        *end = size - 1;
        return 1;
    }
    char num[64];
    long long cl = dash - r;
    if (cl >= (long long)sizeof(num)) return 0;
    memcpy(num, r, (size_t)cl); num[cl] = 0;
    long long s = atoll(num);
    if (s < 0 || s >= size) return 0;
    long long e = *dash ? atoll(dash + 1) : size - 1;
    if (e >= size) e = size - 1;
    if (e < s) return 0;
    *start = s; *end = e;
    return 1;
}

// 响应体是否应 gzip（Accept-Encoding: gzip 且为文本类）
static int px_resp_gzipable(LXValue* headers, const char* ct, int body_len) {
    if (body_len < 1024) return 0;
    LXValue ae = px_header_get(headers, "Accept-Encoding");
    if (ae.type != PX_STR || !strstr(ae.as.obj->as.str.data, "gzip")) return 0;
    if (!ct || !*ct) return 1;
    if (strncasecmp(ct, "text/", 5) == 0) return 1;
    if (strstr(ct, "json") || strstr(ct, "javascript") || strstr(ct, "xml") ||
        strstr(ct, "svg") || strstr(ct, "csv")) return 1;
    return 0;
}

// 连接处理线程（px_spawn 注册）：args[0] = fd
static LXValue px_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // M27：TLS 握手（若 tls_server 注册）→ PxConn 统一读写
    PxConn conn;
    if (px_conn_init(&conn, fd) != 0) { return px_null(); }
    g_cur_conn = &conn;
    __sync_fetch_and_add(&g_px_inflight, 1);

    // M29d：keep-alive 循环——同一连接连续处理多个请求，直到客户端
    // Connection: close / 空闲超时（15s）/ 出错。
    for (;;) {
        char body_tmp_path[1024] = {0};
        int body_tmp_file = -1;
        char* body_buf = NULL;

        // 1. 读请求头（直到 \r\n\r\n，上限 64KB；keep-alive 空闲超时 15s）
        struct timeval tv = { 15, 0 };
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        char buf[65536];
        int len = 0;
        int header_end = -1;
        while (len < (int)sizeof(buf) - 1) {
            ssize_t n = px_conn_read(&conn, buf + len, (size_t)((int)sizeof(buf) - 1 - len));
            if (n <= 0) break;
            len += (int)n;
            buf[len] = 0;
            char* sep = strstr(buf, "\r\n\r\n");
            if (sep) { header_end = (int)(sep - buf); break; }
        }
        if (header_end < 0 || len == 0) { break; }  // 客户端关闭 / 空闲超时

        // 2. 请求行
        char* head = buf;
        char* sp1 = strchr(head, ' ');
        if (!sp1) { break; }
        *sp1 = 0;
        char* method = head;
        char* target = sp1 + 1;
        char* sp2 = strchr(target, ' ');
        if (sp2) *sp2 = 0;
        char path[2048] = {0}, query[2048] = {0};
        char* q = strchr(target, '?');
        char* dec;
        if (q) {
            *q = 0;
            dec = px_url_decode(target); snprintf(path, sizeof(path), "%s", dec ? dec : target); xfree(dec);
            dec = px_url_decode(q + 1); snprintf(query, sizeof(query), "%s", dec ? dec : q + 1); xfree(dec);
        } else {
            dec = px_url_decode(target); snprintf(path, sizeof(path), "%s", dec ? dec : target); xfree(dec);
        }

        // 3. 头部 + Content-Length + keep-alive 判定
        LXValue headers = px_dict();
        int content_length = 0;
        char* hline = sp2 ? sp2 + 1 : target + strlen(target);
        char* nl0 = strchr(hline, '\n');
        hline = nl0 ? nl0 + 1 : head + len;
        int client_keep_alive = 1;
        while (hline && *hline && *hline != '\r' && *hline != '\n') {
            char* eol = strstr(hline, "\r\n");
            if (!eol) eol = strchr(hline, '\n');
            int linelen = eol ? (int)(eol - hline) : (int)strlen(hline);
            char line[4096];
            int cl = linelen < 4095 ? linelen : 4095;
            memcpy(line, hline, (size_t)cl); line[cl] = 0;
            char* colon = strchr(line, ':');
            if (colon) {
                *colon = 0;
                char* k = line;
                char* v = colon + 1;
                while (*v == ' ') v++;
                char* ve = v + strlen(v);
                while (ve > v && (ve[-1] == ' ' || ve[-1] == '\r')) ve--;
                *ve = 0;
                if (strcasecmp(k, "Content-Length") == 0) content_length = atoi(v);
                if (strcasecmp(k, "Connection") == 0 && strncasecmp(v, "close", 5) == 0) client_keep_alive = 0;
                px_dict_set(headers, k, px_str(v));
            }
            hline = eol ? eol + 2 : hline + strlen(hline);
        }
        // HTTP/1.0 默认关闭（除非 keep-alive）；HTTP/1.1 默认 keep
        if (strncmp(target - 5, "HTTP/1.0", 8) == 0 && strncasecmp(target - 5, "HTTP/1.0", 8) == 0) {
            // 版本号在 sp2 之后；简单判断：请求行末尾含 HTTP/1.0
        }
        const char* ver = sp2 ? sp2 + 1 : "HTTP/1.1";
        if (strncmp(ver, "HTTP/1.0", 8) == 0) client_keep_alive = 0;

        // M29c：请求 ID（X-Request-Id 链路追踪）
        char req_id[64];
        px_new_req_id(req_id, sizeof(req_id));

        // 4. 读 body（M27：max_body_size 限制 → 413；>1MB 落盘临时文件防内存溢出）
        int body_len = 0;
        if (content_length > 0) {
            if (content_length > g_px_max_body) {
                char extra[256];
                snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                px_px_send_ex(fd, 413, "text/plain; charset=utf-8", "413 Payload Too Large", 24, 0, 0, extra);
                goto req_done;
            }
            int body_off = header_end + 4;
            int have = len - body_off;
            if (have > 0 && have > content_length) have = content_length;
            if (content_length > 1024 * 1024) {
                const char* tmpdir = getenv("PX_BODY_TMP_DIR");
                if (!tmpdir || !*tmpdir) tmpdir = "/tmp";
                snprintf(body_tmp_path, sizeof(body_tmp_path), "%s/px_body_%d_%d.tmp",
                         tmpdir, (int)getpid(), (int)__sync_fetch_and_add(&g_px_body_seq, 1));
                body_tmp_file = open(body_tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
                if (body_tmp_file < 0) {
                    px_px_send(fd, 500, "text/plain; charset=utf-8", "500 创建 body 临时文件失败", 26, 0);
                    goto req_done;
                }
                if (have > 0) (void)write(body_tmp_file, buf + body_off, (size_t)have);
                int remaining = content_length - have;
                char tmpb[16384];
                while (remaining > 0) {
                    ssize_t n = px_conn_read(&conn, tmpb, sizeof(tmpb));
                    if (n <= 0) break;
                    (void)write(body_tmp_file, tmpb, (size_t)n);
                    remaining -= (int)n;
                }
                close(body_tmp_file);
                body_tmp_file = -1;
            } else {
                body_buf = xmalloc((size_t)content_length + 1);
                int got = have;
                if (have > 0) memcpy(body_buf, buf + body_off, (size_t)have);
                while (got < content_length) {
                    ssize_t n = px_conn_read(&conn, body_buf + got, (size_t)(content_length - got));
                    if (n <= 0) break;
                    got += (int)n;
                }
                body_len = got;
                body_buf[body_len] = 0;
            }
        }

        // 5. 请求 dict + form/files（Content-Type 驱动）
        LXValue req = px_dict();
        px_dict_set(req, "method", px_str(method));
        px_dict_set(req, "target", px_str(target));
        px_dict_set(req, "path", px_str(path));
        px_dict_set(req, "query", px_str(query));
        px_dict_set(req, "version", px_str(ver));
        px_dict_set(req, "headers", headers);
        px_dict_set(req, "request_id", px_str(req_id));
        if (body_tmp_path[0]) {
            px_dict_set(req, "body", px_str(""));
            px_dict_set(req, "body_tmp", px_str(body_tmp_path));
        } else {
            px_dict_set(req, "body", px_str_len(body_buf, body_len));
        }
        {
            LXValue ckv = px_header_get(&headers, "Cookie");
            if (ckv.type == PX_STR) {
                px_dict_set(req, "cookie", px_parse_cookie(ckv.as.obj->as.str.data));
            } else {
                px_dict_set(req, "cookie", px_dict());
            }
        }
        LXValue form = px_dict();
        {
            struct sockaddr_in raddr;
            socklen_t rl = sizeof(raddr);
            if (getpeername(fd, (struct sockaddr*)&raddr, &rl) == 0) {
                char rbuf[64];
                snprintf(rbuf, sizeof(rbuf), "%s:%d", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
                px_dict_set(req, "remote", px_str(rbuf));
            } else {
                px_dict_set(req, "remote", px_str(""));
            }
        }
        LXValue ct_v = px_dict_get_ci(headers, "Content-Type");
        const char* ct = (ct_v.type == PX_STR) ? ct_v.as.obj->as.str.data : "";
        if (body_len > 0 && body_buf) {
            if (strstr(ct, "multipart/form-data")) {
                char* boundary = px_mime_boundary(ct);
                if (boundary) {
                    px_parse_multipart(req, body_buf, body_len, boundary);
                    xfree(boundary);
                }
            } else if (strstr(ct, "application/x-www-form-urlencoded")) {
                form = px_parse_urlenc(body_buf);
                px_dict_set(req, "form", form);
            }
        }

        // M28：路由表非空 → 优先匹配路由（method+path 模式 + :id 参数 + 中间件链）
        if (px_route_has()) {
            char extra[256];
            snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
            if (px_route_try_dispatch(&conn, req, method, strcmp(method, "HEAD") == 0,
                                      client_keep_alive, extra)) {
                goto req_done;
            }
        }

        // 6. 路径映射 + 目录隔离（穿越防护：拒绝 ".." 路径段）
        LXValue root_v = px_get_global("__px_docroot");
        const char* docroot = (root_v.type == PX_STR) ? root_v.as.obj->as.str.data : ".";
        LXValue tout_v = px_get_global("__px_timeout");
        int timeout_ms = (tout_v.type == PX_INT) ? (int)tout_v.as.i : 10000;
        LXValue port_v = px_get_global("__px_port");
        int port = (port_v.type == PX_INT) ? (int)port_v.as.i : 0;

        int head_only = strcmp(method, "HEAD") == 0;
        const char* pp = path;
        int forbid = 0;
        while (*pp) {
            if (pp[0] == '.' && pp[1] == '.' && (pp[2] == 0 || pp[2] == '/')) { forbid = 1; break; }
            pp++;
        }
        if (forbid) {
            char extra[256];
            snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
            px_px_send_ex(fd, 403, "text/plain; charset=utf-8", "403 Forbidden: 路径穿越被拒绝", 30, head_only, client_keep_alive, extra);
            goto req_done;
        }
        char full[4096];
        snprintf(full, sizeof(full), "%s%s", docroot, path);

        struct stat st;
        if (stat(full, &st) != 0) {
            char extra[256];
            snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
            px_px_send_ex(fd, 404, "text/plain; charset=utf-8", "404 Not Found", 13, head_only, client_keep_alive, extra);
            goto req_done;
        }
        char fpath[4096];
        if (S_ISDIR(st.st_mode)) {
            snprintf(fpath, sizeof(fpath), "%s/index.px", full);
            if (stat(fpath, &st) != 0) {
                snprintf(fpath, sizeof(fpath), "%s/index.html", full);
                if (stat(fpath, &st) != 0) {
                    char extra[256];
                    snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                    px_px_send_ex(fd, 404, "text/plain; charset=utf-8", "404 Not Found", 13, head_only, client_keep_alive, extra);
                    goto req_done;
                }
            }
        } else {
            snprintf(fpath, sizeof(fpath), "%s", full);
        }

        int is_px = strstr(fpath, ".px") != NULL && strcmp(fpath + strlen(fpath) - 3, ".px") == 0;

        if (is_px) {
            // ---- .px 脚本执行：fork + exec `px run`，PX_INIT_GLOBALS 传递请求上下文 ----
            LXValue get = px_parse_urlenc(query);
            LXValue post = px_dict_get(req, "form");
            if (post.type != PX_DICT) post = px_dict();
            LXValue server = px_dict();
            px_dict_set(server, "port", px_int(port));
            px_dict_set(server, "docroot", px_str(docroot));
            px_dict_set(server, "script", px_str(fpath));
            px_dict_set(server, "px", px_str("0.1.0"));
            LXValue env = px_dict();
            px_dict_set(env, "REQUEST", req);
            px_dict_set(env, "GET", get);
            px_dict_set(env, "POST", post);
            px_dict_set(env, "SERVER", server);
            LXValue j = px_call(px_get_global("json_stringify"), &env, 1);
            char* env_json = (j.type == PX_STR) ? strdup(j.as.obj->as.str.data) : NULL;

            char* out = NULL;
            int out_len = 0, exit_code = 0;
            int rc = px_pool_run(fpath, env_json, 1, timeout_ms, &out, &out_len, &exit_code);
            if (env_json) free(env_json);
            char extra[256];
            snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
            if (rc == 1) {
                char msg[128];
                int ml = snprintf(msg, sizeof(msg), "504 Gateway Timeout: 脚本执行超时（>%dms）", timeout_ms);
                px_px_send_ex(fd, 504, "text/plain; charset=utf-8", msg, ml, head_only, client_keep_alive, extra);
            } else if (exit_code != 0) {
                char msg[70000];
                int ml = snprintf(msg, sizeof(msg), "500 Internal Server Error\n\n%.60000s", out ? out : "");
                px_px_send_ex(fd, 500, "text/plain; charset=utf-8", msg, ml, head_only, client_keep_alive, extra);
            } else {
                int status = 200;
                const char* ct2 = "text/html; charset=utf-8";
                char* body = out;
                int blen = out_len;
                char* marker = out ? strstr(out, "__PX_RESPONSE__:") : NULL;
                if (marker) {
                    *marker = 0;
                    blen = (int)(marker - out);
                    while (blen > 0 && (body[blen - 1] == '\n' || body[blen - 1] == '\r')) blen--;
                    char* jstr = marker + 16;
                    LXValue jv = px_str(jstr);
                    LXValue resp = px_call(px_get_global("json_parse"), &jv, 1);
                    if (resp.type == PX_DICT) {
                        LXValue stv = px_dict_get(resp, "status");
                        if (stv.type == PX_INT) status = (int)stv.as.i;
                        LXValue b = px_dict_get(resp, "body");
                        if (b.type == PX_STR) {
                            body = b.as.obj->as.str.data;
                            blen = b.as.obj->as.str.len;
                        }
                        LXValue h = px_dict_get(resp, "headers");
                        if (h.type == PX_DICT) {
                            LXObject* ho = h.as.obj;
                            for (int i = 0; i < ho->as.dict.len; i++) {
                                if (strcasecmp(ho->as.dict.keys[i], "Content-Type") == 0 &&
                                    ho->as.dict.vals[i].type == PX_STR) {
                                    ct2 = ho->as.dict.vals[i].as.obj->as.str.data;
                                    break;
                                }
                            }
                        }
                    }
                }
                // M29：gzip 响应压缩（Accept-Encoding: gzip + 文本类 + >1KB）
                char gz_extra[512];
                int hdr_off = snprintf(gz_extra, sizeof(gz_extra), "X-Request-Id: %s\r\n", req_id);
                if (px_resp_gzipable(&headers, ct2, blen)) {
                    int gzlen = 0;
                    char* gz = px_gzip_compress(body ? body : "", blen, &gzlen);
                    if (gz) {
                        hdr_off += snprintf(gz_extra + hdr_off, sizeof(gz_extra) - (size_t)hdr_off,
                                            "Content-Encoding: gzip\r\nVary: Accept-Encoding\r\n");
                        px_px_send_ex(fd, status, ct2, gz, gzlen, head_only, client_keep_alive, gz_extra);
                        xfree(gz);
                        goto script_done;
                    }
                }
                px_px_send_ex(fd, status, ct2, body ? body : "", blen, head_only, client_keep_alive, gz_extra);
            script_done:
                if (out) xfree(out);
            }
            // M29c：结构化访问日志
            fprintf(stderr, "[px-access] %lld %s %s %s %d %d req=%s\n",
                    (long long)time(NULL), "script", method, path, 200, 0, req_id);
        } else {
            // ---- 静态文件：ETag / Last-Modified / 304 / Range + 流式（M29b） ----
            struct stat fst;
            if (stat(fpath, &fst) != 0) {
                char extra[256];
                snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                px_px_send_ex(fd, 404, "text/plain; charset=utf-8", "404 Not Found", 13, head_only, client_keep_alive, extra);
                goto req_done;
            }
            long long fsz = (long long)fst.st_size;
            time_t mt = fst.st_mtime;
            char etag[128], last_mod[64];
            snprintf(etag, sizeof(etag), "\"px-%llx-%llx\"", (unsigned long long)mt, (unsigned long long)fsz);
            px_http_date(mt, last_mod, sizeof(last_mod));

            LXValue inm = px_header_get(&headers, "If-None-Match");
            if ((inm.type == PX_STR && (strcmp(inm.as.obj->as.str.data, etag) == 0 ||
                                        strcmp(inm.as.obj->as.str.data, "*") == 0))) {
                char extra[512];
                snprintf(extra, sizeof(extra), "ETag: %s\r\nLast-Modified: %s\r\nX-Request-Id: %s\r\n",
                         etag, last_mod, req_id);
                px_px_send_ex(fd, 304, NULL, "", 0, head_only, client_keep_alive, extra);
                goto req_done;
            }
            LXValue ims = px_header_get(&headers, "If-Modified-Since");
            if (ims.type == PX_STR) {
                struct tm tmv;
                memset(&tmv, 0, sizeof(tmv));
                const char* is = ims.as.obj->as.str.data;
                if (strptime(is, "%a, %d %b %Y %H:%M:%S GMT", &tmv)) {
                    time_t since = timegm(&tmv);
                    if (mt <= since) {
                        char extra[512];
                        snprintf(extra, sizeof(extra), "ETag: %s\r\nLast-Modified: %s\r\nX-Request-Id: %s\r\n",
                                 etag, last_mod, req_id);
                        px_px_send_ex(fd, 304, NULL, "", 0, head_only, client_keep_alive, extra);
                        goto req_done;
                    }
                }
            }

            long long rstart = 0, rend = fsz - 1;
            int is_range = 0;
            LXValue rv = px_header_get(&headers, "Range");
            if (rv.type == PX_STR && fsz > 0) {
                if (px_parse_range(rv.as.obj->as.str.data, fsz, &rstart, &rend)) is_range = 1;
            }
            long long seg_len = rend - rstart + 1;
            int status = is_range ? 206 : 200;
            const char* ct2 = px_mime_type(fpath);
            char extra[1024];
            int eo = snprintf(extra, sizeof(extra), "ETag: %s\r\nLast-Modified: %s\r\nX-Request-Id: %s\r\n",
                              etag, last_mod, req_id);
            if (is_range) {
                eo += snprintf(extra + eo, sizeof(extra) - (size_t)eo,
                               "Content-Range: bytes %lld-%lld/%lld\r\nAccept-Ranges: bytes\r\n",
                               rstart, rend, fsz);
            }

            // gzip：整文件 200 + 文本 + Accept-Encoding: gzip + >1KB → 读全压缩
            if (!is_range && px_resp_gzipable(&headers, ct2, (int)fsz)) {
                char* data = xmalloc((size_t)fsz + 1);
                FILE* gz_in = fopen(fpath, "rb");
                if (gz_in) {
                    size_t got = fread(data, 1, (size_t)fsz, gz_in);
                    fclose(gz_in);
                    if (got == (size_t)fsz) {
                        int gzlen = 0;
                        char* gz = px_gzip_compress(data, (int)fsz, &gzlen);
                        if (gz) {
                            char hd[2048];
                            int ho = snprintf(hd, sizeof(hd),
                                              "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n%sContent-Encoding: gzip\r\nVary: Accept-Encoding\r\n\r\n",
                                              gzlen, client_keep_alive ? "keep-alive" : "close", ct2, extra);
                            if (g_cur_conn && g_cur_conn->is_tls) {
                                px_conn_write(g_cur_conn, hd, (size_t)ho);
                                if (!head_only) px_conn_write(g_cur_conn, gz, (size_t)gzlen);
                            } else {
                                send(fd, hd, ho, 0);
                                if (!head_only) send(fd, gz, (size_t)gzlen, 0);
                            }
                            xfree(gz);
                            xfree(data);
                            goto req_done;
                        }
                    }
                    xfree(data);
                }
            }

            // 响应头（HEAD 只发头）
            char hd[2048];
            int ho = snprintf(hd, sizeof(hd),
                              "HTTP/1.1 %d %s\r\nContent-Length: %lld\r\nConnection: %s\r\nContent-Type: %s\r\n%s\r\n",
                              status, px_http_status_reason(status), seg_len,
                              client_keep_alive ? "keep-alive" : "close", ct2, extra);
            if (g_cur_conn && g_cur_conn->is_tls) {
                px_conn_write(g_cur_conn, hd, (size_t)ho);
            } else {
                send(fd, hd, ho, 0);
            }
            // 流式发送文件段（64KB 块，不整读进内存）
            if (!head_only && seg_len > 0) {
                FILE* f = fopen(fpath, "rb");
                if (f) {
                    fseeko(f, (off_t)rstart, SEEK_SET);
                    long long remain = seg_len;
                    char chunk[65536];
                    while (remain > 0) {
                        size_t want = (size_t)(remain < 65536 ? remain : 65536);
                        size_t got = fread(chunk, 1, want, f);
                        if (got == 0) break;
                        if (g_cur_conn && g_cur_conn->is_tls) px_conn_write(g_cur_conn, chunk, got);
                        else send(fd, chunk, (int)got, 0);
                        remain -= (long long)got;
                    }
                    fclose(f);
                }
            }
            // M29c：结构化访问日志
            fprintf(stderr, "[px-access] %lld %s %s %s %d %lld req=%s\n",
                    (long long)time(NULL), "static", method, path, status, seg_len, req_id);
        }

    req_done:
        if (body_tmp_path[0]) unlink(body_tmp_path);
        if (body_tmp_file >= 0) close(body_tmp_file);
        if (body_buf) xfree(body_buf);
        px_reset_request_state();
        if (!client_keep_alive) break;
    }
    px_conn_close(&conn);
    __sync_fetch_and_sub(&g_px_inflight, 1);
    g_cur_conn = NULL;
    return px_null();
}

static LXValue bi_px_serve(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 4) px_error("px_serve 需要 (port, docroot[, timeout_ms[, opts]]) 参数");
    if (args[0].type != PX_INT) px_error("px_serve 的 port 需要整数");
    if (args[1].type != PX_STR) px_error("px_serve 的 docroot 需要字符串");
    const char* docroot = args[1].as.obj->as.str.data;
    struct stat st;
    if (stat(docroot, &st) != 0 || !S_ISDIR(st.st_mode)) {
        px_error("px_serve: docroot 不是有效目录: %s", docroot);
    }
    int timeout_ms = 10000;
    if (nargs >= 3 && args[2].type == PX_INT) timeout_ms = (int)args[2].as.i;
    if (timeout_ms < 1) timeout_ms = 1;
    int port = (int)args[0].as.i;
    // M27：opts = {max_body_size, body_tmp_dir}
    g_px_max_body = 10 * 1024 * 1024;
    if (nargs >= 4 && args[3].type == PX_DICT) {
        LXValue mb = px_dict_get(args[3], "max_body_size");
        if (mb.type == PX_INT) g_px_max_body = (int)(mb.as.i >= 1024 ? mb.as.i : 1024);
    }
    // docroot / timeout / port 存全局表（GC 扫描根）
    px_set_global("__px_docroot", px_str(docroot));
    px_set_global("__px_timeout", px_int(timeout_ms));
    px_set_global("__px_port", px_int(port));

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) px_error("px_serve: socket 创建失败");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        px_error("px_serve: 绑定端口 %d 失败", port);
    }
    if (listen(sfd, 128) < 0) {
        close(sfd);
        px_error("px_serve: listen 失败");
    }
    // M27：优雅关闭（SIGINT/SIGTERM → 停止 accept，等待在途请求）
    g_px_stop = 0;
    g_px_listen_fd = sfd;
    signal(SIGINT, px_sigstop_handler);
    signal(SIGTERM, px_sigstop_handler);
    px_session_sweep();
    fprintf(stderr, "[px-serve] 普贤应用服务器 docroot=%s 端口=%d 超时=%dms tls=%d max_body=%d\n",
            docroot, port, timeout_ms, g_srv_tls_ready, g_px_max_body);
    for (;;) {
        if (g_px_stop) break;
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            if (g_px_stop) break;
            continue;
        }
        LXValue arg = px_int(cfd);
        px_spawn(px_conn_worker, &arg, 1);
    }
    g_px_listen_fd = -1;
    close(sfd);
    // 等待在途请求（最多 5s）
    for (int i = 0; i < 100 && g_px_inflight > 0; i++) {
        struct timespec ts = {0, 50 * 1000 * 1000};
        nanosleep(&ts, NULL);
    }
    fprintf(stderr, "[px-serve] 优雅关闭完成（在途 %d）\n", g_px_inflight);
    return px_null();
}

// px_exec(path, params?)：子进程执行 `px run` 并捕获 stdout
// 文件不存在 → null；params dict → PX_INIT_GLOBALS 注入脚本全局变量
static LXValue bi_px_exec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2) px_error("px_exec 需要 (path[, params]) 参数");
    if (args[0].type != PX_STR) px_error("px_exec 的 path 需要字符串");
    const char* path = args[0].as.obj->as.str.data;
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) return px_null();
    char* env_json = NULL;
    LXValue params = (nargs == 2) ? args[1] : px_null();
    // params 为 dict 或 null/缺省 时都构建默认 env（与解释器模式一致：补默认
    // REQUEST/GET/POST/SERVER 空值，Web 风格脚本在非 Web 语境下也能安全运行；
    // dict 的键注入为全局变量，可覆盖默认 4 个）
    if (params.type == PX_DICT || params.type == PX_NULL) {
        LXValue env = px_dict();
        px_dict_set(env, "REQUEST", px_dict());
        px_dict_set(env, "GET", px_dict());
        px_dict_set(env, "POST", px_dict());
        LXValue srv = px_dict();
        px_dict_set(srv, "px", px_str("0.1.0"));
        px_dict_set(env, "SERVER", srv);
        if (params.type == PX_DICT) {
            LXObject* o = params.as.obj;
            for (int i = 0; i < o->as.dict.len; i++) {
                px_dict_set(env, o->as.dict.keys[i], o->as.dict.vals[i]);
            }
        }
        LXValue j = px_call(px_get_global("json_stringify"), &env, 1);
        if (j.type == PX_STR) env_json = strdup(j.as.obj->as.str.data);
    }
    char* out = NULL;
    int out_len = 0, exit_code = 0;
    int rc = px_pool_run(path, env_json, 0, 0, &out, &out_len, &exit_code);
    if (env_json) free(env_json);
    LXValue r = rc == 0 ? px_str_len(out, out_len) : px_str("");
    if (out) xfree(out);
    return r;
}

// ==================== M28 P1：时间 / 时区 ====================
// 纯整数民用日历算法（与解释器 tztime.rs 逐字节一致，双模式确定性）：
// Howard Hinnant days_from_civil / civil_from_days；时区仅 UTC + 固定偏移。
// time_format(ts, fmt[, tz]) → str；time_parse(str, fmt[, tz]) → int|null；tz_offset(tz) → int

// days_from_civil(y, m, d) → 1970-01-01 起天数
static int64_t px_days_from_civil(int64_t y, int64_t m, int64_t d) {
    if (m <= 2) y -= 1;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    int64_t yoe = y - era * 400;
    int64_t mp = (m + 9) % 12;
    int64_t doy = (153 * mp + 2) / 5 + d - 1;
    int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

// civil_from_days(z) → (y, m, d)
static void px_civil_from_days(int64_t z, int64_t* y, int64_t* m, int64_t* d) {
    z += 719468;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    int64_t doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t yy = yoe + era * 400;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    int64_t mp = (5 * doy + 2) / 153;
    int64_t dd = doy - (153 * mp + 2) / 5 + 1;
    int64_t mm = mp < 10 ? mp + 3 : mp - 9;
    *y = mm <= 2 ? yy + 1 : yy;
    *m = mm;
    *d = dd;
}

// tz → 偏移秒；非法返回 0（UTC）
static int64_t px_tz_off(const char* tz) {
    if (!tz || !*tz) return 0;
    if (strcasecmp(tz, "utc") == 0 || strcmp(tz, "Z") == 0) return 0;
    if (tz[0] != '+' && tz[0] != '-') return 0;
    int64_t sign = tz[0] == '-' ? -1 : 1;
    const char* rest = tz + 1;
    int64_t hh = 0, mm = 0;
    const char* colon = strchr(rest, ':');
    if (colon) {
        hh = atoll(rest);
        mm = atoll(colon + 1);
    } else if (strlen(rest) == 4) {
        char hb[3] = {rest[0], rest[1], 0};
        char mb[3] = {rest[2], rest[3], 0};
        hh = atoll(hb);
        mm = atoll(mb);
    } else {
        hh = atoll(rest);
    }
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return 0;
    return sign * (hh * 3600 + mm * 60);
}

// 拆解 epoch 秒 → (y, mo, d, h, mi, s, wd[0=Sun])
static void px_breakdown(int64_t ts, int64_t off, int64_t* y, int64_t* mo, int64_t* d,
                         int64_t* h, int64_t* mi, int64_t* s, int64_t* wd) {
    int64_t local = ts + off;
    int64_t days = local >= 0 ? local / 86400 : -((-local + 86399) / 86400);
    int64_t secs = local - days * 86400;
    if (secs < 0) { secs += 86400; days -= 1; }
    px_civil_from_days(days, y, mo, d);
    *h = secs / 3600;
    *mi = (secs % 3600) / 60;
    *s = secs % 60;
    // 1970-01-01 = 周四(4)
    int64_t w = (4 + days) % 7;
    if (w < 0) w += 7;
    *wd = w;
}

static const char* PX_WEEKDAYS_S[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char* PX_WEEKDAYS_F[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
static const char* PX_MONTHS_S[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char* PX_MONTHS_F[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

// time_format(ts, fmt[, tz]) → str
LXValue bi_time_format(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("time_format 需要 (ts, fmt[, tz]) 参数");
    if (args[0].type != PX_INT || args[1].type != PX_STR) px_error("time_format 参数类型错误");
    int64_t off = 0;
    if (nargs == 3 && args[2].type == PX_STR) off = px_tz_off(args[2].as.obj->as.str.data);
    int64_t y, mo, d, h, mi, s, wd;
    px_breakdown(args[0].as.i, off, &y, &mo, &d, &h, &mi, &s, &wd);
    const char* fmt = args[1].as.obj->as.str.data;
    char out[512];
    int oi = 0;
    for (const char* p = fmt; *p && oi < 500; p++) {
        if (*p == '%' && p[1]) {
            p++;
            switch (*p) {
                case 'Y': oi += snprintf(out + oi, 512 - oi, "%04lld", (long long)y); break;
                case 'y': oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)(y % 100 + (y < 0 ? 100 : 0) - (y < 0 ? 100 : 0))); break;
                case 'm': oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)mo); break;
                case 'd': oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)d); break;
                case 'H': oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)h); break;
                case 'M': oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)mi); break;
                case 'S': oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)s); break;
                case 'j': {
                    int64_t doy = px_days_from_civil(y, mo, d) - px_days_from_civil(y, 1, 1) + 1;
                    oi += snprintf(out + oi, 512 - oi, "%03lld", (long long)doy);
                    break;
                }
                case 'a': oi += snprintf(out + oi, 512 - oi, "%s", PX_WEEKDAYS_S[wd]); break;
                case 'A': oi += snprintf(out + oi, 512 - oi, "%s", PX_WEEKDAYS_F[wd]); break;
                case 'b': case 'h': oi += snprintf(out + oi, 512 - oi, "%s", PX_MONTHS_S[mo - 1]); break;
                case 'B': oi += snprintf(out + oi, 512 - oi, "%s", PX_MONTHS_F[mo - 1]); break;
                case 'p': oi += snprintf(out + oi, 512 - oi, "%s", h < 12 ? "AM" : "PM"); break;
                case 'z': {
                    int64_t a = off < 0 ? -off : off;
                    oi += snprintf(out + oi, 512 - oi, "%c%02lld%02lld", off < 0 ? '-' : '+',
                                   (long long)(a / 3600), (long long)((a % 3600) / 60));
                    break;
                }
                case 'Z': {
                    if (off == 0) {
                        oi += snprintf(out + oi, 512 - oi, "UTC");
                    } else {
                        int64_t a = off < 0 ? -off : off;
                        oi += snprintf(out + oi, 512 - oi, "%c%02lld%02lld", off < 0 ? '-' : '+',
                                       (long long)(a / 3600), (long long)((a % 3600) / 60));
                    }
                    break;
                }
                case 'I': {
                    int64_t h12 = h % 12;
                    oi += snprintf(out + oi, 512 - oi, "%02lld", (long long)(h12 == 0 ? 12 : h12));
                    break;
                }
                case '%': out[oi++] = '%'; break;
                default: out[oi++] = '%'; out[oi++] = *p; break;
            }
        } else {
            out[oi++] = *p;
        }
    }
    out[oi] = 0;
    return px_str(out);
}

// 读连续数字（最多 max 位）→ 成功返回 1 并更新 idx
static int px_read_int(const char* b, int len, int* idx, int max, int64_t* v) {
    int i = *idx;
    int start = i;
    while (i < len && b[i] >= '0' && b[i] <= '9' && (i - start) < max) i++;
    if (i == start) return 0;
    *v = atoll(b + start);
    *idx = i;
    return 1;
}

// time_parse(str, fmt[, tz]) → int|null
LXValue bi_time_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("time_parse 需要 (str, fmt[, tz]) 参数");
    if (args[0].type != PX_STR || args[1].type != PX_STR) px_error("time_parse 参数类型错误");
    int64_t off = 0;
    if (nargs == 3 && args[2].type == PX_STR) off = px_tz_off(args[2].as.obj->as.str.data);
    const char* s = args[0].as.obj->as.str.data;
    int slen = args[0].as.obj->as.str.len;
    const char* fmt = args[1].as.obj->as.str.data;
    int si = 0, fi = 0;
    int64_t year = 1970, mon = 1, day = 1, hour = 0, minute = 0, sec = 0;
    int64_t parsed_z = 0;
    int have_z = 0;
    while (fmt[fi]) {
        if (fmt[fi] == '%' && fmt[fi + 1]) {
            fi++;
            switch (fmt[fi]) {
                case 'Y': if (!px_read_int(s, slen, &si, 4, &year)) return px_null(); break;
                case 'y': {
                    int64_t v;
                    if (!px_read_int(s, slen, &si, 2, &v)) return px_null();
                    year = v < 69 ? 2000 + v : 1900 + v;
                    break;
                }
                case 'm': if (!px_read_int(s, slen, &si, 2, &mon)) return px_null(); break;
                case 'd': if (!px_read_int(s, slen, &si, 2, &day)) return px_null(); break;
                case 'H': case 'I': if (!px_read_int(s, slen, &si, 2, &hour)) return px_null(); break;
                case 'M': if (!px_read_int(s, slen, &si, 2, &minute)) return px_null(); break;
                case 'S': if (!px_read_int(s, slen, &si, 2, &sec)) return px_null(); break;
                case 'z': case 'Z': {
                    if (si < slen && (s[si] == '+' || s[si] == '-')) {
                        int end = si + 5;
                        if (si + 3 < slen && s[si + 3] == ':') end = si + 6;
                        if (end > slen) return px_null();
                        char buf[16];
                        memcpy(buf, s + si, (size_t)(end - si));
                        buf[end - si] = 0;
                        parsed_z = px_tz_off(buf);
                        have_z = 1;
                        si = end;
                    } else if (si + 2 < slen && strncmp(s + si, "UTC", 3) == 0) {
                        parsed_z = 0;
                        have_z = 1;
                        si += 3;
                    }
                    break;
                }
                default:
                    if (si < slen && s[si] == fmt[fi]) si++;
                    break;
            }
        } else {
            if (fmt[fi] == ' ') {
                while (si < slen && s[si] == ' ') si++;
            } else {
                if (si >= slen || s[si] != fmt[fi]) return px_null();
                si++;
            }
        }
        fi++;
    }
    if (mon < 1 || mon > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || sec > 60) return px_null();
    int64_t use_off = have_z ? parsed_z : off;
    int64_t days = px_days_from_civil(year, mon, day);
    // 校验日真实存在
    int64_t y2, m2, d2;
    px_civil_from_days(days, &y2, &m2, &d2);
    if (y2 != year || m2 != mon || d2 != day) return px_null();
    return px_int(days * 86400 + hour * 3600 + minute * 60 + sec - use_off);
}

// tz_offset(tz) → int|null
LXValue bi_tz_offset(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("tz_offset 需要 (tz) 参数");
    const char* tz = args[0].as.obj->as.str.data;
    // 非法时区 → null（与解释器一致）；合法返回偏移
    if (strcasecmp(tz, "utc") == 0 || strcmp(tz, "Z") == 0) return px_int(0);
    if (tz[0] != '+' && tz[0] != '-') return px_null();
    const char* rest = tz + 1;
    int ok = 0;
    if (strchr(rest, ':')) ok = 1;
    else if (strlen(rest) == 4 || strlen(rest) == 2 || strlen(rest) == 1) ok = 1;
    if (!ok) return px_null();
    int64_t off = px_tz_off(tz);
    // px_tz_off 非法返回 0；此处区分"合法 0 偏移"与"非法"
    if (strchr(rest, ':')) {
        const char* colon = strchr(rest, ':');
        int64_t hh = atoll(rest), mm = atoll(colon + 1);
        if (hh > 23 || mm > 59) return px_null();
    } else if (strlen(rest) == 4) {
        int64_t hh = atoll(rest);
        if (hh > 23) return px_null();
    }
    return px_int(off);
}

// ==================== M28 P1：cron 定时调度 ====================
// cron(expr, fn, ...args) → int：6 字段（秒 分 时 日 月 周；周 0/7=周日）
// 每 cron 任务一个线程，每秒 tick 检查匹配 → px_call 触发；clear_timer(id) 取消。
// 表达式解析与解释器 cron.rs 同一语义（*/n、a,b、a-b、a-b/n、固定值；日/周双受限 OR）。

typedef struct {
    int64_t id;
    int active;
    char expr[256];
    // 位集合
    unsigned char sec[60];
    unsigned char min[60];
    unsigned char hour[24];
    unsigned char dom[32];
    unsigned char mon[13];
    unsigned char dow[7];
    int dom_limited, dow_limited;
    LXValue fn;
    LXValue* args;
    int nargs;
} CronJob;

#define MAX_CRON 64
static CronJob g_crons[MAX_CRON];
static pthread_mutex_t g_cron_mu = PTHREAD_MUTEX_INITIALIZER;
static int64_t g_cron_next_id = 0;

// 解析单个 cron 字段 → 位集合（bits 长度 max-min+1；成功返回 1）
static int cron_parse_field(const char* expr, int min, int max, unsigned char* bits) {
    memset(bits, 0, (size_t)(max - min + 1));
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", expr);
    int any = 0;
    char* save = NULL;
    for (char* part = strtok_r(buf, ",", &save); part; part = strtok_r(NULL, ",", &save)) {
        if (!*part) return 0;
        if (strcmp(part, "*") == 0) {
            for (int v = min; v <= max; v++) bits[v - min] = 1;
            any = 1;
            continue;
        }
        char* slash = strchr(part, '/');
        char range[128];
        int step = 1;
        if (slash) {
            *slash = 0;
            step = atoi(slash + 1);
            if (step <= 0) return 0;
        }
        snprintf(range, sizeof(range), "%s", part);
        int64_t lo, hi;
        char* dash = strchr(range, '-');
        if (strcmp(range, "*") == 0) {
            lo = min; hi = max;
        } else if (dash) {
            *dash = 0;
            lo = atoll(range);
            hi = atoll(dash + 1);
        } else {
            lo = hi = atoll(range);
        }
        if (lo < min || hi > max || lo > hi) return 0;
        for (int64_t v = lo; v <= hi; v += step) bits[v - min] = 1;
        any = 1;
    }
    return any;
}

static int cron_parse_expr(const char* expr, CronJob* job) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", expr);
    char* parts[6];
    int n = 0;
    char* save = NULL;
    for (char* t = strtok_r(buf, " \t", &save); t && n < 6; t = strtok_r(NULL, " \t", &save)) {
        parts[n++] = t;
    }
    if (n != 6) return 0;
    if (!cron_parse_field(parts[0], 0, 59, job->sec)) return 0;
    if (!cron_parse_field(parts[1], 0, 59, job->min)) return 0;
    if (!cron_parse_field(parts[2], 0, 23, job->hour)) return 0;
    unsigned char dom_raw[32] = {0}, mon_raw[13] = {0};
    if (!cron_parse_field(parts[3], 1, 31, dom_raw)) return 0;
    if (!cron_parse_field(parts[4], 1, 12, mon_raw)) return 0;
    unsigned char dow8[8] = {0};
    if (!cron_parse_field(parts[5], 0, 7, dow8)) return 0;
    memset(job->dom, 0, sizeof(job->dom));
    for (int i = 1; i <= 31; i++) job->dom[i] = dom_raw[i - 1];
    memset(job->mon, 0, sizeof(job->mon));
    for (int i = 1; i <= 12; i++) job->mon[i] = mon_raw[i - 1];
    memset(job->dow, 0, sizeof(job->dow));
    for (int i = 0; i < 8; i++) if (dow8[i]) job->dow[i % 7] = 1;
    job->dom_limited = strcmp(parts[3], "*") != 0;
    job->dow_limited = strcmp(parts[5], "*") != 0;
    return 1;
}

static int cron_match(const CronJob* job, int64_t ts) {
    int64_t y, mo, d, h, mi, s, wd;
    px_breakdown(ts, 0, &y, &mo, &d, &h, &mi, &s, &wd);
    if (!job->sec[s] || !job->min[mi] || !job->hour[h] || !job->mon[mo]) return 0;
    int dom_ok = job->dom[d];
    int dow_ok = job->dow[wd];
    if (job->dom_limited && job->dow_limited) return dom_ok || dow_ok;
    if (job->dom_limited) return dom_ok;
    if (job->dow_limited) return dow_ok;
    return 1;
}

static int cron_still_active(int64_t id) {
    pthread_mutex_lock(&g_cron_mu);
    int a = 0;
    for (int i = 0; i < MAX_CRON; i++) {
        if (g_crons[i].active && g_crons[i].id == id) { a = g_crons[i].active; break; }
    }
    pthread_mutex_unlock(&g_cron_mu);
    return a;
}

static void cron_release(int64_t id) {
    pthread_mutex_lock(&g_cron_mu);
    for (int i = 0; i < MAX_CRON; i++) {
        if (g_crons[i].active && g_crons[i].id == id) {
            g_crons[i].active = 0;
            g_crons[i].id = 0;
            break;
        }
    }
    pthread_mutex_unlock(&g_cron_mu);
    // 释放 g_timers 槽位（clear_timer 的取消标记消费后清理）
    pthread_mutex_lock(&g_timer_mu);
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (g_timers[i].id == id) {
            g_timers[i].active = 0;
            g_timers[i].id = 0;
            break;
        }
    }
    pthread_mutex_unlock(&g_timer_mu);
}

// cron 任务线程：每秒 tick 检查匹配 → 触发回调
static void* cron_thread(void* p) {
    CronJob* job = (CronJob*)p;
    // 注册 GC 槽位（同 timer_thread）
    pthread_mutex_lock(&g_gc_mu);
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) {
        if (g_threads[i].in_use && (uintptr_t)g_threads[i].tid == 0) {
            g_threads[i].tid = pthread_self();
            break;
        }
    }
    pthread_mutex_unlock(&g_gc_mu);
    LXValue fn = job->fn;
    LXValue args_stack[16];
    int nargs = job->nargs < 16 ? job->nargs : 16;
    for (int i = 0; i < nargs; i++) args_stack[i] = job->args[i];
    int64_t id = job->id;
    for (;;) {
        struct timespec ts = {1, 0};
        nanosleep(&ts, NULL);
        // 与 set_timeout/set_interval 同一取消机制（clear_timer 统一生效）
        if (!timer_still_active(id)) break;
        int64_t now = (int64_t)time(NULL);
        if (cron_match(job, now)) {
            LXValue r = px_call(fn, args_stack, nargs);
            (void)r;
        }
    }
    cron_release(id);
    pthread_mutex_lock(&g_gc_mu);
    g_active_threads--;
    gc_unregister_thread(pthread_self());
    pthread_mutex_unlock(&g_gc_mu);
    if (job->args) xfree(job->args);
    xfree(job);
    return NULL;
}

// cron(expr, fn, ...args) → int
LXValue bi_cron(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2) px_error("cron 需要 (expr, fn[, ...args]) 参数");
    if (args[0].type != PX_STR || args[1].type != PX_FUNC) px_error("cron 参数类型错误");
    pthread_mutex_lock(&g_cron_mu);
    int slot = -1;
    for (int i = 0; i < MAX_CRON; i++) if (!g_crons[i].active) { slot = i; break; }
    if (slot < 0) {
        pthread_mutex_unlock(&g_cron_mu);
        px_error("cron 任务数量超出上限 %d", MAX_CRON);
    }
    // id 与 set_timeout 同一序列 + g_timers 槽位 → clear_timer(id) 统一取消
    int64_t id;
    pthread_mutex_lock(&g_timer_mu);
    int tslot = -1;
    for (int i = 0; i < MAX_TIMERS; i++) if (g_timers[i].id == 0) { tslot = i; break; }
    if (tslot < 0) {
        pthread_mutex_unlock(&g_timer_mu);
        pthread_mutex_unlock(&g_cron_mu);
        px_error("定时器数量超出上限 %d", MAX_TIMERS);
    }
    id = ++g_next_timer_id;
    g_timers[tslot].id = id;
    g_timers[tslot].active = 1;
    pthread_mutex_unlock(&g_timer_mu);
    memset(&g_crons[slot], 0, sizeof(CronJob));
    g_crons[slot].id = id;
    g_crons[slot].active = 1;
    snprintf(g_crons[slot].expr, sizeof(g_crons[slot].expr), "%s", args[0].as.obj->as.str.data);
    pthread_mutex_unlock(&g_cron_mu);

    // 解析表达式（非法 → 回滚 + 报错）
    if (!cron_parse_expr(args[0].as.obj->as.str.data, &g_crons[slot])) {
        pthread_mutex_lock(&g_cron_mu);
        g_crons[slot].active = 0;
        g_crons[slot].id = 0;
        pthread_mutex_unlock(&g_cron_mu);
        pthread_mutex_lock(&g_timer_mu);
        for (int i = 0; i < MAX_TIMERS; i++) {
            if (g_timers[i].id == id) { g_timers[i].active = 0; g_timers[i].id = 0; break; }
        }
        pthread_mutex_unlock(&g_timer_mu);
        px_error("cron 表达式非法: %s", args[0].as.obj->as.str.data);
    }

    // GC 槽位预留（同 px_timer_create）
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    g_active_threads++;
    int gslot = -1;
    for (int i = 0; i < MAX_SPAWN_THREADS; i++) if (!g_threads[i].in_use) { gslot = i; break; }
    if (gslot >= 0) {
        memset(&g_threads[gslot], 0, sizeof(g_threads[gslot]));
        g_threads[gslot].in_use = 1;
        g_threads[gslot].is_main = 0;
    } else {
        g_active_threads--;
    }
    pthread_mutex_unlock(&g_gc_mu);
    if (gslot < 0) {
        cron_release(id);
        px_error("cron: 并发线程数超出上限 %d", MAX_SPAWN_THREADS);
    }

    CronJob* job = (CronJob*)malloc(sizeof(CronJob));
    memcpy(job, &g_crons[slot], sizeof(CronJob));
    job->fn = args[1];
    job->nargs = nargs - 2;
    job->args = (LXValue*)malloc(sizeof(LXValue) * (job->nargs > 0 ? job->nargs : 1));
    if (job->nargs > 0) memcpy(job->args, args + 2, sizeof(LXValue) * (size_t)job->nargs);
    pthread_t t;
    if (pthread_create(&t, NULL, cron_thread, job) != 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_active_threads--;
        if (gslot >= 0) g_threads[gslot].in_use = 0;
        pthread_mutex_unlock(&g_gc_mu);
        cron_release(id);
        free(job->args);
        free(job);
        px_error("cron: 创建线程失败");
    }
    if (gslot >= 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_threads[gslot].tid = t;
        pthread_mutex_unlock(&g_gc_mu);
    }
    pthread_detach(t);
    return px_int(id);
}
