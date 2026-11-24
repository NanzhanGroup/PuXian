// ============================================================
// M442-S2: runtime/pxe.c —— PXE1(ver2) 序列化模块加载器 + 全量校验
// ------------------------------------------------------------
// 格式（小端，与 selfhost/bc_pxe.px 一一对应；权威描述见 /data/autodrive/M442.md §2）：
//   HEADER 96B
//     0 magic "PXE1" · 4 ver u16 · 6 flags u16
//     8 nK · 12 nN · 16 nG · 20 nfuncs · 24 top_idx(0xFFFFFFFF=-1) · 28 nstructs
//     32 ninst · 36 modname_off · 40 slen · 44 total · 48..95 reserved(0)
//   K  nK*24      kind u8 · str_off u32 · i64（str 时=真字节长，M155 口径）· float 位模式
//   N  nN*4       str_off
//   G  nG*4       str_off
//   F  nfuncs*32  name_off · arity · ndefault · nslots · bcoff · nbc · nup · 0
//   FU nup_total*4 闭包捕获名 str_off（函数主序展平，funcs[i] 的段 = base..base+nup）
//   T  nstructs*12  name_off · nfields · fbase
//   TF nf_total*4   字段名 str_off（struct 主序展平）
//   C  ninst*8    op u8 · fl u8 · a u16 · b u16 · c u16
//   S  slen       字符串池（逐个 NUL 结尾，线性去重）
//   TRAILER 40B   crc32(payload) u32 · sha256(payload) 32B · reserved 4B
//                 payload = [0, total-40) —— 两个校验和覆盖同一区间，互不包含（无循环定义）
// 段偏移全部由 header 计数推导（无段表）⇒ 少一层不一致来源。
//
// 校验 V1–V6（执行顺序 V1 → V6 → V2 → V3 → V4 → V5）：
//   V1 magic        V2 版本        V3 头/段/尺寸自洽
//   V4 指令合法（op 已知 · 跳转目标在 [0,nbc) · K/N/G/funcs 下标越界即拒）
//   V5 字符串（偏移 < slen 且 s 段内确有 NUL）
//   V6 校验和（crc32 快筛 + sha256 权威）
// ★ 纪律：**不做尽力修复**。校验失败一律拒收并指名失败项 ——
//   「修复」等于接受损坏数据；一个手写的 .pxe 若能把 pc 跳到任意地址就是任意读/执行。
// ============================================================
#include "pxe.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/sha256.h"

// 上限（拒绝病态输入，避免加载期 OOM/整数溢出）
#define PXE_MAX_NK       (1 << 20)
#define PXE_MAX_NN       (1 << 20)
#define PXE_MAX_NG       (1 << 20)
#define PXE_MAX_NFUNCS   (1 << 20)
#define PXE_MAX_NSTRUCTS (1 << 16)
#define PXE_MAX_NINST    (1 << 22)
#define PXE_MAX_NUP      1024
#define PXE_MAX_NFIELDS  4096
#define PXE_MAX_SLEN     (64u << 20)

#define PXE_TRAILER     40   // crc32 u32 · sha256 32B · reserved 4B（在文件末尾）

#define PXE_SEG_K   24
#define PXE_SEG_PTR 4
#define PXE_SEG_F   32
#define PXE_SEG_T   12
#define PXE_SEG_C   8

