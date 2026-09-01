// 普贤 (PuXian) M19 XML 内置函数（编译模式，与解释器 Rust 输出结构一致）
// - xml_parse(xml) → dict{name, attrs, children, text} 或报错（解析失败）
// - xml_escape(text) / xml_unescape(text)：实体转义/反转义
// - xml_build(node) → str（M24：与 xml_parse 结构对称的 XML 生成）
#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 解析游标
typedef struct {
    const char* s;
    int len;
    int pos;
} XmlP;

// 简易动态字符串缓冲（\0 结尾）
typedef struct {
    char* data;
    int len, cap;
} XBuf;

static void xbuf_push(XBuf* b, char c) {
    if (b->len + 1 >= b->cap) {
        b->cap = b->cap ? b->cap * 2 : 64;
        b->data = (char*)realloc(b->data, b->cap);
    }
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}

static char* xbuf_detach(XBuf* b) { char* d = b->data; *b = (XBuf){0}; return d; }

static int xp_peek(XmlP* p) { return p->pos < p->len ? (unsigned char)p->s[p->pos] : -1; }
static int xp_next(XmlP* p) { return p->pos < p->len ? (unsigned char)p->s[p->pos++] : -1; }

static int xp_starts(XmlP* p, const char* s) {
    int n = (int)strlen(s);
    if (p->pos + n > p->len) return 0;
    return memcmp(p->s + p->pos, s, n) == 0;
}

static void xp_eat_ws(XmlP* p) {
    while (p->pos < p->len) {
        char c = p->s[p->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') p->pos++;
        else break;
    }
}

// 从当前位置查找子串，返回相对偏移或 -1
static int xp_find(XmlP* p, const char* s) {
    int n = (int)strlen(s);
    for (int i = p->pos; i + n <= p->len; i++) {
        if (memcmp(p->s + i, s, n) == 0) return i - p->pos;
    }
    return -1;
}

// 解码实体（&amp; &lt; &gt; &quot; &apos; &#nn; &#xhh;），返回 malloc 缓冲（\0 结尾）
static char* xml_decode_entities(const char* s, int len) {
    XBuf out = {0};
    int i = 0;
    while (i < len) {
        if (s[i] == '&') {
            int semi = -1;
            for (int j = i + 1; j < len && j <= i + 12; j++) {
                if (s[j] == ';') { semi = j; break; }
            }
            if (semi >= 0) {
                int elen = semi - (i + 1);
                int matched = 0;
                if (elen == 3 && memcmp(s + i + 1, "amp", 3) == 0) { xbuf_push(&out, '&'); matched = 1; }
                else if (elen == 2 && memcmp(s + i + 1, "lt", 2) == 0) { xbuf_push(&out, '<'); matched = 1; }
                else if (elen == 2 && memcmp(s + i + 1, "gt", 2) == 0) { xbuf_push(&out, '>'); matched = 1; }
                else if (elen == 4 && memcmp(s + i + 1, "quot", 4) == 0) { xbuf_push(&out, '"'); matched = 1; }
                else if (elen == 4 && memcmp(s + i + 1, "apos", 4) == 0) { xbuf_push(&out, '\''); matched = 1; }
                else if (elen >= 2 && s[i + 1] == '#') {
                    int base = 10, start = i + 2;
                    if (s[i + 2] == 'x' || s[i + 2] == 'X') { base = 16; start = i + 3; }
                    unsigned int cp = 0; int ok = 1;
                    for (int k = start; k < semi; k++) {
                        int d;
                        char c = s[k];
                        if (c >= '0' && c <= '9') d = c - '0';
                        else if (base == 16 && c >= 'a' && c <= 'f') d = c - 'a' + 10;
                        else if (base == 16 && c >= 'A' && c <= 'F') d = c - 'A' + 10;
                        else { ok = 0; break; }
                        cp = cp * base + (unsigned int)d;
                    }
                    if (ok && cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF)) {
                        // 编码 UTF-8（对齐解释器：所有合法码点，含 ASCII）
                        if (cp < 0x80) { xbuf_push(&out, (char)cp); }
                        else if (cp < 0x800) { xbuf_push(&out, (char)(0xC0 | (cp >> 6))); xbuf_push(&out, (char)(0x80 | (cp & 0x3F))); }
                        else if (cp < 0x10000) {
                            xbuf_push(&out, (char)(0xE0 | (cp >> 12)));
                            xbuf_push(&out, (char)(0x80 | ((cp >> 6) & 0x3F)));
                            xbuf_push(&out, (char)(0x80 | (cp & 0x3F)));
                        } else {
                            xbuf_push(&out, (char)(0xF0 | (cp >> 18)));
                            xbuf_push(&out, (char)(0x80 | ((cp >> 12) & 0x3F)));
                            xbuf_push(&out, (char)(0x80 | ((cp >> 6) & 0x3F)));
                            xbuf_push(&out, (char)(0x80 | (cp & 0x3F)));
                        }
                        matched = 1;
                    }
                }
                if (matched) { i = semi + 1; continue; }
            }
        }
        xbuf_push(&out, s[i++]);
    }
    return xbuf_detach(&out);
}

