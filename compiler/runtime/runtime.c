// 普贤 (PuXian) C 运行时库 — runtime.c
// M4 编译模式：动态值系统 + 内置函数
// M8：值对象自动释放（保守标记-清除 GC；并发活跃时降级不回收，线程退出后自动恢复）
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

// M10 HTTPS：mbedtls 静态库（compiler/runtime/mbedtls/）
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"

// std.net（M5.2/M10）前向声明（定义在文件尾部，注册函数在前部使用）
static LXValue bi_tcp_listen(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_accept(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_connect(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_send(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_recv(LXValue* args, int nargs, void* ctx);
static LXValue bi_tcp_close(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_get(LXValue* args, int nargs, void* ctx);
static LXValue bi_http_post(LXValue* args, int nargs, void* ctx);

// M10 HTTPS 内部辅助
static char* lx_http_request(const char* url, const char* method, const char* body, int* out_len);
static int lx_https_request(const char* host, int port, const char* req, char** out, int* out_len);

// ==================== 内存分配（MVP 不释放） ====================

static void* xmalloc(size_t n) {
    void* p = malloc(n ? n : 1);
    if (!p) { fprintf(stderr, "lx: 内存不足\n"); exit(1); }
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
// 并发保护：spawn 线程活跃（g_active_threads > 0）时跳过回收（保持 MVP 行为），
// 全部线程退出后自动恢复 GC。保守扫描只认对象本体地址（8 字节对齐），
// 误标仅推迟回收（安全），漏标由暂存根 + 全局表 + 栈扫描综合兜底。

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
static int g_active_threads = 0;   // spawn 活跃线程数（>0 时 GC 跳过）
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
    h->slots = calloc(c, sizeof(uintptr_t));
    h->cap = c;
    h->count = 0;
}

static void gc_hash_insert(GCHash* h, uintptr_t addr) {
    if ((h->count + 1) * 10 >= h->cap * 7) {  // load factor 0.7 扩容
        size_t ncap = h->cap << 1;
        uintptr_t* ns = calloc(ncap, sizeof(uintptr_t));
        for (size_t i = 0; i < h->cap; i++) {
            uintptr_t a = h->slots[i];
            if (!a) continue;
            size_t j = (size_t)(a ^ (a >> 16)) & (ncap - 1);
            while (ns[j]) j = (j + 1) & (ncap - 1);
            ns[j] = a;
        }
        free(h->slots);
        h->slots = ns;
        h->cap = ncap;
    }
    size_t j = (size_t)(addr ^ (addr >> 16)) & (h->cap - 1);
    while (h->slots[j]) j = (j + 1) & (h->cap - 1);
    h->slots[j] = addr;
    h->count++;
}

static bool gc_hash_has(GCHash* h, uintptr_t addr) {
    size_t j = (size_t)(addr ^ (addr >> 16)) & (h->cap - 1);
    while (h->slots[j]) {
        if (h->slots[j] == addr) return true;
        j = (j + 1) & (h->cap - 1);
    }
    return false;
}

static void gc_hash_free(GCHash* h) { free(h->slots); h->slots = NULL; }

static void gc_init_env(void) {
    const char* d = getenv("PX_GC_DEBUG");
    if (d && d[0] == '1') g_gc_debug = 1;
    const char* t = getenv("PX_GC_THRESHOLD");
    if (t && atoi(t) > 0) g_gc_threshold = atoi(t);
    g_gc_env_inited = 1;
}

static void gc_debug(const char* fmt, ...) {
    if (!g_gc_debug) return;
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[px-gc] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

static bool lx_value_is_obj(LXValue v) {
    switch (v.type) {
        case LX_STR:
        case LX_LIST:
        case LX_DICT:
        case LX_FUNC:
        case LX_NATIVE:
        case LX_STRUCT:
        case LX_ENUM:
        case LX_TUPLE:
        case LX_CHAN:
            return true;
        default:
            return false;
    }
}

// 释放对象内部子分配 + 对象本体（sweep 阶段调用）
static void lx_obj_free(LXObject* o) {
    switch (o->type) {
        case LX_STR: free(o->as.str.data); break;
        case LX_LIST: free(o->as.list.items); break;
        case LX_DICT:
            for (int i = 0; i < o->as.dict.len; i++) free(o->as.dict.keys[i]);
            free(o->as.dict.keys);
            free(o->as.dict.vals);
            break;
        case LX_FUNC: free(o->as.func.name); break;
        case LX_NATIVE: free(o->as.native.name); break;
        case LX_STRUCT:
            free(o->as.struct_inst.type_name);
            for (int i = 0; i < o->as.struct_inst.nfields; i++) free(o->as.struct_inst.fnames[i]);
            free(o->as.struct_inst.fnames);
            free(o->as.struct_inst.fvals);
            break;
        case LX_ENUM:
            free(o->as.enum_inst.type_name);
            free(o->as.enum_inst.variant);
            break;
        case LX_TUPLE: free(o->as.tuple.items); break;
        case LX_CHAN:
            free(o->as.chan.buf);
            pthread_mutex_destroy(&o->as.chan.mu);
            pthread_cond_destroy(&o->as.chan.cv_send);
            pthread_cond_destroy(&o->as.chan.cv_recv);
            break;
        default: break;
    }
    free(o);
}

// 标记单个对象及其可达子对象（显式栈 DFS，避免深链递归栈溢出）
static void gc_mark_obj(GCHash* set, LXObject* o) {
    if (!o) return;
    (void)set;
    LXObject** stack = xmalloc(sizeof(LXObject*) * 1024);
    int cap = 1024, sp = 0;
#define PUSH_OBJ(x) do { if (sp >= cap) { cap *= 2; stack = realloc(stack, sizeof(LXObject*) * cap); } stack[sp++] = (x); } while (0)
    PUSH_OBJ(o);
    while (sp > 0) {
        LXObject* cur = stack[--sp];
        if (cur->gc_mark) continue;
        cur->gc_mark = 1;
        g_gc_marked++;
        switch (cur->type) {
            case LX_LIST: {
                LXValue* items = cur->as.list.items;
                for (int i = 0; i < cur->as.list.len; i++) {
                    if (lx_value_is_obj(items[i]) && items[i].as.obj) PUSH_OBJ(items[i].as.obj);
                }
                break;
            }
            case LX_DICT: {
                LXValue* vals = cur->as.dict.vals;
                for (int i = 0; i < cur->as.dict.len; i++) {
                    if (lx_value_is_obj(vals[i]) && vals[i].as.obj) PUSH_OBJ(vals[i].as.obj);
                }
                break;
            }
            case LX_STRUCT: {
                LXValue* fvals = cur->as.struct_inst.fvals;
                for (int i = 0; i < cur->as.struct_inst.nfields; i++) {
                    if (lx_value_is_obj(fvals[i]) && fvals[i].as.obj) PUSH_OBJ(fvals[i].as.obj);
                }
                break;
            }
            case LX_TUPLE: {
                LXValue* items = cur->as.tuple.items;
                for (int i = 0; i < cur->as.tuple.len; i++) {
                    if (lx_value_is_obj(items[i]) && items[i].as.obj) PUSH_OBJ(items[i].as.obj);
                }
                break;
            }
            case LX_CHAN: {
                // GC 仅在 active_threads==0 时执行，此时无并发访问，安全
                LXValue* buf = cur->as.chan.buf;
                int phys = cur->as.chan.cap > 0 ? cur->as.chan.cap : 1;
                for (int i = 0; i < phys; i++) {
                    if (lx_value_is_obj(buf[i]) && buf[i].as.obj) PUSH_OBJ(buf[i].as.obj);
                }
                break;
            }
            default: break;  // STR / FUNC / NATIVE / ENUM 无子对象
        }
    }
    free(stack);
#undef PUSH_OBJ
}

// 当前线程栈保守扫描：把栈上"看起来像对象地址"的 word 标记为根
static void gc_scan_stack(GCHash* set) {
    pthread_attr_t attr;
    void* stackaddr = NULL;
    size_t stacksize = 0;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        pthread_attr_getstack(&attr, &stackaddr, &stacksize);
        pthread_attr_destroy(&attr);
    }
    if (!stackaddr || stacksize == 0) return;
    volatile char probe = 0;
    uintptr_t cur = (uintptr_t)&probe;          // 当前栈位置（x86_64 栈向下增长）
    uintptr_t end = (uintptr_t)stackaddr + stacksize;  // 栈底（高地址）
    uintptr_t start = cur & ~(uintptr_t)7;
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
#include <ucontext.h>

// 主回收入口：mark + sweep
void lx_gc_collect(void) {
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    if (g_active_threads > 0) {  // 并发活跃：降级不回收
        g_gc_skips++;
        pthread_mutex_unlock(&g_gc_mu);
        return;
    }
    if (g_obj_count == 0) {
        pthread_mutex_unlock(&g_gc_mu);
        return;
    }
    GCHash set;
    gc_hash_init(&set, (size_t)g_obj_count * 2);
    for (int i = 0; i < g_obj_count; i++) gc_hash_insert(&set, (uintptr_t)g_objs[i]);
    // 调试：标记统计
    g_gc_marked = 0;
    // 根 1：全局表
    for (int i = 0; i < g_len; i++) {
        if (lx_value_is_obj(g_vals[i]) && g_vals[i].as.obj) gc_mark_obj(&set, g_vals[i].as.obj);
    }
    // 根 2：暂存根（刚创建、尚未被引用的对象）
    if (g_tmp_root) gc_mark_obj(&set, g_tmp_root);
    // 根 3：当前线程栈保守扫描（getcontext 把全部寄存器写入栈上 ucontext，一并扫描）
    ucontext_t uc;
    getcontext(&uc);
    gc_scan_stack(&set);
    // sweep
    int freed = 0, w = 0;
    for (int i = 0; i < g_obj_count; i++) {
        LXObject* o = g_objs[i];
        if (o->gc_mark) {
            o->gc_mark = 0;
            g_objs[w++] = o;
        } else {
            lx_obj_free(o);
            freed++;
        }
    }
    g_obj_count = w;
    g_alloc_bytes = 0;
    g_gc_freed += freed;
    g_gc_runs++;
    g_tmp_root = NULL;
    // 自动退避：若存活对象仍超阈值，提高阈值避免频繁 GC
    if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
    gc_debug("collect #%d: 标记 %lld/%d 回收 %d 存活 %d 跳过 %d", g_gc_runs, g_gc_marked, g_obj_count + freed, freed, g_obj_count, g_gc_skips);
    gc_hash_free(&set);
    pthread_mutex_unlock(&g_gc_mu);
}

// 注册对象（构造时调用）。est = 估算占用字节（触发字节阈值用，当前主用对象数阈值）。
static void gc_register(LXObject* o, long long est) {
    pthread_mutex_lock(&g_gc_mu);
    if (!g_gc_env_inited) gc_init_env();
    o->gc_mark = 0;   // 关键：xmalloc 未清零，gc_mark 垃圾值=1 会导致 DFS 跳过该节点（子对象漏标）
    if (g_obj_count >= g_obj_cap) {
        int ncap = g_obj_cap ? g_obj_cap * 2 : 8192;
        g_objs = realloc(g_objs, sizeof(LXObject*) * ncap);
        g_obj_cap = ncap;
    }
    g_objs[g_obj_count++] = o;
    g_alloc_bytes += est;
    g_tmp_root = o;  // 保护刚创建对象
    int need = (g_obj_count >= g_gc_threshold) ||
               (g_gc_trigger_bytes && g_alloc_bytes >= g_gc_trigger_bytes);
    pthread_mutex_unlock(&g_gc_mu);
    if (need) lx_gc_collect();
}

int lx_gc_stats(int* live, int* total) {
    pthread_mutex_lock(&g_gc_mu);
    if (live) *live = g_obj_count;
    if (total) *total = g_gc_freed;
    int runs = g_gc_runs;
    pthread_mutex_unlock(&g_gc_mu);
    return runs;
}

// 调试辅助：对象是否仍注册在对象表（未回收）
int lx_gc_contains(LXObject* o) {
    pthread_mutex_lock(&g_gc_mu);
    int found = 0;
    for (int i = 0; i < g_obj_count; i++) {
        if (g_objs[i] == o) { found = 1; break; }
    }
    pthread_mutex_unlock(&g_gc_mu);
    return found;
}

// ==================== 值构造 ====================

LXValue lx_null(void) { LXValue v; v.type = LX_NULL; v.as.i = 0; return v; }
LXValue lx_bool(bool b) { LXValue v; v.type = LX_BOOL; v.as.b = b; return v; }
LXValue lx_int(int64_t i) { LXValue v; v.type = LX_INT; v.as.i = i; return v; }
LXValue lx_float(double f) { LXValue v; v.type = LX_FLOAT; v.as.f = f; return v; }

LXValue lx_str_len(const char* s, int len) {
    LXValue v; v.type = LX_STR;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_STR;
    char* d = xmalloc(len + 1);
    memcpy(d, s, len); d[len] = 0;
    o->as.str.data = d; o->as.str.len = len;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + len + 1);
    return v;
}

LXValue lx_str(const char* s) { return lx_str_len(s, (int)strlen(s)); }

LXValue lx_list(int cap) {
    LXValue v; v.type = LX_LIST;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_LIST;
    o->as.list.items = xmalloc(sizeof(LXValue) * (cap > 0 ? cap : 8));
    o->as.list.len = 0; o->as.list.cap = cap > 0 ? cap : 8;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + (size_t)o->as.list.cap * sizeof(LXValue));
    return v;
}

LXValue lx_list_n(LXValue* items, int n) {
    LXValue v = lx_list(n);
    for (int i = 0; i < n; i++) lx_list_push(v, items[i]);
    return v;
}

LXValue lx_dict(void) {
    LXValue v; v.type = LX_DICT;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_DICT;
    o->as.dict.keys = xmalloc(sizeof(char*) * 8);
    o->as.dict.vals = xmalloc(sizeof(LXValue) * 8);
    o->as.dict.len = 0; o->as.dict.cap = 8;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + 8 * (sizeof(char*) + sizeof(LXValue)));
    return v;
}

LXValue lx_func(const char* name, LXFuncPtr fn, void* ctx) {
    LXValue v; v.type = LX_FUNC;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_FUNC;
    o->as.func.name = xstrdup(name); o->as.func.fn = fn; o->as.func.ctx = ctx;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + strlen(name) + 1);
    return v;
}

LXValue lx_native(const char* name, LXFuncPtr fn) {
    LXValue v; v.type = LX_NATIVE;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_NATIVE;
    o->as.native.name = xstrdup(name); o->as.native.fn = fn;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + strlen(name) + 1);
    return v;
}

LXValue lx_struct(const char* type_name, char** fnames, LXValue* fvals, int nfields) {
    LXValue v; v.type = LX_STRUCT;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_STRUCT;
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

LXValue lx_enum(const char* type_name, const char* variant) {
    LXValue v; v.type = LX_ENUM;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_ENUM;
    o->as.enum_inst.type_name = xstrdup(type_name);
    o->as.enum_inst.variant = xstrdup(variant);
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + strlen(type_name) + strlen(variant) + 2);
    return v;
}

LXValue lx_tuple(LXValue* items, int len) {
    LXValue v; v.type = LX_TUPLE;
    LXObject* o = xmalloc(sizeof(LXObject));
    o->type = LX_TUPLE;
    o->as.tuple.items = xmalloc(sizeof(LXValue) * (len ? len : 1));
    for (int i = 0; i < len; i++) o->as.tuple.items[i] = items[i];
    o->as.tuple.len = len;
    v.as.obj = o;
    gc_register(o, sizeof(LXObject) + (size_t)(len ? len : 1) * sizeof(LXValue));
    return v;
}

// ==================== 类型判断 ====================

bool lx_is_null(LXValue v) { return v.type == LX_NULL; }

bool lx_is_truthy(LXValue v) {
    switch (v.type) {
        case LX_NULL: return false;
        case LX_BOOL: return v.as.b;
        case LX_INT: return v.as.i != 0;
        case LX_FLOAT: return v.as.f != 0.0;
        case LX_STR: return v.as.obj->as.str.len > 0;
        case LX_LIST: return v.as.obj->as.list.len > 0;
        case LX_DICT: return v.as.obj->as.dict.len > 0;
        default: return true;
    }
}

const char* lx_type_name(LXValue v) {
    switch (v.type) {
        case LX_NULL: return "null";
        case LX_BOOL: return "bool";
        case LX_INT: return "int";
        case LX_FLOAT: return "float";
        case LX_STR: return "string";
        case LX_LIST: return "list";
        case LX_DICT: return "dict";
        case LX_FUNC: return "function";
        case LX_NATIVE: return "native";
        case LX_STRUCT: return "struct";
        case LX_ENUM: return "enum";
        case LX_TUPLE: return "tuple";
    }
    return "unknown";
}

// ==================== 错误 ====================

void lx_error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "运行时错误: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

// ==================== 字符串工具 ====================

int lx_unicode_len(const char* s) {
    int n = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
        if ((*p & 0xC0) != 0x80) n++;  // 非连续字节 = 新字符
    }
    return n;
}

