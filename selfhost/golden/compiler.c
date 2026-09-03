/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_peek(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    if (px_is_truthy(px_lt(px_get_global("g_pos"), px_get_global("g_len")))) {
        return px_index(px_get_global("g_src"), px_get_global("g_pos"));
    }
    return px_str("");
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
    return px_null();
}

static LXValue fn_peek2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    if (px_is_truthy(px_lt(px_add(px_get_global("g_pos"), px_int(1LL)), px_get_global("g_len")))) {
        return px_index(px_get_global("g_src"), px_add(px_get_global("g_pos"), px_int(1LL)));
    }
    return px_str("");
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_peek3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    if (px_is_truthy(px_lt(px_add(px_get_global("g_pos"), px_int(2LL)), px_get_global("g_len")))) {
        return px_index(px_get_global("g_src"), px_add(px_get_global("g_pos"), px_int(2LL)));
    }
    return px_str("");
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_advance(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v4 = px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    if (px_is_truthy(px_ge(px_get_global("g_pos"), px_get_global("g_len")))) {
        return px_str("");
    }
    _v4 = px_index(px_get_global("g_src"), px_get_global("g_pos"));
    px_set_global("g_pos", px_add(px_get_global("g_pos"), px_int(1LL)));
    if (px_is_truthy(px_eq(_v4, px_str("\n")))) {
        px_set_global("g_line", px_add(px_get_global("g_line"), px_int(1LL)));
        px_set_global("g_col", px_int(1LL));
    }
    else {
        px_set_global("g_col", px_add(px_get_global("g_col"), px_int(1LL)));
    }
    return _v4;
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_emit_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v6 = (nargs > 0) ? args[0] : px_null();
    LXValue _v7 = (nargs > 1) ? args[1] : px_null();
    LXValue _v8 = (nargs > 2) ? args[2] : px_null();
    LXValue _v9 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    (void)(px_method(px_get_global("g_toks"), "append", (LXValue[]){px_list_n((LXValue[]){_v6, _v7, _v8, _v9}, 4)}, 1));
    px_set_global("g_count", px_add(px_get_global("g_count"), px_int(1LL)));
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_emit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v11 = (nargs > 0) ? args[0] : px_null();
    LXValue _v12 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    (void)(px_call(px_get_global("emit_at"), (LXValue[]){_v11, _v12, px_get_global("g_line"), px_get_global("g_col")}, 4));
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

static LXValue fn_emit_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_15_val = px_null();
    int px_err_15_proped = 0;
    (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_index(_v14, px_int(0LL)), px_index(_v14, px_int(1LL)), px_index(_v14, px_int(2LL)), px_index(_v14, px_int(3LL))}, 4));
px_err_15:
    if (px_err_15_proped) return px_err_15_val;
    return px_null();
}

static LXValue fn_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v16 = (nargs > 0) ? args[0] : px_null();
    LXValue _v17 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_18_val = px_null();
    int px_err_18_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_line")}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_col")}, 1)), px_str(": 词法错误 ")), _v16), px_str(": ")), _v17)}, 1));
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("lex "), _v16)}, 1));
px_err_18:
    if (px_err_18_proped) return px_err_18_val;
    return px_null();
}

static LXValue fn_err_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v19 = (nargs > 0) ? args[0] : px_null();
    LXValue _v20 = (nargs > 1) ? args[1] : px_null();
    LXValue _v21 = (nargs > 2) ? args[2] : px_null();
    LXValue _v22 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_23_val = px_null();
    int px_err_23_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){_v21}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){_v22}, 1)), px_str(": 词法错误 ")), _v19), px_str(": ")), _v20)}, 1));
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("lex "), _v19)}, 1));
px_err_23:
    if (px_err_23_proped) return px_err_23_val;
    return px_null();
}

static LXValue fn_is_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v24 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_25_val = px_null();
    int px_err_25_proped = 0;
    return ({ LXValue _t27 = ({ LXValue _t26 = px_ne(_v24, px_str("")); px_is_truthy(_t26) ? px_ge(_v24, px_str("0")) : _t26; }); px_is_truthy(_t27) ? px_le(_v24, px_str("9")) : _t27; });
px_err_25:
    if (px_err_25_proped) return px_err_25_val;
    return px_null();
}

static LXValue fn_is_hex_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v28 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_29_val = px_null();
    int px_err_29_proped = 0;
    return ({ LXValue _t33 = ({ LXValue _t31 = px_call(px_get_global("is_digit"), (LXValue[]){_v28}, 1); px_is_truthy(_t31) ? _t31 : ({ LXValue _t30 = px_ge(_v28, px_str("a")); px_is_truthy(_t30) ? px_le(_v28, px_str("f")) : _t30; }); }); px_is_truthy(_t33) ? _t33 : ({ LXValue _t32 = px_ge(_v28, px_str("A")); px_is_truthy(_t32) ? px_le(_v28, px_str("F")) : _t32; }); });
px_err_29:
    if (px_err_29_proped) return px_err_29_val;
    return px_null();
}

static LXValue fn_is_alnum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v34 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_35_val = px_null();
    int px_err_35_proped = 0;
    return ({ LXValue _t39 = ({ LXValue _t37 = px_call(px_get_global("is_digit"), (LXValue[]){_v34}, 1); px_is_truthy(_t37) ? _t37 : ({ LXValue _t36 = px_ge(_v34, px_str("a")); px_is_truthy(_t36) ? px_le(_v34, px_str("z")) : _t36; }); }); px_is_truthy(_t39) ? _t39 : ({ LXValue _t38 = px_ge(_v34, px_str("A")); px_is_truthy(_t38) ? px_le(_v34, px_str("Z")) : _t38; }); });
px_err_35:
    if (px_err_35_proped) return px_err_35_val;
    return px_null();
}

static LXValue fn_is_ident_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v40 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_41_val = px_null();
    int px_err_41_proped = 0;
    if (px_is_truthy(px_eq(_v40, px_str("")))) {
        return px_bool(false);
    }
    return ({ LXValue _t46 = ({ LXValue _t45 = ({ LXValue _t44 = ({ LXValue _t42 = px_ge(_v40, px_str("a")); px_is_truthy(_t42) ? px_le(_v40, px_str("z")) : _t42; }); px_is_truthy(_t44) ? _t44 : ({ LXValue _t43 = px_ge(_v40, px_str("A")); px_is_truthy(_t43) ? px_le(_v40, px_str("Z")) : _t43; }); }); px_is_truthy(_t45) ? _t45 : px_eq(_v40, px_str("_")); }); px_is_truthy(_t46) ? _t46 : px_ge(_v40, px_str("")); });
px_err_41:
    if (px_err_41_proped) return px_err_41_val;
    return px_null();
}

static LXValue fn_is_ident_continue(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v47 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_48_val = px_null();
    int px_err_48_proped = 0;
    if (px_is_truthy(px_eq(_v47, px_str("")))) {
        return px_bool(false);
    }
    return ({ LXValue _t49 = px_call(px_get_global("is_ident_start"), (LXValue[]){_v47}, 1); px_is_truthy(_t49) ? _t49 : px_call(px_get_global("is_digit"), (LXValue[]){_v47}, 1); });
px_err_48:
    if (px_err_48_proped) return px_err_48_val;
    return px_null();
}

static LXValue fn_digit_val(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v50 = (nargs > 0) ? args[0] : px_null();
    LXValue _v51 = px_null();
    LXValue _v52 = px_null();
    LXValue px_err_53_val = px_null();
    int px_err_53_proped = 0;
    _v51 = px_str("0123456789abcdefABCDEF");
    _v52 = px_int(0LL);
    while (px_is_truthy(px_lt(_v52, px_call(px_get_global("len"), (LXValue[]){_v51}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v51, _v52), _v50))) {
            if (px_is_truthy(px_ge(_v52, px_int(16LL)))) {
                return px_sub(_v52, px_int(6LL));
            }
            return _v52;
        }
         _v52 = px_add(_v52, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_53:
    if (px_err_53_proped) return px_err_53_val;
    return px_null();
}

static LXValue fn_handle_line_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v54 = px_null();
    LXValue _v55 = px_null();
    LXValue _v56 = px_null();
    LXValue px_err_57_val = px_null();
    int px_err_57_proped = 0;
    _v54 = px_int(0LL);
    while (px_is_truthy(px_bool(true))) {
         _v54 = px_int(0LL);
        while (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(" ")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
             _v54 = px_add(_v54, px_int(1LL));
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\t")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1003"), px_str("缩进禁止使用 tab，请使用空格")}, 2));
        }
        _v55 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v55, px_str("")))) {
            return px_null();
        }
        if (px_is_truthy(px_eq(_v55, px_str("\n")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            continue;
        }
        if (px_is_truthy(px_eq(_v55, px_str("#")))) {
            (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    _v56 = px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)));
    if (px_is_truthy(px_gt(_v54, _v56))) {
        (void)(px_method(px_get_global("g_indent_stack"), "append", (LXValue[]){_v54}, 1));
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("缩进"), px_str("")}, 2));
    }
    else if (px_is_truthy(px_lt(_v54, _v56))) {
        while (px_is_truthy(px_gt(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v54))) {
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        if (px_is_truthy(px_ne(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v54))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E2002"), px_add(px_add(px_str("缩进不一致：当前缩进 "), px_call(px_get_global("str"), (LXValue[]){_v54}, 1)), px_str(" 与上层缩进不匹配"))}, 2));
        }
    }
    px_set_global("g_at_line_start", px_bool(false));
px_err_57:
    if (px_err_57_proped) return px_err_57_val;
    return px_null();
}

static LXValue fn_skip_comment(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v58 = px_null();
    LXValue _v59 = px_null();
    LXValue _v60 = px_null();
    LXValue px_err_61_val = px_null();
    int px_err_61_proped = 0;
    _v58 = px_str("");
    if (px_is_truthy(px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")))) {
         _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
         _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        _v59 = px_int(1LL);
        while (px_is_truthy(px_gt(_v59, px_int(0LL)))) {
            _v60 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            if (px_is_truthy(px_eq(_v60, px_str("")))) {
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1002"), px_str("块注释未闭合（缺少 |#）")}, 2));
            }
            else if (px_is_truthy(({ LXValue _t62 = px_eq(_v60, px_str("#")); px_is_truthy(_t62) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")) : _t62; }))) {
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v59 = px_add(_v59, px_int(1LL));
            }
            else if (px_is_truthy(({ LXValue _t63 = px_eq(_v60, px_str("|")); px_is_truthy(_t63) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("#")) : _t63; }))) {
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v59 = px_sub(_v59, px_int(1LL));
            }
            else {
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
        }
    }
    else {
        while (px_is_truthy(({ LXValue _t64 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t64) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\n")) : _t64; }))) {
             _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    return _v58;
px_err_61:
    if (px_err_61_proped) return px_err_61_val;
    return px_null();
}

static LXValue fn_scan_ident_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v65 = px_null();
    LXValue _v66 = px_null();
    LXValue _v67 = px_null();
    LXValue px_err_68_val = px_null();
    int px_err_68_proped = 0;
    _v65 = px_get_global("g_line");
    _v66 = px_get_global("g_col");
    _v67 = px_str("");
    while (px_is_truthy(px_call(px_get_global("is_ident_continue"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
         _v67 = px_add(_v67, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    if (px_is_truthy(px_method(px_get_global("KEYWORDS"), "has", (LXValue[]){_v67}, 1))) {
        return px_list_n((LXValue[]){px_index(px_get_global("KEYWORDS"), _v67), px_str(""), _v65, _v66}, 4);
    }
    return px_list_n((LXValue[]){px_str("标识符"), _v67, _v65, _v66}, 4);
px_err_68:
    if (px_err_68_proped) return px_err_68_val;
    return px_null();
}

static LXValue fn_scan_radix_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v69 = (nargs > 0) ? args[0] : px_null();
    LXValue _v70 = (nargs > 1) ? args[1] : px_null();
    LXValue _v71 = (nargs > 2) ? args[2] : px_null();
    LXValue _v72 = px_null();
    LXValue _v73 = px_null();
    LXValue _v74 = px_null();
    LXValue _v75 = px_null();
    LXValue _v76 = px_null();
    LXValue _v77 = px_null();
    LXValue px_err_78_val = px_null();
    int px_err_78_proped = 0;
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    _v72 = px_str("");
    while (px_is_truthy(({ LXValue _t79 = px_call(px_get_global("is_alnum"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t79) ? _t79 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
         _v72 = px_add(_v72, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v72}, 1), px_int(0LL)))) {
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_str("进制字面量缺少数字（基数 "), px_call(px_get_global("str"), (LXValue[]){_v69}, 1)), px_str("）"))}, 2));
    }
    _v73 = px_str("");
    _v74 = px_int(0LL);
    while (px_is_truthy(px_lt(_v74, px_call(px_get_global("len"), (LXValue[]){_v72}, 1)))) {
        if (px_is_truthy(px_ne(px_index(_v72, _v74), px_str("_")))) {
             _v73 = px_add(_v73, px_index(_v72, _v74));
        }
         _v74 = px_add(_v74, px_int(1LL));
    }
    _v75 = px_int(0LL);
    _v76 = px_int(0LL);
    while (px_is_truthy(px_lt(_v76, px_call(px_get_global("len"), (LXValue[]){_v73}, 1)))) {
        _v77 = px_call(px_get_global("digit_val"), (LXValue[]){px_index(_v73, _v76)}, 1);
        if (px_is_truthy(({ LXValue _t80 = px_lt(_v77, px_int(0LL)); px_is_truthy(_t80) ? _t80 : px_ge(_v77, _v69); }))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_add(px_str("无效 "), px_call(px_get_global("str"), (LXValue[]){_v69}, 1)), px_str("-进制字面量: ")), _v73)}, 2));
        }
         _v75 = px_add(px_mul(_v75, _v69), _v77);
         _v76 = px_add(_v76, px_int(1LL));
    }
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){_v75}, 1), _v70, _v71}, 4);
px_err_78:
    if (px_err_78_proped) return px_err_78_val;
    return px_null();
}

static LXValue fn_strip_leading_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v81 = (nargs > 0) ? args[0] : px_null();
    LXValue _v82 = px_null();
    LXValue px_err_83_val = px_null();
    int px_err_83_proped = 0;
    _v82 = px_int(0LL);
    while (px_is_truthy(({ LXValue _t84 = px_lt(_v82, px_sub(px_call(px_get_global("len"), (LXValue[]){_v81}, 1), px_int(1LL))); px_is_truthy(_t84) ? px_eq(px_index(_v81, _v82), px_str("0")) : _t84; }))) {
         _v82 = px_add(_v82, px_int(1LL));
    }
    return px_slice(_v81, _v82, px_null(), px_null());
px_err_83:
    if (px_err_83_proped) return px_err_83_val;
    return px_null();
}

static LXValue fn_scan_number_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v85 = px_null();
    LXValue _v86 = px_null();
    LXValue _v87 = px_null();
    LXValue _v88 = px_null();
    LXValue _v89 = px_null();
    LXValue _v90 = px_null();
    LXValue _v91 = px_null();
    LXValue _v92 = px_null();
    LXValue _v93 = px_null();
    LXValue _v94 = px_null();
    LXValue _v95 = px_null();
    LXValue _v96 = px_null();
    LXValue _v97 = px_null();
    LXValue _v98 = px_null();
    LXValue px_err_99_val = px_null();
    int px_err_99_proped = 0;
    _v85 = px_get_global("g_line");
    _v86 = px_get_global("g_col");
    if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("0")))) {
        _v87 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
        if (px_is_truthy(({ LXValue _t100 = px_eq(_v87, px_str("x")); px_is_truthy(_t100) ? _t100 : px_eq(_v87, px_str("X")); }))) {
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(16LL), _v85, _v86}, 3);
        }
        if (px_is_truthy(({ LXValue _t101 = px_eq(_v87, px_str("b")); px_is_truthy(_t101) ? _t101 : px_eq(_v87, px_str("B")); }))) {
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(2LL), _v85, _v86}, 3);
        }
        if (px_is_truthy(({ LXValue _t102 = px_eq(_v87, px_str("o")); px_is_truthy(_t102) ? _t102 : px_eq(_v87, px_str("O")); }))) {
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(8LL), _v85, _v86}, 3);
        }
    }
    _v88 = px_str("");
    while (px_is_truthy(({ LXValue _t103 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t103) ? _t103 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
         _v88 = px_add(_v88, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    _v89 = px_bool(false);
    if (px_is_truthy(({ LXValue _t104 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")); px_is_truthy(_t104) ? px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek2"), (LXValue[]){}, 0)}, 1) : _t104; }))) {
         _v89 = px_bool(true);
         _v88 = px_add(_v88, px_str("."));
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        while (px_is_truthy(({ LXValue _t105 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t105) ? _t105 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
             _v88 = px_add(_v88, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    if (px_is_truthy(({ LXValue _t106 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("e")); px_is_truthy(_t106) ? _t106 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("E")); }))) {
        _v90 = px_get_global("g_pos");
        _v91 = px_get_global("g_line");
        _v92 = px_get_global("g_col");
        _v93 = px_str("");
         _v93 = px_add(_v93, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t107 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("+")); px_is_truthy(_t107) ? _t107 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")); }))) {
             _v93 = px_add(_v93, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
            while (px_is_truthy(({ LXValue _t108 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t108) ? _t108 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
                 _v93 = px_add(_v93, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
             _v88 = px_add(_v88, _v93);
             _v89 = px_bool(true);
        }
        else {
            px_set_global("g_pos", _v90);
            px_set_global("g_line", _v91);
            px_set_global("g_col", _v92);
        }
    }
    _v94 = px_str("");
    _v95 = px_int(0LL);
    while (px_is_truthy(px_lt(_v95, px_call(px_get_global("len"), (LXValue[]){_v88}, 1)))) {
        if (px_is_truthy(px_ne(px_index(_v88, _v95), px_str("_")))) {
             _v94 = px_add(_v94, px_index(_v88, _v95));
        }
         _v95 = px_add(_v95, px_int(1LL));
    }
    if (px_is_truthy(_v89)) {
        _v96 = px_call(px_get_global("float"), (LXValue[]){_v94}, 1);
        _v97 = px_call(px_get_global("str"), (LXValue[]){_v96}, 1);
        if (px_is_truthy(px_call(px_get_global("ends_with"), (LXValue[]){_v97, px_str(".0")}, 2))) {
             _v97 = px_slice(_v97, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v97}, 1), px_int(2LL)), px_null());
        }
        return px_list_n((LXValue[]){px_str("浮点"), _v97, _v85, _v86}, 4);
    }
    _v98 = px_call(px_get_global("strip_leading_zeros"), (LXValue[]){_v94}, 1);
    if (px_is_truthy(({ LXValue _t110 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v98}, 1), px_int(19LL)); px_is_truthy(_t110) ? _t110 : ({ LXValue _t109 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v98}, 1), px_int(19LL)); px_is_truthy(_t109) ? px_gt(_v98, px_str("9223372036854775807")) : _t109; }); }))) {
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_str("无效整数: "), _v88)}, 2));
    }
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v94}, 1)}, 1), _v85, _v86}, 4);
px_err_99:
    if (px_err_99_proped) return px_err_99_val;
    return px_null();
}

static LXValue fn_scan_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v111 = (nargs > 0) ? args[0] : px_null();
    LXValue _v112 = px_null();
    LXValue _v113 = px_null();
    LXValue px_err_114_val = px_null();
    int px_err_114_proped = 0;
    _v112 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){_v111}, 1);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_index(_v112, px_int(0LL))}, 1));
    _v113 = px_int(1LL);
    while (px_is_truthy(px_lt(_v113, px_call(px_get_global("len"), (LXValue[]){_v112}, 1)))) {
        (void)(px_method(px_get_global("g_pending"), "append", (LXValue[]){px_index(_v112, _v113)}, 1));
         _v113 = px_add(_v113, px_int(1LL));
    }
px_err_114:
    if (px_err_114_proped) return px_err_114_val;
    return px_null();
}

static LXValue fn_scan_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v115 = (nargs > 0) ? args[0] : px_null();
    LXValue _v116 = px_null();
    LXValue _v117 = px_null();
    LXValue _v118 = px_null();
    LXValue _v119 = px_null();
    LXValue _v120 = px_null();
    LXValue _v121 = px_null();
    LXValue _v122 = px_null();
    LXValue _v123 = px_null();
    LXValue px_err_124_val = px_null();
    int px_err_124_proped = 0;
    _v116 = px_get_global("g_line");
    _v117 = px_get_global("g_col");
    _v118 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t126 = ({ LXValue _t125 = px_eq(_v118, px_str("\"")); px_is_truthy(_t125) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\"")) : _t125; }); px_is_truthy(_t126) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")) : _t126; }))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("\""), _v116, _v117, _v115}, 4);
    }
    if (px_is_truthy(({ LXValue _t128 = ({ LXValue _t127 = px_eq(_v118, px_str("'")); px_is_truthy(_t127) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("'")) : _t127; }); px_is_truthy(_t128) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")) : _t128; }))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("'"), _v116, _v117, _v115}, 4);
    }
    _v119 = px_list_n((LXValue[]){}, 0);
    _v120 = px_str("");
    while (px_is_truthy(px_bool(true))) {
        _v121 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v121, px_str("")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_add(px_add(px_str("字符串未闭合（缺少 "), _v118), px_str("）")), _v116, _v117}, 4));
        }
        if (px_is_truthy(px_eq(_v121, _v118))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            break;
        }
        if (px_is_truthy(px_eq(_v121, px_str("\\")))) {
             _v120 = px_add(_v120, px_call(px_get_global("scan_escape"), (LXValue[]){_v116, _v117}, 2));
        }
        else if (px_is_truthy(px_eq(_v121, px_str("\n")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v116, _v117}, 4));
        }
        else if (px_is_truthy(({ LXValue _t130 = ({ LXValue _t129 = px_eq(_v121, px_str("$")); px_is_truthy(_t129) ? _v115 : _t129; }); px_is_truthy(_t130) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t130; }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v120}, 1), _v116, _v117}, 4)}, 1));
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v116, _v117}, 4)}, 1));
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v116, _v117}, 4)}, 1));
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v116, _v117}, 4)}, 1));
            _v122 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v116, _v117}, 2);
            _v123 = px_int(0LL);
            while (px_is_truthy(px_lt(_v123, px_call(px_get_global("len"), (LXValue[]){_v122}, 1)))) {
                (void)(px_method(_v119, "append", (LXValue[]){px_index(_v122, _v123)}, 1));
                 _v123 = px_add(_v123, px_int(1LL));
            }
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v116, _v117}, 4)}, 1));
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v116, _v117}, 4)}, 1));
             _v120 = px_str("");
        }
        else {
             _v120 = px_add(_v120, _v121);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v120}, 1), _v116, _v117}, 4)}, 1));
    return _v119;
px_err_124:
    if (px_err_124_proped) return px_err_124_val;
    return px_null();
}

static LXValue fn_scan_multiline_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v131 = (nargs > 0) ? args[0] : px_null();
    LXValue _v132 = (nargs > 1) ? args[1] : px_null();
    LXValue _v133 = (nargs > 2) ? args[2] : px_null();
    LXValue _v134 = (nargs > 3) ? args[3] : px_null();
    LXValue _v135 = px_null();
    LXValue _v136 = px_null();
    LXValue _v137 = px_null();
    LXValue _v138 = px_null();
    LXValue _v139 = px_null();
    LXValue px_err_140_val = px_null();
    int px_err_140_proped = 0;
    _v135 = px_list_n((LXValue[]){}, 0);
    _v136 = px_str("");
    while (px_is_truthy(px_bool(true))) {
        _v137 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v137, px_str("")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("多行字符串未闭合"), _v132, _v133}, 4));
        }
        if (px_is_truthy(({ LXValue _t142 = ({ LXValue _t141 = px_eq(_v137, _v131); px_is_truthy(_t141) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), _v131) : _t141; }); px_is_truthy(_t142) ? px_eq(px_call(px_get_global("peek3"), (LXValue[]){}, 0), _v131) : _t142; }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            break;
        }
        if (px_is_truthy(px_eq(_v137, px_str("\\")))) {
             _v136 = px_add(_v136, px_call(px_get_global("scan_escape"), (LXValue[]){_v132, _v133}, 2));
        }
        else if (px_is_truthy(({ LXValue _t144 = ({ LXValue _t143 = px_eq(_v137, px_str("$")); px_is_truthy(_t143) ? _v134 : _t143; }); px_is_truthy(_t144) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t144; }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v136}, 1), _v132, _v133}, 4)}, 1));
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v132, _v133}, 4)}, 1));
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v132, _v133}, 4)}, 1));
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v132, _v133}, 4)}, 1));
            _v138 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v132, _v133}, 2);
            _v139 = px_int(0LL);
            while (px_is_truthy(px_lt(_v139, px_call(px_get_global("len"), (LXValue[]){_v138}, 1)))) {
                (void)(px_method(_v135, "append", (LXValue[]){px_index(_v138, _v139)}, 1));
                 _v139 = px_add(_v139, px_int(1LL));
            }
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v132, _v133}, 4)}, 1));
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v132, _v133}, 4)}, 1));
             _v136 = px_str("");
        }
        else {
             _v136 = px_add(_v136, _v137);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v136}, 1), _v132, _v133}, 4)}, 1));
    return _v135;
px_err_140:
    if (px_err_140_proped) return px_err_140_val;
    return px_null();
}

static LXValue fn_scan_interp_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v145 = (nargs > 0) ? args[0] : px_null();
    LXValue _v146 = (nargs > 1) ? args[1] : px_null();
    LXValue _v147 = px_null();
    LXValue _v148 = px_null();
    LXValue _v149 = px_null();
    LXValue _v150 = px_null();
    LXValue px_err_151_val = px_null();
    int px_err_151_proped = 0;
    _v147 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        _v148 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v148, px_str("")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值 ${ 未闭合（缺少 }）"), _v145, _v146}, 4));
        }
        if (px_is_truthy(px_eq(_v148, px_str("}")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            break;
        }
        if (px_is_truthy(px_eq(_v148, px_str("{")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持 {} 字面量（dict/set），请先用变量保存")}, 2));
        }
        if (px_is_truthy(px_eq(_v148, px_str("\n")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值表达式不能跨行"), _v145, _v146}, 4));
        }
        if (px_is_truthy(px_eq(_v148, px_str("#")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持注释")}, 2));
        }
        if (px_is_truthy(({ LXValue _t152 = px_eq(_v148, px_str(" ")); px_is_truthy(_t152) ? _t152 : px_eq(_v148, px_str("\t")); }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        else if (px_is_truthy(({ LXValue _t153 = px_eq(_v148, px_str("\"")); px_is_truthy(_t153) ? _t153 : px_eq(_v148, px_str("'")); }))) {
            _v149 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){px_bool(true)}, 1);
            _v150 = px_int(0LL);
            while (px_is_truthy(px_lt(_v150, px_call(px_get_global("len"), (LXValue[]){_v149}, 1)))) {
                (void)(px_method(_v147, "append", (LXValue[]){px_index(_v149, _v150)}, 1));
                 _v150 = px_add(_v150, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v148}, 1))) {
            (void)(px_method(_v147, "append", (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v148}, 1))) {
            (void)(px_method(_v147, "append", (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        }
        else {
            (void)(px_method(_v147, "append", (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
        }
    }
    return _v147;
px_err_151:
    if (px_err_151_proped) return px_err_151_val;
    return px_null();
}

static LXValue fn_hex_to_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v154 = (nargs > 0) ? args[0] : px_null();
    LXValue _v155 = px_null();
    LXValue _v156 = px_null();
    LXValue _v157 = px_null();
    LXValue _v158 = px_null();
    LXValue _v159 = px_null();
    LXValue px_err_160_val = px_null();
    int px_err_160_proped = 0;
    _v155 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v154}, 1);
    if (px_is_truthy(px_le(_v155, px_int(127LL)))) {
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){_v155, px_int(1LL)}, 2)}, 1);
    }
    if (px_is_truthy(px_le(_v155, px_int(2047LL)))) {
        _v156 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(192LL), px_shr(_v155, px_int(6LL))), px_int(1LL)}, 2);
        _v157 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v155, px_int(63LL))), px_int(1LL)}, 2);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v156, _v157}, 2)}, 1);
    }
    if (px_is_truthy(px_le(_v155, px_int(65535LL)))) {
        _v156 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(224LL), px_shr(_v155, px_int(12LL))), px_int(1LL)}, 2);
        _v157 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v155, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
        _v158 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v155, px_int(63LL))), px_int(1LL)}, 2);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v156, _v157, _v158}, 3)}, 1);
    }
    _v156 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(240LL), px_shr(_v155, px_int(18LL))), px_int(1LL)}, 2);
    _v157 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v155, px_int(12LL)), px_int(63LL))), px_int(1LL)}, 2);
    _v158 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v155, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
    _v159 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v155, px_int(63LL))), px_int(1LL)}, 2);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v156, _v157, _v158, _v159}, 4)}, 1);
px_err_160:
    if (px_err_160_proped) return px_err_160_val;
    return px_null();
}

