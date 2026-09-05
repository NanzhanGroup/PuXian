/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_peek(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("peek");
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    px_srcline(30);
    if (px_is_truthy(px_lt(px_get_global("g_pos"), px_get_global("g_len")))) {
        px_srcline(31);
        return px_index(px_get_global("g_src"), px_get_global("g_pos"));
    }
    px_srcline(32);
    return px_str("");
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
    return px_null();
}

static LXValue fn_peek2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("peek2");
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcline(34);
    if (px_is_truthy(px_lt(px_add(px_get_global("g_pos"), px_int(1LL)), px_get_global("g_len")))) {
        px_srcline(35);
        return px_index(px_get_global("g_src"), px_add(px_get_global("g_pos"), px_int(1LL)));
    }
    px_srcline(36);
    return px_str("");
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_peek3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("peek3");
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    px_srcline(38);
    if (px_is_truthy(px_lt(px_add(px_get_global("g_pos"), px_int(2LL)), px_get_global("g_len")))) {
        px_srcline(39);
        return px_index(px_get_global("g_src"), px_add(px_get_global("g_pos"), px_int(2LL)));
    }
    px_srcline(40);
    return px_str("");
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_advance(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("advance");
    LXValue _v4 = px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(42);
    if (px_is_truthy(px_ge(px_get_global("g_pos"), px_get_global("g_len")))) {
        px_srcline(43);
        return px_str("");
    }
    px_srcline(44);
    _v4 = px_index(px_get_global("g_src"), px_get_global("g_pos"));
    px_srcline(45);
    px_set_global("g_pos", px_add(px_get_global("g_pos"), px_int(1LL)));
    px_srcline(46);
    if (px_is_truthy(px_eq(_v4, px_str("\n")))) {
        px_srcline(47);
        px_set_global("g_line", px_add(px_get_global("g_line"), px_int(1LL)));
        px_srcline(48);
        px_set_global("g_col", px_int(1LL));
    }
    else {
        px_srcline(50);
        px_set_global("g_col", px_add(px_get_global("g_col"), px_int(1LL)));
    }
    px_srcline(51);
    return _v4;
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_emit_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("emit_at");
    LXValue _v6 = (nargs > 0) ? args[0] : px_null();
    LXValue _v7 = (nargs > 1) ? args[1] : px_null();
    LXValue _v8 = (nargs > 2) ? args[2] : px_null();
    LXValue _v9 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    px_srcline(54);
    (void)(px_method(px_get_global("g_toks"), "append", (LXValue[]){px_list_n((LXValue[]){_v6, _v7, _v8, _v9}, 4)}, 1));
    px_srcline(55);
    px_set_global("g_count", px_add(px_get_global("g_count"), px_int(1LL)));
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_emit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("emit");
    LXValue _v11 = (nargs > 0) ? args[0] : px_null();
    LXValue _v12 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    px_srcline(57);
    (void)(px_call(px_get_global("emit_at"), (LXValue[]){_v11, _v12, px_get_global("g_line"), px_get_global("g_col")}, 4));
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

static LXValue fn_emit_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("emit_token");
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_15_val = px_null();
    int px_err_15_proped = 0;
    px_srcline(59);
    (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_index(_v14, px_int(0LL)), px_index(_v14, px_int(1LL)), px_index(_v14, px_int(2LL)), px_index(_v14, px_int(3LL))}, 4));
px_err_15:
    if (px_err_15_proped) return px_err_15_val;
    return px_null();
}

static LXValue fn_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("err");
    LXValue _v16 = (nargs > 0) ? args[0] : px_null();
    LXValue _v17 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_18_val = px_null();
    int px_err_18_proped = 0;
    px_srcline(61);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_line")}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_col")}, 1)), px_str(": 词法错误 ")), _v16), px_str(": ")), _v17)}, 1));
    px_srcline(62);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("lex "), _v16)}, 1));
px_err_18:
    if (px_err_18_proped) return px_err_18_val;
    return px_null();
}

static LXValue fn_err_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("err_at");
    LXValue _v19 = (nargs > 0) ? args[0] : px_null();
    LXValue _v20 = (nargs > 1) ? args[1] : px_null();
    LXValue _v21 = (nargs > 2) ? args[2] : px_null();
    LXValue _v22 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_23_val = px_null();
    int px_err_23_proped = 0;
    px_srcline(64);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){_v21}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){_v22}, 1)), px_str(": 词法错误 ")), _v19), px_str(": ")), _v20)}, 1));
    px_srcline(65);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("lex "), _v19)}, 1));
px_err_23:
    if (px_err_23_proped) return px_err_23_val;
    return px_null();
}

static LXValue fn_is_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_digit");
    LXValue _v24 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_25_val = px_null();
    int px_err_25_proped = 0;
    px_srcline(68);
    return ({ LXValue _t27 = ({ LXValue _t26 = px_ne(_v24, px_str("")); px_is_truthy(_t26) ? px_ge(_v24, px_str("0")) : _t26; }); px_is_truthy(_t27) ? px_le(_v24, px_str("9")) : _t27; });
px_err_25:
    if (px_err_25_proped) return px_err_25_val;
    return px_null();
}

static LXValue fn_is_hex_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_hex_digit");
    LXValue _v28 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_29_val = px_null();
    int px_err_29_proped = 0;
    px_srcline(70);
    return ({ LXValue _t33 = ({ LXValue _t31 = px_call(px_get_global("is_digit"), (LXValue[]){_v28}, 1); px_is_truthy(_t31) ? _t31 : ({ LXValue _t30 = px_ge(_v28, px_str("a")); px_is_truthy(_t30) ? px_le(_v28, px_str("f")) : _t30; }); }); px_is_truthy(_t33) ? _t33 : ({ LXValue _t32 = px_ge(_v28, px_str("A")); px_is_truthy(_t32) ? px_le(_v28, px_str("F")) : _t32; }); });
px_err_29:
    if (px_err_29_proped) return px_err_29_val;
    return px_null();
}

static LXValue fn_is_alnum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_alnum");
    LXValue _v34 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_35_val = px_null();
    int px_err_35_proped = 0;
    px_srcline(72);
    return ({ LXValue _t39 = ({ LXValue _t37 = px_call(px_get_global("is_digit"), (LXValue[]){_v34}, 1); px_is_truthy(_t37) ? _t37 : ({ LXValue _t36 = px_ge(_v34, px_str("a")); px_is_truthy(_t36) ? px_le(_v34, px_str("z")) : _t36; }); }); px_is_truthy(_t39) ? _t39 : ({ LXValue _t38 = px_ge(_v34, px_str("A")); px_is_truthy(_t38) ? px_le(_v34, px_str("Z")) : _t38; }); });
px_err_35:
    if (px_err_35_proped) return px_err_35_val;
    return px_null();
}

static LXValue fn_is_ident_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_ident_start");
    LXValue _v40 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_41_val = px_null();
    int px_err_41_proped = 0;
    px_srcline(74);
    if (px_is_truthy(px_eq(_v40, px_str("")))) {
        px_srcline(75);
        return px_bool(false);
    }
    px_srcline(76);
    return ({ LXValue _t46 = ({ LXValue _t45 = ({ LXValue _t44 = ({ LXValue _t42 = px_ge(_v40, px_str("a")); px_is_truthy(_t42) ? px_le(_v40, px_str("z")) : _t42; }); px_is_truthy(_t44) ? _t44 : ({ LXValue _t43 = px_ge(_v40, px_str("A")); px_is_truthy(_t43) ? px_le(_v40, px_str("Z")) : _t43; }); }); px_is_truthy(_t45) ? _t45 : px_eq(_v40, px_str("_")); }); px_is_truthy(_t46) ? _t46 : px_ge(_v40, px_str("")); });
px_err_41:
    if (px_err_41_proped) return px_err_41_val;
    return px_null();
}

static LXValue fn_is_ident_continue(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_ident_continue");
    LXValue _v47 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_48_val = px_null();
    int px_err_48_proped = 0;
    px_srcline(78);
    if (px_is_truthy(px_eq(_v47, px_str("")))) {
        px_srcline(79);
        return px_bool(false);
    }
    px_srcline(80);
    return ({ LXValue _t49 = px_call(px_get_global("is_ident_start"), (LXValue[]){_v47}, 1); px_is_truthy(_t49) ? _t49 : px_call(px_get_global("is_digit"), (LXValue[]){_v47}, 1); });
px_err_48:
    if (px_err_48_proped) return px_err_48_val;
    return px_null();
}

static LXValue fn_digit_val(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("digit_val");
    LXValue _v50 = (nargs > 0) ? args[0] : px_null();
    LXValue _v51 = px_null();
    LXValue _v52 = px_null();
    LXValue px_err_53_val = px_null();
    int px_err_53_proped = 0;
    px_srcline(82);
    _v51 = px_str("0123456789abcdefABCDEF");
    px_srcline(83);
    _v52 = px_int(0LL);
    px_srcline(84);
    while (px_is_truthy(px_lt(_v52, px_call(px_get_global("len"), (LXValue[]){_v51}, 1)))) {
        px_srcline(85);
        if (px_is_truthy(px_eq(px_index(_v51, _v52), _v50))) {
            px_srcline(86);
            if (px_is_truthy(px_ge(_v52, px_int(16LL)))) {
                px_srcline(87);
                return px_sub(_v52, px_int(6LL));
            }
            px_srcline(88);
            return _v52;
        }
        px_srcline(89);
         _v52 = px_add(_v52, px_int(1LL));
    }
    px_srcline(90);
    return px_neg(px_int(1LL));
px_err_53:
    if (px_err_53_proped) return px_err_53_val;
    return px_null();
}

static LXValue fn_handle_line_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("handle_line_start");
    LXValue _v54 = px_null();
    LXValue _v55 = px_null();
    LXValue _v56 = px_null();
    LXValue px_err_57_val = px_null();
    int px_err_57_proped = 0;
    px_srcline(93);
    _v54 = px_int(0LL);
    px_srcline(94);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(95);
         _v54 = px_int(0LL);
        px_srcline(96);
        while (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(" ")))) {
            px_srcline(97);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(98);
             _v54 = px_add(_v54, px_int(1LL));
        }
        px_srcline(99);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\t")))) {
            px_srcline(100);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1003"), px_str("缩进禁止使用 tab，请使用空格")}, 2));
        }
        px_srcline(101);
        _v55 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(102);
        if (px_is_truthy(px_eq(_v55, px_str("")))) {
            px_srcline(103);
            return px_null();
        }
        px_srcline(104);
        if (px_is_truthy(px_eq(_v55, px_str("\n")))) {
            px_srcline(105);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(106);
            continue;
        }
        px_srcline(107);
        if (px_is_truthy(px_eq(_v55, px_str("#")))) {
            px_srcline(108);
            (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
            px_srcline(109);
            continue;
        }
        px_srcline(110);
        break;
    }
    px_srcline(111);
    _v56 = px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)));
    px_srcline(112);
    if (px_is_truthy(px_gt(_v54, _v56))) {
        px_srcline(113);
        (void)(px_method(px_get_global("g_indent_stack"), "append", (LXValue[]){_v54}, 1));
        px_srcline(114);
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("缩进"), px_str("")}, 2));
    }
    else if (px_is_truthy(px_lt(_v54, _v56))) {
        px_srcline(116);
        while (px_is_truthy(px_gt(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v54))) {
            px_srcline(117);
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            px_srcline(118);
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        px_srcline(119);
        if (px_is_truthy(px_ne(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v54))) {
            px_srcline(120);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E2002"), px_add(px_add(px_str("缩进不一致：当前缩进 "), px_call(px_get_global("str"), (LXValue[]){_v54}, 1)), px_str(" 与上层缩进不匹配"))}, 2));
        }
    }
    px_srcline(121);
    px_set_global("g_at_line_start", px_bool(false));
px_err_57:
    if (px_err_57_proped) return px_err_57_val;
    return px_null();
}

static LXValue fn_skip_comment(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_comment");
    LXValue _v58 = px_null();
    LXValue _v59 = px_null();
    LXValue _v60 = px_null();
    LXValue px_err_61_val = px_null();
    int px_err_61_proped = 0;
    px_srcline(124);
    _v58 = px_str("");
    px_srcline(125);
    if (px_is_truthy(px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")))) {
        px_srcline(126);
         _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(127);
         _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(128);
        _v59 = px_int(1LL);
        px_srcline(129);
        while (px_is_truthy(px_gt(_v59, px_int(0LL)))) {
            px_srcline(130);
            _v60 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            px_srcline(131);
            if (px_is_truthy(px_eq(_v60, px_str("")))) {
                px_srcline(132);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1002"), px_str("块注释未闭合（缺少 |#）")}, 2));
            }
            else if (px_is_truthy(({ LXValue _t62 = px_eq(_v60, px_str("#")); px_is_truthy(_t62) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")) : _t62; }))) {
                px_srcline(134);
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(135);
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(136);
                 _v59 = px_add(_v59, px_int(1LL));
            }
            else if (px_is_truthy(({ LXValue _t63 = px_eq(_v60, px_str("|")); px_is_truthy(_t63) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("#")) : _t63; }))) {
                px_srcline(138);
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(139);
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(140);
                 _v59 = px_sub(_v59, px_int(1LL));
            }
            else {
                px_srcline(142);
                 _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
        }
    }
    else {
        px_srcline(144);
        while (px_is_truthy(({ LXValue _t64 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t64) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\n")) : _t64; }))) {
            px_srcline(145);
             _v58 = px_add(_v58, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(146);
    return _v58;
px_err_61:
    if (px_err_61_proped) return px_err_61_val;
    return px_null();
}

static LXValue fn_scan_ident_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_ident_token");
    LXValue _v65 = px_null();
    LXValue _v66 = px_null();
    LXValue _v67 = px_null();
    LXValue px_err_68_val = px_null();
    int px_err_68_proped = 0;
    px_srcline(149);
    _v65 = px_get_global("g_line");
    px_srcline(150);
    _v66 = px_get_global("g_col");
    px_srcline(151);
    _v67 = px_str("");
    px_srcline(152);
    while (px_is_truthy(px_call(px_get_global("is_ident_continue"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
        px_srcline(153);
         _v67 = px_add(_v67, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(154);
    if (px_is_truthy(px_method(px_get_global("KEYWORDS"), "has", (LXValue[]){_v67}, 1))) {
        px_srcline(155);
        return px_list_n((LXValue[]){px_index(px_get_global("KEYWORDS"), _v67), px_str(""), _v65, _v66}, 4);
    }
    px_srcline(156);
    return px_list_n((LXValue[]){px_str("标识符"), _v67, _v65, _v66}, 4);
px_err_68:
    if (px_err_68_proped) return px_err_68_val;
    return px_null();
}

static LXValue fn_scan_radix_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_radix_token");
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
    px_srcline(159);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(160);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(161);
    _v72 = px_str("");
    px_srcline(162);
    while (px_is_truthy(({ LXValue _t79 = px_call(px_get_global("is_alnum"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t79) ? _t79 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
        px_srcline(163);
         _v72 = px_add(_v72, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(164);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v72}, 1), px_int(0LL)))) {
        px_srcline(165);
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_str("进制字面量缺少数字（基数 "), px_call(px_get_global("str"), (LXValue[]){_v69}, 1)), px_str("）"))}, 2));
    }
    px_srcline(166);
    _v73 = px_str("");
    px_srcline(167);
    _v74 = px_int(0LL);
    px_srcline(168);
    while (px_is_truthy(px_lt(_v74, px_call(px_get_global("len"), (LXValue[]){_v72}, 1)))) {
        px_srcline(169);
        if (px_is_truthy(px_ne(px_index(_v72, _v74), px_str("_")))) {
            px_srcline(170);
             _v73 = px_add(_v73, px_index(_v72, _v74));
        }
        px_srcline(171);
         _v74 = px_add(_v74, px_int(1LL));
    }
    px_srcline(172);
    _v75 = px_int(0LL);
    px_srcline(173);
    _v76 = px_int(0LL);
    px_srcline(174);
    while (px_is_truthy(px_lt(_v76, px_call(px_get_global("len"), (LXValue[]){_v73}, 1)))) {
        px_srcline(175);
        _v77 = px_call(px_get_global("digit_val"), (LXValue[]){px_index(_v73, _v76)}, 1);
        px_srcline(176);
        if (px_is_truthy(({ LXValue _t80 = px_lt(_v77, px_int(0LL)); px_is_truthy(_t80) ? _t80 : px_ge(_v77, _v69); }))) {
            px_srcline(177);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_add(px_str("无效 "), px_call(px_get_global("str"), (LXValue[]){_v69}, 1)), px_str("-进制字面量: ")), _v73)}, 2));
        }
        px_srcline(178);
         _v75 = px_add(px_mul(_v75, _v69), _v77);
        px_srcline(179);
         _v76 = px_add(_v76, px_int(1LL));
    }
    px_srcline(180);
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){_v75}, 1), _v70, _v71}, 4);
px_err_78:
    if (px_err_78_proped) return px_err_78_val;
    return px_null();
}

static LXValue fn_strip_leading_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("strip_leading_zeros");
    LXValue _v81 = (nargs > 0) ? args[0] : px_null();
    LXValue _v82 = px_null();
    LXValue px_err_83_val = px_null();
    int px_err_83_proped = 0;
    px_srcline(182);
    _v82 = px_int(0LL);
    px_srcline(183);
    while (px_is_truthy(({ LXValue _t84 = px_lt(_v82, px_sub(px_call(px_get_global("len"), (LXValue[]){_v81}, 1), px_int(1LL))); px_is_truthy(_t84) ? px_eq(px_index(_v81, _v82), px_str("0")) : _t84; }))) {
        px_srcline(184);
         _v82 = px_add(_v82, px_int(1LL));
    }
    px_srcline(185);
    return px_slice(_v81, _v82, px_null(), px_null());
px_err_83:
    if (px_err_83_proped) return px_err_83_val;
    return px_null();
}

static LXValue fn_scan_number_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_number_token");
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
    px_srcline(187);
    _v85 = px_get_global("g_line");
    px_srcline(188);
    _v86 = px_get_global("g_col");
    px_srcline(189);
    if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("0")))) {
        px_srcline(190);
        _v87 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
        px_srcline(191);
        if (px_is_truthy(({ LXValue _t100 = px_eq(_v87, px_str("x")); px_is_truthy(_t100) ? _t100 : px_eq(_v87, px_str("X")); }))) {
            px_srcline(192);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(16LL), _v85, _v86}, 3);
        }
        px_srcline(193);
        if (px_is_truthy(({ LXValue _t101 = px_eq(_v87, px_str("b")); px_is_truthy(_t101) ? _t101 : px_eq(_v87, px_str("B")); }))) {
            px_srcline(194);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(2LL), _v85, _v86}, 3);
        }
        px_srcline(195);
        if (px_is_truthy(({ LXValue _t102 = px_eq(_v87, px_str("o")); px_is_truthy(_t102) ? _t102 : px_eq(_v87, px_str("O")); }))) {
            px_srcline(196);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(8LL), _v85, _v86}, 3);
        }
    }
    px_srcline(197);
    _v88 = px_str("");
    px_srcline(198);
    while (px_is_truthy(({ LXValue _t103 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t103) ? _t103 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
        px_srcline(199);
         _v88 = px_add(_v88, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(200);
    _v89 = px_bool(false);
    px_srcline(201);
    if (px_is_truthy(({ LXValue _t104 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")); px_is_truthy(_t104) ? px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek2"), (LXValue[]){}, 0)}, 1) : _t104; }))) {
        px_srcline(202);
         _v89 = px_bool(true);
        px_srcline(203);
         _v88 = px_add(_v88, px_str("."));
        px_srcline(204);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(205);
        while (px_is_truthy(({ LXValue _t105 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t105) ? _t105 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
            px_srcline(206);
             _v88 = px_add(_v88, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(207);
    if (px_is_truthy(({ LXValue _t106 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("e")); px_is_truthy(_t106) ? _t106 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("E")); }))) {
        px_srcline(208);
        _v90 = px_get_global("g_pos");
        px_srcline(209);
        _v91 = px_get_global("g_line");
        px_srcline(210);
        _v92 = px_get_global("g_col");
        px_srcline(211);
        _v93 = px_str("");
        px_srcline(212);
         _v93 = px_add(_v93, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(213);
        if (px_is_truthy(({ LXValue _t107 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("+")); px_is_truthy(_t107) ? _t107 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")); }))) {
            px_srcline(214);
             _v93 = px_add(_v93, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        px_srcline(215);
        if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
            px_srcline(216);
            while (px_is_truthy(({ LXValue _t108 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t108) ? _t108 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
                px_srcline(217);
                 _v93 = px_add(_v93, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            px_srcline(218);
             _v88 = px_add(_v88, _v93);
            px_srcline(219);
             _v89 = px_bool(true);
        }
        else {
            px_srcline(221);
            px_set_global("g_pos", _v90);
            px_srcline(222);
            px_set_global("g_line", _v91);
            px_srcline(223);
            px_set_global("g_col", _v92);
        }
    }
    px_srcline(224);
    _v94 = px_str("");
    px_srcline(225);
    _v95 = px_int(0LL);
    px_srcline(226);
    while (px_is_truthy(px_lt(_v95, px_call(px_get_global("len"), (LXValue[]){_v88}, 1)))) {
        px_srcline(227);
        if (px_is_truthy(px_ne(px_index(_v88, _v95), px_str("_")))) {
            px_srcline(228);
             _v94 = px_add(_v94, px_index(_v88, _v95));
        }
        px_srcline(229);
         _v95 = px_add(_v95, px_int(1LL));
    }
    px_srcline(230);
    if (px_is_truthy(_v89)) {
        px_srcline(231);
        _v96 = px_call(px_get_global("float"), (LXValue[]){_v94}, 1);
        px_srcline(232);
        _v97 = px_call(px_get_global("str"), (LXValue[]){_v96}, 1);
        px_srcline(233);
        if (px_is_truthy(px_call(px_get_global("ends_with"), (LXValue[]){_v97, px_str(".0")}, 2))) {
            px_srcline(234);
             _v97 = px_slice(_v97, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v97}, 1), px_int(2LL)), px_null());
        }
        px_srcline(235);
        return px_list_n((LXValue[]){px_str("浮点"), _v97, _v85, _v86}, 4);
    }
    px_srcline(236);
    _v98 = px_call(px_get_global("strip_leading_zeros"), (LXValue[]){_v94}, 1);
    px_srcline(237);
    if (px_is_truthy(({ LXValue _t110 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v98}, 1), px_int(19LL)); px_is_truthy(_t110) ? _t110 : ({ LXValue _t109 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v98}, 1), px_int(19LL)); px_is_truthy(_t109) ? px_gt(_v98, px_str("9223372036854775807")) : _t109; }); }))) {
        px_srcline(238);
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_str("无效整数: "), _v88)}, 2));
    }
    px_srcline(239);
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v94}, 1)}, 1), _v85, _v86}, 4);
px_err_99:
    if (px_err_99_proped) return px_err_99_val;
    return px_null();
}

static LXValue fn_scan_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_string");
    LXValue _v111 = (nargs > 0) ? args[0] : px_null();
    LXValue _v112 = px_null();
    LXValue _v113 = px_null();
    LXValue px_err_114_val = px_null();
    int px_err_114_proped = 0;
    px_srcline(242);
    _v112 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){_v111}, 1);
    px_srcline(243);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_index(_v112, px_int(0LL))}, 1));
    px_srcline(244);
    _v113 = px_int(1LL);
    px_srcline(245);
    while (px_is_truthy(px_lt(_v113, px_call(px_get_global("len"), (LXValue[]){_v112}, 1)))) {
        px_srcline(246);
        (void)(px_method(px_get_global("g_pending"), "append", (LXValue[]){px_index(_v112, _v113)}, 1));
        px_srcline(247);
         _v113 = px_add(_v113, px_int(1LL));
    }
px_err_114:
    if (px_err_114_proped) return px_err_114_val;
    return px_null();
}

static LXValue fn_scan_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_string_tokens");
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
    px_srcline(249);
    _v116 = px_get_global("g_line");
    px_srcline(250);
    _v117 = px_get_global("g_col");
    px_srcline(251);
    _v118 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    px_srcline(252);
    if (px_is_truthy(({ LXValue _t126 = ({ LXValue _t125 = px_eq(_v118, px_str("\"")); px_is_truthy(_t125) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\"")) : _t125; }); px_is_truthy(_t126) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")) : _t126; }))) {
        px_srcline(253);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(254);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(255);
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("\""), _v116, _v117, _v115}, 4);
    }
    px_srcline(256);
    if (px_is_truthy(({ LXValue _t128 = ({ LXValue _t127 = px_eq(_v118, px_str("'")); px_is_truthy(_t127) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("'")) : _t127; }); px_is_truthy(_t128) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")) : _t128; }))) {
        px_srcline(257);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(258);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(259);
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("'"), _v116, _v117, _v115}, 4);
    }
    px_srcline(260);
    _v119 = px_list_n((LXValue[]){}, 0);
    px_srcline(261);
    _v120 = px_str("");
    px_srcline(262);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(263);
        _v121 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(264);
        if (px_is_truthy(px_eq(_v121, px_str("")))) {
            px_srcline(265);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_add(px_add(px_str("字符串未闭合（缺少 "), _v118), px_str("）")), _v116, _v117}, 4));
        }
        px_srcline(266);
        if (px_is_truthy(px_eq(_v121, _v118))) {
            px_srcline(267);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(268);
            break;
        }
        px_srcline(269);
        if (px_is_truthy(px_eq(_v121, px_str("\\")))) {
            px_srcline(270);
             _v120 = px_add(_v120, px_call(px_get_global("scan_escape"), (LXValue[]){_v116, _v117}, 2));
        }
        else if (px_is_truthy(px_eq(_v121, px_str("\n")))) {
            px_srcline(272);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v116, _v117}, 4));
        }
        else if (px_is_truthy(({ LXValue _t130 = ({ LXValue _t129 = px_eq(_v121, px_str("$")); px_is_truthy(_t129) ? _v115 : _t129; }); px_is_truthy(_t130) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t130; }))) {
            px_srcline(274);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(275);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(276);
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v120}, 1), _v116, _v117}, 4)}, 1));
            px_srcline(277);
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v116, _v117}, 4)}, 1));
            px_srcline(278);
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v116, _v117}, 4)}, 1));
            px_srcline(279);
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v116, _v117}, 4)}, 1));
            px_srcline(280);
            _v122 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v116, _v117}, 2);
            px_srcline(281);
            _v123 = px_int(0LL);
            px_srcline(282);
            while (px_is_truthy(px_lt(_v123, px_call(px_get_global("len"), (LXValue[]){_v122}, 1)))) {
                px_srcline(283);
                (void)(px_method(_v119, "append", (LXValue[]){px_index(_v122, _v123)}, 1));
                px_srcline(284);
                 _v123 = px_add(_v123, px_int(1LL));
            }
            px_srcline(285);
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v116, _v117}, 4)}, 1));
            px_srcline(286);
            (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v116, _v117}, 4)}, 1));
            px_srcline(287);
             _v120 = px_str("");
        }
        else {
            px_srcline(289);
             _v120 = px_add(_v120, _v121);
            px_srcline(290);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(291);
    (void)(px_method(_v119, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v120}, 1), _v116, _v117}, 4)}, 1));
    px_srcline(292);
    return _v119;
px_err_124:
    if (px_err_124_proped) return px_err_124_val;
    return px_null();
}

static LXValue fn_scan_multiline_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_multiline_string_tokens");
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
    px_srcline(294);
    _v135 = px_list_n((LXValue[]){}, 0);
    px_srcline(295);
    _v136 = px_str("");
    px_srcline(296);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(297);
        _v137 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(298);
        if (px_is_truthy(px_eq(_v137, px_str("")))) {
            px_srcline(299);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("多行字符串未闭合"), _v132, _v133}, 4));
        }
        px_srcline(300);
        if (px_is_truthy(({ LXValue _t142 = ({ LXValue _t141 = px_eq(_v137, _v131); px_is_truthy(_t141) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), _v131) : _t141; }); px_is_truthy(_t142) ? px_eq(px_call(px_get_global("peek3"), (LXValue[]){}, 0), _v131) : _t142; }))) {
            px_srcline(301);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(302);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(303);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(304);
            break;
        }
        px_srcline(305);
        if (px_is_truthy(px_eq(_v137, px_str("\\")))) {
            px_srcline(306);
             _v136 = px_add(_v136, px_call(px_get_global("scan_escape"), (LXValue[]){_v132, _v133}, 2));
        }
        else if (px_is_truthy(({ LXValue _t144 = ({ LXValue _t143 = px_eq(_v137, px_str("$")); px_is_truthy(_t143) ? _v134 : _t143; }); px_is_truthy(_t144) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t144; }))) {
            px_srcline(308);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(309);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(310);
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v136}, 1), _v132, _v133}, 4)}, 1));
            px_srcline(311);
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v132, _v133}, 4)}, 1));
            px_srcline(312);
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v132, _v133}, 4)}, 1));
            px_srcline(313);
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v132, _v133}, 4)}, 1));
            px_srcline(314);
            _v138 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v132, _v133}, 2);
            px_srcline(315);
            _v139 = px_int(0LL);
            px_srcline(316);
            while (px_is_truthy(px_lt(_v139, px_call(px_get_global("len"), (LXValue[]){_v138}, 1)))) {
                px_srcline(317);
                (void)(px_method(_v135, "append", (LXValue[]){px_index(_v138, _v139)}, 1));
                px_srcline(318);
                 _v139 = px_add(_v139, px_int(1LL));
            }
            px_srcline(319);
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v132, _v133}, 4)}, 1));
            px_srcline(320);
            (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v132, _v133}, 4)}, 1));
            px_srcline(321);
             _v136 = px_str("");
        }
        else {
            px_srcline(323);
             _v136 = px_add(_v136, _v137);
            px_srcline(324);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(325);
    (void)(px_method(_v135, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v136}, 1), _v132, _v133}, 4)}, 1));
    px_srcline(326);
    return _v135;
px_err_140:
    if (px_err_140_proped) return px_err_140_val;
    return px_null();
}

static LXValue fn_scan_interp_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_interp_expr");
    LXValue _v145 = (nargs > 0) ? args[0] : px_null();
    LXValue _v146 = (nargs > 1) ? args[1] : px_null();
    LXValue _v147 = px_null();
    LXValue _v148 = px_null();
    LXValue _v149 = px_null();
    LXValue _v150 = px_null();
    LXValue px_err_151_val = px_null();
    int px_err_151_proped = 0;
    px_srcline(328);
    _v147 = px_list_n((LXValue[]){}, 0);
    px_srcline(329);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(330);
        _v148 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(331);
        if (px_is_truthy(px_eq(_v148, px_str("")))) {
            px_srcline(332);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值 ${ 未闭合（缺少 }）"), _v145, _v146}, 4));
        }
        px_srcline(333);
        if (px_is_truthy(px_eq(_v148, px_str("}")))) {
            px_srcline(334);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(335);
            break;
        }
        px_srcline(336);
        if (px_is_truthy(px_eq(_v148, px_str("{")))) {
            px_srcline(337);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持 {} 字面量（dict/set），请先用变量保存")}, 2));
        }
        px_srcline(338);
        if (px_is_truthy(px_eq(_v148, px_str("\n")))) {
            px_srcline(339);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值表达式不能跨行"), _v145, _v146}, 4));
        }
        px_srcline(340);
        if (px_is_truthy(px_eq(_v148, px_str("#")))) {
            px_srcline(341);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持注释")}, 2));
        }
        px_srcline(342);
        if (px_is_truthy(({ LXValue _t152 = px_eq(_v148, px_str(" ")); px_is_truthy(_t152) ? _t152 : px_eq(_v148, px_str("\t")); }))) {
            px_srcline(343);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        else if (px_is_truthy(({ LXValue _t153 = px_eq(_v148, px_str("\"")); px_is_truthy(_t153) ? _t153 : px_eq(_v148, px_str("'")); }))) {
            px_srcline(345);
            _v149 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){px_bool(true)}, 1);
            px_srcline(346);
            _v150 = px_int(0LL);
            px_srcline(347);
            while (px_is_truthy(px_lt(_v150, px_call(px_get_global("len"), (LXValue[]){_v149}, 1)))) {
                px_srcline(348);
                (void)(px_method(_v147, "append", (LXValue[]){px_index(_v149, _v150)}, 1));
                px_srcline(349);
                 _v150 = px_add(_v150, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v148}, 1))) {
            px_srcline(351);
            (void)(px_method(_v147, "append", (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v148}, 1))) {
            px_srcline(353);
            (void)(px_method(_v147, "append", (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(355);
            (void)(px_method(_v147, "append", (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
        }
    }
    px_srcline(356);
    return _v147;
px_err_151:
    if (px_err_151_proped) return px_err_151_val;
    return px_null();
}

static LXValue fn_hex_to_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("hex_to_char");
    LXValue _v154 = (nargs > 0) ? args[0] : px_null();
    LXValue _v155 = px_null();
    LXValue _v156 = px_null();
    LXValue _v157 = px_null();
    LXValue _v158 = px_null();
    LXValue _v159 = px_null();
    LXValue px_err_160_val = px_null();
    int px_err_160_proped = 0;
    px_srcline(358);
    _v155 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v154}, 1);
    px_srcline(359);
    if (px_is_truthy(px_le(_v155, px_int(127LL)))) {
        px_srcline(360);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){_v155, px_int(1LL)}, 2)}, 1);
    }
    px_srcline(361);
    if (px_is_truthy(px_le(_v155, px_int(2047LL)))) {
        px_srcline(362);
        _v156 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(192LL), px_shr(_v155, px_int(6LL))), px_int(1LL)}, 2);
        px_srcline(363);
        _v157 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v155, px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(364);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v156, _v157}, 2)}, 1);
    }
    px_srcline(365);
    if (px_is_truthy(px_le(_v155, px_int(65535LL)))) {
        px_srcline(366);
        _v156 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(224LL), px_shr(_v155, px_int(12LL))), px_int(1LL)}, 2);
        px_srcline(367);
        _v157 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v155, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(368);
        _v158 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v155, px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(369);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v156, _v157, _v158}, 3)}, 1);
    }
    px_srcline(370);
    _v156 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(240LL), px_shr(_v155, px_int(18LL))), px_int(1LL)}, 2);
    px_srcline(371);
    _v157 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v155, px_int(12LL)), px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(372);
    _v158 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v155, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(373);
    _v159 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v155, px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(374);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v156, _v157, _v158, _v159}, 4)}, 1);
px_err_160:
    if (px_err_160_proped) return px_err_160_val;
    return px_null();
}

static LXValue fn_scan_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_escape");
    LXValue _v161 = (nargs > 0) ? args[0] : px_null();
    LXValue _v162 = (nargs > 1) ? args[1] : px_null();
    LXValue _v163 = px_null();
    LXValue _v164 = px_null();
    LXValue _v165 = px_null();
    LXValue _v166 = px_null();
    LXValue px_err_167_val = px_null();
    int px_err_167_proped = 0;
    px_srcline(376);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(377);
    _v163 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    px_srcline(378);
    if (px_is_truthy(px_eq(_v163, px_str("")))) {
        px_srcline(379);
        (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串在转义序列处意外结束"), _v161, _v162}, 4));
    }
    px_srcline(380);
    if (px_is_truthy(px_eq(_v163, px_str("n")))) {
        px_srcline(381);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(382);
        return px_str("\n");
    }
    px_srcline(383);
    if (px_is_truthy(px_eq(_v163, px_str("t")))) {
        px_srcline(384);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(385);
        return px_str("\t");
    }
    px_srcline(386);
    if (px_is_truthy(px_eq(_v163, px_str("r")))) {
        px_srcline(387);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(388);
        return px_str("\r");
    }
    px_srcline(389);
    if (px_is_truthy(px_eq(_v163, px_str("\\")))) {
        px_srcline(390);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(391);
        return px_str("\\");
    }
    px_srcline(392);
    if (px_is_truthy(px_eq(_v163, px_str("\"")))) {
        px_srcline(393);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(394);
        return px_str("\"");
    }
    px_srcline(395);
    if (px_is_truthy(px_eq(_v163, px_str("'")))) {
        px_srcline(396);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(397);
        return px_str("'");
    }
    px_srcline(398);
    if (px_is_truthy(px_eq(_v163, px_str("0")))) {
        px_srcline(399);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(400);
        return px_str("");
    }
    px_srcline(401);
    if (px_is_truthy(px_eq(_v163, px_str("$")))) {
        px_srcline(402);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(403);
        return px_str("$");
    }
    px_srcline(404);
    if (px_is_truthy(px_eq(_v163, px_str("u")))) {
        px_srcline(405);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(406);
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("{")))) {
            px_srcline(407);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义须为 \\u{XXXX} 形式")}, 2));
        }
        px_srcline(408);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(409);
        _v164 = px_str("");
        px_srcline(410);
        while (px_is_truthy(({ LXValue _t168 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t168) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")) : _t168; }))) {
            px_srcline(411);
            _v165 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            px_srcline(412);
            if (px_is_truthy(px_call(px_get_global("is_hex_digit"), (LXValue[]){_v165}, 1))) {
                px_srcline(413);
                 _v164 = px_add(_v164, _v165);
                px_srcline(414);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            else {
                px_srcline(416);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义含非法字符")}, 2));
            }
        }
        px_srcline(417);
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")))) {
            px_srcline(418);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义缺少 }")}, 2));
        }
        px_srcline(419);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(420);
        if (px_is_truthy(px_eq(_v164, px_str("")))) {
            px_srcline(421);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        px_srcline(422);
        _v166 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v164}, 1);
        px_srcline(423);
        if (px_is_truthy(({ LXValue _t169 = px_eq(_v166, px_null()); px_is_truthy(_t169) ? _t169 : px_gt(_v166, px_int(4294967295LL)); }))) {
            px_srcline(424);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        px_srcline(425);
        if (px_is_truthy(({ LXValue _t171 = px_gt(_v166, px_int(1114111LL)); px_is_truthy(_t171) ? _t171 : ({ LXValue _t170 = px_ge(_v166, px_int(55296LL)); px_is_truthy(_t170) ? px_le(_v166, px_int(57343LL)) : _t170; }); }))) {
            px_srcline(426);
            return px_call(px_get_global("hex_to_char"), (LXValue[]){px_str("FFFD")}, 1);
        }
        px_srcline(427);
        return px_call(px_get_global("hex_to_char"), (LXValue[]){_v164}, 1);
    }
    px_srcline(428);
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_add(px_str("非法转义序列 \\"), _v163)}, 2));
    px_srcline(429);
    return px_str("");
px_err_167:
    if (px_err_167_proped) return px_err_167_val;
    return px_null();
}