// 简单数字转字符串（int/float）
static char num_buf[64];
static const char* fmt_num(LXValue v) {
    if (v.type == LX_INT) {
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

void lx_print_value(LXValue v, bool newline) {
    switch (v.type) {
        case LX_NULL: printf("null"); break;
        case LX_BOOL: printf(v.as.b ? "true" : "false"); break;
        case LX_INT: printf("%lld", (long long)v.as.i); break;
        case LX_FLOAT: printf("%g", v.as.f); break;
        case LX_STR: fwrite(v.as.obj->as.str.data, 1, v.as.obj->as.str.len, stdout); break;
        case LX_LIST: {
            printf("[");
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.list.len; i++) {
                if (i) printf(", ");
                lx_print_value(o->as.list.items[i], false);
            }
            printf("]");
            break;
        }
        case LX_TUPLE: {
            printf("(");
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.tuple.len; i++) {
                if (i) printf(", ");
                lx_print_value(o->as.tuple.items[i], false);
            }
            printf(")");
            break;
        }
        case LX_DICT: {
            printf("{");
            LXObject* o = v.as.obj;
            for (int i = 0; i < o->as.dict.len; i++) {
                if (i) printf(", ");
                printf("\"%s\": ", o->as.dict.keys[i]);
                lx_print_value(o->as.dict.vals[i], false);
            }
            printf("}");
            break;
        }
        case LX_FUNC: printf("<fn %s>", v.as.obj->as.func.name); break;
        case LX_NATIVE: printf("<native %s>", v.as.obj->as.native.name); break;
        case LX_STRUCT: {
            LXObject* o = v.as.obj;
            printf("%s(", o->as.struct_inst.type_name);
            for (int i = 0; i < o->as.struct_inst.nfields; i++) {
                if (i) printf(", ");
                printf("%s=", o->as.struct_inst.fnames[i]);
                lx_print_value(o->as.struct_inst.fvals[i], false);
            }
            printf(")");
            break;
        }
        case LX_ENUM: printf("%s.%s", v.as.obj->as.enum_inst.type_name, v.as.obj->as.enum_inst.variant); break;
        default: printf("?"); break;
    }
    if (newline) printf("\n");
}

