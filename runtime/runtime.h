// 普贤 (PuXian) C 运行时库 — runtime.h
// M4 编译模式：动态值系统 + 内置函数
// MVP 策略：值对象不自动释放（进程退出回收），后续 M6 加 GC
#ifndef PX_RUNTIME_H
#define PX_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>   // ssize_t（PxConn 读写返回）

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 值类型 ====================

typedef enum {
    PX_NULL = 0,
    PX_BOOL,
    PX_INT,
    PX_FLOAT,
    PX_STR,
    PX_BYTES,   // M23b：二进制安全字节串（带长度，可含 NUL；union 复用 str 的 data/len）
    PX_LIST,
    PX_DICT,
    PX_FUNC,    // 用户函数（编译为 C 函数）
    PX_NATIVE,  // 内置函数
    PX_STRUCT,  // 结构体实例
    PX_ENUM,    // 枚举值
    PX_TUPLE,   // 元组
    PX_CHAN,    // 通道（并发原语）
    PX_MUTEX,   // 互斥锁（M13：P1 锁原语）
    PX_RWLOCK,  // 读写锁（M13：读多写少）
    PX_GEN,     // 生成器（M32：生成器表达式延迟物化）
    PX_RESULT,  // Result 值（M39：Ok(T) | Err(E)，spec §3.5 错误处理唯一通道）
} LXType;

typedef struct LXValue LXValue;
typedef struct LXObject LXObject;

// 用户函数签名：args 数组 + 参数个数 + 上下文
typedef LXValue (*LXFuncPtr)(LXValue* args, int nargs, void* ctx);

struct LXValue {
    LXType type;
    union {
        bool b;
        int64_t i;
        double f;
        LXObject* obj;
    } as;
};

struct LXObject {
    LXType type;
    unsigned gc_mark : 1;   // M8 GC：标记-清除标记位
    unsigned is_mmap : 1;   // M57-S2：PX_BYTES 的 data 指向 mmap 映射区（GC 回收走 munmap 而非 xfree）
    union {
        struct { char* data; int len; } str;
        struct { LXValue* items; int len, cap; } list;
        struct { char** keys; LXValue* vals; int len, cap; } dict;
        struct { char* name; LXFuncPtr fn; void* ctx; } func;
        struct { char* name; LXFuncPtr fn; } native;
        struct { char* type_name; char** fnames; LXValue* fvals; int nfields; } struct_inst;
        struct { char* type_name; char* variant; } enum_inst;
        struct { LXValue* items; int len; } tuple;
        struct {
            int cap;          // 缓冲容量（0 = 无缓冲 rendezvous）
            int len;          // 当前缓冲元素数
            int head;         // 环形缓冲头
            int closed;       // 是否已关闭
            LXValue* buf;     // 缓冲数组（cap 个）
            pthread_mutex_t mu;
            pthread_cond_t cv_send;  // 发送者等待（缓冲满 / 无缓冲等接收者）
            pthread_cond_t cv_recv;  // 接收者等待（缓冲空）
            int recv_waiting;        // 无缓冲：等待中的接收者数
        } chan;
        struct {
            pthread_mutex_t mu;      // 保护 locked
            pthread_cond_t cv;       // 等待者
            int locked;              // 是否被持有
        } mutex;
        struct {
            pthread_mutex_t mu;      // 保护状态
            pthread_cond_t cv;
            int readers;             // 活跃读者数
            int writer;              // 写者持有
            int writer_waiting;      // 等待中的写者数（写优先）
        } rwlock;
        struct {
            LXValue  list;           // 物化后的列表
            int      cursor;         // gen_next 游标
            // M34 惰性生成器：is_lazy=1 时从 seq 逐项取（不展开），filter 检查 + transform 求值
            int      is_lazy;
            LXValue  seq;            // 迭代源（list / range）
            LXValue  transform;      // 变换闭包 fn(x){expr}
            LXValue  filter;         // 过滤闭包 fn(x){cond}（PX_NULL = 全通过）
        } gen;
        struct {
            int      ok;             // M39：1 = Ok(T)，0 = Err(E)
            LXValue  value;          // 载荷（Ok 的 T 或 Err 的 E）
        } result;
    } as;
};

// ==================== 值构造 ====================

