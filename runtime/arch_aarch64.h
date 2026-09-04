// ============================================================
// arch_aarch64.h —— aarch64 GC 保守扫描（M67-S4）
// ------------------------------------------------------------
// 从 runtime.c 原 #if __aarch64__ 分支迁出（gc_scan_stack / gc_scan_registers /
// gc_scan_thread_stack 三处），行为零变化。glibc/musl 同布局：
//   mcontext.regs[0..30]（x0-x30）+ 独立 sp 字段
// ============================================================
#ifndef PX_ARCH_AARCH64_H
#define PX_ARCH_AARCH64_H

#include <stdint.h>
#include <ucontext.h>

// 当前线程栈指针：直接读 SP（内联汇编，跨 glibc/musl）
static inline uintptr_t arch_read_sp(void) {
    uintptr_t sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp));
    return sp;
}

// 扫描暂停线程寄存器：x0-x30（regs[0..30]）+ sp（glibc/musl 同布局）
static inline void arch_scan_registers(ucontext_t* uc,
                                       void (*mark)(void* ctx, uintptr_t w),
                                       void* ctx) {
    const uintptr_t* regs = (const uintptr_t*)uc->uc_mcontext.regs;
    for (int i = 0; i <= 30; i++) {
        mark(ctx, regs[i]);
    }
    mark(ctx, (uintptr_t)uc->uc_mcontext.sp);
}

// 暂停线程栈扫描起点 SP（ucontext 独立 sp 字段）
static inline uintptr_t arch_uc_sp(ucontext_t* uc) {
    return (uintptr_t)uc->uc_mcontext.sp;
}

#endif /* PX_ARCH_AARCH64_H */
