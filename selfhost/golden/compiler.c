/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_is_cont_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_cont_op");
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcline(47);
    return px_method(px_get_global("CONT_OPS"), "has", (LXValue[]){_v1}, 1);
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_peek(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("peek");
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    px_srcline(53);
    if (px_is_truthy(px_lt(px_get_global("g_pos"), px_get_global("g_len")))) {
        px_srcline(54);
        return px_index(px_get_global("g_src"), px_get_global("g_pos"));
    }
    px_srcline(55);
    return px_str("");
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_peek2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("peek2");
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    px_srcline(57);
    if (px_is_truthy(px_lt(px_add(px_get_global("g_pos"), px_int(1LL)), px_get_global("g_len")))) {
        px_srcline(58);
        return px_index(px_get_global("g_src"), px_add(px_get_global("g_pos"), px_int(1LL)));
    }
    px_srcline(59);
    return px_str("");
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

static LXValue fn_peek3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("peek3");
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(61);
    if (px_is_truthy(px_lt(px_add(px_get_global("g_pos"), px_int(2LL)), px_get_global("g_len")))) {
        px_srcline(62);
        return px_index(px_get_global("g_src"), px_add(px_get_global("g_pos"), px_int(2LL)));
    }
    px_srcline(63);
    return px_str("");
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_advance(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("advance");
    LXValue _v6 = px_null();
    LXValue px_err_7_val = px_null();
    int px_err_7_proped = 0;
    px_srcline(65);
    if (px_is_truthy(px_ge(px_get_global("g_pos"), px_get_global("g_len")))) {
        px_srcline(66);
        return px_str("");
    }
    px_srcline(67);
    _v6 = px_index(px_get_global("g_src"), px_get_global("g_pos"));
    px_srcline(68);
    px_set_global("g_pos", px_add(px_get_global("g_pos"), px_int(1LL)));
    px_srcline(69);
    if (px_is_truthy(px_eq(_v6, px_str("\n")))) {
        px_srcline(70);
        px_set_global("g_line", px_add(px_get_global("g_line"), px_int(1LL)));
        px_srcline(71);
        px_set_global("g_col", px_int(1LL));
    }
    else {
        px_srcline(73);
        px_set_global("g_col", px_add(px_get_global("g_col"), px_int(1LL)));
    }
    px_srcline(74);
    return _v6;
px_err_7:
    if (px_err_7_proped) return px_err_7_val;
    return px_null();
}

static LXValue fn_emit_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("emit_at");
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = (nargs > 1) ? args[1] : px_null();
    LXValue _v10 = (nargs > 2) ? args[2] : px_null();
    LXValue _v11 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_12_val = px_null();
    int px_err_12_proped = 0;
    px_srcline(79);
    if (px_is_truthy(({ LXValue _t14 = ({ LXValue _t13 = px_eq(_v8, px_str("(")); px_is_truthy(_t13) ? _t13 : px_eq(_v8, px_str("[")); }); px_is_truthy(_t14) ? _t14 : px_eq(_v8, px_str("{")); }))) {
        px_srcline(80);
        px_set_global("g_bracket_depth", px_add(px_get_global("g_bracket_depth"), px_int(1LL)));
    }
    else if (px_is_truthy(({ LXValue _t17 = ({ LXValue _t16 = ({ LXValue _t15 = px_eq(_v8, px_str(")")); px_is_truthy(_t15) ? _t15 : px_eq(_v8, px_str("]")); }); px_is_truthy(_t16) ? _t16 : px_eq(_v8, px_str("}")); }); px_is_truthy(_t17) ? px_gt(px_get_global("g_bracket_depth"), px_int(0LL)) : _t17; }))) {
        px_srcline(82);
        px_set_global("g_bracket_depth", px_sub(px_get_global("g_bracket_depth"), px_int(1LL)));
    }
    px_srcline(83);
    (void)(px_method(px_get_global("g_toks"), "append", (LXValue[]){px_list_n((LXValue[]){_v8, _v9, _v10, _v11}, 4)}, 1));
    px_srcline(84);
    px_set_global("g_count", px_add(px_get_global("g_count"), px_int(1LL)));
    px_srcline(85);
    px_set_global("g_last_kind", _v8);
px_err_12:
    if (px_err_12_proped) return px_err_12_val;
    return px_null();
}

static LXValue fn_emit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("emit");
    LXValue _v18 = (nargs > 0) ? args[0] : px_null();
    LXValue _v19 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_20_val = px_null();
    int px_err_20_proped = 0;
    px_srcline(87);
    (void)(px_call(px_get_global("emit_at"), (LXValue[]){_v18, _v19, px_get_global("g_line"), px_get_global("g_col")}, 4));
px_err_20:
    if (px_err_20_proped) return px_err_20_val;
    return px_null();
}

static LXValue fn_emit_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("emit_token");
    LXValue _v21 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_22_val = px_null();
    int px_err_22_proped = 0;
    px_srcline(89);
    (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_index(_v21, px_int(0LL)), px_index(_v21, px_int(1LL)), px_index(_v21, px_int(2LL)), px_index(_v21, px_int(3LL))}, 4));
px_err_22:
    if (px_err_22_proped) return px_err_22_val;
    return px_null();
}

static LXValue fn_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("err");
    LXValue _v23 = (nargs > 0) ? args[0] : px_null();
    LXValue _v24 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_25_val = px_null();
    int px_err_25_proped = 0;
    px_srcline(96);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_line")}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_col")}, 1)), px_str(": 词法错误 ")), _v23), px_str(": ")), _v24)}, 1));
    px_srcline(97);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_25:
    if (px_err_25_proped) return px_err_25_val;
    return px_null();
}

static LXValue fn_err_at(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("err_at");
    LXValue _v26 = (nargs > 0) ? args[0] : px_null();
    LXValue _v27 = (nargs > 1) ? args[1] : px_null();
    LXValue _v28 = (nargs > 2) ? args[2] : px_null();
    LXValue _v29 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_30_val = px_null();
    int px_err_30_proped = 0;
    px_srcline(99);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){_v28}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){_v29}, 1)), px_str(": 词法错误 ")), _v26), px_str(": ")), _v27)}, 1));
    px_srcline(100);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_30:
    if (px_err_30_proped) return px_err_30_val;
    return px_null();
}

static LXValue fn_is_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_digit");
    LXValue _v31 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_32_val = px_null();
    int px_err_32_proped = 0;
    px_srcline(103);
    return ({ LXValue _t34 = ({ LXValue _t33 = px_ne(_v31, px_str("")); px_is_truthy(_t33) ? px_ge(_v31, px_str("0")) : _t33; }); px_is_truthy(_t34) ? px_le(_v31, px_str("9")) : _t34; });
px_err_32:
    if (px_err_32_proped) return px_err_32_val;
    return px_null();
}

static LXValue fn_is_hex_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_hex_digit");
    LXValue _v35 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_36_val = px_null();
    int px_err_36_proped = 0;
    px_srcline(105);
    return ({ LXValue _t40 = ({ LXValue _t38 = px_call(px_get_global("is_digit"), (LXValue[]){_v35}, 1); px_is_truthy(_t38) ? _t38 : ({ LXValue _t37 = px_ge(_v35, px_str("a")); px_is_truthy(_t37) ? px_le(_v35, px_str("f")) : _t37; }); }); px_is_truthy(_t40) ? _t40 : ({ LXValue _t39 = px_ge(_v35, px_str("A")); px_is_truthy(_t39) ? px_le(_v35, px_str("F")) : _t39; }); });
px_err_36:
    if (px_err_36_proped) return px_err_36_val;
    return px_null();
}

static LXValue fn_is_alnum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_alnum");
    LXValue _v41 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_42_val = px_null();
    int px_err_42_proped = 0;
    px_srcline(107);
    return ({ LXValue _t46 = ({ LXValue _t44 = px_call(px_get_global("is_digit"), (LXValue[]){_v41}, 1); px_is_truthy(_t44) ? _t44 : ({ LXValue _t43 = px_ge(_v41, px_str("a")); px_is_truthy(_t43) ? px_le(_v41, px_str("z")) : _t43; }); }); px_is_truthy(_t46) ? _t46 : ({ LXValue _t45 = px_ge(_v41, px_str("A")); px_is_truthy(_t45) ? px_le(_v41, px_str("Z")) : _t45; }); });
px_err_42:
    if (px_err_42_proped) return px_err_42_val;
    return px_null();
}

static LXValue fn_is_ident_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_ident_start");
    LXValue _v47 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_48_val = px_null();
    int px_err_48_proped = 0;
    px_srcline(109);
    if (px_is_truthy(px_eq(_v47, px_str("")))) {
        px_srcline(110);
        return px_bool(false);
    }
    px_srcline(111);
    return ({ LXValue _t53 = ({ LXValue _t52 = ({ LXValue _t51 = ({ LXValue _t49 = px_ge(_v47, px_str("a")); px_is_truthy(_t49) ? px_le(_v47, px_str("z")) : _t49; }); px_is_truthy(_t51) ? _t51 : ({ LXValue _t50 = px_ge(_v47, px_str("A")); px_is_truthy(_t50) ? px_le(_v47, px_str("Z")) : _t50; }); }); px_is_truthy(_t52) ? _t52 : px_eq(_v47, px_str("_")); }); px_is_truthy(_t53) ? _t53 : px_ge(_v47, px_str("")); });
px_err_48:
    if (px_err_48_proped) return px_err_48_val;
    return px_null();
}

static LXValue fn_is_ident_continue(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_ident_continue");
    LXValue _v54 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_55_val = px_null();
    int px_err_55_proped = 0;
    px_srcline(113);
    if (px_is_truthy(px_eq(_v54, px_str("")))) {
        px_srcline(114);
        return px_bool(false);
    }
    px_srcline(115);
    return ({ LXValue _t56 = px_call(px_get_global("is_ident_start"), (LXValue[]){_v54}, 1); px_is_truthy(_t56) ? _t56 : px_call(px_get_global("is_digit"), (LXValue[]){_v54}, 1); });
px_err_55:
    if (px_err_55_proped) return px_err_55_val;
    return px_null();
}

static LXValue fn_digit_val(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("digit_val");
    LXValue _v57 = (nargs > 0) ? args[0] : px_null();
    LXValue _v58 = px_null();
    LXValue _v59 = px_null();
    LXValue px_err_60_val = px_null();
    int px_err_60_proped = 0;
    px_srcline(117);
    _v58 = px_str("0123456789abcdefABCDEF");
    px_srcline(118);
    _v59 = px_int(0LL);
    px_srcline(119);
    while (px_is_truthy(px_lt(_v59, px_call(px_get_global("len"), (LXValue[]){_v58}, 1)))) {
        px_srcline(120);
        if (px_is_truthy(px_eq(px_index(_v58, _v59), _v57))) {
            px_srcline(121);
            if (px_is_truthy(px_ge(_v59, px_int(16LL)))) {
                px_srcline(122);
                return px_sub(_v59, px_int(6LL));
            }
            px_srcline(123);
            return _v59;
        }
        px_srcline(124);
         _v59 = px_add(_v59, px_int(1LL));
    }
    px_srcline(125);
    return px_neg(px_int(1LL));
px_err_60:
    if (px_err_60_proped) return px_err_60_val;
    return px_null();
}

static LXValue fn_handle_line_start(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("handle_line_start");
    LXValue _v61 = px_null();
    LXValue _v62 = px_null();
    LXValue _v63 = px_null();
    LXValue px_err_64_val = px_null();
    int px_err_64_proped = 0;
    px_srcline(128);
    _v61 = px_int(0LL);
    px_srcline(129);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(130);
         _v61 = px_int(0LL);
        px_srcline(131);
        while (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(" ")))) {
            px_srcline(132);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(133);
             _v61 = px_add(_v61, px_int(1LL));
        }
        px_srcline(134);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\t")))) {
            px_srcline(135);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1003"), px_str("缩进禁止使用 tab，请使用空格")}, 2));
        }
        px_srcline(136);
        _v62 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(137);
        if (px_is_truthy(px_eq(_v62, px_str("")))) {
            px_srcline(138);
            return px_null();
        }
        px_srcline(139);
        if (px_is_truthy(px_eq(_v62, px_str("\n")))) {
            px_srcline(140);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(141);
            continue;
        }
        px_srcline(142);
        if (px_is_truthy(px_eq(_v62, px_str("#")))) {
            px_srcline(143);
            (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
            px_srcline(144);
            continue;
        }
        px_srcline(145);
        break;
    }
    px_srcline(152);
    if (px_is_truthy(px_gt(px_get_global("g_bracket_depth"), px_int(0LL)))) {
        px_srcline(153);
        px_set_global("g_at_line_start", px_bool(false));
        px_srcline(154);
        return px_null();
    }
    px_srcline(155);
    _v63 = px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)));
    px_srcline(156);
    if (px_is_truthy(px_gt(_v61, _v63))) {
        px_srcline(157);
        (void)(px_method(px_get_global("g_indent_stack"), "append", (LXValue[]){_v61}, 1));
        px_srcline(158);
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("缩进"), px_str("")}, 2));
    }
    else if (px_is_truthy(px_lt(_v61, _v63))) {
        px_srcline(160);
        while (px_is_truthy(px_gt(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v61))) {
            px_srcline(161);
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            px_srcline(162);
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        px_srcline(163);
        if (px_is_truthy(px_ne(px_index(px_get_global("g_indent_stack"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL))), _v61))) {
            px_srcline(164);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E2002"), px_add(px_add(px_str("缩进不一致：当前缩进 "), px_call(px_get_global("str"), (LXValue[]){_v61}, 1)), px_str(" 与上层缩进不匹配"))}, 2));
        }
    }
    px_srcline(165);
    px_set_global("g_at_line_start", px_bool(false));
px_err_64:
    if (px_err_64_proped) return px_err_64_val;
    return px_null();
}

static LXValue fn_skip_comment(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_comment");
    LXValue _v65 = px_null();
    LXValue _v66 = px_null();
    LXValue _v67 = px_null();
    LXValue px_err_68_val = px_null();
    int px_err_68_proped = 0;
    px_srcline(168);
    _v65 = px_str("");
    px_srcline(169);
    if (px_is_truthy(px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")))) {
        px_srcline(170);
         _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(171);
         _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(172);
        _v66 = px_int(1LL);
        px_srcline(173);
        while (px_is_truthy(px_gt(_v66, px_int(0LL)))) {
            px_srcline(174);
            _v67 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            px_srcline(175);
            if (px_is_truthy(px_eq(_v67, px_str("")))) {
                px_srcline(176);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1002"), px_str("块注释未闭合（缺少 |#）")}, 2));
            }
            else if (px_is_truthy(({ LXValue _t69 = px_eq(_v67, px_str("#")); px_is_truthy(_t69) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")) : _t69; }))) {
                px_srcline(178);
                 _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(179);
                 _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(180);
                 _v66 = px_add(_v66, px_int(1LL));
            }
            else if (px_is_truthy(({ LXValue _t70 = px_eq(_v67, px_str("|")); px_is_truthy(_t70) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("#")) : _t70; }))) {
                px_srcline(182);
                 _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(183);
                 _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(184);
                 _v66 = px_sub(_v66, px_int(1LL));
            }
            else {
                px_srcline(186);
                 _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
        }
    }
    else {
        px_srcline(188);
        while (px_is_truthy(({ LXValue _t71 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t71) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\n")) : _t71; }))) {
            px_srcline(189);
             _v65 = px_add(_v65, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(190);
    return _v65;
px_err_68:
    if (px_err_68_proped) return px_err_68_val;
    return px_null();
}

static LXValue fn_scan_ident_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_ident_token");
    LXValue _v72 = px_null();
    LXValue _v73 = px_null();
    LXValue _v74 = px_null();
    LXValue px_err_75_val = px_null();
    int px_err_75_proped = 0;
    px_srcline(193);
    _v72 = px_get_global("g_line");
    px_srcline(194);
    _v73 = px_get_global("g_col");
    px_srcline(195);
    _v74 = px_str("");
    px_srcline(196);
    while (px_is_truthy(px_call(px_get_global("is_ident_continue"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
        px_srcline(197);
         _v74 = px_add(_v74, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(198);
    if (px_is_truthy(px_method(px_get_global("KEYWORDS"), "has", (LXValue[]){_v74}, 1))) {
        px_srcline(199);
        return px_list_n((LXValue[]){px_index(px_get_global("KEYWORDS"), _v74), px_str(""), _v72, _v73}, 4);
    }
    px_srcline(200);
    return px_list_n((LXValue[]){px_str("标识符"), _v74, _v72, _v73}, 4);
px_err_75:
    if (px_err_75_proped) return px_err_75_val;
    return px_null();
}

static LXValue fn_scan_radix_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_radix_token");
    LXValue _v76 = (nargs > 0) ? args[0] : px_null();
    LXValue _v77 = (nargs > 1) ? args[1] : px_null();
    LXValue _v78 = (nargs > 2) ? args[2] : px_null();
    LXValue _v79 = px_null();
    LXValue _v80 = px_null();
    LXValue _v81 = px_null();
    LXValue _v82 = px_null();
    LXValue _v83 = px_null();
    LXValue _v84 = px_null();
    LXValue px_err_85_val = px_null();
    int px_err_85_proped = 0;
    px_srcline(203);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(204);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(205);
    _v79 = px_str("");
    px_srcline(206);
    while (px_is_truthy(({ LXValue _t86 = px_call(px_get_global("is_alnum"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t86) ? _t86 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
        px_srcline(207);
         _v79 = px_add(_v79, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(208);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v79}, 1), px_int(0LL)))) {
        px_srcline(209);
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_str("进制字面量缺少数字（基数 "), px_call(px_get_global("str"), (LXValue[]){_v76}, 1)), px_str("）"))}, 2));
    }
    px_srcline(210);
    _v80 = px_str("");
    px_srcline(211);
    _v81 = px_int(0LL);
    px_srcline(212);
    while (px_is_truthy(px_lt(_v81, px_call(px_get_global("len"), (LXValue[]){_v79}, 1)))) {
        px_srcline(213);
        if (px_is_truthy(px_ne(px_index(_v79, _v81), px_str("_")))) {
            px_srcline(214);
             _v80 = px_add(_v80, px_index(_v79, _v81));
        }
        px_srcline(215);
         _v81 = px_add(_v81, px_int(1LL));
    }
    px_srcline(216);
    _v82 = px_int(0LL);
    px_srcline(217);
    _v83 = px_int(0LL);
    px_srcline(218);
    while (px_is_truthy(px_lt(_v83, px_call(px_get_global("len"), (LXValue[]){_v80}, 1)))) {
        px_srcline(219);
        _v84 = px_call(px_get_global("digit_val"), (LXValue[]){px_index(_v80, _v83)}, 1);
        px_srcline(220);
        if (px_is_truthy(({ LXValue _t87 = px_lt(_v84, px_int(0LL)); px_is_truthy(_t87) ? _t87 : px_ge(_v84, _v76); }))) {
            px_srcline(221);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_add(px_str("无效 "), px_call(px_get_global("str"), (LXValue[]){_v76}, 1)), px_str("-进制字面量: ")), _v80)}, 2));
        }
        px_srcline(222);
         _v82 = px_add(px_mul(_v82, _v76), _v84);
        px_srcline(223);
         _v83 = px_add(_v83, px_int(1LL));
    }
    px_srcline(224);
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){_v82}, 1), _v77, _v78}, 4);
px_err_85:
    if (px_err_85_proped) return px_err_85_val;
    return px_null();
}

static LXValue fn_strip_leading_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("strip_leading_zeros");
    LXValue _v88 = (nargs > 0) ? args[0] : px_null();
    LXValue _v89 = px_null();
    LXValue px_err_90_val = px_null();
    int px_err_90_proped = 0;
    px_srcline(226);
    _v89 = px_int(0LL);
    px_srcline(227);
    while (px_is_truthy(({ LXValue _t91 = px_lt(_v89, px_sub(px_call(px_get_global("len"), (LXValue[]){_v88}, 1), px_int(1LL))); px_is_truthy(_t91) ? px_eq(px_index(_v88, _v89), px_str("0")) : _t91; }))) {
        px_srcline(228);
         _v89 = px_add(_v89, px_int(1LL));
    }
    px_srcline(229);
    return px_slice(_v88, _v89, px_null(), px_null());
px_err_90:
    if (px_err_90_proped) return px_err_90_val;
    return px_null();
}

static LXValue fn_scan_number_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_number_token");
    LXValue _v92 = px_null();
    LXValue _v93 = px_null();
    LXValue _v94 = px_null();
    LXValue _v95 = px_null();
    LXValue _v96 = px_null();
    LXValue _v97 = px_null();
    LXValue _v98 = px_null();
    LXValue _v99 = px_null();
    LXValue _v100 = px_null();
    LXValue _v101 = px_null();
    LXValue _v102 = px_null();
    LXValue _v103 = px_null();
    LXValue _v104 = px_null();
    LXValue _v105 = px_null();
    LXValue px_err_106_val = px_null();
    int px_err_106_proped = 0;
    px_srcline(231);
    _v92 = px_get_global("g_line");
    px_srcline(232);
    _v93 = px_get_global("g_col");
    px_srcline(233);
    if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("0")))) {
        px_srcline(234);
        _v94 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
        px_srcline(235);
        if (px_is_truthy(({ LXValue _t107 = px_eq(_v94, px_str("x")); px_is_truthy(_t107) ? _t107 : px_eq(_v94, px_str("X")); }))) {
            px_srcline(236);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(16LL), _v92, _v93}, 3);
        }
        px_srcline(237);
        if (px_is_truthy(({ LXValue _t108 = px_eq(_v94, px_str("b")); px_is_truthy(_t108) ? _t108 : px_eq(_v94, px_str("B")); }))) {
            px_srcline(238);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(2LL), _v92, _v93}, 3);
        }
        px_srcline(239);
        if (px_is_truthy(({ LXValue _t109 = px_eq(_v94, px_str("o")); px_is_truthy(_t109) ? _t109 : px_eq(_v94, px_str("O")); }))) {
            px_srcline(240);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(8LL), _v92, _v93}, 3);
        }
    }
    px_srcline(241);
    _v95 = px_str("");
    px_srcline(242);
    while (px_is_truthy(({ LXValue _t110 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t110) ? _t110 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
        px_srcline(243);
         _v95 = px_add(_v95, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(244);
    _v96 = px_bool(false);
    px_srcline(245);
    if (px_is_truthy(({ LXValue _t111 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")); px_is_truthy(_t111) ? px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek2"), (LXValue[]){}, 0)}, 1) : _t111; }))) {
        px_srcline(246);
         _v96 = px_bool(true);
        px_srcline(247);
         _v95 = px_add(_v95, px_str("."));
        px_srcline(248);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(249);
        while (px_is_truthy(({ LXValue _t112 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t112) ? _t112 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
            px_srcline(250);
             _v95 = px_add(_v95, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(251);
    if (px_is_truthy(({ LXValue _t113 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("e")); px_is_truthy(_t113) ? _t113 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("E")); }))) {
        px_srcline(252);
        _v97 = px_get_global("g_pos");
        px_srcline(253);
        _v98 = px_get_global("g_line");
        px_srcline(254);
        _v99 = px_get_global("g_col");
        px_srcline(255);
        _v100 = px_str("");
        px_srcline(256);
         _v100 = px_add(_v100, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(257);
        if (px_is_truthy(({ LXValue _t114 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("+")); px_is_truthy(_t114) ? _t114 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")); }))) {
            px_srcline(258);
             _v100 = px_add(_v100, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        px_srcline(259);
        if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
            px_srcline(260);
            while (px_is_truthy(({ LXValue _t115 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t115) ? _t115 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
                px_srcline(261);
                 _v100 = px_add(_v100, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            px_srcline(262);
             _v95 = px_add(_v95, _v100);
            px_srcline(263);
             _v96 = px_bool(true);
        }
        else {
            px_srcline(265);
            px_set_global("g_pos", _v97);
            px_srcline(266);
            px_set_global("g_line", _v98);
            px_srcline(267);
            px_set_global("g_col", _v99);
        }
    }
    px_srcline(268);
    _v101 = px_str("");
    px_srcline(269);
    _v102 = px_int(0LL);
    px_srcline(270);
    while (px_is_truthy(px_lt(_v102, px_call(px_get_global("len"), (LXValue[]){_v95}, 1)))) {
        px_srcline(271);
        if (px_is_truthy(px_ne(px_index(_v95, _v102), px_str("_")))) {
            px_srcline(272);
             _v101 = px_add(_v101, px_index(_v95, _v102));
        }
        px_srcline(273);
         _v102 = px_add(_v102, px_int(1LL));
    }
    px_srcline(274);
    if (px_is_truthy(_v96)) {
        px_srcline(275);
        _v103 = px_call(px_get_global("float"), (LXValue[]){_v101}, 1);
        px_srcline(276);
        _v104 = px_call(px_get_global("str"), (LXValue[]){_v103}, 1);
        px_srcline(277);
        if (px_is_truthy(px_call(px_get_global("ends_with"), (LXValue[]){_v104, px_str(".0")}, 2))) {
            px_srcline(278);
             _v104 = px_slice(_v104, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v104}, 1), px_int(2LL)), px_null());
        }
        px_srcline(279);
        return px_list_n((LXValue[]){px_str("浮点"), _v104, _v92, _v93}, 4);
    }
    px_srcline(280);
    _v105 = px_call(px_get_global("strip_leading_zeros"), (LXValue[]){_v101}, 1);
    px_srcline(281);
    if (px_is_truthy(({ LXValue _t117 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v105}, 1), px_int(19LL)); px_is_truthy(_t117) ? _t117 : ({ LXValue _t116 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v105}, 1), px_int(19LL)); px_is_truthy(_t116) ? px_gt(_v105, px_str("9223372036854775807")) : _t116; }); }))) {
        px_srcline(282);
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_str("无效整数: "), _v95)}, 2));
    }
    px_srcline(283);
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v101}, 1)}, 1), _v92, _v93}, 4);
px_err_106:
    if (px_err_106_proped) return px_err_106_val;
    return px_null();
}

static LXValue fn_scan_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_string");
    LXValue _v118 = (nargs > 0) ? args[0] : px_null();
    LXValue _v119 = px_null();
    LXValue _v120 = px_null();
    LXValue px_err_121_val = px_null();
    int px_err_121_proped = 0;
    px_srcline(286);
    _v119 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){_v118}, 1);
    px_srcline(287);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_index(_v119, px_int(0LL))}, 1));
    px_srcline(288);
    _v120 = px_int(1LL);
    px_srcline(289);
    while (px_is_truthy(px_lt(_v120, px_call(px_get_global("len"), (LXValue[]){_v119}, 1)))) {
        px_srcline(290);
        (void)(px_method(px_get_global("g_pending"), "append", (LXValue[]){px_index(_v119, _v120)}, 1));
        px_srcline(291);
         _v120 = px_add(_v120, px_int(1LL));
    }
px_err_121:
    if (px_err_121_proped) return px_err_121_val;
    return px_null();
}

static LXValue fn_scan_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_string_tokens");
    LXValue _v122 = (nargs > 0) ? args[0] : px_null();
    LXValue _v123 = px_null();
    LXValue _v124 = px_null();
    LXValue _v125 = px_null();
    LXValue _v126 = px_null();
    LXValue _v127 = px_null();
    LXValue _v128 = px_null();
    LXValue _v129 = px_null();
    LXValue _v130 = px_null();
    LXValue px_err_131_val = px_null();
    int px_err_131_proped = 0;
    px_srcline(293);
    _v123 = px_get_global("g_line");
    px_srcline(294);
    _v124 = px_get_global("g_col");
    px_srcline(295);
    _v125 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    px_srcline(296);
    if (px_is_truthy(({ LXValue _t133 = ({ LXValue _t132 = px_eq(_v125, px_str("\"")); px_is_truthy(_t132) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\"")) : _t132; }); px_is_truthy(_t133) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")) : _t133; }))) {
        px_srcline(297);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(298);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(299);
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("\""), _v123, _v124, _v122}, 4);
    }
    px_srcline(300);
    if (px_is_truthy(({ LXValue _t135 = ({ LXValue _t134 = px_eq(_v125, px_str("'")); px_is_truthy(_t134) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("'")) : _t134; }); px_is_truthy(_t135) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")) : _t135; }))) {
        px_srcline(301);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(302);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(303);
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("'"), _v123, _v124, _v122}, 4);
    }
    px_srcline(304);
    _v126 = px_list_n((LXValue[]){}, 0);
    px_srcline(305);
    _v127 = px_str("");
    px_srcline(306);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(307);
        _v128 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(308);
        if (px_is_truthy(px_eq(_v128, px_str("")))) {
            px_srcline(309);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_add(px_add(px_str("字符串未闭合（缺少 "), _v125), px_str("）")), _v123, _v124}, 4));
        }
        px_srcline(310);
        if (px_is_truthy(px_eq(_v128, _v125))) {
            px_srcline(311);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(312);
            break;
        }
        px_srcline(313);
        if (px_is_truthy(px_eq(_v128, px_str("\\")))) {
            px_srcline(314);
             _v127 = px_add(_v127, px_call(px_get_global("scan_escape"), (LXValue[]){_v123, _v124}, 2));
        }
        else if (px_is_truthy(px_eq(_v128, px_str("\n")))) {
            px_srcline(316);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v123, _v124}, 4));
        }
        else if (px_is_truthy(({ LXValue _t137 = ({ LXValue _t136 = px_eq(_v128, px_str("$")); px_is_truthy(_t136) ? _v122 : _t136; }); px_is_truthy(_t137) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t137; }))) {
            px_srcline(318);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(319);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(320);
            (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v127}, 1), _v123, _v124}, 4)}, 1));
            px_srcline(321);
            (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v123, _v124}, 4)}, 1));
            px_srcline(322);
            (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v123, _v124}, 4)}, 1));
            px_srcline(323);
            (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v123, _v124}, 4)}, 1));
            px_srcline(324);
            _v129 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v123, _v124}, 2);
            px_srcline(325);
            _v130 = px_int(0LL);
            px_srcline(326);
            while (px_is_truthy(px_lt(_v130, px_call(px_get_global("len"), (LXValue[]){_v129}, 1)))) {
                px_srcline(327);
                (void)(px_method(_v126, "append", (LXValue[]){px_index(_v129, _v130)}, 1));
                px_srcline(328);
                 _v130 = px_add(_v130, px_int(1LL));
            }
            px_srcline(329);
            (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v123, _v124}, 4)}, 1));
            px_srcline(330);
            (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v123, _v124}, 4)}, 1));
            px_srcline(331);
             _v127 = px_str("");
        }
        else {
            px_srcline(333);
             _v127 = px_add(_v127, _v128);
            px_srcline(334);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(335);
    (void)(px_method(_v126, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v127}, 1), _v123, _v124}, 4)}, 1));
    px_srcline(336);
    return _v126;
px_err_131:
    if (px_err_131_proped) return px_err_131_val;
    return px_null();
}

static LXValue fn_scan_multiline_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_multiline_string_tokens");
    LXValue _v138 = (nargs > 0) ? args[0] : px_null();
    LXValue _v139 = (nargs > 1) ? args[1] : px_null();
    LXValue _v140 = (nargs > 2) ? args[2] : px_null();
    LXValue _v141 = (nargs > 3) ? args[3] : px_null();
    LXValue _v142 = px_null();
    LXValue _v143 = px_null();
    LXValue _v144 = px_null();
    LXValue _v145 = px_null();
    LXValue _v146 = px_null();
    LXValue px_err_147_val = px_null();
    int px_err_147_proped = 0;
    px_srcline(338);
    _v142 = px_list_n((LXValue[]){}, 0);
    px_srcline(339);
    _v143 = px_str("");
    px_srcline(340);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(341);
        _v144 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(342);
        if (px_is_truthy(px_eq(_v144, px_str("")))) {
            px_srcline(343);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("多行字符串未闭合"), _v139, _v140}, 4));
        }
        px_srcline(344);
        if (px_is_truthy(({ LXValue _t149 = ({ LXValue _t148 = px_eq(_v144, _v138); px_is_truthy(_t148) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), _v138) : _t148; }); px_is_truthy(_t149) ? px_eq(px_call(px_get_global("peek3"), (LXValue[]){}, 0), _v138) : _t149; }))) {
            px_srcline(345);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(346);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(347);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(348);
            break;
        }
        px_srcline(349);
        if (px_is_truthy(px_eq(_v144, px_str("\\")))) {
            px_srcline(350);
             _v143 = px_add(_v143, px_call(px_get_global("scan_escape"), (LXValue[]){_v139, _v140}, 2));
        }
        else if (px_is_truthy(({ LXValue _t151 = ({ LXValue _t150 = px_eq(_v144, px_str("$")); px_is_truthy(_t150) ? _v141 : _t150; }); px_is_truthy(_t151) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t151; }))) {
            px_srcline(352);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(353);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(354);
            (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v143}, 1), _v139, _v140}, 4)}, 1));
            px_srcline(355);
            (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v139, _v140}, 4)}, 1));
            px_srcline(356);
            (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v139, _v140}, 4)}, 1));
            px_srcline(357);
            (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v139, _v140}, 4)}, 1));
            px_srcline(358);
            _v145 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v139, _v140}, 2);
            px_srcline(359);
            _v146 = px_int(0LL);
            px_srcline(360);
            while (px_is_truthy(px_lt(_v146, px_call(px_get_global("len"), (LXValue[]){_v145}, 1)))) {
                px_srcline(361);
                (void)(px_method(_v142, "append", (LXValue[]){px_index(_v145, _v146)}, 1));
                px_srcline(362);
                 _v146 = px_add(_v146, px_int(1LL));
            }
            px_srcline(363);
            (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v139, _v140}, 4)}, 1));
            px_srcline(364);
            (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v139, _v140}, 4)}, 1));
            px_srcline(365);
             _v143 = px_str("");
        }
        else {
            px_srcline(367);
             _v143 = px_add(_v143, _v144);
            px_srcline(368);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(369);
    (void)(px_method(_v142, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v143}, 1), _v139, _v140}, 4)}, 1));
    px_srcline(370);
    return _v142;
px_err_147:
    if (px_err_147_proped) return px_err_147_val;
    return px_null();
}

static LXValue fn_scan_interp_nested_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_interp_nested_string");
    LXValue _v152 = (nargs > 0) ? args[0] : px_null();
    LXValue _v153 = (nargs > 1) ? args[1] : px_null();
    LXValue _v154 = px_null();
    LXValue _v155 = px_null();
    LXValue _v156 = px_null();
    LXValue _v157 = px_null();
    LXValue _v158 = px_null();
    LXValue px_err_159_val = px_null();
    int px_err_159_proped = 0;
    px_srcline(375);
    _v154 = px_get_global("g_line");
    px_srcline(376);
    _v155 = px_get_global("g_col");
    px_srcline(377);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(378);
    _v156 = px_str("");
    px_srcline(379);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(380);
        _v157 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(381);
        if (px_is_truthy(px_eq(_v157, px_str("")))) {
            px_srcline(382);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串未闭合（缺少 \"）"), _v152, _v153}, 4));
        }
        px_srcline(383);
        if (px_is_truthy(px_eq(_v157, px_str("\\")))) {
            px_srcline(384);
            _v158 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
            px_srcline(385);
            if (px_is_truthy(({ LXValue _t160 = px_eq(_v158, px_str("\"")); px_is_truthy(_t160) ? _t160 : px_eq(_v158, px_str("'")); }))) {
                px_srcline(386);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(387);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(388);
                break;
            }
            px_srcline(389);
            if (px_is_truthy(px_eq(_v158, px_str("\\")))) {
                px_srcline(390);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(391);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(392);
                 _v156 = px_add(_v156, px_str("\\"));
                px_srcline(393);
                continue;
            }
            px_srcline(394);
            if (px_is_truthy(px_eq(_v158, px_str("n")))) {
                px_srcline(395);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(396);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(397);
                 _v156 = px_add(_v156, px_str("\n"));
                px_srcline(398);
                continue;
            }
            px_srcline(399);
            if (px_is_truthy(px_eq(_v158, px_str("t")))) {
                px_srcline(400);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(401);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(402);
                 _v156 = px_add(_v156, px_str("\t"));
                px_srcline(403);
                continue;
            }
            px_srcline(404);
            if (px_is_truthy(px_eq(_v158, px_str("r")))) {
                px_srcline(405);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(406);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(407);
                 _v156 = px_add(_v156, px_str("\r"));
                px_srcline(408);
                continue;
            }
            px_srcline(409);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(410);
             _v156 = px_add(_v156, _v157);
            px_srcline(411);
            continue;
        }
        px_srcline(412);
        if (px_is_truthy(px_eq(_v157, px_str("\n")))) {
            px_srcline(413);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v152, _v153}, 4));
        }
        px_srcline(414);
         _v156 = px_add(_v156, _v157);
        px_srcline(415);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(416);
    return px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v156}, 1), _v154, _v155}, 4);
px_err_159:
    if (px_err_159_proped) return px_err_159_val;
    return px_null();
}

static LXValue fn_scan_interp_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_interp_expr");
    LXValue _v161 = (nargs > 0) ? args[0] : px_null();
    LXValue _v162 = (nargs > 1) ? args[1] : px_null();
    LXValue _v163 = px_null();
    LXValue _v164 = px_null();
    LXValue _v165 = px_null();
    LXValue _v166 = px_null();
    LXValue px_err_167_val = px_null();
    int px_err_167_proped = 0;
    px_srcline(418);
    _v163 = px_list_n((LXValue[]){}, 0);
    px_srcline(419);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(420);
        _v164 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(421);
        if (px_is_truthy(px_eq(_v164, px_str("")))) {
            px_srcline(422);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值 ${ 未闭合（缺少 }）"), _v161, _v162}, 4));
        }
        px_srcline(427);
        if (px_is_truthy(({ LXValue _t169 = px_eq(_v164, px_str("\\")); px_is_truthy(_t169) ? ({ LXValue _t168 = px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")); px_is_truthy(_t168) ? _t168 : px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")); }) : _t169; }))) {
            px_srcline(428);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(429);
            _v165 = px_call(px_get_global("scan_interp_nested_string"), (LXValue[]){_v161, _v162}, 2);
            px_srcline(430);
            (void)(px_method(_v163, "append", (LXValue[]){_v165}, 1));
            px_srcline(431);
            continue;
        }
        px_srcline(432);
        if (px_is_truthy(px_eq(_v164, px_str("}")))) {
            px_srcline(433);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(434);
            break;
        }
        px_srcline(435);
        if (px_is_truthy(px_eq(_v164, px_str("{")))) {
            px_srcline(436);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持 {} 字面量（dict/set），请先用变量保存")}, 2));
        }
        px_srcline(437);
        if (px_is_truthy(px_eq(_v164, px_str("\n")))) {
            px_srcline(438);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值表达式不能跨行"), _v161, _v162}, 4));
        }
        px_srcline(439);
        if (px_is_truthy(px_eq(_v164, px_str("#")))) {
            px_srcline(440);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持注释")}, 2));
        }
        px_srcline(441);
        if (px_is_truthy(({ LXValue _t170 = px_eq(_v164, px_str(" ")); px_is_truthy(_t170) ? _t170 : px_eq(_v164, px_str("\t")); }))) {
            px_srcline(442);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        else if (px_is_truthy(({ LXValue _t171 = px_eq(_v164, px_str("\"")); px_is_truthy(_t171) ? _t171 : px_eq(_v164, px_str("'")); }))) {
            px_srcline(444);
            _v165 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){px_bool(true)}, 1);
            px_srcline(445);
            _v166 = px_int(0LL);
            px_srcline(446);
            while (px_is_truthy(px_lt(_v166, px_call(px_get_global("len"), (LXValue[]){_v165}, 1)))) {
                px_srcline(447);
                (void)(px_method(_v163, "append", (LXValue[]){px_index(_v165, _v166)}, 1));
                px_srcline(448);
                 _v166 = px_add(_v166, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v164}, 1))) {
            px_srcline(450);
            (void)(px_method(_v163, "append", (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v164}, 1))) {
            px_srcline(452);
            (void)(px_method(_v163, "append", (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(454);
            (void)(px_method(_v163, "append", (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
        }
    }
    px_srcline(455);
    return _v163;
px_err_167:
    if (px_err_167_proped) return px_err_167_val;
    return px_null();
}

static LXValue fn_hex_to_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("hex_to_char");
    LXValue _v172 = (nargs > 0) ? args[0] : px_null();
    LXValue _v173 = px_null();
    LXValue _v174 = px_null();
    LXValue _v175 = px_null();
    LXValue _v176 = px_null();
    LXValue _v177 = px_null();
    LXValue px_err_178_val = px_null();
    int px_err_178_proped = 0;
    px_srcline(457);
    _v173 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v172}, 1);
    px_srcline(458);
    if (px_is_truthy(px_le(_v173, px_int(127LL)))) {
        px_srcline(459);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){_v173, px_int(1LL)}, 2)}, 1);
    }
    px_srcline(460);
    if (px_is_truthy(px_le(_v173, px_int(2047LL)))) {
        px_srcline(461);
        _v174 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(192LL), px_shr(_v173, px_int(6LL))), px_int(1LL)}, 2);
        px_srcline(462);
        _v175 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v173, px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(463);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v174, _v175}, 2)}, 1);
    }
    px_srcline(464);
    if (px_is_truthy(px_le(_v173, px_int(65535LL)))) {
        px_srcline(465);
        _v174 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(224LL), px_shr(_v173, px_int(12LL))), px_int(1LL)}, 2);
        px_srcline(466);
        _v175 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v173, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(467);
        _v176 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v173, px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(468);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v174, _v175, _v176}, 3)}, 1);
    }
    px_srcline(469);
    _v174 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(240LL), px_shr(_v173, px_int(18LL))), px_int(1LL)}, 2);
    px_srcline(470);
    _v175 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v173, px_int(12LL)), px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(471);
    _v176 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v173, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(472);
    _v177 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v173, px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(473);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v174, _v175, _v176, _v177}, 4)}, 1);
px_err_178:
    if (px_err_178_proped) return px_err_178_val;
    return px_null();
}

static LXValue fn_scan_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_escape");
    LXValue _v179 = (nargs > 0) ? args[0] : px_null();
    LXValue _v180 = (nargs > 1) ? args[1] : px_null();
    LXValue _v181 = px_null();
    LXValue _v182 = px_null();
    LXValue _v183 = px_null();
    LXValue _v184 = px_null();
    LXValue px_err_185_val = px_null();
    int px_err_185_proped = 0;
    px_srcline(475);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(476);
    _v181 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    px_srcline(477);
    if (px_is_truthy(px_eq(_v181, px_str("")))) {
        px_srcline(478);
        (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串在转义序列处意外结束"), _v179, _v180}, 4));
    }
    px_srcline(479);
    if (px_is_truthy(px_eq(_v181, px_str("n")))) {
        px_srcline(480);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(481);
        return px_str("\n");
    }
    px_srcline(482);
    if (px_is_truthy(px_eq(_v181, px_str("t")))) {
        px_srcline(483);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(484);
        return px_str("\t");
    }
    px_srcline(485);
    if (px_is_truthy(px_eq(_v181, px_str("r")))) {
        px_srcline(486);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(487);
        return px_str("\r");
    }
    px_srcline(488);
    if (px_is_truthy(px_eq(_v181, px_str("\\")))) {
        px_srcline(489);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(490);
        return px_str("\\");
    }
    px_srcline(491);
    if (px_is_truthy(px_eq(_v181, px_str("\"")))) {
        px_srcline(492);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(493);
        return px_str("\"");
    }
    px_srcline(494);
    if (px_is_truthy(px_eq(_v181, px_str("'")))) {
        px_srcline(495);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(496);
        return px_str("'");
    }
    px_srcline(497);
    if (px_is_truthy(px_eq(_v181, px_str("0")))) {
        px_srcline(498);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(499);
        return px_str("");
    }
    px_srcline(500);
    if (px_is_truthy(px_eq(_v181, px_str("$")))) {
        px_srcline(501);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(502);
        return px_str("$");
    }
    px_srcline(503);
    if (px_is_truthy(px_eq(_v181, px_str("u")))) {
        px_srcline(504);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(505);
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("{")))) {
            px_srcline(506);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义须为 \\u{XXXX} 形式")}, 2));
        }
        px_srcline(507);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(508);
        _v182 = px_str("");
        px_srcline(509);
        while (px_is_truthy(({ LXValue _t186 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t186) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")) : _t186; }))) {
            px_srcline(510);
            _v183 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            px_srcline(511);
            if (px_is_truthy(px_call(px_get_global("is_hex_digit"), (LXValue[]){_v183}, 1))) {
                px_srcline(512);
                 _v182 = px_add(_v182, _v183);
                px_srcline(513);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            else {
                px_srcline(515);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义含非法字符")}, 2));
            }
        }
        px_srcline(516);
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")))) {
            px_srcline(517);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义缺少 }")}, 2));
        }
        px_srcline(518);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(519);
        if (px_is_truthy(px_eq(_v182, px_str("")))) {
            px_srcline(520);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        px_srcline(521);
        _v184 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v182}, 1);
        px_srcline(522);
        if (px_is_truthy(({ LXValue _t187 = px_eq(_v184, px_null()); px_is_truthy(_t187) ? _t187 : px_gt(_v184, px_int(4294967295LL)); }))) {
            px_srcline(523);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        px_srcline(524);
        if (px_is_truthy(({ LXValue _t189 = px_gt(_v184, px_int(1114111LL)); px_is_truthy(_t189) ? _t189 : ({ LXValue _t188 = px_ge(_v184, px_int(55296LL)); px_is_truthy(_t188) ? px_le(_v184, px_int(57343LL)) : _t188; }); }))) {
            px_srcline(525);
            return px_call(px_get_global("hex_to_char"), (LXValue[]){px_str("FFFD")}, 1);
        }
        px_srcline(526);
        return px_call(px_get_global("hex_to_char"), (LXValue[]){_v182}, 1);
    }
    px_srcline(527);
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_add(px_str("非法转义序列 \\"), _v181)}, 2));
    px_srcline(528);
    return px_str("");
px_err_185:
    if (px_err_185_proped) return px_err_185_val;
    return px_null();
}

static LXValue fn_int_to_hex_nopad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("int_to_hex_nopad");
    LXValue _v190 = (nargs > 0) ? args[0] : px_null();
    LXValue _v191 = px_null();
    LXValue _v192 = px_null();
    LXValue px_err_193_val = px_null();
    int px_err_193_proped = 0;
    px_srcline(531);
    _v191 = px_str("0123456789abcdef");
    px_srcline(532);
    _v192 = px_str("");
    px_srcline(533);
    while (px_is_truthy(px_gt(_v190, px_int(0LL)))) {
        px_srcline(534);
         _v192 = px_add(px_index(_v191, px_mod(_v190, px_int(16LL))), _v192);
        px_srcline(535);
         _v190 = px_idiv(_v190, px_int(16LL));
    }
    px_srcline(536);
    if (px_is_truthy(px_eq(_v192, px_str("")))) {
        px_srcline(537);
        return px_str("0");
    }
    px_srcline(538);
    return _v192;
px_err_193:
    if (px_err_193_proped) return px_err_193_val;
    return px_null();
}

static LXValue fn_char_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("char_debug");
    LXValue _v194 = (nargs > 0) ? args[0] : px_null();
    LXValue _v195 = px_null();
    LXValue px_err_196_val = px_null();
    int px_err_196_proped = 0;
    px_srcline(540);
    if (px_is_truthy(px_eq(_v194, px_str("'")))) {
        px_srcline(541);
        return px_str("'\\''");
    }
    px_srcline(542);
    if (px_is_truthy(px_eq(_v194, px_str("\\")))) {
        px_srcline(543);
        return px_str("'\\\\'");
    }
    px_srcline(544);
    if (px_is_truthy(px_eq(_v194, px_str("\n")))) {
        px_srcline(545);
        return px_str("'\\n'");
    }
    px_srcline(546);
    if (px_is_truthy(px_eq(_v194, px_str("\r")))) {
        px_srcline(547);
        return px_str("'\\r'");
    }
    px_srcline(548);
    if (px_is_truthy(px_eq(_v194, px_str("\t")))) {
        px_srcline(549);
        return px_str("'\\t'");
    }
    px_srcline(550);
    if (px_is_truthy(px_eq(_v194, px_str("")))) {
        px_srcline(551);
        return px_str("'\\0'");
    }
    px_srcline(552);
    _v195 = px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v194}, 1);
    px_srcline(553);
    if (px_is_truthy(px_ge(_v195, px_int(0LL)))) {
        px_srcline(554);
        return px_add(px_add(px_str("'\\u{"), px_call(px_get_global("int_to_hex_nopad"), (LXValue[]){_v195}, 1)), px_str("}'"));
    }
    px_srcline(555);
    return px_add(px_add(px_str("'"), _v194), px_str("'"));
px_err_196:
    if (px_err_196_proped) return px_err_196_val;
    return px_null();
}

static LXValue fn_rust_str_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_str_debug");
    LXValue _v197 = (nargs > 0) ? args[0] : px_null();
    LXValue _v198 = px_null();
    LXValue _v199 = px_null();
    LXValue _v200 = px_null();
    LXValue px_err_201_val = px_null();
    int px_err_201_proped = 0;
    px_srcline(558);
    _v198 = px_str("\"");
    px_srcline(559);
    _v199 = px_int(0LL);
    px_srcline(560);
    while (px_is_truthy(px_lt(_v199, px_call(px_get_global("len"), (LXValue[]){_v197}, 1)))) {
        px_srcline(561);
        _v200 = px_index(_v197, _v199);
        px_srcline(562);
        if (px_is_truthy(px_eq(_v200, px_str("\n")))) {
            px_srcline(563);
             _v198 = px_add(_v198, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v200, px_str("\t")))) {
            px_srcline(565);
             _v198 = px_add(_v198, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v200, px_str("\r")))) {
            px_srcline(567);
             _v198 = px_add(_v198, px_str("\\r"));
        }
        else if (px_is_truthy(px_lt(_v200, px_str("")))) {
            px_srcline(569);
             _v198 = px_add(_v198, px_str("\\0"));
        }
        else if (px_is_truthy(px_eq(_v200, px_str("\"")))) {
            px_srcline(571);
             _v198 = px_add(_v198, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v200, px_str("\\")))) {
            px_srcline(573);
             _v198 = px_add(_v198, px_str("\\\\"));
        }
        else if (px_is_truthy(({ LXValue _t202 = px_ge(_v200, px_str(" ")); px_is_truthy(_t202) ? px_le(_v200, px_str("~")) : _t202; }))) {
            px_srcline(575);
             _v198 = px_add(_v198, _v200);
        }
        else if (px_is_truthy(px_eq(_v200, px_str(" ")))) {
            px_srcline(577);
             _v198 = px_add(_v198, px_str("\\u{a0}"));
        }
        else if (px_is_truthy(px_gt(_v200, px_str(" ")))) {
            px_srcline(579);
             _v198 = px_add(_v198, _v200);
        }
        else {
            px_srcline(581);
             _v198 = px_add(_v198, px_add(px_add(px_str("\\u{"), px_call(px_get_global("ctrl_hex"), (LXValue[]){_v200}, 1)), px_str("}")));
        }
        px_srcline(582);
         _v199 = px_add(_v199, px_int(1LL));
    }
    px_srcline(583);
     _v198 = px_add(_v198, px_str("\""));
    px_srcline(584);
    return _v198;
px_err_201:
    if (px_err_201_proped) return px_err_201_val;
    return px_null();
}

static LXValue fn_ctrl_codepoint(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ctrl_codepoint");
    LXValue _v203 = (nargs > 0) ? args[0] : px_null();
    LXValue _v204 = px_null();
    LXValue px_err_205_val = px_null();
    int px_err_205_proped = 0;
    px_srcline(586);
    _v204 = px_int(0LL);
    px_srcline(587);
    while (px_is_truthy(px_lt(_v204, px_call(px_get_global("len"), (LXValue[]){px_get_global("CTRL_ALL")}, 1)))) {
        px_srcline(588);
        if (px_is_truthy(px_eq(px_index(px_get_global("CTRL_ALL"), _v204), _v203))) {
            px_srcline(589);
            if (px_is_truthy(px_lt(_v204, px_int(28LL)))) {
                px_srcline(590);
                if (px_is_truthy(px_lt(_v204, px_int(8LL)))) {
                    px_srcline(591);
                    return px_add(_v204, px_int(1LL));
                }
                px_srcline(592);
                if (px_is_truthy(px_lt(_v204, px_int(10LL)))) {
                    px_srcline(593);
                    return px_add(_v204, px_int(3LL));
                }
                px_srcline(594);
                return px_add(_v204, px_int(4LL));
            }
            px_srcline(595);
            return px_add(_v204, px_int(99LL));
        }
        px_srcline(596);
         _v204 = px_add(_v204, px_int(1LL));
    }
    px_srcline(597);
    return px_neg(px_int(1LL));
px_err_205:
    if (px_err_205_proped) return px_err_205_val;
    return px_null();
}

static LXValue fn_ctrl_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ctrl_hex");
    LXValue _v206 = (nargs > 0) ? args[0] : px_null();
    LXValue _v207 = px_null();
    LXValue _v208 = px_null();
    LXValue px_err_209_val = px_null();
    int px_err_209_proped = 0;
    px_srcline(599);
    _v207 = px_call(px_get_global("int_to_hex"), (LXValue[]){px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v206}, 1), px_int(16LL)}, 2);
    px_srcline(600);
    _v208 = px_int(0LL);
    px_srcline(601);
    while (px_is_truthy(({ LXValue _t210 = px_lt(_v208, px_call(px_get_global("len"), (LXValue[]){_v207}, 1)); px_is_truthy(_t210) ? px_eq(px_index(_v207, _v208), px_str("0")) : _t210; }))) {
        px_srcline(602);
         _v208 = px_add(_v208, px_int(1LL));
    }
    px_srcline(603);
    if (px_is_truthy(px_eq(_v208, px_call(px_get_global("len"), (LXValue[]){_v207}, 1)))) {
        px_srcline(604);
        return px_str("0");
    }
    px_srcline(605);
    return px_slice(_v207, _v208, px_call(px_get_global("len"), (LXValue[]){_v207}, 1), px_null());
px_err_209:
    if (px_err_209_proped) return px_err_209_val;
    return px_null();
}

static LXValue fn_scan_operator_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_operator_token");
    LXValue _v211 = px_null();
    LXValue _v212 = px_null();
    LXValue _v213 = px_null();
    LXValue px_err_214_val = px_null();
    int px_err_214_proped = 0;
    px_srcline(608);
    _v211 = px_get_global("g_line");
    px_srcline(609);
    _v212 = px_get_global("g_col");
    px_srcline(610);
    _v213 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    px_srcline(611);
    if (px_is_truthy(px_eq(_v213, px_str("(")))) {
        px_srcline(612);
        return px_list_n((LXValue[]){px_str("("), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(613);
    if (px_is_truthy(px_eq(_v213, px_str(")")))) {
        px_srcline(614);
        return px_list_n((LXValue[]){px_str(")"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(615);
    if (px_is_truthy(px_eq(_v213, px_str("[")))) {
        px_srcline(616);
        return px_list_n((LXValue[]){px_str("["), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(617);
    if (px_is_truthy(px_eq(_v213, px_str("]")))) {
        px_srcline(618);
        return px_list_n((LXValue[]){px_str("]"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(619);
    if (px_is_truthy(px_eq(_v213, px_str("{")))) {
        px_srcline(620);
        return px_list_n((LXValue[]){px_str("{"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(621);
    if (px_is_truthy(px_eq(_v213, px_str("}")))) {
        px_srcline(622);
        return px_list_n((LXValue[]){px_str("}"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(623);
    if (px_is_truthy(px_eq(_v213, px_str(",")))) {
        px_srcline(624);
        return px_list_n((LXValue[]){px_str(","), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(625);
    if (px_is_truthy(px_eq(_v213, px_str(":")))) {
        px_srcline(626);
        return px_list_n((LXValue[]){px_str(":"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(627);
    if (px_is_truthy(px_eq(_v213, px_str(".")))) {
        px_srcline(628);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            px_srcline(629);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_str("运算符 '..' 未定义（range 语法尚未支持）")}, 2));
        }
        px_srcline(630);
        return px_list_n((LXValue[]){px_str("."), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(631);
    if (px_is_truthy(px_eq(_v213, px_str("+")))) {
        px_srcline(632);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(633);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(634);
            return px_list_n((LXValue[]){px_str("+="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(635);
        return px_list_n((LXValue[]){px_str("+"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(636);
    if (px_is_truthy(px_eq(_v213, px_str("-")))) {
        px_srcline(637);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(638);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(639);
            return px_list_n((LXValue[]){px_str("->"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(640);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(641);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(642);
            return px_list_n((LXValue[]){px_str("-="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(643);
        return px_list_n((LXValue[]){px_str("-"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(644);
    if (px_is_truthy(px_eq(_v213, px_str("*")))) {
        px_srcline(645);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("*")))) {
            px_srcline(646);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(647);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(648);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(649);
                return px_list_n((LXValue[]){px_str("**="), px_str(""), _v211, _v212}, 4);
            }
            px_srcline(650);
            return px_list_n((LXValue[]){px_str("**"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(651);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(652);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(653);
            return px_list_n((LXValue[]){px_str("*="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(654);
        return px_list_n((LXValue[]){px_str("*"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(655);
    if (px_is_truthy(px_eq(_v213, px_str("/")))) {
        px_srcline(656);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("/")))) {
            px_srcline(657);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(658);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(659);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(660);
                return px_list_n((LXValue[]){px_str("//="), px_str(""), _v211, _v212}, 4);
            }
            px_srcline(661);
            return px_list_n((LXValue[]){px_str("//"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(662);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(663);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(664);
            return px_list_n((LXValue[]){px_str("/="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(665);
        return px_list_n((LXValue[]){px_str("/"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(666);
    if (px_is_truthy(px_eq(_v213, px_str("%")))) {
        px_srcline(667);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(668);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(669);
            return px_list_n((LXValue[]){px_str("%="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(670);
        return px_list_n((LXValue[]){px_str("%"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(671);
    if (px_is_truthy(px_eq(_v213, px_str("^")))) {
        px_srcline(672);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(673);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(674);
            return px_list_n((LXValue[]){px_str("^="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(675);
        return px_list_n((LXValue[]){px_str("^"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(676);
    if (px_is_truthy(px_eq(_v213, px_str("~")))) {
        px_srcline(677);
        return px_list_n((LXValue[]){px_str("~"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(678);
    if (px_is_truthy(px_eq(_v213, px_str("&")))) {
        px_srcline(679);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(680);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(681);
            return px_list_n((LXValue[]){px_str("&="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(682);
        return px_list_n((LXValue[]){px_str("&"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(683);
    if (px_is_truthy(px_eq(_v213, px_str("|")))) {
        px_srcline(684);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(685);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(686);
            return px_list_n((LXValue[]){px_str("|>"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(687);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(688);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(689);
            return px_list_n((LXValue[]){px_str("|="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(690);
        return px_list_n((LXValue[]){px_str("|"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(691);
    if (px_is_truthy(px_eq(_v213, px_str("=")))) {
        px_srcline(692);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(693);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(694);
            return px_list_n((LXValue[]){px_str("=="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(695);
        return px_list_n((LXValue[]){px_str("="), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(696);
    if (px_is_truthy(px_eq(_v213, px_str("!")))) {
        px_srcline(697);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(698);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(699);
            return px_list_n((LXValue[]){px_str("!="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(700);
        return px_list_n((LXValue[]){px_str("!"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(701);
    if (px_is_truthy(px_eq(_v213, px_str("<")))) {
        px_srcline(702);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")))) {
            px_srcline(705);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(706);
            return px_list_n((LXValue[]){px_str("<-"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(707);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("<")))) {
            px_srcline(708);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(709);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(710);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(711);
                return px_list_n((LXValue[]){px_str("<<="), px_str(""), _v211, _v212}, 4);
            }
            px_srcline(712);
            return px_list_n((LXValue[]){px_str("<<"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(713);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(714);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(715);
            return px_list_n((LXValue[]){px_str("<="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(716);
        return px_list_n((LXValue[]){px_str("<"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(717);
    if (px_is_truthy(px_eq(_v213, px_str(">")))) {
        px_srcline(718);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(719);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(720);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
                px_srcline(721);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(722);
                if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                    px_srcline(723);
                    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                    px_srcline(724);
                    return px_list_n((LXValue[]){px_str(">>>="), px_str(""), _v211, _v212}, 4);
                }
                px_srcline(725);
                return px_list_n((LXValue[]){px_str(">>>"), px_str(""), _v211, _v212}, 4);
            }
            px_srcline(726);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(727);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(728);
                return px_list_n((LXValue[]){px_str(">>="), px_str(""), _v211, _v212}, 4);
            }
            px_srcline(729);
            return px_list_n((LXValue[]){px_str(">>"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(730);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(731);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(732);
            return px_list_n((LXValue[]){px_str(">="), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(733);
        return px_list_n((LXValue[]){px_str(">"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(734);
    if (px_is_truthy(px_eq(_v213, px_str("?")))) {
        px_srcline(735);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            px_srcline(736);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(737);
            return px_list_n((LXValue[]){px_str("?."), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(738);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("?")))) {
            px_srcline(739);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(740);
            return px_list_n((LXValue[]){px_str("??"), px_str(""), _v211, _v212}, 4);
        }
        px_srcline(741);
        return px_list_n((LXValue[]){px_str("?"), px_str(""), _v211, _v212}, 4);
    }
    px_srcline(742);
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_add(px_str("非法字符: "), px_call(px_get_global("char_debug"), (LXValue[]){_v213}, 1))}, 2));
    px_srcline(743);
    return px_list_n((LXValue[]){px_str(""), px_str(""), _v211, _v212}, 4);
px_err_214:
    if (px_err_214_proped) return px_err_214_val;
    return px_null();
}

static LXValue fn_next_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("next_token");
    LXValue _v215 = px_null();
    LXValue _v216 = px_null();
    LXValue _v217 = px_null();
    LXValue _v218 = px_null();
    LXValue px_err_219_val = px_null();
    int px_err_219_proped = 0;
    px_srcline(746);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_int(0LL)))) {
        px_srcline(747);
        _v215 = px_index(px_get_global("g_pending"), px_int(0LL));
        px_srcline(748);
        px_set_global("g_pending", px_slice(px_get_global("g_pending"), px_int(1LL), px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_null()));
        px_srcline(749);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){_v215}, 1));
        px_srcline(750);
        return px_bool(true);
    }
    px_srcline(751);
    if (px_is_truthy(px_get_global("g_at_line_start"))) {
        px_srcline(752);
        (void)(px_call(px_get_global("handle_line_start"), (LXValue[]){}, 0));
    }
    px_srcline(753);
    _v216 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    px_srcline(754);
    if (px_is_truthy(px_eq(_v216, px_str("")))) {
        px_srcline(755);
        while (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)))) {
            px_srcline(756);
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            px_srcline(757);
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        px_srcline(758);
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("EOF"), px_str("")}, 2));
        px_srcline(759);
        return px_bool(false);
    }
    px_srcline(760);
    if (px_is_truthy(px_eq(_v216, px_str("\n")))) {
        px_srcline(770);
        if (px_is_truthy(({ LXValue _t220 = px_eq(px_get_global("g_bracket_depth"), px_int(0LL)); px_is_truthy(_t220) ? px_call(px_get_global("is_cont_op"), (LXValue[]){px_get_global("g_last_kind")}, 1) : _t220; }))) {
            px_srcline(771);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(772);
            return px_bool(true);
        }
        px_srcline(773);
        _v217 = px_get_global("g_line");
        px_srcline(774);
        _v218 = px_get_global("g_col");
        px_srcline(775);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(776);
        px_set_global("g_at_line_start", px_bool(true));
        px_srcline(777);
        (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_str("换行"), px_str(""), _v217, _v218}, 4));
        px_srcline(778);
        return px_bool(true);
    }
    px_srcline(779);
    if (px_is_truthy(({ LXValue _t221 = px_eq(_v216, px_str(" ")); px_is_truthy(_t221) ? _t221 : px_eq(_v216, px_str("\t")); }))) {
        px_srcline(780);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(781);
        return px_bool(true);
    }
    px_srcline(782);
    if (px_is_truthy(px_eq(_v216, px_str("#")))) {
        px_srcline(783);
        (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
        px_srcline(784);
        return px_bool(true);
    }
    px_srcline(785);
    if (px_is_truthy(({ LXValue _t222 = px_eq(_v216, px_str("\"")); px_is_truthy(_t222) ? _t222 : px_eq(_v216, px_str("'")); }))) {
        px_srcline(786);
        (void)(px_call(px_get_global("scan_string"), (LXValue[]){px_bool(true)}, 1));
        px_srcline(787);
        return px_bool(true);
    }
    px_srcline(788);
    if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v216}, 1))) {
        px_srcline(789);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        px_srcline(790);
        return px_bool(true);
    }
    px_srcline(791);
    if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v216}, 1))) {
        px_srcline(792);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        px_srcline(793);
        return px_bool(true);
    }
    px_srcline(794);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
    px_srcline(795);
    return px_bool(true);
px_err_219:
    if (px_err_219_proped) return px_err_219_val;
    return px_null();
}

static LXValue fn_check_edition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("check_edition");
    LXValue _v223 = (nargs > 0) ? args[0] : px_null();
    LXValue _v224 = px_null();
    LXValue _v225 = px_null();
    LXValue _v226 = px_null();
    LXValue _v227 = px_null();
    LXValue _v228 = px_null();
    LXValue _v229 = px_null();
    LXValue _v230 = px_null();
    LXValue px_err_231_val = px_null();
    int px_err_231_proped = 0;
    px_srcline(800);
    _v224 = px_int(0LL);
    px_srcline(801);
    _v225 = px_call(px_get_global("len"), (LXValue[]){_v223}, 1);
    px_srcline(802);
    _v226 = px_str("");
    px_srcline(803);
    while (px_is_truthy(({ LXValue _t232 = px_lt(_v224, _v225); px_is_truthy(_t232) ? px_ne(px_index(_v223, _v224), px_str("\n")) : _t232; }))) {
        px_srcline(804);
         _v226 = px_add(_v226, px_index(_v223, _v224));
        px_srcline(805);
         _v224 = px_add(_v224, px_int(1LL));
    }
    px_srcline(806);
    _v227 = px_call(px_get_global("trim"), (LXValue[]){_v226}, 1);
    px_srcline(807);
    _v228 = px_method(_v227, "split", (LXValue[]){px_str(" ")}, 1);
    px_srcline(808);
    if (px_is_truthy(({ LXValue _t234 = ({ LXValue _t233 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v228}, 1), px_int(3LL)); px_is_truthy(_t233) ? px_eq(px_index(_v228, px_int(0LL)), px_str("#")) : _t233; }); px_is_truthy(_t234) ? px_eq(px_index(_v228, px_int(1LL)), px_str("px")) : _t234; }))) {
        px_srcline(809);
        _v229 = px_index(_v228, px_int(2LL));
        px_srcline(810);
        if (px_is_truthy(({ LXValue _t238 = ({ LXValue _t237 = ({ LXValue _t236 = ({ LXValue _t235 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v229}, 1), px_int(4LL)); px_is_truthy(_t235) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v229, px_int(0LL))}, 1) : _t235; }); px_is_truthy(_t236) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v229, px_int(1LL))}, 1) : _t236; }); px_is_truthy(_t237) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v229, px_int(2LL))}, 1) : _t237; }); px_is_truthy(_t238) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v229, px_int(3LL))}, 1) : _t238; }))) {
            px_srcline(811);
            _v230 = px_call(px_get_global("int"), (LXValue[]){_v229}, 1);
            px_srcline(812);
            if (px_is_truthy(px_gt(_v230, px_int(2026LL)))) {
                px_srcline(813);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E-EDITION"), px_add(px_add(px_str("源码声明 edition px "), _v229), px_str(" 高于当前工具链支持（px 2026），请升级编译器"))}, 2));
            }
        }
    }
px_err_231:
    if (px_err_231_proped) return px_err_231_val;
    return px_null();
}

static LXValue fn_lex_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("lex_tokens");
    LXValue _v239 = (nargs > 0) ? args[0] : px_null();
    LXValue _v240 = px_null();
    LXValue px_err_241_val = px_null();
    int px_err_241_proped = 0;
    px_srcline(816);
    (void)(px_call(px_get_global("check_edition"), (LXValue[]){_v239}, 1));
    px_srcline(817);
    px_set_global("g_src", _v239);
    px_srcline(818);
    px_set_global("g_len", px_call(px_get_global("len"), (LXValue[]){_v239}, 1));
    px_srcline(819);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(820);
    px_set_global("g_line", px_int(1LL));
    px_srcline(821);
    px_set_global("g_col", px_int(1LL));
    px_srcline(822);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(823);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(824);
    px_set_global("g_bracket_depth", px_int(0LL));
    px_srcline(825);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(826);
    px_set_global("g_count", px_int(0LL));
    px_srcline(827);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(828);
    _v240 = px_bool(true);
    px_srcline(829);
    while (px_is_truthy(_v240)) {
        px_srcline(830);
         _v240 = px_call(px_get_global("next_token"), (LXValue[]){}, 0);
    }
    px_srcline(831);
    return px_get_global("g_toks");
px_err_241:
    if (px_err_241_proped) return px_err_241_val;
    return px_null();
}

static LXValue fn_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pad");
    LXValue _v242 = (nargs > 0) ? args[0] : px_null();
    LXValue _v243 = px_null();
    LXValue _v244 = px_null();
    LXValue px_err_245_val = px_null();
    int px_err_245_proped = 0;
    px_srcline(20);
    _v243 = px_str("");
    px_srcline(21);
    _v244 = px_int(0LL);
    px_srcline(22);
    while (px_is_truthy(px_lt(_v244, _v242))) {
        px_srcline(23);
         _v243 = px_add(_v243, px_str(" "));
        px_srcline(24);
         _v244 = px_add(_v244, px_int(1LL));
    }
    px_srcline(25);
    return _v243;
px_err_245:
    if (px_err_245_proped) return px_err_245_val;
    return px_null();
}

static LXValue fn_dump_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_node");
    LXValue _v246 = (nargs > 0) ? args[0] : px_null();
    LXValue _v247 = (nargs > 1) ? args[1] : px_null();
    LXValue _v248 = px_null();
    LXValue _v249 = px_null();
    LXValue _v250 = px_null();
    LXValue _v251 = px_null();
    LXValue _v252 = px_null();
    LXValue _v253 = px_null();
    LXValue _v254 = px_null();
    LXValue _v255 = px_null();
    LXValue _v256 = px_null();
    LXValue _v257 = px_null();
    LXValue px_err_258_val = px_null();
    int px_err_258_proped = 0;
    px_srcline(27);
    _v248 = px_index(_v246, px_int(0LL));
    px_srcline(28);
    _v249 = px_index(px_get_global("LAYOUT"), _v248);
    px_srcline(29);
    _v250 = px_index(_v249, px_int(0LL));
    px_srcline(30);
    _v251 = px_index(_v249, px_int(1LL));
    px_srcline(31);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v251}, 1), px_int(0LL)))) {
        px_srcline(32);
        return _v250;
    }
    px_srcline(33);
    _v252 = px_eq(px_index(px_index(_v251, px_int(0LL)), px_int(0LL)), px_null());
    px_srcline(34);
    _v253 = px_list_n((LXValue[]){}, 0);
    px_srcline(35);
    if (px_is_truthy(_v252)) {
        px_srcline(36);
        (void)(px_method(_v253, "append", (LXValue[]){px_add(_v250, px_str("("))}, 1));
    }
    else {
        px_srcline(38);
        (void)(px_method(_v253, "append", (LXValue[]){px_add(_v250, px_str(" {"))}, 1));
    }
    px_srcline(39);
    _v254 = px_int(0LL);
    px_srcline(40);
    while (px_is_truthy(px_lt(_v254, px_call(px_get_global("len"), (LXValue[]){_v251}, 1)))) {
        px_srcline(41);
        _v255 = px_index(_v251, _v254);
        px_srcline(42);
        _v256 = px_index(_v246, px_add(_v254, px_int(1LL)));
        px_srcline(43);
        _v257 = px_call(px_get_global("dump_field"), (LXValue[]){_v256, px_index(_v255, px_int(1LL)), px_add(_v247, px_int(4LL))}, 3);
        px_srcline(44);
        if (px_is_truthy(_v252)) {
            px_srcline(45);
            (void)(px_method(_v253, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v247, px_int(4LL))}, 1), _v257), px_str(","))}, 1));
        }
        else {
            px_srcline(47);
            (void)(px_method(_v253, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v247, px_int(4LL))}, 1), px_index(_v255, px_int(0LL))), px_str(": ")), _v257), px_str(","))}, 1));
        }
        px_srcline(48);
         _v254 = px_add(_v254, px_int(1LL));
    }
    px_srcline(49);
    if (px_is_truthy(_v252)) {
        px_srcline(50);
        (void)(px_method(_v253, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v247}, 1), px_str(")"))}, 1));
    }
    else {
        px_srcline(52);
        (void)(px_method(_v253, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v247}, 1), px_str("}"))}, 1));
    }
    px_srcline(53);
    return px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v253}, 2);
px_err_258:
    if (px_err_258_proped) return px_err_258_val;
    return px_null();
}

static LXValue fn_dump_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_list");
    LXValue _v259 = (nargs > 0) ? args[0] : px_null();
    LXValue _v260 = (nargs > 1) ? args[1] : px_null();
    LXValue _v261 = px_null();
    LXValue _v262 = px_null();
    LXValue px_err_263_val = px_null();
    int px_err_263_proped = 0;
    px_srcline(55);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v259}, 1), px_int(0LL)))) {
        px_srcline(56);
        return px_str("[]");
    }
    px_srcline(57);
    _v261 = px_list_n((LXValue[]){}, 0);
    px_srcline(58);
    _v262 = px_int(0LL);
    px_srcline(59);
    while (px_is_truthy(px_lt(_v262, px_call(px_get_global("len"), (LXValue[]){_v259}, 1)))) {
        px_srcline(60);
        (void)(px_method(_v261, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v260, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v259, _v262), px_add(_v260, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(61);
         _v262 = px_add(_v262, px_int(1LL));
    }
    px_srcline(62);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v261}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v260}, 1)), px_str("]"));
px_err_263:
    if (px_err_263_proped) return px_err_263_val;
    return px_null();
}

static LXValue fn_dump_str_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_str_list");
    LXValue _v264 = (nargs > 0) ? args[0] : px_null();
    LXValue _v265 = (nargs > 1) ? args[1] : px_null();
    LXValue _v266 = px_null();
    LXValue _v267 = px_null();
    LXValue px_err_268_val = px_null();
    int px_err_268_proped = 0;
    px_srcline(64);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v264}, 1), px_int(0LL)))) {
        px_srcline(65);
        return px_str("[]");
    }
    px_srcline(66);
    _v266 = px_list_n((LXValue[]){}, 0);
    px_srcline(67);
    _v267 = px_int(0LL);
    px_srcline(68);
    while (px_is_truthy(px_lt(_v267, px_call(px_get_global("len"), (LXValue[]){_v264}, 1)))) {
        px_srcline(69);
        (void)(px_method(_v266, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v265, px_int(4LL))}, 1), px_index(_v264, _v267)), px_str(","))}, 1));
        px_srcline(70);
         _v267 = px_add(_v267, px_int(1LL));
    }
    px_srcline(71);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v266}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v265}, 1)), px_str("]"));
px_err_268:
    if (px_err_268_proped) return px_err_268_val;
    return px_null();
}

static LXValue fn_dump_ty_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_ty_list");
    LXValue _v269 = (nargs > 0) ? args[0] : px_null();
    LXValue _v270 = (nargs > 1) ? args[1] : px_null();
    LXValue _v271 = px_null();
    LXValue _v272 = px_null();
    LXValue px_err_273_val = px_null();
    int px_err_273_proped = 0;
    px_srcline(73);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v269}, 1), px_int(0LL)))) {
        px_srcline(74);
        return px_str("[]");
    }
    px_srcline(75);
    _v271 = px_list_n((LXValue[]){}, 0);
    px_srcline(76);
    _v272 = px_int(0LL);
    px_srcline(77);
    while (px_is_truthy(px_lt(_v272, px_call(px_get_global("len"), (LXValue[]){_v269}, 1)))) {
        px_srcline(78);
        (void)(px_method(_v271, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v270, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v269, _v272), px_add(_v270, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(79);
         _v272 = px_add(_v272, px_int(1LL));
    }
    px_srcline(80);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v271}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v270}, 1)), px_str("]"));
px_err_273:
    if (px_err_273_proped) return px_err_273_val;
    return px_null();
}

static LXValue fn_dump_pat_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_pat_list");
    LXValue _v274 = (nargs > 0) ? args[0] : px_null();
    LXValue _v275 = (nargs > 1) ? args[1] : px_null();
    LXValue _v276 = px_null();
    LXValue _v277 = px_null();
    LXValue px_err_278_val = px_null();
    int px_err_278_proped = 0;
    px_srcline(82);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v274}, 1), px_int(0LL)))) {
        px_srcline(83);
        return px_str("[]");
    }
    px_srcline(84);
    _v276 = px_list_n((LXValue[]){}, 0);
    px_srcline(85);
    _v277 = px_int(0LL);
    px_srcline(86);
    while (px_is_truthy(px_lt(_v277, px_call(px_get_global("len"), (LXValue[]){_v274}, 1)))) {
        px_srcline(87);
        (void)(px_method(_v276, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v275, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v274, _v277), px_add(_v275, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(88);
         _v277 = px_add(_v277, px_int(1LL));
    }
    px_srcline(89);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v276}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v275}, 1)), px_str("]"));
px_err_278:
    if (px_err_278_proped) return px_err_278_val;
    return px_null();
}

static LXValue fn_dump_opt_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_node");
    LXValue _v279 = (nargs > 0) ? args[0] : px_null();
    LXValue _v280 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_281_val = px_null();
    int px_err_281_proped = 0;
    px_srcline(91);
    if (px_is_truthy(px_eq(_v279, px_null()))) {
        px_srcline(92);
        return px_str("None");
    }
    px_srcline(93);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v280, px_int(4LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){_v279, px_add(_v280, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v280}, 1)), px_str(")"));
px_err_281:
    if (px_err_281_proped) return px_err_281_val;
    return px_null();
}

static LXValue fn_dump_opt_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_str");
    LXValue _v282 = (nargs > 0) ? args[0] : px_null();
    LXValue _v283 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_284_val = px_null();
    int px_err_284_proped = 0;
    px_srcline(95);
    if (px_is_truthy(px_eq(_v282, px_null()))) {
        px_srcline(96);
        return px_str("None");
    }
    px_srcline(97);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v283, px_int(4LL))}, 1)), _v282), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v283}, 1)), px_str(")"));
px_err_284:
    if (px_err_284_proped) return px_err_284_val;
    return px_null();
}

static LXValue fn_dump_opt_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_list");
    LXValue _v285 = (nargs > 0) ? args[0] : px_null();
    LXValue _v286 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_287_val = px_null();
    int px_err_287_proped = 0;
    px_srcline(99);
    if (px_is_truthy(px_eq(_v285, px_null()))) {
        px_srcline(100);
        return px_str("None");
    }
    px_srcline(101);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v286, px_int(4LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){_v285, px_add(_v286, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v286}, 1)), px_str(")"));
px_err_287:
    if (px_err_287_proped) return px_err_287_val;
    return px_null();
}

static LXValue fn_dump_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_pos");
    LXValue _v288 = (nargs > 0) ? args[0] : px_null();
    LXValue _v289 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_290_val = px_null();
    int px_err_290_proped = 0;
    px_srcline(103);
    return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v289, px_int(4LL))}, 1)), px_str("line: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v288, px_int(0LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v289, px_int(4LL))}, 1)), px_str("col: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v288, px_int(1LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v289}, 1)), px_str("}"));
px_err_290:
    if (px_err_290_proped) return px_err_290_val;
    return px_null();
}

static LXValue fn_dump_t2_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t2_list");
    LXValue _v291 = (nargs > 0) ? args[0] : px_null();
    LXValue _v292 = (nargs > 1) ? args[1] : px_null();
    LXValue _v293 = px_null();
    LXValue _v294 = px_null();
    LXValue _v295 = px_null();
    LXValue px_err_296_val = px_null();
    int px_err_296_proped = 0;
    px_srcline(106);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v291}, 1), px_int(0LL)))) {
        px_srcline(107);
        return px_str("[]");
    }
    px_srcline(108);
    _v293 = px_list_n((LXValue[]){}, 0);
    px_srcline(109);
    _v294 = px_int(0LL);
    px_srcline(110);
    while (px_is_truthy(px_lt(_v294, px_call(px_get_global("len"), (LXValue[]){_v291}, 1)))) {
        px_srcline(111);
        _v295 = px_index(_v291, _v294);
        px_srcline(112);
        (void)(px_method(_v293, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v292, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v292, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v295, px_int(0LL)), px_add(_v292, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v292, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v295, px_int(1LL)), px_add(_v292, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v292, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(113);
         _v294 = px_add(_v294, px_int(1LL));
    }
    px_srcline(114);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v293}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v292}, 1)), px_str("]"));
px_err_296:
    if (px_err_296_proped) return px_err_296_val;
    return px_null();
}

static LXValue fn_dump_t2b_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t2b_list");
    LXValue _v297 = (nargs > 0) ? args[0] : px_null();
    LXValue _v298 = (nargs > 1) ? args[1] : px_null();
    LXValue _v299 = px_null();
    LXValue _v300 = px_null();
    LXValue _v301 = px_null();
    LXValue px_err_302_val = px_null();
    int px_err_302_proped = 0;
    px_srcline(117);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v297}, 1), px_int(0LL)))) {
        px_srcline(118);
        return px_str("[]");
    }
    px_srcline(119);
    _v299 = px_list_n((LXValue[]){}, 0);
    px_srcline(120);
    _v300 = px_int(0LL);
    px_srcline(121);
    while (px_is_truthy(px_lt(_v300, px_call(px_get_global("len"), (LXValue[]){_v297}, 1)))) {
        px_srcline(122);
        _v301 = px_index(_v297, _v300);
        px_srcline(123);
        (void)(px_method(_v299, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v298, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v298, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v301, px_int(0LL)), px_add(_v298, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v298, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v301, px_int(1LL)), px_add(_v298, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v298, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(124);
         _v300 = px_add(_v300, px_int(1LL));
    }
    px_srcline(125);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v299}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v298}, 1)), px_str("]"));
px_err_302:
    if (px_err_302_proped) return px_err_302_val;
    return px_null();
}

static LXValue fn_dump_t3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t3_list");
    LXValue _v303 = (nargs > 0) ? args[0] : px_null();
    LXValue _v304 = (nargs > 1) ? args[1] : px_null();
    LXValue _v305 = px_null();
    LXValue _v306 = px_null();
    LXValue _v307 = px_null();
    LXValue _v308 = px_null();
    LXValue _v309 = px_null();
    LXValue px_err_310_val = px_null();
    int px_err_310_proped = 0;
    px_srcline(128);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v303}, 1), px_int(0LL)))) {
        px_srcline(129);
        return px_str("[]");
    }
    px_srcline(130);
    _v305 = px_list_n((LXValue[]){}, 0);
    px_srcline(131);
    _v306 = px_int(0LL);
    px_srcline(132);
    while (px_is_truthy(px_lt(_v306, px_call(px_get_global("len"), (LXValue[]){_v303}, 1)))) {
        px_srcline(133);
        _v307 = px_index(_v303, _v306);
        px_srcline(134);
        _v308 = px_index(_v307, px_int(0LL));
        px_srcline(135);
        _v309 = px_str("None");
        px_srcline(136);
        if (px_is_truthy(px_ne(_v308, px_null()))) {
            px_srcline(137);
             _v309 = px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(12LL))}, 1)), _v308), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(8LL))}, 1)), px_str(")"));
        }
        px_srcline(138);
        (void)(px_method(_v305, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(8LL))}, 1)), _v309), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v307, px_int(1LL)), px_add(_v304, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v307, px_int(2LL)), px_add(_v304, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v304, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(139);
         _v306 = px_add(_v306, px_int(1LL));
    }
    px_srcline(140);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v305}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v304}, 1)), px_str("]"));
px_err_310:
    if (px_err_310_proped) return px_err_310_val;
    return px_null();
}

static LXValue fn_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fmt_float");
    LXValue _v311 = (nargs > 0) ? args[0] : px_null();
    LXValue _v312 = px_null();
    LXValue px_err_313_val = px_null();
    int px_err_313_proped = 0;
    px_srcline(142);
    _v312 = px_call(px_get_global("str"), (LXValue[]){_v311}, 1);
    px_srcline(143);
    if (px_is_truthy(({ LXValue _t314 = px_eq(_v312, px_str("inf")); px_is_truthy(_t314) ? _t314 : px_eq(_v312, px_str("-inf")); }))) {
        px_srcline(144);
        return _v312;
    }
    px_srcline(145);
    if (px_is_truthy(({ LXValue _t316 = ({ LXValue _t315 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v312, px_str(".")}, 2)); px_is_truthy(_t315) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v312, px_str("e")}, 2)) : _t315; }); px_is_truthy(_t316) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v312, px_str("E")}, 2)) : _t316; }))) {
        px_srcline(146);
        return px_add(_v312, px_str(".0"));
    }
    px_srcline(147);
    return _v312;
px_err_313:
    if (px_err_313_proped) return px_err_313_val;
    return px_null();
}

static LXValue fn_dump_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_field");
    LXValue _v317 = (nargs > 0) ? args[0] : px_null();
    LXValue _v318 = (nargs > 1) ? args[1] : px_null();
    LXValue _v319 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_320_val = px_null();
    int px_err_320_proped = 0;
    px_srcline(149);
    if (px_is_truthy(px_eq(_v318, px_str("s")))) {
        px_srcline(150);
        return _v317;
    }
    px_srcline(151);
    if (px_is_truthy(px_eq(_v318, px_str("r")))) {
        px_srcline(152);
        return px_call(px_get_global("str"), (LXValue[]){_v317}, 1);
    }
    px_srcline(153);
    if (px_is_truthy(px_eq(_v318, px_str("f")))) {
        px_srcline(154);
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v317}, 1);
    }
    px_srcline(155);
    if (px_is_truthy(px_eq(_v318, px_str("n")))) {
        px_srcline(156);
        return px_call(px_get_global("dump_node"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(157);
    if (px_is_truthy(px_eq(_v318, px_str("o")))) {
        px_srcline(158);
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(159);
    if (px_is_truthy(px_eq(_v318, px_str("os")))) {
        px_srcline(160);
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(161);
    if (px_is_truthy(px_eq(_v318, px_str("ol")))) {
        px_srcline(162);
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(163);
    if (px_is_truthy(px_eq(_v318, px_str("l")))) {
        px_srcline(164);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(165);
    if (px_is_truthy(px_eq(_v318, px_str("ls")))) {
        px_srcline(166);
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v318, px_str("tl")))) {
        px_srcline(168);
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v318, px_str("lpl")))) {
        px_srcline(170);
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v318, px_str("lp")))) {
        px_srcline(172);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(_v318, px_str("lsf")))) {
        px_srcline(174);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(175);
    if (px_is_truthy(px_eq(_v318, px_str("lev")))) {
        px_srcline(176);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(177);
    if (px_is_truthy(px_eq(_v318, px_str("lfd")))) {
        px_srcline(178);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(179);
    if (px_is_truthy(px_eq(_v318, px_str("ltci")))) {
        px_srcline(181);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(182);
    if (px_is_truthy(px_eq(_v318, px_str("lc")))) {
        px_srcline(183);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(184);
    if (px_is_truthy(px_eq(_v318, px_str("lma")))) {
        px_srcline(185);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(186);
    if (px_is_truthy(px_eq(_v318, px_str("lt2")))) {
        px_srcline(187);
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(188);
    if (px_is_truthy(px_eq(_v318, px_str("lt2b")))) {
        px_srcline(189);
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(190);
    if (px_is_truthy(px_eq(_v318, px_str("lt3")))) {
        px_srcline(191);
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(192);
    if (px_is_truthy(px_eq(_v318, px_str("p")))) {
        px_srcline(193);
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v317, _v319}, 2);
    }
    px_srcline(194);
    return px_call(px_get_global("str"), (LXValue[]){_v317}, 1);
px_err_320:
    if (px_err_320_proped) return px_err_320_val;
    return px_null();
}

static LXValue fn_dump_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_program");
    LXValue _v321 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_322_val = px_null();
    int px_err_322_proped = 0;
    px_srcline(197);
    return px_call(px_get_global("dump_node"), (LXValue[]){_v321, px_int(0LL)}, 2);
px_err_322:
    if (px_err_322_proped) return px_err_322_val;
    return px_null();
}

static LXValue fn_pk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pk");
    LXValue px_err_323_val = px_null();
    int px_err_323_proped = 0;
    px_srcline(107);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
px_err_323:
    if (px_err_323_proped) return px_err_323_val;
    return px_null();
}

static LXValue fn_pk_display(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pk_display");
    LXValue _v324 = px_null();
    LXValue _v325 = px_null();
    LXValue px_err_326_val = px_null();
    int px_err_326_proped = 0;
    px_srcline(110);
    _v324 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
    px_srcline(111);
    _v325 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
    px_srcline(112);
    if (px_is_truthy(px_eq(_v324, px_str("整数")))) {
        px_srcline(113);
        return px_add(px_str("整数 "), _v325);
    }
    px_srcline(114);
    if (px_is_truthy(px_eq(_v324, px_str("浮点")))) {
        px_srcline(115);
        return px_add(px_str("浮点 "), _v325);
    }
    px_srcline(116);
    if (px_is_truthy(px_eq(_v324, px_str("字符串")))) {
        px_srcline(117);
        return px_add(px_str("字符串 "), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v325}, 1));
    }
    px_srcline(118);
    if (px_is_truthy(px_eq(_v324, px_str("标识符")))) {
        px_srcline(119);
        return px_add(px_str("标识符 "), _v325);
    }
    px_srcline(120);
    if (px_is_truthy(px_eq(_v324, px_str("注释")))) {
        px_srcline(121);
        return px_add(px_str("注释 "), _v325);
    }
    px_srcline(122);
    return _v324;
px_err_326:
    if (px_err_326_proped) return px_err_326_val;
    return px_null();
}

static LXValue fn_pv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pv");
    LXValue px_err_327_val = px_null();
    int px_err_327_proped = 0;
    px_srcline(124);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
px_err_327:
    if (px_err_327_proped) return px_err_327_val;
    return px_null();
}

static LXValue fn_pline(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pline");
    LXValue px_err_328_val = px_null();
    int px_err_328_proped = 0;
    px_srcline(126);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL));
px_err_328:
    if (px_err_328_proped) return px_err_328_val;
    return px_null();
}

static LXValue fn_pcol(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pcol");
    LXValue px_err_329_val = px_null();
    int px_err_329_proped = 0;
    px_srcline(128);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL));
px_err_329:
    if (px_err_329_proped) return px_err_329_val;
    return px_null();
}

static LXValue fn_ppos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ppos");
    LXValue px_err_330_val = px_null();
    int px_err_330_proped = 0;
    px_srcline(130);
    return px_list_n((LXValue[]){px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL)), px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL))}, 2);
px_err_330:
    if (px_err_330_proped) return px_err_330_val;
    return px_null();
}

static LXValue fn_adv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("adv");
    LXValue _v331 = px_null();
    LXValue px_err_332_val = px_null();
    int px_err_332_proped = 0;
    px_srcline(132);
    _v331 = px_index(px_get_global("p_toks"), px_get_global("p_pos"));
    px_srcline(133);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(134);
        px_set_global("p_pos", px_add(px_get_global("p_pos"), px_int(1LL)));
    }
    px_srcline(136);
    if (px_is_truthy(({ LXValue _t334 = ({ LXValue _t333 = px_eq(px_index(_v331, px_int(0LL)), px_str("(")); px_is_truthy(_t333) ? _t333 : px_eq(px_index(_v331, px_int(0LL)), px_str("[")); }); px_is_truthy(_t334) ? _t334 : px_eq(px_index(_v331, px_int(0LL)), px_str("{")); }))) {
        px_srcline(137);
        px_set_global("p_brack", px_add(px_get_global("p_brack"), px_int(1LL)));
    }
    else if (px_is_truthy(({ LXValue _t336 = ({ LXValue _t335 = px_eq(px_index(_v331, px_int(0LL)), px_str(")")); px_is_truthy(_t335) ? _t335 : px_eq(px_index(_v331, px_int(0LL)), px_str("]")); }); px_is_truthy(_t336) ? _t336 : px_eq(px_index(_v331, px_int(0LL)), px_str("}")); }))) {
        px_srcline(139);
        if (px_is_truthy(px_gt(px_get_global("p_brack"), px_int(0LL)))) {
            px_srcline(140);
            px_set_global("p_brack", px_sub(px_get_global("p_brack"), px_int(1LL)));
        }
    }
    px_srcline(141);
    return _v331;
px_err_332:
    if (px_err_332_proped) return px_err_332_val;
    return px_null();
}

static LXValue fn_chk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk");
    LXValue _v337 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_338_val = px_null();
    int px_err_338_proped = 0;
    px_srcline(143);
    return px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), _v337);
px_err_338:
    if (px_err_338_proped) return px_err_338_val;
    return px_null();
}

static LXValue fn_chk2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk2");
    LXValue _v339 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_340_val = px_null();
    int px_err_340_proped = 0;
    px_srcline(145);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(146);
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(1LL))), px_int(0LL)), _v339);
    }
    px_srcline(147);
    return px_bool(false);
px_err_340:
    if (px_err_340_proped) return px_err_340_val;
    return px_null();
}

static LXValue fn_chk3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk3");
    LXValue _v341 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_342_val = px_null();
    int px_err_342_proped = 0;
    px_srcline(150);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(2LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(151);
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(2LL))), px_int(0LL)), _v341);
    }
    px_srcline(152);
    return px_bool(false);
px_err_342:
    if (px_err_342_proped) return px_err_342_val;
    return px_null();
}

static LXValue fn_expect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect");
    LXValue _v343 = (nargs > 0) ? args[0] : px_null();
    LXValue _v344 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_345_val = px_null();
    int px_err_345_proped = 0;
    px_srcline(154);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v343}, 1))) {
        px_srcline(155);
        return px_call(px_get_global("adv"), (LXValue[]){}, 0);
    }
    px_srcline(156);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望 "), _v344), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_345:
    if (px_err_345_proped) return px_err_345_val;
    return px_null();
}

static LXValue fn_expect_ident(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect_ident");
    LXValue _v346 = (nargs > 0) ? args[0] : px_null();
    LXValue _v347 = px_null();
    LXValue px_err_348_val = px_null();
    int px_err_348_proped = 0;
    px_srcline(158);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(159);
        _v347 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(160);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(161);
        return _v347;
    }
    px_srcline(162);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v346), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_348:
    if (px_err_348_proped) return px_err_348_val;
    return px_null();
}

static LXValue fn_is_name_kind(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_name_kind");
    LXValue _v349 = (nargs > 0) ? args[0] : px_null();
    LXValue _v350 = px_null();
    LXValue _v351 = px_null();
    LXValue px_err_352_val = px_null();
    int px_err_352_proped = 0;
    px_srcline(164);
    _v350 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture"), px_str("extern")}, 36);
    px_srcline(165);
    _v351 = px_int(0LL);
    px_srcline(166);
    while (px_is_truthy(px_lt(_v351, px_call(px_get_global("len"), (LXValue[]){_v350}, 1)))) {
        px_srcline(167);
        if (px_is_truthy(px_eq(px_index(_v350, _v351), _v349))) {
            px_srcline(168);
            return px_bool(true);
        }
        px_srcline(169);
         _v351 = px_add(_v351, px_int(1LL));
    }
    px_srcline(170);
    return px_bool(false);
px_err_352:
    if (px_err_352_proped) return px_err_352_val;
    return px_null();
}

static LXValue fn_expect_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect_name");
    LXValue _v353 = (nargs > 0) ? args[0] : px_null();
    LXValue _v354 = px_null();
    LXValue px_err_355_val = px_null();
    int px_err_355_proped = 0;
    px_srcline(172);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(173);
        _v354 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(174);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(175);
        return _v354;
    }
    px_srcline(176);
    if (px_is_truthy(px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1))) {
        px_srcline(177);
        _v354 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(178);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(179);
        return _v354;
    }
    px_srcline(180);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v353), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_355:
    if (px_err_355_proped) return px_err_355_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("perr");
    LXValue _v356 = (nargs > 0) ? args[0] : px_null();
    LXValue _v357 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_358_val = px_null();
    int px_err_358_proped = 0;
    px_srcline(184);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1)), px_str(": 语法错误 ")), _v356), px_str(": ")), _v357)}, 1));
    px_srcline(185);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_358:
    if (px_err_358_proped) return px_err_358_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines");
    LXValue px_err_359_val = px_null();
    int px_err_359_proped = 0;
    px_srcline(187);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(188);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_359:
    if (px_err_359_proped) return px_err_359_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_brace_indents");
    LXValue px_err_360_val = px_null();
    int px_err_360_proped = 0;
    px_srcline(190);
    while (px_is_truthy(({ LXValue _t361 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t361) ? _t361 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(191);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_360:
    if (px_err_360_proped) return px_err_360_val;
    return px_null();
}

static LXValue fn_skip_expr_ws(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_expr_ws");
    LXValue px_err_362_val = px_null();
    int px_err_362_proped = 0;
    px_srcline(193);
    while (px_is_truthy(({ LXValue _t364 = ({ LXValue _t363 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t363) ? _t363 : px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); }); px_is_truthy(_t364) ? _t364 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(194);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_362:
    if (px_err_362_proped) return px_err_362_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines_in_block");
    LXValue px_err_365_val = px_null();
    int px_err_365_proped = 0;
    px_srcline(196);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(197);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_365:
    if (px_err_365_proped) return px_err_365_val;
    return px_null();
}

static LXValue fn_chk_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk_op");
    LXValue _v366 = (nargs > 0) ? args[0] : px_null();
    LXValue _v367 = px_null();
    LXValue px_err_368_val = px_null();
    int px_err_368_proped = 0;
    px_srcline(209);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v366}, 1))) {
        px_srcline(210);
        return px_bool(true);
    }
    px_srcline(211);
    if (px_is_truthy(px_le(px_get_global("p_brack"), px_int(0LL)))) {
        px_srcline(212);
        return px_bool(false);
    }
    px_srcline(213);
    if (px_is_truthy(({ LXValue _t370 = ({ LXValue _t369 = px_eq(_v366, px_str("-")); px_is_truthy(_t369) ? _t369 : px_eq(_v366, px_str("~")); }); px_is_truthy(_t370) ? _t370 : px_eq(_v366, px_str("not")); }))) {
        px_srcline(214);
        return px_bool(false);
    }
    px_srcline(215);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)))) {
        px_srcline(216);
        return px_bool(false);
    }
    px_srcline(217);
    _v367 = px_get_global("p_pos");
    px_srcline(218);
    while (px_is_truthy(({ LXValue _t371 = px_lt(_v367, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)); px_is_truthy(_t371) ? px_eq(px_index(px_index(px_get_global("p_toks"), _v367), px_int(0LL)), px_str("换行")) : _t371; }))) {
        px_srcline(219);
         _v367 = px_add(_v367, px_int(1LL));
    }
    px_srcline(220);
    if (px_is_truthy(({ LXValue _t372 = px_lt(_v367, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)); px_is_truthy(_t372) ? px_eq(px_index(px_index(px_get_global("p_toks"), _v367), px_int(0LL)), _v366) : _t372; }))) {
        px_srcline(221);
        px_set_global("p_pos", _v367);
        px_srcline(222);
        return px_bool(true);
    }
    px_srcline(223);
    return px_bool(false);
px_err_368:
    if (px_err_368_proped) return px_err_368_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("node_pos");
    LXValue _v373 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_374_val = px_null();
    int px_err_374_proped = 0;
    px_srcline(225);
    return px_index(_v373, px_sub(px_call(px_get_global("len"), (LXValue[]){_v373}, 1), px_int(1LL)));
px_err_374:
    if (px_err_374_proped) return px_err_374_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("qstr");
    LXValue _v375 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_376_val = px_null();
    int px_err_376_proped = 0;
    px_srcline(227);
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v375}, 1);
px_err_376:
    if (px_err_376_proped) return px_err_376_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_program");
    LXValue _v377 = px_null();
    LXValue px_err_378_val = px_null();
    int px_err_378_proped = 0;
    px_srcline(230);
    _v377 = px_list_n((LXValue[]){}, 0);
    px_srcline(231);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(232);
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        px_srcline(233);
        (void)(px_method(_v377, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(234);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    px_srcline(235);
    return px_list_n((LXValue[]){px_str("Program"), _v377}, 2);
px_err_378:
    if (px_err_378_proped) return px_err_378_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_stmt");
    LXValue _v379 = px_null();
    LXValue _v380 = px_null();
    LXValue _v381 = px_null();
    LXValue _v382 = px_null();
    LXValue px_err_383_val = px_null();
    int px_err_383_proped = 0;
    px_srcline(238);
    _v379 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(239);
    if (px_is_truthy(px_eq(_v379, px_str("let")))) {
        px_srcline(240);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(_v379, px_str("var")))) {
        px_srcline(242);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    px_srcline(243);
    if (px_is_truthy(px_eq(_v379, px_str("const")))) {
        px_srcline(244);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    px_srcline(245);
    if (px_is_truthy(px_eq(_v379, px_str("if")))) {
        px_srcline(246);
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    px_srcline(247);
    if (px_is_truthy(px_eq(_v379, px_str("for")))) {
        px_srcline(248);
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    px_srcline(249);
    if (px_is_truthy(px_eq(_v379, px_str("while")))) {
        px_srcline(250);
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    px_srcline(251);
    if (px_is_truthy(px_eq(_v379, px_str("def")))) {
        px_srcline(252);
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    px_srcline(253);
    if (px_is_truthy(px_eq(_v379, px_str("extern")))) {
        px_srcline(254);
        return px_call(px_get_global("parse_extern_def"), (LXValue[]){}, 0);
    }
    px_srcline(255);
    if (px_is_truthy(px_eq(_v379, px_str("struct")))) {
        px_srcline(256);
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    px_srcline(257);
    if (px_is_truthy(px_eq(_v379, px_str("enum")))) {
        px_srcline(258);
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    px_srcline(259);
    if (px_is_truthy(({ LXValue _t384 = px_eq(_v379, px_str("标识符")); px_is_truthy(_t384) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("type")) : _t384; }))) {
        px_srcline(262);
        if (px_is_truthy(({ LXValue _t385 = px_call(px_get_global("chk2"), (LXValue[]){px_str("标识符")}, 1); px_is_truthy(_t385) ? px_call(px_get_global("chk3"), (LXValue[]){px_str("const")}, 1) : _t385; }))) {
            px_srcline(263);
            return px_call(px_get_global("parse_type_const"), (LXValue[]){}, 0);
        }
        px_srcline(264);
        return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
    }
    px_srcline(265);
    if (px_is_truthy(px_eq(_v379, px_str("trait")))) {
        px_srcline(266);
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    px_srcline(267);
    if (px_is_truthy(px_eq(_v379, px_str("impl")))) {
        px_srcline(268);
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    px_srcline(269);
    if (px_is_truthy(px_eq(_v379, px_str("import")))) {
        px_srcline(270);
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    px_srcline(271);
    if (px_is_truthy(px_eq(_v379, px_str("from")))) {
        px_srcline(272);
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    px_srcline(273);
    if (px_is_truthy(px_eq(_v379, px_str("return")))) {
        px_srcline(274);
        _v380 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(275);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(276);
        if (px_is_truthy(({ LXValue _t387 = ({ LXValue _t386 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t386) ? _t386 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t387) ? _t387 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(277);
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v380}, 3);
        }
        px_srcline(278);
        _v381 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(279);
        return px_list_n((LXValue[]){px_str("Return"), _v381, _v380}, 3);
    }
    px_srcline(280);
    if (px_is_truthy(px_eq(_v379, px_str("break")))) {
        px_srcline(281);
        _v380 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(282);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(283);
        return px_list_n((LXValue[]){px_str("Break"), _v380}, 2);
    }
    px_srcline(284);
    if (px_is_truthy(px_eq(_v379, px_str("continue")))) {
        px_srcline(285);
        _v380 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(286);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(287);
        return px_list_n((LXValue[]){px_str("Continue"), _v380}, 2);
    }
    px_srcline(288);
    if (px_is_truthy(px_eq(_v379, px_str("spawn")))) {
        px_srcline(289);
        _v380 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(290);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(291);
        _v382 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(292);
        return px_list_n((LXValue[]){px_str("Spawn"), _v382, _v380}, 3);
    }
    px_srcline(293);
    if (px_is_truthy(px_eq(_v379, px_str("select")))) {
        px_srcline(294);
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    px_srcline(295);
    if (px_is_truthy(px_eq(_v379, px_str("fn")))) {
        px_srcline(296);
        _v382 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(297);
        _v380 = px_call(px_get_global("node_pos"), (LXValue[]){_v382}, 1);
        px_srcline(298);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v382, _v380}, 3);
    }
    px_srcline(299);
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_383:
    if (px_err_383_proped) return px_err_383_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_var_decl");
    LXValue _v388 = (nargs > 0) ? args[0] : px_null();
    LXValue _v389 = px_null();
    LXValue _v390 = px_null();
    LXValue _v391 = px_null();
    LXValue _v392 = px_null();
    LXValue px_err_393_val = px_null();
    int px_err_393_proped = 0;
    px_srcline(301);
    _v389 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(302);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(304);
    if (px_is_truthy(({ LXValue _t395 = ({ LXValue _t394 = px_eq(_v388, px_str("Let")); px_is_truthy(_t394) ? px_call(px_get_global("chk"), (LXValue[]){px_str("标识符")}, 1) : _t394; }); px_is_truthy(_t395) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("mut")) : _t395; }))) {
        px_srcline(305);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(306);
         _v388 = px_str("Mut");
    }
    px_srcline(307);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        px_srcline(308);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    px_srcline(309);
    _v390 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    px_srcline(310);
    _v391 = px_null();
    px_srcline(311);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(312);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(313);
         _v391 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(314);
    _v392 = px_null();
    px_srcline(315);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        px_srcline(316);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(317);
         _v392 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    px_srcline(318);
    return px_list_n((LXValue[]){px_str("VarDecl"), _v388, px_call(px_get_global("qstr"), (LXValue[]){_v390}, 1), _v391, _v392, _v389}, 6);
px_err_393:
    if (px_err_393_proped) return px_err_393_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_assign_or_expr");
    LXValue _v396 = px_null();
    LXValue _v397 = px_null();
    LXValue _v398 = px_null();
    LXValue _v399 = px_null();
    LXValue _v400 = px_null();
    LXValue _v401 = px_null();
    LXValue _v402 = px_null();
    LXValue px_err_403_val = px_null();
    int px_err_403_proped = 0;
    px_srcline(322);
    _v396 = px_null();
    px_srcline(323);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(324);
         _v396 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
    }
    px_srcline(325);
    _v397 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(326);
    _v398 = px_call(px_get_global("node_pos"), (LXValue[]){_v397}, 1);
    px_srcline(327);
    _v399 = px_null();
    px_srcline(328);
    _v400 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(329);
    if (px_is_truthy(px_eq(_v400, px_str("=")))) {
        px_srcline(330);
         _v399 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("<-")))) {
        px_srcline(333);
         _v399 = px_str("Append");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("+=")))) {
        px_srcline(335);
         _v399 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("-=")))) {
        px_srcline(337);
         _v399 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("*=")))) {
        px_srcline(339);
         _v399 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("/=")))) {
        px_srcline(341);
         _v399 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("//=")))) {
        px_srcline(343);
         _v399 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("%=")))) {
        px_srcline(345);
         _v399 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("**=")))) {
        px_srcline(347);
         _v399 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("&=")))) {
        px_srcline(349);
         _v399 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("|=")))) {
        px_srcline(351);
         _v399 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("^=")))) {
        px_srcline(353);
         _v399 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v400, px_str("<<=")))) {
        px_srcline(355);
         _v399 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v400, px_str(">>=")))) {
        px_srcline(357);
         _v399 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v400, px_str(">>>=")))) {
        px_srcline(359);
         _v399 = px_str("ShrU");
    }
    px_srcline(360);
    if (px_is_truthy(px_ne(_v399, px_null()))) {
        px_srcline(361);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(362);
        _v401 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(363);
        return px_list_n((LXValue[]){px_str("Assign"), _v397, _v399, _v401, _v398}, 5);
    }
    px_srcline(376);
    if (px_is_truthy(px_eq(px_index(_v397, px_int(0LL)), px_str("Var")))) {
        px_srcline(377);
        _v402 = px_null();
        px_srcline(378);
        if (px_is_truthy(px_ne(_v396, px_null()))) {
            px_srcline(379);
             _v402 = _v396;
        }
        else {
            px_srcline(381);
             _v402 = px_index(_v397, px_int(1LL));
        }
        px_srcline(382);
        if (px_is_truthy(px_eq(_v402, px_str("pass")))) {
            px_srcline(383);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2012"), px_str("`pass` 不是 PuXian 关键字（它是普通标识符，`var pass = 0` 合法）：空语句请写 `0`，或直接删掉这一行")}, 2));
        }
        px_srcline(384);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2012"), px_add(px_add(px_str("裸标识符语句没有效果："), _v402), px_str("（调用需要括号 `()`；若想要空语句请写 `0`）"))}, 2));
    }
    px_srcline(385);
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v397, _v398}, 3);
px_err_403:
    if (px_err_403_proped) return px_err_403_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if");
    LXValue _v404 = px_null();
    LXValue _v405 = px_null();
    LXValue _v406 = px_null();
    LXValue _v407 = px_null();
    LXValue _v408 = px_null();
    LXValue _v409 = px_null();
    LXValue _v410 = px_null();
    LXValue _v411 = px_null();
    LXValue px_err_412_val = px_null();
    int px_err_412_proped = 0;
    px_srcline(387);
    _v404 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(388);
    _v405 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(389);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(390);
    _v406 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(391);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(392);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(393);
    _v407 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v405}, 1);
    px_srcline(394);
    _v408 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v406, _v407}, 2)}, 1);
    px_srcline(395);
    _v409 = px_null();
    px_srcline(396);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(397);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            px_srcline(398);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(399);
            _v410 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(400);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(401);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(402);
            _v411 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v405}, 1);
            px_srcline(403);
            (void)(px_method(_v408, "append", (LXValue[]){px_list_n((LXValue[]){_v410, _v411}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            px_srcline(405);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(406);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(407);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(408);
             _v409 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v405}, 1);
            px_srcline(409);
            break;
        }
        else {
            px_srcline(411);
            break;
        }
    }
    px_srcline(412);
    return px_list_n((LXValue[]){px_str("If"), _v408, _v409, _v404}, 4);
px_err_412:
    if (px_err_412_proped) return px_err_412_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_for");
    LXValue _v413 = px_null();
    LXValue _v414 = px_null();
    LXValue _v415 = px_null();
    LXValue _v416 = px_null();
    LXValue _v417 = px_null();
    LXValue px_err_418_val = px_null();
    int px_err_418_proped = 0;
    px_srcline(414);
    _v413 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(415);
    _v414 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(416);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(417);
    _v415 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    px_srcline(418);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    px_srcline(419);
    _v416 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(420);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(421);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(422);
    _v417 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v414}, 1);
    px_srcline(423);
    return px_list_n((LXValue[]){px_str("For"), px_call(px_get_global("qstr"), (LXValue[]){_v415}, 1), _v416, _v417, _v413}, 5);
px_err_418:
    if (px_err_418_proped) return px_err_418_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_while");
    LXValue _v419 = px_null();
    LXValue _v420 = px_null();
    LXValue _v421 = px_null();
    LXValue _v422 = px_null();
    LXValue px_err_423_val = px_null();
    int px_err_423_proped = 0;
    px_srcline(425);
    _v419 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(426);
    _v420 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(427);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(428);
    _v421 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(429);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(430);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(431);
    _v422 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v420}, 1);
    px_srcline(432);
    return px_list_n((LXValue[]){px_str("While"), _v421, _v422, _v419}, 4);
px_err_423:
    if (px_err_423_proped) return px_err_423_val;
    return px_null();
}

static LXValue fn_parse_block_ctxt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block_ctxt");
    LXValue _v424 = (nargs > 0) ? args[0] : px_null();
    LXValue _v425 = px_null();
    LXValue _v426 = px_null();
    LXValue px_err_427_val = px_null();
    int px_err_427_proped = 0;
    px_srcline(439);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(440);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(441);
    if (px_is_truthy(px_le(px_get_global("p_paren_ctxt"), px_int(0LL)))) {
        px_srcline(442);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
        px_srcline(443);
        return px_list_n((LXValue[]){}, 0);
    }
    px_srcline(444);
    _v425 = px_list_n((LXValue[]){}, 0);
    px_srcline(445);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(446);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(447);
        _v426 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(448);
        if (px_is_truthy(({ LXValue _t432 = ({ LXValue _t431 = ({ LXValue _t430 = ({ LXValue _t429 = ({ LXValue _t428 = px_eq(_v426, px_str("去缩进")); px_is_truthy(_t428) ? _t428 : px_eq(_v426, px_str("EOF")); }); px_is_truthy(_t429) ? _t429 : px_eq(_v426, px_str(")")); }); px_is_truthy(_t430) ? _t430 : px_eq(_v426, px_str("]")); }); px_is_truthy(_t431) ? _t431 : px_eq(_v426, px_str("}")); }); px_is_truthy(_t432) ? _t432 : px_eq(_v426, px_str(",")); }))) {
            px_srcline(449);
            break;
        }
        px_srcline(450);
        if (px_is_truthy(px_le(px_call(px_get_global("pcol"), (LXValue[]){}, 0), _v424))) {
            px_srcline(451);
            break;
        }
        px_srcline(452);
        (void)(px_method(_v425, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(453);
    return _v425;
px_err_427:
    if (px_err_427_proped) return px_err_427_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block");
    LXValue _v433 = px_null();
    LXValue px_err_434_val = px_null();
    int px_err_434_proped = 0;
    px_srcline(455);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(456);
    _v433 = px_list_n((LXValue[]){}, 0);
    px_srcline(457);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(458);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(459);
        if (px_is_truthy(({ LXValue _t435 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t435) ? _t435 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(460);
            break;
        }
        px_srcline(461);
        (void)(px_method(_v433, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(462);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(463);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    px_srcline(464);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(465);
    return _v433;
px_err_434:
    if (px_err_434_proped) return px_err_434_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_params");
    LXValue _v436 = px_null();
    LXValue _v437 = px_null();
    LXValue _v438 = px_null();
    LXValue _v439 = px_null();
    LXValue px_err_440_val = px_null();
    int px_err_440_proped = 0;
    px_srcline(469);
    _v436 = px_list_n((LXValue[]){}, 0);
    px_srcline(470);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        px_srcline(471);
        return _v436;
    }
    px_srcline(472);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(473);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(474);
        _v437 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        px_srcline(475);
        _v438 = _v437;
        px_srcline(476);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            px_srcline(477);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(478);
            _v439 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
            px_srcline(479);
             _v438 = px_add(px_add(_v437, px_str(": ")), _v439);
        }
        px_srcline(480);
        (void)(px_method(_v436, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v438}, 1)}, 1));
        px_srcline(481);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(482);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(483);
            continue;
        }
        px_srcline(484);
        break;
    }
    px_srcline(485);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(486);
    return _v436;
px_err_440:
    if (px_err_440_proped) return px_err_440_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_func_def");
    LXValue _v441 = px_null();
    LXValue _v442 = px_null();
    LXValue _v443 = px_null();
    LXValue _v444 = px_null();
    LXValue _v445 = px_null();
    LXValue _v446 = px_null();
    LXValue px_err_447_val = px_null();
    int px_err_447_proped = 0;
    px_srcline(488);
    _v441 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(489);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(490);
    _v442 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(491);
    _v443 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(492);
    _v444 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(493);
    _v445 = px_null();
    px_srcline(494);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(495);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(496);
         _v445 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(497);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(498);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(499);
    _v446 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(500);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v442}, 1), _v444, _v445, _v446, _v441, _v443}, 7);
px_err_447:
    if (px_err_447_proped) return px_err_447_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_extern_def");
    LXValue _v448 = px_null();
    LXValue _v449 = px_null();
    LXValue _v450 = px_null();
    LXValue _v451 = px_null();
    LXValue px_err_452_val = px_null();
    int px_err_452_proped = 0;
    px_srcline(504);
    _v448 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(505);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(506);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    px_srcline(507);
    _v449 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(508);
    _v450 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(509);
    _v451 = px_null();
    px_srcline(510);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(511);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(512);
         _v451 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(513);
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v449}, 1), _v450, _v451, _v448}, 5);
px_err_452:
    if (px_err_452_proped) return px_err_452_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_struct_def");
    LXValue _v453 = px_null();
    LXValue _v454 = px_null();
    LXValue _v455 = px_null();
    LXValue _v456 = px_null();
    LXValue _v457 = px_null();
    LXValue _v458 = px_null();
    LXValue _v459 = px_null();
    LXValue px_err_460_val = px_null();
    int px_err_460_proped = 0;
    px_srcline(515);
    _v453 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(516);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(517);
    _v454 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    px_srcline(518);
    _v455 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(519);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(520);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(521);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(522);
    _v456 = px_list_n((LXValue[]){}, 0);
    px_srcline(523);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(524);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(525);
        if (px_is_truthy(({ LXValue _t461 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t461) ? _t461 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(526);
            break;
        }
        px_srcline(527);
        _v457 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(528);
        _v458 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        px_srcline(529);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(530);
        _v459 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(531);
        (void)(px_method(_v456, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v458}, 1), _v459, _v457}, 4)}, 1));
        px_srcline(532);
        if (px_is_truthy(({ LXValue _t462 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t462) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t462; }))) {
            px_srcline(533);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    px_srcline(534);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(535);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    px_srcline(536);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(537);
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v454}, 1), _v456, _v453, _v455}, 5);
px_err_460:
    if (px_err_460_proped) return px_err_460_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_enum_def");
    LXValue _v463 = px_null();
    LXValue _v464 = px_null();
    LXValue _v465 = px_null();
    LXValue _v466 = px_null();
    LXValue _v467 = px_null();
    LXValue _v468 = px_null();
    LXValue px_err_469_val = px_null();
    int px_err_469_proped = 0;
    px_srcline(539);
    _v463 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(540);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(541);
    _v464 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(542);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(543);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(544);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(545);
    _v465 = px_list_n((LXValue[]){}, 0);
    px_srcline(546);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(547);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(548);
        if (px_is_truthy(({ LXValue _t470 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t470) ? _t470 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(549);
            break;
        }
        px_srcline(550);
        _v466 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(551);
        _v467 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        px_srcline(552);
        _v468 = px_list_n((LXValue[]){}, 0);
        px_srcline(553);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(554);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(555);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(556);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(557);
                    (void)(px_method(_v468, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    px_srcline(558);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(559);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(560);
                        continue;
                    }
                    px_srcline(561);
                    break;
                }
            }
            px_srcline(562);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        px_srcline(563);
        (void)(px_method(_v465, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v467}, 1), _v468, _v466}, 4)}, 1));
        px_srcline(564);
        if (px_is_truthy(({ LXValue _t471 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t471) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t471; }))) {
            px_srcline(565);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    px_srcline(566);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(567);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    px_srcline(568);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(569);
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v464}, 1), _v465, _v463}, 4);
px_err_469:
    if (px_err_469_proped) return px_err_469_val;
    return px_null();
}

static LXValue fn_parse_type_const(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_const");
    LXValue _v472 = px_null();
    LXValue _v473 = px_null();
    LXValue _v474 = px_null();
    LXValue _v475 = px_null();
    LXValue _v476 = px_null();
    LXValue _v477 = px_null();
    LXValue px_err_478_val = px_null();
    int px_err_478_proped = 0;
    px_srcline(574);
    _v472 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(575);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(576);
    _v473 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(577);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("const"), px_str("'const'")}, 2));
    px_srcline(578);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(579);
    _v474 = px_list_n((LXValue[]){}, 0);
    px_srcline(580);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(581);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(582);
            _v475 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(583);
            _v476 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("常量名")}, 1);
            px_srcline(584);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
            px_srcline(585);
            _v477 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(586);
            (void)(px_method(_v474, "append", (LXValue[]){px_list_n((LXValue[]){px_str("TypeConstItem"), px_call(px_get_global("qstr"), (LXValue[]){_v476}, 1), _v477, _v475}, 4)}, 1));
            px_srcline(587);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(588);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(589);
                continue;
            }
            px_srcline(590);
            break;
        }
    }
    px_srcline(591);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(592);
    return px_list_n((LXValue[]){px_str("TypeConst"), px_call(px_get_global("qstr"), (LXValue[]){_v473}, 1), _v474, _v472}, 4);
px_err_478:
    if (px_err_478_proped) return px_err_478_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_trait_def");
    LXValue _v479 = px_null();
    LXValue _v480 = px_null();
    LXValue _v481 = px_null();
    LXValue _v482 = px_null();
    LXValue _v483 = px_null();
    LXValue _v484 = px_null();
    LXValue _v485 = px_null();
    LXValue _v486 = px_null();
    LXValue px_err_487_val = px_null();
    int px_err_487_proped = 0;
    px_srcline(594);
    _v479 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(595);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(596);
    _v480 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    px_srcline(597);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(598);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(599);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(600);
    _v481 = px_list_n((LXValue[]){}, 0);
    px_srcline(601);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(602);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(603);
        if (px_is_truthy(({ LXValue _t488 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t488) ? _t488 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(604);
            break;
        }
        px_srcline(605);
        _v482 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(606);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(607);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        px_srcline(608);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(609);
        _v483 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(610);
        _v484 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(611);
        _v485 = px_null();
        px_srcline(612);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(613);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(614);
             _v485 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(615);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(616);
        _v486 = px_list_n((LXValue[]){}, 0);
        px_srcline(617);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(618);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(619);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(620);
                 _v486 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        px_srcline(621);
        (void)(px_method(_v481, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v483}, 1), _v484, _v485, _v486, _v482, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        px_srcline(622);
        if (px_is_truthy(({ LXValue _t489 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t489) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t489; }))) {
            px_srcline(623);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    px_srcline(624);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(625);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    px_srcline(626);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(627);
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v480}, 1), _v481, _v479}, 4);
px_err_487:
    if (px_err_487_proped) return px_err_487_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_impl_def");
    LXValue _v490 = px_null();
    LXValue _v491 = px_null();
    LXValue _v492 = px_null();
    LXValue _v493 = px_null();
    LXValue _v494 = px_null();
    LXValue _v495 = px_null();
    LXValue _v496 = px_null();
    LXValue _v497 = px_null();
    LXValue _v498 = px_null();
    LXValue _v499 = px_null();
    LXValue px_err_500_val = px_null();
    int px_err_500_proped = 0;
    px_srcline(629);
    _v490 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(630);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(631);
    _v491 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    px_srcline(632);
    _v492 = px_null();
    px_srcline(633);
    _v493 = _v491;
    px_srcline(634);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(635);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(636);
         _v493 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
        px_srcline(637);
         _v492 = px_call(px_get_global("qstr"), (LXValue[]){_v491}, 1);
    }
    px_srcline(638);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(639);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(640);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(641);
    _v494 = px_list_n((LXValue[]){}, 0);
    px_srcline(642);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(643);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(644);
        if (px_is_truthy(({ LXValue _t501 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t501) ? _t501 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(645);
            break;
        }
        px_srcline(646);
        _v495 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(647);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(648);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        px_srcline(649);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(650);
        _v496 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(651);
        _v497 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(652);
        _v498 = px_null();
        px_srcline(653);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(654);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(655);
             _v498 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(656);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(657);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(658);
        _v499 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        px_srcline(659);
        (void)(px_method(_v494, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v496}, 1), _v497, _v498, _v499, _v495, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    px_srcline(660);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(661);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    px_srcline(662);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(663);
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v493}, 1), _v492, _v494, _v490}, 5);
px_err_500:
    if (px_err_500_proped) return px_err_500_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import");
    LXValue _v502 = px_null();
    LXValue _v503 = px_null();
    LXValue _v504 = px_null();
    LXValue px_err_505_val = px_null();
    int px_err_505_proped = 0;
    px_srcline(665);
    _v502 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(666);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(667);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        px_srcline(668);
        _v503 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(669);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(670);
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v503}, 1), px_list_n((LXValue[]){}, 0), _v502}, 4);
    }
    px_srcline(671);
    _v504 = px_list_n((LXValue[]){}, 0);
    px_srcline(672);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(673);
        (void)(px_method(_v504, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(674);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(675);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(676);
            continue;
        }
        px_srcline(677);
        break;
    }
    px_srcline(678);
    return px_list_n((LXValue[]){px_str("Import"), _v504, px_list_n((LXValue[]){}, 0), _v502}, 4);
px_err_505:
    if (px_err_505_proped) return px_err_505_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import_from");
    LXValue _v506 = px_null();
    LXValue _v507 = px_null();
    LXValue _v508 = px_null();
    LXValue px_err_509_val = px_null();
    int px_err_509_proped = 0;
    px_srcline(680);
    _v506 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(681);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(682);
    _v507 = px_list_n((LXValue[]){}, 0);
    px_srcline(683);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(684);
        (void)(px_method(_v507, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(685);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(686);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(687);
            continue;
        }
        px_srcline(688);
        break;
    }
    px_srcline(689);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    px_srcline(690);
    _v508 = px_list_n((LXValue[]){}, 0);
    px_srcline(691);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(692);
        (void)(px_method(_v508, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        px_srcline(693);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(694);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(695);
            continue;
        }
        px_srcline(696);
        break;
    }
    px_srcline(697);
    return px_list_n((LXValue[]){px_str("Import"), _v507, _v508, _v506}, 4);
px_err_509:
    if (px_err_509_proped) return px_err_509_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_select");
    LXValue _v510 = px_null();
    LXValue _v511 = px_null();
    LXValue _v512 = px_null();
    LXValue _v513 = px_null();
    LXValue _v514 = px_null();
    LXValue _v515 = px_null();
    LXValue _v516 = px_null();
    LXValue px_err_517_val = px_null();
    int px_err_517_proped = 0;
    px_srcline(699);
    _v510 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(700);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(701);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(702);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(703);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(704);
    _v511 = px_list_n((LXValue[]){}, 0);
    px_srcline(705);
    _v512 = px_null();
    px_srcline(706);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(707);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(708);
        if (px_is_truthy(({ LXValue _t518 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t518) ? _t518 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(709);
            break;
        }
        px_srcline(710);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(711);
        if (px_is_truthy(({ LXValue _t519 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t519) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t519; }))) {
            px_srcline(712);
            _v513 = px_get_global("p_pos");
            px_srcline(713);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(714);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(715);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(716);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                px_srcline(717);
                 _v512 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                px_srcline(718);
                continue;
            }
            else {
                px_srcline(720);
                px_set_global("p_pos", _v513);
            }
        }
        px_srcline(721);
        _v514 = px_null();
        px_srcline(722);
        if (px_is_truthy(({ LXValue _t520 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t520) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t520; }))) {
            px_srcline(723);
             _v514 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            px_srcline(724);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        px_srcline(725);
        _v515 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(726);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(727);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(728);
        _v516 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        px_srcline(729);
        (void)(px_method(_v511, "append", (LXValue[]){px_list_n((LXValue[]){_v514, _v515, _v516}, 3)}, 1));
    }
    px_srcline(730);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(731);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    px_srcline(732);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(733);
    return px_list_n((LXValue[]){px_str("Select"), _v511, _v512, _v510}, 4);
px_err_517:
    if (px_err_517_proped) return px_err_517_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_case_body");
    LXValue _v521 = px_null();
    LXValue _v522 = px_null();
    LXValue px_err_523_val = px_null();
    int px_err_523_proped = 0;
    px_srcline(735);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(736);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(737);
    _v521 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(738);
    _v522 = px_call(px_get_global("node_pos"), (LXValue[]){_v521}, 1);
    px_srcline(739);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v521, _v522}, 3)}, 1);
px_err_523:
    if (px_err_523_proped) return px_err_523_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_params");
    LXValue _v524 = px_null();
    LXValue _v525 = px_null();
    LXValue _v526 = px_null();
    LXValue _v527 = px_null();
    LXValue _v528 = px_null();
    LXValue px_err_529_val = px_null();
    int px_err_529_proped = 0;
    px_srcline(742);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(743);
    _v524 = px_list_n((LXValue[]){}, 0);
    px_srcline(744);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(745);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(746);
            _v525 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(747);
            _v526 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            px_srcline(748);
            _v527 = px_null();
            px_srcline(749);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(750);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(751);
                 _v527 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            px_srcline(752);
            _v528 = px_null();
            px_srcline(753);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                px_srcline(754);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(755);
                 _v528 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            px_srcline(756);
            (void)(px_method(_v524, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v526}, 1), _v527, _v528, _v525}, 5)}, 1));
            px_srcline(757);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(758);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(759);
                continue;
            }
            px_srcline(760);
            break;
        }
    }
    px_srcline(761);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(762);
    return _v524;
px_err_529:
    if (px_err_529_proped) return px_err_529_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_expr");
    LXValue px_err_530_val = px_null();
    int px_err_530_proped = 0;
    px_srcline(765);
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_530:
    if (px_err_530_proped) return px_err_530_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pipe");
    LXValue _v531 = px_null();
    LXValue _v532 = px_null();
    LXValue _v533 = px_null();
    LXValue px_err_534_val = px_null();
    int px_err_534_proped = 0;
    px_srcline(767);
    _v531 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    px_srcline(768);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|>")}, 1))) {
        px_srcline(769);
        _v532 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(770);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(771);
        _v533 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
        px_srcline(772);
         _v531 = px_list_n((LXValue[]){px_str("Pipe"), _v531, _v533, _v532}, 4);
    }
    px_srcline(773);
    return _v531;
px_err_534:
    if (px_err_534_proped) return px_err_534_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_null_coalesce");
    LXValue _v535 = px_null();
    LXValue _v536 = px_null();
    LXValue _v537 = px_null();
    LXValue px_err_538_val = px_null();
    int px_err_538_proped = 0;
    px_srcline(775);
    _v535 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    px_srcline(776);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("??")}, 1))) {
        px_srcline(777);
        _v536 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(778);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(779);
        _v537 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
        px_srcline(780);
         _v535 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v535, _v537, _v536}, 4);
    }
    px_srcline(781);
    return _v535;
px_err_538:
    if (px_err_538_proped) return px_err_538_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_or");
    LXValue _v539 = px_null();
    LXValue _v540 = px_null();
    LXValue _v541 = px_null();
    LXValue px_err_542_val = px_null();
    int px_err_542_proped = 0;
    px_srcline(783);
    _v539 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    px_srcline(784);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("or")}, 1))) {
        px_srcline(785);
        _v540 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(786);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(787);
        _v541 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
        px_srcline(788);
         _v539 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v539, _v541, _v540}, 5);
    }
    px_srcline(789);
    return _v539;
px_err_542:
    if (px_err_542_proped) return px_err_542_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_and");
    LXValue _v543 = px_null();
    LXValue _v544 = px_null();
    LXValue _v545 = px_null();
    LXValue px_err_546_val = px_null();
    int px_err_546_proped = 0;
    px_srcline(791);
    _v543 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    px_srcline(792);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("and")}, 1))) {
        px_srcline(793);
        _v544 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(794);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(795);
        _v545 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
        px_srcline(796);
         _v543 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v543, _v545, _v544}, 5);
    }
    px_srcline(797);
    return _v543;
px_err_546:
    if (px_err_546_proped) return px_err_546_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comparison");
    LXValue _v547 = px_null();
    LXValue _v548 = px_null();
    LXValue _v549 = px_null();
    LXValue _v550 = px_null();
    LXValue px_err_551_val = px_null();
    int px_err_551_proped = 0;
    px_srcline(799);
    _v547 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    px_srcline(800);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(801);
        _v548 = px_null();
        px_srcline(802);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("==")}, 1))) {
            px_srcline(803);
             _v548 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("!=")}, 1))) {
            px_srcline(805);
             _v548 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<")}, 1))) {
            px_srcline(807);
             _v548 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<=")}, 1))) {
            px_srcline(809);
             _v548 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">")}, 1))) {
            px_srcline(811);
             _v548 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">=")}, 1))) {
            px_srcline(813);
             _v548 = px_str("Ge");
        }
        px_srcline(814);
        if (px_is_truthy(px_eq(_v548, px_null()))) {
            px_srcline(815);
            break;
        }
        px_srcline(816);
        _v549 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(817);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(818);
        _v550 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
        px_srcline(819);
         _v547 = px_list_n((LXValue[]){px_str("Binary"), _v548, _v547, _v550, _v549}, 5);
    }
    px_srcline(820);
    return _v547;
px_err_551:
    if (px_err_551_proped) return px_err_551_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitor");
    LXValue _v552 = px_null();
    LXValue _v553 = px_null();
    LXValue _v554 = px_null();
    LXValue px_err_555_val = px_null();
    int px_err_555_proped = 0;
    px_srcline(822);
    _v552 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    px_srcline(823);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|")}, 1))) {
        px_srcline(824);
        _v553 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(825);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(826);
        _v554 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
        px_srcline(827);
         _v552 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v552, _v554, _v553}, 5);
    }
    px_srcline(828);
    return _v552;
px_err_555:
    if (px_err_555_proped) return px_err_555_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitxor");
    LXValue _v556 = px_null();
    LXValue _v557 = px_null();
    LXValue _v558 = px_null();
    LXValue px_err_559_val = px_null();
    int px_err_559_proped = 0;
    px_srcline(830);
    _v556 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    px_srcline(831);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("^")}, 1))) {
        px_srcline(832);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(833);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(834);
        _v558 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
        px_srcline(835);
         _v556 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v556, _v558, _v557}, 5);
    }
    px_srcline(836);
    return _v556;
px_err_559:
    if (px_err_559_proped) return px_err_559_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitand");
    LXValue _v560 = px_null();
    LXValue _v561 = px_null();
    LXValue _v562 = px_null();
    LXValue px_err_563_val = px_null();
    int px_err_563_proped = 0;
    px_srcline(838);
    _v560 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    px_srcline(839);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("&")}, 1))) {
        px_srcline(840);
        _v561 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(841);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(842);
        _v562 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
        px_srcline(843);
         _v560 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v560, _v562, _v561}, 5);
    }
    px_srcline(844);
    return _v560;
px_err_563:
    if (px_err_563_proped) return px_err_563_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_shift");
    LXValue _v564 = px_null();
    LXValue _v565 = px_null();
    LXValue _v566 = px_null();
    LXValue _v567 = px_null();
    LXValue px_err_568_val = px_null();
    int px_err_568_proped = 0;
    px_srcline(846);
    _v564 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    px_srcline(847);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(848);
        _v565 = px_null();
        px_srcline(849);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<<")}, 1))) {
            px_srcline(850);
             _v565 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>")}, 1))) {
            px_srcline(852);
             _v565 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>>")}, 1))) {
            px_srcline(854);
             _v565 = px_str("ShrU");
        }
        px_srcline(855);
        if (px_is_truthy(px_eq(_v565, px_null()))) {
            px_srcline(856);
            break;
        }
        px_srcline(857);
        _v566 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(858);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(859);
        _v567 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
        px_srcline(860);
         _v564 = px_list_n((LXValue[]){px_str("Binary"), _v565, _v564, _v567, _v566}, 5);
    }
    px_srcline(861);
    return _v564;
px_err_568:
    if (px_err_568_proped) return px_err_568_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_add");
    LXValue _v569 = px_null();
    LXValue _v570 = px_null();
    LXValue _v571 = px_null();
    LXValue _v572 = px_null();
    LXValue px_err_573_val = px_null();
    int px_err_573_proped = 0;
    px_srcline(863);
    _v569 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    px_srcline(864);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(865);
        _v570 = px_null();
        px_srcline(866);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("+")}, 1))) {
            px_srcline(867);
             _v570 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("-")}, 1))) {
            px_srcline(869);
             _v570 = px_str("Sub");
        }
        px_srcline(870);
        if (px_is_truthy(px_eq(_v570, px_null()))) {
            px_srcline(871);
            break;
        }
        px_srcline(872);
        _v571 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(873);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(874);
        _v572 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
        px_srcline(875);
         _v569 = px_list_n((LXValue[]){px_str("Binary"), _v570, _v569, _v572, _v571}, 5);
    }
    px_srcline(876);
    return _v569;
px_err_573:
    if (px_err_573_proped) return px_err_573_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_mul");
    LXValue _v574 = px_null();
    LXValue _v575 = px_null();
    LXValue _v576 = px_null();
    LXValue _v577 = px_null();
    LXValue px_err_578_val = px_null();
    int px_err_578_proped = 0;
    px_srcline(878);
    _v574 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    px_srcline(879);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(880);
        _v575 = px_null();
        px_srcline(881);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("*")}, 1))) {
            px_srcline(882);
             _v575 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("/")}, 1))) {
            px_srcline(884);
             _v575 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("//")}, 1))) {
            px_srcline(886);
             _v575 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("%")}, 1))) {
            px_srcline(888);
             _v575 = px_str("Mod");
        }
        px_srcline(889);
        if (px_is_truthy(px_eq(_v575, px_null()))) {
            px_srcline(890);
            break;
        }
        px_srcline(891);
        _v576 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(892);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(893);
        _v577 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(894);
         _v574 = px_list_n((LXValue[]){px_str("Binary"), _v575, _v574, _v577, _v576}, 5);
    }
    px_srcline(895);
    return _v574;
px_err_578:
    if (px_err_578_proped) return px_err_578_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pow");
    LXValue _v579 = px_null();
    LXValue _v580 = px_null();
    LXValue _v581 = px_null();
    LXValue px_err_582_val = px_null();
    int px_err_582_proped = 0;
    px_srcline(897);
    _v579 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    px_srcline(898);
    if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("**")}, 1))) {
        px_srcline(899);
        _v580 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(900);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(901);
        _v581 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(902);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v579, _v581, _v580}, 5);
    }
    px_srcline(903);
    return _v579;
px_err_582:
    if (px_err_582_proped) return px_err_582_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_unary");
    LXValue _v583 = px_null();
    LXValue _v584 = px_null();
    LXValue _v585 = px_null();
    LXValue px_err_586_val = px_null();
    int px_err_586_proped = 0;
    px_srcline(912);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(913);
    _v583 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(914);
    if (px_is_truthy(px_eq(_v583, px_str("-")))) {
        px_srcline(915);
        _v584 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(916);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(917);
        _v585 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(918);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v585, _v584}, 4);
    }
    px_srcline(919);
    if (px_is_truthy(px_eq(_v583, px_str("not")))) {
        px_srcline(920);
        _v584 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(921);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(922);
        _v585 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(923);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v585, _v584}, 4);
    }
    px_srcline(924);
    if (px_is_truthy(px_eq(_v583, px_str("~")))) {
        px_srcline(925);
        _v584 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(926);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(927);
        _v585 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(928);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v585, _v584}, 4);
    }
    px_srcline(929);
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_586:
    if (px_err_586_proped) return px_err_586_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_postfix");
    LXValue _v587 = px_null();
    LXValue _v588 = px_null();
    LXValue _v589 = px_null();
    LXValue _v590 = px_null();
    LXValue _v591 = px_null();
    LXValue _v592 = px_null();
    LXValue _v593 = px_null();
    LXValue _v594 = px_null();
    LXValue px_err_595_val = px_null();
    int px_err_595_proped = 0;
    px_srcline(931);
    _v587 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    px_srcline(932);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(933);
        _v588 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(934);
        if (px_is_truthy(px_eq(_v588, px_str("(")))) {
            px_srcline(935);
            _v589 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(936);
            _v590 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(937);
             _v587 = px_list_n((LXValue[]){px_str("Call"), _v587, _v590, _v589}, 4);
        }
        else if (px_is_truthy(px_eq(_v588, px_str("[")))) {
            px_srcline(939);
            _v589 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(940);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(941);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(942);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(943);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(944);
                _v591 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                px_srcline(945);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(946);
                _v592 = px_null();
                px_srcline(947);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(948);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(949);
                     _v592 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(950);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                }
                px_srcline(951);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                px_srcline(952);
                 _v587 = px_list_n((LXValue[]){px_str("Slice"), _v587, px_null(), _v591, _v592, _v589}, 6);
            }
            else {
                px_srcline(954);
                _v593 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                px_srcline(955);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(956);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(957);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(958);
                    _v591 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(959);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    px_srcline(960);
                    _v592 = px_null();
                    px_srcline(961);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        px_srcline(962);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(963);
                         _v592 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                        px_srcline(964);
                        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    }
                    px_srcline(965);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(966);
                     _v587 = px_list_n((LXValue[]){px_str("Slice"), _v587, _v593, _v591, _v592, _v589}, 6);
                }
                else {
                    px_srcline(968);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(969);
                     _v587 = px_list_n((LXValue[]){px_str("Index"), _v587, _v593, _v589}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v588, px_str(".")))) {
            px_srcline(971);
            _v589 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(972);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(973);
            _v594 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(974);
             _v587 = px_list_n((LXValue[]){px_str("Field"), _v587, px_call(px_get_global("qstr"), (LXValue[]){_v594}, 1), _v589}, 4);
        }
        else if (px_is_truthy(px_eq(_v588, px_str("?.")))) {
            px_srcline(976);
            _v589 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(977);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(978);
            _v594 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(979);
             _v587 = px_list_n((LXValue[]){px_str("OptionalField"), _v587, px_call(px_get_global("qstr"), (LXValue[]){_v594}, 1), _v589}, 4);
        }
        else if (px_is_truthy(px_eq(_v588, px_str("!")))) {
            px_srcline(981);
            _v589 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(982);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(983);
             _v587 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v587, _v589}, 3);
        }
        else if (px_is_truthy(px_eq(_v588, px_str("?")))) {
            px_srcline(985);
            _v589 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(986);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(987);
             _v587 = px_list_n((LXValue[]){px_str("Try"), _v587, _v589}, 3);
        }
        else {
            px_srcline(989);
            break;
        }
    }
    px_srcline(990);
    return _v587;
px_err_595:
    if (px_err_595_proped) return px_err_595_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_slice_bound");
    LXValue px_err_596_val = px_null();
    int px_err_596_proped = 0;
    px_srcline(992);
    if (px_is_truthy(({ LXValue _t597 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t597) ? _t597 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        px_srcline(993);
        return px_null();
    }
    px_srcline(994);
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_596:
    if (px_err_596_proped) return px_err_596_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_call_args");
    LXValue _v598 = px_null();
    LXValue px_err_599_val = px_null();
    int px_err_599_proped = 0;
    px_srcline(996);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(997);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(998);
    _v598 = px_list_n((LXValue[]){}, 0);
    px_srcline(999);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(1000);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1001);
            (void)(px_method(_v598, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1002);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1003);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1004);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1005);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(1006);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                    px_srcline(1007);
                    break;
                }
                px_srcline(1008);
                continue;
            }
            px_srcline(1009);
            break;
        }
    }
    px_srcline(1010);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1011);
    return _v598;
px_err_599:
    if (px_err_599_proped) return px_err_599_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_primary");
    LXValue _v600 = px_null();
    LXValue _v601 = px_null();
    LXValue _v602 = px_null();
    LXValue _v603 = px_null();
    LXValue _v604 = px_null();
    LXValue _v605 = px_null();
    LXValue px_err_606_val = px_null();
    int px_err_606_proped = 0;
    px_srcline(1014);
    _v600 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1015);
    if (px_is_truthy(px_eq(_v600, px_str("整数")))) {
        px_srcline(1016);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1017);
        _v602 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1018);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1019);
        return px_list_n((LXValue[]){px_str("Int"), _v602, _v601}, 3);
    }
    px_srcline(1020);
    if (px_is_truthy(px_eq(_v600, px_str("浮点")))) {
        px_srcline(1021);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1022);
        _v602 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1023);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1024);
        return px_list_n((LXValue[]){px_str("Float"), _v602, _v601}, 3);
    }
    px_srcline(1025);
    if (px_is_truthy(px_eq(_v600, px_str("字符串")))) {
        px_srcline(1026);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1027);
        _v602 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1028);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1029);
        return px_list_n((LXValue[]){px_str("Str"), _v602, _v601}, 3);
    }
    px_srcline(1030);
    if (px_is_truthy(px_eq(_v600, px_str("true")))) {
        px_srcline(1031);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1032);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1033);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v601}, 3);
    }
    px_srcline(1034);
    if (px_is_truthy(px_eq(_v600, px_str("false")))) {
        px_srcline(1035);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1036);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1037);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v601}, 3);
    }
    px_srcline(1038);
    if (px_is_truthy(px_eq(_v600, px_str("null")))) {
        px_srcline(1039);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1040);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1041);
        return px_list_n((LXValue[]){px_str("Null"), _v601}, 2);
    }
    px_srcline(1042);
    if (px_is_truthy(px_eq(_v600, px_str("self")))) {
        px_srcline(1043);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1044);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1045);
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v601}, 3);
    }
    px_srcline(1046);
    if (px_is_truthy(px_eq(_v600, px_str("标识符")))) {
        px_srcline(1047);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1048);
        _v603 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1049);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1050);
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v603}, 1), _v601}, 3);
    }
    px_srcline(1051);
    if (px_is_truthy(px_eq(_v600, px_str("[")))) {
        px_srcline(1052);
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    px_srcline(1053);
    if (px_is_truthy(px_eq(_v600, px_str("(")))) {
        px_srcline(1054);
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    px_srcline(1055);
    if (px_is_truthy(px_eq(_v600, px_str("{")))) {
        px_srcline(1056);
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    px_srcline(1057);
    if (px_is_truthy(px_eq(_v600, px_str("fn")))) {
        px_srcline(1058);
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    px_srcline(1059);
    if (px_is_truthy(px_eq(_v600, px_str("match")))) {
        px_srcline(1060);
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1061);
    if (px_is_truthy(px_eq(_v600, px_str("if")))) {
        px_srcline(1062);
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1063);
    if (px_is_truthy(px_eq(_v600, px_str("chan")))) {
        px_srcline(1064);
        _v601 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1065);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1066);
        _v604 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v601}, 3);
        px_srcline(1067);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1068);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1069);
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            px_srcline(1070);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        px_srcline(1071);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1072);
            _v605 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(1073);
             _v604 = px_list_n((LXValue[]){px_str("Call"), _v604, _v605, _v601}, 4);
        }
        px_srcline(1074);
        return _v604;
    }
    px_srcline(1075);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1076);
    return px_null();
px_err_606:
    if (px_err_606_proped) return px_err_606_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_list_or_comp");
    LXValue _v607 = px_null();
    LXValue _v608 = px_null();
    LXValue _v609 = px_null();
    LXValue _v610 = px_null();
    LXValue px_err_611_val = px_null();
    int px_err_611_proped = 0;
    px_srcline(1078);
    _v607 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1079);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1080);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1081);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        px_srcline(1082);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1083);
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v607}, 3);
    }
    px_srcline(1084);
    _v608 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1085);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1086);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1087);
        _v609 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1088);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1089);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1090);
        return px_list_n((LXValue[]){px_str("ListComp"), _v608, px_index(_v609, px_int(0LL)), px_index(_v609, px_int(1LL)), _v607}, 5);
    }
    px_srcline(1091);
    _v610 = px_list_n((LXValue[]){_v608}, 1);
    px_srcline(1092);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1093);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1094);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1095);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            px_srcline(1096);
            break;
        }
        px_srcline(1097);
        (void)(px_method(_v610, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        px_srcline(1098);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    }
    px_srcline(1099);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(1100);
    return px_list_n((LXValue[]){px_str("List"), _v610, _v607}, 3);
px_err_611:
    if (px_err_611_proped) return px_err_611_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_vars");
    LXValue _v612 = px_null();
    LXValue px_err_613_val = px_null();
    int px_err_613_proped = 0;
    px_srcline(1102);
    _v612 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    px_srcline(1103);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1104);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1105);
        (void)(px_method(_v612, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    px_srcline(1106);
    return _v612;
px_err_613:
    if (px_err_613_proped) return px_err_613_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_clauses");
    LXValue _v614 = px_null();
    LXValue _v615 = px_null();
    LXValue _v616 = px_null();
    LXValue _v617 = px_null();
    LXValue px_err_618_val = px_null();
    int px_err_618_proped = 0;
    px_srcline(1108);
    _v614 = px_list_n((LXValue[]){}, 0);
    px_srcline(1109);
    _v615 = px_list_n((LXValue[]){}, 0);
    px_srcline(1110);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1111);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1112);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1113);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1114);
            _v616 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            px_srcline(1115);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            px_srcline(1116);
            _v617 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1117);
            (void)(px_method(_v614, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v616, _v617}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1119);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1120);
            (void)(px_method(_v615, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(1122);
            break;
        }
    }
    px_srcline(1123);
    return px_list_n((LXValue[]){_v614, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v615}, 1)}, 2);
px_err_618:
    if (px_err_618_proped) return px_err_618_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fold_comp_conds");
    LXValue _v619 = (nargs > 0) ? args[0] : px_null();
    LXValue _v620 = px_null();
    LXValue _v621 = px_null();
    LXValue _v622 = px_null();
    LXValue px_err_623_val = px_null();
    int px_err_623_proped = 0;
    px_srcline(1125);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v619}, 1), px_int(0LL)))) {
        px_srcline(1126);
        return px_null();
    }
    px_srcline(1127);
    _v620 = px_index(_v619, px_int(0LL));
    px_srcline(1128);
    _v621 = px_int(1LL);
    px_srcline(1129);
    while (px_is_truthy(px_lt(_v621, px_call(px_get_global("len"), (LXValue[]){_v619}, 1)))) {
        px_srcline(1130);
        _v622 = px_call(px_get_global("node_pos"), (LXValue[]){_v620}, 1);
        px_srcline(1131);
         _v620 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v620, px_index(_v619, _v621), _v622}, 5);
        px_srcline(1132);
         _v621 = px_add(_v621, px_int(1LL));
    }
    px_srcline(1133);
    return _v620;
px_err_623:
    if (px_err_623_proped) return px_err_623_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_paren_or_tuple");
    LXValue _v624 = px_null();
    LXValue _v625 = px_null();
    LXValue _v626 = px_null();
    LXValue _v627 = px_null();
    LXValue px_err_628_val = px_null();
    int px_err_628_proped = 0;
    px_srcline(1135);
    _v624 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1136);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1137);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1138);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        px_srcline(1139);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1140);
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v624}, 3);
    }
    px_srcline(1141);
    _v625 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1142);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1143);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1144);
        _v626 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1145);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1146);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1147);
        return px_list_n((LXValue[]){px_str("GenExp"), _v625, px_index(_v626, px_int(0LL)), px_index(_v626, px_int(1LL)), _v624}, 5);
    }
    px_srcline(1148);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1149);
        _v627 = px_list_n((LXValue[]){_v625}, 1);
        px_srcline(1150);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1151);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1152);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1153);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                px_srcline(1154);
                break;
            }
            px_srcline(1155);
            (void)(px_method(_v627, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1156);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        }
        px_srcline(1157);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1158);
        return px_list_n((LXValue[]){px_str("Tuple"), _v627, _v624}, 3);
    }
    px_srcline(1159);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1160);
    return _v625;
px_err_628:
    if (px_err_628_proped) return px_err_628_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("brace_looks_like_dict");
    LXValue _v629 = px_null();
    LXValue _v630 = px_null();
    LXValue _v631 = px_null();
    LXValue px_err_632_val = px_null();
    int px_err_632_proped = 0;
    px_srcline(1162);
    _v629 = px_int(0LL);
    px_srcline(1163);
    _v630 = px_get_global("p_pos");
    px_srcline(1164);
    while (px_is_truthy(px_lt(_v630, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(1165);
        _v631 = px_index(px_index(px_get_global("p_toks"), _v630), px_int(0LL));
        px_srcline(1166);
        if (px_is_truthy(({ LXValue _t633 = px_eq(_v631, px_str(":")); px_is_truthy(_t633) ? px_eq(_v629, px_int(0LL)) : _t633; }))) {
            px_srcline(1167);
            return px_bool(true);
        }
        px_srcline(1168);
        if (px_is_truthy(({ LXValue _t635 = ({ LXValue _t634 = px_eq(_v631, px_str("(")); px_is_truthy(_t634) ? _t634 : px_eq(_v631, px_str("[")); }); px_is_truthy(_t635) ? _t635 : px_eq(_v631, px_str("{")); }))) {
            px_srcline(1169);
             _v629 = px_add(_v629, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t636 = px_eq(_v631, px_str(")")); px_is_truthy(_t636) ? _t636 : px_eq(_v631, px_str("]")); }))) {
            px_srcline(1171);
            if (px_is_truthy(px_gt(_v629, px_int(0LL)))) {
                px_srcline(1172);
                 _v629 = px_sub(_v629, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t637 = px_eq(_v631, px_str("}")); px_is_truthy(_t637) ? px_eq(_v629, px_int(0LL)) : _t637; }))) {
            px_srcline(1174);
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t640 = ({ LXValue _t639 = ({ LXValue _t638 = px_eq(_v631, px_str(",")); px_is_truthy(_t638) ? _t638 : px_eq(_v631, px_str("换行")); }); px_is_truthy(_t639) ? _t639 : px_eq(_v631, px_str("EOF")); }); px_is_truthy(_t640) ? px_eq(_v629, px_int(0LL)) : _t640; }))) {
            px_srcline(1176);
            return px_bool(false);
        }
        px_srcline(1177);
         _v630 = px_add(_v630, px_int(1LL));
    }
    px_srcline(1178);
    return px_bool(false);
px_err_632:
    if (px_err_632_proped) return px_err_632_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_brace");
    LXValue _v641 = px_null();
    LXValue _v642 = px_null();
    LXValue _v643 = px_null();
    LXValue _v644 = px_null();
    LXValue _v645 = px_null();
    LXValue _v646 = px_null();
    LXValue _v647 = px_null();
    LXValue _v648 = px_null();
    LXValue _v649 = px_null();
    LXValue px_err_650_val = px_null();
    int px_err_650_proped = 0;
    px_srcline(1180);
    _v641 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1181);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1182);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1183);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1194);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
        px_srcline(1195);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1196);
        return px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){}, 0), _v641}, 3);
    }
    px_srcline(1197);
    _v642 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    px_srcline(1198);
    if (px_is_truthy(_v642)) {
        px_srcline(1199);
        _v643 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1200);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1201);
        _v644 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1202);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1203);
            _v645 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            px_srcline(1204);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            px_srcline(1205);
            return px_list_n((LXValue[]){px_str("DictComp"), _v643, _v644, px_index(_v645, px_int(0LL)), px_index(_v645, px_int(1LL)), _v641}, 6);
        }
        px_srcline(1206);
        _v646 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v643, _v644}, 2)}, 1);
        px_srcline(1210);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1211);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1212);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1213);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1214);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1215);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            px_srcline(1216);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                px_srcline(1217);
                break;
            }
            px_srcline(1218);
            _v647 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1219);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(1220);
            _v648 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1221);
            (void)(px_method(_v646, "append", (LXValue[]){px_list_n((LXValue[]){_v647, _v648}, 2)}, 1));
            px_srcline(1222);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1223);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1224);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1225);
        return px_list_n((LXValue[]){px_str("Dict"), _v646, _v641}, 3);
    }
    px_srcline(1226);
    _v649 = px_list_n((LXValue[]){}, 0);
    px_srcline(1227);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1228);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1229);
    while (px_is_truthy(({ LXValue _t651 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t651) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t651; }))) {
        px_srcline(1230);
        (void)(px_method(_v649, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(1231);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1232);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    px_srcline(1233);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    px_srcline(1234);
    return px_list_n((LXValue[]){px_str("Block"), _v649, _v641}, 3);
px_err_650:
    if (px_err_650_proped) return px_err_650_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_closure");
    LXValue _v652 = px_null();
    LXValue _v653 = px_null();
    LXValue _v654 = px_null();
    LXValue _v655 = px_null();
    LXValue _v656 = px_null();
    LXValue _v657 = px_null();
    LXValue _v658 = px_null();
    LXValue px_err_659_val = px_null();
    int px_err_659_proped = 0;
    px_srcline(1236);
    _v652 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1237);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1238);
    _v653 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(1239);
    _v654 = px_null();
    px_srcline(1240);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(1241);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1242);
         _v654 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(1243);
    _v655 = px_list_n((LXValue[]){}, 0);
    px_srcline(1244);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        px_srcline(1245);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1246);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1247);
            (void)(px_method(_v655, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            px_srcline(1248);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1249);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1250);
                continue;
            }
            px_srcline(1251);
            break;
        }
    }
    px_srcline(1252);
    _v656 = px_null();
    px_srcline(1253);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        px_srcline(1254);
        _v657 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1255);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1256);
        _v658 = px_list_n((LXValue[]){}, 0);
        px_srcline(1257);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1258);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1259);
        while (px_is_truthy(({ LXValue _t660 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t660) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t660; }))) {
            px_srcline(1260);
            (void)(px_method(_v658, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            px_srcline(1261);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1262);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1263);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1264);
         _v656 = px_list_n((LXValue[]){px_str("Block"), _v658, _v657}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(1266);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1270);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(1271);
            _v657 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1272);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1273);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(1274);
                _v658 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
                px_srcline(1275);
                 _v656 = px_list_n((LXValue[]){px_str("Block"), _v658, _v657}, 3);
            }
            else {
                px_srcline(1282);
                _v658 = px_list_n((LXValue[]){}, 0);
                px_srcline(1283);
                px_set_global("p_paren_ctxt", px_add(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1284);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1285);
                    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
                    px_srcline(1286);
                    if (px_is_truthy(({ LXValue _t665 = ({ LXValue _t664 = ({ LXValue _t663 = ({ LXValue _t662 = ({ LXValue _t661 = px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1); px_is_truthy(_t661) ? _t661 : px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); }); px_is_truthy(_t662) ? _t662 : px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1); }); px_is_truthy(_t663) ? _t663 : px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1); }); px_is_truthy(_t664) ? _t664 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t665) ? _t665 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
                        px_srcline(1287);
                        break;
                    }
                    px_srcline(1288);
                    (void)(px_method(_v658, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
                }
                px_srcline(1289);
                px_set_global("p_paren_ctxt", px_sub(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1290);
                 _v656 = px_list_n((LXValue[]){px_str("Block"), _v658, _v657}, 3);
            }
        }
        else {
            px_srcline(1292);
             _v656 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
    }
    else {
        px_srcline(1294);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    px_srcline(1295);
    return px_list_n((LXValue[]){px_str("Closure"), _v653, _v654, _v656, _v655, _v652}, 6);
px_err_659:
    if (px_err_659_proped) return px_err_659_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_match_expr");
    LXValue _v666 = px_null();
    LXValue _v667 = px_null();
    LXValue _v668 = px_null();
    LXValue _v669 = px_null();
    LXValue _v670 = px_null();
    LXValue _v671 = px_null();
    LXValue _v672 = px_null();
    LXValue _v673 = px_null();
    LXValue px_err_674_val = px_null();
    int px_err_674_proped = 0;
    px_srcline(1297);
    _v666 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1298);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1299);
    _v667 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1300);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1301);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(1302);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(1303);
    _v668 = px_list_n((LXValue[]){}, 0);
    px_srcline(1304);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1305);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(1306);
        if (px_is_truthy(({ LXValue _t675 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t675) ? _t675 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(1307);
            break;
        }
        px_srcline(1308);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(1309);
        _v669 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        px_srcline(1310);
        _v670 = px_null();
        px_srcline(1311);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1312);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1313);
             _v670 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1314);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1315);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(1316);
        _v671 = px_null();
        px_srcline(1317);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            px_srcline(1318);
            _v672 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(1319);
            _v673 = px_null();
            px_srcline(1320);
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v672}, 1), px_int(0LL)))) {
                px_srcline(1321);
                 _v673 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v672, px_int(0LL))}, 1);
            }
            else {
                px_srcline(1323);
                 _v673 = _v666;
            }
            px_srcline(1324);
             _v671 = px_list_n((LXValue[]){px_str("Block"), _v672, _v673}, 3);
        }
        else {
            px_srcline(1326);
             _v671 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1327);
        (void)(px_method(_v668, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v669, _v670, _v671, _v666}, 5)}, 1));
    }
    px_srcline(1328);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(1329);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    px_srcline(1330);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(1331);
    return px_list_n((LXValue[]){px_str("Match"), _v667, _v668, _v666}, 4);
px_err_674:
    if (px_err_674_proped) return px_err_674_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if_expr");
    LXValue _v676 = px_null();
    LXValue _v677 = px_null();
    LXValue _v678 = px_null();
    LXValue _v679 = px_null();
    LXValue px_err_680_val = px_null();
    int px_err_680_proped = 0;
    px_srcline(1333);
    _v676 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1334);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1335);
    _v677 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1336);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1337);
    _v678 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1338);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    px_srcline(1339);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1340);
    _v679 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1341);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v677, _v678, _v679, _v676}, 5);
px_err_680:
    if (px_err_680_proped) return px_err_680_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pattern");
    LXValue _v681 = px_null();
    LXValue _v682 = px_null();
    LXValue _v683 = px_null();
    LXValue _v684 = px_null();
    LXValue _v685 = px_null();
    LXValue _v686 = px_null();
    LXValue _v687 = px_null();
    LXValue _v688 = px_null();
    LXValue px_err_689_val = px_null();
    int px_err_689_proped = 0;
    px_srcline(1344);
    _v681 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1345);
    if (px_is_truthy(({ LXValue _t694 = ({ LXValue _t693 = ({ LXValue _t692 = ({ LXValue _t691 = ({ LXValue _t690 = px_eq(_v681, px_str("整数")); px_is_truthy(_t690) ? _t690 : px_eq(_v681, px_str("浮点")); }); px_is_truthy(_t691) ? _t691 : px_eq(_v681, px_str("字符串")); }); px_is_truthy(_t692) ? _t692 : px_eq(_v681, px_str("true")); }); px_is_truthy(_t693) ? _t693 : px_eq(_v681, px_str("false")); }); px_is_truthy(_t694) ? _t694 : px_eq(_v681, px_str("null")); }))) {
        px_srcline(1346);
        _v682 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        px_srcline(1347);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v682}, 2);
    }
    px_srcline(1348);
    if (px_is_truthy(px_eq(_v681, px_str("标识符")))) {
        px_srcline(1349);
        _v683 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1350);
        _v684 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1351);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1352);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(1354);
            _v685 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1355);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1356);
            _v686 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(1357);
            return px_list_n((LXValue[]){px_str("PatLiteral"), px_list_n((LXValue[]){px_str("Field"), px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v683}, 1), _v685}, 3), px_call(px_get_global("qstr"), (LXValue[]){_v686}, 1), _v685}, 4)}, 2);
        }
        px_srcline(1358);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1359);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1360);
            _v687 = px_list_n((LXValue[]){}, 0);
            px_srcline(1361);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(1362);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1363);
                    (void)(px_method(_v687, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    px_srcline(1364);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(1365);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(1366);
                        continue;
                    }
                    px_srcline(1367);
                    break;
                }
            }
            px_srcline(1368);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            px_srcline(1369);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v683}, 1), _v687}, 3);
        }
        px_srcline(1370);
        if (px_is_truthy(px_eq(_v683, px_str("_")))) {
            px_srcline(1371);
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        px_srcline(1372);
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v683}, 1))) {
            px_srcline(1373);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v683}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        px_srcline(1374);
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v683}, 1)}, 2);
    }
    px_srcline(1375);
    if (px_is_truthy(px_eq(_v681, px_str("(")))) {
        px_srcline(1376);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1377);
        _v688 = px_list_n((LXValue[]){}, 0);
        px_srcline(1378);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1379);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1380);
                (void)(px_method(_v688, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                px_srcline(1381);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1382);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1383);
                    continue;
                }
                px_srcline(1384);
                break;
            }
        }
        px_srcline(1385);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1386);
        return px_list_n((LXValue[]){px_str("PatTuple"), _v688}, 2);
    }
    px_srcline(1387);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1388);
    return px_null();
px_err_689:
    if (px_err_689_proped) return px_err_689_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_upper");
    LXValue _v695 = (nargs > 0) ? args[0] : px_null();
    LXValue _v696 = px_null();
    LXValue px_err_697_val = px_null();
    int px_err_697_proped = 0;
    px_srcline(1390);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v695}, 1), px_int(0LL)))) {
        px_srcline(1391);
        return px_bool(false);
    }
    px_srcline(1392);
    _v696 = px_index(_v695, px_int(0LL));
    px_srcline(1393);
    return ({ LXValue _t698 = px_ge(_v696, px_str("A")); px_is_truthy(_t698) ? px_le(_v696, px_str("Z")) : _t698; });
px_err_697:
    if (px_err_697_proped) return px_err_697_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type");
    LXValue _v699 = px_null();
    LXValue _v700 = px_null();
    LXValue px_err_701_val = px_null();
    int px_err_701_proped = 0;
    px_srcline(1396);
    _v699 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    px_srcline(1397);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        px_srcline(1398);
        _v700 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1399);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1400);
        return px_list_n((LXValue[]){px_str("TyOptional"), _v699, _v700}, 3);
    }
    px_srcline(1401);
    return _v699;
px_err_701:
    if (px_err_701_proped) return px_err_701_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_base");
    LXValue _v702 = px_null();
    LXValue _v703 = px_null();
    LXValue _v704 = px_null();
    LXValue _v705 = px_null();
    LXValue _v706 = px_null();
    LXValue _v707 = px_null();
    LXValue _v708 = px_null();
    LXValue _v709 = px_null();
    LXValue px_err_710_val = px_null();
    int px_err_710_proped = 0;
    px_srcline(1403);
    _v702 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1404);
    if (px_is_truthy(px_eq(_v702, px_str("标识符")))) {
        px_srcline(1405);
        _v703 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1406);
        _v704 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1407);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1408);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1409);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1410);
            _v705 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1411);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            px_srcline(1412);
            if (px_is_truthy(px_eq(_v703, px_str("list")))) {
                px_srcline(1413);
                return px_list_n((LXValue[]){px_str("TyList"), _v705, _v704}, 3);
            }
            px_srcline(1414);
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v703}, 1), px_list_n((LXValue[]){_v705}, 1), _v704}, 4);
        }
        px_srcline(1415);
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v703}, 1), _v704}, 3);
    }
    px_srcline(1416);
    if (px_is_truthy(px_eq(_v702, px_str("[")))) {
        px_srcline(1417);
        _v704 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1418);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1419);
        _v705 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1420);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1421);
        return px_list_n((LXValue[]){px_str("TyList"), _v705, _v704}, 3);
    }
    px_srcline(1422);
    if (px_is_truthy(px_eq(_v702, px_str("{")))) {
        px_srcline(1423);
        _v704 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1424);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1425);
        _v706 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1426);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1427);
        _v707 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1428);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1429);
        return px_list_n((LXValue[]){px_str("TyDict"), _v706, _v707, _v704}, 4);
    }
    px_srcline(1430);
    if (px_is_truthy(px_eq(_v702, px_str("(")))) {
        px_srcline(1431);
        _v704 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1432);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1433);
        _v708 = px_list_n((LXValue[]){}, 0);
        px_srcline(1434);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1435);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1436);
                (void)(px_method(_v708, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                px_srcline(1437);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1438);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1439);
                    continue;
                }
                px_srcline(1440);
                break;
            }
        }
        px_srcline(1441);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1442);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(1443);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1444);
            _v709 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1445);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v708, _v709, _v704}, 4);
        }
        px_srcline(1446);
        return px_list_n((LXValue[]){px_str("TyTuple"), _v708, _v704}, 3);
    }
    px_srcline(1447);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1448);
    return px_null();
px_err_710:
    if (px_err_710_proped) return px_err_710_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt_inner");
    LXValue _v711 = (nargs > 0) ? args[0] : px_null();
    LXValue _v712 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v743 = px_null();
    LXValue _v744 = px_null();
    LXValue _v745 = px_null();
    LXValue _v746 = px_null();
    LXValue _v747 = px_null();
    LXValue _v748 = px_null();
    LXValue _v749 = px_null();
    LXValue _v750 = px_null();
    LXValue px_err_751_val = px_null();
    int px_err_751_proped = 0;
    px_srcline(9);
    LXValue _v752 = px_call(px_get_global("cg_pad"), (LXValue[]){_v712}, 1);
    px_srcline(10);
    _v713 = px_index(_v711, px_int(0LL));
    px_srcline(11);
    if (px_is_truthy(px_eq(_v713, px_str("VarDecl")))) {
        px_srcline(12);
        _v714 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v711, px_int(2LL))}, 1);
        px_srcline(15);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v711}, 1));
        px_srcline(16);
        _v715 = px_str("px_null()");
        px_srcline(17);
        if (px_is_truthy(px_ne(px_index(_v711, px_int(4LL)), px_null()))) {
            px_srcline(18);
             _v715 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(4LL))}, 1);
        }
        px_srcline(20);
        if (px_is_truthy(({ LXValue _t753 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v714}, 2); px_is_truthy(_t753) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t753; }))) {
            px_srcline(21);
            return px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_set_global(\"")), _v714), px_str("\", ")), _v715), px_str(");\n"));
        }
        px_srcline(22);
        _v716 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v714}, 1);
        px_srcline(23);
        if (px_is_truthy(px_eq(_v716, px_null()))) {
            px_srcline(25);
             _v716 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v714}, 1);
            px_srcline(27);
            if (px_is_truthy(px_ne(px_index(_v711, px_int(4LL)), px_null()))) {
                px_srcline(28);
                _v717 = px_index(_v711, px_int(4LL));
                px_srcline(29);
                _v718 = px_null();
                px_srcline(30);
                if (px_is_truthy(px_eq(px_index(_v717, px_int(0LL)), px_str("Constructor")))) {
                    px_srcline(31);
                     _v718 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v717, px_int(1LL))}, 1);
                }
                else if (px_is_truthy(px_eq(px_index(_v717, px_int(0LL)), px_str("Call")))) {
                    px_srcline(33);
                    _v719 = px_index(_v717, px_int(1LL));
                    px_srcline(34);
                    if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Var")))) {
                        px_srcline(35);
                         _v718 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v719, px_int(1LL))}, 1);
                    }
                }
                px_srcline(36);
                if (px_is_truthy(px_ne(_v718, px_null()))) {
                    px_srcline(37);
                    if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v718}, 1))) {
                        px_srcline(38);
                        px_index_set(px_get_global("cg_var_types"), _v714, _v718);
                    }
                }
            }
            px_srcline(39);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v714}, 1))) {
                px_srcline(40);
                return px_add(px_add(px_add(px_add(px_add(_v752, px_str("LXValue ")), _v716), px_str(" = px_cell(")), _v715), px_str(");\n"));
            }
            px_srcline(41);
            return px_add(px_add(px_add(px_add(px_add(_v752, px_str("LXValue ")), _v716), px_str(" = ")), _v715), px_str(";\n"));
        }
        px_srcline(44);
        if (px_is_truthy(px_ne(px_index(_v711, px_int(4LL)), px_null()))) {
            px_srcline(45);
            _v717 = px_index(_v711, px_int(4LL));
            px_srcline(46);
            _v718 = px_null();
            px_srcline(47);
            if (px_is_truthy(px_eq(px_index(_v717, px_int(0LL)), px_str("Constructor")))) {
                px_srcline(48);
                 _v718 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v717, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v717, px_int(0LL)), px_str("Call")))) {
                px_srcline(50);
                _v719 = px_index(_v717, px_int(1LL));
                px_srcline(51);
                if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Var")))) {
                    px_srcline(52);
                     _v718 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v719, px_int(1LL))}, 1);
                }
            }
            px_srcline(53);
            if (px_is_truthy(px_ne(_v718, px_null()))) {
                px_srcline(54);
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v718}, 1))) {
                    px_srcline(55);
                    px_index_set(px_get_global("cg_var_types"), _v714, _v718);
                }
            }
        }
        px_srcline(56);
        return px_add(px_add(_v752, px_call(px_get_global("cg_store_of"), (LXValue[]){_v714, _v716, _v715}, 3)), px_str(";\n"));
    }
    px_srcline(57);
    if (px_is_truthy(px_eq(_v713, px_str("Assign")))) {
        px_srcline(58);
        _v715 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(3LL))}, 1);
        px_srcline(59);
        _v720 = px_index(_v711, px_int(1LL));
        px_srcline(60);
        _v721 = px_index(_v711, px_int(2LL));
        px_srcline(62);
        if (px_is_truthy(px_eq(_v721, px_str("Append")))) {
            px_srcline(63);
            _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v720}, 1);
            px_srcline(64);
            return px_add(px_add(px_add(px_add(px_add(_v752, px_str("(void)(px_method(")), _v722), px_str(", \"append\", (LXValue[]){")), _v715), px_str("}, 1));\n"));
        }
        px_srcline(65);
        _v723 = px_index(_v720, px_int(0LL));
        px_srcline(66);
        if (px_is_truthy(px_eq(_v723, px_str("Var")))) {
            px_srcline(67);
            _v714 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v720, px_int(1LL))}, 1);
            px_srcline(69);
            (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v720, _v721, px_index(_v711, px_int(3LL))}, 3));
            px_srcline(70);
            _v724 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v714}, 1);
            px_srcline(71);
            if (px_is_truthy(px_eq(_v724, px_null()))) {
                px_srcline(73);
                if (px_is_truthy(px_eq(_v721, px_str("Assign")))) {
                    px_srcline(74);
                    return px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_set_global(\"")), _v714), px_str("\", ")), _v715), px_str(");\n"));
                }
                px_srcline(75);
                _v725 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v721, _v714, _v715}, 3);
                px_srcline(76);
                return px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_set_global(\"")), _v714), px_str("\", ")), _v725), px_str(");\n"));
            }
            px_srcline(77);
            _v725 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v721, px_call(px_get_global("cg_load_of"), (LXValue[]){_v714, _v724}, 2), _v715}, 3);
            px_srcline(78);
            return px_add(px_add(px_add(_v752, px_str(" ")), px_call(px_get_global("cg_store_of"), (LXValue[]){_v714, _v724, _v725}, 3)), px_str(";\n"));
        }
        px_srcline(79);
        if (px_is_truthy(px_eq(_v723, px_str("Field")))) {
            px_srcline(80);
            _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v720, px_int(1LL))}, 1);
            px_srcline(81);
            _v726 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v720, px_int(2LL))}, 1);
            px_srcline(82);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_field_set(")), _v722), px_str(", \"")), _v726), px_str("\", ")), _v715), px_str(");\n"));
        }
        px_srcline(83);
        if (px_is_truthy(px_eq(_v723, px_str("Index")))) {
            px_srcline(84);
            _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v720, px_int(1LL))}, 1);
            px_srcline(85);
            _v727 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v720, px_int(2LL))}, 1);
            px_srcline(86);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_index_set(")), _v722), px_str(", ")), _v727), px_str(", ")), _v715), px_str(");\n"));
        }
        px_srcline(87);
        return px_str("不支持的赋值目标");
    }
    px_srcline(88);
    if (px_is_truthy(px_eq(_v713, px_str("ExprStmt")))) {
        px_srcline(89);
        _v717 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1);
        px_srcline(90);
        return px_add(px_add(px_add(_v752, px_str("(void)(")), _v717), px_str(");\n"));
    }
    px_srcline(91);
    if (px_is_truthy(px_eq(_v713, px_str("If")))) {
        px_srcline(92);
        _v728 = px_str("");
        px_srcline(93);
        _v729 = px_index(_v711, px_int(1LL));
        px_srcline(94);
        _v730 = px_int(0LL);
        px_srcline(95);
        while (px_is_truthy(px_lt(_v730, px_call(px_get_global("len"), (LXValue[]){_v729}, 1)))) {
            px_srcline(96);
            _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v729, _v730), px_int(0LL))}, 1);
            px_srcline(97);
            _v732 = px_str("if");
            px_srcline(98);
            if (px_is_truthy(px_gt(_v730, px_int(0LL)))) {
                px_srcline(99);
                 _v732 = px_str("else if");
            }
            px_srcline(100);
             _v728 = px_add(_v728, px_add(px_add(px_add(px_add(_v752, _v732), px_str(" (px_is_truthy(")), _v731), px_str(")) {\n")));
            px_srcline(101);
            _v733 = px_index(px_index(_v729, _v730), px_int(1LL));
            px_srcline(102);
            _v734 = px_int(0LL);
            px_srcline(103);
            while (px_is_truthy(px_lt(_v734, px_call(px_get_global("len"), (LXValue[]){_v733}, 1)))) {
                px_srcline(104);
                 _v728 = px_add(_v728, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v733, _v734), px_add(_v712, px_int(1LL))}, 2));
                px_srcline(105);
                 _v734 = px_add(_v734, px_int(1LL));
            }
            px_srcline(106);
             _v728 = px_add(_v728, px_add(_v752, px_str("}\n")));
            px_srcline(107);
             _v730 = px_add(_v730, px_int(1LL));
        }
        px_srcline(108);
        if (px_is_truthy(px_ne(px_index(_v711, px_int(2LL)), px_null()))) {
            px_srcline(109);
             _v728 = px_add(_v728, px_add(_v752, px_str("else {\n")));
            px_srcline(110);
            _v735 = px_index(_v711, px_int(2LL));
            px_srcline(111);
            _v736 = px_int(0LL);
            px_srcline(112);
            while (px_is_truthy(px_lt(_v736, px_call(px_get_global("len"), (LXValue[]){_v735}, 1)))) {
                px_srcline(113);
                 _v728 = px_add(_v728, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v735, _v736), px_add(_v712, px_int(1LL))}, 2));
                px_srcline(114);
                 _v736 = px_add(_v736, px_int(1LL));
            }
            px_srcline(115);
             _v728 = px_add(_v728, px_add(_v752, px_str("}\n")));
        }
        px_srcline(116);
        return _v728;
    }
    px_srcline(117);
    if (px_is_truthy(px_eq(_v713, px_str("While")))) {
        px_srcline(118);
        _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1);
        px_srcline(119);
        _v728 = px_add(px_add(px_add(_v752, px_str("while (px_is_truthy(")), _v731), px_str(")) {\n"));
        px_srcline(120);
        _v733 = px_index(_v711, px_int(2LL));
        px_srcline(121);
        _v730 = px_int(0LL);
        px_srcline(122);
        while (px_is_truthy(px_lt(_v730, px_call(px_get_global("len"), (LXValue[]){_v733}, 1)))) {
            px_srcline(123);
             _v728 = px_add(_v728, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v733, _v730), px_add(_v712, px_int(1LL))}, 2));
            px_srcline(124);
             _v730 = px_add(_v730, px_int(1LL));
        }
        px_srcline(125);
         _v728 = px_add(_v728, px_add(_v752, px_str("}\n")));
        px_srcline(126);
        return _v728;
    }
    px_srcline(127);
    if (px_is_truthy(px_eq(_v713, px_str("For")))) {
        px_srcline(128);
        _v737 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(2LL))}, 1);
        px_srcline(129);
        _v738 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(130);
        _v739 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(131);
        _v740 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1);
        px_srcline(132);
        _v741 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v740}, 1);
        px_srcline(133);
        _v742 = px_str("LXValue ");
        px_srcline(134);
        if (px_is_truthy(({ LXValue _t754 = px_eq(_v741, px_null()); px_is_truthy(_t754) ? _t754 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)); }))) {
            px_srcline(137);
             _v741 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v740}, 1);
        }
        else {
            px_srcline(140);
             _v742 = px_str("");
        }
        px_srcline(141);
        _v743 = px_add(px_add(px_add(px_add(px_str("px_index("), _v738), px_str(", px_int(")), _v739), px_str("))"));
        px_srcline(142);
        _v744 = px_str("");
        px_srcline(143);
        if (px_is_truthy(px_eq(_v742, px_str("LXValue ")))) {
            px_srcline(145);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v740}, 1))) {
                px_srcline(146);
                 _v744 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v741), px_str(" = px_cell(")), _v743), px_str(");"));
            }
            else {
                px_srcline(148);
                 _v744 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v741), px_str(" = ")), _v743), px_str(";"));
            }
        }
        else {
            px_srcline(151);
             _v744 = px_add(px_call(px_get_global("cg_store_of"), (LXValue[]){_v740, _v741, _v743}, 3), px_str(";"));
        }
        px_srcline(152);
        _v728 = px_add(px_add(px_add(px_add(px_add(_v752, px_str("LXValue ")), _v738), px_str(" = ")), _v737), px_str(";\n"));
        px_srcline(153);
         _v728 = px_add(_v728, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v752, px_str("for (int ")), _v739), px_str(" = 0; ")), _v739), px_str(" < px_len(")), _v738), px_str("); ")), _v739), px_str("++) {\n")));
        px_srcline(154);
         _v728 = px_add(_v728, px_add(px_add(px_add(_v752, px_str("    ")), _v744), px_str("\n")));
        px_srcline(155);
        _v733 = px_index(_v711, px_int(3LL));
        px_srcline(156);
        _v730 = px_int(0LL);
        px_srcline(157);
        while (px_is_truthy(px_lt(_v730, px_call(px_get_global("len"), (LXValue[]){_v733}, 1)))) {
            px_srcline(158);
             _v728 = px_add(_v728, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v733, _v730), px_add(_v712, px_int(1LL))}, 2));
            px_srcline(159);
             _v730 = px_add(_v730, px_int(1LL));
        }
        px_srcline(160);
         _v728 = px_add(_v728, px_add(_v752, px_str("}\n")));
        px_srcline(161);
        return _v728;
    }
    px_srcline(162);
    if (px_is_truthy(px_eq(_v713, px_str("Return")))) {
        px_srcline(163);
        if (px_is_truthy(px_ne(px_index(_v711, px_int(1LL)), px_null()))) {
            px_srcline(164);
            _v717 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1);
            px_srcline(165);
            return px_add(px_add(px_add(_v752, px_str("return ")), _v717), px_str(";\n"));
        }
        px_srcline(166);
        return px_add(_v752, px_str("return px_null();\n"));
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v713, px_str("Break")))) {
        px_srcline(168);
        return px_add(_v752, px_str("break;\n"));
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v713, px_str("Continue")))) {
        px_srcline(170);
        return px_add(_v752, px_str("continue;\n"));
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v713, px_str("Empty")))) {
        px_srcline(172);
        return px_str("");
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(_v713, px_str("ChanDecl")))) {
        px_srcline(174);
        _v724 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1)}, 1);
        px_srcline(175);
        return px_add(px_add(px_add(_v752, px_str("LXValue ")), _v724), px_str(" = px_chan_create(0);\n"));
    }
    px_srcline(176);
    if (px_is_truthy(px_eq(_v713, px_str("Send")))) {
        px_srcline(177);
        _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1);
        px_srcline(178);
        _v724 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(2LL))}, 1);
        px_srcline(179);
        return px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_chan_send(")), _v731), px_str(", ")), _v724), px_str(");\n"));
    }
    px_srcline(180);
    if (px_is_truthy(px_eq(_v713, px_str("Recv")))) {
        px_srcline(181);
        _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v711, px_int(1LL))}, 1);
        px_srcline(182);
        return px_add(px_add(px_add(_v752, px_str("px_chan_recv(")), _v731), px_str(");\n"));
    }
    px_srcline(183);
    if (px_is_truthy(px_eq(_v713, px_str("Spawn")))) {
        px_srcline(184);
        _v745 = px_index(_v711, px_int(1LL));
        px_srcline(185);
        if (px_is_truthy(px_eq(px_index(_v745, px_int(0LL)), px_str("Call")))) {
            px_srcline(186);
            _v719 = px_index(_v745, px_int(1LL));
            px_srcline(187);
            if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Var")))) {
                px_srcline(188);
                _v726 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v719, px_int(1LL))}, 1);
                px_srcline(189);
                _v746 = px_list_n((LXValue[]){}, 0);
                px_srcline(190);
                _v747 = px_index(_v745, px_int(2LL));
                px_srcline(191);
                _v748 = px_int(0LL);
                px_srcline(192);
                while (px_is_truthy(px_lt(_v748, px_call(px_get_global("len"), (LXValue[]){_v747}, 1)))) {
                    px_srcline(193);
                    (void)(px_method(_v746, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v747, _v748)}, 1)}, 1));
                    px_srcline(194);
                     _v748 = px_add(_v748, px_int(1LL));
                }
                px_srcline(195);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v752, px_str("px_spawn_name(\"")), _v726), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v746}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v746}, 1)}, 1)), px_str(");\n"));
            }
            px_srcline(196);
            _v749 = px_index(_v711, px_int(2LL));
            px_srcline(197);
            (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v749, px_int(0LL))}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_index(_v749, px_int(1LL))}, 1)), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)（方法调用请先绑定命名函数）"))}, 1));
            px_srcline(198);
            (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
        }
        px_srcline(199);
        _v750 = px_index(_v711, px_int(2LL));
        px_srcline(200);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v750, px_int(0LL))}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_index(_v750, px_int(1LL))}, 1)), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()）"))}, 1));
        px_srcline(201);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(202);
    if (px_is_truthy(px_eq(_v713, px_str("Select")))) {
        px_srcline(203);
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v711, px_int(1LL)), px_index(_v711, px_int(2LL)), _v712}, 3);
    }
    px_srcline(204);
    if (px_is_truthy(px_eq(_v713, px_str("Import")))) {
        px_srcline(205);
        return px_add(_v752, px_str("/* import 忽略（MVP） */\n"));
    }
    px_srcline(207);
    return px_str("");
px_err_751:
    if (px_err_751_proped) return px_err_751_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt");
    LXValue _v755 = (nargs > 0) ? args[0] : px_null();
    LXValue _v756 = (nargs > 1) ? args[1] : px_null();
    LXValue _v757 = px_null();
    LXValue _v758 = px_null();
    LXValue _v759 = px_null();
    LXValue px_err_760_val = px_null();
    int px_err_760_proped = 0;
    px_srcline(213);
    _v757 = px_int(0LL);
    px_srcline(214);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v755}, 1), px_int(0LL)))) {
        px_srcline(215);
        _v758 = px_index(_v755, px_sub(px_call(px_get_global("len"), (LXValue[]){_v755}, 1), px_int(1LL)));
        px_srcline(216);
        if (px_is_truthy(({ LXValue _t762 = ({ LXValue _t761 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v758}, 1), px_str("list")); px_is_truthy(_t761) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v758}, 1), px_int(1LL)) : _t761; }); px_is_truthy(_t762) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v758, px_int(0LL))}, 1), px_str("int")) : _t762; }))) {
            px_srcline(217);
             _v757 = px_index(_v758, px_int(0LL));
        }
    }
    px_srcline(218);
    _v759 = px_str("");
    px_srcline(219);
    if (px_is_truthy(px_gt(_v757, px_int(0LL)))) {
        px_srcline(220);
         _v759 = px_add(px_add(px_add(px_call(px_get_global("cg_pad"), (LXValue[]){_v756}, 1), px_str("px_srcline(")), px_call(px_get_global("str"), (LXValue[]){_v757}, 1)), px_str(");\n"));
    }
    px_srcline(221);
    return px_add(_v759, px_call(px_get_global("cg_gen_stmt_inner"), (LXValue[]){_v755, _v756}, 2));
px_err_760:
    if (px_err_760_proped) return px_err_760_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_global");
    LXValue _v763 = (nargs > 0) ? args[0] : px_null();
    LXValue _v764 = (nargs > 1) ? args[1] : px_null();
    LXValue _v765 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_766_val = px_null();
    int px_err_766_proped = 0;
    px_srcline(224);
    if (px_is_truthy(px_eq(_v763, px_str("Assign")))) {
        px_srcline(225);
        return _v765;
    }
    px_srcline(226);
    if (px_is_truthy(px_eq(_v763, px_str("Plus")))) {
        px_srcline(227);
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(228);
    if (px_is_truthy(px_eq(_v763, px_str("Minus")))) {
        px_srcline(229);
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(230);
    if (px_is_truthy(px_eq(_v763, px_str("Star")))) {
        px_srcline(231);
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(232);
    if (px_is_truthy(px_eq(_v763, px_str("Slash")))) {
        px_srcline(233);
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(234);
    if (px_is_truthy(px_eq(_v763, px_str("IntDiv")))) {
        px_srcline(235);
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(236);
    if (px_is_truthy(px_eq(_v763, px_str("Mod")))) {
        px_srcline(237);
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(238);
    if (px_is_truthy(px_eq(_v763, px_str("Pow")))) {
        px_srcline(239);
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(240);
    if (px_is_truthy(px_eq(_v763, px_str("BitAnd")))) {
        px_srcline(241);
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(242);
    if (px_is_truthy(px_eq(_v763, px_str("BitOr")))) {
        px_srcline(243);
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(244);
    if (px_is_truthy(px_eq(_v763, px_str("BitXor")))) {
        px_srcline(245);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(246);
    if (px_is_truthy(px_eq(_v763, px_str("Shl")))) {
        px_srcline(247);
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(248);
    if (px_is_truthy(px_eq(_v763, px_str("Shr")))) {
        px_srcline(249);
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(250);
    if (px_is_truthy(px_eq(_v763, px_str("ShrU")))) {
        px_srcline(251);
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v764), px_str("\"), ")), _v765), px_str(")"));
    }
    px_srcline(252);
    return _v765;
px_err_766:
    if (px_err_766_proped) return px_err_766_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_local");
    LXValue _v767 = (nargs > 0) ? args[0] : px_null();
    LXValue _v768 = (nargs > 1) ? args[1] : px_null();
    LXValue _v769 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_770_val = px_null();
    int px_err_770_proped = 0;
    px_srcline(255);
    if (px_is_truthy(px_eq(_v767, px_str("Assign")))) {
        px_srcline(256);
        return _v769;
    }
    px_srcline(257);
    if (px_is_truthy(px_eq(_v767, px_str("Plus")))) {
        px_srcline(258);
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(259);
    if (px_is_truthy(px_eq(_v767, px_str("Minus")))) {
        px_srcline(260);
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(261);
    if (px_is_truthy(px_eq(_v767, px_str("Star")))) {
        px_srcline(262);
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(263);
    if (px_is_truthy(px_eq(_v767, px_str("Slash")))) {
        px_srcline(264);
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(265);
    if (px_is_truthy(px_eq(_v767, px_str("IntDiv")))) {
        px_srcline(266);
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(267);
    if (px_is_truthy(px_eq(_v767, px_str("Mod")))) {
        px_srcline(268);
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(269);
    if (px_is_truthy(px_eq(_v767, px_str("Pow")))) {
        px_srcline(270);
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(271);
    if (px_is_truthy(px_eq(_v767, px_str("BitAnd")))) {
        px_srcline(272);
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(273);
    if (px_is_truthy(px_eq(_v767, px_str("BitOr")))) {
        px_srcline(274);
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(275);
    if (px_is_truthy(px_eq(_v767, px_str("BitXor")))) {
        px_srcline(276);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(277);
    if (px_is_truthy(px_eq(_v767, px_str("Shl")))) {
        px_srcline(278);
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(279);
    if (px_is_truthy(px_eq(_v767, px_str("Shr")))) {
        px_srcline(280);
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(281);
    if (px_is_truthy(px_eq(_v767, px_str("ShrU")))) {
        px_srcline(282);
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v768), px_str(", ")), _v769), px_str(")"));
    }
    px_srcline(283);
    return _v769;
px_err_770:
    if (px_err_770_proped) return px_err_770_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_select");
    LXValue _v771 = (nargs > 0) ? args[0] : px_null();
    LXValue _v772 = (nargs > 1) ? args[1] : px_null();
    LXValue _v773 = (nargs > 2) ? args[2] : px_null();
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
    LXValue px_err_795_val = px_null();
    int px_err_795_proped = 0;
    px_srcline(286);
    LXValue _v796 = px_call(px_get_global("cg_pad"), (LXValue[]){_v773}, 1);
    px_srcline(287);
    _v774 = px_call(px_get_global("len"), (LXValue[]){_v771}, 1);
    px_srcline(288);
    if (px_is_truthy(px_eq(_v774, px_int(0LL)))) {
        px_srcline(289);
        return px_str("select 至少需要一个 case 分支");
    }
    px_srcline(290);
    _v775 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    px_srcline(291);
    _v776 = px_str("");
    px_srcline(292);
    _v777 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(293);
    _v778 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(294);
    _v779 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(296);
    _v780 = px_list_n((LXValue[]){}, 0);
    px_srcline(297);
    _v781 = px_int(0LL);
    px_srcline(298);
    while (px_is_truthy(px_lt(_v781, _v774))) {
        px_srcline(299);
        _v782 = px_index(px_index(_v771, _v781), px_int(1LL));
        px_srcline(300);
        if (px_is_truthy(px_eq(px_index(_v782, px_int(0LL)), px_str("Call")))) {
            px_srcline(301);
            _v783 = px_index(_v782, px_int(1LL));
            px_srcline(302);
            if (px_is_truthy(px_eq(px_index(_v783, px_int(0LL)), px_str("Field")))) {
                px_srcline(303);
                _v784 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v783, px_int(2LL))}, 1);
                px_srcline(304);
                if (px_is_truthy(px_eq(_v784, px_str("recv")))) {
                    px_srcline(305);
                    (void)(px_method(_v780, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v783, px_int(1LL))}, 1)}, 1));
                    px_srcline(306);
                     _v781 = px_add(_v781, px_int(1LL));
                    px_srcline(307);
                    continue;
                }
                px_srcline(308);
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v784), px_str("）"));
            }
            px_srcline(309);
            return px_str("select case 仅支持 ch.recv()");
        }
        px_srcline(310);
        return px_str("select case 仅支持 ch.recv()");
    }
    px_srcline(311);
     _v776 = px_add(_v776, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v796, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v774}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v780}, 2)), px_str("};\n")));
    px_srcline(312);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(": {\n")));
    px_srcline(313);
    _v785 = px_list_n((LXValue[]){}, 0);
    px_srcline(314);
    _v786 = px_int(0LL);
    px_srcline(315);
    while (px_is_truthy(px_lt(_v786, _v774))) {
        px_srcline(316);
        (void)(px_method(_v785, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v786}, 1)}, 1));
        px_srcline(317);
         _v786 = px_add(_v786, px_int(1LL));
    }
    px_srcline(318);
     _v776 = px_add(_v776, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v796, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v774}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v785}, 2)), px_str("};\n")));
    px_srcline(319);
    if (px_is_truthy(px_gt(_v774, px_int(1LL)))) {
        px_srcline(320);
        _v787 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v796, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v774}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("--) { "));
        px_srcline(321);
         _v787 = px_add(_v787, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" + 1); ")));
        px_srcline(322);
         _v787 = px_add(_v787, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("; ")));
        px_srcline(323);
         _v787 = px_add(_v787, px_str("}\n"));
        px_srcline(324);
         _v776 = px_add(_v776, _v787);
    }
    px_srcline(325);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = px_null();\n")));
    px_srcline(326);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = -1;\n")));
    px_srcline(327);
     _v776 = px_add(_v776, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v796, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v774}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("++) {\n")));
    px_srcline(328);
     _v776 = px_add(_v776, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v796, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("];\n")));
    px_srcline(329);
     _v776 = px_add(_v776, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v796, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str("; break; }\n")));
    px_srcline(330);
     _v776 = px_add(_v776, px_add(_v796, px_str("    }\n")));
    px_srcline(332);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" >= 0) {\n")));
    px_srcline(333);
    _v788 = px_int(0LL);
    px_srcline(334);
    while (px_is_truthy(px_lt(_v788, _v774))) {
        px_srcline(335);
        _v789 = px_index(px_index(_v771, _v788), px_int(0LL));
        px_srcline(336);
        _v790 = px_index(px_index(_v771, _v788), px_int(2LL));
        px_srcline(337);
        _v791 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v788}, 1)), px_str(")"));
        px_srcline(338);
        if (px_is_truthy(px_gt(_v788, px_int(0LL)))) {
            px_srcline(339);
             _v791 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v788}, 1)), px_str(")"));
        }
        px_srcline(340);
         _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("        ")), _v791), px_str(" {\n")));
        px_srcline(341);
        if (px_is_truthy(px_ne(_v789, px_null()))) {
            px_srcline(342);
            _v792 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v789}, 1)}, 1);
            px_srcline(343);
             _v776 = px_add(_v776, px_add(px_add(px_add(px_add(px_add(_v796, px_str("            LXValue ")), _v792), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(";\n")));
        }
        px_srcline(344);
        _v793 = px_int(0LL);
        px_srcline(345);
        while (px_is_truthy(px_lt(_v793, px_call(px_get_global("len"), (LXValue[]){_v790}, 1)))) {
            px_srcline(346);
             _v776 = px_add(_v776, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v790, _v793), px_add(_v773, px_int(3LL))}, 2));
            px_srcline(347);
             _v793 = px_add(_v793, px_int(1LL));
        }
        px_srcline(348);
         _v776 = px_add(_v776, px_add(_v796, px_str("        }\n")));
        px_srcline(349);
         _v788 = px_add(_v788, px_int(1LL));
    }
    px_srcline(350);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(";\n")));
    px_srcline(351);
     _v776 = px_add(_v776, px_add(_v796, px_str("    }\n")));
    px_srcline(353);
    if (px_is_truthy(px_ne(_v772, px_null()))) {
        px_srcline(354);
         _v776 = px_add(_v776, px_add(_v796, px_str("    {\n")));
        px_srcline(355);
        _v794 = px_int(0LL);
        px_srcline(356);
        while (px_is_truthy(px_lt(_v794, px_call(px_get_global("len"), (LXValue[]){_v772}, 1)))) {
            px_srcline(357);
             _v776 = px_add(_v776, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v772, _v794), px_add(_v773, px_int(2LL))}, 2));
            px_srcline(358);
             _v794 = px_add(_v794, px_int(1LL));
        }
        px_srcline(359);
         _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(";\n")));
        px_srcline(360);
         _v776 = px_add(_v776, px_add(_v796, px_str("    }\n")));
    }
    px_srcline(362);
     _v776 = px_add(_v776, px_add(_v796, px_str("    px_select_wait();\n")));
    px_srcline(363);
     _v776 = px_add(_v776, px_add(_v796, px_str("}\n")));
    px_srcline(364);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(";\n")));
    px_srcline(365);
     _v776 = px_add(_v776, px_add(px_add(px_add(_v796, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v775}, 1)), px_str(": ;\n")));
    px_srcline(366);
    px_set_global("cg_vars", _v777);
    px_srcline(367);
    px_set_global("cg_var_types", _v778);
    px_srcline(368);
    px_set_global("cg_cells", _v779);
    px_srcline(369);
    return _v776;
px_err_795:
    if (px_err_795_proped) return px_err_795_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_collect");
    LXValue _v797 = (nargs > 0) ? args[0] : px_null();
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
    LXValue px_err_808_val = px_null();
    int px_err_808_proped = 0;
    px_srcline(9);
    _v798 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(10);
    (void)(px_method(_v798, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(11);
    px_index_set(_v798, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_srcline(12);
    px_index_set(_v798, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_srcline(13);
    px_index_set(_v798, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_srcline(14);
    px_index_set(_v798, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_srcline(15);
    px_index_set(_v798, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_srcline(16);
    px_index_set(_v798, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    px_srcline(17);
    _v799 = px_int(0LL);
    px_srcline(18);
    while (px_is_truthy(px_lt(_v799, px_call(px_get_global("len"), (LXValue[]){_v797}, 1)))) {
        px_srcline(19);
        _v800 = px_index(_v797, _v799);
        px_srcline(20);
        _v801 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v800, px_int(2LL))}, 1);
        px_srcline(21);
        (void)(px_method(px_index(_v798, px_str("its")), "append", (LXValue[]){_v801}, 1));
        px_srcline(22);
        (void)(px_method(px_index(_v798, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(23);
        (void)(px_method(px_index(_v798, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(24);
        (void)(px_method(px_index(_v798, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(25);
        _v802 = px_str("");
        px_srcline(26);
        _v803 = px_list_n((LXValue[]){}, 0);
        px_srcline(27);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v800, px_int(1LL))}, 1), px_int(1LL)))) {
            px_srcline(28);
            _v804 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            px_srcline(29);
            _v805 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v800, px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(30);
            _v806 = px_null();
            px_srcline(31);
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v805}, 1))) {
                px_srcline(32);
                 _v806 = px_index(px_get_global("cg_vars"), _v805);
            }
            px_srcline(33);
            px_index_set(px_get_global("cg_vars"), _v805, _v804);
            px_srcline(34);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v805}, 1))) {
                px_srcline(35);
                (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v805}, 1));
            }
            px_srcline(36);
            (void)(px_method(_v803, "append", (LXValue[]){px_list_n((LXValue[]){_v805, _v806}, 2)}, 1));
            px_srcline(37);
             _v802 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v804), px_str(" = ")), px_index(px_index(_v798, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v798, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            px_srcline(39);
            _v807 = px_int(0LL);
            px_srcline(40);
            while (px_is_truthy(px_lt(_v807, px_call(px_get_global("len"), (LXValue[]){px_index(_v800, px_int(1LL))}, 1)))) {
                px_srcline(41);
                _v805 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v800, px_int(1LL)), _v807)}, 1);
                px_srcline(42);
                _v804 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v807}, 1));
                px_srcline(43);
                _v806 = px_null();
                px_srcline(44);
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v805}, 1))) {
                    px_srcline(45);
                     _v806 = px_index(px_get_global("cg_vars"), _v805);
                }
                px_srcline(46);
                px_index_set(px_get_global("cg_vars"), _v805, _v804);
                px_srcline(47);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v805}, 1))) {
                    px_srcline(48);
                    (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v805}, 1));
                }
                px_srcline(49);
                (void)(px_method(_v803, "append", (LXValue[]){px_list_n((LXValue[]){_v805, _v806}, 2)}, 1));
                px_srcline(50);
                 _v802 = px_add(_v802, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v804), px_str(" = px_index(")), px_index(px_index(_v798, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v798, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v807}, 1)), px_str(")); ")));
                px_srcline(51);
                 _v807 = px_add(_v807, px_int(1LL));
            }
        }
        px_srcline(52);
        (void)(px_method(px_index(_v798, px_str("binds")), "append", (LXValue[]){_v802}, 1));
        px_srcline(53);
        (void)(px_method(px_index(_v798, px_str("saved_all")), "append", (LXValue[]){_v803}, 1));
        px_srcline(54);
         _v799 = px_add(_v799, px_int(1LL));
    }
    px_srcline(55);
    return _v798;
px_err_808:
    if (px_err_808_proped) return px_err_808_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_restore");
    LXValue _v809 = (nargs > 0) ? args[0] : px_null();
    LXValue _v810 = px_null();
    LXValue _v811 = px_null();
    LXValue _v812 = px_null();
    LXValue _v813 = px_null();
    LXValue _v814 = px_null();
    LXValue px_err_815_val = px_null();
    int px_err_815_proped = 0;
    px_srcline(57);
    _v810 = px_int(0LL);
    px_srcline(58);
    while (px_is_truthy(px_lt(_v810, px_call(px_get_global("len"), (LXValue[]){_v809}, 1)))) {
        px_srcline(59);
        _v811 = px_index(_v809, _v810);
        px_srcline(60);
        _v812 = px_int(0LL);
        px_srcline(61);
        while (px_is_truthy(px_lt(_v812, px_call(px_get_global("len"), (LXValue[]){_v811}, 1)))) {
            px_srcline(62);
            _v813 = px_index(px_index(_v811, _v812), px_int(0LL));
            px_srcline(63);
            _v814 = px_index(px_index(_v811, _v812), px_int(1LL));
            px_srcline(64);
            if (px_is_truthy(px_eq(_v814, px_null()))) {
                px_srcline(65);
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v813}, 1));
            }
            else {
                px_srcline(67);
                px_index_set(px_get_global("cg_vars"), _v813, _v814);
            }
            px_srcline(68);
             _v812 = px_add(_v812, px_int(1LL));
        }
        px_srcline(69);
         _v810 = px_add(_v810, px_int(1LL));
    }
px_err_815:
    if (px_err_815_proped) return px_err_815_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_body");
    LXValue _v816 = (nargs > 0) ? args[0] : px_null();
    LXValue _v817 = (nargs > 1) ? args[1] : px_null();
    LXValue _v818 = (nargs > 2) ? args[2] : px_null();
    LXValue _v819 = px_null();
    LXValue _v820 = px_null();
    LXValue _v821 = px_null();
    LXValue _v822 = px_null();
    LXValue px_err_823_val = px_null();
    int px_err_823_proped = 0;
    px_srcline(72);
    _v819 = px_str("");
    px_srcline(73);
    if (px_is_truthy(px_ne(_v817, px_null()))) {
        px_srcline(74);
         _v819 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v817), px_str(")) { ")), _v818), px_str("} "));
    }
    else {
        px_srcline(76);
         _v819 = _v818;
    }
    px_srcline(77);
    _v820 = px_call(px_get_global("len"), (LXValue[]){px_index(_v816, px_str("its"))}, 1);
    px_srcline(78);
    _v821 = px_sub(_v820, px_int(1LL));
    px_srcline(79);
    while (px_is_truthy(px_ge(_v821, px_int(0LL)))) {
        px_srcline(80);
        _v822 = px_str("");
        px_srcline(81);
        if (px_is_truthy(px_lt(px_add(_v821, px_int(1LL)), _v820))) {
            px_srcline(82);
             _v822 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v816, px_str("ivs")), px_add(_v821, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v816, px_str("its")), px_add(_v821, px_int(1LL)))), px_str("; "));
        }
        px_srcline(83);
         _v819 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v816, px_str("idxs")), _v821)), px_str("=0; ")), px_index(px_index(_v816, px_str("idxs")), _v821)), px_str("<px_len(")), px_index(px_index(_v816, px_str("ivs")), _v821)), px_str("); ")), px_index(px_index(_v816, px_str("idxs")), _v821)), px_str("++) { LXValue ")), px_index(px_index(_v816, px_str("itms")), _v821)), px_str(" = px_index(")), px_index(px_index(_v816, px_str("ivs")), _v821)), px_str(", px_int(")), px_index(px_index(_v816, px_str("idxs")), _v821)), px_str(")); ")), px_index(px_index(_v816, px_str("binds")), _v821)), _v822), _v819), px_str(" } "));
        px_srcline(84);
         _v821 = px_sub(_v821, px_int(1LL));
    }
    px_srcline(85);
    return _v819;
px_err_823:
    if (px_err_823_proped) return px_err_823_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_expr");
    LXValue _v824 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v837 = px_null();
    LXValue _v838 = px_null();
    LXValue _v839 = px_null();
    LXValue _v840 = px_null();
    LXValue _v841 = px_null();
    LXValue _v842 = px_null();
    LXValue _v843 = px_null();
    LXValue _v844 = px_null();
    LXValue _v845 = px_null();
    LXValue _v846 = px_null();
    LXValue _v847 = px_null();
    LXValue _v848 = px_null();
    LXValue _v849 = px_null();
    LXValue _v850 = px_null();
    LXValue _v851 = px_null();
    LXValue _v852 = px_null();
    LXValue _v853 = px_null();
    LXValue _v854 = px_null();
    LXValue _v855 = px_null();
    LXValue _v856 = px_null();
    LXValue _v857 = px_null();
    LXValue _v858 = px_null();
    LXValue _v859 = px_null();
    LXValue _v860 = px_null();
    LXValue _v861 = px_null();
    LXValue _v862 = px_null();
    LXValue _v863 = px_null();
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
    LXValue _v890 = px_null();
    LXValue _v891 = px_null();
    LXValue _v892 = px_null();
    LXValue _v893 = px_null();
    LXValue _v894 = px_null();
    LXValue _v895 = px_null();
    LXValue _v896 = px_null();
    LXValue px_err_897_val = px_null();
    int px_err_897_proped = 0;
    px_srcline(88);
    _v825 = px_index(_v824, px_int(0LL));
    px_srcline(89);
    if (px_is_truthy(px_eq(_v825, px_str("Int")))) {
        px_srcline(90);
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1)), px_str("LL)"));
    }
    px_srcline(91);
    if (px_is_truthy(px_eq(_v825, px_str("Float")))) {
        px_srcline(92);
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1)), px_str(")"));
    }
    px_srcline(93);
    if (px_is_truthy(px_eq(_v825, px_str("Str")))) {
        px_srcline(94);
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1)}, 1)), px_str("\")"));
    }
    px_srcline(95);
    if (px_is_truthy(px_eq(_v825, px_str("Bool")))) {
        px_srcline(96);
        if (px_is_truthy(px_index(_v824, px_int(1LL)))) {
            px_srcline(97);
            return px_str("px_bool(true)");
        }
        px_srcline(98);
        return px_str("px_bool(false)");
    }
    px_srcline(99);
    if (px_is_truthy(px_eq(_v825, px_str("Null")))) {
        px_srcline(100);
        return px_str("px_null()");
    }
    px_srcline(101);
    if (px_is_truthy(px_eq(_v825, px_str("List")))) {
        px_srcline(102);
        _v826 = px_list_n((LXValue[]){}, 0);
        px_srcline(103);
        _v827 = px_index(_v824, px_int(1LL));
        px_srcline(104);
        _v828 = px_int(0LL);
        px_srcline(105);
        while (px_is_truthy(px_lt(_v828, px_call(px_get_global("len"), (LXValue[]){_v827}, 1)))) {
            px_srcline(106);
            (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v827, _v828)}, 1)}, 1));
            px_srcline(107);
             _v828 = px_add(_v828, px_int(1LL));
        }
        px_srcline(108);
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")"));
    }
    px_srcline(109);
    if (px_is_truthy(px_eq(_v825, px_str("Tuple")))) {
        px_srcline(110);
        _v826 = px_list_n((LXValue[]){}, 0);
        px_srcline(111);
        _v827 = px_index(_v824, px_int(1LL));
        px_srcline(112);
        _v828 = px_int(0LL);
        px_srcline(113);
        while (px_is_truthy(px_lt(_v828, px_call(px_get_global("len"), (LXValue[]){_v827}, 1)))) {
            px_srcline(114);
            (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v827, _v828)}, 1)}, 1));
            px_srcline(115);
             _v828 = px_add(_v828, px_int(1LL));
        }
        px_srcline(116);
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")"));
    }
    px_srcline(117);
    if (px_is_truthy(px_eq(_v825, px_str("Dict")))) {
        px_srcline(118);
        _v829 = px_str("({ LXValue _d = px_dict(); ");
        px_srcline(119);
        _v830 = px_index(_v824, px_int(1LL));
        px_srcline(120);
        _v828 = px_int(0LL);
        px_srcline(121);
        while (px_is_truthy(px_lt(_v828, px_call(px_get_global("len"), (LXValue[]){_v830}, 1)))) {
            px_srcline(122);
            _v831 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v830, _v828), px_int(0LL))}, 1);
            px_srcline(123);
            _v832 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v830, _v828), px_int(1LL))}, 1);
            px_srcline(124);
             _v829 = px_add(_v829, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v831), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v832), px_str("); } ")));
            px_srcline(125);
             _v828 = px_add(_v828, px_int(1LL));
        }
        px_srcline(126);
         _v829 = px_add(_v829, px_str("_d; })"));
        px_srcline(127);
        return _v829;
    }
    px_srcline(128);
    if (px_is_truthy(px_eq(_v825, px_str("Var")))) {
        px_srcline(129);
        _v833 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(130);
        _v834 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v833}, 1);
        px_srcline(131);
        if (px_is_truthy(px_ne(_v834, px_null()))) {
            px_srcline(133);
            return px_call(px_get_global("cg_load_of"), (LXValue[]){_v833, _v834}, 2);
        }
        px_srcline(134);
        return px_add(px_add(px_str("px_get_global(\""), _v833), px_str("\")"));
    }
    px_srcline(135);
    if (px_is_truthy(px_eq(_v825, px_str("Field")))) {
        px_srcline(136);
        _v835 = px_index(_v824, px_int(1LL));
        px_srcline(137);
        _v836 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(139);
        if (px_is_truthy(px_eq(px_index(_v835, px_int(0LL)), px_str("Var")))) {
            px_srcline(140);
            _v837 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v835, px_int(1LL))}, 1);
            px_srcline(141);
            if (px_is_truthy(({ LXValue _t898 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v837}, 1); px_is_truthy(_t898) ? px_method(px_index(px_get_global("cg_const_enums"), _v837), "has", (LXValue[]){_v836}, 1) : _t898; }))) {
                px_srcline(142);
                return px_index(px_index(px_get_global("cg_const_enums"), _v837), _v836);
            }
        }
        px_srcline(144);
        if (px_is_truthy(px_eq(px_index(_v835, px_int(0LL)), px_str("Var")))) {
            px_srcline(145);
            _v837 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v835, px_int(1LL))}, 1);
            px_srcline(146);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v837}, 1))) {
                px_srcline(147);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v837), px_str("\", \"")), _v836), px_str("\")"));
            }
        }
        px_srcline(148);
        _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v835}, 1);
        px_srcline(149);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v838), px_str(", \"")), _v836), px_str("\")"));
    }
    px_srcline(150);
    if (px_is_truthy(px_eq(_v825, px_str("OptionalField")))) {
        px_srcline(151);
        _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(152);
        _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(153);
        _v836 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(154);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v838), px_str("; px_is_null(")), _v839), px_str(") ? px_null() : px_field(")), _v838), px_str(", \"")), _v836), px_str("\"); })"));
    }
    px_srcline(155);
    if (px_is_truthy(px_eq(_v825, px_str("Index")))) {
        px_srcline(156);
        _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(157);
        _v828 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(158);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v838), px_str(", ")), _v828), px_str(")"));
    }
    px_srcline(159);
    if (px_is_truthy(px_eq(_v825, px_str("Slice")))) {
        px_srcline(160);
        _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(161);
        _v829 = px_str("px_null()");
        px_srcline(162);
        if (px_is_truthy(px_ne(px_index(_v824, px_int(2LL)), px_null()))) {
            px_srcline(163);
             _v829 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        }
        px_srcline(164);
        _v840 = px_str("px_null()");
        px_srcline(165);
        if (px_is_truthy(px_ne(px_index(_v824, px_int(3LL)), px_null()))) {
            px_srcline(166);
             _v840 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        }
        px_srcline(167);
        _v841 = px_str("px_null()");
        px_srcline(168);
        if (px_is_truthy(px_ne(px_index(_v824, px_int(4LL)), px_null()))) {
            px_srcline(169);
             _v841 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(4LL))}, 1);
        }
        px_srcline(170);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v838), px_str(", ")), _v829), px_str(", ")), _v840), px_str(", ")), _v841), px_str(")"));
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v825, px_str("Call")))) {
        px_srcline(172);
        _v842 = px_index(_v824, px_int(1LL));
        px_srcline(173);
        _v843 = px_index(_v824, px_int(2LL));
        px_srcline(174);
        if (px_is_truthy(px_eq(px_index(_v842, px_int(0LL)), px_str("Var")))) {
            px_srcline(175);
            _v844 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v842, px_int(1LL))}, 1);
            px_srcline(178);
            (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v842, _v843}, 2));
            px_srcline(179);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v844}, 1))) {
                px_srcline(180);
                _v845 = px_index(px_get_global("cg_ffi"), _v844);
                px_srcline(181);
                _v826 = px_list_n((LXValue[]){}, 0);
                px_srcline(182);
                _v846 = px_int(0LL);
                px_srcline(183);
                while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                    px_srcline(184);
                    (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
                    px_srcline(185);
                     _v846 = px_add(_v846, px_int(1LL));
                }
                px_srcline(186);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v844), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")}, 2)"));
            }
            px_srcline(187);
            if (px_is_truthy(px_eq(_v844, px_str("chan")))) {
                px_srcline(188);
                _v847 = px_str("0");
                px_srcline(189);
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v843}, 1), px_int(0LL)))) {
                    px_srcline(190);
                    if (px_is_truthy(px_eq(px_index(px_index(_v843, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                        px_srcline(191);
                         _v847 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v843, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                        px_srcline(193);
                         _v847 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                px_srcline(194);
                return px_add(px_add(px_str("px_chan_create("), _v847), px_str(")"));
            }
            px_srcline(195);
            if (px_is_truthy(px_eq(_v844, px_str("mutex")))) {
                px_srcline(196);
                return px_str("px_mutex_create()");
            }
            px_srcline(197);
            if (px_is_truthy(px_eq(_v844, px_str("rwlock")))) {
                px_srcline(198);
                return px_str("px_rwlock_create()");
            }
            px_srcline(200);
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v844}, 1))) {
                px_srcline(201);
                _v848 = px_index(px_get_global("cg_structs"), _v844);
                px_srcline(202);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v848}, 1), px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                    px_srcline(203);
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v844), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v848}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v843}, 1)}, 1));
                }
                px_srcline(204);
                _v826 = px_list_n((LXValue[]){}, 0);
                px_srcline(205);
                _v846 = px_int(0LL);
                px_srcline(206);
                while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                    px_srcline(207);
                    (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
                    px_srcline(208);
                     _v846 = px_add(_v846, px_int(1LL));
                }
                px_srcline(209);
                _v849 = px_list_n((LXValue[]){}, 0);
                px_srcline(210);
                _v850 = px_int(0LL);
                px_srcline(211);
                while (px_is_truthy(px_lt(_v850, px_call(px_get_global("len"), (LXValue[]){_v848}, 1)))) {
                    px_srcline(212);
                    (void)(px_method(_v849, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v848, _v850)), px_str("\""))}, 1));
                    px_srcline(213);
                     _v850 = px_add(_v850, px_int(1LL));
                }
                px_srcline(214);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v844), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v849}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v848}, 1)}, 1)), px_str(")"));
            }
            px_srcline(215);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v844}, 1))) {
                px_srcline(216);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v843}, 1), px_int(1LL)))) {
                    px_srcline(217);
                    return px_add(px_add(px_str("枚举 "), _v844), px_str(" 构造需要一个变体名"));
                }
                px_srcline(218);
                _v834 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, px_int(0LL))}, 1);
                px_srcline(219);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v844), px_str("\", (")), _v834), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        px_srcline(221);
        if (px_is_truthy(px_eq(px_index(_v842, px_int(0LL)), px_str("Field")))) {
            px_srcline(222);
            _v835 = px_index(_v842, px_int(1LL));
            px_srcline(223);
            _v851 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v842, px_int(2LL))}, 1);
            px_srcline(225);
            _v852 = px_null();
            px_srcline(226);
            if (px_is_truthy(px_eq(px_index(_v835, px_int(0LL)), px_str("Var")))) {
                px_srcline(227);
                _v837 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v835, px_int(1LL))}, 1);
                px_srcline(228);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v837}, 1))) {
                    px_srcline(229);
                     _v852 = px_index(px_get_global("cg_var_types"), _v837);
                }
            }
            px_srcline(230);
            if (px_is_truthy(({ LXValue _t899 = px_ne(_v852, px_null()); px_is_truthy(_t899) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v852}, 1) : _t899; }))) {
                px_srcline(231);
                _v853 = px_index(px_get_global("cg_impls"), _v852);
                px_srcline(232);
                _v854 = px_bool(false);
                px_srcline(233);
                _v855 = px_int(0LL);
                px_srcline(234);
                while (px_is_truthy(px_lt(_v855, px_call(px_get_global("len"), (LXValue[]){_v853}, 1)))) {
                    px_srcline(235);
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v853, _v855), px_int(1LL))}, 1), _v851))) {
                        px_srcline(236);
                         _v854 = px_bool(true);
                        px_srcline(237);
                        break;
                    }
                    px_srcline(238);
                     _v855 = px_add(_v855, px_int(1LL));
                }
                px_srcline(239);
                if (px_is_truthy(_v854)) {
                    px_srcline(240);
                    _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v835}, 1);
                    px_srcline(241);
                    _v826 = px_list_n((LXValue[]){_v838}, 1);
                    px_srcline(242);
                    _v846 = px_int(0LL);
                    px_srcline(243);
                    while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                        px_srcline(244);
                        (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
                        px_srcline(245);
                         _v846 = px_add(_v846, px_int(1LL));
                    }
                    px_srcline(246);
                    _v856 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v852}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v851}, 1));
                    px_srcline(247);
                    return px_add(px_add(px_add(px_add(px_add(_v856, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            px_srcline(249);
            _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v835}, 1);
            px_srcline(250);
            _v826 = px_list_n((LXValue[]){}, 0);
            px_srcline(251);
            _v846 = px_int(0LL);
            px_srcline(252);
            while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                px_srcline(253);
                (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
                px_srcline(254);
                 _v846 = px_add(_v846, px_int(1LL));
            }
            px_srcline(255);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v838), px_str(", \"")), _v851), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")"));
        }
        px_srcline(257);
        _v857 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v842}, 1);
        px_srcline(258);
        _v826 = px_list_n((LXValue[]){}, 0);
        px_srcline(259);
        _v846 = px_int(0LL);
        px_srcline(260);
        while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
            px_srcline(261);
            (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
            px_srcline(262);
             _v846 = px_add(_v846, px_int(1LL));
        }
        px_srcline(263);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v857), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")"));
    }
    px_srcline(264);
    if (px_is_truthy(px_eq(_v825, px_str("Unary")))) {
        px_srcline(265);
        _v838 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(266);
        _v858 = px_index(_v824, px_int(1LL));
        px_srcline(267);
        if (px_is_truthy(px_eq(_v858, px_str("Neg")))) {
            px_srcline(268);
            return px_add(px_add(px_str("px_neg("), _v838), px_str(")"));
        }
        px_srcline(269);
        if (px_is_truthy(px_eq(_v858, px_str("Not")))) {
            px_srcline(270);
            return px_add(px_add(px_str("px_not("), _v838), px_str(")"));
        }
        px_srcline(271);
        return px_add(px_add(px_str("px_bitnot("), _v838), px_str(")"));
    }
    px_srcline(272);
    if (px_is_truthy(px_eq(_v825, px_str("Binary")))) {
        px_srcline(273);
        _v859 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(274);
        _v860 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        px_srcline(275);
        _v858 = px_index(_v824, px_int(1LL));
        px_srcline(276);
        if (px_is_truthy(px_eq(_v858, px_str("And")))) {
            px_srcline(277);
            _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(278);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v859), px_str("; px_is_truthy(")), _v839), px_str(") ? ")), _v860), px_str(" : ")), _v839), px_str("; })"));
        }
        px_srcline(279);
        if (px_is_truthy(px_eq(_v858, px_str("Or")))) {
            px_srcline(280);
            _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(281);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v859), px_str("; px_is_truthy(")), _v839), px_str(") ? ")), _v839), px_str(" : ")), _v860), px_str("; })"));
        }
        px_srcline(282);
        _v861 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v858}, 1);
        px_srcline(283);
        return px_add(px_add(px_add(px_add(px_add(_v861, px_str("(")), _v859), px_str(", ")), _v860), px_str(")"));
    }
    px_srcline(284);
    if (px_is_truthy(px_eq(_v825, px_str("Pipe")))) {
        px_srcline(285);
        _v834 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(286);
        _v862 = px_index(_v824, px_int(2LL));
        px_srcline(287);
        if (px_is_truthy(px_eq(px_index(_v862, px_int(0LL)), px_str("Call")))) {
            px_srcline(288);
            _v857 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v862, px_int(1LL))}, 1);
            px_srcline(289);
            _v826 = px_list_n((LXValue[]){_v834}, 1);
            px_srcline(290);
            _v846 = px_int(0LL);
            px_srcline(291);
            while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){px_index(_v862, px_int(2LL))}, 1)))) {
                px_srcline(292);
                (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v862, px_int(2LL)), _v846)}, 1)}, 1));
                px_srcline(293);
                 _v846 = px_add(_v846, px_int(1LL));
            }
            px_srcline(294);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v857), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")"));
        }
        px_srcline(295);
        _v861 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v862}, 1);
        px_srcline(296);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v861), px_str(", (LXValue[]){")), _v834), px_str("}, 1)"));
    }
    px_srcline(297);
    if (px_is_truthy(px_eq(_v825, px_str("NullCoalesce")))) {
        px_srcline(298);
        _v859 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(299);
        _v860 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(300);
        _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(301);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v859), px_str("; px_is_null(")), _v839), px_str(") ? ")), _v860), px_str(" : ")), _v839), px_str("; })"));
    }
    px_srcline(302);
    if (px_is_truthy(px_eq(_v825, px_str("Try")))) {
        px_srcline(303);
        _v840 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(304);
        _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(305);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            px_srcline(306);
            _v863 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            px_srcline(307);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v840), px_str("; if (px_is_result(")), _v839), px_str(")) { if (!px_result_ok(")), _v839), px_str(")) { ")), _v863), px_str("_val = ")), _v839), px_str("; ")), _v863), px_str("_proped = 1; goto ")), _v863), px_str("; } ")), _v839), px_str(" = px_result_unwrap(")), _v839), px_str("); } else if (px_is_null(")), _v839), px_str(")) { ")), _v863), px_str("_val = px_null(); ")), _v863), px_str("_proped = 1; goto ")), _v863), px_str("; } ")), _v839), px_str("; })"));
        }
        px_srcline(308);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v840), px_str("; if (px_is_result(")), _v839), px_str(") && !px_result_ok(")), _v839), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v839), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v839), px_str(")) ")), _v839), px_str(" = px_result_unwrap(")), _v839), px_str("); ")), _v839), px_str("; })"));
    }
    px_srcline(309);
    if (px_is_truthy(px_eq(_v825, px_str("ForceUnwrap")))) {
        px_srcline(310);
        _v840 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(311);
        _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(312);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v840), px_str("; if (px_is_result(")), _v839), px_str(")) { if (!px_result_ok(")), _v839), px_str(")) px_error(\"force unwrap Err\"); ")), _v839), px_str(" = px_result_unwrap(")), _v839), px_str("); } if (px_is_null(")), _v839), px_str(")) px_error(\"force unwrap null\"); ")), _v839), px_str("; })"));
    }
    px_srcline(313);
    if (px_is_truthy(px_eq(_v825, px_str("IfExpr")))) {
        px_srcline(314);
        _v857 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(315);
        _v864 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(316);
        _v865 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        px_srcline(317);
        _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(318);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str("; if (px_is_truthy(")), _v857), px_str(")) { ")), _v839), px_str(" = ")), _v864), px_str("; } else { ")), _v839), px_str(" = ")), _v865), px_str("; } ")), _v839), px_str("; })"));
    }
    px_srcline(319);
    if (px_is_truthy(px_eq(_v825, px_str("ListComp")))) {
        px_srcline(320);
        _v866 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(321);
        _v860 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(322);
        _v840 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(323);
        _v867 = px_null();
        px_srcline(324);
        if (px_is_truthy(px_ne(px_index(_v824, px_int(3LL)), px_null()))) {
            px_srcline(325);
             _v867 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        }
        px_srcline(326);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v860, px_str("saved_all"))}, 1));
        px_srcline(327);
        _v868 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v866), px_str(", ")), _v840), px_str("); "));
        px_srcline(328);
        _v869 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v860, _v867, _v868}, 3);
        px_srcline(329);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v866), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v860, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v860, px_str("its")), px_int(0LL))), px_str("; ")), _v869), px_str(" ")), _v866), px_str("; })"));
    }
    px_srcline(330);
    if (px_is_truthy(px_eq(_v825, px_str("GenExp")))) {
        px_srcline(331);
        _v870 = px_index(_v824, px_int(2LL));
        px_srcline(332);
        if (px_is_truthy(({ LXValue _t900 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v870}, 1), px_int(1LL)); px_is_truthy(_t900) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v870, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t900; }))) {
            px_srcline(333);
            _v871 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v870, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(334);
            _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v870, px_int(0LL)), px_int(2LL))}, 1);
            px_srcline(335);
            _v873 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v871}, 1), px_index(_v824, px_int(1LL))}, 2);
            px_srcline(336);
            _v874 = px_str("px_null()");
            px_srcline(337);
            if (px_is_truthy(px_ne(px_index(_v824, px_int(3LL)), px_null()))) {
                px_srcline(338);
                 _v874 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v871}, 1), px_index(_v824, px_int(3LL))}, 2);
            }
            px_srcline(339);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v872), px_str(", ")), _v873), px_str(", ")), _v874), px_str(")"));
        }
        px_srcline(341);
        _v866 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(342);
        _v860 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v870}, 1);
        px_srcline(343);
        _v840 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(344);
        _v867 = px_null();
        px_srcline(345);
        if (px_is_truthy(px_ne(px_index(_v824, px_int(3LL)), px_null()))) {
            px_srcline(346);
             _v867 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        }
        px_srcline(347);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v860, px_str("saved_all"))}, 1));
        px_srcline(348);
        _v868 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v866), px_str(", ")), _v840), px_str("); "));
        px_srcline(349);
        _v869 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v860, _v867, _v868}, 3);
        px_srcline(350);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v866), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v860, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v860, px_str("its")), px_int(0LL))), px_str("; ")), _v869), px_str(" px_gen_from_list(")), _v866), px_str("); })"));
    }
    px_srcline(351);
    if (px_is_truthy(px_eq(_v825, px_str("DictComp")))) {
        px_srcline(352);
        _v866 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(353);
        _v860 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        px_srcline(354);
        _v831 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(355);
        _v832 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(356);
        _v867 = px_null();
        px_srcline(357);
        if (px_is_truthy(px_ne(px_index(_v824, px_int(4LL)), px_null()))) {
            px_srcline(358);
             _v867 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(4LL))}, 1);
        }
        px_srcline(359);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v860, px_str("saved_all"))}, 1));
        px_srcline(360);
        _v868 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v831), px_str("; LXValue _v = ")), _v832), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v866), px_str(", _k.as.obj->as.str.data, _v); } "));
        px_srcline(361);
        _v869 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v860, _v867, _v868}, 3);
        px_srcline(362);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v866), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v860, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v860, px_str("its")), px_int(0LL))), px_str("; ")), _v869), px_str(" ")), _v866), px_str("; })"));
    }
    px_srcline(363);
    if (px_is_truthy(px_eq(_v825, px_str("Closure")))) {
        px_srcline(364);
        px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
        px_srcline(365);
        _v875 = px_get_global("cg_closure_id");
        px_srcline(366);
        _v856 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v875}, 1));
        px_srcline(368);
        _v876 = px_list_n((LXValue[]){}, 0);
        px_srcline(369);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){_v824, _v876}, 2));
        px_srcline(370);
        _v877 = px_list_n((LXValue[]){}, 0);
        px_srcline(371);
        LXValue _t901 = _v876;
        for (int _t902 = 0; _t902 < px_len(_t901); _t902++) {
            _v878 = px_index(_t901, px_int(_t902));
            px_srcline(372);
            if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v878}, 1), px_null()))) {
                px_srcline(373);
                (void)(px_method(_v877, "append", (LXValue[]){_v878}, 1));
            }
        }
        px_srcline(375);
        _v879 = px_str("px_null()");
        px_srcline(376);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v877}, 1), px_int(0LL)))) {
            px_srcline(377);
            _v880 = px_str("({ LXValue _capenv = px_dict(); ");
            px_srcline(378);
            LXValue _t903 = _v877;
            for (int _t904 = 0; _t904 < px_len(_t903); _t904++) {
                _v878 = px_index(_t903, px_int(_t904));
                px_srcline(379);
                _v881 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v878}, 1);
                px_srcline(382);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v878}, 1))) {
                    px_srcline(383);
                     _v880 = px_add(_v880, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v878), px_str("\", ")), _v881), px_str("); ")));
                }
                else {
                    px_srcline(385);
                     _v880 = px_add(_v880, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v878), px_str("\", px_cell(")), _v881), px_str(")); ")));
                }
            }
            px_srcline(386);
             _v880 = px_add(_v880, px_str("_capenv; })"));
            px_srcline(387);
             _v879 = _v880;
        }
        px_srcline(388);
        _v882 = px_add(px_add(px_str("static LXValue "), _v856), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
        px_srcline(389);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v877}, 1), px_int(0LL)))) {
            px_srcline(390);
             _v882 = px_add(_v882, px_str("    (void)nargs;\n"));
        }
        else {
            px_srcline(392);
             _v882 = px_add(_v882, px_str("    (void)ctx;\n"));
        }
        px_srcline(393);
        _v883 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
        px_srcline(394);
        _v884 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
        px_srcline(395);
        _v885 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
        px_srcline(396);
        px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_srcline(397);
        px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_srcline(398);
        px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
        px_srcline(399);
        _v886 = px_index(_v824, px_int(1LL));
        px_srcline(400);
        _v887 = px_int(0LL);
        px_srcline(401);
        while (px_is_truthy(px_lt(_v887, px_call(px_get_global("len"), (LXValue[]){_v886}, 1)))) {
            px_srcline(402);
            _v834 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v886, _v887), px_int(1LL))}, 1)}, 1);
            px_srcline(403);
             _v882 = px_add(_v882, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v834), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v887}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v887}, 1)), px_str("] : px_null();\n")));
            px_srcline(404);
             _v887 = px_add(_v887, px_int(1LL));
        }
        px_srcline(406);
        LXValue _t905 = _v877;
        for (int _t906 = 0; _t906 < px_len(_t905); _t906++) {
            _v878 = px_index(_t905, px_int(_t906));
            px_srcline(407);
            _v888 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v878}, 1);
            px_srcline(408);
            px_index_set(px_get_global("cg_cells"), _v878, px_int(1LL));
            px_srcline(409);
             _v882 = px_add(_v882, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v888), px_str(" = px_env_lookup(ctx, \"")), _v878), px_str("\");\n")));
        }
        px_srcline(410);
        _v865 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(3LL))}, 1);
        px_srcline(411);
         _v882 = px_add(_v882, px_add(px_add(px_str("    return "), _v865), px_str(";\n")));
        px_srcline(412);
         _v882 = px_add(_v882, px_str("}\n"));
        px_srcline(413);
        px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v882));
        px_srcline(414);
        px_set_global("cg_vars", _v883);
        px_srcline(415);
        px_set_global("cg_var_types", _v884);
        px_srcline(416);
        px_set_global("cg_cells", _v885);
        px_srcline(420);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v877}, 1), px_int(0LL)))) {
            px_srcline(421);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v875}, 1)), px_str(">\", ")), _v856), px_str(", ")), _v879), px_str(")"));
        }
        px_srcline(422);
        return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v875}, 1)), px_str(">\", ")), _v856), px_str(", NULL)"));
    }
    px_srcline(423);
    if (px_is_truthy(px_eq(_v825, px_str("Block")))) {
        px_srcline(424);
        _v829 = px_str("({ ");
        px_srcline(425);
         _v829 = px_add(_v829, px_str("LXValue _blk = px_null(); "));
        px_srcline(426);
        _v889 = px_index(_v824, px_int(1LL));
        px_srcline(427);
        _v890 = px_int(0LL);
        px_srcline(428);
        while (px_is_truthy(px_lt(_v890, px_call(px_get_global("len"), (LXValue[]){_v889}, 1)))) {
            px_srcline(429);
            _v891 = px_index(_v889, _v890);
            px_srcline(430);
            if (px_is_truthy(px_eq(px_index(_v891, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(431);
                _v840 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v891, px_int(1LL))}, 1);
                px_srcline(432);
                 _v829 = px_add(_v829, px_add(px_add(px_str("_blk = "), _v840), px_str("; ")));
            }
            else {
                px_srcline(434);
                 _v829 = px_add(_v829, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v891, px_int(0LL)}, 2));
            }
            px_srcline(435);
             _v890 = px_add(_v890, px_int(1LL));
        }
        px_srcline(436);
         _v829 = px_add(_v829, px_str("_blk; })"));
        px_srcline(437);
        return _v829;
    }
    px_srcline(438);
    if (px_is_truthy(px_eq(_v825, px_str("Match")))) {
        px_srcline(439);
        _v892 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(440);
        _v839 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(441);
        _v829 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v839), px_str(" = ")), _v892), px_str("; "));
        px_srcline(442);
        _v893 = px_index(_v824, px_int(2LL));
        px_srcline(443);
        _v894 = px_bool(true);
        px_srcline(444);
        _v846 = px_int(0LL);
        px_srcline(445);
        while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v893}, 1)))) {
            px_srcline(446);
            _v867 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v893, _v846), px_int(1LL)), _v839}, 2);
            px_srcline(447);
            _v895 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v893, _v846), px_int(3LL))}, 1);
            px_srcline(448);
            _v896 = px_str("if");
            px_srcline(449);
            if (px_is_truthy(px_not(_v894))) {
                px_srcline(450);
                 _v896 = px_str("else if");
            }
            px_srcline(451);
             _v829 = px_add(_v829, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v896, px_str(" (")), _v867), px_str(") { ")), _v839), px_str(" = ")), _v895), px_str("; } ")));
            px_srcline(452);
             _v894 = px_bool(false);
            px_srcline(453);
             _v846 = px_add(_v846, px_int(1LL));
        }
        px_srcline(454);
         _v829 = px_add(_v829, px_add(_v839, px_str("; })")));
        px_srcline(455);
        return _v829;
    }
    px_srcline(456);
    if (px_is_truthy(px_eq(_v825, px_str("Constructor")))) {
        px_srcline(457);
        _v833 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1);
        px_srcline(458);
        _v843 = px_index(_v824, px_int(2LL));
        px_srcline(459);
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v833}, 1))) {
            px_srcline(460);
            _v848 = px_index(px_get_global("cg_structs"), _v833);
            px_srcline(461);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v848}, 1), px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                px_srcline(462);
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v833), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v848}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v843}, 1)}, 1));
            }
            px_srcline(463);
            _v826 = px_list_n((LXValue[]){}, 0);
            px_srcline(464);
            _v846 = px_int(0LL);
            px_srcline(465);
            while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
                px_srcline(466);
                (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
                px_srcline(467);
                 _v846 = px_add(_v846, px_int(1LL));
            }
            px_srcline(468);
            _v849 = px_list_n((LXValue[]){}, 0);
            px_srcline(469);
            _v850 = px_int(0LL);
            px_srcline(470);
            while (px_is_truthy(px_lt(_v850, px_call(px_get_global("len"), (LXValue[]){_v848}, 1)))) {
                px_srcline(471);
                (void)(px_method(_v849, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v848, _v850)), px_str("\""))}, 1));
                px_srcline(472);
                 _v850 = px_add(_v850, px_int(1LL));
            }
            px_srcline(473);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v833), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v849}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v848}, 1)}, 1)), px_str(")"));
        }
        px_srcline(474);
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v833}, 1))) {
            px_srcline(475);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v843}, 1), px_int(1LL)))) {
                px_srcline(476);
                return px_add(px_add(px_str("枚举 "), _v833), px_str(" 构造需要一个变体名"));
            }
            px_srcline(477);
            _v834 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, px_int(0LL))}, 1);
            px_srcline(478);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v833), px_str("\", (")), _v834), px_str(").as.obj->as.enum_inst.variant)"));
        }
        px_srcline(479);
        _v826 = px_list_n((LXValue[]){}, 0);
        px_srcline(480);
        _v846 = px_int(0LL);
        px_srcline(481);
        while (px_is_truthy(px_lt(_v846, px_call(px_get_global("len"), (LXValue[]){_v843}, 1)))) {
            px_srcline(482);
            (void)(px_method(_v826, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v843, _v846)}, 1)}, 1));
            px_srcline(483);
             _v846 = px_add(_v846, px_int(1LL));
        }
        px_srcline(484);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v833), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v826}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v826}, 1)}, 1)), px_str(")"));
    }
    px_srcline(485);
    return px_str("px_null()");
px_err_897:
    if (px_err_897_proped) return px_err_897_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_binop_cname");
    LXValue _v907 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_908_val = px_null();
    int px_err_908_proped = 0;
    px_srcline(488);
    if (px_is_truthy(px_eq(_v907, px_str("Add")))) {
        px_srcline(489);
        return px_str("px_add");
    }
    px_srcline(490);
    if (px_is_truthy(px_eq(_v907, px_str("Sub")))) {
        px_srcline(491);
        return px_str("px_sub");
    }
    px_srcline(492);
    if (px_is_truthy(px_eq(_v907, px_str("Mul")))) {
        px_srcline(493);
        return px_str("px_mul");
    }
    px_srcline(494);
    if (px_is_truthy(px_eq(_v907, px_str("Div")))) {
        px_srcline(495);
        return px_str("px_div");
    }
    px_srcline(496);
    if (px_is_truthy(px_eq(_v907, px_str("IntDiv")))) {
        px_srcline(497);
        return px_str("px_idiv");
    }
    px_srcline(498);
    if (px_is_truthy(px_eq(_v907, px_str("Mod")))) {
        px_srcline(499);
        return px_str("px_mod");
    }
    px_srcline(500);
    if (px_is_truthy(px_eq(_v907, px_str("Pow")))) {
        px_srcline(501);
        return px_str("px_pow");
    }
    px_srcline(502);
    if (px_is_truthy(px_eq(_v907, px_str("Eq")))) {
        px_srcline(503);
        return px_str("px_eq");
    }
    px_srcline(504);
    if (px_is_truthy(px_eq(_v907, px_str("Ne")))) {
        px_srcline(505);
        return px_str("px_ne");
    }
    px_srcline(506);
    if (px_is_truthy(px_eq(_v907, px_str("Lt")))) {
        px_srcline(507);
        return px_str("px_lt");
    }
    px_srcline(508);
    if (px_is_truthy(px_eq(_v907, px_str("Le")))) {
        px_srcline(509);
        return px_str("px_le");
    }
    px_srcline(510);
    if (px_is_truthy(px_eq(_v907, px_str("Gt")))) {
        px_srcline(511);
        return px_str("px_gt");
    }
    px_srcline(512);
    if (px_is_truthy(px_eq(_v907, px_str("Ge")))) {
        px_srcline(513);
        return px_str("px_ge");
    }
    px_srcline(514);
    if (px_is_truthy(px_eq(_v907, px_str("BitAnd")))) {
        px_srcline(515);
        return px_str("px_bitand");
    }
    px_srcline(516);
    if (px_is_truthy(px_eq(_v907, px_str("BitOr")))) {
        px_srcline(517);
        return px_str("px_bitor");
    }
    px_srcline(518);
    if (px_is_truthy(px_eq(_v907, px_str("BitXor")))) {
        px_srcline(519);
        return px_str("px_bitxor");
    }
    px_srcline(520);
    if (px_is_truthy(px_eq(_v907, px_str("Shl")))) {
        px_srcline(521);
        return px_str("px_shl");
    }
    px_srcline(522);
    if (px_is_truthy(px_eq(_v907, px_str("Shr")))) {
        px_srcline(523);
        return px_str("px_shr");
    }
    px_srcline(524);
    if (px_is_truthy(px_eq(_v907, px_str("ShrU")))) {
        px_srcline(525);
        return px_str("px_ushr");
    }
    px_srcline(526);
    return px_str("px_add");
px_err_908:
    if (px_err_908_proped) return px_err_908_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_pattern_cond");
    LXValue _v909 = (nargs > 0) ? args[0] : px_null();
    LXValue _v910 = (nargs > 1) ? args[1] : px_null();
    LXValue _v911 = px_null();
    LXValue _v912 = px_null();
    LXValue _v913 = px_null();
    LXValue _v914 = px_null();
    LXValue px_err_915_val = px_null();
    int px_err_915_proped = 0;
    px_srcline(529);
    _v911 = px_index(_v909, px_int(0LL));
    px_srcline(530);
    if (px_is_truthy(px_eq(_v911, px_str("PatLiteral")))) {
        px_srcline(531);
        _v912 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v909, px_int(1LL))}, 1);
        px_srcline(532);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v910), px_str(", ")), _v912), px_str("))"));
    }
    px_srcline(533);
    if (px_is_truthy(px_eq(_v911, px_str("PatBinding")))) {
        px_srcline(534);
        _v913 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v909, px_int(1LL))}, 1);
        px_srcline(535);
        if (px_is_truthy(({ LXValue _t917 = ({ LXValue _t916 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v913}, 1), px_int(0LL)); px_is_truthy(_t916) ? px_ge(px_index(_v913, px_int(0LL)), px_str("A")) : _t916; }); px_is_truthy(_t917) ? px_le(px_index(_v913, px_int(0LL)), px_str("Z")) : _t917; }))) {
            px_srcline(536);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v910), px_str(".type == PX_ENUM && strcmp(")), _v910), px_str(".as.obj->as.enum_inst.variant, \"")), _v913), px_str("\") == 0)"));
        }
        px_srcline(537);
        return px_str("true");
    }
    px_srcline(538);
    if (px_is_truthy(px_eq(_v911, px_str("PatWildcard")))) {
        px_srcline(539);
        return px_str("true");
    }
    px_srcline(540);
    if (px_is_truthy(px_eq(_v911, px_str("PatTuple")))) {
        px_srcline(541);
        _v914 = px_index(_v909, px_int(1LL));
        px_srcline(542);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v914}, 1), px_int(0LL)))) {
            px_srcline(543);
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v914, px_int(0LL)), _v910}, 2);
        }
        px_srcline(544);
        return px_str("true");
    }
    px_srcline(545);
    if (px_is_truthy(px_eq(_v911, px_str("PatConstructor")))) {
        px_srcline(546);
        _v913 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v909, px_int(1LL))}, 1);
        px_srcline(547);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v910), px_str(".type == PX_ENUM && strcmp(")), _v910), px_str(".as.obj->as.enum_inst.variant, \"")), _v913), px_str("\") == 0)"));
    }
    px_srcline(548);
    return px_str("true");
px_err_915:
    if (px_err_915_proped) return px_err_915_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_lambda");
    LXValue _v918 = (nargs > 0) ? args[0] : px_null();
    LXValue _v919 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v935 = px_null();
    LXValue _v936 = px_null();
    LXValue px_err_937_val = px_null();
    int px_err_937_proped = 0;
    px_srcline(551);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(552);
    _v920 = px_get_global("cg_closure_id");
    px_srcline(553);
    _v921 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v920}, 1));
    px_srcline(555);
    _v922 = px_list_n((LXValue[]){}, 0);
    px_srcline(556);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v919, _v922}, 2));
    px_srcline(557);
    _v923 = px_list_n((LXValue[]){}, 0);
    px_srcline(558);
    LXValue _t938 = _v922;
    for (int _t939 = 0; _t939 < px_len(_t938); _t939++) {
        _v924 = px_index(_t938, px_int(_t939));
        px_srcline(559);
        if (px_is_truthy(({ LXValue _t940 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v918, _v924}, 2)); px_is_truthy(_t940) ? px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v924}, 1), px_null()) : _t940; }))) {
            px_srcline(560);
            (void)(px_method(_v923, "append", (LXValue[]){_v924}, 1));
        }
    }
    px_srcline(561);
    _v925 = px_str("px_null()");
    px_srcline(562);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v923}, 1), px_int(0LL)))) {
        px_srcline(563);
        _v926 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(564);
        LXValue _t941 = _v923;
        for (int _t942 = 0; _t942 < px_len(_t941); _t942++) {
            _v924 = px_index(_t941, px_int(_t942));
            px_srcline(565);
            _v927 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v924}, 1);
            px_srcline(566);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v924}, 1))) {
                px_srcline(567);
                 _v926 = px_add(_v926, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v924), px_str("\", ")), _v927), px_str("); ")));
            }
            else {
                px_srcline(569);
                 _v926 = px_add(_v926, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v924), px_str("\", px_cell(")), _v927), px_str(")); ")));
            }
        }
        px_srcline(570);
         _v926 = px_add(_v926, px_str("_capenv; })"));
        px_srcline(571);
         _v925 = _v926;
    }
    px_srcline(572);
    _v928 = px_add(px_add(px_str("static LXValue "), _v921), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(573);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v923}, 1), px_int(0LL)))) {
        px_srcline(574);
         _v928 = px_add(_v928, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(576);
         _v928 = px_add(_v928, px_str("    (void)ctx;\n"));
    }
    px_srcline(577);
    _v929 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(578);
    _v930 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(579);
    _v931 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(580);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(581);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(582);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(583);
    _v932 = px_int(0LL);
    px_srcline(584);
    while (px_is_truthy(px_lt(_v932, px_call(px_get_global("len"), (LXValue[]){_v918}, 1)))) {
        px_srcline(585);
        _v933 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v918, _v932)}, 1);
        px_srcline(586);
         _v928 = px_add(_v928, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v933), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v932}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v932}, 1)), px_str("] : px_null();\n")));
        px_srcline(587);
         _v932 = px_add(_v932, px_int(1LL));
    }
    px_srcline(588);
    LXValue _t943 = _v923;
    for (int _t944 = 0; _t944 < px_len(_t943); _t944++) {
        _v924 = px_index(_t943, px_int(_t944));
        px_srcline(589);
        _v934 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v924}, 1);
        px_srcline(590);
        px_index_set(px_get_global("cg_cells"), _v924, px_int(1LL));
        px_srcline(591);
         _v928 = px_add(_v928, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v934), px_str(" = px_env_lookup(ctx, \"")), _v924), px_str("\");\n")));
    }
    px_srcline(592);
    _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v919}, 1);
    px_srcline(593);
    _v936 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(594);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v936}, 1));
    px_srcline(595);
     _v928 = px_add(_v928, px_add(px_add(px_str("    LXValue "), _v936), px_str("_val = px_null();\n")));
    px_srcline(596);
     _v928 = px_add(_v928, px_add(px_add(px_str("    int "), _v936), px_str("_proped = 0;\n")));
    px_srcline(597);
     _v928 = px_add(_v928, px_add(px_add(px_str("    return "), _v935), px_str(";\n")));
    px_srcline(598);
     _v928 = px_add(_v928, px_add(_v936, px_str(":\n")));
    px_srcline(599);
     _v928 = px_add(_v928, px_add(px_add(px_add(px_add(px_str("    if ("), _v936), px_str("_proped) return ")), _v936), px_str("_val;\n")));
    px_srcline(600);
     _v928 = px_add(_v928, px_str("    return px_null();\n"));
    px_srcline(601);
     _v928 = px_add(_v928, px_str("}\n"));
    px_srcline(602);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(603);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v928));
    px_srcline(604);
    px_set_global("cg_vars", _v929);
    px_srcline(605);
    px_set_global("cg_var_types", _v930);
    px_srcline(606);
    px_set_global("cg_cells", _v931);
    px_srcline(608);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v923}, 1), px_int(0LL)))) {
        px_srcline(609);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v920}, 1)), px_str(">\", ")), _v921), px_str(", ")), _v925), px_str(")"));
    }
    px_srcline(610);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v920}, 1)), px_str(">\", ")), _v921), px_str(", NULL)"));
px_err_937:
    if (px_err_937_proped) return px_err_937_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dirname");
    LXValue _v945 = (nargs > 0) ? args[0] : px_null();
    LXValue _v946 = px_null();
    LXValue px_err_947_val = px_null();
    int px_err_947_proped = 0;
    px_srcline(14);
    _v946 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v945}, 1), px_int(1LL));
    px_srcline(15);
    while (px_is_truthy(px_ge(_v946, px_int(0LL)))) {
        px_srcline(16);
        if (px_is_truthy(px_eq(px_index(_v945, _v946), px_str("/")))) {
            px_srcline(17);
            if (px_is_truthy(px_eq(_v946, px_int(0LL)))) {
                px_srcline(18);
                return px_str("/");
            }
            px_srcline(19);
            return px_slice(_v945, px_int(0LL), _v946, px_null());
        }
        px_srcline(20);
         _v946 = px_sub(_v946, px_int(1LL));
    }
    px_srcline(21);
    return px_str(".");
px_err_947:
    if (px_err_947_proped) return px_err_947_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_stdlib_dir");
    LXValue _v948 = px_null();
    LXValue _v949 = px_null();
    LXValue _v950 = px_null();
    LXValue _v951 = px_null();
    LXValue px_err_952_val = px_null();
    int px_err_952_proped = 0;
    px_srcline(24);
    _v948 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    px_srcline(25);
    if (px_is_truthy(({ LXValue _t953 = px_ne(_v948, px_null()); px_is_truthy(_t953) ? px_call(px_get_global("exists"), (LXValue[]){_v948}, 1) : _t953; }))) {
        px_srcline(26);
        return _v948;
    }
    px_srcline(27);
    _v949 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    px_srcline(28);
    _v950 = px_int(0LL);
    px_srcline(29);
    while (px_is_truthy(px_lt(_v950, px_call(px_get_global("len"), (LXValue[]){_v949}, 1)))) {
        px_srcline(30);
        _v951 = px_index(_v949, _v950);
        px_srcline(31);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v951}, 1))) {
            px_srcline(32);
            return _v951;
        }
        px_srcline(33);
         _v950 = px_add(_v950, px_int(1LL));
    }
    px_srcline(34);
    return px_null();
px_err_952:
    if (px_err_952_proped) return px_err_952_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find_module_path");
    LXValue _v954 = (nargs > 0) ? args[0] : px_null();
    LXValue _v955 = (nargs > 1) ? args[1] : px_null();
    LXValue _v956 = px_null();
    LXValue _v957 = px_null();
    LXValue _v958 = px_null();
    LXValue _v959 = px_null();
    LXValue _v960 = px_null();
    LXValue _v961 = px_null();
    LXValue _v962 = px_null();
    LXValue _v963 = px_null();
    LXValue _v964 = px_null();
    LXValue _v965 = px_null();
    LXValue _v966 = px_null();
    LXValue _v967 = px_null();
    LXValue _v968 = px_null();
    LXValue _v969 = px_null();
    LXValue _v970 = px_null();
    LXValue px_err_971_val = px_null();
    int px_err_971_proped = 0;
    px_srcline(37);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v954}, 1), px_int(0LL)))) {
        px_srcline(38);
        return px_null();
    }
    px_srcline(40);
    if (px_is_truthy(({ LXValue _t973 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v954}, 1), px_int(1LL)); px_is_truthy(_t973) ? ({ LXValue _t972 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v954, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t972) ? _t972 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v954, px_int(0LL)), px_str(".px")}, 2); }) : _t973; }))) {
        px_srcline(41);
        _v956 = px_index(_v954, px_int(0LL));
        px_srcline(42);
        _v957 = _v956;
        px_srcline(43);
        if (px_is_truthy(px_not(({ LXValue _t974 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v956}, 1), px_int(0LL)); px_is_truthy(_t974) ? px_eq(px_index(_v956, px_int(0LL)), px_str("/")) : _t974; })))) {
            px_srcline(44);
             _v957 = px_add(px_add(_v955, px_str("/")), _v956);
        }
        px_srcline(45);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v957}, 1))) {
            px_srcline(46);
            return _v957;
        }
        px_srcline(47);
        return px_null();
    }
    px_srcline(49);
    if (px_is_truthy(px_eq(px_index(_v954, px_int(0LL)), px_str("std")))) {
        px_srcline(50);
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v954}, 1), px_int(2LL)))) {
            px_srcline(51);
            return px_null();
        }
        px_srcline(52);
        _v958 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(53);
        if (px_is_truthy(px_eq(_v958, px_null()))) {
            px_srcline(54);
            return px_null();
        }
        px_srcline(55);
        _v957 = _v958;
        px_srcline(56);
        _v959 = px_int(1LL);
        px_srcline(57);
        while (px_is_truthy(px_lt(_v959, px_call(px_get_global("len"), (LXValue[]){_v954}, 1)))) {
            px_srcline(58);
             _v957 = px_add(_v957, px_add(px_str("/"), px_index(_v954, _v959)));
            px_srcline(59);
             _v959 = px_add(_v959, px_int(1LL));
        }
        px_srcline(60);
        _v960 = px_add(_v957, px_str(".px"));
        px_srcline(61);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v960}, 1))) {
            px_srcline(62);
            return _v960;
        }
        px_srcline(63);
        _v961 = px_add(_v957, px_str("/mod.px"));
        px_srcline(64);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v961}, 1))) {
            px_srcline(65);
            return _v961;
        }
        px_srcline(66);
        return px_null();
    }
    px_srcline(68);
    _v962 = px_list_n((LXValue[]){_v955}, 1);
    px_srcline(69);
    _v963 = px_add(_v955, px_str("/.px_modules"));
    px_srcline(70);
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v963}, 1))) {
        px_srcline(71);
        (void)(px_method(_v962, "append", (LXValue[]){_v963}, 1));
        px_srcline(72);
        _v964 = px_call(px_get_global("list_dir"), (LXValue[]){_v963}, 1);
        px_srcline(73);
        _v965 = px_int(0LL);
        px_srcline(74);
        while (px_is_truthy(px_lt(_v965, px_call(px_get_global("len"), (LXValue[]){_v964}, 1)))) {
            px_srcline(75);
            _v966 = px_index(_v964, _v965);
            px_srcline(76);
            _v967 = px_add(px_add(_v963, px_str("/")), _v966);
            px_srcline(77);
            if (px_is_truthy(({ LXValue _t975 = px_call(px_get_global("exists"), (LXValue[]){_v967}, 1); px_is_truthy(_t975) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v966, px_str(".")}, 2)) : _t975; }))) {
                px_srcline(78);
                (void)(px_method(_v962, "append", (LXValue[]){_v967}, 1));
            }
            px_srcline(79);
             _v965 = px_add(_v965, px_int(1LL));
        }
    }
    px_srcline(80);
    _v968 = px_int(0LL);
    px_srcline(81);
    while (px_is_truthy(px_lt(_v968, px_call(px_get_global("len"), (LXValue[]){_v962}, 1)))) {
        px_srcline(82);
        _v969 = px_index(_v962, _v968);
        px_srcline(83);
        _v957 = _v969;
        px_srcline(84);
        _v959 = px_int(0LL);
        px_srcline(85);
        while (px_is_truthy(px_lt(_v959, px_call(px_get_global("len"), (LXValue[]){_v954}, 1)))) {
            px_srcline(86);
             _v957 = px_add(_v957, px_add(px_str("/"), px_index(_v954, _v959)));
            px_srcline(87);
             _v959 = px_add(_v959, px_int(1LL));
        }
        px_srcline(88);
        _v960 = px_add(_v957, px_str(".px"));
        px_srcline(89);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v960}, 1))) {
            px_srcline(90);
            return _v960;
        }
        px_srcline(91);
        _v961 = px_add(_v957, px_str("/mod.px"));
        px_srcline(92);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v961}, 1))) {
            px_srcline(93);
            return _v961;
        }
        px_srcline(94);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v954}, 1), px_int(1LL)))) {
            px_srcline(95);
            _v970 = px_add(px_add(px_add(_v969, px_str("/")), px_index(_v954, px_int(0LL))), px_str(".px"));
            px_srcline(96);
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v970}, 1))) {
                px_srcline(97);
                return _v970;
            }
        }
        px_srcline(98);
         _v968 = px_add(_v968, px_int(1LL));
    }
    px_srcline(99);
    return px_null();
px_err_971:
    if (px_err_971_proped) return px_err_971_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_definition");
    LXValue _v976 = (nargs > 0) ? args[0] : px_null();
    LXValue _v977 = px_null();
    LXValue px_err_978_val = px_null();
    int px_err_978_proped = 0;
    px_srcline(102);
    _v977 = px_index(_v976, px_int(0LL));
    px_srcline(103);
    if (px_is_truthy(px_eq(_v977, px_str("FuncDef")))) {
        px_srcline(105);
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v976, px_int(1LL))}, 1), px_str("main")))) {
            px_srcline(106);
            return px_bool(false);
        }
        px_srcline(107);
        return px_bool(true);
    }
    px_srcline(108);
    if (px_is_truthy(px_eq(_v977, px_str("ExternDef")))) {
        px_srcline(109);
        return px_bool(true);
    }
    px_srcline(110);
    if (px_is_truthy(({ LXValue _t981 = ({ LXValue _t980 = ({ LXValue _t979 = px_eq(_v977, px_str("StructDef")); px_is_truthy(_t979) ? _t979 : px_eq(_v977, px_str("EnumDef")); }); px_is_truthy(_t980) ? _t980 : px_eq(_v977, px_str("TraitDef")); }); px_is_truthy(_t981) ? _t981 : px_eq(_v977, px_str("ImplDef")); }))) {
        px_srcline(111);
        return px_bool(true);
    }
    px_srcline(112);
    if (px_is_truthy(px_eq(_v977, px_str("VarDecl")))) {
        px_srcline(116);
        return px_bool(true);
    }
    px_srcline(117);
    return px_bool(false);
px_err_978:
    if (px_err_978_proped) return px_err_978_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_def_name");
    LXValue _v982 = (nargs > 0) ? args[0] : px_null();
    LXValue _v983 = px_null();
    LXValue _v984 = px_null();
    LXValue _v985 = px_null();
    LXValue _v986 = px_null();
    LXValue px_err_987_val = px_null();
    int px_err_987_proped = 0;
    px_srcline(120);
    _v983 = px_index(_v982, px_int(0LL));
    px_srcline(121);
    if (px_is_truthy(({ LXValue _t991 = ({ LXValue _t990 = ({ LXValue _t989 = ({ LXValue _t988 = px_eq(_v983, px_str("FuncDef")); px_is_truthy(_t988) ? _t988 : px_eq(_v983, px_str("StructDef")); }); px_is_truthy(_t989) ? _t989 : px_eq(_v983, px_str("EnumDef")); }); px_is_truthy(_t990) ? _t990 : px_eq(_v983, px_str("TraitDef")); }); px_is_truthy(_t991) ? _t991 : px_eq(_v983, px_str("ExternDef")); }))) {
        px_srcline(122);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v982, px_int(1LL))}, 1);
    }
    px_srcline(123);
    if (px_is_truthy(px_eq(_v983, px_str("VarDecl")))) {
        px_srcline(124);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v982, px_int(2LL))}, 1);
    }
    px_srcline(125);
    if (px_is_truthy(px_eq(_v983, px_str("ImplDef")))) {
        px_srcline(126);
        _v984 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v982, px_int(1LL))}, 1);
        px_srcline(127);
        _v985 = px_index(_v982, px_int(2LL));
        px_srcline(128);
        _v986 = px_str("None");
        px_srcline(129);
        if (px_is_truthy(px_ne(_v985, px_null()))) {
            px_srcline(130);
             _v986 = px_add(px_add(px_str("Some("), _v985), px_str(")"));
        }
        px_srcline(131);
        return px_add(px_add(px_add(px_str("impl::"), _v984), px_str("::")), _v986);
    }
    px_srcline(132);
    return px_null();
px_err_987:
    if (px_err_987_proped) return px_err_987_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_module");
    LXValue _v992 = (nargs > 0) ? args[0] : px_null();
    LXValue _v993 = (nargs > 1) ? args[1] : px_null();
    LXValue _v994 = (nargs > 2) ? args[2] : px_null();
    LXValue _v995 = (nargs > 3) ? args[3] : px_null();
    LXValue _v996 = (nargs > 4) ? args[4] : px_null();
    LXValue _v997 = px_null();
    LXValue _v998 = px_null();
    LXValue _v999 = px_null();
    LXValue _v1000 = px_null();
    LXValue _v1001 = px_null();
    LXValue _v1002 = px_null();
    LXValue _v1003 = px_null();
    LXValue _v1004 = px_null();
    LXValue _v1005 = px_null();
    LXValue _v1006 = px_null();
    LXValue _v1007 = px_null();
    LXValue _v1008 = px_null();
    LXValue _v1009 = px_null();
    LXValue _v1010 = px_null();
    LXValue _v1011 = px_null();
    LXValue _v1012 = px_null();
    LXValue _v1013 = px_null();
    LXValue _v1014 = px_null();
    LXValue _v1015 = px_null();
    LXValue px_err_1016_val = px_null();
    int px_err_1016_proped = 0;
    px_srcline(136);
    _v997 = px_list_n((LXValue[]){}, 0);
    px_srcline(137);
    _v998 = px_int(0LL);
    px_srcline(138);
    while (px_is_truthy(px_lt(_v998, px_call(px_get_global("len"), (LXValue[]){_v992}, 1)))) {
        px_srcline(139);
        (void)(px_method(_v997, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v992, _v998)}, 1)}, 1));
        px_srcline(140);
         _v998 = px_add(_v998, px_int(1LL));
    }
    px_srcline(141);
     _v992 = _v997;
    px_srcline(142);
    _v999 = px_list_n((LXValue[]){}, 0);
    px_srcline(143);
    _v1000 = px_int(0LL);
    px_srcline(144);
    while (px_is_truthy(px_lt(_v1000, px_call(px_get_global("len"), (LXValue[]){_v993}, 1)))) {
        px_srcline(145);
        (void)(px_method(_v999, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v993, _v1000)}, 1)}, 1));
        px_srcline(146);
         _v1000 = px_add(_v1000, px_int(1LL));
    }
    px_srcline(147);
     _v993 = _v999;
    px_srcline(148);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v992}, 1), px_int(0LL)))) {
        px_srcline(149);
        return px_null();
    }
    px_srcline(151);
    if (px_is_truthy(({ LXValue _t1019 = ({ LXValue _t1018 = ({ LXValue _t1017 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v992}, 1), px_int(1LL)); px_is_truthy(_t1017) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v992, px_int(0LL))}, 1), px_int(2LL)) : _t1017; }); px_is_truthy(_t1018) ? px_eq(px_slice(px_index(_v992, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t1018; }); px_is_truthy(_t1019) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v992, px_int(0LL)), px_str(".px")}, 2)) : _t1019; }))) {
        px_srcline(152);
        return px_null();
    }
    px_srcline(153);
    _v1001 = px_eq(px_index(_v992, px_int(0LL)), px_str("std"));
    px_srcline(154);
    _v1002 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v992}, 2);
    px_srcline(155);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1002}, 1))) {
        px_srcline(156);
        return px_null();
    }
    px_srcline(157);
    _v1003 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v992, _v994}, 2);
    px_srcline(158);
    if (px_is_truthy(px_eq(_v1003, px_null()))) {
        px_srcline(165);
        _v1004 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(166);
        _v1005 = px_str("");
        px_srcline(167);
        if (px_is_truthy(({ LXValue _t1020 = px_eq(px_index(_v992, px_int(0LL)), px_str("std")); px_is_truthy(_t1020) ? px_eq(_v1004, px_null()) : _t1020; }))) {
            px_srcline(168);
             _v1005 = px_str("；stdlib 未找到（设 PX_STDLIB 或放置 stdlib/）");
        }
        px_srcline(169);
        if (px_is_truthy(px_ne(px_call(px_get_global("env"), (LXValue[]){px_str("PX_STRICT_MODULE")}, 1), px_null()))) {
            px_srcline(170);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3005"), px_add(px_add(px_add(px_str("找不到模块 '"), _v1002), px_str("'")), _v1005)}, 2));
        }
        px_srcline(171);
        (void)(px_call(px_get_global("cg_pwarn"), (LXValue[]){px_add(px_add(px_add(px_str("找不到模块 '"), _v1002), px_str("'（已跳过 → 运行期将报未定义）")), _v1005)}, 1));
        px_srcline(172);
        return px_null();
    }
    px_srcline(173);
    px_index_set(px_get_global("loaded"), _v1002, _v1003);
    px_srcline(174);
    _v1006 = px_call(px_get_global("read_file"), (LXValue[]){_v1003}, 1);
    px_srcline(175);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1006}, 1));
    px_srcline(176);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(177);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(178);
    _v1007 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(180);
    _v1008 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1003}, 1);
    px_srcline(181);
    _v1009 = px_list_n((LXValue[]){}, 0);
    px_srcline(182);
    _v1010 = px_int(0LL);
    px_srcline(183);
    while (px_is_truthy(px_lt(_v1010, px_call(px_get_global("len"), (LXValue[]){px_index(_v1007, px_int(1LL))}, 1)))) {
        px_srcline(184);
        _v1011 = px_index(px_index(_v1007, px_int(1LL)), _v1010);
        px_srcline(185);
        if (px_is_truthy(px_eq(px_index(_v1011, px_int(0LL)), px_str("Import")))) {
            px_srcline(186);
            (void)(px_method(_v1009, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1011, px_int(1LL)), px_index(_v1011, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(187);
         _v1010 = px_add(_v1010, px_int(1LL));
    }
    px_srcline(188);
    _v1012 = px_int(0LL);
    px_srcline(189);
    while (px_is_truthy(px_lt(_v1012, px_call(px_get_global("len"), (LXValue[]){_v1009}, 1)))) {
        px_srcline(190);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1009, _v1012), px_int(0LL)), px_index(px_index(_v1009, _v1012), px_int(1LL)), _v1008, _v995, _v996}, 5));
        px_srcline(191);
         _v1012 = px_add(_v1012, px_int(1LL));
    }
    px_srcline(193);
    _v1013 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v993}, 1), px_int(0LL));
    px_srcline(194);
    _v1014 = px_int(0LL);
    px_srcline(195);
    while (px_is_truthy(px_lt(_v1014, px_call(px_get_global("len"), (LXValue[]){px_index(_v1007, px_int(1LL))}, 1)))) {
        px_srcline(196);
        _v1011 = px_index(px_index(_v1007, px_int(1LL)), _v1014);
        px_srcline(197);
        if (px_is_truthy(px_eq(px_index(_v1011, px_int(0LL)), px_str("Import")))) {
            px_srcline(198);
             _v1014 = px_add(_v1014, px_int(1LL));
            px_srcline(199);
            continue;
        }
        px_srcline(200);
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1011}, 1)))) {
            px_srcline(201);
             _v1014 = px_add(_v1014, px_int(1LL));
            px_srcline(202);
            continue;
        }
        px_srcline(203);
        _v1015 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1011}, 1);
        px_srcline(204);
        if (px_is_truthy(px_eq(_v1015, px_null()))) {
            px_srcline(205);
            (void)(px_method(_v995, "append", (LXValue[]){_v1011}, 1));
        }
        else {
            px_srcline(207);
            if (px_is_truthy(_v1013)) {
                px_srcline(208);
                if (px_is_truthy(({ LXValue _t1021 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1015}, 1), px_int(5LL)); px_is_truthy(_t1021) ? px_eq(px_slice(_v1015, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1021; }))) {
                    px_srcline(209);
                     _v1014 = px_add(_v1014, px_int(1LL));
                    px_srcline(210);
                    continue;
                }
                px_srcline(211);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v993, _v1015}, 2)))) {
                    px_srcline(212);
                     _v1014 = px_add(_v1014, px_int(1LL));
                    px_srcline(213);
                    continue;
                }
            }
            px_srcline(214);
            if (px_is_truthy(_v1001)) {
                px_srcline(215);
                if (px_is_truthy(px_method(_v996, "has", (LXValue[]){_v1015}, 1))) {
                    px_srcline(216);
                     _v1014 = px_add(_v1014, px_int(1LL));
                    px_srcline(217);
                    continue;
                }
                px_srcline(218);
                px_index_set(_v996, _v1015, px_bool(true));
            }
            px_srcline(219);
            (void)(px_method(_v995, "append", (LXValue[]){_v1011}, 1));
        }
        px_srcline(220);
         _v1014 = px_add(_v1014, px_int(1LL));
    }
px_err_1016:
    if (px_err_1016_proped) return px_err_1016_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_resolve_modules");
    LXValue _v1022 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1023 = (nargs > 1) ? args[1] : px_null();
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
    LXValue px_err_1034_val = px_null();
    int px_err_1034_proped = 0;
    px_srcline(223);
    _v1024 = px_index(_v1022, px_int(1LL));
    px_srcline(224);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1024}, 1), px_int(0LL)))) {
        px_srcline(225);
        return _v1022;
    }
    px_srcline(226);
    _v1025 = px_list_n((LXValue[]){}, 0);
    px_srcline(227);
    _v1026 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(228);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(229);
    _v1027 = px_list_n((LXValue[]){}, 0);
    px_srcline(230);
    _v1028 = px_int(0LL);
    px_srcline(231);
    while (px_is_truthy(px_lt(_v1028, px_call(px_get_global("len"), (LXValue[]){_v1024}, 1)))) {
        px_srcline(232);
        _v1029 = px_index(_v1024, _v1028);
        px_srcline(233);
        if (px_is_truthy(px_eq(px_index(_v1029, px_int(0LL)), px_str("Import")))) {
            px_srcline(234);
            (void)(px_method(_v1027, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1029, px_int(1LL)), px_index(_v1029, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(235);
         _v1028 = px_add(_v1028, px_int(1LL));
    }
    px_srcline(236);
    _v1030 = px_int(0LL);
    px_srcline(237);
    while (px_is_truthy(px_lt(_v1030, px_call(px_get_global("len"), (LXValue[]){_v1027}, 1)))) {
        px_srcline(238);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1027, _v1030), px_int(0LL)), px_index(px_index(_v1027, _v1030), px_int(1LL)), _v1023, _v1025, _v1026}, 5));
        px_srcline(239);
         _v1030 = px_add(_v1030, px_int(1LL));
    }
    px_srcline(240);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1025}, 1), px_int(0LL)))) {
        px_srcline(241);
        return _v1022;
    }
    px_srcline(242);
    _v1031 = px_list_n((LXValue[]){}, 0);
    px_srcline(243);
    _v1032 = px_int(0LL);
    px_srcline(244);
    while (px_is_truthy(px_lt(_v1032, px_call(px_get_global("len"), (LXValue[]){_v1025}, 1)))) {
        px_srcline(245);
        (void)(px_method(_v1031, "append", (LXValue[]){px_index(_v1025, _v1032)}, 1));
        px_srcline(246);
         _v1032 = px_add(_v1032, px_int(1LL));
    }
    px_srcline(247);
    _v1033 = px_int(0LL);
    px_srcline(248);
    while (px_is_truthy(px_lt(_v1033, px_call(px_get_global("len"), (LXValue[]){_v1024}, 1)))) {
        px_srcline(249);
        (void)(px_method(_v1031, "append", (LXValue[]){px_index(_v1024, _v1033)}, 1));
        px_srcline(250);
         _v1033 = px_add(_v1033, px_int(1LL));
    }
    px_srcline(251);
    return px_list_n((LXValue[]){px_str("Program"), _v1031}, 2);
px_err_1034:
    if (px_err_1034_proped) return px_err_1034_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_dict");
    LXValue _v1035 = px_null();
    LXValue px_err_1036_val = px_null();
    int px_err_1036_proped = 0;
    px_srcline(57);
    _v1035 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(58);
    (void)(px_method(_v1035, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(59);
    return _v1035;
px_err_1036:
    if (px_err_1036_proped) return px_err_1036_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dict_copy");
    LXValue _v1037 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1038 = px_null();
    LXValue _v1039 = px_null();
    LXValue _v1040 = px_null();
    LXValue px_err_1041_val = px_null();
    int px_err_1041_proped = 0;
    px_srcline(61);
    _v1038 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(62);
    _v1039 = px_method(_v1037, "keys", (LXValue[]){}, 0);
    px_srcline(63);
    _v1040 = px_int(0LL);
    px_srcline(64);
    while (px_is_truthy(px_lt(_v1040, px_call(px_get_global("len"), (LXValue[]){_v1039}, 1)))) {
        px_srcline(65);
        px_index_set(_v1038, px_index(_v1039, _v1040), px_index(_v1037, px_index(_v1039, _v1040)));
        px_srcline(66);
         _v1040 = px_add(_v1040, px_int(1LL));
    }
    px_srcline(67);
    return _v1038;
px_err_1041:
    if (px_err_1041_proped) return px_err_1041_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_uid");
    LXValue px_err_1042_val = px_null();
    int px_err_1042_proped = 0;
    px_srcline(69);
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    px_srcline(70);
    return px_get_global("cg_uidc");
px_err_1042:
    if (px_err_1042_proped) return px_err_1042_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_tmp");
    LXValue px_err_1043_val = px_null();
    int px_err_1043_proped = 0;
    px_srcline(72);
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1043:
    if (px_err_1043_proped) return px_err_1043_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_var");
    LXValue _v1044 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1045 = px_null();
    LXValue px_err_1046_val = px_null();
    int px_err_1046_proped = 0;
    px_srcline(74);
    _v1045 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(75);
    px_index_set(px_get_global("cg_vars"), _v1044, _v1045);
    px_srcline(78);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1044}, 1))) {
        px_srcline(79);
        (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v1044}, 1));
    }
    px_srcline(80);
    return _v1045;
px_err_1046:
    if (px_err_1046_proped) return px_err_1046_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_var_of");
    LXValue _v1047 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1048_val = px_null();
    int px_err_1048_proped = 0;
    px_srcline(82);
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1047}, 1))) {
        px_srcline(83);
        return px_index(px_get_global("cg_vars"), _v1047);
    }
    px_srcline(84);
    return px_null();
px_err_1048:
    if (px_err_1048_proped) return px_err_1048_val;
    return px_null();
}

static LXValue fn_cg_load_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_of");
    LXValue _v1049 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1050 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1051_val = px_null();
    int px_err_1051_proped = 0;
    px_srcline(89);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1049}, 1))) {
        px_srcline(90);
        return px_add(px_add(px_str("px_cell_get("), _v1050), px_str(")"));
    }
    px_srcline(91);
    return _v1050;
px_err_1051:
    if (px_err_1051_proped) return px_err_1051_val;
    return px_null();
}

static LXValue fn_cg_store_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_store_of");
    LXValue _v1052 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1053 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1054 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1055_val = px_null();
    int px_err_1055_proped = 0;
    px_srcline(93);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1052}, 1))) {
        px_srcline(94);
        return px_add(px_add(px_add(px_add(px_str("px_cell_set("), _v1053), px_str(", ")), _v1054), px_str(")"));
    }
    px_srcline(95);
    return px_add(px_add(_v1053, px_str(" = ")), _v1054);
px_err_1055:
    if (px_err_1055_proped) return px_err_1055_val;
    return px_null();
}

static LXValue fn_cg_name_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_name_add");
    LXValue _v1056 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1057 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1058_val = px_null();
    int px_err_1058_proped = 0;
    px_srcline(97);
    if (px_is_truthy(({ LXValue _t1059 = px_ne(_v1057, px_str("")); px_is_truthy(_t1059) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1056, _v1057}, 2)) : _t1059; }))) {
        px_srcline(98);
        (void)(px_method(_v1056, "append", (LXValue[]){_v1057}, 1));
    }
px_err_1058:
    if (px_err_1058_proped) return px_err_1058_val;
    return px_null();
}

static LXValue fn_cg_ast_used(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_used");
    LXValue _v1060 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1061 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1062 = px_null();
    LXValue px_err_1063_val = px_null();
    int px_err_1063_proped = 0;
    px_srcline(101);
    if (px_is_truthy(({ LXValue _t1064 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1060}, 1), px_str("list")); px_is_truthy(_t1064) ? _t1064 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1060}, 1), px_int(0LL)); }))) {
        px_srcline(102);
        return px_null();
    }
    px_srcline(103);
    if (px_is_truthy(({ LXValue _t1067 = ({ LXValue _t1066 = ({ LXValue _t1065 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1060, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1065) ? px_eq(px_index(_v1060, px_int(0LL)), px_str("Var")) : _t1065; }); px_is_truthy(_t1066) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1060}, 1), px_int(2LL)) : _t1066; }); px_is_truthy(_t1067) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1060, px_int(1LL))}, 1), px_str("string")) : _t1067; }))) {
        px_srcline(104);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1061, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1060, px_int(1LL))}, 1)}, 2));
        px_srcline(105);
        return px_null();
    }
    px_srcline(106);
    LXValue _t1068 = _v1060;
    for (int _t1069 = 0; _t1069 < px_len(_t1068); _t1069++) {
        _v1062 = px_index(_t1068, px_int(_t1069));
        px_srcline(107);
        (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1062, _v1061}, 2));
    }
px_err_1063:
    if (px_err_1063_proped) return px_err_1063_val;
    return px_null();
}

static LXValue fn_cg_ast_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_bound");
    LXValue _v1070 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1071 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1072 = px_null();
    LXValue _v1073 = px_null();
    LXValue _v1074 = px_null();
    LXValue _v1075 = px_null();
    LXValue px_err_1076_val = px_null();
    int px_err_1076_proped = 0;
    px_srcline(110);
    if (px_is_truthy(({ LXValue _t1077 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1070}, 1), px_str("list")); px_is_truthy(_t1077) ? _t1077 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1070}, 1), px_int(0LL)); }))) {
        px_srcline(111);
        return px_null();
    }
    px_srcline(112);
    _v1072 = px_index(_v1070, px_int(0LL));
    px_srcline(113);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1072}, 1), px_str("string")))) {
        px_srcline(114);
        if (px_is_truthy(px_eq(_v1072, px_str("VarDecl")))) {
            px_srcline(115);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1071, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1070, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1072, px_str("Assign")))) {
            px_srcline(117);
            _v1073 = px_index(_v1070, px_int(1LL));
            px_srcline(118);
            if (px_is_truthy(({ LXValue _t1079 = ({ LXValue _t1078 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1073}, 1), px_str("list")); px_is_truthy(_t1078) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1073}, 1), px_int(0LL)) : _t1078; }); px_is_truthy(_t1079) ? px_eq(px_index(_v1073, px_int(0LL)), px_str("Var")) : _t1079; }))) {
                px_srcline(119);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1071, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1073, px_int(1LL))}, 1)}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1072, px_str("For")))) {
            px_srcline(121);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1071, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1070, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1072, px_str("Closure")))) {
            px_srcline(123);
            LXValue _t1080 = px_index(_v1070, px_int(1LL));
            for (int _t1081 = 0; _t1081 < px_len(_t1080); _t1081++) {
                _v1074 = px_index(_t1080, px_int(_t1081));
                px_srcline(124);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1071, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1074, px_int(1LL))}, 1)}, 2));
            }
        }
    }
    px_srcline(125);
    LXValue _t1082 = _v1070;
    for (int _t1083 = 0; _t1083 < px_len(_t1082); _t1083++) {
        _v1075 = px_index(_t1082, px_int(_t1083));
        px_srcline(126);
        (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){_v1075, _v1071}, 2));
    }
px_err_1076:
    if (px_err_1076_proped) return px_err_1076_val;
    return px_null();
}

static LXValue fn_cg_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_closure_caps");
    LXValue _v1084 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1085 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1086 = px_null();
    LXValue _v1087 = px_null();
    LXValue _v1088 = px_null();
    LXValue _v1089 = px_null();
    LXValue px_err_1090_val = px_null();
    int px_err_1090_proped = 0;
    px_srcline(129);
    _v1086 = px_list_n((LXValue[]){}, 0);
    px_srcline(130);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){px_index(_v1084, px_int(3LL)), _v1086}, 2));
    px_srcline(131);
    _v1087 = px_list_n((LXValue[]){}, 0);
    px_srcline(132);
    (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){px_index(_v1084, px_int(3LL)), _v1087}, 2));
    px_srcline(133);
    LXValue _t1091 = px_index(_v1084, px_int(1LL));
    for (int _t1092 = 0; _t1092 < px_len(_t1091); _t1092++) {
        _v1088 = px_index(_t1091, px_int(_t1092));
        px_srcline(134);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1087, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1088, px_int(1LL))}, 1)}, 2));
    }
    px_srcline(135);
    LXValue _t1093 = _v1086;
    for (int _t1094 = 0; _t1094 < px_len(_t1093); _t1094++) {
        _v1089 = px_index(_t1093, px_int(_t1094));
        px_srcline(136);
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1087, _v1089}, 2)))) {
            px_srcline(137);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1085, _v1089}, 2));
        }
    }
px_err_1090:
    if (px_err_1090_proped) return px_err_1090_val;
    return px_null();
}

static LXValue fn_cg_scan_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_scan_closure_caps");
    LXValue _v1095 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1096 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1097 = px_null();
    LXValue px_err_1098_val = px_null();
    int px_err_1098_proped = 0;
    px_srcline(140);
    if (px_is_truthy(({ LXValue _t1099 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1095}, 1), px_str("list")); px_is_truthy(_t1099) ? _t1099 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1095}, 1), px_int(0LL)); }))) {
        px_srcline(141);
        return px_null();
    }
    px_srcline(142);
    if (px_is_truthy(({ LXValue _t1100 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1095, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1100) ? px_eq(px_index(_v1095, px_int(0LL)), px_str("Closure")) : _t1100; }))) {
        px_srcline(143);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){_v1095, _v1096}, 2));
    }
    px_srcline(144);
    LXValue _t1101 = _v1095;
    for (int _t1102 = 0; _t1102 < px_len(_t1101); _t1102++) {
        _v1097 = px_index(_t1101, px_int(_t1102));
        px_srcline(145);
        (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1097, _v1096}, 2));
    }
px_err_1098:
    if (px_err_1098_proped) return px_err_1098_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_mark_immutable");
    LXValue _v1103 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1104_val = px_null();
    int px_err_1104_proped = 0;
    px_srcline(148);
    px_index_set(px_get_global("cg_immutables"), _v1103, px_int(1LL));
px_err_1104:
    if (px_err_1104_proped) return px_err_1104_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_immutable");
    LXValue _v1105 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1106_val = px_null();
    int px_err_1106_proped = 0;
    px_srcline(150);
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1105}, 1);
px_err_1106:
    if (px_err_1106_proped) return px_err_1106_val;
    return px_null();
}

static LXValue fn_cg_is_wildcard(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_wildcard");
    LXValue _v1107 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1108_val = px_null();
    int px_err_1108_proped = 0;
    px_srcline(158);
    return px_eq(_v1107, px_str("_"));
px_err_1108:
    if (px_err_1108_proped) return px_err_1108_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_perr");
    LXValue _v1109 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1110 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1111_val = px_null();
    int px_err_1111_proped = 0;
    px_srcline(163);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1109), px_str(": ")), _v1110)}, 1));
    px_srcline(164);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1111:
    if (px_err_1111_proped) return px_err_1111_val;
    return px_null();
}

static LXValue fn_cg_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pwarn");
    LXValue _v1112 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1113_val = px_null();
    int px_err_1113_proped = 0;
    px_srcline(167);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1112)}, 1));
px_err_1113:
    if (px_err_1113_proped) return px_err_1113_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_nonnull_ty");
    LXValue _v1114 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1115_val = px_null();
    int px_err_1115_proped = 0;
    px_srcline(171);
    if (px_is_truthy(px_eq(_v1114, px_null()))) {
        px_srcline(172);
        return px_bool(false);
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(px_index(_v1114, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(174);
        return px_bool(false);
    }
    px_srcline(175);
    return px_bool(true);
px_err_1115:
    if (px_err_1115_proped) return px_err_1115_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_null_lit");
    LXValue _v1116 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1117_val = px_null();
    int px_err_1117_proped = 0;
    px_srcline(178);
    if (px_is_truthy(px_eq(_v1116, px_null()))) {
        px_srcline(179);
        return px_bool(false);
    }
    px_srcline(180);
    if (px_is_truthy(px_eq(px_index(_v1116, px_int(0LL)), px_str("Null")))) {
        px_srcline(181);
        return px_bool(true);
    }
    px_srcline(182);
    return px_bool(false);
px_err_1117:
    if (px_err_1117_proped) return px_err_1117_val;
    return px_null();
}

static LXValue fn_cg_sem_vardecl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_vardecl");
    LXValue _v1118 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1119 = px_null();
    LXValue _v1120 = px_null();
    LXValue px_err_1121_val = px_null();
    int px_err_1121_proped = 0;
    px_srcline(200);
    _v1119 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1118, px_int(2LL))}, 1);
    px_srcline(202);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1119}, 1))) {
        px_srcline(203);
        return px_null();
    }
    px_srcline(204);
    if (px_is_truthy(({ LXValue _t1122 = px_eq(px_index(_v1118, px_int(1LL)), px_str("Let")); px_is_truthy(_t1122) ? _t1122 : px_eq(px_index(_v1118, px_int(1LL)), px_str("Const")); }))) {
        px_srcline(205);
        (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v1119}, 1));
    }
    px_srcline(206);
    _v1120 = px_index(_v1118, px_int(3LL));
    px_srcline(207);
    if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v1120}, 1))) {
        px_srcline(208);
        px_index_set(px_get_global("cg_nonnull"), _v1119, px_int(1LL));
        px_srcline(209);
        if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v1118, px_int(4LL))}, 1))) {
            px_srcline(210);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1120}, 1)), px_str("'（可空类型请用 ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1120}, 1)), px_str("? 声明）"))}, 2));
        }
    }
px_err_1121:
    if (px_err_1121_proped) return px_err_1121_val;
    return px_null();
}

static LXValue fn_cg_sem_assign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_assign");
    LXValue _v1123 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1124 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1125 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1126 = px_null();
    LXValue px_err_1127_val = px_null();
    int px_err_1127_proped = 0;
    px_srcline(214);
    if (px_is_truthy(px_eq(_v1123, px_null()))) {
        px_srcline(215);
        return px_null();
    }
    px_srcline(216);
    if (px_is_truthy(px_ne(px_index(_v1123, px_int(0LL)), px_str("Var")))) {
        px_srcline(217);
        return px_null();
    }
    px_srcline(218);
    _v1126 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1123, px_int(1LL))}, 1);
    px_srcline(220);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1126}, 1))) {
        px_srcline(221);
        return px_null();
    }
    px_srcline(222);
    if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v1126}, 1))) {
        px_srcline(223);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v1126), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
    }
    px_srcline(224);
    if (px_is_truthy(({ LXValue _t1128 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){_v1125}, 1); px_is_truthy(_t1128) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v1126}, 1) : _t1128; }))) {
        px_srcline(225);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v1126), px_str("'（可空类型请声明为 ")), _v1126), px_str(": T?）"))}, 2));
    }
px_err_1127:
    if (px_err_1127_proped) return px_err_1127_val;
    return px_null();
}

static LXValue fn_cg_sem_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_call");
    LXValue _v1129 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1130 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1131 = px_null();
    LXValue _v1132 = px_null();
    LXValue px_err_1133_val = px_null();
    int px_err_1133_proped = 0;
    px_srcline(228);
    if (px_is_truthy(px_eq(_v1129, px_null()))) {
        px_srcline(229);
        return px_null();
    }
    px_srcline(230);
    if (px_is_truthy(px_ne(px_index(_v1129, px_int(0LL)), px_str("Var")))) {
        px_srcline(231);
        return px_null();
    }
    px_srcline(232);
    _v1131 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1129, px_int(1LL))}, 1);
    px_srcline(233);
    if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v1131}, 1))) {
        px_srcline(234);
        _v1132 = px_index(px_get_global("cg_ffi"), _v1131);
        px_srcline(235);
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1130}, 1), px_call(px_get_global("len"), (LXValue[]){_v1132}, 1)))) {
            px_srcline(236);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), px_add(px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v1131), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1132}, 1)}, 1)), px_str(" 个参数，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1130}, 1)}, 1))}, 2));
        }
    }
px_err_1133:
    if (px_err_1133_proped) return px_err_1133_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ty_name");
    LXValue _v1134 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1135_val = px_null();
    int px_err_1135_proped = 0;
    px_srcline(239);
    if (px_is_truthy(px_eq(_v1134, px_null()))) {
        px_srcline(240);
        return px_str("any");
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(px_index(_v1134, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(242);
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1134, px_int(1LL))}, 1), px_str("?"));
    }
    px_srcline(243);
    if (px_is_truthy(px_eq(px_index(_v1134, px_int(0LL)), px_str("TyNamed")))) {
        px_srcline(244);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1134, px_int(1LL))}, 1);
    }
    px_srcline(245);
    if (px_is_truthy(px_eq(px_index(_v1134, px_int(0LL)), px_str("TyList")))) {
        px_srcline(246);
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1134, px_int(1LL))}, 1)), px_str("]"));
    }
    px_srcline(247);
    if (px_is_truthy(px_eq(px_index(_v1134, px_int(0LL)), px_str("TyDict")))) {
        px_srcline(248);
        return px_add(px_add(px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1134, px_int(1LL))}, 1)), px_str(": ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1134, px_int(2LL))}, 1)), px_str("}"));
    }
    px_srcline(249);
    return px_str("any");
px_err_1135:
    if (px_err_1135_proped) return px_err_1135_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_func_cname");
    LXValue _v1136 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1137 = px_null();
    LXValue _v1138 = px_null();
    LXValue _v1139 = px_null();
    LXValue _v1140 = px_null();
    LXValue _v1141 = px_null();
    LXValue _v1142 = px_null();
    LXValue px_err_1143_val = px_null();
    int px_err_1143_proped = 0;
    px_srcline(251);
    _v1137 = px_str("");
    px_srcline(252);
    _v1138 = px_int(0LL);
    px_srcline(253);
    while (px_is_truthy(px_lt(_v1138, px_call(px_get_global("len"), (LXValue[]){_v1136}, 1)))) {
        px_srcline(254);
        _v1139 = px_index(_v1136, _v1138);
        px_srcline(255);
        _v1140 = ({ LXValue _t1144 = px_ge(_v1139, px_str("a")); px_is_truthy(_t1144) ? px_le(_v1139, px_str("z")) : _t1144; });
        px_srcline(256);
        _v1141 = ({ LXValue _t1145 = px_ge(_v1139, px_str("A")); px_is_truthy(_t1145) ? px_le(_v1139, px_str("Z")) : _t1145; });
        px_srcline(257);
        _v1142 = ({ LXValue _t1146 = px_ge(_v1139, px_str("0")); px_is_truthy(_t1146) ? px_le(_v1139, px_str("9")) : _t1146; });
        px_srcline(258);
        if (px_is_truthy(({ LXValue _t1148 = ({ LXValue _t1147 = _v1140; px_is_truthy(_t1147) ? _t1147 : _v1141; }); px_is_truthy(_t1148) ? _t1148 : _v1142; }))) {
            px_srcline(259);
             _v1137 = px_add(_v1137, _v1139);
        }
        else {
            px_srcline(261);
             _v1137 = px_add(_v1137, px_str("_"));
        }
        px_srcline(262);
         _v1138 = px_add(_v1138, px_int(1LL));
    }
    px_srcline(263);
    return _v1137;
px_err_1143:
    if (px_err_1143_proped) return px_err_1143_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find");
    LXValue _v1149 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1150 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1151 = px_null();
    LXValue _v1152 = px_null();
    LXValue _v1153 = px_null();
    LXValue _v1154 = px_null();
    LXValue _v1155 = px_null();
    LXValue px_err_1156_val = px_null();
    int px_err_1156_proped = 0;
    px_srcline(265);
    _v1151 = px_call(px_get_global("len"), (LXValue[]){_v1149}, 1);
    px_srcline(266);
    _v1152 = px_call(px_get_global("len"), (LXValue[]){_v1150}, 1);
    px_srcline(267);
    _v1153 = px_int(0LL);
    px_srcline(268);
    while (px_is_truthy(px_le(px_add(_v1153, _v1152), _v1151))) {
        px_srcline(269);
        _v1154 = px_int(0LL);
        px_srcline(270);
        _v1155 = px_bool(true);
        px_srcline(271);
        while (px_is_truthy(px_lt(_v1154, _v1152))) {
            px_srcline(272);
            if (px_is_truthy(px_ne(px_index(_v1149, px_add(_v1153, _v1154)), px_index(_v1150, _v1154)))) {
                px_srcline(273);
                 _v1155 = px_bool(false);
                px_srcline(274);
                break;
            }
            px_srcline(275);
             _v1154 = px_add(_v1154, px_int(1LL));
        }
        px_srcline(276);
        if (px_is_truthy(_v1155)) {
            px_srcline(277);
            return _v1153;
        }
        px_srcline(278);
         _v1153 = px_add(_v1153, px_int(1LL));
    }
    px_srcline(279);
    return px_neg(px_int(1LL));
px_err_1156:
    if (px_err_1156_proped) return px_err_1156_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad");
    LXValue _v1157 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1158 = px_null();
    LXValue _v1159 = px_null();
    LXValue px_err_1160_val = px_null();
    int px_err_1160_proped = 0;
    px_srcline(281);
    _v1158 = px_str("");
    px_srcline(282);
    _v1159 = px_int(0LL);
    px_srcline(283);
    while (px_is_truthy(px_lt(_v1159, _v1157))) {
        px_srcline(284);
         _v1158 = px_add(_v1158, px_str("    "));
        px_srcline(285);
         _v1159 = px_add(_v1159, px_int(1LL));
    }
    px_srcline(286);
    return _v1158;
px_err_1160:
    if (px_err_1160_proped) return px_err_1160_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_unescape");
    LXValue _v1161 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1162 = px_null();
    LXValue _v1163 = px_null();
    LXValue _v1164 = px_null();
    LXValue _v1165 = px_null();
    LXValue _v1166 = px_null();
    LXValue _v1167 = px_null();
    LXValue _v1168 = px_null();
    LXValue px_err_1169_val = px_null();
    int px_err_1169_proped = 0;
    px_srcline(289);
    _v1162 = px_slice(_v1161, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1161}, 1), px_int(1LL)), px_null());
    px_srcline(290);
    _v1163 = px_str("");
    px_srcline(291);
    _v1164 = px_int(0LL);
    px_srcline(292);
    while (px_is_truthy(px_lt(_v1164, px_call(px_get_global("len"), (LXValue[]){_v1162}, 1)))) {
        px_srcline(293);
        _v1165 = px_index(_v1162, _v1164);
        px_srcline(294);
        if (px_is_truthy(px_eq(_v1165, px_str("\\")))) {
            px_srcline(295);
            _v1166 = px_index(_v1162, px_add(_v1164, px_int(1LL)));
            px_srcline(296);
            if (px_is_truthy(px_eq(_v1166, px_str("n")))) {
                px_srcline(297);
                 _v1163 = px_add(_v1163, px_str("\n"));
                px_srcline(298);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1166, px_str("t")))) {
                px_srcline(300);
                 _v1163 = px_add(_v1163, px_str("\t"));
                px_srcline(301);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1166, px_str("r")))) {
                px_srcline(303);
                 _v1163 = px_add(_v1163, px_str("\r"));
                px_srcline(304);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1166, px_str("0")))) {
                px_srcline(306);
                 _v1163 = px_add(_v1163, px_str(""));
                px_srcline(307);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1166, px_str("\"")))) {
                px_srcline(309);
                 _v1163 = px_add(_v1163, px_str("\""));
                px_srcline(310);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1166, px_str("\\")))) {
                px_srcline(312);
                 _v1163 = px_add(_v1163, px_str("\\"));
                px_srcline(313);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1166, px_str("u")))) {
                px_srcline(315);
                _v1167 = px_add(_v1164, px_int(3LL));
                px_srcline(316);
                _v1168 = px_str("");
                px_srcline(317);
                while (px_is_truthy(({ LXValue _t1170 = px_lt(_v1167, px_call(px_get_global("len"), (LXValue[]){_v1162}, 1)); px_is_truthy(_t1170) ? px_ne(px_index(_v1162, _v1167), px_str("}")) : _t1170; }))) {
                    px_srcline(318);
                     _v1168 = px_add(_v1168, px_index(_v1162, _v1167));
                    px_srcline(319);
                     _v1167 = px_add(_v1167, px_int(1LL));
                }
                px_srcline(320);
                 _v1163 = px_add(_v1163, px_call(px_get_global("hex_to_char"), (LXValue[]){_v1168}, 1));
                px_srcline(321);
                 _v1164 = px_add(_v1167, px_int(1LL));
            }
            else {
                px_srcline(323);
                 _v1163 = px_add(_v1163, _v1166);
                px_srcline(324);
                 _v1164 = px_add(_v1164, px_int(2LL));
            }
        }
        else {
            px_srcline(326);
             _v1163 = px_add(_v1163, _v1165);
            px_srcline(327);
             _v1164 = px_add(_v1164, px_int(1LL));
        }
    }
    px_srcline(328);
    return _v1163;
px_err_1169:
    if (px_err_1169_proped) return px_err_1169_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_escape_str");
    LXValue _v1171 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1172 = px_null();
    LXValue _v1173 = px_null();
    LXValue _v1174 = px_null();
    LXValue px_err_1175_val = px_null();
    int px_err_1175_proped = 0;
    px_srcline(331);
    _v1172 = px_str("");
    px_srcline(332);
    _v1173 = px_int(0LL);
    px_srcline(333);
    while (px_is_truthy(px_lt(_v1173, px_call(px_get_global("len"), (LXValue[]){_v1171}, 1)))) {
        px_srcline(334);
        _v1174 = px_index(_v1171, _v1173);
        px_srcline(335);
        if (px_is_truthy(px_eq(_v1174, px_str("\\")))) {
            px_srcline(336);
             _v1172 = px_add(_v1172, px_str("\\\\"));
        }
        else if (px_is_truthy(px_eq(_v1174, px_str("\"")))) {
            px_srcline(338);
             _v1172 = px_add(_v1172, px_str("\\\""));
        }
        else if (px_is_truthy(px_eq(_v1174, px_str("\n")))) {
            px_srcline(340);
             _v1172 = px_add(_v1172, px_str("\\n"));
        }
        else if (px_is_truthy(px_eq(_v1174, px_str("\r")))) {
            px_srcline(342);
             _v1172 = px_add(_v1172, px_str("\\r"));
        }
        else if (px_is_truthy(px_eq(_v1174, px_str("\t")))) {
            px_srcline(344);
             _v1172 = px_add(_v1172, px_str("\\t"));
        }
        else if (px_is_truthy(px_eq(_v1174, px_str("")))) {
            px_srcline(348);
             _v1172 = px_add(_v1172, px_str(""));
        }
        else {
            px_srcline(350);
             _v1172 = px_add(_v1172, _v1174);
        }
        px_srcline(351);
         _v1173 = px_add(_v1173, px_int(1LL));
    }
    px_srcline(352);
    return _v1172;
px_err_1175:
    if (px_err_1175_proped) return px_err_1175_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad_zeros");
    LXValue _v1176 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1177 = px_null();
    LXValue _v1178 = px_null();
    LXValue px_err_1179_val = px_null();
    int px_err_1179_proped = 0;
    px_srcline(355);
    _v1177 = px_str("");
    px_srcline(356);
    _v1178 = px_int(0LL);
    px_srcline(357);
    while (px_is_truthy(px_lt(_v1178, _v1176))) {
        px_srcline(358);
         _v1177 = px_add(_v1177, px_str("0"));
        px_srcline(359);
         _v1178 = px_add(_v1178, px_int(1LL));
    }
    px_srcline(360);
    return _v1177;
px_err_1179:
    if (px_err_1179_proped) return px_err_1179_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_expand_sci");
    LXValue _v1180 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1181 = px_null();
    LXValue _v1182 = px_null();
    LXValue _v1183 = px_null();
    LXValue _v1184 = px_null();
    LXValue _v1185 = px_null();
    LXValue _v1186 = px_null();
    LXValue _v1187 = px_null();
    LXValue _v1188 = px_null();
    LXValue _v1189 = px_null();
    LXValue _v1190 = px_null();
    LXValue _v1191 = px_null();
    LXValue _v1192 = px_null();
    LXValue _v1193 = px_null();
    LXValue _v1194 = px_null();
    LXValue px_err_1195_val = px_null();
    int px_err_1195_proped = 0;
    px_srcline(362);
    _v1181 = px_neg(px_int(1LL));
    px_srcline(363);
    _v1182 = px_int(0LL);
    px_srcline(364);
    while (px_is_truthy(px_lt(_v1182, px_call(px_get_global("len"), (LXValue[]){_v1180}, 1)))) {
        px_srcline(365);
        if (px_is_truthy(({ LXValue _t1196 = px_eq(px_index(_v1180, _v1182), px_str("e")); px_is_truthy(_t1196) ? _t1196 : px_eq(px_index(_v1180, _v1182), px_str("E")); }))) {
            px_srcline(366);
             _v1181 = _v1182;
            px_srcline(367);
            break;
        }
        px_srcline(368);
         _v1182 = px_add(_v1182, px_int(1LL));
    }
    px_srcline(369);
    if (px_is_truthy(px_lt(_v1181, px_int(0LL)))) {
        px_srcline(370);
        return _v1180;
    }
    px_srcline(371);
    _v1183 = px_slice(_v1180, px_int(0LL), _v1181, px_null());
    px_srcline(372);
    _v1184 = px_slice(_v1180, px_add(_v1181, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1180}, 1), px_null());
    px_srcline(373);
    _v1185 = px_int(1LL);
    px_srcline(374);
    if (px_is_truthy(({ LXValue _t1197 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1184}, 1), px_int(0LL)); px_is_truthy(_t1197) ? px_eq(px_index(_v1184, px_int(0LL)), px_str("+")) : _t1197; }))) {
        px_srcline(375);
         _v1184 = px_slice(_v1184, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1184}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1198 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1184}, 1), px_int(0LL)); px_is_truthy(_t1198) ? px_eq(px_index(_v1184, px_int(0LL)), px_str("-")) : _t1198; }))) {
        px_srcline(377);
         _v1185 = px_neg(px_int(1LL));
        px_srcline(378);
         _v1184 = px_slice(_v1184, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1184}, 1), px_null());
    }
    px_srcline(379);
    _v1186 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1184}, 1), _v1185);
    px_srcline(380);
    _v1187 = px_bool(false);
    px_srcline(381);
    if (px_is_truthy(({ LXValue _t1199 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1183}, 1), px_int(0LL)); px_is_truthy(_t1199) ? px_eq(px_index(_v1183, px_int(0LL)), px_str("-")) : _t1199; }))) {
        px_srcline(382);
         _v1187 = px_bool(true);
        px_srcline(383);
         _v1183 = px_slice(_v1183, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1183}, 1), px_null());
    }
    px_srcline(384);
    _v1188 = px_str("");
    px_srcline(385);
    _v1189 = px_str("");
    px_srcline(386);
    _v1190 = px_neg(px_int(1LL));
    px_srcline(387);
    _v1191 = px_int(0LL);
    px_srcline(388);
    while (px_is_truthy(px_lt(_v1191, px_call(px_get_global("len"), (LXValue[]){_v1183}, 1)))) {
        px_srcline(389);
        if (px_is_truthy(px_eq(px_index(_v1183, _v1191), px_str(".")))) {
            px_srcline(390);
             _v1190 = _v1191;
            px_srcline(391);
            break;
        }
        px_srcline(392);
         _v1191 = px_add(_v1191, px_int(1LL));
    }
    px_srcline(393);
    if (px_is_truthy(px_lt(_v1190, px_int(0LL)))) {
        px_srcline(394);
         _v1188 = _v1183;
    }
    else {
        px_srcline(396);
         _v1188 = px_slice(_v1183, px_int(0LL), _v1190, px_null());
        px_srcline(397);
         _v1189 = px_slice(_v1183, px_add(_v1190, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1183}, 1), px_null());
    }
    px_srcline(398);
    _v1192 = px_add(_v1188, _v1189);
    px_srcline(399);
    _v1193 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1188}, 1), _v1186);
    px_srcline(400);
    _v1194 = px_str("");
    px_srcline(401);
    if (px_is_truthy(px_le(_v1193, px_int(0LL)))) {
        px_srcline(402);
         _v1194 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1193)}, 1)), _v1192);
    }
    else if (px_is_truthy(px_ge(_v1193, px_call(px_get_global("len"), (LXValue[]){_v1192}, 1)))) {
        px_srcline(404);
         _v1194 = px_add(_v1192, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1193, px_call(px_get_global("len"), (LXValue[]){_v1192}, 1))}, 1));
    }
    else {
        px_srcline(406);
         _v1194 = px_add(px_add(px_slice(_v1192, px_int(0LL), _v1193, px_null()), px_str(".")), px_slice(_v1192, _v1193, px_call(px_get_global("len"), (LXValue[]){_v1192}, 1), px_null()));
    }
    px_srcline(407);
    if (px_is_truthy(_v1187)) {
        px_srcline(408);
        return px_add(px_str("-"), _v1194);
    }
    px_srcline(409);
    return _v1194;
px_err_1195:
    if (px_err_1195_proped) return px_err_1195_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_fmt_float");
    LXValue _v1200 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1201 = px_null();
    LXValue _v1202 = px_null();
    LXValue px_err_1203_val = px_null();
    int px_err_1203_proped = 0;
    px_srcline(412);
    _v1201 = px_call(px_get_global("str"), (LXValue[]){_v1200}, 1);
    px_srcline(413);
    if (px_is_truthy(({ LXValue _t1205 = ({ LXValue _t1204 = px_eq(_v1201, px_str("inf")); px_is_truthy(_t1204) ? _t1204 : px_eq(_v1201, px_str("-inf")); }); px_is_truthy(_t1205) ? _t1205 : px_eq(_v1201, px_str("nan")); }))) {
        px_srcline(414);
        return _v1201;
    }
    px_srcline(415);
     _v1201 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1201}, 1);
    px_srcline(416);
    _v1202 = px_call(px_get_global("len"), (LXValue[]){_v1201}, 1);
    px_srcline(417);
    if (px_is_truthy(({ LXValue _t1206 = px_ge(_v1202, px_int(2LL)); px_is_truthy(_t1206) ? px_eq(px_slice(_v1201, px_sub(_v1202, px_int(2LL)), _v1202, px_null()), px_str(".0")) : _t1206; }))) {
        px_srcline(418);
        return px_slice(_v1201, px_int(0LL), px_sub(_v1202, px_int(2LL)), px_null());
    }
    px_srcline(426);
    if (px_is_truthy(({ LXValue _t1207 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1201, px_str(".")}, 2)); px_is_truthy(_t1207) ? px_ge(_v1202, px_int(19LL)) : _t1207; }))) {
        px_srcline(427);
        return px_add(_v1201, px_str(".0"));
    }
    px_srcline(428);
    return _v1201;
px_err_1203:
    if (px_err_1203_proped) return px_err_1203_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_types");
    LXValue _v1208 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1209 = px_null();
    LXValue _v1210 = px_null();
    LXValue _v1211 = px_null();
    LXValue _v1212 = px_null();
    LXValue _v1213 = px_null();
    LXValue _v1214 = px_null();
    LXValue _v1215 = px_null();
    LXValue _v1216 = px_null();
    LXValue _v1217 = px_null();
    LXValue _v1218 = px_null();
    LXValue px_err_1219_val = px_null();
    int px_err_1219_proped = 0;
    px_srcline(431);
    _v1209 = px_index(_v1208, px_int(1LL));
    px_srcline(432);
    _v1210 = px_int(0LL);
    px_srcline(433);
    while (px_is_truthy(px_lt(_v1210, px_call(px_get_global("len"), (LXValue[]){_v1209}, 1)))) {
        px_srcline(434);
        _v1211 = px_index(_v1209, _v1210);
        px_srcline(435);
        _v1212 = px_index(_v1211, px_int(0LL));
        px_srcline(436);
        if (px_is_truthy(px_eq(_v1212, px_str("StructDef")))) {
            px_srcline(437);
            _v1213 = px_list_n((LXValue[]){}, 0);
            px_srcline(438);
            _v1214 = px_int(0LL);
            px_srcline(439);
            while (px_is_truthy(px_lt(_v1214, px_call(px_get_global("len"), (LXValue[]){px_index(_v1211, px_int(2LL))}, 1)))) {
                px_srcline(440);
                (void)(px_method(_v1213, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1211, px_int(2LL)), _v1214), px_int(1LL))}, 1)}, 1));
                px_srcline(441);
                 _v1214 = px_add(_v1214, px_int(1LL));
            }
            px_srcline(442);
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1211, px_int(1LL))}, 1), _v1213);
        }
        else if (px_is_truthy(px_eq(_v1212, px_str("EnumDef")))) {
            px_srcline(444);
            _v1215 = px_list_n((LXValue[]){}, 0);
            px_srcline(445);
            _v1216 = px_int(0LL);
            px_srcline(446);
            while (px_is_truthy(px_lt(_v1216, px_call(px_get_global("len"), (LXValue[]){px_index(_v1211, px_int(2LL))}, 1)))) {
                px_srcline(447);
                (void)(px_method(_v1215, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1211, px_int(2LL)), _v1216), px_int(1LL))}, 1)}, 1));
                px_srcline(448);
                 _v1216 = px_add(_v1216, px_int(1LL));
            }
            px_srcline(449);
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1211, px_int(1LL))}, 1), _v1215);
        }
        else if (px_is_truthy(px_eq(_v1212, px_str("ImplDef")))) {
            px_srcline(451);
            _v1217 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1211, px_int(1LL))}, 1);
            px_srcline(452);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1217}, 1))) {
                px_srcline(453);
                _v1218 = px_int(0LL);
                px_srcline(454);
                while (px_is_truthy(px_lt(_v1218, px_call(px_get_global("len"), (LXValue[]){px_index(_v1211, px_int(3LL))}, 1)))) {
                    px_srcline(455);
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1217), "append", (LXValue[]){px_index(px_index(_v1211, px_int(3LL)), _v1218)}, 1));
                    px_srcline(456);
                     _v1218 = px_add(_v1218, px_int(1LL));
                }
            }
            else {
                px_srcline(458);
                px_index_set(px_get_global("cg_impls"), _v1217, px_index(_v1211, px_int(3LL)));
            }
        }
        px_srcline(459);
         _v1210 = px_add(_v1210, px_int(1LL));
    }
    px_srcline(462);
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1209}, 1));
px_err_1219:
    if (px_err_1219_proped) return px_err_1219_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_consts");
    LXValue _v1220 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1221 = px_null();
    LXValue _v1222 = px_null();
    LXValue _v1223 = px_null();
    LXValue _v1224 = px_null();
    LXValue _v1225 = px_null();
    LXValue _v1226 = px_null();
    LXValue _v1227 = px_null();
    LXValue _v1228 = px_null();
    LXValue px_err_1229_val = px_null();
    int px_err_1229_proped = 0;
    px_srcline(465);
    _v1221 = px_int(0LL);
    px_srcline(466);
    while (px_is_truthy(px_lt(_v1221, px_call(px_get_global("len"), (LXValue[]){_v1220}, 1)))) {
        px_srcline(467);
        _v1222 = px_index(_v1220, _v1221);
        px_srcline(468);
        _v1223 = px_index(_v1222, px_int(0LL));
        px_srcline(469);
        if (px_is_truthy(px_eq(_v1223, px_str("TypeConst")))) {
            px_srcline(470);
            _v1224 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1222, px_int(1LL))}, 1);
            px_srcline(471);
            _v1225 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            px_srcline(472);
            _v1226 = px_int(0LL);
            px_srcline(473);
            while (px_is_truthy(px_lt(_v1226, px_call(px_get_global("len"), (LXValue[]){px_index(_v1222, px_int(2LL))}, 1)))) {
                px_srcline(474);
                _v1227 = px_index(px_index(_v1222, px_int(2LL)), _v1226);
                px_srcline(475);
                px_index_set(_v1225, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1227, px_int(1LL))}, 1), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1227, px_int(2LL))}, 1));
                px_srcline(476);
                 _v1226 = px_add(_v1226, px_int(1LL));
            }
            px_srcline(477);
            px_index_set(px_get_global("cg_const_enums"), _v1224, _v1225);
        }
        else if (px_is_truthy(px_eq(_v1223, px_str("FuncDef")))) {
            px_srcline(479);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1222, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1223, px_str("If")))) {
            px_srcline(481);
            _v1228 = px_int(0LL);
            px_srcline(482);
            while (px_is_truthy(px_lt(_v1228, px_call(px_get_global("len"), (LXValue[]){px_index(_v1222, px_int(1LL))}, 1)))) {
                px_srcline(483);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1222, px_int(1LL)), _v1228), px_int(1LL))}, 1));
                px_srcline(484);
                 _v1228 = px_add(_v1228, px_int(1LL));
            }
            px_srcline(485);
            if (px_is_truthy(px_ne(px_index(_v1222, px_int(2LL)), px_null()))) {
                px_srcline(486);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1222, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1223, px_str("For")))) {
            px_srcline(488);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1222, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1223, px_str("While")))) {
            px_srcline(490);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1222, px_int(2LL))}, 1));
        }
        px_srcline(491);
         _v1221 = px_add(_v1221, px_int(1LL));
    }
px_err_1229:
    if (px_err_1229_proped) return px_err_1229_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_hoist_vars");
    LXValue _v1230 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1231 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1232 = px_null();
    LXValue _v1233 = px_null();
    LXValue _v1234 = px_null();
    LXValue _v1235 = px_null();
    LXValue _v1236 = px_null();
    LXValue _v1237 = px_null();
    LXValue _v1238 = px_null();
    LXValue _v1239 = px_null();
    LXValue px_err_1240_val = px_null();
    int px_err_1240_proped = 0;
    px_srcline(500);
    _v1232 = px_int(0LL);
    px_srcline(501);
    while (px_is_truthy(px_lt(_v1232, px_call(px_get_global("len"), (LXValue[]){_v1230}, 1)))) {
        px_srcline(502);
        _v1233 = px_index(_v1230, _v1232);
        px_srcline(503);
        _v1234 = px_index(_v1233, px_int(0LL));
        px_srcline(504);
        if (px_is_truthy(px_eq(_v1234, px_str("Assign")))) {
            px_srcline(505);
            _v1235 = px_index(_v1233, px_int(1LL));
            px_srcline(506);
            if (px_is_truthy(px_eq(px_index(_v1235, px_int(0LL)), px_str("Var")))) {
                px_srcline(507);
                _v1236 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1235, px_int(1LL))}, 1);
                px_srcline(508);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1231, _v1236}, 2)))) {
                    px_srcline(509);
                    (void)(px_method(_v1231, "append", (LXValue[]){_v1236}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1234, px_str("VarDecl")))) {
            px_srcline(511);
            _v1236 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1233, px_int(2LL))}, 1);
            px_srcline(512);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1231, _v1236}, 2)))) {
                px_srcline(513);
                (void)(px_method(_v1231, "append", (LXValue[]){_v1236}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1234, px_str("If")))) {
            px_srcline(515);
            _v1237 = px_index(_v1233, px_int(1LL));
            px_srcline(516);
            _v1238 = px_int(0LL);
            px_srcline(517);
            while (px_is_truthy(px_lt(_v1238, px_call(px_get_global("len"), (LXValue[]){_v1237}, 1)))) {
                px_srcline(518);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1237, _v1238), px_int(1LL)), _v1231}, 2));
                px_srcline(519);
                 _v1238 = px_add(_v1238, px_int(1LL));
            }
            px_srcline(520);
            if (px_is_truthy(px_ne(px_index(_v1233, px_int(2LL)), px_null()))) {
                px_srcline(521);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1233, px_int(2LL)), _v1231}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1234, px_str("For")))) {
            px_srcline(523);
            _v1239 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1233, px_int(1LL))}, 1);
            px_srcline(524);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1231, _v1239}, 2)))) {
                px_srcline(525);
                (void)(px_method(_v1231, "append", (LXValue[]){_v1239}, 1));
            }
            px_srcline(526);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1233, px_int(3LL)), _v1231}, 2));
        }
        else if (px_is_truthy(px_eq(_v1234, px_str("While")))) {
            px_srcline(528);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1233, px_int(2LL)), _v1231}, 2));
        }
        px_srcline(529);
         _v1232 = px_add(_v1232, px_int(1LL));
    }
px_err_1240:
    if (px_err_1240_proped) return px_err_1240_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func");
    LXValue _v1241 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1242 = px_null();
    LXValue px_err_1243_val = px_null();
    int px_err_1243_proped = 0;
    px_srcline(532);
    _v1242 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1241, px_int(1LL))}, 1)}, 1));
    px_srcline(533);
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1241, _v1242}, 2);
px_err_1243:
    if (px_err_1243_proped) return px_err_1243_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func_named");
    LXValue _v1244 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1245 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v1257 = px_null();
    LXValue _v1258 = px_null();
    LXValue _v1259 = px_null();
    LXValue _v1260 = px_null();
    LXValue _v1261 = px_null();
    LXValue _v1262 = px_null();
    LXValue _v1263 = px_null();
    LXValue _v1264 = px_null();
    LXValue _v1265 = px_null();
    LXValue _v1266 = px_null();
    LXValue _v1267 = px_null();
    LXValue _v1268 = px_null();
    LXValue px_err_1269_val = px_null();
    int px_err_1269_proped = 0;
    px_srcline(535);
    _v1246 = px_add(px_add(px_str("static LXValue "), _v1245), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(536);
     _v1246 = px_add(_v1246, px_str("    (void)ctx;\n"));
    px_srcline(539);
     _v1246 = px_add(_v1246, px_add(px_add(px_str("    px_srcfunc(\""), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1244, px_int(1LL))}, 1)), px_str("\");\n")));
    px_srcline(540);
    _v1247 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(541);
    _v1248 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(542);
    _v1249 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(543);
    _v1250 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(544);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(545);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(546);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(547);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1250}, 1));
    px_srcline(551);
    _v1251 = px_list_n((LXValue[]){}, 0);
    px_srcline(552);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){px_index(_v1244, px_int(4LL)), _v1251}, 2));
    px_srcline(554);
    _v1252 = px_index(_v1244, px_int(2LL));
    px_srcline(555);
    _v1253 = px_int(0LL);
    px_srcline(556);
    _v1254 = px_list_n((LXValue[]){}, 0);
    px_srcline(557);
    while (px_is_truthy(px_lt(_v1253, px_call(px_get_global("len"), (LXValue[]){_v1252}, 1)))) {
        px_srcline(558);
        _v1255 = px_index(_v1252, _v1253);
        px_srcline(559);
        _v1256 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1255, px_int(1LL))}, 1);
        px_srcline(560);
        _v1257 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1256}, 1);
        px_srcline(561);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1251, _v1256}, 2))) {
            px_srcline(562);
            px_index_set(px_get_global("cg_cells"), _v1256, px_int(1LL));
        }
        px_srcline(563);
        _v1258 = px_str("px_null()");
        px_srcline(564);
        if (px_is_truthy(px_ne(px_index(_v1255, px_int(3LL)), px_null()))) {
            px_srcline(565);
             _v1258 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1255, px_int(3LL))}, 1);
        }
        px_srcline(566);
        (void)(px_method(_v1254, "append", (LXValue[]){px_list_n((LXValue[]){_v1257, _v1256, px_add(px_add(px_add(px_add(px_add(px_str("(nargs > "), px_call(px_get_global("str"), (LXValue[]){_v1253}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1253}, 1)), px_str("] : ")), _v1258)}, 3)}, 1));
        px_srcline(567);
         _v1253 = px_add(_v1253, px_int(1LL));
    }
    px_srcline(570);
    _v1259 = px_list_n((LXValue[]){}, 0);
    px_srcline(571);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1244, px_int(4LL)), _v1259}, 2));
    px_srcline(572);
    _v1260 = px_list_n((LXValue[]){}, 0);
    px_srcline(573);
    _v1261 = px_int(0LL);
    px_srcline(574);
    while (px_is_truthy(px_lt(_v1261, px_call(px_get_global("len"), (LXValue[]){_v1259}, 1)))) {
        px_srcline(575);
        _v1262 = px_index(_v1259, _v1261);
        px_srcline(576);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1262}, 1), px_null()))) {
            px_srcline(577);
             _v1261 = px_add(_v1261, px_int(1LL));
            px_srcline(578);
            continue;
        }
        px_srcline(579);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1262}, 2))) {
            px_srcline(580);
             _v1261 = px_add(_v1261, px_int(1LL));
            px_srcline(581);
            continue;
        }
        px_srcline(582);
        _v1257 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1262}, 1);
        px_srcline(583);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1251, _v1262}, 2))) {
            px_srcline(584);
            px_index_set(px_get_global("cg_cells"), _v1262, px_int(1LL));
        }
        px_srcline(585);
        (void)(px_method(_v1260, "append", (LXValue[]){px_list_n((LXValue[]){_v1257, _v1262}, 2)}, 1));
        px_srcline(586);
         _v1261 = px_add(_v1261, px_int(1LL));
    }
    px_srcline(588);
    _v1263 = px_int(0LL);
    px_srcline(589);
    while (px_is_truthy(px_lt(_v1263, px_call(px_get_global("len"), (LXValue[]){_v1254}, 1)))) {
        px_srcline(590);
        _v1264 = px_index(_v1254, _v1263);
        px_srcline(591);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1264, px_int(1LL))}, 1))) {
            px_srcline(592);
             _v1246 = px_add(_v1246, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1264, px_int(0LL))), px_str(" = px_cell(")), px_index(_v1264, px_int(2LL))), px_str(");\n")));
        }
        else {
            px_srcline(594);
             _v1246 = px_add(_v1246, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1264, px_int(0LL))), px_str(" = ")), px_index(_v1264, px_int(2LL))), px_str(";\n")));
        }
        px_srcline(595);
         _v1263 = px_add(_v1263, px_int(1LL));
    }
    px_srcline(596);
     _v1263 = px_int(0LL);
    px_srcline(597);
    while (px_is_truthy(px_lt(_v1263, px_call(px_get_global("len"), (LXValue[]){_v1260}, 1)))) {
        px_srcline(598);
        _v1265 = px_index(_v1260, _v1263);
        px_srcline(599);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1265, px_int(1LL))}, 1))) {
            px_srcline(600);
             _v1246 = px_add(_v1246, px_add(px_add(px_str("    LXValue "), px_index(_v1265, px_int(0LL))), px_str(" = px_cell(px_null());\n")));
        }
        else {
            px_srcline(602);
             _v1246 = px_add(_v1246, px_add(px_add(px_str("    LXValue "), px_index(_v1265, px_int(0LL))), px_str(" = px_null();\n")));
        }
        px_srcline(603);
         _v1263 = px_add(_v1263, px_int(1LL));
    }
    px_srcline(605);
    _v1266 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(606);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1266}, 1));
    px_srcline(607);
     _v1246 = px_add(_v1246, px_add(px_add(px_str("    LXValue "), _v1266), px_str("_val = px_null();\n")));
    px_srcline(608);
     _v1246 = px_add(_v1246, px_add(px_add(px_str("    int "), _v1266), px_str("_proped = 0;\n")));
    px_srcline(610);
    _v1267 = px_index(_v1244, px_int(4LL));
    px_srcline(611);
    _v1268 = px_int(0LL);
    px_srcline(612);
    while (px_is_truthy(px_lt(_v1268, px_call(px_get_global("len"), (LXValue[]){_v1267}, 1)))) {
        px_srcline(613);
         _v1246 = px_add(_v1246, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1267, _v1268), px_int(1LL)}, 2));
        px_srcline(614);
         _v1268 = px_add(_v1268, px_int(1LL));
    }
    px_srcline(615);
     _v1246 = px_add(_v1246, px_add(_v1266, px_str(":\n")));
    px_srcline(616);
     _v1246 = px_add(_v1246, px_add(px_add(px_add(px_add(px_str("    if ("), _v1266), px_str("_proped) return ")), _v1266), px_str("_val;\n")));
    px_srcline(617);
     _v1246 = px_add(_v1246, px_str("    return px_null();\n"));
    px_srcline(618);
     _v1246 = px_add(_v1246, px_str("}\n"));
    px_srcline(619);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(620);
    px_set_global("cg_vars", _v1247);
    px_srcline(621);
    px_set_global("cg_var_types", _v1248);
    px_srcline(622);
    px_set_global("cg_cells", _v1249);
    px_srcline(623);
    px_set_global("cg_immutables", _v1250);
    px_srcline(624);
    return _v1246;
px_err_1269:
    if (px_err_1269_proped) return px_err_1269_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_generate");
    LXValue _v1270 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1271 = px_null();
    LXValue _v1272 = px_null();
    LXValue _v1273 = px_null();
    LXValue _v1274 = px_null();
    LXValue _v1275 = px_null();
    LXValue _v1276 = px_null();
    LXValue _v1277 = px_null();
    LXValue _v1278 = px_null();
    LXValue _v1279 = px_null();
    LXValue _v1280 = px_null();
    LXValue _v1281 = px_null();
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
    LXValue _v1293 = px_null();
    LXValue _v1294 = px_null();
    LXValue _v1295 = px_null();
    LXValue _v1296 = px_null();
    LXValue _v1297 = px_null();
    LXValue _v1298 = px_null();
    LXValue _v1299 = px_null();
    LXValue _v1300 = px_null();
    LXValue _v1301 = px_null();
    LXValue _v1302 = px_null();
    LXValue _v1303 = px_null();
    LXValue px_err_1304_val = px_null();
    int px_err_1304_proped = 0;
    px_srcline(627);
    _v1271 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_srcline(628);
    px_set_global("cg_closures", px_str(""));
    px_srcline(629);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(630);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(631);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(632);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(633);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(634);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(635);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(636);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(637);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(638);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(639);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(640);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(641);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(642);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(643);
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1270}, 1));
    px_srcline(645);
    _v1272 = px_index(_v1270, px_int(1LL));
    px_srcline(646);
    _v1273 = px_int(0LL);
    px_srcline(647);
    while (px_is_truthy(px_lt(_v1273, px_call(px_get_global("len"), (LXValue[]){_v1272}, 1)))) {
        px_srcline(648);
        _v1274 = px_index(_v1272, _v1273);
        px_srcline(649);
        _v1275 = px_index(_v1274, px_int(0LL));
        px_srcline(650);
        if (px_is_truthy(px_eq(_v1275, px_str("FuncDef")))) {
            px_srcline(651);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1274, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1275, px_str("ExternDef")))) {
            px_srcline(654);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1274, px_int(1LL))}, 1), px_index(_v1274, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1275, px_str("VarDecl")))) {
            px_srcline(656);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1274, px_int(2LL))}, 1)}, 1));
            px_srcline(657);
            if (px_is_truthy(({ LXValue _t1305 = px_eq(px_index(_v1274, px_int(1LL)), px_str("Let")); px_is_truthy(_t1305) ? _t1305 : px_eq(px_index(_v1274, px_int(1LL)), px_str("Const")); }))) {
                px_srcline(658);
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1274, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1275, px_str("Assign")))) {
            px_srcline(660);
            _v1276 = px_index(_v1274, px_int(1LL));
            px_srcline(661);
            if (px_is_truthy(px_eq(px_index(_v1276, px_int(0LL)), px_str("Var")))) {
                px_srcline(662);
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1276, px_int(1LL))}, 1)}, 1));
            }
        }
        px_srcline(663);
         _v1273 = px_add(_v1273, px_int(1LL));
    }
    px_srcline(665);
    _v1277 = px_list_n((LXValue[]){}, 0);
    px_srcline(666);
    _v1278 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    px_srcline(667);
    _v1279 = px_int(0LL);
    px_srcline(668);
    while (px_is_truthy(px_lt(_v1279, px_call(px_get_global("len"), (LXValue[]){_v1278}, 1)))) {
        px_srcline(669);
        _v1280 = px_index(_v1278, _v1279);
        px_srcline(670);
        _v1281 = px_index(px_get_global("cg_impls"), _v1280);
        px_srcline(671);
        _v1282 = px_int(0LL);
        px_srcline(672);
        while (px_is_truthy(px_lt(_v1282, px_call(px_get_global("len"), (LXValue[]){_v1281}, 1)))) {
            px_srcline(673);
            (void)(px_method(_v1277, "append", (LXValue[]){px_list_n((LXValue[]){_v1280, px_index(_v1281, _v1282)}, 2)}, 1));
            px_srcline(674);
             _v1282 = px_add(_v1282, px_int(1LL));
        }
        px_srcline(675);
         _v1279 = px_add(_v1279, px_int(1LL));
    }
    px_srcline(677);
    _v1283 = px_int(1LL);
    px_srcline(678);
    while (px_is_truthy(px_lt(_v1283, px_call(px_get_global("len"), (LXValue[]){_v1277}, 1)))) {
        px_srcline(679);
        _v1284 = _v1283;
        px_srcline(680);
        while (px_is_truthy(px_gt(_v1284, px_int(0LL)))) {
            px_srcline(681);
            _v1285 = px_add(px_add(px_index(px_index(_v1277, px_sub(_v1284, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1277, px_sub(_v1284, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(682);
            _v1286 = px_add(px_add(px_index(px_index(_v1277, _v1284), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1277, _v1284), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(683);
            if (px_is_truthy(px_lt(_v1286, _v1285))) {
                px_srcline(684);
                _v1287 = px_index(_v1277, _v1284);
                px_srcline(685);
                px_index_set(_v1277, _v1284, px_index(_v1277, px_sub(_v1284, px_int(1LL))));
                px_srcline(686);
                px_index_set(_v1277, px_sub(_v1284, px_int(1LL)), _v1287);
            }
            px_srcline(687);
             _v1284 = px_sub(_v1284, px_int(1LL));
        }
        px_srcline(688);
         _v1283 = px_add(_v1283, px_int(1LL));
    }
    px_srcline(690);
    _v1288 = px_int(0LL);
    px_srcline(691);
    while (px_is_truthy(px_lt(_v1288, px_call(px_get_global("len"), (LXValue[]){_v1277}, 1)))) {
        px_srcline(692);
        _v1280 = px_index(px_index(_v1277, _v1288), px_int(0LL));
        px_srcline(693);
        _v1289 = px_index(px_index(_v1277, _v1288), px_int(1LL));
        px_srcline(694);
        _v1290 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1280}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1289, px_int(1LL))}, 1)}, 1));
        px_srcline(695);
         _v1271 = px_add(_v1271, px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1289, _v1290}, 2));
        px_srcline(696);
         _v1271 = px_add(_v1271, px_str("\n"));
        px_srcline(697);
         _v1288 = px_add(_v1288, px_int(1LL));
    }
    px_srcline(699);
    _v1291 = px_int(0LL);
    px_srcline(700);
    while (px_is_truthy(px_lt(_v1291, px_call(px_get_global("len"), (LXValue[]){_v1272}, 1)))) {
        px_srcline(701);
        _v1274 = px_index(_v1272, _v1291);
        px_srcline(702);
        if (px_is_truthy(px_eq(px_index(_v1274, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(703);
             _v1271 = px_add(_v1271, px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1274}, 1));
            px_srcline(704);
             _v1271 = px_add(_v1271, px_str("\n"));
        }
        px_srcline(705);
         _v1291 = px_add(_v1291, px_int(1LL));
    }
    px_srcline(707);
     _v1271 = px_add(_v1271, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(708);
     _v1271 = px_add(_v1271, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(709);
     _v1271 = px_add(_v1271, px_str("    px_register_builtins();\n"));
    px_srcline(711);
    _v1292 = px_int(0LL);
    px_srcline(712);
    while (px_is_truthy(px_lt(_v1292, px_call(px_get_global("len"), (LXValue[]){_v1272}, 1)))) {
        px_srcline(713);
        _v1274 = px_index(_v1272, _v1292);
        px_srcline(714);
        if (px_is_truthy(px_eq(px_index(_v1274, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(715);
            _v1290 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1274, px_int(1LL))}, 1);
            px_srcline(716);
            _v1293 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1290}, 1));
            px_srcline(717);
             _v1271 = px_add(_v1271, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1290), px_str("\", px_func(\"")), _v1290), px_str("\", ")), _v1293), px_str(", NULL));\n")));
        }
        px_srcline(718);
         _v1292 = px_add(_v1292, px_int(1LL));
    }
    px_srcline(720);
    _v1294 = px_int(0LL);
    px_srcline(721);
    while (px_is_truthy(px_lt(_v1294, px_call(px_get_global("len"), (LXValue[]){_v1277}, 1)))) {
        px_srcline(722);
        _v1280 = px_index(px_index(_v1277, _v1294), px_int(0LL));
        px_srcline(723);
        _v1289 = px_index(px_index(_v1277, _v1294), px_int(1LL));
        px_srcline(724);
        _v1295 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1289, px_int(1LL))}, 1);
        px_srcline(725);
        _v1290 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1280}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1295}, 1));
        px_srcline(726);
         _v1271 = px_add(_v1271, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1280), px_str(".")), _v1295), px_str("\", px_func(\"")), _v1280), px_str(".")), _v1295), px_str("\", ")), _v1290), px_str(", NULL));\n")));
        px_srcline(727);
         _v1294 = px_add(_v1294, px_int(1LL));
    }
    px_srcline(729);
    _v1296 = px_int(0LL);
    px_srcline(730);
    while (px_is_truthy(px_lt(_v1296, px_call(px_get_global("len"), (LXValue[]){_v1272}, 1)))) {
        px_srcline(731);
        _v1274 = px_index(_v1272, _v1296);
        px_srcline(732);
        _v1275 = px_index(_v1274, px_int(0LL));
        px_srcline(733);
        if (px_is_truthy(({ LXValue _t1311 = ({ LXValue _t1310 = ({ LXValue _t1309 = ({ LXValue _t1308 = ({ LXValue _t1307 = ({ LXValue _t1306 = px_ne(_v1275, px_str("FuncDef")); px_is_truthy(_t1306) ? px_ne(_v1275, px_str("StructDef")) : _t1306; }); px_is_truthy(_t1307) ? px_ne(_v1275, px_str("EnumDef")) : _t1307; }); px_is_truthy(_t1308) ? px_ne(_v1275, px_str("TraitDef")) : _t1308; }); px_is_truthy(_t1309) ? px_ne(_v1275, px_str("ImplDef")) : _t1309; }); px_is_truthy(_t1310) ? px_ne(_v1275, px_str("Import")) : _t1310; }); px_is_truthy(_t1311) ? px_ne(_v1275, px_str("ExternDef")) : _t1311; }))) {
            px_srcline(734);
             _v1271 = px_add(_v1271, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1274, px_int(1LL)}, 2));
        }
        px_srcline(735);
         _v1296 = px_add(_v1296, px_int(1LL));
    }
    px_srcline(737);
    _v1297 = px_bool(false);
    px_srcline(738);
    _v1298 = px_int(0LL);
    px_srcline(739);
    while (px_is_truthy(px_lt(_v1298, px_call(px_get_global("len"), (LXValue[]){_v1272}, 1)))) {
        px_srcline(740);
        _v1274 = px_index(_v1272, _v1298);
        px_srcline(741);
        if (px_is_truthy(({ LXValue _t1312 = px_eq(px_index(_v1274, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1312) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1274, px_int(1LL))}, 1), px_str("main")) : _t1312; }))) {
            px_srcline(742);
             _v1297 = px_bool(true);
            px_srcline(743);
            break;
        }
        px_srcline(744);
         _v1298 = px_add(_v1298, px_int(1LL));
    }
    px_srcline(745);
    if (px_is_truthy(_v1297)) {
        px_srcline(746);
        _v1293 = px_str("fn_main");
        px_srcline(747);
         _v1271 = px_add(_v1271, px_add(px_add(px_str("    { LXValue _r = "), _v1293), px_str("(NULL, 0, NULL); int _code = 0;\n")));
        px_srcline(748);
         _v1271 = px_add(_v1271, px_str("      if (px_is_result(_r)) {\n"));
        px_srcline(749);
         _v1271 = px_add(_v1271, px_str("        if (!px_result_ok(_r)) {\n"));
        px_srcline(750);
         _v1271 = px_add(_v1271, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
        px_srcline(751);
         _v1271 = px_add(_v1271, px_str("          _code = 1;\n"));
        px_srcline(752);
         _v1271 = px_add(_v1271, px_str("        } else {\n"));
        px_srcline(753);
         _v1271 = px_add(_v1271, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
        px_srcline(754);
         _v1271 = px_add(_v1271, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
        px_srcline(755);
         _v1271 = px_add(_v1271, px_str("        }\n"));
        px_srcline(756);
         _v1271 = px_add(_v1271, px_str("      } else if (_r.type == PX_INT) {\n"));
        px_srcline(757);
         _v1271 = px_add(_v1271, px_str("        _code = (int)_r.as.i;\n"));
        px_srcline(758);
         _v1271 = px_add(_v1271, px_str("      }\n"));
        px_srcline(759);
         _v1271 = px_add(_v1271, px_str("      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）\n"));
        px_srcline(760);
         _v1271 = px_add(_v1271, px_str("    }\n"));
    }
    else {
        px_srcline(762);
         _v1271 = px_add(_v1271, px_str("    return px_exit_code_final(0);   // M120（qg-issue 76 E1）\n"));
    }
    px_srcline(763);
     _v1271 = px_add(_v1271, px_str("}\n"));
    px_srcline(765);
    _v1299 = px_call(px_get_global("cg_find"), (LXValue[]){_v1271, px_str("int main(")}, 2);
    px_srcline(766);
    if (px_is_truthy(px_ge(_v1299, px_int(0LL)))) {
        px_srcline(767);
        _v1300 = px_slice(_v1271, px_int(0LL), _v1299, px_null());
        px_srcline(768);
        _v1301 = px_slice(_v1271, _v1299, px_call(px_get_global("len"), (LXValue[]){_v1271}, 1), px_null());
        px_srcline(769);
        _v1302 = px_call(px_get_global("cg_find"), (LXValue[]){_v1300, px_str("static LXValue")}, 2);
        px_srcline(770);
        _v1303 = px_str("");
        px_srcline(771);
        if (px_is_truthy(px_ge(_v1302, px_int(0LL)))) {
            px_srcline(772);
             _v1303 = px_add(px_add(px_add(px_add(px_slice(_v1300, px_int(0LL), _v1302, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1300, _v1302, px_call(px_get_global("len"), (LXValue[]){_v1300}, 1), px_null())), _v1301);
        }
        else {
            px_srcline(774);
             _v1303 = px_add(px_add(px_add(_v1300, px_get_global("cg_closures")), px_str("\n")), _v1301);
        }
        px_srcline(775);
        return _v1303;
    }
    px_srcline(776);
    return _v1271;
px_err_1304:
    if (px_err_1304_proped) return px_err_1304_val;
    return px_null();
}

static LXValue fn_bc_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_dict");
    LXValue _v1313 = px_null();
    LXValue px_err_1314_val = px_null();
    int px_err_1314_proped = 0;
    px_srcline(37);
    _v1313 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(38);
    (void)(px_method(_v1313, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(39);
    return _v1313;
px_err_1314:
    if (px_err_1314_proped) return px_err_1314_val;
    return px_null();
}

static LXValue fn_bc_k_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_find");
    LXValue _v1315 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1316 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1317 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1318 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1319 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1320 = px_null();
    LXValue _v1321 = px_null();
    LXValue px_err_1322_val = px_null();
    int px_err_1322_proped = 0;
    px_srcline(43);
    _v1320 = px_int(0LL);
    px_srcline(44);
    while (px_is_truthy(px_lt(_v1320, px_call(px_get_global("len"), (LXValue[]){_v1315}, 1)))) {
        px_srcline(45);
        _v1321 = px_index(_v1315, _v1320);
        px_srcline(46);
        if (px_is_truthy(px_eq(px_index(_v1321, px_str("kind")), _v1316))) {
            px_srcline(47);
            if (px_is_truthy(px_eq(_v1316, px_str("int")))) {
                px_srcline(48);
                if (px_is_truthy(px_eq(px_index(_v1321, px_str("i")), _v1317))) {
                    px_srcline(49);
                    return _v1320;
                }
            }
            else if (px_is_truthy(px_eq(_v1316, px_str("float")))) {
                px_srcline(51);
                if (px_is_truthy(px_eq(px_index(_v1321, px_str("f")), _v1318))) {
                    px_srcline(52);
                    return _v1320;
                }
            }
            else if (px_is_truthy(px_eq(_v1316, px_str("str")))) {
                px_srcline(54);
                if (px_is_truthy(px_eq(px_index(_v1321, px_str("s")), _v1319))) {
                    px_srcline(55);
                    return _v1320;
                }
            }
            else {
                px_srcline(58);
                if (px_is_truthy(px_eq(px_index(_v1321, px_str("i")), _v1317))) {
                    px_srcline(59);
                    return _v1320;
                }
            }
        }
        px_srcline(60);
         _v1320 = px_add(_v1320, px_int(1LL));
    }
    px_srcline(61);
    return px_neg(px_int(1LL));
px_err_1322:
    if (px_err_1322_proped) return px_err_1322_val;
    return px_null();
}

static LXValue fn_bc_k_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_add");
    LXValue _v1323 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1324 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1325 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1326 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1327 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1328 = px_null();
    LXValue _v1329 = px_null();
    LXValue px_err_1330_val = px_null();
    int px_err_1330_proped = 0;
    px_srcline(63);
    _v1328 = px_call(px_get_global("bc_k_find"), (LXValue[]){_v1323, _v1324, _v1325, _v1326, _v1327}, 5);
    px_srcline(64);
    if (px_is_truthy(px_ge(_v1328, px_int(0LL)))) {
        px_srcline(65);
        return _v1328;
    }
    px_srcline(66);
    _v1329 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(67);
    px_index_set(_v1329, px_str("kind"), _v1324);
    px_srcline(68);
    px_index_set(_v1329, px_str("i"), _v1325);
    px_srcline(69);
    px_index_set(_v1329, px_str("f"), _v1326);
    px_srcline(70);
    px_index_set(_v1329, px_str("s"), _v1327);
    px_srcline(71);
    (void)(px_method(_v1323, "push", (LXValue[]){_v1329}, 1));
    px_srcline(72);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1323}, 1), px_int(1LL));
px_err_1330:
    if (px_err_1330_proped) return px_err_1330_val;
    return px_null();
}

static LXValue fn_bc_n_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_n_add");
    LXValue _v1331 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1332 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1333 = px_null();
    LXValue px_err_1334_val = px_null();
    int px_err_1334_proped = 0;
    px_srcline(75);
    _v1333 = px_int(0LL);
    px_srcline(76);
    while (px_is_truthy(px_lt(_v1333, px_call(px_get_global("len"), (LXValue[]){_v1331}, 1)))) {
        px_srcline(77);
        if (px_is_truthy(px_eq(px_index(_v1331, _v1333), _v1332))) {
            px_srcline(78);
            return _v1333;
        }
        px_srcline(79);
         _v1333 = px_add(_v1333, px_int(1LL));
    }
    px_srcline(80);
    (void)(px_method(_v1331, "push", (LXValue[]){_v1332}, 1));
    px_srcline(81);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1331}, 1), px_int(1LL));
px_err_1334:
    if (px_err_1334_proped) return px_err_1334_val;
    return px_null();
}

static LXValue fn_bc_g_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_g_add");
    LXValue _v1335 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1336 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1337 = px_null();
    LXValue px_err_1338_val = px_null();
    int px_err_1338_proped = 0;
    px_srcline(84);
    _v1337 = px_int(0LL);
    px_srcline(85);
    while (px_is_truthy(px_lt(_v1337, px_call(px_get_global("len"), (LXValue[]){_v1335}, 1)))) {
        px_srcline(86);
        if (px_is_truthy(px_eq(px_index(_v1335, _v1337), _v1336))) {
            px_srcline(87);
            return _v1337;
        }
        px_srcline(88);
         _v1337 = px_add(_v1337, px_int(1LL));
    }
    px_srcline(89);
    (void)(px_method(_v1335, "push", (LXValue[]){_v1336}, 1));
    px_srcline(90);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1335}, 1), px_int(1LL));
px_err_1338:
    if (px_err_1338_proped) return px_err_1338_val;
    return px_null();
}

static LXValue fn_bc_is_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_is_global");
    LXValue _v1339 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1340_val = px_null();
    int px_err_1340_proped = 0;
    px_srcline(93);
    if (px_is_truthy(px_eq(px_get_global("g_bcm"), px_null()))) {
        px_srcline(94);
        return px_bool(false);
    }
    px_srcline(95);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1339}, 2);
px_err_1340:
    if (px_err_1340_proped) return px_err_1340_val;
    return px_null();
}

static LXValue fn_bc_new_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_module");
    LXValue _v1341 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1342 = px_null();
    LXValue px_err_1343_val = px_null();
    int px_err_1343_proped = 0;
    px_srcline(98);
    _v1342 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(99);
    px_index_set(_v1342, px_str("name"), _v1341);
    px_srcline(100);
    px_index_set(_v1342, px_str("k_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(101);
    px_index_set(_v1342, px_str("n_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(102);
    px_index_set(_v1342, px_str("globals"), px_list_n((LXValue[]){}, 0));
    px_srcline(103);
    px_index_set(_v1342, px_str("funcs"), px_list_n((LXValue[]){}, 0));
    px_srcline(104);
    px_index_set(_v1342, px_str("top"), px_neg(px_int(1LL)));
    px_srcline(107);
    px_index_set(_v1342, px_str("structs"), px_list_n((LXValue[]){}, 0));
    px_srcline(108);
    px_index_set(_v1342, px_str("enums"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(109);
    px_index_set(_v1342, px_str("consts"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(110);
    px_index_set(_v1342, px_str("nclosure"), px_int(0LL));
    px_srcline(111);
    return _v1342;
px_err_1343:
    if (px_err_1343_proped) return px_err_1343_val;
    return px_null();
}

static LXValue fn_bc_struct_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_struct_index");
    LXValue _v1344 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1345 = px_null();
    LXValue px_err_1346_val = px_null();
    int px_err_1346_proped = 0;
    px_srcline(114);
    _v1345 = px_int(0LL);
    px_srcline(115);
    while (px_is_truthy(px_lt(_v1345, px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("structs"))}, 1)))) {
        px_srcline(116);
        if (px_is_truthy(px_eq(px_index(px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1345), px_str("name")), _v1344))) {
            px_srcline(117);
            return _v1345;
        }
        px_srcline(118);
         _v1345 = px_add(_v1345, px_int(1LL));
    }
    px_srcline(119);
    return px_neg(px_int(1LL));
px_err_1346:
    if (px_err_1346_proped) return px_err_1346_val;
    return px_null();
}

static LXValue fn_bc_enum_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_enum_has");
    LXValue _v1347 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1348 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1349_val = px_null();
    int px_err_1349_proped = 0;
    px_srcline(122);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1347}, 1)))) {
        px_srcline(123);
        return px_bool(false);
    }
    px_srcline(124);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_index(px_get_global("g_bcm"), px_str("enums")), _v1347), _v1348}, 2);
px_err_1349:
    if (px_err_1349_proped) return px_err_1349_val;
    return px_null();
}

static LXValue fn_bc_const_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_const_find");
    LXValue _v1350 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1351 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1352 = px_null();
    LXValue _v1353 = px_null();
    LXValue px_err_1354_val = px_null();
    int px_err_1354_proped = 0;
    px_srcline(127);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1350}, 1)))) {
        px_srcline(128);
        return px_null();
    }
    px_srcline(129);
    _v1352 = px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1350);
    px_srcline(130);
    _v1353 = px_int(0LL);
    px_srcline(131);
    while (px_is_truthy(px_lt(_v1353, px_call(px_get_global("len"), (LXValue[]){_v1352}, 1)))) {
        px_srcline(132);
        if (px_is_truthy(px_eq(px_index(px_index(_v1352, _v1353), px_str("n")), _v1351))) {
            px_srcline(133);
            return px_index(px_index(_v1352, _v1353), px_str("e"));
        }
        px_srcline(134);
         _v1353 = px_add(_v1353, px_int(1LL));
    }
    px_srcline(135);
    return px_null();
px_err_1354:
    if (px_err_1354_proped) return px_err_1354_val;
    return px_null();
}

static LXValue fn_bc_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_consts");
    LXValue _v1355 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1356 = px_null();
    LXValue _v1357 = px_null();
    LXValue _v1358 = px_null();
    LXValue _v1359 = px_null();
    LXValue _v1360 = px_null();
    LXValue _v1361 = px_null();
    LXValue _v1362 = px_null();
    LXValue _v1363 = px_null();
    LXValue _v1364 = px_null();
    LXValue _v1365 = px_null();
    LXValue px_err_1366_val = px_null();
    int px_err_1366_proped = 0;
    px_srcline(138);
    _v1356 = px_int(0LL);
    px_srcline(139);
    while (px_is_truthy(px_lt(_v1356, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        px_srcline(140);
        _v1357 = px_index(_v1355, _v1356);
        px_srcline(141);
        _v1358 = px_index(_v1357, px_int(0LL));
        px_srcline(142);
        if (px_is_truthy(px_eq(_v1358, px_str("TypeConst")))) {
            px_srcline(143);
            _v1359 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1357, px_int(1LL))}, 1);
            px_srcline(144);
            if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1359}, 1)))) {
                px_srcline(145);
                px_index_set(px_index(px_get_global("g_bcm"), px_str("consts")), _v1359, px_list_n((LXValue[]){}, 0));
            }
            px_srcline(146);
            _v1360 = px_int(0LL);
            px_srcline(147);
            while (px_is_truthy(px_lt(_v1360, px_call(px_get_global("len"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1)))) {
                px_srcline(148);
                _v1361 = px_index(px_index(_v1357, px_int(2LL)), _v1360);
                px_srcline(149);
                _v1362 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
                px_srcline(150);
                px_index_set(_v1362, px_str("n"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1361, px_int(1LL))}, 1));
                px_srcline(151);
                px_index_set(_v1362, px_str("e"), px_index(_v1361, px_int(2LL)));
                px_srcline(152);
                (void)(px_method(px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1359), "push", (LXValue[]){_v1362}, 1));
                px_srcline(153);
                 _v1360 = px_add(_v1360, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("FuncDef")))) {
            px_srcline(155);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1357, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("ImplDef")))) {
            px_srcline(157);
            _v1363 = px_int(0LL);
            px_srcline(158);
            while (px_is_truthy(px_lt(_v1363, px_call(px_get_global("len"), (LXValue[]){px_index(_v1357, px_int(3LL))}, 1)))) {
                px_srcline(159);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1357, px_int(3LL)), _v1363), px_int(4LL))}, 1));
                px_srcline(160);
                 _v1363 = px_add(_v1363, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("If")))) {
            px_srcline(162);
            _v1364 = px_index(_v1357, px_int(1LL));
            px_srcline(163);
            _v1365 = px_int(0LL);
            px_srcline(164);
            while (px_is_truthy(px_lt(_v1365, px_call(px_get_global("len"), (LXValue[]){_v1364}, 1)))) {
                px_srcline(165);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(_v1364, _v1365), px_int(1LL))}, 1));
                px_srcline(166);
                 _v1365 = px_add(_v1365, px_int(1LL));
            }
            px_srcline(167);
            if (px_is_truthy(px_ne(px_index(_v1357, px_int(2LL)), px_null()))) {
                px_srcline(168);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("While")))) {
            px_srcline(170);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("For")))) {
            px_srcline(172);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1357, px_int(3LL))}, 1));
        }
        px_srcline(173);
         _v1356 = px_add(_v1356, px_int(1LL));
    }
px_err_1366:
    if (px_err_1366_proped) return px_err_1366_val;
    return px_null();
}

static LXValue fn_bc_collect_impl_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_impl_list");
    LXValue _v1367 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1368 = px_null();
    LXValue _v1369 = px_null();
    LXValue _v1370 = px_null();
    LXValue _v1371 = px_null();
    LXValue _v1372 = px_null();
    LXValue _v1373 = px_null();
    LXValue _v1374 = px_null();
    LXValue _v1375 = px_null();
    LXValue _v1376 = px_null();
    LXValue _v1377 = px_null();
    LXValue px_err_1378_val = px_null();
    int px_err_1378_proped = 0;
    px_srcline(177);
    _v1368 = px_list_n((LXValue[]){}, 0);
    px_srcline(178);
    _v1369 = px_int(0LL);
    px_srcline(179);
    while (px_is_truthy(px_lt(_v1369, px_call(px_get_global("len"), (LXValue[]){_v1367}, 1)))) {
        px_srcline(180);
        _v1370 = px_index(_v1367, _v1369);
        px_srcline(181);
        if (px_is_truthy(px_eq(px_index(_v1370, px_int(0LL)), px_str("ImplDef")))) {
            px_srcline(182);
            _v1371 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1370, px_int(1LL))}, 1);
            px_srcline(183);
            _v1372 = px_int(0LL);
            px_srcline(184);
            while (px_is_truthy(px_lt(_v1372, px_call(px_get_global("len"), (LXValue[]){px_index(_v1370, px_int(3LL))}, 1)))) {
                px_srcline(185);
                (void)(px_method(_v1368, "append", (LXValue[]){px_list_n((LXValue[]){_v1371, px_index(px_index(_v1370, px_int(3LL)), _v1372)}, 2)}, 1));
                px_srcline(186);
                 _v1372 = px_add(_v1372, px_int(1LL));
            }
        }
        px_srcline(187);
         _v1369 = px_add(_v1369, px_int(1LL));
    }
    px_srcline(189);
    _v1373 = px_int(1LL);
    px_srcline(190);
    while (px_is_truthy(px_lt(_v1373, px_call(px_get_global("len"), (LXValue[]){_v1368}, 1)))) {
        px_srcline(191);
        _v1374 = _v1373;
        px_srcline(192);
        while (px_is_truthy(px_gt(_v1374, px_int(0LL)))) {
            px_srcline(193);
            _v1375 = px_add(px_add(px_index(px_index(_v1368, px_sub(_v1374, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1368, px_sub(_v1374, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(194);
            _v1376 = px_add(px_add(px_index(px_index(_v1368, _v1374), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1368, _v1374), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(195);
            if (px_is_truthy(px_lt(_v1376, _v1375))) {
                px_srcline(196);
                _v1377 = px_index(_v1368, _v1374);
                px_srcline(197);
                px_index_set(_v1368, _v1374, px_index(_v1368, px_sub(_v1374, px_int(1LL))));
                px_srcline(198);
                px_index_set(_v1368, px_sub(_v1374, px_int(1LL)), _v1377);
            }
            px_srcline(199);
             _v1374 = px_sub(_v1374, px_int(1LL));
        }
        px_srcline(200);
         _v1373 = px_add(_v1373, px_int(1LL));
    }
    px_srcline(201);
    return _v1368;
px_err_1378:
    if (px_err_1378_proped) return px_err_1378_val;
    return px_null();
}

static LXValue fn_bc_new_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_func");
    LXValue _v1379 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1380 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1381 = px_null();
    LXValue px_err_1382_val = px_null();
    int px_err_1382_proped = 0;
    px_srcline(206);
    _v1381 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(207);
    px_index_set(_v1381, px_str("name"), _v1379);
    px_srcline(208);
    px_index_set(_v1381, px_str("arity"), _v1380);
    px_srcline(209);
    px_index_set(_v1381, px_str("ndefault"), px_int(0LL));
    px_srcline(210);
    px_index_set(_v1381, px_str("nslots"), _v1380);
    px_srcline(211);
    px_index_set(_v1381, px_str("bc"), px_list_n((LXValue[]){}, 0));
    px_srcline(212);
    px_index_set(_v1381, px_str("smap"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(213);
    px_index_set(_v1381, px_str("next_slot"), _v1380);
    px_srcline(214);
    px_index_set(_v1381, px_str("is_top"), px_bool(false));
    px_srcline(215);
    px_index_set(_v1381, px_str("loops"), px_list_n((LXValue[]){}, 0));
    px_srcline(216);
    return _v1381;
px_err_1382:
    if (px_err_1382_proped) return px_err_1382_val;
    return px_null();
}

static LXValue fn_bc_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_slot");
    LXValue _v1383 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1384 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1385 = px_null();
    LXValue px_err_1386_val = px_null();
    int px_err_1386_proped = 0;
    px_srcline(219);
    if (px_is_truthy(px_method(px_index(_v1383, px_str("smap")), "has", (LXValue[]){_v1384}, 1))) {
        px_srcline(220);
        return px_index(px_index(_v1383, px_str("smap")), _v1384);
    }
    px_srcline(221);
    _v1385 = px_index(_v1383, px_str("next_slot"));
    px_srcline(222);
    px_index_set(_v1383, px_str("next_slot"), px_add(_v1385, px_int(1LL)));
    px_srcline(223);
    px_index_set(px_index(_v1383, px_str("smap")), _v1384, _v1385);
    px_srcline(224);
    return _v1385;
px_err_1386:
    if (px_err_1386_proped) return px_err_1386_val;
    return px_null();
}

static LXValue fn_bc_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_tmp");
    LXValue _v1387 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1388 = px_null();
    LXValue px_err_1389_val = px_null();
    int px_err_1389_proped = 0;
    px_srcline(227);
    _v1388 = px_index(_v1387, px_str("next_slot"));
    px_srcline(228);
    px_index_set(_v1387, px_str("next_slot"), px_add(_v1388, px_int(1LL)));
    px_srcline(229);
    return _v1388;
px_err_1389:
    if (px_err_1389_proped) return px_err_1389_val;
    return px_null();
}

static LXValue fn_bc_emit_inst(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_inst");
    LXValue _v1390 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1391 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1392 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1393 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1394 = (nargs > 4) ? args[4] : px_null();
    LXValue px_err_1395_val = px_null();
    int px_err_1395_proped = 0;
    px_srcline(235);
    (void)(px_method(px_index(_v1390, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){_v1391, px_int(0LL), _v1392, _v1393, _v1394}, 5)}, 1));
px_err_1395:
    if (px_err_1395_proped) return px_err_1395_val;
    return px_null();
}

static LXValue fn_bc_patch_off(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_patch_off");
    LXValue _v1396 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1397 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1398 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1399_val = px_null();
    int px_err_1399_proped = 0;
    px_srcline(239);
    px_index_set(px_index(px_index(_v1396, px_str("bc")), _v1397), px_int(3LL), px_sub(_v1398, px_add(_v1397, px_int(1LL))));
px_err_1399:
    if (px_err_1399_proped) return px_err_1399_val;
    return px_null();
}

static LXValue fn_bc_collect_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_hoist");
    LXValue _v1400 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1401 = (nargs > 1) ? args[1] : px_null();
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
    px_srcline(242);
    _v1402 = px_int(0LL);
    px_srcline(243);
    while (px_is_truthy(px_lt(_v1402, px_call(px_get_global("len"), (LXValue[]){_v1400}, 1)))) {
        px_srcline(244);
        _v1403 = px_index(_v1400, _v1402);
        px_srcline(245);
        _v1404 = px_index(_v1403, px_int(0LL));
        px_srcline(246);
        if (px_is_truthy(px_eq(_v1404, px_str("Assign")))) {
            px_srcline(247);
            _v1405 = px_index(_v1403, px_int(1LL));
            px_srcline(248);
            if (px_is_truthy(px_eq(px_index(_v1405, px_int(0LL)), px_str("Var")))) {
                px_srcline(249);
                _v1406 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1405, px_int(1LL))}, 1);
                px_srcline(250);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1401, _v1406}, 2)))) {
                    px_srcline(251);
                    (void)(px_method(_v1401, "append", (LXValue[]){_v1406}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1404, px_str("VarDecl")))) {
            px_srcline(253);
            _v1406 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1403, px_int(2LL))}, 1);
            px_srcline(254);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1401, _v1406}, 2)))) {
                px_srcline(255);
                (void)(px_method(_v1401, "append", (LXValue[]){_v1406}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1404, px_str("If")))) {
            px_srcline(257);
            _v1407 = px_index(_v1403, px_int(1LL));
            px_srcline(258);
            _v1408 = px_int(0LL);
            px_srcline(259);
            while (px_is_truthy(px_lt(_v1408, px_call(px_get_global("len"), (LXValue[]){_v1407}, 1)))) {
                px_srcline(260);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(px_index(_v1407, _v1408), px_int(1LL)), _v1401}, 2));
                px_srcline(261);
                 _v1408 = px_add(_v1408, px_int(1LL));
            }
            px_srcline(262);
            if (px_is_truthy(px_ne(px_index(_v1403, px_int(2LL)), px_null()))) {
                px_srcline(263);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1403, px_int(2LL)), _v1401}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1404, px_str("While")))) {
            px_srcline(265);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1403, px_int(2LL)), _v1401}, 2));
        }
        else if (px_is_truthy(px_eq(_v1404, px_str("For")))) {
            px_srcline(268);
            _v1409 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1403, px_int(1LL))}, 1);
            px_srcline(269);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1401, _v1409}, 2)))) {
                px_srcline(270);
                (void)(px_method(_v1401, "append", (LXValue[]){_v1409}, 1));
            }
            px_srcline(271);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1403, px_int(3LL)), _v1401}, 2));
        }
        else if (px_is_truthy(px_eq(_v1404, px_str("ChanDecl")))) {
            px_srcline(274);
            _v1410 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1403, px_int(1LL))}, 1);
            px_srcline(275);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1401, _v1410}, 2)))) {
                px_srcline(276);
                (void)(px_method(_v1401, "append", (LXValue[]){_v1410}, 1));
            }
        }
        px_srcline(277);
         _v1402 = px_add(_v1402, px_int(1LL));
    }
px_err_1411:
    if (px_err_1411_proped) return px_err_1411_val;
    return px_null();
}

static LXValue fn_bc_emit_null(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_null");
    LXValue _v1412 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1413 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1414 = px_null();
    LXValue px_err_1415_val = px_null();
    int px_err_1415_proped = 0;
    px_srcline(280);
    _v1414 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(281);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1412, px_str("LOADK"), _v1413, _v1414, px_int(0LL)}, 5));
px_err_1415:
    if (px_err_1415_proped) return px_err_1415_val;
    return px_null();
}

static LXValue fn_bc_emit_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_int");
    LXValue _v1416 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1417 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1418 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1419 = px_null();
    LXValue px_err_1420_val = px_null();
    int px_err_1420_proped = 0;
    px_srcline(284);
    if (px_is_truthy(({ LXValue _t1421 = px_ge(_v1418, px_neg(px_int(32768LL))); px_is_truthy(_t1421) ? px_le(_v1418, px_int(32767LL)) : _t1421; }))) {
        px_srcline(285);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1416, px_str("IMM"), _v1417, _v1418, px_int(0LL)}, 5));
    }
    else {
        px_srcline(287);
        _v1419 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("int"), _v1418, px_float(0), px_str("")}, 5);
        px_srcline(288);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1416, px_str("LOADK"), _v1417, _v1419, px_int(0LL)}, 5));
    }
px_err_1420:
    if (px_err_1420_proped) return px_err_1420_val;
    return px_null();
}

static LXValue fn_bc_emit_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_float");
    LXValue _v1422 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1423 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1424 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1425 = px_null();
    LXValue px_err_1426_val = px_null();
    int px_err_1426_proped = 0;
    px_srcline(291);
    _v1425 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("float"), px_int(0LL), _v1424, px_str("")}, 5);
    px_srcline(292);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1422, px_str("LOADK"), _v1423, _v1425, px_int(0LL)}, 5));
px_err_1426:
    if (px_err_1426_proped) return px_err_1426_val;
    return px_null();
}

static LXValue fn_bc_unop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_unop_op");
    LXValue _v1427 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1428_val = px_null();
    int px_err_1428_proped = 0;
    px_srcline(295);
    if (px_is_truthy(px_eq(_v1427, px_str("Neg")))) {
        px_srcline(296);
        return px_str("NEG");
    }
    px_srcline(297);
    if (px_is_truthy(px_eq(_v1427, px_str("Not")))) {
        px_srcline(298);
        return px_str("NOT");
    }
    px_srcline(299);
    if (px_is_truthy(px_eq(_v1427, px_str("BitNot")))) {
        px_srcline(300);
        return px_str("BITNOT");
    }
    px_srcline(301);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_unop_op 未知一元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1427}, 1))}, 1));
px_err_1428:
    if (px_err_1428_proped) return px_err_1428_val;
    return px_null();
}

static LXValue fn_bc_binop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_binop_op");
    LXValue _v1429 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1430_val = px_null();
    int px_err_1430_proped = 0;
    px_srcline(303);
    if (px_is_truthy(px_eq(_v1429, px_str("Add")))) {
        px_srcline(304);
        return px_str("ADD");
    }
    px_srcline(305);
    if (px_is_truthy(px_eq(_v1429, px_str("Sub")))) {
        px_srcline(306);
        return px_str("SUB");
    }
    px_srcline(307);
    if (px_is_truthy(px_eq(_v1429, px_str("Mul")))) {
        px_srcline(308);
        return px_str("MUL");
    }
    px_srcline(309);
    if (px_is_truthy(px_eq(_v1429, px_str("Div")))) {
        px_srcline(310);
        return px_str("DIV");
    }
    px_srcline(311);
    if (px_is_truthy(px_eq(_v1429, px_str("IntDiv")))) {
        px_srcline(312);
        return px_str("IDIV");
    }
    px_srcline(313);
    if (px_is_truthy(px_eq(_v1429, px_str("Mod")))) {
        px_srcline(314);
        return px_str("MOD");
    }
    px_srcline(315);
    if (px_is_truthy(px_eq(_v1429, px_str("Pow")))) {
        px_srcline(316);
        return px_str("POW");
    }
    px_srcline(317);
    if (px_is_truthy(px_eq(_v1429, px_str("Eq")))) {
        px_srcline(318);
        return px_str("EQ");
    }
    px_srcline(319);
    if (px_is_truthy(px_eq(_v1429, px_str("Ne")))) {
        px_srcline(320);
        return px_str("NE");
    }
    px_srcline(321);
    if (px_is_truthy(px_eq(_v1429, px_str("Lt")))) {
        px_srcline(322);
        return px_str("LT");
    }
    px_srcline(323);
    if (px_is_truthy(px_eq(_v1429, px_str("Le")))) {
        px_srcline(324);
        return px_str("LE");
    }
    px_srcline(325);
    if (px_is_truthy(px_eq(_v1429, px_str("Gt")))) {
        px_srcline(326);
        return px_str("GT");
    }
    px_srcline(327);
    if (px_is_truthy(px_eq(_v1429, px_str("Ge")))) {
        px_srcline(328);
        return px_str("GE");
    }
    px_srcline(329);
    if (px_is_truthy(px_eq(_v1429, px_str("BitAnd")))) {
        px_srcline(330);
        return px_str("BITAND");
    }
    px_srcline(331);
    if (px_is_truthy(px_eq(_v1429, px_str("BitOr")))) {
        px_srcline(332);
        return px_str("BITOR");
    }
    px_srcline(333);
    if (px_is_truthy(px_eq(_v1429, px_str("BitXor")))) {
        px_srcline(334);
        return px_str("BITXOR");
    }
    px_srcline(335);
    if (px_is_truthy(px_eq(_v1429, px_str("Shl")))) {
        px_srcline(336);
        return px_str("SHL");
    }
    px_srcline(337);
    if (px_is_truthy(px_eq(_v1429, px_str("Shr")))) {
        px_srcline(338);
        return px_str("SHR");
    }
    px_srcline(339);
    if (px_is_truthy(px_eq(_v1429, px_str("ShrU")))) {
        px_srcline(340);
        return px_str("SHRU");
    }
    px_srcline(341);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_binop_op 未知二元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1429}, 1))}, 1));
px_err_1430:
    if (px_err_1430_proped) return px_err_1430_val;
    return px_null();
}

static LXValue fn_bc_emit_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_expr");
    LXValue _v1431 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1432 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1433 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1434 = px_null();
    LXValue _v1435 = px_null();
    LXValue _v1436 = px_null();
    LXValue _v1437 = px_null();
    LXValue _v1438 = px_null();
    LXValue _v1439 = px_null();
    LXValue _v1440 = px_null();
    LXValue _v1441 = px_null();
    LXValue _v1442 = px_null();
    LXValue _v1443 = px_null();
    LXValue _v1444 = px_null();
    LXValue _v1445 = px_null();
    LXValue _v1446 = px_null();
    LXValue _v1447 = px_null();
    LXValue _v1448 = px_null();
    LXValue _v1449 = px_null();
    LXValue _v1450 = px_null();
    LXValue _v1451 = px_null();
    LXValue _v1452 = px_null();
    LXValue _v1453 = px_null();
    LXValue _v1454 = px_null();
    LXValue _v1455 = px_null();
    LXValue _v1456 = px_null();
    LXValue _v1457 = px_null();
    LXValue _v1458 = px_null();
    LXValue _v1459 = px_null();
    LXValue _v1460 = px_null();
    LXValue _v1461 = px_null();
    LXValue _v1462 = px_null();
    LXValue _v1463 = px_null();
    LXValue _v1464 = px_null();
    LXValue _v1465 = px_null();
    LXValue _v1466 = px_null();
    LXValue _v1467 = px_null();
    LXValue _v1468 = px_null();
    LXValue _v1469 = px_null();
    LXValue _v1470 = px_null();
    LXValue _v1471 = px_null();
    LXValue _v1472 = px_null();
    LXValue _v1473 = px_null();
    LXValue _v1474 = px_null();
    LXValue _v1475 = px_null();
    LXValue _v1476 = px_null();
    LXValue _v1477 = px_null();
    LXValue _v1478 = px_null();
    LXValue _v1479 = px_null();
    LXValue _v1480 = px_null();
    LXValue _v1481 = px_null();
    LXValue _v1482 = px_null();
    LXValue _v1483 = px_null();
    LXValue _v1484 = px_null();
    LXValue _v1485 = px_null();
    LXValue _v1486 = px_null();
    LXValue _v1487 = px_null();
    LXValue _v1488 = px_null();
    LXValue _v1489 = px_null();
    LXValue px_err_1490_val = px_null();
    int px_err_1490_proped = 0;
    px_srcline(346);
    _v1434 = px_index(_v1431, px_int(0LL));
    px_srcline(347);
    if (px_is_truthy(px_eq(_v1434, px_str("Int")))) {
        px_srcline(348);
        (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1433, _v1432, px_index(_v1431, px_int(1LL))}, 3));
        px_srcline(349);
        return _v1432;
    }
    px_srcline(350);
    if (px_is_truthy(px_eq(_v1434, px_str("Float")))) {
        px_srcline(351);
        (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1433, _v1432, px_index(_v1431, px_int(1LL))}, 3));
        px_srcline(352);
        return _v1432;
    }
    px_srcline(353);
    if (px_is_truthy(px_eq(_v1434, px_str("Unary")))) {
        px_srcline(356);
        _v1435 = px_index(_v1431, px_int(2LL));
        px_srcline(357);
        _v1436 = px_index(_v1431, px_int(1LL));
        px_srcline(358);
        if (px_is_truthy(({ LXValue _t1491 = px_eq(px_index(_v1435, px_int(0LL)), px_str("Int")); px_is_truthy(_t1491) ? px_eq(_v1436, px_str("Neg")) : _t1491; }))) {
            px_srcline(359);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1433, _v1432, px_sub(px_int(0LL), px_index(_v1435, px_int(1LL)))}, 3));
            px_srcline(360);
            return _v1432;
        }
        px_srcline(361);
        if (px_is_truthy(({ LXValue _t1492 = px_eq(px_index(_v1435, px_int(0LL)), px_str("Float")); px_is_truthy(_t1492) ? px_eq(_v1436, px_str("Neg")) : _t1492; }))) {
            px_srcline(362);
            (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1433, _v1432, px_sub(px_int(0LL), px_index(_v1435, px_int(1LL)))}, 3));
            px_srcline(363);
            return _v1432;
        }
        px_srcline(364);
        _v1437 = _v1432;
        px_srcline(365);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(366);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(367);
        _v1438 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(368);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1435, _v1438, _v1433}, 3));
        px_srcline(369);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_call(px_get_global("bc_unop_op"), (LXValue[]){_v1436}, 1), _v1437, _v1438, px_int(0LL)}, 5));
        px_srcline(370);
        return _v1437;
    }
    px_srcline(371);
    if (px_is_truthy(px_eq(_v1434, px_str("Str")))) {
        px_srcline(372);
        _v1439 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1431, px_int(1LL))}, 1)}, 5);
        px_srcline(373);
        if (px_is_truthy(px_ge(_v1432, px_int(0LL)))) {
            px_srcline(374);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("LOADK"), _v1432, _v1439, px_int(0LL)}, 5));
        }
        px_srcline(375);
        return _v1432;
    }
    px_srcline(376);
    if (px_is_truthy(px_eq(_v1434, px_str("Bool")))) {
        px_srcline(377);
        _v1440 = px_int(0LL);
        px_srcline(378);
        if (px_is_truthy(px_index(_v1431, px_int(1LL)))) {
            px_srcline(379);
             _v1440 = px_int(1LL);
        }
        px_srcline(380);
        _v1439 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("bool"), _v1440, px_float(0), px_str("")}, 5);
        px_srcline(381);
        if (px_is_truthy(px_ge(_v1432, px_int(0LL)))) {
            px_srcline(382);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("LOADK"), _v1432, _v1439, px_int(0LL)}, 5));
        }
        px_srcline(383);
        return _v1432;
    }
    px_srcline(384);
    if (px_is_truthy(px_eq(_v1434, px_str("Null")))) {
        px_srcline(385);
        if (px_is_truthy(px_ge(_v1432, px_int(0LL)))) {
            px_srcline(386);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1433, _v1432}, 2));
        }
        px_srcline(387);
        return _v1432;
    }
    px_srcline(388);
    if (px_is_truthy(px_eq(_v1434, px_str("Var")))) {
        px_srcline(389);
        _v1441 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1431, px_int(1LL))}, 1);
        px_srcline(390);
        if (px_is_truthy(px_method(px_index(_v1433, px_str("smap")), "has", (LXValue[]){_v1441}, 1))) {
            px_srcline(392);
            _v1438 = px_index(px_index(_v1433, px_str("smap")), _v1441);
            px_srcline(393);
            if (px_is_truthy(({ LXValue _t1493 = px_ge(_v1432, px_int(0LL)); px_is_truthy(_t1493) ? px_ne(_v1432, _v1438) : _t1493; }))) {
                px_srcline(394);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("MOV"), _v1432, _v1438, px_int(0LL)}, 5));
            }
            px_srcline(395);
            return _v1432;
        }
        px_srcline(397);
        _v1442 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1441}, 2);
        px_srcline(398);
        if (px_is_truthy(px_ge(_v1432, px_int(0LL)))) {
            px_srcline(399);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("GETG"), _v1432, _v1442, px_int(0LL)}, 5));
        }
        px_srcline(400);
        return _v1432;
    }
    px_srcline(401);
    if (px_is_truthy(px_eq(_v1434, px_str("Binary")))) {
        px_srcline(403);
        _v1436 = px_index(_v1431, px_int(1LL));
        px_srcline(404);
        _v1437 = _v1432;
        px_srcline(405);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(406);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(407);
        if (px_is_truthy(px_eq(_v1436, px_str("And")))) {
            px_srcline(409);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1437, _v1433}, 3));
            px_srcline(410);
            _v1443 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
            px_srcline(411);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPF"), _v1437, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(412);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(3LL)), _v1437, _v1433}, 3));
            px_srcline(413);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1443, px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1)}, 3));
            px_srcline(414);
            return _v1437;
        }
        px_srcline(415);
        if (px_is_truthy(px_eq(_v1436, px_str("Or")))) {
            px_srcline(417);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1437, _v1433}, 3));
            px_srcline(418);
            _v1443 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
            px_srcline(419);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPT"), _v1437, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(420);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(3LL)), _v1437, _v1433}, 3));
            px_srcline(421);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1443, px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1)}, 3));
            px_srcline(422);
            return _v1437;
        }
        px_srcline(423);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1437, _v1433}, 3));
        px_srcline(424);
        _v1444 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(425);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(3LL)), _v1444, _v1433}, 3));
        px_srcline(426);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_call(px_get_global("bc_binop_op"), (LXValue[]){_v1436}, 1), _v1437, _v1437, _v1444}, 5));
        px_srcline(427);
        return _v1437;
    }
    px_srcline(428);
    if (px_is_truthy(px_eq(_v1434, px_str("NullCoalesce")))) {
        px_srcline(430);
        _v1437 = _v1432;
        px_srcline(431);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(432);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(433);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1437, _v1433}, 3));
        px_srcline(434);
        _v1445 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(435);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1433, _v1445}, 2));
        px_srcline(436);
        _v1446 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(437);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("EQ"), _v1446, _v1437, _v1445}, 5));
        px_srcline(438);
        _v1443 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
        px_srcline(439);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPF"), _v1446, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(440);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1437, _v1433}, 3));
        px_srcline(441);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1443, px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1)}, 3));
        px_srcline(442);
        return _v1437;
    }
    px_srcline(443);
    if (px_is_truthy(px_eq(_v1434, px_str("IfExpr")))) {
        px_srcline(445);
        _v1437 = _v1432;
        px_srcline(446);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(447);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(448);
        _v1447 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(449);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1447, _v1433}, 3));
        px_srcline(450);
        _v1448 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
        px_srcline(451);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPF"), _v1447, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(452);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1437, _v1433}, 3));
        px_srcline(453);
        _v1449 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
        px_srcline(454);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(455);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1448, px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1)}, 3));
        px_srcline(456);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(3LL)), _v1437, _v1433}, 3));
        px_srcline(457);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1449, px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1)}, 3));
        px_srcline(458);
        return _v1437;
    }
    px_srcline(459);
    if (px_is_truthy(px_eq(_v1434, px_str("Try")))) {
        px_srcline(463);
        _v1437 = _v1432;
        px_srcline(464);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(465);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(466);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1437, _v1433}, 3));
        px_srcline(467);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("TRY"), _v1437, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(468);
        return _v1437;
    }
    px_srcline(469);
    if (px_is_truthy(px_eq(_v1434, px_str("ForceUnwrap")))) {
        px_srcline(471);
        _v1437 = _v1432;
        px_srcline(472);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(473);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(474);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1437, _v1433}, 3));
        px_srcline(475);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("FORCE"), _v1437, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(476);
        return _v1437;
    }
    px_srcline(477);
    if (px_is_truthy(({ LXValue _t1494 = px_eq(_v1434, px_str("List")); px_is_truthy(_t1494) ? _t1494 : px_eq(_v1434, px_str("Tuple")); }))) {
        px_srcline(479);
        _v1450 = px_index(_v1431, px_int(1LL));
        px_srcline(480);
        _v1451 = px_call(px_get_global("len"), (LXValue[]){_v1450}, 1);
        px_srcline(481);
        _v1437 = _v1432;
        px_srcline(482);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(483);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(484);
        _v1452 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(485);
        while (px_is_truthy(px_lt(px_index(_v1433, px_str("next_slot")), px_add(_v1452, _v1451)))) {
            px_srcline(486);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1));
        }
        px_srcline(487);
        _v1453 = px_int(0LL);
        px_srcline(488);
        while (px_is_truthy(px_lt(_v1453, _v1451))) {
            px_srcline(489);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1450, _v1453), px_add(_v1452, _v1453), _v1433}, 3));
            px_srcline(490);
             _v1453 = px_add(_v1453, px_int(1LL));
        }
        px_srcline(491);
        if (px_is_truthy(px_eq(_v1434, px_str("List")))) {
            px_srcline(492);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("NEWLIST"), _v1437, _v1452, _v1451}, 5));
        }
        else {
            px_srcline(494);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("NEWTUPLE"), _v1437, _v1452, _v1451}, 5));
        }
        px_srcline(495);
        return _v1437;
    }
    px_srcline(496);
    if (px_is_truthy(px_eq(_v1434, px_str("Dict")))) {
        px_srcline(498);
        _v1454 = px_index(_v1431, px_int(1LL));
        px_srcline(499);
        _v1451 = px_call(px_get_global("len"), (LXValue[]){_v1454}, 1);
        px_srcline(500);
        _v1437 = _v1432;
        px_srcline(501);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(502);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(503);
        _v1452 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(504);
        while (px_is_truthy(px_lt(px_index(_v1433, px_str("next_slot")), px_add(_v1452, px_mul(px_int(2LL), _v1451))))) {
            px_srcline(505);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1));
        }
        px_srcline(506);
        _v1455 = px_int(0LL);
        px_srcline(507);
        while (px_is_truthy(px_lt(_v1455, _v1451))) {
            px_srcline(508);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1454, _v1455), px_int(0LL)), px_add(_v1452, px_mul(px_int(2LL), _v1455)), _v1433}, 3));
            px_srcline(509);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1454, _v1455), px_int(1LL)), px_add(px_add(_v1452, px_mul(px_int(2LL), _v1455)), px_int(1LL)), _v1433}, 3));
            px_srcline(510);
             _v1455 = px_add(_v1455, px_int(1LL));
        }
        px_srcline(511);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("NEWDICT"), _v1437, _v1452, _v1451}, 5));
        px_srcline(512);
        return _v1437;
    }
    px_srcline(513);
    if (px_is_truthy(px_eq(_v1434, px_str("Index")))) {
        px_srcline(515);
        _v1437 = _v1432;
        px_srcline(516);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(517);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(518);
        _v1456 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(519);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1456, _v1433}, 3));
        px_srcline(520);
        _v1457 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(521);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1457, _v1433}, 3));
        px_srcline(522);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("INDEX"), _v1437, _v1456, _v1457}, 5));
        px_srcline(523);
        return _v1437;
    }
    px_srcline(524);
    if (px_is_truthy(px_eq(_v1434, px_str("Slice")))) {
        px_srcline(526);
        _v1437 = _v1432;
        px_srcline(527);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(528);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(529);
        _v1456 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(530);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1456, _v1433}, 3));
        px_srcline(531);
        _v1452 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(532);
        while (px_is_truthy(px_lt(px_index(_v1433, px_str("next_slot")), px_add(_v1452, px_int(3LL))))) {
            px_srcline(533);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1));
        }
        px_srcline(534);
        if (px_is_truthy(px_eq(px_index(_v1431, px_int(2LL)), px_null()))) {
            px_srcline(535);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1433, _v1452}, 2));
        }
        else {
            px_srcline(537);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(2LL)), _v1452, _v1433}, 3));
        }
        px_srcline(538);
        if (px_is_truthy(px_eq(px_index(_v1431, px_int(3LL)), px_null()))) {
            px_srcline(539);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1433, px_add(_v1452, px_int(1LL))}, 2));
        }
        else {
            px_srcline(541);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(3LL)), px_add(_v1452, px_int(1LL)), _v1433}, 3));
        }
        px_srcline(542);
        if (px_is_truthy(px_eq(px_index(_v1431, px_int(4LL)), px_null()))) {
            px_srcline(543);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1433, px_add(_v1452, px_int(2LL))}, 2));
        }
        else {
            px_srcline(545);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(4LL)), px_add(_v1452, px_int(2LL)), _v1433}, 3));
        }
        px_srcline(546);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("SLICE"), _v1437, _v1456, _v1452}, 5));
        px_srcline(547);
        return _v1437;
    }
    px_srcline(548);
    if (px_is_truthy(px_eq(_v1434, px_str("Field")))) {
        px_srcline(552);
        _v1458 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1431, px_int(2LL))}, 1);
        px_srcline(553);
        _v1459 = px_index(_v1431, px_int(1LL));
        px_srcline(554);
        if (px_is_truthy(px_eq(px_index(_v1459, px_int(0LL)), px_str("Var")))) {
            px_srcline(555);
            _v1460 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1459, px_int(1LL))}, 1);
            px_srcline(556);
            _v1461 = px_call(px_get_global("bc_const_find"), (LXValue[]){_v1460, _v1458}, 2);
            px_srcline(557);
            if (px_is_truthy(px_ne(_v1461, px_null()))) {
                px_srcline(558);
                _v1437 = _v1432;
                px_srcline(559);
                if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
                    px_srcline(560);
                     _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
                }
                px_srcline(561);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1461, _v1437, _v1433}, 3));
                px_srcline(562);
                return _v1437;
            }
            px_srcline(563);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1460, _v1458}, 2))) {
                px_srcline(564);
                _v1437 = _v1432;
                px_srcline(565);
                if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
                    px_srcline(566);
                     _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
                }
                px_srcline(567);
                _v1445 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1460}, 2);
                px_srcline(568);
                _v1462 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1458}, 2);
                px_srcline(569);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("NEWENUM"), _v1437, _v1445, _v1462}, 5));
                px_srcline(570);
                return _v1437;
            }
        }
        px_srcline(571);
        _v1437 = _v1432;
        px_srcline(572);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(573);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(574);
        _v1456 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(575);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1456, _v1433}, 3));
        px_srcline(576);
        _v1463 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1458}, 2);
        px_srcline(577);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("GETF"), _v1437, _v1456, _v1463}, 5));
        px_srcline(578);
        return _v1437;
    }
    px_srcline(579);
    if (px_is_truthy(px_eq(_v1434, px_str("OptionalField")))) {
        px_srcline(581);
        _v1437 = _v1432;
        px_srcline(582);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(583);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(584);
        _v1456 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(585);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1456, _v1433}, 3));
        px_srcline(586);
        _v1463 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1431, px_int(2LL))}, 1)}, 2);
        px_srcline(587);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("GETF_OPT"), _v1437, _v1456, _v1463}, 5));
        px_srcline(588);
        return _v1437;
    }
    px_srcline(589);
    if (px_is_truthy(px_eq(_v1434, px_str("Call")))) {
        px_srcline(592);
        _v1464 = px_index(_v1431, px_int(1LL));
        px_srcline(596);
        (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v1464, px_index(_v1431, px_int(2LL))}, 2));
        px_srcline(597);
        if (px_is_truthy(px_eq(px_index(_v1464, px_int(0LL)), px_str("Var")))) {
            px_srcline(598);
            _v1465 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1464, px_int(1LL))}, 1);
            px_srcline(599);
            _v1466 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1465}, 1);
            px_srcline(600);
            if (px_is_truthy(px_ge(_v1466, px_int(0LL)))) {
                px_srcline(601);
                return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1465, _v1466, px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 5);
            }
            px_srcline(602);
            if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1465}, 1))) {
                px_srcline(603);
                return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1465, px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 4);
            }
        }
        px_srcline(604);
        if (px_is_truthy(px_eq(px_index(_v1464, px_int(0LL)), px_str("Field")))) {
            px_srcline(605);
            return px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1464, px_int(1LL)), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1464, px_int(2LL))}, 1), px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 5);
        }
        px_srcline(606);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1464, px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 4);
    }
    px_srcline(607);
    if (px_is_truthy(px_eq(_v1434, px_str("Constructor")))) {
        px_srcline(609);
        _v1465 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1431, px_int(1LL))}, 1);
        px_srcline(610);
        _v1466 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1465}, 1);
        px_srcline(611);
        if (px_is_truthy(px_ge(_v1466, px_int(0LL)))) {
            px_srcline(612);
            return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1465, _v1466, px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 5);
        }
        px_srcline(613);
        if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1465}, 1))) {
            px_srcline(614);
            return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1465, px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 4);
        }
        px_srcline(616);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1431, px_int(1LL)), px_index(_v1431, px_int(2LL)), _v1432, _v1433}, 4);
    }
    px_srcline(617);
    if (px_is_truthy(px_eq(_v1434, px_str("Pipe")))) {
        px_srcline(619);
        _v1467 = px_index(_v1431, px_int(1LL));
        px_srcline(620);
        _v1468 = px_index(_v1431, px_int(2LL));
        px_srcline(621);
        if (px_is_truthy(px_eq(px_index(_v1468, px_int(0LL)), px_str("Call")))) {
            px_srcline(622);
            _v1469 = px_list_n((LXValue[]){}, 0);
            px_srcline(623);
            (void)(px_method(_v1469, "append", (LXValue[]){_v1467}, 1));
            px_srcline(624);
            _v1470 = px_int(0LL);
            px_srcline(625);
            while (px_is_truthy(px_lt(_v1470, px_call(px_get_global("len"), (LXValue[]){px_index(_v1468, px_int(2LL))}, 1)))) {
                px_srcline(626);
                (void)(px_method(_v1469, "append", (LXValue[]){px_index(px_index(_v1468, px_int(2LL)), _v1470)}, 1));
                px_srcline(627);
                 _v1470 = px_add(_v1470, px_int(1LL));
            }
            px_srcline(628);
            return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1468, px_int(1LL)), _v1469, _v1432, _v1433}, 4);
        }
        px_srcline(629);
        _v1469 = px_list_n((LXValue[]){_v1467}, 1);
        px_srcline(630);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1468, _v1469, _v1432, _v1433}, 4);
    }
    px_srcline(631);
    if (px_is_truthy(px_eq(_v1434, px_str("ListComp")))) {
        px_srcline(633);
        _v1471 = px_index(_v1431, px_int(2LL));
        px_srcline(634);
        _v1437 = _v1432;
        px_srcline(635);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(636);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(637);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("NEWLIST"), _v1437, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(638);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1433, px_str("push"), px_list_n((LXValue[]){px_index(_v1431, px_int(1LL))}, 1), px_index(_v1431, px_int(3LL)), _v1437, _v1471, px_int(0LL)}, 7));
        px_srcline(639);
        return _v1437;
    }
    px_srcline(640);
    if (px_is_truthy(px_eq(_v1434, px_str("DictComp")))) {
        px_srcline(642);
        _v1471 = px_index(_v1431, px_int(3LL));
        px_srcline(643);
        _v1437 = _v1432;
        px_srcline(644);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(645);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(646);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("NEWDICT"), _v1437, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(647);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1433, px_str("dict"), px_list_n((LXValue[]){px_index(_v1431, px_int(1LL)), px_index(_v1431, px_int(2LL))}, 2), px_index(_v1431, px_int(4LL)), _v1437, _v1471, px_int(0LL)}, 7));
        px_srcline(648);
        return _v1437;
    }
    px_srcline(649);
    if (px_is_truthy(px_eq(_v1434, px_str("GenExp")))) {
        px_srcline(650);
        return px_call(px_get_global("bc_emit_genexp"), (LXValue[]){_v1431, _v1432, _v1433}, 3);
    }
    px_srcline(651);
    if (px_is_truthy(px_eq(_v1434, px_str("Match")))) {
        px_srcline(654);
        _v1437 = _v1432;
        px_srcline(655);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(656);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(657);
        _v1472 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        px_srcline(658);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1431, px_int(1LL)), _v1472, _v1433}, 3));
        px_srcline(659);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("MOV"), _v1437, _v1472, px_int(0LL)}, 5));
        px_srcline(660);
        _v1473 = px_index(_v1431, px_int(2LL));
        px_srcline(661);
        _v1474 = px_list_n((LXValue[]){}, 0);
        px_srcline(662);
        _v1475 = px_int(0LL);
        px_srcline(663);
        while (px_is_truthy(px_lt(_v1475, px_call(px_get_global("len"), (LXValue[]){_v1473}, 1)))) {
            px_srcline(664);
            _v1476 = px_index(_v1473, _v1475);
            px_srcline(665);
            _v1477 = px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1476, px_int(1LL)), _v1472, _v1433}, 3);
            px_srcline(666);
            _v1478 = px_neg(px_int(1LL));
            px_srcline(667);
            _v1479 = px_neg(px_int(1LL));
            px_srcline(668);
            if (px_is_truthy(px_eq(_v1477, px_null()))) {
                px_srcline(670);
                if (px_is_truthy(px_ne(px_index(_v1476, px_int(2LL)), px_null()))) {
                    px_srcline(671);
                    _v1480 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
                    px_srcline(672);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1476, px_int(2LL)), _v1480, _v1433}, 3));
                    px_srcline(673);
                    _v1481 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
                    px_srcline(674);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPF"), _v1480, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(675);
                     _v1479 = _v1481;
                }
            }
            else {
                px_srcline(677);
                _v1448 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
                px_srcline(678);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPF"), _v1477, px_int(0LL), px_int(0LL)}, 5));
                px_srcline(679);
                 _v1478 = _v1448;
                px_srcline(680);
                if (px_is_truthy(px_ne(px_index(_v1476, px_int(2LL)), px_null()))) {
                    px_srcline(681);
                    _v1480 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
                    px_srcline(682);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1476, px_int(2LL)), _v1480, _v1433}, 3));
                    px_srcline(683);
                    _v1481 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
                    px_srcline(684);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMPF"), _v1480, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(685);
                     _v1479 = _v1481;
                }
            }
            px_srcline(686);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1476, px_int(3LL)), _v1437, _v1433}, 3));
            px_srcline(687);
            _v1449 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
            px_srcline(688);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(689);
            _v1482 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
            px_srcline(690);
            if (px_is_truthy(px_ge(_v1478, px_int(0LL)))) {
                px_srcline(691);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1478, _v1482}, 3));
            }
            px_srcline(692);
            if (px_is_truthy(px_ge(_v1479, px_int(0LL)))) {
                px_srcline(693);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, _v1479, _v1482}, 3));
            }
            px_srcline(694);
            (void)(px_method(_v1474, "append", (LXValue[]){_v1449}, 1));
            px_srcline(695);
             _v1475 = px_add(_v1475, px_int(1LL));
        }
        px_srcline(696);
        _v1483 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1433, px_str("bc"))}, 1);
        px_srcline(697);
        _v1484 = px_int(0LL);
        px_srcline(698);
        while (px_is_truthy(px_lt(_v1484, px_call(px_get_global("len"), (LXValue[]){_v1474}, 1)))) {
            px_srcline(699);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1433, px_index(_v1474, _v1484), _v1483}, 3));
            px_srcline(700);
             _v1484 = px_add(_v1484, px_int(1LL));
        }
        px_srcline(701);
        return _v1437;
    }
    px_srcline(702);
    if (px_is_truthy(px_eq(_v1434, px_str("Block")))) {
        px_srcline(706);
        _v1437 = _v1432;
        px_srcline(707);
        if (px_is_truthy(px_lt(_v1437, px_int(0LL)))) {
            px_srcline(708);
             _v1437 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1433}, 1);
        }
        px_srcline(709);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1433, _v1437}, 2));
        px_srcline(710);
        _v1485 = px_index(_v1431, px_int(1LL));
        px_srcline(711);
        _v1486 = px_int(0LL);
        px_srcline(712);
        while (px_is_truthy(px_lt(_v1486, px_call(px_get_global("len"), (LXValue[]){_v1485}, 1)))) {
            px_srcline(713);
            _v1487 = px_index(_v1485, _v1486);
            px_srcline(714);
            if (px_is_truthy(px_eq(px_index(_v1487, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(715);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1487, px_int(1LL)), _v1437, _v1433}, 3));
            }
            else {
                px_srcline(717);
                (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){_v1487, _v1433}, 2));
            }
            px_srcline(718);
             _v1486 = px_add(_v1486, px_int(1LL));
        }
        px_srcline(719);
        return _v1437;
    }
    px_srcline(720);
    if (px_is_truthy(px_eq(_v1434, px_str("Closure")))) {
        px_srcline(722);
        _v1488 = px_list_n((LXValue[]){}, 0);
        px_srcline(723);
        _v1484 = px_int(0LL);
        px_srcline(724);
        while (px_is_truthy(px_lt(_v1484, px_call(px_get_global("len"), (LXValue[]){px_index(_v1431, px_int(1LL))}, 1)))) {
            px_srcline(725);
            (void)(px_method(_v1488, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1431, px_int(1LL)), _v1484), px_int(1LL))}, 1)}, 1));
            px_srcline(726);
             _v1484 = px_add(_v1484, px_int(1LL));
        }
        px_srcline(727);
        _v1489 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){_v1488, px_index(_v1431, px_int(3LL))}, 2);
        px_srcline(728);
        _v1439 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1489, px_float(0), px_str("")}, 5);
        px_srcline(729);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1433, px_str("LOADK"), _v1432, _v1439, px_int(0LL)}, 5));
        px_srcline(730);
        return _v1432;
    }
    px_srcline(731);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_expr 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1431}, 1))}, 1));
px_err_1490:
    if (px_err_1490_proped) return px_err_1490_val;
    return px_null();
}

static LXValue fn_bc_emit_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_call");
    LXValue _v1495 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1496 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1497 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1498 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1499 = px_null();
    LXValue _v1500 = px_null();
    LXValue _v1501 = px_null();
    LXValue _v1502 = px_null();
    LXValue _v1503 = px_null();
    LXValue px_err_1504_val = px_null();
    int px_err_1504_proped = 0;
    px_srcline(735);
    _v1499 = _v1497;
    px_srcline(736);
    if (px_is_truthy(px_lt(_v1499, px_int(0LL)))) {
        px_srcline(737);
         _v1499 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1498}, 1);
    }
    px_srcline(738);
    _v1500 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1498}, 1);
    px_srcline(739);
    _v1501 = px_call(px_get_global("len"), (LXValue[]){_v1496}, 1);
    px_srcline(740);
    _v1502 = px_add(px_add(_v1500, px_int(1LL)), _v1501);
    px_srcline(741);
    while (px_is_truthy(px_lt(px_index(_v1498, px_str("next_slot")), _v1502))) {
        px_srcline(742);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1498}, 1));
    }
    px_srcline(743);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1495, _v1500, _v1498}, 3));
    px_srcline(744);
    _v1503 = px_int(0LL);
    px_srcline(745);
    while (px_is_truthy(px_lt(_v1503, _v1501))) {
        px_srcline(746);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1496, _v1503), px_add(px_add(_v1500, px_int(1LL)), _v1503), _v1498}, 3));
        px_srcline(747);
         _v1503 = px_add(_v1503, px_int(1LL));
    }
    px_srcline(748);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1498, px_str("CALL"), _v1499, _v1500, _v1501}, 5));
    px_srcline(749);
    return _v1499;
px_err_1504:
    if (px_err_1504_proped) return px_err_1504_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall");
    LXValue _v1505 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1506 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1507 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1508 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1509 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1510 = px_null();
    LXValue _v1511 = px_null();
    LXValue _v1512 = px_null();
    LXValue _v1513 = px_null();
    LXValue _v1514 = px_null();
    LXValue _v1515 = px_null();
    LXValue px_err_1516_val = px_null();
    int px_err_1516_proped = 0;
    px_srcline(755);
    _v1510 = _v1508;
    px_srcline(756);
    if (px_is_truthy(px_lt(_v1510, px_int(0LL)))) {
        px_srcline(757);
         _v1510 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1509}, 1);
    }
    px_srcline(758);
    _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1509}, 1);
    px_srcline(759);
    _v1512 = px_call(px_get_global("len"), (LXValue[]){_v1507}, 1);
    px_srcline(760);
    _v1513 = px_add(px_add(_v1511, px_int(1LL)), _v1512);
    px_srcline(761);
    while (px_is_truthy(px_lt(px_index(_v1509, px_str("next_slot")), _v1513))) {
        px_srcline(762);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1509}, 1));
    }
    px_srcline(763);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1505, _v1511, _v1509}, 3));
    px_srcline(764);
    _v1514 = px_int(0LL);
    px_srcline(765);
    while (px_is_truthy(px_lt(_v1514, _v1512))) {
        px_srcline(766);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1507, _v1514), px_add(px_add(_v1511, px_int(1LL)), _v1514), _v1509}, 3));
        px_srcline(767);
         _v1514 = px_add(_v1514, px_int(1LL));
    }
    px_srcline(768);
    _v1515 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1506}, 2);
    px_srcline(769);
    (void)(px_method(px_index(_v1509, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1512, _v1510, _v1511, _v1515}, 5)}, 1));
    px_srcline(770);
    return _v1510;
px_err_1516:
    if (px_err_1516_proped) return px_err_1516_val;
    return px_null();
}

static LXValue fn_bc_emit_struct_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_struct_new");
    LXValue _v1517 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1518 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1519 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1520 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1521 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1522 = px_null();
    LXValue _v1523 = px_null();
    LXValue _v1524 = px_null();
    LXValue _v1525 = px_null();
    LXValue _v1526 = px_null();
    LXValue px_err_1527_val = px_null();
    int px_err_1527_proped = 0;
    px_srcline(774);
    _v1522 = px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1518);
    px_srcline(775);
    _v1523 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1522, px_str("fnames"))}, 1);
    px_srcline(776);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1519}, 1), _v1523))) {
        px_srcline(777);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v1517), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){_v1523}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1519}, 1)}, 1))}, 1));
    }
    px_srcline(778);
    _v1524 = _v1520;
    px_srcline(779);
    if (px_is_truthy(px_lt(_v1524, px_int(0LL)))) {
        px_srcline(780);
         _v1524 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1521}, 1);
    }
    px_srcline(781);
    _v1525 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1521}, 1);
    px_srcline(782);
    while (px_is_truthy(px_lt(px_index(_v1521, px_str("next_slot")), px_add(_v1525, _v1523)))) {
        px_srcline(783);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1521}, 1));
    }
    px_srcline(784);
    _v1526 = px_int(0LL);
    px_srcline(785);
    while (px_is_truthy(px_lt(_v1526, _v1523))) {
        px_srcline(786);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1519, _v1526), px_add(_v1525, _v1526), _v1521}, 3));
        px_srcline(787);
         _v1526 = px_add(_v1526, px_int(1LL));
    }
    px_srcline(788);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1521, px_str("NEWSTRUCT"), _v1524, _v1518, _v1525}, 5));
    px_srcline(789);
    return _v1524;
px_err_1527:
    if (px_err_1527_proped) return px_err_1527_val;
    return px_null();
}

static LXValue fn_bc_emit_enum_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_enum_new");
    LXValue _v1528 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1529 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1530 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1531 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1532 = px_null();
    LXValue _v1533 = px_null();
    LXValue _v1534 = px_null();
    LXValue _v1535 = px_null();
    LXValue _v1536 = px_null();
    LXValue _v1537 = px_null();
    LXValue px_err_1538_val = px_null();
    int px_err_1538_proped = 0;
    px_srcline(792);
    _v1532 = px_null();
    px_srcline(793);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1529}, 1), px_int(1LL)))) {
        px_srcline(794);
        _v1533 = px_index(_v1529, px_int(0LL));
        px_srcline(795);
        if (px_is_truthy(px_eq(px_index(_v1533, px_int(0LL)), px_str("Var")))) {
            px_srcline(796);
            _v1534 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1533, px_int(1LL))}, 1);
            px_srcline(797);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1528, _v1534}, 2))) {
                px_srcline(798);
                 _v1532 = _v1534;
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v1533, px_int(0LL)), px_str("Str")))) {
            px_srcline(800);
             _v1532 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1533, px_int(1LL))}, 1);
        }
    }
    px_srcline(801);
    if (px_is_truthy(px_eq(_v1532, px_null()))) {
        px_srcline(802);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit enum 构造未实现（仅支持 Color(Variant)/Color(\"Variant\"): "), px_call(px_get_global("str"), (LXValue[]){_v1528}, 1))}, 1));
    }
    px_srcline(803);
    _v1535 = _v1530;
    px_srcline(804);
    if (px_is_truthy(px_lt(_v1535, px_int(0LL)))) {
        px_srcline(805);
         _v1535 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1531}, 1);
    }
    px_srcline(806);
    _v1536 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1528}, 2);
    px_srcline(807);
    _v1537 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1532}, 2);
    px_srcline(808);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1531, px_str("NEWENUM"), _v1535, _v1536, _v1537}, 5));
    px_srcline(809);
    return _v1535;
px_err_1538:
    if (px_err_1538_proped) return px_err_1538_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall_slot");
    LXValue _v1539 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1540 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1541 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1542 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1543 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1544 = px_null();
    LXValue _v1545 = px_null();
    LXValue _v1546 = px_null();
    LXValue _v1547 = px_null();
    LXValue _v1548 = px_null();
    LXValue _v1549 = px_null();
    LXValue px_err_1550_val = px_null();
    int px_err_1550_proped = 0;
    px_srcline(816);
    _v1544 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1539}, 1);
    px_srcline(817);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1539, px_str("MOV"), _v1544, _v1540, px_int(0LL)}, 5));
    px_srcline(818);
    _v1545 = px_call(px_get_global("len"), (LXValue[]){_v1542}, 1);
    px_srcline(819);
    _v1546 = px_add(px_add(_v1544, px_int(1LL)), _v1545);
    px_srcline(820);
    while (px_is_truthy(px_lt(px_index(_v1539, px_str("next_slot")), _v1546))) {
        px_srcline(821);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1539}, 1));
    }
    px_srcline(822);
    _v1547 = px_int(0LL);
    px_srcline(823);
    while (px_is_truthy(px_lt(_v1547, _v1545))) {
        px_srcline(824);
        _v1548 = px_index(_v1542, _v1547);
        px_srcline(825);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1548}, 1), px_str("int")))) {
            px_srcline(826);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1539, px_str("MOV"), px_add(px_add(_v1544, px_int(1LL)), _v1547), _v1548, px_int(0LL)}, 5));
        }
        else {
            px_srcline(828);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1548, px_add(px_add(_v1544, px_int(1LL)), _v1547), _v1539}, 3));
        }
        px_srcline(829);
         _v1547 = px_add(_v1547, px_int(1LL));
    }
    px_srcline(830);
    _v1549 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1541}, 2);
    px_srcline(831);
    (void)(px_method(px_index(_v1539, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1545, _v1543, _v1544, _v1549}, 5)}, 1));
    px_srcline(832);
    return _v1543;
px_err_1550:
    if (px_err_1550_proped) return px_err_1550_val;
    return px_null();
}

static LXValue fn_bc_emit_push_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_push_lambda");
    LXValue _v1551 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1552 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1553 = px_null();
    LXValue _v1554 = px_null();
    LXValue _v1555 = px_null();
    LXValue _v1556 = px_null();
    LXValue px_err_1557_val = px_null();
    int px_err_1557_proped = 0;
    px_srcline(836);
    _v1553 = px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("nclosure"))}, 1)), px_str(">"));
    px_srcline(837);
    px_index_set(px_get_global("g_bcm"), px_str("nclosure"), px_add(px_index(px_get_global("g_bcm"), px_str("nclosure")), px_int(1LL)));
    px_srcline(838);
    _v1554 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1553, px_call(px_get_global("len"), (LXValue[]){_v1551}, 1)}, 2);
    px_srcline(839);
    _v1555 = px_int(0LL);
    px_srcline(840);
    while (px_is_truthy(px_lt(_v1555, px_call(px_get_global("len"), (LXValue[]){_v1551}, 1)))) {
        px_srcline(841);
        px_index_set(px_index(_v1554, px_str("smap")), px_index(_v1551, _v1555), _v1555);
        px_srcline(842);
         _v1555 = px_add(_v1555, px_int(1LL));
    }
    px_srcline(843);
    (void)(px_method(px_index(px_get_global("g_bcm"), px_str("funcs")), "push", (LXValue[]){_v1554}, 1));
    px_srcline(844);
    _v1556 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1554}, 1);
    px_srcline(845);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1552, _v1556, _v1554}, 3));
    px_srcline(846);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1554, px_str("RET"), _v1556, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(847);
    px_index_set(_v1554, px_str("nslots"), px_index(_v1554, px_str("next_slot")));
    px_srcline(848);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("funcs"))}, 1), px_int(1LL));
px_err_1557:
    if (px_err_1557_proped) return px_err_1557_val;
    return px_null();
}

static LXValue fn_bc_emit_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_comp");
    LXValue _v1558 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1559 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1560 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1561 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1562 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1563 = (nargs > 5) ? args[5] : px_null();
    LXValue _v1564 = (nargs > 6) ? args[6] : px_null();
    LXValue _v1565 = px_null();
    LXValue _v1566 = px_null();
    LXValue _v1567 = px_null();
    LXValue _v1568 = px_null();
    LXValue _v1569 = px_null();
    LXValue _v1570 = px_null();
    LXValue _v1571 = px_null();
    LXValue _v1572 = px_null();
    LXValue _v1573 = px_null();
    LXValue _v1574 = px_null();
    LXValue _v1575 = px_null();
    LXValue _v1576 = px_null();
    LXValue _v1577 = px_null();
    LXValue _v1578 = px_null();
    LXValue _v1579 = px_null();
    LXValue _v1580 = px_null();
    LXValue _v1581 = px_null();
    LXValue _v1582 = px_null();
    LXValue _v1583 = px_null();
    LXValue _v1584 = px_null();
    LXValue _v1585 = px_null();
    LXValue _v1586 = px_null();
    LXValue _v1587 = px_null();
    LXValue _v1588 = px_null();
    LXValue _v1589 = px_null();
    LXValue _v1590 = px_null();
    LXValue _v1591 = px_null();
    LXValue _v1592 = px_null();
    LXValue _v1593 = px_null();
    LXValue px_err_1594_val = px_null();
    int px_err_1594_proped = 0;
    px_srcline(855);
    _v1565 = px_call(px_get_global("len"), (LXValue[]){_v1563}, 1);
    px_srcline(856);
    if (px_is_truthy(px_ge(_v1564, _v1565))) {
        px_srcline(857);
        _v1566 = px_neg(px_int(1LL));
        px_srcline(858);
        if (px_is_truthy(px_ne(_v1561, px_null()))) {
            px_srcline(859);
            _v1567 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
            px_srcline(860);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1561, _v1567, _v1558}, 3));
            px_srcline(861);
             _v1566 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1558, px_str("bc"))}, 1);
            px_srcline(862);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("JMPF"), _v1567, px_int(0LL), px_int(0LL)}, 5));
        }
        px_srcline(863);
        if (px_is_truthy(px_eq(_v1559, px_str("dict")))) {
            px_srcline(864);
            _v1568 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
            px_srcline(865);
            _v1569 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
            px_srcline(866);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1560, px_int(0LL)), _v1568, _v1558}, 3));
            px_srcline(867);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1560, px_int(1LL)), _v1569, _v1558}, 3));
            px_srcline(868);
            (void)(px_call(px_get_global("bc_emit_methodcall_slot"), (LXValue[]){_v1558, _v1562, px_str("set"), px_list_n((LXValue[]){_v1568, _v1569}, 2), px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1)}, 5));
        }
        else {
            px_srcline(870);
            _v1570 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
            px_srcline(871);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1560, px_int(0LL)), _v1570, _v1558}, 3));
            px_srcline(872);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("LISTPUSH"), _v1570, _v1562, px_int(0LL)}, 5));
        }
        px_srcline(873);
        if (px_is_truthy(px_ge(_v1566, px_int(0LL)))) {
            px_srcline(874);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1558, _v1566, px_call(px_get_global("len"), (LXValue[]){px_index(_v1558, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(875);
        return px_null();
    }
    px_srcline(876);
    _v1571 = px_index(_v1563, _v1564);
    px_srcline(877);
    _v1572 = px_index(_v1571, px_int(1LL));
    px_srcline(883);
    _v1573 = px_list_n((LXValue[]){}, 0);
    px_srcline(884);
    _v1574 = px_int(0LL);
    px_srcline(885);
    while (px_is_truthy(px_lt(_v1574, px_call(px_get_global("len"), (LXValue[]){_v1572}, 1)))) {
        px_srcline(886);
        _v1575 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1572, _v1574)}, 1);
        px_srcline(887);
        _v1576 = px_neg(px_int(1LL));
        px_srcline(888);
        if (px_is_truthy(px_method(px_index(_v1558, px_str("smap")), "has", (LXValue[]){_v1575}, 1))) {
            px_srcline(889);
             _v1576 = px_index(px_index(_v1558, px_str("smap")), _v1575);
        }
        px_srcline(890);
        _v1577 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
        px_srcline(891);
        px_index_set(px_index(_v1558, px_str("smap")), _v1575, _v1577);
        px_srcline(892);
        (void)(px_method(_v1573, "append", (LXValue[]){px_list_n((LXValue[]){_v1575, _v1576}, 2)}, 1));
        px_srcline(893);
         _v1574 = px_add(_v1574, px_int(1LL));
    }
    px_srcline(894);
    _v1578 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
    px_srcline(895);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1571, px_int(2LL)), _v1578, _v1558}, 3));
    px_srcline(896);
    _v1579 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
    px_srcline(897);
    _v1580 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
    px_srcline(898);
    while (px_is_truthy(px_lt(px_index(_v1558, px_str("next_slot")), px_add(_v1579, px_int(2LL))))) {
        px_srcline(899);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1));
    }
    px_srcline(900);
    _v1581 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(901);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("GETG"), _v1579, _v1581, px_int(0LL)}, 5));
    px_srcline(902);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("MOV"), px_add(_v1579, px_int(1LL)), _v1578, px_int(0LL)}, 5));
    px_srcline(903);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("CALL"), _v1580, _v1579, px_int(1LL)}, 5));
    px_srcline(904);
    _v1582 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
    px_srcline(905);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1558, _v1582, px_int(0LL)}, 3));
    px_srcline(906);
    _v1583 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1558, px_str("bc"))}, 1);
    px_srcline(907);
    _v1584 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
    px_srcline(908);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("LT"), _v1584, _v1582, _v1580}, 5));
    px_srcline(909);
    _v1585 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1558, px_str("bc"))}, 1);
    px_srcline(910);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("JMPF"), _v1584, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(911);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1572}, 1), px_int(1LL)))) {
        px_srcline(912);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("INDEX"), px_index(px_index(_v1558, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1572, px_int(0LL))}, 1)), _v1578, _v1582}, 5));
    }
    else {
        px_srcline(915);
        _v1586 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
        px_srcline(916);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("INDEX"), _v1586, _v1578, _v1582}, 5));
        px_srcline(917);
        _v1587 = px_int(0LL);
        px_srcline(918);
        while (px_is_truthy(px_lt(_v1587, px_call(px_get_global("len"), (LXValue[]){_v1572}, 1)))) {
            px_srcline(919);
            _v1588 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
            px_srcline(920);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1558, _v1588, _v1587}, 3));
            px_srcline(921);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("INDEX"), px_index(px_index(_v1558, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1572, _v1587)}, 1)), _v1586, _v1588}, 5));
            px_srcline(922);
             _v1587 = px_add(_v1587, px_int(1LL));
        }
    }
    px_srcline(923);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1558, _v1559, _v1560, _v1561, _v1562, _v1563, px_add(_v1564, px_int(1LL))}, 7));
    px_srcline(924);
    _v1589 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1558}, 1);
    px_srcline(925);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1558, _v1589, px_int(1LL)}, 3));
    px_srcline(926);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("ADD"), _v1582, _v1582, _v1589}, 5));
    px_srcline(927);
    _v1590 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1558, px_str("bc"))}, 1);
    px_srcline(928);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1558, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(929);
    _v1591 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1558, px_str("bc"))}, 1);
    px_srcline(930);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1558, _v1590, _v1583}, 3));
    px_srcline(931);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1558, _v1585, _v1591}, 3));
    px_srcline(933);
    _v1592 = px_int(0LL);
    px_srcline(934);
    while (px_is_truthy(px_lt(_v1592, px_call(px_get_global("len"), (LXValue[]){_v1573}, 1)))) {
        px_srcline(935);
        _v1593 = px_index(_v1573, _v1592);
        px_srcline(936);
        if (px_is_truthy(px_lt(px_index(_v1593, px_int(1LL)), px_int(0LL)))) {
            px_srcline(937);
            (void)(px_method(px_index(_v1558, px_str("smap")), "remove", (LXValue[]){px_index(_v1593, px_int(0LL))}, 1));
        }
        else {
            px_srcline(939);
            px_index_set(px_index(_v1558, px_str("smap")), px_index(_v1593, px_int(0LL)), px_index(_v1593, px_int(1LL)));
        }
        px_srcline(940);
         _v1592 = px_add(_v1592, px_int(1LL));
    }
px_err_1594:
    if (px_err_1594_proped) return px_err_1594_val;
    return px_null();
}

static LXValue fn_bc_emit_genexp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_genexp");
    LXValue _v1595 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1596 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1597 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1598 = px_null();
    LXValue _v1599 = px_null();
    LXValue _v1600 = px_null();
    LXValue _v1601 = px_null();
    LXValue _v1602 = px_null();
    LXValue _v1603 = px_null();
    LXValue _v1604 = px_null();
    LXValue _v1605 = px_null();
    LXValue _v1606 = px_null();
    LXValue px_err_1607_val = px_null();
    int px_err_1607_proped = 0;
    px_srcline(944);
    _v1598 = px_index(_v1595, px_int(2LL));
    px_srcline(945);
    if (px_is_truthy(({ LXValue _t1608 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1598}, 1), px_int(1LL)); px_is_truthy(_t1608) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1598, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1608; }))) {
        px_srcline(948);
        _v1599 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1598, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
        px_srcline(949);
        _v1600 = _v1596;
        px_srcline(950);
        if (px_is_truthy(px_lt(_v1600, px_int(0LL)))) {
            px_srcline(951);
             _v1600 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1597}, 1);
        }
        px_srcline(952);
        _v1601 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1597}, 1);
        px_srcline(953);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1598, px_int(0LL)), px_int(2LL)), _v1601, _v1597}, 3));
        px_srcline(954);
        _v1602 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1599}, 1), px_index(_v1595, px_int(1LL))}, 2);
        px_srcline(955);
        _v1603 = px_neg(px_int(1LL));
        px_srcline(956);
        if (px_is_truthy(px_ne(px_index(_v1595, px_int(3LL)), px_null()))) {
            px_srcline(957);
             _v1603 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1599}, 1), px_index(_v1595, px_int(3LL))}, 2);
        }
        px_srcline(958);
        _v1604 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1597}, 1);
        px_srcline(959);
        while (px_is_truthy(px_lt(px_index(_v1597, px_str("next_slot")), px_add(_v1604, px_int(2LL))))) {
            px_srcline(960);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1597}, 1));
        }
        px_srcline(961);
        _v1605 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1602, px_float(0), px_str("")}, 5);
        px_srcline(962);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1597, px_str("LOADK"), _v1604, _v1605, px_int(0LL)}, 5));
        px_srcline(963);
        if (px_is_truthy(px_ge(_v1603, px_int(0LL)))) {
            px_srcline(964);
            _v1606 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1603, px_float(0), px_str("")}, 5);
            px_srcline(965);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1597, px_str("LOADK"), px_add(_v1604, px_int(1LL)), _v1606, px_int(0LL)}, 5));
        }
        else {
            px_srcline(967);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1597, px_add(_v1604, px_int(1LL))}, 2));
        }
        px_srcline(968);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1597, px_str("NEWGEN"), _v1600, _v1601, _v1604}, 5));
        px_srcline(969);
        return _v1600;
    }
    px_srcline(973);
    _v1600 = _v1596;
    px_srcline(974);
    if (px_is_truthy(px_lt(_v1600, px_int(0LL)))) {
        px_srcline(975);
         _v1600 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1597}, 1);
    }
    px_srcline(976);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1597, px_str("NEWLIST"), _v1600, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(977);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1597, px_str("push"), px_list_n((LXValue[]){px_index(_v1595, px_int(1LL))}, 1), px_index(_v1595, px_int(3LL)), _v1600, _v1598, px_int(0LL)}, 7));
    px_srcline(978);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1597, px_str("GENFROMLIST"), _v1600, _v1600, px_int(0LL)}, 5));
    px_srcline(979);
    return _v1600;
px_err_1607:
    if (px_err_1607_proped) return px_err_1607_val;
    return px_null();
}

static LXValue fn_bc_match_enumvar(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_enumvar");
    LXValue _v1609 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1610 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1611 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1612 = px_null();
    LXValue _v1613 = px_null();
    LXValue _v1614 = px_null();
    LXValue _v1615 = px_null();
    LXValue px_err_1616_val = px_null();
    int px_err_1616_proped = 0;
    px_srcline(985);
    _v1612 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1611}, 1);
    px_srcline(986);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1611, px_str("ENUMVAR"), _v1612, _v1610, px_int(0LL)}, 5));
    px_srcline(987);
    _v1613 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1609}, 5);
    px_srcline(988);
    _v1614 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1611}, 1);
    px_srcline(989);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1611, px_str("LOADK"), _v1614, _v1613, px_int(0LL)}, 5));
    px_srcline(990);
    _v1615 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1611}, 1);
    px_srcline(991);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1611, px_str("EQ"), _v1615, _v1612, _v1614}, 5));
    px_srcline(992);
    return _v1615;
px_err_1616:
    if (px_err_1616_proped) return px_err_1616_val;
    return px_null();
}

static LXValue fn_bc_match_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_cond");
    LXValue _v1617 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1618 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1619 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1620 = px_null();
    LXValue _v1621 = px_null();
    LXValue _v1622 = px_null();
    LXValue _v1623 = px_null();
    LXValue _v1624 = px_null();
    LXValue px_err_1625_val = px_null();
    int px_err_1625_proped = 0;
    px_srcline(994);
    _v1620 = px_index(_v1617, px_int(0LL));
    px_srcline(995);
    if (px_is_truthy(px_eq(_v1620, px_str("PatWildcard")))) {
        px_srcline(996);
        return px_null();
    }
    px_srcline(997);
    if (px_is_truthy(px_eq(_v1620, px_str("PatBinding")))) {
        px_srcline(998);
        _v1621 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1617, px_int(1LL))}, 1);
        px_srcline(999);
        if (px_is_truthy(({ LXValue _t1627 = ({ LXValue _t1626 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1621}, 1), px_int(0LL)); px_is_truthy(_t1626) ? px_ge(px_index(_v1621, px_int(0LL)), px_str("A")) : _t1626; }); px_is_truthy(_t1627) ? px_le(px_index(_v1621, px_int(0LL)), px_str("Z")) : _t1627; }))) {
            px_srcline(1001);
            return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){_v1621, _v1618, _v1619}, 3);
        }
        px_srcline(1002);
        return px_null();
    }
    px_srcline(1003);
    if (px_is_truthy(px_eq(_v1620, px_str("PatTuple")))) {
        px_srcline(1004);
        _v1622 = px_index(_v1617, px_int(1LL));
        px_srcline(1005);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1622}, 1), px_int(0LL)))) {
            px_srcline(1006);
            return px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1622, px_int(0LL)), _v1618, _v1619}, 3);
        }
        px_srcline(1007);
        return px_null();
    }
    px_srcline(1008);
    if (px_is_truthy(px_eq(_v1620, px_str("PatConstructor")))) {
        px_srcline(1009);
        return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1617, px_int(1LL))}, 1), _v1618, _v1619}, 3);
    }
    px_srcline(1010);
    if (px_is_truthy(px_eq(_v1620, px_str("PatLiteral")))) {
        px_srcline(1011);
        _v1623 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1619}, 1);
        px_srcline(1012);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1617, px_int(1LL)), _v1623, _v1619}, 3));
        px_srcline(1013);
        _v1624 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1619}, 1);
        px_srcline(1014);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1619, px_str("EQ"), _v1624, _v1618, _v1623}, 5));
        px_srcline(1015);
        return _v1624;
    }
    px_srcline(1016);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_match_cond 未知 pattern: "), px_call(px_get_global("str"), (LXValue[]){_v1617}, 1))}, 1));
px_err_1625:
    if (px_err_1625_proped) return px_err_1625_val;
    return px_null();
}

static LXValue fn_bc_assign_local_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_local_slot");
    LXValue _v1628 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1629 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1630 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1631 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1632 = px_null();
    LXValue _v1633 = px_null();
    LXValue px_err_1634_val = px_null();
    int px_err_1634_proped = 0;
    px_srcline(1020);
    if (px_is_truthy(px_eq(_v1629, px_str("Assign")))) {
        px_srcline(1027);
        _v1632 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1631}, 1);
        px_srcline(1028);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1630, _v1632, _v1631}, 3));
        px_srcline(1029);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1631, px_str("MOV"), _v1628, _v1632, px_int(0LL)}, 5));
        px_srcline(1030);
        return px_null();
    }
    px_srcline(1031);
    _v1633 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1631}, 1);
    px_srcline(1032);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1630, _v1633, _v1631}, 3));
    px_srcline(1033);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1631, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1629}, 1)}, 1), _v1628, _v1628, _v1633}, 5));
px_err_1634:
    if (px_err_1634_proped) return px_err_1634_val;
    return px_null();
}

static LXValue fn_bc_assign_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_global");
    LXValue _v1635 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1636 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1637 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1638 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1639 = px_null();
    LXValue _v1640 = px_null();
    LXValue _v1641 = px_null();
    LXValue px_err_1642_val = px_null();
    int px_err_1642_proped = 0;
    px_srcline(1035);
    _v1639 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1635}, 2);
    px_srcline(1036);
    if (px_is_truthy(px_eq(_v1636, px_str("Assign")))) {
        px_srcline(1037);
        _v1640 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1638}, 1);
        px_srcline(1038);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1637, _v1640, _v1638}, 3));
        px_srcline(1039);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1638, px_str("SETG"), _v1639, _v1640, px_int(0LL)}, 5));
        px_srcline(1040);
        return px_null();
    }
    px_srcline(1041);
    _v1640 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1638}, 1);
    px_srcline(1042);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1638, px_str("GETG"), _v1640, _v1639, px_int(0LL)}, 5));
    px_srcline(1043);
    _v1641 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1638}, 1);
    px_srcline(1044);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1637, _v1641, _v1638}, 3));
    px_srcline(1045);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1638, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1636}, 1)}, 1), _v1640, _v1640, _v1641}, 5));
    px_srcline(1046);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1638, px_str("SETG"), _v1639, _v1640, px_int(0LL)}, 5));
px_err_1642:
    if (px_err_1642_proped) return px_err_1642_val;
    return px_null();
}

static LXValue fn_bc_assign_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_index");
    LXValue _v1643 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1644 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1645 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1646 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1647 = px_null();
    LXValue _v1648 = px_null();
    LXValue _v1649 = px_null();
    LXValue _v1650 = px_null();
    LXValue px_err_1651_val = px_null();
    int px_err_1651_proped = 0;
    px_srcline(1049);
    _v1647 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1646}, 1);
    px_srcline(1050);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1643, px_int(1LL)), _v1647, _v1646}, 3));
    px_srcline(1051);
    _v1648 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1646}, 1);
    px_srcline(1052);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1643, px_int(2LL)), _v1648, _v1646}, 3));
    px_srcline(1053);
    _v1649 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1646}, 1);
    px_srcline(1054);
    if (px_is_truthy(px_eq(_v1644, px_str("Assign")))) {
        px_srcline(1055);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1645, _v1649, _v1646}, 3));
    }
    else {
        px_srcline(1057);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1646, px_str("INDEX"), _v1649, _v1647, _v1648}, 5));
        px_srcline(1058);
        _v1650 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1646}, 1);
        px_srcline(1059);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1645, _v1650, _v1646}, 3));
        px_srcline(1060);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1646, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1644}, 1)}, 1), _v1649, _v1649, _v1650}, 5));
    }
    px_srcline(1061);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1646, px_str("SETIDX"), _v1649, _v1647, _v1648}, 5));
px_err_1651:
    if (px_err_1651_proped) return px_err_1651_val;
    return px_null();
}

static LXValue fn_bc_assign_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_field");
    LXValue _v1652 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1653 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1654 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1655 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1656 = px_null();
    LXValue _v1657 = px_null();
    LXValue _v1658 = px_null();
    LXValue _v1659 = px_null();
    LXValue px_err_1660_val = px_null();
    int px_err_1660_proped = 0;
    px_srcline(1064);
    _v1656 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1065);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1652, px_int(1LL)), _v1656, _v1655}, 3));
    px_srcline(1066);
    _v1657 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1652, px_int(2LL))}, 1)}, 2);
    px_srcline(1067);
    _v1658 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1068);
    if (px_is_truthy(px_eq(_v1653, px_str("Assign")))) {
        px_srcline(1069);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1654, _v1658, _v1655}, 3));
    }
    else {
        px_srcline(1071);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("GETF"), _v1658, _v1656, _v1657}, 5));
        px_srcline(1072);
        _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(1073);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1654, _v1659, _v1655}, 3));
        px_srcline(1074);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1653}, 1)}, 1), _v1658, _v1658, _v1659}, 5));
    }
    px_srcline(1075);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("SETF"), _v1658, _v1656, _v1657}, 5));
px_err_1660:
    if (px_err_1660_proped) return px_err_1660_val;
    return px_null();
}

static LXValue fn_bc_assign_op_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_op_name");
    LXValue _v1661 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1662_val = px_null();
    int px_err_1662_proped = 0;
    px_srcline(1078);
    if (px_is_truthy(px_eq(_v1661, px_str("Plus")))) {
        px_srcline(1079);
        return px_str("Add");
    }
    px_srcline(1080);
    if (px_is_truthy(px_eq(_v1661, px_str("Minus")))) {
        px_srcline(1081);
        return px_str("Sub");
    }
    px_srcline(1082);
    if (px_is_truthy(px_eq(_v1661, px_str("Star")))) {
        px_srcline(1083);
        return px_str("Mul");
    }
    px_srcline(1084);
    if (px_is_truthy(px_eq(_v1661, px_str("Slash")))) {
        px_srcline(1085);
        return px_str("Div");
    }
    px_srcline(1086);
    if (px_is_truthy(px_eq(_v1661, px_str("IntDiv")))) {
        px_srcline(1087);
        return px_str("IntDiv");
    }
    px_srcline(1088);
    if (px_is_truthy(px_eq(_v1661, px_str("Mod")))) {
        px_srcline(1089);
        return px_str("Mod");
    }
    px_srcline(1090);
    if (px_is_truthy(px_eq(_v1661, px_str("Pow")))) {
        px_srcline(1091);
        return px_str("Pow");
    }
    px_srcline(1092);
    if (px_is_truthy(px_eq(_v1661, px_str("BitAnd")))) {
        px_srcline(1093);
        return px_str("BitAnd");
    }
    px_srcline(1094);
    if (px_is_truthy(px_eq(_v1661, px_str("BitOr")))) {
        px_srcline(1095);
        return px_str("BitOr");
    }
    px_srcline(1096);
    if (px_is_truthy(px_eq(_v1661, px_str("BitXor")))) {
        px_srcline(1097);
        return px_str("BitXor");
    }
    px_srcline(1098);
    if (px_is_truthy(px_eq(_v1661, px_str("Shl")))) {
        px_srcline(1099);
        return px_str("Shl");
    }
    px_srcline(1100);
    if (px_is_truthy(px_eq(_v1661, px_str("Shr")))) {
        px_srcline(1101);
        return px_str("Shr");
    }
    px_srcline(1102);
    if (px_is_truthy(px_eq(_v1661, px_str("ShrU")))) {
        px_srcline(1103);
        return px_str("ShrU");
    }
    px_srcline(1104);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_assign_op_name 未知赋值 op: "), px_call(px_get_global("str"), (LXValue[]){_v1661}, 1))}, 1));
px_err_1662:
    if (px_err_1662_proped) return px_err_1662_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt_inner");
    LXValue _v1663 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1664 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1665 = px_null();
    LXValue _v1666 = px_null();
    LXValue _v1667 = px_null();
    LXValue _v1668 = px_null();
    LXValue _v1669 = px_null();
    LXValue _v1670 = px_null();
    LXValue _v1671 = px_null();
    LXValue _v1672 = px_null();
    LXValue _v1673 = px_null();
    LXValue _v1674 = px_null();
    LXValue _v1675 = px_null();
    LXValue _v1676 = px_null();
    LXValue _v1677 = px_null();
    LXValue _v1678 = px_null();
    LXValue _v1679 = px_null();
    LXValue _v1680 = px_null();
    LXValue _v1681 = px_null();
    LXValue _v1682 = px_null();
    LXValue _v1683 = px_null();
    LXValue _v1684 = px_null();
    LXValue _v1685 = px_null();
    LXValue _v1686 = px_null();
    LXValue _v1687 = px_null();
    LXValue _v1688 = px_null();
    LXValue px_err_1689_val = px_null();
    int px_err_1689_proped = 0;
    px_srcline(1107);
    _v1665 = px_index(_v1663, px_int(0LL));
    px_srcline(1108);
    if (px_is_truthy(px_eq(_v1665, px_str("VarDecl")))) {
        px_srcline(1111);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v1663}, 1));
        px_srcline(1113);
        _v1666 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1663, px_int(2LL))}, 1);
        px_srcline(1114);
        if (px_is_truthy(px_eq(px_index(_v1663, px_int(4LL)), px_null()))) {
            px_srcline(1117);
            if (px_is_truthy(px_index(_v1664, px_str("is_top")))) {
                px_srcline(1118);
                _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
                px_srcline(1119);
                (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1664, _v1667}, 2));
                px_srcline(1120);
                _v1668 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1666}, 2);
                px_srcline(1121);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("SETG"), _v1668, _v1667, px_int(0LL)}, 5));
            }
            px_srcline(1122);
            return px_null();
        }
        px_srcline(1123);
        if (px_is_truthy(px_index(_v1664, px_str("is_top")))) {
            px_srcline(1125);
            _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
            px_srcline(1126);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1663, px_int(4LL)), _v1667, _v1664}, 3));
            px_srcline(1127);
            _v1668 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1666}, 2);
            px_srcline(1128);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("SETG"), _v1668, _v1667, px_int(0LL)}, 5));
            px_srcline(1129);
            return px_null();
        }
        px_srcline(1133);
        _v1669 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1664, _v1666}, 2);
        px_srcline(1134);
        _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
        px_srcline(1135);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1663, px_int(4LL)), _v1667, _v1664}, 3));
        px_srcline(1136);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("MOV"), _v1669, _v1667, px_int(0LL)}, 5));
        px_srcline(1137);
        return px_null();
    }
    px_srcline(1138);
    if (px_is_truthy(px_eq(_v1665, px_str("Assign")))) {
        px_srcline(1141);
        _v1670 = px_index(_v1663, px_int(1LL));
        px_srcline(1142);
        _v1671 = px_index(_v1663, px_int(2LL));
        px_srcline(1143);
        _v1672 = px_index(_v1663, px_int(3LL));
        px_srcline(1144);
        if (px_is_truthy(px_eq(_v1671, px_str("Append")))) {
            px_srcline(1145);
            _v1673 = px_list_n((LXValue[]){_v1672}, 1);
            px_srcline(1146);
            (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){_v1670, px_str("append"), _v1673, px_neg(px_int(1LL)), _v1664}, 5));
            px_srcline(1147);
            return px_null();
        }
        px_srcline(1150);
        (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v1670, _v1671, _v1672}, 3));
        px_srcline(1151);
        _v1674 = px_index(_v1670, px_int(0LL));
        px_srcline(1152);
        if (px_is_truthy(px_eq(_v1674, px_str("Var")))) {
            px_srcline(1153);
            _v1666 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1670, px_int(1LL))}, 1);
            px_srcline(1154);
            if (px_is_truthy(px_method(px_index(_v1664, px_str("smap")), "has", (LXValue[]){_v1666}, 1))) {
                px_srcline(1156);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){px_index(px_index(_v1664, px_str("smap")), _v1666), _v1671, _v1672, _v1664}, 4));
                px_srcline(1157);
                return px_null();
            }
            px_srcline(1158);
            if (px_is_truthy(({ LXValue _t1690 = px_not(px_index(_v1664, px_str("is_top"))); px_is_truthy(_t1690) ? px_not(px_call(px_get_global("bc_is_global"), (LXValue[]){_v1666}, 1)) : _t1690; }))) {
                px_srcline(1160);
                _v1669 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1664, _v1666}, 2);
                px_srcline(1161);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){_v1669, _v1671, _v1672, _v1664}, 4));
                px_srcline(1162);
                return px_null();
            }
            px_srcline(1164);
            (void)(px_call(px_get_global("bc_assign_global"), (LXValue[]){_v1666, _v1671, _v1672, _v1664}, 4));
            px_srcline(1165);
            return px_null();
        }
        px_srcline(1166);
        if (px_is_truthy(px_eq(_v1674, px_str("Index")))) {
            px_srcline(1167);
            (void)(px_call(px_get_global("bc_assign_index"), (LXValue[]){_v1670, _v1671, _v1672, _v1664}, 4));
            px_srcline(1168);
            return px_null();
        }
        px_srcline(1169);
        if (px_is_truthy(px_eq(_v1674, px_str("Field")))) {
            px_srcline(1170);
            (void)(px_call(px_get_global("bc_assign_field"), (LXValue[]){_v1670, _v1671, _v1672, _v1664}, 4));
            px_srcline(1171);
            return px_null();
        }
        px_srcline(1172);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt Assign 目标未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1670}, 1))}, 1));
    }
    px_srcline(1173);
    if (px_is_truthy(px_eq(_v1665, px_str("Return")))) {
        px_srcline(1174);
        if (px_is_truthy(px_ne(px_index(_v1663, px_int(1LL)), px_null()))) {
            px_srcline(1175);
            _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
            px_srcline(1176);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1663, px_int(1LL)), _v1667, _v1664}, 3));
            px_srcline(1177);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("RET"), _v1667, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1178);
            return px_null();
        }
        px_srcline(1179);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1180);
        return px_null();
    }
    px_srcline(1181);
    if (px_is_truthy(px_eq(_v1665, px_str("ExprStmt")))) {
        px_srcline(1183);
        _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
        px_srcline(1184);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1663, px_int(1LL)), _v1667, _v1664}, 3));
        px_srcline(1185);
        return px_null();
    }
    px_srcline(1186);
    if (px_is_truthy(px_eq(_v1665, px_str("If")))) {
        px_srcline(1187);
        (void)(px_call(px_get_global("bc_emit_if"), (LXValue[]){_v1663, _v1664}, 2));
        px_srcline(1188);
        return px_null();
    }
    px_srcline(1189);
    if (px_is_truthy(px_eq(_v1665, px_str("While")))) {
        px_srcline(1190);
        (void)(px_call(px_get_global("bc_emit_while"), (LXValue[]){_v1663, _v1664}, 2));
        px_srcline(1191);
        return px_null();
    }
    px_srcline(1192);
    if (px_is_truthy(px_eq(_v1665, px_str("For")))) {
        px_srcline(1193);
        (void)(px_call(px_get_global("bc_emit_for"), (LXValue[]){_v1663, _v1664}, 2));
        px_srcline(1194);
        return px_null();
    }
    px_srcline(1195);
    if (px_is_truthy(px_eq(_v1665, px_str("Break")))) {
        px_srcline(1197);
        _v1675 = px_index(_v1664, px_str("loops"));
        px_srcline(1198);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1675}, 1), px_int(0LL)))) {
            px_srcline(1199);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Break 不在循环内")}, 1));
        }
        px_srcline(1200);
        _v1676 = px_index(_v1675, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1675}, 1), px_int(1LL)));
        px_srcline(1201);
        _v1677 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1664, px_str("bc"))}, 1);
        px_srcline(1202);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1203);
        (void)(px_method(px_index(_v1676, px_str("breaks")), "append", (LXValue[]){_v1677}, 1));
        px_srcline(1204);
        return px_null();
    }
    px_srcline(1205);
    if (px_is_truthy(px_eq(_v1665, px_str("Continue")))) {
        px_srcline(1207);
        _v1675 = px_index(_v1664, px_str("loops"));
        px_srcline(1208);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1675}, 1), px_int(0LL)))) {
            px_srcline(1209);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Continue 不在循环内")}, 1));
        }
        px_srcline(1210);
        _v1676 = px_index(_v1675, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1675}, 1), px_int(1LL)));
        px_srcline(1211);
        _v1677 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1664, px_str("bc"))}, 1);
        px_srcline(1212);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1213);
        (void)(px_method(px_index(_v1676, px_str("conts")), "append", (LXValue[]){_v1677}, 1));
        px_srcline(1214);
        return px_null();
    }
    px_srcline(1215);
    if (px_is_truthy(px_eq(_v1665, px_str("Empty")))) {
        px_srcline(1216);
        return px_null();
    }
    px_srcline(1217);
    if (px_is_truthy(px_eq(_v1665, px_str("TypeConst")))) {
        px_srcline(1219);
        return px_null();
    }
    px_srcline(1220);
    if (px_is_truthy(px_eq(_v1665, px_str("ChanDecl")))) {
        px_srcline(1222);
        _v1666 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1663, px_int(1LL))}, 1);
        px_srcline(1223);
        _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
        px_srcline(1224);
        _v1678 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
        px_srcline(1225);
        _v1679 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan")}, 2);
        px_srcline(1226);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("GETG"), _v1678, _v1679, px_int(0LL)}, 5));
        px_srcline(1227);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("CALL"), _v1667, _v1678, px_int(0LL)}, 5));
        px_srcline(1228);
        if (px_is_truthy(px_index(_v1664, px_str("is_top")))) {
            px_srcline(1229);
            _v1668 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1666}, 2);
            px_srcline(1230);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("SETG"), _v1668, _v1667, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1232);
            _v1669 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1664, _v1666}, 2);
            px_srcline(1233);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("MOV"), _v1669, _v1667, px_int(0LL)}, 5));
        }
        px_srcline(1234);
        return px_null();
    }
    px_srcline(1235);
    if (px_is_truthy(px_eq(_v1665, px_str("Send")))) {
        px_srcline(1237);
        _v1680 = px_list_n((LXValue[]){px_index(_v1663, px_int(2LL))}, 1);
        px_srcline(1238);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1663, px_int(1LL)), px_str("send"), _v1680, px_neg(px_int(1LL)), _v1664}, 5));
        px_srcline(1239);
        return px_null();
    }
    px_srcline(1240);
    if (px_is_truthy(px_eq(_v1665, px_str("Recv")))) {
        px_srcline(1242);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1663, px_int(1LL)), px_str("recv"), px_list_n((LXValue[]){}, 0), px_neg(px_int(1LL)), _v1664}, 5));
        px_srcline(1243);
        return px_null();
    }
    px_srcline(1244);
    if (px_is_truthy(px_eq(_v1665, px_str("Spawn")))) {
        px_srcline(1247);
        _v1681 = px_index(_v1663, px_int(1LL));
        px_srcline(1248);
        if (px_is_truthy(({ LXValue _t1691 = px_eq(px_index(_v1681, px_int(0LL)), px_str("Call")); px_is_truthy(_t1691) ? px_eq(px_index(px_index(_v1681, px_int(1LL)), px_int(0LL)), px_str("Var")) : _t1691; }))) {
            px_srcline(1249);
            _v1682 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1681, px_int(1LL)), px_int(1LL))}, 1);
            px_srcline(1250);
            _v1683 = px_index(_v1681, px_int(2LL));
            px_srcline(1251);
            _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
            px_srcline(1252);
            _v1678 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
            px_srcline(1253);
            _v1684 = px_add(px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1683}, 1));
            px_srcline(1254);
            _v1685 = px_add(px_add(_v1678, px_int(1LL)), _v1684);
            px_srcline(1255);
            while (px_is_truthy(px_lt(px_index(_v1664, px_str("next_slot")), _v1685))) {
                px_srcline(1256);
                (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1));
            }
            px_srcline(1257);
            _v1679 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("spawn")}, 2);
            px_srcline(1258);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("GETG"), _v1678, _v1679, px_int(0LL)}, 5));
            px_srcline(1259);
            _v1686 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1682}, 5);
            px_srcline(1260);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("LOADK"), px_add(_v1678, px_int(1LL)), _v1686, px_int(0LL)}, 5));
            px_srcline(1261);
            _v1687 = px_int(0LL);
            px_srcline(1262);
            while (px_is_truthy(px_lt(_v1687, px_call(px_get_global("len"), (LXValue[]){_v1683}, 1)))) {
                px_srcline(1263);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1683, _v1687), px_add(px_add(_v1678, px_int(2LL)), _v1687), _v1664}, 3));
                px_srcline(1264);
                 _v1687 = px_add(_v1687, px_int(1LL));
            }
            px_srcline(1265);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("CALL"), _v1667, _v1678, _v1684}, 5));
            px_srcline(1266);
            return px_null();
        }
        px_srcline(1271);
        _v1688 = px_index(_v1663, px_int(2LL));
        px_srcline(1272);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1688, px_int(0LL))}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1688, px_int(1LL))}, 1)), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()），多行匿名函数体见 M118"))}, 1));
        px_srcline(1273);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(1274);
    if (px_is_truthy(px_eq(_v1665, px_str("Select")))) {
        px_srcline(1275);
        (void)(px_call(px_get_global("bc_emit_select"), (LXValue[]){_v1663, _v1664}, 2));
        px_srcline(1276);
        return px_null();
    }
    px_srcline(1277);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1663}, 1))}, 1));
px_err_1689:
    if (px_err_1689_proped) return px_err_1689_val;
    return px_null();
}

static LXValue fn_bc_emit_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_select");
    LXValue _v1692 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1693 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1694 = px_null();
    LXValue _v1695 = px_null();
    LXValue _v1696 = px_null();
    LXValue _v1697 = px_null();
    LXValue _v1698 = px_null();
    LXValue _v1699 = px_null();
    LXValue _v1700 = px_null();
    LXValue _v1701 = px_null();
    LXValue _v1702 = px_null();
    LXValue _v1703 = px_null();
    LXValue _v1704 = px_null();
    LXValue _v1705 = px_null();
    LXValue _v1706 = px_null();
    LXValue _v1707 = px_null();
    LXValue _v1708 = px_null();
    LXValue _v1709 = px_null();
    LXValue _v1710 = px_null();
    LXValue _v1711 = px_null();
    LXValue _v1712 = px_null();
    LXValue _v1713 = px_null();
    LXValue _v1714 = px_null();
    LXValue _v1715 = px_null();
    LXValue _v1716 = px_null();
    LXValue _v1717 = px_null();
    LXValue _v1718 = px_null();
    LXValue px_err_1719_val = px_null();
    int px_err_1719_proped = 0;
    px_srcline(1285);
    _v1694 = px_index(_v1692, px_int(1LL));
    px_srcline(1286);
    _v1695 = px_index(_v1692, px_int(2LL));
    px_srcline(1287);
    _v1696 = px_call(px_get_global("len"), (LXValue[]){_v1694}, 1);
    px_srcline(1289);
    _v1697 = px_list_n((LXValue[]){}, 0);
    px_srcline(1290);
    _v1698 = px_int(0LL);
    px_srcline(1291);
    while (px_is_truthy(px_lt(_v1698, _v1696))) {
        px_srcline(1292);
        _v1699 = px_index(px_index(_v1694, _v1698), px_int(1LL));
        px_srcline(1294);
        if (px_is_truthy(({ LXValue _t1720 = px_ne(px_index(_v1699, px_int(0LL)), px_str("Call")); px_is_truthy(_t1720) ? _t1720 : px_ne(px_index(px_index(_v1699, px_int(1LL)), px_int(0LL)), px_str("Field")); }))) {
            px_srcline(1295);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit select case 仅支持 ch.recv(): "), px_call(px_get_global("str"), (LXValue[]){_v1699}, 1))}, 1));
        }
        px_srcline(1296);
        _v1700 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1693}, 1);
        px_srcline(1297);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1699, px_int(1LL)), px_int(1LL)), _v1700, _v1693}, 3));
        px_srcline(1298);
        (void)(px_method(_v1697, "append", (LXValue[]){_v1700}, 1));
        px_srcline(1299);
         _v1698 = px_add(_v1698, px_int(1LL));
    }
    px_srcline(1300);
    _v1701 = px_neg(px_int(1LL));
    px_srcline(1301);
    if (px_is_truthy(px_eq(_v1695, px_null()))) {
        px_srcline(1302);
         _v1701 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1);
    }
    px_srcline(1303);
    _v1702 = px_neg(px_int(1LL));
    px_srcline(1304);
    _v1703 = px_list_n((LXValue[]){}, 0);
    px_srcline(1305);
    _v1704 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan_try_recv")}, 2);
    px_srcline(1306);
    _v1705 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(1307);
     _v1698 = px_int(0LL);
    px_srcline(1308);
    while (px_is_truthy(px_lt(_v1698, _v1696))) {
        px_srcline(1309);
        if (px_is_truthy(px_ge(_v1702, px_int(0LL)))) {
            px_srcline(1310);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1693, _v1702, px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1312);
        _v1706 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1693}, 1);
        px_srcline(1313);
        _v1707 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1693}, 1);
        px_srcline(1314);
        _v1708 = px_add(_v1706, px_int(2LL));
        px_srcline(1315);
        while (px_is_truthy(px_lt(px_index(_v1693, px_str("next_slot")), _v1708))) {
            px_srcline(1316);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1693}, 1));
        }
        px_srcline(1317);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("GETG"), _v1706, _v1704, px_int(0LL)}, 5));
        px_srcline(1318);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("MOV"), px_add(_v1706, px_int(1LL)), px_index(_v1697, _v1698), px_int(0LL)}, 5));
        px_srcline(1319);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("CALL"), _v1707, _v1706, px_int(1LL)}, 5));
        px_srcline(1321);
        _v1709 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1693}, 1);
        px_srcline(1322);
        _v1710 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1693}, 1);
        px_srcline(1323);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("LOADK"), _v1710, _v1705, px_int(0LL)}, 5));
        px_srcline(1324);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("EQ"), _v1709, _v1707, _v1710}, 5));
        px_srcline(1325);
        _v1711 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1);
        px_srcline(1326);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("JMPT"), _v1709, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1327);
         _v1702 = _v1711;
        px_srcline(1329);
        _v1712 = px_index(px_index(_v1694, _v1698), px_int(0LL));
        px_srcline(1330);
        if (px_is_truthy(px_ne(_v1712, px_null()))) {
            px_srcline(1331);
            _v1713 = px_call(px_get_global("rust_unescape"), (LXValue[]){_v1712}, 1);
            px_srcline(1332);
            _v1714 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1693, _v1713}, 2);
            px_srcline(1333);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("MOV"), _v1714, _v1707, px_int(0LL)}, 5));
        }
        px_srcline(1334);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(px_index(_v1694, _v1698), px_int(2LL)), _v1693}, 2));
        px_srcline(1335);
        _v1715 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1);
        px_srcline(1336);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1337);
        (void)(px_method(_v1703, "append", (LXValue[]){_v1715}, 1));
        px_srcline(1338);
         _v1698 = px_add(_v1698, px_int(1LL));
    }
    px_srcline(1340);
    if (px_is_truthy(px_ge(_v1702, px_int(0LL)))) {
        px_srcline(1341);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1693, _v1702, px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1)}, 3));
    }
    px_srcline(1342);
    if (px_is_truthy(px_ne(_v1695, px_null()))) {
        px_srcline(1343);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1695, _v1693}, 2));
    }
    else {
        px_srcline(1345);
        _v1716 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1);
        px_srcline(1346);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1693, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1347);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1693, _v1716, _v1701}, 3));
    }
    px_srcline(1348);
    _v1717 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1693, px_str("bc"))}, 1);
    px_srcline(1349);
    _v1718 = px_int(0LL);
    px_srcline(1350);
    while (px_is_truthy(px_lt(_v1718, px_call(px_get_global("len"), (LXValue[]){_v1703}, 1)))) {
        px_srcline(1351);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1693, px_index(_v1703, _v1718), _v1717}, 3));
        px_srcline(1352);
         _v1718 = px_add(_v1718, px_int(1LL));
    }
px_err_1719:
    if (px_err_1719_proped) return px_err_1719_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt");
    LXValue _v1721 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1722 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1723 = px_null();
    LXValue _v1724 = px_null();
    LXValue px_err_1725_val = px_null();
    int px_err_1725_proped = 0;
    px_srcline(1355);
    _v1723 = px_int(0LL);
    px_srcline(1356);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1721}, 1), px_int(0LL)))) {
        px_srcline(1357);
        _v1724 = px_index(_v1721, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1721}, 1), px_int(1LL)));
        px_srcline(1358);
        if (px_is_truthy(({ LXValue _t1727 = ({ LXValue _t1726 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1724}, 1), px_str("list")); px_is_truthy(_t1726) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1724}, 1), px_int(1LL)) : _t1726; }); px_is_truthy(_t1727) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1724, px_int(0LL))}, 1), px_str("int")) : _t1727; }))) {
            px_srcline(1359);
             _v1723 = px_index(_v1724, px_int(0LL));
        }
    }
    px_srcline(1360);
    if (px_is_truthy(px_gt(_v1723, px_int(0LL)))) {
        px_srcline(1361);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1722, px_str("SRCLINE"), px_int(0LL), _v1723, px_int(0LL)}, 5));
    }
    px_srcline(1362);
    (void)(px_call(px_get_global("bc_emit_stmt_inner"), (LXValue[]){_v1721, _v1722}, 2));
px_err_1725:
    if (px_err_1725_proped) return px_err_1725_val;
    return px_null();
}

static LXValue fn_bc_emit_stmts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmts");
    LXValue _v1728 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1729 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1730 = px_null();
    LXValue px_err_1731_val = px_null();
    int px_err_1731_proped = 0;
    px_srcline(1364);
    _v1730 = px_int(0LL);
    px_srcline(1365);
    while (px_is_truthy(px_lt(_v1730, px_call(px_get_global("len"), (LXValue[]){_v1728}, 1)))) {
        px_srcline(1366);
        (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){px_index(_v1728, _v1730), _v1729}, 2));
        px_srcline(1367);
         _v1730 = px_add(_v1730, px_int(1LL));
    }
px_err_1731:
    if (px_err_1731_proped) return px_err_1731_val;
    return px_null();
}

static LXValue fn_bc_emit_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_if");
    LXValue _v1732 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1733 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1734 = px_null();
    LXValue _v1735 = px_null();
    LXValue _v1736 = px_null();
    LXValue _v1737 = px_null();
    LXValue _v1738 = px_null();
    LXValue _v1739 = px_null();
    LXValue _v1740 = px_null();
    LXValue _v1741 = px_null();
    LXValue _v1742 = px_null();
    LXValue _v1743 = px_null();
    LXValue _v1744 = px_null();
    LXValue _v1745 = px_null();
    LXValue px_err_1746_val = px_null();
    int px_err_1746_proped = 0;
    px_srcline(1373);
    _v1734 = px_index(_v1732, px_int(1LL));
    px_srcline(1374);
    _v1735 = px_index(_v1732, px_int(2LL));
    px_srcline(1375);
    _v1736 = px_call(px_get_global("len"), (LXValue[]){_v1734}, 1);
    px_srcline(1376);
    _v1737 = px_list_n((LXValue[]){}, 0);
    px_srcline(1377);
    _v1738 = px_int(0LL);
    px_srcline(1378);
    while (px_is_truthy(px_lt(_v1738, _v1736))) {
        px_srcline(1379);
        _v1739 = px_index(px_index(_v1734, _v1738), px_int(0LL));
        px_srcline(1380);
        _v1740 = px_index(px_index(_v1734, _v1738), px_int(1LL));
        px_srcline(1381);
        _v1741 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1733}, 1);
        px_srcline(1382);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1739, _v1741, _v1733}, 3));
        px_srcline(1383);
        _v1742 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1733, px_str("bc"))}, 1);
        px_srcline(1384);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1733, px_str("JMPF"), _v1741, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1385);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1740, _v1733}, 2));
        px_srcline(1386);
        if (px_is_truthy(({ LXValue _t1747 = px_lt(_v1738, px_sub(_v1736, px_int(1LL))); px_is_truthy(_t1747) ? _t1747 : px_ne(_v1735, px_null()); }))) {
            px_srcline(1389);
            _v1743 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1733, px_str("bc"))}, 1);
            px_srcline(1390);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1733, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1391);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1733, _v1742, px_call(px_get_global("len"), (LXValue[]){px_index(_v1733, px_str("bc"))}, 1)}, 3));
            px_srcline(1392);
            (void)(px_method(_v1737, "append", (LXValue[]){_v1743}, 1));
        }
        else {
            px_srcline(1395);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1733, _v1742, px_call(px_get_global("len"), (LXValue[]){px_index(_v1733, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1396);
         _v1738 = px_add(_v1738, px_int(1LL));
    }
    px_srcline(1397);
    if (px_is_truthy(px_ne(_v1735, px_null()))) {
        px_srcline(1398);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1735, _v1733}, 2));
    }
    px_srcline(1399);
    _v1744 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1733, px_str("bc"))}, 1);
    px_srcline(1400);
    _v1745 = px_int(0LL);
    px_srcline(1401);
    while (px_is_truthy(px_lt(_v1745, px_call(px_get_global("len"), (LXValue[]){_v1737}, 1)))) {
        px_srcline(1402);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1733, px_index(_v1737, _v1745), _v1744}, 3));
        px_srcline(1403);
         _v1745 = px_add(_v1745, px_int(1LL));
    }
px_err_1746:
    if (px_err_1746_proped) return px_err_1746_val;
    return px_null();
}

static LXValue fn_bc_emit_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_while");
    LXValue _v1748 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1749 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1750 = px_null();
    LXValue _v1751 = px_null();
    LXValue _v1752 = px_null();
    LXValue _v1753 = px_null();
    LXValue _v1754 = px_null();
    LXValue _v1755 = px_null();
    LXValue _v1756 = px_null();
    LXValue _v1757 = px_null();
    LXValue _v1758 = px_null();
    LXValue px_err_1759_val = px_null();
    int px_err_1759_proped = 0;
    px_srcline(1408);
    _v1750 = px_index(_v1748, px_int(1LL));
    px_srcline(1409);
    _v1751 = px_index(_v1748, px_int(2LL));
    px_srcline(1410);
    _v1752 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1749, px_str("bc"))}, 1);
    px_srcline(1411);
    _v1753 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1749}, 1);
    px_srcline(1412);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1750, _v1753, _v1749}, 3));
    px_srcline(1413);
    _v1754 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1749, px_str("bc"))}, 1);
    px_srcline(1414);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1749, px_str("JMPF"), _v1753, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1415);
    _v1755 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1416);
    px_index_set(_v1755, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1417);
    px_index_set(_v1755, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1418);
    (void)(px_method(px_index(_v1749, px_str("loops")), "push", (LXValue[]){_v1755}, 1));
    px_srcline(1419);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1751, _v1749}, 2));
    px_srcline(1420);
    (void)(px_method(px_index(_v1749, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1421);
    _v1756 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1749, px_str("bc"))}, 1);
    px_srcline(1422);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1749, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1423);
    _v1757 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1749, px_str("bc"))}, 1);
    px_srcline(1424);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1749, _v1756, _v1752}, 3));
    px_srcline(1425);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1749, _v1754, _v1757}, 3));
    px_srcline(1426);
    _v1758 = px_int(0LL);
    px_srcline(1427);
    while (px_is_truthy(px_lt(_v1758, px_call(px_get_global("len"), (LXValue[]){px_index(_v1755, px_str("breaks"))}, 1)))) {
        px_srcline(1428);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1749, px_index(px_index(_v1755, px_str("breaks")), _v1758), _v1757}, 3));
        px_srcline(1429);
         _v1758 = px_add(_v1758, px_int(1LL));
    }
    px_srcline(1430);
     _v1758 = px_int(0LL);
    px_srcline(1431);
    while (px_is_truthy(px_lt(_v1758, px_call(px_get_global("len"), (LXValue[]){px_index(_v1755, px_str("conts"))}, 1)))) {
        px_srcline(1432);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1749, px_index(px_index(_v1755, px_str("conts")), _v1758), _v1752}, 3));
        px_srcline(1433);
         _v1758 = px_add(_v1758, px_int(1LL));
    }
px_err_1759:
    if (px_err_1759_proped) return px_err_1759_val;
    return px_null();
}

static LXValue fn_bc_emit_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_for");
    LXValue _v1760 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1761 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1762 = px_null();
    LXValue _v1763 = px_null();
    LXValue _v1764 = px_null();
    LXValue _v1765 = px_null();
    LXValue _v1766 = px_null();
    LXValue _v1767 = px_null();
    LXValue _v1768 = px_null();
    LXValue _v1769 = px_null();
    LXValue _v1770 = px_null();
    LXValue _v1771 = px_null();
    LXValue _v1772 = px_null();
    LXValue _v1773 = px_null();
    LXValue _v1774 = px_null();
    LXValue _v1775 = px_null();
    LXValue _v1776 = px_null();
    LXValue _v1777 = px_null();
    LXValue _v1778 = px_null();
    LXValue _v1779 = px_null();
    LXValue px_err_1780_val = px_null();
    int px_err_1780_proped = 0;
    px_srcline(1440);
    _v1762 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1760, px_int(1LL))}, 1);
    px_srcline(1441);
    _v1763 = px_index(_v1760, px_int(2LL));
    px_srcline(1442);
    _v1764 = px_index(_v1760, px_int(3LL));
    px_srcline(1443);
    _v1765 = px_neg(px_int(1LL));
    px_srcline(1444);
    if (px_is_truthy(px_method(px_index(_v1761, px_str("smap")), "has", (LXValue[]){_v1762}, 1))) {
        px_srcline(1445);
         _v1765 = px_index(px_index(_v1761, px_str("smap")), _v1762);
    }
    else {
        px_srcline(1447);
         _v1765 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1761, _v1762}, 2);
    }
    px_srcline(1449);
    _v1766 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1);
    px_srcline(1450);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1763, _v1766, _v1761}, 3));
    px_srcline(1452);
    _v1767 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1);
    px_srcline(1453);
    _v1768 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1);
    px_srcline(1454);
    while (px_is_truthy(px_lt(px_index(_v1761, px_str("next_slot")), px_add(_v1767, px_int(2LL))))) {
        px_srcline(1455);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1));
    }
    px_srcline(1456);
    _v1769 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1457);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("GETG"), _v1767, _v1769, px_int(0LL)}, 5));
    px_srcline(1458);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("MOV"), px_add(_v1767, px_int(1LL)), _v1766, px_int(0LL)}, 5));
    px_srcline(1459);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("CALL"), _v1768, _v1767, px_int(1LL)}, 5));
    px_srcline(1461);
    _v1770 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1);
    px_srcline(1462);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1761, _v1770, px_int(0LL)}, 3));
    px_srcline(1463);
    _v1771 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1761, px_str("bc"))}, 1);
    px_srcline(1465);
    _v1772 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1);
    px_srcline(1466);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("LT"), _v1772, _v1770, _v1768}, 5));
    px_srcline(1467);
    _v1773 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1761, px_str("bc"))}, 1);
    px_srcline(1468);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("JMPF"), _v1772, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1470);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("INDEX"), _v1765, _v1766, _v1770}, 5));
    px_srcline(1471);
    _v1774 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1472);
    px_index_set(_v1774, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1473);
    px_index_set(_v1774, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1474);
    (void)(px_method(px_index(_v1761, px_str("loops")), "push", (LXValue[]){_v1774}, 1));
    px_srcline(1475);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1764, _v1761}, 2));
    px_srcline(1476);
    (void)(px_method(px_index(_v1761, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1481);
    _v1775 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1761, px_str("bc"))}, 1);
    px_srcline(1482);
    _v1776 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1761}, 1);
    px_srcline(1483);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1761, _v1776, px_int(1LL)}, 3));
    px_srcline(1484);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("ADD"), _v1770, _v1770, _v1776}, 5));
    px_srcline(1485);
    _v1777 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1761, px_str("bc"))}, 1);
    px_srcline(1486);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1761, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1487);
    _v1778 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1761, px_str("bc"))}, 1);
    px_srcline(1488);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1761, _v1777, _v1771}, 3));
    px_srcline(1489);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1761, _v1773, _v1778}, 3));
    px_srcline(1490);
    _v1779 = px_int(0LL);
    px_srcline(1491);
    while (px_is_truthy(px_lt(_v1779, px_call(px_get_global("len"), (LXValue[]){px_index(_v1774, px_str("breaks"))}, 1)))) {
        px_srcline(1492);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1761, px_index(px_index(_v1774, px_str("breaks")), _v1779), _v1778}, 3));
        px_srcline(1493);
         _v1779 = px_add(_v1779, px_int(1LL));
    }
    px_srcline(1494);
     _v1779 = px_int(0LL);
    px_srcline(1495);
    while (px_is_truthy(px_lt(_v1779, px_call(px_get_global("len"), (LXValue[]){px_index(_v1774, px_str("conts"))}, 1)))) {
        px_srcline(1496);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1761, px_index(px_index(_v1774, px_str("conts")), _v1779), _v1775}, 3));
        px_srcline(1497);
         _v1779 = px_add(_v1779, px_int(1LL));
    }
px_err_1780:
    if (px_err_1780_proped) return px_err_1780_val;
    return px_null();
}

static LXValue fn_bc_emit_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_def");
    LXValue _v1781 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1782 = px_null();
    LXValue px_err_1783_val = px_null();
    int px_err_1783_proped = 0;
    px_srcline(1501);
    _v1782 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1781, px_int(1LL))}, 1);
    px_srcline(1502);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v1781, _v1782}, 2);
px_err_1783:
    if (px_err_1783_proped) return px_err_1783_val;
    return px_null();
}

static LXValue fn_bc_emit_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_impl_def");
    LXValue _v1784 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1785 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1786 = px_null();
    LXValue px_err_1787_val = px_null();
    int px_err_1787_proped = 0;
    px_srcline(1505);
    _v1786 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1784, px_int(1LL))}, 1);
    px_srcline(1506);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v1784, px_add(px_add(_v1785, px_str(".")), _v1786)}, 2);
px_err_1787:
    if (px_err_1787_proped) return px_err_1787_val;
    return px_null();
}

static LXValue fn_bc_emit_func_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_body");
    LXValue _v1788 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1789 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1790 = px_null();
    LXValue _v1791 = px_null();
    LXValue _v1792 = px_null();
    LXValue _v1793 = px_null();
    LXValue _v1794 = px_null();
    LXValue _v1795 = px_null();
    LXValue _v1796 = px_null();
    LXValue _v1797 = px_null();
    LXValue _v1798 = px_null();
    LXValue px_err_1799_val = px_null();
    int px_err_1799_proped = 0;
    px_srcline(1515);
    _v1790 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(1516);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1790}, 1));
    px_srcline(1517);
    _v1791 = px_index(_v1788, px_int(2LL));
    px_srcline(1522);
    _v1792 = px_call(px_get_global("len"), (LXValue[]){_v1791}, 1);
    px_srcline(1523);
    _v1793 = px_int(0LL);
    px_srcline(1524);
    _v1794 = px_int(0LL);
    px_srcline(1525);
    while (px_is_truthy(px_lt(_v1794, px_call(px_get_global("len"), (LXValue[]){_v1791}, 1)))) {
        px_srcline(1526);
        if (px_is_truthy(px_ne(px_index(px_index(_v1791, _v1794), px_int(3LL)), px_null()))) {
            px_srcline(1527);
            if (px_is_truthy(px_eq(_v1793, px_int(0LL)))) {
                px_srcline(1528);
                 _v1792 = _v1794;
            }
            px_srcline(1529);
             _v1793 = px_add(_v1793, px_int(1LL));
        }
        px_srcline(1530);
         _v1794 = px_add(_v1794, px_int(1LL));
    }
    px_srcline(1531);
    _v1795 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1789, _v1792}, 2);
    px_srcline(1532);
    px_index_set(_v1795, px_str("ndefault"), _v1793);
    px_srcline(1534);
     _v1794 = px_int(0LL);
    px_srcline(1535);
    while (px_is_truthy(px_lt(_v1794, px_call(px_get_global("len"), (LXValue[]){_v1791}, 1)))) {
        px_srcline(1536);
        px_index_set(px_index(_v1795, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1791, _v1794), px_int(1LL))}, 1), _v1794);
        px_srcline(1537);
         _v1794 = px_add(_v1794, px_int(1LL));
    }
    px_srcline(1538);
    if (px_is_truthy(px_lt(px_index(_v1795, px_str("next_slot")), px_call(px_get_global("len"), (LXValue[]){_v1791}, 1)))) {
        px_srcline(1539);
        px_index_set(_v1795, px_str("next_slot"), px_call(px_get_global("len"), (LXValue[]){_v1791}, 1));
    }
    px_srcline(1541);
    _v1796 = px_list_n((LXValue[]){}, 0);
    px_srcline(1542);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1788, px_int(4LL)), _v1796}, 2));
    px_srcline(1543);
    _v1797 = px_int(0LL);
    px_srcline(1544);
    while (px_is_truthy(px_lt(_v1797, px_call(px_get_global("len"), (LXValue[]){_v1796}, 1)))) {
        px_srcline(1545);
        _v1798 = px_index(_v1796, _v1797);
        px_srcline(1546);
        if (px_is_truthy(px_method(px_index(_v1795, px_str("smap")), "has", (LXValue[]){_v1798}, 1))) {
            px_srcline(1547);
             _v1797 = px_add(_v1797, px_int(1LL));
            px_srcline(1548);
            continue;
        }
        px_srcline(1549);
        if (px_is_truthy(px_call(px_get_global("bc_is_global"), (LXValue[]){_v1798}, 1))) {
            px_srcline(1550);
             _v1797 = px_add(_v1797, px_int(1LL));
            px_srcline(1551);
            continue;
        }
        px_srcline(1552);
        (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v1795, _v1798}, 2));
        px_srcline(1553);
         _v1797 = px_add(_v1797, px_int(1LL));
    }
    px_srcline(1555);
    if (px_is_truthy(px_gt(_v1793, px_int(0LL)))) {
        px_srcline(1556);
        (void)(px_call(px_get_global("bc_emit_default_fill"), (LXValue[]){_v1795, _v1791, _v1792}, 3));
    }
    px_srcline(1557);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(_v1788, px_int(4LL)), _v1795}, 2));
    px_srcline(1558);
    px_set_global("cg_immutables", _v1790);
    px_srcline(1559);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1795, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1560);
    px_index_set(_v1795, px_str("nslots"), px_index(_v1795, px_str("next_slot")));
    px_srcline(1561);
    return _v1795;
px_err_1799:
    if (px_err_1799_proped) return px_err_1799_val;
    return px_null();
}

static LXValue fn_bc_emit_default_fill(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_default_fill");
    LXValue _v1800 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1801 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1802 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1803 = px_null();
    LXValue _v1804 = px_null();
    LXValue _v1805 = px_null();
    LXValue _v1806 = px_null();
    LXValue _v1807 = px_null();
    LXValue px_err_1808_val = px_null();
    int px_err_1808_proped = 0;
    px_srcline(1567);
    _v1803 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1800}, 1);
    px_srcline(1568);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1800, px_str("NARGS"), _v1803, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1569);
    _v1804 = _v1802;
    px_srcline(1570);
    while (px_is_truthy(px_lt(_v1804, px_call(px_get_global("len"), (LXValue[]){_v1801}, 1)))) {
        px_srcline(1571);
        if (px_is_truthy(px_ne(px_index(px_index(_v1801, _v1804), px_int(3LL)), px_null()))) {
            px_srcline(1572);
            _v1805 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1800}, 1);
            px_srcline(1573);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1800, _v1805, _v1804}, 3));
            px_srcline(1574);
            _v1806 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1800}, 1);
            px_srcline(1575);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1800, px_str("LT"), _v1806, _v1805, _v1803}, 5));
            px_srcline(1576);
            _v1807 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1800, px_str("bc"))}, 1);
            px_srcline(1577);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1800, px_str("JMPT"), _v1806, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1578);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1801, _v1804), px_int(3LL)), _v1804, _v1800}, 3));
            px_srcline(1579);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1800, _v1807, px_call(px_get_global("len"), (LXValue[]){px_index(_v1800, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1580);
         _v1804 = px_add(_v1804, px_int(1LL));
    }
px_err_1808:
    if (px_err_1808_proped) return px_err_1808_val;
    return px_null();
}

static LXValue fn_bc_emit_func_top(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_top");
    LXValue _v1809 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1810 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1811 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1812 = px_null();
    LXValue _v1813 = px_null();
    LXValue _v1814 = px_null();
    LXValue _v1815 = px_null();
    LXValue px_err_1816_val = px_null();
    int px_err_1816_proped = 0;
    px_srcline(1585);
    _v1812 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1810, px_int(0LL)}, 2);
    px_srcline(1586);
    px_index_set(_v1812, px_str("is_top"), px_bool(true));
    px_srcline(1587);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1809, _v1812}, 2));
    px_srcline(1588);
    if (px_is_truthy(_v1811)) {
        px_srcline(1589);
        _v1813 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1);
        px_srcline(1590);
        _v1814 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("main")}, 2);
        px_srcline(1591);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("GETG"), _v1813, _v1814, px_int(0LL)}, 5));
        px_srcline(1592);
        _v1815 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1);
        px_srcline(1593);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("CALL"), _v1815, _v1813, px_int(0LL)}, 5));
        px_srcline(1594);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("RET"), _v1815, px_int(0LL), px_int(0LL)}, 5));
    }
    else {
        px_srcline(1596);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("HALT"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    }
    px_srcline(1597);
    px_index_set(_v1812, px_str("nslots"), px_index(_v1812, px_str("next_slot")));
    px_srcline(1598);
    return _v1812;
px_err_1816:
    if (px_err_1816_proped) return px_err_1816_val;
    return px_null();
}

static LXValue fn_bc_emit_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_program");
    LXValue _v1817 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1818 = px_null();
    LXValue _v1819 = px_null();
    LXValue _v1820 = px_null();
    LXValue _v1821 = px_null();
    LXValue _v1822 = px_null();
    LXValue _v1823 = px_null();
    LXValue _v1824 = px_null();
    LXValue _v1825 = px_null();
    LXValue _v1826 = px_null();
    LXValue _v1827 = px_null();
    LXValue _v1828 = px_null();
    LXValue _v1829 = px_null();
    LXValue _v1830 = px_null();
    LXValue _v1831 = px_null();
    LXValue _v1832 = px_null();
    LXValue _v1833 = px_null();
    LXValue _v1834 = px_null();
    LXValue px_err_1835_val = px_null();
    int px_err_1835_proped = 0;
    px_srcline(1602);
    _v1818 = px_index(_v1817, px_int(1LL));
    px_srcline(1603);
    _v1819 = px_call(px_get_global("bc_new_module"), (LXValue[]){px_str("<module>")}, 1);
    px_srcline(1604);
    px_set_global("g_bcm", _v1819);
    px_srcline(1607);
    _v1820 = px_int(0LL);
    px_srcline(1608);
    while (px_is_truthy(px_lt(_v1820, px_call(px_get_global("len"), (LXValue[]){_v1818}, 1)))) {
        px_srcline(1609);
        _v1821 = px_index(_v1818, _v1820);
        px_srcline(1610);
        _v1822 = px_index(_v1821, px_int(0LL));
        px_srcline(1611);
        if (px_is_truthy(px_eq(_v1822, px_str("StructDef")))) {
            px_srcline(1612);
            _v1823 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
            px_srcline(1613);
            px_index_set(_v1823, px_str("name"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1821, px_int(1LL))}, 1));
            px_srcline(1614);
            _v1824 = px_list_n((LXValue[]){}, 0);
            px_srcline(1615);
            _v1825 = px_int(0LL);
            px_srcline(1616);
            while (px_is_truthy(px_lt(_v1825, px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_int(2LL))}, 1)))) {
                px_srcline(1617);
                (void)(px_method(_v1824, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1821, px_int(2LL)), _v1825), px_int(1LL))}, 1)}, 1));
                px_srcline(1618);
                 _v1825 = px_add(_v1825, px_int(1LL));
            }
            px_srcline(1619);
            px_index_set(_v1823, px_str("fnames"), _v1824);
            px_srcline(1620);
            (void)(px_method(px_index(_v1819, px_str("structs")), "push", (LXValue[]){_v1823}, 1));
        }
        else if (px_is_truthy(px_eq(_v1822, px_str("EnumDef")))) {
            px_srcline(1622);
            _v1826 = px_list_n((LXValue[]){}, 0);
            px_srcline(1623);
            _v1827 = px_int(0LL);
            px_srcline(1624);
            while (px_is_truthy(px_lt(_v1827, px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_int(2LL))}, 1)))) {
                px_srcline(1625);
                (void)(px_method(_v1826, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1821, px_int(2LL)), _v1827), px_int(1LL))}, 1)}, 1));
                px_srcline(1626);
                 _v1827 = px_add(_v1827, px_int(1LL));
            }
            px_srcline(1627);
            px_index_set(px_index(_v1819, px_str("enums")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1821, px_int(1LL))}, 1), _v1826);
        }
        px_srcline(1628);
         _v1820 = px_add(_v1820, px_int(1LL));
    }
    px_srcline(1629);
    (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){_v1818}, 1));
    px_srcline(1630);
    _v1828 = px_call(px_get_global("bc_collect_impl_list"), (LXValue[]){_v1818}, 1);
    px_srcline(1637);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1638);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1639);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1640);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1641);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1642);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1643);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1644);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1645);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1646);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(1648);
     _v1820 = px_int(0LL);
    px_srcline(1649);
    while (px_is_truthy(px_lt(_v1820, px_call(px_get_global("len"), (LXValue[]){_v1818}, 1)))) {
        px_srcline(1650);
        _v1821 = px_index(_v1818, _v1820);
        px_srcline(1651);
        _v1822 = px_index(_v1821, px_int(0LL));
        px_srcline(1652);
        if (px_is_truthy(px_eq(_v1822, px_str("FuncDef")))) {
            px_srcline(1653);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v1819, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1821, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1822, px_str("ExternDef")))) {
            px_srcline(1660);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1821, px_int(1LL))}, 1), px_index(_v1821, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1822, px_str("VarDecl")))) {
            px_srcline(1662);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v1819, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1821, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1822, px_str("Assign")))) {
            px_srcline(1664);
            _v1829 = px_index(_v1821, px_int(1LL));
            px_srcline(1665);
            if (px_is_truthy(px_eq(px_index(_v1829, px_int(0LL)), px_str("Var")))) {
                px_srcline(1666);
                (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v1819, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1829, px_int(1LL))}, 1)}, 2));
            }
        }
        px_srcline(1667);
         _v1820 = px_add(_v1820, px_int(1LL));
    }
    px_srcline(1670);
     _v1820 = px_int(0LL);
    px_srcline(1671);
    while (px_is_truthy(px_lt(_v1820, px_call(px_get_global("len"), (LXValue[]){_v1828}, 1)))) {
        px_srcline(1672);
        _v1830 = px_index(_v1828, _v1820);
        px_srcline(1673);
        _v1831 = px_call(px_get_global("bc_emit_impl_def"), (LXValue[]){px_index(_v1830, px_int(1LL)), px_index(_v1830, px_int(0LL))}, 2);
        px_srcline(1674);
        (void)(px_method(px_index(_v1819, px_str("funcs")), "push", (LXValue[]){_v1831}, 1));
        px_srcline(1675);
         _v1820 = px_add(_v1820, px_int(1LL));
    }
    px_srcline(1676);
     _v1820 = px_int(0LL);
    px_srcline(1677);
    while (px_is_truthy(px_lt(_v1820, px_call(px_get_global("len"), (LXValue[]){_v1818}, 1)))) {
        px_srcline(1678);
        _v1821 = px_index(_v1818, _v1820);
        px_srcline(1679);
        if (px_is_truthy(px_eq(px_index(_v1821, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(1680);
            _v1831 = px_call(px_get_global("bc_emit_func_def"), (LXValue[]){_v1821}, 1);
            px_srcline(1681);
            (void)(px_method(px_index(_v1819, px_str("funcs")), "push", (LXValue[]){_v1831}, 1));
        }
        px_srcline(1682);
         _v1820 = px_add(_v1820, px_int(1LL));
    }
    px_srcline(1684);
    _v1832 = px_list_n((LXValue[]){}, 0);
    px_srcline(1685);
     _v1820 = px_int(0LL);
    px_srcline(1686);
    while (px_is_truthy(px_lt(_v1820, px_call(px_get_global("len"), (LXValue[]){_v1818}, 1)))) {
        px_srcline(1687);
        _v1821 = px_index(_v1818, _v1820);
        px_srcline(1688);
        _v1822 = px_index(_v1821, px_int(0LL));
        px_srcline(1689);
        if (px_is_truthy(({ LXValue _t1842 = ({ LXValue _t1841 = ({ LXValue _t1840 = ({ LXValue _t1839 = ({ LXValue _t1838 = ({ LXValue _t1837 = ({ LXValue _t1836 = px_ne(_v1822, px_str("FuncDef")); px_is_truthy(_t1836) ? px_ne(_v1822, px_str("StructDef")) : _t1836; }); px_is_truthy(_t1837) ? px_ne(_v1822, px_str("EnumDef")) : _t1837; }); px_is_truthy(_t1838) ? px_ne(_v1822, px_str("TraitDef")) : _t1838; }); px_is_truthy(_t1839) ? px_ne(_v1822, px_str("ImplDef")) : _t1839; }); px_is_truthy(_t1840) ? px_ne(_v1822, px_str("Import")) : _t1840; }); px_is_truthy(_t1841) ? px_ne(_v1822, px_str("ExternDef")) : _t1841; }); px_is_truthy(_t1842) ? px_ne(_v1822, px_str("TypeConst")) : _t1842; }))) {
            px_srcline(1690);
            (void)(px_method(_v1832, "append", (LXValue[]){_v1821}, 1));
        }
        px_srcline(1691);
         _v1820 = px_add(_v1820, px_int(1LL));
    }
    px_srcline(1693);
    _v1833 = px_bool(false);
    px_srcline(1694);
     _v1820 = px_int(0LL);
    px_srcline(1695);
    while (px_is_truthy(px_lt(_v1820, px_call(px_get_global("len"), (LXValue[]){_v1818}, 1)))) {
        px_srcline(1696);
        _v1821 = px_index(_v1818, _v1820);
        px_srcline(1697);
        if (px_is_truthy(({ LXValue _t1843 = px_eq(px_index(_v1821, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1843) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1821, px_int(1LL))}, 1), px_str("main")) : _t1843; }))) {
            px_srcline(1698);
             _v1833 = px_bool(true);
        }
        px_srcline(1699);
         _v1820 = px_add(_v1820, px_int(1LL));
    }
    px_srcline(1700);
    _v1834 = px_call(px_get_global("bc_emit_func_top"), (LXValue[]){_v1832, px_str("<top>"), _v1833}, 3);
    px_srcline(1701);
    (void)(px_method(px_index(_v1819, px_str("funcs")), "push", (LXValue[]){_v1834}, 1));
    px_srcline(1702);
    px_index_set(_v1819, px_str("top"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v1819, px_str("funcs"))}, 1), px_int(1LL)));
    px_srcline(1703);
    return _v1819;
px_err_1835:
    if (px_err_1835_proped) return px_err_1835_val;
    return px_null();
}

static LXValue fn_bc_dump_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_dump_module");
    LXValue _v1844 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1845 = px_null();
    LXValue _v1846 = px_null();
    LXValue _v1847 = px_null();
    LXValue _v1848 = px_null();
    LXValue _v1849 = px_null();
    LXValue _v1850 = px_null();
    LXValue _v1851 = px_null();
    LXValue _v1852 = px_null();
    LXValue px_err_1853_val = px_null();
    int px_err_1853_proped = 0;
    px_srcline(1706);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# BCModule "), px_index(_v1844, px_str("name")))}, 1));
    px_srcline(1707);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# K "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1844, px_str("k_pool"))}, 1)}, 1))}, 1));
    px_srcline(1708);
    _v1845 = px_int(0LL);
    px_srcline(1709);
    while (px_is_truthy(px_lt(_v1845, px_call(px_get_global("len"), (LXValue[]){px_index(_v1844, px_str("k_pool"))}, 1)))) {
        px_srcline(1710);
        _v1846 = px_index(px_index(_v1844, px_str("k_pool")), _v1845);
        px_srcline(1711);
        if (px_is_truthy(px_eq(px_index(_v1846, px_str("kind")), px_str("int")))) {
            px_srcline(1712);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1845}, 1)), px_str(" int ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1846, px_str("i"))}, 1))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1846, px_str("kind")), px_str("float")))) {
            px_srcline(1714);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1845}, 1)), px_str(" float ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1846, px_str("f"))}, 1))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1846, px_str("kind")), px_str("str")))) {
            px_srcline(1716);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1845}, 1)), px_str(" str \"")), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1846, px_str("s"))}, 1)), px_str("\""))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1846, px_str("kind")), px_str("bool")))) {
            px_srcline(1718);
            _v1847 = px_str("false");
            px_srcline(1719);
            if (px_is_truthy(px_ne(px_index(_v1846, px_str("i")), px_int(0LL)))) {
                px_srcline(1720);
                 _v1847 = px_str("true");
            }
            px_srcline(1721);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1845}, 1)), px_str(" bool ")), _v1847)}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1846, px_str("kind")), px_str("func")))) {
            px_srcline(1723);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1845}, 1)), px_str(" func ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1846, px_str("i"))}, 1))}, 1));
        }
        else {
            px_srcline(1725);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1845}, 1)), px_str(" null"))}, 1));
        }
        px_srcline(1726);
         _v1845 = px_add(_v1845, px_int(1LL));
    }
    px_srcline(1727);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# G "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1844, px_str("globals"))}, 1)}, 1))}, 1));
    px_srcline(1728);
    _v1848 = px_int(0LL);
    px_srcline(1729);
    while (px_is_truthy(px_lt(_v1848, px_call(px_get_global("len"), (LXValue[]){px_index(_v1844, px_str("globals"))}, 1)))) {
        px_srcline(1730);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("G "), px_call(px_get_global("str"), (LXValue[]){_v1848}, 1)), px_str(" ")), px_index(px_index(_v1844, px_str("globals")), _v1848))}, 1));
        px_srcline(1731);
         _v1848 = px_add(_v1848, px_int(1LL));
    }
    px_srcline(1732);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("# funcs "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1844, px_str("funcs"))}, 1)}, 1)), px_str(" top=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1844, px_str("top"))}, 1))}, 1));
    px_srcline(1733);
    _v1849 = px_int(0LL);
    px_srcline(1734);
    while (px_is_truthy(px_lt(_v1849, px_call(px_get_global("len"), (LXValue[]){px_index(_v1844, px_str("funcs"))}, 1)))) {
        px_srcline(1735);
        _v1850 = px_index(px_index(_v1844, px_str("funcs")), _v1849);
        px_srcline(1736);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("== func "), px_index(_v1850, px_str("name"))), px_str(" arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1850, px_str("arity"))}, 1)), px_str(" ndefault=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1850, px_str("ndefault"))}, 1)), px_str(" nslots=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1850, px_str("nslots"))}, 1))}, 1));
        px_srcline(1737);
        _v1851 = px_int(0LL);
        px_srcline(1738);
        while (px_is_truthy(px_lt(_v1851, px_call(px_get_global("len"), (LXValue[]){px_index(_v1850, px_str("bc"))}, 1)))) {
            px_srcline(1739);
            _v1852 = px_index(px_index(_v1850, px_str("bc")), _v1851);
            px_srcline(1740);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v1851}, 1), px_str(":")), px_index(_v1852, px_int(0LL))), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1852, px_int(2LL))}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1852, px_int(3LL))}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1852, px_int(4LL))}, 1))}, 1));
            px_srcline(1741);
             _v1851 = px_add(_v1851, px_int(1LL));
        }
        px_srcline(1742);
         _v1849 = px_add(_v1849, px_int(1LL));
    }
px_err_1853:
    if (px_err_1853_proped) return px_err_1853_val;
    return px_null();
}

static LXValue fn_bc_emit_c_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_c_program");
    LXValue _v1854 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1855 = px_null();
    LXValue _v1856 = px_null();
    LXValue _v1857 = px_null();
    LXValue _v1858 = px_null();
    LXValue _v1859 = px_null();
    LXValue _v1860 = px_null();
    LXValue _v1861 = px_null();
    LXValue _v1862 = px_null();
    LXValue _v1863 = px_null();
    LXValue _v1864 = px_null();
    LXValue _v1865 = px_null();
    LXValue _v1866 = px_null();
    LXValue _v1867 = px_null();
    LXValue _v1868 = px_null();
    LXValue px_err_1869_val = px_null();
    int px_err_1869_proped = 0;
    px_srcline(1749);
    _v1855 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v1854}, 1);
    px_srcline(1750);
    _v1856 = px_str("/* 由 bc_emit.px (M89-S3) 自动生成 — 字节码模块（VM 执行） */\n");
    px_srcline(1751);
     _v1856 = px_add(_v1856, px_str("#include \"runtime.h\"\n#include \"vm.h\"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n"));
    px_srcline(1752);
     _v1856 = px_add(_v1856, px_str("static const PxBCModule s_mod;\n\n"));
    px_srcline(1754);
     _v1856 = px_add(_v1856, px_str("static const PxK s_K[] = {\n"));
    px_srcline(1755);
    _v1857 = px_int(0LL);
    px_srcline(1756);
    while (px_is_truthy(px_lt(_v1857, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("k_pool"))}, 1)))) {
        px_srcline(1757);
        _v1858 = px_index(px_index(_v1855, px_str("k_pool")), _v1857);
        px_srcline(1758);
        if (px_is_truthy(px_eq(px_index(_v1858, px_str("kind")), px_str("int")))) {
            px_srcline(1759);
             _v1856 = px_add(_v1856, px_add(px_add(px_str("    {PXK_INT, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1858, px_str("i"))}, 1)), px_str(", 0, NULL},\n")));
        }
        else if (px_is_truthy(px_eq(px_index(_v1858, px_str("kind")), px_str("float")))) {
            px_srcline(1761);
             _v1856 = px_add(_v1856, px_add(px_add(px_str("    {PXK_FLT, 0, "), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v1858, px_str("f"))}, 1)), px_str(", NULL},\n")));
        }
        else if (px_is_truthy(px_eq(px_index(_v1858, px_str("kind")), px_str("str")))) {
            px_srcline(1763);
             _v1856 = px_add(_v1856, px_add(px_add(px_str("    {PXK_STR, 0, 0, \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1858, px_str("s"))}, 1)), px_str("\"},\n")));
        }
        else if (px_is_truthy(px_eq(px_index(_v1858, px_str("kind")), px_str("bool")))) {
            px_srcline(1765);
            _v1859 = px_str("0");
            px_srcline(1766);
            if (px_is_truthy(px_ne(px_index(_v1858, px_str("i")), px_int(0LL)))) {
                px_srcline(1767);
                 _v1859 = px_str("1");
            }
            px_srcline(1768);
             _v1856 = px_add(_v1856, px_add(px_add(px_str("    {PXK_BOOL, "), _v1859), px_str(", 0, NULL},\n")));
        }
        else if (px_is_truthy(px_eq(px_index(_v1858, px_str("kind")), px_str("func")))) {
            px_srcline(1770);
             _v1856 = px_add(_v1856, px_add(px_add(px_str("    {PXK_FUNC, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1858, px_str("i"))}, 1)), px_str(", 0, NULL},\n")));
        }
        else {
            px_srcline(1772);
             _v1856 = px_add(_v1856, px_str("    {PXK_NULL, 0, 0, NULL},\n"));
        }
        px_srcline(1773);
         _v1857 = px_add(_v1857, px_int(1LL));
    }
    px_srcline(1774);
     _v1856 = px_add(_v1856, px_str("};\n\n"));
    px_srcline(1776);
     _v1856 = px_add(_v1856, px_str("static const char* s_N[] = {\n"));
    px_srcline(1777);
    _v1860 = px_int(0LL);
    px_srcline(1778);
    while (px_is_truthy(px_lt(_v1860, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("n_pool"))}, 1)))) {
        px_srcline(1779);
         _v1856 = px_add(_v1856, px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1855, px_str("n_pool")), _v1860)}, 1)), px_str("\",\n")));
        px_srcline(1780);
         _v1860 = px_add(_v1860, px_int(1LL));
    }
    px_srcline(1781);
     _v1856 = px_add(_v1856, px_str("};\n\n"));
    px_srcline(1783);
     _v1856 = px_add(_v1856, px_str("static const char* s_G[] = {\n"));
    px_srcline(1784);
    _v1861 = px_int(0LL);
    px_srcline(1785);
    while (px_is_truthy(px_lt(_v1861, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("globals"))}, 1)))) {
        px_srcline(1786);
         _v1856 = px_add(_v1856, px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1855, px_str("globals")), _v1861)}, 1)), px_str("\",\n")));
        px_srcline(1787);
         _v1861 = px_add(_v1861, px_int(1LL));
    }
    px_srcline(1788);
     _v1856 = px_add(_v1856, px_str("};\n\n"));
    px_srcline(1790);
    _v1862 = px_int(0LL);
    px_srcline(1791);
    while (px_is_truthy(px_lt(_v1862, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("structs"))}, 1)))) {
        px_srcline(1792);
        _v1863 = px_index(px_index(_v1855, px_str("structs")), _v1862);
        px_srcline(1793);
         _v1856 = px_add(_v1856, px_add(px_add(px_str("static const char* s_st_"), px_call(px_get_global("str"), (LXValue[]){_v1862}, 1)), px_str("[] = {\n")));
        px_srcline(1794);
        _v1864 = px_int(0LL);
        px_srcline(1795);
        while (px_is_truthy(px_lt(_v1864, px_call(px_get_global("len"), (LXValue[]){px_index(_v1863, px_str("fnames"))}, 1)))) {
            px_srcline(1796);
             _v1856 = px_add(_v1856, px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1863, px_str("fnames")), _v1864)}, 1)), px_str("\",\n")));
            px_srcline(1797);
             _v1864 = px_add(_v1864, px_int(1LL));
        }
        px_srcline(1798);
         _v1856 = px_add(_v1856, px_str("};\n"));
        px_srcline(1799);
         _v1862 = px_add(_v1862, px_int(1LL));
    }
    px_srcline(1800);
     _v1856 = px_add(_v1856, px_str("static const PxStructDef s_structs[] = {\n"));
    px_srcline(1801);
     _v1862 = px_int(0LL);
    px_srcline(1802);
    while (px_is_truthy(px_lt(_v1862, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("structs"))}, 1)))) {
        px_srcline(1803);
        _v1863 = px_index(px_index(_v1855, px_str("structs")), _v1862);
        px_srcline(1804);
         _v1856 = px_add(_v1856, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    {\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1863, px_str("name"))}, 1)), px_str("\", s_st_")), px_call(px_get_global("str"), (LXValue[]){_v1862}, 1)), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1863, px_str("fnames"))}, 1)}, 1)), px_str("},\n")));
        px_srcline(1805);
         _v1862 = px_add(_v1862, px_int(1LL));
    }
    px_srcline(1806);
     _v1856 = px_add(_v1856, px_str("};\n\n"));
    px_srcline(1808);
    _v1865 = px_int(0LL);
    px_srcline(1809);
    while (px_is_truthy(px_lt(_v1865, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("funcs"))}, 1)))) {
        px_srcline(1810);
        _v1866 = px_index(px_index(_v1855, px_str("funcs")), _v1865);
        px_srcline(1811);
         _v1856 = px_add(_v1856, px_add(px_add(px_str("static const PxInst s_bc_"), px_call(px_get_global("str"), (LXValue[]){_v1865}, 1)), px_str("[] = {\n")));
        px_srcline(1812);
        _v1867 = px_int(0LL);
        px_srcline(1813);
        while (px_is_truthy(px_lt(_v1867, px_call(px_get_global("len"), (LXValue[]){px_index(_v1866, px_str("bc"))}, 1)))) {
            px_srcline(1814);
            _v1868 = px_index(px_index(_v1866, px_str("bc")), _v1867);
            px_srcline(1815);
             _v1856 = px_add(_v1856, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    {PXOP_"), px_index(_v1868, px_int(0LL))), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1868, px_int(1LL))}, 1)), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1868, px_int(2LL))}, 1)), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1868, px_int(3LL))}, 1)), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1868, px_int(4LL))}, 1)), px_str("},\n")));
            px_srcline(1816);
             _v1867 = px_add(_v1867, px_int(1LL));
        }
        px_srcline(1817);
         _v1856 = px_add(_v1856, px_str("};\n"));
        px_srcline(1818);
         _v1865 = px_add(_v1865, px_int(1LL));
    }
    px_srcline(1819);
     _v1856 = px_add(_v1856, px_str("\n"));
    px_srcline(1821);
     _v1856 = px_add(_v1856, px_str("static const PxVMFunc s_funcs[] = {\n"));
    px_srcline(1822);
     _v1865 = px_int(0LL);
    px_srcline(1823);
    while (px_is_truthy(px_lt(_v1865, px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("funcs"))}, 1)))) {
        px_srcline(1824);
        _v1866 = px_index(px_index(_v1855, px_str("funcs")), _v1865);
        px_srcline(1825);
         _v1856 = px_add(_v1856, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    {.name=\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1866, px_str("name"))}, 1)), px_str("\", .arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1866, px_str("arity"))}, 1)), px_str(", .ndefault=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1866, px_str("ndefault"))}, 1)), px_str(", .nslots=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1866, px_str("nslots"))}, 1)), px_str(", .bc=s_bc_")), px_call(px_get_global("str"), (LXValue[]){_v1865}, 1)), px_str(", .nbc=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1866, px_str("bc"))}, 1)}, 1)), px_str(", .mod=&s_mod, .upvals=NULL},\n")));
        px_srcline(1826);
         _v1865 = px_add(_v1865, px_int(1LL));
    }
    px_srcline(1827);
     _v1856 = px_add(_v1856, px_str("};\n\n"));
    px_srcline(1829);
     _v1856 = px_add(_v1856, px_str("static const PxBCModule s_mod = {\n"));
    px_srcline(1830);
     _v1856 = px_add(_v1856, px_add(px_add(px_add(px_add(px_str("    .name=\"<module>\", .K=s_K, .nK="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("k_pool"))}, 1)}, 1)), px_str(", .N=s_N, .nN=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("n_pool"))}, 1)}, 1)), px_str(",\n")));
    px_srcline(1831);
     _v1856 = px_add(_v1856, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    .G=s_G, .nG="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("globals"))}, 1)}, 1)), px_str(", .funcs=s_funcs, .nfuncs=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("funcs"))}, 1)}, 1)), px_str(", .top_idx=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1855, px_str("top"))}, 1)), px_str(",\n")));
    px_srcline(1832);
     _v1856 = px_add(_v1856, px_add(px_add(px_str("    .structs=s_structs, .nstructs="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1855, px_str("structs"))}, 1)}, 1)), px_str(",\n")));
    px_srcline(1833);
     _v1856 = px_add(_v1856, px_str("};\n\n"));
    px_srcline(1835);
     _v1856 = px_add(_v1856, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(1836);
     _v1856 = px_add(_v1856, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(1837);
     _v1856 = px_add(_v1856, px_str("    px_register_builtins();\n"));
    px_srcline(1838);
     _v1856 = px_add(_v1856, px_str("    px_gc_set_precise(1);   // M92-S2d：VM 轨产物默认 precise 精确根面（退役保守栈扫描；C 轨逃生舱产物不插此调用 → conservative）\n"));
    px_srcline(1839);
     _v1856 = px_add(_v1856, px_str("    LXValue _r = px_vm_run_module(px_vm_state(), &s_mod);\n"));
    px_srcline(1840);
     _v1856 = px_add(_v1856, px_str("    int _code = 0;\n"));
    px_srcline(1841);
     _v1856 = px_add(_v1856, px_str("    if (px_is_result(_r)) {\n"));
    px_srcline(1842);
     _v1856 = px_add(_v1856, px_str("        if (!px_result_ok(_r)) {\n"));
    px_srcline(1843);
     _v1856 = px_add(_v1856, px_str("            fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
    px_srcline(1844);
     _v1856 = px_add(_v1856, px_str("            _code = 1;\n"));
    px_srcline(1845);
     _v1856 = px_add(_v1856, px_str("        } else {\n"));
    px_srcline(1846);
     _v1856 = px_add(_v1856, px_str("            LXValue _uv = px_result_unwrap(_r);\n"));
    px_srcline(1847);
     _v1856 = px_add(_v1856, px_str("            if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
    px_srcline(1848);
     _v1856 = px_add(_v1856, px_str("        }\n"));
    px_srcline(1849);
     _v1856 = px_add(_v1856, px_str("    } else if (_r.type == PX_INT) {\n"));
    px_srcline(1850);
     _v1856 = px_add(_v1856, px_str("        _code = (int)_r.as.i;\n"));
    px_srcline(1851);
     _v1856 = px_add(_v1856, px_str("    }\n"));
    px_srcline(1852);
     _v1856 = px_add(_v1856, px_str("    if (getenv(\"PX_BC_DUMP\")) {\n"));
    px_srcline(1853);
     _v1856 = px_add(_v1856, px_str("        int _i;\n"));
    px_srcline(1854);
     _v1856 = px_add(_v1856, px_str("        for (_i = 0; _i < (int)s_mod.nG; _i++) {\n"));
    px_srcline(1855);
     _v1856 = px_add(_v1856, px_str("            LXValue _v = px_get_global(s_mod.G[_i]);\n"));
    px_srcline(1856);
     _v1856 = px_add(_v1856, px_str("            if (_v.type != PX_FUNC && _v.type != PX_NATIVE)\n"));
    px_srcline(1857);
     _v1856 = px_add(_v1856, px_str("                printf(\"%s=%s\\n\", s_mod.G[_i], px_to_string(_v));\n"));
    px_srcline(1858);
     _v1856 = px_add(_v1856, px_str("        }\n"));
    px_srcline(1859);
     _v1856 = px_add(_v1856, px_str("    }\n"));
    px_srcline(1860);
     _v1856 = px_add(_v1856, px_str("    return px_exit_code_final(_code);   // M120（qg-issue 76 E1）：协程隔离错误 → 退出码非 0\n"));
    px_srcline(1861);
     _v1856 = px_add(_v1856, px_str("}\n"));
    px_srcline(1862);
    return _v1856;
px_err_1869:
    if (px_err_1869_proped) return px_err_1869_val;
    return px_null();
}

static LXValue fn_bc_basename(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_basename");
    LXValue _v1870 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1871 = px_null();
    LXValue _v1872 = px_null();
    LXValue px_err_1873_val = px_null();
    int px_err_1873_proped = 0;
    px_srcline(64);
    _v1871 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1870}, 1), px_int(1LL));
    px_srcline(65);
    while (px_is_truthy(px_ge(_v1871, px_int(0LL)))) {
        px_srcline(66);
        if (px_is_truthy(px_eq(px_index(_v1870, _v1871), px_str("/")))) {
            px_srcline(67);
            _v1872 = px_slice(_v1870, px_add(_v1871, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1870}, 1), px_null());
            px_srcline(68);
            if (px_is_truthy(({ LXValue _t1874 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1872}, 1), px_int(3LL)); px_is_truthy(_t1874) ? px_eq(px_slice(_v1872, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1872}, 1), px_int(3LL)), px_call(px_get_global("len"), (LXValue[]){_v1872}, 1), px_null()), px_str(".px")) : _t1874; }))) {
                px_srcline(69);
                return px_slice(_v1872, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1872}, 1), px_int(3LL)), px_null());
            }
            px_srcline(70);
            return _v1872;
        }
        px_srcline(71);
         _v1871 = px_sub(_v1871, px_int(1LL));
    }
    px_srcline(72);
    if (px_is_truthy(({ LXValue _t1875 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1870}, 1), px_int(3LL)); px_is_truthy(_t1875) ? px_eq(px_slice(_v1870, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1870}, 1), px_int(3LL)), px_call(px_get_global("len"), (LXValue[]){_v1870}, 1), px_null()), px_str(".px")) : _t1875; }))) {
        px_srcline(73);
        return px_slice(_v1870, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1870}, 1), px_int(3LL)), px_null());
    }
    px_srcline(74);
    return _v1870;
px_err_1873:
    if (px_err_1873_proped) return px_err_1873_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v1876 = px_null();
    LXValue _v1877 = px_null();
    LXValue _v1878 = px_null();
    LXValue _v1879 = px_null();
    LXValue _v1880 = px_null();
    LXValue _v1881 = px_null();
    LXValue _v1882 = px_null();
    LXValue _v1883 = px_null();
    LXValue _v1884 = px_null();
    LXValue _v1885 = px_null();
    LXValue _v1886 = px_null();
    LXValue px_err_1887_val = px_null();
    int px_err_1887_proped = 0;
    px_srcline(77);
    _v1876 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    px_srcline(79);
    if (px_is_truthy(({ LXValue _t1889 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1876}, 1), px_int(2LL)); px_is_truthy(_t1889) ? ({ LXValue _t1888 = px_eq(px_index(_v1876, px_int(1LL)), px_str("--version")); px_is_truthy(_t1888) ? _t1888 : px_eq(px_index(_v1876, px_int(1LL)), px_str("-v")); }) : _t1889; }))) {
        px_srcline(80);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        px_srcline(81);
        return px_int(0LL);
    }
    px_srcline(84);
    _v1877 = px_bool(false);
    px_srcline(85);
    _v1878 = px_bool(false);
    px_srcline(86);
    _v1879 = px_int(1LL);
    px_srcline(87);
    while (px_is_truthy(px_lt(_v1879, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1876}, 1), px_int(1LL))))) {
        px_srcline(88);
        if (px_is_truthy(({ LXValue _t1890 = px_eq(px_index(_v1876, _v1879), px_str("bc")); px_is_truthy(_t1890) ? _t1890 : px_eq(px_index(_v1876, _v1879), px_str("--emit-c")); }))) {
            px_srcline(89);
             _v1877 = px_bool(true);
        }
        px_srcline(90);
        if (px_is_truthy(px_eq(px_index(_v1876, _v1879), px_str("--emit-c")))) {
            px_srcline(91);
             _v1878 = px_bool(true);
        }
        px_srcline(92);
         _v1879 = px_add(_v1879, px_int(1LL));
    }
    px_srcline(94);
    _v1880 = px_index(_v1876, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1876}, 1), px_int(1LL)));
    px_srcline(95);
    _v1881 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1880}, 1);
    px_srcline(96);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v1880}, 1)}, 1));
    px_srcline(97);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(98);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(99);
    _v1882 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(100);
    _v1883 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v1882, _v1881}, 2);
    px_srcline(101);
    if (px_is_truthy(_v1877)) {
        px_srcline(102);
        if (px_is_truthy(_v1878)) {
            px_srcline(103);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("bc_emit_c_program"), (LXValue[]){_v1883}, 1)}, 1));
            px_srcline(104);
            return px_int(0LL);
        }
        px_srcline(105);
        _v1884 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v1883}, 1);
        px_srcline(106);
        px_index_set(_v1884, px_str("name"), px_call(px_get_global("bc_basename"), (LXValue[]){_v1880}, 1));
        px_srcline(107);
        (void)(px_call(px_get_global("bc_dump_module"), (LXValue[]){_v1884}, 1));
        px_srcline(108);
        return px_int(0LL);
    }
    px_srcline(109);
    _v1885 = px_call(px_get_global("cg_generate"), (LXValue[]){_v1883}, 1);
    px_srcline(111);
    _v1886 = px_call(px_get_global("len"), (LXValue[]){_v1885}, 1);
    px_srcline(112);
    if (px_is_truthy(({ LXValue _t1891 = px_gt(_v1886, px_int(0LL)); px_is_truthy(_t1891) ? px_eq(px_index(_v1885, px_sub(_v1886, px_int(1LL))), px_str("\n")) : _t1891; }))) {
        px_srcline(113);
         _v1885 = px_slice(_v1885, px_int(0LL), px_sub(_v1886, px_int(1LL)), px_null());
    }
    px_srcline(114);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1885}, 1));
px_err_1887:
    if (px_err_1887_proped) return px_err_1887_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("is_cont_op", px_func("is_cont_op", fn_is_cont_op, NULL));
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
    px_set_global("scan_interp_nested_string", px_func("scan_interp_nested_string", fn_scan_interp_nested_string, NULL));
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
    px_set_global("chk_op", px_func("chk_op", fn_chk_op, NULL));
    px_set_global("node_pos", px_func("node_pos", fn_node_pos, NULL));
    px_set_global("qstr", px_func("qstr", fn_qstr, NULL));
    px_set_global("parse_program", px_func("parse_program", fn_parse_program, NULL));
    px_set_global("parse_stmt", px_func("parse_stmt", fn_parse_stmt, NULL));
    px_set_global("parse_var_decl", px_func("parse_var_decl", fn_parse_var_decl, NULL));
    px_set_global("parse_assign_or_expr", px_func("parse_assign_or_expr", fn_parse_assign_or_expr, NULL));
    px_set_global("parse_if", px_func("parse_if", fn_parse_if, NULL));
    px_set_global("parse_for", px_func("parse_for", fn_parse_for, NULL));
    px_set_global("parse_while", px_func("parse_while", fn_parse_while, NULL));
    px_set_global("parse_block_ctxt", px_func("parse_block_ctxt", fn_parse_block_ctxt, NULL));
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
    px_set_global("cg_load_of", px_func("cg_load_of", fn_cg_load_of, NULL));
    px_set_global("cg_store_of", px_func("cg_store_of", fn_cg_store_of, NULL));
    px_set_global("cg_name_add", px_func("cg_name_add", fn_cg_name_add, NULL));
    px_set_global("cg_ast_used", px_func("cg_ast_used", fn_cg_ast_used, NULL));
    px_set_global("cg_ast_bound", px_func("cg_ast_bound", fn_cg_ast_bound, NULL));
    px_set_global("cg_closure_caps", px_func("cg_closure_caps", fn_cg_closure_caps, NULL));
    px_set_global("cg_scan_closure_caps", px_func("cg_scan_closure_caps", fn_cg_scan_closure_caps, NULL));
    px_set_global("cg_mark_immutable", px_func("cg_mark_immutable", fn_cg_mark_immutable, NULL));
    px_set_global("cg_is_immutable", px_func("cg_is_immutable", fn_cg_is_immutable, NULL));
    px_set_global("cg_is_wildcard", px_func("cg_is_wildcard", fn_cg_is_wildcard, NULL));
    px_set_global("cg_perr", px_func("cg_perr", fn_cg_perr, NULL));
    px_set_global("cg_pwarn", px_func("cg_pwarn", fn_cg_pwarn, NULL));
    px_set_global("cg_is_nonnull_ty", px_func("cg_is_nonnull_ty", fn_cg_is_nonnull_ty, NULL));
    px_set_global("cg_is_null_lit", px_func("cg_is_null_lit", fn_cg_is_null_lit, NULL));
    px_set_global("cg_sem_vardecl", px_func("cg_sem_vardecl", fn_cg_sem_vardecl, NULL));
    px_set_global("cg_sem_assign", px_func("cg_sem_assign", fn_cg_sem_assign, NULL));
    px_set_global("cg_sem_call", px_func("cg_sem_call", fn_cg_sem_call, NULL));
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
    px_set_global("bc_new_dict", px_func("bc_new_dict", fn_bc_new_dict, NULL));
    px_set_global("bc_k_find", px_func("bc_k_find", fn_bc_k_find, NULL));
    px_set_global("bc_k_add", px_func("bc_k_add", fn_bc_k_add, NULL));
    px_set_global("bc_n_add", px_func("bc_n_add", fn_bc_n_add, NULL));
    px_set_global("bc_g_add", px_func("bc_g_add", fn_bc_g_add, NULL));
    px_set_global("bc_is_global", px_func("bc_is_global", fn_bc_is_global, NULL));
    px_set_global("bc_new_module", px_func("bc_new_module", fn_bc_new_module, NULL));
    px_set_global("bc_struct_index", px_func("bc_struct_index", fn_bc_struct_index, NULL));
    px_set_global("bc_enum_has", px_func("bc_enum_has", fn_bc_enum_has, NULL));
    px_set_global("bc_const_find", px_func("bc_const_find", fn_bc_const_find, NULL));
    px_set_global("bc_collect_consts", px_func("bc_collect_consts", fn_bc_collect_consts, NULL));
    px_set_global("bc_collect_impl_list", px_func("bc_collect_impl_list", fn_bc_collect_impl_list, NULL));
    px_set_global("bc_new_func", px_func("bc_new_func", fn_bc_new_func, NULL));
    px_set_global("bc_slot", px_func("bc_slot", fn_bc_slot, NULL));
    px_set_global("bc_tmp", px_func("bc_tmp", fn_bc_tmp, NULL));
    px_set_global("bc_emit_inst", px_func("bc_emit_inst", fn_bc_emit_inst, NULL));
    px_set_global("bc_patch_off", px_func("bc_patch_off", fn_bc_patch_off, NULL));
    px_set_global("bc_collect_hoist", px_func("bc_collect_hoist", fn_bc_collect_hoist, NULL));
    px_set_global("bc_emit_null", px_func("bc_emit_null", fn_bc_emit_null, NULL));
    px_set_global("bc_emit_int", px_func("bc_emit_int", fn_bc_emit_int, NULL));
    px_set_global("bc_emit_float", px_func("bc_emit_float", fn_bc_emit_float, NULL));
    px_set_global("bc_unop_op", px_func("bc_unop_op", fn_bc_unop_op, NULL));
    px_set_global("bc_binop_op", px_func("bc_binop_op", fn_bc_binop_op, NULL));
    px_set_global("bc_emit_expr", px_func("bc_emit_expr", fn_bc_emit_expr, NULL));
    px_set_global("bc_emit_call", px_func("bc_emit_call", fn_bc_emit_call, NULL));
    px_set_global("bc_emit_methodcall", px_func("bc_emit_methodcall", fn_bc_emit_methodcall, NULL));
    px_set_global("bc_emit_struct_new", px_func("bc_emit_struct_new", fn_bc_emit_struct_new, NULL));
    px_set_global("bc_emit_enum_new", px_func("bc_emit_enum_new", fn_bc_emit_enum_new, NULL));
    px_set_global("bc_emit_methodcall_slot", px_func("bc_emit_methodcall_slot", fn_bc_emit_methodcall_slot, NULL));
    px_set_global("bc_emit_push_lambda", px_func("bc_emit_push_lambda", fn_bc_emit_push_lambda, NULL));
    px_set_global("bc_emit_comp", px_func("bc_emit_comp", fn_bc_emit_comp, NULL));
    px_set_global("bc_emit_genexp", px_func("bc_emit_genexp", fn_bc_emit_genexp, NULL));
    px_set_global("bc_match_enumvar", px_func("bc_match_enumvar", fn_bc_match_enumvar, NULL));
    px_set_global("bc_match_cond", px_func("bc_match_cond", fn_bc_match_cond, NULL));
    px_set_global("bc_assign_local_slot", px_func("bc_assign_local_slot", fn_bc_assign_local_slot, NULL));
    px_set_global("bc_assign_global", px_func("bc_assign_global", fn_bc_assign_global, NULL));
    px_set_global("bc_assign_index", px_func("bc_assign_index", fn_bc_assign_index, NULL));
    px_set_global("bc_assign_field", px_func("bc_assign_field", fn_bc_assign_field, NULL));
    px_set_global("bc_assign_op_name", px_func("bc_assign_op_name", fn_bc_assign_op_name, NULL));
    px_set_global("bc_emit_stmt_inner", px_func("bc_emit_stmt_inner", fn_bc_emit_stmt_inner, NULL));
    px_set_global("bc_emit_select", px_func("bc_emit_select", fn_bc_emit_select, NULL));
    px_set_global("bc_emit_stmt", px_func("bc_emit_stmt", fn_bc_emit_stmt, NULL));
    px_set_global("bc_emit_stmts", px_func("bc_emit_stmts", fn_bc_emit_stmts, NULL));
    px_set_global("bc_emit_if", px_func("bc_emit_if", fn_bc_emit_if, NULL));
    px_set_global("bc_emit_while", px_func("bc_emit_while", fn_bc_emit_while, NULL));
    px_set_global("bc_emit_for", px_func("bc_emit_for", fn_bc_emit_for, NULL));
    px_set_global("bc_emit_func_def", px_func("bc_emit_func_def", fn_bc_emit_func_def, NULL));
    px_set_global("bc_emit_impl_def", px_func("bc_emit_impl_def", fn_bc_emit_impl_def, NULL));
    px_set_global("bc_emit_func_body", px_func("bc_emit_func_body", fn_bc_emit_func_body, NULL));
    px_set_global("bc_emit_default_fill", px_func("bc_emit_default_fill", fn_bc_emit_default_fill, NULL));
    px_set_global("bc_emit_func_top", px_func("bc_emit_func_top", fn_bc_emit_func_top, NULL));
    px_set_global("bc_emit_program", px_func("bc_emit_program", fn_bc_emit_program, NULL));
    px_set_global("bc_dump_module", px_func("bc_dump_module", fn_bc_dump_module, NULL));
    px_set_global("bc_emit_c_program", px_func("bc_emit_c_program", fn_bc_emit_c_program, NULL));
    px_set_global("bc_basename", px_func("bc_basename", fn_bc_basename, NULL));
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
    px_srcline(22);
    px_set_global("g_bracket_depth", px_int(0LL));
    px_srcline(23);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(26);
    px_set_global("g_last_kind", px_str(""));
    px_srcline(27);
    px_set_global("g_count", px_int(0LL));
    px_srcline(28);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(30);
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_srcline(39);
    px_set_global("CONT_OPS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("+"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("-"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("*"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("/"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("//"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("%"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("**"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("=="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("!="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("<"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(">"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("<="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(">="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("&"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("|"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("^"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("<<"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(">>"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(">>>"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("??"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("|>"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("+="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("-="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("*="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("/="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("//="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("%="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("**="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("&="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("|="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("^="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("<<="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(">>="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(">>>="); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("."); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("?."); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("=>"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str(","); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }));
    px_srcline(50);
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
    px_srcline(99);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(104);
    px_set_global("p_paren_ctxt", px_int(0LL));
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
    px_srcline(35);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(37);
    px_set_global("cg_closures", px_str(""));
    px_srcline(38);
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(39);
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(40);
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(41);
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(42);
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(44);
    px_set_global("cg_cells", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(45);
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(46);
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(47);
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(48);
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(49);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(50);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(51);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(52);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(53);
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(34);
    px_set_global("g_bcm", px_null());
    px_srcline(25);
    px_set_global("g_src", px_str(""));
    px_srcline(26);
    px_set_global("g_len", px_int(0LL));
    px_srcline(27);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(28);
    px_set_global("g_line", px_int(1LL));
    px_srcline(29);
    px_set_global("g_col", px_int(1LL));
    px_srcline(30);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(31);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(32);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(33);
    px_set_global("g_count", px_int(0LL));
    px_srcline(34);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(35);
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("let")); } { LXValue _k = px_str("var"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("var")); } { LXValue _k = px_str("const"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("const")); } { LXValue _k = px_str("def"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("def")); } { LXValue _k = px_str("fn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("fn")); } { LXValue _k = px_str("struct"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("struct")); } { LXValue _k = px_str("enum"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("enum")); } { LXValue _k = px_str("trait"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("trait")); } { LXValue _k = px_str("impl"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("impl")); } { LXValue _k = px_str("match"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("match")); } { LXValue _k = px_str("case"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("case")); } { LXValue _k = px_str("if"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("if")); } { LXValue _k = px_str("elif"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("elif")); } { LXValue _k = px_str("else"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("else")); } { LXValue _k = px_str("for"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("for")); } { LXValue _k = px_str("while"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("while")); } { LXValue _k = px_str("in"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("in")); } { LXValue _k = px_str("and"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("and")); } { LXValue _k = px_str("or"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("or")); } { LXValue _k = px_str("not"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("not")); } { LXValue _k = px_str("return"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("return")); } { LXValue _k = px_str("break"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("break")); } { LXValue _k = px_str("continue"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("continue")); } { LXValue _k = px_str("import"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("import")); } { LXValue _k = px_str("from"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("from")); } { LXValue _k = px_str("pub"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("pub")); } { LXValue _k = px_str("as"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("as")); } { LXValue _k = px_str("spawn"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("spawn")); } { LXValue _k = px_str("chan"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("chan")); } { LXValue _k = px_str("send"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("send")); } { LXValue _k = px_str("recv"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("recv")); } { LXValue _k = px_str("select"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("select")); } { LXValue _k = px_str("true"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("true")); } { LXValue _k = px_str("false"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("false")); } { LXValue _k = px_str("null"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("None"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("null")); } { LXValue _k = px_str("self"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("self")); } { LXValue _k = px_str("capture"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("capture")); } { LXValue _k = px_str("extern"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("extern")); } _d; }));
    px_srcline(36);
    px_set_global("CTRL_ALL", px_str(""));
    px_srcline(37);
    px_set_global("p_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(38);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(39);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(40);
    px_set_global("cg_closures", px_str(""));
    px_srcline(41);
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(42);
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(43);
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(44);
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(46);
    px_set_global("cg_cells", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(47);
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(48);
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(49);
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(50);
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(51);
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(52);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(53);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(54);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(55);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(56);
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    px_srcline(58);
    px_set_global("g_bcm", px_null());
    px_srcline(60);
    px_set_global("PXC_VER", px_str("0.2.0"));
    px_srcline(61);
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
      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）
    }
}
