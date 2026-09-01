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
    LXValue _v375 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture")}, 35);
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

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v444 = px_null();
    LXValue px_err_445_val = px_null();
    int px_err_445_proped = 0;
    LXValue _v446 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v447 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    LXValue _v448 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v449 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v449 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    LXValue _v450 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v447}, 1), _v448, _v449, _v450, _v446}, 6);
px_err_445:
    if (px_err_445_proped) return px_err_445_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_451_val = px_null();
    int px_err_451_proped = 0;
    LXValue _v452 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v453 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
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
        LXValue _v457 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v458 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_method(_v454, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v457}, 1), _v458, _v456}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t459 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t459) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t459; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v453}, 1), _v454, _v452}, 4);
px_err_451:
    if (px_err_451_proped) return px_err_451_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_460_val = px_null();
    int px_err_460_proped = 0;
    LXValue _v461 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v462 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v463 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t464 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t464) ? _t464 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v465 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v466 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        LXValue _v467 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v467, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        (void)(px_method(_v463, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v466}, 1), _v467, _v465}, 4)}, 1));
        if (px_is_truthy(({ LXValue _t468 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t468) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t468; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v462}, 1), _v463, _v461}, 4);
px_err_460:
    if (px_err_460_proped) return px_err_460_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v469 = px_null();
    LXValue _v470 = px_null();
    LXValue px_err_471_val = px_null();
    int px_err_471_proped = 0;
    LXValue _v472 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v473 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v474 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t475 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t475) ? _t475 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v476 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v477 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        LXValue _v478 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        LXValue _v479 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v479 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v480 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                 _v480 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        (void)(px_method(_v474, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v477}, 1), _v478, _v479, _v480, _v476}, 6)}, 1));
        if (px_is_truthy(({ LXValue _t481 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t481) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t481; }))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v473}, 1), _v474, _v472}, 4);
px_err_471:
    if (px_err_471_proped) return px_err_471_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v482 = px_null();
    LXValue _v483 = px_null();
    LXValue _v484 = px_null();
    LXValue px_err_485_val = px_null();
    int px_err_485_proped = 0;
    LXValue _v486 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v487 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    LXValue _v488 = px_null();
    LXValue _v489 = _v487;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v489 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
         _v488 = px_call(px_get_global("qstr"), (LXValue[]){_v487}, 1);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v490 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t491 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t491) ? _t491 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        LXValue _v492 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v493 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        LXValue _v494 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        LXValue _v495 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v495 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v496 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        (void)(px_method(_v490, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v493}, 1), _v494, _v495, _v496, _v492}, 6)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v489}, 1), _v488, _v490, _v486}, 5);
px_err_485:
    if (px_err_485_proped) return px_err_485_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_497_val = px_null();
    int px_err_497_proped = 0;
    LXValue _v498 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        LXValue _v499 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v499}, 1), px_list_n((LXValue[]){}, 0), _v498}, 4);
    }
    LXValue _v500 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v500, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v500, px_list_n((LXValue[]){}, 0), _v498}, 4);
px_err_497:
    if (px_err_497_proped) return px_err_497_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_501_val = px_null();
    int px_err_501_proped = 0;
    LXValue _v502 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v503 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v503, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    LXValue _v504 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_method(_v504, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            continue;
        }
        break;
    }
    return px_list_n((LXValue[]){px_str("Import"), _v503, _v504, _v502}, 4);
px_err_501:
    if (px_err_501_proped) return px_err_501_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v505 = px_null();
    LXValue _v506 = px_null();
    LXValue px_err_507_val = px_null();
    int px_err_507_proped = 0;
    LXValue _v508 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v509 = px_list_n((LXValue[]){}, 0);
    LXValue _v510 = px_null();
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t511 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t511) ? _t511 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        if (px_is_truthy(({ LXValue _t512 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t512) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t512; }))) {
            LXValue _v513 = px_get_global("p_pos");
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                 _v510 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                continue;
            }
            else {
                px_set_global("p_pos", _v513);
            }
        }
        LXValue _v514 = px_null();
        if (px_is_truthy(({ LXValue _t515 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t515) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t515; }))) {
             _v514 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        LXValue _v516 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v517 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        (void)(px_method(_v509, "append", (LXValue[]){px_list_n((LXValue[]){_v514, _v516, _v517}, 3)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Select"), _v509, _v510, _v508}, 4);
px_err_507:
    if (px_err_507_proped) return px_err_507_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_518_val = px_null();
    int px_err_518_proped = 0;
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    LXValue _v519 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    LXValue _v520 = px_call(px_get_global("node_pos"), (LXValue[]){_v519}, 1);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v519, _v520}, 3)}, 1);
px_err_518:
    if (px_err_518_proped) return px_err_518_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v521 = px_null();
    LXValue _v522 = px_null();
    LXValue px_err_523_val = px_null();
    int px_err_523_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    LXValue _v524 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            LXValue _v525 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            LXValue _v526 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            LXValue _v527 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v527 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            LXValue _v528 = px_null();
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                 _v528 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            (void)(px_method(_v524, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v526}, 1), _v527, _v528, _v525}, 5)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v524;
px_err_523:
    if (px_err_523_proped) return px_err_523_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_529_val = px_null();
    int px_err_529_proped = 0;
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_529:
    if (px_err_529_proped) return px_err_529_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v530 = px_null();
    LXValue px_err_531_val = px_null();
    int px_err_531_proped = 0;
    LXValue _v532 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|>")}, 1))) {
        LXValue _v533 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v534 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
         _v532 = px_list_n((LXValue[]){px_str("Pipe"), _v532, _v534, _v533}, 4);
    }
    return _v532;
px_err_531:
    if (px_err_531_proped) return px_err_531_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v535 = px_null();
    LXValue px_err_536_val = px_null();
    int px_err_536_proped = 0;
    LXValue _v537 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("??")}, 1))) {
        LXValue _v538 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v539 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
         _v537 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v537, _v539, _v538}, 4);
    }
    return _v537;
px_err_536:
    if (px_err_536_proped) return px_err_536_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v540 = px_null();
    LXValue px_err_541_val = px_null();
    int px_err_541_proped = 0;
    LXValue _v542 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("or")}, 1))) {
        LXValue _v543 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v544 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
         _v542 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v542, _v544, _v543}, 5);
    }
    return _v542;
px_err_541:
    if (px_err_541_proped) return px_err_541_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v545 = px_null();
    LXValue px_err_546_val = px_null();
    int px_err_546_proped = 0;
    LXValue _v547 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("and")}, 1))) {
        LXValue _v548 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v549 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
         _v547 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v547, _v549, _v548}, 5);
    }
    return _v547;
px_err_546:
    if (px_err_546_proped) return px_err_546_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v550 = px_null();
    LXValue _v551 = px_null();
    LXValue px_err_552_val = px_null();
    int px_err_552_proped = 0;
    LXValue _v553 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v554 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("==")}, 1))) {
             _v554 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("!=")}, 1))) {
             _v554 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<")}, 1))) {
             _v554 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<=")}, 1))) {
             _v554 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">")}, 1))) {
             _v554 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">=")}, 1))) {
             _v554 = px_str("Ge");
        }
        if (px_is_truthy(px_eq(_v554, px_null()))) {
            break;
        }
        LXValue _v555 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v556 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
         _v553 = px_list_n((LXValue[]){px_str("Binary"), _v554, _v553, _v556, _v555}, 5);
    }
    return _v553;
px_err_552:
    if (px_err_552_proped) return px_err_552_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v557 = px_null();
    LXValue px_err_558_val = px_null();
    int px_err_558_proped = 0;
    LXValue _v559 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|")}, 1))) {
        LXValue _v560 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v561 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
         _v559 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v559, _v561, _v560}, 5);
    }
    return _v559;
px_err_558:
    if (px_err_558_proped) return px_err_558_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v562 = px_null();
    LXValue px_err_563_val = px_null();
    int px_err_563_proped = 0;
    LXValue _v564 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("^")}, 1))) {
        LXValue _v565 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v566 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
         _v564 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v564, _v566, _v565}, 5);
    }
    return _v564;
px_err_563:
    if (px_err_563_proped) return px_err_563_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v567 = px_null();
    LXValue px_err_568_val = px_null();
    int px_err_568_proped = 0;
    LXValue _v569 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("&")}, 1))) {
        LXValue _v570 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v571 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
         _v569 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v569, _v571, _v570}, 5);
    }
    return _v569;
px_err_568:
    if (px_err_568_proped) return px_err_568_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v572 = px_null();
    LXValue _v573 = px_null();
    LXValue px_err_574_val = px_null();
    int px_err_574_proped = 0;
    LXValue _v575 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v576 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<<")}, 1))) {
             _v576 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>")}, 1))) {
             _v576 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>>")}, 1))) {
             _v576 = px_str("ShrU");
        }
        if (px_is_truthy(px_eq(_v576, px_null()))) {
            break;
        }
        LXValue _v577 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v578 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
         _v575 = px_list_n((LXValue[]){px_str("Binary"), _v576, _v575, _v578, _v577}, 5);
    }
    return _v575;
px_err_574:
    if (px_err_574_proped) return px_err_574_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v579 = px_null();
    LXValue _v580 = px_null();
    LXValue px_err_581_val = px_null();
    int px_err_581_proped = 0;
    LXValue _v582 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v583 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("+")}, 1))) {
             _v583 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("-")}, 1))) {
             _v583 = px_str("Sub");
        }
        if (px_is_truthy(px_eq(_v583, px_null()))) {
            break;
        }
        LXValue _v584 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v585 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
         _v582 = px_list_n((LXValue[]){px_str("Binary"), _v583, _v582, _v585, _v584}, 5);
    }
    return _v582;
px_err_581:
    if (px_err_581_proped) return px_err_581_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v586 = px_null();
    LXValue _v587 = px_null();
    LXValue px_err_588_val = px_null();
    int px_err_588_proped = 0;
    LXValue _v589 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v590 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("*")}, 1))) {
             _v590 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("/")}, 1))) {
             _v590 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("//")}, 1))) {
             _v590 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("%")}, 1))) {
             _v590 = px_str("Mod");
        }
        if (px_is_truthy(px_eq(_v590, px_null()))) {
            break;
        }
        LXValue _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v592 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
         _v589 = px_list_n((LXValue[]){px_str("Binary"), _v590, _v589, _v592, _v591}, 5);
    }
    return _v589;
px_err_588:
    if (px_err_588_proped) return px_err_588_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_593_val = px_null();
    int px_err_593_proped = 0;
    LXValue _v594 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("**")}, 1))) {
        LXValue _v595 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v596 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v594, _v596, _v595}, 5);
    }
    return _v594;
px_err_593:
    if (px_err_593_proped) return px_err_593_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_597_val = px_null();
    int px_err_597_proped = 0;
    LXValue _v598 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v598, px_str("-")))) {
        LXValue _v599 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v600 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v600, _v599}, 4);
    }
    if (px_is_truthy(px_eq(_v598, px_str("not")))) {
        LXValue _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v602 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v602, _v601}, 4);
    }
    if (px_is_truthy(px_eq(_v598, px_str("~")))) {
        LXValue _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v604 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v604, _v603}, 4);
    }
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_597:
    if (px_err_597_proped) return px_err_597_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v605 = px_null();
    LXValue _v606 = px_null();
    LXValue px_err_607_val = px_null();
    int px_err_607_proped = 0;
    LXValue _v608 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        LXValue _v609 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v609, px_str("(")))) {
            LXValue _v610 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            LXValue _v611 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v608 = px_list_n((LXValue[]){px_str("Call"), _v608, _v611, _v610}, 4);
        }
        else if (px_is_truthy(px_eq(_v609, px_str("[")))) {
            LXValue _v612 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                LXValue _v613 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                LXValue _v614 = px_null();
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                     _v614 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                }
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                 _v608 = px_list_n((LXValue[]){px_str("Slice"), _v608, px_null(), _v613, _v614, _v612}, 6);
            }
            else {
                LXValue _v615 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    LXValue _v616 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    LXValue _v617 = px_null();
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                         _v617 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    }
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v608 = px_list_n((LXValue[]){px_str("Slice"), _v608, _v615, _v616, _v617, _v612}, 6);
                }
                else {
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                     _v608 = px_list_n((LXValue[]){px_str("Index"), _v608, _v615, _v612}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v609, px_str(".")))) {
            LXValue _v618 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v619 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v608 = px_list_n((LXValue[]){px_str("Field"), _v608, px_call(px_get_global("qstr"), (LXValue[]){_v619}, 1), _v618}, 4);
        }
        else if (px_is_truthy(px_eq(_v609, px_str("?.")))) {
            LXValue _v620 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v621 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
             _v608 = px_list_n((LXValue[]){px_str("OptionalField"), _v608, px_call(px_get_global("qstr"), (LXValue[]){_v621}, 1), _v620}, 4);
        }
        else if (px_is_truthy(px_eq(_v609, px_str("!")))) {
            LXValue _v622 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v608 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v608, _v622}, 3);
        }
        else if (px_is_truthy(px_eq(_v609, px_str("?")))) {
            LXValue _v623 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v608 = px_list_n((LXValue[]){px_str("Try"), _v608, _v623}, 3);
        }
        else {
            break;
        }
    }
    return _v608;
px_err_607:
    if (px_err_607_proped) return px_err_607_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_624_val = px_null();
    int px_err_624_proped = 0;
    if (px_is_truthy(({ LXValue _t625 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t625) ? _t625 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        return px_null();
    }
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_624:
    if (px_err_624_proped) return px_err_624_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_626_val = px_null();
    int px_err_626_proped = 0;
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    LXValue _v627 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v627, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v627;
px_err_626:
    if (px_err_626_proped) return px_err_626_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v628 = px_null();
    LXValue px_err_629_val = px_null();
    int px_err_629_proped = 0;
    LXValue _v630 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v630, px_str("整数")))) {
        LXValue _v631 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v632 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Int"), _v632, _v631}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("浮点")))) {
        LXValue _v633 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v634 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Float"), _v634, _v633}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("字符串")))) {
        LXValue _v635 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v636 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Str"), _v636, _v635}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("true")))) {
        LXValue _v637 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v637}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("false")))) {
        LXValue _v638 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v638}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("null")))) {
        LXValue _v639 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Null"), _v639}, 2);
    }
    if (px_is_truthy(px_eq(_v630, px_str("self")))) {
        LXValue _v640 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v640}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("标识符")))) {
        LXValue _v641 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        LXValue _v642 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v642}, 1), _v641}, 3);
    }
    if (px_is_truthy(px_eq(_v630, px_str("[")))) {
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v630, px_str("(")))) {
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v630, px_str("{")))) {
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v630, px_str("fn")))) {
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v630, px_str("match")))) {
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v630, px_str("if")))) {
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    if (px_is_truthy(px_eq(_v630, px_str("chan")))) {
        LXValue _v643 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v644 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v643}, 3);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            LXValue _v645 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
             _v644 = px_list_n((LXValue[]){px_str("Call"), _v644, _v645, _v643}, 4);
        }
        return _v644;
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_629:
    if (px_err_629_proped) return px_err_629_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_646_val = px_null();
    int px_err_646_proped = 0;
    LXValue _v647 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v647}, 3);
    }
    LXValue _v648 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        LXValue _v649 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("ListComp"), _v648, px_index(_v649, px_int(0LL)), px_index(_v649, px_int(1LL)), _v647}, 5);
    }
    LXValue _v650 = px_list_n((LXValue[]){_v648}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            break;
        }
        (void)(px_method(_v650, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    return px_list_n((LXValue[]){px_str("List"), _v650, _v647}, 3);
px_err_646:
    if (px_err_646_proped) return px_err_646_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_651_val = px_null();
    int px_err_651_proped = 0;
    LXValue _v652 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        (void)(px_method(_v652, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    return _v652;
px_err_651:
    if (px_err_651_proped) return px_err_651_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_653_val = px_null();
    int px_err_653_proped = 0;
    LXValue _v654 = px_list_n((LXValue[]){}, 0);
    LXValue _v655 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v656 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            LXValue _v657 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v654, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v656, _v657}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_method(_v655, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            break;
        }
    }
    return px_list_n((LXValue[]){_v654, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v655}, 1)}, 2);
