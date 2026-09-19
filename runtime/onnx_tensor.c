/* onnx_tensor.c —— 运行期张量核心 + 形状/搬运类算子（M157）
 *
 * 本文件负责「张量的表示与搬家」：分配/访问/形状推断/广播步长，
 *   以及不涉及数值计算密集的算子（Reshape/Transpose/Concat/Gather/Unsqueeze/
 *   Squeeze/Slice/Expand/Tile/Split/Shape/Cast/Identity/Flatten/ConstantOfShape/
 *   Range/Triu）。
 * 数值密集算子（逐元素广播、MatMul、Softmax、归约、LayerNormalization）在 onnx_ops.c。
 */
#include "onnx_tensor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#error "onnx_tensor.c 假定小端宿主（本仓四档目标 aarch64/armv7/riscv64/x86_64 均为小端）"
#endif

/* ══════════════════════════ 生命周期 ══════════════════════════ */

int onnx_t_esize(int dt) {
    switch (dt) {
    case ONNX_DT_FLOAT: case ONNX_DT_INT32: case ONNX_DT_UINT32: return 4;
    case ONNX_DT_INT64: case ONNX_DT_UINT64: case ONNX_DT_DOUBLE:  return 8;
    case ONNX_DT_UINT8: case ONNX_DT_INT8: case ONNX_DT_BOOL:      return 1;
    case ONNX_DT_UINT16: case ONNX_DT_INT16:
    case ONNX_DT_FLOAT16: case ONNX_DT_BFLOAT16:                   return 2;
    default: return 0;
    }
}

OnnxT* onnx_t_new(int dtype, int ndim, const int64_t* dims) {
    OnnxT* t;
    int64_t n = 1, i;
    int es = onnx_t_esize(dtype);
    if (es == 0 || ndim < 0 || ndim > ONNX_MAXDIM) return NULL;
    for (i = 0; i < ndim; i++) { if (dims[i] < 0) return NULL; n *= dims[i]; }
    t = (OnnxT*)calloc(1, sizeof(OnnxT));
    if (!t) return NULL;
    t->dtype = dtype; t->ndim = ndim; t->nelem = n; t->nbytes = (size_t)n * (size_t)es;
    for (i = 0; i < ndim; i++) t->dims[i] = dims[i];
    t->data = (unsigned char*)calloc(1, t->nbytes ? t->nbytes : 1);   /* 0 元素也要非 NULL */
    if (!t->data) { free(t); return NULL; }
    return t;
}

void onnx_t_free(OnnxT* t) { if (t) { free(t->data); free(t); } }

OnnxT* onnx_t_clone(const OnnxT* s) {
    OnnxT* t = onnx_t_new(s->dtype, s->ndim, s->dims);
    if (!t) return NULL;
    memcpy(t->data, s->data, t->nbytes);
    return t;
}

OnnxT* onnx_t_from_init(const OnnxTensor* tp) {
    OnnxT* t;
    if (!tp) return NULL;
    t = onnx_t_new(tp->dtype, tp->ndim, tp->dims);
    if (!t) return NULL;
    if (tp->nbytes == t->nbytes && tp->data) memcpy(t->data, tp->data, t->nbytes);
    return t;
}

void onnx_arena_init(OnnxArena* a) { a->v = NULL; a->n = 0; a->cap = 0; }

OnnxT* onnx_arena_new(OnnxArena* a, int dtype, int ndim, const int64_t* dims) {
    OnnxT* t = onnx_t_new(dtype, ndim, dims);
    if (!t) return NULL;
    if (a->n == a->cap) {
        int c = a->cap ? a->cap * 2 : 64;
        OnnxT** nv = (OnnxT**)realloc(a->v, (size_t)c * sizeof(OnnxT*));
        if (!nv) { onnx_t_free(t); return NULL; }
        a->v = nv; a->cap = c;
    }
    a->v[a->n++] = t;
    return t;
}

void onnx_arena_free(OnnxArena* a) {
    int i;
    for (i = 0; i < a->n; i++) onnx_t_free(a->v[i]);
    free(a->v);
    a->v = NULL; a->n = 0; a->cap = 0;
}

/* ══════════════════════════ 元素访问（小端） ══════════════════════════ */

double onnx_t_getf(const OnnxT* t, int64_t i) {
    const unsigned char* p = t->data;
    switch (t->dtype) {
    case ONNX_DT_FLOAT:  { float f;   memcpy(&f, p + i * 4, 4); return (double)f; }
    case ONNX_DT_DOUBLE: { double d;  memcpy(&d, p + i * 8, 8); return d; }
    case ONNX_DT_INT64:  { int64_t v; memcpy(&v, p + i * 8, 8); return (double)v; }
    case ONNX_DT_UINT64: { uint64_t v;memcpy(&v, p + i * 8, 8); return (double)v; }
    case ONNX_DT_INT32:  { int32_t v; memcpy(&v, p + i * 4, 4); return (double)v; }
    case ONNX_DT_UINT32: { uint32_t v;memcpy(&v, p + i * 4, 4); return (double)v; }
    case ONNX_DT_INT8:   return (double)((signed char)p[i]);
    case ONNX_DT_UINT8:  return (double)p[i];
    case ONNX_DT_BOOL:   return p[i] ? 1.0 : 0.0;
    case ONNX_DT_INT16:  { int16_t v; memcpy(&v, p + i * 2, 2); return (double)v; }
    case ONNX_DT_UINT16: { uint16_t v;memcpy(&v, p + i * 2, 2); return (double)v; }
    default: return 0.0;
    }
}