static LXValue fn_int_to_hex_nopad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("int_to_hex_nopad");
    LXValue _v172 = (nargs > 0) ? args[0] : px_null();
    LXValue _v173 = px_null();
    LXValue _v174 = px_null();
    LXValue px_err_175_val = px_null();
    int px_err_175_proped = 0;
    px_srcline(432);
    _v173 = px_str("0123456789abcdef");
    px_srcline(433);
    _v174 = px_str("");
    px_srcline(434);
    while (px_is_truthy(px_gt(_v172, px_int(0LL)))) {
        px_srcline(435);
         _v174 = px_add(px_index(_v173, px_mod(_v172, px_int(16LL))), _v174);
        px_srcline(436);
         _v172 = px_idiv(_v172, px_int(16LL));
    }
    px_srcline(437);
    if (px_is_truthy(px_eq(_v174, px_str("")))) {
        px_srcline(438);
        return px_str("0");
    }
    px_srcline(439);
    return _v174;
px_err_175:
    if (px_err_175_proped) return px_err_175_val;
    return px_null();
}

static LXValue fn_char_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("char_debug");
    LXValue _v176 = (nargs > 0) ? args[0] : px_null();
    LXValue _v177 = px_null();
    LXValue px_err_178_val = px_null();
    int px_err_178_proped = 0;
    px_srcline(441);
    if (px_is_truthy(px_eq(_v176, px_str("'")))) {
        px_srcline(442);
        return px_str("'\\''");
    }
    px_srcline(443);
    if (px_is_truthy(px_eq(_v176, px_str("\\")))) {
        px_srcline(444);
        return px_str("'\\\\'");
    }
    px_srcline(445);
    if (px_is_truthy(px_eq(_v176, px_str("\n")))) {
        px_srcline(446);
        return px_str("'\\n'");
    }
    px_srcline(447);
    if (px_is_truthy(px_eq(_v176, px_str("\r")))) {
        px_srcline(448);
        return px_str("'\\r'");
    }
    px_srcline(449);
    if (px_is_truthy(px_eq(_v176, px_str("\t")))) {
        px_srcline(450);
        return px_str("'\\t'");
    }
    px_srcline(451);
    if (px_is_truthy(px_eq(_v176, px_str("")))) {
        px_srcline(452);
        return px_str("'\\0'");
    }
    px_srcline(453);
    _v177 = px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v176}, 1);
    px_srcline(454);
    if (px_is_truthy(px_ge(_v177, px_int(0LL)))) {
        px_srcline(455);
        return px_add(px_add(px_str("'\\u{"), px_call(px_get_global("int_to_hex_nopad"), (LXValue[]){_v177}, 1)), px_str("}'"));
    }
    px_srcline(456);
    return px_add(px_add(px_str("'"), _v176), px_str("'"));
px_err_178:
    if (px_err_178_proped) return px_err_178_val;
    return px_null();
}

static LXValue fn_rust_str_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_str_debug");
    LXValue _v179 = (nargs > 0) ? args[0] : px_null();
    LXValue _v180 = px_null();
    LXValue _v181 = px_null();
    LXValue _v182 = px_null();
    LXValue px_err_183_val = px_null();
    int px_err_183_proped = 0;
    px_srcline(459);
    _v180 = px_str("\"");
    px_srcline(460);
    _v181 = px_int(0LL);
    px_srcline(461);
    while (px_is_truthy(px_lt(_v181, px_call(px_get_global("len"), (LXValue[]){_v179}, 1)))) {
        px_srcline(462);
        _v182 = px_index(_v179, _v181);
        px_srcline(463);
        if (px_is_truthy(px_eq(_v182, px_str("\n")))) {
            px_srcline(464);
             _v180 = px_add(_v180, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\t")))) {
            px_srcline(466);
             _v180 = px_add(_v180, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\r")))) {
            px_srcline(468);
             _v180 = px_add(_v180, px_str("\\r"));
        }
        else if (px_is_truthy(px_lt(_v182, px_str("")))) {
            px_srcline(470);
             _v180 = px_add(_v180, px_str("\\0"));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\"")))) {
            px_srcline(472);
             _v180 = px_add(_v180, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v182, px_str("\\")))) {
            px_srcline(474);
             _v180 = px_add(_v180, px_str("\\\\"));
        }
        else if (px_is_truthy(({ LXValue _t184 = px_ge(_v182, px_str(" ")); px_is_truthy(_t184) ? px_le(_v182, px_str("~")) : _t184; }))) {
            px_srcline(476);
             _v180 = px_add(_v180, _v182);
        }
        else if (px_is_truthy(px_eq(_v182, px_str(" ")))) {
            px_srcline(478);
             _v180 = px_add(_v180, px_str("\\u{a0}"));
        }
        else if (px_is_truthy(px_gt(_v182, px_str(" ")))) {
            px_srcline(480);
             _v180 = px_add(_v180, _v182);
        }
        else {
            px_srcline(482);
             _v180 = px_add(_v180, px_add(px_add(px_str("\\u{"), px_call(px_get_global("ctrl_hex"), (LXValue[]){_v182}, 1)), px_str("}")));
        }
        px_srcline(483);
         _v181 = px_add(_v181, px_int(1LL));
    }
    px_srcline(484);
     _v180 = px_add(_v180, px_str("\""));
    px_srcline(485);
    return _v180;
px_err_183:
    if (px_err_183_proped) return px_err_183_val;
    return px_null();
}

static LXValue fn_ctrl_codepoint(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ctrl_codepoint");
    LXValue _v185 = (nargs > 0) ? args[0] : px_null();
    LXValue _v186 = px_null();
    LXValue px_err_187_val = px_null();
    int px_err_187_proped = 0;
    px_srcline(487);
    _v186 = px_int(0LL);
    px_srcline(488);
    while (px_is_truthy(px_lt(_v186, px_call(px_get_global("len"), (LXValue[]){px_get_global("CTRL_ALL")}, 1)))) {
        px_srcline(489);
        if (px_is_truthy(px_eq(px_index(px_get_global("CTRL_ALL"), _v186), _v185))) {
            px_srcline(490);
            if (px_is_truthy(px_lt(_v186, px_int(28LL)))) {
                px_srcline(491);
                if (px_is_truthy(px_lt(_v186, px_int(8LL)))) {
                    px_srcline(492);
                    return px_add(_v186, px_int(1LL));
                }
                px_srcline(493);
                if (px_is_truthy(px_lt(_v186, px_int(10LL)))) {
                    px_srcline(494);
                    return px_add(_v186, px_int(3LL));
                }
                px_srcline(495);
                return px_add(_v186, px_int(4LL));
            }
            px_srcline(496);
            return px_add(_v186, px_int(99LL));
        }
        px_srcline(497);
         _v186 = px_add(_v186, px_int(1LL));
    }
    px_srcline(498);
    return px_neg(px_int(1LL));
px_err_187:
    if (px_err_187_proped) return px_err_187_val;
    return px_null();
}

static LXValue fn_ctrl_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ctrl_hex");
    LXValue _v188 = (nargs > 0) ? args[0] : px_null();
    LXValue _v189 = px_null();
    LXValue _v190 = px_null();
    LXValue px_err_191_val = px_null();
    int px_err_191_proped = 0;
    px_srcline(500);
    _v189 = px_call(px_get_global("int_to_hex"), (LXValue[]){px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v188}, 1), px_int(16LL)}, 2);
    px_srcline(501);
    _v190 = px_int(0LL);
    px_srcline(502);
    while (px_is_truthy(({ LXValue _t192 = px_lt(_v190, px_call(px_get_global("len"), (LXValue[]){_v189}, 1)); px_is_truthy(_t192) ? px_eq(px_index(_v189, _v190), px_str("0")) : _t192; }))) {
        px_srcline(503);
         _v190 = px_add(_v190, px_int(1LL));
    }
    px_srcline(504);
    if (px_is_truthy(px_eq(_v190, px_call(px_get_global("len"), (LXValue[]){_v189}, 1)))) {
        px_srcline(505);
        return px_str("0");
    }
    px_srcline(506);
    return px_slice(_v189, _v190, px_call(px_get_global("len"), (LXValue[]){_v189}, 1), px_null());
px_err_191:
    if (px_err_191_proped) return px_err_191_val;
    return px_null();
}

static LXValue fn_scan_operator_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_operator_token");
    LXValue _v193 = px_null();
    LXValue _v194 = px_null();
    LXValue _v195 = px_null();
    LXValue px_err_196_val = px_null();
    int px_err_196_proped = 0;
    px_srcline(509);
    _v193 = px_get_global("g_line");
    px_srcline(510);
    _v194 = px_get_global("g_col");
    px_srcline(511);
    _v195 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    px_srcline(512);
    if (px_is_truthy(px_eq(_v195, px_str("(")))) {
        px_srcline(513);
        return px_list_n((LXValue[]){px_str("("), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(514);
    if (px_is_truthy(px_eq(_v195, px_str(")")))) {
        px_srcline(515);
        return px_list_n((LXValue[]){px_str(")"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(516);
    if (px_is_truthy(px_eq(_v195, px_str("[")))) {
        px_srcline(517);
        return px_list_n((LXValue[]){px_str("["), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(518);
    if (px_is_truthy(px_eq(_v195, px_str("]")))) {
        px_srcline(519);
        return px_list_n((LXValue[]){px_str("]"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(520);
    if (px_is_truthy(px_eq(_v195, px_str("{")))) {
        px_srcline(521);
        return px_list_n((LXValue[]){px_str("{"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(522);
    if (px_is_truthy(px_eq(_v195, px_str("}")))) {
        px_srcline(523);
        return px_list_n((LXValue[]){px_str("}"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(524);
    if (px_is_truthy(px_eq(_v195, px_str(",")))) {
        px_srcline(525);
        return px_list_n((LXValue[]){px_str(","), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(526);
    if (px_is_truthy(px_eq(_v195, px_str(":")))) {
        px_srcline(527);
        return px_list_n((LXValue[]){px_str(":"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(528);
    if (px_is_truthy(px_eq(_v195, px_str(".")))) {
        px_srcline(529);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            px_srcline(530);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_str("运算符 '..' 未定义（range 语法尚未支持）")}, 2));
        }
        px_srcline(531);
        return px_list_n((LXValue[]){px_str("."), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(532);
    if (px_is_truthy(px_eq(_v195, px_str("+")))) {
        px_srcline(533);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(534);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(535);
            return px_list_n((LXValue[]){px_str("+="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(536);
        return px_list_n((LXValue[]){px_str("+"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(537);
    if (px_is_truthy(px_eq(_v195, px_str("-")))) {
        px_srcline(538);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(539);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(540);
            return px_list_n((LXValue[]){px_str("->"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(541);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(542);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(543);
            return px_list_n((LXValue[]){px_str("-="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(544);
        return px_list_n((LXValue[]){px_str("-"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(545);
    if (px_is_truthy(px_eq(_v195, px_str("*")))) {
        px_srcline(546);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("*")))) {
            px_srcline(547);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(548);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(549);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(550);
                return px_list_n((LXValue[]){px_str("**="), px_str(""), _v193, _v194}, 4);
            }
            px_srcline(551);
            return px_list_n((LXValue[]){px_str("**"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(552);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(553);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(554);
            return px_list_n((LXValue[]){px_str("*="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(555);
        return px_list_n((LXValue[]){px_str("*"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(556);
    if (px_is_truthy(px_eq(_v195, px_str("/")))) {
        px_srcline(557);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("/")))) {
            px_srcline(558);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(559);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(560);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(561);
                return px_list_n((LXValue[]){px_str("//="), px_str(""), _v193, _v194}, 4);
            }
            px_srcline(562);
            return px_list_n((LXValue[]){px_str("//"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(563);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(564);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(565);
            return px_list_n((LXValue[]){px_str("/="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(566);
        return px_list_n((LXValue[]){px_str("/"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(567);
    if (px_is_truthy(px_eq(_v195, px_str("%")))) {
        px_srcline(568);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(569);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(570);
            return px_list_n((LXValue[]){px_str("%="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(571);
        return px_list_n((LXValue[]){px_str("%"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(572);
    if (px_is_truthy(px_eq(_v195, px_str("^")))) {
        px_srcline(573);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(574);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(575);
            return px_list_n((LXValue[]){px_str("^="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(576);
        return px_list_n((LXValue[]){px_str("^"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(577);
    if (px_is_truthy(px_eq(_v195, px_str("~")))) {
        px_srcline(578);
        return px_list_n((LXValue[]){px_str("~"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(579);
    if (px_is_truthy(px_eq(_v195, px_str("&")))) {
        px_srcline(580);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(581);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(582);
            return px_list_n((LXValue[]){px_str("&="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(583);
        return px_list_n((LXValue[]){px_str("&"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(584);
    if (px_is_truthy(px_eq(_v195, px_str("|")))) {
        px_srcline(585);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(586);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(587);
            return px_list_n((LXValue[]){px_str("|>"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(588);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(589);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(590);
            return px_list_n((LXValue[]){px_str("|="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(591);
        return px_list_n((LXValue[]){px_str("|"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(592);
    if (px_is_truthy(px_eq(_v195, px_str("=")))) {
        px_srcline(593);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(594);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(595);
            return px_list_n((LXValue[]){px_str("=="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(596);
        return px_list_n((LXValue[]){px_str("="), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(597);
    if (px_is_truthy(px_eq(_v195, px_str("!")))) {
        px_srcline(598);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(599);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(600);
            return px_list_n((LXValue[]){px_str("!="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(601);
        return px_list_n((LXValue[]){px_str("!"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(602);
    if (px_is_truthy(px_eq(_v195, px_str("<")))) {
        px_srcline(603);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")))) {
            px_srcline(606);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(607);
            return px_list_n((LXValue[]){px_str("<-"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(608);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("<")))) {
            px_srcline(609);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(610);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(611);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(612);
                return px_list_n((LXValue[]){px_str("<<="), px_str(""), _v193, _v194}, 4);
            }
            px_srcline(613);
            return px_list_n((LXValue[]){px_str("<<"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(614);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(615);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(616);
            return px_list_n((LXValue[]){px_str("<="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(617);
        return px_list_n((LXValue[]){px_str("<"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(618);
    if (px_is_truthy(px_eq(_v195, px_str(">")))) {
        px_srcline(619);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(620);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(621);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
                px_srcline(622);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(623);
                if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                    px_srcline(624);
                    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                    px_srcline(625);
                    return px_list_n((LXValue[]){px_str(">>>="), px_str(""), _v193, _v194}, 4);
                }
                px_srcline(626);
                return px_list_n((LXValue[]){px_str(">>>"), px_str(""), _v193, _v194}, 4);
            }
            px_srcline(627);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(628);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(629);
                return px_list_n((LXValue[]){px_str(">>="), px_str(""), _v193, _v194}, 4);
            }
            px_srcline(630);
            return px_list_n((LXValue[]){px_str(">>"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(631);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(632);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(633);
            return px_list_n((LXValue[]){px_str(">="), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(634);
        return px_list_n((LXValue[]){px_str(">"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(635);
    if (px_is_truthy(px_eq(_v195, px_str("?")))) {
        px_srcline(636);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            px_srcline(637);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(638);
            return px_list_n((LXValue[]){px_str("?."), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(639);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("?")))) {
            px_srcline(640);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(641);
            return px_list_n((LXValue[]){px_str("??"), px_str(""), _v193, _v194}, 4);
        }
        px_srcline(642);
        return px_list_n((LXValue[]){px_str("?"), px_str(""), _v193, _v194}, 4);
    }
    px_srcline(643);
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_add(px_str("非法字符: "), px_call(px_get_global("char_debug"), (LXValue[]){_v195}, 1))}, 2));
    px_srcline(644);
    return px_list_n((LXValue[]){px_str(""), px_str(""), _v193, _v194}, 4);
px_err_196:
    if (px_err_196_proped) return px_err_196_val;
    return px_null();
}

static LXValue fn_next_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("next_token");
    LXValue _v197 = px_null();
    LXValue _v198 = px_null();
    LXValue _v199 = px_null();
    LXValue _v200 = px_null();
    LXValue px_err_201_val = px_null();
    int px_err_201_proped = 0;
    px_srcline(647);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_int(0LL)))) {
        px_srcline(648);
        _v197 = px_index(px_get_global("g_pending"), px_int(0LL));
        px_srcline(649);
        px_set_global("g_pending", px_slice(px_get_global("g_pending"), px_int(1LL), px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_null()));
        px_srcline(650);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){_v197}, 1));
        px_srcline(651);
        return px_bool(true);
    }
    px_srcline(652);
    if (px_is_truthy(px_get_global("g_at_line_start"))) {
        px_srcline(653);
        (void)(px_call(px_get_global("handle_line_start"), (LXValue[]){}, 0));
    }
    px_srcline(654);
    _v198 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    px_srcline(655);
    if (px_is_truthy(px_eq(_v198, px_str("")))) {
        px_srcline(656);
        while (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)))) {
            px_srcline(657);
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            px_srcline(658);
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        px_srcline(659);
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("EOF"), px_str("")}, 2));
        px_srcline(660);
        return px_bool(false);
    }
    px_srcline(661);
    if (px_is_truthy(px_eq(_v198, px_str("\n")))) {
        px_srcline(662);
        _v199 = px_get_global("g_line");
        px_srcline(663);
        _v200 = px_get_global("g_col");
        px_srcline(664);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(665);
        px_set_global("g_at_line_start", px_bool(true));
        px_srcline(666);
        (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_str("换行"), px_str(""), _v199, _v200}, 4));
        px_srcline(667);
        return px_bool(true);
    }
    px_srcline(668);
    if (px_is_truthy(({ LXValue _t202 = px_eq(_v198, px_str(" ")); px_is_truthy(_t202) ? _t202 : px_eq(_v198, px_str("\t")); }))) {
        px_srcline(669);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(670);
        return px_bool(true);
    }
    px_srcline(671);
    if (px_is_truthy(px_eq(_v198, px_str("#")))) {
        px_srcline(672);
        (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
        px_srcline(673);
        return px_bool(true);
    }
    px_srcline(674);
    if (px_is_truthy(({ LXValue _t203 = px_eq(_v198, px_str("\"")); px_is_truthy(_t203) ? _t203 : px_eq(_v198, px_str("'")); }))) {
        px_srcline(675);
        (void)(px_call(px_get_global("scan_string"), (LXValue[]){px_bool(true)}, 1));
        px_srcline(676);
        return px_bool(true);
    }
    px_srcline(677);
    if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v198}, 1))) {
        px_srcline(678);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        px_srcline(679);
        return px_bool(true);
    }
    px_srcline(680);
    if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v198}, 1))) {
        px_srcline(681);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        px_srcline(682);
        return px_bool(true);
    }
    px_srcline(683);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
    px_srcline(684);
    return px_bool(true);
px_err_201:
    if (px_err_201_proped) return px_err_201_val;
    return px_null();
}

static LXValue fn_check_edition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("check_edition");
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
    px_srcline(689);
    _v205 = px_int(0LL);
    px_srcline(690);
    _v206 = px_call(px_get_global("len"), (LXValue[]){_v204}, 1);
    px_srcline(691);
    _v207 = px_str("");
    px_srcline(692);
    while (px_is_truthy(({ LXValue _t213 = px_lt(_v205, _v206); px_is_truthy(_t213) ? px_ne(px_index(_v204, _v205), px_str("\n")) : _t213; }))) {
        px_srcline(693);
         _v207 = px_add(_v207, px_index(_v204, _v205));
        px_srcline(694);
         _v205 = px_add(_v205, px_int(1LL));
    }
    px_srcline(695);
    _v208 = px_call(px_get_global("trim"), (LXValue[]){_v207}, 1);
    px_srcline(696);
    _v209 = px_method(_v208, "split", (LXValue[]){px_str(" ")}, 1);
    px_srcline(697);
    if (px_is_truthy(({ LXValue _t215 = ({ LXValue _t214 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v209}, 1), px_int(3LL)); px_is_truthy(_t214) ? px_eq(px_index(_v209, px_int(0LL)), px_str("#")) : _t214; }); px_is_truthy(_t215) ? px_eq(px_index(_v209, px_int(1LL)), px_str("px")) : _t215; }))) {
        px_srcline(698);
        _v210 = px_index(_v209, px_int(2LL));
        px_srcline(699);
        if (px_is_truthy(({ LXValue _t219 = ({ LXValue _t218 = ({ LXValue _t217 = ({ LXValue _t216 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v210}, 1), px_int(4LL)); px_is_truthy(_t216) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(0LL))}, 1) : _t216; }); px_is_truthy(_t217) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(1LL))}, 1) : _t217; }); px_is_truthy(_t218) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(2LL))}, 1) : _t218; }); px_is_truthy(_t219) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v210, px_int(3LL))}, 1) : _t219; }))) {
            px_srcline(700);
            _v211 = px_call(px_get_global("int"), (LXValue[]){_v210}, 1);
            px_srcline(701);
            if (px_is_truthy(px_gt(_v211, px_int(2026LL)))) {
                px_srcline(702);
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
    px_srcfunc("lex_tokens");
    LXValue _v220 = (nargs > 0) ? args[0] : px_null();
    LXValue _v221 = px_null();
    LXValue px_err_222_val = px_null();
    int px_err_222_proped = 0;
    px_srcline(705);
    (void)(px_call(px_get_global("check_edition"), (LXValue[]){_v220}, 1));
    px_srcline(706);
    px_set_global("g_src", _v220);
    px_srcline(707);
    px_set_global("g_len", px_call(px_get_global("len"), (LXValue[]){_v220}, 1));
    px_srcline(708);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(709);
    px_set_global("g_line", px_int(1LL));
    px_srcline(710);
    px_set_global("g_col", px_int(1LL));
    px_srcline(711);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(712);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(713);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(714);
    px_set_global("g_count", px_int(0LL));
    px_srcline(715);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(716);
    _v221 = px_bool(true);
    px_srcline(717);
    while (px_is_truthy(_v221)) {
        px_srcline(718);
         _v221 = px_call(px_get_global("next_token"), (LXValue[]){}, 0);
    }
    px_srcline(719);
    return px_get_global("g_toks");
px_err_222:
    if (px_err_222_proped) return px_err_222_val;
    return px_null();
}

static LXValue fn_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pad");
    LXValue _v223 = (nargs > 0) ? args[0] : px_null();
    LXValue _v224 = px_null();
    LXValue _v225 = px_null();
    LXValue px_err_226_val = px_null();
    int px_err_226_proped = 0;
    px_srcline(20);
    _v224 = px_str("");
    px_srcline(21);
    _v225 = px_int(0LL);
    px_srcline(22);
    while (px_is_truthy(px_lt(_v225, _v223))) {
        px_srcline(23);
         _v224 = px_add(_v224, px_str(" "));
        px_srcline(24);
         _v225 = px_add(_v225, px_int(1LL));
    }
    px_srcline(25);
    return _v224;
px_err_226:
    if (px_err_226_proped) return px_err_226_val;
    return px_null();
}

static LXValue fn_dump_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_node");
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
    px_srcline(27);
    _v229 = px_index(_v227, px_int(0LL));
    px_srcline(28);
    _v230 = px_index(px_get_global("LAYOUT"), _v229);
    px_srcline(29);
    _v231 = px_index(_v230, px_int(0LL));
    px_srcline(30);
    _v232 = px_index(_v230, px_int(1LL));
    px_srcline(31);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v232}, 1), px_int(0LL)))) {
        px_srcline(32);
        return _v231;
    }
    px_srcline(33);
    _v233 = px_eq(px_index(px_index(_v232, px_int(0LL)), px_int(0LL)), px_null());
    px_srcline(34);
    _v234 = px_list_n((LXValue[]){}, 0);
    px_srcline(35);
    if (px_is_truthy(_v233)) {
        px_srcline(36);
        (void)(px_method(_v234, "append", (LXValue[]){px_add(_v231, px_str("("))}, 1));
    }
    else {
        px_srcline(38);
        (void)(px_method(_v234, "append", (LXValue[]){px_add(_v231, px_str(" {"))}, 1));
    }
    px_srcline(39);
    _v235 = px_int(0LL);
    px_srcline(40);
    while (px_is_truthy(px_lt(_v235, px_call(px_get_global("len"), (LXValue[]){_v232}, 1)))) {
        px_srcline(41);
        _v236 = px_index(_v232, _v235);
        px_srcline(42);
        _v237 = px_index(_v227, px_add(_v235, px_int(1LL)));
        px_srcline(43);
        _v238 = px_call(px_get_global("dump_field"), (LXValue[]){_v237, px_index(_v236, px_int(1LL)), px_add(_v228, px_int(4LL))}, 3);
        px_srcline(44);
        if (px_is_truthy(_v233)) {
            px_srcline(45);
            (void)(px_method(_v234, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v228, px_int(4LL))}, 1), _v238), px_str(","))}, 1));
        }
        else {
            px_srcline(47);
            (void)(px_method(_v234, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v228, px_int(4LL))}, 1), px_index(_v236, px_int(0LL))), px_str(": ")), _v238), px_str(","))}, 1));
        }
        px_srcline(48);
         _v235 = px_add(_v235, px_int(1LL));
    }
    px_srcline(49);
    if (px_is_truthy(_v233)) {
        px_srcline(50);
        (void)(px_method(_v234, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v228}, 1), px_str(")"))}, 1));
    }
    else {
        px_srcline(52);
        (void)(px_method(_v234, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v228}, 1), px_str("}"))}, 1));
    }
    px_srcline(53);
    return px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v234}, 2);
px_err_239:
    if (px_err_239_proped) return px_err_239_val;
    return px_null();
}

static LXValue fn_dump_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_list");
    LXValue _v240 = (nargs > 0) ? args[0] : px_null();
    LXValue _v241 = (nargs > 1) ? args[1] : px_null();
    LXValue _v242 = px_null();
    LXValue _v243 = px_null();
    LXValue px_err_244_val = px_null();
    int px_err_244_proped = 0;
    px_srcline(55);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v240}, 1), px_int(0LL)))) {
        px_srcline(56);
        return px_str("[]");
    }
    px_srcline(57);
    _v242 = px_list_n((LXValue[]){}, 0);
    px_srcline(58);
    _v243 = px_int(0LL);
    px_srcline(59);
    while (px_is_truthy(px_lt(_v243, px_call(px_get_global("len"), (LXValue[]){_v240}, 1)))) {
        px_srcline(60);
        (void)(px_method(_v242, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v241, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v240, _v243), px_add(_v241, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(61);
         _v243 = px_add(_v243, px_int(1LL));
    }
    px_srcline(62);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v242}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v241}, 1)), px_str("]"));
px_err_244:
    if (px_err_244_proped) return px_err_244_val;
    return px_null();
}

static LXValue fn_dump_str_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_str_list");
    LXValue _v245 = (nargs > 0) ? args[0] : px_null();
    LXValue _v246 = (nargs > 1) ? args[1] : px_null();
    LXValue _v247 = px_null();
    LXValue _v248 = px_null();
    LXValue px_err_249_val = px_null();
    int px_err_249_proped = 0;
    px_srcline(64);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v245}, 1), px_int(0LL)))) {
        px_srcline(65);
        return px_str("[]");
    }
    px_srcline(66);
    _v247 = px_list_n((LXValue[]){}, 0);
    px_srcline(67);
    _v248 = px_int(0LL);
    px_srcline(68);
    while (px_is_truthy(px_lt(_v248, px_call(px_get_global("len"), (LXValue[]){_v245}, 1)))) {
        px_srcline(69);
        (void)(px_method(_v247, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v246, px_int(4LL))}, 1), px_index(_v245, _v248)), px_str(","))}, 1));
        px_srcline(70);
         _v248 = px_add(_v248, px_int(1LL));
    }
    px_srcline(71);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v247}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v246}, 1)), px_str("]"));
px_err_249:
    if (px_err_249_proped) return px_err_249_val;
    return px_null();
}

static LXValue fn_dump_ty_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_ty_list");
    LXValue _v250 = (nargs > 0) ? args[0] : px_null();
    LXValue _v251 = (nargs > 1) ? args[1] : px_null();
    LXValue _v252 = px_null();
    LXValue _v253 = px_null();
    LXValue px_err_254_val = px_null();
    int px_err_254_proped = 0;
    px_srcline(73);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v250}, 1), px_int(0LL)))) {
        px_srcline(74);
        return px_str("[]");
    }
    px_srcline(75);
    _v252 = px_list_n((LXValue[]){}, 0);
    px_srcline(76);
    _v253 = px_int(0LL);
    px_srcline(77);
    while (px_is_truthy(px_lt(_v253, px_call(px_get_global("len"), (LXValue[]){_v250}, 1)))) {
        px_srcline(78);
        (void)(px_method(_v252, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v251, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v250, _v253), px_add(_v251, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(79);
         _v253 = px_add(_v253, px_int(1LL));
    }
    px_srcline(80);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v252}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v251}, 1)), px_str("]"));
px_err_254:
    if (px_err_254_proped) return px_err_254_val;
    return px_null();
}

static LXValue fn_dump_pat_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_pat_list");
    LXValue _v255 = (nargs > 0) ? args[0] : px_null();
    LXValue _v256 = (nargs > 1) ? args[1] : px_null();
    LXValue _v257 = px_null();
    LXValue _v258 = px_null();
    LXValue px_err_259_val = px_null();
    int px_err_259_proped = 0;
    px_srcline(82);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v255}, 1), px_int(0LL)))) {
        px_srcline(83);
        return px_str("[]");
    }
    px_srcline(84);
    _v257 = px_list_n((LXValue[]){}, 0);
    px_srcline(85);
    _v258 = px_int(0LL);
    px_srcline(86);
    while (px_is_truthy(px_lt(_v258, px_call(px_get_global("len"), (LXValue[]){_v255}, 1)))) {
        px_srcline(87);
        (void)(px_method(_v257, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v256, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v255, _v258), px_add(_v256, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(88);
         _v258 = px_add(_v258, px_int(1LL));
    }
    px_srcline(89);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v257}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v256}, 1)), px_str("]"));
px_err_259:
    if (px_err_259_proped) return px_err_259_val;
    return px_null();
}

static LXValue fn_dump_opt_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_node");
    LXValue _v260 = (nargs > 0) ? args[0] : px_null();
    LXValue _v261 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_262_val = px_null();
    int px_err_262_proped = 0;
    px_srcline(91);
    if (px_is_truthy(px_eq(_v260, px_null()))) {
        px_srcline(92);
        return px_str("None");
    }
    px_srcline(93);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v261, px_int(4LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){_v260, px_add(_v261, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v261}, 1)), px_str(")"));
px_err_262:
    if (px_err_262_proped) return px_err_262_val;
    return px_null();
}

static LXValue fn_dump_opt_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_str");
    LXValue _v263 = (nargs > 0) ? args[0] : px_null();
    LXValue _v264 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_265_val = px_null();
    int px_err_265_proped = 0;
    px_srcline(95);
    if (px_is_truthy(px_eq(_v263, px_null()))) {
        px_srcline(96);
        return px_str("None");
    }
    px_srcline(97);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v264, px_int(4LL))}, 1)), _v263), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v264}, 1)), px_str(")"));
px_err_265:
    if (px_err_265_proped) return px_err_265_val;
    return px_null();
}

static LXValue fn_dump_opt_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_list");
    LXValue _v266 = (nargs > 0) ? args[0] : px_null();
    LXValue _v267 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_268_val = px_null();
    int px_err_268_proped = 0;
    px_srcline(99);
    if (px_is_truthy(px_eq(_v266, px_null()))) {
        px_srcline(100);
        return px_str("None");
    }
    px_srcline(101);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v267, px_int(4LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){_v266, px_add(_v267, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v267}, 1)), px_str(")"));
px_err_268:
    if (px_err_268_proped) return px_err_268_val;
    return px_null();
}

static LXValue fn_dump_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_pos");
    LXValue _v269 = (nargs > 0) ? args[0] : px_null();
    LXValue _v270 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_271_val = px_null();
    int px_err_271_proped = 0;
    px_srcline(103);
    return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v270, px_int(4LL))}, 1)), px_str("line: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v269, px_int(0LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v270, px_int(4LL))}, 1)), px_str("col: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v269, px_int(1LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v270}, 1)), px_str("}"));
px_err_271:
    if (px_err_271_proped) return px_err_271_val;
    return px_null();
}

static LXValue fn_dump_t2_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t2_list");
    LXValue _v272 = (nargs > 0) ? args[0] : px_null();
    LXValue _v273 = (nargs > 1) ? args[1] : px_null();
    LXValue _v274 = px_null();
    LXValue _v275 = px_null();
    LXValue _v276 = px_null();
    LXValue px_err_277_val = px_null();
    int px_err_277_proped = 0;
    px_srcline(106);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v272}, 1), px_int(0LL)))) {
        px_srcline(107);
        return px_str("[]");
    }
    px_srcline(108);
    _v274 = px_list_n((LXValue[]){}, 0);
    px_srcline(109);
    _v275 = px_int(0LL);
    px_srcline(110);
    while (px_is_truthy(px_lt(_v275, px_call(px_get_global("len"), (LXValue[]){_v272}, 1)))) {
        px_srcline(111);
        _v276 = px_index(_v272, _v275);
        px_srcline(112);
        (void)(px_method(_v274, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v276, px_int(0LL)), px_add(_v273, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v276, px_int(1LL)), px_add(_v273, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v273, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(113);
         _v275 = px_add(_v275, px_int(1LL));
    }
    px_srcline(114);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v274}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v273}, 1)), px_str("]"));
px_err_277:
    if (px_err_277_proped) return px_err_277_val;
    return px_null();
}

static LXValue fn_dump_t2b_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t2b_list");
    LXValue _v278 = (nargs > 0) ? args[0] : px_null();
    LXValue _v279 = (nargs > 1) ? args[1] : px_null();
    LXValue _v280 = px_null();
    LXValue _v281 = px_null();
    LXValue _v282 = px_null();
    LXValue px_err_283_val = px_null();
    int px_err_283_proped = 0;
    px_srcline(117);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v278}, 1), px_int(0LL)))) {
        px_srcline(118);
        return px_str("[]");
    }
    px_srcline(119);
    _v280 = px_list_n((LXValue[]){}, 0);
    px_srcline(120);
    _v281 = px_int(0LL);
    px_srcline(121);
    while (px_is_truthy(px_lt(_v281, px_call(px_get_global("len"), (LXValue[]){_v278}, 1)))) {
        px_srcline(122);
        _v282 = px_index(_v278, _v281);
        px_srcline(123);
        (void)(px_method(_v280, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v282, px_int(0LL)), px_add(_v279, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v282, px_int(1LL)), px_add(_v279, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v279, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(124);
         _v281 = px_add(_v281, px_int(1LL));
    }
    px_srcline(125);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v280}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v279}, 1)), px_str("]"));
px_err_283:
    if (px_err_283_proped) return px_err_283_val;
    return px_null();
}

static LXValue fn_dump_t3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t3_list");
    LXValue _v284 = (nargs > 0) ? args[0] : px_null();
    LXValue _v285 = (nargs > 1) ? args[1] : px_null();
    LXValue _v286 = px_null();
    LXValue _v287 = px_null();
    LXValue _v288 = px_null();
    LXValue _v289 = px_null();
    LXValue _v290 = px_null();
    LXValue px_err_291_val = px_null();
    int px_err_291_proped = 0;
    px_srcline(128);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v284}, 1), px_int(0LL)))) {
        px_srcline(129);
        return px_str("[]");
    }
    px_srcline(130);
    _v286 = px_list_n((LXValue[]){}, 0);
    px_srcline(131);
    _v287 = px_int(0LL);
    px_srcline(132);
    while (px_is_truthy(px_lt(_v287, px_call(px_get_global("len"), (LXValue[]){_v284}, 1)))) {
        px_srcline(133);
        _v288 = px_index(_v284, _v287);
        px_srcline(134);
        _v289 = px_index(_v288, px_int(0LL));
        px_srcline(135);
        _v290 = px_str("None");
        px_srcline(136);
        if (px_is_truthy(px_ne(_v289, px_null()))) {
            px_srcline(137);
             _v290 = px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(12LL))}, 1)), _v289), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), px_str(")"));
        }
        px_srcline(138);
        (void)(px_method(_v286, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), _v290), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v288, px_int(1LL)), px_add(_v285, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v288, px_int(2LL)), px_add(_v285, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(139);
         _v287 = px_add(_v287, px_int(1LL));
    }
    px_srcline(140);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v286}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v285}, 1)), px_str("]"));
px_err_291:
    if (px_err_291_proped) return px_err_291_val;
    return px_null();
}

static LXValue fn_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fmt_float");
    LXValue _v292 = (nargs > 0) ? args[0] : px_null();
    LXValue _v293 = px_null();
    LXValue px_err_294_val = px_null();
    int px_err_294_proped = 0;
    px_srcline(142);
    _v293 = px_call(px_get_global("str"), (LXValue[]){_v292}, 1);
    px_srcline(143);
    if (px_is_truthy(({ LXValue _t295 = px_eq(_v293, px_str("inf")); px_is_truthy(_t295) ? _t295 : px_eq(_v293, px_str("-inf")); }))) {
        px_srcline(144);
        return _v293;
    }
    px_srcline(145);
    if (px_is_truthy(({ LXValue _t297 = ({ LXValue _t296 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v293, px_str(".")}, 2)); px_is_truthy(_t296) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v293, px_str("e")}, 2)) : _t296; }); px_is_truthy(_t297) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v293, px_str("E")}, 2)) : _t297; }))) {
        px_srcline(146);
        return px_add(_v293, px_str(".0"));
    }
    px_srcline(147);
    return _v293;
px_err_294:
    if (px_err_294_proped) return px_err_294_val;
    return px_null();
}

