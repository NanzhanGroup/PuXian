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
#include <sys/un.h>       // M56: http_unix（Unix domain socket HTTP 客户端，本地网关/服务调用）
#include <sys/ioctl.h>    // M57-S1: ioctl（i2c-dev/spi-dev/gpio/tty/网卡 设备控制）
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <termios.h>    // M60-S2: tty_config（串口 termios 波特率/raw 模式）
#include <errno.h>
#include <strings.h>
#include <signal.h>
#include <setjmp.h>       // M57-S4：musl 无 getcontext → GC 寄存器 spill 用 setjmp 替代
#include <ucontext.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/resource.h>  // M31 沙箱：setrlimit RLIMIT_AS
#include <poll.h>
#include <stdatomic.h>
#if defined(__GLIBC__)
#include <execinfo.h>   // M57-S4：glibc 特有头，musl 交叉（边缘设备）无此头 → 条件包含
#endif
#include "miniz.h"   // M21 gzip 压缩/解压（raw deflate + gzip 容器，M19 zip 同源）
#include "arch.h"    // M67-S4：GC 架构抽象层（arch_read_sp / arch_scan_registers / arch_uc_sp）

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
// M33：TLS SNI 多证书表（tls_server(cert, key, hostname) 多次注册；按 ClientHello SNI 选证书）
#define PX_MAX_SNI_CERTS 16
typedef struct {
    char hostname[256];
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;
    int active;
} PxSniCert;
static PxSniCert g_sni_certs[PX_MAX_SNI_CERTS];
// M30：服务端 https 连接池——TLS 会话缓存（Session ID + 票据），新连接快速恢复握手
static mbedtls_ssl_cache_context g_srv_tls_cache;
static int g_srv_tls_cache_init = 0;
// M101：并发 TLS 握手修复——mbedtls 3.6.2 预编译库未编线程支持（MBEDTLS_THREADING_C 关，
// config 2100/2111/3630 全注释）→ 库内无互斥；px_serve g_pool 多 worker 并发 mbedtls
// 握手共享全局可变对象 = data race（examples/m101_s2 复现：RSA-TLS1.3 48×3 并发新建
// 连接 ok=2 fail=142 'invalid signature by the server certificate: crypto/rsa:
// verification error' + RSA-TLS1.2 144 全 EOF + EC-TLS1.3 ok=140 fail=4 EOF/reset）。
// 修复（S2，三层）：
//   ① per-连接 RSA 私钥 clone（px_pk_clone_rsa → c->own_pk/own_pk_sni）：签名写共享
//      g_srv_key 消除（RSA CRT 推导/窗口缓存写 ctx → 并发签名错——主因，142/144）；
//   ② session cache 加锁包装（px_srv_cache_get/set 包 g_srv_cache_mu）：无锁链表
//      get/set 竞争（EC 证书残余 4 失败 + TLS1.2 叠加）——次因；
//   ③ **全局握手串行锁** g_srv_hs_mu 包整个 px_conn_tls_handshake（px_conn_tls_handshake
//      包装 / _locked 实现）：clone+cache 锁后 RSA-TLS1.3 仍残余 ~1% 并发特有
//      MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED(-110) + 偶发堆损坏（串行 300 次零失败；
//      TLS1.3 服务端深层共享无法枚举）→ 串行锁根治。clone（EC 共享只读实测安全）+
//      cache 锁保留作双保险（未来换开 threading 的 mbedtls 可去串行锁仍正确）。
// 每连接 ssl/conf/drbg/entropy 独立（px_conn_init），cert 只读共享 —— 均非竞态源。
// 锁序：hs_mu → tls_mu（sni_cb 同序）→ 无死锁；tls_server 注册仅持 tls_mu。
static pthread_mutex_t g_srv_cache_mu = PTHREAD_MUTEX_INITIALIZER;  // session cache 锁
static pthread_mutex_t g_srv_hs_mu = PTHREAD_MUTEX_INITIALIZER;     // 全局握手串行锁
static LXValue bi_tls_server(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_open(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_id(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_set(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_del(LXValue* args, int nargs, void* ctx);
static LXValue bi_session_destroy(LXValue* args, int nargs, void* ctx);
static LXValue bi_basic_auth(LXValue* args, int nargs, void* ctx);

// ==================== M31 安全 / 多租户 / 并发（沙箱 / 虚拟主机 / 限流 / 连接线程池） ====================
// 沙箱：deny 表（禁用的内置函数名；sandbox_enter 激活后 px_call native 分派检查）
#define PX_SANDBOX_DENY_MAX 64
static char g_sandbox_deny[PX_SANDBOX_DENY_MAX][64];
static int g_sandbox_deny_count = 0;
static int g_sandbox_active = 0;
// px_serve opts：限流（max 次 / window_sec 秒，按 IP；0 = 未启用）
static long long g_px_rate_max = 0;
static long long g_px_rate_window = 0;
// M35：多维度限流——key 组合模式（ip / ip|ua / ip|path / ip|ua|path）+ 白名单 IP
static char g_px_rate_key_mode[32] = "ip";
#define PX_RATE_WL_MAX 64
static char g_px_rate_whitelist[PX_RATE_WL_MAX][64];
static int g_px_rate_whitelist_n = 0;
// M33：access log 落盘路径（px_serve opts{access_log}；空 = 仅 stderr）+ Alt-Svc 通告
char g_px_access_log[1024] = {0};
char g_px_alt_svc[256] = {0};
// M53-S4：px_serve opts.http3 的 H3（QUIC/UDP）listener id（0 = 未启用；优雅关闭时回收）
// M57-S4：PX_NO_QUIC 裁剪（边缘设备交叉编译时去掉 QUIC/H3/ngtcp2/openssl 依赖）
#ifndef PX_NO_QUIC
static long long g_px_h3_listener = 0;
#endif
// M36：access log JSON 行格式 + 按天轮转（日期后缀）
int g_px_log_json = 0;
int g_px_log_daily = 0;
// M37：gzip 响应压缩配置（级别 1-9 / 最小体积阈值）
int g_px_gzip_level = 6;
int g_px_gzip_min = 1024;
// 虚拟主机表（vhost(host, docroot|handler)）
#define MAX_VHOSTS 32
typedef struct {
    char host[128];
    int has_root;
    char root[1024];
    int has_handler;
    LXValue handler;       // handler 函数（px_set_global 保护防 GC 回收）
    int active;
} PxVhost;
static PxVhost g_vhosts[MAX_VHOSTS];
static pthread_mutex_t g_vhost_mu = PTHREAD_MUTEX_INITIALIZER;
static int g_vhost_handler_seq = 0;
// 限流滑动窗口（链表：key → 时间戳环形缓冲）
#define PX_RATE_MAX_BUCKETS 4096
typedef struct RateBucket {
    char key[128];
    long long* times;      // 环形时间戳
    int head, count, cap;
    struct RateBucket* next;
} RateBucket;
static RateBucket* g_rate_head = NULL;
static int g_rate_buckets = 0;
static pthread_mutex_t g_rate_mu = PTHREAD_MUTEX_INITIALIZER;
// M31.4b：连接线程池（常驻 worker 直接注册 GC 槽，不走 spawn 槽位）
#define PX_POOL_MAX 256
static pthread_t g_pool_threads[PX_POOL_MAX];
static int g_pool_fds[PX_POOL_MAX];       // 环形队列（cfd）
static int g_pool_head = 0, g_pool_tail = 0, g_pool_count = 0;
static int g_pool_size = 0;
static pthread_mutex_t g_pool_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_pool_cond = PTHREAD_COND_INITIALIZER;
static LXValue bi_sandbox_enter(LXValue* args, int nargs, void* ctx);
static LXValue bi_vhost(LXValue* args, int nargs, void* ctx);
static LXValue bi_rate_limit(LXValue* args, int nargs, void* ctx);
static LXValue bi_gen_next(LXValue* args, int nargs, void* ctx);
static LXValue bi_list(LXValue* args, int nargs, void* ctx);
static LXValue bi_tuple(LXValue* args, int nargs, void* ctx);
static int px_vhost_resolve(const char* host_hdr, const char* default_root,
                            char* out_root, int out_root_sz, LXValue* out_handler, int* has_handler);
static void px_pool_push(int fd);
static void* px_pool_worker(void* arg);
static void px_vhost_normalize(LXValue v, int* status, const char** ct, const char** body, int* body_len,
                               char* extra, int extra_sz);
static int px_vhost_header_allowed(const char* k);
static void px_vhost_docroot_store(const char* root);
static const char* px_vhost_docroot(void);
int px_rate_limit_try(const char* key, long long max, long long window_sec);

// 前向声明：xmalloc/xfree 在 gc_block_stop 定义之前使用（M11 自由链表分配器）
static void gc_block_stop(sigset_t* old);
static void gc_unblock_stop(const sigset_t* old);

// ---- M105-S3：GC 暂停信号「软屏蔽」临界区（免 sigprocmask 系统调用）----
// 依据（M105-S1 量化，见 docs/M105_PLAN.md §2.4/§2.5/§6.5）：gc_block_stop/unblock_stop 一对
//   = 2 次 rt_sigprocmask 系统调用，实测 **343~372ns**；真实负载 compiler_vm bc yaml.px 中
//   4,384,444 次（field_while/loop_sum 的 sys 时间 ≈ user 时间即此项）——S2 削掉名解析后，
//   这是第一大户（裸跳过 sigmask 实测 3.600s→2.570s，即 S2 之上 1.41x）。
// 做法：临界区不再真正屏蔽 SIG_GC_STOP，改为置本线程 TLS 计数 g_gc_crit；暂停信号处理器
//   入口先查 g_gc_crit，非 0 即「延迟暂停」——**立刻返回**，交 GC executor 既有的重发循环
//   （200us 间隔 + 5s 兜底）稍后重试。
// 为什么仍然安全（关键不变量）：GC 只有在**所有已注册线程都上报本轮 paused** 之后才进入 mark；
//   软屏蔽临界区内的线程永不上报 paused ⇒ **标记期绝不与临界区并发**，与「真屏蔽」完全等价。
//   最坏退化 = 既有的 5 秒兜底跳过（与本改动前「信号被阻塞 → 线程不上报 paused」的行为一致）；
//   常态代价 = 临界区退出后 GC 最多晚 1 个重发周期拿到暂停（真屏蔽是挂起信号在解锁瞬间送达）。
// 层栈：block/unblock 与既有 sigprocmask 用法严格 LIFO 配对，**仅最外层做决策**（g_gcs_skip），
//   内层直接返回——外层已建立排他，嵌套重入决策无二义（防「临界区内 spawn → 谓词翻转」导致
//   内外层判定不一致）。
// 谓词：g_active_threads == 0 —— 与 GC executor 的发送闸门（px_gc_collect 并发路径入口）同一
//   变量、同一判据，即「无人会发 SIG_GC_STOP」⇒ 屏蔽是可证明的空操作。该读值若因并发陈旧
//   （临界区内恰好 spawn 线程致谓词翻转），信号确实可能到达，但处理器只做延迟暂停 ⇒
//   **安全性不依赖该读值的时序**（只影响性能）。
static __thread int g_gcs_depth = 0;         // block/unblock 嵌套深度（本线程；仅最外层决策）
static __thread int g_gcs_skip  = 0;         // 最外层是否走软屏蔽（免 sigprocmask）分支
static __thread volatile int g_gc_crit = 0;  // 软屏蔽临界区标志（信号处理器据此延迟暂停）
static long g_gc_deferred = 0;               // 观测：延迟暂停次数（g_gc_debug 输出）

// std.net（M5.2/M10）前向声明（定义在文件尾部，注册函数在前部使用）
static LXValue bi_tcp_listen(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_accept(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_connect(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_send(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_recv(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_close(LXValue* args, int nargs, void* ctx);
// M33：UDP 基础设施（HTTP/3/QUIC 预研）
static LXValue bi_udp_open(LXValue* args, int nargs, void* ctx);
static LXValue bi_udp_send(LXValue* args, int nargs, void* ctx);
static LXValue bi_udp_recv(LXValue* args, int nargs, void* ctx);
static LXValue bi_udp_close(LXValue* args, int nargs, void* ctx);
// M34：事件总线（pub/sub）
static LXValue bi_event_bus(LXValue* args, int nargs, void* ctx);
static LXValue bi_bus_subscribe(LXValue* args, int nargs, void* ctx);
static LXValue bi_bus_publish(LXValue* args, int nargs, void* ctx);
static LXValue bi_bus_unsubscribe(LXValue* args, int nargs, void* ctx);
// M36：请求上下文（线程局部）
static LXValue bi_ctx_set(LXValue* args, int nargs, void* ctx);
static LXValue bi_ctx_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_ctx_clear(LXValue* args, int nargs, void* ctx);
// M37：S3/MinIO 对象存储（AWS SigV4）
static LXValue bi_s3_put(LXValue* args, int nargs, void* ctx);
static LXValue bi_s3_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_s3_delete(LXValue* args, int nargs, void* ctx);
static LXValue bi_s3_list(LXValue* args, int nargs, void* ctx);
// M38：UDP echo 服务端
static LXValue bi_udp_serve(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_post(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_serve(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_serve_unix(LXValue* args, int nargs, void* ctx); // M8x（Issue 15 GAP-SRV-1）
// M23c P1：HTTP 生产化（http_request 连接池 / http_get_stream 流式下载）
static LXValue bi_http_request(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_get_stream(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_unix(LXValue* args, int nargs, void* ctx); // M56
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
// M83-S6（Issue 19 GAP-SRV-SSE）：http_stream 同端口流式路由（http_serve/http_serve_unix）
static LXValue bi_http_stream(LXValue* args, int nargs, void* ctx);
static int stream_match(const char* path);   // 流式路由表匹配（http_conn_worker 用）
// 流式接管连接（定义在 SSE 注册表区之后）；http_conn_worker 前向引用
static LXValue stream_takeover_conn(int fd, LXValue req, int route_idx);
// M23 P1：SSE 客户端（流式消费 / 事件订阅）
static LXValue bi_sse_connect(LXValue* args, int nargs, void* ctx);
static LXValue bi_sse_read(LXValue* args, int nargs, void* ctx);
// M22 P1：强制垃圾回收（gc()）
static LXValue bi_gc(LXValue* args, int nargs, void* ctx);
// M23 P1：进程/信号（os_pid/os_spawn/os_wait/os_kill/signal）
static LXValue bi_os_pid(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_exec(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_rename(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_remove_all(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_random_hex(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_file_sha256(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_spawn(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_spawn_capture(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_wait(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_kill(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_capture(LXValue* args, int nargs, void* ctx); // M66
static LXValue bi_os_popen(LXValue* args, int nargs, void* ctx);   // M66
static LXValue bi_unix_connect(LXValue* args, int nargs, void* ctx); // M66

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
// M57-S1：边缘设备层 fd 原语（open/close/ioctl/os_errno）
static LXValue bi_open(LXValue* args, int nargs, void* ctx);
static LXValue bi_close(LXValue* args, int nargs, void* ctx);
static LXValue bi_ioctl(LXValue* args, int nargs, void* ctx);
static LXValue bi_os_errno(LXValue* args, int nargs, void* ctx);
// M57-S2：边缘设备层 fd 数据通道 + mmap 设备映射（read/write/mmap/munmap/mem_write）
static LXValue bi_read(LXValue* args, int nargs, void* ctx);
static LXValue bi_write(LXValue* args, int nargs, void* ctx);
static LXValue bi_mmap(LXValue* args, int nargs, void* ctx);
static LXValue bi_munmap(LXValue* args, int nargs, void* ctx);
static LXValue bi_mem_write(LXValue* args, int nargs, void* ctx);
// M60-S1：us 级时钟 + fd 控制（sleep_us/now_us/fcntl）
static LXValue bi_sleep_us(LXValue* args, int nargs, void* ctx);
static LXValue bi_now_us(LXValue* args, int nargs, void* ctx);
static LXValue bi_fcntl(LXValue* args, int nargs, void* ctx);
// M60-S2：设备组（tty_config/fd_wait）
static LXValue bi_tty_config(LXValue* args, int nargs, void* ctx);
static LXValue bi_fd_wait(LXValue* args, int nargs, void* ctx);
// M30 P1：字节序可控整数↔bytes（pxdb 存储基石）
static LXValue bi_int_to_bytes(LXValue* args, int nargs, void* ctx);
static LXValue bi_bytes_to_int(LXValue* args, int nargs, void* ctx);

// M23b 字节辅助（字符串/字节串统一 data+len；供 base64/hex 等前置函数使用）
static const char* bdata(LXValue v);
static int blen(LXValue v);
// M83-S2（Issue 20 GAP-ARC-1）：gzip 语言层通用压缩/解压（px_gzip_* 定义在后方 M21 区）
static LXValue bi_gzip_compress(LXValue* args, int nargs, void* ctx);
static LXValue bi_gzip_uncompress(LXValue* args, int nargs, void* ctx);

// M10 HTTPS 内部辅助
static char* px_http_request(const char* url, const char* method, const char* body, int* out_len, char* errbuf, int errcap);
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
// ISSUE28-B1：xfree 局部性 hint——sweep 成批释放同一批 slab 的对象/数据时，跳过 O(log R)
// 反查二分（实测全量 STW 主要耗时在 sweep 的 xfree：120k 对象 ~250ms，单发 p95 尖刺根因）。
#define XFREE_HINT_N 16
static Slab* g_xfree_hint[XFREE_HINT_N];
static int g_xfree_hint_head = 0;
static __thread int g_in_gc_sweep = 0;  // ISSUE28-B1：GC sweep 中（executor 已屏蔽 SIG_GC_STOP、单线程），xfree 跳过每趟 sigprocmask 屏蔽/恢复（实测全量 STW 耗时大头）
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
    int blk = !g_in_gc_sweep;   // ISSUE28-B1：sweep 内 executor 已屏蔽信号且单线程 → 免逐趟 sigprocmask
    if (blk) gc_block_stop(&old);
    pthread_mutex_lock(&g_slab_mu);
    // ISSUE28-B1 快路径：先查最近释放过的 slab hint（成批释放同 slab 命中即免二分反查）。
    // 仅在本锁内读写 hint；slab_reclaim_empty 归还空 slab 时同锁清零 → 无悬垂 hint。
    Slab* s = NULL;
    for (int hi = 0; hi < XFREE_HINT_N; hi++) {
        Slab* h = g_xfree_hint[hi];
        if (!h) continue;
        size_t hh = (sizeof(Slab) + 7) & ~(size_t)7;
        uintptr_t hend = ((uintptr_t)h + hh + h->slot_count * h->class_size + PX_PAGE - 1) & ~(uintptr_t)(PX_PAGE - 1);
        if ((uintptr_t)p >= (uintptr_t)h && (uintptr_t)p < hend) { s = h; break; }
    }
    if (!s) s = slab_find_locked(p);
    if (s) {
        g_xfree_hint[g_xfree_hint_head] = s;
        g_xfree_hint_head = (g_xfree_hint_head + 1) % XFREE_HINT_N;
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
            if (blk) gc_unblock_stop(&old);
            return;
        }
        s->in_use[idx] = 0;
        *(void**)p = s->free_head;
        s->free_head = p;
        s->free_count++;
        pthread_mutex_unlock(&g_slab_mu);
        if (blk) gc_unblock_stop(&old);
        return;
    }
    pthread_mutex_unlock(&g_slab_mu);
    if (blk) gc_unblock_stop(&old);
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

// ==================== ISSUE28-B2（qg-issue 28）：slab 空页归还 OS ====================
// 现象：高频短请求场景堆"只涨不落"（40MB→380MB→1.1GB 不回吐）——对象 sweep 释放后
// 槽位回空闲链表复用，但整块 slab 的 mmap 映射从不归还 OS（无 munmap/madvise 路径）。
// 本函数在 GC sweep 结束后调用：摘除并 munmap **完全空闲**（free_count==slot_count）
// 的非头 slab（每 class 保留当前头 slab 作分配缓冲，防"刚释放又立即重新 mmap"抖动），
// 反查数组 g_slab_ranges 同步移除。调用前提：stop-the-world / 单线程（调用方持
// g_gc_mu 且其他线程已暂停/无并发分配）；本函数内部取 g_slab_mu 防残余并发。
static void slab_reclaim_empty(void) {
    pthread_mutex_lock(&g_slab_mu);
    // 以 g_slab_ranges（全量登记）为准遍历：任何完全空闲 slab（含游离/链上遗漏）
    // 一律摘链 + 移除登记 + munmap；每 class 保留当前头 slab 作分配缓冲防抖动。
    size_t w = 0;
    for (size_t i = 0; i < g_slab_range_count; i++) {
        Slab* s = g_slab_ranges[i];
        if (s->free_count != s->slot_count) { g_slab_ranges[w++] = s; continue; }  // 非空保留
        int ci = -1;
        for (int k = 0; k < SLAB_CLASS_COUNT; k++) if (slab_classes[k] == s->class_size) { ci = k; break; }
        if (ci < 0) { g_slab_ranges[w++] = s; continue; }   // 防御：无法归类则不回收
        if (s == g_slab_heads[ci]) { g_slab_ranges[w++] = s; continue; }  // 头 slab 缓冲保留
        // 从 class 链摘除
        Slab** pp = &g_slab_heads[ci];
        while (*pp) { if (*pp == s) { *pp = s->next; break; } pp = &(*pp)->next; }
        // 先释放 in_use 位图（独立 mmap），再 munmap slab 本体
        slab_raw_free(s->in_use);
        size_t hdr = (sizeof(Slab) + 7) & ~(size_t)7;
        size_t bytes = (hdr + s->slot_count * s->class_size + PX_PAGE - 1) & ~(size_t)(PX_PAGE - 1);
        munmap(s, bytes);
    }
    g_slab_range_count = w;
    memset(g_xfree_hint, 0, sizeof(g_xfree_hint));   // 归还 slab 后清 hint（同锁，防悬垂）
    g_xfree_hint_head = 0;
    pthread_mutex_unlock(&g_slab_mu);
}

// ==================== 全局表（定义前移：GC 标记根集合使用） ====================

#define GLOBAL_CAP 4096
static char* g_keys[GLOBAL_CAP];
static LXValue g_vals[GLOBAL_CAP];
static int g_len = 0;
// M55/P0 修复（GitHub issue#2）：全局符号表锁。px_serve 等并发场景多线程
// 同时读写 g_keys/g_vals/g_len，且与 GC 根扫描（g_vals 遍历，见 px_gc_collect）
// 不互斥 → g_len 非原子++、同槽覆盖、GC 扫到半写对象 → SEGV/内存损坏/优雅关闭
// core。原为互斥锁（全访问串行化 = GIL 效应，qg-issue 28 实测 500 并发 p50=5.2s）；
// ISSUE28-B3（qg-issue 28）改为**读写锁**：读路径（px_get_global/px_global_native/
// struct 方法查找）读锁并发，写路径（px_set_global/GC 根扫描）写锁独占——并发 handler
// 读全局表不再互相串行。持锁临界区一律先 gc_block_stop（协议同 g_gc_mu）：持锁线程
// 不被 GC 暂停 → GC stop-the-world 取写锁时不会被"已暂停的持锁线程"卡死（无死锁）；
// 读锁/写锁互斥语义与原子性等价旧互斥锁（同刻仅一写者；读读并发只读不改，安全）；
// 锁序固定 g_gc_mu → g_globals_mu，无反向获取。
static pthread_rwlock_t g_globals_mu = PTHREAD_RWLOCK_INITIALIZER;

// M105-S2：全局表名解析 O(1) 哈希索引。
// 依据（M105-S1 量化，见 docs/M105_PLAN.md）：真实负载 compiler_vm bc yaml.px 中
//   95.0% 的 strcmp 调用来自本表线性扫描，平均每次 GETG 探测 331.6 项（g_len≈537，
//   即平均扫过全表 62%）——px_get_global 均值 2.08µs，而 rdlock 仅 12ns。
// 设计与约束：
//   - 与 g_keys/g_vals 严格同步：唯一新增槽位点 = px_set_global 的 g_len++ 处落位；
//   - 容量 = 2×GLOBAL_CAP（负载因子 ≤0.5），开放寻址线性探测，按 64 位名哈希比对，
//     仅哈希命中时做一次 strcmp 兜底（保正确性下限）；
//   - **锁语义不变**：读锁下只读索引、写锁下只写索引（与 g_keys/g_vals 同锁保护）；
//   - **GC 根面不变**：根扫描仍线性遍历 g_vals，索引不参与 GC。
#define GHASH_CAP (GLOBAL_CAP * 2)
static uint32_t g_hidx[GHASH_CAP];   // 槽位号 + 1；0 表示空
static uint64_t g_hval[GHASH_CAP];   // 对应探测位的 64 位名哈希
static uint64_t g_name_hash(const char* s) {            // FNV-1a 64 位
    uint64_t h = 1469598103934665603ULL;
    while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
    return h;
}
// 查找：返回 g_keys/g_vals 槽位号；-1 = 未定义。调用方须持 g_globals_mu（读或写）。
static int g_hash_find(const char* name, uint64_t hv) {
    uint32_t h = (uint32_t)hv & (GHASH_CAP - 1);
    for (;;) {
        uint32_t e = g_hidx[h];
        if (e == 0) return -1;                          // 空位 = 探测链终止
        if (g_hval[h] == hv && strcmp(g_keys[e - 1], name) == 0) return (int)(e - 1);
        h = (h + 1) & (GHASH_CAP - 1);
    }
}
// 落位：仅在 px_set_global 新增槽位时调用（持写锁），此后键指针恒稳定（永不回收）。
static void g_hash_put(uint64_t hv, int slot) {
    uint32_t h = (uint32_t)hv & (GHASH_CAP - 1);
    while (g_hidx[h] != 0) h = (h + 1) & (GHASH_CAP - 1);
    g_hidx[h] = (uint32_t)slot + 1;
    g_hval[h] = hv;
}

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
// ISSUE28-B1（qg-issue 28）：GC 延迟到安全点（服务模式）。多线程服务场景把"对象越阈值
// 立即内联全量 STW"从请求热路径挪到**请求间安全点**（worker 空闲/池循环顶）执行，避免
// 300-500ms 周期尖刺打中在途请求（m88b 实测单发 p95=331ms/max=520ms）。g_gc_pending
// 置位后在安全点 px_gc_poll() 回收；单线程 CLI/解释模式无安全点，保持原内联（零回归）。
#define GC_HARD_CAP_FACTOR 4        // 延迟硬上限：对象数 ≥ 阈值×4 仍强制内联（防失控，保内存有界）
static volatile int g_gc_pending = 0;
static int g_gc_force_inline = 0;   // 调试/对拍：PX_GC_INLINE=1 强制请求热路径内联（还原 B1 前行为，验证 A/B）
static int g_gc_env_inited = 0;
static int g_gc_debug = 0;
static int g_active_threads = 0;   // spawn 活跃线程数（>0 时进入并发 GC 路径）

// M11 并发 GC：线程注册表 + 暂停协议
// M88-S1（qg-issue 27）：GC 线程槽动态化。原固定 64 槽（MAX_SPAWN_THREADS）是服务端
// 高并发崩溃根因之一（http_serve/sse_serve 每连接 spawn → 64 并发槽满 → px_error → exit）。
// 现改为按槽上限（PX_MAX_THREADS，默认 1024，可配 [64,4096]）在 gc_init_env 一次性分配动态表：
//   g_threads 指针 + g_thread_cap 容量（= 配置上限，一次到位，无 realloc 指针移动）。
// 并发安全：首次分配在 gc_init_env（各并发入口均先调用，持 g_gc_mu 或单线程阶段）；
//   此后 g_threads/g_thread_cap 只读恒定 → 信号处理器 / GC 遍历读到的始终是同一稳定表，
//   无 realloc/use-after-free 竞态面（比按需 ×2 扩容更稳，代价是默认 1024 槽 ≈1MB 常驻）。
#define SIG_GC_STOP (SIGRTMIN + 2)   // 实时信号：暂停线程（可排队，不与用户信号冲突）
typedef struct {
    pthread_t tid;
    int in_use;          // 槽位占用
    int paused;          // 该线程已暂停（在信号处理器中等待恢复）
    int is_main;         // 主线程槽位（退出时不注销）
    int epoch;           // 暂停所属 GC 轮次（用于区分"本轮真暂停"与"堆积信号短暂暂停"）
    ucontext_t uc;       // 暂停时保存的上下文（寄存器）
    LXObject* tmp_root;  // 暂停时该线程的暂存根（__thread 跨线程不可读，由处理器保存）
    void* vm_state;      // S3-D-1：暂停时该线程的 VM 状态指针（PxVmState*，TLS 跨线程
                         //   不可读，由运行在目标线程上的暂停处理器保存；executor 依此
                         //   遍历其堆上帧槽做精确根标记）
    LXValue* roots;      // M92：暂停时该线程的 TLS 登记根栈指针/长度（precise 模式根面；
    int      root_n;     //   __thread 跨线程不可读，由处理器保存；executor 依此精确标记）
} GCThreadInfo;
#define MAX_SPAWN_THREADS 64   // 历史宏：默认初始容量（保留供旧引用/文档对照；实际容量读 g_thread_cap）
#define PX_DEFAULT_THREAD_CAP 64     // 默认初始容量（= 历史 MAX_SPAWN_THREADS 语义）
#define PX_DEFAULT_THREAD_MAX 1024   // 默认槽上限（env PX_MAX_THREADS 未设时）
#define PX_HARD_THREAD_LIMIT 4096    // 槽硬上限（内存 / GC stop-the-world 停顿权衡；spec 环境变量表明示）
static GCThreadInfo* g_threads = NULL;              // 动态线程表（gc_init_env 分配，此后指针恒定）
static int g_thread_cap = 0;                        // 表容量（= 配置上限，gc_init_env 置位后不变）
static int g_thread_max = PX_DEFAULT_THREAD_MAX;    // 槽上限（env PX_MAX_THREADS 夹取 [64,4096]）
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

// ---- M92 精确 GC：precise/conservative 双模式 + native 桥 TLS 登记根栈 ----
// precise（VM 轨产物）：退役整栈保守扫描（gc_scan_stack/registers/thread_stack 跳过），
//   根 = 全局槽 + VM 帧槽（S3-D-1 跨线程已有）+ TLS 登记根栈 + 暂存根。C 栈上（native
//   桥 bi_*/px_* helper）跨 GC 点的局部 LXValue 引用须经 PX_KEEP 登记，否则误回收。
// conservative（默认）：保持旧行为（逃生舱 fn_* C 局部 + 所有桥局部靠保守扫栈兜底）。
static int g_gc_precise = 0;   // M92：1=precise；0=conservative（默认，零行为变化）
static __thread LXValue* g_px_roots = NULL;       // 登记根栈（本线程）
static __thread int g_px_roots_n = 0;
static __thread int g_px_roots_cap = 0;
static __thread int* g_px_root_marks = NULL;      // 作用域帧标记栈（px_root_push/pop）
static __thread int g_px_root_marks_n = 0;
static __thread int g_px_root_marks_cap = 0;

// ---- S3-D-1：VM 跨线程帧根弱符号接口（vm.c 提供强定义；无 VM 链接时空转零影响）----
// 暂停处理器（运行在目标线程上）经 px_vm_cur_state 读该线程 TLS VM 状态；
// GC executor（另一线程）经 px_vm_gc_mark_state 遍历已暂停线程的堆上帧槽做
// 精确根标记。帧槽数组在堆上，保守 C 栈扫描不可见 → 不标记则活跃对象被误回收。
extern void* px_vm_cur_state(void) __attribute__((weak));
extern void  px_vm_gc_mark_state(void* vst) __attribute__((weak));

// M93-S2：帧协程内核（coro.c 提供强定义；无协程链接时空转零影响）——
//   px_coro_spawn：spawn 分派（VM 函数 → 协程入就绪队列）；px_coro_gc_mark_roots：
//   GC 标记期补标全部存活协程（就绪参数副本 + 已运行帧槽，precise/conservative 皆用）。
extern void px_coro_spawn(void* ctx, LXValue* args, int nargs) __attribute__((weak));
extern void px_coro_spawn_ex(void* ctx, LXValue* args, int nargs,
                             void (*done_cb)(void* ud, LXValue ret),
                             void* done_ud) __attribute__((weak));
extern void px_coro_gc_mark_roots(void) __attribute__((weak));
// M95-S2：http handler 协程化 pending 表 gc 标记（实现在 ConnCtx 区后；前向声明供
//   gc 标记期调用 —— 挂起连接的 req/resp 须入精确根面，漏标 = GC 误回收 UAF）
static void http_pend_gc_mark(void);
void px_pxserve_pend_gc_mark(void);   // M98-S2a（定义见 px_conn_worker 区）
// M93-S2：VM 函数指针判定（px_spawn_name 分派用；C 轨逃生舱无 vm.o → weak 空转）
extern LXValue px_vm_entry(LXValue* args, int nargs, void* ctx) __attribute__((weak));
// M93-S2：协程 worker 复用 spawn 错误隔离 / GC 暂停信号屏蔽原语（本文件强定义导出）
int  px_spawn_isolate_begin(void);
void px_spawn_isolate_end(void);
void px_gc_block_stop_sig(sigset_t* old);
void px_gc_unblock_stop_sig(const sigset_t* old);

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
    const char* inl = getenv("PX_GC_INLINE");
    if (inl && inl[0] == '1') g_gc_force_inline = 1;
    // M92：PX_GC_PRECISE=1 强制 precise 模式（debug/回归驱动；产物插桩正式生效前用）。
    // ⚠️ 仅限 VM 轨产物——C 轨逃生舱产物 + precise = fn_* C 局部失去保守扫栈根 → 误回收。
    const char* pr = getenv("PX_GC_PRECISE");
    if (pr && pr[0] == '1') g_gc_precise = 1;
    // M88-S1：PX_MAX_THREADS 可配槽上限（夹取 [64, 4096]）；线程表一次性按上限分配。
    // 此后 g_threads/g_thread_cap 恒定，无扩容/指针移动（并发安全见 §594 注释）。
    const char* mt = getenv("PX_MAX_THREADS");
    if (mt && atoi(mt) >= PX_DEFAULT_THREAD_CAP && atoi(mt) <= PX_HARD_THREAD_LIMIT)
        g_thread_max = atoi(mt);
    if (!g_threads) {
        g_threads = (GCThreadInfo*)xcalloc((size_t)g_thread_max, sizeof(GCThreadInfo));
        g_thread_cap = g_thread_max;
    }
    gc_install_handler();
    gc_ensure_main_registered();
    g_gc_env_inited = 1;
}

// ==================== ISSUE28-B1：安全点回收 + GC 耗时观测 ====================
static long long gc_mono_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// 请求间/空闲安全点回收：服务模式把 GC 延迟到此处执行（worker 处理完一请求空闲时、
// 连接池循环顶），使全量 STW 不落在在途请求处理中。持 g_gc_mu 判定+清 pending，
// 防多 worker 同时进入（thundering herd）→ 同刻仅一个触发 px_gc_collect。
void px_gc_poll(void) {
    if (!g_gc_pending) return;
    pthread_mutex_lock(&g_gc_mu);
    int run = g_gc_pending;
    g_gc_pending = 0;
    pthread_mutex_unlock(&g_gc_mu);
    if (run) px_gc_collect();
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
        case PX_RESULT:
            return true;
        default:
            return false;
    }
}

// 释放对象内部子分配 + 对象本体（sweep 阶段调用）
static void px_obj_free(LXObject* o) {
    switch (o->type) {
        case PX_STR: xfree(o->as.str.data); xfree(o->as.str.rune_offs); break;   // M106-S2：连缓存偏移表一起回收
        // M57-S2：mmap bytes（is_mmap=1）的 data 是 mmap 映射区 → munmap；普通 bytes → xfree
        case PX_BYTES:
            if (o->is_mmap) {
                if (o->as.str.data && o->as.str.len > 0)
                    munmap(o->as.str.data, (size_t)o->as.str.len);
            } else {
                xfree(o->as.str.data);
            }
            xfree(o->as.str.rune_offs);   // M106-S2：bytes 恒不建表（NULL），防御性释放
            break;
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
        case PX_RESULT: break;  // value 内嵌 LXValue（非指针），子对象由 GC 管理
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
            case PX_RESULT: {
                // M39：Result 载荷递归标记（Ok 的 T / Err 的 E）
                if (px_value_is_obj(cur->as.result.value) && cur->as.result.value.as.obj) {
                    PUSH_OBJ(cur->as.result.value.as.obj);
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
            case PX_GEN: {
                // M89-S3-D2：生成器子对象递归标记 —— gen 可持有 物化 list /
                // 惰性 seq（list/range 等迭代源）/ transform / filter 闭包。
                // 旧实现 default 分支不标 → gen 为唯一活引用时子对象被误回收
                // （惰性 gen 的 transform/filter 闭包在 gen_next 时才调用，
                //  若已被 sweep 则 use-after-free）。四值逐一检查引用类。
                LXValue* gv[4]; int gn = 0;
                gv[gn++] = &cur->as.gen.list;
                gv[gn++] = &cur->as.gen.seq;
                gv[gn++] = &cur->as.gen.transform;
                gv[gn++] = &cur->as.gen.filter;
                for (int i = 0; i < gn; i++) {
                    if (px_value_is_obj(*gv[i]) && gv[i]->as.obj) PUSH_OBJ(gv[i]->as.obj);
                }
                break;
            }
            default: break;  // STR / FUNC / NATIVE / ENUM / MUTEX / RWLOCK 无子对象
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
    // M67-S4：架构抽象（arch.h）—— 内联汇编直接读 SP，跨 glibc/musl/arch
    uintptr_t rsp = arch_read_sp();
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
    // M105-S3：软屏蔽临界区 → 延迟暂停。临界区不被打断（不保存 ucontext / 不上报 paused），
    // 交 executor 的重发循环稍后重试；临界区退出（g_gc_crit=0）后下一次重发即正常暂停。
    if (g_gc_crit) { __sync_fetch_and_add(&g_gc_deferred, 1); return; }
    pthread_t me = pthread_self();
    // M11 修复⑤：若我是当前 GC 执行者（正在跑 px_gc_collect），忽略暂停信号——
    // 否则延迟信号在本轮 GC 执行中投递，handler 自旋等 epoch，而 epoch 只有
    // 本线程自己能推进 → 死锁（依赖 5 秒兜底才恢复，每轮 GC 卡 5 秒）。
    if (g_gc_executor && pthread_equal(g_gc_executor, me)) return;
    if (!g_threads) return;  // 表尚未分配（理论不会：信号仅 GC 进行中发出，GC 前必已 init）
    GCThreadInfo* ti = NULL;
    for (int i = 0; i < g_thread_cap; i++) {
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
    // S3-D-1：保存本线程 VM 状态（若有）——线程自己的 TLS，此处读取安全。
    // executor 暂停全部线程后据此跨线程遍历帧槽（见 px_gc_collect 并发路径）。
    ti->vm_state = px_vm_cur_state ? px_vm_cur_state() : NULL;
    // M92：precise 模式登记根栈快照（线程自旋期无代码执行 → TLS 根栈不变；同 tmp_root 模式）
    ti->roots = g_px_roots;
    ti->root_n = g_px_roots_n;
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

// M105-S3：fork 后子进程继承父线程 TLS → 复位软屏蔽层栈（防「父进程恰在软屏蔽临界区
//   fork」致子进程永久处于 skip 态：不再屏蔽信号、也不上报暂停）。子进程不再持有父进程的
//   锁与线程，故仅复位 TLS 计数。
static void gc_atfork_child(void) {
    g_gcs_depth = 0;
    g_gcs_skip = 0;
    g_gc_crit = 0;
}

static void gc_install_handler(void) {
    static int installed = 0;
    if (installed) return;
    // M88-S2（qg-issue 27）：忽略 SIGPIPE——http_serve/unix/sse_serve 等服务端对已断开
    // 连接 send()（http_conn_worker 内 5 处裸 send）会触发 SIGPIPE，默认终止整个进程
    // （无 core、无日志，表现即"高并发压测中服务进程悄然消失"）。忽略后 send 返回 EPIPE，
    // 由调用方按"连接已断"清理返回，服务进程永不因客户端断开而亡。
    signal(SIGPIPE, SIG_IGN);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = gc_stop_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;  // SA_RESTART：被信号打断的系统调用自动重启
    sigemptyset(&sa.sa_mask);
#ifndef _WIN32
    pthread_atfork(NULL, NULL, gc_atfork_child);   // M105-S3：fork 子进程复位软屏蔽层栈
#endif
    sigaction(SIG_GC_STOP, &sa, NULL);
    installed = 1;
}

// 结构修改关键区「软屏蔽」（M105-S3，语义与安全性论证见文件头 g_gcs_* 注释）：
//   真屏蔽分支保留（并发模式**零行为变化**）；无并发 GC 时免 2 次 rt_sigprocmask
//   （实测 343~372ns/对，M105-S1）。仅涉及 SIG_GC_STOP，不影响其他信号；
//   chan.buf 单 word 原子写无需屏蔽。配对纪律同既有 sigprocmask 用法：LIFO 成对。
static void gc_block_stop(sigset_t* old) {
    int d = ++g_gcs_depth;
    if (d > 16) {   // 防御：block/unblock 疑似不配对（只告警一次，不改变行为）
        static int warned = 0;
        if (!warned) { warned = 1; fprintf(stderr, "lx: gc_block_stop 嵌套 %d 层（block/unblock 不配对？）\n", d); }
    }
    if (d > 1) return;                        // 内层：外层已建立排他（信号已屏蔽 或 g_gc_crit 已置）
    if (g_active_threads == 0) {              // 无并发 GC ⇒ 无人发 SIG_GC_STOP（见文件头论证）
        g_gcs_skip = 1;
        g_gc_crit = 1;
        __atomic_signal_fence(__ATOMIC_SEQ_CST);   // 临界区写入不得被提到标志之前
        return;
    }
    g_gcs_skip = 0;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIG_GC_STOP);
    pthread_sigmask(SIG_BLOCK, &set, old);
}
static void gc_unblock_stop(const sigset_t* old) {
    if (g_gcs_depth <= 0) {                   // 防御：不配对（不应发生）→ 复位，绝不越界
        g_gcs_depth = 0; g_gcs_skip = 0; g_gc_crit = 0;
        return;
    }
    if (--g_gcs_depth > 0) return;            // 内层：外层退出时才解除
    if (g_gcs_skip) {
        __atomic_signal_fence(__ATOMIC_SEQ_CST);   // 标志清零不得被提到临界区写入之前
        g_gc_crit = 0;
        g_gcs_skip = 0;
        return;
    }
    pthread_sigmask(SIG_SETMASK, old, NULL);
}

// 线程槽位注册/注销（调用方须持 g_gc_mu；信号处理器只读不写注册表）
static GCThreadInfo* gc_find_thread(pthread_t tid) {
    for (int i = 0; i < g_thread_cap; i++) {
        if (g_threads[i].in_use && pthread_equal(g_threads[i].tid, tid)) return &g_threads[i];
    }
    return NULL;
}

static int gc_register_thread(pthread_t tid, int is_main) {
    if (gc_find_thread(tid)) return 0;  // 已注册
    for (int i = 0; i < g_thread_cap; i++) {
        if (!g_threads[i].in_use) {
            g_threads[i].tid = tid;
            g_threads[i].in_use = 1;
            g_threads[i].paused = 0;
            g_threads[i].is_main = is_main;
            g_threads[i].epoch = 0;
            memset(&g_threads[i].uc, 0, sizeof(g_threads[i].uc));
            g_threads[i].tmp_root = NULL;
            g_threads[i].vm_state = NULL;
            return 0;
        }
    }
    return -1;  // 槽位满
}

static void gc_unregister_thread(pthread_t tid) {
    for (int i = 0; i < g_thread_cap; i++) {
        if (g_threads[i].in_use && !g_threads[i].is_main && pthread_equal(g_threads[i].tid, tid)) {
            g_threads[i].in_use = 0;
            g_threads[i].paused = 0;
            g_threads[i].epoch = 0;
            g_threads[i].vm_state = NULL;
            return;
        }
    }
}

// 确保主线程已注册（首次 GC 初始化时调用；此时通常仍为主线程）
static void gc_ensure_main_registered(void) {
    if (gc_find_thread(pthread_self())) return;
    gc_register_thread(pthread_self(), 1);
}

// M67-S4：架构无关寄存器 word 标记回调（对象地址识别 + GC 标记；mark_cb 接口）
static void gc_mark_word_cb(void* ctx, uintptr_t w) {
    GCHash* set = (GCHash*)ctx;
    if ((w & 7) == 0 && gc_hash_has(set, w)) gc_mark_obj(set, (LXObject*)w);
}

// 扫描单个暂停线程的通用寄存器（M67-S4：arch.h 按架构实现，见 runtime/arch_*.h）
static void gc_scan_registers(GCHash* set, ucontext_t* uc) {
    arch_scan_registers(uc, gc_mark_word_cb, set);
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
    uintptr_t rsp = arch_uc_sp(uc);   // M67-S4：arch.h 按架构取暂停线程 SP
    uintptr_t start = rsp & ~(uintptr_t)7;
    uintptr_t end = (uintptr_t)stackaddr + stacksize;
    for (uintptr_t p = start; p + sizeof(uintptr_t) <= end; p += sizeof(uintptr_t)) {
        uintptr_t w = *(uintptr_t*)p;
        if ((w & 7) == 0 && gc_hash_has(set, w)) gc_mark_obj(set, (LXObject*)w);
    }
}

// ============ S3-D 止血切片：外部 GC 根（VM 帧槽） ============
// VM 帧槽数组在堆上（PxFrame.slots），保守 GC 只扫 C 栈/全局/暂存根，
// 帧槽引用的活跃对象不可见 → use-after-free（GC 误回收）或堆只增（漏回收）。
// 单线程 GC 标记期 g_gc_cur_set 指向当前集合；vm.c 的 px_vm_gc_mark 遍历
// 当前线程 VM 活跃帧槽调 px_gc_mark_slots 补标。无 VM 链接时弱符号空转零影响。
static GCHash* g_gc_cur_set = NULL;

void px_gc_mark_slots(LXValue* base, int n) {
    if (!base || n <= 0 || !g_gc_cur_set) return;
    for (int i = 0; i < n; i++) {
        if (px_value_is_obj(base[i]) && base[i].as.obj)
            gc_mark_obj(g_gc_cur_set, base[i].as.obj);
    }
}

// VM 活跃帧槽根标记（vm.c 实现；无 VM 链接时弱符号为空，零影响）
extern void px_vm_gc_mark(void) __attribute__((weak));

// 主回收入口：mark + sweep（M11：spawn 活跃时 stop-the-world）
void px_gc_collect(void) {
    // M11 修复④：GC 执行期间屏蔽自己的 SIG_GC_STOP——防止上一轮"延迟信号"
    // 在本轮 GC 执行中投递（handler 会自旋等 epoch，而 epoch 只有本线程能推进
    // → 卡死/5 秒空转）。先拿锁再屏蔽：等锁期间不屏蔽（可被其他 GC 正常暂停），
    // 持锁后屏蔽（防自打断）。出口统一 gc_unblock_stop。
    long long t0 = gc_mono_ms();   // ISSUE28-B1：GC 耗时观测
    sigset_t gc_old;
    pthread_mutex_lock(&g_gc_mu);
    gc_block_stop(&gc_old);
    g_gc_executor = pthread_self();   // 标记我是 GC 执行者（handler 自检防自打断）
    g_gc_pending = 0;                 // ISSUE28-B1：任一回收路径（内联/gc()/安全点）清除延迟标记
    if (!g_gc_env_inited) gc_init_env();
    if (g_obj_count == 0) {
        pthread_mutex_unlock(&g_gc_mu);
        g_gc_executor = 0;
        gc_unblock_stop(&gc_old);
        return;
    }

    if (g_active_threads > 0) {
        // ===== M11 并发路径：stop-the-world =====
        // M55/P0（issue#2）：暂停线程前先持 g_globals_mu。若在暂停线程后才取锁，
        // 会等一个"临界区内被信号暂停、持锁未释放"的线程 → 死锁/GC 空转。先取锁
        // （此刻线程均正常执行，持锁者会跑完释放，无暂停干扰）→ 再 stop-the-world
        // → 根1 扫描全局表（独占）→ 扫完即释放，尽量缩短持锁时长。
        pthread_rwlock_wrlock(&g_globals_mu);
        pthread_t me = pthread_self();
        if (g_gc_debug) {
            char dbg[512]; int dn = 0;
            dn += snprintf(dbg+dn, sizeof(dbg)-dn, "[gc] me=%lx active=%d deferred=%ld\n", (unsigned long)me, g_active_threads, g_gc_deferred);
            for (int i = 0; i < g_thread_cap; i++)
                if (g_threads[i].in_use)
                    dn += snprintf(dbg+dn, sizeof(dbg)-dn, "  [%d] tid=%lx main=%d paused=%d\n", i, (unsigned long)g_threads[i].tid, g_threads[i].is_main, g_threads[i].paused);
            (void)write(2, dbg, (size_t)dn);
        }
        // 本轮 GC 开始（handler 捕获 my_epoch 后自旋等待 epoch 变化）
        g_gc_epoch++;
        g_gc_stop_in_progress = 1;   // 标记进行中（handler 据此区分过期堆积信号）
        __sync_synchronize();
        // 1) 向所有已注册、非自身、已创建完成的线程发送暂停信号（只发一次）
        for (int i = 0; i < g_thread_cap; i++) {
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
            for (int i = 0; i < g_thread_cap; i++) {
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
                pthread_rwlock_unlock(&g_globals_mu);   // M55：释放暂停前持有的全局表锁
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
        // 根1：全局表（M55：GC 于暂停线程前已持 g_globals_mu，此处独占扫描，
        // 防读到 px_set/get_global 半写对象；扫完立即释放，供恢复后的线程使用）
        for (int i = 0; i < g_len; i++) {
            if (px_value_is_obj(g_vals[i]) && g_vals[i].as.obj) gc_mark_obj(&set, g_vals[i].as.obj);
        }
        pthread_rwlock_unlock(&g_globals_mu);
        if (g_gc_debug) (void)write(2, "[mk] globals\n", 13);
        // 根2：本线程（GC 执行者）暂存根
        if (g_tmp_root) gc_mark_obj(&set, g_tmp_root);
        // S3-D-1：VM 帧槽精确根 —— executor 自身 VM 状态 + 各暂停线程 VM 状态。
        // 帧槽数组在堆上，保守栈扫描不可见；g_gc_cur_set 供 px_gc_mark_slots 使用。
        g_gc_cur_set = &set;
        // 根3：本线程栈 + 寄存器（conservative：setjmp 把寄存器写入栈上 jmp_buf，
        //      一并扫描——musl 无 getcontext，M57-S4 改 setjmp，同为外部调用强制 spill）。
        //      M92 precise：退役整栈保守扫描 → 跳过；补标本线程 TLS 登记根栈。
        if (!g_gc_precise) {
            jmp_buf jb;
            (void)setjmp(jb);
            gc_scan_stack(&set);
        }
        if (g_gc_debug) (void)write(2, g_gc_precise ? "[mk] self-precise-roots\n" : "[mk] self-stack\n", g_gc_precise ? 20 : 15);
        // 根3b：executor 自身 VM 活跃帧槽（单线程路径同款补标，此处并发路径）
        if (px_vm_gc_mark) px_vm_gc_mark();
        // 根3c：M92 precise —— executor 自身 TLS 登记根栈
        if (g_gc_precise && g_px_roots_n > 0) px_gc_mark_slots(g_px_roots, g_px_roots_n);
        // 根4：所有本轮暂停线程：寄存器 + 栈 + 暂存根 + VM 帧槽（跨线程）
        //      M92 precise：暂停线程跳过保守栈/寄存器扫描，改标其 TLS 登记根栈快照。
        for (int i = 0; i < g_thread_cap; i++) {
            GCThreadInfo* ti = &g_threads[i];
            if (!ti->in_use || !ti->paused || ti->epoch != g_gc_epoch || pthread_equal(ti->tid, me)) continue;
            if (g_gc_precise) {
                if (ti->roots && ti->root_n > 0) px_gc_mark_slots(ti->roots, ti->root_n);
            } else {
                gc_scan_registers(&set, &ti->uc);
                gc_scan_thread_stack(&set, ti->tid, &ti->uc);
            }
            if (ti->tmp_root) gc_mark_obj(&set, ti->tmp_root);
            if (ti->vm_state && px_vm_gc_mark_state) px_vm_gc_mark_state(ti->vm_state);
            if (g_gc_debug) { char dbg[64]; int dn = snprintf(dbg, sizeof(dbg), "[mk] scanned tid=%lx\n", (unsigned long)ti->tid); (void)write(2, dbg, (size_t)dn); }
        }
        // 根4b：M93-S2 帧协程根面 —— 全部存活协程（就绪参数副本 + 运行/阻塞中帧槽）。
        //   运行中协程 vm 已由所属 worker 的 ti->vm_state 覆盖（重复标无害）；
        //   就绪队列协程的 args 副本仅本表可达 → 必须补标（漏标 = worker 取到 UAF）。
        if (px_coro_gc_mark_roots) px_coro_gc_mark_roots();
        http_pend_gc_mark();   // M95-S2：http 挂起连接 req/resp 补标
        px_pxserve_pend_gc_mark();   // M98-S2a：px_serve 挂起连接 req/resp 补标
        g_gc_cur_set = NULL;
        // 4) sweep
        if (g_gc_debug) { char dbg[128]; int dn = snprintf(dbg, sizeof(dbg), "[mk] 暂停+标记+扫栈耗时%lldms\n", gc_mono_ms() - t0); (void)write(2, dbg, (size_t)dn); }
        if (g_gc_debug) { char dbg[96]; int dn = snprintf(dbg, sizeof(dbg), "[mk] sweep count=%d\n", g_obj_count); (void)write(2, dbg, (size_t)dn); }
        g_in_gc_sweep = 1;   // ISSUE28-B1：sweep 单线程（executor 已屏蔽信号）→ xfree 免逐趟 sigprocmask
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
        g_in_gc_sweep = 0;
        g_obj_count = w;
        g_alloc_bytes = 0;
        g_gc_freed += freed;
        g_gc_runs++;
        g_tmp_root = NULL;
        if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
        gc_debug("collect #%d(并发): 标记 %lld/%d 回收 %d 存活 %d 线程 %d 耗时%lldms", g_gc_runs, g_gc_marked, g_obj_count + freed, freed, g_obj_count, g_paused_count, gc_mono_ms() - t0);
        if (g_gc_debug) (void)write(2, "[mk] after-collect\n", 19);
        gc_hash_free(&set);
        // ISSUE28-B2：sweep 后归还完全空闲 slab 页给 OS（仍 STW，无并发分配，安全）
        slab_reclaim_empty();
        // 5) 本轮结束：epoch++ 唤醒所有暂停线程；清除进行中标志；等待其全部恢复
        g_gc_epoch++;
        g_gc_stop_in_progress = 0;
        __sync_synchronize();
        if (g_gc_debug) (void)write(2, "[mk] resume-wait\n", 17);
        int wspins = 0, wstable = 0;
        for (;;) {
            int any_paused = 0;
            __sync_synchronize();
            for (int i = 0; i < g_thread_cap; i++)
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
    pthread_rwlock_wrlock(&g_globals_mu);
    for (int i = 0; i < g_len; i++) {
        if (px_value_is_obj(g_vals[i]) && g_vals[i].as.obj) gc_mark_obj(&set, g_vals[i].as.obj);
    }
    pthread_rwlock_unlock(&g_globals_mu);
    if (g_tmp_root) gc_mark_obj(&set, g_tmp_root);
    // M92 precise：退役整栈保守扫描 → 跳过本线程栈/寄存器扫描（conservative 保持旧行为）
    if (!g_gc_precise) {
        jmp_buf jb;
        (void)setjmp(jb);
        gc_scan_stack(&set);
    }
    // S3-D 止血：补标当前线程 VM 活跃帧槽（堆上根，保守 C 栈扫不到）
    g_gc_cur_set = &set;
    if (px_vm_gc_mark) px_vm_gc_mark();
    // M92 precise：补标当前线程 TLS 登记根栈（native 桥局部显式根）
    if (g_gc_precise && g_px_roots_n > 0) px_gc_mark_slots(g_px_roots, g_px_roots_n);
    // M93-S2：帧协程根面补标（单线程 GC 路径同款；协程存在即有 worker 活跃走并发路径，
    //   此处兜底纯就绪/创建窗口）
    if (px_coro_gc_mark_roots) px_coro_gc_mark_roots();
    http_pend_gc_mark();   // M95-S2：同上（单线程 GC 路径兜底）
    px_pxserve_pend_gc_mark();   // M98-S2a：同上（单线程 GC 路径兜底）
    g_gc_cur_set = NULL;
    g_in_gc_sweep = 1;   // ISSUE28-B1：单线程 sweep 同上免逐趟 sigprocmask
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
    g_in_gc_sweep = 0;
    g_obj_count = w;
    g_alloc_bytes = 0;
    g_gc_freed += freed;
    g_gc_runs++;
    g_tmp_root = NULL;
    if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
    gc_debug("collect #%d: 标记 %lld/%d 回收 %d 存活 %d 跳过 %d 耗时%lldms", g_gc_runs, g_gc_marked, g_obj_count + freed, freed, g_obj_count, g_gc_skips, gc_mono_ms() - t0);
    gc_hash_free(&set);
    // ISSUE28-B2：sweep 后归还完全空闲 slab 页给 OS（单线程路径，无并发分配）
    slab_reclaim_empty();
    pthread_mutex_unlock(&g_gc_mu);
    g_gc_executor = 0;
    gc_unblock_stop(&gc_old);
}

// ---- M92 精确 GC：precise/conservative 模式 + native 桥根登记 API ----
// 模式切换只在程序启动早期（产物 main）调用一次，不做并发安全（GC 开始后不可切）。
void px_gc_set_precise(int precise) {
    g_gc_precise = precise ? 1 : 0;
}

// 作用域开始：保存当前登记栈深度（native 桥入口调用，与 px_root_pop 配对）。
// 登记栈为 TLS，native 桥在同一线程执行，作用域天然线程隔离。
// M92-S2c：屏蔽 SIG_GC_STOP —— 根栈 push/pop/keep 非原子，若信号落在
// push/keep 的 n++ 中途，GC handler 快照 ti->root_n 读到半态 → 刚登记的
// 局部漏根被误回收（并发 GC 偶发 UAF 根因）。与 list/dict 结构修改同模式。
void px_root_push(void) {
    sigset_t old;
    gc_block_stop(&old);
    if (g_px_root_marks_n >= g_px_root_marks_cap) {
        int nc = g_px_root_marks_cap ? g_px_root_marks_cap * 2 : 16;
        g_px_root_marks = (int*)xrealloc(g_px_root_marks, sizeof(int) * (size_t)nc);
        g_px_root_marks_cap = nc;
    }
    g_px_root_marks[g_px_root_marks_n++] = g_px_roots_n;
    gc_unblock_stop(&old);
}

// 作用域结束：弹回 px_root_push 时的深度（与 push 严格配对）。
void px_root_pop(void) {
    sigset_t old;
    gc_block_stop(&old);
    if (g_px_root_marks_n <= 0) { gc_unblock_stop(&old); return; }
    int mark = g_px_root_marks[--g_px_root_marks_n];
    g_px_roots_n = mark;
    gc_unblock_stop(&old);
}

// 登记一个局部 LXValue 引用（跨可能触发 GC 的调用前调用）。只压引用类值
// （int/bool/null 无对象无需保护）；压入的是值拷贝，后续对局部变量的赋值
// 不影响已登记条目（登记 = 快照该时刻的引用，语义正确：局部变量持有期即该值）。
void px_root_keep(const LXValue* v) {
    if (!v || !px_value_is_obj(*v)) return;
    sigset_t old;
    gc_block_stop(&old);
    if (g_px_roots_n >= g_px_roots_cap) {
        int nc = g_px_roots_cap ? g_px_roots_cap * 2 : 64;
        g_px_roots = (LXValue*)xrealloc(g_px_roots, sizeof(LXValue) * (size_t)nc);
        g_px_roots_cap = nc;
    }
    g_px_roots[g_px_roots_n++] = *v;
    gc_unblock_stop(&old);
}

// 注册对象（构造时调用）。est = 估算占用字节（触发字节阈值用，当前主用对象数阈值）。
static void gc_register(LXObject* o, long long est) {
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    o->gc_mark = 0;   // 关键：xmalloc 未清零，gc_mark 垃圾值=1 会导致 DFS 跳过该节点（子对象漏标）
    o->is_mmap = 0;   // M57-S2：同上，is_mmap 垃圾值=1 会导致 sweep 误对普通 data 走 munmap
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
    int snap_count = g_obj_count;
    int snap_thr = g_gc_threshold;
    int deferrable = (g_active_threads > 0) && !g_gc_force_inline;   // 服务/并发模式：存在请求间安全点（PX_GC_INLINE=1 对拍强制内联）
    pthread_mutex_unlock(&g_gc_mu);
    if (need) {
        // ISSUE28-B1：多线程服务模式（spawn/连接池活跃）把 GC 延迟到安全点（worker 空闲/
        // 池循环顶），避免全量 STW 落在请求热路径；对象数超过 阈值×4 硬上限仍强制内联
        // （内存有界兜底）；单线程 CLI/解释模式无安全点，保持原内联（零行为回归）。
        if (deferrable && (long long)snap_count < (long long)snap_thr * GC_HARD_CAP_FACTOR) {
            g_gc_pending = 1;
        } else {
            px_gc_collect();
        }
    }
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
    o->as.str.rune_len = -1; o->as.str.offs_cnt = 0; o->as.str.rune_offs = NULL;  // M106-S2：惰性缓存初值
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
    o->as.str.rune_len = -1; o->as.str.offs_cnt = 0; o->as.str.rune_offs = NULL;  // M106-S2：bytes 恒不建表
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

LXValue px_enum_variant(LXValue v) {
    if (v.type == PX_ENUM) return px_str(v.as.obj->as.enum_inst.variant);
    return px_null();
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
        case PX_GEN: return "generator";
        case PX_RESULT: return "result";
    }
    return "unknown";
}

// ==================== M32/M34 生成器 ====================

LXValue px_gen_from_list(LXValue list) {
    LXObject* o = xmalloc(sizeof(LXObject));
    memset(o, 0, sizeof(LXObject));
    o->type = PX_GEN;
    o->as.gen.list = list;
    o->as.gen.cursor = 0;
    o->as.gen.is_lazy = 0;
    LXValue v;
    v.type = PX_GEN;
    v.as.obj = o;
    return v;
}

// M34：惰性生成器——seq（list/range）不展开，transform/filter 闭包在 gen_next 时逐项调用。
LXValue px_gen_lazy(LXValue seq, LXValue transform, LXValue filter) {
    LXObject* o = xmalloc(sizeof(LXObject));
    memset(o, 0, sizeof(LXObject));
    o->type = PX_GEN;
    o->as.gen.list = px_list(0);   // 物化缓冲（for-in / list / len 时展开）
    o->as.gen.cursor = 0;
    o->as.gen.is_lazy = 1;
    o->as.gen.seq = seq;
    o->as.gen.transform = transform;
    o->as.gen.filter = filter;
    LXValue v;
    v.type = PX_GEN;
    v.as.obj = o;
    return v;
}

// 惰性 seq 取第 i 个元素（list 索引；C 端 range 已物化为 list）
static void px_gen_materialize(LXObject* o);

static LXValue px_lazy_seq_get(LXValue seq, int i, int* has) {
    if (seq.type == PX_LIST) {
        if (i >= 0 && i < seq.as.obj->as.list.len) {
            *has = 1;
            return seq.as.obj->as.list.items[i];
        }
    }
    // M39 修复（M34 遗留）：惰性生成器的 seq 支持嵌套生成器（gen of gen）
    // ——先物化内层生成器，再按下标取项
    if (seq.type == PX_GEN) {
        LXObject* go = seq.as.obj;
        if (go->as.gen.is_lazy) px_gen_materialize(go);
        LXValue l = go->as.gen.list;
        if (l.type == PX_LIST && i >= 0 && i < l.as.obj->as.list.len) {
            *has = 1;
            return l.as.obj->as.list.items[i];
        }
    }
    *has = 0;
    return px_null();
}

// 惰性生成器剩余项全部物化到 list（for-in / list() / len / 索引时调用，保持 M32 行为一致）
static void px_gen_materialize(LXObject* o) {
    if (!o->as.gen.is_lazy) return;
    o->as.gen.is_lazy = 0;
    for (;;) {
        int has = 0;
        LXValue x = px_lazy_seq_get(o->as.gen.seq, o->as.gen.cursor, &has);
        o->as.gen.cursor++;
        if (!has) break;
        if (o->as.gen.filter.type == PX_FUNC) {
            LXValue keep = px_call(o->as.gen.filter, &x, 1);
            if (!px_is_truthy(keep)) continue;
        }
        LXValue r = px_call(o->as.gen.transform, &x, 1);
        px_list_push(o->as.gen.list, r);
    }
}

LXValue px_gen_next(LXValue g) {
    if (g.type != PX_GEN) px_error("gen_next 需要生成器对象");
    LXObject* o = g.as.obj;
    if (o->as.gen.is_lazy) {
        for (;;) {
            int has = 0;
            LXValue x = px_lazy_seq_get(o->as.gen.seq, o->as.gen.cursor, &has);
            o->as.gen.cursor++;
            if (!has) { o->as.gen.is_lazy = 0; return px_null(); }
            if (o->as.gen.filter.type == PX_FUNC) {
                LXValue keep = px_call(o->as.gen.filter, &x, 1);
                if (!px_is_truthy(keep)) continue;
            }
            return px_call(o->as.gen.transform, &x, 1);
        }
    }
    if (o->as.gen.list.type == PX_LIST && o->as.gen.cursor < o->as.gen.list.as.obj->as.list.len) {
        LXValue r = o->as.gen.list.as.obj->as.list.items[o->as.gen.cursor];
        o->as.gen.cursor++;
        return r;
    }
    return px_null();
}

// ==================== M39 Result（spec §3.5 错误处理唯一通道） ====================

LXValue px_ok(LXValue v) {
    LXValue r;
    r.type = PX_RESULT;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_RESULT;
    o->gc_mark = 0;
    o->as.result.ok = 1;
    o->as.result.value = v;
    r.as.obj = o;
    gc_register(o, sizeof(LXObject));
    return r;
}
LXValue px_err(LXValue v) {
    LXValue r;
    r.type = PX_RESULT;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_RESULT;
    o->gc_mark = 0;
    o->as.result.ok = 0;
    o->as.result.value = v;
    r.as.obj = o;
    gc_register(o, sizeof(LXObject));
    return r;
}
bool px_is_result(LXValue v) { return v.type == PX_RESULT; }
bool px_result_ok(LXValue v) { return v.as.obj->as.result.ok; }
LXValue px_result_unwrap(LXValue v) { return v.as.obj->as.result.value; }
// Some(x) = x（Option 无运行时包装：None 即 null）
LXValue px_some(LXValue v) { return v; }

// ==================== 错误 ====================

// M72-S2（Issue 10 D1）：编译产物运行时 .px 源位置追踪（cg 每语句插 px_srcline、
// 函数入口插 px_srcfunc；px_error 打印最近位置）。__thread → spawn 各协程独立
// 追踪（S3 错误隔离按线程打印现场）。
__thread int g_px_src_line = 0;
__thread const char* g_px_src_func = NULL;
void px_srcline(int line) { g_px_src_line = line; }
void px_srcfunc(const char* name) { g_px_src_func = name; }
// M72-S3（Issue 10 D2）：spawn 协程错误捕获边界（TLS per-thread）——spawn_thread 在
// 调 fn 前 setjmp；协程内 px_error 打印现场后 longjmp 回捕获点 → 该协程安全退出
// （走 GC 注销路径），宿主进程继续。主线程不设捕获 → 顶层错误保持 exit(1)。
static __thread jmp_buf g_err_jmp;
static __thread int g_err_jmp_set = 0;
static __thread char g_err_last_msg[512];   // M96-S2：本线程最后 px_error 文本（px_err_last）

void px_error(const char* fmt, ...) {
    // 先刷新 stdout 缓冲：print 输出在管道/重定向下是全缓冲，exit 前不刷会丢
    fflush(stdout);
    va_list ap;
    va_start(ap, fmt);
    // M96-S2：记录本线程最后一次错误文本（offload 外包线程捕获后回传协程重抛用）。
    //   va_copy 独立消费（vfprintf 随后仍需原 ap）。
    { va_list ap2; va_copy(ap2, ap); vsnprintf(g_err_last_msg, sizeof(g_err_last_msg), fmt, ap2); va_end(ap2); }
    // M72-S2（Issue 10 D1）：运行时错误带 .px 源位置（编译产物路径；pxi 解释器
    // 走 i_err line:col 不受影响）。无追踪位置（native 初始化期）→ 原样前缀。
    if (g_px_src_line > 0) {
        if (g_px_src_func && g_px_src_func[0])
            fprintf(stderr, "运行时错误 [%s 行%d]: ", g_px_src_func, g_px_src_line);
        else
            fprintf(stderr, "运行时错误 [行%d]: ", g_px_src_line);
    } else {
        fprintf(stderr, "运行时错误: ");
    }
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    fflush(stderr);
    // M72-S3（Issue 10 D2）：spawn 协程内错误 → longjmp 隔离（宿主继续）；主线程
    // /无捕获点 → exit(1)（带现场）。隔离由 spawn_thread setjmp 提供。
    if (g_err_jmp_set) {
        longjmp(g_err_jmp, 1);
    }
    exit(1);
}

// ==================== 字符串工具 ====================

// M83-S1（Issue 16 GAP-STR-1-B1）：带字节长度边界的 UTF-8 字符计数——str 内嵌 NUL 时
// len() 尊重 str.len（完整字节边界）而非 C strlen（在首个 NUL 截断）。边界 n 为字节数。
int px_unicode_len_n(const char* s, int n) {
    int c = 0;
    for (int i = 0; i < n; i++) {
        if (((unsigned char)s[i] & 0xC0) != 0x80) c++;  // 非连续字节 = 新字符
    }
    return c;
}

int px_unicode_len(const char* s) {
    return px_unicode_len_n(s, (int)strlen(s));
}

// ==================== M106-S2（Issue 33-A）：str 惰性 rune 缓存 ====================
// 病灶：px_index / px_len 的 PX_STR 分支每次调用都 px_unicode_len_n 全扫一遍（O(n)），
//   且 px_index 还要从 byte0 线性走到第 i 个 rune（O(i)）⇒ 逐字符扫描 .px 代码天然 O(n²)
//   （64KB 实测 5.4s；ws-approve /check 因 json_valid 逐字符两趟 → 11.6s → 越过 fail-closed）。
// 解法：str 对象上挂两个惰性缓存（见 runtime.h 的 str 子结构注释）——
//   rune_len（一次 O(n)，之后 O(1)）+ rune_offs（一次 O(n) 建表，之后 s[i] 取起始偏移 O(1)）。
// 语义红线（逐条对齐改动前）：
//   1. rune_len 值 = px_unicode_len_n 的原结果（**不是**建表步数——畸形 UTF-8 下二者不等）；
//   2. 偏移表按 px_index 原线性走查**完全相同的步进规则**建（前导字节判长），
//      故表中 offs[i] 与旧代码走 i 步后的 p 逐字节相同；
//   3. 索引 i ≥ 表步数时（仅畸形 UTF-8 可能）**回落原线性走查**，连越界读行为都保持原样；
//   4. 单字符结果仍按原 c0 判长（px_utf8_clen(c0)）构造，不改 clen 语义。
// 内存/性能取舍：仅字节长度 ≥ PX_STR_OFFS_MIN 的串才建表（表 = 4B×步数），小串线性走更划算。
#ifndef PX_STR_OFFS_MIN
#define PX_STR_OFFS_MIN 1024          // 可 -DPX_STR_OFFS_MIN=… 覆盖（用于等价性/边界试验）
#endif
// 上界护栏（M106-S3）：偏移表内存 ≈ sizeof(int)×(字节数+1) ≈ 4× 串长。KB 级串无碍；
//   但若对数百 MB 的巨串反复取 s[i]，4× 放大可能压垮内存 ⇒ 超过 PX_STR_OFFS_MAX 的串
//   一律不建表，回落 px_index 原线性走查（与 M105 及更早逐字节同行为），仅保留
//   rune_len 惰性计数（O(1) 空间）。阈值 16 MiB 串 ⇒ 表上界 ≈ 64 MiB。
#ifndef PX_STR_OFFS_MAX
#define PX_STR_OFFS_MAX (16 * 1024 * 1024)   // 可 -DPX_STR_OFFS_MAX=… 覆盖
#endif

// 前导字节 → 该字符字节数（与 px_index 原实现的判定式逐字相同）
static inline int px_utf8_clen(unsigned char cc) {
    if ((cc & 0x80) == 0) return 1;
    if ((cc & 0xE0) == 0xC0) return 2;
    if ((cc & 0xF0) == 0xE0) return 3;
    if ((cc & 0xF8) == 0xF0) return 4;
    return 1;
}

// 惰性 rune 计数（结果与 px_unicode_len_n 完全一致；str 不可变 ⇒ 无失效逻辑）
static inline int px_str_rune_len(LXObject* o) {
    int c = __atomic_load_n(&o->as.str.rune_len, __ATOMIC_RELAXED);
    if (c < 0) {
        c = px_unicode_len_n(o->as.str.data, o->as.str.len);
        __atomic_store_n(&o->as.str.rune_len, c, __ATOMIC_RELAXED);
    }
    return c;
}

// 惰性 rune→byte 起始偏移表；返回 NULL 表示"小串/空串，走原线性走查"
static int* px_str_offs_get(LXObject* o) {
    int* p = __atomic_load_n(&o->as.str.rune_offs, __ATOMIC_ACQUIRE);
    if (p) return p;
    int n = o->as.str.len;
    if (n < PX_STR_OFFS_MIN) return NULL;          // 小串：不建表（避免小串也付分配代价）
    if (n > PX_STR_OFFS_MAX) return NULL;          // M106-S3 巨串：不建表（护栏，防 4× 内存放大）→ 回落线性走查
    const unsigned char* s = (const unsigned char*)o->as.str.data;
    if (!s) return NULL;
    int* offs = (int*)xmalloc(sizeof(int) * ((size_t)n + 1));   // 步数 ≤ 字节数 ⇒ n+1 足够
    int c = 0, i = 0;
    while (i < n) { offs[c++] = i; i += px_utf8_clen(s[i]); }   // 与 px_index 原走查同步进规则
    int* old = __atomic_load_n(&o->as.str.rune_offs, __ATOMIC_ACQUIRE);
    if (old) { xfree(offs); return old; }          // 竞态：他线程已建 → 用它的，丢弃本次
    __atomic_store_n(&o->as.str.offs_cnt, c, __ATOMIC_RELAXED);
    __atomic_store_n(&o->as.str.rune_offs, offs, __ATOMIC_RELEASE);   // 先放数据后发布指针
    return offs;
}

// 简单数字转字符串（int/float）
static char num_buf[64];
static const char* fmt_num(LXValue v) {
    if (v.type == PX_INT) {
        snprintf(num_buf, sizeof(num_buf), "%lld", (long long)v.as.i);
    } else {
        double f = v.as.f;
        // M63-L9：float→str 最短 roundtrip 全精度（替代原 %g 6 位截断，MINI_SUBSET §十三.8）
        // 格式规则 = 语言既有 %g 定点舒适区：定点 iff 十进制指数 x ∈ [-4,15)（即
        // 1e-4 <= |f| < 1e15），否则科学计数——保持语言习惯：100000.0→"100000.0"、
        // 250.0→"250.0"、1e15→"1e+15"（%g6 时代同界；M63 把精度提到 roundtrip 全精度：
        // 0.1+0.2→"0.30000000000000004"（旧 "0.3"）、1/3→"0.3333333333333333"、
        // 123456789.123→"123456789.123"（旧 "1.23457e+08"））。
        // 精度：roundtrip 所需最小位数——%.*f（定点）/%.*e（科学）逐位递增 + strtod
        // 回读校验，首个相等即最短（位数单调，IEEE754 17 位内必达）。区别于早期实现
        // 全扫 %.g 取字符最短（会把 100000.0 显成科学 "1e+05"、250.0 显 "2.5e+02"）。
        if (isnan(f) || isinf(f)) {
            snprintf(num_buf, sizeof(num_buf), "%g", f);          /* nan / inf / -inf */
        } else if (f == 0.0) {
            strcpy(num_buf, signbit(f) ? "-0" : "0");             /* ±0，.0 补丁同下 */
        } else {
            double a2 = (f < 0.0) ? -f : f;
            int x = (int)floor(log10(a2));
            double p10 = pow(10.0, (double)x);
            if (a2 < p10) { x--; p10 /= 10.0; }
            else if (a2 >= p10 * 10.0) { x++; p10 *= 10.0; }
            int dec;
            if (x >= -4 && x < 15) {                              /* 定点舒适区 */
                for (dec = 0; dec <= 17; dec++) {
                    snprintf(num_buf, sizeof(num_buf), "%.*f", dec, f);
                    if (strtod(num_buf, NULL) == f) break;
                }
            } else {                                              /* 科学计数 */
                for (dec = 0; dec <= 16; dec++) {
                    snprintf(num_buf, sizeof(num_buf), "%.*e", dec, f);
                    if (strtod(num_buf, NULL) == f) break;
                }
            }
        }
        // M62-L1：对齐解释器 i_fmt_float（selfhost/ival.px）：整值且有限且 |f| < 1e15
        // 且 %g 输出无 ./e/E/inf/nan → 补 ".0"（编译模式原 %g 打印 3.0 → "3"，解释器 "3.0"，
        // 双模式不对称；补 .0 后与解释器/golden(Rust fmt_float) 一致）
        double af = (f < 0.0) ? -f : f;
        if (af < 1e15 && f == (double)(int64_t)f) {
            if (!strpbrk(num_buf, ".eE") && !strchr(num_buf, 'i') && !strchr(num_buf, 'n')) {
                size_t ln = strlen(num_buf);
                if (ln + 2 < sizeof(num_buf)) {
                    num_buf[ln] = '.';
                    num_buf[ln + 1] = '0';
                    num_buf[ln + 2] = '\0';
                }
            }
        }
    }
    return num_buf;
}

// ---- 字符串构建（M-B5：值格式化用；原位于 regex 替换处，提前复用） ----
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
static char* rsb_done(RStrBuf* b) {
    char* out = xmalloc(b->len + 1);
    if (b->len > 0) memcpy(out, b->data, b->len);
    out[b->len] = 0;
    xfree(b->data);
    return out;
}
static int px_cmp_cstr(const void* a, const void* b) {
    return strcmp(*(const char* const*)a, *(const char* const*)b);
}

// ==================== 输出 ====================

// ---- M-B5：值格式化（对齐 Rust value.rs fmt_value）----
// 递归渲染 list/dict/tuple/enum/struct/result/gen；原子类型直接格式化
// 返回 malloc 字符串（调用方 free）
// 注意：dict 按完整 "k: v" 字符串排序（对齐 Rust parts.sort()，而非按键排序）
static char* px_fmt_value(LXValue v) {
    RStrBuf b = {0};
    switch (v.type) {
        case PX_NULL: rsb_append(&b, "null", 4); break;
        case PX_BOOL: rsb_append(&b, v.as.b ? "true" : "false", v.as.b ? 4 : 5); break;
        case PX_INT: { char t[32]; int n = snprintf(t, sizeof(t), "%lld", (long long)v.as.i); rsb_append(&b, t, n); break; }
        case PX_FLOAT: { const char* t = fmt_num(v); rsb_append(&b, t, (int)strlen(t)); break; }
        case PX_STR: rsb_append(&b, v.as.obj->as.str.data, v.as.obj->as.str.len); break;
        case PX_BYTES: { char t[64]; int n = snprintf(t, sizeof(t), "<bytes %d>", v.as.obj->as.str.len); rsb_append(&b, t, n); break; }
        case PX_LIST: {
            LXObject* o = v.as.obj;
            rsb_append(&b, "[", 1);
            for (int i = 0; i < o->as.list.len; i++) {
                if (i) rsb_append(&b, ", ", 2);
                char* s = px_fmt_value(o->as.list.items[i]);
                rsb_append(&b, s, (int)strlen(s));
                xfree(s);
            }
            rsb_append(&b, "]", 1);
            break;
        }
        case PX_TUPLE: {
            LXObject* o = v.as.obj;
            rsb_append(&b, "(", 1);
            for (int i = 0; i < o->as.tuple.len; i++) {
                if (i) rsb_append(&b, ", ", 2);
                char* s = px_fmt_value(o->as.tuple.items[i]);
                rsb_append(&b, s, (int)strlen(s));
                xfree(s);
            }
            rsb_append(&b, ")", 1);
            break;
        }
        case PX_DICT: {
            LXObject* o = v.as.obj;
            int n = o->as.dict.len;
            char** parts = n > 0 ? xmalloc(n * sizeof(char*)) : NULL;
            for (int i = 0; i < n; i++) {
                char* vs = px_fmt_value(o->as.dict.vals[i]);
                int klen = (int)strlen(o->as.dict.keys[i]);
                int vlen = (int)strlen(vs);
                parts[i] = xmalloc((size_t)klen + vlen + 4);
                memcpy(parts[i], o->as.dict.keys[i], klen);
                parts[i][klen] = ':'; parts[i][klen + 1] = ' ';
                memcpy(parts[i] + klen + 2, vs, vlen);
                parts[i][klen + vlen + 2] = 0;
                xfree(vs);
            }
            if (n > 1) qsort(parts, (size_t)n, sizeof(char*), px_cmp_cstr);
            rsb_append(&b, "{", 1);
            for (int i = 0; i < n; i++) {
                if (i) rsb_append(&b, ", ", 2);
                rsb_append(&b, parts[i], (int)strlen(parts[i]));
                xfree(parts[i]);
            }
            rsb_append(&b, "}", 1);
            xfree(parts);
            break;
        }
        case PX_FUNC: { char t[256]; int n = snprintf(t, sizeof(t), "<fn %s>", v.as.obj->as.func.name); rsb_append(&b, t, n); break; }
        case PX_NATIVE: { char t[256]; int n = snprintf(t, sizeof(t), "<builtin %s>", v.as.obj->as.native.name); rsb_append(&b, t, n); break; }
        case PX_STRUCT: { char t[256]; int n = snprintf(t, sizeof(t), "<struct %s>", v.as.obj->as.struct_inst.type_name); rsb_append(&b, t, n); break; }
        case PX_ENUM: { char t[256]; int n = snprintf(t, sizeof(t), "%s.%s", v.as.obj->as.enum_inst.type_name, v.as.obj->as.enum_inst.variant); rsb_append(&b, t, n); break; }
        case PX_RESULT: {
            LXObject* o = v.as.obj;
            char* s = px_fmt_value(o->as.result.value);
            if (o->as.result.ok) {
                rsb_append(&b, "Ok(", 3);
                rsb_append(&b, s, (int)strlen(s));
                rsb_append(&b, ")", 1);
            } else {
                rsb_append(&b, "Err(", 4);
                rsb_append(&b, s, (int)strlen(s));
                rsb_append(&b, ")", 1);
            }
            xfree(s);
            break;
        }
        case PX_GEN: {
            LXObject* o = v.as.obj;
            char t[128];
            int n = snprintf(t, sizeof(t), "<gen %d items, cursor=%d>",
                             o->as.gen.list.as.obj->as.list.len, o->as.gen.cursor);
            rsb_append(&b, t, n);
            break;
        }
        default: rsb_append(&b, "?", 1); break;
    }
    return rsb_done(&b);
}

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
    // M-B5：统一用 px_fmt_value（对齐 Rust fmt_value），保证 print 与 str() 容器渲染一致
    char* s = px_fmt_value(v);
    printf("%s", s);
    xfree(s);
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
        LXValue r = px_str_len(d, la + lb);
        xfree(d);   // ISSUE28-B2 修复：中间缓冲 px_str_len 已深拷贝，用毕即还 slab（原泄漏每拼接 1 缓冲）
        return r;
    }
    if (a.type == PX_INT && b.type == PX_INT) return px_int(a.as.i + b.as.i);
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) return px_float(num_val(a) + num_val(b));
    if (a.type == PX_LIST && b.type == PX_LIST) {
        LXValue r = px_list(a.as.obj->as.list.len + b.as.obj->as.list.len);
        px_root_push();
        PX_KEEP(r);   // M92 precise：拼接 list 跨 px_list_push 扩容分配
        LXObject* ro = r.as.obj; LXObject* ao = a.as.obj; LXObject* bo = b.as.obj;
        for (int i = 0; i < ao->as.list.len; i++) px_list_push(r, ao->as.list.items[i]);
        for (int i = 0; i < bo->as.list.len; i++) px_list_push(r, bo->as.list.items[i]);
        (void)ro;
        px_root_pop();
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
        LXValue r = px_str_len(d, len * n);
        xfree(d);   // ISSUE28-B2 修复：同上，重复串中间缓冲用毕即还
        return r;
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
    // M-B5：对齐 Rust div_euclid / floor 语义
    //   int//int：欧几里得除法（余数非负 0<=r<|d|）-7//2=-4, 7//-2=-3, -7//-2=4
    //   float 参与：floor(af/bf) 转 int（-5.5//2=-3）
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) {
        double d = num_val(b);
        if (d == 0.0) px_error("除零错误");
        return px_int((int64_t)floor(num_val(a) / d));
    }
    int64_t d = int_val(b);
    if (d == 0) px_error("除零错误");
    int64_t n = int_val(a);
    int64_t r = n % d;
    if (r < 0) r += (d < 0 ? -d : d);  // 欧几里得余数（非负）
    return px_int((n - r) / d);        // 精确整除（对齐 div_euclid 商）
}

LXValue px_mod(LXValue a, LXValue b) {
    int64_t d = int_val(b);
    if (d == 0) px_error("取模除零错误");
    if (a.type == PX_FLOAT || b.type == PX_FLOAT) return px_float(fmod(num_val(a), num_val(b)));
    // M-B5：对齐 Rust rem_euclid（余数非负）-7%3=2, 7%-3=1, -7%-3=2
    int64_t n = int_val(a);
    int64_t r = n % d;
    if (r < 0) r += (d < 0 ? -d : d);
    return px_int(r);
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
    // M-B5：dict 相等——键集合相同 + 每键值递归相等（对齐 Rust HashMap PartialEq；
    // 原缺此分支导致 dict == 恒 true，`{"a":1} == {"a":2}` 错误）
    if (a.type == PX_DICT && b.type == PX_DICT) {
        LXObject* oa = a.as.obj;
        LXObject* ob = b.as.obj;
        if (oa->as.dict.len != ob->as.dict.len)
            return oa->as.dict.len < ob->as.dict.len ? -1 : 1;
        for (int i = 0; i < oa->as.dict.len; i++) {
            const char* k = oa->as.dict.keys[i];
            LXValue* bv = NULL;
            for (int j = 0; j < ob->as.dict.len; j++) {
                if (strcmp(ob->as.dict.keys[j], k) == 0) { bv = &ob->as.dict.vals[j]; break; }
            }
            if (!bv) return 1;  // b 缺键 → a > b（不相等）
            int c = compare_values(oa->as.dict.vals[i], *bv);
            if (c != 0) return c;
        }
        return 0;
    }
    // M39：Result 相等——ok 标志相同 + 载荷递归比较
    if (a.type == PX_RESULT && b.type == PX_RESULT) {
        if (a.as.obj->as.result.ok != b.as.obj->as.result.ok) return a.as.obj->as.result.ok ? 1 : -1;
        return compare_values(a.as.obj->as.result.value, b.as.obj->as.result.value);
    }
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
    if (obj.type == PX_GEN) {
        // M34：惰性生成器先物化剩余（索引语义需要全量结果）
        if (obj.as.obj->as.gen.is_lazy) px_gen_materialize(obj.as.obj);
        obj = obj.as.obj->as.gen.list;
    }
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
        // M-B2 修复：字符串索引按 UTF-8 字符（与解释器字符语义、px_len 一致；原按字节导致中文错位）
        // M89-S3-C1 补漏（M83-S1 GAP-STR-1-B1）：索引越界须用 str.len 字节边界（strlen 在
        //   内嵌 NUL 处截断 → 含 \u{0} 的串 len()=N 但 s[0] 判越界，自举编译 pxlexer.px 崩）
        int i = (int)int_val(idx);
        int ulen = px_str_rune_len(obj.as.obj);   // M106-S2：惰性 rune 计数（首次 O(n)，之后摊还 O(1)）
        if (i < 0) i += ulen;
        if (i < 0 || i >= ulen) px_error("字符串索引越界: %d", i);
        const unsigned char* base = (const unsigned char*)obj.as.obj->as.str.data;
        const unsigned char* p;
        int* offs = px_str_offs_get(obj.as.obj);  // M106-S2：≥1KB 的串建一次偏移表
        int ocnt = offs ? __atomic_load_n(&obj.as.obj->as.str.offs_cnt, __ATOMIC_RELAXED) : 0;
        if (offs && i < ocnt) {
            p = base + offs[i];                   // 第 i 个 rune 的起始字节 —— O(1)
        } else {
            // 回落原线性走查（小串未建表；或 i ≥ 建表步数——仅畸形 UTF-8 可达）：
            //   步进规则与建表完全一致，故 offs[i] 与"走 i 步"结果逐字节相同，此处纯为等义兜底。
            p = base;
            int count = 0;
            while (count < i) { p += px_utf8_clen(*p); count++; }
        }
        unsigned char c0 = *p;
        int clen = px_utf8_clen(c0);              // M106-S2：与原判定式逐字等价
        char buf[8] = {0};
        memcpy(buf, p, clen);
        // M89-S3-C1 补漏：单字符结果须按 clen 带长构造（px_str 用 strlen → 取到 NUL 字符时
        //   截断成空串，与 compare_values 的 memcmp+len 字节安全语义不一致）
        return px_str_len(buf, clen);
    }
    if (obj.type == PX_DICT) {
        if (idx.type == PX_STR) {
            return px_dict_get(obj, idx.as.obj->as.str.data);
        }
        // M37：dict 整数索引 → 返回第 i 个键（for-in dict 用 px_len/px_index 遍历，与解释器 keys 一致）
        if (idx.type == PX_INT) {
            LXObject* o = obj.as.obj;
            int i = (int)idx.as.i;
            if (i < 0) i += o->as.dict.len;
            if (i >= 0 && i < o->as.dict.len) {
                return px_str(o->as.dict.keys[i]);
            }
            px_error("字典索引越界: %d (len=%d)", i, o->as.dict.len);
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
    else if (obj.type == PX_STR) { kind = 2; len = px_str_rune_len(obj.as.obj); }   // M106-S2：惰性缓存
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
        px_root_push();
        PX_KEEP(r);   // M92 precise：切片 list 跨 px_list_push 扩容分配
        if (k_in > 0) { for (int64_t i = s; i < e; i += k_in) px_list_push(r, obj.as.obj->as.list.items[(int)i]); }
        else { for (int64_t i = s; i > e; i += k_in) px_list_push(r, obj.as.obj->as.list.items[(int)i]); }
        px_root_pop();
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
        case PX_STR: return px_str_rune_len(v.as.obj); // M106-S2：惰性 rune 计数（原为每次 px_unicode_len_n 全扫）；M83-S1：尊重 str.len（内嵌 NUL 不再截断）
        case PX_LIST: return v.as.obj->as.list.len;
        case PX_DICT: return v.as.obj->as.dict.len;
        case PX_TUPLE: return v.as.obj->as.tuple.len;
        case PX_GEN:
            // M34：惰性生成器先物化剩余（len 需要全量）
            if (v.as.obj->as.gen.is_lazy) px_gen_materialize(v.as.obj);
            return v.as.obj->as.gen.list.as.obj->as.list.len;
        default: px_error("len 不支持类型 %s", px_type_name(v)); return 0;
    }
}

// ==================== 调用 ====================

LXValue px_call(LXValue fn, LXValue* args, int nargs) {
    if (fn.type == PX_FUNC) return fn.as.obj->as.func.fn(args, nargs, fn.as.obj->as.func.ctx);
    if (fn.type == PX_NATIVE) {
        // M31 沙箱安全：危险函数禁限（sandbox_enter deny 列表 → 调用报错，双模式同文案）
        if (g_sandbox_active && g_sandbox_deny_count > 0) {
            const char* nm = fn.as.obj->as.native.name;
            for (int i = 0; i < g_sandbox_deny_count; i++) {
                if (strcmp(g_sandbox_deny[i], nm) == 0) {
                    px_error("沙箱：函数 %s 已被禁用", nm);
                }
            }
        }
        return fn.as.obj->as.native.fn(args, nargs, NULL);
    }
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
        // M-B2：C 端 list.pop 缺失（自举 lexer 缩进栈用），与解释器一致
        if (strcmp(name, "pop") == 0) {
            if (nargs != 0) px_error("pop 不接受参数");
            LXObject* o = obj.as.obj;
            if (o->as.list.len == 0) px_error("pop 空列表");
            o->as.list.len--;
            return o->as.list.items[o->as.list.len];
        }
    }
    if (obj.type == PX_DICT) {
        if (strcmp(name, "get") == 0) {
            if (nargs < 1) px_error("get 需要 1 个参数");
            // M-B1：支持默认值参数（第 2 参数，键不存在时返回）
            LXValue v = px_dict_get(obj, args[0].as.obj->as.str.data);
            if (px_is_null(v) && nargs >= 2) return args[1];
            return v;
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
            px_root_push();
            PX_KEEP(r);   // M92 precise：keys list 跨 px_list_push/px_str 分配
            for (int i = 0; i < o->as.dict.len; i++) px_list_push(r, px_str(o->as.dict.keys[i]));
            px_root_pop();
            return r;
        }
        if (strcmp(name, "values") == 0) {
            LXObject* o = obj.as.obj;
            LXValue r = px_list(0);
            px_root_push();
            PX_KEEP(r);   // M92 precise：values list 跨 px_list_push 扩容分配
            for (int i = 0; i < o->as.dict.len; i++) px_list_push(r, o->as.dict.vals[i]);
            px_root_pop();
            return r;
        }
        if (strcmp(name, "remove") == 0) {
            // M37 修复：C 端 dict.remove 真删除（原"置 null"导致键残留：has() 仍 true、keys() 仍列出）
            if (nargs < 1) px_error("remove 需要 1 个参数");
            LXObject* o = obj.as.obj;
            const char* key = args[0].as.obj->as.str.data;
            LXValue v = px_null();
            for (int i = 0; i < o->as.dict.len; i++) {
                if (strcmp(o->as.dict.keys[i], key) == 0) {
                    v = o->as.dict.vals[i];
                    // 收缩：后续元素前移
                    for (int j = i; j < o->as.dict.len - 1; j++) {
                        o->as.dict.keys[j] = o->as.dict.keys[j + 1];
                        o->as.dict.vals[j] = o->as.dict.vals[j + 1];
                    }
                    o->as.dict.len--;
                    break;
                }
            }
            return v;
        }
    }
    // M39：Result 方法（is_ok / is_err / unwrap / ok / err）
    if (obj.type == PX_RESULT) {
        if (strcmp(name, "is_ok") == 0) {
            if (nargs != 0) px_error("is_ok 不接受参数");
            return px_bool(obj.as.obj->as.result.ok);
        }
        if (strcmp(name, "is_err") == 0) {
            if (nargs != 0) px_error("is_err 不接受参数");
            return px_bool(!obj.as.obj->as.result.ok);
        }
        if (strcmp(name, "unwrap") == 0) {
            if (nargs != 0) px_error("unwrap 不接受参数");
            if (obj.as.obj->as.result.ok) return obj.as.obj->as.result.value;
            px_error("unwrap 失败: Err(%s)", px_to_string(obj.as.obj->as.result.value));
        }
        if (strcmp(name, "ok") == 0) {
            // Ok(v) → Some(v)=v；Err(_) → null
            return obj.as.obj->as.result.ok ? obj.as.obj->as.result.value : px_null();
        }
        if (strcmp(name, "err") == 0) {
            // Err(e) → e；Ok(_) → null
            return obj.as.obj->as.result.ok ? px_null() : obj.as.obj->as.result.value;
        }
    }
    // M-B1：struct 方法（impl 方法注册为全局 "Type.method"，方法绑定 self）
    // M55/P0（issue#2）：全局表遍历与 px_set/get_global/GC 根扫描经 g_globals_mu
    // 互斥。锁内仅查找并拷贝函数值；px_call 在锁外执行——其内部会拿 g_gc_mu，
    // 避免 g_globals_mu→g_gc_mu 与 GC 的 g_gc_mu→g_globals_mu 反向死锁。
    if (obj.type == PX_STRUCT) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s.%s", obj.as.obj->as.struct_inst.type_name, name);
        LXValue m = px_null();
        bool m_found = false;
        sigset_t old;
        pthread_rwlock_rdlock(&g_globals_mu);
        gc_block_stop(&old);
        // M105-S2：原线性 strcmp 扫描 → O(1) 哈希查找（CALLM 热路径；名=「类型.方法」）
        int mslot = g_hash_find(buf, g_name_hash(buf));
        if (mslot >= 0 && (g_vals[mslot].type == PX_FUNC || g_vals[mslot].type == PX_NATIVE)) {
            m = g_vals[mslot];
            m_found = true;
        }
        gc_unblock_stop(&old);
        pthread_rwlock_unlock(&g_globals_mu);
        if (m_found) {
            LXValue* argv = xmalloc(sizeof(LXValue) * (nargs + 1));
            argv[0] = obj; // self
            for (int j = 0; j < nargs; j++) argv[j + 1] = args[j];
            LXValue r = px_call(m, argv, nargs + 1);
            xfree(argv);
            return r;
        }
    }
    px_error("对象 %s 没有方法 %s", px_type_name(obj), name);
    return px_null();
}

// ==================== 全局表 ====================

LXValue px_get_global(const char* name) {
    // M55/P0（issue#2）：与 px_set_global/GC 根扫描经 g_globals_mu 互斥。持锁 +
    // 屏蔽 SIG_GC_STOP（协议同 g_gc_mu）：临界区不被 GC 暂停，stop-the-world 取
    // 本锁不会被"已暂停持锁线程"卡死；确保读到完整值（锁内拷贝，解锁返回）。
    sigset_t old;
    pthread_rwlock_rdlock(&g_globals_mu);
    gc_block_stop(&old);
    // M105-S2：原 O(g_len) 线性 strcmp 扫描 → O(1) 哈希查找（命中数/返回值不变）
    int gi = g_hash_find(name, g_name_hash(name));
    if (gi >= 0) {
        LXValue v = g_vals[gi];
        gc_unblock_stop(&old);
        pthread_rwlock_unlock(&g_globals_mu);
        return v;
    }
    gc_unblock_stop(&old);
    pthread_rwlock_unlock(&g_globals_mu);
    px_error("未定义变量: %s", name);
    return px_null();
}

// M68：非致命全局 native 查询（bi_ffi_call 双表兜底用；未找到不 px_error）
// 锁协议与 px_get_global 一致（g_globals_mu + gc_block_stop）；仅当全局值存在
// 且为 PX_NATIVE（px_set_global 注册的内置函数）时返回 true 并拷贝出值。
// 用途：ffi_call 查 ffi 注册表未命中 → 兜底查宿主全局 PX_NATIVE → pxi 解释器
// 用户裸脚本（零 extern def）调用 runtime 全部内置函数，与编译产物可达性一致。
bool px_global_native(const char* name, LXValue* out) {
    sigset_t old;
    pthread_rwlock_rdlock(&g_globals_mu);
    gc_block_stop(&old);
    // M105-S2：同 px_get_global，改 O(1) 哈希查找（语义：名存在则按类型返回）
    int gi = g_hash_find(name, g_name_hash(name));
    if (gi >= 0) {
        LXValue v = g_vals[gi];
        gc_unblock_stop(&old);
        pthread_rwlock_unlock(&g_globals_mu);
        if (v.type == PX_NATIVE) {
            if (out) *out = v;
            return true;
        }
        return false;   // 名存在但非内置函数（用户变量/常量/结构体等）
    }
    gc_unblock_stop(&old);
    pthread_rwlock_unlock(&g_globals_mu);
    return false;
}

void px_set_global(const char* name, LXValue v) {
    // M55/P0（issue#2）：写全局表全程持 g_globals_mu（含 g_len++ 与 key/val 槽位
    // 写入），与 px_get_global 读、GC 根扫描互斥；g_len 在锁内更新保证原子可见。
    // 错误路径先解锁再 px_error（px_error 不持锁返回，避免锁泄漏/死锁）。
    sigset_t old;
    uint64_t hv = g_name_hash(name);   // M105-S2：哈希在拿写锁前算（纯函数，只读 name）
    pthread_rwlock_wrlock(&g_globals_mu);
    gc_block_stop(&old);
    // M105-S2：原 O(g_len) 线性 strcmp 扫描 → O(1) 哈希查找；新增槽位处同步落位索引
    int gi = g_hash_find(name, hv);
    if (gi >= 0) {
        g_vals[gi] = v;
        gc_unblock_stop(&old);
        pthread_rwlock_unlock(&g_globals_mu);
        return;
    }
    if (g_len >= GLOBAL_CAP) {
        gc_unblock_stop(&old);
        pthread_rwlock_unlock(&g_globals_mu);
        px_error("全局表溢出");
        return;
    }
    g_keys[g_len] = xstrdup(name);
    g_vals[g_len] = v;
    g_hash_put(hv, g_len);
    g_len++;
    gc_unblock_stop(&old);
    pthread_rwlock_unlock(&g_globals_mu);
}

// ==================== 内置函数 ====================

// M96-S4：并发 print 整行原子锁 —— 帧协程多 worker 共写同一 stdout，行内多次
//   printf 非原子（stdio 锁仅单次调用）→ 行交错合并（R37 行写一半 R39 插入）。
//   GC 标记不涉此锁，简单互斥即可；print 为低频 I/O，串行化开销可忽略。
static pthread_mutex_t g_print_mu = PTHREAD_MUTEX_INITIALIZER;

static LXValue bi_print(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    pthread_mutex_lock(&g_print_mu);
    for (int i = 0; i < nargs; i++) {
        if (i) printf(" ");
        px_print_value(args[i], false);
    }
    printf("\n");
    // M72-S1（Issue 9）：print 即人读日志——stdout 重定向到管道/journald/文件时
    // C stdio 变全缓冲（8192B），不刷则服务日志运行中不可见、崩溃前缓冲丢失。
    // 行尾 fflush → 每行实时（行级 syscall 对日志场景可接受）。
    fflush(stdout);
    pthread_mutex_unlock(&g_print_mu);
    return px_null();
}

// M72-S1（Issue 9 方案 C 最小版）：flush() —— .px 脚本自主刷 stdout/stderr
static LXValue bi_flush(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("flush 不接受参数");
    fflush(stdout);
    fflush(stderr);
    return px_null();
}

// M72-S1（Issue 9/10）：print_err(...) —— 渲染对齐 print（px_fmt_value）但输出到
// stderr（默认无缓冲，天然实时）→ 服务错误/诊断出口统一 stderr 的基础。
static LXValue bi_print_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    pthread_mutex_lock(&g_print_mu);
    for (int i = 0; i < nargs; i++) {
        if (i) fputs(" ", stderr);
        char* s = px_fmt_value(args[i]);
        fputs(s, stderr);
        xfree(s);
    }
    fputc('\n', stderr);
    fflush(stderr);
    pthread_mutex_unlock(&g_print_mu);
    return px_null();
}

// M39：Result/Option 构造函数（spec §3.5；Some(x) = x，None 即 null）
static LXValue bi_ok(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("Ok 需要 1 个参数");
    return px_ok(args[0]);
}
static LXValue bi_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("Err 需要 1 个参数");
    return px_err(args[0]);
}
static LXValue bi_some(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("Some 需要 1 个参数");
    return args[0];
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
    px_root_push();
    PX_KEEP(r);   // M92 precise：累积 list 跨 px_list_push 扩容分配
    if (step > 0) for (int64_t i = start; i < end; i += step) px_list_push(r, px_int(i));
    else for (int64_t i = start; i > end; i += step) px_list_push(r, px_int(i));
    px_root_pop();
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
    // M-B5：统一用 px_fmt_value——str() 支持全部类型（list/dict/enum/struct/result 等），
    // 对齐 Rust 内置 str()（fmt_value 渲染）
    char* s = px_fmt_value(args[0]);
    LXValue r = px_str(s);
    xfree(s);
    return r;
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
    // M88-S2（qg-issue 27）：EINTR 自动续睡——并发 GC（M11 stop-the-world）向所有已注册
    // 线程发 SIG_GC_STOP 实时信号，nanosleep 被信号打断返回 EINTR（nanosleep 不在
    // SA_RESTART 自动重启清单）。若不续睡，主线程 sleep(长) 会在首轮 GC 后提前返回 →
    // main 结束 → 进程静默退出（高并发压测"服务进程悄然消失"根因）。timer_sleep_ms/
    // sleep_us 均已按此模式续睡，此处对齐。
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
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
    // M36 修复：C 端 now_ms 用 CLOCK_REALTIME（Unix 毫秒），与解释器 SystemTime 一致
    // （原用 CLOCK_MONOTONIC 导致双模式时间基准不一致：now_ms()/1000 无法算日期）
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
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

// ==================== M59 数学与随机内置 ====================
// 设计（docs/M59_PLAN.md §三）：延续 abs/sqrt/pow 先例（C libm 内置，零新依赖），
// 域错误透传 C 语义 NaN/+inf（不终止），参数个数/类型错误 px_error 终止（编程契约）。
// pi/e 常量不用 M_PI/M_E（防个别 libm 缺省宏），本地宏常量更稳。

#ifndef PX_PI
#define PX_PI 3.14159265358979323846
#endif
#ifndef PX_E
#define PX_E 2.71828182845904523536
#endif

// 一元数学参数校验：仅接受 int/float（字符串/列表等 → 终止）
static double math_num(LXValue v, const char* fn) {
    if (v.type == PX_INT) return (double)v.as.i;
    if (v.type == PX_FLOAT) return v.as.f;
    px_error("%s 参数必须是数字，实际是 %s", fn, px_type_name(v));
    return 0.0;
}

static LXValue bi_sin(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sin 需要 1 个参数（弧度）");
    return px_float(sin(math_num(args[0], "sin")));
}

static LXValue bi_cos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("cos 需要 1 个参数（弧度）");
    return px_float(cos(math_num(args[0], "cos")));
}

static LXValue bi_tan(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("tan 需要 1 个参数（弧度）");
    return px_float(tan(math_num(args[0], "tan")));
}

static LXValue bi_atan2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("atan2 需要 2 个参数（先 y 后 x）");
    return px_float(atan2(math_num(args[0], "atan2"), math_num(args[1], "atan2")));
}

// ---- M59-S2：floor/ceil/round（返回 float，与 sqrt 一致）+ log/log10/exp ----
// round 用 C99 round：.5 远离零（round(2.5)=3, round(-2.5)=-3）
static LXValue bi_floor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("floor 需要 1 个参数");
    return px_float(floor(math_num(args[0], "floor")));
}

static LXValue bi_ceil(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("ceil 需要 1 个参数");
    return px_float(ceil(math_num(args[0], "ceil")));
}

static LXValue bi_round(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("round 需要 1 个参数");
    return px_float(round(math_num(args[0], "round")));
}

static LXValue bi_log(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("log 需要 1 个参数（自然对数 ln）");
    return px_float(log(math_num(args[0], "log")));
}

static LXValue bi_log10(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("log10 需要 1 个参数");
    return px_float(log10(math_num(args[0], "log10")));
}

static LXValue bi_exp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("exp 需要 1 个参数");
    return px_float(exp(math_num(args[0], "exp")));
}

// ---- M59-S3：splitmix64 PRNG + random/random_int/random_seed ----
// splitmix64：确定性 64 位 PRNG（质量良好、glibc/musl 跨平台序列一致，不依赖 C rand 的平台差异，
// 静态二进制 + aarch64 交叉下序列可复现）。默认种子取 CLOCK_REALTIME 纳秒 ^ PID 混合
// （首次调用惰性初始化）；random_seed(s) 显式设种子后同 seed → 同序列（测试可复现）。
// 线程注意：static 状态在多协程并发调用下序列不保证（游戏/边缘主循环单线程为主，文档注明）。
static uint64_t px_rng_state = 0;
static int px_rng_seeded = 0;

static uint64_t splitmix64_next(void) {
    if (!px_rng_seeded) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        px_rng_state = ((uint64_t)ts.tv_sec << 32) ^ (uint64_t)ts.tv_nsec ^ ((uint64_t)getpid() << 1);
        if (!px_rng_state) px_rng_state = 0x9E3779B97F4A7C15ULL;
        px_rng_seeded = 1;
    }
    uint64_t z = (px_rng_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static LXValue bi_random(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("random 不需要参数");
    // [0,1)：53 位尾数均匀（double 可精确表示 ≤2^53 的整数）
    return px_float((double)(splitmix64_next() >> 11) * (1.0 / 9007199254740992.0));
}

static LXValue bi_random_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("random_int 需要 1 个参数 n（正整数），返回 [0,n) 的整数");
    int64_t n = args[0].as.i;
    if (n <= 0) px_error("random_int 的 n 必须 > 0，实际是 %lld", (long long)n);
    return px_int((int64_t)(splitmix64_next() % (uint64_t)n));
}

static LXValue bi_random_seed(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("random_seed 需要 1 个整数参数（种子）");
    px_rng_state = (uint64_t)args[0].as.i;
    px_rng_seeded = 1;
    return px_null();
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
    px_root_push();
    PX_KEEP(r);   // M92 precise：累积 list 跨 px_list_push/px_str_len 分配
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
        px_root_pop();
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
    px_root_pop();
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

// M83-S1（Issue 16 GAP-STR-1-B1）：字节级子串查找（hay[0..hl)/ned[0..nl)，可含 NUL），
// 替代 strstr 的 C 字符串语义（遇 NUL 截断）。对无内嵌 NUL 的文本与 strstr 结果一致。
static const char* px_memmem(const char* hay, int hl, const char* ned, int nl) {
    if (nl == 0) return hay;
    if (hl < nl) return NULL;
    for (int i = 0; i <= hl - nl; i++) {
        if (hay[i] == ned[0] && memcmp(hay + i, ned, (size_t)nl) == 0) return hay + i;
    }
    return NULL;
}

// contains(容器, 元素) -> bool（字符串/列表）
static LXValue bi_contains(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("contains 需要 2 个参数");
    if (args[0].type == PX_STR) {
        if (args[1].type != PX_STR) return px_bool(false);
        LXObject* h = args[0].as.obj;
        LXObject* n = args[1].as.obj;
        // M83-S1：字节 memmem 语义（str.len 边界，内嵌 NUL 的二进制 str 不再截断）
        return px_bool(px_memmem(h->as.str.data, h->as.str.len, n->as.str.data, n->as.str.len) != NULL);
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
    px_root_push();
    PX_KEEP(r);   // M92 precise：拷贝 list 跨 px_list_push 扩容分配
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
    px_root_pop();
    return r;
}

static LXValue bi_reversed(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("reversed 需要一个参数");
    if (args[0].type == PX_LIST) {
        LXObject* o = args[0].as.obj;
        LXValue r = px_list(o->as.list.len);
        px_root_push();
        PX_KEEP(r);   // M92 precise：累积 list 跨 px_list_push 扩容分配
        for (int i = o->as.list.len - 1; i >= 0; i--) px_list_push(r, o->as.list.items[i]);
        px_root_pop();
        return r;
    }
    if (args[0].type == PX_TUPLE) {
        LXObject* o = args[0].as.obj;
        LXValue r = px_list(o->as.tuple.len);
        px_root_push();
        PX_KEEP(r);   // M92 precise：累积 list 跨 px_list_push 扩容分配
        for (int i = o->as.tuple.len - 1; i >= 0; i--) px_list_push(r, o->as.tuple.items[i]);
        px_root_pop();
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
    if (nargs < 2 || nargs > 3 || args[0].type != PX_STR) px_error("write_file 需要 (路径, 内容[, mode])");
    const char* path = args[0].as.obj->as.str.data;
    const char* content;
    int clen;
    if (args[1].type == PX_STR) { content = args[1].as.obj->as.str.data; clen = args[1].as.obj->as.str.len; }
    else { content = px_to_string(args[1]); clen = (int)strlen(content); }
    mode_t mode = 0666;
    int has_mode = 0;
    if (nargs == 3) {
        if (args[2].type != PX_INT) px_error("write_file 的 mode 需要 int（八进制权限，如 0o600）");
        mode = (mode_t)args[2].as.i;
        has_mode = 1;
    }
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd < 0) px_error("io: 写入文件失败 %s", path);
    if (has_mode) fchmod(fd, mode);  // 显式 mode 不受 umask 影响（写 0600 密钥）
    const char* p = content;
    int left = clen;
    while (left > 0) {
        ssize_t n = write(fd, p, (size_t)left);
        if (n < 0) { if (errno == EINTR) continue; int e = errno; close(fd); errno = e; px_error("io: 写入文件失败 %s", path); }
        p += n; left -= (int)n;
    }
    close(fd);
    return px_null();
}

static LXValue bi_append_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3 || args[0].type != PX_STR) px_error("append_file 需要 (路径, 内容[, mode])");
    const char* path = args[0].as.obj->as.str.data;
    const char* content;
    int clen;
    if (args[1].type == PX_STR) { content = args[1].as.obj->as.str.data; clen = args[1].as.obj->as.str.len; }
    else { content = px_to_string(args[1]); clen = (int)strlen(content); }
    mode_t mode = 0666;
    int has_mode = 0;
    if (nargs == 3) {
        if (args[2].type != PX_INT) px_error("append_file 的 mode 需要 int（八进制权限，如 0o600）");
        mode = (mode_t)args[2].as.i;
        has_mode = 1;
    }
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, mode);
    if (fd < 0) px_error("io: 追加写入失败 %s", path);
    if (has_mode) fchmod(fd, mode);
    const char* p = content;
    int left = clen;
    while (left > 0) {
        ssize_t n = write(fd, p, (size_t)left);
        if (n < 0) { if (errno == EINTR) continue; int e = errno; close(fd); errno = e; px_error("io: 追加写入失败 %s", path); }
        p += n; left -= (int)n;
    }
    close(fd);
    return px_null();
}

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

// ==================== M57-S1：边缘设备层 fd 原语（open/close/ioctl/os_errno） ====================
// 背景（M57 重定向为"边缘设备层支持"，与清歌嵌入式讨论结论一致）：
//   Linux 用户态控制外设 90% 收敛到 open/read/write + ioctl + mmap；而 runtime
//   文件 IO 是路径式（read_at/write_at 内部 open 用完即关），语言面**没有持久
//   fd 句柄** → 本步补齐 open/close（fd 通道）+ ioctl（设备控制主入口，
//   i2c-dev / spi-dev / gpio(老 ioctl) / tty / 网卡全走它）。read/write 在 fd
//   上的封装随 S2（mmap）一并设计，S1 先打通"打开设备 → ioctl → 关闭"闭环。
// 语义：
//   open(path[, mode]) → fd(int)；失败返回 -1，os_errno() 查 errno
//     mode: "r"=O_RDONLY(默认) "w"=O_WRONLY|O_CREAT|O_TRUNC "a"=O_WRONLY|O_CREAT|O_APPEND
//           "rw"/"r+"=O_RDWR "w+"=O_RDWR|O_CREAT|O_TRUNC；设备文件典型 "r"（只读查询）/"rw"（读写控制）
//   close(fd) → bool（是否成功关闭）
//   ioctl(fd, request[, arg]) → int（ioctl 原始返回值；失败 -1 + os_errno()）
//     arg 缺省/null → NULL（无数据 ioctl）
//     arg int       → 整数值直接传递（驱动按 unsigned long 收值的整数型 ioctl）
//     arg bytes/str → 就地 in/out 缓冲区（_IOR/_IOWR 类 ioctl 内核读写该内存；
//                     调用后同一对象内容被更新，用 bytes_to_int 等读回）
//   os_errno() → int（最近一次系统调用失败 errno，线程局部）
//   request 为 32 位码（_IOC 编码，最高 2 位方向位可 >2^31；语言里用 0x 字面量/十进制均可）
static LXValue bi_open(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2) px_error("open 需要 (path[, mode]) 参数");
    const char* path = val_cstr(args[0]);
    const char* mode = (nargs >= 2) ? val_cstr(args[1]) : "r";
    int flags;
    if (!strcmp(mode, "r") || !strcmp(mode, "rb")) flags = O_RDONLY;
    else if (!strcmp(mode, "w") || !strcmp(mode, "wb")) flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (!strcmp(mode, "a") || !strcmp(mode, "ab")) flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (!strcmp(mode, "rw") || !strcmp(mode, "r+")) flags = O_RDWR;
    else if (!strcmp(mode, "w+")) flags = O_RDWR | O_CREAT | O_TRUNC;
    else px_error("open 的 mode 不支持: %s（支持 r/w/a/rw/w+）", mode);
    int fd = open(path, flags, 0644);
    if (fd < 0) return px_int(-1);
    return px_int((int64_t)fd);
}

static LXValue bi_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("close 需要 (fd) 参数");
    return px_bool(close((int)args[0].as.i) == 0);
}

static LXValue bi_ioctl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("ioctl 需要 (fd, request[, arg]) 参数");
    if (args[0].type != PX_INT) px_error("ioctl 的 fd 需要 int");
    if (args[1].type != PX_INT) px_error("ioctl 的 request 需要 int");
    int fd = (int)args[0].as.i;
    // request 码为 32 位（_IOC(dir,type,nr,size) 编码于低 32 位，方向位在最高 2 位）
    unsigned long req = (unsigned long)(uint32_t)args[1].as.i;
    int rc;
    if (nargs < 3 || args[2].type == PX_NULL) {
        rc = ioctl(fd, req, (void*)0);
    } else if (args[2].type == PX_INT) {
        rc = ioctl(fd, req, (void*)(uintptr_t)args[2].as.i);
    } else if (args[2].type == PX_STR || args[2].type == PX_BYTES) {
        // 就地 in/out 缓冲区：驱动读写该对象 data（_IOR 类调用后内容被填充），
        // bytes/str 对象是引用语义，调用方同一变量即可读回结果
        rc = ioctl(fd, req, (void*)args[2].as.obj->as.str.data);
    } else {
        px_error("ioctl 的 arg 需要 int/bytes/str/null，实际 %s", px_type_name(args[2]));
        rc = -1;
    }
    if (rc < 0) return px_int(-1);
    return px_int((int64_t)rc);
}

static LXValue bi_os_errno(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("os_errno 不需要参数");
    return px_int((int64_t)errno);
}

// ==================== M57-S2：边缘设备层 fd 数据通道 + mmap 设备映射 ====================
// 延续 M57-S1 的 fd 原语：S1 打通"打开设备 → ioctl 配置 → 关闭"，本步补齐
//   ① fd 上的 read/write 数据通道（S1 注释承诺：read/write 在 fd 上的封装随 S2 设计；
//      设备文件顺序读写/收发的通用入口，read(2)/write(2) 直通，与 socket 无关）
//   ② mmap/munmap 设备映射：帧缓冲(/dev/fb0)/共享内存/DMA 缓冲 → bytes 视图，
//      配合 ioctl 完成设备"配置 + 大数据块直接内存访问"双通道
//   ③ mem_write：mmap 视图的语言层就地写（bytes_set 是 COW 复制语义改不了映射内存；
//      帧缓冲写像素、共享内存写数据必须就地写底层映射区）
// 生命周期设计（关键）：mmap 返回的 bytes 带 is_mmap 标志（LXObject 位域），其 data
//   指向 mmap 映射区而非 xmalloc 堆块 → GC sweep 时 munmap 而非 xfree；munmap() 显式
//   提前解除后置 data=NULL/len=0/is_mmap=0，防 double-unmap。
// 语义：
//   read(fd, maxlen) → bytes（实际读到的字节；0 长度 = EOF；失败 int -1 + os_errno()）
//   write(fd, data)  → int（实际写入字节数；失败 -1 + os_errno()；data 为 bytes/str）
//   mmap(fd, length[, offset]) → bytes（PROT_READ|PROT_WRITE + MAP_SHARED 活映射视图；
//     GC 自动回收时 munmap；失败 int -1 + os_errno()；length 1..INT_MAX-1，offset 须页对齐）
//   munmap(bytes) → bool（显式提前解除映射；非映射/已解除返回 false）
//   mem_write(mmap_bytes, offset, data) → int（就地写映射视图 [offset..]，超长截断到视图尾）
static LXValue bi_read(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("read 需要 (fd, maxlen) 参数");
    if (args[0].type != PX_INT) px_error("read 的 fd 需要 int");
    int64_t maxlen = int_val(args[1]);
    if (maxlen <= 0 || maxlen > (int64_t)INT_MAX - 1) px_error("read 的 maxlen 需要 1..INT_MAX-1");
    int fd = (int)args[0].as.i;
    char* buf = xmalloc((size_t)maxlen + 1);
    ssize_t n = read(fd, buf, (size_t)maxlen);
    if (n < 0) {
        int e = errno;
        xfree(buf);
        errno = e;
        return px_int(-1);
    }
    LXValue r = px_bytes_len(buf, (int)n);   // n==0 → 空 bytes（EOF），类型上与 int -1 区分
    xfree(buf);
    return r;
}

static LXValue bi_write(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("write 需要 (fd, data) 参数");
    if (args[0].type != PX_INT) px_error("write 的 fd 需要 int");
    LXValue d = args[1];
    if (d.type != PX_BYTES && d.type != PX_STR) px_error("write 的 data 需要 bytes/str");
    int fd = (int)args[0].as.i;
    const char* data = d.as.obj->as.str.data;
    int len = d.as.obj->as.str.len;
    ssize_t n;
    do { n = write(fd, data, (size_t)len); } while (n < 0 && errno == EINTR);
    if (n < 0) return px_int(-1);
    return px_int((int64_t)n);
}

static LXValue bi_mmap(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("mmap 需要 (fd, length[, offset]) 参数");
    if (args[0].type != PX_INT) px_error("mmap 的 fd 需要 int");
    int64_t l = int_val(args[1]);
    if (l <= 0 || l >= (int64_t)INT_MAX) px_error("mmap 的 length 需要 1..INT_MAX-1");
    off_t off = 0;
    if (nargs >= 3) {
        if (args[2].type != PX_INT) px_error("mmap 的 offset 需要 int");
        off = (off_t)args[2].as.i;
    }
    int fd = (int)args[0].as.i;
    size_t len = (size_t)l;
    void* p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, off);
    if (p == MAP_FAILED) return px_int(-1);
    LXValue v; v.type = PX_BYTES;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = PX_BYTES;
    o->as.str.data = (char*)p;
    o->as.str.len = (int)len;
    o->as.str.rune_len = -1; o->as.str.offs_cnt = 0; o->as.str.rune_offs = NULL;  // M106-S2：mmap bytes 恒不建表
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + len);  // 置 gc_mark=0/is_mmap=0 并注册（可能触发 GC；对象受 g_tmp_root 保护）
    o->is_mmap = 1;                          // 注册后再标 mmap，防 sweep 在标记前误判回收
    return v;
}

static LXValue bi_munmap(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("munmap 需要 (bytes) 参数");
    if (args[0].type != PX_BYTES) px_error("munmap 需要 bytes，实际 %s", px_type_name(args[0]));
    LXObject* o = args[0].as.obj;
    if (!o->is_mmap || !o->as.str.data) return px_bool(false);   // 非映射 / 已解除
    if (munmap(o->as.str.data, (size_t)o->as.str.len) != 0) return px_bool(false);
    o->as.str.data = NULL;
    o->as.str.len = 0;
    o->as.str.rune_len = -1; o->as.str.offs_cnt = 0;   // M106-S2：连缓存一起复位（防解除后误用旧表）
    o->as.str.rune_offs = NULL;                        // （bytes 本不建表，纯防御）
    o->is_mmap = 0;
    return px_bool(true);
}

static LXValue bi_mem_write(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("mem_write 需要 (mmap_bytes, offset, data) 参数");
    if (args[0].type != PX_BYTES) px_error("mem_write 的目标需要 bytes，实际 %s", px_type_name(args[0]));
    LXObject* dst = args[0].as.obj;
    if (!dst->is_mmap || !dst->as.str.data)
        px_error("mem_write 的目标需要未解除的 mmap 映射视图（仅 mmap 返回的 bytes 可就地写；普通 bytes 用 bytes_set COW）");
    int dlen = dst->as.str.len;
    int64_t offv = int_val(args[1]);
    if (offv < 0 || offv >= dlen) px_error("mem_write 的 offset 越界（len=%d offset=%lld）", dlen, (long long)offv);
    LXValue src = args[2];
    if (src.type != PX_BYTES && src.type != PX_STR) px_error("mem_write 的 data 需要 bytes/str");
    const char* sdata = src.as.obj->as.str.data;
    int slen = src.as.obj->as.str.len;
    int room = dlen - (int)offv;
    int n = slen < room ? slen : room;
    if (n > 0) memcpy(dst->as.str.data + offv, sdata, (size_t)n);
    return px_int((int64_t)n);
}

// ==================== M60-S1：us 级时钟 + fd 控制（sleep_us/now_us/fcntl） ====================
// 设计（docs/M60_PLAN.md §三.2）：服务边缘设备 GAP #5（1-Wire/DHT 时序需 us 级睡眠/测量）
// 与 fd 标准非阻塞姿势（fcntl，open 无 O_NONBLOCK mode 的补足通道）。
// 语义：
//   sleep_us(us) → null：us 级睡眠（nanosleep，EINTR 自动续睡；<=0 直接返回不睡）
//   now_us()     → int：CLOCK_MONOTONIC 微秒（测量/计时语义，单调不受墙钟调整影响；
//                        与 now_ms 的 CLOCK_REALTIME 墙钟用途区分，文档写明）
//   fcntl(fd, cmd[, arg]) → int：标准 fcntl（F_GETFL/F_SETFL/O_NONBLOCK 等）；
//                        失败 int -1 + os_errno() 查询（不杀进程，延续 M57 设备失败语义）
static LXValue bi_sleep_us(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sleep_us 需要 1 个参数（微秒）");
    int64_t us = int_val(args[0]);
    if (us <= 0) return px_null();
    struct timespec ts;
    ts.tv_sec = us / 1000000L;
    ts.tv_nsec = (us % 1000000L) * 1000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
    return px_null();
}

// M93-S3：阻塞 native 识别（vm.c CALL 预检）——协程上下文里 sleep/sleep_us 改由
//   定时器登记让出（不阻塞 worker）。函数指针比较（static 同编译单元内可达）。
int px_native_blocking_kind(LXValue fn) {
    if (fn.type != PX_NATIVE) return PX_BLK_NONE;
    LXFuncPtr fp = fn.as.obj->as.native.fn;
    if (fp == bi_sleep)    return PX_BLK_SLEEP_MS;
    if (fp == bi_sleep_us) return PX_BLK_SLEEP_US;
    return PX_BLK_NONE;
}

// M96-S2/S3：阻塞网络 native offload 识别（vm.c CALL 预检）。名单 = C 层全协议阻塞桥
//   （http 解析 recv 循环/mbedtls TLS 阻塞/s3 连接池/ws 客户端/tcp/udp/dns —— 阻塞点
//   在 C native 内部，M93-S3 让出（VM 层可控等待）与 M94 抢占（VM 指令边界）都救不了）。
//   协程 ctx 命中 → 外包执行线程池（D1/D2，β 路线）；否则 px_call 直调（pthread 阻塞）。
//   按 native 名匹配（as.native.name）：跨文件安全（ws 在 runtime_ws.c 注册同名 native）
//   且不依赖 static 函数指针跨编译单元可见性。S2 试点 http_get/tcp_recv；S3 扩全集。
//   排除：服务端/控制类（tcp_listen/accept、udp_open/udp_serve、tcp_close/udp_close、
//   ws_serve/ws_close/ws_ping/ws_broadcast/ws_heartbeat —— 非 VM 协程 ctx 或非阻塞）；
//   fd_wait（用户显式短等自控超时）；sleep/chan/mutex（已让出，非网络）。
int px_native_offload_kind(LXValue fn) {
    if (fn.type != PX_NATIVE) return 0;
    const char* nm = fn.as.obj->as.native.name;
    if (!nm || !nm[0]) return 0;
    // HTTP 客户端全协议（get/post/unix socket；get_stream 排除 —— chunk 回调用户 VM
    //   函数，违反 D5「名单 = 纯网络 native 不回调 VM」约束，二期候选）
    if (strcmp(nm, "http_get") == 0) return 1;
    if (strcmp(nm, "http_post") == 0) return 1;
    if (strcmp(nm, "http_unix") == 0) return 1;
    // TCP/UDP 客户端阻塞 syscall
    if (strcmp(nm, "tcp_connect") == 0) return 1;
    if (strcmp(nm, "tcp_send") == 0) return 1;
    if (strcmp(nm, "tcp_recv") == 0) return 1;
    if (strcmp(nm, "udp_send") == 0) return 1;
    if (strcmp(nm, "udp_recv") == 0) return 1;
    // DNS（getaddrinfo 可秒级）
    if (strcmp(nm, "dns_lookup") == 0) return 1;
    // S3 客户端（SigV4 + 连接池 h_exchange）
    if (strcmp(nm, "s3_put") == 0) return 1;
    if (strcmp(nm, "s3_get") == 0) return 1;
    if (strcmp(nm, "s3_delete") == 0) return 1;
    if (strcmp(nm, "s3_list") == 0) return 1;
    // WS 客户端（runtime_ws.c：ws_connect/ws_send/ws_recv/ws_connect_auto）
    if (strcmp(nm, "ws_connect") == 0) return 1;
    if (strcmp(nm, "ws_connect_auto") == 0) return 1;
    if (strcmp(nm, "ws_send") == 0) return 1;
    if (strcmp(nm, "ws_recv") == 0) return 1;
    // M102-S2a：.px 子进程池执行（px_exec → px_pool_run C 阻塞等 px --worker 结果帧；
    //   popen/子进程等待类 —— M96 D2 二期候选兑现）。协程 ctx 命中 → 外包执行线程池跑
    //   px_pool_run（阻塞等子进程在 offload 线程），worker 释放 —— 语言层 .px 调用与
    //   handler 内 px_exec 不卡 worker。px_exec 内 px_call(json_stringify) 为 native
    //   直调（不回调用户 VM）→ 满足 D5 名单约束。
    if (strcmp(nm, "px_exec") == 0) return 1;
    return 0;
}

static LXValue bi_now_us(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return px_int((int64_t)ts.tv_sec * 1000000L + ts.tv_nsec / 1000L);
}

static LXValue bi_fcntl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("fcntl 需要 (fd, cmd[, arg]) 参数");
    if (args[0].type != PX_INT) px_error("fcntl 的 fd 需要 int");
    if (args[1].type != PX_INT) px_error("fcntl 的 cmd 需要 int");
    int fd = (int)args[0].as.i;
    int cmd = (int)args[1].as.i;
    long arg = 0;
    if (nargs >= 3) {
        if (args[2].type == PX_INT) arg = (long)args[2].as.i;
        else if (args[2].type == PX_BOOL) arg = args[2].as.b ? 1 : 0;
        else px_error("fcntl 的 arg 需要 int/bool，实际 %s", px_type_name(args[2]));
    }
    int rc;
    do { rc = fcntl(fd, cmd, arg); } while (rc < 0 && errno == EINTR);
    if (rc < 0) return px_int(-1);   // os_errno() 查询具体原因
    return px_int((int64_t)rc);
}

// ==================== M60-S2：设备组 tty_config / fd_wait ====================
// 设计（docs/M60_PLAN.md §三.2）：服务边缘设备 GAP #3（串口 UART 无 termios：设不了
// 波特率/raw 模式）与 #2（GPIO 边沿中断/多 fd 等待无 fd 多路复用）。内部 poll 已有
// （子进程池读帧用），本步将其暴露为语言函数 fd_wait。失败语义延续 M57：-1/false + os_errno()。
//   tty_config(fd, baud, raw) → bool：tcgetattr →（raw=true 则 cfmakeraw：关 canonical/echo/
//     信号转换，串口 raw 标准姿势）→ cfsetispeed+cfsetospeed → tcsetattr(TCSANOW)。
//     raw=false 仅改波特率保留原模式。baud 支持常规档 9600/19200/38400/57600/115200/
//     230400/460800/921600；不支持的档 px_error（编程契约参数错误）。
//   fd_wait(fds, timeout_ms) → list<就绪 fd>（空=超时）| int -1（poll 系统错误 + os_errno）。
//     fds 接受 int 单 fd 或 list<int>（上限 64）；timeout_ms>=0（0=立即查，负值 px_error）；
//     events 只监听 POLLIN 可读；revents 非 0（POLLIN/POLLHUP/POLLERR/POLLNVAL）即视为
//     事件返回该 fd——HUP/ERR 场景由上层随后 read 判 EOF(空 bytes)/-1+errno。EINTR 自动续等。
static LXValue bi_tty_config(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("tty_config 需要 (fd, baud, raw) 参数");
    if (args[0].type != PX_INT) px_error("tty_config 的 fd 需要 int");
    int fd = (int)args[0].as.i;
    int64_t baud = int_val(args[1]);
    if (args[2].type != PX_BOOL) px_error("tty_config 的 raw 需要 bool");
    int raw = args[2].as.b ? 1 : 0;
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0) return px_bool(false);   // os_errno() 查具体原因
    if (raw) cfmakeraw(&tio);
    speed_t sp;
    switch (baud) {
        case 9600:   sp = B9600;   break;
        case 19200:  sp = B19200;  break;
        case 38400:  sp = B38400;  break;
        case 57600:  sp = B57600;  break;
        case 115200: sp = B115200; break;
        case 230400: sp = B230400; break;
        case 460800: sp = B460800; break;
        case 921600: sp = B921600; break;
        default:
            px_error("tty_config 不支持的波特率 %lld（支持 9600/19200/38400/57600/115200/230400/460800/921600）", (long long)baud);
            return px_bool(false);
    }
    if (cfsetispeed(&tio, sp) != 0) return px_bool(false);
    if (cfsetospeed(&tio, sp) != 0) return px_bool(false);
    if (tcsetattr(fd, TCSANOW, &tio) != 0) return px_bool(false);
    return px_bool(true);
}

static LXValue bi_fd_wait(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("fd_wait 需要 (fds, timeout_ms) 参数");
    if (args[1].type != PX_INT) px_error("fd_wait 的 timeout_ms 需要 int");
    int64_t tmo = args[1].as.i;
    if (tmo < 0) px_error("fd_wait 的 timeout_ms 需要 >= 0");
    if (tmo > 2147483647L) tmo = 2147483647L;   // poll 超时 int 上限保护
    int fds[64];
    int n = 0;
    if (args[0].type == PX_INT) {
        fds[n++] = (int)args[0].as.i;
    } else if (args[0].type == PX_LIST) {
        LXObject* o = args[0].as.obj;
        if (o->as.list.len > 64) px_error("fd_wait 一次最多监听 64 个 fd");
        for (int i = 0; i < o->as.list.len; i++) {
            if (o->as.list.items[i].type != PX_INT) px_error("fd_wait 的 fds 列表元素需要 int");
            fds[n++] = (int)o->as.list.items[i].as.i;
        }
    } else {
        px_error("fd_wait 的 fds 需要 int 或 list<int>，实际 %s", px_type_name(args[0]));
    }
    if (n == 0) return px_list(0);   // 空集合立即返回空
    struct pollfd pfds[64];
    for (int i = 0; i < n; i++) { pfds[i].fd = fds[i]; pfds[i].events = POLLIN; pfds[i].revents = 0; }
    int rc;
    do { rc = poll(pfds, (nfds_t)n, (int)tmo); } while (rc < 0 && errno == EINTR);
    if (rc < 0) return px_int(-1);   // os_errno() 查（如 EINVAL）
    LXValue r = px_list(0);
    if (rc == 0) return r;           // 超时 → 空 list（与就绪返回类型统一，非错误）
    px_root_push();
    PX_KEEP(r);   // M92 precise：就绪 list 跨 px_list_push 扩容分配
    for (int i = 0; i < n; i++) {
        if (pfds[i].revents != 0) px_list_push(r, px_int(pfds[i].fd));
    }
    px_root_pop();
    return r;
}

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
// M84-S2（Issue 21）：bdata/blen 二进制安全——接受 str|bytes|数值，bytes 与含 NUL 载荷
// 完整哈希（不再被 strlen 截断）；纯 ASCII str 输出与旧版逐字节一致（零回归）。
static LXValue bi_sha256(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sha256 需要一个参数");
    const char* data = bdata(args[0]);
    int len = blen(args[0]);
    unsigned char digest[32];
    if (mbedtls_sha256((const unsigned char*)data, (size_t)len, digest, 0) != 0)
        px_error("sha256 计算失败");
    char hex[65];
    bytes_to_hex(digest, 32, hex);
    return px_str(hex);
}

// hmac_sha256(key, msg) → 64 字符小写 hex（HMAC-SHA256）
// M84-S2（Issue 21 GAP-HMAC-1）：key/msg 均收 str|bytes，二进制安全可含 NUL（数值自动
// 字符串化，与 bytes() 语义一致）。腾讯云 TC3 4 级 HMAC 链 / SigV4 / webhook / JWT HS256 解锁。
static void px_hmac_sha256(const unsigned char* key, int klen,
                           const unsigned char* data, int dlen, unsigned char out[32]);
static LXValue bi_hmac_sha256(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("hmac_sha256 需要 (key, msg) 参数");
    const unsigned char* kd = (const unsigned char*)bdata(args[0]);
    int kl = blen(args[0]);
    const unsigned char* md = (const unsigned char*)bdata(args[1]);
    int ml = blen(args[1]);
    unsigned char digest[32];
    px_hmac_sha256(kd, kl, md, ml, digest);
    char hex[65];
    bytes_to_hex(digest, 32, hex);
    return px_str(hex);
}

// dns_lookup(domain) → list[str]（IPv4+IPv6 全部地址，A/AAAA）
// M84-S3（Issue 22 GAP-DNS-1）：getaddrinfo(AF_UNSPEC+SOCK_STREAM) 域名解析原生实现——
// 守护类模块（bs-safeip util.px resolve_ips 每轮解析）不再依赖 getent 外部命令代偿。
// 返回全部 A+AAAA 地址（顺序即 getaddrinfo 返回序）；解析失败
// （NXDOMAIN/超时/无地址记录）返回 Err("dns: <host>: <原因>")，调用方可 is_err()/`?` 判定。
static LXValue bi_dns_lookup(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("dns_lookup 需要一个参数 (domain)");
    const char* host = val_cstr(args[0]);
    char msg[320];
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // A + AAAA
    hints.ai_socktype = SOCK_STREAM;  // 与 hconnect 同口径，避免重复返回
    int rc = getaddrinfo(host, NULL, &hints, &res);
    if (rc != 0) {
        snprintf(msg, sizeof(msg), "dns: %s: %s", host, gai_strerror(rc));
        return px_err(px_str(msg));
    }
    LXValue list = px_list(0);
    px_root_push();
    PX_KEEP(list);   // M92 precise：累积 list 跨 px_list_push/px_str 分配
    char ip[INET6_ADDRSTRLEN];
    for (struct addrinfo* p = res; p; p = p->ai_next) {
        const void* src = NULL;
        if (p->ai_family == AF_INET)
            src = &((struct sockaddr_in*)p->ai_addr)->sin_addr;
        else if (p->ai_family == AF_INET6)
            src = &((struct sockaddr_in6*)p->ai_addr)->sin6_addr;
        if (src && inet_ntop(p->ai_family, src, ip, sizeof(ip)))
            px_list_push(list, px_str(ip));
    }
    freeaddrinfo(res);
    if (list.as.obj->as.list.len == 0) {
        snprintf(msg, sizeof(msg), "dns: %s: no address records", host);
        px_root_pop();
        return px_err(px_str(msg));
    }
    px_root_pop();
    return list;
}

// ---- M103-S2a（Issue 29 GAP-DNS-TXT-1）：DNS TXT 查询（TYPE=16，手写 UDP wire）----
// ws-ddns server 端授权 TXT 校验 native：getaddrinfo 只 A/AAAA（结构上不可能返回 TXT），
// libc res_query 在 musl（交叉 aarch64/armv7/riscv64）仅 A/AAAA 桩、mingw 无 → 手写
// DNS 报文经系统 resolv.conf nameserver UDP 查询，跨 glibc/musl/mingw 零依赖。
// 返回全部 TXT 记录字符串 list（多段 character-string 合并，对齐 Go net.LookupTXT）；
// 无 TXT 记录/NXDOMAIN → 空 list（非报错：授权 TXT 可能未配置，调用方按无记录处理）；
// 查询失败（无 nameserver/超时/TC 截断/格式错）→ Err("dns: <host>: <原因>") 可 is_err 判定。
static uint16_t dns_txt_next_id(void) {
    static unsigned int c = 0x51f15e;
    c = c * 1103515245u + 12345u;
    return (uint16_t)((c >> 8) ^ (unsigned int)getpid() ^ (unsigned int)time(NULL));
}
// 跳过 DNS name（label 序列或压缩指针），返回新 offset；非法返回 -1
static int dns_skip_name(const unsigned char* b, int blen, int off) {
    while (off < blen) {
        int l = b[off];
        if ((l & 0xC0) == 0xC0) return off + 2;   // 压缩指针：2 字节结束
        if (l == 0) return off + 1;               // root 结束
        if (l > 63) return -1;
        off += 1 + l;
    }
    return -1;
}
static LXValue bi_dns_txt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("dns_txt 需要一个参数 (domain)");
    const char* host = val_cstr(args[0]);
    char msg[320];
    if (host[0] == '\0') return px_err(px_str("dns: empty domain"));
    // 1) resolv.conf 首个 nameserver
    char ns[64] = "";
    FILE* rf = fopen("/etc/resolv.conf", "r");
    if (rf) {
        char line[256];
        while (fgets(line, (int)sizeof(line), rf)) {
            char kw[16] = "", ip[64] = "";
            if (sscanf(line, " %15s %63s", kw, ip) == 2 && strcmp(kw, "nameserver") == 0) {
                snprintf(ns, sizeof(ns), "%s", ip);
                break;
            }
        }
        fclose(rf);
    }
    if (ns[0] == '\0') {
        snprintf(msg, sizeof(msg), "dns: %s: no nameserver in /etc/resolv.conf", host);
        return px_err(px_str(msg));
    }
    // 2) 编码 query（header + question QTYPE=TXT(16) QCLASS=IN(1)）
    unsigned char q[512];
    uint16_t id = dns_txt_next_id();
    q[0] = (unsigned char)(id >> 8); q[1] = (unsigned char)(id & 0xFF);
    q[2] = 0x01; q[3] = 0x00;                    // RD
    q[4] = 0; q[5] = 1;                          // QDCOUNT=1
    memset(q + 6, 0, 6);                         // AN/NS/AR=0
    int ql = 12;
    const char* p = host;
    while (*p) {
        const char* dot = strchr(p, '.');
        int l = dot ? (int)(dot - p) : (int)strlen(p);
        if (l <= 0 || l > 63) {
            snprintf(msg, sizeof(msg), "dns: %s: invalid domain label", host);
            return px_err(px_str(msg));
        }
        if (ql + 1 + l + 5 > (int)sizeof(q)) {
            snprintf(msg, sizeof(msg), "dns: %s: domain too long", host);
            return px_err(px_str(msg));
        }
        q[ql++] = (unsigned char)l;
        memcpy(q + ql, p, (size_t)l); ql += l;
        p = dot ? dot + 1 : p + l;
        if (dot && *(dot + 1) == '\0') break;    // 尾点：root 段由下面统一补
    }
    q[ql++] = 0;                                 // root
    q[ql++] = 0; q[ql++] = 16;                   // QTYPE TXT
    q[ql++] = 0; q[ql++] = 1;                    // QCLASS IN
    // 3) UDP 发送（nameserver:53，SO_RCVTIMEO 3s）
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return px_err(px_str("dns: socket() failed"));
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(53);
    if (inet_pton(AF_INET, ns, &sa.sin_addr) != 1) {
        close(fd);
        snprintf(msg, sizeof(msg), "dns: %s: bad nameserver %s", host, ns);
        return px_err(px_str(msg));
    }
    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) != 0) {
        close(fd);
        return px_err(px_str("dns: connect() failed"));
    }
    struct timeval tv;
    tv.tv_sec = 3; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (send(fd, q, ql, 0) != ql) { close(fd); return px_err(px_str("dns: send() failed")); }
    unsigned char rb[4096];
    int rl = (int)recv(fd, rb, sizeof(rb), 0);
    close(fd);
    if (rl < 0) {
        snprintf(msg, sizeof(msg), "dns: %s: query timeout", host);
        return px_err(px_str(msg));
    }
    // 4) 解析响应
    if (rl < 12) return px_err(px_str("dns: short response"));
    uint16_t rid = (uint16_t)((rb[0] << 8) | rb[1]);
    if (rid != id) return px_err(px_str("dns: response id mismatch"));
    uint16_t flags = (uint16_t)((rb[2] << 8) | rb[3]);
    if ((flags & 0x8000) == 0) return px_err(px_str("dns: not a response"));
    int rcode = flags & 0x000F;
    if (rcode == 3) return px_list(0);           // NXDOMAIN → 无记录 → 空 list（非报错）
    if (rcode != 0) {
        snprintf(msg, sizeof(msg), "dns: %s: rcode=%d", host, rcode);
        return px_err(px_str(msg));
    }
    if (flags & 0x0200) {
        snprintf(msg, sizeof(msg), "dns: %s: response truncated (TCP 回退二期)", host);
        return px_err(px_str(msg));
    }
    int qd = (rb[4] << 8) | rb[5];
    int an = (rb[6] << 8) | rb[7];
    if (qd < 1) return px_err(px_str("dns: no question in response"));
    int off = 12;
    for (int i = 0; i < qd; i++) {
        off = dns_skip_name(rb, rl, off);
        if (off < 0 || off + 4 > rl) return px_err(px_str("dns: bad question"));
        off += 4;
    }
    char* txts[128];
    int nt = 0;
    int bad = 0;
    while (an-- > 0) {
        off = dns_skip_name(rb, rl, off);
        if (off < 0 || off + 10 > rl) { bad = 1; break; }
        int rtype = (rb[off] << 8) | rb[off + 1];
        int rclass = (rb[off + 2] << 8) | rb[off + 3];
        int rdlen = (rb[off + 8] << 8) | rb[off + 9];   // TYPE2+CLASS2+TTL4 后 RDLENGTH2
        off += 10;
        if (off + rdlen > rl) { bad = 1; break; }
        if (rtype == 16 && rclass == 1) {
            char tmp[2048];
            int tl = 0;
            int pos = off;
            int end = off + rdlen;
            while (pos < end && tl < (int)sizeof(tmp) - 1) {
                int sl = rb[pos++];
                if (pos + sl > end) break;
                if (tl + sl > (int)sizeof(tmp) - 1) { tl = (int)sizeof(tmp) - 1; break; }
                memcpy(tmp + tl, rb + pos, (size_t)sl); tl += sl; pos += sl;
            }
            tmp[tl] = '\0';
            if (tl > 0 && nt < 128) {
                txts[nt] = (char*)malloc((size_t)tl + 1);
                memcpy(txts[nt], tmp, (size_t)tl + 1);
                nt++;
            }
        }
        off += rdlen;
    }
    LXValue list = px_list(0);
    px_root_push();
    PX_KEEP(list);
    if (!bad) {
        for (int i = 0; i < nt; i++) px_list_push(list, px_str(txts[i]));
    }
    px_root_pop();
    for (int i = 0; i < nt; i++) free(txts[i]);
    if (bad) return px_err(px_str("dns: malformed response"));
    return list;
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
    px_root_push();
    PX_KEEP(d);    // M92 precise：结果 dict 跨 px_dict_set/px_list_push/px_str_len 分配
    px_dict_set(d, "match", px_str_len(text + s, e - s));
    px_dict_set(d, "start", px_int(s));
    px_dict_set(d, "end", px_int(e));
    LXValue gl = px_list(0);
    PX_KEEP(gl);   // M92 precise：groups list 跨 px_list_push/px_str_len 分配（入 d 前仅局部）
    for (int i = 1; i < RG_N; i++) {
        if (g[i] != -1) {
            int gs = (int)(g[i] >> 32), ge = (int)(g[i] & 0xffffffff);
            px_list_push(gl, px_str_len(text + gs, ge - gs));
        } else {
            px_list_push(gl, px_null());
        }
    }
    px_dict_set(d, "groups", gl);
    px_root_pop();
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
    px_root_push();
    PX_KEEP(r);   // M92 precise：累积 list 跨 px_list_push/px_str_len 分配
    int pos = 0;
    while (pos <= tlen) {
        int s, e;
        int64_t g[RG_N];
        if (!rsearch_from(root, (const unsigned char*)text, tlen, pos, &s, &e, g)) break;
        px_list_push(r, px_str_len(text + s, e - s));
        pos = (e == s) ? s + 1 : e;
    }
    rp_free(root);
    px_root_pop();
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
    px_root_push();
    PX_KEEP(r);   // M92 precise：r_split 内部 px_list_push 分配
    r_split(root, (const unsigned char*)text, tlen, r);
    rp_free(root);
    px_root_pop();
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
    px_root_push();
    PX_KEEP(r);   // M92 precise：累积 list 跨 px_list_push/px_str 分配
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
    px_root_pop();
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
        px_root_push();
        PX_KEEP(d);   // M92 precise：递归解析结果 dict 跨 px_dict_set/json_parse_value 分配
        json_ws(j);
        if (*j->p == '}') { j->p++; px_root_pop(); return d; }
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
        px_root_pop();
        return d;
    }
    if (*j->p == '[') {
        j->p++;
        LXValue a = px_list(0);
        px_root_push();
        PX_KEEP(a);   // M92 precise：递归解析结果 list 跨 px_list_push/json_parse_value 分配
        json_ws(j);
        if (*j->p == ']') { j->p++; px_root_pop(); return a; }
        while (1) {
            LXValue v = json_parse_value(j);
            px_list_push(a, v);
            json_ws(j);
            if (*j->p == ',') { j->p++; continue; }
            if (*j->p == ']') { j->p++; break; }
            px_error("json: 数组解析失败");
        }
        px_root_pop();
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
        case PX_RESULT: {
            // M39：Result 序列化为 {"ok":bool,"value":...}（保留 Ok/Err 结构）
            LXObject* ob = v.as.obj;
            jout_append(o, "{\"ok\":");
            jout_append(o, ob->as.result.ok ? "true" : "false");
            jout_append(o, ",\"value\":");
            json_stringify_value(o, ob->as.result.value);
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
            px_root_push();
            PX_KEEP(r);   // M92 precise：深拷贝 list 跨递归 json_value_copy/px_list_push 分配
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.list.len; i++) px_list_push(r, json_value_copy(o->as.list.items[i]));
            px_root_pop();
            return r;
        }
        case PX_TUPLE: {
            LXObject* o = v.as.obj;
            LXValue* items = xmalloc(sizeof(LXValue) * (size_t)(o->as.tuple.len > 0 ? o->as.tuple.len : 1));
            px_root_push();
            for (int i = 0; i < o->as.tuple.len; i++) {
                items[i] = json_value_copy(o->as.tuple.items[i]);
                PX_KEEP(items[i]);   // M92 precise：收集数组仅 C 持有，跨后续递归分配
            }
            LXValue r = px_tuple(items, o->as.tuple.len);
            px_root_pop();
            xfree(items);
            return r;
        }
        case PX_DICT: {
            LXValue r = px_dict();
            px_root_push();
            PX_KEEP(r);   // M92 precise：深拷贝 dict 跨递归 json_value_copy/px_dict_set 分配
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.dict.len; i++) {
                px_dict_set(r, o->as.dict.keys[i], json_value_copy(o->as.dict.vals[i]));
            }
            px_root_pop();
            return r;
        }
        case PX_RESULT: {
            // M39：Result 深拷贝（Ok/Err 标志 + 载荷递归）
            LXObject* o = v.as.obj;
            if (o->as.result.ok) return px_ok(json_value_copy(o->as.result.value));
            return px_err(json_value_copy(o->as.result.value));
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
        // 深拷贝现有元素（M92 precise：items 收集数组 + 结果 list 仅 C 局部持有，
        // 跨 json_value_copy/px_list_push 分配须登记）
        LXValue* items = xmalloc(sizeof(LXValue) * (size_t)(len > 0 ? len : 1));
        px_root_push();
        PX_KEEP(lst);   // base（bi_json_path_set 深拷贝临时，仅 C 持有）跨拷贝分配存活
        for (int i = 0; i < len; i++) {
            items[i] = json_value_copy(o->as.list.items[i]);
            PX_KEEP(items[i]);   // 收集数组仅 C 持有，跨后续递归/分配
        }
        LXValue r = px_list(len);
        PX_KEEP(r);   // 结果 list 跨 px_list_push/递归分配
        for (int i = 0; i < len; i++) px_list_push(r, items[i]);
        xfree(items);
        LXObject* ro = r.as.obj;
        if (real < 0) {
            // 负索引越界 → 插入 0
            LXValue child = json_path_set_at(px_null(), segs + 1, n - 1, new_val);
            // 在 0 处插入：新 list 重建
            LXValue* ni = xmalloc(sizeof(LXValue) * (size_t)(ro->as.list.len + 1));
            ni[0] = child;
            PX_KEEP(child);   // 入 rr 前 child 仅 ni[0] 持有
            for (int i = 0; i < ro->as.list.len; i++) ni[i + 1] = ro->as.list.items[i];
            LXValue rr = px_list(ro->as.list.len + 1);
            PX_KEEP(rr);   // 结果 list 跨 px_list_push 分配
            for (int i = 0; i < ro->as.list.len + 1; i++) px_list_push(rr, ni[i]);
            xfree(ni);
            px_root_pop();
            return rr;
        } else if (real < ro->as.list.len) {
            // 原位设值（child 赋入 r 前无分配，r 已登记）
            LXValue child = json_path_set_at(ro->as.list.items[(int)real], segs + 1, n - 1, new_val);
            ro->as.list.items[(int)real] = child;
            px_root_pop();
            return r;
        } else {
            // 越界扩展：null 填充 + 追加
            while (ro->as.list.len < real) px_list_push(r, px_null());
            LXValue child = json_path_set_at(px_null(), segs + 1, n - 1, new_val);
            PX_KEEP(child);   // 入 r 前仅 C 局部
            px_list_push(r, child);
            px_root_pop();
            return r;
        }
    } else {
        // dict 字段
        LXValue d = (base.type == PX_DICT) ? base : px_dict();
        LXObject* o = d.as.obj;
        LXValue r = px_dict();
        px_root_push();
        PX_KEEP(d);   // base（bi_json_path_set 深拷贝临时，仅 C 持有）跨拷贝分配存活
        PX_KEEP(r);   // 结果 dict 跨 px_dict_set/json_value_copy 分配
        for (int i = 0; i < o->as.dict.len; i++) {
            px_dict_set(r, o->as.dict.keys[i], json_value_copy(o->as.dict.vals[i]));
        }
        LXValue child = px_dict_get(d, segs[0].key);
        LXValue nv = (child.type == PX_NULL)
            ? json_path_set_at(px_null(), segs + 1, n - 1, new_val)
            : json_path_set_at(child, segs + 1, n - 1, new_val);
        PX_KEEP(nv);   // 入 r 前仅 C 局部（递归返回值）
        px_dict_set(r, segs[0].key, nv);
        px_root_pop();
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

// M-B2 修复：编译版保存原始命令行参数（自举 lexer 需要 args() 取输入文件）
static char** g_px_argv = NULL;
static int g_px_argc = 0;

void px_args_init(int argc, char** argv) {
    g_px_argc = argc;
    g_px_argv = argv;
}

static LXValue bi_args(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    LXValue l = px_list(0);
    px_root_push();
    PX_KEEP(l);   // M92 precise：累积 list 跨 px_list_push/px_str 分配
    for (int i = 0; i < g_px_argc; i++) {
        px_list_push(l, px_str(g_px_argv[i]));
    }
    px_root_pop();
    return l;
}

// ---- std.collections（高阶函数） ----
// M92：map/filter/reduce 循环内 px_call 回调用户代码（可大量分配触发 GC）+ px_list_push
// （扩容分配）。precise 模式下 C 栈不扫 → 桥内新建、跨回调/跨分配的局部（结果 list r /
// 累积 acc）须 PX_KEEP 登记（fn/item 为参数/参数子对象，上层根已保护）。

static LXValue bi_map(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_LIST) px_error("map 需要 (list, fn)");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue r = px_list(0);
    px_root_push();
    PX_KEEP(r);          // 累积结果 list：px_list_push 扩容分配/回调期间需存活
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue res = px_call(fn, &item, 1);
        px_list_push(r, res);
    }
    px_root_pop();
    return r;
}

static LXValue bi_filter(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_LIST) px_error("filter 需要 (list, fn)");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue r = px_list(0);
    px_root_push();
    PX_KEEP(r);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue res = px_call(fn, &item, 1);
        if (px_is_truthy(res)) px_list_push(r, item);
    }
    px_root_pop();
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
        px_root_push();
        PX_KEEP(acc);    // 累积值跨 px_call 回调存活（回调内 GC 会回收仅栈持有的对象）
        LXValue nacc = px_call(fn, pair, 2);
        px_root_pop();
        acc = nacc;
    }
    return acc;
}

void px_register_builtins(void) {
    px_set_global("print", px_native("print", bi_print));
    px_set_global("flush", px_native("flush", bi_flush));
    px_set_global("print_err", px_native("print_err", bi_print_err));
    // M39：Result/Option 构造函数
    px_set_global("Ok", px_native("Ok", bi_ok));
    px_set_global("Err", px_native("Err", bi_err));
    px_set_global("Some", px_native("Some", bi_some));
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
    // M59-S1 三角（弧度）+ pi 常量
    px_set_global("sin", px_native("sin", bi_sin));
    px_set_global("cos", px_native("cos", bi_cos));
    px_set_global("tan", px_native("tan", bi_tan));
    px_set_global("atan2", px_native("atan2", bi_atan2));
    px_set_global("pi", px_float(PX_PI));
    // M59-S2 取整/舍入 + 对数/指数 + e 常量
    px_set_global("floor", px_native("floor", bi_floor));
    px_set_global("ceil", px_native("ceil", bi_ceil));
    px_set_global("round", px_native("round", bi_round));
    px_set_global("log", px_native("log", bi_log));
    px_set_global("log10", px_native("log10", bi_log10));
    px_set_global("exp", px_native("exp", bi_exp));
    px_set_global("e", px_float(PX_E));
    // M59-S3 随机（splitmix64：random()→[0,1) / random_int(n)→[0,n) / random_seed(s) 设种子可复现）
    px_set_global("random", px_native("random", bi_random));
    px_set_global("random_int", px_native("random_int", bi_random_int));
    px_set_global("random_seed", px_native("random_seed", bi_random_seed));
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
    // M57-S1：边缘设备层 fd 原语（open/close/ioctl/os_errno）
    px_set_global("open", px_native("open", bi_open));
    px_set_global("close", px_native("close", bi_close));
    px_set_global("ioctl", px_native("ioctl", bi_ioctl));
    px_set_global("os_errno", px_native("os_errno", bi_os_errno));
    // M57-S2：边缘设备层 fd 数据通道 + mmap 设备映射（read/write/mmap/munmap/mem_write）
    px_set_global("read", px_native("read", bi_read));
    px_set_global("write", px_native("write", bi_write));
    px_set_global("mmap", px_native("mmap", bi_mmap));
    px_set_global("munmap", px_native("munmap", bi_munmap));
    px_set_global("mem_write", px_native("mem_write", bi_mem_write));
    // M60-S1：us 级时钟 + fd 控制（sleep_us/now_us/fcntl）
    px_set_global("sleep_us", px_native("sleep_us", bi_sleep_us));
    px_set_global("now_us", px_native("now_us", bi_now_us));
    px_set_global("fcntl", px_native("fcntl", bi_fcntl));
    // M60-S2：设备组（tty_config/fd_wait）
    px_set_global("tty_config", px_native("tty_config", bi_tty_config));
    px_set_global("fd_wait", px_native("fd_wait", bi_fd_wait));
    // M14 P1：crypto 哈希
    px_set_global("sha256", px_native("sha256", bi_sha256));
    px_set_global("hmac_sha256", px_native("hmac_sha256", bi_hmac_sha256));  // M84-S2 (Issue 21 GAP-HMAC-1)
    px_set_global("dns_lookup", px_native("dns_lookup", bi_dns_lookup));     // M84-S3 (Issue 22 GAP-DNS-1)
    px_set_global("dns_txt", px_native("dns_txt", bi_dns_txt));              // M103-S2a (Issue 29 GAP-DNS-TXT-1)
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
    // M33：UDP 基础设施（HTTP/3/QUIC 预研）
    px_set_global("udp_open", px_native("udp_open", bi_udp_open));
    px_set_global("udp_send", px_native("udp_send", bi_udp_send));
    px_set_global("udp_recv", px_native("udp_recv", bi_udp_recv));
    px_set_global("udp_close", px_native("udp_close", bi_udp_close));
    // M34：事件总线（pub/sub，跨线程）
    px_set_global("event_bus", px_native("event_bus", bi_event_bus));
    px_set_global("bus_subscribe", px_native("bus_subscribe", bi_bus_subscribe));
    px_set_global("bus_publish", px_native("bus_publish", bi_bus_publish));
    px_set_global("bus_unsubscribe", px_native("bus_unsubscribe", bi_bus_unsubscribe));
    // M36：请求上下文（线程局部）
    px_set_global("ctx_set", px_native("ctx_set", bi_ctx_set));
    px_set_global("ctx_get", px_native("ctx_get", bi_ctx_get));
    px_set_global("ctx_clear", px_native("ctx_clear", bi_ctx_clear));
    // M37：S3/MinIO
    px_set_global("s3_put", px_native("s3_put", bi_s3_put));
    px_set_global("s3_get", px_native("s3_get", bi_s3_get));
    px_set_global("s3_delete", px_native("s3_delete", bi_s3_delete));
    px_set_global("s3_list", px_native("s3_list", bi_s3_list));
    // M38：UDP echo 服务端
    px_set_global("udp_serve", px_native("udp_serve", bi_udp_serve));
    px_set_global("http_get", px_native("http_get", bi_http_get));
    px_set_global("http_post", px_native("http_post", bi_http_post));
    px_set_global("http_serve", px_native("http_serve", bi_http_serve));
    // M8x：Issue 15 GAP-SRV-1 —— unix socket HTTP 服务端（与 http_serve 同族，AF_UNIX 服务端维度）
    px_set_global("http_serve_unix", px_native("http_serve_unix", bi_http_serve_unix));
    // M23c P1：HTTP 生产化（http_request 连接池 / http_get_stream 流式下载）
    px_set_global("http_request", px_native("http_request", bi_http_request));
    px_set_global("http_get_stream", px_native("http_get_stream", bi_http_get_stream));
    // M56：Unix domain socket HTTP 客户端（本地服务/LLM 网关/容器 daemon 等）
    px_set_global("http_unix", px_native("http_unix", bi_http_unix));
    // M17 .px 脚本执行机制
    px_set_global("px_exec", px_native("px_exec", bi_px_exec));
    px_set_global("px_serve", px_native("px_serve", bi_px_serve));
    // M31 安全/多租户/防爆破：沙箱 / 虚拟主机 / 限流
    px_set_global("sandbox_enter", px_native("sandbox_enter", bi_sandbox_enter));
    px_set_global("vhost", px_native("vhost", bi_vhost));
    px_set_global("rate_limit", px_native("rate_limit", bi_rate_limit));
    // M32 生成器表达式：gen_next 逐项取值 / list 转列表
    px_set_global("gen_next", px_native("gen_next", bi_gen_next));
    px_set_global("list", px_native("list", bi_list));
    px_set_global("tuple", px_native("tuple", bi_tuple));
    // M18 后台定时任务 / 定时器原语
    px_set_global("set_timeout", px_native("set_timeout", bi_set_timeout));
    px_set_global("set_interval", px_native("set_interval", bi_set_interval));
    px_set_global("clear_timer", px_native("clear_timer", bi_clear_timer));
// M85-S1：--no-aes 裁剪（去 runtime_aes.o + mbedtls aes/gcm 引用面）
#ifndef PX_NO_AES
    // M19 P1：AES 加密（企微回调加解密 / 数据落盘加密 / Cookie 签名）
    px_set_global("aes_encrypt", px_native("aes_encrypt", bi_aes_encrypt));
    px_set_global("aes_decrypt", px_native("aes_decrypt", bi_aes_decrypt));
    px_set_global("aes_gcm_encrypt", px_native("aes_gcm_encrypt", bi_aes_gcm_encrypt));
    px_set_global("aes_gcm_decrypt", px_native("aes_gcm_decrypt", bi_aes_gcm_decrypt));
    // M72-S4（Issue 13 GAP-BIN-1）：AES bytes 版（任意二进制，含 \0/非 UTF-8；
    // GCM 输出 密文||tag 原始 bytes，与 Go crypto/aes-gcm 字节兼容）
    px_set_global("aes_encrypt_bytes", px_native("aes_encrypt_bytes", bi_aes_encrypt_bytes));
    px_set_global("aes_decrypt_bytes", px_native("aes_decrypt_bytes", bi_aes_decrypt_bytes));
    px_set_global("aes_gcm_encrypt_bytes", px_native("aes_gcm_encrypt_bytes", bi_aes_gcm_encrypt_bytes));
    px_set_global("aes_gcm_decrypt_bytes", px_native("aes_gcm_decrypt_bytes", bi_aes_gcm_decrypt_bytes));
    // M83-S2（Issue 20 GAP-AES-1）：AES-ECB（PKCS7，无 IV）——微信网关媒体 AES-128-ECB 全链路
    px_set_global("aes_encrypt_ecb", px_native("aes_encrypt_ecb", bi_aes_encrypt_ecb));
    px_set_global("aes_decrypt_ecb", px_native("aes_decrypt_ecb", bi_aes_decrypt_ecb));
    px_set_global("aes_encrypt_ecb_bytes", px_native("aes_encrypt_ecb_bytes", bi_aes_encrypt_ecb_bytes));
    px_set_global("aes_decrypt_ecb_bytes", px_native("aes_decrypt_ecb_bytes", bi_aes_decrypt_ecb_bytes));
#endif // PX_NO_AES
// M85-S1：--no-xml 裁剪（去 runtime_xml.o）
#ifndef PX_NO_XML
    // M19 P1：XML 解析（企微回调 Encrypt 报文 / 配置文件 / 文档）
    px_set_global("xml_parse", px_native("xml_parse", bi_xml_parse));
    px_set_global("xml_escape", px_native("xml_escape", bi_xml_escape));
    px_set_global("xml_unescape", px_native("xml_unescape", bi_xml_unescape));
    px_set_global("xml_build", px_native("xml_build", bi_xml_build));
#endif // PX_NO_XML
// M85-S1：--no-zip 裁剪（去 runtime_zip.o + 其 mbedtls md/pkcs5/aes 引用面）
#ifndef PX_NO_ZIP
    // M19 P1：zip 打包/解压（docx/xlsx/pptx 是 zip+xml，文档工具基石）
    px_set_global("zip_pack", px_native("zip_pack", bi_zip_pack));
    px_set_global("zip_unpack", px_native("zip_unpack", bi_zip_unpack));
#endif // PX_NO_ZIP
    // M83-S2（Issue 20 GAP-ARC-1）：gzip 语言层通用压缩/解压（wsa-heal tar.gz / gen-update 差分包）
    px_set_global("gzip_compress", px_native("gzip_compress", bi_gzip_compress));
    px_set_global("gzip_uncompress", px_native("gzip_uncompress", bi_gzip_uncompress));
    // M21 P1：base64 编解码
    px_set_global("base64_encode", px_native("base64_encode", bi_base64_encode));
    px_set_global("base64_decode", px_native("base64_decode", bi_base64_decode));
    // M21 P1：SSE 服务端（LLM 流式推送 / 实时通知）
    px_set_global("sse_serve", px_native("sse_serve", bi_sse_serve));
    px_set_global("sse_send", px_native("sse_send", bi_sse_send));
    px_set_global("sse_close", px_native("sse_close", bi_sse_close));
    // M83-S6（Issue 19）：http_serve/http_serve_unix 同端口流式路由注册
    px_set_global("http_stream", px_native("http_stream", bi_http_stream));
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
// M85-S1：--no-ws 裁剪（去 runtime_ws.o；ws_serve/ws_connect 等 WS native 缺 → R1001）
#ifndef PX_NO_WS
    // M22 P1：WebSocket（RFC 6455，微信/QQ/飞书长连接 / LLM 流式 / 实时推送）
    px_set_global("ws_serve", px_native("ws_serve", bi_ws_serve));
    px_set_global("ws_connect", px_native("ws_connect", bi_ws_connect));
    // M38：WS 客户端自动重连
    px_set_global("ws_connect_auto", px_native("ws_connect_auto", bi_ws_connect_auto));
    px_set_global("ws_send", px_native("ws_send", bi_ws_send));
    // M34：WebSocket 服务端广播（群发）
    px_set_global("ws_broadcast", px_native("ws_broadcast", bi_ws_broadcast));
    px_set_global("ws_recv", px_native("ws_recv", bi_ws_recv));
    px_set_global("ws_close", px_native("ws_close", bi_ws_close));
    px_set_global("ws_ping", px_native("ws_ping", bi_ws_ping));
    px_set_global("ws_heartbeat", px_native("ws_heartbeat", bi_ws_heartbeat));
#endif // PX_NO_WS
    // M27 P0：WebServer 生产化四件套（服务端 TLS / Session / 基础认证）
    px_set_global("tls_server", px_native("tls_server", bi_tls_server));
    px_set_global("session_open", px_native("session_open", bi_session_open));
    px_set_global("session_id", px_native("session_id", bi_session_id));
    px_set_global("session_get", px_native("session_get", bi_session_get));
    px_set_global("session_set", px_native("session_set", bi_session_set));
    px_set_global("session_del", px_native("session_del", bi_session_del));
    px_set_global("session_destroy", px_native("session_destroy", bi_session_destroy));
    px_set_global("basic_auth", px_native("basic_auth", bi_basic_auth));
// M85-S1：--no-route 裁剪（去 runtime_route.o；route/middleware native 缺 → R1001）
#ifndef PX_NO_ROUTE
    // M28 P1：路由表 + 中间件（runtime_route.c）
    px_set_global("route", px_native("route", bi_route));
    px_set_global("middleware", px_native("middleware", bi_middleware));
#endif // PX_NO_ROUTE
// M85-S1：--no-sqlite 裁剪（去 runtime_sqlite.o + sqlite3.o；sqlite_* native 缺 → R1001）
#ifndef PX_NO_SQLITE
    // M28 P1：SQLite 绑定（runtime_sqlite.c）
    px_set_global("sqlite_open", px_native("sqlite_open", bi_sqlite_open));
    px_set_global("sqlite_exec", px_native("sqlite_exec", bi_sqlite_exec));
    px_set_global("sqlite_query", px_native("sqlite_query", bi_sqlite_query));
    px_set_global("sqlite_close", px_native("sqlite_close", bi_sqlite_close));
    px_set_global("sqlite_escape", px_native("sqlite_escape", bi_sqlite_escape));
    px_set_global("sqlite_last_insert_rowid", px_native("sqlite_last_insert_rowid", bi_sqlite_last_insert_rowid));
#endif // PX_NO_SQLITE
    // M42：FFI C 桥（runtime_ffi.c）—— ffi_call(name, args_list)
    px_set_global("ffi_call", px_native("ffi_call", bi_ffi_call));
// M85-S1：--no-sqlite 裁剪（去 runtime_sqlite.o + sqlite3.o；sqlite_* native 缺 → R1001）
#ifndef PX_NO_SQLITE
    // M42：已链 C 库绑定进 FFI 注册表（语言层 extern def 按名字查找）
    px_ffi_register("sqlite_open", bi_sqlite_open);
    px_ffi_register("sqlite_exec", bi_sqlite_exec);
    px_ffi_register("sqlite_query", bi_sqlite_query);
    px_ffi_register("sqlite_close", bi_sqlite_close);
    px_ffi_register("sqlite_escape", bi_sqlite_escape);
    px_ffi_register("sqlite_last_insert_rowid", bi_sqlite_last_insert_rowid);
#endif // PX_NO_SQLITE
    // M48：hex 纯函数进 FFI 表（capability 字节精确断言/双模式一致用）
    px_ffi_register("bytes_to_hex", bi_bytes_to_hex);
    px_ffi_register("hex_to_bytes", bi_hex_to_bytes);
// M85-S1：--no-zlib 裁剪（去 runtime_zlib.o + libz.a 链；extern zlib_* 缺 → R1001）
#ifndef PX_NO_ZLIB
    // M61-S1：zlib 外部系统库绑定（runtime_zlib.c；libz.a 恒链，无条件注册）
    px_register_zlib();
#endif // PX_NO_ZLIB
#ifndef PX_NO_QUIC
    // M46：QUIC 传输级绑定（runtime_quic.c）—— 语言层 extern def quic_* 按名字查找
    px_register_quic();
    // M47：HTTP/3 语义层（runtime_h3.c）—— QPACK 编解码 + H3 帧 + 请求/响应对拍
    px_register_h3();
    // M49：QPACK 动态表 + SETTINGS 帧（runtime_h3_qpack_dyn.c）—— 连接级 QPACK 会话
    px_register_h3_qpack_dyn();
#endif
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
    px_set_global("os_exec", px_native("os_exec", bi_os_exec));
    px_set_global("os_rename", px_native("os_rename", bi_os_rename));
    px_set_global("os_remove_all", px_native("os_remove_all", bi_os_remove_all));
    px_set_global("os_random_hex", px_native("os_random_hex", bi_os_random_hex));
    px_set_global("os_file_sha256", px_native("os_file_sha256", bi_os_file_sha256));
    px_set_global("os_spawn_capture", px_native("os_spawn_capture", bi_os_spawn_capture));
    px_set_global("os_wait", px_native("os_wait", bi_os_wait));
    px_set_global("os_kill", px_native("os_kill", bi_os_kill));
    // M66：qg-issue 合入（unix_connect/os_capture/os_popen）
    px_set_global("unix_connect", px_native("unix_connect", bi_unix_connect));
    px_set_global("os_capture", px_native("os_capture", bi_os_capture));
    px_set_global("os_popen", px_native("os_popen", bi_os_popen));

    px_set_global("signal", px_native("signal", bi_signal));
// M85-S1：--no-rsa 裁剪（去 runtime_rsa.o + mbedtls rsa/pk 引用面；rsa_* native 缺 → R1001）
#ifndef PX_NO_RSA
    // M23d P1：RSA（PKCS#1 v1.5，密钥/密文/签名均 hex；实现 runtime_rsa.c）
    px_set_global("rsa_gen_key", px_native("rsa_gen_key", bi_rsa_gen_key));
    px_set_global("rsa_encrypt", px_native("rsa_encrypt", bi_rsa_encrypt));
    px_set_global("rsa_decrypt", px_native("rsa_decrypt", bi_rsa_decrypt));
    px_set_global("rsa_sign", px_native("rsa_sign", bi_rsa_sign));
    px_set_global("rsa_verify", px_native("rsa_verify", bi_rsa_verify));
#endif // PX_NO_RSA
// M85-S1：--no-ed25519 裁剪（去 runtime_ed25519.o + tweetnacl.o；ed25519_* native 缺 → R1001）
#ifndef PX_NO_ED25519
    // M83-S3（Issue 17 GAP-ED25519-1）：ed25519（RFC8032，tweetnacl；实现 runtime_ed25519.c）
    //   —— api-server /v1/family 节点互信 + ws-ddns 双向签名（与 Go crypto/ed25519 互通）
    px_set_global("ed25519_sign", px_native("ed25519_sign", bi_ed25519_sign));
    px_set_global("ed25519_verify", px_native("ed25519_verify", bi_ed25519_verify));
    px_set_global("ed25519_keygen", px_native("ed25519_keygen", bi_ed25519_keygen));  // M103-S2b (Issue 29 GAP-ED25519-2)
#endif // PX_NO_ED25519
// M103-S2d：--no-img 裁剪（去 runtime_image.o + third_party/stb；img_* native 缺 → R1001）
#ifndef PX_NO_IMG
    // M103-S2d（Issue 30 GAP-IMG）：图片解码/等比缩放/JPEG 编码（stb_image v2.30 +
    //   stb_image_write v1.16，public domain；实现 runtime_image.c）
    //   —— api-server avatar.go「解码→>512 等比缩放→JPEG q70」头像接口 native 解锁
    px_set_global("img_decode", px_native("img_decode", bi_img_decode));
    px_set_global("img_scale", px_native("img_scale", bi_img_scale));
    px_set_global("img_encode_jpeg", px_native("img_encode_jpeg", bi_img_encode_jpeg));
#endif // PX_NO_IMG
// M85-S1：--no-rsa 裁剪（去 runtime_rsa.o + mbedtls rsa/pk 引用面；rsa_* native 缺 → R1001）
#ifndef PX_NO_RSA
    // M83-S4（Issue 18 GAP-RSA-1）：RSA PKCS1v15-SHA256 标准签名（PEM 入参，DigestInfo 自动封装）
    //   —— ws-pay 商户签名 + agentmail DKIM rsa-sha256（与 Go rsa.SignPKCS1v15/VerifyPKCS1v15 互通）
    px_set_global("rsa_sign_pkcs1v15_sha256", px_native("rsa_sign_pkcs1v15_sha256", bi_rsa_sign_pkcs1v15_sha256));
    px_set_global("rsa_verify_pkcs1v15_sha256", px_native("rsa_verify_pkcs1v15_sha256", bi_rsa_verify_pkcs1v15_sha256));
#endif // PX_NO_RSA
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
    // M83-S2（Issue 20 GAP-PGID-1）：第 3 参可选 group:bool（默认 false 保持现状）→
    //   fork 后子进程 setpgid(0,0) 自成进程组（supervisor 停服需 kill(-pgid) 连孙进程一起清）
    int group = 0;
    if (nargs == 3) {
        if (args[2].type == PX_BOOL) group = args[2].as.b ? 1 : 0;
        else if (args[2].type == PX_INT) group = args[2].as.i != 0;
        else px_error("os_spawn 的 group 需要 bool");
    } else if (nargs != 2) {
        px_error("os_spawn 需要 (cmd, args[, group]) 参数");
    }
    if (args[0].type != PX_STR || args[1].type != PX_LIST)
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
        // 子进程：setpgid（可选）后 execvp（argv[0]=cmd）
        if (group) setpgid(0, 0);
        execvp(cmd, argv);
        _exit(127);
    }
    for (int i = 0; i <= argc; i++) free(argv[i]);
    free(argv);
    return px_int((int64_t)pid);
}

// os_spawn_capture(cmd, args) → [rc:int, output:str] | null（M65：LSP/MCP 子进程捕获）
// fork+execvp，子进程 stdout+stderr 合并到同一管道（2>&1 语义），父进程读尽后
// waitpid 回收。rc：正常=exit code；信号终止=128+sig；exec 失败=127；wait 失败=-1(null)。
// 用途：MCP run/test/bench、LSP 诊断器（pxcheck）子进程输出捕获 —— 语言内编排真自举。
// 注意：输出为整串 str（二进制安全按字节长度截断）；大输出全量驻留内存（演示级上限自持）。
static LXValue bi_os_spawn_capture(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_LIST)
        px_error("os_spawn_capture 需要 (cmd, args) 参数");
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
            px_error("os_spawn_capture 的 args 必须是字符串列表");
        }
        argv[i + 1] = strdup(v.as.obj->as.str.data);
    }
    argv[argc + 1] = NULL;
    int fds[2];
    if (pipe(fds) != 0) {
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], 1);
        dup2(fds[1], 2);
        close(fds[1]);
        execvp(cmd, argv);
        _exit(127);
    }
    close(fds[1]);
    for (int i = 0; i <= argc; i++) free(argv[i]);
    free(argv);
    size_t cap = 8192, n = 0;
    char* buf = (char*)xmalloc(cap);
    for (;;) {
        if (n + 4096 > cap) {
            cap *= 2;
            buf = (char*)xrealloc(buf, cap);
        }
        ssize_t r = read(fds[0], buf + n, cap - n);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (r == 0) break;
        n += (size_t)r;
    }
    close(fds[0]);
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        xfree(buf);
        return px_null();
    }
    int rc = -1;
    if (WIFEXITED(status)) rc = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) rc = 128 + (int)WTERMSIG(status);
    LXValue res[2];
    res[0] = px_int((int64_t)rc);
    res[1] = px_str_len(buf, (int)n);
    xfree(buf);
    return px_list_n(res, 2);
}

// os_exec(cmd, args?) → 进程替换（execvp）：成功不返回，当前进程被目标程序替换；
//                        失败（命令不存在/无权限等）px_error 报错退出。
// 与 os_spawn（fork+execvp 返回 pid）互补：用于 launcher/daemon 场景的语义级 exec。
static LXValue bi_os_exec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2 || args[0].type != PX_STR)
        px_error("os_exec 需要 (cmd, args?) 参数，cmd 为字符串");
    const char* cmd = args[0].as.obj->as.str.data;
    int argc = 0;
    char** argv;
    if (nargs == 2) {
        if (args[1].type != PX_LIST) px_error("os_exec 的 args 必须是字符串列表");
        LXObject* list = args[1].as.obj;
        argc = list->as.list.len;
        argv = (char**)calloc((size_t)argc + 2, sizeof(char*));
        argv[0] = strdup(cmd);
        for (int i = 0; i < argc; i++) {
            LXValue v = list->as.list.items[i];
            if (v.type != PX_STR) {
                for (int j = 0; j <= i; j++) free(argv[j]);
                free(argv);
                px_error("os_exec 的 args 必须是字符串列表");
            }
            argv[i + 1] = strdup(v.as.obj->as.str.data);
        }
        argv[argc + 1] = NULL;
    } else {
        argv = (char**)calloc(2, sizeof(char*));
        argv[0] = strdup(cmd);
        argv[1] = NULL;
    }
    // execvp 成功后当前进程被替换，不会返回
    execvp(cmd, argv);
    // 走到这里说明 exec 失败
    int e = errno;
    for (int j = 0; j <= argc; j++) free(argv[j]);
    free(argv);
    px_error("os_exec 执行 %s 失败: %s", cmd, strerror(e));
    return px_null(); /* unreachable */
}

// os_random_hex(n) → 2n 字符小写 hex 字符串：从 /dev/urandom 读 n 字节随机数转 hex。
// 失败返回 null。用途：生成 agent_id / token（对齐 Go crypto/rand）。
static LXValue bi_os_random_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT)
        px_error("os_random_hex 需要 (nbytes) 参数");
    int64_t n64 = args[0].as.i;
    if (n64 <= 0 || n64 > 1024)
        px_error("os_random_hex nbytes 需在 1..1024");
    int n = (int)n64;
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return px_null();
    unsigned char* buf = (unsigned char*)malloc((size_t)n);
    if (!buf) { fclose(f); return px_null(); }
    size_t got = fread(buf, 1, (size_t)n, f);
    fclose(f);
    if (got != (size_t)n) { free(buf); return px_null(); }
    char* out = (char*)malloc((size_t)n * 2 + 1);
    if (!out) { free(buf); return px_null(); }
    for (int i = 0; i < n; i++) {
        out[i*2]   = "0123456789abcdef"[buf[i] >> 4];
        out[i*2+1] = "0123456789abcdef"[buf[i] & 15];
    }
    out[n*2] = 0;
    free(buf);
    LXValue v = px_str(out);
    free(out);
    return v;
}

// os_file_sha256(path) → 64 字符小写 hex 字符串：对文件内容计算 sha256（mbedtls），
// 失败（不存在/不可读/读不全）返回 null。用途：模块包内容级校验（对齐 Go fileSHA256）。
static LXValue bi_os_file_sha256(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR)
        px_error("os_file_sha256 需要 (path) 参数");
    const char* path = args[0].as.obj->as.str.data;
    FILE* f = fopen(path, "rb");
    if (!f) return px_null();
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return px_null(); }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return px_null(); }
    rewind(f);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz > 0 ? (size_t)sz : 1);
    if (!buf) { fclose(f); return px_null(); }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    unsigned char digest[32];
    if (rd != (size_t)sz || mbedtls_sha256(buf, rd, digest, 0) != 0) {
        free(buf);
        return px_null();
    }
    free(buf);
    char hex[65];
    bytes_to_hex(digest, 32, hex);
    return px_str(hex);
}

// os_rename(old, new) → bool：原子改名/移动（rename(2)）；成功 true，失败 false（os_errno 可查）。
// 用途：原子替换（update/coreup 下载新包后 rename 覆盖）、文件移动。
static LXValue bi_os_rename(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_STR)
        px_error("os_rename 需要 (old_path, new_path) 参数");
    return px_bool(rename(args[0].as.obj->as.str.data, args[1].as.obj->as.str.data) == 0);
}

// os_remove_all(path) → bool：递归删除文件/目录树（unlink 文件与符号链接；子目录递归后 rmdir）。
// 成功 true，失败 false（os_errno 可查）。保护：空串与根目录（/、//）拒绝，防误删根。
static int rm_tree(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) return -1;
    if (S_ISDIR(st.st_mode)) {
        DIR* d = opendir(path);
        if (!d) return -1;
        struct dirent* e;
        int rc = 0;
        while ((e = readdir(d)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char child[2048];
            snprintf(child, sizeof(child), "%s/%s", path, e->d_name);
            if (rm_tree(child) != 0) { rc = -1; break; }
        }
        closedir(d);
        if (rc != 0) return -1;
        return rmdir(path);
    }
    return unlink(path);
}
static LXValue bi_os_remove_all(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR)
        px_error("os_remove_all 需要 (path) 参数");
    const char* path = args[0].as.obj->as.str.data;
    if (!path[0]) return px_bool(false);
    if (strcmp(path, "/") == 0 || strcmp(path, "//") == 0) return px_bool(false); /* 防删根 */
    return px_bool(rm_tree(path) == 0);
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
    if (nargs < 2 || nargs > 3 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("os_kill 需要 (pid, sig[, group]) 参数");
    pid_t pid = (pid_t)args[0].as.i;
    int sig = (int)args[1].as.i;
    int group = 0;
    if (nargs == 3) {
        if (args[2].type == PX_BOOL) group = args[2].as.b ? 1 : 0;
        else if (args[2].type == PX_INT) group = args[2].as.i != 0;
        else px_error("os_kill 的 group 需要 bool");
    }
    if (group) return px_bool(kill(-pid, sig) == 0);  // M66：杀进程组（清 worker/sleep 子进程）
    return px_bool(kill(pid, sig) == 0);
}

// ==================== M66：qg-issue 合入（unix_connect / os_capture / os_popen） ====================
// unix_connect(socket_path) → fd（Issue 1 / 06 T1）：AF_UNIX SOCK_STREAM 裸连接，
//   返回持久 fd；失败返回 -1（os_errno 可查，与 M57 fd 族一致）。
//   收发复用 M57 read(fd)/write(fd,data)（已具备，无需 unix_send/unix_recv）。
static LXValue bi_unix_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR)
        px_error("unix_connect 需要 (socket_path) 参数");
    const char* path = args[0].as.obj->as.str.data;
    if (strlen(path) >= sizeof(((struct sockaddr_un*)0)->sun_path))
        px_error("unix_connect: socket 路径过长");
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return px_int(-1);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return px_int(-1);
    }
    return px_int((int64_t)fd);
}

// os_capture(cmd, args) → {rc:int, stdout:str, stderr:str} | null（05 G1 / 06 T2）
// fork+execvp，子进程 stdout/stderr 各接一条管道（分离捕获），父进程 poll 双 fd
// 读净后 waitpid。rc 约定同 os_spawn_capture：正常=exit code；信号=128+sig；
// exec 失败=127；pipe/fork/wait 失败=null。用途：安装器/系统管理"跑命令判输出"。
static LXValue bi_os_capture(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_LIST)
        px_error("os_capture 需要 (cmd, args) 参数");
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
            px_error("os_capture 的 args 必须是字符串列表");
        }
        argv[i + 1] = strdup(v.as.obj->as.str.data);
    }
    argv[argc + 1] = NULL;
    int pout[2], perr[2];
    if (pipe(pout) != 0 || pipe(perr) != 0) {
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pout[0]); close(pout[1]); close(perr[0]); close(perr[1]);
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    if (pid == 0) {
        setpgid(0, 0);  // M66：自成进程组 → 支持 os_kill(pid, sig, true) 组杀
        close(pout[0]); close(perr[0]);
        dup2(pout[1], 1); dup2(perr[1], 2);
        close(pout[1]); close(perr[1]);
        execvp(cmd, argv);
        _exit(127);
    }
    close(pout[1]); close(perr[1]);
    for (int i = 0; i <= argc; i++) free(argv[i]);
    free(argv);
    // poll 双管道读净（防单管道写满死锁）
    size_t cap_o = 8192, n_o = 0, cap_e = 8192, n_e = 0;
    char* bo = (char*)xmalloc(cap_o);
    char* be = (char*)xmalloc(cap_e);
    int eof_o = 0, eof_e = 0;
    while (!eof_o || !eof_e) {
        struct pollfd fds[2];
        fds[0].fd = pout[0]; fds[0].events = POLLIN; fds[0].revents = 0;
        fds[1].fd = perr[0]; fds[1].events = POLLIN; fds[1].revents = 0;
        int pr = poll(fds, 2, -1);
        if (pr < 0) { if (errno == EINTR) continue; break; }
        for (int i = 0; i < 2; i++) {
            if ((fds[i].revents & (POLLIN | POLLHUP)) == 0) continue;
            int rfd = (i == 0) ? pout[0] : perr[0];
            int* eof = (i == 0) ? &eof_o : &eof_e;
            char** buf = (i == 0) ? &bo : &be;
            size_t* cap = (i == 0) ? &cap_o : &cap_e;
            size_t* n = (i == 0) ? &n_o : &n_e;
            if (*n + 4096 > *cap) { *cap *= 2; *buf = (char*)xrealloc(*buf, *cap); }
            ssize_t r = read(rfd, *buf + *n, *cap - *n);
            if (r < 0) { if (errno == EINTR) continue; *eof = 1; }
            else if (r == 0) *eof = 1;
            else *n += (size_t)r;
        }
    }
    close(pout[0]); close(perr[0]);
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) { xfree(bo); xfree(be); return px_null(); }
    int rc = -1;
    if (WIFEXITED(status)) rc = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) rc = 128 + (int)WTERMSIG(status);
    LXValue d = px_dict();
    px_dict_set(d, "rc", px_int((int64_t)rc));
    px_dict_set(d, "stdout", px_str_len(bo, (int)n_o));
    px_dict_set(d, "stderr", px_str_len(be, (int)n_e));
    xfree(bo); xfree(be);
    return d;
}

// os_popen(cmd, args) → {pid:int, stdin_fd:int, stdout_fd:int} | null（05 G4 / 06 T2）
// 双向管道：向子进程 stdin 注入 + 读回 stdout（对话式进程：chat core / sudo 提权等）。
// 返回 pid + stdin 写端 fd + stdout 读端 fd；用完 os_wait(pid)/os_kill(pid,...) 回收。
static LXValue bi_os_popen(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_LIST)
        px_error("os_popen 需要 (cmd, args) 参数");
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
            px_error("os_popen 的 args 必须是字符串列表");
        }
        argv[i + 1] = strdup(v.as.obj->as.str.data);
    }
    argv[argc + 1] = NULL;
    int pin[2], pout[2];
    if (pipe(pin) != 0 || pipe(pout) != 0) {
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pin[0]); close(pin[1]); close(pout[0]); close(pout[1]);
        for (int i = 0; i <= argc; i++) free(argv[i]);
        free(argv);
        return px_null();
    }
    if (pid == 0) {
        setpgid(0, 0);  // M66：自成进程组 → 支持 os_kill(pid, sig, true) 组杀
        close(pin[1]); close(pout[0]);
        dup2(pin[0], 0); dup2(pout[1], 1);
        close(pin[0]); close(pout[1]);
        execvp(cmd, argv);
        _exit(127);
    }
    close(pin[0]); close(pout[1]);
    for (int i = 0; i <= argc; i++) free(argv[i]);
    free(argv);
    LXValue d = px_dict();
    px_dict_set(d, "pid", px_int((int64_t)pid));
    px_dict_set(d, "stdin_fd", px_int((int64_t)pin[1]));
    px_dict_set(d, "stdout_fd", px_int((int64_t)pout[0]));
    return d;
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
    struct PxCoro* w = NULL;
    pthread_mutex_lock(&o->as.mutex.mu);
    o->as.mutex.locked = 0;
    pthread_cond_signal(&o->as.mutex.cv);
    w = px_coro_take_waiter(&o->as.mutex.cw);   // M93-S3：唤醒首协程 lock 等待者
    pthread_mutex_unlock(&o->as.mutex.mu);
    px_coro_wake(w);
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
    struct PxCoro* w = NULL;
    pthread_mutex_lock(&o->as.rwlock.mu);
    if (o->as.rwlock.readers > 0) o->as.rwlock.readers--;
    if (o->as.rwlock.readers == 0) {
        pthread_cond_broadcast(&o->as.rwlock.cv);
        // M93-S3：写者优先唤醒（有协程写者等 → 先给它；无则协程读者）
        w = px_coro_take_waiter(&o->as.rwlock.cw_w);
        if (!w) w = px_coro_take_waiter(&o->as.rwlock.cw_r);
    }
    pthread_mutex_unlock(&o->as.rwlock.mu);
    px_coro_wake(w);
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
    struct PxCoro* w = NULL;
    pthread_mutex_lock(&o->as.rwlock.mu);
    o->as.rwlock.writer = 0;
    pthread_cond_broadcast(&o->as.rwlock.cv);
    // M93-S3：写者优先唤醒协程等待者（无协程写者则协程读者）
    w = px_coro_take_waiter(&o->as.rwlock.cw_w);
    if (!w) w = px_coro_take_waiter(&o->as.rwlock.cw_r);
    pthread_mutex_unlock(&o->as.rwlock.mu);
    px_coro_wake(w);
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

// M93-S3：通道双轨唤醒辅助（调用方已持 chan.mu）——signal pthread 等待者 +
//   摘链首协程等待者（解锁后由调用方 px_coro_wake）。无协程等待者时头为 NULL，
//   摘链空操作（协程系统未启用/无等待，零开销）。
static struct PxCoro* chan_sig_recv(LXObject* o) {
    pthread_cond_signal(&o->as.chan.cv_recv);
    return px_coro_take_waiter(&o->as.chan.cw_recv);
}
static struct PxCoro* chan_sig_send(LXObject* o) {
    pthread_cond_signal(&o->as.chan.cv_send);
    return px_coro_take_waiter(&o->as.chan.cw_send);
}
// 有「接收者就绪」（pthread recv_waiting 或协程 cw_recv）——无缓冲 send 交付判据
static int chan_has_recver(const LXObject* o) {
    return o->as.chan.recv_waiting > 0 || o->as.chan.cw_recv != NULL;
}

LXValue px_chan_send(LXValue ch, LXValue val) {
    if (ch.type != PX_CHAN) px_error("send: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    struct PxCoro* w = NULL;
    pthread_mutex_lock(&o->as.chan.mu);
    while (1) {
        if (o->as.chan.closed) {
            pthread_mutex_unlock(&o->as.chan.mu);
            px_error("R1011: 向已关闭的通道发送");
        }
        if (o->as.chan.cap == 0) {
            // 无缓冲：等待接收者就绪（pthread recv_waiting 或协程 cw_recv）
            if (chan_has_recver(o)) {
                // M22 修复：写 buf 元素（LXValue 多字节非原子）期间屏蔽 GC 暂停信号，
                // 防 GC 扫描 chan 读到半写入值 → 活跃对象漏标被误回收
                sigset_t old;
                gc_block_stop(&old);
                o->as.chan.buf[0] = val;
                o->as.chan.len = 1;
                gc_unblock_stop(&old);
                w = chan_sig_recv(o);
                pthread_mutex_unlock(&o->as.chan.mu);
                px_coro_wake(w);
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
                w = chan_sig_recv(o);
                pthread_mutex_unlock(&o->as.chan.mu);
                px_coro_wake(w);
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
    struct PxCoro* w = NULL;
    pthread_mutex_lock(&o->as.chan.mu);
    while (1) {
        if (o->as.chan.len > 0) {
            LXValue v = o->as.chan.buf[o->as.chan.head];
            if (o->as.chan.cap == 0) {
                // 无缓冲：清空交付槽，通知等待的发送者
                o->as.chan.len = 0;
                if (o->as.chan.recv_waiting > 0) o->as.chan.recv_waiting--;
            } else {
                o->as.chan.head = (o->as.chan.head + 1) % o->as.chan.cap;
                o->as.chan.len--;
            }
            w = chan_sig_send(o);
            pthread_mutex_unlock(&o->as.chan.mu);
            px_coro_wake(w);
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
    struct PxCoro* w = NULL;
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
        w = chan_sig_send(o);
        ok = true;
    }
    pthread_mutex_unlock(&o->as.chan.mu);
    px_coro_wake(w);
    if (ok) px_select_signal();
    return ok;
}

// M93-S3：非阻塞发送（select/协程 try 用）。成功 = 值已入缓冲/直接交付接收者。
//   closed → false（调用方须走阻塞版 px_chan_send 报 R1011，不得登记等待）。
bool px_chan_try_send(LXValue ch, LXValue val) {
    if (ch.type != PX_CHAN) px_error("send: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    bool ok = false;
    struct PxCoro* w = NULL;
    pthread_mutex_lock(&o->as.chan.mu);
    if (o->as.chan.closed) {
        pthread_mutex_unlock(&o->as.chan.mu);
        return false;
    }
    if (o->as.chan.cap == 0) {
        if (chan_has_recver(o)) {          // 无缓冲：有接收者就绪（pthread/协程）
            sigset_t old;
            gc_block_stop(&old);
            o->as.chan.buf[0] = val;
            o->as.chan.len = 1;
            gc_unblock_stop(&old);
            w = chan_sig_recv(o);
            ok = true;
        }
    } else if (o->as.chan.len < o->as.chan.cap) {
        int tail = (o->as.chan.head + o->as.chan.len) % o->as.chan.cap;
        sigset_t old;
        gc_block_stop(&old);
        o->as.chan.buf[tail] = val;
        o->as.chan.len++;
        gc_unblock_stop(&old);
        w = chan_sig_recv(o);
        ok = true;
    }
    pthread_mutex_unlock(&o->as.chan.mu);
    px_coro_wake(w);
    if (ok) px_select_signal();
    return ok;
}

void px_chan_close(LXValue ch) {
    if (ch.type != PX_CHAN) px_error("close: 目标不是通道（%s）", px_type_name(ch));
    LXObject* o = ch.as.obj;
    struct PxCoro* w1 = NULL;
    struct PxCoro* w2 = NULL;
    pthread_mutex_lock(&o->as.chan.mu);
    o->as.chan.closed = 1;
    pthread_cond_broadcast(&o->as.chan.cv_send);
    pthread_cond_broadcast(&o->as.chan.cv_recv);
    // 协程等待者全部唤醒（醒来重试 try：send 见 closed → 阻塞版报 R1011；
    //   recv len==0 且 closed → 阻塞版报 R1011；期间新到的数据先被取走则继续）
    while ((w1 = px_coro_take_waiter(&o->as.chan.cw_send)) != NULL)
        px_coro_wake(w1);
    while ((w2 = px_coro_take_waiter(&o->as.chan.cw_recv)) != NULL)
        px_coro_wake(w2);
    pthread_mutex_unlock(&o->as.chan.mu);
    px_select_signal();
}

// ==================== spawn（pthread） ====================

typedef struct {
    LXFuncPtr fn;
    void* ctx;          // M89-S3-B5：spawn 目标上下文（VM PX_FUNC 需 ctx=PxVMFunc*；
                        //   旧 C 编译产物 ctx=NULL —— spawn_thread 透传）
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
    for (int i = 0; i < g_thread_cap; i++) {
        if (g_threads[i].in_use && (uintptr_t)g_threads[i].tid == 0) {
            g_threads[i].tid = pthread_self();
            break;
        }
    }
    pthread_mutex_unlock(&g_gc_mu);
    // M72-S3（Issue 10 D2）：spawn 协程错误隔离（默认开；PX_SPAWN_ISOLATE=0 关 →
    // px_error 保持原 exit 语义向后兼容）。协程内运行时错误打印现场后 longjmp 回
    // 此捕获点 → 走下方注销路径安全退出线程，宿主进程继续。注意：崩溃点若在
    // mutex/rwlock 临界区内，longjmp 不展开 pthread 锁 → 锁遗留（宿主可能等锁）；
    // 临界区内请用 Result/? 收敛可预期错误（运行时错误 = bug，隔离保进程不死）。
    int isolate = 1;
    const char* iso_env = getenv("PX_SPAWN_ISOLATE");
    if (iso_env && iso_env[0] == '0') isolate = 0;
    if (isolate) {
        if (setjmp(g_err_jmp) == 0) {
            g_err_jmp_set = 1;
            job->fn(job->args, job->nargs, job->ctx);
            g_err_jmp_set = 0;
        } else {
            g_err_jmp_set = 0;
            fprintf(stderr, "[px-spawn] 协程运行时错误已隔离，宿主继续（错误现场见上）\n");
            fflush(stderr);
        }
    } else {
        job->fn(job->args, job->nargs, job->ctx);
    }
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

// ==================== M93-S2：错误隔离 / GC 信号屏蔽原语导出（coro worker 复用） ====================
// px_spawn_isolate_begin/end：spawn 语义错误隔离点（px_error → longjmp 回 begin 返回 0）。
//   coro.c worker 每个协程执行前调 begin（setjmp 环境在 worker 栈帧，longjmp 回卷安全）；
//   g_err_jmp/g_err_jmp_set 为 TLS static，跨文件不可直接访问 → 经本导出函数使用。
//   语义对齐 spawn_thread：错误打印现场后隔离，宿主/worker 继续（协程异常终止回收）。
int px_spawn_isolate_begin(void) {
    if (setjmp(g_err_jmp) == 0) {
        g_err_jmp_set = 1;
        return 1;   // 正常路径：错误捕获点已安装，继续执行协程体
    }
    g_err_jmp_set = 0;
    fprintf(stderr, "[px-coro] 协程运行时错误已隔离，宿主继续（错误现场见上）\n");
    fflush(stderr);
    return 0;       // 错误路径：longjmp 回此，协程异常终止
}
void px_spawn_isolate_end(void) {
    g_err_jmp_set = 0;
}
// M96-S2：受保护 native 调用（offload 外包线程用；见 runtime.h）。setjmp 在本函数内
//   消化 longjmp —— px_call → px_error → longjmp 回本函数 setjmp → return 1（错误已
//   打印；errbuf 带回 g_err_last_msg 文本，由协程恢复后重抛带源位置）。调用者只见
//   普通一次返回（0/1），不依赖「setjmp 包装 helper 返回两次」的跨函数语义（gcc
//   -O1/-O2 下此类 helper 返回分支可能被优化错判 —— 最小复现证实 → 必须同函数消化）。
int px_native_call_capture(LXValue fn, LXValue* args, int nargs,
                           LXValue* out, char* errbuf, int errbuf_sz) {
    if (setjmp(g_err_jmp) == 0) {
        g_err_jmp_set = 1;
        LXValue r = px_call(fn, args, nargs);
        PX_KEEP(r);                  // 返回前保护（*out 拷出前可被并发 GC STW 暂停）
        g_err_jmp_set = 0;
        if (out) *out = r;
        return 0;
    }
    g_err_jmp_set = 0;
    if (errbuf && errbuf_sz > 0)
        snprintf(errbuf, (size_t)errbuf_sz, "%s",
                 g_err_last_msg[0] ? g_err_last_msg : "外包执行失败");
    return 1;
}
void px_gc_block_stop_sig(sigset_t* old) {
    gc_block_stop(old);
}
void px_gc_unblock_stop_sig(const sigset_t* old) {
    gc_unblock_stop(old);
}

// ==================== M53-S3：外部裸线程纳入并发 GC ====================
// QUIC/H3 托管连接线程由 runtime_quic.c 直接 pthread_create（不经 px_spawn），
// 但 M53-S3 起连接回调会构造普贤对象（请求 dict/响应字段）并可能触发 GC ——
// 线程必须注册进 g_threads（被 stop-the-world 暂停 + 保守扫描栈），否则其栈上
// 对象会被 GC 误回收（use-after-free）。语义与 px_pool_worker 常驻注册一致：
//   enter：g_active_threads++ + 分配 GC 槽位（并发 GC 路径随之启用）
//   leave：g_active_threads-- + 注销槽位（须在不再触碰普贤对象后最后调用）
// M88-S1（qg-issue 27）：g_threads 槽上限现为 g_thread_max（PX_MAX_THREADS 可配，默认 1024）；
// spawn/连接池/H3 共享同一表；槽满时本线程不被 GC 暂停
//     （与连接池 worker 槽满行为一致），H3 生产并发上限评估留待 S4。
void px_gc_thread_enter(void) {
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    g_active_threads++;
    int slot = -1;
    for (int i = 0; i < g_thread_cap; i++) {
        if (!g_threads[i].in_use) { slot = i; break; }
    }
    if (slot >= 0) {
        g_threads[slot].tid = pthread_self();
        g_threads[slot].in_use = 1;
        g_threads[slot].paused = 0;
        g_threads[slot].is_main = 0;
        g_threads[slot].epoch = 0;
        g_threads[slot].tmp_root = NULL;
    }
    pthread_mutex_unlock(&g_gc_mu);
}

void px_gc_thread_leave(void) {
    pthread_mutex_lock(&g_gc_mu);
    if (g_active_threads > 0) g_active_threads--;
    gc_unregister_thread(pthread_self());
    pthread_mutex_unlock(&g_gc_mu);
}

void px_spawn_ctx(LXFuncPtr fn, void* ctx, LXValue* args, int nargs) {
    pthread_mutex_lock(&g_gc_mu);   // M8：创建前先标记活跃（防止主线程 GC 误判）
    if (!g_gc_env_inited) gc_init_env();
    g_active_threads++;
    // M11：同一临界区预留线程槽位（tid=0 表示创建中，GC 视为无需暂停）
    int slot = -1;
    for (int i = 0; i < g_thread_cap; i++) {
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
    if (slot < 0) px_error("spawn: 并发线程数超出上限 %d", g_thread_max);
    SpawnJob* job = xmalloc(sizeof(SpawnJob));
    job->fn = fn;
    job->ctx = ctx;
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

void px_spawn(LXFuncPtr fn, LXValue* args, int nargs) {
    px_spawn_ctx(fn, NULL, args, nargs);
}

void px_spawn_name(const char* fname, LXValue* args, int nargs) {
    LXValue fn = px_get_global(fname);
    if (fn.type == PX_FUNC) {
        // M93-S2：帧协程分派 —— 目标为 VM 函数（PX_FUNC.fn==px_vm_entry，ctx=PxVMFunc*）
        //   → 协程化（入就绪队列，worker 池执行，线程数 = worker 数）；否则 = 旧 C 轨
        //   逃生舱 fn_*（ctx=NULL）→ 原 pthread 路径（帧协程在其上不可行，语义零变化）。
        //   px_vm_entry 为 weak（C 轨逃生舱无 vm.o → NULL → 全走 pthread）。
        if (px_vm_entry && fn.as.obj->as.func.fn == px_vm_entry) {
            if (px_coro_spawn) {       // coro.c 已链（VM 轨 rt 缓存含 coro.o）
                px_coro_spawn(fn.as.obj->as.func.ctx, args, nargs);
                return;
            }
            // 无协程内核（理论不达：rt_src_files 恒含 coro.c）→ 退回 pthread 保语义
        }
        // M89-S3-B5：透传函数 ctx —— VM PX_FUNC(px_vm_entry, ctx=PxVMFunc*) 需 ctx
        //   定位字节码函数；旧 C 编译产物 ctx=NULL 不变（此前只传 fn 丢 ctx → VM spawn
        //   worker 线程 px_vm_entry ctx=NULL 直接返回，静默不执行 → 主线程 chan.recv 死等）
        px_spawn_ctx(fn.as.obj->as.func.fn, fn.as.obj->as.func.ctx, args, nargs);
    } else if (fn.type == PX_NATIVE) {
        px_spawn_ctx(fn.as.obj->as.native.fn, NULL, args, nargs);
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
    for (int i = 0; i < g_thread_cap; i++) {
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
    for (int i = 0; i < g_thread_cap; i++) if (!g_threads[i].in_use) { gslot = i; break; }
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
    if (gslot < 0) px_error("定时器: 并发线程数超出上限 %d", g_thread_max);

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


// ==================== M36：请求上下文（线程局部；与解释器 builtin.rs 一致） ====================
// ctx_set(key, value) / ctx_get(key) / ctx_clear()
// 线程局部（__thread）：每请求线程独立，px_conn_worker 请求开始自动清除
#define PX_CTX_MAX 64
static __thread char g_px_ctx_keys[PX_CTX_MAX][64];
static __thread LXValue g_px_ctx_vals[PX_CTX_MAX];
static __thread int g_px_ctx_n = 0;

static LXValue bi_ctx_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("ctx_set 需要 (key, value) 参数");
    const char* key = args[0].as.obj->as.str.data;
    for (int i = 0; i < g_px_ctx_n; i++) {
        if (strcmp(g_px_ctx_keys[i], key) == 0) { g_px_ctx_vals[i] = args[1]; return px_bool(true); }
    }
    if (g_px_ctx_n >= PX_CTX_MAX) px_error("ctx_set: 上下文条目超出上限 %d", PX_CTX_MAX);
    snprintf(g_px_ctx_keys[g_px_ctx_n], sizeof(g_px_ctx_keys[0]), "%s", key);
    g_px_ctx_vals[g_px_ctx_n] = args[1];
    g_px_ctx_n++;
    return px_bool(true);
}

static LXValue bi_ctx_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("ctx_get 需要 (key) 参数");
    const char* key = args[0].as.obj->as.str.data;
    for (int i = 0; i < g_px_ctx_n; i++) {
        if (strcmp(g_px_ctx_keys[i], key) == 0) return g_px_ctx_vals[i];
    }
    return px_null();
}

static LXValue bi_ctx_clear(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("ctx_clear 不需要参数");
    g_px_ctx_n = 0;
    return px_bool(true);
}


// ==================== M33：UDP 基础设施（HTTP/3/QUIC 预研；与解释器 builtin.rs 一致） ====================
// udp_open([port]) → int（bind 0.0.0.0:port；缺省/0 → 系统分配）
// udp_send(sock, ip, port, data) → int；udp_recv(sock, maxlen) → dict{data,ip,port} | null
// udp_close(sock) → bool
static LXValue bi_udp_open(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs > 1) px_error("udp_open 需要 (port) 参数");
    int port = (nargs == 1 && args[0].type == PX_INT) ? (int)args[0].as.i : 0;
    if (port < 0 || port > 65535) px_error("udp_open 端口范围 0-65535");
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) px_error("udp_open: 创建 socket 失败");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        int e = errno;
        close(fd);
        px_error("udp_open: 绑定端口 %d 失败 (%d)", port, e);
    }
    return px_int(fd);
}

static LXValue bi_udp_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 4 || args[0].type != PX_INT || args[1].type != PX_STR || args[2].type != PX_INT) {
        px_error("udp_send 需要 (sock, ip, port, data) 参数");
    }
    int fd = (int)args[0].as.i;
    const char* ip = args[1].as.obj->as.str.data;
    int port = (int)args[2].as.i;
    const char* data;
    int len;
    if (args[3].type == PX_STR || args[3].type == PX_BYTES) {
        data = args[3].as.obj->as.str.data;
        len = args[3].as.obj->as.str.len;
    } else {
        px_error("udp_send 的 data 需要 str/bytes");
    }
    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &dst.sin_addr) != 1) {
        struct addrinfo hints, *res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        char portstr[16];
        snprintf(portstr, sizeof(portstr), "%d", port);
        if (getaddrinfo(ip, portstr, &hints, &res) != 0 || !res) px_error("udp_send: 解析主机失败 %s", ip);
        memcpy(&dst, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
    }
    int n = (int)sendto(fd, data, (size_t)len, 0, (struct sockaddr*)&dst, sizeof(dst));
    if (n < 0) px_error("udp_send 失败 (errno=%d)", errno);
    return px_int(n);
}

static LXValue bi_udp_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT) {
        px_error("udp_recv 需要 (sock, maxlen) 参数");
    }
    int fd = (int)args[0].as.i;
    int maxlen = (int)args[1].as.i;
    if (maxlen < 1) px_error("udp_recv 的 maxlen 需要正整数");
    char* buf = xmalloc((size_t)maxlen + 1);
    struct sockaddr_in src;
    socklen_t slen = sizeof(src);
    int n = (int)recvfrom(fd, buf, (size_t)maxlen, 0, (struct sockaddr*)&src, &slen);
    if (n < 0) {
        xfree(buf);
        return px_null();
    }
    LXValue r = px_dict();
    px_dict_set(r, "data", px_bytes_len(buf, n));
    char ip[64] = {0};
    inet_ntop(AF_INET, &src.sin_addr, ip, sizeof(ip));
    px_dict_set(r, "ip", px_str(ip));
    px_dict_set(r, "port", px_int(ntohs(src.sin_port)));
    xfree(buf);
    return r;
}

static LXValue bi_udp_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("udp_close 需要 (sock) 参数");
    close((int)args[0].as.i);
    return px_bool(true);
}

// M38：udp_serve(port, handler)——UDP echo 服务端（handler(ip, port, data) → 响应发送回对端）
static LXValue bi_udp_serve(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("udp_serve 需要 (port, handler) 参数");
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("udp_serve 的 handler 必须是函数");
    int port = (int)args[0].as.i;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) px_error("udp_serve: socket 创建失败");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        px_error("udp_serve: 绑定端口 %d 失败", port);
    }
    char buf[65536];
    for (;;) {
        struct sockaddr_in src;
        socklen_t slen = sizeof(src);
        int n = (int)recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &slen);
        if (n < 0) continue;
        buf[n] = 0;
        char ip[64];
        inet_ntop(AF_INET, &src.sin_addr, ip, sizeof(ip));
        LXValue hargs[3];
        hargs[0] = px_str(ip);
        hargs[1] = px_int(ntohs(src.sin_port));
        hargs[2] = px_str_len(buf, n);
        LXValue r = px_call(handler, hargs, 3);
        if (r.type != PX_NULL) {
            const char* resp;
            int rlen;
            if (r.type == PX_STR || r.type == PX_BYTES) {
                resp = r.as.obj->as.str.data;
                rlen = r.as.obj->as.str.len;
            } else {
                resp = px_to_string(r);
                rlen = (int)strlen(resp);
            }
            (void)sendto(fd, resp, (size_t)rlen, 0, (struct sockaddr*)&src, sizeof(src));
        }
    }
    return px_null(); // 不可达
}

// ==================== M34：事件总线（pub/sub，跨线程；与解释器 builtin.rs 一致） ====================
// event_bus() → int；bus_subscribe(bus, topic, fn) → bool；bus_publish(bus, topic, data) → int（同步回调数）
// bus_unsubscribe(bus, topic, fn) → bool（按函数对象指针身份移除）
#define PX_BUS_MAX 64
#define PX_BUS_TOPIC_MAX 32
#define PX_BUS_SUB_MAX 16
typedef struct {
    char topic[64];
    LXValue fns[PX_BUS_SUB_MAX];
    int nfns;
    int active;
} PxBusTopic;
typedef struct {
    int active;
    int64_t id;
    PxBusTopic topics[PX_BUS_TOPIC_MAX];
    int ntopics;
} PxBus;
static PxBus g_px_buses[PX_BUS_MAX];
static int64_t g_px_bus_seq = 1;
static pthread_mutex_t g_bus_mu = PTHREAD_MUTEX_INITIALIZER;

static LXValue bi_event_bus(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("event_bus 不需要参数");
    pthread_mutex_lock(&g_bus_mu);
    int slot = -1;
    for (int i = 0; i < PX_BUS_MAX; i++) if (!g_px_buses[i].active) { slot = i; break; }
    if (slot < 0) {
        pthread_mutex_unlock(&g_bus_mu);
        px_error("事件总线数量超出上限 %d", PX_BUS_MAX);
    }
    int64_t id = g_px_bus_seq++;
    memset(&g_px_buses[slot], 0, sizeof(PxBus));
    g_px_buses[slot].active = 1;
    g_px_buses[slot].id = id;
    pthread_mutex_unlock(&g_bus_mu);
    return px_int(id);
}

static PxBus* bus_find(int64_t id) {
    for (int i = 0; i < PX_BUS_MAX; i++) {
        if (g_px_buses[i].active && g_px_buses[i].id == id) return &g_px_buses[i];
    }
    return NULL;
}

static PxBusTopic* bus_topic_find_or_new(PxBus* b, const char* topic) {
    for (int i = 0; i < b->ntopics; i++) {
        if (b->topics[i].active && strcmp(b->topics[i].topic, topic) == 0) return &b->topics[i];
    }
    if (b->ntopics >= PX_BUS_TOPIC_MAX) return NULL;
    PxBusTopic* t = &b->topics[b->ntopics++];
    memset(t, 0, sizeof(*t));
    snprintf(t->topic, sizeof(t->topic), "%s", topic);
    t->active = 1;
    return t;
}

static LXValue bi_bus_subscribe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_INT || args[1].type != PX_STR || args[2].type != PX_FUNC) {
        px_error("bus_subscribe 需要 (bus, topic, fn) 参数");
    }
    pthread_mutex_lock(&g_bus_mu);
    PxBus* b = bus_find(args[0].as.i);
    if (!b) { pthread_mutex_unlock(&g_bus_mu); px_error("bus_subscribe: 无效 bus id"); }
    PxBusTopic* t = bus_topic_find_or_new(b, args[1].as.obj->as.str.data);
    if (!t || t->nfns >= PX_BUS_SUB_MAX) {
        pthread_mutex_unlock(&g_bus_mu);
        px_error("bus_subscribe: 主题订阅者超出上限 %d", PX_BUS_SUB_MAX);
    }
    t->fns[t->nfns++] = args[2];
    pthread_mutex_unlock(&g_bus_mu);
    return px_bool(true);
}

static LXValue bi_bus_publish(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_INT || args[1].type != PX_STR) {
        px_error("bus_publish 需要 (bus, topic, data) 参数");
    }
    // 快照订阅者（锁外回调，避免持锁执行用户代码）
    LXValue fns[PX_BUS_SUB_MAX];
    int n = 0;
    pthread_mutex_lock(&g_bus_mu);
    PxBus* b = bus_find(args[0].as.i);
    if (!b) { pthread_mutex_unlock(&g_bus_mu); px_error("bus_publish: 无效 bus id"); }
    for (int i = 0; i < b->ntopics; i++) {
        if (b->topics[i].active && strcmp(b->topics[i].topic, args[1].as.obj->as.str.data) == 0) {
            for (int j = 0; j < b->topics[i].nfns && n < PX_BUS_SUB_MAX; j++) fns[n++] = b->topics[i].fns[j];
            break;
        }
    }
    pthread_mutex_unlock(&g_bus_mu);
    LXValue call_args[2];
    call_args[0] = px_str(args[1].as.obj->as.str.data);
    call_args[1] = args[2];
    for (int i = 0; i < n; i++) (void)px_call(fns[i], call_args, 2);
    return px_int(n);
}

static LXValue bi_bus_unsubscribe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_INT || args[1].type != PX_STR || args[2].type != PX_FUNC) {
        px_error("bus_unsubscribe 需要 (bus, topic, fn) 参数");
    }
    int removed = 0;
    pthread_mutex_lock(&g_bus_mu);
    PxBus* b = bus_find(args[0].as.i);
    if (b) {
        for (int i = 0; i < b->ntopics; i++) {
            PxBusTopic* t = &b->topics[i];
            if (!t->active || strcmp(t->topic, args[1].as.obj->as.str.data) != 0) continue;
            for (int j = 0; j < t->nfns; j++) {
                if (t->fns[j].as.obj == args[2].as.obj) {
                    memmove(&t->fns[j], &t->fns[j + 1], (size_t)(t->nfns - j - 1) * sizeof(LXValue));
                    t->nfns--;
                    removed = 1;
                    break;
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&g_bus_mu);
    return px_bool(removed);
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
    if (!s) return;    // M25：关闭前补存会话（TLS 1.3 NewSessionTicket 常在首轮读时到达）
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

// ==================== M32 wss 客户端导出（runtime_ws.c 复用） ====================
// HttpsSession 为 runtime.c 内部类型；对 runtime_ws.c 暴露 void* 包装。
void* px_https_connect_ex(const char* host, int port) {
    return (void*)https_connect(host, port);
}
int px_https_fd_ex(void* hs) {
    HttpsSession* s = (HttpsSession*)hs;
    return s ? s->net.fd : -1;
}
void px_https_close_ex(void* hs) {
    https_close((HttpsSession*)hs);
}
// 把 HTTPS 会话包装为 PxConn（指针指向会话内部；会话生命周期由调用方管理）
PxConn* px_pxconn_from_https(void* hs) {
    HttpsSession* s = (HttpsSession*)hs;
    if (!s) return NULL;
    PxConn* c = (PxConn*)xmalloc(sizeof(PxConn));
    memset(c, 0, sizeof(PxConn));
    c->fd = s->net.fd;
    c->is_tls = 1;
    c->owned = 0;
    c->ssl = &s->ssl;
    c->conf = &s->conf;
    c->ctr_drbg = &s->ctr_drbg;
    c->entropy = &s->entropy;
    return c;
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
static int hparse_url(const char* url, int* is_https, char* host, int host_cap, int* port, const char** path) {
    *is_https = 0;
    const char* rest = url;
    if (strncmp(url, "https://", 8) == 0) { *is_https = 1; rest = url + 8; }
    else if (strncmp(url, "http://", 7) == 0) { rest = url + 7; }
    else return -1;  // 不支持的协议（不再终止；调用方转 Err 返回）
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
    return 0;
}

// ==================== HTTP 客户端网络失败 → Err(result)（语言面修复） ====================
// 历史行为：http_get/http_post/http_request/http_unix/http_get_stream 网络失败（解析/建连/
// TLS/IO/重定向/协议不支持）直接 px_error → 打印"运行时错误:"并 exit(1)，无错误返回；长期
// daemon 内不能安全发起网络调用（MINI_SUBSET §十三 #1/#2，M58 dogfood 暴露）。
// 修复语义：网络失败 → 返回 Err(result)（消息 "net: ..."），调用方可 is_err()/`?`/unwrap 处理，
// 进程不终止；成功返回值不变（http_get/post → body 字符串；http_request/unix →
// dict{status,headers,body}）。参数个数/类型错误（编程契约）仍 px_error 终止。
// px_net_fail：底层 helper 填错误缓冲后返回失败信号（NULL/-1），builtin 再包装 Err。
// px_net_err：builtin 就地构造并返回 Err("net: ...") result。
static void px_net_fail(char* errbuf, int errcap, const char* fmt, ...) {
    if (!errbuf || errcap <= 0) return;
    va_list ap; va_start(ap, fmt);
    vsnprintf(errbuf, (size_t)errcap, fmt, ap);
    va_end(ap);
}
static LXValue px_net_err(const char* fmt, ...) {
    char buf[300];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return px_err(px_str(buf));
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
// M72-S4（Issue 13 GAP-BIN-2）：h_exchange 增 body/body_n 参数——请求体独立发送
// （与 header 分离），支持任意大小二进制 bytes body（\0 不截断、不塞 req 缓冲，
// 顺带修复原 body 并入 16KB req 缓冲的大 payload 溢出）。body 为 NULL/len 0 → 不发。
static int h_exchange(HPoolSlot* slot, const char* req, int rlen,
                      const char* body, int body_n,
                      int* out_status, LXValue* out_headers,
                      char** out_body, int* out_body_len, int* out_keep_alive) {
    int fd = slot->fd;
    HttpsSession* tls = slot->is_tls ? slot->tls : NULL;
    if (conn_send(tls, fd, req, rlen) < 0) return -1;
    if (body && body_n > 0) {
        if (conn_send(tls, fd, body, body_n) < 0) return -1;
    }
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
    // M97-S2（qg-issue 31）：解析响应协议版本——keep_alive 判定须按 HTTP 版本语义
    //   （RFC 7230 §6.3：HTTP/1.1 默认复用、显式 close 才关；HTTP/1.0 默认短连接、
    //   须显式 Connection: keep-alive 才可复用）。修复前版本被 %*s 直接跳过 →
    //   HTTP/1.0 上游（无 Connection 头 + Content-Length、发完即关）的死连接被
    //   误判可复用回池 → 二次请求取死连接失败（清歌 Mahesvara 反代奇偶失败实测）。
    int status = 0;
    int resp10 = 0;
    if (strncmp(buf, "HTTP/1.0", 8) == 0 && (buf[8] == ' ' || buf[8] == '\r')) resp10 = 1;
    sscanf(buf, "HTTP/%*s %d", &status);
    *out_headers = px_dict();
    int chunked = 0, gzip = 0, keep_alive = resp10 ? 0 : 1;
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
            // M97-S2：协议感知 keep_alive——Connection: close 一律关；
            //   HTTP/1.0 显式 Connection: keep-alive 才置可复用（覆盖初值 0）。
            if (strcasecmp(k, "Connection") == 0) {
                if (strcasecmp(v, "close") == 0) keep_alive = 0;
                else if (resp10 && strcasecmp(v, "keep-alive") == 0) keep_alive = 1;
            }
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

// M84-S1（Issue 23）：extra_headers（"K: V\r\n" 逐行拼接）中是否已含指定头名——行首匹配、
// 大小写不敏感（避免 strcasestr 子串误伤 "X-Content-Type" 之类）；用于 CT/CL 判定解耦。
static int px_extra_hdr_has(const char* extra_headers, const char* name) {
    size_t nl = strlen(name);
    const char* p = extra_headers;
    while (*p) {
        if (strncasecmp(p, name, nl) == 0 && p[nl] == ':') return 1;
        const char* eol = strstr(p, "\r\n");
        if (!eol) break;
        p = eol + 2;
    }
    return 0;
}

// http_request(url, method, body?, headers?) → dict{status, headers, body}
static LXValue bi_http_request(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    // M37：http_request(url, method[, body[, headers[, opts{retries,timeout_ms,proxy}]]])
    if (nargs < 2 || nargs > 5) px_error("http_request 需要 (url, method[, body[, headers[, opts]]]) 参数");
    const char* url = val_cstr(args[0]);
    const char* method = val_cstr(args[1]);
    // M72-S4（Issue 13 GAP-BIN-2）：body 收 str|bytes，长度感知（bytes 含 \0 不截断）
    // body_n = 请求体字节长（区别于下方响应体长 body_len 变量）
    const char* body = NULL;
    int body_n = 0;
    if (nargs >= 3 && (args[2].type == PX_STR || args[2].type == PX_BYTES)) {
        body = args[2].as.obj->as.str.data;
        body_n = args[2].as.obj->as.str.len;
    }
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
    // M37：opts{retries, timeout_ms, proxy}
    int retries = 1;
    int timeout_ms = 30000;
    char proxy[256] = {0};
    if (nargs >= 5 && args[4].type == PX_DICT) {
        LXValue rv = px_dict_get(args[4], "retries");
        if (rv.type == PX_INT && rv.as.i >= 0) retries = (int)rv.as.i + 1;
        LXValue tv = px_dict_get(args[4], "timeout_ms");
        if (tv.type == PX_INT && tv.as.i > 0) timeout_ms = (int)tv.as.i;
        LXValue pv = px_dict_get(args[4], "proxy");
        if (pv.type == PX_STR) snprintf(proxy, sizeof(proxy), "%s", pv.as.obj->as.str.data);
    }
    int is_https = 0;
    char host[256];
    int port = 80;
    const char* path = "/";
    if (hparse_url(url, &is_https, host, sizeof(host), &port, &path) != 0)
        return px_net_err("net: 不支持的协议: %s", url);
    // M37：代理——连接代理地址，请求行用绝对 URL
    char conn_host[256];
    int conn_port = is_https ? 443 : 80;
    char req_target[1024];
    if (proxy[0]) {
        const char* pc = strrchr(proxy, ':');
        if (pc) {
            int pl = (int)(pc - proxy);
            if (pl > 0 && pl < 255) { memcpy(conn_host, proxy, (size_t)pl); conn_host[pl] = 0; }
            else snprintf(conn_host, sizeof(conn_host), "%s", proxy);
            conn_port = atoi(pc + 1);
        } else {
            snprintf(conn_host, sizeof(conn_host), "%s", proxy);
            conn_port = 8080;
        }
        snprintf(req_target, sizeof(req_target), "%s://%s:%d%s", is_https ? "https" : "http", host, port, path);
    } else {
        snprintf(conn_host, sizeof(conn_host), "%s", host);
        conn_port = port;
        snprintf(req_target, sizeof(req_target), "%s", path);
    }
    char key[300];
    snprintf(key, sizeof(key), "%s:%d", host, port);
    char req[16384];
    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: PuXian/0.1\r\nConnection: keep-alive\r\n",
        method, req_target, host, port);
    if (extra_headers[0]) { memcpy(req + rlen, extra_headers, strlen(extra_headers)); rlen += (int)strlen(extra_headers); }
    if (body) {
        // M84-S1（Issue 23）：CT 与 CL 判定解耦——调用方已带 Content-Type 则不再补默认表单头
        // （修复带 CT 无 CL → 双 Content-Type，腾讯云等严服务端拒收）；默认 urlencoded 仅
        // 完全未指定 CT 时补；Content-Length 仅在缺失时补（body_n 长度感知，bytes 含 \0 安全）。
        if (!px_extra_hdr_has(extra_headers, "Content-Type"))
            rlen += snprintf(req + rlen, sizeof(req) - rlen,
                "Content-Type: application/x-www-form-urlencoded\r\n");
        if (!px_extra_hdr_has(extra_headers, "Content-Length"))
            rlen += snprintf(req + rlen, sizeof(req) - rlen, "Content-Length: %d\r\n", body_n);
    }
    rlen += snprintf(req + rlen, sizeof(req) - rlen, "\r\n");
    // M72-S4（Issue 13 GAP-BIN-2）：body 独立发送（h_exchange body/body_n 参数），
    // 不并入 req 缓冲 → 二进制 bytes（含 \0）任意大小安全；顺带修复原 body 塞入
    // 16KB req 缓冲的大 payload 溢出。Content-Length 已按 body_n 长度感知。
    int max_attempts = retries;
    // M97-S2（qg-issue 31）：池中死连接（HTTP/1.0 误判回池 / 对端已关）复用失败时
    //   自动丢弃并新建连接重发一次（不消耗 attempt 预算）——修复前默认 retries=1
    //   时池连接失败路径直接返回 Err（奇偶失败：1/3/5 新建成功、2/4/6 取死连接失败）。
    //   安全性：池连接复用首次 IO 失败 = 连接已死（请求未发出或未收到任何业务响应
    //   字节），重发无半响应污染；仅「新建连接」失败才按原 attempt 预算返回 Err。
    int dead_rebuilt = 0;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        HPoolSlot slot;
        int from_pool = (hpool_take(key, &slot) == 0);
        if (!from_pool) {
            // 池中无空闲连接：新建（http 明文 TCP；https TLS 握手）
            slot.is_tls = is_https;
            slot.fd = -1;
            slot.tls = NULL;
            if (is_https) {
                slot.tls = https_connect(conn_host, conn_port);
                slot.fd = slot.tls ? slot.tls->net.fd : -1;
            } else {
                slot.fd = hconnect(conn_host, conn_port);
            }
            if (slot.fd < 0) {
                if (attempt < max_attempts - 1) continue;
                return px_net_err("net: 连接 %s:%d 失败", conn_host, conn_port);
            }
            // M37：超时配置（SO_RCVTIMEO）
            struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
            setsockopt(slot.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(slot.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        }
        int status = 0, body_len = 0, keep_alive = 1;
        LXValue headers = px_null();
        char* resp_body = NULL;
        if (h_exchange(&slot, req, rlen, body, body_n, &status, &headers, &resp_body, &body_len, &keep_alive) == 0) {
            if (keep_alive) hpool_put(key, slot);
            else { if (slot.tls) https_close(slot.tls); close(slot.fd); }
            LXValue d = px_dict();
            px_root_push();   // M92-S2c precise：http_request 响应 dict 构造登记
            PX_KEEP(headers);   // M92-S2c precise：headers（h_exchange 填充 dict）入 d 前跨分配
            PX_KEEP(d);   // M92-S2c precise：d 裸局部跨 px_dict_set/px_str_len 分配
            px_dict_set(d, "status", px_int(status));
            px_dict_set(d, "headers", headers);
            px_dict_set(d, "body", resp_body ? px_str_len(resp_body, body_len) : px_str(""));
            if (resp_body) xfree(resp_body);
            px_root_pop();   // M92-S2c precise
            return d;
        }
        if (slot.tls) https_close(slot.tls);
        close(slot.fd);
        if (from_pool && !dead_rebuilt) {
            // 池连接复用失败（死连接）→ 丢弃重建：不消耗 attempt（for 的 attempt++
            //   抵消本处 attempt--），下一轮池已空 → 新建连接重发一次。
            dead_rebuilt = 1;
            attempt--;
            continue;
        }
        if (attempt < max_attempts - 1) continue; // 新建连接失败重试（原 retries 语义）
        return px_net_err("net: http_request 失败: 连接关闭");
    }
    return px_null();
}

// M56：http_unix(socket_path, url_path, method[, body[, headers]]) → dict{status, headers, body}
// Unix domain socket 上的 HTTP 客户端（本地服务/LLM 网关/容器 daemon 常用）。
// 不池化：每次新建连接、Connection: close，用完即关（本地低频调用足够）。
static LXValue bi_http_unix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 3 || nargs > 5) px_error("http_unix 需要 (socket_path, url_path, method[, body[, headers]]) 参数");
    const char* sock_path = val_cstr(args[0]);
    const char* url_path = val_cstr(args[1]);
    const char* method = val_cstr(args[2]);
    const char* body = NULL;
    if (nargs >= 4 && args[3].type == PX_STR) body = args[3].as.obj->as.str.data;
    char extra_headers[4096] = {0};
    if (nargs >= 5 && args[4].type == PX_DICT) {
        LXObject* ho = args[4].as.obj;
        int off = 0;
        for (int i = 0; i < ho->as.dict.len && off < 4095; i++) {
            if (ho->as.dict.vals[i].type != PX_STR) continue;
            off += snprintf(extra_headers + off, 4096 - (size_t)off, "%s: %s\r\n",
                            ho->as.dict.keys[i], ho->as.dict.vals[i].as.obj->as.str.data);
        }
    }
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return px_net_err("net: 创建 unix socket 失败");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", sock_path);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        int e = errno;
        close(fd);
        return px_net_err("net: 连接 unix socket %s 失败 (%d)", sock_path, e);
    }
    // 本地网关可能响应慢（如 LLM 长文本），接收/发送超时放宽到 180s
    struct timeval tv = { 180, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    char req[16384];
    int rlen = snprintf(req, sizeof(req),
        "%s %s HTTP/1.1\r\nHost: localhost\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n",
        method, url_path);
    if (extra_headers[0]) { memcpy(req + rlen, extra_headers, strlen(extra_headers)); rlen += (int)strlen(extra_headers); }
    if (body) {
        // M84-S1（Issue 23）：同 bi_http_request——CT 与 CL 解耦，单头语义（http_unix 客户端）
        if (!px_extra_hdr_has(extra_headers, "Content-Type"))
            rlen += snprintf(req + rlen, sizeof(req) - rlen,
                "Content-Type: application/x-www-form-urlencoded\r\n");
        if (!px_extra_hdr_has(extra_headers, "Content-Length"))
            rlen += snprintf(req + rlen, sizeof(req) - rlen, "Content-Length: %d\r\n", (int)strlen(body));
    }
    rlen += snprintf(req + rlen, sizeof(req) - rlen, "\r\n");
    if (body) { memcpy(req + rlen, body, strlen(body)); rlen += (int)strlen(body); }
    HPoolSlot slot;
    slot.is_tls = 0;
    slot.fd = fd;
    slot.tls = NULL;
    int status = 0, body_len = 0, keep_alive = 1;
    LXValue headers = px_null();
    char* resp_body = NULL;
    if (h_exchange(&slot, req, rlen, NULL, 0, &status, &headers, &resp_body, &body_len, &keep_alive) != 0) {
        close(fd);
        return px_net_err("net: http_unix 请求失败: 连接关闭");
    }
    close(fd);
    LXValue d = px_dict();
    px_root_push();   // M92-S2c precise：http_unix 响应 dict 构造登记
    PX_KEEP(headers);   // M92-S2c precise：headers（h_exchange 填充 dict）入 d 前跨分配
    PX_KEEP(d);   // M92-S2c precise：d 裸局部跨 px_dict_set/px_str_len 分配
    px_dict_set(d, "status", px_int(status));
    px_dict_set(d, "headers", headers);
    px_dict_set(d, "body", resp_body ? px_str_len(resp_body, body_len) : px_str(""));
    if (resp_body) xfree(resp_body);
    px_root_pop();   // M92-S2c precise
    return d;
}

// ==================== M37：S3/MinIO（AWS SigV4；与解释器 s3.rs 一致） ====================
// HMAC-SHA256（mbedtls sha256 + 标准 HMAC 块处理）
static void px_hmac_sha256(const unsigned char* key, int klen,
                           const unsigned char* data, int dlen, unsigned char out[32]) {
    unsigned char k[64];
    memset(k, 0, 64);
    if (klen > 64) {
        mbedtls_sha256(key, (size_t)klen, k, 0);
    } else {
        memcpy(k, key, (size_t)klen);
    }
    unsigned char ipad[64], opad[64];
    for (int i = 0; i < 64; i++) { ipad[i] = k[i] ^ 0x36; opad[i] = k[i] ^ 0x5c; }
    unsigned char* inner = malloc(64 + (size_t)dlen);
    memcpy(inner, ipad, 64);
    memcpy(inner + 64, data, (size_t)dlen);
    unsigned char h1[32];
    mbedtls_sha256(inner, 64 + (size_t)dlen, h1, 0);
    free(inner);
    unsigned char* outer = malloc(96);
    memcpy(outer, opad, 64);
    memcpy(outer + 64, h1, 32);
    mbedtls_sha256(outer, 96, out, 0);
    free(outer);
}

static void px_sha256_hex(const char* data, int dlen, char* out) {
    unsigned char d[32];
    mbedtls_sha256((const unsigned char*)data, (size_t)dlen, d, 0);
    for (int i = 0; i < 32; i++) sprintf(out + i * 2, "%02x", d[i]);
    out[64] = 0;
}

// SigV4 签名 → Authorization 头（写入 out）
static void px_sigv4(const char* method, const char* host, const char* path, const char* query,
                     const char* amz_date, const char* date_stamp,
                     const char* payload_hash, const char* ak, const char* sk,
                     char* out, int outsz) {
    // canonical headers: host + x-amz-content-sha256 + x-amz-date（排序）
    char ch[1024];
    snprintf(ch, sizeof(ch), "host:%s\nx-amz-content-sha256:%s\nx-amz-date:%s\n",
             host, payload_hash, amz_date);
    const char* signed_h = "host;x-amz-content-sha256;x-amz-date";
    char canonical[2048];
    snprintf(canonical, sizeof(canonical), "%s\n%s\n%s\n%s\n%s\n%s",
             method, path, query, ch, signed_h, payload_hash);
    char chash[65];
    px_sha256_hex(canonical, (int)strlen(canonical), chash);
    char scope[128];
    snprintf(scope, sizeof(scope), "%s/us-east-1/s3/aws4_request", date_stamp);
    char sts[2048];
    snprintf(sts, sizeof(sts), "AWS4-HMAC-SHA256\n%s\n%s\n%s", amz_date, scope, chash);
    // HMAC 链
    char ksec[128];
    snprintf(ksec, sizeof(ksec), "AWS4%s", sk);
    unsigned char kd[32], kr[32], ks[32], ksg[32], sig[32];
    px_hmac_sha256((const unsigned char*)ksec, (int)strlen(ksec),
                   (const unsigned char*)date_stamp, (int)strlen(date_stamp), kd);
    px_hmac_sha256(kd, 32, (const unsigned char*)"us-east-1", 9, kr);
    px_hmac_sha256(kr, 32, (const unsigned char*)"s3", 2, ks);
    px_hmac_sha256(ks, 32, (const unsigned char*)"aws4_request", 12, ksg);
    px_hmac_sha256(ksg, 32, (const unsigned char*)sts, (int)strlen(sts), sig);
    char sighex[65];
    for (int i = 0; i < 32; i++) sprintf(sighex + i * 2, "%02x", sig[i]);
    sighex[64] = 0;
    snprintf(out, (size_t)outsz, "AWS4-HMAC-SHA256 Credential=%s/%s, SignedHeaders=%s, Signature=%s",
             ak, scope, signed_h, sighex);
}

// 构造并发送 S3 请求 → 返回 (status, body)
static int px_s3_exec(const char* endpoint, const char* method, const char* bucket, const char* key,
                      const char* query, const char* body, const char* ak, const char* sk,
                      char* body_out, int body_out_sz, char* errbuf, int errcap) {
    // URL 解析
    int is_https = 0;
    char host[256];
    int port = 80;
    const char* ep = endpoint;
    if (strncmp(ep, "https://", 8) == 0) { is_https = 1; ep += 8; port = 443; }
    else if (strncmp(ep, "http://", 7) == 0) ep += 7;
    else { px_net_fail(errbuf, errcap, "net: 不支持的协议: %s", endpoint); return 0; }
    const char* slash = strchr(ep, '/');
    int hl = slash ? (int)(slash - ep) : (int)strlen(ep);
    if (hl > 255) hl = 255;
    memcpy(host, ep, (size_t)hl); host[hl] = 0;
    char* colon = strchr(host, ':');
    if (colon) { port = atoi(colon + 1); *colon = 0; }
    char path[1024];
    snprintf(path, sizeof(path), "/%s/%s", bucket, key ? key : "");
    char pbody[4096];
    int blen = body ? (int)strlen(body) : 0;
    memcpy(pbody, body ? body : "", (size_t)blen);
    // 时间
    time_t now = time(NULL);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    char date_stamp[16], amz_date[32];
    snprintf(date_stamp, sizeof(date_stamp), "%04d%02d%02d", tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday);
    snprintf(amz_date, sizeof(amz_date), "%04d%02d%02dT%02d%02d%02dZ",
             tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday, tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    // payload hash
    char phash[65];
    px_sha256_hex(pbody, blen, phash);
    char auth[1024];
    px_sigv4(method, host, path, query, amz_date, date_stamp, phash, ak, sk, auth, sizeof(auth));
    // HTTP 请求
    char req[8192];
    int rlen = snprintf(req, sizeof(req),
        "%s %s%s%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n"
        "Content-Type: application/octet-stream\r\n"
        "x-amz-date: %s\r\nx-amz-content-sha256: %s\r\nAuthorization: %s\r\n"
        "Content-Length: %d\r\n\r\n",
        method, path, query[0] ? "?" : "", query,
        host, amz_date, phash, auth, blen);
    if (blen > 0 && rlen + blen < (int)sizeof(req)) {
        memcpy(req + rlen, pbody, (size_t)blen);
        rlen += blen;
    }
    HPoolSlot slot;
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
        px_net_fail(errbuf, errcap, "net: 连接 %s:%d 失败", host, port);
        if (slot.tls) https_close(slot.tls);
        return 0;
    }
    int status = 0, resp_len = 0, keep = 0;
    LXValue hdrs = px_null();
    char* resp = NULL;
    if (h_exchange(&slot, req, rlen, NULL, 0, &status, &hdrs, &resp, &resp_len, &keep) != 0) {
        if (slot.tls) https_close(slot.tls);
        close(slot.fd);
        px_net_fail(errbuf, errcap, "net: S3 请求失败: 连接中断");
        return 0;
    }
    if (body_out && resp && body_out_sz > 0) {
        int cp = resp_len < body_out_sz - 1 ? resp_len : body_out_sz - 1;
        memcpy(body_out, resp, (size_t)cp);
        body_out[cp] = 0;
    }
    if (resp) xfree(resp);
    if (slot.tls) https_close(slot.tls);
    close(slot.fd);
    return status;
}

static LXValue bi_s3_put(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 6) px_error("s3_put 需要 (endpoint, bucket, key, data, ak, sk) 参数");
    char out[8] = {0};
    char err[256] = {0};
    int st = px_s3_exec(val_cstr(args[0]), "PUT", val_cstr(args[1]), val_cstr(args[2]),
                        "", val_cstr(args[3]), val_cstr(args[4]), val_cstr(args[5]),
                        out, sizeof(out), err, (int)sizeof(err));
    if (st == 0) return px_net_err("%s", err[0] ? err : "net: S3 请求失败");
    return px_bool(st == 200 || st == 204);
}

static LXValue bi_s3_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 5) px_error("s3_get 需要 (endpoint, bucket, key, ak, sk) 参数");
    char* body = malloc(1048576);
    char err[256] = {0};
    int st = px_s3_exec(val_cstr(args[0]), "GET", val_cstr(args[1]), val_cstr(args[2]),
                        "", "", val_cstr(args[3]), val_cstr(args[4]),
                        body, 1048576, err, (int)sizeof(err));
    if (st == 0) { free(body); return px_net_err("%s", err[0] ? err : "net: S3 请求失败"); }
    LXValue r = (st == 200) ? px_str(body) : px_null();
    free(body);
    return r;
}

static LXValue bi_s3_delete(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 5) px_error("s3_delete 需要 (endpoint, bucket, key, ak, sk) 参数");
    char out[8] = {0};
    char err[256] = {0};
    int st = px_s3_exec(val_cstr(args[0]), "DELETE", val_cstr(args[1]), val_cstr(args[2]),
                        "", "", val_cstr(args[3]), val_cstr(args[4]),
                        out, sizeof(out), err, (int)sizeof(err));
    if (st == 0) return px_net_err("%s", err[0] ? err : "net: S3 请求失败");
    return px_bool(st == 204 || st == 200 || st == 404);
}

static LXValue bi_s3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 5) px_error("s3_list 需要 (endpoint, bucket, prefix, ak, sk) 参数");
    char query[512];
    snprintf(query, sizeof(query), "list-type=2&prefix=%s", val_cstr(args[2]));
    char* body = malloc(1048576);
    char err[256] = {0};
    int st = px_s3_exec(val_cstr(args[0]), "GET", val_cstr(args[1]), "",
                        query, "", val_cstr(args[3]), val_cstr(args[4]),
                        body, 1048576, err, (int)sizeof(err));
    if (st == 0) { free(body); return px_net_err("%s", err[0] ? err : "net: S3 请求失败"); }
    LXValue l = px_list(0);
    if (st == 200) {
        // 提取 <Key>...</Key>
        const char* p = body;
        while ((p = strstr(p, "<Key>")) != NULL) {
            p += 5;
            const char* e = strstr(p, "</Key>");
            if (!e) break;
            int klen = (int)(e - p);
            char key[1024];
            int kl = klen < 1023 ? klen : 1023;
            memcpy(key, p, (size_t)kl);
            key[kl] = 0;
            px_list_push(l, px_str(key));
            p = e + 6;
        }
    }
    free(body);
    return l;
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
    // M37：opts{retries, timeout_ms, proxy}
    int retries = 1;
    int timeout_ms = 30000;
    char proxy[256] = {0};
    if (nargs >= 5 && args[4].type == PX_DICT) {
        LXValue rv = px_dict_get(args[4], "retries");
        if (rv.type == PX_INT && rv.as.i >= 0) retries = (int)rv.as.i + 1;
        LXValue tv = px_dict_get(args[4], "timeout_ms");
        if (tv.type == PX_INT && tv.as.i > 0) timeout_ms = (int)tv.as.i;
        LXValue pv = px_dict_get(args[4], "proxy");
        if (pv.type == PX_STR) snprintf(proxy, sizeof(proxy), "%s", pv.as.obj->as.str.data);
    }
    int is_https = 0;
    char host[256];
    int port = 80;
    const char* path = "/";
    if (hparse_url(url, &is_https, host, sizeof(host), &port, &path) != 0)
        return px_net_err("net: 不支持的协议: %s", url);
    // M37：代理——连接代理地址，请求行用绝对 URL
    char conn_host[256];
    int conn_port = is_https ? 443 : 80;
    char req_target[1024];
    if (proxy[0]) {
        const char* pc = strrchr(proxy, ':');
        if (pc) {
            int pl = (int)(pc - proxy);
            if (pl > 0 && pl < 255) { memcpy(conn_host, proxy, (size_t)pl); conn_host[pl] = 0; }
            else snprintf(conn_host, sizeof(conn_host), "%s", proxy);
            conn_port = atoi(pc + 1);
        } else {
            snprintf(conn_host, sizeof(conn_host), "%s", proxy);
            conn_port = 8080;
        }
        snprintf(req_target, sizeof(req_target), "%s://%s:%d%s", is_https ? "https" : "http", host, port, path);
    } else {
        snprintf(conn_host, sizeof(conn_host), "%s", host);
        conn_port = port;
        snprintf(req_target, sizeof(req_target), "%s", path);
    }
    HPoolSlot slot;
    slot.is_tls = is_https;
    slot.fd = -1;
    slot.tls = NULL;
    if (is_https) { slot.tls = https_connect(host, port); slot.fd = slot.tls ? slot.tls->net.fd : -1; }
    else slot.fd = hconnect(host, port);
    if (slot.fd < 0) return px_net_err("net: 连接 %s:%d 失败", host, port);
    int fd = slot.fd;
    HttpsSession* tls = slot.is_tls ? slot.tls : NULL;
    char req[8192];
    int rlen = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n\r\n",
        path, host, port);
    if (conn_send(tls, fd, req, rlen) < 0) { if (tls) https_close(tls); else close(fd); return px_net_err("net: 发送请求失败"); }
    // 读响应头
    int cap = 16384, len = 0;
    char* buf = xmalloc(cap);
    int header_end = -1;
    for (;;) {
        if (len + 4096 > cap) { cap *= 2; buf = xrealloc(buf, cap); }
        int n = conn_recv(tls, fd, buf + len, 4096);
        if (n <= 0) { xfree(buf); if (tls) https_close(tls); else close(fd); return px_net_err("net: 读取响应失败"); }
        len += n;
        buf[len] = 0;
        char* sep = strstr(buf, "\r\n\r\n");
        if (sep) { header_end = (int)(sep - buf); break; }
        if (len > 65536) { xfree(buf); if (tls) https_close(tls); else close(fd); return px_net_err("net: 响应头超过 64KB"); }
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
    int* out_len, int* out_status, char* loc, int loc_cap,
    char* errbuf, int errcap) {
    int is_https = 0;
    const char* rest;
    if (strncmp(url, "https://", 8) == 0) { is_https = 1; rest = url + 8; }
    else if (strncmp(url, "http://", 7) == 0) { rest = url + 7; }
    else { px_net_fail(errbuf, errcap, "net: 不支持的协议: %s", url); return NULL; }

    char host[256];
    int hostlen = 0;
    while (rest[hostlen] && rest[hostlen] != '/' && rest[hostlen] != ':' && hostlen < 255) {
        host[hostlen] = rest[hostlen];
        hostlen++;
    }
    host[hostlen] = 0;
    if (hostlen == 0) { px_net_fail(errbuf, errcap, "net: 主机名为空"); return NULL; }
    int port = is_https ? 443 : 80;
    const char* p = rest + hostlen;
    if (*p == ':') {
        port = atoi(p + 1);
        if (port <= 0 || port > 65535) { px_net_fail(errbuf, errcap, "net: 端口非法"); return NULL; }
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
        if (r != 0) { px_net_fail(errbuf, errcap, "net: HTTPS 请求失败 (%d) %s", r, host); return NULL; }
    } else {
        // 明文 http
        struct addrinfo hints, *res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        char portstr[16];
        snprintf(portstr, sizeof(portstr), "%d", port);
        if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res) { px_net_fail(errbuf, errcap, "net: 解析主机失败 %s", host); return NULL; }
        int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0) { freeaddrinfo(res); px_net_fail(errbuf, errcap, "net: 创建 socket 失败"); return NULL; }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            freeaddrinfo(res);
            close(fd);
            px_net_fail(errbuf, errcap, "net: 连接 %s:%d 失败", host, port);
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
static char* px_http_request(const char* url, const char* method, const char* body, int* out_len, char* errbuf, int errcap) {
    char cur[2048];
    snprintf(cur, sizeof(cur), "%s", url);
    for (int i = 0; i < 5; i++) {
        char loc[1024];
        int status = 0;
        int len = 0;
        char* resp = px_http_once(cur, method, body, &len, &status, loc, sizeof(loc), errbuf, errcap);
        if (!resp) return NULL;  // 网络失败：errbuf 已填，上层 bi_http_get/post 包装 Err
        if (status >= 300 && status < 400 && loc[0]) {
            char next[2048];
            if (strncmp(loc, "http://", 7) == 0 || strncmp(loc, "https://", 8) == 0) {
                snprintf(next, sizeof(next), "%s", loc);
            } else if (loc[0] == '/') {
                const char* s = strstr(cur, "://");
                if (!s) { xfree(resp); px_net_fail(errbuf, errcap, "net: 非法 URL"); return NULL; }
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
    px_net_fail(errbuf, errcap, "net: 重定向次数过多（>5）");
    return NULL;
}

// http_get(url) → 响应体（支持 http/https，自动跟随重定向）
static LXValue bi_http_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("http_get 需要 (url) 参数");
    const char* url = args[0].as.obj->as.str.data;
    int len = 0;
    char err[256] = {0};
    char* body = px_http_request(url, "GET", NULL, &len, err, (int)sizeof(err));
    if (!body) return px_net_err("%s", err[0] ? err : "net: HTTP 请求失败");
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
    char err[256] = {0};
    char* resp = px_http_request(url, "POST", body, &len, err, (int)sizeof(err));
    if (!resp) return px_net_err("%s", err[0] ? err : "net: HTTP 请求失败");
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
        case 206: return "Partial Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 413: return "Payload Too Large";
        case 429: return "Too Many Requests";
        case 431: return "Request Header Fields Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        default: return "OK";
    }
}

// 构造 HTTP 响应报文（xmalloc，调用者 xfree）
// ==================== M21 gzip / chunked（HTTP 客户端 + 服务端） ====================

// gzip 压缩：10 字节头 + raw deflate + CRC32 + ISIZE。返回 xmalloc，调用者 xfree；失败返回 NULL。
static char* px_gzip_compress(const char* in, int inlen, int* outlen) {
    mz_stream s;
    memset(&s, 0, sizeof(s));
    int lvl = g_px_gzip_level;
    if (lvl < 1) lvl = 1;
    if (lvl > 9) lvl = 9;
    if (mz_deflateInit2(&s, lvl, MZ_DEFLATED, -15, 8, MZ_DEFAULT_STRATEGY) != MZ_OK) return NULL;
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

// ==================== M83-S2（Issue 20 GAP-ARC-1）：gzip 语言层通用压缩/解压 ====================
// gzip_compress(data) → bytes：标准 gzip 容器（1F 8B 头 + raw deflate + CRC32 + ISIZE），
//   与系统 gzip -9 / Go compress/gzip 互通（wsa-heal 健康快照 tar.gz、gen-update 差分包外层）。
// gzip_uncompress(gz) → bytes|null：解压失败（非 gzip / 截断 / 损坏）返回 null。
//   输入均兼容 str|bytes（bdata/blen 语义，含 NUL 不截断）；输出 bytes（任意二进制）。

static LXValue bi_gzip_compress(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("gzip_compress 需要 1 个参数: (data)");
    const char* in = bdata(args[0]);
    int inlen = blen(args[0]);
    int outlen = 0;
    char* out = px_gzip_compress(in, inlen, &outlen);
    if (!out) return px_null();
    LXValue r = px_bytes_len(out, outlen);
    xfree(out);
    return r;
}

static LXValue bi_gzip_uncompress(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("gzip_uncompress 需要 1 个参数: (gz)");
    const char* in = bdata(args[0]);
    int inlen = blen(args[0]);
    int outlen = 0;
    char* out = px_gzip_decompress(in, inlen, &outlen);
    if (!out) return px_null();
    LXValue r = px_bytes_len(out, outlen);
    xfree(out);
    return r;
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

// ==================== M83-S6（Issue 19）http_stream 流式路由全局表 ====================
// http_stream(path, on_connect) 注册 http_serve/http_serve_unix 同端口流式路由。
// 全局表须在 http_conn_worker（下方）使用前定义；函数实现见 SSE 区（bi_http_stream/
// stream_takeover_conn，定义在 g_sse_conns 注册表可用处之后）。
#define MAX_STREAM_ROUTES 64
static pthread_mutex_t g_stream_mu = PTHREAD_MUTEX_INITIALIZER;
static struct {
    char path[256];
    int active;
} g_stream_routes[MAX_STREAM_ROUTES];

// ==================== M88-B-S2（qg-issue 27 B 类）：http_conn_worker 请求级重构（空闲连接事件驱动） ====================
// 目标：keep-alive 空闲连接不占 worker——响应写完且无下一请求数据在途 → 连接交还 IDLE
// （事件循环 epoll 照看，可读再派发回 fserve 池；15s 空闲超时由事件循环 tick close）。
// 手段（侵入最小）：serve 连接 fd 一律非阻塞；请求读改用 px_recv_wait（poll+recv，语义 =
// 阻塞 recv + SO_RCVTIMEO 15s，非 Linux 同效）；连接收尾统一 px_evc_close（防 fd 复用串扰）。
// 事件驱动内核（px_evc_acquire/close/idle_put、px_ev_ensure、px_fd_nonblock、PxConnCtx）
// 定义在下方 M88-B-S1 区（本 worker 位于其前 → 前向声明；typedef 加标签同型）。
#ifndef FSERVE_KIND_HTTP
#define FSERVE_KIND_HTTP 0
#define FSERVE_KIND_SSE  1
#define FSERVE_KIND_PXSERVE 2   // M99：px_serve 连接（keep-alive 空闲交 IDLE，事件循环照看）
#endif
typedef struct PxConnCtx PxConnCtx;
static PxConnCtx* px_evc_acquire(int fd, int kind);
static void px_evc_close(int fd);
static int px_evc_idle_put(int fd, int kind);
// M99：px_serve 版交 IDLE（fd 保持阻塞；px_evc_idle_put 对 http_serve 强制 px_fd_nonblock）
static int px_evc_idle_put_fd(int fd, int kind, int nonblock);
static void px_ev_ensure(void);
static void px_fd_nonblock(int fd);
static void px_pxpend_close(int fd);   // M99：px_serve 连接统一 close（px_ev_loop tick/断开分支用；定义见 px_conn_worker 区）

// ---- M95-S2：http handler 协程化 —— pending 表前向声明（实现在 ConnCtx 区后）----
// 读+解析完成（段1）→ put(stage=1) + spawn handler 帧协程 → 完成回调写 resp
//   （stage=2）→ fserve_push 续处理；fserve worker 重入循环顶 take → 响应写（段2）。
// 锁 = g_hpend_mu（独立于 g_conn_mu，见定义处 fix 注释）。fd 超容量/不可用 → put 返回
//   0（调用方退回同步路径）。
static int  http_pend_put(int fd, LXValue req, int method_head, int client_close);
static int  http_pend_take(int fd, LXValue* req, LXValue* resp, int* method_head, int* client_close);
static void http_pend_clear(int fd);   // px_evc_close 收尾调用（防 fd 复用串扰）
static void http_pend_gc_mark(void);   // gc 标记期补标 req/resp
// handler 完成回调（coro worker 线程执行；ret 已 PX_KEEP 约定由本函数内处理）
static void http_handler_done(void* ud, LXValue ret);

// 非阻塞 fd 安全读：recv 遇 EAGAIN → poll 等待 tmo_ms → 再 recv（可读/断开/错误均再 recv 一次）。
// 语义 = 阻塞 recv + SO_RCVTIMEO（服务端空闲超时 15s 对齐原 http_conn_worker）。返回 recv 结果：
//   >0 读得字节 / 0 对端关闭 / -1 错误或超时（超时 errno=EAGAIN，与 SO_RCVTIMEO 行为一致）
static ssize_t px_recv_wait(int fd, char* buf, size_t len, int tmo_ms) {
    for (;;) {
        ssize_t n = recv(fd, buf, len, 0);
        if (n >= 0) return n;
        if (errno == EINTR) continue;
        if (errno != EAGAIN && errno != EWOULDBLOCK) return -1;
        struct pollfd pfd;
        pfd.fd = fd; pfd.events = POLLIN; pfd.revents = 0;
        int r = poll(&pfd, 1, tmo_ms);
        if (r == 0) { errno = EAGAIN; return -1; }   // 超时（空闲）
        if (r < 0) { if (errno == EINTR) continue; return -1; }
        // 可读/挂起/对端关闭 → 再 recv（返回 0 = EOF）
    }
}

// 当前是否有请求数据在途（0 超时探测；keep-alive 交还 IDLE 决策用）
static int px_fd_readable_now(int fd) {
    struct pollfd pfd;
    pfd.fd = fd; pfd.events = POLLIN; pfd.revents = 0;
    int r = poll(&pfd, 1, 0);
    return r > 0 && (pfd.revents & (POLLIN | POLLHUP | POLLERR));
}

// M99：px_serve 连接「下一请求数据在途」判定（keep-alive 交 IDLE 决策用）。
//   TLS：PxConn 内部缓冲（mbedtls_ssl_read 一次读整 TLS record 未消费完，roff<rlen）有数据
//   → 在途（epoll 只照看内核 fd，缓冲残留对 epoll 不可见——漏判则交 IDLE 后缓冲中的下一
//   请求永不处理，悬挂至 15s 空闲超时）；明文：poll fd 探测（同 http_serve）。
static int px_pxserve_inflight_data(PxConn* conn, int fd) {
    if (conn && conn->is_tls && conn->roff < conn->rlen) return 1;
    return px_fd_readable_now(fd);
}

// M99：px_serve 响应写完后的 keep-alive 空闲交 IDLE 决策。返回 1 = 已交 IDLE（调用方 return
//   释放 worker，连接由事件循环照看）；0 = 下一请求在途 或 事件循环不可用（非 Linux/未登记）
//   → 调用方走 for(;;) 顶部原阻塞续读（功能不降）。
static int px_pxserve_idle_after_resp(PxConn* conn, int fd) {
    if (!px_pxserve_inflight_data(conn, fd)) {
        px_ev_ensure();
        if (px_evc_idle_put_fd(fd, FSERVE_KIND_PXSERVE, 0) == 0) return 1;
    }
    return 0;
}

// M97-S3（qg-issue 32）：非阻塞 fd 全量写——循环 send，EAGAIN/EWOULDBLOCK →
//   poll(POLLOUT, 15s 写超时) 等可写续写；EINTR 重试；EPIPE/ERR/超时 → -1。
//   修复前 http_send_resp 等用单次裸 send 不检查返回值：非阻塞 fd 高水位时 send
//   返回 EAGAIN 或部分字节 → 响应截断且不重试 → 交还 IDLE 后事件循环只等可读，
//   剩余响应永不写出 → 悬挂至 15s 空闲超时 close（清歌 token-cache 1s fail-closed
//   误报根因）。全量入内核后才交还 IDLE，事件循环无需关心写侧。
//   调用方（worker 线程）：写失败返回 -1 → 连接已不可续（对端关/半关/持续不读），
//   应收尾 px_evc_close，不得进 keep-alive 判定。
static int px_send_all(int fd, const char* data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t w = send(fd, data + off, len - off, 0);
        if (w > 0) { off += (size_t)w; continue; }
        if (w < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) return -1;
        if (w < 0 && errno == EINTR) continue;
        // EAGAIN / 部分写 0：等可写（15s 写超时——对端持续不读 → 放弃，防 worker 无限挂）
        struct pollfd pfd;
        pfd.fd = fd; pfd.events = POLLOUT; pfd.revents = 0;
        int r = poll(&pfd, 1, 15000);
        if (r <= 0) return -1;   // 超时 / poll 错误
        if (pfd.revents & (POLLERR | POLLNVAL)) return -1;
        if (pfd.revents & POLLOUT) continue;   // 可写 → 再 send
        return -1;
    }
    return 0;
}

// 连接处理线程（px_spawn 注册）：args[0] = fd
// ==================== M23c HTTP 服务端 keep-alive（双模式：与解释器 builtin.rs 一致） ====================
static const char* px_file_content_type(const char* path);
// 同一连接循环处理多个请求：HTTP/1.1 默认 keep-alive；客户端 Connection: close、
// handler 返回 keep_alive:false、或空闲超时(15s) → 关闭。handler 返回 dict 支持
// "file": path（流式文件响应，大文件不占内存）。
// ==================== M95-S2（D8-②）：http_serve 系 handler 协程化 ====================
// http_conn_worker 拆段：读+解析（段1）→ VM handler 以帧协程执行（chan/sleep 让出
//   占协程不占 fserve worker）→ 完成回调（http_handler_done，coro worker 线程）写
//   pending 表 + fserve_push 投回 → 本函数重入循环顶 take → http_send_resp（段2
//   响应写 + keep-alive 决策）。PX_NATIVE / 非 VM handler（逃生舱）→ 原同步直调。
// 语义：handler 挂起期连接挂起（同原线程模型 handler 占用期）；空闲交还 IDLE 事件
//   循环照看（M88-B）；请求读仍事件化兜底（job 派发时数据在途，快速）。

// 响应写 + keep-alive 决策（原 http_conn_worker step7-8 抽出；同步/协程续处理共用）。
// 返回：0 = 连接已收尾（px_evc_close 已调，调用方 return）；1 = 已交还 IDLE 事件
//       循环（调用方 return）；2 = 连接可继续读下一请求（调用方 continue）。
static int http_send_resp(int fd, LXValue req, LXValue resp, int method_head, int client_close) {
    // 7. 响应：file 流式（Connection: close，发送后关闭）或普通（keep-alive 判定）
    LXValue file_v = (resp.type == PX_DICT) ? px_dict_get(resp, "file") : px_null();
    if (file_v.type == PX_STR) {
        // 流式文件响应
        const char* fpath = file_v.as.obj->as.str.data;
        FILE* f = fopen(fpath, "rb");
        int werr = 0;
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsz = ftell(f);
            fseek(f, 0, SEEK_SET);
            const char* ct2 = px_file_content_type(fpath);
            char hdr[1024];
            int hl = snprintf(hdr, sizeof(hdr),
                              "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: %s\r\nConnection: close\r\n\r\n",
                              fsz, ct2);
            // M97-S3：全量写（非阻塞 fd 高水位 EAGAIN/部分写 → poll POLLOUT 续写）
            if (hl > 0 && px_send_all(fd, hdr, (size_t)hl) != 0) werr = 1;
            char fbuf[65536];
            size_t rd;
            while (!werr && (rd = fread(fbuf, 1, sizeof(fbuf), f)) > 0) {
                if (px_send_all(fd, fbuf, rd) != 0) { werr = 1; break; }
            }
            fclose(f);
        } else {
            const char* notfound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            if (px_send_all(fd, notfound, strlen(notfound)) != 0) werr = 1;
        }
        px_evc_close(fd);
        return 0;
    }
    int out_len = 0;
    int resp_keep_alive = 1;
    char* out = px_http_build_response(resp, &out_len, &resp_keep_alive);
    if (out) {
        if (method_head) {
            char* sep = strstr(out, "\r\n\r\n");
            if (sep) out_len = (int)(sep - out) + 4;
        }
        // M97-S3：响应字节全量入内核后才交还 IDLE（单次裸 send → px_send_all 循环写；
        //   修复前非阻塞 fd 高水位 EAGAIN/部分写 → 响应截断，交 IDLE 后剩余永不写出）
        if (out_len > 0 && px_send_all(fd, out, (size_t)out_len) != 0) {
            xfree(out);
            px_evc_close(fd);   // 写失败：对端关/半关/持续不读 → 收尾，不留半写连接进 keep-alive
            return 0;
        }
        xfree(out);
    }
    // 8. keep-alive 判定：需关闭 → 统一 px_evc_close（清理连接上下文，防 fd 复用串扰）
    if (client_close || !resp_keep_alive) {
        px_evc_close(fd);
        return 0;
    }
    // M88-B-S2：空闲连接不占 worker——无下一请求数据在途 → 连接交还 IDLE。
    // 事件循环（epoll）照看空闲连接：可读再派发回 fserve 池（worker 接管下一请求突发）；
    // 15s 空闲超时 / 对端断开由事件循环 tick close（语义与原 SO_RCVTIMEO 对齐）。
    // 交还成功即返回释放本 worker 去取新 job；失败（非 Linux / fd 未登记）→ 继续读下一请求
    // （px_recv_wait 15s 超时 = 原空闲语义，功能不降仅无事件驱动优化）。
    if (!px_fd_readable_now(fd)) {
        px_ev_ensure();
        if (px_evc_idle_put(fd, FSERVE_KIND_HTTP) == 0) return 1;
    }
    return 2;
}

static LXValue http_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // M88-B-S2：serve 连接一律非阻塞 + 登记连接上下文（FREE→ACTIVE）。fd 超 PX_MAX_CONNS
    // 或非 Linux 时 acquire 返回 NULL（不登记）→ 后续 px_evc_idle_put 失败走阻塞续读路径，功能不降。
    px_fd_nonblock(fd);
    px_evc_acquire(fd, FSERVE_KIND_HTTP);
    // keep-alive 空闲超时语义：非阻塞 fd 上 SO_RCVTIMEO 不生效，由 px_recv_wait 的 15s poll 等待取代
    // （IDLE 连接空闲超时由 B-S1 事件循环 tick 同样 15s 对齐）。

    for (;;) {
        // M95-S2：handler 协程完成待响应（finish 续处理）→ 先于读下一请求执行。
        //   take 后项已出表（GC 根失效）→ 临时 push 保活 req/resp 至 http_send_resp 完。
        {
            LXValue fr = px_null(), fs = px_null();
            int fh = 0, fc = 0;
            if (http_pend_take(fd, &fr, &fs, &fh, &fc)) {
                px_root_push();
                PX_KEEP(fr); PX_KEEP(fs);
                int act = http_send_resp(fd, fr, fs, fh, fc);
                px_root_pop();
                if (act == 0 || act == 1) return px_null();   // close / 已交还 IDLE
                continue;                  // act==2：下一请求在途 → 继续读
            }
        }
        // 1. 读请求头（直到 \r\n\r\n，上限 64KB；EOF/超时 → 关闭）
        char buf[65536];
        int len = 0;
        int header_end = -1;
        while (len < (int)sizeof(buf) - 1) {
            ssize_t n = px_recv_wait(fd, buf + len, (size_t)((int)sizeof(buf) - 1 - len), 15000);
            if (n == 0) { px_evc_close(fd); return px_null(); }    // 对端关闭
            if (n < 0) { px_evc_close(fd); return px_null(); }     // 空闲超时(15s)/错误
            len += (int)n;
            buf[len] = 0;
            char* sep = strstr(buf, "\r\n\r\n");
            if (sep) { header_end = (int)(sep - buf); break; }
        }
        if (header_end < 0 || len == 0) { px_evc_close(fd); return px_null(); }

        // 2. 解析请求行：METHOD SP target SP version
        char* head = buf;
        char* sp1 = strchr(head, ' ');
        if (!sp1) { px_evc_close(fd); return px_null(); }
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
        px_root_push();   // M92-S2c precise：http_conn_worker 请求迭代登记作用域开始
        LXValue headers = px_dict();
        PX_KEEP(headers);   // M92-S2c precise：headers 裸局部跨 px_dict_set/px_str 分配
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

        // 4. 读 body（Content-Length）——M83-S1（Issue 16 GAP-SRV-2）：
        //    固定 64KB 栈缓冲（content_length>65535 静默截断且不报错 → handler 拿残缺 body
        //    = 静默数据损坏）→ 动态缓冲（xmalloc 跟随 content_length）；上限默认 256MB、
        //    PX_HTTP_BODY_MAX 环境变量可配；超限返回 413（不再静默截断）。
        //    与客户端 http_request 动态读（M72-S4）对称；http_serve 与 http_serve_unix
        //    共享本 worker，一处改两入口通。
        char* body_buf = NULL;
        int body_len = 0;
        if (content_length > 0) {
            int body_max = 256 * 1024 * 1024;
            const char* bm_env = getenv("PX_HTTP_BODY_MAX");
            if (bm_env && atol(bm_env) > 0) {
                long bmv = atol(bm_env);
                body_max = bmv > 0x7fffffffL ? 0x7fffffff : (int)bmv;
            }
            if (content_length > body_max) {
                const char* r413 = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                px_send_all(fd, r413, strlen(r413));   // M97-S3：全量写（尽力；随即 close）
                px_evc_close(fd);
                px_root_pop();   // M92-S2c precise
                return px_null();
            }
            body_buf = xmalloc((size_t)content_length + 1);
            int body_off = header_end + 4;
            int have = len - body_off;
            if (have > 0) {
                int take = have < content_length ? have : content_length;
                memcpy(body_buf, buf + body_off, (size_t)take);
                body_len = take;
            }
            while (body_len < content_length) {
                ssize_t n = px_recv_wait(fd, body_buf + body_len, (size_t)(content_length - body_len), 15000);
                if (n <= 0) break;   // 超时/对端关闭 → 按已收 body 处理（原 SO_RCVTIMEO 语义一致）
                body_len += (int)n;
            }
            if (body_len > content_length) body_len = content_length;
            body_buf[body_len] = 0;
        }

        // 5. 构造请求 dict
        LXValue req = px_dict();
        PX_KEEP(req);   // M92-S2c precise：req 裸局部跨 px_dict_set/px_str/px_call（崩点修复）
        px_dict_set(req, "method", px_str(method));
        px_dict_set(req, "target", px_str(target));
        px_dict_set(req, "path", px_str(path));
        px_dict_set(req, "query", px_str(query));
        px_dict_set(req, "version", px_str(version));
        px_dict_set(req, "headers", headers);
        px_dict_set(req, "body", body_buf ? px_str_len(body_buf, body_len) : px_str("")); // M83-S1：body_buf NULL(无 body) → 空串（避免 px_str_len(NULL,0) UB）
        LXValue form = px_dict();
        PX_KEEP(form);   // M92-S2c precise：form 裸局部跨 px_dict_set/px_str 分配
        {
            // M8x：remote 兼容 AF_UNIX（http_serve_unix 连接无 IP）——sockaddr_storage 判族，
            // AF_INET → ip:port（http_serve 原行为）；AF_UNIX → "unix"（客户端 peer 无 sun_path）
            struct sockaddr_storage raddr;
            memset(&raddr, 0, sizeof(raddr));
            socklen_t rl = sizeof(raddr);
            if (getpeername(fd, (struct sockaddr*)&raddr, &rl) != 0) {
                px_dict_set(req, "remote", px_str(""));
            } else if (raddr.ss_family == AF_INET) {
                struct sockaddr_in* rin = (struct sockaddr_in*)&raddr;
                char rbuf[64];
                snprintf(rbuf, sizeof(rbuf), "%s:%d", inet_ntoa(rin->sin_addr), ntohs(rin->sin_port));
                px_dict_set(req, "remote", px_str(rbuf));
            } else if (raddr.ss_family == AF_UNIX) {
                px_dict_set(req, "remote", px_str("unix"));
            } else {
                px_dict_set(req, "remote", px_str(""));
            }
        }
        // M33：访问日志统一 remote 字段（与解释器 log_access 一致：时间 remote method path status bytes ms req=id）
        const char* log_remote = "unknown";
        {
            LXValue lr = px_dict_get(req, "remote");
            if (lr.type == PX_STR) log_remote = lr.as.obj->as.str.data;
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

        // 5.5（M83-S6 / Issue 19 GAP-SRV-SSE）：http_stream 流式路由优先——
        //    命中（GET）→ 连接转 SSE 通道（复用 sse_send/sse_close），不再走普通 handler/keep-alive。
        //    http_serve 与 http_serve_unix 共享本 worker → 两入口同享。
        if (strcmp(method, "GET") == 0) {
            pthread_mutex_lock(&g_stream_mu);
            int s_idx = stream_match(path);
            pthread_mutex_unlock(&g_stream_mu);
            if (s_idx >= 0) {
                if (body_buf) { xfree(body_buf); body_buf = NULL; }
                LXValue _s2c_sr = stream_takeover_conn(fd, req, s_idx);
                px_root_pop();   // M92-S2c precise
                return _s2c_sr;
            }
        }

        // 6. 调 handler（M95-S2：VM handler → 帧协程异步执行占协程，fserve worker 释放）
        LXValue handler = px_get_global("__http_handler");
        int hvm = (handler.type == PX_FUNC && px_vm_entry &&
                   handler.as.obj->as.func.fn == px_vm_entry);
        int head_flag = (strcmp(method, "HEAD") == 0) ? 1 : 0;
        if (hvm) {
            // M83-S1：body 缓冲用毕即释放（req.body 经 px_str_len 已深拷贝、multipart/form
            // 解析已入 req；handler 异步执行期 body_buf 无引用）——防 keep-alive 长连接累积
            if (body_buf) { xfree(body_buf); body_buf = NULL; }
            // 登记挂起项（stage=1，req 入 GC 根表）→ 成功则 spawn handler 帧协程并释放
            //   本 worker（handler 内 chan/sleep/spawn 让出占协程；完成回调投回续处理）。
            if (!http_pend_put(fd, req, head_flag, client_close)) {
                // 登记失败（fd 超容量等）→ 退回原同步路径（行为零变化）
                LXValue resp = px_null();
                if (handler.type == PX_FUNC || handler.type == PX_NATIVE)
                    resp = px_call(handler, &req, 1);
                PX_KEEP(resp);
                int act = http_send_resp(fd, req, resp, head_flag, client_close);
                px_root_pop();
                if (act == 2) continue;          // 下一请求在途 → 继续迭代
                return px_null();                // close(0) / 已交还 IDLE(1)
            }
            if (px_coro_spawn_ex)
                px_coro_spawn_ex(handler.as.obj->as.func.ctx, &req, 1,
                                 http_handler_done, (void*)(intptr_t)fd);
            else
                http_pend_clear(fd);             // 无协程内核（理论不达）→ 清项兜底
            px_root_pop();
            return px_null();                    // worker 释放；完成回调投回续处理
        }
        // —— 逃生舱（PX_NATIVE / 非 VM handler）：原同步直调路径（行为零变化）——
        LXValue resp = px_null();
        if (handler.type == PX_FUNC || handler.type == PX_NATIVE) {
            resp = px_call(handler, &req, 1);
        }
        PX_KEEP(resp);   // M92-S2c precise：px_call 返回值裸局部（后续构建响应可能 GC）
        if (body_buf) { xfree(body_buf); body_buf = NULL; }
        int act2 = http_send_resp(fd, req, resp, head_flag, client_close);
        px_root_pop();
        if (act2 == 2) continue;                 // 下一请求在途 → 继续迭代
        return px_null();                        // close(0) / 已交还 IDLE(1) 均已收尾
    }
    px_evc_close(fd);
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


// ==================== M88-S2（qg-issue 27）：函数式 serve 连接线程池 ====================
// http_serve / http_serve_unix / sse_serve 原 accept 循环每连接 px_spawn（一请求一线程），
// 并发连接数逼近 spawn 槽上限即 px_error → 服务进程 exit(1)（qg-issue 27 线上事故根因）。
// 现改为预派生常驻 worker 池（与 px_serve 的 M31.4b 池同构，独立于其 g_pool_*，不互相干扰）：
//   accept 只把 (cfd, kind) 投递环形队列（队满阻塞 → 背压到 TCP backlog，绝不 exit）；
//   worker 取 job 按 kind 调 http_conn_worker / sse_conn_worker（处理语义与既有逐字节一致）。
// 池容量 env PX_SERVE_WORKERS（默认 256，夹取 [2,4095]；M95-S2 下限 8→2 见 fserve_ensure）。
// 池 worker 均注册 GC 槽（同
// px_pool_worker 常驻模式），注意 PX_MAX_THREADS 需 ≥ PX_SERVE_WORKERS + 主线程 + 业务 spawn。
// M88-B-S2：http 连接已事件驱动——keep-alive 空闲连接响应写完即交还 IDLE（事件循环照看），
// 不再占 worker 至 15s 超时（见 http_conn_worker 改造）。
// M88-B-S3：SSE 长连接已事件化——handler 返回后明文连接同样交还 IDLE（事件循环照看断开，
// SSE 空闲不超时），worker 释放；仅 TLS SSE 长连接仍占 worker（走原阻塞保持路径，文档注明）。
#ifndef FSERVE_KIND_HTTP
#define FSERVE_KIND_HTTP 0
#define FSERVE_KIND_SSE  1
#define FSERVE_KIND_PXSERVE 2   // M99：px_serve 连接（keep-alive 空闲交 IDLE，事件循环照看）
#endif
#define FSERVE_DEFAULT_WORKERS 256
#define FSERVE_QUEUE_CAP 16384        // 环形队列容量（job 8B → 128KB 背压缓冲）
typedef struct { int fd; int kind; } FServeJob;
static pthread_t* g_fserve_threads = NULL;
static FServeJob* g_fserve_queue = NULL;
static int g_fserve_qcap = 0;
static int g_fserve_head = 0, g_fserve_tail = 0, g_fserve_count = 0;
static int g_fserve_workers = 0;
static int g_fserve_inited = 0;
static pthread_mutex_t g_fserve_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_fserve_cond = PTHREAD_COND_INITIALIZER;

static LXValue sse_conn_worker(LXValue* args, int nargs, void* ctx);  // M88-S2：池 dispatch 前向

// F3-fix（M90-F3 根因修复）：fserve_worker 空闲（cond_wait 等 job）时注销 GC 槽位。
// 背景：事件化 keep-alive（M88-B-S1）后 256 个 worker 绝大多数时间空闲 cond_wait；
//   M11 并发 GC（stop-the-world）每次回收需暂停全部已注册线程——254 个空闲 worker
//   在 cond_wait 中被 SIG_GC_STOP 暂停/自旋/恢复的协商开销巨大，且被暂停 worker 若
//   卡在 cond_wait 锁窗口会让事件循环 fserve_push 长时间拿不到 g_fserve_mu → 连接
//   请求 15s 无人处理（F3 实测根因：RPS 218 vs 禁 GC 后 1400）。空闲 worker 栈上无
//   px 对象（job 仅 fd+kind int）→ 不参与 GC 暂停安全；取到 job 处理前注册（处理中
//   创建 px 对象需 GC 根面）。注册/注销持 g_gc_mu（GC executor 亦持 → 天然互斥）。
static void fserve_gc_reg(int on) {
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    pthread_t me = pthread_self();
    if (on) {
        g_active_threads++;
        for (int i = 0; i < g_thread_cap; i++) if (!g_threads[i].in_use) {
            g_threads[i].tid = me;
            g_threads[i].in_use = 1;
            g_threads[i].paused = 0;
            g_threads[i].is_main = 0;
            g_threads[i].epoch = 0;
            g_threads[i].tmp_root = NULL;
            break;
        }
    } else {
        for (int i = 0; i < g_thread_cap; i++)
            if (g_threads[i].in_use && pthread_equal(g_threads[i].tid, me)) {
                g_threads[i].in_use = 0;
                if (g_active_threads > 0) g_active_threads--;
                break;
            }
    }
    pthread_mutex_unlock(&g_gc_mu);
}

static void* fserve_worker(void* arg) {
    (void)arg;
    // F3-fix：初始不注册 GC 槽（空闲）；取到 job 处理前 fserve_gc_reg(1) 注册。
    for (;;) {
        pthread_mutex_lock(&g_fserve_mu);
        while (g_fserve_count == 0) pthread_cond_wait(&g_fserve_cond, &g_fserve_mu);
        FServeJob job = g_fserve_queue[g_fserve_head];
        g_fserve_head = (g_fserve_head + 1) % g_fserve_qcap;
        g_fserve_count--;
        pthread_cond_broadcast(&g_fserve_cond);   // 唤醒阻塞在 push 的 accept 线程
        pthread_mutex_unlock(&g_fserve_mu);
        fserve_gc_reg(1);   // F3-fix：取到 job → 注册 GC 槽（处理中持 px 对象，需 GC 根）
        LXValue arg = px_int(job.fd);
        if (job.kind == FSERVE_KIND_SSE) sse_conn_worker(&arg, 1, NULL);
        else http_conn_worker(&arg, 1, NULL);
        px_gc_poll();       // ISSUE28-B1：请求间安全点回收（本 worker 仍注册，空闲前跑 GC）
        fserve_gc_reg(0);   // F3-fix：处理完 → 注销（回 cond_wait 空闲，不参与 GC 暂停）
    }
    return NULL;  // 不可达（进程退出由 OS 回收）
}

// 队列满时阻塞等待空位（背压）：调用方为 accept 线程；绝不 exit
static void fserve_push(int fd, int kind) {
    pthread_mutex_lock(&g_fserve_mu);
    while (g_fserve_count >= g_fserve_qcap) pthread_cond_wait(&g_fserve_cond, &g_fserve_mu);
    g_fserve_queue[g_fserve_tail].fd = fd;
    g_fserve_queue[g_fserve_tail].kind = kind;
    g_fserve_tail = (g_fserve_tail + 1) % g_fserve_qcap;
    g_fserve_count++;
    pthread_cond_signal(&g_fserve_cond);
    pthread_mutex_unlock(&g_fserve_mu);
}

// 懒初始化：首个 serve 入口调用时建池（幂等；持 g_fserve_mu 下创建，worker 启动后
// 阻塞在 cond_wait 内部释放本锁 → 无死锁）
static void fserve_ensure(void) {
    if (g_fserve_inited) return;
    pthread_mutex_lock(&g_fserve_mu);
    if (!g_fserve_inited) {
        int workers = FSERVE_DEFAULT_WORKERS;
        const char* we = getenv("PX_SERVE_WORKERS");
        if (we) {
            int w = atoi(we);
            // M95-S2：下限 8→2 —— http_serve handler 协程化后（D8-②）长业务请求占
            //   协程不占 fserve worker，显式小池（2 worker 起）即可承载并发长业务；
            //   默认仍 FSERVE_DEFAULT_WORKERS。逃险舱（PX_NATIVE handler 同步直调）
            //   占 worker 语义不变，显式小池由用户在知晓 handler 形态下配置。
            if (w >= 2 && w <= 4095) workers = w;
        }
        g_fserve_workers = workers;
        g_fserve_qcap = FSERVE_QUEUE_CAP;
        g_fserve_threads = (pthread_t*)xcalloc((size_t)workers, sizeof(pthread_t));
        g_fserve_queue = (FServeJob*)xcalloc((size_t)FSERVE_QUEUE_CAP, sizeof(FServeJob));
        for (int i = 0; i < workers; i++) {
            if (pthread_create(&g_fserve_threads[i], NULL, fserve_worker, NULL) != 0) {
                // 创建失败不致命：保留已建 worker；容量不足时 accept 背压，绝不 exit
                fprintf(stderr, "[px-serve] 函数式连接池建 worker 失败（已建 %d/%d），连接将排队背压\n", i, workers);
                break;
            }
            pthread_detach(g_fserve_threads[i]);
        }
        g_fserve_inited = 1;
    }
    pthread_mutex_unlock(&g_fserve_mu);
}

// http_serve(port, handler)：阻塞 accept 循环（Go 风格），连接交池 worker 处理

// ==================== M88-B-S1（qg-issue 27 B 类）：连接上下文表 + 事件循环内核 ====================
// 目标（M88_PLAN §三）：keep-alive / SSE 空闲连接不占 worker 线程，由事件循环照看——
//   worker 处理完一个请求突发 → px_evc_idle_put(fd) 把连接交还 IDLE（事件循环等 可读/断开/超时）；
//   事件循环 detect 可读 → 摘除（IDLE → 派发）并投回 fserve 队列（worker 再接管处理下一请求突发）；
//   对端断开 / HUP / 空闲超时（15s，沿用 http_conn_worker SO_RCVTIMEO 的 keep-alive 空闲语义）→ close 清理。
// 连接状态机（单持有者原则：同一 fd 任一时刻只一个持有者，杜绝双读 / fd 复用串扰）：
//   FREE（无主）→ ACTIVE（worker 独占处理）→ IDLE（事件循环照看）→（可读）派发 → ACTIVE → …
//   任意态收尾（close/超时/HUP）必经 FREE，防 fd 复用后串扰旧上下文。
// S1 落地本内核（ConnCtx 表 + 状态机 + 事件循环线程 + 接口）；http_conn_worker 请求级接入在
// B-S2、SSE 在 B-S3（见 docs/M88_PLAN.md §三）。未接入前事件循环不启动、行为零变化。
#if defined(__linux__)
#include <sys/epoll.h>
// M88-B-S3：SSE 长连接事件化辅助（实现在 SSE 服务端区）——事件循环断开检测 + 统一关闭清理
static int sse_idle_should_close(int fd);
static void sse_server_close_fd(int fd);
#endif

#define PX_CONN_STATE_FREE   0
#define PX_CONN_STATE_ACTIVE 1
#define PX_CONN_STATE_IDLE   2

// 连接空闲超时（ms）：沿用 http_conn_worker 的 SO_RCVTIMEO 15s keep-alive 空闲语义
#define PX_CONN_IDLE_TMO_MS  15000
// 事件循环 tick（ms）：周期性醒来统一扫空闲超时（epoll 无每 fd 定时器，不做复杂最小堆）
#define PX_EV_TICK_MS        1000
// 默认最大并发连接登记（fd 索引表容量上限，env PX_MAX_CONNS 可配 [1024, 131072]）
#define PX_CONN_DEFAULT_MAX  16384

typedef struct PxConnCtx {
    int fd;                 // 连接 fd；FREE 时为 -1
    int kind;               // FSERVE_KIND_HTTP / FSERVE_KIND_SSE（派发回 fserve 用）
    int state;              // FREE / ACTIVE / IDLE
    int ev_reg;             // 是否已注册进事件循环（IDLE 必真；防重复 epoll_ctl ADD/DEL）
    long long idle_since;   // 进入 IDLE 的单调毫秒（空闲超时检查用）
    // F3-fix 根因深查插桩：本 IDLE 周期内事件循环 epoll_wait 返回该 fd 的次数
    // （观测「漏报活跃连接」到底 epoll 报没报；仅诊断打印用，正常路径零读取）
    int idle_ev_cnt;
    char* pbuf;             // 半请求续接缓冲（B-S2 非阻塞续接启用；S1 预留）
    int pbuf_len, pbuf_cap;
} PxConnCtx;

static pthread_mutex_t g_conn_mu = PTHREAD_MUTEX_INITIALIZER;   // 保护 g_conns 表 + 状态转移
static PxConnCtx* g_conns = NULL;       // fd 索引表（g_conn_cap 个；fd 超上限不登记走原路径）
static int g_conn_cap = 0;
static int g_conn_max = PX_CONN_DEFAULT_MAX;
static int g_conn_max_init = 0;         // 已从 env 读 PX_MAX_CONNS
static int g_ev_epfd = -1;              // Linux: epoll fd（ensure 时建，g_conn_mu 保护 epoll_ctl）
static pthread_t g_ev_thread;
static volatile int g_ev_run = 0;       // 事件循环运行标志
static int g_ev_wakefd = -1;            // 唤醒管道读端（写端 g_ev_wakew；注册/摘除后唤醒重算）
static int g_ev_wakew = -1;
// F3-fix（M90-F3 / qg-issue）：事件驱动 keep-alive 诊断计数器——观测事件循环
// 「交 IDLE → detect → 派发」链路的漏报规模（PX_EV_DIAG=1 时事件循环每 ~5s 打一行 stderr）。
// 计数用 __atomic 宽松自增（仅诊断，不要求精确）；生产（未开 PX_EV_DIAG）零输出、近零开销。
static volatile long long g_diag_idle_put = 0;    // worker 交 IDLE 成功次数（px_evc_idle_put）
static volatile long long g_diag_detect_http = 0; // 事件循环 detect HTTP IDLE 可读并投回 fserve 次数
static volatile long long g_diag_tmo_close = 0;   // 空闲超时关闭次数（poll 确认真空闲）
static volatile long long g_diag_tmo_save = 0;    // 空闲超时救回次数（poll 见在途数据 = 漏报活跃连接）
static int g_ev_diag = 0;                          // PX_EV_DIAG=1 开诊断（px_ev_ensure 启动时读一次）
static long long g_diag_last_log = 0;              // 上次诊断打印时刻（单调 ms）

// 单调毫秒（空闲超时计时；不进 px_now_ms 以免依赖其后定义）
static long long px_ev_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// fd 置非阻塞（连接交还事件循环前调用；B-S2 起 serve 连接一律非阻塞读）
static void px_fd_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl >= 0 && !(fl & O_NONBLOCK)) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

// fd 索引表扩容到至少容纳 idx=fd（上限 g_conn_max；超限返回 -1）。调用方持 g_conn_mu。
static int px_evc_ensure(int fd) {
    if (!g_conn_max_init) {
        g_conn_max_init = 1;
        const char* e = getenv("PX_MAX_CONNS");
        if (e) {
            int v = atoi(e);
            if (v >= 1024 && v <= 131072) g_conn_max = v;
        }
    }
    if (fd < 0 || fd >= g_conn_max) return -1;
    if (fd < g_conn_cap) return 0;
    int ncap = g_conn_cap ? g_conn_cap : 256;
    while (ncap <= fd && ncap < g_conn_max) ncap *= 2;
    if (ncap > g_conn_max) ncap = g_conn_max;
    if (ncap <= fd) return -1;
    PxConnCtx* nc = (PxConnCtx*)xrealloc(g_conns, (size_t)ncap * sizeof(PxConnCtx));
    if (!nc) return -1;
    for (int i = g_conn_cap; i < ncap; i++) {
        nc[i].fd = -1; nc[i].kind = 0; nc[i].state = PX_CONN_STATE_FREE;
        nc[i].ev_reg = 0; nc[i].idle_since = 0;
        nc[i].idle_ev_cnt = 0;
        nc[i].pbuf = NULL; nc[i].pbuf_len = 0; nc[i].pbuf_cap = 0;
    }
    g_conns = nc; g_conn_cap = ncap;
    return 0;
}

// ==================== M95-S2：http handler 协程化 —— 挂起连接表 ====================
// 每活跃连接（fd）至多一个 handler 挂起项：段1（读+解析+req 构造）完成 →
//   http_pend_put(stage=1) + spawn handler 帧协程；协程完成回调 http_handler_done
//   （coro worker 线程）→ resp 写回（stage=2）+ fserve_push 投回续处理；fserve
//   worker 重入 http_conn_worker 循环顶 http_pend_take（stage=2）→ http_send_resp。
// req/resp 为该表 GC 根（http_pend_gc_mark 在 gc 标记期补标 —— precise 必须，
//   漏标 = GC 误回收挂起连接的请求/响应 → UAF）。锁 = g_hpend_mu（独立锁，见定义处 fix 注释）。
typedef struct HttpPend {
    int fd;
    int active;      // 1 = 有挂起项
    int stage;       // 1 = handler 协程运行中；2 = 已完成待响应
    int method_head;
    int client_close;
    LXValue req;     // GC 根（挂起期保活请求 dict）
    LXValue resp;    // GC 根（handler 完成结果）
} HttpPend;
static HttpPend* g_hpend = NULL;
static int g_hpend_cap = 0;
// M95-S2 fix：pending 表独立锁（不复用 g_conn_mu）。原因：GC 标记期 http_pend_gc_mark
//   需遍历本表，而 g_conn_mu 既有临界区（M88-B，px_evc_*）不屏蔽 SIG_GC_STOP —— 若
//   GC mark 拿 g_conn_mu，STW 恰暂停一个持 g_conn_mu 的线程（信号打断临界区）→ executor
//   等锁死锁（daemon 卡死：health 不分配仍响应、分配型请求全挂）。独立锁 + 本表临界区
//   屏蔽 SIG_GC_STOP → GC 信号 pending 到临界区外才递达，mark 无竞争。
//   锁序：g_hpend_mu 与 g_conn_mu / g_fserve_mu 均不嵌套持用（clear 在 g_conn_mu 外调；
//   handler_done 出锁后才 fserve_push）。
static pthread_mutex_t g_hpend_mu = PTHREAD_MUTEX_INITIALIZER;

static HttpPend* http_pend_ctx(int fd) {
    if (fd < 0 || fd >= g_hpend_cap) return NULL;
    return &g_hpend[fd];
}

// 扩容（持 g_hpend_mu；上限 g_conn_max = PX_MAX_CONNS env）。返回 0 可登记。
static int http_pend_ensure(int fd) {
    if (fd < 0 || fd >= g_conn_max) return -1;
    if (fd < g_hpend_cap) return 0;
    int ncap = g_hpend_cap ? g_hpend_cap : 256;
    while (ncap <= fd && ncap < g_conn_max) ncap *= 2;
    if (ncap > g_conn_max) ncap = g_conn_max;
    if (ncap <= fd) return -1;
    HttpPend* np = (HttpPend*)xrealloc(g_hpend, (size_t)ncap * sizeof(HttpPend));
    if (!np) return -1;
    for (int i = g_hpend_cap; i < ncap; i++) {
        np[i].fd = -1; np[i].active = 0; np[i].stage = 0;
        np[i].method_head = 0; np[i].client_close = 0;
        np[i].req.type = PX_NULL; np[i].resp.type = PX_NULL;
    }
    g_hpend = np; g_hpend_cap = ncap;
    return 0;
}

// 登记 handler 挂起（返回 1 成功 → 调用方 spawn 协程；0 → 退回同步路径）
// 持 g_hpend_mu 屏蔽 SIG_GC_STOP：GC mark 同锁遍历，临界区不被 GC 暂停（协议同
//   g_globals_mu/g_gc_mu）——防「GC 等锁 vs 持锁线程被暂停」死锁。
static int http_pend_put(int fd, LXValue req, int method_head, int client_close) {
    int ok = 0;
    sigset_t old;
    pthread_mutex_lock(&g_hpend_mu);
    gc_block_stop(&old);
    if (http_pend_ensure(fd) == 0) {
        HttpPend* p = http_pend_ctx(fd);
        if (p && !p->active) {
            p->fd = fd; p->active = 1; p->stage = 1;
            p->method_head = method_head; p->client_close = client_close;
            p->req = req;                    // GC 根接管（调用方根随后失效无碍）
            p->resp.type = PX_NULL;
            ok = 1;
        }
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_hpend_mu);
    return ok;
}

// 取完成项（stage==2 → 置 inactive 返回 1，req/resp 移交调用方；否则 0）
static int http_pend_take(int fd, LXValue* req, LXValue* resp, int* method_head, int* client_close) {
    int got = 0;
    sigset_t old;
    pthread_mutex_lock(&g_hpend_mu);
    gc_block_stop(&old);
    HttpPend* p = http_pend_ctx(fd);
    if (p && p->active && p->stage == 2) {
        *req = p->req; *resp = p->resp;
        *method_head = p->method_head; *client_close = p->client_close;
        p->active = 0; p->stage = 0; p->fd = -1;
        p->req.type = PX_NULL; p->resp.type = PX_NULL;
        got = 1;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_hpend_mu);
    return got;
}

// 清挂起项（px_evc_close 收尾调用：连接关闭 → 丢弃未决结果，防 fd 复用串扰）
static void http_pend_clear(int fd) {
    sigset_t old;
    pthread_mutex_lock(&g_hpend_mu);
    gc_block_stop(&old);
    HttpPend* p = http_pend_ctx(fd);
    if (p && p->active) {
        p->active = 0; p->stage = 0; p->fd = -1;
        p->req.type = PX_NULL; p->resp.type = PX_NULL;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_hpend_mu);
}

// GC 标记期补标挂起项 req/resp（gc executor 单线程标记期调用；持 g_hpend_mu）
static void http_pend_gc_mark(void) {
    sigset_t old;
    pthread_mutex_lock(&g_hpend_mu);
    gc_block_stop(&old);   // executor 本已自屏蔽（幂等）；写者临界区同屏蔽 → 无持锁被暂停
    if (g_hpend) {
        for (int i = 0; i < g_hpend_cap; i++) {
            HttpPend* p = &g_hpend[i];
            if (p->active) {
                if (p->req.type != PX_NULL) px_gc_mark_slots(&p->req, 1);
                if (p->resp.type != PX_NULL) px_gc_mark_slots(&p->resp, 1);
            }
        }
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_hpend_mu);
}

// handler 帧协程完成回调（coro worker 线程执行；ret = handler 顶层返回值）。
// 顺序：PX_KEEP 保护 ret（precise 窗口）→ g_hpend_mu 内 stage 1→2 写 resp →
//   出锁后 fserve_push 投回续处理 job。连接已关 / 表项已清（fd 复用）→ 丢弃 ret。
static void http_handler_done(void* ud, LXValue ret) {
    int fd = (int)(intptr_t)ud;
    if (fd < 0) return;
    px_root_push();
    PX_KEEP(ret);
    int push = 0;
    sigset_t old;
    pthread_mutex_lock(&g_hpend_mu);
    gc_block_stop(&old);
    HttpPend* p = http_pend_ctx(fd);
    if (p && p->active && p->stage == 1) {
        p->resp = ret;    // GC 根接管（表项 active 期间 mark 补标）
        p->stage = 2;
        push = 1;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_hpend_mu);
    if (push) fserve_push(fd, FSERVE_KIND_HTTP);   // 续处理：响应写 + keep-alive
    px_root_pop();
}

static PxConnCtx* px_evc_ctx(int fd) {
    if (fd < 0 || fd >= g_conn_cap) return NULL;
    return &g_conns[fd];
}

#if defined(__linux__)
// 事件循环唤醒（注册/摘除/关闭后写一字节，epoll_wait 立即醒来重算）
static void px_ev_wake(void) {
    if (g_ev_wakew >= 0) {
        ssize_t w = write(g_ev_wakew, "", 1);
        (void)w;  // EAGAIN 忽略（管道已满说明事件循环已被唤醒在途）
    }
}

// 登记连接为 ACTIVE（worker 开始处理前调用；重复登记同 fd 则复位旧上下文防串扰）
static PxConnCtx* px_evc_acquire(int fd, int kind) {
    pthread_mutex_lock(&g_conn_mu);
    if (px_evc_ensure(fd) != 0) { pthread_mutex_unlock(&g_conn_mu); return NULL; }
    PxConnCtx* c = px_evc_ctx(fd);
    if (c->state != PX_CONN_STATE_FREE) {
        // 旧上下文未收尾（异常路径）：强制清理（调用方须保证该 fd 已 close 或即将接管）
        if (c->ev_reg && g_ev_epfd >= 0) { epoll_ctl(g_ev_epfd, EPOLL_CTL_DEL, fd, NULL); c->ev_reg = 0; }
        if (c->pbuf) { xfree(c->pbuf); c->pbuf = NULL; c->pbuf_len = c->pbuf_cap = 0; }
    }
    c->fd = fd; c->kind = kind; c->state = PX_CONN_STATE_ACTIVE; c->idle_since = 0;
    c->idle_ev_cnt = 0;
    pthread_mutex_unlock(&g_conn_mu);
    return c;
}

// 关闭连接 + 收尾上下文（统一 close 路径，防 fd 复用串扰；供事件循环与 worker 收尾调用）
static void px_evc_close(int fd) {
    if (fd < 0) return;
    pthread_mutex_lock(&g_conn_mu);
    PxConnCtx* c = px_evc_ctx(fd);
    if (c && c->state != PX_CONN_STATE_FREE) {
        if (c->ev_reg && g_ev_epfd >= 0) { epoll_ctl(g_ev_epfd, EPOLL_CTL_DEL, fd, NULL); c->ev_reg = 0; }
        if (c->pbuf) { xfree(c->pbuf); c->pbuf = NULL; c->pbuf_len = c->pbuf_cap = 0; }
        c->fd = -1; c->state = PX_CONN_STATE_FREE; c->kind = 0; c->idle_since = 0;
    }
    pthread_mutex_unlock(&g_conn_mu);
    http_pend_clear(fd);   // M95-S2：清挂起 handler 项（fd 复用防串扰 + done 丢弃）
    close(fd);
}

// M88-B-S3：摘除事件循环登记并把连接上下文置 FREE，但**不 close fd**——供 SSE 统一关闭路径用
// （SSE 连接 fd 由 PxConn 拥有，px_conn_close 负责 close；本函数只防事件循环继续照看已关 fd）。
static void px_evc_detach(int fd) {
    if (fd < 0) return;
    pthread_mutex_lock(&g_conn_mu);
    PxConnCtx* c = px_evc_ctx(fd);
    if (c && c->state != PX_CONN_STATE_FREE) {
        if (c->ev_reg && g_ev_epfd >= 0) { epoll_ctl(g_ev_epfd, EPOLL_CTL_DEL, fd, NULL); c->ev_reg = 0; }
        if (c->pbuf) { xfree(c->pbuf); c->pbuf = NULL; c->pbuf_len = c->pbuf_cap = 0; }
        c->fd = -1; c->state = PX_CONN_STATE_FREE; c->kind = 0; c->idle_since = 0;
    }
    pthread_mutex_unlock(&g_conn_mu);
}

// M88-B-S3：查询 fd 当前是否在 IDLE（事件循环照看）。SSE 的 sse_close 用它区分关闭路径：
//   IDLE（事件化，worker 已释放）→ detach + px_conn_close 统一清理；非 IDLE → 原 shutdown 唤醒语义。
static int px_evc_is_idle(int fd) {
    int r = 0;
    pthread_mutex_lock(&g_conn_mu);
    PxConnCtx* c = px_evc_ctx(fd);
    if (c && c->state == PX_CONN_STATE_IDLE) r = 1;
    pthread_mutex_unlock(&g_conn_mu);
    return r;
}

// worker 响应写完且无下一请求数据 → 连接交还 IDLE（注册事件循环等可读）。
// M99：nonblock 标志——http_serve 用 1（fd 置非阻塞，读走 px_recv_wait poll 语义，原行为）；
//   px_serve 用 0（fd 保持阻塞 + SO_RCVTIMEO recv 读语义，切非阻塞会致 recv EAGAIN 误判断开；
//   epoll 对阻塞 fd 照常报可读，投回时数据在途 → 阻塞 recv 立即返回，零 EAGAIN/零 TLS WANT）。
// 返回 0 成功；连接未登记/事件循环不可用（非 Linux）返回 -1（调用方走原阻塞路径）
static int px_evc_idle_put_fd(int fd, int kind, int nonblock) {
    if (g_ev_epfd < 0) return -1;
    if (nonblock) px_fd_nonblock(fd);
    pthread_mutex_lock(&g_conn_mu);
    PxConnCtx* c = px_evc_ctx(fd);
    if (!c || c->state != PX_CONN_STATE_ACTIVE) { pthread_mutex_unlock(&g_conn_mu); return -1; }
    c->kind = kind;
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    // 边缘触发：可读事件只报一次，读完由 worker 再交还（F3 根因实验确认与 ET/LT 无关，
    //   漏报实为 GC STW 暂停事件循环所致；恢复 ET 减少无谓唤醒）
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(g_ev_epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
        // 注册失败（fd 非法/已达上限等）→ 保持 ACTIVE，调用方按失败处理（原路径关闭）
        pthread_mutex_unlock(&g_conn_mu);
        return -1;
    }
    c->state = PX_CONN_STATE_IDLE; c->ev_reg = 1; c->idle_since = px_ev_now_ms();
    c->idle_ev_cnt = 0;
    pthread_mutex_unlock(&g_conn_mu);
    __atomic_fetch_add(&g_diag_idle_put, 1, __ATOMIC_RELAXED);
    px_ev_wake();
    return 0;
}

// http_serve 用（fd 置非阻塞；原语义）：M99 重构保留 wrapper
static int px_evc_idle_put(int fd, int kind) {
    return px_evc_idle_put_fd(fd, kind, 1);
}

// 事件循环/接管方摘除：IDLE →（摘除事件循环）→ ACTIVE 返回 1；非 IDLE 返回 0
// （事件循环 detect 可读后调用，随后把 fd 投回 fserve 队列）
static int px_evc_idle_pop(int fd) {
    pthread_mutex_lock(&g_conn_mu);
    PxConnCtx* c = px_evc_ctx(fd);
    if (!c || c->state != PX_CONN_STATE_IDLE) { pthread_mutex_unlock(&g_conn_mu); return 0; }
    if (c->ev_reg && g_ev_epfd >= 0) { epoll_ctl(g_ev_epfd, EPOLL_CTL_DEL, fd, NULL); c->ev_reg = 0; }
    c->state = PX_CONN_STATE_ACTIVE; c->idle_since = 0;
    pthread_mutex_unlock(&g_conn_mu);
    return 1;
}

// 事件循环线程：等可读/断开/超时 → 可读派发回 fserve，断开/超时 close 清理。
// M88-B-S3：按连接类型分流——http IDLE 可读 → 派发回池（worker 处理下一请求突发）；
//   SSE IDLE 长连接 → 只做断开检测（读掉/丢弃客户端数据；对端 FIN/错误 → sse_server_close_fd 统一清理）。
//   SSE 空闲不超时（区别于 http keep-alive 15s）：tick 超时扫描仅对 http IDLE 生效。
static void* px_ev_loop(void* arg) {
    (void)arg;
    struct epoll_event evs[256];
    for (;;) {
        int n = epoll_wait(g_ev_epfd, evs, 256, PX_EV_TICK_MS);
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }
        for (int i = 0; i < n; i++) {
            int fd = evs[i].data.fd;
            if (fd == g_ev_wakefd) {           // 唤醒管道：排空
                char tmp[64];
                while (read(g_ev_wakefd, tmp, sizeof(tmp)) > 0) {}
                continue;
            }
            // 先取该 fd 当前 IDLE 连接的类型（锁内快照；FREE/异常跳过）
            // F3-fix 插桩：IDLE 周期内每被 epoll_wait 返回一次 idle_ev_cnt++（观测漏报：
            //   超时救回/关闭时若 cnt==0 → 本 IDLE 周期 epoll 从未报过该 fd）
            int kind = -1;
            pthread_mutex_lock(&g_conn_mu);
            PxConnCtx* c0 = px_evc_ctx(fd);
            if (c0 && c0->state == PX_CONN_STATE_IDLE) { kind = c0->kind; c0->idle_ev_cnt++; }
            // F3-fix 现场抓拍：epoll 报事件但连接 state 非 IDLE（ev_reg=1 = 仍在 epoll）
            //   → 状态与登记不一致（漏报根因候选：某路径改 state 未 DEL，事件循环每次跳过）
            if (g_ev_diag && c0 && c0->ev_reg && c0->state != PX_CONN_STATE_IDLE)
                fprintf(stderr, "[px-ev:INCONSIST] fd=%d state=%d ev_reg=1 events=0x%x\n",
                        fd, c0->state, evs[i].events);
            pthread_mutex_unlock(&g_conn_mu);
            if (kind == FSERVE_KIND_SSE) {
                // SSE 长连接 IDLE：客户端不应发数据——有事件多半是对端断开/异常。
                // 读掉并丢弃客户端数据（原阻塞保持语义）；对端 FIN/读错误 → 统一关闭清理。
                if (sse_idle_should_close(fd)) sse_server_close_fd(fd);
                continue;
            }
            // M99：HTTP 与 px_serve（PXSERVE）IDLE 均按可读/断开/超时派发；SSE 已在上方 continue
            if (kind != FSERVE_KIND_HTTP && kind != FSERVE_KIND_PXSERVE) continue;   // 非 IDLE/状态异常/已关闭：跳过
            if (evs[i].events & (EPOLLIN)) {    // 可读 → 派发回服务池（worker 再接管）
                int k2 = FSERVE_KIND_HTTP;
                pthread_mutex_lock(&g_conn_mu);
                PxConnCtx* c = px_evc_ctx(fd);
                if (c && c->state == PX_CONN_STATE_IDLE) {
                    k2 = c->kind;
                    if (c->ev_reg) { epoll_ctl(g_ev_epfd, EPOLL_CTL_DEL, fd, NULL); c->ev_reg = 0; }
                    c->state = PX_CONN_STATE_ACTIVE; c->idle_since = 0;
                } else {
                    c = NULL;  // 状态异常/已关闭：跳过派发
                }
                pthread_mutex_unlock(&g_conn_mu);
                if (c) {
                    __atomic_fetch_add(&g_diag_detect_http, 1, __ATOMIC_RELAXED);
                    // M99：PXSERVE → 投回 px_serve 的 g_pool（px_pool_push）；HTTP/SSE → fserve
                    if (k2 == FSERVE_KIND_PXSERVE) px_pool_push(fd);
                    else fserve_push(fd, k2);   // 队满阻塞背压（事件循环线程暂停派发，不丢 fd）
                }
                continue;
            }
            if (evs[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {  // 对端断开/异常
                // M99：PXSERVE 连接含 PxConn/TLS/inflight → 走 px_pxpend_close（不可裸 close）
                if (kind == FSERVE_KIND_PXSERVE) px_pxpend_close(fd);
                else px_evc_close(fd);
                continue;
            }
        }
        // 周期性扫空闲超时（统一 tick；与 SO_RCVTIMEO 15s 语义对齐）
        // M88-B-S3：仅 http IDLE 连接受 15s 空闲超时约束；SSE IDLE 长连接不超时（等 sse_close/对端断开）。
        // F3-fix（M90-F3）：空闲超时 close 前 poll(0) 二次确认——杜绝「事件循环漏报 →
        //   活跃连接被 15s tick 误杀」（Connection reset / 长压 RPS 骤降根因）。确认逻辑：
        //   有在途数据（POLLIN）→ 连接实际活跃（漏报），摘除上下文后重新登记 ACTIVE 并投回
        //   fserve 池继续服务；POLLHUP/POLLERR → 对端已断开 → 照常关闭；无事件 → 真空闲 → close。
        long long now = px_ev_now_ms();
        pthread_mutex_lock(&g_conn_mu);
        for (int i = 0; i < g_conn_cap; i++) {
            PxConnCtx* c = &g_conns[i];
            // M99：px_serve（PXSERVE）IDLE 同受 15s 空闲超时约束（与 HTTP keep-alive 语义一致；
            //   SSE 长连接仍不超时，保持历史）。kind 快照供出锁后按类型收尾。
            if (c->state != PX_CONN_STATE_IDLE ||
                (c->kind != FSERVE_KIND_HTTP && c->kind != FSERVE_KIND_PXSERVE) ||
                now - c->idle_since < PX_CONN_IDLE_TMO_MS) continue;
            int fd = c->fd;
            int kind_tmo = c->kind;   // M99：HTTP / PXSERVE（close 收尾分派用）
            int idle_ev_cnt = c->idle_ev_cnt;   // 本 IDLE 周期 epoll 上报次数（诊断用）
            int ev_reg_before = c->ev_reg;      // 超时收尾前是否仍登记在 epoll（诊断用）
            // F3-fix 根因验证：EPOLL_CTL_MOD 试探内核 epoll 是否真有该 fd（ev_reg 可能陈旧——
            //   fd 曾被 close(内核自动 DEL) 但上下文未清 → 新连接复用 fd → 数据到达 epoll 永不报）
            if (g_ev_diag && c->ev_reg && g_ev_epfd >= 0) {
                struct epoll_event mev; memset(&mev, 0, sizeof(mev));
                mev.events = EPOLLIN | EPOLLRDHUP; mev.data.fd = fd;
                int mr = epoll_ctl(g_ev_epfd, EPOLL_CTL_MOD, fd, &mev);
                if (mr != 0 && errno == ENOENT)
                    fprintf(stderr, "[px-ev:tmo] CHECK fd=%d ev_reg=%d → 内核epoll【不在】(ENOENT, ev_reg陈旧!)\n",
                            fd, ev_reg_before);
            }
            // 摘除事件循环登记并释放上下文（统一收尾路径；fd 暂不 close，供下方 poll(0) 二次确认）
            if (c->ev_reg && g_ev_epfd >= 0) { epoll_ctl(g_ev_epfd, EPOLL_CTL_DEL, fd, NULL); c->ev_reg = 0; }
            if (c->pbuf) { xfree(c->pbuf); c->pbuf = NULL; c->pbuf_len = c->pbuf_cap = 0; }
            c->fd = -1; c->state = PX_CONN_STATE_FREE; c->kind = 0; c->idle_since = 0;
            c->idle_ev_cnt = 0;
            pthread_mutex_unlock(&g_conn_mu);
            // —— 二次确认（fd 已从上下文摘除且未 close：数字仍占用，poll 安全）——
            int keep = 0;
            struct pollfd pfd;
            pfd.fd = fd; pfd.events = POLLIN; pfd.revents = 0;
            int pr = poll(&pfd, 1, 0);
            if (pr > 0 && (pfd.revents & POLLIN) &&
                !(pfd.revents & (POLLHUP | POLLERR | POLLNVAL))) {
                keep = 1;                  // 在途数据：活跃连接被漏报 → 救回
            }
            if (keep) {
                __atomic_fetch_add(&g_diag_tmo_save, 1, __ATOMIC_RELAXED);
                if (g_ev_diag) {
                    // 区分「活跃连接漏报」vs「对端 FIN(EOF) 未上报」：MSG_PEEK 看缓冲内容
                    char pkb[80]; int pkn = -1, avail = -1;
                    pkn = (int)recv(fd, pkb, sizeof(pkb) - 1, MSG_PEEK | MSG_DONTWAIT);
                    if (pkn >= 0) pkb[pkn] = 0; else pkb[0] = 0;
                    ioctl(fd, FIONREAD, &avail);
                    fprintf(stderr, "[px-ev:tmo] SAVE fd=%d ev_reg=%d idle_ev_cnt=%d peek=%d avail=%d head='%.*s'\n",
                            fd, ev_reg_before, idle_ev_cnt, pkn, avail,
                            (pkn > 0 && pkn < 80) ? pkn : 0, pkb);
                }
                // M99：按类型投回服务池——PXSERVE → px_serve 的 g_pool；HTTP → fserve
                PxConnCtx* ac = px_evc_acquire(fd, kind_tmo);  // FREE→ACTIVE 重新登记
                if (ac) {
                    if (kind_tmo == FSERVE_KIND_PXSERVE) px_pool_push(fd);
                    else fserve_push(fd, FSERVE_KIND_HTTP);    // 投回池，worker 接管读在途请求
                } else {
                    // 登记失败兜底（理论不可达）：PXSERVE 走 px_pxpend_close（清 PxConn/TLS/inflight）
                    if (kind_tmo == FSERVE_KIND_PXSERVE) px_pxpend_close(fd);
                    else close(fd);
                }
            } else {
                __atomic_fetch_add(&g_diag_tmo_close, 1, __ATOMIC_RELAXED);
                if (g_ev_diag)
                    fprintf(stderr, "[px-ev:tmo] CLOSE fd=%d ev_reg=%d idle_ev_cnt=%d (真空闲/断开)\n",
                            fd, ev_reg_before, idle_ev_cnt);
                // 真空闲/对端断开：空闲超时关闭（keep-alive 15s 语义）
                // M99：PXSERVE 连接含 PxConn/TLS/inflight → 必须走 px_pxpend_close（不可裸 close，
                //   否则泄漏 PxConn 堆对象 + TLS 会话 + inflight 计数错乱）
                if (kind_tmo == FSERVE_KIND_PXSERVE) px_pxpend_close(fd);
                else close(fd);
            }
            pthread_mutex_lock(&g_conn_mu);
        }
        pthread_mutex_unlock(&g_conn_mu);
        // 诊断输出（PX_EV_DIAG=1；每 ~5s 一行；生产默认关）
        if (g_ev_diag && now - g_diag_last_log >= 5000) {
            g_diag_last_log = now;
            fprintf(stderr, "[px-ev:diag] idle_put=%lld detect_http=%lld tmo_close=%lld tmo_save=%lld (tmo_save=漏报救回的活跃连接)\n",
                    __atomic_load_n(&g_diag_idle_put, __ATOMIC_RELAXED),
                    __atomic_load_n(&g_diag_detect_http, __ATOMIC_RELAXED),
                    __atomic_load_n(&g_diag_tmo_close, __ATOMIC_RELAXED),
                    __atomic_load_n(&g_diag_tmo_save, __ATOMIC_RELAXED));
        }
    }
    return NULL;
}

// 懒启动事件循环（首个 px_evc_idle_put 前置调用；幂等）
static void px_ev_ensure(void) {
    if (g_ev_epfd >= 0) return;
    pthread_mutex_lock(&g_conn_mu);
    if (g_ev_epfd < 0) {
        if (getenv("PX_EV_DIAG") && atoi(getenv("PX_EV_DIAG")) > 0) g_ev_diag = 1;
        int epfd = epoll_create1(0);
        if (epfd >= 0) {
            int p[2];
            if (pipe(p) == 0) {
                px_fd_nonblock(p[0]); px_fd_nonblock(p[1]);
                struct epoll_event ev;
                memset(&ev, 0, sizeof(ev));
                ev.events = EPOLLIN; ev.data.fd = p[0];
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, p[0], &ev) == 0) {
                    g_ev_epfd = epfd; g_ev_wakefd = p[0]; g_ev_wakew = p[1];
                    g_ev_run = 1;
                    if (pthread_create(&g_ev_thread, NULL, px_ev_loop, NULL) != 0) {
                        g_ev_run = 0; g_ev_epfd = -1;
                        close(p[0]); close(p[1]); close(epfd);
                        g_ev_wakefd = g_ev_wakew = -1;
                    } else {
                        pthread_detach(g_ev_thread);
                    }
                } else {
                    close(p[0]); close(p[1]); close(epfd);
                }
            } else {
                close(epfd);
            }
        }
    }
    pthread_mutex_unlock(&g_conn_mu);
}
#else
// 非 Linux（Windows 交叉等）：事件驱动内核降级为空操作——连接走既有阻塞处理路径，
// 功能不降仅无空闲不占线程优化（文档明示 Linux epoll 一等）。
static void px_ev_wake(void) { (void)0; }
static PxConnCtx* px_evc_acquire(int fd, int kind) { (void)fd; (void)kind; return NULL; }
static void px_evc_close(int fd) { http_pend_clear(fd); close(fd); }
static void px_evc_detach(int fd) { (void)fd; }
static int px_evc_is_idle(int fd) { (void)fd; return 0; }
static int px_evc_idle_put(int fd, int kind) { (void)fd; (void)kind; return -1; }
static int px_evc_idle_put_fd(int fd, int kind, int nonblock) { (void)fd; (void)kind; (void)nonblock; return -1; }
static int px_evc_idle_pop(int fd) { (void)fd; return 0; }
static void* px_ev_loop(void* arg) { (void)arg; return NULL; }
static void px_ev_ensure(void) { (void)0; }
#endif


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
    // M88-S2：接入连接线程池（取代每连接 px_spawn——高并发不再因 spawn 槽满而 exit）
    fserve_ensure();
    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        fserve_push(cfd, FSERVE_KIND_HTTP);
    }
    return px_null(); // 不可达
}

// http_serve_unix(sock_path, handler)：AF_UNIX HTTP 服务端（M82 / Issue 15 GAP-SRV-1）
// 与 http_serve 同族：HTTP 解析/路由/keep-alive 完全复用 http_conn_worker，
// 仅监听面从 TCP 换成 Unix domain socket（ws-approve serve 等本地 HTTP over unix socket 场景）。
// 差异点：① bind 前 unlink 清理残留 sock 文件（上次异常退出遗留 → EADDRINUSE）；
//        ② bind 后 chmod 0600（审批/令牌数据敏感，仅 owner 可读写）；
//        ③ accept 循环错误容忍（EINTR 直接重试，EMFILE 等短暂让出避免忙循环）。
static LXValue bi_http_serve_unix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("http_serve_unix 需要 (sock_path, handler) 参数");
    const char* sock_path = args[0].as.obj->as.str.data;
    if (sock_path[0] == '\0') px_error("http_serve_unix: sock_path 不能为空");
    if (strlen(sock_path) >= sizeof(((struct sockaddr_un*)0)->sun_path))
        px_error("http_serve_unix: socket 路径过长（> %d）", (int)sizeof(((struct sockaddr_un*)0)->sun_path) - 1);
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("http_serve_unix 的 handler 必须是函数");
    // handler 存入全局表（GC 扫描根），连接线程经全局表取回（与 http_serve 同槽 __http_handler）
    px_set_global("__http_handler", handler);
    // 清理残留 sock 文件（上次异常退出遗留；忽略 ENOENT）
    unlink(sock_path);
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) px_error("http_serve_unix: socket 创建失败");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", sock_path);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        px_error("http_serve_unix: 绑定 %s 失败", sock_path);
    }
    // 审批/令牌数据敏感：sock 文件收紧为 0600（bind 创建时受 umask 影响，显式收紧）
    chmod(sock_path, 0600);
    if (listen(sfd, 128) < 0) {
        close(sfd);
        px_error("http_serve_unix: listen 失败");
    }
    // M88-S2：接入连接线程池（保留 unix 版 accept 错误容忍语义）
    fserve_ensure();
    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            // accept 循环错误容忍：EINTR 重试；EMFILE/ENFILE 等短暂让出避免忙循环
            if (errno == EINTR) continue;
            struct timespec ts = {0, 50 * 1000 * 1000}; // 50ms
            nanosleep(&ts, NULL);
            continue;
        }
        fserve_push(cfd, FSERVE_KIND_HTTP);
    }
    return px_null(); // 不可达
}

// ==================== M21 SSE 服务端（编译模式，与并发 GC 兼容） ====================
// sse_serve(port, handler)：accept 循环，连接交 fserve 池 worker 处理（M88-S2）。
// 连接 worker：解析请求 → 发 SSE 响应头 → 注册连接(conn id) → 调 handler（req 注入 conn）
//           → handler 返回后保持连接，直到 sse_close(conn) 或对端断开。
// sse_send(conn, data)：注册表 + 锁，任意线程可推送；写失败自动清理。
// sse_close(conn)：shutdown 唤醒（阻塞保持路径）或摘除事件循环清理（B-S3 事件化路径），并清注册。

// M88-B-S3（qg-issue 27 B 类）：服务端 SSE 注册表 256 定长 → 动态容量表
// （env PX_MAX_SSE_CONNS，默认 4096，夹取 [64,65536]），支撑上千~上万 SSE 长连接挂载；
// 容量首次使用时一次性分配（无 realloc 竞态，指针恒定）。
#define MAX_SSE_CONNS_DEFAULT 4096
// M95-S4：SSE handler 协程化 —— stage 字段：
//   0 = 常规/无 handler 协程；1 = handler 帧协程运行中（fserve worker 已释放，
//      连接注册表项保留供 sse_send/sse_close）；2 = handler 协程完成待续处理
//      （done 回调置位 + fserve_push(fd, SSE) 投回 → sse_conn_worker 入口检测
//      stage==2 → 执行原 step8 hold 收尾：明文交 IDLE / TLS 阻塞保持读）。
typedef struct SseServerConn { int fd; int64_t id; int active; PxConn* conn; int stage; } SseServerConn;
static pthread_mutex_t g_sse_mu = PTHREAD_MUTEX_INITIALIZER;
static SseServerConn* g_sse_conns = NULL;
static int g_sse_cap = 0;            // 当前容量（首次使用时按 env 上限一次性分配）
static int g_sse_tab_inited = 0;
static int64_t g_sse_next_id = 1;

// 惰性分配服务端 SSE 注册表（调用方须持 g_sse_mu；sse_find/sse_alloc_slot 首行自动调用）
static void sse_tab_ensure(void) {
    if (g_sse_tab_inited) return;
    g_sse_tab_inited = 1;
    int cap = MAX_SSE_CONNS_DEFAULT;
    const char* e = getenv("PX_MAX_SSE_CONNS");
    if (e) {
        int v = atoi(e);
        if (v >= 64 && v <= 65536) cap = v;
    }
    g_sse_conns = (SseServerConn*)xcalloc((size_t)cap, sizeof(SseServerConn));
    for (int i = 0; i < cap; i++) g_sse_conns[i].fd = -1;
    g_sse_cap = cap;
}

static int sse_find(int64_t id) {
    sse_tab_ensure();
    for (int i = 0; i < g_sse_cap; i++) {
        if (g_sse_conns[i].active && g_sse_conns[i].id == id) return i;
    }
    return -1;
}

static int sse_alloc_slot(void) {
    sse_tab_ensure();
    for (int i = 0; i < g_sse_cap; i++) {
        if (!g_sse_conns[i].active) return i;
    }
    return -1;
}

// ==================== M88-B-S3：SSE 长连接事件化辅助 ====================
// 事件循环对 SSE IDLE 连接只做断开检测：SSE 客户端在长连接期间不应发数据，但原保持语义是
// 「阻塞读并丢弃客户端数据直到断开」。事件化后由事件循环读掉并丢弃（drain），并对对端
// FIN/读错误返回"应关闭"。返回 1 = 对端断开/错误（调用方走 sse_server_close_fd）；0 = 保持 IDLE。
static int sse_idle_should_close(int fd) {
    char tmp[4096];
    int rounds = 0;
    for (;;) {
        ssize_t n = recv(fd, tmp, sizeof(tmp), MSG_DONTWAIT);
        if (n > 0) {                 // 读掉并丢弃（客户端违规数据；原阻塞保持语义）
            if (++rounds > 1024) return 0;   // 恶意灌流上限（防占死事件循环；残留等下次事件）
            continue;
        }
        if (n == 0) return 1;        // 对端 FIN → 关闭
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;  // 读尽 → 保持 IDLE
        return 1;                    // 读错误 → 关闭
    }
}

// SSE 服务端连接统一关闭路径（事件循环断开检测 / sse_send 写失败 / sse_close(IDLE 场景) 共用）：
//   ① px_evc_detach：摘除事件循环登记 + 连接上下文置 FREE（不 close fd）；
//   ② g_sse_mu 内清服务端注册表匹配项并取 PxConn*；
//   ③ px_conn_close（幂等，内部 close fd 一次）——无注册项时直接 close(fd) 兜底。
// 锁序：g_conn_mu 与 g_sse_mu 从不嵌套持用（本函数先 g_conn_mu 后 g_sse_mu，各自释放后取下一把）。
static void sse_server_close_fd(int fd) {
    if (fd < 0) return;
    px_evc_detach(fd);
    int found = 0;
    PxConn* pc = NULL;
    pthread_mutex_lock(&g_sse_mu);
    sse_tab_ensure();
    for (int i = 0; i < g_sse_cap; i++) {
        if (g_sse_conns[i].active && g_sse_conns[i].fd == fd) {
            pc = g_sse_conns[i].conn;
            g_sse_conns[i].active = 0;
            g_sse_conns[i].fd = -1;
            g_sse_conns[i].conn = NULL;
            g_sse_conns[i].stage = 0;   // M95-S4：清 handler 协程 stage（防 slot 复用残留）
            found = 1;
            break;
        }
    }
    if (pc) px_conn_close(pc);   // 幂等（closed 标记）；内部 close(fd) 一次
    pthread_mutex_unlock(&g_sse_mu);
    if (!found) close(fd);       // 未注册（调用方兜底）：直接关 fd（无 PxConn 持有）
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
    // M32：自动重连（sse_connect(url, reconnect_ms)）
    long long reconnect_ms;   // >0 时断线自动重连
    char url[512];            // 原始 URL（重连用）
    char last_event_id[512];  // 最近事件 id（重连时带 Last-Event-ID）
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

// ==================== M95-S4：sse_serve handler 协程化辅助 ====================
// 目标：SSE handler（VM px 函数）执行从「fserve worker 线程内同步 px_call（handler
//   内 chan/sleep/spawn 阻塞占线程）」→「handler 帧协程（M93 协程，让出占协程不占
//   fserve worker）」。sse_conn_worker 拆段：
//     · 段1（fserve worker）：TLS 握手 → 读请求 → 解析 → 注册 conn id → 发 SSE 响应头
//       → handler 若 VM → 注册表项 stage=1 + px_coro_spawn_ex(handler, [req],
//       sse_handler_done, fd) → return（worker 释放，连接注册表项保留供 sse_send）；
//     · handler 协程完成回调（coro worker 线程）sse_handler_done → g_sse_mu 内
//       stage 1→2 + fserve_push(fd, SSE) 投回续处理；
//     · fserve worker 重入 sse_conn_worker 入口 → 见注册表项 active && stage==2
//       → take（stage 归 0，active 保留）→ sse_conn_hold 收尾（明文 → 交还 IDLE
//       事件循环；TLS/事件化不可用 → 原阻塞保持读 + 清理注册）→ return。
//   handler 非 VM（PX_NATIVE，逃生舱）→ 原同步 px_call + sse_conn_hold（零变化）。
// GC：handler 协程运行期 req 由协程 args 副本保活（px_coro_gc_mark_roots 标 g_all
//   协程 args）→ 无独立 pending 根面需求；收尾不需 req。连接对象 c = 注册表项
//   conn（C 堆对象非 GC），跨线程经注册表访问。g_cur_conn 全库无读取消费者（仅
//   赋值，历史遗留）→ handler 协程化无需 TLS 连接上下文迁移（D0 确认）。
// ============================================================

// handler 协程完成回调：把注册表项 stage 1→2（连接仍 active）并投回 fserve 续处理。
// 若连接已关闭/注册项已清（sse_close/断开）→ 丢弃（防 fd 复用误投）。
static void sse_handler_done(void* ud, LXValue ret) {
    int fd = (int)(intptr_t)ud;
    if (fd < 0) return;
    px_root_push();
    PX_KEEP(ret);   // precise 窗口保护（SSE handler 返回值语义无接收方 → 随即丢弃）
    int push = 0;
    pthread_mutex_lock(&g_sse_mu);
    sse_tab_ensure();
    for (int i = 0; i < g_sse_cap; i++) {
        if (g_sse_conns[i].active && g_sse_conns[i].fd == fd) {
            if (g_sse_conns[i].stage == 1) { g_sse_conns[i].stage = 2; push = 1; }
            break;
        }
    }
    pthread_mutex_unlock(&g_sse_mu);
    if (push) fserve_push(fd, FSERVE_KIND_SSE);   // 投回续处理（worker 重入入口收尾）
    px_root_pop();
}

// 段2 收尾（原 sse_conn_worker step8/9 抽出；同步路径与 handler 协程续处理共用）。
// 连接注册表项在收尾前仍 active（明文交 IDLE 后保留供 sse_send；TLS/断开清理）。
static void sse_conn_hold(int fd, PxConn* c) {
    if (!c) return;
    // step8：明文 → 交还 IDLE 事件循环照看（事件循环 detect 断开/sse_close/sse_send
    //   写失败 → sse_server_close_fd 统一清理；SSE 空闲不超时）。TLS/事件化不可用
    //   → 原阻塞保持路径（功能不降，仅 TLS SSE 长连接仍占 worker，文档注明）。
    if (!c->is_tls) {
        PxConnCtx* ac = px_evc_acquire(fd, FSERVE_KIND_SSE);
        if (ac) {
            px_ev_ensure();
            if (px_evc_idle_put(fd, FSERVE_KIND_SSE) == 0) {
                // 交还成功：注册表项保留（conn 供 sse_send 写）；本 worker 收尾释放
                __sync_fetch_and_sub(&g_px_inflight, 1);
                g_cur_conn = NULL;
                return;
            }
            px_evc_detach(fd);
            int fl = fcntl(fd, F_GETFL, 0);
            if (fl >= 0) fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
        }
    }
    // 原保持路径（TLS 或事件化不可用）：read 阻塞直到 sse_close（shutdown 唤醒）或对端断开
    char rb[64];
    while (px_conn_read(c, rb, sizeof(rb)) > 0) {}
    // step9：清理注册 + 关闭（只在仍注册时 close，避免与 sse_close 重复关闭）
    int closed = 0;
    pthread_mutex_lock(&g_sse_mu);
    sse_tab_ensure();
    for (int i = 0; i < g_sse_cap; i++) {
        if (g_sse_conns[i].active && g_sse_conns[i].fd == fd) {
            g_sse_conns[i].active = 0;
            g_sse_conns[i].fd = -1;
            g_sse_conns[i].stage = 0;
            closed = 1;
        }
    }
    pthread_mutex_unlock(&g_sse_mu);
    if (closed) {
        pthread_mutex_lock(&g_sse_mu);
        for (int j = 0; j < g_sse_cap; j++) {
            if (g_sse_conns[j].conn == c) g_sse_conns[j].conn = NULL;
        }
        pthread_mutex_unlock(&g_sse_mu);
        px_conn_close(c);  // 对象保留（closed 标记），避免并发 ws/sse 使用悬垂指针
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
    }
}

// SSE 连接线程（px_spawn 注册进 GC 槽位）：args[0] = fd
static LXValue sse_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // M95-S4：handler 协程完成续处理（重入入口）——注册表项 active && stage==2
    //   → take（stage 归 0，active 保留至收尾决定）→ 段2 收尾后返回。
    {
        PxConn* c2 = NULL;
        pthread_mutex_lock(&g_sse_mu);
        sse_tab_ensure();
        for (int i = 0; i < g_sse_cap; i++) {
            if (g_sse_conns[i].active && g_sse_conns[i].fd == fd && g_sse_conns[i].stage == 2) {
                c2 = g_sse_conns[i].conn;
                g_sse_conns[i].stage = 0;
                break;
            }
        }
        pthread_mutex_unlock(&g_sse_mu);
        if (c2) { sse_conn_hold(fd, c2); return px_null(); }
    }
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
    px_root_push();   // M92-S2c precise：sse_conn_worker 登记作用域开始
    LXValue headers = px_dict();
    PX_KEEP(headers);   // M92-S2c precise：headers 裸局部跨 px_dict_set/px_str 分配
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
    PX_KEEP(req);   // M92-S2c precise：req 裸局部跨 px_dict_set/px_str/px_call
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
        px_root_pop();   // M92-S2c precise
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

    // 7. 调 handler（M95-S4：VM handler → 帧协程异步执行占协程，fserve worker 释放；
    //    handler 完成回调 sse_handler_done 投回续处理 → 入口 stage==2 走 sse_conn_hold）
    LXValue handler = px_get_global("__sse_handler");
    int hvm = (handler.type == PX_FUNC && px_vm_entry &&
               handler.as.obj->as.func.fn == px_vm_entry);
    if (hvm) {
        // 登记 stage=1（连接注册表项保留供 sse_send/sse_close）→ spawn handler 帧协程并
        //   释放本 worker（handler 内 chan/sleep/spawn 让出占协程不占 fserve worker）。
        pthread_mutex_lock(&g_sse_mu);
        sse_tab_ensure();
        for (int i = 0; i < g_sse_cap; i++) {
            if (g_sse_conns[i].active && g_sse_conns[i].fd == fd) {
                g_sse_conns[i].stage = 1;
                break;
            }
        }
        pthread_mutex_unlock(&g_sse_mu);
        if (px_coro_spawn_ex)
            px_coro_spawn_ex(handler.as.obj->as.func.ctx, &req, 1,
                             sse_handler_done, (void*)(intptr_t)fd);
        else {
            // 无协程内核（理论不达）→ 退回原同步直调（行为零变化）
            pthread_mutex_lock(&g_sse_mu);
            for (int i = 0; i < g_sse_cap; i++) {
                if (g_sse_conns[i].active && g_sse_conns[i].fd == fd) g_sse_conns[i].stage = 0;
            }
            pthread_mutex_unlock(&g_sse_mu);
            if (handler.type == PX_FUNC || handler.type == PX_NATIVE)
                px_call(handler, &req, 1);
        }
        px_root_pop();   // M92-S2c precise：req 作用域结束（req 已由协程 args 保活）
        return px_null();   // worker 释放；handler 完成回调投回续处理收尾
    }
    // —— 逃生舱（PX_NATIVE / 非 VM handler）：原同步直调路径（行为零变化）——
    if (handler.type == PX_FUNC || handler.type == PX_NATIVE) {
        px_call(handler, &req, 1);
    }
    px_root_pop();   // M92-S2c precise：req 作用域结束（px_call 返回后不再用 req/headers）
    sse_conn_hold(fd, c);   // 段2 收尾（明文交 IDLE / TLS 阻塞保持读 + 清理）
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
    // M88-S2：接入连接线程池；M88-B-S3：SSE 长连接 handler 返回后事件化（空闲不占 worker，
    // 见 sse_conn_worker 第 8 步与池注释）
    fserve_ensure();
    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        fserve_push(cfd, FSERVE_KIND_SSE);
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
    pthread_mutex_unlock(&g_sse_mu);
    xfree(frame);
    if (w < 0) {
        // M88-B-S3：写失败 = 对端已断/连接异常 → 统一关闭路径（摘除事件循环登记 + 清注册 + 关连接）。
        // 锁外执行：px_conn_close 幂等，与其它 sse_send/事件循环清理并发安全。
        sse_server_close_fd(fd);
        return px_bool(false);
    }
    return px_bool(true);
}

// sse_close(conn) → bool（服务端连接 shutdown 唤醒 / 事件化摘除；客户端连接直接关闭）
static LXValue bi_sse_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("sse_close 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    // 服务端连接：先取 fd（存在性判断）
    int fd = -1;
    pthread_mutex_lock(&g_sse_mu);
    int idx = sse_find(conn);
    if (idx >= 0) fd = g_sse_conns[idx].fd;
    pthread_mutex_unlock(&g_sse_mu);
    if (fd < 0) {
        // 客户端连接（原逻辑不变）
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
    // M88-B-S3：区分关闭路径——IDLE（已事件化，worker 已释放）→ sse_server_close_fd 统一清理
    // （摘除事件循环 + 清注册表 + px_conn_close 关 fd）；ACTIVE/未事件化（worker 仍在 handler 或
    // 阻塞保持）→ 原语义：清注册 + shutdown 唤醒阻塞读兜底（worker 醒后见注册已清不二次 close）。
    if (px_evc_is_idle(fd)) {
        sse_server_close_fd(fd);
        return px_bool(true);
    }
    pthread_mutex_lock(&g_sse_mu);
    idx = sse_find(conn);
    if (idx >= 0) {
        g_sse_conns[idx].active = 0;
        g_sse_conns[idx].fd = -1;
        g_sse_conns[idx].conn = NULL;
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    pthread_mutex_unlock(&g_sse_mu);
    return px_bool(true);
}

// ==================== M83-S6（Issue 19 GAP-SRV-SSE）http_stream 同端口流式路由 ====================
// http_stream(path, on_connect)：把 http_serve / http_serve_unix 的同端口路由注册为流式 SSE
//   （B 形态——http 服务面与 sse_send/sse_close 通道复用）：
//     · http_conn_worker 解析请求后先查本路由表（GET + path 精确匹配）
//     · 命中 → 连接包装为 PxConn 并注册进 g_sse_conns（与 sse_serve 同一注册表/锁），
//       req 注入 conn id → 写 SSE 响应头 → 调 on_connect(req)
//     · on_connect 内语言层 sse_send(conn, chunk) 逐块推送（每块即写即刷，线程安全）；
//       可提前 sse_close(conn) 结束；on_connect 返回后本函数自动注销 + 关闭连接
//     · 普通 JSON handler 同端口共存（流式路由优先匹配）；http_serve 与 http_serve_unix 同享
//     · 限制：明文 HTTP / HTTP-over-unix（http_conn_worker 面）；px_serve（应用平台独立
//       worker）暂不接入（文档注明，后续扩展）
// （全局表 g_stream_mu/g_stream_routes 定义于 http_conn_worker 之前，见上）
// 流式路由精确匹配（调用方需持 g_stream_mu）；命中返回路由下标，否则 -1
static int stream_match(const char* path) {
    for (int i = 0; i < MAX_STREAM_ROUTES; i++) {
        if (g_stream_routes[i].active && strcmp(g_stream_routes[i].path, path) == 0) return i;
    }
    return -1;
}

// http_stream(path, on_connect) → bool
static LXValue bi_http_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR) px_error("http_stream 需要 (path, on_connect) 参数");
    const char* p = args[0].as.obj->as.str.data;
    if (p[0] != '/') px_error("http_stream 的 path 必须以 / 开头");
    LXValue fn = args[1];
    if (fn.type != PX_FUNC && fn.type != PX_NATIVE) px_error("http_stream 的 on_connect 必须是函数");
    pthread_mutex_lock(&g_stream_mu);
    int idx = -1;
    for (int i = 0; i < MAX_STREAM_ROUTES; i++) {
        if (g_stream_routes[i].active && strcmp(g_stream_routes[i].path, p) == 0) { idx = i; break; }
    }
    if (idx < 0) {
        for (int i = 0; i < MAX_STREAM_ROUTES; i++) {
            if (!g_stream_routes[i].active) { idx = i; break; }
        }
        if (idx < 0) { pthread_mutex_unlock(&g_stream_mu); return px_bool(false); }
        snprintf(g_stream_routes[idx].path, sizeof(g_stream_routes[idx].path), "%s", p);
        g_stream_routes[idx].active = 1;
    }
    pthread_mutex_unlock(&g_stream_mu);
    // handler 存全局表（GC 扫描根），连接线程经全局表取回
    char key[300];
    snprintf(key, sizeof(key), "__stream_fn_%d", idx);
    px_set_global(key, fn);
    return px_bool(true);
}

// http_stream 连接接管（http_conn_worker 线程内）：已解析的 HTTP 请求连接 → SSE 流式连接。
static LXValue stream_takeover_conn(int fd, LXValue req, int route_idx) {
    // 明文 fd → PxConn 包装（与 sse_serve 同机制：sse_send 经 px_conn_write 写出）
    PxConn* c = xmalloc(sizeof(PxConn));
    if (px_conn_init(c, fd) != 0) { xfree(c); close(fd); return px_null(); }
    g_cur_conn = c;
    __sync_fetch_and_add(&g_px_inflight, 1);

    // 分配 conn id + 注册（防 fd 复用：注册后才接受 sse_send）
    pthread_mutex_lock(&g_sse_mu);
    int64_t conn = g_sse_next_id++;
    int slot = sse_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_sse_mu);
        px_conn_close(c);
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
        return px_null();
    }
    g_sse_conns[slot].fd = fd;
    g_sse_conns[slot].id = conn;
    g_sse_conns[slot].active = 1;
    g_sse_conns[slot].conn = c;
    pthread_mutex_unlock(&g_sse_mu);
    px_dict_set(req, "conn", px_int(conn));

    // SSE 响应头（Connection: close——本连接不 keep-alive 复用）
    const char* hdr = "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream; charset=utf-8\r\n"
                      "Cache-Control: no-cache\r\nConnection: close\r\n\r\n";
    px_conn_write(c, hdr, strlen(hdr));

    // 调 on_connect(req)（路由 fn 经全局表取回，防 GC）
    char key[300];
    snprintf(key, sizeof(key), "__stream_fn_%d", route_idx);
    LXValue fn = px_get_global(key);
    if (fn.type == PX_FUNC || fn.type == PX_NATIVE) {
        px_call(fn, &req, 1);
    }

    // on_connect 返回 → 注销 + 关闭（语言层若已 sse_close，注册项已清 → 不再二次关闭）
    int still = 0;
    pthread_mutex_lock(&g_sse_mu);
    int idx = sse_find(conn);
    if (idx >= 0) {
        g_sse_conns[idx].active = 0;
        g_sse_conns[idx].fd = -1;
        g_sse_conns[idx].conn = NULL;
        still = 1;
    }
    pthread_mutex_unlock(&g_sse_mu);
    if (still) {
        px_conn_close(c);
        __sync_fetch_and_sub(&g_px_inflight, 1);
    }
    g_cur_conn = NULL;
    return px_null();
}

// 解析 SSE 事件文本（field: value 行）为 dict
static LXValue sse_parse_event_c(const char* text, int len) {
    LXValue d = px_dict();
    px_root_push();   // M92-S2c precise：sse_parse_event_c 累积 dict 登记
    PX_KEEP(d);   // M92-S2c precise：d 裸局部跨 px_dict_set/px_str 分配
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
    px_root_pop();   // M92-S2c precise
    xfree(event_buf);
    return d;
}

// sse_connect(url) → conn id | null
// M32：SSE 客户端连接核心（建立连接并填充 slot；不持锁，调用方管理 g_sse_cli_mu）
// 返回 0 成功；失败时 slot 数据未填充（调用方负责清理旧数据）。
static int sse_cli_connect_slot(int slot, const char* url, long long reconnect_ms,
                                const char* last_event_id) {
    int is_https = 0;
    const char* rest;
    if (strncmp(url, "https://", 8) == 0) {
        is_https = 1;
        rest = url + 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        rest = url + 7;
    } else {
        return -1; // 仅支持 http:// 与 https://
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
    if (hl <= 0 || hl >= (int)sizeof(host)) return -1;
    memcpy(host, rest, (size_t)hl);
    host[hl] = 0;
    char* colon = strchr(host, ':');
    if (colon) {
        *colon = 0;
        port = atoi(colon + 1);
        if (port <= 0) return -1;
    }
    int fd = -1;
    HttpsSession* tls = NULL;
    if (is_https) {
        tls = https_connect(host, port);
        if (!tls) return -1;
        fd = tls->net.fd;
    } else {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;
        struct hostent* he = gethostbyname(host);
        if (!he) { close(fd); return -1; }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)port);
        memcpy(&addr.sin_addr, he->h_addr, (size_t)he->h_length);
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return -1; }
    }
    char req[8192];
    char hosthdr[512];
    if (colon) snprintf(hosthdr, sizeof(hosthdr), "%s:%d", host, port);
    else snprintf(hosthdr, sizeof(hosthdr), "%s", host);
    int rl = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: PuXian/0.1\r\nAccept: text/event-stream\r\nConnection: close\r\nCache-Control: no-cache\r\n",
        path, hosthdr);
    if (last_event_id && *last_event_id) {
        rl += snprintf(req + rl, sizeof(req) - (size_t)rl, "Last-Event-ID: %s\r\n", last_event_id);
    }
    rl += snprintf(req + rl, sizeof(req) - (size_t)rl, "\r\n");
    if (rl <= 0 || conn_send(tls, fd, req, rl) < 0) {
        if (tls) https_close(tls); else close(fd);
        return -1;
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
        return -1;
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
        return -1;
    }
    // 填充 slot（剩余字节进 pending）
    int remain = hn - header_end;
    g_sse_clients[slot].fd = fd;
    g_sse_clients[slot].tls = tls;
    g_sse_clients[slot].active = 1;
    g_sse_clients[slot].reconnect_ms = reconnect_ms;
    snprintf(g_sse_clients[slot].url, sizeof(g_sse_clients[slot].url), "%s", url);
    snprintf(g_sse_clients[slot].last_event_id, sizeof(g_sse_clients[slot].last_event_id),
             "%s", last_event_id ? last_event_id : "");
    if (g_sse_clients[slot].pending) xfree(g_sse_clients[slot].pending);
    if (remain > 0) {
        g_sse_clients[slot].pend_cap = remain + 64;
        g_sse_clients[slot].pending = xmalloc((size_t)g_sse_clients[slot].pend_cap);
        memcpy(g_sse_clients[slot].pending, hbuf + header_end, (size_t)remain);
        g_sse_clients[slot].pend_len = remain;
    } else {
        g_sse_clients[slot].pending = NULL;
        g_sse_clients[slot].pend_len = g_sse_clients[slot].pend_cap = 0;
    }
    return 0;
}

// sse_connect(url[, reconnect_ms]) → int conn | null
// reconnect_ms>0：断线自动重连（等待该毫秒后重连，带 Last-Event-ID）
static LXValue bi_sse_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2 || args[0].type != PX_STR) px_error("sse_connect 需要 (url[, reconnect_ms]) 参数");
    const char* url = args[0].as.obj->as.str.data;
    long long reconnect_ms = 0;
    if (nargs == 2) {
        if (args[1].type != PX_INT) px_error("sse_connect 的 reconnect_ms 需要整数");
        reconnect_ms = args[1].as.i;
    }
    pthread_mutex_lock(&g_sse_cli_mu);
    int slot = sse_cli_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_sse_cli_mu);
        return px_null();
    }
    g_sse_clients[slot].active = 0;
    g_sse_clients[slot].fd = -1;
    g_sse_clients[slot].tls = NULL;
    g_sse_clients[slot].pending = NULL;
    g_sse_clients[slot].pend_len = g_sse_clients[slot].pend_cap = 0;
    pthread_mutex_unlock(&g_sse_cli_mu);
    if (sse_cli_connect_slot(slot, url, reconnect_ms, NULL) != 0) {
        // 连接失败：清理 slot
        pthread_mutex_lock(&g_sse_cli_mu);
        g_sse_clients[slot].active = 0;
        g_sse_clients[slot].fd = -1;
        if (g_sse_clients[slot].pending) { xfree(g_sse_clients[slot].pending); g_sse_clients[slot].pending = NULL; }
        g_sse_clients[slot].pend_len = g_sse_clients[slot].pend_cap = 0;
        g_sse_clients[slot].tls = NULL;
        pthread_mutex_unlock(&g_sse_cli_mu);
        return px_null();
    }
    pthread_mutex_lock(&g_sse_cli_mu);
    g_sse_clients[slot].id = g_sse_cli_next_id++;
    pthread_mutex_unlock(&g_sse_cli_mu);
    return px_int(g_sse_clients[slot].id);
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
                // M32：记录 last_event_id（自动重连时带 Last-Event-ID）
                if (g_sse_clients[idx].reconnect_ms > 0 && ev.type == PX_DICT) {
                    LXObject* eo = ev.as.obj;
                    for (int ei = 0; ei < eo->as.dict.len; ei++) {
                        if (strcmp(eo->as.dict.keys[ei], "id") == 0 && eo->as.dict.vals[ei].type == PX_STR) {
                            snprintf(g_sse_clients[idx].last_event_id,
                                     sizeof(g_sse_clients[idx].last_event_id), "%s",
                                     eo->as.dict.vals[ei].as.obj->as.str.data);
                            break;
                        }
                    }
                }
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
            // M32：断线自动重连（reconnect_ms>0：等待后重连，带 Last-Event-ID）
            pthread_mutex_lock(&g_sse_cli_mu);
            HttpsSession* t2 = NULL;
            long long rms = 0;
            char url[512] = "", eid[512] = "";
            if (g_sse_clients[idx].active) {
                rms = g_sse_clients[idx].reconnect_ms;
                snprintf(url, sizeof(url), "%s", g_sse_clients[idx].url);
                snprintf(eid, sizeof(eid), "%s", g_sse_clients[idx].last_event_id);
                g_sse_clients[idx].active = 0;
                if (g_sse_clients[idx].pending) xfree(g_sse_clients[idx].pending);
                g_sse_clients[idx].pending = NULL;
                g_sse_clients[idx].pend_len = g_sse_clients[idx].pend_cap = 0;
                t2 = g_sse_clients[idx].tls;
                g_sse_clients[idx].tls = NULL;
            }
            pthread_mutex_unlock(&g_sse_cli_mu);
            if (t2) https_close(t2); else close(fd);
            if (rms > 0 && url[0]) {
                usleep((useconds_t)(rms * 1000));
                if (sse_cli_connect_slot(idx, url, rms, eid) == 0) {
                    pthread_mutex_lock(&g_sse_cli_mu);
                    g_sse_clients[idx].id = conn;  // 保持同一 conn id
                    fd = g_sse_clients[idx].fd;
                    pthread_mutex_unlock(&g_sse_cli_mu);
                    continue;  // 重连成功，继续读
                }
            }
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
// M25 预派生 N 个 `px --worker` 解释器进程常驻复用：
//   父进程把任务帧（4 字节大端长度 + path\0env_json\0dump\0timeout）写入空闲
//   worker stdin；worker 执行目标脚本，把结果帧（exit_code\0output）写回 stdout。
// 超时 → SIGKILL + 补位；worker 崩溃 → 补位重试，最终回退旧 fork+exec 路径保底。
// M34 配置化：worker 数（PX_POOL_WORKERS，默认 4）+ 空闲回收（PX_POOL_IDLE_MS，0=不回收）
//           + 每 worker 最大请求数（PX_POOL_MAX_REQ，0=不限，防泄漏滚动重启）
#define PX_POOL_SIZE_MAX 16
static int g_px_pool_size = 4;

// 兜底路径前置声明（定义在本节之后）
static int px_run_px_child(const char* path, const char* env_json, int dump_response,
                           int timeout_ms, char** out, int* out_len, int* exit_code);

typedef struct {
    pid_t pid;
    int in_fd;    // 父→worker（worker stdin）
    int out_fd;   // worker→父（worker stdout）
    int alive;
    int busy;
    int gen;      // M32：代际（热更新滚动重启计数）
    // M34 配置化：空闲起始时间（ms，空闲回收用）+ 已处理请求数（max_req 滚动重启用）
    long long idle_since_ms;
    int req_count;
} PXWorker;

static PXWorker g_px_pool[PX_POOL_SIZE_MAX];
static pthread_mutex_t g_px_pool_mu = PTHREAD_MUTEX_INITIALIZER;
static int g_px_pool_ready = 0;
// M32 进程池热更新：脚本/二进制 mtime 变化 → 标记 reload → 空闲 worker 滚动重启
static volatile int g_px_pool_reload = 0;
static int g_pool_gen = 1;
static long long g_last_script_mtime = 0;
static long long g_last_bin_mtime = 0;
static char g_last_script_path[1024] = "";

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
#if defined(__GLIBC__)
        close_range(3, ~0U, 0);
#else
        // musl（M57-S4 交叉编译）无 close_range：循环关闭继承 fd（4096 覆盖常规上限）
        for (int fd = 3; fd < 4096; fd++) close(fd);
#endif
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
    w->gen = g_pool_gen;
    w->idle_since_ms = 0;
    w->req_count = 0;
    return 0;
}

// 惰性初始化池（首次 px_pool_run 时）
static void px_pool_init_lazy(void) {
    pthread_mutex_lock(&g_px_pool_mu);
    if (!g_px_pool_ready) {
        // M34 配置化：worker 数 / 空闲回收 / 最大请求数（环境变量，px_serve 启动可覆盖）
        const char* pw = getenv("PX_POOL_WORKERS");
        if (pw && atoi(pw) >= 1) {
            int n = atoi(pw);
            if (n > PX_POOL_SIZE_MAX) n = PX_POOL_SIZE_MAX;
            g_px_pool_size = n;
        }
        for (int i = 0; i < g_px_pool_size; i++) {
            g_px_pool[i].alive = 0;
            g_px_pool[i].busy = 0;
            g_px_pool[i].idle_since_ms = 0;
            g_px_pool[i].req_count = 0;
            if (px_pool_spawn(&g_px_pool[i]) != 0) g_px_pool[i].alive = 0;
        }
        g_px_pool_ready = 1;
    }
    pthread_mutex_unlock(&g_px_pool_mu);
}

// M34：取当前时间毫秒（进程池空闲回收用）
static long long px_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// 取空闲 worker 下标（无空闲 -1）；M34：空闲超时 worker 先退役补位
static int px_pool_take(void) {
    pthread_mutex_lock(&g_px_pool_mu);
    long long now = px_now_ms();
    long long idle_ms = 0;
    const char* pi = getenv("PX_POOL_IDLE_MS");
    if (pi && atoll(pi) > 0) idle_ms = atoll(pi);
    int idx = -1;
    for (int i = 0; i < g_px_pool_size; i++) {
        if (!g_px_pool[i].alive || g_px_pool[i].busy) continue;
        // M34 空闲回收：空闲超时 → kill + 补位（新 worker 接当前任务）
        if (idle_ms > 0 && g_px_pool[i].idle_since_ms > 0 &&
            now - g_px_pool[i].idle_since_ms > idle_ms) {
            PXWorker w = g_px_pool[i];
            g_px_pool[i].alive = 0;
            if (w.pid > 0) kill(w.pid, SIGKILL);
            if (w.in_fd > 0) close(w.in_fd);
            if (w.out_fd > 0) close(w.out_fd);
            g_px_pool[i].idle_since_ms = 0;
            if (px_pool_spawn(&g_px_pool[i]) == 0) {
                g_px_pool[i].busy = 1;
                idx = i;
            }
            break;
        }
        g_px_pool[i].busy = 1;
        g_px_pool[i].idle_since_ms = 0;
        idx = i;
        break;
    }
    pthread_mutex_unlock(&g_px_pool_mu);
    return idx;
}

static void px_pool_release(int idx) {
    pthread_mutex_lock(&g_px_pool_mu);
    if (idx >= 0 && idx < g_px_pool_size) {
        g_px_pool[idx].busy = 0;
        g_px_pool[idx].req_count++;
        g_px_pool[idx].idle_since_ms = px_now_ms();
        // M34：max_req 超限 → 退役补位（防泄漏滚动重启）
        const char* mr = getenv("PX_POOL_MAX_REQ");
        long long max_req = mr ? atoll(mr) : 0;
        int retire = 0;
        if (max_req > 0 && g_px_pool[idx].req_count >= max_req) retire = 1;
        // M32 热更新：该 worker 是旧代 → 处理完当前任务后滚动重启（kill + 补位）
        if (g_px_pool_reload && g_px_pool[idx].gen < g_pool_gen) retire = 1;
        if (retire) {
            PXWorker w = g_px_pool[idx];
            g_px_pool[idx].alive = 0;
            g_px_pool[idx].req_count = 0;
            g_px_pool[idx].idle_since_ms = 0;
            if (w.pid > 0) kill(w.pid, SIGKILL);
            if (w.in_fd > 0) close(w.in_fd);
            if (w.out_fd > 0) close(w.out_fd);
            if (px_pool_spawn(&g_px_pool[idx]) != 0) g_px_pool[idx].alive = 0;
        }
        // 全部 worker 已是新代 → 清除 reload 标志
        int all_new = 1;
        for (int i = 0; i < g_px_pool_size; i++) {
            if (g_px_pool[i].alive && g_px_pool[i].gen < g_pool_gen) { all_new = 0; break; }
        }
        if (all_new) g_px_pool_reload = 0;
    }
    pthread_mutex_unlock(&g_px_pool_mu);
}

// M32：检测脚本/px 二进制 mtime 变化 → 标记热更新（进程池滚动重启）
static void px_pool_check_hot_reload(const char* path) {
    struct stat st;
    if (path && *path && stat(path, &st) == 0) {
        long long mt = (long long)st.st_mtime;
        if (g_last_script_path[0] == 0) {
            snprintf(g_last_script_path, sizeof(g_last_script_path), "%s", path);
            g_last_script_mtime = mt;
        } else if (strcmp(g_last_script_path, path) == 0 && mt != g_last_script_mtime) {
            g_last_script_mtime = mt;
            g_pool_gen++;              // 新一代
            g_px_pool_reload = 1;      // 脚本变更 → 滚动重启 worker
        }
    }
    const char* bin = px_px_bin();
    if (bin && stat(bin, &st) == 0) {
        long long bmt = (long long)st.st_mtime;
        if (g_last_bin_mtime == 0) {
            g_last_bin_mtime = bmt;
        } else if (bmt != g_last_bin_mtime) {
            g_last_bin_mtime = bmt;
            g_pool_gen++;
            g_px_pool_reload = 1;      // px 二进制替换 → 滚动重启（新二进制生效）
        }
    }
}

// 杀掉并补位 worker（调用方此后不得再引用该下标）
static void px_pool_respawn(int idx) {
    pthread_mutex_lock(&g_px_pool_mu);
    if (idx < 0 || idx >= g_px_pool_size) { pthread_mutex_unlock(&g_px_pool_mu); return; }
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
    // M32：热更新检测——脚本或 px 二进制变更 → 滚动重启 worker（worker 本身每次任务
    // 重新读 .px 文件已天然热更新；滚动重启保证 worker 状态也刷新）
    px_pool_check_hot_reload(path);
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
// M101：session cache 线程安全包装（mbedtls 3.6.2 库内无锁——MBEDTLS_THREADING_C 关；
// 多 worker 并发握手 get/set 同一 g_srv_tls_cache 链表会损坏/错会话数据。自定义
// get/set 包 g_srv_cache_mu，经 mbedtls_ssl_conf_session_cache 注册替换库函数直传。
// 3.6.2 回调签名：get_t/set_t = (void*, const unsigned char*, size_t, session)）。
static int px_srv_cache_get(void* p, const unsigned char* session_id, size_t len,
                            mbedtls_ssl_session* session) {
    pthread_mutex_lock(&g_srv_cache_mu);
    int r = mbedtls_ssl_cache_get((mbedtls_ssl_cache_context*)p, session_id, len, session);
    pthread_mutex_unlock(&g_srv_cache_mu);
    return r;
}
static int px_srv_cache_set(void* p, const unsigned char* session_id, size_t len,
                            const mbedtls_ssl_session* session) {
    pthread_mutex_lock(&g_srv_cache_mu);
    int r = mbedtls_ssl_cache_set((mbedtls_ssl_cache_context*)p, session_id, len, session);
    pthread_mutex_unlock(&g_srv_cache_mu);
    return r;
}
// M101：RSA 私钥 per-连接 clone。mbedtls 3.6.2 无 mbedtls_pk_copy（PSA 化移除）；
// legacy PK 后端启用（USE_PSA_CRYPTO 关）→ pk_setup(PK_RSA) + mbedtls_rsa_copy 深拷贝
// 出独立 mbedtls_rsa_context（签名时 CRT 推导/窗口缓存写各自 ctx → 并发握手无共享写）。
// 返回 malloc 的 mbedtls_pk_context*（调用方 px_conn_close 释放）；非 RSA → NULL（EC
// 私钥签名并发安全实测 → 共享只读）；RSA clone 失败（OOM）→ NULL（调用方退化全局
// 握手锁 g_srv_hs_mu 保底）。调用方应持 g_srv_tls_mu（防源 key 并发重注册覆盖）。
static mbedtls_pk_context* px_pk_clone_rsa(const mbedtls_pk_context* src) {
    if (mbedtls_pk_get_type(src) != MBEDTLS_PK_RSA) return NULL;
    mbedtls_pk_context* dst = (mbedtls_pk_context*)malloc(sizeof(mbedtls_pk_context));
    if (!dst) return NULL;
    mbedtls_pk_init(dst);
    // 3.6.2 pk_setup 第二参是 const mbedtls_pk_info_t*（描述符）而非枚举 → 用
    // mbedtls_pk_info_from_type 取 RSA info（直接传枚举值会当指针解引用段错误）
    const mbedtls_pk_info_t* info = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);
    if (!info || mbedtls_pk_setup(dst, info) != 0 ||
        mbedtls_rsa_copy(mbedtls_pk_rsa(*dst), mbedtls_pk_rsa(*src)) != 0) {
        mbedtls_pk_free(dst);
        free(dst);
        return NULL;
    }
    return dst;
}

// M33：TLS SNI 回调——按 ClientHello 域名从 g_sni_certs 选证书（无匹配 → 默认证书，返回 0）
// M101：p_ctx = PxConn*（conf_sni 传入 c）；命中 slot 且其 key 为 RSA → 锁内 clone 到
//   c->own_pk_sni（per-连接独立私钥，消除与其它 worker 并发签名共享 g_sni_certs[i].key
//   的 data race）；EC key → 共享只读（签名并发安全实测）；RSA clone 失败 → 返回错误
//   （握手失败保守，宁失败不竞态——已在握手中途无法退 g_srv_hs_mu）。
static int px_sni_cb(void* p_ctx, mbedtls_ssl_context* ssl, const unsigned char* name, size_t len) {
    PxConn* c = (PxConn*)p_ctx;
    char host[256];
    size_t cl = len < 255 ? len : 255;
    memcpy(host, name, cl);
    host[cl] = 0;
    int slot = -1;
    pthread_mutex_lock(&g_srv_tls_mu);
    for (int i = 0; i < PX_MAX_SNI_CERTS; i++) {
        if (g_sni_certs[i].active && strcasecmp(g_sni_certs[i].hostname, host) == 0) {
            slot = i;
            break;
        }
    }
    int rc = 0;
    if (slot >= 0) {
        mbedtls_pk_context* use = &g_sni_certs[slot].key;
        if (mbedtls_pk_get_type(use) == MBEDTLS_PK_RSA) {
            mbedtls_pk_context* own = px_pk_clone_rsa(use);
            if (own) {
                c->own_pk_sni = own;
                use = own;
            } else {
                rc = MBEDTLS_ERR_SSL_ALLOC_FAILED;  // RSA clone 失败 → 握手失败保守
            }
        }
        if (rc == 0) rc = mbedtls_ssl_set_hs_own_cert(ssl, &g_sni_certs[slot].cert, use);
    }
    pthread_mutex_unlock(&g_srv_tls_mu);
    return rc;
}

// 服务端 TLS：accept 后 px_conn_init 做 mbedtls 服务端握手（若 tls_server 已注册）。
// M101-final：**全局握手串行锁** g_srv_hs_mu —— mbedtls 3.6.2 预编译库无线程支持
// （MBEDTLS_THREADING_C 关），除私钥签名（per-conn RSA clone 已消除）与 session cache
// （加锁包装已消除）外，TLS1.3 服务端仍存在无法枚举的深层共享（并发实测 RSA-TLS1.3
// 48×3 残余 ~1% MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED(-110) + 偶发堆损坏；串行 300
// 次零失败 → 并发特有）。整个握手持 g_srv_hs_mu → 握手期对 mbedtls 无任何跨线程共享
// 访问。clone + cache 锁保留作双保险（未来若换开 threading 的 mbedtls 可去掉串行锁仍
// 正确）。keep-alive 连接握手仅一次，不受串行影响；握手本地毫秒级，新连接突发排队
// 可接受（边缘/内部 px_serve 以 keep-alive 为主）。锁序：hs_mu → tls_mu（sni_cb 同
// 序）→ 无死锁；tls_server 注册仅持 tls_mu 不碰 hs_mu。
static int px_conn_tls_handshake_locked(PxConn* c) {
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
    // M101：私钥 clone / 共享决策（已持 g_srv_hs_mu → 无并发；clone 仍作双保险：
    //   RSA → per-连接独立 ctx（未来去串行锁仍正确）；EC → 共享只读实测安全）
    pthread_mutex_lock(&g_srv_tls_mu);
    mbedtls_pk_context* own = px_pk_clone_rsa(&g_srv_key);
    int oc;
    if (own) {
        c->own_pk = own;
        oc = mbedtls_ssl_conf_own_cert(conf, &g_srv_cert, own);
    } else {
        oc = mbedtls_ssl_conf_own_cert(conf, &g_srv_cert, &g_srv_key);
    }
    pthread_mutex_unlock(&g_srv_tls_mu);
    if (oc != 0) return -1;
    // M33：TLS SNI——按 ClientHello 域名选择证书（多证书共服；p_ctx=PxConn* 供 SNI clone）
    mbedtls_ssl_conf_sni(conf, px_sni_cb, c);
    // M30/M101：服务端 https 连接池——全局 TLS 会话缓存（加锁包装 get/set，并发安全）
    if (g_srv_tls_cache_init) {
        mbedtls_ssl_conf_session_cache(conf, &g_srv_tls_cache,
                                       px_srv_cache_get, px_srv_cache_set);
    }
    // TLS 1.2 会话票据（与客户端 M25 票据恢复对偶）
    mbedtls_ssl_conf_session_tickets(conf, MBEDTLS_SSL_SESSION_TICKETS_ENABLED);
    // M31.4a：HTTP/2 预检——ALPN 固定 http/1.1（客户端探测 h2 时明确协商 http/1.1）
    // M31.4a/M37：ALPN 声明 h2 + http/1.1（客户端可选 HTTP/2；握手后按协商协议分发）
    // M-B9b：固定 http/1.1 —— vhost handler 仅在 http/1.1 路径生效；h2 帧循环（M37）绕过 vhost，
    //         生产 Web 服务（多站点/deny/SPA/反代）必须走 handler，故 ALPN 只声明 http/1.1。
    static const char* alpn_list[] = { "http/1.1", NULL };
    mbedtls_ssl_conf_alpn_protocols(conf, alpn_list);
    if (mbedtls_ssl_setup(ssl, conf) != 0) return -1;
    mbedtls_ssl_set_bio(ssl, &c->fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    int ret;
    while ((ret = mbedtls_ssl_handshake(ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) return -1;
    }
    return 0;
}

static int px_conn_tls_handshake(PxConn* c) {
    pthread_mutex_lock(&g_srv_hs_mu);
    int rc = px_conn_tls_handshake_locked(c);
    pthread_mutex_unlock(&g_srv_hs_mu);
    return rc;
}

// 初始化连接（fd 上 TLS 握手若已注册服务端证书；失败返回 -1，连接应关闭）
int px_conn_init(PxConn* c, int fd) {
    memset(c, 0, sizeof(*c));
    c->fd = fd;
    c->is_tls = 0;
    c->owned = 1;
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
    if (!c->is_tls) {
        // M33 修复：send 可能部分发送（TCP 缓冲满/非阻塞）→ 循环发送到写完，
        // 否则响应头/体错位（客户端读到上一响应残留字节，双模式不一致根因之一）
        size_t sent = 0;
        while (sent < n) {
            ssize_t k = send(c->fd, (const char*)buf + sent, n - sent, MSG_NOSIGNAL);
            if (k <= 0) return sent > 0 ? (ssize_t)sent : -1;
            sent += (size_t)k;
        }
        return (ssize_t)sent;
    }
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
    // M101：释放条件由「is_tls」放宽为「owned && ssl」——握手失败路径（px_conn_init 中
    //   px_conn_tls_handshake 返回 -1 时 is_tls 尚未置 1）原实现直接跳过释放 ssl/conf/
    //   drbg/entropy（每失败连接泄漏 ~20KB+），并发握手失败高频时泄漏严重；owned=1 时
    //   ssl 非空即本连接 malloc 的 TLS 状态。close_notify 仅在握手完成后发（未完成握手
    //   直接 free，避免对已关对端写 alert）。own_pk/own_pk_sni（M101 per-连接 RSA 私钥
    //   clone）随连接释放：先 ssl_free/config_free（内部引用 conf/key 结束）再 pk_free。
    if (c->owned && c->ssl) {
        if (c->is_tls) {
            mbedtls_ssl_close_notify((mbedtls_ssl_context*)c->ssl);
        }
        mbedtls_ssl_free((mbedtls_ssl_context*)c->ssl);
        mbedtls_ssl_config_free((mbedtls_ssl_config*)c->conf);
        mbedtls_ctr_drbg_free((mbedtls_ctr_drbg_context*)c->ctr_drbg);
        mbedtls_entropy_free((mbedtls_entropy_context*)c->entropy);
        if (c->own_pk) {
            mbedtls_pk_free((mbedtls_pk_context*)c->own_pk);
            free(c->own_pk);
            c->own_pk = NULL;
        }
        if (c->own_pk_sni) {
            mbedtls_pk_free((mbedtls_pk_context*)c->own_pk_sni);
            free(c->own_pk_sni);
            c->own_pk_sni = NULL;
        }
        free(c->ssl); free(c->conf); free(c->ctr_drbg); free(c->entropy);
        c->ssl = c->conf = c->ctr_drbg = c->entropy = NULL;
        // owned=0：TLS 状态由外部（HttpsSession）管理，px_https_close_ex 统一释放
    }
    if (c->fd >= 0) { close(c->fd); c->fd = -1; }
    if (c->owned) c->is_tls = 0;
}

// tls_server(cert, key[, hostname])：注册服务端 TLS（cert/key 为 PEM 路径或 PEM 内容）→ bool
// M33：带 hostname → 加入 SNI 证书表（按 ClientHello 域名选择）；无 hostname → 默认证书。
static LXValue bi_tls_server(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 && nargs != 3) {
        px_error("tls_server 需要 (cert_pem, key_pem[, hostname]) 参数");
    }
    if (args[0].type != PX_STR || args[1].type != PX_STR) {
        px_error("tls_server 需要 (cert_pem, key_pem) 参数");
    }
    const char* cert = args[0].as.obj->as.str.data;
    const char* key = args[1].as.obj->as.str.data;
    const char* hostname = (nargs == 3 && args[2].type == PX_STR)
        ? args[2].as.obj->as.str.data : NULL;
    pthread_mutex_lock(&g_srv_tls_mu);
    int rc1, rc2;
    if (hostname && *hostname) {
        // SNI 证书：找同名覆盖或空槽
        int slot = -1;
        for (int i = 0; i < PX_MAX_SNI_CERTS; i++) {
            if (g_sni_certs[i].active && strcasecmp(g_sni_certs[i].hostname, hostname) == 0) { slot = i; break; }
            if (!g_sni_certs[i].active && slot < 0) slot = i;
        }
        if (slot < 0) {
            pthread_mutex_unlock(&g_srv_tls_mu);
            px_error("SNI 证书数量超出上限 %d", PX_MAX_SNI_CERTS);
        }
        if (g_sni_certs[slot].active) {
            mbedtls_x509_crt_free(&g_sni_certs[slot].cert);
            mbedtls_pk_free(&g_sni_certs[slot].key);
        }
        memset(&g_sni_certs[slot], 0, sizeof(PxSniCert));
        mbedtls_x509_crt_init(&g_sni_certs[slot].cert);
        mbedtls_pk_init(&g_sni_certs[slot].key);
        rc1 = strstr(cert, "-----BEGIN")
            ? mbedtls_x509_crt_parse(&g_sni_certs[slot].cert, (const unsigned char*)cert, strlen(cert) + 1)
            : mbedtls_x509_crt_parse_file(&g_sni_certs[slot].cert, cert);
        rc2 = strstr(key, "-----BEGIN")
            ? mbedtls_pk_parse_key(&g_sni_certs[slot].key, (const unsigned char*)key, strlen(key) + 1, NULL, 0, NULL, NULL)
            : mbedtls_pk_parse_keyfile(&g_sni_certs[slot].key, key, NULL, NULL, NULL);
        if (rc1 != 0 || rc2 != 0) {
            char eb[256];
            mbedtls_strerror(rc1 != 0 ? rc1 : rc2, eb, sizeof(eb));
            pthread_mutex_unlock(&g_srv_tls_mu);
            px_error("tls_server(%s): 证书/私钥解析失败: %s", hostname, eb);
        }
        // 域名规范化（小写、去尾点）
        snprintf(g_sni_certs[slot].hostname, sizeof(g_sni_certs[slot].hostname), "%s", hostname);
        size_t hl = strlen(g_sni_certs[slot].hostname);
        for (size_t i = 0; i < hl; i++) {
            char c = g_sni_certs[slot].hostname[i];
            if (c >= 'A' && c <= 'Z') g_sni_certs[slot].hostname[i] = (char)(c - 'A' + 'a');
        }
        while (hl > 0 && g_sni_certs[slot].hostname[hl - 1] == '.') { g_sni_certs[slot].hostname[--hl] = 0; }
        g_sni_certs[slot].active = 1;
    } else {
        // 默认证书（覆盖）
        mbedtls_x509_crt_free(&g_srv_cert);
        mbedtls_pk_free(&g_srv_key);
        mbedtls_x509_crt_init(&g_srv_cert);
        mbedtls_pk_init(&g_srv_key);
        rc1 = strstr(cert, "-----BEGIN")
            ? mbedtls_x509_crt_parse(&g_srv_cert, (const unsigned char*)cert, strlen(cert) + 1)
            : mbedtls_x509_crt_parse_file(&g_srv_cert, cert);
        rc2 = strstr(key, "-----BEGIN")
            ? mbedtls_pk_parse_key(&g_srv_key, (const unsigned char*)key, strlen(key) + 1, NULL, 0, NULL, NULL)
            : mbedtls_pk_parse_keyfile(&g_srv_key, key, NULL, NULL, NULL);
        if (rc1 != 0 || rc2 != 0) {
            char eb[256];
            mbedtls_strerror(rc1 != 0 ? rc1 : rc2, eb, sizeof(eb));
            pthread_mutex_unlock(&g_srv_tls_mu);
            px_error("tls_server: 证书/私钥解析失败: %s", eb);
        }
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

// M33：结构化访问日志落盘——stderr 输出 + 写文件（px_serve opts{access_log}）+ 大小轮转
#define PX_ACCESS_LOG_MAX (10 * 1024 * 1024)
// M36：普通行 → JSON 行（[px-access] ts remote method path status bytes ms req=id）
static void px_access_log_jsonify(const char* line, char* out, size_t n) {
    long long ts = 0, status = 0, bytes = 0;
    char remote[128] = "", method[32] = "", path[512] = "", ms[32] = "", req[128] = "";
    sscanf(line, "[px-access] %lld %127s %31s %511s %lld %lld %31s req=%127s",
           &ts, remote, method, path, &status, &bytes, ms, req);
    snprintf(out, n,
             "{\"ts\":%lld,\"remote\":\"%s\",\"method\":\"%s\",\"path\":\"%s\","
             "\"status\":%lld,\"bytes\":%lld,\"ms\":%s,\"req\":\"%s\"}\n",
             ts, remote, method, path, status, bytes, ms, req);
}

void px_access_log(const char* fmt, ...) {
    char line[2048];
    char out_line[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    // M36：JSON 行格式（从普通行转 JSON，供 log_json:true 使用）
    if (g_px_log_json && strncmp(line, "[px-access]", 11) == 0) {
        px_access_log_jsonify(line, out_line, sizeof(out_line));
        fputs(out_line, stderr);
    } else {
        fputs(line, stderr);
    }
    if (!g_px_access_log[0]) return;
    // M36：按天轮转——文件名带日期后缀（YYYYMMDD）
    char logpath[1100];
    if (g_px_log_daily) {
        time_t now = time(NULL);
        struct tm tmv;
        gmtime_r(&now, &tmv);
        snprintf(logpath, sizeof(logpath), "%s.%04d%02d%02d", g_px_access_log,
                 tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday);
    } else {
        snprintf(logpath, sizeof(logpath), "%s", g_px_access_log);
    }
    // 轮转：>10MB → .1/.2/.3（保留 3 份）
    struct stat st;
    if (stat(logpath, &st) == 0 && st.st_size > PX_ACCESS_LOG_MAX) {
        for (int i = 3; i >= 1; i--) {
            char src[1100], dst[1100];
            if (i == 1) {
                snprintf(src, sizeof(src), "%s", logpath);
            } else {
                snprintf(src, sizeof(src), "%s.%d", logpath, i - 1);
            }
            snprintf(dst, sizeof(dst), "%s.%d", logpath, i);
            if (access(src, F_OK) == 0) rename(src, dst);
        }
    }
    int fd = open(logpath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        const char* w = g_px_log_json ? out_line : line;
        (void)write(fd, w, strlen(w));
        close(fd);
    }
}

// ==================== M53-S2：PxHttpOut HTTP/1.1 实现 ====================
// PxHttpOut（runtime.h）是请求管道与传输之间的输出抽象：HTTP/1.1 实现把
// status/ct/body_len/extra 格式化为 HTTP/1.1 文本头经 PxConn 写出（明文/TLS 统一），
// 与旧 px_px_send_ex/send(fd) 路径逐字节一致（含 Alt-Svc 注入去重）。
// HTTP/3 实现（runtime_h3.c，S3）把同一参数编码为 HEADERS + DATA 帧 —— 管道无需改动。
static void px_out11_begin(PxHttpOut* o, int status, const char* ct, long long body_len,
                           int head_only, int keep_alive, const char* extra_headers) {
    (void)head_only;  // HTTP/1.1 响应头总是带 Content-Length；HEAD 由上层不写 body 控制
    PxConn* c = (PxConn*)o->impl;
    const char* reason = px_http_status_reason(status);
    char head[2048];
    int off = snprintf(head, sizeof(head),
                       "HTTP/1.1 %d %s\r\nContent-Length: %lld\r\nConnection: %s\r\n",
                       status, reason, body_len, keep_alive ? "keep-alive" : "close");
    if (ct && *ct) off += snprintf(head + off, sizeof(head) - (size_t)off, "Content-Type: %s\r\n", ct);
    if (extra_headers && *extra_headers) {
        int l = (int)strlen(extra_headers);
        if (off + l < (int)sizeof(head)) { memcpy(head + off, extra_headers, (size_t)l); off += l; }
    }
    // M33：Alt-Svc 通告（HTTP/3 协商；px_serve opts{alt_svc}，extra 未含时统一注入）
    if (g_px_alt_svc[0] && !(extra_headers && strstr(extra_headers, "Alt-Svc:"))) {
        off += snprintf(head + off, sizeof(head) - (size_t)off, "Alt-Svc: %s\r\n", g_px_alt_svc);
    }
    off += snprintf(head + off, sizeof(head) - (size_t)off, "\r\n");
    px_conn_write(c, head, (size_t)off);
}

static int px_out11_write(PxHttpOut* o, const void* buf, size_t n) {
    if (n == 0) return 0;
    PxConn* c = (PxConn*)o->impl;
    return (int)px_conn_write(c, buf, n);
}

static void px_out11_end(PxHttpOut* o) {
    (void)o;  // HTTP/1.1 无帧收尾（H3 在此发 DATA FIN）
}

static void px_out11_respond(PxHttpOut* o, int status, const char* ct, const char* body,
                             int body_len, int head_only, int keep_alive,
                             const char* extra_headers) {
    o->begin(o, status, ct, (long long)body_len, head_only, keep_alive, extra_headers);
    if (!head_only && body_len > 0) o->write(o, body, (size_t)body_len);
    o->end(o);
}

void px_http_out_init_conn(PxHttpOut* o, PxConn* c) {
    memset(o, 0, sizeof(*o));
    o->impl = c;
    o->respond = px_out11_respond;
    o->begin = px_out11_begin;
    o->write = px_out11_write;
    o->end = px_out11_end;
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
    if (body_len < g_px_gzip_min) return 0;
    LXValue ae = px_header_get(headers, "Accept-Encoding");
    if (ae.type != PX_STR || !strstr(ae.as.obj->as.str.data, "gzip")) return 0;
    if (!ct || !*ct) return 1;
    if (strncasecmp(ct, "text/", 5) == 0) return 1;
    if (strstr(ct, "json") || strstr(ct, "javascript") || strstr(ct, "xml") ||
        strstr(ct, "svg") || strstr(ct, "csv")) return 1;
    return 0;
}


// ==================== M98-S2b：vhost handler 响应（同步/异步段2 共用） ====================
// vhost handler 返回非 null → vhost_normalize（M57-S7 白名单响应头）+ respond；返回
// null → 空操作（调用方续 docroot 管道）。vhost 历史语义：无访问日志。
static void px_vhost_respond(PxHttpOut* pout, const char* method, int head_only,
                             int keep_alive, const char* req_id, LXValue req, LXValue r) {
    if (r.type == PX_NULL) return;
    // M58-S1 补齐（mahesvara ISSUE-01 2026-09-10）：vhost handler 返回 dict 含 "file": path
    // → C 层 64KB 分块流式发送 + Range 206。此前 file 键被忽略（仅 http_conn_worker 直连
    // 模式实现）→ 大文件 HTTP 200 空 body（soft.wsai.chat 发布源 ws update 下载失败）。
    {
        LXValue file_v = (r.type == PX_DICT) ? px_dict_get(r, "file") : px_null();
        if (file_v.type == PX_STR) {
            const char* fpath = file_v.as.obj->as.str.data;
            struct stat fst;
            if (stat(fpath, &fst) != 0 || !S_ISREG(fst.st_mode)) {
                char e404[256];
                snprintf(e404, sizeof(e404), "X-Request-Id: %s\r\n", req_id);
                pout->respond(pout, 404, "text/plain; charset=utf-8", "404 Not Found", 13,
                              head_only, keep_alive, e404);
                return;
            }
            long long fsz = (long long)fst.st_size;
            long long rstart = 0, rend = fsz - 1;
            int is_range = 0;
            LXValue headers = px_dict_get(req, "headers");
            if (headers.type == PX_DICT) {
                LXValue rv2 = px_header_get(&headers, "Range");
                if (rv2.type == PX_STR && fsz > 0) {
                    if (px_parse_range(rv2.as.obj->as.str.data, fsz, &rstart, &rend)) is_range = 1;
                }
            }
            long long seg_len = rend - rstart + 1;
            int fst_code = is_range ? 206 : 200;
            const char* fct = px_mime_type(fpath);
            char fextra[1536];
            int feo = snprintf(fextra, sizeof(fextra), "X-Request-Id: %s\r\n", req_id);
            if (feo < 0 || feo >= (int)sizeof(fextra)) feo = (int)sizeof(fextra) - 1;
            LXValue fh = px_dict_get(r, "headers");
            if (fh.type == PX_DICT) {
                LXObject* fo = fh.as.obj;
                for (int fi = 0; fi < fo->as.dict.len; fi++) {
                    LXValue hv = fo->as.dict.vals[fi];
                    if (hv.type != PX_STR) continue;
                    const char* hk = fo->as.dict.keys[fi];
                    if (!hk) continue;
                    if (strcasecmp(hk, "Content-Type") == 0) {
                        fct = hv.as.obj->as.str.data;
                        continue;
                    }
                    if (!px_vhost_header_allowed(hk)) continue;
                    const char* hvv = hv.as.obj->as.str.data;
                    size_t klen = strlen(hk), vlen = hv.as.obj->as.str.len;
                    if (memchr(hk, '\r', klen) || memchr(hk, '\n', klen)) continue;
                    if (memchr(hvv, '\r', vlen) || memchr(hvv, '\n', vlen)) continue;
                    int cur = (int)strlen(fextra);
                    long long need = (long long)klen + 2 + (long long)vlen + 2;
                    if ((long long)cur + need < (long long)sizeof(fextra))
                        snprintf(fextra + cur, (size_t)(sizeof(fextra) - cur), "%s: %s\r\n", hk, hvv);
                }
            }
            if (is_range) {
                int cur2 = (int)strlen(fextra);
                snprintf(fextra + cur2, sizeof(fextra) - (size_t)cur2,
                         "Content-Range: bytes %lld-%lld/%lld\r\nAccept-Ranges: bytes\r\n",
                         rstart, rend, fsz);
            }
            pout->begin(pout, fst_code, fct, seg_len, head_only, keep_alive, fextra);
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
                        pout->write(pout, chunk, got);
                        remain -= (long long)got;
                    }
                    fclose(f);
                }
            }
            pout->end(pout);
            return;
        }
    }
    int vst = 200;
    const char* vct = "text/plain; charset=utf-8";
    const char* vbody = "";
    int vblen = 0;
    char extra[1024];
    int extra_off = snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
    if (extra_off < 0 || extra_off >= (int)sizeof(extra)) extra_off = (int)sizeof(extra) - 1;
    // M57-S7：vhost handler dict 自定义响应头（Location/Cache-Control/Set-Cookie/CORS 等）
    // 白名单透传——修复 BUG_REPORT：此前仅透传 Content-Type，其余响应头全丢
    px_vhost_normalize(r, &vst, &vct, &vbody, &vblen,
                       extra + extra_off, (int)sizeof(extra) - extra_off);
    pout->respond(pout, vst, vct, vbody, vblen, head_only, keep_alive, extra);
    (void)method;
}
// ==================== M53-S2：公共请求管道（px_http_dispatch） ====================
// req dict 就绪（method/path/query/headers/body/cookie/form/remote）→ 输出响应。
// 纯重构自 px_conn_worker（HTTP/1.1 请求处理）：所有响应经 PxHttpOut 抽象写出，
// 不再触碰 fd/g_cur_conn —— HTTP/1.1（TCP/TLS）与 HTTP/3（H3 帧）共用本管道。
// 行为与重构前逐字节一致：CORS/限流/vhost/路由/静态(gzip/ETag/Range/304)/.px/访问日志全保留。
// M98-S2a：公共管道返回码 —— 0 = 已同步处理完成（响应已发送/短路）；1 = 已拆段
//   （route VM handler 帧协程运行中，调用方须释放 worker，done 回调投回续处理）。
//   async_ok=1：仅 px_serve HTTP/1.1 池 worker 传（route VM handler 可拆段异步执行）；
//   H3（px_http_dispatch_h3）与其余调用传 0 → 原同步路径逐字节零变化。
static int px_http_dispatch(PxHttpOut* pout, LXValue req, const char* method,
                             const char* path, const char* query,
                             int client_keep_alive, const char* req_id, int async_ok,
                             int skip_pre) {
    LXValue headers = px_dict_get(req, "headers");
    const char* log_remote = "unknown";
    {
        LXValue lr = px_dict_get(req, "remote");
        if (lr.type == PX_STR) log_remote = lr.as.obj->as.str.data;
    }
    // M98-S2b：skip_pre=1 → 跳过 CORS/限流/vhost 段1（vhost handler null 回退续管道重入；
    //   段1 已在首次 dispatch 执行 —— 重复执行会 CORS 预检双计 / 限流双计 / vhost 重解析）
    if (!skip_pre) {
    // M31.4a：CORS 预检——OPTIONS + Access-Control-Request-Method/Origin → 204 + CORS 头
    if (strcmp(method, "OPTIONS") == 0 &&
        (px_header_get(&headers, "Access-Control-Request-Method").type == PX_STR ||
         px_header_get(&headers, "Origin").type == PX_STR)) {
        char extra[512];
        snprintf(extra, sizeof(extra),
                 "X-Request-Id: %s\r\nAccess-Control-Allow-Origin: *\r\n"
                 "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, PATCH, OPTIONS\r\n"
                 "Access-Control-Allow-Headers: Content-Type, Authorization, X-Request-Id\r\n"
                 "Access-Control-Max-Age: 86400\r\n", req_id);
        pout->respond(pout, 204, "text/plain; charset=utf-8", "", 0,
                      strcmp(method, "HEAD") == 0, client_keep_alive, extra);
        return 0;
    }
    // M31.3：服务端内置限流（按 IP；px_serve opts{rate_limit:{max,window_sec}} → 429）
    if (g_px_rate_max > 0 && g_px_rate_window > 0) {
        char ipbuf[64];
        {
            LXValue rmt = px_dict_get(req, "remote");
            const char* rs = (rmt.type == PX_STR) ? rmt.as.obj->as.str.data : "";
            snprintf(ipbuf, sizeof(ipbuf), "%s", rs);
            char* colon = strrchr(ipbuf, ':');
            if (colon && colon[1] >= '0' && colon[1] <= '9') *colon = 0; // 去端口
            if (ipbuf[0] == '[') { char* br = strchr(ipbuf, ']'); if (br) { br++; *br = 0; memmove(ipbuf, ipbuf + 1, strlen(ipbuf)); } }
        }
        // M35：白名单 IP 直接放行（不计数）
        int whitelisted = 0;
        for (int wi = 0; wi < g_px_rate_whitelist_n; wi++) {
            if (strcmp(g_px_rate_whitelist[wi], ipbuf) == 0) { whitelisted = 1; break; }
        }
        if (!whitelisted) {
            // M35：多维 key（ip / ip|ua / ip|path / ip|ua|path）
            char rkey[512];
            snprintf(rkey, sizeof(rkey), "%s", ipbuf);
            if (strstr(g_px_rate_key_mode, "ua")) {
                LXValue uav = px_header_get(&headers, "User-Agent");
                const char* ua = (uav.type == PX_STR) ? uav.as.obj->as.str.data : "";
                char tmp[512];
                snprintf(tmp, sizeof(tmp), "%s|%s", rkey, ua);
                snprintf(rkey, sizeof(rkey), "%s", tmp);
            }
            if (strstr(g_px_rate_key_mode, "path")) {
                char tmp[700];
                snprintf(tmp, sizeof(tmp), "%s|%s", rkey, path);
                snprintf(rkey, sizeof(rkey), "%s", tmp);
            }
            if (!px_rate_limit_try(rkey, g_px_rate_max, g_px_rate_window)) {
                char extra[256];
                snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                pout->respond(pout, 429, "text/plain; charset=utf-8", "429 Too Many Requests",
                              21, strcmp(method, "HEAD") == 0, client_keep_alive, extra);
                px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                        (long long)time(NULL), ipbuf, method, path, 429, 21, req_id);
                return 0;
            }
        }
    }
    // M31.2：虚拟主机（Host 头路由）——docroot 覆盖 + handler 优先（null → 继续默认）
    {
        LXValue root_v = px_get_global("__px_docroot");
        const char* def_root = (root_v.type == PX_STR) ? root_v.as.obj->as.str.data : ".";
        char vroot[1024];
        snprintf(vroot, sizeof(vroot), "%s", def_root);
        LXValue vhandler;
        int has_vhandler = 0;
        LXValue host_hdr = px_header_get(&headers, "Host");
        const char* host_hdrs = (host_hdr.type == PX_STR) ? host_hdr.as.obj->as.str.data : NULL;
        if (px_vhost_resolve(host_hdrs, def_root, vroot, sizeof(vroot), &vhandler, &has_vhandler)) {
            if (has_vhandler) {
                // M98-S2b：async_ok + VM vhost handler → 挂起登记 + 帧协程 spawn（占协程
                //   不占 worker）；done 回调投回续处理（段2 kind=1：respond / null 回退）。
                //   同步路径（非 px_serve 连接 / 非 VM handler / 无协程内核 / 槽忙）零变化。
                if (async_ok && vhandler.type == PX_FUNC && vhandler.as.obj &&
                    vhandler.as.obj->as.func.fn == px_vm_entry) {
                    LXValue hargs[1];
                    hargs[0] = req;
                    if (px_pxserve_defer(pout, req, vhandler, hargs, 1, 1, vroot,
                                         strcmp(method, "HEAD") == 0,
                                         client_keep_alive, req_id))
                        return 1;   // 已拆段：调用方（px_conn_worker）释放 worker，不发送响应
                }
                LXValue r = px_call(vhandler, &req, 1);
                px_root_push();   // M92-S2c precise：vhost handler 登记作用域
                PX_KEEP(r);   // M92-S2c precise：vhost handler 返回值（normalize/respond 期间使用）
                if (r.type != PX_NULL) {
                    px_vhost_respond(pout, method, strcmp(method, "HEAD") == 0,
                                     client_keep_alive, req_id, req, r);
                    px_root_pop();   // M92-S2c precise
                    return 0;
                }
                px_root_pop();   // M92-S2c precise：r==NULL 分支（作用域结束）
            }
        }
        // 每请求重置 docroot（命中 vhost → vhost root；未命中 → 默认）
        px_vhost_docroot_store(vroot);
    }
    }  // M98-S2b：end if(!skip_pre) —— CORS/限流/vhost 段1（vhost null 回退续管道
       //   skip_pre=1 整段跳过，由段2 先 docroot store 再续 route+静态）

// M85-S1：--no-route 裁剪（去 runtime_route.o；route/middleware native 缺 → R1001）
#ifndef PX_NO_ROUTE
    // M28：路由表非空 → 优先匹配路由（method+path 模式 + :id 参数 + 中间件链）
    if (px_route_has()) {
        char extra[256];
        snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
        int rret = px_route_try_dispatch(pout, req, method, strcmp(method, "HEAD") == 0,
                                          client_keep_alive, req_id, async_ok);
        if (rret == 1) return 0;   // 已处理（响应已同步发送）
        if (rret == 2) return 1;   // M98-S2a：已拆段（route VM handler 协程运行中，调用方释放 worker）
    }
#endif // PX_NO_ROUTE

    // 6. 路径映射 + 目录隔离（穿越防护：拒绝 ".." 路径段）
    // M31.2：docroot 已由 vhost 解析覆盖（__thread 存储，见 px_vhost_docroot_store）
    const char* docroot = px_vhost_docroot();
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
        pout->respond(pout, 403, "text/plain; charset=utf-8", "403 Forbidden: 路径穿越被拒绝", 30, head_only, client_keep_alive, extra);
        px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                (long long)time(NULL), log_remote, method, path, 403, 30, req_id);
        return 0;
    }
    char full[4096];
    snprintf(full, sizeof(full), "%s%s", docroot, path);

    struct stat st;
    if (stat(full, &st) != 0) {
        char extra[256];
        snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
        pout->respond(pout, 404, "text/plain; charset=utf-8", "404 Not Found", 13, head_only, client_keep_alive, extra);
        px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                (long long)time(NULL), log_remote, method, path, 404, 13, req_id);
        return 0;
    }
    char fpath[4096];
    if (S_ISDIR(st.st_mode)) {
        snprintf(fpath, sizeof(fpath), "%s/index.px", full);
        if (stat(fpath, &st) != 0) {
            snprintf(fpath, sizeof(fpath), "%s/index.html", full);
            if (stat(fpath, &st) != 0) {
                char extra[256];
                snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                pout->respond(pout, 404, "text/plain; charset=utf-8", "404 Not Found", 13, head_only, client_keep_alive, extra);
                px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                        (long long)time(NULL), log_remote, method, path, 404, 13, req_id);
                return 0;
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
        px_dict_set(server, "px", px_str("0.2.0"));
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
            pout->respond(pout, 504, "text/plain; charset=utf-8", msg, ml, head_only, client_keep_alive, extra);
        } else if (exit_code != 0) {
            char msg[70000];
            int ml = snprintf(msg, sizeof(msg), "500 Internal Server Error\n\n%.60000s", out ? out : "");
            pout->respond(pout, 500, "text/plain; charset=utf-8", msg, ml, head_only, client_keep_alive, extra);
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
                    pout->respond(pout, status, ct2, gz, gzlen, head_only, client_keep_alive, gz_extra);
                    xfree(gz);
                    goto script_done;
                }
            }
            pout->respond(pout, status, ct2, body ? body : "", blen, head_only, client_keep_alive, gz_extra);
        script_done:
            if (out) xfree(out);
        }
        // M29c：结构化访问日志（M33：落盘 + 轮转；格式同解释器：时间 remote method path status bytes ms req=id）
        px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                (long long)time(NULL), log_remote, method, path, 200, 0, req_id);
    } else {
        // ---- 静态文件：ETag / Last-Modified / 304 / Range + 流式（M29b） ----
        struct stat fst;
        if (stat(fpath, &fst) != 0) {
            char extra[256];
            snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
            pout->respond(pout, 404, "text/plain; charset=utf-8", "404 Not Found", 13, head_only, client_keep_alive, extra);
            px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                    (long long)time(NULL), log_remote, method, path, 404, 13, req_id);
            return 0;
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
            pout->respond(pout, 304, NULL, "", 0, head_only, client_keep_alive, extra);
            return 0;
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
                    pout->respond(pout, 304, NULL, "", 0, head_only, client_keep_alive, extra);
                    return 0;
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
        // M33.4：Alt-Svc 通告（HTTP/3 协商；px_serve opts{alt_svc}）
        if (g_px_alt_svc[0]) {
            eo += snprintf(extra + eo, sizeof(extra) - (size_t)eo, "Alt-Svc: %s\r\n", g_px_alt_svc);
        }
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
                        char hd_extra[1536];
                        int heo = snprintf(hd_extra, sizeof(hd_extra),
                                          "%sContent-Encoding: gzip\r\nVary: Accept-Encoding\r\n", extra);
                        (void)heo;
                        pout->begin(pout, 200, ct2, (long long)gzlen, head_only, client_keep_alive, hd_extra);
                        if (!head_only) pout->write(pout, gz, (size_t)gzlen);
                        pout->end(pout);
                        xfree(gz);
                        xfree(data);
                        return 0;
                    }
                }
                xfree(data);
            }
        }

        // M53-S2：静态响应经 PxHttpOut（响应头 + 64KB 块流式文件段）
        pout->begin(pout, status, ct2, seg_len, head_only, client_keep_alive, extra);
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
                    pout->write(pout, chunk, got);
                    remain -= (long long)got;
                }
                fclose(f);
            }
        }
        pout->end(pout);
        // M29c：结构化访问日志（M33：落盘 + 轮转）
        // M29c：结构化访问日志（M33：落盘 + 轮转；格式同解释器）
        px_access_log("[px-access] %lld %s %s %s %d %lld 0ms req=%s\n",
                (long long)time(NULL), log_remote, method, path, status, seg_len, req_id);
    }
    return 0;   // M98-S2a：同步完成（函数末尾兜底）
}

// ==================== M53-S3：HTTP/3 请求接入桥 ====================
// runtime_h3.c 托管连接回调把解码后的 H3 请求（method/path/headers/body/remote…）
// 交给本函数：补齐与 HTTP/1.1 等价的 req dict 字段（query 拆分+URL 解码、
// version="HTTP/3"、request_id、cookie、form/multipart 解析、请求体 gzip 解压），
// 随后送入公共管道 px_http_dispatch —— 与 HTTP/1.1 共用 vhost/路由/限流/日志/静态/.px。
// 响应由 pout（H3 PxHttpOut 实现）编码为 HEADERS/DATA 帧，管道不感知传输差异。
void px_http_dispatch_h3(PxHttpOut* pout, LXValue req, int client_keep_alive) {
    LXValue mv = px_dict_get(req, "method");
    LXValue pv = px_dict_get(req, "path");
    const char* method = (mv.type == PX_STR) ? mv.as.obj->as.str.data : "GET";
    const char* target = (pv.type == PX_STR) ? pv.as.obj->as.str.data : "/";
    char tbuf[4096];
    snprintf(tbuf, sizeof(tbuf), "%s", target);
    char path[2048] = {0}, query[2048] = {0};
    char* q = strchr(tbuf, '?');
    if (q) {
        *q = 0;
        char* dec = px_url_decode(tbuf);
        snprintf(path, sizeof(path), "%s", dec ? dec : tbuf);
        if (dec) xfree(dec);
        dec = px_url_decode(q + 1);
        snprintf(query, sizeof(query), "%s", dec ? dec : q + 1);
        if (dec) xfree(dec);
    } else {
        char* dec = px_url_decode(tbuf);
        snprintf(path, sizeof(path), "%s", dec ? dec : tbuf);
        if (dec) xfree(dec);
    }
    char req_id[64];
    px_new_req_id(req_id, sizeof(req_id));
    g_px_ctx_n = 0;                  // M36：每请求清除线程局部上下文（同 HTTP/1.1 worker）
    px_dict_set(req, "path", px_str(path));
    px_dict_set(req, "query", px_str(query));
    px_dict_set(req, "version", px_str("HTTP/3"));
    px_dict_set(req, "request_id", px_str(req_id));
    LXValue headers = px_dict_get(req, "headers");
    if (headers.type != PX_DICT) { headers = px_dict(); px_dict_set(req, "headers", headers); }
    // M53-S4：HTTP/3 请求无 Host 头（RFC 9114 用 :authority）。补齐 headers["Host"] 使
    // vhost/handler（生产应用 site_handler 读 req.headers.Host）与 HTTP/1.1 行为一致。
    {
        LXValue av = px_dict_get(req, "authority");
        if (av.type == PX_STR && px_header_get(&headers, "host").type == PX_NULL) {
            px_dict_set(headers, "Host", av);
        }
    }
    LXValue body_v = px_dict_get(req, "body");
    // cookie（HTTP/1.1 worker 同款解析）
    if (px_dict_get(req, "cookie").type == PX_NULL) {
        LXValue ckv = px_header_get(&headers, "Cookie");
        if (ckv.type == PX_STR) px_dict_set(req, "cookie", px_parse_cookie(ckv.as.obj->as.str.data));
        else px_dict_set(req, "cookie", px_dict());
    }
    // 请求体：gzip 解压 + form/multipart（Content-Type 驱动）
    if (body_v.type == PX_STR) {
        int body_len = body_v.as.obj->as.str.len;
        char* body_buf = xmalloc((size_t)body_len + 1);
        memcpy(body_buf, body_v.as.obj->as.str.data, (size_t)body_len);
        body_buf[body_len] = 0;
        LXValue ce = px_header_get(&headers, "Content-Encoding");
        if (body_len > 0 && ce.type == PX_STR && strcasestr(ce.as.obj->as.str.data, "gzip")) {
            int olen = 0;
            char* dec = px_gzip_decompress(body_buf, body_len, &olen);
            if (dec) { xfree(body_buf); body_buf = dec; body_len = olen; }
        }
        px_dict_set(req, "body", px_str_len(body_buf ? body_buf : "", body_len));
        if (body_len > 0) {
            LXValue ct_v = px_dict_get_ci(headers, "Content-Type");
            const char* ct = (ct_v.type == PX_STR) ? ct_v.as.obj->as.str.data : "";
            if (strstr(ct, "multipart/form-data")) {
                char* boundary = px_mime_boundary(ct);
                if (boundary) { px_parse_multipart(req, body_buf, body_len, boundary); xfree(boundary); }
            } else if (strstr(ct, "application/x-www-form-urlencoded")) {
                LXValue form = px_parse_urlenc(body_buf);
                px_dict_set(req, "form", form);
            }
        }
        xfree(body_buf);
    }
    px_http_dispatch(pout, req, method, path, query, client_keep_alive, req_id, 0, 0); // H3: async_ok=0 skip_pre=0 同步零变化
    px_reset_request_state();
}



// ==================== M98-S2a：px_serve route handler 协程化 —— 连接挂起注册表 ====================
// px_serve（M31.4b g_pool）原模型：每连接一个常驻 g_pool 线程跑 px_conn_worker 阻塞
// keep-alive 到连接关闭（PxConn 为栈对象，线程结束即丢）。M98-S2a 升级：连接对象堆化并
// 登记本表（fd→PxPend*，TLS 会话/读缓冲跨 worker 存活）；route VM handler 命中 → stage=1
//   + req 入 GC 根 + spawn 帧协程（chan/sleep/spawn 让出占协程不占 worker）→ px_conn_worker
//   返回（g_pool 线程释放取下一 job）；协程完成回调 px_serve_route_done（coro worker 线程）
//   → stage=2 + resp 入根 + px_pool_push(fd) 投回；续处理 worker 重入 px_conn_worker →
//   段2（px_route_respond normalize+respond+访问日志）→ keep-alive 下一请求。
// req/resp 为本表 GC 根（px_pxserve_pend_gc_mark 在 gc 标记期补标 —— precise 必须，
//   漏标 = GC 误回收挂起连接请求/响应 → UAF）。
// 锁 = g_pxpend_mu（独立锁，不复用 g_conn_mu —— 同 http_pend fix：GC 标记期遍历本表，
//   而既有 g_conn_mu 临界区（M88-B px_evc_*）不屏蔽 SIG_GC_STOP → 复用会死锁）。临界区
//   统一屏蔽 SIG_GC_STOP（gc_block_stop/gc_unblock_stop）→ 与 GC executor 无持锁竞争。
// 锁序：g_pxpend_mu 不与其他锁嵌套（done 出锁后才 px_pool_push；close 出锁后才
//   px_conn_close + xfree）。fd 复用防串扰：close 置 active=0 → 旧 done/take 不再命中。
typedef struct PxPend {
    int fd;
    int active;        // 连接槽位在用（conn 存活）
    PxConn* conn;      // 堆连接（跨 worker 存活；含 TLS 会话/读缓冲）
    int stage;         // 0 = 无挂起（处理中/空闲循环）；1 = handler/middleware 协程运行中；
                       // 2 = 已完成待续处理（段2 respond）
    int method_head;   // 挂起请求 HEAD
    int client_close;  // 挂起请求 Connection: close（1 = 响应后关连接）
    char req_id[64];   // 挂起请求 X-Request-Id
    char tmp_path[1024]; // 挂起期 body 临时文件（done 回调清理；handler 期需可读）
    int kind;          // M98-S2b：0 = route 拆段（段2 px_route_respond）；1 = vhost 拆段
                       //   （段2：resp 非 null → px_vhost_respond；null → docroot 回退续管道）；
                       //   M100：2 = middleware 链 defer 运行中（链状态机，见 mw_* 字段）；
                       //   3 = middleware 短路完成（段2 px_route_mw_short_respond）
    char vroot[1024];  // M98-S2b：vhost 拆段登记时的解析 docroot（null 回退段2 store 用）
    LXValue req;       // GC 根（stage>=1）
    LXValue resp;      // GC 根（stage==2）
    // M100：middleware 链 defer 字段（kind==2 期间有效；px_pxserve_pend_gc_mark 补标）
    int mw_i;          // 当前 middleware 段索引（运行中段 = mw_chain[mw_i]；==mw_n 表示
                       //   链全 null 已通过 → handler 段运行中）
    int mw_n;          // 链长快照（defer 登记时拷入；0 = 无链 defer）
    LXValue mw_chain[32]; // 链快照（MAX_MIDDLEWARES=32；GC 根——逐段 spawn 函数值跨
                       //   协程保活，不依赖 g_middlewares 运行期一致性）
    LXValue mw_handler;   // 链通过后的 route handler（GC 根；handler 段 spawn 用）
    LXValue mw_params;    // route 匹配 params（GC 根；handler 段 args[1]）
} PxPend;
static PxPend* g_pxpend = NULL;
static int g_pxpend_cap = 0;
static pthread_mutex_t g_pxpend_mu = PTHREAD_MUTEX_INITIALIZER;

static PxPend* px_pxpend_ctx(int fd) {
    if (fd < 0 || fd >= g_pxpend_cap) return NULL;
    return &g_pxpend[fd];
}

// 扩容（持 g_pxpend_mu；上限 g_conn_max = PX_MAX_CONNS env，同 http_pend）。返回 0 可用。
static int px_pxpend_ensure(int fd) {
    if (fd < 0 || fd >= g_conn_max) return -1;
    if (fd < g_pxpend_cap) return 0;
    int ncap = g_pxpend_cap ? g_pxpend_cap : 256;
    while (ncap <= fd && ncap < g_conn_max) ncap *= 2;
    if (ncap > g_conn_max) ncap = g_conn_max;
    if (ncap <= fd) return -1;
    PxPend* np = (PxPend*)xrealloc(g_pxpend, (size_t)ncap * sizeof(PxPend));
    if (!np) return -1;
    for (int i = g_pxpend_cap; i < ncap; i++) {
        np[i].fd = -1; np[i].active = 0; np[i].stage = 0;
        np[i].conn = NULL; np[i].method_head = 0; np[i].client_close = 0;
        np[i].req_id[0] = 0; np[i].tmp_path[0] = 0;
        np[i].kind = 0; np[i].vroot[0] = 0;
        np[i].req.type = PX_NULL; np[i].resp.type = PX_NULL;
        np[i].mw_i = 0; np[i].mw_n = 0;
        np[i].mw_handler.type = PX_NULL; np[i].mw_params.type = PX_NULL;
        for (int j = 0; j < 32; j++) np[i].mw_chain[j].type = PX_NULL;
    }
    g_pxpend = np; g_pxpend_cap = ncap;
    return 0;
}

// px_conn_worker 每 fd job 入口：取/建连接槽。新连接（!active）→ 堆 PxConn + px_conn_init
// （TLS 握手在锁外做，防长握手阻塞全表/GC）+ inflight++。返回槽指针；失败 NULL。
static PxPend* px_pxpend_enter(int fd) {
    PxPend* e = NULL;
    int create = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    if (px_pxpend_ensure(fd) == 0) {
        e = px_pxpend_ctx(fd);
        if (e && !e->active) create = 1;
        else if (e && e->conn) { /* 续处理：连接/会话复用 */ }
        else e = NULL;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    if (!e) return NULL;
    if (!create) return e;
    // 新建：堆化连接 + TLS 握手（锁外）
    PxConn* c = (PxConn*)xmalloc(sizeof(PxConn));
    if (!c) return NULL;
    if (px_conn_init(c, fd) != 0) { xfree(c); return NULL; }
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    if (e->active) {   // 防御：槽被占（理论不达，fd 唯一在途）→ 弃新建
        gc_unblock_stop(&old);
        pthread_mutex_unlock(&g_pxpend_mu);
        px_conn_close(c); xfree(c);
        return NULL;
    }
    e->fd = fd; e->active = 1; e->stage = 0; e->conn = c;
    e->method_head = 0; e->client_close = 0;
    e->req_id[0] = 0; e->tmp_path[0] = 0;
    e->kind = 0; e->vroot[0] = 0;
    e->req.type = PX_NULL; e->resp.type = PX_NULL;
    e->mw_i = 0; e->mw_n = 0;
    e->mw_handler.type = PX_NULL; e->mw_params.type = PX_NULL;
    for (int j = 0; j < 32; j++) e->mw_chain[j].type = PX_NULL;
    __sync_fetch_and_add(&g_px_inflight, 1);
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    return e;
}

// 关闭连接并释放槽（统一 close 路径；幂等）。close(fd)+TLS 释放+free(conn) 在锁外做；
// tmp 兜底清理。fd 复用防串扰：active=0 → 旧 done 回调/take 不再命中。
static void px_pxpend_close(int fd) {
    if (fd < 0) return;
    PxConn* c = NULL;
    char tmp[1024]; tmp[0] = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active) {
        c = e->conn;
        if (e->tmp_path[0]) { snprintf(tmp, sizeof(tmp), "%s", e->tmp_path); e->tmp_path[0] = 0; }
        e->active = 0; e->stage = 0; e->fd = -1; e->conn = NULL;
        e->req.type = PX_NULL; e->resp.type = PX_NULL;
        __sync_fetch_and_sub(&g_px_inflight, 1);
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    // M99：前置摘除事件循环上下文（防事件循环继续照看将关 fd / fd 复用串扰；detach 不 close，
    //   由 px_conn_close 负责 close。worker 处理中 ACTIVE 关闭 / 事件循环 tick/断开分支均走本函数
    //   ——detach 幂等：FREE 时无操作）
    px_evc_detach(fd);
    if (c) { px_conn_close(c); xfree(c); }
    if (tmp[0]) unlink(tmp);
}

// M99：优雅关闭 —— px_serve accept 循环退出 + g_pool worker join 后调用：关闭事件循环照看
// 的全部 px_serve 连接（IDLE 空闲 + 理论残留 ACTIVE；join 后无 worker 使用），逐条
// px_pxpend_close（其内部先 px_evc_detach 摘上下文再 px_conn_close + inflight--），使
// g_px_inflight 归零、优雅关闭干净退出（否则 IDLE 连接等 tick 15s 或卡 5s 等待循环）。
// 锁序：先 g_conn_mu 快照收集 fd（出锁），再逐个 px_pxpend_close（g_pxpend_mu）——两锁不嵌套。
static void px_pxserve_ev_close_all(void) {
    if (g_conn_cap <= 0) return;
    int* fds = (int*)xmalloc((size_t)g_conn_cap * sizeof(int));
    int n = 0;
    pthread_mutex_lock(&g_conn_mu);
    for (int i = 0; i < g_conn_cap; i++) {
        PxConnCtx* c = &g_conns[i];
        if (c->state != PX_CONN_STATE_FREE && c->kind == FSERVE_KIND_PXSERVE) fds[n++] = c->fd;
    }
    pthread_mutex_unlock(&g_conn_mu);
    for (int i = 0; i < n; i++) px_pxpend_close(fds[i]);
    xfree(fds);
}

// 登记挂起请求 body 临时文件（DEFER 返回前调用；done 回调清理）。锁内写，幂等。
static void px_pxpend_set_tmp(int fd, const char* path) {
    if (!path || !path[0]) return;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active && e->stage == 1) {
        snprintf(e->tmp_path, sizeof(e->tmp_path), "%s", path);
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
}

// 取完成项（stage==2 → 置 stage=0 返回 1，req/resp/kind/vroot/标志移交调用方；否则 0）
static int px_pxpend_take(int fd, LXValue* req, LXValue* resp, int* method_head,
                          int* client_close, char* req_id, size_t req_id_sz,
                          int* kind, char* vroot, size_t vroot_sz) {
    int got = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active && e->stage == 2) {
        *req = e->req; *resp = e->resp;
        *method_head = e->method_head; *client_close = e->client_close;
        if (req_id && req_id_sz > 0) snprintf(req_id, req_id_sz, "%s", e->req_id);
        if (kind) *kind = e->kind;
        if (vroot && vroot_sz > 0) snprintf(vroot, vroot_sz, "%s", e->vroot);
        e->stage = 0;
        e->kind = 0; e->vroot[0] = 0;
        e->req.type = PX_NULL; e->resp.type = PX_NULL;
        got = 1;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    return got;
}

// GC 标记期补标挂起项 req/resp（gc executor 单线程标记期调用；持 g_pxpend_mu）
void px_pxserve_pend_gc_mark(void) {
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);   // executor 本已自屏蔽（幂等）；写者临界区同屏蔽 → 无持锁被暂停
    if (g_pxpend) {
        for (int i = 0; i < g_pxpend_cap; i++) {
            PxPend* e = &g_pxpend[i];
            if (e->active && (e->stage == 1 || e->stage == 2)) {
                if (e->req.type != PX_NULL) px_gc_mark_slots(&e->req, 1);
                if (e->resp.type != PX_NULL) px_gc_mark_slots(&e->resp, 1);
                // M100：middleware 链 defer 运行中（kind==2）→ 链快照 + handler/params
                //   补标（逐段 spawn 的函数值跨协程保活，漏标 = GC 误回收 → 悬垂 PxVMFunc）
                if (e->stage == 1 && e->kind == 2) {
                    int n = e->mw_n;
                    if (n > 32) n = 32;
                    if (n > 0) px_gc_mark_slots(e->mw_chain, n);
                    if (e->mw_handler.type != PX_NULL) px_gc_mark_slots(&e->mw_handler, 1);
                    if (e->mw_params.type != PX_NULL) px_gc_mark_slots(&e->mw_params, 1);
                }
            }
        }
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
}

// route handler 帧协程完成回调（coro worker 线程执行；ret = handler 顶层返回值）。
// 顺序：PX_KEEP 保护 ret（precise 窗口）→ g_pxpend_mu 内 stage 1→2 写 resp + tmp 移交
//   → 出锁后清理 tmp + px_pool_push(fd) 投回续处理。连接已关/表项已清（fd 复用）→
//   丢弃 ret。g_px_stop（优雅关闭，池 worker 已退出）→ 直接关连接防 push 无消费者挂死。
static void px_serve_route_done(void* ud, LXValue ret) {
    int fd = (int)(intptr_t)ud;
    if (fd < 0) return;
    px_root_push();
    PX_KEEP(ret);
    int push = 0;
    char tmp[1024]; tmp[0] = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active && e->stage == 1) {
        e->resp = ret;    // GC 根接管（表项 stage2 期间 mark 补标）
        e->stage = 2;
        push = 1;
        if (e->tmp_path[0]) { snprintf(tmp, sizeof(tmp), "%s", e->tmp_path); e->tmp_path[0] = 0; }
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    if (tmp[0]) unlink(tmp);
    if (push) {
        if (g_px_stop) px_pxpend_close(fd);   // 关闭期：池 worker 已退出 → 直接收尾
        else px_pool_push(fd);                // 续处理：段2 respond + keep-alive
    }
    px_root_pop();
}

// 登记 route/vhost VM handler 挂起并 spawn（M98-S2b 泛化：kind=0 route / 1 vhost；
// hargs[0..nargs) 为 handler 参数；vroot 供 vhost null 回退段2 docroot store）。
// 调用点：runtime_route.c（px_route_try_dispatch）与 runtime.c（px_http_dispatch vhost 块）。
// 返回 1 = 已登记+已 spawn（调用方返回 DEFER/1）；0 = 退回原同步直调路径（非 px_serve
// 连接 / 非 VM handler / 槽忙 / 无协程内核）。
int px_pxserve_defer(PxHttpOut* out, LXValue req, LXValue handler, LXValue* hargs, int nargs,
                     int kind, const char* vroot, int head_only, int keep_alive,
                     const char* req_id) {
    PxConn* c = (out && out->impl) ? (PxConn*)out->impl : NULL;
    if (!c) return 0;
    int fd = c->fd;
    if (fd < 0) return 0;
    if (!px_coro_spawn_ex || !px_vm_entry) return 0;          // 无协程内核 → 同步
    if (handler.type != PX_FUNC || !handler.as.obj ||
        handler.as.obj->as.func.fn != px_vm_entry) return 0;  // 仅 VM handler 拆段
    int ok = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active && e->conn == c && e->stage == 0) {
        e->stage = 1;
        e->kind = (kind == 1) ? 1 : 0;
        e->vroot[0] = 0;
        if (e->kind == 1 && vroot) snprintf(e->vroot, sizeof(e->vroot), "%s", vroot);
        e->method_head = head_only ? 1 : 0;
        e->client_close = keep_alive ? 0 : 1;
        e->req_id[0] = 0;
        if (req_id) snprintf(e->req_id, sizeof(e->req_id), "%s", req_id);
        e->req = req;                 // GC 根接管（调用方根随后失效无碍）
        e->resp.type = PX_NULL;
        ok = 1;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    if (!ok) return 0;
    px_coro_spawn_ex(handler.as.obj->as.func.ctx, hargs, nargs,
                     px_serve_route_done, (void*)(intptr_t)fd);
    return 1;
}

// ==================== M100：middleware 链 defer（链状态机） ====================
// middleware 链（runtime_route.c px_route_try_dispatch）全 VM 且 async_ok → 不再在
// g_pool worker 内同步 for 循环逐个 px_call，改登记 PxPend kind=2 链 defer + spawn
// 首段（middleware[0]）。每段完成回调 px_serve_mw_done（coro worker 线程）推进链：
//   · 段是 middleware（mw_i < mw_n）：ret==null → 推进下一段（mw_i++ 后仍 < mw_n →
//     spawn mw_chain[mw_i] 继续链；== mw_n → 链全 null 通过 → spawn handler 段）；
//     ret!=null → 短路：resp=ret + stage=2 + kind=3 → 投回续处理（段2
//     px_route_mw_short_respond，文案 (middleware) 与同步短路一致）。
//   · 段是 handler（mw_i == mw_n，链已全通过）：resp=ret + stage=2 + kind=0 → 投回
//     （段2 px_route_respond = route handler 完成语义，与 M98 route handler defer 一致）。
// req/mw_chain/mw_handler/mw_params 为挂起表 GC 根（px_pxserve_pend_gc_mark 补标）。
// 锁协议沿用 px_serve_route_done：g_pxpend_mu + SIG_GC_STOP 屏蔽；spawn 在出锁后做
// （锁序：g_pxpend_mu 不嵌套其它锁）。连接已关/表项已清（fd 复用）→ 丢弃 ret；
// g_px_stop（优雅关闭，池 worker 已退出）→ 直接收尾防 push 无消费者挂死。
static void px_serve_mw_done(void* ud, LXValue ret) {
    int fd = (int)(intptr_t)ud;
    if (fd < 0) return;
    px_root_push();
    PX_KEEP(ret);
    int push = 0;
    int cont = 0;          // 续段 spawn（链推进 / 进 handler 段）
    int fnargs = 0;
    LXValue fn = px_null();
    LXValue fargs[2];
    fargs[0].type = PX_NULL; fargs[1].type = PX_NULL;
    char tmp[1024]; tmp[0] = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active && e->stage == 1 && e->kind == 2) {
        if (e->mw_i < e->mw_n) {
            // —— middleware 段完成 ——
            if (ret.type != PX_NULL) {
                // 短路：响应段2（kind=3 → px_route_mw_short_respond，(middleware) 文案）
                e->resp = ret;          // GC 根接管
                e->stage = 2;
                e->kind = 3;
                push = 1;
                if (e->tmp_path[0]) { snprintf(tmp, sizeof(tmp), "%s", e->tmp_path); e->tmp_path[0] = 0; }
            } else {
                // null → 推进链
                e->mw_i++;
                if (e->mw_i < e->mw_n) {
                    fn = e->mw_chain[e->mw_i];   // 下一 middleware 段
                    fargs[0] = e->req;
                    fnargs = 1;
                } else {
                    // 链全 null 通过 → handler 段（args = [req, params]，同步路径同参）
                    fn = e->mw_handler;
                    fargs[0] = e->req;
                    fargs[1] = e->mw_params;
                    fnargs = 2;
                }
                cont = 1;
            }
        } else {
            // —— handler 段完成（链已全通过；kind 复位 0 = route handler 段2 语义）——
            e->resp = ret;              // GC 根接管
            e->stage = 2;
            e->kind = 0;
            push = 1;
            if (e->tmp_path[0]) { snprintf(tmp, sizeof(tmp), "%s", e->tmp_path); e->tmp_path[0] = 0; }
        }
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    if (cont) {
        // 续段 spawn（fn/fargs 已拷出锁；PX_KEEP 保活到 spawn memcpy 完成）
        PX_KEEP(fn);
        PX_KEEP(fargs[0]);
        if (fnargs == 2) PX_KEEP(fargs[1]);
        if (fn.type == PX_FUNC && fn.as.obj && fn.as.obj->as.func.fn == px_vm_entry)
            px_coro_spawn_ex(fn.as.obj->as.func.ctx, fargs, fnargs,
                             px_serve_mw_done, (void*)(intptr_t)fd);
        else
            px_pxpend_close(fd);   // 防御（理论不达：登记时已全 VM 校验）：收尾防悬挂
    } else if (push) {
        if (tmp[0]) unlink(tmp);
        if (g_px_stop) px_pxpend_close(fd);   // 关闭期：池 worker 已退出 → 直接收尾
        else px_pool_push(fd);                // 续处理：段2 respond + keep-alive
    }
    px_root_pop();
}

// 登记 middleware 链 defer 并 spawn 首段（kind=2）。调用点 runtime_route.c
// px_route_try_dispatch（middleware 链全 VM + handler VM + async_ok 时）。
// mws[0..mw_count) 链快照拷入 PxPend（GC 根）；handler/params 同入根（handler 段
// spawn 用，跨多段 defer 存活）。返回 1 = 已登记+已 spawn 首段（调用方返回 DEFER/2）；
// 0 = 退回原同步直调路径（非 px_serve 连接 / 槽忙 / 无协程内核 / 非 VM handler）。
int px_pxserve_mw_defer(PxHttpOut* out, LXValue req, LXValue handler, LXValue params,
                        LXValue* mws, int mw_count, int head_only, int keep_alive,
                        const char* req_id) {
    PxConn* c = (out && out->impl) ? (PxConn*)out->impl : NULL;
    if (!c) return 0;
    int fd = c->fd;
    if (fd < 0) return 0;
    if (!px_coro_spawn_ex || !px_vm_entry) return 0;          // 无协程内核 → 同步
    if (mw_count <= 0 || mw_count > 32) return 0;
    if (handler.type != PX_FUNC || !handler.as.obj ||
        handler.as.obj->as.func.fn != px_vm_entry) return 0;   // 仅 VM handler 链 defer
    int ok = 0;
    sigset_t old;
    pthread_mutex_lock(&g_pxpend_mu);
    gc_block_stop(&old);
    PxPend* e = px_pxpend_ctx(fd);
    if (e && e->active && e->conn == c && e->stage == 0) {
        e->stage = 1;
        e->kind = 2;                 // middleware 链 defer 运行中
        e->mw_i = 0;
        e->mw_n = mw_count;
        e->mw_handler = handler;     // GC 根（handler 段 spawn 用）
        e->mw_params = params;       // GC 根（handler 段 args[1]）
        for (int i = 0; i < 32; i++)
            e->mw_chain[i] = (i < mw_count && mws) ? mws[i] : px_null();
        e->method_head = head_only ? 1 : 0;
        e->client_close = keep_alive ? 0 : 1;
        e->req_id[0] = 0;
        if (req_id) snprintf(e->req_id, sizeof(e->req_id), "%s", req_id);
        e->req = req;                // GC 根接管（调用方根随后失效无碍）
        e->resp.type = PX_NULL;
        ok = 1;
    }
    gc_unblock_stop(&old);
    pthread_mutex_unlock(&g_pxpend_mu);
    if (!ok) return 0;
    LXValue fargs[1];
    fargs[0] = req;
    px_coro_spawn_ex(mws[0].as.obj->as.func.ctx, fargs, 1,
                     px_serve_mw_done, (void*)(intptr_t)fd);
    return 1;
}

// 连接处理线程（px_spawn 注册）：args[0] = fd
static LXValue px_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // M98-S2a：连接槽取/建（fd→PxPend*）—— PxConn 堆化登记（跨 g_pool worker 存活，
    // TLS 会话/读缓冲不随 worker 释放而丢）；新建连接在槽内做握手 + inflight++。
    PxPend* pend = px_pxpend_enter(fd);
    if (!pend) return px_null();
    PxConn* conn = pend->conn;
    g_cur_conn = conn;
    // M99：每 job 登记连接上下文为 ACTIVE（FREE→ACTIVE 或幂等复位；事件循环照看/超时收尾用）。
    //   非 Linux（acquire stub 返回 NULL）→ 不登记，交 IDLE 时 idle_put_fd 返回 -1 → 原阻塞续读。
    px_evc_acquire(fd, FSERVE_KIND_PXSERVE);
    PxHttpOut out;
    px_http_out_init_conn(&out, conn);

// M85-S1：--no-h2 裁剪（去 runtime_h2.o；http_serve 不再协商 h2c/ALPN-h2，退化为 HTTP/1.1）
#ifndef PX_NO_H2
    // M37：TLS ALPN 协商 h2 → 直接 HTTP/2（prior knowledge 帧循环，整连接为 h2）。
    //   h2 连接请求循环在 px_h2_handle 内（h2 handler 协程化另立里程碑，保持同步）。
    if (conn->is_tls) {
        const char* alpn = mbedtls_ssl_get_alpn_protocol((mbedtls_ssl_context*)conn->ssl);
        if (alpn && strcmp(alpn, "h2") == 0) {
            px_h2_handle(conn, 0, NULL, 0);
            px_pxpend_close(fd);   // 释放连接槽（px_conn_close 幂等 + free + inflight--）
            g_cur_conn = NULL;
            return px_null();
        }
    }
#endif // PX_NO_H2

    // M98-S2a/S2b：续处理 —— route/vhost handler 帧协程完成投回（stage==2）：
    //   kind=0（route）段2 px_route_respond（归一化 + 响应 + 访问日志）；kind=1（vhost）：
    //   返回非 null → px_vhost_respond（无访问日志 = vhost 历史语义）；返回 null →
    //   docroot 回退续管道（store vroot + 重入 px_http_dispatch skip_pre=1：不重复
    //   CORS/限流/vhost，直接 route+静态/.px 完成）——vhost 回退与同步路径语义一致。
    if (pend->stage == 2) {
        LXValue sreq, sresp;
        int sh = 0, sc = 1;
        char srid[64];
        int skind = 0;
        char svroot[1024];
        srid[0] = 0; svroot[0] = 0;
        px_pxpend_take(fd, &sreq, &sresp, &sh, &sc, srid, sizeof(srid),
                       &skind, svroot, sizeof(svroot));
        const char* sm = "GET";
        const char* spath = "/";
        const char* squery = "";
        {
            LXValue smv = px_dict_get(sreq, "method");
            if (smv.type == PX_STR) sm = smv.as.obj->as.str.data;
            LXValue pv = px_dict_get(sreq, "path");
            if (pv.type == PX_STR) spath = pv.as.obj->as.str.data;
            LXValue qv = px_dict_get(sreq, "query");
            if (qv.type == PX_STR) squery = qv.as.obj->as.str.data;
        }
        px_root_push();   // M92-S2c precise：续处理段2 登记作用域（sreq/sresp 取自挂起表）
        PX_KEEP(sreq);
        PX_KEEP(sresp);
        if (skind == 1) {
            // vhost 段2
            if (sresp.type != PX_NULL) {
                px_vhost_respond(&out, sm, sh, sc ? 0 : 1, srid, sreq, sresp);
            } else {
                // null 回退 docroot：续管道（route + 静态/.px）完成本请求
                px_vhost_docroot_store(svroot);
                int trc = px_http_dispatch(&out, sreq, sm, spath, squery,
                                           sc ? 0 : 1, srid, 1, 1);
                if (trc == 1) {   // 续管道内 route 命中 → 再次拆段（stage 已在 dispatch 内重新登记）
                    px_root_pop();
                    g_cur_conn = NULL;
                    return px_null();
                }
            }
        } else {
#ifdef PX_NO_ROUTE
            // 理论不达（route 表空 → 无 route/middleware 拆段登记）；防御：释放连接收尾
            px_pxpend_close(fd);
            g_cur_conn = NULL;
            return px_null();
#else
            if (skind == 3) {
                // M100：middleware 短路完成段2（归一化 + respond + (middleware) 访问
                //   日志，与同步短路共用 px_route_mw_short_respond，文案逐字一致）
                px_route_mw_short_respond(&out, sreq, sm, sh, sc ? 0 : 1, srid, sresp);
            } else {
                px_route_respond(&out, sreq, sm, sh, sc ? 0 : 1, srid, sresp);
            }
#endif
        }
        px_root_pop();
        if (sc) {   // 本请求 Connection: close → 响应后关闭连接
            px_pxpend_close(fd);
            g_cur_conn = NULL;
            return px_null();
        }
        // M99：段2（route/vhost handler 协程完成）响应已写完 + keep-alive → 空闲交 IDLE。
        //   与 req_done 后同决策——否则段2 完成直接落 for(;;) 顶部阻塞读下一请求占 worker，
        //   handler defer 后连接的空闲仍不释放（M99 目标：所有请求间空闲都不占 worker）。
        if (px_pxserve_idle_after_resp(conn, fd)) {
            g_cur_conn = NULL;
            return px_null();
        }
    }

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
            ssize_t n = px_conn_read(conn, buf + len, (size_t)((int)sizeof(buf) - 1 - len));
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
        px_root_push();   // M92-S2c precise：px_conn_worker 请求迭代登记作用域开始
        LXValue headers = px_dict();
        PX_KEEP(headers);   // M92-S2c precise：headers 裸局部跨 px_dict_set/px_str 分配
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
        // M36：每请求清除线程局部上下文（防跨请求泄漏）
        g_px_ctx_n = 0;

        // 4. 读 body（M27：max_body_size 限制 → 413；>1MB 落盘临时文件防内存溢出）
        int body_len = 0;
        // M38：Transfer-Encoding: chunked 请求体 → 流式解码 + 大小限制（max_body_size）
        LXValue te_v = px_header_get(&headers, "Transfer-Encoding");
        int is_chunked = te_v.type == PX_STR && strcasestr(te_v.as.obj->as.str.data, "chunked");
        if (is_chunked) {
            int body_off = header_end + 4;
            int pend_len = len - body_off;
            char* pend = buf + body_off;
            int pend_pos = 0;
            body_buf = xmalloc((size_t)g_px_max_body + 1);
            int blen = 0;
            int overflow = 0;
            for (;;) {
                // 读 chunk 大小行（hex\r\n）
                char line[128];
                int llen = 0;
                for (;;) {
                    // 找 pend 中换行
                    int found = -1;
                    for (int i = pend_pos; i < pend_len; i++) {
                        if (pend[i] == '\n') { found = i; break; }
                    }
                    if (found >= 0) {
                        int take = found - pend_pos;
                        if (take > 120) take = 120;
                        if (llen + take < 120) { memcpy(line + llen, pend + pend_pos, (size_t)take); llen += take; }
                        line[llen] = 0; // M38：终止字符串（strtoll 需要）
                        // 去尾部 \r
                        if (llen > 0 && line[llen - 1] == '\r') line[llen - 1] = 0;
                        pend_pos = found + 1;
                        break;
                    }
                    // pend 无换行 → 读 conn 补
                    if (pend_pos < pend_len) { pend_len -= pend_pos; memmove(pend, pend + pend_pos, (size_t)pend_len); pend_pos = 0; }
                    char tmpb[512];
                    ssize_t n = px_conn_read(conn, tmpb, sizeof(tmpb));
                    if (n <= 0) break;
                    // 追加到 pend（扩大？用静态缓冲；简化：直接处理）
                    // 简化：把读到的数据追加到 pend 缓冲（buf 后空间足够 64KB）
                    if (pend_len + (int)n < 65536) {
                        memcpy(pend + pend_len, tmpb, (size_t)n);
                        pend_len += (int)n;
                    }
                }
                // 解析大小（十六进制）
                long long csize = 0;
                {
                    char* semi = strchr(line, ';');
                    if (semi) *semi = 0;
                    char* endp = NULL;
                    csize = strtoll(line, &endp, 16);
                }
                if (csize == 0) break;
                if (blen + csize > g_px_max_body) { overflow = 1; break; }
                // 读 csize 字节 + CRLF
                while (pend_len - pend_pos < csize + 2) {
                    char tmpb[8192];
                    ssize_t n = px_conn_read(conn, tmpb, sizeof(tmpb));
                    if (n <= 0) break;
                    if (pend_len + (int)n < 65536) {
                        memcpy(pend + pend_len, tmpb, (size_t)n);
                        pend_len += (int)n;
                    }
                }
                if (pend_len - pend_pos < csize) break;
                memcpy(body_buf + blen, pend + pend_pos, (size_t)csize);
                blen += (int)csize;
                pend_pos += (int)csize + 2; // 数据 + CRLF
            }
            if (overflow) {
                xfree(body_buf);
                body_buf = NULL;
                char extra[256];
                snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                out.respond(&out, 413, "text/plain; charset=utf-8", "413 Payload Too Large", 24, 0, 0, extra);
                goto req_done;
            }
            body_len = blen;
        } else if (content_length > 0) {
            if (content_length > g_px_max_body) {
                char extra[256];
                snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
                out.respond(&out, 413, "text/plain; charset=utf-8", "413 Payload Too Large", 24, 0, 0, extra);
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
                    out.respond(&out, 500, "text/plain; charset=utf-8", "500 创建 body 临时文件失败", 26, 0, 0, NULL);
                    goto req_done;
                }
                if (have > 0) (void)write(body_tmp_file, buf + body_off, (size_t)have);
                int remaining = content_length - have;
                char tmpb[16384];
                while (remaining > 0) {
                    ssize_t n = px_conn_read(conn, tmpb, sizeof(tmpb));
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
                    ssize_t n = px_conn_read(conn, body_buf + got, (size_t)(content_length - got));
                    if (n <= 0) break;
                    got += (int)n;
                }
                body_len = got;
                body_buf[body_len] = 0;
            }
        }

        // 5. 请求 dict + form/files（Content-Type 驱动）
        // M35：请求体 gzip 自动解压（Content-Encoding: gzip → body_buf 解压后供 REQUEST.body/form）
        if (body_len > 0 && body_buf) {
            LXValue ce = px_header_get(&headers, "Content-Encoding");
            if (ce.type == PX_STR && strcasestr(ce.as.obj->as.str.data, "gzip")) {
                int olen = 0;
                char* dec = px_gzip_decompress(body_buf, body_len, &olen);
                if (dec) {
                    xfree(body_buf);
                    body_buf = dec;
                    body_len = olen;
                }
            }
        }
        LXValue req = px_dict();
        PX_KEEP(req);   // M92-S2c precise：req 裸局部跨 px_dict_set/px_str/px_http_dispatch
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
        PX_KEEP(form);   // M92-S2c precise：form 裸局部跨 px_dict_set/px_parse_urlenc 分配
        {
            // M8x：remote 兼容 AF_UNIX（http_serve_unix 连接无 IP）——sockaddr_storage 判族，
            // AF_INET → ip:port（http_serve 原行为）；AF_UNIX → "unix"（客户端 peer 无 sun_path）
            struct sockaddr_storage raddr;
            memset(&raddr, 0, sizeof(raddr));
            socklen_t rl = sizeof(raddr);
            if (getpeername(fd, (struct sockaddr*)&raddr, &rl) != 0) {
                px_dict_set(req, "remote", px_str(""));
            } else if (raddr.ss_family == AF_INET) {
                struct sockaddr_in* rin = (struct sockaddr_in*)&raddr;
                char rbuf[64];
                snprintf(rbuf, sizeof(rbuf), "%s:%d", inet_ntoa(rin->sin_addr), ntohs(rin->sin_port));
                px_dict_set(req, "remote", px_str(rbuf));
            } else if (raddr.ss_family == AF_UNIX) {
                px_dict_set(req, "remote", px_str("unix"));
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

// M85-S1：--no-h2 裁剪（去 runtime_h2.o；http_serve 不再协商 h2c/ALPN-h2，退化为 HTTP/1.1）
#ifndef PX_NO_H2
        // M35：HTTP/2——h2c Upgrade（Upgrade: h2c + HTTP2-Settings）→ 升级为 h2 帧连接；
        // prior knowledge（请求行 "PRI * HTTP/2.0"）→ 直接 h2。进入帧循环后整连接为 h2。
        {
            LXValue upg = px_header_get(&headers, "Upgrade");
            int is_h2c = upg.type == PX_STR && strcasestr(upg.as.obj->as.str.data, "h2c");
            int is_pri = strncmp(target, "PRI * HTTP/2.0", 14) == 0;
            if (is_h2c || is_pri) {
                // 请求头后缓冲残留（body 后到 len 的字节，可能含 client preface 前几字节）
                const unsigned char* residual = (const unsigned char*)buf + (header_end + 4) + content_length;
                int rlen = 0;
                if (len > (header_end + 4) + content_length) rlen = len - ((header_end + 4) + content_length);
                px_h2_handle(conn, is_h2c ? 1 : 0, residual, rlen);
                goto req_done;
            }
        }
#endif // PX_NO_H2
        // M53-S2：req 就绪 → 公共请求管道（CORS/限流/vhost/路由/静态/.px；输出经
        // PxHttpOut）。async_ok=1：route VM handler 命中可拆段帧协程执行（M98-S2a）。
        int dret = px_http_dispatch(&out, req, method, path, query,
                                    client_keep_alive, req_id, 1, 0);
        if (dret == 1) {
            // 已拆段（route VM handler 协程运行中，挂起表 stage=1）：body 临时文件移交
            // done 回调清理（handler 期需可读）；req 已入挂起表 GC 根。释放本 worker
            // （g_pool 线程立即取下一 job）；完成回调 px_pool_push(fd) 投回续处理。
            if (body_tmp_path[0]) px_pxpend_set_tmp(fd, body_tmp_path);
            if (body_tmp_file >= 0) close(body_tmp_file);
            if (body_buf) xfree(body_buf);
            px_reset_request_state();
            px_root_pop();   // M92-S2c precise：px_serve 请求迭代登记作用域结束
            g_cur_conn = NULL;
            return px_null();
        }
    req_done:
        if (body_tmp_path[0]) unlink(body_tmp_path);
        if (body_tmp_file >= 0) close(body_tmp_file);
        if (body_buf) xfree(body_buf);
        px_reset_request_state();
        px_root_pop();   // M92-S2c precise：px_serve 请求迭代登记作用域结束
        if (!client_keep_alive) break;
        // M99：keep-alive 空闲事件化——响应已写完（px_http_dispatch 同步完成 或 段2 续处理完成
        // 均落此）且无下一请求在途 → 连接交 IDLE（事件循环照看），释放本 worker 取下一 job；
        // 事件循环 detect 可读 → px_pool_push 投回续读。有在途（pipelining/TLS 缓冲残留/竞态）
        // 或交 IDLE 失败 → 落 for(;;) 顶部原阻塞续读（功能不降）。fd 保持阻塞（nonblock=0）：
        // 投回时数据在途 → SO_RCVTIMEO recv 立即返回，无 EAGAIN。
        if (px_pxserve_idle_after_resp(conn, fd)) {
            g_cur_conn = NULL;
            return px_null();   // 已交 IDLE：worker 释放（空闲连接 0 占用 g_pool 线程）
        }
    }
    // 连接结束（客户端关闭 / 空闲超时 / 出错）：释放连接槽（px_conn_close + free + inflight--）
    px_pxpend_close(fd);
    g_cur_conn = NULL;
    return px_null();
}

// ==================== M31 沙箱安全 / 虚拟主机 / 限流 ====================
// sandbox_enter(opts{memory_mb, deny, drop_priv}) → bool
//   memory_mb：setrlimit(RLIMIT_AS) = 当前 VSS + memory_mb（新增上限）
//   deny：禁用的内置函数名列表（px_call native 分派检查）
//   drop_priv：root 降权 nobody(65534)；非 root 请求 → false（不生效）
static LXValue bi_sandbox_enter(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sandbox_enter 需要 (opts) 参数");
    if (args[0].type != PX_DICT) px_error("sandbox_enter 的 opts 需要 dict");
    long long memory_mb = 0;
    int drop_priv = 0;
    // deny 列表先收集（临时数组，最多 64 项）
    char deny_tmp[PX_SANDBOX_DENY_MAX][64];
    int deny_n = 0;
    LXValue mb = px_dict_get(args[0], "memory_mb");
    if (mb.type == PX_INT && mb.as.i > 0) memory_mb = mb.as.i;
    else if (mb.type != PX_NULL) px_error("sandbox_enter: memory_mb 需要正整数");
    LXValue dp = px_dict_get(args[0], "drop_priv");
    if (dp.type == PX_BOOL) drop_priv = dp.as.b ? 1 : 0;
    else if (dp.type != PX_NULL) px_error("sandbox_enter: drop_priv 需要 bool");
    LXValue dn = px_dict_get(args[0], "deny");
    if (dn.type == PX_LIST) {
        LXObject* o = dn.as.obj;
        for (int i = 0; i < o->as.list.len; i++) {
            LXValue it = o->as.list.items[i];
            if (it.type != PX_STR) px_error("sandbox_enter: deny 需要 list[str]");
            if (deny_n >= PX_SANDBOX_DENY_MAX) break;
            snprintf(deny_tmp[deny_n], 64, "%s", it.as.obj->as.str.data);
            deny_n++;
        }
    } else if (dn.type != PX_NULL) px_error("sandbox_enter: deny 需要 list[str]");
    // drop_priv 前置检查：非 root 且请求降权 → false（不生效）
    if (drop_priv && geteuid() != 0) return px_bool(false);
    // memory：RLIMIT_AS = 当前 VSS + memory_mb
    if (memory_mb > 0) {
        long long vss = 0;
        FILE* f = fopen("/proc/self/statm", "r");
        if (f) {
            long long pages = 0;
            if (fscanf(f, "%lld", &pages) == 1) vss = pages * 4096LL;
            fclose(f);
        }
        struct rlimit rl;
        rl.rlim_cur = rl.rlim_max = (rlim_t)(vss + memory_mb * 1024LL * 1024LL);
        if (setrlimit(RLIMIT_AS, &rl) != 0) {
            px_error("沙箱：设置内存上限失败（memory_mb=%lld）", memory_mb);
        }
    }
    // deny：填入全局表并激活
    if (deny_n > 0) {
        for (int i = 0; i < deny_n; i++) {
            if (g_sandbox_deny_count < PX_SANDBOX_DENY_MAX) {
                snprintf(g_sandbox_deny[g_sandbox_deny_count++], 64, "%s", deny_tmp[i]);
            }
        }
    }
    g_sandbox_active = 1;
    // drop_priv：root → nobody（setgid 先于 setuid）
    if (drop_priv) {
        setgid(65534);
        setuid(65534);
    }
    return px_bool(true);
}

// vhost(host, docroot|handler) → bool：多域名共服（Host 头路由）
static LXValue bi_vhost(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("vhost 需要 (host, docroot|handler) 参数");
    if (args[0].type != PX_STR) px_error("vhost 的 host 需要字符串");
    const char* host = args[0].as.obj->as.str.data;
    if (!*host) px_error("vhost: host 不能为空");
    // 小写 host
    char hbuf[128];
    snprintf(hbuf, sizeof(hbuf), "%s", host);
    for (int i = 0; hbuf[i]; i++) {
        if (hbuf[i] >= 'A' && hbuf[i] <= 'Z') hbuf[i] = (char)(hbuf[i] - 'A' + 'a');
    }
    int has_root = 0, has_handler = 0;
    char root[1024] = {0};
    LXValue handler = px_null();
    if (args[1].type == PX_STR) {
        snprintf(root, sizeof(root), "%s", args[1].as.obj->as.str.data);
        struct stat st;
        if (stat(root, &st) != 0 || !S_ISDIR(st.st_mode)) px_error("vhost: docroot 不是有效目录: %s", root);
        has_root = 1;
    } else if (args[1].type == PX_FUNC || args[1].type == PX_NATIVE) {
        handler = args[1];
        has_handler = 1;
    } else {
        px_error("vhost 的第二参数需要 docroot 字符串或 handler 函数");
    }
    pthread_mutex_lock(&g_vhost_mu);
    int slot = -1;
    for (int i = 0; i < MAX_VHOSTS; i++) {
        if (g_vhosts[i].active && strcmp(g_vhosts[i].host, hbuf) == 0) { slot = i; break; }
    }
    if (slot < 0) {
        for (int i = 0; i < MAX_VHOSTS; i++) if (!g_vhosts[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        pthread_mutex_unlock(&g_vhost_mu);
        px_error("虚拟主机数量超出上限 %d", MAX_VHOSTS);
    }
    // 清旧（handler 全局保护名释放）
    if (g_vhosts[slot].active && g_vhosts[slot].has_handler) {
        char gname[48];
        snprintf(gname, sizeof(gname), "__px_vhost_handler_%d", slot);
        px_set_global(gname, px_null());
    }
    memset(&g_vhosts[slot], 0, sizeof(PxVhost));
    snprintf(g_vhosts[slot].host, sizeof(g_vhosts[slot].host), "%s", hbuf);
    g_vhosts[slot].has_root = has_root;
    g_vhosts[slot].has_handler = has_handler;
    if (has_root) snprintf(g_vhosts[slot].root, sizeof(g_vhosts[slot].root), "%s", root);
    if (has_handler) {
        g_vhosts[slot].handler = handler;
        // 注册到全局表保护（保守 GC 只扫描全局表 + 线程栈）
        char gname[48];
        snprintf(gname, sizeof(gname), "__px_vhost_handler_%d", slot);
        px_set_global(gname, handler);
    }
    g_vhosts[slot].active = 1;
    pthread_mutex_unlock(&g_vhost_mu);
    return px_bool(true);
}

// 按 Host 头解析 vhost → 输出 docroot（out_root，未命中填 default_root）+ handler
// 返回 1 = 命中 vhost（含 "*" 兜底）；0 = 未命中（用默认 docroot）
static int px_vhost_resolve(const char* host_hdr, const char* default_root,
                            char* out_root, int out_root_sz, LXValue* out_handler, int* has_handler) {
    snprintf(out_root, (size_t)out_root_sz, "%s", default_root);
    *has_handler = 0;
    if (out_handler) *out_handler = px_null();
    pthread_mutex_lock(&g_vhost_mu);
    int has_any = 0;
    for (int i = 0; i < MAX_VHOSTS; i++) if (g_vhosts[i].active) { has_any = 1; break; }
    if (!has_any) {
        pthread_mutex_unlock(&g_vhost_mu);
        return 0;
    }
    // host 头（小写 + 去端口）
    char hbuf[256] = {0};
    if (host_hdr) {
        snprintf(hbuf, sizeof(hbuf), "%s", host_hdr);
        for (int i = 0; hbuf[i]; i++) {
            if (hbuf[i] >= 'A' && hbuf[i] <= 'Z') hbuf[i] = (char)(hbuf[i] - 'A' + 'a');
        }
    }
    char host_noport[256] = {0};
    {
        const char* colon = strchr(hbuf, ':');
        if (colon) {
            int l = (int)(colon - hbuf);
            if (l > 255) l = 255;
            memcpy(host_noport, hbuf, (size_t)l);
            host_noport[l] = 0;
        } else {
            snprintf(host_noport, sizeof(host_noport), "%s", hbuf);
        }
    }
    int fallback_slot = -1;
    int hit = 0;
    // 从后往前（后注册覆盖）
    for (int i = MAX_VHOSTS - 1; i >= 0; i--) {
        if (!g_vhosts[i].active) continue;
        const char* rh = g_vhosts[i].host;
        if (strcmp(rh, hbuf) == 0) { hit = 1; }
        else if (!strchr(rh, ':') && strcmp(rh, host_noport) == 0 && *host_noport) { hit = 1; }
        else if (strcmp(rh, "*") == 0 && fallback_slot < 0) { fallback_slot = i; continue; }
        else continue;
        if (g_vhosts[i].has_root) snprintf(out_root, (size_t)out_root_sz, "%s", g_vhosts[i].root);
        if (g_vhosts[i].has_handler) {
            *has_handler = 1;
            if (out_handler) *out_handler = g_vhosts[i].handler;
        }
        pthread_mutex_unlock(&g_vhost_mu);
        return 1;
    }
    if (fallback_slot >= 0) {
        if (g_vhosts[fallback_slot].has_root)
            snprintf(out_root, (size_t)out_root_sz, "%s", g_vhosts[fallback_slot].root);
        if (g_vhosts[fallback_slot].has_handler) {
            *has_handler = 1;
            if (out_handler) *out_handler = g_vhosts[fallback_slot].handler;
        }
        pthread_mutex_unlock(&g_vhost_mu);
        return 1;
    }
    pthread_mutex_unlock(&g_vhost_mu);
    return 0;
}

// ==================== M31 限流 / 防爆破（滑动窗口） ====================
// rate_limit(key, max, window_sec) → bool：true 放行 / false 超限

// __thread 当前请求 docroot（vhost 解析覆盖；每请求重置）
static __thread char g_vroot_tls[1024] = {0};
static __thread int g_vroot_tls_set = 0;

static void px_vhost_docroot_store(const char* root) {
    snprintf(g_vroot_tls, sizeof(g_vroot_tls), "%s", (root && *root) ? root : ".");
    g_vroot_tls_set = 1;
}

static const char* px_vhost_docroot(void) {
    return g_vroot_tls_set ? g_vroot_tls : ".";
}

// vhost handler 响应归一化（同解释器 normalize_route_resp）：
// int → 状态码；str → 200 text/plain；dict{status,headers,body} → 完整控制；其他 → px_to_string
// vhost handler 自定义响应头白名单（M57-S7）：键/值均须无 CRLF（防注入），
// 值含 \0 时按 str.len 检测仍拦 \r\n，写出经 %s 以 \0 截断（头值本应为文本）
static int px_vhost_header_allowed(const char* k) {
    static const char* allow[] = {
        "Location", "Cache-Control", "Content-Disposition", "Content-Language",
        "Set-Cookie", "X-Robots-Tag",
        "Access-Control-Allow-Origin", "Access-Control-Allow-Methods",
        "Access-Control-Allow-Headers", "Access-Control-Allow-Credentials",
        "Access-Control-Expose-Headers", "Access-Control-Max-Age",
        NULL
    };
    for (int i = 0; allow[i]; i++)
        if (strcasecmp(k, allow[i]) == 0) return 1;
    return 0;
}

static void px_vhost_normalize(LXValue v, int* status, const char** ct, const char** body, int* body_len,
                               char* extra, int extra_sz) {
    *status = 200;
    *ct = "text/plain; charset=utf-8";
    *body = "";
    *body_len = 0;
    if (v.type == PX_INT) {
        *status = (int)v.as.i;
    } else if (v.type == PX_STR) {
        *body = v.as.obj->as.str.data;
        *body_len = v.as.obj->as.str.len;
    } else if (v.type == PX_DICT) {
        LXValue st = px_dict_get(v, "status");
        if (st.type == PX_INT) *status = (int)st.as.i;
        LXValue b = px_dict_get(v, "body");
        if (b.type == PX_STR) {
            *body = b.as.obj->as.str.data;
            *body_len = b.as.obj->as.str.len;
        } else if (b.type == PX_BYTES) {
            *body = (const char*)b.as.obj->as.str.data;
            *body_len = b.as.obj->as.str.len;
            *ct = "application/octet-stream";
        } else if (b.type == PX_NULL) {
            *body = "";
            *body_len = 0;
        }
        LXValue h = px_dict_get(v, "headers");
        if (h.type == PX_DICT) {
            LXValue ctv = px_dict_get_ci(h, "Content-Type");
            if (ctv.type == PX_STR) *ct = ctv.as.obj->as.str.data;
            // M57-S7：vhost handler 自定义响应头白名单透传（防 CRLF 注入）——
            // 键/值任一含 \r\n 即丢弃；extra 写满安全截断（px_out11_begin 头缓冲 2048 兜底）
            if (extra && extra_sz > 0) {
                LXObject* ho = h.as.obj;
                for (int i = 0; i < ho->as.dict.len; i++) {
                    LXValue hv = ho->as.dict.vals[i];
                    if (hv.type != PX_STR) continue;
                    const char* hk = ho->as.dict.keys[i];
                    if (!hk || strcasecmp(hk, "Content-Type") == 0) continue;
                    if (!px_vhost_header_allowed(hk)) continue;
                    const char* hvv = hv.as.obj->as.str.data;
                    size_t klen = strlen(hk), vlen = hv.as.obj->as.str.len;
                    if (memchr(hk, '\r', klen) || memchr(hk, '\n', klen)) continue;
                    if (memchr(hvv, '\r', vlen) || memchr(hvv, '\n', vlen)) continue;
                    int cur = (int)strlen(extra);
                    long long need = (long long)klen + 2 + (long long)vlen + 2;
                    if ((long long)cur + need < (long long)extra_sz)
                        snprintf(extra + cur, (size_t)(extra_sz - cur), "%s: %s\r\n", hk, hvv);
                }
            }
        }
    } else {
        char* s = px_to_string(v);
        static __thread char vh_buf[4096];
        snprintf(vh_buf, sizeof(vh_buf), "%s", s ? s : "");
        if (s) xfree(s);
        *body = vh_buf;
        *body_len = (int)strlen(vh_buf);
    }
}

static int rate_bucket_find(const char* key, RateBucket** out) {
    for (RateBucket* b = g_rate_head; b; b = b->next) {
        if (strcmp(b->key, key) == 0) { *out = b; return 1; }
    }
    return 0;
}

int px_rate_limit_try(const char* key, long long max, long long window_sec) {
    pthread_mutex_lock(&g_rate_mu);
    RateBucket* b = NULL;
    if (!rate_bucket_find(key, &b)) {
        // 新建桶（防膨胀：超上限清空所有空桶）
        if (g_rate_buckets >= PX_RATE_MAX_BUCKETS) {
            RateBucket** pp = &g_rate_head;
            while (*pp) {
                RateBucket* cur = *pp;
                if (cur->count == 0) {
                    *pp = cur->next;
                    xfree(cur->times);
                    xfree(cur);
                    g_rate_buckets--;
                } else {
                    pp = &cur->next;
                }
            }
        }
        b = xmalloc(sizeof(RateBucket));
        memset(b, 0, sizeof(*b));
        int cap = (int)max < 64 ? 64 : (int)(max + 16);
        if (cap > 100000) cap = 100000;
        b->times = xmalloc(sizeof(long long) * (size_t)cap);
        b->cap = cap;
        snprintf(b->key, sizeof(b->key), "%s", key);
        b->next = g_rate_head;
        g_rate_head = b;
        g_rate_buckets++;
    }
    long long now = (long long)time(NULL);
    long long win_start = now - window_sec;
    // 滑出过期
    while (b->count > 0 && b->times[b->head] < win_start) {
        b->head = (b->head + 1) % b->cap;
        b->count--;
    }
    if (b->count >= (int)max) {
        pthread_mutex_unlock(&g_rate_mu);
        return 0;
    }
    int idx = (b->head + b->count) % b->cap;
    b->times[idx] = now;
    b->count++;
    pthread_mutex_unlock(&g_rate_mu);
    return 1;
}

static LXValue bi_rate_limit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("rate_limit 需要 (key, max, window_sec) 参数");
    if (args[0].type != PX_STR) px_error("rate_limit 的 key 需要字符串");
    if (args[1].type != PX_INT || args[2].type != PX_INT) px_error("rate_limit 的 max/window_sec 需要整数");
    long long max = args[1].as.i;
    long long win = args[2].as.i;
    if (max < 1 || win < 1) px_error("rate_limit: max 与 window_sec 需要正整数");
    return px_bool(px_rate_limit_try(args[0].as.obj->as.str.data, max, win));
}

// gen_next(g) → Value|null（逐项取值；耗尽后 null）
static LXValue bi_gen_next(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("gen_next 需要 1 个参数");
    if (args[0].type != PX_GEN) px_error("gen_next 需要生成器对象");
    return px_gen_next(args[0]);
}

// list(x) → list（list/range/gen/tuple → list；str → 字符列表；dict → keys）
static LXValue bi_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("list 需要 1 个参数");
    LXValue v = args[0];
    if (v.type == PX_LIST) return v;
    if (v.type == PX_TUPLE) return px_list_n(v.as.obj->as.tuple.items, v.as.obj->as.tuple.len);
    if (v.type == PX_GEN) {
        // M34：惰性生成器先物化剩余（list() 需要全量）
        if (v.as.obj->as.gen.is_lazy) px_gen_materialize(v.as.obj);
        return v.as.obj->as.gen.list;
    }
    if (v.type == PX_STR) {
        // UTF-8 字符列表
        const char* s = v.as.obj->as.str.data;
        int n = v.as.obj->as.str.len;
        LXValue l = px_list(0);
        px_root_push();
        PX_KEEP(l);   // M92 precise：字符 list 跨 px_list_push/px_str_len 分配
        int i = 0;
        while (i < n) {
            int cl = 1;
            unsigned char c = (unsigned char)s[i];
            if (c >= 0xF0) cl = 4; else if (c >= 0xE0) cl = 3; else if (c >= 0xC0) cl = 2;
            if (i + cl > n) cl = n - i;
            char buf[8];
            memcpy(buf, s + i, (size_t)cl);
            buf[cl] = 0;
            px_list_push(l, px_str_len(buf, cl));
            i += cl;
        }
        px_root_pop();
        return l;
    }
    if (v.type == PX_DICT) {
        LXValue l = px_list(0);
        px_root_push();
        PX_KEEP(l);   // M92 precise：keys list 跨 px_list_push/px_str 分配
        LXObject* o = v.as.obj;
        for (int i = 0; i < o->as.dict.len; i++) px_list_push(l, px_str(o->as.dict.keys[i]));
        px_root_pop();
        return l;
    }
    // range：C 端 range() 已物化为 list（bi_range 直接返回 list）
    px_error("list 不支持类型 %s", px_type_name(v));
    return px_null();
}

// M-B7：tuple(list/tuple) → tuple（自举 interp 动态构造 tuple 值）
static LXValue bi_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("tuple 需要 1 个参数");
    LXValue v = args[0];
    if (v.type == PX_LIST) return px_tuple(v.as.obj->as.list.items, v.as.obj->as.list.len);
    if (v.type == PX_TUPLE) return v;
    px_error("tuple 参数需要 list/tuple");
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
#ifndef PX_NO_QUIC
    // M53-S4：HTTP/3（QUIC/UDP）—— opts.http3 = true（自签）| {port?, cert?, key?}；port 默认=TCP port
    int h3_enable = 0;
    int h3_port = port;
    char h3_cert[1024] = {0};
    char h3_key[1024] = {0};
#endif
    // M27/M31/M33：opts = {max_body_size, body_tmp_dir, max_conn, rate_limit:{max,window_sec}, access_log, alt_svc}
    g_px_max_body = 10 * 1024 * 1024;
    int max_conn = 32;
    g_px_rate_max = 0;
    g_px_rate_window = 0;
    snprintf(g_px_rate_key_mode, sizeof(g_px_rate_key_mode), "ip");
    g_px_rate_whitelist_n = 0;
    g_px_access_log[0] = 0;
    g_px_alt_svc[0] = 0;
    g_px_log_json = 0;
    g_px_log_daily = 0;
    g_px_gzip_level = 6;
    g_px_gzip_min = 1024;
#ifndef PX_NO_QUIC
    g_px_h3_listener = 0;
#endif
    if (nargs >= 4 && args[3].type == PX_DICT) {
        LXValue mb = px_dict_get(args[3], "max_body_size");
        if (mb.type == PX_INT) g_px_max_body = (int)(mb.as.i >= 1024 ? mb.as.i : 1024);
        LXValue mc = px_dict_get(args[3], "max_conn");
        if (mc.type == PX_INT && mc.as.i >= 1) max_conn = (int)(mc.as.i > PX_POOL_MAX ? PX_POOL_MAX : mc.as.i);
        LXValue rl = px_dict_get(args[3], "rate_limit");
        if (rl.type == PX_DICT) {
            LXValue rm = px_dict_get(rl, "max");
            LXValue rw = px_dict_get(rl, "window_sec");
            if (rm.type == PX_INT && rw.type == PX_INT && rm.as.i >= 1 && rw.as.i >= 1) {
                g_px_rate_max = rm.as.i;
                g_px_rate_window = rw.as.i;
            }
            // M35：key 维度 + 白名单 IP
            LXValue rk = px_dict_get(rl, "key");
            if (rk.type == PX_STR) snprintf(g_px_rate_key_mode, sizeof(g_px_rate_key_mode), "%s",
                                            rk.as.obj->as.str.data);
            LXValue wl = px_dict_get(rl, "whitelist");
            if (wl.type == PX_LIST) {
                g_px_rate_whitelist_n = 0;
                int wn = wl.as.obj->as.list.len;
                for (int i = 0; i < wn && g_px_rate_whitelist_n < PX_RATE_WL_MAX; i++) {
                    LXValue item = wl.as.obj->as.list.items[i];
                    if (item.type == PX_STR) {
                        snprintf(g_px_rate_whitelist[g_px_rate_whitelist_n++],
                                 sizeof(g_px_rate_whitelist[0]), "%s", item.as.obj->as.str.data);
                    }
                }
            }
        }
        LXValue al = px_dict_get(args[3], "access_log");
        if (al.type == PX_STR) snprintf(g_px_access_log, sizeof(g_px_access_log), "%s",
                                        al.as.obj->as.str.data);
        LXValue asvc = px_dict_get(args[3], "alt_svc");
        if (asvc.type == PX_STR) snprintf(g_px_alt_svc, sizeof(g_px_alt_svc), "%s",
                                          asvc.as.obj->as.str.data);
        LXValue lj = px_dict_get(args[3], "log_json");
        if (lj.type == PX_BOOL) g_px_log_json = lj.as.b ? 1 : 0;
        LXValue ld = px_dict_get(args[3], "log_daily");
        if (ld.type == PX_BOOL) g_px_log_daily = ld.as.b ? 1 : 0;
        LXValue gl = px_dict_get(args[3], "gzip_level");
        if (gl.type == PX_INT && gl.as.i >= 1 && gl.as.i <= 9) g_px_gzip_level = (int)gl.as.i;
        LXValue gm = px_dict_get(args[3], "gzip_min_bytes");
        if (gm.type == PX_INT && gm.as.i >= 1) g_px_gzip_min = (int)gm.as.i;
#ifndef PX_NO_QUIC
        // M53-S4：opts.http3 —— bool true（自签证书）或 {port?, cert?, key?}
        LXValue h3o = px_dict_get(args[3], "http3");
        if (h3o.type == PX_BOOL && h3o.as.b) {
            h3_enable = 1;
        } else if (h3o.type == PX_DICT) {
            h3_enable = 1;
            LXValue hp = px_dict_get(h3o, "port");
            if (hp.type == PX_INT && hp.as.i >= 1 && hp.as.i <= 65535) h3_port = (int)hp.as.i;
            LXValue hc = px_dict_get(h3o, "cert");
            if (hc.type == PX_STR) snprintf(h3_cert, sizeof(h3_cert), "%s", hc.as.obj->as.str.data);
            LXValue hk = px_dict_get(h3o, "key");
            if (hk.type == PX_STR) snprintf(h3_key, sizeof(h3_key), "%s", hk.as.obj->as.str.data);
        }
#endif
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
    fprintf(stderr, "[px-serve] 普贤应用服务器 docroot=%s 端口=%d 超时=%dms tls=%d max_body=%d max_conn=%d\n",
            docroot, port, timeout_ms, g_srv_tls_ready, g_px_max_body, max_conn);
#ifndef PX_NO_QUIC
    // M53-S4：HTTP/3（QUIC/UDP）—— 与 HTTP/1.1 同端口（UDP/TCP 不冲突，标准 443+443）或 http3.port；
    // 请求经 runtime_h3.c 管道托管（h3_srv_pipe_cb）走与 HTTP/1.1 同一 vhost/路由/限流/静态/.px 管道。
    if (h3_enable) {
        int64_t lid = px_h3_server_listen_pipe(h3_port, h3_cert[0] ? h3_cert : "",
                                               h3_key[0] ? h3_key : "");
        if (lid > 0) {
            g_px_h3_listener = lid;
            // Alt-Svc 自动通告：http3 开启且未显式配置 alt_svc 时，默认 h3=":port"（客户端据此升级）
            if (!g_px_alt_svc[0]) snprintf(g_px_alt_svc, sizeof(g_px_alt_svc), "h3=\":%d\"", h3_port);
            fprintf(stderr, "[px-serve] HTTP/3 listening udp/%d listener=%lld alt_svc=%s\n",
                    h3_port, (long long)lid, g_px_alt_svc);
        } else {
            fprintf(stderr, "[px-serve] 警告：HTTP/3 listener 启动失败（udp/%d 被占或证书不可用），仅提供 HTTP/1.1\n",
                    h3_port);
        }
    }
#endif
    // M31.4b：并发模型升级——连接线程池（不占 spawn 槽位，突破 64 上限）
    // 预派生 max_conn 个常驻 worker：accept 只把 cfd 放队列，worker 取队列处理。
    g_pool_size = max_conn;
    for (int i = 0; i < g_pool_size; i++) {
        if (pthread_create(&g_pool_threads[i], NULL, px_pool_worker, NULL) != 0) {
            g_pool_size = i;
            px_error("px_serve: 创建连接线程池失败");
        }
    }
    for (;;) {
        if (g_px_stop) break;
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            if (g_px_stop) break;
            continue;
        }
        px_pool_push(cfd);
    }
    g_px_listen_fd = -1;
    close(sfd);
#ifndef PX_NO_QUIC
    // M53-S4：优雅关闭 H3 listener（停收包路由线程 → 关托管连接 → 释放 UDP fd）
    if (g_px_h3_listener > 0) {
        px_quic_raw_close_listener(g_px_h3_listener);
        fprintf(stderr, "[px-serve] HTTP/3 listener 已关闭\n");
        g_px_h3_listener = 0;
    }
#endif
    // 停止 worker：广播唤醒 → worker 处理完当前连接/清空队列后退出 → join
    pthread_mutex_lock(&g_pool_mu);
    pthread_cond_broadcast(&g_pool_cond);
    pthread_mutex_unlock(&g_pool_mu);
    for (int i = 0; i < g_pool_size; i++) pthread_join(g_pool_threads[i], NULL);
    g_pool_size = 0;
    // M99：关闭事件循环照看的 px_serve 连接（keep-alive IDLE 空闲不占 worker → join 已快；
    //   此处清 IDLE 连接使 inflight 归零，优雅关闭干净退出，不再等 15s tick / 5s 等待兜底）
    px_pxserve_ev_close_all();
    // 等待在途请求（最多 5s；连接线程池已 join + IDLE 已清，正常已归零）
    for (int i = 0; i < 100 && g_px_inflight > 0; i++) {
        struct timespec ts = {0, 50 * 1000 * 1000};
        nanosleep(&ts, NULL);
    }
    fprintf(stderr, "[px-serve] 优雅关闭完成（在途 %d）\n", g_px_inflight);
    // M36：进程池优雅关闭——清理 .px 执行进程池 worker（px --worker 子进程）
    // 先 SIGTERM（让 worker 有机会结束当前任务），短暂等待后 SIGKILL 兜底，避免孤儿进程
    if (g_px_pool_ready) {
        pthread_mutex_lock(&g_px_pool_mu);
        for (int i = 0; i < g_px_pool_size; i++) {
            if (g_px_pool[i].alive && g_px_pool[i].pid > 0) kill(g_px_pool[i].pid, SIGTERM);
        }
        pthread_mutex_unlock(&g_px_pool_mu);
        // 等待 worker 退出（最多 3s）
        for (int i = 0; i < 60; i++) {
            int any = 0;
            pthread_mutex_lock(&g_px_pool_mu);
            for (int j = 0; j < g_px_pool_size; j++) {
                if (g_px_pool[j].alive && g_px_pool[j].pid > 0) { any = 1; break; }
            }
            pthread_mutex_unlock(&g_px_pool_mu);
            if (!any) break;
            struct timespec ts = {0, 50 * 1000 * 1000};
            nanosleep(&ts, NULL);
        }
        // 强制清理残留
        pthread_mutex_lock(&g_px_pool_mu);
        for (int i = 0; i < g_px_pool_size; i++) {
            if (g_px_pool[i].alive && g_px_pool[i].pid > 0) {
                kill(g_px_pool[i].pid, SIGKILL);
                if (g_px_pool[i].in_fd > 0) close(g_px_pool[i].in_fd);
                if (g_px_pool[i].out_fd > 0) close(g_px_pool[i].out_fd);
                g_px_pool[i].alive = 0;
            }
        }
        g_px_pool_ready = 0;
        pthread_mutex_unlock(&g_px_pool_mu);
    }
    return px_null();
}

// ==================== M31.4b 连接线程池（常驻 worker，不占 spawn 槽位） ====================
// accept 线程 → 队列 → worker 线程 px_conn_worker。队列满时 accept 阻塞等待（TCP backlog 排队）。

static void px_pool_push(int fd) {
    pthread_mutex_lock(&g_pool_mu);
    while (g_pool_count >= PX_POOL_MAX) {
        pthread_cond_wait(&g_pool_cond, &g_pool_mu);
    }
    g_pool_fds[g_pool_tail] = fd;
    g_pool_tail = (g_pool_tail + 1) % PX_POOL_MAX;
    g_pool_count++;
    pthread_cond_signal(&g_pool_cond);
    pthread_mutex_unlock(&g_pool_mu);
}

static void* px_pool_worker(void* arg) {
    (void)arg;
    // 常驻线程：注册到 GC 槽位（GC 可暂停/扫描本线程栈上对象）
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    g_active_threads++;
    int slot = -1;
    for (int i = 0; i < g_thread_cap; i++) {
        if (!g_threads[i].in_use) { slot = i; break; }
    }
    if (slot >= 0) {
        g_threads[slot].tid = pthread_self();
        g_threads[slot].in_use = 1;
        g_threads[slot].paused = 0;
        g_threads[slot].is_main = 0;
        g_threads[slot].epoch = 0;
        g_threads[slot].tmp_root = NULL;
    }
    pthread_mutex_unlock(&g_gc_mu);
    for (;;) {
        pthread_mutex_lock(&g_pool_mu);
        while (g_pool_count == 0 && !g_px_stop) {
            pthread_cond_wait(&g_pool_cond, &g_pool_mu);
        }
        if (g_pool_count == 0 && g_px_stop) {
            pthread_mutex_unlock(&g_pool_mu);
            break;
        }
        int fd = g_pool_fds[g_pool_head];
        g_pool_head = (g_pool_head + 1) % PX_POOL_MAX;
        g_pool_count--;
        pthread_cond_broadcast(&g_pool_cond);
        pthread_mutex_unlock(&g_pool_mu);
        LXValue arg = px_int(fd);
        px_conn_worker(&arg, 1, NULL);
        px_gc_poll();   // ISSUE28-B1：请求间安全点回收（px_serve 池 worker 同 fserve 语义）
    }
    pthread_mutex_lock(&g_gc_mu);
    g_active_threads--;
    gc_unregister_thread(pthread_self());
    pthread_mutex_unlock(&g_gc_mu);
    return NULL;
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
        px_dict_set(srv, "px", px_str("0.2.0"));
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
    for (int i = 0; i < g_thread_cap; i++) {
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
    for (int i = 0; i < g_thread_cap; i++) if (!g_threads[i].in_use) { gslot = i; break; }
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
        px_error("cron: 并发线程数超出上限 %d", g_thread_max);
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
