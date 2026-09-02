// 普贤 (PuXian) C 运行时 — QPACK 动态表 + SETTINGS（M49）对外接口
// ------------------------------------------------------------
// 语言层绑定注册入口 + 解码器流指令编解码辅助（供 capability 字节精确断言）。
#ifndef PX_RUNTIME_H3_QPACK_DYN_H
#define PX_RUNTIME_H3_QPACK_DYN_H
#include "runtime.h"

// 注册语言层 API（runtime_h3.c px_register_h3 内调用或 runtime.c 直调）
void px_register_h3_qpack_dyn(void);

// 解码器流指令（RFC 9204 §4.4）构造，返回字节数或 -1（cap 不足）：
int px_qd_dec_inst_section_ack(uint8_t* out, int cap, int64_t stream_id);
int px_qd_dec_inst_stream_cancel(uint8_t* out, int cap, int64_t stream_id);
int px_qd_dec_inst_inc(uint8_t* out, int cap, int64_t inc);

#endif /* PX_RUNTIME_H3_QPACK_DYN_H */