static LXValue fn_scan_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v161 = (nargs > 0) ? args[0] : px_null();
    LXValue _v162 = (nargs > 1) ? args[1] : px_null();
    LXValue _v163 = px_null();
    LXValue _v164 = px_null();
    LXValue _v165 = px_null();
    LXValue _v166 = px_null();
    LXValue px_err_167_val = px_null();
    int px_err_167_proped = 0;
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    _v163 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v163, px_str("")))) {
        (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串在转义序列处意外结束"), _v161, _v162}, 4));
    }
    if (px_is_truthy(px_eq(_v163, px_str("n")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\n");
    }
    if (px_is_truthy(px_eq(_v163, px_str("t")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\t");
    }
    if (px_is_truthy(px_eq(_v163, px_str("r")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\r");
    }
    if (px_is_truthy(px_eq(_v163, px_str("\\")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\\");
    }
    if (px_is_truthy(px_eq(_v163, px_str("\"")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\"");
    }
    if (px_is_truthy(px_eq(_v163, px_str("'")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("'");
    }
    if (px_is_truthy(px_eq(_v163, px_str("0")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("");
    }
    if (px_is_truthy(px_eq(_v163, px_str("$")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("$");
    }
    if (px_is_truthy(px_eq(_v163, px_str("u")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("{")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义须为 \\u{XXXX} 形式")}, 2));
        }
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        _v164 = px_str("");
        while (px_is_truthy(({ LXValue _t168 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t168) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")) : _t168; }))) {
            _v165 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            if (px_is_truthy(px_call(px_get_global("is_hex_digit"), (LXValue[]){_v165}, 1))) {
                 _v164 = px_add(_v164, _v165);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            else {
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义含非法字符")}, 2));
            }
        }
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义缺少 }")}, 2));
        }
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        if (px_is_truthy(px_eq(_v164, px_str("")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        _v166 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v164}, 1);
        if (px_is_truthy(({ LXValue _t169 = px_eq(_v166, px_null()); px_is_truthy(_t169) ? _t169 : px_gt(_v166, px_int(4294967295LL)); }))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        if (px_is_truthy(({ LXValue _t171 = px_gt(_v166, px_int(1114111LL)); px_is_truthy(_t171) ? _t171 : ({ LXValue _t170 = px_ge(_v166, px_int(55296LL)); px_is_truthy(_t170) ? px_le(_v166, px_int(57343LL)) : _t170; }); }))) {
            return px_call(px_get_global("hex_to_char"), (LXValue[]){px_str("FFFD")}, 1);
        }
        return px_call(px_get_global("hex_to_char"), (LXValue[]){_v164}, 1);
    }
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_add(px_str("非法转义序列 \\"), _v163)}, 2));
    return px_str("");
px_err_167:
    if (px_err_167_proped) return px_err_167_val;
    return px_null();
}

static LXValue fn_int_to_hex_nopad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v172 = (nargs > 0) ? args[0] : px_null();
    LXValue _v173 = px_null();
    LXValue _v174 = px_null();
    LXValue px_err_175_val = px_null();
    int px_err_175_proped = 0;
    _v173 = px_str("0123456789abcdef");
    _v174 = px_str("");
    while (px_is_truthy(px_gt(_v172, px_int(0LL)))) {
         _v174 = px_add(px_index(_v173, px_mod(_v172, px_int(16LL))), _v174);
         _v172 = px_idiv(_v172, px_int(16LL));
    }
    if (px_is_truthy(px_eq(_v174, px_str("")))) {
        return px_str("0");
    }
    return _v174;
px_err_175:
    if (px_err_175_proped) return px_err_175_val;
    return px_null();
}

static LXValue fn_char_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v176 = (nargs > 0) ? args[0] : px_null();
    LXValue _v177 = px_null();
    LXValue px_err_178_val = px_null();
    int px_err_178_proped = 0;
    if (px_is_truthy(px_eq(_v176, px_str("'")))) {
        return px_str("'\\''");
    }
    if (px_is_truthy(px_eq(_v176, px_str("\\")))) {
        return px_str("'\\\\'");
    }
    if (px_is_truthy(px_eq(_v176, px_str("\n")))) {
        return px_str("'\\n'");
    }
    if (px_is_truthy(px_eq(_v176, px_str("\r")))) {
        return px_str("'\\r'");
    }
    if (px_is_truthy(px_eq(_v176, px_str("\t")))) {
        return px_str("'\\t'");
    }
    if (px_is_truthy(px_eq(_v176, px_str("")))) {
        return px_str("'\\0'");
    }
    _v177 = px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v176}, 1);
    if (px_is_truthy(px_ge(_v177, px_int(0LL)))) {
        return px_add(px_add(px_str("'\\u{"), px_call(px_get_global("int_to_hex_nopad"), (LXValue[]){_v177}, 1)), px_str("}'"));
    }
    return px_add(px_add(px_str("'"), _v176), px_str("'"));
px_err_178:
    if (px_err_178_proped) return px_err_178_val;
    return px_null();
}

static LXValue fn_rust_str_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v179 = (nargs > 0) ? args[0] : px_null();
    LXValue _v180 = px_null();
    LXValue _v181 = px_null();
    LXValue _v182 = px_null();
    LXValue px_err_183_val = px_null();
    int px_err_183_proped = 0;
    _v180 = px_str("\"");
    _v181 = px_int(0LL);
    while (px_is_truthy(px_lt(_v181, px_call(px_get_global("len"), (LXValue[]){_v179}, 1)))) {
        _v182 = px_index(_v179, _v181);
        if (px_is_truthy(px_eq(_v182, px_str("\n")))) {
             _v180 = px_add(_v180, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\t")))) {
             _v180 = px_add(_v180, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\r")))) {
             _v180 = px_add(_v180, px_str("\\r"));
        }
        else if (px_is_truthy(px_lt(_v182, px_str("")))) {
             _v180 = px_add(_v180, px_str("\\0"));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\"")))) {
             _v180 = px_add(_v180, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\\")))) {
             _v180 = px_add(_v180, px_str("\\\\"));
        }
        else if (px_is_truthy(({ LXValue _t184 = px_ge(_v182, px_str(" ")); px_is_truthy(_t184) ? px_le(_v182, px_str("~")) : _t184; }))) {
             _v180 = px_add(_v180, _v182);
        }
        else if (px_is_truthy(px_eq(_v182, px_str(" ")))) {
             _v180 = px_add(_v180, px_str("\\u{a0}"));
        }
        else if (px_is_truthy(px_gt(_v182, px_str(" ")))) {
             _v180 = px_add(_v180, _v182);
        }
        else {
             _v180 = px_add(_v180, px_add(px_add(px_str("\\u{"), px_call(px_get_global("ctrl_hex"), (LXValue[]){_v182}, 1)), px_str("}")));
        }
         _v181 = px_add(_v181, px_int(1LL));
    }
     _v180 = px_add(_v180, px_str("\""));
    return _v180;
px_err_183:
    if (px_err_183_proped) return px_err_183_val;
    return px_null();
}

static LXValue fn_ctrl_codepoint(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v185 = (nargs > 0) ? args[0] : px_null();
    LXValue _v186 = px_null();
    LXValue px_err_187_val = px_null();
    int px_err_187_proped = 0;
    _v186 = px_int(0LL);
    while (px_is_truthy(px_lt(_v186, px_call(px_get_global("len"), (LXValue[]){px_get_global("CTRL_ALL")}, 1)))) {
        if (px_is_truthy(px_eq(px_index(px_get_global("CTRL_ALL"), _v186), _v185))) {
            if (px_is_truthy(px_lt(_v186, px_int(28LL)))) {
                if (px_is_truthy(px_lt(_v186, px_int(8LL)))) {
                    return px_add(_v186, px_int(1LL));
                }
                if (px_is_truthy(px_lt(_v186, px_int(10LL)))) {
                    return px_add(_v186, px_int(3LL));
                }
                return px_add(_v186, px_int(4LL));
            }
            return px_add(_v186, px_int(99LL));
        }
         _v186 = px_add(_v186, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_187:
    if (px_err_187_proped) return px_err_187_val;
    return px_null();
}

static LXValue fn_ctrl_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v188 = (nargs > 0) ? args[0] : px_null();
    LXValue _v189 = px_null();
    LXValue _v190 = px_null();
    LXValue px_err_191_val = px_null();
    int px_err_191_proped = 0;
    _v189 = px_call(px_get_global("int_to_hex"), (LXValue[]){px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v188}, 1), px_int(16LL)}, 2);
    _v190 = px_int(0LL);
    while (px_is_truthy(({ LXValue _t192 = px_lt(_v190, px_call(px_get_global("len"), (LXValue[]){_v189}, 1)); px_is_truthy(_t192) ? px_eq(px_index(_v189, _v190), px_str("0")) : _t192; }))) {
         _v190 = px_add(_v190, px_int(1LL));
    }
    if (px_is_truthy(px_eq(_v190, px_call(px_get_global("len"), (LXValue[]){_v189}, 1)))) {
        return px_str("0");
    }
    return px_slice(_v189, _v190, px_call(px_get_global("len"), (LXValue[]){_v189}, 1), px_null());
px_err_191:
    if (px_err_191_proped) return px_err_191_val;
    return px_null();
}

static LXValue fn_scan_operator_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v193 = px_null();
    LXValue _v194 = px_null();
    LXValue _v195 = px_null();
    LXValue px_err_196_val = px_null();
    int px_err_196_proped = 0;
    _v193 = px_get_global("g_line");
    _v194 = px_get_global("g_col");
    _v195 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v195, px_str("(")))) {
        return px_list_n((LXValue[]){px_str("("), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str(")")))) {
        return px_list_n((LXValue[]){px_str(")"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("[")))) {
        return px_list_n((LXValue[]){px_str("["), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("]")))) {
        return px_list_n((LXValue[]){px_str("]"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("{")))) {
        return px_list_n((LXValue[]){px_str("{"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("}")))) {
        return px_list_n((LXValue[]){px_str("}"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str(",")))) {
        return px_list_n((LXValue[]){px_str(","), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str(":")))) {
        return px_list_n((LXValue[]){px_str(":"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str(".")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_str("运算符 '..' 未定义（range 语法尚未支持）")}, 2));
        }
        return px_list_n((LXValue[]){px_str("."), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("+")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("+="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("+"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("-")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("->"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("-="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("-"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("*")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("*")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str("**="), px_str(""), _v193, _v194}, 4);
            }
            return px_list_n((LXValue[]){px_str("**"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("*="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("*"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("/")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("/")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str("//="), px_str(""), _v193, _v194}, 4);
            }
            return px_list_n((LXValue[]){px_str("//"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("/="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("/"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("%")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("%="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("%"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("^")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("^="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("^"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("~")))) {
        return px_list_n((LXValue[]){px_str("~"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("&")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("&="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("&"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("|")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("|>"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("|="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("|"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("=")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("=="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("="), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("!")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("!="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("!"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("<")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("<-"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("<")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str("<<="), px_str(""), _v193, _v194}, 4);
            }
            return px_list_n((LXValue[]){px_str("<<"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("<="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("<"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str(">")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                    return px_list_n((LXValue[]){px_str(">>>="), px_str(""), _v193, _v194}, 4);
                }
                return px_list_n((LXValue[]){px_str(">>>"), px_str(""), _v193, _v194}, 4);
            }
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str(">>="), px_str(""), _v193, _v194}, 4);
            }
            return px_list_n((LXValue[]){px_str(">>"), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str(">="), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str(">"), px_str(""), _v193, _v194}, 4);
    }
    if (px_is_truthy(px_eq(_v195, px_str("?")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("?."), px_str(""), _v193, _v194}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("?")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("??"), px_str(""), _v193, _v194}, 4);
        }
        return px_list_n((LXValue[]){px_str("?"), px_str(""), _v193, _v194}, 4);
    }
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_add(px_str("非法字符: "), px_call(px_get_global("char_debug"), (LXValue[]){_v195}, 1))}, 2));
    return px_list_n((LXValue[]){px_str(""), px_str(""), _v193, _v194}, 4);
px_err_196:
    if (px_err_196_proped) return px_err_196_val;
    return px_null();
}

static LXValue fn_next_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v197 = px_null();
    LXValue _v198 = px_null();
    LXValue _v199 = px_null();
    LXValue _v200 = px_null();
    LXValue px_err_201_val = px_null();
    int px_err_201_proped = 0;
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_int(0LL)))) {
        _v197 = px_index(px_get_global("g_pending"), px_int(0LL));
        px_set_global("g_pending", px_slice(px_get_global("g_pending"), px_int(1LL), px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_null()));
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){_v197}, 1));
        return px_bool(true);
    }
    if (px_is_truthy(px_get_global("g_at_line_start"))) {
        (void)(px_call(px_get_global("handle_line_start"), (LXValue[]){}, 0));
    }
    _v198 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v198, px_str("")))) {
        while (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)))) {
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("EOF"), px_str("")}, 2));
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(_v198, px_str("\n")))) {
        _v199 = px_get_global("g_line");
        _v200 = px_get_global("g_col");
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_set_global("g_at_line_start", px_bool(true));
        (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_str("换行"), px_str(""), _v199, _v200}, 4));
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t202 = px_eq(_v198, px_str(" ")); px_is_truthy(_t202) ? _t202 : px_eq(_v198, px_str("\t")); }))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_bool(true);
    }
    if (px_is_truthy(px_eq(_v198, px_str("#")))) {
        (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t203 = px_eq(_v198, px_str("\"")); px_is_truthy(_t203) ? _t203 : px_eq(_v198, px_str("'")); }))) {
        (void)(px_call(px_get_global("scan_string"), (LXValue[]){px_bool(true)}, 1));
        return px_bool(true);
    }
    if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v198}, 1))) {
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        return px_bool(true);
    }
    if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v198}, 1))) {
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        return px_bool(true);
    }
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
    return px_bool(true);
px_err_201:
    if (px_err_201_proped) return px_err_201_val;
    return px_null();
}

static LXValue fn_check_edition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v204 = (nargs > 0) ? args[0] : px_null();
    LXValue _v205 = px_null();
    LXValue _v206 = px_null();
    LXValue _v207 = px_null();
    LXValue _v208 = px_null();
    LXValue _v209 = px_null();
    LXValue _v210 = px_null();
    LXValue _v211 = px_null();
    LXValue px_err_212_val = px_null();
    int px_err_212_proped = 0;
    _v205 = px_int(0LL);
    _v206 = px_call(px_get_global("len"), (LXValue[]){_v204}, 1);
    _v207 = px_str("");
    while (px_is_truthy(({ LXValue _t213 = px_lt(_v205, _v206); px_is_truthy(_t213) ? px_ne(px_index(_v204, _v205), px_str("\n")) : _t213; }))) {
         _v207 = px_add(_v207, px_index(_v204, _v205));
         _v205 = px_add(_v205, px_int(1LL));
    }
    _v208 = px_call(px_get_global("trim"), (LXValue[]){_v207}, 1);
    _v209 = px_method(_v208, "split", (LXValue[]){px_str(" ")}, 1);
    if (px_is_truthy(({ LXValue _t215 = ({ LXValue _t214 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v209}, 1), px_int(3LL)); px_is_truthy(_t214) ? px_eq(px_index(_v209, px_int(0LL)), px_str("#")) : _t214; }); px_is_truthy(_t215) ? px_eq(px_index(_v209, px_int(1LL)), px_str("px")) : _t215; }))) {
        _v210 = px_index(_v209, px_int(2LL));
        if (px_is_truthy(({ LXValue _t219 = ({ LXValue _t218 = ({ LXValue _t217 = ({ LXValue _t216 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v210}, 1), px_int(4LL)); px_is_truthy(_t216) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(0LL))}, 1) : _t216; }); px_is_truthy(_t217) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(1LL))}, 1) : _t217; }); px_is_truthy(_t218) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(2LL))}, 1) : _t218; }); px_is_truthy(_t219) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(3LL))}, 1) : _t219; }))) {
            _v211 = px_call(px_get_global("int"), (LXValue[]){_v210}, 1);
            if (px_is_truthy(px_gt(_v211, px_int(2026LL)))) {
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E-EDITION"), px_add(px_add(px_str("源码声明 edition px "), _v210), px_str(" 高于当前工具链支持（px 2026），请升级编译器"))}, 2));
            }
        }
    }
px_err_212:
    if (px_err_212_proped) return px_err_212_val;
    return px_null();
}

static LXValue fn_lex_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v220 = (nargs > 0) ? args[0] : px_null();
    LXValue _v221 = px_null();
    LXValue px_err_222_val = px_null();
    int px_err_222_proped = 0;
    (void)(px_call(px_get_global("check_edition"), (LXValue[]){_v220}, 1));
    px_set_global("g_src", _v220);
    px_set_global("g_len", px_call(px_get_global("len"), (LXValue[]){_v220}, 1));
    px_set_global("g_pos", px_int(0LL));
    px_set_global("g_line", px_int(1LL));
    px_set_global("g_col", px_int(1LL));
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_set_global("g_at_line_start", px_bool(true));
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_set_global("g_count", px_int(0LL));
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    _v221 = px_bool(true);
    while (px_is_truthy(_v221)) {
         _v221 = px_call(px_get_global("next_token"), (LXValue[]){}, 0);
    }
    return px_get_global("g_toks");
px_err_222:
    if (px_err_222_proped) return px_err_222_val;
    return px_null();
}

static LXValue fn_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v223 = (nargs > 0) ? args[0] : px_null();
    LXValue _v224 = px_null();
    LXValue _v225 = px_null();
    LXValue px_err_226_val = px_null();
    int px_err_226_proped = 0;
    _v224 = px_str("");
    _v225 = px_int(0LL);
    while (px_is_truthy(px_lt(_v225, _v223))) {
         _v224 = px_add(_v224, px_str(" "));
         _v225 = px_add(_v225, px_int(1LL));
    }
    return _v224;
px_err_226:
    if (px_err_226_proped) return px_err_226_val;
    return px_null();
}

static LXValue fn_dump_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v227 = (nargs > 0) ? args[0] : px_null();
    LXValue _v228 = (nargs > 1) ? args[1] : px_null();
    LXValue _v229 = px_null();
    LXValue _v230 = px_null();
    LXValue _v231 = px_null();
    LXValue _v232 = px_null();
    LXValue _v233 = px_null();
    LXValue _v234 = px_null();
    LXValue _v235 = px_null();
    LXValue _v236 = px_null();
    LXValue _v237 = px_null();
    LXValue _v238 = px_null();
    LXValue px_err_239_val = px_null();
    int px_err_239_proped = 0;
    _v229 = px_index(_v227, px_int(0LL));
    _v230 = px_index(px_get_global("LAYOUT"), _v229);
    _v231 = px_index(_v230, px_int(0LL));
    _v232 = px_index(_v230, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v232}, 1), px_int(0LL)))) {
        return _v231;
    }
    _v233 = px_eq(px_index(px_index(_v232, px_int(0LL)), px_int(0LL)), px_null());
    _v234 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(_v233)) {
        (void)(px_method(_v234, "append", (LXValue[]){px_add(_v231, px_str("("))}, 1));
    }
    else {
        (void)(px_method(_v234, "append", (LXValue[]){px_add(_v231, px_str(" {"))}, 1));
    }
    _v235 = px_int(0LL);
    while (px_is_truthy(px_lt(_v235, px_call(px_get_global("len"), (LXValue[]){_v232}, 1)))) {
        _v236 = px_index(_v232, _v235);
        _v237 = px_index(_v227, px_add(_v235, px_int(1LL)));
        _v238 = px_call(px_get_global("dump_field"), (LXValue[]){_v237, px_index(_v236, px_int(1LL)), px_add(_v228, px_int(4LL))}, 3);
        if (px_is_truthy(_v233)) {
            (void)(px_method(_v234, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v228, px_int(4LL))}, 1), _v238), px_str(","))}, 1));
        }
        else {
            (void)(px_method(_v234, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v228, px_int(4LL))}, 1), px_index(_v236, px_int(0LL))), px_str(": ")), _v238), px_str(","))}, 1));
        }
         _v235 = px_add(_v235, px_int(1LL));
    }
    if (px_is_truthy(_v233)) {
        (void)(px_method(_v234, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v228}, 1), px_str(")"))}, 1));
    }
    else {
        (void)(px_method(_v234, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v228}, 1), px_str("}"))}, 1));
    }
    return px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v234}, 2);
px_err_239:
    if (px_err_239_proped) return px_err_239_val;
    return px_null();
}

static LXValue fn_dump_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v240 = (nargs > 0) ? args[0] : px_null();
    LXValue _v241 = (nargs > 1) ? args[1] : px_null();
    LXValue _v242 = px_null();
    LXValue _v243 = px_null();
    LXValue px_err_244_val = px_null();
    int px_err_244_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v240}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v242 = px_list_n((LXValue[]){}, 0);
    _v243 = px_int(0LL);
    while (px_is_truthy(px_lt(_v243, px_call(px_get_global("len"), (LXValue[]){_v240}, 1)))) {
        (void)(px_method(_v242, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v241, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v240, _v243), px_add(_v241, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v243 = px_add(_v243, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v242}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v241}, 1)), px_str("]"));
px_err_244:
    if (px_err_244_proped) return px_err_244_val;
    return px_null();
}

static LXValue fn_dump_str_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v245 = (nargs > 0) ? args[0] : px_null();
    LXValue _v246 = (nargs > 1) ? args[1] : px_null();
    LXValue _v247 = px_null();
    LXValue _v248 = px_null();
    LXValue px_err_249_val = px_null();
    int px_err_249_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v245}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v247 = px_list_n((LXValue[]){}, 0);
    _v248 = px_int(0LL);
    while (px_is_truthy(px_lt(_v248, px_call(px_get_global("len"), (LXValue[]){_v245}, 1)))) {
        (void)(px_method(_v247, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v246, px_int(4LL))}, 1), px_index(_v245, _v248)), px_str(","))}, 1));
         _v248 = px_add(_v248, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v247}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v246}, 1)), px_str("]"));
px_err_249:
    if (px_err_249_proped) return px_err_249_val;
    return px_null();
}

static LXValue fn_dump_ty_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v250 = (nargs > 0) ? args[0] : px_null();
    LXValue _v251 = (nargs > 1) ? args[1] : px_null();
    LXValue _v252 = px_null();
    LXValue _v253 = px_null();
    LXValue px_err_254_val = px_null();
    int px_err_254_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v250}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v252 = px_list_n((LXValue[]){}, 0);
    _v253 = px_int(0LL);
    while (px_is_truthy(px_lt(_v253, px_call(px_get_global("len"), (LXValue[]){_v250}, 1)))) {
        (void)(px_method(_v252, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v251, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v250, _v253), px_add(_v251, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v253 = px_add(_v253, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v252}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v251}, 1)), px_str("]"));
px_err_254:
    if (px_err_254_proped) return px_err_254_val;
    return px_null();
}

static LXValue fn_dump_pat_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v255 = (nargs > 0) ? args[0] : px_null();
    LXValue _v256 = (nargs > 1) ? args[1] : px_null();
    LXValue _v257 = px_null();
    LXValue _v258 = px_null();
    LXValue px_err_259_val = px_null();
    int px_err_259_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v255}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v257 = px_list_n((LXValue[]){}, 0);
    _v258 = px_int(0LL);
    while (px_is_truthy(px_lt(_v258, px_call(px_get_global("len"), (LXValue[]){_v255}, 1)))) {
        (void)(px_method(_v257, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v256, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v255, _v258), px_add(_v256, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v258 = px_add(_v258, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v257}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v256}, 1)), px_str("]"));
px_err_259:
    if (px_err_259_proped) return px_err_259_val;
    return px_null();
}

static LXValue fn_dump_opt_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v260 = (nargs > 0) ? args[0] : px_null();
    LXValue _v261 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_262_val = px_null();
    int px_err_262_proped = 0;
    if (px_is_truthy(px_eq(_v260, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v261, px_int(4LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){_v260, px_add(_v261, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v261}, 1)), px_str(")"));
px_err_262:
    if (px_err_262_proped) return px_err_262_val;
    return px_null();
}

static LXValue fn_dump_opt_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v263 = (nargs > 0) ? args[0] : px_null();
    LXValue _v264 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_265_val = px_null();
    int px_err_265_proped = 0;
    if (px_is_truthy(px_eq(_v263, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v264, px_int(4LL))}, 1)), _v263), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v264}, 1)), px_str(")"));
px_err_265:
    if (px_err_265_proped) return px_err_265_val;
    return px_null();
}

static LXValue fn_dump_opt_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v266 = (nargs > 0) ? args[0] : px_null();
    LXValue _v267 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_268_val = px_null();
    int px_err_268_proped = 0;
    if (px_is_truthy(px_eq(_v266, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v267, px_int(4LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){_v266, px_add(_v267, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v267}, 1)), px_str(")"));
px_err_268:
    if (px_err_268_proped) return px_err_268_val;
    return px_null();
}

static LXValue fn_dump_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v269 = (nargs > 0) ? args[0] : px_null();
    LXValue _v270 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_271_val = px_null();
    int px_err_271_proped = 0;
    return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v270, px_int(4LL))}, 1)), px_str("line: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v269, px_int(0LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v270, px_int(4LL))}, 1)), px_str("col: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v269, px_int(1LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v270}, 1)), px_str("}"));
px_err_271:
    if (px_err_271_proped) return px_err_271_val;
    return px_null();
}

static LXValue fn_dump_t2_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v272 = (nargs > 0) ? args[0] : px_null();
    LXValue _v273 = (nargs > 1) ? args[1] : px_null();
    LXValue _v274 = px_null();
    LXValue _v275 = px_null();
    LXValue _v276 = px_null();
    LXValue px_err_277_val = px_null();
    int px_err_277_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v272}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v274 = px_list_n((LXValue[]){}, 0);
    _v275 = px_int(0LL);
    while (px_is_truthy(px_lt(_v275, px_call(px_get_global("len"), (LXValue[]){_v272}, 1)))) {
        _v276 = px_index(_v272, _v275);
        (void)(px_method(_v274, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v276, px_int(0LL)), px_add(_v273, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v276, px_int(1LL)), px_add(_v273, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v275 = px_add(_v275, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v274}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v273}, 1)), px_str("]"));
px_err_277:
    if (px_err_277_proped) return px_err_277_val;
    return px_null();
}

static LXValue fn_dump_t2b_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v278 = (nargs > 0) ? args[0] : px_null();
    LXValue _v279 = (nargs > 1) ? args[1] : px_null();
    LXValue _v280 = px_null();
    LXValue _v281 = px_null();
    LXValue _v282 = px_null();
    LXValue px_err_283_val = px_null();
    int px_err_283_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v278}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v280 = px_list_n((LXValue[]){}, 0);
    _v281 = px_int(0LL);
    while (px_is_truthy(px_lt(_v281, px_call(px_get_global("len"), (LXValue[]){_v278}, 1)))) {
        _v282 = px_index(_v278, _v281);
        (void)(px_method(_v280, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v282, px_int(0LL)), px_add(_v279, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v282, px_int(1LL)), px_add(_v279, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v281 = px_add(_v281, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v280}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v279}, 1)), px_str("]"));
px_err_283:
    if (px_err_283_proped) return px_err_283_val;
    return px_null();
}

static LXValue fn_dump_t3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v284 = (nargs > 0) ? args[0] : px_null();
    LXValue _v285 = (nargs > 1) ? args[1] : px_null();
    LXValue _v286 = px_null();
    LXValue _v287 = px_null();
    LXValue _v288 = px_null();
    LXValue _v289 = px_null();
    LXValue _v290 = px_null();
    LXValue px_err_291_val = px_null();
    int px_err_291_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v284}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    _v286 = px_list_n((LXValue[]){}, 0);
    _v287 = px_int(0LL);
    while (px_is_truthy(px_lt(_v287, px_call(px_get_global("len"), (LXValue[]){_v284}, 1)))) {
        _v288 = px_index(_v284, _v287);
        _v289 = px_index(_v288, px_int(0LL));
        _v290 = px_str("None");
        if (px_is_truthy(px_ne(_v289, px_null()))) {
             _v290 = px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(12LL))}, 1)), _v289), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), px_str(")"));
        }
        (void)(px_method(_v286, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), _v290), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v288, px_int(1LL)), px_add(_v285, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v288, px_int(2LL)), px_add(_v285, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v287 = px_add(_v287, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v286}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v285}, 1)), px_str("]"));
px_err_291:
    if (px_err_291_proped) return px_err_291_val;
    return px_null();
}

static LXValue fn_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v292 = (nargs > 0) ? args[0] : px_null();
    LXValue _v293 = px_null();
    LXValue px_err_294_val = px_null();
    int px_err_294_proped = 0;
    _v293 = px_call(px_get_global("str"), (LXValue[]){_v292}, 1);
    if (px_is_truthy(({ LXValue _t295 = px_eq(_v293, px_str("inf")); px_is_truthy(_t295) ? _t295 : px_eq(_v293, px_str("-inf")); }))) {
        return _v293;
    }
    if (px_is_truthy(({ LXValue _t297 = ({ LXValue _t296 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v293, px_str(".")}, 2)); px_is_truthy(_t296) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v293, px_str("e")}, 2)) : _t296; }); px_is_truthy(_t297) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v293, px_str("E")}, 2)) : _t297; }))) {
        return px_add(_v293, px_str(".0"));
    }
    return _v293;
