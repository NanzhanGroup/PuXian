// 普贤 (PuXian) C 运行时 — 路由表 + 中间件（M28）
// 语言层 API（与解释器 web.rs 双模式一致）：
//   route(method, pattern, handler) → bool   注册路由
//     pattern 支持：字面段 /api/users；参数段 :id → params["id"]；通配段 *（匹配剩余含 /）
//     method "*" 匹配任意方法
//   middleware(fn) → bool                    注册中间件（按注册顺序执行）
//     fn(req) 返回 null 继续；非 null（int/str/dict）短路作为响应
//   px_route_has() / px_route_try_dispatch(PxHttpOut* out, req, method, head_only)
//     —— 供 runtime.c px_http_dispatch 调用：路由表非空且匹配 → 执行中间件链 + handler
//        并经 PxHttpOut 发送响应（M53-S2：HTTP/1.1 与 HTTP/3 共用输出抽象）。
// 响应归一化（同解释器 normalize_route_resp）：
//   int → 状态码（空 body）；null → 204；str → 200 text/plain；
//   dict{status, headers, body} → 完整控制
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// M100：VM 函数判定（px_vm_entry 为 runtime.c weak extern —— C 轨逃生舱无 vm.o → NULL）
extern LXValue px_vm_entry(LXValue* args, int nargs, void* ctx) __attribute__((weak));

#define MAX_ROUTES 128
#define MAX_MIDDLEWARES 32

typedef enum { SEG_LIT, SEG_PARAM, SEG_WILD } SegKind;
typedef struct {
    char seg[256];
    SegKind kind;
} PxRouteSeg;

typedef struct {
    char method[16];
    PxRouteSeg segs[32];
    int nsegs;
    LXValue handler;
    // 路由 key（限流桶前缀）："METHOD pattern"
    char pattern[300];
    // M33：per-route 限流（按来源 IP；max 次 / window_sec 秒 → 超限 429；0 = 未启用）
    long long rate_max;
    long long rate_window;
    int active;
} PxRoute;

static PxRoute g_routes[MAX_ROUTES];
static LXValue g_middlewares[MAX_MIDDLEWARES];
static int g_mw_count = 0;
static pthread_mutex_t g_route_mu = PTHREAD_MUTEX_INITIALIZER;

// ==================== 路由注册 ====================

static void route_parse_pattern(const char* pattern, PxRouteSeg* segs, int* nsegs, char* err, size_t errsz) {
    *nsegs = 0;
    const char* p = pattern;
    while (*p == '/') p++;
    if (*p == 0) return;
    while (*p) {
        const char* slash = strchr(p, '/');
        int len = slash ? (int)(slash - p) : (int)strlen(p);
        if (*nsegs >= 32) { snprintf(err, errsz, "路由段过多: %s", pattern); return; }
        char seg[256];
        int cl = len < 255 ? len : 255;
        memcpy(seg, p, (size_t)cl);
        seg[cl] = 0;
        if (strcmp(seg, "*") == 0) {
            segs[*nsegs].kind = SEG_WILD;
            segs[*nsegs].seg[0] = 0;
        } else if (seg[0] == ':') {
            segs[*nsegs].kind = SEG_PARAM;
            snprintf(segs[*nsegs].seg, sizeof(segs[*nsegs].seg), "%s", seg + 1);
        } else {
            segs[*nsegs].kind = SEG_LIT;
            snprintf(segs[*nsegs].seg, sizeof(segs[*nsegs].seg), "%s", seg);
        }
        (*nsegs)++;
        p = slash ? slash + 1 : p + len;
        while (*p == '/' && slash) p++;
    }
}