px_err_653:
    if (px_err_653_proped) return px_err_653_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v658 = (nargs > 0) ? args[0] : px_null();
    LXValue _v659 = px_null();
    LXValue _v660 = px_null();
    LXValue px_err_661_val = px_null();
    int px_err_661_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v658}, 1), px_int(0LL)))) {
        return px_null();
    }
    LXValue _v662 = px_index(_v658, px_int(0LL));
    LXValue _v663 = px_int(1LL);
    while (px_is_truthy(px_lt(_v663, px_call(px_get_global("len"), (LXValue[]){_v658}, 1)))) {
        LXValue _v664 = px_call(px_get_global("node_pos"), (LXValue[]){_v662}, 1);
         _v662 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v662, px_index(_v658, _v663), _v664}, 5);
         _v663 = px_add(_v663, px_int(1LL));
    }
    return _v662;
px_err_661:
    if (px_err_661_proped) return px_err_661_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_665_val = px_null();
    int px_err_665_proped = 0;
    LXValue _v666 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v666}, 3);
    }
    LXValue _v667 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        LXValue _v668 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("GenExp"), _v667, px_index(_v668, px_int(0LL)), px_index(_v668, px_int(1LL)), _v666}, 5);
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        LXValue _v669 = px_list_n((LXValue[]){_v667}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                break;
            }
            (void)(px_method(_v669, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("Tuple"), _v669, _v666}, 3);
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    return _v667;
px_err_665:
    if (px_err_665_proped) return px_err_665_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v670 = px_null();
    LXValue _v671 = px_null();
    LXValue px_err_672_val = px_null();
    int px_err_672_proped = 0;
    LXValue _v673 = px_int(0LL);
    LXValue _v674 = px_get_global("p_pos");
    while (px_is_truthy(px_lt(_v674, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        LXValue _v675 = px_index(px_index(px_get_global("p_toks"), _v674), px_int(0LL));
        if (px_is_truthy(({ LXValue _t676 = px_eq(_v675, px_str(":")); px_is_truthy(_t676) ? px_eq(_v673, px_int(0LL)) : _t676; }))) {
            return px_bool(true);
        }
        if (px_is_truthy(({ LXValue _t678 = ({ LXValue _t677 = px_eq(_v675, px_str("(")); px_is_truthy(_t677) ? _t677 : px_eq(_v675, px_str("[")); }); px_is_truthy(_t678) ? _t678 : px_eq(_v675, px_str("{")); }))) {
             _v673 = px_add(_v673, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t679 = px_eq(_v675, px_str(")")); px_is_truthy(_t679) ? _t679 : px_eq(_v675, px_str("]")); }))) {
            if (px_is_truthy(px_gt(_v673, px_int(0LL)))) {
                 _v673 = px_sub(_v673, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t680 = px_eq(_v675, px_str("}")); px_is_truthy(_t680) ? px_eq(_v673, px_int(0LL)) : _t680; }))) {
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t683 = ({ LXValue _t682 = ({ LXValue _t681 = px_eq(_v675, px_str(",")); px_is_truthy(_t681) ? _t681 : px_eq(_v675, px_str("换行")); }); px_is_truthy(_t682) ? _t682 : px_eq(_v675, px_str("EOF")); }); px_is_truthy(_t683) ? px_eq(_v673, px_int(0LL)) : _t683; }))) {
            return px_bool(false);
        }
         _v674 = px_add(_v674, px_int(1LL));
    }
    return px_bool(false);
px_err_672:
    if (px_err_672_proped) return px_err_672_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_684_val = px_null();
    int px_err_684_proped = 0;
    LXValue _v685 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    LXValue _v686 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    if (px_is_truthy(_v686)) {
        LXValue _v687 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v688 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            LXValue _v689 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            return px_list_n((LXValue[]){px_str("DictComp"), _v687, _v688, px_index(_v689, px_int(0LL)), px_index(_v689, px_int(1LL)), _v685}, 6);
        }
        LXValue _v690 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v687, _v688}, 2)}, 1);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                break;
            }
            LXValue _v691 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            LXValue _v692 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            (void)(px_method(_v690, "append", (LXValue[]){px_list_n((LXValue[]){_v691, _v692}, 2)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("Dict"), _v690, _v685}, 3);
    }
    LXValue _v693 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    while (px_is_truthy(({ LXValue _t694 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t694) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t694; }))) {
        (void)(px_method(_v693, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    return px_list_n((LXValue[]){px_str("Block"), _v693, _v685}, 3);
px_err_684:
    if (px_err_684_proped) return px_err_684_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v695 = px_null();
    LXValue _v696 = px_null();
    LXValue px_err_697_val = px_null();
    int px_err_697_proped = 0;
    LXValue _v698 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v699 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    LXValue _v700 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v700 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    LXValue _v701 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        while (px_is_truthy(px_bool(true))) {
            (void)(px_method(_v701, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                continue;
            }
            break;
        }
    }
    LXValue _v702 = px_null();
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        LXValue _v703 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v704 = px_list_n((LXValue[]){}, 0);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        while (px_is_truthy(({ LXValue _t705 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t705) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t705; }))) {
            (void)(px_method(_v704, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
         _v702 = px_list_n((LXValue[]){px_str("Block"), _v704, _v703}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
         _v702 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    else {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    return px_list_n((LXValue[]){px_str("Closure"), _v699, _v700, _v702, _v701, _v698}, 6);
px_err_697:
    if (px_err_697_proped) return px_err_697_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v706 = px_null();
    LXValue _v707 = px_null();
    LXValue _v708 = px_null();
    LXValue px_err_709_val = px_null();
    int px_err_709_proped = 0;
    LXValue _v710 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v711 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    LXValue _v712 = px_list_n((LXValue[]){}, 0);
    while (px_is_truthy(px_bool(true))) {
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        if (px_is_truthy(({ LXValue _t713 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t713) ? _t713 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            break;
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        LXValue _v714 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        LXValue _v715 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
             _v715 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        LXValue _v716 = px_null();
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            LXValue _v717 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            LXValue _v718 = px_null();
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v717}, 1), px_int(0LL)))) {
                 _v718 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v717, px_int(0LL))}, 1);
            }
            else {
                 _v718 = _v710;
            }
             _v716 = px_list_n((LXValue[]){px_str("Block"), _v717, _v718}, 3);
        }
        else {
             _v716 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        (void)(px_method(_v712, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v714, _v715, _v716, _v710}, 5)}, 1));
    }
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    return px_list_n((LXValue[]){px_str("Match"), _v711, _v712, _v710}, 4);
px_err_709:
    if (px_err_709_proped) return px_err_709_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_719_val = px_null();
    int px_err_719_proped = 0;
    LXValue _v720 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    LXValue _v721 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    LXValue _v722 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    LXValue _v723 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v721, _v722, _v723, _v720}, 5);
px_err_719:
    if (px_err_719_proped) return px_err_719_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_724_val = px_null();
    int px_err_724_proped = 0;
    LXValue _v725 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t730 = ({ LXValue _t729 = ({ LXValue _t728 = ({ LXValue _t727 = ({ LXValue _t726 = px_eq(_v725, px_str("整数")); px_is_truthy(_t726) ? _t726 : px_eq(_v725, px_str("浮点")); }); px_is_truthy(_t727) ? _t727 : px_eq(_v725, px_str("字符串")); }); px_is_truthy(_t728) ? _t728 : px_eq(_v725, px_str("true")); }); px_is_truthy(_t729) ? _t729 : px_eq(_v725, px_str("false")); }); px_is_truthy(_t730) ? _t730 : px_eq(_v725, px_str("null")); }))) {
        LXValue _v731 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v731}, 2);
    }
    if (px_is_truthy(px_eq(_v725, px_str("标识符")))) {
        LXValue _v732 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        LXValue _v733 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v734 = px_list_n((LXValue[]){}, 0);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                while (px_is_truthy(px_bool(true))) {
                    (void)(px_method(_v734, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        continue;
                    }
                    break;
                }
            }
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v732}, 1), _v734}, 3);
        }
        if (px_is_truthy(px_eq(_v732, px_str("_")))) {
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v732}, 1))) {
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v732}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v732}, 1)}, 2);
    }
    if (px_is_truthy(px_eq(_v725, px_str("(")))) {
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v735 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v735, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    continue;
                }
                break;
            }
        }
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        return px_list_n((LXValue[]){px_str("PatTuple"), _v735}, 2);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_724:
    if (px_err_724_proped) return px_err_724_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v736 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_737_val = px_null();
    int px_err_737_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v736}, 1), px_int(0LL)))) {
        return px_bool(false);
    }
    LXValue _v738 = px_index(_v736, px_int(0LL));
    return ({ LXValue _t739 = px_ge(_v738, px_str("A")); px_is_truthy(_t739) ? px_le(_v738, px_str("Z")) : _t739; });
px_err_737:
    if (px_err_737_proped) return px_err_737_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_740_val = px_null();
    int px_err_740_proped = 0;
    LXValue _v741 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        LXValue _v742 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        return px_list_n((LXValue[]){px_str("TyOptional"), _v741, _v742}, 3);
    }
    return _v741;
px_err_740:
    if (px_err_740_proped) return px_err_740_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_743_val = px_null();
    int px_err_743_proped = 0;
    LXValue _v744 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    if (px_is_truthy(px_eq(_v744, px_str("标识符")))) {
        LXValue _v745 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        LXValue _v746 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            LXValue _v747 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            if (px_is_truthy(px_eq(_v745, px_str("list")))) {
                return px_list_n((LXValue[]){px_str("TyList"), _v747, _v746}, 3);
            }
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v745}, 1), px_list_n((LXValue[]){_v747}, 1), _v746}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v745}, 1), _v746}, 3);
    }
    if (px_is_truthy(px_eq(_v744, px_str("[")))) {
        LXValue _v748 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v749 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        return px_list_n((LXValue[]){px_str("TyList"), _v749, _v748}, 3);
    }
    if (px_is_truthy(px_eq(_v744, px_str("{")))) {
        LXValue _v750 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v751 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        LXValue _v752 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        return px_list_n((LXValue[]){px_str("TyDict"), _v751, _v752, _v750}, 4);
    }
    if (px_is_truthy(px_eq(_v744, px_str("(")))) {
        LXValue _v753 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        LXValue _v754 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            while (px_is_truthy(px_bool(true))) {
                (void)(px_method(_v754, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
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
            LXValue _v755 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v754, _v755, _v753}, 4);
        }
        return px_list_n((LXValue[]){px_str("TyTuple"), _v754, _v753}, 3);
    }
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    return px_null();
px_err_743:
    if (px_err_743_proped) return px_err_743_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v756 = (nargs > 0) ? args[0] : px_null();
    LXValue _v757 = (nargs > 1) ? args[1] : px_null();
    LXValue _v758 = px_null();
    LXValue _v759 = px_null();
    LXValue _v760 = px_null();
    LXValue _v761 = px_null();
    LXValue _v762 = px_null();
    LXValue _v763 = px_null();
    LXValue _v764 = px_null();
    LXValue _v765 = px_null();
    LXValue px_err_766_val = px_null();
    int px_err_766_proped = 0;
    LXValue _v767 = px_call(px_get_global("cg_pad"), (LXValue[]){_v757}, 1);
    LXValue _v768 = px_index(_v756, px_int(0LL));
    if (px_is_truthy(px_eq(_v768, px_str("VarDecl")))) {
        LXValue _v769 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v756, px_int(2LL))}, 1);
        if (px_is_truthy(({ LXValue _t770 = px_eq(px_index(_v756, px_int(1LL)), px_str("Let")); px_is_truthy(_t770) ? _t770 : px_eq(px_index(_v756, px_int(1LL)), px_str("Const")); }))) {
            (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v769}, 1));
        }
        LXValue _v771 = px_index(_v756, px_int(3LL));
        if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v771}, 1))) {
            px_index_set(px_get_global("cg_nonnull"), _v769, px_int(1LL));
            if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v756, px_int(4LL))}, 1))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v771}, 1)), px_str("'（可空类型请用 ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v771}, 1)), px_str("? 声明）"))}, 2));
            }
        }
        LXValue _v772 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v756, px_int(4LL)), px_null()))) {
             _v772 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(4LL))}, 1);
        }
        if (px_is_truthy(({ LXValue _t773 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v769}, 2); px_is_truthy(_t773) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t773; }))) {
            return px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_set_global(\"")), _v769), px_str("\", ")), _v772), px_str(");\n"));
        }
        LXValue _v774 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v769}, 1);
        if (px_is_truthy(px_ne(px_index(_v756, px_int(4LL)), px_null()))) {
            LXValue _v775 = px_index(_v756, px_int(4LL));
            LXValue _v776 = px_null();
            if (px_is_truthy(px_eq(px_index(_v775, px_int(0LL)), px_str("Constructor")))) {
                 _v776 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v775, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v775, px_int(0LL)), px_str("Call")))) {
                LXValue _v777 = px_index(_v775, px_int(1LL));
                if (px_is_truthy(px_eq(px_index(_v777, px_int(0LL)), px_str("Var")))) {
                     _v776 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v777, px_int(1LL))}, 1);
                }
            }
            if (px_is_truthy(px_ne(_v776, px_null()))) {
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v776}, 1))) {
                    px_index_set(px_get_global("cg_var_types"), _v769, _v776);
                }
            }
        }
        return px_add(px_add(px_add(px_add(px_add(_v767, px_str("LXValue ")), _v774), px_str(" = ")), _v772), px_str(";\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Assign")))) {
        LXValue _v778 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(3LL))}, 1);
        LXValue _v779 = px_index(_v756, px_int(1LL));
        LXValue _v780 = px_index(_v756, px_int(2LL));
        LXValue _v781 = px_index(_v779, px_int(0LL));
        if (px_is_truthy(px_eq(_v781, px_str("Var")))) {
            LXValue _v782 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v779, px_int(1LL))}, 1);
            if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v782}, 1))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v782), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
            }
            if (px_is_truthy(({ LXValue _t783 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v756, px_int(3LL))}, 1); px_is_truthy(_t783) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v782}, 1) : _t783; }))) {
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v782), px_str("'（可空类型请声明为 ")), _v782), px_str(": T?）"))}, 2));
            }
            LXValue _v784 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v782}, 1);
            if (px_is_truthy(px_eq(_v784, px_null()))) {
                if (px_is_truthy(px_eq(_v780, px_str("Assign")))) {
                    return px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_set_global(\"")), _v782), px_str("\", ")), _v778), px_str(");\n"));
                }
                LXValue _v785 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v780, _v782, _v778}, 3);
                return px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_set_global(\"")), _v782), px_str("\", ")), _v785), px_str(");\n"));
            }
            LXValue _v786 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v780, _v784, _v778}, 3);
            return px_add(px_add(px_add(px_add(px_add(_v767, px_str(" ")), _v784), px_str(" = ")), _v786), px_str(";\n"));
        }
        if (px_is_truthy(px_eq(_v781, px_str("Field")))) {
            LXValue _v787 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v779, px_int(1LL))}, 1);
            LXValue _v788 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v779, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_field_set(")), _v787), px_str(", \"")), _v788), px_str("\", ")), _v778), px_str(");\n"));
        }
        if (px_is_truthy(px_eq(_v781, px_str("Index")))) {
            LXValue _v789 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v779, px_int(1LL))}, 1);
            LXValue _v790 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v779, px_int(2LL))}, 1);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_index_set(")), _v789), px_str(", ")), _v790), px_str(", ")), _v778), px_str(");\n"));
        }
        return px_str("不支持的赋值目标");
    }
    if (px_is_truthy(px_eq(_v768, px_str("ExprStmt")))) {
        LXValue _v791 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v767, px_str("(void)(")), _v791), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("If")))) {
        LXValue _v792 = px_str("");
        LXValue _v793 = px_index(_v756, px_int(1LL));
        LXValue _v794 = px_int(0LL);
        while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v793}, 1)))) {
            LXValue _v795 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v793, _v794), px_int(0LL))}, 1);
            LXValue _v796 = px_str("if");
            if (px_is_truthy(px_gt(_v794, px_int(0LL)))) {
                 _v796 = px_str("else if");
            }
             _v792 = px_add(_v792, px_add(px_add(px_add(px_add(_v767, _v796), px_str(" (px_is_truthy(")), _v795), px_str(")) {\n")));
            LXValue _v797 = px_index(px_index(_v793, _v794), px_int(1LL));
            LXValue _v798 = px_int(0LL);
            while (px_is_truthy(px_lt(_v798, px_call(px_get_global("len"), (LXValue[]){_v797}, 1)))) {
                 _v792 = px_add(_v792, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v797, _v798), px_add(_v757, px_int(1LL))}, 2));
                 _v798 = px_add(_v798, px_int(1LL));
            }
             _v792 = px_add(_v792, px_add(_v767, px_str("}\n")));
             _v794 = px_add(_v794, px_int(1LL));
        }
        if (px_is_truthy(px_ne(px_index(_v756, px_int(2LL)), px_null()))) {
             _v792 = px_add(_v792, px_add(_v767, px_str("else {\n")));
            LXValue _v799 = px_index(_v756, px_int(2LL));
            LXValue _v800 = px_int(0LL);
            while (px_is_truthy(px_lt(_v800, px_call(px_get_global("len"), (LXValue[]){_v799}, 1)))) {
                 _v792 = px_add(_v792, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v799, _v800), px_add(_v757, px_int(1LL))}, 2));
                 _v800 = px_add(_v800, px_int(1LL));
            }
             _v792 = px_add(_v792, px_add(_v767, px_str("}\n")));
        }
        return _v792;
    }
    if (px_is_truthy(px_eq(_v768, px_str("While")))) {
        LXValue _v801 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1);
        LXValue _v802 = px_add(px_add(px_add(_v767, px_str("while (px_is_truthy(")), _v801), px_str(")) {\n"));
        LXValue _v803 = px_index(_v756, px_int(2LL));
        LXValue _v804 = px_int(0LL);
        while (px_is_truthy(px_lt(_v804, px_call(px_get_global("len"), (LXValue[]){_v803}, 1)))) {
             _v802 = px_add(_v802, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v803, _v804), px_add(_v757, px_int(1LL))}, 2));
             _v804 = px_add(_v804, px_int(1LL));
        }
         _v802 = px_add(_v802, px_add(_v767, px_str("}\n")));
        return _v802;
    }
    if (px_is_truthy(px_eq(_v768, px_str("For")))) {
        LXValue _v805 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(2LL))}, 1);
        LXValue _v806 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v807 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v808 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1)}, 1);
        LXValue _v809 = px_add(px_add(px_add(px_add(px_add(_v767, px_str("LXValue ")), _v806), px_str(" = ")), _v805), px_str(";\n"));
         _v809 = px_add(_v809, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v767, px_str("for (int ")), _v807), px_str(" = 0; ")), _v807), px_str(" < px_len(")), _v806), px_str("); ")), _v807), px_str("++) {\n")));
         _v809 = px_add(_v809, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v767, px_str("    LXValue ")), _v808), px_str(" = px_index(")), _v806), px_str(", px_int(")), _v807), px_str("));\n")));
        LXValue _v810 = px_index(_v756, px_int(3LL));
        LXValue _v811 = px_int(0LL);
        while (px_is_truthy(px_lt(_v811, px_call(px_get_global("len"), (LXValue[]){_v810}, 1)))) {
             _v809 = px_add(_v809, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v810, _v811), px_add(_v757, px_int(1LL))}, 2));
             _v811 = px_add(_v811, px_int(1LL));
        }
         _v809 = px_add(_v809, px_add(_v767, px_str("}\n")));
        return _v809;
    }
    if (px_is_truthy(px_eq(_v768, px_str("Return")))) {
        if (px_is_truthy(px_ne(px_index(_v756, px_int(1LL)), px_null()))) {
            LXValue _v812 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1);
            return px_add(px_add(px_add(_v767, px_str("return ")), _v812), px_str(";\n"));
        }
        return px_add(_v767, px_str("return px_null();\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Break")))) {
        return px_add(_v767, px_str("break;\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Continue")))) {
        return px_add(_v767, px_str("continue;\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Empty")))) {
        return px_str("");
    }
    if (px_is_truthy(px_eq(_v768, px_str("ChanDecl")))) {
        LXValue _v813 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1)}, 1);
        return px_add(px_add(px_add(_v767, px_str("LXValue ")), _v813), px_str(" = px_chan_create(0);\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Send")))) {
        LXValue _v814 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1);
        LXValue _v815 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_chan_send(")), _v814), px_str(", ")), _v815), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Recv")))) {
        LXValue _v816 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v756, px_int(1LL))}, 1);
        return px_add(px_add(px_add(_v767, px_str("px_chan_recv(")), _v816), px_str(");\n"));
    }
    if (px_is_truthy(px_eq(_v768, px_str("Spawn")))) {
        LXValue _v817 = px_index(_v756, px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v817, px_int(0LL)), px_str("Call")))) {
            LXValue _v818 = px_index(_v817, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v818, px_int(0LL)), px_str("Var")))) {
                LXValue _v819 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v818, px_int(1LL))}, 1);
                LXValue _v820 = px_list_n((LXValue[]){}, 0);
                LXValue _v821 = px_index(_v817, px_int(2LL));
                LXValue _v822 = px_int(0LL);
                while (px_is_truthy(px_lt(_v822, px_call(px_get_global("len"), (LXValue[]){_v821}, 1)))) {
                    (void)(px_method(_v820, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v821, _v822)}, 1)}, 1));
                     _v822 = px_add(_v822, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v767, px_str("px_spawn_name(\"")), _v819), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v820}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v820}, 1)}, 1)), px_str(");\n"));
            }
            return px_str("编译模式 spawn 仅支持直接函数调用（方法调用请用 `lx run`）");
        }
        return px_str("编译模式 spawn 仅支持函数调用表达式");
    }
    if (px_is_truthy(px_eq(_v768, px_str("Select")))) {
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v756, px_int(1LL)), px_index(_v756, px_int(2LL)), _v757}, 3);
    }
    if (px_is_truthy(px_eq(_v768, px_str("Import")))) {
        return px_add(_v767, px_str("/* import 忽略（MVP） */\n"));
    }
    return px_str("");