px_err_294:
    if (px_err_294_proped) return px_err_294_val;
    return px_null();
}

static LXValue fn_dump_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v298 = (nargs > 0) ? args[0] : px_null();
    LXValue _v299 = (nargs > 1) ? args[1] : px_null();
    LXValue _v300 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_301_val = px_null();
    int px_err_301_proped = 0;
    if (px_is_truthy(px_eq(_v299, px_str("s")))) {
        return _v298;
    }
    if (px_is_truthy(px_eq(_v299, px_str("r")))) {
        return px_call(px_get_global("str"), (LXValue[]){_v298}, 1);
    }
    if (px_is_truthy(px_eq(_v299, px_str("f")))) {
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v298}, 1);
    }
    if (px_is_truthy(px_eq(_v299, px_str("n")))) {
        return px_call(px_get_global("dump_node"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("o")))) {
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("os")))) {
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("ol")))) {
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("l")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("ls")))) {
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("tl")))) {
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lpl")))) {
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lp")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lsf")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lev")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lfd")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("ltci")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lc")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lma")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lt2")))) {
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lt2b")))) {
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("lt3")))) {
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v298, _v300}, 2);
    }
    if (px_is_truthy(px_eq(_v299, px_str("p")))) {
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v298, _v300}, 2);
    }
    return px_call(px_get_global("str"), (LXValue[]){_v298}, 1);
px_err_301:
    if (px_err_301_proped) return px_err_301_val;
    return px_null();
}

static LXValue fn_dump_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v302 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_303_val = px_null();
    int px_err_303_proped = 0;
    return px_call(px_get_global("dump_node"), (LXValue[]){_v302, px_int(0LL)}, 2);
px_err_303:
    if (px_err_303_proped) return px_err_303_val;
    return px_null();
}

static LXValue fn_pk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_304_val = px_null();
    int px_err_304_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
px_err_304:
    if (px_err_304_proped) return px_err_304_val;
    return px_null();
}

static LXValue fn_pk_display(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v305 = px_null();
    LXValue _v306 = px_null();
    LXValue px_err_307_val = px_null();
    int px_err_307_proped = 0;
    _v305 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
    _v306 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
    if (px_is_truthy(px_eq(_v305, px_str("整数")))) {
        return px_add(px_str("整数 "), _v306);
    }
    if (px_is_truthy(px_eq(_v305, px_str("浮点")))) {
        return px_add(px_str("浮点 "), _v306);
    }
    if (px_is_truthy(px_eq(_v305, px_str("字符串")))) {
        return px_add(px_str("字符串 "), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v306}, 1));
    }
    if (px_is_truthy(px_eq(_v305, px_str("标识符")))) {
        return px_add(px_str("标识符 "), _v306);
    }
    if (px_is_truthy(px_eq(_v305, px_str("注释")))) {
        return px_add(px_str("注释 "), _v306);
    }
    return _v305;
px_err_307:
    if (px_err_307_proped) return px_err_307_val;
    return px_null();
}

static LXValue fn_pv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_308_val = px_null();
    int px_err_308_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
px_err_308:
    if (px_err_308_proped) return px_err_308_val;
    return px_null();
}

static LXValue fn_pline(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_309_val = px_null();
    int px_err_309_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL));
px_err_309:
    if (px_err_309_proped) return px_err_309_val;
    return px_null();
}

static LXValue fn_pcol(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_310_val = px_null();
    int px_err_310_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL));
px_err_310:
    if (px_err_310_proped) return px_err_310_val;
    return px_null();
}

static LXValue fn_ppos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_311_val = px_null();
    int px_err_311_proped = 0;
    return px_list_n((LXValue[]){px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL)), px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL))}, 2);
px_err_311:
    if (px_err_311_proped) return px_err_311_val;
    return px_null();
}

static LXValue fn_adv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v312 = px_null();
    LXValue px_err_313_val = px_null();
    int px_err_313_proped = 0;
    _v312 = px_index(px_get_global("p_toks"), px_get_global("p_pos"));
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_set_global("p_pos", px_add(px_get_global("p_pos"), px_int(1LL)));
    }
    return _v312;
px_err_313:
    if (px_err_313_proped) return px_err_313_val;
    return px_null();
}

static LXValue fn_chk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v314 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_315_val = px_null();
    int px_err_315_proped = 0;
    return px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), _v314);
px_err_315:
    if (px_err_315_proped) return px_err_315_val;
    return px_null();
}

static LXValue fn_chk2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v316 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_317_val = px_null();
    int px_err_317_proped = 0;
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(1LL))), px_int(0LL)), _v316);
    }
    return px_bool(false);
px_err_317:
    if (px_err_317_proped) return px_err_317_val;
    return px_null();
}

static LXValue fn_chk3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v318 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_319_val = px_null();
    int px_err_319_proped = 0;
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(2LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(2LL))), px_int(0LL)), _v318);
    }
    return px_bool(false);
px_err_319:
    if (px_err_319_proped) return px_err_319_val;
    return px_null();
}

static LXValue fn_expect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v320 = (nargs > 0) ? args[0] : px_null();
    LXValue _v321 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_322_val = px_null();
    int px_err_322_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v320}, 1))) {
        return px_call(px_get_global("adv"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望 "), _v321), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_322:
    if (px_err_322_proped) return px_err_322_val;
    return px_null();
}

static LXValue fn_expect_ident(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v323 = (nargs > 0) ? args[0] : px_null();
    LXValue _v324 = px_null();
    LXValue px_err_325_val = px_null();
    int px_err_325_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        _v324 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v324;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v323), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_325:
    if (px_err_325_proped) return px_err_325_val;
    return px_null();
}

static LXValue fn_is_name_kind(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v326 = (nargs > 0) ? args[0] : px_null();
    LXValue _v327 = px_null();
    LXValue _v328 = px_null();
    LXValue px_err_329_val = px_null();
    int px_err_329_proped = 0;
    _v327 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture"), px_str("extern")}, 36);
    _v328 = px_int(0LL);
    while (px_is_truthy(px_lt(_v328, px_call(px_get_global("len"), (LXValue[]){_v327}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v327, _v328), _v326))) {
            return px_bool(true);
        }
         _v328 = px_add(_v328, px_int(1LL));
    }
    return px_bool(false);
px_err_329:
    if (px_err_329_proped) return px_err_329_val;
    return px_null();
}

static LXValue fn_expect_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v330 = (nargs > 0) ? args[0] : px_null();
    LXValue _v331 = px_null();
    LXValue px_err_332_val = px_null();
    int px_err_332_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        _v331 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v331;
    }
    if (px_is_truthy(px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1))) {
        _v331 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v331;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v330), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_332:
    if (px_err_332_proped) return px_err_332_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v333 = (nargs > 0) ? args[0] : px_null();
    LXValue _v334 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_335_val = px_null();
    int px_err_335_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1)), px_str(": 语法错误 ")), _v333), px_str(": ")), _v334)}, 1));
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("parse "), _v333)}, 1));
px_err_335:
    if (px_err_335_proped) return px_err_335_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_336_val = px_null();
    int px_err_336_proped = 0;
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_336:
    if (px_err_336_proped) return px_err_336_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_337_val = px_null();
    int px_err_337_proped = 0;
    while (px_is_truthy(({ LXValue _t338 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t338) ? _t338 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_337:
    if (px_err_337_proped) return px_err_337_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_339_val = px_null();
    int px_err_339_proped = 0;
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_339:
    if (px_err_339_proped) return px_err_339_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v340 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_341_val = px_null();
    int px_err_341_proped = 0;
    return px_index(_v340, px_sub(px_call(px_get_global("len"), (LXValue[]){_v340}, 1), px_int(1LL)));
px_err_341:
    if (px_err_341_proped) return px_err_341_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v342 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_343_val = px_null();
    int px_err_343_proped = 0;
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v342}, 1);
px_err_343:
    if (px_err_343_proped) return px_err_343_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v344 = px_null();
    LXValue px_err_345_val = px_null();
    int px_err_345_proped = 0;
    _v344 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        (void)(px_method(_v344, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v344}, 2);
px_err_345:
    if (px_err_345_proped) return px_err_345_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v346 = px_null();
    LXValue _v347 = px_null();
    LXValue _v348 = px_null();
    LXValue _v349 = px_null();
    LXValue px_err_350_val = px_null();
    int px_err_350_proped = 0;
    _v346 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v346, px_str("let")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    if (px_is_truthy(px_eq(_v346, px_str("var")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    if (px_is_truthy(px_eq(_v346, px_str("const")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    if (px_is_truthy(px_eq(_v346, px_str("if")))) {
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("for")))) {
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("while")))) {
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("def")))) {
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("extern")))) {
        return px_call(px_get_global("parse_extern_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("struct")))) {
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("enum")))) {
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(({ LXValue _t351 = px_eq(_v346, px_str("标识符")); px_is_truthy(_t351) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("type")) : _t351; }))) {
        if (px_is_truthy(({ LXValue _t352 = px_call(px_get_global("chk2"), (LXValue[]){px_str("标识符")}, 1); px_is_truthy(_t352) ? px_call(px_get_global("chk3"), (LXValue[]){px_str("const")}, 1) : _t352; }))) {
            return px_call(px_get_global("parse_type_const"), (LXValue[]){}, 0);
        }
        return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("trait")))) {
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("impl")))) {
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("import")))) {
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("from")))) {
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("return")))) {
        _v347 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t354 = ({ LXValue _t353 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t353) ? _t353 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t354) ? _t354 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v347}, 3);
        }
        _v348 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Return"), _v348, _v347}, 3);
    }
    if (px_is_truthy(px_eq(_v346, px_str("break")))) {
        _v347 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Break"), _v347}, 2);
    }
    if (px_is_truthy(px_eq(_v346, px_str("continue")))) {
        _v347 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Continue"), _v347}, 2);
    }
    if (px_is_truthy(px_eq(_v346, px_str("spawn")))) {
        _v347 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v349 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Spawn"), _v349, _v347}, 3);
    }
    if (px_is_truthy(px_eq(_v346, px_str("select")))) {
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v346, px_str("fn")))) {
        _v349 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        _v347 = px_call(px_get_global("node_pos"), (LXValue[]){_v349}, 1);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v349, _v347}, 3);
    }
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_350:
    if (px_err_350_proped) return px_err_350_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v355 = (nargs > 0) ? args[0] : px_null();
    LXValue _v356 = px_null();
    LXValue _v357 = px_null();
    LXValue _v358 = px_null();
    LXValue _v359 = px_null();
    LXValue px_err_360_val = px_null();
    int px_err_360_proped = 0;
    _v356 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(({ LXValue _t362 = ({ LXValue _t361 = px_eq(_v355, px_str("Let")); px_is_truthy(_t361) ? px_call(px_get_global("chk"), (LXValue[]){px_str("标识符")}, 1) : _t361; }); px_is_truthy(_t362) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("mut")) : _t362; }))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v355 = px_str("Mut");
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    _v357 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    _v358 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v358 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    _v359 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v359 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    return px_list_n((LXValue[]){px_str("VarDecl"), _v355, px_call(px_get_global("qstr"), (LXValue[]){_v357}, 1), _v358, _v359, _v356}, 6);
px_err_360:
    if (px_err_360_proped) return px_err_360_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v363 = px_null();
    LXValue _v364 = px_null();
    LXValue _v365 = px_null();
    LXValue _v366 = px_null();
    LXValue _v367 = px_null();
    LXValue px_err_368_val = px_null();
    int px_err_368_proped = 0;
    _v363 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    _v364 = px_call(px_get_global("node_pos"), (LXValue[]){_v363}, 1);
    _v365 = px_null();
    _v366 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v366, px_str("=")))) {
         _v365 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("<-")))) {
         _v365 = px_str("Append");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("+=")))) {
         _v365 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("-=")))) {
         _v365 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("*=")))) {
         _v365 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("/=")))) {
         _v365 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("//=")))) {
         _v365 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("%=")))) {
         _v365 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("**=")))) {
         _v365 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("&=")))) {
         _v365 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("|=")))) {
         _v365 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("^=")))) {
         _v365 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v366, px_str("<<=")))) {
         _v365 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v366, px_str(">>=")))) {
         _v365 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v366, px_str(">>>=")))) {
         _v365 = px_str("ShrU");
    }
    if (px_is_truthy(px_ne(_v365, px_null()))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v367 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Assign"), _v363, _v365, _v367, _v364}, 5);
    }
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v363, _v364}, 3);
px_err_368:
    if (px_err_368_proped) return px_err_368_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v369 = px_null();
    LXValue _v370 = px_null();
    LXValue _v371 = px_null();
    LXValue _v372 = px_null();
    LXValue _v373 = px_null();
    LXValue _v374 = px_null();
    LXValue _v375 = px_null();
    LXValue px_err_376_val = px_null();
    int px_err_376_proped = 0;
    _v369 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v370 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    _v371 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    _v372 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v370, _v371}, 2)}, 1);
    _v373 = px_null();
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v374 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            _v375 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            (void)(px_method(_v372, "append", (LXValue[]){px_list_n((LXValue[]){_v374, _v375}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
             _v373 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            break;
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){px_str("If"), _v372, _v373, _v369}, 4);
px_err_376:
    if (px_err_376_proped) return px_err_376_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v377 = px_null();
    LXValue _v378 = px_null();
    LXValue _v379 = px_null();
    LXValue _v380 = px_null();
    LXValue px_err_381_val = px_null();
    int px_err_381_proped = 0;
    _v377 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v378 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    _v379 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    _v380 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("For"), px_call(px_get_global("qstr"), (LXValue[]){_v378}, 1), _v379, _v380, _v377}, 5);
px_err_381:
    if (px_err_381_proped) return px_err_381_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v382 = px_null();
    LXValue _v383 = px_null();
    LXValue _v384 = px_null();
    LXValue px_err_385_val = px_null();
    int px_err_385_proped = 0;
    _v382 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v383 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    _v384 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("While"), _v383, _v384, _v382}, 4);
px_err_385:
    if (px_err_385_proped) return px_err_385_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v386 = px_null();
    LXValue px_err_387_val = px_null();
    int px_err_387_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v386 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t388 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t388) ? _t388 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_method(_v386, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return _v386;
px_err_387:
    if (px_err_387_proped) return px_err_387_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v389 = px_null();
    LXValue _v390 = px_null();
    LXValue _v391 = px_null();
    LXValue _v392 = px_null();
    LXValue px_err_393_val = px_null();
    int px_err_393_proped = 0;
    _v389 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        return _v389;
    }
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    while (px_is_truthy(px_bool(true))) {
        _v390 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        _v391 = _v390;
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v392 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
             _v391 = px_add(px_add(_v390, px_str(": ")), _v392);
        }
        (void)(px_method(_v389, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v391}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    return _v389;
px_err_393:
    if (px_err_393_proped) return px_err_393_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v394 = px_null();
    LXValue _v395 = px_null();
    LXValue _v396 = px_null();
    LXValue _v397 = px_null();
    LXValue _v398 = px_null();
    LXValue _v399 = px_null();
    LXValue px_err_400_val = px_null();
    int px_err_400_proped = 0;
    _v394 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v395 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    _v396 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    _v397 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    _v398 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v398 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    _v399 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v395}, 1), _v397, _v398, _v399, _v394, _v396}, 7);
px_err_400:
    if (px_err_400_proped) return px_err_400_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v401 = px_null();
    LXValue _v402 = px_null();
    LXValue _v403 = px_null();
    LXValue _v404 = px_null();
    LXValue px_err_405_val = px_null();
    int px_err_405_proped = 0;
    _v401 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    _v402 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    _v403 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    _v404 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v404 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v402}, 1), _v403, _v404, _v401}, 5);
px_err_405:
    if (px_err_405_proped) return px_err_405_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v406 = px_null();
    LXValue _v407 = px_null();
    LXValue _v408 = px_null();
    LXValue _v409 = px_null();
    LXValue _v410 = px_null();
    LXValue _v411 = px_null();
    LXValue _v412 = px_null();
    LXValue px_err_413_val = px_null();
    int px_err_413_proped = 0;
    _v406 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v407 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    _v408 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v409 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t414 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t414) ? _t414 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        _v410 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        _v411 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        _v412 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_method(_v409, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v411}, 1), _v412, _v410}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t415 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t415) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t415; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v407}, 1), _v409, _v406, _v408}, 5);
px_err_413:
    if (px_err_413_proped) return px_err_413_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v416 = px_null();
    LXValue _v417 = px_null();
    LXValue _v418 = px_null();
    LXValue _v419 = px_null();
    LXValue _v420 = px_null();
    LXValue _v421 = px_null();
    LXValue px_err_422_val = px_null();
    int px_err_422_proped = 0;
    _v416 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v417 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v418 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t423 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t423) ? _t423 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        _v419 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        _v420 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        _v421 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v421, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        (void)(px_method(_v418, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v420}, 1), _v421, _v419}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t424 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t424) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t424; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v417}, 1), _v418, _v416}, 4);
px_err_422:
    if (px_err_422_proped) return px_err_422_val;
    return px_null();
}

static LXValue fn_parse_type_const(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v425 = px_null();
    LXValue _v426 = px_null();
    LXValue _v427 = px_null();
    LXValue _v428 = px_null();
    LXValue _v429 = px_null();
    LXValue _v430 = px_null();
    LXValue px_err_431_val = px_null();
    int px_err_431_proped = 0;
    _v425 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v426 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("const"), px_str("'const'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    _v427 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            _v428 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            _v429 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("常量名")}, 1);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
            _v430 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v427, "append", (LXValue[]){px_list_n((LXValue[]){px_str("TypeConstItem"), px_call(px_get_global("qstr"), (LXValue[]){_v429}, 1), _v430, _v428}, 4)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return px_list_n((LXValue[]){px_str("TypeConst"), px_call(px_get_global("qstr"), (LXValue[]){_v426}, 1), _v427, _v425}, 4);
px_err_431:
    if (px_err_431_proped) return px_err_431_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v432 = px_null();
    LXValue _v433 = px_null();
    LXValue _v434 = px_null();
    LXValue _v435 = px_null();
    LXValue _v436 = px_null();
    LXValue _v437 = px_null();
    LXValue _v438 = px_null();
    LXValue _v439 = px_null();
    LXValue px_err_440_val = px_null();
    int px_err_440_proped = 0;
    _v432 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v433 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v434 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t441 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t441) ? _t441 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        _v435 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v436 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        _v437 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        _v438 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v438 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        _v439 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                 _v439 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        (void)(px_method(_v434, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v436}, 1), _v437, _v438, _v439, _v435, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        if (px_is_truthy(({ LXValue _t442 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t442) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t442; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v433}, 1), _v434, _v432}, 4);
px_err_440:
    if (px_err_440_proped) return px_err_440_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v443 = px_null();
    LXValue _v444 = px_null();
    LXValue _v445 = px_null();
    LXValue _v446 = px_null();
    LXValue _v447 = px_null();
    LXValue _v448 = px_null();
    LXValue _v449 = px_null();
    LXValue _v450 = px_null();
    LXValue _v451 = px_null();
    LXValue _v452 = px_null();
    LXValue px_err_453_val = px_null();
    int px_err_453_proped = 0;
    _v443 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v444 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    _v445 = px_null();
    _v446 = _v444;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v446 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
         _v445 = px_call(px_get_global("qstr"), (LXValue[]){_v444}, 1);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v447 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t454 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t454) ? _t454 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        _v448 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v449 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        _v450 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        _v451 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v451 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        _v452 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        (void)(px_method(_v447, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v449}, 1), _v450, _v451, _v452, _v448, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v446}, 1), _v445, _v447, _v443}, 5);
px_err_453:
    if (px_err_453_proped) return px_err_453_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v455 = px_null();
    LXValue _v456 = px_null();
    LXValue _v457 = px_null();
    LXValue px_err_458_val = px_null();
    int px_err_458_proped = 0;
    _v455 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        _v456 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v456}, 1), px_list_n((LXValue[]){}, 0), _v455}, 4);
    }
    _v457 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v457, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v457, px_list_n((LXValue[]){}, 0), _v455}, 4);
px_err_458:
    if (px_err_458_proped) return px_err_458_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v459 = px_null();
    LXValue _v460 = px_null();
    LXValue _v461 = px_null();
    LXValue px_err_462_val = px_null();
    int px_err_462_proped = 0;
    _v459 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v460 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v460, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    _v461 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v461, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v460, _v461, _v459}, 4);
px_err_462:
    if (px_err_462_proped) return px_err_462_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v463 = px_null();
    LXValue _v464 = px_null();
    LXValue _v465 = px_null();
    LXValue _v466 = px_null();
    LXValue _v467 = px_null();
    LXValue _v468 = px_null();
    LXValue _v469 = px_null();
    LXValue px_err_470_val = px_null();
    int px_err_470_proped = 0;
    _v463 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v464 = px_list_n((LXValue[]){}, 0);
    _v465 = px_null();
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t471 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t471) ? _t471 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        if (px_is_truthy(({ LXValue _t472 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t472) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t472; }))) {
            _v466 = px_get_global("p_pos");
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                 _v465 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                continue;
            }
            else {
                px_set_global("p_pos", _v466);
            }
        }
        _v467 = px_null();
        if (px_is_truthy(({ LXValue _t473 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t473) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t473; }))) {
             _v467 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        _v468 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        _v469 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        (void)(px_method(_v464, "append", (LXValue[]){px_list_n((LXValue[]){_v467, _v468, _v469}, 3)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Select"), _v464, _v465, _v463}, 4);
px_err_470:
    if (px_err_470_proped) return px_err_470_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v474 = px_null();
    LXValue _v475 = px_null();
    LXValue px_err_476_val = px_null();
    int px_err_476_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    _v474 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    _v475 = px_call(px_get_global("node_pos"), (LXValue[]){_v474}, 1);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v474, _v475}, 3)}, 1);
px_err_476:
    if (px_err_476_proped) return px_err_476_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v477 = px_null();
    LXValue _v478 = px_null();
    LXValue _v479 = px_null();
    LXValue _v480 = px_null();
    LXValue _v481 = px_null();
    LXValue px_err_482_val = px_null();
    int px_err_482_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    _v477 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            _v478 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            _v479 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            _v480 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v480 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            _v481 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v481 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            (void)(px_method(_v477, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v479}, 1), _v480, _v481, _v478}, 5)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v477;
px_err_482:
    if (px_err_482_proped) return px_err_482_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_483_val = px_null();
    int px_err_483_proped = 0;
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_483:
    if (px_err_483_proped) return px_err_483_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v484 = px_null();
    LXValue _v485 = px_null();
    LXValue _v486 = px_null();
    LXValue px_err_487_val = px_null();
    int px_err_487_proped = 0;
    _v484 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|>")}, 1))) {
        _v485 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v486 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
         _v484 = px_list_n((LXValue[]){px_str("Pipe"), _v484, _v486, _v485}, 4);
    }
    return _v484;
px_err_487:
    if (px_err_487_proped) return px_err_487_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v488 = px_null();
    LXValue _v489 = px_null();
    LXValue _v490 = px_null();
    LXValue px_err_491_val = px_null();
    int px_err_491_proped = 0;
    _v488 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("??")}, 1))) {
        _v489 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v490 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
         _v488 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v488, _v490, _v489}, 4);
    }
    return _v488;
px_err_491:
    if (px_err_491_proped) return px_err_491_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v492 = px_null();
    LXValue _v493 = px_null();
    LXValue _v494 = px_null();
    LXValue px_err_495_val = px_null();
    int px_err_495_proped = 0;
    _v492 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("or")}, 1))) {
        _v493 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v494 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
         _v492 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v492, _v494, _v493}, 5);
    }
    return _v492;
px_err_495:
    if (px_err_495_proped) return px_err_495_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v496 = px_null();
    LXValue _v497 = px_null();
    LXValue _v498 = px_null();
    LXValue px_err_499_val = px_null();
    int px_err_499_proped = 0;
    _v496 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("and")}, 1))) {
        _v497 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v498 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
         _v496 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v496, _v498, _v497}, 5);
    }
    return _v496;
px_err_499:
    if (px_err_499_proped) return px_err_499_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v500 = px_null();
    LXValue _v501 = px_null();
    LXValue _v502 = px_null();
    LXValue _v503 = px_null();
    LXValue px_err_504_val = px_null();
    int px_err_504_proped = 0;
    _v500 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        _v501 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("==")}, 1))) {
             _v501 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("!=")}, 1))) {
             _v501 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<")}, 1))) {
             _v501 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<=")}, 1))) {
             _v501 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">")}, 1))) {
             _v501 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">=")}, 1))) {
             _v501 = px_str("Ge");
        }
        if (px_is_truthy(px_eq(_v501, px_null()))) {
            break;
        }
        _v502 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v503 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
         _v500 = px_list_n((LXValue[]){px_str("Binary"), _v501, _v500, _v503, _v502}, 5);
    }
    return _v500;
px_err_504:
    if (px_err_504_proped) return px_err_504_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v505 = px_null();
    LXValue _v506 = px_null();
    LXValue _v507 = px_null();
    LXValue px_err_508_val = px_null();
    int px_err_508_proped = 0;
    _v505 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|")}, 1))) {
        _v506 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v507 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
         _v505 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v505, _v507, _v506}, 5);
    }
    return _v505;
px_err_508:
    if (px_err_508_proped) return px_err_508_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v509 = px_null();
    LXValue _v510 = px_null();
    LXValue _v511 = px_null();
    LXValue px_err_512_val = px_null();
    int px_err_512_proped = 0;
    _v509 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("^")}, 1))) {
        _v510 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v511 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
         _v509 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v509, _v511, _v510}, 5);
    }
    return _v509;
px_err_512:
    if (px_err_512_proped) return px_err_512_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v513 = px_null();
    LXValue _v514 = px_null();
    LXValue _v515 = px_null();
    LXValue px_err_516_val = px_null();
    int px_err_516_proped = 0;
    _v513 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("&")}, 1))) {
        _v514 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v515 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
         _v513 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v513, _v515, _v514}, 5);
    }
    return _v513;
px_err_516:
    if (px_err_516_proped) return px_err_516_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v517 = px_null();
    LXValue _v518 = px_null();
    LXValue _v519 = px_null();
    LXValue _v520 = px_null();
    LXValue px_err_521_val = px_null();
    int px_err_521_proped = 0;
    _v517 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        _v518 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<<")}, 1))) {
             _v518 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>")}, 1))) {
             _v518 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>>")}, 1))) {
             _v518 = px_str("ShrU");
        }
        if (px_is_truthy(px_eq(_v518, px_null()))) {
            break;
        }
        _v519 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v520 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
         _v517 = px_list_n((LXValue[]){px_str("Binary"), _v518, _v517, _v520, _v519}, 5);
    }
    return _v517;
px_err_521:
    if (px_err_521_proped) return px_err_521_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v522 = px_null();
    LXValue _v523 = px_null();
    LXValue _v524 = px_null();
    LXValue _v525 = px_null();
    LXValue px_err_526_val = px_null();
    int px_err_526_proped = 0;
    _v522 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        _v523 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("+")}, 1))) {
             _v523 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("-")}, 1))) {
             _v523 = px_str("Sub");
        }
        if (px_is_truthy(px_eq(_v523, px_null()))) {
            break;
        }
        _v524 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v525 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
         _v522 = px_list_n((LXValue[]){px_str("Binary"), _v523, _v522, _v525, _v524}, 5);
    }
    return _v522;
px_err_526:
    if (px_err_526_proped) return px_err_526_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v527 = px_null();
    LXValue _v528 = px_null();
    LXValue _v529 = px_null();
    LXValue _v530 = px_null();
    LXValue px_err_531_val = px_null();
    int px_err_531_proped = 0;
    _v527 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        _v528 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("*")}, 1))) {
             _v528 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("/")}, 1))) {
             _v528 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("//")}, 1))) {
             _v528 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("%")}, 1))) {
             _v528 = px_str("Mod");
        }
        if (px_is_truthy(px_eq(_v528, px_null()))) {
            break;
        }
        _v529 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v530 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
         _v527 = px_list_n((LXValue[]){px_str("Binary"), _v528, _v527, _v530, _v529}, 5);
    }
    return _v527;
px_err_531:
    if (px_err_531_proped) return px_err_531_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v532 = px_null();
    LXValue _v533 = px_null();
    LXValue _v534 = px_null();
    LXValue px_err_535_val = px_null();
    int px_err_535_proped = 0;
    _v532 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("**")}, 1))) {
        _v533 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v534 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v532, _v534, _v533}, 5);
    }
    return _v532;
