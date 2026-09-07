// 普贤 (PuXian) C 运行时库 — vm.h
// M89-S3-A0/A1: 显式帧 + 平坦字节码 VM 骨架（设计定稿 docs/M89_vm_design.md）
// ------------------------------------------------------------
// 定位：单一执行引擎（第三种实现）。PuXian 程序统一编译为平坦字节码，
// 由本 VM（runtime 层 C）执行。发射器（AST→BC，PuXian 自举 bc_emit.px）
// 在 S3-A 起与 codegen 平行开发；本文件 = C 侧数据结构 + 指令集编号
// （编号一经 S3 定稿即冻结，发射器按名字引用）。
//
// 状态（A0→A4，2026-09-08）：解释循环已实现
//   LOADK/IMM/MOV/GETG/SETG/SRCLINE/JMP/JMPT/JMPF/RET/RET0/HALT + B 表运算
//   （NEG..SHRU，A2）+ CALL（A4：VM 函数手动压帧 D3、native/旧 C 产物 px_call）；
//   CALLM/TRY/FORCE/容器等 S3-B/A5 起逐批（默认分发 px_error "指令未实现"）。
// 线程模型：px_vm_entry 在调用线程取/建 __thread PxVmState 执行（与
//   px_pool_worker/px_spawn 线程模型同构）；spawn/连接线程各自独立 VM 状态。
// 语义约定：以现双轨（px build fn_* C 产物 / pxi 树遍历）用例集为对拍基准。
// ============================================================
#ifndef PX_VM_H
#define PX_VM_H

#include "runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 字节码格式 ====================
// 指令 = {op u8, fl u8, a u16, b u16, c u16} = 8B 定长。pc 单位为「条」。
// 操作数约定（指令集 spec §二）：
//   s=帧槽号  k=常量池 K idx  g=全局槽 idx  n=名字池 N idx
//   off=相对跳转条数(int16，存 b 字段)   argc=实参个数
// 跳转语义：off 相对「下一条」—— 目标 pc = (当前pc+1) + off。
// 约定 a=目标槽(dst)/g/专用；b/c=源槽/常量/名字/off（视指令，见各 op 注释）。
typedef struct {
    uint8_t op;
    uint8_t fl;      // 预留 flag（S3 起逐 op 定义：如 TRY 的 Err 携带形态等）
    uint16_t a, b, c;
} PxInst;            // 8B 定长

