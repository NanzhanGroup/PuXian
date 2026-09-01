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

static LXValue fn_check_edition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v238 = (nargs > 0) ? args[0] : px_null();
    LXValue _v239 = px_null();
    LXValue _v240 = px_null();
    LXValue px_err_241_val = px_null();
    int px_err_241_proped = 0;
    LXValue _v242 = px_int(0LL);
    LXValue _v243 = px_call(px_get_global("len"), (LXValue[]){_v238}, 1);
    LXValue _v244 = px_str("");
    while (px_is_truthy(({ LXValue _t245 = px_lt(_v242, _v243); px_is_truthy(_t245) ? px_ne(px_index(_v238, _v242), px_str("\n")) : _t245; }))) {
         _v244 = px_add(_v244, px_index(_v238, _v242));
         _v242 = px_add(_v242, px_int(1LL));
    }
    LXValue _v246 = px_call(px_get_global("trim"), (LXValue[]){_v244}, 1);
    LXValue _v247 = px_method(_v246, "split", (LXValue[]){px_str(" ")}, 1);
    if (px_is_truthy(({ LXValue _t249 = ({ LXValue _t248 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v247}, 1), px_int(3LL)); px_is_truthy(_t248) ? px_eq(px_index(_v247, px_int(0LL)), px_str("#")) : _t248; }); px_is_truthy(_t249) ? px_eq(px_index(_v247, px_int(1LL)), px_str("px")) : _t249; }))) {
        LXValue _v250 = px_index(_v247, px_int(2LL));
        if (px_is_truthy(({ LXValue _t254 = ({ LXValue _t253 = ({ LXValue _t252 = ({ LXValue _t251 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v250}, 1), px_int(4LL)); px_is_truthy(_t251) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v250, px_int(0LL))}, 1) : _t251; }); px_is_truthy(_t252) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v250, px_int(1LL))}, 1) : _t252; }); px_is_truthy(_t253) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v250, px_int(2LL))}, 1) : _t253; }); px_is_truthy(_t254) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v250, px_int(3LL))}, 1) : _t254; }))) {
            LXValue _v255 = px_call(px_get_global("int"), (LXValue[]){_v250}, 1);
            if (px_is_truthy(px_gt(_v255, px_int(2026LL)))) {
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E-EDITION"), px_add(px_add(px_str("源码声明 edition px "), _v250), px_str(" 高于当前工具链支持（px 2026），请升级编译器"))}, 2));
            }
        }
    }
px_err_241:
    if (px_err_241_proped) return px_err_241_val;
    return px_null();
}

static LXValue fn_lex_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v256 = (nargs > 0) ? args[0] : px_null();
    LXValue _v257 = px_null();
    LXValue px_err_258_val = px_null();
    int px_err_258_proped = 0;
    (void)(px_call(px_get_global("check_edition"), (LXValue[]){_v256}, 1));
    px_set_global("g_src", _v256);
    px_set_global("g_len", px_call(px_get_global("len"), (LXValue[]){_v256}, 1));
    px_set_global("g_pos", px_int(0LL));
    px_set_global("g_line", px_int(1LL));
    px_set_global("g_col", px_int(1LL));
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_set_global("g_at_line_start", px_bool(true));
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_set_global("g_count", px_int(0LL));
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    LXValue _v259 = px_bool(true);
    while (px_is_truthy(_v259)) {
         _v259 = px_call(px_get_global("next_token"), (LXValue[]){}, 0);
    }
    return px_get_global("g_toks");
px_err_258:
    if (px_err_258_proped) return px_err_258_val;
    return px_null();
}

static LXValue fn_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v260 = (nargs > 0) ? args[0] : px_null();
    LXValue _v261 = px_null();
    LXValue _v262 = px_null();
    LXValue px_err_263_val = px_null();
    int px_err_263_proped = 0;
    LXValue _v264 = px_str("");
    LXValue _v265 = px_int(0LL);
    while (px_is_truthy(px_lt(_v265, _v260))) {
         _v264 = px_add(_v264, px_str(" "));
         _v265 = px_add(_v265, px_int(1LL));
    }
    return _v264;
px_err_263:
    if (px_err_263_proped) return px_err_263_val;
    return px_null();
}

static LXValue fn_dump_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v266 = (nargs > 0) ? args[0] : px_null();
    LXValue _v267 = (nargs > 1) ? args[1] : px_null();
    LXValue _v268 = px_null();
    LXValue px_err_269_val = px_null();
    int px_err_269_proped = 0;
    LXValue _v270 = px_index(_v266, px_int(0LL));
    LXValue _v271 = px_index(px_get_global("LAYOUT"), _v270);
    LXValue _v272 = px_index(_v271, px_int(0LL));
    LXValue _v273 = px_index(_v271, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v273}, 1), px_int(0LL)))) {
        return _v272;
    }
    LXValue _v274 = px_eq(px_index(px_index(_v273, px_int(0LL)), px_int(0LL)), px_null());
    LXValue _v275 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(_v274)) {
        (void)(px_method(_v275, "append", (LXValue[]){px_add(_v272, px_str("("))}, 1));
    }
    else {
        (void)(px_method(_v275, "append", (LXValue[]){px_add(_v272, px_str(" {"))}, 1));
    }
    LXValue _v276 = px_int(0LL);
    while (px_is_truthy(px_lt(_v276, px_call(px_get_global("len"), (LXValue[]){_v273}, 1)))) {
        LXValue _v277 = px_index(_v273, _v276);
        LXValue _v278 = px_index(_v266, px_add(_v276, px_int(1LL)));
        LXValue _v279 = px_call(px_get_global("dump_field"), (LXValue[]){_v278, px_index(_v277, px_int(1LL)), px_add(_v267, px_int(4LL))}, 3);
        if (px_is_truthy(_v274)) {
            (void)(px_method(_v275, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v267, px_int(4LL))}, 1), _v279), px_str(","))}, 1));
        }
        else {
            (void)(px_method(_v275, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v267, px_int(4LL))}, 1), px_index(_v277, px_int(0LL))), px_str(": ")), _v279), px_str(","))}, 1));
        }
         _v276 = px_add(_v276, px_int(1LL));
    }
    if (px_is_truthy(_v274)) {
        (void)(px_method(_v275, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v267}, 1), px_str(")"))}, 1));
    }
    else {
        (void)(px_method(_v275, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v267}, 1), px_str("}"))}, 1));
    }
    return px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v275}, 2);
px_err_269:
    if (px_err_269_proped) return px_err_269_val;
    return px_null();
}

static LXValue fn_dump_list(LXValue* args, int nargs, void* ctx) {
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

static LXValue fn_dump_str_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v286 = (nargs > 0) ? args[0] : px_null();
    LXValue _v287 = (nargs > 1) ? args[1] : px_null();
    LXValue _v288 = px_null();
    LXValue px_err_289_val = px_null();
    int px_err_289_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v286}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v290 = px_list_n((LXValue[]){}, 0);
    LXValue _v291 = px_int(0LL);
    while (px_is_truthy(px_lt(_v291, px_call(px_get_global("len"), (LXValue[]){_v286}, 1)))) {
        (void)(px_method(_v290, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v287, px_int(4LL))}, 1), px_index(_v286, _v291)), px_str(","))}, 1));
         _v291 = px_add(_v291, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v290}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v287}, 1)), px_str("]"));
px_err_289:
    if (px_err_289_proped) return px_err_289_val;
    return px_null();
}

static LXValue fn_dump_ty_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v292 = (nargs > 0) ? args[0] : px_null();
    LXValue _v293 = (nargs > 1) ? args[1] : px_null();
    LXValue _v294 = px_null();
    LXValue px_err_295_val = px_null();
    int px_err_295_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v292}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v296 = px_list_n((LXValue[]){}, 0);
    LXValue _v297 = px_int(0LL);
    while (px_is_truthy(px_lt(_v297, px_call(px_get_global("len"), (LXValue[]){_v292}, 1)))) {
        (void)(px_method(_v296, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v293, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v292, _v297), px_add(_v293, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v297 = px_add(_v297, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v296}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v293}, 1)), px_str("]"));
px_err_295:
    if (px_err_295_proped) return px_err_295_val;
    return px_null();
}

static LXValue fn_dump_pat_list(LXValue* args, int nargs, void* ctx) {
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
        (void)(px_method(_v302, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v299, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v298, _v303), px_add(_v299, px_int(4LL))}, 2)), px_str(","))}, 1));
         _v303 = px_add(_v303, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v302}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v299}, 1)), px_str("]"));
px_err_301:
    if (px_err_301_proped) return px_err_301_val;
    return px_null();
}

static LXValue fn_dump_opt_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v304 = (nargs > 0) ? args[0] : px_null();
    LXValue _v305 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_306_val = px_null();
    int px_err_306_proped = 0;
    if (px_is_truthy(px_eq(_v304, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v305, px_int(4LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){_v304, px_add(_v305, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v305}, 1)), px_str(")"));
px_err_306:
    if (px_err_306_proped) return px_err_306_val;
    return px_null();
}

static LXValue fn_dump_opt_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v307 = (nargs > 0) ? args[0] : px_null();
    LXValue _v308 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_309_val = px_null();
    int px_err_309_proped = 0;
    if (px_is_truthy(px_eq(_v307, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v308, px_int(4LL))}, 1)), _v307), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v308}, 1)), px_str(")"));
px_err_309:
    if (px_err_309_proped) return px_err_309_val;
    return px_null();
}

static LXValue fn_dump_opt_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v310 = (nargs > 0) ? args[0] : px_null();
    LXValue _v311 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_312_val = px_null();
    int px_err_312_proped = 0;
    if (px_is_truthy(px_eq(_v310, px_null()))) {
        return px_str("None");
    }
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v311, px_int(4LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){_v310, px_add(_v311, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v311}, 1)), px_str(")"));
px_err_312:
    if (px_err_312_proped) return px_err_312_val;
    return px_null();
}

static LXValue fn_dump_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v313 = (nargs > 0) ? args[0] : px_null();
    LXValue _v314 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_315_val = px_null();
    int px_err_315_proped = 0;
    return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v314, px_int(4LL))}, 1)), px_str("line: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v313, px_int(0LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v314, px_int(4LL))}, 1)), px_str("col: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v313, px_int(1LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v314}, 1)), px_str("}"));
px_err_315:
    if (px_err_315_proped) return px_err_315_val;
    return px_null();
}

static LXValue fn_dump_t2_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v316 = (nargs > 0) ? args[0] : px_null();
    LXValue _v317 = (nargs > 1) ? args[1] : px_null();
    LXValue _v318 = px_null();
    LXValue px_err_319_val = px_null();
    int px_err_319_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v316}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v320 = px_list_n((LXValue[]){}, 0);
    LXValue _v321 = px_int(0LL);
    while (px_is_truthy(px_lt(_v321, px_call(px_get_global("len"), (LXValue[]){_v316}, 1)))) {
        LXValue _v322 = px_index(_v316, _v321);
        (void)(px_method(_v320, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v317, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v317, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v322, px_int(0LL)), px_add(_v317, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v317, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v322, px_int(1LL)), px_add(_v317, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v317, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v321 = px_add(_v321, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v320}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v317}, 1)), px_str("]"));
px_err_319:
    if (px_err_319_proped) return px_err_319_val;
    return px_null();
}

static LXValue fn_dump_t2b_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v323 = (nargs > 0) ? args[0] : px_null();
    LXValue _v324 = (nargs > 1) ? args[1] : px_null();
    LXValue _v325 = px_null();
    LXValue px_err_326_val = px_null();
    int px_err_326_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v323}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v327 = px_list_n((LXValue[]){}, 0);
    LXValue _v328 = px_int(0LL);
    while (px_is_truthy(px_lt(_v328, px_call(px_get_global("len"), (LXValue[]){_v323}, 1)))) {
        LXValue _v329 = px_index(_v323, _v328);
        (void)(px_method(_v327, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v324, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v324, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v329, px_int(0LL)), px_add(_v324, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v324, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v329, px_int(1LL)), px_add(_v324, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v324, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v328 = px_add(_v328, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v327}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v324}, 1)), px_str("]"));
px_err_326:
    if (px_err_326_proped) return px_err_326_val;
    return px_null();
}

static LXValue fn_dump_t3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v330 = (nargs > 0) ? args[0] : px_null();
    LXValue _v331 = (nargs > 1) ? args[1] : px_null();
    LXValue _v332 = px_null();
    LXValue _v333 = px_null();
    LXValue px_err_334_val = px_null();
    int px_err_334_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v330}, 1), px_int(0LL)))) {
        return px_str("[]");
    }
    LXValue _v335 = px_list_n((LXValue[]){}, 0);
    LXValue _v336 = px_int(0LL);
    while (px_is_truthy(px_lt(_v336, px_call(px_get_global("len"), (LXValue[]){_v330}, 1)))) {
        LXValue _v337 = px_index(_v330, _v336);
        LXValue _v338 = px_index(_v337, px_int(0LL));
        LXValue _v339 = px_str("None");
        if (px_is_truthy(px_ne(_v338, px_null()))) {
             _v339 = px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(12LL))}, 1)), _v338), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(8LL))}, 1)), px_str(")"));
        }
        (void)(px_method(_v335, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(8LL))}, 1)), _v339), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v337, px_int(1LL)), px_add(_v331, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v337, px_int(2LL)), px_add(_v331, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v331, px_int(4LL))}, 1)), px_str("),"))}, 1));
         _v336 = px_add(_v336, px_int(1LL));
    }
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v335}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v331}, 1)), px_str("]"));
px_err_334:
    if (px_err_334_proped) return px_err_334_val;
    return px_null();
}

static LXValue fn_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v340 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_341_val = px_null();
    int px_err_341_proped = 0;
    LXValue _v342 = px_call(px_get_global("str"), (LXValue[]){_v340}, 1);
    if (px_is_truthy(({ LXValue _t343 = px_eq(_v342, px_str("inf")); px_is_truthy(_t343) ? _t343 : px_eq(_v342, px_str("-inf")); }))) {
        return _v342;
    }
    if (px_is_truthy(({ LXValue _t345 = ({ LXValue _t344 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v342, px_str(".")}, 2)); px_is_truthy(_t344) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v342, px_str("e")}, 2)) : _t344; }); px_is_truthy(_t345) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v342, px_str("E")}, 2)) : _t345; }))) {
        return px_add(_v342, px_str(".0"));
    }
    return _v342;
px_err_341:
    if (px_err_341_proped) return px_err_341_val;
    return px_null();
}

static LXValue fn_dump_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v346 = (nargs > 0) ? args[0] : px_null();
    LXValue _v347 = (nargs > 1) ? args[1] : px_null();
    LXValue _v348 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_349_val = px_null();
    int px_err_349_proped = 0;
    if (px_is_truthy(px_eq(_v347, px_str("s")))) {
        return _v346;
    }
    if (px_is_truthy(px_eq(_v347, px_str("r")))) {
        return px_call(px_get_global("str"), (LXValue[]){_v346}, 1);
    }
    if (px_is_truthy(px_eq(_v347, px_str("f")))) {
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v346}, 1);
    }
    if (px_is_truthy(px_eq(_v347, px_str("n")))) {
        return px_call(px_get_global("dump_node"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("o")))) {
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("os")))) {
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("ol")))) {
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("l")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("ls")))) {
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("tl")))) {
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lpl")))) {
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lp")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lsf")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lev")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lfd")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lc")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lma")))) {
        return px_call(px_get_global("dump_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lt2")))) {
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lt2b")))) {
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("lt3")))) {
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v346, _v348}, 2);
    }
    if (px_is_truthy(px_eq(_v347, px_str("p")))) {
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v346, _v348}, 2);
    }
    return px_call(px_get_global("str"), (LXValue[]){_v346}, 1);
px_err_349:
    if (px_err_349_proped) return px_err_349_val;
    return px_null();
}

static LXValue fn_dump_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v350 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_351_val = px_null();
    int px_err_351_proped = 0;
    return px_call(px_get_global("dump_node"), (LXValue[]){_v350, px_int(0LL)}, 2);
px_err_351:
    if (px_err_351_proped) return px_err_351_val;
    return px_null();
}

static LXValue fn_pk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_352_val = px_null();
    int px_err_352_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
px_err_352:
    if (px_err_352_proped) return px_err_352_val;
    return px_null();
}

static LXValue fn_pk_display(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_353_val = px_null();
    int px_err_353_proped = 0;
    LXValue _v354 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
    LXValue _v355 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
    if (px_is_truthy(px_eq(_v354, px_str("整数")))) {
        return px_add(px_str("整数 "), _v355);
    }
    if (px_is_truthy(px_eq(_v354, px_str("浮点")))) {
        return px_add(px_str("浮点 "), _v355);
    }
    if (px_is_truthy(px_eq(_v354, px_str("字符串")))) {
        return px_add(px_str("字符串 "), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v355}, 1));
    }
    if (px_is_truthy(px_eq(_v354, px_str("标识符")))) {
        return px_add(px_str("标识符 "), _v355);
    }
    if (px_is_truthy(px_eq(_v354, px_str("注释")))) {
        return px_add(px_str("注释 "), _v355);
    }
    return _v354;
px_err_353:
    if (px_err_353_proped) return px_err_353_val;
    return px_null();
}

static LXValue fn_pv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_356_val = px_null();
    int px_err_356_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
px_err_356:
    if (px_err_356_proped) return px_err_356_val;
    return px_null();
}

static LXValue fn_pline(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_357_val = px_null();
    int px_err_357_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL));
px_err_357:
    if (px_err_357_proped) return px_err_357_val;
    return px_null();
}

static LXValue fn_pcol(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_358_val = px_null();
    int px_err_358_proped = 0;
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL));
px_err_358:
    if (px_err_358_proped) return px_err_358_val;
    return px_null();
}

static LXValue fn_ppos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_359_val = px_null();
    int px_err_359_proped = 0;
    return px_list_n((LXValue[]){px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL)), px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL))}, 2);
px_err_359:
    if (px_err_359_proped) return px_err_359_val;
    return px_null();
}

static LXValue fn_adv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_360_val = px_null();
    int px_err_360_proped = 0;
    LXValue _v361 = px_index(px_get_global("p_toks"), px_get_global("p_pos"));
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_set_global("p_pos", px_add(px_get_global("p_pos"), px_int(1LL)));
    }
    return _v361;
px_err_360:
    if (px_err_360_proped) return px_err_360_val;
    return px_null();
}

static LXValue fn_chk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v362 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_363_val = px_null();
    int px_err_363_proped = 0;
    return px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), _v362);
px_err_363:
    if (px_err_363_proped) return px_err_363_val;
    return px_null();
}

static LXValue fn_chk2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v364 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_365_val = px_null();
    int px_err_365_proped = 0;
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(1LL))), px_int(0LL)), _v364);
    }
    return px_bool(false);
px_err_365:
    if (px_err_365_proped) return px_err_365_val;
    return px_null();
}