px_err_535:
    if (px_err_535_proped) return px_err_535_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v536 = px_null();
    LXValue _v537 = px_null();
    LXValue _v538 = px_null();
    LXValue px_err_539_val = px_null();
    int px_err_539_proped = 0;
    _v536 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v536, px_str("-")))) {
        _v537 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v538 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v538, _v537}, 4);
    }
    if (px_is_truthy(px_eq(_v536, px_str("not")))) {
        _v537 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v538 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v538, _v537}, 4);
    }
    if (px_is_truthy(px_eq(_v536, px_str("~")))) {
        _v537 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v538 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v538, _v537}, 4);
    }
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_539:
    if (px_err_539_proped) return px_err_539_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v540 = px_null();
    LXValue _v541 = px_null();
    LXValue _v542 = px_null();
    LXValue _v543 = px_null();
    LXValue _v544 = px_null();
    LXValue _v545 = px_null();
    LXValue _v546 = px_null();
    LXValue _v547 = px_null();
    LXValue px_err_548_val = px_null();
    int px_err_548_proped = 0;
    _v540 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        _v541 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v541, px_str("(")))) {
            _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            _v543 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v540 = px_list_n((LXValue[]){px_str("Call"), _v540, _v543, _v542}, 4);
        }
        else if (px_is_truthy(px_eq(_v541, px_str("[")))) {
            _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                _v544 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                _v545 = px_null();
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                     _v545 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                }
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                 _v540 = px_list_n((LXValue[]){px_str("Slice"), _v540, px_null(), _v544, _v545, _v542}, 6);
            }
            else {
                _v546 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    _v544 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    _v545 = px_null();
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                         _v545 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    }
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v540 = px_list_n((LXValue[]){px_str("Slice"), _v540, _v546, _v544, _v545, _v542}, 6);
                }
                else {
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v540 = px_list_n((LXValue[]){px_str("Index"), _v540, _v546, _v542}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v541, px_str(".")))) {
            _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v547 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v540 = px_list_n((LXValue[]){px_str("Field"), _v540, px_call(px_get_global("qstr"), (LXValue[]){_v547}, 1), _v542}, 4);
        }
        else if (px_is_truthy(px_eq(_v541, px_str("?.")))) {
            _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v547 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v540 = px_list_n((LXValue[]){px_str("OptionalField"), _v540, px_call(px_get_global("qstr"), (LXValue[]){_v547}, 1), _v542}, 4);
        }
        else if (px_is_truthy(px_eq(_v541, px_str("!")))) {
            _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v540 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v540, _v542}, 3);
        }
        else if (px_is_truthy(px_eq(_v541, px_str("?")))) {
            _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v540 = px_list_n((LXValue[]){px_str("Try"), _v540, _v542}, 3);
        }
        else {
            break;
        }
    }
    return _v540;
px_err_548:
    if (px_err_548_proped) return px_err_548_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_549_val = px_null();
    int px_err_549_proped = 0;
    if (px_is_truthy(({ LXValue _t550 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t550) ? _t550 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        return px_null();
    }
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_549:
    if (px_err_549_proped) return px_err_549_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v551 = px_null();
    LXValue px_err_552_val = px_null();
    int px_err_552_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    _v551 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v551, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v551;
px_err_552:
    if (px_err_552_proped) return px_err_552_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v553 = px_null();
    LXValue _v554 = px_null();
    LXValue _v555 = px_null();
    LXValue _v556 = px_null();
    LXValue _v557 = px_null();
    LXValue _v558 = px_null();
    LXValue px_err_559_val = px_null();
    int px_err_559_proped = 0;
    _v553 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v553, px_str("整数")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        _v555 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Int"), _v555, _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("浮点")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        _v555 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Float"), _v555, _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("字符串")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        _v555 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Str"), _v555, _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("true")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("false")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("null")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Null"), _v554}, 2);
    }
    if (px_is_truthy(px_eq(_v553, px_str("self")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("标识符")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        _v556 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v556}, 1), _v554}, 3);
    }
    if (px_is_truthy(px_eq(_v553, px_str("[")))) {
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v553, px_str("(")))) {
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v553, px_str("{")))) {
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v553, px_str("fn")))) {
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v553, px_str("match")))) {
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v553, px_str("if")))) {
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v553, px_str("chan")))) {
        _v554 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v557 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v554}, 3);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            _v558 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v557 = px_list_n((LXValue[]){px_str("Call"), _v557, _v558, _v554}, 4);
        }
        return _v557;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_559:
    if (px_err_559_proped) return px_err_559_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v560 = px_null();
    LXValue _v561 = px_null();
    LXValue _v562 = px_null();
    LXValue _v563 = px_null();
    LXValue px_err_564_val = px_null();
    int px_err_564_proped = 0;
    _v560 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v560}, 3);
    }
    _v561 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        _v562 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("ListComp"), _v561, px_index(_v562, px_int(0LL)), px_index(_v562, px_int(1LL)), _v560}, 5);
    }
    _v563 = px_list_n((LXValue[]){_v561}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            break;
        }
        (void)(px_method(_v563, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    return px_list_n((LXValue[]){px_str("List"), _v563, _v560}, 3);
px_err_564:
    if (px_err_564_proped) return px_err_564_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v565 = px_null();
    LXValue px_err_566_val = px_null();
    int px_err_566_proped = 0;
    _v565 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        (void)(px_method(_v565, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    return _v565;
px_err_566:
    if (px_err_566_proped) return px_err_566_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v567 = px_null();
    LXValue _v568 = px_null();
    LXValue _v569 = px_null();
    LXValue _v570 = px_null();
    LXValue px_err_571_val = px_null();
    int px_err_571_proped = 0;
    _v567 = px_list_n((LXValue[]){}, 0);
    _v568 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v569 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            _v570 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v567, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v569, _v570}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_method(_v568, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){_v567, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v568}, 1)}, 2);
px_err_571:
    if (px_err_571_proped) return px_err_571_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v572 = (nargs > 0) ? args[0] : px_null();
    LXValue _v573 = px_null();
    LXValue _v574 = px_null();
    LXValue _v575 = px_null();
    LXValue px_err_576_val = px_null();
    int px_err_576_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v572}, 1), px_int(0LL)))) {
        return px_null();
    }
    _v573 = px_index(_v572, px_int(0LL));
    _v574 = px_int(1LL);
    while (px_is_truthy(px_lt(_v574, px_call(px_get_global("len"), (LXValue[]){_v572}, 1)))) {
        _v575 = px_call(px_get_global("node_pos"), (LXValue[]){_v573}, 1);
         _v573 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v573, px_index(_v572, _v574), _v575}, 5);
         _v574 = px_add(_v574, px_int(1LL));
    }
    return _v573;
px_err_576:
    if (px_err_576_proped) return px_err_576_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v577 = px_null();
    LXValue _v578 = px_null();
    LXValue _v579 = px_null();
    LXValue _v580 = px_null();
    LXValue px_err_581_val = px_null();
    int px_err_581_proped = 0;
    _v577 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v577}, 3);
    }
    _v578 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        _v579 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("GenExp"), _v578, px_index(_v579, px_int(0LL)), px_index(_v579, px_int(1LL)), _v577}, 5);
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        _v580 = px_list_n((LXValue[]){_v578}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                break;
            }
            (void)(px_method(_v580, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("Tuple"), _v580, _v577}, 3);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v578;
px_err_581:
    if (px_err_581_proped) return px_err_581_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v582 = px_null();
    LXValue _v583 = px_null();
    LXValue _v584 = px_null();
    LXValue px_err_585_val = px_null();
    int px_err_585_proped = 0;
    _v582 = px_int(0LL);
    _v583 = px_get_global("p_pos");
    while (px_is_truthy(px_lt(_v583, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        _v584 = px_index(px_index(px_get_global("p_toks"), _v583), px_int(0LL));
        if (px_is_truthy(({ LXValue _t586 = px_eq(_v584, px_str(":")); px_is_truthy(_t586) ? px_eq(_v582, px_int(0LL)) : _t586; }))) {
            return px_bool(true);
        }
        if (px_is_truthy(({ LXValue _t588 = ({ LXValue _t587 = px_eq(_v584, px_str("(")); px_is_truthy(_t587) ? _t587 : px_eq(_v584, px_str("[")); }); px_is_truthy(_t588) ? _t588 : px_eq(_v584, px_str("{")); }))) {
             _v582 = px_add(_v582, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t589 = px_eq(_v584, px_str(")")); px_is_truthy(_t589) ? _t589 : px_eq(_v584, px_str("]")); }))) {
            if (px_is_truthy(px_gt(_v582, px_int(0LL)))) {
                 _v582 = px_sub(_v582, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t590 = px_eq(_v584, px_str("}")); px_is_truthy(_t590) ? px_eq(_v582, px_int(0LL)) : _t590; }))) {
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t593 = ({ LXValue _t592 = ({ LXValue _t591 = px_eq(_v584, px_str(",")); px_is_truthy(_t591) ? _t591 : px_eq(_v584, px_str("换行")); }); px_is_truthy(_t592) ? _t592 : px_eq(_v584, px_str("EOF")); }); px_is_truthy(_t593) ? px_eq(_v582, px_int(0LL)) : _t593; }))) {
            return px_bool(false);
        }
         _v583 = px_add(_v583, px_int(1LL));
    }
    return px_bool(false);
px_err_585:
    if (px_err_585_proped) return px_err_585_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v594 = px_null();
    LXValue _v595 = px_null();
    LXValue _v596 = px_null();
    LXValue _v597 = px_null();
    LXValue _v598 = px_null();
    LXValue _v599 = px_null();
    LXValue _v600 = px_null();
    LXValue _v601 = px_null();
    LXValue _v602 = px_null();
    LXValue px_err_603_val = px_null();
    int px_err_603_proped = 0;
    _v594 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    _v595 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    if (px_is_truthy(_v595)) {
        _v596 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        _v597 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            _v598 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            return px_list_n((LXValue[]){px_str("DictComp"), _v596, _v597, px_index(_v598, px_int(0LL)), px_index(_v598, px_int(1LL)), _v594}, 6);
        }
        _v599 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v596, _v597}, 2)}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                break;
            }
            _v600 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            _v601 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v599, "append", (LXValue[]){px_list_n((LXValue[]){_v600, _v601}, 2)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("Dict"), _v599, _v594}, 3);
    }
    _v602 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    while (px_is_truthy(({ LXValue _t604 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t604) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t604; }))) {
        (void)(px_method(_v602, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    return px_list_n((LXValue[]){px_str("Block"), _v602, _v594}, 3);
px_err_603:
    if (px_err_603_proped) return px_err_603_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v605 = px_null();
    LXValue _v606 = px_null();
    LXValue _v607 = px_null();
    LXValue _v608 = px_null();
    LXValue _v609 = px_null();
    LXValue _v610 = px_null();
    LXValue _v611 = px_null();
    LXValue px_err_612_val = px_null();
    int px_err_612_proped = 0;
    _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v606 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    _v607 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v607 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    _v608 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v608, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    _v609 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        _v610 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v611 = px_list_n((LXValue[]){}, 0);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        while (px_is_truthy(({ LXValue _t613 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t613) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t613; }))) {
            (void)(px_method(_v611, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
         _v609 = px_list_n((LXValue[]){px_str("Block"), _v611, _v610}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v609 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    else {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    return px_list_n((LXValue[]){px_str("Closure"), _v606, _v607, _v609, _v608, _v605}, 6);
px_err_612:
    if (px_err_612_proped) return px_err_612_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v614 = px_null();
    LXValue _v615 = px_null();
    LXValue _v616 = px_null();
    LXValue _v617 = px_null();
    LXValue _v618 = px_null();
    LXValue _v619 = px_null();
    LXValue _v620 = px_null();
    LXValue _v621 = px_null();
    LXValue px_err_622_val = px_null();
    int px_err_622_proped = 0;
    _v614 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v615 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    _v616 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t623 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t623) ? _t623 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        _v617 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        _v618 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v618 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        _v619 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            _v620 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            _v621 = px_null();
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v620}, 1), px_int(0LL)))) {
                 _v621 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v620, px_int(0LL))}, 1);
            }
            else {
                 _v621 = _v614;
            }
             _v619 = px_list_n((LXValue[]){px_str("Block"), _v620, _v621}, 3);
        }
        else {
             _v619 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_method(_v616, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v617, _v618, _v619, _v614}, 5)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Match"), _v615, _v616, _v614}, 4);
px_err_622:
    if (px_err_622_proped) return px_err_622_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v624 = px_null();
    LXValue _v625 = px_null();
    LXValue _v626 = px_null();
    LXValue _v627 = px_null();
    LXValue px_err_628_val = px_null();
    int px_err_628_proped = 0;
    _v624 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    _v625 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    _v626 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    _v627 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v625, _v626, _v627, _v624}, 5);
px_err_628:
    if (px_err_628_proped) return px_err_628_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v629 = px_null();
    LXValue _v630 = px_null();
    LXValue _v631 = px_null();
    LXValue _v632 = px_null();
    LXValue _v633 = px_null();
    LXValue _v634 = px_null();
    LXValue _v635 = px_null();
    LXValue _v636 = px_null();
    LXValue px_err_637_val = px_null();
    int px_err_637_proped = 0;
    _v629 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t642 = ({ LXValue _t641 = ({ LXValue _t640 = ({ LXValue _t639 = ({ LXValue _t638 = px_eq(_v629, px_str("整数")); px_is_truthy(_t638) ? _t638 : px_eq(_v629, px_str("浮点")); }); px_is_truthy(_t639) ? _t639 : px_eq(_v629, px_str("字符串")); }); px_is_truthy(_t640) ? _t640 : px_eq(_v629, px_str("true")); }); px_is_truthy(_t641) ? _t641 : px_eq(_v629, px_str("false")); }); px_is_truthy(_t642) ? _t642 : px_eq(_v629, px_str("null")); }))) {
        _v630 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v630}, 2);
    }
    if (px_is_truthy(px_eq(_v629, px_str("标识符")))) {
        _v631 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        _v632 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            _v633 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v634 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            return px_list_n((LXValue[]){px_str("PatLiteral"), px_list_n((LXValue[]){px_str("Field"), px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v631}, 1), _v633}, 3), px_call(px_get_global("qstr"), (LXValue[]){_v634}, 1), _v633}, 4)}, 2);
        }
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v635 = px_list_n((LXValue[]){}, 0);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v635, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v631}, 1), _v635}, 3);
        }
        if (px_is_truthy(px_eq(_v631, px_str("_")))) {
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v631}, 1))) {
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v631}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v631}, 1)}, 2);
    }
    if (px_is_truthy(px_eq(_v629, px_str("(")))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v636 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v636, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    continue;
                }
                break;
            }
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("PatTuple"), _v636}, 2);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_637:
    if (px_err_637_proped) return px_err_637_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v643 = (nargs > 0) ? args[0] : px_null();
    LXValue _v644 = px_null();
    LXValue px_err_645_val = px_null();
    int px_err_645_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v643}, 1), px_int(0LL)))) {
        return px_bool(false);
    }
    _v644 = px_index(_v643, px_int(0LL));
    return ({ LXValue _t646 = px_ge(_v644, px_str("A")); px_is_truthy(_t646) ? px_le(_v644, px_str("Z")) : _t646; });
px_err_645:
    if (px_err_645_proped) return px_err_645_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v647 = px_null();
    LXValue _v648 = px_null();
    LXValue px_err_649_val = px_null();
    int px_err_649_proped = 0;
    _v647 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        _v648 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("TyOptional"), _v647, _v648}, 3);
    }
    return _v647;
px_err_649:
    if (px_err_649_proped) return px_err_649_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v650 = px_null();
    LXValue _v651 = px_null();
    LXValue _v652 = px_null();
    LXValue _v653 = px_null();
    LXValue _v654 = px_null();
    LXValue _v655 = px_null();
    LXValue _v656 = px_null();
    LXValue _v657 = px_null();
    LXValue px_err_658_val = px_null();
    int px_err_658_proped = 0;
    _v650 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v650, px_str("标识符")))) {
        _v651 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        _v652 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v653 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            if (px_is_truthy(px_eq(_v651, px_str("list")))) {
                return px_list_n((LXValue[]){px_str("TyList"), _v653, _v652}, 3);
            }
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v651}, 1), px_list_n((LXValue[]){_v653}, 1), _v652}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v651}, 1), _v652}, 3);
    }
    if (px_is_truthy(px_eq(_v650, px_str("[")))) {
        _v652 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v653 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("TyList"), _v653, _v652}, 3);
    }
    if (px_is_truthy(px_eq(_v650, px_str("{")))) {
        _v652 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v654 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        _v655 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("TyDict"), _v654, _v655, _v652}, 4);
    }
    if (px_is_truthy(px_eq(_v650, px_str("(")))) {
        _v652 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        _v656 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v656, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    continue;
                }
                break;
            }
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            _v657 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v656, _v657, _v652}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyTuple"), _v656, _v652}, 3);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_658:
    if (px_err_658_proped) return px_err_658_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v659 = (nargs > 0) ? args[0] : px_null();
    LXValue _v660 = (nargs > 1) ? args[1] : px_null();
    LXValue _v661 = px_null();
    LXValue _v662 = px_null();
    LXValue _v663 = px_null();
    LXValue _v664 = px_null();
    LXValue _v665 = px_null();
    LXValue _v666 = px_null();
    LXValue _v667 = px_null();
    LXValue _v668 = px_null();
    LXValue _v669 = px_null();
    LXValue _v670 = px_null();
    LXValue _v671 = px_null();
    LXValue _v672 = px_null();
    LXValue _v673 = px_null();
    LXValue _v674 = px_null();
    LXValue _v675 = px_null();
    LXValue _v676 = px_null();
    LXValue _v677 = px_null();
    LXValue _v678 = px_null();
    LXValue _v679 = px_null();
    LXValue _v680 = px_null();
    LXValue _v681 = px_null();
    LXValue _v682 = px_null();
    LXValue _v683 = px_null();
    LXValue _v684 = px_null();
    LXValue _v685 = px_null();
    LXValue _v686 = px_null();
    LXValue _v687 = px_null();
    LXValue _v688 = px_null();
    LXValue _v689 = px_null();
    LXValue _v690 = px_null();
    LXValue _v691 = px_null();
    LXValue _v692 = px_null();
    LXValue _v693 = px_null();
    LXValue _v694 = px_null();
    LXValue px_err_695_val = px_null();
    int px_err_695_proped = 0;
    LXValue _v696 = px_call(px_get_global("cg_pad"), (LXValue[]){_v660}, 1);
    _v661 = px_index(_v659, px_int(0LL));
    if (px_is_truthy(px_eq(_v661, px_str("VarDecl")))) {
        _v662 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v659, px_int(2LL))}, 1);
        if (px_is_truthy(({ LXValue _t697 = px_eq(px_index(_v659, px_int(1LL)), px_str("Let")); px_is_truthy(_t697) ? _t697 : px_eq(px_index(_v659, px_int(1LL)), px_str("Const")); }))) {
            (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v662}, 1));
        }
        _v663 = px_index(_v659, px_int(3LL));
        if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v663}, 1))) {
            px_index_set(px_get_global("cg_nonnull"), _v662, px_int(1LL));
            if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v659, px_int(4LL))}, 1))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v663}, 1)), px_str("'（可空类型请用 ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v663}, 1)), px_str("? 声明）"))}, 2));
            }
        }
        _v664 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v659, px_int(4LL)), px_null()))) {
             _v664 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(4LL))}, 1);
        }
        if (px_is_truthy(({ LXValue _t698 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v662}, 2); px_is_truthy(_t698) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t698; }))) {
            return px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_set_global(\"")), _v662), px_str("\", ")), _v664), px_str(");\n"));
        }
        _v665 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v662}, 1);
        if (px_is_truthy(px_eq(_v665, px_null()))) {
             _v665 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v662}, 1);
            if (px_is_truthy(px_ne(px_index(_v659, px_int(4LL)), px_null()))) {
                _v666 = px_index(_v659, px_int(4LL));
                _v667 = px_null();
                if (px_is_truthy(px_eq(px_index(_v666, px_int(0LL)), px_str("Constructor")))) {
                     _v667 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v666, px_int(1LL))}, 1);
                }
                else if (px_is_truthy(px_eq(px_index(_v666, px_int(0LL)), px_str("Call")))) {
                    _v668 = px_index(_v666, px_int(1LL));
                    if (px_is_truthy(px_eq(px_index(_v668, px_int(0LL)), px_str("Var")))) {
                         _v667 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v668, px_int(1LL))}, 1);
                    }
                }
                if (px_is_truthy(px_ne(_v667, px_null()))) {
                    if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v667}, 1))) {
                        px_index_set(px_get_global("cg_var_types"), _v662, _v667);
                    }
                }
            }
            return px_add(px_add(px_add(px_add(px_add(_v696, px_str("LXValue ")), _v665), px_str(" = ")), _v664), px_str(";\n"));
        }
        if (px_is_truthy(px_ne(px_index(_v659, px_int(4LL)), px_null()))) {
            _v666 = px_index(_v659, px_int(4LL));
            _v667 = px_null();
            if (px_is_truthy(px_eq(px_index(_v666, px_int(0LL)), px_str("Constructor")))) {
                 _v667 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v666, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v666, px_int(0LL)), px_str("Call")))) {
                _v668 = px_index(_v666, px_int(1LL));
                if (px_is_truthy(px_eq(px_index(_v668, px_int(0LL)), px_str("Var")))) {
                     _v667 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v668, px_int(1LL))}, 1);
                }
            }
            if (px_is_truthy(px_ne(_v667, px_null()))) {
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v667}, 1))) {
                    px_index_set(px_get_global("cg_var_types"), _v662, _v667);
                }
            }
        }
        return px_add(px_add(px_add(px_add(_v696, _v665), px_str(" = ")), _v664), px_str(";\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Assign")))) {
        _v664 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(3LL))}, 1);
        _v669 = px_index(_v659, px_int(1LL));
        _v670 = px_index(_v659, px_int(2LL));
        if (px_is_truthy(px_eq(_v670, px_str("Append")))) {
            _v671 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v669}, 1);
            return px_add(px_add(px_add(px_add(px_add(_v696, px_str("(void)(px_method(")), _v671), px_str(", \"append\", (LXValue[]){")), _v664), px_str("}, 1));\n"));
        }
        _v672 = px_index(_v669, px_int(0LL));
        if (px_is_truthy(px_eq(_v672, px_str("Var")))) {
            _v662 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v669, px_int(1LL))}, 1);
            if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v662}, 1))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v662), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
            }
            if (px_is_truthy(({ LXValue _t699 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v659, px_int(3LL))}, 1); px_is_truthy(_t699) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v662}, 1) : _t699; }))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v662), px_str("'（可空类型请声明为 ")), _v662), px_str(": T?）"))}, 2));
            }
            _v673 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v662}, 1);
            if (px_is_truthy(px_eq(_v673, px_null()))) {
                if (px_is_truthy(px_eq(_v670, px_str("Assign")))) {
                    return px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_set_global(\"")), _v662), px_str("\", ")), _v664), px_str(");\n"));
                }
                _v674 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v670, _v662, _v664}, 3);
                return px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_set_global(\"")), _v662), px_str("\", ")), _v674), px_str(");\n"));
            }
            _v674 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v670, _v673, _v664}, 3);
            return px_add(px_add(px_add(px_add(px_add(_v696, px_str(" ")), _v673), px_str(" = ")), _v674), px_str(";\n"));
        }
        if (px_is_truthy(px_eq(_v672, px_str("Field")))) {
            _v671 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v669, px_int(1LL))}, 1);
            _v675 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v669, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_field_set(")), _v671), px_str(", \"")), _v675), px_str("\", ")), _v664), px_str(");\n"));
        }
        if (px_is_truthy(px_eq(_v672, px_str("Index")))) {
            _v671 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v669, px_int(1LL))}, 1);
            _v676 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v669, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_index_set(")), _v671), px_str(", ")), _v676), px_str(", ")), _v664), px_str(");\n"));
        }
        return px_str("不支持的赋值目标");
    }
    if (px_is_truthy(px_eq(_v661, px_str("ExprStmt")))) {
        _v666 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v696, px_str("(void)(")), _v666), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("If")))) {
        _v677 = px_str("");
        _v678 = px_index(_v659, px_int(1LL));
        _v679 = px_int(0LL);
        while (px_is_truthy(px_lt(_v679, px_call(px_get_global("len"), (LXValue[]){_v678}, 1)))) {
            _v680 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v678, _v679), px_int(0LL))}, 1);
            _v681 = px_str("if");
            if (px_is_truthy(px_gt(_v679, px_int(0LL)))) {
                 _v681 = px_str("else if");
            }
             _v677 = px_add(_v677, px_add(px_add(px_add(px_add(_v696, _v681), px_str(" (px_is_truthy(")), _v680), px_str(")) {\n")));
            _v682 = px_index(px_index(_v678, _v679), px_int(1LL));
            _v683 = px_int(0LL);
            while (px_is_truthy(px_lt(_v683, px_call(px_get_global("len"), (LXValue[]){_v682}, 1)))) {
                 _v677 = px_add(_v677, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v682, _v683), px_add(_v660, px_int(1LL))}, 2));
                 _v683 = px_add(_v683, px_int(1LL));
            }
             _v677 = px_add(_v677, px_add(_v696, px_str("}\n")));
             _v679 = px_add(_v679, px_int(1LL));
        }
        if (px_is_truthy(px_ne(px_index(_v659, px_int(2LL)), px_null()))) {
             _v677 = px_add(_v677, px_add(_v696, px_str("else {\n")));
            _v684 = px_index(_v659, px_int(2LL));
            _v685 = px_int(0LL);
            while (px_is_truthy(px_lt(_v685, px_call(px_get_global("len"), (LXValue[]){_v684}, 1)))) {
                 _v677 = px_add(_v677, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v684, _v685), px_add(_v660, px_int(1LL))}, 2));
                 _v685 = px_add(_v685, px_int(1LL));
            }
             _v677 = px_add(_v677, px_add(_v696, px_str("}\n")));
        }
        return _v677;
    }
    if (px_is_truthy(px_eq(_v661, px_str("While")))) {
        _v680 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1);
        _v677 = px_add(px_add(px_add(_v696, px_str("while (px_is_truthy(")), _v680), px_str(")) {\n"));
        _v682 = px_index(_v659, px_int(2LL));
        _v679 = px_int(0LL);
        while (px_is_truthy(px_lt(_v679, px_call(px_get_global("len"), (LXValue[]){_v682}, 1)))) {
             _v677 = px_add(_v677, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v682, _v679), px_add(_v660, px_int(1LL))}, 2));
             _v679 = px_add(_v679, px_int(1LL));
        }
         _v677 = px_add(_v677, px_add(_v696, px_str("}\n")));
        return _v677;
    }
    if (px_is_truthy(px_eq(_v661, px_str("For")))) {
        _v686 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(2LL))}, 1);
        _v687 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v688 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v689 = px_call(px_get_global("cg_var_of"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1)}, 1);
        _v690 = px_str("LXValue ");
        if (px_is_truthy(({ LXValue _t700 = px_eq(_v689, px_null()); px_is_truthy(_t700) ? _t700 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)); }))) {
             _v689 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1)}, 1);
        }
        else {
             _v690 = px_str("");
        }
        _v677 = px_add(px_add(px_add(px_add(px_add(_v696, px_str("LXValue ")), _v687), px_str(" = ")), _v686), px_str(";\n"));
         _v677 = px_add(_v677, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v696, px_str("for (int ")), _v688), px_str(" = 0; ")), _v688), px_str(" < px_len(")), _v687), px_str("); ")), _v688), px_str("++) {\n")));
         _v677 = px_add(_v677, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v696, px_str("    ")), _v690), _v689), px_str(" = px_index(")), _v687), px_str(", px_int(")), _v688), px_str("));\n")));
        _v682 = px_index(_v659, px_int(3LL));
        _v679 = px_int(0LL);
        while (px_is_truthy(px_lt(_v679, px_call(px_get_global("len"), (LXValue[]){_v682}, 1)))) {
             _v677 = px_add(_v677, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v682, _v679), px_add(_v660, px_int(1LL))}, 2));
             _v679 = px_add(_v679, px_int(1LL));
        }
         _v677 = px_add(_v677, px_add(_v696, px_str("}\n")));
        return _v677;
    }
    if (px_is_truthy(px_eq(_v661, px_str("Return")))) {
        if (px_is_truthy(px_ne(px_index(_v659, px_int(1LL)), px_null()))) {
            _v666 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1);
            return px_add(px_add(px_add(_v696, px_str("return ")), _v666), px_str(";\n"));
        }
        return px_add(_v696, px_str("return px_null();\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Break")))) {
        return px_add(_v696, px_str("break;\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Continue")))) {
        return px_add(_v696, px_str("continue;\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Empty")))) {
        return px_str("");
    }
    if (px_is_truthy(px_eq(_v661, px_str("ChanDecl")))) {
        _v673 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1)}, 1);
        return px_add(px_add(px_add(_v696, px_str("LXValue ")), _v673), px_str(" = px_chan_create(0);\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Send")))) {
        _v680 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1);
        _v673 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_chan_send(")), _v680), px_str(", ")), _v673), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Recv")))) {
        _v680 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v659, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v696, px_str("px_chan_recv(")), _v680), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v661, px_str("Spawn")))) {
        _v691 = px_index(_v659, px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v691, px_int(0LL)), px_str("Call")))) {
            _v668 = px_index(_v691, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v668, px_int(0LL)), px_str("Var")))) {
                _v675 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v668, px_int(1LL))}, 1);
                _v692 = px_list_n((LXValue[]){}, 0);
                _v693 = px_index(_v691, px_int(2LL));
                _v694 = px_int(0LL);
                while (px_is_truthy(px_lt(_v694, px_call(px_get_global("len"), (LXValue[]){_v693}, 1)))) {
                    (void)(px_method(_v692, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v693, _v694)}, 1)}, 1));
                     _v694 = px_add(_v694, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v696, px_str("px_spawn_name(\"")), _v675), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v692}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v692}, 1)}, 1)), px_str(");\n"));
            }
            return px_str("编译模式 spawn 仅支持直接函数调用（方法调用请用 `lx run`）");
        }
        return px_str("编译模式 spawn 仅支持函数调用表达式");
    }
    if (px_is_truthy(px_eq(_v661, px_str("Select")))) {
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v659, px_int(1LL)), px_index(_v659, px_int(2LL)), _v660}, 3);
    }
    if (px_is_truthy(px_eq(_v661, px_str("Import")))) {
        return px_add(_v696, px_str("/* import 忽略（MVP） */\n"));
    }
    return px_str("");
