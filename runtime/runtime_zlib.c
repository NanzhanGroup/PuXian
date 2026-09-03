// 普贤 (PuXian) C 运行时 — zlib 外部系统库绑定（M61-S1，A 线 FFI proof）
// ------------------------------------------------------------
// 语言层（examples/m61_zlib/m61_zlib.px）：
//   import "c/zlib"
//   extern def zlib_crc32(data: bytes) -> int        # CRC-32（标准值可校验）
//   extern def zlib_compress(data: bytes, level: int) -> bytes   # 压缩
//   extern def zlib_uncompress(data: bytes) -> bytes             # 解压
// 机制（与 runtime_ffi.c / runtime_quic.c 完全同构）：
//   注册进 FFI 表（px_ffi_register），双模式统一走 bi_ffi_call（M42 C 桥）；
//   真正链接外部 zlib 静态库 libz.a（pxc 无条件链，M61-S0；x86_64 与 aarch64
//   两版入库 runtime/third_party/zlib/{lib,lib-aarch64}）。
//   crc32()/compress2()/inflate() 均来自 libz.a → 验证「外部 .a 入库 → C 薄胶水
//   注册 → pxc 链接 → 语言调用 → 跨架构」全链路（GAP「FFI 只绑过内部库」真缺口）。
// 语义：参数/返回值 str|bytes 均走 union str 的 data/len（binary-safe，可含 NUL）；
//       compress2 内用 uLongf* 长度指针（cap→实际）承载 out 长度（FFI 指针压力位）；
//       uncompress 用 z_stream inflate 渐进扩容（真实世界任意 deflate 流，免预知大小）。
// 失败语义：内存/参数错误 → px_error 终止（编程契约）；数据非法（rc != Z_OK）→
//       uncompress/compress 返回 null（与 M57 设备层「失败不杀进程」哲学一致）。
#define _GNU_SOURCE
#include "runtime.h"
#include <zlib.h>
#include <stdlib.h>
#include <string.h>

// str/bytes 通用取数据指针 + 长度（union 复用 str 的 data/len）
static const char* z_buf(LXValue v) {
    if (v.type != PX_STR && v.type != PX_BYTES)
        px_error("zlib: 参数需要 str/bytes");
    return v.as.obj->as.str.data;
}
static int z_buflen(LXValue v) {
    if (v.type != PX_STR && v.type != PX_BYTES)
        px_error("zlib: 参数需要 str/bytes");
    return v.as.obj->as.str.len;
}

// zlib_crc32(data) -> int：标准 CRC-32（IEEE，poly 0xEDB88320）
//   crc32(0L, data, len) 的 0L 为初始值（语言侧恒用 0 起始）
static LXValue bi_zlib_crc32(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("zlib_crc32 需要 1 个参数 (data)");
    const char* d = z_buf(args[0]);
    int n = z_buflen(args[0]);
    uLong c = crc32(0L, (const Bytef*)d, (uInt)n);
    return px_int((int64_t)c);
}

// zlib_compress(data, level) -> bytes | null
//   compress2(dst,&dstLen,src,srcLen,level)：dstLen 为 uLongf* 长度指针（cap→实际），
//   语言侧无需预分配 —— C 内 compressBound 定容、压缩后按实际长度建 bytes 返回。
static LXValue bi_zlib_compress(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("zlib_compress 需要 2 个参数 (data, level)");
    if (args[1].type != PX_INT) px_error("zlib_compress 的 level 需要 int");
    const char* src = z_buf(args[0]);
    int slen = z_buflen(args[0]);
    int level = (int)args[1].as.i;
    if (level < 0 || level > 9) px_error("zlib_compress 的 level 需在 0..9");
    uLongf cap = compressBound((uLong)(slen > 0 ? (size_t)slen : 1));
    Bytef* out = (Bytef*)malloc(cap);
    if (!out) return px_null();
    uLongf olen = cap;
    int rc = compress2(out, &olen, (const Bytef*)src, (uLong)slen, level);
    if (rc != Z_OK) { free(out); return px_null(); }
    LXValue r = px_bytes_len(out, (int)olen);
    free(out);
    return r;
}

// zlib_uncompress(data) -> bytes | null
//   z_stream inflate 渐进扩容解压：不要求语言侧预知解压后大小；
//   非法/截断 deflate 流返回 null（不杀进程）。
static LXValue bi_zlib_uncompress(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("zlib_uncompress 需要 1 个参数 (data)");
    const char* src = z_buf(args[0]);
    int slen = z_buflen(args[0]);
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit(&zs) != Z_OK) return px_null();
    zs.next_in = (Bytef*)(void*)(uintptr_t)src;
    zs.avail_in = (uInt)slen;
    size_t cap = (size_t)slen + 512;
    if (cap < 1024) cap = 1024;
    char* out = (char*)malloc(cap);
    if (!out) { inflateEnd(&zs); return px_null(); }
    int rc;
    for (;;) {
        if ((size_t)zs.total_out >= cap) {           // 输出满 → 扩容
            size_t ncap = cap * 2;
            char* n = (char*)realloc(out, ncap);
            if (!n) { free(out); inflateEnd(&zs); return px_null(); }
            out = n; cap = ncap;
            continue;
        }
        zs.next_out = (Bytef*)(out + zs.total_out);
        zs.avail_out = (uInt)(cap - (size_t)zs.total_out);
        rc = inflate(&zs, Z_NO_FLUSH);
        if (rc == Z_STREAM_END) break;
        if (rc != Z_OK) { free(out); inflateEnd(&zs); return px_null(); }
        if (zs.avail_in == 0) {                       // 输入耗尽仍未结束 → 截断/非法
            free(out); inflateEnd(&zs); return px_null();
        }
        // rc==Z_OK 且仍有输入：要么输出已满（下轮扩容），要么异常 → 安全阀防死循环
        if (zs.avail_out > 0) { free(out); inflateEnd(&zs); return px_null(); }
    }
    LXValue r = px_bytes_len(out, (int)zs.total_out);
    inflateEnd(&zs);
    free(out);
    return r;
}

// 注册（runtime.c px_register_builtins 无条件调用：libz.a 恒链，无需 PX_NO_QUIC 条件）
void px_register_zlib(void) {
    px_ffi_register("zlib_crc32", bi_zlib_crc32);
    px_ffi_register("zlib_compress", bi_zlib_compress);
    px_ffi_register("zlib_uncompress", bi_zlib_uncompress);
}