// route(method, pattern, handler[, opts]) → bool
// opts（M33）：{rate_limit:{max,window_sec}} → 该路由按来源 IP 独立限流（超限 429）
LXValue bi_route(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 && nargs != 4) px_error("route 需要 (method, pattern, handler[, opts]) 参数");
    if (args[0].type != PX_STR || args[1].type != PX_STR) px_error("route 的 method/pattern 需要字符串");
    if (args[2].type != PX_FUNC) px_error("route 的 handler 必须是函数");
    char err[256] = {0};
    PxRouteSeg segs[32];
    int nsegs = 0;
    route_parse_pattern(args[1].as.obj->as.str.data, segs, &nsegs, err, sizeof(err));
    if (err[0]) px_error("%s", err);
    // M33.1：解析 opts{rate_limit:{max,window_sec}}
    long long rate_max = 0, rate_window = 0;
    if (nargs == 4) {
        if (args[3].type != PX_DICT) px_error("route 的第 4 参数 opts 需要 dict");
        LXValue rl = px_dict_get(args[3], "rate_limit");
        if (rl.type == PX_DICT) {
            LXValue m = px_dict_get(rl, "max");
            LXValue w = px_dict_get(rl, "window_sec");
            if (m.type == PX_INT && w.type == PX_INT && m.as.i >= 1 && w.as.i >= 1) {
                rate_max = m.as.i;
                rate_window = w.as.i;
            }
        }
    }
    pthread_mutex_lock(&g_route_mu);
    int slot = -1;
    for (int i = 0; i < MAX_ROUTES; i++) if (!g_routes[i].active) { slot = i; break; }
    if (slot < 0) {
        pthread_mutex_unlock(&g_route_mu);
        px_error("路由数量超出上限 %d", MAX_ROUTES);
    }
    memset(&g_routes[slot], 0, sizeof(PxRoute));
    // 大写 method（"*" 保持）
    const char* m = args[0].as.obj->as.str.data;
    if (strcmp(m, "*") == 0) {
        snprintf(g_routes[slot].method, sizeof(g_routes[slot].method), "*");
    } else {
        int i = 0;
        for (; m[i] && i < 14; i++) {
            char c = m[i];
            g_routes[slot].method[i] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
        }
        g_routes[slot].method[i] = 0;
    }
    memcpy(g_routes[slot].segs, segs, sizeof(PxRouteSeg) * (size_t)nsegs);
    g_routes[slot].nsegs = nsegs;
    g_routes[slot].handler = args[2];
    // 路由 key：大写 method + " " + pattern（限流桶前缀）
    {
        const char* mkey = g_routes[slot].method[0] ? g_routes[slot].method : "*";
        snprintf(g_routes[slot].pattern, sizeof(g_routes[slot].pattern), "%s %s",
                 mkey, args[1].as.obj->as.str.data);
    }
    g_routes[slot].rate_max = rate_max;
    g_routes[slot].rate_window = rate_window;
    g_routes[slot].active = 1;
    pthread_mutex_unlock(&g_route_mu);
    return px_bool(true);
}

// middleware(fn) → bool
LXValue bi_middleware(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("middleware 需要 (fn) 参数");
    if (args[0].type != PX_FUNC) px_error("middleware 的参数必须是函数");
    pthread_mutex_lock(&g_route_mu);
    if (g_mw_count >= MAX_MIDDLEWARES) {
        pthread_mutex_unlock(&g_route_mu);
        px_error("中间件数量超出上限 %d", MAX_MIDDLEWARES);
    }
    g_middlewares[g_mw_count++] = args[0];
    pthread_mutex_unlock(&g_route_mu);
    return px_bool(true);
}

// ==================== 匹配 ====================

int px_route_has(void) {
    pthread_mutex_lock(&g_route_mu);
    int has = 0;
    for (int i = 0; i < MAX_ROUTES; i++) if (g_routes[i].active) { has = 1; break; }
    pthread_mutex_unlock(&g_route_mu);
    return has;
}

