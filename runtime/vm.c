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
// ret_dst：-1 = 顶层（RET 时返回 run_func 调用者）；≥0 = CALL 压帧，RET 时把
// 返回值写入 caller 帧的该槽（D3：px→px 调用 push 帧，返回值经槽回传）。
static PxFrame* vm_frame_push(PxVmState* st, const PxVMFunc* f,
                              LXValue* args, int nargs, int ret_dst) {
    if (st->nframes >= st->cap) {
        st->cap *= 2;
        st->frames = (PxFrame*)realloc(st->frames, (size_t)st->cap * sizeof(PxFrame));
    }
    PxFrame* fr = &st->frames[st->nframes++];
    memset(fr, 0, sizeof(*fr));
    fr->f = f;
    fr->nslots = f ? f->nslots : 0;
    fr->slots = (LXValue*)calloc((size_t)(fr->nslots ? fr->nslots : 1), sizeof(LXValue));
    fr->ret_dst = ret_dst;
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
LXValue px_vm_run_func(PxVmState* st, const PxVMFunc* f, LXValue* args, int nargs) {
    int base = st->nframes;                 // 入口帧前栈深
    vm_frame_push(st, f, args, nargs, -1);  // ret_dst=-1：返回给本函数调用者
    LXValue ret = px_null();
    int done = 0;
    PxFrame* fr = &st->frames[st->nframes - 1];

    while (!done) {
        // 帧可能因 CALL（A4 起）被推入/弹出，每次循环取当前帧
        fr = &st->frames[st->nframes - 1];
        const PxVMFunc* cf = fr->f;          // 当前帧所属函数
        px_srcfunc(cf->name);                // A5：同步 runtime 错误追踪（px_error 文案带函数名）
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
        case PXOP_RET: {
            LXValue v = fr->slots[in.a];
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
            LXValue fnv = fr->slots[in.b];
            int argc = in.c;
            int dst = in.a;
            LXValue* abuf = NULL;
            if (argc > 0) {
                int n = argc;
                if (in.b + 1 + n > fr->nslots) n = fr->nslots - (int)in.b - 1;
                if (n > 0) {
                    abuf = (LXValue*)malloc((size_t)n * sizeof(LXValue));
                    for (int i = 0; i < n; i++) abuf[i] = fr->slots[in.b + 1 + i];
                    argc = n;
                } else argc = 0;
            }
            if (fnv.type == PX_FUNC &&
                fnv.as.obj->as.func.fn == px_vm_entry) {
                PxVMFunc* cf = (PxVMFunc*)fnv.as.obj->as.func.ctx;
                if (argc < cf->arity) {      // 参数不足（默认参数 S3-B 补）
                    free(abuf);
                    px_error("VM %s:%d CALL %s 参数不足: 需 %d 给 %d",
                             fr->f->name, fr->line, cf->name, cf->arity, argc);
                    break;
                }
                vm_frame_push(st, cf, abuf, argc, dst);  // 压子帧，循环继续
                free(abuf);
            } else {
                LXValue r = px_call(fnv, abuf, argc);     // native/旧C/非函数
                free(abuf);
                fr = &st->frames[st->nframes - 1];        // 刷新（未 push，帧不变）
                if (dst < fr->nslots) fr->slots[dst] = r;
            }
            break;
        }
        case PXOP_SRCLINE:
            fr->line = (int)(int16_t)in.b;
            px_srcline(fr->line);            // A5：同步 runtime 错误追踪（行号）
            break;
        case PXOP_IMM:
            fr->slots[in.a] = px_int((int64_t)(int16_t)in.b);
            break;
        case PXOP_MOV:
            fr->slots[in.a] = fr->slots[in.b];
            break;
        case PXOP_GETG: {
            // A1：v1 经 px_get_global（D8 无锁化后置）；b=G idx
            const PxBCModule* m = cf->mod;
            if (!m || in.b >= (uint16_t)m->nG) {
                px_error("VM %s:%d GETG 全局越界 g=%d (nG=%d)",
                         cf->name, fr->line, in.b, m ? m->nG : -1);
            }
            fr->slots[in.a] = px_get_global(m->G[in.b]);
            break;
        }
        case PXOP_SETG: {
            // A1：v1 经 px_set_global；a=G idx，b=src 槽
            const PxBCModule* m = cf->mod;
            if (!m || in.a >= (uint16_t)m->nG) {
                px_error("VM %s:%d SETG 全局越界 g=%d (nG=%d)",
                         cf->name, fr->line, in.a, m ? m->nG : -1);
            }
            px_set_global(m->G[in.a], fr->slots[in.b]);
            break;
        }
        case PXOP_LOADK: {
            const PxBCModule* m = cf->mod;
            if (!m || in.b >= (uint16_t)m->nK) {
                px_error("VM %s: LOADK 常量越界 k=%d (nK=%d)",
                         cf->name, in.b, m ? m->nK : -1);
            }
            fr->slots[in.a] = vm_loadk(&m->K[in.b], m);
            break;
        }
        // ---- B 表：一元/二元运算（A2，语义=调现 px_* C 函数，错误由 px_* 保证）----
        case PXOP_NEG:    fr->slots[in.a] = px_neg(fr->slots[in.b]); break;
        case PXOP_NOT:    fr->slots[in.a] = px_not(fr->slots[in.b]); break;
        case PXOP_BITNOT: fr->slots[in.a] = px_bitnot(fr->slots[in.b]); break;
        case PXOP_ADD:    fr->slots[in.a] = px_add(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_SUB:    fr->slots[in.a] = px_sub(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_MUL:    fr->slots[in.a] = px_mul(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_DIV:    fr->slots[in.a] = px_div(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_IDIV:   fr->slots[in.a] = px_idiv(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_MOD:    fr->slots[in.a] = px_mod(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_POW:    fr->slots[in.a] = px_pow(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_EQ:     fr->slots[in.a] = px_eq(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_NE:     fr->slots[in.a] = px_ne(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_LT:     fr->slots[in.a] = px_lt(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_LE:     fr->slots[in.a] = px_le(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_GT:     fr->slots[in.a] = px_gt(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_GE:     fr->slots[in.a] = px_ge(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_BITAND: fr->slots[in.a] = px_bitand(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_BITOR:  fr->slots[in.a] = px_bitor(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_BITXOR: fr->slots[in.a] = px_bitxor(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_SHL:    fr->slots[in.a] = px_shl(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_SHR:    fr->slots[in.a] = px_shr(fr->slots[in.b], fr->slots[in.c]); break;
        case PXOP_SHRU:   fr->slots[in.a] = px_ushr(fr->slots[in.b], fr->slots[in.c]); break;

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
                for (int i = 0; i < cap; i++) tmp2[i] = fr->slots[in.b + i];
                fr->slots[in.a] = px_list_n(tmp2, cap);
                free(tmp2);
            } else fr->slots[in.a] = px_list_n(NULL, 0);
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
                for (int i = 0; i < cap; i++) tmp2[i] = fr->slots[in.b + i];
                fr->slots[in.a] = px_tuple(tmp2, cap);
                free(tmp2);
            } else fr->slots[in.a] = px_tuple(NULL, 0);
            break;
        }
        // NEWDICT：a=dst，b=连续槽基址，c=项数 n（槽 b..b+2n-1 为 k0,v0,k1,v1..）；
        //   仅字符串键入 dict（对齐 codegen：if (_k.type == PX_STR) px_dict_set）
        case PXOP_NEWDICT: {
            LXValue d = px_dict();
            int n = (int)in.c;
            for (int i = 0; i < n; i++) {
                int k0 = (int)in.b + 2 * i;
                if (k0 + 1 < fr->nslots && fr->slots[k0].type == PX_STR)
                    px_dict_set(d, fr->slots[k0].as.obj->as.str.data, fr->slots[k0 + 1]);
            }
            fr->slots[in.a] = d;
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
            for (int i = 0; i < nf; i++) { vals[i] = fr->slots[in.c + i]; fns[i] = (char*)sd->fnames[i]; }
            fr->slots[in.a] = px_struct(sd->name, fns, vals, nf);
            free(vals); free(fns);
            break;
        }
        // NEWENUM（B2）：a=dst，b=N 类型名 idx，c=N 变体名 idx → px_enum
        case PXOP_NEWENUM: {
            const PxBCModule* mm = cf->mod;
            if (!mm || in.b >= (uint16_t)mm->nN || in.c >= (uint16_t)mm->nN)
                px_error("VM %s:%d NEWENUM 名字越界 b=%d c=%d (nN=%d)",
                         cf->name, fr->line, in.b, in.c, mm ? mm->nN : -1);
            fr->slots[in.a] = px_enum(mm->N[in.b], mm->N[in.c]);
            break;
        }
        // NEWGEN（B3，M34 惰性生成器）：a=dst，b=seq 槽，c=2 连续槽基址
        //   [transform(PX_FUNC), filter(PX_FUNC|null)] → px_gen_lazy（同 codegen 单 for
        //   GenExp：elt 恒为 transform 闭包，cond 有则 filter 闭包）。槽越界容错（防御）。
        case PXOP_NEWGEN: {
            LXValue tf = px_null(), fl = px_null();
            if ((int)in.c >= 0 && (int)in.c < fr->nslots) tf = fr->slots[in.c];
            if ((int)in.c + 1 < fr->nslots) fl = fr->slots[in.c + 1];
            fr->slots[in.a] = px_gen_lazy(fr->slots[in.b], tf, fl);
            break;
        }
        case PXOP_LISTPUSH:  // a=val 槽，b=list 槽（值入列表尾）
            px_list_push(fr->slots[in.b], fr->slots[in.a]);
            break;
        case PXOP_INDEX:     // a=dst，b=obj，c=idx
            fr->slots[in.a] = px_index(fr->slots[in.b], fr->slots[in.c]);
            break;
        case PXOP_SETIDX:    // a=val 槽，b=obj，c=idx（赋值表达式结果=val）
            px_index_set(fr->slots[in.b], fr->slots[in.c], fr->slots[in.a]);
            break;
        case PXOP_SLICE:     // a=dst，b=obj，c=3 连续槽基址[start,end,step]
            if ((int)in.c + 2 < fr->nslots)
                fr->slots[in.a] = px_slice(fr->slots[in.b], fr->slots[in.c],
                                           fr->slots[in.c + 1], fr->slots[in.c + 2]);
            else px_error("VM %s:%d SLICE 槽越界 base=%d", cf->name, fr->line, in.c);
            break;
        case PXOP_GETF:      // a=dst，b=obj，c=N 名字 idx（px_field）
            if (cf->mod && in.c < cf->mod->nN)
                fr->slots[in.a] = px_field(fr->slots[in.b], cf->mod->N[in.c]);
            else px_error("VM %s:%d GETF 名字越界 n=%d", cf->name, fr->line, in.c);
            break;
        case PXOP_GETF_OPT: { // OptionalField：obj null→null，否则 px_field
            LXValue o = fr->slots[in.b];
            if (cf->mod && in.c < cf->mod->nN)
                fr->slots[in.a] = px_is_null(o) ? px_null()
                                                : px_field(o, cf->mod->N[in.c]);
            else px_error("VM %s:%d GETF_OPT 名字越界 n=%d", cf->name, fr->line, in.c);
            break;
        }
        case PXOP_SETF:      // a=val 槽，b=obj，c=N 名字 idx（px_field_set）
            if (cf->mod && in.c < cf->mod->nN)
                px_field_set(fr->slots[in.b], cf->mod->N[in.c], fr->slots[in.a]);
            else px_error("VM %s:%d SETF 名字越界 n=%d", cf->name, fr->line, in.c);
            break;
        // CALLM：方法调用桥（px_method obj.name(args..)，obj=槽 b，方法名=N[c]，
        //   fl=argc，实参=槽 b+1..b+argc；返回写槽 a）——px_method 语义=现 C 桥
        case PXOP_CALLM: {
            LXValue ov = fr->slots[in.b];
            int argc = (int)in.fl;
            LXValue* abuf = NULL;
            if (argc > 0) {
                int n = argc;
                if (in.b + 1 + n > fr->nslots) n = fr->nslots - (int)in.b - 1;
                if (n > 0) {
                    abuf = (LXValue*)malloc((size_t)n * sizeof(LXValue));
                    for (int i = 0; i < n; i++) abuf[i] = fr->slots[in.b + 1 + i];
                    argc = n;
                } else argc = 0;
            }
            LXValue r = px_null();
            if (cf->mod && in.c < cf->mod->nN)
                r = px_method(ov, cf->mod->N[in.c], abuf, argc);
            else px_error("VM %s:%d CALLM 名字越界 n=%d", cf->name, fr->line, in.c);
            free(abuf);
            if (in.a < fr->nslots) fr->slots[in.a] = r;
            break;
        }

        // ---- E 表：错误传播（A5，D7）----
        // TRY（?）：Result-Err → 就地返回 Err（RET 语义回传）；null → 返回 null；
        // Ok → 就地解包覆写槽。对齐 codegen err_tag 模型（函数尾仅转发，语义等价）。
        case PXOP_TRY: {
            LXValue v = fr->slots[in.a];
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
            fr->slots[in.a] = v;
            break;
        }
        // FORCE（!）：Result-Err → px_error；null → px_error；否则就地解包
        case PXOP_FORCE: {
            LXValue v = fr->slots[in.a];
            if (px_is_result(v)) {
                if (!px_result_ok(v))
                    px_error("force unwrap Err: %s", px_to_string(px_result_unwrap(v)));
                v = px_result_unwrap(v);
            }
            if (px_is_null(v)) px_error("force unwrap null");
            fr->slots[in.a] = v;
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

// ---- 运行模块顶层（A1：注册全局函数 + Top bc；main() 调用约定随 S3-B CALL 接入）----
LXValue px_vm_run_module(PxVmState* st, const PxBCModule* m) {
    if (!m) return px_null();
    if (m->top_idx < 0 || m->top_idx >= m->nfuncs) {
        px_error("VM 模块 %s 无 Top 函数", m->name ? m->name : "?");
        return px_null();
    }
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