static LXValue fn_expect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v366 = (nargs > 0) ? args[0] : px_null();
    LXValue _v367 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_368_val = px_null();
    int px_err_368_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v366}, 1))) {
        return px_call(px_get_global("adv"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望 "), _v367), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_368:
    if (px_err_368_proped) return px_err_368_val;
    return px_null();
}

static LXValue fn_expect_ident(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v369 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_370_val = px_null();
    int px_err_370_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        LXValue _v371 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v371;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v369), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_370:
    if (px_err_370_proped) return px_err_370_val;
    return px_null();
}

static LXValue fn_is_name_kind(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v372 = (nargs > 0) ? args[0] : px_null();
    LXValue _v373 = px_null();
    LXValue px_err_374_val = px_null();
    int px_err_374_proped = 0;
    LXValue _v375 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture"), px_str("extern")}, 36);
    LXValue _v376 = px_int(0LL);
    while (px_is_truthy(px_lt(_v376, px_call(px_get_global("len"), (LXValue[]){_v375}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v375, _v376), _v372))) {
            return px_bool(true);
        }
         _v376 = px_add(_v376, px_int(1LL));
    }
    return px_bool(false);
px_err_374:
    if (px_err_374_proped) return px_err_374_val;
    return px_null();
}

static LXValue fn_expect_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v377 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_378_val = px_null();
    int px_err_378_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        LXValue _v379 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v379;
    }
    if (px_is_truthy(px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1))) {
        LXValue _v380 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return _v380;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v377), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_378:
    if (px_err_378_proped) return px_err_378_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v381 = (nargs > 0) ? args[0] : px_null();
    LXValue _v382 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_383_val = px_null();
    int px_err_383_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1)), px_str(": 语法错误 ")), _v381), px_str(": ")), _v382)}, 1));
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("parse "), _v381)}, 1));
px_err_383:
    if (px_err_383_proped) return px_err_383_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_384_val = px_null();
    int px_err_384_proped = 0;
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_384:
    if (px_err_384_proped) return px_err_384_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_385_val = px_null();
    int px_err_385_proped = 0;
    while (px_is_truthy(({ LXValue _t386 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t386) ? _t386 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_385:
    if (px_err_385_proped) return px_err_385_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_387_val = px_null();
    int px_err_387_proped = 0;
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_387:
    if (px_err_387_proped) return px_err_387_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v388 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_389_val = px_null();
    int px_err_389_proped = 0;
    return px_index(_v388, px_sub(px_call(px_get_global("len"), (LXValue[]){_v388}, 1), px_int(1LL)));
px_err_389:
    if (px_err_389_proped) return px_err_389_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v390 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_391_val = px_null();
    int px_err_391_proped = 0;
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v390}, 1);
px_err_391:
    if (px_err_391_proped) return px_err_391_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_392_val = px_null();
    int px_err_392_proped = 0;
    LXValue _v393 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        (void)(px_method(_v393, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v393}, 2);
px_err_392:
    if (px_err_392_proped) return px_err_392_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_394_val = px_null();
    int px_err_394_proped = 0;
    LXValue _v395 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v395, px_str("let")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    if (px_is_truthy(px_eq(_v395, px_str("var")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    if (px_is_truthy(px_eq(_v395, px_str("const")))) {
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    if (px_is_truthy(px_eq(_v395, px_str("if")))) {
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("for")))) {
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("while")))) {
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("def")))) {
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("extern")))) {
        return px_call(px_get_global("parse_extern_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("struct")))) {
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("enum")))) {
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("trait")))) {
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("impl")))) {
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("import")))) {
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("from")))) {
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("return")))) {
        LXValue _v396 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t398 = ({ LXValue _t397 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t397) ? _t397 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t398) ? _t398 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v396}, 3);
        }
        LXValue _v399 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Return"), _v399, _v396}, 3);
    }
    if (px_is_truthy(px_eq(_v395, px_str("break")))) {
        LXValue _v400 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Break"), _v400}, 2);
    }
    if (px_is_truthy(px_eq(_v395, px_str("continue")))) {
        LXValue _v401 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Continue"), _v401}, 2);
    }
    if (px_is_truthy(px_eq(_v395, px_str("spawn")))) {
        LXValue _v402 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v403 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Spawn"), _v403, _v402}, 3);
    }
    if (px_is_truthy(px_eq(_v395, px_str("select")))) {
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v395, px_str("fn")))) {
        LXValue _v404 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        LXValue _v405 = px_call(px_get_global("node_pos"), (LXValue[]){_v404}, 1);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v404, _v405}, 3);
    }
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_394:
    if (px_err_394_proped) return px_err_394_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v406 = (nargs > 0) ? args[0] : px_null();
    LXValue _v407 = px_null();
    LXValue _v408 = px_null();
    LXValue px_err_409_val = px_null();
    int px_err_409_proped = 0;
    LXValue _v410 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(({ LXValue _t412 = ({ LXValue _t411 = px_eq(_v406, px_str("Let")); px_is_truthy(_t411) ? px_call(px_get_global("chk"), (LXValue[]){px_str("标识符")}, 1) : _t411; }); px_is_truthy(_t412) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("mut")) : _t412; }))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v406 = px_str("Mut");
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    LXValue _v413 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    LXValue _v414 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v414 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    LXValue _v415 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v415 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    return px_list_n((LXValue[]){px_str("VarDecl"), _v406, px_call(px_get_global("qstr"), (LXValue[]){_v413}, 1), _v414, _v415, _v410}, 6);
px_err_409:
    if (px_err_409_proped) return px_err_409_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v416 = px_null();
    LXValue px_err_417_val = px_null();
    int px_err_417_proped = 0;
    LXValue _v418 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    LXValue _v419 = px_call(px_get_global("node_pos"), (LXValue[]){_v418}, 1);
    LXValue _v420 = px_null();
    LXValue _v421 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v421, px_str("=")))) {
         _v420 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("+=")))) {
         _v420 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("-=")))) {
         _v420 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("*=")))) {
         _v420 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("/=")))) {
         _v420 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("//=")))) {
         _v420 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("%=")))) {
         _v420 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("**=")))) {
         _v420 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("&=")))) {
         _v420 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("|=")))) {
         _v420 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("^=")))) {
         _v420 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v421, px_str("<<=")))) {
         _v420 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v421, px_str(">>=")))) {
         _v420 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v421, px_str(">>>=")))) {
         _v420 = px_str("ShrU");
    }
    if (px_is_truthy(px_ne(_v420, px_null()))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v422 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Assign"), _v418, _v420, _v422, _v419}, 5);
    }
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v418, _v419}, 3);
px_err_417:
    if (px_err_417_proped) return px_err_417_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v423 = px_null();
    LXValue px_err_424_val = px_null();
    int px_err_424_proped = 0;
    LXValue _v425 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v426 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v427 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    LXValue _v428 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v426, _v427}, 2)}, 1);
    LXValue _v429 = px_null();
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v430 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            LXValue _v431 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            (void)(px_method(_v428, "append", (LXValue[]){px_list_n((LXValue[]){_v430, _v431}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
             _v429 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            break;
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){px_str("If"), _v428, _v429, _v425}, 4);
px_err_424:
    if (px_err_424_proped) return px_err_424_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_432_val = px_null();
    int px_err_432_proped = 0;
    LXValue _v433 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v434 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    LXValue _v435 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v436 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("For"), px_call(px_get_global("qstr"), (LXValue[]){_v434}, 1), _v435, _v436, _v433}, 5);
px_err_432:
    if (px_err_432_proped) return px_err_432_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_437_val = px_null();
    int px_err_437_proped = 0;
    LXValue _v438 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v439 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v440 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("While"), _v439, _v440, _v438}, 4);
px_err_437:
    if (px_err_437_proped) return px_err_437_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_441_val = px_null();
    int px_err_441_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v442 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t443 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t443) ? _t443 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_method(_v442, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return _v442;
px_err_441:
    if (px_err_441_proped) return px_err_441_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v444 = px_null();
    LXValue px_err_445_val = px_null();
    int px_err_445_proped = 0;
    LXValue _v446 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        return _v446;
    }
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    while (px_is_truthy(px_bool(true))) {
        LXValue _v447 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        LXValue _v448 = _v447;
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v449 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
             _v448 = px_add(px_add(_v447, px_str(": ")), _v449);
        }
        (void)(px_method(_v446, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v448}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    return _v446;
px_err_445:
    if (px_err_445_proped) return px_err_445_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v450 = px_null();
    LXValue px_err_451_val = px_null();
    int px_err_451_proped = 0;
    LXValue _v452 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v453 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    LXValue _v454 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    LXValue _v455 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v456 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v456 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v457 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v453}, 1), _v455, _v456, _v457, _v452, _v454}, 7);
px_err_451:
    if (px_err_451_proped) return px_err_451_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v458 = px_null();
    LXValue px_err_459_val = px_null();
    int px_err_459_proped = 0;
    LXValue _v460 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    LXValue _v461 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    LXValue _v462 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v463 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v463 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v461}, 1), _v462, _v463, _v460}, 5);
px_err_459:
    if (px_err_459_proped) return px_err_459_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_464_val = px_null();
    int px_err_464_proped = 0;
    LXValue _v465 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v466 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    LXValue _v467 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v468 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t469 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t469) ? _t469 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v470 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v471 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v472 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_method(_v468, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v471}, 1), _v472, _v470}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t473 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t473) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t473; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v466}, 1), _v468, _v465, _v467}, 5);
px_err_464:
    if (px_err_464_proped) return px_err_464_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_474_val = px_null();
    int px_err_474_proped = 0;
    LXValue _v475 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v476 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v477 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t478 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t478) ? _t478 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v479 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v480 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        LXValue _v481 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v481, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        (void)(px_method(_v477, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v480}, 1), _v481, _v479}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t482 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t482) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t482; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v476}, 1), _v477, _v475}, 4);
px_err_474:
    if (px_err_474_proped) return px_err_474_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v483 = px_null();
    LXValue _v484 = px_null();
    LXValue px_err_485_val = px_null();
    int px_err_485_proped = 0;
    LXValue _v486 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v487 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v488 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t489 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t489) ? _t489 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v490 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v491 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        LXValue _v492 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        LXValue _v493 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v493 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v494 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                 _v494 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        (void)(px_method(_v488, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v491}, 1), _v492, _v493, _v494, _v490, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        if (px_is_truthy(({ LXValue _t495 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t495) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t495; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v487}, 1), _v488, _v486}, 4);
px_err_485:
    if (px_err_485_proped) return px_err_485_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v496 = px_null();
    LXValue _v497 = px_null();
    LXValue _v498 = px_null();
    LXValue px_err_499_val = px_null();
    int px_err_499_proped = 0;
    LXValue _v500 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v501 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    LXValue _v502 = px_null();
    LXValue _v503 = _v501;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v503 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
         _v502 = px_call(px_get_global("qstr"), (LXValue[]){_v501}, 1);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v504 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t505 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t505) ? _t505 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v506 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v507 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        LXValue _v508 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        LXValue _v509 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v509 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v510 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        (void)(px_method(_v504, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v507}, 1), _v508, _v509, _v510, _v506, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v503}, 1), _v502, _v504, _v500}, 5);
px_err_499:
    if (px_err_499_proped) return px_err_499_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_511_val = px_null();
    int px_err_511_proped = 0;
    LXValue _v512 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        LXValue _v513 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v513}, 1), px_list_n((LXValue[]){}, 0), _v512}, 4);
    }
    LXValue _v514 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v514, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v514, px_list_n((LXValue[]){}, 0), _v512}, 4);
px_err_511:
    if (px_err_511_proped) return px_err_511_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_515_val = px_null();
    int px_err_515_proped = 0;
    LXValue _v516 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v517 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v517, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    LXValue _v518 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v518, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v517, _v518, _v516}, 4);
px_err_515:
    if (px_err_515_proped) return px_err_515_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v519 = px_null();
    LXValue _v520 = px_null();
    LXValue px_err_521_val = px_null();
    int px_err_521_proped = 0;
    LXValue _v522 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v523 = px_list_n((LXValue[]){}, 0);
    LXValue _v524 = px_null();
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t525 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t525) ? _t525 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        if (px_is_truthy(({ LXValue _t526 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t526) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t526; }))) {
            LXValue _v527 = px_get_global("p_pos");
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                 _v524 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                continue;
            }
            else {
                px_set_global("p_pos", _v527);
            }
        }
        LXValue _v528 = px_null();
        if (px_is_truthy(({ LXValue _t529 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t529) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t529; }))) {
             _v528 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        LXValue _v530 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v531 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        (void)(px_method(_v523, "append", (LXValue[]){px_list_n((LXValue[]){_v528, _v530, _v531}, 3)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Select"), _v523, _v524, _v522}, 4);
px_err_521:
    if (px_err_521_proped) return px_err_521_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_532_val = px_null();
    int px_err_532_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    LXValue _v533 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    LXValue _v534 = px_call(px_get_global("node_pos"), (LXValue[]){_v533}, 1);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v533, _v534}, 3)}, 1);
px_err_532:
    if (px_err_532_proped) return px_err_532_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v535 = px_null();
    LXValue _v536 = px_null();
    LXValue px_err_537_val = px_null();
    int px_err_537_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    LXValue _v538 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            LXValue _v539 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            LXValue _v540 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            LXValue _v541 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v541 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            LXValue _v542 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v542 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            (void)(px_method(_v538, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v540}, 1), _v541, _v542, _v539}, 5)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v538;
px_err_537:
    if (px_err_537_proped) return px_err_537_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_543_val = px_null();
    int px_err_543_proped = 0;
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_543:
    if (px_err_543_proped) return px_err_543_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v544 = px_null();
    LXValue px_err_545_val = px_null();
    int px_err_545_proped = 0;
    LXValue _v546 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|>")}, 1))) {
        LXValue _v547 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v548 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
         _v546 = px_list_n((LXValue[]){px_str("Pipe"), _v546, _v548, _v547}, 4);
    }
    return _v546;
px_err_545:
    if (px_err_545_proped) return px_err_545_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v549 = px_null();
    LXValue px_err_550_val = px_null();
    int px_err_550_proped = 0;
    LXValue _v551 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("??")}, 1))) {
        LXValue _v552 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v553 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
         _v551 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v551, _v553, _v552}, 4);
    }
    return _v551;
px_err_550:
    if (px_err_550_proped) return px_err_550_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v554 = px_null();
    LXValue px_err_555_val = px_null();
    int px_err_555_proped = 0;
    LXValue _v556 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("or")}, 1))) {
        LXValue _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v558 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
         _v556 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v556, _v558, _v557}, 5);
    }
    return _v556;
px_err_555:
    if (px_err_555_proped) return px_err_555_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v559 = px_null();
    LXValue px_err_560_val = px_null();
    int px_err_560_proped = 0;
    LXValue _v561 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("and")}, 1))) {
        LXValue _v562 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v563 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
         _v561 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v561, _v563, _v562}, 5);
    }
    return _v561;
px_err_560:
    if (px_err_560_proped) return px_err_560_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v564 = px_null();
    LXValue _v565 = px_null();
    LXValue px_err_566_val = px_null();
    int px_err_566_proped = 0;
    LXValue _v567 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v568 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("==")}, 1))) {
             _v568 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("!=")}, 1))) {
             _v568 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<")}, 1))) {
             _v568 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<=")}, 1))) {
             _v568 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">")}, 1))) {
             _v568 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">=")}, 1))) {
             _v568 = px_str("Ge");
        }
        if (px_is_truthy(px_eq(_v568, px_null()))) {
            break;
        }
        LXValue _v569 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v570 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
         _v567 = px_list_n((LXValue[]){px_str("Binary"), _v568, _v567, _v570, _v569}, 5);
    }
    return _v567;
px_err_566:
    if (px_err_566_proped) return px_err_566_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v571 = px_null();
    LXValue px_err_572_val = px_null();
    int px_err_572_proped = 0;
    LXValue _v573 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|")}, 1))) {
        LXValue _v574 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v575 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
         _v573 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v573, _v575, _v574}, 5);
    }
    return _v573;
px_err_572:
    if (px_err_572_proped) return px_err_572_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v576 = px_null();
    LXValue px_err_577_val = px_null();
    int px_err_577_proped = 0;
    LXValue _v578 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("^")}, 1))) {
        LXValue _v579 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v580 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
         _v578 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v578, _v580, _v579}, 5);
    }
    return _v578;
px_err_577:
    if (px_err_577_proped) return px_err_577_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v581 = px_null();
    LXValue px_err_582_val = px_null();
    int px_err_582_proped = 0;
    LXValue _v583 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("&")}, 1))) {
        LXValue _v584 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v585 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
         _v583 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v583, _v585, _v584}, 5);
    }
    return _v583;
px_err_582:
    if (px_err_582_proped) return px_err_582_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v586 = px_null();
    LXValue _v587 = px_null();
    LXValue px_err_588_val = px_null();
    int px_err_588_proped = 0;
    LXValue _v589 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v590 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<<")}, 1))) {
             _v590 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>")}, 1))) {
             _v590 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>>")}, 1))) {
             _v590 = px_str("ShrU");
        }
        if (px_is_truthy(px_eq(_v590, px_null()))) {
            break;
        }
        LXValue _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v592 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
         _v589 = px_list_n((LXValue[]){px_str("Binary"), _v590, _v589, _v592, _v591}, 5);
    }
    return _v589;
px_err_588:
    if (px_err_588_proped) return px_err_588_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v593 = px_null();
    LXValue _v594 = px_null();
    LXValue px_err_595_val = px_null();
    int px_err_595_proped = 0;
    LXValue _v596 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v597 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("+")}, 1))) {
             _v597 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("-")}, 1))) {
             _v597 = px_str("Sub");
        }
        if (px_is_truthy(px_eq(_v597, px_null()))) {
            break;
        }
        LXValue _v598 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v599 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
         _v596 = px_list_n((LXValue[]){px_str("Binary"), _v597, _v596, _v599, _v598}, 5);
    }
    return _v596;
px_err_595:
    if (px_err_595_proped) return px_err_595_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v600 = px_null();
    LXValue _v601 = px_null();
    LXValue px_err_602_val = px_null();
    int px_err_602_proped = 0;
    LXValue _v603 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v604 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("*")}, 1))) {
             _v604 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("/")}, 1))) {
             _v604 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("//")}, 1))) {
             _v604 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("%")}, 1))) {
             _v604 = px_str("Mod");
        }
        if (px_is_truthy(px_eq(_v604, px_null()))) {
            break;
        }
        LXValue _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v606 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
         _v603 = px_list_n((LXValue[]){px_str("Binary"), _v604, _v603, _v606, _v605}, 5);
    }
    return _v603;
px_err_602:
    if (px_err_602_proped) return px_err_602_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_607_val = px_null();
    int px_err_607_proped = 0;
    LXValue _v608 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("**")}, 1))) {
        LXValue _v609 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v610 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v608, _v610, _v609}, 5);
    }
    return _v608;
