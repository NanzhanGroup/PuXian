// 普贤 (PuXian) C 运行时库 — vm.c
// M89-S3-A0/A1: 显式帧 + 平坦字节码 VM
// ------------------------------------------------------------
// 本文件 = S3-A A0 交付（PxVmState/PxFrame/PxVMFunc 生命周期 + px_vm_entry
// trampoline（D2，px_call 零改动兼容）+ 指令分发循环骨架）后，逐 A 扩展：
//   - A1: GETG/SETG 执行（v1 经 px_get_global/px_set_global，D8 无锁化后置）、
//     px_vm_run_module 完整 Top 运行（注册全局函数 D2 trampoline + 跑 Top bc）
//   - A2: B 表一元/二元运算全量（NEG..SHRU，语义=调现 px_* C 函数）
//   - A4: CALL 统一调用（PX_FUNC+px_vm_entry → 手动压帧 D3 不回 C 递归；
//     PX_NATIVE/旧 C 产物 → px_call C 递归一层）——PxFrame 增 ret_dst
//     （CALL 压帧返回写 caller 槽）；RET/RET0 弹帧回传（nframes==base 才返回）
// 已实现指令子集：LOADK IMM MOV GETG SETG SRCLINE JMP JMPT JMPF RET RET0 HALT
//   + B 表运算（A2）+ CALL（A4）；其余 op 分发默认 px_error "指令未实现"
//   （CALLM/TRY/FORCE 等 S3-B/A5 起逐批）。
//
// 执行模型：
//   px_vm_run_func 在 st 上 push 帧（slots 数组）→ 循环取指分发 →
//   RET/RET0 弹帧，当 nframes 回到入口 base → 返回。
//   px→px 调用（PXOP_CALL，A4 实现）在循环内 push/pop，不回 C 递归；
//   px→native→回调 px 经 px_call → px_vm_entry 递归一层（与 CPython
//   PyEval_EvalFrame 同构），共享线程局部 st（帧栈连续）。
//
// 值语义：运算/索引/字段/方法一律走现 runtime px_* C 函数（语义=旧轨），
// 越界/类型错误由 px_* 内 px_error 保证（语义与现一致，A2 起接入）。
//
// 内存：BCModule/PxVMFunc 进程级常驻（发射器生成静态初值，不回收，S3-D 前
// 不纳入 GC 根面）；帧槽数组 calloc/realloc 普通堆（临时，随帧弹释放）。
// ============================================================
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// M94-S2：抢占预算检查（coro.c 提供）。非 VM 轨产物（无 coro.o/vm.o 链的逃生舱）
//   → weak 空转不抢占；VM 轨 vm.o 与 coro.o 同链 → 生效。
extern int px_coro_preempt_check(void) __attribute__((weak));

// ============================================================
// M104-S2：热路径内联助手（O3 类型特化 + O1 真值判定）
// ------------------------------------------------------------
// 定位：本 VM 每个值操作原先一律调用 runtime 的 px_add/px_lt/... 动态分派
//   函数（跨 TU，-O2 下无法内联）。对「INT ⊕ INT」这一无歧义、无分配、无
//   GC/错误副作用的分支，直接在解释循环内联构造 LXValue —— 语义与对应 px_*
//   函数逐字一致（见各 case 注释）；其余类型一律回落原函数，语义零变化。
// 纪律：只内联「与 runtime 源码逐字等价」的分支；任何分配/错误/跨类型
//   语义一律回落，保证 vm_ab 38 例 + diffcheck 逐字节对拍不变。
// ============================================================
static inline LXValue vm_int(int64_t i)  { LXValue v; v.type = PX_INT;   v.as.i = i; return v; }
static inline LXValue vm_bool(bool b)    { LXValue v; v.type = PX_BOOL;  v.as.b = b; return v; }
static inline LXValue vm_float(double f) { LXValue v; v.type = PX_FLOAT; v.as.f = f; return v; }

// 真值判定快路径（对齐 px_is_truthy 的 NULL/BOOL/INT/FLOAT 分支；其余回落）。
static inline bool vm_truthy(LXValue v) {
    switch (v.type) {
    case PX_NULL:  return false;
    case PX_BOOL:  return v.as.b;
    case PX_INT:   return v.as.i != 0;
    case PX_FLOAT: return v.as.f != 0.0;
    default:       return px_is_truthy(v);
    }
}

// M104-S2：INT⊕INT 二元快路径宏（用于 B 表运算 case）。_x/_y = 两源槽值；
//   均为 INT → 用 fast_expr（引用 _x.as.i/_y.as.i）；否则调 slow_call（原 px_*）。
//   仅在同 case 内使用（依赖解释循环的 fr/in 局部）。
#define VM_BIN_F(fast_expr, slow_call) do {                                     \
        LXValue _x = fr->slots[in.b], _y = fr->slots[in.c];                     \
        fr->slots[in.a] = (_x.type == PX_INT && _y.type == PX_INT)              \
                          ? (fast_expr) : (slow_call);                          \
    } while (0)

// M104-S2（O1/O2）：源位置追踪去重 —— 线程局部「已写入 runtime 的镜像」。
//   语义冻结论证：runtime 的 g_px_src_func/g_px_src_line 均为 __thread，且在
//   VM 轨下【仅】由 vm.c 写入（runtime.c 只定义、cg 发射点只服务 C 轨产物，
//   两者不混跑）→ 本线程 tracker 与镜像恒相等，故「镜像值未变即跳过跨 TU 调用」
//   与 M104 前「每指令无条件 px_srcfunc / 每 SRCLINE 无条件 px_srcline」的
//   可观测行为逐字一致（含 native 重入 / 帧弹回后同行号等全部场景）。
//   与 push/pop 版本的区别：不依赖帧切换点，故无「弹帧后 tracker 驻留 callee 行号」
//   一类偏差。
static __thread const char* vm_lsf = NULL;   // 镜像：最近写入 g_px_src_func 的指针
static __thread int         vm_lsl = -1;     // 镜像：最近写入 g_px_src_line 的行号
static inline void vm_track_func(const char* nm) {
    if (nm != vm_lsf) { px_srcfunc(nm); vm_lsf = nm; }
}
static inline void vm_track_line(int ln) {
    if (ln != vm_lsl) { px_srcline(ln); vm_lsl = ln; }
}

