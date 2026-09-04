// ============================================================
// arch_armv7.h —— armv7（armhf / EABI hard-float）GC 保守扫描（M67-S4）
// ------------------------------------------------------------
// 目标：树莓派 2/3、老工业盒子（armv7l-linux-musleabihf）。
// musl arm sigcontext 布局（arch/arm/bits/sigcontext.h，glibc 同构）：
//   trap_no, error_code, oldmask,
//   arm_r0..arm_r10, arm_fp(r11), arm_ip(r12),   ← 连续 13 word（r0-r12）
//   arm_sp(r13), arm_lr(r14), arm_pc(r15), arm_cpsr, fault_address
// 扫描 r0-r12（连续）+ sp；lr/pc 非数据指针（pc 为指令地址、lr 为返回地址，
// 不会指向对象堆）不入根集（与原 x86_64/aarch64 只扫数据寄存器语义一致）。
// ============================================================
#ifndef PX_ARCH_ARMV7_H
#define PX_ARCH_ARMV7_H

#include <stdint.h>
#include <ucontext.h>

// 当前线程栈指针：直接读 SP（ARM 内联汇编）
static inline uintptr_t arch_read_sp(void) {
    uintptr_t sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp));
    return sp;
}

// 扫描暂停线程寄存器：r0-r12（arm_r0..arm_ip 连续）+ sp（arm_sp）
static inline void arch_scan_registers(ucontext_t* uc,
                                       void (*mark)(void* ctx, uintptr_t w),
                                       void* ctx) {
    const unsigned long* m = (const unsigned long*)&uc->uc_mcontext.arm_r0;
    for (int i = 0; i <= 12; i++) {   // arm_r0..arm_r10 + arm_fp(r11) + arm_ip(r12)
        mark(ctx, (uintptr_t)m[i]);
    }
    mark(ctx, (uintptr_t)uc->uc_mcontext.arm_sp);
}

// 暂停线程栈扫描起点 SP（ucontext arm_sp 字段）
static inline uintptr_t arch_uc_sp(ucontext_t* uc) {
    return (uintptr_t)uc->uc_mcontext.arm_sp;
}

#endif /* PX_ARCH_ARMV7_H */