px_err_607:
    if (px_err_607_proped) return px_err_607_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_611_val = px_null();
    int px_err_611_proped = 0;
    LXValue _v612 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v612, px_str("-")))) {
        LXValue _v613 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v614 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v614, _v613}, 4);
    }
    if (px_is_truthy(px_eq(_v612, px_str("not")))) {
        LXValue _v615 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v616 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v616, _v615}, 4);
    }
    if (px_is_truthy(px_eq(_v612, px_str("~")))) {
        LXValue _v617 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v618 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v618, _v617}, 4);
    }
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_611:
    if (px_err_611_proped) return px_err_611_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v619 = px_null();
    LXValue _v620 = px_null();
    LXValue px_err_621_val = px_null();
    int px_err_621_proped = 0;
    LXValue _v622 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v623 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v623, px_str("(")))) {
            LXValue _v624 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            LXValue _v625 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v622 = px_list_n((LXValue[]){px_str("Call"), _v622, _v625, _v624}, 4);
        }
        else if (px_is_truthy(px_eq(_v623, px_str("[")))) {
            LXValue _v626 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                LXValue _v627 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                LXValue _v628 = px_null();
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                     _v628 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                }
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                 _v622 = px_list_n((LXValue[]){px_str("Slice"), _v622, px_null(), _v627, _v628, _v626}, 6);
            }
            else {
                LXValue _v629 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    LXValue _v630 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    LXValue _v631 = px_null();
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                         _v631 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    }
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v622 = px_list_n((LXValue[]){px_str("Slice"), _v622, _v629, _v630, _v631, _v626}, 6);
                }
                else {
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v622 = px_list_n((LXValue[]){px_str("Index"), _v622, _v629, _v626}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v623, px_str(".")))) {
            LXValue _v632 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v633 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v622 = px_list_n((LXValue[]){px_str("Field"), _v622, px_call(px_get_global("qstr"), (LXValue[]){_v633}, 1), _v632}, 4);
        }
        else if (px_is_truthy(px_eq(_v623, px_str("?.")))) {
            LXValue _v634 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v635 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v622 = px_list_n((LXValue[]){px_str("OptionalField"), _v622, px_call(px_get_global("qstr"), (LXValue[]){_v635}, 1), _v634}, 4);
        }
        else if (px_is_truthy(px_eq(_v623, px_str("!")))) {
            LXValue _v636 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v622 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v622, _v636}, 3);
        }
        else if (px_is_truthy(px_eq(_v623, px_str("?")))) {
            LXValue _v637 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v622 = px_list_n((LXValue[]){px_str("Try"), _v622, _v637}, 3);
        }
        else {
            break;
        }
    }
    return _v622;
px_err_621:
    if (px_err_621_proped) return px_err_621_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_638_val = px_null();
    int px_err_638_proped = 0;
    if (px_is_truthy(({ LXValue _t639 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t639) ? _t639 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        return px_null();
    }
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_638:
    if (px_err_638_proped) return px_err_638_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_640_val = px_null();
    int px_err_640_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    LXValue _v641 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v641, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v641;
px_err_640:
    if (px_err_640_proped) return px_err_640_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v642 = px_null();
    LXValue px_err_643_val = px_null();
    int px_err_643_proped = 0;
    LXValue _v644 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v644, px_str("整数")))) {
        LXValue _v645 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v646 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Int"), _v646, _v645}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("浮点")))) {
        LXValue _v647 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v648 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Float"), _v648, _v647}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("字符串")))) {
        LXValue _v649 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v650 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Str"), _v650, _v649}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("true")))) {
        LXValue _v651 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v651}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("false")))) {
        LXValue _v652 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v652}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("null")))) {
        LXValue _v653 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Null"), _v653}, 2);
    }
    if (px_is_truthy(px_eq(_v644, px_str("self")))) {
        LXValue _v654 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v654}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("标识符")))) {
        LXValue _v655 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v656 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v656}, 1), _v655}, 3);
    }
    if (px_is_truthy(px_eq(_v644, px_str("[")))) {
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v644, px_str("(")))) {
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v644, px_str("{")))) {
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v644, px_str("fn")))) {
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v644, px_str("match")))) {
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v644, px_str("if")))) {
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v644, px_str("chan")))) {
        LXValue _v657 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v658 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v657}, 3);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            LXValue _v659 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v658 = px_list_n((LXValue[]){px_str("Call"), _v658, _v659, _v657}, 4);
        }
        return _v658;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_643:
    if (px_err_643_proped) return px_err_643_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_660_val = px_null();
    int px_err_660_proped = 0;
    LXValue _v661 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v661}, 3);
    }
    LXValue _v662 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        LXValue _v663 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("ListComp"), _v662, px_index(_v663, px_int(0LL)), px_index(_v663, px_int(1LL)), _v661}, 5);
    }
    LXValue _v664 = px_list_n((LXValue[]){_v662}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            break;
        }
        (void)(px_method(_v664, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    return px_list_n((LXValue[]){px_str("List"), _v664, _v661}, 3);
px_err_660:
    if (px_err_660_proped) return px_err_660_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_665_val = px_null();
    int px_err_665_proped = 0;
    LXValue _v666 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        (void)(px_method(_v666, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    return _v666;
px_err_665:
    if (px_err_665_proped) return px_err_665_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_667_val = px_null();
    int px_err_667_proped = 0;
    LXValue _v668 = px_list_n((LXValue[]){}, 0);
    LXValue _v669 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v670 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            LXValue _v671 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v668, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v670, _v671}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_method(_v669, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){_v668, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v669}, 1)}, 2);
px_err_667:
    if (px_err_667_proped) return px_err_667_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v672 = (nargs > 0) ? args[0] : px_null();
    LXValue _v673 = px_null();
    LXValue _v674 = px_null();
    LXValue px_err_675_val = px_null();
    int px_err_675_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v672}, 1), px_int(0LL)))) {
        return px_null();
    }
    LXValue _v676 = px_index(_v672, px_int(0LL));
    LXValue _v677 = px_int(1LL);
    while (px_is_truthy(px_lt(_v677, px_call(px_get_global("len"), (LXValue[]){_v672}, 1)))) {
        LXValue _v678 = px_call(px_get_global("node_pos"), (LXValue[]){_v676}, 1);
         _v676 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v676, px_index(_v672, _v677), _v678}, 5);
         _v677 = px_add(_v677, px_int(1LL));
    }
    return _v676;
px_err_675:
    if (px_err_675_proped) return px_err_675_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_679_val = px_null();
    int px_err_679_proped = 0;
    LXValue _v680 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v680}, 3);
    }
    LXValue _v681 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        LXValue _v682 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("GenExp"), _v681, px_index(_v682, px_int(0LL)), px_index(_v682, px_int(1LL)), _v680}, 5);
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        LXValue _v683 = px_list_n((LXValue[]){_v681}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                break;
            }
            (void)(px_method(_v683, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("Tuple"), _v683, _v680}, 3);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v681;
px_err_679:
    if (px_err_679_proped) return px_err_679_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v684 = px_null();
    LXValue _v685 = px_null();
    LXValue px_err_686_val = px_null();
    int px_err_686_proped = 0;
    LXValue _v687 = px_int(0LL);
    LXValue _v688 = px_get_global("p_pos");
    while (px_is_truthy(px_lt(_v688, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        LXValue _v689 = px_index(px_index(px_get_global("p_toks"), _v688), px_int(0LL));
        if (px_is_truthy(({ LXValue _t690 = px_eq(_v689, px_str(":")); px_is_truthy(_t690) ? px_eq(_v687, px_int(0LL)) : _t690; }))) {
            return px_bool(true);
        }
        if (px_is_truthy(({ LXValue _t692 = ({ LXValue _t691 = px_eq(_v689, px_str("(")); px_is_truthy(_t691) ? _t691 : px_eq(_v689, px_str("[")); }); px_is_truthy(_t692) ? _t692 : px_eq(_v689, px_str("{")); }))) {
             _v687 = px_add(_v687, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t693 = px_eq(_v689, px_str(")")); px_is_truthy(_t693) ? _t693 : px_eq(_v689, px_str("]")); }))) {
            if (px_is_truthy(px_gt(_v687, px_int(0LL)))) {
                 _v687 = px_sub(_v687, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t694 = px_eq(_v689, px_str("}")); px_is_truthy(_t694) ? px_eq(_v687, px_int(0LL)) : _t694; }))) {
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t697 = ({ LXValue _t696 = ({ LXValue _t695 = px_eq(_v689, px_str(",")); px_is_truthy(_t695) ? _t695 : px_eq(_v689, px_str("换行")); }); px_is_truthy(_t696) ? _t696 : px_eq(_v689, px_str("EOF")); }); px_is_truthy(_t697) ? px_eq(_v687, px_int(0LL)) : _t697; }))) {
            return px_bool(false);
        }
         _v688 = px_add(_v688, px_int(1LL));
    }
    return px_bool(false);
px_err_686:
    if (px_err_686_proped) return px_err_686_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_698_val = px_null();
    int px_err_698_proped = 0;
    LXValue _v699 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    LXValue _v700 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    if (px_is_truthy(_v700)) {
        LXValue _v701 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v702 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            LXValue _v703 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            return px_list_n((LXValue[]){px_str("DictComp"), _v701, _v702, px_index(_v703, px_int(0LL)), px_index(_v703, px_int(1LL)), _v699}, 6);
        }
        LXValue _v704 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v701, _v702}, 2)}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                break;
            }
            LXValue _v705 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            LXValue _v706 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v704, "append", (LXValue[]){px_list_n((LXValue[]){_v705, _v706}, 2)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("Dict"), _v704, _v699}, 3);
    }
    LXValue _v707 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    while (px_is_truthy(({ LXValue _t708 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t708) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t708; }))) {
        (void)(px_method(_v707, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    return px_list_n((LXValue[]){px_str("Block"), _v707, _v699}, 3);
px_err_698:
    if (px_err_698_proped) return px_err_698_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v709 = px_null();
    LXValue _v710 = px_null();
    LXValue px_err_711_val = px_null();
    int px_err_711_proped = 0;
    LXValue _v712 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v713 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v714 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v714 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    LXValue _v715 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v715, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    LXValue _v716 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        LXValue _v717 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v718 = px_list_n((LXValue[]){}, 0);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        while (px_is_truthy(({ LXValue _t719 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t719) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t719; }))) {
            (void)(px_method(_v718, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
         _v716 = px_list_n((LXValue[]){px_str("Block"), _v718, _v717}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v716 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    else {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    return px_list_n((LXValue[]){px_str("Closure"), _v713, _v714, _v716, _v715, _v712}, 6);
px_err_711:
    if (px_err_711_proped) return px_err_711_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v720 = px_null();
    LXValue _v721 = px_null();
    LXValue _v722 = px_null();
    LXValue px_err_723_val = px_null();
    int px_err_723_proped = 0;
    LXValue _v724 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v725 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v726 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t727 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t727) ? _t727 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        LXValue _v728 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        LXValue _v729 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v729 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v730 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            LXValue _v731 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            LXValue _v732 = px_null();
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v731}, 1), px_int(0LL)))) {
                 _v732 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v731, px_int(0LL))}, 1);
            }
            else {
                 _v732 = _v724;
            }
             _v730 = px_list_n((LXValue[]){px_str("Block"), _v731, _v732}, 3);
        }
        else {
             _v730 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_method(_v726, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v728, _v729, _v730, _v724}, 5)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Match"), _v725, _v726, _v724}, 4);
px_err_723:
    if (px_err_723_proped) return px_err_723_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_733_val = px_null();
    int px_err_733_proped = 0;
    LXValue _v734 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v735 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    LXValue _v736 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    LXValue _v737 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v735, _v736, _v737, _v734}, 5);
px_err_733:
    if (px_err_733_proped) return px_err_733_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_738_val = px_null();
    int px_err_738_proped = 0;
    LXValue _v739 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t744 = ({ LXValue _t743 = ({ LXValue _t742 = ({ LXValue _t741 = ({ LXValue _t740 = px_eq(_v739, px_str("整数")); px_is_truthy(_t740) ? _t740 : px_eq(_v739, px_str("浮点")); }); px_is_truthy(_t741) ? _t741 : px_eq(_v739, px_str("字符串")); }); px_is_truthy(_t742) ? _t742 : px_eq(_v739, px_str("true")); }); px_is_truthy(_t743) ? _t743 : px_eq(_v739, px_str("false")); }); px_is_truthy(_t744) ? _t744 : px_eq(_v739, px_str("null")); }))) {
        LXValue _v745 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v745}, 2);
    }
    if (px_is_truthy(px_eq(_v739, px_str("标识符")))) {
        LXValue _v746 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        LXValue _v747 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v748 = px_list_n((LXValue[]){}, 0);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v748, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v746}, 1), _v748}, 3);
        }
        if (px_is_truthy(px_eq(_v746, px_str("_")))) {
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v746}, 1))) {
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v746}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v746}, 1)}, 2);
    }
    if (px_is_truthy(px_eq(_v739, px_str("(")))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v749 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v749, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    continue;
                }
                break;
            }
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("PatTuple"), _v749}, 2);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_738:
    if (px_err_738_proped) return px_err_738_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v750 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_751_val = px_null();
    int px_err_751_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v750}, 1), px_int(0LL)))) {
        return px_bool(false);
    }
    LXValue _v752 = px_index(_v750, px_int(0LL));
    return ({ LXValue _t753 = px_ge(_v752, px_str("A")); px_is_truthy(_t753) ? px_le(_v752, px_str("Z")) : _t753; });
px_err_751:
    if (px_err_751_proped) return px_err_751_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_754_val = px_null();
    int px_err_754_proped = 0;
    LXValue _v755 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        LXValue _v756 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("TyOptional"), _v755, _v756}, 3);
    }
    return _v755;
px_err_754:
    if (px_err_754_proped) return px_err_754_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_757_val = px_null();
    int px_err_757_proped = 0;
    LXValue _v758 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v758, px_str("标识符")))) {
        LXValue _v759 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        LXValue _v760 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v761 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            if (px_is_truthy(px_eq(_v759, px_str("list")))) {
                return px_list_n((LXValue[]){px_str("TyList"), _v761, _v760}, 3);
            }
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v759}, 1), px_list_n((LXValue[]){_v761}, 1), _v760}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v759}, 1), _v760}, 3);
    }
    if (px_is_truthy(px_eq(_v758, px_str("[")))) {
        LXValue _v762 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v763 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("TyList"), _v763, _v762}, 3);
    }
    if (px_is_truthy(px_eq(_v758, px_str("{")))) {
        LXValue _v764 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v765 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v766 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("TyDict"), _v765, _v766, _v764}, 4);
    }
    if (px_is_truthy(px_eq(_v758, px_str("(")))) {
        LXValue _v767 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v768 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v768, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
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
            LXValue _v769 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v768, _v769, _v767}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyTuple"), _v768, _v767}, 3);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_757:
    if (px_err_757_proped) return px_err_757_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v770 = (nargs > 0) ? args[0] : px_null();
    LXValue _v771 = (nargs > 1) ? args[1] : px_null();
    LXValue _v772 = px_null();
    LXValue _v773 = px_null();
    LXValue _v774 = px_null();
    LXValue _v775 = px_null();
    LXValue _v776 = px_null();
    LXValue _v777 = px_null();
    LXValue _v778 = px_null();
    LXValue _v779 = px_null();
    LXValue px_err_780_val = px_null();
    int px_err_780_proped = 0;
    LXValue _v781 = px_call(px_get_global("cg_pad"), (LXValue[]){_v771}, 1);
    LXValue _v782 = px_index(_v770, px_int(0LL));
    if (px_is_truthy(px_eq(_v782, px_str("VarDecl")))) {
        LXValue _v783 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v770, px_int(2LL))}, 1);
        if (px_is_truthy(({ LXValue _t784 = px_eq(px_index(_v770, px_int(1LL)), px_str("Let")); px_is_truthy(_t784) ? _t784 : px_eq(px_index(_v770, px_int(1LL)), px_str("Const")); }))) {
            (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v783}, 1));
        }
        LXValue _v785 = px_index(_v770, px_int(3LL));
        if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v785}, 1))) {
            px_index_set(px_get_global("cg_nonnull"), _v783, px_int(1LL));
            if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v770, px_int(4LL))}, 1))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v785}, 1)), px_str("'（可空类型请用 ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v785}, 1)), px_str("? 声明）"))}, 2));
            }
        }
        LXValue _v786 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v770, px_int(4LL)), px_null()))) {
             _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(4LL))}, 1);
        }
        if (px_is_truthy(({ LXValue _t787 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v783}, 2); px_is_truthy(_t787) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t787; }))) {
            return px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_set_global(\"")), _v783), px_str("\", ")), _v786), px_str(");\n"));
        }
        LXValue _v788 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v783}, 1);
        if (px_is_truthy(px_ne(px_index(_v770, px_int(4LL)), px_null()))) {
            LXValue _v789 = px_index(_v770, px_int(4LL));
            LXValue _v790 = px_null();
            if (px_is_truthy(px_eq(px_index(_v789, px_int(0LL)), px_str("Constructor")))) {
                 _v790 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v789, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v789, px_int(0LL)), px_str("Call")))) {
                LXValue _v791 = px_index(_v789, px_int(1LL));
                if (px_is_truthy(px_eq(px_index(_v791, px_int(0LL)), px_str("Var")))) {
                     _v790 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v791, px_int(1LL))}, 1);
                }
            }
            if (px_is_truthy(px_ne(_v790, px_null()))) {
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v790}, 1))) {
                    px_index_set(px_get_global("cg_var_types"), _v783, _v790);
                }
            }
        }
        return px_add(px_add(px_add(px_add(px_add(_v781, px_str("LXValue ")), _v788), px_str(" = ")), _v786), px_str(";\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Assign")))) {
        LXValue _v792 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(3LL))}, 1);
        LXValue _v793 = px_index(_v770, px_int(1LL));
        LXValue _v794 = px_index(_v770, px_int(2LL));
        LXValue _v795 = px_index(_v793, px_int(0LL));
        if (px_is_truthy(px_eq(_v795, px_str("Var")))) {
            LXValue _v796 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v793, px_int(1LL))}, 1);
            if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v796}, 1))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v796), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
            }
            if (px_is_truthy(({ LXValue _t797 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v770, px_int(3LL))}, 1); px_is_truthy(_t797) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v796}, 1) : _t797; }))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v796), px_str("'（可空类型请声明为 ")), _v796), px_str(": T?）"))}, 2));
            }
            LXValue _v798 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v796}, 1);
            if (px_is_truthy(px_eq(_v798, px_null()))) {
                if (px_is_truthy(px_eq(_v794, px_str("Assign")))) {
                    return px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_set_global(\"")), _v796), px_str("\", ")), _v792), px_str(");\n"));
                }
                LXValue _v799 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v794, _v796, _v792}, 3);
                return px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_set_global(\"")), _v796), px_str("\", ")), _v799), px_str(");\n"));
            }
            LXValue _v800 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v794, _v798, _v792}, 3);
            return px_add(px_add(px_add(px_add(px_add(_v781, px_str(" ")), _v798), px_str(" = ")), _v800), px_str(";\n"));
        }
        if (px_is_truthy(px_eq(_v795, px_str("Field")))) {
            LXValue _v801 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v793, px_int(1LL))}, 1);
            LXValue _v802 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v793, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_field_set(")), _v801), px_str(", \"")), _v802), px_str("\", ")), _v792), px_str(");\n"));
        }
        if (px_is_truthy(px_eq(_v795, px_str("Index")))) {
            LXValue _v803 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v793, px_int(1LL))}, 1);
            LXValue _v804 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v793, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_index_set(")), _v803), px_str(", ")), _v804), px_str(", ")), _v792), px_str(");\n"));
        }
        return px_str("不支持的赋值目标");
    }
    if (px_is_truthy(px_eq(_v782, px_str("ExprStmt")))) {
        LXValue _v805 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v781, px_str("(void)(")), _v805), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("If")))) {
        LXValue _v806 = px_str("");
        LXValue _v807 = px_index(_v770, px_int(1LL));
        LXValue _v808 = px_int(0LL);
        while (px_is_truthy(px_lt(_v808, px_call(px_get_global("len"), (LXValue[]){_v807}, 1)))) {
            LXValue _v809 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v807, _v808), px_int(0LL))}, 1);
            LXValue _v810 = px_str("if");
            if (px_is_truthy(px_gt(_v808, px_int(0LL)))) {
                 _v810 = px_str("else if");
            }
             _v806 = px_add(_v806, px_add(px_add(px_add(px_add(_v781, _v810), px_str(" (px_is_truthy(")), _v809), px_str(")) {\n")));
            LXValue _v811 = px_index(px_index(_v807, _v808), px_int(1LL));
            LXValue _v812 = px_int(0LL);
            while (px_is_truthy(px_lt(_v812, px_call(px_get_global("len"), (LXValue[]){_v811}, 1)))) {
                 _v806 = px_add(_v806, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v811, _v812), px_add(_v771, px_int(1LL))}, 2));
                 _v812 = px_add(_v812, px_int(1LL));
            }
             _v806 = px_add(_v806, px_add(_v781, px_str("}\n")));
             _v808 = px_add(_v808, px_int(1LL));
        }
        if (px_is_truthy(px_ne(px_index(_v770, px_int(2LL)), px_null()))) {
             _v806 = px_add(_v806, px_add(_v781, px_str("else {\n")));
            LXValue _v813 = px_index(_v770, px_int(2LL));
            LXValue _v814 = px_int(0LL);
            while (px_is_truthy(px_lt(_v814, px_call(px_get_global("len"), (LXValue[]){_v813}, 1)))) {
                 _v806 = px_add(_v806, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v813, _v814), px_add(_v771, px_int(1LL))}, 2));
                 _v814 = px_add(_v814, px_int(1LL));
            }
             _v806 = px_add(_v806, px_add(_v781, px_str("}\n")));
        }
        return _v806;
    }
    if (px_is_truthy(px_eq(_v782, px_str("While")))) {
        LXValue _v815 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1);
        LXValue _v816 = px_add(px_add(px_add(_v781, px_str("while (px_is_truthy(")), _v815), px_str(")) {\n"));
        LXValue _v817 = px_index(_v770, px_int(2LL));
        LXValue _v818 = px_int(0LL);
        while (px_is_truthy(px_lt(_v818, px_call(px_get_global("len"), (LXValue[]){_v817}, 1)))) {
             _v816 = px_add(_v816, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v817, _v818), px_add(_v771, px_int(1LL))}, 2));
             _v818 = px_add(_v818, px_int(1LL));
        }
         _v816 = px_add(_v816, px_add(_v781, px_str("}\n")));
        return _v816;
    }
    if (px_is_truthy(px_eq(_v782, px_str("For")))) {
        LXValue _v819 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(2LL))}, 1);
        LXValue _v820 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v821 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v822 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1)}, 1);
        LXValue _v823 = px_add(px_add(px_add(px_add(px_add(_v781, px_str("LXValue ")), _v820), px_str(" = ")), _v819), px_str(";\n"));
         _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v781, px_str("for (int ")), _v821), px_str(" = 0; ")), _v821), px_str(" < px_len(")), _v820), px_str("); ")), _v821), px_str("++) {\n")));
         _v823 = px_add(_v823, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v781, px_str("    LXValue ")), _v822), px_str(" = px_index(")), _v820), px_str(", px_int(")), _v821), px_str("));\n")));
        LXValue _v824 = px_index(_v770, px_int(3LL));
        LXValue _v825 = px_int(0LL);
        while (px_is_truthy(px_lt(_v825, px_call(px_get_global("len"), (LXValue[]){_v824}, 1)))) {
             _v823 = px_add(_v823, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v824, _v825), px_add(_v771, px_int(1LL))}, 2));
             _v825 = px_add(_v825, px_int(1LL));
        }
         _v823 = px_add(_v823, px_add(_v781, px_str("}\n")));
        return _v823;
    }
    if (px_is_truthy(px_eq(_v782, px_str("Return")))) {
        if (px_is_truthy(px_ne(px_index(_v770, px_int(1LL)), px_null()))) {
            LXValue _v826 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1);
            return px_add(px_add(px_add(_v781, px_str("return ")), _v826), px_str(";\n"));
        }
        return px_add(_v781, px_str("return px_null();\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Break")))) {
        return px_add(_v781, px_str("break;\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Continue")))) {
        return px_add(_v781, px_str("continue;\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Empty")))) {
        return px_str("");
    }
    if (px_is_truthy(px_eq(_v782, px_str("ChanDecl")))) {
        LXValue _v827 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1)}, 1);
        return px_add(px_add(px_add(_v781, px_str("LXValue ")), _v827), px_str(" = px_chan_create(0);\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Send")))) {
        LXValue _v828 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1);
        LXValue _v829 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_chan_send(")), _v828), px_str(", ")), _v829), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Recv")))) {
        LXValue _v830 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v770, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v781, px_str("px_chan_recv(")), _v830), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v782, px_str("Spawn")))) {
        LXValue _v831 = px_index(_v770, px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v831, px_int(0LL)), px_str("Call")))) {
            LXValue _v832 = px_index(_v831, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v832, px_int(0LL)), px_str("Var")))) {
                LXValue _v833 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v832, px_int(1LL))}, 1);
                LXValue _v834 = px_list_n((LXValue[]){}, 0);
                LXValue _v835 = px_index(_v831, px_int(2LL));
                LXValue _v836 = px_int(0LL);
                while (px_is_truthy(px_lt(_v836, px_call(px_get_global("len"), (LXValue[]){_v835}, 1)))) {
                    (void)(px_method(_v834, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v835, _v836)}, 1)}, 1));
                     _v836 = px_add(_v836, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v781, px_str("px_spawn_name(\"")), _v833), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v834}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v834}, 1)}, 1)), px_str(");\n"));
            }
            return px_str("编译模式 spawn 仅支持直接函数调用（方法调用请用 `lx run`）");
        }
        return px_str("编译模式 spawn 仅支持函数调用表达式");
    }
    if (px_is_truthy(px_eq(_v782, px_str("Select")))) {
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v770, px_int(1LL)), px_index(_v770, px_int(2LL)), _v771}, 3);
    }
    if (px_is_truthy(px_eq(_v782, px_str("Import")))) {
        return px_add(_v781, px_str("/* import 忽略（MVP） */\n"));
    }
    return px_str("");