px_err_766:
    if (px_err_766_proped) return px_err_766_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v823 = (nargs > 0) ? args[0] : px_null();
    LXValue _v824 = (nargs > 1) ? args[1] : px_null();
    LXValue _v825 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_826_val = px_null();
    int px_err_826_proped = 0;
    if (px_is_truthy(px_eq(_v823, px_str("Assign")))) {
        return _v825;
    }
    if (px_is_truthy(px_eq(_v823, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v823, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v824), px_str("\"), ")), _v825), px_str(")"));
    }
    return _v825;
px_err_826:
    if (px_err_826_proped) return px_err_826_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v827 = (nargs > 0) ? args[0] : px_null();
    LXValue _v828 = (nargs > 1) ? args[1] : px_null();
    LXValue _v829 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_830_val = px_null();
    int px_err_830_proped = 0;
    if (px_is_truthy(px_eq(_v827, px_str("Assign")))) {
        return _v829;
    }
    if (px_is_truthy(px_eq(_v827, px_str("Plus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Minus")))) {
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Star")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Slash")))) {
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("IntDiv")))) {
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Mod")))) {
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Pow")))) {
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("BitAnd")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("BitOr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("BitXor")))) {
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Shl")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("Shr")))) {
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v827, px_str("ShrU")))) {
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v828), px_str(", ")), _v829), px_str(")"));
    }
    return _v829;
px_err_830:
    if (px_err_830_proped) return px_err_830_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v831 = (nargs > 0) ? args[0] : px_null();
    LXValue _v832 = (nargs > 1) ? args[1] : px_null();
    LXValue _v833 = (nargs > 2) ? args[2] : px_null();
    LXValue _v834 = px_null();
    LXValue _v835 = px_null();
    LXValue _v836 = px_null();
    LXValue _v837 = px_null();
    LXValue _v838 = px_null();
    LXValue _v839 = px_null();
    LXValue _v840 = px_null();
    LXValue _v841 = px_null();
    LXValue px_err_842_val = px_null();
    int px_err_842_proped = 0;
    LXValue _v843 = px_call(px_get_global("cg_pad"), (LXValue[]){_v833}, 1);
    LXValue _v844 = px_call(px_get_global("len"), (LXValue[]){_v831}, 1);
    if (px_is_truthy(px_eq(_v844, px_int(0LL)))) {
        return px_str("select 至少需要一个 case 分支");
    }
    LXValue _v845 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    LXValue _v846 = px_str("");
    LXValue _v847 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v848 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    LXValue _v849 = px_list_n((LXValue[]){}, 0);
    LXValue _v850 = px_int(0LL);
    while (px_is_truthy(px_lt(_v850, _v844))) {
        LXValue _v851 = px_index(px_index(_v831, _v850), px_int(1LL));
        if (px_is_truthy(px_eq(px_index(_v851, px_int(0LL)), px_str("Call")))) {
            LXValue _v852 = px_index(_v851, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v852, px_int(0LL)), px_str("Field")))) {
                LXValue _v853 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v852, px_int(2LL))}, 1);
                if (px_is_truthy(px_eq(_v853, px_str("recv")))) {
                    (void)(px_method(_v849, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v852, px_int(1LL))}, 1)}, 1));
                     _v850 = px_add(_v850, px_int(1LL));
                    continue;
                }
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v853), px_str("）"));
            }
            return px_str("select case 仅支持 ch.recv()");
        }
        return px_str("select case 仅支持 ch.recv()");
    }
     _v846 = px_add(_v846, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v843, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v844}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v849}, 2)), px_str("};\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(": {\n")));
    LXValue _v854 = px_list_n((LXValue[]){}, 0);
    LXValue _v855 = px_int(0LL);
    while (px_is_truthy(px_lt(_v855, _v844))) {
        (void)(px_method(_v854, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v855}, 1)}, 1));
         _v855 = px_add(_v855, px_int(1LL));
    }
     _v846 = px_add(_v846, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v843, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v844}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v854}, 2)), px_str("};\n")));
    if (px_is_truthy(px_gt(_v844, px_int(1LL)))) {
        LXValue _v856 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v843, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v844}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("--) { "));
         _v856 = px_add(_v856, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" + 1); ")));
         _v856 = px_add(_v856, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("; ")));
         _v856 = px_add(_v856, px_str("}\n"));
         _v846 = px_add(_v846, _v856);
    }
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = px_null();\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = -1;\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v843, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v844}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("++) {\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v843, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("];\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v843, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str("; break; }\n")));
     _v846 = px_add(_v846, px_add(_v843, px_str("    }\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" >= 0) {\n")));
    LXValue _v857 = px_int(0LL);
    while (px_is_truthy(px_lt(_v857, _v844))) {
        LXValue _v858 = px_index(px_index(_v831, _v857), px_int(0LL));
        LXValue _v859 = px_index(px_index(_v831, _v857), px_int(2LL));
        LXValue _v860 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v857}, 1)), px_str(")"));
        if (px_is_truthy(px_gt(_v857, px_int(0LL)))) {
             _v860 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v857}, 1)), px_str(")"));
        }
         _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("        ")), _v860), px_str(" {\n")));
        if (px_is_truthy(px_ne(_v858, px_null()))) {
            LXValue _v861 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v858}, 1)}, 1);
             _v846 = px_add(_v846, px_add(px_add(px_add(px_add(px_add(_v843, px_str("            LXValue ")), _v861), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(";\n")));
        }
        LXValue _v862 = px_int(0LL);
        while (px_is_truthy(px_lt(_v862, px_call(px_get_global("len"), (LXValue[]){_v859}, 1)))) {
             _v846 = px_add(_v846, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v859, _v862), px_add(_v833, px_int(3LL))}, 2));
             _v862 = px_add(_v862, px_int(1LL));
        }
         _v846 = px_add(_v846, px_add(_v843, px_str("        }\n")));
         _v857 = px_add(_v857, px_int(1LL));
    }
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(";\n")));
     _v846 = px_add(_v846, px_add(_v843, px_str("    }\n")));
    if (px_is_truthy(px_ne(_v832, px_null()))) {
         _v846 = px_add(_v846, px_add(_v843, px_str("    {\n")));
        LXValue _v863 = px_int(0LL);
        while (px_is_truthy(px_lt(_v863, px_call(px_get_global("len"), (LXValue[]){_v832}, 1)))) {
             _v846 = px_add(_v846, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v832, _v863), px_add(_v833, px_int(2LL))}, 2));
             _v863 = px_add(_v863, px_int(1LL));
        }
         _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(";\n")));
         _v846 = px_add(_v846, px_add(_v843, px_str("    }\n")));
    }
     _v846 = px_add(_v846, px_add(_v843, px_str("    px_select_wait();\n")));
     _v846 = px_add(_v846, px_add(_v843, px_str("}\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(";\n")));
     _v846 = px_add(_v846, px_add(px_add(px_add(_v843, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v845}, 1)), px_str(": ;\n")));
    px_set_global("cg_vars", _v847);
    px_set_global("cg_var_types", _v848);
    return _v846;
px_err_842:
    if (px_err_842_proped) return px_err_842_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v864 = (nargs > 0) ? args[0] : px_null();
    LXValue _v865 = px_null();
    LXValue _v866 = px_null();
    LXValue _v867 = px_null();
    LXValue _v868 = px_null();
    LXValue px_err_869_val = px_null();
    int px_err_869_proped = 0;
    LXValue _v870 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v870, "remove", (LXValue[]){px_str("_")}, 1));
    px_index_set(_v870, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v870, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v870, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v870, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v870, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_index_set(_v870, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    LXValue _v871 = px_int(0LL);
    while (px_is_truthy(px_lt(_v871, px_call(px_get_global("len"), (LXValue[]){_v864}, 1)))) {
        LXValue _v872 = px_index(_v864, _v871);
        LXValue _v873 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v872, px_int(2LL))}, 1);
        (void)(px_method(px_index(_v870, px_str("its")), "append", (LXValue[]){_v873}, 1));
        (void)(px_method(px_index(_v870, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v870, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        (void)(px_method(px_index(_v870, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        LXValue _v874 = px_str("");
        LXValue _v875 = px_list_n((LXValue[]){}, 0);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v872, px_int(1LL))}, 1), px_int(1LL)))) {
            LXValue _v876 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            LXValue _v877 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v872, px_int(1LL)), px_int(0LL))}, 1);
            LXValue _v878 = px_null();
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v877}, 1))) {
                 _v878 = px_index(px_get_global("cg_vars"), _v877);
            }
            px_index_set(px_get_global("cg_vars"), _v877, _v876);
            (void)(px_method(_v875, "append", (LXValue[]){px_list_n((LXValue[]){_v877, _v878}, 2)}, 1));
             _v874 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v876), px_str(" = ")), px_index(px_index(_v870, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v870, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            LXValue _v879 = px_int(0LL);
            while (px_is_truthy(px_lt(_v879, px_call(px_get_global("len"), (LXValue[]){px_index(_v872, px_int(1LL))}, 1)))) {
                LXValue _v880 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v872, px_int(1LL)), _v879)}, 1);
                LXValue _v881 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v879}, 1));
                LXValue _v882 = px_null();
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v880}, 1))) {
                     _v882 = px_index(px_get_global("cg_vars"), _v880);
                }
                px_index_set(px_get_global("cg_vars"), _v880, _v881);
                (void)(px_method(_v875, "append", (LXValue[]){px_list_n((LXValue[]){_v880, _v882}, 2)}, 1));
                 _v874 = px_add(_v874, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v881), px_str(" = px_index(")), px_index(px_index(_v870, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v870, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v879}, 1)), px_str(")); ")));
                 _v879 = px_add(_v879, px_int(1LL));
            }
        }
        (void)(px_method(px_index(_v870, px_str("binds")), "append", (LXValue[]){_v874}, 1));
        (void)(px_method(px_index(_v870, px_str("saved_all")), "append", (LXValue[]){_v875}, 1));
         _v871 = px_add(_v871, px_int(1LL));
    }
    return _v870;
