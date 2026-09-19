/* onnx_exec.c —— ONNX 图执行器（M157，零依赖）
 *
 * 设计取舍（为什么是"数据到位就执行"而不是先做一次 Kahn 拓扑排序）：
 *   · ONNX 规范要求 node 已按拓扑序排列，但**不依赖这个假设**更安全（有些导出器会给
 *     看似乱序的图）；这里用「反复扫描：输入齐了就执行」，天然容忍乱序，并能把
 *     「缺哪个输入 / 哪个算子没实现」直接报出来（而不是死循环）。
 *   · 每轮至少要有 1 个节点可执行，否则判失败 —— 这就是环检测。
 *
 * 内存：所有中间张量都挂在竞技场（OnnxArena）上，结果对象持有竞技场 ⇒
 *   一次 run 的中间张量随结果一起释放（780 节点的图会产出上千个中间张量）。
 *
 * 名字解析：ONNX 允许空名字（可选输入/未用输出）⇒ 空串一律当"没有"。
 *
 * 诊断：环境变量 `PX_ONNX_TRACE=1` 时把"节点 → 输入形状 → 输出形状"打到 stderr。
 *   真实模型（780 节点）出错时，靠它一眼看出是**哪个上游**把形状做歪了
 *   —— 只报"某个 Mul 广播失败"是不够的（M157 实测：真模型第一个错就出在这里）。
 */
#include "onnx_tensor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── 名字 → 张量 的环境（线性表；一张 780 节点的图约 1500 个名字，
 *    查找总量 ~1e6 次 strcmp，实测可忽略；换来的是零哈希实现与易读） ── */
typedef struct { char* name; OnnxT* t; } EnvEntry;

static int onnx_trace_on(void) {
    static int cached = -1;
    if (cached < 0) {
        const char* e = getenv("PX_ONNX_TRACE");
        cached = (e && e[0] && e[0] != '0') ? 1 : 0;
    }
    return cached;
}

static void onnx_trace_shapes(const char* tag, const OnnxT* t) {
    int i;
    fprintf(stderr, "%s[", tag);
    for (i = 0; i < t->ndim; i++) fprintf(stderr, "%s%lld", i ? "," : "", (long long)t->dims[i]);
    fprintf(stderr, "]");
}

typedef struct {
    EnvEntry* v;
    int       n, cap;
} Env;

static OnnxT* env_get(const Env* e, const char* name) {
    int i;
    if (!name || !*name) return NULL;
    for (i = 0; i < e->n; i++) if (strcmp(e->v[i].name, name) == 0) return e->v[i].t;
    return NULL;
}

static int env_has(const Env* e, const char* name) {
    return env_get(e, name) != NULL;
}

static int env_set(Env* e, const char* name, OnnxT* t) {
    if (!name || !*name) return 0;
    if (e->n == e->cap) {
        int c = e->cap ? e->cap * 2 : 256;
        EnvEntry* nv = (EnvEntry*)realloc(e->v, (size_t)c * sizeof(EnvEntry));
        if (!nv) return -1;
        e->v = nv; e->cap = c;
    }
    e->v[e->n].name = strdup(name);
    e->v[e->n].t = t;
    e->n++;
    return 0;
}

/* ── 结果对象 ── */
typedef struct {
    OnnxRunResult pub;
    OnnxArena     arena;
    OnnxT**       outs;
} Run;

static void run_free(Run* r) {
    if (!r) return;
    onnx_arena_free(&r->arena);
    free(r->pub.outputs);
    free(r->pub.op_counts);
    free(r->outs);
    free(r);
}

void onnx_exec_result_free(OnnxRunResult* p) {
    if (!p) return;
    run_free((Run*)p);   /* pub 是 Run 的首成员 ⇒ 指针可回退 */
}

int onnx_exec_find_output(const OnnxRunResult* r, const OnnxModel* m, const char* name) {
    int i;
    for (i = 0; i < r->n_out && i < m->n_out; i++)
        if (strcmp(m->outputs[i].name, name) == 0) return i;
    return -1;
}

