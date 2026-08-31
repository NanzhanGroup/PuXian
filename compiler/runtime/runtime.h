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
LXValue px_struct(const char* type_name, char** fnames, LXValue* fvals, int nfields);
LXValue px_enum(const char* type_name, const char* variant);
LXValue px_tuple(LXValue* items, int len);

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

// ==================== 调用 ====================

LXValue px_call(LXValue fn, LXValue* args, int nargs);
// 方法调用：obj.method(args...)
LXValue px_method(LXValue obj, const char* name, LXValue* args, int nargs);

// ==================== 内置函数注册 ====================

void px_register_builtins(void);
LXValue px_get_global(const char* name);
void px_set_global(const char* name, LXValue v);

// ==================== M19 P1：AES / XML / zip ====================
// 实现文件：runtime_aes.c（mbedtls）、runtime_xml.c、runtime_zip.c（miniz）
LXValue bi_aes_encrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_decrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_gcm_encrypt(LXValue* args, int nargs, void* ctx);
LXValue bi_aes_gcm_decrypt(LXValue* args, int nargs, void* ctx);
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
LXValue bi_ws_send(LXValue* args, int nargs, void* ctx);
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

// ==================== GC（M8：值对象自动释放） ====================
// 保守标记-清除：所有 LXObject 注册到全局对象表，分配累计超阈值自动触发回收。
// 根集合 = 全局表 + 当前线程栈（保守扫描）+ 暂存根（刚创建对象）。
// 并发（spawn 线程活跃）时自动跳过回收（保持正确性），线程全部退出后自动恢复。
void px_gc_collect(void);
// 返回 GC 次数；live 输出当前存活对象数，total 输出累计回收对象数
int px_gc_stats(int* live, int* total);

#ifdef __cplusplus
}
#endif

#endif // PX_RUNTIME_H
// 调试辅助：对象是否仍注册在对象表（未回收）
int px_gc_contains(LXObject* o);
