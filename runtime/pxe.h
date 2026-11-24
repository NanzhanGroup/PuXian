// ============================================================
// M442-S2: runtime/pxe.h —— .pxe（PXE 序列化模块）加载器公开接口
// ------------------------------------------------------------
// 定位：静态产物是默认路径；.pxe 是 opt-in 的「共享宿主 runtime」路径。
//   一份 runtime 加载任意多模块（M436-3 多 worker 场景需要）。
// 与 selfhost/bc_pxe.px 一一对应：那边写，这边读（格式见 M442.md §2）。
// 安全边界：加载前全量校验（V1–V6），**不做尽力修复** —— 校验失败一律拒收。
// ============================================================
#ifndef PX_PXE_H
#define PX_PXE_H

#include <stddef.h>
#include <stdint.h>

#include "vm.h"

#define PXE_VER        2
#define PXE_HDR        96
#define PXE_MAX_SEGS   8

// 返回码（pxe_rc_name 给人类可读名）
#define PXE_OK          0
#define PXE_EVER       2   // 版本不符
#define PXE_ELAYOUT     3   // 头/段/尺寸不自洽
#define PXE_ECODE       4   // 指令非法（op 未知 / 跳转越界 / 池下标越界）
#define PXE_ESTR        5   // 字符串偏移越界或未 NUL 结尾
#define PXE_ECHECK      6   // 校验和不符（crc32 或 sha256）
#define PXE_EIO         7   // 读文件失败
#define PXE_ENOMEM      8   // 内存不足

typedef struct {
    int  rc;
    char msg[256];
} PxeErr;

// 加载结果：m 为首字段 ⇒ 可 (PxBCModule*)pm 直接用；pxe_free 释放全部。
typedef struct {
    PxBCModule     m;
    unsigned char* buf;         // 文件内容（字符串池指针指向它，必须存活）
    void*          blocks[PXE_MAX_SEGS];
    int            nblocks;
} PxeModule;

// 加载入口。成功返回 PXE_OK；失败返回码写入 err（err 可为 NULL）。
int         pxe_load(const char* path, PxeModule* out, PxeErr* err);
// 内存版本（同一份校验逻辑；负控门用它对字节做变异，不必落盘）
int         pxe_load_mem(const unsigned char* data, size_t len, PxeModule* out, PxeErr* err);
void        pxe_free(PxeModule* pm);
const char* pxe_rc_name(int rc);
// 摘要打印（pxvm --info / 门用）
void        pxe_info(const PxeModule* pm);

#endif
