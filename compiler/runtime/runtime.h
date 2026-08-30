// 普贤 (PuXian) C 运行时库 — runtime.h
// M4 编译模式：动态值系统 + 内置函数
// MVP 策略：值对象不自动释放（进程退出回收），后续 M6 加 GC
#ifndef LX_RUNTIME_H
#define LX_RUNTIME_H

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
    LX_NULL = 0,
    LX_BOOL,
    LX_INT,
    LX_FLOAT,
    LX_STR,
    LX_LIST,
    LX_DICT,
    LX_FUNC,    // 用户函数（编译为 C 函数）
    LX_NATIVE,  // 内置函数
    LX_STRUCT,  // 结构体实例
    LX_ENUM,    // 枚举值
    LX_TUPLE,   // 元组
    LX_CHAN,    // 通道（并发原语）
    LX_MUTEX,   // 互斥锁（M13：P1 锁原语）
    LX_RWLOCK,  // 读写锁（M13：读多写少）
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

LXValue lx_null(void);
LXValue lx_bool(bool b);
LXValue lx_int(int64_t i);
LXValue lx_float(double f);
LXValue lx_str(const char* s);
LXValue lx_str_len(const char* s, int len);
LXValue lx_list(int cap);
LXValue lx_list_n(LXValue* items, int n);
LXValue lx_dict(void);
LXValue lx_func(const char* name, LXFuncPtr fn, void* ctx);
LXValue lx_native(const char* name, LXFuncPtr fn);
LXValue lx_struct(const char* type_name, char** fnames, LXValue* fvals, int nfields);
LXValue lx_enum(const char* type_name, const char* variant);
LXValue lx_tuple(LXValue* items, int len);

// ==================== 类型判断 ====================

bool lx_is_null(LXValue v);
bool lx_is_truthy(LXValue v);
const char* lx_type_name(LXValue v);

// ==================== 运算 ====================

LXValue lx_add(LXValue a, LXValue b);
LXValue lx_sub(LXValue a, LXValue b);
LXValue lx_mul(LXValue a, LXValue b);
LXValue lx_div(LXValue a, LXValue b);
LXValue lx_idiv(LXValue a, LXValue b);
LXValue lx_mod(LXValue a, LXValue b);
LXValue lx_pow(LXValue a, LXValue b);
LXValue lx_neg(LXValue a);
LXValue lx_not(LXValue a);
LXValue lx_eq(LXValue a, LXValue b);
LXValue lx_ne(LXValue a, LXValue b);
LXValue lx_lt(LXValue a, LXValue b);
LXValue lx_le(LXValue a, LXValue b);
LXValue lx_gt(LXValue a, LXValue b);
LXValue lx_ge(LXValue a, LXValue b);
LXValue lx_and(LXValue a, LXValue b);
LXValue lx_or(LXValue a, LXValue b);
LXValue lx_bitnot(LXValue a);
LXValue lx_bitand(LXValue a, LXValue b);
LXValue lx_bitor(LXValue a, LXValue b);
LXValue lx_bitxor(LXValue a, LXValue b);
LXValue lx_shl(LXValue a, LXValue b);
LXValue lx_shr(LXValue a, LXValue b);

// ==================== 容器操作 ====================

LXValue lx_index(LXValue obj, LXValue idx);
void lx_index_set(LXValue obj, LXValue idx, LXValue val);
LXValue lx_field(LXValue obj, const char* name);
void lx_field_set(LXValue obj, const char* name, LXValue val);
void lx_list_push(LXValue list, LXValue val);
void lx_dict_set(LXValue dict, const char* key, LXValue val);
LXValue lx_dict_get(LXValue dict, const char* key);
bool lx_dict_has(LXValue dict, const char* key);
int lx_len(LXValue v);

// ==================== 调用 ====================

LXValue lx_call(LXValue fn, LXValue* args, int nargs);
// 方法调用：obj.method(args...)
LXValue lx_method(LXValue obj, const char* name, LXValue* args, int nargs);

// ==================== 内置函数注册 ====================

void lx_register_builtins(void);
LXValue lx_get_global(const char* name);
void lx_set_global(const char* name, LXValue v);

// ==================== 输出 ====================

void lx_print_value(LXValue v, bool newline);
char* lx_to_string(LXValue v);  // 返回静态缓冲（每次调用覆盖）
int lx_unicode_len(const char* s);

// ==================== 并发原语（M4.2） ====================

// 通道：lx_chan_create(cap) —— cap=0 无缓冲（rendezvous），cap>0 有缓冲
LXValue lx_chan_create(int cap);
LXValue lx_chan_send(LXValue ch, LXValue v);  // 阻塞发送（满则等待，关闭报错）
LXValue lx_chan_recv(LXValue ch);             // 阻塞接收（空则等待，关闭且空报错）
bool lx_chan_try_recv(LXValue ch, LXValue* out); // 非阻塞尝试（select 用）
void lx_chan_close(LXValue ch);               // 关闭：唤醒等待者
bool lx_is_chan(LXValue v);

// ==================== 锁原语（M13：mutex / rwlock） ====================
// 互斥锁：同一时刻一个持有者；读写锁：多读者并行 + 写优先（防读饿死写）
LXValue lx_mutex_create(void);
LXValue lx_mutex_lock(LXValue m);
LXValue lx_mutex_unlock(LXValue m);
LXValue lx_mutex_try_lock(LXValue m);   // 成功 true / 失败 false
LXValue lx_rwlock_create(void);
LXValue lx_rwlock_rlock(LXValue m);
LXValue lx_rwlock_runlock(LXValue m);
LXValue lx_rwlock_wlock(LXValue m);
LXValue lx_rwlock_wunlock(LXValue m);
LXValue lx_rwlock_try_rlock(LXValue m);
LXValue lx_rwlock_try_wlock(LXValue m);
bool lx_is_mutex(LXValue v);
bool lx_is_rwlock(LXValue v);

// spawn：在线程中执行 lx_func(fn, args, nargs)，args 由运行时拷贝（调用后可释放）
void lx_spawn(LXFuncPtr fn, LXValue* args, int nargs);
// 通用入口：spawn 函数名（由 codegen 调用 lx_spawn_name）
void lx_spawn_name(const char* fname, LXValue* args, int nargs);

// select：阻塞等待任一通道可接收（返回索引），chan 活动后由运行时自动唤醒
int lx_select_wait_any(LXValue* chans, int n);
// select 休眠（无 else 分支时循环等待）
void lx_select_wait(void);
// 通道活动通知（send/recv/close 后调用，唤醒 select）
void lx_select_signal(void);

// ==================== 运行时错误 ====================

void lx_error(const char* fmt, ...) __attribute__((noreturn));

// ==================== GC（M8：值对象自动释放） ====================
// 保守标记-清除：所有 LXObject 注册到全局对象表，分配累计超阈值自动触发回收。
// 根集合 = 全局表 + 当前线程栈（保守扫描）+ 暂存根（刚创建对象）。
// 并发（spawn 线程活跃）时自动跳过回收（保持正确性），线程全部退出后自动恢复。
void lx_gc_collect(void);
// 返回 GC 次数；live 输出当前存活对象数，total 输出累计回收对象数
int lx_gc_stats(int* live, int* total);

#ifdef __cplusplus
}
#endif

#endif // LX_RUNTIME_H
// 调试辅助：对象是否仍注册在对象表（未回收）
int lx_gc_contains(LXObject* o);