// ---------------- 小端读取（不依赖宿主字节序）----------------
static uint16_t rd16(const unsigned char* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd32(const unsigned char* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static int64_t rd64(const unsigned char* p) {
    uint64_t v = 0;
    for (int i = 7; i >= 0; i--) v = (v << 8) | p[i];
    return (int64_t)v;
}

static void err_set(PxeErr* e, int rc, const char* fmt, ...) {
    if (!e) return;
    e->rc = rc;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e->msg, sizeof(e->msg), fmt, ap);
    va_end(ap);
}

const char* pxe_rc_name(int rc) {
    switch (rc) {
    case PXE_OK:      return "OK";
    case PXE_EVER:    return "EVER(版本不符)";
    case PXE_ELAYOUT: return "ELAYOUT(布局/尺寸不自洽)";
    case PXE_ECODE:   return "ECODE(指令非法)";
    case PXE_ESTR:    return "ESTR(字符串越界)";
    case PXE_ECHECK:  return "ECHECK(校验和不符)";
    case PXE_EIO:     return "EIO(读文件失败)";
    case PXE_ENOMEM:  return "ENOMEM(内存不足)";
    default:          return "EUNKNOWN";
    }
}

// ---------------- crc32（快筛；权威是 sha256）----------------
static uint32_t crc32_of(const unsigned char* d, size_t n) {
    static uint32_t tab[256];
    static int init = 0;
    if (!init) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            tab[i] = c;
        }
        init = 1;
    }
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) c = tab[(c ^ d[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

// sha256(payload) —— payload = 文件前 n 字节（TRAILER 不在其中）
static void sha256_of(const unsigned char* d, size_t n, unsigned char out[32]) {
    mbedtls_sha256(d, n, out, 0);
}

// ---------------- 私有分配（统一走 arena，便于 pxe_free）----------------
static void* arena_alloc(PxeModule* pm, size_t n) {
    if (n == 0) n = 1;
    if (pm->nblocks >= PXE_MAX_SEGS) return NULL;
    void* p = calloc(1, n);
    if (!p) return NULL;
    pm->blocks[pm->nblocks++] = p;
    return p;
}

void pxe_free(PxeModule* pm) {
    if (!pm) return;
    for (int i = 0; i < pm->nblocks; i++) free(pm->blocks[i]);
    if (pm->buf) free(pm->buf);
    memset(pm, 0, sizeof(*pm));
}

// ---------------- V5 辅助：池内字符串 ----------------
// off 必须在 [0, slen) 且从 off 起到 slen 前有 NUL；返回池内指针（不复制）。
static const char* pool_str(const unsigned char* pool, uint32_t slen, uint32_t off) {
    if (off >= slen) return NULL;
    // 单次 memchr，上限 slen-off（不许跨出池）
    const void* z = memchr(pool + off, 0, (size_t)(slen - off));
    if (!z) return NULL;
    return (const char*)(pool + off);
}

// ---------------- 指令合法性（V4）辅助 ----------------
static int op_is_jump(int op) {
    return op == PXOP_JMP || op == PXOP_JMPT || op == PXOP_JMPF || op == PXOP_TRYENTER;
}
// 使用 N 池的指令及其下标字段（1=a, 2=b, 3=c；NEWENUM/CALLM 视具体字段）
static void op_n_indices(int op, int* use_a, int* use_b, int* use_c) {
    *use_a = *use_b = *use_c = 0;
    switch (op) {
    case PXOP_GETF: case PXOP_SETF: case PXOP_GETF_OPT: case PXOP_CALLM:
    case PXOP_CHKINIT: case PXOP_ENUMIS:
        *use_c = 1; break;
    case PXOP_NEWSTRUCT:
        *use_b = 1; break;
    case PXOP_NEWENUM:
        *use_b = 1; *use_c = 1; break;
    default: break;
    }
}

// ============================================================
// 核心：从内存加载并全量校验
// ============================================================
int pxe_load_mem(const unsigned char* data, size_t len, PxeModule* out, PxeErr* err) {
    if (!out) return PXE_ELAYOUT;
    memset(out, 0, sizeof(*out));
    if (err) { err->rc = PXE_OK; err->msg[0] = 0; }

    // ---- V1 magic ----
    if (len < PXE_HDR || memcmp(data, "PXE1", 4) != 0) {
        err_set(err, PXE_ELAYOUT, "V1 magic 不符（不是 PXE1 文件或长度 %zu < %d）", len, PXE_HDR);
        return PXE_ELAYOUT;
    }

    // ---- V6 校验和（先做快筛：损坏文件不必走后续解析）----
    // TRAILER 40B：crc32(payload) · sha256(payload) · reserved；payload = [0, len-40)
    if (len < (size_t)PXE_HDR + PXE_TRAILER) {
        err_set(err, PXE_ELAYOUT, "V1 长度 %zu 不足 头+TRAILER（%d）", len, PXE_HDR + PXE_TRAILER);
        return PXE_ELAYOUT;
    }
    {
        size_t plen = len - PXE_TRAILER;
        uint32_t want_crc = rd32(data + plen);
        uint32_t have_crc = crc32_of(data, plen);
        unsigned char want_sha[32], have_sha[32];
        memcpy(want_sha, data + plen + 4, 32);
        sha256_of(data, plen, have_sha);
        if (want_crc != have_crc || memcmp(want_sha, have_sha, 32) != 0) {
            err_set(err, PXE_ECHECK, "V6 校验和不符（crc32 %s · sha256 %s · payload=%zu）",
                    want_crc == have_crc ? "OK" : "DIFF",
                    memcmp(want_sha, have_sha, 32) == 0 ? "OK" : "DIFF", plen);
            return PXE_ECHECK;
        }
    }

    // ---- V2 版本 ----
    uint16_t ver = rd16(data + 4);
    if (ver != PXE_VER) {
        err_set(err, PXE_EVER, "V2 版本不符：文件 ver=%u，加载器支持 ver=%u", (unsigned)ver, (unsigned)PXE_VER);
        return PXE_EVER;
    }

    // ---- 头部计数 ----
    uint32_t nK = rd32(data + 8), nN = rd32(data + 12), nG = rd32(data + 16);
    uint32_t nfuncs = rd32(data + 20), top_raw = rd32(data + 24), nstructs = rd32(data + 28);
    uint32_t ninst = rd32(data + 32), modname_off = rd32(data + 36);
    uint32_t slen = rd32(data + 40), total = rd32(data + 44);

    // ---- V3 布局自洽 ----
    if (nK > PXE_MAX_NK || nN > PXE_MAX_NN || nG > PXE_MAX_NG || nfuncs > PXE_MAX_NFUNCS ||
        nstructs > PXE_MAX_NSTRUCTS || ninst > PXE_MAX_NINST || slen > PXE_MAX_SLEN) {
        err_set(err, PXE_ELAYOUT, "V3 计数超上限（nK=%u nN=%u nG=%u nfuncs=%u nstructs=%u ninst=%u slen=%u）",
                nK, nN, nG, nfuncs, nstructs, ninst, slen);
        return PXE_ELAYOUT;
    }
    if (total != len) {
        err_set(err, PXE_ELAYOUT, "V3 头声明 total=%u 与实际长度 %zu 不符", total, len);
        return PXE_ELAYOUT;
    }
    if (nfuncs == 0) {
        err_set(err, PXE_ELAYOUT, "V3 nfuncs=0（模块必须至少含 Top）");
        return PXE_ELAYOUT;
    }

    size_t o_K = PXE_HDR;
    size_t o_N = o_K + (size_t)nK * PXE_SEG_K;
    size_t o_G = o_N + (size_t)nN * PXE_SEG_PTR;
    size_t o_F = o_G + (size_t)nG * PXE_SEG_PTR;
    size_t o_FU = o_F + (size_t)nfuncs * PXE_SEG_F;
    size_t o_T, o_TF, o_C, o_S;

    // F 段先扫一遍取 nup_total（FU 段长度由它决定 ⇒ 段偏移可推导）
    uint64_t nup_total = 0;
    for (uint32_t i = 0; i < nfuncs; i++) {
        const unsigned char* f = data + o_F + (size_t)i * PXE_SEG_F;
        uint32_t nup = rd32(f + 24);
        if (nup > PXE_MAX_NUP) {
            err_set(err, PXE_ELAYOUT, "V3 F[%u].nup=%u 超上限 %d", i, nup, PXE_MAX_NUP);
            return PXE_ELAYOUT;
        }
        nup_total += nup;
    }
    o_T = o_FU + (size_t)nup_total * PXE_SEG_PTR;
    o_TF = o_T + (size_t)nstructs * PXE_SEG_T;

    uint64_t nf_total = 0;
    if (o_TF > len) {
        err_set(err, PXE_ELAYOUT, "V3 T 段越界（o_T=%zu o_TF=%zu len=%zu）", o_T, o_TF, len);
        return PXE_ELAYOUT;
    }
    for (uint32_t i = 0; i < nstructs; i++) {
        const unsigned char* t = data + o_T + (size_t)i * PXE_SEG_T;
        uint32_t nf = rd32(t + 4);
        if (nf > PXE_MAX_NFIELDS) {
            err_set(err, PXE_ELAYOUT, "V3 T[%u].nfields=%u 超上限 %d", i, nf, PXE_MAX_NFIELDS);
            return PXE_ELAYOUT;
        }
        nf_total += nf;
    }
    o_C = o_TF + (size_t)nf_total * PXE_SEG_PTR;
    o_S = o_C + (size_t)ninst * PXE_SEG_C;
    if (o_S + (size_t)slen + PXE_TRAILER != (size_t)total) {
        err_set(err, PXE_ELAYOUT, "V3 段推导总长 %zu ≠ total %u（ninst=%u slen=%u）",
                o_S + (size_t)slen + PXE_TRAILER, total, ninst, slen);
        return PXE_ELAYOUT;
    }
    int top_idx = (top_raw == 0xFFFFFFFFu) ? -1 : (int)top_raw;
    if (top_idx >= (int)nfuncs) {
        err_set(err, PXE_ELAYOUT, "V3 top_idx=%d 越界（nfuncs=%u）", top_idx, nfuncs);
        return PXE_ELAYOUT;
    }

    // F 段结构与 bcoff/nbc 自洽
    for (uint32_t i = 0; i < nfuncs; i++) {
        const unsigned char* f = data + o_F + (size_t)i * PXE_SEG_F;
        uint32_t arity = rd32(f + 4), ndefault = rd32(f + 8), nslots = rd32(f + 12);
        uint32_t bcoff = rd32(f + 16), nbc = rd32(f + 20);
        if (arity + ndefault > nslots) {
            err_set(err, PXE_ELAYOUT, "V3 F[%u] arity+ndefault=%u > nslots=%u", i, arity + ndefault, nslots);
            return PXE_ELAYOUT;
        }
        if (bcoff + nbc > ninst) {
            err_set(err, PXE_ELAYOUT, "V3 F[%u] bcoff+nbc=%u > ninst=%u", i, bcoff + nbc, ninst);
            return PXE_ELAYOUT;
        }
        if (nbc == 0) {
            err_set(err, PXE_ELAYOUT, "V3 F[%u] nbc=0（空函数不可执行）", i);
            return PXE_ELAYOUT;
        }
    }

    // ---- V4 指令合法 ----
    for (uint32_t i = 0; i < nfuncs; i++) {
        const unsigned char* f = data + o_F + (size_t)i * PXE_SEG_F;
        uint32_t bcoff = rd32(f + 16), nbc = rd32(f + 20);
        for (uint32_t j = 0; j < nbc; j++) {
            const unsigned char* ip = data + o_C + ((size_t)bcoff + j) * PXE_SEG_C;
            int op = ip[0];
            uint32_t a = rd16(ip + 2), b = rd16(ip + 4), c = rd16(ip + 6);
            if (op <= 0 || op >= PXM_MAX) {
                err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] op=%d 非法（上限 %d）", i, j, op, PXM_MAX - 1);
                return PXE_ECODE;
            }
            if (op_is_jump(op)) {
                // 跳转语义：目标 = (pc+1) + off16（off 存 b 字段）
                int target = (int)j + 1 + (int)(int16_t)b;
                if (target < 0 || target >= (int)nbc) {
                    err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] op=%d 跳转目标 %d 越界 [0,%u)",
                            i, j, op, target, nbc);
                    return PXE_ECODE;
                }
            }
            if (op == PXOP_LOADK && b >= nK) {
                err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] LOADK k=%u 越界（nK=%u）", i, j, b, nK);
                return PXE_ECODE;
            }
            if (op == PXOP_GETG && b >= nG) {
                err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] GETG g=%u 越界（nG=%u）", i, j, b, nG);
                return PXE_ECODE;
            }
            if (op == PXOP_SETG && a >= nG) {
                err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] SETG g=%u 越界（nG=%u）", i, j, a, nG);
                return PXE_ECODE;
            }
            if (op == PXOP_MKCLO && b >= nfuncs) {
                err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] MKCLO funcs=%u 越界（nfuncs=%u）", i, j, b, nfuncs);
                return PXE_ECODE;
            }
            int ua, ub, uc;
            op_n_indices(op, &ua, &ub, &uc);
            if ((ua && a >= nN) || (ub && b >= nN) || (uc && c >= nN)) {
                err_set(err, PXE_ECODE, "V4 F[%u].bc[%u] op=%d 名字下标越界（a=%u b=%u c=%u nN=%u）",
                        i, j, op, a, b, c, nN);
                return PXE_ECODE;
            }
        }
    }

    // ---- V5 字符串 ----
    const unsigned char* pool = data + o_S;
    const char* modname = pool_str(pool, slen, modname_off);
    if (!modname) {
        err_set(err, PXE_ESTR, "V5 modname_off=%u 越界或未 NUL 结尾（slen=%u）", modname_off, slen);
        return PXE_ESTR;
    }
    for (uint32_t i = 0; i < nK; i++) {
        const unsigned char* kp = data + o_K + (size_t)i * PXE_SEG_K;
        uint32_t kind = kp[0], off = rd32(kp + 4);
        if (kind == PXK_STR) {
            if (!pool_str(pool, slen, off)) {
                err_set(err, PXE_ESTR, "V5 K[%u] str_off=%u 越界或未 NUL 结尾", i, off);
                return PXE_ESTR;
            }
        }
    }
    for (uint32_t i = 0; i < nN; i++) {
        uint32_t off = rd32(data + o_N + (size_t)i * PXE_SEG_PTR);
        if (!pool_str(pool, slen, off)) {
            err_set(err, PXE_ESTR, "V5 N[%u] str_off=%u 越界或未 NUL 结尾", i, off);
            return PXE_ESTR;
        }
    }
    for (uint32_t i = 0; i < nG; i++) {
        uint32_t off = rd32(data + o_G + (size_t)i * PXE_SEG_PTR);
        if (!pool_str(pool, slen, off)) {
            err_set(err, PXE_ESTR, "V5 G[%u] str_off=%u 越界或未 NUL 结尾", i, off);
            return PXE_ESTR;
        }
    }
    for (uint32_t i = 0; i < nfuncs; i++) {
        const unsigned char* f = data + o_F + (size_t)i * PXE_SEG_F;
        uint32_t noff = rd32(f + 0);
        if (!pool_str(pool, slen, noff)) {
            err_set(err, PXE_ESTR, "V5 F[%u].name_off=%u 越界或未 NUL 结尾", i, noff);
            return PXE_ESTR;
        }
    }
    {
        size_t base = 0;
        for (uint32_t i = 0; i < nfuncs; i++) {
            const unsigned char* f = data + o_F + (size_t)i * PXE_SEG_F;
            uint32_t nup = rd32(f + 24);
            for (uint32_t u = 0; u < nup; u++) {
                uint32_t uoff = rd32(data + o_FU + (base + u) * PXE_SEG_PTR);
                if (!pool_str(pool, slen, uoff)) {
                    err_set(err, PXE_ESTR, "V5 F[%u].upvals[%u] str_off=%u 越界或未 NUL 结尾", i, u, uoff);
                    return PXE_ESTR;
                }
            }
            base += nup;
        }
    }
    {
        size_t base = 0;
        for (uint32_t i = 0; i < nstructs; i++) {
            const unsigned char* t = data + o_T + (size_t)i * PXE_SEG_T;
            uint32_t noff = rd32(t + 0), nf = rd32(t + 4), fbase = rd32(t + 8);
            if (!pool_str(pool, slen, noff)) {
                err_set(err, PXE_ESTR, "V5 T[%u].name_off=%u 越界或未 NUL 结尾", i, noff);
                return PXE_ESTR;
            }
            if ((uint64_t)fbase + nf > nf_total) {
                err_set(err, PXE_ELAYOUT, "V3 T[%u] fbase+nfields=%llu > nf_total=%llu",
                        i, (unsigned long long)((uint64_t)fbase + nf), (unsigned long long)nf_total);
                return PXE_ELAYOUT;
            }
            for (uint32_t k = 0; k < nf; k++) {
                uint32_t foff = rd32(data + o_TF + (size_t)(fbase + k) * PXE_SEG_PTR);
                if (!pool_str(pool, slen, foff)) {
                    err_set(err, PXE_ESTR, "V5 T[%u].fnames[%u] str_off=%u 越界或未 NUL 结尾", i, k, foff);
                    return PXE_ESTR;
                }
            }
            base += nf;
        }
        (void)base;
    }

    // ================= 构建运行时结构（校验全过才分配）=================
    unsigned char* buf = (unsigned char*)malloc(len);
    if (!buf) {
        err_set(err, PXE_ENOMEM, "文件缓冲分配失败（%zu B）", len);
        return PXE_ENOMEM;
    }
    memcpy(buf, data, len);
    out->buf = buf;
    const unsigned char* d = buf;      // 之后一律用副本（指针须长期有效）
    pool = d + o_S;

    PxK* K = (PxK*)arena_alloc(out, sizeof(PxK) * (nK ? nK : 1));
    const char** N = (const char**)arena_alloc(out, sizeof(char*) * (nN ? nN : 1));
    const char** G = (const char**)arena_alloc(out, sizeof(char*) * (nG ? nG : 1));
    PxVMFunc* funcs = (PxVMFunc*)arena_alloc(out, sizeof(PxVMFunc) * nfuncs);
    const char** upflat = (const char**)arena_alloc(out, sizeof(char*) * (nup_total ? nup_total : 1));
    PxStructDef* structs = (PxStructDef*)arena_alloc(out, sizeof(PxStructDef) * (nstructs ? nstructs : 1));
    const char** fnflat = (const char**)arena_alloc(out, sizeof(char*) * (nf_total ? nf_total : 1));
    if (!K || !N || !G || !funcs || !upflat || !structs || !fnflat) {
        pxe_free(out);
        err_set(err, PXE_ENOMEM, "运行时结构分配失败");
        return PXE_ENOMEM;
    }

    for (uint32_t i = 0; i < nK; i++) {
        const unsigned char* kp = d + o_K + (size_t)i * PXE_SEG_K;
        uint32_t kind = kp[0], off = rd32(kp + 4);
        K[i].kind = (uint8_t)kind;
        K[i].i = rd64(kp + 8);
        K[i].f = 0.0;
        if (kind == PXK_FLT) {
            int64_t bits = rd64(kp + 16);
            memcpy(&K[i].f, &bits, sizeof(double));
        }
        K[i].s = NULL;
        if (kind == PXK_STR && off != 0xFFFFFFFFu) K[i].s = (const char*)(pool + off);
    }
    for (uint32_t i = 0; i < nN; i++) N[i] = (const char*)(pool + rd32(d + o_N + (size_t)i * PXE_SEG_PTR));
    for (uint32_t i = 0; i < nG; i++) G[i] = (const char*)(pool + rd32(d + o_G + (size_t)i * PXE_SEG_PTR));

    out->m.name = (const char*)(pool + modname_off);
    out->m.K = K;        out->m.nK = (int)nK;
    out->m.N = N;        out->m.nN = (int)nN;
    out->m.G = G;        out->m.nG = (int)nG;
    out->m.funcs = funcs; out->m.nfuncs = (int)nfuncs;
    out->m.top_idx = top_idx;
    out->m.structs = structs; out->m.nstructs = (int)nstructs;

    {
        size_t base = 0;
        for (uint32_t i = 0; i < nfuncs; i++) {
            const unsigned char* f = d + o_F + (size_t)i * PXE_SEG_F;
            uint32_t noff = rd32(f + 0), arity = rd32(f + 4), ndefault = rd32(f + 8);
            uint32_t nslots = rd32(f + 12), bcoff = rd32(f + 16), nbc = rd32(f + 20), nup = rd32(f + 24);
            funcs[i].name = (const char*)(pool + noff);
            funcs[i].arity = (int)arity;
            funcs[i].ndefault = (int)ndefault;
            funcs[i].nslots = (int)nslots;
            funcs[i].bc = (const PxInst*)(0);   // 下面统一指到 C 段（按 8B 对齐取指针）
            funcs[i].nbc = (int)nbc;
            funcs[i].mod = &out->m;
            funcs[i].upvals = nup ? (upflat + base) : NULL;
            funcs[i].nup = (int)nup;
            for (uint32_t u = 0; u < nup; u++)
                upflat[base + u] = (const char*)(pool + rd32(d + o_FU + (base + u) * PXE_SEG_PTR));
            base += nup;
        }
    }
    // C 段：不复制，直接按字节序解码到一份独立 PxInst 数组（避免对齐/字节序假设）
    PxInst* inst = (PxInst*)arena_alloc(out, sizeof(PxInst) * (ninst ? ninst : 1));
    if (!inst) {
        pxe_free(out);
        err_set(err, PXE_ENOMEM, "字节码数组分配失败（ninst=%u）", ninst);
        return PXE_ENOMEM;
    }
    for (uint32_t i = 0; i < ninst; i++) {
        const unsigned char* ip = d + o_C + (size_t)i * PXE_SEG_C;
        inst[i].op = (uint8_t)ip[0];
        inst[i].fl = (uint8_t)ip[1];
        inst[i].a = rd16(ip + 2);
        inst[i].b = rd16(ip + 4);
        inst[i].c = rd16(ip + 6);
    }
    for (uint32_t i = 0; i < nfuncs; i++) {
        const unsigned char* f = d + o_F + (size_t)i * PXE_SEG_F;
        funcs[i].bc = inst + rd32(f + 16);
    }

    {
        size_t base = 0;
        for (uint32_t i = 0; i < nstructs; i++) {
            const unsigned char* t = d + o_T + (size_t)i * PXE_SEG_T;
            uint32_t noff = rd32(t + 0), nf = rd32(t + 4), fbase = rd32(t + 8);
            structs[i].name = (const char*)(pool + noff);
            structs[i].nfields = (int)nf;
            structs[i].fnames = nf ? (fnflat + fbase) : NULL;
            for (uint32_t k = 0; k < nf; k++)
                fnflat[fbase + k] = (const char*)(pool + rd32(d + o_TF + (size_t)(fbase + k) * PXE_SEG_PTR));
            base += nf;
        }
        (void)base;
    }
    return PXE_OK;
}

