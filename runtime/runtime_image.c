// 普贤 (PuXian) M103-S2d（Issue 30 GAP-IMG）：图片解码 / 等比缩放 / JPEG 编码
// 实现：stb_image v2.30（解码 png/jpeg/webp/bmp/gif 等 → RGBA）+ stb_image_write v1.16
//   （JPEG 编码，质量可配）。api-server avatar.go「解码→>512 等比缩放→JPEG q70 重编码」
//   的头像接口 native 解锁——纯 .px 无法像素级实现（无 DCT/哈夫曼/输入解码）。
//
// 第三方引入记录（规范：来源/版本/许可/sha256 归档，循 tweetnacl 先例）：
//   源码  https://raw.githubusercontent.com/nothings/stb/master/stb_image.h         （public domain / MIT 双许可）
//         https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
//   版本  stb_image v2.30 / stb_image_write v1.16（上游头内版本行）
//   sha256（本地未改动，逐字节与上游一致）：
//     stb_image.h        594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3
//     stb_image_write.h  cbd5f0ad7a9cf4468affb36354a1d2338034f2c12473cf1a8e32053cb6914a05
//
// - img_decode(data:bytes|str) → dict{w,h,pixels:bytes(RGBA, w*h*4, 行序自顶向下)}
//     | Err("img: ...")——解码失败/空输入/超限返回可判定 Err；png/jpeg/webp 输入均可。
// - img_scale(pixels:bytes, w:int, h:int, nw:int, nh:int) → bytes(RGBA nw*nh*4)
//     | Err——双线性插值等比缩放（avatar ≤512 用；RGBA 输入输出）。
// - img_encode_jpeg(w:int, h:int, pixels:bytes(RGBA), q:int) → bytes(JPEG) | Err
//     ——RGBA→RGB 后 stbi_write_jpg_to_func 内存缓冲（无文件 IO）；q 1-100 clamp。
#include "runtime.h"

#define STB_IMAGE_IMPLEMENTATION
// 输入面裁剪：只需 png/jpeg/webp（api-server 头像上传场景）；裁余量缩编译体积
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_HDR
#define STBI_NO_TGA
#define STBI_NO_PSD
#define STBI_NO_PVR
#define STBI_NO_PKM
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// ---- 取 str|bytes 指针与长度（二进制安全；类型错 px_error）----
static const unsigned char* img_bytes(LXValue v, int* len) {
    if (v.type == PX_STR || v.type == PX_BYTES) {
        *len = v.as.obj->as.str.len;
        return (const unsigned char*)v.as.obj->as.str.data;
    }
    px_error("期望字符串或 bytes，实际是 %s", px_type_name(v));
    return NULL;
}

// img_decode(data) → {w,h,pixels:bytes RGBA} | Err
LXValue bi_img_decode(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("img_decode 需要 1 个参数: (data)");
    int dlen = 0;
    const unsigned char* data = img_bytes(args[0], &dlen);
    if (!data || dlen <= 0) return px_err(px_str("img: 空输入"));
    int w = 0, h = 0, ch = 0;
    unsigned char* px = stbi_load_from_memory(data, dlen, &w, &h, &ch, 4);  // 强制 RGBA
    if (!px || w <= 0 || h <= 0) return px_err(px_str("img: 解码失败（非支持图片格式或数据损坏）"));
    if ((int64_t)w * h > 100000000LL) {  // 1 亿像素上限（防御恶意超大图）
        stbi_image_free(px);
        return px_err(px_str("img: 图片过大"));
    }
    LXValue d = px_dict();
    px_root_push();
    PX_KEEP(d);
    px_dict_set(d, "w", px_int(w));
    px_dict_set(d, "h", px_int(h));
    LXValue pix = px_bytes_len(px, w * h * 4);
    px_dict_set(d, "pixels", pix);
    px_root_pop();
    stbi_image_free(px);
    return d;
}

// ---- 双线性插值 RGBA 缩放 ----
static void img_bilinear(const unsigned char* src, int sw, int sh,
                         unsigned char* dst, int dw, int dh) {
    for (int y = 0; y < dh; y++) {
        float sy = ((float)y + 0.5f) * sh / dh - 0.5f;
        if (sy < 0) sy = 0;
        int y0 = (int)sy;
        if (y0 >= sh - 1) y0 = sh - 1;
        int y1 = y0 + 1 < sh ? y0 + 1 : y0;
        float wy = sy - y0;
        for (int x = 0; x < dw; x++) {
            float sx = ((float)x + 0.5f) * sw / dw - 0.5f;
            if (sx < 0) sx = 0;
            int x0 = (int)sx;
            if (x0 >= sw - 1) x0 = sw - 1;
            int x1 = x0 + 1 < sw ? x0 + 1 : x0;
            float wx = sx - x0;
            const unsigned char* p00 = src + (y0 * sw + x0) * 4;
            const unsigned char* p01 = src + (y0 * sw + x1) * 4;
            const unsigned char* p10 = src + (y1 * sw + x0) * 4;
            const unsigned char* p11 = src + (y1 * sw + x1) * 4;
            unsigned char* o = dst + (y * dw + x) * 4;
            for (int c = 0; c < 4; c++) {
                float top = p00[c] + (p01[c] - p00[c]) * wx;
                float bot = p10[c] + (p11[c] - p10[c]) * wx;
                float v = top + (bot - top) * wy;
                o[c] = (unsigned char)(v + 0.5f);
            }
        }
    }
}