// 匹配路由：成功返回 1 并输出 handler/params/限流配置/路由 key；失败 0
static int route_match(const char* method, const char* path, LXValue* handler_out,
                       LXValue* params_out, long long* rate_max_out, long long* rate_window_out,
                       const char** pattern_out) {
    int found = 0;
    pthread_mutex_lock(&g_route_mu);
    // 大写 method
    char mup[16];
    int mi = 0;
    for (; method[mi] && mi < 14; mi++) {
        char c = method[mi];
        mup[mi] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
    }
    mup[mi] = 0;
    // 拆分路径段（URL 已解码）
    char pathcopy[2048];
    snprintf(pathcopy, sizeof(pathcopy), "%s", path);
    char* parts[128];
    int nparts = 0;
    char* save = NULL;
    for (char* t = strtok_r(pathcopy, "/", &save); t && nparts < 128; t = strtok_r(NULL, "/", &save)) {
        parts[nparts++] = t;
    }
    for (int i = 0; i < MAX_ROUTES && !found; i++) {
        if (!g_routes[i].active) continue;
        if (strcmp(g_routes[i].method, "*") != 0 && strcmp(g_routes[i].method, mup) != 0) continue;
        LXValue params = px_dict();
        px_root_push();   // M92-S2c precise：route_match 单次匹配登记作用域
        PX_KEEP(params);   // M92-S2c precise：route 匹配 params 裸局部跨 px_dict_set/px_str
        int ok = 1;
        int pi = 0;
        for (int s = 0; s < g_routes[i].nsegs; s++) {
            PxRouteSeg* seg = &g_routes[i].segs[s];
            if (seg->kind == SEG_LIT) {
                if (pi >= nparts || strcmp(parts[pi], seg->seg) != 0) { ok = 0; break; }
                pi++;
            } else if (seg->kind == SEG_PARAM) {
                if (pi >= nparts || parts[pi][0] == 0) { ok = 0; break; }
                px_dict_set(params, seg->seg, px_str(parts[pi]));
                pi++;
            } else { // WILD
                char rest[2048] = {0};
                for (int j = pi; j < nparts; j++) {
                    if (j > pi) strcat(rest, "/");
                    strcat(rest, parts[j]);
                }
                px_dict_set(params, "wildcard", px_str(rest));
                pi = nparts;
            }
        }
        if (ok && pi >= nparts) {
            if (handler_out) *handler_out = g_routes[i].handler;
            if (params_out) *params_out = params;
            if (rate_max_out) *rate_max_out = g_routes[i].rate_max;
            if (rate_window_out) *rate_window_out = g_routes[i].rate_window;
            if (pattern_out) *pattern_out = g_routes[i].pattern;
            found = 1;
        }
        px_root_pop();   // M92-S2c precise
    }
    pthread_mutex_unlock(&g_route_mu);
    return found;
}

// ==================== 响应归一化 + 发送 ====================
// M53-S2：路由/中间件响应统一经 PxHttpOut 输出（status/ct/body/extra → HTTP/1.1 文本头
// 或 HTTP/3 HEADERS+DATA 帧）；reason 表与 HTTP/1.1 管道共用 px_http_status_reason。
static void route_send(PxHttpOut* out, int status, const char* ct, const char* body,
                       int body_len, int head_only, int keep_alive,
                       const char* extra_headers) {
    out->respond(out, status, ct, body, body_len, head_only, keep_alive, extra_headers);
}

// 归一化响应值 → (status, content_type, body, body_len)
typedef struct {
    int status;
    const char* ct;      // 默认 Content-Type
    const char* body;    // body 指针（可能是 Value 内部缓冲，发送前有效）
    int body_len;
} RouteResp;

