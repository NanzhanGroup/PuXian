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
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    if (px_is_truthy(px_ge(px_get_global("g_pos"), px_get_global("g_len")))) {
        return px_str("");
    }
    LXValue _v5 = px_index(px_get_global("g_src"), px_get_global("g_pos"));
    px_set_global("g_pos", px_add(px_get_global("g_pos"), px_int(1LL)));
    if (px_is_truthy(px_eq(_v5, px_str("\n")))) {
        px_set_global("g_line", px_add(px_get_global("g_line"), px_int(1LL)));
        px_set_global("g_col", px_int(1LL));
    }
    else {
        px_set_global("g_col", px_add(px_get_global("g_col"), px_int(1LL)));
    }
    return _v5;
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
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
    LXValue px_err_52_val = px_null();
    int px_err_52_proped = 0;
    LXValue _v53 = px_str("0123456789abcdefABCDEF");
    LXValue _v54 = px_int(0LL);
    while (px_is_truthy(px_lt(_v54, px_call(px_get_global("len"), (LXValue[]){_v53}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v53, _v54), _v50))) {
            if (px_is_truthy(px_ge(_v54, px_int(16LL)))) {
                return px_sub(_v54, px_int(6LL));
            }
            return _v54;
        }
         _v54 = px_add(_v54, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_52:
    if (px_err_52_proped) return px_err_52_val;
    return px_null();
}

static LXValue fn_handle_line_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v55 = px_null();
    LXValue px_err_56_val = px_null();
    int px_err_56_proped = 0;
    LXValue _v57 = px_int(0LL);
    while (px_is_truthy(px_bool(true))) {
         _v57 = px_int(0LL);
        while (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(" ")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
             _v57 = px_add(_v57, px_int(1LL));
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\t")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1003"), px_str("缩进禁止使用 tab，请使用空格")}, 2));
        }
        LXValue _v58 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v58, px_str("")))) {
            return px_null();
        }
        if (px_is_truthy(px_eq(_v58, px_str("\n")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            continue;
        }
        if (px_is_truthy(px_eq(_v58, px_str("#")))) {
            (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    LXValue _v59 = px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)));
    if (px_is_truthy(px_gt(_v57, _v59))) {
        (void)(px_method(px_get_global("g_indent_stack"), "append", (LXValue[]){_v57}, 1));
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("缩进"), px_str("")}, 2));
    }
    else if (px_is_truthy(px_lt(_v57, _v59))) {
        while (px_is_truthy(px_gt(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v57))) {
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        if (px_is_truthy(px_ne(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v57))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E2002"), px_add(px_add(px_str("缩进不一致：当前缩进 "), px_call(px_get_global("str"), (LXValue[]){_v57}, 1)), px_str(" 与上层缩进不匹配"))}, 2));
        }
    }
    px_set_global("g_at_line_start", px_bool(false));
px_err_56:
    if (px_err_56_proped) return px_err_56_val;
    return px_null();
}

static LXValue fn_skip_comment(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v60 = px_null();
    LXValue _v61 = px_null();
    LXValue px_err_62_val = px_null();
    int px_err_62_proped = 0;
    LXValue _v63 = px_str("");
    if (px_is_truthy(px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")))) {
         _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
         _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        LXValue _v64 = px_int(1LL);
        while (px_is_truthy(px_gt(_v64, px_int(0LL)))) {
            LXValue _v65 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            if (px_is_truthy(px_eq(_v65, px_str("")))) {
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1002"), px_str("块注释未闭合（缺少 |#）")}, 2));
            }
            else if (px_is_truthy(({ LXValue _t66 = px_eq(_v65, px_str("#")); px_is_truthy(_t66) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")) : _t66; }))) {
                 _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v64 = px_add(_v64, px_int(1LL));
            }
            else if (px_is_truthy(({ LXValue _t67 = px_eq(_v65, px_str("|")); px_is_truthy(_t67) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("#")) : _t67; }))) {
                 _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                 _v64 = px_sub(_v64, px_int(1LL));
            }
            else {
                 _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
        }
    }
    else {
        while (px_is_truthy(({ LXValue _t68 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t68) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\n")) : _t68; }))) {
             _v63 = px_add(_v63, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    return _v63;
px_err_62:
    if (px_err_62_proped) return px_err_62_val;
    return px_null();
}

static LXValue fn_scan_ident_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v69 = px_null();
    LXValue px_err_70_val = px_null();
    int px_err_70_proped = 0;
    LXValue _v71 = px_get_global("g_line");
    LXValue _v72 = px_get_global("g_col");
    LXValue _v73 = px_str("");
    while (px_is_truthy(px_call(px_get_global("is_ident_continue"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
         _v73 = px_add(_v73, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    if (px_is_truthy(px_method(px_get_global("KEYWORDS"), "has", (LXValue[]){_v73}, 1))) {
        return px_list_n((LXValue[]){px_index(px_get_global("KEYWORDS"), _v73), px_str(""), _v71, _v72}, 4);
    }
    return px_list_n((LXValue[]){px_str("标识符"), _v73, _v71, _v72}, 4);
px_err_70:
    if (px_err_70_proped) return px_err_70_val;
    return px_null();
}

static LXValue fn_scan_radix_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v74 = (nargs > 0) ? args[0] : px_null();
    LXValue _v75 = (nargs > 1) ? args[1] : px_null();
    LXValue _v76 = (nargs > 2) ? args[2] : px_null();
    LXValue _v77 = px_null();
    LXValue _v78 = px_null();
    LXValue _v79 = px_null();
    LXValue _v80 = px_null();
    LXValue _v81 = px_null();
    LXValue px_err_82_val = px_null();
    int px_err_82_proped = 0;
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    LXValue _v83 = px_str("");
    while (px_is_truthy(({ LXValue _t84 = px_call(px_get_global("is_alnum"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t84) ? _t84 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
         _v83 = px_add(_v83, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v83}, 1), px_int(0LL)))) {
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_str("进制字面量缺少数字（基数 "), px_call(px_get_global("str"), (LXValue[]){_v74}, 1)), px_str("）"))}, 2));
    }
    LXValue _v85 = px_str("");
    LXValue _v86 = px_int(0LL);
    while (px_is_truthy(px_lt(_v86, px_call(px_get_global("len"), (LXValue[]){_v83}, 1)))) {
        if (px_is_truthy(px_ne(px_index(_v83, _v86), px_str("_")))) {
             _v85 = px_add(_v85, px_index(_v83, _v86));
        }
         _v86 = px_add(_v86, px_int(1LL));
    }
    LXValue _v87 = px_int(0LL);
    LXValue _v88 = px_int(0LL);
    while (px_is_truthy(px_lt(_v88, px_call(px_get_global("len"), (LXValue[]){_v85}, 1)))) {
        LXValue _v89 = px_call(px_get_global("digit_val"), (LXValue[]){px_index(_v85, _v88)}, 1);
        if (px_is_truthy(({ LXValue _t90 = px_lt(_v89, px_int(0LL)); px_is_truthy(_t90) ? _t90 : px_ge(_v89, _v74); }))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_add(px_str("无效 "), px_call(px_get_global("str"), (LXValue[]){_v74}, 1)), px_str("-进制字面量: ")), _v85)}, 2));
        }
         _v87 = px_add(px_mul(_v87, _v74), _v89);
         _v88 = px_add(_v88, px_int(1LL));
    }
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){_v87}, 1), _v75, _v76}, 4);
px_err_82:
    if (px_err_82_proped) return px_err_82_val;
    return px_null();
}

static LXValue fn_strip_leading_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v91 = (nargs > 0) ? args[0] : px_null();
    LXValue _v92 = px_null();
    LXValue px_err_93_val = px_null();
    int px_err_93_proped = 0;
    LXValue _v94 = px_int(0LL);
    while (px_is_truthy(({ LXValue _t95 = px_lt(_v94, px_sub(px_call(px_get_global("len"), (LXValue[]){_v91}, 1), px_int(1LL))); px_is_truthy(_t95) ? px_eq(px_index(_v91, _v94), px_str("0")) : _t95; }))) {
         _v94 = px_add(_v94, px_int(1LL));
    }
    return px_slice(_v91, _v94, px_null(), px_null());
px_err_93:
    if (px_err_93_proped) return px_err_93_val;
    return px_null();
}

static LXValue fn_scan_number_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v96 = px_null();
    LXValue _v97 = px_null();
    LXValue _v98 = px_null();
    LXValue _v99 = px_null();
    LXValue _v100 = px_null();
    LXValue _v101 = px_null();
    LXValue px_err_102_val = px_null();
    int px_err_102_proped = 0;
    LXValue _v103 = px_get_global("g_line");
    LXValue _v104 = px_get_global("g_col");
    if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("0")))) {
        LXValue _v105 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
        if (px_is_truthy(({ LXValue _t106 = px_eq(_v105, px_str("x")); px_is_truthy(_t106) ? _t106 : px_eq(_v105, px_str("X")); }))) {
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(16LL), _v103, _v104}, 3);
        }
        if (px_is_truthy(({ LXValue _t107 = px_eq(_v105, px_str("b")); px_is_truthy(_t107) ? _t107 : px_eq(_v105, px_str("B")); }))) {
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(2LL), _v103, _v104}, 3);
        }
        if (px_is_truthy(({ LXValue _t108 = px_eq(_v105, px_str("o")); px_is_truthy(_t108) ? _t108 : px_eq(_v105, px_str("O")); }))) {
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(8LL), _v103, _v104}, 3);
        }
    }
    LXValue _v109 = px_str("");
    while (px_is_truthy(({ LXValue _t110 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t110) ? _t110 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
         _v109 = px_add(_v109, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    LXValue _v111 = px_bool(false);
    if (px_is_truthy(({ LXValue _t112 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")); px_is_truthy(_t112) ? px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek2"), (LXValue[]){}, 0)}, 1) : _t112; }))) {
         _v111 = px_bool(true);
         _v109 = px_add(_v109, px_str("."));
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        while (px_is_truthy(({ LXValue _t113 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t113) ? _t113 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
             _v109 = px_add(_v109, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    if (px_is_truthy(({ LXValue _t114 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("e")); px_is_truthy(_t114) ? _t114 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("E")); }))) {
        LXValue _v115 = px_get_global("g_pos");
        LXValue _v116 = px_get_global("g_line");
        LXValue _v117 = px_get_global("g_col");
        LXValue _v118 = px_str("");
         _v118 = px_add(_v118, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t119 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("+")); px_is_truthy(_t119) ? _t119 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")); }))) {
             _v118 = px_add(_v118, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
            while (px_is_truthy(({ LXValue _t120 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t120) ? _t120 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
                 _v118 = px_add(_v118, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
             _v109 = px_add(_v109, _v118);
             _v111 = px_bool(true);
        }
        else {
            px_set_global("g_pos", _v115);
            px_set_global("g_line", _v116);
            px_set_global("g_col", _v117);
        }
    }
    LXValue _v121 = px_str("");
    LXValue _v122 = px_int(0LL);
    while (px_is_truthy(px_lt(_v122, px_call(px_get_global("len"), (LXValue[]){_v109}, 1)))) {
        if (px_is_truthy(px_ne(px_index(_v109, _v122), px_str("_")))) {
             _v121 = px_add(_v121, px_index(_v109, _v122));
        }
         _v122 = px_add(_v122, px_int(1LL));
    }
    if (px_is_truthy(_v111)) {
        LXValue _v123 = px_call(px_get_global("float"), (LXValue[]){_v121}, 1);
        LXValue _v124 = px_call(px_get_global("str"), (LXValue[]){_v123}, 1);
        if (px_is_truthy(px_call(px_get_global("ends_with"), (LXValue[]){_v124, px_str(".0")}, 2))) {
             _v124 = px_slice(_v124, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v124}, 1), px_int(2LL)), px_null());
        }
        return px_list_n((LXValue[]){px_str("浮点"), _v124, _v103, _v104}, 4);
    }
    LXValue _v125 = px_call(px_get_global("strip_leading_zeros"), (LXValue[]){_v121}, 1);
    if (px_is_truthy(({ LXValue _t127 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v125}, 1), px_int(19LL)); px_is_truthy(_t127) ? _t127 : ({ LXValue _t126 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v125}, 1), px_int(19LL)); px_is_truthy(_t126) ? px_gt(_v125, px_str("9223372036854775807")) : _t126; }); }))) {
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_str("无效整数: "), _v109)}, 2));
    }
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v121}, 1)}, 1), _v103, _v104}, 4);
px_err_102:
    if (px_err_102_proped) return px_err_102_val;
    return px_null();
}

static LXValue fn_scan_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v128 = (nargs > 0) ? args[0] : px_null();
    LXValue _v129 = px_null();
    LXValue px_err_130_val = px_null();
    int px_err_130_proped = 0;
    LXValue _v131 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){_v128}, 1);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_index(_v131, px_int(0LL))}, 1));
    LXValue _v132 = px_int(1LL);
    while (px_is_truthy(px_lt(_v132, px_call(px_get_global("len"), (LXValue[]){_v131}, 1)))) {
        (void)(px_method(px_get_global("g_pending"), "append", (LXValue[]){px_index(_v131, _v132)}, 1));
         _v132 = px_add(_v132, px_int(1LL));
    }
px_err_130:
    if (px_err_130_proped) return px_err_130_val;
    return px_null();
}

static LXValue fn_scan_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v133 = (nargs > 0) ? args[0] : px_null();
    LXValue _v134 = px_null();
    LXValue _v135 = px_null();
    LXValue px_err_136_val = px_null();
    int px_err_136_proped = 0;
    LXValue _v137 = px_get_global("g_line");
    LXValue _v138 = px_get_global("g_col");
    LXValue _v139 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t141 = ({ LXValue _t140 = px_eq(_v139, px_str("\"")); px_is_truthy(_t140) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\"")) : _t140; }); px_is_truthy(_t141) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")) : _t141; }))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("\""), _v137, _v138, _v133}, 4);
    }
    if (px_is_truthy(({ LXValue _t143 = ({ LXValue _t142 = px_eq(_v139, px_str("'")); px_is_truthy(_t142) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("'")) : _t142; }); px_is_truthy(_t143) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")) : _t143; }))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("'"), _v137, _v138, _v133}, 4);
    }
    LXValue _v144 = px_list_n((LXValue[]){}, 0);
    LXValue _v145 = px_str("");
    while (px_is_truthy(px_bool(true))) {
        LXValue _v146 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v146, px_str("")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_add(px_add(px_str("字符串未闭合（缺少 "), _v139), px_str("）")), _v137, _v138}, 4));
        }
        if (px_is_truthy(px_eq(_v146, _v139))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            break;
        }
        if (px_is_truthy(px_eq(_v146, px_str("\\")))) {
             _v145 = px_add(_v145, px_call(px_get_global("scan_escape"), (LXValue[]){_v137, _v138}, 2));
        }
        else if (px_is_truthy(px_eq(_v146, px_str("\n")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v137, _v138}, 4));
        }
        else if (px_is_truthy(({ LXValue _t148 = ({ LXValue _t147 = px_eq(_v146, px_str("$")); px_is_truthy(_t147) ? _v133 : _t147; }); px_is_truthy(_t148) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t148; }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v145}, 1), _v137, _v138}, 4)}, 1));
            (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v137, _v138}, 4)}, 1));
            (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v137, _v138}, 4)}, 1));
            (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v137, _v138}, 4)}, 1));
            LXValue _v149 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v137, _v138}, 2);
            LXValue _v150 = px_int(0LL);
            while (px_is_truthy(px_lt(_v150, px_call(px_get_global("len"), (LXValue[]){_v149}, 1)))) {
                (void)(px_method(_v144, "append", (LXValue[]){px_index(_v149, _v150)}, 1));
                 _v150 = px_add(_v150, px_int(1LL));
            }
            (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v137, _v138}, 4)}, 1));
            (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v137, _v138}, 4)}, 1));
             _v145 = px_str("");
        }
        else {
             _v145 = px_add(_v145, _v146);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    (void)(px_method(_v144, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v145}, 1), _v137, _v138}, 4)}, 1));
    return _v144;
px_err_136:
    if (px_err_136_proped) return px_err_136_val;
    return px_null();
}

static LXValue fn_scan_multiline_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v151 = (nargs > 0) ? args[0] : px_null();
    LXValue _v152 = (nargs > 1) ? args[1] : px_null();
    LXValue _v153 = (nargs > 2) ? args[2] : px_null();
    LXValue _v154 = (nargs > 3) ? args[3] : px_null();
    LXValue _v155 = px_null();
    LXValue _v156 = px_null();
    LXValue px_err_157_val = px_null();
    int px_err_157_proped = 0;
    LXValue _v158 = px_list_n((LXValue[]){}, 0);
    LXValue _v159 = px_str("");
    while (px_is_truthy(px_bool(true))) {
        LXValue _v160 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v160, px_str("")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("多行字符串未闭合"), _v152, _v153}, 4));
        }
        if (px_is_truthy(({ LXValue _t162 = ({ LXValue _t161 = px_eq(_v160, _v151); px_is_truthy(_t161) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), _v151) : _t161; }); px_is_truthy(_t162) ? px_eq(px_call(px_get_global("peek3"), (LXValue[]){}, 0), _v151) : _t162; }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            break;
        }
        if (px_is_truthy(px_eq(_v160, px_str("\\")))) {
             _v159 = px_add(_v159, px_call(px_get_global("scan_escape"), (LXValue[]){_v152, _v153}, 2));
        }
        else if (px_is_truthy(({ LXValue _t164 = ({ LXValue _t163 = px_eq(_v160, px_str("$")); px_is_truthy(_t163) ? _v154 : _t163; }); px_is_truthy(_t164) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t164; }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v159}, 1), _v152, _v153}, 4)}, 1));
            (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v152, _v153}, 4)}, 1));
            (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v152, _v153}, 4)}, 1));
            (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v152, _v153}, 4)}, 1));
            LXValue _v165 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v152, _v153}, 2);
            LXValue _v166 = px_int(0LL);
            while (px_is_truthy(px_lt(_v166, px_call(px_get_global("len"), (LXValue[]){_v165}, 1)))) {
                (void)(px_method(_v158, "append", (LXValue[]){px_index(_v165, _v166)}, 1));
                 _v166 = px_add(_v166, px_int(1LL));
            }
            (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v152, _v153}, 4)}, 1));
            (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v152, _v153}, 4)}, 1));
             _v159 = px_str("");
        }
        else {
             _v159 = px_add(_v159, _v160);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    (void)(px_method(_v158, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v159}, 1), _v152, _v153}, 4)}, 1));
    return _v158;
px_err_157:
    if (px_err_157_proped) return px_err_157_val;
    return px_null();
}

static LXValue fn_scan_interp_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v167 = (nargs > 0) ? args[0] : px_null();
    LXValue _v168 = (nargs > 1) ? args[1] : px_null();
    LXValue _v169 = px_null();
    LXValue px_err_170_val = px_null();
    int px_err_170_proped = 0;
    LXValue _v171 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v172 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v172, px_str("")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值 ${ 未闭合（缺少 }）"), _v167, _v168}, 4));
        }
        if (px_is_truthy(px_eq(_v172, px_str("}")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            break;
        }
        if (px_is_truthy(px_eq(_v172, px_str("{")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持 {} 字面量（dict/set），请先用变量保存")}, 2));
        }
        if (px_is_truthy(px_eq(_v172, px_str("\n")))) {
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值表达式不能跨行"), _v167, _v168}, 4));
        }
        if (px_is_truthy(px_eq(_v172, px_str("#")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持注释")}, 2));
        }
        if (px_is_truthy(({ LXValue _t173 = px_eq(_v172, px_str(" ")); px_is_truthy(_t173) ? _t173 : px_eq(_v172, px_str("\t")); }))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        else if (px_is_truthy(({ LXValue _t174 = px_eq(_v172, px_str("\"")); px_is_truthy(_t174) ? _t174 : px_eq(_v172, px_str("'")); }))) {
            LXValue _v175 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){px_bool(true)}, 1);
            LXValue _v176 = px_int(0LL);
            while (px_is_truthy(px_lt(_v176, px_call(px_get_global("len"), (LXValue[]){_v175}, 1)))) {
                (void)(px_method(_v171, "append", (LXValue[]){px_index(_v175, _v176)}, 1));
                 _v176 = px_add(_v176, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v172}, 1))) {
            (void)(px_method(_v171, "append", (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v172}, 1))) {
            (void)(px_method(_v171, "append", (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        }
        else {
            (void)(px_method(_v171, "append", (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
        }
    }
    return _v171;
px_err_170:
    if (px_err_170_proped) return px_err_170_val;
    return px_null();
}

static LXValue fn_hex_to_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v177 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_178_val = px_null();
    int px_err_178_proped = 0;
    LXValue _v179 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v177}, 1);
    if (px_is_truthy(px_le(_v179, px_int(127LL)))) {
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){_v179, px_int(1LL)}, 2)}, 1);
    }
    if (px_is_truthy(px_le(_v179, px_int(2047LL)))) {
        LXValue _v180 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(192LL), px_shr(_v179, px_int(6LL))), px_int(1LL)}, 2);
        LXValue _v181 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v179, px_int(63LL))), px_int(1LL)}, 2);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v180, _v181}, 2)}, 1);
    }
    if (px_is_truthy(px_le(_v179, px_int(65535LL)))) {
        LXValue _v182 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(224LL), px_shr(_v179, px_int(12LL))), px_int(1LL)}, 2);
        LXValue _v183 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v179, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
        LXValue _v184 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v179, px_int(63LL))), px_int(1LL)}, 2);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v182, _v183, _v184}, 3)}, 1);
    }
    LXValue _v185 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(240LL), px_shr(_v179, px_int(18LL))), px_int(1LL)}, 2);
    LXValue _v186 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v179, px_int(12LL)), px_int(63LL))), px_int(1LL)}, 2);
    LXValue _v187 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v179, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
    LXValue _v188 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v179, px_int(63LL))), px_int(1LL)}, 2);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v185, _v186, _v187, _v188}, 4)}, 1);
px_err_178:
    if (px_err_178_proped) return px_err_178_val;
    return px_null();
}