static LXValue fn_dump_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_field");
    LXValue _v298 = (nargs > 0) ? args[0] : px_null();
    LXValue _v299 = (nargs > 1) ? args[1] : px_null();
    LXValue _v300 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_301_val = px_null();
    int px_err_301_proped = 0;
    px_srcline(149);
    if (px_is_truthy(px_eq(_v299, px_str("s")))) {
        px_srcline(150);
        return _v298;
    }
    px_srcline(151);
    if (px_is_truthy(px_eq(_v299, px_str("r")))) {
        px_srcline(152);
        return px_call(px_get_global("str"), (LXValue[]){_v298}, 1);
    }
    px_srcline(153);
    if (px_is_truthy(px_eq(_v299, px_str("f")))) {
        px_srcline(154);
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v298}, 1);
    }
    px_srcline(155);
    if (px_is_truthy(px_eq(_v299, px_str("n")))) {
        px_srcline(156);
        return px_call(px_get_global("dump_node"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(157);
    if (px_is_truthy(px_eq(_v299, px_str("o")))) {
        px_srcline(158);
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(159);
    if (px_is_truthy(px_eq(_v299, px_str("os")))) {
        px_srcline(160);
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(161);
    if (px_is_truthy(px_eq(_v299, px_str("ol")))) {
        px_srcline(162);
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(163);
    if (px_is_truthy(px_eq(_v299, px_str("l")))) {
        px_srcline(164);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(165);
    if (px_is_truthy(px_eq(_v299, px_str("ls")))) {
        px_srcline(166);
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v299, px_str("tl")))) {
        px_srcline(168);
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v299, px_str("lpl")))) {
        px_srcline(170);
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v299, px_str("lp")))) {
        px_srcline(172);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(_v299, px_str("lsf")))) {
        px_srcline(174);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(175);
    if (px_is_truthy(px_eq(_v299, px_str("lev")))) {
        px_srcline(176);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(177);
    if (px_is_truthy(px_eq(_v299, px_str("lfd")))) {
        px_srcline(178);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(179);
    if (px_is_truthy(px_eq(_v299, px_str("ltci")))) {
        px_srcline(181);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(182);
    if (px_is_truthy(px_eq(_v299, px_str("lc")))) {
        px_srcline(183);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(184);
    if (px_is_truthy(px_eq(_v299, px_str("lma")))) {
        px_srcline(185);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(186);
    if (px_is_truthy(px_eq(_v299, px_str("lt2")))) {
        px_srcline(187);
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(188);
    if (px_is_truthy(px_eq(_v299, px_str("lt2b")))) {
        px_srcline(189);
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(190);
    if (px_is_truthy(px_eq(_v299, px_str("lt3")))) {
        px_srcline(191);
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(192);
    if (px_is_truthy(px_eq(_v299, px_str("p")))) {
        px_srcline(193);
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v298, _v300}, 2);
    }
    px_srcline(194);
    return px_call(px_get_global("str"), (LXValue[]){_v298}, 1);
px_err_301:
    if (px_err_301_proped) return px_err_301_val;
    return px_null();
}

static LXValue fn_dump_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_program");
    LXValue _v302 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_303_val = px_null();
    int px_err_303_proped = 0;
    px_srcline(197);
    return px_call(px_get_global("dump_node"), (LXValue[]){_v302, px_int(0LL)}, 2);
px_err_303:
    if (px_err_303_proped) return px_err_303_val;
    return px_null();
}

static LXValue fn_pk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pk");
    LXValue px_err_304_val = px_null();
    int px_err_304_proped = 0;
    px_srcline(100);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
px_err_304:
    if (px_err_304_proped) return px_err_304_val;
    return px_null();
}

static LXValue fn_pk_display(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pk_display");
    LXValue _v305 = px_null();
    LXValue _v306 = px_null();
    LXValue px_err_307_val = px_null();
    int px_err_307_proped = 0;
    px_srcline(103);
    _v305 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
    px_srcline(104);
    _v306 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
    px_srcline(105);
    if (px_is_truthy(px_eq(_v305, px_str("整数")))) {
        px_srcline(106);
        return px_add(px_str("整数 "), _v306);
    }
    px_srcline(107);
    if (px_is_truthy(px_eq(_v305, px_str("浮点")))) {
        px_srcline(108);
        return px_add(px_str("浮点 "), _v306);
    }
    px_srcline(109);
    if (px_is_truthy(px_eq(_v305, px_str("字符串")))) {
        px_srcline(110);
        return px_add(px_str("字符串 "), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v306}, 1));
    }
    px_srcline(111);
    if (px_is_truthy(px_eq(_v305, px_str("标识符")))) {
        px_srcline(112);
        return px_add(px_str("标识符 "), _v306);
    }
    px_srcline(113);
    if (px_is_truthy(px_eq(_v305, px_str("注释")))) {
        px_srcline(114);
        return px_add(px_str("注释 "), _v306);
    }
    px_srcline(115);
    return _v305;
px_err_307:
    if (px_err_307_proped) return px_err_307_val;
    return px_null();
}

static LXValue fn_pv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pv");
    LXValue px_err_308_val = px_null();
    int px_err_308_proped = 0;
    px_srcline(117);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
px_err_308:
    if (px_err_308_proped) return px_err_308_val;
    return px_null();
}

static LXValue fn_pline(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pline");
    LXValue px_err_309_val = px_null();
    int px_err_309_proped = 0;
    px_srcline(119);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL));
px_err_309:
    if (px_err_309_proped) return px_err_309_val;
    return px_null();
}

static LXValue fn_pcol(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pcol");
    LXValue px_err_310_val = px_null();
    int px_err_310_proped = 0;
    px_srcline(121);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL));
px_err_310:
    if (px_err_310_proped) return px_err_310_val;
    return px_null();
}

static LXValue fn_ppos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ppos");
    LXValue px_err_311_val = px_null();
    int px_err_311_proped = 0;
    px_srcline(123);
    return px_list_n((LXValue[]){px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL)), px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL))}, 2);
px_err_311:
    if (px_err_311_proped) return px_err_311_val;
    return px_null();
}

static LXValue fn_adv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("adv");
    LXValue _v312 = px_null();
    LXValue px_err_313_val = px_null();
    int px_err_313_proped = 0;
    px_srcline(125);
    _v312 = px_index(px_get_global("p_toks"), px_get_global("p_pos"));
    px_srcline(126);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(127);
        px_set_global("p_pos", px_add(px_get_global("p_pos"), px_int(1LL)));
    }
    px_srcline(128);
    return _v312;
px_err_313:
    if (px_err_313_proped) return px_err_313_val;
    return px_null();
}

static LXValue fn_chk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk");
    LXValue _v314 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_315_val = px_null();
    int px_err_315_proped = 0;
    px_srcline(130);
    return px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), _v314);
px_err_315:
    if (px_err_315_proped) return px_err_315_val;
    return px_null();
}

static LXValue fn_chk2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk2");
    LXValue _v316 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_317_val = px_null();
    int px_err_317_proped = 0;
    px_srcline(132);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(133);
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(1LL))), px_int(0LL)), _v316);
    }
    px_srcline(134);
    return px_bool(false);
px_err_317:
    if (px_err_317_proped) return px_err_317_val;
    return px_null();
}

static LXValue fn_chk3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk3");
    LXValue _v318 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_319_val = px_null();
    int px_err_319_proped = 0;
    px_srcline(137);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(2LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(138);
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(2LL))), px_int(0LL)), _v318);
    }
    px_srcline(139);
    return px_bool(false);
px_err_319:
    if (px_err_319_proped) return px_err_319_val;
    return px_null();
}

static LXValue fn_expect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect");
    LXValue _v320 = (nargs > 0) ? args[0] : px_null();
    LXValue _v321 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_322_val = px_null();
    int px_err_322_proped = 0;
    px_srcline(141);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v320}, 1))) {
        px_srcline(142);
        return px_call(px_get_global("adv"), (LXValue[]){}, 0);
    }
    px_srcline(143);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望 "), _v321), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_322:
    if (px_err_322_proped) return px_err_322_val;
    return px_null();
}

static LXValue fn_expect_ident(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect_ident");
    LXValue _v323 = (nargs > 0) ? args[0] : px_null();
    LXValue _v324 = px_null();
    LXValue px_err_325_val = px_null();
    int px_err_325_proped = 0;
    px_srcline(145);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(146);
        _v324 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(147);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(148);
        return _v324;
    }
    px_srcline(149);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v323), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_325:
    if (px_err_325_proped) return px_err_325_val;
    return px_null();
}

static LXValue fn_is_name_kind(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_name_kind");
    LXValue _v326 = (nargs > 0) ? args[0] : px_null();
    LXValue _v327 = px_null();
    LXValue _v328 = px_null();
    LXValue px_err_329_val = px_null();
    int px_err_329_proped = 0;
    px_srcline(151);
    _v327 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture"), px_str("extern")}, 36);
    px_srcline(152);
    _v328 = px_int(0LL);
    px_srcline(153);
    while (px_is_truthy(px_lt(_v328, px_call(px_get_global("len"), (LXValue[]){_v327}, 1)))) {
        px_srcline(154);
        if (px_is_truthy(px_eq(px_index(_v327, _v328), _v326))) {
            px_srcline(155);
            return px_bool(true);
        }
        px_srcline(156);
         _v328 = px_add(_v328, px_int(1LL));
    }
    px_srcline(157);
    return px_bool(false);
px_err_329:
    if (px_err_329_proped) return px_err_329_val;
    return px_null();
}

static LXValue fn_expect_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect_name");
    LXValue _v330 = (nargs > 0) ? args[0] : px_null();
    LXValue _v331 = px_null();
    LXValue px_err_332_val = px_null();
    int px_err_332_proped = 0;
    px_srcline(159);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(160);
        _v331 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(161);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(162);
        return _v331;
    }
    px_srcline(163);
    if (px_is_truthy(px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1))) {
        px_srcline(164);
        _v331 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(165);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(166);
        return _v331;
    }
    px_srcline(167);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v330), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_332:
    if (px_err_332_proped) return px_err_332_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("perr");
    LXValue _v333 = (nargs > 0) ? args[0] : px_null();
    LXValue _v334 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_335_val = px_null();
    int px_err_335_proped = 0;
    px_srcline(169);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1)), px_str(": 语法错误 ")), _v333), px_str(": ")), _v334)}, 1));
    px_srcline(170);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("parse "), _v333)}, 1));
px_err_335:
    if (px_err_335_proped) return px_err_335_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines");
    LXValue px_err_336_val = px_null();
    int px_err_336_proped = 0;
    px_srcline(172);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(173);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_336:
    if (px_err_336_proped) return px_err_336_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_brace_indents");
    LXValue px_err_337_val = px_null();
    int px_err_337_proped = 0;
    px_srcline(175);
    while (px_is_truthy(({ LXValue _t338 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t338) ? _t338 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(176);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_337:
    if (px_err_337_proped) return px_err_337_val;
    return px_null();
}

static LXValue fn_skip_expr_ws(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_expr_ws");
    LXValue px_err_339_val = px_null();
    int px_err_339_proped = 0;
    px_srcline(178);
    while (px_is_truthy(({ LXValue _t341 = ({ LXValue _t340 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t340) ? _t340 : px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); }); px_is_truthy(_t341) ? _t341 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(179);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_339:
    if (px_err_339_proped) return px_err_339_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines_in_block");
    LXValue px_err_342_val = px_null();
    int px_err_342_proped = 0;
    px_srcline(181);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(182);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_342:
    if (px_err_342_proped) return px_err_342_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("node_pos");
    LXValue _v343 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_344_val = px_null();
    int px_err_344_proped = 0;
    px_srcline(184);
    return px_index(_v343, px_sub(px_call(px_get_global("len"), (LXValue[]){_v343}, 1), px_int(1LL)));
px_err_344:
    if (px_err_344_proped) return px_err_344_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("qstr");
    LXValue _v345 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_346_val = px_null();
    int px_err_346_proped = 0;
    px_srcline(186);
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v345}, 1);
px_err_346:
    if (px_err_346_proped) return px_err_346_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_program");
    LXValue _v347 = px_null();
    LXValue px_err_348_val = px_null();
    int px_err_348_proped = 0;
    px_srcline(189);
    _v347 = px_list_n((LXValue[]){}, 0);
    px_srcline(190);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(191);
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        px_srcline(192);
        (void)(px_method(_v347, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(193);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    px_srcline(194);
    return px_list_n((LXValue[]){px_str("Program"), _v347}, 2);
px_err_348:
    if (px_err_348_proped) return px_err_348_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_stmt");
    LXValue _v349 = px_null();
    LXValue _v350 = px_null();
    LXValue _v351 = px_null();
    LXValue _v352 = px_null();
    LXValue px_err_353_val = px_null();
    int px_err_353_proped = 0;
    px_srcline(197);
    _v349 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(198);
    if (px_is_truthy(px_eq(_v349, px_str("let")))) {
        px_srcline(199);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    px_srcline(200);
    if (px_is_truthy(px_eq(_v349, px_str("var")))) {
        px_srcline(201);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    px_srcline(202);
    if (px_is_truthy(px_eq(_v349, px_str("const")))) {
        px_srcline(203);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    px_srcline(204);
    if (px_is_truthy(px_eq(_v349, px_str("if")))) {
        px_srcline(205);
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    px_srcline(206);
    if (px_is_truthy(px_eq(_v349, px_str("for")))) {
        px_srcline(207);
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    px_srcline(208);
    if (px_is_truthy(px_eq(_v349, px_str("while")))) {
        px_srcline(209);
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    px_srcline(210);
    if (px_is_truthy(px_eq(_v349, px_str("def")))) {
        px_srcline(211);
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    px_srcline(212);
    if (px_is_truthy(px_eq(_v349, px_str("extern")))) {
        px_srcline(213);
        return px_call(px_get_global("parse_extern_def"), (LXValue[]){}, 0);
    }
    px_srcline(214);
    if (px_is_truthy(px_eq(_v349, px_str("struct")))) {
        px_srcline(215);
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    px_srcline(216);
    if (px_is_truthy(px_eq(_v349, px_str("enum")))) {
        px_srcline(217);
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    px_srcline(218);
    if (px_is_truthy(({ LXValue _t354 = px_eq(_v349, px_str("标识符")); px_is_truthy(_t354) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("type")) : _t354; }))) {
        px_srcline(221);
        if (px_is_truthy(({ LXValue _t355 = px_call(px_get_global("chk2"), (LXValue[]){px_str("标识符")}, 1); px_is_truthy(_t355) ? px_call(px_get_global("chk3"), (LXValue[]){px_str("const")}, 1) : _t355; }))) {
            px_srcline(222);
            return px_call(px_get_global("parse_type_const"), (LXValue[]){}, 0);
        }
        px_srcline(223);
        return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
    }
    px_srcline(224);
    if (px_is_truthy(px_eq(_v349, px_str("trait")))) {
        px_srcline(225);
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    px_srcline(226);
    if (px_is_truthy(px_eq(_v349, px_str("impl")))) {
        px_srcline(227);
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    px_srcline(228);
    if (px_is_truthy(px_eq(_v349, px_str("import")))) {
        px_srcline(229);
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    px_srcline(230);
    if (px_is_truthy(px_eq(_v349, px_str("from")))) {
        px_srcline(231);
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    px_srcline(232);
    if (px_is_truthy(px_eq(_v349, px_str("return")))) {
        px_srcline(233);
        _v350 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(234);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(235);
        if (px_is_truthy(({ LXValue _t357 = ({ LXValue _t356 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t356) ? _t356 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t357) ? _t357 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(236);
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v350}, 3);
        }
        px_srcline(237);
        _v351 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(238);
        return px_list_n((LXValue[]){px_str("Return"), _v351, _v350}, 3);
    }
    px_srcline(239);
    if (px_is_truthy(px_eq(_v349, px_str("break")))) {
        px_srcline(240);
        _v350 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(241);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(242);
        return px_list_n((LXValue[]){px_str("Break"), _v350}, 2);
    }
    px_srcline(243);
    if (px_is_truthy(px_eq(_v349, px_str("continue")))) {
        px_srcline(244);
        _v350 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(245);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(246);
        return px_list_n((LXValue[]){px_str("Continue"), _v350}, 2);
    }
    px_srcline(247);
    if (px_is_truthy(px_eq(_v349, px_str("spawn")))) {
        px_srcline(248);
        _v350 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(249);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(250);
        _v352 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(251);
        return px_list_n((LXValue[]){px_str("Spawn"), _v352, _v350}, 3);
    }
    px_srcline(252);
    if (px_is_truthy(px_eq(_v349, px_str("select")))) {
        px_srcline(253);
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    px_srcline(254);
    if (px_is_truthy(px_eq(_v349, px_str("fn")))) {
        px_srcline(255);
        _v352 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(256);
        _v350 = px_call(px_get_global("node_pos"), (LXValue[]){_v352}, 1);
        px_srcline(257);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v352, _v350}, 3);
    }
    px_srcline(258);
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_353:
    if (px_err_353_proped) return px_err_353_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_var_decl");
    LXValue _v358 = (nargs > 0) ? args[0] : px_null();
    LXValue _v359 = px_null();
    LXValue _v360 = px_null();
    LXValue _v361 = px_null();
    LXValue _v362 = px_null();
    LXValue px_err_363_val = px_null();
    int px_err_363_proped = 0;
    px_srcline(260);
    _v359 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(261);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(263);
    if (px_is_truthy(({ LXValue _t365 = ({ LXValue _t364 = px_eq(_v358, px_str("Let")); px_is_truthy(_t364) ? px_call(px_get_global("chk"), (LXValue[]){px_str("标识符")}, 1) : _t364; }); px_is_truthy(_t365) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("mut")) : _t365; }))) {
        px_srcline(264);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(265);
         _v358 = px_str("Mut");
    }
    px_srcline(266);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        px_srcline(267);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    px_srcline(268);
    _v360 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    px_srcline(269);
    _v361 = px_null();
    px_srcline(270);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(271);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(272);
         _v361 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(273);
    _v362 = px_null();
    px_srcline(274);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        px_srcline(275);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(276);
         _v362 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    px_srcline(277);
    return px_list_n((LXValue[]){px_str("VarDecl"), _v358, px_call(px_get_global("qstr"), (LXValue[]){_v360}, 1), _v361, _v362, _v359}, 6);
px_err_363:
    if (px_err_363_proped) return px_err_363_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_assign_or_expr");
    LXValue _v366 = px_null();
    LXValue _v367 = px_null();
    LXValue _v368 = px_null();
    LXValue _v369 = px_null();
    LXValue _v370 = px_null();
    LXValue px_err_371_val = px_null();
    int px_err_371_proped = 0;
    px_srcline(279);
    _v366 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(280);
    _v367 = px_call(px_get_global("node_pos"), (LXValue[]){_v366}, 1);
    px_srcline(281);
    _v368 = px_null();
    px_srcline(282);
    _v369 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(283);
    if (px_is_truthy(px_eq(_v369, px_str("=")))) {
        px_srcline(284);
         _v368 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("<-")))) {
        px_srcline(287);
         _v368 = px_str("Append");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("+=")))) {
        px_srcline(289);
         _v368 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("-=")))) {
        px_srcline(291);
         _v368 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("*=")))) {
        px_srcline(293);
         _v368 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("/=")))) {
        px_srcline(295);
         _v368 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("//=")))) {
        px_srcline(297);
         _v368 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("%=")))) {
        px_srcline(299);
         _v368 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("**=")))) {
        px_srcline(301);
         _v368 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("&=")))) {
        px_srcline(303);
         _v368 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("|=")))) {
        px_srcline(305);
         _v368 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("^=")))) {
        px_srcline(307);
         _v368 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v369, px_str("<<=")))) {
        px_srcline(309);
         _v368 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v369, px_str(">>=")))) {
        px_srcline(311);
         _v368 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v369, px_str(">>>=")))) {
        px_srcline(313);
         _v368 = px_str("ShrU");
    }
    px_srcline(314);
    if (px_is_truthy(px_ne(_v368, px_null()))) {
        px_srcline(315);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(316);
        _v370 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(317);
        return px_list_n((LXValue[]){px_str("Assign"), _v366, _v368, _v370, _v367}, 5);
    }
    px_srcline(318);
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v366, _v367}, 3);
px_err_371:
    if (px_err_371_proped) return px_err_371_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if");
    LXValue _v372 = px_null();
    LXValue _v373 = px_null();
    LXValue _v374 = px_null();
    LXValue _v375 = px_null();
    LXValue _v376 = px_null();
    LXValue _v377 = px_null();
    LXValue _v378 = px_null();
    LXValue px_err_379_val = px_null();
    int px_err_379_proped = 0;
    px_srcline(320);
    _v372 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(321);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(322);
    _v373 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(323);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(324);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(325);
    _v374 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(326);
    _v375 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v373, _v374}, 2)}, 1);
    px_srcline(327);
    _v376 = px_null();
    px_srcline(328);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(329);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            px_srcline(330);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(331);
            _v377 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(332);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(333);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(334);
            _v378 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(335);
            (void)(px_method(_v375, "append", (LXValue[]){px_list_n((LXValue[]){_v377, _v378}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            px_srcline(337);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(338);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(339);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(340);
             _v376 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(341);
            break;
        }
        else {
            px_srcline(343);
            break;
        }
    }
    px_srcline(344);
    return px_list_n((LXValue[]){px_str("If"), _v375, _v376, _v372}, 4);
px_err_379:
    if (px_err_379_proped) return px_err_379_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_for");
    LXValue _v380 = px_null();
    LXValue _v381 = px_null();
    LXValue _v382 = px_null();
    LXValue _v383 = px_null();
    LXValue px_err_384_val = px_null();
    int px_err_384_proped = 0;
    px_srcline(346);
    _v380 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(347);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(348);
    _v381 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    px_srcline(349);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    px_srcline(350);
    _v382 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(351);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(352);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(353);
    _v383 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(354);
    return px_list_n((LXValue[]){px_str("For"), px_call(px_get_global("qstr"), (LXValue[]){_v381}, 1), _v382, _v383, _v380}, 5);
px_err_384:
    if (px_err_384_proped) return px_err_384_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_while");
    LXValue _v385 = px_null();
    LXValue _v386 = px_null();
    LXValue _v387 = px_null();
    LXValue px_err_388_val = px_null();
    int px_err_388_proped = 0;
    px_srcline(356);
    _v385 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(357);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(358);
    _v386 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(359);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(360);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(361);
    _v387 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(362);
    return px_list_n((LXValue[]){px_str("While"), _v386, _v387, _v385}, 4);
px_err_388:
    if (px_err_388_proped) return px_err_388_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block");
    LXValue _v389 = px_null();
    LXValue px_err_390_val = px_null();
    int px_err_390_proped = 0;
    px_srcline(364);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(365);
    _v389 = px_list_n((LXValue[]){}, 0);
    px_srcline(366);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(367);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(368);
        if (px_is_truthy(({ LXValue _t391 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t391) ? _t391 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(369);
            break;
        }
        px_srcline(370);
        (void)(px_method(_v389, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(371);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(372);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    px_srcline(373);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(374);
    return _v389;
px_err_390:
    if (px_err_390_proped) return px_err_390_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_params");
    LXValue _v392 = px_null();
    LXValue _v393 = px_null();
    LXValue _v394 = px_null();
    LXValue _v395 = px_null();
    LXValue px_err_396_val = px_null();
    int px_err_396_proped = 0;
    px_srcline(378);
    _v392 = px_list_n((LXValue[]){}, 0);
    px_srcline(379);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        px_srcline(380);
        return _v392;
    }
    px_srcline(381);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(382);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(383);
        _v393 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        px_srcline(384);
        _v394 = _v393;
        px_srcline(385);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            px_srcline(386);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(387);
            _v395 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
            px_srcline(388);
             _v394 = px_add(px_add(_v393, px_str(": ")), _v395);
        }
        px_srcline(389);
        (void)(px_method(_v392, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v394}, 1)}, 1));
        px_srcline(390);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(391);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(392);
            continue;
        }
        px_srcline(393);
        break;
    }
    px_srcline(394);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(395);
    return _v392;
px_err_396:
    if (px_err_396_proped) return px_err_396_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_func_def");
    LXValue _v397 = px_null();
    LXValue _v398 = px_null();
    LXValue _v399 = px_null();
    LXValue _v400 = px_null();
    LXValue _v401 = px_null();
    LXValue _v402 = px_null();
    LXValue px_err_403_val = px_null();
    int px_err_403_proped = 0;
    px_srcline(397);
    _v397 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(398);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(399);
    _v398 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(400);
    _v399 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(401);
    _v400 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(402);
    _v401 = px_null();
    px_srcline(403);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(404);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(405);
         _v401 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(406);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(407);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(408);
    _v402 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(409);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v398}, 1), _v400, _v401, _v402, _v397, _v399}, 7);
px_err_403:
    if (px_err_403_proped) return px_err_403_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_extern_def");
    LXValue _v404 = px_null();
    LXValue _v405 = px_null();
    LXValue _v406 = px_null();
    LXValue _v407 = px_null();
    LXValue px_err_408_val = px_null();
    int px_err_408_proped = 0;
    px_srcline(413);
    _v404 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(414);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(415);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    px_srcline(416);
    _v405 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(417);
    _v406 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(418);
    _v407 = px_null();
    px_srcline(419);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(420);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(421);
         _v407 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(422);
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v405}, 1), _v406, _v407, _v404}, 5);
px_err_408:
    if (px_err_408_proped) return px_err_408_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_struct_def");
    LXValue _v409 = px_null();
    LXValue _v410 = px_null();
    LXValue _v411 = px_null();
    LXValue _v412 = px_null();
    LXValue _v413 = px_null();
    LXValue _v414 = px_null();
    LXValue _v415 = px_null();
    LXValue px_err_416_val = px_null();
    int px_err_416_proped = 0;
    px_srcline(424);
    _v409 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(425);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(426);
    _v410 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    px_srcline(427);
    _v411 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(428);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(429);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(430);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(431);
    _v412 = px_list_n((LXValue[]){}, 0);
    px_srcline(432);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(433);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(434);
        if (px_is_truthy(({ LXValue _t417 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t417) ? _t417 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(435);
            break;
        }
        px_srcline(436);
        _v413 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(437);
        _v414 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        px_srcline(438);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(439);
        _v415 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(440);
        (void)(px_method(_v412, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v414}, 1), _v415, _v413}, 4)}, 1));
        px_srcline(441);
        if (px_is_truthy(({ LXValue _t418 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t418) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t418; }))) {
            px_srcline(442);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    px_srcline(443);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(444);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    px_srcline(445);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(446);
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v410}, 1), _v412, _v409, _v411}, 5);
px_err_416:
    if (px_err_416_proped) return px_err_416_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_enum_def");
    LXValue _v419 = px_null();
    LXValue _v420 = px_null();
    LXValue _v421 = px_null();
    LXValue _v422 = px_null();
    LXValue _v423 = px_null();
    LXValue _v424 = px_null();
    LXValue px_err_425_val = px_null();
    int px_err_425_proped = 0;
    px_srcline(448);
    _v419 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(449);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(450);
    _v420 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(451);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(452);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(453);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(454);
    _v421 = px_list_n((LXValue[]){}, 0);
    px_srcline(455);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(456);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(457);
        if (px_is_truthy(({ LXValue _t426 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t426) ? _t426 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(458);
            break;
        }
        px_srcline(459);
        _v422 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(460);
        _v423 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        px_srcline(461);
        _v424 = px_list_n((LXValue[]){}, 0);
        px_srcline(462);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(463);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(464);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(465);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(466);
                    (void)(px_method(_v424, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    px_srcline(467);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(468);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(469);
                        continue;
                    }
                    px_srcline(470);
                    break;
                }
            }
            px_srcline(471);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        px_srcline(472);
        (void)(px_method(_v421, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v423}, 1), _v424, _v422}, 4)}, 1));
        px_srcline(473);
        if (px_is_truthy(({ LXValue _t427 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t427) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t427; }))) {
            px_srcline(474);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    px_srcline(475);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(476);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    px_srcline(477);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(478);
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v420}, 1), _v421, _v419}, 4);
px_err_425:
    if (px_err_425_proped) return px_err_425_val;
    return px_null();
}

static LXValue fn_parse_type_const(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_const");
    LXValue _v428 = px_null();
    LXValue _v429 = px_null();
    LXValue _v430 = px_null();
    LXValue _v431 = px_null();
    LXValue _v432 = px_null();
    LXValue _v433 = px_null();
    LXValue px_err_434_val = px_null();
    int px_err_434_proped = 0;
    px_srcline(483);
    _v428 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(484);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(485);
    _v429 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(486);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("const"), px_str("'const'")}, 2));
    px_srcline(487);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(488);
    _v430 = px_list_n((LXValue[]){}, 0);
    px_srcline(489);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(490);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(491);
            _v431 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(492);
            _v432 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("常量名")}, 1);
            px_srcline(493);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
            px_srcline(494);
            _v433 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(495);
            (void)(px_method(_v430, "append", (LXValue[]){px_list_n((LXValue[]){px_str("TypeConstItem"), px_call(px_get_global("qstr"), (LXValue[]){_v432}, 1), _v433, _v431}, 4)}, 1));
            px_srcline(496);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(497);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(498);
                continue;
            }
            px_srcline(499);
            break;
        }
    }
    px_srcline(500);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(501);
    return px_list_n((LXValue[]){px_str("TypeConst"), px_call(px_get_global("qstr"), (LXValue[]){_v429}, 1), _v430, _v428}, 4);
px_err_434:
    if (px_err_434_proped) return px_err_434_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_trait_def");
    LXValue _v435 = px_null();
    LXValue _v436 = px_null();
    LXValue _v437 = px_null();
    LXValue _v438 = px_null();
    LXValue _v439 = px_null();
    LXValue _v440 = px_null();
    LXValue _v441 = px_null();
    LXValue _v442 = px_null();
    LXValue px_err_443_val = px_null();
    int px_err_443_proped = 0;
    px_srcline(503);
    _v435 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(504);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(505);
    _v436 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    px_srcline(506);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(507);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(508);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(509);
    _v437 = px_list_n((LXValue[]){}, 0);
    px_srcline(510);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(511);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(512);
        if (px_is_truthy(({ LXValue _t444 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t444) ? _t444 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(513);
            break;
        }
        px_srcline(514);
        _v438 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(515);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(516);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        px_srcline(517);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(518);
        _v439 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(519);
        _v440 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(520);
        _v441 = px_null();
        px_srcline(521);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(522);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(523);
             _v441 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(524);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(525);
        _v442 = px_list_n((LXValue[]){}, 0);
        px_srcline(526);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(527);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(528);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(529);
                 _v442 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        px_srcline(530);
        (void)(px_method(_v437, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v439}, 1), _v440, _v441, _v442, _v438, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        px_srcline(531);
        if (px_is_truthy(({ LXValue _t445 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t445) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t445; }))) {
            px_srcline(532);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    px_srcline(533);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(534);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    px_srcline(535);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(536);
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v436}, 1), _v437, _v435}, 4);
px_err_443:
    if (px_err_443_proped) return px_err_443_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_impl_def");
    LXValue _v446 = px_null();
    LXValue _v447 = px_null();
    LXValue _v448 = px_null();
    LXValue _v449 = px_null();
    LXValue _v450 = px_null();
    LXValue _v451 = px_null();
    LXValue _v452 = px_null();
    LXValue _v453 = px_null();
    LXValue _v454 = px_null();
    LXValue _v455 = px_null();
    LXValue px_err_456_val = px_null();
    int px_err_456_proped = 0;
    px_srcline(538);
    _v446 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(539);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(540);
    _v447 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    px_srcline(541);
    _v448 = px_null();
    px_srcline(542);
    _v449 = _v447;
    px_srcline(543);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(544);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(545);
         _v449 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
        px_srcline(546);
         _v448 = px_call(px_get_global("qstr"), (LXValue[]){_v447}, 1);
    }
    px_srcline(547);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(548);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(549);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(550);
    _v450 = px_list_n((LXValue[]){}, 0);
    px_srcline(551);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(552);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(553);
        if (px_is_truthy(({ LXValue _t457 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t457) ? _t457 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(554);
            break;
        }
        px_srcline(555);
        _v451 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(556);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(557);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        px_srcline(558);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(559);
        _v452 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(560);
        _v453 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(561);
        _v454 = px_null();
        px_srcline(562);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(563);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(564);
             _v454 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(565);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(566);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(567);
        _v455 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        px_srcline(568);
        (void)(px_method(_v450, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v452}, 1), _v453, _v454, _v455, _v451, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    px_srcline(569);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(570);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    px_srcline(571);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(572);
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v449}, 1), _v448, _v450, _v446}, 5);
px_err_456:
    if (px_err_456_proped) return px_err_456_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import");
    LXValue _v458 = px_null();
    LXValue _v459 = px_null();
    LXValue _v460 = px_null();
    LXValue px_err_461_val = px_null();
    int px_err_461_proped = 0;
    px_srcline(574);
    _v458 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(575);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(576);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        px_srcline(577);
        _v459 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(578);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(579);
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v459}, 1), px_list_n((LXValue[]){}, 0), _v458}, 4);
    }
    px_srcline(580);
    _v460 = px_list_n((LXValue[]){}, 0);
    px_srcline(581);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(582);
        (void)(px_method(_v460, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(583);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(584);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(585);
            continue;
        }
        px_srcline(586);
        break;
    }
    px_srcline(587);
    return px_list_n((LXValue[]){px_str("Import"), _v460, px_list_n((LXValue[]){}, 0), _v458}, 4);
px_err_461:
    if (px_err_461_proped) return px_err_461_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import_from");
    LXValue _v462 = px_null();
    LXValue _v463 = px_null();
    LXValue _v464 = px_null();
    LXValue px_err_465_val = px_null();
    int px_err_465_proped = 0;
    px_srcline(589);
    _v462 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(590);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(591);
    _v463 = px_list_n((LXValue[]){}, 0);
    px_srcline(592);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(593);
        (void)(px_method(_v463, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(594);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(595);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(596);
            continue;
        }
        px_srcline(597);
        break;
    }
    px_srcline(598);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    px_srcline(599);
    _v464 = px_list_n((LXValue[]){}, 0);
    px_srcline(600);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(601);
        (void)(px_method(_v464, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        px_srcline(602);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(603);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(604);
            continue;
        }
        px_srcline(605);
        break;
    }
    px_srcline(606);
    return px_list_n((LXValue[]){px_str("Import"), _v463, _v464, _v462}, 4);
px_err_465:
    if (px_err_465_proped) return px_err_465_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_select");
    LXValue _v466 = px_null();
    LXValue _v467 = px_null();
    LXValue _v468 = px_null();
    LXValue _v469 = px_null();
    LXValue _v470 = px_null();
    LXValue _v471 = px_null();
    LXValue _v472 = px_null();
    LXValue px_err_473_val = px_null();
    int px_err_473_proped = 0;
    px_srcline(608);
    _v466 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(609);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(610);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(611);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(612);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(613);
    _v467 = px_list_n((LXValue[]){}, 0);
    px_srcline(614);
    _v468 = px_null();
    px_srcline(615);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(616);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(617);
        if (px_is_truthy(({ LXValue _t474 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t474) ? _t474 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(618);
            break;
        }
        px_srcline(619);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(620);
        if (px_is_truthy(({ LXValue _t475 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t475) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t475; }))) {
            px_srcline(621);
            _v469 = px_get_global("p_pos");
            px_srcline(622);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(623);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(624);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(625);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                px_srcline(626);
                 _v468 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                px_srcline(627);
                continue;
            }
            else {
                px_srcline(629);
                px_set_global("p_pos", _v469);
            }
        }
        px_srcline(630);
        _v470 = px_null();
        px_srcline(631);
        if (px_is_truthy(({ LXValue _t476 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t476) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t476; }))) {
            px_srcline(632);
             _v470 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            px_srcline(633);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        px_srcline(634);
        _v471 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(635);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(636);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(637);
        _v472 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        px_srcline(638);
        (void)(px_method(_v467, "append", (LXValue[]){px_list_n((LXValue[]){_v470, _v471, _v472}, 3)}, 1));
    }
    px_srcline(639);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(640);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    px_srcline(641);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(642);
    return px_list_n((LXValue[]){px_str("Select"), _v467, _v468, _v466}, 4);
px_err_473:
    if (px_err_473_proped) return px_err_473_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_case_body");
    LXValue _v477 = px_null();
    LXValue _v478 = px_null();
    LXValue px_err_479_val = px_null();
    int px_err_479_proped = 0;
    px_srcline(644);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(645);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(646);
    _v477 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(647);
    _v478 = px_call(px_get_global("node_pos"), (LXValue[]){_v477}, 1);
    px_srcline(648);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v477, _v478}, 3)}, 1);
px_err_479:
    if (px_err_479_proped) return px_err_479_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_params");
    LXValue _v480 = px_null();
    LXValue _v481 = px_null();
    LXValue _v482 = px_null();
    LXValue _v483 = px_null();
    LXValue _v484 = px_null();
    LXValue px_err_485_val = px_null();
    int px_err_485_proped = 0;
    px_srcline(651);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(652);
    _v480 = px_list_n((LXValue[]){}, 0);
    px_srcline(653);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(654);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(655);
            _v481 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(656);
            _v482 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            px_srcline(657);
            _v483 = px_null();
            px_srcline(658);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(659);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(660);
                 _v483 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            px_srcline(661);
            _v484 = px_null();
            px_srcline(662);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                px_srcline(663);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(664);
                 _v484 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            px_srcline(665);
            (void)(px_method(_v480, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v482}, 1), _v483, _v484, _v481}, 5)}, 1));
            px_srcline(666);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(667);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(668);
                continue;
            }
            px_srcline(669);
            break;
        }
    }
    px_srcline(670);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(671);
    return _v480;
px_err_485:
    if (px_err_485_proped) return px_err_485_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_expr");
    LXValue px_err_486_val = px_null();
    int px_err_486_proped = 0;
    px_srcline(674);
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_486:
    if (px_err_486_proped) return px_err_486_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pipe");
    LXValue _v487 = px_null();
    LXValue _v488 = px_null();
    LXValue _v489 = px_null();
    LXValue px_err_490_val = px_null();
    int px_err_490_proped = 0;
    px_srcline(676);
    _v487 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    px_srcline(677);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|>")}, 1))) {
        px_srcline(678);
        _v488 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(679);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(680);
        _v489 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
        px_srcline(681);
         _v487 = px_list_n((LXValue[]){px_str("Pipe"), _v487, _v489, _v488}, 4);
    }
    px_srcline(682);
    return _v487;
px_err_490:
    if (px_err_490_proped) return px_err_490_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_null_coalesce");
    LXValue _v491 = px_null();
    LXValue _v492 = px_null();
    LXValue _v493 = px_null();
    LXValue px_err_494_val = px_null();
    int px_err_494_proped = 0;
    px_srcline(684);
    _v491 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    px_srcline(685);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("??")}, 1))) {
        px_srcline(686);
        _v492 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(687);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(688);
        _v493 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
        px_srcline(689);
         _v491 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v491, _v493, _v492}, 4);
    }
    px_srcline(690);
    return _v491;
px_err_494:
    if (px_err_494_proped) return px_err_494_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_or");
    LXValue _v495 = px_null();
    LXValue _v496 = px_null();
    LXValue _v497 = px_null();
    LXValue px_err_498_val = px_null();
    int px_err_498_proped = 0;
    px_srcline(692);
    _v495 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    px_srcline(693);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("or")}, 1))) {
        px_srcline(694);
        _v496 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(695);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(696);
        _v497 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
        px_srcline(697);
         _v495 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v495, _v497, _v496}, 5);
    }
    px_srcline(698);
    return _v495;
px_err_498:
    if (px_err_498_proped) return px_err_498_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_and");
    LXValue _v499 = px_null();
    LXValue _v500 = px_null();
    LXValue _v501 = px_null();
    LXValue px_err_502_val = px_null();
    int px_err_502_proped = 0;
    px_srcline(700);
    _v499 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    px_srcline(701);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("and")}, 1))) {
        px_srcline(702);
        _v500 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(703);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(704);
        _v501 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
        px_srcline(705);
         _v499 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v499, _v501, _v500}, 5);
    }
    px_srcline(706);
    return _v499;