int64_t onnx_t_geti(const OnnxT* t, int64_t i) {
    switch (t->dtype) {
    case ONNX_DT_INT64: { int64_t v; memcpy(&v, t->data + i * 8, 8); return v; }
    case ONNX_DT_INT32: case ONNX_DT_UINT32: {
        int32_t v; memcpy(&v, t->data + i * 4, 4); return (int64_t)v; }
    case ONNX_DT_INT16: case ONNX_DT_UINT16: {
        int16_t v; memcpy(&v, t->data + i * 2, 2); return (int64_t)v; }
    case ONNX_DT_INT8:  return (int64_t)((signed char)t->data[i]);
    case ONNX_DT_UINT8: return (int64_t)t->data[i];
    case ONNX_DT_BOOL:  return t->data[i] ? 1 : 0;
    default:            return (int64_t)onnx_t_getf(t, i);
    }
}

void onnx_t_setf(OnnxT* t, int64_t i, double v) {
    switch (t->dtype) {
    case ONNX_DT_FLOAT:  { float f = (float)v;   memcpy(t->data + i * 4, &f, 4); break; }
    case ONNX_DT_DOUBLE: { double d = v;         memcpy(t->data + i * 8, &d, 8); break; }
    case ONNX_DT_INT64:  { int64_t x = (int64_t)v; memcpy(t->data + i * 8, &x, 8); break; }
    case ONNX_DT_INT32:  { int32_t x = (int32_t)v; memcpy(t->data + i * 4, &x, 4); break; }
    case ONNX_DT_INT8:   { t->data[i] = (unsigned char)(signed char)(int64_t)v; break; }
    case ONNX_DT_UINT8:  { t->data[i] = (unsigned char)v; break; }
    case ONNX_DT_BOOL:   { t->data[i] = v != 0.0 ? 1 : 0; break; }
    case ONNX_DT_INT16:  { int16_t x = (int16_t)v; memcpy(t->data + i * 2, &x, 2); break; }
    default: break;
    }
}

void onnx_t_seti(OnnxT* t, int64_t i, int64_t v) { onnx_t_setf(t, i, (double)v); }

/* ══════════════════════════ 形状工具 ══════════════════════════ */

int onnx_shape_eq(const OnnxT* a, const OnnxT* b) {
    int i;
    if (a->ndim != b->ndim) return 0;
    for (i = 0; i < a->ndim; i++) if (a->dims[i] != b->dims[i]) return 0;
    return 1;
}

int onnx_bcast_shape(const int64_t* a, int na, const int64_t* b, int nb,
                     int64_t* out, int* nout) {
    int n = na > nb ? na : nb, i;
    for (i = 0; i < n; i++) {
        int ia = na - n + i, ib = nb - n + i;
        int64_t da = ia >= 0 ? a[ia] : 1, db = ib >= 0 ? b[ib] : 1;
        if (da == db) out[i] = da;
        else if (da == 1) out[i] = db;
        else if (db == 1) out[i] = da;
        else return 0;
    }
    *nout = n;
    return 1;
}

void onnx_strides(const int64_t* dims, int ndim, int64_t* st) {
    int i;
    int64_t acc = 1;
    for (i = ndim - 1; i >= 0; i--) { st[i] = acc; acc *= dims[i]; }
}

char* onnx_shape_json(const OnnxT* t) {
    char* s = (char*)malloc((size_t)t->ndim * 24 + 8);
    int i;
    size_t n = 0;
    if (!s) return NULL;
    n += (size_t)sprintf(s + n, "[");
    for (i = 0; i < t->ndim; i++) n += (size_t)sprintf(s + n, "%s%lld", i ? "," : "", (long long)t->dims[i]);
    sprintf(s + n, "]");
    return s;
}

/* ══════════════════════════ 属性助手 ══════════════════════════ */

static const OnnxAttr* find_attr(const OnnxAttr* a, int n, const char* name) {
    int i;
    for (i = 0; i < n; i++) if (a[i].name && strcmp(a[i].name, name) == 0) return &a[i];
    return NULL;
}

int64_t onnx_attr_i(const OnnxAttr* a, int n, const char* name, int64_t def) {
    const OnnxAttr* p = find_attr(a, n, name);
    return p ? p->i : def;
}
double onnx_attr_f(const OnnxAttr* a, int n, const char* name, double def) {
    const OnnxAttr* p = find_attr(a, n, name);
    return p ? p->f : def;
}
const char* onnx_attr_s(const OnnxAttr* a, int n, const char* name, size_t* len) {
    const OnnxAttr* p = find_attr(a, n, name);
    if (!p) return NULL;
    if (len) *len = p->s_len;
    return p->s;
}
const int64_t* onnx_attr_is(const OnnxAttr* a, int n, const char* name, int* cnt) {
    const OnnxAttr* p = find_attr(a, n, name);
    if (!p) { if (cnt) *cnt = 0; return NULL; }
    if (cnt) *cnt = p->n_ints;
    return p->ints;
}
const OnnxTensor* onnx_attr_t(const OnnxAttr* a, int n, const char* name) {
    const OnnxAttr* p = find_attr(a, n, name);
    return p ? p->t : NULL;
}
const double* onnx_attr_fs(const OnnxAttr* a, int n, const char* name, int* cnt) {
    const OnnxAttr* p = find_attr(a, n, name);
    if (!p) { if (cnt) *cnt = 0; return NULL; }
    if (cnt) *cnt = p->n_floats;
    return p->floats;
}

