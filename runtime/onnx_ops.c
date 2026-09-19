/* onnx_ops.c —— ONNX 数值算子 + 广播引擎 + 算子表（M157）
 *
 * 广播：完整实现 NumPy/ONNX 的右对齐广播（最多 ONNX_MAXDIM 维），
 *   用「各输入在输出形状下的元素步长（广播维步长 0）+ 里程表进位」在**一遍线性扫描**里
 *   同时读多个输入，避免为每个元素重算坐标。
 *
 * 数值口径（对拍时的关键，写清楚免得每次重新推）：
 *   · 中间累加一律 **double**（MatMul/Softmax/Reduce/归一化），最后一次落到 float32。
 *     onnxruntime 的 kernel 是 float32 累加 ⇒ 本实现通常**更接近**数学真值；
 *     与 Python 参考（也是 double）对拍时误差只在最后一步舍入。
 *   · 比较/整数算术在**整型 dtype 上走 int64 路径**（不经 double，避免 >2^53 失真）。
 */
#include "onnx_tensor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define FAILF(...) do { if (err) snprintf(err, errc, __VA_ARGS__); return -1; } while (0)

/* ── onnx_tensor.c 里的结构/搬运类算子 ── */
int onnx_op_identity(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_reshape(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_transpose(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_concat(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_shape(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_cast(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_unsqueeze(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_squeeze(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_flatten(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_gather(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_slice(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_expand(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_tile(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);
int onnx_op_split(OnnxArena*, OnnxT**, int, OnnxT**, int, const OnnxAttr*, int, char*, size_t);

/* ══════════════════════════ 广播引擎 ══════════════════════════ */

static int is_int_dt(int dt) {
    return dt == ONNX_DT_INT64 || dt == ONNX_DT_INT32 || dt == ONNX_DT_INT8 ||
           dt == ONNX_DT_UINT8 || dt == ONNX_DT_BOOL || dt == ONNX_DT_INT16 ||
           dt == ONNX_DT_UINT16 || dt == ONNX_DT_UINT32 || dt == ONNX_DT_UINT64;
}

/* 结果 dtype：全同则同；全整型则取第一个；否则 float32（ONNX 的混合数值规则近似） */
static int res_dtype(OnnxT** in, int n) {
    int i, all_same = 1, all_int = 1;
    for (i = 0; i < n; i++) {
        if (in[i]->dtype != in[0]->dtype) all_same = 0;
        if (!is_int_dt(in[i]->dtype)) all_int = 0;
    }
    if (all_same) return in[0]->dtype;
    return all_int ? in[0]->dtype : ONNX_DT_FLOAT;
}

/* t 广播到 oshape 时各轴的**元素步长**（广播维 = 0） */
static void bstrides(const OnnxT* t, int ond, int64_t* st) {
    int64_t tst[ONNX_MAXDIM];
    int i, shift = ond - t->ndim;
    onnx_strides(t->dims, t->ndim, tst);
    for (i = 0; i < ond; i++) {
        int ti = i - shift;
        st[i] = (ti < 0 || t->dims[ti] == 1) ? 0 : tst[ti];
    }
}

static int bcast_all(OnnxT** in, int n, int64_t* odims, int* ond, char* err, size_t errc) {
    int i, nd = 0;
    for (i = 0; i < n; i++) {
        int nd2;
        int64_t tmp[ONNX_MAXDIM];
        if (!onnx_bcast_shape(odims, nd, in[i]->dims, in[i]->ndim, tmp, &nd2))
            FAILF("广播形状不兼容（输入 %d：%s）", i, "见 dims");
        memcpy(odims, tmp, sizeof(int64_t) * (size_t)nd2);
        nd = nd2;
    }
    *ond = nd;
    return 0;
}

enum { E_ADD = 1, E_SUB, E_MUL, E_DIV, E_POW, E_MAX, E_MIN, E_SUM, E_MEAN, E_MOD,
       E_CMP_EQ, E_CMP_LT, E_CMP_GT, E_CMP_LE, E_CMP_GE, E_AND, E_OR, E_XOR };

static int apply_arith(int code, double a, double b, int isint, double* outc, int64_t* outi) {
    switch (code) {
    case E_ADD: *outc = a + b; *outi = (int64_t)a + (int64_t)b; return 0;
    case E_SUB: *outc = a - b; *outi = (int64_t)a - (int64_t)b; return 0;
    case E_MUL: *outc = a * b; *outi = (int64_t)a * (int64_t)b; return 0;
    case E_DIV:
        if (isint) { if ((int64_t)b == 0) return -1; *outi = (int64_t)a / (int64_t)b; }
        else *outc = a / b;
        return 0;
    case E_POW: *outc = pow(a, b); *outi = (int64_t)pow(a, b); return 0;
    case E_MOD:
        if (isint) { if ((int64_t)b == 0) return -1; *outi = (int64_t)a % (int64_t)b; }
        else *outc = fmod(a, b);
        return 0;
    case E_MAX: *outc = a > b ? a : b; *outi = (int64_t)a > (int64_t)b ? (int64_t)a : (int64_t)b; return 0;
    case E_MIN: *outc = a < b ? a : b; *outi = (int64_t)a < (int64_t)b ? (int64_t)a : (int64_t)b; return 0;
    case E_SUM: case E_MEAN: *outc = a + b; *outi = (int64_t)a + (int64_t)b; return 0;
    default: return -1;
    }
}

static int eltwise(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                   int code, char* err, size_t errc) {
    int64_t odims[ONNX_MAXDIM], st[8][ONNX_MAXDIM], offs[8], cnt[ONNX_MAXDIM];
    int64_t i, k, nelem;
    int ond, j, dt, isint, ndiv;
    (void)n_out;
    if (n_in < 2) FAILF("逐元素算子需要 ≥2 个输入");
    if (n_in > 8) FAILF("逐元素算子输入过多（%d）", n_in);
    if (bcast_all(in, n_in, odims, &ond, err, errc)) return -1;
    nelem = 1;
    for (j = 0; j < ond; j++) nelem *= odims[j];
    if (code >= E_CMP_EQ && code <= E_XOR) dt = ONNX_DT_BOOL;
    else dt = res_dtype(in, n_in);
    if (code == E_MEAN) dt = (dt == ONNX_DT_FLOAT || dt == ONNX_DT_DOUBLE) ? dt : ONNX_DT_FLOAT;
    isint = is_int_dt(dt);
    *out = onnx_arena_new(ar, dt, ond, odims);
    if (!*out) FAILF("逐元素算子：分配失败");
    for (i = 0; i < n_in; i++) bstrides(in[i], ond, st[i]);
    for (j = 0; j < ond; j++) cnt[j] = 0;
    for (i = 0; i < n_in; i++) offs[i] = 0;
    for (k = 0; k < nelem; k++) {
        if (code >= E_CMP_EQ && code <= E_XOR) {
            int a = (int)onnx_t_getf(in[0], offs[0]), b = (int)onnx_t_getf(in[1], offs[1]);
            double fa = onnx_t_getf(in[0], offs[0]), fb = onnx_t_getf(in[1], offs[1]);
            int v = 0;
            switch (code) {
            case E_CMP_EQ: v = (is_int_dt(in[0]->dtype) ? (onnx_t_geti(in[0], offs[0]) == onnx_t_geti(in[1], offs[1])) : (fa == fb)); break;
            case E_CMP_LT: v = (is_int_dt(in[0]->dtype) ? (onnx_t_geti(in[0], offs[0]) < onnx_t_geti(in[1], offs[1])) : (fa < fb)); break;
            case E_CMP_GT: v = (is_int_dt(in[0]->dtype) ? (onnx_t_geti(in[0], offs[0]) > onnx_t_geti(in[1], offs[1])) : (fa > fb)); break;
            case E_CMP_LE: v = (is_int_dt(in[0]->dtype) ? (onnx_t_geti(in[0], offs[0]) <= onnx_t_geti(in[1], offs[1])) : (fa <= fb)); break;
            case E_CMP_GE: v = (is_int_dt(in[0]->dtype) ? (onnx_t_geti(in[0], offs[0]) >= onnx_t_geti(in[1], offs[1])) : (fa >= fb)); break;
            case E_AND:    v = (a != 0) && (b != 0); break;
            case E_OR:     v = (a != 0) || (b != 0); break;
            case E_XOR:    v = ((a != 0) != (b != 0)); break;
            default: break;
            }
            onnx_t_seti(*out, k, v);
        } else {
            double acc = 0.0, first = 0.0;
            int64_t iacc = 0;
            double c; int64_t ci;
            for (j = 0; j < n_in; j++) {
                double v = onnx_t_getf(in[j], offs[j]);
                if (j == 0) { acc = first = v; iacc = onnx_t_geti(in[j], offs[j]); continue; }
                if (apply_arith(code, acc, v, isint, &c, &ci)) FAILF("算子：除零或非法运算");
                acc = c; iacc = ci;
            }
            (void)first;
            if (code == E_MEAN) {
                acc /= (double)n_in;
                onnx_t_setf(*out, k, acc);
            } else if (isint && code != E_POW) onnx_t_seti(*out, k, iacc);
            else onnx_t_setf(*out, k, acc);
        }
        /* 里程表进位 */
        for (j = ond - 1; j >= 0; j--) {
            cnt[j]++;
            for (i = 0; i < n_in; i++) offs[i] += st[i][j];
            if (cnt[j] < odims[j]) break;
            cnt[j] = 0;
            for (i = 0; i < n_in; i++) offs[i] -= st[i][j] * odims[j];
        }
    }
    ndiv = 0; (void)ndiv;
    return 0;
}

#define ELT2(name, code) \
int name(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in, \
         const OnnxAttr* attrs, int n_attr, char* err, size_t errc) { \
    (void)attrs; (void)n_attr; \
    return eltwise(ar, out, n_out, in, n_in, code, err, errc); }

ELT2(onnx_op_add, E_ADD)
ELT2(onnx_op_sub, E_SUB)
ELT2(onnx_op_mul, E_MUL)
ELT2(onnx_op_div, E_DIV)
ELT2(onnx_op_pow, E_POW)
ELT2(onnx_op_mod, E_MOD)
ELT2(onnx_op_max, E_MAX)
ELT2(onnx_op_min, E_MIN)
ELT2(onnx_op_sum, E_SUM)
ELT2(onnx_op_mean, E_MEAN)
ELT2(onnx_op_equal, E_CMP_EQ)
ELT2(onnx_op_less, E_CMP_LT)
ELT2(onnx_op_greater, E_CMP_GT)
ELT2(onnx_op_lessorequal, E_CMP_LE)
ELT2(onnx_op_greaterorequal, E_CMP_GE)
ELT2(onnx_op_and, E_AND)
ELT2(onnx_op_or, E_OR)
ELT2(onnx_op_xor, E_XOR)

/* ══════════════════════════ 一元算子 ══════════════════════════ */

enum { U_SQRT = 1, U_ERF, U_EXP, U_LOG, U_ABS, U_NEG, U_CEIL, U_FLOOR, U_ROUND,
       U_RECIP, U_SIGN, U_TANH, U_SIGMOID, U_RELU, U_LEAKY, U_ELU, U_SOFTPLUS, U_NOT };

static double round_half_even(double x) {   /* ONNX Round：四舍六入五取偶 */
    double f = floor(x), d = x - f;
    if (d > 0.5) return f + 1;
    if (d < 0.5) return f;
    return (fmod(f, 2.0) == 0.0) ? f : f + 1;
}

static int unary(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                 int code, double alpha, char* err, size_t errc) {
    int64_t i;
    int dt;
    (void)n_out;
    if (n_in < 1) FAILF("一元算子需要 1 个输入");
    dt = (code == U_NOT) ? ONNX_DT_BOOL : in[0]->dtype;
    *out = onnx_arena_new(ar, dt, in[0]->ndim, in[0]->dims);
    if (!*out) FAILF("一元算子：分配失败");
    for (i = 0; i < in[0]->nelem; i++) {
        if (code == U_NOT) { onnx_t_seti(*out, i, onnx_t_getf(in[0], i) == 0.0); continue; }
        if (is_int_dt(dt) && (code == U_ABS || code == U_NEG || code == U_SIGN)) {
            int64_t v = onnx_t_geti(in[0], i), r = v;
            if (code == U_ABS) r = v < 0 ? -v : v;
            else if (code == U_NEG) r = -v;
            else r = (v > 0) - (v < 0);
            onnx_t_seti(*out, i, r);
            continue;
        }
        {
            double x = onnx_t_getf(in[0], i), r = x;
            switch (code) {
            case U_SQRT:    r = sqrt(x); break;
            case U_ERF:     r = onnx_erf(x); break;
            case U_EXP:     r = exp(x); break;
            case U_LOG:     r = log(x); break;
            case U_ABS:     r = fabs(x); break;
            case U_NEG:     r = -x; break;
            case U_CEIL:    r = ceil(x); break;
            case U_FLOOR:   r = floor(x); break;
            case U_ROUND:   r = round_half_even(x); break;
            case U_RECIP:   r = 1.0 / x; break;
            case U_SIGN:    r = (x > 0) - (x < 0); break;
            case U_TANH:    r = tanh(x); break;
            case U_SIGMOID: r = 1.0 / (1.0 + exp(-x)); break;
            case U_RELU:    r = x > 0 ? x : 0; break;
            case U_LEAKY:   r = x >= 0 ? x : alpha * x; break;
            case U_ELU:     r = x >= 0 ? x : alpha * (exp(x) - 1.0); break;
            case U_SOFTPLUS:r = log1p(exp(-fabs(x))) + (x > 0 ? x : 0); break;
            default: break;
            }
            onnx_t_setf(*out, i, r);
        }
    }
    return 0;
}

#define UN1(name, code, alpha) \
int name(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in, \
         const OnnxAttr* attrs, int n_attr, char* err, size_t errc) { \
    (void)attrs; (void)n_attr; \
    return unary(ar, out, n_out, in, n_in, code, alpha, err, errc); }

UN1(onnx_op_sqrt, U_SQRT, 0.0)
UN1(onnx_op_erf, U_ERF, 0.0)
UN1(onnx_op_exp, U_EXP, 0.0)
UN1(onnx_op_log, U_LOG, 0.0)
UN1(onnx_op_abs, U_ABS, 0.0)
UN1(onnx_op_neg, U_NEG, 0.0)
UN1(onnx_op_ceil, U_CEIL, 0.0)
UN1(onnx_op_floor, U_FLOOR, 0.0)
UN1(onnx_op_round, U_ROUND, 0.0)
UN1(onnx_op_reciprocal, U_RECIP, 0.0)
UN1(onnx_op_sign, U_SIGN, 0.0)
UN1(onnx_op_tanh, U_TANH, 0.0)
UN1(onnx_op_sigmoid, U_SIGMOID, 0.0)
UN1(onnx_op_relu, U_RELU, 0.0)
UN1(onnx_op_not, U_NOT, 0.0)

int onnx_op_leakyrelu(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                      const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    return unary(ar, out, n_out, in, n_in, U_LEAKY, onnx_attr_f(attrs, n_attr, "alpha", 0.01), err, errc);
}
int onnx_op_elu(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    return unary(ar, out, n_out, in, n_in, U_ELU, onnx_attr_f(attrs, n_attr, "alpha", 1.0), err, errc);
}
int onnx_op_softplus(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                     const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    (void)attrs; (void)n_attr;
    return unary(ar, out, n_out, in, n_in, U_SOFTPLUS, 0.0, err, errc);
}

/* Clip：min/max 可来自属性（opset<11）或第 2/3 输入（opset11+） */
int onnx_op_clip(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                 const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    double lo = -INFINITY, hi = INFINITY, r;
    int64_t i;
    int has_lo = 0, has_hi = 0, cnt;
    const double* fs = onnx_attr_fs(attrs, n_attr, "min", &cnt);
    (void)n_out;
    if (fs && cnt) { lo = fs[0]; has_lo = 1; }
    fs = onnx_attr_fs(attrs, n_attr, "max", &cnt);
    if (fs && cnt) { hi = fs[0]; has_hi = 1; }
    if (n_in >= 2 && in[1]->nelem > 0) { lo = onnx_t_getf(in[1], 0); has_lo = 1; }
    if (n_in >= 3 && in[2]->nelem > 0) { hi = onnx_t_getf(in[2], 0); has_hi = 1; }
    if (!has_lo) lo = -INFINITY;
    if (!has_hi) hi = INFINITY;
    if (n_in < 1) FAILF("Clip: 需要输入");
    *out = onnx_arena_new(ar, in[0]->dtype, in[0]->ndim, in[0]->dims);
    if (!*out) FAILF("Clip: 分配失败");
    for (i = 0; i < in[0]->nelem; i++) {
        double x = onnx_t_getf(in[0], i);
        r = x < lo ? lo : (x > hi ? hi : x);
        onnx_t_setf(*out, i, r);
    }
    return 0;
}

/* ══════════════════════════ MatMul（含 batched + 广播） ══════════════════════════ */

int onnx_op_matmul(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                   const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    OnnxT* A = in[0]; OnnxT* B = in[1];
    int64_t a2[ONNX_MAXDIM] = {0}, b2[ONNX_MAXDIM] = {0}, bshape[ONNX_MAXDIM], odims[ONNX_MAXDIM];
    int64_t as[ONNX_MAXDIM], bs[ONNX_MAXDIM];
    int64_t M, K, K2, N, nb = 0, batches = 1, bi, cnt[ONNX_MAXDIM];
    int n2a, n2b, i, j, k, vec_a = 0, vec_b = 0;
    (void)n_out; (void)attrs; (void)n_attr;
    if (n_in != 2) FAILF("MatMul: 需要 2 个输入");
    if (A->ndim == 0 || B->ndim == 0) FAILF("MatMul: 不支持标量");
    /* 1 维输入按 NumPy 语义提升：[K] → [1,K] / [K] → [K,1] */
    if (A->ndim == 1) { vec_a = 1; n2a = 2; a2[0] = 1; a2[1] = A->dims[0]; }
    else { n2a = A->ndim; for (i = 0; i < n2a; i++) a2[i] = A->dims[i]; }
    if (B->ndim == 1) { vec_b = 1; n2b = 2; b2[0] = B->dims[0]; b2[1] = 1; }
    else { n2b = B->ndim; for (i = 0; i < n2b; i++) b2[i] = B->dims[i]; }
    M = a2[n2a - 2]; K = a2[n2a - 1];
    K2 = b2[n2b - 2]; N = b2[n2b - 1];
    if (K != K2) FAILF("MatMul: 内维不一致（%lld vs %lld）", (long long)K, (long long)K2);
    if (!onnx_bcast_shape(a2, n2a - 2, b2, n2b - 2, bshape, (int*)&nb))
        FAILF("MatMul: batch 维不兼容");
    for (i = 0; i < (int)nb; i++) batches *= bshape[i];
    {
        int oi = 0;
        for (i = 0; i < (int)nb; i++) odims[oi++] = bshape[i];
        if (!vec_a) odims[oi++] = M;
        if (!vec_b) odims[oi++] = N;
        *out = onnx_arena_new(ar, A->dtype, oi, odims);
    }
    if (!*out) FAILF("MatMul: 分配失败");
    /* 各 batch 维的元素步长（A/B 的缺失维或大小为 1 的维 ⇒ 0，即广播） */
    onnx_strides(a2, n2a, as); onnx_strides(b2, n2b, bs);
    for (i = 0; i < (int)nb; i++) {
        int ai = (n2a - 2) - (int)nb + i, bi = (n2b - 2) - (int)nb + i;
        int64_t sav = (ai < 0 || a2[ai] == 1) ? 0 : as[ai];
        int64_t sbv = (bi < 0 || b2[bi] == 1) ? 0 : bs[bi];
        as[i] = sav; bs[i] = sbv;
    }
    for (i = 0; i < (int)nb; i++) cnt[i] = 0;
    {
        int64_t ao = 0, bo = 0, outb = 0;
        /* 快路径：float32 双方（宿主小端，见 onnx_tensor.c 顶部断言）⇒ 直接指针访问，
         *   免掉每元素一次 memcpy 式取值。真实模型实测约 3× 提速（M157）。 */
        int fast = (A->dtype == ONNX_DT_FLOAT && B->dtype == ONNX_DT_FLOAT &&
                    (*out)->dtype == ONNX_DT_FLOAT);
        const float* ap = fast ? (const float*)A->data : NULL;
        const float* bp = fast ? (const float*)B->data : NULL;
        float* op = fast ? (float*)(*out)->data : NULL;
        for (bi = 0; bi < batches; bi++) {
            for (i = 0; i < M; i++) {
                for (j = 0; j < N; j++) {
                    double acc = 0.0;
                    if (fast) {
                        const float* arow = ap + ao + (int64_t)i * K;
                        for (k = 0; k < K; k++) acc += (double)arow[k] * (double)bp[bo + (int64_t)k * N + j];
                        if (vec_a)      op[outb + j] = (float)acc;
                        else if (vec_b) op[outb + i] = (float)acc;
                        else            op[outb + (int64_t)i * N + j] = (float)acc;
                        continue;
                    }
                    for (k = 0; k < K; k++)
                        acc += onnx_t_getf(A, ao + (int64_t)i * K + k) *
                               onnx_t_getf(B, bo + (int64_t)k * N + j);
                    if (vec_a)      onnx_t_setf(*out, outb + j, acc);
                    else if (vec_b) onnx_t_setf(*out, outb + i, acc);
                    else            onnx_t_setf(*out, outb + (int64_t)i * N + j, acc);
                }
            }
            outb += (vec_a ? 1 : M) * (vec_b ? 1 : N);
            for (j = (int)nb - 1; j >= 0; j--) {
                cnt[j]++; ao += as[j]; bo += bs[j];
                if (cnt[j] < bshape[j]) break;
                cnt[j] = 0;
                ao -= as[j] * bshape[j]; bo -= bs[j] * bshape[j];
            }
        }
    }
    return 0;
}

/* ══════════════════════════ Softmax / 归约 ══════════════════════════ */

int onnx_op_softmax(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                    const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", -1), outer = 1, inner = 1, ax;
    int64_t i, o, k;
    int nd;
    (void)n_out;
    if (n_in != 1) FAILF("Softmax: 需要 1 个输入");
    nd = in[0]->ndim;
    if (axis < 0) axis += nd;
    if (axis < 0 || axis >= nd) FAILF("Softmax: axis 越界");
    for (i = 0; i < axis; i++) outer *= in[0]->dims[i];
    ax = in[0]->dims[axis];
    for (i = axis + 1; i < nd; i++) inner *= in[0]->dims[i];
    *out = onnx_arena_new(ar, ONNX_DT_FLOAT, nd, in[0]->dims);
    if (!*out) FAILF("Softmax: 分配失败");
    for (o = 0; o < outer; o++) {
        for (k = 0; k < inner; k++) {
            double mx = -INFINITY, sum = 0.0;
            int64_t base = o * ax * inner + k;
            for (i = 0; i < ax; i++) {
                double v = onnx_t_getf(in[0], base + i * inner);
                if (v > mx) mx = v;
            }
            for (i = 0; i < ax; i++) sum += exp(onnx_t_getf(in[0], base + i * inner) - mx);
            for (i = 0; i < ax; i++) {
                double e = exp(onnx_t_getf(in[0], base + i * inner) - mx) / sum;
                onnx_t_setf(*out, base + i * inner, e);
            }
        }
    }
    return 0;
}

enum { R_MEAN = 1, R_SUM, R_MAX, R_MIN, R_PROD };

static int reduce_impl(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                       const OnnxAttr* attrs, int n_attr, int code, char* err, size_t errc) {
    int64_t axes[ONNX_MAXDIM], odims[ONNX_MAXDIM], istr[ONNX_MAXDIM], ostr[ONNX_MAXDIM];
    int red[ONNX_MAXDIM];
    int nax = 0, i, j, nd, keepdims = (int)onnx_attr_i(attrs, n_attr, "keepdims", 1);
    int64_t k, acc_i, cnt;
    const int64_t* a;
    (void)n_out;
    if (n_in < 1) FAILF("归约: 需要输入");
    nd = in[0]->ndim;
    a = onnx_attr_is(attrs, n_attr, "axes", &nax);
    if ((!a || nax == 0) && n_in >= 2 && in[1]->nelem > 0) {
        nax = (int)in[1]->nelem;
        if (nax > ONNX_MAXDIM) FAILF("归约: 轴过多");
        for (i = 0; i < nax; i++) axes[i] = onnx_t_geti(in[1], i);
    } else if (a && nax > 0) {
        for (i = 0; i < nax; i++) axes[i] = a[i];
    } else {
        if (onnx_attr_i(attrs, n_attr, "noop_with_empty_axes", 0)) {
            *out = onnx_t_clone(in[0]);
            if (!*out) FAILF("归约: 分配失败");
            return 0;
        }
        nax = nd;
        for (i = 0; i < nd; i++) axes[i] = i;
    }
    for (i = 0; i < nd; i++) red[i] = 0;
    for (i = 0; i < nax; i++) {
        int64_t x = axes[i];
        if (x < 0) x += nd;
        if (x < 0 || x >= nd) FAILF("归约: 轴 %lld 越界", (long long)axes[i]);
        red[x] = 1;
    }
    {
        int oi = 0;
        for (i = 0; i < nd; i++) {
            if (red[i]) { if (keepdims) odims[oi++] = 1; }
            else odims[oi++] = in[0]->dims[i];
        }
        *out = onnx_arena_new(ar, (code == R_MEAN) ? ONNX_DT_FLOAT : in[0]->dtype, oi, odims);
    }
    if (!*out) FAILF("归约: 分配失败");
    /* 初始化（max/min 需要初值） */
    {
        int64_t on = (*out)->nelem;
        double init = (code == R_MAX) ? -INFINITY : ((code == R_MIN) ? INFINITY : 0.0);
        for (k = 0; k < on; k++) onnx_t_setf(*out, k, (code == R_PROD) ? 1.0 : init);
    }
    onnx_strides(in[0]->dims, nd, istr);
    {
        int64_t odims2[ONNX_MAXDIM];
        for (i = 0; i < nd; i++) odims2[i] = red[i] ? 1 : in[0]->dims[i];
        onnx_strides(odims2, nd, ostr);
    }
    for (k = 0; k < in[0]->nelem; k++) {
        int64_t rem = k, doff = 0;
        for (i = 0; i < nd; i++) {
            int64_t c = rem / istr[i];
            rem %= istr[i];
            if (!red[i]) doff += c * ostr[i];
        }
        {
            double v = onnx_t_getf(in[0], k), cur = onnx_t_getf(*out, doff);
            switch (code) {
            case R_MEAN: case R_SUM: cur += v; break;
            case R_MAX: cur = v > cur ? v : cur; break;
            case R_MIN: cur = v < cur ? v : cur; break;
            case R_PROD: cur *= v; break;
            default: break;
            }
            onnx_t_setf(*out, doff, cur);
        }
    }
    if (code == R_MEAN) {
        cnt = 1;
        for (i = 0; i < nd; i++) if (red[i]) cnt *= in[0]->dims[i];
        for (k = 0; k < (*out)->nelem; k++) onnx_t_setf(*out, k, onnx_t_getf(*out, k) / (double)cnt);
    }
    acc_i = 0; (void)acc_i; (void)j; (void)ostr;
    return 0;
}

#define RED(name, code) \
int name(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in, \
         const OnnxAttr* attrs, int n_attr, char* err, size_t errc) { \
    return reduce_impl(ar, out, n_out, in, n_in, attrs, n_attr, code, err, errc); }

RED(onnx_op_reducemean, R_MEAN)
RED(onnx_op_reducesum, R_SUM)
RED(onnx_op_reducemax, R_MAX)
RED(onnx_op_reducemin, R_MIN)
RED(onnx_op_reduceprod, R_PROD)

int onnx_op_argmax(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                   const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", 0), outer = 1, inner = 1, ax;
    int keepdims = (int)onnx_attr_i(attrs, n_attr, "keepdims", 1);
    int64_t i, o, k, odims[ONNX_MAXDIM];
    int nd, oi = 0;
    (void)n_out;
    if (n_in != 1) FAILF("ArgMax: 需要 1 个输入");
    nd = in[0]->ndim;
    if (axis < 0) axis += nd;
    if (axis < 0 || axis >= nd) FAILF("ArgMax: axis 越界");
    for (i = 0; i < axis; i++) outer *= in[0]->dims[i];
    ax = in[0]->dims[axis];
    for (i = axis + 1; i < nd; i++) inner *= in[0]->dims[i];
    for (i = 0; i < nd; i++) if (i != axis || keepdims) odims[oi++] = (i == axis ? 1 : in[0]->dims[i]);
    *out = onnx_arena_new(ar, ONNX_DT_INT64, oi, odims);
    if (!*out) FAILF("ArgMax: 分配失败");
    for (o = 0; o < outer; o++) {
        for (k = 0; k < inner; k++) {
            int64_t base = o * ax * inner + k, best = 0;
            double bv = -INFINITY;
            for (i = 0; i < ax; i++) {
                double v = onnx_t_getf(in[0], base + i * inner);
                if (v > bv) { bv = v; best = i; }
            }
            onnx_t_seti(*out, o * inner + k, best);
        }
    }
    return 0;
}

/* LayerNormalization（axis 起始的尾部维做归一化；可选 scale/bias） */
int onnx_op_layernorm(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                      const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t axis = onnx_attr_i(attrs, n_attr, "axis", -1), outer = 1, inner = 1;
    double eps = onnx_attr_f(attrs, n_attr, "epsilon", 1e-5);
    int64_t i, o;
    int nd;
    (void)n_out;
    if (n_in < 1) FAILF("LayerNormalization: 需要输入");
    nd = in[0]->ndim;
    if (axis < 0) axis += nd;
    if (axis < 0 || axis >= nd) FAILF("LayerNormalization: axis 越界");
    for (i = 0; i < axis; i++) outer *= in[0]->dims[i];
    for (i = axis; i < nd; i++) inner *= in[0]->dims[i];
    *out = onnx_arena_new(ar, ONNX_DT_FLOAT, nd, in[0]->dims);
    if (!*out) FAILF("LayerNormalization: 分配失败");
    for (o = 0; o < outer; o++) {
        double mean = 0.0, var = 0.0;
        int64_t base = o * inner;
        for (i = 0; i < inner; i++) mean += onnx_t_getf(in[0], base + i);
        mean /= (double)inner;
        for (i = 0; i < inner; i++) {
            double d = onnx_t_getf(in[0], base + i) - mean;
            var += d * d;
        }
        var /= (double)inner;
        {
            double inv = 1.0 / sqrt(var + eps);
            for (i = 0; i < inner; i++) {
                double v = (onnx_t_getf(in[0], base + i) - mean) * inv;
                if (n_in >= 2) v *= onnx_t_getf(in[1], i % in[1]->nelem);
                if (n_in >= 3) v += onnx_t_getf(in[2], i % in[2]->nelem);
                onnx_t_setf(*out, base + i, v);
            }
        }
    }
    return 0;
}

/* ══════════════════════════ Where / 常量类 ══════════════════════════ */

int onnx_op_where(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                  const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t odims[ONNX_MAXDIM], st[3][ONNX_MAXDIM], offs[3], cnt[ONNX_MAXDIM];
    int64_t i, k, nelem;
    int ond, j;
    (void)n_out; (void)attrs; (void)n_attr;
    if (n_in != 3) FAILF("Where: 需要 3 个输入（cond, X, Y）");
    if (bcast_all(in, 3, odims, &ond, err, errc)) return -1;
    nelem = 1;
    for (j = 0; j < ond; j++) nelem *= odims[j];
    *out = onnx_arena_new(ar, in[1]->dtype, ond, odims);
    if (!*out) FAILF("Where: 分配失败");
    for (i = 0; i < 3; i++) bstrides(in[i], ond, st[i]);
    for (j = 0; j < ond; j++) cnt[j] = 0;
    for (i = 0; i < 3; i++) offs[i] = 0;
    for (k = 0; k < nelem; k++) {
        int c = onnx_t_getf(in[0], offs[0]) != 0.0;
        if (is_int_dt(in[1]->dtype)) onnx_t_seti(*out, k, c ? onnx_t_geti(in[1], offs[1]) : onnx_t_geti(in[2], offs[2]));
        else onnx_t_setf(*out, k, c ? onnx_t_getf(in[1], offs[1]) : onnx_t_getf(in[2], offs[2]));
        for (j = ond - 1; j >= 0; j--) {
            cnt[j]++;
            for (i = 0; i < 3; i++) offs[i] += st[i][j];
            if (cnt[j] < odims[j]) break;
            cnt[j] = 0;
            for (i = 0; i < 3; i++) offs[i] -= st[i][j] * odims[j];
        }
    }
    return 0;
}

int onnx_op_constantofshape(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                            const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t odims[ONNX_MAXDIM];
    const OnnxTensor* vt = onnx_attr_t(attrs, n_attr, "value");
    int64_t i, nd, es;
    int dt = vt ? vt->dtype : ONNX_DT_FLOAT;
    (void)n_out;
    if (n_in != 1) FAILF("ConstantOfShape: 需要 1 个输入（shape）");
    if (in[0]->ndim != 1) FAILF("ConstantOfShape: shape 必须是 1 维");
    nd = in[0]->nelem;
    if (nd > ONNX_MAXDIM) FAILF("ConstantOfShape: 秩过大");
    for (i = 0; i < nd; i++) {
        odims[i] = onnx_t_geti(in[0], i);
        if (odims[i] < 0) FAILF("ConstantOfShape: 维为负");
    }
    if (onnx_t_esize(dt) == 0) FAILF("ConstantOfShape: 不支持的值类型");
    *out = onnx_arena_new(ar, dt, (int)nd, odims);
    if (!*out) FAILF("ConstantOfShape: 分配失败");
    es = (int64_t)onnx_t_esize(dt);
    if (vt && vt->nbytes >= (size_t)es) {
        for (i = 0; i < (*out)->nelem; i++)
            memcpy((*out)->data + i * es, vt->data, (size_t)es);
    }
    return 0;
}

int onnx_op_range(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                  const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    double start, limit, delta;
    int64_t n, i, sdims[1];
    int dt;
    (void)n_out; (void)attrs; (void)n_attr;
    if (n_in != 3) FAILF("Range: 需要 3 个输入（start, limit, delta）");
    start = onnx_t_getf(in[0], 0);
    limit = onnx_t_getf(in[1], 0);
    delta = onnx_t_getf(in[2], 0);
    if (delta == 0.0) FAILF("Range: delta 为 0");
    n = (int64_t)ceil((limit - start) / delta);
    if (n < 0) n = 0;
    if (n > 100000000) FAILF("Range: 元素过多（%lld）", (long long)n);
    dt = is_int_dt(in[0]->dtype) ? in[0]->dtype : ONNX_DT_FLOAT;
    sdims[0] = n;
    *out = onnx_arena_new(ar, dt, 1, sdims);
    if (!*out) FAILF("Range: 分配失败");
    for (i = 0; i < n; i++) {
        double v = start + (double)i * delta;
        if (is_int_dt(dt)) onnx_t_seti(*out, i, (int64_t)v); else onnx_t_setf(*out, i, v);
    }
    return 0;
}

int onnx_op_triu(OnnxArena* ar, OnnxT** out, int n_out, OnnxT** in, int n_in,
                 const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    int64_t k = onnx_attr_i(attrs, n_attr, "k", 0), es;
    int64_t i, j, rows, cols, b, batches = 1, o;
    int nd;
    (void)n_out;
    if (n_in != 1) FAILF("Triu: 需要 1 个输入");
    nd = in[0]->ndim;
    if (nd < 2) FAILF("Triu: 秩需 ≥2");
    rows = in[0]->dims[nd - 2];
    cols = in[0]->dims[nd - 1];
    for (i = 0; i < nd - 2; i++) batches *= in[0]->dims[i];
    es = (int64_t)onnx_t_esize(in[0]->dtype);
    *out = onnx_arena_new(ar, in[0]->dtype, nd, in[0]->dims);
    if (!*out) FAILF("Triu: 分配失败");
    memcpy((*out)->data, in[0]->data, in[0]->nbytes);
    for (b = 0; b < batches; b++) {
        int64_t base = b * rows * cols;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                if (j < i + k) {
                    unsigned char* p = (*out)->data + (base + i * cols + j) * es;
                    memset(p, 0, (size_t)es);
                }
            }
        }
    }
    o = 0; (void)o;
    return 0;
}

/* ══════════════════════════ 算子表 ══════════════════════════ */

static const OnnxOpEntry g_ops[] = {
    { "Identity", onnx_op_identity }, { "Reshape", onnx_op_reshape },
    { "Transpose", onnx_op_transpose }, { "Concat", onnx_op_concat },
    { "Shape", onnx_op_shape }, { "Cast", onnx_op_cast },
    { "Unsqueeze", onnx_op_unsqueeze }, { "Squeeze", onnx_op_squeeze },
    { "Flatten", onnx_op_flatten }, { "Gather", onnx_op_gather },
    { "Slice", onnx_op_slice }, { "Expand", onnx_op_expand },
    { "Tile", onnx_op_tile }, { "Split", onnx_op_split },
    { "Add", onnx_op_add }, { "Sub", onnx_op_sub }, { "Mul", onnx_op_mul },
    { "Div", onnx_op_div }, { "Pow", onnx_op_pow }, { "Mod", onnx_op_mod },
    { "Max", onnx_op_max }, { "Min", onnx_op_min }, { "Sum", onnx_op_sum },
    { "Mean", onnx_op_mean },
    { "Equal", onnx_op_equal }, { "Less", onnx_op_less }, { "Greater", onnx_op_greater },
    { "LessOrEqual", onnx_op_lessorequal }, { "GreaterOrEqual", onnx_op_greaterorequal },
    { "And", onnx_op_and }, { "Or", onnx_op_or }, { "Xor", onnx_op_xor },
    { "Not", onnx_op_not },
    { "Sqrt", onnx_op_sqrt }, { "Erf", onnx_op_erf }, { "Exp", onnx_op_exp },
    { "Log", onnx_op_log }, { "Abs", onnx_op_abs }, { "Neg", onnx_op_neg },
    { "Ceil", onnx_op_ceil }, { "Floor", onnx_op_floor }, { "Round", onnx_op_round },
    { "Reciprocal", onnx_op_reciprocal }, { "Sign", onnx_op_sign },
    { "Tanh", onnx_op_tanh }, { "Sigmoid", onnx_op_sigmoid }, { "Relu", onnx_op_relu },
    { "LeakyRelu", onnx_op_leakyrelu }, { "Elu", onnx_op_elu },
    { "Softplus", onnx_op_softplus }, { "Clip", onnx_op_clip },
    { "MatMul", onnx_op_matmul }, { "Softmax", onnx_op_softmax },
    { "ReduceMean", onnx_op_reducemean }, { "ReduceSum", onnx_op_reducesum },
    { "ReduceMax", onnx_op_reducemax }, { "ReduceMin", onnx_op_reducemin },
    { "ReduceProd", onnx_op_reduceprod }, { "ArgMax", onnx_op_argmax },
    { "LayerNormalization", onnx_op_layernorm }, { "Where", onnx_op_where },
    { "ConstantOfShape", onnx_op_constantofshape }, { "Range", onnx_op_range },
    { "Triu", onnx_op_triu },
};

int onnx_op_count(void) { return (int)(sizeof(g_ops) / sizeof(g_ops[0])); }

const char* onnx_op_name_at(int i) {
    if (i < 0 || i >= onnx_op_count()) return NULL;
    return g_ops[i].name;
}

const OnnxOpEntry* onnx_op_lookup(const char* name) {
    size_t i;
    if (!name) return NULL;
    for (i = 0; i < sizeof(g_ops) / sizeof(g_ops[0]); i++)
        if (strcmp(g_ops[i].name, name) == 0) return &g_ops[i];
    return NULL;
}