// img_scale(pixels RGBA, w, h, nw, nh) → bytes RGBA | Err
LXValue bi_img_scale(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 5) px_error("img_scale 需要 5 个参数: (pixels, w, h, nw, nh)");
    int plen = 0;
    const unsigned char* pix = img_bytes(args[0], &plen);
    int64_t w = args[1].as.i, h = args[2].as.i, nw = args[3].as.i, nh = args[4].as.i;
    if (args[1].type != PX_INT || args[2].type != PX_INT ||
        args[3].type != PX_INT || args[4].type != PX_INT)
        px_error("img_scale 的 w/h/nw/nh 需要 int");
    if (w <= 0 || h <= 0 || nw <= 0 || nh <= 0 || nw > 100000 || nh > 100000)
        return px_err(px_str("img: 非法尺寸"));
    if (!pix || plen < w * h * 4) return px_err(px_str("img: pixels 长度不足 w*h*4"));
    if ((int64_t)nw * nh > 100000000LL) return px_err(px_str("img: 输出过大"));
    unsigned char* tmp = (unsigned char*)malloc((size_t)(nw * nh * 4));
    if (!tmp) return px_err(px_str("img: OOM"));
    img_bilinear(pix, (int)w, (int)h, tmp, (int)nw, (int)nh);
    LXValue r = px_bytes_len(tmp, (int)(nw * nh * 4));
    free(tmp);
    return r;
}

// ---- JPEG 内存写缓冲（stbi_write_jpg_to_func 回调收集）----
typedef struct { unsigned char* buf; int len; int cap; int fail; } JpgCtx;
static void jpg_write_fn(void* ctx, void* data, int size) {
    JpgCtx* c = (JpgCtx*)ctx;
    if (c->fail || size < 0) return;
    if (c->len + size > c->cap) {
        int ncap = c->cap ? c->cap * 2 : 65536;
        while (ncap < c->len + size) ncap *= 2;
        unsigned char* nb = (unsigned char*)realloc(c->buf, (size_t)ncap);
        if (!nb) { c->fail = 1; return; }
        c->buf = nb;
        c->cap = ncap;
    }
    memcpy(c->buf + c->len, data, (size_t)size);
    c->len += size;
}

// img_encode_jpeg(w, h, pixels RGBA, q) → bytes(JPEG) | Err
LXValue bi_img_encode_jpeg(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 4) px_error("img_encode_jpeg 需要 4 个参数: (w, h, pixels, q)");
    int plen = 0;
    const unsigned char* pix = img_bytes(args[2], &plen);
    int64_t w = args[0].as.i, h = args[1].as.i, q = args[3].as.i;
    if (args[0].type != PX_INT || args[1].type != PX_INT || args[3].type != PX_INT)
        px_error("img_encode_jpeg 的 w/h/q 需要 int");
    if (w <= 0 || h <= 0 || (int64_t)w * h > 100000000LL)
        return px_err(px_str("img: 非法尺寸"));
    if (!pix || plen < w * h * 4) return px_err(px_str("img: pixels 长度不足 w*h*4"));
    if (q < 1) q = 1;
    if (q > 100) q = 100;
    // RGBA → RGB（JPEG 无 alpha）
    unsigned char* rgb = (unsigned char*)malloc((size_t)(w * h * 3));
    if (!rgb) return px_err(px_str("img: OOM"));
    for (int64_t i = 0; i < w * h; i++) {
        rgb[i * 3] = pix[i * 4];
        rgb[i * 3 + 1] = pix[i * 4 + 1];
        rgb[i * 3 + 2] = pix[i * 4 + 2];
    }
    JpgCtx jc;
    memset(&jc, 0, sizeof(jc));
    int rc = stbi_write_jpg_to_func(jpg_write_fn, &jc, (int)w, (int)h, 3, rgb, (int)q);
    free(rgb);
    if (rc == 0 || jc.fail || !jc.buf) {
        free(jc.buf);
        return px_err(px_str("img: JPEG 编码失败"));
    }
    LXValue r = px_bytes_len(jc.buf, jc.len);
    free(jc.buf);
    return r;
}