/* ══════════════════════════ erf（双路径，double 中间量） ══════════════════════════
 * |x| < 2.5 ：Maclaurin 级数  erf(x) = 2/√π Σ (-1)^n x^(2n+1)/(n!(2n+1))
 * |x| ≥ 2.5 ：erfc 连分式（Lentz 求值）
 *     erfc(x) = e^{-x²}/(√π) · 1/(x + (1/2)/(x + (1)/(x + (3/2)/(x + (2)/(x + ...))))
 *   即 f = b0 + a1/(b1 + a2/(b2 + ...))，b_k = x，a_k = k/2 ⇒ erfc = e^{-x²}/(√π·f)。
 * 用 double 中间量、最后落到 float32 输出；门里对 math.erf 做细网格实测。
 * 取值区间与精度：级数在 x=2.5 处最大项 ≈ 7.7（抵消损失约 1 位），连分式在 x≥2.5
 *   收敛约 30 项 ⇒ 两条路径都能到 1e-15 量级（实测见 examples/m157_onnx_exec）。 */
#define PX_SQRT_PI 1.7724538509055160273

double onnx_erf(double x) {
    double ax = fabs(x);
    if (ax < 2.5) {
        double x2 = x * x, xpow = x, fact = 1.0, sum = 0.0;
        int n;
        for (n = 0; n < 80; n++) {
            double add = xpow / (fact * (double)(2 * n + 1));
            sum += (n % 2 == 0) ? add : -add;
            xpow *= x2;
            fact *= (double)(n + 1);
            if (fabs(add) < 1e-19 * (fabs(sum) + 1e-300) && n > 8) break;
        }
        /* 级数含 x^(2n+1) ⇒ 负 x 自然给负值，**不能**再翻符号（M157 实测踩过） */
        /* 级数含 x^(2n+1) ⇒ 负 x 自然给负值，**不能**再翻符号（M157 实测踩过） */
        /* 级数含 x^(2n+1) ⇒ 负 x 自然给负值，**不能**再翻符号（M157 实测踩过） */
        return (2.0 / PX_SQRT_PI) * sum;
    }
    {   /* erfc 连分式（modified Lentz） */
        double tiny = 1e-300, f, C, D, delta, prev = 1e300;
        int k;
        f = ax; C = ax; D = 0.0;
        for (k = 1; k <= 400; k++) {
            double a = (double)k * 0.5;
            D = ax + a * D;  if (fabs(D) < tiny) D = tiny;  D = 1.0 / D;
            C = ax + a / C;  if (fabs(C) < tiny) C = tiny;
            delta = C * D;
            f *= delta;
            if (fabs(delta - 1.0) <= 1e-17 && k > 4) break;
            prev = f;
        }
        (void)prev;
        {
            double erfc = exp(-ax * ax) / (PX_SQRT_PI * f);
            return x > 0 ? 1.0 - erfc : erfc - 1.0;
        }
    }
}

/* ══════════════════════════ 结构类算子 ══════════════════════════
 * 统一签名见 onnx_tensor.h。均在失败时写 err 并返回 -1。 */
#define FAILF(...) do { if (err) snprintf(err, errc, __VA_ARGS__); return -1; } while (0)

static int copy_flat(OnnxT* out, const OnnxT* in) {
    if (out->nbytes == in->nbytes) { memcpy(out->data, in->data, out->nbytes); return 0; }
    return -1;
}

int onnx_op_identity(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                        const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    (void)attrs; (void)n_attr;
    if (n_in != 1) FAILF("Identity: 需要 1 个输入，得 %d", n_in);
    *out = onnx_arena_new(ar, in[0]->dtype, in[0]->ndim, in[0]->dims);
    if (!*out) FAILF("Identity: 分配失败");
    copy_flat(*out, in[0]);
    return 0;
}