px_err_695:
    if (px_err_695_proped) return px_err_695_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v701 = (nargs > 0) ? args[0] : px_null();
    LXValue _v702 = (nargs > 1) ? args[1] : px_null();
    LXValue _v703 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_704_val = px_null();
    int px_err_704_proped = 0;
    if (px_is_truthy(px_eq(_v701, px_str("Assign")))) {
        return _v703;
    }
    if (px_is_truthy(px_eq(_v701, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v701, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v702), px_str("\"), ")), _v703), px_str(")"));
    }
    return _v703;
px_err_704:
    if (px_err_704_proped) return px_err_704_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v705 = (nargs > 0) ? args[0] : px_null();
    LXValue _v706 = (nargs > 1) ? args[1] : px_null();
    LXValue _v707 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_708_val = px_null();
    int px_err_708_proped = 0;
    if (px_is_truthy(px_eq(_v705, px_str("Assign")))) {
        return _v707;
    }
    if (px_is_truthy(px_eq(_v705, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v705, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v706), px_str(", ")), _v707), px_str(")"));
    }
    return _v707;
px_err_708:
    if (px_err_708_proped) return px_err_708_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v709 = (nargs > 0) ? args[0] : px_null();
    LXValue _v710 = (nargs > 1) ? args[1] : px_null();
    LXValue _v711 = (nargs > 2) ? args[2] : px_null();
    LXValue _v712 = px_null();
    LXValue _v713 = px_null();
    LXValue _v714 = px_null();
    LXValue _v715 = px_null();
    LXValue _v716 = px_null();
    LXValue _v717 = px_null();
    LXValue _v718 = px_null();
    LXValue _v719 = px_null();
    LXValue _v720 = px_null();
    LXValue _v721 = px_null();
    LXValue _v722 = px_null();
    LXValue _v723 = px_null();
    LXValue _v724 = px_null();
    LXValue _v725 = px_null();
    LXValue _v726 = px_null();
    LXValue _v727 = px_null();
    LXValue _v728 = px_null();
    LXValue _v729 = px_null();
    LXValue _v730 = px_null();
    LXValue _v731 = px_null();
    LXValue px_err_732_val = px_null();
    int px_err_732_proped = 0;
    LXValue _v733 = px_call(px_get_global("cg_pad"), (LXValue[]){_v711}, 1);
    _v712 = px_call(px_get_global("len"), (LXValue[]){_v709}, 1);
    if (px_is_truthy(px_eq(_v712, px_int(0LL)))) {
        return px_str("select 至少需要一个 case 分支");
    }
    _v713 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    _v714 = px_str("");
    _v715 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    _v716 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    _v717 = px_list_n((LXValue[]){}, 0);
    _v718 = px_int(0LL);
    while (px_is_truthy(px_lt(_v718, _v712))) {
        _v719 = px_index(px_index(_v709, _v718), px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Call")))) {
            _v720 = px_index(_v719, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v720, px_int(0LL)), px_str("Field")))) {
                _v721 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v720, px_int(2LL))}, 1);
                if (px_is_truthy(px_eq(_v721, px_str("recv")))) {
                    (void)(px_method(_v717, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v720, px_int(1LL))}, 1)}, 1));
                     _v718 = px_add(_v718, px_int(1LL));
                    continue;
                }
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v721), px_str("）"));
            }
            return px_str("select case 仅支持 ch.recv()");
        }
        return px_str("select case 仅支持 ch.recv()");
    }
     _v714 = px_add(_v714, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v733, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v712}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v717}, 2)), px_str("};\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(": {\n")));
    _v722 = px_list_n((LXValue[]){}, 0);
    _v723 = px_int(0LL);
    while (px_is_truthy(px_lt(_v723, _v712))) {
        (void)(px_method(_v722, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v723}, 1)}, 1));
         _v723 = px_add(_v723, px_int(1LL));
    }
     _v714 = px_add(_v714, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v733, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v712}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v722}, 2)), px_str("};\n")));
    if (px_is_truthy(px_gt(_v712, px_int(1LL)))) {
        _v724 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v733, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v712}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("--) { "));
         _v724 = px_add(_v724, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" + 1); ")));
         _v724 = px_add(_v724, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("; ")));
         _v724 = px_add(_v724, px_str("}\n"));
         _v714 = px_add(_v714, _v724);
    }
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = px_null();\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = -1;\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v733, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v712}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("++) {\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v733, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("];\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v733, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str("; break; }\n")));
     _v714 = px_add(_v714, px_add(_v733, px_str("    }\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" >= 0) {\n")));
    _v725 = px_int(0LL);
    while (px_is_truthy(px_lt(_v725, _v712))) {
        _v726 = px_index(px_index(_v709, _v725), px_int(0LL));
        _v727 = px_index(px_index(_v709, _v725), px_int(2LL));
        _v728 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v725}, 1)), px_str(")"));
        if (px_is_truthy(px_gt(_v725, px_int(0LL)))) {
             _v728 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v725}, 1)), px_str(")"));
        }
         _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("        ")), _v728), px_str(" {\n")));
        if (px_is_truthy(px_ne(_v726, px_null()))) {
            _v729 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v726}, 1)}, 1);
             _v714 = px_add(_v714, px_add(px_add(px_add(px_add(px_add(_v733, px_str("            LXValue ")), _v729), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(";\n")));
        }
        _v730 = px_int(0LL);
        while (px_is_truthy(px_lt(_v730, px_call(px_get_global("len"), (LXValue[]){_v727}, 1)))) {
             _v714 = px_add(_v714, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v727, _v730), px_add(_v711, px_int(3LL))}, 2));
             _v730 = px_add(_v730, px_int(1LL));
        }
         _v714 = px_add(_v714, px_add(_v733, px_str("        }\n")));
         _v725 = px_add(_v725, px_int(1LL));
    }
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(";\n")));
     _v714 = px_add(_v714, px_add(_v733, px_str("    }\n")));
    if (px_is_truthy(px_ne(_v710, px_null()))) {
         _v714 = px_add(_v714, px_add(_v733, px_str("    {\n")));
        _v731 = px_int(0LL);
        while (px_is_truthy(px_lt(_v731, px_call(px_get_global("len"), (LXValue[]){_v710}, 1)))) {
             _v714 = px_add(_v714, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v710, _v731), px_add(_v711, px_int(2LL))}, 2));
             _v731 = px_add(_v731, px_int(1LL));
        }
         _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(";\n")));
         _v714 = px_add(_v714, px_add(_v733, px_str("    }\n")));
    }
     _v714 = px_add(_v714, px_add(_v733, px_str("    px_select_wait();\n")));
     _v714 = px_add(_v714, px_add(_v733, px_str("}\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(";\n")));
     _v714 = px_add(_v714, px_add(px_add(px_add(_v733, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v713}, 1)), px_str(": ;\n")));
    px_set_global("cg_vars", _v715);
    px_set_global("cg_var_types", _v716);
    return _v714;
px_err_732:
    if (px_err_732_proped) return px_err_732_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v734 = (nargs > 0) ? args[0] : px_null();
    LXValue _v735 = px_null();
    LXValue _v736 = px_null();
    LXValue _v737 = px_null();
    LXValue _v738 = px_null();
    LXValue _v739 = px_null();
    LXValue _v740 = px_null();
    LXValue _v741 = px_null();
    LXValue _v742 = px_null();
    LXValue _v743 = px_null();
    LXValue _v744 = px_null();
    LXValue px_err_745_val = px_null();
    int px_err_745_proped = 0;
    _v735 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v735, "remove", (LXValue[]){px_str("_")}, 1));
    px_index_set(_v735, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v735, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v735, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v735, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v735, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v735, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    _v736 = px_int(0LL);
    while (px_is_truthy(px_lt(_v736, px_call(px_get_global("len"), (LXValue[]){_v734}, 1)))) {
        _v737 = px_index(_v734, _v736);
        _v738 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v737, px_int(2LL))}, 1);
        (void)(px_method(px_index(_v735, px_str("its")), "append", (LXValue[]){_v738}, 1));
        (void)(px_method(px_index(_v735, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v735, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v735, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        _v739 = px_str("");
        _v740 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v737, px_int(1LL))}, 1), px_int(1LL)))) {
            _v741 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            _v742 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v737, px_int(1LL)), px_int(0LL))}, 1);
            _v743 = px_null();
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v742}, 1))) {
                 _v743 = px_index(px_get_global("cg_vars"), _v742);
            }
            px_index_set(px_get_global("cg_vars"), _v742, _v741);
            (void)(px_method(_v740, "append", (LXValue[]){px_list_n((LXValue[]){_v742, _v743}, 2)}, 1));
             _v739 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v741), px_str(" = ")), px_index(px_index(_v735, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v735, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            _v744 = px_int(0LL);
            while (px_is_truthy(px_lt(_v744, px_call(px_get_global("len"), (LXValue[]){px_index(_v737, px_int(1LL))}, 1)))) {
                _v742 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v737, px_int(1LL)), _v744)}, 1);
                _v741 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v744}, 1));
                _v743 = px_null();
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v742}, 1))) {
                     _v743 = px_index(px_get_global("cg_vars"), _v742);
                }
                px_index_set(px_get_global("cg_vars"), _v742, _v741);
                (void)(px_method(_v740, "append", (LXValue[]){px_list_n((LXValue[]){_v742, _v743}, 2)}, 1));
                 _v739 = px_add(_v739, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v741), px_str(" = px_index(")), px_index(px_index(_v735, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v735, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v744}, 1)), px_str(")); ")));
                 _v744 = px_add(_v744, px_int(1LL));
            }
        }
        (void)(px_method(px_index(_v735, px_str("binds")), "append", (LXValue[]){_v739}, 1));
        (void)(px_method(px_index(_v735, px_str("saved_all")), "append", (LXValue[]){_v740}, 1));
         _v736 = px_add(_v736, px_int(1LL));
    }
    return _v735;
px_err_745:
    if (px_err_745_proped) return px_err_745_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v746 = (nargs > 0) ? args[0] : px_null();
    LXValue _v747 = px_null();
    LXValue _v748 = px_null();
    LXValue _v749 = px_null();
    LXValue _v750 = px_null();
    LXValue _v751 = px_null();
    LXValue px_err_752_val = px_null();
    int px_err_752_proped = 0;
    _v747 = px_int(0LL);
    while (px_is_truthy(px_lt(_v747, px_call(px_get_global("len"), (LXValue[]){_v746}, 1)))) {
        _v748 = px_index(_v746, _v747);
        _v749 = px_int(0LL);
        while (px_is_truthy(px_lt(_v749, px_call(px_get_global("len"), (LXValue[]){_v748}, 1)))) {
            _v750 = px_index(px_index(_v748, _v749), px_int(0LL));
            _v751 = px_index(px_index(_v748, _v749), px_int(1LL));
            if (px_is_truthy(px_eq(_v751, px_null()))) {
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v750}, 1));
            }
            else {
                px_index_set(px_get_global("cg_vars"), _v750, _v751);
            }
             _v749 = px_add(_v749, px_int(1LL));
        }
         _v747 = px_add(_v747, px_int(1LL));
    }
px_err_752:
    if (px_err_752_proped) return px_err_752_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v753 = (nargs > 0) ? args[0] : px_null();
    LXValue _v754 = (nargs > 1) ? args[1] : px_null();
    LXValue _v755 = (nargs > 2) ? args[2] : px_null();
    LXValue _v756 = px_null();
    LXValue _v757 = px_null();
    LXValue _v758 = px_null();
    LXValue _v759 = px_null();
    LXValue px_err_760_val = px_null();
    int px_err_760_proped = 0;
    _v756 = px_str("");
    if (px_is_truthy(px_ne(_v754, px_null()))) {
         _v756 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v754), px_str(")) { ")), _v755), px_str("} "));
    }
    else {
         _v756 = _v755;
    }
    _v757 = px_call(px_get_global("len"), (LXValue[]){px_index(_v753, px_str("its"))}, 1);
    _v758 = px_sub(_v757, px_int(1LL));
    while (px_is_truthy(px_ge(_v758, px_int(0LL)))) {
        _v759 = px_str("");
        if (px_is_truthy(px_lt(px_add(_v758, px_int(1LL)), _v757))) {
             _v759 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v753, px_str("ivs")), px_add(_v758, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v753, px_str("its")), px_add(_v758, px_int(1LL)))), px_str("; "));
        }
         _v756 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v753, px_str("idxs")), _v758)), px_str("=0; ")), px_index(px_index(_v753, px_str("idxs")), _v758)), px_str("<px_len(")), px_index(px_index(_v753, px_str("ivs")), _v758)), px_str("); ")), px_index(px_index(_v753, px_str("idxs")), _v758)), px_str("++) { LXValue ")), px_index(px_index(_v753, px_str("itms")), _v758)), px_str(" = px_index(")), px_index(px_index(_v753, px_str("ivs")), _v758)), px_str(", px_int(")), px_index(px_index(_v753, px_str("idxs")), _v758)), px_str(")); ")), px_index(px_index(_v753, px_str("binds")), _v758)), _v759), _v756), px_str(" } "));
         _v758 = px_sub(_v758, px_int(1LL));
    }
    return _v756;
px_err_760:
    if (px_err_760_proped) return px_err_760_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v761 = (nargs > 0) ? args[0] : px_null();
    LXValue _v762 = px_null();
    LXValue _v763 = px_null();
    LXValue _v764 = px_null();
    LXValue _v765 = px_null();
    LXValue _v766 = px_null();
    LXValue _v767 = px_null();
    LXValue _v768 = px_null();
    LXValue _v769 = px_null();
    LXValue _v770 = px_null();
    LXValue _v771 = px_null();
    LXValue _v772 = px_null();
    LXValue _v773 = px_null();
    LXValue _v774 = px_null();
    LXValue _v775 = px_null();
    LXValue _v776 = px_null();
    LXValue _v777 = px_null();
    LXValue _v778 = px_null();
    LXValue _v779 = px_null();
    LXValue _v780 = px_null();
    LXValue _v781 = px_null();
    LXValue _v782 = px_null();
    LXValue _v783 = px_null();
    LXValue _v784 = px_null();
    LXValue _v785 = px_null();
    LXValue _v786 = px_null();
    LXValue _v787 = px_null();
    LXValue _v788 = px_null();
    LXValue _v789 = px_null();
    LXValue _v790 = px_null();
    LXValue _v791 = px_null();
    LXValue _v792 = px_null();
    LXValue _v793 = px_null();
    LXValue _v794 = px_null();
    LXValue _v795 = px_null();
    LXValue _v796 = px_null();
    LXValue _v797 = px_null();
    LXValue _v798 = px_null();
    LXValue _v799 = px_null();
    LXValue _v800 = px_null();
    LXValue _v801 = px_null();
    LXValue _v802 = px_null();
    LXValue _v803 = px_null();
    LXValue _v804 = px_null();
    LXValue _v805 = px_null();
    LXValue _v806 = px_null();
    LXValue _v807 = px_null();
    LXValue _v808 = px_null();
    LXValue _v809 = px_null();
    LXValue _v810 = px_null();
    LXValue _v811 = px_null();
    LXValue _v812 = px_null();
    LXValue _v813 = px_null();
    LXValue _v814 = px_null();
    LXValue _v815 = px_null();
    LXValue _v816 = px_null();
    LXValue _v817 = px_null();
    LXValue _v818 = px_null();
    LXValue _v819 = px_null();
    LXValue _v820 = px_null();
    LXValue _v821 = px_null();
    LXValue _v822 = px_null();
    LXValue _v823 = px_null();
    LXValue _v824 = px_null();
    LXValue _v825 = px_null();
    LXValue px_err_826_val = px_null();
    int px_err_826_proped = 0;
    _v762 = px_index(_v761, px_int(0LL));
    if (px_is_truthy(px_eq(_v762, px_str("Int")))) {
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1)), px_str("LL)"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Float")))) {
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Str")))) {
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1)}, 1)), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Bool")))) {
        if (px_is_truthy(px_index(_v761, px_int(1LL)))) {
            return px_str("px_bool(true)");
        }
        return px_str("px_bool(false)");
    }
    if (px_is_truthy(px_eq(_v762, px_str("Null")))) {
        return px_str("px_null()");
    }
    if (px_is_truthy(px_eq(_v762, px_str("List")))) {
        _v763 = px_list_n((LXValue[]){}, 0);
        _v764 = px_index(_v761, px_int(1LL));
        _v765 = px_int(0LL);
        while (px_is_truthy(px_lt(_v765, px_call(px_get_global("len"), (LXValue[]){_v764}, 1)))) {
            (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v764, _v765)}, 1)}, 1));
             _v765 = px_add(_v765, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Tuple")))) {
        _v763 = px_list_n((LXValue[]){}, 0);
        _v764 = px_index(_v761, px_int(1LL));
        _v765 = px_int(0LL);
        while (px_is_truthy(px_lt(_v765, px_call(px_get_global("len"), (LXValue[]){_v764}, 1)))) {
            (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v764, _v765)}, 1)}, 1));
             _v765 = px_add(_v765, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Dict")))) {
        _v766 = px_str("({ LXValue _d = px_dict(); ");
        _v767 = px_index(_v761, px_int(1LL));
        _v765 = px_int(0LL);
        while (px_is_truthy(px_lt(_v765, px_call(px_get_global("len"), (LXValue[]){_v767}, 1)))) {
            _v768 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v767, _v765), px_int(0LL))}, 1);
            _v769 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v767, _v765), px_int(1LL))}, 1);
             _v766 = px_add(_v766, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v768), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v769), px_str("); } ")));
             _v765 = px_add(_v765, px_int(1LL));
        }
         _v766 = px_add(_v766, px_str("_d; })"));
        return _v766;
    }
    if (px_is_truthy(px_eq(_v762, px_str("Var")))) {
        _v770 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v771 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v770}, 1);
        if (px_is_truthy(px_ne(_v771, px_null()))) {
            return _v771;
        }
        return px_add(px_add(px_str("px_get_global(\""), _v770), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Field")))) {
        _v772 = px_index(_v761, px_int(1LL));
        _v773 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        if (px_is_truthy(px_eq(px_index(_v772, px_int(0LL)), px_str("Var")))) {
            _v774 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
            if (px_is_truthy(({ LXValue _t827 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v774}, 1); px_is_truthy(_t827) ? px_method(px_index(px_get_global("cg_const_enums"), _v774), "has", (LXValue[]){_v773}, 1) : _t827; }))) {
                return px_index(px_index(px_get_global("cg_const_enums"), _v774), _v773);
            }
        }
        if (px_is_truthy(px_eq(px_index(_v772, px_int(0LL)), px_str("Var")))) {
            _v774 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v774}, 1))) {
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v774), px_str("\", \"")), _v773), px_str("\")"));
            }
        }
        _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v772}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v775), px_str(", \"")), _v773), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("OptionalField")))) {
        _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v773 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v775), px_str("; px_is_null(")), _v776), px_str(") ? px_null() : px_field(")), _v775), px_str(", \"")), _v773), px_str("\"); })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Index")))) {
        _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v765 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v775), px_str(", ")), _v765), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Slice")))) {
        _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v766 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v761, px_int(2LL)), px_null()))) {
             _v766 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        }
        _v777 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v761, px_int(3LL)), px_null()))) {
             _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
        }
        _v778 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v761, px_int(4LL)), px_null()))) {
             _v778 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(4LL))}, 1);
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v775), px_str(", ")), _v766), px_str(", ")), _v777), px_str(", ")), _v778), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Call")))) {
        _v779 = px_index(_v761, px_int(1LL));
        _v780 = px_index(_v761, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v779, px_int(0LL)), px_str("Var")))) {
            _v781 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v779, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v781}, 1))) {
                _v782 = px_index(px_get_global("cg_ffi"), _v781);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v780}, 1), px_call(px_get_global("len"), (LXValue[]){_v782}, 1)))) {
                    (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), px_add(px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v781), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v782}, 1)}, 1)), px_str(" 个参数，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v780}, 1)}, 1))}, 2));
                }
                _v763 = px_list_n((LXValue[]){}, 0);
                _v783 = px_int(0LL);
                while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                    (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
                     _v783 = px_add(_v783, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v781), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")}, 2)"));
            }
            if (px_is_truthy(px_eq(_v781, px_str("chan")))) {
                _v784 = px_str("0");
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v780}, 1), px_int(0LL)))) {
                    if (px_is_truthy(px_eq(px_index(px_index(_v780, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                         _v784 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v780, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                         _v784 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                return px_add(px_add(px_str("px_chan_create("), _v784), px_str(")"));
            }
            if (px_is_truthy(px_eq(_v781, px_str("mutex")))) {
                return px_str("px_mutex_create()");
            }
            if (px_is_truthy(px_eq(_v781, px_str("rwlock")))) {
                return px_str("px_rwlock_create()");
            }
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v781}, 1))) {
                _v785 = px_index(px_get_global("cg_structs"), _v781);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v785}, 1), px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v781), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v785}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v780}, 1)}, 1));
                }
                _v763 = px_list_n((LXValue[]){}, 0);
                _v783 = px_int(0LL);
                while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                    (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
                     _v783 = px_add(_v783, px_int(1LL));
                }
                _v786 = px_list_n((LXValue[]){}, 0);
                _v787 = px_int(0LL);
                while (px_is_truthy(px_lt(_v787, px_call(px_get_global("len"), (LXValue[]){_v785}, 1)))) {
                    (void)(px_method(_v786, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v785, _v787)), px_str("\""))}, 1));
                     _v787 = px_add(_v787, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v781), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v786}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v785}, 1)}, 1)), px_str(")"));
            }
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v781}, 1))) {
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v780}, 1), px_int(1LL)))) {
                    return px_add(px_add(px_str("枚举 "), _v781), px_str(" 构造需要一个变体名"));
                }
                _v771 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, px_int(0LL))}, 1);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v781), px_str("\", (")), _v771), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        if (px_is_truthy(px_eq(px_index(_v779, px_int(0LL)), px_str("Field")))) {
            _v772 = px_index(_v779, px_int(1LL));
            _v788 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v779, px_int(2LL))}, 1);
            _v789 = px_null();
            if (px_is_truthy(px_eq(px_index(_v772, px_int(0LL)), px_str("Var")))) {
                _v774 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v774}, 1))) {
                     _v789 = px_index(px_get_global("cg_var_types"), _v774);
                }
            }
            if (px_is_truthy(({ LXValue _t828 = px_ne(_v789, px_null()); px_is_truthy(_t828) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v789}, 1) : _t828; }))) {
                _v790 = px_index(px_get_global("cg_impls"), _v789);
                _v791 = px_bool(false);
                _v792 = px_int(0LL);
                while (px_is_truthy(px_lt(_v792, px_call(px_get_global("len"), (LXValue[]){_v790}, 1)))) {
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v790, _v792), px_int(1LL))}, 1), _v788))) {
                         _v791 = px_bool(true);
                        break;
                    }
                     _v792 = px_add(_v792, px_int(1LL));
                }
                if (px_is_truthy(_v791)) {
                    _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v772}, 1);
                    _v763 = px_list_n((LXValue[]){_v775}, 1);
                    _v783 = px_int(0LL);
                    while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                        (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
                         _v783 = px_add(_v783, px_int(1LL));
                    }
                    _v793 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v789}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v788}, 1));
                    return px_add(px_add(px_add(px_add(px_add(_v793, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v772}, 1);
            _v763 = px_list_n((LXValue[]){}, 0);
            _v783 = px_int(0LL);
            while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
                 _v783 = px_add(_v783, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v775), px_str(", \"")), _v788), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")"));
        }
        _v794 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v779}, 1);
        _v763 = px_list_n((LXValue[]){}, 0);
        _v783 = px_int(0LL);
        while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
            (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
             _v783 = px_add(_v783, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v794), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Unary")))) {
        _v775 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        _v795 = px_index(_v761, px_int(1LL));
        if (px_is_truthy(px_eq(_v795, px_str("Neg")))) {
            return px_add(px_add(px_str("px_neg("), _v775), px_str(")"));
        }
        if (px_is_truthy(px_eq(_v795, px_str("Not")))) {
            return px_add(px_add(px_str("px_not("), _v775), px_str(")"));
        }
        return px_add(px_add(px_str("px_bitnot("), _v775), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Binary")))) {
        _v796 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        _v797 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
        _v795 = px_index(_v761, px_int(1LL));
        if (px_is_truthy(px_eq(_v795, px_str("And")))) {
            _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v796), px_str("; px_is_truthy(")), _v776), px_str(") ? ")), _v797), px_str(" : ")), _v776), px_str("; })"));
        }
        if (px_is_truthy(px_eq(_v795, px_str("Or")))) {
            _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v796), px_str("; px_is_truthy(")), _v776), px_str(") ? ")), _v776), px_str(" : ")), _v797), px_str("; })"));
        }
        _v798 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v795}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v798, px_str("(")), _v796), px_str(", ")), _v797), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Pipe")))) {
        _v771 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v799 = px_index(_v761, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v799, px_int(0LL)), px_str("Call")))) {
            _v794 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v799, px_int(1LL))}, 1);
            _v763 = px_list_n((LXValue[]){_v771}, 1);
            _v783 = px_int(0LL);
            while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){px_index(_v799, px_int(2LL))}, 1)))) {
                (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v799, px_int(2LL)), _v783)}, 1)}, 1));
                 _v783 = px_add(_v783, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v794), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")"));
        }
        _v798 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v799}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v798), px_str(", (LXValue[]){")), _v771), px_str("}, 1)"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("NullCoalesce")))) {
        _v796 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v797 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v796), px_str("; px_is_null(")), _v776), px_str(") ? ")), _v797), px_str(" : ")), _v776), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Try")))) {
        _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            _v800 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v777), px_str("; if (px_is_result(")), _v776), px_str(")) { if (!px_result_ok(")), _v776), px_str(")) { ")), _v800), px_str("_val = ")), _v776), px_str("; ")), _v800), px_str("_proped = 1; goto ")), _v800), px_str("; } ")), _v776), px_str(" = px_result_unwrap(")), _v776), px_str("); } else if (px_is_null(")), _v776), px_str(")) { ")), _v800), px_str("_val = px_null(); ")), _v800), px_str("_proped = 1; goto ")), _v800), px_str("; } ")), _v776), px_str("; })"));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v777), px_str("; if (px_is_result(")), _v776), px_str(") && !px_result_ok(")), _v776), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v776), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v776), px_str(")) ")), _v776), px_str(" = px_result_unwrap(")), _v776), px_str("); ")), _v776), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("ForceUnwrap")))) {
        _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v777), px_str("; if (px_is_result(")), _v776), px_str(")) { if (!px_result_ok(")), _v776), px_str(")) px_error(\"force unwrap Err\"); ")), _v776), px_str(" = px_result_unwrap(")), _v776), px_str("); } if (px_is_null(")), _v776), px_str(")) px_error(\"force unwrap null\"); ")), _v776), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("IfExpr")))) {
        _v794 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v801 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        _v802 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
        _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str("; if (px_is_truthy(")), _v794), px_str(")) { ")), _v776), px_str(" = ")), _v801), px_str("; } else { ")), _v776), px_str(" = ")), _v802), px_str("; } ")), _v776), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("ListComp")))) {
        _v803 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v797 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v804 = px_null();
        if (px_is_truthy(px_ne(px_index(_v761, px_int(3LL)), px_null()))) {
             _v804 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v797, px_str("saved_all"))}, 1));
        _v805 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v803), px_str(", ")), _v777), px_str("); "));
        _v806 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v797, _v804, _v805}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v803), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v797, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v797, px_str("its")), px_int(0LL))), px_str("; ")), _v806), px_str(" ")), _v803), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("GenExp")))) {
        _v807 = px_index(_v761, px_int(2LL));
        if (px_is_truthy(({ LXValue _t829 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v807}, 1), px_int(1LL)); px_is_truthy(_t829) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v807, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t829; }))) {
            _v808 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v807, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            _v809 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v807, px_int(0LL)), px_int(2LL))}, 1);
            _v810 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v808}, 1), px_index(_v761, px_int(1LL))}, 2);
            _v811 = px_str("px_null()");
            if (px_is_truthy(px_ne(px_index(_v761, px_int(3LL)), px_null()))) {
                 _v811 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v808}, 1), px_index(_v761, px_int(3LL))}, 2);
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v809), px_str(", ")), _v810), px_str(", ")), _v811), px_str(")"));
        }
        _v803 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v797 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v807}, 1);
        _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v804 = px_null();
        if (px_is_truthy(px_ne(px_index(_v761, px_int(3LL)), px_null()))) {
             _v804 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v797, px_str("saved_all"))}, 1));
        _v805 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v803), px_str(", ")), _v777), px_str("); "));
        _v806 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v797, _v804, _v805}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v803), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v797, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v797, px_str("its")), px_int(0LL))), px_str("; ")), _v806), px_str(" px_gen_from_list(")), _v803), px_str("); })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("DictComp")))) {
        _v803 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v797 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
        _v768 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v769 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(2LL))}, 1);
        _v804 = px_null();
        if (px_is_truthy(px_ne(px_index(_v761, px_int(4LL)), px_null()))) {
             _v804 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(4LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v797, px_str("saved_all"))}, 1));
        _v805 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v768), px_str("; LXValue _v = ")), _v769), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v803), px_str(", _k.as.obj->as.str.data, _v); } "));
        _v806 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v797, _v804, _v805}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v803), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v797, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v797, px_str("its")), px_int(0LL))), px_str("; ")), _v806), px_str(" ")), _v803), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Closure")))) {
        px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
        _v812 = px_get_global("cg_closure_id");
        _v793 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v812}, 1));
        _v813 = px_add(px_add(px_str("static LXValue "), _v793), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
         _v813 = px_add(_v813, px_str("    (void)ctx;\n"));
        _v814 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
        _v815 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
        px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        _v816 = px_index(_v761, px_int(1LL));
        _v817 = px_int(0LL);
        while (px_is_truthy(px_lt(_v817, px_call(px_get_global("len"), (LXValue[]){_v816}, 1)))) {
            _v771 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v816, _v817), px_int(1LL))}, 1)}, 1);
             _v813 = px_add(_v813, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v771), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v817}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v817}, 1)), px_str("] : px_null();\n")));
             _v817 = px_add(_v817, px_int(1LL));
        }
        _v802 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(3LL))}, 1);
         _v813 = px_add(_v813, px_add(px_add(px_str("    return "), _v802), px_str(";\n")));
         _v813 = px_add(_v813, px_str("}\n"));
        px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v813));
        px_set_global("cg_vars", _v814);
        px_set_global("cg_var_types", _v815);
        return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v812}, 1)), px_str(">\", ")), _v793), px_str(", NULL)"));
    }
    if (px_is_truthy(px_eq(_v762, px_str("Block")))) {
        _v766 = px_str("({ ");
         _v766 = px_add(_v766, px_str("LXValue _blk = px_null(); "));
        _v818 = px_index(_v761, px_int(1LL));
        _v819 = px_int(0LL);
        while (px_is_truthy(px_lt(_v819, px_call(px_get_global("len"), (LXValue[]){_v818}, 1)))) {
            _v820 = px_index(_v818, _v819);
            if (px_is_truthy(px_eq(px_index(_v820, px_int(0LL)), px_str("ExprStmt")))) {
                _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v820, px_int(1LL))}, 1);
                 _v766 = px_add(_v766, px_add(px_add(px_str("_blk = "), _v777), px_str("; ")));
            }
            else {
                 _v766 = px_add(_v766, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v820, px_int(0LL)}, 2));
            }
             _v819 = px_add(_v819, px_int(1LL));
        }
         _v766 = px_add(_v766, px_str("_blk; })"));
        return _v766;
    }
    if (px_is_truthy(px_eq(_v762, px_str("Match")))) {
        _v821 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v776 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        _v766 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v776), px_str(" = ")), _v821), px_str("; "));
        _v822 = px_index(_v761, px_int(2LL));
        _v823 = px_bool(true);
        _v783 = px_int(0LL);
        while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v822}, 1)))) {
            _v804 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v822, _v783), px_int(1LL)), _v776}, 2);
            _v824 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v822, _v783), px_int(3LL))}, 1);
            _v825 = px_str("if");
            if (px_is_truthy(px_not(_v823))) {
                 _v825 = px_str("else if");
            }
             _v766 = px_add(_v766, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v825, px_str(" (")), _v804), px_str(") { ")), _v776), px_str(" = ")), _v824), px_str("; } ")));
             _v823 = px_bool(false);
             _v783 = px_add(_v783, px_int(1LL));
        }
         _v766 = px_add(_v766, px_add(_v776, px_str("; })")));
        return _v766;
    }
    if (px_is_truthy(px_eq(_v762, px_str("Constructor")))) {
        _v770 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v761, px_int(1LL))}, 1);
        _v780 = px_index(_v761, px_int(2LL));
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v770}, 1))) {
            _v785 = px_index(px_get_global("cg_structs"), _v770);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v785}, 1), px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v770), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v785}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v780}, 1)}, 1));
            }
            _v763 = px_list_n((LXValue[]){}, 0);
            _v783 = px_int(0LL);
            while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
                (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
                 _v783 = px_add(_v783, px_int(1LL));
            }
            _v786 = px_list_n((LXValue[]){}, 0);
            _v787 = px_int(0LL);
            while (px_is_truthy(px_lt(_v787, px_call(px_get_global("len"), (LXValue[]){_v785}, 1)))) {
                (void)(px_method(_v786, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v785, _v787)), px_str("\""))}, 1));
                 _v787 = px_add(_v787, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v770), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v786}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v785}, 1)}, 1)), px_str(")"));
        }
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v770}, 1))) {
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v780}, 1), px_int(1LL)))) {
                return px_add(px_add(px_str("枚举 "), _v770), px_str(" 构造需要一个变体名"));
            }
            _v771 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, px_int(0LL))}, 1);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v770), px_str("\", (")), _v771), px_str(").as.obj->as.enum_inst.variant)"));
        }
        _v763 = px_list_n((LXValue[]){}, 0);
        _v783 = px_int(0LL);
        while (px_is_truthy(px_lt(_v783, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
            (void)(px_method(_v763, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v780, _v783)}, 1)}, 1));
             _v783 = px_add(_v783, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v770), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v763}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v763}, 1)}, 1)), px_str(")"));
    }
    return px_str("px_null()");