px_err_869:
    if (px_err_869_proped) return px_err_869_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v883 = (nargs > 0) ? args[0] : px_null();
    LXValue _v884 = px_null();
    LXValue _v885 = px_null();
    LXValue px_err_886_val = px_null();
    int px_err_886_proped = 0;
    LXValue _v887 = px_int(0LL);
    while (px_is_truthy(px_lt(_v887, px_call(px_get_global("len"), (LXValue[]){_v883}, 1)))) {
        LXValue _v888 = px_index(_v883, _v887);
        LXValue _v889 = px_int(0LL);
        while (px_is_truthy(px_lt(_v889, px_call(px_get_global("len"), (LXValue[]){_v888}, 1)))) {
            LXValue _v890 = px_index(px_index(_v888, _v889), px_int(0LL));
            LXValue _v891 = px_index(px_index(_v888, _v889), px_int(1LL));
            if (px_is_truthy(px_eq(_v891, px_null()))) {
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v890}, 1));
            }
            else {
                px_index_set(px_get_global("cg_vars"), _v890, _v891);
            }
             _v889 = px_add(_v889, px_int(1LL));
        }
         _v887 = px_add(_v887, px_int(1LL));
    }
px_err_886:
    if (px_err_886_proped) return px_err_886_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v892 = (nargs > 0) ? args[0] : px_null();
    LXValue _v893 = (nargs > 1) ? args[1] : px_null();
    LXValue _v894 = (nargs > 2) ? args[2] : px_null();
    LXValue _v895 = px_null();
    LXValue _v896 = px_null();
    LXValue _v897 = px_null();
    LXValue px_err_898_val = px_null();
    int px_err_898_proped = 0;
    LXValue _v899 = px_str("");
    if (px_is_truthy(px_ne(_v893, px_null()))) {
         _v899 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v893), px_str(")) { ")), _v894), px_str("} "));
    }
    else {
         _v899 = _v894;
    }
    LXValue _v900 = px_call(px_get_global("len"), (LXValue[]){px_index(_v892, px_str("its"))}, 1);
    LXValue _v901 = px_sub(_v900, px_int(1LL));
    while (px_is_truthy(px_ge(_v901, px_int(0LL)))) {
        LXValue _v902 = px_str("");
        if (px_is_truthy(px_lt(px_add(_v901, px_int(1LL)), _v900))) {
             _v902 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v892, px_str("ivs")), px_add(_v901, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v892, px_str("its")), px_add(_v901, px_int(1LL)))), px_str("; "));
        }
         _v899 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v892, px_str("idxs")), _v901)), px_str("=0; ")), px_index(px_index(_v892, px_str("idxs")), _v901)), px_str("<px_len(")), px_index(px_index(_v892, px_str("ivs")), _v901)), px_str("); ")), px_index(px_index(_v892, px_str("idxs")), _v901)), px_str("++) { LXValue ")), px_index(px_index(_v892, px_str("itms")), _v901)), px_str(" = px_index(")), px_index(px_index(_v892, px_str("ivs")), _v901)), px_str(", px_int(")), px_index(px_index(_v892, px_str("idxs")), _v901)), px_str(")); ")), px_index(px_index(_v892, px_str("binds")), _v901)), _v902), _v899), px_str(" } "));
         _v901 = px_sub(_v901, px_int(1LL));
    }
    return _v899;
px_err_898:
    if (px_err_898_proped) return px_err_898_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v903 = (nargs > 0) ? args[0] : px_null();
    LXValue _v904 = px_null();
    LXValue _v905 = px_null();
    LXValue _v906 = px_null();
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
    LXValue px_err_921_val = px_null();
    int px_err_921_proped = 0;
    LXValue _v922 = px_index(_v903, px_int(0LL));
    if (px_is_truthy(px_eq(_v922, px_str("Int")))) {
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1)), px_str("LL)"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Float")))) {
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Str")))) {
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1)}, 1)), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Bool")))) {
        if (px_is_truthy(px_index(_v903, px_int(1LL)))) {
            return px_str("px_bool(true)");
        }
        return px_str("px_bool(false)");
    }
    if (px_is_truthy(px_eq(_v922, px_str("Null")))) {
        return px_str("px_null()");
    }
    if (px_is_truthy(px_eq(_v922, px_str("List")))) {
        LXValue _v923 = px_list_n((LXValue[]){}, 0);
        LXValue _v924 = px_index(_v903, px_int(1LL));
        LXValue _v925 = px_int(0LL);
        while (px_is_truthy(px_lt(_v925, px_call(px_get_global("len"), (LXValue[]){_v924}, 1)))) {
            (void)(px_method(_v923, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, _v925)}, 1)}, 1));
             _v925 = px_add(_v925, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v923}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v923}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Tuple")))) {
        LXValue _v926 = px_list_n((LXValue[]){}, 0);
        LXValue _v927 = px_index(_v903, px_int(1LL));
        LXValue _v928 = px_int(0LL);
        while (px_is_truthy(px_lt(_v928, px_call(px_get_global("len"), (LXValue[]){_v927}, 1)))) {
            (void)(px_method(_v926, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v927, _v928)}, 1)}, 1));
             _v928 = px_add(_v928, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v926}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v926}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Dict")))) {
        LXValue _v929 = px_str("({ LXValue _d = px_dict(); ");
        LXValue _v930 = px_index(_v903, px_int(1LL));
        LXValue _v931 = px_int(0LL);
        while (px_is_truthy(px_lt(_v931, px_call(px_get_global("len"), (LXValue[]){_v930}, 1)))) {
            LXValue _v932 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v930, _v931), px_int(0LL))}, 1);
            LXValue _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v930, _v931), px_int(1LL))}, 1);
             _v929 = px_add(_v929, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v932), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v933), px_str("); } ")));
             _v931 = px_add(_v931, px_int(1LL));
        }
         _v929 = px_add(_v929, px_str("_d; })"));
        return _v929;
    }
    if (px_is_truthy(px_eq(_v922, px_str("Var")))) {
        LXValue _v934 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v935 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v934}, 1);
        if (px_is_truthy(px_ne(_v935, px_null()))) {
            return _v935;
        }
        return px_add(px_add(px_str("px_get_global(\""), _v934), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Field")))) {
        LXValue _v936 = px_index(_v903, px_int(1LL));
        LXValue _v937 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        if (px_is_truthy(px_eq(px_index(_v936, px_int(0LL)), px_str("Var")))) {
            LXValue _v938 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v936, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v938}, 1))) {
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v938), px_str("\", \"")), _v937), px_str("\")"));
            }
        }
        LXValue _v939 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v936}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v939), px_str(", \"")), _v937), px_str("\")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("OptionalField")))) {
        LXValue _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v942 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v940), px_str("; px_is_null(")), _v941), px_str(") ? px_null() : px_field(")), _v940), px_str(", \"")), _v942), px_str("\"); })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Index")))) {
        LXValue _v943 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v944 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v943), px_str(", ")), _v944), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Slice")))) {
        LXValue _v945 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v946 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v903, px_int(2LL)), px_null()))) {
             _v946 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        }
        LXValue _v947 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v903, px_int(3LL)), px_null()))) {
             _v947 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
        }
        LXValue _v948 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v903, px_int(4LL)), px_null()))) {
             _v948 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(4LL))}, 1);
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v945), px_str(", ")), _v946), px_str(", ")), _v947), px_str(", ")), _v948), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Call")))) {
        LXValue _v949 = px_index(_v903, px_int(1LL));
        LXValue _v950 = px_index(_v903, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v949, px_int(0LL)), px_str("Var")))) {
            LXValue _v951 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v949, px_int(1LL))}, 1);
            if (px_is_truthy(px_eq(_v951, px_str("chan")))) {
                LXValue _v952 = px_str("0");
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v950}, 1), px_int(0LL)))) {
                    if (px_is_truthy(px_eq(px_index(px_index(_v950, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                         _v952 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v950, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                         _v952 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v950, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                return px_add(px_add(px_str("px_chan_create("), _v952), px_str(")"));
            }
            if (px_is_truthy(px_eq(_v951, px_str("mutex")))) {
                return px_str("px_mutex_create()");
            }
            if (px_is_truthy(px_eq(_v951, px_str("rwlock")))) {
                return px_str("px_rwlock_create()");
            }
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v951}, 1))) {
                LXValue _v953 = px_index(px_get_global("cg_structs"), _v951);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v953}, 1), px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v951), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v953}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v950}, 1)}, 1));
                }
                LXValue _v954 = px_list_n((LXValue[]){}, 0);
                LXValue _v955 = px_int(0LL);
                while (px_is_truthy(px_lt(_v955, px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
                    (void)(px_method(_v954, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v950, _v955)}, 1)}, 1));
                     _v955 = px_add(_v955, px_int(1LL));
                }
                LXValue _v956 = px_list_n((LXValue[]){}, 0);
                LXValue _v957 = px_int(0LL);
                while (px_is_truthy(px_lt(_v957, px_call(px_get_global("len"), (LXValue[]){_v953}, 1)))) {
                    (void)(px_method(_v956, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v953, _v957)), px_str("\""))}, 1));
                     _v957 = px_add(_v957, px_int(1LL));
                }
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v951), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v956}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v954}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v953}, 1)}, 1)), px_str(")"));
            }
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v951}, 1))) {
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v950}, 1), px_int(1LL)))) {
                    return px_add(px_add(px_str("枚举 "), _v951), px_str(" 构造需要一个变体名"));
                }
                LXValue _v958 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v950, px_int(0LL))}, 1);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v951), px_str("\", (")), _v958), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        if (px_is_truthy(px_eq(px_index(_v949, px_int(0LL)), px_str("Field")))) {
            LXValue _v959 = px_index(_v949, px_int(1LL));
            LXValue _v960 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v949, px_int(2LL))}, 1);
            LXValue _v961 = px_null();
            if (px_is_truthy(px_eq(px_index(_v959, px_int(0LL)), px_str("Var")))) {
                LXValue _v962 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v959, px_int(1LL))}, 1);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v962}, 1))) {
                     _v961 = px_index(px_get_global("cg_var_types"), _v962);
                }
            }
            if (px_is_truthy(({ LXValue _t963 = px_ne(_v961, px_null()); px_is_truthy(_t963) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v961}, 1) : _t963; }))) {
                LXValue _v964 = px_index(px_get_global("cg_impls"), _v961);
                LXValue _v965 = px_bool(false);
                LXValue _v966 = px_int(0LL);
                while (px_is_truthy(px_lt(_v966, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v964, _v966), px_int(1LL))}, 1), _v960))) {
                         _v965 = px_bool(true);
                        break;
                    }
                     _v966 = px_add(_v966, px_int(1LL));
                }
                if (px_is_truthy(_v965)) {
                    LXValue _v967 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v959}, 1);
                    LXValue _v968 = px_list_n((LXValue[]){_v967}, 1);
                    LXValue _v969 = px_int(0LL);
                    while (px_is_truthy(px_lt(_v969, px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
                        (void)(px_method(_v968, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v950, _v969)}, 1)}, 1));
                         _v969 = px_add(_v969, px_int(1LL));
                    }
                    LXValue _v970 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v961}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v960}, 1));
                    return px_add(px_add(px_add(px_add(px_add(_v970, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v968}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v968}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            LXValue _v971 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v959}, 1);
            LXValue _v972 = px_list_n((LXValue[]){}, 0);
            LXValue _v973 = px_int(0LL);
            while (px_is_truthy(px_lt(_v973, px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
                (void)(px_method(_v972, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v950, _v973)}, 1)}, 1));
                 _v973 = px_add(_v973, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v971), px_str(", \"")), _v960), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v972}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v972}, 1)}, 1)), px_str(")"));
        }
        LXValue _v974 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v949}, 1);
        LXValue _v975 = px_list_n((LXValue[]){}, 0);
        LXValue _v976 = px_int(0LL);
        while (px_is_truthy(px_lt(_v976, px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
            (void)(px_method(_v975, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v950, _v976)}, 1)}, 1));
             _v976 = px_add(_v976, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v974), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v975}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v975}, 1)}, 1)), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Unary")))) {
        LXValue _v977 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        LXValue _v978 = px_index(_v903, px_int(1LL));
        if (px_is_truthy(px_eq(_v978, px_str("Neg")))) {
            return px_add(px_add(px_str("px_neg("), _v977), px_str(")"));
        }
        if (px_is_truthy(px_eq(_v978, px_str("Not")))) {
            return px_add(px_add(px_str("px_not("), _v977), px_str(")"));
        }
        return px_add(px_add(px_str("px_bitnot("), _v977), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Binary")))) {
        LXValue _v979 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        LXValue _v980 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
        LXValue _v981 = px_index(_v903, px_int(1LL));
        if (px_is_truthy(px_eq(_v981, px_str("And")))) {
            LXValue _v982 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v982), px_str(" = ")), _v979), px_str("; px_is_truthy(")), _v982), px_str(") ? ")), _v980), px_str(" : ")), _v982), px_str("; })"));
        }
        if (px_is_truthy(px_eq(_v981, px_str("Or")))) {
            LXValue _v983 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v983), px_str(" = ")), _v979), px_str("; px_is_truthy(")), _v983), px_str(") ? ")), _v983), px_str(" : ")), _v980), px_str("; })"));
        }
        LXValue _v984 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v981}, 1);
        return px_add(px_add(px_add(px_add(px_add(_v984, px_str("(")), _v979), px_str(", ")), _v980), px_str(")"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Pipe")))) {
        LXValue _v985 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v986 = px_index(_v903, px_int(2LL));
        if (px_is_truthy(px_eq(px_index(_v986, px_int(0LL)), px_str("Call")))) {
            LXValue _v987 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v986, px_int(1LL))}, 1);
            LXValue _v988 = px_list_n((LXValue[]){_v985}, 1);
            LXValue _v989 = px_int(0LL);
            while (px_is_truthy(px_lt(_v989, px_call(px_get_global("len"), (LXValue[]){px_index(_v986, px_int(2LL))}, 1)))) {
                (void)(px_method(_v988, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v986, px_int(2LL)), _v989)}, 1)}, 1));
                 _v989 = px_add(_v989, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v987), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v988}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v988}, 1)}, 1)), px_str(")"));
        }
        LXValue _v990 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v986}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v990), px_str(", (LXValue[]){")), _v985), px_str("}, 1)"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("NullCoalesce")))) {
        LXValue _v991 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v992 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        LXValue _v993 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v993), px_str(" = ")), _v991), px_str("; px_is_null(")), _v993), px_str(") ? ")), _v992), px_str(" : ")), _v993), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Try")))) {
        LXValue _v994 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v995 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            LXValue _v996 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v995), px_str(" = ")), _v994), px_str("; if (px_is_result(")), _v995), px_str(")) { if (!px_result_ok(")), _v995), px_str(")) { ")), _v996), px_str("_val = ")), _v995), px_str("; ")), _v996), px_str("_proped = 1; goto ")), _v996), px_str("; } ")), _v995), px_str(" = px_result_unwrap(")), _v995), px_str("); } else if (px_is_null(")), _v995), px_str(")) { ")), _v996), px_str("_val = px_null(); ")), _v996), px_str("_proped = 1; goto ")), _v996), px_str("; } ")), _v995), px_str("; })"));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v995), px_str(" = ")), _v994), px_str("; if (px_is_result(")), _v995), px_str(") && !px_result_ok(")), _v995), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v995), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v995), px_str(")) ")), _v995), px_str(" = px_result_unwrap(")), _v995), px_str("); ")), _v995), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("ForceUnwrap")))) {
        LXValue _v997 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v998 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v998), px_str(" = ")), _v997), px_str("; if (px_is_result(")), _v998), px_str(")) { if (!px_result_ok(")), _v998), px_str(")) px_error(\"force unwrap Err\"); ")), _v998), px_str(" = px_result_unwrap(")), _v998), px_str("); } if (px_is_null(")), _v998), px_str(")) px_error(\"force unwrap null\"); ")), _v998), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("IfExpr")))) {
        LXValue _v999 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v1000 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        LXValue _v1001 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
        LXValue _v1002 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1002), px_str("; if (px_is_truthy(")), _v999), px_str(")) { ")), _v1002), px_str(" = ")), _v1000), px_str("; } else { ")), _v1002), px_str(" = ")), _v1001), px_str("; } ")), _v1002), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("ListComp")))) {
        LXValue _v1003 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1004 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        LXValue _v1005 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v1006 = px_null();
        if (px_is_truthy(px_ne(px_index(_v903, px_int(3LL)), px_null()))) {
             _v1006 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v1004, px_str("saved_all"))}, 1));
        LXValue _v1007 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v1003), px_str(", ")), _v1005), px_str("); "));
        LXValue _v1008 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v1004, _v1006, _v1007}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1003), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v1004, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v1004, px_str("its")), px_int(0LL))), px_str("; ")), _v1008), px_str(" ")), _v1003), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("GenExp")))) {
        LXValue _v1009 = px_index(_v903, px_int(2LL));
        if (px_is_truthy(({ LXValue _t1010 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1009}, 1), px_int(1LL)); px_is_truthy(_t1010) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1009, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1010; }))) {
            LXValue _v1011 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1009, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            LXValue _v1012 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v1009, px_int(0LL)), px_int(2LL))}, 1);
            LXValue _v1013 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1011}, 1), px_index(_v903, px_int(1LL))}, 2);
            LXValue _v1014 = px_str("px_null()");
            if (px_is_truthy(px_ne(px_index(_v903, px_int(3LL)), px_null()))) {
                 _v1014 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1011}, 1), px_index(_v903, px_int(3LL))}, 2);
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v1012), px_str(", ")), _v1013), px_str(", ")), _v1014), px_str(")"));
        }
        LXValue _v1015 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1016 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v1009}, 1);
        LXValue _v1017 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v1018 = px_null();
        if (px_is_truthy(px_ne(px_index(_v903, px_int(3LL)), px_null()))) {
             _v1018 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v1016, px_str("saved_all"))}, 1));
        LXValue _v1019 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v1015), px_str(", ")), _v1017), px_str("); "));
        LXValue _v1020 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v1016, _v1018, _v1019}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1015), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v1016, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v1016, px_str("its")), px_int(0LL))), px_str("; ")), _v1020), px_str(" px_gen_from_list(")), _v1015), px_str("); })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("DictComp")))) {
        LXValue _v1021 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1022 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
        LXValue _v1023 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v1024 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(2LL))}, 1);
        LXValue _v1025 = px_null();
        if (px_is_truthy(px_ne(px_index(_v903, px_int(4LL)), px_null()))) {
             _v1025 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(4LL))}, 1);
        }
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v1022, px_str("saved_all"))}, 1));
        LXValue _v1026 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v1023), px_str("; LXValue _v = ")), _v1024), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v1021), px_str(", _k.as.obj->as.str.data, _v); } "));
        LXValue _v1027 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v1022, _v1025, _v1026}, 3);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1021), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v1022, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v1022, px_str("its")), px_int(0LL))), px_str("; ")), _v1027), px_str(" ")), _v1021), px_str("; })"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Closure")))) {
        px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
        LXValue _v1028 = px_get_global("cg_closure_id");
        LXValue _v1029 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1028}, 1));
        LXValue _v1030 = px_add(px_add(px_str("static LXValue "), _v1029), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
         _v1030 = px_add(_v1030, px_str("    (void)ctx;\n"));
        LXValue _v1031 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
        LXValue _v1032 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
        px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        LXValue _v1033 = px_index(_v903, px_int(1LL));
        LXValue _v1034 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1034, px_call(px_get_global("len"), (LXValue[]){_v1033}, 1)))) {
            LXValue _v1035 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1033, _v1034), px_int(1LL))}, 1)}, 1);
             _v1030 = px_add(_v1030, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1035), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1034}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1034}, 1)), px_str("] : px_null();\n")));
             _v1034 = px_add(_v1034, px_int(1LL));
        }
        LXValue _v1036 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(3LL))}, 1);
         _v1030 = px_add(_v1030, px_add(px_add(px_str("    return "), _v1036), px_str(";\n")));
         _v1030 = px_add(_v1030, px_str("}\n"));
        px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1030));
        px_set_global("cg_vars", _v1031);
        px_set_global("cg_var_types", _v1032);
        return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1028}, 1)), px_str(">\", ")), _v1029), px_str(", NULL)"));
    }
    if (px_is_truthy(px_eq(_v922, px_str("Block")))) {
        LXValue _v1037 = px_str("({ ");
         _v1037 = px_add(_v1037, px_str("LXValue _blk = px_null(); "));
        LXValue _v1038 = px_index(_v903, px_int(1LL));
        LXValue _v1039 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1039, px_call(px_get_global("len"), (LXValue[]){_v1038}, 1)))) {
            LXValue _v1040 = px_index(_v1038, _v1039);
            if (px_is_truthy(px_eq(px_index(_v1040, px_int(0LL)), px_str("ExprStmt")))) {
                LXValue _v1041 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1040, px_int(1LL))}, 1);
                 _v1037 = px_add(_v1037, px_add(px_add(px_str("_blk = "), _v1041), px_str("; ")));
            }
            else {
                 _v1037 = px_add(_v1037, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1040, px_int(0LL)}, 2));
            }
             _v1039 = px_add(_v1039, px_int(1LL));
        }
         _v1037 = px_add(_v1037, px_str("_blk; })"));
        return _v1037;
    }
    if (px_is_truthy(px_eq(_v922, px_str("Match")))) {
        LXValue _v1042 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v1043 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        LXValue _v1044 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v1043), px_str(" = ")), _v1042), px_str("; "));
        LXValue _v1045 = px_index(_v903, px_int(2LL));
        LXValue _v1046 = px_bool(true);
        LXValue _v1047 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1047, px_call(px_get_global("len"), (LXValue[]){_v1045}, 1)))) {
            LXValue _v1048 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v1045, _v1047), px_int(1LL)), _v1043}, 2);
            LXValue _v1049 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v1045, _v1047), px_int(3LL))}, 1);
            LXValue _v1050 = px_str("if");
            if (px_is_truthy(px_not(_v1046))) {
                 _v1050 = px_str("else if");
            }
             _v1044 = px_add(_v1044, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v1050, px_str(" (")), _v1048), px_str(") { ")), _v1043), px_str(" = ")), _v1049), px_str("; } ")));
             _v1046 = px_bool(false);
             _v1047 = px_add(_v1047, px_int(1LL));
        }
         _v1044 = px_add(_v1044, px_add(_v1043, px_str("; })")));
        return _v1044;
    }
    if (px_is_truthy(px_eq(_v922, px_str("Constructor")))) {
        LXValue _v1051 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v903, px_int(1LL))}, 1);
        LXValue _v1052 = px_index(_v903, px_int(2LL));
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v1051}, 1))) {
            LXValue _v1053 = px_index(px_get_global("cg_structs"), _v1051);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1053}, 1), px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)))) {
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v1051), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1053}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)}, 1));
            }
            LXValue _v1054 = px_list_n((LXValue[]){}, 0);
            LXValue _v1055 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1055, px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)))) {
                (void)(px_method(_v1054, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1052, _v1055)}, 1)}, 1));
                 _v1055 = px_add(_v1055, px_int(1LL));
            }
            LXValue _v1056 = px_list_n((LXValue[]){}, 0);
            LXValue _v1057 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1057, px_call(px_get_global("len"), (LXValue[]){_v1053}, 1)))) {
                (void)(px_method(_v1056, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v1053, _v1057)), px_str("\""))}, 1));
                 _v1057 = px_add(_v1057, px_int(1LL));
            }
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v1051), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1056}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1054}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1053}, 1)}, 1)), px_str(")"));
        }
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v1051}, 1))) {
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1052}, 1), px_int(1LL)))) {
                return px_add(px_add(px_str("枚举 "), _v1051), px_str(" 构造需要一个变体名"));
            }
            LXValue _v1058 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1052, px_int(0LL))}, 1);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v1051), px_str("\", (")), _v1058), px_str(").as.obj->as.enum_inst.variant)"));
        }
        LXValue _v1059 = px_list_n((LXValue[]){}, 0);
        LXValue _v1060 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1060, px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)))) {
            (void)(px_method(_v1059, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1052, _v1060)}, 1)}, 1));
             _v1060 = px_add(_v1060, px_int(1LL));
        }
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v1051), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1059}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1059}, 1)}, 1)), px_str(")"));
    }
    return px_str("px_null()");