int onnx_op_reshape(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                    const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    const OnnxT* sh;
    int64_t dims[ONNX_MAXDIM];
    int i, nd = 0, allowzero = (int)onnx_attr_i(attrs, n_attr, "allowzero", 0);
    int64_t known = 1, infer = -1;
    (void)ar;
    if (n_in < 2) FAILF("Reshape: 需要 2 个输入");
    sh = in[1];
    if (sh->ndim != 1) FAILF("Reshape: shape 必须是 1 维，得 %d 维", sh->ndim);
    nd = (int)sh->nelem;
    if (nd > ONNX_MAXDIM) FAILF("Reshape: 目标维度 %d > %d", nd, ONNX_MAXDIM);
    for (i = 0; i < nd; i++) {
        int64_t d = onnx_t_geti(sh, i);
        if (d == -1) { if (infer >= 0) FAILF("Reshape: 多个 -1"); infer = i; dims[i] = 1; }
        else if (d == 0 && !allowzero) { if (i >= in[0]->ndim) FAILF("Reshape: 0 维越界"); dims[i] = in[0]->dims[i]; known *= dims[i]; }
        else if (d < 0) FAILF("Reshape: 非法维度 %lld", (long long)d);
        else { dims[i] = d; known *= d; }
    }
    if (infer >= 0) {
        if (known == 0) FAILF("Reshape: 推断维时已知维为 0");
        if (in[0]->nelem % known) FAILF("Reshape: 元素数不整除（%lld / %lld）",
                                        (long long)in[0]->nelem, (long long)known);
        dims[infer] = in[0]->nelem / known;
    } else if (known != in[0]->nelem) FAILF("Reshape: 元素数不符（%lld ≠ %lld）",
                                            (long long)known, (long long)in[0]->nelem);
    *out = onnx_arena_new(ar, in[0]->dtype, nd, dims);
    if (!*out) FAILF("Reshape: 分配失败");
    copy_flat(*out, in[0]);
    return 0;
}

int onnx_op_transpose(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                      const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    const OnnxT* x;
    int64_t perm[ONNX_MAXDIM], odims[ONNX_MAXDIM] = {0}, istr[ONNX_MAXDIM], ostr[ONNX_MAXDIM];
    int cnt, i, nd;
    int64_t k;
    (void)ar;
    if (n_in != 1) FAILF("Transpose: 需要 1 个输入");
    x = in[0]; nd = x->ndim;
    const int64_t* p = onnx_attr_is(attrs, n_attr, "perm", &cnt);
    if (p && cnt > 0) {
        if (cnt != nd) FAILF("Transpose: perm 长度 %d ≠ 秩 %d", cnt, nd);
        for (i = 0; i < nd; i++) perm[i] = p[i];
    } else for (i = 0; i < nd; i++) perm[i] = nd - 1 - i;
    for (i = 0; i < nd; i++) {
        if (perm[i] < 0 || perm[i] >= nd) FAILF("Transpose: perm[%d]=%lld 越界", i, (long long)perm[i]);
        odims[i] = x->dims[perm[i]];
    }
    *out = onnx_arena_new(ar, x->dtype, nd, odims);
    if (!*out) FAILF("Transpose: 分配失败");
    onnx_strides(x->dims, nd, istr);
    onnx_strides(odims, nd, ostr);
    for (k = 0; k < x->nelem; k++) {
        int64_t rem = k, srcoff = 0;
        for (i = 0; i < nd; i++) {
            int64_t c = rem / ostr[i];
            rem %= ostr[i];
            srcoff += c * istr[perm[i]];
        }
        memcpy((*out)->data + k * onnx_t_esize(x->dtype),
               x->data + srcoff * onnx_t_esize(x->dtype), (size_t)onnx_t_esize(x->dtype));
    }
    return 0;
}

int onnx_op_concat(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                   const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", 0), odims[ONNX_MAXDIM], osum = 0;
    int i, j, nd, es;
    int64_t outer, inner, o;
    (void)ar;
    if (n_in < 1) FAILF("Concat: 至少 1 个输入");
    nd = in[0]->ndim;
    if (axis < 0) axis += nd;
    if (axis < 0 || axis >= nd) FAILF("Concat: axis %lld 越界（秩 %d）", (long long)axis, nd);
    for (i = 0; i < n_in; i++) {
        if (in[i]->ndim != nd) FAILF("Concat: 秩不一致（%d vs %d）", in[i]->ndim, nd);
        for (j = 0; j < nd; j++)
            if (j != (int)axis && in[i]->dims[j] != in[0]->dims[j])
                FAILF("Concat: 非拼接维 %d 不一致（%lld vs %lld）", j,
                      (long long)in[i]->dims[j], (long long)in[0]->dims[j]);
        osum += in[i]->dims[axis];
    }
    for (j = 0; j < nd; j++) odims[j] = in[0]->dims[j];
    odims[axis] = osum;
    *out = onnx_arena_new(ar, in[0]->dtype, nd, odims);
    if (!*out) FAILF("Concat: 分配失败");
    es = onnx_t_esize(in[0]->dtype);
    outer = 1; for (j = 0; j < (int)axis; j++) outer *= odims[j];
    inner = 1; for (j = (int)axis + 1; j < nd; j++) inner *= odims[j];
    {
        int64_t axoff = 0;
        for (i = 0; i < n_in; i++) {
            int64_t ax = in[i]->dims[axis];
            for (o = 0; o < outer; o++) {
                unsigned char* dst = (*out)->data + (size_t)(o * osum * inner + axoff * inner) * (size_t)es;
                const unsigned char* src = in[i]->data + (size_t)(o * ax * inner) * (size_t)es;
                memcpy(dst, src, (size_t)(ax * inner) * (size_t)es);
            }
            axoff += ax;
        }
    }
    return 0;
}