char* lx_to_string(LXValue v) {
    static char* buf = NULL;
    static int cap = 0;
    // 简化：针对 int/float 直接用 num_buf，字符串用转义缓冲
    if (v.type == LX_INT || v.type == LX_FLOAT) return (char*)fmt_num(v);
    if (v.type == LX_BOOL) return v.as.b ? (char*)"true" : (char*)"false";
    if (v.type == LX_NULL) return (char*)"null";
    if (v.type == LX_STR) return (char*)v.as.obj->as.str.data;
    // 其他类型：写临时文件流
    FILE* tmp = tmpfile();
    if (tmp) { lx_print_value(v, false); fflush(tmp); }
    return (char*)"<object>";
}

// ==================== 运算 ====================

static double num_val(LXValue v) {
    return v.type == LX_INT ? (double)v.as.i : v.as.f;
}
static int64_t int_val(LXValue v) {
    if (v.type == LX_INT) return v.as.i;
    if (v.type == LX_FLOAT) return (int64_t)v.as.f;
    lx_error("期望整数，实际是 %s", lx_type_name(v));
    return 0;
}

LXValue lx_add(LXValue a, LXValue b) {
    if (a.type == LX_STR && b.type == LX_STR) {
        int la = a.as.obj->as.str.len, lb = b.as.obj->as.str.len;
        char* d = xmalloc(la + lb + 1);
        memcpy(d, a.as.obj->as.str.data, la);
        memcpy(d + la, b.as.obj->as.str.data, lb);
        d[la + lb] = 0;
        return lx_str_len(d, la + lb);
    }
    if (a.type == LX_INT && b.type == LX_INT) return lx_int(a.as.i + b.as.i);
    if (a.type == LX_FLOAT || b.type == LX_FLOAT) return lx_float(num_val(a) + num_val(b));
    if (a.type == LX_LIST && b.type == LX_LIST) {
        LXValue r = lx_list(a.as.obj->as.list.len + b.as.obj->as.list.len);
        LXObject* ro = r.as.obj; LXObject* ao = a.as.obj; LXObject* bo = b.as.obj;
        for (int i = 0; i < ao->as.list.len; i++) lx_list_push(r, ao->as.list.items[i]);
        for (int i = 0; i < bo->as.list.len; i++) lx_list_push(r, bo->as.list.items[i]);
        (void)ro;
        return r;
    }
    lx_error("无法相加: %s + %s", lx_type_name(a), lx_type_name(b));
    return lx_null();
}

LXValue lx_sub(LXValue a, LXValue b) {
    if (a.type == LX_INT && b.type == LX_INT) return lx_int(a.as.i - b.as.i);
    if (a.type == LX_FLOAT || b.type == LX_FLOAT) return lx_float(num_val(a) - num_val(b));
    lx_error("无法相减: %s - %s", lx_type_name(a), lx_type_name(b));
    return lx_null();
}

LXValue lx_mul(LXValue a, LXValue b) {
    if (a.type == LX_INT && b.type == LX_INT) return lx_int(a.as.i * b.as.i);
    if (a.type == LX_FLOAT || b.type == LX_FLOAT) return lx_float(num_val(a) * num_val(b));
    if (a.type == LX_STR && b.type == LX_INT) {
        int n = (int)b.as.i;
        int len = a.as.obj->as.str.len;
        char* d = xmalloc(len * n + 1);
        for (int i = 0; i < n; i++) memcpy(d + i * len, a.as.obj->as.str.data, len);
        d[len * n] = 0;
        return lx_str_len(d, len * n);
    }
    lx_error("无法相乘: %s * %s", lx_type_name(a), lx_type_name(b));
    return lx_null();
}

LXValue lx_div(LXValue a, LXValue b) {
    double d = num_val(b);
    if (d == 0) lx_error("除零错误");
    return lx_float(num_val(a) / d);
}

LXValue lx_idiv(LXValue a, LXValue b) {
    int64_t d = int_val(b);
    if (d == 0) lx_error("除零错误");
    return lx_int(int_val(a) / d);
}

LXValue lx_mod(LXValue a, LXValue b) {
    int64_t d = int_val(b);
    if (d == 0) lx_error("取模除零错误");
    if (a.type == LX_FLOAT || b.type == LX_FLOAT) return lx_float(fmod(num_val(a), num_val(b)));
    return lx_int(int_val(a) % d);
}

LXValue lx_pow(LXValue a, LXValue b) {
    if (a.type == LX_INT && b.type == LX_INT && b.as.i >= 0) {
        int64_t r = 1;
        for (int64_t i = 0; i < b.as.i; i++) r *= a.as.i;
        return lx_int(r);
    }
    return lx_float(pow(num_val(a), num_val(b)));
}

LXValue lx_neg(LXValue a) {
    if (a.type == LX_INT) return lx_int(-a.as.i);
    if (a.type == LX_FLOAT) return lx_float(-a.as.f);
    lx_error("无法取负: -%s", lx_type_name(a));
    return lx_null();
}

LXValue lx_not(LXValue a) { return lx_bool(!lx_is_truthy(a)); }
LXValue lx_bitnot(LXValue a) { return lx_int(~int_val(a)); }
LXValue lx_bitand(LXValue a, LXValue b) { return lx_int(int_val(a) & int_val(b)); }
LXValue lx_bitor(LXValue a, LXValue b) { return lx_int(int_val(a) | int_val(b)); }
LXValue lx_bitxor(LXValue a, LXValue b) { return lx_int(int_val(a) ^ int_val(b)); }
LXValue lx_shl(LXValue a, LXValue b) { return lx_int(int_val(a) << int_val(b)); }
LXValue lx_shr(LXValue a, LXValue b) { return lx_int(int_val(a) >> int_val(b)); }

static int compare_values(LXValue a, LXValue b) {
    if (a.type == LX_INT && b.type == LX_INT) {
        return a.as.i < b.as.i ? -1 : (a.as.i > b.as.i ? 1 : 0);
    }
    if ((a.type == LX_INT || a.type == LX_FLOAT) && (b.type == LX_INT || b.type == LX_FLOAT)) {
        double x = num_val(a), y = num_val(b);
        return x < y ? -1 : (x > y ? 1 : 0);
    }
    if (a.type == LX_STR && b.type == LX_STR) {
        int la = a.as.obj->as.str.len, lb = b.as.obj->as.str.len;
        int m = la < lb ? la : lb;
        int c = memcmp(a.as.obj->as.str.data, b.as.obj->as.str.data, m);
        if (c != 0) return c < 0 ? -1 : 1;
        return la < lb ? -1 : (la > lb ? 1 : 0);
    }
    if (a.type == LX_BOOL && b.type == LX_BOOL) {
        return a.as.b == b.as.b ? 0 : (a.as.b ? 1 : -1);
    }
    if (a.type == LX_ENUM && b.type == LX_ENUM) {
        return strcmp(a.as.obj->as.enum_inst.variant, b.as.obj->as.enum_inst.variant);
    }
    if ((a.type == LX_LIST || a.type == LX_TUPLE) && (b.type == LX_LIST || b.type == LX_TUPLE)) {
        int na = (a.type == LX_LIST) ? a.as.obj->as.list.len : a.as.obj->as.tuple.len;
        int nb = (b.type == LX_LIST) ? b.as.obj->as.list.len : b.as.obj->as.tuple.len;
        int m = na < nb ? na : nb;
        for (int i = 0; i < m; i++) {
            LXValue x = (a.type == LX_LIST) ? a.as.obj->as.list.items[i] : a.as.obj->as.tuple.items[i];
            LXValue y = (b.type == LX_LIST) ? b.as.obj->as.list.items[i] : b.as.obj->as.tuple.items[i];
            int c = compare_values(x, y);
            if (c != 0) return c;
        }
        return na < nb ? -1 : (na > nb ? 1 : 0);
    }
    if (a.type == LX_NULL && b.type == LX_NULL) return 0;
    // 默认按类型名比较，保证可比性
    return strcmp(lx_type_name(a), lx_type_name(b));
}

LXValue lx_eq(LXValue a, LXValue b) {
    // 数值跨类型相等：1 == 1.0
    if ((a.type == LX_INT || a.type == LX_FLOAT) && (b.type == LX_INT || b.type == LX_FLOAT))
        return lx_bool(num_val(a) == num_val(b));
    return lx_bool(compare_values(a, b) == 0);
}
LXValue lx_ne(LXValue a, LXValue b) { return lx_bool(compare_values(a, b) != 0); }
LXValue lx_lt(LXValue a, LXValue b) { return lx_bool(compare_values(a, b) < 0); }
LXValue lx_le(LXValue a, LXValue b) { return lx_bool(compare_values(a, b) <= 0); }
LXValue lx_gt(LXValue a, LXValue b) { return lx_bool(compare_values(a, b) > 0); }
LXValue lx_ge(LXValue a, LXValue b) { return lx_bool(compare_values(a, b) >= 0); }

LXValue lx_and(LXValue a, LXValue b) {
    return lx_is_truthy(a) ? b : a;  // 短路由 codegen 保证
}
LXValue lx_or(LXValue a, LXValue b) {
    return lx_is_truthy(a) ? a : b;
}