px_err_921:
    if (px_err_921_proped) return px_err_921_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1061 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1062_val = px_null();
    int px_err_1062_proped = 0;
    if (px_is_truthy(px_eq(_v1061, px_str("Add")))) {
        return px_str("px_add");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Sub")))) {
        return px_str("px_sub");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Mul")))) {
        return px_str("px_mul");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Div")))) {
        return px_str("px_div");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("IntDiv")))) {
        return px_str("px_idiv");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Mod")))) {
        return px_str("px_mod");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Pow")))) {
        return px_str("px_pow");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Eq")))) {
        return px_str("px_eq");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Ne")))) {
        return px_str("px_ne");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Lt")))) {
        return px_str("px_lt");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Le")))) {
        return px_str("px_le");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Gt")))) {
        return px_str("px_gt");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Ge")))) {
        return px_str("px_ge");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("BitAnd")))) {
        return px_str("px_bitand");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("BitOr")))) {
        return px_str("px_bitor");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("BitXor")))) {
        return px_str("px_bitxor");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Shl")))) {
        return px_str("px_shl");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("Shr")))) {
        return px_str("px_shr");
    }
    if (px_is_truthy(px_eq(_v1061, px_str("ShrU")))) {
        return px_str("px_ushr");
    }
    return px_str("px_add");
px_err_1062:
    if (px_err_1062_proped) return px_err_1062_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1063 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1064 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1065_val = px_null();
    int px_err_1065_proped = 0;
    LXValue _v1066 = px_index(_v1063, px_int(0LL));
    if (px_is_truthy(px_eq(_v1066, px_str("PatLiteral")))) {
        LXValue _v1067 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v1064), px_str(", ")), _v1067), px_str("))"));
    }
    if (px_is_truthy(px_eq(_v1066, px_str("PatBinding")))) {
        LXValue _v1068 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1);
        if (px_is_truthy(({ LXValue _t1070 = ({ LXValue _t1069 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1068}, 1), px_int(0LL)); px_is_truthy(_t1069) ? px_ge(px_index(_v1068, px_int(0LL)), px_str("A")) : _t1069; }); px_is_truthy(_t1070) ? px_le(px_index(_v1068, px_int(0LL)), px_str("Z")) : _t1070; }))) {
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v1064), px_str(".type == PX_ENUM && strcmp(")), _v1064), px_str(".as.obj->as.enum_inst.variant, \"")), _v1068), px_str("\") == 0)"));
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1066, px_str("PatWildcard")))) {
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1066, px_str("PatTuple")))) {
        LXValue _v1071 = px_index(_v1063, px_int(1LL));
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1071}, 1), px_int(0LL)))) {
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v1071, px_int(0LL)), _v1064}, 2);
        }
        return px_str("true");
    }
    if (px_is_truthy(px_eq(_v1066, px_str("PatConstructor")))) {
        LXValue _v1072 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v1064), px_str(".type == PX_ENUM && strcmp(")), _v1064), px_str(".as.obj->as.enum_inst.variant, \"")), _v1072), px_str("\") == 0)"));
    }
    return px_str("true");
px_err_1065:
    if (px_err_1065_proped) return px_err_1065_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1073 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1074 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1075 = px_null();
    LXValue _v1076 = px_null();
    LXValue px_err_1077_val = px_null();
    int px_err_1077_proped = 0;
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    LXValue _v1078 = px_get_global("cg_closure_id");
    LXValue _v1079 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1078}, 1));
    LXValue _v1080 = px_add(px_add(px_str("static LXValue "), _v1079), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1080 = px_add(_v1080, px_str("    (void)ctx;\n"));
    LXValue _v1081 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v1082 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1083 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1083, px_call(px_get_global("len"), (LXValue[]){_v1073}, 1)))) {
        LXValue _v1084 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v1073, _v1083)}, 1);
         _v1080 = px_add(_v1080, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1084), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1083}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1083}, 1)), px_str("] : px_null();\n")));
         _v1083 = px_add(_v1083, px_int(1LL));
    }
    LXValue _v1085 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v1074}, 1);
    LXValue _v1086 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1086}, 1));
     _v1080 = px_add(_v1080, px_add(px_add(px_str("    LXValue "), _v1086), px_str("_val = px_null();\n")));
     _v1080 = px_add(_v1080, px_add(px_add(px_str("    int "), _v1086), px_str("_proped = 0;\n")));
     _v1080 = px_add(_v1080, px_add(px_add(px_str("    return "), _v1085), px_str(";\n")));
     _v1080 = px_add(_v1080, px_add(_v1086, px_str(":\n")));
     _v1080 = px_add(_v1080, px_add(px_add(px_add(px_add(px_str("    if ("), _v1086), px_str("_proped) return ")), _v1086), px_str("_val;\n")));
     _v1080 = px_add(_v1080, px_str("    return px_null();\n"));
     _v1080 = px_add(_v1080, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1080));
    px_set_global("cg_vars", _v1081);
    px_set_global("cg_var_types", _v1082);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1078}, 1)), px_str(">\", ")), _v1079), px_str(", NULL)"));
px_err_1077:
    if (px_err_1077_proped) return px_err_1077_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1087 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1088 = px_null();
    LXValue px_err_1089_val = px_null();
    int px_err_1089_proped = 0;
    LXValue _v1090 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1087}, 1), px_int(1LL));
    while (px_is_truthy(px_ge(_v1090, px_int(0LL)))) {
        if (px_is_truthy(px_eq(px_index(_v1087, _v1090), px_str("/")))) {
            if (px_is_truthy(px_eq(_v1090, px_int(0LL)))) {
                return px_str("/");
            }
            return px_slice(_v1087, px_int(0LL), _v1090, px_null());
        }
         _v1090 = px_sub(_v1090, px_int(1LL));
    }
    return px_str(".");
px_err_1089:
    if (px_err_1089_proped) return px_err_1089_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1091 = px_null();
    LXValue px_err_1092_val = px_null();
    int px_err_1092_proped = 0;
    LXValue _v1093 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    if (px_is_truthy(({ LXValue _t1094 = px_ne(_v1093, px_null()); px_is_truthy(_t1094) ? px_call(px_get_global("exists"), (LXValue[]){_v1093}, 1) : _t1094; }))) {
        return _v1093;
    }
    LXValue _v1095 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    LXValue _v1096 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1096, px_call(px_get_global("len"), (LXValue[]){_v1095}, 1)))) {
        LXValue _v1097 = px_index(_v1095, _v1096);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1097}, 1))) {
            return _v1097;
        }
         _v1096 = px_add(_v1096, px_int(1LL));
    }
    return px_null();
px_err_1092:
    if (px_err_1092_proped) return px_err_1092_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1098 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1099 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1100 = px_null();
    LXValue _v1101 = px_null();
    LXValue _v1102 = px_null();
    LXValue _v1103 = px_null();
    LXValue px_err_1104_val = px_null();
    int px_err_1104_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1098}, 1), px_int(0LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t1106 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1098}, 1), px_int(1LL)); px_is_truthy(_t1106) ? ({ LXValue _t1105 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v1098, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1105) ? _t1105 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v1098, px_int(0LL)), px_str(".px")}, 2); }) : _t1106; }))) {
        LXValue _v1107 = px_index(_v1098, px_int(0LL));
        LXValue _v1108 = _v1107;
        if (px_is_truthy(px_not(({ LXValue _t1109 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1107}, 1), px_int(0LL)); px_is_truthy(_t1109) ? px_eq(px_index(_v1107, px_int(0LL)), px_str("/")) : _t1109; })))) {
             _v1108 = px_add(px_add(_v1099, px_str("/")), _v1107);
        }
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1108}, 1))) {
            return _v1108;
        }
        return px_null();
    }
    if (px_is_truthy(px_eq(px_index(_v1098, px_int(0LL)), px_str("std")))) {
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v1098}, 1), px_int(2LL)))) {
            return px_null();
        }
        LXValue _v1110 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        if (px_is_truthy(px_eq(_v1110, px_null()))) {
            return px_null();
        }
        LXValue _v1111 = _v1110;
        LXValue _v1112 = px_int(1LL);
        while (px_is_truthy(px_lt(_v1112, px_call(px_get_global("len"), (LXValue[]){_v1098}, 1)))) {
             _v1111 = px_add(_v1111, px_add(px_str("/"), px_index(_v1098, _v1112)));
             _v1112 = px_add(_v1112, px_int(1LL));
        }
        LXValue _v1113 = px_add(_v1111, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1113}, 1))) {
            return _v1113;
        }
        LXValue _v1114 = px_add(_v1111, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1114}, 1))) {
            return _v1114;
        }
        return px_null();
    }
    LXValue _v1115 = px_list_n((LXValue[]){_v1099}, 1);
    LXValue _v1116 = px_add(_v1099, px_str("/.px_modules"));
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1116}, 1))) {
        (void)(px_method(_v1115, "append", (LXValue[]){_v1116}, 1));
        LXValue _v1117 = px_call(px_get_global("list_dir"), (LXValue[]){_v1116}, 1);
        LXValue _v1118 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1118, px_call(px_get_global("len"), (LXValue[]){_v1117}, 1)))) {
            LXValue _v1119 = px_index(_v1117, _v1118);
            LXValue _v1120 = px_add(px_add(_v1116, px_str("/")), _v1119);
            if (px_is_truthy(({ LXValue _t1121 = px_call(px_get_global("exists"), (LXValue[]){_v1120}, 1); px_is_truthy(_t1121) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1119, px_str(".")}, 2)) : _t1121; }))) {
                (void)(px_method(_v1115, "append", (LXValue[]){_v1120}, 1));
            }
             _v1118 = px_add(_v1118, px_int(1LL));
        }
    }
    LXValue _v1122 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1122, px_call(px_get_global("len"), (LXValue[]){_v1115}, 1)))) {
        LXValue _v1123 = px_index(_v1115, _v1122);
        LXValue _v1124 = _v1123;
        LXValue _v1125 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1125, px_call(px_get_global("len"), (LXValue[]){_v1098}, 1)))) {
             _v1124 = px_add(_v1124, px_add(px_str("/"), px_index(_v1098, _v1125)));
             _v1125 = px_add(_v1125, px_int(1LL));
        }
        LXValue _v1126 = px_add(_v1124, px_str(".px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1126}, 1))) {
            return _v1126;
        }
        LXValue _v1127 = px_add(_v1124, px_str("/mod.px"));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1127}, 1))) {
            return _v1127;
        }
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1098}, 1), px_int(1LL)))) {
            LXValue _v1128 = px_add(px_add(px_add(_v1123, px_str("/")), px_index(_v1098, px_int(0LL))), px_str(".px"));
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1128}, 1))) {
                return _v1128;
            }
        }
         _v1122 = px_add(_v1122, px_int(1LL));
    }
    return px_null();
