/* onnx_proto.c —— 零依赖 ONNX/protobuf 解析（M156 第 38 轮）
 *
 * 只依赖 libc。绝不 dlopen 任何外部库（对照 Go 侧 embedding_engine_onnx.go 依赖
 * onnxruntime .so + cgo）。解析面覆盖 transformer 编码器用到的 proto 子集：
 *   ModelProto → GraphProto → {NodeProto, TensorProto(initializer), ValueInfoProto}
 *   NodeProto → AttributeProto → {i,f,s,ints,floats,TensorProto}
 * 未知字段一律按 wire type 跳过（前向兼容：新 opset 加字段不影响本解析器）。
 */
#include "onnx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* ══════════════════ protobuf wire 解码 ══════════════════ */
typedef struct { const unsigned char* p; const unsigned char* end; } Pb;

static int pb_varint(Pb* b, uint64_t* out) {
    uint64_t v = 0; int shift = 0;
    while (b->p < b->end) {
        unsigned c = *b->p++;
        v |= (uint64_t)(c & 0x7Fu) << shift;
        if (!(c & 0x80u)) { *out = v; return 0; }
        shift += 7;
        if (shift >= 64) return -1;
    }
    return -1;  /* 截断 */
}

static int pb_tag(Pb* b, int* field, int* wire) {
    uint64_t k;
    if (pb_varint(b, &k) != 0) return -1;
    *field = (int)(k >> 3);
    *wire = (int)(k & 7u);
    return (*field == 0) ? -1 : 0;   /* field 0 非法（protobuf 规范） */
}

static int pb_skip(Pb* b, int wire) {
    uint64_t n;
    switch (wire) {
    case 0: return pb_varint(b, &n);
    case 1: if (b->end - b->p < 8) return -1; b->p += 8; return 0;
    case 2: if (pb_varint(b, &n) != 0) return -1;
            if ((uint64_t)(b->end - b->p) < n) return -1; b->p += (size_t)n; return 0;
    case 5: if (b->end - b->p < 4) return -1; b->p += 4; return 0;
    default: return -1;   /* 3/4 组（废弃）不支持 */
    }
}

static int pb_bytes(Pb* b, const unsigned char** d, size_t* n) {
    uint64_t len;
    if (pb_varint(b, &len) != 0) return -1;
    if ((uint64_t)(b->end - b->p) < len) return -1;
    *d = b->p; *n = (size_t)len; b->p += (size_t)len;
    return 0;
}

/* fixed32 / fixed64：显式按小端解释（不依赖宿主字节序） */
static int pb_f32(Pb* b, uint32_t* out) {
    if (b->end - b->p < 4) return -1;
    *out = (uint32_t)b->p[0] | ((uint32_t)b->p[1] << 8) |
           ((uint32_t)b->p[2] << 16) | ((uint32_t)b->p[3] << 24);
    b->p += 4; return 0;
}
static int pb_f64(Pb* b, uint64_t* out) {
    uint64_t v = 0; int i;
    if (b->end - b->p < 8) return -1;
    for (i = 7; i >= 0; i--) v = (v << 8) | b->p[i];
    b->p += 8; *out = v; return 0;
}

static char* dups(const unsigned char* d, size_t n) {
    char* s = (char*)malloc(n + 1);
    if (!s) return NULL;
    if (n) memcpy(s, d, n);
    s[n] = 0;
    return s;
}
/* 从 C 字面量复制：长度用 strlen 量，**不要手数**（手数过一次越界的教训） */
static char* dupz(const char* z) { return dups((const unsigned char*)z, strlen(z)); }

/* ══════════════════ 小工具 ══════════════════ */
typedef struct { unsigned char* p; size_t n, cap; } Buf;

static int buf_add(Buf* b, const void* d, size_t n) {
    if (n == 0) return 0;
    if (b->n + n > b->cap) {
        size_t c = b->cap ? b->cap : 256;
        unsigned char* t;
        while (c < b->n + n) c <<= 1;
        t = (unsigned char*)realloc(b->p, c);
        if (!t) return -1;
        b->p = t; b->cap = c;
    }
    memcpy(b->p + b->n, d, n);
    b->n += n;
    return 0;
}
static void buf_free(Buf* b) { free(b->p); b->p = NULL; b->n = b->cap = 0; }

/* 以**小端**写 w 字节 */
static int le_put(Buf* b, uint64_t v, int w) {
    unsigned char t[8]; int i;
    for (i = 0; i < w; i++) t[i] = (unsigned char)((v >> (8 * i)) & 0xFFu);
    return buf_add(b, t, (size_t)w);
}