LXValue px_null(void);
LXValue px_bool(bool b);
LXValue px_int(int64_t i);
LXValue px_float(double f);
LXValue px_str(const char* s);
LXValue px_str_len(const char* s, int len);
// M23b：二进制安全字节串构造（复制 len 字节，可含 NUL）
LXValue px_bytes_len(const void* data, int len);
LXValue px_list(int cap);
LXValue px_list_n(LXValue* items, int n);
LXValue px_dict(void);
LXValue px_func(const char* name, LXFuncPtr fn, void* ctx);
LXValue px_native(const char* name, LXFuncPtr fn);

// M42：FFI 注册表（runtime_ffi.c）—— 显式 C 库 import 的 C 桥
void    px_ffi_register(const char* name, LXFuncPtr fn);
bool    px_ffi_has(const char* name);
bool    px_global_native(const char* name, LXValue* out);   // M68：非致命全局 native 查询（ffi_call 双表兜底）
LXValue bi_ffi_call(LXValue* args, int nargs, void* ctx);   // ffi_call(name, args_list)
void    px_register_quic(void);                          // M46：QUIC 绑定（runtime_quic.c）
void    px_register_h3(void);                            // M47：HTTP/3 语义层（runtime_h3.c）
void    px_register_h3_qpack_dyn(void);                  // M49：QPACK 动态表 + SETTINGS（runtime_h3_qpack_dyn.c）
void    px_register_zlib(void);                          // M61-S1：zlib 外部库绑定（runtime_zlib.c）
// M47：QUIC raw 接口（runtime_quic.c 导出，供 runtime_h3.c 复用底层收发）
int64_t px_quic_raw_listen(int port);
int64_t px_quic_raw_accept(int64_t listener, int timeout_ms);
int64_t px_quic_raw_connect(const char* ip, int port, const char* alpn);
int64_t px_quic_raw_send(int64_t conn, const uint8_t* data, int len);
int64_t px_quic_raw_recv(int64_t conn, uint8_t* out, int maxlen, int timeout_ms);
// M50：QUIC 多流 raw 接口（runtime_quic.c 导出，供 runtime_h3.c 多路复用）
int64_t px_quic_raw_open_stream(int64_t conn);   // 本地 open 新 bidi 流 → sid | -1
int64_t px_quic_raw_open_uni_stream(int64_t conn); // M51：本地 open 新 uni 流 → sid | -1
int64_t px_quic_raw_send_on(int64_t conn, int64_t sid, const uint8_t* data, int len, int fin);
int64_t px_quic_raw_recv_on(int64_t conn, int64_t sid, uint8_t* out, int maxlen, int timeout_ms);
int64_t px_quic_raw_poll(int64_t conn, int timeout_ms);      // 任一活跃流有数据 → sid | -1/-2
int64_t px_quic_raw_first_stream(int64_t conn);              // 最小活跃 sid（默认流）|-1
bool    px_quic_raw_close(int64_t conn);
bool    px_quic_raw_close_listener(int64_t listener);
// M53：HTTP/3 server 多连接托管（runtime_quic.c）——单 fd 收包路由 + 自动 accept
typedef void (*px_quic_conn_cb)(int64_t conn, void* ud);     // 每连接处理回调（握手后）
void    px_quic_raw_h3_set_conn_cb(px_quic_conn_cb cb, void* ud);
int64_t px_quic_raw_h3_listen(int port, const char* cert, const char* key); // → listener id | -1
// M53-S3：以显式连接回调启动 H3 listener（runtime_h3.c 管道托管用）；对端地址查询
int64_t px_quic_raw_h3_listen_cb(int port, const char* cert, const char* key,
                                 px_quic_conn_cb cb, void* ud);
void    px_quic_raw_peer_addr(int64_t conn, char* out, size_t n);   // 连接对端 "ip:port"
// M53-S4：指定流对端 FIN 是否已到（1=是/0=否或流不存在）—— H3 server 判请求无 body
int     px_quic_raw_stream_fin(int64_t conn, int64_t sid);
LXValue px_struct(const char* type_name, char** fnames, LXValue* fvals, int nfields);
LXValue px_enum(const char* type_name, const char* variant);
LXValue px_tuple(LXValue* items, int len);
// M32：生成器对象（创建时物化，gen_next 逐项消费）
LXValue px_gen_from_list(LXValue list);
// M34：惰性生成器（单层 for 延迟求值：seq 不展开，transform/filter 闭包 gen_next 时调用）
LXValue px_gen_lazy(LXValue seq, LXValue transform, LXValue filter);
LXValue px_gen_next(LXValue g);
// M39：Result 构造 / 判断 / 解包（spec §3.5）
LXValue px_ok(LXValue v);
LXValue px_err(LXValue v);
LXValue px_some(LXValue v);
bool    px_is_result(LXValue v);
bool    px_result_ok(LXValue v);
LXValue px_result_unwrap(LXValue v);