px_err_1104:
    if (px_err_1104_proped) return px_err_1104_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1129 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1130_val = px_null();
    int px_err_1130_proped = 0;
    LXValue _v1131 = px_index(_v1129, px_int(0LL));
    if (px_is_truthy(px_eq(_v1131, px_str("FuncDef")))) {
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1129, px_int(1LL))}, 1), px_str("main")))) {
            return px_bool(false);
        }
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t1134 = ({ LXValue _t1133 = ({ LXValue _t1132 = px_eq(_v1131, px_str("StructDef")); px_is_truthy(_t1132) ? _t1132 : px_eq(_v1131, px_str("EnumDef")); }); px_is_truthy(_t1133) ? _t1133 : px_eq(_v1131, px_str("TraitDef")); }); px_is_truthy(_t1134) ? _t1134 : px_eq(_v1131, px_str("ImplDef")); }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t1135 = px_eq(_v1131, px_str("VarDecl")); px_is_truthy(_t1135) ? px_eq(px_index(_v1129, px_int(1LL)), px_str("Const")) : _t1135; }))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_1130:
    if (px_err_1130_proped) return px_err_1130_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1136 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1137 = px_null();
    LXValue px_err_1138_val = px_null();
    int px_err_1138_proped = 0;
    LXValue _v1139 = px_index(_v1136, px_int(0LL));
    if (px_is_truthy(({ LXValue _t1142 = ({ LXValue _t1141 = ({ LXValue _t1140 = px_eq(_v1139, px_str("FuncDef")); px_is_truthy(_t1140) ? _t1140 : px_eq(_v1139, px_str("StructDef")); }); px_is_truthy(_t1141) ? _t1141 : px_eq(_v1139, px_str("EnumDef")); }); px_is_truthy(_t1142) ? _t1142 : px_eq(_v1139, px_str("TraitDef")); }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1136, px_int(1LL))}, 1);
    }
    if (px_is_truthy(({ LXValue _t1143 = px_eq(_v1139, px_str("VarDecl")); px_is_truthy(_t1143) ? px_eq(px_index(_v1136, px_int(1LL)), px_str("Const")) : _t1143; }))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1136, px_int(2LL))}, 1);
    }
    if (px_is_truthy(px_eq(_v1139, px_str("ImplDef")))) {
        LXValue _v1144 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1136, px_int(1LL))}, 1);
        LXValue _v1145 = px_index(_v1136, px_int(2LL));
        LXValue _v1146 = px_str("None");
        if (px_is_truthy(px_ne(_v1145, px_null()))) {
             _v1146 = px_add(px_add(px_str("Some("), _v1145), px_str(")"));
        }
        return px_add(px_add(px_add(px_str("impl::"), _v1144), px_str("::")), _v1146);
    }
    return px_null();
px_err_1138:
    if (px_err_1138_proped) return px_err_1138_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1147 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1148 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1149 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1150 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1151 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1152 = px_null();
    LXValue _v1153 = px_null();
    LXValue _v1154 = px_null();
    LXValue _v1155 = px_null();
    LXValue _v1156 = px_null();
    LXValue px_err_1157_val = px_null();
    int px_err_1157_proped = 0;
    LXValue _v1158 = px_list_n((LXValue[]){}, 0);
    LXValue _v1159 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1159, px_call(px_get_global("len"), (LXValue[]){_v1147}, 1)))) {
        (void)(px_method(_v1158, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1147, _v1159)}, 1)}, 1));
         _v1159 = px_add(_v1159, px_int(1LL));
    }
     _v1147 = _v1158;
    LXValue _v1160 = px_list_n((LXValue[]){}, 0);
    LXValue _v1161 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1161, px_call(px_get_global("len"), (LXValue[]){_v1148}, 1)))) {
        (void)(px_method(_v1160, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1148, _v1161)}, 1)}, 1));
         _v1161 = px_add(_v1161, px_int(1LL));
    }
     _v1148 = _v1160;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1147}, 1), px_int(0LL)))) {
        return px_null();
    }
    LXValue _v1162 = px_eq(px_index(_v1147, px_int(0LL)), px_str("std"));
    LXValue _v1163 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1147}, 2);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1163}, 1))) {
        return px_null();
    }
    LXValue _v1164 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1147, _v1149}, 2);
    if (px_is_truthy(px_eq(_v1164, px_null()))) {
        if (px_is_truthy(px_ne(px_index(_v1147, px_int(0LL)), px_str("std")))) {
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("[module] 警告: 找不到模块 '"), _v1163), px_str("'（已跳过）"))}, 1));
        }
        return px_null();
    }
    px_index_set(px_get_global("loaded"), _v1163, _v1164);
    LXValue _v1165 = px_call(px_get_global("read_file"), (LXValue[]){_v1164}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1165}, 1));
    px_set_global("p_pos", px_int(0LL));
    LXValue _v1166 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    LXValue _v1167 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1164}, 1);
    LXValue _v1168 = px_list_n((LXValue[]){}, 0);
    LXValue _v1169 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1169, px_call(px_get_global("len"), (LXValue[]){px_index(_v1166, px_int(1LL))}, 1)))) {
        LXValue _v1170 = px_index(px_index(_v1166, px_int(1LL)), _v1169);
        if (px_is_truthy(px_eq(px_index(_v1170, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v1168, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1170, px_int(1LL)), px_index(_v1170, px_int(2LL))}, 2)}, 1));
        }
         _v1169 = px_add(_v1169, px_int(1LL));
    }
    LXValue _v1171 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1171, px_call(px_get_global("len"), (LXValue[]){_v1168}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1168, _v1171), px_int(0LL)), px_index(px_index(_v1168, _v1171), px_int(1LL)), _v1167, _v1150, _v1151}, 5));
         _v1171 = px_add(_v1171, px_int(1LL));
    }
    LXValue _v1172 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1148}, 1), px_int(0LL));
    LXValue _v1173 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1173, px_call(px_get_global("len"), (LXValue[]){px_index(_v1166, px_int(1LL))}, 1)))) {
        LXValue _v1174 = px_index(px_index(_v1166, px_int(1LL)), _v1173);
        if (px_is_truthy(px_eq(px_index(_v1174, px_int(0LL)), px_str("Import")))) {
             _v1173 = px_add(_v1173, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1174}, 1)))) {
             _v1173 = px_add(_v1173, px_int(1LL));
            continue;
        }
        LXValue _v1175 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1174}, 1);
        if (px_is_truthy(px_eq(_v1175, px_null()))) {
            (void)(px_method(_v1150, "append", (LXValue[]){_v1174}, 1));
        }
        else {
            if (px_is_truthy(_v1172)) {
                if (px_is_truthy(({ LXValue _t1176 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1175}, 1), px_int(5LL)); px_is_truthy(_t1176) ? px_eq(px_slice(_v1175, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1176; }))) {
                     _v1173 = px_add(_v1173, px_int(1LL));
                    continue;
                }
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1148, _v1175}, 2)))) {
                     _v1173 = px_add(_v1173, px_int(1LL));
                    continue;
                }
            }
            if (px_is_truthy(_v1162)) {
                if (px_is_truthy(px_method(_v1151, "has", (LXValue[]){_v1175}, 1))) {
                     _v1173 = px_add(_v1173, px_int(1LL));
                    continue;
                }
                px_index_set(_v1151, _v1175, px_bool(true));
            }
            (void)(px_method(_v1150, "append", (LXValue[]){_v1174}, 1));
        }
         _v1173 = px_add(_v1173, px_int(1LL));
    }
px_err_1157:
    if (px_err_1157_proped) return px_err_1157_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1177 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1178 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1179 = px_null();
    LXValue _v1180 = px_null();
    LXValue _v1181 = px_null();
    LXValue _v1182 = px_null();
    LXValue px_err_1183_val = px_null();
    int px_err_1183_proped = 0;
    LXValue _v1184 = px_index(_v1177, px_int(1LL));
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1184}, 1), px_int(0LL)))) {
        return _v1177;
    }
    LXValue _v1185 = px_list_n((LXValue[]){}, 0);
    LXValue _v1186 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    LXValue _v1187 = px_list_n((LXValue[]){}, 0);
    LXValue _v1188 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1188, px_call(px_get_global("len"), (LXValue[]){_v1184}, 1)))) {
        LXValue _v1189 = px_index(_v1184, _v1188);
        if (px_is_truthy(px_eq(px_index(_v1189, px_int(0LL)), px_str("Import")))) {
            (void)(px_method(_v1187, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1189, px_int(1LL)), px_index(_v1189, px_int(2LL))}, 2)}, 1));
        }
         _v1188 = px_add(_v1188, px_int(1LL));
    }
    LXValue _v1190 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1190, px_call(px_get_global("len"), (LXValue[]){_v1187}, 1)))) {
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1187, _v1190), px_int(0LL)), px_index(px_index(_v1187, _v1190), px_int(1LL)), _v1178, _v1185, _v1186}, 5));
         _v1190 = px_add(_v1190, px_int(1LL));
    }
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1185}, 1), px_int(0LL)))) {
        return _v1177;
    }
    LXValue _v1191 = px_list_n((LXValue[]){}, 0);
    LXValue _v1192 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1192, px_call(px_get_global("len"), (LXValue[]){_v1185}, 1)))) {
        (void)(px_method(_v1191, "append", (LXValue[]){px_index(_v1185, _v1192)}, 1));
         _v1192 = px_add(_v1192, px_int(1LL));
    }
    LXValue _v1193 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1193, px_call(px_get_global("len"), (LXValue[]){_v1184}, 1)))) {
        (void)(px_method(_v1191, "append", (LXValue[]){px_index(_v1184, _v1193)}, 1));
         _v1193 = px_add(_v1193, px_int(1LL));
    }
    return px_list_n((LXValue[]){px_str("Program"), _v1191}, 2);
px_err_1183:
    if (px_err_1183_proped) return px_err_1183_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1194_val = px_null();
    int px_err_1194_proped = 0;
    LXValue _v1195 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v1195, "remove", (LXValue[]){px_str("_")}, 1));
    return _v1195;
px_err_1194:
    if (px_err_1194_proped) return px_err_1194_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1196 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1197 = px_null();
    LXValue px_err_1198_val = px_null();
    int px_err_1198_proped = 0;
    LXValue _v1199 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    LXValue _v1200 = px_method(_v1196, "keys", (LXValue[]){}, 0);
    LXValue _v1201 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1201, px_call(px_get_global("len"), (LXValue[]){_v1200}, 1)))) {
        px_index_set(_v1199, px_index(_v1200, _v1201), px_index(_v1196, px_index(_v1200, _v1201)));
         _v1201 = px_add(_v1201, px_int(1LL));
    }
    return _v1199;
px_err_1198:
    if (px_err_1198_proped) return px_err_1198_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1202_val = px_null();
    int px_err_1202_proped = 0;
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    return px_get_global("cg_uidc");
px_err_1202:
    if (px_err_1202_proped) return px_err_1202_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1203_val = px_null();
    int px_err_1203_proped = 0;
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1203:
    if (px_err_1203_proped) return px_err_1203_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1204 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1205_val = px_null();
    int px_err_1205_proped = 0;
    LXValue _v1206 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_index_set(px_get_global("cg_vars"), _v1204, _v1206);
    return _v1206;
px_err_1205:
    if (px_err_1205_proped) return px_err_1205_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1207 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1208_val = px_null();
    int px_err_1208_proped = 0;
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1207}, 1))) {
        return px_index(px_get_global("cg_vars"), _v1207);
    }
    return px_null();
px_err_1208:
    if (px_err_1208_proped) return px_err_1208_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1209 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1210_val = px_null();
    int px_err_1210_proped = 0;
    px_index_set(px_get_global("cg_immutables"), _v1209, px_int(1LL));
px_err_1210:
    if (px_err_1210_proped) return px_err_1210_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1211 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1212_val = px_null();
    int px_err_1212_proped = 0;
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1211}, 1);
px_err_1212:
    if (px_err_1212_proped) return px_err_1212_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1213 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1214 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1215_val = px_null();
    int px_err_1215_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1213), px_str(": ")), _v1214)}, 1));
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1215:
    if (px_err_1215_proped) return px_err_1215_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1216 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1217_val = px_null();
    int px_err_1217_proped = 0;
    if (px_is_truthy(px_eq(_v1216, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(px_index(_v1216, px_int(0LL)), px_str("TyOptional")))) {
        return px_bool(false);
    }
    return px_bool(true);
px_err_1217:
    if (px_err_1217_proped) return px_err_1217_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1218 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1219_val = px_null();
    int px_err_1219_proped = 0;
    if (px_is_truthy(px_eq(_v1218, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(px_index(_v1218, px_int(0LL)), px_str("Null")))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_1219:
    if (px_err_1219_proped) return px_err_1219_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1220 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1221_val = px_null();
    int px_err_1221_proped = 0;
    if (px_is_truthy(px_eq(_v1220, px_null()))) {
        return px_str("any");
    }
    if (px_is_truthy(px_eq(px_index(_v1220, px_int(0LL)), px_str("TyOptional")))) {
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1220, px_int(1LL))}, 1), px_str("?"));
    }
    if (px_is_truthy(px_eq(px_index(_v1220, px_int(0LL)), px_str("TyNamed")))) {
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1220, px_int(1LL))}, 1);
    }
    if (px_is_truthy(px_eq(px_index(_v1220, px_int(0LL)), px_str("TyList")))) {
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1220, px_int(1LL))}, 1)), px_str("]"));
    }
    if (px_is_truthy(px_eq(px_index(_v1220, px_int(0LL)), px_str("TyDict")))) {
        return px_add(px_add(px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1220, px_int(1LL))}, 1)), px_str(": ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1220, px_int(2LL))}, 1)), px_str("}"));
    }
    return px_str("any");
