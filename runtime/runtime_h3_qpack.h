// 普贤 (PuXian) C 运行时 — QPACK 完整 codec 对外接口（M48）
// ------------------------------------------------------------
// runtime_h3.c 通过本头调用 QPACK 编解码（M47 MVP 内联实现已迁移至此完整版）。
#ifndef PX_RUNTIME_H3_QPACK_H
#define PX_RUNTIME_H3_QPACK_H
#include "runtime.h"

// 编码字段段（2 字节前缀 + 字段行，静态表/Huffman 感知）。返回字节数或 -1。
int px_h3_qenc(uint8_t* out, LXValue* fields, int nf);
// 解码字段段 → list of [name,value] | null
LXValue px_h3_qdec(const uint8_t* p, int len);
// Huffman 纯 codec（M48，capability/互操作验证）：
LXValue bi_h3_huff(LXValue* args, int nargs, void* ctx);   // h3_huff(str)->bytes
LXValue bi_h3_unhuff(LXValue* args, int nargs, void* ctx); // h3_unhuff(bytes)->str|null

#endif /* PX_RUNTIME_H3_QPACK_H */