static LXValue fn_scan_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v189 = (nargs > 0) ? args[0] : px_null();
    LXValue _v190 = (nargs > 1) ? args[1] : px_null();
    LXValue _v191 = px_null();
    LXValue px_err_192_val = px_null();
    int px_err_192_proped = 0;
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    LXValue _v193 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v193, px_str("")))) {
        (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串在转义序列处意外结束"), _v189, _v190}, 4));
    }
    if (px_is_truthy(px_eq(_v193, px_str("n")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\n");
    }
    if (px_is_truthy(px_eq(_v193, px_str("t")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\t");
    }
    if (px_is_truthy(px_eq(_v193, px_str("r")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\r");
    }
    if (px_is_truthy(px_eq(_v193, px_str("\\")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\\");
    }
    if (px_is_truthy(px_eq(_v193, px_str("\"")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("\"");
    }
    if (px_is_truthy(px_eq(_v193, px_str("'")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("'");
    }
    if (px_is_truthy(px_eq(_v193, px_str("0")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("");
    }
    if (px_is_truthy(px_eq(_v193, px_str("$")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_str("$");
    }
    if (px_is_truthy(px_eq(_v193, px_str("u")))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("{")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义须为 \\u{XXXX} 形式")}, 2));
        }
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        LXValue _v194 = px_str("");
        while (px_is_truthy(({ LXValue _t195 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t195) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")) : _t195; }))) {
            LXValue _v196 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            if (px_is_truthy(px_call(px_get_global("is_hex_digit"), (LXValue[]){_v196}, 1))) {
                 _v194 = px_add(_v194, _v196);
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
        if (px_is_truthy(px_eq(_v194, px_str("")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        LXValue _v197 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v194}, 1);
        if (px_is_truthy(({ LXValue _t198 = px_eq(_v197, px_null()); px_is_truthy(_t198) ? _t198 : px_gt(_v197, px_int(4294967295LL)); }))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        if (px_is_truthy(({ LXValue _t200 = px_gt(_v197, px_int(1114111LL)); px_is_truthy(_t200) ? _t200 : ({ LXValue _t199 = px_ge(_v197, px_int(55296LL)); px_is_truthy(_t199) ? px_le(_v197, px_int(57343LL)) : _t199; }); }))) {
            return px_call(px_get_global("hex_to_char"), (LXValue[]){px_str("FFFD")}, 1);
        }
        return px_call(px_get_global("hex_to_char"), (LXValue[]){_v194}, 1);
    }
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_add(px_str("非法转义序列 \\"), _v193)}, 2));
    return px_str("");
px_err_192:
    if (px_err_192_proped) return px_err_192_val;
    return px_null();
}

static LXValue fn_int_to_hex_nopad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v201 = (nargs > 0) ? args[0] : px_null();
    LXValue _v202 = px_null();
    LXValue px_err_203_val = px_null();
    int px_err_203_proped = 0;
    LXValue _v204 = px_str("0123456789abcdef");
    LXValue _v205 = px_str("");
    while (px_is_truthy(px_gt(_v201, px_int(0LL)))) {
         _v205 = px_add(px_index(_v204, px_mod(_v201, px_int(16LL))), _v205);
         _v201 = px_idiv(_v201, px_int(16LL));
    }
    if (px_is_truthy(px_eq(_v205, px_str("")))) {
        return px_str("0");
    }
    return _v205;
px_err_203:
    if (px_err_203_proped) return px_err_203_val;
    return px_null();
}

static LXValue fn_char_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v206 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_207_val = px_null();
    int px_err_207_proped = 0;
    if (px_is_truthy(px_eq(_v206, px_str("'")))) {
        return px_str("'\\''");
    }
    if (px_is_truthy(px_eq(_v206, px_str("\\")))) {
        return px_str("'\\\\'");
    }
    if (px_is_truthy(px_eq(_v206, px_str("\n")))) {
        return px_str("'\\n'");
    }
    if (px_is_truthy(px_eq(_v206, px_str("\r")))) {
        return px_str("'\\r'");
    }
    if (px_is_truthy(px_eq(_v206, px_str("\t")))) {
        return px_str("'\\t'");
    }
    if (px_is_truthy(px_eq(_v206, px_str("")))) {
        return px_str("'\\0'");
    }
    LXValue _v208 = px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v206}, 1);
    if (px_is_truthy(px_ge(_v208, px_int(0LL)))) {
        return px_add(px_add(px_str("'\\u{"), px_call(px_get_global("int_to_hex_nopad"), (LXValue[]){_v208}, 1)), px_str("}'"));
    }
    return px_add(px_add(px_str("'"), _v206), px_str("'"));
px_err_207:
    if (px_err_207_proped) return px_err_207_val;
    return px_null();
}

static LXValue fn_rust_str_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v209 = (nargs > 0) ? args[0] : px_null();
    LXValue _v210 = px_null();
    LXValue _v211 = px_null();
    LXValue px_err_212_val = px_null();
    int px_err_212_proped = 0;
    LXValue _v213 = px_str("\"");
    LXValue _v214 = px_int(0LL);
    while (px_is_truthy(px_lt(_v214, px_call(px_get_global("len"), (LXValue[]){_v209}, 1)))) {
        LXValue _v215 = px_index(_v209, _v214);
        if (px_is_truthy(px_eq(_v215, px_str("\n")))) {
             _v213 = px_add(_v213, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v215, px_str("\t")))) {
             _v213 = px_add(_v213, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v215, px_str("\r")))) {
             _v213 = px_add(_v213, px_str("\\r"));
        }
        else if (px_is_truthy(px_lt(_v215, px_str("")))) {
             _v213 = px_add(_v213, px_str("\\0"));
        }
        else if (px_is_truthy(px_eq(_v215, px_str("\"")))) {
             _v213 = px_add(_v213, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v215, px_str("\\")))) {
             _v213 = px_add(_v213, px_str("\\\\"));
        }
        else if (px_is_truthy(({ LXValue _t216 = px_ge(_v215, px_str(" ")); px_is_truthy(_t216) ? px_le(_v215, px_str("~")) : _t216; }))) {
             _v213 = px_add(_v213, _v215);
        }
        else if (px_is_truthy(px_eq(_v215, px_str(" ")))) {
             _v213 = px_add(_v213, px_str("\\u{a0}"));
        }
        else if (px_is_truthy(px_gt(_v215, px_str(" ")))) {
             _v213 = px_add(_v213, _v215);
        }
        else {
             _v213 = px_add(_v213, px_add(px_add(px_str("\\u{"), px_call(px_get_global("ctrl_hex"), (LXValue[]){_v215}, 1)), px_str("}")));
        }
         _v214 = px_add(_v214, px_int(1LL));
    }
     _v213 = px_add(_v213, px_str("\""));
    return _v213;
px_err_212:
    if (px_err_212_proped) return px_err_212_val;
    return px_null();
}

static LXValue fn_ctrl_codepoint(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v217 = (nargs > 0) ? args[0] : px_null();
    LXValue _v218 = px_null();
    LXValue px_err_219_val = px_null();
    int px_err_219_proped = 0;
    LXValue _v220 = px_int(0LL);
    while (px_is_truthy(px_lt(_v220, px_call(px_get_global("len"), (LXValue[]){px_get_global("CTRL_ALL")}, 1)))) {
        if (px_is_truthy(px_eq(px_index(px_get_global("CTRL_ALL"), _v220), _v217))) {
            if (px_is_truthy(px_lt(_v220, px_int(28LL)))) {
                if (px_is_truthy(px_lt(_v220, px_int(8LL)))) {
                    return px_add(_v220, px_int(1LL));
                }
                if (px_is_truthy(px_lt(_v220, px_int(10LL)))) {
                    return px_add(_v220, px_int(3LL));
                }
                return px_add(_v220, px_int(4LL));
            }
            return px_add(_v220, px_int(99LL));
        }
         _v220 = px_add(_v220, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_219:
    if (px_err_219_proped) return px_err_219_val;
    return px_null();
}

static LXValue fn_ctrl_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v221 = (nargs > 0) ? args[0] : px_null();
    LXValue _v222 = px_null();
    LXValue px_err_223_val = px_null();
    int px_err_223_proped = 0;
    LXValue _v224 = px_call(px_get_global("int_to_hex"), (LXValue[]){px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v221}, 1), px_int(16LL)}, 2);
    LXValue _v225 = px_int(0LL);
    while (px_is_truthy(({ LXValue _t226 = px_lt(_v225, px_call(px_get_global("len"), (LXValue[]){_v224}, 1)); px_is_truthy(_t226) ? px_eq(px_index(_v224, _v225), px_str("0")) : _t226; }))) {
         _v225 = px_add(_v225, px_int(1LL));
    }
    if (px_is_truthy(px_eq(_v225, px_call(px_get_global("len"), (LXValue[]){_v224}, 1)))) {
        return px_str("0");
    }
    return px_slice(_v224, _v225, px_call(px_get_global("len"), (LXValue[]){_v224}, 1), px_null());
px_err_223:
    if (px_err_223_proped) return px_err_223_val;
    return px_null();
}

static LXValue fn_scan_operator_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_227_val = px_null();
    int px_err_227_proped = 0;
    LXValue _v228 = px_get_global("g_line");
    LXValue _v229 = px_get_global("g_col");
    LXValue _v230 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v230, px_str("(")))) {
        return px_list_n((LXValue[]){px_str("("), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str(")")))) {
        return px_list_n((LXValue[]){px_str(")"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("[")))) {
        return px_list_n((LXValue[]){px_str("["), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("]")))) {
        return px_list_n((LXValue[]){px_str("]"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("{")))) {
        return px_list_n((LXValue[]){px_str("{"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("}")))) {
        return px_list_n((LXValue[]){px_str("}"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str(",")))) {
        return px_list_n((LXValue[]){px_str(","), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str(":")))) {
        return px_list_n((LXValue[]){px_str(":"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str(".")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_str("运算符 '..' 未定义（range 语法尚未支持）")}, 2));
        }
        return px_list_n((LXValue[]){px_str("."), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("+")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("+="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("+"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("-")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("->"), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("-="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("-"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("*")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("*")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str("**="), px_str(""), _v228, _v229}, 4);
            }
            return px_list_n((LXValue[]){px_str("**"), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("*="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("*"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("/")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("/")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str("//="), px_str(""), _v228, _v229}, 4);
            }
            return px_list_n((LXValue[]){px_str("//"), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("/="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("/"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("%")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("%="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("%"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("^")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("^="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("^"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("~")))) {
        return px_list_n((LXValue[]){px_str("~"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("&")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("&="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("&"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("|")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("|>"), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("|="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("|"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("=")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("=="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("="), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("!")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("!="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("!"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("<")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("<")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str("<<="), px_str(""), _v228, _v229}, 4);
            }
            return px_list_n((LXValue[]){px_str("<<"), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("<="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("<"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str(">")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                    return px_list_n((LXValue[]){px_str(">>>="), px_str(""), _v228, _v229}, 4);
                }
                return px_list_n((LXValue[]){px_str(">>>"), px_str(""), _v228, _v229}, 4);
            }
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                return px_list_n((LXValue[]){px_str(">>="), px_str(""), _v228, _v229}, 4);
            }
            return px_list_n((LXValue[]){px_str(">>"), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str(">="), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str(">"), px_str(""), _v228, _v229}, 4);
    }
    if (px_is_truthy(px_eq(_v230, px_str("?")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("?."), px_str(""), _v228, _v229}, 4);
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("?")))) {
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            return px_list_n((LXValue[]){px_str("??"), px_str(""), _v228, _v229}, 4);
        }
        return px_list_n((LXValue[]){px_str("?"), px_str(""), _v228, _v229}, 4);
    }
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_add(px_str("非法字符: "), px_call(px_get_global("char_debug"), (LXValue[]){_v230}, 1))}, 2));
    return px_list_n((LXValue[]){px_str(""), px_str(""), _v228, _v229}, 4);
px_err_227:
    if (px_err_227_proped) return px_err_227_val;
    return px_null();
}

static LXValue fn_next_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_231_val = px_null();
    int px_err_231_proped = 0;
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_int(0LL)))) {
        LXValue _v232 = px_index(px_get_global("g_pending"), px_int(0LL));
        px_set_global("g_pending", px_slice(px_get_global("g_pending"), px_int(1LL), px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_null()));
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){_v232}, 1));
        return px_bool(true);
    }
    if (px_is_truthy(px_get_global("g_at_line_start"))) {
        (void)(px_call(px_get_global("handle_line_start"), (LXValue[]){}, 0));
    }
    LXValue _v233 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v233, px_str("")))) {
        while (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)))) {
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("EOF"), px_str("")}, 2));
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(_v233, px_str("\n")))) {
        LXValue _v234 = px_get_global("g_line");
        LXValue _v235 = px_get_global("g_col");
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_set_global("g_at_line_start", px_bool(true));
        (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_str("换行"), px_str(""), _v234, _v235}, 4));
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t236 = px_eq(_v233, px_str(" ")); px_is_truthy(_t236) ? _t236 : px_eq(_v233, px_str("\t")); }))) {
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        return px_bool(true);
    }
    if (px_is_truthy(px_eq(_v233, px_str("#")))) {
        (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t237 = px_eq(_v233, px_str("\"")); px_is_truthy(_t237) ? _t237 : px_eq(_v233, px_str("'")); }))) {
        (void)(px_call(px_get_global("scan_string"), (LXValue[]){px_bool(true)}, 1));
        return px_bool(true);
    }
    if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v233}, 1))) {
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        return px_bool(true);
    }
    if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v233}, 1))) {
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        return px_bool(true);
    }
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
    return px_bool(true);
px_err_231:
    if (px_err_231_proped) return px_err_231_val;
    return px_null();
}

static LXValue fn_lex_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v238 = (nargs > 0) ? args[0] : px_null();
    LXValue _v239 = px_null();
    LXValue px_err_240_val = px_null();
    int px_err_240_proped = 0;
    px_set_global("g_src", _v238);
    px_set_global("g_len", px_call(px_get_global("len"), (LXValue[]){_v238}, 1));
    px_set_global("g_pos", px_int(0LL));
    px_set_global("g_line", px_int(1LL));
    px_set_global("g_col", px_int(1LL));
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_set_global("g_at_line_start", px_bool(true));
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_set_global("g_count", px_int(0LL));
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    LXValue _v241 = px_bool(true);
    while (px_is_truthy(_v241)) {
         _v241 = px_call(px_get_global("next_token"), (LXValue[]){}, 0);
    }
    return px_get_global("g_toks");
px_err_240:
    if (px_err_240_proped) return px_err_240_val;
    return px_null();
}

static LXValue fn_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v242 = (nargs > 0) ? args[0] : px_null();
    LXValue _v243 = px_null();
    LXValue _v244 = px_null();
    LXValue px_err_245_val = px_null();
    int px_err_245_proped = 0;
    LXValue _v246 = px_str("");
    LXValue _v247 = px_int(0LL);
    while (px_is_truthy(px_lt(_v247, _v242))) {
         _v246 = px_add(_v246, px_str(" "));
         _v247 = px_add(_v247, px_int(1LL));
    }
    return _v246;
px_err_245:
    if (px_err_245_proped) return px_err_245_val;
    return px_null();
}

static LXValue fn_dump_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v248 = (nargs > 0) ? args[0] : px_null();
    LXValue _v249 = (nargs > 1) ? args[1] : px_null();
    LXValue _v250 = px_null();
    LXValue px_err_251_val = px_null();
    int px_err_251_proped = 0;
    LXValue _v252 = px_index(_v248, px_int(0LL));
    LXValue _v253 = px_index(px_get_global("LAYOUT"), _v252);
    LXValue _v254 = px_index(_v253, px_int(0LL));
    LXValue _v255 = px_index(_v253, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v255}, 1), px_int(0LL)))) {
        return _v254;
    }
    LXValue _v256 = px_eq(px_index(px_index(_v255, px_int(0LL)), px_int(0LL)), px_null());
    LXValue _v257 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(_v256)) {
        (void)(px_method(_v257, "append", (LXValue[]){px_add(_v254, px_str("("))}, 1));
    }
    else {
        (void)(px_method(_v257, "append", (LXValue[]){px_add(_v254, px_str(" {"))}, 1));
    }
    LXValue _v258 = px_int(0LL);
    while (px_is_truthy(px_lt(_v258, px_call(px_get_global("len"), (LXValue[]){_v255}, 1)))) {
        LXValue _v259 = px_index(_v255, _v258);
        LXValue _v260 = px_index(_v248, px_add(_v258, px_int(1LL)));
        LXValue _v261 = px_call(px_get_global("dump_field"), (LXValue[]){_v260, px_index(_v259, px_int(1LL)), px_add(_v249, px_int(4LL))}, 3);
        if (px_is_truthy(_v256)) {
            (void)(px_method(_v257, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v249, px_int(4LL))}, 1), _v261), px_str(","))}, 1));
        }
        else {
            (void)(px_method(_v257, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v249, px_int(4LL))}, 1), px_index(_v259, px_int(0LL))), px_str(": ")), _v261), px_str(","))}, 1));
        }
         _v258 = px_add(_v258, px_int(1LL));
    }
    if (px_is_truthy(_v256)) {
        (void)(px_method(_v257, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v249}, 1), px_str(")"))}, 1));
    }
    else {
        (void)(px_method(_v257, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v249}, 1), px_str("}"))}, 1));
    }
    return px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v257}, 2);
px_err_251:
    if (px_err_251_proped) return px_err_251_val;
    return px_null();
}

static LXValue fn_dump_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v262 = (nargs > 0) ? args[0] : px_null();
    LXValue _v263 = (nargs > 1) ? args[1] : px_null();
    LXValue _v264 = px_null();
    LXValue px_err_265_val = px_null();
    int px_err_265_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v262}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v266 = px_list_n((LXValue[]){}, 0);
    LXValue _v267 = px_int(0LL);
    while (px_is_truthy(px_lt(_v267, px_call(px_get_global("len"), (LXValue[]){_v262}, 1)))) {
        (void)(px_method(_v266, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v263, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v262, _v267), px_add(_v263, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v267 = px_add(_v267, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v266}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v263}, 1)), px_str("]"));
px_err_265:
    if (px_err_265_proped) return px_err_265_val;
    return px_null();
}

static LXValue fn_dump_str_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v268 = (nargs > 0) ? args[0] : px_null();
    LXValue _v269 = (nargs > 1) ? args[1] : px_null();
    LXValue _v270 = px_null();
    LXValue px_err_271_val = px_null();
    int px_err_271_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v268}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v272 = px_list_n((LXValue[]){}, 0);
    LXValue _v273 = px_int(0LL);
    while (px_is_truthy(px_lt(_v273, px_call(px_get_global("len"), (LXValue[]){_v268}, 1)))) {
        (void)(px_method(_v272, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v269, px_int(4LL))}, 1), px_index(_v268, _v273)), px_str(","))}, 1));
         _v273 = px_add(_v273, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v272}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v269}, 1)), px_str("]"));
px_err_271:
    if (px_err_271_proped) return px_err_271_val;
    return px_null();
}

static LXValue fn_dump_ty_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v274 = (nargs > 0) ? args[0] : px_null();
    LXValue _v275 = (nargs > 1) ? args[1] : px_null();
    LXValue _v276 = px_null();
    LXValue px_err_277_val = px_null();
    int px_err_277_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v274}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v278 = px_list_n((LXValue[]){}, 0);
    LXValue _v279 = px_int(0LL);
    while (px_is_truthy(px_lt(_v279, px_call(px_get_global("len"), (LXValue[]){_v274}, 1)))) {
        (void)(px_method(_v278, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v275, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v274, _v279), px_add(_v275, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v279 = px_add(_v279, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v278}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v275}, 1)), px_str("]"));
px_err_277:
    if (px_err_277_proped) return px_err_277_val;
    return px_null();
}

static LXValue fn_dump_pat_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v280 = (nargs > 0) ? args[0] : px_null();
    LXValue _v281 = (nargs > 1) ? args[1] : px_null();
    LXValue _v282 = px_null();
    LXValue px_err_283_val = px_null();
    int px_err_283_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v280}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v284 = px_list_n((LXValue[]){}, 0);
    LXValue _v285 = px_int(0LL);
    while (px_is_truthy(px_lt(_v285, px_call(px_get_global("len"), (LXValue[]){_v280}, 1)))) {
        (void)(px_method(_v284, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v281, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v280, _v285), px_add(_v281, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v285 = px_add(_v285, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v284}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v281}, 1)), px_str("]"));
px_err_283:
    if (px_err_283_proped) return px_err_283_val;
    return px_null();
}

static LXValue fn_dump_opt_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v286 = (nargs > 0) ? args[0] : px_null();
    LXValue _v287 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_288_val = px_null();
    int px_err_288_proped = 0;
    if (px_is_truthy(px_eq(_v286, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v287, px_int(4LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){_v286, px_add(_v287, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v287}, 1)), px_str(")"));
px_err_288:
    if (px_err_288_proped) return px_err_288_val;
    return px_null();
}

static LXValue fn_dump_opt_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v289 = (nargs > 0) ? args[0] : px_null();
    LXValue _v290 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_291_val = px_null();
    int px_err_291_proped = 0;
    if (px_is_truthy(px_eq(_v289, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v290, px_int(4LL))}, 1)), _v289), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v290}, 1)), px_str(")"));
px_err_291:
    if (px_err_291_proped) return px_err_291_val;
    return px_null();
}

static LXValue fn_dump_opt_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v292 = (nargs > 0) ? args[0] : px_null();
    LXValue _v293 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_294_val = px_null();
    int px_err_294_proped = 0;
    if (px_is_truthy(px_eq(_v292, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v293, px_int(4LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){_v292, px_add(_v293, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v293}, 1)), px_str(")"));
px_err_294:
    if (px_err_294_proped) return px_err_294_val;
    return px_null();
}

static LXValue fn_dump_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v295 = (nargs > 0) ? args[0] : px_null();
    LXValue _v296 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_297_val = px_null();
    int px_err_297_proped = 0;
    return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v296, px_int(4LL))}, 1)), px_str("line: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v295, px_int(0LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v296, px_int(4LL))}, 1)), px_str("col: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v295, px_int(1LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v296}, 1)), px_str("}"));
px_err_297:
    if (px_err_297_proped) return px_err_297_val;
    return px_null();
}

static LXValue fn_dump_t2_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v298 = (nargs > 0) ? args[0] : px_null();
    LXValue _v299 = (nargs > 1) ? args[1] : px_null();
    LXValue _v300 = px_null();
    LXValue px_err_301_val = px_null();
    int px_err_301_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v298}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v302 = px_list_n((LXValue[]){}, 0);
    LXValue _v303 = px_int(0LL);
    while (px_is_truthy(px_lt(_v303, px_call(px_get_global("len"), (LXValue[]){_v298}, 1)))) {
        LXValue _v304 = px_index(_v298, _v303);
        (void)(px_method(_v302, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v299, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v299, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v304, px_int(0LL)), px_add(_v299, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v299, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v304, px_int(1LL)), px_add(_v299, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v299, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v303 = px_add(_v303, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v302}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v299}, 1)), px_str("]"));
px_err_301:
    if (px_err_301_proped) return px_err_301_val;
    return px_null();
}

static LXValue fn_dump_t2b_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v305 = (nargs > 0) ? args[0] : px_null();
    LXValue _v306 = (nargs > 1) ? args[1] : px_null();
    LXValue _v307 = px_null();
    LXValue px_err_308_val = px_null();
    int px_err_308_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v305}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v309 = px_list_n((LXValue[]){}, 0);
    LXValue _v310 = px_int(0LL);
    while (px_is_truthy(px_lt(_v310, px_call(px_get_global("len"), (LXValue[]){_v305}, 1)))) {
        LXValue _v311 = px_index(_v305, _v310);
        (void)(px_method(_v309, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v311, px_int(0LL)), px_add(_v306, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v311, px_int(1LL)), px_add(_v306, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v310 = px_add(_v310, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v309}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v306}, 1)), px_str("]"));
px_err_308:
    if (px_err_308_proped) return px_err_308_val;
    return px_null();
}