// ==================== 指令集编号（冻结） ====================
// A 加载/槽
#define PXOP_LOADK   1   // a=dst s, b=K[k]             槽s = 常量 K[k]
#define PXOP_IMM     2   // a=dst s, b=imm16(符号)       槽s = int(imm16) 小整数快捷
#define PXOP_MOV     3   // a=dst s, b=src s2            槽s = 槽s2
#define PXOP_GETG    4   // a=dst s, b=g                 槽s = 全局槽 g（v1 经 px_get_global，D8 无锁化后置）
#define PXOP_SETG    5   // a=g, b=src s                 全局槽 g = 槽s（顶层 var，同步 g_globals）
// B 一元/二元（a=dst s，b/c=源槽；语义=调对应 px_* 现 C 函数）
#define PXOP_NEG     6
#define PXOP_NOT     7
#define PXOP_BITNOT  8
#define PXOP_ADD     9
#define PXOP_SUB    10
#define PXOP_MUL    11
#define PXOP_DIV    12
#define PXOP_IDIV   13
#define PXOP_MOD    14
#define PXOP_POW    15
#define PXOP_EQ     16
#define PXOP_NE     17
#define PXOP_LT     18
#define PXOP_LE     19
#define PXOP_GT     20
#define PXOP_GE     21
#define PXOP_BITAND 22
#define PXOP_BITOR  23
#define PXOP_BITXOR 24
#define PXOP_SHL    25
#define PXOP_SHR    26
#define PXOP_SHRU   27
// C 容器/字段
#define PXOP_INDEX   28  // a=dst, b=obj s2, c=idx s3
#define PXOP_SETIDX  29  // a=val 槽, b=obj s2, c=idx s3（px_index_set；结果=val）
#define PXOP_SLICE   30  // a=dst, b=obj, c=3 连续槽基址[start,end,step]
#define PXOP_GETF    31  // a=dst, b=obj s2, c=N[n]
#define PXOP_SETF    32  // a=val 槽, b=obj s2, c=N[n]（px_field_set；结果=val）
#define PXOP_GETF_OPT 33 // a=dst, b=obj s2, c=N[n]（OptionalField：null→null）
#define PXOP_NEWLIST 34  // a=dst, b=连续槽基址, c=n
#define PXOP_NEWTUPLE 35 // a=dst, b=连续槽基址, c=n
#define PXOP_NEWDICT 36  // a=dst, b=连续槽基址, c=项数 n（槽占用 2n）
#define PXOP_NEWSTRUCT 37 // a=dst, b=类型名 N idx, c=字段值连续槽基址
#define PXOP_NEWENUM 38  // a=dst, b=类型名 N idx, c=变体 N idx
#define PXOP_LISTPUSH 39 // a=val, b=list 槽
// D 调用/返回
#define PXOP_CALL    40  // a=dst, b=callee 槽 s2, c=argc；参数=槽 s2+1..s2+argc 连续区
#define PXOP_CALLM   41  // a=dst, b=obj 槽 s2, c=N[n], fl=argc；方法调用（px_method 桥，args=槽 b+1..b+argc）
#define PXOP_CALLSELF 42 // a=dst, b=callee 槽 s2, c=argc（self=首参，现 call_with_self）
#define PXOP_TAILCALL 43 // b=callee 槽 s2, c=argc（复用当前帧，S3-C）
#define PXOP_RET     44  // a=src s（返回槽 s 值）
#define PXOP_RET0    45  // 返回 px_null()
// E 控制流/错误
#define PXOP_JMP     46  // b=off16
#define PXOP_JMPT    47  // a=槽 s, b=off16（真值跳）
#define PXOP_JMPF    48  // a=槽 s, b=off16（假值跳）
#define PXOP_TRY     49  // a=槽 s（D7：Result-Err→RET s；null→RET0；否则就地解包）
#define PXOP_FORCE   50  // a=槽 s（Err/null→px_error；否则解包）
#define PXOP_SRCLINE 51  // b=line（记录行号到帧，px_srcline 语义）
#define PXOP_HALT    52  // 顶层代码结束 / main 返回
// F 生成器/并发/桥（S3-B 起细化；多数映射既有 px_* C 桥，不重造）
#define PXOP_NEWGEN  53  // a=dst, b=seq 槽, c=transform/filter 连续 2 槽基址（S3-B）
#define PXOP_SPAWN   54  // a=dst, b=callee 槽, c=argc（S3-B：新线程跑 槽s2 函数）
#define PXOP_ENUMVAR 55  // a=dst, b=obj 槽（enum→px_str(variant)；非 enum→null；B3b match 用）
#define PXM_MAX      56

// ==================== 常量子（K 池） ====================
// 发射器按 kind 生成静态项；LOADK 时物化为 LXValue（str 需 strdup/常驻，
// BCModule 进程级常驻 → 直接引用发射器字符串字面量，不回收）。
// B3/B4：PXK_FUNC —— K 项 i=funcs 下标，LOADK 物化为 PX_FUNC(px_vm_entry, &funcs[i])
//   （闭包/生成器 lambda 等非全局注册函数的引用；run_module 注册全局时跳过 '<' 名）。
#define PXK_INT  0
#define PXK_FLT  1
#define PXK_STR  2
#define PXK_BOOL 3
#define PXK_NULL 4
#define PXK_FUNC 5
typedef struct {
    uint8_t kind;      // PXK_*
    int64_t i;         // PXK_INT
    double  f;         // PXK_FLT
    const char* s;     // PXK_STR
} PxK;

