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

// 执行中间件链 + handler 并发送响应。返回 1 = 已处理（匹配到路由）；0 = 未匹配。
int px_route_try_dispatch(PxHttpOut* out, LXValue req, const char* method, int head_only,
                          int keep_alive, const char* req_id) {
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
    for (int i = 0; i < mw_count; i++) {
        LXValue r = px_call(mws[i], &req, 1);
        if (r.type != PX_NULL) {
            RouteResp rr;
            route_normalize(r, &rr);
            fprintf(stderr, "[px-serve] [route] %s %s -> %d (middleware)\n", method,
                    path_v.as.obj->as.str.data, rr.status);
            char rsp_extra[512];
            snprintf(rsp_extra, sizeof(rsp_extra), "X-Request-Id: %s\r\n", req_id);
            route_send(out, rr.status, rr.ct, rr.body, rr.body_len, head_only, keep_alive, rsp_extra);
            // M36：route 响应统一访问日志（与解释器 log_access 一致）
            {
                LXValue rmt = px_dict_get(req, "remote");
                const char* lr = (rmt.type == PX_STR) ? rmt.as.obj->as.str.data : "-";
                px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                        (long long)time(NULL), lr, method,
                        path_v.as.obj->as.str.data, rr.status, rr.body_len, req_id);
            }
            return 1;
        }
    }
    // handler(req, params)
    LXValue hargs[2];
    hargs[0] = req;
    hargs[1] = params;
    LXValue r = px_call(handler, hargs, 2);
    RouteResp rr;
    route_normalize(r, &rr);
    fprintf(stderr, "[px-serve] [route] %s %s -> %d\n", method,
            path_v.as.obj->as.str.data, rr.status);
    char rsp_extra[512];
    snprintf(rsp_extra, sizeof(rsp_extra), "X-Request-Id: %s\r\n", req_id);
    route_send(out, rr.status, rr.ct, rr.body, rr.body_len, head_only, keep_alive, rsp_extra);
    // M36：route 响应统一访问日志（与解释器 log_access 一致）
    {
        LXValue rmt = px_dict_get(req, "remote");
        const char* lr = (rmt.type == PX_STR) ? rmt.as.obj->as.str.data : "-";
        px_access_log("[px-access] %lld %s %s %s %d %d 0ms req=%s\n",
                (long long)time(NULL), lr, method,
                path_v.as.obj->as.str.data, rr.status, rr.body_len, req_id);
    }
    return 1;
}
