// 普贤 (PuXian) C 运行时库 — coro.c
// M93: 帧协程 M:N（px_spawn 用户态协程化）
// ------------------------------------------------------------
// 设计（docs/M93_PLAN.md D1-D4）：px_spawn 目标为 VM 函数（PX_FUNC.fn==px_vm_entry，
// ctx=PxVMFunc*）→ 不再每 spawn 建一个 pthread，而是建「帧协程」入就绪队列；
// M 个常驻 worker 线程（PX_CORO_WORKERS，默认 min(在线CPU,8)）轮取出队执行。
// 协程 = 独立 PxVmState（帧栈全堆上，px→px 调用不回 C 递归）→ 执行状态全部在
// 堆上，切换只换指针（px_vm_bind/unbind），零 ucontext/零汇编。
//
// S2 范围：纯计算协程（跑到结束）。S3（本版）范围：阻塞原语让出 —— chan
// send/recv、mutex lock、rwlock r/wlock 的 try 失败后把当前协程登记到对象等待
// 链表（runtime.h 各对象 cw_* 字段）并让出（state=BLOCKED，worker 解绑继续取
// 下一协程，不占线程）；sleep/sleep_us 登记全局定时表让出（timer 线程到点唤醒）。
// 唤醒方（chan send/recv/close、unlock 等，runtime.c 在各对象 mu 内）take 摘链 →
// px_coro_wake 入就绪队列 → worker 重跑该协程（px_vm_resume 从让出点重试 try，
// 成功即继续）。pthread 阻塞路径（cond_wait）与协程等待链表双轨共存（D4），
// 逃生舱/主线程/嵌套 native 回调场景保持 pthread 语义零变化。
//
// 并发安全设计（关键）：
//   - g_coro_mu 临界区（入队/出队/回收/GC 标记遍历/wake）均屏蔽 SIG_GC_STOP
//     （px_gc_block_stop_sig）→ 持锁临界区不被 STW 信号打断 → GC executor
//     标记协程表拿 g_coro_mu 时，持锁线程必在推进（非被暂停自旋）→ 无死锁。
//   - 登记 API（px_coro_*_wait）在对象 mu 内「条件复查 + 挂链 + state=BLOCKED」
//     原子完成 → 与唤醒方（同锁 take）无 lost wakeup；take 摘链后协程即归就绪
//     队列管理（state=READY 由 wake 置），原 worker 让出返回后不再触碰它。
//   - GC 根面（runtime.c 标记期 weak 调 px_coro_gc_mark_roots）：全部存活协程
//     （READY/RUNNING/BLOCKED）的 args 副本 + 帧槽精确标记；with 展开持锁对象
//     由 px_vm_gc_mark_state 补标（PxFrame.unlock_obj）。worker 跑协程期间经
//     px_gc_thread_enter 注册 GC 槽。
//   - worker 空闲（cond_wait 等就绪队列）不注册 GC（无 px 对象）→ 与 M90
//     F3-fix 同模式：GC STW 只暂停在岗 worker，不因空闲 worker 空转放大开销。
//   - sleep timer 线程不注册 GC（不持 px 对象，只操作就绪队列/定时链表）→ 不
//     被 STW 暂停，持 g_timer_mu 总会放锁，与 GC 拿 g_coro_mu 无死锁。
//   - M94-S3：定时器并入调度循环（单锁 g_coro_mu 保护 g_sleepers + g_rq）——退役
//     独立 timer 线程与 g_timer_mu/g_timer_cond；worker 取协程前置摘到期 sleeper
//     入就绪、就绪空时 cond_timedwait 到最近到期。锁序问题随单锁收敛消除。
//   - M94-S2：抢占式时间片 —— vm_run_loop 每 4096 条指令查 px_coro_preempt_check
//     （本次运行 ≥ quantum → return 2）；worker 把协程直接放回就绪队尾轮转
//     （协程未登记等待 → 无唤醒源 → 无 lost-wakeup/双执行竞态）。PX_CORO_QUANTUM_US
//     默认 5000us，0=关闭（回归 M93 无抢占语义逃生阀）。
// ============================================================
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// ---- 协程状态机 ----
#define CORO_READY   0
#define CORO_RUNNING 1
#define CORO_BLOCKED 2   // 已登记等待（chan/mutex/rwlock/sleep），唤醒后重入就绪
#define CORO_DONE    3

