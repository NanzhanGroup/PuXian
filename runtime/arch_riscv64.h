// ============================================================
// arch_riscv64.h —— riscv64 GC 保守扫描（M67-S4）
// ------------------------------------------------------------
// 目标：香山/玄铁、VisionFive2、信创方向（riscv64-linux-musl）。
// musl riscv64 mcontext 布局（bits/signal.h，RISC-V psABI 序，glibc 同构）：
//   typedef unsigned long __riscv_mc_gp_state[32];   // pc,ra,sp,gp,tp,
//   struct mcontext_t { __riscv_mc_gp_state __gregs; ... }
//   __gregs[0..31]：pc, ra, sp, gp, tp, t0-t2, s0-s1, a0-a7, s2-s11, t3-t6
//   （sp = __gregs[2]，psABI 固定序，跨 glibc/musl 一致）
// 扫描全部 32 word（pc 为指令地址，地址不可能命中对象堆，扫描无害；
//   与 x86_64/aarch64 只扫数据寄存器语义无差异 —— 保守扫描本就允许多余候选）。
// ============================================================
#ifndef PX_ARCH_RISCV64_H
#define PX_ARCH_RISCV64_H

#include <stdint.h>
#include <ucontext.h>

// 当前线程栈指针：直接读 sp（RISC-V 内联汇编 mv）
static inline uintptr_t arch_read_sp(void) {
    uintptr_t sp;
    __asm__ __volatile__("mv %0, sp" : "=r"(sp));
    return sp;
}

// 扫描暂停线程寄存器：mcontext.__gregs[0..31]（全部 32 通用寄存器，psABI 序）
static inline void arch_scan_registers(ucontext_t* uc,
                                       void (*mark)(void* ctx, uintptr_t w),
                                       void* ctx) {
    const unsigned long* m = (const unsigned long*)&uc->uc_mcontext.__gregs;
    for (int i = 0; i < 32; i++) {
        mark(ctx, (uintptr_t)m[i]);
    }
}

// 暂停线程栈扫描起点 SP（__gregs[2] = sp，RISC-V psABI 固定序）
static inline uintptr_t arch_uc_sp(ucontext_t* uc) {
    return (uintptr_t)uc->uc_mcontext.__gregs[2];
}

#endif /* PX_ARCH_RISCV64_H */
