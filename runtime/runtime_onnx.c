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
    px_dict_set(d, "ok", px_bool(1));
    px_dict_set(d, "id", px_int(id));
    px_dict_set(d, "ir_version", px_int(m->ir_version));
    px_dict_set(d, "graph", px_str(m->graph_name ? m->graph_name : ""));
    px_dict_set(d, "nodes", px_int(m->n_node));
    px_dict_set(d, "initializers", px_int(m->n_init));
    px_dict_set(d, "inputs", px_int(m->n_in));
    px_dict_set(d, "outputs", px_int(m->n_out));
    return d;
}

/* onnx_info(id) → JSON */
static LXValue bi_onnx_info(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m;
    char* j;
    (void)ctx;
    if (nargs != 1) px_error("R1002: onnx_info 需要 1 个参数 (id)");
    m = onnx_slot_get((int)args[0].as.i);
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
    m = onnx_slot_get((int)args[0].as.i);
    if (!m) return px_err(px_str("onnx: 无效或已关闭的模型 id"));
    if (args[1].type != PX_STR) return px_err(px_str("onnx: 名字必须是字符串"));
    t = onnx_find_init(m, px_val_cstr(args[1]));
    if (!t) return px_err(px_str("onnx: 没有该 initializer"));
    dims = px_list(0);
    {
        int i;
        for (i = 0; i < t->ndim; i++) px_list_push(dims, px_int((int64_t)t->dims[i]));
    }
    d = px_dict();
    px_dict_set(d, "name", px_str(t->name ? t->name : ""));
    px_dict_set(d, "dtype", px_str(onnx_dtype_name(t->dtype)));
    px_dict_set(d, "dtype_id", px_int(t->dtype));
    px_dict_set(d, "dims", dims);
    px_dict_set(d, "nbytes", px_int((int64_t)t->nbytes));
    px_dict_set(d, "data", t->data ? px_bytes_len(t->data, (int)t->nbytes) : px_bytes_len("", 0));
    return d;
}

/* onnx_initializer_names(id) → [str] */
static LXValue bi_onnx_initializer_names(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m;
    LXValue lst;
    int i;
    (void)ctx;
    if (nargs != 1) px_error("R1002: onnx_initializer_names 需要 1 个参数 (id)");
    m = onnx_slot_get((int)args[0].as.i);
    if (!m) return px_err(px_str("onnx: 无效或已关闭的模型 id"));
    lst = px_list(0);
    for (i = 0; i < m->n_init; i++)
        px_list_push(lst, px_str(m->inits[i].name ? m->inits[i].name : ""));
    return lst;
}

/* onnx_model_close(id) → bool */
static LXValue bi_onnx_model_close(LXValue* args, int nargs, void* ctx) {
    OnnxModel* m = NULL;
    char* path = NULL;
    (void)ctx;
    if (nargs != 1) px_error("R1002: onnx_model_close 需要 1 个参数 (id)");
    if (!onnx_slot_take((int)args[0].as.i, &m, &path)) return px_bool(0);
    free(path);
    onnx_model_free(m);
    return px_bool(1);
}

/* 供 runtime.c 注册调用（与 bi_* 同名但跨编译单元可见） */
void px_onnx_register(void) {
    px_set_global("onnx_model_open", px_native("onnx_model_open", bi_onnx_model_open));
    px_set_global("onnx_info", px_native("onnx_info", bi_onnx_info));
    px_set_global("onnx_initializer", px_native("onnx_initializer", bi_onnx_initializer));
    px_set_global("onnx_initializer_names",
                  px_native("onnx_initializer_names", bi_onnx_initializer_names));
    px_set_global("onnx_model_close", px_native("onnx_model_close", bi_onnx_model_close));
}

#endif /* PX_NO_ONNX */