int onnx_op_shape(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                  const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t sdims[1];
    int i;
    (void)attrs; (void)n_attr;
    if (n_in != 1) FAILF("Shape: 需要 1 个输入");
    sdims[0] = in[0]->ndim;
    *out = onnx_arena_new(ar, ONNX_DT_INT64, 1, sdims);
    if (!*out) FAILF("Shape: 分配失败");
    for (i = 0; i < in[0]->ndim; i++) onnx_t_seti(*out, i, in[0]->dims[i]);
    return 0;
}

int onnx_op_cast(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                 const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t to = onnx_attr_i(attrs, n_attr, "to", 0);
    int64_t i;
    (void)ar;
    if (n_in != 1) FAILF("Cast: 需要 1 个输入");
    if (onnx_t_esize((int)to) == 0) FAILF("Cast: 不支持的目标类型 %lld", (long long)to);
    *out = onnx_arena_new(ar, (int)to, in[0]->ndim, in[0]->dims);
    if (!*out) FAILF("Cast: 分配失败");
    for (i = 0; i < in[0]->nelem; i++) {
        if (to == ONNX_DT_FLOAT || to == ONNX_DT_DOUBLE) onnx_t_setf(*out, i, onnx_t_getf(in[0], i));
        else onnx_t_seti(*out, i, to == ONNX_DT_BOOL ? (onnx_t_getf(in[0], i) != 0.0)
                                                     : onnx_t_geti(in[0], i));
    }
    return 0;
}

int onnx_op_unsqueeze(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                      const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t axes[ONNX_MAXDIM * 2], odims[ONNX_MAXDIM * 2];
    int na = 0, i, j, nd, cnt;
    const int64_t* p = onnx_attr_is(attrs, n_attr, "axes", &cnt);
    (void)ar;
    if (n_in < 1) FAILF("Unsqueeze: 需要数据输入");
    nd = in[0]->ndim;
    if (p && cnt > 0) { if (cnt > ONNX_MAXDIM * 2) FAILF("Unsqueeze: 轴过多"); for (i = 0; i < cnt; i++) axes[na++] = p[i]; }
    else {
        if (n_in < 2) FAILF("Unsqueeze: 需要 axes 输入（opset13+）或 axes 属性");
        if (in[1]->nelem > ONNX_MAXDIM * 2) FAILF("Unsqueeze: 轴过多");
        na = (int)in[1]->nelem;
        for (i = 0; i < na; i++) axes[i] = onnx_t_geti(in[1], i);
    }
    for (i = 0; i < na; i++) if (axes[i] < 0) axes[i] += nd + na;   /* 归一化（含新增维） */
    /* 排序 + 去重检查 */
    for (i = 0; i < na; i++)
        for (j = i + 1; j < na; j++)
            if (axes[i] > axes[j]) { int64_t t = axes[i]; axes[i] = axes[j]; axes[j] = t; }
    for (i = 0; i < na; i++) {
        if (axes[i] < 0 || axes[i] >= nd + na) FAILF("Unsqueeze: 轴 %lld 越界", (long long)axes[i]);
        if (i && axes[i] == axes[i - 1]) FAILF("Unsqueeze: 轴重复 %lld", (long long)axes[i]);
    }
    {
        int si = 0, di = 0;
        for (i = 0; i < nd + na; i++) {
            int is_new = 0;
            for (j = 0; j < na; j++) if (axes[j] == i) is_new = 1;
            odims[i] = is_new ? 1 : in[0]->dims[si++];
            di++;
        }
        (void)di;
    }
    *out = onnx_arena_new(ar, in[0]->dtype, nd + na, odims);
    if (!*out) FAILF("Unsqueeze: 分配失败");
    copy_flat(*out, in[0]);
    return 0;
}

int onnx_op_squeeze(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                    const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t axes[ONNX_MAXDIM], odims[ONNX_MAXDIM];
    int na = 0, i, j, nd, cnt;
    const int64_t* p = onnx_attr_is(attrs, n_attr, "axes", &cnt);
    (void)ar;
    if (n_in < 1) FAILF("Squeeze: 需要数据输入");
    nd = in[0]->ndim;
    if (p && cnt > 0) { for (i = 0; i < cnt; i++) axes[na++] = p[i]; }
    else if (n_in >= 2 && in[1]->nelem > 0) {
        if (in[1]->nelem > ONNX_MAXDIM) FAILF("Squeeze: 轴过多");
        na = (int)in[1]->nelem;
        for (i = 0; i < na; i++) axes[i] = onnx_t_geti(in[1], i);
    }
    for (i = 0; i < na; i++) if (axes[i] < 0) axes[i] += nd;
    {   int keep = nd - na, di = 0;
        for (j = 0; j < nd; j++) {
            int drop = 0;
            for (i = 0; i < na; i++) if (axes[i] == j) drop = 1;
            if (drop) { if (in[0]->dims[j] != 1) FAILF("Squeeze: 维 %d 大小 %lld ≠ 1", j, (long long)in[0]->dims[j]); }
            else odims[di++] = in[0]->dims[j];
        }
        if (di != keep) FAILF("Squeeze: 形状计算异常");
    }
    *out = onnx_arena_new(ar, in[0]->dtype, nd - na, odims);
    if (!*out) FAILF("Squeeze: 分配失败");
    copy_flat(*out, in[0]);
    return 0;
}