static LXValue fn_dump_t3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v312 = (nargs > 0) ? args[0] : px_null();
    LXValue _v313 = (nargs > 1) ? args[1] : px_null();
    LXValue _v314 = px_null();
    LXValue _v315 = px_null();
    LXValue px_err_316_val = px_null();
    int px_err_316_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v312}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v317 = px_list_n((LXValue[]){}, 0);
    LXValue _v318 = px_int(0LL);
    while (px_is_truthy(px_lt(_v318, px_call(px_get_global("len"), (LXValue[]){_v312}, 1)))) {
        LXValue _v319 = px_index(_v312, _v318);
        LXValue _v320 = px_index(_v319, px_int(0LL));
        LXValue _v321 = px_str("None");
        if (px_is_truthy(px_ne(_v320, px_null()))) {
             _v321 = px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(12LL))}, 1)), _v320), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(8LL))}, 1)), px_str(")"));
        }
        (void)(px_method(_v317, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(8LL))}, 1)), _v321), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v319, px_int(1LL)), px_add(_v313, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v319, px_int(2LL)), px_add(_v313, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v313, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v318 = px_add(_v318, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v317}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v313}, 1)), px_str("]"));
px_err_316:
    if (px_err_316_proped) return px_err_316_val;
    return px_null();
}

static LXValue fn_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v322 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_323_val = px_null();
    int px_err_323_proped = 0;
    LXValue _v324 = px_call(px_get_global("str"), (LXValue[]){_v322}, 1);
    if (px_is_truthy(({ LXValue _t325 = px_eq(_v324, px_str("inf")); px_is_truthy(_t325) ? _t325 : px_eq(_v324, px_str("-inf")); }))) {
        return _v324;
    }
    if (px_is_truthy(({ LXValue _t327 = ({ LXValue _t326 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v324, px_str(".")}, 2)); px_is_truthy(_t326) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v324, px_str("e")}, 2)) : _t326; }); px_is_truthy(_t327) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v324, px_str("E")}, 2)) : _t327; }))) {
        return px_add(_v324, px_str(".0"));
    }
    return _v324;
px_err_323:
    if (px_err_323_proped) return px_err_323_val;
    return px_null();
}

static LXValue fn_dump_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v328 = (nargs > 0) ? args[0] : px_null();
    LXValue _v329 = (nargs > 1) ? args[1] : px_null();
    LXValue _v330 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_331_val = px_null();
    int px_err_331_proped = 0;
    if (px_is_truthy(px_eq(_v329, px_str("s")))) {
        return _v328;
    }
    if (px_is_truthy(px_eq(_v329, px_str("r")))) {
        return px_call(px_get_global("str"), (LXValue[]){_v328}, 1);
    }
    if (px_is_truthy(px_eq(_v329, px_str("f")))) {
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v328}, 1);
    }
    if (px_is_truthy(px_eq(_v329, px_str("n")))) {
        return px_call(px_get_global("dump_node"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("o")))) {
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("os")))) {
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("ol")))) {
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("l")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("ls")))) {
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("tl")))) {
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lpl")))) {
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lp")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lsf")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lev")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lfd")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lc")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lma")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lt2")))) {
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lt2b")))) {
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("lt3")))) {
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v328, _v330}, 2);
    }
    if (px_is_truthy(px_eq(_v329, px_str("p")))) {
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v328, _v330}, 2);
    }
    return px_call(px_get_global("str"), (LXValue[]){_v328}, 1);
px_err_331:
    if (px_err_331_proped) return px_err_331_val;
    return px_null();
}

static LXValue fn_dump_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v332 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_333_val = px_null();
    int px_err_333_proped = 0;
    return px_call(px_get_global("dump_node"), (LXValue[]){_v332, px_int(0LL)}, 2);
px_err_333:
    if (px_err_333_proped) return px_err_333_val;
    return px_null();
}

static LXValue fn_pk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_334_val = px_null();
    int px_err_334_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
px_err_334:
    if (px_err_334_proped) return px_err_334_val;
    return px_null();
}

static LXValue fn_pk_display(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_335_val = px_null();
    int px_err_335_proped = 0;
    LXValue _v336 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
    LXValue _v337 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
    if (px_is_truthy(px_eq(_v336, px_str("整数")))) {
        return px_add(px_str("整数 "), _v337);
    }
    if (px_is_truthy(px_eq(_v336, px_str("浮点")))) {
        return px_add(px_str("浮点 "), _v337);
    }
    if (px_is_truthy(px_eq(_v336, px_str("字符串")))) {
        return px_add(px_str("字符串 "), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v337}, 1));
    }
    if (px_is_truthy(px_eq(_v336, px_str("标识符")))) {
        return px_add(px_str("标识符 "), _v337);
    }
    if (px_is_truthy(px_eq(_v336, px_str("注释")))) {
        return px_add(px_str("注释 "), _v337);
    }
    return _v336;
px_err_335:
    if (px_err_335_proped) return px_err_335_val;
    return px_null();
}

static LXValue fn_pv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_338_val = px_null();
    int px_err_338_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
px_err_338:
    if (px_err_338_proped) return px_err_338_val;
    return px_null();
}

static LXValue fn_pline(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_339_val = px_null();
    int px_err_339_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL));
px_err_339:
    if (px_err_339_proped) return px_err_339_val;
    return px_null();
}

static LXValue fn_pcol(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_340_val = px_null();
    int px_err_340_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL));
px_err_340:
    if (px_err_340_proped) return px_err_340_val;
    return px_null();
}

static LXValue fn_ppos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_341_val = px_null();
    int px_err_341_proped = 0;
    return px_list_n((LXValue[]){px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL)), px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL))}, 2);
px_err_341:
    if (px_err_341_proped) return px_err_341_val;
    return px_null();
}

static LXValue fn_adv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_342_val = px_null();
    int px_err_342_proped = 0;
    LXValue _v343 = px_index(px_get_global("p_toks"), px_get_global("p_pos"));
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_set_global("p_pos", px_add(px_get_global("p_pos"), px_int(1LL)));
    }
    return _v343;
px_err_342:
    if (px_err_342_proped) return px_err_342_val;
    return px_null();
}

static LXValue fn_chk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v344 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_345_val = px_null();
    int px_err_345_proped = 0;
    return px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), _v344);
px_err_345:
    if (px_err_345_proped) return px_err_345_val;
    return px_null();
}

static LXValue fn_chk2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v346 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_347_val = px_null();
    int px_err_347_proped = 0;
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(1LL))), px_int(0LL)), _v346);
    }
    return px_bool(false);
px_err_347:
    if (px_err_347_proped) return px_err_347_val;
    return px_null();
}

static LXValue fn_expect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v348 = (nargs > 0) ? args[0] : px_null();
    LXValue _v349 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_350_val = px_null();
    int px_err_350_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v348}, 1))) {
        return px_call(px_get_global("adv"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望 "), _v349), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_350:
    if (px_err_350_proped) return px_err_350_val;
    return px_null();
}

static LXValue fn_expect_ident(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v351 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_352_val = px_null();
    int px_err_352_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        LXValue _v353 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v353;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v351), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_352:
    if (px_err_352_proped) return px_err_352_val;
    return px_null();
}

static LXValue fn_is_name_kind(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v354 = (nargs > 0) ? args[0] : px_null();
    LXValue _v355 = px_null();
    LXValue px_err_356_val = px_null();
    int px_err_356_proped = 0;
    LXValue _v357 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture")}, 35);
    LXValue _v358 = px_int(0LL);
    while (px_is_truthy(px_lt(_v358, px_call(px_get_global("len"), (LXValue[]){_v357}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v357, _v358), _v354))) {
            return px_bool(true);
        }
         _v358 = px_add(_v358, px_int(1LL));
    }
    return px_bool(false);
px_err_356:
    if (px_err_356_proped) return px_err_356_val;
    return px_null();
}

static LXValue fn_expect_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v359 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_360_val = px_null();
    int px_err_360_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        LXValue _v361 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v361;
    }
    if (px_is_truthy(px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1))) {
        LXValue _v362 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v362;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v359), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_360:
    if (px_err_360_proped) return px_err_360_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v363 = (nargs > 0) ? args[0] : px_null();
    LXValue _v364 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_365_val = px_null();
    int px_err_365_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1)), px_str(": 语法错误 ")), _v363), px_str(": ")), _v364)}, 1));
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("parse "), _v363)}, 1));
px_err_365:
    if (px_err_365_proped) return px_err_365_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_366_val = px_null();
    int px_err_366_proped = 0;
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_366:
    if (px_err_366_proped) return px_err_366_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_367_val = px_null();
    int px_err_367_proped = 0;
    while (px_is_truthy(({ LXValue _t368 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t368) ? _t368 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_367:
    if (px_err_367_proped) return px_err_367_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_369_val = px_null();
    int px_err_369_proped = 0;
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_369:
    if (px_err_369_proped) return px_err_369_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v370 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_371_val = px_null();
    int px_err_371_proped = 0;
    return px_index(_v370, px_sub(px_call(px_get_global("len"), (LXValue[]){_v370}, 1), px_int(1LL)));
px_err_371:
    if (px_err_371_proped) return px_err_371_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v372 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_373_val = px_null();
    int px_err_373_proped = 0;
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v372}, 1);
px_err_373:
    if (px_err_373_proped) return px_err_373_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_374_val = px_null();
    int px_err_374_proped = 0;
    LXValue _v375 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        (void)(px_method(_v375, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v375}, 2);
px_err_374:
    if (px_err_374_proped) return px_err_374_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_376_val = px_null();
    int px_err_376_proped = 0;
    LXValue _v377 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v377, px_str("let")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    if (px_is_truthy(px_eq(_v377, px_str("var")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    if (px_is_truthy(px_eq(_v377, px_str("const")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    if (px_is_truthy(px_eq(_v377, px_str("if")))) {
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("for")))) {
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("while")))) {
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("def")))) {
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("struct")))) {
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("enum")))) {
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("trait")))) {
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("impl")))) {
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("import")))) {
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("from")))) {
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("return")))) {
        LXValue _v378 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t380 = ({ LXValue _t379 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t379) ? _t379 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t380) ? _t380 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v378}, 3);
        }
        LXValue _v381 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Return"), _v381, _v378}, 3);
    }
    if (px_is_truthy(px_eq(_v377, px_str("break")))) {
        LXValue _v382 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Break"), _v382}, 2);
    }
    if (px_is_truthy(px_eq(_v377, px_str("continue")))) {
        LXValue _v383 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Continue"), _v383}, 2);
    }
    if (px_is_truthy(px_eq(_v377, px_str("spawn")))) {
        LXValue _v384 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v385 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Spawn"), _v385, _v384}, 3);
    }
    if (px_is_truthy(px_eq(_v377, px_str("select")))) {
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v377, px_str("fn")))) {
        LXValue _v386 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        LXValue _v387 = px_call(px_get_global("node_pos"), (LXValue[]){_v386}, 1);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v386, _v387}, 3);
    }
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_376:
    if (px_err_376_proped) return px_err_376_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v388 = (nargs > 0) ? args[0] : px_null();
    LXValue _v389 = px_null();
    LXValue _v390 = px_null();
    LXValue px_err_391_val = px_null();
    int px_err_391_proped = 0;
    LXValue _v392 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    LXValue _v393 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    LXValue _v394 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v394 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    LXValue _v395 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v395 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    return px_list_n((LXValue[]){px_str("VarDecl"), _v388, px_call(px_get_global("qstr"), (LXValue[]){_v393}, 1), _v394, _v395, _v392}, 6);
px_err_391:
    if (px_err_391_proped) return px_err_391_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v396 = px_null();
    LXValue px_err_397_val = px_null();
    int px_err_397_proped = 0;
    LXValue _v398 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    LXValue _v399 = px_call(px_get_global("node_pos"), (LXValue[]){_v398}, 1);
    LXValue _v400 = px_null();
    LXValue _v401 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v401, px_str("=")))) {
         _v400 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("+=")))) {
         _v400 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("-=")))) {
         _v400 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("*=")))) {
         _v400 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("/=")))) {
         _v400 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("//=")))) {
         _v400 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("%=")))) {
         _v400 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("**=")))) {
         _v400 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("&=")))) {
         _v400 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("|=")))) {
         _v400 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("^=")))) {
         _v400 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v401, px_str("<<=")))) {
         _v400 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v401, px_str(">>=")))) {
         _v400 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v401, px_str(">>>=")))) {
         _v400 = px_str("ShrU");
    }
    if (px_is_truthy(px_ne(_v400, px_null()))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v402 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Assign"), _v398, _v400, _v402, _v399}, 5);
    }
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v398, _v399}, 3);
px_err_397:
    if (px_err_397_proped) return px_err_397_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v403 = px_null();
    LXValue px_err_404_val = px_null();
    int px_err_404_proped = 0;
    LXValue _v405 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v406 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v407 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    LXValue _v408 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v406, _v407}, 2)}, 1);
    LXValue _v409 = px_null();
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v410 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            LXValue _v411 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            (void)(px_method(_v408, "append", (LXValue[]){px_list_n((LXValue[]){_v410, _v411}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
             _v409 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            break;
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){px_str("If"), _v408, _v409, _v405}, 4);
px_err_404:
    if (px_err_404_proped) return px_err_404_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_412_val = px_null();
    int px_err_412_proped = 0;
    LXValue _v413 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v414 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    LXValue _v415 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v416 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("For"), px_call(px_get_global("qstr"), (LXValue[]){_v414}, 1), _v415, _v416, _v413}, 5);
px_err_412:
    if (px_err_412_proped) return px_err_412_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_417_val = px_null();
    int px_err_417_proped = 0;
    LXValue _v418 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v419 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v420 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("While"), _v419, _v420, _v418}, 4);
px_err_417:
    if (px_err_417_proped) return px_err_417_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_421_val = px_null();
    int px_err_421_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v422 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t423 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t423) ? _t423 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_method(_v422, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return _v422;
px_err_421:
    if (px_err_421_proped) return px_err_421_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v424 = px_null();
    LXValue px_err_425_val = px_null();
    int px_err_425_proped = 0;
    LXValue _v426 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v427 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    LXValue _v428 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v429 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v429 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v430 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v427}, 1), _v428, _v429, _v430, _v426}, 6);
px_err_425:
    if (px_err_425_proped) return px_err_425_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_431_val = px_null();
    int px_err_431_proped = 0;
    LXValue _v432 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v433 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v434 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t435 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t435) ? _t435 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v436 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v437 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v438 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_method(_v434, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v437}, 1), _v438, _v436}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t439 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t439) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t439; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v433}, 1), _v434, _v432}, 4);
px_err_431:
    if (px_err_431_proped) return px_err_431_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_440_val = px_null();
    int px_err_440_proped = 0;
    LXValue _v441 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v442 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v443 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t444 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t444) ? _t444 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v445 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v446 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        LXValue _v447 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v447, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        (void)(px_method(_v443, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v446}, 1), _v447, _v445}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t448 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t448) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t448; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v442}, 1), _v443, _v441}, 4);
px_err_440:
    if (px_err_440_proped) return px_err_440_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v449 = px_null();
    LXValue _v450 = px_null();
    LXValue px_err_451_val = px_null();
    int px_err_451_proped = 0;
    LXValue _v452 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v453 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v454 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t455 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t455) ? _t455 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v456 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v457 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        LXValue _v458 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        LXValue _v459 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v459 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v460 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                 _v460 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        (void)(px_method(_v454, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v457}, 1), _v458, _v459, _v460, _v456}, 6)}, 1));
        if (px_is_truthy(({ LXValue _t461 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t461) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t461; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v453}, 1), _v454, _v452}, 4);
px_err_451:
    if (px_err_451_proped) return px_err_451_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v462 = px_null();
    LXValue _v463 = px_null();
    LXValue _v464 = px_null();
    LXValue px_err_465_val = px_null();
    int px_err_465_proped = 0;
    LXValue _v466 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v467 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    LXValue _v468 = px_null();
    LXValue _v469 = _v467;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v469 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
         _v468 = px_call(px_get_global("qstr"), (LXValue[]){_v467}, 1);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v470 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t471 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t471) ? _t471 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v472 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v473 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        LXValue _v474 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        LXValue _v475 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v475 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v476 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        (void)(px_method(_v470, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v473}, 1), _v474, _v475, _v476, _v472}, 6)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v469}, 1), _v468, _v470, _v466}, 5);
px_err_465:
    if (px_err_465_proped) return px_err_465_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_477_val = px_null();
    int px_err_477_proped = 0;
    LXValue _v478 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        LXValue _v479 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v479}, 1), px_list_n((LXValue[]){}, 0), _v478}, 4);
    }
    LXValue _v480 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v480, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v480, px_list_n((LXValue[]){}, 0), _v478}, 4);
px_err_477:
    if (px_err_477_proped) return px_err_477_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_481_val = px_null();
    int px_err_481_proped = 0;
    LXValue _v482 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v483 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v483, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    LXValue _v484 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v484, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v483, _v484, _v482}, 4);
px_err_481:
    if (px_err_481_proped) return px_err_481_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v485 = px_null();
    LXValue _v486 = px_null();
    LXValue px_err_487_val = px_null();
    int px_err_487_proped = 0;
    LXValue _v488 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v489 = px_list_n((LXValue[]){}, 0);
    LXValue _v490 = px_null();
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t491 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t491) ? _t491 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        if (px_is_truthy(({ LXValue _t492 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t492) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t492; }))) {
            LXValue _v493 = px_get_global("p_pos");
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                 _v490 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                continue;
            }
            else {
                px_set_global("p_pos", _v493);
            }
        }
        LXValue _v494 = px_null();
        if (px_is_truthy(({ LXValue _t495 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t495) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t495; }))) {
             _v494 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        LXValue _v496 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v497 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        (void)(px_method(_v489, "append", (LXValue[]){px_list_n((LXValue[]){_v494, _v496, _v497}, 3)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Select"), _v489, _v490, _v488}, 4);
px_err_487:
    if (px_err_487_proped) return px_err_487_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_498_val = px_null();
    int px_err_498_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    LXValue _v499 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    LXValue _v500 = px_call(px_get_global("node_pos"), (LXValue[]){_v499}, 1);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v499, _v500}, 3)}, 1);
px_err_498:
    if (px_err_498_proped) return px_err_498_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v501 = px_null();
    LXValue _v502 = px_null();
    LXValue px_err_503_val = px_null();
    int px_err_503_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    LXValue _v504 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            LXValue _v505 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            LXValue _v506 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            LXValue _v507 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v507 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            LXValue _v508 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v508 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            (void)(px_method(_v504, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v506}, 1), _v507, _v508, _v505}, 5)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v504;
px_err_503:
    if (px_err_503_proped) return px_err_503_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_509_val = px_null();
    int px_err_509_proped = 0;
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_509:
    if (px_err_509_proped) return px_err_509_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v510 = px_null();
    LXValue px_err_511_val = px_null();
    int px_err_511_proped = 0;
    LXValue _v512 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|>")}, 1))) {
        LXValue _v513 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v514 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
         _v512 = px_list_n((LXValue[]){px_str("Pipe"), _v512, _v514, _v513}, 4);
    }
    return _v512;
px_err_511:
    if (px_err_511_proped) return px_err_511_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v515 = px_null();
    LXValue px_err_516_val = px_null();
    int px_err_516_proped = 0;
    LXValue _v517 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("??")}, 1))) {
        LXValue _v518 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v519 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
         _v517 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v517, _v519, _v518}, 4);
    }
    return _v517;
px_err_516:
    if (px_err_516_proped) return px_err_516_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v520 = px_null();
    LXValue px_err_521_val = px_null();
    int px_err_521_proped = 0;
    LXValue _v522 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("or")}, 1))) {
        LXValue _v523 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v524 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
         _v522 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v522, _v524, _v523}, 5);
    }
    return _v522;
px_err_521:
    if (px_err_521_proped) return px_err_521_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v525 = px_null();
    LXValue px_err_526_val = px_null();
    int px_err_526_proped = 0;
    LXValue _v527 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("and")}, 1))) {
        LXValue _v528 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v529 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
         _v527 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v527, _v529, _v528}, 5);
    }
    return _v527;
px_err_526:
    if (px_err_526_proped) return px_err_526_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v530 = px_null();
    LXValue _v531 = px_null();
    LXValue px_err_532_val = px_null();
    int px_err_532_proped = 0;
    LXValue _v533 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v534 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("==")}, 1))) {
             _v534 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("!=")}, 1))) {
             _v534 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<")}, 1))) {
             _v534 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<=")}, 1))) {
             _v534 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">")}, 1))) {
             _v534 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">=")}, 1))) {
             _v534 = px_str("Ge");
        }
        if (px_is_truthy(px_eq(_v534, px_null()))) {
            break;
        }
        LXValue _v535 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v536 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
         _v533 = px_list_n((LXValue[]){px_str("Binary"), _v534, _v533, _v536, _v535}, 5);
    }
    return _v533;
px_err_532:
    if (px_err_532_proped) return px_err_532_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v537 = px_null();
    LXValue px_err_538_val = px_null();
    int px_err_538_proped = 0;
    LXValue _v539 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|")}, 1))) {
        LXValue _v540 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v541 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
         _v539 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v539, _v541, _v540}, 5);
    }
    return _v539;
px_err_538:
    if (px_err_538_proped) return px_err_538_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v542 = px_null();
    LXValue px_err_543_val = px_null();
    int px_err_543_proped = 0;
    LXValue _v544 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("^")}, 1))) {
        LXValue _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v546 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
         _v544 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v544, _v546, _v545}, 5);
    }
    return _v544;
px_err_543:
    if (px_err_543_proped) return px_err_543_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v547 = px_null();
    LXValue px_err_548_val = px_null();
    int px_err_548_proped = 0;
    LXValue _v549 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("&")}, 1))) {
        LXValue _v550 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v551 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
         _v549 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v549, _v551, _v550}, 5);
    }
    return _v549;
px_err_548:
    if (px_err_548_proped) return px_err_548_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v552 = px_null();
    LXValue _v553 = px_null();
    LXValue px_err_554_val = px_null();
    int px_err_554_proped = 0;
    LXValue _v555 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v556 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<<")}, 1))) {
             _v556 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>")}, 1))) {
             _v556 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>>")}, 1))) {
             _v556 = px_str("ShrU");
        }
        if (px_is_truthy(px_eq(_v556, px_null()))) {
            break;
        }
        LXValue _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v558 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
         _v555 = px_list_n((LXValue[]){px_str("Binary"), _v556, _v555, _v558, _v557}, 5);
    }
    return _v555;
px_err_554:
    if (px_err_554_proped) return px_err_554_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v559 = px_null();
    LXValue _v560 = px_null();
    LXValue px_err_561_val = px_null();
    int px_err_561_proped = 0;
    LXValue _v562 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v563 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("+")}, 1))) {
             _v563 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("-")}, 1))) {
             _v563 = px_str("Sub");
        }
        if (px_is_truthy(px_eq(_v563, px_null()))) {
            break;
        }
        LXValue _v564 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v565 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
         _v562 = px_list_n((LXValue[]){px_str("Binary"), _v563, _v562, _v565, _v564}, 5);
    }
    return _v562;
