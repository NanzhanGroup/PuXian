// ============================================================
// arch_x86_64.h —— x86_64 GC 保守扫描（M67-S4）
// ------------------------------------------------------------
// 从 runtime.c 原 #if __x86_64__ 分支迁出（gc_scan_stack / gc_scan_registers /
// gc_scan_thread_stack 三处），行为零变化。glibc/musl 通用
// （gregs[REG_*] 索引为 x86_64 ucontext ABI 常量）。
// ============================================================
#ifndef PX_ARCH_X86_64_H
#define PX_ARCH_X86_64_H

#include <stdint.h>
#include <ucontext.h>

// 当前线程栈指针：直接读 RSP（M57-S4：musl 无 getcontext → 内联汇编跨 glibc/musl）
static inline uintptr_t arch_read_sp(void) {
    uintptr_t rsp;
    __asm__ __volatile__("movq %%rsp, %0" : "=r"(rsp));
    return rsp;
}

// 扫描暂停线程寄存器：只扫数据指针候选寄存器（含 caller/callee-saved，
// 跳过 rip/rsp —— sp 由 arch_uc_sp 单独走线程栈扫描）
static inline void arch_scan_registers(ucontext_t* uc,
                                       void (*mark)(void* ctx, uintptr_t w),
                                       void* ctx) {
    greg_t* regs = uc->uc_mcontext.gregs;
    static const int reg_ids[] = {
        REG_RAX, REG_RBX, REG_RCX, REG_RDX, REG_RSI, REG_RDI,
        REG_RBP, REG_R8, REG_R9, REG_R10, REG_R11, REG_R12,
        REG_R13, REG_R14, REG_R15
    };
    for (unsigned i = 0; i < sizeof(reg_ids) / sizeof(reg_ids[0]); i++) {
        mark(ctx, (uintptr_t)regs[reg_ids[i]]);
    }
}

// 暂停线程栈扫描起点 SP（ucontext gregs[REG_RSP]）
static inline uintptr_t arch_uc_sp(ucontext_t* uc) {
    return (uintptr_t)uc->uc_mcontext.gregs[REG_RSP];
}

#endif /* PX_ARCH_X86_64_H */