static void route_normalize(LXValue v, RouteResp* r) {
    r->status = 200;
    r->ct = "text/plain; charset=utf-8";
    r->body = "";
    r->body_len = 0;
    switch (v.type) {
        case PX_INT:
            r->status = (int)v.as.i;
            break;
        case PX_NULL:
            r->status = 204;
            break;
        case PX_STR:
            r->body = v.as.obj->as.str.data;
            r->body_len = v.as.obj->as.str.len;
            break;
        case PX_BYTES:
            r->body = (const char*)v.as.obj->as.str.data;
            r->body_len = v.as.obj->as.str.len;
            r->ct = "application/octet-stream";
            break;
        case PX_DICT: {
            LXValue st = px_dict_get(v, "status");
            if (st.type == PX_INT) r->status = (int)st.as.i;
            LXValue b = px_dict_get(v, "body");
            if (b.type == PX_STR) {
                r->body = b.as.obj->as.str.data;
                r->body_len = b.as.obj->as.str.len;
            } else if (b.type == PX_BYTES) {
                r->body = (const char*)b.as.obj->as.str.data;
                r->body_len = b.as.obj->as.str.len;
                r->ct = "application/octet-stream";
            } else if (b.type == PX_NULL) {
                r->body = "";
                r->body_len = 0;
            }
            LXValue h = px_dict_get(v, "headers");
            if (h.type == PX_DICT) {
                LXValue ct = px_dict_get_ci(h, "Content-Type");
                if (ct.type == PX_STR) r->ct = ct.as.obj->as.str.data;
            }
            break;
        }
        default: {
            char* s = px_to_string(v);
            r->body = s;
            r->body_len = (int)strlen(s);
            break;
        }
    }
}

// ==================== 请求分派（runtime.c px_conn_worker 调用） ====================

// M98-S2a：route handler 返回值 → 归一化 + 发送 + 访问日志（段2）。
//   同步路径（px_route_try_dispatch 内 px_call 后）与协程续处理（px_serve 续 worker
//   从挂起表取回 stage2 后）共用 —— async/sync 响应语义逐字节一致。
void px_route_respond(PxHttpOut* out, LXValue req, const char* method, int head_only,
                      int keep_alive, const char* req_id, LXValue resp) {
    LXValue path_v = px_dict_get(req, "path");
    const char* pstr = (path_v.type == PX_STR) ? path_v.as.obj->as.str.data : "?";
    RouteResp rr;
    route_normalize(resp, &rr);
    fprintf(stderr, "[px-serve] [route] %s %s -> %d\n", method, pstr, rr.status);
    char rsp_extra[PX_HDR_EXTRA_CAP];
    int rsp_off = snprintf(rsp_extra, sizeof(rsp_extra), "X-Request-Id: %s\r\n", req_id);
    if (rsp_off < 0 || rsp_off >= (int)sizeof(rsp_extra)) rsp_off = (int)sizeof(rsp_extra) - 1;
    // M109-S1/S3：route() handler 的 headers 与 vhost 走**同一套判定**
    //   （拒绝名单 + CRLF 防护 + 预算）。原实现只取 Content-Type，其余全丢 →
    //   301 连 Location 都发不出（qg-issue 36 的 route 同族缺陷）。
    {
        LXValue rh = px_dict_get(resp, "headers");
        if (rh.type == PX_DICT)
            rsp_off = px_hdr_append(rh, rsp_extra, rsp_off, (int)sizeof(rsp_extra), 1, NULL);
    }
    route_send(out, rr.status, rr.ct, rr.body, rr.body_len, head_only, keep_alive, rsp_extra);
    // M36：route 响应统一访问日志（与解释器 log_access 一致）
    {
        LXValue rmt = px_dict_get(req, "remote");
        const char* lr = (rmt.type == PX_STR) ? rmt.as.obj->as.str.data : "-";
        px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                (long long)time(NULL), lr, method, pstr, rr.status, rr.body_len, req_id);
    }
}