/* ── 字符串缓冲（JSON 渲染用） ── */
typedef struct { char* p; size_t n, cap; } Sb;
static int sb_reserve(Sb* s, size_t extra) {
    if (s->n + extra + 1 > s->cap) {
        size_t c = s->cap ? s->cap : 1024;
        char* t;
        while (c < s->n + extra + 1) c <<= 1;
        t = (char*)realloc(s->p, c);
        if (!t) return -1;
        s->p = t; s->cap = c;
    }
    return 0;
}
static int sb_raw(Sb* s, const char* d, size_t n) {
    if (sb_reserve(s, n) != 0) return -1;
    memcpy(s->p + s->n, d, n); s->n += n; s->p[s->n] = 0; return 0;
}
static int sb_puts(Sb* s, const char* z) { return sb_raw(s, z, strlen(z)); }
static int sb_putc(Sb* s, char c) { return sb_raw(s, &c, 1); }
static int sb_addf(Sb* s, const char* fmt, ...) {
    char tmp[512]; va_list ap; int k;
    va_start(ap, fmt);
    k = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (k < 0) return -1;
    if ((size_t)k < sizeof(tmp)) return sb_raw(s, tmp, (size_t)k);
    /* 罕见：超 512 字节，动态来一次 */
    {
        char* big = (char*)malloc((size_t)k + 1);
        if (!big) return -1;
        va_start(ap, fmt);
        vsnprintf(big, (size_t)k + 1, fmt, ap);
        va_end(ap);
        k = sb_raw(s, big, (size_t)k);
        free(big);
        return k;
    }
}
/* JSON 字符串（转义控制字符；UTF-8 原样透传） */
static int sb_json(Sb* s, const char* p, size_t n) {
    size_t i;
    if (sb_putc(s, '"') != 0) return -1;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)p[i];
        switch (c) {
        case '"':  if (sb_puts(s, "\\\"") != 0) return -1; break;
        case '\\': if (sb_puts(s, "\\\\") != 0) return -1; break;
        case '\n': if (sb_puts(s, "\\n") != 0) return -1; break;
        case '\r': if (sb_puts(s, "\\r") != 0) return -1; break;
        case '\t': if (sb_puts(s, "\\t") != 0) return -1; break;
        default:
            if (c < 0x20) { if (sb_addf(s, "\\u%04x", c) != 0) return -1; }
            else if (sb_putc(s, (char)c) != 0) return -1;
        }
    }
    return sb_putc(s, '"');
}

/* ══════════════════ dtype 元信息 ══════════════════ */
int onnx_dtype_size(int dt) {
    switch (dt) {
    case ONNX_DT_FLOAT: case ONNX_DT_INT32: case ONNX_DT_UINT32: return 4;
    case ONNX_DT_UINT8: case ONNX_DT_INT8: case ONNX_DT_BOOL: return 1;
    case ONNX_DT_UINT16: case ONNX_DT_INT16: case ONNX_DT_FLOAT16:
    case ONNX_DT_BFLOAT16: return 2;
    case ONNX_DT_INT64: case ONNX_DT_UINT64: case ONNX_DT_DOUBLE: case ONNX_DT_STRING: return 8;
    default: return 0;
    }
}
const char* onnx_dtype_name(int dt) {
    switch (dt) {
    case ONNX_DT_FLOAT: return "float";
    case ONNX_DT_UINT8: return "uint8";
    case ONNX_DT_INT8: return "int8";
    case ONNX_DT_UINT16: return "uint16";
    case ONNX_DT_INT16: return "int16";
    case ONNX_DT_INT32: return "int32";
    case ONNX_DT_INT64: return "int64";
    case ONNX_DT_STRING: return "string";
    case ONNX_DT_BOOL: return "bool";
    case ONNX_DT_FLOAT16: return "float16";
    case ONNX_DT_DOUBLE: return "double";
    case ONNX_DT_UINT32: return "uint32";
    case ONNX_DT_UINT64: return "uint64";
    case ONNX_DT_BFLOAT16: return "bfloat16";
    default: return "undefined";
    }
}