// ---- 指令名表（指定初始化，避免顺序漂移）----
const char* px_op_name(int op) {
    static const char* names[PXM_MAX] = {
        [PXOP_LOADK] = "LOADK", [PXOP_IMM] = "IMM", [PXOP_MOV] = "MOV",
        [PXOP_GETG] = "GETG", [PXOP_SETG] = "SETG",
        [PXOP_NEG] = "NEG", [PXOP_NOT] = "NOT", [PXOP_BITNOT] = "BITNOT",
        [PXOP_ADD] = "ADD", [PXOP_SUB] = "SUB", [PXOP_MUL] = "MUL",
        [PXOP_DIV] = "DIV", [PXOP_IDIV] = "IDIV", [PXOP_MOD] = "MOD",
        [PXOP_POW] = "POW", [PXOP_EQ] = "EQ", [PXOP_NE] = "NE",
        [PXOP_LT] = "LT", [PXOP_LE] = "LE", [PXOP_GT] = "GT",
        [PXOP_GE] = "GE", [PXOP_BITAND] = "BITAND", [PXOP_BITOR] = "BITOR",
        [PXOP_BITXOR] = "BITXOR", [PXOP_SHL] = "SHL", [PXOP_SHR] = "SHR",
        [PXOP_SHRU] = "SHRU",
        [PXOP_INDEX] = "INDEX", [PXOP_SETIDX] = "SETIDX", [PXOP_SLICE] = "SLICE",
        [PXOP_GETF] = "GETF", [PXOP_SETF] = "SETF", [PXOP_GETF_OPT] = "GETF_OPT",
        [PXOP_NEWLIST] = "NEWLIST", [PXOP_NEWTUPLE] = "NEWTUPLE",
        [PXOP_NEWDICT] = "NEWDICT", [PXOP_NEWSTRUCT] = "NEWSTRUCT",
        [PXOP_NEWENUM] = "NEWENUM", [PXOP_LISTPUSH] = "LISTPUSH",
        [PXOP_CALL] = "CALL", [PXOP_CALLM] = "CALLM", [PXOP_CALLSELF] = "CALLSELF",
        [PXOP_TAILCALL] = "TAILCALL", [PXOP_RET] = "RET", [PXOP_RET0] = "RET0",
        [PXOP_JMP] = "JMP", [PXOP_JMPT] = "JMPT", [PXOP_JMPF] = "JMPF",
        [PXOP_TRY] = "TRY", [PXOP_FORCE] = "FORCE", [PXOP_SRCLINE] = "SRCLINE",
        [PXOP_HALT] = "HALT",
        [PXOP_NEWGEN] = "NEWGEN", [PXOP_SPAWN] = "SPAWN",
        [PXOP_ENUMVAR] = "ENUMVAR",
        [PXOP_GENFROMLIST] = "GENFROMLIST",
        [PXOP_NARGS] = "NARGS",
    };
    if (op < 0 || op >= PXM_MAX || !names[op]) return "?";
    return names[op];
}

// ---- B5：VM 并发/原语 native（chan/mutex/rwlock 构造、spawn 函数名入口、select try_recv）
// 设计：这些 C 构造在 codegen 轨被编译器特判（px_chan_create 等直调，不入全局表）；
// VM 发射器把 chan()/mutex()/rwlock()/spawn/select 编为对注册 native 的普通 CALL，
// 启动期 px_vm_run_module 注册（幂等覆盖）。旧轨产物按名查这些全局的场合无冲突
// （codegen 特判不产生 GETG chan，pxi 轨自带路径）。
static LXValue bi_vm_chan(LXValue* a, int n, void* ctx) {
    (void)ctx;
    int cap = 0;
    if (n > 0 && a[0].type == PX_INT) cap = (int)a[0].as.i;
    return px_chan_create(cap);
}
static LXValue bi_vm_mutex(LXValue* a, int n, void* ctx) {
    (void)a; (void)n; (void)ctx;
    return px_mutex_create();
}
static LXValue bi_vm_rwlock(LXValue* a, int n, void* ctx) {
    (void)a; (void)n; (void)ctx;
    return px_rwlock_create();
}
static LXValue bi_vm_spawn(LXValue* a, int n, void* ctx) {
    (void)ctx;
    if (n < 1 || a[0].type != PX_STR) px_error("spawn 需要函数名（首个参数为 str）");
    px_spawn_name(a[0].as.obj->as.str.data, n > 1 ? &a[1] : NULL, n - 1);
    return px_null();
}
static LXValue bi_vm_chan_try_recv(LXValue* a, int n, void* ctx) {
    (void)ctx;
    if (n < 1 || a[0].type != PX_CHAN) px_error("chan_try_recv 需要通道参数");
    LXValue out = px_null();
    int r = px_chan_try_recv(a[0], &out);
    return r ? out : px_null();   // 命中返回收到的值；未命中返回 null
}
// select 轮询用：命中标记（返回值本身非 null 即命中——null 消息 vs 未命中的区分
//   交由 select 展开（命中返回真值对象；此处语义：返回 PX_BOOL 命中与否 + 值槽）
static LXValue bi_vm_select_try(LXValue* a, int n, void* ctx) {
    (void)ctx;
    if (n < 2 || a[0].type != PX_CHAN) px_error("select_try 需要 (chan, out_slot_ref)");
    LXValue out = px_null();
    int r = px_chan_try_recv(a[0], &out);
    // out 以参数传入（占位 LXValue* 引用）：a[1] 为栈上容器值首地址不可改 → 用返回值解
    (void)out;
    return px_bool(r != 0);
}
static void vm_register_b5_natives(void) {
    px_set_global("chan", px_native("chan", bi_vm_chan));
    px_set_global("mutex", px_native("mutex", bi_vm_mutex));
    px_set_global("rwlock", px_native("rwlock", bi_vm_rwlock));
    px_set_global("spawn", px_native("spawn", bi_vm_spawn));
    px_set_global("chan_try_recv", px_native("chan_try_recv", bi_vm_chan_try_recv));
    px_set_global("select_try", px_native("select_try", bi_vm_select_try));
}

// ---- 线程局部 VM 状态 ----
static __thread PxVmState* g_vm_state = NULL;

// M93-S2：显式初始化一个 PxVmState（帧栈就绪）。协程内核（coro.c）建协程时
//   用它初始化协程持有的独立 VM 状态（零初始化结构不可直接 push 帧——cap=0
//   触发扩容 ncap=0 → malloc(0) 路径缺陷）。帧栈 calloc 分配、空闲协程 free。
void px_vm_state_init(PxVmState* st) {
    if (!st) return;
    if (st->cap < 16) st->cap = 16;
    if (!st->frames) st->frames = (PxFrame*)calloc((size_t)st->cap, sizeof(PxFrame));
}

PxVmState* px_vm_state(void) {
    if (!g_vm_state) {
        g_vm_state = (PxVmState*)calloc(1, sizeof(PxVmState));
        px_vm_state_init(g_vm_state);
    }
    return g_vm_state;
}

// ---- M93-S2：协程 worker 状态绑定 ----
// 协程 = 独立 PxVmState（全堆上帧栈）。worker 取到协程后把该协程 vm 绑为
//   线程当前状态（px_vm_entry → px_vm_state() 命中协程 vm → 帧栈连续推进），
//   跑完/让出后解绑。空闲 worker 不持有 px 对象（绑 NULL）。
void px_vm_bind(PxVmState* st) { g_vm_state = st; }
PxVmState* px_vm_unbind(void) {
    PxVmState* s = g_vm_state;
    g_vm_state = NULL;
    return s;
}

// ---- 帧栈 ----
// ret_dst：-1 = 顶层（RET 时返回 run_func 调用者）；≥0 = CALL 压帧，RET 时把
// 返回值写入 caller 帧的该槽（D3：px→px 调用 push 帧，返回值经槽回传）。
// S3-D-1 暂停安全不变量：**先完整初始化帧条目，最后发布 st->nframes** ——
// 并发 GC 暂停线程后按 frames[0..nframes) 遍历标记，绝不读到半初始化帧
// （旧实现先 nframes++ 再 memset/calloc，暂停落缝会读到垃圾 slots 指针）。
// 帧数组扩容用 malloc+拷贝+发布+释放旧（非 realloc）：任何暂停点 st->frames
// 都指向完整有效缓冲（发布前旧缓冲完整、发布后新缓冲完整），无需屏蔽信号。
static PxFrame* vm_frame_push(PxVmState* st, const PxVMFunc* f,
                              LXValue* args, int nargs, int ret_dst) {
    int idx = st->nframes;
    if (idx >= st->cap) {
        int ncap = st->cap * 2;
        PxFrame* nf = (PxFrame*)malloc((size_t)ncap * sizeof(PxFrame));
        if (nf) {
            memcpy(nf, st->frames, (size_t)st->nframes * sizeof(PxFrame));
            PxFrame* old = st->frames;
            st->frames = nf;      // 发布新缓冲（此刻起遍历安全）
            st->cap = ncap;
            free(old);
        }
        idx = st->nframes;
    }
    PxFrame* fr = &st->frames[idx];
    int nslot = f ? f->nslots : 0;
    fr->slots = (LXValue*)calloc((size_t)(nslot > 0 ? nslot : 1), sizeof(LXValue));
    fr->nslots = nslot;
    if (args && nargs > 0 && nslot > 0) {
        int n = nargs < nslot ? nargs : nslot;
        memcpy(fr->slots, args, (size_t)n * sizeof(LXValue));
    }
    fr->f = f;
    fr->pc = 0;
    fr->line = 0;
    fr->ret_dst = ret_dst;
    fr->nargs = nargs;          // M90-S1/F1：默认参数入口填充依 NARGS 读此
    fr->unlock_kind = 0;        // M93-S3：with 展开后置解锁（帧复用清零）
    fr->unlock_obj = px_null();
    __sync_synchronize();     // 帧字段写完成后再发布 nframes（弱序架构显式屏障）
    st->nframes = idx + 1;
    return fr;
}