px_err_561:
    if (px_err_561_proped) return px_err_561_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v566 = px_null();
    LXValue _v567 = px_null();
    LXValue px_err_568_val = px_null();
    int px_err_568_proped = 0;
    LXValue _v569 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v570 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("*")}, 1))) {
             _v570 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("/")}, 1))) {
             _v570 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("//")}, 1))) {
             _v570 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("%")}, 1))) {
             _v570 = px_str("Mod");
        }
        if (px_is_truthy(px_eq(_v570, px_null()))) {
            break;
        }
        LXValue _v571 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v572 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
         _v569 = px_list_n((LXValue[]){px_str("Binary"), _v570, _v569, _v572, _v571}, 5);
    }
    return _v569;
px_err_568:
    if (px_err_568_proped) return px_err_568_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_573_val = px_null();
    int px_err_573_proped = 0;
    LXValue _v574 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("**")}, 1))) {
        LXValue _v575 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v576 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v574, _v576, _v575}, 5);
    }
    return _v574;
px_err_573:
    if (px_err_573_proped) return px_err_573_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_577_val = px_null();
    int px_err_577_proped = 0;
    LXValue _v578 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v578, px_str("-")))) {
        LXValue _v579 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v580 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v580, _v579}, 4);
    }
    if (px_is_truthy(px_eq(_v578, px_str("not")))) {
        LXValue _v581 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v582 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v582, _v581}, 4);
    }
    if (px_is_truthy(px_eq(_v578, px_str("~")))) {
        LXValue _v583 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v584 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v584, _v583}, 4);
    }
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_577:
    if (px_err_577_proped) return px_err_577_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v585 = px_null();
    LXValue _v586 = px_null();
    LXValue px_err_587_val = px_null();
    int px_err_587_proped = 0;
    LXValue _v588 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v589 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v589, px_str("(")))) {
            LXValue _v590 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            LXValue _v591 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v588 = px_list_n((LXValue[]){px_str("Call"), _v588, _v591, _v590}, 4);
        }
        else if (px_is_truthy(px_eq(_v589, px_str("[")))) {
            LXValue _v592 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                LXValue _v593 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                LXValue _v594 = px_null();
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                     _v594 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                }
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                 _v588 = px_list_n((LXValue[]){px_str("Slice"), _v588, px_null(), _v593, _v594, _v592}, 6);
            }
            else {
                LXValue _v595 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    LXValue _v596 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    LXValue _v597 = px_null();
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                         _v597 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    }
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v588 = px_list_n((LXValue[]){px_str("Slice"), _v588, _v595, _v596, _v597, _v592}, 6);
                }
                else {
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v588 = px_list_n((LXValue[]){px_str("Index"), _v588, _v595, _v592}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v589, px_str(".")))) {
            LXValue _v598 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v599 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v588 = px_list_n((LXValue[]){px_str("Field"), _v588, px_call(px_get_global("qstr"), (LXValue[]){_v599}, 1), _v598}, 4);
        }
        else if (px_is_truthy(px_eq(_v589, px_str("?.")))) {
            LXValue _v600 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v601 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v588 = px_list_n((LXValue[]){px_str("OptionalField"), _v588, px_call(px_get_global("qstr"), (LXValue[]){_v601}, 1), _v600}, 4);
        }
        else if (px_is_truthy(px_eq(_v589, px_str("!")))) {
            LXValue _v602 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v588 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v588, _v602}, 3);
        }
        else if (px_is_truthy(px_eq(_v589, px_str("?")))) {
            LXValue _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v588 = px_list_n((LXValue[]){px_str("Try"), _v588, _v603}, 3);
        }
        else {
            break;
        }
    }
    return _v588;
px_err_587:
    if (px_err_587_proped) return px_err_587_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_604_val = px_null();
    int px_err_604_proped = 0;
    if (px_is_truthy(({ LXValue _t605 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t605) ? _t605 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        return px_null();
    }
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_604:
    if (px_err_604_proped) return px_err_604_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_606_val = px_null();
    int px_err_606_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    LXValue _v607 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v607, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v607;
px_err_606:
    if (px_err_606_proped) return px_err_606_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v608 = px_null();
    LXValue px_err_609_val = px_null();
    int px_err_609_proped = 0;
    LXValue _v610 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v610, px_str("整数")))) {
        LXValue _v611 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v612 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Int"), _v612, _v611}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("浮点")))) {
        LXValue _v613 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v614 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Float"), _v614, _v613}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("字符串")))) {
        LXValue _v615 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v616 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Str"), _v616, _v615}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("true")))) {
        LXValue _v617 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v617}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("false")))) {
        LXValue _v618 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v618}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("null")))) {
        LXValue _v619 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Null"), _v619}, 2);
    }
    if (px_is_truthy(px_eq(_v610, px_str("self")))) {
        LXValue _v620 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v620}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("标识符")))) {
        LXValue _v621 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v622 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v622}, 1), _v621}, 3);
    }
    if (px_is_truthy(px_eq(_v610, px_str("[")))) {
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v610, px_str("(")))) {
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v610, px_str("{")))) {
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v610, px_str("fn")))) {
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v610, px_str("match")))) {
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v610, px_str("if")))) {
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v610, px_str("chan")))) {
        LXValue _v623 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v624 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v623}, 3);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            LXValue _v625 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v624 = px_list_n((LXValue[]){px_str("Call"), _v624, _v625, _v623}, 4);
        }
        return _v624;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_609:
    if (px_err_609_proped) return px_err_609_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_626_val = px_null();
    int px_err_626_proped = 0;
    LXValue _v627 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v627}, 3);
    }
    LXValue _v628 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        LXValue _v629 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("ListComp"), _v628, px_index(_v629, px_int(0LL)), px_index(_v629, px_int(1LL)), _v627}, 5);
    }
    LXValue _v630 = px_list_n((LXValue[]){_v628}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            break;
        }
        (void)(px_method(_v630, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    return px_list_n((LXValue[]){px_str("List"), _v630, _v627}, 3);
px_err_626:
    if (px_err_626_proped) return px_err_626_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_631_val = px_null();
    int px_err_631_proped = 0;
    LXValue _v632 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        (void)(px_method(_v632, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    return _v632;
px_err_631:
    if (px_err_631_proped) return px_err_631_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_633_val = px_null();
    int px_err_633_proped = 0;
    LXValue _v634 = px_list_n((LXValue[]){}, 0);
    LXValue _v635 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v636 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            LXValue _v637 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v634, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v636, _v637}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_method(_v635, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){_v634, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v635}, 1)}, 2);
px_err_633:
    if (px_err_633_proped) return px_err_633_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v638 = (nargs > 0) ? args[0] : px_null();
    LXValue _v639 = px_null();
    LXValue _v640 = px_null();
    LXValue px_err_641_val = px_null();
    int px_err_641_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v638}, 1), px_int(0LL)))) {
        return px_null();
    }
    LXValue _v642 = px_index(_v638, px_int(0LL));
    LXValue _v643 = px_int(1LL);
    while (px_is_truthy(px_lt(_v643, px_call(px_get_global("len"), (LXValue[]){_v638}, 1)))) {
        LXValue _v644 = px_call(px_get_global("node_pos"), (LXValue[]){_v642}, 1);
         _v642 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v642, px_index(_v638, _v643), _v644}, 5);
         _v643 = px_add(_v643, px_int(1LL));
    }
    return _v642;
px_err_641:
    if (px_err_641_proped) return px_err_641_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_645_val = px_null();
    int px_err_645_proped = 0;
    LXValue _v646 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v646}, 3);
    }
    LXValue _v647 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        LXValue _v648 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("GenExp"), _v647, px_index(_v648, px_int(0LL)), px_index(_v648, px_int(1LL)), _v646}, 5);
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        LXValue _v649 = px_list_n((LXValue[]){_v647}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                break;
            }
            (void)(px_method(_v649, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("Tuple"), _v649, _v646}, 3);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v647;
px_err_645:
    if (px_err_645_proped) return px_err_645_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v650 = px_null();
    LXValue _v651 = px_null();
    LXValue px_err_652_val = px_null();
    int px_err_652_proped = 0;
    LXValue _v653 = px_int(0LL);
    LXValue _v654 = px_get_global("p_pos");
    while (px_is_truthy(px_lt(_v654, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        LXValue _v655 = px_index(px_index(px_get_global("p_toks"), _v654), px_int(0LL));
        if (px_is_truthy(({ LXValue _t656 = px_eq(_v655, px_str(":")); px_is_truthy(_t656) ? px_eq(_v653, px_int(0LL)) : _t656; }))) {
            return px_bool(true);
        }
        if (px_is_truthy(({ LXValue _t658 = ({ LXValue _t657 = px_eq(_v655, px_str("(")); px_is_truthy(_t657) ? _t657 : px_eq(_v655, px_str("[")); }); px_is_truthy(_t658) ? _t658 : px_eq(_v655, px_str("{")); }))) {
             _v653 = px_add(_v653, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t659 = px_eq(_v655, px_str(")")); px_is_truthy(_t659) ? _t659 : px_eq(_v655, px_str("]")); }))) {
            if (px_is_truthy(px_gt(_v653, px_int(0LL)))) {
                 _v653 = px_sub(_v653, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t660 = px_eq(_v655, px_str("}")); px_is_truthy(_t660) ? px_eq(_v653, px_int(0LL)) : _t660; }))) {
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t663 = ({ LXValue _t662 = ({ LXValue _t661 = px_eq(_v655, px_str(",")); px_is_truthy(_t661) ? _t661 : px_eq(_v655, px_str("换行")); }); px_is_truthy(_t662) ? _t662 : px_eq(_v655, px_str("EOF")); }); px_is_truthy(_t663) ? px_eq(_v653, px_int(0LL)) : _t663; }))) {
            return px_bool(false);
        }
         _v654 = px_add(_v654, px_int(1LL));
    }
    return px_bool(false);
px_err_652:
    if (px_err_652_proped) return px_err_652_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_664_val = px_null();
    int px_err_664_proped = 0;
    LXValue _v665 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    LXValue _v666 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    if (px_is_truthy(_v666)) {
        LXValue _v667 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v668 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            LXValue _v669 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            return px_list_n((LXValue[]){px_str("DictComp"), _v667, _v668, px_index(_v669, px_int(0LL)), px_index(_v669, px_int(1LL)), _v665}, 6);
        }
        LXValue _v670 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v667, _v668}, 2)}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                break;
            }
            LXValue _v671 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            LXValue _v672 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v670, "append", (LXValue[]){px_list_n((LXValue[]){_v671, _v672}, 2)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("Dict"), _v670, _v665}, 3);
    }
    LXValue _v673 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    while (px_is_truthy(({ LXValue _t674 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t674) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t674; }))) {
        (void)(px_method(_v673, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    return px_list_n((LXValue[]){px_str("Block"), _v673, _v665}, 3);
px_err_664:
    if (px_err_664_proped) return px_err_664_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v675 = px_null();
    LXValue _v676 = px_null();
    LXValue px_err_677_val = px_null();
    int px_err_677_proped = 0;
    LXValue _v678 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v679 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v680 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v680 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    LXValue _v681 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v681, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    LXValue _v682 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        LXValue _v683 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v684 = px_list_n((LXValue[]){}, 0);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        while (px_is_truthy(({ LXValue _t685 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t685) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t685; }))) {
            (void)(px_method(_v684, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
         _v682 = px_list_n((LXValue[]){px_str("Block"), _v684, _v683}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v682 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    else {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    return px_list_n((LXValue[]){px_str("Closure"), _v679, _v680, _v682, _v681, _v678}, 6);
px_err_677:
    if (px_err_677_proped) return px_err_677_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v686 = px_null();
    LXValue _v687 = px_null();
    LXValue _v688 = px_null();
    LXValue px_err_689_val = px_null();
    int px_err_689_proped = 0;
    LXValue _v690 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v691 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v692 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t693 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t693) ? _t693 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        LXValue _v694 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        LXValue _v695 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v695 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v696 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            LXValue _v697 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            LXValue _v698 = px_null();
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v697}, 1), px_int(0LL)))) {
                 _v698 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v697, px_int(0LL))}, 1);
            }
            else {
                 _v698 = _v690;
            }
             _v696 = px_list_n((LXValue[]){px_str("Block"), _v697, _v698}, 3);
        }
        else {
             _v696 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_method(_v692, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v694, _v695, _v696, _v690}, 5)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Match"), _v691, _v692, _v690}, 4);
px_err_689:
    if (px_err_689_proped) return px_err_689_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_699_val = px_null();
    int px_err_699_proped = 0;
    LXValue _v700 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v701 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    LXValue _v702 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    LXValue _v703 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v701, _v702, _v703, _v700}, 5);
px_err_699:
    if (px_err_699_proped) return px_err_699_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_704_val = px_null();
    int px_err_704_proped = 0;
    LXValue _v705 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t710 = ({ LXValue _t709 = ({ LXValue _t708 = ({ LXValue _t707 = ({ LXValue _t706 = px_eq(_v705, px_str("整数")); px_is_truthy(_t706) ? _t706 : px_eq(_v705, px_str("浮点")); }); px_is_truthy(_t707) ? _t707 : px_eq(_v705, px_str("字符串")); }); px_is_truthy(_t708) ? _t708 : px_eq(_v705, px_str("true")); }); px_is_truthy(_t709) ? _t709 : px_eq(_v705, px_str("false")); }); px_is_truthy(_t710) ? _t710 : px_eq(_v705, px_str("null")); }))) {
        LXValue _v711 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v711}, 2);
    }
    if (px_is_truthy(px_eq(_v705, px_str("标识符")))) {
        LXValue _v712 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        LXValue _v713 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v714 = px_list_n((LXValue[]){}, 0);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v714, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v712}, 1), _v714}, 3);
        }
        if (px_is_truthy(px_eq(_v712, px_str("_")))) {
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v712}, 1))) {
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v712}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v712}, 1)}, 2);
    }
    if (px_is_truthy(px_eq(_v705, px_str("(")))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v715 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v715, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    continue;
                }
                break;
            }
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("PatTuple"), _v715}, 2);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_704:
    if (px_err_704_proped) return px_err_704_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v716 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_717_val = px_null();
    int px_err_717_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v716}, 1), px_int(0LL)))) {
        return px_bool(false);
    }
    LXValue _v718 = px_index(_v716, px_int(0LL));
    return ({ LXValue _t719 = px_ge(_v718, px_str("A")); px_is_truthy(_t719) ? px_le(_v718, px_str("Z")) : _t719; });
px_err_717:
    if (px_err_717_proped) return px_err_717_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_720_val = px_null();
    int px_err_720_proped = 0;
    LXValue _v721 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        LXValue _v722 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("TyOptional"), _v721, _v722}, 3);
    }
    return _v721;
px_err_720:
    if (px_err_720_proped) return px_err_720_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_723_val = px_null();
    int px_err_723_proped = 0;
    LXValue _v724 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v724, px_str("标识符")))) {
        LXValue _v725 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        LXValue _v726 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v727 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            if (px_is_truthy(px_eq(_v725, px_str("list")))) {
                return px_list_n((LXValue[]){px_str("TyList"), _v727, _v726}, 3);
            }
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v725}, 1), px_list_n((LXValue[]){_v727}, 1), _v726}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v725}, 1), _v726}, 3);
    }
    if (px_is_truthy(px_eq(_v724, px_str("[")))) {
        LXValue _v728 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v729 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("TyList"), _v729, _v728}, 3);
    }
    if (px_is_truthy(px_eq(_v724, px_str("{")))) {
        LXValue _v730 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v731 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v732 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("TyDict"), _v731, _v732, _v730}, 4);
    }
    if (px_is_truthy(px_eq(_v724, px_str("(")))) {
        LXValue _v733 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v734 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v734, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
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
            LXValue _v735 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v734, _v735, _v733}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyTuple"), _v734, _v733}, 3);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_723:
    if (px_err_723_proped) return px_err_723_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v736 = (nargs > 0) ? args[0] : px_null();
    LXValue _v737 = (nargs > 1) ? args[1] : px_null();
    LXValue _v738 = px_null();
    LXValue _v739 = px_null();
    LXValue _v740 = px_null();
    LXValue _v741 = px_null();
    LXValue _v742 = px_null();
    LXValue _v743 = px_null();
    LXValue _v744 = px_null();
    LXValue _v745 = px_null();
    LXValue px_err_746_val = px_null();
    int px_err_746_proped = 0;
    LXValue _v747 = px_call(px_get_global("cg_pad"), (LXValue[]){_v737}, 1);
    LXValue _v748 = px_index(_v736, px_int(0LL));
    if (px_is_truthy(px_eq(_v748, px_str("VarDecl")))) {
        LXValue _v749 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v736, px_int(2LL))}, 1);
        LXValue _v750 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v736, px_int(4LL)), px_null()))) {
             _v750 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(4LL))}, 1);
        }
        if (px_is_truthy(({ LXValue _t751 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v749}, 2); px_is_truthy(_t751) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t751; }))) {
            return px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_set_global(\"")), _v749), px_str("\", ")), _v750), px_str(");\n"));
        }
        LXValue _v752 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v749}, 1);
        if (px_is_truthy(px_ne(px_index(_v736, px_int(4LL)), px_null()))) {
            LXValue _v753 = px_index(_v736, px_int(4LL));
            LXValue _v754 = px_null();
            if (px_is_truthy(px_eq(px_index(_v753, px_int(0LL)), px_str("Constructor")))) {
                 _v754 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v753, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v753, px_int(0LL)), px_str("Call")))) {
                LXValue _v755 = px_index(_v753, px_int(1LL));
                if (px_is_truthy(px_eq(px_index(_v755, px_int(0LL)), px_str("Var")))) {
                     _v754 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v755, px_int(1LL))}, 1);
                }
            }
            if (px_is_truthy(px_ne(_v754, px_null()))) {
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v754}, 1))) {
                    px_index_set(px_get_global("cg_var_types"), _v749, _v754);
                }
            }
        }
        return px_add(px_add(px_add(px_add(px_add(_v747, px_str("LXValue ")), _v752), px_str(" = ")), _v750), px_str(";\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Assign")))) {
        LXValue _v756 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(3LL))}, 1);
        LXValue _v757 = px_index(_v736, px_int(1LL));
        LXValue _v758 = px_index(_v736, px_int(2LL));
        LXValue _v759 = px_index(_v757, px_int(0LL));
        if (px_is_truthy(px_eq(_v759, px_str("Var")))) {
            LXValue _v760 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v757, px_int(1LL))}, 1);
            LXValue _v761 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v760}, 1);
            if (px_is_truthy(px_eq(_v761, px_null()))) {
                if (px_is_truthy(px_eq(_v758, px_str("Assign")))) {
                    return px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_set_global(\"")), _v760), px_str("\", ")), _v756), px_str(");\n"));
                }
                LXValue _v762 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v758, _v760, _v756}, 3);
                return px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_set_global(\"")), _v760), px_str("\", ")), _v762), px_str(");\n"));
            }
            LXValue _v763 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v758, _v761, _v756}, 3);
            return px_add(px_add(px_add(px_add(px_add(_v747, px_str(" ")), _v761), px_str(" = ")), _v763), px_str(";\n"));
        }
        if (px_is_truthy(px_eq(_v759, px_str("Field")))) {
            LXValue _v764 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v757, px_int(1LL))}, 1);
            LXValue _v765 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v757, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_field_set(")), _v764), px_str(", \"")), _v765), px_str("\", ")), _v756), px_str(");\n"));
        }
        if (px_is_truthy(px_eq(_v759, px_str("Index")))) {
            LXValue _v766 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v757, px_int(1LL))}, 1);
            LXValue _v767 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v757, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_index_set(")), _v766), px_str(", ")), _v767), px_str(", ")), _v756), px_str(");\n"));
        }
        return px_str("不支持的赋值目标");
    }
    if (px_is_truthy(px_eq(_v748, px_str("ExprStmt")))) {
        LXValue _v768 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v747, px_str("(void)(")), _v768), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("If")))) {
        LXValue _v769 = px_str("");
        LXValue _v770 = px_index(_v736, px_int(1LL));
        LXValue _v771 = px_int(0LL);
        while (px_is_truthy(px_lt(_v771, px_call(px_get_global("len"), (LXValue[]){_v770}, 1)))) {
            LXValue _v772 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v770, _v771), px_int(0LL))}, 1);
            LXValue _v773 = px_str("if");
            if (px_is_truthy(px_gt(_v771, px_int(0LL)))) {
                 _v773 = px_str("else if");
            }
             _v769 = px_add(_v769, px_add(px_add(px_add(px_add(_v747, _v773), px_str(" (px_is_truthy(")), _v772), px_str(")) {\n")));
            LXValue _v774 = px_index(px_index(_v770, _v771), px_int(1LL));
            LXValue _v775 = px_int(0LL);
            while (px_is_truthy(px_lt(_v775, px_call(px_get_global("len"), (LXValue[]){_v774}, 1)))) {
                 _v769 = px_add(_v769, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v774, _v775), px_add(_v737, px_int(1LL))}, 2));
                 _v775 = px_add(_v775, px_int(1LL));
            }
             _v769 = px_add(_v769, px_add(_v747, px_str("}\n")));
             _v771 = px_add(_v771, px_int(1LL));
        }
        if (px_is_truthy(px_ne(px_index(_v736, px_int(2LL)), px_null()))) {
             _v769 = px_add(_v769, px_add(_v747, px_str("else {\n")));
            LXValue _v776 = px_index(_v736, px_int(2LL));
            LXValue _v777 = px_int(0LL);
            while (px_is_truthy(px_lt(_v777, px_call(px_get_global("len"), (LXValue[]){_v776}, 1)))) {
                 _v769 = px_add(_v769, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v776, _v777), px_add(_v737, px_int(1LL))}, 2));
                 _v777 = px_add(_v777, px_int(1LL));
            }
             _v769 = px_add(_v769, px_add(_v747, px_str("}\n")));
        }
        return _v769;
    }
    if (px_is_truthy(px_eq(_v748, px_str("While")))) {
        LXValue _v778 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1);
        LXValue _v779 = px_add(px_add(px_add(_v747, px_str("while (px_is_truthy(")), _v778), px_str(")) {\n"));
        LXValue _v780 = px_index(_v736, px_int(2LL));
        LXValue _v781 = px_int(0LL);
        while (px_is_truthy(px_lt(_v781, px_call(px_get_global("len"), (LXValue[]){_v780}, 1)))) {
             _v779 = px_add(_v779, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v780, _v781), px_add(_v737, px_int(1LL))}, 2));
             _v781 = px_add(_v781, px_int(1LL));
        }
         _v779 = px_add(_v779, px_add(_v747, px_str("}\n")));
        return _v779;
    }
    if (px_is_truthy(px_eq(_v748, px_str("For")))) {
        LXValue _v782 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(2LL))}, 1);
        LXValue _v783 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v784 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v785 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1)}, 1);
        LXValue _v786 = px_add(px_add(px_add(px_add(px_add(_v747, px_str("LXValue ")), _v783), px_str(" = ")), _v782), px_str(";\n"));
         _v786 = px_add(_v786, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v747, px_str("for (int ")), _v784), px_str(" = 0; ")), _v784), px_str(" < px_len(")), _v783), px_str("); ")), _v784), px_str("++) {\n")));
         _v786 = px_add(_v786, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v747, px_str("    LXValue ")), _v785), px_str(" = px_index(")), _v783), px_str(", px_int(")), _v784), px_str("));\n")));
        LXValue _v787 = px_index(_v736, px_int(3LL));
        LXValue _v788 = px_int(0LL);
        while (px_is_truthy(px_lt(_v788, px_call(px_get_global("len"), (LXValue[]){_v787}, 1)))) {
             _v786 = px_add(_v786, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v787, _v788), px_add(_v737, px_int(1LL))}, 2));
             _v788 = px_add(_v788, px_int(1LL));
        }
         _v786 = px_add(_v786, px_add(_v747, px_str("}\n")));
        return _v786;
    }
    if (px_is_truthy(px_eq(_v748, px_str("Return")))) {
        if (px_is_truthy(px_ne(px_index(_v736, px_int(1LL)), px_null()))) {
            LXValue _v789 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1);
            return px_add(px_add(px_add(_v747, px_str("return ")), _v789), px_str(";\n"));
        }
        return px_add(_v747, px_str("return px_null();\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Break")))) {
        return px_add(_v747, px_str("break;\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Continue")))) {
        return px_add(_v747, px_str("continue;\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Empty")))) {
        return px_str("");
    }
    if (px_is_truthy(px_eq(_v748, px_str("ChanDecl")))) {
        LXValue _v790 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1)}, 1);
        return px_add(px_add(px_add(_v747, px_str("LXValue ")), _v790), px_str(" = px_chan_create(0);\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Send")))) {
        LXValue _v791 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1);
        LXValue _v792 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_chan_send(")), _v791), px_str(", ")), _v792), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Recv")))) {
        LXValue _v793 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v736, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v747, px_str("px_chan_recv(")), _v793), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v748, px_str("Spawn")))) {
        LXValue _v794 = px_index(_v736, px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v794, px_int(0LL)), px_str("Call")))) {
            LXValue _v795 = px_index(_v794, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v795, px_int(0LL)), px_str("Var")))) {
                LXValue _v796 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v795, px_int(1LL))}, 1);
                LXValue _v797 = px_list_n((LXValue[]){}, 0);
                LXValue _v798 = px_index(_v794, px_int(2LL));
                LXValue _v799 = px_int(0LL);
                while (px_is_truthy(px_lt(_v799, px_call(px_get_global("len"), (LXValue[]){_v798}, 1)))) {
                    (void)(px_method(_v797, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v798, _v799)}, 1)}, 1));
                     _v799 = px_add(_v799, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v747, px_str("px_spawn_name(\"")), _v796), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v797}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v797}, 1)}, 1)), px_str(");\n"));
            }
            return px_str("编译模式 spawn 仅支持直接函数调用（方法调用请用 `lx run`）");
        }
        return px_str("编译模式 spawn 仅支持函数调用表达式");
    }
    if (px_is_truthy(px_eq(_v748, px_str("Select")))) {
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v736, px_int(1LL)), px_index(_v736, px_int(2LL)), _v737}, 3);
    }
    if (px_is_truthy(px_eq(_v748, px_str("Import")))) {
        return px_add(_v747, px_str("/* import 忽略（MVP） */\n"));
    }
    return px_str("");