// 复制字节范围 → \0 结尾缓冲
static char* xml_slice(const char* s, int start, int len) {
    char* r = (char*)malloc(len + 1);
    memcpy(r, s + start, len);
    r[len] = '\0';
    return r;
}

// 解析名字（直到空白/>/=）
static int xml_parse_name(XmlP* p, int* start) {
    *start = p->pos;
    while (p->pos < p->len) {
        char c = p->s[p->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '>' || c == '/' || c == '=') break;
        p->pos++;
    }
    return p->pos - *start;
}

// 跳过注释 / 处理指令 / DOCTYPE（返回 0 表示结构错误）
static int xml_skip_misc(XmlP* p) {
    for (;;) {
        xp_eat_ws(p);
        if (xp_starts(p, "<!--")) {
            int rel = xp_find(p, "-->");
            if (rel < 0) return 0;
            p->pos += rel + 3;
        } else if (xp_starts(p, "<?")) {
            int rel = xp_find(p, "?>");
            if (rel < 0) return 0;
            p->pos += rel + 2;
        } else if (xp_starts(p, "<!DOCTYPE") || xp_starts(p, "<!doctype")) {
            int depth = 0;
            while (p->pos < p->len) {
                char c = p->s[p->pos++];
                if (c == '[') depth++;
                else if (c == ']') { if (depth > 0) depth--; }
                else if (c == '>' && depth == 0) break;
            }
        } else {
            break;
        }
    }
    return 1;
}

static LXValue xml_parse_element(XmlP* p);

// 解析元素内容直到结束标签；children 收集中，text 拼接到 textbuf
static int xml_parse_content(XmlP* p, const char* name, LXValue children, XBuf* textbuf) {
    for (;;) {
        if (p->pos >= p->len) return 0;  // 未闭合
        if (xp_peek(p) == '<') {
            if (xp_starts(p, "</")) {
                p->pos += 2;
                int ns; int nl = xml_parse_name(p, &ns);
                xp_eat_ws(p);
                if (xp_next(p) != '>') return 0;
                if (nl != (int)strlen(name) || memcmp(p->s + ns, name, nl) != 0) return 0;
                return 1;
            } else if (xp_starts(p, "<!--")) {
                int rel = xp_find(p, "-->");
                if (rel < 0) return 0;
                p->pos += rel + 3;
            } else if (xp_starts(p, "<![CDATA[")) {
                p->pos += 9;
                int rel = xp_find(p, "]]>");
                if (rel < 0) return 0;
                // CDATA：原样文本（不解码实体），作为独立文本节点
                px_list_push(children, px_str_len(p->s + p->pos, rel));
                p->pos += rel + 3;
            } else if (xp_starts(p, "<?")) {
                int rel = xp_find(p, "?>");
                if (rel < 0) return 0;
                p->pos += rel + 2;
            } else {
                if (textbuf->len > 0) {
                    char* dec = xml_decode_entities(textbuf->data, textbuf->len);
                    px_list_push(children, px_str(dec));
                    free(dec);
                    textbuf->len = 0; if (textbuf->data) textbuf->data[0] = '\0';
                }
                LXValue child = xml_parse_element(p);
                px_list_push(children, child);
            }
        } else {
            char c = (char)xp_next(p);
            xbuf_push(textbuf, c);
        }
    }
}