// ==================== 容器操作 ====================

LXValue lx_index(LXValue obj, LXValue idx) {
    if (obj.type == LX_LIST) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.list.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) lx_error("列表索引越界: %d (len=%d)", i, len);
        return obj.as.obj->as.list.items[i];
    }
    if (obj.type == LX_TUPLE) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.tuple.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) lx_error("元组索引越界: %d", i);
        return obj.as.obj->as.tuple.items[i];
    }
    if (obj.type == LX_STR) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.str.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) lx_error("字符串索引越界: %d", i);
        // 返回单字符（按字节；中文需要字节切片，MVP 简化）
        char buf[8] = {0};
        buf[0] = obj.as.obj->as.str.data[i];
        return lx_str(buf);
    }
    if (obj.type == LX_DICT) {
        if (idx.type == LX_STR) {
            return lx_dict_get(obj, idx.as.obj->as.str.data);
        }
        lx_error("字典索引需要字符串键");
    }
    lx_error("无法索引: %s", lx_type_name(obj));
    return lx_null();
}

void lx_index_set(LXValue obj, LXValue idx, LXValue val) {
    if (obj.type == LX_LIST) {
        int i = (int)int_val(idx);
        int len = obj.as.obj->as.list.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) lx_error("列表索引越界: %d", i);
        obj.as.obj->as.list.items[i] = val;
        return;
    }
    if (obj.type == LX_DICT) {
        if (idx.type == LX_STR) {
            lx_dict_set(obj, idx.as.obj->as.str.data, val);
            return;
        }
        lx_error("字典索引需要字符串键");
    }
    lx_error("无法索引赋值: %s", lx_type_name(obj));
}

LXValue lx_field(LXValue obj, const char* name) {
    if (obj.type == LX_STRUCT) {
        LXObject* o = obj.as.obj;
        for (int i = 0; i < o->as.struct_inst.nfields; i++) {
            if (strcmp(o->as.struct_inst.fnames[i], name) == 0) return o->as.struct_inst.fvals[i];
        }
        lx_error("结构体 %s 没有字段 %s", o->as.struct_inst.type_name, name);
    }
    if (obj.type == LX_DICT) return lx_dict_get(obj, name);
    lx_error("无法取字段: %s.%s", lx_type_name(obj), name);
    return lx_null();
}

void lx_field_set(LXValue obj, const char* name, LXValue val) {
    if (obj.type == LX_STRUCT) {
        LXObject* o = obj.as.obj;
        for (int i = 0; i < o->as.struct_inst.nfields; i++) {
            if (strcmp(o->as.struct_inst.fnames[i], name) == 0) {
                o->as.struct_inst.fvals[i] = val;
                return;
            }
        }
        lx_error("结构体 %s 没有字段 %s", o->as.struct_inst.type_name, name);
    }
    lx_error("无法字段赋值: %s.%s", lx_type_name(obj), name);
}

void lx_list_push(LXValue list, LXValue val) {
    LXObject* o = list.as.obj;
    if (o->as.list.len >= o->as.list.cap) {
        o->as.list.cap *= 2;
        o->as.list.items = realloc(o->as.list.items, sizeof(LXValue) * o->as.list.cap);
    }
    o->as.list.items[o->as.list.len++] = val;
}

void lx_dict_set(LXValue dict, const char* key, LXValue val) {
    LXObject* o = dict.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], key) == 0) {
            o->as.dict.vals[i] = val;
            return;
        }
    }
    if (o->as.dict.len >= o->as.dict.cap) {
        o->as.dict.cap *= 2;
        o->as.dict.keys = realloc(o->as.dict.keys, sizeof(char*) * o->as.dict.cap);
        o->as.dict.vals = realloc(o->as.dict.vals, sizeof(LXValue) * o->as.dict.cap);
    }
    o->as.dict.keys[o->as.dict.len] = xstrdup(key);
    o->as.dict.vals[o->as.dict.len] = val;
    o->as.dict.len++;
}

LXValue lx_dict_get(LXValue dict, const char* key) {
    LXObject* o = dict.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], key) == 0) return o->as.dict.vals[i];
    }
    return lx_null();
}

bool lx_dict_has(LXValue dict, const char* key) {
    LXObject* o = dict.as.obj;
    for (int i = 0; i < o->as.dict.len; i++) {
        if (strcmp(o->as.dict.keys[i], key) == 0) return true;
    }
    return false;
}

int lx_len(LXValue v) {
    switch (v.type) {
        case LX_STR: return lx_unicode_len(v.as.obj->as.str.data);
        case LX_LIST: return v.as.obj->as.list.len;
        case LX_DICT: return v.as.obj->as.dict.len;
        case LX_TUPLE: return v.as.obj->as.tuple.len;
        default: lx_error("len 不支持类型 %s", lx_type_name(v)); return 0;
    }
}

// ==================== 调用 ====================

LXValue lx_call(LXValue fn, LXValue* args, int nargs) {
    if (fn.type == LX_FUNC) return fn.as.obj->as.func.fn(args, nargs, fn.as.obj->as.func.ctx);
    if (fn.type == LX_NATIVE) return fn.as.obj->as.native.fn(args, nargs, NULL);
    lx_error("无法调用非函数: %s", lx_type_name(fn));
    return lx_null();
}

// 以 self 为第一参数调用全局函数（字符串方法转发）
static LXValue call_with_self(const char* fn, LXValue self, LXValue* args, int nargs) {
    LXValue* a = xmalloc(sizeof(LXValue) * (nargs + 1));
    a[0] = self;
    for (int i = 0; i < nargs; i++) a[i+1] = args[i];
    LXValue r = lx_call(lx_get_global(fn), a, nargs + 1);
    free(a);
    return r;
}

LXValue lx_method(LXValue obj, const char* name, LXValue* args, int nargs) {
    // 通道方法
    if (obj.type == LX_CHAN) {
        if (strcmp(name, "send") == 0) {
            if (nargs != 1) lx_error("send 需要 1 个参数");
            return lx_chan_send(obj, args[0]);
        }
        if (strcmp(name, "recv") == 0) return lx_chan_recv(obj);
        if (strcmp(name, "close") == 0) { lx_chan_close(obj); return lx_null(); }
    }
    // 字符串方法
    if (obj.type == LX_STR) {
        if (strcmp(name, "upper") == 0 || strcmp(name, "to_upper") == 0) {
            return call_with_self("to_upper", obj, args, nargs);
        }
        if (strcmp(name, "lower") == 0 || strcmp(name, "to_lower") == 0) {
            return call_with_self("to_lower", obj, args, nargs);
        }
        if (strcmp(name, "len") == 0) return lx_int(lx_len(obj));
        if (strcmp(name, "trim") == 0) return call_with_self("trim", obj, args, nargs);
        if (strcmp(name, "split") == 0) return call_with_self("split", obj, args, nargs);
        if (strcmp(name, "contains") == 0) return call_with_self("contains", obj, args, nargs);
        if (strcmp(name, "replace") == 0) return call_with_self("replace", obj, args, nargs);
        if (strcmp(name, "starts_with") == 0) return call_with_self("starts_with", obj, args, nargs);
        if (strcmp(name, "ends_with") == 0) return call_with_self("ends_with", obj, args, nargs);
    }
    if (obj.type == LX_LIST) {
        if (strcmp(name, "append") == 0) {
            if (nargs != 1) lx_error("append 需要 1 个参数");
            lx_list_push(obj, args[0]);
            return lx_null();
        }
        if (strcmp(name, "len") == 0) return lx_int(lx_len(obj));
        if (strcmp(name, "push") == 0) { lx_list_push(obj, args[0]); return lx_null(); }
        if (strcmp(name, "contains") == 0) {
            if (nargs < 1) lx_error("contains 需要 1 个参数");
            LXObject* o = obj.as.obj;
            for (int i = 0; i < o->as.list.len; i++) {
                if (lx_eq(o->as.list.items[i], args[0]).as.b) return lx_bool(true);
            }
            return lx_bool(false);
        }
        if (strcmp(name, "join") == 0) return call_with_self("join", args[0], &obj, 1);
    }
    if (obj.type == LX_DICT) {
        if (strcmp(name, "get") == 0) {
            if (nargs < 1) lx_error("get 需要 1 个参数");
            return lx_dict_get(obj, args[0].as.obj->as.str.data);
        }
        if (strcmp(name, "set") == 0) {
            if (nargs < 2) lx_error("set 需要 2 个参数");
            lx_dict_set(obj, args[0].as.obj->as.str.data, args[1]);
            return lx_null();
        }
        if (strcmp(name, "len") == 0) return lx_int(lx_len(obj));
        if (strcmp(name, "has") == 0 || strcmp(name, "contains") == 0) {
            if (nargs < 1) lx_error("has 需要 1 个参数");
            return lx_bool(lx_dict_has(obj, args[0].as.obj->as.str.data));
        }
        if (strcmp(name, "keys") == 0) {
            LXObject* o = obj.as.obj;
            LXValue r = lx_list(0);
            for (int i = 0; i < o->as.dict.len; i++) lx_list_push(r, lx_str(o->as.dict.keys[i]));
            return r;
        }
        if (strcmp(name, "remove") == 0) {
            if (nargs < 1) lx_error("remove 需要 1 个参数");
            LXValue v = lx_dict_get(obj, args[0].as.obj->as.str.data);
            lx_dict_set(obj, args[0].as.obj->as.str.data, lx_null()); // 简化：置 null 表示删除
            return v;
        }
    }
    lx_error("对象 %s 没有方法 %s", lx_type_name(obj), name);
    return lx_null();
}