/* ══════════════════ TensorProto ══════════════════ */
static int parse_tensor(const unsigned char* buf, size_t len, OnnxTensor* t) {
    Pb b = { buf, buf + len };
    int f, w, rc = -1, dtype = 0, ndim = 0;
    uint64_t u;
    Buf fd = { 0 }, i32 = { 0 }, i64 = { 0 }, f64 = { 0 }, u64b = { 0 };
    const unsigned char* raw = NULL; size_t raw_n = 0;
    int64_t dims[ONNX_MAXDIM];
    char* name = NULL;

    memset(t, 0, sizeof(*t));
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) goto done;
        switch (f) {
        case 1: /* repeated int64 dims */
            if (w == 0) {
                if (pb_varint(&b, &u) != 0) goto done;
                if (ndim >= ONNX_MAXDIM) goto done;
                dims[ndim++] = (int64_t)u;
            } else if (w == 2) {   /* packed */
                const unsigned char* d; size_t n; Pb p;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                p.p = d; p.end = d + n;
                while (p.p < p.end) {
                    if (pb_varint(&p, &u) != 0) goto done;
                    if (ndim >= ONNX_MAXDIM) goto done;
                    dims[ndim++] = (int64_t)u;
                }
            } else goto done;
            break;
        case 2: /* int32 data_type */
            if (w != 0) goto done;
            if (pb_varint(&b, &u) != 0) goto done;
            dtype = (int)(int32_t)(uint32_t)u;
            break;
        case 4: /* repeated float float_data (packed 时即小端 f32 流) */
            if (w == 5) { uint32_t x; if (pb_f32(&b, &x) != 0) goto done; if (le_put(&fd, x, 4) != 0) goto done; }
            else if (w == 2) {
                const unsigned char* d; size_t n, i;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                if (n % 4) goto done;
                for (i = 0; i < n; i += 4) {
                    uint32_t x = (uint32_t)d[i] | ((uint32_t)d[i+1] << 8) |
                                 ((uint32_t)d[i+2] << 16) | ((uint32_t)d[i+3] << 24);
                    if (le_put(&fd, x, 4) != 0) goto done;
                }
            } else goto done;
            break;
        case 5: /* repeated int32 int32_data */
            if (w == 0) { if (pb_varint(&b, &u) != 0) goto done; if (le_put(&i32, u, 4) != 0) goto done; }
            else if (w == 2) {
                const unsigned char* d; size_t n; Pb p;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                p.p = d; p.end = d + n;
                while (p.p < p.end) { if (pb_varint(&p, &u) != 0) goto done; if (le_put(&i32, u, 4) != 0) goto done; }
            } else goto done;
            break;
        case 7: /* repeated int64 int64_data */
            if (w == 0) { if (pb_varint(&b, &u) != 0) goto done; if (le_put(&i64, u, 8) != 0) goto done; }
            else if (w == 2) {
                const unsigned char* d; size_t n; Pb p;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                p.p = d; p.end = d + n;
                while (p.p < p.end) { if (pb_varint(&p, &u) != 0) goto done; if (le_put(&i64, u, 8) != 0) goto done; }
            } else goto done;
            break;
        case 8: /* string name */
            if (w != 2) goto done;
            { const unsigned char* d; size_t n; if (pb_bytes(&b, &d, &n) != 0) goto done; free(name); name = dups(d, n); if (!name) goto done; }
            break;
        case 9: /* bytes raw_data（规范即小端，原样持有） */
            if (w != 2) goto done;
            if (pb_bytes(&b, &raw, &raw_n) != 0) goto done;
            break;
        case 10: /* repeated double double_data */
            if (w == 1) { uint64_t v; if (pb_f64(&b, &v) != 0) goto done; if (le_put(&f64, v, 8) != 0) goto done; }
            else if (w == 2) {
                const unsigned char* d; size_t n; Pb p;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                p.p = d; p.end = d + n;
                while (p.p < p.end) { uint64_t v; if (pb_f64(&p, &v) != 0) goto done; if (le_put(&f64, v, 8) != 0) goto done; }
            } else goto done;
            break;
        case 11: /* repeated uint64 uint64_data */
            if (w == 0) { if (pb_varint(&b, &u) != 0) goto done; if (le_put(&u64b, u, 8) != 0) goto done; }
            else if (w == 2) {
                const unsigned char* d; size_t n; Pb p;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                p.p = d; p.end = d + n;
                while (p.p < p.end) { if (pb_varint(&p, &u) != 0) goto done; if (le_put(&u64b, u, 8) != 0) goto done; }
            } else goto done;
            break;
        default:
            if (pb_skip(&b, w) != 0) goto done;
        }
    }
    /* 组装 */
    t->name = name; t->dtype = dtype; t->ndim = ndim;
    if (ndim) memcpy(t->dims, dims, sizeof(int64_t) * (size_t)ndim);
    if (raw) {
        if (raw_n) {
            t->data = (unsigned char*)malloc(raw_n);
            if (!t->data) goto done;
            memcpy(t->data, raw, raw_n);
        }
        t->nbytes = raw_n;
    } else {
        const Buf* src = NULL;
        int elem_w = onnx_dtype_size(dtype);
        switch (dtype) {
        case ONNX_DT_FLOAT:  src = &fd;   break;
        case ONNX_DT_INT64:  src = &i64;  break;
        case ONNX_DT_INT32:  src = &i32;  break;
        case ONNX_DT_DOUBLE: src = &f64;  break;
        case ONNX_DT_UINT64: src = &u64b; break;
        case ONNX_DT_BOOL: case ONNX_DT_UINT8: case ONNX_DT_INT8:
        case ONNX_DT_UINT16: case ONNX_DT_INT16:
        case ONNX_DT_FLOAT16: case ONNX_DT_BFLOAT16:
            src = &i32;   /* 规范：窄类型（1/2 字节）在 int32_data 里每元素一个 int32 */
            break;
        default: break;
        }
        if (src && src->n) {
            if (src == &i32 && elem_w > 0 && elem_w < 4) {
                /* 取每元素**低 elem_w 字节**（小端）；直接拷贝 4 字节会得到 4 倍长度 */
                size_t nelem = src->n / 4, k;
                unsigned char* out = (unsigned char*)malloc(nelem * (size_t)elem_w);
                if (!out) goto done;
                for (k = 0; k < nelem; k++) memcpy(out + k * (size_t)elem_w, src->p + k * 4, (size_t)elem_w);
                t->data = out;
                t->nbytes = nelem * (size_t)elem_w;
            } else {
                t->data = (unsigned char*)malloc(src->n);
                if (!t->data) goto done;
                memcpy(t->data, src->p, src->n);
                t->nbytes = src->n;
            }
        }
    }
    rc = 0;
done:
    buf_free(&fd); buf_free(&i32); buf_free(&i64); buf_free(&f64); buf_free(&u64b);
    if (rc != 0) { free(name); free(t->data); memset(t, 0, sizeof(*t)); }
    return rc;
}

/* ══════════════════ AttributeProto ══════════════════ */
static int attr_push_int(OnnxAttr* a, int64_t v) {
    int64_t* t = (int64_t*)realloc(a->ints, sizeof(int64_t) * (size_t)(a->n_ints + 1));
    if (!t) return -1;
    a->ints = t; a->ints[a->n_ints++] = v; return 0;
}
static int attr_push_float(OnnxAttr* a, double v) {
    double* t = (double*)realloc(a->floats, sizeof(double) * (size_t)(a->n_floats + 1));
    if (!t) return -1;
    a->floats = t; a->floats[a->n_floats++] = v; return 0;
}