// M93-S3：帧弹公共路径 —— 先执行 with 系列登记的后置解锁（unlock_kind），再释放槽。
//   解锁在 slots free 前做（unlock 只拿锁对象锁，不触 slots；顺序无关紧要但保持清晰）。
static void vm_frame_pop(PxVmState* st) {
    if (st->nframes <= 0) return;
    PxFrame* fr = &st->frames[--st->nframes];
    if (fr->unlock_kind && fr->unlock_obj.type != PX_NULL) {
        if (fr->unlock_kind == 1) {
            if (fr->unlock_obj.type == PX_MUTEX) px_mutex_unlock(fr->unlock_obj);
        } else if (fr->unlock_kind == 2) {
            if (fr->unlock_obj.type == PX_RWLOCK) px_rwlock_runlock(fr->unlock_obj);
        } else if (fr->unlock_kind == 3) {
            if (fr->unlock_obj.type == PX_RWLOCK) px_rwlock_wunlock(fr->unlock_obj);
        }
        fr->unlock_kind = 0;
        fr->unlock_obj = px_null();
    }
    free(fr->slots);
    fr->slots = NULL;
}

// ---- LOADK 常量物化 ----
// B4：PXK_FUNC 需模块 funcs 表（k.i=funcs 下标）→ vm_loadk 增加 mod 参数
static LXValue vm_loadk(const PxK* k, const PxBCModule* mod) {
    switch (k->kind) {
    case PXK_INT:   return px_int(k->i);
    case PXK_FLT:   return px_float(k->f);
    case PXK_STR:   return px_str(k->s ? k->s : "");
    case PXK_BOOL:  return px_bool(k->i != 0);
    case PXK_FUNC:
        if (mod && k->i >= 0 && k->i < mod->nfuncs) {
            const PxVMFunc* cf = &mod->funcs[(int)k->i];
            return px_func(cf->name, px_vm_entry, (void*)cf);
        }
        px_error("VM: LOADK FUNC 函数下标越界 k->i=%lld (nfuncs=%d)", k->i, mod ? mod->nfuncs : -1);
        break;
    case PXK_NULL:
    default:        return px_null();
    }
    return px_null();
}

// ---- 便捷分配器（A0 自测/调试用；正式产物静态 PxVMFunc 不走堆）----
PxVMFunc* px_vm_new_func(const char* name, int arity, int ndefault, int nslots,
                         const PxInst* bc, int nbc) {
    PxVMFunc* f = (PxVMFunc*)calloc(1, sizeof(PxVMFunc));
    f->name = name ? name : "";
    f->arity = arity;
    f->ndefault = ndefault;
    f->nslots = nslots;
    f->bc = bc;
    f->nbc = nbc;
    return f;
}

// ---- 解释循环（最小子集；其余 op → px_error "指令未实现"）----
// ---- M93-S3：阻塞方法协程让出拦截辅助（CALLM 预检；仅协程上下文 + 可让出循环）----
// 返回值：0 = 未拦截（调用方落原 px_method / px_call，走 pthread 阻塞语义）；
//   1 = 已处理 —— dst 已写 / fn 帧已压（with 系列，fn 返回由帧弹自动解锁）/
//   已让出（st->suspended=1，fr->pc 已回退到重试指令）。
// 语义对齐：chan.send/recv、mutex.lock、rwlock.rlock/wlock 阻塞版结果（send 返回发送值、
//   recv 返回收到的值、lock 返回 null）；with = try_lock + 回调 + 返回后解锁。回调内
//   px_error（longjmp 跳帧）→ 锁泄漏 —— 与现 px_method with 的 px_call 内错误同语义。
static int vm_coro_method(PxVmState* st, PxFrame* fr, LXValue ov, const char* mname,
                          LXValue* abuf, int argc, int pc, int dst) {
    if (ov.type == PX_CHAN) {
        if (strcmp(mname, "send") == 0) {
            if (argc != 1) return 0;               // 参数错 → 落 px_method 报错
            for (;;) {
                if (px_chan_try_send(ov, abuf[0])) {   // 成功：返回值 = 发送值（同阻塞版）
                    if (dst < fr->nslots) fr->slots[dst] = abuf[0];
                    return 1;
                }
                int r = px_coro_chan_send_wait(ov);    // closed→0(落阻塞版报 R1011)/
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue; // 登记期间条件满足 → 重试
                return 0;
            }
        }
        if (strcmp(mname, "recv") == 0) {
            LXValue out = px_null();
            for (;;) {
                if (px_chan_try_recv(ov, &out)) {
                    if (dst < fr->nslots) fr->slots[dst] = out;
                    return 1;
                }
                int r = px_coro_chan_recv_wait(ov);    // closed 且空 → 0(阻塞版报 R1011)
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue;
                return 0;
            }
        }
        return 0;                                    // close/unlock 等非阻塞 → px_method
    }
    if (ov.type == PX_MUTEX) {
        if (strcmp(mname, "lock") == 0) {
            for (;;) {
                LXValue tl = px_mutex_try_lock(ov);
                if (tl.type == PX_BOOL && tl.as.b) {
                    if (dst < fr->nslots) fr->slots[dst] = px_null();   // lock 返回 null
                    return 1;
                }
                int r = px_coro_mutex_wait(ov);
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue;
                return 0;
            }
        }
        if (strcmp(mname, "with") == 0) {
            if (argc != 1) return 0;
            for (;;) {
                LXValue tl = px_mutex_try_lock(ov);
                if (tl.type == PX_BOOL && tl.as.b) break;    // 已拿锁 → 执行回调
                int r = px_coro_mutex_wait(ov);
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue;
                return 0;
            }
            LXValue fn = abuf[0];
            if (fn.type == PX_FUNC && fn.as.obj->as.func.fn == px_vm_entry) {
                // VM 函数回调：压帧（ret_dst=dst）+ 帧标 unlock 后置 —— fn 中途让出
                //   锁随帧保留（GC 标 unlock_obj 防回收），恢复后 fn RET 弹帧自动解锁。
                PxVMFunc* cf2 = (PxVMFunc*)fn.as.obj->as.func.ctx;
                if (cf2->arity > 0) { px_mutex_unlock(ov); return 0; }  // 带参回调落原语义报错
                vm_frame_push(st, cf2, NULL, 0, dst);
                PxFrame* nf = &st->frames[st->nframes - 1];
                nf->unlock_kind = 1;
                nf->unlock_obj = ov;
                return 1;
            }
            LXValue r = px_call(fn, NULL, 0);              // native 回调：直调后解锁
            px_mutex_unlock(ov);
            if (dst < fr->nslots) fr->slots[dst] = r;
            return 1;
        }
        return 0;
    }
    if (ov.type == PX_RWLOCK) {
        if (strcmp(mname, "rlock") == 0) {
            for (;;) {
                LXValue tl = px_rwlock_try_rlock(ov);
                if (tl.type == PX_BOOL && tl.as.b) {
                    if (dst < fr->nslots) fr->slots[dst] = px_null();
                    return 1;
                }
                int r = px_coro_rwlock_wait_r(ov);
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue;
                return 0;
            }
        }
        if (strcmp(mname, "wlock") == 0) {
            for (;;) {
                LXValue tl = px_rwlock_try_wlock(ov);
                if (tl.type == PX_BOOL && tl.as.b) {
                    if (dst < fr->nslots) fr->slots[dst] = px_null();
                    return 1;
                }
                int r = px_coro_rwlock_wait_w(ov);
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue;
                return 0;
            }
        }
        if (strcmp(mname, "with_read") == 0 || strcmp(mname, "with_write") == 0) {
            if (argc != 1) return 0;
            int is_w = mname[5] == 'w';                  // "with_write"[5]='w'，read 为 'r'
            for (;;) {
                LXValue tl = is_w ? px_rwlock_try_wlock(ov) : px_rwlock_try_rlock(ov);
                if (tl.type == PX_BOOL && tl.as.b) break;
                int r = is_w ? px_coro_rwlock_wait_w(ov) : px_coro_rwlock_wait_r(ov);
                if (r == PX_CORO_WAIT_BLOCKED) { fr->pc = pc; st->suspended = 1; return 1; }
                if (r == PX_CORO_WAIT_RETRY) continue;
                return 0;
            }
            LXValue fn = abuf[0];
            if (fn.type == PX_FUNC && fn.as.obj->as.func.fn == px_vm_entry) {
                PxVMFunc* cf2 = (PxVMFunc*)fn.as.obj->as.func.ctx;
                if (cf2->arity > 0) {
                    if (is_w) px_rwlock_wunlock(ov); else px_rwlock_runlock(ov);
                    return 0;
                }
                vm_frame_push(st, cf2, NULL, 0, dst);
                PxFrame* nf = &st->frames[st->nframes - 1];
                nf->unlock_kind = is_w ? 3 : 2;
                nf->unlock_obj = ov;
                return 1;
            }
            LXValue r = px_call(fn, NULL, 0);
            if (is_w) px_rwlock_wunlock(ov); else px_rwlock_runlock(ov);
            if (dst < fr->nslots) fr->slots[dst] = r;
            return 1;
        }
        return 0;
    }
    return 0;
}