// ==================== 全局表 ====================

LXValue lx_get_global(const char* name) {
    for (int i = 0; i < g_len; i++) {
        if (strcmp(g_keys[i], name) == 0) return g_vals[i];
    }
    lx_error("未定义变量: %s", name);
    return lx_null();
}

void lx_set_global(const char* name, LXValue v) {
    for (int i = 0; i < g_len; i++) {
        if (strcmp(g_keys[i], name) == 0) { g_vals[i] = v; return; }
    }
    if (g_len >= GLOBAL_CAP) lx_error("全局表溢出");
    g_keys[g_len] = xstrdup(name);
    g_vals[g_len] = v;
    g_len++;
}

// ==================== 内置函数 ====================

static LXValue bi_print(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    for (int i = 0; i < nargs; i++) {
        if (i) printf(" ");
        lx_print_value(args[i], false);
    }
    printf("\n");
    return lx_null();
}

static LXValue bi_len(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("len 需要一个参数");
    return lx_int(lx_len(args[0]));
}

static LXValue bi_range(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    int64_t start = 0, end = 0, step = 1;
    if (nargs == 1) { end = int_val(args[0]); }
    else if (nargs == 2) { start = int_val(args[0]); end = int_val(args[1]); }
    else if (nargs == 3) { start = int_val(args[0]); end = int_val(args[1]); step = int_val(args[2]); }
    else lx_error("range 需要 1-3 个参数");
    if (step == 0) lx_error("range step 不能为 0");
    LXValue r = lx_list(0);
    if (step > 0) for (int64_t i = start; i < end; i += step) lx_list_push(r, lx_int(i));
    else for (int64_t i = start; i > end; i += step) lx_list_push(r, lx_int(i));
    return r;
}

static LXValue bi_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("type 需要一个参数");
    return lx_str(lx_type_name(args[0]));
}

static LXValue bi_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("str 需要一个参数");
    // 字符串原样返回，数字转字符串
    if (args[0].type == LX_STR) return args[0];
    if (args[0].type == LX_INT || args[0].type == LX_FLOAT) return lx_str(fmt_num(args[0]));
    if (args[0].type == LX_BOOL) return lx_str(args[0].as.b ? "true" : "false");
    if (args[0].type == LX_NULL) return lx_str("null");
    lx_error("str 不支持类型 %s", lx_type_name(args[0]));
    return lx_null();
}

static LXValue bi_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("int 需要一个参数");
    LXValue a = args[0];
    if (a.type == LX_INT) return a;
    if (a.type == LX_FLOAT) return lx_int((int64_t)a.as.f);
    if (a.type == LX_BOOL) return lx_int(a.as.b ? 1 : 0);
    if (a.type == LX_STR) { return lx_int(atoll(a.as.obj->as.str.data)); }
    lx_error("int 不支持类型 %s", lx_type_name(a));
    return lx_null();
}

static LXValue bi_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("float 需要一个参数");
    LXValue a = args[0];
    if (a.type == LX_FLOAT) return a;
    if (a.type == LX_INT) return lx_float((double)a.as.i);
    if (a.type == LX_STR) return lx_float(atof(a.as.obj->as.str.data));
    lx_error("float 不支持类型 %s", lx_type_name(a));
    return lx_null();
}

static LXValue bi_bool(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("bool 需要一个参数");
    return lx_bool(lx_is_truthy(args[0]));
}

static LXValue bi_assert(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) lx_error("assert 需要 1-2 个参数");
    if (!lx_is_truthy(args[0])) {
        if (nargs >= 2 && args[1].type == LX_STR) lx_error("断言失败: %s", args[1].as.obj->as.str.data);
        else lx_error("断言失败");
    }
    return lx_null();
}

static LXValue bi_panic(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs >= 1 && args[0].type == LX_STR) lx_error("%s", args[0].as.obj->as.str.data);
    lx_error("panic");
    return lx_null();
}

static LXValue bi_sleep(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) lx_error("sleep 需要 1 个参数");
    int64_t ms = int_val(args[0]);
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
    return lx_null();
}

static LXValue bi_to_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("to_upper 需要一个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    int len = (int)strlen(s);
    char* d = xmalloc(len + 1);
    for (int i = 0; i < len; i++) {
        char c = s[i];
        d[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    d[len] = 0;
    return lx_str(d);
}

static LXValue bi_to_lower(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("to_lower 需要一个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    int len = (int)strlen(s);
    char* d = xmalloc(len + 1);
    for (int i = 0; i < len; i++) {
        char c = s[i];
        d[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    }
    d[len] = 0;
    return lx_str(d);
}

static LXValue bi_trim(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("trim 需要一个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\n' || s[len-1] == '\r')) len--;
    return lx_str_len(s, len);
}

static LXValue bi_now_ms(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return lx_int((int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000L);
}

static LXValue bi_abs(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("abs 需要一个参数");
    LXValue a = args[0];
    if (a.type == LX_INT) return lx_int(a.as.i < 0 ? -a.as.i : a.as.i);
    if (a.type == LX_FLOAT) return lx_float(fabs(a.as.f));
    lx_error("abs 不支持类型 %s", lx_type_name(a));
    return lx_null();
}

static LXValue bi_min(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) lx_error("min 需要至少 1 个参数");
    LXValue m = args[0];
    for (int i = 1; i < nargs; i++) {
        LXValue c = lx_lt(args[i], m);
        if (c.as.b) m = args[i];
    }
    return m;
}

static LXValue bi_max(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1) lx_error("max 需要至少 1 个参数");
    LXValue m = args[0];
    for (int i = 1; i < nargs; i++) {
        LXValue c = lx_gt(args[i], m);
        if (c.as.b) m = args[i];
    }
    return m;
}

static LXValue bi_sum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("sum 需要 1 个参数");
    LXValue v = args[0];
    if (v.type != LX_LIST) lx_error("sum 需要一个列表");
    LXObject* o = v.as.obj;
    LXValue r = lx_int(0);
    for (int i = 0; i < o->as.list.len; i++) r = lx_add(r, o->as.list.items[i]);
    return r;
}

static LXValue bi_sqrt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("sqrt 需要一个参数");
    return lx_float(sqrt(num_val(args[0])));
}

// ==================== M5 标准库内置函数 ====================

static LXValue bi_input(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs >= 1 && args[0].type == LX_STR) {
        fwrite(args[0].as.obj->as.str.data, 1, args[0].as.obj->as.str.len, stdout);
        fflush(stdout);
    }
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) return lx_str("");
    int len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) len--;
    return lx_str_len(buf, len);
}

static LXValue bi_exit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    int code = (nargs >= 1 && args[0].type == LX_INT) ? (int)args[0].as.i : 0;
    exit(code);
    return lx_null();
}

// split(s, sep) -> [str]；sep 为空按空白切分
static LXValue bi_split(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2 || args[0].type != LX_STR) lx_error("split 需要 1-2 个参数（字符串, [分隔符]）");
    const char* s = args[0].as.obj->as.str.data;
    const char* sep = (nargs >= 2 && args[1].type == LX_STR) ? args[1].as.obj->as.str.data : " ";
    int sep_len = (int)strlen(sep);
    LXValue r = lx_list(0);
    if (sep_len == 0) {
        // 按空白切分
        const char* p = s;
        while (*p) {
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            if (!*p) break;
            const char* start = p;
            while (*p && !(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
            lx_list_push(r, lx_str_len(start, (int)(p - start)));
        }
        return r;
    }
    const char* p = s;
    while (1) {
        const char* hit = strstr(p, sep);
        if (!hit) {
            lx_list_push(r, lx_str(p));
            break;
        }
        lx_list_push(r, lx_str_len(p, (int)(hit - p)));
        p = hit + sep_len;
    }
    return r;
}

// join(sep, list) -> str
static LXValue bi_join(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR) lx_error("join 需要 2 个参数（分隔符, 列表）");
    const char* sep = args[0].as.obj->as.str.data;
    int sep_len = (int)strlen(sep);
    if (args[1].type != LX_LIST && args[1].type != LX_TUPLE) lx_error("join 第二参数需要 list/tuple");
    LXObject* o = args[1].as.obj;
    int n = (args[1].type == LX_LIST) ? o->as.list.len : o->as.tuple.len;
    // 先计算总长
    size_t total = 1;
    for (int i = 0; i < n; i++) {
        LXValue item = (args[1].type == LX_LIST) ? o->as.list.items[i] : o->as.tuple.items[i];
        char* ts = lx_to_string(item);
        total += strlen(ts) + (i ? sep_len : 0);
    }
    char* out = xmalloc(total);
    out[0] = 0;
    for (int i = 0; i < n; i++) {
        if (i) strncat(out, sep, sep_len);
        LXValue item = (args[1].type == LX_LIST) ? o->as.list.items[i] : o->as.tuple.items[i];
        char* ts = lx_to_string(item);
        strcat(out, ts);
    }
    return lx_str(out);
}

// contains(容器, 元素) -> bool（字符串/列表）
static LXValue bi_contains(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) lx_error("contains 需要 2 个参数");
    if (args[0].type == LX_STR) {
        if (args[1].type != LX_STR) return lx_bool(false);
        return lx_bool(strstr(args[0].as.obj->as.str.data, args[1].as.obj->as.str.data) != NULL);
    }
    if (args[0].type == LX_LIST) {
        LXObject* o = args[0].as.obj;
        for (int i = 0; i < o->as.list.len; i++) {
            if (lx_eq(o->as.list.items[i], args[1]).as.b) return lx_bool(true);
        }
        return lx_bool(false);
    }
    lx_error("contains 不支持类型 %s", lx_type_name(args[0]));
    return lx_null();
}