static int parse_attr(const unsigned char* buf, size_t len, OnnxAttr* a) {
    Pb b = { buf, buf + len };
    int f, w, rc = -1;
    uint64_t u;
    int saw_f = 0, saw_i = 0, saw_s = 0, saw_t = 0, saw_floats = 0, saw_ints = 0;

    memset(a, 0, sizeof(*a));
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) goto done;
        switch (f) {
        case 1: { const unsigned char* d; size_t n;
            if (w != 2) goto done;
            if (pb_bytes(&b, &d, &n) != 0) goto done;
            free(a->name); a->name = dups(d, n); if (!a->name) goto done;
            break; }
        case 2: { uint32_t x; float fx;
            if (w != 5) goto done;
            if (pb_f32(&b, &x) != 0) goto done;
            memcpy(&fx, &x, 4); a->f = (double)fx; saw_f = 1;
            break; }
        case 3: { int64_t v;
            if (w != 0) goto done;
            if (pb_varint(&b, &u) != 0) goto done;
            v = (int64_t)u; a->i = v; saw_i = 1;
            break; }
        case 4: { const unsigned char* d; size_t n;
            if (w != 2) goto done;
            if (pb_bytes(&b, &d, &n) != 0) goto done;
            free(a->s); a->s = dups(d, n); if (!a->s && n) goto done;
            a->s_len = n; saw_s = 1;
            break; }
        case 5: { const unsigned char* d; size_t n; OnnxTensor* t;
            if (w != 2) goto done;
            if (pb_bytes(&b, &d, &n) != 0) goto done;
            t = (OnnxTensor*)calloc(1, sizeof(OnnxTensor));
            if (!t) goto done;
            if (parse_tensor(d, n, t) != 0) { free(t); goto done; }
            free(a->t); a->t = t; saw_t = 1;
            break; }
        case 7: /* repeated float floats */
            if (w == 5) { uint32_t x; float fx; if (pb_f32(&b, &x) != 0) goto done; memcpy(&fx, &x, 4);
                          if (attr_push_float(a, (double)fx) != 0) goto done; saw_floats = 1; }
            else if (w == 2) {
                const unsigned char* d; size_t n, i;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                if (n % 4) goto done;
                for (i = 0; i < n; i += 4) {
                    uint32_t x = (uint32_t)d[i] | ((uint32_t)d[i+1] << 8) |
                                 ((uint32_t)d[i+2] << 16) | ((uint32_t)d[i+3] << 24);
                    float fx; memcpy(&fx, &x, 4);
                    if (attr_push_float(a, (double)fx) != 0) goto done;
                }
                saw_floats = 1;
            } else goto done;
            break;
        case 8: /* repeated int64 ints */
            if (w == 0) { if (pb_varint(&b, &u) != 0) goto done;
                          if (attr_push_int(a, (int64_t)u) != 0) goto done; saw_ints = 1; }
            else if (w == 2) {
                const unsigned char* d; size_t n; Pb p;
                if (pb_bytes(&b, &d, &n) != 0) goto done;
                p.p = d; p.end = d + n;
                while (p.p < p.end) { if (pb_varint(&p, &u) != 0) goto done;
                                      if (attr_push_int(a, (int64_t)u) != 0) goto done; }
                saw_ints = 1;
            } else goto done;
            break;
        case 20: { /* AttributeType */
            if (w != 0) goto done;
            if (pb_varint(&b, &u) != 0) goto done;
            a->type = (int)u;
            break; }
        default:
            if (pb_skip(&b, w) != 0) goto done;
        }
    }
    /* type 缺失（早期 exporter）→ 由已见字段推断（单值优先，其次列表） */
    if (a->type == ONNX_ATTR_UNDEFINED) {
        if (saw_t)           a->type = ONNX_ATTR_TENSOR;
        else if (saw_floats) a->type = ONNX_ATTR_FLOATS;
        else if (saw_ints)   a->type = ONNX_ATTR_INTS;
        else if (saw_s)      a->type = ONNX_ATTR_STRING;
        else if (saw_f)      a->type = ONNX_ATTR_FLOAT;
        else if (saw_i)      a->type = ONNX_ATTR_INT;
    }
    rc = 0;
done:
    if (rc != 0) {
        free(a->name); free(a->s); free(a->ints); free(a->floats);
        if (a->t) { free(a->t->data); free(a->t->name); free(a->t); }
        memset(a, 0, sizeof(*a));
    }
    return rc;
}

/* ══════════════════ NodeProto ══════════════════ */
static int push_str(char*** arr, int* n, const unsigned char* d, size_t len) {
    char** t = (char**)realloc(*arr, sizeof(char*) * (size_t)(*n + 1));
    if (!t) return -1;
    *arr = t;
    (*arr)[*n] = dups(d, len);
    if (!(*arr)[*n]) return -1;
    (*n)++;
    return 0;
}

static int parse_node(const unsigned char* buf, size_t len, OnnxNode* nd) {
    Pb b = { buf, buf + len };
    int f, w, rc = -1;
    memset(nd, 0, sizeof(*nd));
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) goto done;
        switch (f) {
        case 1: case 2: { const unsigned char* d; size_t n;
            if (w != 2) goto done;
            if (pb_bytes(&b, &d, &n) != 0) goto done;
            if (f == 1) { if (push_str(&nd->in, &nd->n_in, d, n) != 0) goto done; }
            else        { if (push_str(&nd->out, &nd->n_out, d, n) != 0) goto done; }
            break; }
        case 3: case 4: case 7: { const unsigned char* d; size_t n; char** slot;
            if (w != 2) goto done;
            if (pb_bytes(&b, &d, &n) != 0) goto done;
            slot = (f == 3) ? &nd->name : (f == 4) ? &nd->op_type : &nd->domain;
            free(*slot); *slot = dups(d, n); if (!*slot) goto done;
            break; }
        case 5: { const unsigned char* d; size_t n; OnnxAttr* t;
            if (w != 2) goto done;
            if (pb_bytes(&b, &d, &n) != 0) goto done;
            t = (OnnxAttr*)realloc(nd->attrs, sizeof(OnnxAttr) * (size_t)(nd->n_attr + 1));
            if (!t) goto done;
            nd->attrs = t;
            if (parse_attr(d, n, &nd->attrs[nd->n_attr]) != 0) goto done;
            nd->n_attr++;
            break; }
        default:
            if (pb_skip(&b, w) != 0) goto done;
        }
    }
    rc = 0;
done:
    if (rc != 0) {
        int i;
        for (i = 0; i < nd->n_in; i++) free(nd->in[i]);
        for (i = 0; i < nd->n_out; i++) free(nd->out[i]);
        free(nd->in); free(nd->out); free(nd->name); free(nd->op_type); free(nd->domain);
        for (i = 0; i < nd->n_attr; i++) {
            free(nd->attrs[i].name); free(nd->attrs[i].s);
            free(nd->attrs[i].ints); free(nd->attrs[i].floats);
            if (nd->attrs[i].t) { free(nd->attrs[i].t->data); free(nd->attrs[i].t->name); free(nd->attrs[i].t); }
        }
        free(nd->attrs);
        memset(nd, 0, sizeof(*nd));
    }
    return rc;
}