px_err_746:
    if (px_err_746_proped) return px_err_746_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v800 = (nargs > 0) ? args[0] : px_null();
    LXValue _v801 = (nargs > 1) ? args[1] : px_null();
    LXValue _v802 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_803_val = px_null();
    int px_err_803_proped = 0;
    if (px_is_truthy(px_eq(_v800, px_str("Assign")))) {
        return _v802;
    }
    if (px_is_truthy(px_eq(_v800, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v800, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v801), px_str("\"), ")), _v802), px_str(")"));
    }
    return _v802;
px_err_803:
    if (px_err_803_proped) return px_err_803_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v804 = (nargs > 0) ? args[0] : px_null();
    LXValue _v805 = (nargs > 1) ? args[1] : px_null();
    LXValue _v806 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_807_val = px_null();
    int px_err_807_proped = 0;
    if (px_is_truthy(px_eq(_v804, px_str("Assign")))) {
        return _v806;
    }
    if (px_is_truthy(px_eq(_v804, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v804, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v805), px_str(", ")), _v806), px_str(")"));
    }
    return _v806;
px_err_807:
    if (px_err_807_proped) return px_err_807_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v808 = (nargs > 0) ? args[0] : px_null();
    LXValue _v809 = (nargs > 1) ? args[1] : px_null();
    LXValue _v810 = (nargs > 2) ? args[2] : px_null();
    LXValue _v811 = px_null();
    LXValue _v812 = px_null();
    LXValue _v813 = px_null();
    LXValue _v814 = px_null();
    LXValue _v815 = px_null();
    LXValue _v816 = px_null();
    LXValue _v817 = px_null();
    LXValue _v818 = px_null();
    LXValue px_err_819_val = px_null();
    int px_err_819_proped = 0;
    LXValue _v820 = px_call(px_get_global("cg_pad"), (LXValue[]){_v810}, 1);
    LXValue _v821 = px_call(px_get_global("len"), (LXValue[]){_v808}, 1);
    if (px_is_truthy(px_eq(_v821, px_int(0LL)))) {
        return px_str("select 至少需要一个 case 分支");
    }
    LXValue _v822 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    LXValue _v823 = px_str("");
    LXValue _v824 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v825 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    LXValue _v826 = px_list_n((LXValue[]){}, 0);
    LXValue _v827 = px_int(0LL);
    while (px_is_truthy(px_lt(_v827, _v821))) {
        LXValue _v828 = px_index(px_index(_v808, _v827), px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v828, px_int(0LL)), px_str("Call")))) {
            LXValue _v829 = px_index(_v828, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v829, px_int(0LL)), px_str("Field")))) {
                LXValue _v830 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v829, px_int(2LL))}, 1);
                if (px_is_truthy(px_eq(_v830, px_str("recv")))) {
                    (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v829, px_int(1LL))}, 1)}, 1));
                     _v827 = px_add(_v827, px_int(1LL));
                    continue;
                }
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v830), px_str("）"));
            }
            return px_str("select case 仅支持 ch.recv()");
        }
        return px_str("select case 仅支持 ch.recv()");
    }
     _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v820, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v821}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("};\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(": {\n")));
    LXValue _v831 = px_list_n((LXValue[]){}, 0);
    LXValue _v832 = px_int(0LL);
    while (px_is_truthy(px_lt(_v832, _v821))) {
        (void)(px_method(_v831, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v832}, 1)}, 1));
         _v832 = px_add(_v832, px_int(1LL));
    }
     _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v820, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v821}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v831}, 2)), px_str("};\n")));
    if (px_is_truthy(px_gt(_v821, px_int(1LL)))) {
        LXValue _v833 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v820, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v821}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("--) { "));
         _v833 = px_add(_v833, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" + 1); ")));
         _v833 = px_add(_v833, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("; ")));
         _v833 = px_add(_v833, px_str("}\n"));
         _v823 = px_add(_v823, _v833);
    }
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = px_null();\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = -1;\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v820, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v821}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("++) {\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v820, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("];\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v820, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str("; break; }\n")));
     _v823 = px_add(_v823, px_add(_v820, px_str("    }\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" >= 0) {\n")));
    LXValue _v834 = px_int(0LL);
    while (px_is_truthy(px_lt(_v834, _v821))) {
        LXValue _v835 = px_index(px_index(_v808, _v834), px_int(0LL));
        LXValue _v836 = px_index(px_index(_v808, _v834), px_int(2LL));
        LXValue _v837 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v834}, 1)), px_str(")"));
        if (px_is_truthy(px_gt(_v834, px_int(0LL)))) {
             _v837 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v834}, 1)), px_str(")"));
        }
         _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("        ")), _v837), px_str(" {\n")));
        if (px_is_truthy(px_ne(_v835, px_null()))) {
            LXValue _v838 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v835}, 1)}, 1);
             _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(_v820, px_str("            LXValue ")), _v838), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(";\n")));
        }
        LXValue _v839 = px_int(0LL);
        while (px_is_truthy(px_lt(_v839, px_call(px_get_global("len"), (LXValue[]){_v836}, 1)))) {
             _v823 = px_add(_v823, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v836, _v839), px_add(_v810, px_int(3LL))}, 2));
             _v839 = px_add(_v839, px_int(1LL));
        }
         _v823 = px_add(_v823, px_add(_v820, px_str("        }\n")));
         _v834 = px_add(_v834, px_int(1LL));
    }
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(";\n")));
     _v823 = px_add(_v823, px_add(_v820, px_str("    }\n")));
    if (px_is_truthy(px_ne(_v809, px_null()))) {
         _v823 = px_add(_v823, px_add(_v820, px_str("    {\n")));
        LXValue _v840 = px_int(0LL);
        while (px_is_truthy(px_lt(_v840, px_call(px_get_global("len"), (LXValue[]){_v809}, 1)))) {
             _v823 = px_add(_v823, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v809, _v840), px_add(_v810, px_int(2LL))}, 2));
             _v840 = px_add(_v840, px_int(1LL));
        }
         _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(";\n")));
         _v823 = px_add(_v823, px_add(_v820, px_str("    }\n")));
    }
     _v823 = px_add(_v823, px_add(_v820, px_str("    px_select_wait();\n")));
     _v823 = px_add(_v823, px_add(_v820, px_str("}\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(";\n")));
     _v823 = px_add(_v823, px_add(px_add(px_add(_v820, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v822}, 1)), px_str(": ;\n")));
    px_set_global("cg_vars", _v824);
    px_set_global("cg_var_types", _v825);
    return _v823;
px_err_819:
    if (px_err_819_proped) return px_err_819_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v841 = (nargs > 0) ? args[0] : px_null();
    LXValue _v842 = px_null();
    LXValue _v843 = px_null();
    LXValue _v844 = px_null();
    LXValue _v845 = px_null();
    LXValue px_err_846_val = px_null();
    int px_err_846_proped = 0;
    LXValue _v847 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v847, "remove", (LXValue[]){px_str("_")}, 1));
    px_index_set(_v847, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v847, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v847, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v847, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v847, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v847, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    LXValue _v848 = px_int(0LL);
    while (px_is_truthy(px_lt(_v848, px_call(px_get_global("len"), (LXValue[]){_v841}, 1)))) {
        LXValue _v849 = px_index(_v841, _v848);
        LXValue _v850 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v849, px_int(2LL))}, 1);
        (void)(px_method(px_index(_v847, px_str("its")), "append", (LXValue[]){_v850}, 1));
        (void)(px_method(px_index(_v847, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v847, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v847, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        LXValue _v851 = px_str("");
        LXValue _v852 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v849, px_int(1LL))}, 1), px_int(1LL)))) {
            LXValue _v853 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            LXValue _v854 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v849, px_int(1LL)), px_int(0LL))}, 1);
            LXValue _v855 = px_null();
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v854}, 1))) {
                 _v855 = px_index(px_get_global("cg_vars"), _v854);
            }
            px_index_set(px_get_global("cg_vars"), _v854, _v853);
            (void)(px_method(_v852, "append", (LXValue[]){px_list_n((LXValue[]){_v854, _v855}, 2)}, 1));
             _v851 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v853), px_str(" = ")), px_index(px_index(_v847, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v847, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            LXValue _v856 = px_int(0LL);
            while (px_is_truthy(px_lt(_v856, px_call(px_get_global("len"), (LXValue[]){px_index(_v849, px_int(1LL))}, 1)))) {
                LXValue _v857 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v849, px_int(1LL)), _v856)}, 1);
                LXValue _v858 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v856}, 1));
                LXValue _v859 = px_null();
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v857}, 1))) {
                     _v859 = px_index(px_get_global("cg_vars"), _v857);
                }
                px_index_set(px_get_global("cg_vars"), _v857, _v858);
                (void)(px_method(_v852, "append", (LXValue[]){px_list_n((LXValue[]){_v857, _v859}, 2)}, 1));
                 _v851 = px_add(_v851, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v858), px_str(" = px_index(")), px_index(px_index(_v847, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v847, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v856}, 1)), px_str(")); ")));
                 _v856 = px_add(_v856, px_int(1LL));
            }
        }
        (void)(px_method(px_index(_v847, px_str("binds")), "append", (LXValue[]){_v851}, 1));
        (void)(px_method(px_index(_v847, px_str("saved_all")), "append", (LXValue[]){_v852}, 1));
         _v848 = px_add(_v848, px_int(1LL));
    }
    return _v847;
px_err_846:
    if (px_err_846_proped) return px_err_846_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v860 = (nargs > 0) ? args[0] : px_null();
    LXValue _v861 = px_null();
    LXValue _v862 = px_null();
    LXValue px_err_863_val = px_null();
    int px_err_863_proped = 0;
    LXValue _v864 = px_int(0LL);
    while (px_is_truthy(px_lt(_v864, px_call(px_get_global("len"), (LXValue[]){_v860}, 1)))) {
        LXValue _v865 = px_index(_v860, _v864);
        LXValue _v866 = px_int(0LL);
        while (px_is_truthy(px_lt(_v866, px_call(px_get_global("len"), (LXValue[]){_v865}, 1)))) {
            LXValue _v867 = px_index(px_index(_v865, _v866), px_int(0LL));
            LXValue _v868 = px_index(px_index(_v865, _v866), px_int(1LL));
            if (px_is_truthy(px_eq(_v868, px_null()))) {
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v867}, 1));
            }
            else {
                px_index_set(px_get_global("cg_vars"), _v867, _v868);
            }
             _v866 = px_add(_v866, px_int(1LL));
        }
         _v864 = px_add(_v864, px_int(1LL));
    }
px_err_863:
    if (px_err_863_proped) return px_err_863_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v869 = (nargs > 0) ? args[0] : px_null();
    LXValue _v870 = (nargs > 1) ? args[1] : px_null();
    LXValue _v871 = (nargs > 2) ? args[2] : px_null();
    LXValue _v872 = px_null();
    LXValue _v873 = px_null();
    LXValue _v874 = px_null();
    LXValue px_err_875_val = px_null();
    int px_err_875_proped = 0;
    LXValue _v876 = px_str("");
    if (px_is_truthy(px_ne(_v870, px_null()))) {
         _v876 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v870), px_str(")) { ")), _v871), px_str("} "));
    }
    else {
         _v876 = _v871;
    }
    LXValue _v877 = px_call(px_get_global("len"), (LXValue[]){px_index(_v869, px_str("its"))}, 1);
    LXValue _v878 = px_sub(_v877, px_int(1LL));
    while (px_is_truthy(px_ge(_v878, px_int(0LL)))) {
        LXValue _v879 = px_str("");
        if (px_is_truthy(px_lt(px_add(_v878, px_int(1LL)), _v877))) {
             _v879 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v869, px_str("ivs")), px_add(_v878, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v869, px_str("its")), px_add(_v878, px_int(1LL)))), px_str("; "));
        }
         _v876 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v869, px_str("idxs")), _v878)), px_str("=0; ")), px_index(px_index(_v869, px_str("idxs")), _v878)), px_str("<px_len(")), px_index(px_index(_v869, px_str("ivs")), _v878)), px_str("); ")), px_index(px_index(_v869, px_str("idxs")), _v878)), px_str("++) { LXValue ")), px_index(px_index(_v869, px_str("itms")), _v878)), px_str(" = px_index(")), px_index(px_index(_v869, px_str("ivs")), _v878)), px_str(", px_int(")), px_index(px_index(_v869, px_str("idxs")), _v878)), px_str(")); ")), px_index(px_index(_v869, px_str("binds")), _v878)), _v879), _v876), px_str(" } "));
         _v878 = px_sub(_v878, px_int(1LL));
    }
    return _v876;