// ---- 解释循环核心（M93-S3 重构：可让出）----
// 参数：base = 本入口应跑到的帧深（px_vm_run_func：push 前深度；协程恢复：0）。
//   yield_ok = 1 表示协程顶层解释循环（阻塞点可让出）；0 = 嵌套/主线程（遇阻塞
//   走原 pthread 路径 —— 语义与现一致）。
// 返回：0 = 完成（*out_ret 有效）；1 = 让出（st->suspended=1，协程已登记等待队列，
//   帧栈保留在 st，唤醒后由 px_vm_resume 从让出点恢复）。
static int vm_run_loop(PxVmState* st, int base, int yield_ok, LXValue* out_ret) {
    LXValue ret = px_null();
    int done = 0;
    PxFrame* fr = &st->frames[st->nframes - 1];
    static __thread unsigned long vm_tick = 0;   // M94-S2：抢占节流计数（每 4096 条查一次）

    while (!done) {
        // M94-S2：抢占 tick —— 仅协程顶层循环（yield_ok=1，主线程/嵌套不抢占）。
        //   每 4096 条指令查一次时间片预算；超时 → return 2（抢占让出）。抢占点 =
        //   解释循环指令边界（绝不在 native C 内部 / 让出登记临界区内）→ 帧栈完整
        //   保留、pc 已指向下一条 → resume 直接续跑；协程未登记等待 → worker 直接
        //   放回就绪队尾（无 lost-wakeup / 双执行竞态，见 coro.c worker yield_rc==2）。
        if (yield_ok && ((++vm_tick & 4095UL) == 0) &&
            px_coro_preempt_check && px_coro_preempt_check()) {
            st->suspended = 1;
            return 2;
        }
        // 帧可能因 CALL（A4 起）被推入/弹出，每次循环取当前帧
        fr = &st->frames[st->nframes - 1];
        const PxVMFunc* cf = fr->f;          // 当前帧所属函数
        // M104-S2（O1）：函数名追踪改为「镜像去重」（见 vm_track_func 注释）——
        //   等价于原「每指令 px_srcfunc(cf->name)」，值未变时免一次跨 TU 调用。
        vm_track_func(cf->name);
        int pc = fr->pc;
        // M104-S2（O4）：槽数组指针提到局部 —— 原每条指令经 `fr->slots[..]` 两次
        //   间接（fr 解引用 + slots 读取）→ 提到寄存器后每条操作数访问省一次内存读。
        //   帧切换（CALL/RET/TRY 传播）后 break 回循环顶重新取 fr/slots，故恒有效；
        //   slots 指向的堆槽数组不随帧数组 realloc 移动，native 递归回调亦安全。
        LXValue* slots = fr->slots;
        if (pc < 0 || pc >= cf->nbc) {       // 越界 → 视作自然落尾
            vm_frame_pop(st);
            if (st->nframes == base) done = 1;
            break;
        }
        const PxInst in = cf->bc[pc];        // 拷贝取指，防 slots realloc 悬垂
        fr->pc = pc + 1;
        switch (in.op) {
        case PXOP_HALT:
            vm_frame_pop(st);
            if (st->nframes == base) done = 1;
            break;
        case PXOP_RET: {
            LXValue v = slots[in.a];
            int rd = fr->ret_dst;
            vm_frame_pop(st);
            if (st->nframes == base) { ret = v; done = 1; }
            else if (rd >= 0) {          // A4：写回 caller 帧 dst 槽（D3 帧回传）
                PxFrame* pf = &st->frames[st->nframes - 1];
                if (rd < pf->nslots) pf->slots[rd] = v;
            }
            break;
        }
        case PXOP_RET0: {
            LXValue v = px_null();
            int rd = fr->ret_dst;
            vm_frame_pop(st);
            if (st->nframes == base) { ret = v; done = 1; }
            else if (rd >= 0) {
                PxFrame* pf = &st->frames[st->nframes - 1];
                if (rd < pf->nslots) pf->slots[rd] = v;
            }
            break;
        }
        case PXOP_CALL: {
            // A4：统一调用。callee 槽 = in.b，参数 = 槽 in.b+1..in.b+argc 连续区，
            // 返回写槽 in.a。分派：VM 函数（PX_FUNC 且 fn==px_vm_entry）→ 手动压帧
            // （D3：px→px 不回 C 递归，帧栈承接递归深度）；PX_NATIVE / 旧 C 编译产物
            // （fn != px_vm_entry）→ px_call 直调（C 递归一层，与 CPython 同构）。
            LXValue fnv = slots[in.b];
            int argc = in.c;
            int dst = in.a;
            LXValue* abuf = NULL;
            if (argc > 0) {
                int n = argc;
                if (in.b + 1 + n > fr->nslots) n = fr->nslots - (int)in.b - 1;
                if (n > 0) {
                    abuf = (LXValue*)malloc((size_t)n * sizeof(LXValue));
                    for (int i = 0; i < n; i++) abuf[i] = slots[in.b + 1 + i];
                    argc = n;
                } else argc = 0;
            }
            // —— M93-S3：协程上下文 + 可让出循环：sleep/sleep_us 定时器让出预检 ——
            //   （登记全局 sleep 等待表 + 让出，worker 不阻塞；到期由 timer 线程唤醒
            //   恢复后直接跑下一条 —— sleep 结果恒 null，让出前预写 dst）
            if (yield_ok && px_coro_active() && argc >= 1) {
                int bk = px_native_blocking_kind(fnv);
                if (bk != PX_BLK_NONE) {
                    long long us = 0;
                    int have = 0;
                    if (abuf[0].type == PX_INT) { us = abuf[0].as.i; have = 1; }
                    else if (abuf[0].type == PX_FLOAT) { us = (long long)abuf[0].as.f; have = 1; }
                    if (bk == PX_BLK_SLEEP_MS) us *= 1000;   // sleep(ms) → us
                    if (have) {
                        int r = px_coro_sleep_us(us);
                        if (r == PX_CORO_WAIT_RETRY) {       // us<=0：立即完成
                            free(abuf);
                            if (dst < fr->nslots) slots[dst] = px_null();
                            break;
                        }
                        if (r == PX_CORO_WAIT_BLOCKED) {     // 已登记定时器 → 让出（不回退 pc）
                            free(abuf);
                            if (dst < fr->nslots) slots[dst] = px_null();
                            st->suspended = 1;
                            return 1;
                        }
                        // r==0：非协程 ctx（理论不达，yield_ok=1 恒协程）→ 落 px_call 兜底
                    }
                }
            }
            // —— M96-S2：阻塞网络 native offload 预检（http_get/tcp_recv 试点；名单见
            //   runtime.c px_native_offload_kind）——
            // 协程 ctx 命中名单（C 层全协议阻塞桥：阻塞点在 C native 内部 recv 循环/
            //   TLS，M93-S3 让出与 M94 抢占都救不了）→ 投递外包执行线程池异步执行，
            //   协程登记 offload 等待让出（pc 不回退：native 已在外包线程跑，resume 后
            //   px_coro_offload_consume 把结果写 dst 槽 —— M93 sleep「预写 dst 让出」模式
            //   的推广）。主线程/嵌套回调/逃生舱（yield_ok=0）→ 落 px_call 直调
            //   （pthread 阻塞语义零变化）。
            if (yield_ok && px_coro_active() && px_native_offload_kind(fnv)) {
                int r = px_coro_offload_submit(fnv, abuf, argc, dst);
                if (r == PX_CORO_WAIT_BLOCKED) {
                    abuf = NULL;          // 实参数组所有权已转移给外包任务（consume 释放）
                    st->suspended = 1;
                    return 1;             // 让出；pc 已前进（不回退）
                }
                // r==0：提交失败（非协程 ctx 理论不达 / 内存失败）→ 落 px_call 直调兜底
            }
            if (fnv.type == PX_FUNC &&
                fnv.as.obj->as.func.fn == px_vm_entry) {
                PxVMFunc* cf2 = (PxVMFunc*)fnv.as.obj->as.func.ctx;
                if (argc < cf2->arity) {      // 参数不足（默认参数 S3-B 补）
                    free(abuf);
                    px_error("VM %s:%d CALL %s 参数不足: 需 %d 给 %d",
                             fr->f->name, fr->line, cf2->name, cf2->arity, argc);
                    break;
                }
                vm_frame_push(st, cf2, abuf, argc, dst);  // 压子帧，循环继续
                free(abuf);
            } else {
                LXValue r = px_call(fnv, abuf, argc);     // native/旧C/非函数
                free(abuf);
                fr = &st->frames[st->nframes - 1];        // 刷新（未 push，帧不变）
                if (dst < fr->nslots) slots[dst] = r;
            }
            break;
        }
        case PXOP_SRCLINE:
            // M104-S2（O2）：行号追踪改为「镜像去重」（见 vm_track_line 注释）——
            //   等价于原「每 SRCLINE 无条件 px_srcline(fr->line)」，值未变时免一次调用。
            fr->line = (int)(int16_t)in.b;
            vm_track_line(fr->line);
            break;
        case PXOP_IMM:
            slots[in.a] = px_int((int64_t)(int16_t)in.b);
            break;
        case PXOP_NARGS:   // M90-S1/F1：槽a = 本帧实际实参数（默认参数入口填充依据）
            slots[in.a] = px_int((int64_t)fr->nargs);
            break;
        case PXOP_MOV:
            slots[in.a] = slots[in.b];
            break;
        case PXOP_GETG: {
            // M107-S2（Issue 35）：经「稳定指针槽位记忆」取值——m->G[b] 是编译期常量指针，
            //   首次按名查找后记忆槽位号，其后 O(1) 命中（语义与 px_get_global 逐字一致：
            //   未定义仍报同一文案、取值同在 g_globals_mu 读锁内拷贝）。
            const PxBCModule* m = cf->mod;
            if (!m || in.b >= (uint16_t)m->nG) {
                px_error("VM %s:%d GETG 全局越界 g=%d (nG=%d)",
                         cf->name, fr->line, in.b, m ? m->nG : -1);
            }
            int gi_g = px_global_resolve_stable(m->G[in.b]);
            if (gi_g < 0) px_error("未定义变量: %s", m->G[in.b]);
            slots[in.a] = px_global_at(gi_g);
            break;
        }
        case PXOP_SETG: {
            // A1：v1 经 px_set_global；a=G idx，b=src 槽
            const PxBCModule* m = cf->mod;
            if (!m || in.a >= (uint16_t)m->nG) {
                px_error("VM %s:%d SETG 全局越界 g=%d (nG=%d)",
                         cf->name, fr->line, in.a, m ? m->nG : -1);
            }
            px_set_global(m->G[in.a], slots[in.b]);
            break;
        }
        case PXOP_LOADK: {
            const PxBCModule* m = cf->mod;
            if (!m || in.b >= (uint16_t)m->nK) {
                px_error("VM %s: LOADK 常量越界 k=%d (nK=%d)",
                         cf->name, in.b, m ? m->nK : -1);
            }
            slots[in.a] = vm_loadk(&m->K[in.b], m);
            break;
        }
        // ---- B 表：一元/二元运算（A2，语义=调现 px_* C 函数，错误由 px_* 保证）----
        // ---- B 表运算：INT⊕INT 类型特化快路径（M104-S2/O3）----
        //   每个 case 先取槽值到局部（避免重复索引 + 让编译器把值留在寄存器）；
        //   快路径命中 → 内联构造结果；未命中 → 回落原 px_* 函数（语义零变化）。
        //   快路径与 runtime 对应函数逐字同语义（见 vm_int/vm_bool 注释）。
        case PXOP_NEG: {
            LXValue x = slots[in.b];
            if (x.type == PX_INT)        slots[in.a] = vm_int(-x.as.i);
            else if (x.type == PX_FLOAT) slots[in.a] = vm_float(-x.as.f);
            else                         slots[in.a] = px_neg(x);   // 其余=px_error 路径
            break;
        }
        case PXOP_NOT:    slots[in.a] = vm_bool(!vm_truthy(slots[in.b])); break;
        case PXOP_BITNOT: {
            LXValue x = slots[in.b];
            slots[in.a] = (x.type == PX_INT) ? vm_int(~x.as.i) : px_bitnot(x);
            break;
        }
        case PXOP_ADD:    VM_BIN_F(vm_int(_x.as.i + _y.as.i), px_add(_x, _y)); break;
        case PXOP_SUB:    VM_BIN_F(vm_int(_x.as.i - _y.as.i), px_sub(_x, _y)); break;
        case PXOP_MUL:    VM_BIN_F(vm_int(_x.as.i * _y.as.i), px_mul(_x, _y)); break;
        case PXOP_DIV: {
            // px_div：d=num_val(b) 为 0 → px_error，随后仍返回 float 除法（逐字对齐）
            LXValue x = slots[in.b], y = slots[in.c];
            if (x.type == PX_INT && y.type == PX_INT) {
                double d = (double)y.as.i;
                if (d == 0) px_error("除零错误");
                slots[in.a] = vm_float((double)x.as.i / d);
            } else slots[in.a] = px_div(x, y);
            break;
        }
        case PXOP_IDIV: {
            // px_idiv（INT-INT 分支）：欧几里得商（余数非负）；d==0 回落 px_idiv 报错
            LXValue x = slots[in.b], y = slots[in.c];
            if (x.type == PX_INT && y.type == PX_INT && y.as.i != 0) {
                int64_t d = y.as.i, n = x.as.i;
                int64_t r = n % d;
                if (r < 0) r += (d < 0 ? -d : d);
                slots[in.a] = vm_int((n - r) / d);
            } else slots[in.a] = px_idiv(x, y);
            break;
        }
        case PXOP_MOD: {
            // px_mod（INT-INT 分支）：rem_euclid（余数非负）；d==0 回落 px_mod 报错
            LXValue x = slots[in.b], y = slots[in.c];
            if (x.type == PX_INT && y.type == PX_INT && y.as.i != 0) {
                int64_t d = y.as.i, n = x.as.i;
                int64_t r = n % d;
                if (r < 0) r += (d < 0 ? -d : d);
                slots[in.a] = vm_int(r);
            } else slots[in.a] = px_mod(x, y);
            break;
        }
        case PXOP_POW:    slots[in.a] = px_pow(slots[in.b], slots[in.c]); break;
        case PXOP_EQ:     VM_BIN_F(vm_bool(_x.as.i == _y.as.i), px_eq(_x, _y)); break;
        case PXOP_NE:     VM_BIN_F(vm_bool(_x.as.i != _y.as.i), px_ne(_x, _y)); break;
        case PXOP_LT:     VM_BIN_F(vm_bool(_x.as.i <  _y.as.i), px_lt(_x, _y)); break;
        case PXOP_LE:     VM_BIN_F(vm_bool(_x.as.i <= _y.as.i), px_le(_x, _y)); break;
        case PXOP_GT:     VM_BIN_F(vm_bool(_x.as.i >  _y.as.i), px_gt(_x, _y)); break;
        case PXOP_GE:     VM_BIN_F(vm_bool(_x.as.i >= _y.as.i), px_ge(_x, _y)); break;
        case PXOP_BITAND: VM_BIN_F(vm_int(_x.as.i & _y.as.i),  px_bitand(_x, _y)); break;
        case PXOP_BITOR:  VM_BIN_F(vm_int(_x.as.i | _y.as.i),  px_bitor(_x, _y)); break;
        case PXOP_BITXOR: VM_BIN_F(vm_int(_x.as.i ^ _y.as.i),  px_bitxor(_x, _y)); break;
        case PXOP_SHL:    VM_BIN_F(vm_int(_x.as.i << _y.as.i), px_shl(_x, _y)); break;
        case PXOP_SHR:    VM_BIN_F(vm_int(_x.as.i >> _y.as.i), px_shr(_x, _y)); break;
        // px_ushr：按 uint64 逻辑右移，移位量 &63（逐字对齐）
        case PXOP_SHRU:   VM_BIN_F(vm_int((int64_t)((uint64_t)_x.as.i >> ((uint64_t)_y.as.i & 63u))),
                                   px_ushr(_x, _y)); break;

        // ---- C 表：容器 / 字段（S3-B B1；语义=调现 px_* C 函数，错误由 px_* 保证）----
        // NEWLIST/NEWTUPLE：a=dst，b=连续槽基址，c=n；自 槽 b..b+n-1 拷贝建容器
        case PXOP_NEWLIST: {
            int n = (int)in.c;
            int cap = 0;
            LXValue* tmp2 = NULL;
            if (n > 0 && (int)in.b + n <= fr->nslots) cap = n;
            else if (n > 0) cap = fr->nslots - (int)in.b;
            if (cap > 0) {
                tmp2 = (LXValue*)malloc((size_t)cap * sizeof(LXValue));
                for (int i = 0; i < cap; i++) tmp2[i] = slots[in.b + i];
                slots[in.a] = px_list_n(tmp2, cap);
                free(tmp2);
            } else slots[in.a] = px_list_n(NULL, 0);
            break;
        }
        case PXOP_NEWTUPLE: {
            int n = (int)in.c;
            int cap = 0;
            LXValue* tmp2 = NULL;
            if (n > 0 && (int)in.b + n <= fr->nslots) cap = n;
            else if (n > 0) cap = fr->nslots - (int)in.b;
            if (cap > 0) {
                tmp2 = (LXValue*)malloc((size_t)cap * sizeof(LXValue));
                for (int i = 0; i < cap; i++) tmp2[i] = slots[in.b + i];
                slots[in.a] = px_tuple(tmp2, cap);
                free(tmp2);
            } else slots[in.a] = px_tuple(NULL, 0);
            break;
        }
        // NEWDICT：a=dst，b=连续槽基址，c=项数 n（槽 b..b+2n-1 为 k0,v0,k1,v1..）；
        //   仅字符串键入 dict（对齐 codegen：if (_k.type == PX_STR) px_dict_set）
        case PXOP_NEWDICT: {
            LXValue d = px_dict();
            int n = (int)in.c;
            for (int i = 0; i < n; i++) {
                int k0 = (int)in.b + 2 * i;
                if (k0 + 1 < fr->nslots && slots[k0].type == PX_STR)
                    px_dict_set(d, slots[k0].as.obj->as.str.data, slots[k0 + 1]);
            }
            slots[in.a] = d;
            break;
        }
        // NEWSTRUCT（B2）：a=dst，b=struct 元数据 idx（mod->structs），
        //   c=字段值连续槽基址（槽 c..c+nfields-1）→ px_struct（同 codegen 构造）
        case PXOP_NEWSTRUCT: {
            const PxBCModule* mm = cf->mod;
            if (!mm || in.b >= (uint16_t)mm->nstructs)
                px_error("VM %s:%d NEWSTRUCT 元数据越界 st=%d (n=%d)",
                         cf->name, fr->line, in.b, mm ? mm->nstructs : -1);
            const PxStructDef* sd = &mm->structs[in.b];
            if (sd->nfields < 0 || (int)in.c + sd->nfields > fr->nslots)
                px_error("VM %s:%d NEWSTRUCT 槽越界 base=%d nf=%d", cf->name, fr->line, in.c, sd->nfields);
            int nf = sd->nfields;
            LXValue* vals = nf > 0 ? (LXValue*)malloc((size_t)nf * sizeof(LXValue)) : NULL;
            char** fns = nf > 0 ? (char**)malloc((size_t)nf * sizeof(char*)) : NULL;
            for (int i = 0; i < nf; i++) { vals[i] = slots[in.c + i]; fns[i] = (char*)sd->fnames[i]; }
            slots[in.a] = px_struct(sd->name, fns, vals, nf);
            free(vals); free(fns);
            break;
        }
        // NEWENUM（B2）：a=dst，b=N 类型名 idx，c=N 变体名 idx → px_enum
        case PXOP_NEWENUM: {
            const PxBCModule* mm = cf->mod;
            if (!mm || in.b >= (uint16_t)mm->nN || in.c >= (uint16_t)mm->nN)
                px_error("VM %s:%d NEWENUM 名字越界 b=%d c=%d (nN=%d)",
                         cf->name, fr->line, in.b, in.c, mm ? mm->nN : -1);
            slots[in.a] = px_enum(mm->N[in.b], mm->N[in.c]);
            break;
        }
        // NEWGEN（B3，M34 惰性生成器）：a=dst，b=seq 槽，c=2 连续槽基址
        //   [transform(PX_FUNC), filter(PX_FUNC|null)] → px_gen_lazy（同 codegen 单 for
        //   GenExp：elt 恒为 transform 闭包，cond 有则 filter 闭包）。槽越界容错（防御）。
        case PXOP_NEWGEN: {
            LXValue tf = px_null(), fl = px_null();
            if ((int)in.c >= 0 && (int)in.c < fr->nslots) tf = slots[in.c];
            if ((int)in.c + 1 < fr->nslots) fl = slots[in.c + 1];
            slots[in.a] = px_gen_lazy(slots[in.b], tf, fl);
            break;
        }
        // ENUMVAR（B3b）：a=dst，b=obj 槽 —— enum→px_str(variant)、非 enum→null
        //   （match 模式匹配 variant 判断；对齐 cg subject.type==PX_ENUM && strcmp）
        case PXOP_ENUMVAR:
            slots[in.a] = px_enum_variant(slots[in.b]);
            break;
        // GENFROMLIST（S3-C C2）：a=dst，b=list 槽 → px_gen_from_list —— 物化
        //   GenExp（多 for/多变量子句）先收集 list 再包成 generator（对齐 codegen
        //   cg GenExp 物化路径 px_gen_from_list；pxi 轨 i_eval GenExp it_gen 同语义）
        case PXOP_GENFROMLIST:
            slots[in.a] = px_gen_from_list(slots[in.b]);
            break;
        case PXOP_LISTPUSH:  // a=val 槽，b=list 槽（值入列表尾）
            px_list_push(slots[in.b], slots[in.a]);
            break;
        case PXOP_INDEX: {   // a=dst，b=obj，c=idx
            // M104-S2（O3）：LIST[INT] 快路径（逐字对齐 px_index 的 PX_LIST 分支：
            //   负索引回绕 + 越界 px_error 文案一致）；其余类型回落 px_index。
            LXValue obj = slots[in.b], idx = slots[in.c];
            if (obj.type == PX_LIST && idx.type == PX_INT) {
                int i = (int)idx.as.i;
                int len = obj.as.obj->as.list.len;
                if (i < 0) i += len;
                if (i < 0 || i >= len) px_error("列表索引越界: %d (len=%d)", i, len);
                slots[in.a] = obj.as.obj->as.list.items[i];
            } else slots[in.a] = px_index(obj, idx);
            break;
        }
        case PXOP_SETIDX:    // a=val 槽，b=obj，c=idx（赋值表达式结果=val）
            px_index_set(slots[in.b], slots[in.c], slots[in.a]);
            break;
        case PXOP_SLICE:     // a=dst，b=obj，c=3 连续槽基址[start,end,step]
            if ((int)in.c + 2 < fr->nslots)
                slots[in.a] = px_slice(slots[in.b], slots[in.c],
                                           slots[in.c + 1], slots[in.c + 2]);
            else px_error("VM %s:%d SLICE 槽越界 base=%d", cf->name, fr->line, in.c);
            break;
        case PXOP_GETF:      // a=dst，b=obj，c=N 名字 idx（px_field）
            if (cf->mod && in.c < cf->mod->nN)
                slots[in.a] = px_field(slots[in.b], cf->mod->N[in.c]);
            else px_error("VM %s:%d GETF 名字越界 n=%d", cf->name, fr->line, in.c);
            break;
        case PXOP_GETF_OPT: { // OptionalField：obj null→null，否则 px_field
            LXValue o = slots[in.b];
            if (cf->mod && in.c < cf->mod->nN)
                slots[in.a] = px_is_null(o) ? px_null()
                                                : px_field(o, cf->mod->N[in.c]);
            else px_error("VM %s:%d GETF_OPT 名字越界 n=%d", cf->name, fr->line, in.c);
            break;
        }
        case PXOP_SETF:      // a=val 槽，b=obj，c=N 名字 idx（px_field_set）
            if (cf->mod && in.c < cf->mod->nN)
                px_field_set(slots[in.b], cf->mod->N[in.c], slots[in.a]);
            else px_error("VM %s:%d SETF 名字越界 n=%d", cf->name, fr->line, in.c);
            break;
        // CALLM：方法调用桥（px_method obj.name(args..)，obj=槽 b，方法名=N[c]，
        //   fl=argc，实参=槽 b+1..b+argc；返回写槽 a）——px_method 语义=现 C 桥
        case PXOP_CALLM: {
            LXValue ov = slots[in.b];
            int argc = (int)in.fl;
            int dst = in.a;
            LXValue* abuf = NULL;
            if (argc > 0) {
                int n = argc;
                if (in.b + 1 + n > fr->nslots) n = fr->nslots - (int)in.b - 1;
                if (n > 0) {
                    abuf = (LXValue*)malloc((size_t)n * sizeof(LXValue));
                    for (int i = 0; i < n; i++) abuf[i] = slots[in.b + 1 + i];
                    argc = n;
                } else argc = 0;
            }
            // —— M93-S3：协程阻塞方法让出预检（chan send/recv、mutex lock/with、
            //    rwlock rlock/wlock/with_read/with_write）——
            if (yield_ok && px_coro_active() && cf->mod && in.c < cf->mod->nN) {
                int h = vm_coro_method(st, fr, ov, cf->mod->N[in.c], abuf, argc, pc, dst);
                if (h) {
                    free(abuf);
                    if (st->suspended) return 1;   // 已让出（登记等待队列，恢复重试）
                    break;                          // 已处理（dst 已写 / fn 帧已压）
                }
            }
            LXValue r = px_null();
            if (cf->mod && in.c < cf->mod->nN)
                r = px_method(ov, cf->mod->N[in.c], abuf, argc);
            else px_error("VM %s:%d CALLM 名字越界 n=%d", cf->name, fr->line, in.c);
            free(abuf);
            if (in.a < fr->nslots) slots[in.a] = r;
            break;
        }

        // ---- E 表：错误传播（A5，D7）----
        // TRY（?）：Result-Err → 就地返回 Err（RET 语义回传）；null → 返回 null；
        // Ok → 就地解包覆写槽。对齐 codegen err_tag 模型（函数尾仅转发，语义等价）。
        case PXOP_TRY: {
            LXValue v = slots[in.a];
            int is_err = px_is_result(v) && !px_result_ok(v);
            int is_nul = !is_err && px_is_null(v);
            if (is_err || is_nul) {
                LXValue ev = is_err ? v : px_null();
                int rd = fr->ret_dst;
                vm_frame_pop(st);
                if (st->nframes == base) { ret = ev; done = 1; }
                else if (rd >= 0) {
                    PxFrame* pf = &st->frames[st->nframes - 1];
                    if (rd < pf->nslots) pf->slots[rd] = ev;
                }
                break;
            }
            if (px_is_result(v)) v = px_result_unwrap(v);   // Ok 就地解包
            slots[in.a] = v;
            break;
        }
        // FORCE（!）：Result-Err → px_error；null → px_error；否则就地解包
        case PXOP_FORCE: {
            LXValue v = slots[in.a];
            if (px_is_result(v)) {
                if (!px_result_ok(v))
                    px_error("force unwrap Err: %s", px_to_string(px_result_unwrap(v)));
                v = px_result_unwrap(v);
            }
            if (px_is_null(v)) px_error("force unwrap null");
            slots[in.a] = v;
            break;
        }
        case PXOP_JMP:
            fr->pc += (int)(int16_t)in.b;   // off 相对下一条：目标=(pc+1)+off
            break;
        case PXOP_JMPT:
            if (vm_truthy(slots[in.a])) fr->pc += (int)(int16_t)in.b;
            break;
        case PXOP_JMPF:
            if (!vm_truthy(slots[in.a])) fr->pc += (int)(int16_t)in.b;
            break;
        default:
            // A1 起逐批实现；错误现场含函数名/行号/op 名，对齐 px_error 语义
            px_error("VM %s:%d 指令未实现: %s (op=%d)",
                     cf->name, fr->line, px_op_name(in.op), in.op);
            break;
        }
    }
    *out_ret = ret;
    return 0;
}