px_err_502:
    if (px_err_502_proped) return px_err_502_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comparison");
    LXValue _v503 = px_null();
    LXValue _v504 = px_null();
    LXValue _v505 = px_null();
    LXValue _v506 = px_null();
    LXValue px_err_507_val = px_null();
    int px_err_507_proped = 0;
    px_srcline(708);
    _v503 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    px_srcline(709);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(710);
        _v504 = px_null();
        px_srcline(711);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("==")}, 1))) {
            px_srcline(712);
             _v504 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("!=")}, 1))) {
            px_srcline(714);
             _v504 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<")}, 1))) {
            px_srcline(716);
             _v504 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<=")}, 1))) {
            px_srcline(718);
             _v504 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">")}, 1))) {
            px_srcline(720);
             _v504 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">=")}, 1))) {
            px_srcline(722);
             _v504 = px_str("Ge");
        }
        px_srcline(723);
        if (px_is_truthy(px_eq(_v504, px_null()))) {
            px_srcline(724);
            break;
        }
        px_srcline(725);
        _v505 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(726);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(727);
        _v506 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
        px_srcline(728);
         _v503 = px_list_n((LXValue[]){px_str("Binary"), _v504, _v503, _v506, _v505}, 5);
    }
    px_srcline(729);
    return _v503;
px_err_507:
    if (px_err_507_proped) return px_err_507_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitor");
    LXValue _v508 = px_null();
    LXValue _v509 = px_null();
    LXValue _v510 = px_null();
    LXValue px_err_511_val = px_null();
    int px_err_511_proped = 0;
    px_srcline(731);
    _v508 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    px_srcline(732);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("|")}, 1))) {
        px_srcline(733);
        _v509 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(734);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(735);
        _v510 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
        px_srcline(736);
         _v508 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v508, _v510, _v509}, 5);
    }
    px_srcline(737);
    return _v508;
px_err_511:
    if (px_err_511_proped) return px_err_511_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitxor");
    LXValue _v512 = px_null();
    LXValue _v513 = px_null();
    LXValue _v514 = px_null();
    LXValue px_err_515_val = px_null();
    int px_err_515_proped = 0;
    px_srcline(739);
    _v512 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    px_srcline(740);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("^")}, 1))) {
        px_srcline(741);
        _v513 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(742);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(743);
        _v514 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
        px_srcline(744);
         _v512 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v512, _v514, _v513}, 5);
    }
    px_srcline(745);
    return _v512;
px_err_515:
    if (px_err_515_proped) return px_err_515_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitand");
    LXValue _v516 = px_null();
    LXValue _v517 = px_null();
    LXValue _v518 = px_null();
    LXValue px_err_519_val = px_null();
    int px_err_519_proped = 0;
    px_srcline(747);
    _v516 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    px_srcline(748);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("&")}, 1))) {
        px_srcline(749);
        _v517 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(750);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(751);
        _v518 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
        px_srcline(752);
         _v516 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v516, _v518, _v517}, 5);
    }
    px_srcline(753);
    return _v516;
px_err_519:
    if (px_err_519_proped) return px_err_519_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_shift");
    LXValue _v520 = px_null();
    LXValue _v521 = px_null();
    LXValue _v522 = px_null();
    LXValue _v523 = px_null();
    LXValue px_err_524_val = px_null();
    int px_err_524_proped = 0;
    px_srcline(755);
    _v520 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    px_srcline(756);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(757);
        _v521 = px_null();
        px_srcline(758);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("<<")}, 1))) {
            px_srcline(759);
             _v521 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>")}, 1))) {
            px_srcline(761);
             _v521 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(">>>")}, 1))) {
            px_srcline(763);
             _v521 = px_str("ShrU");
        }
        px_srcline(764);
        if (px_is_truthy(px_eq(_v521, px_null()))) {
            px_srcline(765);
            break;
        }
        px_srcline(766);
        _v522 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(767);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(768);
        _v523 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
        px_srcline(769);
         _v520 = px_list_n((LXValue[]){px_str("Binary"), _v521, _v520, _v523, _v522}, 5);
    }
    px_srcline(770);
    return _v520;
px_err_524:
    if (px_err_524_proped) return px_err_524_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_add");
    LXValue _v525 = px_null();
    LXValue _v526 = px_null();
    LXValue _v527 = px_null();
    LXValue _v528 = px_null();
    LXValue px_err_529_val = px_null();
    int px_err_529_proped = 0;
    px_srcline(772);
    _v525 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    px_srcline(773);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(774);
        _v526 = px_null();
        px_srcline(775);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("+")}, 1))) {
            px_srcline(776);
             _v526 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("-")}, 1))) {
            px_srcline(778);
             _v526 = px_str("Sub");
        }
        px_srcline(779);
        if (px_is_truthy(px_eq(_v526, px_null()))) {
            px_srcline(780);
            break;
        }
        px_srcline(781);
        _v527 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(782);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(783);
        _v528 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
        px_srcline(784);
         _v525 = px_list_n((LXValue[]){px_str("Binary"), _v526, _v525, _v528, _v527}, 5);
    }
    px_srcline(785);
    return _v525;
px_err_529:
    if (px_err_529_proped) return px_err_529_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_mul");
    LXValue _v530 = px_null();
    LXValue _v531 = px_null();
    LXValue _v532 = px_null();
    LXValue _v533 = px_null();
    LXValue px_err_534_val = px_null();
    int px_err_534_proped = 0;
    px_srcline(787);
    _v530 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    px_srcline(788);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(789);
        _v531 = px_null();
        px_srcline(790);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("*")}, 1))) {
            px_srcline(791);
             _v531 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("/")}, 1))) {
            px_srcline(793);
             _v531 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("//")}, 1))) {
            px_srcline(795);
             _v531 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("%")}, 1))) {
            px_srcline(797);
             _v531 = px_str("Mod");
        }
        px_srcline(798);
        if (px_is_truthy(px_eq(_v531, px_null()))) {
            px_srcline(799);
            break;
        }
        px_srcline(800);
        _v532 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(801);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(802);
        _v533 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(803);
         _v530 = px_list_n((LXValue[]){px_str("Binary"), _v531, _v530, _v533, _v532}, 5);
    }
    px_srcline(804);
    return _v530;
px_err_534:
    if (px_err_534_proped) return px_err_534_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pow");
    LXValue _v535 = px_null();
    LXValue _v536 = px_null();
    LXValue _v537 = px_null();
    LXValue px_err_538_val = px_null();
    int px_err_538_proped = 0;
    px_srcline(806);
    _v535 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    px_srcline(807);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("**")}, 1))) {
        px_srcline(808);
        _v536 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(809);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(810);
        _v537 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(811);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v535, _v537, _v536}, 5);
    }
    px_srcline(812);
    return _v535;
px_err_538:
    if (px_err_538_proped) return px_err_538_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_unary");
    LXValue _v539 = px_null();
    LXValue _v540 = px_null();
    LXValue _v541 = px_null();
    LXValue px_err_542_val = px_null();
    int px_err_542_proped = 0;
    px_srcline(814);
    _v539 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(815);
    if (px_is_truthy(px_eq(_v539, px_str("-")))) {
        px_srcline(816);
        _v540 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(817);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(818);
        _v541 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(819);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v541, _v540}, 4);
    }
    px_srcline(820);
    if (px_is_truthy(px_eq(_v539, px_str("not")))) {
        px_srcline(821);
        _v540 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(822);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(823);
        _v541 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(824);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v541, _v540}, 4);
    }
    px_srcline(825);
    if (px_is_truthy(px_eq(_v539, px_str("~")))) {
        px_srcline(826);
        _v540 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(827);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(828);
        _v541 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(829);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v541, _v540}, 4);
    }
    px_srcline(830);
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_542:
    if (px_err_542_proped) return px_err_542_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_postfix");
    LXValue _v543 = px_null();
    LXValue _v544 = px_null();
    LXValue _v545 = px_null();
    LXValue _v546 = px_null();
    LXValue _v547 = px_null();
    LXValue _v548 = px_null();
    LXValue _v549 = px_null();
    LXValue _v550 = px_null();
    LXValue px_err_551_val = px_null();
    int px_err_551_proped = 0;
    px_srcline(832);
    _v543 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    px_srcline(833);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(834);
        _v544 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(835);
        if (px_is_truthy(px_eq(_v544, px_str("(")))) {
            px_srcline(836);
            _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(837);
            _v546 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(838);
             _v543 = px_list_n((LXValue[]){px_str("Call"), _v543, _v546, _v545}, 4);
        }
        else if (px_is_truthy(px_eq(_v544, px_str("[")))) {
            px_srcline(840);
            _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(841);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(842);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(843);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(844);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(845);
                _v547 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                px_srcline(846);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(847);
                _v548 = px_null();
                px_srcline(848);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(849);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(850);
                     _v548 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(851);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                }
                px_srcline(852);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                px_srcline(853);
                 _v543 = px_list_n((LXValue[]){px_str("Slice"), _v543, px_null(), _v547, _v548, _v545}, 6);
            }
            else {
                px_srcline(855);
                _v549 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                px_srcline(856);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(857);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(858);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(859);
                    _v547 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(860);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    px_srcline(861);
                    _v548 = px_null();
                    px_srcline(862);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        px_srcline(863);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(864);
                         _v548 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                        px_srcline(865);
                        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    }
                    px_srcline(866);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(867);
                     _v543 = px_list_n((LXValue[]){px_str("Slice"), _v543, _v549, _v547, _v548, _v545}, 6);
                }
                else {
                    px_srcline(869);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(870);
                     _v543 = px_list_n((LXValue[]){px_str("Index"), _v543, _v549, _v545}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v544, px_str(".")))) {
            px_srcline(872);
            _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(873);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(874);
            _v550 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(875);
             _v543 = px_list_n((LXValue[]){px_str("Field"), _v543, px_call(px_get_global("qstr"), (LXValue[]){_v550}, 1), _v545}, 4);
        }
        else if (px_is_truthy(px_eq(_v544, px_str("?.")))) {
            px_srcline(877);
            _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(878);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(879);
            _v550 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(880);
             _v543 = px_list_n((LXValue[]){px_str("OptionalField"), _v543, px_call(px_get_global("qstr"), (LXValue[]){_v550}, 1), _v545}, 4);
        }
        else if (px_is_truthy(px_eq(_v544, px_str("!")))) {
            px_srcline(882);
            _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(883);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(884);
             _v543 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v543, _v545}, 3);
        }
        else if (px_is_truthy(px_eq(_v544, px_str("?")))) {
            px_srcline(886);
            _v545 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(887);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(888);
             _v543 = px_list_n((LXValue[]){px_str("Try"), _v543, _v545}, 3);
        }
        else {
            px_srcline(890);
            break;
        }
    }
    px_srcline(891);
    return _v543;
px_err_551:
    if (px_err_551_proped) return px_err_551_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_slice_bound");
    LXValue px_err_552_val = px_null();
    int px_err_552_proped = 0;
    px_srcline(893);
    if (px_is_truthy(({ LXValue _t553 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t553) ? _t553 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        px_srcline(894);
        return px_null();
    }
    px_srcline(895);
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_552:
    if (px_err_552_proped) return px_err_552_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_call_args");
    LXValue _v554 = px_null();
    LXValue px_err_555_val = px_null();
    int px_err_555_proped = 0;
    px_srcline(897);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(898);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(899);
    _v554 = px_list_n((LXValue[]){}, 0);
    px_srcline(900);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(901);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(902);
            (void)(px_method(_v554, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(903);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(904);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(905);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(906);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(907);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                    px_srcline(908);
                    break;
                }
                px_srcline(909);
                continue;
            }
            px_srcline(910);
            break;
        }
    }
    px_srcline(911);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(912);
    return _v554;
px_err_555:
    if (px_err_555_proped) return px_err_555_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_primary");
    LXValue _v556 = px_null();
    LXValue _v557 = px_null();
    LXValue _v558 = px_null();
    LXValue _v559 = px_null();
    LXValue _v560 = px_null();
    LXValue _v561 = px_null();
    LXValue px_err_562_val = px_null();
    int px_err_562_proped = 0;
    px_srcline(915);
    _v556 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(916);
    if (px_is_truthy(px_eq(_v556, px_str("整数")))) {
        px_srcline(917);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(918);
        _v558 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(919);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(920);
        return px_list_n((LXValue[]){px_str("Int"), _v558, _v557}, 3);
    }
    px_srcline(921);
    if (px_is_truthy(px_eq(_v556, px_str("浮点")))) {
        px_srcline(922);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(923);
        _v558 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(924);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(925);
        return px_list_n((LXValue[]){px_str("Float"), _v558, _v557}, 3);
    }
    px_srcline(926);
    if (px_is_truthy(px_eq(_v556, px_str("字符串")))) {
        px_srcline(927);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(928);
        _v558 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(929);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(930);
        return px_list_n((LXValue[]){px_str("Str"), _v558, _v557}, 3);
    }
    px_srcline(931);
    if (px_is_truthy(px_eq(_v556, px_str("true")))) {
        px_srcline(932);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(933);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(934);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v557}, 3);
    }
    px_srcline(935);
    if (px_is_truthy(px_eq(_v556, px_str("false")))) {
        px_srcline(936);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(937);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(938);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v557}, 3);
    }
    px_srcline(939);
    if (px_is_truthy(px_eq(_v556, px_str("null")))) {
        px_srcline(940);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(941);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(942);
        return px_list_n((LXValue[]){px_str("Null"), _v557}, 2);
    }
    px_srcline(943);
    if (px_is_truthy(px_eq(_v556, px_str("self")))) {
        px_srcline(944);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(945);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(946);
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v557}, 3);
    }
    px_srcline(947);
    if (px_is_truthy(px_eq(_v556, px_str("标识符")))) {
        px_srcline(948);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(949);
        _v559 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(950);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(951);
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v559}, 1), _v557}, 3);
    }
    px_srcline(952);
    if (px_is_truthy(px_eq(_v556, px_str("[")))) {
        px_srcline(953);
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    px_srcline(954);
    if (px_is_truthy(px_eq(_v556, px_str("(")))) {
        px_srcline(955);
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    px_srcline(956);
    if (px_is_truthy(px_eq(_v556, px_str("{")))) {
        px_srcline(957);
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    px_srcline(958);
    if (px_is_truthy(px_eq(_v556, px_str("fn")))) {
        px_srcline(959);
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    px_srcline(960);
    if (px_is_truthy(px_eq(_v556, px_str("match")))) {
        px_srcline(961);
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    px_srcline(962);
    if (px_is_truthy(px_eq(_v556, px_str("if")))) {
        px_srcline(963);
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    px_srcline(964);
    if (px_is_truthy(px_eq(_v556, px_str("chan")))) {
        px_srcline(965);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(966);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(967);
        _v560 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v557}, 3);
        px_srcline(968);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(969);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(970);
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            px_srcline(971);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        px_srcline(972);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(973);
            _v561 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(974);
             _v560 = px_list_n((LXValue[]){px_str("Call"), _v560, _v561, _v557}, 4);
        }
        px_srcline(975);
        return _v560;
    }
    px_srcline(976);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(977);
    return px_null();
px_err_562:
    if (px_err_562_proped) return px_err_562_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_list_or_comp");
    LXValue _v563 = px_null();
    LXValue _v564 = px_null();
    LXValue _v565 = px_null();
    LXValue _v566 = px_null();
    LXValue px_err_567_val = px_null();
    int px_err_567_proped = 0;
    px_srcline(979);
    _v563 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(980);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(981);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(982);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        px_srcline(983);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(984);
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v563}, 3);
    }
    px_srcline(985);
    _v564 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(986);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(987);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(988);
        _v565 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(989);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(990);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(991);
        return px_list_n((LXValue[]){px_str("ListComp"), _v564, px_index(_v565, px_int(0LL)), px_index(_v565, px_int(1LL)), _v563}, 5);
    }
    px_srcline(992);
    _v566 = px_list_n((LXValue[]){_v564}, 1);
    px_srcline(993);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(994);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(995);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(996);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            px_srcline(997);
            break;
        }
        px_srcline(998);
        (void)(px_method(_v566, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        px_srcline(999);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    }
    px_srcline(1000);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(1001);
    return px_list_n((LXValue[]){px_str("List"), _v566, _v563}, 3);
px_err_567:
    if (px_err_567_proped) return px_err_567_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_vars");
    LXValue _v568 = px_null();
    LXValue px_err_569_val = px_null();
    int px_err_569_proped = 0;
    px_srcline(1003);
    _v568 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    px_srcline(1004);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1005);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1006);
        (void)(px_method(_v568, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    px_srcline(1007);
    return _v568;
px_err_569:
    if (px_err_569_proped) return px_err_569_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_clauses");
    LXValue _v570 = px_null();
    LXValue _v571 = px_null();
    LXValue _v572 = px_null();
    LXValue _v573 = px_null();
    LXValue px_err_574_val = px_null();
    int px_err_574_proped = 0;
    px_srcline(1009);
    _v570 = px_list_n((LXValue[]){}, 0);
    px_srcline(1010);
    _v571 = px_list_n((LXValue[]){}, 0);
    px_srcline(1011);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1012);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1013);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1014);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1015);
            _v572 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            px_srcline(1016);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            px_srcline(1017);
            _v573 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1018);
            (void)(px_method(_v570, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v572, _v573}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1020);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1021);
            (void)(px_method(_v571, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(1023);
            break;
        }
    }
    px_srcline(1024);
    return px_list_n((LXValue[]){_v570, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v571}, 1)}, 2);
px_err_574:
    if (px_err_574_proped) return px_err_574_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fold_comp_conds");
    LXValue _v575 = (nargs > 0) ? args[0] : px_null();
    LXValue _v576 = px_null();
    LXValue _v577 = px_null();
    LXValue _v578 = px_null();
    LXValue px_err_579_val = px_null();
    int px_err_579_proped = 0;
    px_srcline(1026);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v575}, 1), px_int(0LL)))) {
        px_srcline(1027);
        return px_null();
    }
    px_srcline(1028);
    _v576 = px_index(_v575, px_int(0LL));
    px_srcline(1029);
    _v577 = px_int(1LL);
    px_srcline(1030);
    while (px_is_truthy(px_lt(_v577, px_call(px_get_global("len"), (LXValue[]){_v575}, 1)))) {
        px_srcline(1031);
        _v578 = px_call(px_get_global("node_pos"), (LXValue[]){_v576}, 1);
        px_srcline(1032);
         _v576 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v576, px_index(_v575, _v577), _v578}, 5);
        px_srcline(1033);
         _v577 = px_add(_v577, px_int(1LL));
    }
    px_srcline(1034);
    return _v576;
px_err_579:
    if (px_err_579_proped) return px_err_579_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_paren_or_tuple");
    LXValue _v580 = px_null();
    LXValue _v581 = px_null();
    LXValue _v582 = px_null();
    LXValue _v583 = px_null();
    LXValue px_err_584_val = px_null();
    int px_err_584_proped = 0;
    px_srcline(1036);
    _v580 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1037);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1038);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1039);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        px_srcline(1040);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1041);
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v580}, 3);
    }
    px_srcline(1042);
    _v581 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1043);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1044);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1045);
        _v582 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1046);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1047);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1048);
        return px_list_n((LXValue[]){px_str("GenExp"), _v581, px_index(_v582, px_int(0LL)), px_index(_v582, px_int(1LL)), _v580}, 5);
    }
    px_srcline(1049);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1050);
        _v583 = px_list_n((LXValue[]){_v581}, 1);
        px_srcline(1051);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1052);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1053);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1054);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                px_srcline(1055);
                break;
            }
            px_srcline(1056);
            (void)(px_method(_v583, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1057);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        }
        px_srcline(1058);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1059);
        return px_list_n((LXValue[]){px_str("Tuple"), _v583, _v580}, 3);
    }
    px_srcline(1060);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1061);
    return _v581;
px_err_584:
    if (px_err_584_proped) return px_err_584_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("brace_looks_like_dict");
    LXValue _v585 = px_null();
    LXValue _v586 = px_null();
    LXValue _v587 = px_null();
    LXValue px_err_588_val = px_null();
    int px_err_588_proped = 0;
    px_srcline(1063);
    _v585 = px_int(0LL);
    px_srcline(1064);
    _v586 = px_get_global("p_pos");
    px_srcline(1065);
    while (px_is_truthy(px_lt(_v586, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(1066);
        _v587 = px_index(px_index(px_get_global("p_toks"), _v586), px_int(0LL));
        px_srcline(1067);
        if (px_is_truthy(({ LXValue _t589 = px_eq(_v587, px_str(":")); px_is_truthy(_t589) ? px_eq(_v585, px_int(0LL)) : _t589; }))) {
            px_srcline(1068);
            return px_bool(true);
        }
        px_srcline(1069);
        if (px_is_truthy(({ LXValue _t591 = ({ LXValue _t590 = px_eq(_v587, px_str("(")); px_is_truthy(_t590) ? _t590 : px_eq(_v587, px_str("[")); }); px_is_truthy(_t591) ? _t591 : px_eq(_v587, px_str("{")); }))) {
            px_srcline(1070);
             _v585 = px_add(_v585, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t592 = px_eq(_v587, px_str(")")); px_is_truthy(_t592) ? _t592 : px_eq(_v587, px_str("]")); }))) {
            px_srcline(1072);
            if (px_is_truthy(px_gt(_v585, px_int(0LL)))) {
                px_srcline(1073);
                 _v585 = px_sub(_v585, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t593 = px_eq(_v587, px_str("}")); px_is_truthy(_t593) ? px_eq(_v585, px_int(0LL)) : _t593; }))) {
            px_srcline(1075);
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t596 = ({ LXValue _t595 = ({ LXValue _t594 = px_eq(_v587, px_str(",")); px_is_truthy(_t594) ? _t594 : px_eq(_v587, px_str("换行")); }); px_is_truthy(_t595) ? _t595 : px_eq(_v587, px_str("EOF")); }); px_is_truthy(_t596) ? px_eq(_v585, px_int(0LL)) : _t596; }))) {
            px_srcline(1077);
            return px_bool(false);
        }
        px_srcline(1078);
         _v586 = px_add(_v586, px_int(1LL));
    }
    px_srcline(1079);
    return px_bool(false);
px_err_588:
    if (px_err_588_proped) return px_err_588_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_brace");
    LXValue _v597 = px_null();
    LXValue _v598 = px_null();
    LXValue _v599 = px_null();
    LXValue _v600 = px_null();
    LXValue _v601 = px_null();
    LXValue _v602 = px_null();
    LXValue _v603 = px_null();
    LXValue _v604 = px_null();
    LXValue _v605 = px_null();
    LXValue px_err_606_val = px_null();
    int px_err_606_proped = 0;
    px_srcline(1081);
    _v597 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1082);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1083);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1084);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1085);
    _v598 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    px_srcline(1086);
    if (px_is_truthy(_v598)) {
        px_srcline(1087);
        _v599 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1088);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1089);
        _v600 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1090);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1091);
            _v601 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            px_srcline(1092);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            px_srcline(1093);
            return px_list_n((LXValue[]){px_str("DictComp"), _v599, _v600, px_index(_v601, px_int(0LL)), px_index(_v601, px_int(1LL)), _v597}, 6);
        }
        px_srcline(1094);
        _v602 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v599, _v600}, 2)}, 1);
        px_srcline(1095);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1096);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1097);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1098);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            px_srcline(1099);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                px_srcline(1100);
                break;
            }
            px_srcline(1101);
            _v603 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1102);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(1103);
            _v604 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1104);
            (void)(px_method(_v602, "append", (LXValue[]){px_list_n((LXValue[]){_v603, _v604}, 2)}, 1));
            px_srcline(1105);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1106);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1107);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1108);
        return px_list_n((LXValue[]){px_str("Dict"), _v602, _v597}, 3);
    }
    px_srcline(1109);
    _v605 = px_list_n((LXValue[]){}, 0);
    px_srcline(1110);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1111);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1112);
    while (px_is_truthy(({ LXValue _t607 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t607) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t607; }))) {
        px_srcline(1113);
        (void)(px_method(_v605, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(1114);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1115);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    px_srcline(1116);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    px_srcline(1117);
    return px_list_n((LXValue[]){px_str("Block"), _v605, _v597}, 3);
px_err_606:
    if (px_err_606_proped) return px_err_606_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_closure");
    LXValue _v608 = px_null();
    LXValue _v609 = px_null();
    LXValue _v610 = px_null();
    LXValue _v611 = px_null();
    LXValue _v612 = px_null();
    LXValue _v613 = px_null();
    LXValue _v614 = px_null();
    LXValue px_err_615_val = px_null();
    int px_err_615_proped = 0;
    px_srcline(1119);
    _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1120);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1121);
    _v609 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(1122);
    _v610 = px_null();
    px_srcline(1123);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(1124);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1125);
         _v610 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(1126);
    _v611 = px_list_n((LXValue[]){}, 0);
    px_srcline(1127);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        px_srcline(1128);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1129);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1130);
            (void)(px_method(_v611, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            px_srcline(1131);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1132);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1133);
                continue;
            }
            px_srcline(1134);
            break;
        }
    }
    px_srcline(1135);
    _v612 = px_null();
    px_srcline(1136);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        px_srcline(1137);
        _v613 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1138);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1139);
        _v614 = px_list_n((LXValue[]){}, 0);
        px_srcline(1140);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1141);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1142);
        while (px_is_truthy(({ LXValue _t616 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t616) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t616; }))) {
            px_srcline(1143);
            (void)(px_method(_v614, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            px_srcline(1144);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1145);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1146);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1147);
         _v612 = px_list_n((LXValue[]){px_str("Block"), _v614, _v613}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(1149);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1150);
         _v612 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    else {
        px_srcline(1152);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    px_srcline(1153);
    return px_list_n((LXValue[]){px_str("Closure"), _v609, _v610, _v612, _v611, _v608}, 6);
px_err_615:
    if (px_err_615_proped) return px_err_615_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_match_expr");
    LXValue _v617 = px_null();
    LXValue _v618 = px_null();
    LXValue _v619 = px_null();
    LXValue _v620 = px_null();
    LXValue _v621 = px_null();
    LXValue _v622 = px_null();
    LXValue _v623 = px_null();
    LXValue _v624 = px_null();
    LXValue px_err_625_val = px_null();
    int px_err_625_proped = 0;
    px_srcline(1155);
    _v617 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1156);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1157);
    _v618 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1158);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1159);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(1160);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(1161);
    _v619 = px_list_n((LXValue[]){}, 0);
    px_srcline(1162);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1163);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(1164);
        if (px_is_truthy(({ LXValue _t626 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t626) ? _t626 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(1165);
            break;
        }
        px_srcline(1166);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(1167);
        _v620 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        px_srcline(1168);
        _v621 = px_null();
        px_srcline(1169);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1170);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1171);
             _v621 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1172);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1173);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(1174);
        _v622 = px_null();
        px_srcline(1175);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            px_srcline(1176);
            _v623 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(1177);
            _v624 = px_null();
            px_srcline(1178);
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v623}, 1), px_int(0LL)))) {
                px_srcline(1179);
                 _v624 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v623, px_int(0LL))}, 1);
            }
            else {
                px_srcline(1181);
                 _v624 = _v617;
            }
            px_srcline(1182);
             _v622 = px_list_n((LXValue[]){px_str("Block"), _v623, _v624}, 3);
        }
        else {
            px_srcline(1184);
             _v622 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1185);
        (void)(px_method(_v619, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v620, _v621, _v622, _v617}, 5)}, 1));
    }
    px_srcline(1186);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(1187);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    px_srcline(1188);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(1189);
    return px_list_n((LXValue[]){px_str("Match"), _v618, _v619, _v617}, 4);
px_err_625:
    if (px_err_625_proped) return px_err_625_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if_expr");
    LXValue _v627 = px_null();
    LXValue _v628 = px_null();
    LXValue _v629 = px_null();
    LXValue _v630 = px_null();
    LXValue px_err_631_val = px_null();
    int px_err_631_proped = 0;
    px_srcline(1191);
    _v627 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1192);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1193);
    _v628 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1194);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1195);
    _v629 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1196);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    px_srcline(1197);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1198);
    _v630 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1199);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v628, _v629, _v630, _v627}, 5);
px_err_631:
    if (px_err_631_proped) return px_err_631_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pattern");
    LXValue _v632 = px_null();
    LXValue _v633 = px_null();
    LXValue _v634 = px_null();
    LXValue _v635 = px_null();
    LXValue _v636 = px_null();
    LXValue _v637 = px_null();
    LXValue _v638 = px_null();
    LXValue _v639 = px_null();
    LXValue px_err_640_val = px_null();
    int px_err_640_proped = 0;
    px_srcline(1202);
    _v632 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1203);
    if (px_is_truthy(({ LXValue _t645 = ({ LXValue _t644 = ({ LXValue _t643 = ({ LXValue _t642 = ({ LXValue _t641 = px_eq(_v632, px_str("整数")); px_is_truthy(_t641) ? _t641 : px_eq(_v632, px_str("浮点")); }); px_is_truthy(_t642) ? _t642 : px_eq(_v632, px_str("字符串")); }); px_is_truthy(_t643) ? _t643 : px_eq(_v632, px_str("true")); }); px_is_truthy(_t644) ? _t644 : px_eq(_v632, px_str("false")); }); px_is_truthy(_t645) ? _t645 : px_eq(_v632, px_str("null")); }))) {
        px_srcline(1204);
        _v633 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        px_srcline(1205);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v633}, 2);
    }
    px_srcline(1206);
    if (px_is_truthy(px_eq(_v632, px_str("标识符")))) {
        px_srcline(1207);
        _v634 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1208);
        _v635 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1209);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1210);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(1212);
            _v636 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1213);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1214);
            _v637 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(1215);
            return px_list_n((LXValue[]){px_str("PatLiteral"), px_list_n((LXValue[]){px_str("Field"), px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v634}, 1), _v636}, 3), px_call(px_get_global("qstr"), (LXValue[]){_v637}, 1), _v636}, 4)}, 2);
        }
        px_srcline(1216);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1217);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1218);
            _v638 = px_list_n((LXValue[]){}, 0);
            px_srcline(1219);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(1220);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1221);
                    (void)(px_method(_v638, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    px_srcline(1222);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(1223);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(1224);
                        continue;
                    }
                    px_srcline(1225);
                    break;
                }
            }
            px_srcline(1226);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            px_srcline(1227);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v634}, 1), _v638}, 3);
        }
        px_srcline(1228);
        if (px_is_truthy(px_eq(_v634, px_str("_")))) {
            px_srcline(1229);
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        px_srcline(1230);
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v634}, 1))) {
            px_srcline(1231);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v634}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        px_srcline(1232);
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v634}, 1)}, 2);
    }
    px_srcline(1233);
    if (px_is_truthy(px_eq(_v632, px_str("(")))) {
        px_srcline(1234);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1235);
        _v639 = px_list_n((LXValue[]){}, 0);
        px_srcline(1236);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1237);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1238);
                (void)(px_method(_v639, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                px_srcline(1239);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1240);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1241);
                    continue;
                }
                px_srcline(1242);
                break;
            }
        }
        px_srcline(1243);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1244);
        return px_list_n((LXValue[]){px_str("PatTuple"), _v639}, 2);
    }
    px_srcline(1245);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1246);
    return px_null();
px_err_640:
    if (px_err_640_proped) return px_err_640_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_upper");
    LXValue _v646 = (nargs > 0) ? args[0] : px_null();
    LXValue _v647 = px_null();
    LXValue px_err_648_val = px_null();
    int px_err_648_proped = 0;
    px_srcline(1248);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v646}, 1), px_int(0LL)))) {
        px_srcline(1249);
        return px_bool(false);
    }
    px_srcline(1250);
    _v647 = px_index(_v646, px_int(0LL));
    px_srcline(1251);
    return ({ LXValue _t649 = px_ge(_v647, px_str("A")); px_is_truthy(_t649) ? px_le(_v647, px_str("Z")) : _t649; });
px_err_648:
    if (px_err_648_proped) return px_err_648_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type");
    LXValue _v650 = px_null();
    LXValue _v651 = px_null();
    LXValue px_err_652_val = px_null();
    int px_err_652_proped = 0;
    px_srcline(1254);
    _v650 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    px_srcline(1255);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        px_srcline(1256);
        _v651 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1257);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1258);
        return px_list_n((LXValue[]){px_str("TyOptional"), _v650, _v651}, 3);
    }
    px_srcline(1259);
    return _v650;
px_err_652:
    if (px_err_652_proped) return px_err_652_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_base");
    LXValue _v653 = px_null();
    LXValue _v654 = px_null();
    LXValue _v655 = px_null();
    LXValue _v656 = px_null();
    LXValue _v657 = px_null();
    LXValue _v658 = px_null();
    LXValue _v659 = px_null();
    LXValue _v660 = px_null();
    LXValue px_err_661_val = px_null();
    int px_err_661_proped = 0;
    px_srcline(1261);
    _v653 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1262);
    if (px_is_truthy(px_eq(_v653, px_str("标识符")))) {
        px_srcline(1263);
        _v654 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1264);
        _v655 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1265);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1266);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1267);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1268);
            _v656 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1269);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            px_srcline(1270);
            if (px_is_truthy(px_eq(_v654, px_str("list")))) {
                px_srcline(1271);
                return px_list_n((LXValue[]){px_str("TyList"), _v656, _v655}, 3);
            }
            px_srcline(1272);
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v654}, 1), px_list_n((LXValue[]){_v656}, 1), _v655}, 4);
        }
        px_srcline(1273);
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v654}, 1), _v655}, 3);
    }
    px_srcline(1274);
    if (px_is_truthy(px_eq(_v653, px_str("[")))) {
        px_srcline(1275);
        _v655 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1276);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1277);
        _v656 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1278);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1279);
        return px_list_n((LXValue[]){px_str("TyList"), _v656, _v655}, 3);
    }
    px_srcline(1280);
    if (px_is_truthy(px_eq(_v653, px_str("{")))) {
        px_srcline(1281);
        _v655 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1282);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1283);
        _v657 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1284);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1285);
        _v658 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1286);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1287);
        return px_list_n((LXValue[]){px_str("TyDict"), _v657, _v658, _v655}, 4);
    }
    px_srcline(1288);
    if (px_is_truthy(px_eq(_v653, px_str("(")))) {
        px_srcline(1289);
        _v655 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1290);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1291);
        _v659 = px_list_n((LXValue[]){}, 0);
        px_srcline(1292);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1293);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1294);
                (void)(px_method(_v659, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                px_srcline(1295);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1296);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1297);
                    continue;
                }
                px_srcline(1298);
                break;
            }
        }
        px_srcline(1299);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1300);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(1301);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1302);
            _v660 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1303);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v659, _v660, _v655}, 4);
        }
        px_srcline(1304);
        return px_list_n((LXValue[]){px_str("TyTuple"), _v659, _v655}, 3);
    }
    px_srcline(1305);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1306);
    return px_null();
