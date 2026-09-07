// 普贤 (PuXian) C 运行时库 — vm.c
// M89-S3-A0: 显式帧 + 平坦字节码 VM 骨架
// ------------------------------------------------------------
// 本文件 = S3-A A0 交付：PxVmState/PxFrame/PxVMFunc 生命周期 + px_vm_entry
// trampoline（D2，px_call 零改动兼容）+ 指令分发循环骨架。
// 已实现最小指令子集（自证解释循环可跑）：LOADK IMM MOV SRCLINE JMP JMPT
// JMPF RET RET0 HALT；其余 op 分发默认 px_error "指令未实现"（A1 起逐批）。
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
    };
    if (op < 0 || op >= PXM_MAX || !names[op]) return "?";
    return names[op];
}

// ---- 线程局部 VM 状态 ----
static __thread PxVmState* g_vm_state = NULL;

PxVmState* px_vm_state(void) {
    if (!g_vm_state) {
        g_vm_state = (PxVmState*)calloc(1, sizeof(PxVmState));
        g_vm_state->cap = 16;
        g_vm_state->frames = (PxFrame*)calloc(g_vm_state->cap, sizeof(PxFrame));
    }
    return g_vm_state;
}

// ---- 帧栈 ----
static PxFrame* vm_frame_push(PxVmState* st, const PxVMFunc* f,
                              LXValue* args, int nargs) {
    if (st->nframes >= st->cap) {
        st->cap *= 2;
        st->frames = (PxFrame*)realloc(st->frames, (size_t)st->cap * sizeof(PxFrame));
    }
    PxFrame* fr = &st->frames[st->nframes++];
    memset(fr, 0, sizeof(*fr));
    fr->f = f;
    fr->nslots = f ? f->nslots : 0;
    fr->slots = (LXValue*)calloc((size_t)(fr->nslots ? fr->nslots : 1), sizeof(LXValue));
    // 参数拷入 slots[0..nargs)：calloc 零值 = PX_NULL（type 0），缺省参数由
    // 发射器在帧内预填默认值，实参不足时覆盖（对齐现 (nargs>i)?args[i]:default）。
    if (args && nargs > 0) {
        int n = nargs < fr->nslots ? nargs : fr->nslots;
        memcpy(fr->slots, args, (size_t)n * sizeof(LXValue));
    }
    return fr;
}

static void vm_frame_pop(PxVmState* st) {
    if (st->nframes <= 0) return;
    PxFrame* fr = &st->frames[--st->nframes];
    free(fr->slots);
    fr->slots = NULL;
}

// ---- LOADK 常量物化 ----
static LXValue vm_loadk(const PxK* k) {
    switch (k->kind) {
    case PXK_INT:   return px_int(k->i);
    case PXK_FLT:   return px_float(k->f);
    case PXK_STR:   return px_str(k->s ? k->s : "");
    case PXK_BOOL:  return px_bool(k->i != 0);
    case PXK_NULL:
    default:        return px_null();
    }
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
LXValue px_vm_run_func(PxVmState* st, const PxVMFunc* f, LXValue* args, int nargs) {
    int base = st->nframes;                 // 入口帧前栈深
    vm_frame_push(st, f, args, nargs);
    LXValue ret = px_null();
    int done = 0;
    PxFrame* fr = &st->frames[st->nframes - 1];

    while (!done) {
        // 帧可能因 CALL（A4 起）被推入/弹出，每次循环取当前帧
        fr = &st->frames[st->nframes - 1];
        const PxVMFunc* cf = fr->f;          // 当前帧所属函数
        int pc = fr->pc;
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
        case PXOP_RET:
            ret = fr->slots[in.a];
            vm_frame_pop(st);
            if (st->nframes == base) done = 1;
            break;
        case PXOP_RET0:
            ret = px_null();
            vm_frame_pop(st);
            if (st->nframes == base) done = 1;
            break;
        case PXOP_SRCLINE:
            fr->line = (int)(int16_t)in.b;
            break;
        case PXOP_IMM:
            fr->slots[in.a] = px_int((int64_t)(int16_t)in.b);
            break;
        case PXOP_MOV:
            fr->slots[in.a] = fr->slots[in.b];
            break;
        case PXOP_LOADK: {
            const PxBCModule* m = cf->mod;
            if (!m || in.b >= (uint16_t)m->nK) {
                px_error("VM %s: LOADK 常量越界 k=%d (nK=%d)",
                         cf->name, in.b, m ? m->nK : -1);
            }
            fr->slots[in.a] = vm_loadk(&m->K[in.b]);
            break;
        }
        case PXOP_JMP:
            fr->pc += (int)(int16_t)in.b;   // off 相对下一条：目标=(pc+1)+off
            break;
        case PXOP_JMPT:
            if (px_is_truthy(fr->slots[in.a])) fr->pc += (int)(int16_t)in.b;
            break;
        case PXOP_JMPF:
            if (!px_is_truthy(fr->slots[in.a])) fr->pc += (int)(int16_t)in.b;
            break;
        default:
            // A1 起逐批实现；错误现场含函数名/行号/op 名，对齐 px_error 语义
            px_error("VM %s:%d 指令未实现: %s (op=%d)",
                     cf->name, fr->line, px_op_name(in.op), in.op);
            break;
        }
    }
    return ret;
}

// ---- D2 trampoline：统一函数对象 func.fn = px_vm_entry（ctx=PxVMFunc*）----
LXValue px_vm_entry(LXValue* args, int nargs, void* ctx) {
    PxVMFunc* f = (PxVMFunc*)ctx;
    if (!f) return px_null();
    return px_vm_run_func(px_vm_state(), f, args, nargs);
}

// ---- 运行模块顶层（A1+：注册全局 + Top bc + main 调用）----
LXValue px_vm_run_module(PxVmState* st, const PxBCModule* m) {
    if (!m) return px_null();
    if (m->top_idx < 0 || m->top_idx >= m->nfuncs) {
        px_error("VM 模块 %s 无 Top 函数", m->name ? m->name : "?");
        return px_null();
    }
    // A0：仅骨架占位 —— Top bc 的运行（全局注册/args/main 调用约定）随
    // A1 发射器首个 BCModule 落地。此处直接跑 Top 函数（参数 0）。
    const PxVMFunc* top = &m->funcs[m->top_idx];
    return px_vm_run_func(st, top, NULL, 0);
}