// 解析一个元素（pos 指向 '<'）
static LXValue xml_parse_element(XmlP* p) {
    if (xp_next(p) != '<') px_error("XML 解析错误：期望 '<'");
    int ns; int nl = xml_parse_name(p, &ns);
    if (nl == 0) px_error("XML 解析错误：缺少标签名");
    char* name = xml_slice(p->s, ns, nl);

    LXValue node = px_dict();
    LXValue attrs = px_dict();
    // 属性
    for (;;) {
        xp_eat_ws(p);
        int c = xp_peek(p);
        if (c == '>' || c == '/') break;
        if (c < 0) { free(name); px_error("XML 解析错误：标签未闭合"); }
        int ks; int kl = xml_parse_name(p, &ks);
        if (kl == 0) { free(name); px_error("XML 解析错误：属性名缺失"); }
        xp_eat_ws(p);
        if (xp_next(p) != '=') { free(name); px_error("XML 解析错误：属性缺少 '='"); }
        xp_eat_ws(p);
        int q = xp_next(p);
        if (q != '"' && q != '\'') { free(name); px_error("XML 解析错误：属性值必须用引号括起"); }
        int vstart = p->pos;
        while (p->pos < p->len && p->s[p->pos] != q) p->pos++;
        if (p->pos >= p->len) { free(name); px_error("XML 解析错误：属性值未闭合"); }
        char* key = xml_slice(p->s, ks, kl);
        char* rawv = xml_slice(p->s, vstart, p->pos - vstart);
        p->pos++;
        char* val = xml_decode_entities(rawv, (int)strlen(rawv));
        px_dict_set(attrs, key, px_str(val));
        free(key); free(rawv); free(val);
    }
    px_dict_set(node, "name", px_str(name));
    px_dict_set(node, "attrs", attrs);

    if (xp_peek(p) == '/') {
        xp_next(p);
        if (xp_next(p) != '>') { free(name); px_error("XML 解析错误：自闭合标签格式错误"); }
        px_dict_set(node, "children", px_list(0));
        px_dict_set(node, "text", px_str(""));
        free(name);
        return node;
    }
    if (xp_next(p) != '>') { free(name); px_error("XML 解析错误：标签未以 '>' 结束"); }

    LXValue children = px_list(0);
    XBuf textbuf = {0};
    if (!xml_parse_content(p, name, children, &textbuf)) {
        free(name);
        if (textbuf.data) free(textbuf.data);
        px_error("XML 解析错误：元素 <%s> 未闭合或结束标签不匹配", name);
    }
    if (textbuf.len > 0) {
        char* dec = xml_decode_entities(textbuf.data, textbuf.len);
        px_list_push(children, px_str(dec));
        free(dec);
        textbuf.len = 0;
    }
    if (textbuf.data) free(textbuf.data);
    // text 字段 = 直接文本拼接（children 中的字符串节点）
    XBuf tbuf = {0};
    for (int i = 0; i < children.as.obj->as.list.len; i++) {
        LXValue item = children.as.obj->as.list.items[i];
        if (item.type == PX_STR) {
            for (int k = 0; k < item.as.obj->as.str.len; k++)
                xbuf_push(&tbuf, item.as.obj->as.str.data[k]);
        }
    }
    px_dict_set(node, "children", children);
    px_dict_set(node, "text", tbuf.data ? px_str(tbuf.data) : px_str(""));
    if (tbuf.data) free(tbuf.data);
    free(name);
    return node;
}

