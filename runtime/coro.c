// 普贤 (PuXian) C 运行时库 — coro.c
// M93-S2: 帧协程 M:N（px_spawn 用户态协程化 —— 协程内核）
// ------------------------------------------------------------
// 设计（docs/M93_PLAN.md D1-D4）：px_spawn 目标为 VM 函数（PX_FUNC.fn==px_vm_entry，
// ctx=PxVMFunc*）→ 不再每 spawn 建一个 pthread，而是建「帧协程」入就绪队列；
// M 个常驻 worker 线程（PX_CORO_WORKERS，默认 min(在线CPU,8)）轮取出队执行。
// 协程 = 独立 PxVmState（帧栈全堆上，px→px 调用不回 C 递归）→ 执行状态全部在
// 堆上，切换只换指针（px_vm_bind/unbind），零 ucontext/零汇编。
//
// S2 范围：纯计算协程（跑到结束）。worker 取协程 → bind 其 vm → px_vm_entry
// 推进 → 完成回收。阻塞原语让出（chan/mutex/sleep）在 S3 引入（本文件状态机
// 预留 CORO_BLOCKED；S2 worker 每次取协程跑到 DONE）。
//
// 并发安全设计（关键）：
//   - g_coro_mu 临界区（入队/出队/回收/GC 标记遍历）均屏蔽 SIG_GC_STOP
//     （px_gc_block_stop_sig）→ 持锁临界区不被 STW 信号打断 → GC executor
//     标记协程表拿 g_coro_mu 时，持锁线程必在推进（非被暂停自旋）→ 无死锁。
//   - GC 根面（runtime.c 标记期 weak 调 px_coro_gc_mark_roots）：全部存活协程
//     的 args 副本 + 已运行帧槽精确标记。worker 跑协程期间经 px_gc_thread_enter
//     注册 GC 槽（暂停时 ti->vm_state=协程 vm，运行中帧槽亦被覆盖标记）。
//   - worker 空闲（cond_wait 等就绪队列）不注册 GC（无 px 对象）→ 与 M90
//     F3-fix 同模式：GC STW 只暂停在岗 worker，不因空闲 worker 空转放大开销。
// ============================================================
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ---- 协程状态机（S3 阻塞让出预留 BLOCKED）----
#define CORO_READY   0
#define CORO_RUNNING 1
#define CORO_BLOCKED 2   // S3：阻塞原语让出（chan/mutex/sleep 等待登记）
#define CORO_DONE    3

typedef struct PxCoro {
    struct PxCoro* next;      // 就绪队列链（FIFO）
    struct PxCoro* all_next;  // 全局存活表链（GC 根面 + 完成回收）
    PxVmState      vm;        // 协程独立 VM 状态（帧栈全堆上）
    const PxVMFunc* f;        // 目标 VM 函数（ctx 直传 px_vm_entry）
    LXValue*       args;      // 实参副本（spawn 时复制，协程生命周期内保活）
    int            nargs;
    int            state;
    int            id;
} PxCoro;

static pthread_mutex_t g_coro_mu   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_coro_cond = PTHREAD_COND_INITIALIZER;
static PxCoro* g_rq = NULL;        // 就绪队列头（FIFO）
static PxCoro* g_rq_tail = NULL;   // 就绪队列尾
static PxCoro* g_all = NULL;       // 全局存活协程表（完成即摘除回收）
static int     g_workers_started = 0;
static int     g_worker_target = 0;
static int     g_coro_seq = 0;
static volatile int g_coro_diag = -1;   // PX_CORO_DIAG=1 诊断输出

// ---- 清理协程 vm 残留帧（异常 longjmp 跳过多帧时 slots 需逐个释放）----
static void coro_vm_free(PxVmState* st) {
    if (!st) return;
    if (st->frames) {
        for (int i = 0; i < st->nframes; i++) {
            if (st->frames[i].slots) { free(st->frames[i].slots); st->frames[i].slots = NULL; }
        }
        free(st->frames);
        st->frames = NULL;
    }
    st->nframes = 0;
    st->cap = 0;
}