// M100：middleware 短路响应（归一化 + respond + (middleware) 访问日志）。同步短路
//   （px_route_try_dispatch 内 px_call 后）与协程续处理段2（px_serve kind=3，续 worker
//   从挂起表取回）共用 —— 保证 async/sync 短路语义逐字节一致（文案含 "(middleware)"）。
void px_route_mw_short_respond(PxHttpOut* out, LXValue req, const char* method, int head_only,
                               int keep_alive, const char* req_id, LXValue r) {
    LXValue path_v = px_dict_get(req, "path");
    const char* pstr = (path_v.type == PX_STR) ? path_v.as.obj->as.str.data : "?";
    RouteResp rr;
    route_normalize(r, &rr);
    fprintf(stderr, "[px-serve] [route] %s %s -> %d (middleware)\n", method, pstr, rr.status);
    char rsp_extra[PX_HDR_EXTRA_CAP];
    int rsp_off = snprintf(rsp_extra, sizeof(rsp_extra), "X-Request-Id: %s\r\n", req_id);
    if (rsp_off < 0 || rsp_off >= (int)sizeof(rsp_extra)) rsp_off = (int)sizeof(rsp_extra) - 1;
    {
        LXValue rh = px_dict_get(r, "headers");     // M109-S1/S3：与 vhost 同一套判定
        if (rh.type == PX_DICT)
            rsp_off = px_hdr_append(rh, rsp_extra, rsp_off, (int)sizeof(rsp_extra), 1, NULL);
    }
    route_send(out, rr.status, rr.ct, rr.body, rr.body_len, head_only, keep_alive, rsp_extra);
    // M36：middleware 短路响应统一访问日志（与解释器 log_access 一致）
    {
        LXValue rmt = px_dict_get(req, "remote");
        const char* lr = (rmt.type == PX_STR) ? rmt.as.obj->as.str.data : "-";
        px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                (long long)time(NULL), lr, method, pstr, rr.status, rr.body_len, req_id);
    }
}