// xml_parse(xml) → dict 或报错
LXValue bi_xml_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("xml_parse 需要 1 个参数");
    if (args[0].type != PX_STR) px_error("xml_parse 期望字符串");
    XmlP p = { args[0].as.obj->as.str.data, args[0].as.obj->as.str.len, 0 };
    if (!xml_skip_misc(&p)) px_error("XML 解析错误：注释/处理指令未闭合");
    if (xp_peek(&p) != '<') px_error("XML 解析错误：缺少根元素");
    LXValue root = xml_parse_element(&p);
    if (!xml_skip_misc(&p)) px_error("XML 解析错误：根元素后存在非法内容");
    return root;
}

// xml_escape(text) → str
LXValue bi_xml_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("xml_escape 需要 1 个参数");
    if (args[0].type != PX_STR) px_error("xml_escape 期望字符串");
    const char* s = args[0].as.obj->as.str.data;
    int len = args[0].as.obj->as.str.len;
    XBuf out = {0};
    for (int i = 0; i < len; i++) {
        char c = s[i];
        switch (c) {
            case '&': xbuf_push(&out, '&'); xbuf_push(&out, 'a'); xbuf_push(&out, 'm'); xbuf_push(&out, 'p'); xbuf_push(&out, ';'); break;
            case '<': xbuf_push(&out, '&'); xbuf_push(&out, 'l'); xbuf_push(&out, 't'); xbuf_push(&out, ';'); break;
            case '>': xbuf_push(&out, '&'); xbuf_push(&out, 'g'); xbuf_push(&out, 't'); xbuf_push(&out, ';'); break;
            case '"': xbuf_push(&out, '&'); xbuf_push(&out, 'q'); xbuf_push(&out, 'u'); xbuf_push(&out, 'o'); xbuf_push(&out, 't'); xbuf_push(&out, ';'); break;
            case '\'': xbuf_push(&out, '&'); xbuf_push(&out, 'a'); xbuf_push(&out, 'p'); xbuf_push(&out, 'o'); xbuf_push(&out, 's'); xbuf_push(&out, ';'); break;
            default: xbuf_push(&out, c); break;
        }
    }
    LXValue r = out.data ? px_str(out.data) : px_str("");
    if (out.data) free(out.data);
    return r;
}

// xml_unescape(text) → str
LXValue bi_xml_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("xml_unescape 需要 1 个参数");
    if (args[0].type != PX_STR) px_error("xml_unescape 期望字符串");
    const char* s = args[0].as.obj->as.str.data;
    int len = args[0].as.obj->as.str.len;
    char* dec = xml_decode_entities(s, len);
    LXValue r = px_str(dec);
    free(dec);
    return r;
}

// ==================== M24：XML 生成（xml_build） ====================
// xml_build(node) → str；node 为 dict{name, attrs?, children?, text?} 或 str（纯文本）。
// 与解释器 Rust xml.rs build 逐字节一致：
//   - 属性键排序输出（strcmp），值只接受 字符串/int/bool/null（float 报错）
//   - 文本与属性值均用 bi_xml_escape 转义（& < > " ' 全部转义）
//   - 空元素（无 children 且无 text）输出自闭合 <name attrs.../>
//   - text 非字符串时静默忽略（与解释器一致）

static void xbuf_push_str(XBuf* b, const char* s) {
    while (*s) xbuf_push(b, *s++);
}