px_err_1221:
    if (px_err_1221_proped) return px_err_1221_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1222 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1223 = px_null();
    LXValue _v1224 = px_null();
    LXValue px_err_1225_val = px_null();
    int px_err_1225_proped = 0;
    LXValue _v1226 = px_str("");
    LXValue _v1227 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1227, px_call(px_get_global("len"), (LXValue[]){_v1222}, 1)))) {
        LXValue _v1228 = px_index(_v1222, _v1227);
        LXValue _v1230 = ({ LXValue _t1229 = px_ge(_v1228, px_str("a")); px_is_truthy(_t1229) ? px_le(_v1228, px_str("z")) : _t1229; });
        LXValue _v1232 = ({ LXValue _t1231 = px_ge(_v1228, px_str("A")); px_is_truthy(_t1231) ? px_le(_v1228, px_str("Z")) : _t1231; });
        LXValue _v1234 = ({ LXValue _t1233 = px_ge(_v1228, px_str("0")); px_is_truthy(_t1233) ? px_le(_v1228, px_str("9")) : _t1233; });
        if (px_is_truthy(({ LXValue _t1236 = ({ LXValue _t1235 = _v1230; px_is_truthy(_t1235) ? _t1235 : _v1232; }); px_is_truthy(_t1236) ? _t1236 : _v1234; }))) {
             _v1226 = px_add(_v1226, _v1228);
        }
        else {
             _v1226 = px_add(_v1226, px_str("_"));
        }
         _v1227 = px_add(_v1227, px_int(1LL));
    }
    return _v1226;
px_err_1225:
    if (px_err_1225_proped) return px_err_1225_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1237 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1238 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1239 = px_null();
    LXValue _v1240 = px_null();
    LXValue _v1241 = px_null();
    LXValue px_err_1242_val = px_null();
    int px_err_1242_proped = 0;
    LXValue _v1243 = px_call(px_get_global("len"), (LXValue[]){_v1237}, 1);
    LXValue _v1244 = px_call(px_get_global("len"), (LXValue[]){_v1238}, 1);
    LXValue _v1245 = px_int(0LL);
    while (px_is_truthy(px_le(px_add(_v1245, _v1244), _v1243))) {
        LXValue _v1246 = px_int(0LL);
        LXValue _v1247 = px_bool(true);
        while (px_is_truthy(px_lt(_v1246, _v1244))) {
            if (px_is_truthy(px_ne(px_index(_v1237, px_add(_v1245, _v1246)), px_index(_v1238, _v1246)))) {
                 _v1247 = px_bool(false);
                break;
            }
             _v1246 = px_add(_v1246, px_int(1LL));
        }
        if (px_is_truthy(_v1247)) {
            return _v1245;
        }
         _v1245 = px_add(_v1245, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_1242:
    if (px_err_1242_proped) return px_err_1242_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1248 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1249 = px_null();
    LXValue _v1250 = px_null();
    LXValue px_err_1251_val = px_null();
    int px_err_1251_proped = 0;
    LXValue _v1252 = px_str("");
    LXValue _v1253 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1253, _v1248))) {
         _v1252 = px_add(_v1252, px_str("    "));
         _v1253 = px_add(_v1253, px_int(1LL));
    }
    return _v1252;
px_err_1251:
    if (px_err_1251_proped) return px_err_1251_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1254 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1255 = px_null();
    LXValue _v1256 = px_null();
    LXValue _v1257 = px_null();
    LXValue _v1258 = px_null();
    LXValue px_err_1259_val = px_null();
    int px_err_1259_proped = 0;
    LXValue _v1260 = px_slice(_v1254, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1254}, 1), px_int(1LL)), px_null());
    LXValue _v1261 = px_str("");
    LXValue _v1262 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1262, px_call(px_get_global("len"), (LXValue[]){_v1260}, 1)))) {
        LXValue _v1263 = px_index(_v1260, _v1262);
        if (px_is_truthy(px_eq(_v1263, px_str("\\")))) {
            LXValue _v1264 = px_index(_v1260, px_add(_v1262, px_int(1LL)));
            if (px_is_truthy(px_eq(_v1264, px_str("n")))) {
                 _v1261 = px_add(_v1261, px_str("\n"));
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1264, px_str("t")))) {
                 _v1261 = px_add(_v1261, px_str("\t"));
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1264, px_str("r")))) {
                 _v1261 = px_add(_v1261, px_str("\r"));
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1264, px_str("0")))) {
                 _v1261 = px_add(_v1261, px_str(""));
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1264, px_str("\"")))) {
                 _v1261 = px_add(_v1261, px_str("\""));
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1264, px_str("\\")))) {
                 _v1261 = px_add(_v1261, px_str("\\"));
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1264, px_str("u")))) {
                LXValue _v1265 = px_add(_v1262, px_int(3LL));
                LXValue _v1266 = px_str("");
                while (px_is_truthy(({ LXValue _t1267 = px_lt(_v1265, px_call(px_get_global("len"), (LXValue[]){_v1260}, 1)); px_is_truthy(_t1267) ? px_ne(px_index(_v1260, _v1265), px_str("}")) : _t1267; }))) {
                     _v1266 = px_add(_v1266, px_index(_v1260, _v1265));
                     _v1265 = px_add(_v1265, px_int(1LL));
                }
                 _v1261 = px_add(_v1261, px_call(px_get_global("hex_to_char"), (LXValue[]){_v1266}, 1));
                 _v1262 = px_add(_v1265, px_int(1LL));
            }
            else {
                 _v1261 = px_add(_v1261, _v1264);
                 _v1262 = px_add(_v1262, px_int(2LL));
            }
        }
        else {
             _v1261 = px_add(_v1261, _v1263);
             _v1262 = px_add(_v1262, px_int(1LL));
        }
    }
    return _v1261;
px_err_1259:
    if (px_err_1259_proped) return px_err_1259_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1268 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1269 = px_null();
    LXValue _v1270 = px_null();
    LXValue px_err_1271_val = px_null();
    int px_err_1271_proped = 0;
    LXValue _v1272 = px_str("");
    LXValue _v1273 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1273, px_call(px_get_global("len"), (LXValue[]){_v1268}, 1)))) {
        LXValue _v1274 = px_index(_v1268, _v1273);
        if (px_is_truthy(px_eq(_v1274, px_str("\\")))) {
             _v1272 = px_add(_v1272, px_str("\\\\"));
        }
        else if (px_is_truthy(px_eq(_v1274, px_str("\"")))) {
             _v1272 = px_add(_v1272, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v1274, px_str("\n")))) {
             _v1272 = px_add(_v1272, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v1274, px_str("\r")))) {
             _v1272 = px_add(_v1272, px_str("\\r"));
        }
        else if (px_is_truthy(px_eq(_v1274, px_str("\t")))) {
             _v1272 = px_add(_v1272, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v1274, px_str("")))) {
             _v1272 = px_add(_v1272, px_str(""));
        }
        else {
             _v1272 = px_add(_v1272, _v1274);
        }
         _v1273 = px_add(_v1273, px_int(1LL));
    }
    return _v1272;
px_err_1271:
    if (px_err_1271_proped) return px_err_1271_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1275 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1276 = px_null();
    LXValue _v1277 = px_null();
    LXValue px_err_1278_val = px_null();
    int px_err_1278_proped = 0;
    LXValue _v1279 = px_str("");
    LXValue _v1280 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1280, _v1275))) {
         _v1279 = px_add(_v1279, px_str("0"));
         _v1280 = px_add(_v1280, px_int(1LL));
    }
    return _v1279;
px_err_1278:
    if (px_err_1278_proped) return px_err_1278_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1281 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1282 = px_null();
    LXValue _v1283 = px_null();
    LXValue _v1284 = px_null();
    LXValue _v1285 = px_null();
    LXValue _v1286 = px_null();
    LXValue _v1287 = px_null();
    LXValue _v1288 = px_null();
    LXValue _v1289 = px_null();
    LXValue _v1290 = px_null();
    LXValue _v1291 = px_null();
    LXValue _v1292 = px_null();
    LXValue px_err_1293_val = px_null();
    int px_err_1293_proped = 0;
    LXValue _v1294 = px_neg(px_int(1LL));
    LXValue _v1295 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1295, px_call(px_get_global("len"), (LXValue[]){_v1281}, 1)))) {
        if (px_is_truthy(({ LXValue _t1296 = px_eq(px_index(_v1281, _v1295), px_str("e")); px_is_truthy(_t1296) ? _t1296 : px_eq(px_index(_v1281, _v1295), px_str("E")); }))) {
             _v1294 = _v1295;
            break;
        }
         _v1295 = px_add(_v1295, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1294, px_int(0LL)))) {
        return _v1281;
    }
    LXValue _v1297 = px_slice(_v1281, px_int(0LL), _v1294, px_null());
    LXValue _v1298 = px_slice(_v1281, px_add(_v1294, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1281}, 1), px_null());
    LXValue _v1299 = px_int(1LL);
    if (px_is_truthy(({ LXValue _t1300 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1298}, 1), px_int(0LL)); px_is_truthy(_t1300) ? px_eq(px_index(_v1298, px_int(0LL)), px_str("+")) : _t1300; }))) {
         _v1298 = px_slice(_v1298, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1298}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1301 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1298}, 1), px_int(0LL)); px_is_truthy(_t1301) ? px_eq(px_index(_v1298, px_int(0LL)), px_str("-")) : _t1301; }))) {
         _v1299 = px_neg(px_int(1LL));
         _v1298 = px_slice(_v1298, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1298}, 1), px_null());
    }
    LXValue _v1302 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1298}, 1), _v1299);
    LXValue _v1303 = px_bool(false);
    if (px_is_truthy(({ LXValue _t1304 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1297}, 1), px_int(0LL)); px_is_truthy(_t1304) ? px_eq(px_index(_v1297, px_int(0LL)), px_str("-")) : _t1304; }))) {
         _v1303 = px_bool(true);
         _v1297 = px_slice(_v1297, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1297}, 1), px_null());
    }
    LXValue _v1305 = px_str("");
    LXValue _v1306 = px_str("");
    LXValue _v1307 = px_neg(px_int(1LL));
    LXValue _v1308 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1308, px_call(px_get_global("len"), (LXValue[]){_v1297}, 1)))) {
        if (px_is_truthy(px_eq(px_index(_v1297, _v1308), px_str(".")))) {
             _v1307 = _v1308;
            break;
        }
         _v1308 = px_add(_v1308, px_int(1LL));
    }
    if (px_is_truthy(px_lt(_v1307, px_int(0LL)))) {
         _v1305 = _v1297;
    }
    else {
         _v1305 = px_slice(_v1297, px_int(0LL), _v1307, px_null());
         _v1306 = px_slice(_v1297, px_add(_v1307, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1297}, 1), px_null());
    }
    LXValue _v1309 = px_add(_v1305, _v1306);
    LXValue _v1310 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1305}, 1), _v1302);
    LXValue _v1311 = px_str("");
    if (px_is_truthy(px_le(_v1310, px_int(0LL)))) {
         _v1311 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1310)}, 1)), _v1309);
    }
    else if (px_is_truthy(px_ge(_v1310, px_call(px_get_global("len"), (LXValue[]){_v1309}, 1)))) {
         _v1311 = px_add(_v1309, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1310, px_call(px_get_global("len"), (LXValue[]){_v1309}, 1))}, 1));
    }
    else {
         _v1311 = px_add(px_add(px_slice(_v1309, px_int(0LL), _v1310, px_null()), px_str(".")), px_slice(_v1309, _v1310, px_call(px_get_global("len"), (LXValue[]){_v1309}, 1), px_null()));
    }
    if (px_is_truthy(_v1303)) {
        return px_add(px_str("-"), _v1311);
    }
    return _v1311;
px_err_1293:
    if (px_err_1293_proped) return px_err_1293_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1312 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1313 = px_null();
    LXValue px_err_1314_val = px_null();
    int px_err_1314_proped = 0;
    LXValue _v1315 = px_call(px_get_global("str"), (LXValue[]){_v1312}, 1);
    if (px_is_truthy(({ LXValue _t1317 = ({ LXValue _t1316 = px_eq(_v1315, px_str("inf")); px_is_truthy(_t1316) ? _t1316 : px_eq(_v1315, px_str("-inf")); }); px_is_truthy(_t1317) ? _t1317 : px_eq(_v1315, px_str("nan")); }))) {
        return _v1315;
    }
     _v1315 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1315}, 1);
    LXValue _v1318 = px_call(px_get_global("len"), (LXValue[]){_v1315}, 1);
    if (px_is_truthy(({ LXValue _t1319 = px_ge(_v1318, px_int(2LL)); px_is_truthy(_t1319) ? px_eq(px_slice(_v1315, px_sub(_v1318, px_int(2LL)), _v1318, px_null()), px_str(".0")) : _t1319; }))) {
        return px_slice(_v1315, px_int(0LL), px_sub(_v1318, px_int(2LL)), px_null());
    }
    return _v1315;
px_err_1314:
    if (px_err_1314_proped) return px_err_1314_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1320 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1321 = px_null();
    LXValue _v1322 = px_null();
    LXValue _v1323 = px_null();
    LXValue _v1324 = px_null();
    LXValue px_err_1325_val = px_null();
    int px_err_1325_proped = 0;
    LXValue _v1326 = px_index(_v1320, px_int(1LL));
    LXValue _v1327 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1327, px_call(px_get_global("len"), (LXValue[]){_v1326}, 1)))) {
        LXValue _v1328 = px_index(_v1326, _v1327);
        LXValue _v1329 = px_index(_v1328, px_int(0LL));
        if (px_is_truthy(px_eq(_v1329, px_str("StructDef")))) {
            LXValue _v1330 = px_list_n((LXValue[]){}, 0);
            LXValue _v1331 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1331, px_call(px_get_global("len"), (LXValue[]){px_index(_v1328, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1330, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1328, px_int(2LL)), _v1331), px_int(1LL))}, 1)}, 1));
                 _v1331 = px_add(_v1331, px_int(1LL));
            }
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1328, px_int(1LL))}, 1), _v1330);
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("EnumDef")))) {
            LXValue _v1332 = px_list_n((LXValue[]){}, 0);
            LXValue _v1333 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1333, px_call(px_get_global("len"), (LXValue[]){px_index(_v1328, px_int(2LL))}, 1)))) {
                (void)(px_method(_v1332, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1328, px_int(2LL)), _v1333), px_int(1LL))}, 1)}, 1));
                 _v1333 = px_add(_v1333, px_int(1LL));
            }
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1328, px_int(1LL))}, 1), _v1332);
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("ImplDef")))) {
            LXValue _v1334 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1328, px_int(1LL))}, 1);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1334}, 1))) {
                LXValue _v1335 = px_int(0LL);
                while (px_is_truthy(px_lt(_v1335, px_call(px_get_global("len"), (LXValue[]){px_index(_v1328, px_int(3LL))}, 1)))) {
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1334), "append", (LXValue[]){px_index(px_index(_v1328, px_int(3LL)), _v1335)}, 1));
                     _v1335 = px_add(_v1335, px_int(1LL));
                }
            }
            else {
                px_index_set(px_get_global("cg_impls"), _v1334, px_index(_v1328, px_int(3LL)));
            }
        }
         _v1327 = px_add(_v1327, px_int(1LL));
    }
px_err_1325:
    if (px_err_1325_proped) return px_err_1325_val;
    return px_null();
}

static LXValue fn_cg_collect_assign_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1336 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1337 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1338 = px_null();
    LXValue _v1339 = px_null();
    LXValue px_err_1340_val = px_null();
    int px_err_1340_proped = 0;
    LXValue _v1341 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1341, px_call(px_get_global("len"), (LXValue[]){_v1336}, 1)))) {
        LXValue _v1342 = px_index(_v1336, _v1341);
        LXValue _v1343 = px_index(_v1342, px_int(0LL));
        if (px_is_truthy(px_eq(_v1343, px_str("Assign")))) {
            LXValue _v1344 = px_index(_v1342, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1344, px_int(0LL)), px_str("Var")))) {
                LXValue _v1345 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1344, px_int(1LL))}, 1);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1337, _v1345}, 2)))) {
                    (void)(px_method(_v1337, "append", (LXValue[]){_v1345}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1343, px_str("If")))) {
            LXValue _v1346 = px_index(_v1342, px_int(1LL));
            LXValue _v1347 = px_int(0LL);
            while (px_is_truthy(px_lt(_v1347, px_call(px_get_global("len"), (LXValue[]){_v1346}, 1)))) {
                (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(px_index(_v1346, _v1347), px_int(1LL)), _v1337}, 2));
                 _v1347 = px_add(_v1347, px_int(1LL));
            }
            if (px_is_truthy(px_ne(px_index(_v1342, px_int(2LL)), px_null()))) {
                (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1342, px_int(2LL)), _v1337}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1343, px_str("For")))) {
            (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1342, px_int(3LL)), _v1337}, 2));
        }
        else if (px_is_truthy(px_eq(_v1343, px_str("While")))) {
            (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1342, px_int(2LL)), _v1337}, 2));
        }
         _v1341 = px_add(_v1341, px_int(1LL));
    }
