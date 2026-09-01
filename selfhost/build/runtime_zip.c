// 普贤 (PuXian) M19 zip 内置函数（编译模式，miniz raw deflate + 自带 zip 容器，与解释器结构一致）
// - zip_pack(files, out_path) → bool（files: dict{路径→内容}，deflate 压缩，UTF-8 文件名）
// - zip_unpack(zip_path, out_dir) → int（解压文件数，支持 deflate + store，防路径穿越）
#define _GNU_SOURCE
#include "runtime.h"
#include "miniz.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

// ---- 动态字节缓冲 ----
typedef struct { unsigned char* data; int len, cap; } ZBuf;
static void zbuf_push(ZBuf* b, const unsigned char* d, int n) {
    if (b->len + n > b->cap) {
        int nc = (b->cap ? b->cap : 256) * 2 + n;
        b->data = (unsigned char*)realloc(b->data, nc);
        b->cap = nc;
    }
    memcpy(b->data + b->len, d, n);
    b->len += n;
}
static void zbuf_u16(ZBuf* b, unsigned v) {
    unsigned char t[2] = { (unsigned char)(v & 0xFF), (unsigned char)((v >> 8) & 0xFF) };
    zbuf_push(b, t, 2);
}
static void zbuf_u32(ZBuf* b, unsigned v) {
    unsigned char t[4] = { (unsigned char)(v & 0xFF), (unsigned char)((v >> 8) & 0xFF),
                           (unsigned char)((v >> 16) & 0xFF), (unsigned char)((v >> 24) & 0xFF) };
    zbuf_push(b, t, 4);
}

static unsigned rd_u16(const unsigned char* d, int o) { return d[o] | (d[o + 1] << 8); }
static unsigned rd_u32(const unsigned char* d, int o) {
    return (unsigned)d[o] | ((unsigned)d[o + 1] << 8) | ((unsigned)d[o + 2] << 16) | ((unsigned)d[o + 3] << 24);
}

// 取字符串字节与长度
static const char* zstr(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.data;
}
static int zstrlen(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.len;
}

// raw deflate 压缩（level 6）；失败返回 -1，成功返回压缩长度（out 容量 bound）
static int z_raw_deflate(const unsigned char* in, int inlen, unsigned char* out, int outcap) {
    mz_stream s;
    memset(&s, 0, sizeof(s));
    if (mz_deflateInit2(&s, 6, MZ_DEFLATED, -15, 8, MZ_DEFAULT_STRATEGY) != MZ_OK) return -1;
    s.next_in = in; s.avail_in = (mz_ulong)inlen;
    s.next_out = out; s.avail_out = (mz_ulong)outcap;
    int r = mz_deflate(&s, MZ_FINISH);
    mz_deflateEnd(&s);
    if (r != MZ_STREAM_END) return -1;
    return (int)s.total_out;
}

// raw deflate 解压（期望 uncomp_size 字节）；失败返回 -1
static int z_raw_inflate(const unsigned char* in, int inlen, unsigned char* out, int outcap, int* outlen) {
    mz_stream s;
    memset(&s, 0, sizeof(s));
    if (mz_inflateInit2(&s, -15) != MZ_OK) return -1;
    s.next_in = in; s.avail_in = (mz_ulong)inlen;
    s.next_out = out; s.avail_out = (mz_ulong)outcap;
    int r = mz_inflate(&s, MZ_FINISH);
    mz_inflateEnd(&s);
    if (r != MZ_STREAM_END) return -1;
    *outlen = (int)s.total_out;
    return 0;
}

