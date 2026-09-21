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
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(({ LXValue _s1 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_line")}, 1)), px_str(":")); LXValue _s2 = px_call(px_get_global("str"), (LXValue[]){px_get_global("g_col")}, 1); px_add(_s1, _s2); }), px_str(": 词法错误 ")), _v23), px_str(": ")), _v24)}, 1));
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
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(({ LXValue _s3 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){_v28}, 1)), px_str(":")); LXValue _s4 = px_call(px_get_global("str"), (LXValue[]){_v29}, 1); px_add(_s3, _s4); }), px_str(": 词法错误 ")), _v26), px_str(": ")), _v27)}, 1));
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
    px_srcline(21);
    _v245 = px_str("");
    px_srcline(22);
    _v246 = px_int(0LL);
    px_srcline(23);
    while (px_is_truthy(px_lt(_v246, _v244))) {
        px_srcline(24);
         _v245 = px_add(_v245, px_str(" "));
        px_srcline(25);
         _v246 = px_add(_v246, px_int(1LL));
    }
    px_srcline(26);
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
    px_srcline(28);
    _v250 = px_index(_v248, px_int(0LL));
    px_srcline(29);
    _v251 = px_index(px_get_global("LAYOUT"), _v250);
    px_srcline(30);
    _v252 = px_index(_v251, px_int(0LL));
    px_srcline(31);
    _v253 = px_index(_v251, px_int(1LL));
    px_srcline(32);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v253}, 1), px_int(0LL)))) {
        px_srcline(33);
        return _v252;
    }
    px_srcline(34);
    _v254 = px_eq(px_index(px_index(_v253, px_int(0LL)), px_int(0LL)), px_null());
    px_srcline(35);
    _v255 = px_list_n((LXValue[]){}, 0);
    px_srcline(36);
    if (px_is_truthy(_v254)) {
        px_srcline(37);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(_v252, px_str("("))}, 1));
    }
    else {
        px_srcline(39);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(_v252, px_str(" {"))}, 1));
    }
    px_srcline(40);
    _v256 = px_int(0LL);
    px_srcline(41);
    while (px_is_truthy(px_lt(_v256, px_call(px_get_global("len"), (LXValue[]){_v253}, 1)))) {
        px_srcline(42);
        _v257 = px_index(_v253, _v256);
        px_srcline(43);
        _v258 = px_index(_v248, px_add(_v256, px_int(1LL)));
        px_srcline(44);
        _v259 = px_call(px_get_global("dump_field"), (LXValue[]){_v258, px_index(_v257, px_int(1LL)), px_add(_v249, px_int(4LL))}, 3);
        px_srcline(45);
        if (px_is_truthy(_v254)) {
            px_srcline(46);
            (void)(px_method(_v255, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v249, px_int(4LL))}, 1), _v259), px_str(","))}, 1));
        }
        else {
            px_srcline(48);
            (void)(px_method(_v255, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v249, px_int(4LL))}, 1), px_index(_v257, px_int(0LL))), px_str(": ")), _v259), px_str(","))}, 1));
        }
        px_srcline(49);
         _v256 = px_add(_v256, px_int(1LL));
    }
    px_srcline(50);
    if (px_is_truthy(_v254)) {
        px_srcline(51);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v249}, 1), px_str(")"))}, 1));
    }
    else {
        px_srcline(53);
        (void)(px_method(_v255, "append", (LXValue[]){px_add(px_call(px_get_global("pad"), (LXValue[]){_v249}, 1), px_str("}"))}, 1));
    }
    px_srcline(54);
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
    px_srcline(56);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v261}, 1), px_int(0LL)))) {
        px_srcline(57);
        return px_str("[]");
    }
    px_srcline(58);
    _v263 = px_list_n((LXValue[]){}, 0);
    px_srcline(59);
    _v264 = px_int(0LL);
    px_srcline(60);
    while (px_is_truthy(px_lt(_v264, px_call(px_get_global("len"), (LXValue[]){_v261}, 1)))) {
        px_srcline(61);
        (void)(px_method(_v263, "append", (LXValue[]){px_add(({ LXValue _s5 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v262, px_int(4LL))}, 1); LXValue _s6 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v261, _v264), px_add(_v262, px_int(4LL))}, 2); px_add(_s5, _s6); }), px_str(","))}, 1));
        px_srcline(62);
         _v264 = px_add(_v264, px_int(1LL));
    }
    px_srcline(63);
    return px_add(({ LXValue _s7 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v263}, 2)), px_str("\n")); LXValue _s8 = px_call(px_get_global("pad"), (LXValue[]){_v262}, 1); px_add(_s7, _s8); }), px_str("]"));
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
    px_srcline(65);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v266}, 1), px_int(0LL)))) {
        px_srcline(66);
        return px_str("[]");
    }
    px_srcline(67);
    _v268 = px_list_n((LXValue[]){}, 0);
    px_srcline(68);
    _v269 = px_int(0LL);
    px_srcline(69);
    while (px_is_truthy(px_lt(_v269, px_call(px_get_global("len"), (LXValue[]){_v266}, 1)))) {
        px_srcline(70);
        (void)(px_method(_v268, "append", (LXValue[]){px_add(px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v267, px_int(4LL))}, 1), px_index(_v266, _v269)), px_str(","))}, 1));
        px_srcline(71);
         _v269 = px_add(_v269, px_int(1LL));
    }
    px_srcline(72);
    return px_add(({ LXValue _s9 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v268}, 2)), px_str("\n")); LXValue _s10 = px_call(px_get_global("pad"), (LXValue[]){_v267}, 1); px_add(_s9, _s10); }), px_str("]"));
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
    px_srcline(74);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v271}, 1), px_int(0LL)))) {
        px_srcline(75);
        return px_str("[]");
    }
    px_srcline(76);
    _v273 = px_list_n((LXValue[]){}, 0);
    px_srcline(77);
    _v274 = px_int(0LL);
    px_srcline(78);
    while (px_is_truthy(px_lt(_v274, px_call(px_get_global("len"), (LXValue[]){_v271}, 1)))) {
        px_srcline(79);
        (void)(px_method(_v273, "append", (LXValue[]){px_add(({ LXValue _s11 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v272, px_int(4LL))}, 1); LXValue _s12 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v271, _v274), px_add(_v272, px_int(4LL))}, 2); px_add(_s11, _s12); }), px_str(","))}, 1));
        px_srcline(80);
         _v274 = px_add(_v274, px_int(1LL));
    }
    px_srcline(81);
    return px_add(({ LXValue _s13 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v273}, 2)), px_str("\n")); LXValue _s14 = px_call(px_get_global("pad"), (LXValue[]){_v272}, 1); px_add(_s13, _s14); }), px_str("]"));
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
    px_srcline(83);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v276}, 1), px_int(0LL)))) {
        px_srcline(84);
        return px_str("[]");
    }
    px_srcline(85);
    _v278 = px_list_n((LXValue[]){}, 0);
    px_srcline(86);
    _v279 = px_int(0LL);
    px_srcline(87);
    while (px_is_truthy(px_lt(_v279, px_call(px_get_global("len"), (LXValue[]){_v276}, 1)))) {
        px_srcline(88);
        (void)(px_method(_v278, "append", (LXValue[]){px_add(({ LXValue _s15 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v277, px_int(4LL))}, 1); LXValue _s16 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v276, _v279), px_add(_v277, px_int(4LL))}, 2); px_add(_s15, _s16); }), px_str(","))}, 1));
        px_srcline(89);
         _v279 = px_add(_v279, px_int(1LL));
    }
    px_srcline(90);
    return px_add(({ LXValue _s17 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v278}, 2)), px_str("\n")); LXValue _s18 = px_call(px_get_global("pad"), (LXValue[]){_v277}, 1); px_add(_s17, _s18); }), px_str("]"));
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
    px_srcline(92);
    if (px_is_truthy(px_eq(_v281, px_null()))) {
        px_srcline(93);
        return px_str("None");
    }
    px_srcline(94);
    return px_add(({ LXValue _s21 = px_add(({ LXValue _s19 = px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v282, px_int(4LL))}, 1)); LXValue _s20 = px_call(px_get_global("dump_node"), (LXValue[]){_v281, px_add(_v282, px_int(4LL))}, 2); px_add(_s19, _s20); }), px_str(",\n")); LXValue _s22 = px_call(px_get_global("pad"), (LXValue[]){_v282}, 1); px_add(_s21, _s22); }), px_str(")"));
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
    px_srcline(96);
    if (px_is_truthy(px_eq(_v284, px_null()))) {
        px_srcline(97);
        return px_str("None");
    }
    px_srcline(98);
    return px_add(({ LXValue _s23 = px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v285, px_int(4LL))}, 1)), _v284), px_str(",\n")); LXValue _s24 = px_call(px_get_global("pad"), (LXValue[]){_v285}, 1); px_add(_s23, _s24); }), px_str(")"));
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
    px_srcline(100);
    if (px_is_truthy(px_eq(_v287, px_null()))) {
        px_srcline(101);
        return px_str("None");
    }
    px_srcline(102);
    return px_add(({ LXValue _s27 = px_add(({ LXValue _s25 = px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v288, px_int(4LL))}, 1)); LXValue _s26 = px_call(px_get_global("dump_list"), (LXValue[]){_v287, px_add(_v288, px_int(4LL))}, 2); px_add(_s25, _s26); }), px_str(",\n")); LXValue _s28 = px_call(px_get_global("pad"), (LXValue[]){_v288}, 1); px_add(_s27, _s28); }), px_str(")"));
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
    px_srcline(104);
    return px_add(({ LXValue _s35 = px_add(({ LXValue _s33 = px_add(({ LXValue _s31 = px_add(({ LXValue _s29 = px_add(px_add(px_str("Pos {\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v291, px_int(4LL))}, 1)), px_str("line: ")); LXValue _s30 = px_call(px_get_global("str"), (LXValue[]){px_index(_v290, px_int(0LL))}, 1); px_add(_s29, _s30); }), px_str(",\n")); LXValue _s32 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v291, px_int(4LL))}, 1); px_add(_s31, _s32); }), px_str("col: ")); LXValue _s34 = px_call(px_get_global("str"), (LXValue[]){px_index(_v290, px_int(1LL))}, 1); px_add(_s33, _s34); }), px_str(",\n")); LXValue _s36 = px_call(px_get_global("pad"), (LXValue[]){_v291}, 1); px_add(_s35, _s36); }), px_str("}"));
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
    px_srcline(107);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v293}, 1), px_int(0LL)))) {
        px_srcline(108);
        return px_str("[]");
    }
    px_srcline(109);
    _v295 = px_list_n((LXValue[]){}, 0);
    px_srcline(110);
    _v296 = px_int(0LL);
    px_srcline(111);
    while (px_is_truthy(px_lt(_v296, px_call(px_get_global("len"), (LXValue[]){_v293}, 1)))) {
        px_srcline(112);
        _v297 = px_index(_v293, _v296);
        px_srcline(113);
        (void)(px_method(_v295, "append", (LXValue[]){px_add(({ LXValue _s45 = px_add(({ LXValue _s43 = ({ LXValue _s41 = px_add(({ LXValue _s39 = ({ LXValue _s37 = px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(4LL))}, 1), px_str("(\n")); LXValue _s38 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(8LL))}, 1); px_add(_s37, _s38); }); LXValue _s40 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v297, px_int(0LL)), px_add(_v294, px_int(8LL))}, 2); px_add(_s39, _s40); }), px_str(",\n")); LXValue _s42 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(8LL))}, 1); px_add(_s41, _s42); }); LXValue _s44 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v297, px_int(1LL)), px_add(_v294, px_int(8LL))}, 2); px_add(_s43, _s44); }), px_str(",\n")); LXValue _s46 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v294, px_int(4LL))}, 1); px_add(_s45, _s46); }), px_str("),"))}, 1));
        px_srcline(114);
         _v296 = px_add(_v296, px_int(1LL));
    }
    px_srcline(115);
    return px_add(({ LXValue _s47 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v295}, 2)), px_str("\n")); LXValue _s48 = px_call(px_get_global("pad"), (LXValue[]){_v294}, 1); px_add(_s47, _s48); }), px_str("]"));
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
    px_srcline(118);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v299}, 1), px_int(0LL)))) {
        px_srcline(119);
        return px_str("[]");
    }
    px_srcline(120);
    _v301 = px_list_n((LXValue[]){}, 0);
    px_srcline(121);
    _v302 = px_int(0LL);
    px_srcline(122);
    while (px_is_truthy(px_lt(_v302, px_call(px_get_global("len"), (LXValue[]){_v299}, 1)))) {
        px_srcline(123);
        _v303 = px_index(_v299, _v302);
        px_srcline(124);
        (void)(px_method(_v301, "append", (LXValue[]){px_add(({ LXValue _s57 = px_add(({ LXValue _s55 = ({ LXValue _s53 = px_add(({ LXValue _s51 = ({ LXValue _s49 = px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(4LL))}, 1), px_str("(\n")); LXValue _s50 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(8LL))}, 1); px_add(_s49, _s50); }); LXValue _s52 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v303, px_int(0LL)), px_add(_v300, px_int(8LL))}, 2); px_add(_s51, _s52); }), px_str(",\n")); LXValue _s54 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(8LL))}, 1); px_add(_s53, _s54); }); LXValue _s56 = px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v303, px_int(1LL)), px_add(_v300, px_int(8LL))}, 2); px_add(_s55, _s56); }), px_str(",\n")); LXValue _s58 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v300, px_int(4LL))}, 1); px_add(_s57, _s58); }), px_str("),"))}, 1));
        px_srcline(125);
         _v302 = px_add(_v302, px_int(1LL));
    }
    px_srcline(126);
    return px_add(({ LXValue _s59 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v301}, 2)), px_str("\n")); LXValue _s60 = px_call(px_get_global("pad"), (LXValue[]){_v300}, 1); px_add(_s59, _s60); }), px_str("]"));
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
    px_srcline(129);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v305}, 1), px_int(0LL)))) {
        px_srcline(130);
        return px_str("[]");
    }
    px_srcline(131);
    _v307 = px_list_n((LXValue[]){}, 0);
    px_srcline(132);
    _v308 = px_int(0LL);
    px_srcline(133);
    while (px_is_truthy(px_lt(_v308, px_call(px_get_global("len"), (LXValue[]){_v305}, 1)))) {
        px_srcline(134);
        _v309 = px_index(_v305, _v308);
        px_srcline(135);
        _v310 = px_index(_v309, px_int(0LL));
        px_srcline(136);
        _v311 = px_str("None");
        px_srcline(137);
        if (px_is_truthy(px_ne(_v310, px_null()))) {
            px_srcline(138);
             _v311 = px_add(({ LXValue _s61 = px_add(px_add(px_add(px_str("Some(\n"), px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(12LL))}, 1)), _v310), px_str(",\n")); LXValue _s62 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1); px_add(_s61, _s62); }), px_str(")"));
        }
        px_srcline(139);
        (void)(px_method(_v307, "append", (LXValue[]){px_add(({ LXValue _s73 = px_add(({ LXValue _s71 = ({ LXValue _s69 = px_add(({ LXValue _s67 = ({ LXValue _s65 = px_add(px_add(({ LXValue _s63 = px_add(px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(4LL))}, 1), px_str("(\n")); LXValue _s64 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1); px_add(_s63, _s64); }), _v311), px_str(",\n")); LXValue _s66 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1); px_add(_s65, _s66); }); LXValue _s68 = px_call(px_get_global("dump_node"), (LXValue[]){px_index(_v309, px_int(1LL)), px_add(_v306, px_int(8LL))}, 2); px_add(_s67, _s68); }), px_str(",\n")); LXValue _s70 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(8LL))}, 1); px_add(_s69, _s70); }); LXValue _s72 = px_call(px_get_global("dump_list"), (LXValue[]){px_index(_v309, px_int(2LL)), px_add(_v306, px_int(8LL))}, 2); px_add(_s71, _s72); }), px_str(",\n")); LXValue _s74 = px_call(px_get_global("pad"), (LXValue[]){px_add(_v306, px_int(4LL))}, 1); px_add(_s73, _s74); }), px_str("),"))}, 1));
        px_srcline(140);
         _v308 = px_add(_v308, px_int(1LL));
    }
    px_srcline(141);
    return px_add(({ LXValue _s75 = px_add(px_add(px_str("[\n"), px_call(px_get_global("join"), (LXValue[]){px_str("\n"), _v307}, 2)), px_str("\n")); LXValue _s76 = px_call(px_get_global("pad"), (LXValue[]){_v306}, 1); px_add(_s75, _s76); }), px_str("]"));
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
    px_srcline(143);
    _v314 = px_call(px_get_global("str"), (LXValue[]){_v313}, 1);
    px_srcline(144);
    if (px_is_truthy(({ LXValue _t316 = px_eq(_v314, px_str("inf")); px_is_truthy(_t316) ? _t316 : px_eq(_v314, px_str("-inf")); }))) {
        px_srcline(145);
        return _v314;
    }
    px_srcline(146);
    if (px_is_truthy(({ LXValue _t318 = ({ LXValue _t317 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v314, px_str(".")}, 2)); px_is_truthy(_t317) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v314, px_str("e")}, 2)) : _t317; }); px_is_truthy(_t318) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v314, px_str("E")}, 2)) : _t318; }))) {
        px_srcline(147);
        return px_add(_v314, px_str(".0"));
    }
    px_srcline(148);
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
    px_srcline(150);
    if (px_is_truthy(px_eq(_v320, px_str("s")))) {
        px_srcline(151);
        return _v319;
    }
    px_srcline(152);
    if (px_is_truthy(px_eq(_v320, px_str("vs")))) {
        px_srcline(155);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v319}, 1), px_str("string")))) {
            px_srcline(156);
            return _v319;
        }
        px_srcline(157);
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(158);
    if (px_is_truthy(px_eq(_v320, px_str("r")))) {
        px_srcline(159);
        return px_call(px_get_global("str"), (LXValue[]){_v319}, 1);
    }
    px_srcline(160);
    if (px_is_truthy(px_eq(_v320, px_str("f")))) {
        px_srcline(161);
        return px_call(px_get_global("fmt_float"), (LXValue[]){_v319}, 1);
    }
    px_srcline(162);
    if (px_is_truthy(px_eq(_v320, px_str("n")))) {
        px_srcline(163);
        return px_call(px_get_global("dump_node"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(164);
    if (px_is_truthy(px_eq(_v320, px_str("o")))) {
        px_srcline(165);
        return px_call(px_get_global("dump_opt_node"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(166);
    if (px_is_truthy(px_eq(_v320, px_str("os")))) {
        px_srcline(167);
        return px_call(px_get_global("dump_opt_str"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(168);
    if (px_is_truthy(px_eq(_v320, px_str("ol")))) {
        px_srcline(169);
        return px_call(px_get_global("dump_opt_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(170);
    if (px_is_truthy(px_eq(_v320, px_str("l")))) {
        px_srcline(171);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(172);
    if (px_is_truthy(px_eq(_v320, px_str("ls")))) {
        px_srcline(173);
        return px_call(px_get_global("dump_str_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(174);
    if (px_is_truthy(px_eq(_v320, px_str("tl")))) {
        px_srcline(175);
        return px_call(px_get_global("dump_ty_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(176);
    if (px_is_truthy(px_eq(_v320, px_str("lpl")))) {
        px_srcline(177);
        return px_call(px_get_global("dump_pat_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(178);
    if (px_is_truthy(px_eq(_v320, px_str("lp")))) {
        px_srcline(179);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(180);
    if (px_is_truthy(px_eq(_v320, px_str("lsf")))) {
        px_srcline(181);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(182);
    if (px_is_truthy(px_eq(_v320, px_str("lev")))) {
        px_srcline(183);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(184);
    if (px_is_truthy(px_eq(_v320, px_str("lfd")))) {
        px_srcline(185);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(186);
    if (px_is_truthy(px_eq(_v320, px_str("ltci")))) {
        px_srcline(188);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(189);
    if (px_is_truthy(px_eq(_v320, px_str("lc")))) {
        px_srcline(190);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(191);
    if (px_is_truthy(px_eq(_v320, px_str("lma")))) {
        px_srcline(192);
        return px_call(px_get_global("dump_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(193);
    if (px_is_truthy(px_eq(_v320, px_str("lt2")))) {
        px_srcline(194);
        return px_call(px_get_global("dump_t2_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(195);
    if (px_is_truthy(px_eq(_v320, px_str("lt2b")))) {
        px_srcline(196);
        return px_call(px_get_global("dump_t2b_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(197);
    if (px_is_truthy(px_eq(_v320, px_str("lt3")))) {
        px_srcline(198);
        return px_call(px_get_global("dump_t3_list"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(199);
    if (px_is_truthy(px_eq(_v320, px_str("p")))) {
        px_srcline(200);
        return px_call(px_get_global("dump_pos"), (LXValue[]){_v319, _v321}, 2);
    }
    px_srcline(201);
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
    px_srcline(204);
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
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(({ LXValue _s77 = px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")); LXValue _s78 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1); px_add(_s77, _s78); }), px_str(": 语法错误 ")), _v358), px_str(": ")), _v359)}, 1));
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
    LXValue _v420 = px_null();
    LXValue _v421 = px_null();
    LXValue px_err_422_val = px_null();
    int px_err_422_proped = 0;
    px_srcline(414);
    _v415 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(415);
    _v416 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(416);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(421);
    _v417 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    px_srcline(422);
    _v418 = px_call(px_get_global("qstr"), (LXValue[]){_v417}, 1);
    px_srcline(423);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(424);
        _v419 = px_list_n((LXValue[]){_v418}, 1);
        px_srcline(425);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(426);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(427);
            (void)(px_method(_v419, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1)}, 1)}, 1));
        }
        px_srcline(428);
         _v418 = _v419;
    }
    px_srcline(429);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    px_srcline(430);
    _v420 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(431);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(432);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(433);
    _v421 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v416}, 1);
    px_srcline(434);
    return px_list_n((LXValue[]){px_str("For"), _v418, _v420, _v421, _v415}, 5);
px_err_422:
    if (px_err_422_proped) return px_err_422_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_while");
    LXValue _v423 = px_null();
    LXValue _v424 = px_null();
    LXValue _v425 = px_null();
    LXValue _v426 = px_null();
    LXValue px_err_427_val = px_null();
    int px_err_427_proped = 0;
    px_srcline(436);
    _v423 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(437);
    _v424 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(438);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(439);
    _v425 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(440);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(441);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(442);
    _v426 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v424}, 1);
    px_srcline(443);
    return px_list_n((LXValue[]){px_str("While"), _v425, _v426, _v423}, 4);
px_err_427:
    if (px_err_427_proped) return px_err_427_val;
    return px_null();
}

static LXValue fn_parse_block_ctxt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block_ctxt");
    LXValue _v428 = (nargs > 0) ? args[0] : px_null();
    LXValue _v429 = px_null();
    LXValue _v430 = px_null();
    LXValue px_err_431_val = px_null();
    int px_err_431_proped = 0;
    px_srcline(450);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(451);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(452);
    if (px_is_truthy(px_le(px_get_global("p_paren_ctxt"), px_int(0LL)))) {
        px_srcline(453);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
        px_srcline(454);
        return px_list_n((LXValue[]){}, 0);
    }
    px_srcline(455);
    _v429 = px_list_n((LXValue[]){}, 0);
    px_srcline(456);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(457);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(458);
        _v430 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(459);
        if (px_is_truthy(({ LXValue _t436 = ({ LXValue _t435 = ({ LXValue _t434 = ({ LXValue _t433 = ({ LXValue _t432 = px_eq(_v430, px_str("去缩进")); px_is_truthy(_t432) ? _t432 : px_eq(_v430, px_str("EOF")); }); px_is_truthy(_t433) ? _t433 : px_eq(_v430, px_str(")")); }); px_is_truthy(_t434) ? _t434 : px_eq(_v430, px_str("]")); }); px_is_truthy(_t435) ? _t435 : px_eq(_v430, px_str("}")); }); px_is_truthy(_t436) ? _t436 : px_eq(_v430, px_str(",")); }))) {
            px_srcline(460);
            break;
        }
        px_srcline(461);
        if (px_is_truthy(px_le(px_call(px_get_global("pcol"), (LXValue[]){}, 0), _v428))) {
            px_srcline(462);
            break;
        }
        px_srcline(463);
        (void)(px_method(_v429, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(464);
    return _v429;
px_err_431:
    if (px_err_431_proped) return px_err_431_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block");
    LXValue _v437 = px_null();
    LXValue px_err_438_val = px_null();
    int px_err_438_proped = 0;
    px_srcline(466);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(467);
    _v437 = px_list_n((LXValue[]){}, 0);
    px_srcline(468);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(469);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(470);
        if (px_is_truthy(({ LXValue _t439 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t439) ? _t439 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(471);
            break;
        }
        px_srcline(472);
        (void)(px_method(_v437, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(473);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(474);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    px_srcline(475);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(476);
    return _v437;
px_err_438:
    if (px_err_438_proped) return px_err_438_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_params");
    LXValue _v440 = px_null();
    LXValue _v441 = px_null();
    LXValue _v442 = px_null();
    LXValue _v443 = px_null();
    LXValue px_err_444_val = px_null();
    int px_err_444_proped = 0;
    px_srcline(480);
    _v440 = px_list_n((LXValue[]){}, 0);
    px_srcline(481);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        px_srcline(482);
        return _v440;
    }
    px_srcline(483);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(484);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(485);
        _v441 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        px_srcline(486);
        _v442 = _v441;
        px_srcline(487);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            px_srcline(488);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(489);
            _v443 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
            px_srcline(490);
             _v442 = px_add(px_add(_v441, px_str(": ")), _v443);
        }
        px_srcline(491);
        (void)(px_method(_v440, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v442}, 1)}, 1));
        px_srcline(492);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(493);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(494);
            continue;
        }
        px_srcline(495);
        break;
    }
    px_srcline(496);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(497);
    return _v440;
px_err_444:
    if (px_err_444_proped) return px_err_444_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_func_def");
    LXValue _v445 = px_null();
    LXValue _v446 = px_null();
    LXValue _v447 = px_null();
    LXValue _v448 = px_null();
    LXValue _v449 = px_null();
    LXValue _v450 = px_null();
    LXValue px_err_451_val = px_null();
    int px_err_451_proped = 0;
    px_srcline(499);
    _v445 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(500);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(501);
    _v446 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(502);
    _v447 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(503);
    _v448 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(504);
    _v449 = px_null();
    px_srcline(505);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(506);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(507);
         _v449 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(508);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(509);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(510);
    _v450 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(511);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v446}, 1), _v448, _v449, _v450, _v445, _v447}, 7);
px_err_451:
    if (px_err_451_proped) return px_err_451_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_extern_def");
    LXValue _v452 = px_null();
    LXValue _v453 = px_null();
    LXValue _v454 = px_null();
    LXValue _v455 = px_null();
    LXValue px_err_456_val = px_null();
    int px_err_456_proped = 0;
    px_srcline(515);
    _v452 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(516);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(517);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    px_srcline(518);
    _v453 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(519);
    _v454 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(520);
    _v455 = px_null();
    px_srcline(521);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(522);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(523);
         _v455 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(524);
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v453}, 1), _v454, _v455, _v452}, 5);
px_err_456:
    if (px_err_456_proped) return px_err_456_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_struct_def");
    LXValue _v457 = px_null();
    LXValue _v458 = px_null();
    LXValue _v459 = px_null();
    LXValue _v460 = px_null();
    LXValue _v461 = px_null();
    LXValue _v462 = px_null();
    LXValue _v463 = px_null();
    LXValue px_err_464_val = px_null();
    int px_err_464_proped = 0;
    px_srcline(526);
    _v457 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(527);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(528);
    _v458 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    px_srcline(529);
    _v459 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(530);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(531);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(532);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(533);
    _v460 = px_list_n((LXValue[]){}, 0);
    px_srcline(534);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(535);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(536);
        if (px_is_truthy(({ LXValue _t465 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t465) ? _t465 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(537);
            break;
        }
        px_srcline(538);
        _v461 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(539);
        _v462 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        px_srcline(540);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(541);
        _v463 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(542);
        (void)(px_method(_v460, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v462}, 1), _v463, _v461}, 4)}, 1));
        px_srcline(543);
        if (px_is_truthy(({ LXValue _t466 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t466) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t466; }))) {
            px_srcline(544);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    px_srcline(545);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(546);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    px_srcline(547);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(548);
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v458}, 1), _v460, _v457, _v459}, 5);
px_err_464:
    if (px_err_464_proped) return px_err_464_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_enum_def");
    LXValue _v467 = px_null();
    LXValue _v468 = px_null();
    LXValue _v469 = px_null();
    LXValue _v470 = px_null();
    LXValue _v471 = px_null();
    LXValue _v472 = px_null();
    LXValue px_err_473_val = px_null();
    int px_err_473_proped = 0;
    px_srcline(550);
    _v467 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(551);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(552);
    _v468 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(553);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(554);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(555);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(556);
    _v469 = px_list_n((LXValue[]){}, 0);
    px_srcline(557);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(558);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(559);
        if (px_is_truthy(({ LXValue _t474 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t474) ? _t474 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(560);
            break;
        }
        px_srcline(561);
        _v470 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(562);
        _v471 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        px_srcline(563);
        _v472 = px_list_n((LXValue[]){}, 0);
        px_srcline(564);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(565);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(566);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(567);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(568);
                    (void)(px_method(_v472, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    px_srcline(569);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(570);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(571);
                        continue;
                    }
                    px_srcline(572);
                    break;
                }
            }
            px_srcline(573);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        px_srcline(574);
        (void)(px_method(_v469, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v471}, 1), _v472, _v470}, 4)}, 1));
        px_srcline(575);
        if (px_is_truthy(({ LXValue _t475 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t475) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t475; }))) {
            px_srcline(576);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    px_srcline(577);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(578);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    px_srcline(579);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(580);
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v468}, 1), _v469, _v467}, 4);
px_err_473:
    if (px_err_473_proped) return px_err_473_val;
    return px_null();
}

static LXValue fn_parse_type_const(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_const");
    LXValue _v476 = px_null();
    LXValue _v477 = px_null();
    LXValue _v478 = px_null();
    LXValue _v479 = px_null();
    LXValue _v480 = px_null();
    LXValue _v481 = px_null();
    LXValue px_err_482_val = px_null();
    int px_err_482_proped = 0;
    px_srcline(585);
    _v476 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(586);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(587);
    _v477 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(588);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("const"), px_str("'const'")}, 2));
    px_srcline(589);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(590);
    _v478 = px_list_n((LXValue[]){}, 0);
    px_srcline(591);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(592);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(593);
            _v479 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(594);
            _v480 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("常量名")}, 1);
            px_srcline(595);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
            px_srcline(596);
            _v481 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(597);
            (void)(px_method(_v478, "append", (LXValue[]){px_list_n((LXValue[]){px_str("TypeConstItem"), px_call(px_get_global("qstr"), (LXValue[]){_v480}, 1), _v481, _v479}, 4)}, 1));
            px_srcline(598);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(599);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(600);
                continue;
            }
            px_srcline(601);
            break;
        }
    }
    px_srcline(602);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(603);
    return px_list_n((LXValue[]){px_str("TypeConst"), px_call(px_get_global("qstr"), (LXValue[]){_v477}, 1), _v478, _v476}, 4);
px_err_482:
    if (px_err_482_proped) return px_err_482_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_trait_def");
    LXValue _v483 = px_null();
    LXValue _v484 = px_null();
    LXValue _v485 = px_null();
    LXValue _v486 = px_null();
    LXValue _v487 = px_null();
    LXValue _v488 = px_null();
    LXValue _v489 = px_null();
    LXValue _v490 = px_null();
    LXValue px_err_491_val = px_null();
    int px_err_491_proped = 0;
    px_srcline(605);
    _v483 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(606);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(607);
    _v484 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    px_srcline(608);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(609);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(610);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(611);
    _v485 = px_list_n((LXValue[]){}, 0);
    px_srcline(612);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(613);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(614);
        if (px_is_truthy(({ LXValue _t492 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t492) ? _t492 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(615);
            break;
        }
        px_srcline(616);
        _v486 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(617);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(618);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        px_srcline(619);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(620);
        _v487 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(621);
        _v488 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(622);
        _v489 = px_null();
        px_srcline(623);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(624);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(625);
             _v489 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(626);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(627);
        _v490 = px_list_n((LXValue[]){}, 0);
        px_srcline(628);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(629);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(630);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(631);
                 _v490 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        px_srcline(632);
        (void)(px_method(_v485, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v487}, 1), _v488, _v489, _v490, _v486, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        px_srcline(633);
        if (px_is_truthy(({ LXValue _t493 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t493) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t493; }))) {
            px_srcline(634);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    px_srcline(635);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(636);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    px_srcline(637);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(638);
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v484}, 1), _v485, _v483}, 4);
px_err_491:
    if (px_err_491_proped) return px_err_491_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_impl_def");
    LXValue _v494 = px_null();
    LXValue _v495 = px_null();
    LXValue _v496 = px_null();
    LXValue _v497 = px_null();
    LXValue _v498 = px_null();
    LXValue _v499 = px_null();
    LXValue _v500 = px_null();
    LXValue _v501 = px_null();
    LXValue _v502 = px_null();
    LXValue _v503 = px_null();
    LXValue px_err_504_val = px_null();
    int px_err_504_proped = 0;
    px_srcline(640);
    _v494 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(641);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(642);
    _v495 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    px_srcline(643);
    _v496 = px_null();
    px_srcline(644);
    _v497 = _v495;
    px_srcline(645);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(646);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(647);
         _v497 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
        px_srcline(648);
         _v496 = px_call(px_get_global("qstr"), (LXValue[]){_v495}, 1);
    }
    px_srcline(649);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(650);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(651);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(652);
    _v498 = px_list_n((LXValue[]){}, 0);
    px_srcline(653);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(654);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(655);
        if (px_is_truthy(({ LXValue _t505 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t505) ? _t505 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(656);
            break;
        }
        px_srcline(657);
        _v499 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(658);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(659);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        px_srcline(660);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(661);
        _v500 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(662);
        _v501 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(663);
        _v502 = px_null();
        px_srcline(664);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(665);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(666);
             _v502 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(667);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(668);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(669);
        _v503 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        px_srcline(670);
        (void)(px_method(_v498, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v500}, 1), _v501, _v502, _v503, _v499, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    px_srcline(671);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(672);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    px_srcline(673);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(674);
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v497}, 1), _v496, _v498, _v494}, 5);
px_err_504:
    if (px_err_504_proped) return px_err_504_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import");
    LXValue _v506 = px_null();
    LXValue _v507 = px_null();
    LXValue _v508 = px_null();
    LXValue px_err_509_val = px_null();
    int px_err_509_proped = 0;
    px_srcline(676);
    _v506 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(677);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(678);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        px_srcline(679);
        _v507 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(680);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(681);
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v507}, 1), px_list_n((LXValue[]){}, 0), _v506}, 4);
    }
    px_srcline(682);
    _v508 = px_list_n((LXValue[]){}, 0);
    px_srcline(683);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(684);
        (void)(px_method(_v508, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
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
    return px_list_n((LXValue[]){px_str("Import"), _v508, px_list_n((LXValue[]){}, 0), _v506}, 4);
px_err_509:
    if (px_err_509_proped) return px_err_509_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import_from");
    LXValue _v510 = px_null();
    LXValue _v511 = px_null();
    LXValue _v512 = px_null();
    LXValue px_err_513_val = px_null();
    int px_err_513_proped = 0;
    px_srcline(691);
    _v510 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(692);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(693);
    _v511 = px_list_n((LXValue[]){}, 0);
    px_srcline(694);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(695);
        (void)(px_method(_v511, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(696);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(697);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(698);
            continue;
        }
        px_srcline(699);
        break;
    }
    px_srcline(700);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    px_srcline(701);
    _v512 = px_list_n((LXValue[]){}, 0);
    px_srcline(702);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(703);
        (void)(px_method(_v512, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        px_srcline(704);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(705);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(706);
            continue;
        }
        px_srcline(707);
        break;
    }
    px_srcline(708);
    return px_list_n((LXValue[]){px_str("Import"), _v511, _v512, _v510}, 4);
px_err_513:
    if (px_err_513_proped) return px_err_513_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_select");
    LXValue _v514 = px_null();
    LXValue _v515 = px_null();
    LXValue _v516 = px_null();
    LXValue _v517 = px_null();
    LXValue _v518 = px_null();
    LXValue _v519 = px_null();
    LXValue _v520 = px_null();
    LXValue px_err_521_val = px_null();
    int px_err_521_proped = 0;
    px_srcline(710);
    _v514 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(711);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(712);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(713);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(714);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(715);
    _v515 = px_list_n((LXValue[]){}, 0);
    px_srcline(716);
    _v516 = px_null();
    px_srcline(717);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(718);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(719);
        if (px_is_truthy(({ LXValue _t522 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t522) ? _t522 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(720);
            break;
        }
        px_srcline(721);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(722);
        if (px_is_truthy(({ LXValue _t523 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t523) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t523; }))) {
            px_srcline(723);
            _v517 = px_get_global("p_pos");
            px_srcline(724);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(725);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(726);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(727);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                px_srcline(728);
                 _v516 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                px_srcline(729);
                continue;
            }
            else {
                px_srcline(731);
                px_set_global("p_pos", _v517);
            }
        }
        px_srcline(732);
        _v518 = px_null();
        px_srcline(733);
        if (px_is_truthy(({ LXValue _t524 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t524) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t524; }))) {
            px_srcline(734);
             _v518 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            px_srcline(735);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        px_srcline(736);
        _v519 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(737);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(738);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(739);
        _v520 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        px_srcline(740);
        (void)(px_method(_v515, "append", (LXValue[]){px_list_n((LXValue[]){_v518, _v519, _v520}, 3)}, 1));
    }
    px_srcline(741);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(742);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    px_srcline(743);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(744);
    return px_list_n((LXValue[]){px_str("Select"), _v515, _v516, _v514}, 4);
px_err_521:
    if (px_err_521_proped) return px_err_521_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_case_body");
    LXValue _v525 = px_null();
    LXValue _v526 = px_null();
    LXValue px_err_527_val = px_null();
    int px_err_527_proped = 0;
    px_srcline(746);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(747);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(748);
    _v525 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(749);
    _v526 = px_call(px_get_global("node_pos"), (LXValue[]){_v525}, 1);
    px_srcline(750);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v525, _v526}, 3)}, 1);
px_err_527:
    if (px_err_527_proped) return px_err_527_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_params");
    LXValue _v528 = px_null();
    LXValue _v529 = px_null();
    LXValue _v530 = px_null();
    LXValue _v531 = px_null();
    LXValue _v532 = px_null();
    LXValue px_err_533_val = px_null();
    int px_err_533_proped = 0;
    px_srcline(753);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(754);
    _v528 = px_list_n((LXValue[]){}, 0);
    px_srcline(755);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(756);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(757);
            _v529 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(758);
            _v530 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名")}, 1);
            px_srcline(759);
            _v531 = px_null();
            px_srcline(760);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(761);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(762);
                 _v531 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            px_srcline(763);
            _v532 = px_null();
            px_srcline(764);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                px_srcline(765);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(766);
                 _v532 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            px_srcline(767);
            (void)(px_method(_v528, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v530}, 1), _v531, _v532, _v529}, 5)}, 1));
            px_srcline(768);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(769);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(770);
                continue;
            }
            px_srcline(771);
            break;
        }
    }
    px_srcline(772);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(773);
    return _v528;
px_err_533:
    if (px_err_533_proped) return px_err_533_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_expr");
    LXValue px_err_534_val = px_null();
    int px_err_534_proped = 0;
    px_srcline(776);
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_534:
    if (px_err_534_proped) return px_err_534_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pipe");
    LXValue _v535 = px_null();
    LXValue _v536 = px_null();
    LXValue _v537 = px_null();
    LXValue px_err_538_val = px_null();
    int px_err_538_proped = 0;
    px_srcline(778);
    _v535 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    px_srcline(779);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|>")}, 1))) {
        px_srcline(780);
        _v536 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(781);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(782);
        _v537 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
        px_srcline(783);
         _v535 = px_list_n((LXValue[]){px_str("Pipe"), _v535, _v537, _v536}, 4);
    }
    px_srcline(784);
    return _v535;
px_err_538:
    if (px_err_538_proped) return px_err_538_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_null_coalesce");
    LXValue _v539 = px_null();
    LXValue _v540 = px_null();
    LXValue _v541 = px_null();
    LXValue px_err_542_val = px_null();
    int px_err_542_proped = 0;
    px_srcline(786);
    _v539 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    px_srcline(787);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("??")}, 1))) {
        px_srcline(788);
        _v540 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(789);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(790);
        _v541 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
        px_srcline(791);
         _v539 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v539, _v541, _v540}, 4);
    }
    px_srcline(792);
    return _v539;
px_err_542:
    if (px_err_542_proped) return px_err_542_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_or");
    LXValue _v543 = px_null();
    LXValue _v544 = px_null();
    LXValue _v545 = px_null();
    LXValue px_err_546_val = px_null();
    int px_err_546_proped = 0;
    px_srcline(794);
    _v543 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    px_srcline(795);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("or")}, 1))) {
        px_srcline(796);
        _v544 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(797);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(798);
        _v545 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
        px_srcline(799);
         _v543 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v543, _v545, _v544}, 5);
    }
    px_srcline(800);
    return _v543;
px_err_546:
    if (px_err_546_proped) return px_err_546_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_and");
    LXValue _v547 = px_null();
    LXValue _v548 = px_null();
    LXValue _v549 = px_null();
    LXValue px_err_550_val = px_null();
    int px_err_550_proped = 0;
    px_srcline(802);
    _v547 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    px_srcline(803);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("and")}, 1))) {
        px_srcline(804);
        _v548 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(805);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(806);
        _v549 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
        px_srcline(807);
         _v547 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v547, _v549, _v548}, 5);
    }
    px_srcline(808);
    return _v547;
px_err_550:
    if (px_err_550_proped) return px_err_550_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comparison");
    LXValue _v551 = px_null();
    LXValue _v552 = px_null();
    LXValue _v553 = px_null();
    LXValue _v554 = px_null();
    LXValue px_err_555_val = px_null();
    int px_err_555_proped = 0;
    px_srcline(810);
    _v551 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    px_srcline(811);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(812);
        _v552 = px_null();
        px_srcline(813);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("==")}, 1))) {
            px_srcline(814);
             _v552 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("!=")}, 1))) {
            px_srcline(816);
             _v552 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<")}, 1))) {
            px_srcline(818);
             _v552 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<=")}, 1))) {
            px_srcline(820);
             _v552 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">")}, 1))) {
            px_srcline(822);
             _v552 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">=")}, 1))) {
            px_srcline(824);
             _v552 = px_str("Ge");
        }
        px_srcline(825);
        if (px_is_truthy(px_eq(_v552, px_null()))) {
            px_srcline(826);
            break;
        }
        px_srcline(827);
        _v553 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(828);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(829);
        _v554 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
        px_srcline(830);
         _v551 = px_list_n((LXValue[]){px_str("Binary"), _v552, _v551, _v554, _v553}, 5);
    }
    px_srcline(831);
    return _v551;
px_err_555:
    if (px_err_555_proped) return px_err_555_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitor");
    LXValue _v556 = px_null();
    LXValue _v557 = px_null();
    LXValue _v558 = px_null();
    LXValue px_err_559_val = px_null();
    int px_err_559_proped = 0;
    px_srcline(833);
    _v556 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    px_srcline(834);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|")}, 1))) {
        px_srcline(835);
        _v557 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(836);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(837);
        _v558 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
        px_srcline(838);
         _v556 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v556, _v558, _v557}, 5);
    }
    px_srcline(839);
    return _v556;
px_err_559:
    if (px_err_559_proped) return px_err_559_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitxor");
    LXValue _v560 = px_null();
    LXValue _v561 = px_null();
    LXValue _v562 = px_null();
    LXValue px_err_563_val = px_null();
    int px_err_563_proped = 0;
    px_srcline(841);
    _v560 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    px_srcline(842);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("^")}, 1))) {
        px_srcline(843);
        _v561 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(844);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(845);
        _v562 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
        px_srcline(846);
         _v560 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v560, _v562, _v561}, 5);
    }
    px_srcline(847);
    return _v560;
px_err_563:
    if (px_err_563_proped) return px_err_563_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitand");
    LXValue _v564 = px_null();
    LXValue _v565 = px_null();
    LXValue _v566 = px_null();
    LXValue px_err_567_val = px_null();
    int px_err_567_proped = 0;
    px_srcline(849);
    _v564 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    px_srcline(850);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("&")}, 1))) {
        px_srcline(851);
        _v565 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(852);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(853);
        _v566 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
        px_srcline(854);
         _v564 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v564, _v566, _v565}, 5);
    }
    px_srcline(855);
    return _v564;
px_err_567:
    if (px_err_567_proped) return px_err_567_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_shift");
    LXValue _v568 = px_null();
    LXValue _v569 = px_null();
    LXValue _v570 = px_null();
    LXValue _v571 = px_null();
    LXValue px_err_572_val = px_null();
    int px_err_572_proped = 0;
    px_srcline(857);
    _v568 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    px_srcline(858);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(859);
        _v569 = px_null();
        px_srcline(860);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<<")}, 1))) {
            px_srcline(861);
             _v569 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>")}, 1))) {
            px_srcline(863);
             _v569 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>>")}, 1))) {
            px_srcline(865);
             _v569 = px_str("ShrU");
        }
        px_srcline(866);
        if (px_is_truthy(px_eq(_v569, px_null()))) {
            px_srcline(867);
            break;
        }
        px_srcline(868);
        _v570 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(869);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(870);
        _v571 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
        px_srcline(871);
         _v568 = px_list_n((LXValue[]){px_str("Binary"), _v569, _v568, _v571, _v570}, 5);
    }
    px_srcline(872);
    return _v568;
px_err_572:
    if (px_err_572_proped) return px_err_572_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_add");
    LXValue _v573 = px_null();
    LXValue _v574 = px_null();
    LXValue _v575 = px_null();
    LXValue _v576 = px_null();
    LXValue px_err_577_val = px_null();
    int px_err_577_proped = 0;
    px_srcline(874);
    _v573 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    px_srcline(875);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(876);
        _v574 = px_null();
        px_srcline(877);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("+")}, 1))) {
            px_srcline(878);
             _v574 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("-")}, 1))) {
            px_srcline(880);
             _v574 = px_str("Sub");
        }
        px_srcline(881);
        if (px_is_truthy(px_eq(_v574, px_null()))) {
            px_srcline(882);
            break;
        }
        px_srcline(883);
        _v575 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(884);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(885);
        _v576 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
        px_srcline(886);
         _v573 = px_list_n((LXValue[]){px_str("Binary"), _v574, _v573, _v576, _v575}, 5);
    }
    px_srcline(887);
    return _v573;
px_err_577:
    if (px_err_577_proped) return px_err_577_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_mul");
    LXValue _v578 = px_null();
    LXValue _v579 = px_null();
    LXValue _v580 = px_null();
    LXValue _v581 = px_null();
    LXValue px_err_582_val = px_null();
    int px_err_582_proped = 0;
    px_srcline(889);
    _v578 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    px_srcline(890);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(891);
        _v579 = px_null();
        px_srcline(892);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("*")}, 1))) {
            px_srcline(893);
             _v579 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("/")}, 1))) {
            px_srcline(895);
             _v579 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("//")}, 1))) {
            px_srcline(897);
             _v579 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("%")}, 1))) {
            px_srcline(899);
             _v579 = px_str("Mod");
        }
        px_srcline(900);
        if (px_is_truthy(px_eq(_v579, px_null()))) {
            px_srcline(901);
            break;
        }
        px_srcline(902);
        _v580 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(903);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(904);
        _v581 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(905);
         _v578 = px_list_n((LXValue[]){px_str("Binary"), _v579, _v578, _v581, _v580}, 5);
    }
    px_srcline(906);
    return _v578;
px_err_582:
    if (px_err_582_proped) return px_err_582_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pow");
    LXValue _v583 = px_null();
    LXValue _v584 = px_null();
    LXValue _v585 = px_null();
    LXValue px_err_586_val = px_null();
    int px_err_586_proped = 0;
    px_srcline(908);
    _v583 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    px_srcline(909);
    if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("**")}, 1))) {
        px_srcline(910);
        _v584 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(911);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(912);
        _v585 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(913);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v583, _v585, _v584}, 5);
    }
    px_srcline(914);
    return _v583;
px_err_586:
    if (px_err_586_proped) return px_err_586_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_unary");
    LXValue _v587 = px_null();
    LXValue _v588 = px_null();
    LXValue _v589 = px_null();
    LXValue px_err_590_val = px_null();
    int px_err_590_proped = 0;
    px_srcline(923);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(924);
    _v587 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(925);
    if (px_is_truthy(px_eq(_v587, px_str("-")))) {
        px_srcline(926);
        _v588 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(927);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(928);
        _v589 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(929);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v589, _v588}, 4);
    }
    px_srcline(930);
    if (px_is_truthy(px_eq(_v587, px_str("not")))) {
        px_srcline(931);
        _v588 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(932);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(933);
        _v589 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(934);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v589, _v588}, 4);
    }
    px_srcline(935);
    if (px_is_truthy(px_eq(_v587, px_str("~")))) {
        px_srcline(936);
        _v588 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(937);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(938);
        _v589 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(939);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v589, _v588}, 4);
    }
    px_srcline(940);
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_590:
    if (px_err_590_proped) return px_err_590_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_postfix");
    LXValue _v591 = px_null();
    LXValue _v592 = px_null();
    LXValue _v593 = px_null();
    LXValue _v594 = px_null();
    LXValue _v595 = px_null();
    LXValue _v596 = px_null();
    LXValue _v597 = px_null();
    LXValue _v598 = px_null();
    LXValue px_err_599_val = px_null();
    int px_err_599_proped = 0;
    px_srcline(942);
    _v591 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    px_srcline(943);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(944);
        _v592 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(945);
        if (px_is_truthy(px_eq(_v592, px_str("(")))) {
            px_srcline(946);
            _v593 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(947);
            _v594 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(948);
             _v591 = px_list_n((LXValue[]){px_str("Call"), _v591, _v594, _v593}, 4);
        }
        else if (px_is_truthy(px_eq(_v592, px_str("[")))) {
            px_srcline(950);
            _v593 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(951);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(952);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(953);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(954);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(955);
                _v595 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                px_srcline(956);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(957);
                _v596 = px_null();
                px_srcline(958);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(959);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(960);
                     _v596 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(961);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                }
                px_srcline(962);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                px_srcline(963);
                 _v591 = px_list_n((LXValue[]){px_str("Slice"), _v591, px_null(), _v595, _v596, _v593}, 6);
            }
            else {
                px_srcline(965);
                _v597 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                px_srcline(966);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(967);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(968);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(969);
                    _v595 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(970);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    px_srcline(971);
                    _v596 = px_null();
                    px_srcline(972);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        px_srcline(973);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(974);
                         _v596 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                        px_srcline(975);
                        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    }
                    px_srcline(976);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(977);
                     _v591 = px_list_n((LXValue[]){px_str("Slice"), _v591, _v597, _v595, _v596, _v593}, 6);
                }
                else {
                    px_srcline(979);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(980);
                     _v591 = px_list_n((LXValue[]){px_str("Index"), _v591, _v597, _v593}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v592, px_str(".")))) {
            px_srcline(982);
            _v593 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(983);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(984);
            _v598 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(985);
             _v591 = px_list_n((LXValue[]){px_str("Field"), _v591, px_call(px_get_global("qstr"), (LXValue[]){_v598}, 1), _v593}, 4);
        }
        else if (px_is_truthy(px_eq(_v592, px_str("?.")))) {
            px_srcline(987);
            _v593 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(988);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(989);
            _v598 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(990);
             _v591 = px_list_n((LXValue[]){px_str("OptionalField"), _v591, px_call(px_get_global("qstr"), (LXValue[]){_v598}, 1), _v593}, 4);
        }
        else if (px_is_truthy(px_eq(_v592, px_str("!")))) {
            px_srcline(992);
            _v593 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(993);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(994);
             _v591 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v591, _v593}, 3);
        }
        else if (px_is_truthy(px_eq(_v592, px_str("?")))) {
            px_srcline(996);
            _v593 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(997);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(998);
             _v591 = px_list_n((LXValue[]){px_str("Try"), _v591, _v593}, 3);
        }
        else {
            px_srcline(1000);
            break;
        }
    }
    px_srcline(1001);
    return _v591;
px_err_599:
    if (px_err_599_proped) return px_err_599_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_slice_bound");
    LXValue px_err_600_val = px_null();
    int px_err_600_proped = 0;
    px_srcline(1003);
    if (px_is_truthy(({ LXValue _t601 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t601) ? _t601 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        px_srcline(1004);
        return px_null();
    }
    px_srcline(1005);
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_600:
    if (px_err_600_proped) return px_err_600_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_call_args");
    LXValue _v602 = px_null();
    LXValue px_err_603_val = px_null();
    int px_err_603_proped = 0;
    px_srcline(1007);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(1008);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1009);
    _v602 = px_list_n((LXValue[]){}, 0);
    px_srcline(1010);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(1011);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1012);
            (void)(px_method(_v602, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1013);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1014);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1015);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1016);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(1017);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                    px_srcline(1018);
                    break;
                }
                px_srcline(1019);
                continue;
            }
            px_srcline(1020);
            break;
        }
    }
    px_srcline(1021);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1022);
    return _v602;
px_err_603:
    if (px_err_603_proped) return px_err_603_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_primary");
    LXValue _v604 = px_null();
    LXValue _v605 = px_null();
    LXValue _v606 = px_null();
    LXValue _v607 = px_null();
    LXValue _v608 = px_null();
    LXValue _v609 = px_null();
    LXValue px_err_610_val = px_null();
    int px_err_610_proped = 0;
    px_srcline(1025);
    _v604 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1026);
    if (px_is_truthy(px_eq(_v604, px_str("整数")))) {
        px_srcline(1027);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1028);
        _v606 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1029);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1030);
        return px_list_n((LXValue[]){px_str("Int"), _v606, _v605}, 3);
    }
    px_srcline(1031);
    if (px_is_truthy(px_eq(_v604, px_str("浮点")))) {
        px_srcline(1032);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1033);
        _v606 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1034);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1035);
        return px_list_n((LXValue[]){px_str("Float"), _v606, _v605}, 3);
    }
    px_srcline(1036);
    if (px_is_truthy(px_eq(_v604, px_str("字符串")))) {
        px_srcline(1037);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1038);
        _v606 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1039);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1040);
        return px_list_n((LXValue[]){px_str("Str"), _v606, _v605}, 3);
    }
    px_srcline(1041);
    if (px_is_truthy(px_eq(_v604, px_str("true")))) {
        px_srcline(1042);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1043);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1044);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v605}, 3);
    }
    px_srcline(1045);
    if (px_is_truthy(px_eq(_v604, px_str("false")))) {
        px_srcline(1046);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1047);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1048);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v605}, 3);
    }
    px_srcline(1049);
    if (px_is_truthy(px_eq(_v604, px_str("null")))) {
        px_srcline(1050);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1051);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1052);
        return px_list_n((LXValue[]){px_str("Null"), _v605}, 2);
    }
    px_srcline(1053);
    if (px_is_truthy(px_eq(_v604, px_str("self")))) {
        px_srcline(1054);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1055);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1056);
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v605}, 3);
    }
    px_srcline(1057);
    if (px_is_truthy(px_eq(_v604, px_str("标识符")))) {
        px_srcline(1058);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1059);
        _v607 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1060);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1061);
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v607}, 1), _v605}, 3);
    }
    px_srcline(1062);
    if (px_is_truthy(px_eq(_v604, px_str("[")))) {
        px_srcline(1063);
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    px_srcline(1064);
    if (px_is_truthy(px_eq(_v604, px_str("(")))) {
        px_srcline(1065);
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    px_srcline(1066);
    if (px_is_truthy(px_eq(_v604, px_str("{")))) {
        px_srcline(1067);
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    px_srcline(1068);
    if (px_is_truthy(px_eq(_v604, px_str("fn")))) {
        px_srcline(1069);
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    px_srcline(1070);
    if (px_is_truthy(px_eq(_v604, px_str("match")))) {
        px_srcline(1071);
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1072);
    if (px_is_truthy(px_eq(_v604, px_str("if")))) {
        px_srcline(1073);
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1074);
    if (px_is_truthy(px_eq(_v604, px_str("chan")))) {
        px_srcline(1075);
        _v605 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1076);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1077);
        _v608 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v605}, 3);
        px_srcline(1078);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1079);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1080);
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            px_srcline(1081);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        px_srcline(1082);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1083);
            _v609 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(1084);
             _v608 = px_list_n((LXValue[]){px_str("Call"), _v608, _v609, _v605}, 4);
        }
        px_srcline(1085);
        return _v608;
    }
    px_srcline(1086);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1087);
    return px_null();
px_err_610:
    if (px_err_610_proped) return px_err_610_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_list_or_comp");
    LXValue _v611 = px_null();
    LXValue _v612 = px_null();
    LXValue _v613 = px_null();
    LXValue _v614 = px_null();
    LXValue px_err_615_val = px_null();
    int px_err_615_proped = 0;
    px_srcline(1089);
    _v611 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1090);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1091);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1092);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        px_srcline(1093);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1094);
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v611}, 3);
    }
    px_srcline(1095);
    _v612 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1096);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1097);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1098);
        _v613 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1099);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1100);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1101);
        return px_list_n((LXValue[]){px_str("ListComp"), _v612, px_index(_v613, px_int(0LL)), px_index(_v613, px_int(1LL)), _v611}, 5);
    }
    px_srcline(1102);
    _v614 = px_list_n((LXValue[]){_v612}, 1);
    px_srcline(1103);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1104);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1105);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1106);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            px_srcline(1107);
            break;
        }
        px_srcline(1108);
        (void)(px_method(_v614, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        px_srcline(1109);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    }
    px_srcline(1110);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(1111);
    return px_list_n((LXValue[]){px_str("List"), _v614, _v611}, 3);
px_err_615:
    if (px_err_615_proped) return px_err_615_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_vars");
    LXValue _v616 = px_null();
    LXValue px_err_617_val = px_null();
    int px_err_617_proped = 0;
    px_srcline(1113);
    _v616 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    px_srcline(1114);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1115);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1116);
        (void)(px_method(_v616, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    px_srcline(1117);
    return _v616;
px_err_617:
    if (px_err_617_proped) return px_err_617_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_clauses");
    LXValue _v618 = px_null();
    LXValue _v619 = px_null();
    LXValue _v620 = px_null();
    LXValue _v621 = px_null();
    LXValue px_err_622_val = px_null();
    int px_err_622_proped = 0;
    px_srcline(1119);
    _v618 = px_list_n((LXValue[]){}, 0);
    px_srcline(1120);
    _v619 = px_list_n((LXValue[]){}, 0);
    px_srcline(1121);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1122);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1123);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1124);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1125);
            _v620 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            px_srcline(1126);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            px_srcline(1127);
            _v621 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1128);
            (void)(px_method(_v618, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v620, _v621}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1130);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1131);
            (void)(px_method(_v619, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(1133);
            break;
        }
    }
    px_srcline(1134);
    return px_list_n((LXValue[]){_v618, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v619}, 1)}, 2);
px_err_622:
    if (px_err_622_proped) return px_err_622_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fold_comp_conds");
    LXValue _v623 = (nargs > 0) ? args[0] : px_null();
    LXValue _v624 = px_null();
    LXValue _v625 = px_null();
    LXValue _v626 = px_null();
    LXValue px_err_627_val = px_null();
    int px_err_627_proped = 0;
    px_srcline(1136);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v623}, 1), px_int(0LL)))) {
        px_srcline(1137);
        return px_null();
    }
    px_srcline(1138);
    _v624 = px_index(_v623, px_int(0LL));
    px_srcline(1139);
    _v625 = px_int(1LL);
    px_srcline(1140);
    while (px_is_truthy(px_lt(_v625, px_call(px_get_global("len"), (LXValue[]){_v623}, 1)))) {
        px_srcline(1141);
        _v626 = px_call(px_get_global("node_pos"), (LXValue[]){_v624}, 1);
        px_srcline(1142);
         _v624 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v624, px_index(_v623, _v625), _v626}, 5);
        px_srcline(1143);
         _v625 = px_add(_v625, px_int(1LL));
    }
    px_srcline(1144);
    return _v624;
px_err_627:
    if (px_err_627_proped) return px_err_627_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_paren_or_tuple");
    LXValue _v628 = px_null();
    LXValue _v629 = px_null();
    LXValue _v630 = px_null();
    LXValue _v631 = px_null();
    LXValue px_err_632_val = px_null();
    int px_err_632_proped = 0;
    px_srcline(1146);
    _v628 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1147);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1148);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1149);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        px_srcline(1150);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1151);
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v628}, 3);
    }
    px_srcline(1152);
    _v629 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1153);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1154);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1155);
        _v630 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1156);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1157);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1158);
        return px_list_n((LXValue[]){px_str("GenExp"), _v629, px_index(_v630, px_int(0LL)), px_index(_v630, px_int(1LL)), _v628}, 5);
    }
    px_srcline(1159);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1160);
        _v631 = px_list_n((LXValue[]){_v629}, 1);
        px_srcline(1161);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1162);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1163);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1164);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                px_srcline(1165);
                break;
            }
            px_srcline(1166);
            (void)(px_method(_v631, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1167);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        }
        px_srcline(1168);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1169);
        return px_list_n((LXValue[]){px_str("Tuple"), _v631, _v628}, 3);
    }
    px_srcline(1170);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1171);
    return _v629;
px_err_632:
    if (px_err_632_proped) return px_err_632_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("brace_looks_like_dict");
    LXValue _v633 = px_null();
    LXValue _v634 = px_null();
    LXValue _v635 = px_null();
    LXValue px_err_636_val = px_null();
    int px_err_636_proped = 0;
    px_srcline(1173);
    _v633 = px_int(0LL);
    px_srcline(1174);
    _v634 = px_get_global("p_pos");
    px_srcline(1175);
    while (px_is_truthy(px_lt(_v634, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(1176);
        _v635 = px_index(px_index(px_get_global("p_toks"), _v634), px_int(0LL));
        px_srcline(1177);
        if (px_is_truthy(({ LXValue _t637 = px_eq(_v635, px_str(":")); px_is_truthy(_t637) ? px_eq(_v633, px_int(0LL)) : _t637; }))) {
            px_srcline(1178);
            return px_bool(true);
        }
        px_srcline(1179);
        if (px_is_truthy(({ LXValue _t639 = ({ LXValue _t638 = px_eq(_v635, px_str("(")); px_is_truthy(_t638) ? _t638 : px_eq(_v635, px_str("[")); }); px_is_truthy(_t639) ? _t639 : px_eq(_v635, px_str("{")); }))) {
            px_srcline(1180);
             _v633 = px_add(_v633, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t640 = px_eq(_v635, px_str(")")); px_is_truthy(_t640) ? _t640 : px_eq(_v635, px_str("]")); }))) {
            px_srcline(1182);
            if (px_is_truthy(px_gt(_v633, px_int(0LL)))) {
                px_srcline(1183);
                 _v633 = px_sub(_v633, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t641 = px_eq(_v635, px_str("}")); px_is_truthy(_t641) ? px_eq(_v633, px_int(0LL)) : _t641; }))) {
            px_srcline(1185);
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t644 = ({ LXValue _t643 = ({ LXValue _t642 = px_eq(_v635, px_str(",")); px_is_truthy(_t642) ? _t642 : px_eq(_v635, px_str("换行")); }); px_is_truthy(_t643) ? _t643 : px_eq(_v635, px_str("EOF")); }); px_is_truthy(_t644) ? px_eq(_v633, px_int(0LL)) : _t644; }))) {
            px_srcline(1187);
            return px_bool(false);
        }
        px_srcline(1188);
         _v634 = px_add(_v634, px_int(1LL));
    }
    px_srcline(1189);
    return px_bool(false);
px_err_636:
    if (px_err_636_proped) return px_err_636_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_brace");
    LXValue _v645 = px_null();
    LXValue _v646 = px_null();
    LXValue _v647 = px_null();
    LXValue _v648 = px_null();
    LXValue _v649 = px_null();
    LXValue _v650 = px_null();
    LXValue _v651 = px_null();
    LXValue _v652 = px_null();
    LXValue _v653 = px_null();
    LXValue px_err_654_val = px_null();
    int px_err_654_proped = 0;
    px_srcline(1191);
    _v645 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1192);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1193);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1194);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1205);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
        px_srcline(1206);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1207);
        return px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){}, 0), _v645}, 3);
    }
    px_srcline(1208);
    _v646 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    px_srcline(1209);
    if (px_is_truthy(_v646)) {
        px_srcline(1210);
        _v647 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1211);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1212);
        _v648 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1213);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1214);
            _v649 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            px_srcline(1215);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            px_srcline(1216);
            return px_list_n((LXValue[]){px_str("DictComp"), _v647, _v648, px_index(_v649, px_int(0LL)), px_index(_v649, px_int(1LL)), _v645}, 6);
        }
        px_srcline(1217);
        _v650 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v647, _v648}, 2)}, 1);
        px_srcline(1221);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1222);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1223);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1224);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1225);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1226);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            px_srcline(1227);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                px_srcline(1228);
                break;
            }
            px_srcline(1229);
            _v651 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1230);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(1231);
            _v652 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1232);
            (void)(px_method(_v650, "append", (LXValue[]){px_list_n((LXValue[]){_v651, _v652}, 2)}, 1));
            px_srcline(1233);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1234);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1235);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1236);
        return px_list_n((LXValue[]){px_str("Dict"), _v650, _v645}, 3);
    }
    px_srcline(1237);
    _v653 = px_list_n((LXValue[]){}, 0);
    px_srcline(1238);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1239);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1240);
    while (px_is_truthy(({ LXValue _t655 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t655) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t655; }))) {
        px_srcline(1241);
        (void)(px_method(_v653, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(1242);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1243);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    px_srcline(1244);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    px_srcline(1245);
    return px_list_n((LXValue[]){px_str("Block"), _v653, _v645}, 3);
px_err_654:
    if (px_err_654_proped) return px_err_654_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_closure");
    LXValue _v656 = px_null();
    LXValue _v657 = px_null();
    LXValue _v658 = px_null();
    LXValue _v659 = px_null();
    LXValue _v660 = px_null();
    LXValue _v661 = px_null();
    LXValue _v662 = px_null();
    LXValue px_err_663_val = px_null();
    int px_err_663_proped = 0;
    px_srcline(1247);
    _v656 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1248);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1249);
    _v657 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(1250);
    _v658 = px_null();
    px_srcline(1251);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(1252);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1253);
         _v658 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(1254);
    _v659 = px_list_n((LXValue[]){}, 0);
    px_srcline(1255);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        px_srcline(1256);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1257);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1258);
            (void)(px_method(_v659, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            px_srcline(1259);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1260);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1261);
                continue;
            }
            px_srcline(1262);
            break;
        }
    }
    px_srcline(1263);
    _v660 = px_null();
    px_srcline(1264);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        px_srcline(1265);
        _v661 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1266);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1267);
        _v662 = px_list_n((LXValue[]){}, 0);
        px_srcline(1268);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1269);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1270);
        while (px_is_truthy(({ LXValue _t664 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t664) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t664; }))) {
            px_srcline(1271);
            (void)(px_method(_v662, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            px_srcline(1272);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1273);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1274);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1275);
         _v660 = px_list_n((LXValue[]){px_str("Block"), _v662, _v661}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(1277);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1281);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(1282);
            _v661 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1283);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1284);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(1285);
                _v662 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
                px_srcline(1286);
                 _v660 = px_list_n((LXValue[]){px_str("Block"), _v662, _v661}, 3);
            }
            else {
                px_srcline(1293);
                _v662 = px_list_n((LXValue[]){}, 0);
                px_srcline(1294);
                px_set_global("p_paren_ctxt", px_add(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1295);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1296);
                    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
                    px_srcline(1297);
                    if (px_is_truthy(({ LXValue _t669 = ({ LXValue _t668 = ({ LXValue _t667 = ({ LXValue _t666 = ({ LXValue _t665 = px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1); px_is_truthy(_t665) ? _t665 : px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); }); px_is_truthy(_t666) ? _t666 : px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1); }); px_is_truthy(_t667) ? _t667 : px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1); }); px_is_truthy(_t668) ? _t668 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t669) ? _t669 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
                        px_srcline(1298);
                        break;
                    }
                    px_srcline(1299);
                    (void)(px_method(_v662, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
                }
                px_srcline(1300);
                px_set_global("p_paren_ctxt", px_sub(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1301);
                 _v660 = px_list_n((LXValue[]){px_str("Block"), _v662, _v661}, 3);
            }
        }
        else {
            px_srcline(1303);
             _v660 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
    }
    else {
        px_srcline(1305);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    px_srcline(1306);
    return px_list_n((LXValue[]){px_str("Closure"), _v657, _v658, _v660, _v659, _v656}, 6);
px_err_663:
    if (px_err_663_proped) return px_err_663_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_match_expr");
    LXValue _v670 = px_null();
    LXValue _v671 = px_null();
    LXValue _v672 = px_null();
    LXValue _v673 = px_null();
    LXValue _v674 = px_null();
    LXValue _v675 = px_null();
    LXValue _v676 = px_null();
    LXValue _v677 = px_null();
    LXValue px_err_678_val = px_null();
    int px_err_678_proped = 0;
    px_srcline(1308);
    _v670 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1309);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1310);
    _v671 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1311);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1312);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(1313);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(1314);
    _v672 = px_list_n((LXValue[]){}, 0);
    px_srcline(1315);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1316);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(1317);
        if (px_is_truthy(({ LXValue _t679 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t679) ? _t679 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(1318);
            break;
        }
        px_srcline(1319);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(1320);
        _v673 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        px_srcline(1321);
        _v674 = px_null();
        px_srcline(1322);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1323);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1324);
             _v674 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1325);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1326);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(1327);
        _v675 = px_null();
        px_srcline(1328);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            px_srcline(1329);
            _v676 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(1330);
            _v677 = px_null();
            px_srcline(1331);
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v676}, 1), px_int(0LL)))) {
                px_srcline(1332);
                 _v677 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v676, px_int(0LL))}, 1);
            }
            else {
                px_srcline(1334);
                 _v677 = _v670;
            }
            px_srcline(1335);
             _v675 = px_list_n((LXValue[]){px_str("Block"), _v676, _v677}, 3);
        }
        else {
            px_srcline(1337);
             _v675 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1338);
        (void)(px_method(_v672, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v673, _v674, _v675, _v670}, 5)}, 1));
    }
    px_srcline(1339);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(1340);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    px_srcline(1341);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(1342);
    return px_list_n((LXValue[]){px_str("Match"), _v671, _v672, _v670}, 4);
px_err_678:
    if (px_err_678_proped) return px_err_678_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if_expr");
    LXValue _v680 = px_null();
    LXValue _v681 = px_null();
    LXValue _v682 = px_null();
    LXValue _v683 = px_null();
    LXValue px_err_684_val = px_null();
    int px_err_684_proped = 0;
    px_srcline(1344);
    _v680 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1345);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1346);
    _v681 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1347);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1348);
    _v682 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1349);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    px_srcline(1350);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1351);
    _v683 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1352);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v681, _v682, _v683, _v680}, 5);
px_err_684:
    if (px_err_684_proped) return px_err_684_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pattern");
    LXValue _v685 = px_null();
    LXValue _v686 = px_null();
    LXValue _v687 = px_null();
    LXValue _v688 = px_null();
    LXValue _v689 = px_null();
    LXValue _v690 = px_null();
    LXValue _v691 = px_null();
    LXValue _v692 = px_null();
    LXValue px_err_693_val = px_null();
    int px_err_693_proped = 0;
    px_srcline(1355);
    _v685 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1356);
    if (px_is_truthy(({ LXValue _t698 = ({ LXValue _t697 = ({ LXValue _t696 = ({ LXValue _t695 = ({ LXValue _t694 = px_eq(_v685, px_str("整数")); px_is_truthy(_t694) ? _t694 : px_eq(_v685, px_str("浮点")); }); px_is_truthy(_t695) ? _t695 : px_eq(_v685, px_str("字符串")); }); px_is_truthy(_t696) ? _t696 : px_eq(_v685, px_str("true")); }); px_is_truthy(_t697) ? _t697 : px_eq(_v685, px_str("false")); }); px_is_truthy(_t698) ? _t698 : px_eq(_v685, px_str("null")); }))) {
        px_srcline(1357);
        _v686 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        px_srcline(1358);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v686}, 2);
    }
    px_srcline(1359);
    if (px_is_truthy(px_eq(_v685, px_str("标识符")))) {
        px_srcline(1360);
        _v687 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1361);
        _v688 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1362);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1363);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(1365);
            _v689 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1366);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1367);
            _v690 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名")}, 1);
            px_srcline(1368);
            return px_list_n((LXValue[]){px_str("PatLiteral"), ({ LXValue _s79 = px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v687}, 1), _v689}, 3); LXValue _s80 = px_call(px_get_global("qstr"), (LXValue[]){_v690}, 1); px_list_n((LXValue[]){px_str("Field"), _s79, _s80, _v689}, 4); })}, 2);
        }
        px_srcline(1369);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1370);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1371);
            _v691 = px_list_n((LXValue[]){}, 0);
            px_srcline(1372);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(1373);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1374);
                    (void)(px_method(_v691, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    px_srcline(1375);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(1376);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(1377);
                        continue;
                    }
                    px_srcline(1378);
                    break;
                }
            }
            px_srcline(1379);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            px_srcline(1380);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v687}, 1), _v691}, 3);
        }
        px_srcline(1381);
        if (px_is_truthy(px_eq(_v687, px_str("_")))) {
            px_srcline(1382);
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        px_srcline(1383);
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v687}, 1))) {
            px_srcline(1384);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v687}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        px_srcline(1385);
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v687}, 1)}, 2);
    }
    px_srcline(1386);
    if (px_is_truthy(px_eq(_v685, px_str("(")))) {
        px_srcline(1387);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1388);
        _v692 = px_list_n((LXValue[]){}, 0);
        px_srcline(1389);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1390);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1391);
                (void)(px_method(_v692, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                px_srcline(1392);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1393);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1394);
                    continue;
                }
                px_srcline(1395);
                break;
            }
        }
        px_srcline(1396);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1397);
        return px_list_n((LXValue[]){px_str("PatTuple"), _v692}, 2);
    }
    px_srcline(1398);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1399);
    return px_null();
px_err_693:
    if (px_err_693_proped) return px_err_693_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_upper");
    LXValue _v699 = (nargs > 0) ? args[0] : px_null();
    LXValue _v700 = px_null();
    LXValue px_err_701_val = px_null();
    int px_err_701_proped = 0;
    px_srcline(1401);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v699}, 1), px_int(0LL)))) {
        px_srcline(1402);
        return px_bool(false);
    }
    px_srcline(1403);
    _v700 = px_index(_v699, px_int(0LL));
    px_srcline(1404);
    return ({ LXValue _t702 = px_ge(_v700, px_str("A")); px_is_truthy(_t702) ? px_le(_v700, px_str("Z")) : _t702; });
px_err_701:
    if (px_err_701_proped) return px_err_701_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type");
    LXValue _v703 = px_null();
    LXValue _v704 = px_null();
    LXValue px_err_705_val = px_null();
    int px_err_705_proped = 0;
    px_srcline(1407);
    _v703 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    px_srcline(1408);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        px_srcline(1409);
        _v704 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1410);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1411);
        return px_list_n((LXValue[]){px_str("TyOptional"), _v703, _v704}, 3);
    }
    px_srcline(1412);
    return _v703;
px_err_705:
    if (px_err_705_proped) return px_err_705_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_base");
    LXValue _v706 = px_null();
    LXValue _v707 = px_null();
    LXValue _v708 = px_null();
    LXValue _v709 = px_null();
    LXValue _v710 = px_null();
    LXValue _v711 = px_null();
    LXValue _v712 = px_null();
    LXValue _v713 = px_null();
    LXValue px_err_714_val = px_null();
    int px_err_714_proped = 0;
    px_srcline(1414);
    _v706 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1415);
    if (px_is_truthy(px_eq(_v706, px_str("标识符")))) {
        px_srcline(1416);
        _v707 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1417);
        _v708 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1418);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1419);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1420);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1421);
            _v709 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1422);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            px_srcline(1423);
            if (px_is_truthy(px_eq(_v707, px_str("list")))) {
                px_srcline(1424);
                return px_list_n((LXValue[]){px_str("TyList"), _v709, _v708}, 3);
            }
            px_srcline(1425);
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v707}, 1), px_list_n((LXValue[]){_v709}, 1), _v708}, 4);
        }
        px_srcline(1426);
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v707}, 1), _v708}, 3);
    }
    px_srcline(1427);
    if (px_is_truthy(px_eq(_v706, px_str("[")))) {
        px_srcline(1428);
        _v708 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1429);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1430);
        _v709 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1431);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1432);
        return px_list_n((LXValue[]){px_str("TyList"), _v709, _v708}, 3);
    }
    px_srcline(1433);
    if (px_is_truthy(px_eq(_v706, px_str("{")))) {
        px_srcline(1434);
        _v708 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1435);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1436);
        _v710 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1437);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1438);
        _v711 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1439);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1440);
        return px_list_n((LXValue[]){px_str("TyDict"), _v710, _v711, _v708}, 4);
    }
    px_srcline(1441);
    if (px_is_truthy(px_eq(_v706, px_str("(")))) {
        px_srcline(1442);
        _v708 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1443);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1444);
        _v712 = px_list_n((LXValue[]){}, 0);
        px_srcline(1445);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1446);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1447);
                (void)(px_method(_v712, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                px_srcline(1448);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1449);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1450);
                    continue;
                }
                px_srcline(1451);
                break;
            }
        }
        px_srcline(1452);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1453);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(1454);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1455);
            _v713 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1456);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v712, _v713, _v708}, 4);
        }
        px_srcline(1457);
        return px_list_n((LXValue[]){px_str("TyTuple"), _v712, _v708}, 3);
    }
    px_srcline(1458);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1459);
    return px_null();
px_err_714:
    if (px_err_714_proped) return px_err_714_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt_inner");
    LXValue _v715 = (nargs > 0) ? args[0] : px_null();
    LXValue _v716 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v756 = px_null();
    LXValue _v757 = px_null();
    LXValue _v758 = px_null();
    LXValue _v759 = px_null();
    LXValue _v760 = px_null();
    LXValue _v761 = px_null();
    LXValue _v762 = px_null();
    LXValue _v763 = px_null();
    LXValue _v764 = px_null();
    LXValue _v765 = px_null();
    LXValue _v766 = px_null();
    LXValue _v767 = px_null();
    LXValue _v768 = px_null();
    LXValue px_err_769_val = px_null();
    int px_err_769_proped = 0;
    px_srcline(9);
    _v717 = px_call(px_get_global("cg_pad"), (LXValue[]){_v716}, 1);
    px_srcline(10);
    _v718 = px_index(_v715, px_int(0LL));
    px_srcline(11);
    if (px_is_truthy(px_eq(_v718, px_str("VarDecl")))) {
        px_srcline(12);
        _v719 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v715, px_int(2LL))}, 1);
        px_srcline(15);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v715}, 1));
        px_srcline(16);
        _v720 = px_str("px_null()");
        px_srcline(17);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(4LL)), px_null()))) {
            px_srcline(18);
             _v720 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(4LL))}, 1);
        }
        px_srcline(23);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(26);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(27);
            return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v719), px_str("\", ")), _v720), px_str(");\n"));
        }
        px_srcline(28);
        _v721 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v719}, 1);
        px_srcline(29);
        if (px_is_truthy(px_eq(_v721, px_null()))) {
            px_srcline(31);
             _v721 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v719}, 1);
            px_srcline(33);
            if (px_is_truthy(px_ne(px_index(_v715, px_int(4LL)), px_null()))) {
                px_srcline(34);
                _v722 = px_index(_v715, px_int(4LL));
                px_srcline(35);
                _v723 = px_null();
                px_srcline(36);
                if (px_is_truthy(px_eq(px_index(_v722, px_int(0LL)), px_str("Constructor")))) {
                    px_srcline(37);
                     _v723 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v722, px_int(1LL))}, 1);
                }
                else if (px_is_truthy(px_eq(px_index(_v722, px_int(0LL)), px_str("Call")))) {
                    px_srcline(39);
                    _v724 = px_index(_v722, px_int(1LL));
                    px_srcline(40);
                    if (px_is_truthy(px_eq(px_index(_v724, px_int(0LL)), px_str("Var")))) {
                        px_srcline(41);
                         _v723 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v724, px_int(1LL))}, 1);
                    }
                }
                px_srcline(42);
                if (px_is_truthy(px_ne(_v723, px_null()))) {
                    px_srcline(43);
                    if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v723}, 1))) {
                        px_srcline(44);
                        px_index_set(px_get_global("cg_var_types"), _v719, _v723);
                    }
                }
            }
            px_srcline(45);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v719}, 1))) {
                px_srcline(46);
                return px_add(px_add(px_add(px_add(px_add(_v717, px_str("LXValue ")), _v721), px_str(" = px_cell(")), _v720), px_str(");\n"));
            }
            px_srcline(47);
            return px_add(px_add(px_add(px_add(px_add(_v717, px_str("LXValue ")), _v721), px_str(" = ")), _v720), px_str(";\n"));
        }
        px_srcline(50);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(4LL)), px_null()))) {
            px_srcline(51);
            _v722 = px_index(_v715, px_int(4LL));
            px_srcline(52);
            _v723 = px_null();
            px_srcline(53);
            if (px_is_truthy(px_eq(px_index(_v722, px_int(0LL)), px_str("Constructor")))) {
                px_srcline(54);
                 _v723 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v722, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v722, px_int(0LL)), px_str("Call")))) {
                px_srcline(56);
                _v724 = px_index(_v722, px_int(1LL));
                px_srcline(57);
                if (px_is_truthy(px_eq(px_index(_v724, px_int(0LL)), px_str("Var")))) {
                    px_srcline(58);
                     _v723 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v724, px_int(1LL))}, 1);
                }
            }
            px_srcline(59);
            if (px_is_truthy(px_ne(_v723, px_null()))) {
                px_srcline(60);
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v723}, 1))) {
                    px_srcline(61);
                    px_index_set(px_get_global("cg_var_types"), _v719, _v723);
                }
            }
        }
        px_srcline(62);
        return px_add(px_add(_v717, px_call(px_get_global("cg_store_of"), (LXValue[]){_v719, _v721, _v720}, 3)), px_str(";\n"));
    }
    px_srcline(63);
    if (px_is_truthy(px_eq(_v718, px_str("Assign")))) {
        px_srcline(64);
        _v720 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(3LL))}, 1);
        px_srcline(65);
        _v725 = px_index(_v715, px_int(1LL));
        px_srcline(66);
        _v726 = px_index(_v715, px_int(2LL));
        px_srcline(68);
        if (px_is_truthy(px_eq(_v726, px_str("Append")))) {
            px_srcline(69);
            _v727 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v725}, 1);
            px_srcline(70);
            _v728 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){_v725, px_index(_v715, px_int(3LL))}, 2), px_list_n((LXValue[]){_v727, _v720}, 2)}, 2);
            px_srcline(71);
            _v729 = px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v728, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_method("), px_index(px_index(_v728, px_int(1LL)), px_int(0LL))), px_str(", \"append\", (LXValue[]){")), px_index(px_index(_v728, px_int(1LL)), px_int(1LL))), px_str("}, 1)"))}, 2);
            px_srcline(72);
            return px_add(px_add(px_add(_v717, px_str("(void)(")), _v729), px_str(");\n"));
        }
        px_srcline(73);
        _v730 = px_index(_v725, px_int(0LL));
        px_srcline(74);
        if (px_is_truthy(px_eq(_v730, px_str("Var")))) {
            px_srcline(75);
            _v719 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v725, px_int(1LL))}, 1);
            px_srcline(77);
            (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v725, _v726, px_index(_v715, px_int(3LL))}, 3));
            px_srcline(78);
            _v731 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v719}, 1);
            px_srcline(79);
            if (px_is_truthy(px_eq(_v731, px_null()))) {
                px_srcline(81);
                if (px_is_truthy(px_eq(_v726, px_str("Assign")))) {
                    px_srcline(82);
                    return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v719), px_str("\", ")), _v720), px_str(");\n"));
                }
                px_srcline(83);
                _v732 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v726, _v719, _v720}, 3);
                px_srcline(84);
                return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v719), px_str("\", ")), _v732), px_str(");\n"));
            }
            px_srcline(85);
            _v732 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v726, px_call(px_get_global("cg_load_of"), (LXValue[]){_v719, _v731}, 2), _v720}, 3);
            px_srcline(86);
            return px_add(px_add(px_add(_v717, px_str(" ")), px_call(px_get_global("cg_store_of"), (LXValue[]){_v719, _v731, _v732}, 3)), px_str(";\n"));
        }
        px_srcline(87);
        if (px_is_truthy(px_eq(_v730, px_str("Field")))) {
            px_srcline(88);
            _v727 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v725, px_int(1LL))}, 1);
            px_srcline(89);
            _v733 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v725, px_int(2LL))}, 1);
            px_srcline(90);
            _v728 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v725, px_int(1LL)), px_index(_v715, px_int(3LL))}, 2), px_list_n((LXValue[]){_v727, _v720}, 2)}, 2);
            px_srcline(91);
            return px_add(px_add(_v717, px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v728, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_field_set("), px_index(px_index(_v728, px_int(1LL)), px_int(0LL))), px_str(", \"")), _v733), px_str("\", ")), px_index(px_index(_v728, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2)), px_str(";\n"));
        }
        px_srcline(92);
        if (px_is_truthy(px_eq(_v730, px_str("Index")))) {
            px_srcline(93);
            _v727 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v725, px_int(1LL))}, 1);
            px_srcline(94);
            _v734 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v725, px_int(2LL))}, 1);
            px_srcline(95);
            _v728 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v725, px_int(1LL)), px_index(_v725, px_int(2LL)), px_index(_v715, px_int(3LL))}, 3), px_list_n((LXValue[]){_v727, _v734, _v720}, 3)}, 2);
            px_srcline(96);
            return px_add(px_add(_v717, px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v728, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_index_set("), px_index(px_index(_v728, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v728, px_int(1LL)), px_int(1LL))), px_str(", ")), px_index(px_index(_v728, px_int(1LL)), px_int(2LL))), px_str(")"))}, 2)), px_str(";\n"));
        }
        px_srcline(97);
        return px_str("不支持的赋值目标");
    }
    px_srcline(98);
    if (px_is_truthy(px_eq(_v718, px_str("ExprStmt")))) {
        px_srcline(99);
        _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(100);
        return px_add(px_add(px_add(_v717, px_str("(void)(")), _v722), px_str(");\n"));
    }
    px_srcline(101);
    if (px_is_truthy(px_eq(_v718, px_str("If")))) {
        px_srcline(102);
        _v735 = px_str("");
        px_srcline(103);
        _v736 = px_index(_v715, px_int(1LL));
        px_srcline(104);
        _v737 = px_int(0LL);
        px_srcline(105);
        while (px_is_truthy(px_lt(_v737, px_call(px_get_global("len"), (LXValue[]){_v736}, 1)))) {
            px_srcline(106);
            _v738 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v736, _v737), px_int(0LL))}, 1);
            px_srcline(107);
            _v739 = px_str("if");
            px_srcline(108);
            if (px_is_truthy(px_gt(_v737, px_int(0LL)))) {
                px_srcline(109);
                 _v739 = px_str("else if");
            }
            px_srcline(110);
             _v735 = px_add(_v735, px_add(px_add(px_add(px_add(_v717, _v739), px_str(" (px_is_truthy(")), _v738), px_str(")) {\n")));
            px_srcline(111);
            _v740 = px_index(px_index(_v736, _v737), px_int(1LL));
            px_srcline(112);
            _v741 = px_int(0LL);
            px_srcline(113);
            while (px_is_truthy(px_lt(_v741, px_call(px_get_global("len"), (LXValue[]){_v740}, 1)))) {
                px_srcline(114);
                 _v735 = px_add(_v735, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v740, _v741), px_add(_v716, px_int(1LL))}, 2));
                px_srcline(115);
                 _v741 = px_add(_v741, px_int(1LL));
            }
            px_srcline(116);
             _v735 = px_add(_v735, px_add(_v717, px_str("}\n")));
            px_srcline(117);
             _v737 = px_add(_v737, px_int(1LL));
        }
        px_srcline(118);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(2LL)), px_null()))) {
            px_srcline(119);
             _v735 = px_add(_v735, px_add(_v717, px_str("else {\n")));
            px_srcline(120);
            _v742 = px_index(_v715, px_int(2LL));
            px_srcline(121);
            _v743 = px_int(0LL);
            px_srcline(122);
            while (px_is_truthy(px_lt(_v743, px_call(px_get_global("len"), (LXValue[]){_v742}, 1)))) {
                px_srcline(123);
                 _v735 = px_add(_v735, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v742, _v743), px_add(_v716, px_int(1LL))}, 2));
                px_srcline(124);
                 _v743 = px_add(_v743, px_int(1LL));
            }
            px_srcline(125);
             _v735 = px_add(_v735, px_add(_v717, px_str("}\n")));
        }
        px_srcline(126);
        return _v735;
    }
    px_srcline(127);
    if (px_is_truthy(px_eq(_v718, px_str("While")))) {
        px_srcline(128);
        _v738 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(129);
        _v735 = px_add(px_add(px_add(_v717, px_str("while (px_is_truthy(")), _v738), px_str(")) {\n"));
        px_srcline(130);
        _v740 = px_index(_v715, px_int(2LL));
        px_srcline(131);
        _v737 = px_int(0LL);
        px_srcline(132);
        while (px_is_truthy(px_lt(_v737, px_call(px_get_global("len"), (LXValue[]){_v740}, 1)))) {
            px_srcline(133);
             _v735 = px_add(_v735, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v740, _v737), px_add(_v716, px_int(1LL))}, 2));
            px_srcline(134);
             _v737 = px_add(_v737, px_int(1LL));
        }
        px_srcline(135);
         _v735 = px_add(_v735, px_add(_v717, px_str("}\n")));
        px_srcline(136);
        return _v735;
    }
    px_srcline(137);
    if (px_is_truthy(px_eq(_v718, px_str("For")))) {
        px_srcline(138);
        _v744 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(2LL))}, 1);
        px_srcline(139);
        _v745 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(140);
        _v746 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(142);
        _v747 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(143);
        _v748 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v747}, 1), px_int(1LL));
        px_srcline(144);
        _v749 = px_str("");
        px_srcline(145);
        _v750 = px_int(0LL);
        px_srcline(146);
        if (px_is_truthy(_v748)) {
            px_srcline(147);
             _v749 = px_call(px_get_global("cg_unpack_tmp"), (LXValue[]){}, 0);
            px_srcline(148);
             _v750 = px_call(px_get_global("len"), (LXValue[]){_v747}, 1);
        }
        px_srcline(150);
        _v751 = px_list_n((LXValue[]){}, 0);
        px_srcline(156);
        _v752 = px_int(0LL);
        px_srcline(157);
        while (px_is_truthy(px_lt(_v752, px_call(px_get_global("len"), (LXValue[]){_v747}, 1)))) {
            px_srcline(158);
            _v753 = px_index(_v747, _v752);
            px_srcline(159);
            _v754 = px_str("");
            px_srcline(160);
            if (px_is_truthy(_v748)) {
                px_srcline(162);
                 _v754 = px_add(px_add(px_add(px_add(px_str("px_index("), _v749), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v752}, 1)), px_str("))"));
            }
            else {
                px_srcline(166);
                 _v754 = px_add(px_add(px_add(px_add(px_str("px_iter_at("), _v745), px_str(", px_int(")), _v746), px_str("))"));
            }
            px_srcline(167);
            if (px_is_truthy(px_get_global("cg_topbody"))) {
                px_srcline(169);
                (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
                px_srcline(170);
                (void)(px_method(_v751, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("px_set_global(\""), _v753), px_str("\", ")), _v754), px_str(");"))}, 1));
                px_srcline(171);
                 _v752 = px_add(_v752, px_int(1LL));
                px_srcline(172);
                continue;
            }
            px_srcline(173);
            _v755 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v753}, 1);
            px_srcline(174);
            _v756 = px_str("LXValue ");
            px_srcline(175);
            if (px_is_truthy(px_eq(_v755, px_null()))) {
                px_srcline(177);
                 _v755 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v753}, 1);
            }
            else {
                px_srcline(180);
                 _v756 = px_str("");
            }
            px_srcline(181);
            if (px_is_truthy(px_eq(_v756, px_str("LXValue ")))) {
                px_srcline(183);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v753}, 1))) {
                    px_srcline(184);
                    (void)(px_method(_v751, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("LXValue "), _v755), px_str(" = px_cell(")), _v754), px_str(");"))}, 1));
                }
                else {
                    px_srcline(186);
                    (void)(px_method(_v751, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("LXValue "), _v755), px_str(" = ")), _v754), px_str(";"))}, 1));
                }
            }
            else {
                px_srcline(189);
                (void)(px_method(_v751, "append", (LXValue[]){px_add(px_call(px_get_global("cg_store_of"), (LXValue[]){_v753, _v755, _v754}, 3), px_str(";"))}, 1));
            }
            px_srcline(190);
             _v752 = px_add(_v752, px_int(1LL));
        }
        px_srcline(191);
        _v735 = px_add(px_add(px_add(px_add(px_add(_v717, px_str("LXValue ")), _v745), px_str(" = ")), _v744), px_str(";\n"));
        px_srcline(194);
        _v757 = px_call(px_get_global("cg_iter_len_tmp"), (LXValue[]){}, 0);
        px_srcline(195);
         _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_add(_v717, px_str("int ")), _v757), px_str(" = (int)px_len(")), _v745), px_str(");\n")));
        px_srcline(196);
         _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v717, px_str("for (int ")), _v746), px_str(" = 0; ")), _v746), px_str(" < ")), _v757), px_str("; ")), _v746), px_str("++) {\n")));
        px_srcline(197);
         _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_add(_v717, px_str("    px_iter_ck(")), _v745), px_str(", ")), _v757), px_str(");\n")));
        px_srcline(198);
        if (px_is_truthy(_v748)) {
            px_srcline(203);
             _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v717, px_str("    LXValue ")), _v749), px_str(" = px_iter_at(")), _v745), px_str(", px_int(")), _v746), px_str("));\n")));
            px_srcline(204);
             _v735 = px_add(_v735, px_add(px_add(px_add(px_add(px_add(_v717, px_str("    px_unpack_ck(")), _v749), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){_v750}, 1)), px_str(");\n")));
            px_srcline(205);
            _v758 = px_int(0LL);
            px_srcline(206);
            while (px_is_truthy(px_lt(_v758, px_call(px_get_global("len"), (LXValue[]){_v751}, 1)))) {
                px_srcline(207);
                 _v735 = px_add(_v735, px_add(px_add(px_add(_v717, px_str("    ")), px_index(_v751, _v758)), px_str("\n")));
                px_srcline(208);
                 _v758 = px_add(_v758, px_int(1LL));
            }
        }
        else {
            px_srcline(210);
             _v735 = px_add(_v735, px_add(px_add(px_add(_v717, px_str("    ")), px_index(_v751, px_int(0LL))), px_str("\n")));
        }
        px_srcline(211);
        _v740 = px_index(_v715, px_int(3LL));
        px_srcline(212);
        _v737 = px_int(0LL);
        px_srcline(213);
        while (px_is_truthy(px_lt(_v737, px_call(px_get_global("len"), (LXValue[]){_v740}, 1)))) {
            px_srcline(214);
             _v735 = px_add(_v735, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v740, _v737), px_add(_v716, px_int(1LL))}, 2));
            px_srcline(215);
             _v737 = px_add(_v737, px_int(1LL));
        }
        px_srcline(216);
         _v735 = px_add(_v735, px_add(_v717, px_str("}\n")));
        px_srcline(217);
        return _v735;
    }
    px_srcline(218);
    if (px_is_truthy(px_eq(_v718, px_str("Return")))) {
        px_srcline(219);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(1LL)), px_null()))) {
            px_srcline(220);
            _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
            px_srcline(221);
            return px_add(px_add(px_add(_v717, px_str("return ")), _v722), px_str(";\n"));
        }
        px_srcline(222);
        return px_add(_v717, px_str("return px_null();\n"));
    }
    px_srcline(223);
    if (px_is_truthy(px_eq(_v718, px_str("Break")))) {
        px_srcline(224);
        return px_add(_v717, px_str("break;\n"));
    }
    px_srcline(225);
    if (px_is_truthy(px_eq(_v718, px_str("Continue")))) {
        px_srcline(226);
        return px_add(_v717, px_str("continue;\n"));
    }
    px_srcline(227);
    if (px_is_truthy(px_eq(_v718, px_str("Empty")))) {
        px_srcline(228);
        return px_str("");
    }
    px_srcline(229);
    if (px_is_truthy(px_eq(_v718, px_str("ChanDecl")))) {
        px_srcline(231);
        _v759 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(232);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(233);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(234);
            return px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v759), px_str("\", px_chan_create(0));\n"));
        }
        px_srcline(235);
        _v731 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v759}, 1);
        px_srcline(236);
        return px_add(px_add(px_add(_v717, px_str("LXValue ")), _v731), px_str(" = px_chan_create(0);\n"));
    }
    px_srcline(237);
    if (px_is_truthy(px_eq(_v718, px_str("Send")))) {
        px_srcline(238);
        _v738 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(239);
        _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(2LL))}, 1);
        px_srcline(240);
        return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_chan_send(")), _v738), px_str(", ")), _v731), px_str(");\n"));
    }
    px_srcline(241);
    if (px_is_truthy(px_eq(_v718, px_str("Recv")))) {
        px_srcline(242);
        _v738 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(243);
        return px_add(px_add(px_add(_v717, px_str("px_chan_recv(")), _v738), px_str(");\n"));
    }
    px_srcline(244);
    if (px_is_truthy(px_eq(_v718, px_str("Spawn")))) {
        px_srcline(245);
        _v760 = px_index(_v715, px_int(1LL));
        px_srcline(246);
        if (px_is_truthy(px_eq(px_index(_v760, px_int(0LL)), px_str("Call")))) {
            px_srcline(247);
            _v724 = px_index(_v760, px_int(1LL));
            px_srcline(248);
            if (px_is_truthy(px_eq(px_index(_v724, px_int(0LL)), px_str("Var")))) {
                px_srcline(249);
                _v733 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v724, px_int(1LL))}, 1);
                px_srcline(250);
                _v761 = px_list_n((LXValue[]){}, 0);
                px_srcline(251);
                _v762 = px_index(_v760, px_int(2LL));
                px_srcline(252);
                _v763 = px_int(0LL);
                px_srcline(253);
                while (px_is_truthy(px_lt(_v763, px_call(px_get_global("len"), (LXValue[]){_v762}, 1)))) {
                    px_srcline(254);
                    (void)(px_method(_v761, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v762, _v763)}, 1)}, 1));
                    px_srcline(255);
                     _v763 = px_add(_v763, px_int(1LL));
                }
                px_srcline(256);
                _v728 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v762, _v761}, 2);
                px_srcline(257);
                _v729 = px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v728, px_int(0LL)), px_add(({ LXValue _s81 = px_add(px_add(px_add(px_add(px_str("px_spawn_name(\""), _v733), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v728, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s82 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v761}, 1)}, 1); px_add(_s81, _s82); }), px_str(")"))}, 2);
                px_srcline(258);
                return px_add(px_add(_v717, _v729), px_str(";\n"));
            }
            px_srcline(259);
            _v764 = px_index(_v715, px_int(2LL));
            px_srcline(260);
            (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s83 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v764, px_int(0LL))}, 1)), px_str(":")); LXValue _s84 = px_call(px_get_global("str"), (LXValue[]){px_index(_v764, px_int(1LL))}, 1); px_add(_s83, _s84); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)（方法调用请先绑定命名函数）"))}, 1));
            px_srcline(261);
            (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
        }
        px_srcline(262);
        _v765 = px_index(_v715, px_int(2LL));
        px_srcline(263);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s85 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v765, px_int(0LL))}, 1)), px_str(":")); LXValue _s86 = px_call(px_get_global("str"), (LXValue[]){px_index(_v765, px_int(1LL))}, 1); px_add(_s85, _s86); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()）"))}, 1));
        px_srcline(264);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(265);
    if (px_is_truthy(px_eq(_v718, px_str("Select")))) {
        px_srcline(266);
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v715, px_int(1LL)), px_index(_v715, px_int(2LL)), _v716}, 3);
    }
    px_srcline(267);
    if (px_is_truthy(px_eq(_v718, px_str("Import")))) {
        px_srcline(268);
        return px_add(_v717, px_str("/* import 忽略（MVP） */\n"));
    }
    px_srcline(269);
    if (px_is_truthy(px_eq(_v718, px_str("FuncDef")))) {
        px_srcline(274);
        _v766 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(275);
        _v767 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v766}, 1);
        px_srcline(278);
        _v768 = px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v715, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v715, px_int(4LL))}, 2), _v766}, 3);
        px_srcline(279);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(284);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(285);
            return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v766), px_str("\", ")), _v768), px_str(");\n"));
        }
        px_srcline(286);
        if (px_is_truthy(px_ne(_v767, px_null()))) {
            px_srcline(287);
            return px_add(px_add(_v717, px_call(px_get_global("cg_store_of"), (LXValue[]){_v766, _v767, _v768}, 3)), px_str(";\n"));
        }
        px_srcline(288);
        return px_str("");
    }
    px_srcline(290);
    return px_str("");
px_err_769:
    if (px_err_769_proped) return px_err_769_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt");
    LXValue _v770 = (nargs > 0) ? args[0] : px_null();
    LXValue _v771 = (nargs > 1) ? args[1] : px_null();
    LXValue _v772 = px_null();
    LXValue _v773 = px_null();
    LXValue _v774 = px_null();
    LXValue px_err_775_val = px_null();
    int px_err_775_proped = 0;
    px_srcline(296);
    _v772 = px_int(0LL);
    px_srcline(297);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v770}, 1), px_int(0LL)))) {
        px_srcline(298);
        _v773 = px_index(_v770, px_sub(px_call(px_get_global("len"), (LXValue[]){_v770}, 1), px_int(1LL)));
        px_srcline(299);
        if (px_is_truthy(({ LXValue _t777 = ({ LXValue _t776 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v773}, 1), px_str("list")); px_is_truthy(_t776) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v773}, 1), px_int(1LL)) : _t776; }); px_is_truthy(_t777) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v773, px_int(0LL))}, 1), px_str("int")) : _t777; }))) {
            px_srcline(300);
             _v772 = px_index(_v773, px_int(0LL));
        }
    }
    px_srcline(301);
    _v774 = px_str("");
    px_srcline(302);
    if (px_is_truthy(px_gt(_v772, px_int(0LL)))) {
        px_srcline(303);
         _v774 = px_add(({ LXValue _s87 = px_add(px_call(px_get_global("cg_pad"), (LXValue[]){_v771}, 1), px_str("px_srcline(")); LXValue _s88 = px_call(px_get_global("str"), (LXValue[]){_v772}, 1); px_add(_s87, _s88); }), px_str(");\n"));
    }
    px_srcline(304);
    return px_add(_v774, px_call(px_get_global("cg_gen_stmt_inner"), (LXValue[]){_v770, _v771}, 2));
px_err_775:
    if (px_err_775_proped) return px_err_775_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_global");
    LXValue _v778 = (nargs > 0) ? args[0] : px_null();
    LXValue _v779 = (nargs > 1) ? args[1] : px_null();
    LXValue _v780 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_781_val = px_null();
    int px_err_781_proped = 0;
    px_srcline(307);
    if (px_is_truthy(px_eq(_v778, px_str("Assign")))) {
        px_srcline(308);
        return _v780;
    }
    px_srcline(309);
    if (px_is_truthy(px_eq(_v778, px_str("Plus")))) {
        px_srcline(310);
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(311);
    if (px_is_truthy(px_eq(_v778, px_str("Minus")))) {
        px_srcline(312);
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(313);
    if (px_is_truthy(px_eq(_v778, px_str("Star")))) {
        px_srcline(314);
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(315);
    if (px_is_truthy(px_eq(_v778, px_str("Slash")))) {
        px_srcline(316);
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(317);
    if (px_is_truthy(px_eq(_v778, px_str("IntDiv")))) {
        px_srcline(318);
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(319);
    if (px_is_truthy(px_eq(_v778, px_str("Mod")))) {
        px_srcline(320);
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(321);
    if (px_is_truthy(px_eq(_v778, px_str("Pow")))) {
        px_srcline(322);
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(323);
    if (px_is_truthy(px_eq(_v778, px_str("BitAnd")))) {
        px_srcline(324);
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(325);
    if (px_is_truthy(px_eq(_v778, px_str("BitOr")))) {
        px_srcline(326);
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(327);
    if (px_is_truthy(px_eq(_v778, px_str("BitXor")))) {
        px_srcline(328);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(329);
    if (px_is_truthy(px_eq(_v778, px_str("Shl")))) {
        px_srcline(330);
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(331);
    if (px_is_truthy(px_eq(_v778, px_str("Shr")))) {
        px_srcline(332);
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(333);
    if (px_is_truthy(px_eq(_v778, px_str("ShrU")))) {
        px_srcline(334);
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v779), px_str("\"), ")), _v780), px_str(")"));
    }
    px_srcline(335);
    return _v780;
px_err_781:
    if (px_err_781_proped) return px_err_781_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_local");
    LXValue _v782 = (nargs > 0) ? args[0] : px_null();
    LXValue _v783 = (nargs > 1) ? args[1] : px_null();
    LXValue _v784 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_785_val = px_null();
    int px_err_785_proped = 0;
    px_srcline(338);
    if (px_is_truthy(px_eq(_v782, px_str("Assign")))) {
        px_srcline(339);
        return _v784;
    }
    px_srcline(340);
    if (px_is_truthy(px_eq(_v782, px_str("Plus")))) {
        px_srcline(341);
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(342);
    if (px_is_truthy(px_eq(_v782, px_str("Minus")))) {
        px_srcline(343);
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(344);
    if (px_is_truthy(px_eq(_v782, px_str("Star")))) {
        px_srcline(345);
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(346);
    if (px_is_truthy(px_eq(_v782, px_str("Slash")))) {
        px_srcline(347);
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(348);
    if (px_is_truthy(px_eq(_v782, px_str("IntDiv")))) {
        px_srcline(349);
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(350);
    if (px_is_truthy(px_eq(_v782, px_str("Mod")))) {
        px_srcline(351);
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(352);
    if (px_is_truthy(px_eq(_v782, px_str("Pow")))) {
        px_srcline(353);
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(354);
    if (px_is_truthy(px_eq(_v782, px_str("BitAnd")))) {
        px_srcline(355);
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(356);
    if (px_is_truthy(px_eq(_v782, px_str("BitOr")))) {
        px_srcline(357);
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(358);
    if (px_is_truthy(px_eq(_v782, px_str("BitXor")))) {
        px_srcline(359);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(360);
    if (px_is_truthy(px_eq(_v782, px_str("Shl")))) {
        px_srcline(361);
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(362);
    if (px_is_truthy(px_eq(_v782, px_str("Shr")))) {
        px_srcline(363);
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(364);
    if (px_is_truthy(px_eq(_v782, px_str("ShrU")))) {
        px_srcline(365);
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v783), px_str(", ")), _v784), px_str(")"));
    }
    px_srcline(366);
    return _v784;
px_err_785:
    if (px_err_785_proped) return px_err_785_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_select");
    LXValue _v786 = (nargs > 0) ? args[0] : px_null();
    LXValue _v787 = (nargs > 1) ? args[1] : px_null();
    LXValue _v788 = (nargs > 2) ? args[2] : px_null();
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
    LXValue px_err_811_val = px_null();
    int px_err_811_proped = 0;
    px_srcline(369);
    _v789 = px_call(px_get_global("cg_pad"), (LXValue[]){_v788}, 1);
    px_srcline(370);
    _v790 = px_call(px_get_global("len"), (LXValue[]){_v786}, 1);
    px_srcline(371);
    if (px_is_truthy(px_eq(_v790, px_int(0LL)))) {
        px_srcline(372);
        return px_str("select 至少需要一个 case 分支");
    }
    px_srcline(373);
    _v791 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    px_srcline(374);
    _v792 = px_str("");
    px_srcline(375);
    _v793 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(376);
    _v794 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(377);
    _v795 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(379);
    _v796 = px_list_n((LXValue[]){}, 0);
    px_srcline(380);
    _v797 = px_int(0LL);
    px_srcline(381);
    while (px_is_truthy(px_lt(_v797, _v790))) {
        px_srcline(382);
        _v798 = px_index(px_index(_v786, _v797), px_int(1LL));
        px_srcline(383);
        if (px_is_truthy(px_eq(px_index(_v798, px_int(0LL)), px_str("Call")))) {
            px_srcline(384);
            _v799 = px_index(_v798, px_int(1LL));
            px_srcline(385);
            if (px_is_truthy(px_eq(px_index(_v799, px_int(0LL)), px_str("Field")))) {
                px_srcline(386);
                _v800 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v799, px_int(2LL))}, 1);
                px_srcline(387);
                if (px_is_truthy(px_eq(_v800, px_str("recv")))) {
                    px_srcline(388);
                    (void)(px_method(_v796, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v799, px_int(1LL))}, 1)}, 1));
                    px_srcline(389);
                     _v797 = px_add(_v797, px_int(1LL));
                    px_srcline(390);
                    continue;
                }
                px_srcline(391);
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v800), px_str("）"));
            }
            px_srcline(392);
            return px_str("select case 仅支持 ch.recv()");
        }
        px_srcline(393);
        return px_str("select case 仅支持 ch.recv()");
    }
    px_srcline(394);
     _v792 = px_add(_v792, px_add(({ LXValue _s91 = px_add(({ LXValue _s89 = px_add(px_add(px_add(_v789, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str("[")); LXValue _s90 = px_call(px_get_global("str"), (LXValue[]){_v790}, 1); px_add(_s89, _s90); }), px_str("] = {")); LXValue _s92 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v796}, 2); px_add(_s91, _s92); }), px_str("};\n")));
    px_srcline(395);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(": {\n")));
    px_srcline(396);
    _v801 = px_list_n((LXValue[]){}, 0);
    px_srcline(397);
    _v802 = px_int(0LL);
    px_srcline(398);
    while (px_is_truthy(px_lt(_v802, _v790))) {
        px_srcline(399);
        (void)(px_method(_v801, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v802}, 1)}, 1));
        px_srcline(400);
         _v802 = px_add(_v802, px_int(1LL));
    }
    px_srcline(401);
     _v792 = px_add(_v792, px_add(({ LXValue _s95 = px_add(({ LXValue _s93 = px_add(px_add(px_add(_v789, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str("[")); LXValue _s94 = px_call(px_get_global("str"), (LXValue[]){_v790}, 1); px_add(_s93, _s94); }), px_str("] = {")); LXValue _s96 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v801}, 2); px_add(_s95, _s96); }), px_str("};\n")));
    px_srcline(402);
    if (px_is_truthy(px_gt(_v790, px_int(1LL)))) {
        px_srcline(403);
        _v803 = px_add(({ LXValue _s101 = px_add(({ LXValue _s99 = px_add(({ LXValue _s97 = px_add(px_add(px_add(_v789, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = ")); LXValue _s98 = px_call(px_get_global("str"), (LXValue[]){_v790}, 1); px_add(_s97, _s98); }), px_str(" - 1; _i")); LXValue _s100 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s99, _s100); }), px_str(" > 0; _i")); LXValue _s102 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s101, _s102); }), px_str("--) { "));
        px_srcline(404);
         _v803 = px_add(_v803, px_add(({ LXValue _s103 = px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = rand() % (_i")); LXValue _s104 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s103, _s104); }), px_str(" + 1); ")));
        px_srcline(405);
         _v803 = px_add(_v803, px_add(({ LXValue _s121 = px_add(({ LXValue _s119 = px_add(({ LXValue _s117 = px_add(({ LXValue _s115 = px_add(({ LXValue _s113 = px_add(({ LXValue _s111 = px_add(({ LXValue _s109 = px_add(({ LXValue _s107 = px_add(({ LXValue _s105 = px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = _ord")); LXValue _s106 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s105, _s106); }), px_str("[_i")); LXValue _s108 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s107, _s108); }), px_str("]; _ord")); LXValue _s110 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s109, _s110); }), px_str("[_i")); LXValue _s112 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s111, _s112); }), px_str("] = _ord")); LXValue _s114 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s113, _s114); }), px_str("[_j")); LXValue _s116 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s115, _s116); }), px_str("]; _ord")); LXValue _s118 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s117, _s118); }), px_str("[_j")); LXValue _s120 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s119, _s120); }), px_str("] = _t")); LXValue _s122 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s121, _s122); }), px_str("; ")));
        px_srcline(406);
         _v803 = px_add(_v803, px_str("}\n"));
        px_srcline(407);
         _v792 = px_add(_v792, _v803);
    }
    px_srcline(408);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = px_null();\n")));
    px_srcline(409);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = -1;\n")));
    px_srcline(410);
     _v792 = px_add(_v792, px_add(({ LXValue _s127 = px_add(({ LXValue _s125 = px_add(({ LXValue _s123 = px_add(px_add(px_add(_v789, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = 0; _k")); LXValue _s124 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s123, _s124); }), px_str(" < ")); LXValue _s126 = px_call(px_get_global("str"), (LXValue[]){_v790}, 1); px_add(_s125, _s126); }), px_str("; _k")); LXValue _s128 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s127, _s128); }), px_str("++) {\n")));
    px_srcline(411);
     _v792 = px_add(_v792, px_add(({ LXValue _s131 = px_add(({ LXValue _s129 = px_add(px_add(px_add(_v789, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" = _ord")); LXValue _s130 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s129, _s130); }), px_str("[_k")); LXValue _s132 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s131, _s132); }), px_str("];\n")));
    px_srcline(412);
     _v792 = px_add(_v792, px_add(({ LXValue _s139 = px_add(({ LXValue _s137 = px_add(({ LXValue _s135 = px_add(({ LXValue _s133 = px_add(px_add(px_add(_v789, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str("[_idx")); LXValue _s134 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s133, _s134); }), px_str("], &_rv")); LXValue _s136 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s135, _s136); }), px_str(")) { _picked")); LXValue _s138 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s137, _s138); }), px_str(" = _idx")); LXValue _s140 = px_call(px_get_global("str"), (LXValue[]){_v791}, 1); px_add(_s139, _s140); }), px_str("; break; }\n")));
    px_srcline(413);
     _v792 = px_add(_v792, px_add(_v789, px_str("    }\n")));
    px_srcline(415);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" >= 0) {\n")));
    px_srcline(416);
    _v804 = px_int(0LL);
    px_srcline(417);
    while (px_is_truthy(px_lt(_v804, _v790))) {
        px_srcline(418);
        _v805 = px_index(px_index(_v786, _v804), px_int(0LL));
        px_srcline(419);
        _v806 = px_index(px_index(_v786, _v804), px_int(2LL));
        px_srcline(420);
        _v807 = px_add(({ LXValue _s141 = px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" == ")); LXValue _s142 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s141, _s142); }), px_str(")"));
        px_srcline(421);
        if (px_is_truthy(px_gt(_v804, px_int(0LL)))) {
            px_srcline(422);
             _v807 = px_add(({ LXValue _s143 = px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(" == ")); LXValue _s144 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s143, _s144); }), px_str(")"));
        }
        px_srcline(423);
         _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("        ")), _v807), px_str(" {\n")));
        px_srcline(424);
        if (px_is_truthy(px_ne(_v805, px_null()))) {
            px_srcline(425);
            _v808 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v805}, 1)}, 1);
            px_srcline(426);
             _v792 = px_add(_v792, px_add(px_add(px_add(px_add(px_add(_v789, px_str("            LXValue ")), _v808), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(";\n")));
        }
        px_srcline(427);
        _v809 = px_int(0LL);
        px_srcline(428);
        while (px_is_truthy(px_lt(_v809, px_call(px_get_global("len"), (LXValue[]){_v806}, 1)))) {
            px_srcline(429);
             _v792 = px_add(_v792, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v806, _v809), px_add(_v788, px_int(3LL))}, 2));
            px_srcline(430);
             _v809 = px_add(_v809, px_int(1LL));
        }
        px_srcline(431);
         _v792 = px_add(_v792, px_add(_v789, px_str("        }\n")));
        px_srcline(432);
         _v804 = px_add(_v804, px_int(1LL));
    }
    px_srcline(433);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(";\n")));
    px_srcline(434);
     _v792 = px_add(_v792, px_add(_v789, px_str("    }\n")));
    px_srcline(436);
    if (px_is_truthy(px_ne(_v787, px_null()))) {
        px_srcline(437);
         _v792 = px_add(_v792, px_add(_v789, px_str("    {\n")));
        px_srcline(438);
        _v810 = px_int(0LL);
        px_srcline(439);
        while (px_is_truthy(px_lt(_v810, px_call(px_get_global("len"), (LXValue[]){_v787}, 1)))) {
            px_srcline(440);
             _v792 = px_add(_v792, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v787, _v810), px_add(_v788, px_int(2LL))}, 2));
            px_srcline(441);
             _v810 = px_add(_v810, px_int(1LL));
        }
        px_srcline(442);
         _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(";\n")));
        px_srcline(443);
         _v792 = px_add(_v792, px_add(_v789, px_str("    }\n")));
    }
    px_srcline(445);
     _v792 = px_add(_v792, px_add(_v789, px_str("    px_select_wait();\n")));
    px_srcline(446);
     _v792 = px_add(_v792, px_add(_v789, px_str("}\n")));
    px_srcline(447);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(";\n")));
    px_srcline(448);
     _v792 = px_add(_v792, px_add(px_add(px_add(_v789, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v791}, 1)), px_str(": ;\n")));
    px_srcline(449);
    px_set_global("cg_vars", _v793);
    px_srcline(450);
    px_set_global("cg_var_types", _v794);
    px_srcline(451);
    px_set_global("cg_cells", _v795);
    px_srcline(452);
    return _v792;
px_err_811:
    if (px_err_811_proped) return px_err_811_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_collect");
    LXValue _v812 = (nargs > 0) ? args[0] : px_null();
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
    LXValue px_err_823_val = px_null();
    int px_err_823_proped = 0;
    px_srcline(9);
    _v813 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(10);
    (void)(px_method(_v813, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(11);
    px_index_set(_v813, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_srcline(12);
    px_index_set(_v813, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_srcline(13);
    px_index_set(_v813, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_srcline(14);
    px_index_set(_v813, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_srcline(15);
    px_index_set(_v813, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_srcline(16);
    px_index_set(_v813, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    px_srcline(17);
    _v814 = px_int(0LL);
    px_srcline(18);
    while (px_is_truthy(px_lt(_v814, px_call(px_get_global("len"), (LXValue[]){_v812}, 1)))) {
        px_srcline(19);
        _v815 = px_index(_v812, _v814);
        px_srcline(20);
        _v816 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v815, px_int(2LL))}, 1);
        px_srcline(21);
        (void)(px_method(px_index(_v813, px_str("its")), "append", (LXValue[]){_v816}, 1));
        px_srcline(22);
        (void)(px_method(px_index(_v813, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(23);
        (void)(px_method(px_index(_v813, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(24);
        (void)(px_method(px_index(_v813, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(25);
        _v817 = px_str("");
        px_srcline(26);
        _v818 = px_list_n((LXValue[]){}, 0);
        px_srcline(27);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v815, px_int(1LL))}, 1), px_int(1LL)))) {
            px_srcline(28);
            _v819 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            px_srcline(29);
            _v820 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v815, px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(30);
            _v821 = px_null();
            px_srcline(31);
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v820}, 1))) {
                px_srcline(32);
                 _v821 = px_index(px_get_global("cg_vars"), _v820);
            }
            px_srcline(33);
            px_index_set(px_get_global("cg_vars"), _v820, _v819);
            px_srcline(34);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v820}, 1))) {
                px_srcline(35);
                (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v820}, 1));
            }
            px_srcline(36);
            (void)(px_method(_v818, "append", (LXValue[]){px_list_n((LXValue[]){_v820, _v821}, 2)}, 1));
            px_srcline(37);
             _v817 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v819), px_str(" = ")), px_index(px_index(_v813, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v813, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            px_srcline(39);
            _v822 = px_int(0LL);
            px_srcline(44);
             _v817 = px_add(({ LXValue _s145 = px_add(px_add(px_str("px_unpack_ck("), px_index(px_index(_v813, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v813, px_str("itms"))}, 1), px_int(1LL)))), px_str(", ")); LXValue _s146 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v815, px_int(1LL))}, 1)}, 1); px_add(_s145, _s146); }), px_str("); "));
            px_srcline(45);
            while (px_is_truthy(px_lt(_v822, px_call(px_get_global("len"), (LXValue[]){px_index(_v815, px_int(1LL))}, 1)))) {
                px_srcline(46);
                _v820 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v815, px_int(1LL)), _v822)}, 1);
                px_srcline(47);
                _v819 = ({ LXValue _s147 = px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")); LXValue _s148 = px_call(px_get_global("str"), (LXValue[]){_v822}, 1); px_add(_s147, _s148); });
                px_srcline(48);
                _v821 = px_null();
                px_srcline(49);
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v820}, 1))) {
                    px_srcline(50);
                     _v821 = px_index(px_get_global("cg_vars"), _v820);
                }
                px_srcline(51);
                px_index_set(px_get_global("cg_vars"), _v820, _v819);
                px_srcline(52);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v820}, 1))) {
                    px_srcline(53);
                    (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v820}, 1));
                }
                px_srcline(54);
                (void)(px_method(_v818, "append", (LXValue[]){px_list_n((LXValue[]){_v820, _v821}, 2)}, 1));
                px_srcline(55);
                 _v817 = px_add(_v817, px_add(({ LXValue _s149 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v819), px_str(" = px_index(")), px_index(px_index(_v813, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v813, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")); LXValue _s150 = px_call(px_get_global("str"), (LXValue[]){_v822}, 1); px_add(_s149, _s150); }), px_str(")); ")));
                px_srcline(56);
                 _v822 = px_add(_v822, px_int(1LL));
            }
        }
        px_srcline(57);
        (void)(px_method(px_index(_v813, px_str("binds")), "append", (LXValue[]){_v817}, 1));
        px_srcline(58);
        (void)(px_method(px_index(_v813, px_str("saved_all")), "append", (LXValue[]){_v818}, 1));
        px_srcline(59);
         _v814 = px_add(_v814, px_int(1LL));
    }
    px_srcline(60);
    return _v813;
px_err_823:
    if (px_err_823_proped) return px_err_823_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_restore");
    LXValue _v824 = (nargs > 0) ? args[0] : px_null();
    LXValue _v825 = px_null();
    LXValue _v826 = px_null();
    LXValue _v827 = px_null();
    LXValue _v828 = px_null();
    LXValue _v829 = px_null();
    LXValue px_err_830_val = px_null();
    int px_err_830_proped = 0;
    px_srcline(62);
    _v825 = px_int(0LL);
    px_srcline(63);
    while (px_is_truthy(px_lt(_v825, px_call(px_get_global("len"), (LXValue[]){_v824}, 1)))) {
        px_srcline(64);
        _v826 = px_index(_v824, _v825);
        px_srcline(65);
        _v827 = px_int(0LL);
        px_srcline(66);
        while (px_is_truthy(px_lt(_v827, px_call(px_get_global("len"), (LXValue[]){_v826}, 1)))) {
            px_srcline(67);
            _v828 = px_index(px_index(_v826, _v827), px_int(0LL));
            px_srcline(68);
            _v829 = px_index(px_index(_v826, _v827), px_int(1LL));
            px_srcline(69);
            if (px_is_truthy(px_eq(_v829, px_null()))) {
                px_srcline(70);
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v828}, 1));
            }
            else {
                px_srcline(72);
                px_index_set(px_get_global("cg_vars"), _v828, _v829);
            }
            px_srcline(73);
             _v827 = px_add(_v827, px_int(1LL));
        }
        px_srcline(74);
         _v825 = px_add(_v825, px_int(1LL));
    }
px_err_830:
    if (px_err_830_proped) return px_err_830_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_body");
    LXValue _v831 = (nargs > 0) ? args[0] : px_null();
    LXValue _v832 = (nargs > 1) ? args[1] : px_null();
    LXValue _v833 = (nargs > 2) ? args[2] : px_null();
    LXValue _v834 = px_null();
    LXValue _v835 = px_null();
    LXValue _v836 = px_null();
    LXValue _v837 = px_null();
    LXValue _v838 = px_null();
    LXValue px_err_839_val = px_null();
    int px_err_839_proped = 0;
    px_srcline(77);
    _v834 = px_str("");
    px_srcline(78);
    if (px_is_truthy(px_ne(_v832, px_null()))) {
        px_srcline(79);
         _v834 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v832), px_str(")) { ")), _v833), px_str("} "));
    }
    else {
        px_srcline(81);
         _v834 = _v833;
    }
    px_srcline(82);
    _v835 = px_call(px_get_global("len"), (LXValue[]){px_index(_v831, px_str("its"))}, 1);
    px_srcline(83);
    _v836 = px_sub(_v835, px_int(1LL));
    px_srcline(84);
    while (px_is_truthy(px_ge(_v836, px_int(0LL)))) {
        px_srcline(85);
        _v837 = px_str("");
        px_srcline(86);
        if (px_is_truthy(px_lt(px_add(_v836, px_int(1LL)), _v835))) {
            px_srcline(87);
             _v837 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v831, px_str("ivs")), px_add(_v836, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v831, px_str("its")), px_add(_v836, px_int(1LL)))), px_str("; "));
        }
        px_srcline(92);
        _v838 = px_call(px_get_global("cg_iter_len_tmp"), (LXValue[]){}, 0);
        px_srcline(93);
         _v834 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), _v838), px_str(" = (int)px_len(")), px_index(px_index(_v831, px_str("ivs")), _v836)), px_str("), ")), px_index(px_index(_v831, px_str("idxs")), _v836)), px_str("=0; ")), px_index(px_index(_v831, px_str("idxs")), _v836)), px_str("<")), _v838), px_str("; ")), px_index(px_index(_v831, px_str("idxs")), _v836)), px_str("++) { px_iter_ck(")), px_index(px_index(_v831, px_str("ivs")), _v836)), px_str(", ")), _v838), px_str("); LXValue ")), px_index(px_index(_v831, px_str("itms")), _v836)), px_str(" = px_iter_at(")), px_index(px_index(_v831, px_str("ivs")), _v836)), px_str(", px_int(")), px_index(px_index(_v831, px_str("idxs")), _v836)), px_str(")); ")), px_index(px_index(_v831, px_str("binds")), _v836)), _v837), _v834), px_str(" } "));
        px_srcline(94);
         _v836 = px_sub(_v836, px_int(1LL));
    }
    px_srcline(95);
    return _v834;
px_err_839:
    if (px_err_839_proped) return px_err_839_val;
    return px_null();
}

static LXValue fn_cg_gen_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_closure");
    LXValue _v840 = (nargs > 0) ? args[0] : px_null();
    LXValue _v841 = (nargs > 1) ? args[1] : px_null();
    LXValue _v842 = (nargs > 2) ? args[2] : px_null();
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
    LXValue px_err_868_val = px_null();
    int px_err_868_proped = 0;
    px_srcline(102);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(103);
    _v843 = px_get_global("cg_closure_id");
    px_srcline(104);
    _v844 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v843}, 1));
    px_srcline(106);
    _v845 = px_list_n((LXValue[]){}, 0);
    px_srcline(107);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v840, px_null(), _v841, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v845}, 2));
    px_srcline(108);
    _v846 = px_list_n((LXValue[]){}, 0);
    px_srcline(109);
    LXValue _t869 = _v845;
    int _il1 = (int)px_len(_t869);
    for (int _t870 = 0; _t870 < _il1; _t870++) {
        px_iter_ck(_t869, _il1);
        _v847 = px_iter_at(_t869, px_int(_t870));
        px_srcline(110);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v847}, 1), px_null()))) {
            px_srcline(111);
            (void)(px_method(_v846, "append", (LXValue[]){_v847}, 1));
        }
    }
    px_srcline(113);
    _v848 = px_str("px_null()");
    px_srcline(114);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v846}, 1), px_int(0LL)))) {
        px_srcline(115);
        _v849 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(116);
        LXValue _t871 = _v846;
        int _il2 = (int)px_len(_t871);
        for (int _t872 = 0; _t872 < _il2; _t872++) {
            px_iter_ck(_t871, _il2);
            _v847 = px_iter_at(_t871, px_int(_t872));
            px_srcline(117);
            _v850 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v847}, 1);
            px_srcline(120);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v847}, 1))) {
                px_srcline(121);
                 _v849 = px_add(_v849, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v847), px_str("\", ")), _v850), px_str("); ")));
            }
            else {
                px_srcline(123);
                 _v849 = px_add(_v849, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v847), px_str("\", px_cell(")), _v850), px_str(")); ")));
            }
        }
        px_srcline(124);
         _v849 = px_add(_v849, px_str("_capenv; })"));
        px_srcline(125);
         _v848 = _v849;
    }
    px_srcline(126);
    _v851 = px_add(px_add(px_str("static LXValue "), _v844), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(127);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v846}, 1), px_int(0LL)))) {
        px_srcline(128);
         _v851 = px_add(_v851, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(130);
         _v851 = px_add(_v851, px_str("    (void)ctx;\n"));
    }
    px_srcline(131);
    _v852 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(132);
    _v853 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(133);
    _v854 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(135);
    _v855 = px_get_global("cg_topbody");
    px_srcline(136);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(137);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(138);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(139);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(140);
    _v856 = px_int(0LL);
    px_srcline(141);
    while (px_is_truthy(px_lt(_v856, px_call(px_get_global("len"), (LXValue[]){_v840}, 1)))) {
        px_srcline(142);
        _v857 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v840, _v856), px_int(1LL))}, 1)}, 1);
        px_srcline(143);
         _v851 = px_add(_v851, px_add(({ LXValue _s151 = px_add(px_add(px_add(px_add(px_str("    LXValue "), _v857), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v856}, 1)), px_str(") ? args[")); LXValue _s152 = px_call(px_get_global("str"), (LXValue[]){_v856}, 1); px_add(_s151, _s152); }), px_str("] : px_null();\n")));
        px_srcline(144);
         _v856 = px_add(_v856, px_int(1LL));
    }
    px_srcline(146);
    LXValue _t873 = _v846;
    int _il3 = (int)px_len(_t873);
    for (int _t874 = 0; _t874 < _il3; _t874++) {
        px_iter_ck(_t873, _il3);
        _v847 = px_iter_at(_t873, px_int(_t874));
        px_srcline(147);
        _v858 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v847}, 1);
        px_srcline(148);
        px_index_set(px_get_global("cg_cells"), _v847, px_int(1LL));
        px_srcline(149);
         _v851 = px_add(_v851, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v858), px_str(" = px_env_lookup(ctx, \"")), _v847), px_str("\");\n")));
    }
    px_srcline(159);
    _v859 = px_list_n((LXValue[]){}, 0);
    px_srcline(160);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v841, _v859}, 2));
    px_srcline(161);
    _v860 = px_list_n((LXValue[]){}, 0);
    px_srcline(162);
    _v861 = px_list_n((LXValue[]){}, 0);
    px_srcline(163);
    if (px_is_truthy(({ LXValue _t876 = ({ LXValue _t875 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v841}, 1), px_str("list")); px_is_truthy(_t875) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v841}, 1), px_int(0LL)) : _t875; }); px_is_truthy(_t876) ? px_eq(px_index(_v841, px_int(0LL)), px_str("Block")) : _t876; }))) {
        px_srcline(164);
        (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v841, px_int(1LL)), _v860}, 2));
        px_srcline(165);
        (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v841, px_int(1LL)), _v861}, 2));
    }
    px_srcline(166);
    _v862 = px_list_n((LXValue[]){}, 0);
    px_srcline(167);
    _v863 = px_int(0LL);
    px_srcline(168);
    while (px_is_truthy(px_lt(_v863, px_call(px_get_global("len"), (LXValue[]){_v860}, 1)))) {
        px_srcline(169);
        _v864 = px_index(_v860, _v863);
        px_srcline(171);
        if (px_is_truthy(({ LXValue _t878 = px_eq(px_call(px_get_global("cg_var_of"), (LXValue[]){_v864}, 1), px_null()); px_is_truthy(_t878) ? px_not(({ LXValue _t877 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v861, _v864}, 2)); px_is_truthy(_t877) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_topnames"), _v864}, 2) : _t877; })) : _t878; }))) {
            px_srcline(172);
            _v865 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v864}, 1);
            px_srcline(173);
            if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v859, _v864}, 2))) {
                px_srcline(174);
                px_index_set(px_get_global("cg_cells"), _v864, px_int(1LL));
            }
            px_srcline(175);
            (void)(px_method(_v862, "append", (LXValue[]){px_list_n((LXValue[]){_v865, _v864}, 2)}, 1));
        }
        px_srcline(176);
         _v863 = px_add(_v863, px_int(1LL));
    }
    px_srcline(177);
    _v866 = px_int(0LL);
    px_srcline(178);
    while (px_is_truthy(px_lt(_v866, px_call(px_get_global("len"), (LXValue[]){_v862}, 1)))) {
        px_srcline(179);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(px_index(_v862, _v866), px_int(1LL))}, 1))) {
            px_srcline(180);
             _v851 = px_add(_v851, px_add(px_add(px_str("    LXValue "), px_index(px_index(_v862, _v866), px_int(0LL))), px_str(" = px_cell(px_null());\n")));
        }
        else {
            px_srcline(182);
             _v851 = px_add(_v851, px_add(px_add(px_str("    LXValue "), px_index(px_index(_v862, _v866), px_int(0LL))), px_str(" = px_null();\n")));
        }
        px_srcline(183);
         _v866 = px_add(_v866, px_int(1LL));
    }
    px_srcline(184);
    _v867 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v841}, 1);
    px_srcline(185);
     _v851 = px_add(_v851, px_add(px_add(px_str("    return "), _v867), px_str(";\n")));
    px_srcline(186);
     _v851 = px_add(_v851, px_str("}\n"));
    px_srcline(187);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v851));
    px_srcline(188);
    px_set_global("cg_vars", _v852);
    px_srcline(189);
    px_set_global("cg_var_types", _v853);
    px_srcline(190);
    px_set_global("cg_cells", _v854);
    px_srcline(191);
    px_set_global("cg_topbody", _v855);
    px_srcline(195);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v846}, 1), px_int(0LL)))) {
        px_srcline(196);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\""), _v842), px_str("\", ")), _v844), px_str(", ")), _v848), px_str(")"));
    }
    px_srcline(197);
    return px_add(px_add(px_add(px_add(px_str("px_func(\""), _v842), px_str("\", ")), _v844), px_str(", NULL)"));
px_err_868:
    if (px_err_868_proped) return px_err_868_val;
    return px_null();
}

static LXValue fn_cg_side_effect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_side_effect");
    LXValue _v879 = (nargs > 0) ? args[0] : px_null();
    LXValue _v880 = px_null();
    LXValue _v881 = px_null();
    LXValue px_err_882_val = px_null();
    int px_err_882_proped = 0;
    px_srcline(211);
    if (px_is_truthy(({ LXValue _t883 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v879}, 1), px_str("list")); px_is_truthy(_t883) ? _t883 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v879}, 1), px_int(0LL)); }))) {
        px_srcline(212);
        return px_bool(false);
    }
    px_srcline(213);
    _v880 = px_index(_v879, px_int(0LL));
    px_srcline(214);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v880}, 1), px_str("string")))) {
        px_srcline(215);
        if (px_is_truthy(({ LXValue _t887 = ({ LXValue _t886 = ({ LXValue _t885 = ({ LXValue _t884 = px_eq(_v880, px_str("Call")); px_is_truthy(_t884) ? _t884 : px_eq(_v880, px_str("Pipe")); }); px_is_truthy(_t885) ? _t885 : px_eq(_v880, px_str("ListComp")); }); px_is_truthy(_t886) ? _t886 : px_eq(_v880, px_str("DictComp")); }); px_is_truthy(_t887) ? _t887 : px_eq(_v880, px_str("GenExp")); }))) {
            px_srcline(216);
            return px_bool(true);
        }
        px_srcline(219);
        if (px_is_truthy(px_eq(_v880, px_str("Closure")))) {
            px_srcline(220);
            return px_bool(false);
        }
    }
    px_srcline(221);
    _v881 = px_int(0LL);
    px_srcline(222);
    while (px_is_truthy(px_lt(_v881, px_call(px_get_global("len"), (LXValue[]){_v879}, 1)))) {
        px_srcline(223);
        if (px_is_truthy(px_call(px_get_global("cg_side_effect"), (LXValue[]){px_index(_v879, _v881)}, 1))) {
            px_srcline(224);
            return px_bool(true);
        }
        px_srcline(225);
         _v881 = px_add(_v881, px_int(1LL));
    }
    px_srcline(226);
    return px_bool(false);
px_err_882:
    if (px_err_882_proped) return px_err_882_val;
    return px_null();
}

static LXValue fn_cg_seq_operands(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_operands");
    LXValue _v888 = (nargs > 0) ? args[0] : px_null();
    LXValue _v889 = (nargs > 1) ? args[1] : px_null();
    LXValue _v890 = px_null();
    LXValue _v891 = px_null();
    LXValue _v892 = px_null();
    LXValue _v893 = px_null();
    LXValue _v894 = px_null();
    LXValue px_err_895_val = px_null();
    int px_err_895_proped = 0;
    px_srcline(229);
    _v890 = px_int(0LL);
    px_srcline(230);
    _v891 = px_int(0LL);
    px_srcline(231);
    while (px_is_truthy(px_lt(_v891, px_call(px_get_global("len"), (LXValue[]){_v889}, 1)))) {
        px_srcline(232);
        if (px_is_truthy(px_index(_v889, _v891))) {
            px_srcline(233);
             _v890 = px_add(_v890, px_int(1LL));
        }
        px_srcline(234);
         _v891 = px_add(_v891, px_int(1LL));
    }
    px_srcline(235);
    if (px_is_truthy(px_lt(_v890, px_int(2LL)))) {
        px_srcline(236);
        return px_list_n((LXValue[]){px_null(), px_null()}, 2);
    }
    px_srcline(237);
    _v892 = px_str("");
    px_srcline(238);
    _v893 = px_list_n((LXValue[]){}, 0);
    px_srcline(239);
     _v891 = px_int(0LL);
    px_srcline(240);
    while (px_is_truthy(px_lt(_v891, px_call(px_get_global("len"), (LXValue[]){_v888}, 1)))) {
        px_srcline(241);
        if (px_is_truthy(px_index(_v889, _v891))) {
            px_srcline(245);
            px_set_global("cg_seq_uid", px_add(px_get_global("cg_seq_uid"), px_int(1LL)));
            px_srcline(246);
            _v894 = px_add(px_str("_s"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_seq_uid")}, 1));
            px_srcline(247);
             _v892 = px_add(px_add(px_add(px_add(px_add(_v892, px_str("LXValue ")), _v894), px_str(" = ")), px_index(_v888, _v891)), px_str("; "));
            px_srcline(248);
            (void)(px_method(_v893, "append", (LXValue[]){_v894}, 1));
        }
        else {
            px_srcline(250);
            (void)(px_method(_v893, "append", (LXValue[]){px_index(_v888, _v891)}, 1));
        }
        px_srcline(251);
         _v891 = px_add(_v891, px_int(1LL));
    }
    px_srcline(252);
    return px_list_n((LXValue[]){_v892, _v893}, 2);
px_err_895:
    if (px_err_895_proped) return px_err_895_val;
    return px_null();
}

static LXValue fn_cg_seq_join(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_join");
    LXValue _v896 = (nargs > 0) ? args[0] : px_null();
    LXValue _v897 = (nargs > 1) ? args[1] : px_null();
    LXValue _v898 = px_null();
    LXValue _v899 = px_null();
    LXValue _v900 = px_null();
    LXValue px_err_901_val = px_null();
    int px_err_901_proped = 0;
    px_srcline(259);
    _v898 = px_list_n((LXValue[]){}, 0);
    px_srcline(260);
    _v899 = px_int(0LL);
    px_srcline(261);
    while (px_is_truthy(px_lt(_v899, px_call(px_get_global("len"), (LXValue[]){_v896}, 1)))) {
        px_srcline(262);
        (void)(px_method(_v898, "append", (LXValue[]){px_call(px_get_global("cg_side_effect"), (LXValue[]){px_index(_v896, _v899)}, 1)}, 1));
        px_srcline(263);
         _v899 = px_add(_v899, px_int(1LL));
    }
    px_srcline(264);
    _v900 = px_call(px_get_global("cg_seq_operands"), (LXValue[]){_v897, _v898}, 2);
    px_srcline(265);
    if (px_is_truthy(px_eq(px_index(_v900, px_int(0LL)), px_null()))) {
        px_srcline(266);
        return px_list_n((LXValue[]){px_null(), _v897}, 2);
    }
    px_srcline(267);
    return px_list_n((LXValue[]){px_index(_v900, px_int(0LL)), px_index(_v900, px_int(1LL))}, 2);
px_err_901:
    if (px_err_901_proped) return px_err_901_val;
    return px_null();
}

static LXValue fn_cg_seq_wrap(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_wrap");
    LXValue _v902 = (nargs > 0) ? args[0] : px_null();
    LXValue _v903 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_904_val = px_null();
    int px_err_904_proped = 0;
    px_srcline(270);
    if (px_is_truthy(px_eq(_v902, px_null()))) {
        px_srcline(271);
        return _v903;
    }
    px_srcline(272);
    return px_add(px_add(px_add(px_str("({ "), _v902), _v903), px_str("; })"));
px_err_904:
    if (px_err_904_proped) return px_err_904_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_expr");
    LXValue _v905 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v949 = px_null();
    LXValue _v950 = px_null();
    LXValue _v951 = px_null();
    LXValue _v952 = px_null();
    LXValue _v953 = px_null();
    LXValue _v954 = px_null();
    LXValue _v955 = px_null();
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
    LXValue px_err_967_val = px_null();
    int px_err_967_proped = 0;
    px_srcline(274);
    _v906 = px_index(_v905, px_int(0LL));
    px_srcline(275);
    if (px_is_truthy(px_eq(_v906, px_str("Int")))) {
        px_srcline(276);
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1)), px_str("LL)"));
    }
    px_srcline(277);
    if (px_is_truthy(px_eq(_v906, px_str("Float")))) {
        px_srcline(278);
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1)), px_str(")"));
    }
    px_srcline(279);
    if (px_is_truthy(px_eq(_v906, px_str("Str")))) {
        px_srcline(280);
        _v907 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(284);
        if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){_v907}, 1))) {
            px_srcline(285);
            return px_add(px_add(px_str("PX_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v907}, 1)), px_str("\")"));
        }
        px_srcline(286);
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v907}, 1)), px_str("\")"));
    }
    px_srcline(287);
    if (px_is_truthy(px_eq(_v906, px_str("Bool")))) {
        px_srcline(288);
        if (px_is_truthy(px_index(_v905, px_int(1LL)))) {
            px_srcline(289);
            return px_str("px_bool(true)");
        }
        px_srcline(290);
        return px_str("px_bool(false)");
    }
    px_srcline(291);
    if (px_is_truthy(px_eq(_v906, px_str("Null")))) {
        px_srcline(292);
        return px_str("px_null()");
    }
    px_srcline(293);
    if (px_is_truthy(px_eq(_v906, px_str("List")))) {
        px_srcline(294);
        _v908 = px_list_n((LXValue[]){}, 0);
        px_srcline(295);
        _v909 = px_index(_v905, px_int(1LL));
        px_srcline(296);
        _v910 = px_int(0LL);
        px_srcline(297);
        while (px_is_truthy(px_lt(_v910, px_call(px_get_global("len"), (LXValue[]){_v909}, 1)))) {
            px_srcline(298);
            (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v909, _v910)}, 1)}, 1));
            px_srcline(299);
             _v910 = px_add(_v910, px_int(1LL));
        }
        px_srcline(300);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v909, _v908}, 2);
        px_srcline(301);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s153 = px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s154 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v908}, 1)}, 1); px_add(_s153, _s154); }), px_str(")"))}, 2);
    }
    px_srcline(302);
    if (px_is_truthy(px_eq(_v906, px_str("Tuple")))) {
        px_srcline(303);
        _v908 = px_list_n((LXValue[]){}, 0);
        px_srcline(304);
        _v909 = px_index(_v905, px_int(1LL));
        px_srcline(305);
        _v910 = px_int(0LL);
        px_srcline(306);
        while (px_is_truthy(px_lt(_v910, px_call(px_get_global("len"), (LXValue[]){_v909}, 1)))) {
            px_srcline(307);
            (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v909, _v910)}, 1)}, 1));
            px_srcline(308);
             _v910 = px_add(_v910, px_int(1LL));
        }
        px_srcline(309);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v909, _v908}, 2);
        px_srcline(310);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s155 = px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s156 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v908}, 1)}, 1); px_add(_s155, _s156); }), px_str(")"))}, 2);
    }
    px_srcline(311);
    if (px_is_truthy(px_eq(_v906, px_str("Dict")))) {
        px_srcline(312);
        _v912 = px_str("({ LXValue _d = px_dict(); ");
        px_srcline(313);
        _v913 = px_index(_v905, px_int(1LL));
        px_srcline(314);
        _v910 = px_int(0LL);
        px_srcline(315);
        while (px_is_truthy(px_lt(_v910, px_call(px_get_global("len"), (LXValue[]){_v913}, 1)))) {
            px_srcline(316);
            _v914 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v913, _v910), px_int(0LL))}, 1);
            px_srcline(317);
            _v915 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v913, _v910), px_int(1LL))}, 1);
            px_srcline(322);
             _v912 = px_add(_v912, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v914), px_str("; LXValue _v = ")), _v915), px_str("; px_dict_set_checked(_d, _k, _v); } ")));
            px_srcline(323);
             _v910 = px_add(_v910, px_int(1LL));
        }
        px_srcline(324);
         _v912 = px_add(_v912, px_str("_d; })"));
        px_srcline(325);
        return _v912;
    }
    px_srcline(326);
    if (px_is_truthy(px_eq(_v906, px_str("Var")))) {
        px_srcline(327);
        _v916 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(328);
        _v917 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v916}, 1);
        px_srcline(329);
        if (px_is_truthy(px_ne(_v917, px_null()))) {
            px_srcline(331);
            return px_call(px_get_global("cg_load_of"), (LXValue[]){_v916, _v917}, 2);
        }
        px_srcline(332);
        return px_add(px_add(px_str("px_get_global(\""), _v916), px_str("\")"));
    }
    px_srcline(333);
    if (px_is_truthy(px_eq(_v906, px_str("Field")))) {
        px_srcline(334);
        _v918 = px_index(_v905, px_int(1LL));
        px_srcline(335);
        _v919 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(337);
        if (px_is_truthy(px_eq(px_index(_v918, px_int(0LL)), px_str("Var")))) {
            px_srcline(338);
            _v920 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v918, px_int(1LL))}, 1);
            px_srcline(339);
            if (px_is_truthy(({ LXValue _t968 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v920}, 1); px_is_truthy(_t968) ? px_method(px_index(px_get_global("cg_const_enums"), _v920), "has", (LXValue[]){_v919}, 1) : _t968; }))) {
                px_srcline(340);
                return px_index(px_index(px_get_global("cg_const_enums"), _v920), _v919);
            }
        }
        px_srcline(342);
        if (px_is_truthy(px_eq(px_index(_v918, px_int(0LL)), px_str("Var")))) {
            px_srcline(343);
            _v920 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v918, px_int(1LL))}, 1);
            px_srcline(344);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v920}, 1))) {
                px_srcline(345);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v920), px_str("\", \"")), _v919), px_str("\")"));
            }
        }
        px_srcline(346);
        _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v918}, 1);
        px_srcline(347);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v921), px_str(", \"")), _v919), px_str("\")"));
    }
    px_srcline(348);
    if (px_is_truthy(px_eq(_v906, px_str("OptionalField")))) {
        px_srcline(349);
        _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(350);
        _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(351);
        _v919 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(352);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v921), px_str("; px_is_null(")), _v922), px_str(") ? px_null() : px_field(")), _v921), px_str(", \"")), _v919), px_str("\"); })"));
    }
    px_srcline(353);
    if (px_is_truthy(px_eq(_v906, px_str("Index")))) {
        px_srcline(354);
        _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(355);
        _v910 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(356);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v905, px_int(1LL)), px_index(_v905, px_int(2LL))}, 2), px_list_n((LXValue[]){_v921, _v910}, 2)}, 2);
        px_srcline(357);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_index("), px_index(px_index(_v911, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v911, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2);
    }
    px_srcline(358);
    if (px_is_truthy(px_eq(_v906, px_str("Slice")))) {
        px_srcline(359);
        _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(360);
        _v912 = px_str("px_null()");
        px_srcline(361);
        if (px_is_truthy(px_ne(px_index(_v905, px_int(2LL)), px_null()))) {
            px_srcline(362);
             _v912 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        }
        px_srcline(363);
        _v923 = px_str("px_null()");
        px_srcline(364);
        if (px_is_truthy(px_ne(px_index(_v905, px_int(3LL)), px_null()))) {
            px_srcline(365);
             _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(3LL))}, 1);
        }
        px_srcline(366);
        _v924 = px_str("px_null()");
        px_srcline(367);
        if (px_is_truthy(px_ne(px_index(_v905, px_int(4LL)), px_null()))) {
            px_srcline(368);
             _v924 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(4LL))}, 1);
        }
        px_srcline(369);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v905, px_int(1LL)), px_index(_v905, px_int(2LL)), px_index(_v905, px_int(3LL)), px_index(_v905, px_int(4LL))}, 4), px_list_n((LXValue[]){_v921, _v912, _v923, _v924}, 4)}, 2);
        px_srcline(370);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), px_index(px_index(_v911, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v911, px_int(1LL)), px_int(1LL))), px_str(", ")), px_index(px_index(_v911, px_int(1LL)), px_int(2LL))), px_str(", ")), px_index(px_index(_v911, px_int(1LL)), px_int(3LL))), px_str(")"))}, 2);
    }
    px_srcline(371);
    if (px_is_truthy(px_eq(_v906, px_str("Call")))) {
        px_srcline(372);
        _v925 = px_index(_v905, px_int(1LL));
        px_srcline(373);
        _v926 = px_index(_v905, px_int(2LL));
        px_srcline(374);
        if (px_is_truthy(px_eq(px_index(_v925, px_int(0LL)), px_str("Var")))) {
            px_srcline(375);
            _v927 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v925, px_int(1LL))}, 1);
            px_srcline(378);
            (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v925, _v926}, 2));
            px_srcline(379);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v927}, 1))) {
                px_srcline(380);
                _v928 = px_index(px_get_global("cg_ffi"), _v927);
                px_srcline(381);
                _v908 = px_list_n((LXValue[]){}, 0);
                px_srcline(382);
                _v929 = px_int(0LL);
                px_srcline(383);
                while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
                    px_srcline(384);
                    (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
                    px_srcline(385);
                     _v929 = px_add(_v929, px_int(1LL));
                }
                px_srcline(386);
                _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v926, _v908}, 2);
                px_srcline(387);
                return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s157 = px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v927), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s158 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v908}, 1)}, 1); px_add(_s157, _s158); }), px_str(")}, 2)"))}, 2);
            }
            px_srcline(388);
            if (px_is_truthy(px_eq(_v927, px_str("chan")))) {
                px_srcline(389);
                _v930 = px_str("0");
                px_srcline(390);
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v926}, 1), px_int(0LL)))) {
                    px_srcline(391);
                    if (px_is_truthy(px_eq(px_index(px_index(_v926, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                        px_srcline(392);
                         _v930 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v926, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                        px_srcline(394);
                         _v930 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                px_srcline(395);
                return px_add(px_add(px_str("px_chan_create("), _v930), px_str(")"));
            }
            px_srcline(396);
            if (px_is_truthy(px_eq(_v927, px_str("mutex")))) {
                px_srcline(397);
                return px_str("px_mutex_create()");
            }
            px_srcline(398);
            if (px_is_truthy(px_eq(_v927, px_str("rwlock")))) {
                px_srcline(399);
                return px_str("px_rwlock_create()");
            }
            px_srcline(401);
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v927}, 1))) {
                px_srcline(402);
                _v931 = px_index(px_get_global("cg_structs"), _v927);
                px_srcline(403);
                if (px_is_truthy(({ LXValue _s159 = px_call(px_get_global("len"), (LXValue[]){_v931}, 1); LXValue _s160 = px_call(px_get_global("len"), (LXValue[]){_v926}, 1); px_ne(_s159, _s160); }))) {
                    px_srcline(404);
                    return ({ LXValue _s161 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v927), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v931}, 1)}, 1)), px_str(" 个字段，给出 ")); LXValue _s162 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v926}, 1)}, 1); px_add(_s161, _s162); });
                }
                px_srcline(405);
                _v908 = px_list_n((LXValue[]){}, 0);
                px_srcline(406);
                _v929 = px_int(0LL);
                px_srcline(407);
                while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
                    px_srcline(408);
                    (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
                    px_srcline(409);
                     _v929 = px_add(_v929, px_int(1LL));
                }
                px_srcline(410);
                _v932 = px_list_n((LXValue[]){}, 0);
                px_srcline(411);
                _v933 = px_int(0LL);
                px_srcline(412);
                while (px_is_truthy(px_lt(_v933, px_call(px_get_global("len"), (LXValue[]){_v931}, 1)))) {
                    px_srcline(413);
                    (void)(px_method(_v932, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v931, _v933)), px_str("\""))}, 1));
                    px_srcline(414);
                     _v933 = px_add(_v933, px_int(1LL));
                }
                px_srcline(415);
                _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v926, _v908}, 2);
                px_srcline(416);
                return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s165 = px_add(({ LXValue _s163 = px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v927), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v932}, 2)), px_str("}, (LXValue[]){")); LXValue _s164 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2); px_add(_s163, _s164); }), px_str("}, ")); LXValue _s166 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v931}, 1)}, 1); px_add(_s165, _s166); }), px_str(")"))}, 2);
            }
            px_srcline(417);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v927}, 1))) {
                px_srcline(418);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v926}, 1), px_int(1LL)))) {
                    px_srcline(419);
                    return px_add(px_add(px_str("枚举 "), _v927), px_str(" 构造需要一个变体名"));
                }
                px_srcline(420);
                _v917 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, px_int(0LL))}, 1);
                px_srcline(421);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v927), px_str("\", (")), _v917), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        px_srcline(423);
        if (px_is_truthy(px_eq(px_index(_v925, px_int(0LL)), px_str("Field")))) {
            px_srcline(424);
            _v918 = px_index(_v925, px_int(1LL));
            px_srcline(425);
            _v934 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v925, px_int(2LL))}, 1);
            px_srcline(427);
            _v935 = px_null();
            px_srcline(428);
            if (px_is_truthy(px_eq(px_index(_v918, px_int(0LL)), px_str("Var")))) {
                px_srcline(429);
                _v920 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v918, px_int(1LL))}, 1);
                px_srcline(430);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v920}, 1))) {
                    px_srcline(431);
                     _v935 = px_index(px_get_global("cg_var_types"), _v920);
                }
            }
            px_srcline(432);
            if (px_is_truthy(({ LXValue _t969 = px_ne(_v935, px_null()); px_is_truthy(_t969) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v935}, 1) : _t969; }))) {
                px_srcline(433);
                _v936 = px_index(px_get_global("cg_impls"), _v935);
                px_srcline(434);
                _v937 = px_bool(false);
                px_srcline(435);
                _v938 = px_int(0LL);
                px_srcline(436);
                while (px_is_truthy(px_lt(_v938, px_call(px_get_global("len"), (LXValue[]){_v936}, 1)))) {
                    px_srcline(437);
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v936, _v938), px_int(1LL))}, 1), _v934))) {
                        px_srcline(438);
                         _v937 = px_bool(true);
                        px_srcline(439);
                        break;
                    }
                    px_srcline(440);
                     _v938 = px_add(_v938, px_int(1LL));
                }
                px_srcline(441);
                if (px_is_truthy(_v937)) {
                    px_srcline(442);
                    _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v918}, 1);
                    px_srcline(443);
                    _v908 = px_list_n((LXValue[]){_v921}, 1);
                    px_srcline(444);
                    _v939 = px_list_n((LXValue[]){_v918}, 1);
                    px_srcline(445);
                    _v929 = px_int(0LL);
                    px_srcline(446);
                    while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
                        px_srcline(447);
                        (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
                        px_srcline(448);
                        (void)(px_method(_v939, "append", (LXValue[]){px_index(_v926, _v929)}, 1));
                        px_srcline(449);
                         _v929 = px_add(_v929, px_int(1LL));
                    }
                    px_srcline(450);
                    _v940 = ({ LXValue _s167 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v935}, 1)), px_str("_")); LXValue _s168 = px_call(px_get_global("cg_func_cname"), (LXValue[]){_v934}, 1); px_add(_s167, _s168); });
                    px_srcline(451);
                    _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v939, _v908}, 2);
                    px_srcline(452);
                    return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s169 = px_add(px_add(px_add(_v940, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s170 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v908}, 1)}, 1); px_add(_s169, _s170); }), px_str(", NULL)"))}, 2);
                }
            }
            px_srcline(456);
            _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v918}, 1);
            px_srcline(457);
            _v908 = px_list_n((LXValue[]){_v921}, 1);
            px_srcline(458);
            _v939 = px_list_n((LXValue[]){_v918}, 1);
            px_srcline(459);
            _v929 = px_int(0LL);
            px_srcline(460);
            while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
                px_srcline(461);
                (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
                px_srcline(462);
                (void)(px_method(_v939, "append", (LXValue[]){px_index(_v926, _v929)}, 1));
                px_srcline(463);
                 _v929 = px_add(_v929, px_int(1LL));
            }
            px_srcline(464);
            _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v939, _v908}, 2);
            px_srcline(465);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s171 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), px_index(px_index(_v911, px_int(1LL)), px_int(0LL))), px_str(", \"")), _v934), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_slice(px_index(_v911, px_int(1LL)), px_int(1LL), px_null(), px_null())}, 2)), px_str("}, ")); LXValue _s172 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v926}, 1)}, 1); px_add(_s171, _s172); }), px_str(")"))}, 2);
        }
        px_srcline(469);
        _v941 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v925}, 1);
        px_srcline(470);
        _v908 = px_list_n((LXValue[]){_v941}, 1);
        px_srcline(471);
        _v939 = px_list_n((LXValue[]){_v925}, 1);
        px_srcline(472);
        _v929 = px_int(0LL);
        px_srcline(473);
        while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
            px_srcline(474);
            (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
            px_srcline(475);
            (void)(px_method(_v939, "append", (LXValue[]){px_index(_v926, _v929)}, 1));
            px_srcline(476);
             _v929 = px_add(_v929, px_int(1LL));
        }
        px_srcline(477);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v939, _v908}, 2);
        px_srcline(478);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s173 = px_add(px_add(px_add(px_add(px_str("px_call("), px_index(px_index(_v911, px_int(1LL)), px_int(0LL))), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_slice(px_index(_v911, px_int(1LL)), px_int(1LL), px_null(), px_null())}, 2)), px_str("}, ")); LXValue _s174 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v926}, 1)}, 1); px_add(_s173, _s174); }), px_str(")"))}, 2);
    }
    px_srcline(479);
    if (px_is_truthy(px_eq(_v906, px_str("Unary")))) {
        px_srcline(480);
        _v921 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(481);
        _v942 = px_index(_v905, px_int(1LL));
        px_srcline(482);
        if (px_is_truthy(px_eq(_v942, px_str("Neg")))) {
            px_srcline(483);
            return px_add(px_add(px_str("px_neg("), _v921), px_str(")"));
        }
        px_srcline(484);
        if (px_is_truthy(px_eq(_v942, px_str("Not")))) {
            px_srcline(485);
            return px_add(px_add(px_str("px_not("), _v921), px_str(")"));
        }
        px_srcline(486);
        return px_add(px_add(px_str("px_bitnot("), _v921), px_str(")"));
    }
    px_srcline(487);
    if (px_is_truthy(px_eq(_v906, px_str("Binary")))) {
        px_srcline(488);
        _v943 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(489);
        _v944 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(3LL))}, 1);
        px_srcline(490);
        _v942 = px_index(_v905, px_int(1LL));
        px_srcline(491);
        if (px_is_truthy(px_eq(_v942, px_str("And")))) {
            px_srcline(492);
            _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(493);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v943), px_str("; px_is_truthy(")), _v922), px_str(") ? ")), _v944), px_str(" : ")), _v922), px_str("; })"));
        }
        px_srcline(494);
        if (px_is_truthy(px_eq(_v942, px_str("Or")))) {
            px_srcline(495);
            _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(496);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v943), px_str("; px_is_truthy(")), _v922), px_str(") ? ")), _v922), px_str(" : ")), _v944), px_str("; })"));
        }
        px_srcline(497);
        _v945 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v942}, 1);
        px_srcline(498);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v905, px_int(2LL)), px_index(_v905, px_int(3LL))}, 2), px_list_n((LXValue[]){_v943, _v944}, 2)}, 2);
        px_srcline(499);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(_v945, px_str("(")), px_index(px_index(_v911, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v911, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2);
    }
    px_srcline(500);
    if (px_is_truthy(px_eq(_v906, px_str("Pipe")))) {
        px_srcline(501);
        _v917 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(502);
        _v946 = px_index(_v905, px_int(2LL));
        px_srcline(503);
        if (px_is_truthy(px_eq(px_index(_v946, px_int(0LL)), px_str("Call")))) {
            px_srcline(504);
            _v941 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v946, px_int(1LL))}, 1);
            px_srcline(505);
            _v908 = px_list_n((LXValue[]){_v917}, 1);
            px_srcline(506);
            _v939 = px_list_n((LXValue[]){px_index(_v905, px_int(1LL))}, 1);
            px_srcline(507);
            _v929 = px_int(0LL);
            px_srcline(508);
            while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){px_index(_v946, px_int(2LL))}, 1)))) {
                px_srcline(509);
                (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v946, px_int(2LL)), _v929)}, 1)}, 1));
                px_srcline(510);
                (void)(px_method(_v939, "append", (LXValue[]){px_index(px_index(_v946, px_int(2LL)), _v929)}, 1));
                px_srcline(511);
                 _v929 = px_add(_v929, px_int(1LL));
            }
            px_srcline(512);
            _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v939, _v908}, 2);
            px_srcline(513);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s175 = px_add(px_add(px_add(px_add(px_str("px_call("), _v941), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s176 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v908}, 1)}, 1); px_add(_s175, _s176); }), px_str(")"))}, 2);
        }
        px_srcline(514);
        _v945 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v946}, 1);
        px_srcline(515);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v905, px_int(1LL)), px_index(_v905, px_int(2LL))}, 2), px_list_n((LXValue[]){_v917, _v945}, 2)}, 2);
        px_srcline(516);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_call("), px_index(px_index(_v911, px_int(1LL)), px_int(1LL))), px_str(", (LXValue[]){")), px_index(px_index(_v911, px_int(1LL)), px_int(0LL))), px_str("}, 1)"))}, 2);
    }
    px_srcline(517);
    if (px_is_truthy(px_eq(_v906, px_str("NullCoalesce")))) {
        px_srcline(518);
        _v943 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(519);
        _v944 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(520);
        _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(521);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v943), px_str("; px_is_null(")), _v922), px_str(") ? ")), _v944), px_str(" : ")), _v922), px_str("; })"));
    }
    px_srcline(522);
    if (px_is_truthy(px_eq(_v906, px_str("Try")))) {
        px_srcline(523);
        _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(524);
        _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(525);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            px_srcline(526);
            _v947 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            px_srcline(527);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v923), px_str("; if (px_is_result(")), _v922), px_str(")) { if (!px_result_ok(")), _v922), px_str(")) { ")), _v947), px_str("_val = ")), _v922), px_str("; ")), _v947), px_str("_proped = 1; goto ")), _v947), px_str("; } ")), _v922), px_str(" = px_result_unwrap(")), _v922), px_str("); } else if (px_is_null(")), _v922), px_str(")) { ")), _v947), px_str("_val = px_null(); ")), _v947), px_str("_proped = 1; goto ")), _v947), px_str("; } ")), _v922), px_str("; })"));
        }
        px_srcline(528);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v923), px_str("; if (px_is_result(")), _v922), px_str(") && !px_result_ok(")), _v922), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v922), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v922), px_str(")) ")), _v922), px_str(" = px_result_unwrap(")), _v922), px_str("); ")), _v922), px_str("; })"));
    }
    px_srcline(529);
    if (px_is_truthy(px_eq(_v906, px_str("ForceUnwrap")))) {
        px_srcline(530);
        _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(531);
        _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(532);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v923), px_str("; if (px_is_result(")), _v922), px_str(")) { if (!px_result_ok(")), _v922), px_str(")) px_error(\"force unwrap Err\"); ")), _v922), px_str(" = px_result_unwrap(")), _v922), px_str("); } if (px_is_null(")), _v922), px_str(")) px_error(\"force unwrap null\"); ")), _v922), px_str("; })"));
    }
    px_srcline(533);
    if (px_is_truthy(px_eq(_v906, px_str("IfExpr")))) {
        px_srcline(534);
        _v941 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(535);
        _v948 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(536);
        _v949 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(3LL))}, 1);
        px_srcline(537);
        _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(538);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str("; if (px_is_truthy(")), _v941), px_str(")) { ")), _v922), px_str(" = ")), _v948), px_str("; } else { ")), _v922), px_str(" = ")), _v949), px_str("; } ")), _v922), px_str("; })"));
    }
    px_srcline(539);
    if (px_is_truthy(px_eq(_v906, px_str("ListComp")))) {
        px_srcline(540);
        _v950 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(541);
        _v944 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(542);
        _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(543);
        _v951 = px_null();
        px_srcline(544);
        if (px_is_truthy(px_ne(px_index(_v905, px_int(3LL)), px_null()))) {
            px_srcline(545);
             _v951 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(3LL))}, 1);
        }
        px_srcline(546);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v944, px_str("saved_all"))}, 1));
        px_srcline(547);
        _v952 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v950), px_str(", ")), _v923), px_str("); "));
        px_srcline(548);
        _v953 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v944, _v951, _v952}, 3);
        px_srcline(549);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v950), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v944, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v944, px_str("its")), px_int(0LL))), px_str("; ")), _v953), px_str(" ")), _v950), px_str("; })"));
    }
    px_srcline(550);
    if (px_is_truthy(px_eq(_v906, px_str("GenExp")))) {
        px_srcline(551);
        _v954 = px_index(_v905, px_int(2LL));
        px_srcline(552);
        if (px_is_truthy(({ LXValue _t970 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v954}, 1), px_int(1LL)); px_is_truthy(_t970) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v954, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t970; }))) {
            px_srcline(553);
            _v955 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v954, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(554);
            _v956 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v954, px_int(0LL)), px_int(2LL))}, 1);
            px_srcline(555);
            _v957 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v955}, 1), px_index(_v905, px_int(1LL))}, 2);
            px_srcline(556);
            _v958 = px_str("px_null()");
            px_srcline(557);
            if (px_is_truthy(px_ne(px_index(_v905, px_int(3LL)), px_null()))) {
                px_srcline(558);
                 _v958 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v955}, 1), px_index(_v905, px_int(3LL))}, 2);
            }
            px_srcline(559);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v956), px_str(", ")), _v957), px_str(", ")), _v958), px_str(")"));
        }
        px_srcline(561);
        _v950 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(562);
        _v944 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v954}, 1);
        px_srcline(563);
        _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(564);
        _v951 = px_null();
        px_srcline(565);
        if (px_is_truthy(px_ne(px_index(_v905, px_int(3LL)), px_null()))) {
            px_srcline(566);
             _v951 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(3LL))}, 1);
        }
        px_srcline(567);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v944, px_str("saved_all"))}, 1));
        px_srcline(568);
        _v952 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v950), px_str(", ")), _v923), px_str("); "));
        px_srcline(569);
        _v953 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v944, _v951, _v952}, 3);
        px_srcline(570);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v950), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v944, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v944, px_str("its")), px_int(0LL))), px_str("; ")), _v953), px_str(" px_gen_from_list(")), _v950), px_str("); })"));
    }
    px_srcline(571);
    if (px_is_truthy(px_eq(_v906, px_str("DictComp")))) {
        px_srcline(572);
        _v950 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(573);
        _v944 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v905, px_int(3LL))}, 1);
        px_srcline(574);
        _v914 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(575);
        _v915 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(2LL))}, 1);
        px_srcline(576);
        _v951 = px_null();
        px_srcline(577);
        if (px_is_truthy(px_ne(px_index(_v905, px_int(4LL)), px_null()))) {
            px_srcline(578);
             _v951 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(4LL))}, 1);
        }
        px_srcline(579);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v944, px_str("saved_all"))}, 1));
        px_srcline(580);
        _v952 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v914), px_str("; LXValue _v = ")), _v915), px_str("; px_dict_set_checked(")), _v950), px_str(", _k, _v); } "));
        px_srcline(581);
        _v953 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v944, _v951, _v952}, 3);
        px_srcline(582);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v950), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v944, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v944, px_str("its")), px_int(0LL))), px_str("; ")), _v953), px_str(" ")), _v950), px_str("; })"));
    }
    px_srcline(583);
    if (px_is_truthy(px_eq(_v906, px_str("Closure")))) {
        px_srcline(586);
        return px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v905, px_int(1LL)), px_index(_v905, px_int(3LL)), px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_add(px_get_global("cg_closure_id"), px_int(1LL))}, 1)), px_str(">"))}, 3);
    }
    px_srcline(587);
    if (px_is_truthy(px_eq(_v906, px_str("Block")))) {
        px_srcline(588);
        _v912 = px_str("({ ");
        px_srcline(589);
         _v912 = px_add(_v912, px_str("LXValue _blk = px_null(); "));
        px_srcline(590);
        _v959 = px_index(_v905, px_int(1LL));
        px_srcline(591);
        _v960 = px_int(0LL);
        px_srcline(592);
        while (px_is_truthy(px_lt(_v960, px_call(px_get_global("len"), (LXValue[]){_v959}, 1)))) {
            px_srcline(593);
            _v961 = px_index(_v959, _v960);
            px_srcline(594);
            if (px_is_truthy(px_eq(px_index(_v961, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(595);
                _v923 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v961, px_int(1LL))}, 1);
                px_srcline(596);
                 _v912 = px_add(_v912, px_add(px_add(px_str("_blk = "), _v923), px_str("; ")));
            }
            else {
                px_srcline(598);
                 _v912 = px_add(_v912, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v961, px_int(0LL)}, 2));
            }
            px_srcline(599);
             _v960 = px_add(_v960, px_int(1LL));
        }
        px_srcline(600);
         _v912 = px_add(_v912, px_str("_blk; })"));
        px_srcline(601);
        return _v912;
    }
    px_srcline(602);
    if (px_is_truthy(px_eq(_v906, px_str("Match")))) {
        px_srcline(603);
        _v962 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(604);
        _v922 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(605);
        _v912 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v922), px_str(" = ")), _v962), px_str("; "));
        px_srcline(606);
        _v963 = px_index(_v905, px_int(2LL));
        px_srcline(607);
        _v964 = px_bool(true);
        px_srcline(608);
        _v929 = px_int(0LL);
        px_srcline(609);
        while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v963}, 1)))) {
            px_srcline(610);
            _v951 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v963, _v929), px_int(1LL)), _v922}, 2);
            px_srcline(611);
            _v965 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v963, _v929), px_int(3LL))}, 1);
            px_srcline(612);
            _v966 = px_str("if");
            px_srcline(613);
            if (px_is_truthy(px_not(_v964))) {
                px_srcline(614);
                 _v966 = px_str("else if");
            }
            px_srcline(615);
             _v912 = px_add(_v912, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v966, px_str(" (")), _v951), px_str(") { ")), _v922), px_str(" = ")), _v965), px_str("; } ")));
            px_srcline(616);
             _v964 = px_bool(false);
            px_srcline(617);
             _v929 = px_add(_v929, px_int(1LL));
        }
        px_srcline(618);
         _v912 = px_add(_v912, px_add(_v922, px_str("; })")));
        px_srcline(619);
        return _v912;
    }
    px_srcline(620);
    if (px_is_truthy(px_eq(_v906, px_str("Constructor")))) {
        px_srcline(621);
        _v916 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v905, px_int(1LL))}, 1);
        px_srcline(622);
        _v926 = px_index(_v905, px_int(2LL));
        px_srcline(623);
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v916}, 1))) {
            px_srcline(624);
            _v931 = px_index(px_get_global("cg_structs"), _v916);
            px_srcline(625);
            if (px_is_truthy(({ LXValue _s177 = px_call(px_get_global("len"), (LXValue[]){_v931}, 1); LXValue _s178 = px_call(px_get_global("len"), (LXValue[]){_v926}, 1); px_ne(_s177, _s178); }))) {
                px_srcline(626);
                return ({ LXValue _s179 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v916), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v931}, 1)}, 1)), px_str(" 个字段，给出 ")); LXValue _s180 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v926}, 1)}, 1); px_add(_s179, _s180); });
            }
            px_srcline(627);
            _v908 = px_list_n((LXValue[]){}, 0);
            px_srcline(628);
            _v929 = px_int(0LL);
            px_srcline(629);
            while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
                px_srcline(630);
                (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
                px_srcline(631);
                 _v929 = px_add(_v929, px_int(1LL));
            }
            px_srcline(632);
            _v932 = px_list_n((LXValue[]){}, 0);
            px_srcline(633);
            _v933 = px_int(0LL);
            px_srcline(634);
            while (px_is_truthy(px_lt(_v933, px_call(px_get_global("len"), (LXValue[]){_v931}, 1)))) {
                px_srcline(635);
                (void)(px_method(_v932, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v931, _v933)), px_str("\""))}, 1));
                px_srcline(636);
                 _v933 = px_add(_v933, px_int(1LL));
            }
            px_srcline(637);
            _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v926, _v908}, 2);
            px_srcline(638);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s183 = px_add(({ LXValue _s181 = px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v916), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v932}, 2)), px_str("}, (LXValue[]){")); LXValue _s182 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2); px_add(_s181, _s182); }), px_str("}, ")); LXValue _s184 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v931}, 1)}, 1); px_add(_s183, _s184); }), px_str(")"))}, 2);
        }
        px_srcline(639);
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v916}, 1))) {
            px_srcline(640);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v926}, 1), px_int(1LL)))) {
                px_srcline(641);
                return px_add(px_add(px_str("枚举 "), _v916), px_str(" 构造需要一个变体名"));
            }
            px_srcline(642);
            _v917 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, px_int(0LL))}, 1);
            px_srcline(643);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v916), px_str("\", (")), _v917), px_str(").as.obj->as.enum_inst.variant)"));
        }
        px_srcline(644);
        _v908 = px_list_n((LXValue[]){}, 0);
        px_srcline(645);
        _v929 = px_int(0LL);
        px_srcline(646);
        while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v926}, 1)))) {
            px_srcline(647);
            (void)(px_method(_v908, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v926, _v929)}, 1)}, 1));
            px_srcline(648);
             _v929 = px_add(_v929, px_int(1LL));
        }
        px_srcline(649);
        _v911 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v926, _v908}, 2);
        px_srcline(650);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v911, px_int(0LL)), px_add(({ LXValue _s185 = px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v916), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v911, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s186 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v908}, 1)}, 1); px_add(_s185, _s186); }), px_str(")"))}, 2);
    }
    px_srcline(651);
    return px_str("px_null()");
px_err_967:
    if (px_err_967_proped) return px_err_967_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_binop_cname");
    LXValue _v971 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_972_val = px_null();
    int px_err_972_proped = 0;
    px_srcline(654);
    if (px_is_truthy(px_eq(_v971, px_str("Add")))) {
        px_srcline(655);
        return px_str("px_add");
    }
    px_srcline(656);
    if (px_is_truthy(px_eq(_v971, px_str("Sub")))) {
        px_srcline(657);
        return px_str("px_sub");
    }
    px_srcline(658);
    if (px_is_truthy(px_eq(_v971, px_str("Mul")))) {
        px_srcline(659);
        return px_str("px_mul");
    }
    px_srcline(660);
    if (px_is_truthy(px_eq(_v971, px_str("Div")))) {
        px_srcline(661);
        return px_str("px_div");
    }
    px_srcline(662);
    if (px_is_truthy(px_eq(_v971, px_str("IntDiv")))) {
        px_srcline(663);
        return px_str("px_idiv");
    }
    px_srcline(664);
    if (px_is_truthy(px_eq(_v971, px_str("Mod")))) {
        px_srcline(665);
        return px_str("px_mod");
    }
    px_srcline(666);
    if (px_is_truthy(px_eq(_v971, px_str("Pow")))) {
        px_srcline(667);
        return px_str("px_pow");
    }
    px_srcline(668);
    if (px_is_truthy(px_eq(_v971, px_str("Eq")))) {
        px_srcline(669);
        return px_str("px_eq");
    }
    px_srcline(670);
    if (px_is_truthy(px_eq(_v971, px_str("Ne")))) {
        px_srcline(671);
        return px_str("px_ne");
    }
    px_srcline(672);
    if (px_is_truthy(px_eq(_v971, px_str("Lt")))) {
        px_srcline(673);
        return px_str("px_lt");
    }
    px_srcline(674);
    if (px_is_truthy(px_eq(_v971, px_str("Le")))) {
        px_srcline(675);
        return px_str("px_le");
    }
    px_srcline(676);
    if (px_is_truthy(px_eq(_v971, px_str("Gt")))) {
        px_srcline(677);
        return px_str("px_gt");
    }
    px_srcline(678);
    if (px_is_truthy(px_eq(_v971, px_str("Ge")))) {
        px_srcline(679);
        return px_str("px_ge");
    }
    px_srcline(680);
    if (px_is_truthy(px_eq(_v971, px_str("BitAnd")))) {
        px_srcline(681);
        return px_str("px_bitand");
    }
    px_srcline(682);
    if (px_is_truthy(px_eq(_v971, px_str("BitOr")))) {
        px_srcline(683);
        return px_str("px_bitor");
    }
    px_srcline(684);
    if (px_is_truthy(px_eq(_v971, px_str("BitXor")))) {
        px_srcline(685);
        return px_str("px_bitxor");
    }
    px_srcline(686);
    if (px_is_truthy(px_eq(_v971, px_str("Shl")))) {
        px_srcline(687);
        return px_str("px_shl");
    }
    px_srcline(688);
    if (px_is_truthy(px_eq(_v971, px_str("Shr")))) {
        px_srcline(689);
        return px_str("px_shr");
    }
    px_srcline(690);
    if (px_is_truthy(px_eq(_v971, px_str("ShrU")))) {
        px_srcline(691);
        return px_str("px_ushr");
    }
    px_srcline(692);
    return px_str("px_add");
px_err_972:
    if (px_err_972_proped) return px_err_972_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_pattern_cond");
    LXValue _v973 = (nargs > 0) ? args[0] : px_null();
    LXValue _v974 = (nargs > 1) ? args[1] : px_null();
    LXValue _v975 = px_null();
    LXValue _v976 = px_null();
    LXValue _v977 = px_null();
    LXValue _v978 = px_null();
    LXValue px_err_979_val = px_null();
    int px_err_979_proped = 0;
    px_srcline(695);
    _v975 = px_index(_v973, px_int(0LL));
    px_srcline(696);
    if (px_is_truthy(px_eq(_v975, px_str("PatLiteral")))) {
        px_srcline(697);
        _v976 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v973, px_int(1LL))}, 1);
        px_srcline(698);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v974), px_str(", ")), _v976), px_str("))"));
    }
    px_srcline(699);
    if (px_is_truthy(px_eq(_v975, px_str("PatBinding")))) {
        px_srcline(700);
        _v977 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v973, px_int(1LL))}, 1);
        px_srcline(701);
        if (px_is_truthy(({ LXValue _t981 = ({ LXValue _t980 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v977}, 1), px_int(0LL)); px_is_truthy(_t980) ? px_ge(px_index(_v977, px_int(0LL)), px_str("A")) : _t980; }); px_is_truthy(_t981) ? px_le(px_index(_v977, px_int(0LL)), px_str("Z")) : _t981; }))) {
            px_srcline(702);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v974), px_str(".type == PX_ENUM && strcmp(")), _v974), px_str(".as.obj->as.enum_inst.variant, \"")), _v977), px_str("\") == 0)"));
        }
        px_srcline(703);
        return px_str("true");
    }
    px_srcline(704);
    if (px_is_truthy(px_eq(_v975, px_str("PatWildcard")))) {
        px_srcline(705);
        return px_str("true");
    }
    px_srcline(706);
    if (px_is_truthy(px_eq(_v975, px_str("PatTuple")))) {
        px_srcline(707);
        _v978 = px_index(_v973, px_int(1LL));
        px_srcline(708);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v978}, 1), px_int(0LL)))) {
            px_srcline(709);
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v978, px_int(0LL)), _v974}, 2);
        }
        px_srcline(710);
        return px_str("true");
    }
    px_srcline(711);
    if (px_is_truthy(px_eq(_v975, px_str("PatConstructor")))) {
        px_srcline(712);
        _v977 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v973, px_int(1LL))}, 1);
        px_srcline(713);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v974), px_str(".type == PX_ENUM && strcmp(")), _v974), px_str(".as.obj->as.enum_inst.variant, \"")), _v977), px_str("\") == 0)"));
    }
    px_srcline(714);
    return px_str("true");
px_err_979:
    if (px_err_979_proped) return px_err_979_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_lambda");
    LXValue _v982 = (nargs > 0) ? args[0] : px_null();
    LXValue _v983 = (nargs > 1) ? args[1] : px_null();
    LXValue _v984 = px_null();
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
    LXValue _v1000 = px_null();
    LXValue px_err_1001_val = px_null();
    int px_err_1001_proped = 0;
    px_srcline(717);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(718);
    _v984 = px_get_global("cg_closure_id");
    px_srcline(719);
    _v985 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v984}, 1));
    px_srcline(721);
    _v986 = px_list_n((LXValue[]){}, 0);
    px_srcline(722);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v983, _v986}, 2));
    px_srcline(723);
    _v987 = px_list_n((LXValue[]){}, 0);
    px_srcline(724);
    LXValue _t1002 = _v986;
    int _il4 = (int)px_len(_t1002);
    for (int _t1003 = 0; _t1003 < _il4; _t1003++) {
        px_iter_ck(_t1002, _il4);
        _v988 = px_iter_at(_t1002, px_int(_t1003));
        px_srcline(725);
        if (px_is_truthy(({ LXValue _t1004 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v982, _v988}, 2)); px_is_truthy(_t1004) ? px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v988}, 1), px_null()) : _t1004; }))) {
            px_srcline(726);
            (void)(px_method(_v987, "append", (LXValue[]){_v988}, 1));
        }
    }
    px_srcline(727);
    _v989 = px_str("px_null()");
    px_srcline(728);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v987}, 1), px_int(0LL)))) {
        px_srcline(729);
        _v990 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(730);
        LXValue _t1005 = _v987;
        int _il5 = (int)px_len(_t1005);
        for (int _t1006 = 0; _t1006 < _il5; _t1006++) {
            px_iter_ck(_t1005, _il5);
            _v988 = px_iter_at(_t1005, px_int(_t1006));
            px_srcline(731);
            _v991 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v988}, 1);
            px_srcline(735);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v988}, 1))) {
                px_srcline(736);
                 _v990 = px_add(_v990, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v988), px_str("\", px_cell(px_cell_get(")), _v991), px_str("))); ")));
            }
            else {
                px_srcline(738);
                 _v990 = px_add(_v990, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v988), px_str("\", px_cell(")), _v991), px_str(")); ")));
            }
        }
        px_srcline(739);
         _v990 = px_add(_v990, px_str("_capenv; })"));
        px_srcline(740);
         _v989 = _v990;
    }
    px_srcline(741);
    _v992 = px_add(px_add(px_str("static LXValue "), _v985), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(742);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v987}, 1), px_int(0LL)))) {
        px_srcline(743);
         _v992 = px_add(_v992, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(745);
         _v992 = px_add(_v992, px_str("    (void)ctx;\n"));
    }
    px_srcline(746);
    _v993 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(747);
    _v994 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(748);
    _v995 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(749);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(750);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(751);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(752);
    _v996 = px_int(0LL);
    px_srcline(753);
    while (px_is_truthy(px_lt(_v996, px_call(px_get_global("len"), (LXValue[]){_v982}, 1)))) {
        px_srcline(754);
        _v997 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v982, _v996)}, 1);
        px_srcline(755);
         _v992 = px_add(_v992, px_add(({ LXValue _s187 = px_add(px_add(px_add(px_add(px_str("    LXValue "), _v997), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v996}, 1)), px_str(") ? args[")); LXValue _s188 = px_call(px_get_global("str"), (LXValue[]){_v996}, 1); px_add(_s187, _s188); }), px_str("] : px_null();\n")));
        px_srcline(756);
         _v996 = px_add(_v996, px_int(1LL));
    }
    px_srcline(757);
    LXValue _t1007 = _v987;
    int _il6 = (int)px_len(_t1007);
    for (int _t1008 = 0; _t1008 < _il6; _t1008++) {
        px_iter_ck(_t1007, _il6);
        _v988 = px_iter_at(_t1007, px_int(_t1008));
        px_srcline(758);
        _v998 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v988}, 1);
        px_srcline(759);
        px_index_set(px_get_global("cg_cells"), _v988, px_int(1LL));
        px_srcline(760);
         _v992 = px_add(_v992, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v998), px_str(" = px_env_lookup(ctx, \"")), _v988), px_str("\");\n")));
    }
    px_srcline(761);
    _v999 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v983}, 1);
    px_srcline(762);
    _v1000 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(763);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1000}, 1));
    px_srcline(764);
     _v992 = px_add(_v992, px_add(px_add(px_str("    LXValue "), _v1000), px_str("_val = px_null();\n")));
    px_srcline(765);
     _v992 = px_add(_v992, px_add(px_add(px_str("    int "), _v1000), px_str("_proped = 0;\n")));
    px_srcline(766);
     _v992 = px_add(_v992, px_add(px_add(px_str("    return "), _v999), px_str(";\n")));
    px_srcline(767);
     _v992 = px_add(_v992, px_add(_v1000, px_str(":\n")));
    px_srcline(768);
     _v992 = px_add(_v992, px_add(px_add(px_add(px_add(px_str("    if ("), _v1000), px_str("_proped) return ")), _v1000), px_str("_val;\n")));
    px_srcline(769);
     _v992 = px_add(_v992, px_str("    return px_null();\n"));
    px_srcline(770);
     _v992 = px_add(_v992, px_str("}\n"));
    px_srcline(771);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(772);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v992));
    px_srcline(773);
    px_set_global("cg_vars", _v993);
    px_srcline(774);
    px_set_global("cg_var_types", _v994);
    px_srcline(775);
    px_set_global("cg_cells", _v995);
    px_srcline(777);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v987}, 1), px_int(0LL)))) {
        px_srcline(778);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v984}, 1)), px_str(">\", ")), _v985), px_str(", ")), _v989), px_str(")"));
    }
    px_srcline(779);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v984}, 1)), px_str(">\", ")), _v985), px_str(", NULL)"));
px_err_1001:
    if (px_err_1001_proped) return px_err_1001_val;
    return px_null();
}

static LXValue fn_cgm_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_perr");
    LXValue _v1009 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1010 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1011_val = px_null();
    int px_err_1011_proped = 0;
    px_srcline(20);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1009), px_str(": ")), _v1010)}, 1));
    px_srcline(21);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1011:
    if (px_err_1011_proped) return px_err_1011_val;
    return px_null();
}

static LXValue fn_cgm_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_pwarn");
    LXValue _v1012 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1013_val = px_null();
    int px_err_1013_proped = 0;
    px_srcline(23);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1012)}, 1));
px_err_1013:
    if (px_err_1013_proped) return px_err_1013_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dirname");
    LXValue _v1014 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1015 = px_null();
    LXValue px_err_1016_val = px_null();
    int px_err_1016_proped = 0;
    px_srcline(26);
    _v1015 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1014}, 1), px_int(1LL));
    px_srcline(27);
    while (px_is_truthy(px_ge(_v1015, px_int(0LL)))) {
        px_srcline(28);
        if (px_is_truthy(px_eq(px_index(_v1014, _v1015), px_str("/")))) {
            px_srcline(29);
            if (px_is_truthy(px_eq(_v1015, px_int(0LL)))) {
                px_srcline(30);
                return px_str("/");
            }
            px_srcline(31);
            return px_slice(_v1014, px_int(0LL), _v1015, px_null());
        }
        px_srcline(32);
         _v1015 = px_sub(_v1015, px_int(1LL));
    }
    px_srcline(33);
    return px_str(".");
px_err_1016:
    if (px_err_1016_proped) return px_err_1016_val;
    return px_null();
}

static LXValue fn_cg_norm_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_norm_path");
    LXValue _v1017 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1018 = px_null();
    LXValue _v1019 = px_null();
    LXValue _v1020 = px_null();
    LXValue _v1021 = px_null();
    LXValue _v1022 = px_null();
    LXValue _v1023 = px_null();
    LXValue px_err_1024_val = px_null();
    int px_err_1024_proped = 0;
    px_srcline(37);
    _v1018 = ({ LXValue _t1025 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1017}, 1), px_int(0LL)); px_is_truthy(_t1025) ? px_eq(px_index(_v1017, px_int(0LL)), px_str("/")) : _t1025; });
    px_srcline(38);
    _v1019 = px_list_n((LXValue[]){}, 0);
    px_srcline(39);
    _v1020 = px_str("");
    px_srcline(40);
    _v1021 = px_int(0LL);
    px_srcline(41);
    while (px_is_truthy(px_le(_v1021, px_call(px_get_global("len"), (LXValue[]){_v1017}, 1)))) {
        px_srcline(42);
        if (px_is_truthy(({ LXValue _t1026 = px_eq(_v1021, px_call(px_get_global("len"), (LXValue[]){_v1017}, 1)); px_is_truthy(_t1026) ? _t1026 : px_eq(px_index(_v1017, _v1021), px_str("/")); }))) {
            px_srcline(43);
            if (px_is_truthy(px_eq(_v1020, px_str("..")))) {
                px_srcline(44);
                if (px_is_truthy(({ LXValue _t1027 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1019}, 1), px_int(0LL)); px_is_truthy(_t1027) ? px_ne(px_index(_v1019, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1019}, 1), px_int(1LL))), px_str("..")) : _t1027; }))) {
                    px_srcline(45);
                    (void)(px_method(_v1019, "pop", (LXValue[]){}, 0));
                }
                else if (px_is_truthy(px_not(_v1018))) {
                    px_srcline(47);
                    (void)(px_method(_v1019, "append", (LXValue[]){px_str("..")}, 1));
                }
            }
            else if (px_is_truthy(({ LXValue _t1028 = px_ne(_v1020, px_str("")); px_is_truthy(_t1028) ? px_ne(_v1020, px_str(".")) : _t1028; }))) {
                px_srcline(49);
                (void)(px_method(_v1019, "append", (LXValue[]){_v1020}, 1));
            }
            px_srcline(50);
             _v1020 = px_str("");
        }
        else {
            px_srcline(52);
             _v1020 = px_add(_v1020, px_index(_v1017, _v1021));
        }
        px_srcline(53);
         _v1021 = px_add(_v1021, px_int(1LL));
    }
    px_srcline(54);
    _v1022 = px_str("");
    px_srcline(55);
    _v1023 = px_int(0LL);
    px_srcline(56);
    while (px_is_truthy(px_lt(_v1023, px_call(px_get_global("len"), (LXValue[]){_v1019}, 1)))) {
        px_srcline(57);
         _v1022 = px_add(_v1022, px_add(px_str("/"), px_index(_v1019, _v1023)));
        px_srcline(58);
         _v1023 = px_add(_v1023, px_int(1LL));
    }
    px_srcline(59);
    if (px_is_truthy(_v1018)) {
        px_srcline(60);
        if (px_is_truthy(px_eq(_v1022, px_str("")))) {
            px_srcline(61);
            return px_str("/");
        }
        px_srcline(62);
        return _v1022;
    }
    px_srcline(63);
    if (px_is_truthy(px_eq(_v1022, px_str("")))) {
        px_srcline(64);
        return px_str(".");
    }
    px_srcline(65);
    return px_slice(_v1022, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1022}, 1), px_null());
px_err_1024:
    if (px_err_1024_proped) return px_err_1024_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_stdlib_dir");
    LXValue _v1029 = px_null();
    LXValue _v1030 = px_null();
    LXValue _v1031 = px_null();
    LXValue _v1032 = px_null();
    LXValue px_err_1033_val = px_null();
    int px_err_1033_proped = 0;
    px_srcline(68);
    _v1029 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    px_srcline(69);
    if (px_is_truthy(({ LXValue _t1034 = px_ne(_v1029, px_null()); px_is_truthy(_t1034) ? px_call(px_get_global("exists"), (LXValue[]){_v1029}, 1) : _t1034; }))) {
        px_srcline(70);
        return _v1029;
    }
    px_srcline(71);
    _v1030 = px_list_n((LXValue[]){px_str("/data/code/puxian/stdlib"), px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 5);
    px_srcline(72);
    _v1031 = px_int(0LL);
    px_srcline(73);
    while (px_is_truthy(px_lt(_v1031, px_call(px_get_global("len"), (LXValue[]){_v1030}, 1)))) {
        px_srcline(74);
        _v1032 = px_index(_v1030, _v1031);
        px_srcline(75);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1032}, 1))) {
            px_srcline(76);
            return _v1032;
        }
        px_srcline(77);
         _v1031 = px_add(_v1031, px_int(1LL));
    }
    px_srcline(78);
    return px_null();
px_err_1033:
    if (px_err_1033_proped) return px_err_1033_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find_module_path");
    LXValue _v1035 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1036 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1037 = px_null();
    LXValue _v1038 = px_null();
    LXValue _v1039 = px_null();
    LXValue _v1040 = px_null();
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
    LXValue _v1051 = px_null();
    LXValue px_err_1052_val = px_null();
    int px_err_1052_proped = 0;
    px_srcline(81);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1035}, 1), px_int(0LL)))) {
        px_srcline(82);
        return px_null();
    }
    px_srcline(84);
    if (px_is_truthy(({ LXValue _t1054 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1035}, 1), px_int(1LL)); px_is_truthy(_t1054) ? ({ LXValue _t1053 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v1035, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1053) ? _t1053 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v1035, px_int(0LL)), px_str(".px")}, 2); }) : _t1054; }))) {
        px_srcline(85);
        _v1037 = px_index(_v1035, px_int(0LL));
        px_srcline(86);
        _v1038 = _v1037;
        px_srcline(87);
        if (px_is_truthy(px_not(({ LXValue _t1055 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1037}, 1), px_int(0LL)); px_is_truthy(_t1055) ? px_eq(px_index(_v1037, px_int(0LL)), px_str("/")) : _t1055; })))) {
            px_srcline(88);
             _v1038 = px_add(px_add(_v1036, px_str("/")), _v1037);
        }
        px_srcline(89);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1038}, 1))) {
            px_srcline(90);
            return _v1038;
        }
        px_srcline(91);
        return px_null();
    }
    px_srcline(93);
    if (px_is_truthy(px_eq(px_index(_v1035, px_int(0LL)), px_str("std")))) {
        px_srcline(94);
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v1035}, 1), px_int(2LL)))) {
            px_srcline(95);
            return px_null();
        }
        px_srcline(96);
        _v1039 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(97);
        if (px_is_truthy(px_eq(_v1039, px_null()))) {
            px_srcline(98);
            return px_null();
        }
        px_srcline(99);
        _v1038 = _v1039;
        px_srcline(100);
        _v1040 = px_int(1LL);
        px_srcline(101);
        while (px_is_truthy(px_lt(_v1040, px_call(px_get_global("len"), (LXValue[]){_v1035}, 1)))) {
            px_srcline(102);
             _v1038 = px_add(_v1038, px_add(px_str("/"), px_index(_v1035, _v1040)));
            px_srcline(103);
             _v1040 = px_add(_v1040, px_int(1LL));
        }
        px_srcline(104);
        _v1041 = px_add(_v1038, px_str(".px"));
        px_srcline(105);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1041}, 1))) {
            px_srcline(106);
            return _v1041;
        }
        px_srcline(107);
        _v1042 = px_add(_v1038, px_str("/mod.px"));
        px_srcline(108);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1042}, 1))) {
            px_srcline(109);
            return _v1042;
        }
        px_srcline(110);
        return px_null();
    }
    px_srcline(112);
    _v1043 = px_list_n((LXValue[]){_v1036}, 1);
    px_srcline(113);
    _v1044 = px_add(_v1036, px_str("/.px_modules"));
    px_srcline(114);
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1044}, 1))) {
        px_srcline(115);
        (void)(px_method(_v1043, "append", (LXValue[]){_v1044}, 1));
        px_srcline(116);
        _v1045 = px_call(px_get_global("list_dir"), (LXValue[]){_v1044}, 1);
        px_srcline(117);
        _v1046 = px_int(0LL);
        px_srcline(118);
        while (px_is_truthy(px_lt(_v1046, px_call(px_get_global("len"), (LXValue[]){_v1045}, 1)))) {
            px_srcline(119);
            _v1047 = px_index(_v1045, _v1046);
            px_srcline(120);
            _v1048 = px_add(px_add(_v1044, px_str("/")), _v1047);
            px_srcline(121);
            if (px_is_truthy(({ LXValue _t1056 = px_call(px_get_global("exists"), (LXValue[]){_v1048}, 1); px_is_truthy(_t1056) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1047, px_str(".")}, 2)) : _t1056; }))) {
                px_srcline(122);
                (void)(px_method(_v1043, "append", (LXValue[]){_v1048}, 1));
            }
            px_srcline(123);
             _v1046 = px_add(_v1046, px_int(1LL));
        }
    }
    px_srcline(124);
    _v1049 = px_int(0LL);
    px_srcline(125);
    while (px_is_truthy(px_lt(_v1049, px_call(px_get_global("len"), (LXValue[]){_v1043}, 1)))) {
        px_srcline(126);
        _v1050 = px_index(_v1043, _v1049);
        px_srcline(127);
        _v1038 = _v1050;
        px_srcline(128);
        _v1040 = px_int(0LL);
        px_srcline(129);
        while (px_is_truthy(px_lt(_v1040, px_call(px_get_global("len"), (LXValue[]){_v1035}, 1)))) {
            px_srcline(130);
             _v1038 = px_add(_v1038, px_add(px_str("/"), px_index(_v1035, _v1040)));
            px_srcline(131);
             _v1040 = px_add(_v1040, px_int(1LL));
        }
        px_srcline(132);
        _v1041 = px_add(_v1038, px_str(".px"));
        px_srcline(133);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1041}, 1))) {
            px_srcline(134);
            return _v1041;
        }
        px_srcline(135);
        _v1042 = px_add(_v1038, px_str("/mod.px"));
        px_srcline(136);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1042}, 1))) {
            px_srcline(137);
            return _v1042;
        }
        px_srcline(138);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1035}, 1), px_int(1LL)))) {
            px_srcline(139);
            _v1051 = px_add(px_add(px_add(_v1050, px_str("/")), px_index(_v1035, px_int(0LL))), px_str(".px"));
            px_srcline(140);
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1051}, 1))) {
                px_srcline(141);
                return _v1051;
            }
        }
        px_srcline(142);
         _v1049 = px_add(_v1049, px_int(1LL));
    }
    px_srcline(143);
    return px_null();
px_err_1052:
    if (px_err_1052_proped) return px_err_1052_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_definition");
    LXValue _v1057 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1058 = px_null();
    LXValue px_err_1059_val = px_null();
    int px_err_1059_proped = 0;
    px_srcline(146);
    _v1058 = px_index(_v1057, px_int(0LL));
    px_srcline(147);
    if (px_is_truthy(px_eq(_v1058, px_str("FuncDef")))) {
        px_srcline(149);
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1057, px_int(1LL))}, 1), px_str("main")))) {
            px_srcline(150);
            return px_bool(false);
        }
        px_srcline(151);
        return px_bool(true);
    }
    px_srcline(152);
    if (px_is_truthy(px_eq(_v1058, px_str("ExternDef")))) {
        px_srcline(153);
        return px_bool(true);
    }
    px_srcline(154);
    if (px_is_truthy(({ LXValue _t1062 = ({ LXValue _t1061 = ({ LXValue _t1060 = px_eq(_v1058, px_str("StructDef")); px_is_truthy(_t1060) ? _t1060 : px_eq(_v1058, px_str("EnumDef")); }); px_is_truthy(_t1061) ? _t1061 : px_eq(_v1058, px_str("TraitDef")); }); px_is_truthy(_t1062) ? _t1062 : px_eq(_v1058, px_str("ImplDef")); }))) {
        px_srcline(155);
        return px_bool(true);
    }
    px_srcline(156);
    if (px_is_truthy(px_eq(_v1058, px_str("VarDecl")))) {
        px_srcline(160);
        return px_bool(true);
    }
    px_srcline(161);
    return px_bool(false);
px_err_1059:
    if (px_err_1059_proped) return px_err_1059_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_def_name");
    LXValue _v1063 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1064 = px_null();
    LXValue _v1065 = px_null();
    LXValue _v1066 = px_null();
    LXValue _v1067 = px_null();
    LXValue px_err_1068_val = px_null();
    int px_err_1068_proped = 0;
    px_srcline(164);
    _v1064 = px_index(_v1063, px_int(0LL));
    px_srcline(165);
    if (px_is_truthy(({ LXValue _t1072 = ({ LXValue _t1071 = ({ LXValue _t1070 = ({ LXValue _t1069 = px_eq(_v1064, px_str("FuncDef")); px_is_truthy(_t1069) ? _t1069 : px_eq(_v1064, px_str("StructDef")); }); px_is_truthy(_t1070) ? _t1070 : px_eq(_v1064, px_str("EnumDef")); }); px_is_truthy(_t1071) ? _t1071 : px_eq(_v1064, px_str("TraitDef")); }); px_is_truthy(_t1072) ? _t1072 : px_eq(_v1064, px_str("ExternDef")); }))) {
        px_srcline(166);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1);
    }
    px_srcline(167);
    if (px_is_truthy(px_eq(_v1064, px_str("VarDecl")))) {
        px_srcline(168);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1063, px_int(2LL))}, 1);
    }
    px_srcline(169);
    if (px_is_truthy(px_eq(_v1064, px_str("ImplDef")))) {
        px_srcline(170);
        _v1065 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1063, px_int(1LL))}, 1);
        px_srcline(171);
        _v1066 = px_index(_v1063, px_int(2LL));
        px_srcline(172);
        _v1067 = px_str("None");
        px_srcline(173);
        if (px_is_truthy(px_ne(_v1066, px_null()))) {
            px_srcline(174);
             _v1067 = px_add(px_add(px_str("Some("), _v1066), px_str(")"));
        }
        px_srcline(175);
        return px_add(px_add(px_add(px_str("impl::"), _v1065), px_str("::")), _v1067);
    }
    px_srcline(176);
    return px_null();
px_err_1068:
    if (px_err_1068_proped) return px_err_1068_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_module");
    LXValue _v1073 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1074 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1075 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1076 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1077 = (nargs > 4) ? args[4] : px_null();
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
    LXValue _v1093 = px_null();
    LXValue _v1094 = px_null();
    LXValue _v1095 = px_null();
    LXValue _v1096 = px_null();
    LXValue _v1097 = px_null();
    LXValue px_err_1098_val = px_null();
    int px_err_1098_proped = 0;
    px_srcline(180);
    _v1078 = px_list_n((LXValue[]){}, 0);
    px_srcline(181);
    _v1079 = px_int(0LL);
    px_srcline(182);
    while (px_is_truthy(px_lt(_v1079, px_call(px_get_global("len"), (LXValue[]){_v1073}, 1)))) {
        px_srcline(183);
        (void)(px_method(_v1078, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1073, _v1079)}, 1)}, 1));
        px_srcline(184);
         _v1079 = px_add(_v1079, px_int(1LL));
    }
    px_srcline(185);
     _v1073 = _v1078;
    px_srcline(186);
    _v1080 = px_list_n((LXValue[]){}, 0);
    px_srcline(187);
    _v1081 = px_int(0LL);
    px_srcline(188);
    while (px_is_truthy(px_lt(_v1081, px_call(px_get_global("len"), (LXValue[]){_v1074}, 1)))) {
        px_srcline(189);
        (void)(px_method(_v1080, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1074, _v1081)}, 1)}, 1));
        px_srcline(190);
         _v1081 = px_add(_v1081, px_int(1LL));
    }
    px_srcline(191);
     _v1074 = _v1080;
    px_srcline(192);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1073}, 1), px_int(0LL)))) {
        px_srcline(193);
        return px_null();
    }
    px_srcline(195);
    if (px_is_truthy(({ LXValue _t1101 = ({ LXValue _t1100 = ({ LXValue _t1099 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1073}, 1), px_int(1LL)); px_is_truthy(_t1099) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1073, px_int(0LL))}, 1), px_int(2LL)) : _t1099; }); px_is_truthy(_t1100) ? px_eq(px_slice(px_index(_v1073, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t1100; }); px_is_truthy(_t1101) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v1073, px_int(0LL)), px_str(".px")}, 2)) : _t1101; }))) {
        px_srcline(196);
        return px_null();
    }
    px_srcline(197);
    _v1082 = px_eq(px_index(_v1073, px_int(0LL)), px_str("std"));
    px_srcline(198);
    _v1083 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1073}, 2);
    px_srcline(199);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1083}, 1))) {
        px_srcline(200);
        return px_null();
    }
    px_srcline(201);
    _v1084 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1073, _v1075}, 2);
    px_srcline(202);
    if (px_is_truthy(px_eq(_v1084, px_null()))) {
        px_srcline(209);
        _v1085 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(210);
        _v1086 = px_str("");
        px_srcline(211);
        if (px_is_truthy(({ LXValue _t1102 = px_eq(px_index(_v1073, px_int(0LL)), px_str("std")); px_is_truthy(_t1102) ? px_eq(_v1085, px_null()) : _t1102; }))) {
            px_srcline(212);
             _v1086 = px_str("；stdlib 未找到（设 PX_STDLIB 或放置 stdlib/）");
        }
        px_srcline(213);
        if (px_is_truthy(px_ne(px_call(px_get_global("env"), (LXValue[]){px_str("PX_STRICT_MODULE")}, 1), px_null()))) {
            px_srcline(214);
            (void)(px_call(px_get_global("cgm_perr"), (LXValue[]){px_str("E3005"), px_add(px_add(px_add(px_str("找不到模块 '"), _v1083), px_str("'")), _v1086)}, 2));
        }
        px_srcline(215);
        (void)(px_call(px_get_global("cgm_pwarn"), (LXValue[]){px_add(px_add(px_add(px_str("找不到模块 '"), _v1083), px_str("'（已跳过 → 运行期将报未定义）")), _v1086)}, 1));
        px_srcline(216);
        return px_null();
    }
    px_srcline(220);
    _v1087 = px_add(px_str("#"), px_call(px_get_global("cg_norm_path"), (LXValue[]){_v1084}, 1));
    px_srcline(221);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1087}, 1))) {
        px_srcline(222);
        return px_null();
    }
    px_srcline(223);
    px_index_set(px_get_global("loaded"), _v1083, _v1084);
    px_srcline(224);
    px_index_set(px_get_global("loaded"), _v1087, _v1084);
    px_srcline(225);
    _v1088 = px_call(px_get_global("read_file"), (LXValue[]){_v1084}, 1);
    px_srcline(226);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1088}, 1));
    px_srcline(227);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(228);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(229);
    _v1089 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(231);
    _v1090 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1084}, 1);
    px_srcline(232);
    _v1091 = px_list_n((LXValue[]){}, 0);
    px_srcline(233);
    _v1092 = px_int(0LL);
    px_srcline(234);
    while (px_is_truthy(px_lt(_v1092, px_call(px_get_global("len"), (LXValue[]){px_index(_v1089, px_int(1LL))}, 1)))) {
        px_srcline(235);
        _v1093 = px_index(px_index(_v1089, px_int(1LL)), _v1092);
        px_srcline(236);
        if (px_is_truthy(px_eq(px_index(_v1093, px_int(0LL)), px_str("Import")))) {
            px_srcline(237);
            (void)(px_method(_v1091, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1093, px_int(1LL)), px_index(_v1093, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(238);
         _v1092 = px_add(_v1092, px_int(1LL));
    }
    px_srcline(239);
    _v1094 = px_int(0LL);
    px_srcline(240);
    while (px_is_truthy(px_lt(_v1094, px_call(px_get_global("len"), (LXValue[]){_v1091}, 1)))) {
        px_srcline(241);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1091, _v1094), px_int(0LL)), px_index(px_index(_v1091, _v1094), px_int(1LL)), _v1090, _v1076, _v1077}, 5));
        px_srcline(242);
         _v1094 = px_add(_v1094, px_int(1LL));
    }
    px_srcline(244);
    _v1095 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1074}, 1), px_int(0LL));
    px_srcline(245);
    _v1096 = px_int(0LL);
    px_srcline(246);
    while (px_is_truthy(px_lt(_v1096, px_call(px_get_global("len"), (LXValue[]){px_index(_v1089, px_int(1LL))}, 1)))) {
        px_srcline(247);
        _v1093 = px_index(px_index(_v1089, px_int(1LL)), _v1096);
        px_srcline(248);
        if (px_is_truthy(px_eq(px_index(_v1093, px_int(0LL)), px_str("Import")))) {
            px_srcline(249);
             _v1096 = px_add(_v1096, px_int(1LL));
            px_srcline(250);
            continue;
        }
        px_srcline(251);
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1093}, 1)))) {
            px_srcline(252);
             _v1096 = px_add(_v1096, px_int(1LL));
            px_srcline(253);
            continue;
        }
        px_srcline(254);
        _v1097 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1093}, 1);
        px_srcline(255);
        if (px_is_truthy(px_eq(_v1097, px_null()))) {
            px_srcline(256);
            (void)(px_method(_v1076, "append", (LXValue[]){_v1093}, 1));
        }
        else {
            px_srcline(258);
            if (px_is_truthy(_v1095)) {
                px_srcline(259);
                if (px_is_truthy(({ LXValue _t1103 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1097}, 1), px_int(5LL)); px_is_truthy(_t1103) ? px_eq(px_slice(_v1097, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1103; }))) {
                    px_srcline(260);
                     _v1096 = px_add(_v1096, px_int(1LL));
                    px_srcline(261);
                    continue;
                }
                px_srcline(262);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1074, _v1097}, 2)))) {
                    px_srcline(263);
                     _v1096 = px_add(_v1096, px_int(1LL));
                    px_srcline(264);
                    continue;
                }
            }
            px_srcline(265);
            if (px_is_truthy(_v1082)) {
                px_srcline(266);
                if (px_is_truthy(px_method(_v1077, "has", (LXValue[]){_v1097}, 1))) {
                    px_srcline(267);
                     _v1096 = px_add(_v1096, px_int(1LL));
                    px_srcline(268);
                    continue;
                }
                px_srcline(269);
                px_index_set(_v1077, _v1097, px_bool(true));
            }
            px_srcline(270);
            (void)(px_method(_v1076, "append", (LXValue[]){_v1093}, 1));
        }
        px_srcline(271);
         _v1096 = px_add(_v1096, px_int(1LL));
    }
px_err_1098:
    if (px_err_1098_proped) return px_err_1098_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_resolve_modules");
    LXValue _v1104 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1105 = (nargs > 1) ? args[1] : px_null();
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
    LXValue px_err_1116_val = px_null();
    int px_err_1116_proped = 0;
    px_srcline(274);
    _v1106 = px_index(_v1104, px_int(1LL));
    px_srcline(275);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1106}, 1), px_int(0LL)))) {
        px_srcline(276);
        return _v1104;
    }
    px_srcline(277);
    _v1107 = px_list_n((LXValue[]){}, 0);
    px_srcline(278);
    _v1108 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(279);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(280);
    _v1109 = px_list_n((LXValue[]){}, 0);
    px_srcline(281);
    _v1110 = px_int(0LL);
    px_srcline(282);
    while (px_is_truthy(px_lt(_v1110, px_call(px_get_global("len"), (LXValue[]){_v1106}, 1)))) {
        px_srcline(283);
        _v1111 = px_index(_v1106, _v1110);
        px_srcline(284);
        if (px_is_truthy(px_eq(px_index(_v1111, px_int(0LL)), px_str("Import")))) {
            px_srcline(285);
            (void)(px_method(_v1109, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1111, px_int(1LL)), px_index(_v1111, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(286);
         _v1110 = px_add(_v1110, px_int(1LL));
    }
    px_srcline(287);
    _v1112 = px_int(0LL);
    px_srcline(288);
    while (px_is_truthy(px_lt(_v1112, px_call(px_get_global("len"), (LXValue[]){_v1109}, 1)))) {
        px_srcline(289);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1109, _v1112), px_int(0LL)), px_index(px_index(_v1109, _v1112), px_int(1LL)), _v1105, _v1107, _v1108}, 5));
        px_srcline(290);
         _v1112 = px_add(_v1112, px_int(1LL));
    }
    px_srcline(291);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1107}, 1), px_int(0LL)))) {
        px_srcline(292);
        return _v1104;
    }
    px_srcline(293);
    _v1113 = px_list_n((LXValue[]){}, 0);
    px_srcline(294);
    _v1114 = px_int(0LL);
    px_srcline(295);
    while (px_is_truthy(px_lt(_v1114, px_call(px_get_global("len"), (LXValue[]){_v1107}, 1)))) {
        px_srcline(296);
        (void)(px_method(_v1113, "append", (LXValue[]){px_index(_v1107, _v1114)}, 1));
        px_srcline(297);
         _v1114 = px_add(_v1114, px_int(1LL));
    }
    px_srcline(298);
    _v1115 = px_int(0LL);
    px_srcline(299);
    while (px_is_truthy(px_lt(_v1115, px_call(px_get_global("len"), (LXValue[]){_v1106}, 1)))) {
        px_srcline(300);
        (void)(px_method(_v1113, "append", (LXValue[]){px_index(_v1106, _v1115)}, 1));
        px_srcline(301);
         _v1115 = px_add(_v1115, px_int(1LL));
    }
    px_srcline(302);
    return px_list_n((LXValue[]){px_str("Program"), _v1113}, 2);
px_err_1116:
    if (px_err_1116_proped) return px_err_1116_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_dict");
    LXValue _v1117 = px_null();
    LXValue px_err_1118_val = px_null();
    int px_err_1118_proped = 0;
    px_srcline(79);
    _v1117 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(80);
    (void)(px_method(_v1117, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(81);
    return _v1117;
px_err_1118:
    if (px_err_1118_proped) return px_err_1118_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dict_copy");
    LXValue _v1119 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1120 = px_null();
    LXValue _v1121 = px_null();
    LXValue _v1122 = px_null();
    LXValue px_err_1123_val = px_null();
    int px_err_1123_proped = 0;
    px_srcline(83);
    _v1120 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(84);
    _v1121 = px_method(_v1119, "keys", (LXValue[]){}, 0);
    px_srcline(85);
    _v1122 = px_int(0LL);
    px_srcline(86);
    while (px_is_truthy(px_lt(_v1122, px_call(px_get_global("len"), (LXValue[]){_v1121}, 1)))) {
        px_srcline(87);
        px_index_set(_v1120, px_index(_v1121, _v1122), px_index(_v1119, px_index(_v1121, _v1122)));
        px_srcline(88);
         _v1122 = px_add(_v1122, px_int(1LL));
    }
    px_srcline(89);
    return _v1120;
px_err_1123:
    if (px_err_1123_proped) return px_err_1123_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_uid");
    LXValue px_err_1124_val = px_null();
    int px_err_1124_proped = 0;
    px_srcline(91);
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    px_srcline(92);
    return px_get_global("cg_uidc");
px_err_1124:
    if (px_err_1124_proped) return px_err_1124_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_tmp");
    LXValue px_err_1125_val = px_null();
    int px_err_1125_proped = 0;
    px_srcline(94);
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1125:
    if (px_err_1125_proped) return px_err_1125_val;
    return px_null();
}

static LXValue fn_cg_iter_len_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_iter_len_tmp");
    LXValue px_err_1126_val = px_null();
    int px_err_1126_proped = 0;
    px_srcline(99);
    px_set_global("cg_iter_uid", px_add(px_get_global("cg_iter_uid"), px_int(1LL)));
    px_srcline(100);
    return px_add(px_str("_il"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_iter_uid")}, 1));
px_err_1126:
    if (px_err_1126_proped) return px_err_1126_val;
    return px_null();
}

static LXValue fn_cg_unpack_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_unpack_tmp");
    LXValue px_err_1127_val = px_null();
    int px_err_1127_proped = 0;
    px_srcline(104);
    px_set_global("cg_unpack_uid", px_add(px_get_global("cg_unpack_uid"), px_int(1LL)));
    px_srcline(105);
    return px_add(px_str("_up"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_unpack_uid")}, 1));
px_err_1127:
    if (px_err_1127_proped) return px_err_1127_val;
    return px_null();
}

static LXValue fn_cg_for_names(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_for_names");
    LXValue _v1128 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1129 = px_null();
    LXValue _v1130 = px_null();
    LXValue px_err_1131_val = px_null();
    int px_err_1131_proped = 0;
    px_srcline(108);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1128}, 1), px_str("string")))) {
        px_srcline(109);
        return px_list_n((LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v1128}, 1)}, 1);
    }
    px_srcline(110);
    _v1129 = px_list_n((LXValue[]){}, 0);
    px_srcline(111);
    _v1130 = px_int(0LL);
    px_srcline(112);
    while (px_is_truthy(px_lt(_v1130, px_call(px_get_global("len"), (LXValue[]){_v1128}, 1)))) {
        px_srcline(113);
        (void)(px_method(_v1129, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1128, _v1130)}, 1)}, 1));
        px_srcline(114);
         _v1130 = px_add(_v1130, px_int(1LL));
    }
    px_srcline(115);
    return _v1129;
px_err_1131:
    if (px_err_1131_proped) return px_err_1131_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_var");
    LXValue _v1132 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1133 = px_null();
    LXValue px_err_1134_val = px_null();
    int px_err_1134_proped = 0;
    px_srcline(117);
    _v1133 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(118);
    px_index_set(px_get_global("cg_vars"), _v1132, _v1133);
    px_srcline(121);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1132}, 1))) {
        px_srcline(122);
        (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v1132}, 1));
    }
    px_srcline(123);
    return _v1133;
px_err_1134:
    if (px_err_1134_proped) return px_err_1134_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_var_of");
    LXValue _v1135 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1136_val = px_null();
    int px_err_1136_proped = 0;
    px_srcline(125);
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1135}, 1))) {
        px_srcline(126);
        return px_index(px_get_global("cg_vars"), _v1135);
    }
    px_srcline(127);
    return px_null();
px_err_1136:
    if (px_err_1136_proped) return px_err_1136_val;
    return px_null();
}

static LXValue fn_cg_load_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_of");
    LXValue _v1137 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1138 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1139_val = px_null();
    int px_err_1139_proped = 0;
    px_srcline(132);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1137}, 1))) {
        px_srcline(133);
        return px_add(px_add(px_str("px_cell_get("), _v1138), px_str(")"));
    }
    px_srcline(134);
    return _v1138;
px_err_1139:
    if (px_err_1139_proped) return px_err_1139_val;
    return px_null();
}

static LXValue fn_cg_store_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_store_of");
    LXValue _v1140 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1141 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1142 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1143_val = px_null();
    int px_err_1143_proped = 0;
    px_srcline(136);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1140}, 1))) {
        px_srcline(137);
        return px_add(px_add(px_add(px_add(px_str("px_cell_set("), _v1141), px_str(", ")), _v1142), px_str(")"));
    }
    px_srcline(138);
    return px_add(px_add(_v1141, px_str(" = ")), _v1142);
px_err_1143:
    if (px_err_1143_proped) return px_err_1143_val;
    return px_null();
}

static LXValue fn_cg_name_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_name_add");
    LXValue _v1144 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1145 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1146_val = px_null();
    int px_err_1146_proped = 0;
    px_srcline(140);
    if (px_is_truthy(({ LXValue _t1147 = px_ne(_v1145, px_str("")); px_is_truthy(_t1147) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1144, _v1145}, 2)) : _t1147; }))) {
        px_srcline(141);
        (void)(px_method(_v1144, "append", (LXValue[]){_v1145}, 1));
    }
px_err_1146:
    if (px_err_1146_proped) return px_err_1146_val;
    return px_null();
}

static LXValue fn_cg_ast_used(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_used");
    LXValue _v1148 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1149 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1150 = px_null();
    LXValue px_err_1151_val = px_null();
    int px_err_1151_proped = 0;
    px_srcline(144);
    if (px_is_truthy(({ LXValue _t1152 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1148}, 1), px_str("list")); px_is_truthy(_t1152) ? _t1152 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1148}, 1), px_int(0LL)); }))) {
        px_srcline(145);
        return px_null();
    }
    px_srcline(146);
    if (px_is_truthy(({ LXValue _t1155 = ({ LXValue _t1154 = ({ LXValue _t1153 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1148, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1153) ? px_eq(px_index(_v1148, px_int(0LL)), px_str("Var")) : _t1153; }); px_is_truthy(_t1154) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1148}, 1), px_int(2LL)) : _t1154; }); px_is_truthy(_t1155) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1148, px_int(1LL))}, 1), px_str("string")) : _t1155; }))) {
        px_srcline(147);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1149, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1148, px_int(1LL))}, 1)}, 2));
        px_srcline(148);
        return px_null();
    }
    px_srcline(149);
    LXValue _t1156 = _v1148;
    int _il7 = (int)px_len(_t1156);
    for (int _t1157 = 0; _t1157 < _il7; _t1157++) {
        px_iter_ck(_t1156, _il7);
        _v1150 = px_iter_at(_t1156, px_int(_t1157));
        px_srcline(150);
        (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1150, _v1149}, 2));
    }
px_err_1151:
    if (px_err_1151_proped) return px_err_1151_val;
    return px_null();
}

static LXValue fn_cg_ast_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_bound");
    LXValue _v1158 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1159 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1160 = px_null();
    LXValue _v1161 = px_null();
    LXValue _v1162 = px_null();
    LXValue _v1163 = px_null();
    LXValue _v1164 = px_null();
    LXValue px_err_1165_val = px_null();
    int px_err_1165_proped = 0;
    px_srcline(153);
    if (px_is_truthy(({ LXValue _t1166 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1158}, 1), px_str("list")); px_is_truthy(_t1166) ? _t1166 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1158}, 1), px_int(0LL)); }))) {
        px_srcline(154);
        return px_null();
    }
    px_srcline(155);
    _v1160 = px_index(_v1158, px_int(0LL));
    px_srcline(156);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1160}, 1), px_str("string")))) {
        px_srcline(157);
        if (px_is_truthy(px_eq(_v1160, px_str("VarDecl")))) {
            px_srcline(158);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1159, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1158, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1160, px_str("For")))) {
            px_srcline(171);
            _v1161 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1158, px_int(1LL))}, 1);
            px_srcline(172);
            _v1162 = px_int(0LL);
            px_srcline(173);
            while (px_is_truthy(px_lt(_v1162, px_call(px_get_global("len"), (LXValue[]){_v1161}, 1)))) {
                px_srcline(174);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1159, px_index(_v1161, _v1162)}, 2));
                px_srcline(175);
                 _v1162 = px_add(_v1162, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1160, px_str("Closure")))) {
            px_srcline(177);
            LXValue _t1167 = px_index(_v1158, px_int(1LL));
            int _il8 = (int)px_len(_t1167);
            for (int _t1168 = 0; _t1168 < _il8; _t1168++) {
                px_iter_ck(_t1167, _il8);
                _v1163 = px_iter_at(_t1167, px_int(_t1168));
                px_srcline(178);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1159, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1163, px_int(1LL))}, 1)}, 2));
            }
        }
    }
    px_srcline(179);
    LXValue _t1169 = _v1158;
    int _il9 = (int)px_len(_t1169);
    for (int _t1170 = 0; _t1170 < _il9; _t1170++) {
        px_iter_ck(_t1169, _il9);
        _v1164 = px_iter_at(_t1169, px_int(_t1170));
        px_srcline(180);
        (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){_v1164, _v1159}, 2));
    }
px_err_1165:
    if (px_err_1165_proped) return px_err_1165_val;
    return px_null();
}

static LXValue fn_cg_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_closure_caps");
    LXValue _v1171 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1172 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1173 = px_null();
    LXValue _v1174 = px_null();
    LXValue _v1175 = px_null();
    LXValue _v1176 = px_null();
    LXValue px_err_1177_val = px_null();
    int px_err_1177_proped = 0;
    px_srcline(183);
    _v1173 = px_list_n((LXValue[]){}, 0);
    px_srcline(184);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){px_index(_v1171, px_int(3LL)), _v1173}, 2));
    px_srcline(185);
    _v1174 = px_list_n((LXValue[]){}, 0);
    px_srcline(186);
    (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){px_index(_v1171, px_int(3LL)), _v1174}, 2));
    px_srcline(187);
    LXValue _t1178 = px_index(_v1171, px_int(1LL));
    int _il10 = (int)px_len(_t1178);
    for (int _t1179 = 0; _t1179 < _il10; _t1179++) {
        px_iter_ck(_t1178, _il10);
        _v1175 = px_iter_at(_t1178, px_int(_t1179));
        px_srcline(188);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1174, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1175, px_int(1LL))}, 1)}, 2));
    }
    px_srcline(189);
    LXValue _t1180 = _v1173;
    int _il11 = (int)px_len(_t1180);
    for (int _t1181 = 0; _t1181 < _il11; _t1181++) {
        px_iter_ck(_t1180, _il11);
        _v1176 = px_iter_at(_t1180, px_int(_t1181));
        px_srcline(190);
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1174, _v1176}, 2)))) {
            px_srcline(191);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1172, _v1176}, 2));
        }
    }
px_err_1177:
    if (px_err_1177_proped) return px_err_1177_val;
    return px_null();
}

static LXValue fn_cg_scan_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_scan_closure_caps");
    LXValue _v1182 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1183 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1184 = px_null();
    LXValue px_err_1185_val = px_null();
    int px_err_1185_proped = 0;
    px_srcline(194);
    if (px_is_truthy(({ LXValue _t1186 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1182}, 1), px_str("list")); px_is_truthy(_t1186) ? _t1186 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1182}, 1), px_int(0LL)); }))) {
        px_srcline(195);
        return px_null();
    }
    px_srcline(196);
    if (px_is_truthy(({ LXValue _t1187 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1182, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1187) ? px_eq(px_index(_v1182, px_int(0LL)), px_str("Closure")) : _t1187; }))) {
        px_srcline(197);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){_v1182, _v1183}, 2));
    }
    px_srcline(198);
    if (px_is_truthy(({ LXValue _t1188 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1182, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1188) ? px_eq(px_index(_v1182, px_int(0LL)), px_str("FuncDef")) : _t1188; }))) {
        px_srcline(202);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), px_index(_v1182, px_int(2LL)), px_index(_v1182, px_int(3LL)), px_index(_v1182, px_int(4LL)), px_list_n((LXValue[]){}, 0), px_index(_v1182, px_int(5LL))}, 6), _v1183}, 2));
    }
    px_srcline(203);
    LXValue _t1189 = _v1182;
    int _il12 = (int)px_len(_t1189);
    for (int _t1190 = 0; _t1190 < _il12; _t1190++) {
        px_iter_ck(_t1189, _il12);
        _v1184 = px_iter_at(_t1189, px_int(_t1190));
        px_srcline(204);
        (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1184, _v1183}, 2));
    }
px_err_1185:
    if (px_err_1185_proped) return px_err_1185_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_mark_immutable");
    LXValue _v1191 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1192_val = px_null();
    int px_err_1192_proped = 0;
    px_srcline(207);
    px_index_set(px_get_global("cg_immutables"), _v1191, px_int(1LL));
px_err_1192:
    if (px_err_1192_proped) return px_err_1192_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_immutable");
    LXValue _v1193 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1194_val = px_null();
    int px_err_1194_proped = 0;
    px_srcline(209);
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1193}, 1);
px_err_1194:
    if (px_err_1194_proped) return px_err_1194_val;
    return px_null();
}

static LXValue fn_cg_is_wildcard(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_wildcard");
    LXValue _v1195 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1196_val = px_null();
    int px_err_1196_proped = 0;
    px_srcline(217);
    return px_eq(_v1195, px_str("_"));
px_err_1196:
    if (px_err_1196_proped) return px_err_1196_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_perr");
    LXValue _v1197 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1198 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1199_val = px_null();
    int px_err_1199_proped = 0;
    px_srcline(222);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1197), px_str(": ")), _v1198)}, 1));
    px_srcline(223);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1199:
    if (px_err_1199_proped) return px_err_1199_val;
    return px_null();
}

static LXValue fn_cg_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pwarn");
    LXValue _v1200 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1201_val = px_null();
    int px_err_1201_proped = 0;
    px_srcline(226);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1200)}, 1));
px_err_1201:
    if (px_err_1201_proped) return px_err_1201_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_nonnull_ty");
    LXValue _v1202 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1203_val = px_null();
    int px_err_1203_proped = 0;
    px_srcline(230);
    if (px_is_truthy(px_eq(_v1202, px_null()))) {
        px_srcline(231);
        return px_bool(false);
    }
    px_srcline(232);
    if (px_is_truthy(px_eq(px_index(_v1202, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(233);
        return px_bool(false);
    }
    px_srcline(234);
    return px_bool(true);
px_err_1203:
    if (px_err_1203_proped) return px_err_1203_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_null_lit");
    LXValue _v1204 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1205_val = px_null();
    int px_err_1205_proped = 0;
    px_srcline(237);
    if (px_is_truthy(px_eq(_v1204, px_null()))) {
        px_srcline(238);
        return px_bool(false);
    }
    px_srcline(239);
    if (px_is_truthy(px_eq(px_index(_v1204, px_int(0LL)), px_str("Null")))) {
        px_srcline(240);
        return px_bool(true);
    }
    px_srcline(241);
    return px_bool(false);
px_err_1205:
    if (px_err_1205_proped) return px_err_1205_val;
    return px_null();
}

static LXValue fn_cg_sem_vardecl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_vardecl");
    LXValue _v1206 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1207 = px_null();
    LXValue _v1208 = px_null();
    LXValue px_err_1209_val = px_null();
    int px_err_1209_proped = 0;
    px_srcline(259);
    _v1207 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1206, px_int(2LL))}, 1);
    px_srcline(261);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1207}, 1))) {
        px_srcline(262);
        return px_null();
    }
    px_srcline(263);
    if (px_is_truthy(({ LXValue _t1210 = px_eq(px_index(_v1206, px_int(1LL)), px_str("Let")); px_is_truthy(_t1210) ? _t1210 : px_eq(px_index(_v1206, px_int(1LL)), px_str("Const")); }))) {
        px_srcline(264);
        (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v1207}, 1));
    }
    px_srcline(265);
    _v1208 = px_index(_v1206, px_int(3LL));
    px_srcline(266);
    if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v1208}, 1))) {
        px_srcline(267);
        px_index_set(px_get_global("cg_nonnull"), _v1207, px_int(1LL));
        px_srcline(268);
        if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v1206, px_int(4LL))}, 1))) {
            px_srcline(269);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(({ LXValue _s189 = px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1208}, 1)), px_str("'（可空类型请用 ")); LXValue _s190 = px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1208}, 1); px_add(_s189, _s190); }), px_str("? 声明）"))}, 2));
        }
    }
px_err_1209:
    if (px_err_1209_proped) return px_err_1209_val;
    return px_null();
}

static LXValue fn_cg_sem_assign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_assign");
    LXValue _v1211 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1212 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1213 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1214 = px_null();
    LXValue px_err_1215_val = px_null();
    int px_err_1215_proped = 0;
    px_srcline(273);
    if (px_is_truthy(px_eq(_v1211, px_null()))) {
        px_srcline(274);
        return px_null();
    }
    px_srcline(275);
    if (px_is_truthy(px_ne(px_index(_v1211, px_int(0LL)), px_str("Var")))) {
        px_srcline(276);
        return px_null();
    }
    px_srcline(277);
    _v1214 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1211, px_int(1LL))}, 1);
    px_srcline(279);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1214}, 1))) {
        px_srcline(280);
        return px_null();
    }
    px_srcline(281);
    if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v1214}, 1))) {
        px_srcline(282);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v1214), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
    }
    px_srcline(283);
    if (px_is_truthy(({ LXValue _t1216 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){_v1213}, 1); px_is_truthy(_t1216) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v1214}, 1) : _t1216; }))) {
        px_srcline(284);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v1214), px_str("'（可空类型请声明为 ")), _v1214), px_str(": T?）"))}, 2));
    }
px_err_1215:
    if (px_err_1215_proped) return px_err_1215_val;
    return px_null();
}

static LXValue fn_cg_sem_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_call");
    LXValue _v1217 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1218 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1219 = px_null();
    LXValue _v1220 = px_null();
    LXValue px_err_1221_val = px_null();
    int px_err_1221_proped = 0;
    px_srcline(287);
    if (px_is_truthy(px_eq(_v1217, px_null()))) {
        px_srcline(288);
        return px_null();
    }
    px_srcline(289);
    if (px_is_truthy(px_ne(px_index(_v1217, px_int(0LL)), px_str("Var")))) {
        px_srcline(290);
        return px_null();
    }
    px_srcline(291);
    _v1219 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1217, px_int(1LL))}, 1);
    px_srcline(292);
    if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v1219}, 1))) {
        px_srcline(293);
        _v1220 = px_index(px_get_global("cg_ffi"), _v1219);
        px_srcline(294);
        if (px_is_truthy(({ LXValue _s191 = px_call(px_get_global("len"), (LXValue[]){_v1218}, 1); LXValue _s192 = px_call(px_get_global("len"), (LXValue[]){_v1220}, 1); px_ne(_s191, _s192); }))) {
            px_srcline(295);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), ({ LXValue _s193 = px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v1219), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1220}, 1)}, 1)), px_str(" 个参数，给出 ")); LXValue _s194 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1218}, 1)}, 1); px_add(_s193, _s194); })}, 2));
        }
    }
px_err_1221:
    if (px_err_1221_proped) return px_err_1221_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ty_name");
    LXValue _v1222 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1223_val = px_null();
    int px_err_1223_proped = 0;
    px_srcline(298);
    if (px_is_truthy(px_eq(_v1222, px_null()))) {
        px_srcline(299);
        return px_str("any");
    }
    px_srcline(300);
    if (px_is_truthy(px_eq(px_index(_v1222, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(301);
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1222, px_int(1LL))}, 1), px_str("?"));
    }
    px_srcline(302);
    if (px_is_truthy(px_eq(px_index(_v1222, px_int(0LL)), px_str("TyNamed")))) {
        px_srcline(303);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1222, px_int(1LL))}, 1);
    }
    px_srcline(304);
    if (px_is_truthy(px_eq(px_index(_v1222, px_int(0LL)), px_str("TyList")))) {
        px_srcline(305);
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1222, px_int(1LL))}, 1)), px_str("]"));
    }
    px_srcline(306);
    if (px_is_truthy(px_eq(px_index(_v1222, px_int(0LL)), px_str("TyDict")))) {
        px_srcline(307);
        return px_add(({ LXValue _s195 = px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1222, px_int(1LL))}, 1)), px_str(": ")); LXValue _s196 = px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1222, px_int(2LL))}, 1); px_add(_s195, _s196); }), px_str("}"));
    }
    px_srcline(308);
    return px_str("any");
px_err_1223:
    if (px_err_1223_proped) return px_err_1223_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_func_cname");
    LXValue _v1224 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1225 = px_null();
    LXValue _v1226 = px_null();
    LXValue _v1227 = px_null();
    LXValue _v1228 = px_null();
    LXValue _v1229 = px_null();
    LXValue _v1230 = px_null();
    LXValue px_err_1231_val = px_null();
    int px_err_1231_proped = 0;
    px_srcline(310);
    _v1225 = px_list_n((LXValue[]){}, 0);
    px_srcline(311);
    _v1226 = px_int(0LL);
    px_srcline(312);
    while (px_is_truthy(px_lt(_v1226, px_call(px_get_global("len"), (LXValue[]){_v1224}, 1)))) {
        px_srcline(313);
        _v1227 = px_index(_v1224, _v1226);
        px_srcline(314);
        _v1228 = ({ LXValue _t1232 = px_ge(_v1227, px_str("a")); px_is_truthy(_t1232) ? px_le(_v1227, px_str("z")) : _t1232; });
        px_srcline(315);
        _v1229 = ({ LXValue _t1233 = px_ge(_v1227, px_str("A")); px_is_truthy(_t1233) ? px_le(_v1227, px_str("Z")) : _t1233; });
        px_srcline(316);
        _v1230 = ({ LXValue _t1234 = px_ge(_v1227, px_str("0")); px_is_truthy(_t1234) ? px_le(_v1227, px_str("9")) : _t1234; });
        px_srcline(317);
        if (px_is_truthy(({ LXValue _t1236 = ({ LXValue _t1235 = _v1228; px_is_truthy(_t1235) ? _t1235 : _v1229; }); px_is_truthy(_t1236) ? _t1236 : _v1230; }))) {
            px_srcline(318);
            (void)(px_method(_v1225, "append", (LXValue[]){_v1227}, 1));
        }
        else {
            px_srcline(320);
            (void)(px_method(_v1225, "append", (LXValue[]){px_str("_")}, 1));
        }
        px_srcline(321);
         _v1226 = px_add(_v1226, px_int(1LL));
    }
    px_srcline(322);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1225}, 2);
px_err_1231:
    if (px_err_1231_proped) return px_err_1231_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find");
    LXValue _v1237 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1238 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1239 = px_null();
    LXValue _v1240 = px_null();
    LXValue _v1241 = px_null();
    LXValue _v1242 = px_null();
    LXValue _v1243 = px_null();
    LXValue px_err_1244_val = px_null();
    int px_err_1244_proped = 0;
    px_srcline(324);
    _v1239 = px_call(px_get_global("len"), (LXValue[]){_v1237}, 1);
    px_srcline(325);
    _v1240 = px_call(px_get_global("len"), (LXValue[]){_v1238}, 1);
    px_srcline(326);
    _v1241 = px_int(0LL);
    px_srcline(327);
    while (px_is_truthy(px_le(px_add(_v1241, _v1240), _v1239))) {
        px_srcline(328);
        _v1242 = px_int(0LL);
        px_srcline(329);
        _v1243 = px_bool(true);
        px_srcline(330);
        while (px_is_truthy(px_lt(_v1242, _v1240))) {
            px_srcline(331);
            if (px_is_truthy(px_ne(px_index(_v1237, px_add(_v1241, _v1242)), px_index(_v1238, _v1242)))) {
                px_srcline(332);
                 _v1243 = px_bool(false);
                px_srcline(333);
                break;
            }
            px_srcline(334);
             _v1242 = px_add(_v1242, px_int(1LL));
        }
        px_srcline(335);
        if (px_is_truthy(_v1243)) {
            px_srcline(336);
            return _v1241;
        }
        px_srcline(337);
         _v1241 = px_add(_v1241, px_int(1LL));
    }
    px_srcline(338);
    return px_neg(px_int(1LL));
px_err_1244:
    if (px_err_1244_proped) return px_err_1244_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad");
    LXValue _v1245 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1246 = px_null();
    LXValue _v1247 = px_null();
    LXValue px_err_1248_val = px_null();
    int px_err_1248_proped = 0;
    px_srcline(340);
    _v1246 = px_list_n((LXValue[]){}, 0);
    px_srcline(341);
    _v1247 = px_int(0LL);
    px_srcline(342);
    while (px_is_truthy(px_lt(_v1247, _v1245))) {
        px_srcline(343);
        (void)(px_method(_v1246, "append", (LXValue[]){px_str("    ")}, 1));
        px_srcline(344);
         _v1247 = px_add(_v1247, px_int(1LL));
    }
    px_srcline(345);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1246}, 2);
px_err_1248:
    if (px_err_1248_proped) return px_err_1248_val;
    return px_null();
}

static LXValue fn_cg_nul_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_nul_char");
    LXValue px_err_1249_val = px_null();
    int px_err_1249_proped = 0;
    px_srcline(352);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){px_int(0LL), px_int(1LL)}, 2)}, 1);
px_err_1249:
    if (px_err_1249_proped) return px_err_1249_val;
    return px_null();
}

static LXValue fn_cg_has_nul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_has_nul");
    LXValue _v1250 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1251 = px_null();
    LXValue _v1252 = px_null();
    LXValue px_err_1253_val = px_null();
    int px_err_1253_proped = 0;
    px_srcline(355);
    _v1251 = px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0);
    px_srcline(356);
    _v1252 = px_int(0LL);
    px_srcline(357);
    while (px_is_truthy(px_lt(_v1252, px_call(px_get_global("len"), (LXValue[]){_v1250}, 1)))) {
        px_srcline(358);
        if (px_is_truthy(px_eq(px_index(_v1250, _v1252), _v1251))) {
            px_srcline(359);
            return px_bool(true);
        }
        px_srcline(360);
         _v1252 = px_add(_v1252, px_int(1LL));
    }
    px_srcline(361);
    return px_bool(false);
px_err_1253:
    if (px_err_1253_proped) return px_err_1253_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_unescape");
    LXValue _v1254 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1255 = px_null();
    LXValue _v1256 = px_null();
    LXValue _v1257 = px_null();
    LXValue _v1258 = px_null();
    LXValue _v1259 = px_null();
    LXValue _v1260 = px_null();
    LXValue _v1261 = px_null();
    LXValue px_err_1262_val = px_null();
    int px_err_1262_proped = 0;
    px_srcline(363);
    _v1255 = px_slice(_v1254, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1254}, 1), px_int(1LL)), px_null());
    px_srcline(364);
    _v1256 = px_list_n((LXValue[]){}, 0);
    px_srcline(365);
    _v1257 = px_int(0LL);
    px_srcline(366);
    while (px_is_truthy(px_lt(_v1257, px_call(px_get_global("len"), (LXValue[]){_v1255}, 1)))) {
        px_srcline(367);
        _v1258 = px_index(_v1255, _v1257);
        px_srcline(368);
        if (px_is_truthy(px_eq(_v1258, px_str("\\")))) {
            px_srcline(369);
            _v1259 = px_index(_v1255, px_add(_v1257, px_int(1LL)));
            px_srcline(370);
            if (px_is_truthy(px_eq(_v1259, px_str("n")))) {
                px_srcline(371);
                (void)(px_method(_v1256, "append", (LXValue[]){px_str("\n")}, 1));
                px_srcline(372);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1259, px_str("t")))) {
                px_srcline(374);
                (void)(px_method(_v1256, "append", (LXValue[]){px_str("\t")}, 1));
                px_srcline(375);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1259, px_str("r")))) {
                px_srcline(377);
                (void)(px_method(_v1256, "append", (LXValue[]){px_str("\r")}, 1));
                px_srcline(378);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1259, px_str("0")))) {
                px_srcline(380);
                (void)(px_method(_v1256, "append", (LXValue[]){px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)}, 1));
                px_srcline(381);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1259, px_str("\"")))) {
                px_srcline(383);
                (void)(px_method(_v1256, "append", (LXValue[]){px_str("\"")}, 1));
                px_srcline(384);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1259, px_str("\\")))) {
                px_srcline(386);
                (void)(px_method(_v1256, "append", (LXValue[]){px_str("\\")}, 1));
                px_srcline(387);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1259, px_str("u")))) {
                px_srcline(389);
                _v1260 = px_add(_v1257, px_int(3LL));
                px_srcline(390);
                _v1261 = px_list_n((LXValue[]){}, 0);
                px_srcline(391);
                while (px_is_truthy(({ LXValue _t1263 = px_lt(_v1260, px_call(px_get_global("len"), (LXValue[]){_v1255}, 1)); px_is_truthy(_t1263) ? px_ne(px_index(_v1255, _v1260), px_str("}")) : _t1263; }))) {
                    px_srcline(392);
                    (void)(px_method(_v1261, "append", (LXValue[]){px_index(_v1255, _v1260)}, 1));
                    px_srcline(393);
                     _v1260 = px_add(_v1260, px_int(1LL));
                }
                px_srcline(394);
                (void)(px_method(_v1256, "append", (LXValue[]){px_call(px_get_global("hex_to_char"), (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1261}, 2)}, 1)}, 1));
                px_srcline(395);
                 _v1257 = px_add(_v1260, px_int(1LL));
            }
            else {
                px_srcline(397);
                (void)(px_method(_v1256, "append", (LXValue[]){_v1259}, 1));
                px_srcline(398);
                 _v1257 = px_add(_v1257, px_int(2LL));
            }
        }
        else {
            px_srcline(400);
            (void)(px_method(_v1256, "append", (LXValue[]){_v1258}, 1));
            px_srcline(401);
             _v1257 = px_add(_v1257, px_int(1LL));
        }
    }
    px_srcline(402);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1256}, 2);
px_err_1262:
    if (px_err_1262_proped) return px_err_1262_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_escape_str");
    LXValue _v1264 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1265 = px_null();
    LXValue _v1266 = px_null();
    LXValue _v1267 = px_null();
    LXValue px_err_1268_val = px_null();
    int px_err_1268_proped = 0;
    px_srcline(405);
    _v1265 = px_list_n((LXValue[]){}, 0);
    px_srcline(406);
    _v1266 = px_int(0LL);
    px_srcline(407);
    while (px_is_truthy(px_lt(_v1266, px_call(px_get_global("len"), (LXValue[]){_v1264}, 1)))) {
        px_srcline(408);
        _v1267 = px_index(_v1264, _v1266);
        px_srcline(409);
        if (px_is_truthy(px_eq(_v1267, px_str("\\")))) {
            px_srcline(410);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("\\\\")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1267, px_str("\"")))) {
            px_srcline(412);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("\\\"")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1267, px_str("\n")))) {
            px_srcline(414);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("\\n")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1267, px_str("\r")))) {
            px_srcline(416);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("\\r")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1267, px_str("\t")))) {
            px_srcline(418);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("\\t")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1267, px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)))) {
            px_srcline(426);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("\\000")}, 1));
        }
        else {
            px_srcline(428);
            (void)(px_method(_v1265, "append", (LXValue[]){_v1267}, 1));
        }
        px_srcline(429);
         _v1266 = px_add(_v1266, px_int(1LL));
    }
    px_srcline(430);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1265}, 2);
px_err_1268:
    if (px_err_1268_proped) return px_err_1268_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad_zeros");
    LXValue _v1269 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1270 = px_null();
    LXValue _v1271 = px_null();
    LXValue px_err_1272_val = px_null();
    int px_err_1272_proped = 0;
    px_srcline(433);
    _v1270 = px_list_n((LXValue[]){}, 0);
    px_srcline(434);
    _v1271 = px_int(0LL);
    px_srcline(435);
    while (px_is_truthy(px_lt(_v1271, _v1269))) {
        px_srcline(436);
        (void)(px_method(_v1270, "append", (LXValue[]){px_str("0")}, 1));
        px_srcline(437);
         _v1271 = px_add(_v1271, px_int(1LL));
    }
    px_srcline(438);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1270}, 2);
px_err_1272:
    if (px_err_1272_proped) return px_err_1272_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_expand_sci");
    LXValue _v1273 = (nargs > 0) ? args[0] : px_null();
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
    LXValue px_err_1288_val = px_null();
    int px_err_1288_proped = 0;
    px_srcline(440);
    _v1274 = px_neg(px_int(1LL));
    px_srcline(441);
    _v1275 = px_int(0LL);
    px_srcline(442);
    while (px_is_truthy(px_lt(_v1275, px_call(px_get_global("len"), (LXValue[]){_v1273}, 1)))) {
        px_srcline(443);
        if (px_is_truthy(({ LXValue _t1289 = px_eq(px_index(_v1273, _v1275), px_str("e")); px_is_truthy(_t1289) ? _t1289 : px_eq(px_index(_v1273, _v1275), px_str("E")); }))) {
            px_srcline(444);
             _v1274 = _v1275;
            px_srcline(445);
            break;
        }
        px_srcline(446);
         _v1275 = px_add(_v1275, px_int(1LL));
    }
    px_srcline(447);
    if (px_is_truthy(px_lt(_v1274, px_int(0LL)))) {
        px_srcline(448);
        return _v1273;
    }
    px_srcline(449);
    _v1276 = px_slice(_v1273, px_int(0LL), _v1274, px_null());
    px_srcline(450);
    _v1277 = px_slice(_v1273, px_add(_v1274, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1273}, 1), px_null());
    px_srcline(451);
    _v1278 = px_int(1LL);
    px_srcline(452);
    if (px_is_truthy(({ LXValue _t1290 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1277}, 1), px_int(0LL)); px_is_truthy(_t1290) ? px_eq(px_index(_v1277, px_int(0LL)), px_str("+")) : _t1290; }))) {
        px_srcline(453);
         _v1277 = px_slice(_v1277, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1277}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1291 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1277}, 1), px_int(0LL)); px_is_truthy(_t1291) ? px_eq(px_index(_v1277, px_int(0LL)), px_str("-")) : _t1291; }))) {
        px_srcline(455);
         _v1278 = px_neg(px_int(1LL));
        px_srcline(456);
         _v1277 = px_slice(_v1277, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1277}, 1), px_null());
    }
    px_srcline(457);
    _v1279 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1277}, 1), _v1278);
    px_srcline(458);
    _v1280 = px_bool(false);
    px_srcline(459);
    if (px_is_truthy(({ LXValue _t1292 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1276}, 1), px_int(0LL)); px_is_truthy(_t1292) ? px_eq(px_index(_v1276, px_int(0LL)), px_str("-")) : _t1292; }))) {
        px_srcline(460);
         _v1280 = px_bool(true);
        px_srcline(461);
         _v1276 = px_slice(_v1276, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1276}, 1), px_null());
    }
    px_srcline(462);
    _v1281 = px_str("");
    px_srcline(463);
    _v1282 = px_str("");
    px_srcline(464);
    _v1283 = px_neg(px_int(1LL));
    px_srcline(465);
    _v1284 = px_int(0LL);
    px_srcline(466);
    while (px_is_truthy(px_lt(_v1284, px_call(px_get_global("len"), (LXValue[]){_v1276}, 1)))) {
        px_srcline(467);
        if (px_is_truthy(px_eq(px_index(_v1276, _v1284), px_str(".")))) {
            px_srcline(468);
             _v1283 = _v1284;
            px_srcline(469);
            break;
        }
        px_srcline(470);
         _v1284 = px_add(_v1284, px_int(1LL));
    }
    px_srcline(471);
    if (px_is_truthy(px_lt(_v1283, px_int(0LL)))) {
        px_srcline(472);
         _v1281 = _v1276;
    }
    else {
        px_srcline(474);
         _v1281 = px_slice(_v1276, px_int(0LL), _v1283, px_null());
        px_srcline(475);
         _v1282 = px_slice(_v1276, px_add(_v1283, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1276}, 1), px_null());
    }
    px_srcline(476);
    _v1285 = px_add(_v1281, _v1282);
    px_srcline(477);
    _v1286 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1281}, 1), _v1279);
    px_srcline(478);
    _v1287 = px_str("");
    px_srcline(479);
    if (px_is_truthy(px_le(_v1286, px_int(0LL)))) {
        px_srcline(480);
         _v1287 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1286)}, 1)), _v1285);
    }
    else if (px_is_truthy(px_ge(_v1286, px_call(px_get_global("len"), (LXValue[]){_v1285}, 1)))) {
        px_srcline(482);
         _v1287 = px_add(_v1285, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1286, px_call(px_get_global("len"), (LXValue[]){_v1285}, 1))}, 1));
    }
    else {
        px_srcline(484);
         _v1287 = px_add(px_add(px_slice(_v1285, px_int(0LL), _v1286, px_null()), px_str(".")), px_slice(_v1285, _v1286, px_call(px_get_global("len"), (LXValue[]){_v1285}, 1), px_null()));
    }
    px_srcline(485);
    if (px_is_truthy(_v1280)) {
        px_srcline(486);
        return px_add(px_str("-"), _v1287);
    }
    px_srcline(487);
    return _v1287;
px_err_1288:
    if (px_err_1288_proped) return px_err_1288_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_fmt_float");
    LXValue _v1293 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1294 = px_null();
    LXValue _v1295 = px_null();
    LXValue _v1296 = px_null();
    LXValue px_err_1297_val = px_null();
    int px_err_1297_proped = 0;
    px_srcline(490);
    _v1294 = px_call(px_get_global("str"), (LXValue[]){_v1293}, 1);
    px_srcline(491);
    if (px_is_truthy(({ LXValue _t1300 = ({ LXValue _t1299 = ({ LXValue _t1298 = px_eq(_v1294, px_str("inf")); px_is_truthy(_t1298) ? _t1298 : px_eq(_v1294, px_str("-inf")); }); px_is_truthy(_t1299) ? _t1299 : px_eq(_v1294, px_str("nan")); }); px_is_truthy(_t1300) ? _t1300 : px_eq(_v1294, px_str("-nan")); }))) {
        px_srcline(492);
        return _v1294;
    }
    px_srcline(493);
     _v1294 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1294}, 1);
    px_srcline(494);
    _v1295 = px_call(px_get_global("len"), (LXValue[]){_v1294}, 1);
    px_srcline(495);
    if (px_is_truthy(({ LXValue _t1301 = px_ge(_v1295, px_int(2LL)); px_is_truthy(_t1301) ? px_eq(px_slice(_v1294, px_sub(_v1295, px_int(2LL)), _v1295, px_null()), px_str(".0")) : _t1301; }))) {
        px_srcline(501);
        _v1296 = px_slice(_v1294, px_int(0LL), px_sub(_v1295, px_int(2LL)), px_null());
        px_srcline(502);
        if (px_is_truthy(px_eq(_v1296, px_str("-0")))) {
            px_srcline(503);
            return _v1294;
        }
        px_srcline(504);
        return _v1296;
    }
    px_srcline(512);
    if (px_is_truthy(({ LXValue _t1302 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1294, px_str(".")}, 2)); px_is_truthy(_t1302) ? px_ge(_v1295, px_int(19LL)) : _t1302; }))) {
        px_srcline(513);
        return px_add(_v1294, px_str(".0"));
    }
    px_srcline(514);
    return _v1294;
px_err_1297:
    if (px_err_1297_proped) return px_err_1297_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_types");
    LXValue _v1303 = (nargs > 0) ? args[0] : px_null();
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
    px_srcline(517);
    _v1304 = px_index(_v1303, px_int(1LL));
    px_srcline(518);
    _v1305 = px_int(0LL);
    px_srcline(519);
    while (px_is_truthy(px_lt(_v1305, px_call(px_get_global("len"), (LXValue[]){_v1304}, 1)))) {
        px_srcline(520);
        _v1306 = px_index(_v1304, _v1305);
        px_srcline(521);
        _v1307 = px_index(_v1306, px_int(0LL));
        px_srcline(522);
        if (px_is_truthy(px_eq(_v1307, px_str("StructDef")))) {
            px_srcline(523);
            _v1308 = px_list_n((LXValue[]){}, 0);
            px_srcline(524);
            _v1309 = px_int(0LL);
            px_srcline(525);
            while (px_is_truthy(px_lt(_v1309, px_call(px_get_global("len"), (LXValue[]){px_index(_v1306, px_int(2LL))}, 1)))) {
                px_srcline(526);
                (void)(px_method(_v1308, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1306, px_int(2LL)), _v1309), px_int(1LL))}, 1)}, 1));
                px_srcline(527);
                 _v1309 = px_add(_v1309, px_int(1LL));
            }
            px_srcline(528);
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1306, px_int(1LL))}, 1), _v1308);
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("EnumDef")))) {
            px_srcline(530);
            _v1310 = px_list_n((LXValue[]){}, 0);
            px_srcline(531);
            _v1311 = px_int(0LL);
            px_srcline(532);
            while (px_is_truthy(px_lt(_v1311, px_call(px_get_global("len"), (LXValue[]){px_index(_v1306, px_int(2LL))}, 1)))) {
                px_srcline(533);
                (void)(px_method(_v1310, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1306, px_int(2LL)), _v1311), px_int(1LL))}, 1)}, 1));
                px_srcline(534);
                 _v1311 = px_add(_v1311, px_int(1LL));
            }
            px_srcline(535);
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1306, px_int(1LL))}, 1), _v1310);
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("ImplDef")))) {
            px_srcline(537);
            _v1312 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1306, px_int(1LL))}, 1);
            px_srcline(538);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1312}, 1))) {
                px_srcline(539);
                _v1313 = px_int(0LL);
                px_srcline(540);
                while (px_is_truthy(px_lt(_v1313, px_call(px_get_global("len"), (LXValue[]){px_index(_v1306, px_int(3LL))}, 1)))) {
                    px_srcline(541);
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1312), "append", (LXValue[]){px_index(px_index(_v1306, px_int(3LL)), _v1313)}, 1));
                    px_srcline(542);
                     _v1313 = px_add(_v1313, px_int(1LL));
                }
            }
            else {
                px_srcline(544);
                px_index_set(px_get_global("cg_impls"), _v1312, px_index(_v1306, px_int(3LL)));
            }
        }
        px_srcline(545);
         _v1305 = px_add(_v1305, px_int(1LL));
    }
    px_srcline(548);
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1304}, 1));
px_err_1314:
    if (px_err_1314_proped) return px_err_1314_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_consts");
    LXValue _v1315 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1316 = px_null();
    LXValue _v1317 = px_null();
    LXValue _v1318 = px_null();
    LXValue _v1319 = px_null();
    LXValue _v1320 = px_null();
    LXValue _v1321 = px_null();
    LXValue _v1322 = px_null();
    LXValue _v1323 = px_null();
    LXValue px_err_1324_val = px_null();
    int px_err_1324_proped = 0;
    px_srcline(551);
    _v1316 = px_int(0LL);
    px_srcline(552);
    while (px_is_truthy(px_lt(_v1316, px_call(px_get_global("len"), (LXValue[]){_v1315}, 1)))) {
        px_srcline(553);
        _v1317 = px_index(_v1315, _v1316);
        px_srcline(554);
        _v1318 = px_index(_v1317, px_int(0LL));
        px_srcline(555);
        if (px_is_truthy(px_eq(_v1318, px_str("TypeConst")))) {
            px_srcline(556);
            _v1319 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1317, px_int(1LL))}, 1);
            px_srcline(557);
            _v1320 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            px_srcline(558);
            _v1321 = px_int(0LL);
            px_srcline(559);
            while (px_is_truthy(px_lt(_v1321, px_call(px_get_global("len"), (LXValue[]){px_index(_v1317, px_int(2LL))}, 1)))) {
                px_srcline(560);
                _v1322 = px_index(px_index(_v1317, px_int(2LL)), _v1321);
                px_srcline(561);
                ({ LXValue _s197 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1322, px_int(1LL))}, 1); LXValue _s198 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1322, px_int(2LL))}, 1); px_index_set(_v1320, _s197, _s198); });
                px_srcline(562);
                 _v1321 = px_add(_v1321, px_int(1LL));
            }
            px_srcline(563);
            px_index_set(px_get_global("cg_const_enums"), _v1319, _v1320);
        }
        else if (px_is_truthy(px_eq(_v1318, px_str("FuncDef")))) {
            px_srcline(565);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1317, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1318, px_str("If")))) {
            px_srcline(567);
            _v1323 = px_int(0LL);
            px_srcline(568);
            while (px_is_truthy(px_lt(_v1323, px_call(px_get_global("len"), (LXValue[]){px_index(_v1317, px_int(1LL))}, 1)))) {
                px_srcline(569);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1317, px_int(1LL)), _v1323), px_int(1LL))}, 1));
                px_srcline(570);
                 _v1323 = px_add(_v1323, px_int(1LL));
            }
            px_srcline(571);
            if (px_is_truthy(px_ne(px_index(_v1317, px_int(2LL)), px_null()))) {
                px_srcline(572);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1317, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1318, px_str("For")))) {
            px_srcline(574);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1317, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1318, px_str("While")))) {
            px_srcline(576);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1317, px_int(2LL))}, 1));
        }
        px_srcline(577);
         _v1316 = px_add(_v1316, px_int(1LL));
    }
px_err_1324:
    if (px_err_1324_proped) return px_err_1324_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_hoist_vars");
    LXValue _v1325 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1326 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1327 = px_null();
    LXValue _v1328 = px_null();
    LXValue _v1329 = px_null();
    LXValue _v1330 = px_null();
    LXValue _v1331 = px_null();
    LXValue _v1332 = px_null();
    LXValue _v1333 = px_null();
    LXValue _v1334 = px_null();
    LXValue _v1335 = px_null();
    LXValue px_err_1336_val = px_null();
    int px_err_1336_proped = 0;
    px_srcline(586);
    _v1327 = px_int(0LL);
    px_srcline(587);
    while (px_is_truthy(px_lt(_v1327, px_call(px_get_global("len"), (LXValue[]){_v1325}, 1)))) {
        px_srcline(588);
        _v1328 = px_index(_v1325, _v1327);
        px_srcline(589);
        _v1329 = px_index(_v1328, px_int(0LL));
        px_srcline(590);
        if (px_is_truthy(px_eq(_v1329, px_str("Assign")))) {
            px_srcline(591);
            _v1330 = px_index(_v1328, px_int(1LL));
            px_srcline(592);
            if (px_is_truthy(px_eq(px_index(_v1330, px_int(0LL)), px_str("Var")))) {
                px_srcline(593);
                _v1331 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1330, px_int(1LL))}, 1);
                px_srcline(594);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1326, _v1331}, 2)))) {
                    px_srcline(595);
                    (void)(px_method(_v1326, "append", (LXValue[]){_v1331}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("VarDecl")))) {
            px_srcline(597);
            _v1331 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1328, px_int(2LL))}, 1);
            px_srcline(598);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1326, _v1331}, 2)))) {
                px_srcline(599);
                (void)(px_method(_v1326, "append", (LXValue[]){_v1331}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("FuncDef")))) {
            px_srcline(604);
            _v1331 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1328, px_int(1LL))}, 1);
            px_srcline(605);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1326, _v1331}, 2)))) {
                px_srcline(606);
                (void)(px_method(_v1326, "append", (LXValue[]){_v1331}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("If")))) {
            px_srcline(608);
            _v1332 = px_index(_v1328, px_int(1LL));
            px_srcline(609);
            _v1333 = px_int(0LL);
            px_srcline(610);
            while (px_is_truthy(px_lt(_v1333, px_call(px_get_global("len"), (LXValue[]){_v1332}, 1)))) {
                px_srcline(611);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1332, _v1333), px_int(1LL)), _v1326}, 2));
                px_srcline(612);
                 _v1333 = px_add(_v1333, px_int(1LL));
            }
            px_srcline(613);
            if (px_is_truthy(px_ne(px_index(_v1328, px_int(2LL)), px_null()))) {
                px_srcline(614);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1328, px_int(2LL)), _v1326}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("For")))) {
            px_srcline(617);
            _v1334 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1328, px_int(1LL))}, 1);
            px_srcline(618);
            _v1335 = px_int(0LL);
            px_srcline(619);
            while (px_is_truthy(px_lt(_v1335, px_call(px_get_global("len"), (LXValue[]){_v1334}, 1)))) {
                px_srcline(620);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1326, px_index(_v1334, _v1335)}, 2)))) {
                    px_srcline(621);
                    (void)(px_method(_v1326, "append", (LXValue[]){px_index(_v1334, _v1335)}, 1));
                }
                px_srcline(622);
                 _v1335 = px_add(_v1335, px_int(1LL));
            }
            px_srcline(623);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1328, px_int(3LL)), _v1326}, 2));
        }
        else if (px_is_truthy(px_eq(_v1329, px_str("While")))) {
            px_srcline(625);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1328, px_int(2LL)), _v1326}, 2));
        }
        px_srcline(626);
         _v1327 = px_add(_v1327, px_int(1LL));
    }
px_err_1336:
    if (px_err_1336_proped) return px_err_1336_val;
    return px_null();
}

static LXValue fn_cg_collect_decl_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_decl_vars");
    LXValue _v1337 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1338 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1339 = px_null();
    LXValue _v1340 = px_null();
    LXValue _v1341 = px_null();
    LXValue _v1342 = px_null();
    LXValue _v1343 = px_null();
    LXValue _v1344 = px_null();
    LXValue _v1345 = px_null();
    LXValue _v1346 = px_null();
    LXValue _v1347 = px_null();
    LXValue px_err_1348_val = px_null();
    int px_err_1348_proped = 0;
    px_srcline(637);
    _v1339 = px_int(0LL);
    px_srcline(638);
    while (px_is_truthy(px_lt(_v1339, px_call(px_get_global("len"), (LXValue[]){_v1337}, 1)))) {
        px_srcline(639);
        _v1340 = px_index(_v1337, _v1339);
        px_srcline(640);
        _v1341 = px_index(_v1340, px_int(0LL));
        px_srcline(641);
        if (px_is_truthy(px_eq(_v1341, px_str("VarDecl")))) {
            px_srcline(642);
            _v1342 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1340, px_int(2LL))}, 1);
            px_srcline(643);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1338, _v1342}, 2)))) {
                px_srcline(644);
                (void)(px_method(_v1338, "append", (LXValue[]){_v1342}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1341, px_str("FuncDef")))) {
            px_srcline(646);
            _v1342 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1340, px_int(1LL))}, 1);
            px_srcline(647);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1338, _v1342}, 2)))) {
                px_srcline(648);
                (void)(px_method(_v1338, "append", (LXValue[]){_v1342}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1341, px_str("ChanDecl")))) {
            px_srcline(650);
            _v1343 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1340, px_int(1LL))}, 1);
            px_srcline(651);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1338, _v1343}, 2)))) {
                px_srcline(652);
                (void)(px_method(_v1338, "append", (LXValue[]){_v1343}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1341, px_str("If")))) {
            px_srcline(654);
            _v1344 = px_index(_v1340, px_int(1LL));
            px_srcline(655);
            _v1345 = px_int(0LL);
            px_srcline(656);
            while (px_is_truthy(px_lt(_v1345, px_call(px_get_global("len"), (LXValue[]){_v1344}, 1)))) {
                px_srcline(657);
                (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(px_index(_v1344, _v1345), px_int(1LL)), _v1338}, 2));
                px_srcline(658);
                 _v1345 = px_add(_v1345, px_int(1LL));
            }
            px_srcline(659);
            if (px_is_truthy(px_ne(px_index(_v1340, px_int(2LL)), px_null()))) {
                px_srcline(660);
                (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1340, px_int(2LL)), _v1338}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1341, px_str("For")))) {
            px_srcline(662);
            _v1346 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1340, px_int(1LL))}, 1);
            px_srcline(663);
            _v1347 = px_int(0LL);
            px_srcline(664);
            while (px_is_truthy(px_lt(_v1347, px_call(px_get_global("len"), (LXValue[]){_v1346}, 1)))) {
                px_srcline(665);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1338, px_index(_v1346, _v1347)}, 2)))) {
                    px_srcline(666);
                    (void)(px_method(_v1338, "append", (LXValue[]){px_index(_v1346, _v1347)}, 1));
                }
                px_srcline(667);
                 _v1347 = px_add(_v1347, px_int(1LL));
            }
            px_srcline(668);
            (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1340, px_int(3LL)), _v1338}, 2));
        }
        else if (px_is_truthy(px_eq(_v1341, px_str("While")))) {
            px_srcline(670);
            (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1340, px_int(2LL)), _v1338}, 2));
        }
        px_srcline(671);
         _v1339 = px_add(_v1339, px_int(1LL));
    }
px_err_1348:
    if (px_err_1348_proped) return px_err_1348_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func");
    LXValue _v1349 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1350 = px_null();
    LXValue px_err_1351_val = px_null();
    int px_err_1351_proped = 0;
    px_srcline(674);
    _v1350 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1349, px_int(1LL))}, 1)}, 1));
    px_srcline(675);
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1349, _v1350}, 2);
px_err_1351:
    if (px_err_1351_proped) return px_err_1351_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func_named");
    LXValue _v1352 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1353 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1354 = px_null();
    LXValue _v1355 = px_null();
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
    LXValue _v1366 = px_null();
    LXValue _v1367 = px_null();
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
    LXValue _v1378 = px_null();
    LXValue _v1379 = px_null();
    LXValue px_err_1380_val = px_null();
    int px_err_1380_proped = 0;
    px_srcline(677);
    _v1354 = px_add(px_add(px_str("static LXValue "), _v1353), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(678);
     _v1354 = px_add(_v1354, px_str("    (void)ctx;\n"));
    px_srcline(681);
     _v1354 = px_add(_v1354, px_add(px_add(px_str("    px_srcfunc(\""), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1352, px_int(1LL))}, 1)), px_str("\");\n")));
    px_srcline(682);
    _v1355 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(683);
    _v1356 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(684);
    _v1357 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(685);
    _v1358 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(687);
    _v1359 = px_get_global("cg_topbody");
    px_srcline(688);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(689);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(690);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(691);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(692);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1358}, 1));
    px_srcline(696);
    _v1360 = px_list_n((LXValue[]){}, 0);
    px_srcline(697);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){px_index(_v1352, px_int(4LL)), _v1360}, 2));
    px_srcline(699);
    _v1361 = px_index(_v1352, px_int(2LL));
    px_srcline(700);
    _v1362 = px_int(0LL);
    px_srcline(701);
    _v1363 = px_list_n((LXValue[]){}, 0);
    px_srcline(702);
    while (px_is_truthy(px_lt(_v1362, px_call(px_get_global("len"), (LXValue[]){_v1361}, 1)))) {
        px_srcline(703);
        _v1364 = px_index(_v1361, _v1362);
        px_srcline(704);
        _v1365 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1364, px_int(1LL))}, 1);
        px_srcline(705);
        _v1366 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1365}, 1);
        px_srcline(706);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1360, _v1365}, 2))) {
            px_srcline(707);
            px_index_set(px_get_global("cg_cells"), _v1365, px_int(1LL));
        }
        px_srcline(708);
        _v1367 = px_str("px_null()");
        px_srcline(709);
        if (px_is_truthy(px_ne(px_index(_v1364, px_int(3LL)), px_null()))) {
            px_srcline(710);
             _v1367 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1364, px_int(3LL))}, 1);
        }
        px_srcline(711);
        (void)(px_method(_v1363, "append", (LXValue[]){px_list_n((LXValue[]){_v1366, _v1365, px_add(px_add(({ LXValue _s199 = px_add(px_add(px_str("(nargs > "), px_call(px_get_global("str"), (LXValue[]){_v1362}, 1)), px_str(") ? args[")); LXValue _s200 = px_call(px_get_global("str"), (LXValue[]){_v1362}, 1); px_add(_s199, _s200); }), px_str("] : ")), _v1367)}, 3)}, 1));
        px_srcline(712);
         _v1362 = px_add(_v1362, px_int(1LL));
    }
    px_srcline(715);
    _v1368 = px_list_n((LXValue[]){}, 0);
    px_srcline(716);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1352, px_int(4LL)), _v1368}, 2));
    px_srcline(719);
    _v1369 = px_list_n((LXValue[]){}, 0);
    px_srcline(720);
    (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1352, px_int(4LL)), _v1369}, 2));
    px_srcline(721);
    _v1370 = px_list_n((LXValue[]){}, 0);
    px_srcline(722);
    _v1371 = px_int(0LL);
    px_srcline(723);
    while (px_is_truthy(px_lt(_v1371, px_call(px_get_global("len"), (LXValue[]){_v1368}, 1)))) {
        px_srcline(724);
        _v1372 = px_index(_v1368, _v1371);
        px_srcline(725);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1372}, 1), px_null()))) {
            px_srcline(726);
             _v1371 = px_add(_v1371, px_int(1LL));
            px_srcline(727);
            continue;
        }
        px_srcline(728);
        if (px_is_truthy(({ LXValue _t1381 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1369, _v1372}, 2)); px_is_truthy(_t1381) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_topnames"), _v1372}, 2) : _t1381; }))) {
            px_srcline(729);
             _v1371 = px_add(_v1371, px_int(1LL));
            px_srcline(730);
            continue;
        }
        px_srcline(731);
        _v1366 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1372}, 1);
        px_srcline(732);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1360, _v1372}, 2))) {
            px_srcline(733);
            px_index_set(px_get_global("cg_cells"), _v1372, px_int(1LL));
        }
        px_srcline(734);
        (void)(px_method(_v1370, "append", (LXValue[]){px_list_n((LXValue[]){_v1366, _v1372}, 2)}, 1));
        px_srcline(735);
         _v1371 = px_add(_v1371, px_int(1LL));
    }
    px_srcline(737);
    _v1373 = px_int(0LL);
    px_srcline(738);
    while (px_is_truthy(px_lt(_v1373, px_call(px_get_global("len"), (LXValue[]){_v1363}, 1)))) {
        px_srcline(739);
        _v1374 = px_index(_v1363, _v1373);
        px_srcline(740);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1374, px_int(1LL))}, 1))) {
            px_srcline(741);
             _v1354 = px_add(_v1354, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1374, px_int(0LL))), px_str(" = px_cell(")), px_index(_v1374, px_int(2LL))), px_str(");\n")));
        }
        else {
            px_srcline(743);
             _v1354 = px_add(_v1354, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1374, px_int(0LL))), px_str(" = ")), px_index(_v1374, px_int(2LL))), px_str(";\n")));
        }
        px_srcline(744);
         _v1373 = px_add(_v1373, px_int(1LL));
    }
    px_srcline(745);
     _v1373 = px_int(0LL);
    px_srcline(746);
    while (px_is_truthy(px_lt(_v1373, px_call(px_get_global("len"), (LXValue[]){_v1370}, 1)))) {
        px_srcline(747);
        _v1375 = px_index(_v1370, _v1373);
        px_srcline(748);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1375, px_int(1LL))}, 1))) {
            px_srcline(749);
             _v1354 = px_add(_v1354, px_add(px_add(px_str("    LXValue "), px_index(_v1375, px_int(0LL))), px_str(" = px_cell(px_null());\n")));
        }
        else {
            px_srcline(751);
             _v1354 = px_add(_v1354, px_add(px_add(px_str("    LXValue "), px_index(_v1375, px_int(0LL))), px_str(" = px_null();\n")));
        }
        px_srcline(752);
         _v1373 = px_add(_v1373, px_int(1LL));
    }
    px_srcline(754);
    _v1376 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(755);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1376}, 1));
    px_srcline(756);
     _v1354 = px_add(_v1354, px_add(px_add(px_str("    LXValue "), _v1376), px_str("_val = px_null();\n")));
    px_srcline(757);
     _v1354 = px_add(_v1354, px_add(px_add(px_str("    int "), _v1376), px_str("_proped = 0;\n")));
    px_srcline(761);
    _v1377 = px_index(_v1352, px_int(4LL));
    px_srcline(762);
    _v1378 = px_list_n((LXValue[]){}, 0);
    px_srcline(763);
    _v1379 = px_int(0LL);
    px_srcline(764);
    while (px_is_truthy(px_lt(_v1379, px_call(px_get_global("len"), (LXValue[]){_v1377}, 1)))) {
        px_srcline(765);
        (void)(px_method(_v1378, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1377, _v1379), px_int(1LL)}, 2)}, 1));
        px_srcline(766);
         _v1379 = px_add(_v1379, px_int(1LL));
    }
    px_srcline(767);
     _v1354 = px_add(_v1354, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1378}, 2));
    px_srcline(768);
     _v1354 = px_add(_v1354, px_add(_v1376, px_str(":\n")));
    px_srcline(769);
     _v1354 = px_add(_v1354, px_add(px_add(px_add(px_add(px_str("    if ("), _v1376), px_str("_proped) return ")), _v1376), px_str("_val;\n")));
    px_srcline(770);
     _v1354 = px_add(_v1354, px_str("    return px_null();\n"));
    px_srcline(771);
     _v1354 = px_add(_v1354, px_str("}\n"));
    px_srcline(772);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(773);
    px_set_global("cg_vars", _v1355);
    px_srcline(774);
    px_set_global("cg_var_types", _v1356);
    px_srcline(775);
    px_set_global("cg_cells", _v1357);
    px_srcline(776);
    px_set_global("cg_immutables", _v1358);
    px_srcline(777);
    px_set_global("cg_topbody", _v1359);
    px_srcline(778);
    return _v1354;
px_err_1380:
    if (px_err_1380_proped) return px_err_1380_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_generate");
    LXValue _v1382 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1383 = px_null();
    LXValue _v1384 = px_null();
    LXValue _v1385 = px_null();
    LXValue _v1386 = px_null();
    LXValue _v1387 = px_null();
    LXValue _v1388 = px_null();
    LXValue _v1389 = px_null();
    LXValue _v1390 = px_null();
    LXValue _v1391 = px_null();
    LXValue _v1392 = px_null();
    LXValue _v1393 = px_null();
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
    LXValue _v1404 = px_null();
    LXValue _v1405 = px_null();
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
    LXValue _v1416 = px_null();
    LXValue _v1417 = px_null();
    LXValue px_err_1418_val = px_null();
    int px_err_1418_proped = 0;
    px_srcline(781);
    _v1383 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_srcline(782);
    px_set_global("cg_closures", px_str(""));
    px_srcline(783);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(784);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(785);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(786);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(787);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(788);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(789);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(790);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(791);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(792);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(793);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(794);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(795);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(796);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(797);
    px_set_global("cg_seq_uid", px_int(0LL));
    px_srcline(798);
    px_set_global("cg_iter_uid", px_int(0LL));
    px_srcline(799);
    px_set_global("cg_unpack_uid", px_int(0LL));
    px_srcline(800);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(801);
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1382}, 1));
    px_srcline(803);
    _v1384 = px_index(_v1382, px_int(1LL));
    px_srcline(807);
    px_set_global("cg_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(808);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){_v1384, px_get_global("cg_topnames")}, 2));
    px_srcline(809);
    _v1385 = px_int(0LL);
    px_srcline(810);
    while (px_is_truthy(px_lt(_v1385, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
        px_srcline(811);
        _v1386 = px_index(_v1384, _v1385);
        px_srcline(812);
        _v1387 = px_index(_v1386, px_int(0LL));
        px_srcline(813);
        if (px_is_truthy(px_eq(_v1387, px_str("FuncDef")))) {
            px_srcline(814);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1387, px_str("ExternDef")))) {
            px_srcline(817);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(1LL))}, 1), px_index(_v1386, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1387, px_str("VarDecl")))) {
            px_srcline(819);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(2LL))}, 1)}, 1));
            px_srcline(820);
            if (px_is_truthy(({ LXValue _t1419 = px_eq(px_index(_v1386, px_int(1LL)), px_str("Let")); px_is_truthy(_t1419) ? _t1419 : px_eq(px_index(_v1386, px_int(1LL)), px_str("Const")); }))) {
                px_srcline(821);
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1387, px_str("Assign")))) {
            px_srcline(823);
            _v1388 = px_index(_v1386, px_int(1LL));
            px_srcline(824);
            if (px_is_truthy(px_eq(px_index(_v1388, px_int(0LL)), px_str("Var")))) {
                px_srcline(825);
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1388, px_int(1LL))}, 1)}, 1));
            }
        }
        px_srcline(826);
         _v1385 = px_add(_v1385, px_int(1LL));
    }
    px_srcline(828);
    _v1389 = px_list_n((LXValue[]){}, 0);
    px_srcline(829);
    _v1390 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    px_srcline(830);
    _v1391 = px_int(0LL);
    px_srcline(831);
    while (px_is_truthy(px_lt(_v1391, px_call(px_get_global("len"), (LXValue[]){_v1390}, 1)))) {
        px_srcline(832);
        _v1392 = px_index(_v1390, _v1391);
        px_srcline(833);
        _v1393 = px_index(px_get_global("cg_impls"), _v1392);
        px_srcline(834);
        _v1394 = px_int(0LL);
        px_srcline(835);
        while (px_is_truthy(px_lt(_v1394, px_call(px_get_global("len"), (LXValue[]){_v1393}, 1)))) {
            px_srcline(836);
            (void)(px_method(_v1389, "append", (LXValue[]){px_list_n((LXValue[]){_v1392, px_index(_v1393, _v1394)}, 2)}, 1));
            px_srcline(837);
             _v1394 = px_add(_v1394, px_int(1LL));
        }
        px_srcline(838);
         _v1391 = px_add(_v1391, px_int(1LL));
    }
    px_srcline(840);
    _v1395 = px_int(1LL);
    px_srcline(841);
    while (px_is_truthy(px_lt(_v1395, px_call(px_get_global("len"), (LXValue[]){_v1389}, 1)))) {
        px_srcline(842);
        _v1396 = _v1395;
        px_srcline(843);
        while (px_is_truthy(px_gt(_v1396, px_int(0LL)))) {
            px_srcline(844);
            _v1397 = px_add(px_add(px_index(px_index(_v1389, px_sub(_v1396, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1389, px_sub(_v1396, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(845);
            _v1398 = px_add(px_add(px_index(px_index(_v1389, _v1396), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1389, _v1396), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(846);
            if (px_is_truthy(px_lt(_v1398, _v1397))) {
                px_srcline(847);
                _v1399 = px_index(_v1389, _v1396);
                px_srcline(848);
                px_index_set(_v1389, _v1396, px_index(_v1389, px_sub(_v1396, px_int(1LL))));
                px_srcline(849);
                px_index_set(_v1389, px_sub(_v1396, px_int(1LL)), _v1399);
            }
            px_srcline(850);
             _v1396 = px_sub(_v1396, px_int(1LL));
        }
        px_srcline(851);
         _v1395 = px_add(_v1395, px_int(1LL));
    }
    px_srcline(856);
    _v1400 = px_list_n((LXValue[]){}, 0);
    px_srcline(857);
    _v1401 = px_int(0LL);
    px_srcline(858);
    while (px_is_truthy(px_lt(_v1401, px_call(px_get_global("len"), (LXValue[]){_v1389}, 1)))) {
        px_srcline(859);
        _v1392 = px_index(px_index(_v1389, _v1401), px_int(0LL));
        px_srcline(860);
        _v1402 = px_index(px_index(_v1389, _v1401), px_int(1LL));
        px_srcline(861);
        _v1403 = ({ LXValue _s201 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1392}, 1)), px_str("_")); LXValue _s202 = px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1402, px_int(1LL))}, 1)}, 1); px_add(_s201, _s202); });
        px_srcline(862);
        (void)(px_method(_v1400, "push", (LXValue[]){px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1402, _v1403}, 2)}, 1));
        px_srcline(863);
        (void)(px_method(_v1400, "push", (LXValue[]){px_str("\n")}, 1));
        px_srcline(864);
         _v1401 = px_add(_v1401, px_int(1LL));
    }
    px_srcline(866);
    _v1404 = px_int(0LL);
    px_srcline(867);
    while (px_is_truthy(px_lt(_v1404, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
        px_srcline(868);
        _v1386 = px_index(_v1384, _v1404);
        px_srcline(869);
        if (px_is_truthy(px_eq(px_index(_v1386, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(870);
            (void)(px_method(_v1400, "push", (LXValue[]){px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1386}, 1)}, 1));
            px_srcline(871);
            (void)(px_method(_v1400, "push", (LXValue[]){px_str("\n")}, 1));
        }
        px_srcline(872);
         _v1404 = px_add(_v1404, px_int(1LL));
    }
    px_srcline(873);
     _v1383 = px_add(_v1383, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1400}, 2));
    px_srcline(875);
     _v1383 = px_add(_v1383, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(876);
     _v1383 = px_add(_v1383, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(877);
     _v1383 = px_add(_v1383, px_str("    px_register_builtins();\n"));
    px_srcline(879);
    _v1405 = px_int(0LL);
    px_srcline(880);
    while (px_is_truthy(px_lt(_v1405, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
        px_srcline(881);
        _v1386 = px_index(_v1384, _v1405);
        px_srcline(882);
        if (px_is_truthy(px_eq(px_index(_v1386, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(883);
            _v1403 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(1LL))}, 1);
            px_srcline(884);
            _v1406 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1403}, 1));
            px_srcline(885);
             _v1383 = px_add(_v1383, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1403), px_str("\", px_func(\"")), _v1403), px_str("\", ")), _v1406), px_str(", NULL));\n")));
        }
        px_srcline(886);
         _v1405 = px_add(_v1405, px_int(1LL));
    }
    px_srcline(888);
    _v1407 = px_int(0LL);
    px_srcline(889);
    while (px_is_truthy(px_lt(_v1407, px_call(px_get_global("len"), (LXValue[]){_v1389}, 1)))) {
        px_srcline(890);
        _v1392 = px_index(px_index(_v1389, _v1407), px_int(0LL));
        px_srcline(891);
        _v1402 = px_index(px_index(_v1389, _v1407), px_int(1LL));
        px_srcline(892);
        _v1408 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1402, px_int(1LL))}, 1);
        px_srcline(893);
        _v1403 = ({ LXValue _s203 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1392}, 1)), px_str("_")); LXValue _s204 = px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1408}, 1); px_add(_s203, _s204); });
        px_srcline(894);
         _v1383 = px_add(_v1383, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1392), px_str(".")), _v1408), px_str("\", px_func(\"")), _v1392), px_str(".")), _v1408), px_str("\", ")), _v1403), px_str(", NULL));\n")));
        px_srcline(895);
         _v1407 = px_add(_v1407, px_int(1LL));
    }
    px_srcline(898);
    _v1409 = px_list_n((LXValue[]){}, 0);
    px_srcline(900);
    px_set_global("cg_topbody", px_bool(true));
    px_srcline(901);
    _v1410 = px_int(0LL);
    px_srcline(902);
    while (px_is_truthy(px_lt(_v1410, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
        px_srcline(903);
        _v1386 = px_index(_v1384, _v1410);
        px_srcline(904);
        _v1387 = px_index(_v1386, px_int(0LL));
        px_srcline(905);
        if (px_is_truthy(({ LXValue _t1425 = ({ LXValue _t1424 = ({ LXValue _t1423 = ({ LXValue _t1422 = ({ LXValue _t1421 = ({ LXValue _t1420 = px_ne(_v1387, px_str("FuncDef")); px_is_truthy(_t1420) ? px_ne(_v1387, px_str("StructDef")) : _t1420; }); px_is_truthy(_t1421) ? px_ne(_v1387, px_str("EnumDef")) : _t1421; }); px_is_truthy(_t1422) ? px_ne(_v1387, px_str("TraitDef")) : _t1422; }); px_is_truthy(_t1423) ? px_ne(_v1387, px_str("ImplDef")) : _t1423; }); px_is_truthy(_t1424) ? px_ne(_v1387, px_str("Import")) : _t1424; }); px_is_truthy(_t1425) ? px_ne(_v1387, px_str("ExternDef")) : _t1425; }))) {
            px_srcline(906);
            (void)(px_method(_v1409, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1386, px_int(1LL)}, 2)}, 1));
        }
        px_srcline(907);
         _v1410 = px_add(_v1410, px_int(1LL));
    }
    px_srcline(908);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(909);
     _v1383 = px_add(_v1383, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1409}, 2));
    px_srcline(911);
    _v1411 = px_bool(false);
    px_srcline(912);
    _v1412 = px_int(0LL);
    px_srcline(913);
    while (px_is_truthy(px_lt(_v1412, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
        px_srcline(914);
        _v1386 = px_index(_v1384, _v1412);
        px_srcline(915);
        if (px_is_truthy(({ LXValue _t1426 = px_eq(px_index(_v1386, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1426) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1386, px_int(1LL))}, 1), px_str("main")) : _t1426; }))) {
            px_srcline(916);
             _v1411 = px_bool(true);
            px_srcline(917);
            break;
        }
        px_srcline(918);
         _v1412 = px_add(_v1412, px_int(1LL));
    }
    px_srcline(919);
    if (px_is_truthy(_v1411)) {
        px_srcline(920);
        _v1406 = px_str("fn_main");
        px_srcline(921);
         _v1383 = px_add(_v1383, px_add(px_add(px_str("    { LXValue _r = "), _v1406), px_str("(NULL, 0, NULL); int _code = 0;\n")));
        px_srcline(922);
         _v1383 = px_add(_v1383, px_str("      if (px_is_result(_r)) {\n"));
        px_srcline(923);
         _v1383 = px_add(_v1383, px_str("        if (!px_result_ok(_r)) {\n"));
        px_srcline(924);
         _v1383 = px_add(_v1383, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
        px_srcline(925);
         _v1383 = px_add(_v1383, px_str("          _code = 1;\n"));
        px_srcline(926);
         _v1383 = px_add(_v1383, px_str("        } else {\n"));
        px_srcline(927);
         _v1383 = px_add(_v1383, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
        px_srcline(928);
         _v1383 = px_add(_v1383, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
        px_srcline(929);
         _v1383 = px_add(_v1383, px_str("        }\n"));
        px_srcline(930);
         _v1383 = px_add(_v1383, px_str("      } else if (_r.type == PX_INT) {\n"));
        px_srcline(931);
         _v1383 = px_add(_v1383, px_str("        _code = (int)_r.as.i;\n"));
        px_srcline(932);
         _v1383 = px_add(_v1383, px_str("      }\n"));
        px_srcline(933);
         _v1383 = px_add(_v1383, px_str("      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）\n"));
        px_srcline(934);
         _v1383 = px_add(_v1383, px_str("    }\n"));
    }
    else {
        px_srcline(936);
         _v1383 = px_add(_v1383, px_str("    return px_exit_code_final(0);   // M120（qg-issue 76 E1）\n"));
    }
    px_srcline(937);
     _v1383 = px_add(_v1383, px_str("}\n"));
    px_srcline(939);
    _v1413 = px_call(px_get_global("cg_find"), (LXValue[]){_v1383, px_str("int main(")}, 2);
    px_srcline(940);
    if (px_is_truthy(px_ge(_v1413, px_int(0LL)))) {
        px_srcline(941);
        _v1414 = px_slice(_v1383, px_int(0LL), _v1413, px_null());
        px_srcline(942);
        _v1415 = px_slice(_v1383, _v1413, px_call(px_get_global("len"), (LXValue[]){_v1383}, 1), px_null());
        px_srcline(943);
        _v1416 = px_call(px_get_global("cg_find"), (LXValue[]){_v1414, px_str("static LXValue")}, 2);
        px_srcline(944);
        _v1417 = px_str("");
        px_srcline(945);
        if (px_is_truthy(px_ge(_v1416, px_int(0LL)))) {
            px_srcline(946);
             _v1417 = px_add(px_add(px_add(px_add(px_slice(_v1414, px_int(0LL), _v1416, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1414, _v1416, px_call(px_get_global("len"), (LXValue[]){_v1414}, 1), px_null())), _v1415);
        }
        else {
            px_srcline(948);
             _v1417 = px_add(px_add(px_add(_v1414, px_get_global("cg_closures")), px_str("\n")), _v1415);
        }
        px_srcline(949);
        return _v1417;
    }
    px_srcline(950);
    return _v1383;
px_err_1418:
    if (px_err_1418_proped) return px_err_1418_val;
    return px_null();
}

static LXValue fn_bc_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_dict");
    LXValue _v1427 = px_null();
    LXValue px_err_1428_val = px_null();
    int px_err_1428_proped = 0;
    px_srcline(41);
    _v1427 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(42);
    (void)(px_method(_v1427, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(43);
    return _v1427;
px_err_1428:
    if (px_err_1428_proped) return px_err_1428_val;
    return px_null();
}

static LXValue fn_bc_k_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_find");
    LXValue _v1429 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1430 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1431 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1432 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1433 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1434 = px_null();
    LXValue _v1435 = px_null();
    LXValue px_err_1436_val = px_null();
    int px_err_1436_proped = 0;
    px_srcline(47);
    _v1434 = px_int(0LL);
    px_srcline(48);
    while (px_is_truthy(px_lt(_v1434, px_call(px_get_global("len"), (LXValue[]){_v1429}, 1)))) {
        px_srcline(49);
        _v1435 = px_index(_v1429, _v1434);
        px_srcline(50);
        if (px_is_truthy(px_eq(px_index(_v1435, px_str("kind")), _v1430))) {
            px_srcline(51);
            if (px_is_truthy(px_eq(_v1430, px_str("int")))) {
                px_srcline(52);
                if (px_is_truthy(px_eq(px_index(_v1435, px_str("i")), _v1431))) {
                    px_srcline(53);
                    return _v1434;
                }
            }
            else if (px_is_truthy(px_eq(_v1430, px_str("float")))) {
                px_srcline(59);
                if (px_is_truthy(({ LXValue _s205 = px_call(px_get_global("float64_bits"), (LXValue[]){px_index(_v1435, px_str("f"))}, 1); LXValue _s206 = px_call(px_get_global("float64_bits"), (LXValue[]){_v1432}, 1); px_eq(_s205, _s206); }))) {
                    px_srcline(60);
                    return _v1434;
                }
            }
            else if (px_is_truthy(px_eq(_v1430, px_str("str")))) {
                px_srcline(62);
                if (px_is_truthy(px_eq(px_index(_v1435, px_str("s")), _v1433))) {
                    px_srcline(63);
                    return _v1434;
                }
            }
            else {
                px_srcline(66);
                if (px_is_truthy(px_eq(px_index(_v1435, px_str("i")), _v1431))) {
                    px_srcline(67);
                    return _v1434;
                }
            }
        }
        px_srcline(68);
         _v1434 = px_add(_v1434, px_int(1LL));
    }
    px_srcline(69);
    return px_neg(px_int(1LL));
px_err_1436:
    if (px_err_1436_proped) return px_err_1436_val;
    return px_null();
}

static LXValue fn_bc_k_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_add");
    LXValue _v1437 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1438 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1439 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1440 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1441 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1442 = px_null();
    LXValue _v1443 = px_null();
    LXValue px_err_1444_val = px_null();
    int px_err_1444_proped = 0;
    px_srcline(71);
    _v1442 = px_call(px_get_global("bc_k_find"), (LXValue[]){_v1437, _v1438, _v1439, _v1440, _v1441}, 5);
    px_srcline(72);
    if (px_is_truthy(px_ge(_v1442, px_int(0LL)))) {
        px_srcline(73);
        return _v1442;
    }
    px_srcline(74);
    _v1443 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(75);
    px_index_set(_v1443, px_str("kind"), _v1438);
    px_srcline(76);
    px_index_set(_v1443, px_str("i"), _v1439);
    px_srcline(77);
    px_index_set(_v1443, px_str("f"), _v1440);
    px_srcline(78);
    px_index_set(_v1443, px_str("s"), _v1441);
    px_srcline(79);
    (void)(px_method(_v1437, "push", (LXValue[]){_v1443}, 1));
    px_srcline(80);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1437}, 1), px_int(1LL));
px_err_1444:
    if (px_err_1444_proped) return px_err_1444_val;
    return px_null();
}

static LXValue fn_bc_n_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_n_add");
    LXValue _v1445 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1446 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1447 = px_null();
    LXValue px_err_1448_val = px_null();
    int px_err_1448_proped = 0;
    px_srcline(83);
    _v1447 = px_int(0LL);
    px_srcline(84);
    while (px_is_truthy(px_lt(_v1447, px_call(px_get_global("len"), (LXValue[]){_v1445}, 1)))) {
        px_srcline(85);
        if (px_is_truthy(px_eq(px_index(_v1445, _v1447), _v1446))) {
            px_srcline(86);
            return _v1447;
        }
        px_srcline(87);
         _v1447 = px_add(_v1447, px_int(1LL));
    }
    px_srcline(88);
    (void)(px_method(_v1445, "push", (LXValue[]){_v1446}, 1));
    px_srcline(89);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1445}, 1), px_int(1LL));
px_err_1448:
    if (px_err_1448_proped) return px_err_1448_val;
    return px_null();
}

static LXValue fn_bc_g_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_g_add");
    LXValue _v1449 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1450 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1451 = px_null();
    LXValue px_err_1452_val = px_null();
    int px_err_1452_proped = 0;
    px_srcline(92);
    _v1451 = px_int(0LL);
    px_srcline(93);
    while (px_is_truthy(px_lt(_v1451, px_call(px_get_global("len"), (LXValue[]){_v1449}, 1)))) {
        px_srcline(94);
        if (px_is_truthy(px_eq(px_index(_v1449, _v1451), _v1450))) {
            px_srcline(95);
            return _v1451;
        }
        px_srcline(96);
         _v1451 = px_add(_v1451, px_int(1LL));
    }
    px_srcline(97);
    (void)(px_method(_v1449, "push", (LXValue[]){_v1450}, 1));
    px_srcline(98);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1449}, 1), px_int(1LL));
px_err_1452:
    if (px_err_1452_proped) return px_err_1452_val;
    return px_null();
}

static LXValue fn_bc_is_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_is_global");
    LXValue _v1453 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1454_val = px_null();
    int px_err_1454_proped = 0;
    px_srcline(101);
    if (px_is_truthy(px_eq(px_get_global("g_bcm"), px_null()))) {
        px_srcline(102);
        return px_bool(false);
    }
    px_srcline(103);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1453}, 2);
px_err_1454:
    if (px_err_1454_proped) return px_err_1454_val;
    return px_null();
}

static LXValue fn_bc_new_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_module");
    LXValue _v1455 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1456 = px_null();
    LXValue px_err_1457_val = px_null();
    int px_err_1457_proped = 0;
    px_srcline(106);
    _v1456 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(107);
    px_index_set(_v1456, px_str("name"), _v1455);
    px_srcline(108);
    px_index_set(_v1456, px_str("k_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(109);
    px_index_set(_v1456, px_str("n_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(110);
    px_index_set(_v1456, px_str("globals"), px_list_n((LXValue[]){}, 0));
    px_srcline(111);
    px_index_set(_v1456, px_str("funcs"), px_list_n((LXValue[]){}, 0));
    px_srcline(112);
    px_index_set(_v1456, px_str("top"), px_neg(px_int(1LL)));
    px_srcline(115);
    px_index_set(_v1456, px_str("structs"), px_list_n((LXValue[]){}, 0));
    px_srcline(116);
    px_index_set(_v1456, px_str("enums"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(117);
    px_index_set(_v1456, px_str("consts"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(118);
    px_index_set(_v1456, px_str("nclosure"), px_int(0LL));
    px_srcline(119);
    return _v1456;
px_err_1457:
    if (px_err_1457_proped) return px_err_1457_val;
    return px_null();
}

static LXValue fn_bc_struct_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_struct_index");
    LXValue _v1458 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1459 = px_null();
    LXValue px_err_1460_val = px_null();
    int px_err_1460_proped = 0;
    px_srcline(122);
    _v1459 = px_int(0LL);
    px_srcline(123);
    while (px_is_truthy(px_lt(_v1459, px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("structs"))}, 1)))) {
        px_srcline(124);
        if (px_is_truthy(px_eq(px_index(px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1459), px_str("name")), _v1458))) {
            px_srcline(125);
            return _v1459;
        }
        px_srcline(126);
         _v1459 = px_add(_v1459, px_int(1LL));
    }
    px_srcline(127);
    return px_neg(px_int(1LL));
px_err_1460:
    if (px_err_1460_proped) return px_err_1460_val;
    return px_null();
}

static LXValue fn_bc_enum_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_enum_has");
    LXValue _v1461 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1462 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1463_val = px_null();
    int px_err_1463_proped = 0;
    px_srcline(130);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1461}, 1)))) {
        px_srcline(131);
        return px_bool(false);
    }
    px_srcline(132);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_index(px_get_global("g_bcm"), px_str("enums")), _v1461), _v1462}, 2);
px_err_1463:
    if (px_err_1463_proped) return px_err_1463_val;
    return px_null();
}

static LXValue fn_bc_const_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_const_find");
    LXValue _v1464 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1465 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1466 = px_null();
    LXValue _v1467 = px_null();
    LXValue px_err_1468_val = px_null();
    int px_err_1468_proped = 0;
    px_srcline(135);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1464}, 1)))) {
        px_srcline(136);
        return px_null();
    }
    px_srcline(137);
    _v1466 = px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1464);
    px_srcline(138);
    _v1467 = px_int(0LL);
    px_srcline(139);
    while (px_is_truthy(px_lt(_v1467, px_call(px_get_global("len"), (LXValue[]){_v1466}, 1)))) {
        px_srcline(140);
        if (px_is_truthy(px_eq(px_index(px_index(_v1466, _v1467), px_str("n")), _v1465))) {
            px_srcline(141);
            return px_index(px_index(_v1466, _v1467), px_str("e"));
        }
        px_srcline(142);
         _v1467 = px_add(_v1467, px_int(1LL));
    }
    px_srcline(143);
    return px_null();
px_err_1468:
    if (px_err_1468_proped) return px_err_1468_val;
    return px_null();
}

static LXValue fn_bc_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_consts");
    LXValue _v1469 = (nargs > 0) ? args[0] : px_null();
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
    LXValue px_err_1480_val = px_null();
    int px_err_1480_proped = 0;
    px_srcline(146);
    _v1470 = px_int(0LL);
    px_srcline(147);
    while (px_is_truthy(px_lt(_v1470, px_call(px_get_global("len"), (LXValue[]){_v1469}, 1)))) {
        px_srcline(148);
        _v1471 = px_index(_v1469, _v1470);
        px_srcline(149);
        _v1472 = px_index(_v1471, px_int(0LL));
        px_srcline(150);
        if (px_is_truthy(px_eq(_v1472, px_str("TypeConst")))) {
            px_srcline(151);
            _v1473 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1471, px_int(1LL))}, 1);
            px_srcline(152);
            if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1473}, 1)))) {
                px_srcline(153);
                px_index_set(px_index(px_get_global("g_bcm"), px_str("consts")), _v1473, px_list_n((LXValue[]){}, 0));
            }
            px_srcline(154);
            _v1474 = px_int(0LL);
            px_srcline(155);
            while (px_is_truthy(px_lt(_v1474, px_call(px_get_global("len"), (LXValue[]){px_index(_v1471, px_int(2LL))}, 1)))) {
                px_srcline(156);
                _v1475 = px_index(px_index(_v1471, px_int(2LL)), _v1474);
                px_srcline(157);
                _v1476 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
                px_srcline(158);
                px_index_set(_v1476, px_str("n"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1475, px_int(1LL))}, 1));
                px_srcline(159);
                px_index_set(_v1476, px_str("e"), px_index(_v1475, px_int(2LL)));
                px_srcline(160);
                (void)(px_method(px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1473), "push", (LXValue[]){_v1476}, 1));
                px_srcline(161);
                 _v1474 = px_add(_v1474, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1472, px_str("FuncDef")))) {
            px_srcline(163);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1471, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1472, px_str("ImplDef")))) {
            px_srcline(165);
            _v1477 = px_int(0LL);
            px_srcline(166);
            while (px_is_truthy(px_lt(_v1477, px_call(px_get_global("len"), (LXValue[]){px_index(_v1471, px_int(3LL))}, 1)))) {
                px_srcline(167);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1471, px_int(3LL)), _v1477), px_int(4LL))}, 1));
                px_srcline(168);
                 _v1477 = px_add(_v1477, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1472, px_str("If")))) {
            px_srcline(170);
            _v1478 = px_index(_v1471, px_int(1LL));
            px_srcline(171);
            _v1479 = px_int(0LL);
            px_srcline(172);
            while (px_is_truthy(px_lt(_v1479, px_call(px_get_global("len"), (LXValue[]){_v1478}, 1)))) {
                px_srcline(173);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(_v1478, _v1479), px_int(1LL))}, 1));
                px_srcline(174);
                 _v1479 = px_add(_v1479, px_int(1LL));
            }
            px_srcline(175);
            if (px_is_truthy(px_ne(px_index(_v1471, px_int(2LL)), px_null()))) {
                px_srcline(176);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1471, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1472, px_str("While")))) {
            px_srcline(178);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1471, px_int(2LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1472, px_str("For")))) {
            px_srcline(180);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1471, px_int(3LL))}, 1));
        }
        px_srcline(181);
         _v1470 = px_add(_v1470, px_int(1LL));
    }
px_err_1480:
    if (px_err_1480_proped) return px_err_1480_val;
    return px_null();
}

static LXValue fn_bc_collect_impl_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_impl_list");
    LXValue _v1481 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1482 = px_null();
    LXValue _v1483 = px_null();
    LXValue _v1484 = px_null();
    LXValue _v1485 = px_null();
    LXValue _v1486 = px_null();
    LXValue _v1487 = px_null();
    LXValue _v1488 = px_null();
    LXValue _v1489 = px_null();
    LXValue _v1490 = px_null();
    LXValue _v1491 = px_null();
    LXValue px_err_1492_val = px_null();
    int px_err_1492_proped = 0;
    px_srcline(185);
    _v1482 = px_list_n((LXValue[]){}, 0);
    px_srcline(186);
    _v1483 = px_int(0LL);
    px_srcline(187);
    while (px_is_truthy(px_lt(_v1483, px_call(px_get_global("len"), (LXValue[]){_v1481}, 1)))) {
        px_srcline(188);
        _v1484 = px_index(_v1481, _v1483);
        px_srcline(189);
        if (px_is_truthy(px_eq(px_index(_v1484, px_int(0LL)), px_str("ImplDef")))) {
            px_srcline(190);
            _v1485 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1484, px_int(1LL))}, 1);
            px_srcline(191);
            _v1486 = px_int(0LL);
            px_srcline(192);
            while (px_is_truthy(px_lt(_v1486, px_call(px_get_global("len"), (LXValue[]){px_index(_v1484, px_int(3LL))}, 1)))) {
                px_srcline(193);
                (void)(px_method(_v1482, "append", (LXValue[]){px_list_n((LXValue[]){_v1485, px_index(px_index(_v1484, px_int(3LL)), _v1486)}, 2)}, 1));
                px_srcline(194);
                 _v1486 = px_add(_v1486, px_int(1LL));
            }
        }
        px_srcline(195);
         _v1483 = px_add(_v1483, px_int(1LL));
    }
    px_srcline(197);
    _v1487 = px_int(1LL);
    px_srcline(198);
    while (px_is_truthy(px_lt(_v1487, px_call(px_get_global("len"), (LXValue[]){_v1482}, 1)))) {
        px_srcline(199);
        _v1488 = _v1487;
        px_srcline(200);
        while (px_is_truthy(px_gt(_v1488, px_int(0LL)))) {
            px_srcline(201);
            _v1489 = px_add(px_add(px_index(px_index(_v1482, px_sub(_v1488, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1482, px_sub(_v1488, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(202);
            _v1490 = px_add(px_add(px_index(px_index(_v1482, _v1488), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1482, _v1488), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(203);
            if (px_is_truthy(px_lt(_v1490, _v1489))) {
                px_srcline(204);
                _v1491 = px_index(_v1482, _v1488);
                px_srcline(205);
                px_index_set(_v1482, _v1488, px_index(_v1482, px_sub(_v1488, px_int(1LL))));
                px_srcline(206);
                px_index_set(_v1482, px_sub(_v1488, px_int(1LL)), _v1491);
            }
            px_srcline(207);
             _v1488 = px_sub(_v1488, px_int(1LL));
        }
        px_srcline(208);
         _v1487 = px_add(_v1487, px_int(1LL));
    }
    px_srcline(209);
    return _v1482;
px_err_1492:
    if (px_err_1492_proped) return px_err_1492_val;
    return px_null();
}

static LXValue fn_bc_new_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_func");
    LXValue _v1493 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1494 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1495 = px_null();
    LXValue px_err_1496_val = px_null();
    int px_err_1496_proped = 0;
    px_srcline(214);
    _v1495 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(215);
    px_index_set(_v1495, px_str("name"), _v1493);
    px_srcline(216);
    px_index_set(_v1495, px_str("arity"), _v1494);
    px_srcline(217);
    px_index_set(_v1495, px_str("ndefault"), px_int(0LL));
    px_srcline(218);
    px_index_set(_v1495, px_str("nslots"), _v1494);
    px_srcline(219);
    px_index_set(_v1495, px_str("bc"), px_list_n((LXValue[]){}, 0));
    px_srcline(220);
    px_index_set(_v1495, px_str("smap"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(221);
    px_index_set(_v1495, px_str("next_slot"), _v1494);
    px_srcline(222);
    px_index_set(_v1495, px_str("is_top"), px_bool(false));
    px_srcline(223);
    px_index_set(_v1495, px_str("loops"), px_list_n((LXValue[]){}, 0));
    px_srcline(229);
    px_index_set(_v1495, px_str("upnames"), px_list_n((LXValue[]){}, 0));
    px_srcline(230);
    px_index_set(_v1495, px_str("cell"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(231);
    px_index_set(_v1495, px_str("boxed"), px_list_n((LXValue[]){}, 0));
    px_srcline(232);
    return _v1495;
px_err_1496:
    if (px_err_1496_proped) return px_err_1496_val;
    return px_null();
}

static LXValue fn_bc_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_slot");
    LXValue _v1497 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1498 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1499 = px_null();
    LXValue px_err_1500_val = px_null();
    int px_err_1500_proped = 0;
    px_srcline(235);
    if (px_is_truthy(px_method(px_index(_v1497, px_str("smap")), "has", (LXValue[]){_v1498}, 1))) {
        px_srcline(236);
        return px_index(px_index(_v1497, px_str("smap")), _v1498);
    }
    px_srcline(237);
    _v1499 = px_index(_v1497, px_str("next_slot"));
    px_srcline(238);
    px_index_set(_v1497, px_str("next_slot"), px_add(_v1499, px_int(1LL)));
    px_srcline(239);
    px_index_set(px_index(_v1497, px_str("smap")), _v1498, _v1499);
    px_srcline(240);
    return _v1499;
px_err_1500:
    if (px_err_1500_proped) return px_err_1500_val;
    return px_null();
}

static LXValue fn_bc_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_tmp");
    LXValue _v1501 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1502 = px_null();
    LXValue px_err_1503_val = px_null();
    int px_err_1503_proped = 0;
    px_srcline(243);
    _v1502 = px_index(_v1501, px_str("next_slot"));
    px_srcline(244);
    px_index_set(_v1501, px_str("next_slot"), px_add(_v1502, px_int(1LL)));
    px_srcline(245);
    return _v1502;
px_err_1503:
    if (px_err_1503_proped) return px_err_1503_val;
    return px_null();
}

static LXValue fn_bc_cell_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_has");
    LXValue _v1504 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1505 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1506_val = px_null();
    int px_err_1506_proped = 0;
    px_srcline(250);
    return px_method(px_index(_v1504, px_str("cell")), "has", (LXValue[]){_v1505}, 1);
px_err_1506:
    if (px_err_1506_proped) return px_err_1506_val;
    return px_null();
}

static LXValue fn_bc_cell_mark(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_mark");
    LXValue _v1507 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1508 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1509_val = px_null();
    int px_err_1509_proped = 0;
    px_srcline(252);
    px_index_set(px_index(_v1507, px_str("cell")), _v1508, px_int(1LL));
px_err_1509:
    if (px_err_1509_proped) return px_err_1509_val;
    return px_null();
}

static LXValue fn_bc_load_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_load_var");
    LXValue _v1510 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1511 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1512 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1513 = px_null();
    LXValue _v1514 = px_null();
    LXValue px_err_1515_val = px_null();
    int px_err_1515_proped = 0;
    px_srcline(255);
    _v1513 = px_index(px_index(_v1510, px_str("smap")), _v1511);
    px_srcline(256);
    if (px_is_truthy(px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1510, _v1511}, 2)))) {
        px_srcline(257);
        if (px_is_truthy(({ LXValue _t1516 = px_ge(_v1512, px_int(0LL)); px_is_truthy(_t1516) ? px_ne(_v1512, _v1513) : _t1516; }))) {
            px_srcline(258);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1510, px_str("MOV"), _v1512, _v1513, px_int(0LL)}, 5));
        }
        px_srcline(259);
        return px_null();
    }
    px_srcline(260);
    if (px_is_truthy(px_lt(_v1512, px_int(0LL)))) {
        px_srcline(261);
        return px_null();
    }
    px_srcline(262);
    if (px_is_truthy(px_eq(_v1512, _v1513))) {
        px_srcline(265);
        _v1514 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1510}, 1);
        px_srcline(266);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1510, px_str("CELLGET"), _v1514, _v1513, px_int(0LL)}, 5));
        px_srcline(267);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1510, px_str("MOV"), _v1513, _v1514, px_int(0LL)}, 5));
        px_srcline(268);
        return px_null();
    }
    px_srcline(269);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1510, px_str("CELLGET"), _v1512, _v1513, px_int(0LL)}, 5));
px_err_1515:
    if (px_err_1515_proped) return px_err_1515_val;
    return px_null();
}

static LXValue fn_bc_store_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_store_var");
    LXValue _v1517 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1518 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1519 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1520 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_1521_val = px_null();
    int px_err_1521_proped = 0;
    px_srcline(272);
    if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1517, _v1518}, 2))) {
        px_srcline(273);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1517, px_str("CELLSET"), _v1520, _v1519, px_int(0LL)}, 5));
    }
    else {
        px_srcline(275);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1517, px_str("MOV"), _v1519, _v1520, px_int(0LL)}, 5));
    }
px_err_1521:
    if (px_err_1521_proped) return px_err_1521_val;
    return px_null();
}

static LXValue fn_bc_closure_free(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_closure_free");
    LXValue _v1522 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1523 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1524 = px_null();
    LXValue px_err_1525_val = px_null();
    int px_err_1525_proped = 0;
    px_srcline(279);
    _v1524 = px_list_n((LXValue[]){}, 0);
    px_srcline(280);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v1522, px_null(), _v1523, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v1524}, 2));
    px_srcline(281);
    return _v1524;
px_err_1525:
    if (px_err_1525_proped) return px_err_1525_val;
    return px_null();
}

static LXValue fn_bc_lambda_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_hoist");
    LXValue _v1526 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1527 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1528_val = px_null();
    int px_err_1528_proped = 0;
    px_srcline(284);
    if (px_is_truthy(({ LXValue _t1530 = ({ LXValue _t1529 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1526}, 1), px_str("list")); px_is_truthy(_t1529) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1526}, 1), px_int(0LL)) : _t1529; }); px_is_truthy(_t1530) ? px_eq(px_index(_v1526, px_int(0LL)), px_str("Block")) : _t1530; }))) {
        px_srcline(285);
        (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1526, px_int(1LL)), _v1527}, 2));
    }
px_err_1528:
    if (px_err_1528_proped) return px_err_1528_val;
    return px_null();
}

static LXValue fn_bc_lambda_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_decl");
    LXValue _v1531 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1532 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1533_val = px_null();
    int px_err_1533_proped = 0;
    px_srcline(288);
    if (px_is_truthy(({ LXValue _t1535 = ({ LXValue _t1534 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1531}, 1), px_str("list")); px_is_truthy(_t1534) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1531}, 1), px_int(0LL)) : _t1534; }); px_is_truthy(_t1535) ? px_eq(px_index(_v1531, px_int(0LL)), px_str("Block")) : _t1535; }))) {
        px_srcline(289);
        (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1531, px_int(1LL)), _v1532}, 2));
    }
px_err_1533:
    if (px_err_1533_proped) return px_err_1533_val;
    return px_null();
}

static LXValue fn_bc_box_frame(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_box_frame");
    LXValue _v1536 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1537 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1538 = px_null();
    LXValue _v1539 = px_null();
    LXValue _v1540 = px_null();
    LXValue px_err_1541_val = px_null();
    int px_err_1541_proped = 0;
    px_srcline(295);
    _v1538 = px_list_n((LXValue[]){}, 0);
    px_srcline(296);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1537, _v1538}, 2));
    px_srcline(297);
    _v1539 = px_int(0LL);
    px_srcline(298);
    while (px_is_truthy(px_lt(_v1539, px_call(px_get_global("len"), (LXValue[]){_v1538}, 1)))) {
        px_srcline(299);
        _v1540 = px_index(_v1538, _v1539);
        px_srcline(300);
        if (px_is_truthy(({ LXValue _t1542 = px_method(px_index(_v1536, px_str("smap")), "has", (LXValue[]){_v1540}, 1); px_is_truthy(_t1542) ? px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1536, _v1540}, 2)) : _t1542; }))) {
            px_srcline(301);
            (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1536, _v1540}, 2));
            px_srcline(302);
            (void)(px_method(px_index(_v1536, px_str("boxed")), "push", (LXValue[]){_v1540}, 1));
            px_srcline(303);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1536, px_str("CELLNEW"), px_index(px_index(_v1536, px_str("smap")), _v1540), px_index(px_index(_v1536, px_str("smap")), _v1540), px_int(0LL)}, 5));
        }
        px_srcline(304);
         _v1539 = px_add(_v1539, px_int(1LL));
    }
px_err_1541:
    if (px_err_1541_proped) return px_err_1541_val;
    return px_null();
}

static LXValue fn_bc_emit_inst(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_inst");
    LXValue _v1543 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1544 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1545 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1546 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1547 = (nargs > 4) ? args[4] : px_null();
    LXValue px_err_1548_val = px_null();
    int px_err_1548_proped = 0;
    px_srcline(310);
    (void)(px_method(px_index(_v1543, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){_v1544, px_int(0LL), _v1545, _v1546, _v1547}, 5)}, 1));
px_err_1548:
    if (px_err_1548_proped) return px_err_1548_val;
    return px_null();
}

static LXValue fn_bc_patch_off(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_patch_off");
    LXValue _v1549 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1550 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1551 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1552_val = px_null();
    int px_err_1552_proped = 0;
    px_srcline(314);
    px_index_set(px_index(px_index(_v1549, px_str("bc")), _v1550), px_int(3LL), px_sub(_v1551, px_add(_v1550, px_int(1LL))));
px_err_1552:
    if (px_err_1552_proped) return px_err_1552_val;
    return px_null();
}

static LXValue fn_bc_collect_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_hoist");
    LXValue _v1553 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1554 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1555 = px_null();
    LXValue _v1556 = px_null();
    LXValue _v1557 = px_null();
    LXValue _v1558 = px_null();
    LXValue _v1559 = px_null();
    LXValue _v1560 = px_null();
    LXValue _v1561 = px_null();
    LXValue _v1562 = px_null();
    LXValue _v1563 = px_null();
    LXValue _v1564 = px_null();
    LXValue px_err_1565_val = px_null();
    int px_err_1565_proped = 0;
    px_srcline(317);
    _v1555 = px_int(0LL);
    px_srcline(318);
    while (px_is_truthy(px_lt(_v1555, px_call(px_get_global("len"), (LXValue[]){_v1553}, 1)))) {
        px_srcline(319);
        _v1556 = px_index(_v1553, _v1555);
        px_srcline(320);
        _v1557 = px_index(_v1556, px_int(0LL));
        px_srcline(321);
        if (px_is_truthy(px_eq(_v1557, px_str("Assign")))) {
            px_srcline(322);
            _v1558 = px_index(_v1556, px_int(1LL));
            px_srcline(323);
            if (px_is_truthy(px_eq(px_index(_v1558, px_int(0LL)), px_str("Var")))) {
                px_srcline(324);
                _v1559 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1558, px_int(1LL))}, 1);
                px_srcline(325);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1554, _v1559}, 2)))) {
                    px_srcline(326);
                    (void)(px_method(_v1554, "append", (LXValue[]){_v1559}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1557, px_str("VarDecl")))) {
            px_srcline(328);
            _v1559 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1556, px_int(2LL))}, 1);
            px_srcline(329);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1554, _v1559}, 2)))) {
                px_srcline(330);
                (void)(px_method(_v1554, "append", (LXValue[]){_v1559}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1557, px_str("FuncDef")))) {
            px_srcline(334);
            _v1559 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1556, px_int(1LL))}, 1);
            px_srcline(335);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1554, _v1559}, 2)))) {
                px_srcline(336);
                (void)(px_method(_v1554, "append", (LXValue[]){_v1559}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1557, px_str("If")))) {
            px_srcline(338);
            _v1560 = px_index(_v1556, px_int(1LL));
            px_srcline(339);
            _v1561 = px_int(0LL);
            px_srcline(340);
            while (px_is_truthy(px_lt(_v1561, px_call(px_get_global("len"), (LXValue[]){_v1560}, 1)))) {
                px_srcline(341);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(px_index(_v1560, _v1561), px_int(1LL)), _v1554}, 2));
                px_srcline(342);
                 _v1561 = px_add(_v1561, px_int(1LL));
            }
            px_srcline(343);
            if (px_is_truthy(px_ne(px_index(_v1556, px_int(2LL)), px_null()))) {
                px_srcline(344);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1556, px_int(2LL)), _v1554}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1557, px_str("While")))) {
            px_srcline(346);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1556, px_int(2LL)), _v1554}, 2));
        }
        else if (px_is_truthy(px_eq(_v1557, px_str("For")))) {
            px_srcline(350);
            _v1562 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1556, px_int(1LL))}, 1);
            px_srcline(351);
            _v1563 = px_int(0LL);
            px_srcline(352);
            while (px_is_truthy(px_lt(_v1563, px_call(px_get_global("len"), (LXValue[]){_v1562}, 1)))) {
                px_srcline(353);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1554, px_index(_v1562, _v1563)}, 2)))) {
                    px_srcline(354);
                    (void)(px_method(_v1554, "append", (LXValue[]){px_index(_v1562, _v1563)}, 1));
                }
                px_srcline(355);
                 _v1563 = px_add(_v1563, px_int(1LL));
            }
            px_srcline(356);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1556, px_int(3LL)), _v1554}, 2));
        }
        else if (px_is_truthy(px_eq(_v1557, px_str("ChanDecl")))) {
            px_srcline(359);
            _v1564 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1556, px_int(1LL))}, 1);
            px_srcline(360);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1554, _v1564}, 2)))) {
                px_srcline(361);
                (void)(px_method(_v1554, "append", (LXValue[]){_v1564}, 1));
            }
        }
        px_srcline(362);
         _v1555 = px_add(_v1555, px_int(1LL));
    }
px_err_1565:
    if (px_err_1565_proped) return px_err_1565_val;
    return px_null();
}

static LXValue fn_bc_collect_decl_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_decl_vars");
    LXValue _v1566 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1567 = (nargs > 1) ? args[1] : px_null();
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
    LXValue px_err_1578_val = px_null();
    int px_err_1578_proped = 0;
    px_srcline(371);
    _v1568 = px_int(0LL);
    px_srcline(372);
    while (px_is_truthy(px_lt(_v1568, px_call(px_get_global("len"), (LXValue[]){_v1566}, 1)))) {
        px_srcline(373);
        _v1569 = px_index(_v1566, _v1568);
        px_srcline(374);
        _v1570 = px_index(_v1569, px_int(0LL));
        px_srcline(375);
        if (px_is_truthy(px_eq(_v1570, px_str("VarDecl")))) {
            px_srcline(376);
            _v1571 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1569, px_int(2LL))}, 1);
            px_srcline(377);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1567, _v1571}, 2)))) {
                px_srcline(378);
                (void)(px_method(_v1567, "append", (LXValue[]){_v1571}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1570, px_str("FuncDef")))) {
            px_srcline(380);
            _v1572 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1569, px_int(1LL))}, 1);
            px_srcline(381);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1567, _v1572}, 2)))) {
                px_srcline(382);
                (void)(px_method(_v1567, "append", (LXValue[]){_v1572}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1570, px_str("ChanDecl")))) {
            px_srcline(384);
            _v1573 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1569, px_int(1LL))}, 1);
            px_srcline(385);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1567, _v1573}, 2)))) {
                px_srcline(386);
                (void)(px_method(_v1567, "append", (LXValue[]){_v1573}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1570, px_str("If")))) {
            px_srcline(388);
            _v1574 = px_index(_v1569, px_int(1LL));
            px_srcline(389);
            _v1575 = px_int(0LL);
            px_srcline(390);
            while (px_is_truthy(px_lt(_v1575, px_call(px_get_global("len"), (LXValue[]){_v1574}, 1)))) {
                px_srcline(391);
                (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(px_index(_v1574, _v1575), px_int(1LL)), _v1567}, 2));
                px_srcline(392);
                 _v1575 = px_add(_v1575, px_int(1LL));
            }
            px_srcline(393);
            if (px_is_truthy(px_ne(px_index(_v1569, px_int(2LL)), px_null()))) {
                px_srcline(394);
                (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1569, px_int(2LL)), _v1567}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1570, px_str("For")))) {
            px_srcline(396);
            _v1576 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1569, px_int(1LL))}, 1);
            px_srcline(397);
            _v1577 = px_int(0LL);
            px_srcline(398);
            while (px_is_truthy(px_lt(_v1577, px_call(px_get_global("len"), (LXValue[]){_v1576}, 1)))) {
                px_srcline(399);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1567, px_index(_v1576, _v1577)}, 2)))) {
                    px_srcline(400);
                    (void)(px_method(_v1567, "append", (LXValue[]){px_index(_v1576, _v1577)}, 1));
                }
                px_srcline(401);
                 _v1577 = px_add(_v1577, px_int(1LL));
            }
            px_srcline(402);
            (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1569, px_int(3LL)), _v1567}, 2));
        }
        else if (px_is_truthy(px_eq(_v1570, px_str("While")))) {
            px_srcline(404);
            (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1569, px_int(2LL)), _v1567}, 2));
        }
        px_srcline(405);
         _v1568 = px_add(_v1568, px_int(1LL));
    }
px_err_1578:
    if (px_err_1578_proped) return px_err_1578_val;
    return px_null();
}

static LXValue fn_bc_emit_null(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_null");
    LXValue _v1579 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1580 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1581 = px_null();
    LXValue px_err_1582_val = px_null();
    int px_err_1582_proped = 0;
    px_srcline(408);
    _v1581 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(409);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1579, px_str("LOADK"), _v1580, _v1581, px_int(0LL)}, 5));
px_err_1582:
    if (px_err_1582_proped) return px_err_1582_val;
    return px_null();
}

static LXValue fn_bc_emit_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_int");
    LXValue _v1583 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1584 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1585 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1586 = px_null();
    LXValue px_err_1587_val = px_null();
    int px_err_1587_proped = 0;
    px_srcline(412);
    if (px_is_truthy(({ LXValue _t1588 = px_ge(_v1585, px_neg(px_int(32768LL))); px_is_truthy(_t1588) ? px_le(_v1585, px_int(32767LL)) : _t1588; }))) {
        px_srcline(413);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1583, px_str("IMM"), _v1584, _v1585, px_int(0LL)}, 5));
    }
    else {
        px_srcline(415);
        _v1586 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("int"), _v1585, px_float(0), px_str("")}, 5);
        px_srcline(416);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1583, px_str("LOADK"), _v1584, _v1586, px_int(0LL)}, 5));
    }
px_err_1587:
    if (px_err_1587_proped) return px_err_1587_val;
    return px_null();
}

static LXValue fn_bc_neg_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_neg_float");
    LXValue _v1589 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1590_val = px_null();
    int px_err_1590_proped = 0;
    px_srcline(423);
    return px_call(px_get_global("bits_to_float64"), (LXValue[]){px_bitxor(px_call(px_get_global("float64_bits"), (LXValue[]){_v1589}, 1), px_sub(px_sub(px_int(0LL), px_int(9223372036854775807LL)), px_int(1LL)))}, 1);
px_err_1590:
    if (px_err_1590_proped) return px_err_1590_val;
    return px_null();
}

static LXValue fn_bc_emit_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_float");
    LXValue _v1591 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1592 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1593 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1594 = px_null();
    LXValue px_err_1595_val = px_null();
    int px_err_1595_proped = 0;
    px_srcline(426);
    _v1594 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("float"), px_int(0LL), _v1593, px_str("")}, 5);
    px_srcline(427);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1591, px_str("LOADK"), _v1592, _v1594, px_int(0LL)}, 5));
px_err_1595:
    if (px_err_1595_proped) return px_err_1595_val;
    return px_null();
}

static LXValue fn_bc_unop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_unop_op");
    LXValue _v1596 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1597_val = px_null();
    int px_err_1597_proped = 0;
    px_srcline(430);
    if (px_is_truthy(px_eq(_v1596, px_str("Neg")))) {
        px_srcline(431);
        return px_str("NEG");
    }
    px_srcline(432);
    if (px_is_truthy(px_eq(_v1596, px_str("Not")))) {
        px_srcline(433);
        return px_str("NOT");
    }
    px_srcline(434);
    if (px_is_truthy(px_eq(_v1596, px_str("BitNot")))) {
        px_srcline(435);
        return px_str("BITNOT");
    }
    px_srcline(436);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_unop_op 未知一元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1596}, 1))}, 1));
px_err_1597:
    if (px_err_1597_proped) return px_err_1597_val;
    return px_null();
}

static LXValue fn_bc_binop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_binop_op");
    LXValue _v1598 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1599_val = px_null();
    int px_err_1599_proped = 0;
    px_srcline(438);
    if (px_is_truthy(px_eq(_v1598, px_str("Add")))) {
        px_srcline(439);
        return px_str("ADD");
    }
    px_srcline(440);
    if (px_is_truthy(px_eq(_v1598, px_str("Sub")))) {
        px_srcline(441);
        return px_str("SUB");
    }
    px_srcline(442);
    if (px_is_truthy(px_eq(_v1598, px_str("Mul")))) {
        px_srcline(443);
        return px_str("MUL");
    }
    px_srcline(444);
    if (px_is_truthy(px_eq(_v1598, px_str("Div")))) {
        px_srcline(445);
        return px_str("DIV");
    }
    px_srcline(446);
    if (px_is_truthy(px_eq(_v1598, px_str("IntDiv")))) {
        px_srcline(447);
        return px_str("IDIV");
    }
    px_srcline(448);
    if (px_is_truthy(px_eq(_v1598, px_str("Mod")))) {
        px_srcline(449);
        return px_str("MOD");
    }
    px_srcline(450);
    if (px_is_truthy(px_eq(_v1598, px_str("Pow")))) {
        px_srcline(451);
        return px_str("POW");
    }
    px_srcline(452);
    if (px_is_truthy(px_eq(_v1598, px_str("Eq")))) {
        px_srcline(453);
        return px_str("EQ");
    }
    px_srcline(454);
    if (px_is_truthy(px_eq(_v1598, px_str("Ne")))) {
        px_srcline(455);
        return px_str("NE");
    }
    px_srcline(456);
    if (px_is_truthy(px_eq(_v1598, px_str("Lt")))) {
        px_srcline(457);
        return px_str("LT");
    }
    px_srcline(458);
    if (px_is_truthy(px_eq(_v1598, px_str("Le")))) {
        px_srcline(459);
        return px_str("LE");
    }
    px_srcline(460);
    if (px_is_truthy(px_eq(_v1598, px_str("Gt")))) {
        px_srcline(461);
        return px_str("GT");
    }
    px_srcline(462);
    if (px_is_truthy(px_eq(_v1598, px_str("Ge")))) {
        px_srcline(463);
        return px_str("GE");
    }
    px_srcline(464);
    if (px_is_truthy(px_eq(_v1598, px_str("BitAnd")))) {
        px_srcline(465);
        return px_str("BITAND");
    }
    px_srcline(466);
    if (px_is_truthy(px_eq(_v1598, px_str("BitOr")))) {
        px_srcline(467);
        return px_str("BITOR");
    }
    px_srcline(468);
    if (px_is_truthy(px_eq(_v1598, px_str("BitXor")))) {
        px_srcline(469);
        return px_str("BITXOR");
    }
    px_srcline(470);
    if (px_is_truthy(px_eq(_v1598, px_str("Shl")))) {
        px_srcline(471);
        return px_str("SHL");
    }
    px_srcline(472);
    if (px_is_truthy(px_eq(_v1598, px_str("Shr")))) {
        px_srcline(473);
        return px_str("SHR");
    }
    px_srcline(474);
    if (px_is_truthy(px_eq(_v1598, px_str("ShrU")))) {
        px_srcline(475);
        return px_str("SHRU");
    }
    px_srcline(476);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_binop_op 未知二元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1598}, 1))}, 1));
px_err_1599:
    if (px_err_1599_proped) return px_err_1599_val;
    return px_null();
}

static LXValue fn_bc_emit_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_expr");
    LXValue _v1600 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1601 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1602 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1603 = px_null();
    LXValue _v1604 = px_null();
    LXValue _v1605 = px_null();
    LXValue _v1606 = px_null();
    LXValue _v1607 = px_null();
    LXValue _v1608 = px_null();
    LXValue _v1609 = px_null();
    LXValue _v1610 = px_null();
    LXValue _v1611 = px_null();
    LXValue _v1612 = px_null();
    LXValue _v1613 = px_null();
    LXValue _v1614 = px_null();
    LXValue _v1615 = px_null();
    LXValue _v1616 = px_null();
    LXValue _v1617 = px_null();
    LXValue _v1618 = px_null();
    LXValue _v1619 = px_null();
    LXValue _v1620 = px_null();
    LXValue _v1621 = px_null();
    LXValue _v1622 = px_null();
    LXValue _v1623 = px_null();
    LXValue _v1624 = px_null();
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
    LXValue _v1635 = px_null();
    LXValue _v1636 = px_null();
    LXValue _v1637 = px_null();
    LXValue _v1638 = px_null();
    LXValue _v1639 = px_null();
    LXValue _v1640 = px_null();
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
    LXValue _v1654 = px_null();
    LXValue _v1655 = px_null();
    LXValue _v1656 = px_null();
    LXValue px_err_1657_val = px_null();
    int px_err_1657_proped = 0;
    px_srcline(481);
    _v1603 = px_index(_v1600, px_int(0LL));
    px_srcline(482);
    if (px_is_truthy(px_eq(_v1603, px_str("Int")))) {
        px_srcline(483);
        (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1602, _v1601, px_index(_v1600, px_int(1LL))}, 3));
        px_srcline(484);
        return _v1601;
    }
    px_srcline(485);
    if (px_is_truthy(px_eq(_v1603, px_str("Float")))) {
        px_srcline(486);
        (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1602, _v1601, px_index(_v1600, px_int(1LL))}, 3));
        px_srcline(487);
        return _v1601;
    }
    px_srcline(488);
    if (px_is_truthy(px_eq(_v1603, px_str("Unary")))) {
        px_srcline(491);
        _v1604 = px_index(_v1600, px_int(2LL));
        px_srcline(492);
        _v1605 = px_index(_v1600, px_int(1LL));
        px_srcline(493);
        if (px_is_truthy(({ LXValue _t1658 = px_eq(px_index(_v1604, px_int(0LL)), px_str("Int")); px_is_truthy(_t1658) ? px_eq(_v1605, px_str("Neg")) : _t1658; }))) {
            px_srcline(494);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1602, _v1601, px_sub(px_int(0LL), px_index(_v1604, px_int(1LL)))}, 3));
            px_srcline(495);
            return _v1601;
        }
        px_srcline(496);
        if (px_is_truthy(({ LXValue _t1659 = px_eq(px_index(_v1604, px_int(0LL)), px_str("Float")); px_is_truthy(_t1659) ? px_eq(_v1605, px_str("Neg")) : _t1659; }))) {
            px_srcline(503);
            (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1602, _v1601, px_call(px_get_global("bc_neg_float"), (LXValue[]){px_index(_v1604, px_int(1LL))}, 1)}, 3));
            px_srcline(504);
            return _v1601;
        }
        px_srcline(505);
        _v1606 = _v1601;
        px_srcline(506);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(507);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(508);
        _v1607 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(509);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1604, _v1607, _v1602}, 3));
        px_srcline(510);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_call(px_get_global("bc_unop_op"), (LXValue[]){_v1605}, 1), _v1606, _v1607, px_int(0LL)}, 5));
        px_srcline(511);
        return _v1606;
    }
    px_srcline(512);
    if (px_is_truthy(px_eq(_v1603, px_str("Str")))) {
        px_srcline(513);
        _v1608 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1600, px_int(1LL))}, 1)}, 5);
        px_srcline(514);
        if (px_is_truthy(px_ge(_v1601, px_int(0LL)))) {
            px_srcline(515);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("LOADK"), _v1601, _v1608, px_int(0LL)}, 5));
        }
        px_srcline(516);
        return _v1601;
    }
    px_srcline(517);
    if (px_is_truthy(px_eq(_v1603, px_str("Bool")))) {
        px_srcline(518);
        _v1609 = px_int(0LL);
        px_srcline(519);
        if (px_is_truthy(px_index(_v1600, px_int(1LL)))) {
            px_srcline(520);
             _v1609 = px_int(1LL);
        }
        px_srcline(521);
        _v1608 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("bool"), _v1609, px_float(0), px_str("")}, 5);
        px_srcline(522);
        if (px_is_truthy(px_ge(_v1601, px_int(0LL)))) {
            px_srcline(523);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("LOADK"), _v1601, _v1608, px_int(0LL)}, 5));
        }
        px_srcline(524);
        return _v1601;
    }
    px_srcline(525);
    if (px_is_truthy(px_eq(_v1603, px_str("Null")))) {
        px_srcline(526);
        if (px_is_truthy(px_ge(_v1601, px_int(0LL)))) {
            px_srcline(527);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1602, _v1601}, 2));
        }
        px_srcline(528);
        return _v1601;
    }
    px_srcline(529);
    if (px_is_truthy(px_eq(_v1603, px_str("Var")))) {
        px_srcline(530);
        _v1610 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1600, px_int(1LL))}, 1);
        px_srcline(531);
        if (px_is_truthy(px_method(px_index(_v1602, px_str("smap")), "has", (LXValue[]){_v1610}, 1))) {
            px_srcline(533);
            (void)(px_call(px_get_global("bc_load_var"), (LXValue[]){_v1602, _v1610, _v1601}, 3));
            px_srcline(534);
            return _v1601;
        }
        px_srcline(536);
        _v1611 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1610}, 2);
        px_srcline(537);
        if (px_is_truthy(px_ge(_v1601, px_int(0LL)))) {
            px_srcline(538);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("GETG"), _v1601, _v1611, px_int(0LL)}, 5));
        }
        px_srcline(539);
        return _v1601;
    }
    px_srcline(540);
    if (px_is_truthy(px_eq(_v1603, px_str("Binary")))) {
        px_srcline(542);
        _v1605 = px_index(_v1600, px_int(1LL));
        px_srcline(543);
        _v1606 = _v1601;
        px_srcline(544);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(545);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(546);
        if (px_is_truthy(px_eq(_v1605, px_str("And")))) {
            px_srcline(548);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1606, _v1602}, 3));
            px_srcline(549);
            _v1612 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
            px_srcline(550);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPF"), _v1606, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(551);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(3LL)), _v1606, _v1602}, 3));
            px_srcline(552);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1612, px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1)}, 3));
            px_srcline(553);
            return _v1606;
        }
        px_srcline(554);
        if (px_is_truthy(px_eq(_v1605, px_str("Or")))) {
            px_srcline(556);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1606, _v1602}, 3));
            px_srcline(557);
            _v1612 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
            px_srcline(558);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPT"), _v1606, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(559);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(3LL)), _v1606, _v1602}, 3));
            px_srcline(560);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1612, px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1)}, 3));
            px_srcline(561);
            return _v1606;
        }
        px_srcline(562);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1606, _v1602}, 3));
        px_srcline(563);
        _v1613 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(564);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(3LL)), _v1613, _v1602}, 3));
        px_srcline(565);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_call(px_get_global("bc_binop_op"), (LXValue[]){_v1605}, 1), _v1606, _v1606, _v1613}, 5));
        px_srcline(566);
        return _v1606;
    }
    px_srcline(567);
    if (px_is_truthy(px_eq(_v1603, px_str("NullCoalesce")))) {
        px_srcline(569);
        _v1606 = _v1601;
        px_srcline(570);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(571);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(572);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1606, _v1602}, 3));
        px_srcline(573);
        _v1614 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(574);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1602, _v1614}, 2));
        px_srcline(575);
        _v1615 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(576);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("EQ"), _v1615, _v1606, _v1614}, 5));
        px_srcline(577);
        _v1612 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
        px_srcline(578);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPF"), _v1615, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(579);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1606, _v1602}, 3));
        px_srcline(580);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1612, px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1)}, 3));
        px_srcline(581);
        return _v1606;
    }
    px_srcline(582);
    if (px_is_truthy(px_eq(_v1603, px_str("IfExpr")))) {
        px_srcline(584);
        _v1606 = _v1601;
        px_srcline(585);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(586);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(587);
        _v1616 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(588);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1616, _v1602}, 3));
        px_srcline(589);
        _v1617 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
        px_srcline(590);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPF"), _v1616, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(591);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1606, _v1602}, 3));
        px_srcline(592);
        _v1618 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
        px_srcline(593);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(594);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1617, px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1)}, 3));
        px_srcline(595);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(3LL)), _v1606, _v1602}, 3));
        px_srcline(596);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1618, px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1)}, 3));
        px_srcline(597);
        return _v1606;
    }
    px_srcline(598);
    if (px_is_truthy(px_eq(_v1603, px_str("Try")))) {
        px_srcline(602);
        _v1606 = _v1601;
        px_srcline(603);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(604);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(605);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1606, _v1602}, 3));
        px_srcline(606);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("TRY"), _v1606, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(607);
        return _v1606;
    }
    px_srcline(608);
    if (px_is_truthy(px_eq(_v1603, px_str("ForceUnwrap")))) {
        px_srcline(610);
        _v1606 = _v1601;
        px_srcline(611);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(612);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(613);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1606, _v1602}, 3));
        px_srcline(614);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("FORCE"), _v1606, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(615);
        return _v1606;
    }
    px_srcline(616);
    if (px_is_truthy(({ LXValue _t1660 = px_eq(_v1603, px_str("List")); px_is_truthy(_t1660) ? _t1660 : px_eq(_v1603, px_str("Tuple")); }))) {
        px_srcline(618);
        _v1619 = px_index(_v1600, px_int(1LL));
        px_srcline(619);
        _v1620 = px_call(px_get_global("len"), (LXValue[]){_v1619}, 1);
        px_srcline(620);
        _v1606 = _v1601;
        px_srcline(621);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(622);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(623);
        _v1621 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(624);
        while (px_is_truthy(px_lt(px_index(_v1602, px_str("next_slot")), px_add(_v1621, _v1620)))) {
            px_srcline(625);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1));
        }
        px_srcline(626);
        _v1622 = px_int(0LL);
        px_srcline(627);
        while (px_is_truthy(px_lt(_v1622, _v1620))) {
            px_srcline(628);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1619, _v1622), px_add(_v1621, _v1622), _v1602}, 3));
            px_srcline(629);
             _v1622 = px_add(_v1622, px_int(1LL));
        }
        px_srcline(630);
        if (px_is_truthy(px_eq(_v1603, px_str("List")))) {
            px_srcline(631);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWLIST"), _v1606, _v1621, _v1620}, 5));
        }
        else {
            px_srcline(633);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWTUPLE"), _v1606, _v1621, _v1620}, 5));
        }
        px_srcline(634);
        return _v1606;
    }
    px_srcline(635);
    if (px_is_truthy(px_eq(_v1603, px_str("Dict")))) {
        px_srcline(637);
        _v1623 = px_index(_v1600, px_int(1LL));
        px_srcline(638);
        _v1620 = px_call(px_get_global("len"), (LXValue[]){_v1623}, 1);
        px_srcline(639);
        _v1606 = _v1601;
        px_srcline(640);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(641);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(642);
        _v1621 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(643);
        while (px_is_truthy(px_lt(px_index(_v1602, px_str("next_slot")), px_add(_v1621, px_mul(px_int(2LL), _v1620))))) {
            px_srcline(644);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1));
        }
        px_srcline(645);
        _v1624 = px_int(0LL);
        px_srcline(646);
        while (px_is_truthy(px_lt(_v1624, _v1620))) {
            px_srcline(647);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1623, _v1624), px_int(0LL)), px_add(_v1621, px_mul(px_int(2LL), _v1624)), _v1602}, 3));
            px_srcline(648);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1623, _v1624), px_int(1LL)), px_add(px_add(_v1621, px_mul(px_int(2LL), _v1624)), px_int(1LL)), _v1602}, 3));
            px_srcline(649);
             _v1624 = px_add(_v1624, px_int(1LL));
        }
        px_srcline(650);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWDICT"), _v1606, _v1621, _v1620}, 5));
        px_srcline(651);
        return _v1606;
    }
    px_srcline(652);
    if (px_is_truthy(px_eq(_v1603, px_str("Index")))) {
        px_srcline(654);
        _v1606 = _v1601;
        px_srcline(655);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(656);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(657);
        _v1625 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(658);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1625, _v1602}, 3));
        px_srcline(659);
        _v1626 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(660);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1626, _v1602}, 3));
        px_srcline(661);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("INDEX"), _v1606, _v1625, _v1626}, 5));
        px_srcline(662);
        return _v1606;
    }
    px_srcline(663);
    if (px_is_truthy(px_eq(_v1603, px_str("Slice")))) {
        px_srcline(665);
        _v1606 = _v1601;
        px_srcline(666);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(667);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(668);
        _v1625 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(669);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1625, _v1602}, 3));
        px_srcline(670);
        _v1621 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(671);
        while (px_is_truthy(px_lt(px_index(_v1602, px_str("next_slot")), px_add(_v1621, px_int(3LL))))) {
            px_srcline(672);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1));
        }
        px_srcline(673);
        if (px_is_truthy(px_eq(px_index(_v1600, px_int(2LL)), px_null()))) {
            px_srcline(674);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1602, _v1621}, 2));
        }
        else {
            px_srcline(676);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(2LL)), _v1621, _v1602}, 3));
        }
        px_srcline(677);
        if (px_is_truthy(px_eq(px_index(_v1600, px_int(3LL)), px_null()))) {
            px_srcline(678);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1602, px_add(_v1621, px_int(1LL))}, 2));
        }
        else {
            px_srcline(680);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(3LL)), px_add(_v1621, px_int(1LL)), _v1602}, 3));
        }
        px_srcline(681);
        if (px_is_truthy(px_eq(px_index(_v1600, px_int(4LL)), px_null()))) {
            px_srcline(682);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1602, px_add(_v1621, px_int(2LL))}, 2));
        }
        else {
            px_srcline(684);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(4LL)), px_add(_v1621, px_int(2LL)), _v1602}, 3));
        }
        px_srcline(685);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("SLICE"), _v1606, _v1625, _v1621}, 5));
        px_srcline(686);
        return _v1606;
    }
    px_srcline(687);
    if (px_is_truthy(px_eq(_v1603, px_str("Field")))) {
        px_srcline(691);
        _v1627 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1600, px_int(2LL))}, 1);
        px_srcline(692);
        _v1628 = px_index(_v1600, px_int(1LL));
        px_srcline(693);
        if (px_is_truthy(px_eq(px_index(_v1628, px_int(0LL)), px_str("Var")))) {
            px_srcline(694);
            _v1629 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1628, px_int(1LL))}, 1);
            px_srcline(695);
            _v1630 = px_call(px_get_global("bc_const_find"), (LXValue[]){_v1629, _v1627}, 2);
            px_srcline(696);
            if (px_is_truthy(px_ne(_v1630, px_null()))) {
                px_srcline(697);
                _v1606 = _v1601;
                px_srcline(698);
                if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
                    px_srcline(699);
                     _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
                }
                px_srcline(700);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1630, _v1606, _v1602}, 3));
                px_srcline(701);
                return _v1606;
            }
            px_srcline(702);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1629, _v1627}, 2))) {
                px_srcline(703);
                _v1606 = _v1601;
                px_srcline(704);
                if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
                    px_srcline(705);
                     _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
                }
                px_srcline(706);
                _v1614 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1629}, 2);
                px_srcline(707);
                _v1631 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1627}, 2);
                px_srcline(708);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWENUM"), _v1606, _v1614, _v1631}, 5));
                px_srcline(709);
                return _v1606;
            }
        }
        px_srcline(710);
        _v1606 = _v1601;
        px_srcline(711);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(712);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(713);
        _v1625 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(714);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1625, _v1602}, 3));
        px_srcline(715);
        _v1632 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1627}, 2);
        px_srcline(716);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("GETF"), _v1606, _v1625, _v1632}, 5));
        px_srcline(717);
        return _v1606;
    }
    px_srcline(718);
    if (px_is_truthy(px_eq(_v1603, px_str("OptionalField")))) {
        px_srcline(720);
        _v1606 = _v1601;
        px_srcline(721);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(722);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(723);
        _v1625 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(724);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1625, _v1602}, 3));
        px_srcline(725);
        _v1632 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1600, px_int(2LL))}, 1)}, 2);
        px_srcline(726);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("GETF_OPT"), _v1606, _v1625, _v1632}, 5));
        px_srcline(727);
        return _v1606;
    }
    px_srcline(728);
    if (px_is_truthy(px_eq(_v1603, px_str("Call")))) {
        px_srcline(731);
        _v1633 = px_index(_v1600, px_int(1LL));
        px_srcline(735);
        (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v1633, px_index(_v1600, px_int(2LL))}, 2));
        px_srcline(736);
        if (px_is_truthy(px_eq(px_index(_v1633, px_int(0LL)), px_str("Var")))) {
            px_srcline(737);
            _v1634 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1633, px_int(1LL))}, 1);
            px_srcline(738);
            _v1635 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1634}, 1);
            px_srcline(739);
            if (px_is_truthy(px_ge(_v1635, px_int(0LL)))) {
                px_srcline(740);
                return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1634, _v1635, px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 5);
            }
            px_srcline(741);
            if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1634}, 1))) {
                px_srcline(742);
                return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1634, px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 4);
            }
        }
        px_srcline(743);
        if (px_is_truthy(px_eq(px_index(_v1633, px_int(0LL)), px_str("Field")))) {
            px_srcline(744);
            return px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1633, px_int(1LL)), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1633, px_int(2LL))}, 1), px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 5);
        }
        px_srcline(745);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1633, px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 4);
    }
    px_srcline(746);
    if (px_is_truthy(px_eq(_v1603, px_str("Constructor")))) {
        px_srcline(748);
        _v1634 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1600, px_int(1LL))}, 1);
        px_srcline(749);
        _v1635 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1634}, 1);
        px_srcline(750);
        if (px_is_truthy(px_ge(_v1635, px_int(0LL)))) {
            px_srcline(751);
            return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1634, _v1635, px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 5);
        }
        px_srcline(752);
        if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1634}, 1))) {
            px_srcline(753);
            return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1634, px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 4);
        }
        px_srcline(755);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1600, px_int(1LL)), px_index(_v1600, px_int(2LL)), _v1601, _v1602}, 4);
    }
    px_srcline(756);
    if (px_is_truthy(px_eq(_v1603, px_str("Pipe")))) {
        px_srcline(758);
        _v1636 = px_index(_v1600, px_int(1LL));
        px_srcline(759);
        _v1637 = px_index(_v1600, px_int(2LL));
        px_srcline(760);
        if (px_is_truthy(px_eq(px_index(_v1637, px_int(0LL)), px_str("Call")))) {
            px_srcline(761);
            _v1638 = px_list_n((LXValue[]){}, 0);
            px_srcline(762);
            (void)(px_method(_v1638, "append", (LXValue[]){_v1636}, 1));
            px_srcline(763);
            _v1639 = px_int(0LL);
            px_srcline(764);
            while (px_is_truthy(px_lt(_v1639, px_call(px_get_global("len"), (LXValue[]){px_index(_v1637, px_int(2LL))}, 1)))) {
                px_srcline(765);
                (void)(px_method(_v1638, "append", (LXValue[]){px_index(px_index(_v1637, px_int(2LL)), _v1639)}, 1));
                px_srcline(766);
                 _v1639 = px_add(_v1639, px_int(1LL));
            }
            px_srcline(767);
            return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1637, px_int(1LL)), _v1638, _v1601, _v1602}, 4);
        }
        px_srcline(768);
        _v1638 = px_list_n((LXValue[]){_v1636}, 1);
        px_srcline(769);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1637, _v1638, _v1601, _v1602}, 4);
    }
    px_srcline(770);
    if (px_is_truthy(px_eq(_v1603, px_str("ListComp")))) {
        px_srcline(772);
        _v1640 = px_index(_v1600, px_int(2LL));
        px_srcline(773);
        _v1606 = _v1601;
        px_srcline(774);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(775);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(776);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWLIST"), _v1606, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(777);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1602, px_str("push"), px_list_n((LXValue[]){px_index(_v1600, px_int(1LL))}, 1), px_index(_v1600, px_int(3LL)), _v1606, _v1640, px_int(0LL)}, 7));
        px_srcline(778);
        return _v1606;
    }
    px_srcline(779);
    if (px_is_truthy(px_eq(_v1603, px_str("DictComp")))) {
        px_srcline(782);
        _v1640 = px_index(_v1600, px_int(3LL));
        px_srcline(783);
        _v1606 = _v1601;
        px_srcline(784);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(785);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(786);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("NEWDICT"), _v1606, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(787);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1602, px_str("dict"), px_list_n((LXValue[]){px_index(_v1600, px_int(1LL)), px_index(_v1600, px_int(2LL))}, 2), px_index(_v1600, px_int(4LL)), _v1606, _v1640, px_int(0LL)}, 7));
        px_srcline(788);
        return _v1606;
    }
    px_srcline(789);
    if (px_is_truthy(px_eq(_v1603, px_str("GenExp")))) {
        px_srcline(790);
        return px_call(px_get_global("bc_emit_genexp"), (LXValue[]){_v1600, _v1601, _v1602}, 3);
    }
    px_srcline(791);
    if (px_is_truthy(px_eq(_v1603, px_str("Match")))) {
        px_srcline(794);
        _v1606 = _v1601;
        px_srcline(795);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(796);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(797);
        _v1641 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        px_srcline(798);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1600, px_int(1LL)), _v1641, _v1602}, 3));
        px_srcline(799);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("MOV"), _v1606, _v1641, px_int(0LL)}, 5));
        px_srcline(800);
        _v1642 = px_index(_v1600, px_int(2LL));
        px_srcline(801);
        _v1643 = px_list_n((LXValue[]){}, 0);
        px_srcline(802);
        _v1644 = px_int(0LL);
        px_srcline(803);
        while (px_is_truthy(px_lt(_v1644, px_call(px_get_global("len"), (LXValue[]){_v1642}, 1)))) {
            px_srcline(804);
            _v1645 = px_index(_v1642, _v1644);
            px_srcline(805);
            _v1646 = px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1645, px_int(1LL)), _v1641, _v1602}, 3);
            px_srcline(806);
            _v1647 = px_neg(px_int(1LL));
            px_srcline(807);
            _v1648 = px_neg(px_int(1LL));
            px_srcline(808);
            if (px_is_truthy(px_eq(_v1646, px_null()))) {
                px_srcline(810);
                if (px_is_truthy(px_ne(px_index(_v1645, px_int(2LL)), px_null()))) {
                    px_srcline(811);
                    _v1649 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
                    px_srcline(812);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1645, px_int(2LL)), _v1649, _v1602}, 3));
                    px_srcline(813);
                    _v1650 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
                    px_srcline(814);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPF"), _v1649, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(815);
                     _v1648 = _v1650;
                }
            }
            else {
                px_srcline(817);
                _v1617 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
                px_srcline(818);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPF"), _v1646, px_int(0LL), px_int(0LL)}, 5));
                px_srcline(819);
                 _v1647 = _v1617;
                px_srcline(820);
                if (px_is_truthy(px_ne(px_index(_v1645, px_int(2LL)), px_null()))) {
                    px_srcline(821);
                    _v1649 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
                    px_srcline(822);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1645, px_int(2LL)), _v1649, _v1602}, 3));
                    px_srcline(823);
                    _v1650 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
                    px_srcline(824);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMPF"), _v1649, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(825);
                     _v1648 = _v1650;
                }
            }
            px_srcline(826);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1645, px_int(3LL)), _v1606, _v1602}, 3));
            px_srcline(827);
            _v1618 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
            px_srcline(828);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1602, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(829);
            _v1651 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
            px_srcline(830);
            if (px_is_truthy(px_ge(_v1647, px_int(0LL)))) {
                px_srcline(831);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1647, _v1651}, 3));
            }
            px_srcline(832);
            if (px_is_truthy(px_ge(_v1648, px_int(0LL)))) {
                px_srcline(833);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, _v1648, _v1651}, 3));
            }
            px_srcline(834);
            (void)(px_method(_v1643, "append", (LXValue[]){_v1618}, 1));
            px_srcline(835);
             _v1644 = px_add(_v1644, px_int(1LL));
        }
        px_srcline(836);
        _v1652 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1602, px_str("bc"))}, 1);
        px_srcline(837);
        _v1653 = px_int(0LL);
        px_srcline(838);
        while (px_is_truthy(px_lt(_v1653, px_call(px_get_global("len"), (LXValue[]){_v1643}, 1)))) {
            px_srcline(839);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1602, px_index(_v1643, _v1653), _v1652}, 3));
            px_srcline(840);
             _v1653 = px_add(_v1653, px_int(1LL));
        }
        px_srcline(841);
        return _v1606;
    }
    px_srcline(842);
    if (px_is_truthy(px_eq(_v1603, px_str("Block")))) {
        px_srcline(846);
        _v1606 = _v1601;
        px_srcline(847);
        if (px_is_truthy(px_lt(_v1606, px_int(0LL)))) {
            px_srcline(848);
             _v1606 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1602}, 1);
        }
        px_srcline(849);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1602, _v1606}, 2));
        px_srcline(850);
        _v1654 = px_index(_v1600, px_int(1LL));
        px_srcline(851);
        _v1655 = px_int(0LL);
        px_srcline(852);
        while (px_is_truthy(px_lt(_v1655, px_call(px_get_global("len"), (LXValue[]){_v1654}, 1)))) {
            px_srcline(853);
            _v1656 = px_index(_v1654, _v1655);
            px_srcline(854);
            if (px_is_truthy(px_eq(px_index(_v1656, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(855);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1656, px_int(1LL)), _v1606, _v1602}, 3));
            }
            else {
                px_srcline(857);
                (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){_v1656, _v1602}, 2));
            }
            px_srcline(858);
             _v1655 = px_add(_v1655, px_int(1LL));
        }
        px_srcline(859);
        return _v1606;
    }
    px_srcline(860);
    if (px_is_truthy(px_eq(_v1603, px_str("Closure")))) {
        px_srcline(863);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1600, px_int(1LL)), px_index(_v1600, px_int(3LL)), _v1601, _v1602}, 4));
        px_srcline(864);
        return _v1601;
    }
    px_srcline(865);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_expr 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1600}, 1))}, 1));
px_err_1657:
    if (px_err_1657_proped) return px_err_1657_val;
    return px_null();
}

static LXValue fn_bc_emit_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_call");
    LXValue _v1661 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1662 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1663 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1664 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1665 = px_null();
    LXValue _v1666 = px_null();
    LXValue _v1667 = px_null();
    LXValue _v1668 = px_null();
    LXValue _v1669 = px_null();
    LXValue px_err_1670_val = px_null();
    int px_err_1670_proped = 0;
    px_srcline(869);
    _v1665 = _v1663;
    px_srcline(870);
    if (px_is_truthy(px_lt(_v1665, px_int(0LL)))) {
        px_srcline(871);
         _v1665 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
    }
    px_srcline(872);
    _v1666 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1);
    px_srcline(873);
    _v1667 = px_call(px_get_global("len"), (LXValue[]){_v1662}, 1);
    px_srcline(874);
    _v1668 = px_add(px_add(_v1666, px_int(1LL)), _v1667);
    px_srcline(875);
    while (px_is_truthy(px_lt(px_index(_v1664, px_str("next_slot")), _v1668))) {
        px_srcline(876);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1664}, 1));
    }
    px_srcline(877);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1661, _v1666, _v1664}, 3));
    px_srcline(878);
    _v1669 = px_int(0LL);
    px_srcline(879);
    while (px_is_truthy(px_lt(_v1669, _v1667))) {
        px_srcline(880);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1662, _v1669), px_add(px_add(_v1666, px_int(1LL)), _v1669), _v1664}, 3));
        px_srcline(881);
         _v1669 = px_add(_v1669, px_int(1LL));
    }
    px_srcline(882);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1664, px_str("CALL"), _v1665, _v1666, _v1667}, 5));
    px_srcline(883);
    return _v1665;
px_err_1670:
    if (px_err_1670_proped) return px_err_1670_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall");
    LXValue _v1671 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1672 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1673 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1674 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1675 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1676 = px_null();
    LXValue _v1677 = px_null();
    LXValue _v1678 = px_null();
    LXValue _v1679 = px_null();
    LXValue _v1680 = px_null();
    LXValue _v1681 = px_null();
    LXValue px_err_1682_val = px_null();
    int px_err_1682_proped = 0;
    px_srcline(889);
    _v1676 = _v1674;
    px_srcline(890);
    if (px_is_truthy(px_lt(_v1676, px_int(0LL)))) {
        px_srcline(891);
         _v1676 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1675}, 1);
    }
    px_srcline(892);
    _v1677 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1675}, 1);
    px_srcline(893);
    _v1678 = px_call(px_get_global("len"), (LXValue[]){_v1673}, 1);
    px_srcline(894);
    _v1679 = px_add(px_add(_v1677, px_int(1LL)), _v1678);
    px_srcline(895);
    while (px_is_truthy(px_lt(px_index(_v1675, px_str("next_slot")), _v1679))) {
        px_srcline(896);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1675}, 1));
    }
    px_srcline(897);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1671, _v1677, _v1675}, 3));
    px_srcline(898);
    _v1680 = px_int(0LL);
    px_srcline(899);
    while (px_is_truthy(px_lt(_v1680, _v1678))) {
        px_srcline(900);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1673, _v1680), px_add(px_add(_v1677, px_int(1LL)), _v1680), _v1675}, 3));
        px_srcline(901);
         _v1680 = px_add(_v1680, px_int(1LL));
    }
    px_srcline(902);
    _v1681 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1672}, 2);
    px_srcline(903);
    (void)(px_method(px_index(_v1675, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1678, _v1676, _v1677, _v1681}, 5)}, 1));
    px_srcline(904);
    return _v1676;
px_err_1682:
    if (px_err_1682_proped) return px_err_1682_val;
    return px_null();
}

static LXValue fn_bc_emit_struct_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_struct_new");
    LXValue _v1683 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1684 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1685 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1686 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1687 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1688 = px_null();
    LXValue _v1689 = px_null();
    LXValue _v1690 = px_null();
    LXValue _v1691 = px_null();
    LXValue _v1692 = px_null();
    LXValue px_err_1693_val = px_null();
    int px_err_1693_proped = 0;
    px_srcline(908);
    _v1688 = px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1684);
    px_srcline(909);
    _v1689 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1688, px_str("fnames"))}, 1);
    px_srcline(910);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1685}, 1), _v1689))) {
        px_srcline(911);
        (void)(px_call(px_get_global("panic"), (LXValue[]){({ LXValue _s207 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v1683), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){_v1689}, 1)), px_str(" 个字段，给出 ")); LXValue _s208 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1685}, 1)}, 1); px_add(_s207, _s208); })}, 1));
    }
    px_srcline(912);
    _v1690 = _v1686;
    px_srcline(913);
    if (px_is_truthy(px_lt(_v1690, px_int(0LL)))) {
        px_srcline(914);
         _v1690 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1687}, 1);
    }
    px_srcline(915);
    _v1691 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1687}, 1);
    px_srcline(916);
    while (px_is_truthy(px_lt(px_index(_v1687, px_str("next_slot")), px_add(_v1691, _v1689)))) {
        px_srcline(917);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1687}, 1));
    }
    px_srcline(918);
    _v1692 = px_int(0LL);
    px_srcline(919);
    while (px_is_truthy(px_lt(_v1692, _v1689))) {
        px_srcline(920);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1685, _v1692), px_add(_v1691, _v1692), _v1687}, 3));
        px_srcline(921);
         _v1692 = px_add(_v1692, px_int(1LL));
    }
    px_srcline(922);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1687, px_str("NEWSTRUCT"), _v1690, _v1684, _v1691}, 5));
    px_srcline(923);
    return _v1690;
px_err_1693:
    if (px_err_1693_proped) return px_err_1693_val;
    return px_null();
}

static LXValue fn_bc_emit_enum_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_enum_new");
    LXValue _v1694 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1695 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1696 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1697 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1698 = px_null();
    LXValue _v1699 = px_null();
    LXValue _v1700 = px_null();
    LXValue _v1701 = px_null();
    LXValue _v1702 = px_null();
    LXValue _v1703 = px_null();
    LXValue px_err_1704_val = px_null();
    int px_err_1704_proped = 0;
    px_srcline(926);
    _v1698 = px_null();
    px_srcline(927);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1695}, 1), px_int(1LL)))) {
        px_srcline(928);
        _v1699 = px_index(_v1695, px_int(0LL));
        px_srcline(929);
        if (px_is_truthy(px_eq(px_index(_v1699, px_int(0LL)), px_str("Var")))) {
            px_srcline(930);
            _v1700 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1699, px_int(1LL))}, 1);
            px_srcline(931);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1694, _v1700}, 2))) {
                px_srcline(932);
                 _v1698 = _v1700;
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v1699, px_int(0LL)), px_str("Str")))) {
            px_srcline(934);
             _v1698 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1699, px_int(1LL))}, 1);
        }
    }
    px_srcline(935);
    if (px_is_truthy(px_eq(_v1698, px_null()))) {
        px_srcline(936);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit enum 构造未实现（仅支持 Color(Variant)/Color(\"Variant\"): "), px_call(px_get_global("str"), (LXValue[]){_v1694}, 1))}, 1));
    }
    px_srcline(937);
    _v1701 = _v1696;
    px_srcline(938);
    if (px_is_truthy(px_lt(_v1701, px_int(0LL)))) {
        px_srcline(939);
         _v1701 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1697}, 1);
    }
    px_srcline(940);
    _v1702 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1694}, 2);
    px_srcline(941);
    _v1703 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1698}, 2);
    px_srcline(942);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1697, px_str("NEWENUM"), _v1701, _v1702, _v1703}, 5));
    px_srcline(943);
    return _v1701;
px_err_1704:
    if (px_err_1704_proped) return px_err_1704_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall_slot");
    LXValue _v1705 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1706 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1707 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1708 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1709 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1710 = px_null();
    LXValue _v1711 = px_null();
    LXValue _v1712 = px_null();
    LXValue _v1713 = px_null();
    LXValue _v1714 = px_null();
    LXValue _v1715 = px_null();
    LXValue px_err_1716_val = px_null();
    int px_err_1716_proped = 0;
    px_srcline(950);
    _v1710 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1705}, 1);
    px_srcline(951);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1705, px_str("MOV"), _v1710, _v1706, px_int(0LL)}, 5));
    px_srcline(952);
    _v1711 = px_call(px_get_global("len"), (LXValue[]){_v1708}, 1);
    px_srcline(953);
    _v1712 = px_add(px_add(_v1710, px_int(1LL)), _v1711);
    px_srcline(954);
    while (px_is_truthy(px_lt(px_index(_v1705, px_str("next_slot")), _v1712))) {
        px_srcline(955);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1705}, 1));
    }
    px_srcline(956);
    _v1713 = px_int(0LL);
    px_srcline(957);
    while (px_is_truthy(px_lt(_v1713, _v1711))) {
        px_srcline(958);
        _v1714 = px_index(_v1708, _v1713);
        px_srcline(959);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1714}, 1), px_str("int")))) {
            px_srcline(960);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1705, px_str("MOV"), px_add(px_add(_v1710, px_int(1LL)), _v1713), _v1714, px_int(0LL)}, 5));
        }
        else {
            px_srcline(962);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1714, px_add(px_add(_v1710, px_int(1LL)), _v1713), _v1705}, 3));
        }
        px_srcline(963);
         _v1713 = px_add(_v1713, px_int(1LL));
    }
    px_srcline(964);
    _v1715 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1707}, 2);
    px_srcline(965);
    (void)(px_method(px_index(_v1705, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1711, _v1709, _v1710, _v1715}, 5)}, 1));
    px_srcline(966);
    return _v1709;
px_err_1716:
    if (px_err_1716_proped) return px_err_1716_val;
    return px_null();
}

static LXValue fn_bc_emit_push_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_push_lambda");
    LXValue _v1717 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1718 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1719 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1720 = px_null();
    LXValue _v1721 = px_null();
    LXValue _v1722 = px_null();
    LXValue _v1723 = px_null();
    LXValue _v1724 = px_null();
    LXValue _v1725 = px_null();
    LXValue _v1726 = px_null();
    LXValue _v1727 = px_null();
    LXValue _v1728 = px_null();
    LXValue _v1729 = px_null();
    LXValue _v1730 = px_null();
    LXValue px_err_1731_val = px_null();
    int px_err_1731_proped = 0;
    px_srcline(970);
    _v1720 = px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("nclosure"))}, 1)), px_str(">"));
    px_srcline(971);
    px_index_set(px_get_global("g_bcm"), px_str("nclosure"), px_add(px_index(px_get_global("g_bcm"), px_str("nclosure")), px_int(1LL)));
    px_srcline(972);
    _v1721 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1720, px_call(px_get_global("len"), (LXValue[]){_v1717}, 1)}, 2);
    px_srcline(973);
    _v1722 = px_int(0LL);
    px_srcline(974);
    while (px_is_truthy(px_lt(_v1722, px_call(px_get_global("len"), (LXValue[]){_v1717}, 1)))) {
        px_srcline(975);
        px_index_set(px_index(_v1721, px_str("smap")), px_index(_v1717, _v1722), _v1722);
        px_srcline(976);
         _v1722 = px_add(_v1722, px_int(1LL));
    }
    px_srcline(979);
    _v1723 = px_int(0LL);
    px_srcline(980);
    while (px_is_truthy(px_lt(_v1723, px_call(px_get_global("len"), (LXValue[]){_v1719}, 1)))) {
        px_srcline(981);
        _v1724 = px_index(_v1719, _v1723);
        px_srcline(982);
        px_index_set(px_index(_v1721, px_str("smap")), _v1724, px_add(px_call(px_get_global("len"), (LXValue[]){_v1717}, 1), _v1723));
        px_srcline(983);
        (void)(px_method(px_index(_v1721, px_str("upnames")), "push", (LXValue[]){_v1724}, 1));
        px_srcline(984);
        (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1721, _v1724}, 2));
        px_srcline(985);
         _v1723 = px_add(_v1723, px_int(1LL));
    }
    px_srcline(986);
    if (px_is_truthy(px_lt(px_index(_v1721, px_str("next_slot")), ({ LXValue _s209 = px_call(px_get_global("len"), (LXValue[]){_v1717}, 1); LXValue _s210 = px_call(px_get_global("len"), (LXValue[]){_v1719}, 1); px_add(_s209, _s210); })))) {
        px_srcline(987);
        px_index_set(_v1721, px_str("next_slot"), ({ LXValue _s211 = px_call(px_get_global("len"), (LXValue[]){_v1717}, 1); LXValue _s212 = px_call(px_get_global("len"), (LXValue[]){_v1719}, 1); px_add(_s211, _s212); }));
    }
    px_srcline(988);
    (void)(px_method(px_index(px_get_global("g_bcm"), px_str("funcs")), "push", (LXValue[]){_v1721}, 1));
    px_srcline(993);
    _v1725 = px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("funcs"))}, 1), px_int(1LL));
    px_srcline(997);
    _v1726 = px_list_n((LXValue[]){}, 0);
    px_srcline(998);
    (void)(px_call(px_get_global("bc_lambda_hoist"), (LXValue[]){_v1718, _v1726}, 2));
    px_srcline(1000);
    _v1727 = px_list_n((LXValue[]){}, 0);
    px_srcline(1001);
    (void)(px_call(px_get_global("bc_lambda_decl"), (LXValue[]){_v1718, _v1727}, 2));
    px_srcline(1002);
    _v1728 = px_int(0LL);
    px_srcline(1003);
    while (px_is_truthy(px_lt(_v1728, px_call(px_get_global("len"), (LXValue[]){_v1726}, 1)))) {
        px_srcline(1004);
        _v1729 = px_index(_v1726, _v1728);
        px_srcline(1013);
        if (px_is_truthy(({ LXValue _t1733 = px_not(px_method(px_index(_v1721, px_str("smap")), "has", (LXValue[]){_v1729}, 1)); px_is_truthy(_t1733) ? px_not(({ LXValue _t1732 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1727, _v1729}, 2)); px_is_truthy(_t1732) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v1729}, 2) : _t1732; })) : _t1733; }))) {
            px_srcline(1014);
            (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v1721, _v1729}, 2));
        }
        px_srcline(1015);
         _v1728 = px_add(_v1728, px_int(1LL));
    }
    px_srcline(1017);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v1721, _v1718}, 2));
    px_srcline(1018);
    _v1730 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1721}, 1);
    px_srcline(1019);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1718, _v1730, _v1721}, 3));
    px_srcline(1020);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1721, px_str("RET"), _v1730, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1021);
    px_index_set(_v1721, px_str("nslots"), px_index(_v1721, px_str("next_slot")));
    px_srcline(1022);
    return _v1725;
px_err_1731:
    if (px_err_1731_proped) return px_err_1731_val;
    return px_null();
}

static LXValue fn_bc_emit_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_closure");
    LXValue _v1734 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1735 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1736 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1737 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1738 = px_null();
    LXValue _v1739 = px_null();
    LXValue _v1740 = px_null();
    LXValue _v1741 = px_null();
    LXValue _v1742 = px_null();
    LXValue _v1743 = px_null();
    LXValue _v1744 = px_null();
    LXValue _v1745 = px_null();
    LXValue _v1746 = px_null();
    LXValue _v1747 = px_null();
    LXValue _v1748 = px_null();
    LXValue _v1749 = px_null();
    LXValue _v1750 = px_null();
    LXValue px_err_1751_val = px_null();
    int px_err_1751_proped = 0;
    px_srcline(1027);
    _v1738 = px_list_n((LXValue[]){}, 0);
    px_srcline(1028);
    _v1739 = px_int(0LL);
    px_srcline(1029);
    while (px_is_truthy(px_lt(_v1739, px_call(px_get_global("len"), (LXValue[]){_v1734}, 1)))) {
        px_srcline(1030);
        (void)(px_method(_v1738, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1734, _v1739), px_int(1LL))}, 1)}, 1));
        px_srcline(1031);
         _v1739 = px_add(_v1739, px_int(1LL));
    }
    px_srcline(1032);
    _v1740 = px_call(px_get_global("bc_closure_free"), (LXValue[]){_v1734, _v1735}, 2);
    px_srcline(1033);
    _v1741 = px_list_n((LXValue[]){}, 0);
    px_srcline(1034);
    _v1742 = px_int(0LL);
    px_srcline(1035);
    while (px_is_truthy(px_lt(_v1742, px_call(px_get_global("len"), (LXValue[]){_v1740}, 1)))) {
        px_srcline(1036);
        _v1743 = px_index(_v1740, _v1742);
        px_srcline(1037);
        if (px_is_truthy(px_method(px_index(_v1737, px_str("smap")), "has", (LXValue[]){_v1743}, 1))) {
            px_srcline(1038);
            (void)(px_method(_v1741, "append", (LXValue[]){_v1743}, 1));
        }
        px_srcline(1039);
         _v1742 = px_add(_v1742, px_int(1LL));
    }
    px_srcline(1040);
    _v1744 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){_v1738, _v1735, _v1741}, 3);
    px_srcline(1041);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1741}, 1), px_int(0LL)))) {
        px_srcline(1042);
        _v1745 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1744, px_float(0), px_str("")}, 5);
        px_srcline(1043);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1737, px_str("LOADK"), _v1736, _v1745, px_int(0LL)}, 5));
        px_srcline(1044);
        return _v1736;
    }
    px_srcline(1047);
    _v1746 = px_index(_v1737, px_str("next_slot"));
    px_srcline(1048);
    _v1747 = px_int(0LL);
    px_srcline(1049);
    while (px_is_truthy(px_lt(_v1747, px_call(px_get_global("len"), (LXValue[]){_v1741}, 1)))) {
        px_srcline(1050);
        _v1748 = px_index(px_index(_v1737, px_str("smap")), px_index(_v1741, _v1747));
        px_srcline(1051);
        _v1749 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1737}, 1);
        px_srcline(1052);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1737, px_str("MOV"), _v1749, _v1748, px_int(0LL)}, 5));
        px_srcline(1053);
         _v1747 = px_add(_v1747, px_int(1LL));
    }
    px_srcline(1054);
    _v1750 = _v1736;
    px_srcline(1055);
    if (px_is_truthy(px_lt(_v1750, px_int(0LL)))) {
        px_srcline(1056);
         _v1750 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1737}, 1);
    }
    px_srcline(1057);
    (void)(px_method(px_index(_v1737, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1750, _v1744, _v1746}, 5)}, 1));
    px_srcline(1058);
    return _v1750;
px_err_1751:
    if (px_err_1751_proped) return px_err_1751_val;
    return px_null();
}

static LXValue fn_bc_emit_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_comp");
    LXValue _v1752 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1753 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1754 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1755 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1756 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1757 = (nargs > 5) ? args[5] : px_null();
    LXValue _v1758 = (nargs > 6) ? args[6] : px_null();
    LXValue _v1759 = px_null();
    LXValue _v1760 = px_null();
    LXValue _v1761 = px_null();
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
    LXValue _v1780 = px_null();
    LXValue _v1781 = px_null();
    LXValue _v1782 = px_null();
    LXValue _v1783 = px_null();
    LXValue _v1784 = px_null();
    LXValue _v1785 = px_null();
    LXValue _v1786 = px_null();
    LXValue _v1787 = px_null();
    LXValue px_err_1788_val = px_null();
    int px_err_1788_proped = 0;
    px_srcline(1065);
    _v1759 = px_call(px_get_global("len"), (LXValue[]){_v1757}, 1);
    px_srcline(1066);
    if (px_is_truthy(px_ge(_v1758, _v1759))) {
        px_srcline(1067);
        _v1760 = px_neg(px_int(1LL));
        px_srcline(1068);
        if (px_is_truthy(px_ne(_v1755, px_null()))) {
            px_srcline(1069);
            _v1761 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
            px_srcline(1070);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1755, _v1761, _v1752}, 3));
            px_srcline(1071);
             _v1760 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1752, px_str("bc"))}, 1);
            px_srcline(1072);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("JMPF"), _v1761, px_int(0LL), px_int(0LL)}, 5));
        }
        px_srcline(1073);
        if (px_is_truthy(px_eq(_v1753, px_str("dict")))) {
            px_srcline(1074);
            _v1762 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
            px_srcline(1075);
            _v1763 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
            px_srcline(1076);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1754, px_int(0LL)), _v1762, _v1752}, 3));
            px_srcline(1077);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1754, px_int(1LL)), _v1763, _v1752}, 3));
            px_srcline(1082);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("DICTSET"), _v1756, _v1762, _v1763}, 5));
        }
        else {
            px_srcline(1084);
            _v1764 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
            px_srcline(1085);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1754, px_int(0LL)), _v1764, _v1752}, 3));
            px_srcline(1086);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("LISTPUSH"), _v1764, _v1756, px_int(0LL)}, 5));
        }
        px_srcline(1087);
        if (px_is_truthy(px_ge(_v1760, px_int(0LL)))) {
            px_srcline(1088);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1752, _v1760, px_call(px_get_global("len"), (LXValue[]){px_index(_v1752, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1089);
        return px_null();
    }
    px_srcline(1090);
    _v1765 = px_index(_v1757, _v1758);
    px_srcline(1091);
    _v1766 = px_index(_v1765, px_int(1LL));
    px_srcline(1097);
    _v1767 = px_list_n((LXValue[]){}, 0);
    px_srcline(1098);
    _v1768 = px_int(0LL);
    px_srcline(1099);
    while (px_is_truthy(px_lt(_v1768, px_call(px_get_global("len"), (LXValue[]){_v1766}, 1)))) {
        px_srcline(1100);
        _v1769 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1766, _v1768)}, 1);
        px_srcline(1101);
        _v1770 = px_neg(px_int(1LL));
        px_srcline(1102);
        if (px_is_truthy(px_method(px_index(_v1752, px_str("smap")), "has", (LXValue[]){_v1769}, 1))) {
            px_srcline(1103);
             _v1770 = px_index(px_index(_v1752, px_str("smap")), _v1769);
        }
        px_srcline(1104);
        _v1771 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
        px_srcline(1105);
        px_index_set(px_index(_v1752, px_str("smap")), _v1769, _v1771);
        px_srcline(1106);
        (void)(px_method(_v1767, "append", (LXValue[]){px_list_n((LXValue[]){_v1769, _v1770}, 2)}, 1));
        px_srcline(1107);
         _v1768 = px_add(_v1768, px_int(1LL));
    }
    px_srcline(1108);
    _v1772 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
    px_srcline(1109);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1765, px_int(2LL)), _v1772, _v1752}, 3));
    px_srcline(1110);
    _v1773 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
    px_srcline(1111);
    _v1774 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
    px_srcline(1112);
    while (px_is_truthy(px_lt(px_index(_v1752, px_str("next_slot")), px_add(_v1773, px_int(2LL))))) {
        px_srcline(1113);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1));
    }
    px_srcline(1114);
    _v1775 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1115);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("GETG"), _v1773, _v1775, px_int(0LL)}, 5));
    px_srcline(1116);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("MOV"), px_add(_v1773, px_int(1LL)), _v1772, px_int(0LL)}, 5));
    px_srcline(1117);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("CALL"), _v1774, _v1773, px_int(1LL)}, 5));
    px_srcline(1118);
    _v1776 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
    px_srcline(1119);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1752, _v1776, px_int(0LL)}, 3));
    px_srcline(1120);
    _v1777 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1752, px_str("bc"))}, 1);
    px_srcline(1121);
    _v1778 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
    px_srcline(1122);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("LT"), _v1778, _v1776, _v1774}, 5));
    px_srcline(1123);
    _v1779 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1752, px_str("bc"))}, 1);
    px_srcline(1124);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("JMPF"), _v1778, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1126);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("ITERLEN"), _v1772, _v1774, px_int(0LL)}, 5));
    px_srcline(1127);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1766}, 1), px_int(1LL)))) {
        px_srcline(1129);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("ITERAT"), px_index(px_index(_v1752, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1766, px_int(0LL))}, 1)), _v1772, _v1776}, 5));
    }
    else {
        px_srcline(1134);
        _v1780 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
        px_srcline(1135);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("ITERAT"), _v1780, _v1772, _v1776}, 5));
        px_srcline(1136);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("UNPACKCK"), _v1780, px_call(px_get_global("len"), (LXValue[]){_v1766}, 1), px_int(0LL)}, 5));
        px_srcline(1137);
        _v1781 = px_int(0LL);
        px_srcline(1138);
        while (px_is_truthy(px_lt(_v1781, px_call(px_get_global("len"), (LXValue[]){_v1766}, 1)))) {
            px_srcline(1139);
            _v1782 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
            px_srcline(1140);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1752, _v1782, _v1781}, 3));
            px_srcline(1141);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("INDEX"), px_index(px_index(_v1752, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1766, _v1781)}, 1)), _v1780, _v1782}, 5));
            px_srcline(1142);
             _v1781 = px_add(_v1781, px_int(1LL));
        }
    }
    px_srcline(1143);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1752, _v1753, _v1754, _v1755, _v1756, _v1757, px_add(_v1758, px_int(1LL))}, 7));
    px_srcline(1144);
    _v1783 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1752}, 1);
    px_srcline(1145);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1752, _v1783, px_int(1LL)}, 3));
    px_srcline(1146);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("ADD"), _v1776, _v1776, _v1783}, 5));
    px_srcline(1147);
    _v1784 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1752, px_str("bc"))}, 1);
    px_srcline(1148);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1752, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1149);
    _v1785 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1752, px_str("bc"))}, 1);
    px_srcline(1150);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1752, _v1784, _v1777}, 3));
    px_srcline(1151);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1752, _v1779, _v1785}, 3));
    px_srcline(1153);
    _v1786 = px_int(0LL);
    px_srcline(1154);
    while (px_is_truthy(px_lt(_v1786, px_call(px_get_global("len"), (LXValue[]){_v1767}, 1)))) {
        px_srcline(1155);
        _v1787 = px_index(_v1767, _v1786);
        px_srcline(1156);
        if (px_is_truthy(px_lt(px_index(_v1787, px_int(1LL)), px_int(0LL)))) {
            px_srcline(1157);
            (void)(px_method(px_index(_v1752, px_str("smap")), "remove", (LXValue[]){px_index(_v1787, px_int(0LL))}, 1));
        }
        else {
            px_srcline(1159);
            px_index_set(px_index(_v1752, px_str("smap")), px_index(_v1787, px_int(0LL)), px_index(_v1787, px_int(1LL)));
        }
        px_srcline(1160);
         _v1786 = px_add(_v1786, px_int(1LL));
    }
px_err_1788:
    if (px_err_1788_proped) return px_err_1788_val;
    return px_null();
}

static LXValue fn_bc_emit_caps_snapshot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_caps_snapshot");
    LXValue _v1789 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1790 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1791 = px_null();
    LXValue _v1792 = px_null();
    LXValue _v1793 = px_null();
    LXValue _v1794 = px_null();
    LXValue _v1795 = px_null();
    LXValue _v1796 = px_null();
    LXValue _v1797 = px_null();
    LXValue _v1798 = px_null();
    LXValue _v1799 = px_null();
    LXValue px_err_1800_val = px_null();
    int px_err_1800_proped = 0;
    px_srcline(1168);
    _v1791 = px_call(px_get_global("len"), (LXValue[]){_v1790}, 1);
    px_srcline(1169);
    _v1792 = px_index(_v1789, px_str("next_slot"));
    px_srcline(1170);
    _v1793 = px_int(0LL);
    px_srcline(1171);
    while (px_is_truthy(px_lt(_v1793, _v1791))) {
        px_srcline(1172);
        _v1794 = px_index(_v1790, _v1793);
        px_srcline(1173);
        _v1795 = px_index(px_index(_v1789, px_str("smap")), _v1794);
        px_srcline(1174);
        _v1796 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1789}, 1);
        px_srcline(1175);
        if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1789, _v1794}, 2))) {
            px_srcline(1176);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1789, px_str("CELLGET"), _v1796, _v1795, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1178);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1789, px_str("MOV"), _v1796, _v1795, px_int(0LL)}, 5));
        }
        px_srcline(1179);
         _v1793 = px_add(_v1793, px_int(1LL));
    }
    px_srcline(1180);
    _v1797 = px_index(_v1789, px_str("next_slot"));
    px_srcline(1181);
    _v1798 = px_int(0LL);
    px_srcline(1182);
    while (px_is_truthy(px_lt(_v1798, _v1791))) {
        px_srcline(1183);
        _v1799 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1789}, 1);
        px_srcline(1184);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1789, px_str("CELLNEW"), _v1799, px_add(_v1792, _v1798), px_int(0LL)}, 5));
        px_srcline(1185);
         _v1798 = px_add(_v1798, px_int(1LL));
    }
    px_srcline(1186);
    return _v1797;
px_err_1800:
    if (px_err_1800_proped) return px_err_1800_val;
    return px_null();
}

static LXValue fn_bc_genexp_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_genexp_caps");
    LXValue _v1801 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1802 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1803 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1804 = px_null();
    LXValue _v1805 = px_null();
    LXValue _v1806 = px_null();
    LXValue _v1807 = px_null();
    LXValue px_err_1808_val = px_null();
    int px_err_1808_proped = 0;
    px_srcline(1201);
    _v1804 = px_list_n((LXValue[]){}, 0);
    px_srcline(1202);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1802, _v1804}, 2));
    px_srcline(1203);
    _v1805 = px_list_n((LXValue[]){}, 0);
    px_srcline(1204);
    _v1806 = px_int(0LL);
    px_srcline(1205);
    while (px_is_truthy(px_lt(_v1806, px_call(px_get_global("len"), (LXValue[]){_v1804}, 1)))) {
        px_srcline(1206);
        _v1807 = px_index(_v1804, _v1806);
        px_srcline(1207);
        if (px_is_truthy(({ LXValue _t1809 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1801, _v1807}, 2)); px_is_truthy(_t1809) ? px_method(px_index(_v1803, px_str("smap")), "has", (LXValue[]){_v1807}, 1) : _t1809; }))) {
            px_srcline(1208);
            (void)(px_method(_v1805, "append", (LXValue[]){_v1807}, 1));
        }
        px_srcline(1209);
         _v1806 = px_add(_v1806, px_int(1LL));
    }
    px_srcline(1210);
    return _v1805;
px_err_1808:
    if (px_err_1808_proped) return px_err_1808_val;
    return px_null();
}

static LXValue fn_bc_emit_genexp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_genexp");
    LXValue _v1810 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1811 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1812 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1813 = px_null();
    LXValue _v1814 = px_null();
    LXValue _v1815 = px_null();
    LXValue _v1816 = px_null();
    LXValue _v1817 = px_null();
    LXValue _v1818 = px_null();
    LXValue _v1819 = px_null();
    LXValue _v1820 = px_null();
    LXValue _v1821 = px_null();
    LXValue _v1822 = px_null();
    LXValue _v1823 = px_null();
    LXValue _v1824 = px_null();
    LXValue _v1825 = px_null();
    LXValue px_err_1826_val = px_null();
    int px_err_1826_proped = 0;
    px_srcline(1214);
    _v1813 = px_index(_v1810, px_int(2LL));
    px_srcline(1215);
    if (px_is_truthy(({ LXValue _t1827 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1813}, 1), px_int(1LL)); px_is_truthy(_t1827) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1813, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1827; }))) {
        px_srcline(1218);
        _v1814 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1813, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
        px_srcline(1219);
        _v1815 = _v1811;
        px_srcline(1220);
        if (px_is_truthy(px_lt(_v1815, px_int(0LL)))) {
            px_srcline(1221);
             _v1815 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1);
        }
        px_srcline(1222);
        _v1816 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1);
        px_srcline(1223);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1813, px_int(0LL)), px_int(2LL)), _v1816, _v1812}, 3));
        px_srcline(1229);
        _v1817 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){px_list_n((LXValue[]){_v1814}, 1), px_index(_v1810, px_int(1LL)), _v1812}, 3);
        px_srcline(1230);
        _v1818 = px_list_n((LXValue[]){}, 0);
        px_srcline(1231);
        if (px_is_truthy(px_ne(px_index(_v1810, px_int(3LL)), px_null()))) {
            px_srcline(1232);
             _v1818 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){px_list_n((LXValue[]){_v1814}, 1), px_index(_v1810, px_int(3LL)), _v1812}, 3);
        }
        px_srcline(1233);
        _v1819 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1814}, 1), px_index(_v1810, px_int(1LL)), _v1817}, 3);
        px_srcline(1234);
        _v1820 = px_neg(px_int(1LL));
        px_srcline(1235);
        if (px_is_truthy(px_ne(px_index(_v1810, px_int(3LL)), px_null()))) {
            px_srcline(1236);
             _v1820 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1814}, 1), px_index(_v1810, px_int(3LL)), _v1818}, 3);
        }
        px_srcline(1237);
        _v1821 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1);
        px_srcline(1238);
        while (px_is_truthy(px_lt(px_index(_v1812, px_str("next_slot")), px_add(_v1821, px_int(2LL))))) {
            px_srcline(1239);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1));
        }
        px_srcline(1242);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1817}, 1), px_int(0LL)))) {
            px_srcline(1243);
            _v1822 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1819, px_float(0), px_str("")}, 5);
            px_srcline(1244);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("LOADK"), _v1821, _v1822, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1246);
            _v1823 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1812, _v1817}, 2);
            px_srcline(1247);
            (void)(px_method(px_index(_v1812, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1821, _v1819, _v1823}, 5)}, 1));
        }
        px_srcline(1248);
        if (px_is_truthy(px_ge(_v1820, px_int(0LL)))) {
            px_srcline(1249);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1818}, 1), px_int(0LL)))) {
                px_srcline(1250);
                _v1824 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1820, px_float(0), px_str("")}, 5);
                px_srcline(1251);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("LOADK"), px_add(_v1821, px_int(1LL)), _v1824, px_int(0LL)}, 5));
            }
            else {
                px_srcline(1253);
                _v1825 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1812, _v1818}, 2);
                px_srcline(1254);
                (void)(px_method(px_index(_v1812, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), px_add(_v1821, px_int(1LL)), _v1820, _v1825}, 5)}, 1));
            }
        }
        else {
            px_srcline(1256);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1812, px_add(_v1821, px_int(1LL))}, 2));
        }
        px_srcline(1257);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("NEWGEN"), _v1815, _v1816, _v1821}, 5));
        px_srcline(1258);
        return _v1815;
    }
    px_srcline(1262);
    _v1815 = _v1811;
    px_srcline(1263);
    if (px_is_truthy(px_lt(_v1815, px_int(0LL)))) {
        px_srcline(1264);
         _v1815 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1812}, 1);
    }
    px_srcline(1265);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("NEWLIST"), _v1815, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1266);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1812, px_str("push"), px_list_n((LXValue[]){px_index(_v1810, px_int(1LL))}, 1), px_index(_v1810, px_int(3LL)), _v1815, _v1813, px_int(0LL)}, 7));
    px_srcline(1267);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1812, px_str("GENFROMLIST"), _v1815, _v1815, px_int(0LL)}, 5));
    px_srcline(1268);
    return _v1815;
px_err_1826:
    if (px_err_1826_proped) return px_err_1826_val;
    return px_null();
}

static LXValue fn_bc_match_enumvar(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_enumvar");
    LXValue _v1828 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1829 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1830 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1831 = px_null();
    LXValue _v1832 = px_null();
    LXValue _v1833 = px_null();
    LXValue _v1834 = px_null();
    LXValue px_err_1835_val = px_null();
    int px_err_1835_proped = 0;
    px_srcline(1274);
    _v1831 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1830}, 1);
    px_srcline(1275);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1830, px_str("ENUMVAR"), _v1831, _v1829, px_int(0LL)}, 5));
    px_srcline(1276);
    _v1832 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1828}, 5);
    px_srcline(1277);
    _v1833 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1830}, 1);
    px_srcline(1278);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1830, px_str("LOADK"), _v1833, _v1832, px_int(0LL)}, 5));
    px_srcline(1279);
    _v1834 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1830}, 1);
    px_srcline(1280);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1830, px_str("EQ"), _v1834, _v1831, _v1833}, 5));
    px_srcline(1281);
    return _v1834;
px_err_1835:
    if (px_err_1835_proped) return px_err_1835_val;
    return px_null();
}

static LXValue fn_bc_match_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_cond");
    LXValue _v1836 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1837 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1838 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1839 = px_null();
    LXValue _v1840 = px_null();
    LXValue _v1841 = px_null();
    LXValue _v1842 = px_null();
    LXValue _v1843 = px_null();
    LXValue px_err_1844_val = px_null();
    int px_err_1844_proped = 0;
    px_srcline(1283);
    _v1839 = px_index(_v1836, px_int(0LL));
    px_srcline(1284);
    if (px_is_truthy(px_eq(_v1839, px_str("PatWildcard")))) {
        px_srcline(1285);
        return px_null();
    }
    px_srcline(1286);
    if (px_is_truthy(px_eq(_v1839, px_str("PatBinding")))) {
        px_srcline(1287);
        _v1840 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1836, px_int(1LL))}, 1);
        px_srcline(1288);
        if (px_is_truthy(({ LXValue _t1846 = ({ LXValue _t1845 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1840}, 1), px_int(0LL)); px_is_truthy(_t1845) ? px_ge(px_index(_v1840, px_int(0LL)), px_str("A")) : _t1845; }); px_is_truthy(_t1846) ? px_le(px_index(_v1840, px_int(0LL)), px_str("Z")) : _t1846; }))) {
            px_srcline(1290);
            return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){_v1840, _v1837, _v1838}, 3);
        }
        px_srcline(1291);
        return px_null();
    }
    px_srcline(1292);
    if (px_is_truthy(px_eq(_v1839, px_str("PatTuple")))) {
        px_srcline(1293);
        _v1841 = px_index(_v1836, px_int(1LL));
        px_srcline(1294);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1841}, 1), px_int(0LL)))) {
            px_srcline(1295);
            return px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1841, px_int(0LL)), _v1837, _v1838}, 3);
        }
        px_srcline(1296);
        return px_null();
    }
    px_srcline(1297);
    if (px_is_truthy(px_eq(_v1839, px_str("PatConstructor")))) {
        px_srcline(1298);
        return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1836, px_int(1LL))}, 1), _v1837, _v1838}, 3);
    }
    px_srcline(1299);
    if (px_is_truthy(px_eq(_v1839, px_str("PatLiteral")))) {
        px_srcline(1300);
        _v1842 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1838}, 1);
        px_srcline(1301);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1836, px_int(1LL)), _v1842, _v1838}, 3));
        px_srcline(1302);
        _v1843 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1838}, 1);
        px_srcline(1303);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1838, px_str("EQ"), _v1843, _v1837, _v1842}, 5));
        px_srcline(1304);
        return _v1843;
    }
    px_srcline(1305);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_match_cond 未知 pattern: "), px_call(px_get_global("str"), (LXValue[]){_v1836}, 1))}, 1));
px_err_1844:
    if (px_err_1844_proped) return px_err_1844_val;
    return px_null();
}

static LXValue fn_bc_assign_local_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_local_slot");
    LXValue _v1847 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1848 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1849 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1850 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1851 = px_null();
    LXValue _v1852 = px_null();
    LXValue px_err_1853_val = px_null();
    int px_err_1853_proped = 0;
    px_srcline(1309);
    if (px_is_truthy(px_eq(_v1848, px_str("Assign")))) {
        px_srcline(1316);
        _v1851 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1850}, 1);
        px_srcline(1317);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1849, _v1851, _v1850}, 3));
        px_srcline(1318);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1850, px_str("MOV"), _v1847, _v1851, px_int(0LL)}, 5));
        px_srcline(1319);
        return px_null();
    }
    px_srcline(1320);
    _v1852 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1850}, 1);
    px_srcline(1321);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1849, _v1852, _v1850}, 3));
    px_srcline(1322);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1850, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1848}, 1)}, 1), _v1847, _v1847, _v1852}, 5));
px_err_1853:
    if (px_err_1853_proped) return px_err_1853_val;
    return px_null();
}

static LXValue fn_bc_assign_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_global");
    LXValue _v1854 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1855 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1856 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1857 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1858 = px_null();
    LXValue _v1859 = px_null();
    LXValue _v1860 = px_null();
    LXValue px_err_1861_val = px_null();
    int px_err_1861_proped = 0;
    px_srcline(1324);
    _v1858 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1854}, 2);
    px_srcline(1325);
    if (px_is_truthy(px_eq(_v1855, px_str("Assign")))) {
        px_srcline(1326);
        _v1859 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1857}, 1);
        px_srcline(1327);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1856, _v1859, _v1857}, 3));
        px_srcline(1328);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1857, px_str("SETG"), _v1858, _v1859, px_int(0LL)}, 5));
        px_srcline(1329);
        return px_null();
    }
    px_srcline(1330);
    _v1859 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1857}, 1);
    px_srcline(1331);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1857, px_str("GETG"), _v1859, _v1858, px_int(0LL)}, 5));
    px_srcline(1332);
    _v1860 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1857}, 1);
    px_srcline(1333);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1856, _v1860, _v1857}, 3));
    px_srcline(1334);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1857, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1855}, 1)}, 1), _v1859, _v1859, _v1860}, 5));
    px_srcline(1335);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1857, px_str("SETG"), _v1858, _v1859, px_int(0LL)}, 5));
px_err_1861:
    if (px_err_1861_proped) return px_err_1861_val;
    return px_null();
}

static LXValue fn_bc_assign_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_index");
    LXValue _v1862 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1863 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1864 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1865 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1866 = px_null();
    LXValue _v1867 = px_null();
    LXValue _v1868 = px_null();
    LXValue _v1869 = px_null();
    LXValue px_err_1870_val = px_null();
    int px_err_1870_proped = 0;
    px_srcline(1338);
    _v1866 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1865}, 1);
    px_srcline(1339);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1862, px_int(1LL)), _v1866, _v1865}, 3));
    px_srcline(1340);
    _v1867 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1865}, 1);
    px_srcline(1341);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1862, px_int(2LL)), _v1867, _v1865}, 3));
    px_srcline(1342);
    _v1868 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1865}, 1);
    px_srcline(1343);
    if (px_is_truthy(px_eq(_v1863, px_str("Assign")))) {
        px_srcline(1344);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1864, _v1868, _v1865}, 3));
    }
    else {
        px_srcline(1346);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1865, px_str("INDEX"), _v1868, _v1866, _v1867}, 5));
        px_srcline(1347);
        _v1869 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1865}, 1);
        px_srcline(1348);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1864, _v1869, _v1865}, 3));
        px_srcline(1349);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1865, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1863}, 1)}, 1), _v1868, _v1868, _v1869}, 5));
    }
    px_srcline(1350);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1865, px_str("SETIDX"), _v1868, _v1866, _v1867}, 5));
px_err_1870:
    if (px_err_1870_proped) return px_err_1870_val;
    return px_null();
}

static LXValue fn_bc_assign_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_field");
    LXValue _v1871 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1872 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1873 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1874 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1875 = px_null();
    LXValue _v1876 = px_null();
    LXValue _v1877 = px_null();
    LXValue _v1878 = px_null();
    LXValue px_err_1879_val = px_null();
    int px_err_1879_proped = 0;
    px_srcline(1353);
    _v1875 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1874}, 1);
    px_srcline(1354);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1871, px_int(1LL)), _v1875, _v1874}, 3));
    px_srcline(1355);
    _v1876 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1871, px_int(2LL))}, 1)}, 2);
    px_srcline(1356);
    _v1877 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1874}, 1);
    px_srcline(1357);
    if (px_is_truthy(px_eq(_v1872, px_str("Assign")))) {
        px_srcline(1358);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1873, _v1877, _v1874}, 3));
    }
    else {
        px_srcline(1360);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1874, px_str("GETF"), _v1877, _v1875, _v1876}, 5));
        px_srcline(1361);
        _v1878 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1874}, 1);
        px_srcline(1362);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1873, _v1878, _v1874}, 3));
        px_srcline(1363);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1874, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1872}, 1)}, 1), _v1877, _v1877, _v1878}, 5));
    }
    px_srcline(1364);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1874, px_str("SETF"), _v1877, _v1875, _v1876}, 5));
px_err_1879:
    if (px_err_1879_proped) return px_err_1879_val;
    return px_null();
}

static LXValue fn_bc_assign_op_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_op_name");
    LXValue _v1880 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1881_val = px_null();
    int px_err_1881_proped = 0;
    px_srcline(1367);
    if (px_is_truthy(px_eq(_v1880, px_str("Plus")))) {
        px_srcline(1368);
        return px_str("Add");
    }
    px_srcline(1369);
    if (px_is_truthy(px_eq(_v1880, px_str("Minus")))) {
        px_srcline(1370);
        return px_str("Sub");
    }
    px_srcline(1371);
    if (px_is_truthy(px_eq(_v1880, px_str("Star")))) {
        px_srcline(1372);
        return px_str("Mul");
    }
    px_srcline(1373);
    if (px_is_truthy(px_eq(_v1880, px_str("Slash")))) {
        px_srcline(1374);
        return px_str("Div");
    }
    px_srcline(1375);
    if (px_is_truthy(px_eq(_v1880, px_str("IntDiv")))) {
        px_srcline(1376);
        return px_str("IntDiv");
    }
    px_srcline(1377);
    if (px_is_truthy(px_eq(_v1880, px_str("Mod")))) {
        px_srcline(1378);
        return px_str("Mod");
    }
    px_srcline(1379);
    if (px_is_truthy(px_eq(_v1880, px_str("Pow")))) {
        px_srcline(1380);
        return px_str("Pow");
    }
    px_srcline(1381);
    if (px_is_truthy(px_eq(_v1880, px_str("BitAnd")))) {
        px_srcline(1382);
        return px_str("BitAnd");
    }
    px_srcline(1383);
    if (px_is_truthy(px_eq(_v1880, px_str("BitOr")))) {
        px_srcline(1384);
        return px_str("BitOr");
    }
    px_srcline(1385);
    if (px_is_truthy(px_eq(_v1880, px_str("BitXor")))) {
        px_srcline(1386);
        return px_str("BitXor");
    }
    px_srcline(1387);
    if (px_is_truthy(px_eq(_v1880, px_str("Shl")))) {
        px_srcline(1388);
        return px_str("Shl");
    }
    px_srcline(1389);
    if (px_is_truthy(px_eq(_v1880, px_str("Shr")))) {
        px_srcline(1390);
        return px_str("Shr");
    }
    px_srcline(1391);
    if (px_is_truthy(px_eq(_v1880, px_str("ShrU")))) {
        px_srcline(1392);
        return px_str("ShrU");
    }
    px_srcline(1393);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_assign_op_name 未知赋值 op: "), px_call(px_get_global("str"), (LXValue[]){_v1880}, 1))}, 1));
px_err_1881:
    if (px_err_1881_proped) return px_err_1881_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt_inner");
    LXValue _v1882 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1883 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1884 = px_null();
    LXValue _v1885 = px_null();
    LXValue _v1886 = px_null();
    LXValue _v1887 = px_null();
    LXValue _v1888 = px_null();
    LXValue _v1889 = px_null();
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
    LXValue _v1908 = px_null();
    LXValue _v1909 = px_null();
    LXValue _v1910 = px_null();
    LXValue _v1911 = px_null();
    LXValue _v1912 = px_null();
    LXValue _v1913 = px_null();
    LXValue _v1914 = px_null();
    LXValue _v1915 = px_null();
    LXValue px_err_1916_val = px_null();
    int px_err_1916_proped = 0;
    px_srcline(1396);
    _v1884 = px_index(_v1882, px_int(0LL));
    px_srcline(1397);
    if (px_is_truthy(px_eq(_v1884, px_str("VarDecl")))) {
        px_srcline(1400);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v1882}, 1));
        px_srcline(1402);
        _v1885 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1882, px_int(2LL))}, 1);
        px_srcline(1403);
        if (px_is_truthy(px_eq(px_index(_v1882, px_int(4LL)), px_null()))) {
            px_srcline(1406);
            if (px_is_truthy(px_index(_v1883, px_str("is_top")))) {
                px_srcline(1407);
                _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
                px_srcline(1408);
                (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1883, _v1886}, 2));
                px_srcline(1409);
                _v1887 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1885}, 2);
                px_srcline(1410);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("SETG"), _v1887, _v1886, px_int(0LL)}, 5));
            }
            px_srcline(1411);
            return px_null();
        }
        px_srcline(1412);
        if (px_is_truthy(px_index(_v1883, px_str("is_top")))) {
            px_srcline(1414);
            _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
            px_srcline(1415);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1882, px_int(4LL)), _v1886, _v1883}, 3));
            px_srcline(1416);
            _v1887 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1885}, 2);
            px_srcline(1417);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("SETG"), _v1887, _v1886, px_int(0LL)}, 5));
            px_srcline(1418);
            return px_null();
        }
        px_srcline(1422);
        _v1888 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1883, _v1885}, 2);
        px_srcline(1423);
        _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
        px_srcline(1424);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1882, px_int(4LL)), _v1886, _v1883}, 3));
        px_srcline(1425);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1883, _v1885, _v1888, _v1886}, 4));
        px_srcline(1426);
        return px_null();
    }
    px_srcline(1427);
    if (px_is_truthy(px_eq(_v1884, px_str("Assign")))) {
        px_srcline(1430);
        _v1889 = px_index(_v1882, px_int(1LL));
        px_srcline(1431);
        _v1890 = px_index(_v1882, px_int(2LL));
        px_srcline(1432);
        _v1891 = px_index(_v1882, px_int(3LL));
        px_srcline(1433);
        if (px_is_truthy(px_eq(_v1890, px_str("Append")))) {
            px_srcline(1434);
            _v1892 = px_list_n((LXValue[]){_v1891}, 1);
            px_srcline(1435);
            (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){_v1889, px_str("append"), _v1892, px_neg(px_int(1LL)), _v1883}, 5));
            px_srcline(1436);
            return px_null();
        }
        px_srcline(1439);
        (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v1889, _v1890, _v1891}, 3));
        px_srcline(1440);
        _v1893 = px_index(_v1889, px_int(0LL));
        px_srcline(1441);
        if (px_is_truthy(px_eq(_v1893, px_str("Var")))) {
            px_srcline(1442);
            _v1885 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1889, px_int(1LL))}, 1);
            px_srcline(1443);
            if (px_is_truthy(px_method(px_index(_v1883, px_str("smap")), "has", (LXValue[]){_v1885}, 1))) {
                px_srcline(1445);
                if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1883, _v1885}, 2))) {
                    px_srcline(1446);
                    _v1894 = px_index(px_index(_v1883, px_str("smap")), _v1885);
                    px_srcline(1447);
                    if (px_is_truthy(px_eq(_v1890, px_str("Assign")))) {
                        px_srcline(1448);
                        _v1895 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
                        px_srcline(1449);
                        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1891, _v1895, _v1883}, 3));
                        px_srcline(1450);
                        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("CELLSET"), _v1895, _v1894, px_int(0LL)}, 5));
                        px_srcline(1451);
                        return px_null();
                    }
                    px_srcline(1452);
                    _v1896 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
                    px_srcline(1453);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("CELLGET"), _v1896, _v1894, px_int(0LL)}, 5));
                    px_srcline(1454);
                    _v1897 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
                    px_srcline(1455);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1891, _v1897, _v1883}, 3));
                    px_srcline(1456);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1890}, 1)}, 1), _v1896, _v1896, _v1897}, 5));
                    px_srcline(1457);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("CELLSET"), _v1896, _v1894, px_int(0LL)}, 5));
                    px_srcline(1458);
                    return px_null();
                }
                px_srcline(1459);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){px_index(px_index(_v1883, px_str("smap")), _v1885), _v1890, _v1891, _v1883}, 4));
                px_srcline(1460);
                return px_null();
            }
            px_srcline(1461);
            if (px_is_truthy(({ LXValue _t1917 = px_not(px_index(_v1883, px_str("is_top"))); px_is_truthy(_t1917) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v1885}, 2)) : _t1917; }))) {
                px_srcline(1464);
                _v1888 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1883, _v1885}, 2);
                px_srcline(1465);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){_v1888, _v1890, _v1891, _v1883}, 4));
                px_srcline(1466);
                return px_null();
            }
            px_srcline(1468);
            (void)(px_call(px_get_global("bc_assign_global"), (LXValue[]){_v1885, _v1890, _v1891, _v1883}, 4));
            px_srcline(1469);
            return px_null();
        }
        px_srcline(1470);
        if (px_is_truthy(px_eq(_v1893, px_str("Index")))) {
            px_srcline(1471);
            (void)(px_call(px_get_global("bc_assign_index"), (LXValue[]){_v1889, _v1890, _v1891, _v1883}, 4));
            px_srcline(1472);
            return px_null();
        }
        px_srcline(1473);
        if (px_is_truthy(px_eq(_v1893, px_str("Field")))) {
            px_srcline(1474);
            (void)(px_call(px_get_global("bc_assign_field"), (LXValue[]){_v1889, _v1890, _v1891, _v1883}, 4));
            px_srcline(1475);
            return px_null();
        }
        px_srcline(1476);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt Assign 目标未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1889}, 1))}, 1));
    }
    px_srcline(1477);
    if (px_is_truthy(px_eq(_v1884, px_str("Return")))) {
        px_srcline(1478);
        if (px_is_truthy(px_ne(px_index(_v1882, px_int(1LL)), px_null()))) {
            px_srcline(1479);
            _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
            px_srcline(1480);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1882, px_int(1LL)), _v1886, _v1883}, 3));
            px_srcline(1481);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("RET"), _v1886, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1482);
            return px_null();
        }
        px_srcline(1483);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1484);
        return px_null();
    }
    px_srcline(1485);
    if (px_is_truthy(px_eq(_v1884, px_str("ExprStmt")))) {
        px_srcline(1487);
        _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
        px_srcline(1488);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1882, px_int(1LL)), _v1886, _v1883}, 3));
        px_srcline(1489);
        return px_null();
    }
    px_srcline(1490);
    if (px_is_truthy(px_eq(_v1884, px_str("If")))) {
        px_srcline(1491);
        (void)(px_call(px_get_global("bc_emit_if"), (LXValue[]){_v1882, _v1883}, 2));
        px_srcline(1492);
        return px_null();
    }
    px_srcline(1493);
    if (px_is_truthy(px_eq(_v1884, px_str("While")))) {
        px_srcline(1494);
        (void)(px_call(px_get_global("bc_emit_while"), (LXValue[]){_v1882, _v1883}, 2));
        px_srcline(1495);
        return px_null();
    }
    px_srcline(1496);
    if (px_is_truthy(px_eq(_v1884, px_str("For")))) {
        px_srcline(1497);
        (void)(px_call(px_get_global("bc_emit_for"), (LXValue[]){_v1882, _v1883}, 2));
        px_srcline(1498);
        return px_null();
    }
    px_srcline(1499);
    if (px_is_truthy(px_eq(_v1884, px_str("Break")))) {
        px_srcline(1501);
        _v1898 = px_index(_v1883, px_str("loops"));
        px_srcline(1502);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1898}, 1), px_int(0LL)))) {
            px_srcline(1503);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Break 不在循环内")}, 1));
        }
        px_srcline(1504);
        _v1899 = px_index(_v1898, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1898}, 1), px_int(1LL)));
        px_srcline(1505);
        _v1900 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1883, px_str("bc"))}, 1);
        px_srcline(1506);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1507);
        (void)(px_method(px_index(_v1899, px_str("breaks")), "append", (LXValue[]){_v1900}, 1));
        px_srcline(1508);
        return px_null();
    }
    px_srcline(1509);
    if (px_is_truthy(px_eq(_v1884, px_str("Continue")))) {
        px_srcline(1511);
        _v1898 = px_index(_v1883, px_str("loops"));
        px_srcline(1512);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1898}, 1), px_int(0LL)))) {
            px_srcline(1513);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Continue 不在循环内")}, 1));
        }
        px_srcline(1514);
        _v1899 = px_index(_v1898, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1898}, 1), px_int(1LL)));
        px_srcline(1515);
        _v1900 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1883, px_str("bc"))}, 1);
        px_srcline(1516);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1517);
        (void)(px_method(px_index(_v1899, px_str("conts")), "append", (LXValue[]){_v1900}, 1));
        px_srcline(1518);
        return px_null();
    }
    px_srcline(1519);
    if (px_is_truthy(px_eq(_v1884, px_str("Empty")))) {
        px_srcline(1520);
        return px_null();
    }
    px_srcline(1521);
    if (px_is_truthy(px_eq(_v1884, px_str("TypeConst")))) {
        px_srcline(1523);
        return px_null();
    }
    px_srcline(1524);
    if (px_is_truthy(px_eq(_v1884, px_str("ChanDecl")))) {
        px_srcline(1526);
        _v1885 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1882, px_int(1LL))}, 1);
        px_srcline(1527);
        _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
        px_srcline(1528);
        _v1894 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
        px_srcline(1529);
        _v1901 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan")}, 2);
        px_srcline(1530);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("GETG"), _v1894, _v1901, px_int(0LL)}, 5));
        px_srcline(1531);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("CALL"), _v1886, _v1894, px_int(0LL)}, 5));
        px_srcline(1532);
        if (px_is_truthy(px_index(_v1883, px_str("is_top")))) {
            px_srcline(1533);
            _v1887 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1885}, 2);
            px_srcline(1534);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("SETG"), _v1887, _v1886, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1536);
            _v1888 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1883, _v1885}, 2);
            px_srcline(1537);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("MOV"), _v1888, _v1886, px_int(0LL)}, 5));
        }
        px_srcline(1538);
        return px_null();
    }
    px_srcline(1539);
    if (px_is_truthy(px_eq(_v1884, px_str("Send")))) {
        px_srcline(1541);
        _v1902 = px_list_n((LXValue[]){px_index(_v1882, px_int(2LL))}, 1);
        px_srcline(1542);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1882, px_int(1LL)), px_str("send"), _v1902, px_neg(px_int(1LL)), _v1883}, 5));
        px_srcline(1543);
        return px_null();
    }
    px_srcline(1544);
    if (px_is_truthy(px_eq(_v1884, px_str("Recv")))) {
        px_srcline(1546);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1882, px_int(1LL)), px_str("recv"), px_list_n((LXValue[]){}, 0), px_neg(px_int(1LL)), _v1883}, 5));
        px_srcline(1547);
        return px_null();
    }
    px_srcline(1548);
    if (px_is_truthy(px_eq(_v1884, px_str("Spawn")))) {
        px_srcline(1551);
        _v1903 = px_index(_v1882, px_int(1LL));
        px_srcline(1552);
        if (px_is_truthy(({ LXValue _t1918 = px_eq(px_index(_v1903, px_int(0LL)), px_str("Call")); px_is_truthy(_t1918) ? px_eq(px_index(px_index(_v1903, px_int(1LL)), px_int(0LL)), px_str("Var")) : _t1918; }))) {
            px_srcline(1553);
            _v1904 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1903, px_int(1LL)), px_int(1LL))}, 1);
            px_srcline(1554);
            _v1905 = px_index(_v1903, px_int(2LL));
            px_srcline(1555);
            _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
            px_srcline(1556);
            _v1894 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
            px_srcline(1557);
            _v1906 = px_add(px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1905}, 1));
            px_srcline(1558);
            _v1907 = px_add(px_add(_v1894, px_int(1LL)), _v1906);
            px_srcline(1559);
            while (px_is_truthy(px_lt(px_index(_v1883, px_str("next_slot")), _v1907))) {
                px_srcline(1560);
                (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1));
            }
            px_srcline(1561);
            _v1901 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("spawn")}, 2);
            px_srcline(1562);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("GETG"), _v1894, _v1901, px_int(0LL)}, 5));
            px_srcline(1563);
            _v1908 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1904}, 5);
            px_srcline(1564);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("LOADK"), px_add(_v1894, px_int(1LL)), _v1908, px_int(0LL)}, 5));
            px_srcline(1565);
            _v1909 = px_int(0LL);
            px_srcline(1566);
            while (px_is_truthy(px_lt(_v1909, px_call(px_get_global("len"), (LXValue[]){_v1905}, 1)))) {
                px_srcline(1567);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1905, _v1909), px_add(px_add(_v1894, px_int(2LL)), _v1909), _v1883}, 3));
                px_srcline(1568);
                 _v1909 = px_add(_v1909, px_int(1LL));
            }
            px_srcline(1569);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("CALL"), _v1886, _v1894, _v1906}, 5));
            px_srcline(1570);
            return px_null();
        }
        px_srcline(1575);
        _v1910 = px_index(_v1882, px_int(2LL));
        px_srcline(1576);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s213 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1910, px_int(0LL))}, 1)), px_str(":")); LXValue _s214 = px_call(px_get_global("str"), (LXValue[]){px_index(_v1910, px_int(1LL))}, 1); px_add(_s213, _s214); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()），多行匿名函数体见 M118"))}, 1));
        px_srcline(1577);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(1578);
    if (px_is_truthy(px_eq(_v1884, px_str("Select")))) {
        px_srcline(1579);
        (void)(px_call(px_get_global("bc_emit_select"), (LXValue[]){_v1882, _v1883}, 2));
        px_srcline(1580);
        return px_null();
    }
    px_srcline(1581);
    if (px_is_truthy(px_eq(_v1884, px_str("FuncDef")))) {
        px_srcline(1586);
        _v1911 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1882, px_int(1LL))}, 1);
        px_srcline(1587);
        if (px_is_truthy(px_index(_v1883, px_str("is_top")))) {
            px_srcline(1592);
            _v1912 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
            px_srcline(1593);
            (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1882, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1882, px_int(4LL))}, 2), _v1912, _v1883}, 4));
            px_srcline(1594);
            _v1913 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1911}, 2);
            px_srcline(1595);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1883, px_str("SETG"), _v1913, _v1912, px_int(0LL)}, 5));
            px_srcline(1596);
            return px_null();
        }
        px_srcline(1597);
        _v1914 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1883, _v1911}, 2);
        px_srcline(1598);
        _v1915 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1883}, 1);
        px_srcline(1599);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1882, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1882, px_int(4LL))}, 2), _v1915, _v1883}, 4));
        px_srcline(1600);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1883, _v1911, _v1914, _v1915}, 4));
        px_srcline(1601);
        return px_null();
    }
    px_srcline(1602);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1882}, 1))}, 1));
px_err_1916:
    if (px_err_1916_proped) return px_err_1916_val;
    return px_null();
}

static LXValue fn_bc_emit_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_select");
    LXValue _v1919 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1920 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1921 = px_null();
    LXValue _v1922 = px_null();
    LXValue _v1923 = px_null();
    LXValue _v1924 = px_null();
    LXValue _v1925 = px_null();
    LXValue _v1926 = px_null();
    LXValue _v1927 = px_null();
    LXValue _v1928 = px_null();
    LXValue _v1929 = px_null();
    LXValue _v1930 = px_null();
    LXValue _v1931 = px_null();
    LXValue _v1932 = px_null();
    LXValue _v1933 = px_null();
    LXValue _v1934 = px_null();
    LXValue _v1935 = px_null();
    LXValue _v1936 = px_null();
    LXValue _v1937 = px_null();
    LXValue _v1938 = px_null();
    LXValue _v1939 = px_null();
    LXValue _v1940 = px_null();
    LXValue _v1941 = px_null();
    LXValue _v1942 = px_null();
    LXValue _v1943 = px_null();
    LXValue _v1944 = px_null();
    LXValue _v1945 = px_null();
    LXValue px_err_1946_val = px_null();
    int px_err_1946_proped = 0;
    px_srcline(1610);
    _v1921 = px_index(_v1919, px_int(1LL));
    px_srcline(1611);
    _v1922 = px_index(_v1919, px_int(2LL));
    px_srcline(1612);
    _v1923 = px_call(px_get_global("len"), (LXValue[]){_v1921}, 1);
    px_srcline(1614);
    _v1924 = px_list_n((LXValue[]){}, 0);
    px_srcline(1615);
    _v1925 = px_int(0LL);
    px_srcline(1616);
    while (px_is_truthy(px_lt(_v1925, _v1923))) {
        px_srcline(1617);
        _v1926 = px_index(px_index(_v1921, _v1925), px_int(1LL));
        px_srcline(1619);
        if (px_is_truthy(({ LXValue _t1947 = px_ne(px_index(_v1926, px_int(0LL)), px_str("Call")); px_is_truthy(_t1947) ? _t1947 : px_ne(px_index(px_index(_v1926, px_int(1LL)), px_int(0LL)), px_str("Field")); }))) {
            px_srcline(1620);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit select case 仅支持 ch.recv(): "), px_call(px_get_global("str"), (LXValue[]){_v1926}, 1))}, 1));
        }
        px_srcline(1621);
        _v1927 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1920}, 1);
        px_srcline(1622);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1926, px_int(1LL)), px_int(1LL)), _v1927, _v1920}, 3));
        px_srcline(1623);
        (void)(px_method(_v1924, "append", (LXValue[]){_v1927}, 1));
        px_srcline(1624);
         _v1925 = px_add(_v1925, px_int(1LL));
    }
    px_srcline(1625);
    _v1928 = px_neg(px_int(1LL));
    px_srcline(1626);
    if (px_is_truthy(px_eq(_v1922, px_null()))) {
        px_srcline(1627);
         _v1928 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1);
    }
    px_srcline(1628);
    _v1929 = px_neg(px_int(1LL));
    px_srcline(1629);
    _v1930 = px_list_n((LXValue[]){}, 0);
    px_srcline(1630);
    _v1931 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan_try_recv")}, 2);
    px_srcline(1631);
    _v1932 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(1632);
     _v1925 = px_int(0LL);
    px_srcline(1633);
    while (px_is_truthy(px_lt(_v1925, _v1923))) {
        px_srcline(1634);
        if (px_is_truthy(px_ge(_v1929, px_int(0LL)))) {
            px_srcline(1635);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1920, _v1929, px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1637);
        _v1933 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1920}, 1);
        px_srcline(1638);
        _v1934 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1920}, 1);
        px_srcline(1639);
        _v1935 = px_add(_v1933, px_int(2LL));
        px_srcline(1640);
        while (px_is_truthy(px_lt(px_index(_v1920, px_str("next_slot")), _v1935))) {
            px_srcline(1641);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1920}, 1));
        }
        px_srcline(1642);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("GETG"), _v1933, _v1931, px_int(0LL)}, 5));
        px_srcline(1643);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("MOV"), px_add(_v1933, px_int(1LL)), px_index(_v1924, _v1925), px_int(0LL)}, 5));
        px_srcline(1644);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("CALL"), _v1934, _v1933, px_int(1LL)}, 5));
        px_srcline(1646);
        _v1936 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1920}, 1);
        px_srcline(1647);
        _v1937 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1920}, 1);
        px_srcline(1648);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("LOADK"), _v1937, _v1932, px_int(0LL)}, 5));
        px_srcline(1649);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("EQ"), _v1936, _v1934, _v1937}, 5));
        px_srcline(1650);
        _v1938 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1);
        px_srcline(1651);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("JMPT"), _v1936, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1652);
         _v1929 = _v1938;
        px_srcline(1654);
        _v1939 = px_index(px_index(_v1921, _v1925), px_int(0LL));
        px_srcline(1655);
        if (px_is_truthy(px_ne(_v1939, px_null()))) {
            px_srcline(1656);
            _v1940 = px_call(px_get_global("rust_unescape"), (LXValue[]){_v1939}, 1);
            px_srcline(1657);
            _v1941 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1920, _v1940}, 2);
            px_srcline(1658);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("MOV"), _v1941, _v1934, px_int(0LL)}, 5));
        }
        px_srcline(1659);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(px_index(_v1921, _v1925), px_int(2LL)), _v1920}, 2));
        px_srcline(1660);
        _v1942 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1);
        px_srcline(1661);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1662);
        (void)(px_method(_v1930, "append", (LXValue[]){_v1942}, 1));
        px_srcline(1663);
         _v1925 = px_add(_v1925, px_int(1LL));
    }
    px_srcline(1665);
    if (px_is_truthy(px_ge(_v1929, px_int(0LL)))) {
        px_srcline(1666);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1920, _v1929, px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1)}, 3));
    }
    px_srcline(1667);
    if (px_is_truthy(px_ne(_v1922, px_null()))) {
        px_srcline(1668);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1922, _v1920}, 2));
    }
    else {
        px_srcline(1670);
        _v1943 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1);
        px_srcline(1671);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1920, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1672);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1920, _v1943, _v1928}, 3));
    }
    px_srcline(1673);
    _v1944 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1920, px_str("bc"))}, 1);
    px_srcline(1674);
    _v1945 = px_int(0LL);
    px_srcline(1675);
    while (px_is_truthy(px_lt(_v1945, px_call(px_get_global("len"), (LXValue[]){_v1930}, 1)))) {
        px_srcline(1676);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1920, px_index(_v1930, _v1945), _v1944}, 3));
        px_srcline(1677);
         _v1945 = px_add(_v1945, px_int(1LL));
    }
px_err_1946:
    if (px_err_1946_proped) return px_err_1946_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt");
    LXValue _v1948 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1949 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1950 = px_null();
    LXValue _v1951 = px_null();
    LXValue px_err_1952_val = px_null();
    int px_err_1952_proped = 0;
    px_srcline(1680);
    _v1950 = px_int(0LL);
    px_srcline(1681);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1948}, 1), px_int(0LL)))) {
        px_srcline(1682);
        _v1951 = px_index(_v1948, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1948}, 1), px_int(1LL)));
        px_srcline(1683);
        if (px_is_truthy(({ LXValue _t1954 = ({ LXValue _t1953 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1951}, 1), px_str("list")); px_is_truthy(_t1953) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1951}, 1), px_int(1LL)) : _t1953; }); px_is_truthy(_t1954) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1951, px_int(0LL))}, 1), px_str("int")) : _t1954; }))) {
            px_srcline(1684);
             _v1950 = px_index(_v1951, px_int(0LL));
        }
    }
    px_srcline(1685);
    if (px_is_truthy(px_gt(_v1950, px_int(0LL)))) {
        px_srcline(1686);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1949, px_str("SRCLINE"), px_int(0LL), _v1950, px_int(0LL)}, 5));
    }
    px_srcline(1687);
    (void)(px_call(px_get_global("bc_emit_stmt_inner"), (LXValue[]){_v1948, _v1949}, 2));
px_err_1952:
    if (px_err_1952_proped) return px_err_1952_val;
    return px_null();
}

static LXValue fn_bc_emit_stmts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmts");
    LXValue _v1955 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1956 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1957 = px_null();
    LXValue px_err_1958_val = px_null();
    int px_err_1958_proped = 0;
    px_srcline(1689);
    _v1957 = px_int(0LL);
    px_srcline(1690);
    while (px_is_truthy(px_lt(_v1957, px_call(px_get_global("len"), (LXValue[]){_v1955}, 1)))) {
        px_srcline(1691);
        (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){px_index(_v1955, _v1957), _v1956}, 2));
        px_srcline(1692);
         _v1957 = px_add(_v1957, px_int(1LL));
    }
px_err_1958:
    if (px_err_1958_proped) return px_err_1958_val;
    return px_null();
}

static LXValue fn_bc_emit_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_if");
    LXValue _v1959 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1960 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1961 = px_null();
    LXValue _v1962 = px_null();
    LXValue _v1963 = px_null();
    LXValue _v1964 = px_null();
    LXValue _v1965 = px_null();
    LXValue _v1966 = px_null();
    LXValue _v1967 = px_null();
    LXValue _v1968 = px_null();
    LXValue _v1969 = px_null();
    LXValue _v1970 = px_null();
    LXValue _v1971 = px_null();
    LXValue _v1972 = px_null();
    LXValue px_err_1973_val = px_null();
    int px_err_1973_proped = 0;
    px_srcline(1698);
    _v1961 = px_index(_v1959, px_int(1LL));
    px_srcline(1699);
    _v1962 = px_index(_v1959, px_int(2LL));
    px_srcline(1700);
    _v1963 = px_call(px_get_global("len"), (LXValue[]){_v1961}, 1);
    px_srcline(1701);
    _v1964 = px_list_n((LXValue[]){}, 0);
    px_srcline(1702);
    _v1965 = px_int(0LL);
    px_srcline(1703);
    while (px_is_truthy(px_lt(_v1965, _v1963))) {
        px_srcline(1704);
        _v1966 = px_index(px_index(_v1961, _v1965), px_int(0LL));
        px_srcline(1705);
        _v1967 = px_index(px_index(_v1961, _v1965), px_int(1LL));
        px_srcline(1706);
        _v1968 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1960}, 1);
        px_srcline(1707);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1966, _v1968, _v1960}, 3));
        px_srcline(1708);
        _v1969 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1960, px_str("bc"))}, 1);
        px_srcline(1709);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1960, px_str("JMPF"), _v1968, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1710);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1967, _v1960}, 2));
        px_srcline(1711);
        if (px_is_truthy(({ LXValue _t1974 = px_lt(_v1965, px_sub(_v1963, px_int(1LL))); px_is_truthy(_t1974) ? _t1974 : px_ne(_v1962, px_null()); }))) {
            px_srcline(1714);
            _v1970 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1960, px_str("bc"))}, 1);
            px_srcline(1715);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1960, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1716);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1960, _v1969, px_call(px_get_global("len"), (LXValue[]){px_index(_v1960, px_str("bc"))}, 1)}, 3));
            px_srcline(1717);
            (void)(px_method(_v1964, "append", (LXValue[]){_v1970}, 1));
        }
        else {
            px_srcline(1720);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1960, _v1969, px_call(px_get_global("len"), (LXValue[]){px_index(_v1960, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1721);
         _v1965 = px_add(_v1965, px_int(1LL));
    }
    px_srcline(1722);
    if (px_is_truthy(px_ne(_v1962, px_null()))) {
        px_srcline(1723);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1962, _v1960}, 2));
    }
    px_srcline(1724);
    _v1971 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1960, px_str("bc"))}, 1);
    px_srcline(1725);
    _v1972 = px_int(0LL);
    px_srcline(1726);
    while (px_is_truthy(px_lt(_v1972, px_call(px_get_global("len"), (LXValue[]){_v1964}, 1)))) {
        px_srcline(1727);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1960, px_index(_v1964, _v1972), _v1971}, 3));
        px_srcline(1728);
         _v1972 = px_add(_v1972, px_int(1LL));
    }
px_err_1973:
    if (px_err_1973_proped) return px_err_1973_val;
    return px_null();
}

static LXValue fn_bc_emit_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_while");
    LXValue _v1975 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1976 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1977 = px_null();
    LXValue _v1978 = px_null();
    LXValue _v1979 = px_null();
    LXValue _v1980 = px_null();
    LXValue _v1981 = px_null();
    LXValue _v1982 = px_null();
    LXValue _v1983 = px_null();
    LXValue _v1984 = px_null();
    LXValue _v1985 = px_null();
    LXValue px_err_1986_val = px_null();
    int px_err_1986_proped = 0;
    px_srcline(1733);
    _v1977 = px_index(_v1975, px_int(1LL));
    px_srcline(1734);
    _v1978 = px_index(_v1975, px_int(2LL));
    px_srcline(1735);
    _v1979 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1976, px_str("bc"))}, 1);
    px_srcline(1736);
    _v1980 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1976}, 1);
    px_srcline(1737);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1977, _v1980, _v1976}, 3));
    px_srcline(1738);
    _v1981 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1976, px_str("bc"))}, 1);
    px_srcline(1739);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1976, px_str("JMPF"), _v1980, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1740);
    _v1982 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1741);
    px_index_set(_v1982, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1742);
    px_index_set(_v1982, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1743);
    (void)(px_method(px_index(_v1976, px_str("loops")), "push", (LXValue[]){_v1982}, 1));
    px_srcline(1744);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1978, _v1976}, 2));
    px_srcline(1745);
    (void)(px_method(px_index(_v1976, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1746);
    _v1983 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1976, px_str("bc"))}, 1);
    px_srcline(1747);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1976, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1748);
    _v1984 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1976, px_str("bc"))}, 1);
    px_srcline(1749);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1976, _v1983, _v1979}, 3));
    px_srcline(1750);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1976, _v1981, _v1984}, 3));
    px_srcline(1751);
    _v1985 = px_int(0LL);
    px_srcline(1752);
    while (px_is_truthy(px_lt(_v1985, px_call(px_get_global("len"), (LXValue[]){px_index(_v1982, px_str("breaks"))}, 1)))) {
        px_srcline(1753);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1976, px_index(px_index(_v1982, px_str("breaks")), _v1985), _v1984}, 3));
        px_srcline(1754);
         _v1985 = px_add(_v1985, px_int(1LL));
    }
    px_srcline(1755);
     _v1985 = px_int(0LL);
    px_srcline(1756);
    while (px_is_truthy(px_lt(_v1985, px_call(px_get_global("len"), (LXValue[]){px_index(_v1982, px_str("conts"))}, 1)))) {
        px_srcline(1757);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1976, px_index(px_index(_v1982, px_str("conts")), _v1985), _v1979}, 3));
        px_srcline(1758);
         _v1985 = px_add(_v1985, px_int(1LL));
    }
px_err_1986:
    if (px_err_1986_proped) return px_err_1986_val;
    return px_null();
}

static LXValue fn_bc_for_names(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_for_names");
    LXValue _v1987 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1988 = px_null();
    LXValue _v1989 = px_null();
    LXValue px_err_1990_val = px_null();
    int px_err_1990_proped = 0;
    px_srcline(1767);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1987}, 1), px_str("string")))) {
        px_srcline(1768);
        return px_list_n((LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v1987}, 1)}, 1);
    }
    px_srcline(1769);
    _v1988 = px_list_n((LXValue[]){}, 0);
    px_srcline(1770);
    _v1989 = px_int(0LL);
    px_srcline(1771);
    while (px_is_truthy(px_lt(_v1989, px_call(px_get_global("len"), (LXValue[]){_v1987}, 1)))) {
        px_srcline(1772);
        (void)(px_method(_v1988, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1987, _v1989)}, 1)}, 1));
        px_srcline(1773);
         _v1989 = px_add(_v1989, px_int(1LL));
    }
    px_srcline(1774);
    return _v1988;
px_err_1990:
    if (px_err_1990_proped) return px_err_1990_val;
    return px_null();
}

static LXValue fn_bc_emit_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_for");
    LXValue _v1991 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1992 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1993 = px_null();
    LXValue _v1994 = px_null();
    LXValue _v1995 = px_null();
    LXValue _v1996 = px_null();
    LXValue _v1997 = px_null();
    LXValue _v1998 = px_null();
    LXValue _v1999 = px_null();
    LXValue _v2000 = px_null();
    LXValue _v2001 = px_null();
    LXValue _v2002 = px_null();
    LXValue _v2003 = px_null();
    LXValue _v2004 = px_null();
    LXValue _v2005 = px_null();
    LXValue _v2006 = px_null();
    LXValue _v2007 = px_null();
    LXValue _v2008 = px_null();
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
    LXValue _v2020 = px_null();
    LXValue px_err_2021_val = px_null();
    int px_err_2021_proped = 0;
    px_srcline(1776);
    _v1993 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1991, px_int(1LL))}, 1);
    px_srcline(1777);
    _v1994 = px_index(_v1991, px_int(2LL));
    px_srcline(1778);
    _v1995 = px_index(_v1991, px_int(3LL));
    px_srcline(1787);
    _v1996 = px_index(_v1992, px_str("is_top"));
    px_srcline(1789);
    _v1997 = px_list_n((LXValue[]){}, 0);
    px_srcline(1790);
    _v1998 = px_int(0LL);
    px_srcline(1791);
    while (px_is_truthy(px_lt(_v1998, px_call(px_get_global("len"), (LXValue[]){_v1993}, 1)))) {
        px_srcline(1792);
        _v1999 = px_neg(px_int(1LL));
        px_srcline(1793);
        if (px_is_truthy(_v1996)) {
            px_srcline(1794);
             _v1999 = px_neg(px_int(1LL));
        }
        else if (px_is_truthy(px_method(px_index(_v1992, px_str("smap")), "has", (LXValue[]){px_index(_v1993, _v1998)}, 1))) {
            px_srcline(1796);
             _v1999 = px_index(px_index(_v1992, px_str("smap")), px_index(_v1993, _v1998));
        }
        else {
            px_srcline(1798);
             _v1999 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1992, px_index(_v1993, _v1998)}, 2);
        }
        px_srcline(1799);
        (void)(px_method(_v1997, "append", (LXValue[]){_v1999}, 1));
        px_srcline(1800);
         _v1998 = px_add(_v1998, px_int(1LL));
    }
    px_srcline(1802);
    _v2000 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
    px_srcline(1803);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1994, _v2000, _v1992}, 3));
    px_srcline(1805);
    _v2001 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
    px_srcline(1806);
    _v2002 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
    px_srcline(1807);
    while (px_is_truthy(px_lt(px_index(_v1992, px_str("next_slot")), px_add(_v2001, px_int(2LL))))) {
        px_srcline(1808);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1));
    }
    px_srcline(1809);
    _v2003 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1810);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("GETG"), _v2001, _v2003, px_int(0LL)}, 5));
    px_srcline(1811);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("MOV"), px_add(_v2001, px_int(1LL)), _v2000, px_int(0LL)}, 5));
    px_srcline(1812);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("CALL"), _v2002, _v2001, px_int(1LL)}, 5));
    px_srcline(1814);
    _v2004 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
    px_srcline(1815);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1992, _v2004, px_int(0LL)}, 3));
    px_srcline(1816);
    _v2005 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1992, px_str("bc"))}, 1);
    px_srcline(1818);
    _v2006 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
    px_srcline(1819);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("LT"), _v2006, _v2004, _v2002}, 5));
    px_srcline(1820);
    _v2007 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1992, px_str("bc"))}, 1);
    px_srcline(1821);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("JMPF"), _v2006, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1824);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("ITERLEN"), _v2000, _v2002, px_int(0LL)}, 5));
    px_srcline(1827);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1993}, 1), px_int(1LL)))) {
        px_srcline(1834);
        if (px_is_truthy(_v1996)) {
            px_srcline(1836);
            _v2008 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
            px_srcline(1837);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("ITERAT"), _v2008, _v2000, _v2004}, 5));
            px_srcline(1838);
            _v2009 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_index(_v1993, px_int(0LL))}, 2);
            px_srcline(1839);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("SETG"), _v2009, _v2008, px_int(0LL)}, 5));
        }
        else if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1992, px_index(_v1993, px_int(0LL))}, 2))) {
            px_srcline(1841);
            _v2008 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
            px_srcline(1842);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("ITERAT"), _v2008, _v2000, _v2004}, 5));
            px_srcline(1843);
            (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1992, px_index(_v1993, px_int(0LL)), px_index(_v1997, px_int(0LL)), _v2008}, 4));
        }
        else {
            px_srcline(1845);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("ITERAT"), px_index(_v1997, px_int(0LL)), _v2000, _v2004}, 5));
        }
    }
    else {
        px_srcline(1849);
        _v2010 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
        px_srcline(1850);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("ITERAT"), _v2010, _v2000, _v2004}, 5));
        px_srcline(1851);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("UNPACKCK"), _v2010, px_call(px_get_global("len"), (LXValue[]){_v1993}, 1), px_int(0LL)}, 5));
        px_srcline(1852);
        _v2011 = px_int(0LL);
        px_srcline(1853);
        while (px_is_truthy(px_lt(_v2011, px_call(px_get_global("len"), (LXValue[]){_v1993}, 1)))) {
            px_srcline(1854);
            _v2012 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
            px_srcline(1855);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1992, _v2012, _v2011}, 3));
            px_srcline(1856);
            if (px_is_truthy(_v1996)) {
                px_srcline(1858);
                _v2013 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
                px_srcline(1859);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("INDEX"), _v2013, _v2010, _v2012}, 5));
                px_srcline(1860);
                _v2014 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_index(_v1993, _v2011)}, 2);
                px_srcline(1861);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("SETG"), _v2014, _v2013, px_int(0LL)}, 5));
            }
            else if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1992, px_index(_v1993, _v2011)}, 2))) {
                px_srcline(1864);
                _v2013 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
                px_srcline(1865);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("INDEX"), _v2013, _v2010, _v2012}, 5));
                px_srcline(1866);
                (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1992, px_index(_v1993, _v2011), px_index(_v1997, _v2011), _v2013}, 4));
            }
            else {
                px_srcline(1868);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("INDEX"), px_index(_v1997, _v2011), _v2010, _v2012}, 5));
            }
            px_srcline(1869);
             _v2011 = px_add(_v2011, px_int(1LL));
        }
    }
    px_srcline(1870);
    _v2015 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1871);
    px_index_set(_v2015, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1872);
    px_index_set(_v2015, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1873);
    (void)(px_method(px_index(_v1992, px_str("loops")), "push", (LXValue[]){_v2015}, 1));
    px_srcline(1874);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1995, _v1992}, 2));
    px_srcline(1875);
    (void)(px_method(px_index(_v1992, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1880);
    _v2016 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1992, px_str("bc"))}, 1);
    px_srcline(1881);
    _v2017 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1992}, 1);
    px_srcline(1882);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1992, _v2017, px_int(1LL)}, 3));
    px_srcline(1883);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("ADD"), _v2004, _v2004, _v2017}, 5));
    px_srcline(1884);
    _v2018 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1992, px_str("bc"))}, 1);
    px_srcline(1885);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1992, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1886);
    _v2019 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1992, px_str("bc"))}, 1);
    px_srcline(1887);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1992, _v2018, _v2005}, 3));
    px_srcline(1888);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1992, _v2007, _v2019}, 3));
    px_srcline(1889);
    _v2020 = px_int(0LL);
    px_srcline(1890);
    while (px_is_truthy(px_lt(_v2020, px_call(px_get_global("len"), (LXValue[]){px_index(_v2015, px_str("breaks"))}, 1)))) {
        px_srcline(1891);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1992, px_index(px_index(_v2015, px_str("breaks")), _v2020), _v2019}, 3));
        px_srcline(1892);
         _v2020 = px_add(_v2020, px_int(1LL));
    }
    px_srcline(1893);
     _v2020 = px_int(0LL);
    px_srcline(1894);
    while (px_is_truthy(px_lt(_v2020, px_call(px_get_global("len"), (LXValue[]){px_index(_v2015, px_str("conts"))}, 1)))) {
        px_srcline(1895);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1992, px_index(px_index(_v2015, px_str("conts")), _v2020), _v2016}, 3));
        px_srcline(1896);
         _v2020 = px_add(_v2020, px_int(1LL));
    }
px_err_2021:
    if (px_err_2021_proped) return px_err_2021_val;
    return px_null();
}

static LXValue fn_bc_emit_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_def");
    LXValue _v2022 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2023 = px_null();
    LXValue px_err_2024_val = px_null();
    int px_err_2024_proped = 0;
    px_srcline(1900);
    _v2023 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2022, px_int(1LL))}, 1);
    px_srcline(1901);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v2022, _v2023}, 2);
px_err_2024:
    if (px_err_2024_proped) return px_err_2024_val;
    return px_null();
}

static LXValue fn_bc_emit_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_impl_def");
    LXValue _v2025 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2026 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2027 = px_null();
    LXValue px_err_2028_val = px_null();
    int px_err_2028_proped = 0;
    px_srcline(1904);
    _v2027 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2025, px_int(1LL))}, 1);
    px_srcline(1905);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v2025, px_add(px_add(_v2026, px_str(".")), _v2027)}, 2);
px_err_2028:
    if (px_err_2028_proped) return px_err_2028_val;
    return px_null();
}

static LXValue fn_bc_emit_func_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_body");
    LXValue _v2029 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2030 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2031 = px_null();
    LXValue _v2032 = px_null();
    LXValue _v2033 = px_null();
    LXValue _v2034 = px_null();
    LXValue _v2035 = px_null();
    LXValue _v2036 = px_null();
    LXValue _v2037 = px_null();
    LXValue _v2038 = px_null();
    LXValue _v2039 = px_null();
    LXValue _v2040 = px_null();
    LXValue px_err_2041_val = px_null();
    int px_err_2041_proped = 0;
    px_srcline(1914);
    _v2031 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(1915);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v2031}, 1));
    px_srcline(1916);
    _v2032 = px_index(_v2029, px_int(2LL));
    px_srcline(1921);
    _v2033 = px_call(px_get_global("len"), (LXValue[]){_v2032}, 1);
    px_srcline(1922);
    _v2034 = px_int(0LL);
    px_srcline(1923);
    _v2035 = px_int(0LL);
    px_srcline(1924);
    while (px_is_truthy(px_lt(_v2035, px_call(px_get_global("len"), (LXValue[]){_v2032}, 1)))) {
        px_srcline(1925);
        if (px_is_truthy(px_ne(px_index(px_index(_v2032, _v2035), px_int(3LL)), px_null()))) {
            px_srcline(1926);
            if (px_is_truthy(px_eq(_v2034, px_int(0LL)))) {
                px_srcline(1927);
                 _v2033 = _v2035;
            }
            px_srcline(1928);
             _v2034 = px_add(_v2034, px_int(1LL));
        }
        px_srcline(1929);
         _v2035 = px_add(_v2035, px_int(1LL));
    }
    px_srcline(1930);
    _v2036 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v2030, _v2033}, 2);
    px_srcline(1931);
    px_index_set(_v2036, px_str("ndefault"), _v2034);
    px_srcline(1933);
     _v2035 = px_int(0LL);
    px_srcline(1934);
    while (px_is_truthy(px_lt(_v2035, px_call(px_get_global("len"), (LXValue[]){_v2032}, 1)))) {
        px_srcline(1935);
        px_index_set(px_index(_v2036, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2032, _v2035), px_int(1LL))}, 1), _v2035);
        px_srcline(1936);
         _v2035 = px_add(_v2035, px_int(1LL));
    }
    px_srcline(1937);
    if (px_is_truthy(px_lt(px_index(_v2036, px_str("next_slot")), px_call(px_get_global("len"), (LXValue[]){_v2032}, 1)))) {
        px_srcline(1938);
        px_index_set(_v2036, px_str("next_slot"), px_call(px_get_global("len"), (LXValue[]){_v2032}, 1));
    }
    px_srcline(1940);
    _v2037 = px_list_n((LXValue[]){}, 0);
    px_srcline(1941);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v2029, px_int(4LL)), _v2037}, 2));
    px_srcline(1943);
    _v2038 = px_list_n((LXValue[]){}, 0);
    px_srcline(1944);
    (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v2029, px_int(4LL)), _v2038}, 2));
    px_srcline(1945);
    _v2039 = px_int(0LL);
    px_srcline(1946);
    while (px_is_truthy(px_lt(_v2039, px_call(px_get_global("len"), (LXValue[]){_v2037}, 1)))) {
        px_srcline(1947);
        _v2040 = px_index(_v2037, _v2039);
        px_srcline(1948);
        if (px_is_truthy(px_method(px_index(_v2036, px_str("smap")), "has", (LXValue[]){_v2040}, 1))) {
            px_srcline(1949);
             _v2039 = px_add(_v2039, px_int(1LL));
            px_srcline(1950);
            continue;
        }
        px_srcline(1955);
        if (px_is_truthy(({ LXValue _t2042 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v2038, _v2040}, 2)); px_is_truthy(_t2042) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v2040}, 2) : _t2042; }))) {
            px_srcline(1956);
             _v2039 = px_add(_v2039, px_int(1LL));
            px_srcline(1957);
            continue;
        }
        px_srcline(1958);
        (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v2036, _v2040}, 2));
        px_srcline(1959);
         _v2039 = px_add(_v2039, px_int(1LL));
    }
    px_srcline(1964);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v2036, px_index(_v2029, px_int(4LL))}, 2));
    px_srcline(1966);
    if (px_is_truthy(px_gt(_v2034, px_int(0LL)))) {
        px_srcline(1967);
        (void)(px_call(px_get_global("bc_emit_default_fill"), (LXValue[]){_v2036, _v2032, _v2033}, 3));
    }
    px_srcline(1968);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(_v2029, px_int(4LL)), _v2036}, 2));
    px_srcline(1969);
    px_set_global("cg_immutables", _v2031);
    px_srcline(1970);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2036, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1971);
    px_index_set(_v2036, px_str("nslots"), px_index(_v2036, px_str("next_slot")));
    px_srcline(1972);
    return _v2036;
px_err_2041:
    if (px_err_2041_proped) return px_err_2041_val;
    return px_null();
}

static LXValue fn_bc_emit_default_fill(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_default_fill");
    LXValue _v2043 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2044 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2045 = (nargs > 2) ? args[2] : px_null();
    LXValue _v2046 = px_null();
    LXValue _v2047 = px_null();
    LXValue _v2048 = px_null();
    LXValue _v2049 = px_null();
    LXValue _v2050 = px_null();
    LXValue _v2051 = px_null();
    LXValue px_err_2052_val = px_null();
    int px_err_2052_proped = 0;
    px_srcline(1978);
    _v2046 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2043}, 1);
    px_srcline(1979);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2043, px_str("NARGS"), _v2046, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1980);
    _v2047 = _v2045;
    px_srcline(1981);
    while (px_is_truthy(px_lt(_v2047, px_call(px_get_global("len"), (LXValue[]){_v2044}, 1)))) {
        px_srcline(1982);
        if (px_is_truthy(px_ne(px_index(px_index(_v2044, _v2047), px_int(3LL)), px_null()))) {
            px_srcline(1983);
            _v2048 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2043}, 1);
            px_srcline(1984);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2043, _v2048, _v2047}, 3));
            px_srcline(1985);
            _v2049 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2043}, 1);
            px_srcline(1986);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2043, px_str("LT"), _v2049, _v2048, _v2046}, 5));
            px_srcline(1987);
            _v2050 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2043, px_str("bc"))}, 1);
            px_srcline(1988);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2043, px_str("JMPT"), _v2049, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1991);
            if (px_is_truthy(px_method(px_index(_v2043, px_str("cell")), "has", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2044, _v2047), px_int(1LL))}, 1)}, 1))) {
                px_srcline(1992);
                _v2051 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2043}, 1);
                px_srcline(1993);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2044, _v2047), px_int(3LL)), _v2051, _v2043}, 3));
                px_srcline(1994);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2043, px_str("CELLSET"), _v2051, _v2047, px_int(0LL)}, 5));
            }
            else {
                px_srcline(1996);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2044, _v2047), px_int(3LL)), _v2047, _v2043}, 3));
            }
            px_srcline(1997);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2043, _v2050, px_call(px_get_global("len"), (LXValue[]){px_index(_v2043, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1998);
         _v2047 = px_add(_v2047, px_int(1LL));
    }
px_err_2052:
    if (px_err_2052_proped) return px_err_2052_val;
    return px_null();
}

static LXValue fn_bc_emit_func_top(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_top");
    LXValue _v2053 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2054 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2055 = (nargs > 2) ? args[2] : px_null();
    LXValue _v2056 = px_null();
    LXValue _v2057 = px_null();
    LXValue _v2058 = px_null();
    LXValue _v2059 = px_null();
    LXValue px_err_2060_val = px_null();
    int px_err_2060_proped = 0;
    px_srcline(2003);
    _v2056 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v2054, px_int(0LL)}, 2);
    px_srcline(2004);
    px_index_set(_v2056, px_str("is_top"), px_bool(true));
    px_srcline(2005);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2053, _v2056}, 2));
    px_srcline(2006);
    if (px_is_truthy(_v2055)) {
        px_srcline(2007);
        _v2057 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2056}, 1);
        px_srcline(2008);
        _v2058 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("main")}, 2);
        px_srcline(2009);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2056, px_str("GETG"), _v2057, _v2058, px_int(0LL)}, 5));
        px_srcline(2010);
        _v2059 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2056}, 1);
        px_srcline(2011);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2056, px_str("CALL"), _v2059, _v2057, px_int(0LL)}, 5));
        px_srcline(2012);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2056, px_str("RET"), _v2059, px_int(0LL), px_int(0LL)}, 5));
    }
    else {
        px_srcline(2014);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2056, px_str("HALT"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    }
    px_srcline(2015);
    px_index_set(_v2056, px_str("nslots"), px_index(_v2056, px_str("next_slot")));
    px_srcline(2016);
    return _v2056;
px_err_2060:
    if (px_err_2060_proped) return px_err_2060_val;
    return px_null();
}

static LXValue fn_bc_emit_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_program");
    LXValue _v2061 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2062 = px_null();
    LXValue _v2063 = px_null();
    LXValue _v2064 = px_null();
    LXValue _v2065 = px_null();
    LXValue _v2066 = px_null();
    LXValue _v2067 = px_null();
    LXValue _v2068 = px_null();
    LXValue _v2069 = px_null();
    LXValue _v2070 = px_null();
    LXValue _v2071 = px_null();
    LXValue _v2072 = px_null();
    LXValue _v2073 = px_null();
    LXValue _v2074 = px_null();
    LXValue _v2075 = px_null();
    LXValue _v2076 = px_null();
    LXValue _v2077 = px_null();
    LXValue _v2078 = px_null();
    LXValue px_err_2079_val = px_null();
    int px_err_2079_proped = 0;
    px_srcline(2020);
    _v2062 = px_index(_v2061, px_int(1LL));
    px_srcline(2021);
    _v2063 = px_call(px_get_global("bc_new_module"), (LXValue[]){px_str("<module>")}, 1);
    px_srcline(2022);
    px_set_global("g_bcm", _v2063);
    px_srcline(2025);
    _v2064 = px_int(0LL);
    px_srcline(2026);
    while (px_is_truthy(px_lt(_v2064, px_call(px_get_global("len"), (LXValue[]){_v2062}, 1)))) {
        px_srcline(2027);
        _v2065 = px_index(_v2062, _v2064);
        px_srcline(2028);
        _v2066 = px_index(_v2065, px_int(0LL));
        px_srcline(2029);
        if (px_is_truthy(px_eq(_v2066, px_str("StructDef")))) {
            px_srcline(2030);
            _v2067 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
            px_srcline(2031);
            px_index_set(_v2067, px_str("name"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2065, px_int(1LL))}, 1));
            px_srcline(2032);
            _v2068 = px_list_n((LXValue[]){}, 0);
            px_srcline(2033);
            _v2069 = px_int(0LL);
            px_srcline(2034);
            while (px_is_truthy(px_lt(_v2069, px_call(px_get_global("len"), (LXValue[]){px_index(_v2065, px_int(2LL))}, 1)))) {
                px_srcline(2035);
                (void)(px_method(_v2068, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v2065, px_int(2LL)), _v2069), px_int(1LL))}, 1)}, 1));
                px_srcline(2036);
                 _v2069 = px_add(_v2069, px_int(1LL));
            }
            px_srcline(2037);
            px_index_set(_v2067, px_str("fnames"), _v2068);
            px_srcline(2038);
            (void)(px_method(px_index(_v2063, px_str("structs")), "push", (LXValue[]){_v2067}, 1));
        }
        else if (px_is_truthy(px_eq(_v2066, px_str("EnumDef")))) {
            px_srcline(2040);
            _v2070 = px_list_n((LXValue[]){}, 0);
            px_srcline(2041);
            _v2071 = px_int(0LL);
            px_srcline(2042);
            while (px_is_truthy(px_lt(_v2071, px_call(px_get_global("len"), (LXValue[]){px_index(_v2065, px_int(2LL))}, 1)))) {
                px_srcline(2043);
                (void)(px_method(_v2070, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v2065, px_int(2LL)), _v2071), px_int(1LL))}, 1)}, 1));
                px_srcline(2044);
                 _v2071 = px_add(_v2071, px_int(1LL));
            }
            px_srcline(2045);
            px_index_set(px_index(_v2063, px_str("enums")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2065, px_int(1LL))}, 1), _v2070);
        }
        px_srcline(2046);
         _v2064 = px_add(_v2064, px_int(1LL));
    }
    px_srcline(2047);
    (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){_v2062}, 1));
    px_srcline(2048);
    _v2072 = px_call(px_get_global("bc_collect_impl_list"), (LXValue[]){_v2062}, 1);
    px_srcline(2055);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2056);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2057);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2058);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2059);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2060);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2061);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2062);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2063);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2064);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(2066);
     _v2064 = px_int(0LL);
    px_srcline(2067);
    while (px_is_truthy(px_lt(_v2064, px_call(px_get_global("len"), (LXValue[]){_v2062}, 1)))) {
        px_srcline(2068);
        _v2065 = px_index(_v2062, _v2064);
        px_srcline(2069);
        _v2066 = px_index(_v2065, px_int(0LL));
        px_srcline(2070);
        if (px_is_truthy(px_eq(_v2066, px_str("FuncDef")))) {
            px_srcline(2071);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2063, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2065, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v2066, px_str("ExternDef")))) {
            px_srcline(2078);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2065, px_int(1LL))}, 1), px_index(_v2065, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v2066, px_str("VarDecl")))) {
            px_srcline(2080);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2063, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2065, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v2066, px_str("Assign")))) {
            px_srcline(2082);
            _v2073 = px_index(_v2065, px_int(1LL));
            px_srcline(2083);
            if (px_is_truthy(px_eq(px_index(_v2073, px_int(0LL)), px_str("Var")))) {
                px_srcline(2084);
                (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2063, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2073, px_int(1LL))}, 1)}, 2));
            }
        }
        px_srcline(2085);
         _v2064 = px_add(_v2064, px_int(1LL));
    }
    px_srcline(2091);
    px_set_global("g_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(2092);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){_v2062, px_get_global("g_topnames")}, 2));
    px_srcline(2095);
     _v2064 = px_int(0LL);
    px_srcline(2096);
    while (px_is_truthy(px_lt(_v2064, px_call(px_get_global("len"), (LXValue[]){_v2072}, 1)))) {
        px_srcline(2097);
        _v2074 = px_index(_v2072, _v2064);
        px_srcline(2098);
        _v2075 = px_call(px_get_global("bc_emit_impl_def"), (LXValue[]){px_index(_v2074, px_int(1LL)), px_index(_v2074, px_int(0LL))}, 2);
        px_srcline(2099);
        (void)(px_method(px_index(_v2063, px_str("funcs")), "push", (LXValue[]){_v2075}, 1));
        px_srcline(2100);
         _v2064 = px_add(_v2064, px_int(1LL));
    }
    px_srcline(2101);
     _v2064 = px_int(0LL);
    px_srcline(2102);
    while (px_is_truthy(px_lt(_v2064, px_call(px_get_global("len"), (LXValue[]){_v2062}, 1)))) {
        px_srcline(2103);
        _v2065 = px_index(_v2062, _v2064);
        px_srcline(2104);
        if (px_is_truthy(px_eq(px_index(_v2065, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(2105);
            _v2075 = px_call(px_get_global("bc_emit_func_def"), (LXValue[]){_v2065}, 1);
            px_srcline(2106);
            (void)(px_method(px_index(_v2063, px_str("funcs")), "push", (LXValue[]){_v2075}, 1));
        }
        px_srcline(2107);
         _v2064 = px_add(_v2064, px_int(1LL));
    }
    px_srcline(2109);
    _v2076 = px_list_n((LXValue[]){}, 0);
    px_srcline(2110);
     _v2064 = px_int(0LL);
    px_srcline(2111);
    while (px_is_truthy(px_lt(_v2064, px_call(px_get_global("len"), (LXValue[]){_v2062}, 1)))) {
        px_srcline(2112);
        _v2065 = px_index(_v2062, _v2064);
        px_srcline(2113);
        _v2066 = px_index(_v2065, px_int(0LL));
        px_srcline(2114);
        if (px_is_truthy(({ LXValue _t2086 = ({ LXValue _t2085 = ({ LXValue _t2084 = ({ LXValue _t2083 = ({ LXValue _t2082 = ({ LXValue _t2081 = ({ LXValue _t2080 = px_ne(_v2066, px_str("FuncDef")); px_is_truthy(_t2080) ? px_ne(_v2066, px_str("StructDef")) : _t2080; }); px_is_truthy(_t2081) ? px_ne(_v2066, px_str("EnumDef")) : _t2081; }); px_is_truthy(_t2082) ? px_ne(_v2066, px_str("TraitDef")) : _t2082; }); px_is_truthy(_t2083) ? px_ne(_v2066, px_str("ImplDef")) : _t2083; }); px_is_truthy(_t2084) ? px_ne(_v2066, px_str("Import")) : _t2084; }); px_is_truthy(_t2085) ? px_ne(_v2066, px_str("ExternDef")) : _t2085; }); px_is_truthy(_t2086) ? px_ne(_v2066, px_str("TypeConst")) : _t2086; }))) {
            px_srcline(2115);
            (void)(px_method(_v2076, "append", (LXValue[]){_v2065}, 1));
        }
        px_srcline(2116);
         _v2064 = px_add(_v2064, px_int(1LL));
    }
    px_srcline(2118);
    _v2077 = px_bool(false);
    px_srcline(2119);
     _v2064 = px_int(0LL);
    px_srcline(2120);
    while (px_is_truthy(px_lt(_v2064, px_call(px_get_global("len"), (LXValue[]){_v2062}, 1)))) {
        px_srcline(2121);
        _v2065 = px_index(_v2062, _v2064);
        px_srcline(2122);
        if (px_is_truthy(({ LXValue _t2087 = px_eq(px_index(_v2065, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t2087) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2065, px_int(1LL))}, 1), px_str("main")) : _t2087; }))) {
            px_srcline(2123);
             _v2077 = px_bool(true);
        }
        px_srcline(2124);
         _v2064 = px_add(_v2064, px_int(1LL));
    }
    px_srcline(2125);
    _v2078 = px_call(px_get_global("bc_emit_func_top"), (LXValue[]){_v2076, px_str("<top>"), _v2077}, 3);
    px_srcline(2126);
    (void)(px_method(px_index(_v2063, px_str("funcs")), "push", (LXValue[]){_v2078}, 1));
    px_srcline(2127);
    px_index_set(_v2063, px_str("top"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v2063, px_str("funcs"))}, 1), px_int(1LL)));
    px_srcline(2128);
    return _v2063;
px_err_2079:
    if (px_err_2079_proped) return px_err_2079_val;
    return px_null();
}

static LXValue fn_bc_dump_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_dump_module");
    LXValue _v2088 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2089 = px_null();
    LXValue _v2090 = px_null();
    LXValue _v2091 = px_null();
    LXValue _v2092 = px_null();
    LXValue _v2093 = px_null();
    LXValue _v2094 = px_null();
    LXValue _v2095 = px_null();
    LXValue _v2096 = px_null();
    LXValue _v2097 = px_null();
    LXValue px_err_2098_val = px_null();
    int px_err_2098_proped = 0;
    px_srcline(2131);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# BCModule "), px_index(_v2088, px_str("name")))}, 1));
    px_srcline(2132);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# K "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2088, px_str("k_pool"))}, 1)}, 1))}, 1));
    px_srcline(2133);
    _v2089 = px_int(0LL);
    px_srcline(2134);
    while (px_is_truthy(px_lt(_v2089, px_call(px_get_global("len"), (LXValue[]){px_index(_v2088, px_str("k_pool"))}, 1)))) {
        px_srcline(2135);
        _v2090 = px_index(px_index(_v2088, px_str("k_pool")), _v2089);
        px_srcline(2136);
        if (px_is_truthy(px_eq(px_index(_v2090, px_str("kind")), px_str("int")))) {
            px_srcline(2137);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s215 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2089}, 1)), px_str(" int ")); LXValue _s216 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2090, px_str("i"))}, 1); px_add(_s215, _s216); })}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2090, px_str("kind")), px_str("float")))) {
            px_srcline(2139);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s217 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2089}, 1)), px_str(" float ")); LXValue _s218 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2090, px_str("f"))}, 1); px_add(_s217, _s218); })}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2090, px_str("kind")), px_str("str")))) {
            px_srcline(2141);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s219 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2089}, 1)), px_str(" str \"")); LXValue _s220 = px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2090, px_str("s"))}, 1); px_add(_s219, _s220); }), px_str("\""))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2090, px_str("kind")), px_str("bool")))) {
            px_srcline(2143);
            _v2091 = px_str("false");
            px_srcline(2144);
            if (px_is_truthy(px_ne(px_index(_v2090, px_str("i")), px_int(0LL)))) {
                px_srcline(2145);
                 _v2091 = px_str("true");
            }
            px_srcline(2146);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2089}, 1)), px_str(" bool ")), _v2091)}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2090, px_str("kind")), px_str("func")))) {
            px_srcline(2148);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s221 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2089}, 1)), px_str(" func ")); LXValue _s222 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2090, px_str("i"))}, 1); px_add(_s221, _s222); })}, 1));
        }
        else {
            px_srcline(2150);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2089}, 1)), px_str(" null"))}, 1));
        }
        px_srcline(2151);
         _v2089 = px_add(_v2089, px_int(1LL));
    }
    px_srcline(2152);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# G "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2088, px_str("globals"))}, 1)}, 1))}, 1));
    px_srcline(2153);
    _v2092 = px_int(0LL);
    px_srcline(2154);
    while (px_is_truthy(px_lt(_v2092, px_call(px_get_global("len"), (LXValue[]){px_index(_v2088, px_str("globals"))}, 1)))) {
        px_srcline(2155);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("G "), px_call(px_get_global("str"), (LXValue[]){_v2092}, 1)), px_str(" ")), px_index(px_index(_v2088, px_str("globals")), _v2092))}, 1));
        px_srcline(2156);
         _v2092 = px_add(_v2092, px_int(1LL));
    }
    px_srcline(2157);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s223 = px_add(px_add(px_str("# funcs "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2088, px_str("funcs"))}, 1)}, 1)), px_str(" top=")); LXValue _s224 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2088, px_str("top"))}, 1); px_add(_s223, _s224); })}, 1));
    px_srcline(2158);
    _v2093 = px_int(0LL);
    px_srcline(2159);
    while (px_is_truthy(px_lt(_v2093, px_call(px_get_global("len"), (LXValue[]){px_index(_v2088, px_str("funcs"))}, 1)))) {
        px_srcline(2160);
        _v2094 = px_index(px_index(_v2088, px_str("funcs")), _v2093);
        px_srcline(2161);
        _v2095 = px_str("");
        px_srcline(2162);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2094, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2163);
             _v2095 = px_add(({ LXValue _s225 = px_add(px_add(px_str(" up="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2094, px_str("upnames"))}, 1)}, 1)), px_str("[")); LXValue _s226 = px_call(px_get_global("join"), (LXValue[]){px_str(","), px_index(_v2094, px_str("upnames"))}, 2); px_add(_s225, _s226); }), px_str("]"));
        }
        px_srcline(2164);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s229 = px_add(({ LXValue _s227 = px_add(px_add(px_add(px_add(px_str("== func "), px_index(_v2094, px_str("name"))), px_str(" arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v2094, px_str("arity"))}, 1)), px_str(" ndefault=")); LXValue _s228 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2094, px_str("ndefault"))}, 1); px_add(_s227, _s228); }), px_str(" nslots=")); LXValue _s230 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2094, px_str("nslots"))}, 1); px_add(_s229, _s230); }), _v2095)}, 1));
        px_srcline(2165);
        _v2096 = px_int(0LL);
        px_srcline(2166);
        while (px_is_truthy(px_lt(_v2096, px_call(px_get_global("len"), (LXValue[]){px_index(_v2094, px_str("bc"))}, 1)))) {
            px_srcline(2167);
            _v2097 = px_index(px_index(_v2094, px_str("bc")), _v2096);
            px_srcline(2168);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s235 = px_add(({ LXValue _s233 = px_add(({ LXValue _s231 = px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v2096}, 1), px_str(":")), px_index(_v2097, px_int(0LL))), px_str(" ")); LXValue _s232 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2097, px_int(2LL))}, 1); px_add(_s231, _s232); }), px_str(" ")); LXValue _s234 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2097, px_int(3LL))}, 1); px_add(_s233, _s234); }), px_str(" ")); LXValue _s236 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2097, px_int(4LL))}, 1); px_add(_s235, _s236); })}, 1));
            px_srcline(2169);
             _v2096 = px_add(_v2096, px_int(1LL));
        }
        px_srcline(2170);
         _v2093 = px_add(_v2093, px_int(1LL));
    }
px_err_2098:
    if (px_err_2098_proped) return px_err_2098_val;
    return px_null();
}

static LXValue fn_bc_emit_c_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_c_program");
    LXValue _v2099 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2100 = px_null();
    LXValue _v2101 = px_null();
    LXValue _v2102 = px_null();
    LXValue _v2103 = px_null();
    LXValue _v2104 = px_null();
    LXValue _v2105 = px_null();
    LXValue _v2106 = px_null();
    LXValue _v2107 = px_null();
    LXValue _v2108 = px_null();
    LXValue _v2109 = px_null();
    LXValue _v2110 = px_null();
    LXValue _v2111 = px_null();
    LXValue _v2112 = px_null();
    LXValue _v2113 = px_null();
    LXValue _v2114 = px_null();
    LXValue _v2115 = px_null();
    LXValue _v2116 = px_null();
    LXValue px_err_2117_val = px_null();
    int px_err_2117_proped = 0;
    px_srcline(2177);
    _v2100 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2099}, 1);
    px_srcline(2182);
    _v2101 = px_list_n((LXValue[]){px_str("/* 由 bc_emit.px (M89-S3) 自动生成 — 字节码模块（VM 执行） */\n")}, 1);
    px_srcline(2183);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("#include \"runtime.h\"\n#include \"vm.h\"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n")}, 1));
    px_srcline(2184);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const PxBCModule s_mod;\n\n")}, 1));
    px_srcline(2186);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const PxK s_K[] = {\n")}, 1));
    px_srcline(2187);
    _v2102 = px_int(0LL);
    px_srcline(2188);
    while (px_is_truthy(px_lt(_v2102, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("k_pool"))}, 1)))) {
        px_srcline(2189);
        _v2103 = px_index(px_index(_v2100, px_str("k_pool")), _v2102);
        px_srcline(2190);
        if (px_is_truthy(px_eq(px_index(_v2103, px_str("kind")), px_str("int")))) {
            px_srcline(2191);
            (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_INT, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v2103, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2103, px_str("kind")), px_str("float")))) {
            px_srcline(2193);
            (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FLT, 0, "), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v2103, px_str("f"))}, 1)), px_str(", NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2103, px_str("kind")), px_str("str")))) {
            px_srcline(2196);
            if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){px_index(_v2103, px_str("s"))}, 1))) {
                px_srcline(2197);
                (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    PXK_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2103, px_str("s"))}, 1)), px_str("\"),\n"))}, 1));
            }
            else {
                px_srcline(2199);
                (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_STR, 0, 0, \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2103, px_str("s"))}, 1)), px_str("\"},\n"))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v2103, px_str("kind")), px_str("bool")))) {
            px_srcline(2201);
            _v2104 = px_str("0");
            px_srcline(2202);
            if (px_is_truthy(px_ne(px_index(_v2103, px_str("i")), px_int(0LL)))) {
                px_srcline(2203);
                 _v2104 = px_str("1");
            }
            px_srcline(2204);
            (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_BOOL, "), _v2104), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2103, px_str("kind")), px_str("func")))) {
            px_srcline(2206);
            (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FUNC, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v2103, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else {
            px_srcline(2208);
            (void)(px_method(_v2101, "push", (LXValue[]){px_str("    {PXK_NULL, 0, 0, NULL},\n")}, 1));
        }
        px_srcline(2209);
         _v2102 = px_add(_v2102, px_int(1LL));
    }
    px_srcline(2210);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2212);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const char* s_N[] = {\n")}, 1));
    px_srcline(2213);
    _v2105 = px_int(0LL);
    px_srcline(2214);
    while (px_is_truthy(px_lt(_v2105, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("n_pool"))}, 1)))) {
        px_srcline(2215);
        (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2100, px_str("n_pool")), _v2105)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2216);
         _v2105 = px_add(_v2105, px_int(1LL));
    }
    px_srcline(2217);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2219);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const char* s_G[] = {\n")}, 1));
    px_srcline(2220);
    _v2106 = px_int(0LL);
    px_srcline(2221);
    while (px_is_truthy(px_lt(_v2106, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("globals"))}, 1)))) {
        px_srcline(2222);
        (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2100, px_str("globals")), _v2106)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2223);
         _v2106 = px_add(_v2106, px_int(1LL));
    }
    px_srcline(2224);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2226);
    _v2107 = px_int(0LL);
    px_srcline(2227);
    while (px_is_truthy(px_lt(_v2107, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("structs"))}, 1)))) {
        px_srcline(2228);
        _v2108 = px_index(px_index(_v2100, px_str("structs")), _v2107);
        px_srcline(2229);
        (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("static const char* s_st_"), px_call(px_get_global("str"), (LXValue[]){_v2107}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2230);
        _v2109 = px_int(0LL);
        px_srcline(2231);
        while (px_is_truthy(px_lt(_v2109, px_call(px_get_global("len"), (LXValue[]){px_index(_v2108, px_str("fnames"))}, 1)))) {
            px_srcline(2232);
            (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2108, px_str("fnames")), _v2109)}, 1)), px_str("\",\n"))}, 1));
            px_srcline(2233);
             _v2109 = px_add(_v2109, px_int(1LL));
        }
        px_srcline(2234);
        (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2235);
         _v2107 = px_add(_v2107, px_int(1LL));
    }
    px_srcline(2236);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const PxStructDef s_structs[] = {\n")}, 1));
    px_srcline(2237);
     _v2107 = px_int(0LL);
    px_srcline(2238);
    while (px_is_truthy(px_lt(_v2107, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("structs"))}, 1)))) {
        px_srcline(2239);
        _v2108 = px_index(px_index(_v2100, px_str("structs")), _v2107);
        px_srcline(2240);
        (void)(px_method(_v2101, "push", (LXValue[]){px_add(({ LXValue _s239 = px_add(({ LXValue _s237 = px_add(px_add(px_str("    {\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2108, px_str("name"))}, 1)), px_str("\", s_st_")); LXValue _s238 = px_call(px_get_global("str"), (LXValue[]){_v2107}, 1); px_add(_s237, _s238); }), px_str(", ")); LXValue _s240 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2108, px_str("fnames"))}, 1)}, 1); px_add(_s239, _s240); }), px_str("},\n"))}, 1));
        px_srcline(2241);
         _v2107 = px_add(_v2107, px_int(1LL));
    }
    px_srcline(2242);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2244);
    _v2110 = px_int(0LL);
    px_srcline(2245);
    while (px_is_truthy(px_lt(_v2110, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("funcs"))}, 1)))) {
        px_srcline(2246);
        _v2111 = px_index(px_index(_v2100, px_str("funcs")), _v2110);
        px_srcline(2247);
        (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("static const PxInst s_bc_"), px_call(px_get_global("str"), (LXValue[]){_v2110}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2248);
        _v2112 = px_int(0LL);
        px_srcline(2249);
        while (px_is_truthy(px_lt(_v2112, px_call(px_get_global("len"), (LXValue[]){px_index(_v2111, px_str("bc"))}, 1)))) {
            px_srcline(2250);
            _v2113 = px_index(px_index(_v2111, px_str("bc")), _v2112);
            px_srcline(2256);
            (void)(px_method(_v2101, "push", (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), ({ LXValue _s241 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2113, px_int(1LL))}, 1); LXValue _s242 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2113, px_int(2LL))}, 1); LXValue _s243 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2113, px_int(3LL))}, 1); LXValue _s244 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2113, px_int(4LL))}, 1); px_list_n((LXValue[]){px_str("    {PXOP_"), px_index(_v2113, px_int(0LL)), px_str(", "), _s241, px_str(", "), _s242, px_str(", "), _s243, px_str(", "), _s244, px_str("},\n")}, 11); })}, 2)}, 1));
            px_srcline(2257);
             _v2112 = px_add(_v2112, px_int(1LL));
        }
        px_srcline(2258);
        (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2259);
         _v2110 = px_add(_v2110, px_int(1LL));
    }
    px_srcline(2260);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("\n")}, 1));
    px_srcline(2262);
     _v2110 = px_int(0LL);
    px_srcline(2263);
    while (px_is_truthy(px_lt(_v2110, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("funcs"))}, 1)))) {
        px_srcline(2264);
        _v2111 = px_index(px_index(_v2100, px_str("funcs")), _v2110);
        px_srcline(2265);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2111, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2266);
            _v2114 = px_list_n((LXValue[]){}, 0);
            px_srcline(2267);
            _v2115 = px_int(0LL);
            px_srcline(2268);
            while (px_is_truthy(px_lt(_v2115, px_call(px_get_global("len"), (LXValue[]){px_index(_v2111, px_str("upnames"))}, 1)))) {
                px_srcline(2269);
                (void)(px_method(_v2114, "push", (LXValue[]){px_add(px_add(px_str("\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2111, px_str("upnames")), _v2115)}, 1)), px_str("\""))}, 1));
                px_srcline(2270);
                 _v2115 = px_add(_v2115, px_int(1LL));
            }
            px_srcline(2271);
            (void)(px_method(_v2101, "push", (LXValue[]){px_add(({ LXValue _s245 = px_add(px_add(px_str("static const char* s_up_"), px_call(px_get_global("str"), (LXValue[]){_v2110}, 1)), px_str("[] = {")); LXValue _s246 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v2114}, 2); px_add(_s245, _s246); }), px_str("};\n"))}, 1));
        }
        px_srcline(2272);
         _v2110 = px_add(_v2110, px_int(1LL));
    }
    px_srcline(2274);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const PxVMFunc s_funcs[] = {\n")}, 1));
    px_srcline(2275);
     _v2110 = px_int(0LL);
    px_srcline(2276);
    while (px_is_truthy(px_lt(_v2110, px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("funcs"))}, 1)))) {
        px_srcline(2277);
        _v2111 = px_index(px_index(_v2100, px_str("funcs")), _v2110);
        px_srcline(2278);
        _v2116 = px_str(".upvals=NULL");
        px_srcline(2279);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2111, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2280);
             _v2116 = ({ LXValue _s247 = px_add(px_add(px_str(".upvals=s_up_"), px_call(px_get_global("str"), (LXValue[]){_v2110}, 1)), px_str(", .nup=")); LXValue _s248 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2111, px_str("upnames"))}, 1)}, 1); px_add(_s247, _s248); });
        }
        px_srcline(2281);
        (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_add(({ LXValue _s257 = px_add(({ LXValue _s255 = px_add(({ LXValue _s253 = px_add(({ LXValue _s251 = px_add(({ LXValue _s249 = px_add(px_add(px_str("    {.name=\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2111, px_str("name"))}, 1)), px_str("\", .arity=")); LXValue _s250 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2111, px_str("arity"))}, 1); px_add(_s249, _s250); }), px_str(", .ndefault=")); LXValue _s252 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2111, px_str("ndefault"))}, 1); px_add(_s251, _s252); }), px_str(", .nslots=")); LXValue _s254 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2111, px_str("nslots"))}, 1); px_add(_s253, _s254); }), px_str(", .bc=s_bc_")); LXValue _s256 = px_call(px_get_global("str"), (LXValue[]){_v2110}, 1); px_add(_s255, _s256); }), px_str(", .nbc=")); LXValue _s258 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2111, px_str("bc"))}, 1)}, 1); px_add(_s257, _s258); }), px_str(", .mod=&s_mod, ")), _v2116), px_str("},\n"))}, 1));
        px_srcline(2282);
         _v2110 = px_add(_v2110, px_int(1LL));
    }
    px_srcline(2283);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2285);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("static const PxBCModule s_mod = {\n")}, 1));
    px_srcline(2286);
    (void)(px_method(_v2101, "push", (LXValue[]){px_add(({ LXValue _s259 = px_add(px_add(px_str("    .name=\"<module>\", .K=s_K, .nK="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("k_pool"))}, 1)}, 1)), px_str(", .N=s_N, .nN=")); LXValue _s260 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("n_pool"))}, 1)}, 1); px_add(_s259, _s260); }), px_str(",\n"))}, 1));
    px_srcline(2287);
    (void)(px_method(_v2101, "push", (LXValue[]){px_add(({ LXValue _s263 = px_add(({ LXValue _s261 = px_add(px_add(px_str("    .G=s_G, .nG="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("globals"))}, 1)}, 1)), px_str(", .funcs=s_funcs, .nfuncs=")); LXValue _s262 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("funcs"))}, 1)}, 1); px_add(_s261, _s262); }), px_str(", .top_idx=")); LXValue _s264 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2100, px_str("top"))}, 1); px_add(_s263, _s264); }), px_str(",\n"))}, 1));
    px_srcline(2288);
    (void)(px_method(_v2101, "push", (LXValue[]){px_add(px_add(px_str("    .structs=s_structs, .nstructs="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2100, px_str("structs"))}, 1)}, 1)), px_str(",\n"))}, 1));
    px_srcline(2289);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2291);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("int main(int argc, char** argv) {\n")}, 1));
    px_srcline(2292);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    px_args_init(argc, argv);\n")}, 1));
    px_srcline(2293);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    px_register_builtins();\n")}, 1));
    px_srcline(2294);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    px_gc_set_precise(1);   // M92-S2d：VM 轨产物默认 precise 精确根面（退役保守栈扫描；C 轨逃生舱产物不插此调用 → conservative）\n")}, 1));
    px_srcline(2295);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    LXValue _r = px_vm_run_module(px_vm_state(), &s_mod);\n")}, 1));
    px_srcline(2296);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    int _code = 0;\n")}, 1));
    px_srcline(2297);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    if (px_is_result(_r)) {\n")}, 1));
    px_srcline(2298);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        if (!px_result_ok(_r)) {\n")}, 1));
    px_srcline(2299);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("            fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n")}, 1));
    px_srcline(2300);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("            _code = 1;\n")}, 1));
    px_srcline(2301);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        } else {\n")}, 1));
    px_srcline(2302);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("            LXValue _uv = px_result_unwrap(_r);\n")}, 1));
    px_srcline(2303);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("            if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n")}, 1));
    px_srcline(2304);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2305);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    } else if (_r.type == PX_INT) {\n")}, 1));
    px_srcline(2306);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        _code = (int)_r.as.i;\n")}, 1));
    px_srcline(2307);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2308);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    if (getenv(\"PX_BC_DUMP\")) {\n")}, 1));
    px_srcline(2309);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        int _i;\n")}, 1));
    px_srcline(2310);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        for (_i = 0; _i < (int)s_mod.nG; _i++) {\n")}, 1));
    px_srcline(2311);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("            LXValue _v = px_get_global(s_mod.G[_i]);\n")}, 1));
    px_srcline(2312);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("            if (_v.type != PX_FUNC && _v.type != PX_NATIVE)\n")}, 1));
    px_srcline(2313);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("                printf(\"%s=%s\\n\", s_mod.G[_i], px_to_string(_v));\n")}, 1));
    px_srcline(2314);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2315);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2316);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("    return px_exit_code_final(_code);   // M120（qg-issue 76 E1）：协程隔离错误 → 退出码非 0\n")}, 1));
    px_srcline(2317);
    (void)(px_method(_v2101, "push", (LXValue[]){px_str("}\n")}, 1));
    px_srcline(2318);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v2101}, 2);
px_err_2117:
    if (px_err_2117_proped) return px_err_2117_val;
    return px_null();
}

static LXValue fn_bc_basename(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_basename");
    LXValue _v2118 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2119 = px_null();
    LXValue _v2120 = px_null();
    LXValue px_err_2121_val = px_null();
    int px_err_2121_proped = 0;
    px_srcline(74);
    _v2119 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2118}, 1), px_int(1LL));
    px_srcline(75);
    while (px_is_truthy(px_ge(_v2119, px_int(0LL)))) {
        px_srcline(76);
        if (px_is_truthy(px_eq(px_index(_v2118, _v2119), px_str("/")))) {
            px_srcline(77);
            _v2120 = px_slice(_v2118, px_add(_v2119, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v2118}, 1), px_null());
            px_srcline(78);
            if (px_is_truthy(({ LXValue _t2122 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2120}, 1), px_int(3LL)); px_is_truthy(_t2122) ? px_eq(({ LXValue _s265 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2120}, 1), px_int(3LL)); LXValue _s266 = px_call(px_get_global("len"), (LXValue[]){_v2120}, 1); px_slice(_v2120, _s265, _s266, px_null()); }), px_str(".px")) : _t2122; }))) {
                px_srcline(79);
                return px_slice(_v2120, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2120}, 1), px_int(3LL)), px_null());
            }
            px_srcline(80);
            return _v2120;
        }
        px_srcline(81);
         _v2119 = px_sub(_v2119, px_int(1LL));
    }
    px_srcline(82);
    if (px_is_truthy(({ LXValue _t2123 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2118}, 1), px_int(3LL)); px_is_truthy(_t2123) ? px_eq(({ LXValue _s267 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2118}, 1), px_int(3LL)); LXValue _s268 = px_call(px_get_global("len"), (LXValue[]){_v2118}, 1); px_slice(_v2118, _s267, _s268, px_null()); }), px_str(".px")) : _t2123; }))) {
        px_srcline(83);
        return px_slice(_v2118, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2118}, 1), px_int(3LL)), px_null());
    }
    px_srcline(84);
    return _v2118;
px_err_2121:
    if (px_err_2121_proped) return px_err_2121_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v2124 = px_null();
    LXValue _v2125 = px_null();
    LXValue _v2126 = px_null();
    LXValue _v2127 = px_null();
    LXValue _v2128 = px_null();
    LXValue _v2129 = px_null();
    LXValue _v2130 = px_null();
    LXValue _v2131 = px_null();
    LXValue _v2132 = px_null();
    LXValue _v2133 = px_null();
    LXValue _v2134 = px_null();
    LXValue px_err_2135_val = px_null();
    int px_err_2135_proped = 0;
    px_srcline(87);
    _v2124 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    px_srcline(89);
    if (px_is_truthy(({ LXValue _t2137 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v2124}, 1), px_int(2LL)); px_is_truthy(_t2137) ? ({ LXValue _t2136 = px_eq(px_index(_v2124, px_int(1LL)), px_str("--version")); px_is_truthy(_t2136) ? _t2136 : px_eq(px_index(_v2124, px_int(1LL)), px_str("-v")); }) : _t2137; }))) {
        px_srcline(90);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        px_srcline(91);
        return px_int(0LL);
    }
    px_srcline(94);
    _v2125 = px_bool(false);
    px_srcline(95);
    _v2126 = px_bool(false);
    px_srcline(96);
    _v2127 = px_int(1LL);
    px_srcline(97);
    while (px_is_truthy(px_lt(_v2127, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2124}, 1), px_int(1LL))))) {
        px_srcline(98);
        if (px_is_truthy(({ LXValue _t2138 = px_eq(px_index(_v2124, _v2127), px_str("bc")); px_is_truthy(_t2138) ? _t2138 : px_eq(px_index(_v2124, _v2127), px_str("--emit-c")); }))) {
            px_srcline(99);
             _v2125 = px_bool(true);
        }
        px_srcline(100);
        if (px_is_truthy(px_eq(px_index(_v2124, _v2127), px_str("--emit-c")))) {
            px_srcline(101);
             _v2126 = px_bool(true);
        }
        px_srcline(102);
         _v2127 = px_add(_v2127, px_int(1LL));
    }
    px_srcline(104);
    _v2128 = px_index(_v2124, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2124}, 1), px_int(1LL)));
    px_srcline(105);
    _v2129 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v2128}, 1);
    px_srcline(106);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v2128}, 1)}, 1));
    px_srcline(107);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(108);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(109);
    _v2130 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(110);
    _v2131 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v2130, _v2129}, 2);
    px_srcline(111);
    if (px_is_truthy(_v2125)) {
        px_srcline(112);
        if (px_is_truthy(_v2126)) {
            px_srcline(113);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("bc_emit_c_program"), (LXValue[]){_v2131}, 1)}, 1));
            px_srcline(114);
            return px_int(0LL);
        }
        px_srcline(115);
        _v2132 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2131}, 1);
        px_srcline(116);
        px_index_set(_v2132, px_str("name"), px_call(px_get_global("bc_basename"), (LXValue[]){_v2128}, 1));
        px_srcline(117);
        (void)(px_call(px_get_global("bc_dump_module"), (LXValue[]){_v2132}, 1));
        px_srcline(118);
        return px_int(0LL);
    }
    px_srcline(119);
    _v2133 = px_call(px_get_global("cg_generate"), (LXValue[]){_v2131}, 1);
    px_srcline(121);
    _v2134 = px_call(px_get_global("len"), (LXValue[]){_v2133}, 1);
    px_srcline(122);
    if (px_is_truthy(({ LXValue _t2139 = px_gt(_v2134, px_int(0LL)); px_is_truthy(_t2139) ? px_eq(px_index(_v2133, px_sub(_v2134, px_int(1LL))), px_str("\n")) : _t2139; }))) {
        px_srcline(123);
         _v2133 = px_slice(_v2133, px_int(0LL), px_sub(_v2134, px_int(1LL)), px_null());
    }
    px_srcline(124);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v2133}, 1));
px_err_2135:
    if (px_err_2135_proped) return px_err_2135_val;
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
    px_set_global("cg_side_effect", px_func("cg_side_effect", fn_cg_side_effect, NULL));
    px_set_global("cg_seq_operands", px_func("cg_seq_operands", fn_cg_seq_operands, NULL));
    px_set_global("cg_seq_join", px_func("cg_seq_join", fn_cg_seq_join, NULL));
    px_set_global("cg_seq_wrap", px_func("cg_seq_wrap", fn_cg_seq_wrap, NULL));
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
    px_set_global("cg_iter_len_tmp", px_func("cg_iter_len_tmp", fn_cg_iter_len_tmp, NULL));
    px_set_global("cg_unpack_tmp", px_func("cg_unpack_tmp", fn_cg_unpack_tmp, NULL));
    px_set_global("cg_for_names", px_func("cg_for_names", fn_cg_for_names, NULL));
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
    px_set_global("cg_collect_decl_vars", px_func("cg_collect_decl_vars", fn_cg_collect_decl_vars, NULL));
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
    px_set_global("bc_lambda_decl", px_func("bc_lambda_decl", fn_bc_lambda_decl, NULL));
    px_set_global("bc_box_frame", px_func("bc_box_frame", fn_bc_box_frame, NULL));
    px_set_global("bc_emit_inst", px_func("bc_emit_inst", fn_bc_emit_inst, NULL));
    px_set_global("bc_patch_off", px_func("bc_patch_off", fn_bc_patch_off, NULL));
    px_set_global("bc_collect_hoist", px_func("bc_collect_hoist", fn_bc_collect_hoist, NULL));
    px_set_global("bc_collect_decl_vars", px_func("bc_collect_decl_vars", fn_bc_collect_decl_vars, NULL));
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
    px_set_global("bc_for_names", px_func("bc_for_names", fn_bc_for_names, NULL));
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
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); LXValue _v = px_str("let"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("var"); LXValue _v = px_str("var"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("const"); LXValue _v = px_str("const"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("def"); LXValue _v = px_str("def"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("fn"); LXValue _v = px_str("fn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("struct"); LXValue _v = px_str("struct"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("enum"); LXValue _v = px_str("enum"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("trait"); LXValue _v = px_str("trait"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("impl"); LXValue _v = px_str("impl"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("match"); LXValue _v = px_str("match"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("case"); LXValue _v = px_str("case"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("if"); LXValue _v = px_str("if"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("elif"); LXValue _v = px_str("elif"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("else"); LXValue _v = px_str("else"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("for"); LXValue _v = px_str("for"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("while"); LXValue _v = px_str("while"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("in"); LXValue _v = px_str("in"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("and"); LXValue _v = px_str("and"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("or"); LXValue _v = px_str("or"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("not"); LXValue _v = px_str("not"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("return"); LXValue _v = px_str("return"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("break"); LXValue _v = px_str("break"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("continue"); LXValue _v = px_str("continue"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("import"); LXValue _v = px_str("import"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("from"); LXValue _v = px_str("from"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pub"); LXValue _v = px_str("pub"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("as"); LXValue _v = px_str("as"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("spawn"); LXValue _v = px_str("spawn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("chan"); LXValue _v = px_str("chan"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("send"); LXValue _v = px_str("send"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("recv"); LXValue _v = px_str("recv"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("select"); LXValue _v = px_str("select"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("true"); LXValue _v = px_str("true"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("false"); LXValue _v = px_str("false"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("null"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("None"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("self"); LXValue _v = px_str("self"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("capture"); LXValue _v = px_str("capture"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("extern"); LXValue _v = px_str("extern"); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(39);
    px_set_global("CONT_OPS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("+"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("-"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("*"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("/"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("//"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("%"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("**"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("=="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("!="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("<"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(">"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("<="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(">="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("and"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("or"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("&"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("|"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("^"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("<<"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(">>"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(">>>"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("??"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("|>"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("+="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("-="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("*="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("/="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("//="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("%="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("**="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("&="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("|="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("^="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("<<="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(">>="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(">>>="); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("."); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("?."); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("=>"); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str(","); LXValue _v = px_int(1LL); px_dict_set_checked(_d, _k, _v); } _d; }));
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
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); LXValue _v = px_str("let"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("var"); LXValue _v = px_str("var"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("const"); LXValue _v = px_str("const"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("def"); LXValue _v = px_str("def"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("fn"); LXValue _v = px_str("fn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("struct"); LXValue _v = px_str("struct"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("enum"); LXValue _v = px_str("enum"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("trait"); LXValue _v = px_str("trait"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("impl"); LXValue _v = px_str("impl"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("match"); LXValue _v = px_str("match"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("case"); LXValue _v = px_str("case"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("if"); LXValue _v = px_str("if"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("elif"); LXValue _v = px_str("elif"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("else"); LXValue _v = px_str("else"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("for"); LXValue _v = px_str("for"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("while"); LXValue _v = px_str("while"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("in"); LXValue _v = px_str("in"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("and"); LXValue _v = px_str("and"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("or"); LXValue _v = px_str("or"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("not"); LXValue _v = px_str("not"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("return"); LXValue _v = px_str("return"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("break"); LXValue _v = px_str("break"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("continue"); LXValue _v = px_str("continue"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("import"); LXValue _v = px_str("import"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("from"); LXValue _v = px_str("from"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pub"); LXValue _v = px_str("pub"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("as"); LXValue _v = px_str("as"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("spawn"); LXValue _v = px_str("spawn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("chan"); LXValue _v = px_str("chan"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("send"); LXValue _v = px_str("send"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("recv"); LXValue _v = px_str("recv"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("select"); LXValue _v = px_str("select"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("true"); LXValue _v = px_str("true"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("false"); LXValue _v = px_str("false"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("null"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("None"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("self"); LXValue _v = px_str("self"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("capture"); LXValue _v = px_str("capture"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("extern"); LXValue _v = px_str("extern"); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(22);
    px_set_global("CTRL_ALL", px_str(""));
    px_srcline(23);
    px_set_global("LAYOUT", ({ LXValue _d = px_dict(); { LXValue _k = px_str("Program"); LXValue _v = px_list_n((LXValue[]){px_str("Program"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("items"), px_str("l")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("VarDecl"); LXValue _v = px_list_n((LXValue[]){px_str("VarDecl"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("kind"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Assign"); LXValue _v = px_list_n((LXValue[]){px_str("Assign"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("target"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("op"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ExprStmt"); LXValue _v = px_list_n((LXValue[]){px_str("ExprStmt"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("If"); LXValue _v = px_list_n((LXValue[]){px_str("If"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("branches"), px_str("lt2b")}, 2), px_list_n((LXValue[]){px_str("else_branch"), px_str("ol")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("For"); LXValue _v = px_list_n((LXValue[]){px_str("For"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("var"), px_str("vs")}, 2), px_list_n((LXValue[]){px_str("iterable"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("While"); LXValue _v = px_list_n((LXValue[]){px_str("While"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("cond"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Return"); LXValue _v = px_list_n((LXValue[]){px_str("Return"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Break"); LXValue _v = px_list_n((LXValue[]){px_str("Break"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Continue"); LXValue _v = px_list_n((LXValue[]){px_str("Continue"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("FuncDef"); LXValue _v = px_list_n((LXValue[]){px_str("FuncDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("params"), px_str("lp")}, 2), px_list_n((LXValue[]){px_str("ret_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2), px_list_n((LXValue[]){px_str("type_params"), px_str("ls")}, 2)}, 6)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("StructDef"); LXValue _v = px_list_n((LXValue[]){px_str("StructDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("fields"), px_str("lsf")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2), px_list_n((LXValue[]){px_str("type_params"), px_str("ls")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("EnumDef"); LXValue _v = px_list_n((LXValue[]){px_str("EnumDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("variants"), px_str("lev")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TypeConst"); LXValue _v = px_list_n((LXValue[]){px_str("TypeConst"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("items"), px_str("ltci")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TraitDef"); LXValue _v = px_list_n((LXValue[]){px_str("TraitDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("methods"), px_str("lfd")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ImplDef"); LXValue _v = px_list_n((LXValue[]){px_str("ImplDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("type_name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("trait_name"), px_str("os")}, 2), px_list_n((LXValue[]){px_str("methods"), px_str("lfd")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Import"); LXValue _v = px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("module"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("names"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ExternDef"); LXValue _v = px_list_n((LXValue[]){px_str("ExternDef"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("params"), px_str("lp")}, 2), px_list_n((LXValue[]){px_str("ret_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Spawn"); LXValue _v = px_list_n((LXValue[]){px_str("Spawn"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ChanDecl"); LXValue _v = px_list_n((LXValue[]){px_str("ChanDecl"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("elem_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Send"); LXValue _v = px_list_n((LXValue[]){px_str("Send"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("chan"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Recv"); LXValue _v = px_list_n((LXValue[]){px_str("Recv"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("chan"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Select"); LXValue _v = px_list_n((LXValue[]){px_str("Select"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("arms"), px_str("lt3")}, 2), px_list_n((LXValue[]){px_str("else_branch"), px_str("ol")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Empty"); LXValue _v = px_list_n((LXValue[]){px_str("Empty"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Int"); LXValue _v = px_list_n((LXValue[]){px_str("Int"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Float"); LXValue _v = px_list_n((LXValue[]){px_str("Float"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("f")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Str"); LXValue _v = px_list_n((LXValue[]){px_str("Str"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Bool"); LXValue _v = px_list_n((LXValue[]){px_str("Bool"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Null"); LXValue _v = px_list_n((LXValue[]){px_str("Null"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("List"); LXValue _v = px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("items"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Tuple"); LXValue _v = px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("items"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Dict"); LXValue _v = px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("entries"), px_str("lt2")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Var"); LXValue _v = px_list_n((LXValue[]){px_str("Var"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Field"); LXValue _v = px_list_n((LXValue[]){px_str("Field"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("OptionalField"); LXValue _v = px_list_n((LXValue[]){px_str("OptionalField"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Index"); LXValue _v = px_list_n((LXValue[]){px_str("Index"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("index"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Slice"); LXValue _v = px_list_n((LXValue[]){px_str("Slice"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("obj"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("start"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("end"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("step"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Call"); LXValue _v = px_list_n((LXValue[]){px_str("Call"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("callee"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("args"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Unary"); LXValue _v = px_list_n((LXValue[]){px_str("Unary"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("op"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("operand"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Binary"); LXValue _v = px_list_n((LXValue[]){px_str("Binary"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("op"), px_str("r")}, 2), px_list_n((LXValue[]){px_str("left"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("right"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Pipe"); LXValue _v = px_list_n((LXValue[]){px_str("Pipe"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("func"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("NullCoalesce"); LXValue _v = px_list_n((LXValue[]){px_str("NullCoalesce"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("left"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("right"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Try"); LXValue _v = px_list_n((LXValue[]){px_str("Try"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ForceUnwrap"); LXValue _v = px_list_n((LXValue[]){px_str("ForceUnwrap"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("IfExpr"); LXValue _v = px_list_n((LXValue[]){px_str("IfExpr"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("cond"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("then"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("else_"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ListComp"); LXValue _v = px_list_n((LXValue[]){px_str("ListComp"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("clauses"), px_str("lc")}, 2), px_list_n((LXValue[]){px_str("cond"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("DictComp"); LXValue _v = px_list_n((LXValue[]){px_str("DictComp"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("key"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("clauses"), px_str("lc")}, 2), px_list_n((LXValue[]){px_str("cond"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("GenExp"); LXValue _v = px_list_n((LXValue[]){px_str("GenExp"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("expr"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("clauses"), px_str("lc")}, 2), px_list_n((LXValue[]){px_str("cond"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Closure"); LXValue _v = px_list_n((LXValue[]){px_str("Closure"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("params"), px_str("lp")}, 2), px_list_n((LXValue[]){px_str("ret_ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("captures"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 5)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Block"); LXValue _v = px_list_n((LXValue[]){px_str("Block"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("stmts"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Match"); LXValue _v = px_list_n((LXValue[]){px_str("Match"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("subject"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("arms"), px_str("lma")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Constructor"); LXValue _v = px_list_n((LXValue[]){px_str("Constructor"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("args"), px_str("l")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("Param"); LXValue _v = px_list_n((LXValue[]){px_str("Param"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("ty"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("default"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("StructField"); LXValue _v = px_list_n((LXValue[]){px_str("StructField"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("ty"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("EnumVariant"); LXValue _v = px_list_n((LXValue[]){px_str("EnumVariant"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("fields"), px_str("tl")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TypeConstItem"); LXValue _v = px_list_n((LXValue[]){px_str("TypeConstItem"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("name"), px_str("s")}, 2), px_list_n((LXValue[]){px_str("value"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("MatchArm"); LXValue _v = px_list_n((LXValue[]){px_str("MatchArm"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("pattern"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("guard"), px_str("o")}, 2), px_list_n((LXValue[]){px_str("body"), px_str("n")}, 2), px_list_n((LXValue[]){px_str("pos"), px_str("p")}, 2)}, 4)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("CompClause"); LXValue _v = px_list_n((LXValue[]){px_str("CompClause"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("vars"), px_str("ls")}, 2), px_list_n((LXValue[]){px_str("iterable"), px_str("n")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("PatLiteral"); LXValue _v = px_list_n((LXValue[]){px_str("Literal"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("PatBinding"); LXValue _v = px_list_n((LXValue[]){px_str("Binding"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("PatWildcard"); LXValue _v = px_list_n((LXValue[]){px_str("Wildcard"), px_list_n((LXValue[]){}, 0)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("PatTuple"); LXValue _v = px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("lpl")}, 2)}, 1)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("PatConstructor"); LXValue _v = px_list_n((LXValue[]){px_str("Constructor"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2), px_list_n((LXValue[]){px_null(), px_str("lpl")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyNamed"); LXValue _v = px_list_n((LXValue[]){px_str("Named"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyOptional"); LXValue _v = px_list_n((LXValue[]){px_str("Optional"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyList"); LXValue _v = px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyDict"); LXValue _v = px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyTuple"); LXValue _v = px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("tl")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 2)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyFunc"); LXValue _v = px_list_n((LXValue[]){px_str("Func"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("tl")}, 2), px_list_n((LXValue[]){px_null(), px_str("n")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("TyGeneric"); LXValue _v = px_list_n((LXValue[]){px_str("Generic"), px_list_n((LXValue[]){px_list_n((LXValue[]){px_null(), px_str("s")}, 2), px_list_n((LXValue[]){px_null(), px_str("tl")}, 2), px_list_n((LXValue[]){px_null(), px_str("p")}, 2)}, 3)}, 2); px_dict_set_checked(_d, _k, _v); } _d; }));
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
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); LXValue _v = px_str("let"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("var"); LXValue _v = px_str("var"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("const"); LXValue _v = px_str("const"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("def"); LXValue _v = px_str("def"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("fn"); LXValue _v = px_str("fn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("struct"); LXValue _v = px_str("struct"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("enum"); LXValue _v = px_str("enum"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("trait"); LXValue _v = px_str("trait"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("impl"); LXValue _v = px_str("impl"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("match"); LXValue _v = px_str("match"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("case"); LXValue _v = px_str("case"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("if"); LXValue _v = px_str("if"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("elif"); LXValue _v = px_str("elif"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("else"); LXValue _v = px_str("else"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("for"); LXValue _v = px_str("for"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("while"); LXValue _v = px_str("while"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("in"); LXValue _v = px_str("in"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("and"); LXValue _v = px_str("and"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("or"); LXValue _v = px_str("or"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("not"); LXValue _v = px_str("not"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("return"); LXValue _v = px_str("return"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("break"); LXValue _v = px_str("break"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("continue"); LXValue _v = px_str("continue"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("import"); LXValue _v = px_str("import"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("from"); LXValue _v = px_str("from"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pub"); LXValue _v = px_str("pub"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("as"); LXValue _v = px_str("as"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("spawn"); LXValue _v = px_str("spawn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("chan"); LXValue _v = px_str("chan"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("send"); LXValue _v = px_str("send"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("recv"); LXValue _v = px_str("recv"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("select"); LXValue _v = px_str("select"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("true"); LXValue _v = px_str("true"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("false"); LXValue _v = px_str("false"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("null"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("None"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("self"); LXValue _v = px_str("self"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("capture"); LXValue _v = px_str("capture"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("extern"); LXValue _v = px_str("extern"); px_dict_set_checked(_d, _k, _v); } _d; }));
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
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(39);
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(40);
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(41);
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(42);
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(44);
    px_set_global("cg_cells", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(45);
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(46);
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(47);
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(48);
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(49);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(64);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(68);
    px_set_global("cg_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(69);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(70);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(71);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(72);
    px_set_global("cg_seq_uid", px_int(0LL));
    px_srcline(73);
    px_set_global("cg_iter_uid", px_int(0LL));
    px_srcline(74);
    px_set_global("cg_unpack_uid", px_int(0LL));
    px_srcline(75);
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(34);
    px_set_global("g_bcm", px_null());
    px_srcline(38);
    px_set_global("g_topnames", px_list_n((LXValue[]){}, 0));
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
    px_set_global("KEYWORDS", ({ LXValue _d = px_dict(); { LXValue _k = px_str("let"); LXValue _v = px_str("let"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("var"); LXValue _v = px_str("var"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("const"); LXValue _v = px_str("const"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("def"); LXValue _v = px_str("def"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("fn"); LXValue _v = px_str("fn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("struct"); LXValue _v = px_str("struct"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("enum"); LXValue _v = px_str("enum"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("trait"); LXValue _v = px_str("trait"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("impl"); LXValue _v = px_str("impl"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("match"); LXValue _v = px_str("match"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("case"); LXValue _v = px_str("case"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("if"); LXValue _v = px_str("if"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("elif"); LXValue _v = px_str("elif"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("else"); LXValue _v = px_str("else"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("for"); LXValue _v = px_str("for"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("while"); LXValue _v = px_str("while"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("in"); LXValue _v = px_str("in"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("and"); LXValue _v = px_str("and"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("or"); LXValue _v = px_str("or"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("not"); LXValue _v = px_str("not"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("return"); LXValue _v = px_str("return"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("break"); LXValue _v = px_str("break"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("continue"); LXValue _v = px_str("continue"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("import"); LXValue _v = px_str("import"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("from"); LXValue _v = px_str("from"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pub"); LXValue _v = px_str("pub"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("as"); LXValue _v = px_str("as"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("spawn"); LXValue _v = px_str("spawn"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("chan"); LXValue _v = px_str("chan"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("send"); LXValue _v = px_str("send"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("recv"); LXValue _v = px_str("recv"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("select"); LXValue _v = px_str("select"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("true"); LXValue _v = px_str("true"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("false"); LXValue _v = px_str("false"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("null"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("None"); LXValue _v = px_str("null"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("self"); LXValue _v = px_str("self"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("capture"); LXValue _v = px_str("capture"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("extern"); LXValue _v = px_str("extern"); px_dict_set_checked(_d, _k, _v); } _d; }));
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
    px_set_global("cg_structs", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(42);
    px_set_global("cg_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(43);
    px_set_global("cg_impls", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(44);
    px_set_global("cg_vars", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(46);
    px_set_global("cg_cells", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(47);
    px_set_global("cg_var_types", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(48);
    px_set_global("cg_immutables", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(49);
    px_set_global("cg_nonnull", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(50);
    px_set_global("cg_ffi", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(51);
    px_set_global("cg_const_enums", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(52);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(56);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(57);
    px_set_global("cg_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(58);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(59);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(60);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(63);
    px_set_global("cg_seq_uid", px_int(0LL));
    px_srcline(65);
    px_set_global("cg_iter_uid", px_int(0LL));
    px_srcline(66);
    px_set_global("loaded", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(68);
    px_set_global("g_bcm", px_null());
    px_srcline(70);
    px_set_global("PXC_VER", px_str("0.2.0"));
    px_srcline(71);
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