px_err_1340:
    if (px_err_1340_proped) return px_err_1340_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1348 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1349_val = px_null();
    int px_err_1349_proped = 0;
    LXValue _v1350 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1348, px_int(1LL))}, 1)}, 1));
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1348, _v1350}, 2);
px_err_1349:
    if (px_err_1349_proped) return px_err_1349_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1351 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1352 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1353 = px_null();
    LXValue _v1354 = px_null();
    LXValue _v1355 = px_null();
    LXValue _v1356 = px_null();
    LXValue _v1357 = px_null();
    LXValue px_err_1358_val = px_null();
    int px_err_1358_proped = 0;
    LXValue _v1359 = px_add(px_add(px_str("static LXValue "), _v1352), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
     _v1359 = px_add(_v1359, px_str("    (void)ctx;\n"));
    LXValue _v1360 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    LXValue _v1361 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    LXValue _v1362 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1362}, 1));
    LXValue _v1363 = px_index(_v1351, px_int(2LL));
    LXValue _v1364 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1364, px_call(px_get_global("len"), (LXValue[]){_v1363}, 1)))) {
        LXValue _v1365 = px_index(_v1363, _v1364);
        LXValue _v1366 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1365, px_int(1LL))}, 1)}, 1);
        LXValue _v1367 = px_str("px_null()");
        if (px_is_truthy(px_ne(px_index(_v1365, px_int(3LL)), px_null()))) {
             _v1367 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1365, px_int(3LL))}, 1);
        }
         _v1359 = px_add(_v1359, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1366), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1364}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1364}, 1)), px_str("] : ")), _v1367), px_str(";\n")));
         _v1364 = px_add(_v1364, px_int(1LL));
    }
    LXValue _v1368 = px_list_n((LXValue[]){}, 0);
    (void)(px_call(px_get_global("cg_collect_assign_vars"), (LXValue[]){px_index(_v1351, px_int(4LL)), _v1368}, 2));
    LXValue _v1369 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1369, px_call(px_get_global("len"), (LXValue[]){_v1368}, 1)))) {
        LXValue _v1370 = px_index(_v1368, _v1369);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1370}, 1), px_null()))) {
             _v1369 = px_add(_v1369, px_int(1LL));
            continue;
        }
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1370}, 2))) {
             _v1369 = px_add(_v1369, px_int(1LL));
            continue;
        }
        LXValue _v1371 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1370}, 1);
         _v1359 = px_add(_v1359, px_add(px_add(px_str("    LXValue "), _v1371), px_str(" = px_null();\n")));
         _v1369 = px_add(_v1369, px_int(1LL));
    }
    LXValue _v1372 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1372}, 1));
     _v1359 = px_add(_v1359, px_add(px_add(px_str("    LXValue "), _v1372), px_str("_val = px_null();\n")));
     _v1359 = px_add(_v1359, px_add(px_add(px_str("    int "), _v1372), px_str("_proped = 0;\n")));
    LXValue _v1373 = px_index(_v1351, px_int(4LL));
    LXValue _v1374 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1374, px_call(px_get_global("len"), (LXValue[]){_v1373}, 1)))) {
         _v1359 = px_add(_v1359, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1373, _v1374), px_int(1LL)}, 2));
         _v1374 = px_add(_v1374, px_int(1LL));
    }
     _v1359 = px_add(_v1359, px_add(_v1372, px_str(":\n")));
     _v1359 = px_add(_v1359, px_add(px_add(px_add(px_add(px_str("    if ("), _v1372), px_str("_proped) return ")), _v1372), px_str("_val;\n")));
     _v1359 = px_add(_v1359, px_str("    return px_null();\n"));
     _v1359 = px_add(_v1359, px_str("}\n"));
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_set_global("cg_vars", _v1360);
    px_set_global("cg_var_types", _v1361);
    px_set_global("cg_immutables", _v1362);
    return _v1359;
px_err_1358:
    if (px_err_1358_proped) return px_err_1358_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1375 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1376 = px_null();
    LXValue _v1377 = px_null();
    LXValue _v1378 = px_null();
    LXValue _v1379 = px_null();
    LXValue _v1380 = px_null();
    LXValue _v1381 = px_null();
    LXValue _v1382 = px_null();
    LXValue _v1383 = px_null();
    LXValue _v1384 = px_null();
    LXValue _v1385 = px_null();
    LXValue _v1386 = px_null();
    LXValue _v1387 = px_null();
    LXValue _v1388 = px_null();
    LXValue _v1389 = px_null();
    LXValue px_err_1390_val = px_null();
    int px_err_1390_proped = 0;
    LXValue _v1391 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_set_global("cg_closures", px_str(""));
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_set_global("cg_uidc", px_int(0LL));
    px_set_global("cg_closure_id", px_int(0LL));
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1375}, 1));
    LXValue _v1392 = px_index(_v1375, px_int(1LL));
    LXValue _v1393 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1393, px_call(px_get_global("len"), (LXValue[]){_v1392}, 1)))) {
        LXValue _v1394 = px_index(_v1392, _v1393);
        LXValue _v1395 = px_index(_v1394, px_int(0LL));
        if (px_is_truthy(px_eq(_v1395, px_str("FuncDef")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1394, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1395, px_str("VarDecl")))) {
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1394, px_int(2LL))}, 1)}, 1));
            if (px_is_truthy(({ LXValue _t1396 = px_eq(px_index(_v1394, px_int(1LL)), px_str("Let")); px_is_truthy(_t1396) ? _t1396 : px_eq(px_index(_v1394, px_int(1LL)), px_str("Const")); }))) {
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1394, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1395, px_str("Assign")))) {
            LXValue _v1397 = px_index(_v1394, px_int(1LL));
            if (px_is_truthy(px_eq(px_index(_v1397, px_int(0LL)), px_str("Var")))) {
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1397, px_int(1LL))}, 1)}, 1));
            }
        }
         _v1393 = px_add(_v1393, px_int(1LL));
    }
    LXValue _v1398 = px_list_n((LXValue[]){}, 0);
    LXValue _v1399 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    LXValue _v1400 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1400, px_call(px_get_global("len"), (LXValue[]){_v1399}, 1)))) {
        LXValue _v1401 = px_index(_v1399, _v1400);
        LXValue _v1402 = px_index(px_get_global("cg_impls"), _v1401);
        LXValue _v1403 = px_int(0LL);
        while (px_is_truthy(px_lt(_v1403, px_call(px_get_global("len"), (LXValue[]){_v1402}, 1)))) {
            (void)(px_method(_v1398, "append", (LXValue[]){px_list_n((LXValue[]){_v1401, px_index(_v1402, _v1403)}, 2)}, 1));
             _v1403 = px_add(_v1403, px_int(1LL));
        }
         _v1400 = px_add(_v1400, px_int(1LL));
    }
    LXValue _v1404 = px_int(1LL);
    while (px_is_truthy(px_lt(_v1404, px_call(px_get_global("len"), (LXValue[]){_v1398}, 1)))) {
        LXValue _v1405 = _v1404;
        while (px_is_truthy(px_gt(_v1405, px_int(0LL)))) {
            LXValue _v1406 = px_add(px_add(px_index(px_index(_v1398, px_sub(_v1405, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1398, px_sub(_v1405, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            LXValue _v1407 = px_add(px_add(px_index(px_index(_v1398, _v1405), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1398, _v1405), px_int(1LL)), px_int(1LL))}, 1));
            if (px_is_truthy(px_lt(_v1407, _v1406))) {
                LXValue _v1408 = px_index(_v1398, _v1405);
                px_index_set(_v1398, _v1405, px_index(_v1398, px_sub(_v1405, px_int(1LL))));
                px_index_set(_v1398, px_sub(_v1405, px_int(1LL)), _v1408);
            }
             _v1405 = px_sub(_v1405, px_int(1LL));
        }
         _v1404 = px_add(_v1404, px_int(1LL));
    }
    LXValue _v1409 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1409, px_call(px_get_global("len"), (LXValue[]){_v1398}, 1)))) {
        LXValue _v1410 = px_index(px_index(_v1398, _v1409), px_int(0LL));
        LXValue _v1411 = px_index(px_index(_v1398, _v1409), px_int(1LL));
        LXValue _v1412 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1410}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1411, px_int(1LL))}, 1)}, 1));
         _v1391 = px_add(_v1391, px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1411, _v1412}, 2));
         _v1391 = px_add(_v1391, px_str("\n"));
         _v1409 = px_add(_v1409, px_int(1LL));
    }
    LXValue _v1413 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1413, px_call(px_get_global("len"), (LXValue[]){_v1392}, 1)))) {
        LXValue _v1414 = px_index(_v1392, _v1413);
        if (px_is_truthy(px_eq(px_index(_v1414, px_int(0LL)), px_str("FuncDef")))) {
             _v1391 = px_add(_v1391, px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1414}, 1));
             _v1391 = px_add(_v1391, px_str("\n"));
        }
         _v1413 = px_add(_v1413, px_int(1LL));
    }
     _v1391 = px_add(_v1391, px_str("int main(int argc, char** argv) {\n"));
     _v1391 = px_add(_v1391, px_str("    px_args_init(argc, argv);\n"));
     _v1391 = px_add(_v1391, px_str("    px_register_builtins();\n"));
    LXValue _v1415 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1415, px_call(px_get_global("len"), (LXValue[]){_v1392}, 1)))) {
        LXValue _v1416 = px_index(_v1392, _v1415);
        if (px_is_truthy(px_eq(px_index(_v1416, px_int(0LL)), px_str("FuncDef")))) {
            LXValue _v1417 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1416, px_int(1LL))}, 1);
            LXValue _v1418 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1417}, 1));
             _v1391 = px_add(_v1391, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1417), px_str("\", px_func(\"")), _v1417), px_str("\", ")), _v1418), px_str(", NULL));\n")));
        }
         _v1415 = px_add(_v1415, px_int(1LL));
    }
    LXValue _v1419 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1419, px_call(px_get_global("len"), (LXValue[]){_v1398}, 1)))) {
        LXValue _v1420 = px_index(px_index(_v1398, _v1419), px_int(0LL));
        LXValue _v1421 = px_index(px_index(_v1398, _v1419), px_int(1LL));
        LXValue _v1422 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1421, px_int(1LL))}, 1);
        LXValue _v1423 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1420}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1422}, 1));
         _v1391 = px_add(_v1391, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1420), px_str(".")), _v1422), px_str("\", px_func(\"")), _v1420), px_str(".")), _v1422), px_str("\", ")), _v1423), px_str(", NULL));\n")));
         _v1419 = px_add(_v1419, px_int(1LL));
    }
    LXValue _v1424 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1424, px_call(px_get_global("len"), (LXValue[]){_v1392}, 1)))) {
        LXValue _v1425 = px_index(_v1392, _v1424);
        LXValue _v1426 = px_index(_v1425, px_int(0LL));
        if (px_is_truthy(({ LXValue _t1431 = ({ LXValue _t1430 = ({ LXValue _t1429 = ({ LXValue _t1428 = ({ LXValue _t1427 = px_ne(_v1426, px_str("FuncDef")); px_is_truthy(_t1427) ? px_ne(_v1426, px_str("StructDef")) : _t1427; }); px_is_truthy(_t1428) ? px_ne(_v1426, px_str("EnumDef")) : _t1428; }); px_is_truthy(_t1429) ? px_ne(_v1426, px_str("TraitDef")) : _t1429; }); px_is_truthy(_t1430) ? px_ne(_v1426, px_str("ImplDef")) : _t1430; }); px_is_truthy(_t1431) ? px_ne(_v1426, px_str("Import")) : _t1431; }))) {
             _v1391 = px_add(_v1391, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1425, px_int(1LL)}, 2));
        }
         _v1424 = px_add(_v1424, px_int(1LL));
    }
    LXValue _v1432 = px_bool(false);
    LXValue _v1433 = px_int(0LL);
    while (px_is_truthy(px_lt(_v1433, px_call(px_get_global("len"), (LXValue[]){_v1392}, 1)))) {
        LXValue _v1434 = px_index(_v1392, _v1433);
        if (px_is_truthy(({ LXValue _t1435 = px_eq(px_index(_v1434, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1435) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1434, px_int(1LL))}, 1), px_str("main")) : _t1435; }))) {
             _v1432 = px_bool(true);
            break;
        }
         _v1433 = px_add(_v1433, px_int(1LL));
    }
    if (px_is_truthy(_v1432)) {
        LXValue _v1436 = px_str("fn_main");
         _v1391 = px_add(_v1391, px_add(px_add(px_str("    { LXValue _r = "), _v1436), px_str("(NULL, 0, NULL); int _code = 0;\n")));
         _v1391 = px_add(_v1391, px_str("      if (px_is_result(_r)) {\n"));
         _v1391 = px_add(_v1391, px_str("        if (!px_result_ok(_r)) {\n"));
         _v1391 = px_add(_v1391, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
         _v1391 = px_add(_v1391, px_str("          _code = 1;\n"));
         _v1391 = px_add(_v1391, px_str("        } else {\n"));
         _v1391 = px_add(_v1391, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
         _v1391 = px_add(_v1391, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
         _v1391 = px_add(_v1391, px_str("        }\n"));
         _v1391 = px_add(_v1391, px_str("      } else if (_r.type == PX_INT) {\n"));
         _v1391 = px_add(_v1391, px_str("        _code = (int)_r.as.i;\n"));
         _v1391 = px_add(_v1391, px_str("      }\n"));
         _v1391 = px_add(_v1391, px_str("      return _code;\n"));
         _v1391 = px_add(_v1391, px_str("    }\n"));
    }
    else {
         _v1391 = px_add(_v1391, px_str("    return 0;\n"));
    }
     _v1391 = px_add(_v1391, px_str("}\n"));
    LXValue _v1437 = px_call(px_get_global("cg_find"), (LXValue[]){_v1391, px_str("int main(")}, 2);
    if (px_is_truthy(px_ge(_v1437, px_int(0LL)))) {
        LXValue _v1438 = px_slice(_v1391, px_int(0LL), _v1437, px_null());
        LXValue _v1439 = px_slice(_v1391, _v1437, px_call(px_get_global("len"), (LXValue[]){_v1391}, 1), px_null());
        LXValue _v1440 = px_call(px_get_global("cg_find"), (LXValue[]){_v1438, px_str("static LXValue")}, 2);
        LXValue _v1441 = px_str("");
        if (px_is_truthy(px_ge(_v1440, px_int(0LL)))) {
             _v1441 = px_add(px_add(px_add(px_add(px_slice(_v1438, px_int(0LL), _v1440, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1438, _v1440, px_call(px_get_global("len"), (LXValue[]){_v1438}, 1), px_null())), _v1439);
        }
        else {
             _v1441 = px_add(px_add(px_add(_v1438, px_get_global("cg_closures")), px_str("\n")), _v1439);
        }
        return _v1441;
    }
    return _v1391;
px_err_1390:
    if (px_err_1390_proped) return px_err_1390_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1442 = px_null();
    LXValue px_err_1443_val = px_null();
    int px_err_1443_proped = 0;
    LXValue _v1444 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    LXValue _v1445 = px_index(_v1444, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1444}, 1), px_int(1LL)));
    LXValue _v1446 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1445}, 1);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v1445}, 1)}, 1));
    px_set_global("p_pos", px_int(0LL));
    LXValue _v1447 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    LXValue _v1448 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v1447, _v1446}, 2);
    LXValue _v1449 = px_call(px_get_global("cg_generate"), (LXValue[]){_v1448}, 1);
    LXValue _v1450 = px_call(px_get_global("len"), (LXValue[]){_v1449}, 1);
    if (px_is_truthy(({ LXValue _t1451 = px_gt(_v1450, px_int(0LL)); px_is_truthy(_t1451) ? px_eq(px_index(_v1449, px_sub(_v1450, px_int(1LL))), px_str("\n")) : _t1451; }))) {
         _v1449 = px_slice(_v1449, px_int(0LL), px_sub(_v1450, px_int(1LL)), px_null());
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1449}, 1));
px_err_1443:
    if (px_err_1443_proped) return px_err_1443_val;
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
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
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