// ==================== 类型判断 ====================

bool px_is_null(LXValue v);
bool px_is_truthy(LXValue v);
const char* px_type_name(LXValue v);

// ==================== 运算 ====================

LXValue px_add(LXValue a, LXValue b);
LXValue px_sub(LXValue a, LXValue b);
LXValue px_mul(LXValue a, LXValue b);
LXValue px_div(LXValue a, LXValue b);
LXValue px_idiv(LXValue a, LXValue b);
LXValue px_mod(LXValue a, LXValue b);
LXValue px_pow(LXValue a, LXValue b);
LXValue px_neg(LXValue a);
LXValue px_not(LXValue a);
LXValue px_eq(LXValue a, LXValue b);
LXValue px_ne(LXValue a, LXValue b);
LXValue px_lt(LXValue a, LXValue b);
LXValue px_le(LXValue a, LXValue b);
LXValue px_gt(LXValue a, LXValue b);
LXValue px_ge(LXValue a, LXValue b);
LXValue px_and(LXValue a, LXValue b);
LXValue px_or(LXValue a, LXValue b);
LXValue px_bitnot(LXValue a);
LXValue px_bitand(LXValue a, LXValue b);
LXValue px_bitor(LXValue a, LXValue b);
LXValue px_bitxor(LXValue a, LXValue b);
LXValue px_shl(LXValue a, LXValue b);
LXValue px_shr(LXValue a, LXValue b);
LXValue px_ushr(LXValue a, LXValue b);

// ==================== 容器操作 ====================

LXValue px_index(LXValue obj, LXValue idx);
// M21/M24：切片 a[start:end] / a[start:end:step]（start/end/step 为 null 表示省略；
// str 按 UTF-8 字符、list/tuple/bytes 取元素；step<0 反向，step=0 报错）
LXValue px_slice(LXValue obj, LXValue start, LXValue end, LXValue step);
void px_index_set(LXValue obj, LXValue idx, LXValue val);
LXValue px_field(LXValue obj, const char* name);
void px_field_set(LXValue obj, const char* name, LXValue val);
void px_list_push(LXValue list, LXValue val);
void px_dict_set(LXValue dict, const char* key, LXValue val);
LXValue px_dict_get(LXValue dict, const char* key);
bool px_dict_has(LXValue dict, const char* key);
int px_len(LXValue v);
// 大小写不敏感取 dict 键（HTTP 头等场景；未找到返回 px_null）——M28 起公共
LXValue px_dict_get_ci(LXValue d, const char* key);

// ==================== 调用 ====================

LXValue px_call(LXValue fn, LXValue* args, int nargs);
// 方法调用：obj.method(args...)
LXValue px_method(LXValue obj, const char* name, LXValue* args, int nargs);

// ==================== 内置函数注册 ====================

void px_register_builtins(void);
LXValue px_get_global(const char* name);
void px_set_global(const char* name, LXValue v);
void px_args_init(int argc, char** argv);

