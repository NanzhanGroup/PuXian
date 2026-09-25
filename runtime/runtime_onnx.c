/* runtime_onnx.c —— ONNX 的语言侧接口（M156，零依赖）
 *
 * 对照物：token-cache 的 Go 实现 `embedding_engine_onnx.go`
 *   —— 走 `github.com/yalue/onnxruntime_go`（cgo + dlopen
 *   `/usr/local/onnxruntime-linux-x64-1.20.1/lib/libonnxruntime.so`）。
 * 本模块**不 dlopen 任何外部库**：解析器在 onnx_proto.c 里自己实现。
 *
 * 语言侧接口（本轮 = 解析 + 权重访问；执行面 M157）：
 *   onnx_model_open(path) → {ok:true, id:int, ir_version, nodes, initializers, inputs, outputs}
 *                         | Err("onnx: ...")
 *   onnx_info(id)         → str(JSON 结构摘要) | Err
 *   onnx_initializer(id, name) → {name, dtype:"float", dims:[...], nbytes:int, data:bytes} | Err
 *   onnx_initializer_names(id) → [str]（图内声明顺序）
 *   onnx_model_close(id)  → bool（幂等：重复关返回 false）
 *
 * 句柄表固定 8 槽（模型含权重，90MB 级；槽数与进程内存成比例，故取得小）。
 */
#include "runtime.h"
#include "onnx.h"
#include "onnx_tensor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef PX_NO_ONNX

#define ONNX_MAX_SLOTS 8

typedef struct { int used; OnnxModel* m; char* path; } OnnxSlot;

static OnnxSlot g_onnx_slots[ONNX_MAX_SLOTS];
static pthread_mutex_t g_onnx_mu = PTHREAD_MUTEX_INITIALIZER;

static int onnx_slot_put(OnnxModel* m, const char* path) {
    int id = 0, i;
    pthread_mutex_lock(&g_onnx_mu);
    for (i = 0; i < ONNX_MAX_SLOTS; i++) {
        if (!g_onnx_slots[i].used) {
            g_onnx_slots[i].used = 1;
            g_onnx_slots[i].m = m;
            g_onnx_slots[i].path = strdup(path);
            id = i + 1;
            break;
        }
    }
    pthread_mutex_unlock(&g_onnx_mu);
    return id;
}

static OnnxModel* onnx_slot_get(int id) {
    OnnxModel* m = NULL;
    if (id < 1 || id > ONNX_MAX_SLOTS) return NULL;
    pthread_mutex_lock(&g_onnx_mu);
    if (g_onnx_slots[id - 1].used) m = g_onnx_slots[id - 1].m;
    pthread_mutex_unlock(&g_onnx_mu);
    return m;
}

static int onnx_slot_take(int id, OnnxModel** out, char** path) {
    if (id < 1 || id > ONNX_MAX_SLOTS) return 0;
    pthread_mutex_lock(&g_onnx_mu);
    if (g_onnx_slots[id - 1].used) {
        *out = g_onnx_slots[id - 1].m;
        if (path) *path = g_onnx_slots[id - 1].path; else free(g_onnx_slots[id - 1].path);
        g_onnx_slots[id - 1].used = 0;
        g_onnx_slots[id - 1].m = NULL;
        g_onnx_slots[id - 1].path = NULL;
        pthread_mutex_unlock(&g_onnx_mu);
        return 1;
    }
    pthread_mutex_unlock(&g_onnx_mu);
    return 0;
}

