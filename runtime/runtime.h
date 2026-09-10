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
#include <signal.h>      // M93-S2: sigset_t（协程 worker GC 信号屏蔽导出 API 参数）
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
struct PxCoro;   // M93-S3：帧协程（coro.c）——对象协程等待者链表节点类型前向声明

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
        struct {
            char* data; int len;
            // M106-S2（Issue 33-A）：str 惰性 rune 缓存 —— 把 s[i] / len(s) / for ch in s
            //   从 O(n) 降为摊还 O(1)。仅 PX_STR 使用；PX_BYTES 共享同一表示，但恒为
            //   (-1, 0, NULL)，且按 type 分派，绝不为 bytes 建表。
            //     rune_len ：px_unicode_len_n 的缓存值（-1 = 未计算）
            //     offs_cnt ：rune_offs 表有效条目数（表已分配 offs_cnt 个 int）
            //     rune_offs：惰性 rune→byte 起始偏移表（PX_STR_OFFS_MIN ≤ 字节长度 ≤ PX_STR_OFFS_MAX 时建；
            //               超上界回落线性走查，防巨串 4× 内存放大 —— M106-S3 护栏）
            //   二者均为纯数据（非 LXObject*）→ gc_mark_obj 的 PX_STR/PX_BYTES
            //   default 分支不扫描 union，precise/conservative 双模式都不受影响。
            int rune_len; int offs_cnt; int* rune_offs;
        } str;
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
            // M93-S3：协程等待者链表（帧协程让出登记；与 pthread cond_wait 双轨共存，
            //   唤醒方在 mu 保护下摘链入就绪队列）。协程节点 next 指针 = PxCoro.w_next。
            struct PxCoro* cw_send;  // 协程 send 等待者（满 / 无缓冲等接收者）
            struct PxCoro* cw_recv;  // 协程 recv 等待者（空）
        } chan;
        struct {
            pthread_mutex_t mu;      // 保护 locked
            pthread_cond_t cv;       // 等待者
            int locked;              // 是否被持有
            struct PxCoro* cw;       // M93-S3：协程 lock 等待者（互斥锁持有者让出）
        } mutex;
        struct {
            pthread_mutex_t mu;      // 保护状态
            pthread_cond_t cv;
            int readers;             // 活跃读者数
            int writer;              // 写者持有
            int writer_waiting;      // 等待中的写者数（写优先）
            // M93-S3：协程等待者链表（写优先：先唤醒 cw_w，无则 cw_r）
            struct PxCoro* cw_w;
            struct PxCoro* cw_r;
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
// M89-S3-B3b：enum 值变体名（match 模式匹配 subject.type==PX_ENUM && variant==name 用；
//   非 enum → px_null，不报错——对齐 cg 的 type 短路判断）
LXValue px_enum_variant(LXValue v);
LXValue px_tuple(LXValue* items, int len);
// M32：生成器对象（创建时物化，gen_next 逐项消费）
LXValue px_gen_from_list(LXValue list);
// M34：惰性生成器（单层 for 延迟求值：seq 不展开，transform/filter 闭包 gen_next 时调用）
LXValue px_gen_lazy(LXValue seq, LXValue transform, LXValue filter);
LXValue px_gen_next(LXValue g);
// S3-D 止血：外部 GC 根标记（vm.c 帧槽等堆上根区间 → 补进当前 GC 标记集）
void px_gc_mark_slots(LXValue* base, int n);
// VM 活跃帧槽根标记实现（vm.c；runtime 单线程 GC 弱引用调用）
void px_vm_gc_mark(void);
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
// M83-S2（Issue 20 GAP-AES-1）：AES-ECB（PKCS7，无 IV）——hex 版（aes_encrypt_ecb/aes_decrypt_ecb）
//   供文本互通场景（与 Go crypto/aes NewCipher 块加密逐字节一致，openssl enc -aes-128-ecb 可对拍）；
//   微信网关媒体全链路 AES-128-ECB 二进制（CDN 下载→ECB 解密→上传）走 _bytes 版。
LXValue bi_aes_encrypt_ecb(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_decrypt_ecb(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_encrypt_ecb_bytes(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_decrypt_ecb_bytes(LXValue* args, int nargs, void* ctx);
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
// M83-S3（Issue 17 GAP-ED25519-1）：ed25519 签名/验签（RFC8032，tweetnacl；实现 runtime_ed25519.c）
LXValue bi_ed25519_sign(LXValue* args, int nargs, void* ctx);
LXValue bi_ed25519_verify(LXValue* args, int nargs, void* ctx);
// M103-S2b（Issue 29 GAP-ED25519-2）：ed25519_keygen 密钥对生成
LXValue bi_ed25519_keygen(LXValue* args, int nargs, void* ctx);
// M103-S2d（Issue 30 GAP-IMG）：图片解码/缩放/JPEG 编码（stb；实现 runtime_image.c）
LXValue bi_img_decode(LXValue* args, int nargs, void* ctx);
LXValue bi_img_scale(LXValue* args, int nargs, void* ctx);
LXValue bi_img_encode_jpeg(LXValue* args, int nargs, void* ctx);
// M83-S4（Issue 18 GAP-RSA-1）：RSA PKCS1v15-SHA256 标准签名（PEM 入参，DigestInfo 自动封装；实现 runtime_rsa.c）
LXValue bi_rsa_sign_pkcs1v15_sha256(LXValue* args, int nargs, void* ctx);
LXValue bi_rsa_verify_pkcs1v15_sha256(LXValue* args, int nargs, void* ctx);
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
bool px_chan_try_send(LXValue ch, LXValue v);    // M93-S3：非阻塞发送（满/无接收者 false）
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
// M89-S3-B5：带上下文 spawn（VM PX_FUNC=px_vm_entry 需 ctx=PxVMFunc* 定位字节码函数；
//   旧 C 编译产物 ctx=NULL —— px_spawn_name 透传 func.ctx）
void px_spawn_ctx(LXFuncPtr fn, void* ctx, LXValue* args, int nargs);
// M53-S3：外部裸线程（QUIC/H3 托管连接线程）纳入并发 GC（enter 注册/leave 注销）
void px_gc_thread_enter(void);
void px_gc_thread_leave(void);
// 通用入口：spawn 函数名（由 codegen 调用 px_spawn_name）
void px_spawn_name(const char* fname, LXValue* args, int nargs);
// M93-S2：spawn 错误隔离点（coro worker 复用 spawn_thread 的 setjmp 隔离语义）。
//   begin 正常返回 1（捕获点已装）；错误 longjmp 回 → 返回 0（协程异常终止）。
int  px_spawn_isolate_begin(void);
void px_spawn_isolate_end(void);
// M93-S2：GC 暂停信号屏蔽原语（coro.c 临界区用；屏蔽 SIG_GC_STOP → 持锁临界区
//   不被 STW 打断 → GC executor 标记协程表拿锁不与其死锁）。
void px_gc_block_stop_sig(sigset_t* old);
void px_gc_unblock_stop_sig(const sigset_t* old);

// ==================== M93-S3：阻塞原语让出桥（协程登记 + 唤醒） ====================
// 设计（docs/M93_PLAN.md D3/D4，路线 B）：阻塞 native（chan/mutex/rwlock/sleep）在
//   VM 解释循环（CALL/CALLM）被预检拦截 —— 协程上下文里先走 try 变体，失败则把
//   当前协程登记到对象等待链表并让出（worker 不阻塞、继续取下一协程），条件满足
//   后由唤醒方摘链入就绪队列 → 协程恢复重试 try。非协程上下文（主线程 / 逃生舱
//   pthread / 嵌套 native 回调）返回 0 → 调用方走原 pthread 阻塞路径（语义零变化）。
// 返回码约定（各 wait 函数）：0 = 不在协程上下文（走原阻塞）；1 = 已登记并让出；
//   2 = 条件已满足（调用方重新 try）。
#define PX_CORO_WAIT_BLOCKED 1   // 已登记让出（worker 继续取下一协程）
#define PX_CORO_WAIT_RETRY    2   // 条件已满足，重新 try
int  px_coro_active(void);                          // 当前线程是否在协程上下文
int  px_coro_chan_send_wait(LXValue ch);            // try_send 失败后登记 send 等待
int  px_coro_chan_recv_wait(LXValue ch);            // try_recv 失败后登记 recv 等待
int  px_coro_mutex_wait(LXValue m);                 // try_lock 失败后登记 lock 等待
int  px_coro_rwlock_wait_r(LXValue m);              // try_rlock 失败后登记 rlock 等待
int  px_coro_rwlock_wait_w(LXValue m);              // try_wlock 失败后登记 wlock 等待
int  px_coro_sleep_us(long long us);                // 协程 ctx：登记定时器让出；否则 0
// 唤醒原语（runtime.c 在对象锁内摘链 → 解锁后 wake；协程恢复后重试 try 成功）
struct PxCoro* px_coro_take_waiter(struct PxCoro** head);  // 摘链首节点（调用方持对象锁）
void           px_coro_wake(struct PxCoro* c);             // 入就绪队列（内部 g_coro_mu）
// 阻塞 native 识别（vm.c CALL 预检用）：fn 为 sleep/sleep_us 返回类型码，否则 0
#define PX_BLK_NONE 0
#define PX_BLK_SLEEP_MS 1
#define PX_BLK_SLEEP_US 2
int px_native_blocking_kind(LXValue fn);
// M96-S2：阻塞网络 native offload 识别（vm.c CALL 预检用）。命中名单（C 层全协议
//   阻塞桥：http_get/tcp_recv 等客户端网络，M94 抢占救不了）返回 1，否则 0。
//   按 native 名字匹配（as.native.name，跨文件安全 —— ws 在 runtime_ws.c）。名单在
//   runtime.c 维护（S2 试点 http_get/tcp_recv；S3 扩全集）。
int px_native_offload_kind(LXValue fn);
// M96-S2：offload 提交（coro.c 定义；vm.c CALL 预检调）。协程 ctx：打包任务入外包
//   执行线程池 + 协程登记 offload 等待（c->off_task/off_dst）→ 返回 PX_CORO_WAIT_BLOCKED
//   （调用方让出，pc 不回退）。失败（非协程 ctx / 内存）→ 0（调用方落 px_call 直调）。
//   args 数组所有权转移给任务（外包线程执行完释放）；调用方不得再 free。
int px_coro_offload_submit(LXValue fn, LXValue* args, int nargs, int dst);

// select：阻塞等待任一通道可接收（返回索引），chan 活动后由运行时自动唤醒
int px_select_wait_any(LXValue* chans, int n);
// select 休眠（无 else 分支时循环等待）
void px_select_wait(void);
// 通道活动通知（send/recv/close 后调用，唤醒 select）
void px_select_signal(void);

// ==================== 运行时错误 ====================

void px_error(const char* fmt, ...) __attribute__((noreturn));
// M96-S2：受保护 native 调用（offload 外包线程用）。setjmp 在本函数内消化 longjmp：
//   px_call → px_error → longjmp 回本函数 setjmp → return 1（错误已打印现场，errbuf
//   带回本线程最后一次 px_error 文本）。调用者只见普通一次返回（0=成功 *out 有效；
//   1=px_error 被捕获）—— 无跨函数 returns_twice 语义泄漏（gcc 对「包装 setjmp 的
//   helper 返回后分支」优化不可靠：最小复现 -O1/-O2 下调用者把 helper 返回 0 误判为
//   真 → 死循环/段错误 → 必须同函数消化，不把 setjmp 分支暴露给跨编译单元调用者）。
int px_native_call_capture(LXValue fn, LXValue* args, int nargs,
                           LXValue* out, char* errbuf, int errbuf_sz);
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
// ISSUE28-B1：请求间/空闲安全点回收（多线程服务模式延迟 GC 的触发点；单线程零开销）
void px_gc_poll(void);
// 返回 GC 次数；live 输出当前存活对象数，total 输出累计回收对象数
int px_gc_stats(int* live, int* total);
// M92 精确 GC（退役整栈保守扫描）：precise/conservative 双模式 + native 桥根登记。
//   px_gc_set_precise(1) = precise（VM 轨产物 main 调用；根=全局槽+VM 帧槽+TLS 登记根栈，
//     跳过整 C 栈保守扫描）；0 = conservative（默认；C 轨逃生舱产物，保持旧行为）。
//   px_root_push/pop 界定登记作用域（native 桥入口/出口配对）；px_root_keep/PX_KEEP 登记
//     跨「可能触发 GC 的调用」的局部 LXValue 引用（args 指向对象无需登记——调用者帧槽/
//     上层根已保护；冗余登记无害，漏登记 = GC 误回收 use-after-free）。
void px_gc_set_precise(int precise);
void px_root_push(void);
void px_root_pop(void);
void px_root_keep(const LXValue* v);
#define PX_KEEP(v) px_root_keep(&(v))

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
// 匹配路由并执行中间件链 + handler，发送响应。返回 0=未匹配 / 1=已处理 / 2=已拆段
// （route VM handler 已协程化，调用方须释放 worker，完成回调投回续处理）。
// out 为 PxHttpOut*（M53-S2：HTTP/1.1 与 HTTP/3 共用输出抽象）；async_ok=1 允许
// route VM handler 拆段异步执行（仅 px_serve HTTP/1.1 池 worker 传 1；H3/非 VM 零变化）。
int px_route_try_dispatch(PxHttpOut* out, LXValue req, const char* method, int head_only,
                          int keep_alive, const char* req_id, int async_ok);
// M98-S2a：route handler 返回值 → 归一化 + 响应发送 + 访问日志（段2）。
//   同步路径（px_route_try_dispatch 内 px_call 后）与协程续处理（px_serve 续 worker
//   从挂起表取回 stage2 后）共用 —— 保证 async/sync 响应语义逐字节一致。
void px_route_respond(PxHttpOut* out, LXValue req, const char* method, int head_only,
                      int keep_alive, const char* req_id, LXValue resp);
// M100（runtime_route.c 定义）：middleware 短路响应（归一化 + respond + (middleware)
//   访问日志）—— 同步短路（px_route_try_dispatch 内）与协程续处理段2（kind=3）共用，
//   保证 async/sync 短路响应语义逐字节一致。
void px_route_mw_short_respond(PxHttpOut* out, LXValue req, const char* method,
                               int head_only, int keep_alive, const char* req_id,
                               LXValue resp);
// M98-S2a/S2b（runtime.c 实现）：px_serve route/vhost handler 挂起登记 + 帧协程 spawn。
//   命中 VM handler（handler.type==PX_FUNC && fn==px_vm_entry）且连接在 px 挂起
//   注册表（stage 0）→ stage=1 + kind/vroot 记录 + req 入 GC 根 + px_coro_spawn_ex
//   (handler, hargs, nargs, done) → 返回 1（调用方返回 DEFER）；否则返回 0（调用方走
//   原同步直调路径）。kind=0 route（段2 px_route_respond）/ 1 vhost（段2 respond 或
//   null 回退续管道）。
int px_pxserve_defer(PxHttpOut* out, LXValue req, LXValue handler, LXValue* hargs, int nargs,
                     int kind, const char* vroot, int head_only, int keep_alive,
                     const char* req_id);
// M100（runtime.c 实现）：middleware 链 defer 登记 + spawn 首段。调用点 runtime_route.c
//   px_route_try_dispatch —— middleware 链全 VM（每段 PX_FUNC 且 fn==px_vm_entry）且
//   handler 亦 VM 且 async_ok 时启用：链状态机逐段帧协程（null → 推进下一 middleware /
//   全 null → handler 段 / 非 null → 短路 kind=3 段2）。mws[0..mw_count) 链快照 +
//   handler/params 入 PxPend GC 根。返回 1 = 已登记+已 spawn 首段（调用方返回 DEFER/2）；
//   0 = 退回原同步直调路径（非 px_serve 连接 / 槽忙 / 无协程内核）。
int px_pxserve_mw_defer(PxHttpOut* out, LXValue req, LXValue handler, LXValue params,
                        LXValue* mws, int mw_count, int head_only, int keep_alive,
                        const char* req_id);
// M98-S2a：px 连接挂起表 GC 补标（gc 标记期调用；同 http_pend_gc_mark）
void px_pxserve_pend_gc_mark(void);
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
    void* own_pk;      // M101: per-连接服务端私钥副本（mbedtls_pk_context*，RSA clone；
                       //      多 worker 并发握手消除共享 g_srv_key 签名 data race；
                       //      owned=1 时 px_conn_close 释放）
    void* own_pk_sni;  // M101: SNI 命中的 per-连接私钥副本（mbedtls_pk_context*，若有；
                       //      px_sni_cb 内 clone；owned=1 时 px_conn_close 释放）
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