px_err_780:
    if (px_err_780_proped) return px_err_780_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v837 = (nargs > 0) ? args[0] : px_null();
    LXValue _v838 = (nargs > 1) ? args[1] : px_null();
    LXValue _v839 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_840_val = px_null();
    int px_err_840_proped = 0;
    if (px_is_truthy(px_eq(_v837, px_str("Assign")))) {
        return _v839;
    }
    if (px_is_truthy(px_eq(_v837, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v837, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v838), px_str("\"), ")), _v839), px_str(")"));
    }
    return _v839;
px_err_840:
    if (px_err_840_proped) return px_err_840_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v841 = (nargs > 0) ? args[0] : px_null();
    LXValue _v842 = (nargs > 1) ? args[1] : px_null();
    LXValue _v843 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_844_val = px_null();
    int px_err_844_proped = 0;
    if (px_is_truthy(px_eq(_v841, px_str("Assign")))) {
        return _v843;
    }
    if (px_is_truthy(px_eq(_v841, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v841, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v842), px_str(", ")), _v843), px_str(")"));
    }
    return _v843;
px_err_844:
    if (px_err_844_proped) return px_err_844_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v845 = (nargs > 0) ? args[0] : px_null();
    LXValue _v846 = (nargs > 1) ? args[1] : px_null();
    LXValue _v847 = (nargs > 2) ? args[2] : px_null();
    LXValue _v848 = px_null();
    LXValue _v849 = px_null();
    LXValue _v850 = px_null();
    LXValue _v851 = px_null();
    LXValue _v852 = px_null();
    LXValue _v853 = px_null();
    LXValue _v854 = px_null();
    LXValue _v855 = px_null();
    LXValue px_err_856_val = px_null();
    int px_err_856_proped = 0;
    LXValue _v857 = px_call(px_get_global("cg_pad"), (LXValue[]){_v847}, 1);
    LXValue _v858 = px_call(px_get_global("len"), (LXValue[]){_v845}, 1);
    if (px_is_truthy(px_eq(_v858, px_int(0LL)))) {
        return px_str("select 至少需要一个 case 分支");
    }
    LXValue _v859 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    LXValue _v860 = px_str("");
    LXValue _v861 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v862 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    LXValue _v863 = px_list_n((LXValue[]){}, 0);
    LXValue _v864 = px_int(0LL);
    while (px_is_truthy(px_lt(_v864, _v858))) {
        LXValue _v865 = px_index(px_index(_v845, _v864), px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v865, px_int(0LL)), px_str("Call")))) {
            LXValue _v866 = px_index(_v865, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v866, px_int(0LL)), px_str("Field")))) {
                LXValue _v867 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v866, px_int(2LL))}, 1);
                if (px_is_truthy(px_eq(_v867, px_str("recv")))) {
                    (void)(px_method(_v863, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v866, px_int(1LL))}, 1)}, 1));
                     _v864 = px_add(_v864, px_int(1LL));
                    continue;
                }
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v867), px_str("）"));
            }
            return px_str("select case 仅支持 ch.recv()");
        }
        return px_str("select case 仅支持 ch.recv()");
    }
     _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v857, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v858}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v863}, 2)), px_str("};\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(": {\n")));
    LXValue _v868 = px_list_n((LXValue[]){}, 0);
    LXValue _v869 = px_int(0LL);
    while (px_is_truthy(px_lt(_v869, _v858))) {
        (void)(px_method(_v868, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v869}, 1)}, 1));
         _v869 = px_add(_v869, px_int(1LL));
    }
     _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v857, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v858}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v868}, 2)), px_str("};\n")));
    if (px_is_truthy(px_gt(_v858, px_int(1LL)))) {
        LXValue _v870 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v857, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v858}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("--) { "));
         _v870 = px_add(_v870, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" + 1); ")));
         _v870 = px_add(_v870, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("; ")));
         _v870 = px_add(_v870, px_str("}\n"));
         _v860 = px_add(_v860, _v870);
    }
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = px_null();\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = -1;\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v857, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v858}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("++) {\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v857, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("];\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v857, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("; break; }\n")));
     _v860 = px_add(_v860, px_add(_v857, px_str("    }\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" >= 0) {\n")));
    LXValue _v871 = px_int(0LL);
    while (px_is_truthy(px_lt(_v871, _v858))) {
        LXValue _v872 = px_index(px_index(_v845, _v871), px_int(0LL));
        LXValue _v873 = px_index(px_index(_v845, _v871), px_int(2LL));
        LXValue _v874 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v871}, 1)), px_str(")"));
        if (px_is_truthy(px_gt(_v871, px_int(0LL)))) {
             _v874 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v871}, 1)), px_str(")"));
        }
         _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("        ")), _v874), px_str(" {\n")));
        if (px_is_truthy(px_ne(_v872, px_null()))) {
            LXValue _v875 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v872}, 1)}, 1);
             _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_add(_v857, px_str("            LXValue ")), _v875), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(";\n")));
        }
        LXValue _v876 = px_int(0LL);
        while (px_is_truthy(px_lt(_v876, px_call(px_get_global("len"), (LXValue[]){_v873}, 1)))) {
             _v860 = px_add(_v860, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v873, _v876), px_add(_v847, px_int(3LL))}, 2));
             _v876 = px_add(_v876, px_int(1LL));
        }
         _v860 = px_add(_v860, px_add(_v857, px_str("        }\n")));
         _v871 = px_add(_v871, px_int(1LL));
    }
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(";\n")));
     _v860 = px_add(_v860, px_add(_v857, px_str("    }\n")));
    if (px_is_truthy(px_ne(_v846, px_null()))) {
         _v860 = px_add(_v860, px_add(_v857, px_str("    {\n")));
        LXValue _v877 = px_int(0LL);
        while (px_is_truthy(px_lt(_v877, px_call(px_get_global("len"), (LXValue[]){_v846}, 1)))) {
             _v860 = px_add(_v860, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v846, _v877), px_add(_v847, px_int(2LL))}, 2));
             _v877 = px_add(_v877, px_int(1LL));
        }
         _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(";\n")));
         _v860 = px_add(_v860, px_add(_v857, px_str("    }\n")));
    }
     _v860 = px_add(_v860, px_add(_v857, px_str("    px_select_wait();\n")));
     _v860 = px_add(_v860, px_add(_v857, px_str("}\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(";\n")));
     _v860 = px_add(_v860, px_add(px_add(px_add(_v857, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(": ;\n")));
    px_set_global("cg_vars", _v861);
    px_set_global("cg_var_types", _v862);
    return _v860;
px_err_856:
    if (px_err_856_proped) return px_err_856_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v878 = (nargs > 0) ? args[0] : px_null();
    LXValue _v879 = px_null();
    LXValue _v880 = px_null();
    LXValue _v881 = px_null();
    LXValue _v882 = px_null();
    LXValue px_err_883_val = px_null();
    int px_err_883_proped = 0;
    LXValue _v884 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v884, "remove", (LXValue[]){px_str("_")}, 1));
    px_index_set(_v884, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v884, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v884, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v884, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v884, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v884, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    LXValue _v885 = px_int(0LL);
    while (px_is_truthy(px_lt(_v885, px_call(px_get_global("len"), (LXValue[]){_v878}, 1)))) {
        LXValue _v886 = px_index(_v878, _v885);
        LXValue _v887 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v886, px_int(2LL))}, 1);
        (void)(px_method(px_index(_v884, px_str("its")), "append", (LXValue[]){_v887}, 1));
        (void)(px_method(px_index(_v884, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v884, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v884, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        LXValue _v888 = px_str("");
        LXValue _v889 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v886, px_int(1LL))}, 1), px_int(1LL)))) {
            LXValue _v890 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            LXValue _v891 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v886, px_int(1LL)), px_int(0LL))}, 1);
            LXValue _v892 = px_null();
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v891}, 1))) {
                 _v892 = px_index(px_get_global("cg_vars"), _v891);
            }
            px_index_set(px_get_global("cg_vars"), _v891, _v890);
            (void)(px_method(_v889, "append", (LXValue[]){px_list_n((LXValue[]){_v891, _v892}, 2)}, 1));
             _v888 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v890), px_str(" = ")), px_index(px_index(_v884, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v884, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            LXValue _v893 = px_int(0LL);
            while (px_is_truthy(px_lt(_v893, px_call(px_get_global("len"), (LXValue[]){px_index(_v886, px_int(1LL))}, 1)))) {
                LXValue _v894 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v886, px_int(1LL)), _v893)}, 1);
                LXValue _v895 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v893}, 1));
                LXValue _v896 = px_null();
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v894}, 1))) {
                     _v896 = px_index(px_get_global("cg_vars"), _v894);
                }
                px_index_set(px_get_global("cg_vars"), _v894, _v895);
                (void)(px_method(_v889, "append", (LXValue[]){px_list_n((LXValue[]){_v894, _v896}, 2)}, 1));
                 _v888 = px_add(_v888, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v895), px_str(" = px_index(")), px_index(px_index(_v884, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v884, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v893}, 1)), px_str(")); ")));
                 _v893 = px_add(_v893, px_int(1LL));
            }
        }
        (void)(px_method(px_index(_v884, px_str("binds")), "append", (LXValue[]){_v888}, 1));
        (void)(px_method(px_index(_v884, px_str("saved_all")), "append", (LXValue[]){_v889}, 1));
         _v885 = px_add(_v885, px_int(1LL));
    }
    return _v884;
px_err_883:
    if (px_err_883_proped) return px_err_883_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v897 = (nargs > 0) ? args[0] : px_null();
    LXValue _v898 = px_null();
    LXValue _v899 = px_null();
    LXValue px_err_900_val = px_null();
    int px_err_900_proped = 0;
    LXValue _v901 = px_int(0LL);
    while (px_is_truthy(px_lt(_v901, px_call(px_get_global("len"), (LXValue[]){_v897}, 1)))) {
        LXValue _v902 = px_index(_v897, _v901);
        LXValue _v903 = px_int(0LL);
        while (px_is_truthy(px_lt(_v903, px_call(px_get_global("len"), (LXValue[]){_v902}, 1)))) {
            LXValue _v904 = px_index(px_index(_v902, _v903), px_int(0LL));
            LXValue _v905 = px_index(px_index(_v902, _v903), px_int(1LL));
            if (px_is_truthy(px_eq(_v905, px_null()))) {
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v904}, 1));
            }
            else {
                px_index_set(px_get_global("cg_vars"), _v904, _v905);
            }
             _v903 = px_add(_v903, px_int(1LL));
        }
         _v901 = px_add(_v901, px_int(1LL));
    }
px_err_900:
    if (px_err_900_proped) return px_err_900_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v906 = (nargs > 0) ? args[0] : px_null();
    LXValue _v907 = (nargs > 1) ? args[1] : px_null();
    LXValue _v908 = (nargs > 2) ? args[2] : px_null();
    LXValue _v909 = px_null();
    LXValue _v910 = px_null();
    LXValue _v911 = px_null();
    LXValue px_err_912_val = px_null();
    int px_err_912_proped = 0;
    LXValue _v913 = px_str("");
    if (px_is_truthy(px_ne(_v907, px_null()))) {
         _v913 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v907), px_str(")) { ")), _v908), px_str("} "));
    }
    else {
         _v913 = _v908;
    }
    LXValue _v914 = px_call(px_get_global("len"), (LXValue[]){px_index(_v906, px_str("its"))}, 1);
    LXValue _v915 = px_sub(_v914, px_int(1LL));
    while (px_is_truthy(px_ge(_v915, px_int(0LL)))) {
        LXValue _v916 = px_str("");
        if (px_is_truthy(px_lt(px_add(_v915, px_int(1LL)), _v914))) {
             _v916 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v906, px_str("ivs")), px_add(_v915, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v906, px_str("its")), px_add(_v915, px_int(1LL)))), px_str("; "));
        }
         _v913 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v906, px_str("idxs")), _v915)), px_str("=0; ")), px_index(px_index(_v906, px_str("idxs")), _v915)), px_str("<px_len(")), px_index(px_index(_v906, px_str("ivs")), _v915)), px_str("); ")), px_index(px_index(_v906, px_str("idxs")), _v915)), px_str("++) { LXValue ")), px_index(px_index(_v906, px_str("itms")), _v915)), px_str(" = px_index(")), px_index(px_index(_v906, px_str("ivs")), _v915)), px_str(", px_int(")), px_index(px_index(_v906, px_str("idxs")), _v915)), px_str(")); ")), px_index(px_index(_v906, px_str("binds")), _v915)), _v916), _v913), px_str(" } "));
         _v915 = px_sub(_v915, px_int(1LL));
    }
    return _v913;