// ==================== 函数/模块元数据 ====================
// PxVMFunc 非 LXObject（进程级常驻，不参与 GC 回收，S3-D 才纳入标记根面）；
// 统一函数对象 PX_FUNC {name, fn=px_vm_entry, ctx=PxVMFunc*} → px_call 零改动（D2）。
typedef struct PxBCModule PxBCModule;
typedef struct {
    const char*     name;      // 函数名（"Type.method" / "main" / "fn@<file>" 等）
    int             arity;     // 必需参数数
    int             ndefault;  // 默认参数数（总参数 = arity + ndefault）
    int             nslots;    // 槽容量（参数 + cg 在该函数内 uid 峰值）
    const PxInst*   bc;        // 字节码数组
    int             nbc;       // 指令条数
    const PxBCModule* mod;     // 所属模块（K/N/G 池访问；发射器前向声明静态初值）
    // P2（闭包 cell）预留：upvalue 描述表指针
    const void*     upvals;
} PxVMFunc;

// B2：struct 类型元数据（NEWSTRUCT 运行时字段名来源；发射器按声明序收集）
typedef struct {
    const char*     name;      // 类型名
    const char**    fnames;    // 字段名表（声明序，nfields 项）
    int             nfields;
} PxStructDef;

struct PxBCModule {
    const char*     name;      // 模块名（主文件名去扩展）
    const PxK*      K;  int nK;   // 常量池（int/float/str/bool/null 统一）
    const char**    N;  int nN;   // 名字池（全局名/字段名/方法名/类型名）
    const char**    G;  int nG;   // 声明全局槽名（函数名/顶层 var）
    const PxVMFunc* funcs; int nfuncs;  // 函数表（含 Top 与全部用户函数）
    int             top_idx;          // Top（顶层代码）在 funcs 下标；-1=无
    const PxStructDef* structs; int nstructs;  // B2：struct 类型元数据表
};

// ==================== 显式帧栈 ====================
// D3：px→px 调用 push 帧不回 C 递归（深递归安全）；px↔C 交替才经 px_vm_entry
// 递归一层。D7：TRY 遇 Err 即 RET 传播 → 帧无需 err 字段（RET 即返回 err 值）。
typedef struct {
    const PxVMFunc* f;
    LXValue*        slots;     // 槽数组：slots[0..arity-1] 参数；其后局部+临时
    int             nslots;
    int             pc;        // 返回时恢复
    int             line;      // SRCLINE 最近行号（px_srcline 语义）
    int             ret_dst;   // A4：CALL 压帧 → 返回写 caller 帧槽号；-1=顶层
                               //   （返回给 px_vm_run_func 调用者 / px_vm_entry）
} PxFrame;

typedef struct {
    PxFrame* frames;           // 帧栈（动态增长）
    int      nframes;
    int      cap;
} PxVmState;

// ==================== API ====================

const char* px_op_name(int op);                 // 调试/未实现文案（越界返回 "?"）
PxVmState*  px_vm_state(void);                  // 线程局部 VM 状态（懒建，不销毁）

// D2 trampoline：与 LXFuncPtr 兼容 —— 统一函数对象 func.fn = px_vm_entry。
// ctx 必须是 PxVMFunc*（px_func 构造 PX_FUNC 时传入）。
LXValue px_vm_entry(LXValue* args, int nargs, void* ctx);

// 在 st 上运行函数 f（压帧→解释→弹帧→返回值）。递归入口（顶层/px_call 回调用）。
LXValue px_vm_run_func(PxVmState* st, const PxVMFunc* f, LXValue* args, int nargs);

// 运行模块顶层（Top bc：注册全局 + 顶层语句 + main 调用；A1+）。
LXValue px_vm_run_module(PxVmState* st, const PxBCModule* m);

// 便捷分配器（A0 自测/调试用；正式产物由发射器生成静态 PxVMFunc，不走堆）。
PxVMFunc* px_vm_new_func(const char* name, int arity, int ndefault, int nslots,
                         const PxInst* bc, int nbc);

#ifdef __cplusplus
}
#endif

#endif // PX_VM_H
