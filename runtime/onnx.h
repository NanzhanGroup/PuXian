/* onnx.h —— 普贤零依赖 ONNX 子集（M156）：protobuf 解析 + 张量 + 算子 + 执行器
 *
 * 背景：token-cache 的 embedding 引擎在 Go 侧是 `embedding_engine_onnx.go`
 *   （//go:build embed_onnx）—— 依赖 `github.com/yalue/onnxruntime_go` 调
 *   `/usr/local/onnxruntime-linux-x64-1.20.1/lib/libonnxruntime.so`（外部 .so + cgo）。
 *   不带 build tag 时是空桩（生产二进制就是空桩）。PuXian 的定位是「零依赖、
 *   CGO 无关」，因此本模块**不 dlopen 任何外部库**：自己解析 .onnx（protobuf wire）
 *   并自己执行 transformer 编码器所需的算子子集。
 *
 * 分层（严格单向依赖，便于逐层对拍）：
 *   onnx_proto.c   字节 → 结构（ModelProto/GraphProto/NodeProto/AttributeProto/
 *                  TensorProto/ValueInfoProto）—— 本文件声明
 *   onnx_tensor.c  张量（N 维视图 + 广播 + 形状推断）
 *   onnx_exec.c    按拓扑序执行 + 算子实现
 *   runtime_onnx.c native 胶水（onnx_* 语言侧接口）
 *
 * 字节序：ONNX 规范规定 TensorProto 的 raw_data 与 float_data 等均为**小端**。
 *   本实现所有值一律**显式**按小端解释/写出（不依赖宿主字节序），
 *   因此在 riscv64/aarch64/armv7/x86_64（本仓四档目标）上行为一致。
 */
#ifndef PX_ONNX_H
#define PX_ONNX_H

#include <stdint.h>
#include <stddef.h>

/* ── ONNX TensorProto.DataType ── */
enum {
    ONNX_DT_UNDEFINED = 0, ONNX_DT_FLOAT = 1, ONNX_DT_UINT8 = 2, ONNX_DT_INT8 = 3,
    ONNX_DT_UINT16 = 4, ONNX_DT_INT16 = 5, ONNX_DT_INT32 = 6, ONNX_DT_INT64 = 7,
    ONNX_DT_STRING = 8, ONNX_DT_BOOL = 9, ONNX_DT_FLOAT16 = 10, ONNX_DT_DOUBLE = 11,
    ONNX_DT_UINT32 = 12, ONNX_DT_UINT64 = 13, ONNX_DT_BFLOAT16 = 16
};

/* ── AttributeProto.AttributeType ── */
enum {
    ONNX_ATTR_UNDEFINED = 0, ONNX_ATTR_FLOAT = 1, ONNX_ATTR_INT = 2,
    ONNX_ATTR_STRING = 3, ONNX_ATTR_TENSOR = 4, ONNX_ATTR_GRAPH = 5,
    ONNX_ATTR_SPARSE_TENSOR = 11, ONNX_ATTR_TYPE_PROTO = 13,
    ONNX_ATTR_FLOATS = 6, ONNX_ATTR_INTS = 7, ONNX_ATTR_STRINGS = 8,
    ONNX_ATTR_TENSORS = 9, ONNX_ATTR_GRAPHS = 10
};

#define ONNX_MAXDIM 8   /* 支持到 8 维（transformer 最高 4 维，留余量） */
#define ONNX_MAXRANK 8

/* ── TensorProto（initializer / Constant 的 value / 图常量） ──
 * 语义：`data` 是**规范化小端字节缓冲**，长度 = nbytes = prod(dims) × dtype_size。
 *   raw_data 直接用；float_data/int32_data/… 逐元素解出后按小端写入。 */
typedef struct {
    char*          name;
    int            dtype;
    int            ndim;
    int64_t        dims[ONNX_MAXDIM];
    size_t         nbytes;
    unsigned char* data;
} OnnxTensor;

/* ── AttributeProto（只保留会用到/需对拍的字段） ── */
typedef struct {
    char*     name;
    int       type;        /* AttributeType；缺失时由字段推断 */
    int64_t   i;
    double    f;
    char*     s;
    size_t    s_len;
    int64_t*  ints;
    int       n_ints;
    double*   floats;
    int       n_floats;
    OnnxTensor* t;         /* TENSOR 属性（Constant.value / ConstantOfShape.value） */
} OnnxAttr;

/* ── NodeProto ── */
typedef struct {
    char*     op_type;
    char*     name;
    char*     domain;
    int       n_in;
    char**    in;
    int       n_out;
    char**    out;
    int       n_attr;
    OnnxAttr* attrs;
} OnnxNode;

/* ── ValueInfoProto（图输入/输出/value_info：名字 + 元素类型 + 形状） ── */
typedef struct {
    char*  name;
    int    dtype;          /* 0 = 未知 */
    int    ndim;           /* -1 = 未定型 */
    int64_t dims[ONNX_MAXDIM];
    char*  dim_param[ONNX_MAXDIM];  /* 符号维（如 "batch"），无则 NULL */
} OnnxValueInfo;

/* ── ModelProto（子集） ── */
typedef struct {
    int    ir_version;
    char*  producer_name;
    char*  producer_version;
    char*  domain;
    char*  graph_name;
    int    n_node;   OnnxNode*      nodes;
    int    n_init;   OnnxTensor*    inits;
    int    n_in;     OnnxValueInfo* inputs;
    int    n_out;    OnnxValueInfo* outputs;
    int    n_vi;     OnnxValueInfo* value_infos;
    int    n_opset;  int64_t* opset_version;  char** opset_domain;
} OnnxModel;

/* ── 解析 API（onnx_proto.c） ──
 * 成功返回模型（调用方 free），失败返回 NULL 并 *err = malloc 的错误串。 */
OnnxModel* onnx_model_parse(const unsigned char* buf, size_t len, char** err);
void       onnx_model_free(OnnxModel* m);
int        onnx_model_opsets_echo(const OnnxModel* m, char* out, size_t cap);

const OnnxTensor*    onnx_find_init(const OnnxModel* m, const char* name);
const OnnxValueInfo* onnx_find_vi(const OnnxModel* m, const char* name);

const char* onnx_dtype_name(int dt);
int         onnx_dtype_size(int dt);   /* 字节数；未知返回 0 */

/* 结构摘要 JSON（onnx_info 的语言侧口径；双方共用一个渲染器避免口径漂移） */
char* onnx_model_info_json(const OnnxModel* m);

/* ── 张量 + 算子 + 执行器：**M157**（下一轮）──
 * 本轮（M156）只到「字节 → 结构 + 权重数据访问」：
 *   onnx_model_open / onnx_info / onnx_initializer / onnx_model_close
 * 张量（N 维视图/广播）与算子执行（MatMul/Softmax/Erf/LayerNorm/Gather/…）
 * 在 M157 落进 onnx_tensor.c / onnx_exec.c，届时本文件再补声明。
 * 这样切分是为了让「解析」这一层先被独立对拍证明（事实集 vs 独立参考实现），
 * 避免解析错误与算子错误混在一起难定位。 */

#endif /* PX_ONNX_H */