// ---- worker 线程：循环取就绪协程执行 ----
static void* coro_worker(void* arg) {
    (void)arg;
    for (;;) {
        // 取队首就绪协程（空闲期 cond_wait；未注册 GC → STW 无空闲放大）
        sigset_t old;
        px_gc_block_stop_sig(&old);
        pthread_mutex_lock(&g_coro_mu);
        PxCoro* c = NULL;
        while (!g_rq) {
            pthread_cond_wait(&g_coro_cond, &g_coro_mu);
        }
        c = g_rq;
        g_rq = c->next;
        if (!g_rq) g_rq_tail = NULL;
        c->next = NULL;
        c->state = CORO_RUNNING;
        pthread_mutex_unlock(&g_coro_mu);
        px_gc_unblock_stop_sig(&old);
        if (!c) break;

        // 跑协程：注册 GC（即将持 px 对象）→ bind vm → 隔离执行 → 解绑 → 注销
        px_gc_thread_enter();
        px_vm_bind(&c->vm);
        if (px_spawn_isolate_begin()) {          // 正常：执行协程体
            px_vm_entry(c->args, c->nargs, (void*)c->f);
            px_spawn_isolate_end();
        }                                        // 错误：longjmp 回 → 协程异常终止
        px_vm_unbind();
        px_gc_thread_leave();

        // 完成 → 从全局存活表摘除 + 回收
        px_gc_block_stop_sig(&old);
        pthread_mutex_lock(&g_coro_mu);
        c->state = CORO_DONE;
        PxCoro** pp = &g_all;
        while (*pp && *pp != c) pp = &(*pp)->all_next;
        if (*pp) *pp = c->all_next;
        pthread_mutex_unlock(&g_coro_mu);
        px_gc_unblock_stop_sig(&old);
        if (g_coro_diag == 1)
            fprintf(stderr, "[px-coro] #%d done (f=%s)\n", c->id, c->f && c->f->name ? c->f->name : "?");
        free(c->args);
        coro_vm_free(&c->vm);
        free(c);
    }
    return NULL;
}

// ---- worker 池惰性启动（首个协程 spawn 时）----
static void coro_ensure_workers(void) {
    if (g_workers_started) return;
    if (g_worker_target <= 0) {
        const char* env = getenv("PX_CORO_WORKERS");
        int n = env ? atoi(env) : 0;
        if (n < 1) {
            long cpu = sysconf(_SC_NPROCESSORS_ONLN);
            n = (int)(cpu > 0 ? cpu : 2);
        }
        if (n > 64) n = 64;
        if (n < 1) n = 1;
        g_worker_target = n;
    }
    const char* diag = getenv("PX_CORO_DIAG");
    if (diag && diag[0] == '1') g_coro_diag = 1;
    pthread_mutex_lock(&g_coro_mu);
    if (g_workers_started) { pthread_mutex_unlock(&g_coro_mu); return; }
    for (int i = 0; i < g_worker_target; i++) {
        pthread_t t;
        if (pthread_create(&t, NULL, coro_worker, NULL) != 0) break;
        pthread_detach(t);
    }
    g_workers_started = 1;
    pthread_mutex_unlock(&g_coro_mu);
}

// ---- spawn 分派入口（runtime.c px_spawn_name weak 调用；ctx=PxVMFunc*）----
void px_coro_spawn(void* ctx, LXValue* args, int nargs) {
    PxCoro* c = (PxCoro*)calloc(1, sizeof(PxCoro));
    if (!c) return;
    c->f = (const PxVMFunc*)ctx;
    px_vm_state_init(&c->vm);
    c->nargs = nargs;
    c->args = (LXValue*)malloc(sizeof(LXValue) * (nargs > 0 ? nargs : 1));
    if (c->args && nargs > 0 && args)
        memcpy(c->args, args, sizeof(LXValue) * nargs);
    c->state = CORO_READY;
    if (g_coro_diag < 0) {   // 首个 spawn 前初始化诊断开关（worker 池惰性启动同读）
        const char* diag = getenv("PX_CORO_DIAG");
        g_coro_diag = (diag && diag[0] == '1') ? 1 : 0;
    }
    sigset_t old;
    px_gc_block_stop_sig(&old);
    pthread_mutex_lock(&g_coro_mu);
    c->id = ++g_coro_seq;
    int cid = c->id;
    c->all_next = g_all;                 // 挂全局存活表（GC 根面）
    g_all = c;
    c->next = NULL;                      // 就绪队列尾插（FIFO）
    if (g_rq_tail) g_rq_tail->next = c; else g_rq = c;
    g_rq_tail = c;
    pthread_mutex_unlock(&g_coro_mu);
    px_gc_unblock_stop_sig(&old);
    if (g_coro_diag == 1)
        fprintf(stderr, "[px-coro] spawn -> #%d (f=%s, nargs=%d)\n",
                cid, c->f && c->f->name ? c->f->name : "?", nargs);
    coro_ensure_workers();
    pthread_cond_signal(&g_coro_cond);
}

// ---- GC 根面：全部存活协程（runtime.c 标记期 weak 调用）----
// 就绪协程 args 副本仅本表可达 → 必须补标（漏标 = worker 取到悬空引用 UAF）；
// 运行中协程 vm 帧槽已由所属 worker 的 ti->vm_state 覆盖标（重复无害，此处兜底）。
void px_coro_gc_mark_roots(void) {
    sigset_t old;
    px_gc_block_stop_sig(&old);
    pthread_mutex_lock(&g_coro_mu);
    for (PxCoro* c = g_all; c; c = c->all_next) {
        if (c->args && c->nargs > 0) px_gc_mark_slots(c->args, c->nargs);
        if (c->vm.nframes > 0) px_vm_gc_mark_state(&c->vm);
    }
    pthread_mutex_unlock(&g_coro_mu);
    px_gc_unblock_stop_sig(&old);
}