px_err_912:
    if (px_err_912_proped) return px_err_912_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v917 = (nargs > 0) ? args[0] : px_null();
    LXValue _v918 = px_null();
    LXValue _v919 = px_null();
    LXValue _v920 = px_null();
    LXValue _v921 = px_null();
    LXValue _v922 = px_null();
    LXValue _v923 = px_null();
    LXValue _v924 = px_null();
    LXValue _v925 = px_null();
    LXValue _v926 = px_null();
    LXValue _v927 = px_null();
    LXValue _v928 = px_null();
    LXValue _v929 = px_null();
    LXValue _v930 = px_null();
    LXValue _v931 = px_null();
    LXValue _v932 = px_null();
    LXValue _v933 = px_null();
    LXValue _v934 = px_null();
    LXValue px_err_935_val = px_null();
    int px_err_935_proped = 0;
    LXValue _v936 = px_index(_v917, px_int(0LL));
    if (px_is_truthy(px_eq(_v936, px_str("Int")))) {
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1)), px_str("LL)"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Float")))) {
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Str")))) {
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1)}, 1)), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Bool")))) {
        if (px_is_truthy(px_index(_v917, px_int(1LL)))) {
            return px_str("px_bool(true)");
        }
        return px_str("px_bool(false)");
    }
    if (px_is_truthy(px_eq(_v936, px_str("Null")))) {
        return px_str("px_null()");
    }
    if (px_is_truthy(px_eq(_v936, px_str("List")))) {
        LXValue _v937 = px_list_n((LXValue[]){}, 0);
        LXValue _v938 = px_index(_v917, px_int(1LL));
        LXValue _v939 = px_int(0LL);
        while (px_is_truthy(px_lt(_v939, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
            (void)(px_method(_v937, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v939)}, 1)}, 1));
             _v939 = px_add(_v939, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v937}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v937}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Tuple")))) {
        LXValue _v940 = px_list_n((LXValue[]){}, 0);
        LXValue _v941 = px_index(_v917, px_int(1LL));
        LXValue _v942 = px_int(0LL);
        while (px_is_truthy(px_lt(_v942, px_call(px_get_global("len"), (LXValue[]){_v941}, 1)))) {
            (void)(px_method(_v940, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v941, _v942)}, 1)}, 1));
             _v942 = px_add(_v942, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v940}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v940}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Dict")))) {
        LXValue _v943 = px_str("({ LXValue _d = px_dict(); ");
        LXValue _v944 = px_index(_v917, px_int(1LL));
        LXValue _v945 = px_int(0LL);
        while (px_is_truthy(px_lt(_v945, px_call(px_get_global("len"), (LXValue[]){_v944}, 1)))) {
            LXValue _v946 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v944, _v945), px_int(0LL))}, 1);
            LXValue _v947 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v944, _v945), px_int(1LL))}, 1);
             _v943 = px_add(_v943, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v946), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v947), px_str("); } ")));
             _v945 = px_add(_v945, px_int(1LL));
        }
         _v943 = px_add(_v943, px_str("_d; })"));
        return _v943;
    }
    if (px_is_truthy(px_eq(_v936, px_str("Var")))) {
        LXValue _v948 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v949 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v948}, 1);
        if (px_is_truthy(px_ne(_v949, px_null()))) {
            return _v949;
        }
        return px_add(px_add(px_str("px_get_global(\""), _v948), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Field")))) {
        LXValue _v950 = px_index(_v917, px_int(1LL));
        LXValue _v951 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        if (px_is_truthy(px_eq(px_index(_v950, px_int(0LL)), px_str("Var")))) {
            LXValue _v952 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v950, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v952}, 1))) {
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v952), px_str("\", \"")), _v951), px_str("\")"));
            }
        }
        LXValue _v953 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v950}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v953), px_str(", \"")), _v951), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("OptionalField")))) {
        LXValue _v954 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v955 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v956 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v955), px_str(" = ")), _v954), px_str("; px_is_null(")), _v955), px_str(") ? px_null() : px_field(")), _v954), px_str(", \"")), _v956), px_str("\"); })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Index")))) {
        LXValue _v957 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v958 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v957), px_str(", ")), _v958), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Slice")))) {
        LXValue _v959 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v960 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v917, px_int(2LL)), px_null()))) {
             _v960 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        }
        LXValue _v961 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
             _v961 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        }
        LXValue _v962 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v917, px_int(4LL)), px_null()))) {
             _v962 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(4LL))}, 1);
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v959), px_str(", ")), _v960), px_str(", ")), _v961), px_str(", ")), _v962), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Call")))) {
        LXValue _v963 = px_index(_v917, px_int(1LL));
        LXValue _v964 = px_index(_v917, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v963, px_int(0LL)), px_str("Var")))) {
            LXValue _v965 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v963, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v965}, 1))) {
                LXValue _v966 = px_index(px_get_global("cg_ffi"), _v965);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v964}, 1), px_call(px_get_global("len"), (LXValue[]){_v966}, 1)))) {
                    (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), px_add(px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v965), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v966}, 1)}, 1)), px_str(" 个参数，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v964}, 1)}, 1))}, 2));
                }
                LXValue _v967 = px_list_n((LXValue[]){}, 0);
                LXValue _v968 = px_int(0LL);
                while (px_is_truthy(px_lt(_v968, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
                    (void)(px_method(_v967, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, _v968)}, 1)}, 1));
                     _v968 = px_add(_v968, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v965), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v967}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v967}, 1)}, 1)), px_str(")}, 2)"));
            }
            if (px_is_truthy(px_eq(_v965, px_str("chan")))) {
                LXValue _v969 = px_str("0");
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v964}, 1), px_int(0LL)))) {
                    if (px_is_truthy(px_eq(px_index(px_index(_v964, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                         _v969 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v964, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                         _v969 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                return px_add(px_add(px_str("px_chan_create("), _v969), px_str(")"));
            }
            if (px_is_truthy(px_eq(_v965, px_str("mutex")))) {
                return px_str("px_mutex_create()");
            }
            if (px_is_truthy(px_eq(_v965, px_str("rwlock")))) {
                return px_str("px_rwlock_create()");
            }
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v965}, 1))) {
                LXValue _v970 = px_index(px_get_global("cg_structs"), _v965);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v970}, 1), px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v965), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v970}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v964}, 1)}, 1));
                }
                LXValue _v971 = px_list_n((LXValue[]){}, 0);
                LXValue _v972 = px_int(0LL);
                while (px_is_truthy(px_lt(_v972, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
                    (void)(px_method(_v971, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, _v972)}, 1)}, 1));
                     _v972 = px_add(_v972, px_int(1LL));
                }
                LXValue _v973 = px_list_n((LXValue[]){}, 0);
                LXValue _v974 = px_int(0LL);
                while (px_is_truthy(px_lt(_v974, px_call(px_get_global("len"), (LXValue[]){_v970}, 1)))) {
                    (void)(px_method(_v973, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v970, _v974)), px_str("\""))}, 1));
                     _v974 = px_add(_v974, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v965), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v973}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v971}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v970}, 1)}, 1)), px_str(")"));
            }
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v965}, 1))) {
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v964}, 1), px_int(1LL)))) {
                    return px_add(px_add(px_str("枚举 "), _v965), px_str(" 构造需要一个变体名"));
                }
                LXValue _v975 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, px_int(0LL))}, 1);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v965), px_str("\", (")), _v975), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        if (px_is_truthy(px_eq(px_index(_v963, px_int(0LL)), px_str("Field")))) {
            LXValue _v976 = px_index(_v963, px_int(1LL));
            LXValue _v977 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v963, px_int(2LL))}, 1);
            LXValue _v978 = px_null();
            if (px_is_truthy(px_eq(px_index(_v976, px_int(0LL)), px_str("Var")))) {
                LXValue _v979 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v976, px_int(1LL))}, 1);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v979}, 1))) {
                     _v978 = px_index(px_get_global("cg_var_types"), _v979);
                }
            }
            if (px_is_truthy(({ LXValue _t980 = px_ne(_v978, px_null()); px_is_truthy(_t980) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v978}, 1) : _t980; }))) {
                LXValue _v981 = px_index(px_get_global("cg_impls"), _v978);
                LXValue _v982 = px_bool(false);
                LXValue _v983 = px_int(0LL);
                while (px_is_truthy(px_lt(_v983, px_call(px_get_global("len"), (LXValue[]){_v981}, 1)))) {
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v981, _v983), px_int(1LL))}, 1), _v977))) {
                         _v982 = px_bool(true);
                        break;
                    }
                     _v983 = px_add(_v983, px_int(1LL));
                }
                if (px_is_truthy(_v982)) {
                    LXValue _v984 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v976}, 1);
                    LXValue _v985 = px_list_n((LXValue[]){_v984}, 1);
                    LXValue _v986 = px_int(0LL);
                    while (px_is_truthy(px_lt(_v986, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
                        (void)(px_method(_v985, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, _v986)}, 1)}, 1));
                         _v986 = px_add(_v986, px_int(1LL));
                    }
                    LXValue _v987 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v978}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v977}, 1));
                    return px_add(px_add(px_add(px_add(px_add(_v987, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v985}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v985}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            LXValue _v988 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v976}, 1);
            LXValue _v989 = px_list_n((LXValue[]){}, 0);
            LXValue _v990 = px_int(0LL);
            while (px_is_truthy(px_lt(_v990, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
                (void)(px_method(_v989, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, _v990)}, 1)}, 1));
                 _v990 = px_add(_v990, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v988), px_str(", \"")), _v977), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v989}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v989}, 1)}, 1)), px_str(")"));
        }
        LXValue _v991 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v963}, 1);
        LXValue _v992 = px_list_n((LXValue[]){}, 0);
        LXValue _v993 = px_int(0LL);
        while (px_is_truthy(px_lt(_v993, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
            (void)(px_method(_v992, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v964, _v993)}, 1)}, 1));
             _v993 = px_add(_v993, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v991), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v992}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v992}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Unary")))) {
        LXValue _v994 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        LXValue _v995 = px_index(_v917, px_int(1LL));
        if (px_is_truthy(px_eq(_v995, px_str("Neg")))) {
            return px_add(px_add(px_str("px_neg("), _v994), px_str(")"));
        }
        if (px_is_truthy(px_eq(_v995, px_str("Not")))) {
            return px_add(px_add(px_str("px_not("), _v994), px_str(")"));
        }
        return px_add(px_add(px_str("px_bitnot("), _v994), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Binary")))) {
        LXValue _v996 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        LXValue _v997 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        LXValue _v998 = px_index(_v917, px_int(1LL));
        if (px_is_truthy(px_eq(_v998, px_str("And")))) {
            LXValue _v999 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v999), px_str(" = ")), _v996), px_str("; px_is_truthy(")), _v999), px_str(") ? ")), _v997), px_str(" : ")), _v999), px_str("; })"));
        }
        if (px_is_truthy(px_eq(_v998, px_str("Or")))) {
            LXValue _v1000 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1000), px_str(" = ")), _v996), px_str("; px_is_truthy(")), _v1000), px_str(") ? ")), _v1000), px_str(" : ")), _v997), px_str("; })"));
        }
        LXValue _v1001 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v998}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v1001, px_str("(")), _v996), px_str(", ")), _v997), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Pipe")))) {
        LXValue _v1002 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1003 = px_index(_v917, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v1003, px_int(0LL)), px_str("Call")))) {
            LXValue _v1004 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1003, px_int(1LL))}, 1);
            LXValue _v1005 = px_list_n((LXValue[]){_v1002}, 1);
            LXValue _v1006 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1006, px_call(px_get_global("len"), (LXValue[]){px_index(_v1003, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1005, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v1003, px_int(2LL)), _v1006)}, 1)}, 1));
                 _v1006 = px_add(_v1006, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v1004), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1005}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1005}, 1)}, 1)), px_str(")"));
        }
        LXValue _v1007 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v1003}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v1007), px_str(", (LXValue[]){")), _v1002), px_str("}, 1)"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("NullCoalesce")))) {
        LXValue _v1008 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1009 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        LXValue _v1010 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1010), px_str(" = ")), _v1008), px_str("; px_is_null(")), _v1010), px_str(") ? ")), _v1009), px_str(" : ")), _v1010), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Try")))) {
        LXValue _v1011 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1012 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            LXValue _v1013 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1012), px_str(" = ")), _v1011), px_str("; if (px_is_result(")), _v1012), px_str(")) { if (!px_result_ok(")), _v1012), px_str(")) { ")), _v1013), px_str("_val = ")), _v1012), px_str("; ")), _v1013), px_str("_proped = 1; goto ")), _v1013), px_str("; } ")), _v1012), px_str(" = px_result_unwrap(")), _v1012), px_str("); } else if (px_is_null(")), _v1012), px_str(")) { ")), _v1013), px_str("_val = px_null(); ")), _v1013), px_str("_proped = 1; goto ")), _v1013), px_str("; } ")), _v1012), px_str("; })"));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1012), px_str(" = ")), _v1011), px_str("; if (px_is_result(")), _v1012), px_str(") && !px_result_ok(")), _v1012), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v1012), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v1012), px_str(")) ")), _v1012), px_str(" = px_result_unwrap(")), _v1012), px_str("); ")), _v1012), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("ForceUnwrap")))) {
        LXValue _v1014 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1015 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1015), px_str(" = ")), _v1014), px_str("; if (px_is_result(")), _v1015), px_str(")) { if (!px_result_ok(")), _v1015), px_str(")) px_error(\"force unwrap Err\"); ")), _v1015), px_str(" = px_result_unwrap(")), _v1015), px_str("); } if (px_is_null(")), _v1015), px_str(")) px_error(\"force unwrap null\"); ")), _v1015), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("IfExpr")))) {
        LXValue _v1016 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1017 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        LXValue _v1018 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        LXValue _v1019 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1019), px_str("; if (px_is_truthy(")), _v1016), px_str(")) { ")), _v1019), px_str(" = ")), _v1017), px_str("; } else { ")), _v1019), px_str(" = ")), _v1018), px_str("; } ")), _v1019), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("ListComp")))) {
        LXValue _v1020 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1021 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        LXValue _v1022 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1023 = px_null();
        if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
             _v1023 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v1021, px_str("saved_all"))}, 1));
        LXValue _v1024 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v1020), px_str(", ")), _v1022), px_str("); "));
        LXValue _v1025 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v1021, _v1023, _v1024}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1020), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v1021, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v1021, px_str("its")), px_int(0LL))), px_str("; ")), _v1025), px_str(" ")), _v1020), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("GenExp")))) {
        LXValue _v1026 = px_index(_v917, px_int(2LL));
        if (px_is_truthy(({ LXValue _t1027 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1026}, 1), px_int(1LL)); px_is_truthy(_t1027) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1026, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1027; }))) {
            LXValue _v1028 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1026, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            LXValue _v1029 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v1026, px_int(0LL)), px_int(2LL))}, 1);
            LXValue _v1030 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1028}, 1), px_index(_v917, px_int(1LL))}, 2);
            LXValue _v1031 = px_str("px_null()");
            if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
                 _v1031 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1028}, 1), px_index(_v917, px_int(3LL))}, 2);
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v1029), px_str(", ")), _v1030), px_str(", ")), _v1031), px_str(")"));
        }
        LXValue _v1032 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1033 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v1026}, 1);
        LXValue _v1034 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1035 = px_null();
        if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
             _v1035 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v1033, px_str("saved_all"))}, 1));
        LXValue _v1036 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v1032), px_str(", ")), _v1034), px_str("); "));
        LXValue _v1037 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v1033, _v1035, _v1036}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1032), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v1033, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v1033, px_str("its")), px_int(0LL))), px_str("; ")), _v1037), px_str(" px_gen_from_list(")), _v1032), px_str("); })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("DictComp")))) {
        LXValue _v1038 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1039 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        LXValue _v1040 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1041 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        LXValue _v1042 = px_null();
        if (px_is_truthy(px_ne(px_index(_v917, px_int(4LL)), px_null()))) {
             _v1042 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(4LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v1039, px_str("saved_all"))}, 1));
        LXValue _v1043 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v1040), px_str("; LXValue _v = ")), _v1041), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v1038), px_str(", _k.as.obj->as.str.data, _v); } "));
        LXValue _v1044 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v1039, _v1042, _v1043}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1038), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v1039, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v1039, px_str("its")), px_int(0LL))), px_str("; ")), _v1044), px_str(" ")), _v1038), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Closure")))) {
        px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
        LXValue _v1045 = px_get_global("cg_closure_id");
        LXValue _v1046 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1045}, 1));
        LXValue _v1047 = px_add(px_add(px_str("static LXValue "), _v1046), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
         _v1047 = px_add(_v1047, px_str("    (void)ctx;\n"));
        LXValue _v1048 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
        LXValue _v1049 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
        px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        LXValue _v1050 = px_index(_v917, px_int(1LL));
        LXValue _v1051 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1051, px_call(px_get_global("len"), (LXValue[]){_v1050}, 1)))) {
            LXValue _v1052 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1050, _v1051), px_int(1LL))}, 1)}, 1);
             _v1047 = px_add(_v1047, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1052), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1051}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1051}, 1)), px_str("] : px_null();\n")));
             _v1051 = px_add(_v1051, px_int(1LL));
        }
        LXValue _v1053 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
         _v1047 = px_add(_v1047, px_add(px_add(px_str("    return "), _v1053), px_str(";\n")));
         _v1047 = px_add(_v1047, px_str("}\n"));
        px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1047));
        px_set_global("cg_vars", _v1048);
        px_set_global("cg_var_types", _v1049);
        return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1045}, 1)), px_str(">\", ")), _v1046), px_str(", NULL)"));
    }
    if (px_is_truthy(px_eq(_v936, px_str("Block")))) {
        LXValue _v1054 = px_str("({ ");
         _v1054 = px_add(_v1054, px_str("LXValue _blk = px_null(); "));
        LXValue _v1055 = px_index(_v917, px_int(1LL));
        LXValue _v1056 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1056, px_call(px_get_global("len"), (LXValue[]){_v1055}, 1)))) {
            LXValue _v1057 = px_index(_v1055, _v1056);
            if (px_is_truthy(px_eq(px_index(_v1057, px_int(0LL)), px_str("ExprStmt")))) {
                LXValue _v1058 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1057, px_int(1LL))}, 1);
                 _v1054 = px_add(_v1054, px_add(px_add(px_str("_blk = "), _v1058), px_str("; ")));
            }
            else {
                 _v1054 = px_add(_v1054, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1057, px_int(0LL)}, 2));
            }
             _v1056 = px_add(_v1056, px_int(1LL));
        }
         _v1054 = px_add(_v1054, px_str("_blk; })"));
        return _v1054;
    }
    if (px_is_truthy(px_eq(_v936, px_str("Match")))) {
        LXValue _v1059 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1060 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1061 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1060), px_str(" = ")), _v1059), px_str("; "));
        LXValue _v1062 = px_index(_v917, px_int(2LL));
        LXValue _v1063 = px_bool(true);
        LXValue _v1064 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1064, px_call(px_get_global("len"), (LXValue[]){_v1062}, 1)))) {
            LXValue _v1065 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v1062, _v1064), px_int(1LL)), _v1060}, 2);
            LXValue _v1066 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v1062, _v1064), px_int(3LL))}, 1);
            LXValue _v1067 = px_str("if");
            if (px_is_truthy(px_not(_v1063))) {
                 _v1067 = px_str("else if");
            }
             _v1061 = px_add(_v1061, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v1067, px_str(" (")), _v1065), px_str(") { ")), _v1060), px_str(" = ")), _v1066), px_str("; } ")));
             _v1063 = px_bool(false);
             _v1064 = px_add(_v1064, px_int(1LL));
        }
         _v1061 = px_add(_v1061, px_add(_v1060, px_str("; })")));
        return _v1061;
    }
    if (px_is_truthy(px_eq(_v936, px_str("Constructor")))) {
        LXValue _v1068 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        LXValue _v1069 = px_index(_v917, px_int(2LL));
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v1068}, 1))) {
            LXValue _v1070 = px_index(px_get_global("cg_structs"), _v1068);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1070}, 1), px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)))) {
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v1068), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1070}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)}, 1));
            }
            LXValue _v1071 = px_list_n((LXValue[]){}, 0);
            LXValue _v1072 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1072, px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)))) {
                (void)(px_method(_v1071, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1069, _v1072)}, 1)}, 1));
                 _v1072 = px_add(_v1072, px_int(1LL));
            }
            LXValue _v1073 = px_list_n((LXValue[]){}, 0);
            LXValue _v1074 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1074, px_call(px_get_global("len"), (LXValue[]){_v1070}, 1)))) {
                (void)(px_method(_v1073, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v1070, _v1074)), px_str("\""))}, 1));
                 _v1074 = px_add(_v1074, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v1068), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1073}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1071}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1070}, 1)}, 1)), px_str(")"));
        }
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v1068}, 1))) {
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1069}, 1), px_int(1LL)))) {
                return px_add(px_add(px_str("枚举 "), _v1068), px_str(" 构造需要一个变体名"));
            }
            LXValue _v1075 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1069, px_int(0LL))}, 1);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v1068), px_str("\", (")), _v1075), px_str(").as.obj->as.enum_inst.variant)"));
        }
        LXValue _v1076 = px_list_n((LXValue[]){}, 0);
        LXValue _v1077 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1077, px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)))) {
            (void)(px_method(_v1076, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1069, _v1077)}, 1)}, 1));
             _v1077 = px_add(_v1077, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v1068), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1076}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1076}, 1)}, 1)), px_str(")"));
    }
    return px_str("px_null()");