// ---- 入口三函数 ----
// px_vm_run_func：不可让出（yield_ok=0）——主线程/嵌套 native 回调（px_call →
//   px_vm_entry）遇阻塞 native 走原 pthread 路径，语义与现完全一致。
LXValue px_vm_run_func(PxVmState* st, const PxVMFunc* f, LXValue* args, int nargs) {
    int base = st->nframes;                 // 入口帧前栈深
    vm_frame_push(st, f, args, nargs, -1);  // ret_dst=-1：返回给本函数调用者
    LXValue ret = px_null();
    vm_run_loop(st, base, 0, &ret);
    return ret;
}

// px_vm_run_coro / px_vm_resume（M93-S3，coro.c worker 用）：可让出（yield_ok=1）。
//   返回码（vm_run_loop 透传）：0 = 跑完（worker 回收）；1 = 阻塞让出（协程已登记
//   等待队列，worker 不再触碰）；2 = 抢占让出（M94-S2：时间片耗尽，协程未登记任何
//   等待，worker 直接放回就绪队尾）。st->suspended 由让出点置 1（诊断用，worker 不读）。
int px_vm_run_coro(PxVmState* st, const PxVMFunc* f, LXValue* args, int nargs) {
    int base = st->nframes;                 // 协程全新：0
    st->suspended = 0;
    vm_frame_push(st, f, args, nargs, -1);
    LXValue ret = px_null();
    int rc = vm_run_loop(st, base, 1, &ret);
    if (rc == 0) st->ret_val = ret;         // M95-S2：跑完保存顶层返回值（完成回调取）
    return rc;
}