/* 整文件读入（二进制安全）。返回 malloc 缓冲，*n 出长度；失败 NULL。 */
static unsigned char* onnx_read_all(const char* path, size_t* n) {
    FILE* f = fopen(path, "rb");
    long sz;
    unsigned char* buf;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    buf = (unsigned char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    if (sz > 0 && fread(buf, 1, (size_t)sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *n = (size_t)sz;
    return buf;
}

/* onnx_model_open(path) */
static LXValue bi_onnx_model_open(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    const char* path;
    unsigned char* buf;
    size_t n = 0;
    char* err = NULL;
    OnnxModel* m;
    int id;
    LXValue d;
    if (nargs != 1) px_error("R1002: onnx_model_open 需要 1 个参数 (path)");
    if (args[0].type != PX_STR) return px_err(px_str("onnx: 路径必须是字符串"));
    path = px_val_cstr(args[0]);
    buf = onnx_read_all(path, &n);
    if (!buf) return px_err(px_str("onnx: 无法读取模型文件"));
    m = onnx_model_parse(buf, n, &err);
    free(buf);   /* 解析器把需要的数据都拷走了（initializer 权重亦然） */
    if (!m) {
        char msg[512];
        snprintf(msg, sizeof(msg), "onnx: %s", err ? err : "解析失败");
        free(err);
        return px_err(px_str(msg));
    }
    id = onnx_slot_put(m, path);
    if (id == 0) { onnx_model_free(m); return px_err(px_str("onnx: 句柄槽已满（上限 8）")); }
    d = px_dict();
    px_root_push_keep(d);   //   分配必须登记（否则 d 被误回收 ⇒ 返回已释放 dict）
    px_dict_set(d, "ok", px_bool(1));
    px_dict_set(d, "id", px_int(id));
    px_dict_set(d, "ir_version", px_int(m->ir_version));
    px_dict_set(d, "graph", px_str(m->graph_name ? m->graph_name : ""));
    px_dict_set(d, "nodes", px_int(m->n_node));
    px_dict_set(d, "initializers", px_int(m->n_init));
    px_dict_set(d, "inputs", px_int(m->n_in));
    px_dict_set(d, "outputs", px_int(m->n_out));
    px_root_pop();
    return d;
}

/* onnx_info(id) → JSON */
static LXValue bi_onnx_info(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m;
    char* j;
    (void)ctx;
    if (nargs != 1) px_error("R1002: onnx_info 需要 1 个参数 (id)");
    // M195：`(int)args[0].as.i` 对 float 读的是**位模式**（1.5 ⇒ 天文数字）⇒ 显式判 int
    m = onnx_slot_get((int)px_arg_int(args[0], "onnx_info", "id"));
    if (!m) return px_err(px_str("onnx: 无效或已关闭的模型 id"));
    j = onnx_model_info_json(m);
    if (!j) return px_err(px_str("onnx: 结构渲染失败（内存不足）"));
    {
        LXValue v = px_str(j);
        free(j);
        return v;
    }
}

/* onnx_initializer(id, name) → {name,dtype,dims,nbytes,data} */
static LXValue bi_onnx_initializer(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m;
    const OnnxTensor* t;
    LXValue d;
    LXValue dims;
    (void)ctx;
    if (nargs != 2) px_error("R1002: onnx_initializer 需要 2 个参数 (id, name)");
    m = onnx_slot_get((int)px_arg_int(args[0], "onnx_initializer", "id"));
    if (!m) return px_err(px_str("onnx: 无效或已关闭的模型 id"));
    if (args[1].type != PX_STR) return px_err(px_str("onnx: 名字必须是字符串"));
    t = onnx_find_init(m, px_val_cstr(args[1]));
    if (!t) return px_err(px_str("onnx: 没有该 initializer"));
    dims = px_list(0);
    px_root_push_keep(dims);
    {
        int i;
        for (i = 0; i < t->ndim; i++) px_list_push(dims, px_int((int64_t)t->dims[i]));
    }
    d = px_dict();
    PX_KEEP(d);
    px_dict_set(d, "name", px_str(t->name ? t->name : ""));
    px_dict_set(d, "dtype", px_str(onnx_dtype_name(t->dtype)));
    px_dict_set(d, "dtype_id", px_int(t->dtype));
    px_dict_set(d, "dims", dims);
    px_dict_set(d, "nbytes", px_int((int64_t)t->nbytes));
    px_dict_set(d, "data", t->data ? px_bytes_len(t->data, (int)t->nbytes) : px_bytes_len("", 0));
    px_root_pop();
    return d;
}

/* onnx_initializer_names(id) → [str] */
static LXValue bi_onnx_initializer_names(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m;
    LXValue lst;
    int i;
    (void)ctx;
    if (nargs != 1) px_error("R1002: onnx_initializer_names 需要 1 个参数 (id)");
    m = onnx_slot_get((int)px_arg_int(args[0], "onnx_initializer_names", "id"));
    if (!m) return px_err(px_str("onnx: 无效或已关闭的模型 id"));
    lst = px_list(0);
    px_root_push_keep(lst);
    for (i = 0; i < m->n_init; i++)
        px_list_push(lst, px_str(m->inits[i].name ? m->inits[i].name : ""));
    px_root_pop();
    return lst;
}

/* onnx_model_close(id) → bool */
static LXValue bi_onnx_model_close(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m = NULL;
    char* path = NULL;
    (void)ctx;
    if (nargs != 1) px_error("R1002: onnx_model_close 需要 1 个参数 (id)");
    if (!onnx_slot_take((int)px_arg_int(args[0], "onnx_model_close", "id"), &m, &path)) return px_bool(0);
    free(path);
    onnx_model_free(m);
    return px_bool(1);
}


/* ══════════════════════════ M157：执行面（张量/算子/执行器） ══════════════════════════
 * 语言侧接口：
 *   onnx_op_names()            → [str]（运行时支持的算子名；与 C 侧算子表同源）
 *   onnx_run(id, feeds)        → {ok:true, nodes, outputs:{名:{dtype,dtype_id,dims,nbytes,data}}, stats}
 *                              | Err("onnx: ...")
 *   f32_bytes([float...])      → bytes（小端 float32 打包）
 *   f32_at(bytes, i)           → float（小端解码第 i 个）
 *   f32_count(bytes)           → int
 *   i64_bytes([int...]) / i64_at(bytes, i) / i64_count(bytes)  同上（int64）
 * feeds 的每个值：{dims:[...],  dtype?:str,  data?:bytes | f32?:[float] | i64?:[int] | i32?:[int] | f64?:[float]}
 *   · 给 data 时必须给 dtype；给 f32/i64/i32/f64 时 dtype 可省（按列表名推断）。
 */
static int onnx_dt_from_name(const char* s) {
    if (!s) return -1;
    if (!strcmp(s, "float") || !strcmp(s, "float32")) return ONNX_DT_FLOAT;
    if (!strcmp(s, "double") || !strcmp(s, "float64")) return ONNX_DT_DOUBLE;
    if (!strcmp(s, "int64")) return ONNX_DT_INT64;
    if (!strcmp(s, "int32")) return ONNX_DT_INT32;
    if (!strcmp(s, "int8")) return ONNX_DT_INT8;
    if (!strcmp(s, "uint8")) return ONNX_DT_UINT8;
    if (!strcmp(s, "bool")) return ONNX_DT_BOOL;
    return -1;
}

static int onnx_list_len(LXValue v) {
    if (v.type != PX_LIST || !v.as.obj) return -1;
    return v.as.obj->as.list.len;
}

static LXValue onnx_list_at(LXValue v, int i) { return v.as.obj->as.list.items[i]; }

/* {dims, dtype?, data|f32|i64|i32|f64} → OnnxT（malloc 给调用方） */
static OnnxT* onnx_spec_to_tensor(LXValue spec, char* err, size_t errc) {
    int64_t dims[ONNX_MAXDIM];
    int nd = 0, dt = -1, i, n = -1;
    LXValue dv, li;
    OnnxT* t;
    if (spec.type != PX_DICT) { snprintf(err, errc, "feeds 的值必须是字典 {dims, ...}"); return NULL; }
    dv = px_dict_get(spec, "dims");
    if (dv.type != PX_NULL) {
        int nl = onnx_list_len(dv);
        if (nl < 0) { snprintf(err, errc, "dims 必须是列表"); return NULL; }
        if (nl > ONNX_MAXDIM) { snprintf(err, errc, "dims 维数 %d > %d", nl, ONNX_MAXDIM); return NULL; }
        for (i = 0; i < nl; i++) {
            LXValue e = onnx_list_at(dv, i);
            if (e.type != PX_INT) { snprintf(err, errc, "dims 元素必须是整数"); return NULL; }
            dims[i] = e.as.i;
        }
        nd = nl;
    }
    {
        LXValue dn = px_dict_get(spec, "dtype");
        if (dn.type != PX_NULL) {
            if (dn.type != PX_STR) { snprintf(err, errc, "dtype 必须是字符串"); return NULL; }
            dt = onnx_dt_from_name(px_val_cstr(dn));
            if (dt < 0) { snprintf(err, errc, "不支持的 dtype: %s", px_val_cstr(dn)); return NULL; }
        }
    }
    /* 推断 dtype + 元素个数 */
    if (dt < 0) {
        if (px_dict_has(spec, "f32") || px_dict_has(spec, "f64")) dt = (px_dict_has(spec, "f64") && !px_dict_has(spec, "f32")) ? ONNX_DT_DOUBLE : ONNX_DT_FLOAT;
        else if (px_dict_has(spec, "i64")) dt = ONNX_DT_INT64;
        else if (px_dict_has(spec, "i32")) dt = ONNX_DT_INT32;
        else { snprintf(err, errc, "既没有 data 也没有 f32/i64/i32/f64，无法确定 dtype"); return NULL; }
    }
    t = onnx_t_new(dt, nd, dims);
    if (!t) { snprintf(err, errc, "张量分配失败（形状或类型非法）"); return NULL; }
    li = px_dict_get(spec, "data");
    if (li.type == PX_BYTES || li.type == PX_STR) {
        const char* b = li.as.obj->as.str.data;
        int bl = li.as.obj->as.str.len;
        if ((size_t)bl != t->nbytes) {
            snprintf(err, errc, "data 字节数 %d ≠ 形状×类型要求的 %zu", bl, t->nbytes);
            onnx_t_free(t); return NULL;
        }
        if (bl > 0) memcpy(t->data, b, (size_t)bl);
        return t;
    }
    if (!strcmp(onnx_dtype_name(dt), "float") || dt == ONNX_DT_FLOAT || dt == ONNX_DT_DOUBLE) {
        LXValue f = px_dict_get(spec, dt == ONNX_DT_DOUBLE ? "f64" : "f32");
        if (f.type == PX_NULL) f = px_dict_get(spec, "f32");
        if (f.type == PX_NULL) f = px_dict_get(spec, "f64");
        if ((n = onnx_list_len(f)) < 0) { snprintf(err, errc, "f32 必须是列表"); onnx_t_free(t); return NULL; }
        if ((int64_t)n != t->nelem) { snprintf(err, errc, "f32 长度 %d ≠ 元素数 %lld", n, (long long)t->nelem); onnx_t_free(t); return NULL; }
        for (i = 0; i < n; i++) {
            LXValue e = onnx_list_at(f, i);
            if (e.type != PX_FLOAT && e.type != PX_INT) { snprintf(err, errc, "f32 元素必须是数值"); onnx_t_free(t); return NULL; }
            onnx_t_setf(t, i, e.type == PX_FLOAT ? e.as.f : (double)e.as.i);
        }
        return t;
    }
    {
        LXValue iv = px_dict_get(spec, dt == ONNX_DT_INT32 ? "i32" : "i64");
        if (iv.type == PX_NULL) iv = px_dict_get(spec, "i64");
        if (iv.type == PX_NULL) iv = px_dict_get(spec, "i32");
        if ((n = onnx_list_len(iv)) < 0) { snprintf(err, errc, "i64/i32 必须是列表"); onnx_t_free(t); return NULL; }
        if ((int64_t)n != t->nelem) { snprintf(err, errc, "整数列表长度 %d ≠ 元素数 %lld", n, (long long)t->nelem); onnx_t_free(t); return NULL; }
        for (i = 0; i < n; i++) {
            LXValue e = onnx_list_at(iv, i);
            if (e.type != PX_INT && e.type != PX_BOOL) { snprintf(err, errc, "整数列表元素必须是整数"); onnx_t_free(t); return NULL; }
            onnx_t_seti(t, i, e.type == PX_INT ? e.as.i : (int64_t)e.as.b);
        }
    }
    return t;
}

/* onnx_run(id, feeds) */
static LXValue bi_onnx_run(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m;
    OnnxT* feed_t[64];
    const char* feed_n[64];
    int nf = 0, i;
    OnnxRunResult* res;
    char* err = NULL;
    LXValue out;
    (void)ctx;
    if (nargs != 2) px_error("R1002: onnx_run 需要 2 个参数 (id, feeds)");
    m = onnx_slot_get((int)px_arg_int(args[0], "onnx_run", "id"));
    if (!m) return px_err(px_str("onnx: 无效或已关闭的模型 id"));
    if (args[1].type != PX_DICT) return px_err(px_str("onnx: feeds 必须是字典"));
    {
        LXObject* d = args[1].as.obj;
        if (d->as.dict.len > 64) return px_err(px_str("onnx: feeds 项数 > 64"));
        for (i = 0; i < d->as.dict.len; i++) {
            char ebuf[ONNX_ERRC];
            OnnxT* t;
            ebuf[0] = 0;
            t = onnx_spec_to_tensor(d->as.dict.vals[i], ebuf, sizeof(ebuf));
            if (!t) {
                char msg[ONNX_ERRC];
                snprintf(msg, sizeof(msg), "onnx: feed %.60s：%.100s", d->as.dict.keys[i], ebuf);
                return px_err(px_str(msg));
            }
            feed_n[nf] = d->as.dict.keys[i];
            feed_t[nf] = t;
            nf++;
        }
    }
    res = onnx_exec_run(m, feed_n, feed_t, nf, &err);
    for (i = 0; i < nf; i++) onnx_t_free(feed_t[i]);
    if (!res) {
        LXValue e;
        e = px_err(px_str(err ? err : "onnx: 执行失败"));
        free(err);
        return e;
    }
    out = px_dict();
    px_root_push_keep(out);
    px_dict_set(out, "ok", px_bool(1));
    px_dict_set(out, "nodes", px_int(res->nodes_run));
    {
        LXValue outs = px_dict();
        PX_KEEP(outs);
        LXValue stats = px_dict();
        PX_KEEP(stats);
        LXValue byop = px_dict();
        PX_KEEP(byop);
        for (i = 0; i < res->n_out; i++) {
            OnnxT* t = res->outputs[i];
            LXValue d = px_dict();
            px_root_push_keep(d);
            LXValue dims = px_list(0);
            PX_KEEP(dims);
            int j;
            char* js = onnx_shape_json(t);
            for (j = 0; j < t->ndim; j++) px_list_push(dims, px_int(t->dims[j]));
            px_dict_set(d, "dtype", px_str(onnx_dtype_name(t->dtype)));
            px_dict_set(d, "dtype_id", px_int(t->dtype));
            px_dict_set(d, "dims", dims);
            px_dict_set(d, "shape", px_str(js ? js : ""));
            px_dict_set(d, "nbytes", px_int((int64_t)t->nbytes));
            px_dict_set(d, "data", px_bytes_len(t->data, (int)t->nbytes));
            px_dict_set(outs, m->outputs[i].name ? m->outputs[i].name : "", d);
            px_root_pop();
            free(js);
        }
        for (i = 0; i < res->n_ops; i++)
            if (res->op_counts[i] > 0)
                px_dict_set(byop, onnx_op_name_at(i), px_int(res->op_counts[i]));
        px_dict_set(stats, "nodes", px_int(res->nodes_run));
        px_dict_set(stats, "by_op", byop);
        px_dict_set(out, "outputs", outs);
        px_dict_set(out, "stats", stats);
    }
    px_root_pop();
    onnx_exec_result_free(res);
    return out;
}

/* onnx_op_names() → [str] */
static LXValue bi_onnx_op_names(LXValue* args, int nargs, void* ctx) {
    LXValue lst = px_list(0);
    int i;
    if (nargs != 0) px_error("R1002: onnx_op_names 需要 0 个参数");
    (void)args; (void)ctx;
    px_root_push_keep(lst);
    for (i = 0; i < onnx_op_count(); i++) px_list_push(lst, px_str(onnx_op_name_at(i)));
    px_root_pop();
    return lst;
}

/* f32_bytes([...]) / f32_at(bytes,i) / f32_count(bytes) */
static LXValue bi_f32_bytes(LXValue* args, int nargs, void* ctx) {
    int n, i;
    float* buf;
    LXValue r;
    (void)ctx;
    if (nargs != 1) px_error("R1002: f32_bytes 需要 1 个参数 (列表)");
    n = onnx_list_len(args[0]);
    if (n < 0) return px_err(px_str("f32_bytes: 参数必须是列表"));
    buf = (float*)malloc(sizeof(float) * (size_t)(n > 0 ? n : 1));
    if (!buf) return px_err(px_str("f32_bytes: 内存不足"));
    for (i = 0; i < n; i++) {
        LXValue e = onnx_list_at(args[0], i);
        if (e.type != PX_FLOAT && e.type != PX_INT) { free(buf); return px_err(px_str("f32_bytes: 元素必须是数值")); }
        buf[i] = (float)(e.type == PX_FLOAT ? e.as.f : (double)e.as.i);
    }
    r = px_bytes_len(buf, (int)(sizeof(float) * (size_t)n));
    free(buf);
    return r;
}

static const char* onnx_bytes_arg(LXValue v, int* len) {
    if (v.type != PX_BYTES && v.type != PX_STR) return NULL;
    if (len) *len = v.as.obj->as.str.len;
    return v.as.obj->as.str.data;
}

static LXValue bi_f32_at(LXValue* args, int nargs, void* ctx) {
    int len = 0, i;
    const char* b;
    float f;
    (void)ctx;
    if (nargs != 2) px_error("R1002: f32_at 需要 2 个参数 (bytes, i)");
    b = onnx_bytes_arg(args[0], &len);
    if (!b || args[1].type != PX_INT) return px_err(px_str("f32_at: 参数类型错误"));
    i = (int)args[1].as.i;
    if (i < 0 || (size_t)(i * 4 + 4) > (size_t)len) return px_err(px_str("f32_at: 下标越界"));
    memcpy(&f, b + i * 4, 4);
    return px_float((double)f);
}

static LXValue bi_f32_count(LXValue* args, int nargs, void* ctx) {
    int len = 0;
    (void)ctx;
    if (nargs != 1) px_error("R1002: f32_count 需要 1 个参数 (bytes)");
    if (!onnx_bytes_arg(args[0], &len)) return px_err(px_str("f32_count: 参数类型错误"));
    return px_int(len / 4);
}

static LXValue bi_i64_bytes(LXValue* args, int nargs, void* ctx) {
    int n, i;
    int64_t* buf;
    LXValue r;
    (void)ctx;
    if (nargs != 1) px_error("R1002: i64_bytes 需要 1 个参数 (列表)");
    n = onnx_list_len(args[0]);
    if (n < 0) return px_err(px_str("i64_bytes: 参数必须是列表"));
    buf = (int64_t*)malloc(sizeof(int64_t) * (size_t)(n > 0 ? n : 1));
    if (!buf) return px_err(px_str("i64_bytes: 内存不足"));
    for (i = 0; i < n; i++) {
        LXValue e = onnx_list_at(args[0], i);
        if (e.type != PX_INT && e.type != PX_BOOL) { free(buf); return px_err(px_str("i64_bytes: 元素必须是整数")); }
        buf[i] = (e.type == PX_INT) ? e.as.i : (int64_t)e.as.b;
    }
    r = px_bytes_len(buf, (int)(sizeof(int64_t) * (size_t)n));
    free(buf);
    return r;
}

static LXValue bi_i64_at(LXValue* args, int nargs, void* ctx) {
    int len = 0, i;
    const char* b;
    int64_t v;
    (void)ctx;
    if (nargs != 2) px_error("R1002: i64_at 需要 2 个参数 (bytes, i)");
    b = onnx_bytes_arg(args[0], &len);
    if (!b || args[1].type != PX_INT) return px_err(px_str("i64_at: 参数类型错误"));
    i = (int)args[1].as.i;
    if (i < 0 || (size_t)(i * 8 + 8) > (size_t)len) return px_err(px_str("i64_at: 下标越界"));
    memcpy(&v, b + i * 8, 8);
    return px_int(v);
}

static LXValue bi_i64_count(LXValue* args, int nargs, void* ctx) {
    int len = 0;
    (void)ctx;
    if (nargs != 1) px_error("R1002: i64_count 需要 1 个参数 (bytes)");
    if (!onnx_bytes_arg(args[0], &len)) return px_err(px_str("i64_count: 参数类型错误"));
    return px_int(len / 8);
}

/* 供 runtime.c 注册调用（与 bi_* 同名但跨编译单元可见） */
void px_onnx_register(void) {
    px_set_global("onnx_model_open", px_native("onnx_model_open", bi_onnx_model_open));
    px_set_global("onnx_info", px_native("onnx_info", bi_onnx_info));
    px_set_global("onnx_initializer", px_native("onnx_initializer", bi_onnx_initializer));
    px_set_global("onnx_initializer_names",
                  px_native("onnx_initializer_names", bi_onnx_initializer_names));
    px_set_global("onnx_model_close", px_native("onnx_model_close", bi_onnx_model_close));
    /* M157：执行面 */
    px_set_global("onnx_run", px_native("onnx_run", bi_onnx_run));
    px_set_global("onnx_op_names", px_native("onnx_op_names", bi_onnx_op_names));
    px_set_global("f32_bytes", px_native("f32_bytes", bi_f32_bytes));
    px_set_global("f32_at", px_native("f32_at", bi_f32_at));
    px_set_global("f32_count", px_native("f32_count", bi_f32_count));
    px_set_global("i64_bytes", px_native("i64_bytes", bi_i64_bytes));
    px_set_global("i64_at", px_native("i64_at", bi_i64_at));
    px_set_global("i64_count", px_native("i64_count", bi_i64_count));
}

#endif /* PX_NO_ONNX */