px_err_935:
    if (px_err_935_proped) return px_err_935_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1078 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1079_val = px_null();
    int px_err_1079_proped = 0;
    if (px_is_truthy(px_eq(_v1078, px_str("Add")))) {
        return px_str("px_add");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Sub")))) {
        return px_str("px_sub");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Mul")))) {
        return px_str("px_mul");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Div")))) {
        return px_str("px_div");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("IntDiv")))) {
        return px_str("px_idiv");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Mod")))) {
        return px_str("px_mod");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Pow")))) {
        return px_str("px_pow");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Eq")))) {
        return px_str("px_eq");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Ne")))) {
        return px_str("px_ne");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Lt")))) {
        return px_str("px_lt");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Le")))) {
        return px_str("px_le");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Gt")))) {
        return px_str("px_gt");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Ge")))) {
        return px_str("px_ge");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("BitAnd")))) {
        return px_str("px_bitand");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("BitOr")))) {
        return px_str("px_bitor");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("BitXor")))) {
        return px_str("px_bitxor");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Shl")))) {
        return px_str("px_shl");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("Shr")))) {
        return px_str("px_shr");
    }
    if (px_is_truthy(px_eq(_v1078, px_str("ShrU")))) {
        return px_str("px_ushr");
    }
    return px_str("px_add");
px_err_1079:
    if (px_err_1079_proped) return px_err_1079_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1080 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1081 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1082_val = px_null();
    int px_err_1082_proped = 0;
    LXValue _v1083 = px_index(_v1080, px_int(0LL));
    if (px_is_truthy(px_eq(_v1083, px_str("PatLiteral")))) {
        LXValue _v1084 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1080, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v1081), px_str(", ")), _v1084), px_str("))"));
    }
    if (px_is_truthy(px_eq(_v1083, px_str("PatBinding")))) {
        LXValue _v1085 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1080, px_int(1LL))}, 1);
        if (px_is_truthy(({ LXValue _t1087 = ({ LXValue _t1086 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1085}, 1), px_int(0LL)); px_is_truthy(_t1086) ? px_ge(px_index(_v1085, px_int(0LL)), px_str("A")) : _t1086; }); px_is_truthy(_t1087) ? px_le(px_index(_v1085, px_int(0LL)), px_str("Z")) : _t1087; }))) {
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v1081), px_str(".type == PX_ENUM && strcmp(")), _v1081), px_str(".as.obj->as.enum_inst.variant, \"")), _v1085), px_str("\") == 0)"));
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1083, px_str("PatWildcard")))) {
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1083, px_str("PatTuple")))) {
        LXValue _v1088 = px_index(_v1080, px_int(1LL));
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1088}, 1), px_int(0LL)))) {
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v1088, px_int(0LL)), _v1081}, 2);
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1083, px_str("PatConstructor")))) {
        LXValue _v1089 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1080, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v1081), px_str(".type == PX_ENUM && strcmp(")), _v1081), px_str(".as.obj->as.enum_inst.variant, \"")), _v1089), px_str("\") == 0)"));
    }
    return px_str("true");
px_err_1082:
    if (px_err_1082_proped) return px_err_1082_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1090 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1091 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1092 = px_null();
    LXValue _v1093 = px_null();
    LXValue px_err_1094_val = px_null();
    int px_err_1094_proped = 0;
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    LXValue _v1095 = px_get_global("cg_closure_id");
    LXValue _v1096 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1095}, 1));
    LXValue _v1097 = px_add(px_add(px_str("static LXValue "), _v1096), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1097 = px_add(_v1097, px_str("    (void)ctx;\n"));
    LXValue _v1098 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v1099 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1100 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1100, px_call(px_get_global("len"), (LXValue[]){_v1090}, 1)))) {
        LXValue _v1101 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v1090, _v1100)}, 1);
         _v1097 = px_add(_v1097, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1101), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1100}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1100}, 1)), px_str("] : px_null();\n")));
         _v1100 = px_add(_v1100, px_int(1LL));
    }
    LXValue _v1102 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v1091}, 1);
    LXValue _v1103 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1103}, 1));
     _v1097 = px_add(_v1097, px_add(px_add(px_str("    LXValue "), _v1103), px_str("_val = px_null();\n")));
     _v1097 = px_add(_v1097, px_add(px_add(px_str("    int "), _v1103), px_str("_proped = 0;\n")));
     _v1097 = px_add(_v1097, px_add(px_add(px_str("    return "), _v1102), px_str(";\n")));
     _v1097 = px_add(_v1097, px_add(_v1103, px_str(":\n")));
     _v1097 = px_add(_v1097, px_add(px_add(px_add(px_add(px_str("    if ("), _v1103), px_str("_proped) return ")), _v1103), px_str("_val;\n")));
     _v1097 = px_add(_v1097, px_str("    return px_null();\n"));
     _v1097 = px_add(_v1097, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1097));
    px_set_global("cg_vars", _v1098);
    px_set_global("cg_var_types", _v1099);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1095}, 1)), px_str(">\", ")), _v1096), px_str(", NULL)"));
px_err_1094:
    if (px_err_1094_proped) return px_err_1094_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1104 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1105 = px_null();
    LXValue px_err_1106_val = px_null();
    int px_err_1106_proped = 0;
    LXValue _v1107 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1104}, 1), px_int(1LL));
    while (px_is_truthy(px_ge(_v1107, px_int(0LL)))) {
        if (px_is_truthy(px_eq(px_index(_v1104, _v1107), px_str("/")))) {
            if (px_is_truthy(px_eq(_v1107, px_int(0LL)))) {
                return px_str("/");
            }
            return px_slice(_v1104, px_int(0LL), _v1107, px_null());
        }
         _v1107 = px_sub(_v1107, px_int(1LL));
    }
    return px_str(".");
px_err_1106:
    if (px_err_1106_proped) return px_err_1106_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1108 = px_null();
    LXValue px_err_1109_val = px_null();
    int px_err_1109_proped = 0;
    LXValue _v1110 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    if (px_is_truthy(({ LXValue _t1111 = px_ne(_v1110, px_null()); px_is_truthy(_t1111) ? px_call(px_get_global("exists"), (LXValue[]){_v1110}, 1) : _t1111; }))) {
        return _v1110;
    }
    LXValue _v1112 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    LXValue _v1113 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1113, px_call(px_get_global("len"), (LXValue[]){_v1112}, 1)))) {
        LXValue _v1114 = px_index(_v1112, _v1113);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1114}, 1))) {
            return _v1114;
        }
         _v1113 = px_add(_v1113, px_int(1LL));
    }
    return px_null();
px_err_1109:
    if (px_err_1109_proped) return px_err_1109_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1115 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1116 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1117 = px_null();
    LXValue _v1118 = px_null();
    LXValue _v1119 = px_null();
    LXValue _v1120 = px_null();
    LXValue px_err_1121_val = px_null();
    int px_err_1121_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1115}, 1), px_int(0LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t1123 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1115}, 1), px_int(1LL)); px_is_truthy(_t1123) ? ({ LXValue _t1122 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v1115, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1122) ? _t1122 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v1115, px_int(0LL)), px_str(".px")}, 2); }) : _t1123; }))) {
        LXValue _v1124 = px_index(_v1115, px_int(0LL));
        LXValue _v1125 = _v1124;
        if (px_is_truthy(px_not(({ LXValue _t1126 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1124}, 1), px_int(0LL)); px_is_truthy(_t1126) ? px_eq(px_index(_v1124, px_int(0LL)), px_str("/")) : _t1126; })))) {
             _v1125 = px_add(px_add(_v1116, px_str("/")), _v1124);
        }
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1125}, 1))) {
            return _v1125;
        }
        return px_null();
    }
    if (px_is_truthy(px_eq(px_index(_v1115, px_int(0LL)), px_str("std")))) {
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v1115}, 1), px_int(2LL)))) {
            return px_null();
        }
        LXValue _v1127 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v1127, px_null()))) {
            return px_null();
        }
        LXValue _v1128 = _v1127;
        LXValue _v1129 = px_int(1LL);
        while (px_is_truthy(px_lt(_v1129, px_call(px_get_global("len"), (LXValue[]){_v1115}, 1)))) {
             _v1128 = px_add(_v1128, px_add(px_str("/"), px_index(_v1115, _v1129)));
             _v1129 = px_add(_v1129, px_int(1LL));
        }
        LXValue _v1130 = px_add(_v1128, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1130}, 1))) {
            return _v1130;
        }
        LXValue _v1131 = px_add(_v1128, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1131}, 1))) {
            return _v1131;
        }
        return px_null();
    }
    LXValue _v1132 = px_list_n((LXValue[]){_v1116}, 1);
    LXValue _v1133 = px_add(_v1116, px_str("/.px_modules"));
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1133}, 1))) {
        (void)(px_method(_v1132, "append", (LXValue[]){_v1133}, 1));
        LXValue _v1134 = px_call(px_get_global("list_dir"), (LXValue[]){_v1133}, 1);
        LXValue _v1135 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1135, px_call(px_get_global("len"), (LXValue[]){_v1134}, 1)))) {
            LXValue _v1136 = px_index(_v1134, _v1135);
            LXValue _v1137 = px_add(px_add(_v1133, px_str("/")), _v1136);
            if (px_is_truthy(({ LXValue _t1138 = px_call(px_get_global("exists"), (LXValue[]){_v1137}, 1); px_is_truthy(_t1138) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1136, px_str(".")}, 2)) : _t1138; }))) {
                (void)(px_method(_v1132, "append", (LXValue[]){_v1137}, 1));
            }
             _v1135 = px_add(_v1135, px_int(1LL));
        }
    }
    LXValue _v1139 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1139, px_call(px_get_global("len"), (LXValue[]){_v1132}, 1)))) {
        LXValue _v1140 = px_index(_v1132, _v1139);
        LXValue _v1141 = _v1140;
        LXValue _v1142 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1142, px_call(px_get_global("len"), (LXValue[]){_v1115}, 1)))) {
             _v1141 = px_add(_v1141, px_add(px_str("/"), px_index(_v1115, _v1142)));
             _v1142 = px_add(_v1142, px_int(1LL));
        }
        LXValue _v1143 = px_add(_v1141, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1143}, 1))) {
            return _v1143;
        }
        LXValue _v1144 = px_add(_v1141, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1144}, 1))) {
            return _v1144;
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1115}, 1), px_int(1LL)))) {
            LXValue _v1145 = px_add(px_add(px_add(_v1140, px_str("/")), px_index(_v1115, px_int(0LL))), px_str(".px"));
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1145}, 1))) {
                return _v1145;
            }
        }
         _v1139 = px_add(_v1139, px_int(1LL));
    }
    return px_null();
px_err_1121:
    if (px_err_1121_proped) return px_err_1121_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1146 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1147_val = px_null();
    int px_err_1147_proped = 0;
    LXValue _v1148 = px_index(_v1146, px_int(0LL));
    if (px_is_truthy(px_eq(_v1148, px_str("FuncDef")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1146, px_int(1LL))}, 1), px_str("main")))) {
            return px_bool(false);
        }
        return px_bool(true);
    }
    if (px_is_truthy(px_eq(_v1148, px_str("ExternDef")))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t1151 = ({ LXValue _t1150 = ({ LXValue _t1149 = px_eq(_v1148, px_str("StructDef")); px_is_truthy(_t1149) ? _t1149 : px_eq(_v1148, px_str("EnumDef")); }); px_is_truthy(_t1150) ? _t1150 : px_eq(_v1148, px_str("TraitDef")); }); px_is_truthy(_t1151) ? _t1151 : px_eq(_v1148, px_str("ImplDef")); }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t1152 = px_eq(_v1148, px_str("VarDecl")); px_is_truthy(_t1152) ? px_eq(px_index(_v1146, px_int(1LL)), px_str("Const")) : _t1152; }))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_1147:
    if (px_err_1147_proped) return px_err_1147_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1153 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1154 = px_null();
    LXValue px_err_1155_val = px_null();
    int px_err_1155_proped = 0;
    LXValue _v1156 = px_index(_v1153, px_int(0LL));
    if (px_is_truthy(({ LXValue _t1160 = ({ LXValue _t1159 = ({ LXValue _t1158 = ({ LXValue _t1157 = px_eq(_v1156, px_str("FuncDef")); px_is_truthy(_t1157) ? _t1157 : px_eq(_v1156, px_str("StructDef")); }); px_is_truthy(_t1158) ? _t1158 : px_eq(_v1156, px_str("EnumDef")); }); px_is_truthy(_t1159) ? _t1159 : px_eq(_v1156, px_str("TraitDef")); }); px_is_truthy(_t1160) ? _t1160 : px_eq(_v1156, px_str("ExternDef")); }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1153, px_int(1LL))}, 1);
    }
    if (px_is_truthy(({ LXValue _t1161 = px_eq(_v1156, px_str("VarDecl")); px_is_truthy(_t1161) ? px_eq(px_index(_v1153, px_int(1LL)), px_str("Const")) : _t1161; }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1153, px_int(2LL))}, 1);
    }
    if (px_is_truthy(px_eq(_v1156, px_str("ImplDef")))) {
        LXValue _v1162 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1153, px_int(1LL))}, 1);
        LXValue _v1163 = px_index(_v1153, px_int(2LL));
        LXValue _v1164 = px_str("None");
        if (px_is_truthy(px_ne(_v1163, px_null()))) {
             _v1164 = px_add(px_add(px_str("Some("), _v1163), px_str(")"));
        }
        return px_add(px_add(px_add(px_str("impl::"), _v1162), px_str("::")), _v1164);
    }
    return px_null();
px_err_1155:
    if (px_err_1155_proped) return px_err_1155_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1165 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1166 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1167 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1168 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1169 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1170 = px_null();
    LXValue _v1171 = px_null();
    LXValue _v1172 = px_null();
    LXValue _v1173 = px_null();
    LXValue _v1174 = px_null();
    LXValue px_err_1175_val = px_null();
    int px_err_1175_proped = 0;
    LXValue _v1176 = px_list_n((LXValue[]){}, 0);
    LXValue _v1177 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1177, px_call(px_get_global("len"), (LXValue[]){_v1165}, 1)))) {
        (void)(px_method(_v1176, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1165, _v1177)}, 1)}, 1));
         _v1177 = px_add(_v1177, px_int(1LL));
    }
     _v1165 = _v1176;
    LXValue _v1178 = px_list_n((LXValue[]){}, 0);
    LXValue _v1179 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1179, px_call(px_get_global("len"), (LXValue[]){_v1166}, 1)))) {
        (void)(px_method(_v1178, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1166, _v1179)}, 1)}, 1));
         _v1179 = px_add(_v1179, px_int(1LL));
    }
     _v1166 = _v1178;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1165}, 1), px_int(0LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t1182 = ({ LXValue _t1181 = ({ LXValue _t1180 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1165}, 1), px_int(1LL)); px_is_truthy(_t1180) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1165, px_int(0LL))}, 1), px_int(2LL)) : _t1180; }); px_is_truthy(_t1181) ? px_eq(px_slice(px_index(_v1165, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t1181; }); px_is_truthy(_t1182) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v1165, px_int(0LL)), px_str(".px")}, 2)) : _t1182; }))) {
        return px_null();
    }
    LXValue _v1183 = px_eq(px_index(_v1165, px_int(0LL)), px_str("std"));
    LXValue _v1184 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1165}, 2);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1184}, 1))) {
        return px_null();
    }
    LXValue _v1185 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1165, _v1167}, 2);
    if (px_is_truthy(px_eq(_v1185, px_null()))) {
        if (px_is_truthy(px_ne(px_index(_v1165, px_int(0LL)), px_str("std")))) {
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("[module] 警告: 找不到模块 '"), _v1184), px_str("'（已跳过）"))}, 1));
        }
        return px_null();
    }
    px_index_set(px_get_global("loaded"), _v1184, _v1185);
    LXValue _v1186 = px_call(px_get_global("read_file"), (LXValue[]){_v1185}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1186}, 1));
    px_set_global("p_pos", px_int(0LL));
    LXValue _v1187 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    LXValue _v1188 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1185}, 1);
    LXValue _v1189 = px_list_n((LXValue[]){}, 0);
    LXValue _v1190 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1190, px_call(px_get_global("len"), (LXValue[]){px_index(_v1187, px_int(1LL))}, 1)))) {
        LXValue _v1191 = px_index(px_index(_v1187, px_int(1LL)), _v1190);
        if (px_is_truthy(px_eq(px_index(_v1191, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v1189, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1191, px_int(1LL)), px_index(_v1191, px_int(2LL))}, 2)}, 1));
        }
         _v1190 = px_add(_v1190, px_int(1LL));
    }
    LXValue _v1192 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1192, px_call(px_get_global("len"), (LXValue[]){_v1189}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1189, _v1192), px_int(0LL)), px_index(px_index(_v1189, _v1192), px_int(1LL)), _v1188, _v1168, _v1169}, 5));
         _v1192 = px_add(_v1192, px_int(1LL));
    }
    LXValue _v1193 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1166}, 1), px_int(0LL));
    LXValue _v1194 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1194, px_call(px_get_global("len"), (LXValue[]){px_index(_v1187, px_int(1LL))}, 1)))) {
        LXValue _v1195 = px_index(px_index(_v1187, px_int(1LL)), _v1194);
        if (px_is_truthy(px_eq(px_index(_v1195, px_int(0LL)), px_str("Import")))) {
             _v1194 = px_add(_v1194, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1195}, 1)))) {
             _v1194 = px_add(_v1194, px_int(1LL));
            continue;
        }
        LXValue _v1196 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1195}, 1);
        if (px_is_truthy(px_eq(_v1196, px_null()))) {
            (void)(px_method(_v1168, "append", (LXValue[]){_v1195}, 1));
        }
        else {
            if (px_is_truthy(_v1193)) {
                if (px_is_truthy(({ LXValue _t1197 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1196}, 1), px_int(5LL)); px_is_truthy(_t1197) ? px_eq(px_slice(_v1196, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1197; }))) {
                     _v1194 = px_add(_v1194, px_int(1LL));
                    continue;
                }
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1166, _v1196}, 2)))) {
                     _v1194 = px_add(_v1194, px_int(1LL));
                    continue;
                }
            }
            if (px_is_truthy(_v1183)) {
                if (px_is_truthy(px_method(_v1169, "has", (LXValue[]){_v1196}, 1))) {
                     _v1194 = px_add(_v1194, px_int(1LL));
                    continue;
                }
                px_index_set(_v1169, _v1196, px_bool(true));
            }
            (void)(px_method(_v1168, "append", (LXValue[]){_v1195}, 1));
        }
         _v1194 = px_add(_v1194, px_int(1LL));
    }