// replace(s, old, new) -> str
static LXValue bi_replace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != LX_STR || args[1].type != LX_STR || args[2].type != LX_STR)
        lx_error("replace 需要 3 个字符串参数");
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
    return lx_str(out);
}

static LXValue bi_starts_with(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR || args[1].type != LX_STR) lx_error("starts_with 需要 2 个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    const char* p = args[1].as.obj->as.str.data;
    size_t lp = strlen(p);
    return lx_bool(strncmp(s, p, lp) == 0);
}

static LXValue bi_ends_with(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR || args[1].type != LX_STR) lx_error("ends_with 需要 2 个字符串参数");
    const char* s = args[0].as.obj->as.str.data;
    const char* p = args[1].as.obj->as.str.data;
    size_t ls = strlen(s), lp = strlen(p);
    if (lp > ls) return lx_bool(false);
    return lx_bool(memcmp(s + ls - lp, p, lp) == 0);
}

static LXValue bi_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) lx_error("pow 需要 2 个参数");
    if (args[0].type == LX_INT && args[1].type == LX_INT && args[1].as.i >= 0) {
        int64_t r = 1;
        for (int64_t i = 0; i < args[1].as.i; i++) r *= args[0].as.i;
        return lx_int(r);
    }
    return lx_float(pow(num_val(args[0]), num_val(args[1])));
}

// sorted(list) -> list（按 compare_values 排序，冒泡）
static LXValue bi_sorted(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_LIST) lx_error("sorted 需要一个列表");
    LXObject* o = args[0].as.obj;
    LXValue r = lx_list(o->as.list.len);
    for (int i = 0; i < o->as.list.len; i++) lx_list_push(r, o->as.list.items[i]);
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
    if (nargs != 1) lx_error("reversed 需要一个参数");
    if (args[0].type == LX_LIST) {
        LXObject* o = args[0].as.obj;
        LXValue r = lx_list(o->as.list.len);
        for (int i = o->as.list.len - 1; i >= 0; i--) lx_list_push(r, o->as.list.items[i]);
        return r;
    }
    if (args[0].type == LX_TUPLE) {
        LXObject* o = args[0].as.obj;
        LXValue r = lx_list(o->as.tuple.len);
        for (int i = o->as.tuple.len - 1; i >= 0; i--) lx_list_push(r, o->as.tuple.items[i]);
        return r;
    }
    if (args[0].type == LX_STR) {
        const char* s = args[0].as.obj->as.str.data;
        int len = (int)strlen(s);
        char* d = xmalloc(len + 1);
        for (int i = 0; i < len; i++) d[i] = s[len - 1 - i];
        d[len] = 0;
        return lx_str(d);
    }
    lx_error("reversed 不支持类型 %s", lx_type_name(args[0]));
    return lx_null();
}

// ---- std.io / std.fs ----

static LXValue bi_read_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("read_file 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    FILE* f = fopen(path, "rb");
    if (!f) lx_error("io: 读取文件失败 %s", path);
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = xmalloc(sz + 1);
    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = 0;
    fclose(f);
    return lx_str_len(buf, (int)rd);
}

static LXValue bi_write_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR) lx_error("write_file 需要 (路径, 内容)");
    const char* path = args[0].as.obj->as.str.data;
    const char* content;
    int clen;
    if (args[1].type == LX_STR) { content = args[1].as.obj->as.str.data; clen = args[1].as.obj->as.str.len; }
    else { content = lx_to_string(args[1]); clen = (int)strlen(content); }
    FILE* f = fopen(path, "wb");
    if (!f) lx_error("io: 写入文件失败 %s", path);
    fwrite(content, 1, clen, f);
    fclose(f);
    return lx_null();
}

static LXValue bi_append_file(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR) lx_error("append_file 需要 (路径, 内容)");
    const char* path = args[0].as.obj->as.str.data;
    const char* content;
    int clen;
    if (args[1].type == LX_STR) { content = args[1].as.obj->as.str.data; clen = args[1].as.obj->as.str.len; }
    else { content = lx_to_string(args[1]); clen = (int)strlen(content); }
    FILE* f = fopen(path, "ab");
    if (!f) lx_error("io: 追加写入失败 %s", path);
    fwrite(content, 1, clen, f);
    fclose(f);
    return lx_null();
}

static LXValue bi_exists(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("exists 需要一个路径参数");
    struct stat st;
    return lx_bool(stat(args[0].as.obj->as.str.data, &st) == 0);
}

static LXValue bi_list_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("list_dir 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    DIR* d = opendir(path);
    if (!d) lx_error("fs: 读取目录失败 %s", path);
    LXValue r = lx_list(0);
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        lx_list_push(r, lx_str(e->d_name));
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
    if (nargs != 1 || args[0].type != LX_STR) lx_error("mkdir 需要一个路径参数");
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
    return lx_null();
}

static LXValue bi_remove(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("remove 需要一个路径参数");
    const char* path = args[0].as.obj->as.str.data;
    if (remove(path) != 0 && rmdir(path) != 0) lx_error("fs: 删除失败 %s", path);
    return lx_null();
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
        LXValue d = lx_dict();
        json_ws(j);
        if (*j->p == '}') { j->p++; return d; }
        while (1) {
            json_ws(j);
            if (*j->p != '"') lx_error("json: 期望对象键");
            j->p++;
            char* key = json_str_raw(j);
            json_ws(j);
            if (*j->p != ':') lx_error("json: 期望 ':'");
            j->p++;
            LXValue v = json_parse_value(j);
            lx_dict_set(d, key, v);
            free(key);
            json_ws(j);
            if (*j->p == ',') { j->p++; continue; }
            if (*j->p == '}') { j->p++; break; }
            lx_error("json: 对象解析失败");
        }
        return d;
    }
    if (*j->p == '[') {
        j->p++;
        LXValue a = lx_list(0);
        json_ws(j);
        if (*j->p == ']') { j->p++; return a; }
        while (1) {
            LXValue v = json_parse_value(j);
            lx_list_push(a, v);
            json_ws(j);
            if (*j->p == ',') { j->p++; continue; }
            if (*j->p == ']') { j->p++; break; }
            lx_error("json: 数组解析失败");
        }
        return a;
    }
    if (*j->p == '"') {
        j->p++;
        char* s = json_str_raw(j);
        LXValue r = lx_str(s);
        free(s);
        return r;
    }
    if (strncmp(j->p, "true", 4) == 0) { j->p += 4; return lx_bool(true); }
    if (strncmp(j->p, "false", 5) == 0) { j->p += 5; return lx_bool(false); }
    if (strncmp(j->p, "null", 4) == 0) { j->p += 4; return lx_null(); }
    // 数字
    char* end;
    long long iv = strtoll(j->p, &end, 10);
    if (end != j->p && (*end == 0 || *end == ',' || *end == '}' || *end == ']' || *end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        j->p = end;
        return lx_int(iv);
    }
    double dv = strtod(j->p, &end);
    if (end != j->p) {
        j->p = end;
        return lx_float(dv);
    }
    lx_error("json: 无法解析");
    return lx_null();
}

static LXValue bi_json_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("json_parse 需要一个字符串参数");
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
        o->buf = realloc(o->buf, o->cap);
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
        case LX_NULL: jout_append(o, "null"); break;
        case LX_BOOL: jout_append(o, v.as.b ? "true" : "false"); break;
        case LX_INT: {
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "%lld", (long long)v.as.i);
            jout_append(o, tmp);
            break;
        }
        case LX_FLOAT: {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "%g", v.as.f);
            jout_append(o, tmp);
            break;
        }
        case LX_STR: jout_escape(o, v.as.obj->as.str.data); break;
        case LX_LIST: {
            jout_append(o, "[");
            LXObject* ob = v.as.obj;
            for (int i = 0; i < ob->as.list.len; i++) {
                if (i) jout_append(o, ",");
                json_stringify_value(o, ob->as.list.items[i]);
            }
            jout_append(o, "]");
            break;
        }
        case LX_TUPLE: {
            jout_append(o, "[");
            LXObject* ob = v.as.obj;
            for (int i = 0; i < ob->as.tuple.len; i++) {
                if (i) jout_append(o, ",");
                json_stringify_value(o, ob->as.tuple.items[i]);
            }
            jout_append(o, "]");
            break;
        }
        case LX_DICT: {
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
        default: lx_error("json: 无法序列化类型 %s", lx_type_name(v));
    }
}
static LXValue bi_json_stringify(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) lx_error("json_stringify 需要一个参数");
    JOut o = { NULL, 0, 0 };
    o.buf = xmalloc(64); o.cap = 64; o.buf[0] = 0;
    json_stringify_value(&o, args[0]);
    LXValue r = lx_str(o.buf);
    free(o.buf);
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
    return lx_str(buf);
}

// ---- std.os ----

static LXValue bi_env(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("env 需要一个变量名");
    const char* v = getenv(args[0].as.obj->as.str.data);
    return v ? lx_str(v) : lx_null();
}

static LXValue bi_args(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)nargs; (void)ctx;
    // 编译版不保留原始 argv，返回空列表（脚本版返回 args）
    return lx_list(0);
}

// ---- std.collections（高阶函数） ----