px_err_875:
    if (px_err_875_proped) return px_err_875_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v880 = (nargs > 0) ? args[0] : px_null();
    LXValue _v881 = px_null();
    LXValue _v882 = px_null();
    LXValue _v883 = px_null();
    LXValue _v884 = px_null();
    LXValue _v885 = px_null();
    LXValue _v886 = px_null();
    LXValue _v887 = px_null();
    LXValue _v888 = px_null();
    LXValue _v889 = px_null();
    LXValue _v890 = px_null();
    LXValue _v891 = px_null();
    LXValue _v892 = px_null();
    LXValue _v893 = px_null();
    LXValue _v894 = px_null();
    LXValue _v895 = px_null();
    LXValue _v896 = px_null();
    LXValue _v897 = px_null();
    LXValue px_err_898_val = px_null();
    int px_err_898_proped = 0;
    LXValue _v899 = px_index(_v880, px_int(0LL));
    if (px_is_truthy(px_eq(_v899, px_str("Int")))) {
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1)), px_str("LL)"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Float")))) {
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Str")))) {
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1)}, 1)), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Bool")))) {
        if (px_is_truthy(px_index(_v880, px_int(1LL)))) {
            return px_str("px_bool(true)");
        }
        return px_str("px_bool(false)");
    }
    if (px_is_truthy(px_eq(_v899, px_str("Null")))) {
        return px_str("px_null()");
    }
    if (px_is_truthy(px_eq(_v899, px_str("List")))) {
        LXValue _v900 = px_list_n((LXValue[]){}, 0);
        LXValue _v901 = px_index(_v880, px_int(1LL));
        LXValue _v902 = px_int(0LL);
        while (px_is_truthy(px_lt(_v902, px_call(px_get_global("len"), (LXValue[]){_v901}, 1)))) {
            (void)(px_method(_v900, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v901, _v902)}, 1)}, 1));
             _v902 = px_add(_v902, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v900}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v900}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Tuple")))) {
        LXValue _v903 = px_list_n((LXValue[]){}, 0);
        LXValue _v904 = px_index(_v880, px_int(1LL));
        LXValue _v905 = px_int(0LL);
        while (px_is_truthy(px_lt(_v905, px_call(px_get_global("len"), (LXValue[]){_v904}, 1)))) {
            (void)(px_method(_v903, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v904, _v905)}, 1)}, 1));
             _v905 = px_add(_v905, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v903}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v903}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Dict")))) {
        LXValue _v906 = px_str("({ LXValue _d = px_dict(); ");
        LXValue _v907 = px_index(_v880, px_int(1LL));
        LXValue _v908 = px_int(0LL);
        while (px_is_truthy(px_lt(_v908, px_call(px_get_global("len"), (LXValue[]){_v907}, 1)))) {
            LXValue _v909 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v907, _v908), px_int(0LL))}, 1);
            LXValue _v910 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v907, _v908), px_int(1LL))}, 1);
             _v906 = px_add(_v906, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v909), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v910), px_str("); } ")));
             _v908 = px_add(_v908, px_int(1LL));
        }
         _v906 = px_add(_v906, px_str("_d; })"));
        return _v906;
    }
    if (px_is_truthy(px_eq(_v899, px_str("Var")))) {
        LXValue _v911 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v912 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v911}, 1);
        if (px_is_truthy(px_ne(_v912, px_null()))) {
            return _v912;
        }
        return px_add(px_add(px_str("px_get_global(\""), _v911), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Field")))) {
        LXValue _v913 = px_index(_v880, px_int(1LL));
        LXValue _v914 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        if (px_is_truthy(px_eq(px_index(_v913, px_int(0LL)), px_str("Var")))) {
            LXValue _v915 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v913, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v915}, 1))) {
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v915), px_str("\", \"")), _v914), px_str("\")"));
            }
        }
        LXValue _v916 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v913}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v916), px_str(", \"")), _v914), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("OptionalField")))) {
        LXValue _v917 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v918 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v919 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v918), px_str(" = ")), _v917), px_str("; px_is_null(")), _v918), px_str(") ? px_null() : px_field(")), _v917), px_str(", \"")), _v919), px_str("\"); })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Index")))) {
        LXValue _v920 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v920), px_str(", ")), _v921), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Slice")))) {
        LXValue _v922 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v923 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v880, px_int(2LL)), px_null()))) {
             _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        }
        LXValue _v924 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v880, px_int(3LL)), px_null()))) {
             _v924 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
        }
        LXValue _v925 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v880, px_int(4LL)), px_null()))) {
             _v925 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(4LL))}, 1);
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v922), px_str(", ")), _v923), px_str(", ")), _v924), px_str(", ")), _v925), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Call")))) {
        LXValue _v926 = px_index(_v880, px_int(1LL));
        LXValue _v927 = px_index(_v880, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v926, px_int(0LL)), px_str("Var")))) {
            LXValue _v928 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v926, px_int(1LL))}, 1);
            if (px_is_truthy(px_eq(_v928, px_str("chan")))) {
                LXValue _v929 = px_str("0");
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v927}, 1), px_int(0LL)))) {
                    if (px_is_truthy(px_eq(px_index(px_index(_v927, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                         _v929 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v927, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                         _v929 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                return px_add(px_add(px_str("px_chan_create("), _v929), px_str(")"));
            }
            if (px_is_truthy(px_eq(_v928, px_str("mutex")))) {
                return px_str("px_mutex_create()");
            }
            if (px_is_truthy(px_eq(_v928, px_str("rwlock")))) {
                return px_str("px_rwlock_create()");
            }
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v928}, 1))) {
                LXValue _v930 = px_index(px_get_global("cg_structs"), _v928);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v930}, 1), px_call(px_get_global("len"), (LXValue[]){_v927}, 1)))) {
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v928), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v930}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1));
                }
                LXValue _v931 = px_list_n((LXValue[]){}, 0);
                LXValue _v932 = px_int(0LL);
                while (px_is_truthy(px_lt(_v932, px_call(px_get_global("len"), (LXValue[]){_v927}, 1)))) {
                    (void)(px_method(_v931, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, _v932)}, 1)}, 1));
                     _v932 = px_add(_v932, px_int(1LL));
                }
                LXValue _v933 = px_list_n((LXValue[]){}, 0);
                LXValue _v934 = px_int(0LL);
                while (px_is_truthy(px_lt(_v934, px_call(px_get_global("len"), (LXValue[]){_v930}, 1)))) {
                    (void)(px_method(_v933, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v930, _v934)), px_str("\""))}, 1));
                     _v934 = px_add(_v934, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v928), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v933}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v931}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v930}, 1)}, 1)), px_str(")"));
            }
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v928}, 1))) {
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v927}, 1), px_int(1LL)))) {
                    return px_add(px_add(px_str("枚举 "), _v928), px_str(" 构造需要一个变体名"));
                }
                LXValue _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, px_int(0LL))}, 1);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v928), px_str("\", (")), _v935), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        if (px_is_truthy(px_eq(px_index(_v926, px_int(0LL)), px_str("Field")))) {
            LXValue _v936 = px_index(_v926, px_int(1LL));
            LXValue _v937 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v926, px_int(2LL))}, 1);
            LXValue _v938 = px_null();
            if (px_is_truthy(px_eq(px_index(_v936, px_int(0LL)), px_str("Var")))) {
                LXValue _v939 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v936, px_int(1LL))}, 1);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v939}, 1))) {
                     _v938 = px_index(px_get_global("cg_var_types"), _v939);
                }
            }
            if (px_is_truthy(({ LXValue _t940 = px_ne(_v938, px_null()); px_is_truthy(_t940) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v938}, 1) : _t940; }))) {
                LXValue _v941 = px_index(px_get_global("cg_impls"), _v938);
                LXValue _v942 = px_bool(false);
                LXValue _v943 = px_int(0LL);
                while (px_is_truthy(px_lt(_v943, px_call(px_get_global("len"), (LXValue[]){_v941}, 1)))) {
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v941, _v943), px_int(1LL))}, 1), _v937))) {
                         _v942 = px_bool(true);
                        break;
                    }
                     _v943 = px_add(_v943, px_int(1LL));
                }
                if (px_is_truthy(_v942)) {
                    LXValue _v944 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v936}, 1);
                    LXValue _v945 = px_list_n((LXValue[]){_v944}, 1);
                    LXValue _v946 = px_int(0LL);
                    while (px_is_truthy(px_lt(_v946, px_call(px_get_global("len"), (LXValue[]){_v927}, 1)))) {
                        (void)(px_method(_v945, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, _v946)}, 1)}, 1));
                         _v946 = px_add(_v946, px_int(1LL));
                    }
                    LXValue _v947 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v938}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v937}, 1));
                    return px_add(px_add(px_add(px_add(px_add(_v947, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v945}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v945}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            LXValue _v948 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v936}, 1);
            LXValue _v949 = px_list_n((LXValue[]){}, 0);
            LXValue _v950 = px_int(0LL);
            while (px_is_truthy(px_lt(_v950, px_call(px_get_global("len"), (LXValue[]){_v927}, 1)))) {
                (void)(px_method(_v949, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, _v950)}, 1)}, 1));
                 _v950 = px_add(_v950, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v948), px_str(", \"")), _v937), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v949}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v949}, 1)}, 1)), px_str(")"));
        }
        LXValue _v951 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v926}, 1);
        LXValue _v952 = px_list_n((LXValue[]){}, 0);
        LXValue _v953 = px_int(0LL);
        while (px_is_truthy(px_lt(_v953, px_call(px_get_global("len"), (LXValue[]){_v927}, 1)))) {
            (void)(px_method(_v952, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, _v953)}, 1)}, 1));
             _v953 = px_add(_v953, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v951), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v952}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v952}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Unary")))) {
        LXValue _v954 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        LXValue _v955 = px_index(_v880, px_int(1LL));
        if (px_is_truthy(px_eq(_v955, px_str("Neg")))) {
            return px_add(px_add(px_str("px_neg("), _v954), px_str(")"));
        }
        if (px_is_truthy(px_eq(_v955, px_str("Not")))) {
            return px_add(px_add(px_str("px_not("), _v954), px_str(")"));
        }
        return px_add(px_add(px_str("px_bitnot("), _v954), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Binary")))) {
        LXValue _v956 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        LXValue _v957 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
        LXValue _v958 = px_index(_v880, px_int(1LL));
        if (px_is_truthy(px_eq(_v958, px_str("And")))) {
            LXValue _v959 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v959), px_str(" = ")), _v956), px_str("; px_is_truthy(")), _v959), px_str(") ? ")), _v957), px_str(" : ")), _v959), px_str("; })"));
        }
        if (px_is_truthy(px_eq(_v958, px_str("Or")))) {
            LXValue _v960 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v960), px_str(" = ")), _v956), px_str("; px_is_truthy(")), _v960), px_str(") ? ")), _v960), px_str(" : ")), _v957), px_str("; })"));
        }
        LXValue _v961 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v958}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v961, px_str("(")), _v956), px_str(", ")), _v957), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Pipe")))) {
        LXValue _v962 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v963 = px_index(_v880, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v963, px_int(0LL)), px_str("Call")))) {
            LXValue _v964 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v963, px_int(1LL))}, 1);
            LXValue _v965 = px_list_n((LXValue[]){_v962}, 1);
            LXValue _v966 = px_int(0LL);
            while (px_is_truthy(px_lt(_v966, px_call(px_get_global("len"), (LXValue[]){px_index(_v963, px_int(2LL))}, 1)))) {
                (void)(px_method(_v965, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v963, px_int(2LL)), _v966)}, 1)}, 1));
                 _v966 = px_add(_v966, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v964), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v965}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v965}, 1)}, 1)), px_str(")"));
        }
        LXValue _v967 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v963}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v967), px_str(", (LXValue[]){")), _v962), px_str("}, 1)"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("NullCoalesce")))) {
        LXValue _v968 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v969 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        LXValue _v970 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v970), px_str(" = ")), _v968), px_str("; px_is_null(")), _v970), px_str(") ? ")), _v969), px_str(" : ")), _v970), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Try")))) {
        LXValue _v971 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v972 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            LXValue _v973 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v972), px_str(" = ")), _v971), px_str("; if (px_is_result(")), _v972), px_str(")) { if (!px_result_ok(")), _v972), px_str(")) { ")), _v973), px_str("_val = ")), _v972), px_str("; ")), _v973), px_str("_proped = 1; goto ")), _v973), px_str("; } ")), _v972), px_str(" = px_result_unwrap(")), _v972), px_str("); } else if (px_is_null(")), _v972), px_str(")) { ")), _v973), px_str("_val = px_null(); ")), _v973), px_str("_proped = 1; goto ")), _v973), px_str("; } ")), _v972), px_str("; })"));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v972), px_str(" = ")), _v971), px_str("; if (px_is_result(")), _v972), px_str(") && !px_result_ok(")), _v972), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v972), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v972), px_str(")) ")), _v972), px_str(" = px_result_unwrap(")), _v972), px_str("); ")), _v972), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("ForceUnwrap")))) {
        LXValue _v974 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v975 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v975), px_str(" = ")), _v974), px_str("; if (px_is_result(")), _v975), px_str(")) { if (!px_result_ok(")), _v975), px_str(")) px_error(\"force unwrap Err\"); ")), _v975), px_str(" = px_result_unwrap(")), _v975), px_str("); } if (px_is_null(")), _v975), px_str(")) px_error(\"force unwrap null\"); ")), _v975), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("IfExpr")))) {
        LXValue _v976 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v977 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        LXValue _v978 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
        LXValue _v979 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v979), px_str("; if (px_is_truthy(")), _v976), px_str(")) { ")), _v979), px_str(" = ")), _v977), px_str("; } else { ")), _v979), px_str(" = ")), _v978), px_str("; } ")), _v979), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("ListComp")))) {
        LXValue _v980 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v981 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        LXValue _v982 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v983 = px_null();
        if (px_is_truthy(px_ne(px_index(_v880, px_int(3LL)), px_null()))) {
             _v983 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v981, px_str("saved_all"))}, 1));
        LXValue _v984 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v980), px_str(", ")), _v982), px_str("); "));
        LXValue _v985 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v981, _v983, _v984}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v980), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v981, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v981, px_str("its")), px_int(0LL))), px_str("; ")), _v985), px_str(" ")), _v980), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("GenExp")))) {
        LXValue _v986 = px_index(_v880, px_int(2LL));
        if (px_is_truthy(({ LXValue _t987 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v986}, 1), px_int(1LL)); px_is_truthy(_t987) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v986, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t987; }))) {
            LXValue _v988 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v986, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            LXValue _v989 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v986, px_int(0LL)), px_int(2LL))}, 1);
            LXValue _v990 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v988}, 1), px_index(_v880, px_int(1LL))}, 2);
            LXValue _v991 = px_str("px_null()");
            if (px_is_truthy(px_ne(px_index(_v880, px_int(3LL)), px_null()))) {
                 _v991 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v988}, 1), px_index(_v880, px_int(3LL))}, 2);
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v989), px_str(", ")), _v990), px_str(", ")), _v991), px_str(")"));
        }
        LXValue _v992 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v993 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v986}, 1);
        LXValue _v994 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v995 = px_null();
        if (px_is_truthy(px_ne(px_index(_v880, px_int(3LL)), px_null()))) {
             _v995 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v993, px_str("saved_all"))}, 1));
        LXValue _v996 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v992), px_str(", ")), _v994), px_str("); "));
        LXValue _v997 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v993, _v995, _v996}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v992), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v993, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v993, px_str("its")), px_int(0LL))), px_str("; ")), _v997), px_str(" px_gen_from_list(")), _v992), px_str("); })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("DictComp")))) {
        LXValue _v998 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v999 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
        LXValue _v1000 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v1001 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(2LL))}, 1);
        LXValue _v1002 = px_null();
        if (px_is_truthy(px_ne(px_index(_v880, px_int(4LL)), px_null()))) {
             _v1002 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(4LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v999, px_str("saved_all"))}, 1));
        LXValue _v1003 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v1000), px_str("; LXValue _v = ")), _v1001), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v998), px_str(", _k.as.obj->as.str.data, _v); } "));
        LXValue _v1004 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v999, _v1002, _v1003}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v998), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v999, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v999, px_str("its")), px_int(0LL))), px_str("; ")), _v1004), px_str(" ")), _v998), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Closure")))) {
        px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
        LXValue _v1005 = px_get_global("cg_closure_id");
        LXValue _v1006 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1005}, 1));
        LXValue _v1007 = px_add(px_add(px_str("static LXValue "), _v1006), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
         _v1007 = px_add(_v1007, px_str("    (void)ctx;\n"));
        LXValue _v1008 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
        LXValue _v1009 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
        px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        LXValue _v1010 = px_index(_v880, px_int(1LL));
        LXValue _v1011 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1011, px_call(px_get_global("len"), (LXValue[]){_v1010}, 1)))) {
            LXValue _v1012 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1010, _v1011), px_int(1LL))}, 1)}, 1);
             _v1007 = px_add(_v1007, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1012), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1011}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1011}, 1)), px_str("] : px_null();\n")));
             _v1011 = px_add(_v1011, px_int(1LL));
        }
        LXValue _v1013 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(3LL))}, 1);
         _v1007 = px_add(_v1007, px_add(px_add(px_str("    return "), _v1013), px_str(";\n")));
         _v1007 = px_add(_v1007, px_str("}\n"));
        px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1007));
        px_set_global("cg_vars", _v1008);
        px_set_global("cg_var_types", _v1009);
        return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1005}, 1)), px_str(">\", ")), _v1006), px_str(", NULL)"));
    }
    if (px_is_truthy(px_eq(_v899, px_str("Block")))) {
        LXValue _v1014 = px_str("({ ");
         _v1014 = px_add(_v1014, px_str("LXValue _blk = px_null(); "));
        LXValue _v1015 = px_index(_v880, px_int(1LL));
        LXValue _v1016 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1016, px_call(px_get_global("len"), (LXValue[]){_v1015}, 1)))) {
            LXValue _v1017 = px_index(_v1015, _v1016);
            if (px_is_truthy(px_eq(px_index(_v1017, px_int(0LL)), px_str("ExprStmt")))) {
                LXValue _v1018 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1017, px_int(1LL))}, 1);
                 _v1014 = px_add(_v1014, px_add(px_add(px_str("_blk = "), _v1018), px_str("; ")));
            }
            else {
                 _v1014 = px_add(_v1014, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1017, px_int(0LL)}, 2));
            }
             _v1016 = px_add(_v1016, px_int(1LL));
        }
         _v1014 = px_add(_v1014, px_str("_blk; })"));
        return _v1014;
    }
    if (px_is_truthy(px_eq(_v899, px_str("Match")))) {
        LXValue _v1019 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v1020 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1021 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1020), px_str(" = ")), _v1019), px_str("; "));
        LXValue _v1022 = px_index(_v880, px_int(2LL));
        LXValue _v1023 = px_bool(true);
        LXValue _v1024 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1024, px_call(px_get_global("len"), (LXValue[]){_v1022}, 1)))) {
            LXValue _v1025 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v1022, _v1024), px_int(1LL)), _v1020}, 2);
            LXValue _v1026 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v1022, _v1024), px_int(3LL))}, 1);
            LXValue _v1027 = px_str("if");
            if (px_is_truthy(px_not(_v1023))) {
                 _v1027 = px_str("else if");
            }
             _v1021 = px_add(_v1021, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v1027, px_str(" (")), _v1025), px_str(") { ")), _v1020), px_str(" = ")), _v1026), px_str("; } ")));
             _v1023 = px_bool(false);
             _v1024 = px_add(_v1024, px_int(1LL));
        }
         _v1021 = px_add(_v1021, px_add(_v1020, px_str("; })")));
        return _v1021;
    }
    if (px_is_truthy(px_eq(_v899, px_str("Constructor")))) {
        LXValue _v1028 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v880, px_int(1LL))}, 1);
        LXValue _v1029 = px_index(_v880, px_int(2LL));
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v1028}, 1))) {
            LXValue _v1030 = px_index(px_get_global("cg_structs"), _v1028);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1030}, 1), px_call(px_get_global("len"), (LXValue[]){_v1029}, 1)))) {
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v1028), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1030}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1029}, 1)}, 1));
            }
            LXValue _v1031 = px_list_n((LXValue[]){}, 0);
            LXValue _v1032 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1032, px_call(px_get_global("len"), (LXValue[]){_v1029}, 1)))) {
                (void)(px_method(_v1031, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1029, _v1032)}, 1)}, 1));
                 _v1032 = px_add(_v1032, px_int(1LL));
            }
            LXValue _v1033 = px_list_n((LXValue[]){}, 0);
            LXValue _v1034 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1034, px_call(px_get_global("len"), (LXValue[]){_v1030}, 1)))) {
                (void)(px_method(_v1033, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v1030, _v1034)), px_str("\""))}, 1));
                 _v1034 = px_add(_v1034, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v1028), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1033}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1031}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1030}, 1)}, 1)), px_str(")"));
        }
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v1028}, 1))) {
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1029}, 1), px_int(1LL)))) {
                return px_add(px_add(px_str("枚举 "), _v1028), px_str(" 构造需要一个变体名"));
            }
            LXValue _v1035 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1029, px_int(0LL))}, 1);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v1028), px_str("\", (")), _v1035), px_str(").as.obj->as.enum_inst.variant)"));
        }
        LXValue _v1036 = px_list_n((LXValue[]){}, 0);
        LXValue _v1037 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1037, px_call(px_get_global("len"), (LXValue[]){_v1029}, 1)))) {
            (void)(px_method(_v1036, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1029, _v1037)}, 1)}, 1));
             _v1037 = px_add(_v1037, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v1028), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1036}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1036}, 1)}, 1)), px_str(")"));
    }
    return px_str("px_null()");
px_err_898:
    if (px_err_898_proped) return px_err_898_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1038 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1039_val = px_null();
    int px_err_1039_proped = 0;
    if (px_is_truthy(px_eq(_v1038, px_str("Add")))) {
        return px_str("px_add");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Sub")))) {
        return px_str("px_sub");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Mul")))) {
        return px_str("px_mul");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Div")))) {
        return px_str("px_div");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("IntDiv")))) {
        return px_str("px_idiv");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Mod")))) {
        return px_str("px_mod");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Pow")))) {
        return px_str("px_pow");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Eq")))) {
        return px_str("px_eq");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Ne")))) {
        return px_str("px_ne");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Lt")))) {
        return px_str("px_lt");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Le")))) {
        return px_str("px_le");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Gt")))) {
        return px_str("px_gt");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Ge")))) {
        return px_str("px_ge");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("BitAnd")))) {
        return px_str("px_bitand");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("BitOr")))) {
        return px_str("px_bitor");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("BitXor")))) {
        return px_str("px_bitxor");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Shl")))) {
        return px_str("px_shl");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("Shr")))) {
        return px_str("px_shr");
    }
    if (px_is_truthy(px_eq(_v1038, px_str("ShrU")))) {
        return px_str("px_ushr");
    }
    return px_str("px_add");
px_err_1039:
    if (px_err_1039_proped) return px_err_1039_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1040 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1041 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1042_val = px_null();
    int px_err_1042_proped = 0;
    LXValue _v1043 = px_index(_v1040, px_int(0LL));
    if (px_is_truthy(px_eq(_v1043, px_str("PatLiteral")))) {
        LXValue _v1044 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1040, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v1041), px_str(", ")), _v1044), px_str("))"));
    }
    if (px_is_truthy(px_eq(_v1043, px_str("PatBinding")))) {
        LXValue _v1045 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1040, px_int(1LL))}, 1);
        if (px_is_truthy(({ LXValue _t1047 = ({ LXValue _t1046 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1045}, 1), px_int(0LL)); px_is_truthy(_t1046) ? px_ge(px_index(_v1045, px_int(0LL)), px_str("A")) : _t1046; }); px_is_truthy(_t1047) ? px_le(px_index(_v1045, px_int(0LL)), px_str("Z")) : _t1047; }))) {
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v1041), px_str(".type == PX_ENUM && strcmp(")), _v1041), px_str(".as.obj->as.enum_inst.variant, \"")), _v1045), px_str("\") == 0)"));
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1043, px_str("PatWildcard")))) {
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1043, px_str("PatTuple")))) {
        LXValue _v1048 = px_index(_v1040, px_int(1LL));
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1048}, 1), px_int(0LL)))) {
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v1048, px_int(0LL)), _v1041}, 2);
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1043, px_str("PatConstructor")))) {
        LXValue _v1049 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1040, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v1041), px_str(".type == PX_ENUM && strcmp(")), _v1041), px_str(".as.obj->as.enum_inst.variant, \"")), _v1049), px_str("\") == 0)"));
    }
    return px_str("true");
px_err_1042:
    if (px_err_1042_proped) return px_err_1042_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1050 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1051 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1052 = px_null();
    LXValue _v1053 = px_null();
    LXValue px_err_1054_val = px_null();
    int px_err_1054_proped = 0;
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    LXValue _v1055 = px_get_global("cg_closure_id");
    LXValue _v1056 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1055}, 1));
    LXValue _v1057 = px_add(px_add(px_str("static LXValue "), _v1056), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1057 = px_add(_v1057, px_str("    (void)ctx;\n"));
    LXValue _v1058 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v1059 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1060 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1060, px_call(px_get_global("len"), (LXValue[]){_v1050}, 1)))) {
        LXValue _v1061 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v1050, _v1060)}, 1);
         _v1057 = px_add(_v1057, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1061), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1060}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1060}, 1)), px_str("] : px_null();\n")));
         _v1060 = px_add(_v1060, px_int(1LL));
    }
    LXValue _v1062 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v1051}, 1);
    LXValue _v1063 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1063}, 1));
     _v1057 = px_add(_v1057, px_add(px_add(px_str("    LXValue "), _v1063), px_str("_val = px_null();\n")));
     _v1057 = px_add(_v1057, px_add(px_add(px_str("    int "), _v1063), px_str("_proped = 0;\n")));
     _v1057 = px_add(_v1057, px_add(px_add(px_str("    return "), _v1062), px_str(";\n")));
     _v1057 = px_add(_v1057, px_add(_v1063, px_str(":\n")));
     _v1057 = px_add(_v1057, px_add(px_add(px_add(px_add(px_str("    if ("), _v1063), px_str("_proped) return ")), _v1063), px_str("_val;\n")));
     _v1057 = px_add(_v1057, px_str("    return px_null();\n"));
     _v1057 = px_add(_v1057, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1057));
    px_set_global("cg_vars", _v1058);
    px_set_global("cg_var_types", _v1059);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1055}, 1)), px_str(">\", ")), _v1056), px_str(", NULL)"));
px_err_1054:
    if (px_err_1054_proped) return px_err_1054_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1064 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1065 = px_null();
    LXValue px_err_1066_val = px_null();
    int px_err_1066_proped = 0;
    LXValue _v1067 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1064}, 1), px_int(1LL));
    while (px_is_truthy(px_ge(_v1067, px_int(0LL)))) {
        if (px_is_truthy(px_eq(px_index(_v1064, _v1067), px_str("/")))) {
            if (px_is_truthy(px_eq(_v1067, px_int(0LL)))) {
                return px_str("/");
            }
            return px_slice(_v1064, px_int(0LL), _v1067, px_null());
        }
         _v1067 = px_sub(_v1067, px_int(1LL));
    }
    return px_str(".");
px_err_1066:
    if (px_err_1066_proped) return px_err_1066_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1068 = px_null();
    LXValue px_err_1069_val = px_null();
    int px_err_1069_proped = 0;
    LXValue _v1070 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    if (px_is_truthy(({ LXValue _t1071 = px_ne(_v1070, px_null()); px_is_truthy(_t1071) ? px_call(px_get_global("exists"), (LXValue[]){_v1070}, 1) : _t1071; }))) {
        return _v1070;
    }
    LXValue _v1072 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    LXValue _v1073 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1073, px_call(px_get_global("len"), (LXValue[]){_v1072}, 1)))) {
        LXValue _v1074 = px_index(_v1072, _v1073);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1074}, 1))) {
            return _v1074;
        }
         _v1073 = px_add(_v1073, px_int(1LL));
    }
    return px_null();
px_err_1069:
    if (px_err_1069_proped) return px_err_1069_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1075 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1076 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1077 = px_null();
    LXValue _v1078 = px_null();
    LXValue _v1079 = px_null();
    LXValue _v1080 = px_null();
    LXValue px_err_1081_val = px_null();
    int px_err_1081_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1075}, 1), px_int(0LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t1083 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1075}, 1), px_int(1LL)); px_is_truthy(_t1083) ? ({ LXValue _t1082 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v1075, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1082) ? _t1082 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v1075, px_int(0LL)), px_str(".px")}, 2); }) : _t1083; }))) {
        LXValue _v1084 = px_index(_v1075, px_int(0LL));
        LXValue _v1085 = _v1084;
        if (px_is_truthy(px_not(({ LXValue _t1086 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1084}, 1), px_int(0LL)); px_is_truthy(_t1086) ? px_eq(px_index(_v1084, px_int(0LL)), px_str("/")) : _t1086; })))) {
             _v1085 = px_add(px_add(_v1076, px_str("/")), _v1084);
        }
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1085}, 1))) {
            return _v1085;
        }
        return px_null();
    }
    if (px_is_truthy(px_eq(px_index(_v1075, px_int(0LL)), px_str("std")))) {
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v1075}, 1), px_int(2LL)))) {
            return px_null();
        }
        LXValue _v1087 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v1087, px_null()))) {
            return px_null();
        }
        LXValue _v1088 = _v1087;
        LXValue _v1089 = px_int(1LL);
        while (px_is_truthy(px_lt(_v1089, px_call(px_get_global("len"), (LXValue[]){_v1075}, 1)))) {
             _v1088 = px_add(_v1088, px_add(px_str("/"), px_index(_v1075, _v1089)));
             _v1089 = px_add(_v1089, px_int(1LL));
        }
        LXValue _v1090 = px_add(_v1088, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1090}, 1))) {
            return _v1090;
        }
        LXValue _v1091 = px_add(_v1088, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1091}, 1))) {
            return _v1091;
        }
        return px_null();
    }
    LXValue _v1092 = px_list_n((LXValue[]){_v1076}, 1);
    LXValue _v1093 = px_add(_v1076, px_str("/.px_modules"));
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1093}, 1))) {
        (void)(px_method(_v1092, "append", (LXValue[]){_v1093}, 1));
        LXValue _v1094 = px_call(px_get_global("list_dir"), (LXValue[]){_v1093}, 1);
        LXValue _v1095 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1095, px_call(px_get_global("len"), (LXValue[]){_v1094}, 1)))) {
            LXValue _v1096 = px_index(_v1094, _v1095);
            LXValue _v1097 = px_add(px_add(_v1093, px_str("/")), _v1096);
            if (px_is_truthy(({ LXValue _t1098 = px_call(px_get_global("exists"), (LXValue[]){_v1097}, 1); px_is_truthy(_t1098) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1096, px_str(".")}, 2)) : _t1098; }))) {
                (void)(px_method(_v1092, "append", (LXValue[]){_v1097}, 1));
            }
             _v1095 = px_add(_v1095, px_int(1LL));
        }
    }
    LXValue _v1099 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1099, px_call(px_get_global("len"), (LXValue[]){_v1092}, 1)))) {
        LXValue _v1100 = px_index(_v1092, _v1099);
        LXValue _v1101 = _v1100;
        LXValue _v1102 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1102, px_call(px_get_global("len"), (LXValue[]){_v1075}, 1)))) {
             _v1101 = px_add(_v1101, px_add(px_str("/"), px_index(_v1075, _v1102)));
             _v1102 = px_add(_v1102, px_int(1LL));
        }
        LXValue _v1103 = px_add(_v1101, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1103}, 1))) {
            return _v1103;
        }
        LXValue _v1104 = px_add(_v1101, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1104}, 1))) {
            return _v1104;
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1075}, 1), px_int(1LL)))) {
            LXValue _v1105 = px_add(px_add(px_add(_v1100, px_str("/")), px_index(_v1075, px_int(0LL))), px_str(".px"));
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1105}, 1))) {
                return _v1105;
            }
        }
         _v1099 = px_add(_v1099, px_int(1LL));
    }
    return px_null();