px_err_1175:
    if (px_err_1175_proped) return px_err_1175_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1198 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1199 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1200 = px_null();
    LXValue _v1201 = px_null();
    LXValue _v1202 = px_null();
    LXValue _v1203 = px_null();
    LXValue px_err_1204_val = px_null();
    int px_err_1204_proped = 0;
    LXValue _v1205 = px_index(_v1198, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1205}, 1), px_int(0LL)))) {
        return _v1198;
    }
    LXValue _v1206 = px_list_n((LXValue[]){}, 0);
    LXValue _v1207 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1208 = px_list_n((LXValue[]){}, 0);
    LXValue _v1209 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1209, px_call(px_get_global("len"), (LXValue[]){_v1205}, 1)))) {
        LXValue _v1210 = px_index(_v1205, _v1209);
        if (px_is_truthy(px_eq(px_index(_v1210, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v1208, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1210, px_int(1LL)), px_index(_v1210, px_int(2LL))}, 2)}, 1));
        }
         _v1209 = px_add(_v1209, px_int(1LL));
    }
    LXValue _v1211 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1211, px_call(px_get_global("len"), (LXValue[]){_v1208}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1208, _v1211), px_int(0LL)), px_index(px_index(_v1208, _v1211), px_int(1LL)), _v1199, _v1206, _v1207}, 5));
         _v1211 = px_add(_v1211, px_int(1LL));
    }
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1206}, 1), px_int(0LL)))) {
        return _v1198;
    }
    LXValue _v1212 = px_list_n((LXValue[]){}, 0);
    LXValue _v1213 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1213, px_call(px_get_global("len"), (LXValue[]){_v1206}, 1)))) {
        (void)(px_method(_v1212, "append", (LXValue[]){px_index(_v1206, _v1213)}, 1));
         _v1213 = px_add(_v1213, px_int(1LL));
    }
    LXValue _v1214 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1214, px_call(px_get_global("len"), (LXValue[]){_v1205}, 1)))) {
        (void)(px_method(_v1212, "append", (LXValue[]){px_index(_v1205, _v1214)}, 1));
         _v1214 = px_add(_v1214, px_int(1LL));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v1212}, 2);
px_err_1204:
    if (px_err_1204_proped) return px_err_1204_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1215_val = px_null();
    int px_err_1215_proped = 0;
    LXValue _v1216 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v1216, "remove", (LXValue[]){px_str("_")}, 1));
    return _v1216;
px_err_1215:
    if (px_err_1215_proped) return px_err_1215_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1217 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1218 = px_null();
    LXValue px_err_1219_val = px_null();
    int px_err_1219_proped = 0;
    LXValue _v1220 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    LXValue _v1221 = px_method(_v1217, "keys", (LXValue[]){}, 0);
    LXValue _v1222 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1222, px_call(px_get_global("len"), (LXValue[]){_v1221}, 1)))) {
        px_index_set(_v1220, px_index(_v1221, _v1222), px_index(_v1217, px_index(_v1221, _v1222)));
         _v1222 = px_add(_v1222, px_int(1LL));
    }
    return _v1220;
px_err_1219:
    if (px_err_1219_proped) return px_err_1219_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1223_val = px_null();
    int px_err_1223_proped = 0;
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    return px_get_global("cg_uidc");
px_err_1223:
    if (px_err_1223_proped) return px_err_1223_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1224_val = px_null();
    int px_err_1224_proped = 0;
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1224:
    if (px_err_1224_proped) return px_err_1224_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1225 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1226_val = px_null();
    int px_err_1226_proped = 0;
    LXValue _v1227 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_index_set(px_get_global("cg_vars"), _v1225, _v1227);
    return _v1227;
px_err_1226:
    if (px_err_1226_proped) return px_err_1226_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1228 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1229_val = px_null();
    int px_err_1229_proped = 0;
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1228}, 1))) {
        return px_index(px_get_global("cg_vars"), _v1228);
    }
    return px_null();
px_err_1229:
    if (px_err_1229_proped) return px_err_1229_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1230 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1231_val = px_null();
    int px_err_1231_proped = 0;
    px_index_set(px_get_global("cg_immutables"), _v1230, px_int(1LL));
px_err_1231:
    if (px_err_1231_proped) return px_err_1231_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1232 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1233_val = px_null();
    int px_err_1233_proped = 0;
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1232}, 1);
px_err_1233:
    if (px_err_1233_proped) return px_err_1233_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1234 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1235 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1236_val = px_null();
    int px_err_1236_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1234), px_str(": ")), _v1235)}, 1));
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1236:
    if (px_err_1236_proped) return px_err_1236_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1237 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1238_val = px_null();
    int px_err_1238_proped = 0;
    if (px_is_truthy(px_eq(_v1237, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(px_index(_v1237, px_int(0LL)), px_str("TyOptional")))) {
        return px_bool(false);
    }
    return px_bool(true);
px_err_1238:
    if (px_err_1238_proped) return px_err_1238_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1239 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1240_val = px_null();
    int px_err_1240_proped = 0;
    if (px_is_truthy(px_eq(_v1239, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(px_index(_v1239, px_int(0LL)), px_str("Null")))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_1240:
    if (px_err_1240_proped) return px_err_1240_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1241 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1242_val = px_null();
    int px_err_1242_proped = 0;
    if (px_is_truthy(px_eq(_v1241, px_null()))) {
        return px_str("any");
    }
    if (px_is_truthy(px_eq(px_index(_v1241, px_int(0LL)), px_str("TyOptional")))) {
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1241, px_int(1LL))}, 1), px_str("?"));
    }
    if (px_is_truthy(px_eq(px_index(_v1241, px_int(0LL)), px_str("TyNamed")))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1241, px_int(1LL))}, 1);
    }
    if (px_is_truthy(px_eq(px_index(_v1241, px_int(0LL)), px_str("TyList")))) {
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1241, px_int(1LL))}, 1)), px_str("]"));
    }
    if (px_is_truthy(px_eq(px_index(_v1241, px_int(0LL)), px_str("TyDict")))) {
        return px_add(px_add(px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1241, px_int(1LL))}, 1)), px_str(": ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1241, px_int(2LL))}, 1)), px_str("}"));
    }
    return px_str("any");
px_err_1242:
    if (px_err_1242_proped) return px_err_1242_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1243 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1244 = px_null();
    LXValue _v1245 = px_null();
    LXValue px_err_1246_val = px_null();
    int px_err_1246_proped = 0;
    LXValue _v1247 = px_str("");
    LXValue _v1248 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1248, px_call(px_get_global("len"), (LXValue[]){_v1243}, 1)))) {
        LXValue _v1249 = px_index(_v1243, _v1248);
        LXValue _v1251 = ({ LXValue _t1250 = px_ge(_v1249, px_str("a")); px_is_truthy(_t1250) ? px_le(_v1249, px_str("z")) : _t1250; });
        LXValue _v1253 = ({ LXValue _t1252 = px_ge(_v1249, px_str("A")); px_is_truthy(_t1252) ? px_le(_v1249, px_str("Z")) : _t1252; });
        LXValue _v1255 = ({ LXValue _t1254 = px_ge(_v1249, px_str("0")); px_is_truthy(_t1254) ? px_le(_v1249, px_str("9")) : _t1254; });
        if (px_is_truthy(({ LXValue _t1257 = ({ LXValue _t1256 = _v1251; px_is_truthy(_t1256) ? _t1256 : _v1253; }); px_is_truthy(_t1257) ? _t1257 : _v1255; }))) {
             _v1247 = px_add(_v1247, _v1249);
        }
        else {
             _v1247 = px_add(_v1247, px_str("_"));
        }
         _v1248 = px_add(_v1248, px_int(1LL));
    }
    return _v1247;
px_err_1246:
    if (px_err_1246_proped) return px_err_1246_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1258 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1259 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1260 = px_null();
    LXValue _v1261 = px_null();
    LXValue _v1262 = px_null();
    LXValue px_err_1263_val = px_null();
    int px_err_1263_proped = 0;
    LXValue _v1264 = px_call(px_get_global("len"), (LXValue[]){_v1258}, 1);
    LXValue _v1265 = px_call(px_get_global("len"), (LXValue[]){_v1259}, 1);
    LXValue _v1266 = px_int(0LL);
    while (px_is_truthy(px_le(px_add(_v1266, _v1265), _v1264))) {
        LXValue _v1267 = px_int(0LL);
        LXValue _v1268 = px_bool(true);
        while (px_is_truthy(px_lt(_v1267, _v1265))) {
            if (px_is_truthy(px_ne(px_index(_v1258, px_add(_v1266, _v1267)), px_index(_v1259, _v1267)))) {
                 _v1268 = px_bool(false);
                break;
            }
             _v1267 = px_add(_v1267, px_int(1LL));
        }
        if (px_is_truthy(_v1268)) {
            return _v1266;
        }
         _v1266 = px_add(_v1266, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_1263:
    if (px_err_1263_proped) return px_err_1263_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1269 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1270 = px_null();
    LXValue _v1271 = px_null();
    LXValue px_err_1272_val = px_null();
    int px_err_1272_proped = 0;
    LXValue _v1273 = px_str("");
    LXValue _v1274 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1274, _v1269))) {
         _v1273 = px_add(_v1273, px_str("    "));
         _v1274 = px_add(_v1274, px_int(1LL));
    }
    return _v1273;
px_err_1272:
    if (px_err_1272_proped) return px_err_1272_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1275 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1276 = px_null();
    LXValue _v1277 = px_null();
    LXValue _v1278 = px_null();
    LXValue _v1279 = px_null();
    LXValue px_err_1280_val = px_null();
    int px_err_1280_proped = 0;
    LXValue _v1281 = px_slice(_v1275, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1275}, 1), px_int(1LL)), px_null());
    LXValue _v1282 = px_str("");
    LXValue _v1283 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1283, px_call(px_get_global("len"), (LXValue[]){_v1281}, 1)))) {
        LXValue _v1284 = px_index(_v1281, _v1283);
        if (px_is_truthy(px_eq(_v1284, px_str("\\")))) {
            LXValue _v1285 = px_index(_v1281, px_add(_v1283, px_int(1LL)));
            if (px_is_truthy(px_eq(_v1285, px_str("n")))) {
                 _v1282 = px_add(_v1282, px_str("\n"));
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1285, px_str("t")))) {
                 _v1282 = px_add(_v1282, px_str("\t"));
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1285, px_str("r")))) {
                 _v1282 = px_add(_v1282, px_str("\r"));
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1285, px_str("0")))) {
                 _v1282 = px_add(_v1282, px_str(""));
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1285, px_str("\"")))) {
                 _v1282 = px_add(_v1282, px_str("\""));
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1285, px_str("\\")))) {
                 _v1282 = px_add(_v1282, px_str("\\"));
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1285, px_str("u")))) {
                LXValue _v1286 = px_add(_v1283, px_int(3LL));
                LXValue _v1287 = px_str("");
                while (px_is_truthy(({ LXValue _t1288 = px_lt(_v1286, px_call(px_get_global("len"), (LXValue[]){_v1281}, 1)); px_is_truthy(_t1288) ? px_ne(px_index(_v1281, _v1286), px_str("}")) : _t1288; }))) {
                     _v1287 = px_add(_v1287, px_index(_v1281, _v1286));
                     _v1286 = px_add(_v1286, px_int(1LL));
                }
                 _v1282 = px_add(_v1282, px_call(px_get_global("hex_to_char"), (LXValue[]){_v1287}, 1));
                 _v1283 = px_add(_v1286, px_int(1LL));
            }
            else {
                 _v1282 = px_add(_v1282, _v1285);
                 _v1283 = px_add(_v1283, px_int(2LL));
            }
        }
        else {
             _v1282 = px_add(_v1282, _v1284);
             _v1283 = px_add(_v1283, px_int(1LL));
        }
    }
    return _v1282;
px_err_1280:
    if (px_err_1280_proped) return px_err_1280_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1289 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1290 = px_null();
    LXValue _v1291 = px_null();
    LXValue px_err_1292_val = px_null();
    int px_err_1292_proped = 0;
    LXValue _v1293 = px_str("");
    LXValue _v1294 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1294, px_call(px_get_global("len"), (LXValue[]){_v1289}, 1)))) {
        LXValue _v1295 = px_index(_v1289, _v1294);
        if (px_is_truthy(px_eq(_v1295, px_str("\\")))) {
             _v1293 = px_add(_v1293, px_str("\\\\"));
        }
        else if (px_is_truthy(px_eq(_v1295, px_str("\"")))) {
             _v1293 = px_add(_v1293, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v1295, px_str("\n")))) {
             _v1293 = px_add(_v1293, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v1295, px_str("\r")))) {
             _v1293 = px_add(_v1293, px_str("\\r"));
        }
        else if (px_is_truthy(px_eq(_v1295, px_str("\t")))) {
             _v1293 = px_add(_v1293, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v1295, px_str("")))) {
             _v1293 = px_add(_v1293, px_str(""));
        }
        else {
             _v1293 = px_add(_v1293, _v1295);
        }
         _v1294 = px_add(_v1294, px_int(1LL));
    }
    return _v1293;
px_err_1292:
    if (px_err_1292_proped) return px_err_1292_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1296 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1297 = px_null();
    LXValue _v1298 = px_null();
    LXValue px_err_1299_val = px_null();
    int px_err_1299_proped = 0;
    LXValue _v1300 = px_str("");
    LXValue _v1301 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1301, _v1296))) {
         _v1300 = px_add(_v1300, px_str("0"));
         _v1301 = px_add(_v1301, px_int(1LL));
    }
    return _v1300;
px_err_1299:
    if (px_err_1299_proped) return px_err_1299_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1302 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1303 = px_null();
    LXValue _v1304 = px_null();
    LXValue _v1305 = px_null();
    LXValue _v1306 = px_null();
    LXValue _v1307 = px_null();
    LXValue _v1308 = px_null();
    LXValue _v1309 = px_null();
    LXValue _v1310 = px_null();
    LXValue _v1311 = px_null();
    LXValue _v1312 = px_null();
    LXValue _v1313 = px_null();
    LXValue px_err_1314_val = px_null();
    int px_err_1314_proped = 0;
    LXValue _v1315 = px_neg(px_int(1LL));
    LXValue _v1316 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1316, px_call(px_get_global("len"), (LXValue[]){_v1302}, 1)))) {
        if (px_is_truthy(({ LXValue _t1317 = px_eq(px_index(_v1302, _v1316), px_str("e")); px_is_truthy(_t1317) ? _t1317 : px_eq(px_index(_v1302, _v1316), px_str("E")); }))) {
             _v1315 = _v1316;
            break;
        }
         _v1316 = px_add(_v1316, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1315, px_int(0LL)))) {
        return _v1302;
    }
    LXValue _v1318 = px_slice(_v1302, px_int(0LL), _v1315, px_null());
    LXValue _v1319 = px_slice(_v1302, px_add(_v1315, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1302}, 1), px_null());
    LXValue _v1320 = px_int(1LL);
    if (px_is_truthy(({ LXValue _t1321 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1319}, 1), px_int(0LL)); px_is_truthy(_t1321) ? px_eq(px_index(_v1319, px_int(0LL)), px_str("+")) : _t1321; }))) {
         _v1319 = px_slice(_v1319, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1319}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1322 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1319}, 1), px_int(0LL)); px_is_truthy(_t1322) ? px_eq(px_index(_v1319, px_int(0LL)), px_str("-")) : _t1322; }))) {
         _v1320 = px_neg(px_int(1LL));
         _v1319 = px_slice(_v1319, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1319}, 1), px_null());
    }
    LXValue _v1323 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1319}, 1), _v1320);
    LXValue _v1324 = px_bool(false);
    if (px_is_truthy(({ LXValue _t1325 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1318}, 1), px_int(0LL)); px_is_truthy(_t1325) ? px_eq(px_index(_v1318, px_int(0LL)), px_str("-")) : _t1325; }))) {
         _v1324 = px_bool(true);
         _v1318 = px_slice(_v1318, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1318}, 1), px_null());
    }
    LXValue _v1326 = px_str("");
    LXValue _v1327 = px_str("");
    LXValue _v1328 = px_neg(px_int(1LL));
    LXValue _v1329 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1329, px_call(px_get_global("len"), (LXValue[]){_v1318}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v1318, _v1329), px_str(".")))) {
             _v1328 = _v1329;
            break;
        }
         _v1329 = px_add(_v1329, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1328, px_int(0LL)))) {
         _v1326 = _v1318;
    }
    else {
         _v1326 = px_slice(_v1318, px_int(0LL), _v1328, px_null());
         _v1327 = px_slice(_v1318, px_add(_v1328, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1318}, 1), px_null());
    }
    LXValue _v1330 = px_add(_v1326, _v1327);
    LXValue _v1331 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1326}, 1), _v1323);
    LXValue _v1332 = px_str("");
    if (px_is_truthy(px_le(_v1331, px_int(0LL)))) {
         _v1332 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1331)}, 1)), _v1330);
    }
    else if (px_is_truthy(px_ge(_v1331, px_call(px_get_global("len"), (LXValue[]){_v1330}, 1)))) {
         _v1332 = px_add(_v1330, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1331, px_call(px_get_global("len"), (LXValue[]){_v1330}, 1))}, 1));
    }
    else {
         _v1332 = px_add(px_add(px_slice(_v1330, px_int(0LL), _v1331, px_null()), px_str(".")), px_slice(_v1330, _v1331, px_call(px_get_global("len"), (LXValue[]){_v1330}, 1), px_null()));
    }
    if (px_is_truthy(_v1324)) {
        return px_add(px_str("-"), _v1332);
    }
    return _v1332;
px_err_1314:
    if (px_err_1314_proped) return px_err_1314_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1333 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1334 = px_null();
    LXValue px_err_1335_val = px_null();
    int px_err_1335_proped = 0;
    LXValue _v1336 = px_call(px_get_global("str"), (LXValue[]){_v1333}, 1);
    if (px_is_truthy(({ LXValue _t1338 = ({ LXValue _t1337 = px_eq(_v1336, px_str("inf")); px_is_truthy(_t1337) ? _t1337 : px_eq(_v1336, px_str("-inf")); }); px_is_truthy(_t1338) ? _t1338 : px_eq(_v1336, px_str("nan")); }))) {
        return _v1336;
    }
     _v1336 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1336}, 1);
    LXValue _v1339 = px_call(px_get_global("len"), (LXValue[]){_v1336}, 1);
    if (px_is_truthy(({ LXValue _t1340 = px_ge(_v1339, px_int(2LL)); px_is_truthy(_t1340) ? px_eq(px_slice(_v1336, px_sub(_v1339, px_int(2LL)), _v1339, px_null()), px_str(".0")) : _t1340; }))) {
        return px_slice(_v1336, px_int(0LL), px_sub(_v1339, px_int(2LL)), px_null());
    }
    return _v1336;