static LXValue bi_map(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_LIST) lx_error("map 需要 (list, fn)");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue r = lx_list(0);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue res = lx_call(fn, &item, 1);
        lx_list_push(r, res);
    }
    return r;
}

static LXValue bi_filter(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_LIST) lx_error("filter 需要 (list, fn)");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue r = lx_list(0);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue res = lx_call(fn, &item, 1);
        if (lx_is_truthy(res)) lx_list_push(r, item);
    }
    return r;
}

static LXValue bi_reduce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3 || args[0].type != LX_LIST) lx_error("reduce 需要 (list, fn, [init])");
    LXObject* o = args[0].as.obj;
    LXValue fn = args[1];
    LXValue acc = (nargs >= 3) ? args[2] : lx_int(0);
    for (int i = 0; i < o->as.list.len; i++) {
        LXValue item = o->as.list.items[i];
        LXValue pair[2] = { acc, item };
        acc = lx_call(fn, pair, 2);
    }
    return acc;
}

void lx_register_builtins(void) {
    lx_set_global("print", lx_native("print", bi_print));
    lx_set_global("len", lx_native("len", bi_len));
    lx_set_global("range", lx_native("range", bi_range));
    lx_set_global("type", lx_native("type", bi_type));
    lx_set_global("str", lx_native("str", bi_str));
    lx_set_global("int", lx_native("int", bi_int));
    lx_set_global("float", lx_native("float", bi_float));
    lx_set_global("bool", lx_native("bool", bi_bool));
    lx_set_global("assert", lx_native("assert", bi_assert));
    lx_set_global("panic", lx_native("panic", bi_panic));
    lx_set_global("sleep", lx_native("sleep", bi_sleep));
    lx_set_global("to_upper", lx_native("to_upper", bi_to_upper));
    lx_set_global("to_lower", lx_native("to_lower", bi_to_lower));
    lx_set_global("trim", lx_native("trim", bi_trim));
    lx_set_global("now_ms", lx_native("now_ms", bi_now_ms));
    lx_set_global("abs", lx_native("abs", bi_abs));
    lx_set_global("min", lx_native("min", bi_min));
    lx_set_global("max", lx_native("max", bi_max));
    lx_set_global("sum", lx_native("sum", bi_sum));
    lx_set_global("sqrt", lx_native("sqrt", bi_sqrt));
    // M5 标准库
    lx_set_global("input", lx_native("input", bi_input));
    lx_set_global("exit", lx_native("exit", bi_exit));
    lx_set_global("split", lx_native("split", bi_split));
    lx_set_global("join", lx_native("join", bi_join));
    lx_set_global("contains", lx_native("contains", bi_contains));
    lx_set_global("replace", lx_native("replace", bi_replace));
    lx_set_global("starts_with", lx_native("starts_with", bi_starts_with));
    lx_set_global("ends_with", lx_native("ends_with", bi_ends_with));
    lx_set_global("pow", lx_native("pow", bi_pow));
    lx_set_global("sorted", lx_native("sorted", bi_sorted));
    lx_set_global("reversed", lx_native("reversed", bi_reversed));
    lx_set_global("read_file", lx_native("read_file", bi_read_file));
    lx_set_global("write_file", lx_native("write_file", bi_write_file));
    lx_set_global("append_file", lx_native("append_file", bi_append_file));
    lx_set_global("exists", lx_native("exists", bi_exists));
    lx_set_global("list_dir", lx_native("list_dir", bi_list_dir));
    lx_set_global("mkdir", lx_native("mkdir", bi_mkdir));
    lx_set_global("remove", lx_native("remove", bi_remove));
    lx_set_global("json_parse", lx_native("json_parse", bi_json_parse));
    lx_set_global("json_stringify", lx_native("json_stringify", bi_json_stringify));
    lx_set_global("now", lx_native("now", bi_now));
    lx_set_global("env", lx_native("env", bi_env));
    lx_set_global("args", lx_native("args", bi_args));
    lx_set_global("map", lx_native("map", bi_map));
    lx_set_global("filter", lx_native("filter", bi_filter));
    lx_set_global("reduce", lx_native("reduce", bi_reduce));
    // std.net（M5.2）
    lx_set_global("tcp_listen", lx_native("tcp_listen", bi_tcp_listen));
    lx_set_global("tcp_accept", lx_native("tcp_accept", bi_tcp_accept));
    lx_set_global("tcp_connect", lx_native("tcp_connect", bi_tcp_connect));
    lx_set_global("tcp_send", lx_native("tcp_send", bi_tcp_send));
    lx_set_global("tcp_recv", lx_native("tcp_recv", bi_tcp_recv));
    lx_set_global("tcp_close", lx_native("tcp_close", bi_tcp_close));
    lx_set_global("http_get", lx_native("http_get", bi_http_get));
    lx_set_global("http_post", lx_native("http_post", bi_http_post));
}

// ==================== 并发原语（M4.2） ====================

// 全局 select 唤醒：任何 chan 操作后广播，select 循环醒来重试
static pthread_mutex_t g_sel_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_sel_cv = PTHREAD_COND_INITIALIZER;

void lx_select_signal(void) {
    pthread_mutex_lock(&g_sel_mu);
    pthread_cond_broadcast(&g_sel_cv);
    pthread_mutex_unlock(&g_sel_mu);
}

void lx_select_wait(void) {
    pthread_mutex_lock(&g_sel_mu);
    pthread_cond_wait(&g_sel_cv, &g_sel_mu);
    pthread_mutex_unlock(&g_sel_mu);
}

bool lx_is_chan(LXValue v) { return v.type == LX_CHAN; }

LXValue lx_chan_create(int cap) {
    LXObject* o = calloc(1, sizeof(LXObject));
    o->type = LX_CHAN;
    o->as.chan.cap = cap;
    o->as.chan.len = 0;
    o->as.chan.head = 0;
    o->as.chan.closed = 0;
    o->as.chan.recv_waiting = 0;
    int phys = cap > 0 ? cap : 1;  // 无缓冲也保留 1 个交付槽
    o->as.chan.buf = calloc(phys, sizeof(LXValue));
    pthread_mutex_init(&o->as.chan.mu, NULL);
    pthread_cond_init(&o->as.chan.cv_send, NULL);
    pthread_cond_init(&o->as.chan.cv_recv, NULL);
    LXValue v; v.type = LX_CHAN; v.as.obj = o;
    gc_register(o, sizeof(LXObject) + (size_t)phys * sizeof(LXValue));
    return v;
}

LXValue lx_chan_send(LXValue ch, LXValue val) {
    if (ch.type != LX_CHAN) lx_error("send: 目标不是通道（%s）", lx_type_name(ch));
    LXObject* o = ch.as.obj;
    pthread_mutex_lock(&o->as.chan.mu);
    while (1) {
        if (o->as.chan.closed) {
            pthread_mutex_unlock(&o->as.chan.mu);
            lx_error("R1011: 向已关闭的通道发送");
        }
        if (o->as.chan.cap == 0) {
            // 无缓冲：等待接收者就绪
            if (o->as.chan.recv_waiting > 0) {
                o->as.chan.buf[0] = val;
                o->as.chan.len = 1;
                pthread_cond_signal(&o->as.chan.cv_recv);
                pthread_mutex_unlock(&o->as.chan.mu);
                lx_select_signal();
                return val;
            }
            pthread_cond_wait(&o->as.chan.cv_send, &o->as.chan.mu);
        } else {
            // 有缓冲：满则等待
            if (o->as.chan.len < o->as.chan.cap) {
                int tail = (o->as.chan.head + o->as.chan.len) % o->as.chan.cap;
                o->as.chan.buf[tail] = val;
                o->as.chan.len++;
                pthread_cond_signal(&o->as.chan.cv_recv);
                pthread_mutex_unlock(&o->as.chan.mu);
                lx_select_signal();
                return val;
            }
            pthread_cond_wait(&o->as.chan.cv_send, &o->as.chan.mu);
        }
    }
}

LXValue lx_chan_recv(LXValue ch) {
    if (ch.type != LX_CHAN) lx_error("recv: 目标不是通道（%s）", lx_type_name(ch));
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
            lx_select_signal();
            return v;
        }
        if (o->as.chan.closed) {
            pthread_mutex_unlock(&o->as.chan.mu);
            lx_error("R1011: 从已关闭且为空的通道接收");
        }
        if (o->as.chan.cap == 0) o->as.chan.recv_waiting++;
        pthread_cond_wait(&o->as.chan.cv_recv, &o->as.chan.mu);
        if (o->as.chan.cap == 0 && o->as.chan.len == 0 && !o->as.chan.closed) {
            // 被唤醒但值被别的接收者取走（竞争），继续等待
        }
    }
}

bool lx_chan_try_recv(LXValue ch, LXValue* out) {
    if (ch.type != LX_CHAN) lx_error("recv: 目标不是通道（%s）", lx_type_name(ch));
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
    if (ok) lx_select_signal();
    return ok;
}

void lx_chan_close(LXValue ch) {
    if (ch.type != LX_CHAN) lx_error("close: 目标不是通道（%s）", lx_type_name(ch));
    LXObject* o = ch.as.obj;
    pthread_mutex_lock(&o->as.chan.mu);
    o->as.chan.closed = 1;
    pthread_cond_broadcast(&o->as.chan.cv_send);
    pthread_cond_broadcast(&o->as.chan.cv_recv);
    pthread_mutex_unlock(&o->as.chan.mu);
    lx_select_signal();
}

// ==================== spawn（pthread） ====================