px_err_826:
    if (px_err_826_proped) return px_err_826_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v830 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_831_val = px_null();
    int px_err_831_proped = 0;
    if (px_is_truthy(px_eq(_v830, px_str("Add")))) {
        return px_str("px_add");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Sub")))) {
        return px_str("px_sub");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Mul")))) {
        return px_str("px_mul");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Div")))) {
        return px_str("px_div");
    }
    if (px_is_truthy(px_eq(_v830, px_str("IntDiv")))) {
        return px_str("px_idiv");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Mod")))) {
        return px_str("px_mod");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Pow")))) {
        return px_str("px_pow");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Eq")))) {
        return px_str("px_eq");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Ne")))) {
        return px_str("px_ne");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Lt")))) {
        return px_str("px_lt");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Le")))) {
        return px_str("px_le");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Gt")))) {
        return px_str("px_gt");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Ge")))) {
        return px_str("px_ge");
    }
    if (px_is_truthy(px_eq(_v830, px_str("BitAnd")))) {
        return px_str("px_bitand");
    }
    if (px_is_truthy(px_eq(_v830, px_str("BitOr")))) {
        return px_str("px_bitor");
    }
    if (px_is_truthy(px_eq(_v830, px_str("BitXor")))) {
        return px_str("px_bitxor");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Shl")))) {
        return px_str("px_shl");
    }
    if (px_is_truthy(px_eq(_v830, px_str("Shr")))) {
        return px_str("px_shr");
    }
    if (px_is_truthy(px_eq(_v830, px_str("ShrU")))) {
        return px_str("px_ushr");
    }
    return px_str("px_add");
px_err_831:
    if (px_err_831_proped) return px_err_831_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v832 = (nargs > 0) ? args[0] : px_null();
    LXValue _v833 = (nargs > 1) ? args[1] : px_null();
    LXValue _v834 = px_null();
    LXValue _v835 = px_null();
    LXValue _v836 = px_null();
    LXValue _v837 = px_null();
    LXValue px_err_838_val = px_null();
    int px_err_838_proped = 0;
    _v834 = px_index(_v832, px_int(0LL));
    if (px_is_truthy(px_eq(_v834, px_str("PatLiteral")))) {
        _v835 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v832, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v833), px_str(", ")), _v835), px_str("))"));
    }
    if (px_is_truthy(px_eq(_v834, px_str("PatBinding")))) {
        _v836 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v832, px_int(1LL))}, 1);
        if (px_is_truthy(({ LXValue _t840 = ({ LXValue _t839 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v836}, 1), px_int(0LL)); px_is_truthy(_t839) ? px_ge(px_index(_v836, px_int(0LL)), px_str("A")) : _t839; }); px_is_truthy(_t840) ? px_le(px_index(_v836, px_int(0LL)), px_str("Z")) : _t840; }))) {
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v833), px_str(".type == PX_ENUM && strcmp(")), _v833), px_str(".as.obj->as.enum_inst.variant, \"")), _v836), px_str("\") == 0)"));
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v834, px_str("PatWildcard")))) {
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v834, px_str("PatTuple")))) {
        _v837 = px_index(_v832, px_int(1LL));
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v837}, 1), px_int(0LL)))) {
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v837, px_int(0LL)), _v833}, 2);
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v834, px_str("PatConstructor")))) {
        _v836 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v832, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v833), px_str(".type == PX_ENUM && strcmp(")), _v833), px_str(".as.obj->as.enum_inst.variant, \"")), _v836), px_str("\") == 0)"));
    }
    return px_str("true");
px_err_838:
    if (px_err_838_proped) return px_err_838_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v841 = (nargs > 0) ? args[0] : px_null();
    LXValue _v842 = (nargs > 1) ? args[1] : px_null();
    LXValue _v843 = px_null();
    LXValue _v844 = px_null();
    LXValue _v845 = px_null();
    LXValue _v846 = px_null();
    LXValue _v847 = px_null();
    LXValue _v848 = px_null();
    LXValue _v849 = px_null();
    LXValue _v850 = px_null();
    LXValue _v851 = px_null();
    LXValue px_err_852_val = px_null();
    int px_err_852_proped = 0;
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    _v843 = px_get_global("cg_closure_id");
    _v844 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v843}, 1));
    _v845 = px_add(px_add(px_str("static LXValue "), _v844), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v845 = px_add(_v845, px_str("    (void)ctx;\n"));
    _v846 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    _v847 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    _v848 = px_int(0LL);
    while (px_is_truthy(px_lt(_v848, px_call(px_get_global("len"), (LXValue[]){_v841}, 1)))) {
        _v849 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v841, _v848)}, 1);
         _v845 = px_add(_v845, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v849), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v848}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v848}, 1)), px_str("] : px_null();\n")));
         _v848 = px_add(_v848, px_int(1LL));
    }
    _v850 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v842}, 1);
    _v851 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v851}, 1));
     _v845 = px_add(_v845, px_add(px_add(px_str("    LXValue "), _v851), px_str("_val = px_null();\n")));
     _v845 = px_add(_v845, px_add(px_add(px_str("    int "), _v851), px_str("_proped = 0;\n")));
     _v845 = px_add(_v845, px_add(px_add(px_str("    return "), _v850), px_str(";\n")));
     _v845 = px_add(_v845, px_add(_v851, px_str(":\n")));
     _v845 = px_add(_v845, px_add(px_add(px_add(px_add(px_str("    if ("), _v851), px_str("_proped) return ")), _v851), px_str("_val;\n")));
     _v845 = px_add(_v845, px_str("    return px_null();\n"));
     _v845 = px_add(_v845, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v845));
    px_set_global("cg_vars", _v846);
    px_set_global("cg_var_types", _v847);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v843}, 1)), px_str(">\", ")), _v844), px_str(", NULL)"));
px_err_852:
    if (px_err_852_proped) return px_err_852_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v853 = (nargs > 0) ? args[0] : px_null();
    LXValue _v854 = px_null();
    LXValue px_err_855_val = px_null();
    int px_err_855_proped = 0;
    _v854 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v853}, 1), px_int(1LL));
    while (px_is_truthy(px_ge(_v854, px_int(0LL)))) {
        if (px_is_truthy(px_eq(px_index(_v853, _v854), px_str("/")))) {
            if (px_is_truthy(px_eq(_v854, px_int(0LL)))) {
                return px_str("/");
            }
            return px_slice(_v853, px_int(0LL), _v854, px_null());
        }
         _v854 = px_sub(_v854, px_int(1LL));
    }
    return px_str(".");
px_err_855:
    if (px_err_855_proped) return px_err_855_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v856 = px_null();
    LXValue _v857 = px_null();
    LXValue _v858 = px_null();
    LXValue _v859 = px_null();
    LXValue px_err_860_val = px_null();
    int px_err_860_proped = 0;
    _v856 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    if (px_is_truthy(({ LXValue _t861 = px_ne(_v856, px_null()); px_is_truthy(_t861) ? px_call(px_get_global("exists"), (LXValue[]){_v856}, 1) : _t861; }))) {
        return _v856;
    }
    _v857 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    _v858 = px_int(0LL);
    while (px_is_truthy(px_lt(_v858, px_call(px_get_global("len"), (LXValue[]){_v857}, 1)))) {
        _v859 = px_index(_v857, _v858);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v859}, 1))) {
            return _v859;
        }
         _v858 = px_add(_v858, px_int(1LL));
    }
    return px_null();
px_err_860:
    if (px_err_860_proped) return px_err_860_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v862 = (nargs > 0) ? args[0] : px_null();
    LXValue _v863 = (nargs > 1) ? args[1] : px_null();
    LXValue _v864 = px_null();
    LXValue _v865 = px_null();
    LXValue _v866 = px_null();
    LXValue _v867 = px_null();
    LXValue _v868 = px_null();
    LXValue _v869 = px_null();
    LXValue _v870 = px_null();
    LXValue _v871 = px_null();
    LXValue _v872 = px_null();
    LXValue _v873 = px_null();
    LXValue _v874 = px_null();
    LXValue _v875 = px_null();
    LXValue _v876 = px_null();
    LXValue _v877 = px_null();
    LXValue _v878 = px_null();
    LXValue px_err_879_val = px_null();
    int px_err_879_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v862}, 1), px_int(0LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t881 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v862}, 1), px_int(1LL)); px_is_truthy(_t881) ? ({ LXValue _t880 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v862, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t880) ? _t880 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v862, px_int(0LL)), px_str(".px")}, 2); }) : _t881; }))) {
        _v864 = px_index(_v862, px_int(0LL));
        _v865 = _v864;
        if (px_is_truthy(px_not(({ LXValue _t882 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v864}, 1), px_int(0LL)); px_is_truthy(_t882) ? px_eq(px_index(_v864, px_int(0LL)), px_str("/")) : _t882; })))) {
             _v865 = px_add(px_add(_v863, px_str("/")), _v864);
        }
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v865}, 1))) {
            return _v865;
        }
        return px_null();
    }
    if (px_is_truthy(px_eq(px_index(_v862, px_int(0LL)), px_str("std")))) {
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v862}, 1), px_int(2LL)))) {
            return px_null();
        }
        _v866 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v866, px_null()))) {
            return px_null();
        }
        _v865 = _v866;
        _v867 = px_int(1LL);
        while (px_is_truthy(px_lt(_v867, px_call(px_get_global("len"), (LXValue[]){_v862}, 1)))) {
             _v865 = px_add(_v865, px_add(px_str("/"), px_index(_v862, _v867)));
             _v867 = px_add(_v867, px_int(1LL));
        }
        _v868 = px_add(_v865, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v868}, 1))) {
            return _v868;
        }
        _v869 = px_add(_v865, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v869}, 1))) {
            return _v869;
        }
        return px_null();
    }
    _v870 = px_list_n((LXValue[]){_v863}, 1);
    _v871 = px_add(_v863, px_str("/.px_modules"));
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v871}, 1))) {
        (void)(px_method(_v870, "append", (LXValue[]){_v871}, 1));
        _v872 = px_call(px_get_global("list_dir"), (LXValue[]){_v871}, 1);
        _v873 = px_int(0LL);
        while (px_is_truthy(px_lt(_v873, px_call(px_get_global("len"), (LXValue[]){_v872}, 1)))) {
            _v874 = px_index(_v872, _v873);
            _v875 = px_add(px_add(_v871, px_str("/")), _v874);
            if (px_is_truthy(({ LXValue _t883 = px_call(px_get_global("exists"), (LXValue[]){_v875}, 1); px_is_truthy(_t883) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v874, px_str(".")}, 2)) : _t883; }))) {
                (void)(px_method(_v870, "append", (LXValue[]){_v875}, 1));
            }
             _v873 = px_add(_v873, px_int(1LL));
        }
    }
    _v876 = px_int(0LL);
    while (px_is_truthy(px_lt(_v876, px_call(px_get_global("len"), (LXValue[]){_v870}, 1)))) {
        _v877 = px_index(_v870, _v876);
        _v865 = _v877;
        _v867 = px_int(0LL);
        while (px_is_truthy(px_lt(_v867, px_call(px_get_global("len"), (LXValue[]){_v862}, 1)))) {
             _v865 = px_add(_v865, px_add(px_str("/"), px_index(_v862, _v867)));
             _v867 = px_add(_v867, px_int(1LL));
        }
        _v868 = px_add(_v865, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v868}, 1))) {
            return _v868;
        }
        _v869 = px_add(_v865, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v869}, 1))) {
            return _v869;
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v862}, 1), px_int(1LL)))) {
            _v878 = px_add(px_add(px_add(_v877, px_str("/")), px_index(_v862, px_int(0LL))), px_str(".px"));
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v878}, 1))) {
                return _v878;
            }
        }
         _v876 = px_add(_v876, px_int(1LL));
    }
    return px_null();
px_err_879:
    if (px_err_879_proped) return px_err_879_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v884 = (nargs > 0) ? args[0] : px_null();
    LXValue _v885 = px_null();
    LXValue px_err_886_val = px_null();
    int px_err_886_proped = 0;
    _v885 = px_index(_v884, px_int(0LL));
    if (px_is_truthy(px_eq(_v885, px_str("FuncDef")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v884, px_int(1LL))}, 1), px_str("main")))) {
            return px_bool(false);
        }
        return px_bool(true);
    }
    if (px_is_truthy(px_eq(_v885, px_str("ExternDef")))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t889 = ({ LXValue _t888 = ({ LXValue _t887 = px_eq(_v885, px_str("StructDef")); px_is_truthy(_t887) ? _t887 : px_eq(_v885, px_str("EnumDef")); }); px_is_truthy(_t888) ? _t888 : px_eq(_v885, px_str("TraitDef")); }); px_is_truthy(_t889) ? _t889 : px_eq(_v885, px_str("ImplDef")); }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t890 = px_eq(_v885, px_str("VarDecl")); px_is_truthy(_t890) ? px_eq(px_index(_v884, px_int(1LL)), px_str("Const")) : _t890; }))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_886:
    if (px_err_886_proped) return px_err_886_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v891 = (nargs > 0) ? args[0] : px_null();
    LXValue _v892 = px_null();
    LXValue _v893 = px_null();
    LXValue _v894 = px_null();
    LXValue _v895 = px_null();
    LXValue px_err_896_val = px_null();
    int px_err_896_proped = 0;
    _v892 = px_index(_v891, px_int(0LL));
    if (px_is_truthy(({ LXValue _t900 = ({ LXValue _t899 = ({ LXValue _t898 = ({ LXValue _t897 = px_eq(_v892, px_str("FuncDef")); px_is_truthy(_t897) ? _t897 : px_eq(_v892, px_str("StructDef")); }); px_is_truthy(_t898) ? _t898 : px_eq(_v892, px_str("EnumDef")); }); px_is_truthy(_t899) ? _t899 : px_eq(_v892, px_str("TraitDef")); }); px_is_truthy(_t900) ? _t900 : px_eq(_v892, px_str("ExternDef")); }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v891, px_int(1LL))}, 1);
    }
    if (px_is_truthy(({ LXValue _t901 = px_eq(_v892, px_str("VarDecl")); px_is_truthy(_t901) ? px_eq(px_index(_v891, px_int(1LL)), px_str("Const")) : _t901; }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v891, px_int(2LL))}, 1);
    }
    if (px_is_truthy(px_eq(_v892, px_str("ImplDef")))) {
        _v893 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v891, px_int(1LL))}, 1);
        _v894 = px_index(_v891, px_int(2LL));
        _v895 = px_str("None");
        if (px_is_truthy(px_ne(_v894, px_null()))) {
             _v895 = px_add(px_add(px_str("Some("), _v894), px_str(")"));
        }
        return px_add(px_add(px_add(px_str("impl::"), _v893), px_str("::")), _v895);
    }
    return px_null();
px_err_896:
    if (px_err_896_proped) return px_err_896_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v902 = (nargs > 0) ? args[0] : px_null();
    LXValue _v903 = (nargs > 1) ? args[1] : px_null();
    LXValue _v904 = (nargs > 2) ? args[2] : px_null();
    LXValue _v905 = (nargs > 3) ? args[3] : px_null();
    LXValue _v906 = (nargs > 4) ? args[4] : px_null();
    LXValue _v907 = px_null();
    LXValue _v908 = px_null();
    LXValue _v909 = px_null();
    LXValue _v910 = px_null();
    LXValue _v911 = px_null();
    LXValue _v912 = px_null();
    LXValue _v913 = px_null();
    LXValue _v914 = px_null();
    LXValue _v915 = px_null();
    LXValue _v916 = px_null();
    LXValue _v917 = px_null();
    LXValue _v918 = px_null();
    LXValue _v919 = px_null();
    LXValue _v920 = px_null();
    LXValue _v921 = px_null();
    LXValue _v922 = px_null();
    LXValue _v923 = px_null();
    LXValue px_err_924_val = px_null();
    int px_err_924_proped = 0;
    _v907 = px_list_n((LXValue[]){}, 0);
    _v908 = px_int(0LL);
    while (px_is_truthy(px_lt(_v908, px_call(px_get_global("len"), (LXValue[]){_v902}, 1)))) {
        (void)(px_method(_v907, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v902, _v908)}, 1)}, 1));
         _v908 = px_add(_v908, px_int(1LL));
    }
     _v902 = _v907;
    _v909 = px_list_n((LXValue[]){}, 0);
    _v910 = px_int(0LL);
    while (px_is_truthy(px_lt(_v910, px_call(px_get_global("len"), (LXValue[]){_v903}, 1)))) {
        (void)(px_method(_v909, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v903, _v910)}, 1)}, 1));
         _v910 = px_add(_v910, px_int(1LL));
    }
     _v903 = _v909;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v902}, 1), px_int(0LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t927 = ({ LXValue _t926 = ({ LXValue _t925 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v902}, 1), px_int(1LL)); px_is_truthy(_t925) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v902, px_int(0LL))}, 1), px_int(2LL)) : _t925; }); px_is_truthy(_t926) ? px_eq(px_slice(px_index(_v902, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t926; }); px_is_truthy(_t927) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v902, px_int(0LL)), px_str(".px")}, 2)) : _t927; }))) {
        return px_null();
    }
    _v911 = px_eq(px_index(_v902, px_int(0LL)), px_str("std"));
    _v912 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v902}, 2);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v912}, 1))) {
        return px_null();
    }
    _v913 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v902, _v904}, 2);
    if (px_is_truthy(px_eq(_v913, px_null()))) {
        if (px_is_truthy(px_ne(px_index(_v902, px_int(0LL)), px_str("std")))) {
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("[module] 警告: 找不到模块 '"), _v912), px_str("'（已跳过）"))}, 1));
        }
        return px_null();
    }
    px_index_set(px_get_global("loaded"), _v912, _v913);
    _v914 = px_call(px_get_global("read_file"), (LXValue[]){_v913}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v914}, 1));
    px_set_global("p_pos", px_int(0LL));
    _v915 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    _v916 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v913}, 1);
    _v917 = px_list_n((LXValue[]){}, 0);
    _v918 = px_int(0LL);
    while (px_is_truthy(px_lt(_v918, px_call(px_get_global("len"), (LXValue[]){px_index(_v915, px_int(1LL))}, 1)))) {
        _v919 = px_index(px_index(_v915, px_int(1LL)), _v918);
        if (px_is_truthy(px_eq(px_index(_v919, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v917, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v919, px_int(1LL)), px_index(_v919, px_int(2LL))}, 2)}, 1));
        }
         _v918 = px_add(_v918, px_int(1LL));
    }
    _v920 = px_int(0LL);
    while (px_is_truthy(px_lt(_v920, px_call(px_get_global("len"), (LXValue[]){_v917}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v917, _v920), px_int(0LL)), px_index(px_index(_v917, _v920), px_int(1LL)), _v916, _v905, _v906}, 5));
         _v920 = px_add(_v920, px_int(1LL));
    }
    _v921 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v903}, 1), px_int(0LL));
    _v922 = px_int(0LL);
    while (px_is_truthy(px_lt(_v922, px_call(px_get_global("len"), (LXValue[]){px_index(_v915, px_int(1LL))}, 1)))) {
        _v919 = px_index(px_index(_v915, px_int(1LL)), _v922);
        if (px_is_truthy(px_eq(px_index(_v919, px_int(0LL)), px_str("Import")))) {
             _v922 = px_add(_v922, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v919}, 1)))) {
             _v922 = px_add(_v922, px_int(1LL));
            continue;
        }
        _v923 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v919}, 1);
        if (px_is_truthy(px_eq(_v923, px_null()))) {
            (void)(px_method(_v905, "append", (LXValue[]){_v919}, 1));
        }
        else {
            if (px_is_truthy(_v921)) {
                if (px_is_truthy(({ LXValue _t928 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v923}, 1), px_int(5LL)); px_is_truthy(_t928) ? px_eq(px_slice(_v923, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t928; }))) {
                     _v922 = px_add(_v922, px_int(1LL));
                    continue;
                }
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v903, _v923}, 2)))) {
                     _v922 = px_add(_v922, px_int(1LL));
                    continue;
                }
            }
            if (px_is_truthy(_v911)) {
                if (px_is_truthy(px_method(_v906, "has", (LXValue[]){_v923}, 1))) {
                     _v922 = px_add(_v922, px_int(1LL));
                    continue;
                }
                px_index_set(_v906, _v923, px_bool(true));
            }
            (void)(px_method(_v905, "append", (LXValue[]){_v919}, 1));
        }
         _v922 = px_add(_v922, px_int(1LL));
    }
px_err_924:
    if (px_err_924_proped) return px_err_924_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v929 = (nargs > 0) ? args[0] : px_null();
    LXValue _v930 = (nargs > 1) ? args[1] : px_null();
    LXValue _v931 = px_null();
    LXValue _v932 = px_null();
    LXValue _v933 = px_null();
    LXValue _v934 = px_null();
    LXValue _v935 = px_null();
    LXValue _v936 = px_null();
    LXValue _v937 = px_null();
    LXValue _v938 = px_null();
    LXValue _v939 = px_null();
    LXValue _v940 = px_null();
    LXValue px_err_941_val = px_null();
    int px_err_941_proped = 0;
    _v931 = px_index(_v929, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v931}, 1), px_int(0LL)))) {
        return _v929;
    }
    _v932 = px_list_n((LXValue[]){}, 0);
    _v933 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    _v934 = px_list_n((LXValue[]){}, 0);
    _v935 = px_int(0LL);
    while (px_is_truthy(px_lt(_v935, px_call(px_get_global("len"), (LXValue[]){_v931}, 1)))) {
        _v936 = px_index(_v931, _v935);
        if (px_is_truthy(px_eq(px_index(_v936, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v934, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v936, px_int(1LL)), px_index(_v936, px_int(2LL))}, 2)}, 1));
        }
         _v935 = px_add(_v935, px_int(1LL));
    }
    _v937 = px_int(0LL);
    while (px_is_truthy(px_lt(_v937, px_call(px_get_global("len"), (LXValue[]){_v934}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v934, _v937), px_int(0LL)), px_index(px_index(_v934, _v937), px_int(1LL)), _v930, _v932, _v933}, 5));
         _v937 = px_add(_v937, px_int(1LL));
    }
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v932}, 1), px_int(0LL)))) {
        return _v929;
    }
    _v938 = px_list_n((LXValue[]){}, 0);
    _v939 = px_int(0LL);
    while (px_is_truthy(px_lt(_v939, px_call(px_get_global("len"), (LXValue[]){_v932}, 1)))) {
        (void)(px_method(_v938, "append", (LXValue[]){px_index(_v932, _v939)}, 1));
         _v939 = px_add(_v939, px_int(1LL));
    }
    _v940 = px_int(0LL);
    while (px_is_truthy(px_lt(_v940, px_call(px_get_global("len"), (LXValue[]){_v931}, 1)))) {
        (void)(px_method(_v938, "append", (LXValue[]){px_index(_v931, _v940)}, 1));
         _v940 = px_add(_v940, px_int(1LL));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v938}, 2);
px_err_941:
    if (px_err_941_proped) return px_err_941_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v942 = px_null();
    LXValue px_err_943_val = px_null();
    int px_err_943_proped = 0;
    _v942 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v942, "remove", (LXValue[]){px_str("_")}, 1));
    return _v942;