px_err_661:
    if (px_err_661_proped) return px_err_661_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt_inner");
    LXValue _v662 = (nargs > 0) ? args[0] : px_null();
    LXValue _v663 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v695 = px_null();
    LXValue _v696 = px_null();
    LXValue _v697 = px_null();
    LXValue px_err_698_val = px_null();
    int px_err_698_proped = 0;
    px_srcline(9);
    LXValue _v699 = px_call(px_get_global("cg_pad"), (LXValue[]){_v663}, 1);
    px_srcline(10);
    _v664 = px_index(_v662, px_int(0LL));
    px_srcline(11);
    if (px_is_truthy(px_eq(_v664, px_str("VarDecl")))) {
        px_srcline(12);
        _v665 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v662, px_int(2LL))}, 1);
        px_srcline(14);
        if (px_is_truthy(({ LXValue _t700 = px_eq(px_index(_v662, px_int(1LL)), px_str("Let")); px_is_truthy(_t700) ? _t700 : px_eq(px_index(_v662, px_int(1LL)), px_str("Const")); }))) {
            px_srcline(15);
            (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v665}, 1));
        }
        px_srcline(18);
        _v666 = px_index(_v662, px_int(3LL));
        px_srcline(19);
        if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v666}, 1))) {
            px_srcline(20);
            px_index_set(px_get_global("cg_nonnull"), _v665, px_int(1LL));
            px_srcline(21);
            if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v662, px_int(4LL))}, 1))) {
                px_srcline(22);
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v666}, 1)), px_str("'（可空类型请用 ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v666}, 1)), px_str("? 声明）"))}, 2));
            }
        }
        px_srcline(23);
        _v667 = px_str("px_null()");
        px_srcline(24);
        if (px_is_truthy(px_ne(px_index(_v662, px_int(4LL)), px_null()))) {
            px_srcline(25);
             _v667 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(4LL))}, 1);
        }
        px_srcline(27);
        if (px_is_truthy(({ LXValue _t701 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v665}, 2); px_is_truthy(_t701) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t701; }))) {
            px_srcline(28);
            return px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_set_global(\"")), _v665), px_str("\", ")), _v667), px_str(");\n"));
        }
        px_srcline(29);
        _v668 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v665}, 1);
        px_srcline(30);
        if (px_is_truthy(px_eq(_v668, px_null()))) {
            px_srcline(32);
             _v668 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v665}, 1);
            px_srcline(34);
            if (px_is_truthy(px_ne(px_index(_v662, px_int(4LL)), px_null()))) {
                px_srcline(35);
                _v669 = px_index(_v662, px_int(4LL));
                px_srcline(36);
                _v670 = px_null();
                px_srcline(37);
                if (px_is_truthy(px_eq(px_index(_v669, px_int(0LL)), px_str("Constructor")))) {
                    px_srcline(38);
                     _v670 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v669, px_int(1LL))}, 1);
                }
                else if (px_is_truthy(px_eq(px_index(_v669, px_int(0LL)), px_str("Call")))) {
                    px_srcline(40);
                    _v671 = px_index(_v669, px_int(1LL));
                    px_srcline(41);
                    if (px_is_truthy(px_eq(px_index(_v671, px_int(0LL)), px_str("Var")))) {
                        px_srcline(42);
                         _v670 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v671, px_int(1LL))}, 1);
                    }
                }
                px_srcline(43);
                if (px_is_truthy(px_ne(_v670, px_null()))) {
                    px_srcline(44);
                    if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v670}, 1))) {
                        px_srcline(45);
                        px_index_set(px_get_global("cg_var_types"), _v665, _v670);
                    }
                }
            }
            px_srcline(46);
            return px_add(px_add(px_add(px_add(px_add(_v699, px_str("LXValue ")), _v668), px_str(" = ")), _v667), px_str(";\n"));
        }
        px_srcline(49);
        if (px_is_truthy(px_ne(px_index(_v662, px_int(4LL)), px_null()))) {
            px_srcline(50);
            _v669 = px_index(_v662, px_int(4LL));
            px_srcline(51);
            _v670 = px_null();
            px_srcline(52);
            if (px_is_truthy(px_eq(px_index(_v669, px_int(0LL)), px_str("Constructor")))) {
                px_srcline(53);
                 _v670 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v669, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v669, px_int(0LL)), px_str("Call")))) {
                px_srcline(55);
                _v671 = px_index(_v669, px_int(1LL));
                px_srcline(56);
                if (px_is_truthy(px_eq(px_index(_v671, px_int(0LL)), px_str("Var")))) {
                    px_srcline(57);
                     _v670 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v671, px_int(1LL))}, 1);
                }
            }
            px_srcline(58);
            if (px_is_truthy(px_ne(_v670, px_null()))) {
                px_srcline(59);
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v670}, 1))) {
                    px_srcline(60);
                    px_index_set(px_get_global("cg_var_types"), _v665, _v670);
                }
            }
        }
        px_srcline(61);
        return px_add(px_add(px_add(px_add(_v699, _v668), px_str(" = ")), _v667), px_str(";\n"));
    }
    px_srcline(62);
    if (px_is_truthy(px_eq(_v664, px_str("Assign")))) {
        px_srcline(63);
        _v667 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(3LL))}, 1);
        px_srcline(64);
        _v672 = px_index(_v662, px_int(1LL));
        px_srcline(65);
        _v673 = px_index(_v662, px_int(2LL));
        px_srcline(67);
        if (px_is_truthy(px_eq(_v673, px_str("Append")))) {
            px_srcline(68);
            _v674 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v672}, 1);
            px_srcline(69);
            return px_add(px_add(px_add(px_add(px_add(_v699, px_str("(void)(px_method(")), _v674), px_str(", \"append\", (LXValue[]){")), _v667), px_str("}, 1));\n"));
        }
        px_srcline(70);
        _v675 = px_index(_v672, px_int(0LL));
        px_srcline(71);
        if (px_is_truthy(px_eq(_v675, px_str("Var")))) {
            px_srcline(72);
            _v665 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v672, px_int(1LL))}, 1);
            px_srcline(74);
            if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v665}, 1))) {
                px_srcline(75);
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v665), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
            }
            px_srcline(77);
            if (px_is_truthy(({ LXValue _t702 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v662, px_int(3LL))}, 1); px_is_truthy(_t702) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v665}, 1) : _t702; }))) {
                px_srcline(78);
                (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v665), px_str("'（可空类型请声明为 ")), _v665), px_str(": T?）"))}, 2));
            }
            px_srcline(79);
            _v676 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v665}, 1);
            px_srcline(80);
            if (px_is_truthy(px_eq(_v676, px_null()))) {
                px_srcline(82);
                if (px_is_truthy(px_eq(_v673, px_str("Assign")))) {
                    px_srcline(83);
                    return px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_set_global(\"")), _v665), px_str("\", ")), _v667), px_str(");\n"));
                }
                px_srcline(84);
                _v677 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v673, _v665, _v667}, 3);
                px_srcline(85);
                return px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_set_global(\"")), _v665), px_str("\", ")), _v677), px_str(");\n"));
            }
            px_srcline(86);
            _v677 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v673, _v676, _v667}, 3);
            px_srcline(87);
            return px_add(px_add(px_add(px_add(px_add(_v699, px_str(" ")), _v676), px_str(" = ")), _v677), px_str(";\n"));
        }
        px_srcline(88);
        if (px_is_truthy(px_eq(_v675, px_str("Field")))) {
            px_srcline(89);
            _v674 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v672, px_int(1LL))}, 1);
            px_srcline(90);
            _v678 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v672, px_int(2LL))}, 1);
            px_srcline(91);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_field_set(")), _v674), px_str(", \"")), _v678), px_str("\", ")), _v667), px_str(");\n"));
        }
        px_srcline(92);
        if (px_is_truthy(px_eq(_v675, px_str("Index")))) {
            px_srcline(93);
            _v674 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v672, px_int(1LL))}, 1);
            px_srcline(94);
            _v679 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v672, px_int(2LL))}, 1);
            px_srcline(95);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_index_set(")), _v674), px_str(", ")), _v679), px_str(", ")), _v667), px_str(");\n"));
        }
        px_srcline(96);
        return px_str("不支持的赋值目标");
    }
    px_srcline(97);
    if (px_is_truthy(px_eq(_v664, px_str("ExprStmt")))) {
        px_srcline(98);
        _v669 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1);
        px_srcline(99);
        return px_add(px_add(px_add(_v699, px_str("(void)(")), _v669), px_str(");\n"));
    }
    px_srcline(100);
    if (px_is_truthy(px_eq(_v664, px_str("If")))) {
        px_srcline(101);
        _v680 = px_str("");
        px_srcline(102);
        _v681 = px_index(_v662, px_int(1LL));
        px_srcline(103);
        _v682 = px_int(0LL);
        px_srcline(104);
        while (px_is_truthy(px_lt(_v682, px_call(px_get_global("len"), (LXValue[]){_v681}, 1)))) {
            px_srcline(105);
            _v683 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v681, _v682), px_int(0LL))}, 1);
            px_srcline(106);
            _v684 = px_str("if");
            px_srcline(107);
            if (px_is_truthy(px_gt(_v682, px_int(0LL)))) {
                px_srcline(108);
                 _v684 = px_str("else if");
            }
            px_srcline(109);
             _v680 = px_add(_v680, px_add(px_add(px_add(px_add(_v699, _v684), px_str(" (px_is_truthy(")), _v683), px_str(")) {\n")));
            px_srcline(110);
            _v685 = px_index(px_index(_v681, _v682), px_int(1LL));
            px_srcline(111);
            _v686 = px_int(0LL);
            px_srcline(112);
            while (px_is_truthy(px_lt(_v686, px_call(px_get_global("len"), (LXValue[]){_v685}, 1)))) {
                px_srcline(113);
                 _v680 = px_add(_v680, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v685, _v686), px_add(_v663, px_int(1LL))}, 2));
                px_srcline(114);
                 _v686 = px_add(_v686, px_int(1LL));
            }
            px_srcline(115);
             _v680 = px_add(_v680, px_add(_v699, px_str("}\n")));
            px_srcline(116);
             _v682 = px_add(_v682, px_int(1LL));
        }
        px_srcline(117);
        if (px_is_truthy(px_ne(px_index(_v662, px_int(2LL)), px_null()))) {
            px_srcline(118);
             _v680 = px_add(_v680, px_add(_v699, px_str("else {\n")));
            px_srcline(119);
            _v687 = px_index(_v662, px_int(2LL));
            px_srcline(120);
            _v688 = px_int(0LL);
            px_srcline(121);
            while (px_is_truthy(px_lt(_v688, px_call(px_get_global("len"), (LXValue[]){_v687}, 1)))) {
                px_srcline(122);
                 _v680 = px_add(_v680, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v687, _v688), px_add(_v663, px_int(1LL))}, 2));
                px_srcline(123);
                 _v688 = px_add(_v688, px_int(1LL));
            }
            px_srcline(124);
             _v680 = px_add(_v680, px_add(_v699, px_str("}\n")));
        }
        px_srcline(125);
        return _v680;
    }
    px_srcline(126);
    if (px_is_truthy(px_eq(_v664, px_str("While")))) {
        px_srcline(127);
        _v683 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1);
        px_srcline(128);
        _v680 = px_add(px_add(px_add(_v699, px_str("while (px_is_truthy(")), _v683), px_str(")) {\n"));
        px_srcline(129);
        _v685 = px_index(_v662, px_int(2LL));
        px_srcline(130);
        _v682 = px_int(0LL);
        px_srcline(131);
        while (px_is_truthy(px_lt(_v682, px_call(px_get_global("len"), (LXValue[]){_v685}, 1)))) {
            px_srcline(132);
             _v680 = px_add(_v680, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v685, _v682), px_add(_v663, px_int(1LL))}, 2));
            px_srcline(133);
             _v682 = px_add(_v682, px_int(1LL));
        }
        px_srcline(134);
         _v680 = px_add(_v680, px_add(_v699, px_str("}\n")));
        px_srcline(135);
        return _v680;
    }
    px_srcline(136);
    if (px_is_truthy(px_eq(_v664, px_str("For")))) {
        px_srcline(137);
        _v689 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(2LL))}, 1);
        px_srcline(138);
        _v690 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(139);
        _v691 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(140);
        _v692 = px_call(px_get_global("cg_var_of"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1)}, 1);
        px_srcline(141);
        _v693 = px_str("LXValue ");
        px_srcline(142);
        if (px_is_truthy(({ LXValue _t703 = px_eq(_v692, px_null()); px_is_truthy(_t703) ? _t703 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)); }))) {
            px_srcline(145);
             _v692 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1)}, 1);
        }
        else {
            px_srcline(148);
             _v693 = px_str("");
        }
        px_srcline(149);
        _v680 = px_add(px_add(px_add(px_add(px_add(_v699, px_str("LXValue ")), _v690), px_str(" = ")), _v689), px_str(";\n"));
        px_srcline(150);
         _v680 = px_add(_v680, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v699, px_str("for (int ")), _v691), px_str(" = 0; ")), _v691), px_str(" < px_len(")), _v690), px_str("); ")), _v691), px_str("++) {\n")));
        px_srcline(151);
         _v680 = px_add(_v680, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v699, px_str("    ")), _v693), _v692), px_str(" = px_index(")), _v690), px_str(", px_int(")), _v691), px_str("));\n")));
        px_srcline(152);
        _v685 = px_index(_v662, px_int(3LL));
        px_srcline(153);
        _v682 = px_int(0LL);
        px_srcline(154);
        while (px_is_truthy(px_lt(_v682, px_call(px_get_global("len"), (LXValue[]){_v685}, 1)))) {
            px_srcline(155);
             _v680 = px_add(_v680, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v685, _v682), px_add(_v663, px_int(1LL))}, 2));
            px_srcline(156);
             _v682 = px_add(_v682, px_int(1LL));
        }
        px_srcline(157);
         _v680 = px_add(_v680, px_add(_v699, px_str("}\n")));
        px_srcline(158);
        return _v680;
    }
    px_srcline(159);
    if (px_is_truthy(px_eq(_v664, px_str("Return")))) {
        px_srcline(160);
        if (px_is_truthy(px_ne(px_index(_v662, px_int(1LL)), px_null()))) {
            px_srcline(161);
            _v669 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1);
            px_srcline(162);
            return px_add(px_add(px_add(_v699, px_str("return ")), _v669), px_str(";\n"));
        }
        px_srcline(163);
        return px_add(_v699, px_str("return px_null();\n"));
    }
    px_srcline(164);
    if (px_is_truthy(px_eq(_v664, px_str("Break")))) {
        px_srcline(165);
        return px_add(_v699, px_str("break;\n"));
    }
    px_srcline(166);
    if (px_is_truthy(px_eq(_v664, px_str("Continue")))) {
        px_srcline(167);
        return px_add(_v699, px_str("continue;\n"));
    }
    px_srcline(168);
    if (px_is_truthy(px_eq(_v664, px_str("Empty")))) {
        px_srcline(169);
        return px_str("");
    }
    px_srcline(170);
    if (px_is_truthy(px_eq(_v664, px_str("ChanDecl")))) {
        px_srcline(171);
        _v676 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1)}, 1);
        px_srcline(172);
        return px_add(px_add(px_add(_v699, px_str("LXValue ")), _v676), px_str(" = px_chan_create(0);\n"));
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(_v664, px_str("Send")))) {
        px_srcline(174);
        _v683 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1);
        px_srcline(175);
        _v676 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(2LL))}, 1);
        px_srcline(176);
        return px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_chan_send(")), _v683), px_str(", ")), _v676), px_str(");\n"));
    }
    px_srcline(177);
    if (px_is_truthy(px_eq(_v664, px_str("Recv")))) {
        px_srcline(178);
        _v683 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v662, px_int(1LL))}, 1);
        px_srcline(179);
        return px_add(px_add(px_add(_v699, px_str("px_chan_recv(")), _v683), px_str(");\n"));
    }
    px_srcline(180);
    if (px_is_truthy(px_eq(_v664, px_str("Spawn")))) {
        px_srcline(181);
        _v694 = px_index(_v662, px_int(1LL));
        px_srcline(182);
        if (px_is_truthy(px_eq(px_index(_v694, px_int(0LL)), px_str("Call")))) {
            px_srcline(183);
            _v671 = px_index(_v694, px_int(1LL));
            px_srcline(184);
            if (px_is_truthy(px_eq(px_index(_v671, px_int(0LL)), px_str("Var")))) {
                px_srcline(185);
                _v678 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v671, px_int(1LL))}, 1);
                px_srcline(186);
                _v695 = px_list_n((LXValue[]){}, 0);
                px_srcline(187);
                _v696 = px_index(_v694, px_int(2LL));
                px_srcline(188);
                _v697 = px_int(0LL);
                px_srcline(189);
                while (px_is_truthy(px_lt(_v697, px_call(px_get_global("len"), (LXValue[]){_v696}, 1)))) {
                    px_srcline(190);
                    (void)(px_method(_v695, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v696, _v697)}, 1)}, 1));
                    px_srcline(191);
                     _v697 = px_add(_v697, px_int(1LL));
                }
                px_srcline(192);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v699, px_str("px_spawn_name(\"")), _v678), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v695}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v695}, 1)}, 1)), px_str(");\n"));
            }
            px_srcline(193);
            return px_str("编译模式 spawn 仅支持直接函数调用（方法调用请用 `lx run`）");
        }
        px_srcline(194);
        return px_str("编译模式 spawn 仅支持函数调用表达式");
    }
    px_srcline(195);
    if (px_is_truthy(px_eq(_v664, px_str("Select")))) {
        px_srcline(196);
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v662, px_int(1LL)), px_index(_v662, px_int(2LL)), _v663}, 3);
    }
    px_srcline(197);
    if (px_is_truthy(px_eq(_v664, px_str("Import")))) {
        px_srcline(198);
        return px_add(_v699, px_str("/* import 忽略（MVP） */\n"));
    }
    px_srcline(200);
    return px_str("");
px_err_698:
    if (px_err_698_proped) return px_err_698_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt");
    LXValue _v704 = (nargs > 0) ? args[0] : px_null();
    LXValue _v705 = (nargs > 1) ? args[1] : px_null();
    LXValue _v706 = px_null();
    LXValue _v707 = px_null();
    LXValue _v708 = px_null();
    LXValue px_err_709_val = px_null();
    int px_err_709_proped = 0;
    px_srcline(206);
    _v706 = px_int(0LL);
    px_srcline(207);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v704}, 1), px_int(0LL)))) {
        px_srcline(208);
        _v707 = px_index(_v704, px_sub(px_call(px_get_global("len"), (LXValue[]){_v704}, 1), px_int(1LL)));
        px_srcline(209);
        if (px_is_truthy(({ LXValue _t711 = ({ LXValue _t710 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v707}, 1), px_str("list")); px_is_truthy(_t710) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v707}, 1), px_int(1LL)) : _t710; }); px_is_truthy(_t711) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v707, px_int(0LL))}, 1), px_str("int")) : _t711; }))) {
            px_srcline(210);
             _v706 = px_index(_v707, px_int(0LL));
        }
    }
    px_srcline(211);
    _v708 = px_str("");
    px_srcline(212);
    if (px_is_truthy(px_gt(_v706, px_int(0LL)))) {
        px_srcline(213);
         _v708 = px_add(px_add(px_add(px_call(px_get_global("cg_pad"), (LXValue[]){_v705}, 1), px_str("px_srcline(")), px_call(px_get_global("str"), (LXValue[]){_v706}, 1)), px_str(");\n"));
    }
    px_srcline(214);
    return px_add(_v708, px_call(px_get_global("cg_gen_stmt_inner"), (LXValue[]){_v704, _v705}, 2));
px_err_709:
    if (px_err_709_proped) return px_err_709_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_global");
    LXValue _v712 = (nargs > 0) ? args[0] : px_null();
    LXValue _v713 = (nargs > 1) ? args[1] : px_null();
    LXValue _v714 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_715_val = px_null();
    int px_err_715_proped = 0;
    px_srcline(217);
    if (px_is_truthy(px_eq(_v712, px_str("Assign")))) {
        px_srcline(218);
        return _v714;
    }
    px_srcline(219);
    if (px_is_truthy(px_eq(_v712, px_str("Plus")))) {
        px_srcline(220);
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(221);
    if (px_is_truthy(px_eq(_v712, px_str("Minus")))) {
        px_srcline(222);
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(223);
    if (px_is_truthy(px_eq(_v712, px_str("Star")))) {
        px_srcline(224);
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(225);
    if (px_is_truthy(px_eq(_v712, px_str("Slash")))) {
        px_srcline(226);
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(227);
    if (px_is_truthy(px_eq(_v712, px_str("IntDiv")))) {
        px_srcline(228);
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(229);
    if (px_is_truthy(px_eq(_v712, px_str("Mod")))) {
        px_srcline(230);
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(231);
    if (px_is_truthy(px_eq(_v712, px_str("Pow")))) {
        px_srcline(232);
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(233);
    if (px_is_truthy(px_eq(_v712, px_str("BitAnd")))) {
        px_srcline(234);
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(235);
    if (px_is_truthy(px_eq(_v712, px_str("BitOr")))) {
        px_srcline(236);
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(237);
    if (px_is_truthy(px_eq(_v712, px_str("BitXor")))) {
        px_srcline(238);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(239);
    if (px_is_truthy(px_eq(_v712, px_str("Shl")))) {
        px_srcline(240);
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(_v712, px_str("Shr")))) {
        px_srcline(242);
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(243);
    if (px_is_truthy(px_eq(_v712, px_str("ShrU")))) {
        px_srcline(244);
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v713), px_str("\"), ")), _v714), px_str(")"));
    }
    px_srcline(245);
    return _v714;
px_err_715:
    if (px_err_715_proped) return px_err_715_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_local");
    LXValue _v716 = (nargs > 0) ? args[0] : px_null();
    LXValue _v717 = (nargs > 1) ? args[1] : px_null();
    LXValue _v718 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_719_val = px_null();
    int px_err_719_proped = 0;
    px_srcline(248);
    if (px_is_truthy(px_eq(_v716, px_str("Assign")))) {
        px_srcline(249);
        return _v718;
    }
    px_srcline(250);
    if (px_is_truthy(px_eq(_v716, px_str("Plus")))) {
        px_srcline(251);
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(252);
    if (px_is_truthy(px_eq(_v716, px_str("Minus")))) {
        px_srcline(253);
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(254);
    if (px_is_truthy(px_eq(_v716, px_str("Star")))) {
        px_srcline(255);
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(256);
    if (px_is_truthy(px_eq(_v716, px_str("Slash")))) {
        px_srcline(257);
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(258);
    if (px_is_truthy(px_eq(_v716, px_str("IntDiv")))) {
        px_srcline(259);
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(260);
    if (px_is_truthy(px_eq(_v716, px_str("Mod")))) {
        px_srcline(261);
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(262);
    if (px_is_truthy(px_eq(_v716, px_str("Pow")))) {
        px_srcline(263);
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(264);
    if (px_is_truthy(px_eq(_v716, px_str("BitAnd")))) {
        px_srcline(265);
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(266);
    if (px_is_truthy(px_eq(_v716, px_str("BitOr")))) {
        px_srcline(267);
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(268);
    if (px_is_truthy(px_eq(_v716, px_str("BitXor")))) {
        px_srcline(269);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(270);
    if (px_is_truthy(px_eq(_v716, px_str("Shl")))) {
        px_srcline(271);
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(272);
    if (px_is_truthy(px_eq(_v716, px_str("Shr")))) {
        px_srcline(273);
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(274);
    if (px_is_truthy(px_eq(_v716, px_str("ShrU")))) {
        px_srcline(275);
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v717), px_str(", ")), _v718), px_str(")"));
    }
    px_srcline(276);
    return _v718;
px_err_719:
    if (px_err_719_proped) return px_err_719_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_select");
    LXValue _v720 = (nargs > 0) ? args[0] : px_null();
    LXValue _v721 = (nargs > 1) ? args[1] : px_null();
    LXValue _v722 = (nargs > 2) ? args[2] : px_null();
    LXValue _v723 = px_null();
    LXValue _v724 = px_null();
    LXValue _v725 = px_null();
    LXValue _v726 = px_null();
    LXValue _v727 = px_null();
    LXValue _v728 = px_null();
    LXValue _v729 = px_null();
    LXValue _v730 = px_null();
    LXValue _v731 = px_null();
    LXValue _v732 = px_null();
    LXValue _v733 = px_null();
    LXValue _v734 = px_null();
    LXValue _v735 = px_null();
    LXValue _v736 = px_null();
    LXValue _v737 = px_null();
    LXValue _v738 = px_null();
    LXValue _v739 = px_null();
    LXValue _v740 = px_null();
    LXValue _v741 = px_null();
    LXValue _v742 = px_null();
    LXValue px_err_743_val = px_null();
    int px_err_743_proped = 0;
    px_srcline(279);
    LXValue _v744 = px_call(px_get_global("cg_pad"), (LXValue[]){_v722}, 1);
    px_srcline(280);
    _v723 = px_call(px_get_global("len"), (LXValue[]){_v720}, 1);
    px_srcline(281);
    if (px_is_truthy(px_eq(_v723, px_int(0LL)))) {
        px_srcline(282);
        return px_str("select 至少需要一个 case 分支");
    }
    px_srcline(283);
    _v724 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    px_srcline(284);
    _v725 = px_str("");
    px_srcline(285);
    _v726 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(286);
    _v727 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(288);
    _v728 = px_list_n((LXValue[]){}, 0);
    px_srcline(289);
    _v729 = px_int(0LL);
    px_srcline(290);
    while (px_is_truthy(px_lt(_v729, _v723))) {
        px_srcline(291);
        _v730 = px_index(px_index(_v720, _v729), px_int(1LL));
        px_srcline(292);
        if (px_is_truthy(px_eq(px_index(_v730, px_int(0LL)), px_str("Call")))) {
            px_srcline(293);
            _v731 = px_index(_v730, px_int(1LL));
            px_srcline(294);
            if (px_is_truthy(px_eq(px_index(_v731, px_int(0LL)), px_str("Field")))) {
                px_srcline(295);
                _v732 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v731, px_int(2LL))}, 1);
                px_srcline(296);
                if (px_is_truthy(px_eq(_v732, px_str("recv")))) {
                    px_srcline(297);
                    (void)(px_method(_v728, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v731, px_int(1LL))}, 1)}, 1));
                    px_srcline(298);
                     _v729 = px_add(_v729, px_int(1LL));
                    px_srcline(299);
                    continue;
                }
                px_srcline(300);
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v732), px_str("）"));
            }
            px_srcline(301);
            return px_str("select case 仅支持 ch.recv()");
        }
        px_srcline(302);
        return px_str("select case 仅支持 ch.recv()");
    }
    px_srcline(303);
     _v725 = px_add(_v725, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v744, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v723}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v728}, 2)), px_str("};\n")));
    px_srcline(304);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(": {\n")));
    px_srcline(305);
    _v733 = px_list_n((LXValue[]){}, 0);
    px_srcline(306);
    _v734 = px_int(0LL);
    px_srcline(307);
    while (px_is_truthy(px_lt(_v734, _v723))) {
        px_srcline(308);
        (void)(px_method(_v733, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v734}, 1)}, 1));
        px_srcline(309);
         _v734 = px_add(_v734, px_int(1LL));
    }
    px_srcline(310);
     _v725 = px_add(_v725, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v744, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v723}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v733}, 2)), px_str("};\n")));
    px_srcline(311);
    if (px_is_truthy(px_gt(_v723, px_int(1LL)))) {
        px_srcline(312);
        _v735 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v744, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v723}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("--) { "));
        px_srcline(313);
         _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" + 1); ")));
        px_srcline(314);
         _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("; ")));
        px_srcline(315);
         _v735 = px_add(_v735, px_str("}\n"));
        px_srcline(316);
         _v725 = px_add(_v725, _v735);
    }
    px_srcline(317);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = px_null();\n")));
    px_srcline(318);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = -1;\n")));
    px_srcline(319);
     _v725 = px_add(_v725, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v744, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v723}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("++) {\n")));
    px_srcline(320);
     _v725 = px_add(_v725, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v744, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("];\n")));
    px_srcline(321);
     _v725 = px_add(_v725, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v744, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str("; break; }\n")));
    px_srcline(322);
     _v725 = px_add(_v725, px_add(_v744, px_str("    }\n")));
    px_srcline(324);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" >= 0) {\n")));
    px_srcline(325);
    _v736 = px_int(0LL);
    px_srcline(326);
    while (px_is_truthy(px_lt(_v736, _v723))) {
        px_srcline(327);
        _v737 = px_index(px_index(_v720, _v736), px_int(0LL));
        px_srcline(328);
        _v738 = px_index(px_index(_v720, _v736), px_int(2LL));
        px_srcline(329);
        _v739 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v736}, 1)), px_str(")"));
        px_srcline(330);
        if (px_is_truthy(px_gt(_v736, px_int(0LL)))) {
            px_srcline(331);
             _v739 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v736}, 1)), px_str(")"));
        }
        px_srcline(332);
         _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("        ")), _v739), px_str(" {\n")));
        px_srcline(333);
        if (px_is_truthy(px_ne(_v737, px_null()))) {
            px_srcline(334);
            _v740 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v737}, 1)}, 1);
            px_srcline(335);
             _v725 = px_add(_v725, px_add(px_add(px_add(px_add(px_add(_v744, px_str("            LXValue ")), _v740), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(";\n")));
        }
        px_srcline(336);
        _v741 = px_int(0LL);
        px_srcline(337);
        while (px_is_truthy(px_lt(_v741, px_call(px_get_global("len"), (LXValue[]){_v738}, 1)))) {
            px_srcline(338);
             _v725 = px_add(_v725, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v738, _v741), px_add(_v722, px_int(3LL))}, 2));
            px_srcline(339);
             _v741 = px_add(_v741, px_int(1LL));
        }
        px_srcline(340);
         _v725 = px_add(_v725, px_add(_v744, px_str("        }\n")));
        px_srcline(341);
         _v736 = px_add(_v736, px_int(1LL));
    }
    px_srcline(342);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(";\n")));
    px_srcline(343);
     _v725 = px_add(_v725, px_add(_v744, px_str("    }\n")));
    px_srcline(345);
    if (px_is_truthy(px_ne(_v721, px_null()))) {
        px_srcline(346);
         _v725 = px_add(_v725, px_add(_v744, px_str("    {\n")));
        px_srcline(347);
        _v742 = px_int(0LL);
        px_srcline(348);
        while (px_is_truthy(px_lt(_v742, px_call(px_get_global("len"), (LXValue[]){_v721}, 1)))) {
            px_srcline(349);
             _v725 = px_add(_v725, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v721, _v742), px_add(_v722, px_int(2LL))}, 2));
            px_srcline(350);
             _v742 = px_add(_v742, px_int(1LL));
        }
        px_srcline(351);
         _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(";\n")));
        px_srcline(352);
         _v725 = px_add(_v725, px_add(_v744, px_str("    }\n")));
    }
    px_srcline(354);
     _v725 = px_add(_v725, px_add(_v744, px_str("    px_select_wait();\n")));
    px_srcline(355);
     _v725 = px_add(_v725, px_add(_v744, px_str("}\n")));
    px_srcline(356);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(";\n")));
    px_srcline(357);
     _v725 = px_add(_v725, px_add(px_add(px_add(_v744, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v724}, 1)), px_str(": ;\n")));
    px_srcline(358);
    px_set_global("cg_vars", _v726);
    px_srcline(359);
    px_set_global("cg_var_types", _v727);
    px_srcline(360);
    return _v725;
px_err_743:
    if (px_err_743_proped) return px_err_743_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_collect");
    LXValue _v745 = (nargs > 0) ? args[0] : px_null();
    LXValue _v746 = px_null();
    LXValue _v747 = px_null();
    LXValue _v748 = px_null();
    LXValue _v749 = px_null();
    LXValue _v750 = px_null();
    LXValue _v751 = px_null();
    LXValue _v752 = px_null();
    LXValue _v753 = px_null();
    LXValue _v754 = px_null();
    LXValue _v755 = px_null();
    LXValue px_err_756_val = px_null();
    int px_err_756_proped = 0;
    px_srcline(9);
    _v746 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(10);
    (void)(px_method(_v746, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(11);
    px_index_set(_v746, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_srcline(12);
    px_index_set(_v746, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_srcline(13);
    px_index_set(_v746, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_srcline(14);
    px_index_set(_v746, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_srcline(15);
    px_index_set(_v746, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_srcline(16);
    px_index_set(_v746, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    px_srcline(17);
    _v747 = px_int(0LL);
    px_srcline(18);
    while (px_is_truthy(px_lt(_v747, px_call(px_get_global("len"), (LXValue[]){_v745}, 1)))) {
        px_srcline(19);
        _v748 = px_index(_v745, _v747);
        px_srcline(20);
        _v749 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v748, px_int(2LL))}, 1);
        px_srcline(21);
        (void)(px_method(px_index(_v746, px_str("its")), "append", (LXValue[]){_v749}, 1));
        px_srcline(22);
        (void)(px_method(px_index(_v746, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(23);
        (void)(px_method(px_index(_v746, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(24);
        (void)(px_method(px_index(_v746, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(25);
        _v750 = px_str("");
        px_srcline(26);
        _v751 = px_list_n((LXValue[]){}, 0);
        px_srcline(27);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v748, px_int(1LL))}, 1), px_int(1LL)))) {
            px_srcline(28);
            _v752 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            px_srcline(29);
            _v753 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v748, px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(30);
            _v754 = px_null();
            px_srcline(31);
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v753}, 1))) {
                px_srcline(32);
                 _v754 = px_index(px_get_global("cg_vars"), _v753);
            }
            px_srcline(33);
            px_index_set(px_get_global("cg_vars"), _v753, _v752);
            px_srcline(34);
            (void)(px_method(_v751, "append", (LXValue[]){px_list_n((LXValue[]){_v753, _v754}, 2)}, 1));
            px_srcline(35);
             _v750 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v752), px_str(" = ")), px_index(px_index(_v746, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v746, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            px_srcline(37);
            _v755 = px_int(0LL);
            px_srcline(38);
            while (px_is_truthy(px_lt(_v755, px_call(px_get_global("len"), (LXValue[]){px_index(_v748, px_int(1LL))}, 1)))) {
                px_srcline(39);
                _v753 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v748, px_int(1LL)), _v755)}, 1);
                px_srcline(40);
                _v752 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v755}, 1));
                px_srcline(41);
                _v754 = px_null();
                px_srcline(42);
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v753}, 1))) {
                    px_srcline(43);
                     _v754 = px_index(px_get_global("cg_vars"), _v753);
                }
                px_srcline(44);
                px_index_set(px_get_global("cg_vars"), _v753, _v752);
                px_srcline(45);
                (void)(px_method(_v751, "append", (LXValue[]){px_list_n((LXValue[]){_v753, _v754}, 2)}, 1));
                px_srcline(46);
                 _v750 = px_add(_v750, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v752), px_str(" = px_index(")), px_index(px_index(_v746, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v746, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v755}, 1)), px_str(")); ")));
                px_srcline(47);
                 _v755 = px_add(_v755, px_int(1LL));
            }
        }
        px_srcline(48);
        (void)(px_method(px_index(_v746, px_str("binds")), "append", (LXValue[]){_v750}, 1));
        px_srcline(49);
        (void)(px_method(px_index(_v746, px_str("saved_all")), "append", (LXValue[]){_v751}, 1));
        px_srcline(50);
         _v747 = px_add(_v747, px_int(1LL));
    }
    px_srcline(51);
    return _v746;
px_err_756:
    if (px_err_756_proped) return px_err_756_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_restore");
    LXValue _v757 = (nargs > 0) ? args[0] : px_null();
    LXValue _v758 = px_null();
    LXValue _v759 = px_null();
    LXValue _v760 = px_null();
    LXValue _v761 = px_null();
    LXValue _v762 = px_null();
    LXValue px_err_763_val = px_null();
    int px_err_763_proped = 0;
    px_srcline(53);
    _v758 = px_int(0LL);
    px_srcline(54);
    while (px_is_truthy(px_lt(_v758, px_call(px_get_global("len"), (LXValue[]){_v757}, 1)))) {
        px_srcline(55);
        _v759 = px_index(_v757, _v758);
        px_srcline(56);
        _v760 = px_int(0LL);
        px_srcline(57);
        while (px_is_truthy(px_lt(_v760, px_call(px_get_global("len"), (LXValue[]){_v759}, 1)))) {
            px_srcline(58);
            _v761 = px_index(px_index(_v759, _v760), px_int(0LL));
            px_srcline(59);
            _v762 = px_index(px_index(_v759, _v760), px_int(1LL));
            px_srcline(60);
            if (px_is_truthy(px_eq(_v762, px_null()))) {
                px_srcline(61);
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v761}, 1));
            }
            else {
                px_srcline(63);
                px_index_set(px_get_global("cg_vars"), _v761, _v762);
            }
            px_srcline(64);
             _v760 = px_add(_v760, px_int(1LL));
        }
        px_srcline(65);
         _v758 = px_add(_v758, px_int(1LL));
    }
px_err_763:
    if (px_err_763_proped) return px_err_763_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_body");
    LXValue _v764 = (nargs > 0) ? args[0] : px_null();
    LXValue _v765 = (nargs > 1) ? args[1] : px_null();
    LXValue _v766 = (nargs > 2) ? args[2] : px_null();
    LXValue _v767 = px_null();
    LXValue _v768 = px_null();
    LXValue _v769 = px_null();
    LXValue _v770 = px_null();
    LXValue px_err_771_val = px_null();
    int px_err_771_proped = 0;
    px_srcline(68);
    _v767 = px_str("");
    px_srcline(69);
    if (px_is_truthy(px_ne(_v765, px_null()))) {
        px_srcline(70);
         _v767 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v765), px_str(")) { ")), _v766), px_str("} "));
    }
    else {
        px_srcline(72);
         _v767 = _v766;
    }
    px_srcline(73);
    _v768 = px_call(px_get_global("len"), (LXValue[]){px_index(_v764, px_str("its"))}, 1);
    px_srcline(74);
    _v769 = px_sub(_v768, px_int(1LL));
    px_srcline(75);
    while (px_is_truthy(px_ge(_v769, px_int(0LL)))) {
        px_srcline(76);
        _v770 = px_str("");
        px_srcline(77);
        if (px_is_truthy(px_lt(px_add(_v769, px_int(1LL)), _v768))) {
            px_srcline(78);
             _v770 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v764, px_str("ivs")), px_add(_v769, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v764, px_str("its")), px_add(_v769, px_int(1LL)))), px_str("; "));
        }
        px_srcline(79);
         _v767 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v764, px_str("idxs")), _v769)), px_str("=0; ")), px_index(px_index(_v764, px_str("idxs")), _v769)), px_str("<px_len(")), px_index(px_index(_v764, px_str("ivs")), _v769)), px_str("); ")), px_index(px_index(_v764, px_str("idxs")), _v769)), px_str("++) { LXValue ")), px_index(px_index(_v764, px_str("itms")), _v769)), px_str(" = px_index(")), px_index(px_index(_v764, px_str("ivs")), _v769)), px_str(", px_int(")), px_index(px_index(_v764, px_str("idxs")), _v769)), px_str(")); ")), px_index(px_index(_v764, px_str("binds")), _v769)), _v770), _v767), px_str(" } "));
        px_srcline(80);
         _v769 = px_sub(_v769, px_int(1LL));
    }
    px_srcline(81);
    return _v767;
