#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M262 门的负控助手
# ------------------------------------------------------------
#   revert-route-match    把 `route_match` **忠实退回** M262 之前的实现
#                         （锁内构造 params：px_dict / px_root_push_keep / px_dict_set / px_str）
#                         ⇒ 守卫必须判红并**指名** route_match
#   neuter-guard <out>    生成「判据失明」版守卫（清空 DIRECT/INDIRECT 两张表）
#                         ⇒ 用它跑同一份被植入缺陷的源码**必须不再判红**
#   inject-probe          往 px_route_respond 注入一条**与旧实现等价**的探测式读
#                         （px_dict_get(str,"headers")）⇒ 读入口「诊断档」判据的**自持触发器**
#                         （M420 缺陷 845 修掉运行时的探测读后，旧触发源已不存在）
#   inject-probe-neuter   inject-probe + 把 px_ctr_livechk_on() 钉成 0（摘掉诊断开关）
#                         ⇒ 负控 C：诊断档 CTR 必须回到 0，证明「>0」这条判据真的会红
# ⚠️ 本脚本**只改源码**；还原由 verify.sh 的快照 + trap 负责（逐字节 cmp 自证）。
# ============================================================
import io
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
RT = os.path.join(ROOT, 'runtime', 'runtime_route.c')
GUARD = os.path.join(ROOT, 'selfhost', 'check_lock_alloc.py')
RT_C = os.path.join(ROOT, 'runtime', 'runtime.c')

# M420 补：px_route_respond 里 M420 的 dict 守卫锚点（注入点紧随其后）
PROBE_ANCHOR = '    if (resp.type == PX_DICT) {   // M420（缺陷 845）'
PROBE_LINE = ('    { LXValue _pv = px_str("m262-probe"); (void)px_dict_get(_pv, "headers"); }'
              '  /* M262 门自持探针（旧实现的探测式读） */\n')


def inject_probe(neuter=False):
    src = io.open(RT, encoding='utf-8').read()
    if PROBE_LINE in src:
        print("✗ 探针已在位（勿重复注入）")
        return 1
    n = src.count(PROBE_ANCHOR)
    if n != 1:
        print("✗ 注入锚点失配（%d 处，应为 1）" % n)
        return 1
    io.open(RT, 'w', encoding='utf-8').write(src.replace(PROBE_ANCHOR, PROBE_LINE + PROBE_ANCHOR))
    chk = io.open(RT, encoding='utf-8').read()
    if chk.count(PROBE_LINE) != 1:
        print("✗ 注入后自证失败")
        return 1
    print("✓ 已注入自持探针（px_dict_get(str,\"headers\")）")
    return 0

FN_SIG = 'static int route_match(const char* method, const char* path, LXValue* handler_out,'

OLD_BODY = r"""static int route_match(const char* method, const char* path, LXValue* handler_out,
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
        px_root_push_keep(params);   // M92-S2c precise：route 匹配 params 裸局部跨 px_dict_set/px_str
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
}"""


def _strip_lits(s):
    out, i, n = [], 0, len(s)
    while i < n:
        c = s[i]
        if c in '\'"':
            q = c
            i += 1
            while i < n and s[i] != q:
                if s[i] == '\\':
                    i += 1
                i += 1
            i += 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def cut_function(src, sig):
    i = src.find(sig)
    if i < 0:
        return None, None
    depth = 0
    for j in range(i, len(src)):
        if src[j] == '{':
            depth += 1
        elif src[j] == '}':
            depth -= 1
            if depth == 0:
                return i, j + 1
    return None, None


def neuter_stmt(src, var):
    """把 `var = re.compile(...)` 这**整条语句**（可能跨行）换成永不匹配的模式。
    ⚠️ 自伤教训（M261）：只替换 `\\b(xmalloc` 的第一个分支 ⇒ `|xrealloc|xcalloc|…` 仍在；
       且**数括号不能数字面量里的**（跨行 raw string 的括号让配平永不闭合）。"""
    lines = src.split('\n')
    out, i, hit = [], 0, 0
    while i < len(lines):
        if lines[i].startswith(var + ' = '):
            depth = 0
            j = i
            while j < len(lines):
                st = _strip_lits(lines[j])
                depth += st.count('(') - st.count(')')
                j += 1
                if depth <= 0:
                    break
            out.append("%s = re.compile(r'(?!x)x')   # [NC] 负控：判据失明" % var)
            i = j
            hit += 1
        else:
            out.append(lines[i])
            i += 1
    if hit != 1:
        raise SystemExit("✗ 中性化 %s 命中 %d 次（期望 1）" % (var, hit))
    return '\n'.join(out)


def main():
    if len(sys.argv) < 2:
        print("用法：negctl2.py revert-route-match | neuter-guard <out>")
        return 2
    cmd = sys.argv[1]

    if cmd == 'revert-route-match':
        src = io.open(RT, encoding='utf-8').read()
        a, b = cut_function(src, FN_SIG)
        if a is None:
            print("✗ 找不到 route_match")
            return 1
        cur = src[a:b]
        if 'memcpy(snap, g_routes[i].segs' not in cur:
            print("✗ 当前实现**看起来已是**旧形态（勿重复植入）")
            return 1
        io.open(RT, 'w', encoding='utf-8').write(src[:a] + OLD_BODY + src[b:])
        # 自证：植入后必须**确实**回到锁内构造
        nw = io.open(RT, encoding='utf-8').read()
        if 'memcpy(snap, g_routes[i].segs' in nw or 'px_dict_set(params, seg->seg, px_str(parts[pi]));' not in nw:
            print("✗ 植入后自证失败")
            return 1
        print("✓ 已植入 M262 之前的 route_match（锁内构造 params）")
        return 0

    if cmd == 'neuter-guard':
        if len(sys.argv) < 3:
            print("用法：negctl2.py neuter-guard <out>")
            return 2
        s = io.open(GUARD, encoding='utf-8').read()
        s2 = neuter_stmt(neuter_stmt(s, 'DIRECT_RX'), 'INDIRECT_RX')
        if s2 == s:
            print("✗ 两张表都未替换（锚点失配 ⇒ 自伤负控会假绿）")
            return 1
        io.open(sys.argv[2], 'w', encoding='utf-8').write(s2)
        print("✓ 已生成判据失明版守卫：%s" % sys.argv[2])
        return 0

    if cmd == 'inject-probe':
        return inject_probe(neuter=False)

    if cmd == 'inject-probe-neuter':
        rc = inject_probe(neuter=True)
        if rc != 0:
            return rc
        src = io.open(RT_C, encoding='utf-8').read()
        a = 'static int px_ctr_livechk_on(void) {\n    static int cached = -1;'
        if src.count(a) != 1:
            print("✗ px_ctr_livechk_on 锚点失配（%d 处）" % src.count(a))
            return 1
        io.open(RT_C, 'w', encoding='utf-8').write(
            src.replace(a, 'static int px_ctr_livechk_on(void) {\n    static int cached = 0;  /* 负控 C：诊断开关摘掉 */'))
        chk = io.open(RT_C, encoding='utf-8').read()
        if 'static int cached = -1;' in chk.split('px_ctr_livechk_on')[1][:200]:
            print("✗ 摘开关自证失败")
            return 1
        print("✓ 已把 px_ctr_livechk_on() 钉成 0（负控 C：本判据必须转红）")
        return 0

    print("未知子命令：%s" % cmd)
    return 2


sys.exit(main())