typedef struct PxCoro {
    struct PxCoro* next;      // 就绪队列链（FIFO）
    struct PxCoro* all_next;  // 全局存活表链（GC 根面 + 完成回收）
    struct PxCoro* w_next;    // M93-S3：对象等待链表（chan cw_send/cw_recv、
                              //   mutex cw、rwlock cw_w/cw_r —— take/wake 用）
    struct PxCoro* s_next;    // M93-S3：sleep 定时表链（按 due_us 升序）
    PxVmState      vm;        // 协程独立 VM 状态（帧栈全堆上；让出保留、恢复续跑）
    const PxVMFunc* f;        // 目标 VM 函数（ctx 直传 px_vm_run_coro）
    LXValue*       args;      // 实参副本（spawn 时复制，首次运行拷贝入帧后仍保活）
    int            nargs;
    int            state;
    int            id;
    int            first;     // M93-S3：1 = 尚未首次运行（worker 用 px_vm_run_coro）
    int            wake_pending; // M93-S3：让出窗口被唤醒但协程仍在跑（延迟到让出完成
                               //   入队 —— 见 worker 让出协议 / px_coro_wake）
    long long      due_us;    // M93-S3：sleep 到期（CLOCK_REALTIME us）
    long long      run_begin_us; // M94-S2：本次运行时间片起点（worker resume 前记）
    // M95-S2：完成回调（协程跑完 return 0 → worker 回收前调；服务端 handler
    //   协程化用它把结果投回续处理。回调在 worker 线程、仍注册 GC 时执行，
    //   回调内须 PX_KEEP 保护 ret 再转移给全局 GC 根。）
    void (*done_cb)(void* ud, LXValue ret);
    void*          done_ud;
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
static __thread PxCoro* g_cur_coro = NULL;  // M93-S3：当前 worker 正在执行的协程

// ---- M94-S3：sleep 定时表（并入调度循环，g_coro_mu 保护 —— 无独立 timer 线程）----
// g_sleepers 按 due_us 升序。登记 = 持 g_coro_mu 升序插入；摘取 = worker 取就绪
//   协程前在 g_coro_mu 内把到期者摘入就绪队列（见 coro_worker）。单锁收敛消除了
//   旧双锁（g_timer_mu→g_coro_mu）的锁序问题；无专职 timer 线程 → 线程数再 -1。
static PxCoro* g_sleepers = NULL;        // 按 due_us 升序（M94-S3：g_coro_mu 保护）
static long long g_quantum_us = -1;  // M94-S2：抢占时间片 us；PX_CORO_QUANTUM_US
                                     //   （0=关、<200 夹 200、>1e6 夹 1e6；-1=未初始化）

static long long coro_now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

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

// ---- M93-S3：取协程摘链 / 入就绪队列唤醒 ----
// take：调用方必须已持对象锁（与登记 API 同锁互斥 → 无 lost wakeup）。
// wake：入就绪队列 + signal worker；可在对象锁内或锁外调用（只碰 g_coro_mu）。
struct PxCoro* px_coro_take_waiter(struct PxCoro** head) {
    struct PxCoro* c = *head;
    if (c) { *head = c->w_next; c->w_next = NULL; }
    return c;
}

void px_coro_wake(struct PxCoro* c) {
    if (!c) return;
    sigset_t old;
    px_gc_block_stop_sig(&old);
    pthread_mutex_lock(&g_coro_mu);
    if (c->state == CORO_RUNNING) {
        // 协程还在让出返回路径上（登记完成可被摘链、但 worker 尚未置 BLOCKED）：
        //   直接入队会让协程被第二个 worker 并行 resume（双执行撕裂）。置 pending，
        //   由让出 worker 在置 BLOCKED 后查 pending 自行入队 —— 调度权转移原子化。
        c->wake_pending = 1;
    } else if (c->state == CORO_BLOCKED) {
        // 正常：协程已让出完成（worker 置 BLOCKED）→ 入就绪队列
        c->state = CORO_READY;
        c->next = NULL;
        if (g_rq_tail) g_rq_tail->next = c; else g_rq = c;
        g_rq_tail = c;
    }
    // state==READY（已在队列，理论不达——take 只摘一次等待链）/ DONE/异常 → 忽略
    pthread_mutex_unlock(&g_coro_mu);
    px_gc_unblock_stop_sig(&old);
    pthread_cond_signal(&g_coro_cond);
}

// ---- worker 线程：循环取就绪协程执行（跑到让出/抢占/完成）----
// M94-S3：定时摘取并入本循环 —— 取就绪前先摘到期 sleeper 入就绪队列（等效原
//   timer 线程 px_coro_wake，内联避免二次入队）；就绪空且有未来 sleeper →
//   cond_timedwait 到最近到期（绝对时间，到期自醒重摘）；就绪空且无 sleeper →
//   cond_wait 等唤醒（新协程入队 / 新 sleeper 头登记 signal）。任一空闲 worker
//   承担到点摘取职责，无专职 timer 线程。
static void* coro_worker(void* arg) {
    (void)arg;
    for (;;) {
        // 取队首就绪协程（空闲期 cond_wait；未注册 GC → STW 无空闲放大）
        sigset_t old;
        px_gc_block_stop_sig(&old);
        pthread_mutex_lock(&g_coro_mu);
        PxCoro* c = NULL;
        for (;;) {
            // 1) 摘到期 sleeper 入就绪队列（唤醒规则同 px_coro_wake：RUNNING →
            //    wake_pending 由让出 worker 自入队；BLOCKED → 直接入队 READY）
            long long now = coro_now_us();
            while (g_sleepers && g_sleepers->due_us <= now) {
                PxCoro* s = g_sleepers;
                g_sleepers = s->s_next;
                s->s_next = NULL;
                if (s->state == CORO_RUNNING) {     // 还在让出返回路径（登记未完成）
                    s->wake_pending = 1;            //   让出 worker 置 BLOCKED 后自入队
                } else if (s->state == CORO_BLOCKED) {
                    s->state = CORO_READY;
                    if (g_rq_tail) g_rq_tail->next = s; else g_rq = s;
                    g_rq_tail = s;
                }
                // 其它态（READY/DONE/异常）理论不达：sleeper 只被定时摘取，忽略
            }
            if (g_rq) break;                        // 就绪非空（含刚摘的 sleeper）→ 取队首
            if (g_sleepers) {                       // 无就绪、有未来 sleeper → 等到最近到期
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                long long rel = g_sleepers->due_us - now;
                if (rel < 1) rel = 1;
                ts.tv_sec += rel / 1000000;
                ts.tv_nsec += (rel % 1000000) * 1000;
                if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
                pthread_cond_timedwait(&g_coro_cond, &g_coro_mu, &ts);
            } else {
                pthread_cond_wait(&g_coro_cond, &g_coro_mu);
            }
            // 醒来：cond_signal（新就绪 / 新 sleeper 头登记）或 timedwait 到期 →
            //   循环顶重摘（多个 worker 同刻醒来也只会有一个摘到，其余回 wait）
        }
        c = g_rq;
        g_rq = c->next;
        if (!g_rq) g_rq_tail = NULL;
        c->next = NULL;
        c->state = CORO_RUNNING;
        pthread_mutex_unlock(&g_coro_mu);
        px_gc_unblock_stop_sig(&old);
        if (!c) break;

        // 跑协程：注册 GC → bind vm → 隔离执行 → 解绑 → 注销
        int cid = c->id;              // 跑前保存（让出后协程可能被唤醒并回收，不可再读 c）
        px_gc_thread_enter();
        px_vm_bind(&c->vm);
        g_cur_coro = c;
        int yield_rc = 0;
        c->run_begin_us = coro_now_us();   // M94-S2：本次时间片起点（抢占预算基准）
        if (px_spawn_isolate_begin()) {
            if (c->first) {                    // 首次：压顶层帧跑（可让出）
                c->first = 0;
                yield_rc = px_vm_run_coro(&c->vm, c->f, c->args, c->nargs);
            } else {                            // 恢复：从让出点继续（不压帧）
                yield_rc = px_vm_resume(&c->vm);
            }
            px_spawn_isolate_end();
        }                                        // 错误：longjmp 回 → 协程异常终止
        g_cur_coro = NULL;
        px_vm_unbind();
        px_gc_thread_leave();

        if (yield_rc == 2) {
            // M94-S2：抢占让出（vm_run_loop 时间片耗尽 return 2）。协程**未登记任何
            //   等待**（抢占点仅解释循环指令边界，不在 native 内/登记窗口内）→ 无
            //   唤醒源、无 wake_pending 竞态 → worker 直接放回就绪队尾（FIFO 轮转）。
            //   与阻塞让出（yield_rc==1，已挂等待链表，worker 不得再触碰）本质不同。
            sigset_t old2;
            px_gc_block_stop_sig(&old2);
            pthread_mutex_lock(&g_coro_mu);
            c->state = CORO_READY;
            c->next = NULL;
            if (g_rq_tail) g_rq_tail->next = c; else g_rq = c;
            g_rq_tail = c;
            pthread_mutex_unlock(&g_coro_mu);
            px_gc_unblock_stop_sig(&old2);
            pthread_cond_signal(&g_coro_cond);
            if (g_coro_diag == 1)
                fprintf(stderr, "[px-coro] #%d preempt\n", cid);
            continue;
        }
        if (yield_rc) {
            // M93-S3：协程让出（运行函数返回码 1 —— 已登记到等待队列 cw 链表/sleep
            //   定时表）。状态机收敛在 g_coro_mu 内：置 BLOCKED（此刻起可被唤醒方
            //   take+wake 调度）；若让出窗口内已被唤醒（wake_pending，唤醒方见
            //   RUNNING 延迟入队）→ 立即入就绪队列。完成后本 worker 不再触碰 c
            //   （c 可能随即被其它 worker 唤醒/恢复/回收 —— use-after-free 防护）。
            sigset_t old2;
            px_gc_block_stop_sig(&old2);
            pthread_mutex_lock(&g_coro_mu);
            c->state = CORO_BLOCKED;
            if (c->wake_pending) {
                c->wake_pending = 0;
                c->state = CORO_READY;
                c->next = NULL;
                if (g_rq_tail) g_rq_tail->next = c; else g_rq = c;
                g_rq_tail = c;
            }
            pthread_mutex_unlock(&g_coro_mu);
            px_gc_unblock_stop_sig(&old2);
            pthread_cond_signal(&g_coro_cond);
            if (g_coro_diag == 1)
                fprintf(stderr, "[px-coro] #%d block-wait\n", cid);
            continue;
        }

        // 完成 → 从全局存活表摘除 + 回收（完成协程未被并发触碰：未让出 = 未登记）
        px_gc_block_stop_sig(&old);
        pthread_mutex_lock(&g_coro_mu);
        c->state = CORO_DONE;
        PxCoro** pp = &g_all;
        while (*pp && *pp != c) pp = &(*pp)->all_next;
        if (*pp) *pp = c->all_next;
        pthread_mutex_unlock(&g_coro_mu);
        px_gc_unblock_stop_sig(&old);
        if (g_coro_diag == 1)
            fprintf(stderr, "[px-coro] #%d done (f=%s)\n", cid,
                    c->f && c->f->name ? c->f->name : "?");
        // M95-S2：完成回调（摘除 g_all 后、free 前；worker 仍注册 GC。ret 已存
        //   vm.ret_val；回调内须 PX_KEEP 保护后再转移给全局 GC 根 —— 见调用方约定）
        if (c->done_cb) {
            LXValue rv = c->vm.ret_val;
            c->done_cb(c->done_ud, rv);
        }
        free(c->args);
        coro_vm_free(&c->vm);
        free(c);
    }
    return NULL;
}

// ---- 并发 worker 池惰性启动（首个协程 spawn 时）----
// 注意：g_quantum_us 在创建 worker 线程**之前**初始化（写）→ worker 跑协程时
//   preempt_check 只读已定值，无数据竞争。
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
    if (g_quantum_us < 0) {          // M94-S2：抢占时间片（-1=未初始化 → 读 env 一次）
        const char* q = getenv("PX_CORO_QUANTUM_US");
        long long v = q ? atoll(q) : 5000;   // 默认 5ms
        if (v != 0) {
            if (v < 200) v = 200;
            if (v > 1000000) v = 1000000;
        }
        g_quantum_us = v;            // 0 = 显式关闭抢占（回归 M93 语义逃生阀）
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
// px_coro_spawn_ex：带完成回调版（M95-S2）。px_coro_spawn = ex 的 NULL 回调壳。
void px_coro_spawn_ex(void* ctx, LXValue* args, int nargs,
                      void (*done_cb)(void* ud, LXValue ret), void* done_ud) {
    PxCoro* c = (PxCoro*)calloc(1, sizeof(PxCoro));
    if (!c) return;
    c->f = (const PxVMFunc*)ctx;
    px_vm_state_init(&c->vm);
    c->nargs = nargs;
    c->args = (LXValue*)malloc(sizeof(LXValue) * (nargs > 0 ? nargs : 1));
    if (c->args && nargs > 0 && args)
        memcpy(c->args, args, sizeof(LXValue) * nargs);
    c->state = CORO_READY;
    c->first = 1;                 // M93-S3：首次需压顶层帧
    c->done_cb = done_cb;         // M95-S2
    c->done_ud = done_ud;         // M95-S2
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

// px_coro_spawn：无完成回调（M93 原语义壳）
void px_coro_spawn(void* ctx, LXValue* args, int nargs) {
    px_coro_spawn_ex(ctx, args, nargs, NULL, NULL);
}

// ==================== M93-S3：阻塞登记 API ====================
// 返回码（runtime.h 约定）：0 = 不在协程 ctx（调用方走原 pthread 阻塞）；
//   PX_CORO_WAIT_BLOCKED(1) = 已登记并让出（state=BLOCKED）；PX_CORO_WAIT_RETRY(2)
//   = 条件已满足（调用方重新 try）。登记在对象 mu 内「复查 + 挂链 + BLOCKED」原子
//   完成（唤醒方同锁 take → 无 lost wakeup）。仅在 g_cur_coro 非 NULL（协程 ctx）
//   生效；逃生舱 pthread spawn / 主线程 / 嵌套 native 回调返回 0 → 走阻塞版。

int px_coro_active(void) { return g_cur_coro != NULL; }

static void coro_list_append(struct PxCoro** head, PxCoro* c) {
    c->w_next = NULL;
    if (!*head) { *head = c; return; }
    PxCoro* t = *head;
    while (t->w_next) t = t->w_next;
    t->w_next = c;
}

// chan send：满 / 无缓冲无接收者 → 挂 cw_send；closed → 0（调用方走阻塞版报 R1011）
int px_coro_chan_send_wait(LXValue ch) {
    if (!g_cur_coro) return 0;
    if (ch.type != PX_CHAN) return 0;
    LXObject* o = ch.as.obj;
    int r = 0;
    pthread_mutex_lock(&o->as.chan.mu);
    if (o->as.chan.closed) {
        r = 0;
    } else if (o->as.chan.cap == 0
                 ? (o->as.chan.recv_waiting > 0 || o->as.chan.cw_recv != NULL)
                 : o->as.chan.len < o->as.chan.cap) {
        r = PX_CORO_WAIT_RETRY;                    // 登记前资源已腾出 → 重试 try
    } else {
        coro_list_append(&o->as.chan.cw_send, g_cur_coro);
        r = PX_CORO_WAIT_BLOCKED;
    }
    pthread_mutex_unlock(&o->as.chan.mu);
    return r;
}

// chan recv：空 → 挂 cw_recv；closed 且空 → 0（阻塞版报 R1011）
int px_coro_chan_recv_wait(LXValue ch) {
    if (!g_cur_coro) return 0;
    if (ch.type != PX_CHAN) return 0;
    LXObject* o = ch.as.obj;
    int r = 0;
    pthread_mutex_lock(&o->as.chan.mu);
    if (o->as.chan.len > 0) {
        r = PX_CORO_WAIT_RETRY;
    } else if (o->as.chan.closed) {
        r = 0;
    } else {
        coro_list_append(&o->as.chan.cw_recv, g_cur_coro);
        r = PX_CORO_WAIT_BLOCKED;
    }
    pthread_mutex_unlock(&o->as.chan.mu);
    return r;
}

// mutex lock：被占 → 挂 cw（mutex 无 closed，全部协程 ctx 内可让出）
int px_coro_mutex_wait(LXValue m) {
    if (!g_cur_coro) return 0;
    if (m.type != PX_MUTEX) return 0;
    LXObject* o = m.as.obj;
    int r;
    pthread_mutex_lock(&o->as.mutex.mu);
    if (!o->as.mutex.locked) {
        r = PX_CORO_WAIT_RETRY;
    } else {
        coro_list_append(&o->as.mutex.cw, g_cur_coro);
        r = PX_CORO_WAIT_BLOCKED;
    }
    pthread_mutex_unlock(&o->as.mutex.mu);
    return r;
}

// rwlock rlock：写者持有或写者等待 → 挂 cw_r（条件对齐 try_rlock）
int px_coro_rwlock_wait_r(LXValue m) {
    if (!g_cur_coro) return 0;
    if (m.type != PX_RWLOCK) return 0;
    LXObject* o = m.as.obj;
    int r;
    pthread_mutex_lock(&o->as.rwlock.mu);
    if (!o->as.rwlock.writer && o->as.rwlock.writer_waiting == 0) {
        r = PX_CORO_WAIT_RETRY;
    } else {
        coro_list_append(&o->as.rwlock.cw_r, g_cur_coro);
        r = PX_CORO_WAIT_BLOCKED;
    }
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return r;
}

// rwlock wlock：写者/读者活跃 → 挂 cw_w（条件对齐 try_wlock；写优先由唤醒方保证）
int px_coro_rwlock_wait_w(LXValue m) {
    if (!g_cur_coro) return 0;
    if (m.type != PX_RWLOCK) return 0;
    LXObject* o = m.as.obj;
    int r;
    pthread_mutex_lock(&o->as.rwlock.mu);
    if (!o->as.rwlock.writer && o->as.rwlock.readers == 0) {
        r = PX_CORO_WAIT_RETRY;
    } else {
        coro_list_append(&o->as.rwlock.cw_w, g_cur_coro);
        r = PX_CORO_WAIT_BLOCKED;
    }
    pthread_mutex_unlock(&o->as.rwlock.mu);
    return r;
}

// sleep/sleep_us：登记全局定时表（timer 线程到点唤醒入就绪）→ 让出。
//   us<=0 立即返回（RETRY → 调用方写 null 继续）；非协程 ctx → 0（走 nanosleep）。
int px_coro_sleep_us(long long us) {
    if (!g_cur_coro) return 0;
    if (us <= 0) return PX_CORO_WAIT_RETRY;
    PxCoro* c = g_cur_coro;
    c->due_us = coro_now_us() + us;
    sigset_t old;
    px_gc_block_stop_sig(&old);
    pthread_mutex_lock(&g_coro_mu);            // M94-S3：单锁（与就绪队列同锁）
    PxCoro** pp = &g_sleepers;                 // 按到期升序插入
    while (*pp && (*pp)->due_us <= c->due_us) pp = &(*pp)->s_next;
    c->s_next = *pp;
    *pp = c;
    int is_head = (g_sleepers == c);           // 成为新的最近到期？
    pthread_mutex_unlock(&g_coro_mu);
    px_gc_unblock_stop_sig(&old);
    if (is_head) pthread_cond_signal(&g_coro_cond);  // 早于 worker 当前 timedwait 目标
                                                      //   → 唤醒一个 worker 重算 deadline；
                                                      //   非头则已有更早 sleeper，worker 必
                                                      //   已 timedwait 到 ≤ 本 due，无需打扰
    return PX_CORO_WAIT_BLOCKED;
}

// M94-S2：抢占预算检查（vm_run_loop 每 4096 条指令 weak 调一次）。仅协程上下文
//   （g_cur_coro 非空）；本次运行时长 ≥ quantum → 返回 1（解释循环抢占让出 return 2，
//   worker 直接放回就绪队尾）。g_quantum_us<=0（env=0 关闭 / -1 未初始化）→ 不抢占。
int px_coro_preempt_check(void) {
    if (g_quantum_us <= 0) return 0;
    PxCoro* c = g_cur_coro;
    if (!c) return 0;
    long long now = coro_now_us();
    return (now - c->run_begin_us) >= g_quantum_us ? 1 : 0;
}

// ---- GC 根面：全部存活协程（runtime.c 标记期 weak 调用）----
// 就绪/阻塞协程 args 副本与帧槽仅本表可达 → 必须补标（漏标 = worker 取到悬空
// 引用 UAF）；运行中协程 vm 帧槽已由所属 worker 的 ti->vm_state 覆盖标（重复无害）。
// BLOCKED 协程帧栈保留在堆（让出点），经 px_vm_gc_mark_state 精确标记 —— 挂起
// 协程无 C 栈 = 纯帧槽根（M92 精确 GC 直接受益）。
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