int px_vm_resume(PxVmState* st) {
    st->suspended = 0;
    // M96-S2：offload 完成消费 —— 仅 resume 路径调一次（不占热循环）：若本协程有待
    //   消费 offload 任务（阻塞 native 已在外包线程执行完）→ 结果写回让出点帧槽 /
    //   错误重抛（px_error → longjmp worker 隔离点，协程异常终止，语义 = 直调）。
    //   chan/mutex/sleep/抢占 resume 无任务（off_task==NULL）→ 判空即返，开销可忽略；
    //   抢占 resume 协程必未登记 offload（抢占点不在 native 内）→ 恒无任务。
    px_coro_offload_consume(st);
    LXValue ret = px_null();
    int rc = vm_run_loop(st, 0, 1, &ret);
    if (rc == 0) st->ret_val = ret;         // M95-S2：同上
    return rc;
}

// ---- D2 trampoline：统一函数对象 func.fn = px_vm_entry（ctx=PxVMFunc*）----
LXValue px_vm_entry(LXValue* args, int nargs, void* ctx) {
    PxVMFunc* f = (PxVMFunc*)ctx;
    if (!f) return px_null();
    return px_vm_run_func(px_vm_state(), f, args, nargs);
}

// ---- 运行模块顶层（A1：注册全局函数 + Top bc；main() 调用约定随 S3-B CALL 接入）----
LXValue px_vm_run_module(PxVmState* st, const PxBCModule* m) {
    if (!m) return px_null();
    if (m->top_idx < 0 || m->top_idx >= m->nfuncs) {
        px_error("VM 模块 %s 无 Top 函数", m->name ? m->name : "?");
        return px_null();
    }
    // B5：VM 并发/原语构造 native（chan/mutex/rwlock/spawn/chan_try_recv/select_try）
    //   幂等注册（旧轨 codegen 特判不产生 GETG chan，无冲突）
    vm_register_b5_natives();
    // 注册全局函数（除 Top / 闭包）：统一 px_func(name, px_vm_entry, &funcs[i])（D2 trampoline）
    // —— 与 codegen main() 里 px_set_global 注册等价；此后 px_get_global("fname") 可调用
    // B2：impl 方法名 "Type.method" 亦在 funcs → 一并注册（px_method struct 分支按名转发）。
    // B4 起：闭包（name 以 '<' 开头）不注册全局（LOADK PXK_FUNC 直接引用，B4）。
    int i = 0;
    for (i = 0; i < m->nfuncs; i++) {
        if (i == m->top_idx) continue;
        const PxVMFunc* f = &m->funcs[i];
        if (f->name[0] == '<') continue;
        px_set_global(f->name, px_func(f->name, px_vm_entry, (void*)f));
    }
    // 跑顶层 bc（Top：声明/赋值 + S3-B 起 main() 调用约定）
    return px_vm_run_func(st, &m->funcs[m->top_idx], NULL, 0);
}

