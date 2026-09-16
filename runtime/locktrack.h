// ============================================================
// runtime/locktrack.h —— M127（qg-issue 84）：隔离点**回卷的锁安全审计**
// ------------------------------------------------------------
// 用法：作为某 TU 的**最后一个** #include（宏会改写此后本 TU 内的所有 pthread 锁调用）：
//     #include "locktrack.h"
//
// 为什么需要它（病灶）：
//   M125/M126 把「运行时错误 / 分配失败」从进程级收紧到请求级（longjmp 回隔离点）。
//   但 **longjmp 不展开 pthread 锁**：若失败点落在临界区内，回卷会把该锁永久留在
//   「已锁」状态 ⇒ 其他线程随后阻塞在 pthread_mutex_lock 上 ⇒ 进程**既不服务也不退出**
//   （假死：systemd 看不到失败、监控看不到异常）。实测现场（gdb，全线程栈）：
//       #0 __lll_lock_wait  #1 pthread_mutex_lock  #2 gc_register  #3 px_str_len  #4 vm_run_loop
//   即：gc_register 持 g_gc_mu 时 `xrealloc(g_objs,…)` → xmalloc 分配失败 → px_alloc_fail
//   → longjmp ⇒ g_gc_mu 永锁 ⇒ 之后**任何**分配（= 任何请求）都阻塞。
//
// 做法：
//   · 本头把每个加锁/解锁动作**同时**记进「本线程的持锁栈」（TLS，O(1)、无分配、
//     可在任意上下文调用，不引入新的失败点）；
//   · 隔离点在 longjmp 之前审计（见 runtime.c 的 px_unwind_lock_guard）：
//       - 无锁在身 → 照旧 longjmp（请求级 5xx）—— 绝大多数情况，与 M125/M126 行为完全一致；
//       - 有锁在身 → **无法安全回滚**（强行解锁会破坏被放弃的临界区不变量）⇒ 打印持锁
//         清单并 `_exit(1)`（systemd 3s 内拉起；服务中断 ~3s，而不是永久假死）。
//   · 逃生舱 `PX_UNWIND_LOCK_GUARD=0` → 回退「无条件 longjmp」（M127 门负控用，生产勿设）。
//
// 边界（如实，勿误读）：
//   1. `pthread_cond_wait` 期间互斥量由 pthread 释放 ⇒ 本头在等待期间把它出栈（口径正确：
//      「此刻不持锁」）。但「等待中被 longjmp 丢弃」会绕过 glibc 的 cond 组计数清理 ——
//      属已知遗留（罕见，且远优于留锁假死）。
//   2. 只覆盖 pthread 锁；自旋/`_Atomic` 类自定义同步不在内（本仓 runtime 的运行时锁均为
//      pthread 锁）。
//   3. 持锁栈深度 > PX_LT_MAX 时只计数、不记录名字（审计仍会触发，只少列几个名字）。
//   4. 计数必须**动态平衡**：任何「加锁后提前 return 不解锁」的既有缺陷会表现为审计假阳性
//      （保守方向：宁可 exit(1)，也不留锁假死）。
// ============================================================
#ifndef PX_LOCKTRACK_H
#define PX_LOCKTRACK_H

#include <pthread.h>

#define PX_LT_MAX 24

// 存储定义在 runtime.c（TLS，跨 TU 共享同一份）
extern __thread const void* px_lt_mu[PX_LT_MAX];
extern __thread const char* px_lt_name[PX_LT_MAX];
extern __thread int         px_lt_n;

static inline void px_lt_enter_(const void* mu, const char* name) {
    if (px_lt_n >= 0 && px_lt_n < PX_LT_MAX) { px_lt_mu[px_lt_n] = mu; px_lt_name[px_lt_n] = name; }
    px_lt_n++;
}
static inline void px_lt_leave_(const void* mu) {
    (void)mu;
    if (px_lt_n > 0) px_lt_n--;
}
static inline const char* px_lt_top_name_(const void* mu) {
    if (px_lt_n > 0 && px_lt_n <= PX_LT_MAX && px_lt_mu[px_lt_n - 1] == mu) return px_lt_name[px_lt_n - 1];
    return NULL;
}

// —— 包装函数（**必须定义在下面的 #define 之前**：此刻 pthread_mutex_lock 仍是真函数）——
static inline int px_lt_lock_(pthread_mutex_t* m, const char* name) {
    int r = pthread_mutex_lock(m);
    if (r == 0) px_lt_enter_(m, name);
    return r;
}
static inline int px_lt_unlock_(pthread_mutex_t* m) {
    px_lt_leave_(m);
    return pthread_mutex_unlock(m);
}
static inline int px_lt_rdlock_(pthread_rwlock_t* l, const char* name) {
    int r = pthread_rwlock_rdlock(l);
    if (r == 0) px_lt_enter_(l, name);
    return r;
}
static inline int px_lt_wrlock_(pthread_rwlock_t* l, const char* name) {
    int r = pthread_rwlock_wrlock(l);
    if (r == 0) px_lt_enter_(l, name);
    return r;
}
static inline int px_lt_rwunlock_(pthread_rwlock_t* l) {
    px_lt_leave_(l);
    return pthread_rwlock_unlock(l);
}
// cond_wait：等待期间互斥量由 pthread 释放 ⇒ 出栈（口径 = 此刻本线程不持锁），返回后恢复。
static inline int px_lt_condwait_(pthread_cond_t* c, pthread_mutex_t* m, const char* name) {
    const char* saved = px_lt_top_name_(m);
    px_lt_leave_(m);
    int r = pthread_cond_wait(c, m);
    px_lt_enter_(m, saved ? saved : name);
    return r;
}
static inline int px_lt_condtimedwait_(pthread_cond_t* c, pthread_mutex_t* m,
                                       const struct timespec* ts, const char* name) {
    const char* saved = px_lt_top_name_(m);
    px_lt_leave_(m);
    int r = pthread_cond_timedwait(c, m, ts);
    px_lt_enter_(m, saved ? saved : name);
    return r;
}

// —— 此后本 TU 内的锁调用全部走上面（注释里的同形文本也一并替换，无副作用）——
#define pthread_mutex_lock(m)        px_lt_lock_((m), #m)
#define pthread_mutex_unlock(m)      px_lt_unlock_((m))
#define pthread_rwlock_rdlock(l)     px_lt_rdlock_((l), #l)
#define pthread_rwlock_wrlock(l)     px_lt_wrlock_((l), #l)
#define pthread_rwlock_unlock(l)     px_lt_rwunlock_((l))
#define pthread_cond_wait(c, m)      px_lt_condwait_((c), (m), #m)
#define pthread_cond_timedwait(c, m, ts) px_lt_condtimedwait_((c), (m), (ts), #m)

#endif // PX_LOCKTRACK_H
