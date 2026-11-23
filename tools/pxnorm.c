/* tools/pxnorm.c —— 源码「语义指纹」归一化器（M439-1 · 缺陷 921）
 * ============================================================
 * 目的：把「注释」从源码指纹里剔除 ⇒「只改注释」不再触发重烘 / 缓存全冷。
 *   病象（M438 实测）：改 2 行注释 ⇒ rebake --check-all 14/14 → 0/14 ⇒ 重烘
 *   ⇒ .rtcache 冷 ⇒ 全量链从 2h 变 2.5–4h（超时红门 914/918 的成因之一）。
 *
 * 双指纹：
 *   raw = 原样字节 sha256 → 进产物（PXSRC-）与构建日志，留审计痕迹，不当判据
 *   sem = 归一化后 sha256 → 决定「要不要重烘」/ 缓存 key / --check-all 判绿
 *
 * 两档规则（差异是实质性的，不能统一）：
 *   C 档（.c/.h）：**可丢行** —— 整行注释连换行一起删；行尾注释只删注释体；
 *                  块注释整体替换为一个空格（其中的换行一并丢弃）；
 *                  字符串/字符字面量原样（含转义与反斜杠续行）。
 *                  前提：runtime 目录下的 .c/.h 里 __LINE__ 0 处 / __FILE__ 0 处（--premise 自证）
 *                  实证：改注释后 .o 逐字节相同（-O2 无 -g，行号不进产物）
 *   .px 档       ：**必须保行** —— 注释体全删、换行一个不删；块注释 `#| |#` 留 1 个空格防粘连。
 *                  原因：行号进模块（错误定位/栈回溯），丢行 = 改语义。
 *                  实证：同名文件块注释长度 3→40，px build 产物逐字节相同
 *
 * 保守回退：任何解析异常（未闭合注释/字符串、行首块注释后同行有代码）⇒ ok=0，
 *   调用方按 raw 处理，等价于 M438 之前的口径。宁可少省一次重烘，不改语义。
 *
 * 为什么是 C 而不是 awk/.px：
 *   · awk —— gawk 与 mawk 的输出不保证逐字节相同，跨机器指纹会漂（这正是
 *     rebake_bin.sh 顶部记着的 PR #8 假红那一族）。
 *   · .px —— 解释器逐字符扫 2MB 实测 >30s（/tmp/perf.px），做不到。
 *   · C  —— 同一份源码、同一算法，任何机器结论一致；cc 本来就是 px build 的前置。
 *
 * 用法：
 *   pxnorm --version
 *   pxnorm --selftest
 *   pxnorm --fp FILE...          # 每文件一行：路径 raw=<16> sem=<16> state=ok|fallback:…
 *   pxnorm --join FILE...        # 一行 16-hex：按参数序把各文件 sem 摘要串起来再哈希
 *   pxnorm --joinraw FILE...     # 一行 16-hex：同上但用 raw（供 map 键对拍）
 *   pxnorm --emit c|px|auto FILE # 归一化正文写 stdout（调试/取证用）
 *   pxnorm --premise DIR         # 统计 DIR/runtime 下 .c/.h 的 __LINE__/__FILE__（>0 ⇒ rc=3）
 * 退出码：0 正常 / 2 参数或读取失败 / 3 前提不成立（--premise）
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>

#define PXNORM_VER "1.0.0"

/* ---------------- SHA-256 ---------------- */
typedef struct { uint32_t h[8]; uint64_t len; unsigned char b[64]; size_t bl; } shactx;
static const uint32_t K[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
static void sha_init(shactx* c) {
    static const uint32_t iv[8] = {0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
                                   0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
    memcpy(c->h, iv, sizeof iv); c->len = 0; c->bl = 0;
}
static void sha_block(shactx* c, const unsigned char* p) {
    uint32_t w[64], a, b, d, e, f, g, h, t1, t2; int i;
    for (i = 0; i < 16; i++)
        w[i] = (uint32_t)p[i*4] << 24 | (uint32_t)p[i*4+1] << 16 | (uint32_t)p[i*4+2] << 8 | p[i*4+3];
    for (i = 16; i < 64; i++) {
        uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
        uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    a=c->h[0]; b=c->h[1]; d=c->h[3]; e=c->h[4]; f=c->h[5]; g=c->h[6]; h=c->h[7];
    {   uint32_t cc = c->h[2];
        for (i = 0; i < 64; i++) {
            uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            t1 = h + S1 + ch + K[i] + w[i];
            uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
            uint32_t mj = (a & b) ^ (a & cc) ^ (b & cc);
            t2 = S0 + mj;
            h=g; g=f; f=e; e=d+t1; d=cc; cc=b; b=a; a=t1+t2;
        }
        c->h[0]+=a; c->h[1]+=b; c->h[2]+=cc; c->h[3]+=d; c->h[4]+=e; c->h[5]+=f; c->h[6]+=g; c->h[7]+=h;
    }
}
static void sha_update(shactx* c, const void* p, size_t n) {
    const unsigned char* s = (const unsigned char*)p;
    c->len += n;
    while (n) {
        size_t take = 64 - c->bl; if (take > n) take = n;
        memcpy(c->b + c->bl, s, take); c->bl += take; s += take; n -= take;
        if (c->bl == 64) { sha_block(c, c->b); c->bl = 0; }
    }
}
static void sha_final(shactx* c, unsigned char out[32]) {
    uint64_t bits = c->len * 8; unsigned char pad = 0x80, zero = 0, lenb[8]; int i;
    sha_update(c, &pad, 1);
    while (c->bl != 56) sha_update(c, &zero, 1);
    for (i = 0; i < 8; i++) lenb[i] = (unsigned char)(bits >> (56 - 8 * i));
    sha_update(c, lenb, 8);
    for (i = 0; i < 8; i++) {
        out[i*4]   = (unsigned char)(c->h[i] >> 24);
        out[i*4+1] = (unsigned char)(c->h[i] >> 16);
        out[i*4+2] = (unsigned char)(c->h[i] >> 8);
        out[i*4+3] = (unsigned char)(c->h[i]);
    }
}
static void hex_lower(const unsigned char* d, size_t n, char* o) {
    static const char* H = "0123456789abcdef"; size_t i;
    for (i = 0; i < n; i++) { o[i*2] = H[d[i] >> 4]; o[i*2+1] = H[d[i] & 15]; }
    o[n*2] = 0;
}
static void hash16(const void* p, size_t n, char o[17]) {
    unsigned char d[32]; shactx c; char full[65];
    sha_init(&c); sha_update(&c, p, n); sha_final(&c, d); hex_lower(d, 32, full);
    memcpy(o, full, 16); o[16] = 0;
}

/* ---------------- 输出缓冲 ---------------- */
typedef struct { unsigned char* p; size_t n, cap; int oom; } buf;
static void bgrow(buf* b, size_t need) {
    if (b->n + need <= b->cap) return;
    size_t nc = b->cap ? b->cap : 4096;
    while (nc < b->n + need) nc *= 2;
    unsigned char* np = (unsigned char*)realloc(b->p, nc);
    if (!np) { b->oom = 1; return; }
    b->p = np; b->cap = nc;
}
static void bput(buf* b, unsigned char c) { bgrow(b, 1); if (!b->oom) b->p[b->n++] = c; }

/* ---------------- C 档：可丢行 ---------------- */
static int norm_c(const unsigned char* s, size_t n, buf* o) {
    size_t i = 0; long nlpos = -1;
    while (i < n) {
        unsigned char c = s[i];
        if (c == '/' && i + 1 < n && s[i+1] == '*') {          /* 块注释 → 一个空格 */
            size_t j = i + 2; int closed = 0;
            while (j + 1 < n) { if (s[j] == '*' && s[j+1] == '/') { j += 2; closed = 1; break; } j++; }
            if (!closed) return 0;
            bput(o, ' '); i = j;                                /* 换行一并丢弃 ⇒ nlpos 不变 */
        } else if (c == '/' && i + 1 < n && s[i+1] == '/') {    /* 行注释 */
            size_t j = i + 2, k; int onlyws = 1;
            while (j < n) {
                if (s[j] == '\\' && j + 1 < n && s[j+1] == '\n') { j += 2; continue; }
                if (s[j] == '\n') break;
                j++;
            }
            for (k = (size_t)(nlpos + 1); k < o->n; k++)
                if (o->p[k] != ' ' && o->p[k] != '\t' && o->p[k] != '\r') { onlyws = 0; break; }
            if (onlyws) {                                       /* 整行删除：连换行一起丢 */
                o->n = (size_t)(nlpos + 1);
                if (j < n) j++;
            }
            i = j;
        } else if (c == '"' || c == '\'') {                     /* 字面量原样 */
            unsigned char q = c; int closed = 0;
            bput(o, c); i++;
            while (i < n) {
                if (s[i] == '\\' && i + 1 < n) {
                    bput(o, s[i]); bput(o, s[i+1]);
                    if (s[i+1] == '\n') nlpos = (long)o->n - 1;
                    i += 2; continue;
                }
                bput(o, s[i]);
                if (s[i] == '\n') return 0;                     /* 字符串跨行未闭合 */
                if (s[i] == q) { i++; closed = 1; break; }
                i++;
            }
            if (!closed) return 0;
        } else {
            bput(o, c); if (c == '\n') nlpos = (long)o->n - 1; i++;
        }
        if (o->oom) return 0;
    }
    return 1;
}

/* ---------------- .px 档：必须保行 ---------------- */
static int norm_px(const unsigned char* s, size_t n, buf* o) {
    size_t i = 0; int hascode = 0;
    while (i < n) {
        unsigned char c = s[i];
        if (c == '\n') { bput(o, c); hascode = 0; i++; }
        else if (c == ' ' || c == '\t' || c == '\r') { bput(o, c); i++; }
        else if (c == '"' || c == '\'') {
            unsigned char q = c;
            if (i + 2 < n && s[i+1] == q && s[i+2] == q) {       /* 三引号：整段原样 */
                size_t e = i + 3; int closed = 0;
                while (e < n) {
                    if (s[e] == '\\' && e + 1 < n) { e += 2; continue; }
                    if (e + 2 < n && s[e] == q && s[e+1] == q && s[e+2] == q) { e += 3; closed = 1; break; }
                    e++;
                }
                if (!closed) return 0;
                for (; i < e; i++) { bput(o, s[i]); if (s[i] == '\n') hascode = 0; }
                hascode = 1; continue;
            }
            bput(o, c); i++;
            {   int closed = 0;
                while (i < n) {
                    if (s[i] == '\\' && i + 1 < n) { bput(o, s[i]); bput(o, s[i+1]); i += 2; continue; }
                    if (s[i] == '$' && i + 1 < n && s[i+1] == '{') {  /* 插值整段原样（保守） */
                        int depth = 1;
                        bput(o, s[i]); bput(o, s[i+1]); i += 2;
                        while (i < n && depth > 0) {
                            if (s[i] == '{') depth++;
                            else if (s[i] == '}') depth--;
                            bput(o, s[i]); i++;
                        }
                        if (depth > 0) return 0;
                        continue;
                    }
                    if (s[i] == q) { bput(o, q); i++; closed = 1; break; }
                    if (s[i] == '\n') return 0;
                    bput(o, s[i]); i++;
                }
                if (!closed) return 0;
            }
            hascode = 1;
        } else if (c == '#') {
            if (i + 1 < n && s[i+1] == '|') {                    /* 块注释：内容全删、换行全留 */
                int starthas = hascode, depth = 1; size_t nl = 0, k;
                i += 2;
                while (i < n && depth > 0) {
                    if (i + 1 < n && s[i] == '#' && s[i+1] == '|') { depth++; i += 2; continue; }
                    if (i + 1 < n && s[i] == '|' && s[i+1] == '#') { depth--; i += 2; continue; }
                    if (s[i] == '\n') nl++;
                    i++;
                }
                if (depth > 0) return 0;
                if (!starthas) {                                 /* 行首块注释 + 同行后有代码 ⇒ 回退 */
                    for (k = i; k < n && s[k] != '\n'; k++)
                        if (s[k] != ' ' && s[k] != '\t' && s[k] != '\r') return 0;
                }
                bput(o, ' ');
                for (k = 0; k < nl; k++) bput(o, '\n');
                hascode = nl > 0 ? 0 : starthas;
            } else {                                             /* 行注释：删到行尾（换行留给外层） */
                while (i < n && s[i] != '\n') i++;
            }
        } else { bput(o, c); hascode = 1; i++; }
        if (o->oom) return 0;
    }
    return 1;
}

/* ---------------- 通用封装 ---------------- */
static int mode_of(const char* path) {
    size_t n = strlen(path);
    return (n >= 3 && strcmp(path + n - 3, ".px") == 0) ? 1 : 0;
}
static int norm_auto(const unsigned char* s, size_t n, int mode, buf* o) {
    return mode ? norm_px(s, n, o) : norm_c(s, n, o);
}
static unsigned char* slurp(const char* path, size_t* nout) {
    FILE* f = fopen(path, "rb");
    unsigned char* p = NULL; size_t n = 0, cap = 0;
    if (!f) return NULL;
    for (;;) {
        if (n == cap) { cap = cap ? cap * 2 : 65536; p = (unsigned char*)realloc(p, cap); if (!p) { fclose(f); return NULL; } }
        size_t r = fread(p + n, 1, cap - n, f);
        n += r;
        if (r == 0) break;
    }
    if (ferror(f)) { free(p); fclose(f); return NULL; }
    fclose(f); *nout = n; return p;
}

/* ---------------- 子命令 ---------------- */
static int cmd_fp(int argc, char** argv, int join, int rawjoin) {
    int i, bad = 0, fb = 0;
    if (join || rawjoin) {
        shactx c; unsigned char d[32]; char hx[17];
        sha_init(&c);
        for (i = 0; i < argc; i++) {
            size_t n = 0; unsigned char* s = slurp(argv[i], &n);
            if (!s) { fprintf(stderr, "读取失败 %s\n", argv[i]); bad++; continue; }
            if (rawjoin) hash16(s, n, hx);
            else {
                buf o; memset(&o, 0, sizeof o);
                int ok = norm_auto(s, n, mode_of(argv[i]), &o);
                hash16(ok ? o.p : s, ok ? o.n : n, hx);
                free(o.p);
            }
            sha_update(&c, hx, 16);
            free(s);
        }
        sha_final(&c, d);
        char full[65]; hex_lower(d, 32, full);
        printf("%.16s\n", full);
        return bad ? 2 : 0;
    }
    for (i = 0; i < argc; i++) {
        size_t n = 0; unsigned char* s = slurp(argv[i], &n);
        char rh[17], sh[17];
        if (!s) { fprintf(stderr, "读取失败 %s\n", argv[i]); bad++; continue; }
        buf o; memset(&o, 0, sizeof o);
        int ok = norm_auto(s, n, mode_of(argv[i]), &o);
        hash16(s, n, rh);
        hash16(ok ? o.p : s, ok ? o.n : n, sh);
        if (!ok) fb++;
        printf("%s raw=%s sem=%s state=%s\n", argv[i], rh, sh, ok ? "ok" : "fallback");
        free(o.p); free(s);
    }
    if (fb) fprintf(stderr, "回退 %d 个文件（按 raw 处理）\n", fb);
    return bad ? 2 : 0;
}
static int cmd_emit(int argc, char** argv, const char* mode) {
    if (argc != 1) { fprintf(stderr, "用法: pxnorm --emit c|px|auto FILE\n"); return 2; }
    size_t n = 0; unsigned char* s = slurp(argv[0], &n);
    if (!s) { fprintf(stderr, "读取失败 %s\n", argv[0]); return 2; }
    int m = strcmp(mode, "px") == 0 ? 1 : (strcmp(mode, "c") == 0 ? 0 : mode_of(argv[0]));
    buf o; memset(&o, 0, sizeof o);
    int ok = norm_auto(s, n, m, &o);
    if (!ok) { fwrite(s, 1, n, stdout); free(o.p); free(s); return 0; }
    fwrite(o.p, 1, o.n, stdout);
    free(o.p); free(s);
    return 0;
}
static int has_ext(const char* nm, const char* ext) {
    size_t a = strlen(nm), b = strlen(ext);
    return a > b && strcmp(nm + a - b, ext) == 0;
}
static int cmd_premise(const char* root) {
    char dir[4096]; DIR* d; struct dirent* e; long line = 0, file = 0;
    snprintf(dir, sizeof dir - 16, "%s/runtime", root);
    d = opendir(dir);
    if (!d) { fprintf(stderr, "打不开 %s\n", dir); return 2; }
    while ((e = readdir(d))) {
        char* p = NULL; size_t n = 0;
        if (!has_ext(e->d_name, ".c") && !has_ext(e->d_name, ".h")) continue;
        p = (char*)malloc(strlen(dir) + strlen(e->d_name) + 2);
        if (!p) break;
        sprintf(p, "%s/%s", dir, e->d_name);
        unsigned char* s = slurp(p, &n);
        if (!s) { free(p); continue; }
        size_t i; int lf = 0;
        for (i = 0; i + 8 <= n; i++) {
            if (memcmp(s + i, "__LINE__", 8) == 0) { line++; lf = 1; }
            if (i + 8 <= n && memcmp(s + i, "__FILE__", 8) == 0) { file++; lf = 1; }
        }
        if (lf) printf("  %s\n", p);
        free(s); free(p);
    }
    closedir(d);
    printf("__LINE__=%ld __FILE__=%ld\n", line, file);
    return (line == 0 && file == 0) ? 0 : 3;
}

/* ---------------- 自测 ---------------- */
static int st_pass = 0, st_fail = 0;
static void st_ok(int c, const char* msg) { if (c) { st_pass++; } else { st_fail++; printf("  FAIL %s\n", msg); } }
static void sem(int mode, const char* s, char o[17]) {
    buf b; memset(&b, 0, sizeof b);
    int ok = norm_auto((const unsigned char*)s, strlen(s), mode, &b);
    hash16(ok ? b.p : (const unsigned char*)s, ok ? b.n : strlen(s), o);
    free(b.p);
}
static int same(int mode, const char* a, const char* b) { char x[17], y[17]; sem(mode, a, x); sem(mode, b, y); return strcmp(x, y) == 0; }
static int cmd_selftest(void) {
    printf("── pxnorm --selftest\n");
    /* C 档 */
    st_ok(same(0, "int a; // x\n", "int a; // yyyy\n"), "C1 行尾注释文字变化 ⇒ sem 不变");
    st_ok(same(0, "int a;\n// c\nint b;\n", "int a;\nint b;\n"), "C2 整行注释（连行）⇒ sem 不变");
    st_ok(same(0, "int a;\nint b;\n", "int a;\n// 新增一行注释\nint b;\n"), "C3 加一行注释 ⇒ sem 不变（可丢行）");
    st_ok(!same(0, "int a;\nint b;\n", "int a;\nint b2;\n"), "C4 负控：改代码 ⇒ sem 必变");
    st_ok(same(0, "char* s = \"/*x*/\";\n", "char* s = \"/*x*/\";\n// c\n"), "C5 字符串里的 /* 不被吞");
    st_ok(!same(0, "char* s = \"/*\";\n", "char* s = \"/**/\";\n"), "C6 负控：字符串内容变化 ⇒ sem 必变");
    st_ok(same(0, "int a; /* b */\n", "int a; /* ccc */\n"), "C7 同行块注释文字变化 ⇒ sem 不变");
    st_ok(same(0, "// a \\\n b\nint x;\n", "// z \\\n qqq\nint x;\n"), "C8 反斜杠续行的 // 注释整段吞掉");
    st_ok(same(0, "int a;\r\n", "int a;\r\n"), "C9 幂等");
    {   /* 未闭合块注释 ⇒ 回退（state=fallback，sem==raw） */
        buf b; memset(&b, 0, sizeof b);
        int ok = norm_c((const unsigned char*)"int a; /* x", 11, &b);
        st_ok(!ok, "C10 未闭合块注释 ⇒ 回退"); free(b.p);
    }
    /* .px 档 */
    st_ok(same(1, "# c\ndef f():\n    pass\n", "# zzzz\ndef f():\n    pass\n"), "P1 行注释文字变化 ⇒ sem 不变");
    st_ok(!same(1, "# c\ndef f():\n", "\n# c\ndef f():\n"), "P2 负控：加一行 ⇒ sem 必变（保行）");
    st_ok(same(1, "#| a |#\ndef f():\n", "#| bbbb |#\ndef f():\n"), "P3 块注释文字变化 ⇒ sem 不变");
    st_ok(same(1, "#| a #| nest |# c |#\nx\n", "#| z #| qqqq |# r |#\nx\n"), "P4 嵌套块注释");
    st_ok(same(1, "let s = \"#|\"\n# c\n", "let s = \"#|\"\n# zzzz\n"), "P5 字符串里的 #| 不被当注释");
    st_ok(!same(1, "let s = \"#|\"\n", "let s = \"#|x\"\n"), "P6 负控：字符串内容变化 ⇒ sem 必变");
    st_ok(same(1, "let s = \"\"\"a\n# b\n\"\"\"\n", "let s = \"\"\"a\n# b\n\"\"\"\n"), "P7 三引号原样");
    {   buf b; memset(&b, 0, sizeof b);
        int ok = norm_px((const unsigned char*)"#| x\n", 5, &b);
        st_ok(!ok, "P8 未闭合块注释 ⇒ 回退"); free(b.p);
    }
    st_ok(same(1, "# c\nx = 1\n", "# c\nx = 1\n"), "P9 幂等");
    printf("  pxnorm selftest: %d 通过 / %d 失败\n", st_pass, st_fail);
    return st_fail ? 1 : 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "用法: pxnorm --fp|--join|--joinraw|--emit|--premise|--selftest|--version ...\n");
        return 2;
    }
    if (strcmp(argv[1], "--version") == 0) { printf("pxnorm %s\n", PXNORM_VER); return 0; }
    if (strcmp(argv[1], "--selftest") == 0) return cmd_selftest();
    if (strcmp(argv[1], "--fp") == 0) return cmd_fp(argc - 2, argv + 2, 0, 0);
    if (strcmp(argv[1], "--join") == 0) return cmd_fp(argc - 2, argv + 2, 1, 0);
    if (strcmp(argv[1], "--joinraw") == 0) return cmd_fp(argc - 2, argv + 2, 1, 1);
    if (strcmp(argv[1], "--emit") == 0) {
        const char* m = argc > 3 ? argv[2] : "auto";
        return cmd_emit(argc - 3, argv + 3, m);
    }
    if (strcmp(argv[1], "--premise") == 0) return cmd_premise(argc > 2 ? argv[2] : ".");
    fprintf(stderr, "未知子命令 %s\n", argv[1]);
    return 2;
}