/* ══════════════════ ValueInfoProto / TypeProto ══════════════════ */
static int parse_type_proto(const unsigned char* buf, size_t len, OnnxValueInfo* vi) {
    Pb b = { buf, buf + len };
    int f, w;
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) return -1;
        if (f == 1 && w == 2) {          /* tensor_type */
            const unsigned char* d; size_t n; Pb p; int f2, w2;
            if (pb_bytes(&b, &d, &n) != 0) return -1;
            p.p = d; p.end = d + n;
            while (p.p < p.end) {
                if (pb_tag(&p, &f2, &w2) != 0) return -1;
                if (f2 == 1 && w2 == 0) {          /* elem_type */
                    uint64_t u;
                    if (pb_varint(&p, &u) != 0) return -1;
                    vi->dtype = (int)u;
                } else if (f2 == 2 && w2 == 2) {   /* shape */
                    const unsigned char* sd; size_t sn; Pb sp; int f3, w3;
                    if (pb_bytes(&p, &sd, &sn) != 0) return -1;
                    if (vi->ndim < 0) vi->ndim = 0;
                    sp.p = sd; sp.end = sd + sn;
                    while (sp.p < sp.end) {
                        if (pb_tag(&sp, &f3, &w3) != 0) return -1;
                        if (f3 == 1 && w3 == 2) {  /* Dimension */
                            const unsigned char* dd; size_t dn; Pb dp; int f4, w4;
                            int64_t dv = -1; char* dpar = NULL;
                            if (pb_bytes(&sp, &dd, &dn) != 0) return -1;
                            dp.p = dd; dp.end = dd + dn;
                            while (dp.p < dp.end) {
                                if (pb_tag(&dp, &f4, &w4) != 0) { free(dpar); return -1; }
                                if (f4 == 1 && w4 == 0) {
                                    uint64_t u;
                                    if (pb_varint(&dp, &u) != 0) { free(dpar); return -1; }
                                    dv = (int64_t)u;
                                } else if (f4 == 2 && w4 == 2) {
                                    const unsigned char* q; size_t qn;
                                    if (pb_bytes(&dp, &q, &qn) != 0) { free(dpar); return -1; }
                                    free(dpar); dpar = dups(q, qn);
                                    if (!dpar) return -1;
                                } else if (pb_skip(&dp, w4) != 0) { free(dpar); return -1; }
                            }
                            if (vi->ndim < ONNX_MAXDIM) {
                                vi->dims[vi->ndim] = dv;
                                vi->dim_param[vi->ndim] = dpar;
                                vi->ndim++;
                            } else {
                                /* 缺陷 154（本轮照出）：超 ONNX_MAXDIM **不能静默丢弃** ——
                                   静默丢弃 = 把"我没解析"伪装成"解析成功"。 */
                                free(dpar);
                                return -1;
                            }
                        } else if (pb_skip(&sp, w3) != 0) return -1;
                    }
                } else if (pb_skip(&p, w2) != 0) return -1;
            }
        } else if (pb_skip(&b, w) != 0) return -1;
    }
    return 0;
}

static int parse_value_info(const unsigned char* buf, size_t len, OnnxValueInfo* vi) {
    Pb b = { buf, buf + len };
    int f, w;
    memset(vi, 0, sizeof(*vi));
    vi->ndim = -1;
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) goto fail;
        if (f == 1 && w == 2) {
            const unsigned char* d; size_t n;
            if (pb_bytes(&b, &d, &n) != 0) goto fail;
            free(vi->name); vi->name = dups(d, n);
            if (!vi->name) goto fail;
        } else if (f == 2 && w == 2) {
            const unsigned char* d; size_t n;
            if (pb_bytes(&b, &d, &n) != 0) goto fail;
            if (parse_type_proto(d, n, vi) != 0) goto fail;
        } else if (pb_skip(&b, w) != 0) goto fail;
    }
    return 0;
fail:
    { int i; free(vi->name); for (i = 0; i < ONNX_MAXDIM; i++) free(vi->dim_param[i]); }
    memset(vi, 0, sizeof(*vi));
    return -1;
}

/* ══════════════════ GraphProto / ModelProto ══════════════════ */
static int parse_graph(const unsigned char* buf, size_t len, OnnxModel* m) {
    Pb b = { buf, buf + len };
    int f, w;
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) return -1;
        switch (f) {
        case 1: { const unsigned char* d; size_t n; OnnxNode* t;
            if (w != 2) return -1;
            if (pb_bytes(&b, &d, &n) != 0) return -1;
            t = (OnnxNode*)realloc(m->nodes, sizeof(OnnxNode) * (size_t)(m->n_node + 1));
            if (!t) return -1;
            m->nodes = t;
            if (parse_node(d, n, &m->nodes[m->n_node]) != 0) return -1;
            m->n_node++;
            break; }
        case 2: { const unsigned char* d; size_t n;
            if (w != 2) return -1;
            if (pb_bytes(&b, &d, &n) != 0) return -1;
            free(m->graph_name); m->graph_name = dups(d, n);
            if (!m->graph_name) return -1;
            break; }
        case 5: { const unsigned char* d; size_t n; OnnxTensor* t;
            if (w != 2) return -1;
            if (pb_bytes(&b, &d, &n) != 0) return -1;
            t = (OnnxTensor*)realloc(m->inits, sizeof(OnnxTensor) * (size_t)(m->n_init + 1));
            if (!t) return -1;
            m->inits = t;
            if (parse_tensor(d, n, &m->inits[m->n_init]) != 0) return -1;
            m->n_init++;
            break; }
        case 11: case 12: case 13: { const unsigned char* d; size_t n;
            OnnxValueInfo* t; int* cnt; OnnxValueInfo** arr;
            if (w != 2) return -1;
            if (pb_bytes(&b, &d, &n) != 0) return -1;
            if (f == 11)      { arr = &m->inputs;      cnt = &m->n_in; }
            else if (f == 12) { arr = &m->outputs;     cnt = &m->n_out; }
            else              { arr = &m->value_infos; cnt = &m->n_vi; }
            t = (OnnxValueInfo*)realloc(*arr, sizeof(OnnxValueInfo) * (size_t)(*cnt + 1));
            if (!t) return -1;
            *arr = t;
            if (parse_value_info(d, n, &(*arr)[*cnt]) != 0) return -1;
            (*cnt)++;
            break; }
        default:
            if (pb_skip(&b, w) != 0) return -1;
        }
    }
    return 0;
}

