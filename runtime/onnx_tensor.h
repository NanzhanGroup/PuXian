/* onnx_tensor.h —— 运行期张量（N 维视图 + 广播）+ 算子实现（M157，零依赖）
 *
 * 与解析层（onnx.h）的分工：
 *   OnnxTensor（onnx.h）  = **文件里的** initializer：规范化小端字节 + dims/dtype。
 *   OnnxT（本文件）       = **运行期的** 张量：算子的输入/输出，含形状与字节缓冲。
 * 两者用 onnx_t_from_init() 转换（拷贝，避免算子就越界改到模型权重）。
 *
 * 坐标约定：数据一律**行主序**（C 序，ONNX/NumPy 同）。dims 是最右侧维变化最快。
 *
 * 为什么自己写而不是调库：PuXian 的定位是零依赖 + CGO 无关；token-cache 的 Go 侧
 *   走 onnxruntime（dlopen 外部 .so，不带 build tag 时是空桩）。本模块要在
 *   **无任何外部 .so** 的前提下把 transformer 编码器跑出来。
 *
 * 数值口径（与 Go/Python 对拍时的关键）：
 *   · float 全程 **float32 累加**（与 onnxruntime 的 float32 kernel 同口径）；
 *   · erf 用「|x|<3 Maclaurin 级数 / |x|≥3 连分式 erfc」双路径，double 中间量，
 *     目标 max abs err ≲ 1e-15（门里对 math.erf 做 1001 点实测）。
 */
#ifndef PX_ONNX_TENSOR_H
#define PX_ONNX_TENSOR_H

#include "onnx.h"

#define ONNX_ERRC 200

/* ── 运行期张量 ── */
typedef struct {
    int            dtype;
    int            ndim;
    int64_t        dims[ONNX_MAXDIM];
    int64_t        nelem;
    size_t         nbytes;
    unsigned char* data;   /* 小端；len = nbytes */
} OnnxT;

/* ── 竞技场：中间张量统一释放（避免逐节点手写 free 漏掉错误分支） ── */
typedef struct { OnnxT** v; int n, cap; } OnnxArena;

void   onnx_arena_init(OnnxArena* a);
void   onnx_arena_free(OnnxArena* a);
OnnxT* onnx_arena_new(OnnxArena* a, int dtype, int ndim, const int64_t* dims);

/* ── 生命周期 ── */
int    onnx_t_esize(int dtype);          /* 元素字节数；不支持返回 0 */
OnnxT* onnx_t_new(int dtype, int ndim, const int64_t* dims);
void   onnx_t_free(OnnxT* t);
OnnxT* onnx_t_clone(const OnnxT* s);
OnnxT* onnx_t_from_init(const OnnxTensor* tp);   /* initializer → 运行期张量 */

/* ── 元素访问（小端）── */
double  onnx_t_getf(const OnnxT* t, int64_t i);
int64_t onnx_t_geti(const OnnxT* t, int64_t i);
void    onnx_t_setf(OnnxT* t, int64_t i, double v);
void    onnx_t_seti(OnnxT* t, int64_t i, int64_t v);

/* ── 形状工具 ── */
int  onnx_shape_eq(const OnnxT* a, const OnnxT* b);
int  onnx_bcast_shape(const int64_t* a, int na, const int64_t* b, int nb,
                      int64_t* out, int* nout);        /* NumPy 广播规则；0=不兼容 */
void onnx_strides(const int64_t* dims, int ndim, int64_t* st); /* 行主序元素步长 */
char* onnx_shape_json(const OnnxT* t);                 /* "[1,512,768]"（malloc） */

/* ── 算子表 ──
 * 统一签名：成功返回 0 并把新张量放进 *out（由竞技场持有）；失败返回 -1 并写 err。
 * 属性按 **名字 + 类型** 取（缺省值由各算子自己给）。 */
typedef int (*OnnxOpFn)(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                        const OnnxAttr* attrs, int n_attr, char* err, size_t errc);

typedef struct { const char* name; OnnxOpFn fn; } OnnxOpEntry;
const OnnxOpEntry* onnx_op_lookup(const char* name);
int                onnx_op_count(void);
const char*        onnx_op_name_at(int i);

/* 属性取值助手（导出给 exec 用） */
int64_t     onnx_attr_i(const OnnxAttr* a, int n, const char* name, int64_t def);
double      onnx_attr_f(const OnnxAttr* a, int n, const char* name, double def);
const char* onnx_attr_s(const OnnxAttr* a, int n, const char* name, size_t* len);
const int64_t* onnx_attr_is(const OnnxAttr* a, int n, const char* name, int* cnt);
const OnnxTensor* onnx_attr_t(const OnnxAttr* a, int n, const char* name);
const double* onnx_attr_fs(const OnnxAttr* a, int n, const char* name, int* cnt);

/* 数值助手（门里也直接调） */
double onnx_erf(double x);

/* ══════════════════════════ 执行器（onnx_exec.c） ══════════════════════════ */
typedef struct {
    OnnxT** outputs;    /* 按 model->outputs 顺序；由结果持有 */
    int     n_out;
    int     nodes_run;  /* 实际执行的节点数（不含 Constant 之外被跳过的） */
    int     n_ops;      /* 算子表大小（与 onnx_op_name_at 顺序一致） */
    int*    op_counts;  /* 每算子执行次数 */
} OnnxRunResult;

/* 跑一遍图。feed_names/feed_vals 是图输入（按名字给；未给的输入必须是 initializer，
 * 否则报错）。内部把 feed 与 initializer **拷贝**进竞技场 ⇒ 调用方保留所有权。 */
OnnxRunResult* onnx_exec_run(const OnnxModel* m, const char** feed_names, OnnxT** feed_vals,
                             int n_feed, char** err);
void  onnx_exec_result_free(OnnxRunResult* r);
char* onnx_exec_stats_json(const OnnxRunResult* r);   /* {"nodes":N,"by_op":{...}} */

/* 跑图但**只保留**指定输出名（其余中间结果仍在竞技场内，随结果一起释放） */
int   onnx_exec_find_output(const OnnxRunResult* r, const OnnxModel* m, const char* name);

#endif /* PX_ONNX_TENSOR_H */