// ==================== M19 P1：AES / XML / zip ====================
// 实现文件：runtime_aes.c（mbedtls）、runtime_xml.c、runtime_zip.c（miniz）
LXValue bi_aes_encrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_decrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_gcm_encrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_gcm_decrypt(LXValue* args, int nargs, void* ctx);
// M72-S4（Issue 13 GAP-BIN-1）：AES bytes 版（二进制安全，去 utf8/NUL 限制）
LXValue bi_aes_encrypt_bytes(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_decrypt_bytes(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_gcm_encrypt_bytes(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_gcm_decrypt_bytes(LXValue* args, int nargs, void* ctx);
LXValue bi_xml_parse(LXValue* args, int nargs, void* ctx);
LXValue bi_xml_escape(LXValue* args, int nargs, void* ctx);
LXValue bi_xml_unescape(LXValue* args, int nargs, void* ctx);
// M24：xml_build(node) → str（与 xml_parse 结构对称的 XML 生成）
LXValue bi_xml_build(LXValue* args, int nargs, void* ctx);
LXValue bi_zip_pack(LXValue* args, int nargs, void* ctx);
LXValue bi_zip_unpack(LXValue* args, int nargs, void* ctx);

// ==================== M22 P1：WebSocket（RFC 6455） ====================
// 实现文件：runtime_ws.c（mbedtls sha1 握手 + 帧协议 + 连接注册表）
LXValue bi_ws_serve(LXValue* args, int nargs, void* ctx);
LXValue bi_ws_connect(LXValue* args, int nargs, void* ctx);
// M38：ws_connect_auto(url, reconnect_ms) → conn（断线自动重连）
LXValue bi_ws_connect_auto(LXValue* args, int nargs, void* ctx);
LXValue bi_ws_send(LXValue* args, int nargs, void* ctx);
// M34：ws_broadcast(data) → int（向全部活跃连接群发）
LXValue bi_ws_broadcast(LXValue* args, int nargs, void* ctx);
LXValue bi_ws_recv(LXValue* args, int nargs, void* ctx);
LXValue bi_ws_close(LXValue* args, int nargs, void* ctx);
LXValue bi_ws_ping(LXValue* args, int nargs, void* ctx);
// M26：内置自动心跳（定时 ping + 死链检测）
LXValue bi_ws_heartbeat(LXValue* args, int nargs, void* ctx);
// M23d P1：RSA 非对称加密（实现 runtime_rsa.c）
LXValue bi_rsa_gen_key(LXValue* args, int nargs, void* ctx);
LXValue bi_rsa_encrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_rsa_decrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_rsa_sign(LXValue* args, int nargs, void* ctx);
LXValue bi_rsa_verify(LXValue* args, int nargs, void* ctx);
LXValue ws_conn_worker(LXValue* args, int nargs, void* ctx);
const char* px_val_cstr(LXValue v);

// ==================== 输出 ====================

void px_print_value(LXValue v, bool newline);
char* px_to_string(LXValue v);  // 返回静态缓冲（每次调用覆盖）
int px_unicode_len(const char* s);

// ==================== 并发原语（M4.2） ====================

// 通道：px_chan_create(cap) —— cap=0 无缓冲（rendezvous），cap>0 有缓冲
LXValue px_chan_create(int cap);
LXValue px_chan_send(LXValue ch, LXValue v);  // 阻塞发送（满则等待，关闭报错）
LXValue px_chan_recv(LXValue ch);             // 阻塞接收（空则等待，关闭且空报错）
bool px_chan_try_recv(LXValue ch, LXValue* out); // 非阻塞尝试（select 用）
void px_chan_close(LXValue ch);               // 关闭：唤醒等待者
bool px_is_chan(LXValue v);

// ==================== 锁原语（M13：mutex / rwlock） ====================
// 互斥锁：同一时刻一个持有者；读写锁：多读者并行 + 写优先（防读饿死写）
LXValue px_mutex_create(void);
LXValue px_mutex_lock(LXValue m);
LXValue px_mutex_unlock(LXValue m);
LXValue px_mutex_try_lock(LXValue m);   // 成功 true / 失败 false
LXValue px_rwlock_create(void);
LXValue px_rwlock_rlock(LXValue m);
LXValue px_rwlock_runlock(LXValue m);
LXValue px_rwlock_wlock(LXValue m);
LXValue px_rwlock_wunlock(LXValue m);
LXValue px_rwlock_try_rlock(LXValue m);
LXValue px_rwlock_try_wlock(LXValue m);
bool px_is_mutex(LXValue v);
bool px_is_rwlock(LXValue v);

// spawn：在线程中执行 px_func(fn, args, nargs)，args 由运行时拷贝（调用后可释放）
void px_spawn(LXFuncPtr fn, LXValue* args, int nargs);
// M53-S3：外部裸线程（QUIC/H3 托管连接线程）纳入并发 GC（enter 注册/leave 注销）
void px_gc_thread_enter(void);
void px_gc_thread_leave(void);
// 通用入口：spawn 函数名（由 codegen 调用 px_spawn_name）
void px_spawn_name(const char* fname, LXValue* args, int nargs);

// select：阻塞等待任一通道可接收（返回索引），chan 活动后由运行时自动唤醒
int px_select_wait_any(LXValue* chans, int n);
// select 休眠（无 else 分支时循环等待）
void px_select_wait(void);
// 通道活动通知（send/recv/close 后调用，唤醒 select）
void px_select_signal(void);

// ==================== 运行时错误 ====================

void px_error(const char* fmt, ...) __attribute__((noreturn));
// M72-S2（Issue 10 D1）：编译产物运行时 .px 源位置追踪——cg 在每条可执行语句前
// 生成 px_srcline(<源行>) 调用、每个用户函数入口生成 px_srcfunc("<函数名>")；
// px_error 打印最近位置 → 运行时错误带源行号（AI 一次定位）。线程局部（spawn
// 各协程独立追踪当前位置，S3 隔离用）。
void px_srcline(int line);
void px_srcfunc(const char* name);

// ==================== GC（M8：值对象自动释放） ====================
// 保守标记-清除：所有 LXObject 注册到全局对象表，分配累计超阈值自动触发回收。
// 根集合 = 全局表 + 当前线程栈（保守扫描）+ 暂存根（刚创建对象）。
// 并发（spawn 线程活跃）时自动跳过回收（保持正确性），线程全部退出后自动恢复。
void px_gc_collect(void);
// 返回 GC 次数；live 输出当前存活对象数，total 输出累计回收对象数
int px_gc_stats(int* live, int* total);

// ==================== M28 P1：路由表 + 中间件（runtime_route.c） ====================
typedef struct PxHttpOut PxHttpOut;   // M53-S2：HTTP 输出抽象（结构体定义见下方 M27 段）
LXValue bi_route(LXValue* args, int nargs, void* ctx);
LXValue bi_middleware(LXValue* args, int nargs, void* ctx);
// M31 限流（M33 供 per-route 限流使用）：true 放行 / false 超限
int px_rate_limit_try(const char* key, long long max, long long window_sec);
// M35：访问日志落盘路径 + Alt-Svc 通告（runtime.c 定义，runtime_route.c 引用）
extern char g_px_access_log[1024];
extern char g_px_alt_svc[256];
// M35：HTTP/2 最小服务端（runtime_h2.c）：h2c Upgrade + HPACK + 帧层
void px_h2_handle(void* c, int upgrade, const unsigned char* residual, int rlen);
// M33：结构化访问日志（stderr + 落盘 + 轮转；runtime_route.c 的 per-route 429 共用）
void px_access_log(const char* fmt, ...);
// 路由表非空？（决定 px_serve 是否走路由优先）
int px_route_has(void);
// 匹配路由并执行中间件链 + handler，发送响应。返回 1=已处理 / 0=未匹配。
// out 为 PxHttpOut*（M53-S2：HTTP/1.1 与 HTTP/3 共用输出抽象）
int px_route_try_dispatch(PxHttpOut* out, LXValue req, const char* method, int head_only,
                          int keep_alive, const char* req_id);
// M28 P1：SQLite 绑定（runtime_sqlite.c）
LXValue bi_sqlite_open(LXValue* args, int nargs, void* ctx);
LXValue bi_sqlite_exec(LXValue* args, int nargs, void* ctx);
LXValue bi_sqlite_query(LXValue* args, int nargs, void* ctx);
LXValue bi_sqlite_close(LXValue* args, int nargs, void* ctx);
LXValue bi_sqlite_escape(LXValue* args, int nargs, void* ctx);
LXValue bi_sqlite_last_insert_rowid(LXValue* args, int nargs, void* ctx);
// M28 P1：时间时区 + cron（runtime.c 内实现）
LXValue bi_time_format(LXValue* args, int nargs, void* ctx);
LXValue bi_time_parse(LXValue* args, int nargs, void* ctx);
LXValue bi_tz_offset(LXValue* args, int nargs, void* ctx);
LXValue bi_cron(LXValue* args, int nargs, void* ctx);

// ==================== M27 P0：服务端 TLS / PxConn 连接抽象 ====================
// PxConn 统一明文/TLS 连接（px_serve / sse_serve / ws_serve 服务端用）：
// TLS 成员用 void* 保持 runtime.h 不依赖 mbedtls 头文件（runtime.c 内转型使用）。
typedef struct PxConn {
    int fd;
    int is_tls;        // 1 = TLS（已握手）
    void* ssl;         // mbedtls_ssl_context*
    void* conf;        // mbedtls_ssl_config*
    void* ctr_drbg;    // mbedtls_ctr_drbg_context*
    void* entropy;     // mbedtls_entropy_context*
    unsigned char rbuf[16384]; // TLS 读缓冲（SSL_read 一次可多读）
    int rlen, roff;
    int closed;        // 连接已关闭（px_conn_close 置 1；对象保留避免并发 use-after-free）
    int owned;         // M32：1 = ssl/conf/ctr_drbg/entropy 独立 malloc（px_conn_close 释放）；
                       //      0 = 指向外部 HttpsSession（wss 客户端，由 px_https_close_ex 释放）
} PxConn;

// 初始化（fd 上做 TLS 握手若服务端 TLS 已注册；失败返回 -1）
int px_conn_init(PxConn* c, int fd);
ssize_t px_conn_read(PxConn* c, void* buf, size_t n);
ssize_t px_conn_write(PxConn* c, const void* buf, size_t n);
void px_conn_close(PxConn* c);
// 当前线程正在处理的连接（px_px_send 等旧 fd 接口自动转发 TLS 写）
extern __thread PxConn* g_cur_conn;
// 在途请求数（px_serve/sse_serve/ws_serve 连接线程计数；优雅关闭等待归零）
extern volatile int g_px_inflight;

// ==================== M53-S2：HTTP 输出抽象（PxHttpOut） ====================
// 请求管道（px_http_dispatch / px_route_try_dispatch）只面向 PxHttpOut 写响应，
// 不直接触碰 fd/g_cur_conn —— 使 HTTP/1.1（TCP/TLS 文本头）与 HTTP/3（QUIC 流
// H3 HEADERS/DATA 帧）共用同一套 vhost/路由/限流/日志/静态/.px 管道。
//   respond：一次性完整响应（头 + body）；HEAD 由 head_only 控制不发 body
//   begin/write/end：流式（大文件 Range 分段 / gzip 直发 / H3 DATA 分帧）
//   impl：传输后端（HTTP/1.1 = PxConn*）
typedef void (*PxOutRespondFn)(PxHttpOut* o, int status, const char* ct,
                               const char* body, int body_len, int head_only,
                               int keep_alive, const char* extra_headers);
typedef void (*PxOutBeginFn)(PxHttpOut* o, int status, const char* ct,
                             long long body_len, int head_only, int keep_alive,
                             const char* extra_headers);
typedef int  (*PxOutWriteFn)(PxHttpOut* o, const void* buf, size_t n);
typedef void (*PxOutEndFn)(PxHttpOut* o);
struct PxHttpOut {
    void* impl;
    PxOutRespondFn respond;  // 一次性完整响应
    PxOutBeginFn begin;      // 流式：响应头（Content-Length: body_len）
    PxOutWriteFn write;      // 流式：body 片段
    PxOutEndFn end;          // 流式收尾（HTTP/1.1 no-op；H3 发 DATA FIN）
};
// HTTP/1.1 实现初始化（impl=PxConn*；明文/TLS 统一，行为与旧 px_px_send_ex 一致）
void px_http_out_init_conn(PxHttpOut* o, PxConn* c);
// M53-S3：HTTP/3 请求接入桥（runtime.c）——把 H3 req dict 补全为与 HTTP/1.1 等价
// （query 拆分+解码 / version="HTTP/3" / request_id / cookie / form / body gzip 解压）
// 后送入公共管道 px_http_dispatch。req 需含 method/path/headers/body/remote（sid 等可选）。
void px_http_dispatch_h3(PxHttpOut* pout, LXValue req, int client_keep_alive);
// M53-S4：px_serve opts.http3 用 —— 以公共 HTTP 管道托管启动 H3（QUIC/UDP）listener。
// cert/key 为空串 → 运行时自签（测试）；返回 listener id | -1（runtime_h3.c 定义）。
int64_t px_h3_server_listen_pipe(int port, const char* cert, const char* key);

#ifdef __cplusplus
}
#endif

#endif // PX_RUNTIME_H
// 调试辅助：对象是否仍注册在对象表（未回收）
int px_gc_contains(LXObject* o);