px_err_771:
    if (px_err_771_proped) return px_err_771_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_expr");
    LXValue _v772 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v826 = px_null();
    LXValue _v827 = px_null();
    LXValue _v828 = px_null();
    LXValue _v829 = px_null();
    LXValue _v830 = px_null();
    LXValue _v831 = px_null();
    LXValue _v832 = px_null();
    LXValue _v833 = px_null();
    LXValue _v834 = px_null();
    LXValue _v835 = px_null();
    LXValue _v836 = px_null();
    LXValue px_err_837_val = px_null();
    int px_err_837_proped = 0;
    px_srcline(84);
    _v773 = px_index(_v772, px_int(0LL));
    px_srcline(85);
    if (px_is_truthy(px_eq(_v773, px_str("Int")))) {
        px_srcline(86);
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1)), px_str("LL)"));
    }
    px_srcline(87);
    if (px_is_truthy(px_eq(_v773, px_str("Float")))) {
        px_srcline(88);
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1)), px_str(")"));
    }
    px_srcline(89);
    if (px_is_truthy(px_eq(_v773, px_str("Str")))) {
        px_srcline(90);
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1)}, 1)), px_str("\")"));
    }
    px_srcline(91);
    if (px_is_truthy(px_eq(_v773, px_str("Bool")))) {
        px_srcline(92);
        if (px_is_truthy(px_index(_v772, px_int(1LL)))) {
            px_srcline(93);
            return px_str("px_bool(true)");
        }
        px_srcline(94);
        return px_str("px_bool(false)");
    }
    px_srcline(95);
    if (px_is_truthy(px_eq(_v773, px_str("Null")))) {
        px_srcline(96);
        return px_str("px_null()");
    }
    px_srcline(97);
    if (px_is_truthy(px_eq(_v773, px_str("List")))) {
        px_srcline(98);
        _v774 = px_list_n((LXValue[]){}, 0);
        px_srcline(99);
        _v775 = px_index(_v772, px_int(1LL));
        px_srcline(100);
        _v776 = px_int(0LL);
        px_srcline(101);
        while (px_is_truthy(px_lt(_v776, px_call(px_get_global("len"), (LXValue[]){_v775}, 1)))) {
            px_srcline(102);
            (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v775, _v776)}, 1)}, 1));
            px_srcline(103);
             _v776 = px_add(_v776, px_int(1LL));
        }
        px_srcline(104);
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")"));
    }
    px_srcline(105);
    if (px_is_truthy(px_eq(_v773, px_str("Tuple")))) {
        px_srcline(106);
        _v774 = px_list_n((LXValue[]){}, 0);
        px_srcline(107);
        _v775 = px_index(_v772, px_int(1LL));
        px_srcline(108);
        _v776 = px_int(0LL);
        px_srcline(109);
        while (px_is_truthy(px_lt(_v776, px_call(px_get_global("len"), (LXValue[]){_v775}, 1)))) {
            px_srcline(110);
            (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v775, _v776)}, 1)}, 1));
            px_srcline(111);
             _v776 = px_add(_v776, px_int(1LL));
        }
        px_srcline(112);
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")"));
    }
    px_srcline(113);
    if (px_is_truthy(px_eq(_v773, px_str("Dict")))) {
        px_srcline(114);
        _v777 = px_str("({ LXValue _d = px_dict(); ");
        px_srcline(115);
        _v778 = px_index(_v772, px_int(1LL));
        px_srcline(116);
        _v776 = px_int(0LL);
        px_srcline(117);
        while (px_is_truthy(px_lt(_v776, px_call(px_get_global("len"), (LXValue[]){_v778}, 1)))) {
            px_srcline(118);
            _v779 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v778, _v776), px_int(0LL))}, 1);
            px_srcline(119);
            _v780 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v778, _v776), px_int(1LL))}, 1);
            px_srcline(120);
             _v777 = px_add(_v777, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v779), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v780), px_str("); } ")));
            px_srcline(121);
             _v776 = px_add(_v776, px_int(1LL));
        }
        px_srcline(122);
         _v777 = px_add(_v777, px_str("_d; })"));
        px_srcline(123);
        return _v777;
    }
    px_srcline(124);
    if (px_is_truthy(px_eq(_v773, px_str("Var")))) {
        px_srcline(125);
        _v781 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(126);
        _v782 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v781}, 1);
        px_srcline(127);
        if (px_is_truthy(px_ne(_v782, px_null()))) {
            px_srcline(128);
            return _v782;
        }
        px_srcline(129);
        return px_add(px_add(px_str("px_get_global(\""), _v781), px_str("\")"));
    }
    px_srcline(130);
    if (px_is_truthy(px_eq(_v773, px_str("Field")))) {
        px_srcline(131);
        _v783 = px_index(_v772, px_int(1LL));
        px_srcline(132);
        _v784 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(134);
        if (px_is_truthy(px_eq(px_index(_v783, px_int(0LL)), px_str("Var")))) {
            px_srcline(135);
            _v785 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v783, px_int(1LL))}, 1);
            px_srcline(136);
            if (px_is_truthy(({ LXValue _t838 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v785}, 1); px_is_truthy(_t838) ? px_method(px_index(px_get_global("cg_const_enums"), _v785), "has", (LXValue[]){_v784}, 1) : _t838; }))) {
                px_srcline(137);
                return px_index(px_index(px_get_global("cg_const_enums"), _v785), _v784);
            }
        }
        px_srcline(139);
        if (px_is_truthy(px_eq(px_index(_v783, px_int(0LL)), px_str("Var")))) {
            px_srcline(140);
            _v785 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v783, px_int(1LL))}, 1);
            px_srcline(141);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v785}, 1))) {
                px_srcline(142);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v785), px_str("\", \"")), _v784), px_str("\")"));
            }
        }
        px_srcline(143);
        _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v783}, 1);
        px_srcline(144);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v786), px_str(", \"")), _v784), px_str("\")"));
    }
    px_srcline(145);
    if (px_is_truthy(px_eq(_v773, px_str("OptionalField")))) {
        px_srcline(146);
        _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(147);
        _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(148);
        _v784 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(149);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v786), px_str("; px_is_null(")), _v787), px_str(") ? px_null() : px_field(")), _v786), px_str(", \"")), _v784), px_str("\"); })"));
    }
    px_srcline(150);
    if (px_is_truthy(px_eq(_v773, px_str("Index")))) {
        px_srcline(151);
        _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(152);
        _v776 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(153);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v786), px_str(", ")), _v776), px_str(")"));
    }
    px_srcline(154);
    if (px_is_truthy(px_eq(_v773, px_str("Slice")))) {
        px_srcline(155);
        _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(156);
        _v777 = px_str("px_null()");
        px_srcline(157);
        if (px_is_truthy(px_ne(px_index(_v772, px_int(2LL)), px_null()))) {
            px_srcline(158);
             _v777 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        }
        px_srcline(159);
        _v788 = px_str("px_null()");
        px_srcline(160);
        if (px_is_truthy(px_ne(px_index(_v772, px_int(3LL)), px_null()))) {
            px_srcline(161);
             _v788 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        }
        px_srcline(162);
        _v789 = px_str("px_null()");
        px_srcline(163);
        if (px_is_truthy(px_ne(px_index(_v772, px_int(4LL)), px_null()))) {
            px_srcline(164);
             _v789 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(4LL))}, 1);
        }
        px_srcline(165);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v786), px_str(", ")), _v777), px_str(", ")), _v788), px_str(", ")), _v789), px_str(")"));
    }
    px_srcline(166);
    if (px_is_truthy(px_eq(_v773, px_str("Call")))) {
        px_srcline(167);
        _v790 = px_index(_v772, px_int(1LL));
        px_srcline(168);
        _v791 = px_index(_v772, px_int(2LL));
        px_srcline(169);
        if (px_is_truthy(px_eq(px_index(_v790, px_int(0LL)), px_str("Var")))) {
            px_srcline(170);
            _v792 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v790, px_int(1LL))}, 1);
            px_srcline(172);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v792}, 1))) {
                px_srcline(173);
                _v793 = px_index(px_get_global("cg_ffi"), _v792);
                px_srcline(174);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v791}, 1), px_call(px_get_global("len"), (LXValue[]){_v793}, 1)))) {
                    px_srcline(175);
                    (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), px_add(px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v792), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v793}, 1)}, 1)), px_str(" 个参数，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v791}, 1)}, 1))}, 2));
                }
                px_srcline(176);
                _v774 = px_list_n((LXValue[]){}, 0);
                px_srcline(177);
                _v794 = px_int(0LL);
                px_srcline(178);
                while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                    px_srcline(179);
                    (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
                    px_srcline(180);
                     _v794 = px_add(_v794, px_int(1LL));
                }
                px_srcline(181);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v792), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")}, 2)"));
            }
            px_srcline(182);
            if (px_is_truthy(px_eq(_v792, px_str("chan")))) {
                px_srcline(183);
                _v795 = px_str("0");
                px_srcline(184);
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v791}, 1), px_int(0LL)))) {
                    px_srcline(185);
                    if (px_is_truthy(px_eq(px_index(px_index(_v791, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                        px_srcline(186);
                         _v795 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v791, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                        px_srcline(188);
                         _v795 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                px_srcline(189);
                return px_add(px_add(px_str("px_chan_create("), _v795), px_str(")"));
            }
            px_srcline(190);
            if (px_is_truthy(px_eq(_v792, px_str("mutex")))) {
                px_srcline(191);
                return px_str("px_mutex_create()");
            }
            px_srcline(192);
            if (px_is_truthy(px_eq(_v792, px_str("rwlock")))) {
                px_srcline(193);
                return px_str("px_rwlock_create()");
            }
            px_srcline(195);
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v792}, 1))) {
                px_srcline(196);
                _v796 = px_index(px_get_global("cg_structs"), _v792);
                px_srcline(197);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v796}, 1), px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                    px_srcline(198);
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v792), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v796}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v791}, 1)}, 1));
                }
                px_srcline(199);
                _v774 = px_list_n((LXValue[]){}, 0);
                px_srcline(200);
                _v794 = px_int(0LL);
                px_srcline(201);
                while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                    px_srcline(202);
                    (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
                    px_srcline(203);
                     _v794 = px_add(_v794, px_int(1LL));
                }
                px_srcline(204);
                _v797 = px_list_n((LXValue[]){}, 0);
                px_srcline(205);
                _v798 = px_int(0LL);
                px_srcline(206);
                while (px_is_truthy(px_lt(_v798, px_call(px_get_global("len"), (LXValue[]){_v796}, 1)))) {
                    px_srcline(207);
                    (void)(px_method(_v797, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v796, _v798)), px_str("\""))}, 1));
                    px_srcline(208);
                     _v798 = px_add(_v798, px_int(1LL));
                }
                px_srcline(209);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v792), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v797}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v796}, 1)}, 1)), px_str(")"));
            }
            px_srcline(210);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v792}, 1))) {
                px_srcline(211);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v791}, 1), px_int(1LL)))) {
                    px_srcline(212);
                    return px_add(px_add(px_str("枚举 "), _v792), px_str(" 构造需要一个变体名"));
                }
                px_srcline(213);
                _v782 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, px_int(0LL))}, 1);
                px_srcline(214);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v792), px_str("\", (")), _v782), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        px_srcline(216);
        if (px_is_truthy(px_eq(px_index(_v790, px_int(0LL)), px_str("Field")))) {
            px_srcline(217);
            _v783 = px_index(_v790, px_int(1LL));
            px_srcline(218);
            _v799 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v790, px_int(2LL))}, 1);
            px_srcline(220);
            _v800 = px_null();
            px_srcline(221);
            if (px_is_truthy(px_eq(px_index(_v783, px_int(0LL)), px_str("Var")))) {
                px_srcline(222);
                _v785 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v783, px_int(1LL))}, 1);
                px_srcline(223);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v785}, 1))) {
                    px_srcline(224);
                     _v800 = px_index(px_get_global("cg_var_types"), _v785);
                }
            }
            px_srcline(225);
            if (px_is_truthy(({ LXValue _t839 = px_ne(_v800, px_null()); px_is_truthy(_t839) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v800}, 1) : _t839; }))) {
                px_srcline(226);
                _v801 = px_index(px_get_global("cg_impls"), _v800);
                px_srcline(227);
                _v802 = px_bool(false);
                px_srcline(228);
                _v803 = px_int(0LL);
                px_srcline(229);
                while (px_is_truthy(px_lt(_v803, px_call(px_get_global("len"), (LXValue[]){_v801}, 1)))) {
                    px_srcline(230);
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v801, _v803), px_int(1LL))}, 1), _v799))) {
                        px_srcline(231);
                         _v802 = px_bool(true);
                        px_srcline(232);
                        break;
                    }
                    px_srcline(233);
                     _v803 = px_add(_v803, px_int(1LL));
                }
                px_srcline(234);
                if (px_is_truthy(_v802)) {
                    px_srcline(235);
                    _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v783}, 1);
                    px_srcline(236);
                    _v774 = px_list_n((LXValue[]){_v786}, 1);
                    px_srcline(237);
                    _v794 = px_int(0LL);
                    px_srcline(238);
                    while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                        px_srcline(239);
                        (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
                        px_srcline(240);
                         _v794 = px_add(_v794, px_int(1LL));
                    }
                    px_srcline(241);
                    _v804 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v800}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v799}, 1));
                    px_srcline(242);
                    return px_add(px_add(px_add(px_add(px_add(_v804, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            px_srcline(244);
            _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v783}, 1);
            px_srcline(245);
            _v774 = px_list_n((LXValue[]){}, 0);
            px_srcline(246);
            _v794 = px_int(0LL);
            px_srcline(247);
            while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                px_srcline(248);
                (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
                px_srcline(249);
                 _v794 = px_add(_v794, px_int(1LL));
            }
            px_srcline(250);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v786), px_str(", \"")), _v799), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")"));
        }
        px_srcline(252);
        _v805 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v790}, 1);
        px_srcline(253);
        _v774 = px_list_n((LXValue[]){}, 0);
        px_srcline(254);
        _v794 = px_int(0LL);
        px_srcline(255);
        while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
            px_srcline(256);
            (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
            px_srcline(257);
             _v794 = px_add(_v794, px_int(1LL));
        }
        px_srcline(258);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v805), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")"));
    }
    px_srcline(259);
    if (px_is_truthy(px_eq(_v773, px_str("Unary")))) {
        px_srcline(260);
        _v786 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(261);
        _v806 = px_index(_v772, px_int(1LL));
        px_srcline(262);
        if (px_is_truthy(px_eq(_v806, px_str("Neg")))) {
            px_srcline(263);
            return px_add(px_add(px_str("px_neg("), _v786), px_str(")"));
        }
        px_srcline(264);
        if (px_is_truthy(px_eq(_v806, px_str("Not")))) {
            px_srcline(265);
            return px_add(px_add(px_str("px_not("), _v786), px_str(")"));
        }
        px_srcline(266);
        return px_add(px_add(px_str("px_bitnot("), _v786), px_str(")"));
    }
    px_srcline(267);
    if (px_is_truthy(px_eq(_v773, px_str("Binary")))) {
        px_srcline(268);
        _v807 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(269);
        _v808 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        px_srcline(270);
        _v806 = px_index(_v772, px_int(1LL));
        px_srcline(271);
        if (px_is_truthy(px_eq(_v806, px_str("And")))) {
            px_srcline(272);
            _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(273);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v807), px_str("; px_is_truthy(")), _v787), px_str(") ? ")), _v808), px_str(" : ")), _v787), px_str("; })"));
        }
        px_srcline(274);
        if (px_is_truthy(px_eq(_v806, px_str("Or")))) {
            px_srcline(275);
            _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(276);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v807), px_str("; px_is_truthy(")), _v787), px_str(") ? ")), _v787), px_str(" : ")), _v808), px_str("; })"));
        }
        px_srcline(277);
        _v809 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v806}, 1);
        px_srcline(278);
        return px_add(px_add(px_add(px_add(px_add(_v809, px_str("(")), _v807), px_str(", ")), _v808), px_str(")"));
    }
    px_srcline(279);
    if (px_is_truthy(px_eq(_v773, px_str("Pipe")))) {
        px_srcline(280);
        _v782 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(281);
        _v810 = px_index(_v772, px_int(2LL));
        px_srcline(282);
        if (px_is_truthy(px_eq(px_index(_v810, px_int(0LL)), px_str("Call")))) {
            px_srcline(283);
            _v805 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v810, px_int(1LL))}, 1);
            px_srcline(284);
            _v774 = px_list_n((LXValue[]){_v782}, 1);
            px_srcline(285);
            _v794 = px_int(0LL);
            px_srcline(286);
            while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){px_index(_v810, px_int(2LL))}, 1)))) {
                px_srcline(287);
                (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v810, px_int(2LL)), _v794)}, 1)}, 1));
                px_srcline(288);
                 _v794 = px_add(_v794, px_int(1LL));
            }
            px_srcline(289);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v805), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")"));
        }
        px_srcline(290);
        _v809 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v810}, 1);
        px_srcline(291);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v809), px_str(", (LXValue[]){")), _v782), px_str("}, 1)"));
    }
    px_srcline(292);
    if (px_is_truthy(px_eq(_v773, px_str("NullCoalesce")))) {
        px_srcline(293);
        _v807 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(294);
        _v808 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(295);
        _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(296);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v807), px_str("; px_is_null(")), _v787), px_str(") ? ")), _v808), px_str(" : ")), _v787), px_str("; })"));
    }
    px_srcline(297);
    if (px_is_truthy(px_eq(_v773, px_str("Try")))) {
        px_srcline(298);
        _v788 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(299);
        _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(300);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            px_srcline(301);
            _v811 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            px_srcline(302);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v788), px_str("; if (px_is_result(")), _v787), px_str(")) { if (!px_result_ok(")), _v787), px_str(")) { ")), _v811), px_str("_val = ")), _v787), px_str("; ")), _v811), px_str("_proped = 1; goto ")), _v811), px_str("; } ")), _v787), px_str(" = px_result_unwrap(")), _v787), px_str("); } else if (px_is_null(")), _v787), px_str(")) { ")), _v811), px_str("_val = px_null(); ")), _v811), px_str("_proped = 1; goto ")), _v811), px_str("; } ")), _v787), px_str("; })"));
        }
        px_srcline(303);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v788), px_str("; if (px_is_result(")), _v787), px_str(") && !px_result_ok(")), _v787), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v787), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v787), px_str(")) ")), _v787), px_str(" = px_result_unwrap(")), _v787), px_str("); ")), _v787), px_str("; })"));
    }
    px_srcline(304);
    if (px_is_truthy(px_eq(_v773, px_str("ForceUnwrap")))) {
        px_srcline(305);
        _v788 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(306);
        _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(307);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v788), px_str("; if (px_is_result(")), _v787), px_str(")) { if (!px_result_ok(")), _v787), px_str(")) px_error(\"force unwrap Err\"); ")), _v787), px_str(" = px_result_unwrap(")), _v787), px_str("); } if (px_is_null(")), _v787), px_str(")) px_error(\"force unwrap null\"); ")), _v787), px_str("; })"));
    }
    px_srcline(308);
    if (px_is_truthy(px_eq(_v773, px_str("IfExpr")))) {
        px_srcline(309);
        _v805 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(310);
        _v812 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(311);
        _v813 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        px_srcline(312);
        _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(313);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str("; if (px_is_truthy(")), _v805), px_str(")) { ")), _v787), px_str(" = ")), _v812), px_str("; } else { ")), _v787), px_str(" = ")), _v813), px_str("; } ")), _v787), px_str("; })"));
    }
    px_srcline(314);
    if (px_is_truthy(px_eq(_v773, px_str("ListComp")))) {
        px_srcline(315);
        _v814 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(316);
        _v808 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(317);
        _v788 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(318);
        _v815 = px_null();
        px_srcline(319);
        if (px_is_truthy(px_ne(px_index(_v772, px_int(3LL)), px_null()))) {
            px_srcline(320);
             _v815 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        }
        px_srcline(321);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v808, px_str("saved_all"))}, 1));
        px_srcline(322);
        _v816 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v814), px_str(", ")), _v788), px_str("); "));
        px_srcline(323);
        _v817 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v808, _v815, _v816}, 3);
        px_srcline(324);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v814), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v808, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v808, px_str("its")), px_int(0LL))), px_str("; ")), _v817), px_str(" ")), _v814), px_str("; })"));
    }
    px_srcline(325);
    if (px_is_truthy(px_eq(_v773, px_str("GenExp")))) {
        px_srcline(326);
        _v818 = px_index(_v772, px_int(2LL));
        px_srcline(327);
        if (px_is_truthy(({ LXValue _t840 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v818}, 1), px_int(1LL)); px_is_truthy(_t840) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v818, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t840; }))) {
            px_srcline(328);
            _v819 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v818, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(329);
            _v820 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v818, px_int(0LL)), px_int(2LL))}, 1);
            px_srcline(330);
            _v821 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v819}, 1), px_index(_v772, px_int(1LL))}, 2);
            px_srcline(331);
            _v822 = px_str("px_null()");
            px_srcline(332);
            if (px_is_truthy(px_ne(px_index(_v772, px_int(3LL)), px_null()))) {
                px_srcline(333);
                 _v822 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v819}, 1), px_index(_v772, px_int(3LL))}, 2);
            }
            px_srcline(334);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v820), px_str(", ")), _v821), px_str(", ")), _v822), px_str(")"));
        }
        px_srcline(336);
        _v814 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(337);
        _v808 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v818}, 1);
        px_srcline(338);
        _v788 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(339);
        _v815 = px_null();
        px_srcline(340);
        if (px_is_truthy(px_ne(px_index(_v772, px_int(3LL)), px_null()))) {
            px_srcline(341);
             _v815 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        }
        px_srcline(342);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v808, px_str("saved_all"))}, 1));
        px_srcline(343);
        _v816 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v814), px_str(", ")), _v788), px_str("); "));
        px_srcline(344);
        _v817 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v808, _v815, _v816}, 3);
        px_srcline(345);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v814), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v808, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v808, px_str("its")), px_int(0LL))), px_str("; ")), _v817), px_str(" px_gen_from_list(")), _v814), px_str("); })"));
    }
    px_srcline(346);
    if (px_is_truthy(px_eq(_v773, px_str("DictComp")))) {
        px_srcline(347);
        _v814 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(348);
        _v808 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        px_srcline(349);
        _v779 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(350);
        _v780 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(2LL))}, 1);
        px_srcline(351);
        _v815 = px_null();
        px_srcline(352);
        if (px_is_truthy(px_ne(px_index(_v772, px_int(4LL)), px_null()))) {
            px_srcline(353);
             _v815 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(4LL))}, 1);
        }
        px_srcline(354);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v808, px_str("saved_all"))}, 1));
        px_srcline(355);
        _v816 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v779), px_str("; LXValue _v = ")), _v780), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v814), px_str(", _k.as.obj->as.str.data, _v); } "));
        px_srcline(356);
        _v817 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v808, _v815, _v816}, 3);
        px_srcline(357);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v814), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v808, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v808, px_str("its")), px_int(0LL))), px_str("; ")), _v817), px_str(" ")), _v814), px_str("; })"));
    }
    px_srcline(358);
    if (px_is_truthy(px_eq(_v773, px_str("Closure")))) {
        px_srcline(359);
        px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
        px_srcline(360);
        _v823 = px_get_global("cg_closure_id");
        px_srcline(361);
        _v804 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v823}, 1));
        px_srcline(362);
        _v824 = px_add(px_add(px_str("static LXValue "), _v804), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
        px_srcline(363);
         _v824 = px_add(_v824, px_str("    (void)ctx;\n"));
        px_srcline(364);
        _v825 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
        px_srcline(365);
        _v826 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
        px_srcline(366);
        px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_srcline(367);
        px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_srcline(368);
        _v827 = px_index(_v772, px_int(1LL));
        px_srcline(369);
        _v828 = px_int(0LL);
        px_srcline(370);
        while (px_is_truthy(px_lt(_v828, px_call(px_get_global("len"), (LXValue[]){_v827}, 1)))) {
            px_srcline(371);
            _v782 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v827, _v828), px_int(1LL))}, 1)}, 1);
            px_srcline(372);
             _v824 = px_add(_v824, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v782), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v828}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v828}, 1)), px_str("] : px_null();\n")));
            px_srcline(373);
             _v828 = px_add(_v828, px_int(1LL));
        }
        px_srcline(374);
        _v813 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(3LL))}, 1);
        px_srcline(375);
         _v824 = px_add(_v824, px_add(px_add(px_str("    return "), _v813), px_str(";\n")));
        px_srcline(376);
         _v824 = px_add(_v824, px_str("}\n"));
        px_srcline(377);
        px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v824));
        px_srcline(378);
        px_set_global("cg_vars", _v825);
        px_srcline(379);
        px_set_global("cg_var_types", _v826);
        px_srcline(380);
        return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v823}, 1)), px_str(">\", ")), _v804), px_str(", NULL)"));
    }
    px_srcline(381);
    if (px_is_truthy(px_eq(_v773, px_str("Block")))) {
        px_srcline(382);
        _v777 = px_str("({ ");
        px_srcline(383);
         _v777 = px_add(_v777, px_str("LXValue _blk = px_null(); "));
        px_srcline(384);
        _v829 = px_index(_v772, px_int(1LL));
        px_srcline(385);
        _v830 = px_int(0LL);
        px_srcline(386);
        while (px_is_truthy(px_lt(_v830, px_call(px_get_global("len"), (LXValue[]){_v829}, 1)))) {
            px_srcline(387);
            _v831 = px_index(_v829, _v830);
            px_srcline(388);
            if (px_is_truthy(px_eq(px_index(_v831, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(389);
                _v788 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v831, px_int(1LL))}, 1);
                px_srcline(390);
                 _v777 = px_add(_v777, px_add(px_add(px_str("_blk = "), _v788), px_str("; ")));
            }
            else {
                px_srcline(392);
                 _v777 = px_add(_v777, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v831, px_int(0LL)}, 2));
            }
            px_srcline(393);
             _v830 = px_add(_v830, px_int(1LL));
        }
        px_srcline(394);
         _v777 = px_add(_v777, px_str("_blk; })"));
        px_srcline(395);
        return _v777;
    }
    px_srcline(396);
    if (px_is_truthy(px_eq(_v773, px_str("Match")))) {
        px_srcline(397);
        _v832 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(398);
        _v787 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(399);
        _v777 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v787), px_str(" = ")), _v832), px_str("; "));
        px_srcline(400);
        _v833 = px_index(_v772, px_int(2LL));
        px_srcline(401);
        _v834 = px_bool(true);
        px_srcline(402);
        _v794 = px_int(0LL);
        px_srcline(403);
        while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v833}, 1)))) {
            px_srcline(404);
            _v815 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v833, _v794), px_int(1LL)), _v787}, 2);
            px_srcline(405);
            _v835 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v833, _v794), px_int(3LL))}, 1);
            px_srcline(406);
            _v836 = px_str("if");
            px_srcline(407);
            if (px_is_truthy(px_not(_v834))) {
                px_srcline(408);
                 _v836 = px_str("else if");
            }
            px_srcline(409);
             _v777 = px_add(_v777, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v836, px_str(" (")), _v815), px_str(") { ")), _v787), px_str(" = ")), _v835), px_str("; } ")));
            px_srcline(410);
             _v834 = px_bool(false);
            px_srcline(411);
             _v794 = px_add(_v794, px_int(1LL));
        }
        px_srcline(412);
         _v777 = px_add(_v777, px_add(_v787, px_str("; })")));
        px_srcline(413);
        return _v777;
    }
    px_srcline(414);
    if (px_is_truthy(px_eq(_v773, px_str("Constructor")))) {
        px_srcline(415);
        _v781 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1);
        px_srcline(416);
        _v791 = px_index(_v772, px_int(2LL));
        px_srcline(417);
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v781}, 1))) {
            px_srcline(418);
            _v796 = px_index(px_get_global("cg_structs"), _v781);
            px_srcline(419);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v796}, 1), px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                px_srcline(420);
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v781), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v796}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v791}, 1)}, 1));
            }
            px_srcline(421);
            _v774 = px_list_n((LXValue[]){}, 0);
            px_srcline(422);
            _v794 = px_int(0LL);
            px_srcline(423);
            while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
                px_srcline(424);
                (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
                px_srcline(425);
                 _v794 = px_add(_v794, px_int(1LL));
            }
            px_srcline(426);
            _v797 = px_list_n((LXValue[]){}, 0);
            px_srcline(427);
            _v798 = px_int(0LL);
            px_srcline(428);
            while (px_is_truthy(px_lt(_v798, px_call(px_get_global("len"), (LXValue[]){_v796}, 1)))) {
                px_srcline(429);
                (void)(px_method(_v797, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v796, _v798)), px_str("\""))}, 1));
                px_srcline(430);
                 _v798 = px_add(_v798, px_int(1LL));
            }
            px_srcline(431);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v781), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v797}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v796}, 1)}, 1)), px_str(")"));
        }
        px_srcline(432);
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v781}, 1))) {
            px_srcline(433);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v791}, 1), px_int(1LL)))) {
                px_srcline(434);
                return px_add(px_add(px_str("枚举 "), _v781), px_str(" 构造需要一个变体名"));
            }
            px_srcline(435);
            _v782 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, px_int(0LL))}, 1);
            px_srcline(436);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v781), px_str("\", (")), _v782), px_str(").as.obj->as.enum_inst.variant)"));
        }
        px_srcline(437);
        _v774 = px_list_n((LXValue[]){}, 0);
        px_srcline(438);
        _v794 = px_int(0LL);
        px_srcline(439);
        while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v791}, 1)))) {
            px_srcline(440);
            (void)(px_method(_v774, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v791, _v794)}, 1)}, 1));
            px_srcline(441);
             _v794 = px_add(_v794, px_int(1LL));
        }
        px_srcline(442);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v781), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v774}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v774}, 1)}, 1)), px_str(")"));
    }
    px_srcline(443);
    return px_str("px_null()");
px_err_837:
    if (px_err_837_proped) return px_err_837_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_binop_cname");
    LXValue _v841 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_842_val = px_null();
    int px_err_842_proped = 0;
    px_srcline(446);
    if (px_is_truthy(px_eq(_v841, px_str("Add")))) {
        px_srcline(447);
        return px_str("px_add");
    }
    px_srcline(448);
    if (px_is_truthy(px_eq(_v841, px_str("Sub")))) {
        px_srcline(449);
        return px_str("px_sub");
    }
    px_srcline(450);
    if (px_is_truthy(px_eq(_v841, px_str("Mul")))) {
        px_srcline(451);
        return px_str("px_mul");
    }
    px_srcline(452);
    if (px_is_truthy(px_eq(_v841, px_str("Div")))) {
        px_srcline(453);
        return px_str("px_div");
    }
    px_srcline(454);
    if (px_is_truthy(px_eq(_v841, px_str("IntDiv")))) {
        px_srcline(455);
        return px_str("px_idiv");
    }
    px_srcline(456);
    if (px_is_truthy(px_eq(_v841, px_str("Mod")))) {
        px_srcline(457);
        return px_str("px_mod");
    }
    px_srcline(458);
    if (px_is_truthy(px_eq(_v841, px_str("Pow")))) {
        px_srcline(459);
        return px_str("px_pow");
    }
    px_srcline(460);
    if (px_is_truthy(px_eq(_v841, px_str("Eq")))) {
        px_srcline(461);
        return px_str("px_eq");
    }
    px_srcline(462);
    if (px_is_truthy(px_eq(_v841, px_str("Ne")))) {
        px_srcline(463);
        return px_str("px_ne");
    }
    px_srcline(464);
    if (px_is_truthy(px_eq(_v841, px_str("Lt")))) {
        px_srcline(465);
        return px_str("px_lt");
    }
    px_srcline(466);
    if (px_is_truthy(px_eq(_v841, px_str("Le")))) {
        px_srcline(467);
        return px_str("px_le");
    }
    px_srcline(468);
    if (px_is_truthy(px_eq(_v841, px_str("Gt")))) {
        px_srcline(469);
        return px_str("px_gt");
    }
    px_srcline(470);
    if (px_is_truthy(px_eq(_v841, px_str("Ge")))) {
        px_srcline(471);
        return px_str("px_ge");
    }
    px_srcline(472);
    if (px_is_truthy(px_eq(_v841, px_str("BitAnd")))) {
        px_srcline(473);
        return px_str("px_bitand");
    }
    px_srcline(474);
    if (px_is_truthy(px_eq(_v841, px_str("BitOr")))) {
        px_srcline(475);
        return px_str("px_bitor");
    }
    px_srcline(476);
    if (px_is_truthy(px_eq(_v841, px_str("BitXor")))) {
        px_srcline(477);
        return px_str("px_bitxor");
    }
    px_srcline(478);
    if (px_is_truthy(px_eq(_v841, px_str("Shl")))) {
        px_srcline(479);
        return px_str("px_shl");
    }
    px_srcline(480);
    if (px_is_truthy(px_eq(_v841, px_str("Shr")))) {
        px_srcline(481);
        return px_str("px_shr");
    }
    px_srcline(482);
    if (px_is_truthy(px_eq(_v841, px_str("ShrU")))) {
        px_srcline(483);
        return px_str("px_ushr");
    }
    px_srcline(484);
    return px_str("px_add");
px_err_842:
    if (px_err_842_proped) return px_err_842_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_pattern_cond");
    LXValue _v843 = (nargs > 0) ? args[0] : px_null();
    LXValue _v844 = (nargs > 1) ? args[1] : px_null();
    LXValue _v845 = px_null();
    LXValue _v846 = px_null();
    LXValue _v847 = px_null();
    LXValue _v848 = px_null();
    LXValue px_err_849_val = px_null();
    int px_err_849_proped = 0;
    px_srcline(487);
    _v845 = px_index(_v843, px_int(0LL));
    px_srcline(488);
    if (px_is_truthy(px_eq(_v845, px_str("PatLiteral")))) {
        px_srcline(489);
        _v846 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, px_int(1LL))}, 1);
        px_srcline(490);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v844), px_str(", ")), _v846), px_str("))"));
    }
    px_srcline(491);
    if (px_is_truthy(px_eq(_v845, px_str("PatBinding")))) {
        px_srcline(492);
        _v847 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v843, px_int(1LL))}, 1);
        px_srcline(493);
        if (px_is_truthy(({ LXValue _t851 = ({ LXValue _t850 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v847}, 1), px_int(0LL)); px_is_truthy(_t850) ? px_ge(px_index(_v847, px_int(0LL)), px_str("A")) : _t850; }); px_is_truthy(_t851) ? px_le(px_index(_v847, px_int(0LL)), px_str("Z")) : _t851; }))) {
            px_srcline(494);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v844), px_str(".type == PX_ENUM && strcmp(")), _v844), px_str(".as.obj->as.enum_inst.variant, \"")), _v847), px_str("\") == 0)"));
        }
        px_srcline(495);
        return px_str("true");
    }
    px_srcline(496);
    if (px_is_truthy(px_eq(_v845, px_str("PatWildcard")))) {
        px_srcline(497);
        return px_str("true");
    }
    px_srcline(498);
    if (px_is_truthy(px_eq(_v845, px_str("PatTuple")))) {
        px_srcline(499);
        _v848 = px_index(_v843, px_int(1LL));
        px_srcline(500);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v848}, 1), px_int(0LL)))) {
            px_srcline(501);
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v848, px_int(0LL)), _v844}, 2);
        }
        px_srcline(502);
        return px_str("true");
    }
    px_srcline(503);
    if (px_is_truthy(px_eq(_v845, px_str("PatConstructor")))) {
        px_srcline(504);
        _v847 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v843, px_int(1LL))}, 1);
        px_srcline(505);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v844), px_str(".type == PX_ENUM && strcmp(")), _v844), px_str(".as.obj->as.enum_inst.variant, \"")), _v847), px_str("\") == 0)"));
    }
    px_srcline(506);
    return px_str("true");
px_err_849:
    if (px_err_849_proped) return px_err_849_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_lambda");
    LXValue _v852 = (nargs > 0) ? args[0] : px_null();
    LXValue _v853 = (nargs > 1) ? args[1] : px_null();
    LXValue _v854 = px_null();
    LXValue _v855 = px_null();
    LXValue _v856 = px_null();
    LXValue _v857 = px_null();
    LXValue _v858 = px_null();
    LXValue _v859 = px_null();
    LXValue _v860 = px_null();
    LXValue _v861 = px_null();
    LXValue _v862 = px_null();
    LXValue px_err_863_val = px_null();
    int px_err_863_proped = 0;
    px_srcline(509);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(510);
    _v854 = px_get_global("cg_closure_id");
    px_srcline(511);
    _v855 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v854}, 1));
    px_srcline(512);
    _v856 = px_add(px_add(px_str("static LXValue "), _v855), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(513);
     _v856 = px_add(_v856, px_str("    (void)ctx;\n"));
    px_srcline(514);
    _v857 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(515);
    _v858 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(516);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(517);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(518);
    _v859 = px_int(0LL);
    px_srcline(519);
    while (px_is_truthy(px_lt(_v859, px_call(px_get_global("len"), (LXValue[]){_v852}, 1)))) {
        px_srcline(520);
        _v860 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v852, _v859)}, 1);
        px_srcline(521);
         _v856 = px_add(_v856, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v860), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v859}, 1)), px_str("] : px_null();\n")));
        px_srcline(522);
         _v859 = px_add(_v859, px_int(1LL));
    }
    px_srcline(523);
    _v861 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v853}, 1);
    px_srcline(524);
    _v862 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(525);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v862}, 1));
    px_srcline(526);
     _v856 = px_add(_v856, px_add(px_add(px_str("    LXValue "), _v862), px_str("_val = px_null();\n")));
    px_srcline(527);
     _v856 = px_add(_v856, px_add(px_add(px_str("    int "), _v862), px_str("_proped = 0;\n")));
    px_srcline(528);
     _v856 = px_add(_v856, px_add(px_add(px_str("    return "), _v861), px_str(";\n")));
    px_srcline(529);
     _v856 = px_add(_v856, px_add(_v862, px_str(":\n")));
    px_srcline(530);
     _v856 = px_add(_v856, px_add(px_add(px_add(px_add(px_str("    if ("), _v862), px_str("_proped) return ")), _v862), px_str("_val;\n")));
    px_srcline(531);
     _v856 = px_add(_v856, px_str("    return px_null();\n"));
    px_srcline(532);
     _v856 = px_add(_v856, px_str("}\n"));
    px_srcline(533);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(534);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v856));
    px_srcline(535);
    px_set_global("cg_vars", _v857);
    px_srcline(536);
    px_set_global("cg_var_types", _v858);
    px_srcline(537);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v854}, 1)), px_str(">\", ")), _v855), px_str(", NULL)"));
px_err_863:
    if (px_err_863_proped) return px_err_863_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dirname");
    LXValue _v864 = (nargs > 0) ? args[0] : px_null();
    LXValue _v865 = px_null();
    LXValue px_err_866_val = px_null();
    int px_err_866_proped = 0;
    px_srcline(14);
    _v865 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v864}, 1), px_int(1LL));
    px_srcline(15);
    while (px_is_truthy(px_ge(_v865, px_int(0LL)))) {
        px_srcline(16);
        if (px_is_truthy(px_eq(px_index(_v864, _v865), px_str("/")))) {
            px_srcline(17);
            if (px_is_truthy(px_eq(_v865, px_int(0LL)))) {
                px_srcline(18);
                return px_str("/");
            }
            px_srcline(19);
            return px_slice(_v864, px_int(0LL), _v865, px_null());
        }
        px_srcline(20);
         _v865 = px_sub(_v865, px_int(1LL));
    }
    px_srcline(21);
    return px_str(".");
px_err_866:
    if (px_err_866_proped) return px_err_866_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_stdlib_dir");
    LXValue _v867 = px_null();
    LXValue _v868 = px_null();
    LXValue _v869 = px_null();
    LXValue _v870 = px_null();
    LXValue px_err_871_val = px_null();
    int px_err_871_proped = 0;
    px_srcline(24);
    _v867 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    px_srcline(25);
    if (px_is_truthy(({ LXValue _t872 = px_ne(_v867, px_null()); px_is_truthy(_t872) ? px_call(px_get_global("exists"), (LXValue[]){_v867}, 1) : _t872; }))) {
        px_srcline(26);
        return _v867;
    }
    px_srcline(27);
    _v868 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    px_srcline(28);
    _v869 = px_int(0LL);
    px_srcline(29);
    while (px_is_truthy(px_lt(_v869, px_call(px_get_global("len"), (LXValue[]){_v868}, 1)))) {
        px_srcline(30);
        _v870 = px_index(_v868, _v869);
        px_srcline(31);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v870}, 1))) {
            px_srcline(32);
            return _v870;
        }
        px_srcline(33);
         _v869 = px_add(_v869, px_int(1LL));
    }
    px_srcline(34);
    return px_null();