px_err_943:
    if (px_err_943_proped) return px_err_943_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v944 = (nargs > 0) ? args[0] : px_null();
    LXValue _v945 = px_null();
    LXValue _v946 = px_null();
    LXValue _v947 = px_null();
    LXValue px_err_948_val = px_null();
    int px_err_948_proped = 0;
    _v945 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    _v946 = px_method(_v944, "keys", (LXValue[]){}, 0);
    _v947 = px_int(0LL);
    while (px_is_truthy(px_lt(_v947, px_call(px_get_global("len"), (LXValue[]){_v946}, 1)))) {
        px_index_set(_v945, px_index(_v946, _v947), px_index(_v944, px_index(_v946, _v947)));
         _v947 = px_add(_v947, px_int(1LL));
    }
    return _v945;
px_err_948:
    if (px_err_948_proped) return px_err_948_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_949_val = px_null();
    int px_err_949_proped = 0;
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    return px_get_global("cg_uidc");
px_err_949:
    if (px_err_949_proped) return px_err_949_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_950_val = px_null();
    int px_err_950_proped = 0;
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_950:
    if (px_err_950_proped) return px_err_950_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v951 = (nargs > 0) ? args[0] : px_null();
    LXValue _v952 = px_null();
    LXValue px_err_953_val = px_null();
    int px_err_953_proped = 0;
    _v952 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_index_set(px_get_global("cg_vars"), _v951, _v952);
    return _v952;
px_err_953:
    if (px_err_953_proped) return px_err_953_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v954 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_955_val = px_null();
    int px_err_955_proped = 0;
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v954}, 1))) {
        return px_index(px_get_global("cg_vars"), _v954);
    }
    return px_null();
px_err_955:
    if (px_err_955_proped) return px_err_955_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v956 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_957_val = px_null();
    int px_err_957_proped = 0;
    px_index_set(px_get_global("cg_immutables"), _v956, px_int(1LL));
px_err_957:
    if (px_err_957_proped) return px_err_957_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v958 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_959_val = px_null();
    int px_err_959_proped = 0;
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v958}, 1);
px_err_959:
    if (px_err_959_proped) return px_err_959_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v960 = (nargs > 0) ? args[0] : px_null();
    LXValue _v961 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_962_val = px_null();
    int px_err_962_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v960), px_str(": ")), _v961)}, 1));
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_962:
    if (px_err_962_proped) return px_err_962_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v963 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_964_val = px_null();
    int px_err_964_proped = 0;
    if (px_is_truthy(px_eq(_v963, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(px_index(_v963, px_int(0LL)), px_str("TyOptional")))) {
        return px_bool(false);
    }
    return px_bool(true);
px_err_964:
    if (px_err_964_proped) return px_err_964_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v965 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_966_val = px_null();
    int px_err_966_proped = 0;
    if (px_is_truthy(px_eq(_v965, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(px_index(_v965, px_int(0LL)), px_str("Null")))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_966:
    if (px_err_966_proped) return px_err_966_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v967 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_968_val = px_null();
    int px_err_968_proped = 0;
    if (px_is_truthy(px_eq(_v967, px_null()))) {
        return px_str("any");
    }
    if (px_is_truthy(px_eq(px_index(_v967, px_int(0LL)), px_str("TyOptional")))) {
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v967, px_int(1LL))}, 1), px_str("?"));
    }
    if (px_is_truthy(px_eq(px_index(_v967, px_int(0LL)), px_str("TyNamed")))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v967, px_int(1LL))}, 1);
    }
    if (px_is_truthy(px_eq(px_index(_v967, px_int(0LL)), px_str("TyList")))) {
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v967, px_int(1LL))}, 1)), px_str("]"));
    }
    if (px_is_truthy(px_eq(px_index(_v967, px_int(0LL)), px_str("TyDict")))) {
        return px_add(px_add(px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v967, px_int(1LL))}, 1)), px_str(": ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v967, px_int(2LL))}, 1)), px_str("}"));
    }
    return px_str("any");
px_err_968:
    if (px_err_968_proped) return px_err_968_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v969 = (nargs > 0) ? args[0] : px_null();
    LXValue _v970 = px_null();
    LXValue _v971 = px_null();
    LXValue _v972 = px_null();
    LXValue _v973 = px_null();
    LXValue _v974 = px_null();
    LXValue _v975 = px_null();
    LXValue px_err_976_val = px_null();
    int px_err_976_proped = 0;
    _v970 = px_str("");
    _v971 = px_int(0LL);
    while (px_is_truthy(px_lt(_v971, px_call(px_get_global("len"), (LXValue[]){_v969}, 1)))) {
        _v972 = px_index(_v969, _v971);
        _v973 = ({ LXValue _t977 = px_ge(_v972, px_str("a")); px_is_truthy(_t977) ? px_le(_v972, px_str("z")) : _t977; });
        _v974 = ({ LXValue _t978 = px_ge(_v972, px_str("A")); px_is_truthy(_t978) ? px_le(_v972, px_str("Z")) : _t978; });
        _v975 = ({ LXValue _t979 = px_ge(_v972, px_str("0")); px_is_truthy(_t979) ? px_le(_v972, px_str("9")) : _t979; });
        if (px_is_truthy(({ LXValue _t981 = ({ LXValue _t980 = _v973; px_is_truthy(_t980) ? _t980 : _v974; }); px_is_truthy(_t981) ? _t981 : _v975; }))) {
             _v970 = px_add(_v970, _v972);
        }
        else {
             _v970 = px_add(_v970, px_str("_"));
        }
         _v971 = px_add(_v971, px_int(1LL));
    }
    return _v970;
px_err_976:
    if (px_err_976_proped) return px_err_976_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v982 = (nargs > 0) ? args[0] : px_null();
    LXValue _v983 = (nargs > 1) ? args[1] : px_null();
    LXValue _v984 = px_null();
    LXValue _v985 = px_null();
    LXValue _v986 = px_null();
    LXValue _v987 = px_null();
    LXValue _v988 = px_null();
    LXValue px_err_989_val = px_null();
    int px_err_989_proped = 0;
    _v984 = px_call(px_get_global("len"), (LXValue[]){_v982}, 1);
    _v985 = px_call(px_get_global("len"), (LXValue[]){_v983}, 1);
    _v986 = px_int(0LL);
    while (px_is_truthy(px_le(px_add(_v986, _v985), _v984))) {
        _v987 = px_int(0LL);
        _v988 = px_bool(true);
        while (px_is_truthy(px_lt(_v987, _v985))) {
            if (px_is_truthy(px_ne(px_index(_v982, px_add(_v986, _v987)), px_index(_v983, _v987)))) {
                 _v988 = px_bool(false);
                break;
            }
             _v987 = px_add(_v987, px_int(1LL));
        }
        if (px_is_truthy(_v988)) {
            return _v986;
        }
         _v986 = px_add(_v986, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_989:
    if (px_err_989_proped) return px_err_989_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v990 = (nargs > 0) ? args[0] : px_null();
    LXValue _v991 = px_null();
    LXValue _v992 = px_null();
    LXValue px_err_993_val = px_null();
    int px_err_993_proped = 0;
    _v991 = px_str("");
    _v992 = px_int(0LL);
    while (px_is_truthy(px_lt(_v992, _v990))) {
         _v991 = px_add(_v991, px_str("    "));
         _v992 = px_add(_v992, px_int(1LL));
    }
    return _v991;
px_err_993:
    if (px_err_993_proped) return px_err_993_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v994 = (nargs > 0) ? args[0] : px_null();
    LXValue _v995 = px_null();
    LXValue _v996 = px_null();
    LXValue _v997 = px_null();
    LXValue _v998 = px_null();
    LXValue _v999 = px_null();
    LXValue _v1000 = px_null();
    LXValue _v1001 = px_null();
    LXValue px_err_1002_val = px_null();
    int px_err_1002_proped = 0;
    _v995 = px_slice(_v994, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v994}, 1), px_int(1LL)), px_null());
    _v996 = px_str("");
    _v997 = px_int(0LL);
    while (px_is_truthy(px_lt(_v997, px_call(px_get_global("len"), (LXValue[]){_v995}, 1)))) {
        _v998 = px_index(_v995, _v997);
        if (px_is_truthy(px_eq(_v998, px_str("\\")))) {
            _v999 = px_index(_v995, px_add(_v997, px_int(1LL)));
            if (px_is_truthy(px_eq(_v999, px_str("n")))) {
                 _v996 = px_add(_v996, px_str("\n"));
                 _v997 = px_add(_v997, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v999, px_str("t")))) {
                 _v996 = px_add(_v996, px_str("\t"));
                 _v997 = px_add(_v997, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v999, px_str("r")))) {
                 _v996 = px_add(_v996, px_str("\r"));
                 _v997 = px_add(_v997, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v999, px_str("0")))) {
                 _v996 = px_add(_v996, px_str(""));
                 _v997 = px_add(_v997, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v999, px_str("\"")))) {
                 _v996 = px_add(_v996, px_str("\""));
                 _v997 = px_add(_v997, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v999, px_str("\\")))) {
                 _v996 = px_add(_v996, px_str("\\"));
                 _v997 = px_add(_v997, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v999, px_str("u")))) {
                _v1000 = px_add(_v997, px_int(3LL));
                _v1001 = px_str("");
                while (px_is_truthy(({ LXValue _t1003 = px_lt(_v1000, px_call(px_get_global("len"), (LXValue[]){_v995}, 1)); px_is_truthy(_t1003) ? px_ne(px_index(_v995, _v1000), px_str("}")) : _t1003; }))) {
                     _v1001 = px_add(_v1001, px_index(_v995, _v1000));
                     _v1000 = px_add(_v1000, px_int(1LL));
                }
                 _v996 = px_add(_v996, px_call(px_get_global("hex_to_char"), (LXValue[]){_v1001}, 1));
                 _v997 = px_add(_v1000, px_int(1LL));
            }
            else {
                 _v996 = px_add(_v996, _v999);
                 _v997 = px_add(_v997, px_int(2LL));
            }
        }
        else {
             _v996 = px_add(_v996, _v998);
             _v997 = px_add(_v997, px_int(1LL));
        }
    }
    return _v996;
px_err_1002:
    if (px_err_1002_proped) return px_err_1002_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1004 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1005 = px_null();
    LXValue _v1006 = px_null();
    LXValue _v1007 = px_null();
    LXValue px_err_1008_val = px_null();
    int px_err_1008_proped = 0;
    _v1005 = px_str("");
    _v1006 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1006, px_call(px_get_global("len"), (LXValue[]){_v1004}, 1)))) {
        _v1007 = px_index(_v1004, _v1006);
        if (px_is_truthy(px_eq(_v1007, px_str("\\")))) {
             _v1005 = px_add(_v1005, px_str("\\\\"));
        }
        else if (px_is_truthy(px_eq(_v1007, px_str("\"")))) {
             _v1005 = px_add(_v1005, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v1007, px_str("\n")))) {
             _v1005 = px_add(_v1005, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v1007, px_str("\r")))) {
             _v1005 = px_add(_v1005, px_str("\\r"));
        }
        else if (px_is_truthy(px_eq(_v1007, px_str("\t")))) {
             _v1005 = px_add(_v1005, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v1007, px_str("")))) {
             _v1005 = px_add(_v1005, px_str(""));
        }
        else {
             _v1005 = px_add(_v1005, _v1007);
        }
         _v1006 = px_add(_v1006, px_int(1LL));
    }
    return _v1005;
px_err_1008:
    if (px_err_1008_proped) return px_err_1008_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1009 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1010 = px_null();
    LXValue _v1011 = px_null();
    LXValue px_err_1012_val = px_null();
    int px_err_1012_proped = 0;
    _v1010 = px_str("");
    _v1011 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1011, _v1009))) {
         _v1010 = px_add(_v1010, px_str("0"));
         _v1011 = px_add(_v1011, px_int(1LL));
    }
    return _v1010;
px_err_1012:
    if (px_err_1012_proped) return px_err_1012_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1013 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1014 = px_null();
    LXValue _v1015 = px_null();
    LXValue _v1016 = px_null();
    LXValue _v1017 = px_null();
    LXValue _v1018 = px_null();
    LXValue _v1019 = px_null();
    LXValue _v1020 = px_null();
    LXValue _v1021 = px_null();
    LXValue _v1022 = px_null();
    LXValue _v1023 = px_null();
    LXValue _v1024 = px_null();
    LXValue _v1025 = px_null();
    LXValue _v1026 = px_null();
    LXValue _v1027 = px_null();
    LXValue px_err_1028_val = px_null();
    int px_err_1028_proped = 0;
    _v1014 = px_neg(px_int(1LL));
    _v1015 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1015, px_call(px_get_global("len"), (LXValue[]){_v1013}, 1)))) {
        if (px_is_truthy(({ LXValue _t1029 = px_eq(px_index(_v1013, _v1015), px_str("e")); px_is_truthy(_t1029) ? _t1029 : px_eq(px_index(_v1013, _v1015), px_str("E")); }))) {
             _v1014 = _v1015;
            break;
        }
         _v1015 = px_add(_v1015, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1014, px_int(0LL)))) {
        return _v1013;
    }
    _v1016 = px_slice(_v1013, px_int(0LL), _v1014, px_null());
    _v1017 = px_slice(_v1013, px_add(_v1014, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1013}, 1), px_null());
    _v1018 = px_int(1LL);
    if (px_is_truthy(({ LXValue _t1030 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1017}, 1), px_int(0LL)); px_is_truthy(_t1030) ? px_eq(px_index(_v1017, px_int(0LL)), px_str("+")) : _t1030; }))) {
         _v1017 = px_slice(_v1017, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1017}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1031 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1017}, 1), px_int(0LL)); px_is_truthy(_t1031) ? px_eq(px_index(_v1017, px_int(0LL)), px_str("-")) : _t1031; }))) {
         _v1018 = px_neg(px_int(1LL));
         _v1017 = px_slice(_v1017, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1017}, 1), px_null());
    }
    _v1019 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1017}, 1), _v1018);
    _v1020 = px_bool(false);
    if (px_is_truthy(({ LXValue _t1032 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1016}, 1), px_int(0LL)); px_is_truthy(_t1032) ? px_eq(px_index(_v1016, px_int(0LL)), px_str("-")) : _t1032; }))) {
         _v1020 = px_bool(true);
         _v1016 = px_slice(_v1016, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1016}, 1), px_null());
    }
    _v1021 = px_str("");
    _v1022 = px_str("");
    _v1023 = px_neg(px_int(1LL));
    _v1024 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1024, px_call(px_get_global("len"), (LXValue[]){_v1016}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v1016, _v1024), px_str(".")))) {
             _v1023 = _v1024;
            break;
        }
         _v1024 = px_add(_v1024, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1023, px_int(0LL)))) {
         _v1021 = _v1016;
    }
    else {
         _v1021 = px_slice(_v1016, px_int(0LL), _v1023, px_null());
         _v1022 = px_slice(_v1016, px_add(_v1023, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1016}, 1), px_null());
    }
    _v1025 = px_add(_v1021, _v1022);
    _v1026 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1021}, 1), _v1019);
    _v1027 = px_str("");
    if (px_is_truthy(px_le(_v1026, px_int(0LL)))) {
         _v1027 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1026)}, 1)), _v1025);
    }
    else if (px_is_truthy(px_ge(_v1026, px_call(px_get_global("len"), (LXValue[]){_v1025}, 1)))) {
         _v1027 = px_add(_v1025, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1026, px_call(px_get_global("len"), (LXValue[]){_v1025}, 1))}, 1));
    }
    else {
         _v1027 = px_add(px_add(px_slice(_v1025, px_int(0LL), _v1026, px_null()), px_str(".")), px_slice(_v1025, _v1026, px_call(px_get_global("len"), (LXValue[]){_v1025}, 1), px_null()));
    }
    if (px_is_truthy(_v1020)) {
        return px_add(px_str("-"), _v1027);
    }
    return _v1027;
px_err_1028:
    if (px_err_1028_proped) return px_err_1028_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1033 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1034 = px_null();
    LXValue _v1035 = px_null();
    LXValue px_err_1036_val = px_null();
    int px_err_1036_proped = 0;
    _v1034 = px_call(px_get_global("str"), (LXValue[]){_v1033}, 1);
    if (px_is_truthy(({ LXValue _t1038 = ({ LXValue _t1037 = px_eq(_v1034, px_str("inf")); px_is_truthy(_t1037) ? _t1037 : px_eq(_v1034, px_str("-inf")); }); px_is_truthy(_t1038) ? _t1038 : px_eq(_v1034, px_str("nan")); }))) {
        return _v1034;
    }
     _v1034 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1034}, 1);
    _v1035 = px_call(px_get_global("len"), (LXValue[]){_v1034}, 1);
    if (px_is_truthy(({ LXValue _t1039 = px_ge(_v1035, px_int(2LL)); px_is_truthy(_t1039) ? px_eq(px_slice(_v1034, px_sub(_v1035, px_int(2LL)), _v1035, px_null()), px_str(".0")) : _t1039; }))) {
        return px_slice(_v1034, px_int(0LL), px_sub(_v1035, px_int(2LL)), px_null());
    }
    return _v1034;
px_err_1036:
    if (px_err_1036_proped) return px_err_1036_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1040 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1041 = px_null();
    LXValue _v1042 = px_null();
    LXValue _v1043 = px_null();
    LXValue _v1044 = px_null();
    LXValue _v1045 = px_null();
    LXValue _v1046 = px_null();
    LXValue _v1047 = px_null();
    LXValue _v1048 = px_null();
    LXValue _v1049 = px_null();
    LXValue _v1050 = px_null();
    LXValue px_err_1051_val = px_null();
    int px_err_1051_proped = 0;
    _v1041 = px_index(_v1040, px_int(1LL));
    _v1042 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1042, px_call(px_get_global("len"), (LXValue[]){_v1041}, 1)))) {
        _v1043 = px_index(_v1041, _v1042);
        _v1044 = px_index(_v1043, px_int(0LL));
        if (px_is_truthy(px_eq(_v1044, px_str("StructDef")))) {
            _v1045 = px_list_n((LXValue[]){}, 0);
            _v1046 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1046, px_call(px_get_global("len"), (LXValue[]){px_index(_v1043, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1045, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1043, px_int(2LL)), _v1046), px_int(1LL))}, 1)}, 1));
                 _v1046 = px_add(_v1046, px_int(1LL));
            }
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1043, px_int(1LL))}, 1), _v1045);
        }
        else if (px_is_truthy(px_eq(_v1044, px_str("EnumDef")))) {
            _v1047 = px_list_n((LXValue[]){}, 0);
            _v1048 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1048, px_call(px_get_global("len"), (LXValue[]){px_index(_v1043, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1047, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1043, px_int(2LL)), _v1048), px_int(1LL))}, 1)}, 1));
                 _v1048 = px_add(_v1048, px_int(1LL));
            }
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1043, px_int(1LL))}, 1), _v1047);
        }
        else if (px_is_truthy(px_eq(_v1044, px_str("ImplDef")))) {
            _v1049 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1043, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1049}, 1))) {
                _v1050 = px_int(0LL);
                while (px_is_truthy(px_lt(_v1050, px_call(px_get_global("len"), (LXValue[]){px_index(_v1043, px_int(3LL))}, 1)))) {
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1049), "append", (LXValue[]){px_index(px_index(_v1043, px_int(3LL)), _v1050)}, 1));
                     _v1050 = px_add(_v1050, px_int(1LL));
                }
            }
            else {
                px_index_set(px_get_global("cg_impls"), _v1049, px_index(_v1043, px_int(3LL)));
            }
        }
         _v1042 = px_add(_v1042, px_int(1LL));
    }
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1041}, 1));
px_err_1051:
    if (px_err_1051_proped) return px_err_1051_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1052 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1053 = px_null();
    LXValue _v1054 = px_null();
    LXValue _v1055 = px_null();
    LXValue _v1056 = px_null();
    LXValue _v1057 = px_null();
    LXValue _v1058 = px_null();
    LXValue _v1059 = px_null();
    LXValue _v1060 = px_null();
    LXValue px_err_1061_val = px_null();
    int px_err_1061_proped = 0;
    _v1053 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1053, px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)))) {
        _v1054 = px_index(_v1052, _v1053);
        _v1055 = px_index(_v1054, px_int(0LL));
        if (px_is_truthy(px_eq(_v1055, px_str("TypeConst")))) {
            _v1056 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1054, px_int(1LL))}, 1);
            _v1057 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            _v1058 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1058, px_call(px_get_global("len"), (LXValue[]){px_index(_v1054, px_int(2LL))}, 1)))) {
                _v1059 = px_index(px_index(_v1054, px_int(2LL)), _v1058);
                px_index_set(_v1057, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1059, px_int(1LL))}, 1), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1059, px_int(2LL))}, 1));
                 _v1058 = px_add(_v1058, px_int(1LL));
            }
            px_index_set(px_get_global("cg_const_enums"), _v1056, _v1057);
        }
        else if (px_is_truthy(px_eq(_v1055, px_str("FuncDef")))) {
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1054, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1055, px_str("If")))) {
            _v1060 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1060, px_call(px_get_global("len"), (LXValue[]){px_index(_v1054, px_int(1LL))}, 1)))) {
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1054, px_int(1LL)), _v1060), px_int(1LL))}, 1));
                 _v1060 = px_add(_v1060, px_int(1LL));
            }
            if (px_is_truthy(px_ne(px_index(_v1054, px_int(2LL)), px_null()))) {
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1054, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1055, px_str("For")))) {
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1054, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1055, px_str("While")))) {
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1054, px_int(2LL))}, 1));
        }
         _v1053 = px_add(_v1053, px_int(1LL));
    }
px_err_1061:
    if (px_err_1061_proped) return px_err_1061_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1062 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1063 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1064 = px_null();
    LXValue _v1065 = px_null();
    LXValue _v1066 = px_null();
    LXValue _v1067 = px_null();
    LXValue _v1068 = px_null();
    LXValue _v1069 = px_null();
    LXValue _v1070 = px_null();
    LXValue _v1071 = px_null();
    LXValue px_err_1072_val = px_null();
    int px_err_1072_proped = 0;
    _v1064 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1064, px_call(px_get_global("len"), (LXValue[]){_v1062}, 1)))) {
        _v1065 = px_index(_v1062, _v1064);
        _v1066 = px_index(_v1065, px_int(0LL));
        if (px_is_truthy(px_eq(_v1066, px_str("Assign")))) {
            _v1067 = px_index(_v1065, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1067, px_int(0LL)), px_str("Var")))) {
                _v1068 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1067, px_int(1LL))}, 1);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1063, _v1068}, 2)))) {
                    (void)(px_method(_v1063, "append", (LXValue[]){_v1068}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1066, px_str("VarDecl")))) {
            _v1068 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1065, px_int(2LL))}, 1);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1063, _v1068}, 2)))) {
                (void)(px_method(_v1063, "append", (LXValue[]){_v1068}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1066, px_str("If")))) {
            _v1069 = px_index(_v1065, px_int(1LL));
            _v1070 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1070, px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)))) {
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1069, _v1070), px_int(1LL)), _v1063}, 2));
                 _v1070 = px_add(_v1070, px_int(1LL));
            }
            if (px_is_truthy(px_ne(px_index(_v1065, px_int(2LL)), px_null()))) {
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1065, px_int(2LL)), _v1063}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1066, px_str("For")))) {
            _v1071 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1065, px_int(1LL))}, 1);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1063, _v1071}, 2)))) {
                (void)(px_method(_v1063, "append", (LXValue[]){_v1071}, 1));
            }
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1065, px_int(3LL)), _v1063}, 2));
        }
        else if (px_is_truthy(px_eq(_v1066, px_str("While")))) {
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1065, px_int(2LL)), _v1063}, 2));
        }
         _v1064 = px_add(_v1064, px_int(1LL));
    }