// S3-D 止血切片：runtime 单线程 GC 弱引用调用 —— 把当前线程 VM 活跃帧槽
// 补标为根。帧槽数组在堆上，保守 GC 只扫 C 栈/全局/暂存根 → 槽内活跃对象
// 不可见会被误回收（use-after-free）或漏回收（堆只增）。按帧逐槽标记：
// 未用槽 = calloc 零值（PX_NULL），px_value_is_obj 为假无副作用；帧存活期
// 槽值保守全标（宁漏回收不误回收），弹帧后 slots 已 free 且不在 frames[0..nframes)
// 范围内不再标记。
// S3-D-1 扩展：px_vm_gc_mark_state(void*) 供并发 GC executor 遍历「已暂停线程」
// 的 VM 状态做跨线程帧根标记（旧实现只标本线程 → 多 spawn 线程跑 VM 时其余
// 线程帧槽漏标 → 活跃对象被误回收 use-after-free）；px_vm_cur_state 供
// GC 暂停信号处理器（运行在目标线程上）读取其 TLS VM 状态指针（不懒建）。
void px_vm_gc_mark_state(void* vst) {
    PxVmState* st = (PxVmState*)vst;
    if (!st) return;
    // M95-S2：协程完成返回值（完成-回调窗口 GC 兜底；线程 vm ret_val 恒 null 无碍）
    if (st->ret_val.type != PX_NULL) px_gc_mark_slots(&st->ret_val, 1);
    if (st->nframes <= 0) return;
    for (int i = 0; i < st->nframes; i++) {
        PxFrame* fr = &st->frames[i];
        if (fr->slots && fr->nslots > 0)
            px_gc_mark_slots(fr->slots, fr->nslots);
        if (fr->unlock_kind && fr->unlock_obj.type != PX_NULL)   // M93-S3：with 展开
            px_gc_mark_slots(&fr->unlock_obj, 1);                // 持锁对象保活（帧弹前）
    }
}

void px_vm_gc_mark(void) {
    px_vm_gc_mark_state(g_vm_state);
}

void* px_vm_cur_state(void) {
    return g_vm_state;
}