px_err_871:
    if (px_err_871_proped) return px_err_871_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find_module_path");
    LXValue _v873 = (nargs > 0) ? args[0] : px_null();
    LXValue _v874 = (nargs > 1) ? args[1] : px_null();
    LXValue _v875 = px_null();
    LXValue _v876 = px_null();
    LXValue _v877 = px_null();
    LXValue _v878 = px_null();
    LXValue _v879 = px_null();
    LXValue _v880 = px_null();
    LXValue _v881 = px_null();
    LXValue _v882 = px_null();
    LXValue _v883 = px_null();
    LXValue _v884 = px_null();
    LXValue _v885 = px_null();
    LXValue _v886 = px_null();
    LXValue _v887 = px_null();
    LXValue _v888 = px_null();
    LXValue _v889 = px_null();
    LXValue px_err_890_val = px_null();
    int px_err_890_proped = 0;
    px_srcline(37);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v873}, 1), px_int(0LL)))) {
        px_srcline(38);
        return px_null();
    }
    px_srcline(40);
    if (px_is_truthy(({ LXValue _t892 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v873}, 1), px_int(1LL)); px_is_truthy(_t892) ? ({ LXValue _t891 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v873, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t891) ? _t891 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v873, px_int(0LL)), px_str(".px")}, 2); }) : _t892; }))) {
        px_srcline(41);
        _v875 = px_index(_v873, px_int(0LL));
        px_srcline(42);
        _v876 = _v875;
        px_srcline(43);
        if (px_is_truthy(px_not(({ LXValue _t893 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v875}, 1), px_int(0LL)); px_is_truthy(_t893) ? px_eq(px_index(_v875, px_int(0LL)), px_str("/")) : _t893; })))) {
            px_srcline(44);
             _v876 = px_add(px_add(_v874, px_str("/")), _v875);
        }
        px_srcline(45);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v876}, 1))) {
            px_srcline(46);
            return _v876;
        }
        px_srcline(47);
        return px_null();
    }
    px_srcline(49);
    if (px_is_truthy(px_eq(px_index(_v873, px_int(0LL)), px_str("std")))) {
        px_srcline(50);
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v873}, 1), px_int(2LL)))) {
            px_srcline(51);
            return px_null();
        }
        px_srcline(52);
        _v877 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(53);
        if (px_is_truthy(px_eq(_v877, px_null()))) {
            px_srcline(54);
            return px_null();
        }
        px_srcline(55);
        _v876 = _v877;
        px_srcline(56);
        _v878 = px_int(1LL);
        px_srcline(57);
        while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v873}, 1)))) {
            px_srcline(58);
             _v876 = px_add(_v876, px_add(px_str("/"), px_index(_v873, _v878)));
            px_srcline(59);
             _v878 = px_add(_v878, px_int(1LL));
        }
        px_srcline(60);
        _v879 = px_add(_v876, px_str(".px"));
        px_srcline(61);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v879}, 1))) {
            px_srcline(62);
            return _v879;
        }
        px_srcline(63);
        _v880 = px_add(_v876, px_str("/mod.px"));
        px_srcline(64);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v880}, 1))) {
            px_srcline(65);
            return _v880;
        }
        px_srcline(66);
        return px_null();
    }
    px_srcline(68);
    _v881 = px_list_n((LXValue[]){_v874}, 1);
    px_srcline(69);
    _v882 = px_add(_v874, px_str("/.px_modules"));
    px_srcline(70);
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v882}, 1))) {
        px_srcline(71);
        (void)(px_method(_v881, "append", (LXValue[]){_v882}, 1));
        px_srcline(72);
        _v883 = px_call(px_get_global("list_dir"), (LXValue[]){_v882}, 1);
        px_srcline(73);
        _v884 = px_int(0LL);
        px_srcline(74);
        while (px_is_truthy(px_lt(_v884, px_call(px_get_global("len"), (LXValue[]){_v883}, 1)))) {
            px_srcline(75);
            _v885 = px_index(_v883, _v884);
            px_srcline(76);
            _v886 = px_add(px_add(_v882, px_str("/")), _v885);
            px_srcline(77);
            if (px_is_truthy(({ LXValue _t894 = px_call(px_get_global("exists"), (LXValue[]){_v886}, 1); px_is_truthy(_t894) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v885, px_str(".")}, 2)) : _t894; }))) {
                px_srcline(78);
                (void)(px_method(_v881, "append", (LXValue[]){_v886}, 1));
            }
            px_srcline(79);
             _v884 = px_add(_v884, px_int(1LL));
        }
    }
    px_srcline(80);
    _v887 = px_int(0LL);
    px_srcline(81);
    while (px_is_truthy(px_lt(_v887, px_call(px_get_global("len"), (LXValue[]){_v881}, 1)))) {
        px_srcline(82);
        _v888 = px_index(_v881, _v887);
        px_srcline(83);
        _v876 = _v888;
        px_srcline(84);
        _v878 = px_int(0LL);
        px_srcline(85);
        while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v873}, 1)))) {
            px_srcline(86);
             _v876 = px_add(_v876, px_add(px_str("/"), px_index(_v873, _v878)));
            px_srcline(87);
             _v878 = px_add(_v878, px_int(1LL));
        }
        px_srcline(88);
        _v879 = px_add(_v876, px_str(".px"));
        px_srcline(89);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v879}, 1))) {
            px_srcline(90);
            return _v879;
        }
        px_srcline(91);
        _v880 = px_add(_v876, px_str("/mod.px"));
        px_srcline(92);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v880}, 1))) {
            px_srcline(93);
            return _v880;
        }
        px_srcline(94);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v873}, 1), px_int(1LL)))) {
            px_srcline(95);
            _v889 = px_add(px_add(px_add(_v888, px_str("/")), px_index(_v873, px_int(0LL))), px_str(".px"));
            px_srcline(96);
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v889}, 1))) {
                px_srcline(97);
                return _v889;
            }
        }
        px_srcline(98);
         _v887 = px_add(_v887, px_int(1LL));
    }
    px_srcline(99);
    return px_null();
px_err_890:
    if (px_err_890_proped) return px_err_890_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_definition");
    LXValue _v895 = (nargs > 0) ? args[0] : px_null();
    LXValue _v896 = px_null();
    LXValue px_err_897_val = px_null();
    int px_err_897_proped = 0;
    px_srcline(102);
    _v896 = px_index(_v895, px_int(0LL));
    px_srcline(103);
    if (px_is_truthy(px_eq(_v896, px_str("FuncDef")))) {
        px_srcline(105);
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v895, px_int(1LL))}, 1), px_str("main")))) {
            px_srcline(106);
            return px_bool(false);
        }
        px_srcline(107);
        return px_bool(true);
    }
    px_srcline(108);
    if (px_is_truthy(px_eq(_v896, px_str("ExternDef")))) {
        px_srcline(109);
        return px_bool(true);
    }
    px_srcline(110);
    if (px_is_truthy(({ LXValue _t900 = ({ LXValue _t899 = ({ LXValue _t898 = px_eq(_v896, px_str("StructDef")); px_is_truthy(_t898) ? _t898 : px_eq(_v896, px_str("EnumDef")); }); px_is_truthy(_t899) ? _t899 : px_eq(_v896, px_str("TraitDef")); }); px_is_truthy(_t900) ? _t900 : px_eq(_v896, px_str("ImplDef")); }))) {
        px_srcline(111);
        return px_bool(true);
    }
    px_srcline(112);
    if (px_is_truthy(px_eq(_v896, px_str("VarDecl")))) {
        px_srcline(116);
        return px_bool(true);
    }
    px_srcline(117);
    return px_bool(false);
px_err_897:
    if (px_err_897_proped) return px_err_897_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_def_name");
    LXValue _v901 = (nargs > 0) ? args[0] : px_null();
    LXValue _v902 = px_null();
    LXValue _v903 = px_null();
    LXValue _v904 = px_null();
    LXValue _v905 = px_null();
    LXValue px_err_906_val = px_null();
    int px_err_906_proped = 0;
    px_srcline(120);
    _v902 = px_index(_v901, px_int(0LL));
    px_srcline(121);
    if (px_is_truthy(({ LXValue _t910 = ({ LXValue _t909 = ({ LXValue _t908 = ({ LXValue _t907 = px_eq(_v902, px_str("FuncDef")); px_is_truthy(_t907) ? _t907 : px_eq(_v902, px_str("StructDef")); }); px_is_truthy(_t908) ? _t908 : px_eq(_v902, px_str("EnumDef")); }); px_is_truthy(_t909) ? _t909 : px_eq(_v902, px_str("TraitDef")); }); px_is_truthy(_t910) ? _t910 : px_eq(_v902, px_str("ExternDef")); }))) {
        px_srcline(122);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v901, px_int(1LL))}, 1);
    }
    px_srcline(123);
    if (px_is_truthy(px_eq(_v902, px_str("VarDecl")))) {
        px_srcline(124);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v901, px_int(2LL))}, 1);
    }
    px_srcline(125);
    if (px_is_truthy(px_eq(_v902, px_str("ImplDef")))) {
        px_srcline(126);
        _v903 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v901, px_int(1LL))}, 1);
        px_srcline(127);
        _v904 = px_index(_v901, px_int(2LL));
        px_srcline(128);
        _v905 = px_str("None");
        px_srcline(129);
        if (px_is_truthy(px_ne(_v904, px_null()))) {
            px_srcline(130);
             _v905 = px_add(px_add(px_str("Some("), _v904), px_str(")"));
        }
        px_srcline(131);
        return px_add(px_add(px_add(px_str("impl::"), _v903), px_str("::")), _v905);
    }
    px_srcline(132);
    return px_null();
px_err_906:
    if (px_err_906_proped) return px_err_906_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_module");
    LXValue _v911 = (nargs > 0) ? args[0] : px_null();
    LXValue _v912 = (nargs > 1) ? args[1] : px_null();
    LXValue _v913 = (nargs > 2) ? args[2] : px_null();
    LXValue _v914 = (nargs > 3) ? args[3] : px_null();
    LXValue _v915 = (nargs > 4) ? args[4] : px_null();
    LXValue _v916 = px_null();
    LXValue _v917 = px_null();
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
    LXValue px_err_933_val = px_null();
    int px_err_933_proped = 0;
    px_srcline(136);
    _v916 = px_list_n((LXValue[]){}, 0);
    px_srcline(137);
    _v917 = px_int(0LL);
    px_srcline(138);
    while (px_is_truthy(px_lt(_v917, px_call(px_get_global("len"), (LXValue[]){_v911}, 1)))) {
        px_srcline(139);
        (void)(px_method(_v916, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v911, _v917)}, 1)}, 1));
        px_srcline(140);
         _v917 = px_add(_v917, px_int(1LL));
    }
    px_srcline(141);
     _v911 = _v916;
    px_srcline(142);
    _v918 = px_list_n((LXValue[]){}, 0);
    px_srcline(143);
    _v919 = px_int(0LL);
    px_srcline(144);
    while (px_is_truthy(px_lt(_v919, px_call(px_get_global("len"), (LXValue[]){_v912}, 1)))) {
        px_srcline(145);
        (void)(px_method(_v918, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v912, _v919)}, 1)}, 1));
        px_srcline(146);
         _v919 = px_add(_v919, px_int(1LL));
    }
    px_srcline(147);
     _v912 = _v918;
    px_srcline(148);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v911}, 1), px_int(0LL)))) {
        px_srcline(149);
        return px_null();
    }
    px_srcline(151);
    if (px_is_truthy(({ LXValue _t936 = ({ LXValue _t935 = ({ LXValue _t934 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v911}, 1), px_int(1LL)); px_is_truthy(_t934) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v911, px_int(0LL))}, 1), px_int(2LL)) : _t934; }); px_is_truthy(_t935) ? px_eq(px_slice(px_index(_v911, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t935; }); px_is_truthy(_t936) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v911, px_int(0LL)), px_str(".px")}, 2)) : _t936; }))) {
        px_srcline(152);
        return px_null();
    }
    px_srcline(153);
    _v920 = px_eq(px_index(_v911, px_int(0LL)), px_str("std"));
    px_srcline(154);
    _v921 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v911}, 2);
    px_srcline(155);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v921}, 1))) {
        px_srcline(156);
        return px_null();
    }
    px_srcline(157);
    _v922 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v911, _v913}, 2);
    px_srcline(158);
    if (px_is_truthy(px_eq(_v922, px_null()))) {
        px_srcline(160);
        if (px_is_truthy(px_ne(px_index(_v911, px_int(0LL)), px_str("std")))) {
            px_srcline(161);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("[module] 警告: 找不到模块 '"), _v921), px_str("'（已跳过）"))}, 1));
        }
        px_srcline(162);
        return px_null();
    }
    px_srcline(163);
    px_index_set(px_get_global("loaded"), _v921, _v922);
    px_srcline(164);
    _v923 = px_call(px_get_global("read_file"), (LXValue[]){_v922}, 1);
    px_srcline(165);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v923}, 1));
    px_srcline(166);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(167);
    _v924 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(169);
    _v925 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v922}, 1);
    px_srcline(170);
    _v926 = px_list_n((LXValue[]){}, 0);
    px_srcline(171);
    _v927 = px_int(0LL);
    px_srcline(172);
    while (px_is_truthy(px_lt(_v927, px_call(px_get_global("len"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1)))) {
        px_srcline(173);
        _v928 = px_index(px_index(_v924, px_int(1LL)), _v927);
        px_srcline(174);
        if (px_is_truthy(px_eq(px_index(_v928, px_int(0LL)), px_str("Import")))) {
            px_srcline(175);
            (void)(px_method(_v926, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v928, px_int(1LL)), px_index(_v928, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(176);
         _v927 = px_add(_v927, px_int(1LL));
    }
    px_srcline(177);
    _v929 = px_int(0LL);
    px_srcline(178);
    while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
        px_srcline(179);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v926, _v929), px_int(0LL)), px_index(px_index(_v926, _v929), px_int(1LL)), _v925, _v914, _v915}, 5));
        px_srcline(180);
         _v929 = px_add(_v929, px_int(1LL));
    }
    px_srcline(182);
    _v930 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v912}, 1), px_int(0LL));
    px_srcline(183);
    _v931 = px_int(0LL);
    px_srcline(184);
    while (px_is_truthy(px_lt(_v931, px_call(px_get_global("len"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1)))) {
        px_srcline(185);
        _v928 = px_index(px_index(_v924, px_int(1LL)), _v931);
        px_srcline(186);
        if (px_is_truthy(px_eq(px_index(_v928, px_int(0LL)), px_str("Import")))) {
            px_srcline(187);
             _v931 = px_add(_v931, px_int(1LL));
            px_srcline(188);
            continue;
        }
        px_srcline(189);
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v928}, 1)))) {
            px_srcline(190);
             _v931 = px_add(_v931, px_int(1LL));
            px_srcline(191);
            continue;
        }
        px_srcline(192);
        _v932 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v928}, 1);
        px_srcline(193);
        if (px_is_truthy(px_eq(_v932, px_null()))) {
            px_srcline(194);
            (void)(px_method(_v914, "append", (LXValue[]){_v928}, 1));
        }
        else {
            px_srcline(196);
            if (px_is_truthy(_v930)) {
                px_srcline(197);
                if (px_is_truthy(({ LXValue _t937 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v932}, 1), px_int(5LL)); px_is_truthy(_t937) ? px_eq(px_slice(_v932, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t937; }))) {
                    px_srcline(198);
                     _v931 = px_add(_v931, px_int(1LL));
                    px_srcline(199);
                    continue;
                }
                px_srcline(200);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v912, _v932}, 2)))) {
                    px_srcline(201);
                     _v931 = px_add(_v931, px_int(1LL));
                    px_srcline(202);
                    continue;
                }
            }
            px_srcline(203);
            if (px_is_truthy(_v920)) {
                px_srcline(204);
                if (px_is_truthy(px_method(_v915, "has", (LXValue[]){_v932}, 1))) {
                    px_srcline(205);
                     _v931 = px_add(_v931, px_int(1LL));
                    px_srcline(206);
                    continue;
                }
                px_srcline(207);
                px_index_set(_v915, _v932, px_bool(true));
            }
            px_srcline(208);
            (void)(px_method(_v914, "append", (LXValue[]){_v928}, 1));
        }
        px_srcline(209);
         _v931 = px_add(_v931, px_int(1LL));
    }
px_err_933:
    if (px_err_933_proped) return px_err_933_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_resolve_modules");
    LXValue _v938 = (nargs > 0) ? args[0] : px_null();
    LXValue _v939 = (nargs > 1) ? args[1] : px_null();
    LXValue _v940 = px_null();
    LXValue _v941 = px_null();
    LXValue _v942 = px_null();
    LXValue _v943 = px_null();
    LXValue _v944 = px_null();
    LXValue _v945 = px_null();
    LXValue _v946 = px_null();
    LXValue _v947 = px_null();
    LXValue _v948 = px_null();
    LXValue _v949 = px_null();
    LXValue px_err_950_val = px_null();
    int px_err_950_proped = 0;
    px_srcline(212);
    _v940 = px_index(_v938, px_int(1LL));
    px_srcline(213);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v940}, 1), px_int(0LL)))) {
        px_srcline(214);
        return _v938;
    }
    px_srcline(215);
    _v941 = px_list_n((LXValue[]){}, 0);
    px_srcline(216);
    _v942 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(217);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(218);
    _v943 = px_list_n((LXValue[]){}, 0);
    px_srcline(219);
    _v944 = px_int(0LL);
    px_srcline(220);
    while (px_is_truthy(px_lt(_v944, px_call(px_get_global("len"), (LXValue[]){_v940}, 1)))) {
        px_srcline(221);
        _v945 = px_index(_v940, _v944);
        px_srcline(222);
        if (px_is_truthy(px_eq(px_index(_v945, px_int(0LL)), px_str("Import")))) {
            px_srcline(223);
            (void)(px_method(_v943, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v945, px_int(1LL)), px_index(_v945, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(224);
         _v944 = px_add(_v944, px_int(1LL));
    }
    px_srcline(225);
    _v946 = px_int(0LL);
    px_srcline(226);
    while (px_is_truthy(px_lt(_v946, px_call(px_get_global("len"), (LXValue[]){_v943}, 1)))) {
        px_srcline(227);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v943, _v946), px_int(0LL)), px_index(px_index(_v943, _v946), px_int(1LL)), _v939, _v941, _v942}, 5));
        px_srcline(228);
         _v946 = px_add(_v946, px_int(1LL));
    }
    px_srcline(229);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v941}, 1), px_int(0LL)))) {
        px_srcline(230);
        return _v938;
    }
    px_srcline(231);
    _v947 = px_list_n((LXValue[]){}, 0);
    px_srcline(232);
    _v948 = px_int(0LL);
    px_srcline(233);
    while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v941}, 1)))) {
        px_srcline(234);
        (void)(px_method(_v947, "append", (LXValue[]){px_index(_v941, _v948)}, 1));
        px_srcline(235);
         _v948 = px_add(_v948, px_int(1LL));
    }
    px_srcline(236);
    _v949 = px_int(0LL);
    px_srcline(237);
    while (px_is_truthy(px_lt(_v949, px_call(px_get_global("len"), (LXValue[]){_v940}, 1)))) {
        px_srcline(238);
        (void)(px_method(_v947, "append", (LXValue[]){px_index(_v940, _v949)}, 1));
        px_srcline(239);
         _v949 = px_add(_v949, px_int(1LL));
    }
    px_srcline(240);
    return px_list_n((LXValue[]){px_str("Program"), _v947}, 2);
px_err_950:
    if (px_err_950_proped) return px_err_950_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_dict");
    LXValue _v951 = px_null();
    LXValue px_err_952_val = px_null();
    int px_err_952_proped = 0;
    px_srcline(54);
    _v951 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(55);
    (void)(px_method(_v951, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(56);
    return _v951;
px_err_952:
    if (px_err_952_proped) return px_err_952_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dict_copy");
    LXValue _v953 = (nargs > 0) ? args[0] : px_null();
    LXValue _v954 = px_null();
    LXValue _v955 = px_null();
    LXValue _v956 = px_null();
    LXValue px_err_957_val = px_null();
    int px_err_957_proped = 0;
    px_srcline(58);
    _v954 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(59);
    _v955 = px_method(_v953, "keys", (LXValue[]){}, 0);
    px_srcline(60);
    _v956 = px_int(0LL);
    px_srcline(61);
    while (px_is_truthy(px_lt(_v956, px_call(px_get_global("len"), (LXValue[]){_v955}, 1)))) {
        px_srcline(62);
        px_index_set(_v954, px_index(_v955, _v956), px_index(_v953, px_index(_v955, _v956)));
        px_srcline(63);
         _v956 = px_add(_v956, px_int(1LL));
    }
    px_srcline(64);
    return _v954;
px_err_957:
    if (px_err_957_proped) return px_err_957_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_uid");
    LXValue px_err_958_val = px_null();
    int px_err_958_proped = 0;
    px_srcline(66);
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    px_srcline(67);
    return px_get_global("cg_uidc");
px_err_958:
    if (px_err_958_proped) return px_err_958_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_tmp");
    LXValue px_err_959_val = px_null();
    int px_err_959_proped = 0;
    px_srcline(69);
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_959:
    if (px_err_959_proped) return px_err_959_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_var");
    LXValue _v960 = (nargs > 0) ? args[0] : px_null();
    LXValue _v961 = px_null();
    LXValue px_err_962_val = px_null();
    int px_err_962_proped = 0;
    px_srcline(71);
    _v961 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(72);
    px_index_set(px_get_global("cg_vars"), _v960, _v961);
    px_srcline(73);
    return _v961;
px_err_962:
    if (px_err_962_proped) return px_err_962_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_var_of");
    LXValue _v963 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_964_val = px_null();
    int px_err_964_proped = 0;
    px_srcline(75);
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v963}, 1))) {
        px_srcline(76);
        return px_index(px_get_global("cg_vars"), _v963);
    }
    px_srcline(77);
    return px_null();
px_err_964:
    if (px_err_964_proped) return px_err_964_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_mark_immutable");
    LXValue _v965 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_966_val = px_null();
    int px_err_966_proped = 0;
    px_srcline(80);
    px_index_set(px_get_global("cg_immutables"), _v965, px_int(1LL));
px_err_966:
    if (px_err_966_proped) return px_err_966_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_immutable");
    LXValue _v967 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_968_val = px_null();
    int px_err_968_proped = 0;
    px_srcline(82);
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v967}, 1);
px_err_968:
    if (px_err_968_proped) return px_err_968_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_perr");
    LXValue _v969 = (nargs > 0) ? args[0] : px_null();
    LXValue _v970 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_971_val = px_null();
    int px_err_971_proped = 0;
    px_srcline(84);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v969), px_str(": ")), _v970)}, 1));
    px_srcline(85);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_971:
    if (px_err_971_proped) return px_err_971_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_nonnull_ty");
    LXValue _v972 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_973_val = px_null();
    int px_err_973_proped = 0;
    px_srcline(89);
    if (px_is_truthy(px_eq(_v972, px_null()))) {
        px_srcline(90);
        return px_bool(false);
    }
    px_srcline(91);
    if (px_is_truthy(px_eq(px_index(_v972, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(92);
        return px_bool(false);
    }
    px_srcline(93);
    return px_bool(true);
px_err_973:
    if (px_err_973_proped) return px_err_973_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_null_lit");
    LXValue _v974 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_975_val = px_null();
    int px_err_975_proped = 0;
    px_srcline(96);
    if (px_is_truthy(px_eq(_v974, px_null()))) {
        px_srcline(97);
        return px_bool(false);
    }
    px_srcline(98);
    if (px_is_truthy(px_eq(px_index(_v974, px_int(0LL)), px_str("Null")))) {
        px_srcline(99);
        return px_bool(true);
    }
    px_srcline(100);
    return px_bool(false);
px_err_975:
    if (px_err_975_proped) return px_err_975_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ty_name");
    LXValue _v976 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_977_val = px_null();
    int px_err_977_proped = 0;
    px_srcline(103);
    if (px_is_truthy(px_eq(_v976, px_null()))) {
        px_srcline(104);
        return px_str("any");
    }
    px_srcline(105);
    if (px_is_truthy(px_eq(px_index(_v976, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(106);
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v976, px_int(1LL))}, 1), px_str("?"));
    }
    px_srcline(107);
    if (px_is_truthy(px_eq(px_index(_v976, px_int(0LL)), px_str("TyNamed")))) {
        px_srcline(108);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v976, px_int(1LL))}, 1);
    }
    px_srcline(109);
    if (px_is_truthy(px_eq(px_index(_v976, px_int(0LL)), px_str("TyList")))) {
        px_srcline(110);
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v976, px_int(1LL))}, 1)), px_str("]"));
    }
    px_srcline(111);
    if (px_is_truthy(px_eq(px_index(_v976, px_int(0LL)), px_str("TyDict")))) {
        px_srcline(112);
        return px_add(px_add(px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v976, px_int(1LL))}, 1)), px_str(": ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v976, px_int(2LL))}, 1)), px_str("}"));
    }
    px_srcline(113);
    return px_str("any");
px_err_977:
    if (px_err_977_proped) return px_err_977_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_func_cname");
    LXValue _v978 = (nargs > 0) ? args[0] : px_null();
    LXValue _v979 = px_null();
    LXValue _v980 = px_null();
    LXValue _v981 = px_null();
    LXValue _v982 = px_null();
    LXValue _v983 = px_null();
    LXValue _v984 = px_null();
    LXValue px_err_985_val = px_null();
    int px_err_985_proped = 0;
    px_srcline(115);
    _v979 = px_str("");
    px_srcline(116);
    _v980 = px_int(0LL);
    px_srcline(117);
    while (px_is_truthy(px_lt(_v980, px_call(px_get_global("len"), (LXValue[]){_v978}, 1)))) {
        px_srcline(118);
        _v981 = px_index(_v978, _v980);
        px_srcline(119);
        _v982 = ({ LXValue _t986 = px_ge(_v981, px_str("a")); px_is_truthy(_t986) ? px_le(_v981, px_str("z")) : _t986; });
        px_srcline(120);
        _v983 = ({ LXValue _t987 = px_ge(_v981, px_str("A")); px_is_truthy(_t987) ? px_le(_v981, px_str("Z")) : _t987; });
        px_srcline(121);
        _v984 = ({ LXValue _t988 = px_ge(_v981, px_str("0")); px_is_truthy(_t988) ? px_le(_v981, px_str("9")) : _t988; });
        px_srcline(122);
        if (px_is_truthy(({ LXValue _t990 = ({ LXValue _t989 = _v982; px_is_truthy(_t989) ? _t989 : _v983; }); px_is_truthy(_t990) ? _t990 : _v984; }))) {
            px_srcline(123);
             _v979 = px_add(_v979, _v981);
        }
        else {
            px_srcline(125);
             _v979 = px_add(_v979, px_str("_"));
        }
        px_srcline(126);
         _v980 = px_add(_v980, px_int(1LL));
    }
    px_srcline(127);
    return _v979;
px_err_985:
    if (px_err_985_proped) return px_err_985_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find");
    LXValue _v991 = (nargs > 0) ? args[0] : px_null();
    LXValue _v992 = (nargs > 1) ? args[1] : px_null();
    LXValue _v993 = px_null();
    LXValue _v994 = px_null();
    LXValue _v995 = px_null();
    LXValue _v996 = px_null();
    LXValue _v997 = px_null();
    LXValue px_err_998_val = px_null();
    int px_err_998_proped = 0;
    px_srcline(129);
    _v993 = px_call(px_get_global("len"), (LXValue[]){_v991}, 1);
    px_srcline(130);
    _v994 = px_call(px_get_global("len"), (LXValue[]){_v992}, 1);
    px_srcline(131);
    _v995 = px_int(0LL);
    px_srcline(132);
    while (px_is_truthy(px_le(px_add(_v995, _v994), _v993))) {
        px_srcline(133);
        _v996 = px_int(0LL);
        px_srcline(134);
        _v997 = px_bool(true);
        px_srcline(135);
        while (px_is_truthy(px_lt(_v996, _v994))) {
            px_srcline(136);
            if (px_is_truthy(px_ne(px_index(_v991, px_add(_v995, _v996)), px_index(_v992, _v996)))) {
                px_srcline(137);
                 _v997 = px_bool(false);
                px_srcline(138);
                break;
            }
            px_srcline(139);
             _v996 = px_add(_v996, px_int(1LL));
        }
        px_srcline(140);
        if (px_is_truthy(_v997)) {
            px_srcline(141);
            return _v995;
        }
        px_srcline(142);
         _v995 = px_add(_v995, px_int(1LL));
    }
    px_srcline(143);
    return px_neg(px_int(1LL));
px_err_998:
    if (px_err_998_proped) return px_err_998_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad");
    LXValue _v999 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1000 = px_null();
    LXValue _v1001 = px_null();
    LXValue px_err_1002_val = px_null();
    int px_err_1002_proped = 0;
    px_srcline(145);
    _v1000 = px_str("");
    px_srcline(146);
    _v1001 = px_int(0LL);
    px_srcline(147);
    while (px_is_truthy(px_lt(_v1001, _v999))) {
        px_srcline(148);
         _v1000 = px_add(_v1000, px_str("    "));
        px_srcline(149);
         _v1001 = px_add(_v1001, px_int(1LL));
    }
    px_srcline(150);
    return _v1000;
px_err_1002:
    if (px_err_1002_proped) return px_err_1002_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_unescape");
    LXValue _v1003 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1004 = px_null();
    LXValue _v1005 = px_null();
    LXValue _v1006 = px_null();
    LXValue _v1007 = px_null();
    LXValue _v1008 = px_null();
    LXValue _v1009 = px_null();
    LXValue _v1010 = px_null();
    LXValue px_err_1011_val = px_null();
    int px_err_1011_proped = 0;
    px_srcline(153);
    _v1004 = px_slice(_v1003, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1003}, 1), px_int(1LL)), px_null());
    px_srcline(154);
    _v1005 = px_str("");
    px_srcline(155);
    _v1006 = px_int(0LL);
    px_srcline(156);
    while (px_is_truthy(px_lt(_v1006, px_call(px_get_global("len"), (LXValue[]){_v1004}, 1)))) {
        px_srcline(157);
        _v1007 = px_index(_v1004, _v1006);
        px_srcline(158);
        if (px_is_truthy(px_eq(_v1007, px_str("\\")))) {
            px_srcline(159);
            _v1008 = px_index(_v1004, px_add(_v1006, px_int(1LL)));
            px_srcline(160);
            if (px_is_truthy(px_eq(_v1008, px_str("n")))) {
                px_srcline(161);
                 _v1005 = px_add(_v1005, px_str("\n"));
                px_srcline(162);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1008, px_str("t")))) {
                px_srcline(164);
                 _v1005 = px_add(_v1005, px_str("\t"));
                px_srcline(165);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1008, px_str("r")))) {
                px_srcline(167);
                 _v1005 = px_add(_v1005, px_str("\r"));
                px_srcline(168);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1008, px_str("0")))) {
                px_srcline(170);
                 _v1005 = px_add(_v1005, px_str(""));
                px_srcline(171);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1008, px_str("\"")))) {
                px_srcline(173);
                 _v1005 = px_add(_v1005, px_str("\""));
                px_srcline(174);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1008, px_str("\\")))) {
                px_srcline(176);
                 _v1005 = px_add(_v1005, px_str("\\"));
                px_srcline(177);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1008, px_str("u")))) {
                px_srcline(179);
                _v1009 = px_add(_v1006, px_int(3LL));
                px_srcline(180);
                _v1010 = px_str("");
                px_srcline(181);
                while (px_is_truthy(({ LXValue _t1012 = px_lt(_v1009, px_call(px_get_global("len"), (LXValue[]){_v1004}, 1)); px_is_truthy(_t1012) ? px_ne(px_index(_v1004, _v1009), px_str("}")) : _t1012; }))) {
                    px_srcline(182);
                     _v1010 = px_add(_v1010, px_index(_v1004, _v1009));
                    px_srcline(183);
                     _v1009 = px_add(_v1009, px_int(1LL));
                }
                px_srcline(184);
                 _v1005 = px_add(_v1005, px_call(px_get_global("hex_to_char"), (LXValue[]){_v1010}, 1));
                px_srcline(185);
                 _v1006 = px_add(_v1009, px_int(1LL));
            }
            else {
                px_srcline(187);
                 _v1005 = px_add(_v1005, _v1008);
                px_srcline(188);
                 _v1006 = px_add(_v1006, px_int(2LL));
            }
        }
        else {
            px_srcline(190);
             _v1005 = px_add(_v1005, _v1007);
            px_srcline(191);
             _v1006 = px_add(_v1006, px_int(1LL));
        }
    }
    px_srcline(192);
    return _v1005;
px_err_1011:
    if (px_err_1011_proped) return px_err_1011_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_escape_str");
    LXValue _v1013 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1014 = px_null();
    LXValue _v1015 = px_null();
    LXValue _v1016 = px_null();
    LXValue px_err_1017_val = px_null();
    int px_err_1017_proped = 0;
    px_srcline(195);
    _v1014 = px_str("");
    px_srcline(196);
    _v1015 = px_int(0LL);
    px_srcline(197);
    while (px_is_truthy(px_lt(_v1015, px_call(px_get_global("len"), (LXValue[]){_v1013}, 1)))) {
        px_srcline(198);
        _v1016 = px_index(_v1013, _v1015);
        px_srcline(199);
        if (px_is_truthy(px_eq(_v1016, px_str("\\")))) {
            px_srcline(200);
             _v1014 = px_add(_v1014, px_str("\\\\"));
        }
        else if (px_is_truthy(px_eq(_v1016, px_str("\"")))) {
            px_srcline(202);
             _v1014 = px_add(_v1014, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v1016, px_str("\n")))) {
            px_srcline(204);
             _v1014 = px_add(_v1014, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v1016, px_str("\r")))) {
            px_srcline(206);
             _v1014 = px_add(_v1014, px_str("\\r"));
        }
        else if (px_is_truthy(px_eq(_v1016, px_str("\t")))) {
            px_srcline(208);
             _v1014 = px_add(_v1014, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v1016, px_str("")))) {
            px_srcline(212);
             _v1014 = px_add(_v1014, px_str(""));
        }
        else {
            px_srcline(214);
             _v1014 = px_add(_v1014, _v1016);
        }
        px_srcline(215);
         _v1015 = px_add(_v1015, px_int(1LL));
    }
    px_srcline(216);
    return _v1014;
px_err_1017:
    if (px_err_1017_proped) return px_err_1017_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad_zeros");
    LXValue _v1018 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1019 = px_null();
    LXValue _v1020 = px_null();
    LXValue px_err_1021_val = px_null();
    int px_err_1021_proped = 0;
    px_srcline(219);
    _v1019 = px_str("");
    px_srcline(220);
    _v1020 = px_int(0LL);
    px_srcline(221);
    while (px_is_truthy(px_lt(_v1020, _v1018))) {
        px_srcline(222);
         _v1019 = px_add(_v1019, px_str("0"));
        px_srcline(223);
         _v1020 = px_add(_v1020, px_int(1LL));
    }
    px_srcline(224);
    return _v1019;
px_err_1021:
    if (px_err_1021_proped) return px_err_1021_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_expand_sci");
    LXValue _v1022 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1023 = px_null();
    LXValue _v1024 = px_null();
    LXValue _v1025 = px_null();
    LXValue _v1026 = px_null();
    LXValue _v1027 = px_null();
    LXValue _v1028 = px_null();
    LXValue _v1029 = px_null();
    LXValue _v1030 = px_null();
    LXValue _v1031 = px_null();
    LXValue _v1032 = px_null();
    LXValue _v1033 = px_null();
    LXValue _v1034 = px_null();
    LXValue _v1035 = px_null();
    LXValue _v1036 = px_null();
    LXValue px_err_1037_val = px_null();
    int px_err_1037_proped = 0;
    px_srcline(226);
    _v1023 = px_neg(px_int(1LL));
    px_srcline(227);
    _v1024 = px_int(0LL);
    px_srcline(228);
    while (px_is_truthy(px_lt(_v1024, px_call(px_get_global("len"), (LXValue[]){_v1022}, 1)))) {
        px_srcline(229);
        if (px_is_truthy(({ LXValue _t1038 = px_eq(px_index(_v1022, _v1024), px_str("e")); px_is_truthy(_t1038) ? _t1038 : px_eq(px_index(_v1022, _v1024), px_str("E")); }))) {
            px_srcline(230);
             _v1023 = _v1024;
            px_srcline(231);
            break;
        }
        px_srcline(232);
         _v1024 = px_add(_v1024, px_int(1LL));
    }
    px_srcline(233);
    if (px_is_truthy(px_lt(_v1023, px_int(0LL)))) {
        px_srcline(234);
        return _v1022;
    }
    px_srcline(235);
    _v1025 = px_slice(_v1022, px_int(0LL), _v1023, px_null());
    px_srcline(236);
    _v1026 = px_slice(_v1022, px_add(_v1023, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1022}, 1), px_null());
    px_srcline(237);
    _v1027 = px_int(1LL);
    px_srcline(238);
    if (px_is_truthy(({ LXValue _t1039 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1026}, 1), px_int(0LL)); px_is_truthy(_t1039) ? px_eq(px_index(_v1026, px_int(0LL)), px_str("+")) : _t1039; }))) {
        px_srcline(239);
         _v1026 = px_slice(_v1026, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1026}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1040 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1026}, 1), px_int(0LL)); px_is_truthy(_t1040) ? px_eq(px_index(_v1026, px_int(0LL)), px_str("-")) : _t1040; }))) {
        px_srcline(241);
         _v1027 = px_neg(px_int(1LL));
        px_srcline(242);
         _v1026 = px_slice(_v1026, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1026}, 1), px_null());
    }
    px_srcline(243);
    _v1028 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1026}, 1), _v1027);
    px_srcline(244);
    _v1029 = px_bool(false);
    px_srcline(245);
    if (px_is_truthy(({ LXValue _t1041 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1025}, 1), px_int(0LL)); px_is_truthy(_t1041) ? px_eq(px_index(_v1025, px_int(0LL)), px_str("-")) : _t1041; }))) {
        px_srcline(246);
         _v1029 = px_bool(true);
        px_srcline(247);
         _v1025 = px_slice(_v1025, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1025}, 1), px_null());
    }
    px_srcline(248);
    _v1030 = px_str("");
    px_srcline(249);
    _v1031 = px_str("");
    px_srcline(250);
    _v1032 = px_neg(px_int(1LL));
    px_srcline(251);
    _v1033 = px_int(0LL);
    px_srcline(252);
    while (px_is_truthy(px_lt(_v1033, px_call(px_get_global("len"), (LXValue[]){_v1025}, 1)))) {
        px_srcline(253);
        if (px_is_truthy(px_eq(px_index(_v1025, _v1033), px_str(".")))) {
            px_srcline(254);
             _v1032 = _v1033;
            px_srcline(255);
            break;
        }
        px_srcline(256);
         _v1033 = px_add(_v1033, px_int(1LL));
    }
    px_srcline(257);
    if (px_is_truthy(px_lt(_v1032, px_int(0LL)))) {
        px_srcline(258);
         _v1030 = _v1025;
    }
    else {
        px_srcline(260);
         _v1030 = px_slice(_v1025, px_int(0LL), _v1032, px_null());
        px_srcline(261);
         _v1031 = px_slice(_v1025, px_add(_v1032, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1025}, 1), px_null());
    }
    px_srcline(262);
    _v1034 = px_add(_v1030, _v1031);
    px_srcline(263);
    _v1035 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1030}, 1), _v1028);
    px_srcline(264);
    _v1036 = px_str("");
    px_srcline(265);
    if (px_is_truthy(px_le(_v1035, px_int(0LL)))) {
        px_srcline(266);
         _v1036 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1035)}, 1)), _v1034);
    }
    else if (px_is_truthy(px_ge(_v1035, px_call(px_get_global("len"), (LXValue[]){_v1034}, 1)))) {
        px_srcline(268);
         _v1036 = px_add(_v1034, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1035, px_call(px_get_global("len"), (LXValue[]){_v1034}, 1))}, 1));
    }
    else {
        px_srcline(270);
         _v1036 = px_add(px_add(px_slice(_v1034, px_int(0LL), _v1035, px_null()), px_str(".")), px_slice(_v1034, _v1035, px_call(px_get_global("len"), (LXValue[]){_v1034}, 1), px_null()));
    }
    px_srcline(271);
    if (px_is_truthy(_v1029)) {
        px_srcline(272);
        return px_add(px_str("-"), _v1036);
    }
    px_srcline(273);
    return _v1036;