static int parse_opset(const unsigned char* buf, size_t len, int64_t* ver, char** dom) {
    Pb b = { buf, buf + len };
    int f, w;
    *ver = 0; *dom = NULL;
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) return -1;
        if (f == 1 && w == 2) {          /* domain */
            const unsigned char* d; size_t n;
            if (pb_bytes(&b, &d, &n) != 0) return -1;
            free(*dom); *dom = dups(d, n);
            if (!*dom) return -1;
        } else if (f == 2 && w == 0) {   /* version */
            uint64_t u;
            if (pb_varint(&b, &u) != 0) return -1;
            *ver = (int64_t)u;
        } else if (pb_skip(&b, w) != 0) return -1;
    }
    return 0;
}

static void free_model(OnnxModel* m) {
    int i, j;
    if (!m) return;
    for (i = 0; i < m->n_node; i++) {
        OnnxNode* nd = &m->nodes[i];
        for (j = 0; j < nd->n_in; j++) free(nd->in[j]);
        for (j = 0; j < nd->n_out; j++) free(nd->out[j]);
        free(nd->in); free(nd->out); free(nd->name); free(nd->op_type); free(nd->domain);
        for (j = 0; j < nd->n_attr; j++) {
            free(nd->attrs[j].name); free(nd->attrs[j].s);
            free(nd->attrs[j].ints); free(nd->attrs[j].floats);
            if (nd->attrs[j].t) { free(nd->attrs[j].t->data); free(nd->attrs[j].t->name); free(nd->attrs[j].t); }
        }
        free(nd->attrs);
    }
    free(m->nodes);
    for (i = 0; i < m->n_init; i++) { free(m->inits[i].name); free(m->inits[i].data); }
    free(m->inits);
    {
        OnnxValueInfo* groups[3]; int counts[3];
        groups[0] = m->inputs;      counts[0] = m->n_in;
        groups[1] = m->outputs;     counts[1] = m->n_out;
        groups[2] = m->value_infos; counts[2] = m->n_vi;
        for (i = 0; i < 3; i++)
            for (j = 0; j < counts[i]; j++) {
                int k;
                free(groups[i][j].name);
                for (k = 0; k < ONNX_MAXDIM; k++) free(groups[i][j].dim_param[k]);
            }
        free(m->inputs); free(m->outputs); free(m->value_infos);
    }
    for (i = 0; i < m->n_opset; i++) free(m->opset_domain[i]);
    free(m->opset_domain); free(m->opset_version);
    free(m->producer_name); free(m->producer_version); free(m->domain); free(m->graph_name);
    free(m);
}

OnnxModel* onnx_model_parse(const unsigned char* buf, size_t len, char** err) {
    Pb b = { buf, buf + len };
    OnnxModel* m = (OnnxModel*)calloc(1, sizeof(OnnxModel));
    int f, w, saw_graph = 0;
    if (err) *err = NULL;
    if (!m) { if (err) *err = dupz("内存不足"); return NULL; }
    if (!buf || len == 0) goto bad;
    while (b.p < b.end) {
        if (pb_tag(&b, &f, &w) != 0) goto bad;
        switch (f) {
        case 1:
            if (w != 0) goto bad;
            { uint64_t u; if (pb_varint(&b, &u) != 0) goto bad; m->ir_version = (int)u; }
            break;
        case 2: case 3: case 4: { const unsigned char* d; size_t n; char** slot;
            if (w != 2) goto bad;
            if (pb_bytes(&b, &d, &n) != 0) goto bad;
            slot = (f == 2) ? &m->producer_name : (f == 3) ? &m->producer_version : &m->domain;
            free(*slot); *slot = dups(d, n);
            if (!*slot) goto bad;
            break; }
        case 7: { const unsigned char* d; size_t n;
            if (w != 2) goto bad;
            if (pb_bytes(&b, &d, &n) != 0) goto bad;
            if (parse_graph(d, n, m) != 0) goto bad;
            saw_graph = 1;
            break; }
        case 8: { const unsigned char* d; size_t n;
            int64_t ver = 0; char* dom = NULL;
            int64_t* tv; char** td;
            if (w != 2) goto bad;
            if (pb_bytes(&b, &d, &n) != 0) goto bad;
            if (parse_opset(d, n, &ver, &dom) != 0) { free(dom); goto bad; }
            tv = (int64_t*)realloc(m->opset_version, sizeof(int64_t) * (size_t)(m->n_opset + 1));
            if (!tv) { free(dom); goto bad; }
            m->opset_version = tv;
            td = (char**)realloc(m->opset_domain, sizeof(char*) * (size_t)(m->n_opset + 1));
            if (!td) { free(dom); goto bad; }
            m->opset_domain = td;
            m->opset_domain[m->n_opset] = dom;
            m->opset_version[m->n_opset] = ver;
            m->n_opset++;
            break; }
        default:
            if (pb_skip(&b, w) != 0) goto bad;
        }
    }
    if (!saw_graph) {
        if (err) *err = dupz("缺少 graph 字段（非 ONNX 模型？）");
        free_model(m);
        return NULL;
    }
    return m;
bad:
    if (err) {
        /* 带**字节偏移**：定位非法字段的唯一起点（对拍时两侧都能报同一偏移） */
        char tmp[160];
        snprintf(tmp, sizeof(tmp), "protobuf 解析失败（截断或字段非法）@%lld",
                 (long long)(b.p - buf));
        *err = dupz(tmp);
    }
    free_model(m);
    return NULL;
}