char* onnx_exec_stats_json(const OnnxRunResult* r) {
    size_t cap = 256;
    int i, first = 1;
    char* s = (char*)malloc(cap);
    size_t n = 0;
    if (!s) return NULL;
    n += (size_t)sprintf(s + n, "{\"nodes\":%d,\"by_op\":{", r->nodes_run);
    for (i = 0; i < r->n_ops; i++) {
        if (r->op_counts[i] == 0) continue;
        n += (size_t)sprintf(s + n, "%s\"%s\":%d", first ? "" : ",", onnx_op_name_at(i), r->op_counts[i]);
        first = 0;
    }
    sprintf(s + n, "}}");
    return s;
}

/* ── Constant 节点：从属性造张量（value / value_float / value_int / value_floats / value_ints） ── */
static OnnxT* node_constant(OnnxArena* ar, const OnnxAttr* attrs, int n_attr, char* err, size_t errc) {
    const OnnxTensor* vt = onnx_attr_t(attrs, n_attr, "value");
    int cnt = 0;
    if (vt) {
        OnnxT* t = onnx_arena_new(ar, vt->dtype, vt->ndim, vt->dims);
        if (!t) { snprintf(err, errc, "Constant: 分配失败"); return NULL; }
        if (vt->data && vt->nbytes) memcpy(t->data, vt->data, t->nbytes);
        return t;
    }
    {
        const int64_t* is = onnx_attr_is(attrs, n_attr, "value_ints", &cnt);
        if (is && cnt > 0) {
            int64_t d[1] = { cnt };
            OnnxT* t = onnx_arena_new(ar, ONNX_DT_INT64, 1, d);
            int i;
            if (!t) { snprintf(err, errc, "Constant: 分配失败"); return NULL; }
            for (i = 0; i < cnt; i++) onnx_t_seti(t, i, is[i]);
            return t;
        }
    }
    {
        const double* fs = onnx_attr_fs(attrs, n_attr, "value_floats", &cnt);
        if (fs && cnt > 0) {
            int64_t d[1] = { cnt };
            OnnxT* t = onnx_arena_new(ar, ONNX_DT_FLOAT, 1, d);
            int i;
            if (!t) { snprintf(err, errc, "Constant: 分配失败"); return NULL; }
            for (i = 0; i < cnt; i++) onnx_t_setf(t, i, fs[i]);
            return t;
        }
    }
    {
        int64_t d[1] = { 1 };
        OnnxT* t;
        /* 属性存在性：借助 i / f 的默认值区分（value_int / value_float） */
        {
            const OnnxAttr* a = NULL;
            int i;
            for (i = 0; i < n_attr; i++) if (attrs[i].name && strcmp(attrs[i].name, "value_int") == 0) a = &attrs[i];
            if (a) {
                t = onnx_arena_new(ar, ONNX_DT_INT64, 1, d);
                if (!t) { snprintf(err, errc, "Constant: 分配失败"); return NULL; }
                onnx_t_seti(t, 0, a->i);
                return t;
            }
            for (i = 0; i < n_attr; i++) if (attrs[i].name && strcmp(attrs[i].name, "value_float") == 0) a = &attrs[i];
            if (a) {
                t = onnx_arena_new(ar, ONNX_DT_FLOAT, 1, d);
                if (!t) { snprintf(err, errc, "Constant: 分配失败"); return NULL; }
                onnx_t_setf(t, 0, a->f);
                return t;
            }
        }
    }
    snprintf(err, errc, "Constant: 无可识别的 value 属性");
    return NULL;
}