static LXValue xml_build_node(LXValue node) {
    if (node.type == PX_STR) {
        // 纯文本节点 → 转义文本
        return bi_xml_escape(&node, 1, NULL);
    }
    if (node.type != PX_DICT) {
        px_error("xml_build: 节点必须是 dict 或 str，实际是 %s", px_type_name(node));
        return px_null();
    }
    LXValue name_v = px_dict_get(node, "name");
    if (name_v.type != PX_STR) px_error("xml_build: 节点 dict 缺少字符串 name");
    const char* name = name_v.as.obj->as.str.data;

    // 属性（键排序）
    XBuf attr_s = {0};
    LXValue attrs = px_dict_get(node, "attrs");
    if (attrs.type == PX_DICT) {
        LXObject* ao = attrs.as.obj;
        int n = ao->as.dict.len;
        char** keys = (char**)malloc(sizeof(char*) * (size_t)(n > 0 ? n : 1));
        int* idx = (int*)malloc(sizeof(int) * (size_t)(n > 0 ? n : 1));
        for (int i = 0; i < n; i++) { keys[i] = ao->as.dict.keys[i]; idx[i] = i; }
        // 插入排序（属性数通常很少）
        for (int i = 1; i < n; i++) {
            char* k = keys[i]; int id = idx[i]; int j = i - 1;
            while (j >= 0 && strcmp(keys[j], k) > 0) { keys[j + 1] = keys[j]; idx[j + 1] = idx[j]; j--; }
            keys[j + 1] = k; idx[j + 1] = id;
        }
        for (int i = 0; i < n; i++) {
            int id = idx[i];
            LXValue v = ao->as.dict.vals[id];
            const char* vs = NULL;
            char numbuf[64];
            if (v.type == PX_STR) vs = v.as.obj->as.str.data;
            else if (v.type == PX_INT) { snprintf(numbuf, sizeof(numbuf), "%lld", (long long)v.as.i); vs = numbuf; }
            else if (v.type == PX_BOOL) { vs = v.as.b ? "true" : "false"; }
            else if (v.type == PX_NULL) { vs = ""; }
            else { px_error("xml_build: 属性 %s 值必须是字符串/int/bool/null，实际是 %s",
                            ao->as.dict.keys[id], px_type_name(v)); }
            LXValue sv = px_str(vs);
            LXValue escv = bi_xml_escape(&sv, 1, NULL);
            xbuf_push(&attr_s, ' ');
            xbuf_push_str(&attr_s, ao->as.dict.keys[id]);
            xbuf_push_str(&attr_s, "=\"");
            xbuf_push_str(&attr_s, escv.as.obj->as.str.data);
            xbuf_push(&attr_s, '"');
        }
        free(keys);
        free(idx);
    }

    // 子节点 + 文本
    XBuf inner = {0};
    LXValue children = px_dict_get(node, "children");
    if (children.type == PX_LIST) {
        LXObject* lo = children.as.obj;
        for (int i = 0; i < lo->as.list.len; i++) {
            LXValue child = xml_build_node(lo->as.list.items[i]);
            if (child.type == PX_STR) xbuf_push_str(&inner, child.as.obj->as.str.data);
        }
    }
    LXValue text = px_dict_get(node, "text");
    if (text.type == PX_STR) {
        LXValue escv = bi_xml_escape(&text, 1, NULL);
        xbuf_push_str(&inner, escv.as.obj->as.str.data);
    }

    XBuf out = {0};
    xbuf_push(&out, '<');
    xbuf_push_str(&out, name);
    if (attr_s.data) xbuf_push_str(&out, attr_s.data);
    if (inner.len == 0) {
        xbuf_push_str(&out, "/>");
    } else {
        xbuf_push(&out, '>');
        xbuf_push_str(&out, inner.data ? inner.data : "");
        xbuf_push_str(&out, "</");
        xbuf_push_str(&out, name);
        xbuf_push(&out, '>');
    }
    LXValue r = out.data ? px_str(out.data) : px_str("");
    if (out.data) free(out.data);
    if (attr_s.data) free(attr_s.data);
    if (inner.data) free(inner.data);
    return r;
}

// xml_build(node) → str
LXValue bi_xml_build(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("xml_build 需要 1 个参数");
    return xml_build_node(args[0]);
}