int onnx_op_flatten(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                    const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", 1), odims[2] = { 1, 1 };
    int i;
    (void)ar;
    if (n_in != 1) FAILF("Flatten: 需要 1 个输入");
    if (axis < 0) axis += in[0]->ndim;
    if (axis < 0 || axis > in[0]->ndim) FAILF("Flatten: axis 越界");
    for (i = 0; i < (int)axis; i++) odims[0] *= in[0]->dims[i];
    for (i = (int)axis; i < in[0]->ndim; i++) odims[1] *= in[0]->dims[i];
    *out = onnx_arena_new(ar, in[0]->dtype, 2, odims);
    if (!*out) FAILF("Flatten: 分配失败");
    copy_flat(*out, in[0]);
    return 0;
}

int onnx_op_gather(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                   const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    const OnnxT* data; const OnnxT* idx;
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", 0), odims[ONNX_MAXDIM];
    int i, j, od, da, ir, es;
    int64_t outer, ax, inner, k, o;
    (void)ar;
    if (n_in != 2) FAILF("Gather: 需要 2 个输入（data, indices）");
    data = in[0]; idx = in[1];
    da = data->ndim; ir = idx->ndim;
    if (axis < 0) axis += da;
    if (axis < 0 || axis >= da) FAILF("Gather: axis %lld 越界（秩 %d）", (long long)axis, da);
    od = da + ir - 1;
    if (od > ONNX_MAXDIM) FAILF("Gather: 输出秩 %d > %d", od, ONNX_MAXDIM);
    /* ONNX：输出 = data.dims[:axis] + indices.dims + data.dims[axis+1:]
     * ⚠️ indices 的形状是**插在 axis 位置**，不是追加到末尾（M157 实测踩过：
     *    追加到末尾会把 [2,H] × [1,T] 做成 [H,1,T]，随后 Add 广播必炸）。 */
    j = 0;
    for (i = 0; i < (int)axis; i++) odims[j++] = data->dims[i];
    for (i = 0; i < ir; i++) odims[j++] = idx->dims[i];
    for (i = (int)axis + 1; i < da; i++) odims[j++] = data->dims[i];
    if (j != od) FAILF("Gather: 输出秩计算异常");
    *out = onnx_arena_new(ar, data->dtype, od, odims);
    if (!*out) FAILF("Gather: 分配失败");
    es = onnx_t_esize(data->dtype);
    outer = 1; for (i = 0; i < (int)axis; i++) outer *= data->dims[i];
    ax = data->dims[axis];
    inner = 1; for (i = (int)axis + 1; i < da; i++) inner *= data->dims[i];
    for (k = 0; k < idx->nelem; k++) {
        int64_t sel = onnx_t_geti(idx, k);
        if (sel < 0) sel += ax;
        if (sel < 0 || sel >= ax) FAILF("Gather: 索引 %lld 越界（axis 大小 %lld）",
                                        (long long)onnx_t_geti(idx, k), (long long)ax);
        for (o = 0; o < outer; o++) {
            const unsigned char* src = data->data + (size_t)((o * ax + sel) * inner) * (size_t)es;
            unsigned char* dst = (*out)->data + (size_t)((o * idx->nelem + k) * inner) * (size_t)es;
            memcpy(dst, src, (size_t)inner * (size_t)es);
        }
    }
    return 0;
}

/* Slice（opset 10+：starts/ends/axes/steps 走输入；负步长按 ONNX 语义处理）。
 * 坐标法：输出元素 k → 各维坐标 c[i] → 源下标 start[i] + c[i]*step[i]。 */
