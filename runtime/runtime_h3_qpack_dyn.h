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

// ============ M51：纯 C 会话接口（runtime_h3.c 线上接线） ============
// QPACK 会话：会话 id 1..QD_MAX_SESS；cap = 本端 SETTINGS 的 QPACK_MAX_TABLE_CAPACITY。
int64_t px_qd_open(int64_t cap);                      // → 会话 id | 0
void    px_qd_close(int64_t id);
int  px_qd_enc(int64_t id, char* const* names, char* const* vals,
               int* nls, int* vls, int nf, uint8_t* sect, int scap); // → 字段段长 | -1
int  px_qd_take_enc(int64_t id, uint8_t* out, int cap);              // → eout 字节 | -1
int  px_qd_ingest(int64_t id, const uint8_t* p, int len);            // → 0 | -1
int  px_qd_dec(int64_t id, const uint8_t* p, int len,
               char*** names, char*** vals, int** nls, int** vls);   // → n | -1 非法 | -2 阻塞
int  px_qd_en_len(int64_t id);    // 本端编码表条目 | -1
int  px_qd_de_len(int64_t id);    // 对端镜像表条目 | -1
int  px_qd_ins(int64_t id);       // 已 ingest 插入数 | -1
int  px_qd_eout_len(int64_t id);  // eout 待发字节 | -1

#endif /* PX_RUNTIME_H3_QPACK_DYN_H */