void onnx_model_free(OnnxModel* m) { free_model(m); }

const OnnxTensor* onnx_find_init(const OnnxModel* m, const char* name) {
    int i;
    if (!m || !name) return NULL;
    for (i = 0; i < m->n_init; i++)
        if (m->inits[i].name && strcmp(m->inits[i].name, name) == 0) return &m->inits[i];
    return NULL;
}
const OnnxValueInfo* onnx_find_vi(const OnnxModel* m, const char* name) {
    int i;
    if (!m || !name) return NULL;
    for (i = 0; i < m->n_in; i++)      if (m->inputs[i].name && strcmp(m->inputs[i].name, name) == 0) return &m->inputs[i];
    for (i = 0; i < m->n_out; i++)     if (m->outputs[i].name && strcmp(m->outputs[i].name, name) == 0) return &m->outputs[i];
    for (i = 0; i < m->n_vi; i++)      if (m->value_infos[i].name && strcmp(m->value_infos[i].name, name) == 0) return &m->value_infos[i];
    return NULL;
}

/* ══════════════════ 结构摘要 JSON ══════════════════
 * 语言侧 onnx_info() 与对拍门**共用**这一渲染器 ⇒ 不存在"门和实现各写一套口径"。
 * initializer 不带数据（只 name/dtype/dims/nbytes），避免 90MB 模型刷出巨量文本。 */
static int sb_dims(Sb* s, int ndim, const int64_t* dims) {
    int i;
    if (sb_putc(s, '[') != 0) return -1;
    for (i = 0; i < ndim; i++)
        if (sb_addf(s, "%s%lld", i ? "," : "", (long long)dims[i]) != 0) return -1;
    return sb_putc(s, ']');
}
static int sb_vi(Sb* s, const OnnxValueInfo* vi) {
    int i;
    if (sb_putc(s, '{') != 0) return -1;
    if (sb_puts(s, "\"name\":") != 0 || sb_json(s, vi->name ? vi->name : "", vi->name ? strlen(vi->name) : 0) != 0) return -1;
    if (sb_addf(s, ",\"dtype\":\"%s\",\"dims\":[", onnx_dtype_name(vi->dtype)) != 0) return -1;
    for (i = 0; i < vi->ndim; i++) {
        if (i && sb_putc(s, ',') != 0) return -1;
        if (vi->dim_param[i]) { if (sb_json(s, vi->dim_param[i], strlen(vi->dim_param[i])) != 0) return -1; }
        else if (sb_addf(s, "%lld", (long long)vi->dims[i]) != 0) return -1;
    }
    if (sb_puts(s, "]}") != 0) return -1;
    return 0;
}
static const char* attr_type_name(int t) {
    switch (t) {
    case ONNX_ATTR_FLOAT: return "FLOAT";
    case ONNX_ATTR_INT: return "INT";
    case ONNX_ATTR_STRING: return "STRING";
    case ONNX_ATTR_TENSOR: return "TENSOR";
    case ONNX_ATTR_GRAPH: return "GRAPH";
    case ONNX_ATTR_FLOATS: return "FLOATS";
    case ONNX_ATTR_INTS: return "INTS";
    case ONNX_ATTR_STRINGS: return "STRINGS";
    default: return "UNDEFINED";
    }
}