int onnx_op_slice(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                  const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    const OnnxT* x;
    int64_t S[ONNX_MAXDIM], E[ONNX_MAXDIM], P[ONNX_MAXDIM];
    int64_t start[ONNX_MAXDIM], len[ONNX_MAXDIM];
    int64_t odims[ONNX_MAXDIM], istr[ONNX_MAXDIM], ostr[ONNX_MAXDIM];
    int64_t ns, i, k, es;
    (void)attrs; (void)n_attr; (void)ar;
    if (n_in < 3) FAILF("Slice: 至少需要 data/starts/ends 三个输入");
    x = in[0];
    if (x->ndim > ONNX_MAXDIM) FAILF("Slice: 秩过大");
    ns = in[1]->nelem;
    if (in[2]->nelem != ns) FAILF("Slice: starts/ends 长度不等");
    if (ns > x->ndim) FAILF("Slice: starts 长度 %lld > 秩 %d", (long long)ns, x->ndim);
    for (i = 0; i < x->ndim; i++) { S[i] = 0; E[i] = x->dims[i]; P[i] = 1; }
    for (i = 0; i < ns; i++) {
        int64_t a = i;
        if (n_in >= 4 && in[3]->nelem > 0) {
            if (in[3]->nelem != ns) FAILF("Slice: axes 长度 %lld ≠ starts 长度 %lld",
                                          (long long)in[3]->nelem, (long long)ns);
            a = onnx_t_geti(in[3], i);
            if (a < 0) a += x->ndim;
        }
        if (a < 0 || a >= x->ndim) FAILF("Slice: 轴 %lld 越界（秩 %d）", (long long)a, x->ndim);
        S[a] = onnx_t_geti(in[1], i);
        E[a] = onnx_t_geti(in[2], i);
        if (n_in >= 5 && in[4]->nelem > 0) {
            if (in[4]->nelem != ns) FAILF("Slice: steps 长度 %lld ≠ starts 长度 %lld",
                                          (long long)in[4]->nelem, (long long)ns);
            P[a] = onnx_t_geti(in[4], i);
            if (P[a] == 0) FAILF("Slice: step 为 0");
        }
    }
    for (i = 0; i < x->ndim; i++) {
        int64_t dim = x->dims[i], s = S[i], e = E[i], p = P[i];
        if (p > 0) {
            if (s < 0) s += dim;
            if (e < 0) e += dim;
            if (s < 0) s = 0;
            if (s > dim) s = dim;
            if (e < 0) e = 0;
            if (e > dim) e = dim;
            len[i] = e > s ? (e - s + p - 1) / p : 0;
            start[i] = (s < dim) ? s : (dim > 0 ? dim - 1 : 0);
        } else {
            if (s < 0) s += dim;
            if (e != INT64_MIN) { if (e < 0) e += dim; }
            if (s < 0) s = 0;
            if (s > dim - 1) s = dim - 1;
            if (e < -1) e = -1;
            if (e > dim - 1) e = dim - 1;
            len[i] = (s > e) ? ((s - e) + (-p) - 1) / (-p) : 0;
            start[i] = (s < 0) ? 0 : s;
        }
        if (len[i] < 0) FAILF("Slice: 轴 %lld 长度为负", (long long)i);
        odims[i] = len[i];
    }
    *out = onnx_arena_new(ar, x->dtype, x->ndim, odims);
    if (!*out) FAILF("Slice: 分配失败");
    if ((*out)->nelem == 0) return 0;
    onnx_strides(x->dims, x->ndim, istr);
    onnx_strides(odims, x->ndim, ostr);
    es = onnx_t_esize(x->dtype);
    for (k = 0; k < (*out)->nelem; k++) {
        int64_t rem = k, srcoff = 0;
        for (i = 0; i < x->ndim; i++) {
            int64_t c = rem / ostr[i];
            rem %= ostr[i];
            srcoff += (start[i] + c * P[i]) * istr[i];
        }
        memcpy((*out)->data + k * es, x->data + srcoff * es, (size_t)es);
    }
    return 0;
}

/* Expand：把 input 广播到 shape 指定的形状（shape 里的 1 取 input 的对应维）。 */
int onnx_op_expand(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                   const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    const OnnxT* shape;
    int64_t want[ONNX_MAXDIM], got[ONNX_MAXDIM], tst[ONNX_MAXDIM], ost[ONNX_MAXDIM];
    int64_t odims[ONNX_MAXDIM];
    int64_t nd, i, k, es;
    (void)attrs; (void)n_attr; (void)ar;
    if (n_in != 2) FAILF("Expand: 需要 2 个输入（input, shape）");
    shape = in[1];
    if (shape->ndim != 1) FAILF("Expand: shape 必须是 1 维");
    nd = shape->nelem;
    if (nd > ONNX_MAXDIM) FAILF("Expand: 目标秩 %lld > %d", (long long)nd, ONNX_MAXDIM);
    for (i = 0; i < nd; i++) {
        want[i] = onnx_t_geti(shape, i);
        if (want[i] < 0) FAILF("Expand: 目标维 %lld 为负", (long long)want[i]);
    }
    /* ONNX Expand：结果形状 = broadcast(input.dims, want)，其中 want 的 1 会被 input 维顶替 */
    for (i = 0; i < nd; i++) {
        int64_t ti = in[0]->ndim - nd + i;
        int64_t d = (ti >= 0) ? in[0]->dims[ti] : 1;
        if (want[i] == 1) odims[i] = d;
        else if (d == 1 || d == want[i]) odims[i] = want[i];
        else FAILF("Expand: 维 %lld 不兼容（input %lld vs 目标 %lld）",
                   (long long)i, (long long)d, (long long)want[i]);
    }
    for (i = 0; i < nd; i++) {
        int64_t ti = in[0]->ndim - nd + i;
        got[i] = (ti >= 0) ? in[0]->dims[ti] : 1;
    }
    *out = onnx_arena_new(ar, in[0]->dtype, (int)nd, odims);
    if (!*out) FAILF("Expand: 分配失败");
    if ((*out)->nelem == 0) return 0;
    /* 输入在各轴上的步长（广播维为 0） */
    {
        int64_t si[ONNX_MAXDIM];
        onnx_strides(in[0]->dims, in[0]->ndim, si);
        for (i = 0; i < nd; i++) {
            int64_t ti = in[0]->ndim - nd + i;
            tst[i] = (ti < 0 || got[i] == 1) ? 0 : si[ti];
        }
    }
    onnx_strides(odims, (int)nd, ost);
    es = onnx_t_esize(in[0]->dtype);
    for (k = 0; k < (*out)->nelem; k++) {
        int64_t rem = k, srcoff = 0;
        for (i = 0; i < nd; i++) {
            int64_t c = rem / ost[i];
            rem %= ost[i];
            srcoff += c * tst[i];
        }
        memcpy((*out)->data + k * es, in[0]->data + srcoff * es, (size_t)es);
    }
    return 0;
}