px_err_1335:
    if (px_err_1335_proped) return px_err_1335_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1341 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1342 = px_null();
    LXValue _v1343 = px_null();
    LXValue _v1344 = px_null();
    LXValue _v1345 = px_null();
    LXValue px_err_1346_val = px_null();
    int px_err_1346_proped = 0;
    LXValue _v1347 = px_index(_v1341, px_int(1LL));
    LXValue _v1348 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1348, px_call(px_get_global("len"), (LXValue[]){_v1347}, 1)))) {
        LXValue _v1349 = px_index(_v1347, _v1348);
        LXValue _v1350 = px_index(_v1349, px_int(0LL));
        if (px_is_truthy(px_eq(_v1350, px_str("StructDef")))) {
            LXValue _v1351 = px_list_n((LXValue[]){}, 0);
            LXValue _v1352 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1352, px_call(px_get_global("len"), (LXValue[]){px_index(_v1349, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1351, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1349, px_int(2LL)), _v1352), px_int(1LL))}, 1)}, 1));
                 _v1352 = px_add(_v1352, px_int(1LL));
            }
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1349, px_int(1LL))}, 1), _v1351);
        }
        else if (px_is_truthy(px_eq(_v1350, px_str("EnumDef")))) {
            LXValue _v1353 = px_list_n((LXValue[]){}, 0);
            LXValue _v1354 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1354, px_call(px_get_global("len"), (LXValue[]){px_index(_v1349, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1353, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1349, px_int(2LL)), _v1354), px_int(1LL))}, 1)}, 1));
                 _v1354 = px_add(_v1354, px_int(1LL));
            }
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1349, px_int(1LL))}, 1), _v1353);
        }
        else if (px_is_truthy(px_eq(_v1350, px_str("ImplDef")))) {
            LXValue _v1355 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1349, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1355}, 1))) {
                LXValue _v1356 = px_int(0LL);
                while (px_is_truthy(px_lt(_v1356, px_call(px_get_global("len"), (LXValue[]){px_index(_v1349, px_int(3LL))}, 1)))) {
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1355), "append", (LXValue[]){px_index(px_index(_v1349, px_int(3LL)), _v1356)}, 1));
                     _v1356 = px_add(_v1356, px_int(1LL));
                }
            }
            else {
                px_index_set(px_get_global("cg_impls"), _v1355, px_index(_v1349, px_int(3LL)));
            }
        }
         _v1348 = px_add(_v1348, px_int(1LL));
    }
px_err_1346:
    if (px_err_1346_proped) return px_err_1346_val;
    return px_null();
}

static LXValue fn_cg_collect_assign_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1357 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1358 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1359 = px_null();
    LXValue _v1360 = px_null();
    LXValue px_err_1361_val = px_null();
    int px_err_1361_proped = 0;
    LXValue _v1362 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1362, px_call(px_get_global("len"), (LXValue[]){_v1357}, 1)))) {
        LXValue _v1363 = px_index(_v1357, _v1362);
        LXValue _v1364 = px_index(_v1363, px_int(0LL));
        if (px_is_truthy(px_eq(_v1364, px_str("Assign")))) {
            LXValue _v1365 = px_index(_v1363, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1365, px_int(0LL)), px_str("Var")))) {
                LXValue _v1366 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1365, px_int(1LL))}, 1);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1358, _v1366}, 2)))) {
                    (void)(px_method(_v1358, "append", (LXValue[]){_v1366}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1364, px_str("If")))) {
            LXValue _v1367 = px_index(_v1363, px_int(1LL));
            LXValue _v1368 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1368, px_call(px_get_global("len"), (LXValue[]){_v1367}, 1)))) {
                (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(px_index(_v1367, _v1368), px_int(1LL)), _v1358}, 2));
                 _v1368 = px_add(_v1368, px_int(1LL));
            }
            if (px_is_truthy(px_ne(px_index(_v1363, px_int(2LL)), px_null()))) {
                (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1363, px_int(2LL)), _v1358}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1364, px_str("For")))) {
            (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1363, px_int(3LL)), _v1358}, 2));
        }
        else if (px_is_truthy(px_eq(_v1364, px_str("While")))) {
            (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1363, px_int(2LL)), _v1358}, 2));
        }
         _v1362 = px_add(_v1362, px_int(1LL));
    }
px_err_1361:
    if (px_err_1361_proped) return px_err_1361_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1369 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1370_val = px_null();
    int px_err_1370_proped = 0;
    LXValue _v1371 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1369, px_int(1LL))}, 1)}, 1));
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1369, _v1371}, 2);
px_err_1370:
    if (px_err_1370_proped) return px_err_1370_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1372 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1373 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1374 = px_null();
    LXValue _v1375 = px_null();
    LXValue _v1376 = px_null();
    LXValue _v1377 = px_null();
    LXValue _v1378 = px_null();
    LXValue px_err_1379_val = px_null();
    int px_err_1379_proped = 0;
    LXValue _v1380 = px_add(px_add(px_str("static LXValue "), _v1373), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1380 = px_add(_v1380, px_str("    (void)ctx;\n"));
    LXValue _v1381 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v1382 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    LXValue _v1383 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1383}, 1));
    LXValue _v1384 = px_index(_v1372, px_int(2LL));
    LXValue _v1385 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1385, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
        LXValue _v1386 = px_index(_v1384, _v1385);
        LXValue _v1387 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(1LL))}, 1)}, 1);
        LXValue _v1388 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v1386, px_int(3LL)), px_null()))) {
             _v1388 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1386, px_int(3LL))}, 1);
        }
         _v1380 = px_add(_v1380, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1387), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1385}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1385}, 1)), px_str("] : ")), _v1388), px_str(";\n")));
         _v1385 = px_add(_v1385, px_int(1LL));
    }
    LXValue _v1389 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1372, px_int(4LL)), _v1389}, 2));
    LXValue _v1390 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1390, px_call(px_get_global("len"), (LXValue[]){_v1389}, 1)))) {
        LXValue _v1391 = px_index(_v1389, _v1390);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1391}, 1), px_null()))) {
             _v1390 = px_add(_v1390, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1391}, 2))) {
             _v1390 = px_add(_v1390, px_int(1LL));
            continue;
        }
        LXValue _v1392 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1391}, 1);
         _v1380 = px_add(_v1380, px_add(px_add(px_str("    LXValue "), _v1392), px_str(" = px_null();\n")));
         _v1390 = px_add(_v1390, px_int(1LL));
    }
    LXValue _v1393 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1393}, 1));
     _v1380 = px_add(_v1380, px_add(px_add(px_str("    LXValue "), _v1393), px_str("_val = px_null();\n")));
     _v1380 = px_add(_v1380, px_add(px_add(px_str("    int "), _v1393), px_str("_proped = 0;\n")));
    LXValue _v1394 = px_index(_v1372, px_int(4LL));
    LXValue _v1395 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1395, px_call(px_get_global("len"), (LXValue[]){_v1394}, 1)))) {
         _v1380 = px_add(_v1380, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1394, _v1395), px_int(1LL)}, 2));
         _v1395 = px_add(_v1395, px_int(1LL));
    }
     _v1380 = px_add(_v1380, px_add(_v1393, px_str(":\n")));
     _v1380 = px_add(_v1380, px_add(px_add(px_add(px_add(px_str("    if ("), _v1393), px_str("_proped) return ")), _v1393), px_str("_val;\n")));
     _v1380 = px_add(_v1380, px_str("    return px_null();\n"));
     _v1380 = px_add(_v1380, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_vars", _v1381);
    px_set_global("cg_var_types", _v1382);
    px_set_global("cg_immutables", _v1383);
    return _v1380;
px_err_1379:
    if (px_err_1379_proped) return px_err_1379_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1396 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1397 = px_null();
    LXValue _v1398 = px_null();
    LXValue _v1399 = px_null();
    LXValue _v1400 = px_null();
    LXValue _v1401 = px_null();
    LXValue _v1402 = px_null();
    LXValue _v1403 = px_null();
    LXValue _v1404 = px_null();
    LXValue _v1405 = px_null();
    LXValue _v1406 = px_null();
    LXValue _v1407 = px_null();
    LXValue _v1408 = px_null();
    LXValue _v1409 = px_null();
    LXValue _v1410 = px_null();
    LXValue px_err_1411_val = px_null();
    int px_err_1411_proped = 0;
    LXValue _v1412 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_set_global("cg_closures", px_str(""));
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_uidc", px_int(0LL));
    px_set_global("cg_closure_id", px_int(0LL));
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1396}, 1));
    LXValue _v1413 = px_index(_v1396, px_int(1LL));
    LXValue _v1414 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1414, px_call(px_get_global("len"), (LXValue[]){_v1413}, 1)))) {
        LXValue _v1415 = px_index(_v1413, _v1414);
        LXValue _v1416 = px_index(_v1415, px_int(0LL));
        if (px_is_truthy(px_eq(_v1416, px_str("FuncDef")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1415, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1416, px_str("ExternDef")))) {
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1415, px_int(1LL))}, 1), px_index(_v1415, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1416, px_str("VarDecl")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1415, px_int(2LL))}, 1)}, 1));
            if (px_is_truthy(({ LXValue _t1417 = px_eq(px_index(_v1415, px_int(1LL)), px_str("Let")); px_is_truthy(_t1417) ? _t1417 : px_eq(px_index(_v1415, px_int(1LL)), px_str("Const")); }))) {
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1415, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1416, px_str("Assign")))) {
            LXValue _v1418 = px_index(_v1415, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1418, px_int(0LL)), px_str("Var")))) {
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1418, px_int(1LL))}, 1)}, 1));
            }
        }
         _v1414 = px_add(_v1414, px_int(1LL));
    }
    LXValue _v1419 = px_list_n((LXValue[]){}, 0);
    LXValue _v1420 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    LXValue _v1421 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1421, px_call(px_get_global("len"), (LXValue[]){_v1420}, 1)))) {
        LXValue _v1422 = px_index(_v1420, _v1421);
        LXValue _v1423 = px_index(px_get_global("cg_impls"), _v1422);
        LXValue _v1424 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1424, px_call(px_get_global("len"), (LXValue[]){_v1423}, 1)))) {
            (void)(px_method(_v1419, "append", (LXValue[]){px_list_n((LXValue[]){_v1422, px_index(_v1423, _v1424)}, 2)}, 1));
             _v1424 = px_add(_v1424, px_int(1LL));
        }
         _v1421 = px_add(_v1421, px_int(1LL));
    }
    LXValue _v1425 = px_int(1LL);
    while (px_is_truthy(px_lt(_v1425, px_call(px_get_global("len"), (LXValue[]){_v1419}, 1)))) {
        LXValue _v1426 = _v1425;
        while (px_is_truthy(px_gt(_v1426, px_int(0LL)))) {
            LXValue _v1427 = px_add(px_add(px_index(px_index(_v1419, px_sub(_v1426, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1419, px_sub(_v1426, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            LXValue _v1428 = px_add(px_add(px_index(px_index(_v1419, _v1426), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1419, _v1426), px_int(1LL)), px_int(1LL))}, 1));
            if (px_is_truthy(px_lt(_v1428, _v1427))) {
                LXValue _v1429 = px_index(_v1419, _v1426);
                px_index_set(_v1419, _v1426, px_index(_v1419, px_sub(_v1426, px_int(1LL))));
                px_index_set(_v1419, px_sub(_v1426, px_int(1LL)), _v1429);
            }
             _v1426 = px_sub(_v1426, px_int(1LL));
        }
         _v1425 = px_add(_v1425, px_int(1LL));
    }
    LXValue _v1430 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1430, px_call(px_get_global("len"), (LXValue[]){_v1419}, 1)))) {
        LXValue _v1431 = px_index(px_index(_v1419, _v1430), px_int(0LL));
        LXValue _v1432 = px_index(px_index(_v1419, _v1430), px_int(1LL));
        LXValue _v1433 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1431}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1432, px_int(1LL))}, 1)}, 1));
         _v1412 = px_add(_v1412, px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1432, _v1433}, 2));
         _v1412 = px_add(_v1412, px_str("\n"));
         _v1430 = px_add(_v1430, px_int(1LL));
    }
    LXValue _v1434 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1434, px_call(px_get_global("len"), (LXValue[]){_v1413}, 1)))) {
        LXValue _v1435 = px_index(_v1413, _v1434);
        if (px_is_truthy(px_eq(px_index(_v1435, px_int(0LL)), px_str("FuncDef")))) {
             _v1412 = px_add(_v1412, px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1435}, 1));
             _v1412 = px_add(_v1412, px_str("\n"));
        }
         _v1434 = px_add(_v1434, px_int(1LL));
    }
     _v1412 = px_add(_v1412, px_str("int main(int argc, char** argv) {\n"));
     _v1412 = px_add(_v1412, px_str("    px_args_init(argc, argv);\n"));
     _v1412 = px_add(_v1412, px_str("    px_register_builtins();\n"));
    LXValue _v1436 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1436, px_call(px_get_global("len"), (LXValue[]){_v1413}, 1)))) {
        LXValue _v1437 = px_index(_v1413, _v1436);
        if (px_is_truthy(px_eq(px_index(_v1437, px_int(0LL)), px_str("FuncDef")))) {
            LXValue _v1438 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1437, px_int(1LL))}, 1);
            LXValue _v1439 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1438}, 1));
             _v1412 = px_add(_v1412, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1438), px_str("\", px_func(\"")), _v1438), px_str("\", ")), _v1439), px_str(", NULL));\n")));
        }
         _v1436 = px_add(_v1436, px_int(1LL));
    }
    LXValue _v1440 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1440, px_call(px_get_global("len"), (LXValue[]){_v1419}, 1)))) {
        LXValue _v1441 = px_index(px_index(_v1419, _v1440), px_int(0LL));
        LXValue _v1442 = px_index(px_index(_v1419, _v1440), px_int(1LL));
        LXValue _v1443 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1442, px_int(1LL))}, 1);
        LXValue _v1444 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1441}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1443}, 1));
         _v1412 = px_add(_v1412, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1441), px_str(".")), _v1443), px_str("\", px_func(\"")), _v1441), px_str(".")), _v1443), px_str("\", ")), _v1444), px_str(", NULL));\n")));
         _v1440 = px_add(_v1440, px_int(1LL));
    }
    LXValue _v1445 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1445, px_call(px_get_global("len"), (LXValue[]){_v1413}, 1)))) {
        LXValue _v1446 = px_index(_v1413, _v1445);
        LXValue _v1447 = px_index(_v1446, px_int(0LL));
        if (px_is_truthy(({ LXValue _t1453 = ({ LXValue _t1452 = ({ LXValue _t1451 = ({ LXValue _t1450 = ({ LXValue _t1449 = ({ LXValue _t1448 = px_ne(_v1447, px_str("FuncDef")); px_is_truthy(_t1448) ? px_ne(_v1447, px_str("StructDef")) : _t1448; }); px_is_truthy(_t1449) ? px_ne(_v1447, px_str("EnumDef")) : _t1449; }); px_is_truthy(_t1450) ? px_ne(_v1447, px_str("TraitDef")) : _t1450; }); px_is_truthy(_t1451) ? px_ne(_v1447, px_str("ImplDef")) : _t1451; }); px_is_truthy(_t1452) ? px_ne(_v1447, px_str("Import")) : _t1452; }); px_is_truthy(_t1453) ? px_ne(_v1447, px_str("ExternDef")) : _t1453; }))) {
             _v1412 = px_add(_v1412, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1446, px_int(1LL)}, 2));
        }
         _v1445 = px_add(_v1445, px_int(1LL));
    }
    LXValue _v1454 = px_bool(false);
    LXValue _v1455 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1455, px_call(px_get_global("len"), (LXValue[]){_v1413}, 1)))) {
        LXValue _v1456 = px_index(_v1413, _v1455);
        if (px_is_truthy(({ LXValue _t1457 = px_eq(px_index(_v1456, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1457) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1456, px_int(1LL))}, 1), px_str("main")) : _t1457; }))) {
             _v1454 = px_bool(true);
            break;
        }
         _v1455 = px_add(_v1455, px_int(1LL));
    }
    if (px_is_truthy(_v1454)) {
        LXValue _v1458 = px_str("fn_main");
         _v1412 = px_add(_v1412, px_add(px_add(px_str("    { LXValue _r = "), _v1458), px_str("(NULL, 0, NULL); int _code = 0;\n")));
         _v1412 = px_add(_v1412, px_str("      if (px_is_result(_r)) {\n"));
         _v1412 = px_add(_v1412, px_str("        if (!px_result_ok(_r)) {\n"));
         _v1412 = px_add(_v1412, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
         _v1412 = px_add(_v1412, px_str("          _code = 1;\n"));
         _v1412 = px_add(_v1412, px_str("        } else {\n"));
         _v1412 = px_add(_v1412, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
         _v1412 = px_add(_v1412, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
         _v1412 = px_add(_v1412, px_str("        }\n"));
         _v1412 = px_add(_v1412, px_str("      } else if (_r.type == PX_INT) {\n"));
         _v1412 = px_add(_v1412, px_str("        _code = (int)_r.as.i;\n"));
         _v1412 = px_add(_v1412, px_str("      }\n"));
         _v1412 = px_add(_v1412, px_str("      return _code;\n"));
         _v1412 = px_add(_v1412, px_str("    }\n"));
    }
    else {
         _v1412 = px_add(_v1412, px_str("    return 0;\n"));
    }
     _v1412 = px_add(_v1412, px_str("}\n"));
    LXValue _v1459 = px_call(px_get_global("cg_find"), (LXValue[]){_v1412, px_str("int main(")}, 2);
    if (px_is_truthy(px_ge(_v1459, px_int(0LL)))) {
        LXValue _v1460 = px_slice(_v1412, px_int(0LL), _v1459, px_null());
        LXValue _v1461 = px_slice(_v1412, _v1459, px_call(px_get_global("len"), (LXValue[]){_v1412}, 1), px_null());
        LXValue _v1462 = px_call(px_get_global("cg_find"), (LXValue[]){_v1460, px_str("static LXValue")}, 2);
        LXValue _v1463 = px_str("");
        if (px_is_truthy(px_ge(_v1462, px_int(0LL)))) {
             _v1463 = px_add(px_add(px_add(px_add(px_slice(_v1460, px_int(0LL), _v1462, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1460, _v1462, px_call(px_get_global("len"), (LXValue[]){_v1460}, 1), px_null())), _v1461);
        }
        else {
             _v1463 = px_add(px_add(px_add(_v1460, px_get_global("cg_closures")), px_str("\n")), _v1461);
        }
        return _v1463;
    }
    return _v1412;
px_err_1411:
    if (px_err_1411_proped) return px_err_1411_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1464 = px_null();
    LXValue px_err_1465_val = px_null();
    int px_err_1465_proped = 0;
    LXValue _v1466 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    LXValue _v1467 = px_index(_v1466, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1466}, 1), px_int(1LL)));
    LXValue _v1468 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1467}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v1467}, 1)}, 1));
    px_set_global("p_pos", px_int(0LL));
    LXValue _v1469 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    LXValue _v1470 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v1469, _v1468}, 2);
    LXValue _v1471 = px_call(px_get_global("cg_generate"), (LXValue[]){_v1470}, 1);
    LXValue _v1472 = px_call(px_get_global("len"), (LXValue[]){_v1471}, 1);
    if (px_is_truthy(({ LXValue _t1473 = px_gt(_v1472, px_int(0LL)); px_is_truthy(_t1473) ? px_eq(px_index(_v1471, px_sub(_v1472, px_int(1LL))), px_str("\n")) : _t1473; }))) {
         _v1471 = px_slice(_v1471, px_int(0LL), px_sub(_v1472, px_int(1LL)), px_null());
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1471}, 1));
px_err_1465:
    if (px_err_1465_proped) return px_err_1465_val;
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