px_err_1081:
    if (px_err_1081_proped) return px_err_1081_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1106 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1107_val = px_null();
    int px_err_1107_proped = 0;
    LXValue _v1108 = px_index(_v1106, px_int(0LL));
    if (px_is_truthy(px_eq(_v1108, px_str("FuncDef")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1106, px_int(1LL))}, 1), px_str("main")))) {
            return px_bool(false);
        }
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t1111 = ({ LXValue _t1110 = ({ LXValue _t1109 = px_eq(_v1108, px_str("StructDef")); px_is_truthy(_t1109) ? _t1109 : px_eq(_v1108, px_str("EnumDef")); }); px_is_truthy(_t1110) ? _t1110 : px_eq(_v1108, px_str("TraitDef")); }); px_is_truthy(_t1111) ? _t1111 : px_eq(_v1108, px_str("ImplDef")); }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t1112 = px_eq(_v1108, px_str("VarDecl")); px_is_truthy(_t1112) ? px_eq(px_index(_v1106, px_int(1LL)), px_str("Const")) : _t1112; }))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_1107:
    if (px_err_1107_proped) return px_err_1107_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1113 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1114 = px_null();
    LXValue px_err_1115_val = px_null();
    int px_err_1115_proped = 0;
    LXValue _v1116 = px_index(_v1113, px_int(0LL));
    if (px_is_truthy(({ LXValue _t1119 = ({ LXValue _t1118 = ({ LXValue _t1117 = px_eq(_v1116, px_str("FuncDef")); px_is_truthy(_t1117) ? _t1117 : px_eq(_v1116, px_str("StructDef")); }); px_is_truthy(_t1118) ? _t1118 : px_eq(_v1116, px_str("EnumDef")); }); px_is_truthy(_t1119) ? _t1119 : px_eq(_v1116, px_str("TraitDef")); }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1113, px_int(1LL))}, 1);
    }
    if (px_is_truthy(({ LXValue _t1120 = px_eq(_v1116, px_str("VarDecl")); px_is_truthy(_t1120) ? px_eq(px_index(_v1113, px_int(1LL)), px_str("Const")) : _t1120; }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1113, px_int(2LL))}, 1);
    }
    if (px_is_truthy(px_eq(_v1116, px_str("ImplDef")))) {
        LXValue _v1121 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1113, px_int(1LL))}, 1);
        LXValue _v1122 = px_index(_v1113, px_int(2LL));
        LXValue _v1123 = px_str("None");
        if (px_is_truthy(px_ne(_v1122, px_null()))) {
             _v1123 = px_add(px_add(px_str("Some("), _v1122), px_str(")"));
        }
        return px_add(px_add(px_add(px_str("impl::"), _v1121), px_str("::")), _v1123);
    }
    return px_null();
px_err_1115:
    if (px_err_1115_proped) return px_err_1115_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1124 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1125 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1126 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1127 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1128 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1129 = px_null();
    LXValue _v1130 = px_null();
    LXValue _v1131 = px_null();
    LXValue _v1132 = px_null();
    LXValue _v1133 = px_null();
    LXValue px_err_1134_val = px_null();
    int px_err_1134_proped = 0;
    LXValue _v1135 = px_list_n((LXValue[]){}, 0);
    LXValue _v1136 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1136, px_call(px_get_global("len"), (LXValue[]){_v1124}, 1)))) {
        (void)(px_method(_v1135, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1124, _v1136)}, 1)}, 1));
         _v1136 = px_add(_v1136, px_int(1LL));
    }
     _v1124 = _v1135;
    LXValue _v1137 = px_list_n((LXValue[]){}, 0);
    LXValue _v1138 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1138, px_call(px_get_global("len"), (LXValue[]){_v1125}, 1)))) {
        (void)(px_method(_v1137, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1125, _v1138)}, 1)}, 1));
         _v1138 = px_add(_v1138, px_int(1LL));
    }
     _v1125 = _v1137;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1124}, 1), px_int(0LL)))) {
        return px_null();
    }
    LXValue _v1139 = px_eq(px_index(_v1124, px_int(0LL)), px_str("std"));
    LXValue _v1140 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1124}, 2);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1140}, 1))) {
        return px_null();
    }
    LXValue _v1141 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1124, _v1126}, 2);
    if (px_is_truthy(px_eq(_v1141, px_null()))) {
        if (px_is_truthy(px_ne(px_index(_v1124, px_int(0LL)), px_str("std")))) {
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("[module] 警告: 找不到模块 '"), _v1140), px_str("'（已跳过）"))}, 1));
        }
        return px_null();
    }
    px_index_set(px_get_global("loaded"), _v1140, _v1141);
    LXValue _v1142 = px_call(px_get_global("read_file"), (LXValue[]){_v1141}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1142}, 1));
    px_set_global("p_pos", px_int(0LL));
    LXValue _v1143 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    LXValue _v1144 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1141}, 1);
    LXValue _v1145 = px_list_n((LXValue[]){}, 0);
    LXValue _v1146 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1146, px_call(px_get_global("len"), (LXValue[]){px_index(_v1143, px_int(1LL))}, 1)))) {
        LXValue _v1147 = px_index(px_index(_v1143, px_int(1LL)), _v1146);
        if (px_is_truthy(px_eq(px_index(_v1147, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v1145, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1147, px_int(1LL)), px_index(_v1147, px_int(2LL))}, 2)}, 1));
        }
         _v1146 = px_add(_v1146, px_int(1LL));
    }
    LXValue _v1148 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1148, px_call(px_get_global("len"), (LXValue[]){_v1145}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1145, _v1148), px_int(0LL)), px_index(px_index(_v1145, _v1148), px_int(1LL)), _v1144, _v1127, _v1128}, 5));
         _v1148 = px_add(_v1148, px_int(1LL));
    }
    LXValue _v1149 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1125}, 1), px_int(0LL));
    LXValue _v1150 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1150, px_call(px_get_global("len"), (LXValue[]){px_index(_v1143, px_int(1LL))}, 1)))) {
        LXValue _v1151 = px_index(px_index(_v1143, px_int(1LL)), _v1150);
        if (px_is_truthy(px_eq(px_index(_v1151, px_int(0LL)), px_str("Import")))) {
             _v1150 = px_add(_v1150, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1151}, 1)))) {
             _v1150 = px_add(_v1150, px_int(1LL));
            continue;
        }
        LXValue _v1152 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1151}, 1);
        if (px_is_truthy(px_eq(_v1152, px_null()))) {
            (void)(px_method(_v1127, "append", (LXValue[]){_v1151}, 1));
        }
        else {
            if (px_is_truthy(_v1149)) {
                if (px_is_truthy(({ LXValue _t1153 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1152}, 1), px_int(5LL)); px_is_truthy(_t1153) ? px_eq(px_slice(_v1152, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1153; }))) {
                     _v1150 = px_add(_v1150, px_int(1LL));
                    continue;
                }
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1125, _v1152}, 2)))) {
                     _v1150 = px_add(_v1150, px_int(1LL));
                    continue;
                }
            }
            if (px_is_truthy(_v1139)) {
                if (px_is_truthy(px_method(_v1128, "has", (LXValue[]){_v1152}, 1))) {
                     _v1150 = px_add(_v1150, px_int(1LL));
                    continue;
                }
                px_index_set(_v1128, _v1152, px_bool(true));
            }
            (void)(px_method(_v1127, "append", (LXValue[]){_v1151}, 1));
        }
         _v1150 = px_add(_v1150, px_int(1LL));
    }
px_err_1134:
    if (px_err_1134_proped) return px_err_1134_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1154 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1155 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1156 = px_null();
    LXValue _v1157 = px_null();
    LXValue _v1158 = px_null();
    LXValue _v1159 = px_null();
    LXValue px_err_1160_val = px_null();
    int px_err_1160_proped = 0;
    LXValue _v1161 = px_index(_v1154, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1161}, 1), px_int(0LL)))) {
        return _v1154;
    }
    LXValue _v1162 = px_list_n((LXValue[]){}, 0);
    LXValue _v1163 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1164 = px_list_n((LXValue[]){}, 0);
    LXValue _v1165 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1165, px_call(px_get_global("len"), (LXValue[]){_v1161}, 1)))) {
        LXValue _v1166 = px_index(_v1161, _v1165);
        if (px_is_truthy(px_eq(px_index(_v1166, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v1164, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1166, px_int(1LL)), px_index(_v1166, px_int(2LL))}, 2)}, 1));
        }
         _v1165 = px_add(_v1165, px_int(1LL));
    }
    LXValue _v1167 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1167, px_call(px_get_global("len"), (LXValue[]){_v1164}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1164, _v1167), px_int(0LL)), px_index(px_index(_v1164, _v1167), px_int(1LL)), _v1155, _v1162, _v1163}, 5));
         _v1167 = px_add(_v1167, px_int(1LL));
    }
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1162}, 1), px_int(0LL)))) {
        return _v1154;
    }
    LXValue _v1168 = px_list_n((LXValue[]){}, 0);
    LXValue _v1169 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1169, px_call(px_get_global("len"), (LXValue[]){_v1162}, 1)))) {
        (void)(px_method(_v1168, "append", (LXValue[]){px_index(_v1162, _v1169)}, 1));
         _v1169 = px_add(_v1169, px_int(1LL));
    }
    LXValue _v1170 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1170, px_call(px_get_global("len"), (LXValue[]){_v1161}, 1)))) {
        (void)(px_method(_v1168, "append", (LXValue[]){px_index(_v1161, _v1170)}, 1));
         _v1170 = px_add(_v1170, px_int(1LL));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v1168}, 2);
px_err_1160:
    if (px_err_1160_proped) return px_err_1160_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1171_val = px_null();
    int px_err_1171_proped = 0;
    LXValue _v1172 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v1172, "remove", (LXValue[]){px_str("_")}, 1));
    return _v1172;
px_err_1171:
    if (px_err_1171_proped) return px_err_1171_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1173 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1174 = px_null();
    LXValue px_err_1175_val = px_null();
    int px_err_1175_proped = 0;
    LXValue _v1176 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    LXValue _v1177 = px_method(_v1173, "keys", (LXValue[]){}, 0);
    LXValue _v1178 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1178, px_call(px_get_global("len"), (LXValue[]){_v1177}, 1)))) {
        px_index_set(_v1176, px_index(_v1177, _v1178), px_index(_v1173, px_index(_v1177, _v1178)));
         _v1178 = px_add(_v1178, px_int(1LL));
    }
    return _v1176;
px_err_1175:
    if (px_err_1175_proped) return px_err_1175_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1179_val = px_null();
    int px_err_1179_proped = 0;
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    return px_get_global("cg_uidc");
px_err_1179:
    if (px_err_1179_proped) return px_err_1179_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1180_val = px_null();
    int px_err_1180_proped = 0;
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1180:
    if (px_err_1180_proped) return px_err_1180_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1181 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1182_val = px_null();
    int px_err_1182_proped = 0;
    LXValue _v1183 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_index_set(px_get_global("cg_vars"), _v1181, _v1183);
    return _v1183;
px_err_1182:
    if (px_err_1182_proped) return px_err_1182_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1184 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1185_val = px_null();
    int px_err_1185_proped = 0;
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1184}, 1))) {
        return px_index(px_get_global("cg_vars"), _v1184);
    }
    return px_null();
px_err_1185:
    if (px_err_1185_proped) return px_err_1185_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1186 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1187 = px_null();
    LXValue _v1188 = px_null();
    LXValue px_err_1189_val = px_null();
    int px_err_1189_proped = 0;
    LXValue _v1190 = px_str("");
    LXValue _v1191 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1191, px_call(px_get_global("len"), (LXValue[]){_v1186}, 1)))) {
        LXValue _v1192 = px_index(_v1186, _v1191);
        LXValue _v1194 = ({ LXValue _t1193 = px_ge(_v1192, px_str("a")); px_is_truthy(_t1193) ? px_le(_v1192, px_str("z")) : _t1193; });
        LXValue _v1196 = ({ LXValue _t1195 = px_ge(_v1192, px_str("A")); px_is_truthy(_t1195) ? px_le(_v1192, px_str("Z")) : _t1195; });
        LXValue _v1198 = ({ LXValue _t1197 = px_ge(_v1192, px_str("0")); px_is_truthy(_t1197) ? px_le(_v1192, px_str("9")) : _t1197; });
        if (px_is_truthy(({ LXValue _t1200 = ({ LXValue _t1199 = _v1194; px_is_truthy(_t1199) ? _t1199 : _v1196; }); px_is_truthy(_t1200) ? _t1200 : _v1198; }))) {
             _v1190 = px_add(_v1190, _v1192);
        }
        else {
             _v1190 = px_add(_v1190, px_str("_"));
        }
         _v1191 = px_add(_v1191, px_int(1LL));
    }
    return _v1190;
px_err_1189:
    if (px_err_1189_proped) return px_err_1189_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1201 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1202 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1203 = px_null();
    LXValue _v1204 = px_null();
    LXValue _v1205 = px_null();
    LXValue px_err_1206_val = px_null();
    int px_err_1206_proped = 0;
    LXValue _v1207 = px_call(px_get_global("len"), (LXValue[]){_v1201}, 1);
    LXValue _v1208 = px_call(px_get_global("len"), (LXValue[]){_v1202}, 1);
    LXValue _v1209 = px_int(0LL);
    while (px_is_truthy(px_le(px_add(_v1209, _v1208), _v1207))) {
        LXValue _v1210 = px_int(0LL);
        LXValue _v1211 = px_bool(true);
        while (px_is_truthy(px_lt(_v1210, _v1208))) {
            if (px_is_truthy(px_ne(px_index(_v1201, px_add(_v1209, _v1210)), px_index(_v1202, _v1210)))) {
                 _v1211 = px_bool(false);
                break;
            }
             _v1210 = px_add(_v1210, px_int(1LL));
        }
        if (px_is_truthy(_v1211)) {
            return _v1209;
        }
         _v1209 = px_add(_v1209, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_1206:
    if (px_err_1206_proped) return px_err_1206_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1212 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1213 = px_null();
    LXValue _v1214 = px_null();
    LXValue px_err_1215_val = px_null();
    int px_err_1215_proped = 0;
    LXValue _v1216 = px_str("");
    LXValue _v1217 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1217, _v1212))) {
         _v1216 = px_add(_v1216, px_str("    "));
         _v1217 = px_add(_v1217, px_int(1LL));
    }
    return _v1216;
px_err_1215:
    if (px_err_1215_proped) return px_err_1215_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1218 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1219 = px_null();
    LXValue _v1220 = px_null();
    LXValue _v1221 = px_null();
    LXValue _v1222 = px_null();
    LXValue px_err_1223_val = px_null();
    int px_err_1223_proped = 0;
    LXValue _v1224 = px_slice(_v1218, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1218}, 1), px_int(1LL)), px_null());
    LXValue _v1225 = px_str("");
    LXValue _v1226 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1226, px_call(px_get_global("len"), (LXValue[]){_v1224}, 1)))) {
        LXValue _v1227 = px_index(_v1224, _v1226);
        if (px_is_truthy(px_eq(_v1227, px_str("\\")))) {
            LXValue _v1228 = px_index(_v1224, px_add(_v1226, px_int(1LL)));
            if (px_is_truthy(px_eq(_v1228, px_str("n")))) {
                 _v1225 = px_add(_v1225, px_str("\n"));
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1228, px_str("t")))) {
                 _v1225 = px_add(_v1225, px_str("\t"));
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1228, px_str("r")))) {
                 _v1225 = px_add(_v1225, px_str("\r"));
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1228, px_str("0")))) {
                 _v1225 = px_add(_v1225, px_str(""));
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1228, px_str("\"")))) {
                 _v1225 = px_add(_v1225, px_str("\""));
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1228, px_str("\\")))) {
                 _v1225 = px_add(_v1225, px_str("\\"));
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1228, px_str("u")))) {
                LXValue _v1229 = px_add(_v1226, px_int(3LL));
                LXValue _v1230 = px_str("");
                while (px_is_truthy(({ LXValue _t1231 = px_lt(_v1229, px_call(px_get_global("len"), (LXValue[]){_v1224}, 1)); px_is_truthy(_t1231) ? px_ne(px_index(_v1224, _v1229), px_str("}")) : _t1231; }))) {
                     _v1230 = px_add(_v1230, px_index(_v1224, _v1229));
                     _v1229 = px_add(_v1229, px_int(1LL));
                }
                 _v1225 = px_add(_v1225, px_call(px_get_global("hex_to_char"), (LXValue[]){_v1230}, 1));
                 _v1226 = px_add(_v1229, px_int(1LL));
            }
            else {
                 _v1225 = px_add(_v1225, _v1228);
                 _v1226 = px_add(_v1226, px_int(2LL));
            }
        }
        else {
             _v1225 = px_add(_v1225, _v1227);
             _v1226 = px_add(_v1226, px_int(1LL));
        }
    }
    return _v1225;
px_err_1223:
    if (px_err_1223_proped) return px_err_1223_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1232 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1233 = px_null();
    LXValue _v1234 = px_null();
    LXValue px_err_1235_val = px_null();
    int px_err_1235_proped = 0;
    LXValue _v1236 = px_str("");
    LXValue _v1237 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1237, px_call(px_get_global("len"), (LXValue[]){_v1232}, 1)))) {
        LXValue _v1238 = px_index(_v1232, _v1237);
        if (px_is_truthy(px_eq(_v1238, px_str("\\")))) {
             _v1236 = px_add(_v1236, px_str("\\\\"));
        }
        else if (px_is_truthy(px_eq(_v1238, px_str("\"")))) {
             _v1236 = px_add(_v1236, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v1238, px_str("\n")))) {
             _v1236 = px_add(_v1236, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v1238, px_str("\r")))) {
             _v1236 = px_add(_v1236, px_str("\\r"));
        }
        else if (px_is_truthy(px_eq(_v1238, px_str("\t")))) {
             _v1236 = px_add(_v1236, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v1238, px_str("")))) {
             _v1236 = px_add(_v1236, px_str(""));
        }
        else {
             _v1236 = px_add(_v1236, _v1238);
        }
         _v1237 = px_add(_v1237, px_int(1LL));
    }
    return _v1236;
px_err_1235:
    if (px_err_1235_proped) return px_err_1235_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1239 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1240 = px_null();
    LXValue _v1241 = px_null();
    LXValue px_err_1242_val = px_null();
    int px_err_1242_proped = 0;
    LXValue _v1243 = px_str("");
    LXValue _v1244 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1244, _v1239))) {
         _v1243 = px_add(_v1243, px_str("0"));
         _v1244 = px_add(_v1244, px_int(1LL));
    }
    return _v1243;
px_err_1242:
    if (px_err_1242_proped) return px_err_1242_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1245 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1246 = px_null();
    LXValue _v1247 = px_null();
    LXValue _v1248 = px_null();
    LXValue _v1249 = px_null();
    LXValue _v1250 = px_null();
    LXValue _v1251 = px_null();
    LXValue _v1252 = px_null();
    LXValue _v1253 = px_null();
    LXValue _v1254 = px_null();
    LXValue _v1255 = px_null();
    LXValue _v1256 = px_null();
    LXValue px_err_1257_val = px_null();
    int px_err_1257_proped = 0;
    LXValue _v1258 = px_neg(px_int(1LL));
    LXValue _v1259 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1259, px_call(px_get_global("len"), (LXValue[]){_v1245}, 1)))) {
        if (px_is_truthy(({ LXValue _t1260 = px_eq(px_index(_v1245, _v1259), px_str("e")); px_is_truthy(_t1260) ? _t1260 : px_eq(px_index(_v1245, _v1259), px_str("E")); }))) {
             _v1258 = _v1259;
            break;
        }
         _v1259 = px_add(_v1259, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1258, px_int(0LL)))) {
        return _v1245;
    }
    LXValue _v1261 = px_slice(_v1245, px_int(0LL), _v1258, px_null());
    LXValue _v1262 = px_slice(_v1245, px_add(_v1258, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1245}, 1), px_null());
    LXValue _v1263 = px_int(1LL);
    if (px_is_truthy(({ LXValue _t1264 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1262}, 1), px_int(0LL)); px_is_truthy(_t1264) ? px_eq(px_index(_v1262, px_int(0LL)), px_str("+")) : _t1264; }))) {
         _v1262 = px_slice(_v1262, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1262}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1265 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1262}, 1), px_int(0LL)); px_is_truthy(_t1265) ? px_eq(px_index(_v1262, px_int(0LL)), px_str("-")) : _t1265; }))) {
         _v1263 = px_neg(px_int(1LL));
         _v1262 = px_slice(_v1262, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1262}, 1), px_null());
    }
    LXValue _v1266 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1262}, 1), _v1263);
    LXValue _v1267 = px_bool(false);
    if (px_is_truthy(({ LXValue _t1268 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1261}, 1), px_int(0LL)); px_is_truthy(_t1268) ? px_eq(px_index(_v1261, px_int(0LL)), px_str("-")) : _t1268; }))) {
         _v1267 = px_bool(true);
         _v1261 = px_slice(_v1261, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1261}, 1), px_null());
    }
    LXValue _v1269 = px_str("");
    LXValue _v1270 = px_str("");
    LXValue _v1271 = px_neg(px_int(1LL));
    LXValue _v1272 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1272, px_call(px_get_global("len"), (LXValue[]){_v1261}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v1261, _v1272), px_str(".")))) {
             _v1271 = _v1272;
            break;
        }
         _v1272 = px_add(_v1272, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1271, px_int(0LL)))) {
         _v1269 = _v1261;
    }
    else {
         _v1269 = px_slice(_v1261, px_int(0LL), _v1271, px_null());
         _v1270 = px_slice(_v1261, px_add(_v1271, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1261}, 1), px_null());
    }
    LXValue _v1273 = px_add(_v1269, _v1270);
    LXValue _v1274 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1269}, 1), _v1266);
    LXValue _v1275 = px_str("");
    if (px_is_truthy(px_le(_v1274, px_int(0LL)))) {
         _v1275 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1274)}, 1)), _v1273);
    }
    else if (px_is_truthy(px_ge(_v1274, px_call(px_get_global("len"), (LXValue[]){_v1273}, 1)))) {
         _v1275 = px_add(_v1273, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1274, px_call(px_get_global("len"), (LXValue[]){_v1273}, 1))}, 1));
    }
    else {
         _v1275 = px_add(px_add(px_slice(_v1273, px_int(0LL), _v1274, px_null()), px_str(".")), px_slice(_v1273, _v1274, px_call(px_get_global("len"), (LXValue[]){_v1273}, 1), px_null()));
    }
    if (px_is_truthy(_v1267)) {
        return px_add(px_str("-"), _v1275);
    }
    return _v1275;