// ---------------- 文件版：读入内存后走同一份校验 ----------------
int pxe_load(const char* path, PxeModule* out, PxeErr* err) {
    if (!path || !out) return PXE_EIO;
    memset(out, 0, sizeof(*out));
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        err_set(err, PXE_EIO, "打开失败 %s", path);
        return PXE_EIO;
    }
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); err_set(err, PXE_EIO, "定位失败 %s", path); return PXE_EIO; }
    long sz = ftell(fp);
    if (sz < 0) { fclose(fp); err_set(err, PXE_EIO, "取长度失败 %s", path); return PXE_EIO; }
    rewind(fp);
    unsigned char* d = (unsigned char*)malloc((size_t)sz ? (size_t)sz : 1);
    if (!d) { fclose(fp); err_set(err, PXE_ENOMEM, "读缓冲分配失败（%ld B）", sz); return PXE_ENOMEM; }
    size_t got = fread(d, 1, (size_t)sz, fp);
    fclose(fp);
    if (got != (size_t)sz) {
        free(d);
        err_set(err, PXE_EIO, "短读 %s（%zu / %ld B）", path, got, sz);
        return PXE_EIO;
    }
    int rc = pxe_load_mem(d, (size_t)sz, out, err);
    free(d);   // pxe_load_mem 内部已复制（out->buf 自有）
    return rc;
}