/* Tile：按 repeats（与 input 同秩）逐轴重复。 */
int onnx_op_tile(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                 const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)n_out;
    int64_t odims[ONNX_MAXDIM], rep[ONNX_MAXDIM], istr[ONNX_MAXDIM], ostr[ONNX_MAXDIM];
    int64_t i, k, es;
    int nd;
    (void)attrs; (void)n_attr; (void)ar;
    if (n_in != 2) FAILF("Tile: 需要 2 个输入（input, repeats）");
    nd = in[0]->ndim;
    if (in[1]->nelem != nd) FAILF("Tile: repeats 长度 %lld ≠ 秩 %d", (long long)in[1]->nelem, nd);
    for (i = 0; i < nd; i++) {
        rep[i] = onnx_t_geti(in[1], i);
        if (rep[i] < 0) FAILF("Tile: repeats[%lld] 为负", (long long)i);
        odims[i] = in[0]->dims[i] * rep[i];
    }
    *out = onnx_arena_new(ar, in[0]->dtype, nd, odims);
    if (!*out) FAILF("Tile: 分配失败");
    if ((*out)->nelem == 0) return 0;
    onnx_strides(in[0]->dims, nd, istr);
    onnx_strides(odims, nd, ostr);
    es = onnx_t_esize(in[0]->dtype);
    for (k = 0; k < (*out)->nelem; k++) {
        int64_t rem = k, srcoff = 0;
        for (i = 0; i < nd; i++) {
            int64_t c = rem / ostr[i];
            rem %= ostr[i];
            srcoff += (c % in[0]->dims[i]) * istr[i];
        }
        memcpy((*out)->data + k * es, in[0]->data + srcoff * es, (size_t)es);
    }
    return 0;
}

/* Split：输出 n_out 个张量，沿 axis 切片。
 * 尺寸来源优先级：`split` 属性 → 第 2 输入（opset13+）→ `num_outputs` 均分（不能整除则报错）。 */
int onnx_op_split(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                  const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", 0);
    int64_t sizes[64], odims[ONNX_MAXDIM];
    const int64_t* sp; int nsp = 0;
    int64_t outer = 1, inner = 1, off = 0, es;
    int nd, i, o, j;
    (void)ar;
    if (n_in < 1) FAILF("Split: 需要输入");
    if (n_out < 1 || n_out > 64) FAILF("Split: 输出个数 %d 非法", n_out);
    nd = in[0]->ndim;
    if (axis < 0) axis += nd;
    if (axis < 0 || axis >= nd) FAILF("Split: axis %lld 越界（秩 %d）", (long long)axis, nd);
    sp = onnx_attr_is(attrs, n_attr, "split", &nsp);
    if (!sp || nsp == 0) {
        if (n_in >= 2 && in[1]->nelem > 0) {
            if (in[1]->nelem != n_out) FAILF("Split: split 输入长度 %lld ≠ 输出个数 %d",
                                             (long long)in[1]->nelem, n_out);
            for (i = 0; i < n_out; i++) sizes[i] = onnx_t_geti(in[1], i);
        } else {
            int64_t total = in[0]->dims[axis];
            if (n_out == 0 || total % n_out) FAILF("Split: 无法均分（%lld / %d）",
                                                   (long long)total, n_out);
            for (i = 0; i < n_out; i++) sizes[i] = total / n_out;
        }
    } else {
        if (nsp != n_out) FAILF("Split: split 属性长度 %d ≠ 输出个数 %d", nsp, n_out);
        for (i = 0; i < n_out; i++) sizes[i] = sp[i];
    }
    {
        int64_t sum = 0;
        for (i = 0; i < n_out; i++) {
            if (sizes[i] < 0) FAILF("Split: 尺寸为负");
            sum += sizes[i];
        }
        if (sum != in[0]->dims[axis]) FAILF("Split: 尺寸和 %lld ≠ 轴大小 %lld",
                                            (long long)sum, (long long)in[0]->dims[axis]);
    }
    for (i = 0; i < (int)axis; i++) outer *= in[0]->dims[i];
    for (i = (int)axis + 1; i < nd; i++) inner *= in[0]->dims[i];
    es = onnx_t_esize(in[0]->dtype);
    for (o = 0; o < n_out; o++) {
        for (j = 0; j < nd; j++) odims[j] = in[0]->dims[j];
        odims[axis] = sizes[o];
        out[o] = onnx_arena_new(ar, in[0]->dtype, nd, odims);
        if (!out[o]) FAILF("Split: 第 %d 个输出分配失败", o);
        for (i = 0; i < outer; i++) {
            const unsigned char* src = in[0]->data +
                (size_t)((i * in[0]->dims[axis] + off) * inner) * (size_t)es;
            unsigned char* dst = out[o]->data + (size_t)(i * sizes[o] * inner) * (size_t)es;
            memcpy(dst, src, (size_t)(sizes[o] * inner) * (size_t)es);
        }
        off += sizes[o];
    }
    return 0;
}