px_err_1257:
    if (px_err_1257_proped) return px_err_1257_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1276 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1277 = px_null();
    LXValue px_err_1278_val = px_null();
    int px_err_1278_proped = 0;
    LXValue _v1279 = px_call(px_get_global("str"), (LXValue[]){_v1276}, 1);
    if (px_is_truthy(({ LXValue _t1281 = ({ LXValue _t1280 = px_eq(_v1279, px_str("inf")); px_is_truthy(_t1280) ? _t1280 : px_eq(_v1279, px_str("-inf")); }); px_is_truthy(_t1281) ? _t1281 : px_eq(_v1279, px_str("nan")); }))) {
        return _v1279;
    }
     _v1279 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1279}, 1);
    LXValue _v1282 = px_call(px_get_global("len"), (LXValue[]){_v1279}, 1);
    if (px_is_truthy(({ LXValue _t1283 = px_ge(_v1282, px_int(2LL)); px_is_truthy(_t1283) ? px_eq(px_slice(_v1279, px_sub(_v1282, px_int(2LL)), _v1282, px_null()), px_str(".0")) : _t1283; }))) {
        return px_slice(_v1279, px_int(0LL), px_sub(_v1282, px_int(2LL)), px_null());
    }
    return _v1279;
px_err_1278:
    if (px_err_1278_proped) return px_err_1278_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1284 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1285 = px_null();
    LXValue _v1286 = px_null();
    LXValue _v1287 = px_null();
    LXValue _v1288 = px_null();
    LXValue px_err_1289_val = px_null();
    int px_err_1289_proped = 0;
    LXValue _v1290 = px_index(_v1284, px_int(1LL));
    LXValue _v1291 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1291, px_call(px_get_global("len"), (LXValue[]){_v1290}, 1)))) {
        LXValue _v1292 = px_index(_v1290, _v1291);
        LXValue _v1293 = px_index(_v1292, px_int(0LL));
        if (px_is_truthy(px_eq(_v1293, px_str("StructDef")))) {
            LXValue _v1294 = px_list_n((LXValue[]){}, 0);
            LXValue _v1295 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1295, px_call(px_get_global("len"), (LXValue[]){px_index(_v1292, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1294, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1292, px_int(2LL)), _v1295), px_int(1LL))}, 1)}, 1));
                 _v1295 = px_add(_v1295, px_int(1LL));
            }
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1292, px_int(1LL))}, 1), _v1294);
        }
        else if (px_is_truthy(px_eq(_v1293, px_str("EnumDef")))) {
            LXValue _v1296 = px_list_n((LXValue[]){}, 0);
            LXValue _v1297 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1297, px_call(px_get_global("len"), (LXValue[]){px_index(_v1292, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1296, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1292, px_int(2LL)), _v1297), px_int(1LL))}, 1)}, 1));
                 _v1297 = px_add(_v1297, px_int(1LL));
            }
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1292, px_int(1LL))}, 1), _v1296);
        }
        else if (px_is_truthy(px_eq(_v1293, px_str("ImplDef")))) {
            LXValue _v1298 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1292, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1298}, 1))) {
                LXValue _v1299 = px_int(0LL);
                while (px_is_truthy(px_lt(_v1299, px_call(px_get_global("len"), (LXValue[]){px_index(_v1292, px_int(3LL))}, 1)))) {
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1298), "append", (LXValue[]){px_index(px_index(_v1292, px_int(3LL)), _v1299)}, 1));
                     _v1299 = px_add(_v1299, px_int(1LL));
                }
            }
            else {
                px_index_set(px_get_global("cg_impls"), _v1298, px_index(_v1292, px_int(3LL)));
            }
        }
         _v1291 = px_add(_v1291, px_int(1LL));
    }
px_err_1289:
    if (px_err_1289_proped) return px_err_1289_val;
    return px_null();
}

static LXValue fn_cg_collect_assign_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1300 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1301 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1302 = px_null();
    LXValue _v1303 = px_null();
    LXValue px_err_1304_val = px_null();
    int px_err_1304_proped = 0;
    LXValue _v1305 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1305, px_call(px_get_global("len"), (LXValue[]){_v1300}, 1)))) {
        LXValue _v1306 = px_index(_v1300, _v1305);
        LXValue _v1307 = px_index(_v1306, px_int(0LL));
        if (px_is_truthy(px_eq(_v1307, px_str("Assign")))) {
            LXValue _v1308 = px_index(_v1306, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1308, px_int(0LL)), px_str("Var")))) {
                LXValue _v1309 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1308, px_int(1LL))}, 1);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1301, _v1309}, 2)))) {
                    (void)(px_method(_v1301, "append", (LXValue[]){_v1309}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("If")))) {
            LXValue _v1310 = px_index(_v1306, px_int(1LL));
            LXValue _v1311 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1311, px_call(px_get_global("len"), (LXValue[]){_v1310}, 1)))) {
                (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(px_index(_v1310, _v1311), px_int(1LL)), _v1301}, 2));
                 _v1311 = px_add(_v1311, px_int(1LL));
            }
            if (px_is_truthy(px_ne(px_index(_v1306, px_int(2LL)), px_null()))) {
                (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1306, px_int(2LL)), _v1301}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("For")))) {
            (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1306, px_int(3LL)), _v1301}, 2));
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("While")))) {
            (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1306, px_int(2LL)), _v1301}, 2));
        }
         _v1305 = px_add(_v1305, px_int(1LL));
    }
px_err_1304:
    if (px_err_1304_proped) return px_err_1304_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1312 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1313_val = px_null();
    int px_err_1313_proped = 0;
    LXValue _v1314 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1312, px_int(1LL))}, 1)}, 1));
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1312, _v1314}, 2);
px_err_1313:
    if (px_err_1313_proped) return px_err_1313_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1315 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1316 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1317 = px_null();
    LXValue _v1318 = px_null();
    LXValue _v1319 = px_null();
    LXValue _v1320 = px_null();
    LXValue _v1321 = px_null();
    LXValue px_err_1322_val = px_null();
    int px_err_1322_proped = 0;
    LXValue _v1323 = px_add(px_add(px_str("static LXValue "), _v1316), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1323 = px_add(_v1323, px_str("    (void)ctx;\n"));
    LXValue _v1324 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v1325 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1326 = px_index(_v1315, px_int(2LL));
    LXValue _v1327 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1327, px_call(px_get_global("len"), (LXValue[]){_v1326}, 1)))) {
        LXValue _v1328 = px_index(_v1326, _v1327);
        LXValue _v1329 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1328, px_int(1LL))}, 1)}, 1);
        LXValue _v1330 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v1328, px_int(3LL)), px_null()))) {
             _v1330 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1328, px_int(3LL))}, 1);
        }
         _v1323 = px_add(_v1323, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1329), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1327}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1327}, 1)), px_str("] : ")), _v1330), px_str(";\n")));
         _v1327 = px_add(_v1327, px_int(1LL));
    }
    LXValue _v1331 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1315, px_int(4LL)), _v1331}, 2));
    LXValue _v1332 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1332, px_call(px_get_global("len"), (LXValue[]){_v1331}, 1)))) {
        LXValue _v1333 = px_index(_v1331, _v1332);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1333}, 1), px_null()))) {
             _v1332 = px_add(_v1332, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1333}, 2))) {
             _v1332 = px_add(_v1332, px_int(1LL));
            continue;
        }
        LXValue _v1334 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1333}, 1);
         _v1323 = px_add(_v1323, px_add(px_add(px_str("    LXValue "), _v1334), px_str(" = px_null();\n")));
         _v1332 = px_add(_v1332, px_int(1LL));
    }
    LXValue _v1335 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1335}, 1));
     _v1323 = px_add(_v1323, px_add(px_add(px_str("    LXValue "), _v1335), px_str("_val = px_null();\n")));
     _v1323 = px_add(_v1323, px_add(px_add(px_str("    int "), _v1335), px_str("_proped = 0;\n")));
    LXValue _v1336 = px_index(_v1315, px_int(4LL));
    LXValue _v1337 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1337, px_call(px_get_global("len"), (LXValue[]){_v1336}, 1)))) {
         _v1323 = px_add(_v1323, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1336, _v1337), px_int(1LL)}, 2));
         _v1337 = px_add(_v1337, px_int(1LL));
    }
     _v1323 = px_add(_v1323, px_add(_v1335, px_str(":\n")));
     _v1323 = px_add(_v1323, px_add(px_add(px_add(px_add(px_str("    if ("), _v1335), px_str("_proped) return ")), _v1335), px_str("_val;\n")));
     _v1323 = px_add(_v1323, px_str("    return px_null();\n"));
     _v1323 = px_add(_v1323, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_vars", _v1324);
    px_set_global("cg_var_types", _v1325);
    return _v1323;
px_err_1322:
    if (px_err_1322_proped) return px_err_1322_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1338 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1339 = px_null();
    LXValue _v1340 = px_null();
    LXValue _v1341 = px_null();
    LXValue _v1342 = px_null();
    LXValue _v1343 = px_null();
    LXValue _v1344 = px_null();
    LXValue _v1345 = px_null();
    LXValue _v1346 = px_null();
    LXValue _v1347 = px_null();
    LXValue _v1348 = px_null();
    LXValue _v1349 = px_null();
    LXValue _v1350 = px_null();
    LXValue _v1351 = px_null();
    LXValue _v1352 = px_null();
    LXValue px_err_1353_val = px_null();
    int px_err_1353_proped = 0;
    LXValue _v1354 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_set_global("cg_closures", px_str(""));
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_uidc", px_int(0LL));
    px_set_global("cg_closure_id", px_int(0LL));
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1338}, 1));
    LXValue _v1355 = px_index(_v1338, px_int(1LL));
    LXValue _v1356 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1356, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        LXValue _v1357 = px_index(_v1355, _v1356);
        LXValue _v1358 = px_index(_v1357, px_int(0LL));
        if (px_is_truthy(px_eq(_v1358, px_str("FuncDef")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1357, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("VarDecl")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("Assign")))) {
            LXValue _v1359 = px_index(_v1357, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1359, px_int(0LL)), px_str("Var")))) {
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1359, px_int(1LL))}, 1)}, 1));
            }
        }
         _v1356 = px_add(_v1356, px_int(1LL));
    }
    LXValue _v1360 = px_list_n((LXValue[]){}, 0);
    LXValue _v1361 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    LXValue _v1362 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1362, px_call(px_get_global("len"), (LXValue[]){_v1361}, 1)))) {
        LXValue _v1363 = px_index(_v1361, _v1362);
        LXValue _v1364 = px_index(px_get_global("cg_impls"), _v1363);
        LXValue _v1365 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1365, px_call(px_get_global("len"), (LXValue[]){_v1364}, 1)))) {
            (void)(px_method(_v1360, "append", (LXValue[]){px_list_n((LXValue[]){_v1363, px_index(_v1364, _v1365)}, 2)}, 1));
             _v1365 = px_add(_v1365, px_int(1LL));
        }
         _v1362 = px_add(_v1362, px_int(1LL));
    }
    LXValue _v1366 = px_int(1LL);
    while (px_is_truthy(px_lt(_v1366, px_call(px_get_global("len"), (LXValue[]){_v1360}, 1)))) {
        LXValue _v1367 = _v1366;
        while (px_is_truthy(px_gt(_v1367, px_int(0LL)))) {
            LXValue _v1368 = px_add(px_add(px_index(px_index(_v1360, px_sub(_v1367, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1360, px_sub(_v1367, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            LXValue _v1369 = px_add(px_add(px_index(px_index(_v1360, _v1367), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1360, _v1367), px_int(1LL)), px_int(1LL))}, 1));
            if (px_is_truthy(px_lt(_v1369, _v1368))) {
                LXValue _v1370 = px_index(_v1360, _v1367);
                px_index_set(_v1360, _v1367, px_index(_v1360, px_sub(_v1367, px_int(1LL))));
                px_index_set(_v1360, px_sub(_v1367, px_int(1LL)), _v1370);
            }
             _v1367 = px_sub(_v1367, px_int(1LL));
        }
         _v1366 = px_add(_v1366, px_int(1LL));
    }
    LXValue _v1371 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1371, px_call(px_get_global("len"), (LXValue[]){_v1360}, 1)))) {
        LXValue _v1372 = px_index(px_index(_v1360, _v1371), px_int(0LL));
        LXValue _v1373 = px_index(px_index(_v1360, _v1371), px_int(1LL));
        LXValue _v1374 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1372}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1373, px_int(1LL))}, 1)}, 1));
         _v1354 = px_add(_v1354, px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1373, _v1374}, 2));
         _v1354 = px_add(_v1354, px_str("\n"));
         _v1371 = px_add(_v1371, px_int(1LL));
    }
    LXValue _v1375 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1375, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        LXValue _v1376 = px_index(_v1355, _v1375);
        if (px_is_truthy(px_eq(px_index(_v1376, px_int(0LL)), px_str("FuncDef")))) {
             _v1354 = px_add(_v1354, px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1376}, 1));
             _v1354 = px_add(_v1354, px_str("\n"));
        }
         _v1375 = px_add(_v1375, px_int(1LL));
    }
     _v1354 = px_add(_v1354, px_str("int main(int argc, char** argv) {\n"));
     _v1354 = px_add(_v1354, px_str("    px_args_init(argc, argv);\n"));
     _v1354 = px_add(_v1354, px_str("    px_register_builtins();\n"));
    LXValue _v1377 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1377, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        LXValue _v1378 = px_index(_v1355, _v1377);
        if (px_is_truthy(px_eq(px_index(_v1378, px_int(0LL)), px_str("FuncDef")))) {
            LXValue _v1379 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1378, px_int(1LL))}, 1);
            LXValue _v1380 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1379}, 1));
             _v1354 = px_add(_v1354, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1379), px_str("\", px_func(\"")), _v1379), px_str("\", ")), _v1380), px_str(", NULL));\n")));
        }
         _v1377 = px_add(_v1377, px_int(1LL));
    }
    LXValue _v1381 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1381, px_call(px_get_global("len"), (LXValue[]){_v1360}, 1)))) {
        LXValue _v1382 = px_index(px_index(_v1360, _v1381), px_int(0LL));
        LXValue _v1383 = px_index(px_index(_v1360, _v1381), px_int(1LL));
        LXValue _v1384 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1383, px_int(1LL))}, 1);
        LXValue _v1385 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1382}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1384}, 1));
         _v1354 = px_add(_v1354, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1382), px_str(".")), _v1384), px_str("\", px_func(\"")), _v1382), px_str(".")), _v1384), px_str("\", ")), _v1385), px_str(", NULL));\n")));
         _v1381 = px_add(_v1381, px_int(1LL));
    }
    LXValue _v1386 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1386, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        LXValue _v1387 = px_index(_v1355, _v1386);
        LXValue _v1388 = px_index(_v1387, px_int(0LL));
        if (px_is_truthy(({ LXValue _t1393 = ({ LXValue _t1392 = ({ LXValue _t1391 = ({ LXValue _t1390 = ({ LXValue _t1389 = px_ne(_v1388, px_str("FuncDef")); px_is_truthy(_t1389) ? px_ne(_v1388, px_str("StructDef")) : _t1389; }); px_is_truthy(_t1390) ? px_ne(_v1388, px_str("EnumDef")) : _t1390; }); px_is_truthy(_t1391) ? px_ne(_v1388, px_str("TraitDef")) : _t1391; }); px_is_truthy(_t1392) ? px_ne(_v1388, px_str("ImplDef")) : _t1392; }); px_is_truthy(_t1393) ? px_ne(_v1388, px_str("Import")) : _t1393; }))) {
             _v1354 = px_add(_v1354, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1387, px_int(1LL)}, 2));
        }
         _v1386 = px_add(_v1386, px_int(1LL));
    }
    LXValue _v1394 = px_bool(false);
    LXValue _v1395 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1395, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        LXValue _v1396 = px_index(_v1355, _v1395);
        if (px_is_truthy(({ LXValue _t1397 = px_eq(px_index(_v1396, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1397) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1396, px_int(1LL))}, 1), px_str("main")) : _t1397; }))) {
             _v1394 = px_bool(true);
            break;
        }
         _v1395 = px_add(_v1395, px_int(1LL));
    }
    if (px_is_truthy(_v1394)) {
        LXValue _v1398 = px_str("fn_main");
         _v1354 = px_add(_v1354, px_add(px_add(px_str("    { LXValue _r = "), _v1398), px_str("(NULL, 0, NULL); int _code = 0;\n")));
         _v1354 = px_add(_v1354, px_str("      if (px_is_result(_r)) {\n"));
         _v1354 = px_add(_v1354, px_str("        if (!px_result_ok(_r)) {\n"));
         _v1354 = px_add(_v1354, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
         _v1354 = px_add(_v1354, px_str("          _code = 1;\n"));
         _v1354 = px_add(_v1354, px_str("        } else {\n"));
         _v1354 = px_add(_v1354, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
         _v1354 = px_add(_v1354, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
         _v1354 = px_add(_v1354, px_str("        }\n"));
         _v1354 = px_add(_v1354, px_str("      } else if (_r.type == PX_INT) {\n"));
         _v1354 = px_add(_v1354, px_str("        _code = (int)_r.as.i;\n"));
         _v1354 = px_add(_v1354, px_str("      }\n"));
         _v1354 = px_add(_v1354, px_str("      return _code;\n"));
         _v1354 = px_add(_v1354, px_str("    }\n"));
    }
    else {
         _v1354 = px_add(_v1354, px_str("    return 0;\n"));
    }
     _v1354 = px_add(_v1354, px_str("}\n"));
    LXValue _v1399 = px_call(px_get_global("cg_find"), (LXValue[]){_v1354, px_str("int main(")}, 2);
    if (px_is_truthy(px_ge(_v1399, px_int(0LL)))) {
        LXValue _v1400 = px_slice(_v1354, px_int(0LL), _v1399, px_null());
        LXValue _v1401 = px_slice(_v1354, _v1399, px_call(px_get_global("len"), (LXValue[]){_v1354}, 1), px_null());
        LXValue _v1402 = px_call(px_get_global("cg_find"), (LXValue[]){_v1400, px_str("static LXValue")}, 2);
        LXValue _v1403 = px_str("");
        if (px_is_truthy(px_ge(_v1402, px_int(0LL)))) {
             _v1403 = px_add(px_add(px_add(px_add(px_slice(_v1400, px_int(0LL), _v1402, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1400, _v1402, px_call(px_get_global("len"), (LXValue[]){_v1400}, 1), px_null())), _v1401);
        }
        else {
             _v1403 = px_add(px_add(px_add(_v1400, px_get_global("cg_closures")), px_str("\n")), _v1401);
        }
        return _v1403;
    }
    return _v1354;
px_err_1353:
    if (px_err_1353_proped) return px_err_1353_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1404 = px_null();
    LXValue px_err_1405_val = px_null();
    int px_err_1405_proped = 0;
    LXValue _v1406 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    LXValue _v1407 = px_index(_v1406, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1406}, 1), px_int(1LL)));
    LXValue _v1408 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1407}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v1407}, 1)}, 1));
    px_set_global("p_pos", px_int(0LL));
    LXValue _v1409 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    LXValue _v1410 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v1409, _v1408}, 2);
    LXValue _v1411 = px_call(px_get_global("cg_generate"), (LXValue[]){_v1410}, 1);
    LXValue _v1412 = px_call(px_get_global("len"), (LXValue[]){_v1411}, 1);
    if (px_is_truthy(({ LXValue _t1413 = px_gt(_v1412, px_int(0LL)); px_is_truthy(_t1413) ? px_eq(px_index(_v1411, px_sub(_v1412, px_int(1LL))), px_str("\n")) : _t1413; }))) {
         _v1411 = px_slice(_v1411, px_int(0LL), px_sub(_v1412, px_int(1LL)), px_null());
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1411}, 1));
px_err_1405:
    if (px_err_1405_proped) return px_err_1405_val;
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
    px_set_global("parse_func_def", px_func("parse_func_def", fn_parse_func_def, NULL));
    px_set_global("parse_struct_def", px_func("parse_struct_def", fn_parse_struct_def, NULL));
    px_set_global("parse_enum_def", px_func("parse_enum_def", fn_parse_enum_def, NULL));
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
    px_set_global("cg_func_cname", px_func("cg_func_cname", fn_cg_func_cname, NULL));
    px_set_global("cg_find", px_func("cg_find", fn_cg_find, NULL));
    px_set_global("cg_pad", px_func("cg_pad", fn_cg_pad, NULL));
    px_set_global("rust_unescape", px_func("rust_unescape", fn_rust_unescape, NULL));
    px_set_global("cg_escape_str", px_func("cg_escape_str", fn_cg_escape_str, NULL));
    px_set_global("cg_pad_zeros", px_func("cg_pad_zeros", fn_cg_pad_zeros, NULL));
    px_set_global("cg_expand_sci", px_func("cg_expand_sci", fn_cg_expand_sci, NULL));
    px_set_global("cg_fmt_float", px_func("cg_fmt_float", fn_cg_fmt_float, NULL));
    px_set_global("cg_collect_types", px_func("cg_collect_types", fn_cg_collect_types, NULL));
    px_set_global("cg_collect_assign_vars", px_func("cg_collect_assign_vars", fn_cg_collect_assign_vars, NULL));
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
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } _d; }));
    px_set_global("CTRL_ALL", px_str(""));
    px_set_global("p_toks", px_list_n((LXValue[]){}, 0));
    px_set_global("p_pos", px_int(0LL));
    px_set_global("cg_closures", px_str(""));
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_uidc", px_int(0LL));
    px_set_global("cg_closure_id", px_int(0LL));
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
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