char* onnx_model_info_json(const OnnxModel* m) {
    Sb s = { 0 };
    int i, j;
    if (!m) return NULL;
    if (sb_addf(&s, "{\"ir_version\":%d,\"producer\":", m->ir_version) != 0) goto fail;
    if (sb_json(&s, m->producer_name ? m->producer_name : "", m->producer_name ? strlen(m->producer_name) : 0) != 0) goto fail;
    if (sb_puts(&s, ",\"producer_version\":") != 0) goto fail;
    if (sb_json(&s, m->producer_version ? m->producer_version : "", m->producer_version ? strlen(m->producer_version) : 0) != 0) goto fail;
    if (sb_puts(&s, ",\"graph\":") != 0) goto fail;
    if (sb_json(&s, m->graph_name ? m->graph_name : "", m->graph_name ? strlen(m->graph_name) : 0) != 0) goto fail;

    if (sb_puts(&s, ",\"opset\":[") != 0) goto fail;
    for (i = 0; i < m->n_opset; i++) {
        if (i && sb_putc(&s, ',') != 0) goto fail;
        if (sb_puts(&s, "{\"domain\":") != 0) goto fail;
        if (sb_json(&s, m->opset_domain[i] ? m->opset_domain[i] : "",
                    m->opset_domain[i] ? strlen(m->opset_domain[i]) : 0) != 0) goto fail;
        if (sb_addf(&s, ",\"version\":%lld}", (long long)m->opset_version[i]) != 0) goto fail;
    }
    if (sb_puts(&s, "],\"inputs\":[") != 0) goto fail;
    for (i = 0; i < m->n_in; i++) {
        if (i && sb_putc(&s, ',') != 0) goto fail;
        if (sb_vi(&s, &m->inputs[i]) != 0) goto fail;
    }
    if (sb_puts(&s, "],\"outputs\":[") != 0) goto fail;
    for (i = 0; i < m->n_out; i++) {
        if (i && sb_putc(&s, ',') != 0) goto fail;
        if (sb_vi(&s, &m->outputs[i]) != 0) goto fail;
    }
    if (sb_puts(&s, "],\"initializers\":[") != 0) goto fail;
    for (i = 0; i < m->n_init; i++) {
        const OnnxTensor* t = &m->inits[i];
        if (i && sb_putc(&s, ',') != 0) goto fail;
        if (sb_puts(&s, "{\"name\":") != 0) goto fail;
        if (sb_json(&s, t->name ? t->name : "", t->name ? strlen(t->name) : 0) != 0) goto fail;
        if (sb_addf(&s, ",\"dtype\":\"%s\",\"dims\":", onnx_dtype_name(t->dtype)) != 0) goto fail;
        if (sb_dims(&s, t->ndim, t->dims) != 0) goto fail;
        if (sb_addf(&s, ",\"nbytes\":%llu}", (unsigned long long)t->nbytes) != 0) goto fail;
    }
    if (sb_puts(&s, "],\"value_infos\":[") != 0) goto fail;
    for (i = 0; i < m->n_vi; i++) {
        if (i && sb_putc(&s, ',') != 0) goto fail;
        if (sb_vi(&s, &m->value_infos[i]) != 0) goto fail;
    }
    if (sb_puts(&s, "],\"nodes\":[") != 0) goto fail;
    for (i = 0; i < m->n_node; i++) {
        const OnnxNode* nd = &m->nodes[i];
        if (i && sb_putc(&s, ',') != 0) goto fail;
        if (sb_puts(&s, "{\"op\":") != 0) goto fail;
        if (sb_json(&s, nd->op_type ? nd->op_type : "", nd->op_type ? strlen(nd->op_type) : 0) != 0) goto fail;
        if (sb_puts(&s, ",\"name\":") != 0) goto fail;
        if (sb_json(&s, nd->name ? nd->name : "", nd->name ? strlen(nd->name) : 0) != 0) goto fail;
        if (sb_puts(&s, ",\"domain\":") != 0) goto fail;
        if (sb_json(&s, nd->domain ? nd->domain : "", nd->domain ? strlen(nd->domain) : 0) != 0) goto fail;
        if (sb_puts(&s, ",\"in\":[") != 0) goto fail;
        for (j = 0; j < nd->n_in; j++) {
            if (j && sb_putc(&s, ',') != 0) goto fail;
            if (sb_json(&s, nd->in[j], strlen(nd->in[j])) != 0) goto fail;
        }
        if (sb_puts(&s, "],\"out\":[") != 0) goto fail;
        for (j = 0; j < nd->n_out; j++) {
            if (j && sb_putc(&s, ',') != 0) goto fail;
            if (sb_json(&s, nd->out[j], strlen(nd->out[j])) != 0) goto fail;
        }
        if (sb_puts(&s, "],\"attrs\":[") != 0) goto fail;
        for (j = 0; j < nd->n_attr; j++) {
            const OnnxAttr* a = &nd->attrs[j];
            int k;
            if (j && sb_putc(&s, ',') != 0) goto fail;
            if (sb_puts(&s, "{\"name\":") != 0) goto fail;
            if (sb_json(&s, a->name ? a->name : "", a->name ? strlen(a->name) : 0) != 0) goto fail;
            if (sb_addf(&s, ",\"type\":\"%s\"", attr_type_name(a->type)) != 0) goto fail;
            switch (a->type) {
            case ONNX_ATTR_INT:
                if (sb_addf(&s, ",\"i\":%lld", (long long)a->i) != 0) goto fail;
                break;
            case ONNX_ATTR_FLOAT:
                if (sb_addf(&s, ",\"f\":%.9g", a->f) != 0) goto fail;
                break;
            case ONNX_ATTR_STRING:
                if (sb_puts(&s, ",\"s\":") != 0) goto fail;
                if (sb_json(&s, a->s ? a->s : "", a->s_len) != 0) goto fail;
                break;
            case ONNX_ATTR_INTS:
                if (sb_puts(&s, ",\"ints\":[") != 0) goto fail;
                for (k = 0; k < a->n_ints; k++)
                    if (sb_addf(&s, "%s%lld", k ? "," : "", (long long)a->ints[k]) != 0) goto fail;
                if (sb_putc(&s, ']') != 0) goto fail;
                break;
            case ONNX_ATTR_FLOATS:
                if (sb_puts(&s, ",\"floats\":[") != 0) goto fail;
                for (k = 0; k < a->n_floats; k++)
                    if (sb_addf(&s, "%s%.9g", k ? "," : "", a->floats[k]) != 0) goto fail;
                if (sb_putc(&s, ']') != 0) goto fail;
                break;
            case ONNX_ATTR_TENSOR:
                if (a->t) {
                    if (sb_puts(&s, ",\"t\":{\"dtype\":\"") != 0) goto fail;
                    if (sb_puts(&s, onnx_dtype_name(a->t->dtype)) != 0) goto fail;
                    if (sb_puts(&s, "\",\"dims\":") != 0) goto fail;
                    if (sb_dims(&s, a->t->ndim, a->t->dims) != 0) goto fail;
                    if (sb_addf(&s, ",\"nbytes\":%llu}", (unsigned long long)a->t->nbytes) != 0) goto fail;
                }
                break;
            default:
                break;
            }
            if (sb_putc(&s, '}') != 0) goto fail;
        }
        if (sb_puts(&s, "]}") != 0) goto fail;
    }
    if (sb_puts(&s, "]}") != 0) goto fail;
    return s.p;
fail:
    free(s.p);
    return NULL;
}