/* ── 主执行 ── */
OnnxRunResult* onnx_exec_run(const OnnxModel* m, const char** feed_names, OnnxT** feed_vals,
                             int n_feed, char** err_out) {
    Run* r = (Run*)calloc(1, sizeof(Run));
    Env env;
    unsigned char* done;
    char err[ONNX_ERRC];
    int i, round, remaining, progress;

    if (!r) { if (err_out) *err_out = strdup("onnx: 分配失败"); return NULL; }
    env.v = NULL; env.n = 0; env.cap = 0;
    onnx_arena_init(&r->arena);
    err[0] = 0;

    if (!m) {
        snprintf(err, sizeof(err), "onnx: 模型为空");
        goto fail;
    }

    /* 1) initializer 进环境（先放，图输入同名时以 initializer 为准，符合 ONNX 语义） */
    for (i = 0; i < m->n_init; i++) {
        OnnxT* t = onnx_t_from_init(&m->inits[i]);
        OnnxT* keep;
        if (!t) { snprintf(err, sizeof(err), "onnx: initializer %s 分配失败", m->inits[i].name); goto fail; }
        if (r->arena.n == r->arena.cap) {
            int c = r->arena.cap ? r->arena.cap * 2 : 128;
            OnnxT** nv = (OnnxT**)realloc(r->arena.v, (size_t)c * sizeof(OnnxT*));
            if (!nv) { onnx_t_free(t); snprintf(err, sizeof(err), "onnx: 竞技场扩容失败"); goto fail; }
            r->arena.v = nv; r->arena.cap = c;
        }
        r->arena.v[r->arena.n++] = t;
        keep = t;
        if (env_set(&env, m->inits[i].name, keep)) { snprintf(err, sizeof(err), "onnx: 环境分配失败"); goto fail; }
    }

    /* 2) feed 进环境（拷贝） */
    for (i = 0; i < n_feed; i++) {
        OnnxT* t;
        if (!feed_names[i] || !*feed_names[i]) continue;
        t = feed_vals[i] ? onnx_t_clone(feed_vals[i]) : NULL;
        if (!t) { snprintf(err, sizeof(err), "onnx: feed %s 拷贝失败", feed_names[i]); goto fail; }
        if (r->arena.n == r->arena.cap) {
            int c = r->arena.cap ? r->arena.cap * 2 : 128;
            OnnxT** nv = (OnnxT**)realloc(r->arena.v, (size_t)c * sizeof(OnnxT*));
            if (!nv) { onnx_t_free(t); snprintf(err, sizeof(err), "onnx: 竞技场扩容失败"); goto fail; }
            r->arena.v = nv; r->arena.cap = c;
        }
        r->arena.v[r->arena.n++] = t;
        if (env_set(&env, feed_names[i], t)) { snprintf(err, sizeof(err), "onnx: 环境分配失败"); goto fail; }
    }

    /* 3) 检查每个图输入都有来源 */
    for (i = 0; i < m->n_in; i++) {
        if (!env_has(&env, m->inputs[i].name)) {
            snprintf(err, sizeof(err), "onnx: 缺少图输入 %s（既非 initializer 也未 feed）", m->inputs[i].name);
            goto fail;
        }
    }

    /* 4) 按"输入齐了就执行"的顺序跑（容忍乱序 + 环检测） */
    done = (unsigned char*)calloc((size_t)(m->n_node > 0 ? m->n_node : 1), 1);
    if (!done) { snprintf(err, sizeof(err), "onnx: 分配失败"); goto fail; }
    r->pub.n_ops = onnx_op_count();
    r->pub.op_counts = (int*)calloc((size_t)r->pub.n_ops, sizeof(int));
    if (!r->pub.op_counts) { free(done); snprintf(err, sizeof(err), "onnx: 分配失败"); goto fail; }
    remaining = m->n_node;
    for (round = 0; remaining > 0 && round <= m->n_node + 1; round++) {
        progress = 0;
        for (i = 0; i < m->n_node; i++) {
            const OnnxNode* nd = &m->nodes[i];
            OnnxT* in[64];
            OnnxT* out[64];
            int j, ready = 1, nout;
            const OnnxOpEntry* op;
            if (done[i]) continue;
            nout = nd->n_out;
            if (nout > 64) { snprintf(err, sizeof(err), "onnx: 节点 %s 输出过多（%d）", nd->name ? nd->name : nd->op_type, nout); goto fail2; }
            if (nd->n_in > 64) { snprintf(err, sizeof(err), "onnx: 节点 %s 输入过多（%d）", nd->name ? nd->name : nd->op_type, nd->n_in); goto fail2; }
            /* Constant 天生无输入 */
            if (strcmp(nd->op_type, "Constant") == 0) {
                out[0] = node_constant(&r->arena, nd->attrs, nd->n_attr, err, sizeof(err));
                if (!out[0]) goto fail2;
                for (j = 0; j < nout; j++) {
                    if (out[j] && env_set(&env, nd->out[j], out[j])) { snprintf(err, sizeof(err), "onnx: 环境分配失败"); goto fail2; }
                }
                done[i] = 1; remaining--; progress++; r->pub.nodes_run++;
                continue;
            }
            for (j = 0; j < nd->n_in; j++) {
                const char* nm = nd->in[j];
                if (!nm || !*nm) { in[j] = NULL; continue; }
                in[j] = env_get(&env, nm);
                if (!in[j]) { ready = 0; break; }
            }
            if (!ready) continue;
            op = onnx_op_lookup(nd->op_type);
            if (!op) {
                snprintf(err, sizeof(err), "onnx: 未实现的算子 %s（节点 %s）",
                         nd->op_type, nd->name ? nd->name : "-");
                goto fail2;
            }
            for (j = 0; j < nout; j++) out[j] = NULL;
            if (onnx_trace_on()) {
                fprintf(stderr, "[onnx] #%d %s %s", i, nd->op_type, nd->name ? nd->name : "-");
                for (j = 0; j < nd->n_in; j++) {
                    if (!in[j]) { fprintf(stderr, " (nil)"); continue; }
                    onnx_trace_shapes(" ", in[j]);
                }
                fprintf(stderr, "\n");
            }
            if (op->fn(&r->arena, out, nout, in, nd->n_in, nd->attrs, nd->n_attr, err, sizeof(err))) {
                /* 补上下文（算子自己的 err 说"为什么"） */
                char inner[ONNX_ERRC];
                char opn[64];
                snprintf(inner, sizeof(inner), "%s", err);
                snprintf(opn, sizeof(opn), "%s", nd->op_type);
                snprintf(err, sizeof(err), "onnx: 节点 %.40s（op=%.20s）执行失败：%.80s",
                         nd->name ? nd->name : "-", opn, inner);
                goto fail2;
            }
            for (j = 0; j < nout; j++) {
                if (out[j] && env_set(&env, nd->out[j], out[j])) { snprintf(err, sizeof(err), "onnx: 环境分配失败"); goto fail2; }
            }
            for (j = 0; j < r->pub.n_ops; j++)
                if (strcmp(onnx_op_name_at(j), nd->op_type) == 0) { r->pub.op_counts[j]++; break; }
            done[i] = 1; remaining--; progress++; r->pub.nodes_run++;
        }
        if (remaining > 0 && progress == 0) {
            /* 找出第一个卡住的节点，报它缺哪个输入 */
            for (i = 0; i < m->n_node; i++) {
                int j;
                if (done[i]) continue;
                for (j = 0; j < m->nodes[i].n_in; j++) {
                    const char* nm = m->nodes[i].in[j];
                    if (nm && *nm && !env_has(&env, nm)) {
                        snprintf(err, sizeof(err),
                                 "onnx: 图无法继续 —— 节点 %s（op=%s）的输入 %s 无来源（环或未声明）",
                                 m->nodes[i].name ? m->nodes[i].name : "-", m->nodes[i].op_type, nm);
                        goto fail2;
                    }
                }
                break;
            }
            snprintf(err, sizeof(err), "onnx: 图无法继续（剩余 %d 个节点，疑似环）", remaining);
            goto fail2;
        }
    }
    free(done);
    done = NULL;
    if (remaining > 0) { snprintf(err, sizeof(err), "onnx: 迭代轮数超限（剩余 %d 节点）", remaining); goto fail; }

    /* 5) 取图输出 */
    r->pub.n_out = m->n_out;
    r->pub.outputs = (OnnxT**)calloc((size_t)(m->n_out > 0 ? m->n_out : 1), sizeof(OnnxT*));
    if (!r->pub.outputs) { snprintf(err, sizeof(err), "onnx: 分配失败"); goto fail; }
    for (i = 0; i < m->n_out; i++) {
        OnnxT* t = env_get(&env, m->outputs[i].name);
        if (!t) { snprintf(err, sizeof(err), "onnx: 图输出 %s 未产出", m->outputs[i].name); goto fail; }
        r->pub.outputs[i] = t;
    }
    for (i = 0; i < env.n; i++) free(env.v[i].name);
    free(env.v);
    return &r->pub;

fail2:
    free(done);
fail:
    for (i = 0; i < env.n; i++) free(env.v[i].name);
    free(env.v);
    run_free(r);
    if (err_out) *err_out = strdup(err[0] ? err : "onnx: 执行失败");
    return NULL;
}
