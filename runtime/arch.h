// ============================================================
// arch.h —— M67-S4：GC 保守扫描架构抽象层（qg-issue 07 阶段二）
// ------------------------------------------------------------
// 统一接口（GC 主逻辑不再见架构 #if 分支）：
//   arch_read_sp()                        —— 当前线程栈指针（内联汇编，跨 glibc/musl）
//   arch_scan_registers(uc, mark, ctx)    —— 扫描暂停线程 ucontext 的通用寄存器，
//                                            对每个寄存器 word 调 mark(ctx, word)
//                                            （mark 负责做对象地址识别 + GC 标记）
//   arch_uc_sp(uc)                        —— 从 ucontext 取 SP（线程栈扫描起点）
// 按架构分文件：
//   arch_x86_64.h   —— 通用 x86_64（原 runtime.c #if __x86_64__ 分支迁出，行为零变化）
//   arch_aarch64.h  —— aarch64（原 runtime.c #if __aarch64__ 分支迁出，行为零变化）
//   arch_armv7.h    —— armv7（armhf/EABI hard-float；树莓派 2/3、老工业盒子）
//   arch_riscv64.h  —— riscv64（香山/玄铁、VisionFive2、信创）
// 以后加架构只需新增 arch_<arch>.h + 本文件加一行 include 分支，GC 主逻辑不动。
// ============================================================
#ifndef PX_ARCH_H
#define PX_ARCH_H

#if defined(__aarch64__)
#include "arch_aarch64.h"
#elif defined(__x86_64__)
#include "arch_x86_64.h"
#elif defined(__arm__)
#include "arch_armv7.h"
#elif defined(__riscv) && (__riscv_xlen == 64)
#include "arch_riscv64.h"
#else
#error "PuXian runtime: 不支持的架构（GC 保守扫描需 arch_*.h：aarch64 / x86_64 / armv7 / riscv64）"
#endif

#endif /* PX_ARCH_H */