px_err_1037:
    if (px_err_1037_proped) return px_err_1037_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_fmt_float");
    LXValue _v1042 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1043 = px_null();
    LXValue _v1044 = px_null();
    LXValue px_err_1045_val = px_null();
    int px_err_1045_proped = 0;
    px_srcline(276);
    _v1043 = px_call(px_get_global("str"), (LXValue[]){_v1042}, 1);
    px_srcline(277);
    if (px_is_truthy(({ LXValue _t1047 = ({ LXValue _t1046 = px_eq(_v1043, px_str("inf")); px_is_truthy(_t1046) ? _t1046 : px_eq(_v1043, px_str("-inf")); }); px_is_truthy(_t1047) ? _t1047 : px_eq(_v1043, px_str("nan")); }))) {
        px_srcline(278);
        return _v1043;
    }
    px_srcline(279);
     _v1043 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1043}, 1);
    px_srcline(280);
    _v1044 = px_call(px_get_global("len"), (LXValue[]){_v1043}, 1);
    px_srcline(281);
    if (px_is_truthy(({ LXValue _t1048 = px_ge(_v1044, px_int(2LL)); px_is_truthy(_t1048) ? px_eq(px_slice(_v1043, px_sub(_v1044, px_int(2LL)), _v1044, px_null()), px_str(".0")) : _t1048; }))) {
        px_srcline(282);
        return px_slice(_v1043, px_int(0LL), px_sub(_v1044, px_int(2LL)), px_null());
    }
    px_srcline(283);
    return _v1043;
px_err_1045:
    if (px_err_1045_proped) return px_err_1045_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_types");
    LXValue _v1049 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1050 = px_null();
    LXValue _v1051 = px_null();
    LXValue _v1052 = px_null();
    LXValue _v1053 = px_null();
    LXValue _v1054 = px_null();
    LXValue _v1055 = px_null();
    LXValue _v1056 = px_null();
    LXValue _v1057 = px_null();
    LXValue _v1058 = px_null();
    LXValue _v1059 = px_null();
    LXValue px_err_1060_val = px_null();
    int px_err_1060_proped = 0;
    px_srcline(286);
    _v1050 = px_index(_v1049, px_int(1LL));
    px_srcline(287);
    _v1051 = px_int(0LL);
    px_srcline(288);
    while (px_is_truthy(px_lt(_v1051, px_call(px_get_global("len"), (LXValue[]){_v1050}, 1)))) {
        px_srcline(289);
        _v1052 = px_index(_v1050, _v1051);
        px_srcline(290);
        _v1053 = px_index(_v1052, px_int(0LL));
        px_srcline(291);
        if (px_is_truthy(px_eq(_v1053, px_str("StructDef")))) {
            px_srcline(292);
            _v1054 = px_list_n((LXValue[]){}, 0);
            px_srcline(293);
            _v1055 = px_int(0LL);
            px_srcline(294);
            while (px_is_truthy(px_lt(_v1055, px_call(px_get_global("len"), (LXValue[]){px_index(_v1052, px_int(2LL))}, 1)))) {
                px_srcline(295);
                (void)(px_method(_v1054, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1052, px_int(2LL)), _v1055), px_int(1LL))}, 1)}, 1));
                px_srcline(296);
                 _v1055 = px_add(_v1055, px_int(1LL));
            }
            px_srcline(297);
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1052, px_int(1LL))}, 1), _v1054);
        }
        else if (px_is_truthy(px_eq(_v1053, px_str("EnumDef")))) {
            px_srcline(299);
            _v1056 = px_list_n((LXValue[]){}, 0);
            px_srcline(300);
            _v1057 = px_int(0LL);
            px_srcline(301);
            while (px_is_truthy(px_lt(_v1057, px_call(px_get_global("len"), (LXValue[]){px_index(_v1052, px_int(2LL))}, 1)))) {
                px_srcline(302);
                (void)(px_method(_v1056, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1052, px_int(2LL)), _v1057), px_int(1LL))}, 1)}, 1));
                px_srcline(303);
                 _v1057 = px_add(_v1057, px_int(1LL));
            }
            px_srcline(304);
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1052, px_int(1LL))}, 1), _v1056);
        }
        else if (px_is_truthy(px_eq(_v1053, px_str("ImplDef")))) {
            px_srcline(306);
            _v1058 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1052, px_int(1LL))}, 1);
            px_srcline(307);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1058}, 1))) {
                px_srcline(308);
                _v1059 = px_int(0LL);
                px_srcline(309);
                while (px_is_truthy(px_lt(_v1059, px_call(px_get_global("len"), (LXValue[]){px_index(_v1052, px_int(3LL))}, 1)))) {
                    px_srcline(310);
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1058), "append", (LXValue[]){px_index(px_index(_v1052, px_int(3LL)), _v1059)}, 1));
                    px_srcline(311);
                     _v1059 = px_add(_v1059, px_int(1LL));
                }
            }
            else {
                px_srcline(313);
                px_index_set(px_get_global("cg_impls"), _v1058, px_index(_v1052, px_int(3LL)));
            }
        }
        px_srcline(314);
         _v1051 = px_add(_v1051, px_int(1LL));
    }
    px_srcline(317);
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1050}, 1));
px_err_1060:
    if (px_err_1060_proped) return px_err_1060_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_consts");
    LXValue _v1061 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1062 = px_null();
    LXValue _v1063 = px_null();
    LXValue _v1064 = px_null();
    LXValue _v1065 = px_null();
    LXValue _v1066 = px_null();
    LXValue _v1067 = px_null();
    LXValue _v1068 = px_null();
    LXValue _v1069 = px_null();
    LXValue px_err_1070_val = px_null();
    int px_err_1070_proped = 0;
    px_srcline(320);
    _v1062 = px_int(0LL);
    px_srcline(321);
    while (px_is_truthy(px_lt(_v1062, px_call(px_get_global("len"), (LXValue[]){_v1061}, 1)))) {
        px_srcline(322);
        _v1063 = px_index(_v1061, _v1062);
        px_srcline(323);
        _v1064 = px_index(_v1063, px_int(0LL));
        px_srcline(324);
        if (px_is_truthy(px_eq(_v1064, px_str("TypeConst")))) {
            px_srcline(325);
            _v1065 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1);
            px_srcline(326);
            _v1066 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            px_srcline(327);
            _v1067 = px_int(0LL);
            px_srcline(328);
            while (px_is_truthy(px_lt(_v1067, px_call(px_get_global("len"), (LXValue[]){px_index(_v1063, px_int(2LL))}, 1)))) {
                px_srcline(329);
                _v1068 = px_index(px_index(_v1063, px_int(2LL)), _v1067);
                px_srcline(330);
                px_index_set(_v1066, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1068, px_int(1LL))}, 1), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1068, px_int(2LL))}, 1));
                px_srcline(331);
                 _v1067 = px_add(_v1067, px_int(1LL));
            }
            px_srcline(332);
            px_index_set(px_get_global("cg_const_enums"), _v1065, _v1066);
        }
        else if (px_is_truthy(px_eq(_v1064, px_str("FuncDef")))) {
            px_srcline(334);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1063, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1064, px_str("If")))) {
            px_srcline(336);
            _v1069 = px_int(0LL);
            px_srcline(337);
            while (px_is_truthy(px_lt(_v1069, px_call(px_get_global("len"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1)))) {
                px_srcline(338);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1063, px_int(1LL)), _v1069), px_int(1LL))}, 1));
                px_srcline(339);
                 _v1069 = px_add(_v1069, px_int(1LL));
            }
            px_srcline(340);
            if (px_is_truthy(px_ne(px_index(_v1063, px_int(2LL)), px_null()))) {
                px_srcline(341);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1063, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1064, px_str("For")))) {
            px_srcline(343);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1063, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1064, px_str("While")))) {
            px_srcline(345);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1063, px_int(2LL))}, 1));
        }
        px_srcline(346);
         _v1062 = px_add(_v1062, px_int(1LL));
    }
px_err_1070:
    if (px_err_1070_proped) return px_err_1070_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_hoist_vars");
    LXValue _v1071 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1072 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1073 = px_null();
    LXValue _v1074 = px_null();
    LXValue _v1075 = px_null();
    LXValue _v1076 = px_null();
    LXValue _v1077 = px_null();
    LXValue _v1078 = px_null();
    LXValue _v1079 = px_null();
    LXValue _v1080 = px_null();
    LXValue px_err_1081_val = px_null();
    int px_err_1081_proped = 0;
    px_srcline(355);
    _v1073 = px_int(0LL);
    px_srcline(356);
    while (px_is_truthy(px_lt(_v1073, px_call(px_get_global("len"), (LXValue[]){_v1071}, 1)))) {
        px_srcline(357);
        _v1074 = px_index(_v1071, _v1073);
        px_srcline(358);
        _v1075 = px_index(_v1074, px_int(0LL));
        px_srcline(359);
        if (px_is_truthy(px_eq(_v1075, px_str("Assign")))) {
            px_srcline(360);
            _v1076 = px_index(_v1074, px_int(1LL));
            px_srcline(361);
            if (px_is_truthy(px_eq(px_index(_v1076, px_int(0LL)), px_str("Var")))) {
                px_srcline(362);
                _v1077 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1076, px_int(1LL))}, 1);
                px_srcline(363);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1072, _v1077}, 2)))) {
                    px_srcline(364);
                    (void)(px_method(_v1072, "append", (LXValue[]){_v1077}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1075, px_str("VarDecl")))) {
            px_srcline(366);
            _v1077 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1074, px_int(2LL))}, 1);
            px_srcline(367);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1072, _v1077}, 2)))) {
                px_srcline(368);
                (void)(px_method(_v1072, "append", (LXValue[]){_v1077}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1075, px_str("If")))) {
            px_srcline(370);
            _v1078 = px_index(_v1074, px_int(1LL));
            px_srcline(371);
            _v1079 = px_int(0LL);
            px_srcline(372);
            while (px_is_truthy(px_lt(_v1079, px_call(px_get_global("len"), (LXValue[]){_v1078}, 1)))) {
                px_srcline(373);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1078, _v1079), px_int(1LL)), _v1072}, 2));
                px_srcline(374);
                 _v1079 = px_add(_v1079, px_int(1LL));
            }
            px_srcline(375);
            if (px_is_truthy(px_ne(px_index(_v1074, px_int(2LL)), px_null()))) {
                px_srcline(376);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1074, px_int(2LL)), _v1072}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1075, px_str("For")))) {
            px_srcline(378);
            _v1080 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1074, px_int(1LL))}, 1);
            px_srcline(379);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1072, _v1080}, 2)))) {
                px_srcline(380);
                (void)(px_method(_v1072, "append", (LXValue[]){_v1080}, 1));
            }
            px_srcline(381);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1074, px_int(3LL)), _v1072}, 2));
        }
        else if (px_is_truthy(px_eq(_v1075, px_str("While")))) {
            px_srcline(383);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1074, px_int(2LL)), _v1072}, 2));
        }
        px_srcline(384);
         _v1073 = px_add(_v1073, px_int(1LL));
    }
px_err_1081:
    if (px_err_1081_proped) return px_err_1081_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func");
    LXValue _v1082 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1083 = px_null();
    LXValue px_err_1084_val = px_null();
    int px_err_1084_proped = 0;
    px_srcline(387);
    _v1083 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1082, px_int(1LL))}, 1)}, 1));
    px_srcline(388);
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1082, _v1083}, 2);
px_err_1084:
    if (px_err_1084_proped) return px_err_1084_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func_named");
    LXValue _v1085 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1086 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1087 = px_null();
    LXValue _v1088 = px_null();
    LXValue _v1089 = px_null();
    LXValue _v1090 = px_null();
    LXValue _v1091 = px_null();
    LXValue _v1092 = px_null();
    LXValue _v1093 = px_null();
    LXValue _v1094 = px_null();
    LXValue _v1095 = px_null();
    LXValue _v1096 = px_null();
    LXValue _v1097 = px_null();
    LXValue _v1098 = px_null();
    LXValue _v1099 = px_null();
    LXValue _v1100 = px_null();
    LXValue _v1101 = px_null();
    LXValue px_err_1102_val = px_null();
    int px_err_1102_proped = 0;
    px_srcline(390);
    _v1087 = px_add(px_add(px_str("static LXValue "), _v1086), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(391);
     _v1087 = px_add(_v1087, px_str("    (void)ctx;\n"));
    px_srcline(394);
     _v1087 = px_add(_v1087, px_add(px_add(px_str("    px_srcfunc(\""), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1085, px_int(1LL))}, 1)), px_str("\");\n")));
    px_srcline(395);
    _v1088 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(396);
    _v1089 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(397);
    _v1090 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(398);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(399);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(400);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1090}, 1));
    px_srcline(402);
    _v1091 = px_index(_v1085, px_int(2LL));
    px_srcline(403);
    _v1092 = px_int(0LL);
    px_srcline(404);
    while (px_is_truthy(px_lt(_v1092, px_call(px_get_global("len"), (LXValue[]){_v1091}, 1)))) {
        px_srcline(405);
        _v1093 = px_index(_v1091, _v1092);
        px_srcline(406);
        _v1094 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1093, px_int(1LL))}, 1)}, 1);
        px_srcline(407);
        _v1095 = px_str("px_null()");
        px_srcline(408);
        if (px_is_truthy(px_ne(px_index(_v1093, px_int(3LL)), px_null()))) {
            px_srcline(409);
             _v1095 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1093, px_int(3LL))}, 1);
        }
        px_srcline(410);
         _v1087 = px_add(_v1087, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1094), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1092}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1092}, 1)), px_str("] : ")), _v1095), px_str(";\n")));
        px_srcline(411);
         _v1092 = px_add(_v1092, px_int(1LL));
    }
    px_srcline(414);
    _v1096 = px_list_n((LXValue[]){}, 0);
    px_srcline(415);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1085, px_int(4LL)), _v1096}, 2));
    px_srcline(416);
    _v1097 = px_int(0LL);
    px_srcline(417);
    while (px_is_truthy(px_lt(_v1097, px_call(px_get_global("len"), (LXValue[]){_v1096}, 1)))) {
        px_srcline(418);
        _v1098 = px_index(_v1096, _v1097);
        px_srcline(419);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1098}, 1), px_null()))) {
            px_srcline(420);
             _v1097 = px_add(_v1097, px_int(1LL));
            px_srcline(421);
            continue;
        }
        px_srcline(422);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1098}, 2))) {
            px_srcline(423);
             _v1097 = px_add(_v1097, px_int(1LL));
            px_srcline(424);
            continue;
        }
        px_srcline(425);
        _v1094 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1098}, 1);
        px_srcline(426);
         _v1087 = px_add(_v1087, px_add(px_add(px_str("    LXValue "), _v1094), px_str(" = px_null();\n")));
        px_srcline(427);
         _v1097 = px_add(_v1097, px_int(1LL));
    }
    px_srcline(429);
    _v1099 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(430);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1099}, 1));
    px_srcline(431);
     _v1087 = px_add(_v1087, px_add(px_add(px_str("    LXValue "), _v1099), px_str("_val = px_null();\n")));
    px_srcline(432);
     _v1087 = px_add(_v1087, px_add(px_add(px_str("    int "), _v1099), px_str("_proped = 0;\n")));
    px_srcline(434);
    _v1100 = px_index(_v1085, px_int(4LL));
    px_srcline(435);
    _v1101 = px_int(0LL);
    px_srcline(436);
    while (px_is_truthy(px_lt(_v1101, px_call(px_get_global("len"), (LXValue[]){_v1100}, 1)))) {
        px_srcline(437);
         _v1087 = px_add(_v1087, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1100, _v1101), px_int(1LL)}, 2));
        px_srcline(438);
         _v1101 = px_add(_v1101, px_int(1LL));
    }
    px_srcline(439);
     _v1087 = px_add(_v1087, px_add(_v1099, px_str(":\n")));
    px_srcline(440);
     _v1087 = px_add(_v1087, px_add(px_add(px_add(px_add(px_str("    if ("), _v1099), px_str("_proped) return ")), _v1099), px_str("_val;\n")));
    px_srcline(441);
     _v1087 = px_add(_v1087, px_str("    return px_null();\n"));
    px_srcline(442);
     _v1087 = px_add(_v1087, px_str("}\n"));
    px_srcline(443);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(444);
    px_set_global("cg_vars", _v1088);
    px_srcline(445);
    px_set_global("cg_var_types", _v1089);
    px_srcline(446);
    px_set_global("cg_immutables", _v1090);
    px_srcline(447);
    return _v1087;
px_err_1102:
    if (px_err_1102_proped) return px_err_1102_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_generate");
    LXValue _v1103 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v1128 = px_null();
    LXValue _v1129 = px_null();
    LXValue _v1130 = px_null();
    LXValue _v1131 = px_null();
    LXValue _v1132 = px_null();
    LXValue _v1133 = px_null();
    LXValue _v1134 = px_null();
    LXValue _v1135 = px_null();
    LXValue _v1136 = px_null();
    LXValue px_err_1137_val = px_null();
    int px_err_1137_proped = 0;
    px_srcline(450);
    _v1104 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_srcline(451);
    px_set_global("cg_closures", px_str(""));
    px_srcline(452);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(453);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(454);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(455);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(456);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(457);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(458);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(459);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(460);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(461);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(462);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(463);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(464);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(465);
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1103}, 1));
    px_srcline(467);
    _v1105 = px_index(_v1103, px_int(1LL));
    px_srcline(468);
    _v1106 = px_int(0LL);
    px_srcline(469);
    while (px_is_truthy(px_lt(_v1106, px_call(px_get_global("len"), (LXValue[]){_v1105}, 1)))) {
        px_srcline(470);
        _v1107 = px_index(_v1105, _v1106);
        px_srcline(471);
        _v1108 = px_index(_v1107, px_int(0LL));
        px_srcline(472);
        if (px_is_truthy(px_eq(_v1108, px_str("FuncDef")))) {
            px_srcline(473);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1107, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1108, px_str("ExternDef")))) {
            px_srcline(476);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1107, px_int(1LL))}, 1), px_index(_v1107, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1108, px_str("VarDecl")))) {
            px_srcline(478);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1107, px_int(2LL))}, 1)}, 1));
            px_srcline(479);
            if (px_is_truthy(({ LXValue _t1138 = px_eq(px_index(_v1107, px_int(1LL)), px_str("Let")); px_is_truthy(_t1138) ? _t1138 : px_eq(px_index(_v1107, px_int(1LL)), px_str("Const")); }))) {
                px_srcline(480);
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1107, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1108, px_str("Assign")))) {
            px_srcline(482);
            _v1109 = px_index(_v1107, px_int(1LL));
            px_srcline(483);
            if (px_is_truthy(px_eq(px_index(_v1109, px_int(0LL)), px_str("Var")))) {
                px_srcline(484);
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1109, px_int(1LL))}, 1)}, 1));
            }
        }
        px_srcline(485);
         _v1106 = px_add(_v1106, px_int(1LL));
    }
    px_srcline(487);
    _v1110 = px_list_n((LXValue[]){}, 0);
    px_srcline(488);
    _v1111 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    px_srcline(489);
    _v1112 = px_int(0LL);
    px_srcline(490);
    while (px_is_truthy(px_lt(_v1112, px_call(px_get_global("len"), (LXValue[]){_v1111}, 1)))) {
        px_srcline(491);
        _v1113 = px_index(_v1111, _v1112);
        px_srcline(492);
        _v1114 = px_index(px_get_global("cg_impls"), _v1113);
        px_srcline(493);
        _v1115 = px_int(0LL);
        px_srcline(494);
        while (px_is_truthy(px_lt(_v1115, px_call(px_get_global("len"), (LXValue[]){_v1114}, 1)))) {
            px_srcline(495);
            (void)(px_method(_v1110, "append", (LXValue[]){px_list_n((LXValue[]){_v1113, px_index(_v1114, _v1115)}, 2)}, 1));
            px_srcline(496);
             _v1115 = px_add(_v1115, px_int(1LL));
        }
        px_srcline(497);
         _v1112 = px_add(_v1112, px_int(1LL));
    }
    px_srcline(499);
    _v1116 = px_int(1LL);
    px_srcline(500);
    while (px_is_truthy(px_lt(_v1116, px_call(px_get_global("len"), (LXValue[]){_v1110}, 1)))) {
        px_srcline(501);
        _v1117 = _v1116;
        px_srcline(502);
        while (px_is_truthy(px_gt(_v1117, px_int(0LL)))) {
            px_srcline(503);
            _v1118 = px_add(px_add(px_index(px_index(_v1110, px_sub(_v1117, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1110, px_sub(_v1117, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(504);
            _v1119 = px_add(px_add(px_index(px_index(_v1110, _v1117), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1110, _v1117), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(505);
            if (px_is_truthy(px_lt(_v1119, _v1118))) {
                px_srcline(506);
                _v1120 = px_index(_v1110, _v1117);
                px_srcline(507);
                px_index_set(_v1110, _v1117, px_index(_v1110, px_sub(_v1117, px_int(1LL))));
                px_srcline(508);
                px_index_set(_v1110, px_sub(_v1117, px_int(1LL)), _v1120);
            }
            px_srcline(509);
             _v1117 = px_sub(_v1117, px_int(1LL));
        }
        px_srcline(510);
         _v1116 = px_add(_v1116, px_int(1LL));
    }
    px_srcline(512);
    _v1121 = px_int(0LL);
    px_srcline(513);
    while (px_is_truthy(px_lt(_v1121, px_call(px_get_global("len"), (LXValue[]){_v1110}, 1)))) {
        px_srcline(514);
        _v1113 = px_index(px_index(_v1110, _v1121), px_int(0LL));
        px_srcline(515);
        _v1122 = px_index(px_index(_v1110, _v1121), px_int(1LL));
        px_srcline(516);
        _v1123 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1113}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1122, px_int(1LL))}, 1)}, 1));
        px_srcline(517);
         _v1104 = px_add(_v1104, px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1122, _v1123}, 2));
        px_srcline(518);
         _v1104 = px_add(_v1104, px_str("\n"));
        px_srcline(519);
         _v1121 = px_add(_v1121, px_int(1LL));
    }
    px_srcline(521);
    _v1124 = px_int(0LL);
    px_srcline(522);
    while (px_is_truthy(px_lt(_v1124, px_call(px_get_global("len"), (LXValue[]){_v1105}, 1)))) {
        px_srcline(523);
        _v1107 = px_index(_v1105, _v1124);
        px_srcline(524);
        if (px_is_truthy(px_eq(px_index(_v1107, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(525);
             _v1104 = px_add(_v1104, px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1107}, 1));
            px_srcline(526);
             _v1104 = px_add(_v1104, px_str("\n"));
        }
        px_srcline(527);
         _v1124 = px_add(_v1124, px_int(1LL));
    }
    px_srcline(529);
     _v1104 = px_add(_v1104, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(530);
     _v1104 = px_add(_v1104, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(531);
     _v1104 = px_add(_v1104, px_str("    px_register_builtins();\n"));
    px_srcline(533);
    _v1125 = px_int(0LL);
    px_srcline(534);
    while (px_is_truthy(px_lt(_v1125, px_call(px_get_global("len"), (LXValue[]){_v1105}, 1)))) {
        px_srcline(535);
        _v1107 = px_index(_v1105, _v1125);
        px_srcline(536);
        if (px_is_truthy(px_eq(px_index(_v1107, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(537);
            _v1123 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1107, px_int(1LL))}, 1);
            px_srcline(538);
            _v1126 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1123}, 1));
            px_srcline(539);
             _v1104 = px_add(_v1104, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1123), px_str("\", px_func(\"")), _v1123), px_str("\", ")), _v1126), px_str(", NULL));\n")));
        }
        px_srcline(540);
         _v1125 = px_add(_v1125, px_int(1LL));
    }
    px_srcline(542);
    _v1127 = px_int(0LL);
    px_srcline(543);
    while (px_is_truthy(px_lt(_v1127, px_call(px_get_global("len"), (LXValue[]){_v1110}, 1)))) {
        px_srcline(544);
        _v1113 = px_index(px_index(_v1110, _v1127), px_int(0LL));
        px_srcline(545);
        _v1122 = px_index(px_index(_v1110, _v1127), px_int(1LL));
        px_srcline(546);
        _v1128 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1122, px_int(1LL))}, 1);
        px_srcline(547);
        _v1123 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1113}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1128}, 1));
        px_srcline(548);
         _v1104 = px_add(_v1104, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1113), px_str(".")), _v1128), px_str("\", px_func(\"")), _v1113), px_str(".")), _v1128), px_str("\", ")), _v1123), px_str(", NULL));\n")));
        px_srcline(549);
         _v1127 = px_add(_v1127, px_int(1LL));
    }
    px_srcline(551);
    _v1129 = px_int(0LL);
    px_srcline(552);
    while (px_is_truthy(px_lt(_v1129, px_call(px_get_global("len"), (LXValue[]){_v1105}, 1)))) {
        px_srcline(553);
        _v1107 = px_index(_v1105, _v1129);
        px_srcline(554);
        _v1108 = px_index(_v1107, px_int(0LL));
        px_srcline(555);
        if (px_is_truthy(({ LXValue _t1144 = ({ LXValue _t1143 = ({ LXValue _t1142 = ({ LXValue _t1141 = ({ LXValue _t1140 = ({ LXValue _t1139 = px_ne(_v1108, px_str("FuncDef")); px_is_truthy(_t1139) ? px_ne(_v1108, px_str("StructDef")) : _t1139; }); px_is_truthy(_t1140) ? px_ne(_v1108, px_str("EnumDef")) : _t1140; }); px_is_truthy(_t1141) ? px_ne(_v1108, px_str("TraitDef")) : _t1141; }); px_is_truthy(_t1142) ? px_ne(_v1108, px_str("ImplDef")) : _t1142; }); px_is_truthy(_t1143) ? px_ne(_v1108, px_str("Import")) : _t1143; }); px_is_truthy(_t1144) ? px_ne(_v1108, px_str("ExternDef")) : _t1144; }))) {
            px_srcline(556);
             _v1104 = px_add(_v1104, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1107, px_int(1LL)}, 2));
        }
        px_srcline(557);
         _v1129 = px_add(_v1129, px_int(1LL));
    }
    px_srcline(559);
    _v1130 = px_bool(false);
    px_srcline(560);
    _v1131 = px_int(0LL);
    px_srcline(561);
    while (px_is_truthy(px_lt(_v1131, px_call(px_get_global("len"), (LXValue[]){_v1105}, 1)))) {
        px_srcline(562);
        _v1107 = px_index(_v1105, _v1131);
        px_srcline(563);
        if (px_is_truthy(({ LXValue _t1145 = px_eq(px_index(_v1107, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1145) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1107, px_int(1LL))}, 1), px_str("main")) : _t1145; }))) {
            px_srcline(564);
             _v1130 = px_bool(true);
            px_srcline(565);
            break;
        }
        px_srcline(566);
         _v1131 = px_add(_v1131, px_int(1LL));
    }
    px_srcline(567);
    if (px_is_truthy(_v1130)) {
        px_srcline(568);
        _v1126 = px_str("fn_main");
        px_srcline(569);
         _v1104 = px_add(_v1104, px_add(px_add(px_str("    { LXValue _r = "), _v1126), px_str("(NULL, 0, NULL); int _code = 0;\n")));
        px_srcline(570);
         _v1104 = px_add(_v1104, px_str("      if (px_is_result(_r)) {\n"));
        px_srcline(571);
         _v1104 = px_add(_v1104, px_str("        if (!px_result_ok(_r)) {\n"));
        px_srcline(572);
         _v1104 = px_add(_v1104, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
        px_srcline(573);
         _v1104 = px_add(_v1104, px_str("          _code = 1;\n"));
        px_srcline(574);
         _v1104 = px_add(_v1104, px_str("        } else {\n"));
        px_srcline(575);
         _v1104 = px_add(_v1104, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
        px_srcline(576);
         _v1104 = px_add(_v1104, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
        px_srcline(577);
         _v1104 = px_add(_v1104, px_str("        }\n"));
        px_srcline(578);
         _v1104 = px_add(_v1104, px_str("      } else if (_r.type == PX_INT) {\n"));
        px_srcline(579);
         _v1104 = px_add(_v1104, px_str("        _code = (int)_r.as.i;\n"));
        px_srcline(580);
         _v1104 = px_add(_v1104, px_str("      }\n"));
        px_srcline(581);
         _v1104 = px_add(_v1104, px_str("      return _code;\n"));
        px_srcline(582);
         _v1104 = px_add(_v1104, px_str("    }\n"));
    }
    else {
        px_srcline(584);
         _v1104 = px_add(_v1104, px_str("    return 0;\n"));
    }
    px_srcline(585);
     _v1104 = px_add(_v1104, px_str("}\n"));
    px_srcline(587);
    _v1132 = px_call(px_get_global("cg_find"), (LXValue[]){_v1104, px_str("int main(")}, 2);
    px_srcline(588);
    if (px_is_truthy(px_ge(_v1132, px_int(0LL)))) {
        px_srcline(589);
        _v1133 = px_slice(_v1104, px_int(0LL), _v1132, px_null());
        px_srcline(590);
        _v1134 = px_slice(_v1104, _v1132, px_call(px_get_global("len"), (LXValue[]){_v1104}, 1), px_null());
        px_srcline(591);
        _v1135 = px_call(px_get_global("cg_find"), (LXValue[]){_v1133, px_str("static LXValue")}, 2);
        px_srcline(592);
        _v1136 = px_str("");
        px_srcline(593);
        if (px_is_truthy(px_ge(_v1135, px_int(0LL)))) {
            px_srcline(594);
             _v1136 = px_add(px_add(px_add(px_add(px_slice(_v1133, px_int(0LL), _v1135, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1133, _v1135, px_call(px_get_global("len"), (LXValue[]){_v1133}, 1), px_null())), _v1134);
        }
        else {
            px_srcline(596);
             _v1136 = px_add(px_add(px_add(_v1133, px_get_global("cg_closures")), px_str("\n")), _v1134);
        }
        px_srcline(597);
        return _v1136;
    }
    px_srcline(598);
    return _v1104;
px_err_1137:
    if (px_err_1137_proped) return px_err_1137_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v1146 = px_null();
    LXValue _v1147 = px_null();
    LXValue _v1148 = px_null();
    LXValue _v1149 = px_null();
    LXValue _v1150 = px_null();
    LXValue _v1151 = px_null();
    LXValue _v1152 = px_null();
    LXValue px_err_1153_val = px_null();
    int px_err_1153_proped = 0;
    px_srcline(50);
    _v1146 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    px_srcline(52);
    if (px_is_truthy(({ LXValue _t1155 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1146}, 1), px_int(2LL)); px_is_truthy(_t1155) ? ({ LXValue _t1154 = px_eq(px_index(_v1146, px_int(1LL)), px_str("--version")); px_is_truthy(_t1154) ? _t1154 : px_eq(px_index(_v1146, px_int(1LL)), px_str("-v")); }) : _t1155; }))) {
        px_srcline(53);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        px_srcline(54);
        return px_int(0LL);
    }
    px_srcline(56);
    _v1147 = px_index(_v1146, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1146}, 1), px_int(1LL)));
    px_srcline(57);
    _v1148 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1147}, 1);
    px_srcline(58);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v1147}, 1)}, 1));
    px_srcline(59);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(60);
    _v1149 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(61);
    _v1150 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v1149, _v1148}, 2);
    px_srcline(62);
    _v1151 = px_call(px_get_global("cg_generate"), (LXValue[]){_v1150}, 1);
    px_srcline(64);
    _v1152 = px_call(px_get_global("len"), (LXValue[]){_v1151}, 1);
    px_srcline(65);
    if (px_is_truthy(({ LXValue _t1156 = px_gt(_v1152, px_int(0LL)); px_is_truthy(_t1156) ? px_eq(px_index(_v1151, px_sub(_v1152, px_int(1LL))), px_str("\n")) : _t1156; }))) {
        px_srcline(66);
         _v1151 = px_slice(_v1151, px_int(0LL), px_sub(_v1152, px_int(1LL)), px_null());
    }
    px_srcline(67);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1151}, 1));
px_err_1153:
    if (px_err_1153_proped) return px_err_1153_val;
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
    px_set_global("skip_expr_ws", px_func("skip_expr_ws", fn_skip_expr_ws, NULL));
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
    px_set_global("cg_gen_stmt_inner", px_func("cg_gen_stmt_inner", fn_cg_gen_stmt_inner, NULL));
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
    px_srcline(13);
    px_set_global("g_src", px_str(""));
    px_srcline(14);
    px_set_global("g_len", px_int(0LL));
    px_srcline(15);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(16);
    px_set_global("g_line", px_int(1LL));
    px_srcline(17);
    px_set_global("g_col", px_int(1LL));
    px_srcline(18);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(19);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(20);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(21);
    px_set_global("g_count", px_int(0LL));
    px_srcline(22);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(24);
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_srcline(27);
    px_set_global("CTRL_ALL", px_str(""));
    px_srcline(11);
    px_set_global("g_src", px_str(""));
    px_srcline(12);
    px_set_global("g_len", px_int(0LL));
    px_srcline(13);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(14);
    px_set_global("g_line", px_int(1LL));
    px_srcline(15);
    px_set_global("g_col", px_int(1LL));
    px_srcline(16);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(17);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(18);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(19);
    px_set_global("g_count", px_int(0LL));
    px_srcline(20);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(21);
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_srcline(22);
    px_set_global("CTRL_ALL", px_str(""));
    px_srcline(23);
    px_set_global("LAYOUT", ({ LXValue _d = px_dict(); { LXValue _k = px_str("Program"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Program"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("items"), px_str("l")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("VarDecl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("VarDecl"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("kind"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2)); } { LXValue _k = px_str("Assign"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Assign"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("target"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("op"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("ExprStmt"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("ExprStmt"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("If"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("If"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("branches"), px_str("lt2b")}, 2), px_list_n((LXValue[]){px_str("else_branch"), px_str("ol")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("For"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("For"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("var"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("iterable"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("While"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("While"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("cond"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Return"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Break"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("Continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Continue"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("FuncDef"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("FuncDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("params"), px_str("lp")}, 2), px_list_n((LXValue[]){px_str("ret_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2), px_list_n((LXValue[]){px_str("type_params"), px_str("ls")}, 2)}, 6)}, 2)); } { LXValue _k = px_str("StructDef"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("StructDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("fields"), px_str("lsf")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2), px_list_n((LXValue[]){px_str("type_params"), px_str("ls")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("EnumDef"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("EnumDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("variants"), px_str("lev")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("TypeConst"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("TypeConst"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("items"), px_str("ltci")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("TraitDef"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("TraitDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("methods"), px_str("lfd")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("ImplDef"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("ImplDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("type_name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("trait_name"), px_str("os")}, 2), px_list_n((LXValue[]){px_str("methods"), px_str("lfd")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("Import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("module"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("names"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("ExternDef"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("ExternDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("params"), px_str("lp")}, 2), px_list_n((LXValue[]){px_str("ret_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("Spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Spawn"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("ChanDecl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("ChanDecl"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("elem_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Send"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("chan"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Recv"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("chan"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Select"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("arms"), px_str("lt3")}, 2), px_list_n((LXValue[]){px_str("else_branch"), px_str("ol")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Empty"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Empty"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("Int"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Int"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Float"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Float"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("f")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Str"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Str"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Bool"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Bool"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Null"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("List"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("items"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Tuple"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("items"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Dict"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("entries"), px_str("lt2")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Var"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Field"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Field"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("OptionalField"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("OptionalField"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Index"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Index"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("index"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Slice"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Slice"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("start"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("end"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("step"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2)); } { LXValue _k = px_str("Call"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Call"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("callee"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("args"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Unary"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Unary"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("op"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("operand"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Binary"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Binary"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("op"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("left"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("right"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("Pipe"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Pipe"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("func"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("NullCoalesce"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("NullCoalesce"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("left"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("right"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Try"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Try"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("ForceUnwrap"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("ForceUnwrap"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("IfExpr"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("IfExpr"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("cond"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("then"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("else_"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("ListComp"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("ListComp"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("clauses"), px_str("lc")}, 2), px_list_n((LXValue[]){px_str("cond"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("DictComp"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("DictComp"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("key"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("clauses"), px_str("lc")}, 2), px_list_n((LXValue[]){px_str("cond"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2)); } { LXValue _k = px_str("GenExp"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("GenExp"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("clauses"), px_str("lc")}, 2), px_list_n((LXValue[]){px_str("cond"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("Closure"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Closure"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("params"), px_str("lp")}, 2), px_list_n((LXValue[]){px_str("ret_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("captures"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2)); } { LXValue _k = px_str("Block"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Block"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("stmts"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("Match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Match"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("subject"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("arms"), px_str("lma")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Constructor"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Constructor"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("args"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("Param"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Param"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("default"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("StructField"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("StructField"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("ty"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("EnumVariant"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("EnumVariant"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("fields"), px_str("tl")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("TypeConstItem"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("TypeConstItem"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("MatchArm"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("MatchArm"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pattern"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("guard"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2)); } { LXValue _k = px_str("CompClause"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("CompClause"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("vars"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("iterable"), px_str("n")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("PatLiteral"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Literal"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("PatBinding"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Binding"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("PatWildcard"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Wildcard"), px_list_n((LXValue[]){}, 0)}, 2)); } { LXValue _k = px_str("PatTuple"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("lpl")}, 2)}, 1)}, 2)); } { LXValue _k = px_str("PatConstructor"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Constructor"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2), px_list_n((LXValue[]){px_null(), px_str("lpl")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("TyNamed"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Named"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("TyOptional"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Optional"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("TyList"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("TyDict"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("TyTuple"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("tl")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2)); } { LXValue _k = px_str("TyFunc"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Func"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("tl")}, 2), px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 3)}, 2)); } { LXValue _k = px_str("TyGeneric"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_str("Generic"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2), px_list_n((LXValue[]){px_null(), px_str("tl")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 3)}, 2)); } _d; }));
    px_srcline(96);
    px_set_global("p_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(97);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(21);
    px_set_global("g_src", px_str(""));
    px_srcline(22);
    px_set_global("g_len", px_int(0LL));
    px_srcline(23);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(24);
    px_set_global("g_line", px_int(1LL));
    px_srcline(25);
    px_set_global("g_col", px_int(1LL));
    px_srcline(26);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(27);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(28);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(29);
    px_set_global("g_count", px_int(0LL));
    px_srcline(30);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(31);
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_srcline(32);
    px_set_global("CTRL_ALL", px_str(""));
    px_srcline(33);
    px_set_global("p_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(34);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(36);
    px_set_global("cg_closures", px_str(""));
    px_srcline(37);
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(38);
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(39);
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(40);
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(41);
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(42);
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(43);
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(44);
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(45);
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(46);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(47);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(48);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(49);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(50);
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(16);
    px_set_global("g_src", px_str(""));
    px_srcline(17);
    px_set_global("g_len", px_int(0LL));
    px_srcline(18);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(19);
    px_set_global("g_line", px_int(1LL));
    px_srcline(20);
    px_set_global("g_col", px_int(1LL));
    px_srcline(21);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(22);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(23);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(24);
    px_set_global("g_count", px_int(0LL));
    px_srcline(25);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(26);
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_srcline(27);
    px_set_global("CTRL_ALL", px_str(""));
    px_srcline(28);
    px_set_global("p_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(29);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(30);
    px_set_global("cg_closures", px_str(""));
    px_srcline(31);
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(32);
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(33);
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(34);
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(35);
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(36);
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(37);
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(38);
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(39);
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(40);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(41);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(42);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(43);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(44);
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(46);
    px_set_global("PXC_VER", px_str("0.1.0"));
    px_srcline(47);
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