px_err_1072:
    if (px_err_1072_proped) return px_err_1072_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1073 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1074 = px_null();
    LXValue px_err_1075_val = px_null();
    int px_err_1075_proped = 0;
    _v1074 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1073, px_int(1LL))}, 1)}, 1));
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1073, _v1074}, 2);
px_err_1075:
    if (px_err_1075_proped) return px_err_1075_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1076 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1077 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1078 = px_null();
    LXValue _v1079 = px_null();
    LXValue _v1080 = px_null();
    LXValue _v1081 = px_null();
    LXValue _v1082 = px_null();
    LXValue _v1083 = px_null();
    LXValue _v1084 = px_null();
    LXValue _v1085 = px_null();
    LXValue _v1086 = px_null();
    LXValue _v1087 = px_null();
    LXValue _v1088 = px_null();
    LXValue _v1089 = px_null();
    LXValue _v1090 = px_null();
    LXValue _v1091 = px_null();
    LXValue _v1092 = px_null();
    LXValue px_err_1093_val = px_null();
    int px_err_1093_proped = 0;
    _v1078 = px_add(px_add(px_str("static LXValue "), _v1077), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1078 = px_add(_v1078, px_str("    (void)ctx;\n"));
    _v1079 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    _v1080 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    _v1081 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1081}, 1));
    _v1082 = px_index(_v1076, px_int(2LL));
    _v1083 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1083, px_call(px_get_global("len"), (LXValue[]){_v1082}, 1)))) {
        _v1084 = px_index(_v1082, _v1083);
        _v1085 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1084, px_int(1LL))}, 1)}, 1);
        _v1086 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v1084, px_int(3LL)), px_null()))) {
             _v1086 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1084, px_int(3LL))}, 1);
        }
         _v1078 = px_add(_v1078, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1085), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1083}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1083}, 1)), px_str("] : ")), _v1086), px_str(";\n")));
         _v1083 = px_add(_v1083, px_int(1LL));
    }
    _v1087 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1076, px_int(4LL)), _v1087}, 2));
    _v1088 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1088, px_call(px_get_global("len"), (LXValue[]){_v1087}, 1)))) {
        _v1089 = px_index(_v1087, _v1088);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1089}, 1), px_null()))) {
             _v1088 = px_add(_v1088, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1089}, 2))) {
             _v1088 = px_add(_v1088, px_int(1LL));
            continue;
        }
        _v1085 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1089}, 1);
         _v1078 = px_add(_v1078, px_add(px_add(px_str("    LXValue "), _v1085), px_str(" = px_null();\n")));
         _v1088 = px_add(_v1088, px_int(1LL));
    }
    _v1090 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1090}, 1));
     _v1078 = px_add(_v1078, px_add(px_add(px_str("    LXValue "), _v1090), px_str("_val = px_null();\n")));
     _v1078 = px_add(_v1078, px_add(px_add(px_str("    int "), _v1090), px_str("_proped = 0;\n")));
    _v1091 = px_index(_v1076, px_int(4LL));
    _v1092 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1092, px_call(px_get_global("len"), (LXValue[]){_v1091}, 1)))) {
         _v1078 = px_add(_v1078, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1091, _v1092), px_int(1LL)}, 2));
         _v1092 = px_add(_v1092, px_int(1LL));
    }
     _v1078 = px_add(_v1078, px_add(_v1090, px_str(":\n")));
     _v1078 = px_add(_v1078, px_add(px_add(px_add(px_add(px_str("    if ("), _v1090), px_str("_proped) return ")), _v1090), px_str("_val;\n")));
     _v1078 = px_add(_v1078, px_str("    return px_null();\n"));
     _v1078 = px_add(_v1078, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_vars", _v1079);
    px_set_global("cg_var_types", _v1080);
    px_set_global("cg_immutables", _v1081);
    return _v1078;
px_err_1093:
    if (px_err_1093_proped) return px_err_1093_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1094 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1095 = px_null();
    LXValue _v1096 = px_null();
    LXValue _v1097 = px_null();
    LXValue _v1098 = px_null();
    LXValue _v1099 = px_null();
    LXValue _v1100 = px_null();
    LXValue _v1101 = px_null();
    LXValue _v1102 = px_null();
    LXValue _v1103 = px_null();
    LXValue _v1104 = px_null();
    LXValue _v1105 = px_null();
    LXValue _v1106 = px_null();
    LXValue _v1107 = px_null();
    LXValue _v1108 = px_null();
    LXValue _v1109 = px_null();
    LXValue _v1110 = px_null();
    LXValue _v1111 = px_null();
    LXValue _v1112 = px_null();
    LXValue _v1113 = px_null();
    LXValue _v1114 = px_null();
    LXValue _v1115 = px_null();
    LXValue _v1116 = px_null();
    LXValue _v1117 = px_null();
    LXValue _v1118 = px_null();
    LXValue _v1119 = px_null();
    LXValue _v1120 = px_null();
    LXValue _v1121 = px_null();
    LXValue _v1122 = px_null();
    LXValue _v1123 = px_null();
    LXValue _v1124 = px_null();
    LXValue _v1125 = px_null();
    LXValue _v1126 = px_null();
    LXValue _v1127 = px_null();
    LXValue px_err_1128_val = px_null();
    int px_err_1128_proped = 0;
    _v1095 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_set_global("cg_closures", px_str(""));
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_uidc", px_int(0LL));
    px_set_global("cg_closure_id", px_int(0LL));
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1094}, 1));
    _v1096 = px_index(_v1094, px_int(1LL));
    _v1097 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1097, px_call(px_get_global("len"), (LXValue[]){_v1096}, 1)))) {
        _v1098 = px_index(_v1096, _v1097);
        _v1099 = px_index(_v1098, px_int(0LL));
        if (px_is_truthy(px_eq(_v1099, px_str("FuncDef")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1099, px_str("ExternDef")))) {
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, px_int(1LL))}, 1), px_index(_v1098, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1099, px_str("VarDecl")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, px_int(2LL))}, 1)}, 1));
            if (px_is_truthy(({ LXValue _t1129 = px_eq(px_index(_v1098, px_int(1LL)), px_str("Let")); px_is_truthy(_t1129) ? _t1129 : px_eq(px_index(_v1098, px_int(1LL)), px_str("Const")); }))) {
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1099, px_str("Assign")))) {
            _v1100 = px_index(_v1098, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1100, px_int(0LL)), px_str("Var")))) {
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1100, px_int(1LL))}, 1)}, 1));
            }
        }
         _v1097 = px_add(_v1097, px_int(1LL));
    }
    _v1101 = px_list_n((LXValue[]){}, 0);
    _v1102 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    _v1103 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1103, px_call(px_get_global("len"), (LXValue[]){_v1102}, 1)))) {
        _v1104 = px_index(_v1102, _v1103);
        _v1105 = px_index(px_get_global("cg_impls"), _v1104);
        _v1106 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1106, px_call(px_get_global("len"), (LXValue[]){_v1105}, 1)))) {
            (void)(px_method(_v1101, "append", (LXValue[]){px_list_n((LXValue[]){_v1104, px_index(_v1105, _v1106)}, 2)}, 1));
             _v1106 = px_add(_v1106, px_int(1LL));
        }
         _v1103 = px_add(_v1103, px_int(1LL));
    }
    _v1107 = px_int(1LL);
    while (px_is_truthy(px_lt(_v1107, px_call(px_get_global("len"), (LXValue[]){_v1101}, 1)))) {
        _v1108 = _v1107;
        while (px_is_truthy(px_gt(_v1108, px_int(0LL)))) {
            _v1109 = px_add(px_add(px_index(px_index(_v1101, px_sub(_v1108, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1101, px_sub(_v1108, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            _v1110 = px_add(px_add(px_index(px_index(_v1101, _v1108), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1101, _v1108), px_int(1LL)), px_int(1LL))}, 1));
            if (px_is_truthy(px_lt(_v1110, _v1109))) {
                _v1111 = px_index(_v1101, _v1108);
                px_index_set(_v1101, _v1108, px_index(_v1101, px_sub(_v1108, px_int(1LL))));
                px_index_set(_v1101, px_sub(_v1108, px_int(1LL)), _v1111);
            }
             _v1108 = px_sub(_v1108, px_int(1LL));
        }
         _v1107 = px_add(_v1107, px_int(1LL));
    }
    _v1112 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1112, px_call(px_get_global("len"), (LXValue[]){_v1101}, 1)))) {
        _v1104 = px_index(px_index(_v1101, _v1112), px_int(0LL));
        _v1113 = px_index(px_index(_v1101, _v1112), px_int(1LL));
        _v1114 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1104}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1113, px_int(1LL))}, 1)}, 1));
         _v1095 = px_add(_v1095, px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1113, _v1114}, 2));
         _v1095 = px_add(_v1095, px_str("\n"));
         _v1112 = px_add(_v1112, px_int(1LL));
    }
    _v1115 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1115, px_call(px_get_global("len"), (LXValue[]){_v1096}, 1)))) {
        _v1098 = px_index(_v1096, _v1115);
        if (px_is_truthy(px_eq(px_index(_v1098, px_int(0LL)), px_str("FuncDef")))) {
             _v1095 = px_add(_v1095, px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1098}, 1));
             _v1095 = px_add(_v1095, px_str("\n"));
        }
         _v1115 = px_add(_v1115, px_int(1LL));
    }
     _v1095 = px_add(_v1095, px_str("int main(int argc, char** argv) {\n"));
     _v1095 = px_add(_v1095, px_str("    px_args_init(argc, argv);\n"));
     _v1095 = px_add(_v1095, px_str("    px_register_builtins();\n"));
    _v1116 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1116, px_call(px_get_global("len"), (LXValue[]){_v1096}, 1)))) {
        _v1098 = px_index(_v1096, _v1116);
        if (px_is_truthy(px_eq(px_index(_v1098, px_int(0LL)), px_str("FuncDef")))) {
            _v1114 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, px_int(1LL))}, 1);
            _v1117 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1114}, 1));
             _v1095 = px_add(_v1095, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1114), px_str("\", px_func(\"")), _v1114), px_str("\", ")), _v1117), px_str(", NULL));\n")));
        }
         _v1116 = px_add(_v1116, px_int(1LL));
    }
    _v1118 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1118, px_call(px_get_global("len"), (LXValue[]){_v1101}, 1)))) {
        _v1104 = px_index(px_index(_v1101, _v1118), px_int(0LL));
        _v1113 = px_index(px_index(_v1101, _v1118), px_int(1LL));
        _v1119 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1113, px_int(1LL))}, 1);
        _v1114 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1104}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1119}, 1));
         _v1095 = px_add(_v1095, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1104), px_str(".")), _v1119), px_str("\", px_func(\"")), _v1104), px_str(".")), _v1119), px_str("\", ")), _v1114), px_str(", NULL));\n")));
         _v1118 = px_add(_v1118, px_int(1LL));
    }
    _v1120 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1120, px_call(px_get_global("len"), (LXValue[]){_v1096}, 1)))) {
        _v1098 = px_index(_v1096, _v1120);
        _v1099 = px_index(_v1098, px_int(0LL));
        if (px_is_truthy(({ LXValue _t1135 = ({ LXValue _t1134 = ({ LXValue _t1133 = ({ LXValue _t1132 = ({ LXValue _t1131 = ({ LXValue _t1130 = px_ne(_v1099, px_str("FuncDef")); px_is_truthy(_t1130) ? px_ne(_v1099, px_str("StructDef")) : _t1130; }); px_is_truthy(_t1131) ? px_ne(_v1099, px_str("EnumDef")) : _t1131; }); px_is_truthy(_t1132) ? px_ne(_v1099, px_str("TraitDef")) : _t1132; }); px_is_truthy(_t1133) ? px_ne(_v1099, px_str("ImplDef")) : _t1133; }); px_is_truthy(_t1134) ? px_ne(_v1099, px_str("Import")) : _t1134; }); px_is_truthy(_t1135) ? px_ne(_v1099, px_str("ExternDef")) : _t1135; }))) {
             _v1095 = px_add(_v1095, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1098, px_int(1LL)}, 2));
        }
         _v1120 = px_add(_v1120, px_int(1LL));
    }
    _v1121 = px_bool(false);
    _v1122 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1122, px_call(px_get_global("len"), (LXValue[]){_v1096}, 1)))) {
        _v1098 = px_index(_v1096, _v1122);
        if (px_is_truthy(({ LXValue _t1136 = px_eq(px_index(_v1098, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1136) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, px_int(1LL))}, 1), px_str("main")) : _t1136; }))) {
             _v1121 = px_bool(true);
            break;
        }
         _v1122 = px_add(_v1122, px_int(1LL));
    }
    if (px_is_truthy(_v1121)) {
        _v1117 = px_str("fn_main");
         _v1095 = px_add(_v1095, px_add(px_add(px_str("    { LXValue _r = "), _v1117), px_str("(NULL, 0, NULL); int _code = 0;\n")));
         _v1095 = px_add(_v1095, px_str("      if (px_is_result(_r)) {\n"));
         _v1095 = px_add(_v1095, px_str("        if (!px_result_ok(_r)) {\n"));
         _v1095 = px_add(_v1095, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
         _v1095 = px_add(_v1095, px_str("          _code = 1;\n"));
         _v1095 = px_add(_v1095, px_str("        } else {\n"));
         _v1095 = px_add(_v1095, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
         _v1095 = px_add(_v1095, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
         _v1095 = px_add(_v1095, px_str("        }\n"));
         _v1095 = px_add(_v1095, px_str("      } else if (_r.type == PX_INT) {\n"));
         _v1095 = px_add(_v1095, px_str("        _code = (int)_r.as.i;\n"));
         _v1095 = px_add(_v1095, px_str("      }\n"));
         _v1095 = px_add(_v1095, px_str("      return _code;\n"));
         _v1095 = px_add(_v1095, px_str("    }\n"));
    }
    else {
         _v1095 = px_add(_v1095, px_str("    return 0;\n"));
    }
     _v1095 = px_add(_v1095, px_str("}\n"));
    _v1123 = px_call(px_get_global("cg_find"), (LXValue[]){_v1095, px_str("int main(")}, 2);
    if (px_is_truthy(px_ge(_v1123, px_int(0LL)))) {
        _v1124 = px_slice(_v1095, px_int(0LL), _v1123, px_null());
        _v1125 = px_slice(_v1095, _v1123, px_call(px_get_global("len"), (LXValue[]){_v1095}, 1), px_null());
        _v1126 = px_call(px_get_global("cg_find"), (LXValue[]){_v1124, px_str("static LXValue")}, 2);
        _v1127 = px_str("");
        if (px_is_truthy(px_ge(_v1126, px_int(0LL)))) {
             _v1127 = px_add(px_add(px_add(px_add(px_slice(_v1124, px_int(0LL), _v1126, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1124, _v1126, px_call(px_get_global("len"), (LXValue[]){_v1124}, 1), px_null())), _v1125);
        }
        else {
             _v1127 = px_add(px_add(px_add(_v1124, px_get_global("cg_closures")), px_str("\n")), _v1125);
        }
        return _v1127;
    }
    return _v1095;
px_err_1128:
    if (px_err_1128_proped) return px_err_1128_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1137 = px_null();
    LXValue _v1138 = px_null();
    LXValue _v1139 = px_null();
    LXValue _v1140 = px_null();
    LXValue _v1141 = px_null();
    LXValue _v1142 = px_null();
    LXValue _v1143 = px_null();
    LXValue px_err_1144_val = px_null();
    int px_err_1144_proped = 0;
    _v1137 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t1146 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1137}, 1), px_int(2LL)); px_is_truthy(_t1146) ? ({ LXValue _t1145 = px_eq(px_index(_v1137, px_int(1LL)), px_str("--version")); px_is_truthy(_t1145) ? _t1145 : px_eq(px_index(_v1137, px_int(1LL)), px_str("-v")); }) : _t1146; }))) {
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        return px_int(0LL);
    }
    _v1138 = px_index(_v1137, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1137}, 1), px_int(1LL)));
    _v1139 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1138}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v1138}, 1)}, 1));
    px_set_global("p_pos", px_int(0LL));
    _v1140 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    _v1141 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v1140, _v1139}, 2);
    _v1142 = px_call(px_get_global("cg_generate"), (LXValue[]){_v1141}, 1);
    _v1143 = px_call(px_get_global("len"), (LXValue[]){_v1142}, 1);
    if (px_is_truthy(({ LXValue _t1147 = px_gt(_v1143, px_int(0LL)); px_is_truthy(_t1147) ? px_eq(px_index(_v1142, px_sub(_v1143, px_int(1LL))), px_str("\n")) : _t1147; }))) {
         _v1142 = px_slice(_v1142, px_int(0LL), px_sub(_v1143, px_int(1LL)), px_null());
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1142}, 1));
px_err_1144:
    if (px_err_1144_proped) return px_err_1144_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("peek", px_func("peek", fn_peek, NULL));
    px_set_global("peek2", px_func("peek2", fn_peek2, NULL));
    px_set_global("peek3", px_func("peek3", fn_peek3, NULL));
    px_set_global("advance", px_func("advance", fn_advance, NULL));
    px_set_global("emit_at", px_func("emit_at", fn_emit_at, NULL));
    px_set_global("emit", px_func("emit", fn_emit, NULL));
    px_set_global("emit_token", px_func("emit_token", fn_emit_token, NULL));
    px_set_global("err", px_func("err", fn_err, NULL));
    px_set_global("err_at", px_func("err_at", fn_err_at, NULL));
    px_set_global("is_digit", px_func("is_digit", fn_is_digit, NULL));
    px_set_global("is_hex_digit", px_func("is_hex_digit", fn_is_hex_digit, NULL));
    px_set_global("is_alnum", px_func("is_alnum", fn_is_alnum, NULL));
    px_set_global("is_ident_start", px_func("is_ident_start", fn_is_ident_start, NULL));
    px_set_global("is_ident_continue", px_func("is_ident_continue", fn_is_ident_continue, NULL));
    px_set_global("digit_val", px_func("digit_val", fn_digit_val, NULL));
    px_set_global("handle_line_start", px_func("handle_line_start", fn_handle_line_start, NULL));
    px_set_global("skip_comment", px_func("skip_comment", fn_skip_comment, NULL));
    px_set_global("scan_ident_token", px_func("scan_ident_token", fn_scan_ident_token, NULL));
    px_set_global("scan_radix_token", px_func("scan_radix_token", fn_scan_radix_token, NULL));
    px_set_global("strip_leading_zeros", px_func("strip_leading_zeros", fn_strip_leading_zeros, NULL));
    px_set_global("scan_number_token", px_func("scan_number_token", fn_scan_number_token, NULL));
    px_set_global("scan_string", px_func("scan_string", fn_scan_string, NULL));
    px_set_global("scan_string_tokens", px_func("scan_string_tokens", fn_scan_string_tokens, NULL));
    px_set_global("scan_multiline_string_tokens", px_func("scan_multiline_string_tokens", fn_scan_multiline_string_tokens, NULL));
    px_set_global("scan_interp_expr", px_func("scan_interp_expr", fn_scan_interp_expr, NULL));
    px_set_global("hex_to_char", px_func("hex_to_char", fn_hex_to_char, NULL));
    px_set_global("scan_escape", px_func("scan_escape", fn_scan_escape, NULL));
    px_set_global("int_to_hex_nopad", px_func("int_to_hex_nopad", fn_int_to_hex_nopad, NULL));
    px_set_global("char_debug", px_func("char_debug", fn_char_debug, NULL));
    px_set_global("rust_str_debug", px_func("rust_str_debug", fn_rust_str_debug, NULL));
    px_set_global("ctrl_codepoint", px_func("ctrl_codepoint", fn_ctrl_codepoint, NULL));
    px_set_global("ctrl_hex", px_func("ctrl_hex", fn_ctrl_hex, NULL));
    px_set_global("scan_operator_token", px_func("scan_operator_token", fn_scan_operator_token, NULL));
    px_set_global("next_token", px_func("next_token", fn_next_token, NULL));
    px_set_global("check_edition", px_func("check_edition", fn_check_edition, NULL));
    px_set_global("lex_tokens", px_func("lex_tokens", fn_lex_tokens, NULL));
    px_set_global("pad", px_func("pad", fn_pad, NULL));
    px_set_global("dump_node", px_func("dump_node", fn_dump_node, NULL));
    px_set_global("dump_list", px_func("dump_list", fn_dump_list, NULL));
    px_set_global("dump_str_list", px_func("dump_str_list", fn_dump_str_list, NULL));
    px_set_global("dump_ty_list", px_func("dump_ty_list", fn_dump_ty_list, NULL));
    px_set_global("dump_pat_list", px_func("dump_pat_list", fn_dump_pat_list, NULL));
    px_set_global("dump_opt_node", px_func("dump_opt_node", fn_dump_opt_node, NULL));
    px_set_global("dump_opt_str", px_func("dump_opt_str", fn_dump_opt_str, NULL));
    px_set_global("dump_opt_list", px_func("dump_opt_list", fn_dump_opt_list, NULL));
    px_set_global("dump_pos", px_func("dump_pos", fn_dump_pos, NULL));
    px_set_global("dump_t2_list", px_func("dump_t2_list", fn_dump_t2_list, NULL));
    px_set_global("dump_t2b_list", px_func("dump_t2b_list", fn_dump_t2b_list, NULL));
    px_set_global("dump_t3_list", px_func("dump_t3_list", fn_dump_t3_list, NULL));
    px_set_global("fmt_float", px_func("fmt_float", fn_fmt_float, NULL));
    px_set_global("dump_field", px_func("dump_field", fn_dump_field, NULL));
    px_set_global("dump_program", px_func("dump_program", fn_dump_program, NULL));
    px_set_global("pk", px_func("pk", fn_pk, NULL));
    px_set_global("pk_display", px_func("pk_display", fn_pk_display, NULL));
    px_set_global("pv", px_func("pv", fn_pv, NULL));
    px_set_global("pline", px_func("pline", fn_pline, NULL));
    px_set_global("pcol", px_func("pcol", fn_pcol, NULL));
    px_set_global("ppos", px_func("ppos", fn_ppos, NULL));
    px_set_global("adv", px_func("adv", fn_adv, NULL));
    px_set_global("chk", px_func("chk", fn_chk, NULL));
    px_set_global("chk2", px_func("chk2", fn_chk2, NULL));
    px_set_global("chk3", px_func("chk3", fn_chk3, NULL));
    px_set_global("expect", px_func("expect", fn_expect, NULL));
    px_set_global("expect_ident", px_func("expect_ident", fn_expect_ident, NULL));
    px_set_global("is_name_kind", px_func("is_name_kind", fn_is_name_kind, NULL));
    px_set_global("expect_name", px_func("expect_name", fn_expect_name, NULL));
    px_set_global("perr", px_func("perr", fn_perr, NULL));
    px_set_global("skip_newlines", px_func("skip_newlines", fn_skip_newlines, NULL));
    px_set_global("skip_brace_indents", px_func("skip_brace_indents", fn_skip_brace_indents, NULL));
    px_set_global("skip_newlines_in_block", px_func("skip_newlines_in_block", fn_skip_newlines_in_block, NULL));
    px_set_global("node_pos", px_func("node_pos", fn_node_pos, NULL));
    px_set_global("qstr", px_func("qstr", fn_qstr, NULL));
    px_set_global("parse_program", px_func("parse_program", fn_parse_program, NULL));
    px_set_global("parse_stmt", px_func("parse_stmt", fn_parse_stmt, NULL));
    px_set_global("parse_var_decl", px_func("parse_var_decl", fn_parse_var_decl, NULL));
    px_set_global("parse_assign_or_expr", px_func("parse_assign_or_expr", fn_parse_assign_or_expr, NULL));
    px_set_global("parse_if", px_func("parse_if", fn_parse_if, NULL));
    px_set_global("parse_for", px_func("parse_for", fn_parse_for, NULL));
    px_set_global("parse_while", px_func("parse_while", fn_parse_while, NULL));
    px_set_global("parse_block", px_func("parse_block", fn_parse_block, NULL));
    px_set_global("parse_type_params", px_func("parse_type_params", fn_parse_type_params, NULL));
    px_set_global("parse_func_def", px_func("parse_func_def", fn_parse_func_def, NULL));
    px_set_global("parse_extern_def", px_func("parse_extern_def", fn_parse_extern_def, NULL));
    px_set_global("parse_struct_def", px_func("parse_struct_def", fn_parse_struct_def, NULL));
    px_set_global("parse_enum_def", px_func("parse_enum_def", fn_parse_enum_def, NULL));
    px_set_global("parse_type_const", px_func("parse_type_const", fn_parse_type_const, NULL));
    px_set_global("parse_trait_def", px_func("parse_trait_def", fn_parse_trait_def, NULL));
    px_set_global("parse_impl_def", px_func("parse_impl_def", fn_parse_impl_def, NULL));
    px_set_global("parse_import", px_func("parse_import", fn_parse_import, NULL));
    px_set_global("parse_import_from", px_func("parse_import_from", fn_parse_import_from, NULL));
    px_set_global("parse_select", px_func("parse_select", fn_parse_select, NULL));
    px_set_global("parse_case_body", px_func("parse_case_body", fn_parse_case_body, NULL));
    px_set_global("parse_params", px_func("parse_params", fn_parse_params, NULL));
    px_set_global("parse_expr", px_func("parse_expr", fn_parse_expr, NULL));
    px_set_global("parse_pipe", px_func("parse_pipe", fn_parse_pipe, NULL));
    px_set_global("parse_null_coalesce", px_func("parse_null_coalesce", fn_parse_null_coalesce, NULL));
    px_set_global("parse_or", px_func("parse_or", fn_parse_or, NULL));
    px_set_global("parse_and", px_func("parse_and", fn_parse_and, NULL));
    px_set_global("parse_comparison", px_func("parse_comparison", fn_parse_comparison, NULL));
    px_set_global("parse_bitor", px_func("parse_bitor", fn_parse_bitor, NULL));
    px_set_global("parse_bitxor", px_func("parse_bitxor", fn_parse_bitxor, NULL));
    px_set_global("parse_bitand", px_func("parse_bitand", fn_parse_bitand, NULL));
    px_set_global("parse_shift", px_func("parse_shift", fn_parse_shift, NULL));
    px_set_global("parse_add", px_func("parse_add", fn_parse_add, NULL));
    px_set_global("parse_mul", px_func("parse_mul", fn_parse_mul, NULL));
    px_set_global("parse_pow", px_func("parse_pow", fn_parse_pow, NULL));
    px_set_global("parse_unary", px_func("parse_unary", fn_parse_unary, NULL));
    px_set_global("parse_postfix", px_func("parse_postfix", fn_parse_postfix, NULL));
    px_set_global("parse_slice_bound", px_func("parse_slice_bound", fn_parse_slice_bound, NULL));
    px_set_global("parse_call_args", px_func("parse_call_args", fn_parse_call_args, NULL));
    px_set_global("parse_primary", px_func("parse_primary", fn_parse_primary, NULL));
    px_set_global("parse_list_or_comp", px_func("parse_list_or_comp", fn_parse_list_or_comp, NULL));
    px_set_global("parse_comp_vars", px_func("parse_comp_vars", fn_parse_comp_vars, NULL));
    px_set_global("parse_comp_clauses", px_func("parse_comp_clauses", fn_parse_comp_clauses, NULL));
    px_set_global("fold_comp_conds", px_func("fold_comp_conds", fn_fold_comp_conds, NULL));
    px_set_global("parse_paren_or_tuple", px_func("parse_paren_or_tuple", fn_parse_paren_or_tuple, NULL));
    px_set_global("brace_looks_like_dict", px_func("brace_looks_like_dict", fn_brace_looks_like_dict, NULL));
    px_set_global("parse_brace", px_func("parse_brace", fn_parse_brace, NULL));
    px_set_global("parse_closure", px_func("parse_closure", fn_parse_closure, NULL));
    px_set_global("parse_match_expr", px_func("parse_match_expr", fn_parse_match_expr, NULL));
    px_set_global("parse_if_expr", px_func("parse_if_expr", fn_parse_if_expr, NULL));
    px_set_global("parse_pattern", px_func("parse_pattern", fn_parse_pattern, NULL));
    px_set_global("is_upper", px_func("is_upper", fn_is_upper, NULL));
    px_set_global("parse_type", px_func("parse_type", fn_parse_type, NULL));
    px_set_global("parse_type_base", px_func("parse_type_base", fn_parse_type_base, NULL));
    px_set_global("cg_gen_stmt", px_func("cg_gen_stmt", fn_cg_gen_stmt, NULL));
    px_set_global("cg_assign_op_global", px_func("cg_assign_op_global", fn_cg_assign_op_global, NULL));
    px_set_global("cg_assign_op_local", px_func("cg_assign_op_local", fn_cg_assign_op_local, NULL));
    px_set_global("cg_gen_select", px_func("cg_gen_select", fn_cg_gen_select, NULL));
    px_set_global("cg_comp_collect", px_func("cg_comp_collect", fn_cg_comp_collect, NULL));
    px_set_global("cg_comp_restore", px_func("cg_comp_restore", fn_cg_comp_restore, NULL));
    px_set_global("cg_comp_body", px_func("cg_comp_body", fn_cg_comp_body, NULL));
    px_set_global("cg_gen_expr", px_func("cg_gen_expr", fn_cg_gen_expr, NULL));
    px_set_global("cg_binop_cname", px_func("cg_binop_cname", fn_cg_binop_cname, NULL));
    px_set_global("cg_gen_pattern_cond", px_func("cg_gen_pattern_cond", fn_cg_gen_pattern_cond, NULL));
    px_set_global("cg_gen_lambda", px_func("cg_gen_lambda", fn_cg_gen_lambda, NULL));
    px_set_global("cg_dirname", px_func("cg_dirname", fn_cg_dirname, NULL));
    px_set_global("cg_stdlib_dir", px_func("cg_stdlib_dir", fn_cg_stdlib_dir, NULL));
    px_set_global("cg_find_module_path", px_func("cg_find_module_path", fn_cg_find_module_path, NULL));
    px_set_global("cg_is_definition", px_func("cg_is_definition", fn_cg_is_definition, NULL));
    px_set_global("cg_def_name", px_func("cg_def_name", fn_cg_def_name, NULL));
    px_set_global("cg_load_module", px_func("cg_load_module", fn_cg_load_module, NULL));
    px_set_global("cg_resolve_modules", px_func("cg_resolve_modules", fn_cg_resolve_modules, NULL));
    px_set_global("cg_new_dict", px_func("cg_new_dict", fn_cg_new_dict, NULL));
    px_set_global("cg_dict_copy", px_func("cg_dict_copy", fn_cg_dict_copy, NULL));
    px_set_global("cg_uid", px_func("cg_uid", fn_cg_uid, NULL));
    px_set_global("cg_tmp", px_func("cg_tmp", fn_cg_tmp, NULL));
    px_set_global("cg_new_var", px_func("cg_new_var", fn_cg_new_var, NULL));
    px_set_global("cg_var_of", px_func("cg_var_of", fn_cg_var_of, NULL));
    px_set_global("cg_mark_immutable", px_func("cg_mark_immutable", fn_cg_mark_immutable, NULL));
    px_set_global("cg_is_immutable", px_func("cg_is_immutable", fn_cg_is_immutable, NULL));
    px_set_global("cg_perr", px_func("cg_perr", fn_cg_perr, NULL));
    px_set_global("cg_is_nonnull_ty", px_func("cg_is_nonnull_ty", fn_cg_is_nonnull_ty, NULL));
    px_set_global("cg_is_null_lit", px_func("cg_is_null_lit", fn_cg_is_null_lit, NULL));
    px_set_global("cg_ty_name", px_func("cg_ty_name", fn_cg_ty_name, NULL));
    px_set_global("cg_func_cname", px_func("cg_func_cname", fn_cg_func_cname, NULL));
    px_set_global("cg_find", px_func("cg_find", fn_cg_find, NULL));
    px_set_global("cg_pad", px_func("cg_pad", fn_cg_pad, NULL));
    px_set_global("rust_unescape", px_func("rust_unescape", fn_rust_unescape, NULL));
    px_set_global("cg_escape_str", px_func("cg_escape_str", fn_cg_escape_str, NULL));
    px_set_global("cg_pad_zeros", px_func("cg_pad_zeros", fn_cg_pad_zeros, NULL));
    px_set_global("cg_expand_sci", px_func("cg_expand_sci", fn_cg_expand_sci, NULL));
    px_set_global("cg_fmt_float", px_func("cg_fmt_float", fn_cg_fmt_float, NULL));
    px_set_global("cg_collect_types", px_func("cg_collect_types", fn_cg_collect_types, NULL));
    px_set_global("cg_collect_consts", px_func("cg_collect_consts", fn_cg_collect_consts, NULL));
    px_set_global("cg_collect_hoist_vars", px_func("cg_collect_hoist_vars", fn_cg_collect_hoist_vars, NULL));
    px_set_global("cg_gen_func", px_func("cg_gen_func", fn_cg_gen_func, NULL));
    px_set_global("cg_gen_func_named", px_func("cg_gen_func_named", fn_cg_gen_func_named, NULL));
    px_set_global("cg_generate", px_func("cg_generate", fn_cg_generate, NULL));
    px_set_global("main", px_func("main", fn_main, NULL));
    px_set_global("g_src", px_str(""));
    px_set_global("g_len", px_int(0LL));
    px_set_global("g_pos", px_int(0LL));
    px_set_global("g_line", px_int(1LL));
    px_set_global("g_col", px_int(1LL));
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_set_global("g_at_line_start", px_bool(true));
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_set_global("g_count", px_int(0LL));
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_set_global("CTRL_ALL", px_str(""));
    px_set_global("p_toks", px_list_n((LXValue[]){}, 0));
    px_set_global("p_pos", px_int(0LL));
    px_set_global("cg_closures", px_str(""));
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_uidc", px_int(0LL));
    px_set_global("cg_closure_id", px_int(0LL));
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("PXC_VER", px_str("0.1.0"));
    px_set_global("PXC_MS", px_str("M-B9a"));
    { LXValue _r = fn_main(NULL, 0, NULL); int _code = 0;
      if (px_is_result(_r)) {
        if (!px_result_ok(_r)) {
          fprintf(stderr, "错误: %s\n", px_to_string(px_result_unwrap(_r)));
          _code = 1;
        } else {
          LXValue _uv = px_result_unwrap(_r);
          if (_uv.type == PX_INT) _code = (int)_uv.as.i;
        }
      } else if (_r.type == PX_INT) {
        _code = (int)_r.as.i;
      }
      return _code;
    }
}