void pxe_info(const PxeModule* pm) {
    if (!pm) return;
    const PxBCModule* m = &pm->m;
    printf("模块    %s\n", m->name ? m->name : "(null)");
    printf("nK=%d nN=%d nG=%d nfuncs=%d top=%d nstructs=%d\n",
           m->nK, m->nN, m->nG, m->nfuncs, m->top_idx, m->nstructs);
    int ninst = 0;
    for (int i = 0; i < m->nfuncs; i++) {
        printf("  F[%d] %-24s arity=%d ndefault=%d nslots=%d nbc=%d nup=%d\n",
               i, m->funcs[i].name ? m->funcs[i].name : "(null)",
               m->funcs[i].arity, m->funcs[i].ndefault, m->funcs[i].nslots,
               m->funcs[i].nbc, m->funcs[i].nup);
        if (m->funcs[i].nup > 0) {
            printf("        up=[");
            for (int u = 0; u < m->funcs[i].nup; u++)
                printf("%s%s", u ? "," : "", m->funcs[i].upvals[u] ? m->funcs[i].upvals[u] : "(null)");
            printf("]\n");
        }
        ninst += m->funcs[i].nbc;
    }
    for (int i = 0; i < m->nstructs; i++) {
        printf("  S[%d] %s nfields=%d fnames=[", i,
               m->structs[i].name ? m->structs[i].name : "(null)", m->structs[i].nfields);
        for (int k = 0; k < m->structs[i].nfields; k++)
            printf("%s%s", k ? "," : "", m->structs[i].fnames[k] ? m->structs[i].fnames[k] : "(null)");
        printf("]\n");
    }
    printf("指令合计 %d\n", ninst);
}