// zip_pack(files, out_path) → bool
LXValue bi_zip_pack(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("zip_pack 需要 2 个参数: (files, out_path)");
    if (args[0].type != PX_DICT) px_error("zip_pack 第一个参数须为 dict{路径→内容}");
    const char* out_path = zstr(args[1]);
    LXObject* d = args[0].as.obj;
    ZBuf out = {0};
    ZBuf cd = {0};
    for (int i = 0; i < d->as.dict.len; i++) {
        const char* name = d->as.dict.keys[i];
        LXValue v = d->as.dict.vals[i];
        const char* content = zstr(v);
        int clen = zstrlen(v);
        if (name[0] == '\0' || name[0] == '/' || strstr(name, "..") != NULL)
            px_error("zip 条目路径非法: '%s'", name);
        unsigned crc = mz_crc32(0, (const mz_uint8*)content, (size_t)clen);
        // deflate level 6；若压缩反而变大则退回 store
        int bound = (int)mz_compressBound((mz_ulong)clen);
        unsigned char* comp = (unsigned char*)malloc(bound);
        int comp_len = z_raw_deflate((const unsigned char*)content, clen, comp, bound);
        int method;
        const unsigned char* data;
        int dlen;
        if (comp_len >= 0 && comp_len < clen) { method = 8; data = comp; dlen = comp_len; }
        else { method = 0; data = (const unsigned char*)content; dlen = clen; }
        int name_len = (int)strlen(name);
        int local_offset = out.len;
        // Local File Header
        zbuf_push(&out, (const unsigned char*)"PK\003\004", 4);
        zbuf_u16(&out, 20);        // version needed
        zbuf_u16(&out, 0x0800);    // flags: UTF-8
        zbuf_u16(&out, method);
        zbuf_u16(&out, 0);         // mod time（固定，确定性）
        zbuf_u16(&out, 0);         // mod date
        zbuf_u32(&out, crc);
        zbuf_u32(&out, (unsigned)dlen);
        zbuf_u32(&out, (unsigned)clen);
        zbuf_u16(&out, (unsigned)name_len);
        zbuf_u16(&out, 0);         // extra
        zbuf_push(&out, (const unsigned char*)name, name_len);
        zbuf_push(&out, data, dlen);
        // Central Directory 条目
        zbuf_push(&cd, (const unsigned char*)"PK\001\002", 4);
        zbuf_u16(&cd, 20);         // version made by
        zbuf_u16(&cd, 20);         // version needed
        zbuf_u16(&cd, 0x0800);
        zbuf_u16(&cd, method);
        zbuf_u16(&cd, 0);
        zbuf_u16(&cd, 0);
        zbuf_u32(&cd, crc);
        zbuf_u32(&cd, (unsigned)dlen);
        zbuf_u32(&cd, (unsigned)clen);
        zbuf_u16(&cd, (unsigned)name_len);
        zbuf_u16(&cd, 0);          // extra
        zbuf_u16(&cd, 0);          // comment
        zbuf_u16(&cd, 0);          // disk
        zbuf_u16(&cd, 0);          // internal attr
        zbuf_u32(&cd, 0);          // external attr
        zbuf_u32(&cd, (unsigned)local_offset);
        zbuf_push(&cd, (const unsigned char*)name, name_len);
        free(comp);
    }
    int cd_offset = out.len;
    zbuf_push(&out, cd.data, cd.len);
    // End of Central Directory
    zbuf_push(&out, (const unsigned char*)"PK\005\006", 4);
    zbuf_u16(&out, 0);             // disk
    zbuf_u16(&out, 0);             // cd disk
    zbuf_u16(&out, (unsigned)d->as.dict.len);
    zbuf_u16(&out, (unsigned)d->as.dict.len);
    zbuf_u32(&out, (unsigned)cd.len);
    zbuf_u32(&out, (unsigned)cd_offset);
    zbuf_u16(&out, 0);             // comment len
    FILE* f = fopen(out_path, "wb");
    if (!f) px_error("zip 写入失败 %s", out_path);
    int ok = fwrite(out.data, 1, out.len, f) == (size_t)out.len;
    fclose(f);
    if (cd.data) free(cd.data);
    if (out.data) free(out.data);
    if (!ok) px_error("zip 写入失败 %s", out_path);
    return px_bool(true);
}

// 创建父目录（递归）
static void z_mkdirs(const char* path) {
    char* tmp = strdup(path);
    int len = (int)strlen(tmp);
    for (int i = 0; i < len; i++) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            if (tmp[0]) mkdir(tmp, 0755);
            tmp[i] = '/';
        }
    }
    free(tmp);
}