// 执行中间件链 + handler 并发送响应。返回 0 = 未匹配；1 = 已处理（同步完成响应）；
// 2 = 已拆段（route VM handler 帧协程运行中，调用方须释放 worker，done 回调投回续处理）。
int px_route_try_dispatch(PxHttpOut* out, LXValue req, const char* method, int head_only,
                          int keep_alive, const char* req_id, int async_ok) {
    LXValue path_v = px_dict_get(req, "path");
    if (path_v.type != PX_STR) return 0;
    LXValue handler, params;
    long long rate_max = 0, rate_window = 0;
    const char* route_pattern = NULL;
    if (!route_match(method, path_v.as.obj->as.str.data, &handler, &params,
                     &rate_max, &rate_window, &route_pattern)) return 0;

    // M33.1：per-route 限流——匹配路由后按 "路由|IP" 计数（各路由独立桶），超限 429
    if (rate_max > 0 && rate_window > 0) {
        char ipbuf[64];
        LXValue rmt = px_dict_get(req, "remote");
        const char* rs = (rmt.type == PX_STR) ? rmt.as.obj->as.str.data : "";
        snprintf(ipbuf, sizeof(ipbuf), "%s", rs);
        char* colon = strrchr(ipbuf, ':');
        if (colon && colon[1] >= '0' && colon[1] <= '9') *colon = 0; // 去端口
        if (ipbuf[0] == '[') { char* br = strchr(ipbuf, ']'); if (br) { br++; *br = 0; memmove(ipbuf, ipbuf + 1, strlen(ipbuf)); } }
        char rkey[420];
        snprintf(rkey, sizeof(rkey), "route:%s|%s",
                 route_pattern ? route_pattern : "?", ipbuf);
        if (!px_rate_limit_try(rkey, rate_max, rate_window)) {
            char extra[512];
            int el = snprintf(extra, sizeof(extra), "X-Request-Id: %s\r\n", req_id);
            if (g_px_alt_svc[0]) el += snprintf(extra + el, sizeof(extra) - (size_t)el, "Alt-Svc: %s\r\n", g_px_alt_svc);
            route_send(out, 429, "text/plain; charset=utf-8", "429 Too Many Requests", 21,
                       head_only, keep_alive, extra);
            // M33：per-route 429 也记访问日志（格式同解释器）
            px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                    (long long)time(NULL), ipbuf, method,
                    path_v.as.obj->as.str.data, 429, 21, req_id);
            return 1;
        }
    }

    // 中间件链：fn(req) → null 继续 / 非 null 短路
    pthread_mutex_lock(&g_route_mu);
    int mw_count = g_mw_count;
    LXValue mws[MAX_MIDDLEWARES];
    if (mw_count > 0) memcpy(mws, g_middlewares, sizeof(LXValue) * (size_t)mw_count);
    pthread_mutex_unlock(&g_route_mu);
    px_root_push();   // M92-S2c precise：px_route_try_dispatch 登记作用域
    PX_KEEP(params);   // M92-S2c precise：route_match 传出 params（跨中间件/handler px_call）
    // M100：middleware 链协程化 —— 链非空且每段 middleware 与 handler 均为 VM 函数
    //   （fn==px_vm_entry，可帧协程让出）且 async_ok → 链状态机 defer
    //   （px_pxserve_mw_defer：登记 kind=2 + 链快照入 GC 根 + spawn 首段；done 回调逐段
    //   推进：null → 下一 middleware / 全 null → handler 段 / 非 null → 短路 kind=3 段2）
    //   → 返回 2（调用方释放 worker，占协程不占线程）。含 C 闭包 middleware 段 / 非 VM
    //   handler / async_ok=0 / 无协程内核 / 登记失败 → 回落下方原同步链（M98 行为零变化）。
    if (async_ok && mw_count > 0) {
        int all_vm = (handler.type == PX_FUNC && handler.as.obj &&
                      px_vm_entry && handler.as.obj->as.func.fn == px_vm_entry);
        for (int i = 0; i < mw_count && all_vm; i++) {
            if (mws[i].type != PX_FUNC || !mws[i].as.obj ||
                !px_vm_entry || mws[i].as.obj->as.func.fn != px_vm_entry) all_vm = 0;
        }
        if (all_vm &&
            px_pxserve_mw_defer(out, req, handler, params, mws, mw_count,
                                head_only, keep_alive, req_id)) {
            px_root_pop();   // M92-S2c precise（req/params 已入挂起表 GC 根）
            return 2;        // 已拆段（middleware 链状态机运行中）：调用方释放 worker
        }
    }
    for (int i = 0; i < mw_count; i++) {
        LXValue r = px_call(mws[i], &req, 1);
        PX_KEEP(r);   // M92-S2c precise：middleware px_call 返回值（route_normalize/route_send 期间使用）
        if (r.type != PX_NULL) {
            // M100：短路响应抽公共函数（同步短路与协程段2 kind=3 共用，文案逐字一致）
            px_route_mw_short_respond(out, req, method, head_only, keep_alive, req_id, r);
            px_root_pop();   // M92-S2c precise
            return 1;
        }
    }
    // handler(req, params)
    LXValue hargs[2];
    hargs[0] = req;
    hargs[1] = params;
    // M98-S2a：async_ok + VM route handler → 挂起登记 + 帧协程 spawn（占协程不占 worker）；
    //   完成回调 px_serve_done 写 resp + px_pool_push 投回 → 续 worker px_route_respond 段2。
    if (async_ok && px_pxserve_defer(out, req, handler, hargs, 2, 0, NULL,
                                           head_only, keep_alive, req_id)) {
        px_root_pop();   // M92-S2c precise（req 已入挂起表 GC 根；params 已随 spawn 拷贝）
        return 2;        // 已拆段：调用方释放 worker，不发送响应
    }
    LXValue r = px_call(handler, hargs, 2);
    PX_KEEP(r);   // M92-S2c precise：handler px_call 返回值（route_normalize/route_send 期间使用）
    px_route_respond(out, req, method, head_only, keep_alive, req_id, r);
    px_root_pop();   // M92-S2c precise
    return 1;
}
