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
    px_srcline(170);
    _v65 = px_list_n((LXValue[]){}, 0);
    px_srcline(171);
    if (px_is_truthy(px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")))) {
        px_srcline(172);
        (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
        px_srcline(173);
        (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
        px_srcline(174);
        _v66 = px_int(1LL);
        px_srcline(175);
        while (px_is_truthy(px_gt(_v66, px_int(0LL)))) {
            px_srcline(176);
            _v67 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            px_srcline(177);
            if (px_is_truthy(px_eq(_v67, px_str("")))) {
                px_srcline(178);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1002"), px_str("块注释未闭合（缺少 |#）")}, 2));
            }
            else if (px_is_truthy(({ LXValue _t69 = px_eq(_v67, px_str("#")); px_is_truthy(_t69) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("|")) : _t69; }))) {
                px_srcline(180);
                (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
                px_srcline(181);
                (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
                px_srcline(182);
                 _v66 = px_add(_v66, px_int(1LL));
            }
            else if (px_is_truthy(({ LXValue _t70 = px_eq(_v67, px_str("|")); px_is_truthy(_t70) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("#")) : _t70; }))) {
                px_srcline(184);
                (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
                px_srcline(185);
                (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
                px_srcline(186);
                 _v66 = px_sub(_v66, px_int(1LL));
            }
            else {
                px_srcline(188);
                (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
            }
        }
    }
    else {
        px_srcline(190);
        while (px_is_truthy(({ LXValue _t71 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t71) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\n")) : _t71; }))) {
            px_srcline(191);
            (void)(px_method(_v65, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
        }
    }
    px_srcline(192);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v65}, 2);
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
    LXValue _v75 = px_null();
    LXValue px_err_76_val = px_null();
    int px_err_76_proped = 0;
    px_srcline(195);
    _v72 = px_get_global("g_line");
    px_srcline(196);
    _v73 = px_get_global("g_col");
    px_srcline(201);
    _v74 = px_list_n((LXValue[]){}, 0);
    px_srcline(202);
    while (px_is_truthy(px_call(px_get_global("is_ident_continue"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
        px_srcline(203);
        (void)(px_method(_v74, "append", (LXValue[]){px_call(px_get_global("advance"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(204);
    _v75 = px_call(px_get_global("join"), (LXValue[]){px_str(""), _v74}, 2);
    px_srcline(205);
    if (px_is_truthy(px_method(px_get_global("KEYWORDS"), "has", (LXValue[]){_v75}, 1))) {
        px_srcline(206);
        return px_list_n((LXValue[]){px_index(px_get_global("KEYWORDS"), _v75), px_str(""), _v72, _v73}, 4);
    }
    px_srcline(207);
    return px_list_n((LXValue[]){px_str("标识符"), _v75, _v72, _v73}, 4);
px_err_76:
    if (px_err_76_proped) return px_err_76_val;
    return px_null();
}

static LXValue fn_scan_radix_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_radix_token");
    LXValue _v77 = (nargs > 0) ? args[0] : px_null();
    LXValue _v78 = (nargs > 1) ? args[1] : px_null();
    LXValue _v79 = (nargs > 2) ? args[2] : px_null();
    LXValue _v80 = px_null();
    LXValue _v81 = px_null();
    LXValue _v82 = px_null();
    LXValue _v83 = px_null();
    LXValue _v84 = px_null();
    LXValue _v85 = px_null();
    LXValue px_err_86_val = px_null();
    int px_err_86_proped = 0;
    px_srcline(210);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(211);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(212);
    _v80 = px_str("");
    px_srcline(213);
    while (px_is_truthy(({ LXValue _t87 = px_call(px_get_global("is_alnum"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t87) ? _t87 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
        px_srcline(214);
         _v80 = px_add(_v80, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(215);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v80}, 1), px_int(0LL)))) {
        px_srcline(216);
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_str("进制字面量缺少数字（基数 "), px_call(px_get_global("str"), (LXValue[]){_v77}, 1)), px_str("）"))}, 2));
    }
    px_srcline(217);
    _v81 = px_str("");
    px_srcline(218);
    _v82 = px_int(0LL);
    px_srcline(219);
    while (px_is_truthy(px_lt(_v82, px_call(px_get_global("len"), (LXValue[]){_v80}, 1)))) {
        px_srcline(220);
        if (px_is_truthy(px_ne(px_index(_v80, _v82), px_str("_")))) {
            px_srcline(221);
             _v81 = px_add(_v81, px_index(_v80, _v82));
        }
        px_srcline(222);
         _v82 = px_add(_v82, px_int(1LL));
    }
    px_srcline(223);
    _v83 = px_int(0LL);
    px_srcline(224);
    _v84 = px_int(0LL);
    px_srcline(225);
    while (px_is_truthy(px_lt(_v84, px_call(px_get_global("len"), (LXValue[]){_v81}, 1)))) {
        px_srcline(226);
        _v85 = px_call(px_get_global("digit_val"), (LXValue[]){px_index(_v81, _v84)}, 1);
        px_srcline(227);
        if (px_is_truthy(({ LXValue _t88 = px_lt(_v85, px_int(0LL)); px_is_truthy(_t88) ? _t88 : px_ge(_v85, _v77); }))) {
            px_srcline(228);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_add(px_add(px_str("无效 "), px_call(px_get_global("str"), (LXValue[]){_v77}, 1)), px_str("-进制字面量: ")), _v81)}, 2));
        }
        px_srcline(229);
         _v83 = px_add(px_mul(_v83, _v77), _v85);
        px_srcline(230);
         _v84 = px_add(_v84, px_int(1LL));
    }
    px_srcline(231);
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){_v83}, 1), _v78, _v79}, 4);
px_err_86:
    if (px_err_86_proped) return px_err_86_val;
    return px_null();
}

static LXValue fn_strip_leading_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("strip_leading_zeros");
    LXValue _v89 = (nargs > 0) ? args[0] : px_null();
    LXValue _v90 = px_null();
    LXValue px_err_91_val = px_null();
    int px_err_91_proped = 0;
    px_srcline(233);
    _v90 = px_int(0LL);
    px_srcline(234);
    while (px_is_truthy(({ LXValue _t92 = px_lt(_v90, px_sub(px_call(px_get_global("len"), (LXValue[]){_v89}, 1), px_int(1LL))); px_is_truthy(_t92) ? px_eq(px_index(_v89, _v90), px_str("0")) : _t92; }))) {
        px_srcline(235);
         _v90 = px_add(_v90, px_int(1LL));
    }
    px_srcline(236);
    return px_slice(_v89, _v90, px_null(), px_null());
px_err_91:
    if (px_err_91_proped) return px_err_91_val;
    return px_null();
}

static LXValue fn_scan_number_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_number_token");
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
    LXValue _v106 = px_null();
    LXValue px_err_107_val = px_null();
    int px_err_107_proped = 0;
    px_srcline(238);
    _v93 = px_get_global("g_line");
    px_srcline(239);
    _v94 = px_get_global("g_col");
    px_srcline(240);
    if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("0")))) {
        px_srcline(241);
        _v95 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
        px_srcline(242);
        if (px_is_truthy(({ LXValue _t108 = px_eq(_v95, px_str("x")); px_is_truthy(_t108) ? _t108 : px_eq(_v95, px_str("X")); }))) {
            px_srcline(243);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(16LL), _v93, _v94}, 3);
        }
        px_srcline(244);
        if (px_is_truthy(({ LXValue _t109 = px_eq(_v95, px_str("b")); px_is_truthy(_t109) ? _t109 : px_eq(_v95, px_str("B")); }))) {
            px_srcline(245);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(2LL), _v93, _v94}, 3);
        }
        px_srcline(246);
        if (px_is_truthy(({ LXValue _t110 = px_eq(_v95, px_str("o")); px_is_truthy(_t110) ? _t110 : px_eq(_v95, px_str("O")); }))) {
            px_srcline(247);
            return px_call(px_get_global("scan_radix_token"), (LXValue[]){px_int(8LL), _v93, _v94}, 3);
        }
    }
    px_srcline(248);
    _v96 = px_str("");
    px_srcline(249);
    while (px_is_truthy(({ LXValue _t111 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t111) ? _t111 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
        px_srcline(250);
         _v96 = px_add(_v96, px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(251);
    _v97 = px_bool(false);
    px_srcline(252);
    if (px_is_truthy(({ LXValue _t112 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")); px_is_truthy(_t112) ? px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek2"), (LXValue[]){}, 0)}, 1) : _t112; }))) {
        px_srcline(253);
         _v97 = px_bool(true);
        px_srcline(254);
         _v96 = px_add(_v96, px_str("."));
        px_srcline(255);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(256);
        while (px_is_truthy(({ LXValue _t113 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t113) ? _t113 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
            px_srcline(257);
             _v96 = px_add(_v96, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(258);
    if (px_is_truthy(({ LXValue _t114 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("e")); px_is_truthy(_t114) ? _t114 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("E")); }))) {
        px_srcline(259);
        _v98 = px_get_global("g_pos");
        px_srcline(260);
        _v99 = px_get_global("g_line");
        px_srcline(261);
        _v100 = px_get_global("g_col");
        px_srcline(262);
        _v101 = px_str("");
        px_srcline(263);
         _v101 = px_add(_v101, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(264);
        if (px_is_truthy(({ LXValue _t115 = px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("+")); px_is_truthy(_t115) ? _t115 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")); }))) {
            px_srcline(265);
             _v101 = px_add(_v101, px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        px_srcline(266);
        if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1))) {
            px_srcline(267);
            while (px_is_truthy(({ LXValue _t116 = px_call(px_get_global("is_digit"), (LXValue[]){px_call(px_get_global("peek"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t116) ? _t116 : px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("_")); }))) {
                px_srcline(268);
                 _v101 = px_add(_v101, px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            px_srcline(269);
             _v96 = px_add(_v96, _v101);
            px_srcline(270);
             _v97 = px_bool(true);
        }
        else {
            px_srcline(272);
            px_set_global("g_pos", _v98);
            px_srcline(273);
            px_set_global("g_line", _v99);
            px_srcline(274);
            px_set_global("g_col", _v100);
        }
    }
    px_srcline(275);
    _v102 = px_str("");
    px_srcline(276);
    _v103 = px_int(0LL);
    px_srcline(277);
    while (px_is_truthy(px_lt(_v103, px_call(px_get_global("len"), (LXValue[]){_v96}, 1)))) {
        px_srcline(278);
        if (px_is_truthy(px_ne(px_index(_v96, _v103), px_str("_")))) {
            px_srcline(279);
             _v102 = px_add(_v102, px_index(_v96, _v103));
        }
        px_srcline(280);
         _v103 = px_add(_v103, px_int(1LL));
    }
    px_srcline(281);
    if (px_is_truthy(_v97)) {
        px_srcline(282);
        _v104 = px_call(px_get_global("float"), (LXValue[]){_v102}, 1);
        px_srcline(283);
        _v105 = px_call(px_get_global("str"), (LXValue[]){_v104}, 1);
        px_srcline(284);
        if (px_is_truthy(px_call(px_get_global("ends_with"), (LXValue[]){_v105, px_str(".0")}, 2))) {
            px_srcline(285);
             _v105 = px_slice(_v105, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v105}, 1), px_int(2LL)), px_null());
        }
        px_srcline(286);
        return px_list_n((LXValue[]){px_str("浮点"), _v105, _v93, _v94}, 4);
    }
    px_srcline(287);
    _v106 = px_call(px_get_global("strip_leading_zeros"), (LXValue[]){_v102}, 1);
    px_srcline(288);
    if (px_is_truthy(({ LXValue _t118 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v106}, 1), px_int(19LL)); px_is_truthy(_t118) ? _t118 : ({ LXValue _t117 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v106}, 1), px_int(19LL)); px_is_truthy(_t117) ? px_gt(_v106, px_str("9223372036854775807")) : _t117; }); }))) {
        px_srcline(289);
        (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1004"), px_add(px_str("无效整数: "), _v96)}, 2));
    }
    px_srcline(290);
    return px_list_n((LXValue[]){px_str("整数"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v102}, 1)}, 1), _v93, _v94}, 4);
px_err_107:
    if (px_err_107_proped) return px_err_107_val;
    return px_null();
}

static LXValue fn_scan_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_string");
    LXValue _v119 = (nargs > 0) ? args[0] : px_null();
    LXValue _v120 = px_null();
    LXValue _v121 = px_null();
    LXValue px_err_122_val = px_null();
    int px_err_122_proped = 0;
    px_srcline(293);
    _v120 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){_v119}, 1);
    px_srcline(294);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_index(_v120, px_int(0LL))}, 1));
    px_srcline(295);
    _v121 = px_int(1LL);
    px_srcline(296);
    while (px_is_truthy(px_lt(_v121, px_call(px_get_global("len"), (LXValue[]){_v120}, 1)))) {
        px_srcline(297);
        (void)(px_method(px_get_global("g_pending"), "append", (LXValue[]){px_index(_v120, _v121)}, 1));
        px_srcline(298);
         _v121 = px_add(_v121, px_int(1LL));
    }
px_err_122:
    if (px_err_122_proped) return px_err_122_val;
    return px_null();
}

static LXValue fn_scan_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_string_tokens");
    LXValue _v123 = (nargs > 0) ? args[0] : px_null();
    LXValue _v124 = px_null();
    LXValue _v125 = px_null();
    LXValue _v126 = px_null();
    LXValue _v127 = px_null();
    LXValue _v128 = px_null();
    LXValue _v129 = px_null();
    LXValue _v130 = px_null();
    LXValue _v131 = px_null();
    LXValue px_err_132_val = px_null();
    int px_err_132_proped = 0;
    px_srcline(300);
    _v124 = px_get_global("g_line");
    px_srcline(301);
    _v125 = px_get_global("g_col");
    px_srcline(302);
    _v126 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    px_srcline(303);
    if (px_is_truthy(({ LXValue _t134 = ({ LXValue _t133 = px_eq(_v126, px_str("\"")); px_is_truthy(_t133) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("\"")) : _t133; }); px_is_truthy(_t134) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")) : _t134; }))) {
        px_srcline(304);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(305);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(306);
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("\""), _v124, _v125, _v123}, 4);
    }
    px_srcline(307);
    if (px_is_truthy(({ LXValue _t136 = ({ LXValue _t135 = px_eq(_v126, px_str("'")); px_is_truthy(_t135) ? px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("'")) : _t135; }); px_is_truthy(_t136) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")) : _t136; }))) {
        px_srcline(308);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(309);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(310);
        return px_call(px_get_global("scan_multiline_string_tokens"), (LXValue[]){px_str("'"), _v124, _v125, _v123}, 4);
    }
    px_srcline(311);
    _v127 = px_list_n((LXValue[]){}, 0);
    px_srcline(312);
    _v128 = px_str("");
    px_srcline(313);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(314);
        _v129 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(315);
        if (px_is_truthy(px_eq(_v129, px_str("")))) {
            px_srcline(316);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_add(px_add(px_str("字符串未闭合（缺少 "), _v126), px_str("）")), _v124, _v125}, 4));
        }
        px_srcline(317);
        if (px_is_truthy(px_eq(_v129, _v126))) {
            px_srcline(318);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(319);
            break;
        }
        px_srcline(320);
        if (px_is_truthy(px_eq(_v129, px_str("\\")))) {
            px_srcline(321);
             _v128 = px_add(_v128, px_call(px_get_global("scan_escape"), (LXValue[]){_v124, _v125}, 2));
        }
        else if (px_is_truthy(px_eq(_v129, px_str("\n")))) {
            px_srcline(323);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v124, _v125}, 4));
        }
        else if (px_is_truthy(({ LXValue _t138 = ({ LXValue _t137 = px_eq(_v129, px_str("$")); px_is_truthy(_t137) ? _v123 : _t137; }); px_is_truthy(_t138) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t138; }))) {
            px_srcline(325);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(326);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(327);
            (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v128}, 1), _v124, _v125}, 4)}, 1));
            px_srcline(328);
            (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v124, _v125}, 4)}, 1));
            px_srcline(329);
            (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v124, _v125}, 4)}, 1));
            px_srcline(330);
            (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v124, _v125}, 4)}, 1));
            px_srcline(331);
            _v130 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v124, _v125}, 2);
            px_srcline(332);
            _v131 = px_int(0LL);
            px_srcline(333);
            while (px_is_truthy(px_lt(_v131, px_call(px_get_global("len"), (LXValue[]){_v130}, 1)))) {
                px_srcline(334);
                (void)(px_method(_v127, "append", (LXValue[]){px_index(_v130, _v131)}, 1));
                px_srcline(335);
                 _v131 = px_add(_v131, px_int(1LL));
            }
            px_srcline(336);
            (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v124, _v125}, 4)}, 1));
            px_srcline(337);
            (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v124, _v125}, 4)}, 1));
            px_srcline(338);
             _v128 = px_str("");
        }
        else {
            px_srcline(340);
             _v128 = px_add(_v128, _v129);
            px_srcline(341);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(342);
    (void)(px_method(_v127, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v128}, 1), _v124, _v125}, 4)}, 1));
    px_srcline(343);
    return _v127;
px_err_132:
    if (px_err_132_proped) return px_err_132_val;
    return px_null();
}

static LXValue fn_scan_multiline_string_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_multiline_string_tokens");
    LXValue _v139 = (nargs > 0) ? args[0] : px_null();
    LXValue _v140 = (nargs > 1) ? args[1] : px_null();
    LXValue _v141 = (nargs > 2) ? args[2] : px_null();
    LXValue _v142 = (nargs > 3) ? args[3] : px_null();
    LXValue _v143 = px_null();
    LXValue _v144 = px_null();
    LXValue _v145 = px_null();
    LXValue _v146 = px_null();
    LXValue _v147 = px_null();
    LXValue px_err_148_val = px_null();
    int px_err_148_proped = 0;
    px_srcline(345);
    _v143 = px_list_n((LXValue[]){}, 0);
    px_srcline(346);
    _v144 = px_str("");
    px_srcline(347);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(348);
        _v145 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(349);
        if (px_is_truthy(px_eq(_v145, px_str("")))) {
            px_srcline(350);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("多行字符串未闭合"), _v140, _v141}, 4));
        }
        px_srcline(351);
        if (px_is_truthy(({ LXValue _t150 = ({ LXValue _t149 = px_eq(_v145, _v139); px_is_truthy(_t149) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), _v139) : _t149; }); px_is_truthy(_t150) ? px_eq(px_call(px_get_global("peek3"), (LXValue[]){}, 0), _v139) : _t150; }))) {
            px_srcline(352);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(353);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(354);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(355);
            break;
        }
        px_srcline(356);
        if (px_is_truthy(px_eq(_v145, px_str("\\")))) {
            px_srcline(357);
             _v144 = px_add(_v144, px_call(px_get_global("scan_escape"), (LXValue[]){_v140, _v141}, 2));
        }
        else if (px_is_truthy(({ LXValue _t152 = ({ LXValue _t151 = px_eq(_v145, px_str("$")); px_is_truthy(_t151) ? _v142 : _t151; }); px_is_truthy(_t152) ? px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("{")) : _t152; }))) {
            px_srcline(359);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(360);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(361);
            (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v144}, 1), _v140, _v141}, 4)}, 1));
            px_srcline(362);
            (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v140, _v141}, 4)}, 1));
            px_srcline(363);
            (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str("标识符"), px_str("str"), _v140, _v141}, 4)}, 1));
            px_srcline(364);
            (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str("("), px_str(""), _v140, _v141}, 4)}, 1));
            px_srcline(365);
            _v146 = px_call(px_get_global("scan_interp_expr"), (LXValue[]){_v140, _v141}, 2);
            px_srcline(366);
            _v147 = px_int(0LL);
            px_srcline(367);
            while (px_is_truthy(px_lt(_v147, px_call(px_get_global("len"), (LXValue[]){_v146}, 1)))) {
                px_srcline(368);
                (void)(px_method(_v143, "append", (LXValue[]){px_index(_v146, _v147)}, 1));
                px_srcline(369);
                 _v147 = px_add(_v147, px_int(1LL));
            }
            px_srcline(370);
            (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str(")"), px_str(""), _v140, _v141}, 4)}, 1));
            px_srcline(371);
            (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str("+"), px_str(""), _v140, _v141}, 4)}, 1));
            px_srcline(372);
             _v144 = px_str("");
        }
        else {
            px_srcline(374);
             _v144 = px_add(_v144, _v145);
            px_srcline(375);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
    }
    px_srcline(376);
    (void)(px_method(_v143, "append", (LXValue[]){px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v144}, 1), _v140, _v141}, 4)}, 1));
    px_srcline(377);
    return _v143;
px_err_148:
    if (px_err_148_proped) return px_err_148_val;
    return px_null();
}

static LXValue fn_scan_interp_nested_string(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_interp_nested_string");
    LXValue _v153 = (nargs > 0) ? args[0] : px_null();
    LXValue _v154 = (nargs > 1) ? args[1] : px_null();
    LXValue _v155 = px_null();
    LXValue _v156 = px_null();
    LXValue _v157 = px_null();
    LXValue _v158 = px_null();
    LXValue _v159 = px_null();
    LXValue px_err_160_val = px_null();
    int px_err_160_proped = 0;
    px_srcline(382);
    _v155 = px_get_global("g_line");
    px_srcline(383);
    _v156 = px_get_global("g_col");
    px_srcline(384);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(385);
    _v157 = px_str("");
    px_srcline(386);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(387);
        _v158 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(388);
        if (px_is_truthy(px_eq(_v158, px_str("")))) {
            px_srcline(389);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串未闭合（缺少 \"）"), _v153, _v154}, 4));
        }
        px_srcline(390);
        if (px_is_truthy(px_eq(_v158, px_str("\\")))) {
            px_srcline(391);
            _v159 = px_call(px_get_global("peek2"), (LXValue[]){}, 0);
            px_srcline(392);
            if (px_is_truthy(({ LXValue _t161 = px_eq(_v159, px_str("\"")); px_is_truthy(_t161) ? _t161 : px_eq(_v159, px_str("'")); }))) {
                px_srcline(393);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(394);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(395);
                break;
            }
            px_srcline(396);
            if (px_is_truthy(px_eq(_v159, px_str("\\")))) {
                px_srcline(397);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(398);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(399);
                 _v157 = px_add(_v157, px_str("\\"));
                px_srcline(400);
                continue;
            }
            px_srcline(401);
            if (px_is_truthy(px_eq(_v159, px_str("n")))) {
                px_srcline(402);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(403);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(404);
                 _v157 = px_add(_v157, px_str("\n"));
                px_srcline(405);
                continue;
            }
            px_srcline(406);
            if (px_is_truthy(px_eq(_v159, px_str("t")))) {
                px_srcline(407);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(408);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(409);
                 _v157 = px_add(_v157, px_str("\t"));
                px_srcline(410);
                continue;
            }
            px_srcline(411);
            if (px_is_truthy(px_eq(_v159, px_str("r")))) {
                px_srcline(412);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(413);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(414);
                 _v157 = px_add(_v157, px_str("\r"));
                px_srcline(415);
                continue;
            }
            px_srcline(416);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(417);
             _v157 = px_add(_v157, _v158);
            px_srcline(418);
            continue;
        }
        px_srcline(419);
        if (px_is_truthy(px_eq(_v158, px_str("\n")))) {
            px_srcline(420);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("单行字符串不能跨行，请使用 \"\"\" 多行字符串"), _v153, _v154}, 4));
        }
        px_srcline(421);
         _v157 = px_add(_v157, _v158);
        px_srcline(422);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    }
    px_srcline(423);
    return px_list_n((LXValue[]){px_str("字符串"), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v157}, 1), _v155, _v156}, 4);
px_err_160:
    if (px_err_160_proped) return px_err_160_val;
    return px_null();
}

static LXValue fn_scan_interp_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_interp_expr");
    LXValue _v162 = (nargs > 0) ? args[0] : px_null();
    LXValue _v163 = (nargs > 1) ? args[1] : px_null();
    LXValue _v164 = px_null();
    LXValue _v165 = px_null();
    LXValue _v166 = px_null();
    LXValue _v167 = px_null();
    LXValue px_err_168_val = px_null();
    int px_err_168_proped = 0;
    px_srcline(425);
    _v164 = px_list_n((LXValue[]){}, 0);
    px_srcline(426);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(427);
        _v165 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
        px_srcline(428);
        if (px_is_truthy(px_eq(_v165, px_str("")))) {
            px_srcline(429);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值 ${ 未闭合（缺少 }）"), _v162, _v163}, 4));
        }
        px_srcline(434);
        if (px_is_truthy(({ LXValue _t170 = px_eq(_v165, px_str("\\")); px_is_truthy(_t170) ? ({ LXValue _t169 = px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("\"")); px_is_truthy(_t169) ? _t169 : px_eq(px_call(px_get_global("peek2"), (LXValue[]){}, 0), px_str("'")); }) : _t170; }))) {
            px_srcline(435);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(436);
            _v166 = px_call(px_get_global("scan_interp_nested_string"), (LXValue[]){_v162, _v163}, 2);
            px_srcline(437);
            (void)(px_method(_v164, "append", (LXValue[]){_v166}, 1));
            px_srcline(438);
            continue;
        }
        px_srcline(439);
        if (px_is_truthy(px_eq(_v165, px_str("}")))) {
            px_srcline(440);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(441);
            break;
        }
        px_srcline(442);
        if (px_is_truthy(px_eq(_v165, px_str("{")))) {
            px_srcline(443);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持 {} 字面量（dict/set），请先用变量保存")}, 2));
        }
        px_srcline(444);
        if (px_is_truthy(px_eq(_v165, px_str("\n")))) {
            px_srcline(445);
            (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串插值表达式不能跨行"), _v162, _v163}, 4));
        }
        px_srcline(446);
        if (px_is_truthy(px_eq(_v165, px_str("#")))) {
            px_srcline(447);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1006"), px_str("插值表达式不支持注释")}, 2));
        }
        px_srcline(448);
        if (px_is_truthy(({ LXValue _t171 = px_eq(_v165, px_str(" ")); px_is_truthy(_t171) ? _t171 : px_eq(_v165, px_str("\t")); }))) {
            px_srcline(449);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        }
        else if (px_is_truthy(({ LXValue _t172 = px_eq(_v165, px_str("\"")); px_is_truthy(_t172) ? _t172 : px_eq(_v165, px_str("'")); }))) {
            px_srcline(451);
            _v166 = px_call(px_get_global("scan_string_tokens"), (LXValue[]){px_bool(true)}, 1);
            px_srcline(452);
            _v167 = px_int(0LL);
            px_srcline(453);
            while (px_is_truthy(px_lt(_v167, px_call(px_get_global("len"), (LXValue[]){_v166}, 1)))) {
                px_srcline(454);
                (void)(px_method(_v164, "append", (LXValue[]){px_index(_v166, _v167)}, 1));
                px_srcline(455);
                 _v167 = px_add(_v167, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v165}, 1))) {
            px_srcline(457);
            (void)(px_method(_v164, "append", (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v165}, 1))) {
            px_srcline(459);
            (void)(px_method(_v164, "append", (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(461);
            (void)(px_method(_v164, "append", (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
        }
    }
    px_srcline(462);
    return _v164;
px_err_168:
    if (px_err_168_proped) return px_err_168_val;
    return px_null();
}

static LXValue fn_lx_nul_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("lx_nul_char");
    LXValue px_err_173_val = px_null();
    int px_err_173_proped = 0;
    px_srcline(468);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){px_int(0LL), px_int(1LL)}, 2)}, 1);
px_err_173:
    if (px_err_173_proped) return px_err_173_val;
    return px_null();
}

static LXValue fn_hex_to_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("hex_to_char");
    LXValue _v174 = (nargs > 0) ? args[0] : px_null();
    LXValue _v175 = px_null();
    LXValue _v176 = px_null();
    LXValue _v177 = px_null();
    LXValue _v178 = px_null();
    LXValue _v179 = px_null();
    LXValue px_err_180_val = px_null();
    int px_err_180_proped = 0;
    px_srcline(470);
    _v175 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v174}, 1);
    px_srcline(471);
    if (px_is_truthy(px_le(_v175, px_int(127LL)))) {
        px_srcline(472);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){_v175, px_int(1LL)}, 2)}, 1);
    }
    px_srcline(473);
    if (px_is_truthy(px_le(_v175, px_int(2047LL)))) {
        px_srcline(474);
        _v176 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(192LL), px_shr(_v175, px_int(6LL))), px_int(1LL)}, 2);
        px_srcline(475);
        _v177 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v175, px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(476);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v176, _v177}, 2)}, 1);
    }
    px_srcline(477);
    if (px_is_truthy(px_le(_v175, px_int(65535LL)))) {
        px_srcline(478);
        _v176 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(224LL), px_shr(_v175, px_int(12LL))), px_int(1LL)}, 2);
        px_srcline(479);
        _v177 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v175, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(480);
        _v178 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v175, px_int(63LL))), px_int(1LL)}, 2);
        px_srcline(481);
        return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v176, _v177, _v178}, 3)}, 1);
    }
    px_srcline(482);
    _v176 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(240LL), px_shr(_v175, px_int(18LL))), px_int(1LL)}, 2);
    px_srcline(483);
    _v177 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v175, px_int(12LL)), px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(484);
    _v178 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(px_shr(_v175, px_int(6LL)), px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(485);
    _v179 = px_call(px_get_global("int_to_bytes"), (LXValue[]){px_bitor(px_int(128LL), px_bitand(_v175, px_int(63LL))), px_int(1LL)}, 2);
    px_srcline(486);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("bytes_concat"), (LXValue[]){_v176, _v177, _v178, _v179}, 4)}, 1);
px_err_180:
    if (px_err_180_proped) return px_err_180_val;
    return px_null();
}

static LXValue fn_scan_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_escape");
    LXValue _v181 = (nargs > 0) ? args[0] : px_null();
    LXValue _v182 = (nargs > 1) ? args[1] : px_null();
    LXValue _v183 = px_null();
    LXValue _v184 = px_null();
    LXValue _v185 = px_null();
    LXValue _v186 = px_null();
    LXValue px_err_187_val = px_null();
    int px_err_187_proped = 0;
    px_srcline(488);
    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
    px_srcline(489);
    _v183 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    px_srcline(490);
    if (px_is_truthy(px_eq(_v183, px_str("")))) {
        px_srcline(491);
        (void)(px_call(px_get_global("err_at"), (LXValue[]){px_str("E1002"), px_str("字符串在转义序列处意外结束"), _v181, _v182}, 4));
    }
    px_srcline(492);
    if (px_is_truthy(px_eq(_v183, px_str("n")))) {
        px_srcline(493);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(494);
        return px_str("\n");
    }
    px_srcline(495);
    if (px_is_truthy(px_eq(_v183, px_str("t")))) {
        px_srcline(496);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(497);
        return px_str("\t");
    }
    px_srcline(498);
    if (px_is_truthy(px_eq(_v183, px_str("r")))) {
        px_srcline(499);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(500);
        return px_str("\r");
    }
    px_srcline(501);
    if (px_is_truthy(px_eq(_v183, px_str("\\")))) {
        px_srcline(502);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(503);
        return px_str("\\");
    }
    px_srcline(504);
    if (px_is_truthy(px_eq(_v183, px_str("\"")))) {
        px_srcline(505);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(506);
        return px_str("\"");
    }
    px_srcline(507);
    if (px_is_truthy(px_eq(_v183, px_str("'")))) {
        px_srcline(508);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(509);
        return px_str("'");
    }
    px_srcline(510);
    if (px_is_truthy(px_eq(_v183, px_str("0")))) {
        px_srcline(511);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(512);
        return px_call(px_get_global("lx_nul_char"), (LXValue[]){}, 0);
    }
    px_srcline(513);
    if (px_is_truthy(px_eq(_v183, px_str("$")))) {
        px_srcline(514);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(515);
        return px_str("$");
    }
    px_srcline(516);
    if (px_is_truthy(px_eq(_v183, px_str("u")))) {
        px_srcline(517);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(518);
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("{")))) {
            px_srcline(519);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义须为 \\u{XXXX} 形式")}, 2));
        }
        px_srcline(520);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(521);
        _v184 = px_str("");
        px_srcline(522);
        while (px_is_truthy(({ LXValue _t188 = px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("")); px_is_truthy(_t188) ? px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")) : _t188; }))) {
            px_srcline(523);
            _v185 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
            px_srcline(524);
            if (px_is_truthy(px_call(px_get_global("is_hex_digit"), (LXValue[]){_v185}, 1))) {
                px_srcline(525);
                 _v184 = px_add(_v184, _v185);
                px_srcline(526);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            }
            else {
                px_srcline(528);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义含非法字符")}, 2));
            }
        }
        px_srcline(529);
        if (px_is_truthy(px_ne(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("}")))) {
            px_srcline(530);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义缺少 }")}, 2));
        }
        px_srcline(531);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(532);
        if (px_is_truthy(px_eq(_v184, px_str("")))) {
            px_srcline(533);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        px_srcline(534);
        _v186 = px_call(px_get_global("hex_to_int"), (LXValue[]){_v184}, 1);
        px_srcline(535);
        if (px_is_truthy(({ LXValue _t189 = px_eq(_v186, px_null()); px_is_truthy(_t189) ? _t189 : px_gt(_v186, px_int(4294967295LL)); }))) {
            px_srcline(536);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_str("Unicode 转义无效")}, 2));
        }
        px_srcline(537);
        if (px_is_truthy(({ LXValue _t191 = px_gt(_v186, px_int(1114111LL)); px_is_truthy(_t191) ? _t191 : ({ LXValue _t190 = px_ge(_v186, px_int(55296LL)); px_is_truthy(_t190) ? px_le(_v186, px_int(57343LL)) : _t190; }); }))) {
            px_srcline(538);
            return px_call(px_get_global("hex_to_char"), (LXValue[]){px_str("FFFD")}, 1);
        }
        px_srcline(539);
        return px_call(px_get_global("hex_to_char"), (LXValue[]){_v184}, 1);
    }
    px_srcline(540);
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1005"), px_add(px_str("非法转义序列 \\"), _v183)}, 2));
    px_srcline(541);
    return px_str("");
px_err_187:
    if (px_err_187_proped) return px_err_187_val;
    return px_null();
}

static LXValue fn_int_to_hex_nopad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("int_to_hex_nopad");
    LXValue _v192 = (nargs > 0) ? args[0] : px_null();
    LXValue _v193 = px_null();
    LXValue _v194 = px_null();
    LXValue px_err_195_val = px_null();
    int px_err_195_proped = 0;
    px_srcline(544);
    _v193 = px_str("0123456789abcdef");
    px_srcline(545);
    _v194 = px_str("");
    px_srcline(546);
    while (px_is_truthy(px_gt(_v192, px_int(0LL)))) {
        px_srcline(547);
         _v194 = px_add(px_index(_v193, px_mod(_v192, px_int(16LL))), _v194);
        px_srcline(548);
         _v192 = px_idiv(_v192, px_int(16LL));
    }
    px_srcline(549);
    if (px_is_truthy(px_eq(_v194, px_str("")))) {
        px_srcline(550);
        return px_str("0");
    }
    px_srcline(551);
    return _v194;
px_err_195:
    if (px_err_195_proped) return px_err_195_val;
    return px_null();
}

static LXValue fn_char_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("char_debug");
    LXValue _v196 = (nargs > 0) ? args[0] : px_null();
    LXValue _v197 = px_null();
    LXValue px_err_198_val = px_null();
    int px_err_198_proped = 0;
    px_srcline(553);
    if (px_is_truthy(px_eq(_v196, px_str("'")))) {
        px_srcline(554);
        return px_str("'\\''");
    }
    px_srcline(555);
    if (px_is_truthy(px_eq(_v196, px_str("\\")))) {
        px_srcline(556);
        return px_str("'\\\\'");
    }
    px_srcline(557);
    if (px_is_truthy(px_eq(_v196, px_str("\n")))) {
        px_srcline(558);
        return px_str("'\\n'");
    }
    px_srcline(559);
    if (px_is_truthy(px_eq(_v196, px_str("\r")))) {
        px_srcline(560);
        return px_str("'\\r'");
    }
    px_srcline(561);
    if (px_is_truthy(px_eq(_v196, px_str("\t")))) {
        px_srcline(562);
        return px_str("'\\t'");
    }
    px_srcline(563);
    if (px_is_truthy(px_eq(_v196, px_call(px_get_global("lx_nul_char"), (LXValue[]){}, 0)))) {
        px_srcline(564);
        return px_str("'\\0'");
    }
    px_srcline(565);
    _v197 = px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v196}, 1);
    px_srcline(566);
    if (px_is_truthy(px_ge(_v197, px_int(0LL)))) {
        px_srcline(567);
        return px_add(px_add(px_str("'\\u{"), px_call(px_get_global("int_to_hex_nopad"), (LXValue[]){_v197}, 1)), px_str("}'"));
    }
    px_srcline(568);
    return px_add(px_add(px_str("'"), _v196), px_str("'"));
px_err_198:
    if (px_err_198_proped) return px_err_198_val;
    return px_null();
}

static LXValue fn_rust_str_debug(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_str_debug");
    LXValue _v199 = (nargs > 0) ? args[0] : px_null();
    LXValue _v200 = px_null();
    LXValue _v201 = px_null();
    LXValue _v202 = px_null();
    LXValue px_err_203_val = px_null();
    int px_err_203_proped = 0;
    px_srcline(573);
    _v200 = px_list_n((LXValue[]){px_str("\"")}, 1);
    px_srcline(574);
    _v201 = px_int(0LL);
    px_srcline(575);
    while (px_is_truthy(px_lt(_v201, px_call(px_get_global("len"), (LXValue[]){_v199}, 1)))) {
        px_srcline(576);
        _v202 = px_index(_v199, _v201);
        px_srcline(577);
        if (px_is_truthy(px_eq(_v202, px_str("\n")))) {
            px_srcline(578);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\n")}, 1));
        }
        else if (px_is_truthy(px_eq(_v202, px_str("\t")))) {
            px_srcline(580);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\t")}, 1));
        }
        else if (px_is_truthy(px_eq(_v202, px_str("\r")))) {
            px_srcline(582);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\r")}, 1));
        }
        else if (px_is_truthy(px_lt(_v202, px_str("")))) {
            px_srcline(584);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\0")}, 1));
        }
        else if (px_is_truthy(px_eq(_v202, px_str("\"")))) {
            px_srcline(586);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\\"")}, 1));
        }
        else if (px_is_truthy(px_eq(_v202, px_str("\\")))) {
            px_srcline(588);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\\\")}, 1));
        }
        else if (px_is_truthy(({ LXValue _t204 = px_ge(_v202, px_str(" ")); px_is_truthy(_t204) ? px_le(_v202, px_str("~")) : _t204; }))) {
            px_srcline(590);
            (void)(px_method(_v200, "append", (LXValue[]){_v202}, 1));
        }
        else if (px_is_truthy(px_eq(_v202, px_str(" ")))) {
            px_srcline(592);
            (void)(px_method(_v200, "append", (LXValue[]){px_str("\\u{a0}")}, 1));
        }
        else if (px_is_truthy(px_gt(_v202, px_str(" ")))) {
            px_srcline(594);
            (void)(px_method(_v200, "append", (LXValue[]){_v202}, 1));
        }
        else {
            px_srcline(596);
            (void)(px_method(_v200, "append", (LXValue[]){px_add(px_add(px_str("\\u{"), px_call(px_get_global("ctrl_hex"), (LXValue[]){_v202}, 1)), px_str("}"))}, 1));
        }
        px_srcline(597);
         _v201 = px_add(_v201, px_int(1LL));
    }
    px_srcline(598);
    (void)(px_method(_v200, "append", (LXValue[]){px_str("\"")}, 1));
    px_srcline(599);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v200}, 2);
px_err_203:
    if (px_err_203_proped) return px_err_203_val;
    return px_null();
}

static LXValue fn_ctrl_codepoint(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ctrl_codepoint");
    LXValue _v205 = (nargs > 0) ? args[0] : px_null();
    LXValue _v206 = px_null();
    LXValue px_err_207_val = px_null();
    int px_err_207_proped = 0;
    px_srcline(601);
    _v206 = px_int(0LL);
    px_srcline(602);
    while (px_is_truthy(px_lt(_v206, px_call(px_get_global("len"), (LXValue[]){px_get_global("CTRL_ALL")}, 1)))) {
        px_srcline(603);
        if (px_is_truthy(px_eq(px_index(px_get_global("CTRL_ALL"), _v206), _v205))) {
            px_srcline(604);
            if (px_is_truthy(px_lt(_v206, px_int(28LL)))) {
                px_srcline(605);
                if (px_is_truthy(px_lt(_v206, px_int(8LL)))) {
                    px_srcline(606);
                    return px_add(_v206, px_int(1LL));
                }
                px_srcline(607);
                if (px_is_truthy(px_lt(_v206, px_int(10LL)))) {
                    px_srcline(608);
                    return px_add(_v206, px_int(3LL));
                }
                px_srcline(609);
                return px_add(_v206, px_int(4LL));
            }
            px_srcline(610);
            return px_add(_v206, px_int(99LL));
        }
        px_srcline(611);
         _v206 = px_add(_v206, px_int(1LL));
    }
    px_srcline(612);
    return px_neg(px_int(1LL));
px_err_207:
    if (px_err_207_proped) return px_err_207_val;
    return px_null();
}

static LXValue fn_ctrl_hex(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ctrl_hex");
    LXValue _v208 = (nargs > 0) ? args[0] : px_null();
    LXValue _v209 = px_null();
    LXValue _v210 = px_null();
    LXValue px_err_211_val = px_null();
    int px_err_211_proped = 0;
    px_srcline(614);
    _v209 = px_call(px_get_global("int_to_hex"), (LXValue[]){px_call(px_get_global("ctrl_codepoint"), (LXValue[]){_v208}, 1), px_int(16LL)}, 2);
    px_srcline(615);
    _v210 = px_int(0LL);
    px_srcline(616);
    while (px_is_truthy(({ LXValue _t212 = px_lt(_v210, px_call(px_get_global("len"), (LXValue[]){_v209}, 1)); px_is_truthy(_t212) ? px_eq(px_index(_v209, _v210), px_str("0")) : _t212; }))) {
        px_srcline(617);
         _v210 = px_add(_v210, px_int(1LL));
    }
    px_srcline(618);
    if (px_is_truthy(px_eq(_v210, px_call(px_get_global("len"), (LXValue[]){_v209}, 1)))) {
        px_srcline(619);
        return px_str("0");
    }
    px_srcline(620);
    return px_slice(_v209, _v210, px_call(px_get_global("len"), (LXValue[]){_v209}, 1), px_null());
px_err_211:
    if (px_err_211_proped) return px_err_211_val;
    return px_null();
}

static LXValue fn_scan_operator_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("scan_operator_token");
    LXValue _v213 = px_null();
    LXValue _v214 = px_null();
    LXValue _v215 = px_null();
    LXValue px_err_216_val = px_null();
    int px_err_216_proped = 0;
    px_srcline(623);
    _v213 = px_get_global("g_line");
    px_srcline(624);
    _v214 = px_get_global("g_col");
    px_srcline(625);
    _v215 = px_call(px_get_global("advance"), (LXValue[]){}, 0);
    px_srcline(626);
    if (px_is_truthy(px_eq(_v215, px_str("(")))) {
        px_srcline(627);
        return px_list_n((LXValue[]){px_str("("), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(628);
    if (px_is_truthy(px_eq(_v215, px_str(")")))) {
        px_srcline(629);
        return px_list_n((LXValue[]){px_str(")"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(630);
    if (px_is_truthy(px_eq(_v215, px_str("[")))) {
        px_srcline(631);
        return px_list_n((LXValue[]){px_str("["), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(632);
    if (px_is_truthy(px_eq(_v215, px_str("]")))) {
        px_srcline(633);
        return px_list_n((LXValue[]){px_str("]"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(634);
    if (px_is_truthy(px_eq(_v215, px_str("{")))) {
        px_srcline(635);
        return px_list_n((LXValue[]){px_str("{"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(636);
    if (px_is_truthy(px_eq(_v215, px_str("}")))) {
        px_srcline(637);
        return px_list_n((LXValue[]){px_str("}"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(638);
    if (px_is_truthy(px_eq(_v215, px_str(",")))) {
        px_srcline(639);
        return px_list_n((LXValue[]){px_str(","), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(640);
    if (px_is_truthy(px_eq(_v215, px_str(":")))) {
        px_srcline(641);
        return px_list_n((LXValue[]){px_str(":"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(642);
    if (px_is_truthy(px_eq(_v215, px_str(".")))) {
        px_srcline(643);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            px_srcline(644);
            (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_str("运算符 '..' 未定义（range 语法尚未支持）")}, 2));
        }
        px_srcline(645);
        return px_list_n((LXValue[]){px_str("."), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(646);
    if (px_is_truthy(px_eq(_v215, px_str("+")))) {
        px_srcline(647);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(648);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(649);
            return px_list_n((LXValue[]){px_str("+="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(650);
        return px_list_n((LXValue[]){px_str("+"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(651);
    if (px_is_truthy(px_eq(_v215, px_str("-")))) {
        px_srcline(652);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(653);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(654);
            return px_list_n((LXValue[]){px_str("->"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(655);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(656);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(657);
            return px_list_n((LXValue[]){px_str("-="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(658);
        return px_list_n((LXValue[]){px_str("-"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(659);
    if (px_is_truthy(px_eq(_v215, px_str("*")))) {
        px_srcline(660);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("*")))) {
            px_srcline(661);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(662);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(663);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(664);
                return px_list_n((LXValue[]){px_str("**="), px_str(""), _v213, _v214}, 4);
            }
            px_srcline(665);
            return px_list_n((LXValue[]){px_str("**"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(666);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(667);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(668);
            return px_list_n((LXValue[]){px_str("*="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(669);
        return px_list_n((LXValue[]){px_str("*"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(670);
    if (px_is_truthy(px_eq(_v215, px_str("/")))) {
        px_srcline(671);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("/")))) {
            px_srcline(672);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(673);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(674);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(675);
                return px_list_n((LXValue[]){px_str("//="), px_str(""), _v213, _v214}, 4);
            }
            px_srcline(676);
            return px_list_n((LXValue[]){px_str("//"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(677);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(678);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(679);
            return px_list_n((LXValue[]){px_str("/="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(680);
        return px_list_n((LXValue[]){px_str("/"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(681);
    if (px_is_truthy(px_eq(_v215, px_str("%")))) {
        px_srcline(682);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(683);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(684);
            return px_list_n((LXValue[]){px_str("%="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(685);
        return px_list_n((LXValue[]){px_str("%"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(686);
    if (px_is_truthy(px_eq(_v215, px_str("^")))) {
        px_srcline(687);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(688);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(689);
            return px_list_n((LXValue[]){px_str("^="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(690);
        return px_list_n((LXValue[]){px_str("^"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(691);
    if (px_is_truthy(px_eq(_v215, px_str("~")))) {
        px_srcline(692);
        return px_list_n((LXValue[]){px_str("~"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(693);
    if (px_is_truthy(px_eq(_v215, px_str("&")))) {
        px_srcline(694);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(695);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(696);
            return px_list_n((LXValue[]){px_str("&="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(697);
        return px_list_n((LXValue[]){px_str("&"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(698);
    if (px_is_truthy(px_eq(_v215, px_str("|")))) {
        px_srcline(699);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(700);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(701);
            return px_list_n((LXValue[]){px_str("|>"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(702);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(703);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(704);
            return px_list_n((LXValue[]){px_str("|="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(705);
        return px_list_n((LXValue[]){px_str("|"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(706);
    if (px_is_truthy(px_eq(_v215, px_str("=")))) {
        px_srcline(707);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(708);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(709);
            return px_list_n((LXValue[]){px_str("=="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(710);
        return px_list_n((LXValue[]){px_str("="), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(711);
    if (px_is_truthy(px_eq(_v215, px_str("!")))) {
        px_srcline(712);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(713);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(714);
            return px_list_n((LXValue[]){px_str("!="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(715);
        return px_list_n((LXValue[]){px_str("!"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(716);
    if (px_is_truthy(px_eq(_v215, px_str("<")))) {
        px_srcline(717);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("-")))) {
            px_srcline(720);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(721);
            return px_list_n((LXValue[]){px_str("<-"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(722);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("<")))) {
            px_srcline(723);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(724);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(725);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(726);
                return px_list_n((LXValue[]){px_str("<<="), px_str(""), _v213, _v214}, 4);
            }
            px_srcline(727);
            return px_list_n((LXValue[]){px_str("<<"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(728);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(729);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(730);
            return px_list_n((LXValue[]){px_str("<="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(731);
        return px_list_n((LXValue[]){px_str("<"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(732);
    if (px_is_truthy(px_eq(_v215, px_str(">")))) {
        px_srcline(733);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
            px_srcline(734);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(735);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(">")))) {
                px_srcline(736);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(737);
                if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                    px_srcline(738);
                    (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                    px_srcline(739);
                    return px_list_n((LXValue[]){px_str(">>>="), px_str(""), _v213, _v214}, 4);
                }
                px_srcline(740);
                return px_list_n((LXValue[]){px_str(">>>"), px_str(""), _v213, _v214}, 4);
            }
            px_srcline(741);
            if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
                px_srcline(742);
                (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
                px_srcline(743);
                return px_list_n((LXValue[]){px_str(">>="), px_str(""), _v213, _v214}, 4);
            }
            px_srcline(744);
            return px_list_n((LXValue[]){px_str(">>"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(745);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("=")))) {
            px_srcline(746);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(747);
            return px_list_n((LXValue[]){px_str(">="), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(748);
        return px_list_n((LXValue[]){px_str(">"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(749);
    if (px_is_truthy(px_eq(_v215, px_str("?")))) {
        px_srcline(750);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str(".")))) {
            px_srcline(751);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(752);
            return px_list_n((LXValue[]){px_str("?."), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(753);
        if (px_is_truthy(px_eq(px_call(px_get_global("peek"), (LXValue[]){}, 0), px_str("?")))) {
            px_srcline(754);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(755);
            return px_list_n((LXValue[]){px_str("??"), px_str(""), _v213, _v214}, 4);
        }
        px_srcline(756);
        return px_list_n((LXValue[]){px_str("?"), px_str(""), _v213, _v214}, 4);
    }
    px_srcline(757);
    (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E1001"), px_add(px_str("非法字符: "), px_call(px_get_global("char_debug"), (LXValue[]){_v215}, 1))}, 2));
    px_srcline(758);
    return px_list_n((LXValue[]){px_str(""), px_str(""), _v213, _v214}, 4);
px_err_216:
    if (px_err_216_proped) return px_err_216_val;
    return px_null();
}

static LXValue fn_next_token(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("next_token");
    LXValue _v217 = px_null();
    LXValue _v218 = px_null();
    LXValue _v219 = px_null();
    LXValue _v220 = px_null();
    LXValue px_err_221_val = px_null();
    int px_err_221_proped = 0;
    px_srcline(761);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_int(0LL)))) {
        px_srcline(762);
        _v217 = px_index(px_get_global("g_pending"), px_int(0LL));
        px_srcline(763);
        px_set_global("g_pending", px_slice(px_get_global("g_pending"), px_int(1LL), px_call(px_get_global("len"), (LXValue[]){px_get_global("g_pending")}, 1), px_null()));
        px_srcline(764);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){_v217}, 1));
        px_srcline(765);
        return px_bool(true);
    }
    px_srcline(766);
    if (px_is_truthy(px_get_global("g_at_line_start"))) {
        px_srcline(767);
        (void)(px_call(px_get_global("handle_line_start"), (LXValue[]){}, 0));
    }
    px_srcline(768);
    _v218 = px_call(px_get_global("peek"), (LXValue[]){}, 0);
    px_srcline(769);
    if (px_is_truthy(px_eq(_v218, px_str("")))) {
        px_srcline(770);
        while (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("g_indent_stack")}, 1), px_int(1LL)))) {
            px_srcline(771);
            (void)(px_method(px_get_global("g_indent_stack"), "pop", (LXValue[]){}, 0));
            px_srcline(772);
            (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("去缩进"), px_str("")}, 2));
        }
        px_srcline(773);
        (void)(px_call(px_get_global("emit"), (LXValue[]){px_str("EOF"), px_str("")}, 2));
        px_srcline(774);
        return px_bool(false);
    }
    px_srcline(775);
    if (px_is_truthy(px_eq(_v218, px_str("\n")))) {
        px_srcline(785);
        if (px_is_truthy(({ LXValue _t222 = px_eq(px_get_global("g_bracket_depth"), px_int(0LL)); px_is_truthy(_t222) ? px_call(px_get_global("is_cont_op"), (LXValue[]){px_get_global("g_last_kind")}, 1) : _t222; }))) {
            px_srcline(786);
            (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
            px_srcline(787);
            return px_bool(true);
        }
        px_srcline(788);
        _v219 = px_get_global("g_line");
        px_srcline(789);
        _v220 = px_get_global("g_col");
        px_srcline(790);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(791);
        px_set_global("g_at_line_start", px_bool(true));
        px_srcline(792);
        (void)(px_call(px_get_global("emit_at"), (LXValue[]){px_str("换行"), px_str(""), _v219, _v220}, 4));
        px_srcline(793);
        return px_bool(true);
    }
    px_srcline(794);
    if (px_is_truthy(({ LXValue _t223 = px_eq(_v218, px_str(" ")); px_is_truthy(_t223) ? _t223 : px_eq(_v218, px_str("\t")); }))) {
        px_srcline(795);
        (void)(px_call(px_get_global("advance"), (LXValue[]){}, 0));
        px_srcline(796);
        return px_bool(true);
    }
    px_srcline(797);
    if (px_is_truthy(px_eq(_v218, px_str("#")))) {
        px_srcline(798);
        (void)(px_call(px_get_global("skip_comment"), (LXValue[]){}, 0));
        px_srcline(799);
        return px_bool(true);
    }
    px_srcline(800);
    if (px_is_truthy(({ LXValue _t224 = px_eq(_v218, px_str("\"")); px_is_truthy(_t224) ? _t224 : px_eq(_v218, px_str("'")); }))) {
        px_srcline(801);
        (void)(px_call(px_get_global("scan_string"), (LXValue[]){px_bool(true)}, 1));
        px_srcline(802);
        return px_bool(true);
    }
    px_srcline(803);
    if (px_is_truthy(px_call(px_get_global("is_digit"), (LXValue[]){_v218}, 1))) {
        px_srcline(804);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_number_token"), (LXValue[]){}, 0)}, 1));
        px_srcline(805);
        return px_bool(true);
    }
    px_srcline(806);
    if (px_is_truthy(px_call(px_get_global("is_ident_start"), (LXValue[]){_v218}, 1))) {
        px_srcline(807);
        (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_ident_token"), (LXValue[]){}, 0)}, 1));
        px_srcline(808);
        return px_bool(true);
    }
    px_srcline(809);
    (void)(px_call(px_get_global("emit_token"), (LXValue[]){px_call(px_get_global("scan_operator_token"), (LXValue[]){}, 0)}, 1));
    px_srcline(810);
    return px_bool(true);
px_err_221:
    if (px_err_221_proped) return px_err_221_val;
    return px_null();
}

static LXValue fn_check_edition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("check_edition");
    LXValue _v225 = (nargs > 0) ? args[0] : px_null();
    LXValue _v226 = px_null();
    LXValue _v227 = px_null();
    LXValue _v228 = px_null();
    LXValue _v229 = px_null();
    LXValue _v230 = px_null();
    LXValue _v231 = px_null();
    LXValue _v232 = px_null();
    LXValue px_err_233_val = px_null();
    int px_err_233_proped = 0;
    px_srcline(815);
    _v226 = px_int(0LL);
    px_srcline(816);
    _v227 = px_call(px_get_global("len"), (LXValue[]){_v225}, 1);
    px_srcline(817);
    _v228 = px_str("");
    px_srcline(818);
    while (px_is_truthy(({ LXValue _t234 = px_lt(_v226, _v227); px_is_truthy(_t234) ? px_ne(px_index(_v225, _v226), px_str("\n")) : _t234; }))) {
        px_srcline(819);
         _v228 = px_add(_v228, px_index(_v225, _v226));
        px_srcline(820);
         _v226 = px_add(_v226, px_int(1LL));
    }
    px_srcline(821);
    _v229 = px_call(px_get_global("trim"), (LXValue[]){_v228}, 1);
    px_srcline(822);
    _v230 = px_method(_v229, "split", (LXValue[]){px_str(" ")}, 1);
    px_srcline(823);
    if (px_is_truthy(({ LXValue _t236 = ({ LXValue _t235 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v230}, 1), px_int(3LL)); px_is_truthy(_t235) ? px_eq(px_index(_v230, px_int(0LL)), px_str("#")) : _t235; }); px_is_truthy(_t236) ? px_eq(px_index(_v230, px_int(1LL)), px_str("px")) : _t236; }))) {
        px_srcline(824);
        _v231 = px_index(_v230, px_int(2LL));
        px_srcline(825);
        if (px_is_truthy(({ LXValue _t240 = ({ LXValue _t239 = ({ LXValue _t238 = ({ LXValue _t237 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v231}, 1), px_int(4LL)); px_is_truthy(_t237) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v231, px_int(0LL))}, 1) : _t237; }); px_is_truthy(_t238) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v231, px_int(1LL))}, 1) : _t238; }); px_is_truthy(_t239) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v231, px_int(2LL))}, 1) : _t239; }); px_is_truthy(_t240) ? px_call(px_get_global("is_digit"), (LXValue[]){px_index(_v231, px_int(3LL))}, 1) : _t240; }))) {
            px_srcline(826);
            _v232 = px_call(px_get_global("int"), (LXValue[]){_v231}, 1);
            px_srcline(827);
            if (px_is_truthy(px_gt(_v232, px_int(2026LL)))) {
                px_srcline(828);
                (void)(px_call(px_get_global("err"), (LXValue[]){px_str("E-EDITION"), px_add(px_add(px_str("源码声明 edition px "), _v231), px_str(" 高于当前工具链支持（px 2026），请升级编译器"))}, 2));
            }
        }
    }
px_err_233:
    if (px_err_233_proped) return px_err_233_val;
    return px_null();
}

static LXValue fn_lex_tokens(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("lex_tokens");
    LXValue _v241 = (nargs > 0) ? args[0] : px_null();
    LXValue _v242 = px_null();
    LXValue px_err_243_val = px_null();
    int px_err_243_proped = 0;
    px_srcline(831);
    (void)(px_call(px_get_global("check_edition"), (LXValue[]){_v241}, 1));
    px_srcline(832);
    px_set_global("g_src", _v241);
    px_srcline(833);
    px_set_global("g_len", px_call(px_get_global("len"), (LXValue[]){_v241}, 1));
    px_srcline(834);
    px_set_global("g_pos", px_int(0LL));
    px_srcline(835);
    px_set_global("g_line", px_int(1LL));
    px_srcline(836);
    px_set_global("g_col", px_int(1LL));
    px_srcline(837);
    px_set_global("g_indent_stack", px_list_n((LXValue[]){px_int(0LL)}, 1));
    px_srcline(838);
    px_set_global("g_at_line_start", px_bool(true));
    px_srcline(839);
    px_set_global("g_bracket_depth", px_int(0LL));
    px_srcline(840);
    px_set_global("g_toks", px_list_n((LXValue[]){}, 0));
    px_srcline(841);
    px_set_global("g_count", px_int(0LL));
    px_srcline(842);
    px_set_global("g_pending", px_list_n((LXValue[]){}, 0));
    px_srcline(843);
    _v242 = px_bool(true);
    px_srcline(844);
    while (px_is_truthy(_v242)) {
        px_srcline(845);
         _v242 = px_call(px_get_global("next_token"), (LXValue[]){}, 0);
    }
    px_srcline(846);
    return px_get_global("g_toks");
px_err_243:
    if (px_err_243_proped) return px_err_243_val;
    return px_null();
}

static LXValue fn_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pad");
    LXValue _v244 = (nargs > 0) ? args[0] : px_null();
    LXValue _v245 = px_null();
    LXValue _v246 = px_null();
    LXValue px_err_247_val = px_null();
    int px_err_247_proped = 0;
    px_srcline(20);
    _v245 = px_str("");
    px_srcline(21);
    _v246 = px_int(0LL);
    px_srcline(22);
    while (px_is_truthy(px_lt(_v246, _v244))) {
        px_srcline(23);
         _v245 = px_add(_v245, px_str(" "));
        px_srcline(24);
         _v246 = px_add(_v246, px_int(1LL));
    }
    px_srcline(25);
    return _v245;
px_err_247:
    if (px_err_247_proped) return px_err_247_val;
    return px_null();
}

static LXValue fn_dump_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_node");
    LXValue _v248 = (nargs > 0) ? args[0] : px_null();
    LXValue _v249 = (nargs > 1) ? args[1] : px_null();
    LXValue _v250 = px_null();
    LXValue _v251 = px_null();
    LXValue _v252 = px_null();
    LXValue _v253 = px_null();
    LXValue _v254 = px_null();
    LXValue _v255 = px_null();
    LXValue _v256 = px_null();
    LXValue _v257 = px_null();
    LXValue _v258 = px_null();
    LXValue _v259 = px_null();
    LXValue px_err_260_val = px_null();
    int px_err_260_proped = 0;
    px_srcline(27);
    _v250 = px_index(_v248, px_int(0LL));
    px_srcline(28);
    _v251 = px_index(px_get_global("LAYOUT"), _v250);
    px_srcline(29);
    _v252 = px_index(_v251, px_int(0LL));
    px_srcline(30);
    _v253 = px_index(_v251, px_int(1LL));
    px_srcline(31);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v253}, 1), px_int(0LL)))) {
        px_srcline(32);
        return _v252;
    }
    px_srcline(33);
    _v254 = px_eq(px_index(px_index(_v253, px_int(0LL)), px_int(0LL)), px_null());
    px_srcline(34);
    _v255 = px_list_n((LXValue[]){}, 0);
    px_srcline(35);
    if (px_is_truthy(_v254)) {
        px_srcline(36);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(_v252, px_str("("))}, 1));
    }
    else {
        px_srcline(38);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(_v252, px_str(" {"))}, 1));
    }
    px_srcline(39);
    _v256 = px_int(0LL);
    px_srcline(40);
    while (px_is_truthy(px_lt(_v256, px_call(px_get_global("len"), (LXValue[]){_v253}, 1)))) {
        px_srcline(41);
        _v257 = px_index(_v253, _v256);
        px_srcline(42);
        _v258 = px_index(_v248, px_add(_v256, px_int(1LL)));
        px_srcline(43);
        _v259 = px_call(px_get_global("dump_field"), (LXValue[]){_v258, px_index(_v257, px_int(1LL)), px_add(_v249, px_int(4LL))}, 3);
        px_srcline(44);
        if (px_is_truthy(_v254)) {
            px_srcline(45);
            (void)(px_method(_v255, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v249, px_int(4LL))}, 1), _v259), px_str(","))}, 1));
        }
        else {
            px_srcline(47);
            (void)(px_method(_v255, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v249, px_int(4LL))}, 1), px_index(_v257, px_int(0LL))), px_str(": ")), _v259), px_str(","))}, 1));
        }
        px_srcline(48);
         _v256 = px_add(_v256, px_int(1LL));
    }
    px_srcline(49);
    if (px_is_truthy(_v254)) {
        px_srcline(50);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v249}, 1), px_str(")"))}, 1));
    }
    else {
        px_srcline(52);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v249}, 1), px_str("}"))}, 1));
    }
    px_srcline(53);
    return px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v255}, 2);
px_err_260:
    if (px_err_260_proped) return px_err_260_val;
    return px_null();
}

static LXValue fn_dump_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_list");
    LXValue _v261 = (nargs > 0) ? args[0] : px_null();
    LXValue _v262 = (nargs > 1) ? args[1] : px_null();
    LXValue _v263 = px_null();
    LXValue _v264 = px_null();
    LXValue px_err_265_val = px_null();
    int px_err_265_proped = 0;
    px_srcline(55);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v261}, 1), px_int(0LL)))) {
        px_srcline(56);
        return px_str("[]");
    }
    px_srcline(57);
    _v263 = px_list_n((LXValue[]){}, 0);
    px_srcline(58);
    _v264 = px_int(0LL);
    px_srcline(59);
    while (px_is_truthy(px_lt(_v264, px_call(px_get_global("len"), (LXValue[]){_v261}, 1)))) {
        px_srcline(60);
        (void)(px_method(_v263, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v262, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v261, _v264), px_add(_v262, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(61);
         _v264 = px_add(_v264, px_int(1LL));
    }
    px_srcline(62);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v263}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v262}, 1)), px_str("]"));
px_err_265:
    if (px_err_265_proped) return px_err_265_val;
    return px_null();
}

static LXValue fn_dump_str_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_str_list");
    LXValue _v266 = (nargs > 0) ? args[0] : px_null();
    LXValue _v267 = (nargs > 1) ? args[1] : px_null();
    LXValue _v268 = px_null();
    LXValue _v269 = px_null();
    LXValue px_err_270_val = px_null();
    int px_err_270_proped = 0;
    px_srcline(64);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v266}, 1), px_int(0LL)))) {
        px_srcline(65);
        return px_str("[]");
    }
    px_srcline(66);
    _v268 = px_list_n((LXValue[]){}, 0);
    px_srcline(67);
    _v269 = px_int(0LL);
    px_srcline(68);
    while (px_is_truthy(px_lt(_v269, px_call(px_get_global("len"), (LXValue[]){_v266}, 1)))) {
        px_srcline(69);
        (void)(px_method(_v268, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v267, px_int(4LL))}, 1), px_index(_v266, _v269)), px_str(","))}, 1));
        px_srcline(70);
         _v269 = px_add(_v269, px_int(1LL));
    }
    px_srcline(71);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v268}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v267}, 1)), px_str("]"));
px_err_270:
    if (px_err_270_proped) return px_err_270_val;
    return px_null();
}

static LXValue fn_dump_ty_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_ty_list");
    LXValue _v271 = (nargs > 0) ? args[0] : px_null();
    LXValue _v272 = (nargs > 1) ? args[1] : px_null();
    LXValue _v273 = px_null();
    LXValue _v274 = px_null();
    LXValue px_err_275_val = px_null();
    int px_err_275_proped = 0;
    px_srcline(73);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v271}, 1), px_int(0LL)))) {
        px_srcline(74);
        return px_str("[]");
    }
    px_srcline(75);
    _v273 = px_list_n((LXValue[]){}, 0);
    px_srcline(76);
    _v274 = px_int(0LL);
    px_srcline(77);
    while (px_is_truthy(px_lt(_v274, px_call(px_get_global("len"), (LXValue[]){_v271}, 1)))) {
        px_srcline(78);
        (void)(px_method(_v273, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v272, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v271, _v274), px_add(_v272, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(79);
         _v274 = px_add(_v274, px_int(1LL));
    }
    px_srcline(80);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v273}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v272}, 1)), px_str("]"));
px_err_275:
    if (px_err_275_proped) return px_err_275_val;
    return px_null();
}

static LXValue fn_dump_pat_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_pat_list");
    LXValue _v276 = (nargs > 0) ? args[0] : px_null();
    LXValue _v277 = (nargs > 1) ? args[1] : px_null();
    LXValue _v278 = px_null();
    LXValue _v279 = px_null();
    LXValue px_err_280_val = px_null();
    int px_err_280_proped = 0;
    px_srcline(82);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v276}, 1), px_int(0LL)))) {
        px_srcline(83);
        return px_str("[]");
    }
    px_srcline(84);
    _v278 = px_list_n((LXValue[]){}, 0);
    px_srcline(85);
    _v279 = px_int(0LL);
    px_srcline(86);
    while (px_is_truthy(px_lt(_v279, px_call(px_get_global("len"), (LXValue[]){_v276}, 1)))) {
        px_srcline(87);
        (void)(px_method(_v278, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v277, px_int(4LL))}, 1), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v276, _v279), px_add(_v277, px_int(4LL))}, 2)), px_str(","))}, 1));
        px_srcline(88);
         _v279 = px_add(_v279, px_int(1LL));
    }
    px_srcline(89);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v278}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v277}, 1)), px_str("]"));
px_err_280:
    if (px_err_280_proped) return px_err_280_val;
    return px_null();
}

static LXValue fn_dump_opt_node(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_node");
    LXValue _v281 = (nargs > 0) ? args[0] : px_null();
    LXValue _v282 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_283_val = px_null();
    int px_err_283_proped = 0;
    px_srcline(91);
    if (px_is_truthy(px_eq(_v281, px_null()))) {
        px_srcline(92);
        return px_str("None");
    }
    px_srcline(93);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v282, px_int(4LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){_v281, px_add(_v282, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v282}, 1)), px_str(")"));
px_err_283:
    if (px_err_283_proped) return px_err_283_val;
    return px_null();
}

static LXValue fn_dump_opt_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_str");
    LXValue _v284 = (nargs > 0) ? args[0] : px_null();
    LXValue _v285 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_286_val = px_null();
    int px_err_286_proped = 0;
    px_srcline(95);
    if (px_is_truthy(px_eq(_v284, px_null()))) {
        px_srcline(96);
        return px_str("None");
    }
    px_srcline(97);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(4LL))}, 1)), _v284), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v285}, 1)), px_str(")"));
px_err_286:
    if (px_err_286_proped) return px_err_286_val;
    return px_null();
}

static LXValue fn_dump_opt_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_opt_list");
    LXValue _v287 = (nargs > 0) ? args[0] : px_null();
    LXValue _v288 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_289_val = px_null();
    int px_err_289_proped = 0;
    px_srcline(99);
    if (px_is_truthy(px_eq(_v287, px_null()))) {
        px_srcline(100);
        return px_str("None");
    }
    px_srcline(101);
    return px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v288, px_int(4LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){_v287, px_add(_v288, px_int(4LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v288}, 1)), px_str(")"));
px_err_289:
    if (px_err_289_proped) return px_err_289_val;
    return px_null();
}

static LXValue fn_dump_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_pos");
    LXValue _v290 = (nargs > 0) ? args[0] : px_null();
    LXValue _v291 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_292_val = px_null();
    int px_err_292_proped = 0;
    px_srcline(103);
    return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v291, px_int(4LL))}, 1)), px_str("line: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v290, px_int(0LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v291, px_int(4LL))}, 1)), px_str("col: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v290, px_int(1LL))}, 1)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){_v291}, 1)), px_str("}"));
px_err_292:
    if (px_err_292_proped) return px_err_292_val;
    return px_null();
}

static LXValue fn_dump_t2_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t2_list");
    LXValue _v293 = (nargs > 0) ? args[0] : px_null();
    LXValue _v294 = (nargs > 1) ? args[1] : px_null();
    LXValue _v295 = px_null();
    LXValue _v296 = px_null();
    LXValue _v297 = px_null();
    LXValue px_err_298_val = px_null();
    int px_err_298_proped = 0;
    px_srcline(106);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v293}, 1), px_int(0LL)))) {
        px_srcline(107);
        return px_str("[]");
    }
    px_srcline(108);
    _v295 = px_list_n((LXValue[]){}, 0);
    px_srcline(109);
    _v296 = px_int(0LL);
    px_srcline(110);
    while (px_is_truthy(px_lt(_v296, px_call(px_get_global("len"), (LXValue[]){_v293}, 1)))) {
        px_srcline(111);
        _v297 = px_index(_v293, _v296);
        px_srcline(112);
        (void)(px_method(_v295, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v297, px_int(0LL)), px_add(_v294, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v297, px_int(1LL)), px_add(_v294, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(113);
         _v296 = px_add(_v296, px_int(1LL));
    }
    px_srcline(114);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v295}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v294}, 1)), px_str("]"));
px_err_298:
    if (px_err_298_proped) return px_err_298_val;
    return px_null();
}

static LXValue fn_dump_t2b_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t2b_list");
    LXValue _v299 = (nargs > 0) ? args[0] : px_null();
    LXValue _v300 = (nargs > 1) ? args[1] : px_null();
    LXValue _v301 = px_null();
    LXValue _v302 = px_null();
    LXValue _v303 = px_null();
    LXValue px_err_304_val = px_null();
    int px_err_304_proped = 0;
    px_srcline(117);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v299}, 1), px_int(0LL)))) {
        px_srcline(118);
        return px_str("[]");
    }
    px_srcline(119);
    _v301 = px_list_n((LXValue[]){}, 0);
    px_srcline(120);
    _v302 = px_int(0LL);
    px_srcline(121);
    while (px_is_truthy(px_lt(_v302, px_call(px_get_global("len"), (LXValue[]){_v299}, 1)))) {
        px_srcline(122);
        _v303 = px_index(_v299, _v302);
        px_srcline(123);
        (void)(px_method(_v301, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v303, px_int(0LL)), px_add(_v300, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v303, px_int(1LL)), px_add(_v300, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(124);
         _v302 = px_add(_v302, px_int(1LL));
    }
    px_srcline(125);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v301}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v300}, 1)), px_str("]"));
px_err_304:
    if (px_err_304_proped) return px_err_304_val;
    return px_null();
}

static LXValue fn_dump_t3_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_t3_list");
    LXValue _v305 = (nargs > 0) ? args[0] : px_null();
    LXValue _v306 = (nargs > 1) ? args[1] : px_null();
    LXValue _v307 = px_null();
    LXValue _v308 = px_null();
    LXValue _v309 = px_null();
    LXValue _v310 = px_null();
    LXValue _v311 = px_null();
    LXValue px_err_312_val = px_null();
    int px_err_312_proped = 0;
    px_srcline(128);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v305}, 1), px_int(0LL)))) {
        px_srcline(129);
        return px_str("[]");
    }
    px_srcline(130);
    _v307 = px_list_n((LXValue[]){}, 0);
    px_srcline(131);
    _v308 = px_int(0LL);
    px_srcline(132);
    while (px_is_truthy(px_lt(_v308, px_call(px_get_global("len"), (LXValue[]){_v305}, 1)))) {
        px_srcline(133);
        _v309 = px_index(_v305, _v308);
        px_srcline(134);
        _v310 = px_index(_v309, px_int(0LL));
        px_srcline(135);
        _v311 = px_str("None");
        px_srcline(136);
        if (px_is_truthy(px_ne(_v310, px_null()))) {
            px_srcline(137);
             _v311 = px_add(px_add(px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(12LL))}, 1)), _v310), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1)), px_str(")"));
        }
        px_srcline(138);
        (void)(px_method(_v307, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(4LL))}, 1), px_str("(\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1)), _v311), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1)), px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v309, px_int(1LL)), px_add(_v306, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1)), px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v309, px_int(2LL)), px_add(_v306, px_int(8LL))}, 2)), px_str(",\n")), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(4LL))}, 1)), px_str("),"))}, 1));
        px_srcline(139);
         _v308 = px_add(_v308, px_int(1LL));
    }
    px_srcline(140);
    return px_add(px_add(px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v307}, 2)), px_str("\n")), px_call(px_get_global("pad"), (LXValue[]){_v306}, 1)), px_str("]"));
px_err_312:
    if (px_err_312_proped) return px_err_312_val;
    return px_null();
}

static LXValue fn_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fmt_float");
    LXValue _v313 = (nargs > 0) ? args[0] : px_null();
    LXValue _v314 = px_null();
    LXValue px_err_315_val = px_null();
    int px_err_315_proped = 0;
    px_srcline(142);
    _v314 = px_call(px_get_global("str"), (LXValue[]){_v313}, 1);
    px_srcline(143);
    if (px_is_truthy(({ LXValue _t316 = px_eq(_v314, px_str("inf")); px_is_truthy(_t316) ? _t316 : px_eq(_v314, px_str("-inf")); }))) {
        px_srcline(144);
        return _v314;
    }
    px_srcline(145);
    if (px_is_truthy(({ LXValue _t318 = ({ LXValue _t317 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v314, px_str(".")}, 2)); px_is_truthy(_t317) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v314, px_str("e")}, 2)) : _t317; }); px_is_truthy(_t318) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v314, px_str("E")}, 2)) : _t318; }))) {
        px_srcline(146);
        return px_add(_v314, px_str(".0"));
    }
    px_srcline(147);
    return _v314;
px_err_315:
    if (px_err_315_proped) return px_err_315_val;
    return px_null();
}

static LXValue fn_dump_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_field");
    LXValue _v319 = (nargs > 0) ? args[0] : px_null();
    LXValue _v320 = (nargs > 1) ? args[1] : px_null();
    LXValue _v321 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_322_val = px_null();
    int px_err_322_proped = 0;
    px_srcline(149);
    if (px_is_truthy(px_eq(_v320, px_str("s")))) {
        px_srcline(150);
        return _v319;
    }
    px_srcline(151);
    if (px_is_truthy(px_eq(_v320, px_str("r")))) {
        px_srcline(152);
        return px_call(px_get_global("str"), (LXValue[]){_v319}, 1);
    }
    px_srcline(153);
    if (px_is_truthy(px_eq(_v320, px_str("f")))) {
        px_srcline(154);
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v319}, 1);
    }
    px_srcline(155);
    if (px_is_truthy(px_eq(_v320, px_str("n")))) {
        px_srcline(156);
        return px_call(px_get_global("dump_node"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(157);
    if (px_is_truthy(px_eq(_v320, px_str("o")))) {
        px_srcline(158);
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(159);
    if (px_is_truthy(px_eq(_v320, px_str("os")))) {
        px_srcline(160);
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(161);
    if (px_is_truthy(px_eq(_v320, px_str("ol")))) {
        px_srcline(162);
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(163);
    if (px_is_truthy(px_eq(_v320, px_str("l")))) {
        px_srcline(164);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(165);
    if (px_is_truthy(px_eq(_v320, px_str("ls")))) {
        px_srcline(166);
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v320, px_str("tl")))) {
        px_srcline(168);
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v320, px_str("lpl")))) {
        px_srcline(170);
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v320, px_str("lp")))) {
        px_srcline(172);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(_v320, px_str("lsf")))) {
        px_srcline(174);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(175);
    if (px_is_truthy(px_eq(_v320, px_str("lev")))) {
        px_srcline(176);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(177);
    if (px_is_truthy(px_eq(_v320, px_str("lfd")))) {
        px_srcline(178);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(179);
    if (px_is_truthy(px_eq(_v320, px_str("ltci")))) {
        px_srcline(181);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(182);
    if (px_is_truthy(px_eq(_v320, px_str("lc")))) {
        px_srcline(183);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(184);
    if (px_is_truthy(px_eq(_v320, px_str("lma")))) {
        px_srcline(185);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(186);
    if (px_is_truthy(px_eq(_v320, px_str("lt2")))) {
        px_srcline(187);
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(188);
    if (px_is_truthy(px_eq(_v320, px_str("lt2b")))) {
        px_srcline(189);
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(190);
    if (px_is_truthy(px_eq(_v320, px_str("lt3")))) {
        px_srcline(191);
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(192);
    if (px_is_truthy(px_eq(_v320, px_str("p")))) {
        px_srcline(193);
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(194);
    return px_call(px_get_global("str"), (LXValue[]){_v319}, 1);
px_err_322:
    if (px_err_322_proped) return px_err_322_val;
    return px_null();
}

static LXValue fn_dump_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("dump_program");
    LXValue _v323 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_324_val = px_null();
    int px_err_324_proped = 0;
    px_srcline(197);
    return px_call(px_get_global("dump_node"), (LXValue[]){_v323, px_int(0LL)}, 2);
px_err_324:
    if (px_err_324_proped) return px_err_324_val;
    return px_null();
}

static LXValue fn_pk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pk");
    LXValue px_err_325_val = px_null();
    int px_err_325_proped = 0;
    px_srcline(107);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
px_err_325:
    if (px_err_325_proped) return px_err_325_val;
    return px_null();
}

static LXValue fn_pk_display(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pk_display");
    LXValue _v326 = px_null();
    LXValue _v327 = px_null();
    LXValue px_err_328_val = px_null();
    int px_err_328_proped = 0;
    px_srcline(110);
    _v326 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(0LL));
    px_srcline(111);
    _v327 = px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
    px_srcline(112);
    if (px_is_truthy(px_eq(_v326, px_str("整数")))) {
        px_srcline(113);
        return px_add(px_str("整数 "), _v327);
    }
    px_srcline(114);
    if (px_is_truthy(px_eq(_v326, px_str("浮点")))) {
        px_srcline(115);
        return px_add(px_str("浮点 "), _v327);
    }
    px_srcline(116);
    if (px_is_truthy(px_eq(_v326, px_str("字符串")))) {
        px_srcline(117);
        return px_add(px_str("字符串 "), px_call(px_get_global("rust_str_debug"), (LXValue[]){_v327}, 1));
    }
    px_srcline(118);
    if (px_is_truthy(px_eq(_v326, px_str("标识符")))) {
        px_srcline(119);
        return px_add(px_str("标识符 "), _v327);
    }
    px_srcline(120);
    if (px_is_truthy(px_eq(_v326, px_str("注释")))) {
        px_srcline(121);
        return px_add(px_str("注释 "), _v327);
    }
    px_srcline(122);
    return _v326;
px_err_328:
    if (px_err_328_proped) return px_err_328_val;
    return px_null();
}

static LXValue fn_pv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pv");
    LXValue px_err_329_val = px_null();
    int px_err_329_proped = 0;
    px_srcline(124);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(1LL));
px_err_329:
    if (px_err_329_proped) return px_err_329_val;
    return px_null();
}

static LXValue fn_pline(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pline");
    LXValue px_err_330_val = px_null();
    int px_err_330_proped = 0;
    px_srcline(126);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL));
px_err_330:
    if (px_err_330_proped) return px_err_330_val;
    return px_null();
}

static LXValue fn_pcol(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("pcol");
    LXValue px_err_331_val = px_null();
    int px_err_331_proped = 0;
    px_srcline(128);
    return px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL));
px_err_331:
    if (px_err_331_proped) return px_err_331_val;
    return px_null();
}

static LXValue fn_ppos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("ppos");
    LXValue px_err_332_val = px_null();
    int px_err_332_proped = 0;
    px_srcline(130);
    return px_list_n((LXValue[]){px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(2LL)), px_index(px_index(px_get_global("p_toks"), px_get_global("p_pos")), px_int(3LL))}, 2);
px_err_332:
    if (px_err_332_proped) return px_err_332_val;
    return px_null();
}

static LXValue fn_adv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("adv");
    LXValue _v333 = px_null();
    LXValue px_err_334_val = px_null();
    int px_err_334_proped = 0;
    px_srcline(132);
    _v333 = px_index(px_get_global("p_toks"), px_get_global("p_pos"));
    px_srcline(133);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(134);
        px_set_global("p_pos", px_add(px_get_global("p_pos"), px_int(1LL)));
    }
    px_srcline(136);
    if (px_is_truthy(({ LXValue _t336 = ({ LXValue _t335 = px_eq(px_index(_v333, px_int(0LL)), px_str("(")); px_is_truthy(_t335) ? _t335 : px_eq(px_index(_v333, px_int(0LL)), px_str("[")); }); px_is_truthy(_t336) ? _t336 : px_eq(px_index(_v333, px_int(0LL)), px_str("{")); }))) {
        px_srcline(137);
        px_set_global("p_brack", px_add(px_get_global("p_brack"), px_int(1LL)));
    }
    else if (px_is_truthy(({ LXValue _t338 = ({ LXValue _t337 = px_eq(px_index(_v333, px_int(0LL)), px_str(")")); px_is_truthy(_t337) ? _t337 : px_eq(px_index(_v333, px_int(0LL)), px_str("]")); }); px_is_truthy(_t338) ? _t338 : px_eq(px_index(_v333, px_int(0LL)), px_str("}")); }))) {
        px_srcline(139);
        if (px_is_truthy(px_gt(px_get_global("p_brack"), px_int(0LL)))) {
            px_srcline(140);
            px_set_global("p_brack", px_sub(px_get_global("p_brack"), px_int(1LL)));
        }
    }
    px_srcline(141);
    return _v333;
px_err_334:
    if (px_err_334_proped) return px_err_334_val;
    return px_null();
}

static LXValue fn_chk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk");
    LXValue _v339 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_340_val = px_null();
    int px_err_340_proped = 0;
    px_srcline(143);
    return px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), _v339);
px_err_340:
    if (px_err_340_proped) return px_err_340_val;
    return px_null();
}

static LXValue fn_chk2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk2");
    LXValue _v341 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_342_val = px_null();
    int px_err_342_proped = 0;
    px_srcline(145);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(146);
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(1LL))), px_int(0LL)), _v341);
    }
    px_srcline(147);
    return px_bool(false);
px_err_342:
    if (px_err_342_proped) return px_err_342_val;
    return px_null();
}

static LXValue fn_chk3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk3");
    LXValue _v343 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_344_val = px_null();
    int px_err_344_proped = 0;
    px_srcline(150);
    if (px_is_truthy(px_lt(px_add(px_get_global("p_pos"), px_int(2LL)), px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(151);
        return px_eq(px_index(px_index(px_get_global("p_toks"), px_add(px_get_global("p_pos"), px_int(2LL))), px_int(0LL)), _v343);
    }
    px_srcline(152);
    return px_bool(false);
px_err_344:
    if (px_err_344_proped) return px_err_344_val;
    return px_null();
}

static LXValue fn_expect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect");
    LXValue _v345 = (nargs > 0) ? args[0] : px_null();
    LXValue _v346 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_347_val = px_null();
    int px_err_347_proped = 0;
    px_srcline(154);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v345}, 1))) {
        px_srcline(155);
        return px_call(px_get_global("adv"), (LXValue[]){}, 0);
    }
    px_srcline(156);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望 "), _v346), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_347:
    if (px_err_347_proped) return px_err_347_val;
    return px_null();
}

static LXValue fn_expect_ident(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect_ident");
    LXValue _v348 = (nargs > 0) ? args[0] : px_null();
    LXValue _v349 = px_null();
    LXValue px_err_350_val = px_null();
    int px_err_350_proped = 0;
    px_srcline(158);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(159);
        _v349 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(160);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(161);
        return _v349;
    }
    px_srcline(162);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v348), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_350:
    if (px_err_350_proped) return px_err_350_val;
    return px_null();
}

static LXValue fn_is_name_kind(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_name_kind");
    LXValue _v351 = (nargs > 0) ? args[0] : px_null();
    LXValue _v352 = px_null();
    LXValue _v353 = px_null();
    LXValue px_err_354_val = px_null();
    int px_err_354_proped = 0;
    px_srcline(164);
    _v352 = px_list_n((LXValue[]){px_str("let"), px_str("var"), px_str("const"), px_str("def"), px_str("fn"), px_str("struct"), px_str("enum"), px_str("trait"), px_str("impl"), px_str("match"), px_str("case"), px_str("if"), px_str("elif"), px_str("else"), px_str("for"), px_str("while"), px_str("in"), px_str("return"), px_str("break"), px_str("continue"), px_str("import"), px_str("from"), px_str("pub"), px_str("as"), px_str("spawn"), px_str("chan"), px_str("send"), px_str("recv"), px_str("select"), px_str("true"), px_str("false"), px_str("null"), px_str("self"), px_str("type"), px_str("capture"), px_str("extern")}, 36);
    px_srcline(165);
    _v353 = px_int(0LL);
    px_srcline(166);
    while (px_is_truthy(px_lt(_v353, px_call(px_get_global("len"), (LXValue[]){_v352}, 1)))) {
        px_srcline(167);
        if (px_is_truthy(px_eq(px_index(_v352, _v353), _v351))) {
            px_srcline(168);
            return px_bool(true);
        }
        px_srcline(169);
         _v353 = px_add(_v353, px_int(1LL));
    }
    px_srcline(170);
    return px_bool(false);
px_err_354:
    if (px_err_354_proped) return px_err_354_val;
    return px_null();
}

static LXValue fn_expect_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("expect_name");
    LXValue _v355 = (nargs > 0) ? args[0] : px_null();
    LXValue _v356 = px_null();
    LXValue px_err_357_val = px_null();
    int px_err_357_proped = 0;
    px_srcline(172);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(173);
        _v356 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(174);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(175);
        return _v356;
    }
    px_srcline(176);
    if (px_is_truthy(px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1))) {
        px_srcline(177);
        _v356 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(178);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(179);
        return _v356;
    }
    px_srcline(180);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v355), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_357:
    if (px_err_357_proped) return px_err_357_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("perr");
    LXValue _v358 = (nargs > 0) ? args[0] : px_null();
    LXValue _v359 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_360_val = px_null();
    int px_err_360_proped = 0;
    px_srcline(184);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1)), px_str(": 语法错误 ")), _v358), px_str(": ")), _v359)}, 1));
    px_srcline(185);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_360:
    if (px_err_360_proped) return px_err_360_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines");
    LXValue px_err_361_val = px_null();
    int px_err_361_proped = 0;
    px_srcline(187);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(188);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_361:
    if (px_err_361_proped) return px_err_361_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_brace_indents");
    LXValue px_err_362_val = px_null();
    int px_err_362_proped = 0;
    px_srcline(190);
    while (px_is_truthy(({ LXValue _t363 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t363) ? _t363 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(191);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_362:
    if (px_err_362_proped) return px_err_362_val;
    return px_null();
}

static LXValue fn_skip_expr_ws(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_expr_ws");
    LXValue px_err_364_val = px_null();
    int px_err_364_proped = 0;
    px_srcline(193);
    while (px_is_truthy(({ LXValue _t366 = ({ LXValue _t365 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t365) ? _t365 : px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); }); px_is_truthy(_t366) ? _t366 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(194);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_364:
    if (px_err_364_proped) return px_err_364_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines_in_block");
    LXValue px_err_367_val = px_null();
    int px_err_367_proped = 0;
    px_srcline(196);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(197);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_367:
    if (px_err_367_proped) return px_err_367_val;
    return px_null();
}

static LXValue fn_chk_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk_op");
    LXValue _v368 = (nargs > 0) ? args[0] : px_null();
    LXValue _v369 = px_null();
    LXValue px_err_370_val = px_null();
    int px_err_370_proped = 0;
    px_srcline(209);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v368}, 1))) {
        px_srcline(210);
        return px_bool(true);
    }
    px_srcline(211);
    if (px_is_truthy(px_le(px_get_global("p_brack"), px_int(0LL)))) {
        px_srcline(212);
        return px_bool(false);
    }
    px_srcline(213);
    if (px_is_truthy(({ LXValue _t372 = ({ LXValue _t371 = px_eq(_v368, px_str("-")); px_is_truthy(_t371) ? _t371 : px_eq(_v368, px_str("~")); }); px_is_truthy(_t372) ? _t372 : px_eq(_v368, px_str("not")); }))) {
        px_srcline(214);
        return px_bool(false);
    }
    px_srcline(215);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)))) {
        px_srcline(216);
        return px_bool(false);
    }
    px_srcline(217);
    _v369 = px_get_global("p_pos");
    px_srcline(218);
    while (px_is_truthy(({ LXValue _t373 = px_lt(_v369, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)); px_is_truthy(_t373) ? px_eq(px_index(px_index(px_get_global("p_toks"), _v369), px_int(0LL)), px_str("换行")) : _t373; }))) {
        px_srcline(219);
         _v369 = px_add(_v369, px_int(1LL));
    }
    px_srcline(220);
    if (px_is_truthy(({ LXValue _t374 = px_lt(_v369, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)); px_is_truthy(_t374) ? px_eq(px_index(px_index(px_get_global("p_toks"), _v369), px_int(0LL)), _v368) : _t374; }))) {
        px_srcline(221);
        px_set_global("p_pos", _v369);
        px_srcline(222);
        return px_bool(true);
    }
    px_srcline(223);
    return px_bool(false);
px_err_370:
    if (px_err_370_proped) return px_err_370_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("node_pos");
    LXValue _v375 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_376_val = px_null();
    int px_err_376_proped = 0;
    px_srcline(225);
    return px_index(_v375, px_sub(px_call(px_get_global("len"), (LXValue[]){_v375}, 1), px_int(1LL)));
px_err_376:
    if (px_err_376_proped) return px_err_376_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("qstr");
    LXValue _v377 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_378_val = px_null();
    int px_err_378_proped = 0;
    px_srcline(227);
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v377}, 1);
px_err_378:
    if (px_err_378_proped) return px_err_378_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_program");
    LXValue _v379 = px_null();
    LXValue px_err_380_val = px_null();
    int px_err_380_proped = 0;
    px_srcline(230);
    _v379 = px_list_n((LXValue[]){}, 0);
    px_srcline(231);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(232);
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        px_srcline(233);
        (void)(px_method(_v379, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(234);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    px_srcline(235);
    return px_list_n((LXValue[]){px_str("Program"), _v379}, 2);
px_err_380:
    if (px_err_380_proped) return px_err_380_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_stmt");
    LXValue _v381 = px_null();
    LXValue _v382 = px_null();
    LXValue _v383 = px_null();
    LXValue _v384 = px_null();
    LXValue px_err_385_val = px_null();
    int px_err_385_proped = 0;
    px_srcline(238);
    _v381 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(239);
    if (px_is_truthy(px_eq(_v381, px_str("let")))) {
        px_srcline(240);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(_v381, px_str("var")))) {
        px_srcline(242);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    px_srcline(243);
    if (px_is_truthy(px_eq(_v381, px_str("const")))) {
        px_srcline(244);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    px_srcline(245);
    if (px_is_truthy(px_eq(_v381, px_str("if")))) {
        px_srcline(246);
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    px_srcline(247);
    if (px_is_truthy(px_eq(_v381, px_str("for")))) {
        px_srcline(248);
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    px_srcline(249);
    if (px_is_truthy(px_eq(_v381, px_str("while")))) {
        px_srcline(250);
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    px_srcline(251);
    if (px_is_truthy(px_eq(_v381, px_str("def")))) {
        px_srcline(252);
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    px_srcline(253);
    if (px_is_truthy(px_eq(_v381, px_str("extern")))) {
        px_srcline(254);
        return px_call(px_get_global("parse_extern_def"), (LXValue[]){}, 0);
    }
    px_srcline(255);
    if (px_is_truthy(px_eq(_v381, px_str("struct")))) {
        px_srcline(256);
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    px_srcline(257);
    if (px_is_truthy(px_eq(_v381, px_str("enum")))) {
        px_srcline(258);
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    px_srcline(259);
    if (px_is_truthy(({ LXValue _t386 = px_eq(_v381, px_str("标识符")); px_is_truthy(_t386) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("type")) : _t386; }))) {
        px_srcline(262);
        if (px_is_truthy(({ LXValue _t387 = px_call(px_get_global("chk2"), (LXValue[]){px_str("标识符")}, 1); px_is_truthy(_t387) ? px_call(px_get_global("chk3"), (LXValue[]){px_str("const")}, 1) : _t387; }))) {
            px_srcline(263);
            return px_call(px_get_global("parse_type_const"), (LXValue[]){}, 0);
        }
        px_srcline(264);
        return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
    }
    px_srcline(265);
    if (px_is_truthy(px_eq(_v381, px_str("trait")))) {
        px_srcline(266);
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    px_srcline(267);
    if (px_is_truthy(px_eq(_v381, px_str("impl")))) {
        px_srcline(268);
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    px_srcline(269);
    if (px_is_truthy(px_eq(_v381, px_str("import")))) {
        px_srcline(270);
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    px_srcline(271);
    if (px_is_truthy(px_eq(_v381, px_str("from")))) {
        px_srcline(272);
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    px_srcline(273);
    if (px_is_truthy(px_eq(_v381, px_str("return")))) {
        px_srcline(274);
        _v382 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(275);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(276);
        if (px_is_truthy(({ LXValue _t389 = ({ LXValue _t388 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t388) ? _t388 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t389) ? _t389 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(277);
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v382}, 3);
        }
        px_srcline(278);
        _v383 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(279);
        return px_list_n((LXValue[]){px_str("Return"), _v383, _v382}, 3);
    }
    px_srcline(280);
    if (px_is_truthy(px_eq(_v381, px_str("break")))) {
        px_srcline(281);
        _v382 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(282);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(283);
        return px_list_n((LXValue[]){px_str("Break"), _v382}, 2);
    }
    px_srcline(284);
    if (px_is_truthy(px_eq(_v381, px_str("continue")))) {
        px_srcline(285);
        _v382 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(286);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(287);
        return px_list_n((LXValue[]){px_str("Continue"), _v382}, 2);
    }
    px_srcline(288);
    if (px_is_truthy(px_eq(_v381, px_str("spawn")))) {
        px_srcline(289);
        _v382 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(290);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(291);
        _v384 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(292);
        return px_list_n((LXValue[]){px_str("Spawn"), _v384, _v382}, 3);
    }
    px_srcline(293);
    if (px_is_truthy(px_eq(_v381, px_str("select")))) {
        px_srcline(294);
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    px_srcline(295);
    if (px_is_truthy(px_eq(_v381, px_str("fn")))) {
        px_srcline(296);
        _v384 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(297);
        _v382 = px_call(px_get_global("node_pos"), (LXValue[]){_v384}, 1);
        px_srcline(298);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v384, _v382}, 3);
    }
    px_srcline(299);
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_385:
    if (px_err_385_proped) return px_err_385_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_var_decl");
    LXValue _v390 = (nargs > 0) ? args[0] : px_null();
    LXValue _v391 = px_null();
    LXValue _v392 = px_null();
    LXValue _v393 = px_null();
    LXValue _v394 = px_null();
    LXValue px_err_395_val = px_null();
    int px_err_395_proped = 0;
    px_srcline(301);
    _v391 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(302);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(304);
    if (px_is_truthy(({ LXValue _t397 = ({ LXValue _t396 = px_eq(_v390, px_str("Let")); px_is_truthy(_t396) ? px_call(px_get_global("chk"), (LXValue[]){px_str("标识符")}, 1) : _t396; }); px_is_truthy(_t397) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("mut")) : _t397; }))) {
        px_srcline(305);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(306);
         _v390 = px_str("Mut");
    }
    px_srcline(307);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        px_srcline(308);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    px_srcline(309);
    _v392 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    px_srcline(310);
    _v393 = px_null();
    px_srcline(311);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(312);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(313);
         _v393 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(314);
    _v394 = px_null();
    px_srcline(315);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        px_srcline(316);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(317);
         _v394 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    px_srcline(318);
    return px_list_n((LXValue[]){px_str("VarDecl"), _v390, px_call(px_get_global("qstr"), (LXValue[]){_v392}, 1), _v393, _v394, _v391}, 6);
px_err_395:
    if (px_err_395_proped) return px_err_395_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_assign_or_expr");
    LXValue _v398 = px_null();
    LXValue _v399 = px_null();
    LXValue _v400 = px_null();
    LXValue _v401 = px_null();
    LXValue _v402 = px_null();
    LXValue _v403 = px_null();
    LXValue _v404 = px_null();
    LXValue px_err_405_val = px_null();
    int px_err_405_proped = 0;
    px_srcline(322);
    _v398 = px_null();
    px_srcline(323);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(324);
         _v398 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
    }
    px_srcline(325);
    _v399 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(326);
    _v400 = px_call(px_get_global("node_pos"), (LXValue[]){_v399}, 1);
    px_srcline(327);
    _v401 = px_null();
    px_srcline(328);
    _v402 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(329);
    if (px_is_truthy(px_eq(_v402, px_str("=")))) {
        px_srcline(330);
         _v401 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("<-")))) {
        px_srcline(333);
         _v401 = px_str("Append");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("+=")))) {
        px_srcline(335);
         _v401 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("-=")))) {
        px_srcline(337);
         _v401 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("*=")))) {
        px_srcline(339);
         _v401 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("/=")))) {
        px_srcline(341);
         _v401 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("//=")))) {
        px_srcline(343);
         _v401 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("%=")))) {
        px_srcline(345);
         _v401 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("**=")))) {
        px_srcline(347);
         _v401 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("&=")))) {
        px_srcline(349);
         _v401 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("|=")))) {
        px_srcline(351);
         _v401 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("^=")))) {
        px_srcline(353);
         _v401 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v402, px_str("<<=")))) {
        px_srcline(355);
         _v401 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v402, px_str(">>=")))) {
        px_srcline(357);
         _v401 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v402, px_str(">>>=")))) {
        px_srcline(359);
         _v401 = px_str("ShrU");
    }
    px_srcline(360);
    if (px_is_truthy(px_ne(_v401, px_null()))) {
        px_srcline(361);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(362);
        _v403 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(363);
        return px_list_n((LXValue[]){px_str("Assign"), _v399, _v401, _v403, _v400}, 5);
    }
    px_srcline(376);
    if (px_is_truthy(px_eq(px_index(_v399, px_int(0LL)), px_str("Var")))) {
        px_srcline(377);
        _v404 = px_null();
        px_srcline(378);
        if (px_is_truthy(px_ne(_v398, px_null()))) {
            px_srcline(379);
             _v404 = _v398;
        }
        else {
            px_srcline(381);
             _v404 = px_index(_v399, px_int(1LL));
        }
        px_srcline(382);
        if (px_is_truthy(px_eq(_v404, px_str("pass")))) {
            px_srcline(383);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2012"), px_str("`pass` 不是 PuXian 关键字（它是普通标识符，`var pass = 0` 合法）：空语句请写 `0`，或直接删掉这一行")}, 2));
        }
        px_srcline(384);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2012"), px_add(px_add(px_str("裸标识符语句没有效果："), _v404), px_str("（调用需要括号 `()`；若想要空语句请写 `0`）"))}, 2));
    }
    px_srcline(385);
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v399, _v400}, 3);
px_err_405:
    if (px_err_405_proped) return px_err_405_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if");
    LXValue _v406 = px_null();
    LXValue _v407 = px_null();
    LXValue _v408 = px_null();
    LXValue _v409 = px_null();
    LXValue _v410 = px_null();
    LXValue _v411 = px_null();
    LXValue _v412 = px_null();
    LXValue _v413 = px_null();
    LXValue px_err_414_val = px_null();
    int px_err_414_proped = 0;
    px_srcline(387);
    _v406 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(388);
    _v407 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(389);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(390);
    _v408 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(391);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(392);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(393);
    _v409 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v407}, 1);
    px_srcline(394);
    _v410 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v408, _v409}, 2)}, 1);
    px_srcline(395);
    _v411 = px_null();
    px_srcline(396);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(397);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            px_srcline(398);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(399);
            _v412 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(400);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(401);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(402);
            _v413 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v407}, 1);
            px_srcline(403);
            (void)(px_method(_v410, "append", (LXValue[]){px_list_n((LXValue[]){_v412, _v413}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            px_srcline(405);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(406);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(407);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(408);
             _v411 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v407}, 1);
            px_srcline(409);
            break;
        }
        else {
            px_srcline(411);
            break;
        }
    }
    px_srcline(412);
    return px_list_n((LXValue[]){px_str("If"), _v410, _v411, _v406}, 4);
px_err_414:
    if (px_err_414_proped) return px_err_414_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_for");
    LXValue _v415 = px_null();
    LXValue _v416 = px_null();
    LXValue _v417 = px_null();
    LXValue _v418 = px_null();
    LXValue _v419 = px_null();
    LXValue px_err_420_val = px_null();
    int px_err_420_proped = 0;
    px_srcline(414);
    _v415 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(415);
    _v416 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(416);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(417);
    _v417 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    px_srcline(418);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    px_srcline(419);
    _v418 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(420);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(421);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(422);
    _v419 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v416}, 1);
    px_srcline(423);
    return px_list_n((LXValue[]){px_str("For"), px_call(px_get_global("qstr"), (LXValue[]){_v417}, 1), _v418, _v419, _v415}, 5);
px_err_420:
    if (px_err_420_proped) return px_err_420_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_while");
    LXValue _v421 = px_null();
    LXValue _v422 = px_null();
    LXValue _v423 = px_null();
    LXValue _v424 = px_null();
    LXValue px_err_425_val = px_null();
    int px_err_425_proped = 0;
    px_srcline(425);
    _v421 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(426);
    _v422 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(427);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(428);
    _v423 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(429);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(430);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(431);
    _v424 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v422}, 1);
    px_srcline(432);
    return px_list_n((LXValue[]){px_str("While"), _v423, _v424, _v421}, 4);
px_err_425:
    if (px_err_425_proped) return px_err_425_val;
    return px_null();
}

static LXValue fn_parse_block_ctxt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block_ctxt");
    LXValue _v426 = (nargs > 0) ? args[0] : px_null();
    LXValue _v427 = px_null();
    LXValue _v428 = px_null();
    LXValue px_err_429_val = px_null();
    int px_err_429_proped = 0;
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
    _v427 = px_list_n((LXValue[]){}, 0);
    px_srcline(445);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(446);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(447);
        _v428 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(448);
        if (px_is_truthy(({ LXValue _t434 = ({ LXValue _t433 = ({ LXValue _t432 = ({ LXValue _t431 = ({ LXValue _t430 = px_eq(_v428, px_str("去缩进")); px_is_truthy(_t430) ? _t430 : px_eq(_v428, px_str("EOF")); }); px_is_truthy(_t431) ? _t431 : px_eq(_v428, px_str(")")); }); px_is_truthy(_t432) ? _t432 : px_eq(_v428, px_str("]")); }); px_is_truthy(_t433) ? _t433 : px_eq(_v428, px_str("}")); }); px_is_truthy(_t434) ? _t434 : px_eq(_v428, px_str(",")); }))) {
            px_srcline(449);
            break;
        }
        px_srcline(450);
        if (px_is_truthy(px_le(px_call(px_get_global("pcol"), (LXValue[]){}, 0), _v426))) {
            px_srcline(451);
            break;
        }
        px_srcline(452);
        (void)(px_method(_v427, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(453);
    return _v427;
px_err_429:
    if (px_err_429_proped) return px_err_429_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block");
    LXValue _v435 = px_null();
    LXValue px_err_436_val = px_null();
    int px_err_436_proped = 0;
    px_srcline(455);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(456);
    _v435 = px_list_n((LXValue[]){}, 0);
    px_srcline(457);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(458);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(459);
        if (px_is_truthy(({ LXValue _t437 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t437) ? _t437 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(460);
            break;
        }
        px_srcline(461);
        (void)(px_method(_v435, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(462);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(463);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    px_srcline(464);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(465);
    return _v435;
px_err_436:
    if (px_err_436_proped) return px_err_436_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_params");
    LXValue _v438 = px_null();
    LXValue _v439 = px_null();
    LXValue _v440 = px_null();
    LXValue _v441 = px_null();
    LXValue px_err_442_val = px_null();
    int px_err_442_proped = 0;
    px_srcline(469);
    _v438 = px_list_n((LXValue[]){}, 0);
    px_srcline(470);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        px_srcline(471);
        return _v438;
    }
    px_srcline(472);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(473);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(474);
        _v439 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        px_srcline(475);
        _v440 = _v439;
        px_srcline(476);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            px_srcline(477);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(478);
            _v441 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
            px_srcline(479);
             _v440 = px_add(px_add(_v439, px_str(": ")), _v441);
        }
        px_srcline(480);
        (void)(px_method(_v438, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v440}, 1)}, 1));
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
    return _v438;
px_err_442:
    if (px_err_442_proped) return px_err_442_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_func_def");
    LXValue _v443 = px_null();
    LXValue _v444 = px_null();
    LXValue _v445 = px_null();
    LXValue _v446 = px_null();
    LXValue _v447 = px_null();
    LXValue _v448 = px_null();
    LXValue px_err_449_val = px_null();
    int px_err_449_proped = 0;
    px_srcline(488);
    _v443 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(489);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(490);
    _v444 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(491);
    _v445 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(492);
    _v446 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(493);
    _v447 = px_null();
    px_srcline(494);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(495);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(496);
         _v447 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(497);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(498);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(499);
    _v448 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(500);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v444}, 1), _v446, _v447, _v448, _v443, _v445}, 7);
px_err_449:
    if (px_err_449_proped) return px_err_449_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_extern_def");
    LXValue _v450 = px_null();
    LXValue _v451 = px_null();
    LXValue _v452 = px_null();
    LXValue _v453 = px_null();
    LXValue px_err_454_val = px_null();
    int px_err_454_proped = 0;
    px_srcline(504);
    _v450 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(505);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(506);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    px_srcline(507);
    _v451 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(508);
    _v452 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(509);
    _v453 = px_null();
    px_srcline(510);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(511);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(512);
         _v453 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(513);
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v451}, 1), _v452, _v453, _v450}, 5);
px_err_454:
    if (px_err_454_proped) return px_err_454_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_struct_def");
    LXValue _v455 = px_null();
    LXValue _v456 = px_null();
    LXValue _v457 = px_null();
    LXValue _v458 = px_null();
    LXValue _v459 = px_null();
    LXValue _v460 = px_null();
    LXValue _v461 = px_null();
    LXValue px_err_462_val = px_null();
    int px_err_462_proped = 0;
    px_srcline(515);
    _v455 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(516);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(517);
    _v456 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    px_srcline(518);
    _v457 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(519);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(520);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(521);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(522);
    _v458 = px_list_n((LXValue[]){}, 0);
    px_srcline(523);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(524);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(525);
        if (px_is_truthy(({ LXValue _t463 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t463) ? _t463 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(526);
            break;
        }
        px_srcline(527);
        _v459 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(528);
        _v460 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        px_srcline(529);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(530);
        _v461 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(531);
        (void)(px_method(_v458, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v460}, 1), _v461, _v459}, 4)}, 1));
        px_srcline(532);
        if (px_is_truthy(({ LXValue _t464 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t464) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t464; }))) {
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
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v456}, 1), _v458, _v455, _v457}, 5);
px_err_462:
    if (px_err_462_proped) return px_err_462_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_enum_def");
    LXValue _v465 = px_null();
    LXValue _v466 = px_null();
    LXValue _v467 = px_null();
    LXValue _v468 = px_null();
    LXValue _v469 = px_null();
    LXValue _v470 = px_null();
    LXValue px_err_471_val = px_null();
    int px_err_471_proped = 0;
    px_srcline(539);
    _v465 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(540);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(541);
    _v466 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(542);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(543);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(544);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(545);
    _v467 = px_list_n((LXValue[]){}, 0);
    px_srcline(546);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(547);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(548);
        if (px_is_truthy(({ LXValue _t472 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t472) ? _t472 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(549);
            break;
        }
        px_srcline(550);
        _v468 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(551);
        _v469 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        px_srcline(552);
        _v470 = px_list_n((LXValue[]){}, 0);
        px_srcline(553);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(554);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(555);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(556);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(557);
                    (void)(px_method(_v470, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
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
        (void)(px_method(_v467, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v469}, 1), _v470, _v468}, 4)}, 1));
        px_srcline(564);
        if (px_is_truthy(({ LXValue _t473 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t473) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t473; }))) {
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
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v466}, 1), _v467, _v465}, 4);
px_err_471:
    if (px_err_471_proped) return px_err_471_val;
    return px_null();
}

static LXValue fn_parse_type_const(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_const");
    LXValue _v474 = px_null();
    LXValue _v475 = px_null();
    LXValue _v476 = px_null();
    LXValue _v477 = px_null();
    LXValue _v478 = px_null();
    LXValue _v479 = px_null();
    LXValue px_err_480_val = px_null();
    int px_err_480_proped = 0;
    px_srcline(574);
    _v474 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(575);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(576);
    _v475 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(577);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("const"), px_str("'const'")}, 2));
    px_srcline(578);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(579);
    _v476 = px_list_n((LXValue[]){}, 0);
    px_srcline(580);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(581);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(582);
            _v477 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(583);
            _v478 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("常量名")}, 1);
            px_srcline(584);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
            px_srcline(585);
            _v479 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(586);
            (void)(px_method(_v476, "append", (LXValue[]){px_list_n((LXValue[]){px_str("TypeConstItem"), px_call(px_get_global("qstr"), (LXValue[]){_v478}, 1), _v479, _v477}, 4)}, 1));
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
    return px_list_n((LXValue[]){px_str("TypeConst"), px_call(px_get_global("qstr"), (LXValue[]){_v475}, 1), _v476, _v474}, 4);
px_err_480:
    if (px_err_480_proped) return px_err_480_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_trait_def");
    LXValue _v481 = px_null();
    LXValue _v482 = px_null();
    LXValue _v483 = px_null();
    LXValue _v484 = px_null();
    LXValue _v485 = px_null();
    LXValue _v486 = px_null();
    LXValue _v487 = px_null();
    LXValue _v488 = px_null();
    LXValue px_err_489_val = px_null();
    int px_err_489_proped = 0;
    px_srcline(594);
    _v481 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(595);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(596);
    _v482 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    px_srcline(597);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(598);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(599);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(600);
    _v483 = px_list_n((LXValue[]){}, 0);
    px_srcline(601);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(602);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(603);
        if (px_is_truthy(({ LXValue _t490 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t490) ? _t490 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(604);
            break;
        }
        px_srcline(605);
        _v484 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(606);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(607);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        px_srcline(608);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(609);
        _v485 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(610);
        _v486 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(611);
        _v487 = px_null();
        px_srcline(612);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(613);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(614);
             _v487 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(615);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(616);
        _v488 = px_list_n((LXValue[]){}, 0);
        px_srcline(617);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(618);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(619);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(620);
                 _v488 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        px_srcline(621);
        (void)(px_method(_v483, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v485}, 1), _v486, _v487, _v488, _v484, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        px_srcline(622);
        if (px_is_truthy(({ LXValue _t491 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t491) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t491; }))) {
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
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v482}, 1), _v483, _v481}, 4);
px_err_489:
    if (px_err_489_proped) return px_err_489_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_impl_def");
    LXValue _v492 = px_null();
    LXValue _v493 = px_null();
    LXValue _v494 = px_null();
    LXValue _v495 = px_null();
    LXValue _v496 = px_null();
    LXValue _v497 = px_null();
    LXValue _v498 = px_null();
    LXValue _v499 = px_null();
    LXValue _v500 = px_null();
    LXValue _v501 = px_null();
    LXValue px_err_502_val = px_null();
    int px_err_502_proped = 0;
    px_srcline(629);
    _v492 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(630);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(631);
    _v493 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    px_srcline(632);
    _v494 = px_null();
    px_srcline(633);
    _v495 = _v493;
    px_srcline(634);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(635);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(636);
         _v495 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
        px_srcline(637);
         _v494 = px_call(px_get_global("qstr"), (LXValue[]){_v493}, 1);
    }
    px_srcline(638);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(639);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(640);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(641);
    _v496 = px_list_n((LXValue[]){}, 0);
    px_srcline(642);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(643);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(644);
        if (px_is_truthy(({ LXValue _t503 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t503) ? _t503 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(645);
            break;
        }
        px_srcline(646);
        _v497 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(647);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(648);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        px_srcline(649);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(650);
        _v498 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(651);
        _v499 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(652);
        _v500 = px_null();
        px_srcline(653);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(654);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(655);
             _v500 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(656);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(657);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(658);
        _v501 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        px_srcline(659);
        (void)(px_method(_v496, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v498}, 1), _v499, _v500, _v501, _v497, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    px_srcline(660);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(661);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    px_srcline(662);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(663);
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v495}, 1), _v494, _v496, _v492}, 5);
px_err_502:
    if (px_err_502_proped) return px_err_502_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import");
    LXValue _v504 = px_null();
    LXValue _v505 = px_null();
    LXValue _v506 = px_null();
    LXValue px_err_507_val = px_null();
    int px_err_507_proped = 0;
    px_srcline(665);
    _v504 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(666);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(667);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        px_srcline(668);
        _v505 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(669);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(670);
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v505}, 1), px_list_n((LXValue[]){}, 0), _v504}, 4);
    }
    px_srcline(671);
    _v506 = px_list_n((LXValue[]){}, 0);
    px_srcline(672);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(673);
        (void)(px_method(_v506, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
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
    return px_list_n((LXValue[]){px_str("Import"), _v506, px_list_n((LXValue[]){}, 0), _v504}, 4);
px_err_507:
    if (px_err_507_proped) return px_err_507_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import_from");
    LXValue _v508 = px_null();
    LXValue _v509 = px_null();
    LXValue _v510 = px_null();
    LXValue px_err_511_val = px_null();
    int px_err_511_proped = 0;
    px_srcline(680);
    _v508 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(681);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(682);
    _v509 = px_list_n((LXValue[]){}, 0);
    px_srcline(683);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(684);
        (void)(px_method(_v509, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
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
    _v510 = px_list_n((LXValue[]){}, 0);
    px_srcline(691);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(692);
        (void)(px_method(_v510, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
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
    return px_list_n((LXValue[]){px_str("Import"), _v509, _v510, _v508}, 4);
px_err_511:
    if (px_err_511_proped) return px_err_511_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_select");
    LXValue _v512 = px_null();
    LXValue _v513 = px_null();
    LXValue _v514 = px_null();
    LXValue _v515 = px_null();
    LXValue _v516 = px_null();
    LXValue _v517 = px_null();
    LXValue _v518 = px_null();
    LXValue px_err_519_val = px_null();
    int px_err_519_proped = 0;
    px_srcline(699);
    _v512 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(700);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(701);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(702);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(703);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(704);
    _v513 = px_list_n((LXValue[]){}, 0);
    px_srcline(705);
    _v514 = px_null();
    px_srcline(706);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(707);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(708);
        if (px_is_truthy(({ LXValue _t520 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t520) ? _t520 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(709);
            break;
        }
        px_srcline(710);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(711);
        if (px_is_truthy(({ LXValue _t521 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t521) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t521; }))) {
            px_srcline(712);
            _v515 = px_get_global("p_pos");
            px_srcline(713);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(714);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(715);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(716);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                px_srcline(717);
                 _v514 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                px_srcline(718);
                continue;
            }
            else {
                px_srcline(720);
                px_set_global("p_pos", _v515);
            }
        }
        px_srcline(721);
        _v516 = px_null();
        px_srcline(722);
        if (px_is_truthy(({ LXValue _t522 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t522) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t522; }))) {
            px_srcline(723);
             _v516 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            px_srcline(724);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        px_srcline(725);
        _v517 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(726);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(727);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(728);
        _v518 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        px_srcline(729);
        (void)(px_method(_v513, "append", (LXValue[]){px_list_n((LXValue[]){_v516, _v517, _v518}, 3)}, 1));
    }
    px_srcline(730);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(731);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    px_srcline(732);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(733);
    return px_list_n((LXValue[]){px_str("Select"), _v513, _v514, _v512}, 4);
px_err_519:
    if (px_err_519_proped) return px_err_519_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_case_body");
    LXValue _v523 = px_null();
    LXValue _v524 = px_null();
    LXValue px_err_525_val = px_null();
    int px_err_525_proped = 0;
    px_srcline(735);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(736);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(737);
    _v523 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(738);
    _v524 = px_call(px_get_global("node_pos"), (LXValue[]){_v523}, 1);
    px_srcline(739);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v523, _v524}, 3)}, 1);
px_err_525:
    if (px_err_525_proped) return px_err_525_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_params");
    LXValue _v526 = px_null();
    LXValue _v527 = px_null();
    LXValue _v528 = px_null();
    LXValue _v529 = px_null();
    LXValue _v530 = px_null();
    LXValue px_err_531_val = px_null();
    int px_err_531_proped = 0;
    px_srcline(742);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(743);
    _v526 = px_list_n((LXValue[]){}, 0);
    px_srcline(744);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(745);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(746);
            _v527 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(747);
            _v528 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            px_srcline(748);
            _v529 = px_null();
            px_srcline(749);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(750);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(751);
                 _v529 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            px_srcline(752);
            _v530 = px_null();
            px_srcline(753);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                px_srcline(754);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(755);
                 _v530 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            px_srcline(756);
            (void)(px_method(_v526, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v528}, 1), _v529, _v530, _v527}, 5)}, 1));
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
    return _v526;
px_err_531:
    if (px_err_531_proped) return px_err_531_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_expr");
    LXValue px_err_532_val = px_null();
    int px_err_532_proped = 0;
    px_srcline(765);
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_532:
    if (px_err_532_proped) return px_err_532_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pipe");
    LXValue _v533 = px_null();
    LXValue _v534 = px_null();
    LXValue _v535 = px_null();
    LXValue px_err_536_val = px_null();
    int px_err_536_proped = 0;
    px_srcline(767);
    _v533 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    px_srcline(768);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|>")}, 1))) {
        px_srcline(769);
        _v534 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(770);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(771);
        _v535 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
        px_srcline(772);
         _v533 = px_list_n((LXValue[]){px_str("Pipe"), _v533, _v535, _v534}, 4);
    }
    px_srcline(773);
    return _v533;
px_err_536:
    if (px_err_536_proped) return px_err_536_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_null_coalesce");
    LXValue _v537 = px_null();
    LXValue _v538 = px_null();
    LXValue _v539 = px_null();
    LXValue px_err_540_val = px_null();
    int px_err_540_proped = 0;
    px_srcline(775);
    _v537 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    px_srcline(776);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("??")}, 1))) {
        px_srcline(777);
        _v538 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(778);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(779);
        _v539 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
        px_srcline(780);
         _v537 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v537, _v539, _v538}, 4);
    }
    px_srcline(781);
    return _v537;
px_err_540:
    if (px_err_540_proped) return px_err_540_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_or");
    LXValue _v541 = px_null();
    LXValue _v542 = px_null();
    LXValue _v543 = px_null();
    LXValue px_err_544_val = px_null();
    int px_err_544_proped = 0;
    px_srcline(783);
    _v541 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    px_srcline(784);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("or")}, 1))) {
        px_srcline(785);
        _v542 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(786);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(787);
        _v543 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
        px_srcline(788);
         _v541 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v541, _v543, _v542}, 5);
    }
    px_srcline(789);
    return _v541;
px_err_544:
    if (px_err_544_proped) return px_err_544_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_and");
    LXValue _v545 = px_null();
    LXValue _v546 = px_null();
    LXValue _v547 = px_null();
    LXValue px_err_548_val = px_null();
    int px_err_548_proped = 0;
    px_srcline(791);
    _v545 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    px_srcline(792);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("and")}, 1))) {
        px_srcline(793);
        _v546 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(794);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(795);
        _v547 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
        px_srcline(796);
         _v545 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v545, _v547, _v546}, 5);
    }
    px_srcline(797);
    return _v545;
px_err_548:
    if (px_err_548_proped) return px_err_548_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comparison");
    LXValue _v549 = px_null();
    LXValue _v550 = px_null();
    LXValue _v551 = px_null();
    LXValue _v552 = px_null();
    LXValue px_err_553_val = px_null();
    int px_err_553_proped = 0;
    px_srcline(799);
    _v549 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    px_srcline(800);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(801);
        _v550 = px_null();
        px_srcline(802);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("==")}, 1))) {
            px_srcline(803);
             _v550 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("!=")}, 1))) {
            px_srcline(805);
             _v550 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<")}, 1))) {
            px_srcline(807);
             _v550 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<=")}, 1))) {
            px_srcline(809);
             _v550 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">")}, 1))) {
            px_srcline(811);
             _v550 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">=")}, 1))) {
            px_srcline(813);
             _v550 = px_str("Ge");
        }
        px_srcline(814);
        if (px_is_truthy(px_eq(_v550, px_null()))) {
            px_srcline(815);
            break;
        }
        px_srcline(816);
        _v551 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(817);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(818);
        _v552 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
        px_srcline(819);
         _v549 = px_list_n((LXValue[]){px_str("Binary"), _v550, _v549, _v552, _v551}, 5);
    }
    px_srcline(820);
    return _v549;
px_err_553:
    if (px_err_553_proped) return px_err_553_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitor");
    LXValue _v554 = px_null();
    LXValue _v555 = px_null();
    LXValue _v556 = px_null();
    LXValue px_err_557_val = px_null();
    int px_err_557_proped = 0;
    px_srcline(822);
    _v554 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    px_srcline(823);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|")}, 1))) {
        px_srcline(824);
        _v555 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(825);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(826);
        _v556 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
        px_srcline(827);
         _v554 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v554, _v556, _v555}, 5);
    }
    px_srcline(828);
    return _v554;
px_err_557:
    if (px_err_557_proped) return px_err_557_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitxor");
    LXValue _v558 = px_null();
    LXValue _v559 = px_null();
    LXValue _v560 = px_null();
    LXValue px_err_561_val = px_null();
    int px_err_561_proped = 0;
    px_srcline(830);
    _v558 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    px_srcline(831);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("^")}, 1))) {
        px_srcline(832);
        _v559 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(833);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(834);
        _v560 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
        px_srcline(835);
         _v558 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v558, _v560, _v559}, 5);
    }
    px_srcline(836);
    return _v558;
px_err_561:
    if (px_err_561_proped) return px_err_561_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitand");
    LXValue _v562 = px_null();
    LXValue _v563 = px_null();
    LXValue _v564 = px_null();
    LXValue px_err_565_val = px_null();
    int px_err_565_proped = 0;
    px_srcline(838);
    _v562 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    px_srcline(839);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("&")}, 1))) {
        px_srcline(840);
        _v563 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(841);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(842);
        _v564 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
        px_srcline(843);
         _v562 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v562, _v564, _v563}, 5);
    }
    px_srcline(844);
    return _v562;
px_err_565:
    if (px_err_565_proped) return px_err_565_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_shift");
    LXValue _v566 = px_null();
    LXValue _v567 = px_null();
    LXValue _v568 = px_null();
    LXValue _v569 = px_null();
    LXValue px_err_570_val = px_null();
    int px_err_570_proped = 0;
    px_srcline(846);
    _v566 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    px_srcline(847);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(848);
        _v567 = px_null();
        px_srcline(849);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<<")}, 1))) {
            px_srcline(850);
             _v567 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>")}, 1))) {
            px_srcline(852);
             _v567 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>>")}, 1))) {
            px_srcline(854);
             _v567 = px_str("ShrU");
        }
        px_srcline(855);
        if (px_is_truthy(px_eq(_v567, px_null()))) {
            px_srcline(856);
            break;
        }
        px_srcline(857);
        _v568 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(858);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(859);
        _v569 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
        px_srcline(860);
         _v566 = px_list_n((LXValue[]){px_str("Binary"), _v567, _v566, _v569, _v568}, 5);
    }
    px_srcline(861);
    return _v566;
px_err_570:
    if (px_err_570_proped) return px_err_570_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_add");
    LXValue _v571 = px_null();
    LXValue _v572 = px_null();
    LXValue _v573 = px_null();
    LXValue _v574 = px_null();
    LXValue px_err_575_val = px_null();
    int px_err_575_proped = 0;
    px_srcline(863);
    _v571 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    px_srcline(864);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(865);
        _v572 = px_null();
        px_srcline(866);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("+")}, 1))) {
            px_srcline(867);
             _v572 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("-")}, 1))) {
            px_srcline(869);
             _v572 = px_str("Sub");
        }
        px_srcline(870);
        if (px_is_truthy(px_eq(_v572, px_null()))) {
            px_srcline(871);
            break;
        }
        px_srcline(872);
        _v573 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(873);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(874);
        _v574 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
        px_srcline(875);
         _v571 = px_list_n((LXValue[]){px_str("Binary"), _v572, _v571, _v574, _v573}, 5);
    }
    px_srcline(876);
    return _v571;
px_err_575:
    if (px_err_575_proped) return px_err_575_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_mul");
    LXValue _v576 = px_null();
    LXValue _v577 = px_null();
    LXValue _v578 = px_null();
    LXValue _v579 = px_null();
    LXValue px_err_580_val = px_null();
    int px_err_580_proped = 0;
    px_srcline(878);
    _v576 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    px_srcline(879);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(880);
        _v577 = px_null();
        px_srcline(881);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("*")}, 1))) {
            px_srcline(882);
             _v577 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("/")}, 1))) {
            px_srcline(884);
             _v577 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("//")}, 1))) {
            px_srcline(886);
             _v577 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("%")}, 1))) {
            px_srcline(888);
             _v577 = px_str("Mod");
        }
        px_srcline(889);
        if (px_is_truthy(px_eq(_v577, px_null()))) {
            px_srcline(890);
            break;
        }
        px_srcline(891);
        _v578 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(892);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(893);
        _v579 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(894);
         _v576 = px_list_n((LXValue[]){px_str("Binary"), _v577, _v576, _v579, _v578}, 5);
    }
    px_srcline(895);
    return _v576;
px_err_580:
    if (px_err_580_proped) return px_err_580_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pow");
    LXValue _v581 = px_null();
    LXValue _v582 = px_null();
    LXValue _v583 = px_null();
    LXValue px_err_584_val = px_null();
    int px_err_584_proped = 0;
    px_srcline(897);
    _v581 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    px_srcline(898);
    if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("**")}, 1))) {
        px_srcline(899);
        _v582 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(900);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(901);
        _v583 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(902);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v581, _v583, _v582}, 5);
    }
    px_srcline(903);
    return _v581;
px_err_584:
    if (px_err_584_proped) return px_err_584_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_unary");
    LXValue _v585 = px_null();
    LXValue _v586 = px_null();
    LXValue _v587 = px_null();
    LXValue px_err_588_val = px_null();
    int px_err_588_proped = 0;
    px_srcline(912);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(913);
    _v585 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(914);
    if (px_is_truthy(px_eq(_v585, px_str("-")))) {
        px_srcline(915);
        _v586 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(916);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(917);
        _v587 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(918);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v587, _v586}, 4);
    }
    px_srcline(919);
    if (px_is_truthy(px_eq(_v585, px_str("not")))) {
        px_srcline(920);
        _v586 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(921);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(922);
        _v587 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(923);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v587, _v586}, 4);
    }
    px_srcline(924);
    if (px_is_truthy(px_eq(_v585, px_str("~")))) {
        px_srcline(925);
        _v586 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(926);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(927);
        _v587 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(928);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v587, _v586}, 4);
    }
    px_srcline(929);
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_588:
    if (px_err_588_proped) return px_err_588_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_postfix");
    LXValue _v589 = px_null();
    LXValue _v590 = px_null();
    LXValue _v591 = px_null();
    LXValue _v592 = px_null();
    LXValue _v593 = px_null();
    LXValue _v594 = px_null();
    LXValue _v595 = px_null();
    LXValue _v596 = px_null();
    LXValue px_err_597_val = px_null();
    int px_err_597_proped = 0;
    px_srcline(931);
    _v589 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    px_srcline(932);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(933);
        _v590 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(934);
        if (px_is_truthy(px_eq(_v590, px_str("(")))) {
            px_srcline(935);
            _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(936);
            _v592 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(937);
             _v589 = px_list_n((LXValue[]){px_str("Call"), _v589, _v592, _v591}, 4);
        }
        else if (px_is_truthy(px_eq(_v590, px_str("[")))) {
            px_srcline(939);
            _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(940);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(941);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(942);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(943);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(944);
                _v593 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                px_srcline(945);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(946);
                _v594 = px_null();
                px_srcline(947);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(948);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(949);
                     _v594 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(950);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                }
                px_srcline(951);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                px_srcline(952);
                 _v589 = px_list_n((LXValue[]){px_str("Slice"), _v589, px_null(), _v593, _v594, _v591}, 6);
            }
            else {
                px_srcline(954);
                _v595 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                px_srcline(955);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(956);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(957);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(958);
                    _v593 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(959);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    px_srcline(960);
                    _v594 = px_null();
                    px_srcline(961);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        px_srcline(962);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(963);
                         _v594 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                        px_srcline(964);
                        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    }
                    px_srcline(965);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(966);
                     _v589 = px_list_n((LXValue[]){px_str("Slice"), _v589, _v595, _v593, _v594, _v591}, 6);
                }
                else {
                    px_srcline(968);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(969);
                     _v589 = px_list_n((LXValue[]){px_str("Index"), _v589, _v595, _v591}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v590, px_str(".")))) {
            px_srcline(971);
            _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(972);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(973);
            _v596 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(974);
             _v589 = px_list_n((LXValue[]){px_str("Field"), _v589, px_call(px_get_global("qstr"), (LXValue[]){_v596}, 1), _v591}, 4);
        }
        else if (px_is_truthy(px_eq(_v590, px_str("?.")))) {
            px_srcline(976);
            _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(977);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(978);
            _v596 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(979);
             _v589 = px_list_n((LXValue[]){px_str("OptionalField"), _v589, px_call(px_get_global("qstr"), (LXValue[]){_v596}, 1), _v591}, 4);
        }
        else if (px_is_truthy(px_eq(_v590, px_str("!")))) {
            px_srcline(981);
            _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(982);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(983);
             _v589 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v589, _v591}, 3);
        }
        else if (px_is_truthy(px_eq(_v590, px_str("?")))) {
            px_srcline(985);
            _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(986);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(987);
             _v589 = px_list_n((LXValue[]){px_str("Try"), _v589, _v591}, 3);
        }
        else {
            px_srcline(989);
            break;
        }
    }
    px_srcline(990);
    return _v589;
px_err_597:
    if (px_err_597_proped) return px_err_597_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_slice_bound");
    LXValue px_err_598_val = px_null();
    int px_err_598_proped = 0;
    px_srcline(992);
    if (px_is_truthy(({ LXValue _t599 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t599) ? _t599 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        px_srcline(993);
        return px_null();
    }
    px_srcline(994);
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_598:
    if (px_err_598_proped) return px_err_598_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_call_args");
    LXValue _v600 = px_null();
    LXValue px_err_601_val = px_null();
    int px_err_601_proped = 0;
    px_srcline(996);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(997);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(998);
    _v600 = px_list_n((LXValue[]){}, 0);
    px_srcline(999);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(1000);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1001);
            (void)(px_method(_v600, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
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
    return _v600;
px_err_601:
    if (px_err_601_proped) return px_err_601_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_primary");
    LXValue _v602 = px_null();
    LXValue _v603 = px_null();
    LXValue _v604 = px_null();
    LXValue _v605 = px_null();
    LXValue _v606 = px_null();
    LXValue _v607 = px_null();
    LXValue px_err_608_val = px_null();
    int px_err_608_proped = 0;
    px_srcline(1014);
    _v602 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1015);
    if (px_is_truthy(px_eq(_v602, px_str("整数")))) {
        px_srcline(1016);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1017);
        _v604 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1018);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1019);
        return px_list_n((LXValue[]){px_str("Int"), _v604, _v603}, 3);
    }
    px_srcline(1020);
    if (px_is_truthy(px_eq(_v602, px_str("浮点")))) {
        px_srcline(1021);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1022);
        _v604 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1023);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1024);
        return px_list_n((LXValue[]){px_str("Float"), _v604, _v603}, 3);
    }
    px_srcline(1025);
    if (px_is_truthy(px_eq(_v602, px_str("字符串")))) {
        px_srcline(1026);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1027);
        _v604 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1028);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1029);
        return px_list_n((LXValue[]){px_str("Str"), _v604, _v603}, 3);
    }
    px_srcline(1030);
    if (px_is_truthy(px_eq(_v602, px_str("true")))) {
        px_srcline(1031);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1032);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1033);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v603}, 3);
    }
    px_srcline(1034);
    if (px_is_truthy(px_eq(_v602, px_str("false")))) {
        px_srcline(1035);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1036);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1037);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v603}, 3);
    }
    px_srcline(1038);
    if (px_is_truthy(px_eq(_v602, px_str("null")))) {
        px_srcline(1039);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1040);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1041);
        return px_list_n((LXValue[]){px_str("Null"), _v603}, 2);
    }
    px_srcline(1042);
    if (px_is_truthy(px_eq(_v602, px_str("self")))) {
        px_srcline(1043);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1044);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1045);
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v603}, 3);
    }
    px_srcline(1046);
    if (px_is_truthy(px_eq(_v602, px_str("标识符")))) {
        px_srcline(1047);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1048);
        _v605 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1049);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1050);
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v605}, 1), _v603}, 3);
    }
    px_srcline(1051);
    if (px_is_truthy(px_eq(_v602, px_str("[")))) {
        px_srcline(1052);
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    px_srcline(1053);
    if (px_is_truthy(px_eq(_v602, px_str("(")))) {
        px_srcline(1054);
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    px_srcline(1055);
    if (px_is_truthy(px_eq(_v602, px_str("{")))) {
        px_srcline(1056);
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    px_srcline(1057);
    if (px_is_truthy(px_eq(_v602, px_str("fn")))) {
        px_srcline(1058);
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    px_srcline(1059);
    if (px_is_truthy(px_eq(_v602, px_str("match")))) {
        px_srcline(1060);
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1061);
    if (px_is_truthy(px_eq(_v602, px_str("if")))) {
        px_srcline(1062);
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1063);
    if (px_is_truthy(px_eq(_v602, px_str("chan")))) {
        px_srcline(1064);
        _v603 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1065);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1066);
        _v606 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v603}, 3);
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
            _v607 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(1073);
             _v606 = px_list_n((LXValue[]){px_str("Call"), _v606, _v607, _v603}, 4);
        }
        px_srcline(1074);
        return _v606;
    }
    px_srcline(1075);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1076);
    return px_null();
px_err_608:
    if (px_err_608_proped) return px_err_608_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_list_or_comp");
    LXValue _v609 = px_null();
    LXValue _v610 = px_null();
    LXValue _v611 = px_null();
    LXValue _v612 = px_null();
    LXValue px_err_613_val = px_null();
    int px_err_613_proped = 0;
    px_srcline(1078);
    _v609 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1079);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1080);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1081);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        px_srcline(1082);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1083);
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v609}, 3);
    }
    px_srcline(1084);
    _v610 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1085);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1086);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1087);
        _v611 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1088);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1089);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1090);
        return px_list_n((LXValue[]){px_str("ListComp"), _v610, px_index(_v611, px_int(0LL)), px_index(_v611, px_int(1LL)), _v609}, 5);
    }
    px_srcline(1091);
    _v612 = px_list_n((LXValue[]){_v610}, 1);
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
        (void)(px_method(_v612, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        px_srcline(1098);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    }
    px_srcline(1099);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(1100);
    return px_list_n((LXValue[]){px_str("List"), _v612, _v609}, 3);
px_err_613:
    if (px_err_613_proped) return px_err_613_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_vars");
    LXValue _v614 = px_null();
    LXValue px_err_615_val = px_null();
    int px_err_615_proped = 0;
    px_srcline(1102);
    _v614 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    px_srcline(1103);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1104);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1105);
        (void)(px_method(_v614, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    px_srcline(1106);
    return _v614;
px_err_615:
    if (px_err_615_proped) return px_err_615_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_clauses");
    LXValue _v616 = px_null();
    LXValue _v617 = px_null();
    LXValue _v618 = px_null();
    LXValue _v619 = px_null();
    LXValue px_err_620_val = px_null();
    int px_err_620_proped = 0;
    px_srcline(1108);
    _v616 = px_list_n((LXValue[]){}, 0);
    px_srcline(1109);
    _v617 = px_list_n((LXValue[]){}, 0);
    px_srcline(1110);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1111);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1112);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1113);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1114);
            _v618 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            px_srcline(1115);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            px_srcline(1116);
            _v619 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1117);
            (void)(px_method(_v616, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v618, _v619}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1119);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1120);
            (void)(px_method(_v617, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(1122);
            break;
        }
    }
    px_srcline(1123);
    return px_list_n((LXValue[]){_v616, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v617}, 1)}, 2);
px_err_620:
    if (px_err_620_proped) return px_err_620_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fold_comp_conds");
    LXValue _v621 = (nargs > 0) ? args[0] : px_null();
    LXValue _v622 = px_null();
    LXValue _v623 = px_null();
    LXValue _v624 = px_null();
    LXValue px_err_625_val = px_null();
    int px_err_625_proped = 0;
    px_srcline(1125);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v621}, 1), px_int(0LL)))) {
        px_srcline(1126);
        return px_null();
    }
    px_srcline(1127);
    _v622 = px_index(_v621, px_int(0LL));
    px_srcline(1128);
    _v623 = px_int(1LL);
    px_srcline(1129);
    while (px_is_truthy(px_lt(_v623, px_call(px_get_global("len"), (LXValue[]){_v621}, 1)))) {
        px_srcline(1130);
        _v624 = px_call(px_get_global("node_pos"), (LXValue[]){_v622}, 1);
        px_srcline(1131);
         _v622 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v622, px_index(_v621, _v623), _v624}, 5);
        px_srcline(1132);
         _v623 = px_add(_v623, px_int(1LL));
    }
    px_srcline(1133);
    return _v622;
px_err_625:
    if (px_err_625_proped) return px_err_625_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_paren_or_tuple");
    LXValue _v626 = px_null();
    LXValue _v627 = px_null();
    LXValue _v628 = px_null();
    LXValue _v629 = px_null();
    LXValue px_err_630_val = px_null();
    int px_err_630_proped = 0;
    px_srcline(1135);
    _v626 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1136);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1137);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1138);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        px_srcline(1139);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1140);
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v626}, 3);
    }
    px_srcline(1141);
    _v627 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1142);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1143);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1144);
        _v628 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1145);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1146);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1147);
        return px_list_n((LXValue[]){px_str("GenExp"), _v627, px_index(_v628, px_int(0LL)), px_index(_v628, px_int(1LL)), _v626}, 5);
    }
    px_srcline(1148);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1149);
        _v629 = px_list_n((LXValue[]){_v627}, 1);
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
            (void)(px_method(_v629, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1156);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        }
        px_srcline(1157);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1158);
        return px_list_n((LXValue[]){px_str("Tuple"), _v629, _v626}, 3);
    }
    px_srcline(1159);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1160);
    return _v627;
px_err_630:
    if (px_err_630_proped) return px_err_630_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("brace_looks_like_dict");
    LXValue _v631 = px_null();
    LXValue _v632 = px_null();
    LXValue _v633 = px_null();
    LXValue px_err_634_val = px_null();
    int px_err_634_proped = 0;
    px_srcline(1162);
    _v631 = px_int(0LL);
    px_srcline(1163);
    _v632 = px_get_global("p_pos");
    px_srcline(1164);
    while (px_is_truthy(px_lt(_v632, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(1165);
        _v633 = px_index(px_index(px_get_global("p_toks"), _v632), px_int(0LL));
        px_srcline(1166);
        if (px_is_truthy(({ LXValue _t635 = px_eq(_v633, px_str(":")); px_is_truthy(_t635) ? px_eq(_v631, px_int(0LL)) : _t635; }))) {
            px_srcline(1167);
            return px_bool(true);
        }
        px_srcline(1168);
        if (px_is_truthy(({ LXValue _t637 = ({ LXValue _t636 = px_eq(_v633, px_str("(")); px_is_truthy(_t636) ? _t636 : px_eq(_v633, px_str("[")); }); px_is_truthy(_t637) ? _t637 : px_eq(_v633, px_str("{")); }))) {
            px_srcline(1169);
             _v631 = px_add(_v631, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t638 = px_eq(_v633, px_str(")")); px_is_truthy(_t638) ? _t638 : px_eq(_v633, px_str("]")); }))) {
            px_srcline(1171);
            if (px_is_truthy(px_gt(_v631, px_int(0LL)))) {
                px_srcline(1172);
                 _v631 = px_sub(_v631, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t639 = px_eq(_v633, px_str("}")); px_is_truthy(_t639) ? px_eq(_v631, px_int(0LL)) : _t639; }))) {
            px_srcline(1174);
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t642 = ({ LXValue _t641 = ({ LXValue _t640 = px_eq(_v633, px_str(",")); px_is_truthy(_t640) ? _t640 : px_eq(_v633, px_str("换行")); }); px_is_truthy(_t641) ? _t641 : px_eq(_v633, px_str("EOF")); }); px_is_truthy(_t642) ? px_eq(_v631, px_int(0LL)) : _t642; }))) {
            px_srcline(1176);
            return px_bool(false);
        }
        px_srcline(1177);
         _v632 = px_add(_v632, px_int(1LL));
    }
    px_srcline(1178);
    return px_bool(false);
px_err_634:
    if (px_err_634_proped) return px_err_634_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_brace");
    LXValue _v643 = px_null();
    LXValue _v644 = px_null();
    LXValue _v645 = px_null();
    LXValue _v646 = px_null();
    LXValue _v647 = px_null();
    LXValue _v648 = px_null();
    LXValue _v649 = px_null();
    LXValue _v650 = px_null();
    LXValue _v651 = px_null();
    LXValue px_err_652_val = px_null();
    int px_err_652_proped = 0;
    px_srcline(1180);
    _v643 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
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
        return px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){}, 0), _v643}, 3);
    }
    px_srcline(1197);
    _v644 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    px_srcline(1198);
    if (px_is_truthy(_v644)) {
        px_srcline(1199);
        _v645 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1200);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1201);
        _v646 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1202);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1203);
            _v647 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            px_srcline(1204);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            px_srcline(1205);
            return px_list_n((LXValue[]){px_str("DictComp"), _v645, _v646, px_index(_v647, px_int(0LL)), px_index(_v647, px_int(1LL)), _v643}, 6);
        }
        px_srcline(1206);
        _v648 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v645, _v646}, 2)}, 1);
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
            _v649 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1219);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(1220);
            _v650 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1221);
            (void)(px_method(_v648, "append", (LXValue[]){px_list_n((LXValue[]){_v649, _v650}, 2)}, 1));
            px_srcline(1222);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1223);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1224);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1225);
        return px_list_n((LXValue[]){px_str("Dict"), _v648, _v643}, 3);
    }
    px_srcline(1226);
    _v651 = px_list_n((LXValue[]){}, 0);
    px_srcline(1227);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1228);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1229);
    while (px_is_truthy(({ LXValue _t653 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t653) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t653; }))) {
        px_srcline(1230);
        (void)(px_method(_v651, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(1231);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1232);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    px_srcline(1233);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    px_srcline(1234);
    return px_list_n((LXValue[]){px_str("Block"), _v651, _v643}, 3);
px_err_652:
    if (px_err_652_proped) return px_err_652_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_closure");
    LXValue _v654 = px_null();
    LXValue _v655 = px_null();
    LXValue _v656 = px_null();
    LXValue _v657 = px_null();
    LXValue _v658 = px_null();
    LXValue _v659 = px_null();
    LXValue _v660 = px_null();
    LXValue px_err_661_val = px_null();
    int px_err_661_proped = 0;
    px_srcline(1236);
    _v654 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1237);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1238);
    _v655 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(1239);
    _v656 = px_null();
    px_srcline(1240);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(1241);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1242);
         _v656 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(1243);
    _v657 = px_list_n((LXValue[]){}, 0);
    px_srcline(1244);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        px_srcline(1245);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1246);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1247);
            (void)(px_method(_v657, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
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
    _v658 = px_null();
    px_srcline(1253);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        px_srcline(1254);
        _v659 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1255);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1256);
        _v660 = px_list_n((LXValue[]){}, 0);
        px_srcline(1257);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1258);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1259);
        while (px_is_truthy(({ LXValue _t662 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t662) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t662; }))) {
            px_srcline(1260);
            (void)(px_method(_v660, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            px_srcline(1261);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1262);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1263);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1264);
         _v658 = px_list_n((LXValue[]){px_str("Block"), _v660, _v659}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(1266);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1270);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(1271);
            _v659 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1272);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1273);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(1274);
                _v660 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
                px_srcline(1275);
                 _v658 = px_list_n((LXValue[]){px_str("Block"), _v660, _v659}, 3);
            }
            else {
                px_srcline(1282);
                _v660 = px_list_n((LXValue[]){}, 0);
                px_srcline(1283);
                px_set_global("p_paren_ctxt", px_add(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1284);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1285);
                    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
                    px_srcline(1286);
                    if (px_is_truthy(({ LXValue _t667 = ({ LXValue _t666 = ({ LXValue _t665 = ({ LXValue _t664 = ({ LXValue _t663 = px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1); px_is_truthy(_t663) ? _t663 : px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); }); px_is_truthy(_t664) ? _t664 : px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1); }); px_is_truthy(_t665) ? _t665 : px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1); }); px_is_truthy(_t666) ? _t666 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t667) ? _t667 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
                        px_srcline(1287);
                        break;
                    }
                    px_srcline(1288);
                    (void)(px_method(_v660, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
                }
                px_srcline(1289);
                px_set_global("p_paren_ctxt", px_sub(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1290);
                 _v658 = px_list_n((LXValue[]){px_str("Block"), _v660, _v659}, 3);
            }
        }
        else {
            px_srcline(1292);
             _v658 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
    }
    else {
        px_srcline(1294);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    px_srcline(1295);
    return px_list_n((LXValue[]){px_str("Closure"), _v655, _v656, _v658, _v657, _v654}, 6);
px_err_661:
    if (px_err_661_proped) return px_err_661_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_match_expr");
    LXValue _v668 = px_null();
    LXValue _v669 = px_null();
    LXValue _v670 = px_null();
    LXValue _v671 = px_null();
    LXValue _v672 = px_null();
    LXValue _v673 = px_null();
    LXValue _v674 = px_null();
    LXValue _v675 = px_null();
    LXValue px_err_676_val = px_null();
    int px_err_676_proped = 0;
    px_srcline(1297);
    _v668 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1298);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1299);
    _v669 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1300);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1301);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(1302);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(1303);
    _v670 = px_list_n((LXValue[]){}, 0);
    px_srcline(1304);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1305);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(1306);
        if (px_is_truthy(({ LXValue _t677 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t677) ? _t677 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(1307);
            break;
        }
        px_srcline(1308);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(1309);
        _v671 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        px_srcline(1310);
        _v672 = px_null();
        px_srcline(1311);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1312);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1313);
             _v672 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1314);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1315);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(1316);
        _v673 = px_null();
        px_srcline(1317);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            px_srcline(1318);
            _v674 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(1319);
            _v675 = px_null();
            px_srcline(1320);
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v674}, 1), px_int(0LL)))) {
                px_srcline(1321);
                 _v675 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v674, px_int(0LL))}, 1);
            }
            else {
                px_srcline(1323);
                 _v675 = _v668;
            }
            px_srcline(1324);
             _v673 = px_list_n((LXValue[]){px_str("Block"), _v674, _v675}, 3);
        }
        else {
            px_srcline(1326);
             _v673 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1327);
        (void)(px_method(_v670, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v671, _v672, _v673, _v668}, 5)}, 1));
    }
    px_srcline(1328);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(1329);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    px_srcline(1330);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(1331);
    return px_list_n((LXValue[]){px_str("Match"), _v669, _v670, _v668}, 4);
px_err_676:
    if (px_err_676_proped) return px_err_676_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if_expr");
    LXValue _v678 = px_null();
    LXValue _v679 = px_null();
    LXValue _v680 = px_null();
    LXValue _v681 = px_null();
    LXValue px_err_682_val = px_null();
    int px_err_682_proped = 0;
    px_srcline(1333);
    _v678 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1334);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1335);
    _v679 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1336);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1337);
    _v680 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1338);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    px_srcline(1339);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1340);
    _v681 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1341);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v679, _v680, _v681, _v678}, 5);
px_err_682:
    if (px_err_682_proped) return px_err_682_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pattern");
    LXValue _v683 = px_null();
    LXValue _v684 = px_null();
    LXValue _v685 = px_null();
    LXValue _v686 = px_null();
    LXValue _v687 = px_null();
    LXValue _v688 = px_null();
    LXValue _v689 = px_null();
    LXValue _v690 = px_null();
    LXValue px_err_691_val = px_null();
    int px_err_691_proped = 0;
    px_srcline(1344);
    _v683 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1345);
    if (px_is_truthy(({ LXValue _t696 = ({ LXValue _t695 = ({ LXValue _t694 = ({ LXValue _t693 = ({ LXValue _t692 = px_eq(_v683, px_str("整数")); px_is_truthy(_t692) ? _t692 : px_eq(_v683, px_str("浮点")); }); px_is_truthy(_t693) ? _t693 : px_eq(_v683, px_str("字符串")); }); px_is_truthy(_t694) ? _t694 : px_eq(_v683, px_str("true")); }); px_is_truthy(_t695) ? _t695 : px_eq(_v683, px_str("false")); }); px_is_truthy(_t696) ? _t696 : px_eq(_v683, px_str("null")); }))) {
        px_srcline(1346);
        _v684 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        px_srcline(1347);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v684}, 2);
    }
    px_srcline(1348);
    if (px_is_truthy(px_eq(_v683, px_str("标识符")))) {
        px_srcline(1349);
        _v685 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1350);
        _v686 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1351);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1352);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(1354);
            _v687 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1355);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1356);
            _v688 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(1357);
            return px_list_n((LXValue[]){px_str("PatLiteral"), px_list_n((LXValue[]){px_str("Field"), px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v685}, 1), _v687}, 3), px_call(px_get_global("qstr"), (LXValue[]){_v688}, 1), _v687}, 4)}, 2);
        }
        px_srcline(1358);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1359);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1360);
            _v689 = px_list_n((LXValue[]){}, 0);
            px_srcline(1361);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(1362);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1363);
                    (void)(px_method(_v689, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
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
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v685}, 1), _v689}, 3);
        }
        px_srcline(1370);
        if (px_is_truthy(px_eq(_v685, px_str("_")))) {
            px_srcline(1371);
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        px_srcline(1372);
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v685}, 1))) {
            px_srcline(1373);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v685}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        px_srcline(1374);
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v685}, 1)}, 2);
    }
    px_srcline(1375);
    if (px_is_truthy(px_eq(_v683, px_str("(")))) {
        px_srcline(1376);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1377);
        _v690 = px_list_n((LXValue[]){}, 0);
        px_srcline(1378);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1379);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1380);
                (void)(px_method(_v690, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
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
        return px_list_n((LXValue[]){px_str("PatTuple"), _v690}, 2);
    }
    px_srcline(1387);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1388);
    return px_null();
px_err_691:
    if (px_err_691_proped) return px_err_691_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_upper");
    LXValue _v697 = (nargs > 0) ? args[0] : px_null();
    LXValue _v698 = px_null();
    LXValue px_err_699_val = px_null();
    int px_err_699_proped = 0;
    px_srcline(1390);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v697}, 1), px_int(0LL)))) {
        px_srcline(1391);
        return px_bool(false);
    }
    px_srcline(1392);
    _v698 = px_index(_v697, px_int(0LL));
    px_srcline(1393);
    return ({ LXValue _t700 = px_ge(_v698, px_str("A")); px_is_truthy(_t700) ? px_le(_v698, px_str("Z")) : _t700; });
px_err_699:
    if (px_err_699_proped) return px_err_699_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type");
    LXValue _v701 = px_null();
    LXValue _v702 = px_null();
    LXValue px_err_703_val = px_null();
    int px_err_703_proped = 0;
    px_srcline(1396);
    _v701 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    px_srcline(1397);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        px_srcline(1398);
        _v702 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1399);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1400);
        return px_list_n((LXValue[]){px_str("TyOptional"), _v701, _v702}, 3);
    }
    px_srcline(1401);
    return _v701;
px_err_703:
    if (px_err_703_proped) return px_err_703_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_base");
    LXValue _v704 = px_null();
    LXValue _v705 = px_null();
    LXValue _v706 = px_null();
    LXValue _v707 = px_null();
    LXValue _v708 = px_null();
    LXValue _v709 = px_null();
    LXValue _v710 = px_null();
    LXValue _v711 = px_null();
    LXValue px_err_712_val = px_null();
    int px_err_712_proped = 0;
    px_srcline(1403);
    _v704 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1404);
    if (px_is_truthy(px_eq(_v704, px_str("标识符")))) {
        px_srcline(1405);
        _v705 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1406);
        _v706 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1407);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1408);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1409);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1410);
            _v707 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1411);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            px_srcline(1412);
            if (px_is_truthy(px_eq(_v705, px_str("list")))) {
                px_srcline(1413);
                return px_list_n((LXValue[]){px_str("TyList"), _v707, _v706}, 3);
            }
            px_srcline(1414);
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v705}, 1), px_list_n((LXValue[]){_v707}, 1), _v706}, 4);
        }
        px_srcline(1415);
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v705}, 1), _v706}, 3);
    }
    px_srcline(1416);
    if (px_is_truthy(px_eq(_v704, px_str("[")))) {
        px_srcline(1417);
        _v706 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1418);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1419);
        _v707 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1420);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1421);
        return px_list_n((LXValue[]){px_str("TyList"), _v707, _v706}, 3);
    }
    px_srcline(1422);
    if (px_is_truthy(px_eq(_v704, px_str("{")))) {
        px_srcline(1423);
        _v706 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1424);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1425);
        _v708 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1426);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1427);
        _v709 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1428);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1429);
        return px_list_n((LXValue[]){px_str("TyDict"), _v708, _v709, _v706}, 4);
    }
    px_srcline(1430);
    if (px_is_truthy(px_eq(_v704, px_str("(")))) {
        px_srcline(1431);
        _v706 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1432);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1433);
        _v710 = px_list_n((LXValue[]){}, 0);
        px_srcline(1434);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1435);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1436);
                (void)(px_method(_v710, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
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
            _v711 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1445);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v710, _v711, _v706}, 4);
        }
        px_srcline(1446);
        return px_list_n((LXValue[]){px_str("TyTuple"), _v710, _v706}, 3);
    }
    px_srcline(1447);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1448);
    return px_null();
px_err_712:
    if (px_err_712_proped) return px_err_712_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt_inner");
    LXValue _v713 = (nargs > 0) ? args[0] : px_null();
    LXValue _v714 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v751 = px_null();
    LXValue _v752 = px_null();
    LXValue _v753 = px_null();
    LXValue _v754 = px_null();
    LXValue _v755 = px_null();
    LXValue px_err_756_val = px_null();
    int px_err_756_proped = 0;
    px_srcline(9);
    LXValue _v757 = px_call(px_get_global("cg_pad"), (LXValue[]){_v714}, 1);
    px_srcline(10);
    _v715 = px_index(_v713, px_int(0LL));
    px_srcline(11);
    if (px_is_truthy(px_eq(_v715, px_str("VarDecl")))) {
        px_srcline(12);
        _v716 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v713, px_int(2LL))}, 1);
        px_srcline(15);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v713}, 1));
        px_srcline(16);
        _v717 = px_str("px_null()");
        px_srcline(17);
        if (px_is_truthy(px_ne(px_index(_v713, px_int(4LL)), px_null()))) {
            px_srcline(18);
             _v717 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(4LL))}, 1);
        }
        px_srcline(20);
        if (px_is_truthy(({ LXValue _t758 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v716}, 2); px_is_truthy(_t758) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)) : _t758; }))) {
            px_srcline(21);
            return px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_set_global(\"")), _v716), px_str("\", ")), _v717), px_str(");\n"));
        }
        px_srcline(22);
        _v718 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v716}, 1);
        px_srcline(23);
        if (px_is_truthy(px_eq(_v718, px_null()))) {
            px_srcline(25);
             _v718 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v716}, 1);
            px_srcline(27);
            if (px_is_truthy(px_ne(px_index(_v713, px_int(4LL)), px_null()))) {
                px_srcline(28);
                _v719 = px_index(_v713, px_int(4LL));
                px_srcline(29);
                _v720 = px_null();
                px_srcline(30);
                if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Constructor")))) {
                    px_srcline(31);
                     _v720 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v719, px_int(1LL))}, 1);
                }
                else if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Call")))) {
                    px_srcline(33);
                    _v721 = px_index(_v719, px_int(1LL));
                    px_srcline(34);
                    if (px_is_truthy(px_eq(px_index(_v721, px_int(0LL)), px_str("Var")))) {
                        px_srcline(35);
                         _v720 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v721, px_int(1LL))}, 1);
                    }
                }
                px_srcline(36);
                if (px_is_truthy(px_ne(_v720, px_null()))) {
                    px_srcline(37);
                    if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v720}, 1))) {
                        px_srcline(38);
                        px_index_set(px_get_global("cg_var_types"), _v716, _v720);
                    }
                }
            }
            px_srcline(39);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v716}, 1))) {
                px_srcline(40);
                return px_add(px_add(px_add(px_add(px_add(_v757, px_str("LXValue ")), _v718), px_str(" = px_cell(")), _v717), px_str(");\n"));
            }
            px_srcline(41);
            return px_add(px_add(px_add(px_add(px_add(_v757, px_str("LXValue ")), _v718), px_str(" = ")), _v717), px_str(";\n"));
        }
        px_srcline(44);
        if (px_is_truthy(px_ne(px_index(_v713, px_int(4LL)), px_null()))) {
            px_srcline(45);
            _v719 = px_index(_v713, px_int(4LL));
            px_srcline(46);
            _v720 = px_null();
            px_srcline(47);
            if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Constructor")))) {
                px_srcline(48);
                 _v720 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v719, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v719, px_int(0LL)), px_str("Call")))) {
                px_srcline(50);
                _v721 = px_index(_v719, px_int(1LL));
                px_srcline(51);
                if (px_is_truthy(px_eq(px_index(_v721, px_int(0LL)), px_str("Var")))) {
                    px_srcline(52);
                     _v720 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v721, px_int(1LL))}, 1);
                }
            }
            px_srcline(53);
            if (px_is_truthy(px_ne(_v720, px_null()))) {
                px_srcline(54);
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v720}, 1))) {
                    px_srcline(55);
                    px_index_set(px_get_global("cg_var_types"), _v716, _v720);
                }
            }
        }
        px_srcline(56);
        return px_add(px_add(_v757, px_call(px_get_global("cg_store_of"), (LXValue[]){_v716, _v718, _v717}, 3)), px_str(";\n"));
    }
    px_srcline(57);
    if (px_is_truthy(px_eq(_v715, px_str("Assign")))) {
        px_srcline(58);
        _v717 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(3LL))}, 1);
        px_srcline(59);
        _v722 = px_index(_v713, px_int(1LL));
        px_srcline(60);
        _v723 = px_index(_v713, px_int(2LL));
        px_srcline(62);
        if (px_is_truthy(px_eq(_v723, px_str("Append")))) {
            px_srcline(63);
            _v724 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v722}, 1);
            px_srcline(64);
            return px_add(px_add(px_add(px_add(px_add(_v757, px_str("(void)(px_method(")), _v724), px_str(", \"append\", (LXValue[]){")), _v717), px_str("}, 1));\n"));
        }
        px_srcline(65);
        _v725 = px_index(_v722, px_int(0LL));
        px_srcline(66);
        if (px_is_truthy(px_eq(_v725, px_str("Var")))) {
            px_srcline(67);
            _v716 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v722, px_int(1LL))}, 1);
            px_srcline(69);
            (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v722, _v723, px_index(_v713, px_int(3LL))}, 3));
            px_srcline(70);
            _v726 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v716}, 1);
            px_srcline(71);
            if (px_is_truthy(px_eq(_v726, px_null()))) {
                px_srcline(73);
                if (px_is_truthy(px_eq(_v723, px_str("Assign")))) {
                    px_srcline(74);
                    return px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_set_global(\"")), _v716), px_str("\", ")), _v717), px_str(");\n"));
                }
                px_srcline(75);
                _v727 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v723, _v716, _v717}, 3);
                px_srcline(76);
                return px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_set_global(\"")), _v716), px_str("\", ")), _v727), px_str(");\n"));
            }
            px_srcline(77);
            _v727 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v723, px_call(px_get_global("cg_load_of"), (LXValue[]){_v716, _v726}, 2), _v717}, 3);
            px_srcline(78);
            return px_add(px_add(px_add(_v757, px_str(" ")), px_call(px_get_global("cg_store_of"), (LXValue[]){_v716, _v726, _v727}, 3)), px_str(";\n"));
        }
        px_srcline(79);
        if (px_is_truthy(px_eq(_v725, px_str("Field")))) {
            px_srcline(80);
            _v724 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v722, px_int(1LL))}, 1);
            px_srcline(81);
            _v728 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v722, px_int(2LL))}, 1);
            px_srcline(82);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_field_set(")), _v724), px_str(", \"")), _v728), px_str("\", ")), _v717), px_str(");\n"));
        }
        px_srcline(83);
        if (px_is_truthy(px_eq(_v725, px_str("Index")))) {
            px_srcline(84);
            _v724 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v722, px_int(1LL))}, 1);
            px_srcline(85);
            _v729 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v722, px_int(2LL))}, 1);
            px_srcline(86);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_index_set(")), _v724), px_str(", ")), _v729), px_str(", ")), _v717), px_str(");\n"));
        }
        px_srcline(87);
        return px_str("不支持的赋值目标");
    }
    px_srcline(88);
    if (px_is_truthy(px_eq(_v715, px_str("ExprStmt")))) {
        px_srcline(89);
        _v719 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
        px_srcline(90);
        return px_add(px_add(px_add(_v757, px_str("(void)(")), _v719), px_str(");\n"));
    }
    px_srcline(91);
    if (px_is_truthy(px_eq(_v715, px_str("If")))) {
        px_srcline(92);
        _v730 = px_str("");
        px_srcline(93);
        _v731 = px_index(_v713, px_int(1LL));
        px_srcline(94);
        _v732 = px_int(0LL);
        px_srcline(95);
        while (px_is_truthy(px_lt(_v732, px_call(px_get_global("len"), (LXValue[]){_v731}, 1)))) {
            px_srcline(96);
            _v733 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v731, _v732), px_int(0LL))}, 1);
            px_srcline(97);
            _v734 = px_str("if");
            px_srcline(98);
            if (px_is_truthy(px_gt(_v732, px_int(0LL)))) {
                px_srcline(99);
                 _v734 = px_str("else if");
            }
            px_srcline(100);
             _v730 = px_add(_v730, px_add(px_add(px_add(px_add(_v757, _v734), px_str(" (px_is_truthy(")), _v733), px_str(")) {\n")));
            px_srcline(101);
            _v735 = px_index(px_index(_v731, _v732), px_int(1LL));
            px_srcline(102);
            _v736 = px_int(0LL);
            px_srcline(103);
            while (px_is_truthy(px_lt(_v736, px_call(px_get_global("len"), (LXValue[]){_v735}, 1)))) {
                px_srcline(104);
                 _v730 = px_add(_v730, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v735, _v736), px_add(_v714, px_int(1LL))}, 2));
                px_srcline(105);
                 _v736 = px_add(_v736, px_int(1LL));
            }
            px_srcline(106);
             _v730 = px_add(_v730, px_add(_v757, px_str("}\n")));
            px_srcline(107);
             _v732 = px_add(_v732, px_int(1LL));
        }
        px_srcline(108);
        if (px_is_truthy(px_ne(px_index(_v713, px_int(2LL)), px_null()))) {
            px_srcline(109);
             _v730 = px_add(_v730, px_add(_v757, px_str("else {\n")));
            px_srcline(110);
            _v737 = px_index(_v713, px_int(2LL));
            px_srcline(111);
            _v738 = px_int(0LL);
            px_srcline(112);
            while (px_is_truthy(px_lt(_v738, px_call(px_get_global("len"), (LXValue[]){_v737}, 1)))) {
                px_srcline(113);
                 _v730 = px_add(_v730, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v737, _v738), px_add(_v714, px_int(1LL))}, 2));
                px_srcline(114);
                 _v738 = px_add(_v738, px_int(1LL));
            }
            px_srcline(115);
             _v730 = px_add(_v730, px_add(_v757, px_str("}\n")));
        }
        px_srcline(116);
        return _v730;
    }
    px_srcline(117);
    if (px_is_truthy(px_eq(_v715, px_str("While")))) {
        px_srcline(118);
        _v733 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
        px_srcline(119);
        _v730 = px_add(px_add(px_add(_v757, px_str("while (px_is_truthy(")), _v733), px_str(")) {\n"));
        px_srcline(120);
        _v735 = px_index(_v713, px_int(2LL));
        px_srcline(121);
        _v732 = px_int(0LL);
        px_srcline(122);
        while (px_is_truthy(px_lt(_v732, px_call(px_get_global("len"), (LXValue[]){_v735}, 1)))) {
            px_srcline(123);
             _v730 = px_add(_v730, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v735, _v732), px_add(_v714, px_int(1LL))}, 2));
            px_srcline(124);
             _v732 = px_add(_v732, px_int(1LL));
        }
        px_srcline(125);
         _v730 = px_add(_v730, px_add(_v757, px_str("}\n")));
        px_srcline(126);
        return _v730;
    }
    px_srcline(127);
    if (px_is_truthy(px_eq(_v715, px_str("For")))) {
        px_srcline(128);
        _v739 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(2LL))}, 1);
        px_srcline(129);
        _v740 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(130);
        _v741 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(131);
        _v742 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
        px_srcline(132);
        _v743 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v742}, 1);
        px_srcline(133);
        _v744 = px_str("LXValue ");
        px_srcline(134);
        if (px_is_truthy(({ LXValue _t759 = px_eq(_v743, px_null()); px_is_truthy(_t759) ? _t759 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)); }))) {
            px_srcline(137);
             _v743 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v742}, 1);
        }
        else {
            px_srcline(140);
             _v744 = px_str("");
        }
        px_srcline(141);
        _v745 = px_add(px_add(px_add(px_add(px_str("px_index("), _v740), px_str(", px_int(")), _v741), px_str("))"));
        px_srcline(142);
        _v746 = px_str("");
        px_srcline(143);
        if (px_is_truthy(px_eq(_v744, px_str("LXValue ")))) {
            px_srcline(145);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v742}, 1))) {
                px_srcline(146);
                 _v746 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v743), px_str(" = px_cell(")), _v745), px_str(");"));
            }
            else {
                px_srcline(148);
                 _v746 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v743), px_str(" = ")), _v745), px_str(";"));
            }
        }
        else {
            px_srcline(151);
             _v746 = px_add(px_call(px_get_global("cg_store_of"), (LXValue[]){_v742, _v743, _v745}, 3), px_str(";"));
        }
        px_srcline(152);
        _v730 = px_add(px_add(px_add(px_add(px_add(_v757, px_str("LXValue ")), _v740), px_str(" = ")), _v739), px_str(";\n"));
        px_srcline(153);
         _v730 = px_add(_v730, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v757, px_str("for (int ")), _v741), px_str(" = 0; ")), _v741), px_str(" < px_len(")), _v740), px_str("); ")), _v741), px_str("++) {\n")));
        px_srcline(154);
         _v730 = px_add(_v730, px_add(px_add(px_add(_v757, px_str("    ")), _v746), px_str("\n")));
        px_srcline(155);
        _v735 = px_index(_v713, px_int(3LL));
        px_srcline(156);
        _v732 = px_int(0LL);
        px_srcline(157);
        while (px_is_truthy(px_lt(_v732, px_call(px_get_global("len"), (LXValue[]){_v735}, 1)))) {
            px_srcline(158);
             _v730 = px_add(_v730, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v735, _v732), px_add(_v714, px_int(1LL))}, 2));
            px_srcline(159);
             _v732 = px_add(_v732, px_int(1LL));
        }
        px_srcline(160);
         _v730 = px_add(_v730, px_add(_v757, px_str("}\n")));
        px_srcline(161);
        return _v730;
    }
    px_srcline(162);
    if (px_is_truthy(px_eq(_v715, px_str("Return")))) {
        px_srcline(163);
        if (px_is_truthy(px_ne(px_index(_v713, px_int(1LL)), px_null()))) {
            px_srcline(164);
            _v719 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
            px_srcline(165);
            return px_add(px_add(px_add(_v757, px_str("return ")), _v719), px_str(";\n"));
        }
        px_srcline(166);
        return px_add(_v757, px_str("return px_null();\n"));
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v715, px_str("Break")))) {
        px_srcline(168);
        return px_add(_v757, px_str("break;\n"));
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v715, px_str("Continue")))) {
        px_srcline(170);
        return px_add(_v757, px_str("continue;\n"));
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v715, px_str("Empty")))) {
        px_srcline(172);
        return px_str("");
    }
    px_srcline(173);
    if (px_is_truthy(px_eq(_v715, px_str("ChanDecl")))) {
        px_srcline(174);
        _v726 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1)}, 1);
        px_srcline(175);
        return px_add(px_add(px_add(_v757, px_str("LXValue ")), _v726), px_str(" = px_chan_create(0);\n"));
    }
    px_srcline(176);
    if (px_is_truthy(px_eq(_v715, px_str("Send")))) {
        px_srcline(177);
        _v733 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
        px_srcline(178);
        _v726 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(2LL))}, 1);
        px_srcline(179);
        return px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_chan_send(")), _v733), px_str(", ")), _v726), px_str(");\n"));
    }
    px_srcline(180);
    if (px_is_truthy(px_eq(_v715, px_str("Recv")))) {
        px_srcline(181);
        _v733 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
        px_srcline(182);
        return px_add(px_add(px_add(_v757, px_str("px_chan_recv(")), _v733), px_str(");\n"));
    }
    px_srcline(183);
    if (px_is_truthy(px_eq(_v715, px_str("Spawn")))) {
        px_srcline(184);
        _v747 = px_index(_v713, px_int(1LL));
        px_srcline(185);
        if (px_is_truthy(px_eq(px_index(_v747, px_int(0LL)), px_str("Call")))) {
            px_srcline(186);
            _v721 = px_index(_v747, px_int(1LL));
            px_srcline(187);
            if (px_is_truthy(px_eq(px_index(_v721, px_int(0LL)), px_str("Var")))) {
                px_srcline(188);
                _v728 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v721, px_int(1LL))}, 1);
                px_srcline(189);
                _v748 = px_list_n((LXValue[]){}, 0);
                px_srcline(190);
                _v749 = px_index(_v747, px_int(2LL));
                px_srcline(191);
                _v750 = px_int(0LL);
                px_srcline(192);
                while (px_is_truthy(px_lt(_v750, px_call(px_get_global("len"), (LXValue[]){_v749}, 1)))) {
                    px_srcline(193);
                    (void)(px_method(_v748, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v749, _v750)}, 1)}, 1));
                    px_srcline(194);
                     _v750 = px_add(_v750, px_int(1LL));
                }
                px_srcline(195);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v757, px_str("px_spawn_name(\"")), _v728), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v748}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v748}, 1)}, 1)), px_str(");\n"));
            }
            px_srcline(196);
            _v751 = px_index(_v713, px_int(2LL));
            px_srcline(197);
            (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v751, px_int(0LL))}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_index(_v751, px_int(1LL))}, 1)), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)（方法调用请先绑定命名函数）"))}, 1));
            px_srcline(198);
            (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
        }
        px_srcline(199);
        _v752 = px_index(_v713, px_int(2LL));
        px_srcline(200);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v752, px_int(0LL))}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_index(_v752, px_int(1LL))}, 1)), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()）"))}, 1));
        px_srcline(201);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(202);
    if (px_is_truthy(px_eq(_v715, px_str("Select")))) {
        px_srcline(203);
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v713, px_int(1LL)), px_index(_v713, px_int(2LL)), _v714}, 3);
    }
    px_srcline(204);
    if (px_is_truthy(px_eq(_v715, px_str("Import")))) {
        px_srcline(205);
        return px_add(_v757, px_str("/* import 忽略（MVP） */\n"));
    }
    px_srcline(206);
    if (px_is_truthy(px_eq(_v715, px_str("FuncDef")))) {
        px_srcline(211);
        _v753 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v713, px_int(1LL))}, 1);
        px_srcline(212);
        _v754 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v753}, 1);
        px_srcline(215);
        _v755 = px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v713, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v713, px_int(4LL))}, 2), _v753}, 3);
        px_srcline(216);
        if (px_is_truthy(px_ne(_v754, px_null()))) {
            px_srcline(217);
            return px_add(px_add(_v757, px_call(px_get_global("cg_store_of"), (LXValue[]){_v753, _v754, _v755}, 3)), px_str(";\n"));
        }
        px_srcline(218);
        return px_str("");
    }
    px_srcline(220);
    return px_str("");
px_err_756:
    if (px_err_756_proped) return px_err_756_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt");
    LXValue _v760 = (nargs > 0) ? args[0] : px_null();
    LXValue _v761 = (nargs > 1) ? args[1] : px_null();
    LXValue _v762 = px_null();
    LXValue _v763 = px_null();
    LXValue _v764 = px_null();
    LXValue px_err_765_val = px_null();
    int px_err_765_proped = 0;
    px_srcline(226);
    _v762 = px_int(0LL);
    px_srcline(227);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v760}, 1), px_int(0LL)))) {
        px_srcline(228);
        _v763 = px_index(_v760, px_sub(px_call(px_get_global("len"), (LXValue[]){_v760}, 1), px_int(1LL)));
        px_srcline(229);
        if (px_is_truthy(({ LXValue _t767 = ({ LXValue _t766 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v763}, 1), px_str("list")); px_is_truthy(_t766) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v763}, 1), px_int(1LL)) : _t766; }); px_is_truthy(_t767) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v763, px_int(0LL))}, 1), px_str("int")) : _t767; }))) {
            px_srcline(230);
             _v762 = px_index(_v763, px_int(0LL));
        }
    }
    px_srcline(231);
    _v764 = px_str("");
    px_srcline(232);
    if (px_is_truthy(px_gt(_v762, px_int(0LL)))) {
        px_srcline(233);
         _v764 = px_add(px_add(px_add(px_call(px_get_global("cg_pad"), (LXValue[]){_v761}, 1), px_str("px_srcline(")), px_call(px_get_global("str"), (LXValue[]){_v762}, 1)), px_str(");\n"));
    }
    px_srcline(234);
    return px_add(_v764, px_call(px_get_global("cg_gen_stmt_inner"), (LXValue[]){_v760, _v761}, 2));
px_err_765:
    if (px_err_765_proped) return px_err_765_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_global");
    LXValue _v768 = (nargs > 0) ? args[0] : px_null();
    LXValue _v769 = (nargs > 1) ? args[1] : px_null();
    LXValue _v770 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_771_val = px_null();
    int px_err_771_proped = 0;
    px_srcline(237);
    if (px_is_truthy(px_eq(_v768, px_str("Assign")))) {
        px_srcline(238);
        return _v770;
    }
    px_srcline(239);
    if (px_is_truthy(px_eq(_v768, px_str("Plus")))) {
        px_srcline(240);
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(_v768, px_str("Minus")))) {
        px_srcline(242);
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(243);
    if (px_is_truthy(px_eq(_v768, px_str("Star")))) {
        px_srcline(244);
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(245);
    if (px_is_truthy(px_eq(_v768, px_str("Slash")))) {
        px_srcline(246);
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(247);
    if (px_is_truthy(px_eq(_v768, px_str("IntDiv")))) {
        px_srcline(248);
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(249);
    if (px_is_truthy(px_eq(_v768, px_str("Mod")))) {
        px_srcline(250);
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(251);
    if (px_is_truthy(px_eq(_v768, px_str("Pow")))) {
        px_srcline(252);
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(253);
    if (px_is_truthy(px_eq(_v768, px_str("BitAnd")))) {
        px_srcline(254);
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(255);
    if (px_is_truthy(px_eq(_v768, px_str("BitOr")))) {
        px_srcline(256);
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(257);
    if (px_is_truthy(px_eq(_v768, px_str("BitXor")))) {
        px_srcline(258);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(259);
    if (px_is_truthy(px_eq(_v768, px_str("Shl")))) {
        px_srcline(260);
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(261);
    if (px_is_truthy(px_eq(_v768, px_str("Shr")))) {
        px_srcline(262);
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(263);
    if (px_is_truthy(px_eq(_v768, px_str("ShrU")))) {
        px_srcline(264);
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v769), px_str("\"), ")), _v770), px_str(")"));
    }
    px_srcline(265);
    return _v770;
px_err_771:
    if (px_err_771_proped) return px_err_771_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_local");
    LXValue _v772 = (nargs > 0) ? args[0] : px_null();
    LXValue _v773 = (nargs > 1) ? args[1] : px_null();
    LXValue _v774 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_775_val = px_null();
    int px_err_775_proped = 0;
    px_srcline(268);
    if (px_is_truthy(px_eq(_v772, px_str("Assign")))) {
        px_srcline(269);
        return _v774;
    }
    px_srcline(270);
    if (px_is_truthy(px_eq(_v772, px_str("Plus")))) {
        px_srcline(271);
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(272);
    if (px_is_truthy(px_eq(_v772, px_str("Minus")))) {
        px_srcline(273);
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(274);
    if (px_is_truthy(px_eq(_v772, px_str("Star")))) {
        px_srcline(275);
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(276);
    if (px_is_truthy(px_eq(_v772, px_str("Slash")))) {
        px_srcline(277);
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(278);
    if (px_is_truthy(px_eq(_v772, px_str("IntDiv")))) {
        px_srcline(279);
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(280);
    if (px_is_truthy(px_eq(_v772, px_str("Mod")))) {
        px_srcline(281);
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(282);
    if (px_is_truthy(px_eq(_v772, px_str("Pow")))) {
        px_srcline(283);
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(284);
    if (px_is_truthy(px_eq(_v772, px_str("BitAnd")))) {
        px_srcline(285);
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(286);
    if (px_is_truthy(px_eq(_v772, px_str("BitOr")))) {
        px_srcline(287);
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(288);
    if (px_is_truthy(px_eq(_v772, px_str("BitXor")))) {
        px_srcline(289);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(290);
    if (px_is_truthy(px_eq(_v772, px_str("Shl")))) {
        px_srcline(291);
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(292);
    if (px_is_truthy(px_eq(_v772, px_str("Shr")))) {
        px_srcline(293);
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(294);
    if (px_is_truthy(px_eq(_v772, px_str("ShrU")))) {
        px_srcline(295);
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v773), px_str(", ")), _v774), px_str(")"));
    }
    px_srcline(296);
    return _v774;
px_err_775:
    if (px_err_775_proped) return px_err_775_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_select");
    LXValue _v776 = (nargs > 0) ? args[0] : px_null();
    LXValue _v777 = (nargs > 1) ? args[1] : px_null();
    LXValue _v778 = (nargs > 2) ? args[2] : px_null();
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
    LXValue px_err_800_val = px_null();
    int px_err_800_proped = 0;
    px_srcline(299);
    LXValue _v801 = px_call(px_get_global("cg_pad"), (LXValue[]){_v778}, 1);
    px_srcline(300);
    _v779 = px_call(px_get_global("len"), (LXValue[]){_v776}, 1);
    px_srcline(301);
    if (px_is_truthy(px_eq(_v779, px_int(0LL)))) {
        px_srcline(302);
        return px_str("select 至少需要一个 case 分支");
    }
    px_srcline(303);
    _v780 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    px_srcline(304);
    _v781 = px_str("");
    px_srcline(305);
    _v782 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(306);
    _v783 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(307);
    _v784 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(309);
    _v785 = px_list_n((LXValue[]){}, 0);
    px_srcline(310);
    _v786 = px_int(0LL);
    px_srcline(311);
    while (px_is_truthy(px_lt(_v786, _v779))) {
        px_srcline(312);
        _v787 = px_index(px_index(_v776, _v786), px_int(1LL));
        px_srcline(313);
        if (px_is_truthy(px_eq(px_index(_v787, px_int(0LL)), px_str("Call")))) {
            px_srcline(314);
            _v788 = px_index(_v787, px_int(1LL));
            px_srcline(315);
            if (px_is_truthy(px_eq(px_index(_v788, px_int(0LL)), px_str("Field")))) {
                px_srcline(316);
                _v789 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v788, px_int(2LL))}, 1);
                px_srcline(317);
                if (px_is_truthy(px_eq(_v789, px_str("recv")))) {
                    px_srcline(318);
                    (void)(px_method(_v785, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v788, px_int(1LL))}, 1)}, 1));
                    px_srcline(319);
                     _v786 = px_add(_v786, px_int(1LL));
                    px_srcline(320);
                    continue;
                }
                px_srcline(321);
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v789), px_str("）"));
            }
            px_srcline(322);
            return px_str("select case 仅支持 ch.recv()");
        }
        px_srcline(323);
        return px_str("select case 仅支持 ch.recv()");
    }
    px_srcline(324);
     _v781 = px_add(_v781, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v801, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v779}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v785}, 2)), px_str("};\n")));
    px_srcline(325);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(": {\n")));
    px_srcline(326);
    _v790 = px_list_n((LXValue[]){}, 0);
    px_srcline(327);
    _v791 = px_int(0LL);
    px_srcline(328);
    while (px_is_truthy(px_lt(_v791, _v779))) {
        px_srcline(329);
        (void)(px_method(_v790, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v791}, 1)}, 1));
        px_srcline(330);
         _v791 = px_add(_v791, px_int(1LL));
    }
    px_srcline(331);
     _v781 = px_add(_v781, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v801, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[")), px_call(px_get_global("str"), (LXValue[]){_v779}, 1)), px_str("] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v790}, 2)), px_str("};\n")));
    px_srcline(332);
    if (px_is_truthy(px_gt(_v779, px_int(1LL)))) {
        px_srcline(333);
        _v792 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v801, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = ")), px_call(px_get_global("str"), (LXValue[]){_v779}, 1)), px_str(" - 1; _i")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" > 0; _i")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("--) { "));
        px_srcline(334);
         _v792 = px_add(_v792, px_add(px_add(px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = rand() % (_i")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" + 1); ")));
        px_srcline(335);
         _v792 = px_add(_v792, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[_i")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("] = _ord")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("]; _ord")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[_j")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("] = _t")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("; ")));
        px_srcline(336);
         _v792 = px_add(_v792, px_str("}\n"));
        px_srcline(337);
         _v781 = px_add(_v781, _v792);
    }
    px_srcline(338);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = px_null();\n")));
    px_srcline(339);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = -1;\n")));
    px_srcline(340);
     _v781 = px_add(_v781, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v801, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = 0; _k")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" < ")), px_call(px_get_global("str"), (LXValue[]){_v779}, 1)), px_str("; _k")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("++) {\n")));
    px_srcline(341);
     _v781 = px_add(_v781, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v801, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = _ord")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[_k")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("];\n")));
    px_srcline(342);
     _v781 = px_add(_v781, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v801, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("[_idx")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("], &_rv")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(")) { _picked")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" = _idx")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str("; break; }\n")));
    px_srcline(343);
     _v781 = px_add(_v781, px_add(_v801, px_str("    }\n")));
    px_srcline(345);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" >= 0) {\n")));
    px_srcline(346);
    _v793 = px_int(0LL);
    px_srcline(347);
    while (px_is_truthy(px_lt(_v793, _v779))) {
        px_srcline(348);
        _v794 = px_index(px_index(_v776, _v793), px_int(0LL));
        px_srcline(349);
        _v795 = px_index(px_index(_v776, _v793), px_int(2LL));
        px_srcline(350);
        _v796 = px_add(px_add(px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v793}, 1)), px_str(")"));
        px_srcline(351);
        if (px_is_truthy(px_gt(_v793, px_int(0LL)))) {
            px_srcline(352);
             _v796 = px_add(px_add(px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(" == ")), px_call(px_get_global("str"), (LXValue[]){_v793}, 1)), px_str(")"));
        }
        px_srcline(353);
         _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("        ")), _v796), px_str(" {\n")));
        px_srcline(354);
        if (px_is_truthy(px_ne(_v794, px_null()))) {
            px_srcline(355);
            _v797 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v794}, 1)}, 1);
            px_srcline(356);
             _v781 = px_add(_v781, px_add(px_add(px_add(px_add(px_add(_v801, px_str("            LXValue ")), _v797), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(";\n")));
        }
        px_srcline(357);
        _v798 = px_int(0LL);
        px_srcline(358);
        while (px_is_truthy(px_lt(_v798, px_call(px_get_global("len"), (LXValue[]){_v795}, 1)))) {
            px_srcline(359);
             _v781 = px_add(_v781, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v795, _v798), px_add(_v778, px_int(3LL))}, 2));
            px_srcline(360);
             _v798 = px_add(_v798, px_int(1LL));
        }
        px_srcline(361);
         _v781 = px_add(_v781, px_add(_v801, px_str("        }\n")));
        px_srcline(362);
         _v793 = px_add(_v793, px_int(1LL));
    }
    px_srcline(363);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(";\n")));
    px_srcline(364);
     _v781 = px_add(_v781, px_add(_v801, px_str("    }\n")));
    px_srcline(366);
    if (px_is_truthy(px_ne(_v777, px_null()))) {
        px_srcline(367);
         _v781 = px_add(_v781, px_add(_v801, px_str("    {\n")));
        px_srcline(368);
        _v799 = px_int(0LL);
        px_srcline(369);
        while (px_is_truthy(px_lt(_v799, px_call(px_get_global("len"), (LXValue[]){_v777}, 1)))) {
            px_srcline(370);
             _v781 = px_add(_v781, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v777, _v799), px_add(_v778, px_int(2LL))}, 2));
            px_srcline(371);
             _v799 = px_add(_v799, px_int(1LL));
        }
        px_srcline(372);
         _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(";\n")));
        px_srcline(373);
         _v781 = px_add(_v781, px_add(_v801, px_str("    }\n")));
    }
    px_srcline(375);
     _v781 = px_add(_v781, px_add(_v801, px_str("    px_select_wait();\n")));
    px_srcline(376);
     _v781 = px_add(_v781, px_add(_v801, px_str("}\n")));
    px_srcline(377);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(";\n")));
    px_srcline(378);
     _v781 = px_add(_v781, px_add(px_add(px_add(_v801, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v780}, 1)), px_str(": ;\n")));
    px_srcline(379);
    px_set_global("cg_vars", _v782);
    px_srcline(380);
    px_set_global("cg_var_types", _v783);
    px_srcline(381);
    px_set_global("cg_cells", _v784);
    px_srcline(382);
    return _v781;
px_err_800:
    if (px_err_800_proped) return px_err_800_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_collect");
    LXValue _v802 = (nargs > 0) ? args[0] : px_null();
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
    LXValue px_err_813_val = px_null();
    int px_err_813_proped = 0;
    px_srcline(9);
    _v803 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(10);
    (void)(px_method(_v803, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(11);
    px_index_set(_v803, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_srcline(12);
    px_index_set(_v803, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_srcline(13);
    px_index_set(_v803, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_srcline(14);
    px_index_set(_v803, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_srcline(15);
    px_index_set(_v803, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_srcline(16);
    px_index_set(_v803, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    px_srcline(17);
    _v804 = px_int(0LL);
    px_srcline(18);
    while (px_is_truthy(px_lt(_v804, px_call(px_get_global("len"), (LXValue[]){_v802}, 1)))) {
        px_srcline(19);
        _v805 = px_index(_v802, _v804);
        px_srcline(20);
        _v806 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v805, px_int(2LL))}, 1);
        px_srcline(21);
        (void)(px_method(px_index(_v803, px_str("its")), "append", (LXValue[]){_v806}, 1));
        px_srcline(22);
        (void)(px_method(px_index(_v803, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(23);
        (void)(px_method(px_index(_v803, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(24);
        (void)(px_method(px_index(_v803, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(25);
        _v807 = px_str("");
        px_srcline(26);
        _v808 = px_list_n((LXValue[]){}, 0);
        px_srcline(27);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v805, px_int(1LL))}, 1), px_int(1LL)))) {
            px_srcline(28);
            _v809 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            px_srcline(29);
            _v810 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v805, px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(30);
            _v811 = px_null();
            px_srcline(31);
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v810}, 1))) {
                px_srcline(32);
                 _v811 = px_index(px_get_global("cg_vars"), _v810);
            }
            px_srcline(33);
            px_index_set(px_get_global("cg_vars"), _v810, _v809);
            px_srcline(34);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v810}, 1))) {
                px_srcline(35);
                (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v810}, 1));
            }
            px_srcline(36);
            (void)(px_method(_v808, "append", (LXValue[]){px_list_n((LXValue[]){_v810, _v811}, 2)}, 1));
            px_srcline(37);
             _v807 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v809), px_str(" = ")), px_index(px_index(_v803, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v803, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            px_srcline(39);
            _v812 = px_int(0LL);
            px_srcline(40);
            while (px_is_truthy(px_lt(_v812, px_call(px_get_global("len"), (LXValue[]){px_index(_v805, px_int(1LL))}, 1)))) {
                px_srcline(41);
                _v810 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v805, px_int(1LL)), _v812)}, 1);
                px_srcline(42);
                _v809 = px_add(px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")), px_call(px_get_global("str"), (LXValue[]){_v812}, 1));
                px_srcline(43);
                _v811 = px_null();
                px_srcline(44);
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v810}, 1))) {
                    px_srcline(45);
                     _v811 = px_index(px_get_global("cg_vars"), _v810);
                }
                px_srcline(46);
                px_index_set(px_get_global("cg_vars"), _v810, _v809);
                px_srcline(47);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v810}, 1))) {
                    px_srcline(48);
                    (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v810}, 1));
                }
                px_srcline(49);
                (void)(px_method(_v808, "append", (LXValue[]){px_list_n((LXValue[]){_v810, _v811}, 2)}, 1));
                px_srcline(50);
                 _v807 = px_add(_v807, px_add(px_add(px_add(px_add(px_add(px_add(px_str("LXValue "), _v809), px_str(" = px_index(")), px_index(px_index(_v803, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v803, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v812}, 1)), px_str(")); ")));
                px_srcline(51);
                 _v812 = px_add(_v812, px_int(1LL));
            }
        }
        px_srcline(52);
        (void)(px_method(px_index(_v803, px_str("binds")), "append", (LXValue[]){_v807}, 1));
        px_srcline(53);
        (void)(px_method(px_index(_v803, px_str("saved_all")), "append", (LXValue[]){_v808}, 1));
        px_srcline(54);
         _v804 = px_add(_v804, px_int(1LL));
    }
    px_srcline(55);
    return _v803;
px_err_813:
    if (px_err_813_proped) return px_err_813_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_restore");
    LXValue _v814 = (nargs > 0) ? args[0] : px_null();
    LXValue _v815 = px_null();
    LXValue _v816 = px_null();
    LXValue _v817 = px_null();
    LXValue _v818 = px_null();
    LXValue _v819 = px_null();
    LXValue px_err_820_val = px_null();
    int px_err_820_proped = 0;
    px_srcline(57);
    _v815 = px_int(0LL);
    px_srcline(58);
    while (px_is_truthy(px_lt(_v815, px_call(px_get_global("len"), (LXValue[]){_v814}, 1)))) {
        px_srcline(59);
        _v816 = px_index(_v814, _v815);
        px_srcline(60);
        _v817 = px_int(0LL);
        px_srcline(61);
        while (px_is_truthy(px_lt(_v817, px_call(px_get_global("len"), (LXValue[]){_v816}, 1)))) {
            px_srcline(62);
            _v818 = px_index(px_index(_v816, _v817), px_int(0LL));
            px_srcline(63);
            _v819 = px_index(px_index(_v816, _v817), px_int(1LL));
            px_srcline(64);
            if (px_is_truthy(px_eq(_v819, px_null()))) {
                px_srcline(65);
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v818}, 1));
            }
            else {
                px_srcline(67);
                px_index_set(px_get_global("cg_vars"), _v818, _v819);
            }
            px_srcline(68);
             _v817 = px_add(_v817, px_int(1LL));
        }
        px_srcline(69);
         _v815 = px_add(_v815, px_int(1LL));
    }
px_err_820:
    if (px_err_820_proped) return px_err_820_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_body");
    LXValue _v821 = (nargs > 0) ? args[0] : px_null();
    LXValue _v822 = (nargs > 1) ? args[1] : px_null();
    LXValue _v823 = (nargs > 2) ? args[2] : px_null();
    LXValue _v824 = px_null();
    LXValue _v825 = px_null();
    LXValue _v826 = px_null();
    LXValue _v827 = px_null();
    LXValue px_err_828_val = px_null();
    int px_err_828_proped = 0;
    px_srcline(72);
    _v824 = px_str("");
    px_srcline(73);
    if (px_is_truthy(px_ne(_v822, px_null()))) {
        px_srcline(74);
         _v824 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v822), px_str(")) { ")), _v823), px_str("} "));
    }
    else {
        px_srcline(76);
         _v824 = _v823;
    }
    px_srcline(77);
    _v825 = px_call(px_get_global("len"), (LXValue[]){px_index(_v821, px_str("its"))}, 1);
    px_srcline(78);
    _v826 = px_sub(_v825, px_int(1LL));
    px_srcline(79);
    while (px_is_truthy(px_ge(_v826, px_int(0LL)))) {
        px_srcline(80);
        _v827 = px_str("");
        px_srcline(81);
        if (px_is_truthy(px_lt(px_add(_v826, px_int(1LL)), _v825))) {
            px_srcline(82);
             _v827 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v821, px_str("ivs")), px_add(_v826, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v821, px_str("its")), px_add(_v826, px_int(1LL)))), px_str("; "));
        }
        px_srcline(83);
         _v824 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), px_index(px_index(_v821, px_str("idxs")), _v826)), px_str("=0; ")), px_index(px_index(_v821, px_str("idxs")), _v826)), px_str("<px_len(")), px_index(px_index(_v821, px_str("ivs")), _v826)), px_str("); ")), px_index(px_index(_v821, px_str("idxs")), _v826)), px_str("++) { LXValue ")), px_index(px_index(_v821, px_str("itms")), _v826)), px_str(" = px_index(")), px_index(px_index(_v821, px_str("ivs")), _v826)), px_str(", px_int(")), px_index(px_index(_v821, px_str("idxs")), _v826)), px_str(")); ")), px_index(px_index(_v821, px_str("binds")), _v826)), _v827), _v824), px_str(" } "));
        px_srcline(84);
         _v826 = px_sub(_v826, px_int(1LL));
    }
    px_srcline(85);
    return _v824;
px_err_828:
    if (px_err_828_proped) return px_err_828_val;
    return px_null();
}

static LXValue fn_cg_gen_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_closure");
    LXValue _v829 = (nargs > 0) ? args[0] : px_null();
    LXValue _v830 = (nargs > 1) ? args[1] : px_null();
    LXValue _v831 = (nargs > 2) ? args[2] : px_null();
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
    LXValue px_err_848_val = px_null();
    int px_err_848_proped = 0;
    px_srcline(92);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(93);
    _v832 = px_get_global("cg_closure_id");
    px_srcline(94);
    _v833 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v832}, 1));
    px_srcline(96);
    _v834 = px_list_n((LXValue[]){}, 0);
    px_srcline(97);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v829, px_null(), _v830, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v834}, 2));
    px_srcline(98);
    _v835 = px_list_n((LXValue[]){}, 0);
    px_srcline(99);
    LXValue _t849 = _v834;
    for (int _t850 = 0; _t850 < px_len(_t849); _t850++) {
        _v836 = px_index(_t849, px_int(_t850));
        px_srcline(100);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v836}, 1), px_null()))) {
            px_srcline(101);
            (void)(px_method(_v835, "append", (LXValue[]){_v836}, 1));
        }
    }
    px_srcline(103);
    _v837 = px_str("px_null()");
    px_srcline(104);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v835}, 1), px_int(0LL)))) {
        px_srcline(105);
        _v838 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(106);
        LXValue _t851 = _v835;
        for (int _t852 = 0; _t852 < px_len(_t851); _t852++) {
            _v836 = px_index(_t851, px_int(_t852));
            px_srcline(107);
            _v839 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v836}, 1);
            px_srcline(110);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v836}, 1))) {
                px_srcline(111);
                 _v838 = px_add(_v838, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v836), px_str("\", ")), _v839), px_str("); ")));
            }
            else {
                px_srcline(113);
                 _v838 = px_add(_v838, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v836), px_str("\", px_cell(")), _v839), px_str(")); ")));
            }
        }
        px_srcline(114);
         _v838 = px_add(_v838, px_str("_capenv; })"));
        px_srcline(115);
         _v837 = _v838;
    }
    px_srcline(116);
    _v840 = px_add(px_add(px_str("static LXValue "), _v833), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(117);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v835}, 1), px_int(0LL)))) {
        px_srcline(118);
         _v840 = px_add(_v840, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(120);
         _v840 = px_add(_v840, px_str("    (void)ctx;\n"));
    }
    px_srcline(121);
    _v841 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(122);
    _v842 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(123);
    _v843 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(124);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(125);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(126);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(127);
    _v844 = px_int(0LL);
    px_srcline(128);
    while (px_is_truthy(px_lt(_v844, px_call(px_get_global("len"), (LXValue[]){_v829}, 1)))) {
        px_srcline(129);
        _v845 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v829, _v844), px_int(1LL))}, 1)}, 1);
        px_srcline(130);
         _v840 = px_add(_v840, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v845), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v844}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v844}, 1)), px_str("] : px_null();\n")));
        px_srcline(131);
         _v844 = px_add(_v844, px_int(1LL));
    }
    px_srcline(133);
    LXValue _t853 = _v835;
    for (int _t854 = 0; _t854 < px_len(_t853); _t854++) {
        _v836 = px_index(_t853, px_int(_t854));
        px_srcline(134);
        _v846 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v836}, 1);
        px_srcline(135);
        px_index_set(px_get_global("cg_cells"), _v836, px_int(1LL));
        px_srcline(136);
         _v840 = px_add(_v840, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v846), px_str(" = px_env_lookup(ctx, \"")), _v836), px_str("\");\n")));
    }
    px_srcline(137);
    _v847 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v830}, 1);
    px_srcline(138);
     _v840 = px_add(_v840, px_add(px_add(px_str("    return "), _v847), px_str(";\n")));
    px_srcline(139);
     _v840 = px_add(_v840, px_str("}\n"));
    px_srcline(140);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v840));
    px_srcline(141);
    px_set_global("cg_vars", _v841);
    px_srcline(142);
    px_set_global("cg_var_types", _v842);
    px_srcline(143);
    px_set_global("cg_cells", _v843);
    px_srcline(147);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v835}, 1), px_int(0LL)))) {
        px_srcline(148);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\""), _v831), px_str("\", ")), _v833), px_str(", ")), _v837), px_str(")"));
    }
    px_srcline(149);
    return px_add(px_add(px_add(px_add(px_str("px_func(\""), _v831), px_str("\", ")), _v833), px_str(", NULL)"));
px_err_848:
    if (px_err_848_proped) return px_err_848_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_expr");
    LXValue _v855 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v897 = px_null();
    LXValue _v898 = px_null();
    LXValue _v899 = px_null();
    LXValue _v900 = px_null();
    LXValue _v901 = px_null();
    LXValue _v902 = px_null();
    LXValue _v903 = px_null();
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
    LXValue px_err_915_val = px_null();
    int px_err_915_proped = 0;
    px_srcline(152);
    _v856 = px_index(_v855, px_int(0LL));
    px_srcline(153);
    if (px_is_truthy(px_eq(_v856, px_str("Int")))) {
        px_srcline(154);
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1)), px_str("LL)"));
    }
    px_srcline(155);
    if (px_is_truthy(px_eq(_v856, px_str("Float")))) {
        px_srcline(156);
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1)), px_str(")"));
    }
    px_srcline(157);
    if (px_is_truthy(px_eq(_v856, px_str("Str")))) {
        px_srcline(158);
        _v857 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(162);
        if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){_v857}, 1))) {
            px_srcline(163);
            return px_add(px_add(px_str("PX_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v857}, 1)), px_str("\")"));
        }
        px_srcline(164);
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v857}, 1)), px_str("\")"));
    }
    px_srcline(165);
    if (px_is_truthy(px_eq(_v856, px_str("Bool")))) {
        px_srcline(166);
        if (px_is_truthy(px_index(_v855, px_int(1LL)))) {
            px_srcline(167);
            return px_str("px_bool(true)");
        }
        px_srcline(168);
        return px_str("px_bool(false)");
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v856, px_str("Null")))) {
        px_srcline(170);
        return px_str("px_null()");
    }
    px_srcline(171);
    if (px_is_truthy(px_eq(_v856, px_str("List")))) {
        px_srcline(172);
        _v858 = px_list_n((LXValue[]){}, 0);
        px_srcline(173);
        _v859 = px_index(_v855, px_int(1LL));
        px_srcline(174);
        _v860 = px_int(0LL);
        px_srcline(175);
        while (px_is_truthy(px_lt(_v860, px_call(px_get_global("len"), (LXValue[]){_v859}, 1)))) {
            px_srcline(176);
            (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v859, _v860)}, 1)}, 1));
            px_srcline(177);
             _v860 = px_add(_v860, px_int(1LL));
        }
        px_srcline(178);
        return px_add(px_add(px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")"));
    }
    px_srcline(179);
    if (px_is_truthy(px_eq(_v856, px_str("Tuple")))) {
        px_srcline(180);
        _v858 = px_list_n((LXValue[]){}, 0);
        px_srcline(181);
        _v859 = px_index(_v855, px_int(1LL));
        px_srcline(182);
        _v860 = px_int(0LL);
        px_srcline(183);
        while (px_is_truthy(px_lt(_v860, px_call(px_get_global("len"), (LXValue[]){_v859}, 1)))) {
            px_srcline(184);
            (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v859, _v860)}, 1)}, 1));
            px_srcline(185);
             _v860 = px_add(_v860, px_int(1LL));
        }
        px_srcline(186);
        return px_add(px_add(px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")"));
    }
    px_srcline(187);
    if (px_is_truthy(px_eq(_v856, px_str("Dict")))) {
        px_srcline(188);
        _v861 = px_str("({ LXValue _d = px_dict(); ");
        px_srcline(189);
        _v862 = px_index(_v855, px_int(1LL));
        px_srcline(190);
        _v860 = px_int(0LL);
        px_srcline(191);
        while (px_is_truthy(px_lt(_v860, px_call(px_get_global("len"), (LXValue[]){_v862}, 1)))) {
            px_srcline(192);
            _v863 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v862, _v860), px_int(0LL))}, 1);
            px_srcline(193);
            _v864 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v862, _v860), px_int(1LL))}, 1);
            px_srcline(194);
             _v861 = px_add(_v861, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v863), px_str("; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ")), _v864), px_str("); } ")));
            px_srcline(195);
             _v860 = px_add(_v860, px_int(1LL));
        }
        px_srcline(196);
         _v861 = px_add(_v861, px_str("_d; })"));
        px_srcline(197);
        return _v861;
    }
    px_srcline(198);
    if (px_is_truthy(px_eq(_v856, px_str("Var")))) {
        px_srcline(199);
        _v865 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(200);
        _v866 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v865}, 1);
        px_srcline(201);
        if (px_is_truthy(px_ne(_v866, px_null()))) {
            px_srcline(203);
            return px_call(px_get_global("cg_load_of"), (LXValue[]){_v865, _v866}, 2);
        }
        px_srcline(204);
        return px_add(px_add(px_str("px_get_global(\""), _v865), px_str("\")"));
    }
    px_srcline(205);
    if (px_is_truthy(px_eq(_v856, px_str("Field")))) {
        px_srcline(206);
        _v867 = px_index(_v855, px_int(1LL));
        px_srcline(207);
        _v868 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(209);
        if (px_is_truthy(px_eq(px_index(_v867, px_int(0LL)), px_str("Var")))) {
            px_srcline(210);
            _v869 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v867, px_int(1LL))}, 1);
            px_srcline(211);
            if (px_is_truthy(({ LXValue _t916 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v869}, 1); px_is_truthy(_t916) ? px_method(px_index(px_get_global("cg_const_enums"), _v869), "has", (LXValue[]){_v868}, 1) : _t916; }))) {
                px_srcline(212);
                return px_index(px_index(px_get_global("cg_const_enums"), _v869), _v868);
            }
        }
        px_srcline(214);
        if (px_is_truthy(px_eq(px_index(_v867, px_int(0LL)), px_str("Var")))) {
            px_srcline(215);
            _v869 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v867, px_int(1LL))}, 1);
            px_srcline(216);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v869}, 1))) {
                px_srcline(217);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v869), px_str("\", \"")), _v868), px_str("\")"));
            }
        }
        px_srcline(218);
        _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v867}, 1);
        px_srcline(219);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v870), px_str(", \"")), _v868), px_str("\")"));
    }
    px_srcline(220);
    if (px_is_truthy(px_eq(_v856, px_str("OptionalField")))) {
        px_srcline(221);
        _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(222);
        _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(223);
        _v868 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(224);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v870), px_str("; px_is_null(")), _v871), px_str(") ? px_null() : px_field(")), _v870), px_str(", \"")), _v868), px_str("\"); })"));
    }
    px_srcline(225);
    if (px_is_truthy(px_eq(_v856, px_str("Index")))) {
        px_srcline(226);
        _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(227);
        _v860 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(228);
        return px_add(px_add(px_add(px_add(px_str("px_index("), _v870), px_str(", ")), _v860), px_str(")"));
    }
    px_srcline(229);
    if (px_is_truthy(px_eq(_v856, px_str("Slice")))) {
        px_srcline(230);
        _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(231);
        _v861 = px_str("px_null()");
        px_srcline(232);
        if (px_is_truthy(px_ne(px_index(_v855, px_int(2LL)), px_null()))) {
            px_srcline(233);
             _v861 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        }
        px_srcline(234);
        _v872 = px_str("px_null()");
        px_srcline(235);
        if (px_is_truthy(px_ne(px_index(_v855, px_int(3LL)), px_null()))) {
            px_srcline(236);
             _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(3LL))}, 1);
        }
        px_srcline(237);
        _v873 = px_str("px_null()");
        px_srcline(238);
        if (px_is_truthy(px_ne(px_index(_v855, px_int(4LL)), px_null()))) {
            px_srcline(239);
             _v873 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(4LL))}, 1);
        }
        px_srcline(240);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), _v870), px_str(", ")), _v861), px_str(", ")), _v872), px_str(", ")), _v873), px_str(")"));
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(_v856, px_str("Call")))) {
        px_srcline(242);
        _v874 = px_index(_v855, px_int(1LL));
        px_srcline(243);
        _v875 = px_index(_v855, px_int(2LL));
        px_srcline(244);
        if (px_is_truthy(px_eq(px_index(_v874, px_int(0LL)), px_str("Var")))) {
            px_srcline(245);
            _v876 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v874, px_int(1LL))}, 1);
            px_srcline(248);
            (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v874, _v875}, 2));
            px_srcline(249);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v876}, 1))) {
                px_srcline(250);
                _v877 = px_index(px_get_global("cg_ffi"), _v876);
                px_srcline(251);
                _v858 = px_list_n((LXValue[]){}, 0);
                px_srcline(252);
                _v878 = px_int(0LL);
                px_srcline(253);
                while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                    px_srcline(254);
                    (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
                    px_srcline(255);
                     _v878 = px_add(_v878, px_int(1LL));
                }
                px_srcline(256);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v876), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")}, 2)"));
            }
            px_srcline(257);
            if (px_is_truthy(px_eq(_v876, px_str("chan")))) {
                px_srcline(258);
                _v879 = px_str("0");
                px_srcline(259);
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v875}, 1), px_int(0LL)))) {
                    px_srcline(260);
                    if (px_is_truthy(px_eq(px_index(px_index(_v875, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                        px_srcline(261);
                         _v879 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v875, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                        px_srcline(263);
                         _v879 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                px_srcline(264);
                return px_add(px_add(px_str("px_chan_create("), _v879), px_str(")"));
            }
            px_srcline(265);
            if (px_is_truthy(px_eq(_v876, px_str("mutex")))) {
                px_srcline(266);
                return px_str("px_mutex_create()");
            }
            px_srcline(267);
            if (px_is_truthy(px_eq(_v876, px_str("rwlock")))) {
                px_srcline(268);
                return px_str("px_rwlock_create()");
            }
            px_srcline(270);
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v876}, 1))) {
                px_srcline(271);
                _v880 = px_index(px_get_global("cg_structs"), _v876);
                px_srcline(272);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v880}, 1), px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                    px_srcline(273);
                    return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v876), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v880}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v875}, 1)}, 1));
                }
                px_srcline(274);
                _v858 = px_list_n((LXValue[]){}, 0);
                px_srcline(275);
                _v878 = px_int(0LL);
                px_srcline(276);
                while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                    px_srcline(277);
                    (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
                    px_srcline(278);
                     _v878 = px_add(_v878, px_int(1LL));
                }
                px_srcline(279);
                _v881 = px_list_n((LXValue[]){}, 0);
                px_srcline(280);
                _v882 = px_int(0LL);
                px_srcline(281);
                while (px_is_truthy(px_lt(_v882, px_call(px_get_global("len"), (LXValue[]){_v880}, 1)))) {
                    px_srcline(282);
                    (void)(px_method(_v881, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v880, _v882)), px_str("\""))}, 1));
                    px_srcline(283);
                     _v882 = px_add(_v882, px_int(1LL));
                }
                px_srcline(284);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v876), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v881}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v880}, 1)}, 1)), px_str(")"));
            }
            px_srcline(285);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v876}, 1))) {
                px_srcline(286);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v875}, 1), px_int(1LL)))) {
                    px_srcline(287);
                    return px_add(px_add(px_str("枚举 "), _v876), px_str(" 构造需要一个变体名"));
                }
                px_srcline(288);
                _v866 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, px_int(0LL))}, 1);
                px_srcline(289);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v876), px_str("\", (")), _v866), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        px_srcline(291);
        if (px_is_truthy(px_eq(px_index(_v874, px_int(0LL)), px_str("Field")))) {
            px_srcline(292);
            _v867 = px_index(_v874, px_int(1LL));
            px_srcline(293);
            _v883 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v874, px_int(2LL))}, 1);
            px_srcline(295);
            _v884 = px_null();
            px_srcline(296);
            if (px_is_truthy(px_eq(px_index(_v867, px_int(0LL)), px_str("Var")))) {
                px_srcline(297);
                _v869 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v867, px_int(1LL))}, 1);
                px_srcline(298);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v869}, 1))) {
                    px_srcline(299);
                     _v884 = px_index(px_get_global("cg_var_types"), _v869);
                }
            }
            px_srcline(300);
            if (px_is_truthy(({ LXValue _t917 = px_ne(_v884, px_null()); px_is_truthy(_t917) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v884}, 1) : _t917; }))) {
                px_srcline(301);
                _v885 = px_index(px_get_global("cg_impls"), _v884);
                px_srcline(302);
                _v886 = px_bool(false);
                px_srcline(303);
                _v887 = px_int(0LL);
                px_srcline(304);
                while (px_is_truthy(px_lt(_v887, px_call(px_get_global("len"), (LXValue[]){_v885}, 1)))) {
                    px_srcline(305);
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v885, _v887), px_int(1LL))}, 1), _v883))) {
                        px_srcline(306);
                         _v886 = px_bool(true);
                        px_srcline(307);
                        break;
                    }
                    px_srcline(308);
                     _v887 = px_add(_v887, px_int(1LL));
                }
                px_srcline(309);
                if (px_is_truthy(_v886)) {
                    px_srcline(310);
                    _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v867}, 1);
                    px_srcline(311);
                    _v858 = px_list_n((LXValue[]){_v870}, 1);
                    px_srcline(312);
                    _v878 = px_int(0LL);
                    px_srcline(313);
                    while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                        px_srcline(314);
                        (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
                        px_srcline(315);
                         _v878 = px_add(_v878, px_int(1LL));
                    }
                    px_srcline(316);
                    _v888 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v884}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v883}, 1));
                    px_srcline(317);
                    return px_add(px_add(px_add(px_add(px_add(_v888, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(", NULL)"));
                }
            }
            px_srcline(319);
            _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v867}, 1);
            px_srcline(320);
            _v858 = px_list_n((LXValue[]){}, 0);
            px_srcline(321);
            _v878 = px_int(0LL);
            px_srcline(322);
            while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                px_srcline(323);
                (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
                px_srcline(324);
                 _v878 = px_add(_v878, px_int(1LL));
            }
            px_srcline(325);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), _v870), px_str(", \"")), _v883), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")"));
        }
        px_srcline(327);
        _v889 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v874}, 1);
        px_srcline(328);
        _v858 = px_list_n((LXValue[]){}, 0);
        px_srcline(329);
        _v878 = px_int(0LL);
        px_srcline(330);
        while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
            px_srcline(331);
            (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
            px_srcline(332);
             _v878 = px_add(_v878, px_int(1LL));
        }
        px_srcline(333);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v889), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")"));
    }
    px_srcline(334);
    if (px_is_truthy(px_eq(_v856, px_str("Unary")))) {
        px_srcline(335);
        _v870 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(336);
        _v890 = px_index(_v855, px_int(1LL));
        px_srcline(337);
        if (px_is_truthy(px_eq(_v890, px_str("Neg")))) {
            px_srcline(338);
            return px_add(px_add(px_str("px_neg("), _v870), px_str(")"));
        }
        px_srcline(339);
        if (px_is_truthy(px_eq(_v890, px_str("Not")))) {
            px_srcline(340);
            return px_add(px_add(px_str("px_not("), _v870), px_str(")"));
        }
        px_srcline(341);
        return px_add(px_add(px_str("px_bitnot("), _v870), px_str(")"));
    }
    px_srcline(342);
    if (px_is_truthy(px_eq(_v856, px_str("Binary")))) {
        px_srcline(343);
        _v891 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(344);
        _v892 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(3LL))}, 1);
        px_srcline(345);
        _v890 = px_index(_v855, px_int(1LL));
        px_srcline(346);
        if (px_is_truthy(px_eq(_v890, px_str("And")))) {
            px_srcline(347);
            _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(348);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v891), px_str("; px_is_truthy(")), _v871), px_str(") ? ")), _v892), px_str(" : ")), _v871), px_str("; })"));
        }
        px_srcline(349);
        if (px_is_truthy(px_eq(_v890, px_str("Or")))) {
            px_srcline(350);
            _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(351);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v891), px_str("; px_is_truthy(")), _v871), px_str(") ? ")), _v871), px_str(" : ")), _v892), px_str("; })"));
        }
        px_srcline(352);
        _v893 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v890}, 1);
        px_srcline(353);
        return px_add(px_add(px_add(px_add(px_add(_v893, px_str("(")), _v891), px_str(", ")), _v892), px_str(")"));
    }
    px_srcline(354);
    if (px_is_truthy(px_eq(_v856, px_str("Pipe")))) {
        px_srcline(355);
        _v866 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(356);
        _v894 = px_index(_v855, px_int(2LL));
        px_srcline(357);
        if (px_is_truthy(px_eq(px_index(_v894, px_int(0LL)), px_str("Call")))) {
            px_srcline(358);
            _v889 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v894, px_int(1LL))}, 1);
            px_srcline(359);
            _v858 = px_list_n((LXValue[]){_v866}, 1);
            px_srcline(360);
            _v878 = px_int(0LL);
            px_srcline(361);
            while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){px_index(_v894, px_int(2LL))}, 1)))) {
                px_srcline(362);
                (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v894, px_int(2LL)), _v878)}, 1)}, 1));
                px_srcline(363);
                 _v878 = px_add(_v878, px_int(1LL));
            }
            px_srcline(364);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call("), _v889), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")"));
        }
        px_srcline(365);
        _v893 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v894}, 1);
        px_srcline(366);
        return px_add(px_add(px_add(px_add(px_str("px_call("), _v893), px_str(", (LXValue[]){")), _v866), px_str("}, 1)"));
    }
    px_srcline(367);
    if (px_is_truthy(px_eq(_v856, px_str("NullCoalesce")))) {
        px_srcline(368);
        _v891 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(369);
        _v892 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(370);
        _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(371);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v891), px_str("; px_is_null(")), _v871), px_str(") ? ")), _v892), px_str(" : ")), _v871), px_str("; })"));
    }
    px_srcline(372);
    if (px_is_truthy(px_eq(_v856, px_str("Try")))) {
        px_srcline(373);
        _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(374);
        _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(375);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            px_srcline(376);
            _v895 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            px_srcline(377);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v872), px_str("; if (px_is_result(")), _v871), px_str(")) { if (!px_result_ok(")), _v871), px_str(")) { ")), _v895), px_str("_val = ")), _v871), px_str("; ")), _v895), px_str("_proped = 1; goto ")), _v895), px_str("; } ")), _v871), px_str(" = px_result_unwrap(")), _v871), px_str("); } else if (px_is_null(")), _v871), px_str(")) { ")), _v895), px_str("_val = px_null(); ")), _v895), px_str("_proped = 1; goto ")), _v895), px_str("; } ")), _v871), px_str("; })"));
        }
        px_srcline(378);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v872), px_str("; if (px_is_result(")), _v871), px_str(") && !px_result_ok(")), _v871), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v871), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v871), px_str(")) ")), _v871), px_str(" = px_result_unwrap(")), _v871), px_str("); ")), _v871), px_str("; })"));
    }
    px_srcline(379);
    if (px_is_truthy(px_eq(_v856, px_str("ForceUnwrap")))) {
        px_srcline(380);
        _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(381);
        _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(382);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v872), px_str("; if (px_is_result(")), _v871), px_str(")) { if (!px_result_ok(")), _v871), px_str(")) px_error(\"force unwrap Err\"); ")), _v871), px_str(" = px_result_unwrap(")), _v871), px_str("); } if (px_is_null(")), _v871), px_str(")) px_error(\"force unwrap null\"); ")), _v871), px_str("; })"));
    }
    px_srcline(383);
    if (px_is_truthy(px_eq(_v856, px_str("IfExpr")))) {
        px_srcline(384);
        _v889 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(385);
        _v896 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(386);
        _v897 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(3LL))}, 1);
        px_srcline(387);
        _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(388);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str("; if (px_is_truthy(")), _v889), px_str(")) { ")), _v871), px_str(" = ")), _v896), px_str("; } else { ")), _v871), px_str(" = ")), _v897), px_str("; } ")), _v871), px_str("; })"));
    }
    px_srcline(389);
    if (px_is_truthy(px_eq(_v856, px_str("ListComp")))) {
        px_srcline(390);
        _v898 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(391);
        _v892 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(392);
        _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(393);
        _v899 = px_null();
        px_srcline(394);
        if (px_is_truthy(px_ne(px_index(_v855, px_int(3LL)), px_null()))) {
            px_srcline(395);
             _v899 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(3LL))}, 1);
        }
        px_srcline(396);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v892, px_str("saved_all"))}, 1));
        px_srcline(397);
        _v900 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v898), px_str(", ")), _v872), px_str("); "));
        px_srcline(398);
        _v901 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v892, _v899, _v900}, 3);
        px_srcline(399);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v898), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v892, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v892, px_str("its")), px_int(0LL))), px_str("; ")), _v901), px_str(" ")), _v898), px_str("; })"));
    }
    px_srcline(400);
    if (px_is_truthy(px_eq(_v856, px_str("GenExp")))) {
        px_srcline(401);
        _v902 = px_index(_v855, px_int(2LL));
        px_srcline(402);
        if (px_is_truthy(({ LXValue _t918 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v902}, 1), px_int(1LL)); px_is_truthy(_t918) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v902, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t918; }))) {
            px_srcline(403);
            _v903 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v902, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(404);
            _v904 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v902, px_int(0LL)), px_int(2LL))}, 1);
            px_srcline(405);
            _v905 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v903}, 1), px_index(_v855, px_int(1LL))}, 2);
            px_srcline(406);
            _v906 = px_str("px_null()");
            px_srcline(407);
            if (px_is_truthy(px_ne(px_index(_v855, px_int(3LL)), px_null()))) {
                px_srcline(408);
                 _v906 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v903}, 1), px_index(_v855, px_int(3LL))}, 2);
            }
            px_srcline(409);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v904), px_str(", ")), _v905), px_str(", ")), _v906), px_str(")"));
        }
        px_srcline(411);
        _v898 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(412);
        _v892 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v902}, 1);
        px_srcline(413);
        _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(414);
        _v899 = px_null();
        px_srcline(415);
        if (px_is_truthy(px_ne(px_index(_v855, px_int(3LL)), px_null()))) {
            px_srcline(416);
             _v899 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(3LL))}, 1);
        }
        px_srcline(417);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v892, px_str("saved_all"))}, 1));
        px_srcline(418);
        _v900 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v898), px_str(", ")), _v872), px_str("); "));
        px_srcline(419);
        _v901 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v892, _v899, _v900}, 3);
        px_srcline(420);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v898), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v892, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v892, px_str("its")), px_int(0LL))), px_str("; ")), _v901), px_str(" px_gen_from_list(")), _v898), px_str("); })"));
    }
    px_srcline(421);
    if (px_is_truthy(px_eq(_v856, px_str("DictComp")))) {
        px_srcline(422);
        _v898 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(423);
        _v892 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v855, px_int(3LL))}, 1);
        px_srcline(424);
        _v863 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(425);
        _v864 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(2LL))}, 1);
        px_srcline(426);
        _v899 = px_null();
        px_srcline(427);
        if (px_is_truthy(px_ne(px_index(_v855, px_int(4LL)), px_null()))) {
            px_srcline(428);
             _v899 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(4LL))}, 1);
        }
        px_srcline(429);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v892, px_str("saved_all"))}, 1));
        px_srcline(430);
        _v900 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v863), px_str("; LXValue _v = ")), _v864), px_str("; if (_k.type == PX_STR) px_dict_set(")), _v898), px_str(", _k.as.obj->as.str.data, _v); } "));
        px_srcline(431);
        _v901 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v892, _v899, _v900}, 3);
        px_srcline(432);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v898), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v892, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v892, px_str("its")), px_int(0LL))), px_str("; ")), _v901), px_str(" ")), _v898), px_str("; })"));
    }
    px_srcline(433);
    if (px_is_truthy(px_eq(_v856, px_str("Closure")))) {
        px_srcline(436);
        return px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v855, px_int(1LL)), px_index(_v855, px_int(3LL)), px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_add(px_get_global("cg_closure_id"), px_int(1LL))}, 1)), px_str(">"))}, 3);
    }
    px_srcline(437);
    if (px_is_truthy(px_eq(_v856, px_str("Block")))) {
        px_srcline(438);
        _v861 = px_str("({ ");
        px_srcline(439);
         _v861 = px_add(_v861, px_str("LXValue _blk = px_null(); "));
        px_srcline(440);
        _v907 = px_index(_v855, px_int(1LL));
        px_srcline(441);
        _v908 = px_int(0LL);
        px_srcline(442);
        while (px_is_truthy(px_lt(_v908, px_call(px_get_global("len"), (LXValue[]){_v907}, 1)))) {
            px_srcline(443);
            _v909 = px_index(_v907, _v908);
            px_srcline(444);
            if (px_is_truthy(px_eq(px_index(_v909, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(445);
                _v872 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v909, px_int(1LL))}, 1);
                px_srcline(446);
                 _v861 = px_add(_v861, px_add(px_add(px_str("_blk = "), _v872), px_str("; ")));
            }
            else {
                px_srcline(448);
                 _v861 = px_add(_v861, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v909, px_int(0LL)}, 2));
            }
            px_srcline(449);
             _v908 = px_add(_v908, px_int(1LL));
        }
        px_srcline(450);
         _v861 = px_add(_v861, px_str("_blk; })"));
        px_srcline(451);
        return _v861;
    }
    px_srcline(452);
    if (px_is_truthy(px_eq(_v856, px_str("Match")))) {
        px_srcline(453);
        _v910 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(454);
        _v871 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(455);
        _v861 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v871), px_str(" = ")), _v910), px_str("; "));
        px_srcline(456);
        _v911 = px_index(_v855, px_int(2LL));
        px_srcline(457);
        _v912 = px_bool(true);
        px_srcline(458);
        _v878 = px_int(0LL);
        px_srcline(459);
        while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v911}, 1)))) {
            px_srcline(460);
            _v899 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v911, _v878), px_int(1LL)), _v871}, 2);
            px_srcline(461);
            _v913 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v911, _v878), px_int(3LL))}, 1);
            px_srcline(462);
            _v914 = px_str("if");
            px_srcline(463);
            if (px_is_truthy(px_not(_v912))) {
                px_srcline(464);
                 _v914 = px_str("else if");
            }
            px_srcline(465);
             _v861 = px_add(_v861, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v914, px_str(" (")), _v899), px_str(") { ")), _v871), px_str(" = ")), _v913), px_str("; } ")));
            px_srcline(466);
             _v912 = px_bool(false);
            px_srcline(467);
             _v878 = px_add(_v878, px_int(1LL));
        }
        px_srcline(468);
         _v861 = px_add(_v861, px_add(_v871, px_str("; })")));
        px_srcline(469);
        return _v861;
    }
    px_srcline(470);
    if (px_is_truthy(px_eq(_v856, px_str("Constructor")))) {
        px_srcline(471);
        _v865 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v855, px_int(1LL))}, 1);
        px_srcline(472);
        _v875 = px_index(_v855, px_int(2LL));
        px_srcline(473);
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v865}, 1))) {
            px_srcline(474);
            _v880 = px_index(px_get_global("cg_structs"), _v865);
            px_srcline(475);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v880}, 1), px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                px_srcline(476);
                return px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v865), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v880}, 1)}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v875}, 1)}, 1));
            }
            px_srcline(477);
            _v858 = px_list_n((LXValue[]){}, 0);
            px_srcline(478);
            _v878 = px_int(0LL);
            px_srcline(479);
            while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
                px_srcline(480);
                (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
                px_srcline(481);
                 _v878 = px_add(_v878, px_int(1LL));
            }
            px_srcline(482);
            _v881 = px_list_n((LXValue[]){}, 0);
            px_srcline(483);
            _v882 = px_int(0LL);
            px_srcline(484);
            while (px_is_truthy(px_lt(_v882, px_call(px_get_global("len"), (LXValue[]){_v880}, 1)))) {
                px_srcline(485);
                (void)(px_method(_v881, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v880, _v882)), px_str("\""))}, 1));
                px_srcline(486);
                 _v882 = px_add(_v882, px_int(1LL));
            }
            px_srcline(487);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v865), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v881}, 2)), px_str("}, (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v880}, 1)}, 1)), px_str(")"));
        }
        px_srcline(488);
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v865}, 1))) {
            px_srcline(489);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v875}, 1), px_int(1LL)))) {
                px_srcline(490);
                return px_add(px_add(px_str("枚举 "), _v865), px_str(" 构造需要一个变体名"));
            }
            px_srcline(491);
            _v866 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, px_int(0LL))}, 1);
            px_srcline(492);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v865), px_str("\", (")), _v866), px_str(").as.obj->as.enum_inst.variant)"));
        }
        px_srcline(493);
        _v858 = px_list_n((LXValue[]){}, 0);
        px_srcline(494);
        _v878 = px_int(0LL);
        px_srcline(495);
        while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v875}, 1)))) {
            px_srcline(496);
            (void)(px_method(_v858, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v875, _v878)}, 1)}, 1));
            px_srcline(497);
             _v878 = px_add(_v878, px_int(1LL));
        }
        px_srcline(498);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v865), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v858}, 2)), px_str("}, ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v858}, 1)}, 1)), px_str(")"));
    }
    px_srcline(499);
    return px_str("px_null()");
px_err_915:
    if (px_err_915_proped) return px_err_915_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_binop_cname");
    LXValue _v919 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_920_val = px_null();
    int px_err_920_proped = 0;
    px_srcline(502);
    if (px_is_truthy(px_eq(_v919, px_str("Add")))) {
        px_srcline(503);
        return px_str("px_add");
    }
    px_srcline(504);
    if (px_is_truthy(px_eq(_v919, px_str("Sub")))) {
        px_srcline(505);
        return px_str("px_sub");
    }
    px_srcline(506);
    if (px_is_truthy(px_eq(_v919, px_str("Mul")))) {
        px_srcline(507);
        return px_str("px_mul");
    }
    px_srcline(508);
    if (px_is_truthy(px_eq(_v919, px_str("Div")))) {
        px_srcline(509);
        return px_str("px_div");
    }
    px_srcline(510);
    if (px_is_truthy(px_eq(_v919, px_str("IntDiv")))) {
        px_srcline(511);
        return px_str("px_idiv");
    }
    px_srcline(512);
    if (px_is_truthy(px_eq(_v919, px_str("Mod")))) {
        px_srcline(513);
        return px_str("px_mod");
    }
    px_srcline(514);
    if (px_is_truthy(px_eq(_v919, px_str("Pow")))) {
        px_srcline(515);
        return px_str("px_pow");
    }
    px_srcline(516);
    if (px_is_truthy(px_eq(_v919, px_str("Eq")))) {
        px_srcline(517);
        return px_str("px_eq");
    }
    px_srcline(518);
    if (px_is_truthy(px_eq(_v919, px_str("Ne")))) {
        px_srcline(519);
        return px_str("px_ne");
    }
    px_srcline(520);
    if (px_is_truthy(px_eq(_v919, px_str("Lt")))) {
        px_srcline(521);
        return px_str("px_lt");
    }
    px_srcline(522);
    if (px_is_truthy(px_eq(_v919, px_str("Le")))) {
        px_srcline(523);
        return px_str("px_le");
    }
    px_srcline(524);
    if (px_is_truthy(px_eq(_v919, px_str("Gt")))) {
        px_srcline(525);
        return px_str("px_gt");
    }
    px_srcline(526);
    if (px_is_truthy(px_eq(_v919, px_str("Ge")))) {
        px_srcline(527);
        return px_str("px_ge");
    }
    px_srcline(528);
    if (px_is_truthy(px_eq(_v919, px_str("BitAnd")))) {
        px_srcline(529);
        return px_str("px_bitand");
    }
    px_srcline(530);
    if (px_is_truthy(px_eq(_v919, px_str("BitOr")))) {
        px_srcline(531);
        return px_str("px_bitor");
    }
    px_srcline(532);
    if (px_is_truthy(px_eq(_v919, px_str("BitXor")))) {
        px_srcline(533);
        return px_str("px_bitxor");
    }
    px_srcline(534);
    if (px_is_truthy(px_eq(_v919, px_str("Shl")))) {
        px_srcline(535);
        return px_str("px_shl");
    }
    px_srcline(536);
    if (px_is_truthy(px_eq(_v919, px_str("Shr")))) {
        px_srcline(537);
        return px_str("px_shr");
    }
    px_srcline(538);
    if (px_is_truthy(px_eq(_v919, px_str("ShrU")))) {
        px_srcline(539);
        return px_str("px_ushr");
    }
    px_srcline(540);
    return px_str("px_add");
px_err_920:
    if (px_err_920_proped) return px_err_920_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_pattern_cond");
    LXValue _v921 = (nargs > 0) ? args[0] : px_null();
    LXValue _v922 = (nargs > 1) ? args[1] : px_null();
    LXValue _v923 = px_null();
    LXValue _v924 = px_null();
    LXValue _v925 = px_null();
    LXValue _v926 = px_null();
    LXValue px_err_927_val = px_null();
    int px_err_927_proped = 0;
    px_srcline(543);
    _v923 = px_index(_v921, px_int(0LL));
    px_srcline(544);
    if (px_is_truthy(px_eq(_v923, px_str("PatLiteral")))) {
        px_srcline(545);
        _v924 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v921, px_int(1LL))}, 1);
        px_srcline(546);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v922), px_str(", ")), _v924), px_str("))"));
    }
    px_srcline(547);
    if (px_is_truthy(px_eq(_v923, px_str("PatBinding")))) {
        px_srcline(548);
        _v925 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v921, px_int(1LL))}, 1);
        px_srcline(549);
        if (px_is_truthy(({ LXValue _t929 = ({ LXValue _t928 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v925}, 1), px_int(0LL)); px_is_truthy(_t928) ? px_ge(px_index(_v925, px_int(0LL)), px_str("A")) : _t928; }); px_is_truthy(_t929) ? px_le(px_index(_v925, px_int(0LL)), px_str("Z")) : _t929; }))) {
            px_srcline(550);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v922), px_str(".type == PX_ENUM && strcmp(")), _v922), px_str(".as.obj->as.enum_inst.variant, \"")), _v925), px_str("\") == 0)"));
        }
        px_srcline(551);
        return px_str("true");
    }
    px_srcline(552);
    if (px_is_truthy(px_eq(_v923, px_str("PatWildcard")))) {
        px_srcline(553);
        return px_str("true");
    }
    px_srcline(554);
    if (px_is_truthy(px_eq(_v923, px_str("PatTuple")))) {
        px_srcline(555);
        _v926 = px_index(_v921, px_int(1LL));
        px_srcline(556);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v926}, 1), px_int(0LL)))) {
            px_srcline(557);
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v926, px_int(0LL)), _v922}, 2);
        }
        px_srcline(558);
        return px_str("true");
    }
    px_srcline(559);
    if (px_is_truthy(px_eq(_v923, px_str("PatConstructor")))) {
        px_srcline(560);
        _v925 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v921, px_int(1LL))}, 1);
        px_srcline(561);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v922), px_str(".type == PX_ENUM && strcmp(")), _v922), px_str(".as.obj->as.enum_inst.variant, \"")), _v925), px_str("\") == 0)"));
    }
    px_srcline(562);
    return px_str("true");
px_err_927:
    if (px_err_927_proped) return px_err_927_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_lambda");
    LXValue _v930 = (nargs > 0) ? args[0] : px_null();
    LXValue _v931 = (nargs > 1) ? args[1] : px_null();
    LXValue _v932 = px_null();
    LXValue _v933 = px_null();
    LXValue _v934 = px_null();
    LXValue _v935 = px_null();
    LXValue _v936 = px_null();
    LXValue _v937 = px_null();
    LXValue _v938 = px_null();
    LXValue _v939 = px_null();
    LXValue _v940 = px_null();
    LXValue _v941 = px_null();
    LXValue _v942 = px_null();
    LXValue _v943 = px_null();
    LXValue _v944 = px_null();
    LXValue _v945 = px_null();
    LXValue _v946 = px_null();
    LXValue _v947 = px_null();
    LXValue _v948 = px_null();
    LXValue px_err_949_val = px_null();
    int px_err_949_proped = 0;
    px_srcline(565);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(566);
    _v932 = px_get_global("cg_closure_id");
    px_srcline(567);
    _v933 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v932}, 1));
    px_srcline(569);
    _v934 = px_list_n((LXValue[]){}, 0);
    px_srcline(570);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v931, _v934}, 2));
    px_srcline(571);
    _v935 = px_list_n((LXValue[]){}, 0);
    px_srcline(572);
    LXValue _t950 = _v934;
    for (int _t951 = 0; _t951 < px_len(_t950); _t951++) {
        _v936 = px_index(_t950, px_int(_t951));
        px_srcline(573);
        if (px_is_truthy(({ LXValue _t952 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v930, _v936}, 2)); px_is_truthy(_t952) ? px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v936}, 1), px_null()) : _t952; }))) {
            px_srcline(574);
            (void)(px_method(_v935, "append", (LXValue[]){_v936}, 1));
        }
    }
    px_srcline(575);
    _v937 = px_str("px_null()");
    px_srcline(576);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v935}, 1), px_int(0LL)))) {
        px_srcline(577);
        _v938 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(578);
        LXValue _t953 = _v935;
        for (int _t954 = 0; _t954 < px_len(_t953); _t954++) {
            _v936 = px_index(_t953, px_int(_t954));
            px_srcline(579);
            _v939 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v936}, 1);
            px_srcline(583);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v936}, 1))) {
                px_srcline(584);
                 _v938 = px_add(_v938, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v936), px_str("\", px_cell(px_cell_get(")), _v939), px_str("))); ")));
            }
            else {
                px_srcline(586);
                 _v938 = px_add(_v938, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v936), px_str("\", px_cell(")), _v939), px_str(")); ")));
            }
        }
        px_srcline(587);
         _v938 = px_add(_v938, px_str("_capenv; })"));
        px_srcline(588);
         _v937 = _v938;
    }
    px_srcline(589);
    _v940 = px_add(px_add(px_str("static LXValue "), _v933), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(590);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v935}, 1), px_int(0LL)))) {
        px_srcline(591);
         _v940 = px_add(_v940, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(593);
         _v940 = px_add(_v940, px_str("    (void)ctx;\n"));
    }
    px_srcline(594);
    _v941 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(595);
    _v942 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(596);
    _v943 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(597);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(598);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(599);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(600);
    _v944 = px_int(0LL);
    px_srcline(601);
    while (px_is_truthy(px_lt(_v944, px_call(px_get_global("len"), (LXValue[]){_v930}, 1)))) {
        px_srcline(602);
        _v945 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v930, _v944)}, 1);
        px_srcline(603);
         _v940 = px_add(_v940, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    LXValue "), _v945), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v944}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v944}, 1)), px_str("] : px_null();\n")));
        px_srcline(604);
         _v944 = px_add(_v944, px_int(1LL));
    }
    px_srcline(605);
    LXValue _t955 = _v935;
    for (int _t956 = 0; _t956 < px_len(_t955); _t956++) {
        _v936 = px_index(_t955, px_int(_t956));
        px_srcline(606);
        _v946 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v936}, 1);
        px_srcline(607);
        px_index_set(px_get_global("cg_cells"), _v936, px_int(1LL));
        px_srcline(608);
         _v940 = px_add(_v940, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v946), px_str(" = px_env_lookup(ctx, \"")), _v936), px_str("\");\n")));
    }
    px_srcline(609);
    _v947 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v931}, 1);
    px_srcline(610);
    _v948 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(611);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v948}, 1));
    px_srcline(612);
     _v940 = px_add(_v940, px_add(px_add(px_str("    LXValue "), _v948), px_str("_val = px_null();\n")));
    px_srcline(613);
     _v940 = px_add(_v940, px_add(px_add(px_str("    int "), _v948), px_str("_proped = 0;\n")));
    px_srcline(614);
     _v940 = px_add(_v940, px_add(px_add(px_str("    return "), _v947), px_str(";\n")));
    px_srcline(615);
     _v940 = px_add(_v940, px_add(_v948, px_str(":\n")));
    px_srcline(616);
     _v940 = px_add(_v940, px_add(px_add(px_add(px_add(px_str("    if ("), _v948), px_str("_proped) return ")), _v948), px_str("_val;\n")));
    px_srcline(617);
     _v940 = px_add(_v940, px_str("    return px_null();\n"));
    px_srcline(618);
     _v940 = px_add(_v940, px_str("}\n"));
    px_srcline(619);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(620);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v940));
    px_srcline(621);
    px_set_global("cg_vars", _v941);
    px_srcline(622);
    px_set_global("cg_var_types", _v942);
    px_srcline(623);
    px_set_global("cg_cells", _v943);
    px_srcline(625);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v935}, 1), px_int(0LL)))) {
        px_srcline(626);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v932}, 1)), px_str(">\", ")), _v933), px_str(", ")), _v937), px_str(")"));
    }
    px_srcline(627);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v932}, 1)), px_str(">\", ")), _v933), px_str(", NULL)"));
px_err_949:
    if (px_err_949_proped) return px_err_949_val;
    return px_null();
}

static LXValue fn_cgm_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_perr");
    LXValue _v957 = (nargs > 0) ? args[0] : px_null();
    LXValue _v958 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_959_val = px_null();
    int px_err_959_proped = 0;
    px_srcline(20);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v957), px_str(": ")), _v958)}, 1));
    px_srcline(21);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_959:
    if (px_err_959_proped) return px_err_959_val;
    return px_null();
}

static LXValue fn_cgm_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_pwarn");
    LXValue _v960 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_961_val = px_null();
    int px_err_961_proped = 0;
    px_srcline(23);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v960)}, 1));
px_err_961:
    if (px_err_961_proped) return px_err_961_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dirname");
    LXValue _v962 = (nargs > 0) ? args[0] : px_null();
    LXValue _v963 = px_null();
    LXValue px_err_964_val = px_null();
    int px_err_964_proped = 0;
    px_srcline(26);
    _v963 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v962}, 1), px_int(1LL));
    px_srcline(27);
    while (px_is_truthy(px_ge(_v963, px_int(0LL)))) {
        px_srcline(28);
        if (px_is_truthy(px_eq(px_index(_v962, _v963), px_str("/")))) {
            px_srcline(29);
            if (px_is_truthy(px_eq(_v963, px_int(0LL)))) {
                px_srcline(30);
                return px_str("/");
            }
            px_srcline(31);
            return px_slice(_v962, px_int(0LL), _v963, px_null());
        }
        px_srcline(32);
         _v963 = px_sub(_v963, px_int(1LL));
    }
    px_srcline(33);
    return px_str(".");
px_err_964:
    if (px_err_964_proped) return px_err_964_val;
    return px_null();
}

static LXValue fn_cg_norm_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_norm_path");
    LXValue _v965 = (nargs > 0) ? args[0] : px_null();
    LXValue _v966 = px_null();
    LXValue _v967 = px_null();
    LXValue _v968 = px_null();
    LXValue _v969 = px_null();
    LXValue _v970 = px_null();
    LXValue _v971 = px_null();
    LXValue px_err_972_val = px_null();
    int px_err_972_proped = 0;
    px_srcline(37);
    _v966 = ({ LXValue _t973 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v965}, 1), px_int(0LL)); px_is_truthy(_t973) ? px_eq(px_index(_v965, px_int(0LL)), px_str("/")) : _t973; });
    px_srcline(38);
    _v967 = px_list_n((LXValue[]){}, 0);
    px_srcline(39);
    _v968 = px_str("");
    px_srcline(40);
    _v969 = px_int(0LL);
    px_srcline(41);
    while (px_is_truthy(px_le(_v969, px_call(px_get_global("len"), (LXValue[]){_v965}, 1)))) {
        px_srcline(42);
        if (px_is_truthy(({ LXValue _t974 = px_eq(_v969, px_call(px_get_global("len"), (LXValue[]){_v965}, 1)); px_is_truthy(_t974) ? _t974 : px_eq(px_index(_v965, _v969), px_str("/")); }))) {
            px_srcline(43);
            if (px_is_truthy(px_eq(_v968, px_str("..")))) {
                px_srcline(44);
                if (px_is_truthy(({ LXValue _t975 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v967}, 1), px_int(0LL)); px_is_truthy(_t975) ? px_ne(px_index(_v967, px_sub(px_call(px_get_global("len"), (LXValue[]){_v967}, 1), px_int(1LL))), px_str("..")) : _t975; }))) {
                    px_srcline(45);
                    (void)(px_method(_v967, "pop", (LXValue[]){}, 0));
                }
                else if (px_is_truthy(px_not(_v966))) {
                    px_srcline(47);
                    (void)(px_method(_v967, "append", (LXValue[]){px_str("..")}, 1));
                }
            }
            else if (px_is_truthy(({ LXValue _t976 = px_ne(_v968, px_str("")); px_is_truthy(_t976) ? px_ne(_v968, px_str(".")) : _t976; }))) {
                px_srcline(49);
                (void)(px_method(_v967, "append", (LXValue[]){_v968}, 1));
            }
            px_srcline(50);
             _v968 = px_str("");
        }
        else {
            px_srcline(52);
             _v968 = px_add(_v968, px_index(_v965, _v969));
        }
        px_srcline(53);
         _v969 = px_add(_v969, px_int(1LL));
    }
    px_srcline(54);
    _v970 = px_str("");
    px_srcline(55);
    _v971 = px_int(0LL);
    px_srcline(56);
    while (px_is_truthy(px_lt(_v971, px_call(px_get_global("len"), (LXValue[]){_v967}, 1)))) {
        px_srcline(57);
         _v970 = px_add(_v970, px_add(px_str("/"), px_index(_v967, _v971)));
        px_srcline(58);
         _v971 = px_add(_v971, px_int(1LL));
    }
    px_srcline(59);
    if (px_is_truthy(_v966)) {
        px_srcline(60);
        if (px_is_truthy(px_eq(_v970, px_str("")))) {
            px_srcline(61);
            return px_str("/");
        }
        px_srcline(62);
        return _v970;
    }
    px_srcline(63);
    if (px_is_truthy(px_eq(_v970, px_str("")))) {
        px_srcline(64);
        return px_str(".");
    }
    px_srcline(65);
    return px_slice(_v970, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v970}, 1), px_null());
px_err_972:
    if (px_err_972_proped) return px_err_972_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_stdlib_dir");
    LXValue _v977 = px_null();
    LXValue _v978 = px_null();
    LXValue _v979 = px_null();
    LXValue _v980 = px_null();
    LXValue px_err_981_val = px_null();
    int px_err_981_proped = 0;
    px_srcline(68);
    _v977 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    px_srcline(69);
    if (px_is_truthy(({ LXValue _t982 = px_ne(_v977, px_null()); px_is_truthy(_t982) ? px_call(px_get_global("exists"), (LXValue[]){_v977}, 1) : _t982; }))) {
        px_srcline(70);
        return _v977;
    }
    px_srcline(71);
    _v978 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    px_srcline(72);
    _v979 = px_int(0LL);
    px_srcline(73);
    while (px_is_truthy(px_lt(_v979, px_call(px_get_global("len"), (LXValue[]){_v978}, 1)))) {
        px_srcline(74);
        _v980 = px_index(_v978, _v979);
        px_srcline(75);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v980}, 1))) {
            px_srcline(76);
            return _v980;
        }
        px_srcline(77);
         _v979 = px_add(_v979, px_int(1LL));
    }
    px_srcline(78);
    return px_null();
px_err_981:
    if (px_err_981_proped) return px_err_981_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find_module_path");
    LXValue _v983 = (nargs > 0) ? args[0] : px_null();
    LXValue _v984 = (nargs > 1) ? args[1] : px_null();
    LXValue _v985 = px_null();
    LXValue _v986 = px_null();
    LXValue _v987 = px_null();
    LXValue _v988 = px_null();
    LXValue _v989 = px_null();
    LXValue _v990 = px_null();
    LXValue _v991 = px_null();
    LXValue _v992 = px_null();
    LXValue _v993 = px_null();
    LXValue _v994 = px_null();
    LXValue _v995 = px_null();
    LXValue _v996 = px_null();
    LXValue _v997 = px_null();
    LXValue _v998 = px_null();
    LXValue _v999 = px_null();
    LXValue px_err_1000_val = px_null();
    int px_err_1000_proped = 0;
    px_srcline(81);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v983}, 1), px_int(0LL)))) {
        px_srcline(82);
        return px_null();
    }
    px_srcline(84);
    if (px_is_truthy(({ LXValue _t1002 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v983}, 1), px_int(1LL)); px_is_truthy(_t1002) ? ({ LXValue _t1001 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v983, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1001) ? _t1001 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v983, px_int(0LL)), px_str(".px")}, 2); }) : _t1002; }))) {
        px_srcline(85);
        _v985 = px_index(_v983, px_int(0LL));
        px_srcline(86);
        _v986 = _v985;
        px_srcline(87);
        if (px_is_truthy(px_not(({ LXValue _t1003 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v985}, 1), px_int(0LL)); px_is_truthy(_t1003) ? px_eq(px_index(_v985, px_int(0LL)), px_str("/")) : _t1003; })))) {
            px_srcline(88);
             _v986 = px_add(px_add(_v984, px_str("/")), _v985);
        }
        px_srcline(89);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v986}, 1))) {
            px_srcline(90);
            return _v986;
        }
        px_srcline(91);
        return px_null();
    }
    px_srcline(93);
    if (px_is_truthy(px_eq(px_index(_v983, px_int(0LL)), px_str("std")))) {
        px_srcline(94);
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v983}, 1), px_int(2LL)))) {
            px_srcline(95);
            return px_null();
        }
        px_srcline(96);
        _v987 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(97);
        if (px_is_truthy(px_eq(_v987, px_null()))) {
            px_srcline(98);
            return px_null();
        }
        px_srcline(99);
        _v986 = _v987;
        px_srcline(100);
        _v988 = px_int(1LL);
        px_srcline(101);
        while (px_is_truthy(px_lt(_v988, px_call(px_get_global("len"), (LXValue[]){_v983}, 1)))) {
            px_srcline(102);
             _v986 = px_add(_v986, px_add(px_str("/"), px_index(_v983, _v988)));
            px_srcline(103);
             _v988 = px_add(_v988, px_int(1LL));
        }
        px_srcline(104);
        _v989 = px_add(_v986, px_str(".px"));
        px_srcline(105);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v989}, 1))) {
            px_srcline(106);
            return _v989;
        }
        px_srcline(107);
        _v990 = px_add(_v986, px_str("/mod.px"));
        px_srcline(108);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v990}, 1))) {
            px_srcline(109);
            return _v990;
        }
        px_srcline(110);
        return px_null();
    }
    px_srcline(112);
    _v991 = px_list_n((LXValue[]){_v984}, 1);
    px_srcline(113);
    _v992 = px_add(_v984, px_str("/.px_modules"));
    px_srcline(114);
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v992}, 1))) {
        px_srcline(115);
        (void)(px_method(_v991, "append", (LXValue[]){_v992}, 1));
        px_srcline(116);
        _v993 = px_call(px_get_global("list_dir"), (LXValue[]){_v992}, 1);
        px_srcline(117);
        _v994 = px_int(0LL);
        px_srcline(118);
        while (px_is_truthy(px_lt(_v994, px_call(px_get_global("len"), (LXValue[]){_v993}, 1)))) {
            px_srcline(119);
            _v995 = px_index(_v993, _v994);
            px_srcline(120);
            _v996 = px_add(px_add(_v992, px_str("/")), _v995);
            px_srcline(121);
            if (px_is_truthy(({ LXValue _t1004 = px_call(px_get_global("exists"), (LXValue[]){_v996}, 1); px_is_truthy(_t1004) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v995, px_str(".")}, 2)) : _t1004; }))) {
                px_srcline(122);
                (void)(px_method(_v991, "append", (LXValue[]){_v996}, 1));
            }
            px_srcline(123);
             _v994 = px_add(_v994, px_int(1LL));
        }
    }
    px_srcline(124);
    _v997 = px_int(0LL);
    px_srcline(125);
    while (px_is_truthy(px_lt(_v997, px_call(px_get_global("len"), (LXValue[]){_v991}, 1)))) {
        px_srcline(126);
        _v998 = px_index(_v991, _v997);
        px_srcline(127);
        _v986 = _v998;
        px_srcline(128);
        _v988 = px_int(0LL);
        px_srcline(129);
        while (px_is_truthy(px_lt(_v988, px_call(px_get_global("len"), (LXValue[]){_v983}, 1)))) {
            px_srcline(130);
             _v986 = px_add(_v986, px_add(px_str("/"), px_index(_v983, _v988)));
            px_srcline(131);
             _v988 = px_add(_v988, px_int(1LL));
        }
        px_srcline(132);
        _v989 = px_add(_v986, px_str(".px"));
        px_srcline(133);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v989}, 1))) {
            px_srcline(134);
            return _v989;
        }
        px_srcline(135);
        _v990 = px_add(_v986, px_str("/mod.px"));
        px_srcline(136);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v990}, 1))) {
            px_srcline(137);
            return _v990;
        }
        px_srcline(138);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v983}, 1), px_int(1LL)))) {
            px_srcline(139);
            _v999 = px_add(px_add(px_add(_v998, px_str("/")), px_index(_v983, px_int(0LL))), px_str(".px"));
            px_srcline(140);
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v999}, 1))) {
                px_srcline(141);
                return _v999;
            }
        }
        px_srcline(142);
         _v997 = px_add(_v997, px_int(1LL));
    }
    px_srcline(143);
    return px_null();
px_err_1000:
    if (px_err_1000_proped) return px_err_1000_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_definition");
    LXValue _v1005 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1006 = px_null();
    LXValue px_err_1007_val = px_null();
    int px_err_1007_proped = 0;
    px_srcline(146);
    _v1006 = px_index(_v1005, px_int(0LL));
    px_srcline(147);
    if (px_is_truthy(px_eq(_v1006, px_str("FuncDef")))) {
        px_srcline(149);
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1005, px_int(1LL))}, 1), px_str("main")))) {
            px_srcline(150);
            return px_bool(false);
        }
        px_srcline(151);
        return px_bool(true);
    }
    px_srcline(152);
    if (px_is_truthy(px_eq(_v1006, px_str("ExternDef")))) {
        px_srcline(153);
        return px_bool(true);
    }
    px_srcline(154);
    if (px_is_truthy(({ LXValue _t1010 = ({ LXValue _t1009 = ({ LXValue _t1008 = px_eq(_v1006, px_str("StructDef")); px_is_truthy(_t1008) ? _t1008 : px_eq(_v1006, px_str("EnumDef")); }); px_is_truthy(_t1009) ? _t1009 : px_eq(_v1006, px_str("TraitDef")); }); px_is_truthy(_t1010) ? _t1010 : px_eq(_v1006, px_str("ImplDef")); }))) {
        px_srcline(155);
        return px_bool(true);
    }
    px_srcline(156);
    if (px_is_truthy(px_eq(_v1006, px_str("VarDecl")))) {
        px_srcline(160);
        return px_bool(true);
    }
    px_srcline(161);
    return px_bool(false);
px_err_1007:
    if (px_err_1007_proped) return px_err_1007_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_def_name");
    LXValue _v1011 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1012 = px_null();
    LXValue _v1013 = px_null();
    LXValue _v1014 = px_null();
    LXValue _v1015 = px_null();
    LXValue px_err_1016_val = px_null();
    int px_err_1016_proped = 0;
    px_srcline(164);
    _v1012 = px_index(_v1011, px_int(0LL));
    px_srcline(165);
    if (px_is_truthy(({ LXValue _t1020 = ({ LXValue _t1019 = ({ LXValue _t1018 = ({ LXValue _t1017 = px_eq(_v1012, px_str("FuncDef")); px_is_truthy(_t1017) ? _t1017 : px_eq(_v1012, px_str("StructDef")); }); px_is_truthy(_t1018) ? _t1018 : px_eq(_v1012, px_str("EnumDef")); }); px_is_truthy(_t1019) ? _t1019 : px_eq(_v1012, px_str("TraitDef")); }); px_is_truthy(_t1020) ? _t1020 : px_eq(_v1012, px_str("ExternDef")); }))) {
        px_srcline(166);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1011, px_int(1LL))}, 1);
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v1012, px_str("VarDecl")))) {
        px_srcline(168);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1011, px_int(2LL))}, 1);
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v1012, px_str("ImplDef")))) {
        px_srcline(170);
        _v1013 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1011, px_int(1LL))}, 1);
        px_srcline(171);
        _v1014 = px_index(_v1011, px_int(2LL));
        px_srcline(172);
        _v1015 = px_str("None");
        px_srcline(173);
        if (px_is_truthy(px_ne(_v1014, px_null()))) {
            px_srcline(174);
             _v1015 = px_add(px_add(px_str("Some("), _v1014), px_str(")"));
        }
        px_srcline(175);
        return px_add(px_add(px_add(px_str("impl::"), _v1013), px_str("::")), _v1015);
    }
    px_srcline(176);
    return px_null();
px_err_1016:
    if (px_err_1016_proped) return px_err_1016_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_module");
    LXValue _v1021 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1022 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1023 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1024 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1025 = (nargs > 4) ? args[4] : px_null();
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
    LXValue _v1037 = px_null();
    LXValue _v1038 = px_null();
    LXValue _v1039 = px_null();
    LXValue _v1040 = px_null();
    LXValue _v1041 = px_null();
    LXValue _v1042 = px_null();
    LXValue _v1043 = px_null();
    LXValue _v1044 = px_null();
    LXValue _v1045 = px_null();
    LXValue px_err_1046_val = px_null();
    int px_err_1046_proped = 0;
    px_srcline(180);
    _v1026 = px_list_n((LXValue[]){}, 0);
    px_srcline(181);
    _v1027 = px_int(0LL);
    px_srcline(182);
    while (px_is_truthy(px_lt(_v1027, px_call(px_get_global("len"), (LXValue[]){_v1021}, 1)))) {
        px_srcline(183);
        (void)(px_method(_v1026, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1021, _v1027)}, 1)}, 1));
        px_srcline(184);
         _v1027 = px_add(_v1027, px_int(1LL));
    }
    px_srcline(185);
     _v1021 = _v1026;
    px_srcline(186);
    _v1028 = px_list_n((LXValue[]){}, 0);
    px_srcline(187);
    _v1029 = px_int(0LL);
    px_srcline(188);
    while (px_is_truthy(px_lt(_v1029, px_call(px_get_global("len"), (LXValue[]){_v1022}, 1)))) {
        px_srcline(189);
        (void)(px_method(_v1028, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1022, _v1029)}, 1)}, 1));
        px_srcline(190);
         _v1029 = px_add(_v1029, px_int(1LL));
    }
    px_srcline(191);
     _v1022 = _v1028;
    px_srcline(192);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1021}, 1), px_int(0LL)))) {
        px_srcline(193);
        return px_null();
    }
    px_srcline(195);
    if (px_is_truthy(({ LXValue _t1049 = ({ LXValue _t1048 = ({ LXValue _t1047 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1021}, 1), px_int(1LL)); px_is_truthy(_t1047) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1021, px_int(0LL))}, 1), px_int(2LL)) : _t1047; }); px_is_truthy(_t1048) ? px_eq(px_slice(px_index(_v1021, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t1048; }); px_is_truthy(_t1049) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v1021, px_int(0LL)), px_str(".px")}, 2)) : _t1049; }))) {
        px_srcline(196);
        return px_null();
    }
    px_srcline(197);
    _v1030 = px_eq(px_index(_v1021, px_int(0LL)), px_str("std"));
    px_srcline(198);
    _v1031 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1021}, 2);
    px_srcline(199);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1031}, 1))) {
        px_srcline(200);
        return px_null();
    }
    px_srcline(201);
    _v1032 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1021, _v1023}, 2);
    px_srcline(202);
    if (px_is_truthy(px_eq(_v1032, px_null()))) {
        px_srcline(209);
        _v1033 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(210);
        _v1034 = px_str("");
        px_srcline(211);
        if (px_is_truthy(({ LXValue _t1050 = px_eq(px_index(_v1021, px_int(0LL)), px_str("std")); px_is_truthy(_t1050) ? px_eq(_v1033, px_null()) : _t1050; }))) {
            px_srcline(212);
             _v1034 = px_str("；stdlib 未找到（设 PX_STDLIB 或放置 stdlib/）");
        }
        px_srcline(213);
        if (px_is_truthy(px_ne(px_call(px_get_global("env"), (LXValue[]){px_str("PX_STRICT_MODULE")}, 1), px_null()))) {
            px_srcline(214);
            (void)(px_call(px_get_global("cgm_perr"), (LXValue[]){px_str("E3005"), px_add(px_add(px_add(px_str("找不到模块 '"), _v1031), px_str("'")), _v1034)}, 2));
        }
        px_srcline(215);
        (void)(px_call(px_get_global("cgm_pwarn"), (LXValue[]){px_add(px_add(px_add(px_str("找不到模块 '"), _v1031), px_str("'（已跳过 → 运行期将报未定义）")), _v1034)}, 1));
        px_srcline(216);
        return px_null();
    }
    px_srcline(220);
    _v1035 = px_add(px_str("#"), px_call(px_get_global("cg_norm_path"), (LXValue[]){_v1032}, 1));
    px_srcline(221);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1035}, 1))) {
        px_srcline(222);
        return px_null();
    }
    px_srcline(223);
    px_index_set(px_get_global("loaded"), _v1031, _v1032);
    px_srcline(224);
    px_index_set(px_get_global("loaded"), _v1035, _v1032);
    px_srcline(225);
    _v1036 = px_call(px_get_global("read_file"), (LXValue[]){_v1032}, 1);
    px_srcline(226);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1036}, 1));
    px_srcline(227);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(228);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(229);
    _v1037 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(231);
    _v1038 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1032}, 1);
    px_srcline(232);
    _v1039 = px_list_n((LXValue[]){}, 0);
    px_srcline(233);
    _v1040 = px_int(0LL);
    px_srcline(234);
    while (px_is_truthy(px_lt(_v1040, px_call(px_get_global("len"), (LXValue[]){px_index(_v1037, px_int(1LL))}, 1)))) {
        px_srcline(235);
        _v1041 = px_index(px_index(_v1037, px_int(1LL)), _v1040);
        px_srcline(236);
        if (px_is_truthy(px_eq(px_index(_v1041, px_int(0LL)), px_str("Import")))) {
            px_srcline(237);
            (void)(px_method(_v1039, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1041, px_int(1LL)), px_index(_v1041, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(238);
         _v1040 = px_add(_v1040, px_int(1LL));
    }
    px_srcline(239);
    _v1042 = px_int(0LL);
    px_srcline(240);
    while (px_is_truthy(px_lt(_v1042, px_call(px_get_global("len"), (LXValue[]){_v1039}, 1)))) {
        px_srcline(241);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1039, _v1042), px_int(0LL)), px_index(px_index(_v1039, _v1042), px_int(1LL)), _v1038, _v1024, _v1025}, 5));
        px_srcline(242);
         _v1042 = px_add(_v1042, px_int(1LL));
    }
    px_srcline(244);
    _v1043 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1022}, 1), px_int(0LL));
    px_srcline(245);
    _v1044 = px_int(0LL);
    px_srcline(246);
    while (px_is_truthy(px_lt(_v1044, px_call(px_get_global("len"), (LXValue[]){px_index(_v1037, px_int(1LL))}, 1)))) {
        px_srcline(247);
        _v1041 = px_index(px_index(_v1037, px_int(1LL)), _v1044);
        px_srcline(248);
        if (px_is_truthy(px_eq(px_index(_v1041, px_int(0LL)), px_str("Import")))) {
            px_srcline(249);
             _v1044 = px_add(_v1044, px_int(1LL));
            px_srcline(250);
            continue;
        }
        px_srcline(251);
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1041}, 1)))) {
            px_srcline(252);
             _v1044 = px_add(_v1044, px_int(1LL));
            px_srcline(253);
            continue;
        }
        px_srcline(254);
        _v1045 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1041}, 1);
        px_srcline(255);
        if (px_is_truthy(px_eq(_v1045, px_null()))) {
            px_srcline(256);
            (void)(px_method(_v1024, "append", (LXValue[]){_v1041}, 1));
        }
        else {
            px_srcline(258);
            if (px_is_truthy(_v1043)) {
                px_srcline(259);
                if (px_is_truthy(({ LXValue _t1051 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1045}, 1), px_int(5LL)); px_is_truthy(_t1051) ? px_eq(px_slice(_v1045, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1051; }))) {
                    px_srcline(260);
                     _v1044 = px_add(_v1044, px_int(1LL));
                    px_srcline(261);
                    continue;
                }
                px_srcline(262);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1022, _v1045}, 2)))) {
                    px_srcline(263);
                     _v1044 = px_add(_v1044, px_int(1LL));
                    px_srcline(264);
                    continue;
                }
            }
            px_srcline(265);
            if (px_is_truthy(_v1030)) {
                px_srcline(266);
                if (px_is_truthy(px_method(_v1025, "has", (LXValue[]){_v1045}, 1))) {
                    px_srcline(267);
                     _v1044 = px_add(_v1044, px_int(1LL));
                    px_srcline(268);
                    continue;
                }
                px_srcline(269);
                px_index_set(_v1025, _v1045, px_bool(true));
            }
            px_srcline(270);
            (void)(px_method(_v1024, "append", (LXValue[]){_v1041}, 1));
        }
        px_srcline(271);
         _v1044 = px_add(_v1044, px_int(1LL));
    }
px_err_1046:
    if (px_err_1046_proped) return px_err_1046_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_resolve_modules");
    LXValue _v1052 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1053 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1054 = px_null();
    LXValue _v1055 = px_null();
    LXValue _v1056 = px_null();
    LXValue _v1057 = px_null();
    LXValue _v1058 = px_null();
    LXValue _v1059 = px_null();
    LXValue _v1060 = px_null();
    LXValue _v1061 = px_null();
    LXValue _v1062 = px_null();
    LXValue _v1063 = px_null();
    LXValue px_err_1064_val = px_null();
    int px_err_1064_proped = 0;
    px_srcline(274);
    _v1054 = px_index(_v1052, px_int(1LL));
    px_srcline(275);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1054}, 1), px_int(0LL)))) {
        px_srcline(276);
        return _v1052;
    }
    px_srcline(277);
    _v1055 = px_list_n((LXValue[]){}, 0);
    px_srcline(278);
    _v1056 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(279);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(280);
    _v1057 = px_list_n((LXValue[]){}, 0);
    px_srcline(281);
    _v1058 = px_int(0LL);
    px_srcline(282);
    while (px_is_truthy(px_lt(_v1058, px_call(px_get_global("len"), (LXValue[]){_v1054}, 1)))) {
        px_srcline(283);
        _v1059 = px_index(_v1054, _v1058);
        px_srcline(284);
        if (px_is_truthy(px_eq(px_index(_v1059, px_int(0LL)), px_str("Import")))) {
            px_srcline(285);
            (void)(px_method(_v1057, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1059, px_int(1LL)), px_index(_v1059, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(286);
         _v1058 = px_add(_v1058, px_int(1LL));
    }
    px_srcline(287);
    _v1060 = px_int(0LL);
    px_srcline(288);
    while (px_is_truthy(px_lt(_v1060, px_call(px_get_global("len"), (LXValue[]){_v1057}, 1)))) {
        px_srcline(289);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1057, _v1060), px_int(0LL)), px_index(px_index(_v1057, _v1060), px_int(1LL)), _v1053, _v1055, _v1056}, 5));
        px_srcline(290);
         _v1060 = px_add(_v1060, px_int(1LL));
    }
    px_srcline(291);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1055}, 1), px_int(0LL)))) {
        px_srcline(292);
        return _v1052;
    }
    px_srcline(293);
    _v1061 = px_list_n((LXValue[]){}, 0);
    px_srcline(294);
    _v1062 = px_int(0LL);
    px_srcline(295);
    while (px_is_truthy(px_lt(_v1062, px_call(px_get_global("len"), (LXValue[]){_v1055}, 1)))) {
        px_srcline(296);
        (void)(px_method(_v1061, "append", (LXValue[]){px_index(_v1055, _v1062)}, 1));
        px_srcline(297);
         _v1062 = px_add(_v1062, px_int(1LL));
    }
    px_srcline(298);
    _v1063 = px_int(0LL);
    px_srcline(299);
    while (px_is_truthy(px_lt(_v1063, px_call(px_get_global("len"), (LXValue[]){_v1054}, 1)))) {
        px_srcline(300);
        (void)(px_method(_v1061, "append", (LXValue[]){px_index(_v1054, _v1063)}, 1));
        px_srcline(301);
         _v1063 = px_add(_v1063, px_int(1LL));
    }
    px_srcline(302);
    return px_list_n((LXValue[]){px_str("Program"), _v1061}, 2);
px_err_1064:
    if (px_err_1064_proped) return px_err_1064_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_dict");
    LXValue _v1065 = px_null();
    LXValue px_err_1066_val = px_null();
    int px_err_1066_proped = 0;
    px_srcline(57);
    _v1065 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(58);
    (void)(px_method(_v1065, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(59);
    return _v1065;
px_err_1066:
    if (px_err_1066_proped) return px_err_1066_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dict_copy");
    LXValue _v1067 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1068 = px_null();
    LXValue _v1069 = px_null();
    LXValue _v1070 = px_null();
    LXValue px_err_1071_val = px_null();
    int px_err_1071_proped = 0;
    px_srcline(61);
    _v1068 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(62);
    _v1069 = px_method(_v1067, "keys", (LXValue[]){}, 0);
    px_srcline(63);
    _v1070 = px_int(0LL);
    px_srcline(64);
    while (px_is_truthy(px_lt(_v1070, px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)))) {
        px_srcline(65);
        px_index_set(_v1068, px_index(_v1069, _v1070), px_index(_v1067, px_index(_v1069, _v1070)));
        px_srcline(66);
         _v1070 = px_add(_v1070, px_int(1LL));
    }
    px_srcline(67);
    return _v1068;
px_err_1071:
    if (px_err_1071_proped) return px_err_1071_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_uid");
    LXValue px_err_1072_val = px_null();
    int px_err_1072_proped = 0;
    px_srcline(69);
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    px_srcline(70);
    return px_get_global("cg_uidc");
px_err_1072:
    if (px_err_1072_proped) return px_err_1072_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_tmp");
    LXValue px_err_1073_val = px_null();
    int px_err_1073_proped = 0;
    px_srcline(72);
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1073:
    if (px_err_1073_proped) return px_err_1073_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_var");
    LXValue _v1074 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1075 = px_null();
    LXValue px_err_1076_val = px_null();
    int px_err_1076_proped = 0;
    px_srcline(74);
    _v1075 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(75);
    px_index_set(px_get_global("cg_vars"), _v1074, _v1075);
    px_srcline(78);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1074}, 1))) {
        px_srcline(79);
        (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v1074}, 1));
    }
    px_srcline(80);
    return _v1075;
px_err_1076:
    if (px_err_1076_proped) return px_err_1076_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_var_of");
    LXValue _v1077 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1078_val = px_null();
    int px_err_1078_proped = 0;
    px_srcline(82);
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1077}, 1))) {
        px_srcline(83);
        return px_index(px_get_global("cg_vars"), _v1077);
    }
    px_srcline(84);
    return px_null();
px_err_1078:
    if (px_err_1078_proped) return px_err_1078_val;
    return px_null();
}

static LXValue fn_cg_load_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_of");
    LXValue _v1079 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1080 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1081_val = px_null();
    int px_err_1081_proped = 0;
    px_srcline(89);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1079}, 1))) {
        px_srcline(90);
        return px_add(px_add(px_str("px_cell_get("), _v1080), px_str(")"));
    }
    px_srcline(91);
    return _v1080;
px_err_1081:
    if (px_err_1081_proped) return px_err_1081_val;
    return px_null();
}

static LXValue fn_cg_store_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_store_of");
    LXValue _v1082 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1083 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1084 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1085_val = px_null();
    int px_err_1085_proped = 0;
    px_srcline(93);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1082}, 1))) {
        px_srcline(94);
        return px_add(px_add(px_add(px_add(px_str("px_cell_set("), _v1083), px_str(", ")), _v1084), px_str(")"));
    }
    px_srcline(95);
    return px_add(px_add(_v1083, px_str(" = ")), _v1084);
px_err_1085:
    if (px_err_1085_proped) return px_err_1085_val;
    return px_null();
}

static LXValue fn_cg_name_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_name_add");
    LXValue _v1086 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1087 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1088_val = px_null();
    int px_err_1088_proped = 0;
    px_srcline(97);
    if (px_is_truthy(({ LXValue _t1089 = px_ne(_v1087, px_str("")); px_is_truthy(_t1089) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1086, _v1087}, 2)) : _t1089; }))) {
        px_srcline(98);
        (void)(px_method(_v1086, "append", (LXValue[]){_v1087}, 1));
    }
px_err_1088:
    if (px_err_1088_proped) return px_err_1088_val;
    return px_null();
}

static LXValue fn_cg_ast_used(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_used");
    LXValue _v1090 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1091 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1092 = px_null();
    LXValue px_err_1093_val = px_null();
    int px_err_1093_proped = 0;
    px_srcline(101);
    if (px_is_truthy(({ LXValue _t1094 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1090}, 1), px_str("list")); px_is_truthy(_t1094) ? _t1094 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1090}, 1), px_int(0LL)); }))) {
        px_srcline(102);
        return px_null();
    }
    px_srcline(103);
    if (px_is_truthy(({ LXValue _t1097 = ({ LXValue _t1096 = ({ LXValue _t1095 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1090, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1095) ? px_eq(px_index(_v1090, px_int(0LL)), px_str("Var")) : _t1095; }); px_is_truthy(_t1096) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1090}, 1), px_int(2LL)) : _t1096; }); px_is_truthy(_t1097) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1090, px_int(1LL))}, 1), px_str("string")) : _t1097; }))) {
        px_srcline(104);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1091, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1090, px_int(1LL))}, 1)}, 2));
        px_srcline(105);
        return px_null();
    }
    px_srcline(106);
    LXValue _t1098 = _v1090;
    for (int _t1099 = 0; _t1099 < px_len(_t1098); _t1099++) {
        _v1092 = px_index(_t1098, px_int(_t1099));
        px_srcline(107);
        (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1092, _v1091}, 2));
    }
px_err_1093:
    if (px_err_1093_proped) return px_err_1093_val;
    return px_null();
}

static LXValue fn_cg_ast_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_bound");
    LXValue _v1100 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1101 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1102 = px_null();
    LXValue _v1103 = px_null();
    LXValue _v1104 = px_null();
    LXValue px_err_1105_val = px_null();
    int px_err_1105_proped = 0;
    px_srcline(110);
    if (px_is_truthy(({ LXValue _t1106 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1100}, 1), px_str("list")); px_is_truthy(_t1106) ? _t1106 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1100}, 1), px_int(0LL)); }))) {
        px_srcline(111);
        return px_null();
    }
    px_srcline(112);
    _v1102 = px_index(_v1100, px_int(0LL));
    px_srcline(113);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1102}, 1), px_str("string")))) {
        px_srcline(114);
        if (px_is_truthy(px_eq(_v1102, px_str("VarDecl")))) {
            px_srcline(115);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1101, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1100, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1102, px_str("For")))) {
            px_srcline(127);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1101, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1100, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1102, px_str("Closure")))) {
            px_srcline(129);
            LXValue _t1107 = px_index(_v1100, px_int(1LL));
            for (int _t1108 = 0; _t1108 < px_len(_t1107); _t1108++) {
                _v1103 = px_index(_t1107, px_int(_t1108));
                px_srcline(130);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1101, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1103, px_int(1LL))}, 1)}, 2));
            }
        }
    }
    px_srcline(131);
    LXValue _t1109 = _v1100;
    for (int _t1110 = 0; _t1110 < px_len(_t1109); _t1110++) {
        _v1104 = px_index(_t1109, px_int(_t1110));
        px_srcline(132);
        (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){_v1104, _v1101}, 2));
    }
px_err_1105:
    if (px_err_1105_proped) return px_err_1105_val;
    return px_null();
}

static LXValue fn_cg_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_closure_caps");
    LXValue _v1111 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1112 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1113 = px_null();
    LXValue _v1114 = px_null();
    LXValue _v1115 = px_null();
    LXValue _v1116 = px_null();
    LXValue px_err_1117_val = px_null();
    int px_err_1117_proped = 0;
    px_srcline(135);
    _v1113 = px_list_n((LXValue[]){}, 0);
    px_srcline(136);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){px_index(_v1111, px_int(3LL)), _v1113}, 2));
    px_srcline(137);
    _v1114 = px_list_n((LXValue[]){}, 0);
    px_srcline(138);
    (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){px_index(_v1111, px_int(3LL)), _v1114}, 2));
    px_srcline(139);
    LXValue _t1118 = px_index(_v1111, px_int(1LL));
    for (int _t1119 = 0; _t1119 < px_len(_t1118); _t1119++) {
        _v1115 = px_index(_t1118, px_int(_t1119));
        px_srcline(140);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1114, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1115, px_int(1LL))}, 1)}, 2));
    }
    px_srcline(141);
    LXValue _t1120 = _v1113;
    for (int _t1121 = 0; _t1121 < px_len(_t1120); _t1121++) {
        _v1116 = px_index(_t1120, px_int(_t1121));
        px_srcline(142);
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1114, _v1116}, 2)))) {
            px_srcline(143);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1112, _v1116}, 2));
        }
    }
px_err_1117:
    if (px_err_1117_proped) return px_err_1117_val;
    return px_null();
}

static LXValue fn_cg_scan_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_scan_closure_caps");
    LXValue _v1122 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1123 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1124 = px_null();
    LXValue px_err_1125_val = px_null();
    int px_err_1125_proped = 0;
    px_srcline(146);
    if (px_is_truthy(({ LXValue _t1126 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1122}, 1), px_str("list")); px_is_truthy(_t1126) ? _t1126 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1122}, 1), px_int(0LL)); }))) {
        px_srcline(147);
        return px_null();
    }
    px_srcline(148);
    if (px_is_truthy(({ LXValue _t1127 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1122, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1127) ? px_eq(px_index(_v1122, px_int(0LL)), px_str("Closure")) : _t1127; }))) {
        px_srcline(149);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){_v1122, _v1123}, 2));
    }
    px_srcline(150);
    if (px_is_truthy(({ LXValue _t1128 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1122, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1128) ? px_eq(px_index(_v1122, px_int(0LL)), px_str("FuncDef")) : _t1128; }))) {
        px_srcline(154);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), px_index(_v1122, px_int(2LL)), px_index(_v1122, px_int(3LL)), px_index(_v1122, px_int(4LL)), px_list_n((LXValue[]){}, 0), px_index(_v1122, px_int(5LL))}, 6), _v1123}, 2));
    }
    px_srcline(155);
    LXValue _t1129 = _v1122;
    for (int _t1130 = 0; _t1130 < px_len(_t1129); _t1130++) {
        _v1124 = px_index(_t1129, px_int(_t1130));
        px_srcline(156);
        (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1124, _v1123}, 2));
    }
px_err_1125:
    if (px_err_1125_proped) return px_err_1125_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_mark_immutable");
    LXValue _v1131 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1132_val = px_null();
    int px_err_1132_proped = 0;
    px_srcline(159);
    px_index_set(px_get_global("cg_immutables"), _v1131, px_int(1LL));
px_err_1132:
    if (px_err_1132_proped) return px_err_1132_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_immutable");
    LXValue _v1133 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1134_val = px_null();
    int px_err_1134_proped = 0;
    px_srcline(161);
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1133}, 1);
px_err_1134:
    if (px_err_1134_proped) return px_err_1134_val;
    return px_null();
}

static LXValue fn_cg_is_wildcard(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_wildcard");
    LXValue _v1135 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1136_val = px_null();
    int px_err_1136_proped = 0;
    px_srcline(169);
    return px_eq(_v1135, px_str("_"));
px_err_1136:
    if (px_err_1136_proped) return px_err_1136_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_perr");
    LXValue _v1137 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1138 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1139_val = px_null();
    int px_err_1139_proped = 0;
    px_srcline(174);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1137), px_str(": ")), _v1138)}, 1));
    px_srcline(175);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1139:
    if (px_err_1139_proped) return px_err_1139_val;
    return px_null();
}

static LXValue fn_cg_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pwarn");
    LXValue _v1140 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1141_val = px_null();
    int px_err_1141_proped = 0;
    px_srcline(178);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1140)}, 1));
px_err_1141:
    if (px_err_1141_proped) return px_err_1141_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_nonnull_ty");
    LXValue _v1142 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1143_val = px_null();
    int px_err_1143_proped = 0;
    px_srcline(182);
    if (px_is_truthy(px_eq(_v1142, px_null()))) {
        px_srcline(183);
        return px_bool(false);
    }
    px_srcline(184);
    if (px_is_truthy(px_eq(px_index(_v1142, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(185);
        return px_bool(false);
    }
    px_srcline(186);
    return px_bool(true);
px_err_1143:
    if (px_err_1143_proped) return px_err_1143_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_null_lit");
    LXValue _v1144 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1145_val = px_null();
    int px_err_1145_proped = 0;
    px_srcline(189);
    if (px_is_truthy(px_eq(_v1144, px_null()))) {
        px_srcline(190);
        return px_bool(false);
    }
    px_srcline(191);
    if (px_is_truthy(px_eq(px_index(_v1144, px_int(0LL)), px_str("Null")))) {
        px_srcline(192);
        return px_bool(true);
    }
    px_srcline(193);
    return px_bool(false);
px_err_1145:
    if (px_err_1145_proped) return px_err_1145_val;
    return px_null();
}

static LXValue fn_cg_sem_vardecl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_vardecl");
    LXValue _v1146 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1147 = px_null();
    LXValue _v1148 = px_null();
    LXValue px_err_1149_val = px_null();
    int px_err_1149_proped = 0;
    px_srcline(211);
    _v1147 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1146, px_int(2LL))}, 1);
    px_srcline(213);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1147}, 1))) {
        px_srcline(214);
        return px_null();
    }
    px_srcline(215);
    if (px_is_truthy(({ LXValue _t1150 = px_eq(px_index(_v1146, px_int(1LL)), px_str("Let")); px_is_truthy(_t1150) ? _t1150 : px_eq(px_index(_v1146, px_int(1LL)), px_str("Const")); }))) {
        px_srcline(216);
        (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v1147}, 1));
    }
    px_srcline(217);
    _v1148 = px_index(_v1146, px_int(3LL));
    px_srcline(218);
    if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v1148}, 1))) {
        px_srcline(219);
        px_index_set(px_get_global("cg_nonnull"), _v1147, px_int(1LL));
        px_srcline(220);
        if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v1146, px_int(4LL))}, 1))) {
            px_srcline(221);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1148}, 1)), px_str("'（可空类型请用 ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1148}, 1)), px_str("? 声明）"))}, 2));
        }
    }
px_err_1149:
    if (px_err_1149_proped) return px_err_1149_val;
    return px_null();
}

static LXValue fn_cg_sem_assign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_assign");
    LXValue _v1151 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1152 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1153 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1154 = px_null();
    LXValue px_err_1155_val = px_null();
    int px_err_1155_proped = 0;
    px_srcline(225);
    if (px_is_truthy(px_eq(_v1151, px_null()))) {
        px_srcline(226);
        return px_null();
    }
    px_srcline(227);
    if (px_is_truthy(px_ne(px_index(_v1151, px_int(0LL)), px_str("Var")))) {
        px_srcline(228);
        return px_null();
    }
    px_srcline(229);
    _v1154 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1151, px_int(1LL))}, 1);
    px_srcline(231);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1154}, 1))) {
        px_srcline(232);
        return px_null();
    }
    px_srcline(233);
    if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v1154}, 1))) {
        px_srcline(234);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v1154), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
    }
    px_srcline(235);
    if (px_is_truthy(({ LXValue _t1156 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){_v1153}, 1); px_is_truthy(_t1156) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v1154}, 1) : _t1156; }))) {
        px_srcline(236);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v1154), px_str("'（可空类型请声明为 ")), _v1154), px_str(": T?）"))}, 2));
    }
px_err_1155:
    if (px_err_1155_proped) return px_err_1155_val;
    return px_null();
}

static LXValue fn_cg_sem_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_call");
    LXValue _v1157 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1158 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1159 = px_null();
    LXValue _v1160 = px_null();
    LXValue px_err_1161_val = px_null();
    int px_err_1161_proped = 0;
    px_srcline(239);
    if (px_is_truthy(px_eq(_v1157, px_null()))) {
        px_srcline(240);
        return px_null();
    }
    px_srcline(241);
    if (px_is_truthy(px_ne(px_index(_v1157, px_int(0LL)), px_str("Var")))) {
        px_srcline(242);
        return px_null();
    }
    px_srcline(243);
    _v1159 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1157, px_int(1LL))}, 1);
    px_srcline(244);
    if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v1159}, 1))) {
        px_srcline(245);
        _v1160 = px_index(px_get_global("cg_ffi"), _v1159);
        px_srcline(246);
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1158}, 1), px_call(px_get_global("len"), (LXValue[]){_v1160}, 1)))) {
            px_srcline(247);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), px_add(px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v1159), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1160}, 1)}, 1)), px_str(" 个参数，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1158}, 1)}, 1))}, 2));
        }
    }
px_err_1161:
    if (px_err_1161_proped) return px_err_1161_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ty_name");
    LXValue _v1162 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1163_val = px_null();
    int px_err_1163_proped = 0;
    px_srcline(250);
    if (px_is_truthy(px_eq(_v1162, px_null()))) {
        px_srcline(251);
        return px_str("any");
    }
    px_srcline(252);
    if (px_is_truthy(px_eq(px_index(_v1162, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(253);
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1162, px_int(1LL))}, 1), px_str("?"));
    }
    px_srcline(254);
    if (px_is_truthy(px_eq(px_index(_v1162, px_int(0LL)), px_str("TyNamed")))) {
        px_srcline(255);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1162, px_int(1LL))}, 1);
    }
    px_srcline(256);
    if (px_is_truthy(px_eq(px_index(_v1162, px_int(0LL)), px_str("TyList")))) {
        px_srcline(257);
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1162, px_int(1LL))}, 1)), px_str("]"));
    }
    px_srcline(258);
    if (px_is_truthy(px_eq(px_index(_v1162, px_int(0LL)), px_str("TyDict")))) {
        px_srcline(259);
        return px_add(px_add(px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1162, px_int(1LL))}, 1)), px_str(": ")), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1162, px_int(2LL))}, 1)), px_str("}"));
    }
    px_srcline(260);
    return px_str("any");
px_err_1163:
    if (px_err_1163_proped) return px_err_1163_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_func_cname");
    LXValue _v1164 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1165 = px_null();
    LXValue _v1166 = px_null();
    LXValue _v1167 = px_null();
    LXValue _v1168 = px_null();
    LXValue _v1169 = px_null();
    LXValue _v1170 = px_null();
    LXValue px_err_1171_val = px_null();
    int px_err_1171_proped = 0;
    px_srcline(262);
    _v1165 = px_list_n((LXValue[]){}, 0);
    px_srcline(263);
    _v1166 = px_int(0LL);
    px_srcline(264);
    while (px_is_truthy(px_lt(_v1166, px_call(px_get_global("len"), (LXValue[]){_v1164}, 1)))) {
        px_srcline(265);
        _v1167 = px_index(_v1164, _v1166);
        px_srcline(266);
        _v1168 = ({ LXValue _t1172 = px_ge(_v1167, px_str("a")); px_is_truthy(_t1172) ? px_le(_v1167, px_str("z")) : _t1172; });
        px_srcline(267);
        _v1169 = ({ LXValue _t1173 = px_ge(_v1167, px_str("A")); px_is_truthy(_t1173) ? px_le(_v1167, px_str("Z")) : _t1173; });
        px_srcline(268);
        _v1170 = ({ LXValue _t1174 = px_ge(_v1167, px_str("0")); px_is_truthy(_t1174) ? px_le(_v1167, px_str("9")) : _t1174; });
        px_srcline(269);
        if (px_is_truthy(({ LXValue _t1176 = ({ LXValue _t1175 = _v1168; px_is_truthy(_t1175) ? _t1175 : _v1169; }); px_is_truthy(_t1176) ? _t1176 : _v1170; }))) {
            px_srcline(270);
            (void)(px_method(_v1165, "append", (LXValue[]){_v1167}, 1));
        }
        else {
            px_srcline(272);
            (void)(px_method(_v1165, "append", (LXValue[]){px_str("_")}, 1));
        }
        px_srcline(273);
         _v1166 = px_add(_v1166, px_int(1LL));
    }
    px_srcline(274);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1165}, 2);
px_err_1171:
    if (px_err_1171_proped) return px_err_1171_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find");
    LXValue _v1177 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1178 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1179 = px_null();
    LXValue _v1180 = px_null();
    LXValue _v1181 = px_null();
    LXValue _v1182 = px_null();
    LXValue _v1183 = px_null();
    LXValue px_err_1184_val = px_null();
    int px_err_1184_proped = 0;
    px_srcline(276);
    _v1179 = px_call(px_get_global("len"), (LXValue[]){_v1177}, 1);
    px_srcline(277);
    _v1180 = px_call(px_get_global("len"), (LXValue[]){_v1178}, 1);
    px_srcline(278);
    _v1181 = px_int(0LL);
    px_srcline(279);
    while (px_is_truthy(px_le(px_add(_v1181, _v1180), _v1179))) {
        px_srcline(280);
        _v1182 = px_int(0LL);
        px_srcline(281);
        _v1183 = px_bool(true);
        px_srcline(282);
        while (px_is_truthy(px_lt(_v1182, _v1180))) {
            px_srcline(283);
            if (px_is_truthy(px_ne(px_index(_v1177, px_add(_v1181, _v1182)), px_index(_v1178, _v1182)))) {
                px_srcline(284);
                 _v1183 = px_bool(false);
                px_srcline(285);
                break;
            }
            px_srcline(286);
             _v1182 = px_add(_v1182, px_int(1LL));
        }
        px_srcline(287);
        if (px_is_truthy(_v1183)) {
            px_srcline(288);
            return _v1181;
        }
        px_srcline(289);
         _v1181 = px_add(_v1181, px_int(1LL));
    }
    px_srcline(290);
    return px_neg(px_int(1LL));
px_err_1184:
    if (px_err_1184_proped) return px_err_1184_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad");
    LXValue _v1185 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1186 = px_null();
    LXValue _v1187 = px_null();
    LXValue px_err_1188_val = px_null();
    int px_err_1188_proped = 0;
    px_srcline(292);
    _v1186 = px_list_n((LXValue[]){}, 0);
    px_srcline(293);
    _v1187 = px_int(0LL);
    px_srcline(294);
    while (px_is_truthy(px_lt(_v1187, _v1185))) {
        px_srcline(295);
        (void)(px_method(_v1186, "append", (LXValue[]){px_str("    ")}, 1));
        px_srcline(296);
         _v1187 = px_add(_v1187, px_int(1LL));
    }
    px_srcline(297);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1186}, 2);
px_err_1188:
    if (px_err_1188_proped) return px_err_1188_val;
    return px_null();
}

static LXValue fn_cg_nul_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_nul_char");
    LXValue px_err_1189_val = px_null();
    int px_err_1189_proped = 0;
    px_srcline(304);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){px_int(0LL), px_int(1LL)}, 2)}, 1);
px_err_1189:
    if (px_err_1189_proped) return px_err_1189_val;
    return px_null();
}

static LXValue fn_cg_has_nul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_has_nul");
    LXValue _v1190 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1191 = px_null();
    LXValue _v1192 = px_null();
    LXValue px_err_1193_val = px_null();
    int px_err_1193_proped = 0;
    px_srcline(307);
    _v1191 = px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0);
    px_srcline(308);
    _v1192 = px_int(0LL);
    px_srcline(309);
    while (px_is_truthy(px_lt(_v1192, px_call(px_get_global("len"), (LXValue[]){_v1190}, 1)))) {
        px_srcline(310);
        if (px_is_truthy(px_eq(px_index(_v1190, _v1192), _v1191))) {
            px_srcline(311);
            return px_bool(true);
        }
        px_srcline(312);
         _v1192 = px_add(_v1192, px_int(1LL));
    }
    px_srcline(313);
    return px_bool(false);
px_err_1193:
    if (px_err_1193_proped) return px_err_1193_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_unescape");
    LXValue _v1194 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1195 = px_null();
    LXValue _v1196 = px_null();
    LXValue _v1197 = px_null();
    LXValue _v1198 = px_null();
    LXValue _v1199 = px_null();
    LXValue _v1200 = px_null();
    LXValue _v1201 = px_null();
    LXValue px_err_1202_val = px_null();
    int px_err_1202_proped = 0;
    px_srcline(315);
    _v1195 = px_slice(_v1194, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1194}, 1), px_int(1LL)), px_null());
    px_srcline(316);
    _v1196 = px_list_n((LXValue[]){}, 0);
    px_srcline(317);
    _v1197 = px_int(0LL);
    px_srcline(318);
    while (px_is_truthy(px_lt(_v1197, px_call(px_get_global("len"), (LXValue[]){_v1195}, 1)))) {
        px_srcline(319);
        _v1198 = px_index(_v1195, _v1197);
        px_srcline(320);
        if (px_is_truthy(px_eq(_v1198, px_str("\\")))) {
            px_srcline(321);
            _v1199 = px_index(_v1195, px_add(_v1197, px_int(1LL)));
            px_srcline(322);
            if (px_is_truthy(px_eq(_v1199, px_str("n")))) {
                px_srcline(323);
                (void)(px_method(_v1196, "append", (LXValue[]){px_str("\n")}, 1));
                px_srcline(324);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1199, px_str("t")))) {
                px_srcline(326);
                (void)(px_method(_v1196, "append", (LXValue[]){px_str("\t")}, 1));
                px_srcline(327);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1199, px_str("r")))) {
                px_srcline(329);
                (void)(px_method(_v1196, "append", (LXValue[]){px_str("\r")}, 1));
                px_srcline(330);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1199, px_str("0")))) {
                px_srcline(332);
                (void)(px_method(_v1196, "append", (LXValue[]){px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)}, 1));
                px_srcline(333);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1199, px_str("\"")))) {
                px_srcline(335);
                (void)(px_method(_v1196, "append", (LXValue[]){px_str("\"")}, 1));
                px_srcline(336);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1199, px_str("\\")))) {
                px_srcline(338);
                (void)(px_method(_v1196, "append", (LXValue[]){px_str("\\")}, 1));
                px_srcline(339);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1199, px_str("u")))) {
                px_srcline(341);
                _v1200 = px_add(_v1197, px_int(3LL));
                px_srcline(342);
                _v1201 = px_list_n((LXValue[]){}, 0);
                px_srcline(343);
                while (px_is_truthy(({ LXValue _t1203 = px_lt(_v1200, px_call(px_get_global("len"), (LXValue[]){_v1195}, 1)); px_is_truthy(_t1203) ? px_ne(px_index(_v1195, _v1200), px_str("}")) : _t1203; }))) {
                    px_srcline(344);
                    (void)(px_method(_v1201, "append", (LXValue[]){px_index(_v1195, _v1200)}, 1));
                    px_srcline(345);
                     _v1200 = px_add(_v1200, px_int(1LL));
                }
                px_srcline(346);
                (void)(px_method(_v1196, "append", (LXValue[]){px_call(px_get_global("hex_to_char"), (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1201}, 2)}, 1)}, 1));
                px_srcline(347);
                 _v1197 = px_add(_v1200, px_int(1LL));
            }
            else {
                px_srcline(349);
                (void)(px_method(_v1196, "append", (LXValue[]){_v1199}, 1));
                px_srcline(350);
                 _v1197 = px_add(_v1197, px_int(2LL));
            }
        }
        else {
            px_srcline(352);
            (void)(px_method(_v1196, "append", (LXValue[]){_v1198}, 1));
            px_srcline(353);
             _v1197 = px_add(_v1197, px_int(1LL));
        }
    }
    px_srcline(354);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1196}, 2);
px_err_1202:
    if (px_err_1202_proped) return px_err_1202_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_escape_str");
    LXValue _v1204 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1205 = px_null();
    LXValue _v1206 = px_null();
    LXValue _v1207 = px_null();
    LXValue px_err_1208_val = px_null();
    int px_err_1208_proped = 0;
    px_srcline(357);
    _v1205 = px_list_n((LXValue[]){}, 0);
    px_srcline(358);
    _v1206 = px_int(0LL);
    px_srcline(359);
    while (px_is_truthy(px_lt(_v1206, px_call(px_get_global("len"), (LXValue[]){_v1204}, 1)))) {
        px_srcline(360);
        _v1207 = px_index(_v1204, _v1206);
        px_srcline(361);
        if (px_is_truthy(px_eq(_v1207, px_str("\\")))) {
            px_srcline(362);
            (void)(px_method(_v1205, "append", (LXValue[]){px_str("\\\\")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1207, px_str("\"")))) {
            px_srcline(364);
            (void)(px_method(_v1205, "append", (LXValue[]){px_str("\\\"")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1207, px_str("\n")))) {
            px_srcline(366);
            (void)(px_method(_v1205, "append", (LXValue[]){px_str("\\n")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1207, px_str("\r")))) {
            px_srcline(368);
            (void)(px_method(_v1205, "append", (LXValue[]){px_str("\\r")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1207, px_str("\t")))) {
            px_srcline(370);
            (void)(px_method(_v1205, "append", (LXValue[]){px_str("\\t")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1207, px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)))) {
            px_srcline(378);
            (void)(px_method(_v1205, "append", (LXValue[]){px_str("\\000")}, 1));
        }
        else {
            px_srcline(380);
            (void)(px_method(_v1205, "append", (LXValue[]){_v1207}, 1));
        }
        px_srcline(381);
         _v1206 = px_add(_v1206, px_int(1LL));
    }
    px_srcline(382);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1205}, 2);
px_err_1208:
    if (px_err_1208_proped) return px_err_1208_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad_zeros");
    LXValue _v1209 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1210 = px_null();
    LXValue _v1211 = px_null();
    LXValue px_err_1212_val = px_null();
    int px_err_1212_proped = 0;
    px_srcline(385);
    _v1210 = px_list_n((LXValue[]){}, 0);
    px_srcline(386);
    _v1211 = px_int(0LL);
    px_srcline(387);
    while (px_is_truthy(px_lt(_v1211, _v1209))) {
        px_srcline(388);
        (void)(px_method(_v1210, "append", (LXValue[]){px_str("0")}, 1));
        px_srcline(389);
         _v1211 = px_add(_v1211, px_int(1LL));
    }
    px_srcline(390);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1210}, 2);
px_err_1212:
    if (px_err_1212_proped) return px_err_1212_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_expand_sci");
    LXValue _v1213 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1214 = px_null();
    LXValue _v1215 = px_null();
    LXValue _v1216 = px_null();
    LXValue _v1217 = px_null();
    LXValue _v1218 = px_null();
    LXValue _v1219 = px_null();
    LXValue _v1220 = px_null();
    LXValue _v1221 = px_null();
    LXValue _v1222 = px_null();
    LXValue _v1223 = px_null();
    LXValue _v1224 = px_null();
    LXValue _v1225 = px_null();
    LXValue _v1226 = px_null();
    LXValue _v1227 = px_null();
    LXValue px_err_1228_val = px_null();
    int px_err_1228_proped = 0;
    px_srcline(392);
    _v1214 = px_neg(px_int(1LL));
    px_srcline(393);
    _v1215 = px_int(0LL);
    px_srcline(394);
    while (px_is_truthy(px_lt(_v1215, px_call(px_get_global("len"), (LXValue[]){_v1213}, 1)))) {
        px_srcline(395);
        if (px_is_truthy(({ LXValue _t1229 = px_eq(px_index(_v1213, _v1215), px_str("e")); px_is_truthy(_t1229) ? _t1229 : px_eq(px_index(_v1213, _v1215), px_str("E")); }))) {
            px_srcline(396);
             _v1214 = _v1215;
            px_srcline(397);
            break;
        }
        px_srcline(398);
         _v1215 = px_add(_v1215, px_int(1LL));
    }
    px_srcline(399);
    if (px_is_truthy(px_lt(_v1214, px_int(0LL)))) {
        px_srcline(400);
        return _v1213;
    }
    px_srcline(401);
    _v1216 = px_slice(_v1213, px_int(0LL), _v1214, px_null());
    px_srcline(402);
    _v1217 = px_slice(_v1213, px_add(_v1214, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1213}, 1), px_null());
    px_srcline(403);
    _v1218 = px_int(1LL);
    px_srcline(404);
    if (px_is_truthy(({ LXValue _t1230 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1217}, 1), px_int(0LL)); px_is_truthy(_t1230) ? px_eq(px_index(_v1217, px_int(0LL)), px_str("+")) : _t1230; }))) {
        px_srcline(405);
         _v1217 = px_slice(_v1217, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1217}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1231 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1217}, 1), px_int(0LL)); px_is_truthy(_t1231) ? px_eq(px_index(_v1217, px_int(0LL)), px_str("-")) : _t1231; }))) {
        px_srcline(407);
         _v1218 = px_neg(px_int(1LL));
        px_srcline(408);
         _v1217 = px_slice(_v1217, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1217}, 1), px_null());
    }
    px_srcline(409);
    _v1219 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1217}, 1), _v1218);
    px_srcline(410);
    _v1220 = px_bool(false);
    px_srcline(411);
    if (px_is_truthy(({ LXValue _t1232 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1216}, 1), px_int(0LL)); px_is_truthy(_t1232) ? px_eq(px_index(_v1216, px_int(0LL)), px_str("-")) : _t1232; }))) {
        px_srcline(412);
         _v1220 = px_bool(true);
        px_srcline(413);
         _v1216 = px_slice(_v1216, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1216}, 1), px_null());
    }
    px_srcline(414);
    _v1221 = px_str("");
    px_srcline(415);
    _v1222 = px_str("");
    px_srcline(416);
    _v1223 = px_neg(px_int(1LL));
    px_srcline(417);
    _v1224 = px_int(0LL);
    px_srcline(418);
    while (px_is_truthy(px_lt(_v1224, px_call(px_get_global("len"), (LXValue[]){_v1216}, 1)))) {
        px_srcline(419);
        if (px_is_truthy(px_eq(px_index(_v1216, _v1224), px_str(".")))) {
            px_srcline(420);
             _v1223 = _v1224;
            px_srcline(421);
            break;
        }
        px_srcline(422);
         _v1224 = px_add(_v1224, px_int(1LL));
    }
    px_srcline(423);
    if (px_is_truthy(px_lt(_v1223, px_int(0LL)))) {
        px_srcline(424);
         _v1221 = _v1216;
    }
    else {
        px_srcline(426);
         _v1221 = px_slice(_v1216, px_int(0LL), _v1223, px_null());
        px_srcline(427);
         _v1222 = px_slice(_v1216, px_add(_v1223, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1216}, 1), px_null());
    }
    px_srcline(428);
    _v1225 = px_add(_v1221, _v1222);
    px_srcline(429);
    _v1226 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1221}, 1), _v1219);
    px_srcline(430);
    _v1227 = px_str("");
    px_srcline(431);
    if (px_is_truthy(px_le(_v1226, px_int(0LL)))) {
        px_srcline(432);
         _v1227 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1226)}, 1)), _v1225);
    }
    else if (px_is_truthy(px_ge(_v1226, px_call(px_get_global("len"), (LXValue[]){_v1225}, 1)))) {
        px_srcline(434);
         _v1227 = px_add(_v1225, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1226, px_call(px_get_global("len"), (LXValue[]){_v1225}, 1))}, 1));
    }
    else {
        px_srcline(436);
         _v1227 = px_add(px_add(px_slice(_v1225, px_int(0LL), _v1226, px_null()), px_str(".")), px_slice(_v1225, _v1226, px_call(px_get_global("len"), (LXValue[]){_v1225}, 1), px_null()));
    }
    px_srcline(437);
    if (px_is_truthy(_v1220)) {
        px_srcline(438);
        return px_add(px_str("-"), _v1227);
    }
    px_srcline(439);
    return _v1227;
px_err_1228:
    if (px_err_1228_proped) return px_err_1228_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_fmt_float");
    LXValue _v1233 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1234 = px_null();
    LXValue _v1235 = px_null();
    LXValue _v1236 = px_null();
    LXValue px_err_1237_val = px_null();
    int px_err_1237_proped = 0;
    px_srcline(442);
    _v1234 = px_call(px_get_global("str"), (LXValue[]){_v1233}, 1);
    px_srcline(443);
    if (px_is_truthy(({ LXValue _t1240 = ({ LXValue _t1239 = ({ LXValue _t1238 = px_eq(_v1234, px_str("inf")); px_is_truthy(_t1238) ? _t1238 : px_eq(_v1234, px_str("-inf")); }); px_is_truthy(_t1239) ? _t1239 : px_eq(_v1234, px_str("nan")); }); px_is_truthy(_t1240) ? _t1240 : px_eq(_v1234, px_str("-nan")); }))) {
        px_srcline(444);
        return _v1234;
    }
    px_srcline(445);
     _v1234 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1234}, 1);
    px_srcline(446);
    _v1235 = px_call(px_get_global("len"), (LXValue[]){_v1234}, 1);
    px_srcline(447);
    if (px_is_truthy(({ LXValue _t1241 = px_ge(_v1235, px_int(2LL)); px_is_truthy(_t1241) ? px_eq(px_slice(_v1234, px_sub(_v1235, px_int(2LL)), _v1235, px_null()), px_str(".0")) : _t1241; }))) {
        px_srcline(453);
        _v1236 = px_slice(_v1234, px_int(0LL), px_sub(_v1235, px_int(2LL)), px_null());
        px_srcline(454);
        if (px_is_truthy(px_eq(_v1236, px_str("-0")))) {
            px_srcline(455);
            return _v1234;
        }
        px_srcline(456);
        return _v1236;
    }
    px_srcline(464);
    if (px_is_truthy(({ LXValue _t1242 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1234, px_str(".")}, 2)); px_is_truthy(_t1242) ? px_ge(_v1235, px_int(19LL)) : _t1242; }))) {
        px_srcline(465);
        return px_add(_v1234, px_str(".0"));
    }
    px_srcline(466);
    return _v1234;
px_err_1237:
    if (px_err_1237_proped) return px_err_1237_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_types");
    LXValue _v1243 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1244 = px_null();
    LXValue _v1245 = px_null();
    LXValue _v1246 = px_null();
    LXValue _v1247 = px_null();
    LXValue _v1248 = px_null();
    LXValue _v1249 = px_null();
    LXValue _v1250 = px_null();
    LXValue _v1251 = px_null();
    LXValue _v1252 = px_null();
    LXValue _v1253 = px_null();
    LXValue px_err_1254_val = px_null();
    int px_err_1254_proped = 0;
    px_srcline(469);
    _v1244 = px_index(_v1243, px_int(1LL));
    px_srcline(470);
    _v1245 = px_int(0LL);
    px_srcline(471);
    while (px_is_truthy(px_lt(_v1245, px_call(px_get_global("len"), (LXValue[]){_v1244}, 1)))) {
        px_srcline(472);
        _v1246 = px_index(_v1244, _v1245);
        px_srcline(473);
        _v1247 = px_index(_v1246, px_int(0LL));
        px_srcline(474);
        if (px_is_truthy(px_eq(_v1247, px_str("StructDef")))) {
            px_srcline(475);
            _v1248 = px_list_n((LXValue[]){}, 0);
            px_srcline(476);
            _v1249 = px_int(0LL);
            px_srcline(477);
            while (px_is_truthy(px_lt(_v1249, px_call(px_get_global("len"), (LXValue[]){px_index(_v1246, px_int(2LL))}, 1)))) {
                px_srcline(478);
                (void)(px_method(_v1248, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1246, px_int(2LL)), _v1249), px_int(1LL))}, 1)}, 1));
                px_srcline(479);
                 _v1249 = px_add(_v1249, px_int(1LL));
            }
            px_srcline(480);
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1246, px_int(1LL))}, 1), _v1248);
        }
        else if (px_is_truthy(px_eq(_v1247, px_str("EnumDef")))) {
            px_srcline(482);
            _v1250 = px_list_n((LXValue[]){}, 0);
            px_srcline(483);
            _v1251 = px_int(0LL);
            px_srcline(484);
            while (px_is_truthy(px_lt(_v1251, px_call(px_get_global("len"), (LXValue[]){px_index(_v1246, px_int(2LL))}, 1)))) {
                px_srcline(485);
                (void)(px_method(_v1250, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1246, px_int(2LL)), _v1251), px_int(1LL))}, 1)}, 1));
                px_srcline(486);
                 _v1251 = px_add(_v1251, px_int(1LL));
            }
            px_srcline(487);
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1246, px_int(1LL))}, 1), _v1250);
        }
        else if (px_is_truthy(px_eq(_v1247, px_str("ImplDef")))) {
            px_srcline(489);
            _v1252 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1246, px_int(1LL))}, 1);
            px_srcline(490);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1252}, 1))) {
                px_srcline(491);
                _v1253 = px_int(0LL);
                px_srcline(492);
                while (px_is_truthy(px_lt(_v1253, px_call(px_get_global("len"), (LXValue[]){px_index(_v1246, px_int(3LL))}, 1)))) {
                    px_srcline(493);
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1252), "append", (LXValue[]){px_index(px_index(_v1246, px_int(3LL)), _v1253)}, 1));
                    px_srcline(494);
                     _v1253 = px_add(_v1253, px_int(1LL));
                }
            }
            else {
                px_srcline(496);
                px_index_set(px_get_global("cg_impls"), _v1252, px_index(_v1246, px_int(3LL)));
            }
        }
        px_srcline(497);
         _v1245 = px_add(_v1245, px_int(1LL));
    }
    px_srcline(500);
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1244}, 1));
px_err_1254:
    if (px_err_1254_proped) return px_err_1254_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_consts");
    LXValue _v1255 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1256 = px_null();
    LXValue _v1257 = px_null();
    LXValue _v1258 = px_null();
    LXValue _v1259 = px_null();
    LXValue _v1260 = px_null();
    LXValue _v1261 = px_null();
    LXValue _v1262 = px_null();
    LXValue _v1263 = px_null();
    LXValue px_err_1264_val = px_null();
    int px_err_1264_proped = 0;
    px_srcline(503);
    _v1256 = px_int(0LL);
    px_srcline(504);
    while (px_is_truthy(px_lt(_v1256, px_call(px_get_global("len"), (LXValue[]){_v1255}, 1)))) {
        px_srcline(505);
        _v1257 = px_index(_v1255, _v1256);
        px_srcline(506);
        _v1258 = px_index(_v1257, px_int(0LL));
        px_srcline(507);
        if (px_is_truthy(px_eq(_v1258, px_str("TypeConst")))) {
            px_srcline(508);
            _v1259 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1257, px_int(1LL))}, 1);
            px_srcline(509);
            _v1260 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            px_srcline(510);
            _v1261 = px_int(0LL);
            px_srcline(511);
            while (px_is_truthy(px_lt(_v1261, px_call(px_get_global("len"), (LXValue[]){px_index(_v1257, px_int(2LL))}, 1)))) {
                px_srcline(512);
                _v1262 = px_index(px_index(_v1257, px_int(2LL)), _v1261);
                px_srcline(513);
                px_index_set(_v1260, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1262, px_int(1LL))}, 1), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1262, px_int(2LL))}, 1));
                px_srcline(514);
                 _v1261 = px_add(_v1261, px_int(1LL));
            }
            px_srcline(515);
            px_index_set(px_get_global("cg_const_enums"), _v1259, _v1260);
        }
        else if (px_is_truthy(px_eq(_v1258, px_str("FuncDef")))) {
            px_srcline(517);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1257, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1258, px_str("If")))) {
            px_srcline(519);
            _v1263 = px_int(0LL);
            px_srcline(520);
            while (px_is_truthy(px_lt(_v1263, px_call(px_get_global("len"), (LXValue[]){px_index(_v1257, px_int(1LL))}, 1)))) {
                px_srcline(521);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1257, px_int(1LL)), _v1263), px_int(1LL))}, 1));
                px_srcline(522);
                 _v1263 = px_add(_v1263, px_int(1LL));
            }
            px_srcline(523);
            if (px_is_truthy(px_ne(px_index(_v1257, px_int(2LL)), px_null()))) {
                px_srcline(524);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1257, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1258, px_str("For")))) {
            px_srcline(526);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1257, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1258, px_str("While")))) {
            px_srcline(528);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1257, px_int(2LL))}, 1));
        }
        px_srcline(529);
         _v1256 = px_add(_v1256, px_int(1LL));
    }
px_err_1264:
    if (px_err_1264_proped) return px_err_1264_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_hoist_vars");
    LXValue _v1265 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1266 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1267 = px_null();
    LXValue _v1268 = px_null();
    LXValue _v1269 = px_null();
    LXValue _v1270 = px_null();
    LXValue _v1271 = px_null();
    LXValue _v1272 = px_null();
    LXValue _v1273 = px_null();
    LXValue _v1274 = px_null();
    LXValue px_err_1275_val = px_null();
    int px_err_1275_proped = 0;
    px_srcline(538);
    _v1267 = px_int(0LL);
    px_srcline(539);
    while (px_is_truthy(px_lt(_v1267, px_call(px_get_global("len"), (LXValue[]){_v1265}, 1)))) {
        px_srcline(540);
        _v1268 = px_index(_v1265, _v1267);
        px_srcline(541);
        _v1269 = px_index(_v1268, px_int(0LL));
        px_srcline(542);
        if (px_is_truthy(px_eq(_v1269, px_str("Assign")))) {
            px_srcline(543);
            _v1270 = px_index(_v1268, px_int(1LL));
            px_srcline(544);
            if (px_is_truthy(px_eq(px_index(_v1270, px_int(0LL)), px_str("Var")))) {
                px_srcline(545);
                _v1271 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1270, px_int(1LL))}, 1);
                px_srcline(546);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1266, _v1271}, 2)))) {
                    px_srcline(547);
                    (void)(px_method(_v1266, "append", (LXValue[]){_v1271}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1269, px_str("VarDecl")))) {
            px_srcline(549);
            _v1271 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1268, px_int(2LL))}, 1);
            px_srcline(550);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1266, _v1271}, 2)))) {
                px_srcline(551);
                (void)(px_method(_v1266, "append", (LXValue[]){_v1271}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1269, px_str("FuncDef")))) {
            px_srcline(556);
            _v1271 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1268, px_int(1LL))}, 1);
            px_srcline(557);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1266, _v1271}, 2)))) {
                px_srcline(558);
                (void)(px_method(_v1266, "append", (LXValue[]){_v1271}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1269, px_str("If")))) {
            px_srcline(560);
            _v1272 = px_index(_v1268, px_int(1LL));
            px_srcline(561);
            _v1273 = px_int(0LL);
            px_srcline(562);
            while (px_is_truthy(px_lt(_v1273, px_call(px_get_global("len"), (LXValue[]){_v1272}, 1)))) {
                px_srcline(563);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1272, _v1273), px_int(1LL)), _v1266}, 2));
                px_srcline(564);
                 _v1273 = px_add(_v1273, px_int(1LL));
            }
            px_srcline(565);
            if (px_is_truthy(px_ne(px_index(_v1268, px_int(2LL)), px_null()))) {
                px_srcline(566);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1268, px_int(2LL)), _v1266}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1269, px_str("For")))) {
            px_srcline(568);
            _v1274 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1268, px_int(1LL))}, 1);
            px_srcline(569);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1266, _v1274}, 2)))) {
                px_srcline(570);
                (void)(px_method(_v1266, "append", (LXValue[]){_v1274}, 1));
            }
            px_srcline(571);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1268, px_int(3LL)), _v1266}, 2));
        }
        else if (px_is_truthy(px_eq(_v1269, px_str("While")))) {
            px_srcline(573);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1268, px_int(2LL)), _v1266}, 2));
        }
        px_srcline(574);
         _v1267 = px_add(_v1267, px_int(1LL));
    }
px_err_1275:
    if (px_err_1275_proped) return px_err_1275_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func");
    LXValue _v1276 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1277 = px_null();
    LXValue px_err_1278_val = px_null();
    int px_err_1278_proped = 0;
    px_srcline(577);
    _v1277 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1276, px_int(1LL))}, 1)}, 1));
    px_srcline(578);
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1276, _v1277}, 2);
px_err_1278:
    if (px_err_1278_proped) return px_err_1278_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func_named");
    LXValue _v1279 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1280 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v1304 = px_null();
    LXValue px_err_1305_val = px_null();
    int px_err_1305_proped = 0;
    px_srcline(580);
    _v1281 = px_add(px_add(px_str("static LXValue "), _v1280), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(581);
     _v1281 = px_add(_v1281, px_str("    (void)ctx;\n"));
    px_srcline(584);
     _v1281 = px_add(_v1281, px_add(px_add(px_str("    px_srcfunc(\""), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1279, px_int(1LL))}, 1)), px_str("\");\n")));
    px_srcline(585);
    _v1282 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(586);
    _v1283 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(587);
    _v1284 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(588);
    _v1285 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(589);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(590);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(591);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(592);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1285}, 1));
    px_srcline(596);
    _v1286 = px_list_n((LXValue[]){}, 0);
    px_srcline(597);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){px_index(_v1279, px_int(4LL)), _v1286}, 2));
    px_srcline(599);
    _v1287 = px_index(_v1279, px_int(2LL));
    px_srcline(600);
    _v1288 = px_int(0LL);
    px_srcline(601);
    _v1289 = px_list_n((LXValue[]){}, 0);
    px_srcline(602);
    while (px_is_truthy(px_lt(_v1288, px_call(px_get_global("len"), (LXValue[]){_v1287}, 1)))) {
        px_srcline(603);
        _v1290 = px_index(_v1287, _v1288);
        px_srcline(604);
        _v1291 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1290, px_int(1LL))}, 1);
        px_srcline(605);
        _v1292 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1291}, 1);
        px_srcline(606);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1286, _v1291}, 2))) {
            px_srcline(607);
            px_index_set(px_get_global("cg_cells"), _v1291, px_int(1LL));
        }
        px_srcline(608);
        _v1293 = px_str("px_null()");
        px_srcline(609);
        if (px_is_truthy(px_ne(px_index(_v1290, px_int(3LL)), px_null()))) {
            px_srcline(610);
             _v1293 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1290, px_int(3LL))}, 1);
        }
        px_srcline(611);
        (void)(px_method(_v1289, "append", (LXValue[]){px_list_n((LXValue[]){_v1292, _v1291, px_add(px_add(px_add(px_add(px_add(px_str("(nargs > "), px_call(px_get_global("str"), (LXValue[]){_v1288}, 1)), px_str(") ? args[")), px_call(px_get_global("str"), (LXValue[]){_v1288}, 1)), px_str("] : ")), _v1293)}, 3)}, 1));
        px_srcline(612);
         _v1288 = px_add(_v1288, px_int(1LL));
    }
    px_srcline(615);
    _v1294 = px_list_n((LXValue[]){}, 0);
    px_srcline(616);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1279, px_int(4LL)), _v1294}, 2));
    px_srcline(617);
    _v1295 = px_list_n((LXValue[]){}, 0);
    px_srcline(618);
    _v1296 = px_int(0LL);
    px_srcline(619);
    while (px_is_truthy(px_lt(_v1296, px_call(px_get_global("len"), (LXValue[]){_v1294}, 1)))) {
        px_srcline(620);
        _v1297 = px_index(_v1294, _v1296);
        px_srcline(621);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1297}, 1), px_null()))) {
            px_srcline(622);
             _v1296 = px_add(_v1296, px_int(1LL));
            px_srcline(623);
            continue;
        }
        px_srcline(624);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_globals"), _v1297}, 2))) {
            px_srcline(625);
             _v1296 = px_add(_v1296, px_int(1LL));
            px_srcline(626);
            continue;
        }
        px_srcline(627);
        _v1292 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1297}, 1);
        px_srcline(628);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1286, _v1297}, 2))) {
            px_srcline(629);
            px_index_set(px_get_global("cg_cells"), _v1297, px_int(1LL));
        }
        px_srcline(630);
        (void)(px_method(_v1295, "append", (LXValue[]){px_list_n((LXValue[]){_v1292, _v1297}, 2)}, 1));
        px_srcline(631);
         _v1296 = px_add(_v1296, px_int(1LL));
    }
    px_srcline(633);
    _v1298 = px_int(0LL);
    px_srcline(634);
    while (px_is_truthy(px_lt(_v1298, px_call(px_get_global("len"), (LXValue[]){_v1289}, 1)))) {
        px_srcline(635);
        _v1299 = px_index(_v1289, _v1298);
        px_srcline(636);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1299, px_int(1LL))}, 1))) {
            px_srcline(637);
             _v1281 = px_add(_v1281, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1299, px_int(0LL))), px_str(" = px_cell(")), px_index(_v1299, px_int(2LL))), px_str(");\n")));
        }
        else {
            px_srcline(639);
             _v1281 = px_add(_v1281, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1299, px_int(0LL))), px_str(" = ")), px_index(_v1299, px_int(2LL))), px_str(";\n")));
        }
        px_srcline(640);
         _v1298 = px_add(_v1298, px_int(1LL));
    }
    px_srcline(641);
     _v1298 = px_int(0LL);
    px_srcline(642);
    while (px_is_truthy(px_lt(_v1298, px_call(px_get_global("len"), (LXValue[]){_v1295}, 1)))) {
        px_srcline(643);
        _v1300 = px_index(_v1295, _v1298);
        px_srcline(644);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1300, px_int(1LL))}, 1))) {
            px_srcline(645);
             _v1281 = px_add(_v1281, px_add(px_add(px_str("    LXValue "), px_index(_v1300, px_int(0LL))), px_str(" = px_cell(px_null());\n")));
        }
        else {
            px_srcline(647);
             _v1281 = px_add(_v1281, px_add(px_add(px_str("    LXValue "), px_index(_v1300, px_int(0LL))), px_str(" = px_null();\n")));
        }
        px_srcline(648);
         _v1298 = px_add(_v1298, px_int(1LL));
    }
    px_srcline(650);
    _v1301 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(651);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1301}, 1));
    px_srcline(652);
     _v1281 = px_add(_v1281, px_add(px_add(px_str("    LXValue "), _v1301), px_str("_val = px_null();\n")));
    px_srcline(653);
     _v1281 = px_add(_v1281, px_add(px_add(px_str("    int "), _v1301), px_str("_proped = 0;\n")));
    px_srcline(657);
    _v1302 = px_index(_v1279, px_int(4LL));
    px_srcline(658);
    _v1303 = px_list_n((LXValue[]){}, 0);
    px_srcline(659);
    _v1304 = px_int(0LL);
    px_srcline(660);
    while (px_is_truthy(px_lt(_v1304, px_call(px_get_global("len"), (LXValue[]){_v1302}, 1)))) {
        px_srcline(661);
        (void)(px_method(_v1303, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1302, _v1304), px_int(1LL)}, 2)}, 1));
        px_srcline(662);
         _v1304 = px_add(_v1304, px_int(1LL));
    }
    px_srcline(663);
     _v1281 = px_add(_v1281, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1303}, 2));
    px_srcline(664);
     _v1281 = px_add(_v1281, px_add(_v1301, px_str(":\n")));
    px_srcline(665);
     _v1281 = px_add(_v1281, px_add(px_add(px_add(px_add(px_str("    if ("), _v1301), px_str("_proped) return ")), _v1301), px_str("_val;\n")));
    px_srcline(666);
     _v1281 = px_add(_v1281, px_str("    return px_null();\n"));
    px_srcline(667);
     _v1281 = px_add(_v1281, px_str("}\n"));
    px_srcline(668);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(669);
    px_set_global("cg_vars", _v1282);
    px_srcline(670);
    px_set_global("cg_var_types", _v1283);
    px_srcline(671);
    px_set_global("cg_cells", _v1284);
    px_srcline(672);
    px_set_global("cg_immutables", _v1285);
    px_srcline(673);
    return _v1281;
px_err_1305:
    if (px_err_1305_proped) return px_err_1305_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_generate");
    LXValue _v1306 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1307 = px_null();
    LXValue _v1308 = px_null();
    LXValue _v1309 = px_null();
    LXValue _v1310 = px_null();
    LXValue _v1311 = px_null();
    LXValue _v1312 = px_null();
    LXValue _v1313 = px_null();
    LXValue _v1314 = px_null();
    LXValue _v1315 = px_null();
    LXValue _v1316 = px_null();
    LXValue _v1317 = px_null();
    LXValue _v1318 = px_null();
    LXValue _v1319 = px_null();
    LXValue _v1320 = px_null();
    LXValue _v1321 = px_null();
    LXValue _v1322 = px_null();
    LXValue _v1323 = px_null();
    LXValue _v1324 = px_null();
    LXValue _v1325 = px_null();
    LXValue _v1326 = px_null();
    LXValue _v1327 = px_null();
    LXValue _v1328 = px_null();
    LXValue _v1329 = px_null();
    LXValue _v1330 = px_null();
    LXValue _v1331 = px_null();
    LXValue _v1332 = px_null();
    LXValue _v1333 = px_null();
    LXValue _v1334 = px_null();
    LXValue _v1335 = px_null();
    LXValue _v1336 = px_null();
    LXValue _v1337 = px_null();
    LXValue _v1338 = px_null();
    LXValue _v1339 = px_null();
    LXValue _v1340 = px_null();
    LXValue _v1341 = px_null();
    LXValue px_err_1342_val = px_null();
    int px_err_1342_proped = 0;
    px_srcline(676);
    _v1307 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_srcline(677);
    px_set_global("cg_closures", px_str(""));
    px_srcline(678);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(679);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(680);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(681);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(682);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(683);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(684);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(685);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(686);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(687);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(688);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(689);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(690);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(691);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(692);
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1306}, 1));
    px_srcline(694);
    _v1308 = px_index(_v1306, px_int(1LL));
    px_srcline(695);
    _v1309 = px_int(0LL);
    px_srcline(696);
    while (px_is_truthy(px_lt(_v1309, px_call(px_get_global("len"), (LXValue[]){_v1308}, 1)))) {
        px_srcline(697);
        _v1310 = px_index(_v1308, _v1309);
        px_srcline(698);
        _v1311 = px_index(_v1310, px_int(0LL));
        px_srcline(699);
        if (px_is_truthy(px_eq(_v1311, px_str("FuncDef")))) {
            px_srcline(700);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1310, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1311, px_str("ExternDef")))) {
            px_srcline(703);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1310, px_int(1LL))}, 1), px_index(_v1310, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1311, px_str("VarDecl")))) {
            px_srcline(705);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1310, px_int(2LL))}, 1)}, 1));
            px_srcline(706);
            if (px_is_truthy(({ LXValue _t1343 = px_eq(px_index(_v1310, px_int(1LL)), px_str("Let")); px_is_truthy(_t1343) ? _t1343 : px_eq(px_index(_v1310, px_int(1LL)), px_str("Const")); }))) {
                px_srcline(707);
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1310, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1311, px_str("Assign")))) {
            px_srcline(709);
            _v1312 = px_index(_v1310, px_int(1LL));
            px_srcline(710);
            if (px_is_truthy(px_eq(px_index(_v1312, px_int(0LL)), px_str("Var")))) {
                px_srcline(711);
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1312, px_int(1LL))}, 1)}, 1));
            }
        }
        px_srcline(712);
         _v1309 = px_add(_v1309, px_int(1LL));
    }
    px_srcline(714);
    _v1313 = px_list_n((LXValue[]){}, 0);
    px_srcline(715);
    _v1314 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    px_srcline(716);
    _v1315 = px_int(0LL);
    px_srcline(717);
    while (px_is_truthy(px_lt(_v1315, px_call(px_get_global("len"), (LXValue[]){_v1314}, 1)))) {
        px_srcline(718);
        _v1316 = px_index(_v1314, _v1315);
        px_srcline(719);
        _v1317 = px_index(px_get_global("cg_impls"), _v1316);
        px_srcline(720);
        _v1318 = px_int(0LL);
        px_srcline(721);
        while (px_is_truthy(px_lt(_v1318, px_call(px_get_global("len"), (LXValue[]){_v1317}, 1)))) {
            px_srcline(722);
            (void)(px_method(_v1313, "append", (LXValue[]){px_list_n((LXValue[]){_v1316, px_index(_v1317, _v1318)}, 2)}, 1));
            px_srcline(723);
             _v1318 = px_add(_v1318, px_int(1LL));
        }
        px_srcline(724);
         _v1315 = px_add(_v1315, px_int(1LL));
    }
    px_srcline(726);
    _v1319 = px_int(1LL);
    px_srcline(727);
    while (px_is_truthy(px_lt(_v1319, px_call(px_get_global("len"), (LXValue[]){_v1313}, 1)))) {
        px_srcline(728);
        _v1320 = _v1319;
        px_srcline(729);
        while (px_is_truthy(px_gt(_v1320, px_int(0LL)))) {
            px_srcline(730);
            _v1321 = px_add(px_add(px_index(px_index(_v1313, px_sub(_v1320, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1313, px_sub(_v1320, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(731);
            _v1322 = px_add(px_add(px_index(px_index(_v1313, _v1320), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1313, _v1320), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(732);
            if (px_is_truthy(px_lt(_v1322, _v1321))) {
                px_srcline(733);
                _v1323 = px_index(_v1313, _v1320);
                px_srcline(734);
                px_index_set(_v1313, _v1320, px_index(_v1313, px_sub(_v1320, px_int(1LL))));
                px_srcline(735);
                px_index_set(_v1313, px_sub(_v1320, px_int(1LL)), _v1323);
            }
            px_srcline(736);
             _v1320 = px_sub(_v1320, px_int(1LL));
        }
        px_srcline(737);
         _v1319 = px_add(_v1319, px_int(1LL));
    }
    px_srcline(742);
    _v1324 = px_list_n((LXValue[]){}, 0);
    px_srcline(743);
    _v1325 = px_int(0LL);
    px_srcline(744);
    while (px_is_truthy(px_lt(_v1325, px_call(px_get_global("len"), (LXValue[]){_v1313}, 1)))) {
        px_srcline(745);
        _v1316 = px_index(px_index(_v1313, _v1325), px_int(0LL));
        px_srcline(746);
        _v1326 = px_index(px_index(_v1313, _v1325), px_int(1LL));
        px_srcline(747);
        _v1327 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1316}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1326, px_int(1LL))}, 1)}, 1));
        px_srcline(748);
        (void)(px_method(_v1324, "push", (LXValue[]){px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1326, _v1327}, 2)}, 1));
        px_srcline(749);
        (void)(px_method(_v1324, "push", (LXValue[]){px_str("\n")}, 1));
        px_srcline(750);
         _v1325 = px_add(_v1325, px_int(1LL));
    }
    px_srcline(752);
    _v1328 = px_int(0LL);
    px_srcline(753);
    while (px_is_truthy(px_lt(_v1328, px_call(px_get_global("len"), (LXValue[]){_v1308}, 1)))) {
        px_srcline(754);
        _v1310 = px_index(_v1308, _v1328);
        px_srcline(755);
        if (px_is_truthy(px_eq(px_index(_v1310, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(756);
            (void)(px_method(_v1324, "push", (LXValue[]){px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1310}, 1)}, 1));
            px_srcline(757);
            (void)(px_method(_v1324, "push", (LXValue[]){px_str("\n")}, 1));
        }
        px_srcline(758);
         _v1328 = px_add(_v1328, px_int(1LL));
    }
    px_srcline(759);
     _v1307 = px_add(_v1307, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1324}, 2));
    px_srcline(761);
     _v1307 = px_add(_v1307, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(762);
     _v1307 = px_add(_v1307, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(763);
     _v1307 = px_add(_v1307, px_str("    px_register_builtins();\n"));
    px_srcline(765);
    _v1329 = px_int(0LL);
    px_srcline(766);
    while (px_is_truthy(px_lt(_v1329, px_call(px_get_global("len"), (LXValue[]){_v1308}, 1)))) {
        px_srcline(767);
        _v1310 = px_index(_v1308, _v1329);
        px_srcline(768);
        if (px_is_truthy(px_eq(px_index(_v1310, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(769);
            _v1327 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1310, px_int(1LL))}, 1);
            px_srcline(770);
            _v1330 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1327}, 1));
            px_srcline(771);
             _v1307 = px_add(_v1307, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1327), px_str("\", px_func(\"")), _v1327), px_str("\", ")), _v1330), px_str(", NULL));\n")));
        }
        px_srcline(772);
         _v1329 = px_add(_v1329, px_int(1LL));
    }
    px_srcline(774);
    _v1331 = px_int(0LL);
    px_srcline(775);
    while (px_is_truthy(px_lt(_v1331, px_call(px_get_global("len"), (LXValue[]){_v1313}, 1)))) {
        px_srcline(776);
        _v1316 = px_index(px_index(_v1313, _v1331), px_int(0LL));
        px_srcline(777);
        _v1326 = px_index(px_index(_v1313, _v1331), px_int(1LL));
        px_srcline(778);
        _v1332 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1326, px_int(1LL))}, 1);
        px_srcline(779);
        _v1327 = px_add(px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1316}, 1)), px_str("_")), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1332}, 1));
        px_srcline(780);
         _v1307 = px_add(_v1307, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1316), px_str(".")), _v1332), px_str("\", px_func(\"")), _v1316), px_str(".")), _v1332), px_str("\", ")), _v1327), px_str(", NULL));\n")));
        px_srcline(781);
         _v1331 = px_add(_v1331, px_int(1LL));
    }
    px_srcline(784);
    _v1333 = px_list_n((LXValue[]){}, 0);
    px_srcline(785);
    _v1334 = px_int(0LL);
    px_srcline(786);
    while (px_is_truthy(px_lt(_v1334, px_call(px_get_global("len"), (LXValue[]){_v1308}, 1)))) {
        px_srcline(787);
        _v1310 = px_index(_v1308, _v1334);
        px_srcline(788);
        _v1311 = px_index(_v1310, px_int(0LL));
        px_srcline(789);
        if (px_is_truthy(({ LXValue _t1349 = ({ LXValue _t1348 = ({ LXValue _t1347 = ({ LXValue _t1346 = ({ LXValue _t1345 = ({ LXValue _t1344 = px_ne(_v1311, px_str("FuncDef")); px_is_truthy(_t1344) ? px_ne(_v1311, px_str("StructDef")) : _t1344; }); px_is_truthy(_t1345) ? px_ne(_v1311, px_str("EnumDef")) : _t1345; }); px_is_truthy(_t1346) ? px_ne(_v1311, px_str("TraitDef")) : _t1346; }); px_is_truthy(_t1347) ? px_ne(_v1311, px_str("ImplDef")) : _t1347; }); px_is_truthy(_t1348) ? px_ne(_v1311, px_str("Import")) : _t1348; }); px_is_truthy(_t1349) ? px_ne(_v1311, px_str("ExternDef")) : _t1349; }))) {
            px_srcline(790);
            (void)(px_method(_v1333, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1310, px_int(1LL)}, 2)}, 1));
        }
        px_srcline(791);
         _v1334 = px_add(_v1334, px_int(1LL));
    }
    px_srcline(792);
     _v1307 = px_add(_v1307, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1333}, 2));
    px_srcline(794);
    _v1335 = px_bool(false);
    px_srcline(795);
    _v1336 = px_int(0LL);
    px_srcline(796);
    while (px_is_truthy(px_lt(_v1336, px_call(px_get_global("len"), (LXValue[]){_v1308}, 1)))) {
        px_srcline(797);
        _v1310 = px_index(_v1308, _v1336);
        px_srcline(798);
        if (px_is_truthy(({ LXValue _t1350 = px_eq(px_index(_v1310, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1350) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1310, px_int(1LL))}, 1), px_str("main")) : _t1350; }))) {
            px_srcline(799);
             _v1335 = px_bool(true);
            px_srcline(800);
            break;
        }
        px_srcline(801);
         _v1336 = px_add(_v1336, px_int(1LL));
    }
    px_srcline(802);
    if (px_is_truthy(_v1335)) {
        px_srcline(803);
        _v1330 = px_str("fn_main");
        px_srcline(804);
         _v1307 = px_add(_v1307, px_add(px_add(px_str("    { LXValue _r = "), _v1330), px_str("(NULL, 0, NULL); int _code = 0;\n")));
        px_srcline(805);
         _v1307 = px_add(_v1307, px_str("      if (px_is_result(_r)) {\n"));
        px_srcline(806);
         _v1307 = px_add(_v1307, px_str("        if (!px_result_ok(_r)) {\n"));
        px_srcline(807);
         _v1307 = px_add(_v1307, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
        px_srcline(808);
         _v1307 = px_add(_v1307, px_str("          _code = 1;\n"));
        px_srcline(809);
         _v1307 = px_add(_v1307, px_str("        } else {\n"));
        px_srcline(810);
         _v1307 = px_add(_v1307, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
        px_srcline(811);
         _v1307 = px_add(_v1307, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
        px_srcline(812);
         _v1307 = px_add(_v1307, px_str("        }\n"));
        px_srcline(813);
         _v1307 = px_add(_v1307, px_str("      } else if (_r.type == PX_INT) {\n"));
        px_srcline(814);
         _v1307 = px_add(_v1307, px_str("        _code = (int)_r.as.i;\n"));
        px_srcline(815);
         _v1307 = px_add(_v1307, px_str("      }\n"));
        px_srcline(816);
         _v1307 = px_add(_v1307, px_str("      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）\n"));
        px_srcline(817);
         _v1307 = px_add(_v1307, px_str("    }\n"));
    }
    else {
        px_srcline(819);
         _v1307 = px_add(_v1307, px_str("    return px_exit_code_final(0);   // M120（qg-issue 76 E1）\n"));
    }
    px_srcline(820);
     _v1307 = px_add(_v1307, px_str("}\n"));
    px_srcline(822);
    _v1337 = px_call(px_get_global("cg_find"), (LXValue[]){_v1307, px_str("int main(")}, 2);
    px_srcline(823);
    if (px_is_truthy(px_ge(_v1337, px_int(0LL)))) {
        px_srcline(824);
        _v1338 = px_slice(_v1307, px_int(0LL), _v1337, px_null());
        px_srcline(825);
        _v1339 = px_slice(_v1307, _v1337, px_call(px_get_global("len"), (LXValue[]){_v1307}, 1), px_null());
        px_srcline(826);
        _v1340 = px_call(px_get_global("cg_find"), (LXValue[]){_v1338, px_str("static LXValue")}, 2);
        px_srcline(827);
        _v1341 = px_str("");
        px_srcline(828);
        if (px_is_truthy(px_ge(_v1340, px_int(0LL)))) {
            px_srcline(829);
             _v1341 = px_add(px_add(px_add(px_add(px_slice(_v1338, px_int(0LL), _v1340, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1338, _v1340, px_call(px_get_global("len"), (LXValue[]){_v1338}, 1), px_null())), _v1339);
        }
        else {
            px_srcline(831);
             _v1341 = px_add(px_add(px_add(_v1338, px_get_global("cg_closures")), px_str("\n")), _v1339);
        }
        px_srcline(832);
        return _v1341;
    }
    px_srcline(833);
    return _v1307;
px_err_1342:
    if (px_err_1342_proped) return px_err_1342_val;
    return px_null();
}

static LXValue fn_bc_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_dict");
    LXValue _v1351 = px_null();
    LXValue px_err_1352_val = px_null();
    int px_err_1352_proped = 0;
    px_srcline(37);
    _v1351 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    px_srcline(38);
    (void)(px_method(_v1351, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(39);
    return _v1351;
px_err_1352:
    if (px_err_1352_proped) return px_err_1352_val;
    return px_null();
}

static LXValue fn_bc_k_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_find");
    LXValue _v1353 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1354 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1355 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1356 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1357 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1358 = px_null();
    LXValue _v1359 = px_null();
    LXValue px_err_1360_val = px_null();
    int px_err_1360_proped = 0;
    px_srcline(43);
    _v1358 = px_int(0LL);
    px_srcline(44);
    while (px_is_truthy(px_lt(_v1358, px_call(px_get_global("len"), (LXValue[]){_v1353}, 1)))) {
        px_srcline(45);
        _v1359 = px_index(_v1353, _v1358);
        px_srcline(46);
        if (px_is_truthy(px_eq(px_index(_v1359, px_str("kind")), _v1354))) {
            px_srcline(47);
            if (px_is_truthy(px_eq(_v1354, px_str("int")))) {
                px_srcline(48);
                if (px_is_truthy(px_eq(px_index(_v1359, px_str("i")), _v1355))) {
                    px_srcline(49);
                    return _v1358;
                }
            }
            else if (px_is_truthy(px_eq(_v1354, px_str("float")))) {
                px_srcline(55);
                if (px_is_truthy(px_eq(px_call(px_get_global("float64_bits"), (LXValue[]){px_index(_v1359, px_str("f"))}, 1), px_call(px_get_global("float64_bits"), (LXValue[]){_v1356}, 1)))) {
                    px_srcline(56);
                    return _v1358;
                }
            }
            else if (px_is_truthy(px_eq(_v1354, px_str("str")))) {
                px_srcline(58);
                if (px_is_truthy(px_eq(px_index(_v1359, px_str("s")), _v1357))) {
                    px_srcline(59);
                    return _v1358;
                }
            }
            else {
                px_srcline(62);
                if (px_is_truthy(px_eq(px_index(_v1359, px_str("i")), _v1355))) {
                    px_srcline(63);
                    return _v1358;
                }
            }
        }
        px_srcline(64);
         _v1358 = px_add(_v1358, px_int(1LL));
    }
    px_srcline(65);
    return px_neg(px_int(1LL));
px_err_1360:
    if (px_err_1360_proped) return px_err_1360_val;
    return px_null();
}

static LXValue fn_bc_k_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_add");
    LXValue _v1361 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1362 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1363 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1364 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1365 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1366 = px_null();
    LXValue _v1367 = px_null();
    LXValue px_err_1368_val = px_null();
    int px_err_1368_proped = 0;
    px_srcline(67);
    _v1366 = px_call(px_get_global("bc_k_find"), (LXValue[]){_v1361, _v1362, _v1363, _v1364, _v1365}, 5);
    px_srcline(68);
    if (px_is_truthy(px_ge(_v1366, px_int(0LL)))) {
        px_srcline(69);
        return _v1366;
    }
    px_srcline(70);
    _v1367 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(71);
    px_index_set(_v1367, px_str("kind"), _v1362);
    px_srcline(72);
    px_index_set(_v1367, px_str("i"), _v1363);
    px_srcline(73);
    px_index_set(_v1367, px_str("f"), _v1364);
    px_srcline(74);
    px_index_set(_v1367, px_str("s"), _v1365);
    px_srcline(75);
    (void)(px_method(_v1361, "push", (LXValue[]){_v1367}, 1));
    px_srcline(76);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1361}, 1), px_int(1LL));
px_err_1368:
    if (px_err_1368_proped) return px_err_1368_val;
    return px_null();
}

static LXValue fn_bc_n_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_n_add");
    LXValue _v1369 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1370 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1371 = px_null();
    LXValue px_err_1372_val = px_null();
    int px_err_1372_proped = 0;
    px_srcline(79);
    _v1371 = px_int(0LL);
    px_srcline(80);
    while (px_is_truthy(px_lt(_v1371, px_call(px_get_global("len"), (LXValue[]){_v1369}, 1)))) {
        px_srcline(81);
        if (px_is_truthy(px_eq(px_index(_v1369, _v1371), _v1370))) {
            px_srcline(82);
            return _v1371;
        }
        px_srcline(83);
         _v1371 = px_add(_v1371, px_int(1LL));
    }
    px_srcline(84);
    (void)(px_method(_v1369, "push", (LXValue[]){_v1370}, 1));
    px_srcline(85);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1369}, 1), px_int(1LL));
px_err_1372:
    if (px_err_1372_proped) return px_err_1372_val;
    return px_null();
}

static LXValue fn_bc_g_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_g_add");
    LXValue _v1373 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1374 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1375 = px_null();
    LXValue px_err_1376_val = px_null();
    int px_err_1376_proped = 0;
    px_srcline(88);
    _v1375 = px_int(0LL);
    px_srcline(89);
    while (px_is_truthy(px_lt(_v1375, px_call(px_get_global("len"), (LXValue[]){_v1373}, 1)))) {
        px_srcline(90);
        if (px_is_truthy(px_eq(px_index(_v1373, _v1375), _v1374))) {
            px_srcline(91);
            return _v1375;
        }
        px_srcline(92);
         _v1375 = px_add(_v1375, px_int(1LL));
    }
    px_srcline(93);
    (void)(px_method(_v1373, "push", (LXValue[]){_v1374}, 1));
    px_srcline(94);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1373}, 1), px_int(1LL));
px_err_1376:
    if (px_err_1376_proped) return px_err_1376_val;
    return px_null();
}

static LXValue fn_bc_is_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_is_global");
    LXValue _v1377 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1378_val = px_null();
    int px_err_1378_proped = 0;
    px_srcline(97);
    if (px_is_truthy(px_eq(px_get_global("g_bcm"), px_null()))) {
        px_srcline(98);
        return px_bool(false);
    }
    px_srcline(99);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1377}, 2);
px_err_1378:
    if (px_err_1378_proped) return px_err_1378_val;
    return px_null();
}

static LXValue fn_bc_new_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_module");
    LXValue _v1379 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1380 = px_null();
    LXValue px_err_1381_val = px_null();
    int px_err_1381_proped = 0;
    px_srcline(102);
    _v1380 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(103);
    px_index_set(_v1380, px_str("name"), _v1379);
    px_srcline(104);
    px_index_set(_v1380, px_str("k_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(105);
    px_index_set(_v1380, px_str("n_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(106);
    px_index_set(_v1380, px_str("globals"), px_list_n((LXValue[]){}, 0));
    px_srcline(107);
    px_index_set(_v1380, px_str("funcs"), px_list_n((LXValue[]){}, 0));
    px_srcline(108);
    px_index_set(_v1380, px_str("top"), px_neg(px_int(1LL)));
    px_srcline(111);
    px_index_set(_v1380, px_str("structs"), px_list_n((LXValue[]){}, 0));
    px_srcline(112);
    px_index_set(_v1380, px_str("enums"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(113);
    px_index_set(_v1380, px_str("consts"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(114);
    px_index_set(_v1380, px_str("nclosure"), px_int(0LL));
    px_srcline(115);
    return _v1380;
px_err_1381:
    if (px_err_1381_proped) return px_err_1381_val;
    return px_null();
}

static LXValue fn_bc_struct_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_struct_index");
    LXValue _v1382 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1383 = px_null();
    LXValue px_err_1384_val = px_null();
    int px_err_1384_proped = 0;
    px_srcline(118);
    _v1383 = px_int(0LL);
    px_srcline(119);
    while (px_is_truthy(px_lt(_v1383, px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("structs"))}, 1)))) {
        px_srcline(120);
        if (px_is_truthy(px_eq(px_index(px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1383), px_str("name")), _v1382))) {
            px_srcline(121);
            return _v1383;
        }
        px_srcline(122);
         _v1383 = px_add(_v1383, px_int(1LL));
    }
    px_srcline(123);
    return px_neg(px_int(1LL));
px_err_1384:
    if (px_err_1384_proped) return px_err_1384_val;
    return px_null();
}

static LXValue fn_bc_enum_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_enum_has");
    LXValue _v1385 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1386 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1387_val = px_null();
    int px_err_1387_proped = 0;
    px_srcline(126);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1385}, 1)))) {
        px_srcline(127);
        return px_bool(false);
    }
    px_srcline(128);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_index(px_get_global("g_bcm"), px_str("enums")), _v1385), _v1386}, 2);
px_err_1387:
    if (px_err_1387_proped) return px_err_1387_val;
    return px_null();
}

static LXValue fn_bc_const_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_const_find");
    LXValue _v1388 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1389 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1390 = px_null();
    LXValue _v1391 = px_null();
    LXValue px_err_1392_val = px_null();
    int px_err_1392_proped = 0;
    px_srcline(131);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1388}, 1)))) {
        px_srcline(132);
        return px_null();
    }
    px_srcline(133);
    _v1390 = px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1388);
    px_srcline(134);
    _v1391 = px_int(0LL);
    px_srcline(135);
    while (px_is_truthy(px_lt(_v1391, px_call(px_get_global("len"), (LXValue[]){_v1390}, 1)))) {
        px_srcline(136);
        if (px_is_truthy(px_eq(px_index(px_index(_v1390, _v1391), px_str("n")), _v1389))) {
            px_srcline(137);
            return px_index(px_index(_v1390, _v1391), px_str("e"));
        }
        px_srcline(138);
         _v1391 = px_add(_v1391, px_int(1LL));
    }
    px_srcline(139);
    return px_null();
px_err_1392:
    if (px_err_1392_proped) return px_err_1392_val;
    return px_null();
}

static LXValue fn_bc_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_consts");
    LXValue _v1393 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1394 = px_null();
    LXValue _v1395 = px_null();
    LXValue _v1396 = px_null();
    LXValue _v1397 = px_null();
    LXValue _v1398 = px_null();
    LXValue _v1399 = px_null();
    LXValue _v1400 = px_null();
    LXValue _v1401 = px_null();
    LXValue _v1402 = px_null();
    LXValue _v1403 = px_null();
    LXValue px_err_1404_val = px_null();
    int px_err_1404_proped = 0;
    px_srcline(142);
    _v1394 = px_int(0LL);
    px_srcline(143);
    while (px_is_truthy(px_lt(_v1394, px_call(px_get_global("len"), (LXValue[]){_v1393}, 1)))) {
        px_srcline(144);
        _v1395 = px_index(_v1393, _v1394);
        px_srcline(145);
        _v1396 = px_index(_v1395, px_int(0LL));
        px_srcline(146);
        if (px_is_truthy(px_eq(_v1396, px_str("TypeConst")))) {
            px_srcline(147);
            _v1397 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1395, px_int(1LL))}, 1);
            px_srcline(148);
            if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1397}, 1)))) {
                px_srcline(149);
                px_index_set(px_index(px_get_global("g_bcm"), px_str("consts")), _v1397, px_list_n((LXValue[]){}, 0));
            }
            px_srcline(150);
            _v1398 = px_int(0LL);
            px_srcline(151);
            while (px_is_truthy(px_lt(_v1398, px_call(px_get_global("len"), (LXValue[]){px_index(_v1395, px_int(2LL))}, 1)))) {
                px_srcline(152);
                _v1399 = px_index(px_index(_v1395, px_int(2LL)), _v1398);
                px_srcline(153);
                _v1400 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
                px_srcline(154);
                px_index_set(_v1400, px_str("n"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1399, px_int(1LL))}, 1));
                px_srcline(155);
                px_index_set(_v1400, px_str("e"), px_index(_v1399, px_int(2LL)));
                px_srcline(156);
                (void)(px_method(px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1397), "push", (LXValue[]){_v1400}, 1));
                px_srcline(157);
                 _v1398 = px_add(_v1398, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1396, px_str("FuncDef")))) {
            px_srcline(159);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1395, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1396, px_str("ImplDef")))) {
            px_srcline(161);
            _v1401 = px_int(0LL);
            px_srcline(162);
            while (px_is_truthy(px_lt(_v1401, px_call(px_get_global("len"), (LXValue[]){px_index(_v1395, px_int(3LL))}, 1)))) {
                px_srcline(163);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1395, px_int(3LL)), _v1401), px_int(4LL))}, 1));
                px_srcline(164);
                 _v1401 = px_add(_v1401, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1396, px_str("If")))) {
            px_srcline(166);
            _v1402 = px_index(_v1395, px_int(1LL));
            px_srcline(167);
            _v1403 = px_int(0LL);
            px_srcline(168);
            while (px_is_truthy(px_lt(_v1403, px_call(px_get_global("len"), (LXValue[]){_v1402}, 1)))) {
                px_srcline(169);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(_v1402, _v1403), px_int(1LL))}, 1));
                px_srcline(170);
                 _v1403 = px_add(_v1403, px_int(1LL));
            }
            px_srcline(171);
            if (px_is_truthy(px_ne(px_index(_v1395, px_int(2LL)), px_null()))) {
                px_srcline(172);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1395, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1396, px_str("While")))) {
            px_srcline(174);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1395, px_int(2LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1396, px_str("For")))) {
            px_srcline(176);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1395, px_int(3LL))}, 1));
        }
        px_srcline(177);
         _v1394 = px_add(_v1394, px_int(1LL));
    }
px_err_1404:
    if (px_err_1404_proped) return px_err_1404_val;
    return px_null();
}

static LXValue fn_bc_collect_impl_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_impl_list");
    LXValue _v1405 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1406 = px_null();
    LXValue _v1407 = px_null();
    LXValue _v1408 = px_null();
    LXValue _v1409 = px_null();
    LXValue _v1410 = px_null();
    LXValue _v1411 = px_null();
    LXValue _v1412 = px_null();
    LXValue _v1413 = px_null();
    LXValue _v1414 = px_null();
    LXValue _v1415 = px_null();
    LXValue px_err_1416_val = px_null();
    int px_err_1416_proped = 0;
    px_srcline(181);
    _v1406 = px_list_n((LXValue[]){}, 0);
    px_srcline(182);
    _v1407 = px_int(0LL);
    px_srcline(183);
    while (px_is_truthy(px_lt(_v1407, px_call(px_get_global("len"), (LXValue[]){_v1405}, 1)))) {
        px_srcline(184);
        _v1408 = px_index(_v1405, _v1407);
        px_srcline(185);
        if (px_is_truthy(px_eq(px_index(_v1408, px_int(0LL)), px_str("ImplDef")))) {
            px_srcline(186);
            _v1409 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1408, px_int(1LL))}, 1);
            px_srcline(187);
            _v1410 = px_int(0LL);
            px_srcline(188);
            while (px_is_truthy(px_lt(_v1410, px_call(px_get_global("len"), (LXValue[]){px_index(_v1408, px_int(3LL))}, 1)))) {
                px_srcline(189);
                (void)(px_method(_v1406, "append", (LXValue[]){px_list_n((LXValue[]){_v1409, px_index(px_index(_v1408, px_int(3LL)), _v1410)}, 2)}, 1));
                px_srcline(190);
                 _v1410 = px_add(_v1410, px_int(1LL));
            }
        }
        px_srcline(191);
         _v1407 = px_add(_v1407, px_int(1LL));
    }
    px_srcline(193);
    _v1411 = px_int(1LL);
    px_srcline(194);
    while (px_is_truthy(px_lt(_v1411, px_call(px_get_global("len"), (LXValue[]){_v1406}, 1)))) {
        px_srcline(195);
        _v1412 = _v1411;
        px_srcline(196);
        while (px_is_truthy(px_gt(_v1412, px_int(0LL)))) {
            px_srcline(197);
            _v1413 = px_add(px_add(px_index(px_index(_v1406, px_sub(_v1412, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1406, px_sub(_v1412, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(198);
            _v1414 = px_add(px_add(px_index(px_index(_v1406, _v1412), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1406, _v1412), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(199);
            if (px_is_truthy(px_lt(_v1414, _v1413))) {
                px_srcline(200);
                _v1415 = px_index(_v1406, _v1412);
                px_srcline(201);
                px_index_set(_v1406, _v1412, px_index(_v1406, px_sub(_v1412, px_int(1LL))));
                px_srcline(202);
                px_index_set(_v1406, px_sub(_v1412, px_int(1LL)), _v1415);
            }
            px_srcline(203);
             _v1412 = px_sub(_v1412, px_int(1LL));
        }
        px_srcline(204);
         _v1411 = px_add(_v1411, px_int(1LL));
    }
    px_srcline(205);
    return _v1406;
px_err_1416:
    if (px_err_1416_proped) return px_err_1416_val;
    return px_null();
}

static LXValue fn_bc_new_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_func");
    LXValue _v1417 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1418 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1419 = px_null();
    LXValue px_err_1420_val = px_null();
    int px_err_1420_proped = 0;
    px_srcline(210);
    _v1419 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(211);
    px_index_set(_v1419, px_str("name"), _v1417);
    px_srcline(212);
    px_index_set(_v1419, px_str("arity"), _v1418);
    px_srcline(213);
    px_index_set(_v1419, px_str("ndefault"), px_int(0LL));
    px_srcline(214);
    px_index_set(_v1419, px_str("nslots"), _v1418);
    px_srcline(215);
    px_index_set(_v1419, px_str("bc"), px_list_n((LXValue[]){}, 0));
    px_srcline(216);
    px_index_set(_v1419, px_str("smap"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(217);
    px_index_set(_v1419, px_str("next_slot"), _v1418);
    px_srcline(218);
    px_index_set(_v1419, px_str("is_top"), px_bool(false));
    px_srcline(219);
    px_index_set(_v1419, px_str("loops"), px_list_n((LXValue[]){}, 0));
    px_srcline(225);
    px_index_set(_v1419, px_str("upnames"), px_list_n((LXValue[]){}, 0));
    px_srcline(226);
    px_index_set(_v1419, px_str("cell"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(227);
    px_index_set(_v1419, px_str("boxed"), px_list_n((LXValue[]){}, 0));
    px_srcline(228);
    return _v1419;
px_err_1420:
    if (px_err_1420_proped) return px_err_1420_val;
    return px_null();
}

static LXValue fn_bc_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_slot");
    LXValue _v1421 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1422 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1423 = px_null();
    LXValue px_err_1424_val = px_null();
    int px_err_1424_proped = 0;
    px_srcline(231);
    if (px_is_truthy(px_method(px_index(_v1421, px_str("smap")), "has", (LXValue[]){_v1422}, 1))) {
        px_srcline(232);
        return px_index(px_index(_v1421, px_str("smap")), _v1422);
    }
    px_srcline(233);
    _v1423 = px_index(_v1421, px_str("next_slot"));
    px_srcline(234);
    px_index_set(_v1421, px_str("next_slot"), px_add(_v1423, px_int(1LL)));
    px_srcline(235);
    px_index_set(px_index(_v1421, px_str("smap")), _v1422, _v1423);
    px_srcline(236);
    return _v1423;
px_err_1424:
    if (px_err_1424_proped) return px_err_1424_val;
    return px_null();
}

static LXValue fn_bc_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_tmp");
    LXValue _v1425 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1426 = px_null();
    LXValue px_err_1427_val = px_null();
    int px_err_1427_proped = 0;
    px_srcline(239);
    _v1426 = px_index(_v1425, px_str("next_slot"));
    px_srcline(240);
    px_index_set(_v1425, px_str("next_slot"), px_add(_v1426, px_int(1LL)));
    px_srcline(241);
    return _v1426;
px_err_1427:
    if (px_err_1427_proped) return px_err_1427_val;
    return px_null();
}

static LXValue fn_bc_cell_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_has");
    LXValue _v1428 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1429 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1430_val = px_null();
    int px_err_1430_proped = 0;
    px_srcline(246);
    return px_method(px_index(_v1428, px_str("cell")), "has", (LXValue[]){_v1429}, 1);
px_err_1430:
    if (px_err_1430_proped) return px_err_1430_val;
    return px_null();
}

static LXValue fn_bc_cell_mark(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_mark");
    LXValue _v1431 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1432 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1433_val = px_null();
    int px_err_1433_proped = 0;
    px_srcline(248);
    px_index_set(px_index(_v1431, px_str("cell")), _v1432, px_int(1LL));
px_err_1433:
    if (px_err_1433_proped) return px_err_1433_val;
    return px_null();
}

static LXValue fn_bc_load_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_load_var");
    LXValue _v1434 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1435 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1436 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1437 = px_null();
    LXValue _v1438 = px_null();
    LXValue px_err_1439_val = px_null();
    int px_err_1439_proped = 0;
    px_srcline(251);
    _v1437 = px_index(px_index(_v1434, px_str("smap")), _v1435);
    px_srcline(252);
    if (px_is_truthy(px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1434, _v1435}, 2)))) {
        px_srcline(253);
        if (px_is_truthy(({ LXValue _t1440 = px_ge(_v1436, px_int(0LL)); px_is_truthy(_t1440) ? px_ne(_v1436, _v1437) : _t1440; }))) {
            px_srcline(254);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1434, px_str("MOV"), _v1436, _v1437, px_int(0LL)}, 5));
        }
        px_srcline(255);
        return px_null();
    }
    px_srcline(256);
    if (px_is_truthy(px_lt(_v1436, px_int(0LL)))) {
        px_srcline(257);
        return px_null();
    }
    px_srcline(258);
    if (px_is_truthy(px_eq(_v1436, _v1437))) {
        px_srcline(261);
        _v1438 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1434}, 1);
        px_srcline(262);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1434, px_str("CELLGET"), _v1438, _v1437, px_int(0LL)}, 5));
        px_srcline(263);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1434, px_str("MOV"), _v1437, _v1438, px_int(0LL)}, 5));
        px_srcline(264);
        return px_null();
    }
    px_srcline(265);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1434, px_str("CELLGET"), _v1436, _v1437, px_int(0LL)}, 5));
px_err_1439:
    if (px_err_1439_proped) return px_err_1439_val;
    return px_null();
}

static LXValue fn_bc_store_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_store_var");
    LXValue _v1441 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1442 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1443 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1444 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_1445_val = px_null();
    int px_err_1445_proped = 0;
    px_srcline(268);
    if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1441, _v1442}, 2))) {
        px_srcline(269);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1441, px_str("CELLSET"), _v1444, _v1443, px_int(0LL)}, 5));
    }
    else {
        px_srcline(271);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1441, px_str("MOV"), _v1443, _v1444, px_int(0LL)}, 5));
    }
px_err_1445:
    if (px_err_1445_proped) return px_err_1445_val;
    return px_null();
}

static LXValue fn_bc_closure_free(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_closure_free");
    LXValue _v1446 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1447 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1448 = px_null();
    LXValue px_err_1449_val = px_null();
    int px_err_1449_proped = 0;
    px_srcline(275);
    _v1448 = px_list_n((LXValue[]){}, 0);
    px_srcline(276);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v1446, px_null(), _v1447, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v1448}, 2));
    px_srcline(277);
    return _v1448;
px_err_1449:
    if (px_err_1449_proped) return px_err_1449_val;
    return px_null();
}

static LXValue fn_bc_lambda_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_hoist");
    LXValue _v1450 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1451 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1452_val = px_null();
    int px_err_1452_proped = 0;
    px_srcline(280);
    if (px_is_truthy(({ LXValue _t1454 = ({ LXValue _t1453 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1450}, 1), px_str("list")); px_is_truthy(_t1453) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1450}, 1), px_int(0LL)) : _t1453; }); px_is_truthy(_t1454) ? px_eq(px_index(_v1450, px_int(0LL)), px_str("Block")) : _t1454; }))) {
        px_srcline(281);
        (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1450, px_int(1LL)), _v1451}, 2));
    }
px_err_1452:
    if (px_err_1452_proped) return px_err_1452_val;
    return px_null();
}

static LXValue fn_bc_box_frame(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_box_frame");
    LXValue _v1455 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1456 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1457 = px_null();
    LXValue _v1458 = px_null();
    LXValue _v1459 = px_null();
    LXValue px_err_1460_val = px_null();
    int px_err_1460_proped = 0;
    px_srcline(287);
    _v1457 = px_list_n((LXValue[]){}, 0);
    px_srcline(288);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1456, _v1457}, 2));
    px_srcline(289);
    _v1458 = px_int(0LL);
    px_srcline(290);
    while (px_is_truthy(px_lt(_v1458, px_call(px_get_global("len"), (LXValue[]){_v1457}, 1)))) {
        px_srcline(291);
        _v1459 = px_index(_v1457, _v1458);
        px_srcline(292);
        if (px_is_truthy(({ LXValue _t1461 = px_method(px_index(_v1455, px_str("smap")), "has", (LXValue[]){_v1459}, 1); px_is_truthy(_t1461) ? px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1455, _v1459}, 2)) : _t1461; }))) {
            px_srcline(293);
            (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1455, _v1459}, 2));
            px_srcline(294);
            (void)(px_method(px_index(_v1455, px_str("boxed")), "push", (LXValue[]){_v1459}, 1));
            px_srcline(295);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1455, px_str("CELLNEW"), px_index(px_index(_v1455, px_str("smap")), _v1459), px_index(px_index(_v1455, px_str("smap")), _v1459), px_int(0LL)}, 5));
        }
        px_srcline(296);
         _v1458 = px_add(_v1458, px_int(1LL));
    }
px_err_1460:
    if (px_err_1460_proped) return px_err_1460_val;
    return px_null();
}

static LXValue fn_bc_emit_inst(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_inst");
    LXValue _v1462 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1463 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1464 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1465 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1466 = (nargs > 4) ? args[4] : px_null();
    LXValue px_err_1467_val = px_null();
    int px_err_1467_proped = 0;
    px_srcline(302);
    (void)(px_method(px_index(_v1462, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){_v1463, px_int(0LL), _v1464, _v1465, _v1466}, 5)}, 1));
px_err_1467:
    if (px_err_1467_proped) return px_err_1467_val;
    return px_null();
}

static LXValue fn_bc_patch_off(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_patch_off");
    LXValue _v1468 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1469 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1470 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1471_val = px_null();
    int px_err_1471_proped = 0;
    px_srcline(306);
    px_index_set(px_index(px_index(_v1468, px_str("bc")), _v1469), px_int(3LL), px_sub(_v1470, px_add(_v1469, px_int(1LL))));
px_err_1471:
    if (px_err_1471_proped) return px_err_1471_val;
    return px_null();
}

static LXValue fn_bc_collect_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_hoist");
    LXValue _v1472 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1473 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1474 = px_null();
    LXValue _v1475 = px_null();
    LXValue _v1476 = px_null();
    LXValue _v1477 = px_null();
    LXValue _v1478 = px_null();
    LXValue _v1479 = px_null();
    LXValue _v1480 = px_null();
    LXValue _v1481 = px_null();
    LXValue _v1482 = px_null();
    LXValue px_err_1483_val = px_null();
    int px_err_1483_proped = 0;
    px_srcline(309);
    _v1474 = px_int(0LL);
    px_srcline(310);
    while (px_is_truthy(px_lt(_v1474, px_call(px_get_global("len"), (LXValue[]){_v1472}, 1)))) {
        px_srcline(311);
        _v1475 = px_index(_v1472, _v1474);
        px_srcline(312);
        _v1476 = px_index(_v1475, px_int(0LL));
        px_srcline(313);
        if (px_is_truthy(px_eq(_v1476, px_str("Assign")))) {
            px_srcline(314);
            _v1477 = px_index(_v1475, px_int(1LL));
            px_srcline(315);
            if (px_is_truthy(px_eq(px_index(_v1477, px_int(0LL)), px_str("Var")))) {
                px_srcline(316);
                _v1478 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1477, px_int(1LL))}, 1);
                px_srcline(317);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1473, _v1478}, 2)))) {
                    px_srcline(318);
                    (void)(px_method(_v1473, "append", (LXValue[]){_v1478}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1476, px_str("VarDecl")))) {
            px_srcline(320);
            _v1478 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1475, px_int(2LL))}, 1);
            px_srcline(321);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1473, _v1478}, 2)))) {
                px_srcline(322);
                (void)(px_method(_v1473, "append", (LXValue[]){_v1478}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1476, px_str("FuncDef")))) {
            px_srcline(326);
            _v1478 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1475, px_int(1LL))}, 1);
            px_srcline(327);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1473, _v1478}, 2)))) {
                px_srcline(328);
                (void)(px_method(_v1473, "append", (LXValue[]){_v1478}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1476, px_str("If")))) {
            px_srcline(330);
            _v1479 = px_index(_v1475, px_int(1LL));
            px_srcline(331);
            _v1480 = px_int(0LL);
            px_srcline(332);
            while (px_is_truthy(px_lt(_v1480, px_call(px_get_global("len"), (LXValue[]){_v1479}, 1)))) {
                px_srcline(333);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(px_index(_v1479, _v1480), px_int(1LL)), _v1473}, 2));
                px_srcline(334);
                 _v1480 = px_add(_v1480, px_int(1LL));
            }
            px_srcline(335);
            if (px_is_truthy(px_ne(px_index(_v1475, px_int(2LL)), px_null()))) {
                px_srcline(336);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1475, px_int(2LL)), _v1473}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1476, px_str("While")))) {
            px_srcline(338);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1475, px_int(2LL)), _v1473}, 2));
        }
        else if (px_is_truthy(px_eq(_v1476, px_str("For")))) {
            px_srcline(341);
            _v1481 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1475, px_int(1LL))}, 1);
            px_srcline(342);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1473, _v1481}, 2)))) {
                px_srcline(343);
                (void)(px_method(_v1473, "append", (LXValue[]){_v1481}, 1));
            }
            px_srcline(344);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1475, px_int(3LL)), _v1473}, 2));
        }
        else if (px_is_truthy(px_eq(_v1476, px_str("ChanDecl")))) {
            px_srcline(347);
            _v1482 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1475, px_int(1LL))}, 1);
            px_srcline(348);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1473, _v1482}, 2)))) {
                px_srcline(349);
                (void)(px_method(_v1473, "append", (LXValue[]){_v1482}, 1));
            }
        }
        px_srcline(350);
         _v1474 = px_add(_v1474, px_int(1LL));
    }
px_err_1483:
    if (px_err_1483_proped) return px_err_1483_val;
    return px_null();
}

static LXValue fn_bc_emit_null(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_null");
    LXValue _v1484 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1485 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1486 = px_null();
    LXValue px_err_1487_val = px_null();
    int px_err_1487_proped = 0;
    px_srcline(353);
    _v1486 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(354);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1484, px_str("LOADK"), _v1485, _v1486, px_int(0LL)}, 5));
px_err_1487:
    if (px_err_1487_proped) return px_err_1487_val;
    return px_null();
}

static LXValue fn_bc_emit_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_int");
    LXValue _v1488 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1489 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1490 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1491 = px_null();
    LXValue px_err_1492_val = px_null();
    int px_err_1492_proped = 0;
    px_srcline(357);
    if (px_is_truthy(({ LXValue _t1493 = px_ge(_v1490, px_neg(px_int(32768LL))); px_is_truthy(_t1493) ? px_le(_v1490, px_int(32767LL)) : _t1493; }))) {
        px_srcline(358);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1488, px_str("IMM"), _v1489, _v1490, px_int(0LL)}, 5));
    }
    else {
        px_srcline(360);
        _v1491 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("int"), _v1490, px_float(0), px_str("")}, 5);
        px_srcline(361);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1488, px_str("LOADK"), _v1489, _v1491, px_int(0LL)}, 5));
    }
px_err_1492:
    if (px_err_1492_proped) return px_err_1492_val;
    return px_null();
}

static LXValue fn_bc_neg_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_neg_float");
    LXValue _v1494 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1495_val = px_null();
    int px_err_1495_proped = 0;
    px_srcline(368);
    return px_call(px_get_global("bits_to_float64"), (LXValue[]){px_bitxor(px_call(px_get_global("float64_bits"), (LXValue[]){_v1494}, 1), px_sub(px_sub(px_int(0LL), px_int(9223372036854775807LL)), px_int(1LL)))}, 1);
px_err_1495:
    if (px_err_1495_proped) return px_err_1495_val;
    return px_null();
}

static LXValue fn_bc_emit_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_float");
    LXValue _v1496 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1497 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1498 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1499 = px_null();
    LXValue px_err_1500_val = px_null();
    int px_err_1500_proped = 0;
    px_srcline(371);
    _v1499 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("float"), px_int(0LL), _v1498, px_str("")}, 5);
    px_srcline(372);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1496, px_str("LOADK"), _v1497, _v1499, px_int(0LL)}, 5));
px_err_1500:
    if (px_err_1500_proped) return px_err_1500_val;
    return px_null();
}

static LXValue fn_bc_unop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_unop_op");
    LXValue _v1501 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1502_val = px_null();
    int px_err_1502_proped = 0;
    px_srcline(375);
    if (px_is_truthy(px_eq(_v1501, px_str("Neg")))) {
        px_srcline(376);
        return px_str("NEG");
    }
    px_srcline(377);
    if (px_is_truthy(px_eq(_v1501, px_str("Not")))) {
        px_srcline(378);
        return px_str("NOT");
    }
    px_srcline(379);
    if (px_is_truthy(px_eq(_v1501, px_str("BitNot")))) {
        px_srcline(380);
        return px_str("BITNOT");
    }
    px_srcline(381);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_unop_op 未知一元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1501}, 1))}, 1));
px_err_1502:
    if (px_err_1502_proped) return px_err_1502_val;
    return px_null();
}

static LXValue fn_bc_binop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_binop_op");
    LXValue _v1503 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1504_val = px_null();
    int px_err_1504_proped = 0;
    px_srcline(383);
    if (px_is_truthy(px_eq(_v1503, px_str("Add")))) {
        px_srcline(384);
        return px_str("ADD");
    }
    px_srcline(385);
    if (px_is_truthy(px_eq(_v1503, px_str("Sub")))) {
        px_srcline(386);
        return px_str("SUB");
    }
    px_srcline(387);
    if (px_is_truthy(px_eq(_v1503, px_str("Mul")))) {
        px_srcline(388);
        return px_str("MUL");
    }
    px_srcline(389);
    if (px_is_truthy(px_eq(_v1503, px_str("Div")))) {
        px_srcline(390);
        return px_str("DIV");
    }
    px_srcline(391);
    if (px_is_truthy(px_eq(_v1503, px_str("IntDiv")))) {
        px_srcline(392);
        return px_str("IDIV");
    }
    px_srcline(393);
    if (px_is_truthy(px_eq(_v1503, px_str("Mod")))) {
        px_srcline(394);
        return px_str("MOD");
    }
    px_srcline(395);
    if (px_is_truthy(px_eq(_v1503, px_str("Pow")))) {
        px_srcline(396);
        return px_str("POW");
    }
    px_srcline(397);
    if (px_is_truthy(px_eq(_v1503, px_str("Eq")))) {
        px_srcline(398);
        return px_str("EQ");
    }
    px_srcline(399);
    if (px_is_truthy(px_eq(_v1503, px_str("Ne")))) {
        px_srcline(400);
        return px_str("NE");
    }
    px_srcline(401);
    if (px_is_truthy(px_eq(_v1503, px_str("Lt")))) {
        px_srcline(402);
        return px_str("LT");
    }
    px_srcline(403);
    if (px_is_truthy(px_eq(_v1503, px_str("Le")))) {
        px_srcline(404);
        return px_str("LE");
    }
    px_srcline(405);
    if (px_is_truthy(px_eq(_v1503, px_str("Gt")))) {
        px_srcline(406);
        return px_str("GT");
    }
    px_srcline(407);
    if (px_is_truthy(px_eq(_v1503, px_str("Ge")))) {
        px_srcline(408);
        return px_str("GE");
    }
    px_srcline(409);
    if (px_is_truthy(px_eq(_v1503, px_str("BitAnd")))) {
        px_srcline(410);
        return px_str("BITAND");
    }
    px_srcline(411);
    if (px_is_truthy(px_eq(_v1503, px_str("BitOr")))) {
        px_srcline(412);
        return px_str("BITOR");
    }
    px_srcline(413);
    if (px_is_truthy(px_eq(_v1503, px_str("BitXor")))) {
        px_srcline(414);
        return px_str("BITXOR");
    }
    px_srcline(415);
    if (px_is_truthy(px_eq(_v1503, px_str("Shl")))) {
        px_srcline(416);
        return px_str("SHL");
    }
    px_srcline(417);
    if (px_is_truthy(px_eq(_v1503, px_str("Shr")))) {
        px_srcline(418);
        return px_str("SHR");
    }
    px_srcline(419);
    if (px_is_truthy(px_eq(_v1503, px_str("ShrU")))) {
        px_srcline(420);
        return px_str("SHRU");
    }
    px_srcline(421);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_binop_op 未知二元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1503}, 1))}, 1));
px_err_1504:
    if (px_err_1504_proped) return px_err_1504_val;
    return px_null();
}

static LXValue fn_bc_emit_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_expr");
    LXValue _v1505 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1506 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1507 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1508 = px_null();
    LXValue _v1509 = px_null();
    LXValue _v1510 = px_null();
    LXValue _v1511 = px_null();
    LXValue _v1512 = px_null();
    LXValue _v1513 = px_null();
    LXValue _v1514 = px_null();
    LXValue _v1515 = px_null();
    LXValue _v1516 = px_null();
    LXValue _v1517 = px_null();
    LXValue _v1518 = px_null();
    LXValue _v1519 = px_null();
    LXValue _v1520 = px_null();
    LXValue _v1521 = px_null();
    LXValue _v1522 = px_null();
    LXValue _v1523 = px_null();
    LXValue _v1524 = px_null();
    LXValue _v1525 = px_null();
    LXValue _v1526 = px_null();
    LXValue _v1527 = px_null();
    LXValue _v1528 = px_null();
    LXValue _v1529 = px_null();
    LXValue _v1530 = px_null();
    LXValue _v1531 = px_null();
    LXValue _v1532 = px_null();
    LXValue _v1533 = px_null();
    LXValue _v1534 = px_null();
    LXValue _v1535 = px_null();
    LXValue _v1536 = px_null();
    LXValue _v1537 = px_null();
    LXValue _v1538 = px_null();
    LXValue _v1539 = px_null();
    LXValue _v1540 = px_null();
    LXValue _v1541 = px_null();
    LXValue _v1542 = px_null();
    LXValue _v1543 = px_null();
    LXValue _v1544 = px_null();
    LXValue _v1545 = px_null();
    LXValue _v1546 = px_null();
    LXValue _v1547 = px_null();
    LXValue _v1548 = px_null();
    LXValue _v1549 = px_null();
    LXValue _v1550 = px_null();
    LXValue _v1551 = px_null();
    LXValue _v1552 = px_null();
    LXValue _v1553 = px_null();
    LXValue _v1554 = px_null();
    LXValue _v1555 = px_null();
    LXValue _v1556 = px_null();
    LXValue _v1557 = px_null();
    LXValue _v1558 = px_null();
    LXValue _v1559 = px_null();
    LXValue _v1560 = px_null();
    LXValue _v1561 = px_null();
    LXValue px_err_1562_val = px_null();
    int px_err_1562_proped = 0;
    px_srcline(426);
    _v1508 = px_index(_v1505, px_int(0LL));
    px_srcline(427);
    if (px_is_truthy(px_eq(_v1508, px_str("Int")))) {
        px_srcline(428);
        (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1507, _v1506, px_index(_v1505, px_int(1LL))}, 3));
        px_srcline(429);
        return _v1506;
    }
    px_srcline(430);
    if (px_is_truthy(px_eq(_v1508, px_str("Float")))) {
        px_srcline(431);
        (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1507, _v1506, px_index(_v1505, px_int(1LL))}, 3));
        px_srcline(432);
        return _v1506;
    }
    px_srcline(433);
    if (px_is_truthy(px_eq(_v1508, px_str("Unary")))) {
        px_srcline(436);
        _v1509 = px_index(_v1505, px_int(2LL));
        px_srcline(437);
        _v1510 = px_index(_v1505, px_int(1LL));
        px_srcline(438);
        if (px_is_truthy(({ LXValue _t1563 = px_eq(px_index(_v1509, px_int(0LL)), px_str("Int")); px_is_truthy(_t1563) ? px_eq(_v1510, px_str("Neg")) : _t1563; }))) {
            px_srcline(439);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1507, _v1506, px_sub(px_int(0LL), px_index(_v1509, px_int(1LL)))}, 3));
            px_srcline(440);
            return _v1506;
        }
        px_srcline(441);
        if (px_is_truthy(({ LXValue _t1564 = px_eq(px_index(_v1509, px_int(0LL)), px_str("Float")); px_is_truthy(_t1564) ? px_eq(_v1510, px_str("Neg")) : _t1564; }))) {
            px_srcline(448);
            (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1507, _v1506, px_call(px_get_global("bc_neg_float"), (LXValue[]){px_index(_v1509, px_int(1LL))}, 1)}, 3));
            px_srcline(449);
            return _v1506;
        }
        px_srcline(450);
        _v1511 = _v1506;
        px_srcline(451);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(452);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(453);
        _v1512 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(454);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1509, _v1512, _v1507}, 3));
        px_srcline(455);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_call(px_get_global("bc_unop_op"), (LXValue[]){_v1510}, 1), _v1511, _v1512, px_int(0LL)}, 5));
        px_srcline(456);
        return _v1511;
    }
    px_srcline(457);
    if (px_is_truthy(px_eq(_v1508, px_str("Str")))) {
        px_srcline(458);
        _v1513 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1505, px_int(1LL))}, 1)}, 5);
        px_srcline(459);
        if (px_is_truthy(px_ge(_v1506, px_int(0LL)))) {
            px_srcline(460);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("LOADK"), _v1506, _v1513, px_int(0LL)}, 5));
        }
        px_srcline(461);
        return _v1506;
    }
    px_srcline(462);
    if (px_is_truthy(px_eq(_v1508, px_str("Bool")))) {
        px_srcline(463);
        _v1514 = px_int(0LL);
        px_srcline(464);
        if (px_is_truthy(px_index(_v1505, px_int(1LL)))) {
            px_srcline(465);
             _v1514 = px_int(1LL);
        }
        px_srcline(466);
        _v1513 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("bool"), _v1514, px_float(0), px_str("")}, 5);
        px_srcline(467);
        if (px_is_truthy(px_ge(_v1506, px_int(0LL)))) {
            px_srcline(468);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("LOADK"), _v1506, _v1513, px_int(0LL)}, 5));
        }
        px_srcline(469);
        return _v1506;
    }
    px_srcline(470);
    if (px_is_truthy(px_eq(_v1508, px_str("Null")))) {
        px_srcline(471);
        if (px_is_truthy(px_ge(_v1506, px_int(0LL)))) {
            px_srcline(472);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1507, _v1506}, 2));
        }
        px_srcline(473);
        return _v1506;
    }
    px_srcline(474);
    if (px_is_truthy(px_eq(_v1508, px_str("Var")))) {
        px_srcline(475);
        _v1515 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1505, px_int(1LL))}, 1);
        px_srcline(476);
        if (px_is_truthy(px_method(px_index(_v1507, px_str("smap")), "has", (LXValue[]){_v1515}, 1))) {
            px_srcline(478);
            (void)(px_call(px_get_global("bc_load_var"), (LXValue[]){_v1507, _v1515, _v1506}, 3));
            px_srcline(479);
            return _v1506;
        }
        px_srcline(481);
        _v1516 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1515}, 2);
        px_srcline(482);
        if (px_is_truthy(px_ge(_v1506, px_int(0LL)))) {
            px_srcline(483);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("GETG"), _v1506, _v1516, px_int(0LL)}, 5));
        }
        px_srcline(484);
        return _v1506;
    }
    px_srcline(485);
    if (px_is_truthy(px_eq(_v1508, px_str("Binary")))) {
        px_srcline(487);
        _v1510 = px_index(_v1505, px_int(1LL));
        px_srcline(488);
        _v1511 = _v1506;
        px_srcline(489);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(490);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(491);
        if (px_is_truthy(px_eq(_v1510, px_str("And")))) {
            px_srcline(493);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1511, _v1507}, 3));
            px_srcline(494);
            _v1517 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
            px_srcline(495);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPF"), _v1511, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(496);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(3LL)), _v1511, _v1507}, 3));
            px_srcline(497);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1517, px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1)}, 3));
            px_srcline(498);
            return _v1511;
        }
        px_srcline(499);
        if (px_is_truthy(px_eq(_v1510, px_str("Or")))) {
            px_srcline(501);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1511, _v1507}, 3));
            px_srcline(502);
            _v1517 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
            px_srcline(503);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPT"), _v1511, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(504);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(3LL)), _v1511, _v1507}, 3));
            px_srcline(505);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1517, px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1)}, 3));
            px_srcline(506);
            return _v1511;
        }
        px_srcline(507);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1511, _v1507}, 3));
        px_srcline(508);
        _v1518 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(509);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(3LL)), _v1518, _v1507}, 3));
        px_srcline(510);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_call(px_get_global("bc_binop_op"), (LXValue[]){_v1510}, 1), _v1511, _v1511, _v1518}, 5));
        px_srcline(511);
        return _v1511;
    }
    px_srcline(512);
    if (px_is_truthy(px_eq(_v1508, px_str("NullCoalesce")))) {
        px_srcline(514);
        _v1511 = _v1506;
        px_srcline(515);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(516);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(517);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1511, _v1507}, 3));
        px_srcline(518);
        _v1519 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(519);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1507, _v1519}, 2));
        px_srcline(520);
        _v1520 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(521);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("EQ"), _v1520, _v1511, _v1519}, 5));
        px_srcline(522);
        _v1517 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
        px_srcline(523);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPF"), _v1520, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(524);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1511, _v1507}, 3));
        px_srcline(525);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1517, px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1)}, 3));
        px_srcline(526);
        return _v1511;
    }
    px_srcline(527);
    if (px_is_truthy(px_eq(_v1508, px_str("IfExpr")))) {
        px_srcline(529);
        _v1511 = _v1506;
        px_srcline(530);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(531);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(532);
        _v1521 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(533);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1521, _v1507}, 3));
        px_srcline(534);
        _v1522 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
        px_srcline(535);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPF"), _v1521, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(536);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1511, _v1507}, 3));
        px_srcline(537);
        _v1523 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
        px_srcline(538);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(539);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1522, px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1)}, 3));
        px_srcline(540);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(3LL)), _v1511, _v1507}, 3));
        px_srcline(541);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1523, px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1)}, 3));
        px_srcline(542);
        return _v1511;
    }
    px_srcline(543);
    if (px_is_truthy(px_eq(_v1508, px_str("Try")))) {
        px_srcline(547);
        _v1511 = _v1506;
        px_srcline(548);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(549);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(550);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1511, _v1507}, 3));
        px_srcline(551);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("TRY"), _v1511, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(552);
        return _v1511;
    }
    px_srcline(553);
    if (px_is_truthy(px_eq(_v1508, px_str("ForceUnwrap")))) {
        px_srcline(555);
        _v1511 = _v1506;
        px_srcline(556);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(557);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(558);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1511, _v1507}, 3));
        px_srcline(559);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("FORCE"), _v1511, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(560);
        return _v1511;
    }
    px_srcline(561);
    if (px_is_truthy(({ LXValue _t1565 = px_eq(_v1508, px_str("List")); px_is_truthy(_t1565) ? _t1565 : px_eq(_v1508, px_str("Tuple")); }))) {
        px_srcline(563);
        _v1524 = px_index(_v1505, px_int(1LL));
        px_srcline(564);
        _v1525 = px_call(px_get_global("len"), (LXValue[]){_v1524}, 1);
        px_srcline(565);
        _v1511 = _v1506;
        px_srcline(566);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(567);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(568);
        _v1526 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(569);
        while (px_is_truthy(px_lt(px_index(_v1507, px_str("next_slot")), px_add(_v1526, _v1525)))) {
            px_srcline(570);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1));
        }
        px_srcline(571);
        _v1527 = px_int(0LL);
        px_srcline(572);
        while (px_is_truthy(px_lt(_v1527, _v1525))) {
            px_srcline(573);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1524, _v1527), px_add(_v1526, _v1527), _v1507}, 3));
            px_srcline(574);
             _v1527 = px_add(_v1527, px_int(1LL));
        }
        px_srcline(575);
        if (px_is_truthy(px_eq(_v1508, px_str("List")))) {
            px_srcline(576);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("NEWLIST"), _v1511, _v1526, _v1525}, 5));
        }
        else {
            px_srcline(578);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("NEWTUPLE"), _v1511, _v1526, _v1525}, 5));
        }
        px_srcline(579);
        return _v1511;
    }
    px_srcline(580);
    if (px_is_truthy(px_eq(_v1508, px_str("Dict")))) {
        px_srcline(582);
        _v1528 = px_index(_v1505, px_int(1LL));
        px_srcline(583);
        _v1525 = px_call(px_get_global("len"), (LXValue[]){_v1528}, 1);
        px_srcline(584);
        _v1511 = _v1506;
        px_srcline(585);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(586);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(587);
        _v1526 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(588);
        while (px_is_truthy(px_lt(px_index(_v1507, px_str("next_slot")), px_add(_v1526, px_mul(px_int(2LL), _v1525))))) {
            px_srcline(589);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1));
        }
        px_srcline(590);
        _v1529 = px_int(0LL);
        px_srcline(591);
        while (px_is_truthy(px_lt(_v1529, _v1525))) {
            px_srcline(592);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1528, _v1529), px_int(0LL)), px_add(_v1526, px_mul(px_int(2LL), _v1529)), _v1507}, 3));
            px_srcline(593);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1528, _v1529), px_int(1LL)), px_add(px_add(_v1526, px_mul(px_int(2LL), _v1529)), px_int(1LL)), _v1507}, 3));
            px_srcline(594);
             _v1529 = px_add(_v1529, px_int(1LL));
        }
        px_srcline(595);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("NEWDICT"), _v1511, _v1526, _v1525}, 5));
        px_srcline(596);
        return _v1511;
    }
    px_srcline(597);
    if (px_is_truthy(px_eq(_v1508, px_str("Index")))) {
        px_srcline(599);
        _v1511 = _v1506;
        px_srcline(600);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(601);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(602);
        _v1530 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(603);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1530, _v1507}, 3));
        px_srcline(604);
        _v1531 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(605);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1531, _v1507}, 3));
        px_srcline(606);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("INDEX"), _v1511, _v1530, _v1531}, 5));
        px_srcline(607);
        return _v1511;
    }
    px_srcline(608);
    if (px_is_truthy(px_eq(_v1508, px_str("Slice")))) {
        px_srcline(610);
        _v1511 = _v1506;
        px_srcline(611);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(612);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(613);
        _v1530 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(614);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1530, _v1507}, 3));
        px_srcline(615);
        _v1526 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(616);
        while (px_is_truthy(px_lt(px_index(_v1507, px_str("next_slot")), px_add(_v1526, px_int(3LL))))) {
            px_srcline(617);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1));
        }
        px_srcline(618);
        if (px_is_truthy(px_eq(px_index(_v1505, px_int(2LL)), px_null()))) {
            px_srcline(619);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1507, _v1526}, 2));
        }
        else {
            px_srcline(621);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(2LL)), _v1526, _v1507}, 3));
        }
        px_srcline(622);
        if (px_is_truthy(px_eq(px_index(_v1505, px_int(3LL)), px_null()))) {
            px_srcline(623);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1507, px_add(_v1526, px_int(1LL))}, 2));
        }
        else {
            px_srcline(625);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(3LL)), px_add(_v1526, px_int(1LL)), _v1507}, 3));
        }
        px_srcline(626);
        if (px_is_truthy(px_eq(px_index(_v1505, px_int(4LL)), px_null()))) {
            px_srcline(627);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1507, px_add(_v1526, px_int(2LL))}, 2));
        }
        else {
            px_srcline(629);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(4LL)), px_add(_v1526, px_int(2LL)), _v1507}, 3));
        }
        px_srcline(630);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("SLICE"), _v1511, _v1530, _v1526}, 5));
        px_srcline(631);
        return _v1511;
    }
    px_srcline(632);
    if (px_is_truthy(px_eq(_v1508, px_str("Field")))) {
        px_srcline(636);
        _v1532 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1505, px_int(2LL))}, 1);
        px_srcline(637);
        _v1533 = px_index(_v1505, px_int(1LL));
        px_srcline(638);
        if (px_is_truthy(px_eq(px_index(_v1533, px_int(0LL)), px_str("Var")))) {
            px_srcline(639);
            _v1534 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1533, px_int(1LL))}, 1);
            px_srcline(640);
            _v1535 = px_call(px_get_global("bc_const_find"), (LXValue[]){_v1534, _v1532}, 2);
            px_srcline(641);
            if (px_is_truthy(px_ne(_v1535, px_null()))) {
                px_srcline(642);
                _v1511 = _v1506;
                px_srcline(643);
                if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
                    px_srcline(644);
                     _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
                }
                px_srcline(645);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1535, _v1511, _v1507}, 3));
                px_srcline(646);
                return _v1511;
            }
            px_srcline(647);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1534, _v1532}, 2))) {
                px_srcline(648);
                _v1511 = _v1506;
                px_srcline(649);
                if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
                    px_srcline(650);
                     _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
                }
                px_srcline(651);
                _v1519 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1534}, 2);
                px_srcline(652);
                _v1536 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1532}, 2);
                px_srcline(653);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("NEWENUM"), _v1511, _v1519, _v1536}, 5));
                px_srcline(654);
                return _v1511;
            }
        }
        px_srcline(655);
        _v1511 = _v1506;
        px_srcline(656);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(657);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(658);
        _v1530 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(659);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1530, _v1507}, 3));
        px_srcline(660);
        _v1537 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1532}, 2);
        px_srcline(661);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("GETF"), _v1511, _v1530, _v1537}, 5));
        px_srcline(662);
        return _v1511;
    }
    px_srcline(663);
    if (px_is_truthy(px_eq(_v1508, px_str("OptionalField")))) {
        px_srcline(665);
        _v1511 = _v1506;
        px_srcline(666);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(667);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(668);
        _v1530 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(669);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1530, _v1507}, 3));
        px_srcline(670);
        _v1537 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1505, px_int(2LL))}, 1)}, 2);
        px_srcline(671);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("GETF_OPT"), _v1511, _v1530, _v1537}, 5));
        px_srcline(672);
        return _v1511;
    }
    px_srcline(673);
    if (px_is_truthy(px_eq(_v1508, px_str("Call")))) {
        px_srcline(676);
        _v1538 = px_index(_v1505, px_int(1LL));
        px_srcline(680);
        (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v1538, px_index(_v1505, px_int(2LL))}, 2));
        px_srcline(681);
        if (px_is_truthy(px_eq(px_index(_v1538, px_int(0LL)), px_str("Var")))) {
            px_srcline(682);
            _v1539 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1538, px_int(1LL))}, 1);
            px_srcline(683);
            _v1540 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1539}, 1);
            px_srcline(684);
            if (px_is_truthy(px_ge(_v1540, px_int(0LL)))) {
                px_srcline(685);
                return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1539, _v1540, px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 5);
            }
            px_srcline(686);
            if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1539}, 1))) {
                px_srcline(687);
                return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1539, px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 4);
            }
        }
        px_srcline(688);
        if (px_is_truthy(px_eq(px_index(_v1538, px_int(0LL)), px_str("Field")))) {
            px_srcline(689);
            return px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1538, px_int(1LL)), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1538, px_int(2LL))}, 1), px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 5);
        }
        px_srcline(690);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1538, px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 4);
    }
    px_srcline(691);
    if (px_is_truthy(px_eq(_v1508, px_str("Constructor")))) {
        px_srcline(693);
        _v1539 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1505, px_int(1LL))}, 1);
        px_srcline(694);
        _v1540 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1539}, 1);
        px_srcline(695);
        if (px_is_truthy(px_ge(_v1540, px_int(0LL)))) {
            px_srcline(696);
            return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1539, _v1540, px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 5);
        }
        px_srcline(697);
        if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1539}, 1))) {
            px_srcline(698);
            return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1539, px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 4);
        }
        px_srcline(700);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1505, px_int(1LL)), px_index(_v1505, px_int(2LL)), _v1506, _v1507}, 4);
    }
    px_srcline(701);
    if (px_is_truthy(px_eq(_v1508, px_str("Pipe")))) {
        px_srcline(703);
        _v1541 = px_index(_v1505, px_int(1LL));
        px_srcline(704);
        _v1542 = px_index(_v1505, px_int(2LL));
        px_srcline(705);
        if (px_is_truthy(px_eq(px_index(_v1542, px_int(0LL)), px_str("Call")))) {
            px_srcline(706);
            _v1543 = px_list_n((LXValue[]){}, 0);
            px_srcline(707);
            (void)(px_method(_v1543, "append", (LXValue[]){_v1541}, 1));
            px_srcline(708);
            _v1544 = px_int(0LL);
            px_srcline(709);
            while (px_is_truthy(px_lt(_v1544, px_call(px_get_global("len"), (LXValue[]){px_index(_v1542, px_int(2LL))}, 1)))) {
                px_srcline(710);
                (void)(px_method(_v1543, "append", (LXValue[]){px_index(px_index(_v1542, px_int(2LL)), _v1544)}, 1));
                px_srcline(711);
                 _v1544 = px_add(_v1544, px_int(1LL));
            }
            px_srcline(712);
            return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1542, px_int(1LL)), _v1543, _v1506, _v1507}, 4);
        }
        px_srcline(713);
        _v1543 = px_list_n((LXValue[]){_v1541}, 1);
        px_srcline(714);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1542, _v1543, _v1506, _v1507}, 4);
    }
    px_srcline(715);
    if (px_is_truthy(px_eq(_v1508, px_str("ListComp")))) {
        px_srcline(717);
        _v1545 = px_index(_v1505, px_int(2LL));
        px_srcline(718);
        _v1511 = _v1506;
        px_srcline(719);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(720);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(721);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("NEWLIST"), _v1511, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(722);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1507, px_str("push"), px_list_n((LXValue[]){px_index(_v1505, px_int(1LL))}, 1), px_index(_v1505, px_int(3LL)), _v1511, _v1545, px_int(0LL)}, 7));
        px_srcline(723);
        return _v1511;
    }
    px_srcline(724);
    if (px_is_truthy(px_eq(_v1508, px_str("DictComp")))) {
        px_srcline(726);
        _v1545 = px_index(_v1505, px_int(3LL));
        px_srcline(727);
        _v1511 = _v1506;
        px_srcline(728);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(729);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(730);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("NEWDICT"), _v1511, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(731);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1507, px_str("dict"), px_list_n((LXValue[]){px_index(_v1505, px_int(1LL)), px_index(_v1505, px_int(2LL))}, 2), px_index(_v1505, px_int(4LL)), _v1511, _v1545, px_int(0LL)}, 7));
        px_srcline(732);
        return _v1511;
    }
    px_srcline(733);
    if (px_is_truthy(px_eq(_v1508, px_str("GenExp")))) {
        px_srcline(734);
        return px_call(px_get_global("bc_emit_genexp"), (LXValue[]){_v1505, _v1506, _v1507}, 3);
    }
    px_srcline(735);
    if (px_is_truthy(px_eq(_v1508, px_str("Match")))) {
        px_srcline(738);
        _v1511 = _v1506;
        px_srcline(739);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(740);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(741);
        _v1546 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        px_srcline(742);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1505, px_int(1LL)), _v1546, _v1507}, 3));
        px_srcline(743);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("MOV"), _v1511, _v1546, px_int(0LL)}, 5));
        px_srcline(744);
        _v1547 = px_index(_v1505, px_int(2LL));
        px_srcline(745);
        _v1548 = px_list_n((LXValue[]){}, 0);
        px_srcline(746);
        _v1549 = px_int(0LL);
        px_srcline(747);
        while (px_is_truthy(px_lt(_v1549, px_call(px_get_global("len"), (LXValue[]){_v1547}, 1)))) {
            px_srcline(748);
            _v1550 = px_index(_v1547, _v1549);
            px_srcline(749);
            _v1551 = px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1550, px_int(1LL)), _v1546, _v1507}, 3);
            px_srcline(750);
            _v1552 = px_neg(px_int(1LL));
            px_srcline(751);
            _v1553 = px_neg(px_int(1LL));
            px_srcline(752);
            if (px_is_truthy(px_eq(_v1551, px_null()))) {
                px_srcline(754);
                if (px_is_truthy(px_ne(px_index(_v1550, px_int(2LL)), px_null()))) {
                    px_srcline(755);
                    _v1554 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
                    px_srcline(756);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1550, px_int(2LL)), _v1554, _v1507}, 3));
                    px_srcline(757);
                    _v1555 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
                    px_srcline(758);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPF"), _v1554, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(759);
                     _v1553 = _v1555;
                }
            }
            else {
                px_srcline(761);
                _v1522 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
                px_srcline(762);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPF"), _v1551, px_int(0LL), px_int(0LL)}, 5));
                px_srcline(763);
                 _v1552 = _v1522;
                px_srcline(764);
                if (px_is_truthy(px_ne(px_index(_v1550, px_int(2LL)), px_null()))) {
                    px_srcline(765);
                    _v1554 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
                    px_srcline(766);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1550, px_int(2LL)), _v1554, _v1507}, 3));
                    px_srcline(767);
                    _v1555 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
                    px_srcline(768);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMPF"), _v1554, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(769);
                     _v1553 = _v1555;
                }
            }
            px_srcline(770);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1550, px_int(3LL)), _v1511, _v1507}, 3));
            px_srcline(771);
            _v1523 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
            px_srcline(772);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1507, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(773);
            _v1556 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
            px_srcline(774);
            if (px_is_truthy(px_ge(_v1552, px_int(0LL)))) {
                px_srcline(775);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1552, _v1556}, 3));
            }
            px_srcline(776);
            if (px_is_truthy(px_ge(_v1553, px_int(0LL)))) {
                px_srcline(777);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, _v1553, _v1556}, 3));
            }
            px_srcline(778);
            (void)(px_method(_v1548, "append", (LXValue[]){_v1523}, 1));
            px_srcline(779);
             _v1549 = px_add(_v1549, px_int(1LL));
        }
        px_srcline(780);
        _v1557 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1507, px_str("bc"))}, 1);
        px_srcline(781);
        _v1558 = px_int(0LL);
        px_srcline(782);
        while (px_is_truthy(px_lt(_v1558, px_call(px_get_global("len"), (LXValue[]){_v1548}, 1)))) {
            px_srcline(783);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1507, px_index(_v1548, _v1558), _v1557}, 3));
            px_srcline(784);
             _v1558 = px_add(_v1558, px_int(1LL));
        }
        px_srcline(785);
        return _v1511;
    }
    px_srcline(786);
    if (px_is_truthy(px_eq(_v1508, px_str("Block")))) {
        px_srcline(790);
        _v1511 = _v1506;
        px_srcline(791);
        if (px_is_truthy(px_lt(_v1511, px_int(0LL)))) {
            px_srcline(792);
             _v1511 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1507}, 1);
        }
        px_srcline(793);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1507, _v1511}, 2));
        px_srcline(794);
        _v1559 = px_index(_v1505, px_int(1LL));
        px_srcline(795);
        _v1560 = px_int(0LL);
        px_srcline(796);
        while (px_is_truthy(px_lt(_v1560, px_call(px_get_global("len"), (LXValue[]){_v1559}, 1)))) {
            px_srcline(797);
            _v1561 = px_index(_v1559, _v1560);
            px_srcline(798);
            if (px_is_truthy(px_eq(px_index(_v1561, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(799);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1561, px_int(1LL)), _v1511, _v1507}, 3));
            }
            else {
                px_srcline(801);
                (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){_v1561, _v1507}, 2));
            }
            px_srcline(802);
             _v1560 = px_add(_v1560, px_int(1LL));
        }
        px_srcline(803);
        return _v1511;
    }
    px_srcline(804);
    if (px_is_truthy(px_eq(_v1508, px_str("Closure")))) {
        px_srcline(807);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1505, px_int(1LL)), px_index(_v1505, px_int(3LL)), _v1506, _v1507}, 4));
        px_srcline(808);
        return _v1506;
    }
    px_srcline(809);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_expr 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1505}, 1))}, 1));
px_err_1562:
    if (px_err_1562_proped) return px_err_1562_val;
    return px_null();
}

static LXValue fn_bc_emit_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_call");
    LXValue _v1566 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1567 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1568 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1569 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1570 = px_null();
    LXValue _v1571 = px_null();
    LXValue _v1572 = px_null();
    LXValue _v1573 = px_null();
    LXValue _v1574 = px_null();
    LXValue px_err_1575_val = px_null();
    int px_err_1575_proped = 0;
    px_srcline(813);
    _v1570 = _v1568;
    px_srcline(814);
    if (px_is_truthy(px_lt(_v1570, px_int(0LL)))) {
        px_srcline(815);
         _v1570 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1569}, 1);
    }
    px_srcline(816);
    _v1571 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1569}, 1);
    px_srcline(817);
    _v1572 = px_call(px_get_global("len"), (LXValue[]){_v1567}, 1);
    px_srcline(818);
    _v1573 = px_add(px_add(_v1571, px_int(1LL)), _v1572);
    px_srcline(819);
    while (px_is_truthy(px_lt(px_index(_v1569, px_str("next_slot")), _v1573))) {
        px_srcline(820);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1569}, 1));
    }
    px_srcline(821);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1566, _v1571, _v1569}, 3));
    px_srcline(822);
    _v1574 = px_int(0LL);
    px_srcline(823);
    while (px_is_truthy(px_lt(_v1574, _v1572))) {
        px_srcline(824);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1567, _v1574), px_add(px_add(_v1571, px_int(1LL)), _v1574), _v1569}, 3));
        px_srcline(825);
         _v1574 = px_add(_v1574, px_int(1LL));
    }
    px_srcline(826);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1569, px_str("CALL"), _v1570, _v1571, _v1572}, 5));
    px_srcline(827);
    return _v1570;
px_err_1575:
    if (px_err_1575_proped) return px_err_1575_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall");
    LXValue _v1576 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1577 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1578 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1579 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1580 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1581 = px_null();
    LXValue _v1582 = px_null();
    LXValue _v1583 = px_null();
    LXValue _v1584 = px_null();
    LXValue _v1585 = px_null();
    LXValue _v1586 = px_null();
    LXValue px_err_1587_val = px_null();
    int px_err_1587_proped = 0;
    px_srcline(833);
    _v1581 = _v1579;
    px_srcline(834);
    if (px_is_truthy(px_lt(_v1581, px_int(0LL)))) {
        px_srcline(835);
         _v1581 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1580}, 1);
    }
    px_srcline(836);
    _v1582 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1580}, 1);
    px_srcline(837);
    _v1583 = px_call(px_get_global("len"), (LXValue[]){_v1578}, 1);
    px_srcline(838);
    _v1584 = px_add(px_add(_v1582, px_int(1LL)), _v1583);
    px_srcline(839);
    while (px_is_truthy(px_lt(px_index(_v1580, px_str("next_slot")), _v1584))) {
        px_srcline(840);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1580}, 1));
    }
    px_srcline(841);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1576, _v1582, _v1580}, 3));
    px_srcline(842);
    _v1585 = px_int(0LL);
    px_srcline(843);
    while (px_is_truthy(px_lt(_v1585, _v1583))) {
        px_srcline(844);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1578, _v1585), px_add(px_add(_v1582, px_int(1LL)), _v1585), _v1580}, 3));
        px_srcline(845);
         _v1585 = px_add(_v1585, px_int(1LL));
    }
    px_srcline(846);
    _v1586 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1577}, 2);
    px_srcline(847);
    (void)(px_method(px_index(_v1580, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1583, _v1581, _v1582, _v1586}, 5)}, 1));
    px_srcline(848);
    return _v1581;
px_err_1587:
    if (px_err_1587_proped) return px_err_1587_val;
    return px_null();
}

static LXValue fn_bc_emit_struct_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_struct_new");
    LXValue _v1588 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1589 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1590 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1591 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1592 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1593 = px_null();
    LXValue _v1594 = px_null();
    LXValue _v1595 = px_null();
    LXValue _v1596 = px_null();
    LXValue _v1597 = px_null();
    LXValue px_err_1598_val = px_null();
    int px_err_1598_proped = 0;
    px_srcline(852);
    _v1593 = px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1589);
    px_srcline(853);
    _v1594 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1593, px_str("fnames"))}, 1);
    px_srcline(854);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1590}, 1), _v1594))) {
        px_srcline(855);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_str("结构体 "), _v1588), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){_v1594}, 1)), px_str(" 个字段，给出 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1590}, 1)}, 1))}, 1));
    }
    px_srcline(856);
    _v1595 = _v1591;
    px_srcline(857);
    if (px_is_truthy(px_lt(_v1595, px_int(0LL)))) {
        px_srcline(858);
         _v1595 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1592}, 1);
    }
    px_srcline(859);
    _v1596 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1592}, 1);
    px_srcline(860);
    while (px_is_truthy(px_lt(px_index(_v1592, px_str("next_slot")), px_add(_v1596, _v1594)))) {
        px_srcline(861);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1592}, 1));
    }
    px_srcline(862);
    _v1597 = px_int(0LL);
    px_srcline(863);
    while (px_is_truthy(px_lt(_v1597, _v1594))) {
        px_srcline(864);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1590, _v1597), px_add(_v1596, _v1597), _v1592}, 3));
        px_srcline(865);
         _v1597 = px_add(_v1597, px_int(1LL));
    }
    px_srcline(866);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1592, px_str("NEWSTRUCT"), _v1595, _v1589, _v1596}, 5));
    px_srcline(867);
    return _v1595;
px_err_1598:
    if (px_err_1598_proped) return px_err_1598_val;
    return px_null();
}

static LXValue fn_bc_emit_enum_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_enum_new");
    LXValue _v1599 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1600 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1601 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1602 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1603 = px_null();
    LXValue _v1604 = px_null();
    LXValue _v1605 = px_null();
    LXValue _v1606 = px_null();
    LXValue _v1607 = px_null();
    LXValue _v1608 = px_null();
    LXValue px_err_1609_val = px_null();
    int px_err_1609_proped = 0;
    px_srcline(870);
    _v1603 = px_null();
    px_srcline(871);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1600}, 1), px_int(1LL)))) {
        px_srcline(872);
        _v1604 = px_index(_v1600, px_int(0LL));
        px_srcline(873);
        if (px_is_truthy(px_eq(px_index(_v1604, px_int(0LL)), px_str("Var")))) {
            px_srcline(874);
            _v1605 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1604, px_int(1LL))}, 1);
            px_srcline(875);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1599, _v1605}, 2))) {
                px_srcline(876);
                 _v1603 = _v1605;
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v1604, px_int(0LL)), px_str("Str")))) {
            px_srcline(878);
             _v1603 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1604, px_int(1LL))}, 1);
        }
    }
    px_srcline(879);
    if (px_is_truthy(px_eq(_v1603, px_null()))) {
        px_srcline(880);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit enum 构造未实现（仅支持 Color(Variant)/Color(\"Variant\"): "), px_call(px_get_global("str"), (LXValue[]){_v1599}, 1))}, 1));
    }
    px_srcline(881);
    _v1606 = _v1601;
    px_srcline(882);
    if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
        px_srcline(883);
         _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
    }
    px_srcline(884);
    _v1607 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1599}, 2);
    px_srcline(885);
    _v1608 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1603}, 2);
    px_srcline(886);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWENUM"), _v1606, _v1607, _v1608}, 5));
    px_srcline(887);
    return _v1606;
px_err_1609:
    if (px_err_1609_proped) return px_err_1609_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall_slot");
    LXValue _v1610 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1611 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1612 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1613 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1614 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1615 = px_null();
    LXValue _v1616 = px_null();
    LXValue _v1617 = px_null();
    LXValue _v1618 = px_null();
    LXValue _v1619 = px_null();
    LXValue _v1620 = px_null();
    LXValue px_err_1621_val = px_null();
    int px_err_1621_proped = 0;
    px_srcline(894);
    _v1615 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1610}, 1);
    px_srcline(895);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1610, px_str("MOV"), _v1615, _v1611, px_int(0LL)}, 5));
    px_srcline(896);
    _v1616 = px_call(px_get_global("len"), (LXValue[]){_v1613}, 1);
    px_srcline(897);
    _v1617 = px_add(px_add(_v1615, px_int(1LL)), _v1616);
    px_srcline(898);
    while (px_is_truthy(px_lt(px_index(_v1610, px_str("next_slot")), _v1617))) {
        px_srcline(899);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1610}, 1));
    }
    px_srcline(900);
    _v1618 = px_int(0LL);
    px_srcline(901);
    while (px_is_truthy(px_lt(_v1618, _v1616))) {
        px_srcline(902);
        _v1619 = px_index(_v1613, _v1618);
        px_srcline(903);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1619}, 1), px_str("int")))) {
            px_srcline(904);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1610, px_str("MOV"), px_add(px_add(_v1615, px_int(1LL)), _v1618), _v1619, px_int(0LL)}, 5));
        }
        else {
            px_srcline(906);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1619, px_add(px_add(_v1615, px_int(1LL)), _v1618), _v1610}, 3));
        }
        px_srcline(907);
         _v1618 = px_add(_v1618, px_int(1LL));
    }
    px_srcline(908);
    _v1620 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1612}, 2);
    px_srcline(909);
    (void)(px_method(px_index(_v1610, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1616, _v1614, _v1615, _v1620}, 5)}, 1));
    px_srcline(910);
    return _v1614;
px_err_1621:
    if (px_err_1621_proped) return px_err_1621_val;
    return px_null();
}

static LXValue fn_bc_emit_push_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_push_lambda");
    LXValue _v1622 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1623 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1624 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1625 = px_null();
    LXValue _v1626 = px_null();
    LXValue _v1627 = px_null();
    LXValue _v1628 = px_null();
    LXValue _v1629 = px_null();
    LXValue _v1630 = px_null();
    LXValue _v1631 = px_null();
    LXValue _v1632 = px_null();
    LXValue _v1633 = px_null();
    LXValue _v1634 = px_null();
    LXValue px_err_1635_val = px_null();
    int px_err_1635_proped = 0;
    px_srcline(914);
    _v1625 = px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("nclosure"))}, 1)), px_str(">"));
    px_srcline(915);
    px_index_set(px_get_global("g_bcm"), px_str("nclosure"), px_add(px_index(px_get_global("g_bcm"), px_str("nclosure")), px_int(1LL)));
    px_srcline(916);
    _v1626 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1625, px_call(px_get_global("len"), (LXValue[]){_v1622}, 1)}, 2);
    px_srcline(917);
    _v1627 = px_int(0LL);
    px_srcline(918);
    while (px_is_truthy(px_lt(_v1627, px_call(px_get_global("len"), (LXValue[]){_v1622}, 1)))) {
        px_srcline(919);
        px_index_set(px_index(_v1626, px_str("smap")), px_index(_v1622, _v1627), _v1627);
        px_srcline(920);
         _v1627 = px_add(_v1627, px_int(1LL));
    }
    px_srcline(923);
    _v1628 = px_int(0LL);
    px_srcline(924);
    while (px_is_truthy(px_lt(_v1628, px_call(px_get_global("len"), (LXValue[]){_v1624}, 1)))) {
        px_srcline(925);
        _v1629 = px_index(_v1624, _v1628);
        px_srcline(926);
        px_index_set(px_index(_v1626, px_str("smap")), _v1629, px_add(px_call(px_get_global("len"), (LXValue[]){_v1622}, 1), _v1628));
        px_srcline(927);
        (void)(px_method(px_index(_v1626, px_str("upnames")), "push", (LXValue[]){_v1629}, 1));
        px_srcline(928);
        (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1626, _v1629}, 2));
        px_srcline(929);
         _v1628 = px_add(_v1628, px_int(1LL));
    }
    px_srcline(930);
    if (px_is_truthy(px_lt(px_index(_v1626, px_str("next_slot")), px_add(px_call(px_get_global("len"), (LXValue[]){_v1622}, 1), px_call(px_get_global("len"), (LXValue[]){_v1624}, 1))))) {
        px_srcline(931);
        px_index_set(_v1626, px_str("next_slot"), px_add(px_call(px_get_global("len"), (LXValue[]){_v1622}, 1), px_call(px_get_global("len"), (LXValue[]){_v1624}, 1)));
    }
    px_srcline(932);
    (void)(px_method(px_index(px_get_global("g_bcm"), px_str("funcs")), "push", (LXValue[]){_v1626}, 1));
    px_srcline(937);
    _v1630 = px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("funcs"))}, 1), px_int(1LL));
    px_srcline(941);
    _v1631 = px_list_n((LXValue[]){}, 0);
    px_srcline(942);
    (void)(px_call(px_get_global("bc_lambda_hoist"), (LXValue[]){_v1623, _v1631}, 2));
    px_srcline(943);
    _v1632 = px_int(0LL);
    px_srcline(944);
    while (px_is_truthy(px_lt(_v1632, px_call(px_get_global("len"), (LXValue[]){_v1631}, 1)))) {
        px_srcline(945);
        _v1633 = px_index(_v1631, _v1632);
        px_srcline(946);
        if (px_is_truthy(({ LXValue _t1636 = px_not(px_method(px_index(_v1626, px_str("smap")), "has", (LXValue[]){_v1633}, 1)); px_is_truthy(_t1636) ? px_not(px_call(px_get_global("bc_is_global"), (LXValue[]){_v1633}, 1)) : _t1636; }))) {
            px_srcline(947);
            (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v1626, _v1633}, 2));
        }
        px_srcline(948);
         _v1632 = px_add(_v1632, px_int(1LL));
    }
    px_srcline(950);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v1626, _v1623}, 2));
    px_srcline(951);
    _v1634 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1626}, 1);
    px_srcline(952);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1623, _v1634, _v1626}, 3));
    px_srcline(953);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1626, px_str("RET"), _v1634, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(954);
    px_index_set(_v1626, px_str("nslots"), px_index(_v1626, px_str("next_slot")));
    px_srcline(955);
    return _v1630;
px_err_1635:
    if (px_err_1635_proped) return px_err_1635_val;
    return px_null();
}

static LXValue fn_bc_emit_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_closure");
    LXValue _v1637 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1638 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1639 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1640 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1641 = px_null();
    LXValue _v1642 = px_null();
    LXValue _v1643 = px_null();
    LXValue _v1644 = px_null();
    LXValue _v1645 = px_null();
    LXValue _v1646 = px_null();
    LXValue _v1647 = px_null();
    LXValue _v1648 = px_null();
    LXValue _v1649 = px_null();
    LXValue _v1650 = px_null();
    LXValue _v1651 = px_null();
    LXValue _v1652 = px_null();
    LXValue _v1653 = px_null();
    LXValue px_err_1654_val = px_null();
    int px_err_1654_proped = 0;
    px_srcline(960);
    _v1641 = px_list_n((LXValue[]){}, 0);
    px_srcline(961);
    _v1642 = px_int(0LL);
    px_srcline(962);
    while (px_is_truthy(px_lt(_v1642, px_call(px_get_global("len"), (LXValue[]){_v1637}, 1)))) {
        px_srcline(963);
        (void)(px_method(_v1641, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1637, _v1642), px_int(1LL))}, 1)}, 1));
        px_srcline(964);
         _v1642 = px_add(_v1642, px_int(1LL));
    }
    px_srcline(965);
    _v1643 = px_call(px_get_global("bc_closure_free"), (LXValue[]){_v1637, _v1638}, 2);
    px_srcline(966);
    _v1644 = px_list_n((LXValue[]){}, 0);
    px_srcline(967);
    _v1645 = px_int(0LL);
    px_srcline(968);
    while (px_is_truthy(px_lt(_v1645, px_call(px_get_global("len"), (LXValue[]){_v1643}, 1)))) {
        px_srcline(969);
        _v1646 = px_index(_v1643, _v1645);
        px_srcline(970);
        if (px_is_truthy(px_method(px_index(_v1640, px_str("smap")), "has", (LXValue[]){_v1646}, 1))) {
            px_srcline(971);
            (void)(px_method(_v1644, "append", (LXValue[]){_v1646}, 1));
        }
        px_srcline(972);
         _v1645 = px_add(_v1645, px_int(1LL));
    }
    px_srcline(973);
    _v1647 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){_v1641, _v1638, _v1644}, 3);
    px_srcline(974);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1644}, 1), px_int(0LL)))) {
        px_srcline(975);
        _v1648 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1647, px_float(0), px_str("")}, 5);
        px_srcline(976);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1640, px_str("LOADK"), _v1639, _v1648, px_int(0LL)}, 5));
        px_srcline(977);
        return _v1639;
    }
    px_srcline(980);
    _v1649 = px_index(_v1640, px_str("next_slot"));
    px_srcline(981);
    _v1650 = px_int(0LL);
    px_srcline(982);
    while (px_is_truthy(px_lt(_v1650, px_call(px_get_global("len"), (LXValue[]){_v1644}, 1)))) {
        px_srcline(983);
        _v1651 = px_index(px_index(_v1640, px_str("smap")), px_index(_v1644, _v1650));
        px_srcline(984);
        _v1652 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1640}, 1);
        px_srcline(985);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1640, px_str("MOV"), _v1652, _v1651, px_int(0LL)}, 5));
        px_srcline(986);
         _v1650 = px_add(_v1650, px_int(1LL));
    }
    px_srcline(987);
    _v1653 = _v1639;
    px_srcline(988);
    if (px_is_truthy(px_lt(_v1653, px_int(0LL)))) {
        px_srcline(989);
         _v1653 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1640}, 1);
    }
    px_srcline(990);
    (void)(px_method(px_index(_v1640, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1653, _v1647, _v1649}, 5)}, 1));
    px_srcline(991);
    return _v1653;
px_err_1654:
    if (px_err_1654_proped) return px_err_1654_val;
    return px_null();
}

static LXValue fn_bc_emit_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_comp");
    LXValue _v1655 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1656 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1657 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1658 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1659 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1660 = (nargs > 5) ? args[5] : px_null();
    LXValue _v1661 = (nargs > 6) ? args[6] : px_null();
    LXValue _v1662 = px_null();
    LXValue _v1663 = px_null();
    LXValue _v1664 = px_null();
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
    LXValue _v1689 = px_null();
    LXValue _v1690 = px_null();
    LXValue px_err_1691_val = px_null();
    int px_err_1691_proped = 0;
    px_srcline(998);
    _v1662 = px_call(px_get_global("len"), (LXValue[]){_v1660}, 1);
    px_srcline(999);
    if (px_is_truthy(px_ge(_v1661, _v1662))) {
        px_srcline(1000);
        _v1663 = px_neg(px_int(1LL));
        px_srcline(1001);
        if (px_is_truthy(px_ne(_v1658, px_null()))) {
            px_srcline(1002);
            _v1664 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
            px_srcline(1003);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1658, _v1664, _v1655}, 3));
            px_srcline(1004);
             _v1663 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
            px_srcline(1005);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1664, px_int(0LL), px_int(0LL)}, 5));
        }
        px_srcline(1006);
        if (px_is_truthy(px_eq(_v1656, px_str("dict")))) {
            px_srcline(1007);
            _v1665 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
            px_srcline(1008);
            _v1666 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
            px_srcline(1009);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1657, px_int(0LL)), _v1665, _v1655}, 3));
            px_srcline(1010);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1657, px_int(1LL)), _v1666, _v1655}, 3));
            px_srcline(1011);
            (void)(px_call(px_get_global("bc_emit_methodcall_slot"), (LXValue[]){_v1655, _v1659, px_str("set"), px_list_n((LXValue[]){_v1665, _v1666}, 2), px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1)}, 5));
        }
        else {
            px_srcline(1013);
            _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
            px_srcline(1014);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1657, px_int(0LL)), _v1667, _v1655}, 3));
            px_srcline(1015);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("LISTPUSH"), _v1667, _v1659, px_int(0LL)}, 5));
        }
        px_srcline(1016);
        if (px_is_truthy(px_ge(_v1663, px_int(0LL)))) {
            px_srcline(1017);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1663, px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1018);
        return px_null();
    }
    px_srcline(1019);
    _v1668 = px_index(_v1660, _v1661);
    px_srcline(1020);
    _v1669 = px_index(_v1668, px_int(1LL));
    px_srcline(1026);
    _v1670 = px_list_n((LXValue[]){}, 0);
    px_srcline(1027);
    _v1671 = px_int(0LL);
    px_srcline(1028);
    while (px_is_truthy(px_lt(_v1671, px_call(px_get_global("len"), (LXValue[]){_v1669}, 1)))) {
        px_srcline(1029);
        _v1672 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1669, _v1671)}, 1);
        px_srcline(1030);
        _v1673 = px_neg(px_int(1LL));
        px_srcline(1031);
        if (px_is_truthy(px_method(px_index(_v1655, px_str("smap")), "has", (LXValue[]){_v1672}, 1))) {
            px_srcline(1032);
             _v1673 = px_index(px_index(_v1655, px_str("smap")), _v1672);
        }
        px_srcline(1033);
        _v1674 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(1034);
        px_index_set(px_index(_v1655, px_str("smap")), _v1672, _v1674);
        px_srcline(1035);
        (void)(px_method(_v1670, "append", (LXValue[]){px_list_n((LXValue[]){_v1672, _v1673}, 2)}, 1));
        px_srcline(1036);
         _v1671 = px_add(_v1671, px_int(1LL));
    }
    px_srcline(1037);
    _v1675 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1038);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1668, px_int(2LL)), _v1675, _v1655}, 3));
    px_srcline(1039);
    _v1676 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1040);
    _v1677 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1041);
    while (px_is_truthy(px_lt(px_index(_v1655, px_str("next_slot")), px_add(_v1676, px_int(2LL))))) {
        px_srcline(1042);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1));
    }
    px_srcline(1043);
    _v1678 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1044);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("GETG"), _v1676, _v1678, px_int(0LL)}, 5));
    px_srcline(1045);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("MOV"), px_add(_v1676, px_int(1LL)), _v1675, px_int(0LL)}, 5));
    px_srcline(1046);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("CALL"), _v1677, _v1676, px_int(1LL)}, 5));
    px_srcline(1047);
    _v1679 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1048);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1655, _v1679, px_int(0LL)}, 3));
    px_srcline(1049);
    _v1680 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
    px_srcline(1050);
    _v1681 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1051);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("LT"), _v1681, _v1679, _v1677}, 5));
    px_srcline(1052);
    _v1682 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
    px_srcline(1053);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1681, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1054);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1669}, 1), px_int(1LL)))) {
        px_srcline(1055);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("INDEX"), px_index(px_index(_v1655, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1669, px_int(0LL))}, 1)), _v1675, _v1679}, 5));
    }
    else {
        px_srcline(1058);
        _v1683 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(1059);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("INDEX"), _v1683, _v1675, _v1679}, 5));
        px_srcline(1060);
        _v1684 = px_int(0LL);
        px_srcline(1061);
        while (px_is_truthy(px_lt(_v1684, px_call(px_get_global("len"), (LXValue[]){_v1669}, 1)))) {
            px_srcline(1062);
            _v1685 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
            px_srcline(1063);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1655, _v1685, _v1684}, 3));
            px_srcline(1064);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("INDEX"), px_index(px_index(_v1655, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1669, _v1684)}, 1)), _v1683, _v1685}, 5));
            px_srcline(1065);
             _v1684 = px_add(_v1684, px_int(1LL));
        }
    }
    px_srcline(1066);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1655, _v1656, _v1657, _v1658, _v1659, _v1660, px_add(_v1661, px_int(1LL))}, 7));
    px_srcline(1067);
    _v1686 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
    px_srcline(1068);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1655, _v1686, px_int(1LL)}, 3));
    px_srcline(1069);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("ADD"), _v1679, _v1679, _v1686}, 5));
    px_srcline(1070);
    _v1687 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
    px_srcline(1071);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1072);
    _v1688 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
    px_srcline(1073);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1687, _v1680}, 3));
    px_srcline(1074);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1682, _v1688}, 3));
    px_srcline(1076);
    _v1689 = px_int(0LL);
    px_srcline(1077);
    while (px_is_truthy(px_lt(_v1689, px_call(px_get_global("len"), (LXValue[]){_v1670}, 1)))) {
        px_srcline(1078);
        _v1690 = px_index(_v1670, _v1689);
        px_srcline(1079);
        if (px_is_truthy(px_lt(px_index(_v1690, px_int(1LL)), px_int(0LL)))) {
            px_srcline(1080);
            (void)(px_method(px_index(_v1655, px_str("smap")), "remove", (LXValue[]){px_index(_v1690, px_int(0LL))}, 1));
        }
        else {
            px_srcline(1082);
            px_index_set(px_index(_v1655, px_str("smap")), px_index(_v1690, px_int(0LL)), px_index(_v1690, px_int(1LL)));
        }
        px_srcline(1083);
         _v1689 = px_add(_v1689, px_int(1LL));
    }
px_err_1691:
    if (px_err_1691_proped) return px_err_1691_val;
    return px_null();
}

static LXValue fn_bc_emit_caps_snapshot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_caps_snapshot");
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
    LXValue px_err_1703_val = px_null();
    int px_err_1703_proped = 0;
    px_srcline(1091);
    _v1694 = px_call(px_get_global("len"), (LXValue[]){_v1693}, 1);
    px_srcline(1092);
    _v1695 = px_index(_v1692, px_str("next_slot"));
    px_srcline(1093);
    _v1696 = px_int(0LL);
    px_srcline(1094);
    while (px_is_truthy(px_lt(_v1696, _v1694))) {
        px_srcline(1095);
        _v1697 = px_index(_v1693, _v1696);
        px_srcline(1096);
        _v1698 = px_index(px_index(_v1692, px_str("smap")), _v1697);
        px_srcline(1097);
        _v1699 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1692}, 1);
        px_srcline(1098);
        if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1692, _v1697}, 2))) {
            px_srcline(1099);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1692, px_str("CELLGET"), _v1699, _v1698, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1101);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1692, px_str("MOV"), _v1699, _v1698, px_int(0LL)}, 5));
        }
        px_srcline(1102);
         _v1696 = px_add(_v1696, px_int(1LL));
    }
    px_srcline(1103);
    _v1700 = px_index(_v1692, px_str("next_slot"));
    px_srcline(1104);
    _v1701 = px_int(0LL);
    px_srcline(1105);
    while (px_is_truthy(px_lt(_v1701, _v1694))) {
        px_srcline(1106);
        _v1702 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1692}, 1);
        px_srcline(1107);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1692, px_str("CELLNEW"), _v1702, px_add(_v1695, _v1701), px_int(0LL)}, 5));
        px_srcline(1108);
         _v1701 = px_add(_v1701, px_int(1LL));
    }
    px_srcline(1109);
    return _v1700;
px_err_1703:
    if (px_err_1703_proped) return px_err_1703_val;
    return px_null();
}

static LXValue fn_bc_genexp_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_genexp_caps");
    LXValue _v1704 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1705 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1706 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1707 = px_null();
    LXValue _v1708 = px_null();
    LXValue _v1709 = px_null();
    LXValue _v1710 = px_null();
    LXValue px_err_1711_val = px_null();
    int px_err_1711_proped = 0;
    px_srcline(1113);
    _v1707 = px_list_n((LXValue[]){}, 0);
    px_srcline(1114);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v1704, px_null(), _v1705, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v1707}, 2));
    px_srcline(1115);
    _v1708 = px_list_n((LXValue[]){}, 0);
    px_srcline(1116);
    _v1709 = px_int(0LL);
    px_srcline(1117);
    while (px_is_truthy(px_lt(_v1709, px_call(px_get_global("len"), (LXValue[]){_v1707}, 1)))) {
        px_srcline(1118);
        _v1710 = px_index(_v1707, _v1709);
        px_srcline(1119);
        if (px_is_truthy(px_method(px_index(_v1706, px_str("smap")), "has", (LXValue[]){_v1710}, 1))) {
            px_srcline(1120);
            (void)(px_method(_v1708, "append", (LXValue[]){_v1710}, 1));
        }
        px_srcline(1121);
         _v1709 = px_add(_v1709, px_int(1LL));
    }
    px_srcline(1122);
    return _v1708;
px_err_1711:
    if (px_err_1711_proped) return px_err_1711_val;
    return px_null();
}

static LXValue fn_bc_emit_genexp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_genexp");
    LXValue _v1712 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1713 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1714 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1715 = px_null();
    LXValue _v1716 = px_null();
    LXValue _v1717 = px_null();
    LXValue _v1718 = px_null();
    LXValue _v1719 = px_null();
    LXValue _v1720 = px_null();
    LXValue _v1721 = px_null();
    LXValue _v1722 = px_null();
    LXValue _v1723 = px_null();
    LXValue _v1724 = px_null();
    LXValue _v1725 = px_null();
    LXValue _v1726 = px_null();
    LXValue _v1727 = px_null();
    LXValue _v1728 = px_null();
    LXValue px_err_1729_val = px_null();
    int px_err_1729_proped = 0;
    px_srcline(1126);
    _v1715 = px_index(_v1712, px_int(2LL));
    px_srcline(1127);
    if (px_is_truthy(({ LXValue _t1730 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1715}, 1), px_int(1LL)); px_is_truthy(_t1730) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1715, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1730; }))) {
        px_srcline(1130);
        _v1716 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1715, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
        px_srcline(1131);
        _v1717 = _v1713;
        px_srcline(1132);
        if (px_is_truthy(px_lt(_v1717, px_int(0LL)))) {
            px_srcline(1133);
             _v1717 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1714}, 1);
        }
        px_srcline(1134);
        _v1718 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1714}, 1);
        px_srcline(1135);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1715, px_int(0LL)), px_int(2LL)), _v1718, _v1714}, 3));
        px_srcline(1140);
        _v1719 = px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("Param"), _v1716, px_null(), px_null(), px_null()}, 5)}, 1);
        px_srcline(1141);
        _v1720 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){_v1719, px_index(_v1712, px_int(1LL)), _v1714}, 3);
        px_srcline(1142);
        _v1721 = px_list_n((LXValue[]){}, 0);
        px_srcline(1143);
        if (px_is_truthy(px_ne(px_index(_v1712, px_int(3LL)), px_null()))) {
            px_srcline(1144);
             _v1721 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){_v1719, px_index(_v1712, px_int(3LL)), _v1714}, 3);
        }
        px_srcline(1145);
        _v1722 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1716}, 1), px_index(_v1712, px_int(1LL)), _v1720}, 3);
        px_srcline(1146);
        _v1723 = px_neg(px_int(1LL));
        px_srcline(1147);
        if (px_is_truthy(px_ne(px_index(_v1712, px_int(3LL)), px_null()))) {
            px_srcline(1148);
             _v1723 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1716}, 1), px_index(_v1712, px_int(3LL)), _v1721}, 3);
        }
        px_srcline(1149);
        _v1724 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1714}, 1);
        px_srcline(1150);
        while (px_is_truthy(px_lt(px_index(_v1714, px_str("next_slot")), px_add(_v1724, px_int(2LL))))) {
            px_srcline(1151);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1714}, 1));
        }
        px_srcline(1154);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1720}, 1), px_int(0LL)))) {
            px_srcline(1155);
            _v1725 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1722, px_float(0), px_str("")}, 5);
            px_srcline(1156);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1714, px_str("LOADK"), _v1724, _v1725, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1158);
            _v1726 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1714, _v1720}, 2);
            px_srcline(1159);
            (void)(px_method(px_index(_v1714, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1724, _v1722, _v1726}, 5)}, 1));
        }
        px_srcline(1160);
        if (px_is_truthy(px_ge(_v1723, px_int(0LL)))) {
            px_srcline(1161);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1721}, 1), px_int(0LL)))) {
                px_srcline(1162);
                _v1727 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1723, px_float(0), px_str("")}, 5);
                px_srcline(1163);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1714, px_str("LOADK"), px_add(_v1724, px_int(1LL)), _v1727, px_int(0LL)}, 5));
            }
            else {
                px_srcline(1165);
                _v1728 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1714, _v1721}, 2);
                px_srcline(1166);
                (void)(px_method(px_index(_v1714, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), px_add(_v1724, px_int(1LL)), _v1723, _v1728}, 5)}, 1));
            }
        }
        else {
            px_srcline(1168);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1714, px_add(_v1724, px_int(1LL))}, 2));
        }
        px_srcline(1169);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1714, px_str("NEWGEN"), _v1717, _v1718, _v1724}, 5));
        px_srcline(1170);
        return _v1717;
    }
    px_srcline(1174);
    _v1717 = _v1713;
    px_srcline(1175);
    if (px_is_truthy(px_lt(_v1717, px_int(0LL)))) {
        px_srcline(1176);
         _v1717 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1714}, 1);
    }
    px_srcline(1177);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1714, px_str("NEWLIST"), _v1717, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1178);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1714, px_str("push"), px_list_n((LXValue[]){px_index(_v1712, px_int(1LL))}, 1), px_index(_v1712, px_int(3LL)), _v1717, _v1715, px_int(0LL)}, 7));
    px_srcline(1179);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1714, px_str("GENFROMLIST"), _v1717, _v1717, px_int(0LL)}, 5));
    px_srcline(1180);
    return _v1717;
px_err_1729:
    if (px_err_1729_proped) return px_err_1729_val;
    return px_null();
}

static LXValue fn_bc_match_enumvar(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_enumvar");
    LXValue _v1731 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1732 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1733 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1734 = px_null();
    LXValue _v1735 = px_null();
    LXValue _v1736 = px_null();
    LXValue _v1737 = px_null();
    LXValue px_err_1738_val = px_null();
    int px_err_1738_proped = 0;
    px_srcline(1186);
    _v1734 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1733}, 1);
    px_srcline(1187);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1733, px_str("ENUMVAR"), _v1734, _v1732, px_int(0LL)}, 5));
    px_srcline(1188);
    _v1735 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1731}, 5);
    px_srcline(1189);
    _v1736 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1733}, 1);
    px_srcline(1190);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1733, px_str("LOADK"), _v1736, _v1735, px_int(0LL)}, 5));
    px_srcline(1191);
    _v1737 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1733}, 1);
    px_srcline(1192);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1733, px_str("EQ"), _v1737, _v1734, _v1736}, 5));
    px_srcline(1193);
    return _v1737;
px_err_1738:
    if (px_err_1738_proped) return px_err_1738_val;
    return px_null();
}

static LXValue fn_bc_match_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_cond");
    LXValue _v1739 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1740 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1741 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1742 = px_null();
    LXValue _v1743 = px_null();
    LXValue _v1744 = px_null();
    LXValue _v1745 = px_null();
    LXValue _v1746 = px_null();
    LXValue px_err_1747_val = px_null();
    int px_err_1747_proped = 0;
    px_srcline(1195);
    _v1742 = px_index(_v1739, px_int(0LL));
    px_srcline(1196);
    if (px_is_truthy(px_eq(_v1742, px_str("PatWildcard")))) {
        px_srcline(1197);
        return px_null();
    }
    px_srcline(1198);
    if (px_is_truthy(px_eq(_v1742, px_str("PatBinding")))) {
        px_srcline(1199);
        _v1743 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1739, px_int(1LL))}, 1);
        px_srcline(1200);
        if (px_is_truthy(({ LXValue _t1749 = ({ LXValue _t1748 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1743}, 1), px_int(0LL)); px_is_truthy(_t1748) ? px_ge(px_index(_v1743, px_int(0LL)), px_str("A")) : _t1748; }); px_is_truthy(_t1749) ? px_le(px_index(_v1743, px_int(0LL)), px_str("Z")) : _t1749; }))) {
            px_srcline(1202);
            return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){_v1743, _v1740, _v1741}, 3);
        }
        px_srcline(1203);
        return px_null();
    }
    px_srcline(1204);
    if (px_is_truthy(px_eq(_v1742, px_str("PatTuple")))) {
        px_srcline(1205);
        _v1744 = px_index(_v1739, px_int(1LL));
        px_srcline(1206);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1744}, 1), px_int(0LL)))) {
            px_srcline(1207);
            return px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1744, px_int(0LL)), _v1740, _v1741}, 3);
        }
        px_srcline(1208);
        return px_null();
    }
    px_srcline(1209);
    if (px_is_truthy(px_eq(_v1742, px_str("PatConstructor")))) {
        px_srcline(1210);
        return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1739, px_int(1LL))}, 1), _v1740, _v1741}, 3);
    }
    px_srcline(1211);
    if (px_is_truthy(px_eq(_v1742, px_str("PatLiteral")))) {
        px_srcline(1212);
        _v1745 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1741}, 1);
        px_srcline(1213);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1739, px_int(1LL)), _v1745, _v1741}, 3));
        px_srcline(1214);
        _v1746 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1741}, 1);
        px_srcline(1215);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1741, px_str("EQ"), _v1746, _v1740, _v1745}, 5));
        px_srcline(1216);
        return _v1746;
    }
    px_srcline(1217);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_match_cond 未知 pattern: "), px_call(px_get_global("str"), (LXValue[]){_v1739}, 1))}, 1));
px_err_1747:
    if (px_err_1747_proped) return px_err_1747_val;
    return px_null();
}

static LXValue fn_bc_assign_local_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_local_slot");
    LXValue _v1750 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1751 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1752 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1753 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1754 = px_null();
    LXValue _v1755 = px_null();
    LXValue px_err_1756_val = px_null();
    int px_err_1756_proped = 0;
    px_srcline(1221);
    if (px_is_truthy(px_eq(_v1751, px_str("Assign")))) {
        px_srcline(1228);
        _v1754 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1753}, 1);
        px_srcline(1229);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1752, _v1754, _v1753}, 3));
        px_srcline(1230);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1753, px_str("MOV"), _v1750, _v1754, px_int(0LL)}, 5));
        px_srcline(1231);
        return px_null();
    }
    px_srcline(1232);
    _v1755 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1753}, 1);
    px_srcline(1233);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1752, _v1755, _v1753}, 3));
    px_srcline(1234);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1753, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1751}, 1)}, 1), _v1750, _v1750, _v1755}, 5));
px_err_1756:
    if (px_err_1756_proped) return px_err_1756_val;
    return px_null();
}

static LXValue fn_bc_assign_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_global");
    LXValue _v1757 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1758 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1759 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1760 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1761 = px_null();
    LXValue _v1762 = px_null();
    LXValue _v1763 = px_null();
    LXValue px_err_1764_val = px_null();
    int px_err_1764_proped = 0;
    px_srcline(1236);
    _v1761 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1757}, 2);
    px_srcline(1237);
    if (px_is_truthy(px_eq(_v1758, px_str("Assign")))) {
        px_srcline(1238);
        _v1762 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1760}, 1);
        px_srcline(1239);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1759, _v1762, _v1760}, 3));
        px_srcline(1240);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1760, px_str("SETG"), _v1761, _v1762, px_int(0LL)}, 5));
        px_srcline(1241);
        return px_null();
    }
    px_srcline(1242);
    _v1762 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1760}, 1);
    px_srcline(1243);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1760, px_str("GETG"), _v1762, _v1761, px_int(0LL)}, 5));
    px_srcline(1244);
    _v1763 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1760}, 1);
    px_srcline(1245);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1759, _v1763, _v1760}, 3));
    px_srcline(1246);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1760, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1758}, 1)}, 1), _v1762, _v1762, _v1763}, 5));
    px_srcline(1247);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1760, px_str("SETG"), _v1761, _v1762, px_int(0LL)}, 5));
px_err_1764:
    if (px_err_1764_proped) return px_err_1764_val;
    return px_null();
}

static LXValue fn_bc_assign_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_index");
    LXValue _v1765 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1766 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1767 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1768 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1769 = px_null();
    LXValue _v1770 = px_null();
    LXValue _v1771 = px_null();
    LXValue _v1772 = px_null();
    LXValue px_err_1773_val = px_null();
    int px_err_1773_proped = 0;
    px_srcline(1250);
    _v1769 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1768}, 1);
    px_srcline(1251);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1765, px_int(1LL)), _v1769, _v1768}, 3));
    px_srcline(1252);
    _v1770 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1768}, 1);
    px_srcline(1253);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1765, px_int(2LL)), _v1770, _v1768}, 3));
    px_srcline(1254);
    _v1771 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1768}, 1);
    px_srcline(1255);
    if (px_is_truthy(px_eq(_v1766, px_str("Assign")))) {
        px_srcline(1256);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1767, _v1771, _v1768}, 3));
    }
    else {
        px_srcline(1258);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1768, px_str("INDEX"), _v1771, _v1769, _v1770}, 5));
        px_srcline(1259);
        _v1772 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1768}, 1);
        px_srcline(1260);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1767, _v1772, _v1768}, 3));
        px_srcline(1261);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1768, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1766}, 1)}, 1), _v1771, _v1771, _v1772}, 5));
    }
    px_srcline(1262);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1768, px_str("SETIDX"), _v1771, _v1769, _v1770}, 5));
px_err_1773:
    if (px_err_1773_proped) return px_err_1773_val;
    return px_null();
}

static LXValue fn_bc_assign_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_field");
    LXValue _v1774 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1775 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1776 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1777 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1778 = px_null();
    LXValue _v1779 = px_null();
    LXValue _v1780 = px_null();
    LXValue _v1781 = px_null();
    LXValue px_err_1782_val = px_null();
    int px_err_1782_proped = 0;
    px_srcline(1265);
    _v1778 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1777}, 1);
    px_srcline(1266);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1774, px_int(1LL)), _v1778, _v1777}, 3));
    px_srcline(1267);
    _v1779 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1774, px_int(2LL))}, 1)}, 2);
    px_srcline(1268);
    _v1780 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1777}, 1);
    px_srcline(1269);
    if (px_is_truthy(px_eq(_v1775, px_str("Assign")))) {
        px_srcline(1270);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1776, _v1780, _v1777}, 3));
    }
    else {
        px_srcline(1272);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1777, px_str("GETF"), _v1780, _v1778, _v1779}, 5));
        px_srcline(1273);
        _v1781 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1777}, 1);
        px_srcline(1274);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1776, _v1781, _v1777}, 3));
        px_srcline(1275);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1777, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1775}, 1)}, 1), _v1780, _v1780, _v1781}, 5));
    }
    px_srcline(1276);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1777, px_str("SETF"), _v1780, _v1778, _v1779}, 5));
px_err_1782:
    if (px_err_1782_proped) return px_err_1782_val;
    return px_null();
}

static LXValue fn_bc_assign_op_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_op_name");
    LXValue _v1783 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1784_val = px_null();
    int px_err_1784_proped = 0;
    px_srcline(1279);
    if (px_is_truthy(px_eq(_v1783, px_str("Plus")))) {
        px_srcline(1280);
        return px_str("Add");
    }
    px_srcline(1281);
    if (px_is_truthy(px_eq(_v1783, px_str("Minus")))) {
        px_srcline(1282);
        return px_str("Sub");
    }
    px_srcline(1283);
    if (px_is_truthy(px_eq(_v1783, px_str("Star")))) {
        px_srcline(1284);
        return px_str("Mul");
    }
    px_srcline(1285);
    if (px_is_truthy(px_eq(_v1783, px_str("Slash")))) {
        px_srcline(1286);
        return px_str("Div");
    }
    px_srcline(1287);
    if (px_is_truthy(px_eq(_v1783, px_str("IntDiv")))) {
        px_srcline(1288);
        return px_str("IntDiv");
    }
    px_srcline(1289);
    if (px_is_truthy(px_eq(_v1783, px_str("Mod")))) {
        px_srcline(1290);
        return px_str("Mod");
    }
    px_srcline(1291);
    if (px_is_truthy(px_eq(_v1783, px_str("Pow")))) {
        px_srcline(1292);
        return px_str("Pow");
    }
    px_srcline(1293);
    if (px_is_truthy(px_eq(_v1783, px_str("BitAnd")))) {
        px_srcline(1294);
        return px_str("BitAnd");
    }
    px_srcline(1295);
    if (px_is_truthy(px_eq(_v1783, px_str("BitOr")))) {
        px_srcline(1296);
        return px_str("BitOr");
    }
    px_srcline(1297);
    if (px_is_truthy(px_eq(_v1783, px_str("BitXor")))) {
        px_srcline(1298);
        return px_str("BitXor");
    }
    px_srcline(1299);
    if (px_is_truthy(px_eq(_v1783, px_str("Shl")))) {
        px_srcline(1300);
        return px_str("Shl");
    }
    px_srcline(1301);
    if (px_is_truthy(px_eq(_v1783, px_str("Shr")))) {
        px_srcline(1302);
        return px_str("Shr");
    }
    px_srcline(1303);
    if (px_is_truthy(px_eq(_v1783, px_str("ShrU")))) {
        px_srcline(1304);
        return px_str("ShrU");
    }
    px_srcline(1305);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_assign_op_name 未知赋值 op: "), px_call(px_get_global("str"), (LXValue[]){_v1783}, 1))}, 1));
px_err_1784:
    if (px_err_1784_proped) return px_err_1784_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt_inner");
    LXValue _v1785 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1786 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1787 = px_null();
    LXValue _v1788 = px_null();
    LXValue _v1789 = px_null();
    LXValue _v1790 = px_null();
    LXValue _v1791 = px_null();
    LXValue _v1792 = px_null();
    LXValue _v1793 = px_null();
    LXValue _v1794 = px_null();
    LXValue _v1795 = px_null();
    LXValue _v1796 = px_null();
    LXValue _v1797 = px_null();
    LXValue _v1798 = px_null();
    LXValue _v1799 = px_null();
    LXValue _v1800 = px_null();
    LXValue _v1801 = px_null();
    LXValue _v1802 = px_null();
    LXValue _v1803 = px_null();
    LXValue _v1804 = px_null();
    LXValue _v1805 = px_null();
    LXValue _v1806 = px_null();
    LXValue _v1807 = px_null();
    LXValue _v1808 = px_null();
    LXValue _v1809 = px_null();
    LXValue _v1810 = px_null();
    LXValue _v1811 = px_null();
    LXValue _v1812 = px_null();
    LXValue _v1813 = px_null();
    LXValue _v1814 = px_null();
    LXValue _v1815 = px_null();
    LXValue _v1816 = px_null();
    LXValue px_err_1817_val = px_null();
    int px_err_1817_proped = 0;
    px_srcline(1308);
    _v1787 = px_index(_v1785, px_int(0LL));
    px_srcline(1309);
    if (px_is_truthy(px_eq(_v1787, px_str("VarDecl")))) {
        px_srcline(1312);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v1785}, 1));
        px_srcline(1314);
        _v1788 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1785, px_int(2LL))}, 1);
        px_srcline(1315);
        if (px_is_truthy(px_eq(px_index(_v1785, px_int(4LL)), px_null()))) {
            px_srcline(1318);
            if (px_is_truthy(px_index(_v1786, px_str("is_top")))) {
                px_srcline(1319);
                _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
                px_srcline(1320);
                (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1786, _v1789}, 2));
                px_srcline(1321);
                _v1790 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1788}, 2);
                px_srcline(1322);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("SETG"), _v1790, _v1789, px_int(0LL)}, 5));
            }
            px_srcline(1323);
            return px_null();
        }
        px_srcline(1324);
        if (px_is_truthy(px_index(_v1786, px_str("is_top")))) {
            px_srcline(1326);
            _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
            px_srcline(1327);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1785, px_int(4LL)), _v1789, _v1786}, 3));
            px_srcline(1328);
            _v1790 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1788}, 2);
            px_srcline(1329);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("SETG"), _v1790, _v1789, px_int(0LL)}, 5));
            px_srcline(1330);
            return px_null();
        }
        px_srcline(1334);
        _v1791 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1786, _v1788}, 2);
        px_srcline(1335);
        _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
        px_srcline(1336);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1785, px_int(4LL)), _v1789, _v1786}, 3));
        px_srcline(1337);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1786, _v1788, _v1791, _v1789}, 4));
        px_srcline(1338);
        return px_null();
    }
    px_srcline(1339);
    if (px_is_truthy(px_eq(_v1787, px_str("Assign")))) {
        px_srcline(1342);
        _v1792 = px_index(_v1785, px_int(1LL));
        px_srcline(1343);
        _v1793 = px_index(_v1785, px_int(2LL));
        px_srcline(1344);
        _v1794 = px_index(_v1785, px_int(3LL));
        px_srcline(1345);
        if (px_is_truthy(px_eq(_v1793, px_str("Append")))) {
            px_srcline(1346);
            _v1795 = px_list_n((LXValue[]){_v1794}, 1);
            px_srcline(1347);
            (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){_v1792, px_str("append"), _v1795, px_neg(px_int(1LL)), _v1786}, 5));
            px_srcline(1348);
            return px_null();
        }
        px_srcline(1351);
        (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v1792, _v1793, _v1794}, 3));
        px_srcline(1352);
        _v1796 = px_index(_v1792, px_int(0LL));
        px_srcline(1353);
        if (px_is_truthy(px_eq(_v1796, px_str("Var")))) {
            px_srcline(1354);
            _v1788 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1792, px_int(1LL))}, 1);
            px_srcline(1355);
            if (px_is_truthy(px_method(px_index(_v1786, px_str("smap")), "has", (LXValue[]){_v1788}, 1))) {
                px_srcline(1357);
                if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1786, _v1788}, 2))) {
                    px_srcline(1358);
                    _v1797 = px_index(px_index(_v1786, px_str("smap")), _v1788);
                    px_srcline(1359);
                    if (px_is_truthy(px_eq(_v1793, px_str("Assign")))) {
                        px_srcline(1360);
                        _v1798 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
                        px_srcline(1361);
                        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1794, _v1798, _v1786}, 3));
                        px_srcline(1362);
                        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("CELLSET"), _v1798, _v1797, px_int(0LL)}, 5));
                        px_srcline(1363);
                        return px_null();
                    }
                    px_srcline(1364);
                    _v1799 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
                    px_srcline(1365);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("CELLGET"), _v1799, _v1797, px_int(0LL)}, 5));
                    px_srcline(1366);
                    _v1800 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
                    px_srcline(1367);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1794, _v1800, _v1786}, 3));
                    px_srcline(1368);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1793}, 1)}, 1), _v1799, _v1799, _v1800}, 5));
                    px_srcline(1369);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("CELLSET"), _v1799, _v1797, px_int(0LL)}, 5));
                    px_srcline(1370);
                    return px_null();
                }
                px_srcline(1371);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){px_index(px_index(_v1786, px_str("smap")), _v1788), _v1793, _v1794, _v1786}, 4));
                px_srcline(1372);
                return px_null();
            }
            px_srcline(1373);
            if (px_is_truthy(({ LXValue _t1818 = px_not(px_index(_v1786, px_str("is_top"))); px_is_truthy(_t1818) ? px_not(px_call(px_get_global("bc_is_global"), (LXValue[]){_v1788}, 1)) : _t1818; }))) {
                px_srcline(1375);
                _v1791 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1786, _v1788}, 2);
                px_srcline(1376);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){_v1791, _v1793, _v1794, _v1786}, 4));
                px_srcline(1377);
                return px_null();
            }
            px_srcline(1379);
            (void)(px_call(px_get_global("bc_assign_global"), (LXValue[]){_v1788, _v1793, _v1794, _v1786}, 4));
            px_srcline(1380);
            return px_null();
        }
        px_srcline(1381);
        if (px_is_truthy(px_eq(_v1796, px_str("Index")))) {
            px_srcline(1382);
            (void)(px_call(px_get_global("bc_assign_index"), (LXValue[]){_v1792, _v1793, _v1794, _v1786}, 4));
            px_srcline(1383);
            return px_null();
        }
        px_srcline(1384);
        if (px_is_truthy(px_eq(_v1796, px_str("Field")))) {
            px_srcline(1385);
            (void)(px_call(px_get_global("bc_assign_field"), (LXValue[]){_v1792, _v1793, _v1794, _v1786}, 4));
            px_srcline(1386);
            return px_null();
        }
        px_srcline(1387);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt Assign 目标未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1792}, 1))}, 1));
    }
    px_srcline(1388);
    if (px_is_truthy(px_eq(_v1787, px_str("Return")))) {
        px_srcline(1389);
        if (px_is_truthy(px_ne(px_index(_v1785, px_int(1LL)), px_null()))) {
            px_srcline(1390);
            _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
            px_srcline(1391);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1785, px_int(1LL)), _v1789, _v1786}, 3));
            px_srcline(1392);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("RET"), _v1789, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1393);
            return px_null();
        }
        px_srcline(1394);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1395);
        return px_null();
    }
    px_srcline(1396);
    if (px_is_truthy(px_eq(_v1787, px_str("ExprStmt")))) {
        px_srcline(1398);
        _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
        px_srcline(1399);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1785, px_int(1LL)), _v1789, _v1786}, 3));
        px_srcline(1400);
        return px_null();
    }
    px_srcline(1401);
    if (px_is_truthy(px_eq(_v1787, px_str("If")))) {
        px_srcline(1402);
        (void)(px_call(px_get_global("bc_emit_if"), (LXValue[]){_v1785, _v1786}, 2));
        px_srcline(1403);
        return px_null();
    }
    px_srcline(1404);
    if (px_is_truthy(px_eq(_v1787, px_str("While")))) {
        px_srcline(1405);
        (void)(px_call(px_get_global("bc_emit_while"), (LXValue[]){_v1785, _v1786}, 2));
        px_srcline(1406);
        return px_null();
    }
    px_srcline(1407);
    if (px_is_truthy(px_eq(_v1787, px_str("For")))) {
        px_srcline(1408);
        (void)(px_call(px_get_global("bc_emit_for"), (LXValue[]){_v1785, _v1786}, 2));
        px_srcline(1409);
        return px_null();
    }
    px_srcline(1410);
    if (px_is_truthy(px_eq(_v1787, px_str("Break")))) {
        px_srcline(1412);
        _v1801 = px_index(_v1786, px_str("loops"));
        px_srcline(1413);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1801}, 1), px_int(0LL)))) {
            px_srcline(1414);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Break 不在循环内")}, 1));
        }
        px_srcline(1415);
        _v1802 = px_index(_v1801, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1801}, 1), px_int(1LL)));
        px_srcline(1416);
        _v1803 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1786, px_str("bc"))}, 1);
        px_srcline(1417);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1418);
        (void)(px_method(px_index(_v1802, px_str("breaks")), "append", (LXValue[]){_v1803}, 1));
        px_srcline(1419);
        return px_null();
    }
    px_srcline(1420);
    if (px_is_truthy(px_eq(_v1787, px_str("Continue")))) {
        px_srcline(1422);
        _v1801 = px_index(_v1786, px_str("loops"));
        px_srcline(1423);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1801}, 1), px_int(0LL)))) {
            px_srcline(1424);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Continue 不在循环内")}, 1));
        }
        px_srcline(1425);
        _v1802 = px_index(_v1801, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1801}, 1), px_int(1LL)));
        px_srcline(1426);
        _v1803 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1786, px_str("bc"))}, 1);
        px_srcline(1427);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1428);
        (void)(px_method(px_index(_v1802, px_str("conts")), "append", (LXValue[]){_v1803}, 1));
        px_srcline(1429);
        return px_null();
    }
    px_srcline(1430);
    if (px_is_truthy(px_eq(_v1787, px_str("Empty")))) {
        px_srcline(1431);
        return px_null();
    }
    px_srcline(1432);
    if (px_is_truthy(px_eq(_v1787, px_str("TypeConst")))) {
        px_srcline(1434);
        return px_null();
    }
    px_srcline(1435);
    if (px_is_truthy(px_eq(_v1787, px_str("ChanDecl")))) {
        px_srcline(1437);
        _v1788 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1785, px_int(1LL))}, 1);
        px_srcline(1438);
        _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
        px_srcline(1439);
        _v1797 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
        px_srcline(1440);
        _v1804 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan")}, 2);
        px_srcline(1441);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("GETG"), _v1797, _v1804, px_int(0LL)}, 5));
        px_srcline(1442);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("CALL"), _v1789, _v1797, px_int(0LL)}, 5));
        px_srcline(1443);
        if (px_is_truthy(px_index(_v1786, px_str("is_top")))) {
            px_srcline(1444);
            _v1790 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1788}, 2);
            px_srcline(1445);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("SETG"), _v1790, _v1789, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1447);
            _v1791 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1786, _v1788}, 2);
            px_srcline(1448);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("MOV"), _v1791, _v1789, px_int(0LL)}, 5));
        }
        px_srcline(1449);
        return px_null();
    }
    px_srcline(1450);
    if (px_is_truthy(px_eq(_v1787, px_str("Send")))) {
        px_srcline(1452);
        _v1805 = px_list_n((LXValue[]){px_index(_v1785, px_int(2LL))}, 1);
        px_srcline(1453);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1785, px_int(1LL)), px_str("send"), _v1805, px_neg(px_int(1LL)), _v1786}, 5));
        px_srcline(1454);
        return px_null();
    }
    px_srcline(1455);
    if (px_is_truthy(px_eq(_v1787, px_str("Recv")))) {
        px_srcline(1457);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1785, px_int(1LL)), px_str("recv"), px_list_n((LXValue[]){}, 0), px_neg(px_int(1LL)), _v1786}, 5));
        px_srcline(1458);
        return px_null();
    }
    px_srcline(1459);
    if (px_is_truthy(px_eq(_v1787, px_str("Spawn")))) {
        px_srcline(1462);
        _v1806 = px_index(_v1785, px_int(1LL));
        px_srcline(1463);
        if (px_is_truthy(({ LXValue _t1819 = px_eq(px_index(_v1806, px_int(0LL)), px_str("Call")); px_is_truthy(_t1819) ? px_eq(px_index(px_index(_v1806, px_int(1LL)), px_int(0LL)), px_str("Var")) : _t1819; }))) {
            px_srcline(1464);
            _v1807 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1806, px_int(1LL)), px_int(1LL))}, 1);
            px_srcline(1465);
            _v1808 = px_index(_v1806, px_int(2LL));
            px_srcline(1466);
            _v1789 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
            px_srcline(1467);
            _v1797 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
            px_srcline(1468);
            _v1809 = px_add(px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1808}, 1));
            px_srcline(1469);
            _v1810 = px_add(px_add(_v1797, px_int(1LL)), _v1809);
            px_srcline(1470);
            while (px_is_truthy(px_lt(px_index(_v1786, px_str("next_slot")), _v1810))) {
                px_srcline(1471);
                (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1));
            }
            px_srcline(1472);
            _v1804 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("spawn")}, 2);
            px_srcline(1473);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("GETG"), _v1797, _v1804, px_int(0LL)}, 5));
            px_srcline(1474);
            _v1811 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1807}, 5);
            px_srcline(1475);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("LOADK"), px_add(_v1797, px_int(1LL)), _v1811, px_int(0LL)}, 5));
            px_srcline(1476);
            _v1812 = px_int(0LL);
            px_srcline(1477);
            while (px_is_truthy(px_lt(_v1812, px_call(px_get_global("len"), (LXValue[]){_v1808}, 1)))) {
                px_srcline(1478);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1808, _v1812), px_add(px_add(_v1797, px_int(2LL)), _v1812), _v1786}, 3));
                px_srcline(1479);
                 _v1812 = px_add(_v1812, px_int(1LL));
            }
            px_srcline(1480);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1786, px_str("CALL"), _v1789, _v1797, _v1809}, 5));
            px_srcline(1481);
            return px_null();
        }
        px_srcline(1486);
        _v1813 = px_index(_v1785, px_int(2LL));
        px_srcline(1487);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1813, px_int(0LL))}, 1)), px_str(":")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1813, px_int(1LL))}, 1)), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()），多行匿名函数体见 M118"))}, 1));
        px_srcline(1488);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(1489);
    if (px_is_truthy(px_eq(_v1787, px_str("Select")))) {
        px_srcline(1490);
        (void)(px_call(px_get_global("bc_emit_select"), (LXValue[]){_v1785, _v1786}, 2));
        px_srcline(1491);
        return px_null();
    }
    px_srcline(1492);
    if (px_is_truthy(px_eq(_v1787, px_str("FuncDef")))) {
        px_srcline(1497);
        _v1814 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1785, px_int(1LL))}, 1);
        px_srcline(1498);
        _v1815 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1786, _v1814}, 2);
        px_srcline(1499);
        _v1816 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1786}, 1);
        px_srcline(1500);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1785, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1785, px_int(4LL))}, 2), _v1816, _v1786}, 4));
        px_srcline(1501);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1786, _v1814, _v1815, _v1816}, 4));
        px_srcline(1502);
        return px_null();
    }
    px_srcline(1503);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1785}, 1))}, 1));
px_err_1817:
    if (px_err_1817_proped) return px_err_1817_val;
    return px_null();
}

static LXValue fn_bc_emit_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_select");
    LXValue _v1820 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1821 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v1835 = px_null();
    LXValue _v1836 = px_null();
    LXValue _v1837 = px_null();
    LXValue _v1838 = px_null();
    LXValue _v1839 = px_null();
    LXValue _v1840 = px_null();
    LXValue _v1841 = px_null();
    LXValue _v1842 = px_null();
    LXValue _v1843 = px_null();
    LXValue _v1844 = px_null();
    LXValue _v1845 = px_null();
    LXValue _v1846 = px_null();
    LXValue px_err_1847_val = px_null();
    int px_err_1847_proped = 0;
    px_srcline(1511);
    _v1822 = px_index(_v1820, px_int(1LL));
    px_srcline(1512);
    _v1823 = px_index(_v1820, px_int(2LL));
    px_srcline(1513);
    _v1824 = px_call(px_get_global("len"), (LXValue[]){_v1822}, 1);
    px_srcline(1515);
    _v1825 = px_list_n((LXValue[]){}, 0);
    px_srcline(1516);
    _v1826 = px_int(0LL);
    px_srcline(1517);
    while (px_is_truthy(px_lt(_v1826, _v1824))) {
        px_srcline(1518);
        _v1827 = px_index(px_index(_v1822, _v1826), px_int(1LL));
        px_srcline(1520);
        if (px_is_truthy(({ LXValue _t1848 = px_ne(px_index(_v1827, px_int(0LL)), px_str("Call")); px_is_truthy(_t1848) ? _t1848 : px_ne(px_index(px_index(_v1827, px_int(1LL)), px_int(0LL)), px_str("Field")); }))) {
            px_srcline(1521);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit select case 仅支持 ch.recv(): "), px_call(px_get_global("str"), (LXValue[]){_v1827}, 1))}, 1));
        }
        px_srcline(1522);
        _v1828 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1821}, 1);
        px_srcline(1523);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1827, px_int(1LL)), px_int(1LL)), _v1828, _v1821}, 3));
        px_srcline(1524);
        (void)(px_method(_v1825, "append", (LXValue[]){_v1828}, 1));
        px_srcline(1525);
         _v1826 = px_add(_v1826, px_int(1LL));
    }
    px_srcline(1526);
    _v1829 = px_neg(px_int(1LL));
    px_srcline(1527);
    if (px_is_truthy(px_eq(_v1823, px_null()))) {
        px_srcline(1528);
         _v1829 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1);
    }
    px_srcline(1529);
    _v1830 = px_neg(px_int(1LL));
    px_srcline(1530);
    _v1831 = px_list_n((LXValue[]){}, 0);
    px_srcline(1531);
    _v1832 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan_try_recv")}, 2);
    px_srcline(1532);
    _v1833 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(1533);
     _v1826 = px_int(0LL);
    px_srcline(1534);
    while (px_is_truthy(px_lt(_v1826, _v1824))) {
        px_srcline(1535);
        if (px_is_truthy(px_ge(_v1830, px_int(0LL)))) {
            px_srcline(1536);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1821, _v1830, px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1538);
        _v1834 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1821}, 1);
        px_srcline(1539);
        _v1835 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1821}, 1);
        px_srcline(1540);
        _v1836 = px_add(_v1834, px_int(2LL));
        px_srcline(1541);
        while (px_is_truthy(px_lt(px_index(_v1821, px_str("next_slot")), _v1836))) {
            px_srcline(1542);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1821}, 1));
        }
        px_srcline(1543);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("GETG"), _v1834, _v1832, px_int(0LL)}, 5));
        px_srcline(1544);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("MOV"), px_add(_v1834, px_int(1LL)), px_index(_v1825, _v1826), px_int(0LL)}, 5));
        px_srcline(1545);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("CALL"), _v1835, _v1834, px_int(1LL)}, 5));
        px_srcline(1547);
        _v1837 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1821}, 1);
        px_srcline(1548);
        _v1838 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1821}, 1);
        px_srcline(1549);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("LOADK"), _v1838, _v1833, px_int(0LL)}, 5));
        px_srcline(1550);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("EQ"), _v1837, _v1835, _v1838}, 5));
        px_srcline(1551);
        _v1839 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1);
        px_srcline(1552);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("JMPT"), _v1837, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1553);
         _v1830 = _v1839;
        px_srcline(1555);
        _v1840 = px_index(px_index(_v1822, _v1826), px_int(0LL));
        px_srcline(1556);
        if (px_is_truthy(px_ne(_v1840, px_null()))) {
            px_srcline(1557);
            _v1841 = px_call(px_get_global("rust_unescape"), (LXValue[]){_v1840}, 1);
            px_srcline(1558);
            _v1842 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1821, _v1841}, 2);
            px_srcline(1559);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("MOV"), _v1842, _v1835, px_int(0LL)}, 5));
        }
        px_srcline(1560);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(px_index(_v1822, _v1826), px_int(2LL)), _v1821}, 2));
        px_srcline(1561);
        _v1843 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1);
        px_srcline(1562);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1563);
        (void)(px_method(_v1831, "append", (LXValue[]){_v1843}, 1));
        px_srcline(1564);
         _v1826 = px_add(_v1826, px_int(1LL));
    }
    px_srcline(1566);
    if (px_is_truthy(px_ge(_v1830, px_int(0LL)))) {
        px_srcline(1567);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1821, _v1830, px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1)}, 3));
    }
    px_srcline(1568);
    if (px_is_truthy(px_ne(_v1823, px_null()))) {
        px_srcline(1569);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1823, _v1821}, 2));
    }
    else {
        px_srcline(1571);
        _v1844 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1);
        px_srcline(1572);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1821, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1573);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1821, _v1844, _v1829}, 3));
    }
    px_srcline(1574);
    _v1845 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1821, px_str("bc"))}, 1);
    px_srcline(1575);
    _v1846 = px_int(0LL);
    px_srcline(1576);
    while (px_is_truthy(px_lt(_v1846, px_call(px_get_global("len"), (LXValue[]){_v1831}, 1)))) {
        px_srcline(1577);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1821, px_index(_v1831, _v1846), _v1845}, 3));
        px_srcline(1578);
         _v1846 = px_add(_v1846, px_int(1LL));
    }
px_err_1847:
    if (px_err_1847_proped) return px_err_1847_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt");
    LXValue _v1849 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1850 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1851 = px_null();
    LXValue _v1852 = px_null();
    LXValue px_err_1853_val = px_null();
    int px_err_1853_proped = 0;
    px_srcline(1581);
    _v1851 = px_int(0LL);
    px_srcline(1582);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1849}, 1), px_int(0LL)))) {
        px_srcline(1583);
        _v1852 = px_index(_v1849, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1849}, 1), px_int(1LL)));
        px_srcline(1584);
        if (px_is_truthy(({ LXValue _t1855 = ({ LXValue _t1854 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1852}, 1), px_str("list")); px_is_truthy(_t1854) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1852}, 1), px_int(1LL)) : _t1854; }); px_is_truthy(_t1855) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1852, px_int(0LL))}, 1), px_str("int")) : _t1855; }))) {
            px_srcline(1585);
             _v1851 = px_index(_v1852, px_int(0LL));
        }
    }
    px_srcline(1586);
    if (px_is_truthy(px_gt(_v1851, px_int(0LL)))) {
        px_srcline(1587);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1850, px_str("SRCLINE"), px_int(0LL), _v1851, px_int(0LL)}, 5));
    }
    px_srcline(1588);
    (void)(px_call(px_get_global("bc_emit_stmt_inner"), (LXValue[]){_v1849, _v1850}, 2));
px_err_1853:
    if (px_err_1853_proped) return px_err_1853_val;
    return px_null();
}

static LXValue fn_bc_emit_stmts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmts");
    LXValue _v1856 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1857 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1858 = px_null();
    LXValue px_err_1859_val = px_null();
    int px_err_1859_proped = 0;
    px_srcline(1590);
    _v1858 = px_int(0LL);
    px_srcline(1591);
    while (px_is_truthy(px_lt(_v1858, px_call(px_get_global("len"), (LXValue[]){_v1856}, 1)))) {
        px_srcline(1592);
        (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){px_index(_v1856, _v1858), _v1857}, 2));
        px_srcline(1593);
         _v1858 = px_add(_v1858, px_int(1LL));
    }
px_err_1859:
    if (px_err_1859_proped) return px_err_1859_val;
    return px_null();
}

static LXValue fn_bc_emit_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_if");
    LXValue _v1860 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1861 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1862 = px_null();
    LXValue _v1863 = px_null();
    LXValue _v1864 = px_null();
    LXValue _v1865 = px_null();
    LXValue _v1866 = px_null();
    LXValue _v1867 = px_null();
    LXValue _v1868 = px_null();
    LXValue _v1869 = px_null();
    LXValue _v1870 = px_null();
    LXValue _v1871 = px_null();
    LXValue _v1872 = px_null();
    LXValue _v1873 = px_null();
    LXValue px_err_1874_val = px_null();
    int px_err_1874_proped = 0;
    px_srcline(1599);
    _v1862 = px_index(_v1860, px_int(1LL));
    px_srcline(1600);
    _v1863 = px_index(_v1860, px_int(2LL));
    px_srcline(1601);
    _v1864 = px_call(px_get_global("len"), (LXValue[]){_v1862}, 1);
    px_srcline(1602);
    _v1865 = px_list_n((LXValue[]){}, 0);
    px_srcline(1603);
    _v1866 = px_int(0LL);
    px_srcline(1604);
    while (px_is_truthy(px_lt(_v1866, _v1864))) {
        px_srcline(1605);
        _v1867 = px_index(px_index(_v1862, _v1866), px_int(0LL));
        px_srcline(1606);
        _v1868 = px_index(px_index(_v1862, _v1866), px_int(1LL));
        px_srcline(1607);
        _v1869 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1861}, 1);
        px_srcline(1608);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1867, _v1869, _v1861}, 3));
        px_srcline(1609);
        _v1870 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1861, px_str("bc"))}, 1);
        px_srcline(1610);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1861, px_str("JMPF"), _v1869, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1611);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1868, _v1861}, 2));
        px_srcline(1612);
        if (px_is_truthy(({ LXValue _t1875 = px_lt(_v1866, px_sub(_v1864, px_int(1LL))); px_is_truthy(_t1875) ? _t1875 : px_ne(_v1863, px_null()); }))) {
            px_srcline(1615);
            _v1871 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1861, px_str("bc"))}, 1);
            px_srcline(1616);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1861, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1617);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1861, _v1870, px_call(px_get_global("len"), (LXValue[]){px_index(_v1861, px_str("bc"))}, 1)}, 3));
            px_srcline(1618);
            (void)(px_method(_v1865, "append", (LXValue[]){_v1871}, 1));
        }
        else {
            px_srcline(1621);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1861, _v1870, px_call(px_get_global("len"), (LXValue[]){px_index(_v1861, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1622);
         _v1866 = px_add(_v1866, px_int(1LL));
    }
    px_srcline(1623);
    if (px_is_truthy(px_ne(_v1863, px_null()))) {
        px_srcline(1624);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1863, _v1861}, 2));
    }
    px_srcline(1625);
    _v1872 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1861, px_str("bc"))}, 1);
    px_srcline(1626);
    _v1873 = px_int(0LL);
    px_srcline(1627);
    while (px_is_truthy(px_lt(_v1873, px_call(px_get_global("len"), (LXValue[]){_v1865}, 1)))) {
        px_srcline(1628);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1861, px_index(_v1865, _v1873), _v1872}, 3));
        px_srcline(1629);
         _v1873 = px_add(_v1873, px_int(1LL));
    }
px_err_1874:
    if (px_err_1874_proped) return px_err_1874_val;
    return px_null();
}

static LXValue fn_bc_emit_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_while");
    LXValue _v1876 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1877 = (nargs > 1) ? args[1] : px_null();
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
    px_srcline(1634);
    _v1878 = px_index(_v1876, px_int(1LL));
    px_srcline(1635);
    _v1879 = px_index(_v1876, px_int(2LL));
    px_srcline(1636);
    _v1880 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1877, px_str("bc"))}, 1);
    px_srcline(1637);
    _v1881 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1877}, 1);
    px_srcline(1638);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1878, _v1881, _v1877}, 3));
    px_srcline(1639);
    _v1882 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1877, px_str("bc"))}, 1);
    px_srcline(1640);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("JMPF"), _v1881, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1641);
    _v1883 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1642);
    px_index_set(_v1883, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1643);
    px_index_set(_v1883, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1644);
    (void)(px_method(px_index(_v1877, px_str("loops")), "push", (LXValue[]){_v1883}, 1));
    px_srcline(1645);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1879, _v1877}, 2));
    px_srcline(1646);
    (void)(px_method(px_index(_v1877, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1647);
    _v1884 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1877, px_str("bc"))}, 1);
    px_srcline(1648);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1649);
    _v1885 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1877, px_str("bc"))}, 1);
    px_srcline(1650);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1877, _v1884, _v1880}, 3));
    px_srcline(1651);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1877, _v1882, _v1885}, 3));
    px_srcline(1652);
    _v1886 = px_int(0LL);
    px_srcline(1653);
    while (px_is_truthy(px_lt(_v1886, px_call(px_get_global("len"), (LXValue[]){px_index(_v1883, px_str("breaks"))}, 1)))) {
        px_srcline(1654);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1877, px_index(px_index(_v1883, px_str("breaks")), _v1886), _v1885}, 3));
        px_srcline(1655);
         _v1886 = px_add(_v1886, px_int(1LL));
    }
    px_srcline(1656);
     _v1886 = px_int(0LL);
    px_srcline(1657);
    while (px_is_truthy(px_lt(_v1886, px_call(px_get_global("len"), (LXValue[]){px_index(_v1883, px_str("conts"))}, 1)))) {
        px_srcline(1658);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1877, px_index(px_index(_v1883, px_str("conts")), _v1886), _v1880}, 3));
        px_srcline(1659);
         _v1886 = px_add(_v1886, px_int(1LL));
    }
px_err_1887:
    if (px_err_1887_proped) return px_err_1887_val;
    return px_null();
}

static LXValue fn_bc_emit_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_for");
    LXValue _v1888 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1889 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1890 = px_null();
    LXValue _v1891 = px_null();
    LXValue _v1892 = px_null();
    LXValue _v1893 = px_null();
    LXValue _v1894 = px_null();
    LXValue _v1895 = px_null();
    LXValue _v1896 = px_null();
    LXValue _v1897 = px_null();
    LXValue _v1898 = px_null();
    LXValue _v1899 = px_null();
    LXValue _v1900 = px_null();
    LXValue _v1901 = px_null();
    LXValue _v1902 = px_null();
    LXValue _v1903 = px_null();
    LXValue _v1904 = px_null();
    LXValue _v1905 = px_null();
    LXValue _v1906 = px_null();
    LXValue _v1907 = px_null();
    LXValue px_err_1908_val = px_null();
    int px_err_1908_proped = 0;
    px_srcline(1666);
    _v1890 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1888, px_int(1LL))}, 1);
    px_srcline(1667);
    _v1891 = px_index(_v1888, px_int(2LL));
    px_srcline(1668);
    _v1892 = px_index(_v1888, px_int(3LL));
    px_srcline(1669);
    _v1893 = px_neg(px_int(1LL));
    px_srcline(1670);
    if (px_is_truthy(px_method(px_index(_v1889, px_str("smap")), "has", (LXValue[]){_v1890}, 1))) {
        px_srcline(1671);
         _v1893 = px_index(px_index(_v1889, px_str("smap")), _v1890);
    }
    else {
        px_srcline(1673);
         _v1893 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1889, _v1890}, 2);
    }
    px_srcline(1675);
    _v1894 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1);
    px_srcline(1676);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1891, _v1894, _v1889}, 3));
    px_srcline(1678);
    _v1895 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1);
    px_srcline(1679);
    _v1896 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1);
    px_srcline(1680);
    while (px_is_truthy(px_lt(px_index(_v1889, px_str("next_slot")), px_add(_v1895, px_int(2LL))))) {
        px_srcline(1681);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1));
    }
    px_srcline(1682);
    _v1897 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1683);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("GETG"), _v1895, _v1897, px_int(0LL)}, 5));
    px_srcline(1684);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("MOV"), px_add(_v1895, px_int(1LL)), _v1894, px_int(0LL)}, 5));
    px_srcline(1685);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("CALL"), _v1896, _v1895, px_int(1LL)}, 5));
    px_srcline(1687);
    _v1898 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1);
    px_srcline(1688);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1889, _v1898, px_int(0LL)}, 3));
    px_srcline(1689);
    _v1899 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1889, px_str("bc"))}, 1);
    px_srcline(1691);
    _v1900 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1);
    px_srcline(1692);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("LT"), _v1900, _v1898, _v1896}, 5));
    px_srcline(1693);
    _v1901 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1889, px_str("bc"))}, 1);
    px_srcline(1694);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("JMPF"), _v1900, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1696);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("INDEX"), _v1893, _v1894, _v1898}, 5));
    px_srcline(1697);
    _v1902 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1698);
    px_index_set(_v1902, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1699);
    px_index_set(_v1902, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1700);
    (void)(px_method(px_index(_v1889, px_str("loops")), "push", (LXValue[]){_v1902}, 1));
    px_srcline(1701);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1892, _v1889}, 2));
    px_srcline(1702);
    (void)(px_method(px_index(_v1889, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1707);
    _v1903 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1889, px_str("bc"))}, 1);
    px_srcline(1708);
    _v1904 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1889}, 1);
    px_srcline(1709);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1889, _v1904, px_int(1LL)}, 3));
    px_srcline(1710);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("ADD"), _v1898, _v1898, _v1904}, 5));
    px_srcline(1711);
    _v1905 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1889, px_str("bc"))}, 1);
    px_srcline(1712);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1889, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1713);
    _v1906 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1889, px_str("bc"))}, 1);
    px_srcline(1714);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1889, _v1905, _v1899}, 3));
    px_srcline(1715);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1889, _v1901, _v1906}, 3));
    px_srcline(1716);
    _v1907 = px_int(0LL);
    px_srcline(1717);
    while (px_is_truthy(px_lt(_v1907, px_call(px_get_global("len"), (LXValue[]){px_index(_v1902, px_str("breaks"))}, 1)))) {
        px_srcline(1718);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1889, px_index(px_index(_v1902, px_str("breaks")), _v1907), _v1906}, 3));
        px_srcline(1719);
         _v1907 = px_add(_v1907, px_int(1LL));
    }
    px_srcline(1720);
     _v1907 = px_int(0LL);
    px_srcline(1721);
    while (px_is_truthy(px_lt(_v1907, px_call(px_get_global("len"), (LXValue[]){px_index(_v1902, px_str("conts"))}, 1)))) {
        px_srcline(1722);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1889, px_index(px_index(_v1902, px_str("conts")), _v1907), _v1903}, 3));
        px_srcline(1723);
         _v1907 = px_add(_v1907, px_int(1LL));
    }
px_err_1908:
    if (px_err_1908_proped) return px_err_1908_val;
    return px_null();
}

static LXValue fn_bc_emit_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_def");
    LXValue _v1909 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1910 = px_null();
    LXValue px_err_1911_val = px_null();
    int px_err_1911_proped = 0;
    px_srcline(1727);
    _v1910 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1909, px_int(1LL))}, 1);
    px_srcline(1728);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v1909, _v1910}, 2);
px_err_1911:
    if (px_err_1911_proped) return px_err_1911_val;
    return px_null();
}

static LXValue fn_bc_emit_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_impl_def");
    LXValue _v1912 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1913 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1914 = px_null();
    LXValue px_err_1915_val = px_null();
    int px_err_1915_proped = 0;
    px_srcline(1731);
    _v1914 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1912, px_int(1LL))}, 1);
    px_srcline(1732);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v1912, px_add(px_add(_v1913, px_str(".")), _v1914)}, 2);
px_err_1915:
    if (px_err_1915_proped) return px_err_1915_val;
    return px_null();
}

static LXValue fn_bc_emit_func_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_body");
    LXValue _v1916 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1917 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1918 = px_null();
    LXValue _v1919 = px_null();
    LXValue _v1920 = px_null();
    LXValue _v1921 = px_null();
    LXValue _v1922 = px_null();
    LXValue _v1923 = px_null();
    LXValue _v1924 = px_null();
    LXValue _v1925 = px_null();
    LXValue _v1926 = px_null();
    LXValue px_err_1927_val = px_null();
    int px_err_1927_proped = 0;
    px_srcline(1741);
    _v1918 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(1742);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1918}, 1));
    px_srcline(1743);
    _v1919 = px_index(_v1916, px_int(2LL));
    px_srcline(1748);
    _v1920 = px_call(px_get_global("len"), (LXValue[]){_v1919}, 1);
    px_srcline(1749);
    _v1921 = px_int(0LL);
    px_srcline(1750);
    _v1922 = px_int(0LL);
    px_srcline(1751);
    while (px_is_truthy(px_lt(_v1922, px_call(px_get_global("len"), (LXValue[]){_v1919}, 1)))) {
        px_srcline(1752);
        if (px_is_truthy(px_ne(px_index(px_index(_v1919, _v1922), px_int(3LL)), px_null()))) {
            px_srcline(1753);
            if (px_is_truthy(px_eq(_v1921, px_int(0LL)))) {
                px_srcline(1754);
                 _v1920 = _v1922;
            }
            px_srcline(1755);
             _v1921 = px_add(_v1921, px_int(1LL));
        }
        px_srcline(1756);
         _v1922 = px_add(_v1922, px_int(1LL));
    }
    px_srcline(1757);
    _v1923 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1917, _v1920}, 2);
    px_srcline(1758);
    px_index_set(_v1923, px_str("ndefault"), _v1921);
    px_srcline(1760);
     _v1922 = px_int(0LL);
    px_srcline(1761);
    while (px_is_truthy(px_lt(_v1922, px_call(px_get_global("len"), (LXValue[]){_v1919}, 1)))) {
        px_srcline(1762);
        px_index_set(px_index(_v1923, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1919, _v1922), px_int(1LL))}, 1), _v1922);
        px_srcline(1763);
         _v1922 = px_add(_v1922, px_int(1LL));
    }
    px_srcline(1764);
    if (px_is_truthy(px_lt(px_index(_v1923, px_str("next_slot")), px_call(px_get_global("len"), (LXValue[]){_v1919}, 1)))) {
        px_srcline(1765);
        px_index_set(_v1923, px_str("next_slot"), px_call(px_get_global("len"), (LXValue[]){_v1919}, 1));
    }
    px_srcline(1767);
    _v1924 = px_list_n((LXValue[]){}, 0);
    px_srcline(1768);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1916, px_int(4LL)), _v1924}, 2));
    px_srcline(1769);
    _v1925 = px_int(0LL);
    px_srcline(1770);
    while (px_is_truthy(px_lt(_v1925, px_call(px_get_global("len"), (LXValue[]){_v1924}, 1)))) {
        px_srcline(1771);
        _v1926 = px_index(_v1924, _v1925);
        px_srcline(1772);
        if (px_is_truthy(px_method(px_index(_v1923, px_str("smap")), "has", (LXValue[]){_v1926}, 1))) {
            px_srcline(1773);
             _v1925 = px_add(_v1925, px_int(1LL));
            px_srcline(1774);
            continue;
        }
        px_srcline(1775);
        if (px_is_truthy(px_call(px_get_global("bc_is_global"), (LXValue[]){_v1926}, 1))) {
            px_srcline(1776);
             _v1925 = px_add(_v1925, px_int(1LL));
            px_srcline(1777);
            continue;
        }
        px_srcline(1778);
        (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v1923, _v1926}, 2));
        px_srcline(1779);
         _v1925 = px_add(_v1925, px_int(1LL));
    }
    px_srcline(1784);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v1923, px_index(_v1916, px_int(4LL))}, 2));
    px_srcline(1786);
    if (px_is_truthy(px_gt(_v1921, px_int(0LL)))) {
        px_srcline(1787);
        (void)(px_call(px_get_global("bc_emit_default_fill"), (LXValue[]){_v1923, _v1919, _v1920}, 3));
    }
    px_srcline(1788);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(_v1916, px_int(4LL)), _v1923}, 2));
    px_srcline(1789);
    px_set_global("cg_immutables", _v1918);
    px_srcline(1790);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1923, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1791);
    px_index_set(_v1923, px_str("nslots"), px_index(_v1923, px_str("next_slot")));
    px_srcline(1792);
    return _v1923;
px_err_1927:
    if (px_err_1927_proped) return px_err_1927_val;
    return px_null();
}

static LXValue fn_bc_emit_default_fill(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_default_fill");
    LXValue _v1928 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1929 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1930 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1931 = px_null();
    LXValue _v1932 = px_null();
    LXValue _v1933 = px_null();
    LXValue _v1934 = px_null();
    LXValue _v1935 = px_null();
    LXValue _v1936 = px_null();
    LXValue px_err_1937_val = px_null();
    int px_err_1937_proped = 0;
    px_srcline(1798);
    _v1931 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
    px_srcline(1799);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("NARGS"), _v1931, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1800);
    _v1932 = _v1930;
    px_srcline(1801);
    while (px_is_truthy(px_lt(_v1932, px_call(px_get_global("len"), (LXValue[]){_v1929}, 1)))) {
        px_srcline(1802);
        if (px_is_truthy(px_ne(px_index(px_index(_v1929, _v1932), px_int(3LL)), px_null()))) {
            px_srcline(1803);
            _v1933 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
            px_srcline(1804);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1928, _v1933, _v1932}, 3));
            px_srcline(1805);
            _v1934 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
            px_srcline(1806);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("LT"), _v1934, _v1933, _v1931}, 5));
            px_srcline(1807);
            _v1935 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1928, px_str("bc"))}, 1);
            px_srcline(1808);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("JMPT"), _v1934, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1811);
            if (px_is_truthy(px_method(px_index(_v1928, px_str("cell")), "has", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1929, _v1932), px_int(1LL))}, 1)}, 1))) {
                px_srcline(1812);
                _v1936 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
                px_srcline(1813);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1929, _v1932), px_int(3LL)), _v1936, _v1928}, 3));
                px_srcline(1814);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("CELLSET"), _v1936, _v1932, px_int(0LL)}, 5));
            }
            else {
                px_srcline(1816);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1929, _v1932), px_int(3LL)), _v1932, _v1928}, 3));
            }
            px_srcline(1817);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1928, _v1935, px_call(px_get_global("len"), (LXValue[]){px_index(_v1928, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1818);
         _v1932 = px_add(_v1932, px_int(1LL));
    }
px_err_1937:
    if (px_err_1937_proped) return px_err_1937_val;
    return px_null();
}

static LXValue fn_bc_emit_func_top(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_top");
    LXValue _v1938 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1939 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1940 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1941 = px_null();
    LXValue _v1942 = px_null();
    LXValue _v1943 = px_null();
    LXValue _v1944 = px_null();
    LXValue px_err_1945_val = px_null();
    int px_err_1945_proped = 0;
    px_srcline(1823);
    _v1941 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1939, px_int(0LL)}, 2);
    px_srcline(1824);
    px_index_set(_v1941, px_str("is_top"), px_bool(true));
    px_srcline(1825);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1938, _v1941}, 2));
    px_srcline(1826);
    if (px_is_truthy(_v1940)) {
        px_srcline(1827);
        _v1942 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1941}, 1);
        px_srcline(1828);
        _v1943 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("main")}, 2);
        px_srcline(1829);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1941, px_str("GETG"), _v1942, _v1943, px_int(0LL)}, 5));
        px_srcline(1830);
        _v1944 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1941}, 1);
        px_srcline(1831);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1941, px_str("CALL"), _v1944, _v1942, px_int(0LL)}, 5));
        px_srcline(1832);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1941, px_str("RET"), _v1944, px_int(0LL), px_int(0LL)}, 5));
    }
    else {
        px_srcline(1834);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1941, px_str("HALT"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    }
    px_srcline(1835);
    px_index_set(_v1941, px_str("nslots"), px_index(_v1941, px_str("next_slot")));
    px_srcline(1836);
    return _v1941;
px_err_1945:
    if (px_err_1945_proped) return px_err_1945_val;
    return px_null();
}

static LXValue fn_bc_emit_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_program");
    LXValue _v1946 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1947 = px_null();
    LXValue _v1948 = px_null();
    LXValue _v1949 = px_null();
    LXValue _v1950 = px_null();
    LXValue _v1951 = px_null();
    LXValue _v1952 = px_null();
    LXValue _v1953 = px_null();
    LXValue _v1954 = px_null();
    LXValue _v1955 = px_null();
    LXValue _v1956 = px_null();
    LXValue _v1957 = px_null();
    LXValue _v1958 = px_null();
    LXValue _v1959 = px_null();
    LXValue _v1960 = px_null();
    LXValue _v1961 = px_null();
    LXValue _v1962 = px_null();
    LXValue _v1963 = px_null();
    LXValue px_err_1964_val = px_null();
    int px_err_1964_proped = 0;
    px_srcline(1840);
    _v1947 = px_index(_v1946, px_int(1LL));
    px_srcline(1841);
    _v1948 = px_call(px_get_global("bc_new_module"), (LXValue[]){px_str("<module>")}, 1);
    px_srcline(1842);
    px_set_global("g_bcm", _v1948);
    px_srcline(1845);
    _v1949 = px_int(0LL);
    px_srcline(1846);
    while (px_is_truthy(px_lt(_v1949, px_call(px_get_global("len"), (LXValue[]){_v1947}, 1)))) {
        px_srcline(1847);
        _v1950 = px_index(_v1947, _v1949);
        px_srcline(1848);
        _v1951 = px_index(_v1950, px_int(0LL));
        px_srcline(1849);
        if (px_is_truthy(px_eq(_v1951, px_str("StructDef")))) {
            px_srcline(1850);
            _v1952 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
            px_srcline(1851);
            px_index_set(_v1952, px_str("name"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1950, px_int(1LL))}, 1));
            px_srcline(1852);
            _v1953 = px_list_n((LXValue[]){}, 0);
            px_srcline(1853);
            _v1954 = px_int(0LL);
            px_srcline(1854);
            while (px_is_truthy(px_lt(_v1954, px_call(px_get_global("len"), (LXValue[]){px_index(_v1950, px_int(2LL))}, 1)))) {
                px_srcline(1855);
                (void)(px_method(_v1953, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1950, px_int(2LL)), _v1954), px_int(1LL))}, 1)}, 1));
                px_srcline(1856);
                 _v1954 = px_add(_v1954, px_int(1LL));
            }
            px_srcline(1857);
            px_index_set(_v1952, px_str("fnames"), _v1953);
            px_srcline(1858);
            (void)(px_method(px_index(_v1948, px_str("structs")), "push", (LXValue[]){_v1952}, 1));
        }
        else if (px_is_truthy(px_eq(_v1951, px_str("EnumDef")))) {
            px_srcline(1860);
            _v1955 = px_list_n((LXValue[]){}, 0);
            px_srcline(1861);
            _v1956 = px_int(0LL);
            px_srcline(1862);
            while (px_is_truthy(px_lt(_v1956, px_call(px_get_global("len"), (LXValue[]){px_index(_v1950, px_int(2LL))}, 1)))) {
                px_srcline(1863);
                (void)(px_method(_v1955, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1950, px_int(2LL)), _v1956), px_int(1LL))}, 1)}, 1));
                px_srcline(1864);
                 _v1956 = px_add(_v1956, px_int(1LL));
            }
            px_srcline(1865);
            px_index_set(px_index(_v1948, px_str("enums")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1950, px_int(1LL))}, 1), _v1955);
        }
        px_srcline(1866);
         _v1949 = px_add(_v1949, px_int(1LL));
    }
    px_srcline(1867);
    (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){_v1947}, 1));
    px_srcline(1868);
    _v1957 = px_call(px_get_global("bc_collect_impl_list"), (LXValue[]){_v1947}, 1);
    px_srcline(1875);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1876);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1877);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1878);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1879);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1880);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1881);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1882);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1883);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(1884);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(1886);
     _v1949 = px_int(0LL);
    px_srcline(1887);
    while (px_is_truthy(px_lt(_v1949, px_call(px_get_global("len"), (LXValue[]){_v1947}, 1)))) {
        px_srcline(1888);
        _v1950 = px_index(_v1947, _v1949);
        px_srcline(1889);
        _v1951 = px_index(_v1950, px_int(0LL));
        px_srcline(1890);
        if (px_is_truthy(px_eq(_v1951, px_str("FuncDef")))) {
            px_srcline(1891);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v1948, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1950, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1951, px_str("ExternDef")))) {
            px_srcline(1898);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1950, px_int(1LL))}, 1), px_index(_v1950, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1951, px_str("VarDecl")))) {
            px_srcline(1900);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v1948, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1950, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1951, px_str("Assign")))) {
            px_srcline(1902);
            _v1958 = px_index(_v1950, px_int(1LL));
            px_srcline(1903);
            if (px_is_truthy(px_eq(px_index(_v1958, px_int(0LL)), px_str("Var")))) {
                px_srcline(1904);
                (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v1948, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1958, px_int(1LL))}, 1)}, 2));
            }
        }
        px_srcline(1905);
         _v1949 = px_add(_v1949, px_int(1LL));
    }
    px_srcline(1908);
     _v1949 = px_int(0LL);
    px_srcline(1909);
    while (px_is_truthy(px_lt(_v1949, px_call(px_get_global("len"), (LXValue[]){_v1957}, 1)))) {
        px_srcline(1910);
        _v1959 = px_index(_v1957, _v1949);
        px_srcline(1911);
        _v1960 = px_call(px_get_global("bc_emit_impl_def"), (LXValue[]){px_index(_v1959, px_int(1LL)), px_index(_v1959, px_int(0LL))}, 2);
        px_srcline(1912);
        (void)(px_method(px_index(_v1948, px_str("funcs")), "push", (LXValue[]){_v1960}, 1));
        px_srcline(1913);
         _v1949 = px_add(_v1949, px_int(1LL));
    }
    px_srcline(1914);
     _v1949 = px_int(0LL);
    px_srcline(1915);
    while (px_is_truthy(px_lt(_v1949, px_call(px_get_global("len"), (LXValue[]){_v1947}, 1)))) {
        px_srcline(1916);
        _v1950 = px_index(_v1947, _v1949);
        px_srcline(1917);
        if (px_is_truthy(px_eq(px_index(_v1950, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(1918);
            _v1960 = px_call(px_get_global("bc_emit_func_def"), (LXValue[]){_v1950}, 1);
            px_srcline(1919);
            (void)(px_method(px_index(_v1948, px_str("funcs")), "push", (LXValue[]){_v1960}, 1));
        }
        px_srcline(1920);
         _v1949 = px_add(_v1949, px_int(1LL));
    }
    px_srcline(1922);
    _v1961 = px_list_n((LXValue[]){}, 0);
    px_srcline(1923);
     _v1949 = px_int(0LL);
    px_srcline(1924);
    while (px_is_truthy(px_lt(_v1949, px_call(px_get_global("len"), (LXValue[]){_v1947}, 1)))) {
        px_srcline(1925);
        _v1950 = px_index(_v1947, _v1949);
        px_srcline(1926);
        _v1951 = px_index(_v1950, px_int(0LL));
        px_srcline(1927);
        if (px_is_truthy(({ LXValue _t1971 = ({ LXValue _t1970 = ({ LXValue _t1969 = ({ LXValue _t1968 = ({ LXValue _t1967 = ({ LXValue _t1966 = ({ LXValue _t1965 = px_ne(_v1951, px_str("FuncDef")); px_is_truthy(_t1965) ? px_ne(_v1951, px_str("StructDef")) : _t1965; }); px_is_truthy(_t1966) ? px_ne(_v1951, px_str("EnumDef")) : _t1966; }); px_is_truthy(_t1967) ? px_ne(_v1951, px_str("TraitDef")) : _t1967; }); px_is_truthy(_t1968) ? px_ne(_v1951, px_str("ImplDef")) : _t1968; }); px_is_truthy(_t1969) ? px_ne(_v1951, px_str("Import")) : _t1969; }); px_is_truthy(_t1970) ? px_ne(_v1951, px_str("ExternDef")) : _t1970; }); px_is_truthy(_t1971) ? px_ne(_v1951, px_str("TypeConst")) : _t1971; }))) {
            px_srcline(1928);
            (void)(px_method(_v1961, "append", (LXValue[]){_v1950}, 1));
        }
        px_srcline(1929);
         _v1949 = px_add(_v1949, px_int(1LL));
    }
    px_srcline(1931);
    _v1962 = px_bool(false);
    px_srcline(1932);
     _v1949 = px_int(0LL);
    px_srcline(1933);
    while (px_is_truthy(px_lt(_v1949, px_call(px_get_global("len"), (LXValue[]){_v1947}, 1)))) {
        px_srcline(1934);
        _v1950 = px_index(_v1947, _v1949);
        px_srcline(1935);
        if (px_is_truthy(({ LXValue _t1972 = px_eq(px_index(_v1950, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1972) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1950, px_int(1LL))}, 1), px_str("main")) : _t1972; }))) {
            px_srcline(1936);
             _v1962 = px_bool(true);
        }
        px_srcline(1937);
         _v1949 = px_add(_v1949, px_int(1LL));
    }
    px_srcline(1938);
    _v1963 = px_call(px_get_global("bc_emit_func_top"), (LXValue[]){_v1961, px_str("<top>"), _v1962}, 3);
    px_srcline(1939);
    (void)(px_method(px_index(_v1948, px_str("funcs")), "push", (LXValue[]){_v1963}, 1));
    px_srcline(1940);
    px_index_set(_v1948, px_str("top"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v1948, px_str("funcs"))}, 1), px_int(1LL)));
    px_srcline(1941);
    return _v1948;
px_err_1964:
    if (px_err_1964_proped) return px_err_1964_val;
    return px_null();
}

static LXValue fn_bc_dump_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_dump_module");
    LXValue _v1973 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1974 = px_null();
    LXValue _v1975 = px_null();
    LXValue _v1976 = px_null();
    LXValue _v1977 = px_null();
    LXValue _v1978 = px_null();
    LXValue _v1979 = px_null();
    LXValue _v1980 = px_null();
    LXValue _v1981 = px_null();
    LXValue _v1982 = px_null();
    LXValue px_err_1983_val = px_null();
    int px_err_1983_proped = 0;
    px_srcline(1944);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# BCModule "), px_index(_v1973, px_str("name")))}, 1));
    px_srcline(1945);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# K "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1973, px_str("k_pool"))}, 1)}, 1))}, 1));
    px_srcline(1946);
    _v1974 = px_int(0LL);
    px_srcline(1947);
    while (px_is_truthy(px_lt(_v1974, px_call(px_get_global("len"), (LXValue[]){px_index(_v1973, px_str("k_pool"))}, 1)))) {
        px_srcline(1948);
        _v1975 = px_index(px_index(_v1973, px_str("k_pool")), _v1974);
        px_srcline(1949);
        if (px_is_truthy(px_eq(px_index(_v1975, px_str("kind")), px_str("int")))) {
            px_srcline(1950);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1974}, 1)), px_str(" int ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1975, px_str("i"))}, 1))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1975, px_str("kind")), px_str("float")))) {
            px_srcline(1952);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1974}, 1)), px_str(" float ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1975, px_str("f"))}, 1))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1975, px_str("kind")), px_str("str")))) {
            px_srcline(1954);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1974}, 1)), px_str(" str \"")), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1975, px_str("s"))}, 1)), px_str("\""))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1975, px_str("kind")), px_str("bool")))) {
            px_srcline(1956);
            _v1976 = px_str("false");
            px_srcline(1957);
            if (px_is_truthy(px_ne(px_index(_v1975, px_str("i")), px_int(0LL)))) {
                px_srcline(1958);
                 _v1976 = px_str("true");
            }
            px_srcline(1959);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1974}, 1)), px_str(" bool ")), _v1976)}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1975, px_str("kind")), px_str("func")))) {
            px_srcline(1961);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1974}, 1)), px_str(" func ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1975, px_str("i"))}, 1))}, 1));
        }
        else {
            px_srcline(1963);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v1974}, 1)), px_str(" null"))}, 1));
        }
        px_srcline(1964);
         _v1974 = px_add(_v1974, px_int(1LL));
    }
    px_srcline(1965);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# G "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1973, px_str("globals"))}, 1)}, 1))}, 1));
    px_srcline(1966);
    _v1977 = px_int(0LL);
    px_srcline(1967);
    while (px_is_truthy(px_lt(_v1977, px_call(px_get_global("len"), (LXValue[]){px_index(_v1973, px_str("globals"))}, 1)))) {
        px_srcline(1968);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("G "), px_call(px_get_global("str"), (LXValue[]){_v1977}, 1)), px_str(" ")), px_index(px_index(_v1973, px_str("globals")), _v1977))}, 1));
        px_srcline(1969);
         _v1977 = px_add(_v1977, px_int(1LL));
    }
    px_srcline(1970);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("# funcs "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1973, px_str("funcs"))}, 1)}, 1)), px_str(" top=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1973, px_str("top"))}, 1))}, 1));
    px_srcline(1971);
    _v1978 = px_int(0LL);
    px_srcline(1972);
    while (px_is_truthy(px_lt(_v1978, px_call(px_get_global("len"), (LXValue[]){px_index(_v1973, px_str("funcs"))}, 1)))) {
        px_srcline(1973);
        _v1979 = px_index(px_index(_v1973, px_str("funcs")), _v1978);
        px_srcline(1974);
        _v1980 = px_str("");
        px_srcline(1975);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1979, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(1976);
             _v1980 = px_add(px_add(px_add(px_add(px_str(" up="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1979, px_str("upnames"))}, 1)}, 1)), px_str("[")), px_call(px_get_global("join"), (LXValue[]){px_str(","), px_index(_v1979, px_str("upnames"))}, 2)), px_str("]"));
        }
        px_srcline(1977);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("== func "), px_index(_v1979, px_str("name"))), px_str(" arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1979, px_str("arity"))}, 1)), px_str(" ndefault=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1979, px_str("ndefault"))}, 1)), px_str(" nslots=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1979, px_str("nslots"))}, 1)), _v1980)}, 1));
        px_srcline(1978);
        _v1981 = px_int(0LL);
        px_srcline(1979);
        while (px_is_truthy(px_lt(_v1981, px_call(px_get_global("len"), (LXValue[]){px_index(_v1979, px_str("bc"))}, 1)))) {
            px_srcline(1980);
            _v1982 = px_index(px_index(_v1979, px_str("bc")), _v1981);
            px_srcline(1981);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v1981}, 1), px_str(":")), px_index(_v1982, px_int(0LL))), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1982, px_int(2LL))}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1982, px_int(3LL))}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1982, px_int(4LL))}, 1))}, 1));
            px_srcline(1982);
             _v1981 = px_add(_v1981, px_int(1LL));
        }
        px_srcline(1983);
         _v1978 = px_add(_v1978, px_int(1LL));
    }
px_err_1983:
    if (px_err_1983_proped) return px_err_1983_val;
    return px_null();
}

static LXValue fn_bc_emit_c_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_c_program");
    LXValue _v1984 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1985 = px_null();
    LXValue _v1986 = px_null();
    LXValue _v1987 = px_null();
    LXValue _v1988 = px_null();
    LXValue _v1989 = px_null();
    LXValue _v1990 = px_null();
    LXValue _v1991 = px_null();
    LXValue _v1992 = px_null();
    LXValue _v1993 = px_null();
    LXValue _v1994 = px_null();
    LXValue _v1995 = px_null();
    LXValue _v1996 = px_null();
    LXValue _v1997 = px_null();
    LXValue _v1998 = px_null();
    LXValue _v1999 = px_null();
    LXValue _v2000 = px_null();
    LXValue _v2001 = px_null();
    LXValue px_err_2002_val = px_null();
    int px_err_2002_proped = 0;
    px_srcline(1990);
    _v1985 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v1984}, 1);
    px_srcline(1995);
    _v1986 = px_list_n((LXValue[]){px_str("/* 由 bc_emit.px (M89-S3) 自动生成 — 字节码模块（VM 执行） */\n")}, 1);
    px_srcline(1996);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("#include \"runtime.h\"\n#include \"vm.h\"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n")}, 1));
    px_srcline(1997);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const PxBCModule s_mod;\n\n")}, 1));
    px_srcline(1999);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const PxK s_K[] = {\n")}, 1));
    px_srcline(2000);
    _v1987 = px_int(0LL);
    px_srcline(2001);
    while (px_is_truthy(px_lt(_v1987, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("k_pool"))}, 1)))) {
        px_srcline(2002);
        _v1988 = px_index(px_index(_v1985, px_str("k_pool")), _v1987);
        px_srcline(2003);
        if (px_is_truthy(px_eq(px_index(_v1988, px_str("kind")), px_str("int")))) {
            px_srcline(2004);
            (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_INT, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1988, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1988, px_str("kind")), px_str("float")))) {
            px_srcline(2006);
            (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FLT, 0, "), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v1988, px_str("f"))}, 1)), px_str(", NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1988, px_str("kind")), px_str("str")))) {
            px_srcline(2009);
            if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){px_index(_v1988, px_str("s"))}, 1))) {
                px_srcline(2010);
                (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    PXK_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1988, px_str("s"))}, 1)), px_str("\"),\n"))}, 1));
            }
            else {
                px_srcline(2012);
                (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_STR, 0, 0, \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1988, px_str("s"))}, 1)), px_str("\"},\n"))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v1988, px_str("kind")), px_str("bool")))) {
            px_srcline(2014);
            _v1989 = px_str("0");
            px_srcline(2015);
            if (px_is_truthy(px_ne(px_index(_v1988, px_str("i")), px_int(0LL)))) {
                px_srcline(2016);
                 _v1989 = px_str("1");
            }
            px_srcline(2017);
            (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_BOOL, "), _v1989), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v1988, px_str("kind")), px_str("func")))) {
            px_srcline(2019);
            (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FUNC, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1988, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else {
            px_srcline(2021);
            (void)(px_method(_v1986, "push", (LXValue[]){px_str("    {PXK_NULL, 0, 0, NULL},\n")}, 1));
        }
        px_srcline(2022);
         _v1987 = px_add(_v1987, px_int(1LL));
    }
    px_srcline(2023);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2025);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const char* s_N[] = {\n")}, 1));
    px_srcline(2026);
    _v1990 = px_int(0LL);
    px_srcline(2027);
    while (px_is_truthy(px_lt(_v1990, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("n_pool"))}, 1)))) {
        px_srcline(2028);
        (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1985, px_str("n_pool")), _v1990)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2029);
         _v1990 = px_add(_v1990, px_int(1LL));
    }
    px_srcline(2030);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2032);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const char* s_G[] = {\n")}, 1));
    px_srcline(2033);
    _v1991 = px_int(0LL);
    px_srcline(2034);
    while (px_is_truthy(px_lt(_v1991, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("globals"))}, 1)))) {
        px_srcline(2035);
        (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1985, px_str("globals")), _v1991)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2036);
         _v1991 = px_add(_v1991, px_int(1LL));
    }
    px_srcline(2037);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2039);
    _v1992 = px_int(0LL);
    px_srcline(2040);
    while (px_is_truthy(px_lt(_v1992, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("structs"))}, 1)))) {
        px_srcline(2041);
        _v1993 = px_index(px_index(_v1985, px_str("structs")), _v1992);
        px_srcline(2042);
        (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("static const char* s_st_"), px_call(px_get_global("str"), (LXValue[]){_v1992}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2043);
        _v1994 = px_int(0LL);
        px_srcline(2044);
        while (px_is_truthy(px_lt(_v1994, px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("fnames"))}, 1)))) {
            px_srcline(2045);
            (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1993, px_str("fnames")), _v1994)}, 1)), px_str("\",\n"))}, 1));
            px_srcline(2046);
             _v1994 = px_add(_v1994, px_int(1LL));
        }
        px_srcline(2047);
        (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2048);
         _v1992 = px_add(_v1992, px_int(1LL));
    }
    px_srcline(2049);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const PxStructDef s_structs[] = {\n")}, 1));
    px_srcline(2050);
     _v1992 = px_int(0LL);
    px_srcline(2051);
    while (px_is_truthy(px_lt(_v1992, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("structs"))}, 1)))) {
        px_srcline(2052);
        _v1993 = px_index(px_index(_v1985, px_str("structs")), _v1992);
        px_srcline(2053);
        (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_str("    {\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1993, px_str("name"))}, 1)), px_str("\", s_st_")), px_call(px_get_global("str"), (LXValue[]){_v1992}, 1)), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("fnames"))}, 1)}, 1)), px_str("},\n"))}, 1));
        px_srcline(2054);
         _v1992 = px_add(_v1992, px_int(1LL));
    }
    px_srcline(2055);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2057);
    _v1995 = px_int(0LL);
    px_srcline(2058);
    while (px_is_truthy(px_lt(_v1995, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("funcs"))}, 1)))) {
        px_srcline(2059);
        _v1996 = px_index(px_index(_v1985, px_str("funcs")), _v1995);
        px_srcline(2060);
        (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("static const PxInst s_bc_"), px_call(px_get_global("str"), (LXValue[]){_v1995}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2061);
        _v1997 = px_int(0LL);
        px_srcline(2062);
        while (px_is_truthy(px_lt(_v1997, px_call(px_get_global("len"), (LXValue[]){px_index(_v1996, px_str("bc"))}, 1)))) {
            px_srcline(2063);
            _v1998 = px_index(px_index(_v1996, px_str("bc")), _v1997);
            px_srcline(2069);
            (void)(px_method(_v1986, "push", (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), px_list_n((LXValue[]){px_str("    {PXOP_"), px_index(_v1998, px_int(0LL)), px_str(", "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1998, px_int(1LL))}, 1), px_str(", "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1998, px_int(2LL))}, 1), px_str(", "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1998, px_int(3LL))}, 1), px_str(", "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1998, px_int(4LL))}, 1), px_str("},\n")}, 11)}, 2)}, 1));
            px_srcline(2070);
             _v1997 = px_add(_v1997, px_int(1LL));
        }
        px_srcline(2071);
        (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2072);
         _v1995 = px_add(_v1995, px_int(1LL));
    }
    px_srcline(2073);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("\n")}, 1));
    px_srcline(2075);
     _v1995 = px_int(0LL);
    px_srcline(2076);
    while (px_is_truthy(px_lt(_v1995, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("funcs"))}, 1)))) {
        px_srcline(2077);
        _v1996 = px_index(px_index(_v1985, px_str("funcs")), _v1995);
        px_srcline(2078);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1996, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2079);
            _v1999 = px_list_n((LXValue[]){}, 0);
            px_srcline(2080);
            _v2000 = px_int(0LL);
            px_srcline(2081);
            while (px_is_truthy(px_lt(_v2000, px_call(px_get_global("len"), (LXValue[]){px_index(_v1996, px_str("upnames"))}, 1)))) {
                px_srcline(2082);
                (void)(px_method(_v1999, "push", (LXValue[]){px_add(px_add(px_str("\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v1996, px_str("upnames")), _v2000)}, 1)), px_str("\""))}, 1));
                px_srcline(2083);
                 _v2000 = px_add(_v2000, px_int(1LL));
            }
            px_srcline(2084);
            (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_add(px_add(px_str("static const char* s_up_"), px_call(px_get_global("str"), (LXValue[]){_v1995}, 1)), px_str("[] = {")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v1999}, 2)), px_str("};\n"))}, 1));
        }
        px_srcline(2085);
         _v1995 = px_add(_v1995, px_int(1LL));
    }
    px_srcline(2087);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const PxVMFunc s_funcs[] = {\n")}, 1));
    px_srcline(2088);
     _v1995 = px_int(0LL);
    px_srcline(2089);
    while (px_is_truthy(px_lt(_v1995, px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("funcs"))}, 1)))) {
        px_srcline(2090);
        _v1996 = px_index(px_index(_v1985, px_str("funcs")), _v1995);
        px_srcline(2091);
        _v2001 = px_str(".upvals=NULL");
        px_srcline(2092);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1996, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2093);
             _v2001 = px_add(px_add(px_add(px_str(".upvals=s_up_"), px_call(px_get_global("str"), (LXValue[]){_v1995}, 1)), px_str(", .nup=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1996, px_str("upnames"))}, 1)}, 1));
        }
        px_srcline(2094);
        (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    {.name=\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v1996, px_str("name"))}, 1)), px_str("\", .arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1996, px_str("arity"))}, 1)), px_str(", .ndefault=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1996, px_str("ndefault"))}, 1)), px_str(", .nslots=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1996, px_str("nslots"))}, 1)), px_str(", .bc=s_bc_")), px_call(px_get_global("str"), (LXValue[]){_v1995}, 1)), px_str(", .nbc=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1996, px_str("bc"))}, 1)}, 1)), px_str(", .mod=&s_mod, ")), _v2001), px_str("},\n"))}, 1));
        px_srcline(2095);
         _v1995 = px_add(_v1995, px_int(1LL));
    }
    px_srcline(2096);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2098);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("static const PxBCModule s_mod = {\n")}, 1));
    px_srcline(2099);
    (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_add(px_add(px_str("    .name=\"<module>\", .K=s_K, .nK="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("k_pool"))}, 1)}, 1)), px_str(", .N=s_N, .nN=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("n_pool"))}, 1)}, 1)), px_str(",\n"))}, 1));
    px_srcline(2100);
    (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_add(px_str("    .G=s_G, .nG="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("globals"))}, 1)}, 1)), px_str(", .funcs=s_funcs, .nfuncs=")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("funcs"))}, 1)}, 1)), px_str(", .top_idx=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v1985, px_str("top"))}, 1)), px_str(",\n"))}, 1));
    px_srcline(2101);
    (void)(px_method(_v1986, "push", (LXValue[]){px_add(px_add(px_str("    .structs=s_structs, .nstructs="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v1985, px_str("structs"))}, 1)}, 1)), px_str(",\n"))}, 1));
    px_srcline(2102);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2104);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("int main(int argc, char** argv) {\n")}, 1));
    px_srcline(2105);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    px_args_init(argc, argv);\n")}, 1));
    px_srcline(2106);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    px_register_builtins();\n")}, 1));
    px_srcline(2107);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    px_gc_set_precise(1);   // M92-S2d：VM 轨产物默认 precise 精确根面（退役保守栈扫描；C 轨逃生舱产物不插此调用 → conservative）\n")}, 1));
    px_srcline(2108);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    LXValue _r = px_vm_run_module(px_vm_state(), &s_mod);\n")}, 1));
    px_srcline(2109);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    int _code = 0;\n")}, 1));
    px_srcline(2110);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    if (px_is_result(_r)) {\n")}, 1));
    px_srcline(2111);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        if (!px_result_ok(_r)) {\n")}, 1));
    px_srcline(2112);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("            fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n")}, 1));
    px_srcline(2113);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("            _code = 1;\n")}, 1));
    px_srcline(2114);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        } else {\n")}, 1));
    px_srcline(2115);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("            LXValue _uv = px_result_unwrap(_r);\n")}, 1));
    px_srcline(2116);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("            if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n")}, 1));
    px_srcline(2117);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2118);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    } else if (_r.type == PX_INT) {\n")}, 1));
    px_srcline(2119);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        _code = (int)_r.as.i;\n")}, 1));
    px_srcline(2120);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2121);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    if (getenv(\"PX_BC_DUMP\")) {\n")}, 1));
    px_srcline(2122);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        int _i;\n")}, 1));
    px_srcline(2123);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        for (_i = 0; _i < (int)s_mod.nG; _i++) {\n")}, 1));
    px_srcline(2124);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("            LXValue _v = px_get_global(s_mod.G[_i]);\n")}, 1));
    px_srcline(2125);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("            if (_v.type != PX_FUNC && _v.type != PX_NATIVE)\n")}, 1));
    px_srcline(2126);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("                printf(\"%s=%s\\n\", s_mod.G[_i], px_to_string(_v));\n")}, 1));
    px_srcline(2127);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2128);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2129);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("    return px_exit_code_final(_code);   // M120（qg-issue 76 E1）：协程隔离错误 → 退出码非 0\n")}, 1));
    px_srcline(2130);
    (void)(px_method(_v1986, "push", (LXValue[]){px_str("}\n")}, 1));
    px_srcline(2131);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1986}, 2);
px_err_2002:
    if (px_err_2002_proped) return px_err_2002_val;
    return px_null();
}

static LXValue fn_bc_basename(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_basename");
    LXValue _v2003 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2004 = px_null();
    LXValue _v2005 = px_null();
    LXValue px_err_2006_val = px_null();
    int px_err_2006_proped = 0;
    px_srcline(64);
    _v2004 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2003}, 1), px_int(1LL));
    px_srcline(65);
    while (px_is_truthy(px_ge(_v2004, px_int(0LL)))) {
        px_srcline(66);
        if (px_is_truthy(px_eq(px_index(_v2003, _v2004), px_str("/")))) {
            px_srcline(67);
            _v2005 = px_slice(_v2003, px_add(_v2004, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v2003}, 1), px_null());
            px_srcline(68);
            if (px_is_truthy(({ LXValue _t2007 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2005}, 1), px_int(3LL)); px_is_truthy(_t2007) ? px_eq(px_slice(_v2005, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2005}, 1), px_int(3LL)), px_call(px_get_global("len"), (LXValue[]){_v2005}, 1), px_null()), px_str(".px")) : _t2007; }))) {
                px_srcline(69);
                return px_slice(_v2005, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2005}, 1), px_int(3LL)), px_null());
            }
            px_srcline(70);
            return _v2005;
        }
        px_srcline(71);
         _v2004 = px_sub(_v2004, px_int(1LL));
    }
    px_srcline(72);
    if (px_is_truthy(({ LXValue _t2008 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2003}, 1), px_int(3LL)); px_is_truthy(_t2008) ? px_eq(px_slice(_v2003, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2003}, 1), px_int(3LL)), px_call(px_get_global("len"), (LXValue[]){_v2003}, 1), px_null()), px_str(".px")) : _t2008; }))) {
        px_srcline(73);
        return px_slice(_v2003, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2003}, 1), px_int(3LL)), px_null());
    }
    px_srcline(74);
    return _v2003;
px_err_2006:
    if (px_err_2006_proped) return px_err_2006_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v2009 = px_null();
    LXValue _v2010 = px_null();
    LXValue _v2011 = px_null();
    LXValue _v2012 = px_null();
    LXValue _v2013 = px_null();
    LXValue _v2014 = px_null();
    LXValue _v2015 = px_null();
    LXValue _v2016 = px_null();
    LXValue _v2017 = px_null();
    LXValue _v2018 = px_null();
    LXValue _v2019 = px_null();
    LXValue px_err_2020_val = px_null();
    int px_err_2020_proped = 0;
    px_srcline(77);
    _v2009 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    px_srcline(79);
    if (px_is_truthy(({ LXValue _t2022 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v2009}, 1), px_int(2LL)); px_is_truthy(_t2022) ? ({ LXValue _t2021 = px_eq(px_index(_v2009, px_int(1LL)), px_str("--version")); px_is_truthy(_t2021) ? _t2021 : px_eq(px_index(_v2009, px_int(1LL)), px_str("-v")); }) : _t2022; }))) {
        px_srcline(80);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        px_srcline(81);
        return px_int(0LL);
    }
    px_srcline(84);
    _v2010 = px_bool(false);
    px_srcline(85);
    _v2011 = px_bool(false);
    px_srcline(86);
    _v2012 = px_int(1LL);
    px_srcline(87);
    while (px_is_truthy(px_lt(_v2012, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2009}, 1), px_int(1LL))))) {
        px_srcline(88);
        if (px_is_truthy(({ LXValue _t2023 = px_eq(px_index(_v2009, _v2012), px_str("bc")); px_is_truthy(_t2023) ? _t2023 : px_eq(px_index(_v2009, _v2012), px_str("--emit-c")); }))) {
            px_srcline(89);
             _v2010 = px_bool(true);
        }
        px_srcline(90);
        if (px_is_truthy(px_eq(px_index(_v2009, _v2012), px_str("--emit-c")))) {
            px_srcline(91);
             _v2011 = px_bool(true);
        }
        px_srcline(92);
         _v2012 = px_add(_v2012, px_int(1LL));
    }
    px_srcline(94);
    _v2013 = px_index(_v2009, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2009}, 1), px_int(1LL)));
    px_srcline(95);
    _v2014 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v2013}, 1);
    px_srcline(96);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v2013}, 1)}, 1));
    px_srcline(97);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(98);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(99);
    _v2015 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(100);
    _v2016 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v2015, _v2014}, 2);
    px_srcline(101);
    if (px_is_truthy(_v2010)) {
        px_srcline(102);
        if (px_is_truthy(_v2011)) {
            px_srcline(103);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("bc_emit_c_program"), (LXValue[]){_v2016}, 1)}, 1));
            px_srcline(104);
            return px_int(0LL);
        }
        px_srcline(105);
        _v2017 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2016}, 1);
        px_srcline(106);
        px_index_set(_v2017, px_str("name"), px_call(px_get_global("bc_basename"), (LXValue[]){_v2013}, 1));
        px_srcline(107);
        (void)(px_call(px_get_global("bc_dump_module"), (LXValue[]){_v2017}, 1));
        px_srcline(108);
        return px_int(0LL);
    }
    px_srcline(109);
    _v2018 = px_call(px_get_global("cg_generate"), (LXValue[]){_v2016}, 1);
    px_srcline(111);
    _v2019 = px_call(px_get_global("len"), (LXValue[]){_v2018}, 1);
    px_srcline(112);
    if (px_is_truthy(({ LXValue _t2024 = px_gt(_v2019, px_int(0LL)); px_is_truthy(_t2024) ? px_eq(px_index(_v2018, px_sub(_v2019, px_int(1LL))), px_str("\n")) : _t2024; }))) {
        px_srcline(113);
         _v2018 = px_slice(_v2018, px_int(0LL), px_sub(_v2019, px_int(1LL)), px_null());
    }
    px_srcline(114);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v2018}, 1));
px_err_2020:
    if (px_err_2020_proped) return px_err_2020_val;
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
    px_set_global("lx_nul_char", px_func("lx_nul_char", fn_lx_nul_char, NULL));
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
    px_set_global("cg_gen_closure", px_func("cg_gen_closure", fn_cg_gen_closure, NULL));
    px_set_global("cg_gen_expr", px_func("cg_gen_expr", fn_cg_gen_expr, NULL));
    px_set_global("cg_binop_cname", px_func("cg_binop_cname", fn_cg_binop_cname, NULL));
    px_set_global("cg_gen_pattern_cond", px_func("cg_gen_pattern_cond", fn_cg_gen_pattern_cond, NULL));
    px_set_global("cg_gen_lambda", px_func("cg_gen_lambda", fn_cg_gen_lambda, NULL));
    px_set_global("cgm_perr", px_func("cgm_perr", fn_cgm_perr, NULL));
    px_set_global("cgm_pwarn", px_func("cgm_pwarn", fn_cgm_pwarn, NULL));
    px_set_global("cg_dirname", px_func("cg_dirname", fn_cg_dirname, NULL));
    px_set_global("cg_norm_path", px_func("cg_norm_path", fn_cg_norm_path, NULL));
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
    px_set_global("cg_nul_char", px_func("cg_nul_char", fn_cg_nul_char, NULL));
    px_set_global("cg_has_nul", px_func("cg_has_nul", fn_cg_has_nul, NULL));
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
    px_set_global("bc_cell_has", px_func("bc_cell_has", fn_bc_cell_has, NULL));
    px_set_global("bc_cell_mark", px_func("bc_cell_mark", fn_bc_cell_mark, NULL));
    px_set_global("bc_load_var", px_func("bc_load_var", fn_bc_load_var, NULL));
    px_set_global("bc_store_var", px_func("bc_store_var", fn_bc_store_var, NULL));
    px_set_global("bc_closure_free", px_func("bc_closure_free", fn_bc_closure_free, NULL));
    px_set_global("bc_lambda_hoist", px_func("bc_lambda_hoist", fn_bc_lambda_hoist, NULL));
    px_set_global("bc_box_frame", px_func("bc_box_frame", fn_bc_box_frame, NULL));
    px_set_global("bc_emit_inst", px_func("bc_emit_inst", fn_bc_emit_inst, NULL));
    px_set_global("bc_patch_off", px_func("bc_patch_off", fn_bc_patch_off, NULL));
    px_set_global("bc_collect_hoist", px_func("bc_collect_hoist", fn_bc_collect_hoist, NULL));
    px_set_global("bc_emit_null", px_func("bc_emit_null", fn_bc_emit_null, NULL));
    px_set_global("bc_emit_int", px_func("bc_emit_int", fn_bc_emit_int, NULL));
    px_set_global("bc_neg_float", px_func("bc_neg_float", fn_bc_neg_float, NULL));
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
    px_set_global("bc_emit_closure", px_func("bc_emit_closure", fn_bc_emit_closure, NULL));
    px_set_global("bc_emit_comp", px_func("bc_emit_comp", fn_bc_emit_comp, NULL));
    px_set_global("bc_emit_caps_snapshot", px_func("bc_emit_caps_snapshot", fn_bc_emit_caps_snapshot, NULL));
    px_set_global("bc_genexp_caps", px_func("bc_genexp_caps", fn_bc_genexp_caps, NULL));
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