// zip_unpack(zip_path, out_dir) → int（解压文件数）
LXValue bi_zip_unpack(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("zip_unpack 需要 2 个参数: (zip_path, out_dir)");
    const char* zip_path = zstr(args[0]);
    const char* out_dir = zstr(args[1]);
    // 读整个 zip
    FILE* f = fopen(zip_path, "rb");
    if (!f) px_error("zip 读取失败 %s", zip_path);
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = (unsigned char*)malloc(fsize > 0 ? fsize : 1);
    if (fsize > 0 && fread(data, 1, fsize, f) != (size_t)fsize) { fclose(f); free(data); px_error("zip 读取失败 %s", zip_path); }
    fclose(f);
    // 找 EOCD
    int eocd = -1;
    int start = fsize - 65557; if (start < 0) start = 0;
    for (int i = fsize - 4; i >= start; i--) {
        if (data[i] == 'P' && data[i + 1] == 'K' && data[i + 2] == 5 && data[i + 3] == 6) { eocd = i; break; }
    }
    if (eocd < 0) { free(data); px_error("zip 缺少 EOCD"); }
    int entries = (int)rd_u16(data, eocd + 10);
    int cd_size = (int)rd_u32(data, eocd + 12);
    int cd_off = (int)rd_u32(data, eocd + 16);
    if (cd_off + cd_size > fsize) { free(data); px_error("zip 中央目录越界"); }
    int count = 0;
    int pos = cd_off;
    for (int i = 0; i < entries; i++) {
        if (pos + 46 > fsize || data[pos] != 'P' || data[pos + 1] != 'K') { free(data); px_error("zip 中央目录条目损坏"); }
        int entry_start = pos;
        int method = (int)rd_u16(data, pos + 10);
        int comp_size = (int)rd_u32(data, pos + 20);
        int uncomp_size = (int)rd_u32(data, pos + 24);
        int name_len = (int)rd_u16(data, pos + 28);
        int extra_len = (int)rd_u16(data, pos + 30);
        int comment_len = (int)rd_u16(data, pos + 32);
        int local_off = (int)rd_u32(data, pos + 42);
        unsigned crc = rd_u32(data, entry_start + 16);
        if (pos + 46 + name_len > fsize) { free(data); px_error("zip 中央目录条目越界"); }
        char* name = (char*)malloc(name_len + 1);
        memcpy(name, data + pos + 46, name_len);
        name[name_len] = '\0';
        pos += 46 + name_len + extra_len + comment_len;
        // 目录条目跳过
        if (name_len > 0 && name[name_len - 1] == '/') { free(name); continue; }
        // 安全校验：拒绝绝对路径与路径穿越
        int unsafe = name[0] == '\0' || name[0] == '/';
        for (int k = 0; k < name_len && !unsafe; k++) {
            if (k + 1 < name_len && name[k] == '.' && name[k + 1] == '.')
                if (k == 0 || name[k - 1] == '/') unsafe = 1;
        }
        if (unsafe) { free(data); px_error("zip 条目路径非法: '%s'", name); }
        // 读取 local header 与数据
        if (local_off + 30 > fsize || data[local_off] != 'P' || data[local_off + 1] != 'K') { free(name); free(data); px_error("zip 本地文件头损坏"); }
        int l_name_len = (int)rd_u16(data, local_off + 26);
        int l_extra_len = (int)rd_u16(data, local_off + 28);
        int data_start = local_off + 30 + l_name_len + l_extra_len;
        if (data_start + comp_size > fsize) { free(name); free(data); px_error("zip 文件数据越界"); }
        // 解压
        unsigned char* content = (unsigned char*)malloc(uncomp_size > 0 ? uncomp_size : 1);
        int content_len;
        if (method == 0) {
            if (comp_size != uncomp_size) { free(name); free(content); free(data); px_error("zip store 长度不符"); }
            memcpy(content, data + data_start, comp_size);
            content_len = comp_size;
        } else if (method == 8) {
            if (z_raw_inflate(data + data_start, comp_size, content, uncomp_size, &content_len) != 0) {
                free(name); free(content); free(data); px_error("zip 解压失败: '%s'", name);
            }
        } else {
            free(name); free(content); free(data);
            px_error("zip 不支持的压缩方法: %d", method);
        }
        if (content_len != uncomp_size) { free(name); free(content); free(data); px_error("zip 解压长度不符"); }
        if (mz_crc32(0, content, (size_t)content_len) != crc) { free(name); free(content); free(data); px_error("zip CRC32 校验失败: '%s'", name); }
        // 写文件
        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", out_dir, name);
        z_mkdirs(full);
        FILE* wf = fopen(full, "wb");
        if (!wf) { free(name); free(content); free(data); px_error("zip 写入文件失败 %s", full); }
        int wok = fwrite(content, 1, content_len, wf) == (size_t)content_len;
        fclose(wf);
        free(name); free(content);
        if (!wok) { free(data); px_error("zip 写入文件失败 %s", full); }
        count++;
    }
    free(data);
    return px_int(count);
}