typedef struct {
    LXFuncPtr fn;
    LXValue* args;
    int nargs;
} SpawnJob;

static void* spawn_thread(void* p) {
    SpawnJob* job = (SpawnJob*)p;
    job->fn(job->args, job->nargs, NULL);
    free(job->args);
    free(job);
    pthread_mutex_lock(&g_gc_mu);   // M8：线程退出，活跃计数减一（GC 可恢复）
    g_active_threads--;
    pthread_mutex_unlock(&g_gc_mu);
    return NULL;
}

void lx_spawn(LXFuncPtr fn, LXValue* args, int nargs) {
    pthread_mutex_lock(&g_gc_mu);   // M8：创建前先标记活跃（防止主线程 GC 误判）
    g_active_threads++;
    pthread_mutex_unlock(&g_gc_mu);
    SpawnJob* job = malloc(sizeof(SpawnJob));
    job->fn = fn;
    job->nargs = nargs;
    job->args = malloc(sizeof(LXValue) * (nargs > 0 ? nargs : 1));
    if (nargs > 0) memcpy(job->args, args, sizeof(LXValue) * nargs);
    pthread_t t;
    if (pthread_create(&t, NULL, spawn_thread, job) != 0) {
        pthread_mutex_lock(&g_gc_mu);
        g_active_threads--;
        pthread_mutex_unlock(&g_gc_mu);
        lx_error("spawn: 创建线程失败");
    }
    pthread_detach(t);
}

void lx_spawn_name(const char* fname, LXValue* args, int nargs) {
    LXValue fn = lx_get_global(fname);
    if (fn.type != LX_FUNC) lx_error("spawn: 未找到函数 %s", fname);
    lx_spawn(fn.as.obj->as.func.fn, args, nargs);
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
    if (nargs != 1 || args[0].type != LX_INT) lx_error("tcp_listen 需要 (port) 参数");
    int port = (int)args[0].as.i;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) lx_error("net: 创建 socket 失败");
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        lx_error("net: 监听端口 %d 失败", port);
    }
    if (listen(fd, 16) < 0) {
        close(fd);
        lx_error("net: listen 失败");
    }
    return lx_int(fd);
}

static LXValue bi_tcp_accept(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_INT) lx_error("tcp_accept 需要 (listener_id) 参数");
    int lfd = (int)args[0].as.i;
    struct sockaddr_in cli;
    socklen_t cli_len = sizeof(cli);
    int cfd = accept(lfd, (struct sockaddr*)&cli, &cli_len);
    if (cfd < 0) lx_error("net: accept 失败");
    return lx_int(cfd);
}

static LXValue bi_tcp_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR || args[1].type != LX_INT) lx_error("tcp_connect 需要 (host, port) 参数");
    const char* host = args[0].as.obj->as.str.data;
    int port = (int)args[1].as.i;
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res) lx_error("net: 解析主机失败 %s", host);
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) { freeaddrinfo(res); lx_error("net: 创建 socket 失败"); }
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        int e = errno;
        freeaddrinfo(res);
        close(fd);
        lx_error("net: 连接 %s:%d 失败 (%d)", host, port, e);
    }
    freeaddrinfo(res);
    return lx_int(fd);
}

static LXValue bi_tcp_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_INT) lx_error("tcp_send 需要 (conn_id, data) 参数");
    int fd = (int)args[0].as.i;
    const char* data;
    int len;
    if (args[1].type == LX_STR) { data = args[1].as.obj->as.str.data; len = args[1].as.obj->as.str.len; }
    else { data = lx_to_string(args[1]); len = (int)strlen(data); }
    int n = sock_send_all(fd, data, len);
    if (n < 0) lx_error("net: 发送失败");
    return lx_int(n);
}

static LXValue bi_tcp_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_INT || args[1].type != LX_INT) lx_error("tcp_recv 需要 (conn_id, maxlen) 参数");
    int fd = (int)args[0].as.i;
    int maxlen = (int)args[1].as.i;
    if (maxlen <= 0) maxlen = 1;
    char* buf = xmalloc(maxlen + 1);
    int n = (int)recv(fd, buf, maxlen, 0);
    if (n <= 0) { free(buf); return lx_str(""); }
    buf[n] = 0;
    LXValue r = lx_str_len(buf, n);
    free(buf);
    return r;
}

static LXValue bi_tcp_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_INT) lx_error("tcp_close 需要 (id) 参数");
    close((int)args[0].as.i);
    return lx_null();
}

// ==================== M10 HTTP / HTTPS 客户端 ====================
// 统一 http/https GET/POST：lx_http_request(url, method, body) → malloc 响应体
// 自动跟随重定向（最多 5 次）；https 走 mbedtls（静态链接，保持静态二进制）

// mbedtls 全局 CA 证书缓存（避免每次解析 CA bundle）
static pthread_mutex_t g_cacert_mu = PTHREAD_MUTEX_INITIALIZER;
static mbedtls_x509_crt g_cacert;
static int g_cacert_loaded = 0;

static void lx_ensure_cacert(void) {
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
    pthread_mutex_unlock(&g_cacert_mu);
}

// mbedtls HTTPS 请求：返回 malloc 响应（含响应头+体），*out_len 输出长度；0=成功
static int lx_https_request(const char* host, int port, const char* req, char** out, int* out_len) {
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

    lx_ensure_cacert();
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
        if (total + 4096 > cap) { cap *= 2; buf = realloc(buf, cap); }
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
static char* lx_http_once(const char* url, const char* method, const char* body,
    int* out_len, int* out_status, char* loc, int loc_cap) {
    int is_https = 0;
    const char* rest;
    if (strncmp(url, "https://", 8) == 0) { is_https = 1; rest = url + 8; }
    else if (strncmp(url, "http://", 7) == 0) { rest = url + 7; }
    else { lx_error("net: 不支持的协议: %s", url); return NULL; }

    char host[256];
    int hostlen = 0;
    while (rest[hostlen] && rest[hostlen] != '/' && rest[hostlen] != ':' && hostlen < 255) {
        host[hostlen] = rest[hostlen];
        hostlen++;
    }
    host[hostlen] = 0;
    if (hostlen == 0) { lx_error("net: 主机名为空"); return NULL; }
    int port = is_https ? 443 : 80;
    const char* p = rest + hostlen;
    if (*p == ':') {
        port = atoi(p + 1);
        if (port <= 0 || port > 65535) { lx_error("net: 端口非法"); return NULL; }
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
        int r = lx_https_request(host, port, req, &resp, &resp_len);
        if (r != 0) { lx_error("net: HTTPS 请求失败 (%d) %s", r, host); return NULL; }
    } else {
        // 明文 http
        struct addrinfo hints, *res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        char portstr[16];
        snprintf(portstr, sizeof(portstr), "%d", port);
        if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res) { lx_error("net: 解析主机失败 %s", host); return NULL; }
        int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0) { freeaddrinfo(res); lx_error("net: 创建 socket 失败"); return NULL; }
        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            freeaddrinfo(res);
            close(fd);
            lx_error("net: 连接 %s:%d 失败", host, port);
            return NULL;
        }
        freeaddrinfo(res);
        sock_send_all(fd, req, rlen);
        int cap = 4096, len = 0;
        resp = xmalloc(cap);
        for (;;) {
            if (len + 4096 > cap) { cap *= 2; resp = realloc(resp, cap); }
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
static char* lx_http_request(const char* url, const char* method, const char* body, int* out_len) {
    char cur[2048];
    snprintf(cur, sizeof(cur), "%s", url);
    for (int i = 0; i < 5; i++) {
        char loc[1024];
        int status = 0;
        int len = 0;
        char* resp = lx_http_once(cur, method, body, &len, &status, loc, sizeof(loc));
        if (status >= 300 && status < 400 && loc[0]) {
            char next[2048];
            if (strncmp(loc, "http://", 7) == 0 || strncmp(loc, "https://", 8) == 0) {
                snprintf(next, sizeof(next), "%s", loc);
            } else if (loc[0] == '/') {
                const char* s = strstr(cur, "://");
                if (!s) { free(resp); lx_error("net: 非法 URL"); return NULL; }
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
            free(resp);
            snprintf(cur, sizeof(cur), "%s", next);
            continue;
        }
        // 分离响应体
        char* body_start = resp;
        char* sep = strstr(resp, "\r\n\r\n");
        if (sep) body_start = sep + 4;
        int body_len = (int)(resp + len - body_start);
        char* r = xmalloc(body_len + 1);
        memcpy(r, body_start, body_len);
        r[body_len] = 0;
        free(resp);
        *out_len = body_len;
        return r;
    }
    lx_error("net: 重定向次数过多（>5）");
    return NULL;
}

// http_get(url) → 响应体（支持 http/https，自动跟随重定向）
static LXValue bi_http_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != LX_STR) lx_error("http_get 需要 (url) 参数");
    const char* url = args[0].as.obj->as.str.data;
    int len = 0;
    char* body = lx_http_request(url, "GET", NULL, &len);
    LXValue r = lx_str_len(body, len);
    free(body);
    return r;
}

// http_post(url, body) → 响应体（支持 http/https，自动跟随重定向）
static LXValue bi_http_post(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != LX_STR || args[1].type != LX_STR) lx_error("http_post 需要 (url, body) 参数");
    const char* url = args[0].as.obj->as.str.data;
    const char* body = args[1].as.obj->as.str.data;
    int len = 0;
    char* resp = lx_http_request(url, "POST", body, &len);
    LXValue r = lx_str_len(resp, len);
    free(resp);
    return r;
}
