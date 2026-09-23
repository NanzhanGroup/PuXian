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
    LXValue _v6 = px_uninit();
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
    LXValue _v58 = px_uninit();
    LXValue _v59 = px_uninit();
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
    LXValue _v61 = px_uninit();
    LXValue _v62 = px_uninit();
    LXValue _v63 = px_uninit();
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
    LXValue _v65 = px_uninit();
    LXValue _v66 = px_uninit();
    LXValue _v67 = px_uninit();
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
    LXValue _v72 = px_uninit();
    LXValue _v73 = px_uninit();
    LXValue _v74 = px_uninit();
    LXValue _v75 = px_uninit();
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
    LXValue _v80 = px_uninit();
    LXValue _v81 = px_uninit();
    LXValue _v82 = px_uninit();
    LXValue _v83 = px_uninit();
    LXValue _v84 = px_uninit();
    LXValue _v85 = px_uninit();
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
    LXValue _v90 = px_uninit();
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
    LXValue _v93 = px_uninit();
    LXValue _v94 = px_uninit();
    LXValue _v95 = px_uninit();
    LXValue _v96 = px_uninit();
    LXValue _v97 = px_uninit();
    LXValue _v98 = px_uninit();
    LXValue _v99 = px_uninit();
    LXValue _v100 = px_uninit();
    LXValue _v101 = px_uninit();
    LXValue _v102 = px_uninit();
    LXValue _v103 = px_uninit();
    LXValue _v104 = px_uninit();
    LXValue _v105 = px_uninit();
    LXValue _v106 = px_uninit();
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
    LXValue _v120 = px_uninit();
    LXValue _v121 = px_uninit();
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
    LXValue _v124 = px_uninit();
    LXValue _v125 = px_uninit();
    LXValue _v126 = px_uninit();
    LXValue _v127 = px_uninit();
    LXValue _v128 = px_uninit();
    LXValue _v129 = px_uninit();
    LXValue _v130 = px_uninit();
    LXValue _v131 = px_uninit();
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
    LXValue _v143 = px_uninit();
    LXValue _v144 = px_uninit();
    LXValue _v145 = px_uninit();
    LXValue _v146 = px_uninit();
    LXValue _v147 = px_uninit();
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
    LXValue _v155 = px_uninit();
    LXValue _v156 = px_uninit();
    LXValue _v157 = px_uninit();
    LXValue _v158 = px_uninit();
    LXValue _v159 = px_uninit();
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
    LXValue _v164 = px_uninit();
    LXValue _v165 = px_uninit();
    LXValue _v166 = px_uninit();
    LXValue _v167 = px_uninit();
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
    LXValue _v175 = px_uninit();
    LXValue _v176 = px_uninit();
    LXValue _v177 = px_uninit();
    LXValue _v178 = px_uninit();
    LXValue _v179 = px_uninit();
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
    LXValue _v183 = px_uninit();
    LXValue _v184 = px_uninit();
    LXValue _v185 = px_uninit();
    LXValue _v186 = px_uninit();
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
    LXValue _v193 = px_uninit();
    LXValue _v194 = px_uninit();
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
    LXValue _v197 = px_uninit();
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
    LXValue _v200 = px_uninit();
    LXValue _v201 = px_uninit();
    LXValue _v202 = px_uninit();
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
    LXValue _v206 = px_uninit();
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
    LXValue _v209 = px_uninit();
    LXValue _v210 = px_uninit();
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
    LXValue _v213 = px_uninit();
    LXValue _v214 = px_uninit();
    LXValue _v215 = px_uninit();
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
    LXValue _v217 = px_uninit();
    LXValue _v218 = px_uninit();
    LXValue _v219 = px_uninit();
    LXValue _v220 = px_uninit();
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
    LXValue _v226 = px_uninit();
    LXValue _v227 = px_uninit();
    LXValue _v228 = px_uninit();
    LXValue _v229 = px_uninit();
    LXValue _v230 = px_uninit();
    LXValue _v231 = px_uninit();
    LXValue _v232 = px_uninit();
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
    LXValue _v242 = px_uninit();
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
    LXValue _v245 = px_uninit();
    LXValue _v246 = px_uninit();
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
    LXValue _v250 = px_uninit();
    LXValue _v251 = px_uninit();
    LXValue _v252 = px_uninit();
    LXValue _v253 = px_uninit();
    LXValue _v254 = px_uninit();
    LXValue _v255 = px_uninit();
    LXValue _v256 = px_uninit();
    LXValue _v257 = px_uninit();
    LXValue _v258 = px_uninit();
    LXValue _v259 = px_uninit();
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
    LXValue _v263 = px_uninit();
    LXValue _v264 = px_uninit();
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
    LXValue _v268 = px_uninit();
    LXValue _v269 = px_uninit();
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
    LXValue _v273 = px_uninit();
    LXValue _v274 = px_uninit();
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
    LXValue _v278 = px_uninit();
    LXValue _v279 = px_uninit();
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
    LXValue _v295 = px_uninit();
    LXValue _v296 = px_uninit();
    LXValue _v297 = px_uninit();
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
    LXValue _v301 = px_uninit();
    LXValue _v302 = px_uninit();
    LXValue _v303 = px_uninit();
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
    LXValue _v307 = px_uninit();
    LXValue _v308 = px_uninit();
    LXValue _v309 = px_uninit();
    LXValue _v310 = px_uninit();
    LXValue _v311 = px_uninit();
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
    LXValue _v314 = px_uninit();
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
    LXValue _v326 = px_uninit();
    LXValue _v327 = px_uninit();
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
    LXValue _v333 = px_uninit();
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
    LXValue _v349 = px_uninit();
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
    LXValue _v352 = px_uninit();
    LXValue _v353 = px_uninit();
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
    LXValue _v356 = px_uninit();
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
    LXValue _v369 = px_uninit();
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
    LXValue _v379 = px_uninit();
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
    LXValue _v381 = px_uninit();
    LXValue _v382 = px_uninit();
    LXValue _v383 = px_uninit();
    LXValue _v384 = px_uninit();
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
    LXValue _v391 = px_uninit();
    LXValue _v392 = px_uninit();
    LXValue _v393 = px_uninit();
    LXValue _v394 = px_uninit();
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
    LXValue _v398 = px_uninit();
    LXValue _v399 = px_uninit();
    LXValue _v400 = px_uninit();
    LXValue _v401 = px_uninit();
    LXValue _v402 = px_uninit();
    LXValue _v403 = px_uninit();
    LXValue _v404 = px_uninit();
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
    LXValue _v406 = px_uninit();
    LXValue _v407 = px_uninit();
    LXValue _v408 = px_uninit();
    LXValue _v409 = px_uninit();
    LXValue _v410 = px_uninit();
    LXValue _v411 = px_uninit();
    LXValue _v412 = px_uninit();
    LXValue _v413 = px_uninit();
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
    LXValue _v415 = px_uninit();
    LXValue _v416 = px_uninit();
    LXValue _v417 = px_uninit();
    LXValue _v418 = px_uninit();
    LXValue _v419 = px_uninit();
    LXValue _v420 = px_uninit();
    LXValue _v421 = px_uninit();
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
    LXValue _v423 = px_uninit();
    LXValue _v424 = px_uninit();
    LXValue _v425 = px_uninit();
    LXValue _v426 = px_uninit();
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
    LXValue _v429 = px_uninit();
    LXValue _v430 = px_uninit();
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
    LXValue _v437 = px_uninit();
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
    LXValue _v440 = px_uninit();
    LXValue _v441 = px_uninit();
    LXValue _v442 = px_uninit();
    LXValue _v443 = px_uninit();
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
    LXValue _v445 = px_uninit();
    LXValue _v446 = px_uninit();
    LXValue _v447 = px_uninit();
    LXValue _v448 = px_uninit();
    LXValue _v449 = px_uninit();
    LXValue _v450 = px_uninit();
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
    LXValue _v452 = px_uninit();
    LXValue _v453 = px_uninit();
    LXValue _v454 = px_uninit();
    LXValue _v455 = px_uninit();
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
    LXValue _v457 = px_uninit();
    LXValue _v458 = px_uninit();
    LXValue _v459 = px_uninit();
    LXValue _v460 = px_uninit();
    LXValue _v461 = px_uninit();
    LXValue _v462 = px_uninit();
    LXValue _v463 = px_uninit();
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
    LXValue _v467 = px_uninit();
    LXValue _v468 = px_uninit();
    LXValue _v469 = px_uninit();
    LXValue _v470 = px_uninit();
    LXValue _v471 = px_uninit();
    LXValue _v472 = px_uninit();
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
    LXValue _v476 = px_uninit();
    LXValue _v477 = px_uninit();
    LXValue _v478 = px_uninit();
    LXValue _v479 = px_uninit();
    LXValue _v480 = px_uninit();
    LXValue _v481 = px_uninit();
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
    LXValue _v483 = px_uninit();
    LXValue _v484 = px_uninit();
    LXValue _v485 = px_uninit();
    LXValue _v486 = px_uninit();
    LXValue _v487 = px_uninit();
    LXValue _v488 = px_uninit();
    LXValue _v489 = px_uninit();
    LXValue _v490 = px_uninit();
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
    LXValue _v494 = px_uninit();
    LXValue _v495 = px_uninit();
    LXValue _v496 = px_uninit();
    LXValue _v497 = px_uninit();
    LXValue _v498 = px_uninit();
    LXValue _v499 = px_uninit();
    LXValue _v500 = px_uninit();
    LXValue _v501 = px_uninit();
    LXValue _v502 = px_uninit();
    LXValue _v503 = px_uninit();
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
    LXValue _v506 = px_uninit();
    LXValue _v507 = px_uninit();
    LXValue _v508 = px_uninit();
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
    LXValue _v510 = px_uninit();
    LXValue _v511 = px_uninit();
    LXValue _v512 = px_uninit();
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
    LXValue _v514 = px_uninit();
    LXValue _v515 = px_uninit();
    LXValue _v516 = px_uninit();
    LXValue _v517 = px_uninit();
    LXValue _v518 = px_uninit();
    LXValue _v519 = px_uninit();
    LXValue _v520 = px_uninit();
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
    LXValue _v525 = px_uninit();
    LXValue _v526 = px_uninit();
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
    LXValue _v528 = px_uninit();
    LXValue _v529 = px_uninit();
    LXValue _v530 = px_uninit();
    LXValue _v531 = px_uninit();
    LXValue _v532 = px_uninit();
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
    LXValue _v535 = px_uninit();
    LXValue _v536 = px_uninit();
    LXValue _v537 = px_uninit();
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
    LXValue _v539 = px_uninit();
    LXValue _v540 = px_uninit();
    LXValue _v541 = px_uninit();
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
    LXValue _v543 = px_uninit();
    LXValue _v544 = px_uninit();
    LXValue _v545 = px_uninit();
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
    LXValue _v547 = px_uninit();
    LXValue _v548 = px_uninit();
    LXValue _v549 = px_uninit();
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
    LXValue _v551 = px_uninit();
    LXValue _v552 = px_uninit();
    LXValue _v553 = px_uninit();
    LXValue _v554 = px_uninit();
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
    LXValue _v556 = px_uninit();
    LXValue _v557 = px_uninit();
    LXValue _v558 = px_uninit();
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
    LXValue _v560 = px_uninit();
    LXValue _v561 = px_uninit();
    LXValue _v562 = px_uninit();
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
    LXValue _v564 = px_uninit();
    LXValue _v565 = px_uninit();
    LXValue _v566 = px_uninit();
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
    LXValue _v568 = px_uninit();
    LXValue _v569 = px_uninit();
    LXValue _v570 = px_uninit();
    LXValue _v571 = px_uninit();
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
    LXValue _v573 = px_uninit();
    LXValue _v574 = px_uninit();
    LXValue _v575 = px_uninit();
    LXValue _v576 = px_uninit();
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
    LXValue _v578 = px_uninit();
    LXValue _v579 = px_uninit();
    LXValue _v580 = px_uninit();
    LXValue _v581 = px_uninit();
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
    LXValue _v583 = px_uninit();
    LXValue _v584 = px_uninit();
    LXValue _v585 = px_uninit();
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
    LXValue _v587 = px_uninit();
    LXValue _v588 = px_uninit();
    LXValue _v589 = px_uninit();
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
    LXValue _v591 = px_uninit();
    LXValue _v592 = px_uninit();
    LXValue _v593 = px_uninit();
    LXValue _v594 = px_uninit();
    LXValue _v595 = px_uninit();
    LXValue _v596 = px_uninit();
    LXValue _v597 = px_uninit();
    LXValue _v598 = px_uninit();
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
    LXValue _v602 = px_uninit();
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
    LXValue _v604 = px_uninit();
    LXValue _v605 = px_uninit();
    LXValue _v606 = px_uninit();
    LXValue _v607 = px_uninit();
    LXValue _v608 = px_uninit();
    LXValue _v609 = px_uninit();
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
    LXValue _v611 = px_uninit();
    LXValue _v612 = px_uninit();
    LXValue _v613 = px_uninit();
    LXValue _v614 = px_uninit();
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
    LXValue _v616 = px_uninit();
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
    LXValue _v618 = px_uninit();
    LXValue _v619 = px_uninit();
    LXValue _v620 = px_uninit();
    LXValue _v621 = px_uninit();
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
    LXValue _v624 = px_uninit();
    LXValue _v625 = px_uninit();
    LXValue _v626 = px_uninit();
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
    LXValue _v628 = px_uninit();
    LXValue _v629 = px_uninit();
    LXValue _v630 = px_uninit();
    LXValue _v631 = px_uninit();
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
    LXValue _v633 = px_uninit();
    LXValue _v634 = px_uninit();
    LXValue _v635 = px_uninit();
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
    LXValue _v645 = px_uninit();
    LXValue _v646 = px_uninit();
    LXValue _v647 = px_uninit();
    LXValue _v648 = px_uninit();
    LXValue _v649 = px_uninit();
    LXValue _v650 = px_uninit();
    LXValue _v651 = px_uninit();
    LXValue _v652 = px_uninit();
    LXValue _v653 = px_uninit();
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
    LXValue _v656 = px_uninit();
    LXValue _v657 = px_uninit();
    LXValue _v658 = px_uninit();
    LXValue _v659 = px_uninit();
    LXValue _v660 = px_uninit();
    LXValue _v661 = px_uninit();
    LXValue _v662 = px_uninit();
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
    LXValue _v670 = px_uninit();
    LXValue _v671 = px_uninit();
    LXValue _v672 = px_uninit();
    LXValue _v673 = px_uninit();
    LXValue _v674 = px_uninit();
    LXValue _v675 = px_uninit();
    LXValue _v676 = px_uninit();
    LXValue _v677 = px_uninit();
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
    LXValue _v680 = px_uninit();
    LXValue _v681 = px_uninit();
    LXValue _v682 = px_uninit();
    LXValue _v683 = px_uninit();
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
    LXValue _v685 = px_uninit();
    LXValue _v686 = px_uninit();
    LXValue _v687 = px_uninit();
    LXValue _v688 = px_uninit();
    LXValue _v689 = px_uninit();
    LXValue _v690 = px_uninit();
    LXValue _v691 = px_uninit();
    LXValue _v692 = px_uninit();
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
    LXValue _v700 = px_uninit();
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
    LXValue _v703 = px_uninit();
    LXValue _v704 = px_uninit();
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
    LXValue _v706 = px_uninit();
    LXValue _v707 = px_uninit();
    LXValue _v708 = px_uninit();
    LXValue _v709 = px_uninit();
    LXValue _v710 = px_uninit();
    LXValue _v711 = px_uninit();
    LXValue _v712 = px_uninit();
    LXValue _v713 = px_uninit();
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
    LXValue _v717 = px_uninit();
    LXValue _v718 = px_uninit();
    LXValue _v719 = px_uninit();
    LXValue _v720 = px_uninit();
    LXValue _v721 = px_uninit();
    LXValue _v722 = px_uninit();
    LXValue _v723 = px_uninit();
    LXValue _v724 = px_uninit();
    LXValue _v725 = px_uninit();
    LXValue _v726 = px_uninit();
    LXValue _v727 = px_uninit();
    LXValue _v728 = px_uninit();
    LXValue _v729 = px_uninit();
    LXValue _v730 = px_uninit();
    LXValue _v731 = px_uninit();
    LXValue _v732 = px_uninit();
    LXValue _v733 = px_uninit();
    LXValue _v734 = px_uninit();
    LXValue _v735 = px_uninit();
    LXValue _v736 = px_uninit();
    LXValue _v737 = px_uninit();
    LXValue _v738 = px_uninit();
    LXValue _v739 = px_uninit();
    LXValue _v740 = px_uninit();
    LXValue _v741 = px_uninit();
    LXValue _v742 = px_uninit();
    LXValue _v743 = px_uninit();
    LXValue _v744 = px_uninit();
    LXValue _v745 = px_uninit();
    LXValue _v746 = px_uninit();
    LXValue _v747 = px_uninit();
    LXValue _v748 = px_uninit();
    LXValue _v749 = px_uninit();
    LXValue _v750 = px_uninit();
    LXValue _v751 = px_uninit();
    LXValue _v752 = px_uninit();
    LXValue _v753 = px_uninit();
    LXValue _v754 = px_uninit();
    LXValue _v755 = px_uninit();
    LXValue _v756 = px_uninit();
    LXValue _v757 = px_uninit();
    LXValue _v758 = px_uninit();
    LXValue _v759 = px_uninit();
    LXValue _v760 = px_uninit();
    LXValue _v761 = px_uninit();
    LXValue _v762 = px_uninit();
    LXValue _v763 = px_uninit();
    LXValue _v764 = px_uninit();
    LXValue _v765 = px_uninit();
    LXValue _v766 = px_uninit();
    LXValue _v767 = px_uninit();
    LXValue _v768 = px_uninit();
    LXValue _v769 = px_uninit();
    LXValue _v770 = px_uninit();
    LXValue _v771 = px_uninit();
    LXValue _v772 = px_uninit();
    LXValue _v773 = px_uninit();
    LXValue _v774 = px_uninit();
    LXValue _v775 = px_uninit();
    LXValue _v776 = px_uninit();
    LXValue px_err_777_val = px_null();
    int px_err_777_proped = 0;
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
                (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v719}, 1));
                px_srcline(47);
                return px_add(px_add(px_add(px_add(px_add(_v717, px_str("LXValue ")), _v721), px_str(" = px_cell(")), _v720), px_str(");\n"));
            }
            px_srcline(48);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v719}, 1));
            px_srcline(49);
            return px_add(px_add(px_add(px_add(px_add(_v717, px_str("LXValue ")), _v721), px_str(" = ")), _v720), px_str(";\n"));
        }
        px_srcline(52);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(4LL)), px_null()))) {
            px_srcline(53);
            _v722 = px_index(_v715, px_int(4LL));
            px_srcline(54);
            _v723 = px_null();
            px_srcline(55);
            if (px_is_truthy(px_eq(px_index(_v722, px_int(0LL)), px_str("Constructor")))) {
                px_srcline(56);
                 _v723 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v722, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v722, px_int(0LL)), px_str("Call")))) {
                px_srcline(58);
                _v724 = px_index(_v722, px_int(1LL));
                px_srcline(59);
                if (px_is_truthy(px_eq(px_index(_v724, px_int(0LL)), px_str("Var")))) {
                    px_srcline(60);
                     _v723 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v724, px_int(1LL))}, 1);
                }
            }
            px_srcline(61);
            if (px_is_truthy(px_ne(_v723, px_null()))) {
                px_srcline(62);
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v723}, 1))) {
                    px_srcline(63);
                    px_index_set(px_get_global("cg_var_types"), _v719, _v723);
                }
            }
        }
        px_srcline(64);
        _v725 = px_add(px_add(_v717, px_call(px_get_global("cg_store_of"), (LXValue[]){_v719, _v721, _v720}, 3)), px_str(";\n"));
        px_srcline(66);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v719}, 1));
        px_srcline(67);
        return _v725;
    }
    px_srcline(68);
    if (px_is_truthy(px_eq(_v718, px_str("Assign")))) {
        px_srcline(69);
        _v720 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(3LL))}, 1);
        px_srcline(70);
        _v726 = px_index(_v715, px_int(1LL));
        px_srcline(71);
        _v727 = px_index(_v715, px_int(2LL));
        px_srcline(73);
        if (px_is_truthy(px_eq(_v727, px_str("Append")))) {
            px_srcline(74);
            _v728 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v726}, 1);
            px_srcline(75);
            _v729 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){_v726, px_index(_v715, px_int(3LL))}, 2), px_list_n((LXValue[]){_v728, _v720}, 2)}, 2);
            px_srcline(76);
            _v730 = px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v729, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_method("), px_index(px_index(_v729, px_int(1LL)), px_int(0LL))), px_str(", \"append\", (LXValue[]){")), px_index(px_index(_v729, px_int(1LL)), px_int(1LL))), px_str("}, 1)"))}, 2);
            px_srcline(77);
            return px_add(px_add(px_add(_v717, px_str("(void)(")), _v730), px_str(");\n"));
        }
        px_srcline(78);
        _v731 = px_index(_v726, px_int(0LL));
        px_srcline(79);
        if (px_is_truthy(px_eq(_v731, px_str("Var")))) {
            px_srcline(80);
            _v719 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v726, px_int(1LL))}, 1);
            px_srcline(82);
            (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v726, _v727, px_index(_v715, px_int(3LL))}, 3));
            px_srcline(83);
            _v732 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v719}, 1);
            px_srcline(84);
            if (px_is_truthy(px_eq(_v732, px_null()))) {
                px_srcline(86);
                if (px_is_truthy(px_eq(_v727, px_str("Assign")))) {
                    px_srcline(87);
                    return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v719), px_str("\", ")), _v720), px_str(");\n"));
                }
                px_srcline(88);
                _v733 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v727, _v719, _v720}, 3);
                px_srcline(89);
                return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v719), px_str("\", ")), _v733), px_str(");\n"));
            }
            px_srcline(90);
            _v733 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v727, px_call(px_get_global("cg_load_ck"), (LXValue[]){_v719, _v732}, 2), _v720}, 3);
            px_srcline(91);
            _v734 = px_add(px_add(px_add(_v717, px_str(" ")), px_call(px_get_global("cg_store_of"), (LXValue[]){_v719, _v732, _v733}, 3)), px_str(";\n"));
            px_srcline(93);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v719}, 1));
            px_srcline(94);
            return _v734;
        }
        px_srcline(95);
        if (px_is_truthy(px_eq(_v731, px_str("Field")))) {
            px_srcline(96);
            _v728 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v726, px_int(1LL))}, 1);
            px_srcline(97);
            _v735 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v726, px_int(2LL))}, 1);
            px_srcline(98);
            _v729 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v726, px_int(1LL)), px_index(_v715, px_int(3LL))}, 2), px_list_n((LXValue[]){_v728, _v720}, 2)}, 2);
            px_srcline(99);
            return px_add(px_add(_v717, px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v729, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_field_set("), px_index(px_index(_v729, px_int(1LL)), px_int(0LL))), px_str(", \"")), _v735), px_str("\", ")), px_index(px_index(_v729, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2)), px_str(";\n"));
        }
        px_srcline(100);
        if (px_is_truthy(px_eq(_v731, px_str("Index")))) {
            px_srcline(101);
            _v728 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v726, px_int(1LL))}, 1);
            px_srcline(102);
            _v736 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v726, px_int(2LL))}, 1);
            px_srcline(103);
            _v729 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v726, px_int(1LL)), px_index(_v726, px_int(2LL)), px_index(_v715, px_int(3LL))}, 3), px_list_n((LXValue[]){_v728, _v736, _v720}, 3)}, 2);
            px_srcline(104);
            return px_add(px_add(_v717, px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v729, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_index_set("), px_index(px_index(_v729, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v729, px_int(1LL)), px_int(1LL))), px_str(", ")), px_index(px_index(_v729, px_int(1LL)), px_int(2LL))), px_str(")"))}, 2)), px_str(";\n"));
        }
        px_srcline(105);
        return px_str("不支持的赋值目标");
    }
    px_srcline(106);
    if (px_is_truthy(px_eq(_v718, px_str("ExprStmt")))) {
        px_srcline(107);
        _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(108);
        return px_add(px_add(px_add(_v717, px_str("(void)(")), _v722), px_str(");\n"));
    }
    px_srcline(109);
    if (px_is_truthy(px_eq(_v718, px_str("If")))) {
        px_srcline(110);
        _v737 = px_str("");
        px_srcline(111);
        _v738 = px_index(_v715, px_int(1LL));
        px_srcline(114);
        _v739 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
        px_srcline(115);
        _v740 = px_null();
        px_srcline(116);
        _v741 = px_int(0LL);
        px_srcline(117);
        while (px_is_truthy(px_lt(_v741, px_call(px_get_global("len"), (LXValue[]){_v738}, 1)))) {
            px_srcline(118);
            (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v739}, 1));
            px_srcline(119);
            _v742 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v738, _v741), px_int(0LL))}, 1);
            px_srcline(120);
            _v743 = px_str("if");
            px_srcline(121);
            if (px_is_truthy(px_gt(_v741, px_int(0LL)))) {
                px_srcline(122);
                 _v743 = px_str("else if");
            }
            px_srcline(123);
             _v737 = px_add(_v737, px_add(px_add(px_add(px_add(_v717, _v743), px_str(" (px_is_truthy(")), _v742), px_str(")) {\n")));
            px_srcline(124);
            _v744 = px_index(px_index(_v738, _v741), px_int(1LL));
            px_srcline(125);
            _v745 = px_int(0LL);
            px_srcline(126);
            while (px_is_truthy(px_lt(_v745, px_call(px_get_global("len"), (LXValue[]){_v744}, 1)))) {
                px_srcline(127);
                 _v737 = px_add(_v737, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v744, _v745), px_add(_v716, px_int(1LL))}, 2));
                px_srcline(128);
                 _v745 = px_add(_v745, px_int(1LL));
            }
            px_srcline(129);
             _v737 = px_add(_v737, px_add(_v717, px_str("}\n")));
            px_srcline(130);
            if (px_is_truthy(px_eq(_v740, px_null()))) {
                px_srcline(131);
                 _v740 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
            }
            else {
                px_srcline(133);
                 _v740 = px_call(px_get_global("cg_inited_intersect"), (LXValue[]){_v740, px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0)}, 2);
            }
            px_srcline(134);
             _v741 = px_add(_v741, px_int(1LL));
        }
        px_srcline(135);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v739}, 1));
        px_srcline(136);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(2LL)), px_null()))) {
            px_srcline(137);
             _v737 = px_add(_v737, px_add(_v717, px_str("else {\n")));
            px_srcline(138);
            _v746 = px_index(_v715, px_int(2LL));
            px_srcline(139);
            _v747 = px_int(0LL);
            px_srcline(140);
            while (px_is_truthy(px_lt(_v747, px_call(px_get_global("len"), (LXValue[]){_v746}, 1)))) {
                px_srcline(141);
                 _v737 = px_add(_v737, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v746, _v747), px_add(_v716, px_int(1LL))}, 2));
                px_srcline(142);
                 _v747 = px_add(_v747, px_int(1LL));
            }
            px_srcline(143);
             _v737 = px_add(_v737, px_add(_v717, px_str("}\n")));
            px_srcline(144);
             _v740 = px_call(px_get_global("cg_inited_intersect"), (LXValue[]){_v740, px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0)}, 2);
        }
        else {
            px_srcline(146);
             _v740 = px_call(px_get_global("cg_inited_intersect"), (LXValue[]){_v740, _v739}, 2);
        }
        px_srcline(147);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v740}, 1));
        px_srcline(148);
        return _v737;
    }
    px_srcline(149);
    if (px_is_truthy(px_eq(_v718, px_str("While")))) {
        px_srcline(151);
        _v748 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
        px_srcline(152);
        _v742 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(153);
        _v737 = px_add(px_add(px_add(_v717, px_str("while (px_is_truthy(")), _v742), px_str(")) {\n"));
        px_srcline(154);
        _v744 = px_index(_v715, px_int(2LL));
        px_srcline(155);
        _v741 = px_int(0LL);
        px_srcline(156);
        while (px_is_truthy(px_lt(_v741, px_call(px_get_global("len"), (LXValue[]){_v744}, 1)))) {
            px_srcline(157);
             _v737 = px_add(_v737, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v744, _v741), px_add(_v716, px_int(1LL))}, 2));
            px_srcline(158);
             _v741 = px_add(_v741, px_int(1LL));
        }
        px_srcline(159);
         _v737 = px_add(_v737, px_add(_v717, px_str("}\n")));
        px_srcline(160);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v748}, 1));
        px_srcline(161);
        return _v737;
    }
    px_srcline(162);
    if (px_is_truthy(px_eq(_v718, px_str("For")))) {
        px_srcline(165);
        _v749 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
        px_srcline(166);
        _v750 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(2LL))}, 1);
        px_srcline(167);
        _v751 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(168);
        _v752 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(170);
        _v753 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(171);
        _v754 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v753}, 1), px_int(1LL));
        px_srcline(172);
        _v755 = px_str("");
        px_srcline(173);
        _v756 = px_int(0LL);
        px_srcline(174);
        if (px_is_truthy(_v754)) {
            px_srcline(175);
             _v755 = px_call(px_get_global("cg_unpack_tmp"), (LXValue[]){}, 0);
            px_srcline(176);
             _v756 = px_call(px_get_global("len"), (LXValue[]){_v753}, 1);
        }
        px_srcline(178);
        _v757 = px_list_n((LXValue[]){}, 0);
        px_srcline(184);
        _v758 = px_int(0LL);
        px_srcline(185);
        while (px_is_truthy(px_lt(_v758, px_call(px_get_global("len"), (LXValue[]){_v753}, 1)))) {
            px_srcline(186);
            _v759 = px_index(_v753, _v758);
            px_srcline(187);
            _v760 = px_str("");
            px_srcline(188);
            if (px_is_truthy(_v754)) {
                px_srcline(190);
                 _v760 = px_add(px_add(px_add(px_add(px_str("px_index("), _v755), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v758}, 1)), px_str("))"));
            }
            else {
                px_srcline(194);
                 _v760 = px_add(px_add(px_add(px_add(px_str("px_iter_at("), _v751), px_str(", px_int(")), _v752), px_str("))"));
            }
            px_srcline(195);
            if (px_is_truthy(px_get_global("cg_topbody"))) {
                px_srcline(197);
                (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
                px_srcline(198);
                (void)(px_method(_v757, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("px_set_global(\""), _v759), px_str("\", ")), _v760), px_str(");"))}, 1));
                px_srcline(199);
                 _v758 = px_add(_v758, px_int(1LL));
                px_srcline(200);
                continue;
            }
            px_srcline(201);
            _v761 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v759}, 1);
            px_srcline(202);
            _v762 = px_str("LXValue ");
            px_srcline(203);
            if (px_is_truthy(px_eq(_v761, px_null()))) {
                px_srcline(205);
                 _v761 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v759}, 1);
            }
            else {
                px_srcline(208);
                 _v762 = px_str("");
            }
            px_srcline(209);
            if (px_is_truthy(px_eq(_v762, px_str("LXValue ")))) {
                px_srcline(211);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v759}, 1))) {
                    px_srcline(212);
                    (void)(px_method(_v757, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("LXValue "), _v761), px_str(" = px_cell(")), _v760), px_str(");"))}, 1));
                }
                else {
                    px_srcline(214);
                    (void)(px_method(_v757, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("LXValue "), _v761), px_str(" = ")), _v760), px_str(";"))}, 1));
                }
            }
            else {
                px_srcline(217);
                (void)(px_method(_v757, "append", (LXValue[]){px_add(px_call(px_get_global("cg_store_of"), (LXValue[]){_v759, _v761, _v760}, 3), px_str(";"))}, 1));
            }
            px_srcline(218);
             _v758 = px_add(_v758, px_int(1LL));
        }
        px_srcline(219);
        _v737 = px_add(px_add(px_add(px_add(px_add(_v717, px_str("LXValue ")), _v751), px_str(" = ")), _v750), px_str(";\n"));
        px_srcline(222);
        _v763 = px_call(px_get_global("cg_iter_len_tmp"), (LXValue[]){}, 0);
        px_srcline(223);
         _v737 = px_add(_v737, px_add(px_add(px_add(px_add(px_add(_v717, px_str("int ")), _v763), px_str(" = (int)px_len(")), _v751), px_str(");\n")));
        px_srcline(224);
         _v737 = px_add(_v737, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v717, px_str("for (int ")), _v752), px_str(" = 0; ")), _v752), px_str(" < ")), _v763), px_str("; ")), _v752), px_str("++) {\n")));
        px_srcline(225);
         _v737 = px_add(_v737, px_add(px_add(px_add(px_add(px_add(_v717, px_str("    px_iter_ck(")), _v751), px_str(", ")), _v763), px_str(");\n")));
        px_srcline(226);
        if (px_is_truthy(_v754)) {
            px_srcline(231);
             _v737 = px_add(_v737, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v717, px_str("    LXValue ")), _v755), px_str(" = px_iter_at(")), _v751), px_str(", px_int(")), _v752), px_str("));\n")));
            px_srcline(232);
             _v737 = px_add(_v737, px_add(px_add(px_add(px_add(px_add(_v717, px_str("    px_unpack_ck(")), _v755), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){_v756}, 1)), px_str(");\n")));
            px_srcline(233);
            _v764 = px_int(0LL);
            px_srcline(234);
            while (px_is_truthy(px_lt(_v764, px_call(px_get_global("len"), (LXValue[]){_v757}, 1)))) {
                px_srcline(235);
                 _v737 = px_add(_v737, px_add(px_add(px_add(_v717, px_str("    ")), px_index(_v757, _v764)), px_str("\n")));
                px_srcline(236);
                 _v764 = px_add(_v764, px_int(1LL));
            }
        }
        else {
            px_srcline(238);
             _v737 = px_add(_v737, px_add(px_add(px_add(_v717, px_str("    ")), px_index(_v757, px_int(0LL))), px_str("\n")));
        }
        px_srcline(239);
        _v744 = px_index(_v715, px_int(3LL));
        px_srcline(241);
        if (px_is_truthy(px_not(px_get_global("cg_topbody")))) {
            px_srcline(242);
            _v765 = px_int(0LL);
            px_srcline(243);
            while (px_is_truthy(px_lt(_v765, px_call(px_get_global("len"), (LXValue[]){_v753}, 1)))) {
                px_srcline(244);
                (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){px_index(_v753, _v765)}, 1));
                px_srcline(245);
                 _v765 = px_add(_v765, px_int(1LL));
            }
        }
        px_srcline(246);
        _v741 = px_int(0LL);
        px_srcline(247);
        while (px_is_truthy(px_lt(_v741, px_call(px_get_global("len"), (LXValue[]){_v744}, 1)))) {
            px_srcline(248);
             _v737 = px_add(_v737, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v744, _v741), px_add(_v716, px_int(1LL))}, 2));
            px_srcline(249);
             _v741 = px_add(_v741, px_int(1LL));
        }
        px_srcline(250);
         _v737 = px_add(_v737, px_add(_v717, px_str("}\n")));
        px_srcline(251);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v749}, 1));
        px_srcline(252);
        return _v737;
    }
    px_srcline(253);
    if (px_is_truthy(px_eq(_v718, px_str("Return")))) {
        px_srcline(254);
        if (px_is_truthy(px_ne(px_index(_v715, px_int(1LL)), px_null()))) {
            px_srcline(255);
            _v722 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
            px_srcline(256);
            return px_add(px_add(px_add(_v717, px_str("return ")), _v722), px_str(";\n"));
        }
        px_srcline(257);
        return px_add(_v717, px_str("return px_null();\n"));
    }
    px_srcline(258);
    if (px_is_truthy(px_eq(_v718, px_str("Break")))) {
        px_srcline(259);
        return px_add(_v717, px_str("break;\n"));
    }
    px_srcline(260);
    if (px_is_truthy(px_eq(_v718, px_str("Continue")))) {
        px_srcline(261);
        return px_add(_v717, px_str("continue;\n"));
    }
    px_srcline(262);
    if (px_is_truthy(px_eq(_v718, px_str("Empty")))) {
        px_srcline(263);
        return px_str("");
    }
    px_srcline(264);
    if (px_is_truthy(px_eq(_v718, px_str("ChanDecl")))) {
        px_srcline(266);
        _v766 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(267);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(268);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(269);
            return px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v766), px_str("\", px_chan_create(0));\n"));
        }
        px_srcline(270);
        _v732 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v766}, 1);
        px_srcline(272);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v766}, 1));
        px_srcline(273);
        return px_add(px_add(px_add(_v717, px_str("LXValue ")), _v732), px_str(" = px_chan_create(0);\n"));
    }
    px_srcline(274);
    if (px_is_truthy(px_eq(_v718, px_str("Send")))) {
        px_srcline(275);
        _v742 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(276);
        _v732 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(2LL))}, 1);
        px_srcline(277);
        return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_chan_send(")), _v742), px_str(", ")), _v732), px_str(");\n"));
    }
    px_srcline(278);
    if (px_is_truthy(px_eq(_v718, px_str("Recv")))) {
        px_srcline(279);
        _v742 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(280);
        return px_add(px_add(px_add(_v717, px_str("px_chan_recv(")), _v742), px_str(");\n"));
    }
    px_srcline(281);
    if (px_is_truthy(px_eq(_v718, px_str("Spawn")))) {
        px_srcline(282);
        _v767 = px_index(_v715, px_int(1LL));
        px_srcline(283);
        if (px_is_truthy(px_eq(px_index(_v767, px_int(0LL)), px_str("Call")))) {
            px_srcline(284);
            _v724 = px_index(_v767, px_int(1LL));
            px_srcline(285);
            if (px_is_truthy(px_eq(px_index(_v724, px_int(0LL)), px_str("Var")))) {
                px_srcline(286);
                _v735 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v724, px_int(1LL))}, 1);
                px_srcline(287);
                _v768 = px_list_n((LXValue[]){}, 0);
                px_srcline(288);
                _v769 = px_index(_v767, px_int(2LL));
                px_srcline(289);
                _v770 = px_int(0LL);
                px_srcline(290);
                while (px_is_truthy(px_lt(_v770, px_call(px_get_global("len"), (LXValue[]){_v769}, 1)))) {
                    px_srcline(291);
                    (void)(px_method(_v768, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v769, _v770)}, 1)}, 1));
                    px_srcline(292);
                     _v770 = px_add(_v770, px_int(1LL));
                }
                px_srcline(293);
                _v729 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v769, _v768}, 2);
                px_srcline(294);
                _v730 = px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v729, px_int(0LL)), px_add(({ LXValue _s81 = px_add(px_add(px_add(px_add(px_str("px_spawn_name(\""), _v735), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v729, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s82 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v768}, 1)}, 1); px_add(_s81, _s82); }), px_str(")"))}, 2);
                px_srcline(295);
                return px_add(px_add(_v717, _v730), px_str(";\n"));
            }
            px_srcline(296);
            _v771 = px_index(_v715, px_int(2LL));
            px_srcline(297);
            (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s83 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v771, px_int(0LL))}, 1)), px_str(":")); LXValue _s84 = px_call(px_get_global("str"), (LXValue[]){px_index(_v771, px_int(1LL))}, 1); px_add(_s83, _s84); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)（方法调用请先绑定命名函数）"))}, 1));
            px_srcline(298);
            (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
        }
        px_srcline(299);
        _v772 = px_index(_v715, px_int(2LL));
        px_srcline(300);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s85 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v772, px_int(0LL))}, 1)), px_str(":")); LXValue _s86 = px_call(px_get_global("str"), (LXValue[]){px_index(_v772, px_int(1LL))}, 1); px_add(_s85, _s86); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()）"))}, 1));
        px_srcline(301);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(302);
    if (px_is_truthy(px_eq(_v718, px_str("Select")))) {
        px_srcline(303);
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v715, px_int(1LL)), px_index(_v715, px_int(2LL)), _v716}, 3);
    }
    px_srcline(304);
    if (px_is_truthy(px_eq(_v718, px_str("Import")))) {
        px_srcline(305);
        return px_add(_v717, px_str("/* import 忽略（MVP） */\n"));
    }
    px_srcline(306);
    if (px_is_truthy(px_eq(_v718, px_str("FuncDef")))) {
        px_srcline(311);
        _v773 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v715, px_int(1LL))}, 1);
        px_srcline(312);
        _v774 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v773}, 1);
        px_srcline(315);
        _v775 = px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v715, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v715, px_int(4LL))}, 2), _v773}, 3);
        px_srcline(316);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(321);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(322);
            return px_add(px_add(px_add(px_add(px_add(_v717, px_str("px_set_global(\"")), _v773), px_str("\", ")), _v775), px_str(");\n"));
        }
        px_srcline(323);
        if (px_is_truthy(px_ne(_v774, px_null()))) {
            px_srcline(324);
            _v776 = px_add(px_add(_v717, px_call(px_get_global("cg_store_of"), (LXValue[]){_v773, _v774, _v775}, 3)), px_str(";\n"));
            px_srcline(326);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v773}, 1));
            px_srcline(327);
            return _v776;
        }
        px_srcline(328);
        return px_str("");
    }
    px_srcline(330);
    return px_str("");
px_err_777:
    if (px_err_777_proped) return px_err_777_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt");
    LXValue _v778 = (nargs > 0) ? args[0] : px_null();
    LXValue _v779 = (nargs > 1) ? args[1] : px_null();
    LXValue _v780 = px_uninit();
    LXValue _v781 = px_uninit();
    LXValue _v782 = px_uninit();
    LXValue px_err_783_val = px_null();
    int px_err_783_proped = 0;
    px_srcline(336);
    _v780 = px_int(0LL);
    px_srcline(337);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v778}, 1), px_int(0LL)))) {
        px_srcline(338);
        _v781 = px_index(_v778, px_sub(px_call(px_get_global("len"), (LXValue[]){_v778}, 1), px_int(1LL)));
        px_srcline(339);
        if (px_is_truthy(({ LXValue _t785 = ({ LXValue _t784 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v781}, 1), px_str("list")); px_is_truthy(_t784) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v781}, 1), px_int(1LL)) : _t784; }); px_is_truthy(_t785) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v781, px_int(0LL))}, 1), px_str("int")) : _t785; }))) {
            px_srcline(340);
             _v780 = px_index(_v781, px_int(0LL));
        }
    }
    px_srcline(341);
    _v782 = px_str("");
    px_srcline(342);
    if (px_is_truthy(px_gt(_v780, px_int(0LL)))) {
        px_srcline(343);
         _v782 = px_add(({ LXValue _s87 = px_add(px_call(px_get_global("cg_pad"), (LXValue[]){_v779}, 1), px_str("px_srcline(")); LXValue _s88 = px_call(px_get_global("str"), (LXValue[]){_v780}, 1); px_add(_s87, _s88); }), px_str(");\n"));
    }
    px_srcline(344);
    return px_add(_v782, px_call(px_get_global("cg_gen_stmt_inner"), (LXValue[]){_v778, _v779}, 2));
px_err_783:
    if (px_err_783_proped) return px_err_783_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_global");
    LXValue _v786 = (nargs > 0) ? args[0] : px_null();
    LXValue _v787 = (nargs > 1) ? args[1] : px_null();
    LXValue _v788 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_789_val = px_null();
    int px_err_789_proped = 0;
    px_srcline(347);
    if (px_is_truthy(px_eq(_v786, px_str("Assign")))) {
        px_srcline(348);
        return _v788;
    }
    px_srcline(349);
    if (px_is_truthy(px_eq(_v786, px_str("Plus")))) {
        px_srcline(350);
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(351);
    if (px_is_truthy(px_eq(_v786, px_str("Minus")))) {
        px_srcline(352);
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(353);
    if (px_is_truthy(px_eq(_v786, px_str("Star")))) {
        px_srcline(354);
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(355);
    if (px_is_truthy(px_eq(_v786, px_str("Slash")))) {
        px_srcline(356);
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(357);
    if (px_is_truthy(px_eq(_v786, px_str("IntDiv")))) {
        px_srcline(358);
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(359);
    if (px_is_truthy(px_eq(_v786, px_str("Mod")))) {
        px_srcline(360);
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(361);
    if (px_is_truthy(px_eq(_v786, px_str("Pow")))) {
        px_srcline(362);
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(363);
    if (px_is_truthy(px_eq(_v786, px_str("BitAnd")))) {
        px_srcline(364);
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(365);
    if (px_is_truthy(px_eq(_v786, px_str("BitOr")))) {
        px_srcline(366);
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(367);
    if (px_is_truthy(px_eq(_v786, px_str("BitXor")))) {
        px_srcline(368);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(369);
    if (px_is_truthy(px_eq(_v786, px_str("Shl")))) {
        px_srcline(370);
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(371);
    if (px_is_truthy(px_eq(_v786, px_str("Shr")))) {
        px_srcline(372);
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(373);
    if (px_is_truthy(px_eq(_v786, px_str("ShrU")))) {
        px_srcline(374);
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v787), px_str("\"), ")), _v788), px_str(")"));
    }
    px_srcline(375);
    return _v788;
px_err_789:
    if (px_err_789_proped) return px_err_789_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_local");
    LXValue _v790 = (nargs > 0) ? args[0] : px_null();
    LXValue _v791 = (nargs > 1) ? args[1] : px_null();
    LXValue _v792 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_793_val = px_null();
    int px_err_793_proped = 0;
    px_srcline(378);
    if (px_is_truthy(px_eq(_v790, px_str("Assign")))) {
        px_srcline(379);
        return _v792;
    }
    px_srcline(380);
    if (px_is_truthy(px_eq(_v790, px_str("Plus")))) {
        px_srcline(381);
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(382);
    if (px_is_truthy(px_eq(_v790, px_str("Minus")))) {
        px_srcline(383);
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(384);
    if (px_is_truthy(px_eq(_v790, px_str("Star")))) {
        px_srcline(385);
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(386);
    if (px_is_truthy(px_eq(_v790, px_str("Slash")))) {
        px_srcline(387);
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(388);
    if (px_is_truthy(px_eq(_v790, px_str("IntDiv")))) {
        px_srcline(389);
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(390);
    if (px_is_truthy(px_eq(_v790, px_str("Mod")))) {
        px_srcline(391);
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(392);
    if (px_is_truthy(px_eq(_v790, px_str("Pow")))) {
        px_srcline(393);
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(394);
    if (px_is_truthy(px_eq(_v790, px_str("BitAnd")))) {
        px_srcline(395);
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(396);
    if (px_is_truthy(px_eq(_v790, px_str("BitOr")))) {
        px_srcline(397);
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(398);
    if (px_is_truthy(px_eq(_v790, px_str("BitXor")))) {
        px_srcline(399);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(400);
    if (px_is_truthy(px_eq(_v790, px_str("Shl")))) {
        px_srcline(401);
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(402);
    if (px_is_truthy(px_eq(_v790, px_str("Shr")))) {
        px_srcline(403);
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(404);
    if (px_is_truthy(px_eq(_v790, px_str("ShrU")))) {
        px_srcline(405);
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v791), px_str(", ")), _v792), px_str(")"));
    }
    px_srcline(406);
    return _v792;
px_err_793:
    if (px_err_793_proped) return px_err_793_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_select");
    LXValue _v794 = (nargs > 0) ? args[0] : px_null();
    LXValue _v795 = (nargs > 1) ? args[1] : px_null();
    LXValue _v796 = (nargs > 2) ? args[2] : px_null();
    LXValue _v797 = px_uninit();
    LXValue _v798 = px_uninit();
    LXValue _v799 = px_uninit();
    LXValue _v800 = px_uninit();
    LXValue _v801 = px_uninit();
    LXValue _v802 = px_uninit();
    LXValue _v803 = px_uninit();
    LXValue _v804 = px_uninit();
    LXValue _v805 = px_uninit();
    LXValue _v806 = px_uninit();
    LXValue _v807 = px_uninit();
    LXValue _v808 = px_uninit();
    LXValue _v809 = px_uninit();
    LXValue _v810 = px_uninit();
    LXValue _v811 = px_uninit();
    LXValue _v812 = px_uninit();
    LXValue _v813 = px_uninit();
    LXValue _v814 = px_uninit();
    LXValue _v815 = px_uninit();
    LXValue _v816 = px_uninit();
    LXValue _v817 = px_uninit();
    LXValue _v818 = px_uninit();
    LXValue _v819 = px_uninit();
    LXValue px_err_820_val = px_null();
    int px_err_820_proped = 0;
    px_srcline(409);
    _v797 = px_call(px_get_global("cg_pad"), (LXValue[]){_v796}, 1);
    px_srcline(410);
    _v798 = px_call(px_get_global("len"), (LXValue[]){_v794}, 1);
    px_srcline(411);
    if (px_is_truthy(px_eq(_v798, px_int(0LL)))) {
        px_srcline(412);
        return px_str("select 至少需要一个 case 分支");
    }
    px_srcline(413);
    _v799 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    px_srcline(414);
    _v800 = px_str("");
    px_srcline(417);
    _v801 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(418);
    _v802 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(419);
    _v803 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(420);
    _v804 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(422);
    _v805 = px_list_n((LXValue[]){}, 0);
    px_srcline(423);
    _v806 = px_int(0LL);
    px_srcline(424);
    while (px_is_truthy(px_lt(_v806, _v798))) {
        px_srcline(425);
        _v807 = px_index(px_index(_v794, _v806), px_int(1LL));
        px_srcline(426);
        if (px_is_truthy(px_eq(px_index(_v807, px_int(0LL)), px_str("Call")))) {
            px_srcline(427);
            _v808 = px_index(_v807, px_int(1LL));
            px_srcline(428);
            if (px_is_truthy(px_eq(px_index(_v808, px_int(0LL)), px_str("Field")))) {
                px_srcline(429);
                _v809 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v808, px_int(2LL))}, 1);
                px_srcline(430);
                if (px_is_truthy(px_eq(_v809, px_str("recv")))) {
                    px_srcline(431);
                    (void)(px_method(_v805, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v808, px_int(1LL))}, 1)}, 1));
                    px_srcline(432);
                     _v806 = px_add(_v806, px_int(1LL));
                    px_srcline(433);
                    continue;
                }
                px_srcline(434);
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v809), px_str("）"));
            }
            px_srcline(435);
            return px_str("select case 仅支持 ch.recv()");
        }
        px_srcline(436);
        return px_str("select case 仅支持 ch.recv()");
    }
    px_srcline(437);
     _v800 = px_add(_v800, px_add(({ LXValue _s91 = px_add(({ LXValue _s89 = px_add(px_add(px_add(_v797, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str("[")); LXValue _s90 = px_call(px_get_global("str"), (LXValue[]){_v798}, 1); px_add(_s89, _s90); }), px_str("] = {")); LXValue _s92 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v805}, 2); px_add(_s91, _s92); }), px_str("};\n")));
    px_srcline(438);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(": {\n")));
    px_srcline(439);
    _v810 = px_list_n((LXValue[]){}, 0);
    px_srcline(440);
    _v811 = px_int(0LL);
    px_srcline(441);
    while (px_is_truthy(px_lt(_v811, _v798))) {
        px_srcline(442);
        (void)(px_method(_v810, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v811}, 1)}, 1));
        px_srcline(443);
         _v811 = px_add(_v811, px_int(1LL));
    }
    px_srcline(444);
     _v800 = px_add(_v800, px_add(({ LXValue _s95 = px_add(({ LXValue _s93 = px_add(px_add(px_add(_v797, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str("[")); LXValue _s94 = px_call(px_get_global("str"), (LXValue[]){_v798}, 1); px_add(_s93, _s94); }), px_str("] = {")); LXValue _s96 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v810}, 2); px_add(_s95, _s96); }), px_str("};\n")));
    px_srcline(445);
    if (px_is_truthy(px_gt(_v798, px_int(1LL)))) {
        px_srcline(446);
        _v812 = px_add(({ LXValue _s101 = px_add(({ LXValue _s99 = px_add(({ LXValue _s97 = px_add(px_add(px_add(_v797, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = ")); LXValue _s98 = px_call(px_get_global("str"), (LXValue[]){_v798}, 1); px_add(_s97, _s98); }), px_str(" - 1; _i")); LXValue _s100 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s99, _s100); }), px_str(" > 0; _i")); LXValue _s102 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s101, _s102); }), px_str("--) { "));
        px_srcline(447);
         _v812 = px_add(_v812, px_add(({ LXValue _s103 = px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = rand() % (_i")); LXValue _s104 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s103, _s104); }), px_str(" + 1); ")));
        px_srcline(448);
         _v812 = px_add(_v812, px_add(({ LXValue _s121 = px_add(({ LXValue _s119 = px_add(({ LXValue _s117 = px_add(({ LXValue _s115 = px_add(({ LXValue _s113 = px_add(({ LXValue _s111 = px_add(({ LXValue _s109 = px_add(({ LXValue _s107 = px_add(({ LXValue _s105 = px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = _ord")); LXValue _s106 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s105, _s106); }), px_str("[_i")); LXValue _s108 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s107, _s108); }), px_str("]; _ord")); LXValue _s110 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s109, _s110); }), px_str("[_i")); LXValue _s112 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s111, _s112); }), px_str("] = _ord")); LXValue _s114 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s113, _s114); }), px_str("[_j")); LXValue _s116 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s115, _s116); }), px_str("]; _ord")); LXValue _s118 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s117, _s118); }), px_str("[_j")); LXValue _s120 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s119, _s120); }), px_str("] = _t")); LXValue _s122 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s121, _s122); }), px_str("; ")));
        px_srcline(449);
         _v812 = px_add(_v812, px_str("}\n"));
        px_srcline(450);
         _v800 = px_add(_v800, _v812);
    }
    px_srcline(451);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = px_null();\n")));
    px_srcline(452);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = -1;\n")));
    px_srcline(453);
     _v800 = px_add(_v800, px_add(({ LXValue _s127 = px_add(({ LXValue _s125 = px_add(({ LXValue _s123 = px_add(px_add(px_add(_v797, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = 0; _k")); LXValue _s124 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s123, _s124); }), px_str(" < ")); LXValue _s126 = px_call(px_get_global("str"), (LXValue[]){_v798}, 1); px_add(_s125, _s126); }), px_str("; _k")); LXValue _s128 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s127, _s128); }), px_str("++) {\n")));
    px_srcline(454);
     _v800 = px_add(_v800, px_add(({ LXValue _s131 = px_add(({ LXValue _s129 = px_add(px_add(px_add(_v797, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" = _ord")); LXValue _s130 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s129, _s130); }), px_str("[_k")); LXValue _s132 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s131, _s132); }), px_str("];\n")));
    px_srcline(455);
     _v800 = px_add(_v800, px_add(({ LXValue _s139 = px_add(({ LXValue _s137 = px_add(({ LXValue _s135 = px_add(({ LXValue _s133 = px_add(px_add(px_add(_v797, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str("[_idx")); LXValue _s134 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s133, _s134); }), px_str("], &_rv")); LXValue _s136 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s135, _s136); }), px_str(")) { _picked")); LXValue _s138 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s137, _s138); }), px_str(" = _idx")); LXValue _s140 = px_call(px_get_global("str"), (LXValue[]){_v799}, 1); px_add(_s139, _s140); }), px_str("; break; }\n")));
    px_srcline(456);
     _v800 = px_add(_v800, px_add(_v797, px_str("    }\n")));
    px_srcline(458);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" >= 0) {\n")));
    px_srcline(459);
    _v813 = px_int(0LL);
    px_srcline(460);
    while (px_is_truthy(px_lt(_v813, _v798))) {
        px_srcline(461);
        _v814 = px_index(px_index(_v794, _v813), px_int(0LL));
        px_srcline(462);
        _v815 = px_index(px_index(_v794, _v813), px_int(2LL));
        px_srcline(463);
        _v816 = px_add(({ LXValue _s141 = px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" == ")); LXValue _s142 = px_call(px_get_global("str"), (LXValue[]){_v813}, 1); px_add(_s141, _s142); }), px_str(")"));
        px_srcline(464);
        if (px_is_truthy(px_gt(_v813, px_int(0LL)))) {
            px_srcline(465);
             _v816 = px_add(({ LXValue _s143 = px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(" == ")); LXValue _s144 = px_call(px_get_global("str"), (LXValue[]){_v813}, 1); px_add(_s143, _s144); }), px_str(")"));
        }
        px_srcline(466);
         _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("        ")), _v816), px_str(" {\n")));
        px_srcline(467);
        if (px_is_truthy(px_ne(_v814, px_null()))) {
            px_srcline(468);
            _v817 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v814}, 1)}, 1);
            px_srcline(469);
             _v800 = px_add(_v800, px_add(px_add(px_add(px_add(px_add(_v797, px_str("            LXValue ")), _v817), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(";\n")));
        }
        px_srcline(470);
        _v818 = px_int(0LL);
        px_srcline(471);
        while (px_is_truthy(px_lt(_v818, px_call(px_get_global("len"), (LXValue[]){_v815}, 1)))) {
            px_srcline(472);
             _v800 = px_add(_v800, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v815, _v818), px_add(_v796, px_int(3LL))}, 2));
            px_srcline(473);
             _v818 = px_add(_v818, px_int(1LL));
        }
        px_srcline(474);
         _v800 = px_add(_v800, px_add(_v797, px_str("        }\n")));
        px_srcline(475);
         _v813 = px_add(_v813, px_int(1LL));
    }
    px_srcline(476);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(";\n")));
    px_srcline(477);
     _v800 = px_add(_v800, px_add(_v797, px_str("    }\n")));
    px_srcline(479);
    if (px_is_truthy(px_ne(_v795, px_null()))) {
        px_srcline(480);
         _v800 = px_add(_v800, px_add(_v797, px_str("    {\n")));
        px_srcline(481);
        _v819 = px_int(0LL);
        px_srcline(482);
        while (px_is_truthy(px_lt(_v819, px_call(px_get_global("len"), (LXValue[]){_v795}, 1)))) {
            px_srcline(483);
             _v800 = px_add(_v800, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v795, _v819), px_add(_v796, px_int(2LL))}, 2));
            px_srcline(484);
             _v819 = px_add(_v819, px_int(1LL));
        }
        px_srcline(485);
         _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(";\n")));
        px_srcline(486);
         _v800 = px_add(_v800, px_add(_v797, px_str("    }\n")));
    }
    px_srcline(488);
     _v800 = px_add(_v800, px_add(_v797, px_str("    px_select_wait();\n")));
    px_srcline(489);
     _v800 = px_add(_v800, px_add(_v797, px_str("}\n")));
    px_srcline(490);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(";\n")));
    px_srcline(491);
     _v800 = px_add(_v800, px_add(px_add(px_add(_v797, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v799}, 1)), px_str(": ;\n")));
    px_srcline(492);
    px_set_global("cg_vars", _v802);
    px_srcline(493);
    px_set_global("cg_var_types", _v803);
    px_srcline(494);
    px_set_global("cg_cells", _v804);
    px_srcline(495);
    (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v801}, 1));
    px_srcline(496);
    return _v800;
px_err_820:
    if (px_err_820_proped) return px_err_820_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_collect");
    LXValue _v821 = (nargs > 0) ? args[0] : px_null();
    LXValue _v822 = px_uninit();
    LXValue _v823 = px_uninit();
    LXValue _v824 = px_uninit();
    LXValue _v825 = px_uninit();
    LXValue _v826 = px_uninit();
    LXValue _v827 = px_uninit();
    LXValue _v828 = px_uninit();
    LXValue _v829 = px_uninit();
    LXValue _v830 = px_uninit();
    LXValue _v831 = px_uninit();
    LXValue px_err_832_val = px_null();
    int px_err_832_proped = 0;
    px_srcline(9);
    _v822 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(10);
    (void)(px_method(_v822, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(11);
    px_index_set(_v822, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_srcline(12);
    px_index_set(_v822, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_srcline(13);
    px_index_set(_v822, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_srcline(14);
    px_index_set(_v822, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_srcline(15);
    px_index_set(_v822, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_srcline(16);
    px_index_set(_v822, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    px_srcline(17);
    _v823 = px_int(0LL);
    px_srcline(18);
    while (px_is_truthy(px_lt(_v823, px_call(px_get_global("len"), (LXValue[]){_v821}, 1)))) {
        px_srcline(19);
        _v824 = px_index(_v821, _v823);
        px_srcline(20);
        _v825 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v824, px_int(2LL))}, 1);
        px_srcline(21);
        (void)(px_method(px_index(_v822, px_str("its")), "append", (LXValue[]){_v825}, 1));
        px_srcline(22);
        (void)(px_method(px_index(_v822, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(23);
        (void)(px_method(px_index(_v822, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(24);
        (void)(px_method(px_index(_v822, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(25);
        _v826 = px_str("");
        px_srcline(26);
        _v827 = px_list_n((LXValue[]){}, 0);
        px_srcline(27);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1), px_int(1LL)))) {
            px_srcline(28);
            _v828 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            px_srcline(29);
            _v829 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v824, px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(30);
            _v830 = px_null();
            px_srcline(31);
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v829}, 1))) {
                px_srcline(32);
                 _v830 = px_index(px_get_global("cg_vars"), _v829);
            }
            px_srcline(33);
            px_index_set(px_get_global("cg_vars"), _v829, _v828);
            px_srcline(34);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v829}, 1))) {
                px_srcline(35);
                (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v829}, 1));
            }
            px_srcline(36);
            (void)(px_method(_v827, "append", (LXValue[]){px_list_n((LXValue[]){_v829, _v830}, 2)}, 1));
            px_srcline(37);
             _v826 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v828), px_str(" = ")), px_index(px_index(_v822, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v822, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            px_srcline(39);
            _v831 = px_int(0LL);
            px_srcline(44);
             _v826 = px_add(({ LXValue _s145 = px_add(px_add(px_str("px_unpack_ck("), px_index(px_index(_v822, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v822, px_str("itms"))}, 1), px_int(1LL)))), px_str(", ")); LXValue _s146 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1)}, 1); px_add(_s145, _s146); }), px_str("); "));
            px_srcline(45);
            while (px_is_truthy(px_lt(_v831, px_call(px_get_global("len"), (LXValue[]){px_index(_v824, px_int(1LL))}, 1)))) {
                px_srcline(46);
                _v829 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v824, px_int(1LL)), _v831)}, 1);
                px_srcline(47);
                _v828 = ({ LXValue _s147 = px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")); LXValue _s148 = px_call(px_get_global("str"), (LXValue[]){_v831}, 1); px_add(_s147, _s148); });
                px_srcline(48);
                _v830 = px_null();
                px_srcline(49);
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v829}, 1))) {
                    px_srcline(50);
                     _v830 = px_index(px_get_global("cg_vars"), _v829);
                }
                px_srcline(51);
                px_index_set(px_get_global("cg_vars"), _v829, _v828);
                px_srcline(52);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v829}, 1))) {
                    px_srcline(53);
                    (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v829}, 1));
                }
                px_srcline(54);
                (void)(px_method(_v827, "append", (LXValue[]){px_list_n((LXValue[]){_v829, _v830}, 2)}, 1));
                px_srcline(55);
                 _v826 = px_add(_v826, px_add(({ LXValue _s149 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v828), px_str(" = px_index(")), px_index(px_index(_v822, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v822, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")); LXValue _s150 = px_call(px_get_global("str"), (LXValue[]){_v831}, 1); px_add(_s149, _s150); }), px_str(")); ")));
                px_srcline(56);
                 _v831 = px_add(_v831, px_int(1LL));
            }
        }
        px_srcline(57);
        (void)(px_method(px_index(_v822, px_str("binds")), "append", (LXValue[]){_v826}, 1));
        px_srcline(58);
        (void)(px_method(px_index(_v822, px_str("saved_all")), "append", (LXValue[]){_v827}, 1));
        px_srcline(59);
         _v823 = px_add(_v823, px_int(1LL));
    }
    px_srcline(60);
    return _v822;
px_err_832:
    if (px_err_832_proped) return px_err_832_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_restore");
    LXValue _v833 = (nargs > 0) ? args[0] : px_null();
    LXValue _v834 = px_uninit();
    LXValue _v835 = px_uninit();
    LXValue _v836 = px_uninit();
    LXValue _v837 = px_uninit();
    LXValue _v838 = px_uninit();
    LXValue px_err_839_val = px_null();
    int px_err_839_proped = 0;
    px_srcline(62);
    _v834 = px_int(0LL);
    px_srcline(63);
    while (px_is_truthy(px_lt(_v834, px_call(px_get_global("len"), (LXValue[]){_v833}, 1)))) {
        px_srcline(64);
        _v835 = px_index(_v833, _v834);
        px_srcline(65);
        _v836 = px_int(0LL);
        px_srcline(66);
        while (px_is_truthy(px_lt(_v836, px_call(px_get_global("len"), (LXValue[]){_v835}, 1)))) {
            px_srcline(67);
            _v837 = px_index(px_index(_v835, _v836), px_int(0LL));
            px_srcline(68);
            _v838 = px_index(px_index(_v835, _v836), px_int(1LL));
            px_srcline(69);
            if (px_is_truthy(px_eq(_v838, px_null()))) {
                px_srcline(70);
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v837}, 1));
            }
            else {
                px_srcline(72);
                px_index_set(px_get_global("cg_vars"), _v837, _v838);
            }
            px_srcline(73);
             _v836 = px_add(_v836, px_int(1LL));
        }
        px_srcline(74);
         _v834 = px_add(_v834, px_int(1LL));
    }
px_err_839:
    if (px_err_839_proped) return px_err_839_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_body");
    LXValue _v840 = (nargs > 0) ? args[0] : px_null();
    LXValue _v841 = (nargs > 1) ? args[1] : px_null();
    LXValue _v842 = (nargs > 2) ? args[2] : px_null();
    LXValue _v843 = px_uninit();
    LXValue _v844 = px_uninit();
    LXValue _v845 = px_uninit();
    LXValue _v846 = px_uninit();
    LXValue _v847 = px_uninit();
    LXValue px_err_848_val = px_null();
    int px_err_848_proped = 0;
    px_srcline(77);
    _v843 = px_str("");
    px_srcline(78);
    if (px_is_truthy(px_ne(_v841, px_null()))) {
        px_srcline(79);
         _v843 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v841), px_str(")) { ")), _v842), px_str("} "));
    }
    else {
        px_srcline(81);
         _v843 = _v842;
    }
    px_srcline(82);
    _v844 = px_call(px_get_global("len"), (LXValue[]){px_index(_v840, px_str("its"))}, 1);
    px_srcline(83);
    _v845 = px_sub(_v844, px_int(1LL));
    px_srcline(84);
    while (px_is_truthy(px_ge(_v845, px_int(0LL)))) {
        px_srcline(85);
        _v846 = px_str("");
        px_srcline(86);
        if (px_is_truthy(px_lt(px_add(_v845, px_int(1LL)), _v844))) {
            px_srcline(87);
             _v846 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v840, px_str("ivs")), px_add(_v845, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v840, px_str("its")), px_add(_v845, px_int(1LL)))), px_str("; "));
        }
        px_srcline(92);
        _v847 = px_call(px_get_global("cg_iter_len_tmp"), (LXValue[]){}, 0);
        px_srcline(93);
         _v843 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), _v847), px_str(" = (int)px_len(")), px_index(px_index(_v840, px_str("ivs")), _v845)), px_str("), ")), px_index(px_index(_v840, px_str("idxs")), _v845)), px_str("=0; ")), px_index(px_index(_v840, px_str("idxs")), _v845)), px_str("<")), _v847), px_str("; ")), px_index(px_index(_v840, px_str("idxs")), _v845)), px_str("++) { px_iter_ck(")), px_index(px_index(_v840, px_str("ivs")), _v845)), px_str(", ")), _v847), px_str("); LXValue ")), px_index(px_index(_v840, px_str("itms")), _v845)), px_str(" = px_iter_at(")), px_index(px_index(_v840, px_str("ivs")), _v845)), px_str(", px_int(")), px_index(px_index(_v840, px_str("idxs")), _v845)), px_str(")); ")), px_index(px_index(_v840, px_str("binds")), _v845)), _v846), _v843), px_str(" } "));
        px_srcline(94);
         _v845 = px_sub(_v845, px_int(1LL));
    }
    px_srcline(95);
    return _v843;
px_err_848:
    if (px_err_848_proped) return px_err_848_val;
    return px_null();
}

static LXValue fn_cg_gen_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_closure");
    LXValue _v849 = (nargs > 0) ? args[0] : px_null();
    LXValue _v850 = (nargs > 1) ? args[1] : px_null();
    LXValue _v851 = (nargs > 2) ? args[2] : px_null();
    LXValue _v852 = px_uninit();
    LXValue _v853 = px_uninit();
    LXValue _v854 = px_uninit();
    LXValue _v855 = px_uninit();
    LXValue _v856 = px_uninit();
    LXValue _v857 = px_uninit();
    LXValue _v858 = px_uninit();
    LXValue _v859 = px_uninit();
    LXValue _v860 = px_uninit();
    LXValue _v861 = px_uninit();
    LXValue _v862 = px_uninit();
    LXValue _v863 = px_uninit();
    LXValue _v864 = px_uninit();
    LXValue _v865 = px_uninit();
    LXValue _v866 = px_uninit();
    LXValue _v867 = px_uninit();
    LXValue _v868 = px_uninit();
    LXValue _v869 = px_uninit();
    LXValue _v870 = px_uninit();
    LXValue _v871 = px_uninit();
    LXValue _v872 = px_uninit();
    LXValue _v873 = px_uninit();
    LXValue _v874 = px_uninit();
    LXValue _v875 = px_uninit();
    LXValue _v876 = px_uninit();
    LXValue _v877 = px_uninit();
    LXValue _v878 = px_uninit();
    LXValue _v879 = px_uninit();
    LXValue px_err_880_val = px_null();
    int px_err_880_proped = 0;
    px_srcline(102);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(103);
    _v852 = px_get_global("cg_closure_id");
    px_srcline(104);
    _v853 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v852}, 1));
    px_srcline(106);
    _v854 = px_list_n((LXValue[]){}, 0);
    px_srcline(107);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v849, px_null(), _v850, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v854}, 2));
    px_srcline(108);
    _v855 = px_list_n((LXValue[]){}, 0);
    px_srcline(109);
    LXValue _t881 = _v854;
    int _il1 = (int)px_len(_t881);
    for (int _t882 = 0; _t882 < _il1; _t882++) {
        px_iter_ck(_t881, _il1);
        _v856 = px_iter_at(_t881, px_int(_t882));
        px_srcline(110);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v856}, 1), px_null()))) {
            px_srcline(111);
            (void)(px_method(_v855, "append", (LXValue[]){_v856}, 1));
        }
    }
    px_srcline(113);
    _v857 = px_str("px_null()");
    px_srcline(114);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v855}, 1), px_int(0LL)))) {
        px_srcline(115);
        _v858 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(116);
        LXValue _t883 = _v855;
        int _il2 = (int)px_len(_t883);
        for (int _t884 = 0; _t884 < _il2; _t884++) {
            px_iter_ck(_t883, _il2);
            _v856 = px_iter_at(_t883, px_int(_t884));
            px_srcline(117);
            _v859 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v856}, 1);
            px_srcline(120);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v856}, 1))) {
                px_srcline(121);
                 _v858 = px_add(_v858, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v856), px_str("\", ")), _v859), px_str("); ")));
            }
            else {
                px_srcline(123);
                 _v858 = px_add(_v858, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v856), px_str("\", px_cell(")), _v859), px_str(")); ")));
            }
        }
        px_srcline(124);
         _v858 = px_add(_v858, px_str("_capenv; })"));
        px_srcline(125);
         _v857 = _v858;
    }
    px_srcline(126);
    _v860 = px_add(px_add(px_str("static LXValue "), _v853), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(127);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v855}, 1), px_int(0LL)))) {
        px_srcline(128);
         _v860 = px_add(_v860, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(130);
         _v860 = px_add(_v860, px_str("    (void)ctx;\n"));
    }
    px_srcline(131);
    _v861 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(132);
    _v862 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(133);
    _v863 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(136);
    _v864 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(137);
    _v865 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(138);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(140);
    _v866 = px_get_global("cg_topbody");
    px_srcline(141);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(142);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(143);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(144);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(145);
    _v867 = px_int(0LL);
    px_srcline(146);
    while (px_is_truthy(px_lt(_v867, px_call(px_get_global("len"), (LXValue[]){_v849}, 1)))) {
        px_srcline(147);
        _v868 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v849, _v867), px_int(1LL))}, 1);
        px_srcline(148);
        _v869 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v868}, 1);
        px_srcline(149);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v868}, 1));
        px_srcline(150);
         _v860 = px_add(_v860, px_add(({ LXValue _s151 = px_add(px_add(px_add(px_add(px_str("    LXValue "), _v869), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v867}, 1)), px_str(") ? args[")); LXValue _s152 = px_call(px_get_global("str"), (LXValue[]){_v867}, 1); px_add(_s151, _s152); }), px_str("] : px_null();\n")));
        px_srcline(151);
         _v867 = px_add(_v867, px_int(1LL));
    }
    px_srcline(153);
    LXValue _t885 = _v855;
    int _il3 = (int)px_len(_t885);
    for (int _t886 = 0; _t886 < _il3; _t886++) {
        px_iter_ck(_t885, _il3);
        _v856 = px_iter_at(_t885, px_int(_t886));
        px_srcline(154);
        _v870 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v856}, 1);
        px_srcline(155);
        px_index_set(px_get_global("cg_cells"), _v856, px_int(1LL));
        px_srcline(156);
        if (px_is_truthy(px_method(_v865, "has", (LXValue[]){_v856}, 1))) {
            px_srcline(157);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v856}, 1));
        }
        px_srcline(158);
         _v860 = px_add(_v860, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v870), px_str(" = px_env_lookup(ctx, \"")), _v856), px_str("\");\n")));
    }
    px_srcline(168);
    _v871 = px_list_n((LXValue[]){}, 0);
    px_srcline(169);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v850, _v871}, 2));
    px_srcline(170);
    _v872 = px_list_n((LXValue[]){}, 0);
    px_srcline(171);
    _v873 = px_list_n((LXValue[]){}, 0);
    px_srcline(172);
    if (px_is_truthy(({ LXValue _t888 = ({ LXValue _t887 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v850}, 1), px_str("list")); px_is_truthy(_t887) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v850}, 1), px_int(0LL)) : _t887; }); px_is_truthy(_t888) ? px_eq(px_index(_v850, px_int(0LL)), px_str("Block")) : _t888; }))) {
        px_srcline(173);
        (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v850, px_int(1LL)), _v872}, 2));
        px_srcline(174);
        (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v850, px_int(1LL)), _v873}, 2));
    }
    px_srcline(175);
    _v874 = px_list_n((LXValue[]){}, 0);
    px_srcline(176);
    _v875 = px_int(0LL);
    px_srcline(177);
    while (px_is_truthy(px_lt(_v875, px_call(px_get_global("len"), (LXValue[]){_v872}, 1)))) {
        px_srcline(178);
        _v876 = px_index(_v872, _v875);
        px_srcline(180);
        if (px_is_truthy(({ LXValue _t890 = px_eq(px_call(px_get_global("cg_var_of"), (LXValue[]){_v876}, 1), px_null()); px_is_truthy(_t890) ? px_not(({ LXValue _t889 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v873, _v876}, 2)); px_is_truthy(_t889) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_topnames"), _v876}, 2) : _t889; })) : _t890; }))) {
            px_srcline(181);
            _v877 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v876}, 1);
            px_srcline(182);
            if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v871, _v876}, 2))) {
                px_srcline(183);
                px_index_set(px_get_global("cg_cells"), _v876, px_int(1LL));
            }
            px_srcline(184);
            (void)(px_method(_v874, "append", (LXValue[]){px_list_n((LXValue[]){_v877, _v876}, 2)}, 1));
        }
        px_srcline(185);
         _v875 = px_add(_v875, px_int(1LL));
    }
    px_srcline(186);
    _v878 = px_int(0LL);
    px_srcline(187);
    while (px_is_truthy(px_lt(_v878, px_call(px_get_global("len"), (LXValue[]){_v874}, 1)))) {
        px_srcline(188);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(px_index(_v874, _v878), px_int(1LL))}, 1))) {
            px_srcline(189);
             _v860 = px_add(_v860, px_add(px_add(px_str("    LXValue "), px_index(px_index(_v874, _v878), px_int(0LL))), px_str(" = px_cell(px_uninit());\n")));
        }
        else {
            px_srcline(191);
             _v860 = px_add(_v860, px_add(px_add(px_str("    LXValue "), px_index(px_index(_v874, _v878), px_int(0LL))), px_str(" = px_uninit();\n")));
        }
        px_srcline(192);
         _v878 = px_add(_v878, px_int(1LL));
    }
    px_srcline(193);
    _v879 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v850}, 1);
    px_srcline(194);
     _v860 = px_add(_v860, px_add(px_add(px_str("    return "), _v879), px_str(";\n")));
    px_srcline(195);
     _v860 = px_add(_v860, px_str("}\n"));
    px_srcline(196);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v860));
    px_srcline(197);
    px_set_global("cg_vars", _v861);
    px_srcline(198);
    px_set_global("cg_var_types", _v862);
    px_srcline(199);
    px_set_global("cg_cells", _v863);
    px_srcline(200);
    px_set_global("cg_topbody", _v866);
    px_srcline(201);
    px_set_global("cg_inited", _v864);
    px_srcline(205);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v855}, 1), px_int(0LL)))) {
        px_srcline(206);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\""), _v851), px_str("\", ")), _v853), px_str(", ")), _v857), px_str(")"));
    }
    px_srcline(207);
    return px_add(px_add(px_add(px_add(px_str("px_func(\""), _v851), px_str("\", ")), _v853), px_str(", NULL)"));
px_err_880:
    if (px_err_880_proped) return px_err_880_val;
    return px_null();
}

static LXValue fn_cg_side_effect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_side_effect");
    LXValue _v891 = (nargs > 0) ? args[0] : px_null();
    LXValue _v892 = px_uninit();
    LXValue _v893 = px_uninit();
    LXValue px_err_894_val = px_null();
    int px_err_894_proped = 0;
    px_srcline(221);
    if (px_is_truthy(({ LXValue _t895 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v891}, 1), px_str("list")); px_is_truthy(_t895) ? _t895 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v891}, 1), px_int(0LL)); }))) {
        px_srcline(222);
        return px_bool(false);
    }
    px_srcline(223);
    _v892 = px_index(_v891, px_int(0LL));
    px_srcline(224);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v892}, 1), px_str("string")))) {
        px_srcline(225);
        if (px_is_truthy(({ LXValue _t899 = ({ LXValue _t898 = ({ LXValue _t897 = ({ LXValue _t896 = px_eq(_v892, px_str("Call")); px_is_truthy(_t896) ? _t896 : px_eq(_v892, px_str("Pipe")); }); px_is_truthy(_t897) ? _t897 : px_eq(_v892, px_str("ListComp")); }); px_is_truthy(_t898) ? _t898 : px_eq(_v892, px_str("DictComp")); }); px_is_truthy(_t899) ? _t899 : px_eq(_v892, px_str("GenExp")); }))) {
            px_srcline(226);
            return px_bool(true);
        }
        px_srcline(229);
        if (px_is_truthy(px_eq(_v892, px_str("Closure")))) {
            px_srcline(230);
            return px_bool(false);
        }
    }
    px_srcline(231);
    _v893 = px_int(0LL);
    px_srcline(232);
    while (px_is_truthy(px_lt(_v893, px_call(px_get_global("len"), (LXValue[]){_v891}, 1)))) {
        px_srcline(233);
        if (px_is_truthy(px_call(px_get_global("cg_side_effect"), (LXValue[]){px_index(_v891, _v893)}, 1))) {
            px_srcline(234);
            return px_bool(true);
        }
        px_srcline(235);
         _v893 = px_add(_v893, px_int(1LL));
    }
    px_srcline(236);
    return px_bool(false);
px_err_894:
    if (px_err_894_proped) return px_err_894_val;
    return px_null();
}

static LXValue fn_cg_seq_operands(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_operands");
    LXValue _v900 = (nargs > 0) ? args[0] : px_null();
    LXValue _v901 = (nargs > 1) ? args[1] : px_null();
    LXValue _v902 = px_uninit();
    LXValue _v903 = px_uninit();
    LXValue _v904 = px_uninit();
    LXValue _v905 = px_uninit();
    LXValue _v906 = px_uninit();
    LXValue px_err_907_val = px_null();
    int px_err_907_proped = 0;
    px_srcline(239);
    _v902 = px_int(0LL);
    px_srcline(240);
    _v903 = px_int(0LL);
    px_srcline(241);
    while (px_is_truthy(px_lt(_v903, px_call(px_get_global("len"), (LXValue[]){_v901}, 1)))) {
        px_srcline(242);
        if (px_is_truthy(px_index(_v901, _v903))) {
            px_srcline(243);
             _v902 = px_add(_v902, px_int(1LL));
        }
        px_srcline(244);
         _v903 = px_add(_v903, px_int(1LL));
    }
    px_srcline(245);
    if (px_is_truthy(px_lt(_v902, px_int(2LL)))) {
        px_srcline(246);
        return px_list_n((LXValue[]){px_null(), px_null()}, 2);
    }
    px_srcline(247);
    _v904 = px_str("");
    px_srcline(248);
    _v905 = px_list_n((LXValue[]){}, 0);
    px_srcline(249);
     _v903 = px_int(0LL);
    px_srcline(250);
    while (px_is_truthy(px_lt(_v903, px_call(px_get_global("len"), (LXValue[]){_v900}, 1)))) {
        px_srcline(251);
        if (px_is_truthy(px_index(_v901, _v903))) {
            px_srcline(255);
            px_set_global("cg_seq_uid", px_add(px_get_global("cg_seq_uid"), px_int(1LL)));
            px_srcline(256);
            _v906 = px_add(px_str("_s"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_seq_uid")}, 1));
            px_srcline(257);
             _v904 = px_add(px_add(px_add(px_add(px_add(_v904, px_str("LXValue ")), _v906), px_str(" = ")), px_index(_v900, _v903)), px_str("; "));
            px_srcline(258);
            (void)(px_method(_v905, "append", (LXValue[]){_v906}, 1));
        }
        else {
            px_srcline(260);
            (void)(px_method(_v905, "append", (LXValue[]){px_index(_v900, _v903)}, 1));
        }
        px_srcline(261);
         _v903 = px_add(_v903, px_int(1LL));
    }
    px_srcline(262);
    return px_list_n((LXValue[]){_v904, _v905}, 2);
px_err_907:
    if (px_err_907_proped) return px_err_907_val;
    return px_null();
}

static LXValue fn_cg_seq_join(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_join");
    LXValue _v908 = (nargs > 0) ? args[0] : px_null();
    LXValue _v909 = (nargs > 1) ? args[1] : px_null();
    LXValue _v910 = px_uninit();
    LXValue _v911 = px_uninit();
    LXValue _v912 = px_uninit();
    LXValue px_err_913_val = px_null();
    int px_err_913_proped = 0;
    px_srcline(269);
    _v910 = px_list_n((LXValue[]){}, 0);
    px_srcline(270);
    _v911 = px_int(0LL);
    px_srcline(271);
    while (px_is_truthy(px_lt(_v911, px_call(px_get_global("len"), (LXValue[]){_v908}, 1)))) {
        px_srcline(272);
        (void)(px_method(_v910, "append", (LXValue[]){px_call(px_get_global("cg_side_effect"), (LXValue[]){px_index(_v908, _v911)}, 1)}, 1));
        px_srcline(273);
         _v911 = px_add(_v911, px_int(1LL));
    }
    px_srcline(274);
    _v912 = px_call(px_get_global("cg_seq_operands"), (LXValue[]){_v909, _v910}, 2);
    px_srcline(275);
    if (px_is_truthy(px_eq(px_index(_v912, px_int(0LL)), px_null()))) {
        px_srcline(276);
        return px_list_n((LXValue[]){px_null(), _v909}, 2);
    }
    px_srcline(277);
    return px_list_n((LXValue[]){px_index(_v912, px_int(0LL)), px_index(_v912, px_int(1LL))}, 2);
px_err_913:
    if (px_err_913_proped) return px_err_913_val;
    return px_null();
}

static LXValue fn_cg_seq_wrap(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_wrap");
    LXValue _v914 = (nargs > 0) ? args[0] : px_null();
    LXValue _v915 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_916_val = px_null();
    int px_err_916_proped = 0;
    px_srcline(280);
    if (px_is_truthy(px_eq(_v914, px_null()))) {
        px_srcline(281);
        return _v915;
    }
    px_srcline(282);
    return px_add(px_add(px_add(px_str("({ "), _v914), _v915), px_str("; })"));
px_err_916:
    if (px_err_916_proped) return px_err_916_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_expr");
    LXValue _v917 = (nargs > 0) ? args[0] : px_null();
    LXValue _v918 = px_uninit();
    LXValue _v919 = px_uninit();
    LXValue _v920 = px_uninit();
    LXValue _v921 = px_uninit();
    LXValue _v922 = px_uninit();
    LXValue _v923 = px_uninit();
    LXValue _v924 = px_uninit();
    LXValue _v925 = px_uninit();
    LXValue _v926 = px_uninit();
    LXValue _v927 = px_uninit();
    LXValue _v928 = px_uninit();
    LXValue _v929 = px_uninit();
    LXValue _v930 = px_uninit();
    LXValue _v931 = px_uninit();
    LXValue _v932 = px_uninit();
    LXValue _v933 = px_uninit();
    LXValue _v934 = px_uninit();
    LXValue _v935 = px_uninit();
    LXValue _v936 = px_uninit();
    LXValue _v937 = px_uninit();
    LXValue _v938 = px_uninit();
    LXValue _v939 = px_uninit();
    LXValue _v940 = px_uninit();
    LXValue _v941 = px_uninit();
    LXValue _v942 = px_uninit();
    LXValue _v943 = px_uninit();
    LXValue _v944 = px_uninit();
    LXValue _v945 = px_uninit();
    LXValue _v946 = px_uninit();
    LXValue _v947 = px_uninit();
    LXValue _v948 = px_uninit();
    LXValue _v949 = px_uninit();
    LXValue _v950 = px_uninit();
    LXValue _v951 = px_uninit();
    LXValue _v952 = px_uninit();
    LXValue _v953 = px_uninit();
    LXValue _v954 = px_uninit();
    LXValue _v955 = px_uninit();
    LXValue _v956 = px_uninit();
    LXValue _v957 = px_uninit();
    LXValue _v958 = px_uninit();
    LXValue _v959 = px_uninit();
    LXValue _v960 = px_uninit();
    LXValue _v961 = px_uninit();
    LXValue _v962 = px_uninit();
    LXValue _v963 = px_uninit();
    LXValue _v964 = px_uninit();
    LXValue _v965 = px_uninit();
    LXValue _v966 = px_uninit();
    LXValue _v967 = px_uninit();
    LXValue _v968 = px_uninit();
    LXValue _v969 = px_uninit();
    LXValue _v970 = px_uninit();
    LXValue _v971 = px_uninit();
    LXValue _v972 = px_uninit();
    LXValue _v973 = px_uninit();
    LXValue _v974 = px_uninit();
    LXValue _v975 = px_uninit();
    LXValue _v976 = px_uninit();
    LXValue _v977 = px_uninit();
    LXValue _v978 = px_uninit();
    LXValue px_err_979_val = px_null();
    int px_err_979_proped = 0;
    px_srcline(284);
    _v918 = px_index(_v917, px_int(0LL));
    px_srcline(285);
    if (px_is_truthy(px_eq(_v918, px_str("Int")))) {
        px_srcline(286);
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1)), px_str("LL)"));
    }
    px_srcline(287);
    if (px_is_truthy(px_eq(_v918, px_str("Float")))) {
        px_srcline(288);
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1)), px_str(")"));
    }
    px_srcline(289);
    if (px_is_truthy(px_eq(_v918, px_str("Str")))) {
        px_srcline(290);
        _v919 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(294);
        if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){_v919}, 1))) {
            px_srcline(295);
            return px_add(px_add(px_str("PX_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v919}, 1)), px_str("\")"));
        }
        px_srcline(296);
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v919}, 1)), px_str("\")"));
    }
    px_srcline(297);
    if (px_is_truthy(px_eq(_v918, px_str("Bool")))) {
        px_srcline(298);
        if (px_is_truthy(px_index(_v917, px_int(1LL)))) {
            px_srcline(299);
            return px_str("px_bool(true)");
        }
        px_srcline(300);
        return px_str("px_bool(false)");
    }
    px_srcline(301);
    if (px_is_truthy(px_eq(_v918, px_str("Null")))) {
        px_srcline(302);
        return px_str("px_null()");
    }
    px_srcline(303);
    if (px_is_truthy(px_eq(_v918, px_str("List")))) {
        px_srcline(304);
        _v920 = px_list_n((LXValue[]){}, 0);
        px_srcline(305);
        _v921 = px_index(_v917, px_int(1LL));
        px_srcline(306);
        _v922 = px_int(0LL);
        px_srcline(307);
        while (px_is_truthy(px_lt(_v922, px_call(px_get_global("len"), (LXValue[]){_v921}, 1)))) {
            px_srcline(308);
            (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v921, _v922)}, 1)}, 1));
            px_srcline(309);
             _v922 = px_add(_v922, px_int(1LL));
        }
        px_srcline(310);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v921, _v920}, 2);
        px_srcline(311);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s153 = px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s154 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v920}, 1)}, 1); px_add(_s153, _s154); }), px_str(")"))}, 2);
    }
    px_srcline(312);
    if (px_is_truthy(px_eq(_v918, px_str("Tuple")))) {
        px_srcline(313);
        _v920 = px_list_n((LXValue[]){}, 0);
        px_srcline(314);
        _v921 = px_index(_v917, px_int(1LL));
        px_srcline(315);
        _v922 = px_int(0LL);
        px_srcline(316);
        while (px_is_truthy(px_lt(_v922, px_call(px_get_global("len"), (LXValue[]){_v921}, 1)))) {
            px_srcline(317);
            (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v921, _v922)}, 1)}, 1));
            px_srcline(318);
             _v922 = px_add(_v922, px_int(1LL));
        }
        px_srcline(319);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v921, _v920}, 2);
        px_srcline(320);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s155 = px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s156 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v920}, 1)}, 1); px_add(_s155, _s156); }), px_str(")"))}, 2);
    }
    px_srcline(321);
    if (px_is_truthy(px_eq(_v918, px_str("Dict")))) {
        px_srcline(322);
        _v924 = px_str("({ LXValue _d = px_dict(); ");
        px_srcline(323);
        _v925 = px_index(_v917, px_int(1LL));
        px_srcline(324);
        _v922 = px_int(0LL);
        px_srcline(325);
        while (px_is_truthy(px_lt(_v922, px_call(px_get_global("len"), (LXValue[]){_v925}, 1)))) {
            px_srcline(326);
            _v926 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v925, _v922), px_int(0LL))}, 1);
            px_srcline(327);
            _v927 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v925, _v922), px_int(1LL))}, 1);
            px_srcline(332);
             _v924 = px_add(_v924, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v926), px_str("; LXValue _v = ")), _v927), px_str("; px_dict_set_checked(_d, _k, _v); } ")));
            px_srcline(333);
             _v922 = px_add(_v922, px_int(1LL));
        }
        px_srcline(334);
         _v924 = px_add(_v924, px_str("_d; })"));
        px_srcline(335);
        return _v924;
    }
    px_srcline(336);
    if (px_is_truthy(px_eq(_v918, px_str("Var")))) {
        px_srcline(337);
        _v928 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(338);
        _v929 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v928}, 1);
        px_srcline(339);
        if (px_is_truthy(px_ne(_v929, px_null()))) {
            px_srcline(342);
            return px_call(px_get_global("cg_load_ck"), (LXValue[]){_v928, _v929}, 2);
        }
        px_srcline(343);
        return px_add(px_add(px_str("px_get_global(\""), _v928), px_str("\")"));
    }
    px_srcline(344);
    if (px_is_truthy(px_eq(_v918, px_str("Field")))) {
        px_srcline(345);
        _v930 = px_index(_v917, px_int(1LL));
        px_srcline(346);
        _v931 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(348);
        if (px_is_truthy(px_eq(px_index(_v930, px_int(0LL)), px_str("Var")))) {
            px_srcline(349);
            _v932 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v930, px_int(1LL))}, 1);
            px_srcline(350);
            if (px_is_truthy(({ LXValue _t980 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v932}, 1); px_is_truthy(_t980) ? px_method(px_index(px_get_global("cg_const_enums"), _v932), "has", (LXValue[]){_v931}, 1) : _t980; }))) {
                px_srcline(351);
                return px_index(px_index(px_get_global("cg_const_enums"), _v932), _v931);
            }
        }
        px_srcline(353);
        if (px_is_truthy(px_eq(px_index(_v930, px_int(0LL)), px_str("Var")))) {
            px_srcline(354);
            _v932 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v930, px_int(1LL))}, 1);
            px_srcline(355);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v932}, 1))) {
                px_srcline(356);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v932), px_str("\", \"")), _v931), px_str("\")"));
            }
        }
        px_srcline(357);
        _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v930}, 1);
        px_srcline(358);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v933), px_str(", \"")), _v931), px_str("\")"));
    }
    px_srcline(359);
    if (px_is_truthy(px_eq(_v918, px_str("OptionalField")))) {
        px_srcline(360);
        _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(361);
        _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(362);
        _v931 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(363);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v933), px_str("; px_is_null(")), _v934), px_str(") ? px_null() : px_field(")), _v933), px_str(", \"")), _v931), px_str("\"); })"));
    }
    px_srcline(364);
    if (px_is_truthy(px_eq(_v918, px_str("Index")))) {
        px_srcline(365);
        _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(366);
        _v922 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(367);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v917, px_int(1LL)), px_index(_v917, px_int(2LL))}, 2), px_list_n((LXValue[]){_v933, _v922}, 2)}, 2);
        px_srcline(368);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_index("), px_index(px_index(_v923, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v923, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2);
    }
    px_srcline(369);
    if (px_is_truthy(px_eq(_v918, px_str("Slice")))) {
        px_srcline(370);
        _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(371);
        _v924 = px_str("px_null()");
        px_srcline(372);
        if (px_is_truthy(px_ne(px_index(_v917, px_int(2LL)), px_null()))) {
            px_srcline(373);
             _v924 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        }
        px_srcline(374);
        _v935 = px_str("px_null()");
        px_srcline(375);
        if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
            px_srcline(376);
             _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        }
        px_srcline(377);
        _v936 = px_str("px_null()");
        px_srcline(378);
        if (px_is_truthy(px_ne(px_index(_v917, px_int(4LL)), px_null()))) {
            px_srcline(379);
             _v936 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(4LL))}, 1);
        }
        px_srcline(380);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v917, px_int(1LL)), px_index(_v917, px_int(2LL)), px_index(_v917, px_int(3LL)), px_index(_v917, px_int(4LL))}, 4), px_list_n((LXValue[]){_v933, _v924, _v935, _v936}, 4)}, 2);
        px_srcline(381);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), px_index(px_index(_v923, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v923, px_int(1LL)), px_int(1LL))), px_str(", ")), px_index(px_index(_v923, px_int(1LL)), px_int(2LL))), px_str(", ")), px_index(px_index(_v923, px_int(1LL)), px_int(3LL))), px_str(")"))}, 2);
    }
    px_srcline(382);
    if (px_is_truthy(px_eq(_v918, px_str("Call")))) {
        px_srcline(383);
        _v937 = px_index(_v917, px_int(1LL));
        px_srcline(384);
        _v938 = px_index(_v917, px_int(2LL));
        px_srcline(385);
        if (px_is_truthy(px_eq(px_index(_v937, px_int(0LL)), px_str("Var")))) {
            px_srcline(386);
            _v939 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v937, px_int(1LL))}, 1);
            px_srcline(389);
            (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v937, _v938}, 2));
            px_srcline(390);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v939}, 1))) {
                px_srcline(391);
                _v940 = px_index(px_get_global("cg_ffi"), _v939);
                px_srcline(392);
                _v920 = px_list_n((LXValue[]){}, 0);
                px_srcline(393);
                _v941 = px_int(0LL);
                px_srcline(394);
                while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
                    px_srcline(395);
                    (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
                    px_srcline(396);
                     _v941 = px_add(_v941, px_int(1LL));
                }
                px_srcline(397);
                _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v938, _v920}, 2);
                px_srcline(398);
                return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s157 = px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v939), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s158 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v920}, 1)}, 1); px_add(_s157, _s158); }), px_str(")}, 2)"))}, 2);
            }
            px_srcline(399);
            if (px_is_truthy(px_eq(_v939, px_str("chan")))) {
                px_srcline(400);
                _v942 = px_str("0");
                px_srcline(401);
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v938}, 1), px_int(0LL)))) {
                    px_srcline(402);
                    if (px_is_truthy(px_eq(px_index(px_index(_v938, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                        px_srcline(403);
                         _v942 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v938, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                        px_srcline(405);
                         _v942 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                px_srcline(406);
                return px_add(px_add(px_str("px_chan_create("), _v942), px_str(")"));
            }
            px_srcline(407);
            if (px_is_truthy(px_eq(_v939, px_str("mutex")))) {
                px_srcline(408);
                return px_str("px_mutex_create()");
            }
            px_srcline(409);
            if (px_is_truthy(px_eq(_v939, px_str("rwlock")))) {
                px_srcline(410);
                return px_str("px_rwlock_create()");
            }
            px_srcline(412);
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v939}, 1))) {
                px_srcline(413);
                _v943 = px_index(px_get_global("cg_structs"), _v939);
                px_srcline(414);
                if (px_is_truthy(({ LXValue _s159 = px_call(px_get_global("len"), (LXValue[]){_v943}, 1); LXValue _s160 = px_call(px_get_global("len"), (LXValue[]){_v938}, 1); px_ne(_s159, _s160); }))) {
                    px_srcline(415);
                    return ({ LXValue _s161 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v939), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v943}, 1)}, 1)), px_str(" 个字段，给出 ")); LXValue _s162 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v938}, 1)}, 1); px_add(_s161, _s162); });
                }
                px_srcline(416);
                _v920 = px_list_n((LXValue[]){}, 0);
                px_srcline(417);
                _v941 = px_int(0LL);
                px_srcline(418);
                while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
                    px_srcline(419);
                    (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
                    px_srcline(420);
                     _v941 = px_add(_v941, px_int(1LL));
                }
                px_srcline(421);
                _v944 = px_list_n((LXValue[]){}, 0);
                px_srcline(422);
                _v945 = px_int(0LL);
                px_srcline(423);
                while (px_is_truthy(px_lt(_v945, px_call(px_get_global("len"), (LXValue[]){_v943}, 1)))) {
                    px_srcline(424);
                    (void)(px_method(_v944, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v943, _v945)), px_str("\""))}, 1));
                    px_srcline(425);
                     _v945 = px_add(_v945, px_int(1LL));
                }
                px_srcline(426);
                _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v938, _v920}, 2);
                px_srcline(427);
                return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s165 = px_add(({ LXValue _s163 = px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v939), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v944}, 2)), px_str("}, (LXValue[]){")); LXValue _s164 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2); px_add(_s163, _s164); }), px_str("}, ")); LXValue _s166 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v943}, 1)}, 1); px_add(_s165, _s166); }), px_str(")"))}, 2);
            }
            px_srcline(428);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v939}, 1))) {
                px_srcline(429);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v938}, 1), px_int(1LL)))) {
                    px_srcline(430);
                    return px_add(px_add(px_str("枚举 "), _v939), px_str(" 构造需要一个变体名"));
                }
                px_srcline(431);
                _v929 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, px_int(0LL))}, 1);
                px_srcline(432);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v939), px_str("\", (")), _v929), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        px_srcline(434);
        if (px_is_truthy(px_eq(px_index(_v937, px_int(0LL)), px_str("Field")))) {
            px_srcline(435);
            _v930 = px_index(_v937, px_int(1LL));
            px_srcline(436);
            _v946 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v937, px_int(2LL))}, 1);
            px_srcline(438);
            _v947 = px_null();
            px_srcline(439);
            if (px_is_truthy(px_eq(px_index(_v930, px_int(0LL)), px_str("Var")))) {
                px_srcline(440);
                _v932 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v930, px_int(1LL))}, 1);
                px_srcline(441);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v932}, 1))) {
                    px_srcline(442);
                     _v947 = px_index(px_get_global("cg_var_types"), _v932);
                }
            }
            px_srcline(443);
            if (px_is_truthy(({ LXValue _t981 = px_ne(_v947, px_null()); px_is_truthy(_t981) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v947}, 1) : _t981; }))) {
                px_srcline(444);
                _v948 = px_index(px_get_global("cg_impls"), _v947);
                px_srcline(445);
                _v949 = px_bool(false);
                px_srcline(446);
                _v950 = px_int(0LL);
                px_srcline(447);
                while (px_is_truthy(px_lt(_v950, px_call(px_get_global("len"), (LXValue[]){_v948}, 1)))) {
                    px_srcline(448);
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v948, _v950), px_int(1LL))}, 1), _v946))) {
                        px_srcline(449);
                         _v949 = px_bool(true);
                        px_srcline(450);
                        break;
                    }
                    px_srcline(451);
                     _v950 = px_add(_v950, px_int(1LL));
                }
                px_srcline(452);
                if (px_is_truthy(_v949)) {
                    px_srcline(453);
                    _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v930}, 1);
                    px_srcline(454);
                    _v920 = px_list_n((LXValue[]){_v933}, 1);
                    px_srcline(455);
                    _v951 = px_list_n((LXValue[]){_v930}, 1);
                    px_srcline(456);
                    _v941 = px_int(0LL);
                    px_srcline(457);
                    while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
                        px_srcline(458);
                        (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
                        px_srcline(459);
                        (void)(px_method(_v951, "append", (LXValue[]){px_index(_v938, _v941)}, 1));
                        px_srcline(460);
                         _v941 = px_add(_v941, px_int(1LL));
                    }
                    px_srcline(461);
                    _v952 = ({ LXValue _s167 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v947}, 1)), px_str("_")); LXValue _s168 = px_call(px_get_global("cg_func_cname"), (LXValue[]){_v946}, 1); px_add(_s167, _s168); });
                    px_srcline(462);
                    _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v951, _v920}, 2);
                    px_srcline(463);
                    return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s169 = px_add(px_add(px_add(_v952, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s170 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v920}, 1)}, 1); px_add(_s169, _s170); }), px_str(", NULL)"))}, 2);
                }
            }
            px_srcline(467);
            _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v930}, 1);
            px_srcline(468);
            _v920 = px_list_n((LXValue[]){_v933}, 1);
            px_srcline(469);
            _v951 = px_list_n((LXValue[]){_v930}, 1);
            px_srcline(470);
            _v941 = px_int(0LL);
            px_srcline(471);
            while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
                px_srcline(472);
                (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
                px_srcline(473);
                (void)(px_method(_v951, "append", (LXValue[]){px_index(_v938, _v941)}, 1));
                px_srcline(474);
                 _v941 = px_add(_v941, px_int(1LL));
            }
            px_srcline(475);
            _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v951, _v920}, 2);
            px_srcline(476);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s171 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), px_index(px_index(_v923, px_int(1LL)), px_int(0LL))), px_str(", \"")), _v946), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_slice(px_index(_v923, px_int(1LL)), px_int(1LL), px_null(), px_null())}, 2)), px_str("}, ")); LXValue _s172 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v938}, 1)}, 1); px_add(_s171, _s172); }), px_str(")"))}, 2);
        }
        px_srcline(480);
        _v953 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v937}, 1);
        px_srcline(481);
        _v920 = px_list_n((LXValue[]){_v953}, 1);
        px_srcline(482);
        _v951 = px_list_n((LXValue[]){_v937}, 1);
        px_srcline(483);
        _v941 = px_int(0LL);
        px_srcline(484);
        while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
            px_srcline(485);
            (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
            px_srcline(486);
            (void)(px_method(_v951, "append", (LXValue[]){px_index(_v938, _v941)}, 1));
            px_srcline(487);
             _v941 = px_add(_v941, px_int(1LL));
        }
        px_srcline(488);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v951, _v920}, 2);
        px_srcline(489);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s173 = px_add(px_add(px_add(px_add(px_str("px_call("), px_index(px_index(_v923, px_int(1LL)), px_int(0LL))), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_slice(px_index(_v923, px_int(1LL)), px_int(1LL), px_null(), px_null())}, 2)), px_str("}, ")); LXValue _s174 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v938}, 1)}, 1); px_add(_s173, _s174); }), px_str(")"))}, 2);
    }
    px_srcline(490);
    if (px_is_truthy(px_eq(_v918, px_str("Unary")))) {
        px_srcline(491);
        _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(492);
        _v954 = px_index(_v917, px_int(1LL));
        px_srcline(493);
        if (px_is_truthy(px_eq(_v954, px_str("Neg")))) {
            px_srcline(494);
            return px_add(px_add(px_str("px_neg("), _v933), px_str(")"));
        }
        px_srcline(495);
        if (px_is_truthy(px_eq(_v954, px_str("Not")))) {
            px_srcline(496);
            return px_add(px_add(px_str("px_not("), _v933), px_str(")"));
        }
        px_srcline(497);
        return px_add(px_add(px_str("px_bitnot("), _v933), px_str(")"));
    }
    px_srcline(498);
    if (px_is_truthy(px_eq(_v918, px_str("Binary")))) {
        px_srcline(499);
        _v955 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(500);
        _v956 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        px_srcline(501);
        _v954 = px_index(_v917, px_int(1LL));
        px_srcline(502);
        if (px_is_truthy(px_eq(_v954, px_str("And")))) {
            px_srcline(503);
            _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(504);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v955), px_str("; px_is_truthy(")), _v934), px_str(") ? ")), _v956), px_str(" : ")), _v934), px_str("; })"));
        }
        px_srcline(505);
        if (px_is_truthy(px_eq(_v954, px_str("Or")))) {
            px_srcline(506);
            _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(507);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v955), px_str("; px_is_truthy(")), _v934), px_str(") ? ")), _v934), px_str(" : ")), _v956), px_str("; })"));
        }
        px_srcline(508);
        _v957 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v954}, 1);
        px_srcline(509);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v917, px_int(2LL)), px_index(_v917, px_int(3LL))}, 2), px_list_n((LXValue[]){_v955, _v956}, 2)}, 2);
        px_srcline(510);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(_v957, px_str("(")), px_index(px_index(_v923, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v923, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2);
    }
    px_srcline(511);
    if (px_is_truthy(px_eq(_v918, px_str("Pipe")))) {
        px_srcline(512);
        _v929 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(513);
        _v958 = px_index(_v917, px_int(2LL));
        px_srcline(514);
        if (px_is_truthy(px_eq(px_index(_v958, px_int(0LL)), px_str("Call")))) {
            px_srcline(515);
            _v953 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v958, px_int(1LL))}, 1);
            px_srcline(516);
            _v920 = px_list_n((LXValue[]){_v929}, 1);
            px_srcline(517);
            _v951 = px_list_n((LXValue[]){px_index(_v917, px_int(1LL))}, 1);
            px_srcline(518);
            _v941 = px_int(0LL);
            px_srcline(519);
            while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){px_index(_v958, px_int(2LL))}, 1)))) {
                px_srcline(520);
                (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v958, px_int(2LL)), _v941)}, 1)}, 1));
                px_srcline(521);
                (void)(px_method(_v951, "append", (LXValue[]){px_index(px_index(_v958, px_int(2LL)), _v941)}, 1));
                px_srcline(522);
                 _v941 = px_add(_v941, px_int(1LL));
            }
            px_srcline(523);
            _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v951, _v920}, 2);
            px_srcline(524);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s175 = px_add(px_add(px_add(px_add(px_str("px_call("), _v953), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s176 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v920}, 1)}, 1); px_add(_s175, _s176); }), px_str(")"))}, 2);
        }
        px_srcline(525);
        _v957 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v958}, 1);
        px_srcline(526);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v917, px_int(1LL)), px_index(_v917, px_int(2LL))}, 2), px_list_n((LXValue[]){_v929, _v957}, 2)}, 2);
        px_srcline(527);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_call("), px_index(px_index(_v923, px_int(1LL)), px_int(1LL))), px_str(", (LXValue[]){")), px_index(px_index(_v923, px_int(1LL)), px_int(0LL))), px_str("}, 1)"))}, 2);
    }
    px_srcline(528);
    if (px_is_truthy(px_eq(_v918, px_str("NullCoalesce")))) {
        px_srcline(529);
        _v955 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(530);
        _v956 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(531);
        _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(532);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v955), px_str("; px_is_null(")), _v934), px_str(") ? ")), _v956), px_str(" : ")), _v934), px_str("; })"));
    }
    px_srcline(533);
    if (px_is_truthy(px_eq(_v918, px_str("Try")))) {
        px_srcline(534);
        _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(535);
        _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(536);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            px_srcline(537);
            _v959 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            px_srcline(538);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v935), px_str("; if (px_is_result(")), _v934), px_str(")) { if (!px_result_ok(")), _v934), px_str(")) { ")), _v959), px_str("_val = ")), _v934), px_str("; ")), _v959), px_str("_proped = 1; goto ")), _v959), px_str("; } ")), _v934), px_str(" = px_result_unwrap(")), _v934), px_str("); } else if (px_is_null(")), _v934), px_str(")) { ")), _v959), px_str("_val = px_null(); ")), _v959), px_str("_proped = 1; goto ")), _v959), px_str("; } ")), _v934), px_str("; })"));
        }
        px_srcline(539);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v935), px_str("; if (px_is_result(")), _v934), px_str(") && !px_result_ok(")), _v934), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 Err\"); if (px_is_null(")), _v934), px_str(")) px_error(\"错误传播 ?: 顶层不能传播 null\"); if (px_is_result(")), _v934), px_str(")) ")), _v934), px_str(" = px_result_unwrap(")), _v934), px_str("); ")), _v934), px_str("; })"));
    }
    px_srcline(540);
    if (px_is_truthy(px_eq(_v918, px_str("ForceUnwrap")))) {
        px_srcline(541);
        _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(542);
        _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(543);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v935), px_str("; if (px_is_result(")), _v934), px_str(")) { if (!px_result_ok(")), _v934), px_str(")) px_error(\"force unwrap Err\"); ")), _v934), px_str(" = px_result_unwrap(")), _v934), px_str("); } if (px_is_null(")), _v934), px_str(")) px_error(\"force unwrap null\"); ")), _v934), px_str("; })"));
    }
    px_srcline(544);
    if (px_is_truthy(px_eq(_v918, px_str("IfExpr")))) {
        px_srcline(545);
        _v953 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(546);
        _v960 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(547);
        _v961 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        px_srcline(548);
        _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(549);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str("; if (px_is_truthy(")), _v953), px_str(")) { ")), _v934), px_str(" = ")), _v960), px_str("; } else { ")), _v934), px_str(" = ")), _v961), px_str("; } ")), _v934), px_str("; })"));
    }
    px_srcline(550);
    if (px_is_truthy(px_eq(_v918, px_str("ListComp")))) {
        px_srcline(551);
        _v962 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(552);
        _v956 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(553);
        _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(554);
        _v963 = px_null();
        px_srcline(555);
        if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
            px_srcline(556);
             _v963 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        }
        px_srcline(557);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v956, px_str("saved_all"))}, 1));
        px_srcline(558);
        _v964 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v962), px_str(", ")), _v935), px_str("); "));
        px_srcline(559);
        _v965 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v956, _v963, _v964}, 3);
        px_srcline(560);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v962), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v956, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v956, px_str("its")), px_int(0LL))), px_str("; ")), _v965), px_str(" ")), _v962), px_str("; })"));
    }
    px_srcline(561);
    if (px_is_truthy(px_eq(_v918, px_str("GenExp")))) {
        px_srcline(562);
        _v966 = px_index(_v917, px_int(2LL));
        px_srcline(563);
        if (px_is_truthy(({ LXValue _t982 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v966}, 1), px_int(1LL)); px_is_truthy(_t982) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v966, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t982; }))) {
            px_srcline(564);
            _v967 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v966, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(565);
            _v968 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v966, px_int(0LL)), px_int(2LL))}, 1);
            px_srcline(566);
            _v969 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v967}, 1), px_index(_v917, px_int(1LL))}, 2);
            px_srcline(567);
            _v970 = px_str("px_null()");
            px_srcline(568);
            if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
                px_srcline(569);
                 _v970 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v967}, 1), px_index(_v917, px_int(3LL))}, 2);
            }
            px_srcline(570);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v968), px_str(", ")), _v969), px_str(", ")), _v970), px_str(")"));
        }
        px_srcline(572);
        _v962 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(573);
        _v956 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v966}, 1);
        px_srcline(574);
        _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(575);
        _v963 = px_null();
        px_srcline(576);
        if (px_is_truthy(px_ne(px_index(_v917, px_int(3LL)), px_null()))) {
            px_srcline(577);
             _v963 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        }
        px_srcline(578);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v956, px_str("saved_all"))}, 1));
        px_srcline(579);
        _v964 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v962), px_str(", ")), _v935), px_str("); "));
        px_srcline(580);
        _v965 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v956, _v963, _v964}, 3);
        px_srcline(581);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v962), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v956, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v956, px_str("its")), px_int(0LL))), px_str("; ")), _v965), px_str(" px_gen_from_list(")), _v962), px_str("); })"));
    }
    px_srcline(582);
    if (px_is_truthy(px_eq(_v918, px_str("DictComp")))) {
        px_srcline(583);
        _v962 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(584);
        _v956 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v917, px_int(3LL))}, 1);
        px_srcline(585);
        _v926 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(586);
        _v927 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(2LL))}, 1);
        px_srcline(587);
        _v963 = px_null();
        px_srcline(588);
        if (px_is_truthy(px_ne(px_index(_v917, px_int(4LL)), px_null()))) {
            px_srcline(589);
             _v963 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(4LL))}, 1);
        }
        px_srcline(590);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v956, px_str("saved_all"))}, 1));
        px_srcline(591);
        _v964 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v926), px_str("; LXValue _v = ")), _v927), px_str("; px_dict_set_checked(")), _v962), px_str(", _k, _v); } "));
        px_srcline(592);
        _v965 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v956, _v963, _v964}, 3);
        px_srcline(593);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v962), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v956, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v956, px_str("its")), px_int(0LL))), px_str("; ")), _v965), px_str(" ")), _v962), px_str("; })"));
    }
    px_srcline(594);
    if (px_is_truthy(px_eq(_v918, px_str("Closure")))) {
        px_srcline(597);
        return px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v917, px_int(1LL)), px_index(_v917, px_int(3LL)), px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_add(px_get_global("cg_closure_id"), px_int(1LL))}, 1)), px_str(">"))}, 3);
    }
    px_srcline(598);
    if (px_is_truthy(px_eq(_v918, px_str("Block")))) {
        px_srcline(599);
        _v924 = px_str("({ ");
        px_srcline(600);
         _v924 = px_add(_v924, px_str("LXValue _blk = px_null(); "));
        px_srcline(601);
        _v971 = px_index(_v917, px_int(1LL));
        px_srcline(602);
        _v972 = px_int(0LL);
        px_srcline(603);
        while (px_is_truthy(px_lt(_v972, px_call(px_get_global("len"), (LXValue[]){_v971}, 1)))) {
            px_srcline(604);
            _v973 = px_index(_v971, _v972);
            px_srcline(605);
            if (px_is_truthy(px_eq(px_index(_v973, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(606);
                _v935 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v973, px_int(1LL))}, 1);
                px_srcline(607);
                 _v924 = px_add(_v924, px_add(px_add(px_str("_blk = "), _v935), px_str("; ")));
            }
            else {
                px_srcline(609);
                 _v924 = px_add(_v924, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v973, px_int(0LL)}, 2));
            }
            px_srcline(610);
             _v972 = px_add(_v972, px_int(1LL));
        }
        px_srcline(611);
         _v924 = px_add(_v924, px_str("_blk; })"));
        px_srcline(612);
        return _v924;
    }
    px_srcline(613);
    if (px_is_truthy(px_eq(_v918, px_str("Match")))) {
        px_srcline(614);
        _v974 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(615);
        _v934 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(616);
        _v924 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v934), px_str(" = ")), _v974), px_str("; "));
        px_srcline(617);
        _v975 = px_index(_v917, px_int(2LL));
        px_srcline(618);
        _v976 = px_bool(true);
        px_srcline(619);
        _v941 = px_int(0LL);
        px_srcline(620);
        while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v975}, 1)))) {
            px_srcline(621);
            _v963 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v975, _v941), px_int(1LL)), _v934}, 2);
            px_srcline(622);
            _v977 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v975, _v941), px_int(3LL))}, 1);
            px_srcline(623);
            _v978 = px_str("if");
            px_srcline(624);
            if (px_is_truthy(px_not(_v976))) {
                px_srcline(625);
                 _v978 = px_str("else if");
            }
            px_srcline(626);
             _v924 = px_add(_v924, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v978, px_str(" (")), _v963), px_str(") { ")), _v934), px_str(" = ")), _v977), px_str("; } ")));
            px_srcline(627);
             _v976 = px_bool(false);
            px_srcline(628);
             _v941 = px_add(_v941, px_int(1LL));
        }
        px_srcline(629);
         _v924 = px_add(_v924, px_add(_v934, px_str("; })")));
        px_srcline(630);
        return _v924;
    }
    px_srcline(631);
    if (px_is_truthy(px_eq(_v918, px_str("Constructor")))) {
        px_srcline(632);
        _v928 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v917, px_int(1LL))}, 1);
        px_srcline(633);
        _v938 = px_index(_v917, px_int(2LL));
        px_srcline(634);
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v928}, 1))) {
            px_srcline(635);
            _v943 = px_index(px_get_global("cg_structs"), _v928);
            px_srcline(636);
            if (px_is_truthy(({ LXValue _s177 = px_call(px_get_global("len"), (LXValue[]){_v943}, 1); LXValue _s178 = px_call(px_get_global("len"), (LXValue[]){_v938}, 1); px_ne(_s177, _s178); }))) {
                px_srcline(637);
                return ({ LXValue _s179 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v928), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v943}, 1)}, 1)), px_str(" 个字段，给出 ")); LXValue _s180 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v938}, 1)}, 1); px_add(_s179, _s180); });
            }
            px_srcline(638);
            _v920 = px_list_n((LXValue[]){}, 0);
            px_srcline(639);
            _v941 = px_int(0LL);
            px_srcline(640);
            while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
                px_srcline(641);
                (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
                px_srcline(642);
                 _v941 = px_add(_v941, px_int(1LL));
            }
            px_srcline(643);
            _v944 = px_list_n((LXValue[]){}, 0);
            px_srcline(644);
            _v945 = px_int(0LL);
            px_srcline(645);
            while (px_is_truthy(px_lt(_v945, px_call(px_get_global("len"), (LXValue[]){_v943}, 1)))) {
                px_srcline(646);
                (void)(px_method(_v944, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v943, _v945)), px_str("\""))}, 1));
                px_srcline(647);
                 _v945 = px_add(_v945, px_int(1LL));
            }
            px_srcline(648);
            _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v938, _v920}, 2);
            px_srcline(649);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s183 = px_add(({ LXValue _s181 = px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v928), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v944}, 2)), px_str("}, (LXValue[]){")); LXValue _s182 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2); px_add(_s181, _s182); }), px_str("}, ")); LXValue _s184 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v943}, 1)}, 1); px_add(_s183, _s184); }), px_str(")"))}, 2);
        }
        px_srcline(650);
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v928}, 1))) {
            px_srcline(651);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v938}, 1), px_int(1LL)))) {
                px_srcline(652);
                return px_add(px_add(px_str("枚举 "), _v928), px_str(" 构造需要一个变体名"));
            }
            px_srcline(653);
            _v929 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, px_int(0LL))}, 1);
            px_srcline(654);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v928), px_str("\", (")), _v929), px_str(").as.obj->as.enum_inst.variant)"));
        }
        px_srcline(655);
        _v920 = px_list_n((LXValue[]){}, 0);
        px_srcline(656);
        _v941 = px_int(0LL);
        px_srcline(657);
        while (px_is_truthy(px_lt(_v941, px_call(px_get_global("len"), (LXValue[]){_v938}, 1)))) {
            px_srcline(658);
            (void)(px_method(_v920, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v938, _v941)}, 1)}, 1));
            px_srcline(659);
             _v941 = px_add(_v941, px_int(1LL));
        }
        px_srcline(660);
        _v923 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v938, _v920}, 2);
        px_srcline(661);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v923, px_int(0LL)), px_add(({ LXValue _s185 = px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v928), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v923, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s186 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v920}, 1)}, 1); px_add(_s185, _s186); }), px_str(")"))}, 2);
    }
    px_srcline(662);
    return px_str("px_null()");
px_err_979:
    if (px_err_979_proped) return px_err_979_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_binop_cname");
    LXValue _v983 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_984_val = px_null();
    int px_err_984_proped = 0;
    px_srcline(665);
    if (px_is_truthy(px_eq(_v983, px_str("Add")))) {
        px_srcline(666);
        return px_str("px_add");
    }
    px_srcline(667);
    if (px_is_truthy(px_eq(_v983, px_str("Sub")))) {
        px_srcline(668);
        return px_str("px_sub");
    }
    px_srcline(669);
    if (px_is_truthy(px_eq(_v983, px_str("Mul")))) {
        px_srcline(670);
        return px_str("px_mul");
    }
    px_srcline(671);
    if (px_is_truthy(px_eq(_v983, px_str("Div")))) {
        px_srcline(672);
        return px_str("px_div");
    }
    px_srcline(673);
    if (px_is_truthy(px_eq(_v983, px_str("IntDiv")))) {
        px_srcline(674);
        return px_str("px_idiv");
    }
    px_srcline(675);
    if (px_is_truthy(px_eq(_v983, px_str("Mod")))) {
        px_srcline(676);
        return px_str("px_mod");
    }
    px_srcline(677);
    if (px_is_truthy(px_eq(_v983, px_str("Pow")))) {
        px_srcline(678);
        return px_str("px_pow");
    }
    px_srcline(679);
    if (px_is_truthy(px_eq(_v983, px_str("Eq")))) {
        px_srcline(680);
        return px_str("px_eq");
    }
    px_srcline(681);
    if (px_is_truthy(px_eq(_v983, px_str("Ne")))) {
        px_srcline(682);
        return px_str("px_ne");
    }
    px_srcline(683);
    if (px_is_truthy(px_eq(_v983, px_str("Lt")))) {
        px_srcline(684);
        return px_str("px_lt");
    }
    px_srcline(685);
    if (px_is_truthy(px_eq(_v983, px_str("Le")))) {
        px_srcline(686);
        return px_str("px_le");
    }
    px_srcline(687);
    if (px_is_truthy(px_eq(_v983, px_str("Gt")))) {
        px_srcline(688);
        return px_str("px_gt");
    }
    px_srcline(689);
    if (px_is_truthy(px_eq(_v983, px_str("Ge")))) {
        px_srcline(690);
        return px_str("px_ge");
    }
    px_srcline(691);
    if (px_is_truthy(px_eq(_v983, px_str("BitAnd")))) {
        px_srcline(692);
        return px_str("px_bitand");
    }
    px_srcline(693);
    if (px_is_truthy(px_eq(_v983, px_str("BitOr")))) {
        px_srcline(694);
        return px_str("px_bitor");
    }
    px_srcline(695);
    if (px_is_truthy(px_eq(_v983, px_str("BitXor")))) {
        px_srcline(696);
        return px_str("px_bitxor");
    }
    px_srcline(697);
    if (px_is_truthy(px_eq(_v983, px_str("Shl")))) {
        px_srcline(698);
        return px_str("px_shl");
    }
    px_srcline(699);
    if (px_is_truthy(px_eq(_v983, px_str("Shr")))) {
        px_srcline(700);
        return px_str("px_shr");
    }
    px_srcline(701);
    if (px_is_truthy(px_eq(_v983, px_str("ShrU")))) {
        px_srcline(702);
        return px_str("px_ushr");
    }
    px_srcline(703);
    return px_str("px_add");
px_err_984:
    if (px_err_984_proped) return px_err_984_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_pattern_cond");
    LXValue _v985 = (nargs > 0) ? args[0] : px_null();
    LXValue _v986 = (nargs > 1) ? args[1] : px_null();
    LXValue _v987 = px_uninit();
    LXValue _v988 = px_uninit();
    LXValue _v989 = px_uninit();
    LXValue _v990 = px_uninit();
    LXValue px_err_991_val = px_null();
    int px_err_991_proped = 0;
    px_srcline(706);
    _v987 = px_index(_v985, px_int(0LL));
    px_srcline(707);
    if (px_is_truthy(px_eq(_v987, px_str("PatLiteral")))) {
        px_srcline(708);
        _v988 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v985, px_int(1LL))}, 1);
        px_srcline(709);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v986), px_str(", ")), _v988), px_str("))"));
    }
    px_srcline(710);
    if (px_is_truthy(px_eq(_v987, px_str("PatBinding")))) {
        px_srcline(711);
        _v989 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v985, px_int(1LL))}, 1);
        px_srcline(712);
        if (px_is_truthy(({ LXValue _t993 = ({ LXValue _t992 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v989}, 1), px_int(0LL)); px_is_truthy(_t992) ? px_ge(px_index(_v989, px_int(0LL)), px_str("A")) : _t992; }); px_is_truthy(_t993) ? px_le(px_index(_v989, px_int(0LL)), px_str("Z")) : _t993; }))) {
            px_srcline(713);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v986), px_str(".type == PX_ENUM && strcmp(")), _v986), px_str(".as.obj->as.enum_inst.variant, \"")), _v989), px_str("\") == 0)"));
        }
        px_srcline(714);
        return px_str("true");
    }
    px_srcline(715);
    if (px_is_truthy(px_eq(_v987, px_str("PatWildcard")))) {
        px_srcline(716);
        return px_str("true");
    }
    px_srcline(717);
    if (px_is_truthy(px_eq(_v987, px_str("PatTuple")))) {
        px_srcline(718);
        _v990 = px_index(_v985, px_int(1LL));
        px_srcline(719);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v990}, 1), px_int(0LL)))) {
            px_srcline(720);
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v990, px_int(0LL)), _v986}, 2);
        }
        px_srcline(721);
        return px_str("true");
    }
    px_srcline(722);
    if (px_is_truthy(px_eq(_v987, px_str("PatConstructor")))) {
        px_srcline(723);
        _v989 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v985, px_int(1LL))}, 1);
        px_srcline(724);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v986), px_str(".type == PX_ENUM && strcmp(")), _v986), px_str(".as.obj->as.enum_inst.variant, \"")), _v989), px_str("\") == 0)"));
    }
    px_srcline(725);
    return px_str("true");
px_err_991:
    if (px_err_991_proped) return px_err_991_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_lambda");
    LXValue _v994 = (nargs > 0) ? args[0] : px_null();
    LXValue _v995 = (nargs > 1) ? args[1] : px_null();
    LXValue _v996 = px_uninit();
    LXValue _v997 = px_uninit();
    LXValue _v998 = px_uninit();
    LXValue _v999 = px_uninit();
    LXValue _v1000 = px_uninit();
    LXValue _v1001 = px_uninit();
    LXValue _v1002 = px_uninit();
    LXValue _v1003 = px_uninit();
    LXValue _v1004 = px_uninit();
    LXValue _v1005 = px_uninit();
    LXValue _v1006 = px_uninit();
    LXValue _v1007 = px_uninit();
    LXValue _v1008 = px_uninit();
    LXValue _v1009 = px_uninit();
    LXValue _v1010 = px_uninit();
    LXValue _v1011 = px_uninit();
    LXValue _v1012 = px_uninit();
    LXValue _v1013 = px_uninit();
    LXValue _v1014 = px_uninit();
    LXValue px_err_1015_val = px_null();
    int px_err_1015_proped = 0;
    px_srcline(728);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(729);
    _v996 = px_get_global("cg_closure_id");
    px_srcline(730);
    _v997 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v996}, 1));
    px_srcline(732);
    _v998 = px_list_n((LXValue[]){}, 0);
    px_srcline(733);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v995, _v998}, 2));
    px_srcline(734);
    _v999 = px_list_n((LXValue[]){}, 0);
    px_srcline(735);
    LXValue _t1016 = _v998;
    int _il4 = (int)px_len(_t1016);
    for (int _t1017 = 0; _t1017 < _il4; _t1017++) {
        px_iter_ck(_t1016, _il4);
        _v1000 = px_iter_at(_t1016, px_int(_t1017));
        px_srcline(736);
        if (px_is_truthy(({ LXValue _t1018 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v994, _v1000}, 2)); px_is_truthy(_t1018) ? px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1000}, 1), px_null()) : _t1018; }))) {
            px_srcline(737);
            (void)(px_method(_v999, "append", (LXValue[]){_v1000}, 1));
        }
    }
    px_srcline(738);
    _v1001 = px_str("px_null()");
    px_srcline(739);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v999}, 1), px_int(0LL)))) {
        px_srcline(740);
        _v1002 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(741);
        LXValue _t1019 = _v999;
        int _il5 = (int)px_len(_t1019);
        for (int _t1020 = 0; _t1020 < _il5; _t1020++) {
            px_iter_ck(_t1019, _il5);
            _v1000 = px_iter_at(_t1019, px_int(_t1020));
            px_srcline(742);
            _v1003 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v1000}, 1);
            px_srcline(746);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1000}, 1))) {
                px_srcline(747);
                 _v1002 = px_add(_v1002, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v1000), px_str("\", px_cell(px_cell_get(")), _v1003), px_str("))); ")));
            }
            else {
                px_srcline(749);
                 _v1002 = px_add(_v1002, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v1000), px_str("\", px_cell(")), _v1003), px_str(")); ")));
            }
        }
        px_srcline(750);
         _v1002 = px_add(_v1002, px_str("_capenv; })"));
        px_srcline(751);
         _v1001 = _v1002;
    }
    px_srcline(752);
    _v1004 = px_add(px_add(px_str("static LXValue "), _v997), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(753);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v999}, 1), px_int(0LL)))) {
        px_srcline(754);
         _v1004 = px_add(_v1004, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(756);
         _v1004 = px_add(_v1004, px_str("    (void)ctx;\n"));
    }
    px_srcline(757);
    _v1005 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(758);
    _v1006 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(759);
    _v1007 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(761);
    _v1008 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(762);
    _v1009 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(763);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(764);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(765);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(766);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(767);
    _v1010 = px_int(0LL);
    px_srcline(768);
    while (px_is_truthy(px_lt(_v1010, px_call(px_get_global("len"), (LXValue[]){_v994}, 1)))) {
        px_srcline(769);
        _v1011 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v994, _v1010)}, 1);
        px_srcline(770);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){px_index(_v994, _v1010)}, 1));
        px_srcline(771);
         _v1004 = px_add(_v1004, px_add(({ LXValue _s187 = px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1011), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1010}, 1)), px_str(") ? args[")); LXValue _s188 = px_call(px_get_global("str"), (LXValue[]){_v1010}, 1); px_add(_s187, _s188); }), px_str("] : px_null();\n")));
        px_srcline(772);
         _v1010 = px_add(_v1010, px_int(1LL));
    }
    px_srcline(773);
    LXValue _t1021 = _v999;
    int _il6 = (int)px_len(_t1021);
    for (int _t1022 = 0; _t1022 < _il6; _t1022++) {
        px_iter_ck(_t1021, _il6);
        _v1000 = px_iter_at(_t1021, px_int(_t1022));
        px_srcline(774);
        _v1012 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1000}, 1);
        px_srcline(775);
        px_index_set(px_get_global("cg_cells"), _v1000, px_int(1LL));
        px_srcline(776);
        if (px_is_truthy(px_method(_v1009, "has", (LXValue[]){_v1000}, 1))) {
            px_srcline(777);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v1000}, 1));
        }
        px_srcline(778);
         _v1004 = px_add(_v1004, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1012), px_str(" = px_env_lookup(ctx, \"")), _v1000), px_str("\");\n")));
    }
    px_srcline(779);
    _v1013 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v995}, 1);
    px_srcline(780);
    _v1014 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(781);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1014}, 1));
    px_srcline(782);
     _v1004 = px_add(_v1004, px_add(px_add(px_str("    LXValue "), _v1014), px_str("_val = px_null();\n")));
    px_srcline(783);
     _v1004 = px_add(_v1004, px_add(px_add(px_str("    int "), _v1014), px_str("_proped = 0;\n")));
    px_srcline(784);
     _v1004 = px_add(_v1004, px_add(px_add(px_str("    return "), _v1013), px_str(";\n")));
    px_srcline(785);
     _v1004 = px_add(_v1004, px_add(_v1014, px_str(":\n")));
    px_srcline(786);
     _v1004 = px_add(_v1004, px_add(px_add(px_add(px_add(px_str("    if ("), _v1014), px_str("_proped) return ")), _v1014), px_str("_val;\n")));
    px_srcline(787);
     _v1004 = px_add(_v1004, px_str("    return px_null();\n"));
    px_srcline(788);
     _v1004 = px_add(_v1004, px_str("}\n"));
    px_srcline(789);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(790);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1004));
    px_srcline(791);
    px_set_global("cg_vars", _v1005);
    px_srcline(792);
    px_set_global("cg_var_types", _v1006);
    px_srcline(793);
    px_set_global("cg_cells", _v1007);
    px_srcline(794);
    px_set_global("cg_inited", _v1008);
    px_srcline(796);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v999}, 1), px_int(0LL)))) {
        px_srcline(797);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v996}, 1)), px_str(">\", ")), _v997), px_str(", ")), _v1001), px_str(")"));
    }
    px_srcline(798);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v996}, 1)), px_str(">\", ")), _v997), px_str(", NULL)"));
px_err_1015:
    if (px_err_1015_proped) return px_err_1015_val;
    return px_null();
}

static LXValue fn_cgm_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_perr");
    LXValue _v1023 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1024 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1025_val = px_null();
    int px_err_1025_proped = 0;
    px_srcline(20);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1023), px_str(": ")), _v1024)}, 1));
    px_srcline(21);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1025:
    if (px_err_1025_proped) return px_err_1025_val;
    return px_null();
}

static LXValue fn_cgm_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_pwarn");
    LXValue _v1026 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1027_val = px_null();
    int px_err_1027_proped = 0;
    px_srcline(23);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1026)}, 1));
px_err_1027:
    if (px_err_1027_proped) return px_err_1027_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dirname");
    LXValue _v1028 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1029 = px_uninit();
    LXValue px_err_1030_val = px_null();
    int px_err_1030_proped = 0;
    px_srcline(26);
    _v1029 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1028}, 1), px_int(1LL));
    px_srcline(27);
    while (px_is_truthy(px_ge(_v1029, px_int(0LL)))) {
        px_srcline(28);
        if (px_is_truthy(px_eq(px_index(_v1028, _v1029), px_str("/")))) {
            px_srcline(29);
            if (px_is_truthy(px_eq(_v1029, px_int(0LL)))) {
                px_srcline(30);
                return px_str("/");
            }
            px_srcline(31);
            return px_slice(_v1028, px_int(0LL), _v1029, px_null());
        }
        px_srcline(32);
         _v1029 = px_sub(_v1029, px_int(1LL));
    }
    px_srcline(33);
    return px_str(".");
px_err_1030:
    if (px_err_1030_proped) return px_err_1030_val;
    return px_null();
}

static LXValue fn_cg_norm_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_norm_path");
    LXValue _v1031 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1032 = px_uninit();
    LXValue _v1033 = px_uninit();
    LXValue _v1034 = px_uninit();
    LXValue _v1035 = px_uninit();
    LXValue _v1036 = px_uninit();
    LXValue _v1037 = px_uninit();
    LXValue px_err_1038_val = px_null();
    int px_err_1038_proped = 0;
    px_srcline(37);
    _v1032 = ({ LXValue _t1039 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1031}, 1), px_int(0LL)); px_is_truthy(_t1039) ? px_eq(px_index(_v1031, px_int(0LL)), px_str("/")) : _t1039; });
    px_srcline(38);
    _v1033 = px_list_n((LXValue[]){}, 0);
    px_srcline(39);
    _v1034 = px_str("");
    px_srcline(40);
    _v1035 = px_int(0LL);
    px_srcline(41);
    while (px_is_truthy(px_le(_v1035, px_call(px_get_global("len"), (LXValue[]){_v1031}, 1)))) {
        px_srcline(42);
        if (px_is_truthy(({ LXValue _t1040 = px_eq(_v1035, px_call(px_get_global("len"), (LXValue[]){_v1031}, 1)); px_is_truthy(_t1040) ? _t1040 : px_eq(px_index(_v1031, _v1035), px_str("/")); }))) {
            px_srcline(43);
            if (px_is_truthy(px_eq(_v1034, px_str("..")))) {
                px_srcline(44);
                if (px_is_truthy(({ LXValue _t1041 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1033}, 1), px_int(0LL)); px_is_truthy(_t1041) ? px_ne(px_index(_v1033, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1033}, 1), px_int(1LL))), px_str("..")) : _t1041; }))) {
                    px_srcline(45);
                    (void)(px_method(_v1033, "pop", (LXValue[]){}, 0));
                }
                else if (px_is_truthy(px_not(_v1032))) {
                    px_srcline(47);
                    (void)(px_method(_v1033, "append", (LXValue[]){px_str("..")}, 1));
                }
            }
            else if (px_is_truthy(({ LXValue _t1042 = px_ne(_v1034, px_str("")); px_is_truthy(_t1042) ? px_ne(_v1034, px_str(".")) : _t1042; }))) {
                px_srcline(49);
                (void)(px_method(_v1033, "append", (LXValue[]){_v1034}, 1));
            }
            px_srcline(50);
             _v1034 = px_str("");
        }
        else {
            px_srcline(52);
             _v1034 = px_add(_v1034, px_index(_v1031, _v1035));
        }
        px_srcline(53);
         _v1035 = px_add(_v1035, px_int(1LL));
    }
    px_srcline(54);
    _v1036 = px_str("");
    px_srcline(55);
    _v1037 = px_int(0LL);
    px_srcline(56);
    while (px_is_truthy(px_lt(_v1037, px_call(px_get_global("len"), (LXValue[]){_v1033}, 1)))) {
        px_srcline(57);
         _v1036 = px_add(_v1036, px_add(px_str("/"), px_index(_v1033, _v1037)));
        px_srcline(58);
         _v1037 = px_add(_v1037, px_int(1LL));
    }
    px_srcline(59);
    if (px_is_truthy(_v1032)) {
        px_srcline(60);
        if (px_is_truthy(px_eq(_v1036, px_str("")))) {
            px_srcline(61);
            return px_str("/");
        }
        px_srcline(62);
        return _v1036;
    }
    px_srcline(63);
    if (px_is_truthy(px_eq(_v1036, px_str("")))) {
        px_srcline(64);
        return px_str(".");
    }
    px_srcline(65);
    return px_slice(_v1036, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1036}, 1), px_null());
px_err_1038:
    if (px_err_1038_proped) return px_err_1038_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_stdlib_dir");
    LXValue _v1043 = px_uninit();
    LXValue _v1044 = px_uninit();
    LXValue _v1045 = px_uninit();
    LXValue _v1046 = px_uninit();
    LXValue _v1047 = px_uninit();
    LXValue _v1048 = px_uninit();
    LXValue px_err_1049_val = px_null();
    int px_err_1049_proped = 0;
    px_srcline(75);
    _v1043 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    px_srcline(76);
    if (px_is_truthy(({ LXValue _t1050 = px_ne(_v1043, px_null()); px_is_truthy(_t1050) ? px_call(px_get_global("exists"), (LXValue[]){_v1043}, 1) : _t1050; }))) {
        px_srcline(77);
        return _v1043;
    }
    px_srcline(78);
    _v1044 = px_call(px_get_global("os_self_path"), (LXValue[]){}, 0);
    px_srcline(79);
    if (px_is_truthy(({ LXValue _t1051 = px_ne(_v1044, px_null()); px_is_truthy(_t1051) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1044}, 1), px_int(0LL)) : _t1051; }))) {
        px_srcline(80);
        _v1045 = px_call(px_get_global("cg_norm_path"), (LXValue[]){px_add(px_call(px_get_global("cg_dirname"), (LXValue[]){_v1044}, 1), px_str("/../stdlib"))}, 1);
        px_srcline(81);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1045}, 1))) {
            px_srcline(82);
            return _v1045;
        }
    }
    px_srcline(83);
    _v1046 = px_list_n((LXValue[]){px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 4);
    px_srcline(84);
    _v1047 = px_int(0LL);
    px_srcline(85);
    while (px_is_truthy(px_lt(_v1047, px_call(px_get_global("len"), (LXValue[]){_v1046}, 1)))) {
        px_srcline(86);
        _v1048 = px_index(_v1046, _v1047);
        px_srcline(87);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1048}, 1))) {
            px_srcline(88);
            return _v1048;
        }
        px_srcline(89);
         _v1047 = px_add(_v1047, px_int(1LL));
    }
    px_srcline(90);
    return px_null();
px_err_1049:
    if (px_err_1049_proped) return px_err_1049_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find_module_path");
    LXValue _v1052 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1053 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1054 = px_uninit();
    LXValue _v1055 = px_uninit();
    LXValue _v1056 = px_uninit();
    LXValue _v1057 = px_uninit();
    LXValue _v1058 = px_uninit();
    LXValue _v1059 = px_uninit();
    LXValue _v1060 = px_uninit();
    LXValue _v1061 = px_uninit();
    LXValue _v1062 = px_uninit();
    LXValue _v1063 = px_uninit();
    LXValue _v1064 = px_uninit();
    LXValue _v1065 = px_uninit();
    LXValue _v1066 = px_uninit();
    LXValue _v1067 = px_uninit();
    LXValue _v1068 = px_uninit();
    LXValue px_err_1069_val = px_null();
    int px_err_1069_proped = 0;
    px_srcline(93);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1052}, 1), px_int(0LL)))) {
        px_srcline(94);
        return px_null();
    }
    px_srcline(96);
    if (px_is_truthy(({ LXValue _t1071 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1052}, 1), px_int(1LL)); px_is_truthy(_t1071) ? ({ LXValue _t1070 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v1052, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1070) ? _t1070 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v1052, px_int(0LL)), px_str(".px")}, 2); }) : _t1071; }))) {
        px_srcline(97);
        _v1054 = px_index(_v1052, px_int(0LL));
        px_srcline(98);
        _v1055 = _v1054;
        px_srcline(99);
        if (px_is_truthy(px_not(({ LXValue _t1072 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1054}, 1), px_int(0LL)); px_is_truthy(_t1072) ? px_eq(px_index(_v1054, px_int(0LL)), px_str("/")) : _t1072; })))) {
            px_srcline(100);
             _v1055 = px_add(px_add(_v1053, px_str("/")), _v1054);
        }
        px_srcline(101);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1055}, 1))) {
            px_srcline(102);
            return _v1055;
        }
        px_srcline(103);
        return px_null();
    }
    px_srcline(105);
    if (px_is_truthy(px_eq(px_index(_v1052, px_int(0LL)), px_str("std")))) {
        px_srcline(106);
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v1052}, 1), px_int(2LL)))) {
            px_srcline(107);
            return px_null();
        }
        px_srcline(108);
        _v1056 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(109);
        if (px_is_truthy(px_eq(_v1056, px_null()))) {
            px_srcline(110);
            return px_null();
        }
        px_srcline(111);
        _v1055 = _v1056;
        px_srcline(112);
        _v1057 = px_int(1LL);
        px_srcline(113);
        while (px_is_truthy(px_lt(_v1057, px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)))) {
            px_srcline(114);
             _v1055 = px_add(_v1055, px_add(px_str("/"), px_index(_v1052, _v1057)));
            px_srcline(115);
             _v1057 = px_add(_v1057, px_int(1LL));
        }
        px_srcline(116);
        _v1058 = px_add(_v1055, px_str(".px"));
        px_srcline(117);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1058}, 1))) {
            px_srcline(118);
            return _v1058;
        }
        px_srcline(119);
        _v1059 = px_add(_v1055, px_str("/mod.px"));
        px_srcline(120);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1059}, 1))) {
            px_srcline(121);
            return _v1059;
        }
        px_srcline(122);
        return px_null();
    }
    px_srcline(124);
    _v1060 = px_list_n((LXValue[]){_v1053}, 1);
    px_srcline(125);
    _v1061 = px_add(_v1053, px_str("/.px_modules"));
    px_srcline(126);
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1061}, 1))) {
        px_srcline(127);
        (void)(px_method(_v1060, "append", (LXValue[]){_v1061}, 1));
        px_srcline(128);
        _v1062 = px_call(px_get_global("list_dir"), (LXValue[]){_v1061}, 1);
        px_srcline(129);
        _v1063 = px_int(0LL);
        px_srcline(130);
        while (px_is_truthy(px_lt(_v1063, px_call(px_get_global("len"), (LXValue[]){_v1062}, 1)))) {
            px_srcline(131);
            _v1064 = px_index(_v1062, _v1063);
            px_srcline(132);
            _v1065 = px_add(px_add(_v1061, px_str("/")), _v1064);
            px_srcline(133);
            if (px_is_truthy(({ LXValue _t1073 = px_call(px_get_global("exists"), (LXValue[]){_v1065}, 1); px_is_truthy(_t1073) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1064, px_str(".")}, 2)) : _t1073; }))) {
                px_srcline(134);
                (void)(px_method(_v1060, "append", (LXValue[]){_v1065}, 1));
            }
            px_srcline(135);
             _v1063 = px_add(_v1063, px_int(1LL));
        }
    }
    px_srcline(136);
    _v1066 = px_int(0LL);
    px_srcline(137);
    while (px_is_truthy(px_lt(_v1066, px_call(px_get_global("len"), (LXValue[]){_v1060}, 1)))) {
        px_srcline(138);
        _v1067 = px_index(_v1060, _v1066);
        px_srcline(139);
        _v1055 = _v1067;
        px_srcline(140);
        _v1057 = px_int(0LL);
        px_srcline(141);
        while (px_is_truthy(px_lt(_v1057, px_call(px_get_global("len"), (LXValue[]){_v1052}, 1)))) {
            px_srcline(142);
             _v1055 = px_add(_v1055, px_add(px_str("/"), px_index(_v1052, _v1057)));
            px_srcline(143);
             _v1057 = px_add(_v1057, px_int(1LL));
        }
        px_srcline(144);
        _v1058 = px_add(_v1055, px_str(".px"));
        px_srcline(145);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1058}, 1))) {
            px_srcline(146);
            return _v1058;
        }
        px_srcline(147);
        _v1059 = px_add(_v1055, px_str("/mod.px"));
        px_srcline(148);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1059}, 1))) {
            px_srcline(149);
            return _v1059;
        }
        px_srcline(150);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1052}, 1), px_int(1LL)))) {
            px_srcline(151);
            _v1068 = px_add(px_add(px_add(_v1067, px_str("/")), px_index(_v1052, px_int(0LL))), px_str(".px"));
            px_srcline(152);
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1068}, 1))) {
                px_srcline(153);
                return _v1068;
            }
        }
        px_srcline(154);
         _v1066 = px_add(_v1066, px_int(1LL));
    }
    px_srcline(155);
    return px_null();
px_err_1069:
    if (px_err_1069_proped) return px_err_1069_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_definition");
    LXValue _v1074 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1075 = px_uninit();
    LXValue px_err_1076_val = px_null();
    int px_err_1076_proped = 0;
    px_srcline(158);
    _v1075 = px_index(_v1074, px_int(0LL));
    px_srcline(159);
    if (px_is_truthy(px_eq(_v1075, px_str("FuncDef")))) {
        px_srcline(161);
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1074, px_int(1LL))}, 1), px_str("main")))) {
            px_srcline(162);
            return px_bool(false);
        }
        px_srcline(163);
        return px_bool(true);
    }
    px_srcline(164);
    if (px_is_truthy(px_eq(_v1075, px_str("ExternDef")))) {
        px_srcline(165);
        return px_bool(true);
    }
    px_srcline(166);
    if (px_is_truthy(({ LXValue _t1079 = ({ LXValue _t1078 = ({ LXValue _t1077 = px_eq(_v1075, px_str("StructDef")); px_is_truthy(_t1077) ? _t1077 : px_eq(_v1075, px_str("EnumDef")); }); px_is_truthy(_t1078) ? _t1078 : px_eq(_v1075, px_str("TraitDef")); }); px_is_truthy(_t1079) ? _t1079 : px_eq(_v1075, px_str("ImplDef")); }))) {
        px_srcline(167);
        return px_bool(true);
    }
    px_srcline(168);
    if (px_is_truthy(px_eq(_v1075, px_str("VarDecl")))) {
        px_srcline(172);
        return px_bool(true);
    }
    px_srcline(173);
    return px_bool(false);
px_err_1076:
    if (px_err_1076_proped) return px_err_1076_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_def_name");
    LXValue _v1080 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1081 = px_uninit();
    LXValue _v1082 = px_uninit();
    LXValue _v1083 = px_uninit();
    LXValue _v1084 = px_uninit();
    LXValue px_err_1085_val = px_null();
    int px_err_1085_proped = 0;
    px_srcline(176);
    _v1081 = px_index(_v1080, px_int(0LL));
    px_srcline(177);
    if (px_is_truthy(({ LXValue _t1089 = ({ LXValue _t1088 = ({ LXValue _t1087 = ({ LXValue _t1086 = px_eq(_v1081, px_str("FuncDef")); px_is_truthy(_t1086) ? _t1086 : px_eq(_v1081, px_str("StructDef")); }); px_is_truthy(_t1087) ? _t1087 : px_eq(_v1081, px_str("EnumDef")); }); px_is_truthy(_t1088) ? _t1088 : px_eq(_v1081, px_str("TraitDef")); }); px_is_truthy(_t1089) ? _t1089 : px_eq(_v1081, px_str("ExternDef")); }))) {
        px_srcline(178);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1080, px_int(1LL))}, 1);
    }
    px_srcline(179);
    if (px_is_truthy(px_eq(_v1081, px_str("VarDecl")))) {
        px_srcline(180);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1080, px_int(2LL))}, 1);
    }
    px_srcline(181);
    if (px_is_truthy(px_eq(_v1081, px_str("ImplDef")))) {
        px_srcline(182);
        _v1082 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1080, px_int(1LL))}, 1);
        px_srcline(183);
        _v1083 = px_index(_v1080, px_int(2LL));
        px_srcline(184);
        _v1084 = px_str("None");
        px_srcline(185);
        if (px_is_truthy(px_ne(_v1083, px_null()))) {
            px_srcline(186);
             _v1084 = px_add(px_add(px_str("Some("), _v1083), px_str(")"));
        }
        px_srcline(187);
        return px_add(px_add(px_add(px_str("impl::"), _v1082), px_str("::")), _v1084);
    }
    px_srcline(188);
    return px_null();
px_err_1085:
    if (px_err_1085_proped) return px_err_1085_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_module");
    LXValue _v1090 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1091 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1092 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1093 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1094 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1095 = px_uninit();
    LXValue _v1096 = px_uninit();
    LXValue _v1097 = px_uninit();
    LXValue _v1098 = px_uninit();
    LXValue _v1099 = px_uninit();
    LXValue _v1100 = px_uninit();
    LXValue _v1101 = px_uninit();
    LXValue _v1102 = px_uninit();
    LXValue _v1103 = px_uninit();
    LXValue _v1104 = px_uninit();
    LXValue _v1105 = px_uninit();
    LXValue _v1106 = px_uninit();
    LXValue _v1107 = px_uninit();
    LXValue _v1108 = px_uninit();
    LXValue _v1109 = px_uninit();
    LXValue _v1110 = px_uninit();
    LXValue _v1111 = px_uninit();
    LXValue _v1112 = px_uninit();
    LXValue _v1113 = px_uninit();
    LXValue _v1114 = px_uninit();
    LXValue px_err_1115_val = px_null();
    int px_err_1115_proped = 0;
    px_srcline(192);
    _v1095 = px_list_n((LXValue[]){}, 0);
    px_srcline(193);
    _v1096 = px_int(0LL);
    px_srcline(194);
    while (px_is_truthy(px_lt(_v1096, px_call(px_get_global("len"), (LXValue[]){_v1090}, 1)))) {
        px_srcline(195);
        (void)(px_method(_v1095, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1090, _v1096)}, 1)}, 1));
        px_srcline(196);
         _v1096 = px_add(_v1096, px_int(1LL));
    }
    px_srcline(197);
     _v1090 = _v1095;
    px_srcline(198);
    _v1097 = px_list_n((LXValue[]){}, 0);
    px_srcline(199);
    _v1098 = px_int(0LL);
    px_srcline(200);
    while (px_is_truthy(px_lt(_v1098, px_call(px_get_global("len"), (LXValue[]){_v1091}, 1)))) {
        px_srcline(201);
        (void)(px_method(_v1097, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1091, _v1098)}, 1)}, 1));
        px_srcline(202);
         _v1098 = px_add(_v1098, px_int(1LL));
    }
    px_srcline(203);
     _v1091 = _v1097;
    px_srcline(204);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1090}, 1), px_int(0LL)))) {
        px_srcline(205);
        return px_null();
    }
    px_srcline(207);
    if (px_is_truthy(({ LXValue _t1118 = ({ LXValue _t1117 = ({ LXValue _t1116 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1090}, 1), px_int(1LL)); px_is_truthy(_t1116) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1090, px_int(0LL))}, 1), px_int(2LL)) : _t1116; }); px_is_truthy(_t1117) ? px_eq(px_slice(px_index(_v1090, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t1117; }); px_is_truthy(_t1118) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v1090, px_int(0LL)), px_str(".px")}, 2)) : _t1118; }))) {
        px_srcline(208);
        return px_null();
    }
    px_srcline(209);
    _v1099 = px_eq(px_index(_v1090, px_int(0LL)), px_str("std"));
    px_srcline(210);
    _v1100 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1090}, 2);
    px_srcline(211);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1100}, 1))) {
        px_srcline(212);
        return px_null();
    }
    px_srcline(213);
    _v1101 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1090, _v1092}, 2);
    px_srcline(214);
    if (px_is_truthy(px_eq(_v1101, px_null()))) {
        px_srcline(221);
        _v1102 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(222);
        _v1103 = px_str("");
        px_srcline(223);
        if (px_is_truthy(({ LXValue _t1119 = px_eq(px_index(_v1090, px_int(0LL)), px_str("std")); px_is_truthy(_t1119) ? px_eq(_v1102, px_null()) : _t1119; }))) {
            px_srcline(224);
             _v1103 = px_str("；stdlib 未找到（设 PX_STDLIB 或放置 stdlib/）");
        }
        px_srcline(225);
        if (px_is_truthy(px_ne(px_call(px_get_global("env"), (LXValue[]){px_str("PX_STRICT_MODULE")}, 1), px_null()))) {
            px_srcline(226);
            (void)(px_call(px_get_global("cgm_perr"), (LXValue[]){px_str("E3005"), px_add(px_add(px_add(px_str("找不到模块 '"), _v1100), px_str("'")), _v1103)}, 2));
        }
        px_srcline(227);
        (void)(px_call(px_get_global("cgm_pwarn"), (LXValue[]){px_add(px_add(px_add(px_str("找不到模块 '"), _v1100), px_str("'（已跳过 → 运行期将报未定义）")), _v1103)}, 1));
        px_srcline(228);
        return px_null();
    }
    px_srcline(232);
    _v1104 = px_add(px_str("#"), px_call(px_get_global("cg_norm_path"), (LXValue[]){_v1101}, 1));
    px_srcline(233);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1104}, 1))) {
        px_srcline(234);
        return px_null();
    }
    px_srcline(235);
    px_index_set(px_get_global("loaded"), _v1100, _v1101);
    px_srcline(236);
    px_index_set(px_get_global("loaded"), _v1104, _v1101);
    px_srcline(237);
    _v1105 = px_call(px_get_global("read_file"), (LXValue[]){_v1101}, 1);
    px_srcline(238);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1105}, 1));
    px_srcline(239);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(240);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(241);
    _v1106 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(243);
    _v1107 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1101}, 1);
    px_srcline(244);
    _v1108 = px_list_n((LXValue[]){}, 0);
    px_srcline(245);
    _v1109 = px_int(0LL);
    px_srcline(246);
    while (px_is_truthy(px_lt(_v1109, px_call(px_get_global("len"), (LXValue[]){px_index(_v1106, px_int(1LL))}, 1)))) {
        px_srcline(247);
        _v1110 = px_index(px_index(_v1106, px_int(1LL)), _v1109);
        px_srcline(248);
        if (px_is_truthy(px_eq(px_index(_v1110, px_int(0LL)), px_str("Import")))) {
            px_srcline(249);
            (void)(px_method(_v1108, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1110, px_int(1LL)), px_index(_v1110, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(250);
         _v1109 = px_add(_v1109, px_int(1LL));
    }
    px_srcline(251);
    _v1111 = px_int(0LL);
    px_srcline(252);
    while (px_is_truthy(px_lt(_v1111, px_call(px_get_global("len"), (LXValue[]){_v1108}, 1)))) {
        px_srcline(253);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1108, _v1111), px_int(0LL)), px_index(px_index(_v1108, _v1111), px_int(1LL)), _v1107, _v1093, _v1094}, 5));
        px_srcline(254);
         _v1111 = px_add(_v1111, px_int(1LL));
    }
    px_srcline(256);
    _v1112 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1091}, 1), px_int(0LL));
    px_srcline(257);
    _v1113 = px_int(0LL);
    px_srcline(258);
    while (px_is_truthy(px_lt(_v1113, px_call(px_get_global("len"), (LXValue[]){px_index(_v1106, px_int(1LL))}, 1)))) {
        px_srcline(259);
        _v1110 = px_index(px_index(_v1106, px_int(1LL)), _v1113);
        px_srcline(260);
        if (px_is_truthy(px_eq(px_index(_v1110, px_int(0LL)), px_str("Import")))) {
            px_srcline(261);
             _v1113 = px_add(_v1113, px_int(1LL));
            px_srcline(262);
            continue;
        }
        px_srcline(263);
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1110}, 1)))) {
            px_srcline(264);
             _v1113 = px_add(_v1113, px_int(1LL));
            px_srcline(265);
            continue;
        }
        px_srcline(266);
        _v1114 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1110}, 1);
        px_srcline(267);
        if (px_is_truthy(px_eq(_v1114, px_null()))) {
            px_srcline(268);
            (void)(px_method(_v1093, "append", (LXValue[]){_v1110}, 1));
        }
        else {
            px_srcline(270);
            if (px_is_truthy(_v1112)) {
                px_srcline(271);
                if (px_is_truthy(({ LXValue _t1120 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1114}, 1), px_int(5LL)); px_is_truthy(_t1120) ? px_eq(px_slice(_v1114, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1120; }))) {
                    px_srcline(272);
                     _v1113 = px_add(_v1113, px_int(1LL));
                    px_srcline(273);
                    continue;
                }
                px_srcline(274);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1091, _v1114}, 2)))) {
                    px_srcline(275);
                     _v1113 = px_add(_v1113, px_int(1LL));
                    px_srcline(276);
                    continue;
                }
            }
            px_srcline(277);
            if (px_is_truthy(_v1099)) {
                px_srcline(278);
                if (px_is_truthy(px_method(_v1094, "has", (LXValue[]){_v1114}, 1))) {
                    px_srcline(279);
                     _v1113 = px_add(_v1113, px_int(1LL));
                    px_srcline(280);
                    continue;
                }
                px_srcline(281);
                px_index_set(_v1094, _v1114, px_bool(true));
            }
            px_srcline(282);
            (void)(px_method(_v1093, "append", (LXValue[]){_v1110}, 1));
        }
        px_srcline(283);
         _v1113 = px_add(_v1113, px_int(1LL));
    }
px_err_1115:
    if (px_err_1115_proped) return px_err_1115_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_resolve_modules");
    LXValue _v1121 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1122 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1123 = px_uninit();
    LXValue _v1124 = px_uninit();
    LXValue _v1125 = px_uninit();
    LXValue _v1126 = px_uninit();
    LXValue _v1127 = px_uninit();
    LXValue _v1128 = px_uninit();
    LXValue _v1129 = px_uninit();
    LXValue _v1130 = px_uninit();
    LXValue _v1131 = px_uninit();
    LXValue _v1132 = px_uninit();
    LXValue px_err_1133_val = px_null();
    int px_err_1133_proped = 0;
    px_srcline(286);
    _v1123 = px_index(_v1121, px_int(1LL));
    px_srcline(287);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1123}, 1), px_int(0LL)))) {
        px_srcline(288);
        return _v1121;
    }
    px_srcline(289);
    _v1124 = px_list_n((LXValue[]){}, 0);
    px_srcline(290);
    _v1125 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(291);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(292);
    _v1126 = px_list_n((LXValue[]){}, 0);
    px_srcline(293);
    _v1127 = px_int(0LL);
    px_srcline(294);
    while (px_is_truthy(px_lt(_v1127, px_call(px_get_global("len"), (LXValue[]){_v1123}, 1)))) {
        px_srcline(295);
        _v1128 = px_index(_v1123, _v1127);
        px_srcline(296);
        if (px_is_truthy(px_eq(px_index(_v1128, px_int(0LL)), px_str("Import")))) {
            px_srcline(297);
            (void)(px_method(_v1126, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1128, px_int(1LL)), px_index(_v1128, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(298);
         _v1127 = px_add(_v1127, px_int(1LL));
    }
    px_srcline(299);
    _v1129 = px_int(0LL);
    px_srcline(300);
    while (px_is_truthy(px_lt(_v1129, px_call(px_get_global("len"), (LXValue[]){_v1126}, 1)))) {
        px_srcline(301);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1126, _v1129), px_int(0LL)), px_index(px_index(_v1126, _v1129), px_int(1LL)), _v1122, _v1124, _v1125}, 5));
        px_srcline(302);
         _v1129 = px_add(_v1129, px_int(1LL));
    }
    px_srcline(303);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1124}, 1), px_int(0LL)))) {
        px_srcline(304);
        return _v1121;
    }
    px_srcline(305);
    _v1130 = px_list_n((LXValue[]){}, 0);
    px_srcline(306);
    _v1131 = px_int(0LL);
    px_srcline(307);
    while (px_is_truthy(px_lt(_v1131, px_call(px_get_global("len"), (LXValue[]){_v1124}, 1)))) {
        px_srcline(308);
        (void)(px_method(_v1130, "append", (LXValue[]){px_index(_v1124, _v1131)}, 1));
        px_srcline(309);
         _v1131 = px_add(_v1131, px_int(1LL));
    }
    px_srcline(310);
    _v1132 = px_int(0LL);
    px_srcline(311);
    while (px_is_truthy(px_lt(_v1132, px_call(px_get_global("len"), (LXValue[]){_v1123}, 1)))) {
        px_srcline(312);
        (void)(px_method(_v1130, "append", (LXValue[]){px_index(_v1123, _v1132)}, 1));
        px_srcline(313);
         _v1132 = px_add(_v1132, px_int(1LL));
    }
    px_srcline(314);
    return px_list_n((LXValue[]){px_str("Program"), _v1130}, 2);
px_err_1133:
    if (px_err_1133_proped) return px_err_1133_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_dict");
    LXValue _v1134 = px_uninit();
    LXValue px_err_1135_val = px_null();
    int px_err_1135_proped = 0;
    px_srcline(86);
    _v1134 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(87);
    (void)(px_method(_v1134, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(88);
    return _v1134;
px_err_1135:
    if (px_err_1135_proped) return px_err_1135_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dict_copy");
    LXValue _v1136 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1137 = px_uninit();
    LXValue _v1138 = px_uninit();
    LXValue _v1139 = px_uninit();
    LXValue px_err_1140_val = px_null();
    int px_err_1140_proped = 0;
    px_srcline(90);
    _v1137 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(91);
    _v1138 = px_method(_v1136, "keys", (LXValue[]){}, 0);
    px_srcline(92);
    _v1139 = px_int(0LL);
    px_srcline(93);
    while (px_is_truthy(px_lt(_v1139, px_call(px_get_global("len"), (LXValue[]){_v1138}, 1)))) {
        px_srcline(94);
        px_index_set(_v1137, px_index(_v1138, _v1139), px_index(_v1136, px_index(_v1138, _v1139)));
        px_srcline(95);
         _v1139 = px_add(_v1139, px_int(1LL));
    }
    px_srcline(96);
    return _v1137;
px_err_1140:
    if (px_err_1140_proped) return px_err_1140_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_uid");
    LXValue px_err_1141_val = px_null();
    int px_err_1141_proped = 0;
    px_srcline(98);
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    px_srcline(99);
    return px_get_global("cg_uidc");
px_err_1141:
    if (px_err_1141_proped) return px_err_1141_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_tmp");
    LXValue px_err_1142_val = px_null();
    int px_err_1142_proped = 0;
    px_srcline(101);
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1142:
    if (px_err_1142_proped) return px_err_1142_val;
    return px_null();
}

static LXValue fn_cg_iter_len_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_iter_len_tmp");
    LXValue px_err_1143_val = px_null();
    int px_err_1143_proped = 0;
    px_srcline(106);
    px_set_global("cg_iter_uid", px_add(px_get_global("cg_iter_uid"), px_int(1LL)));
    px_srcline(107);
    return px_add(px_str("_il"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_iter_uid")}, 1));
px_err_1143:
    if (px_err_1143_proped) return px_err_1143_val;
    return px_null();
}

static LXValue fn_cg_unpack_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_unpack_tmp");
    LXValue px_err_1144_val = px_null();
    int px_err_1144_proped = 0;
    px_srcline(111);
    px_set_global("cg_unpack_uid", px_add(px_get_global("cg_unpack_uid"), px_int(1LL)));
    px_srcline(112);
    return px_add(px_str("_up"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_unpack_uid")}, 1));
px_err_1144:
    if (px_err_1144_proped) return px_err_1144_val;
    return px_null();
}

static LXValue fn_cg_for_names(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_for_names");
    LXValue _v1145 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1146 = px_uninit();
    LXValue _v1147 = px_uninit();
    LXValue px_err_1148_val = px_null();
    int px_err_1148_proped = 0;
    px_srcline(115);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1145}, 1), px_str("string")))) {
        px_srcline(116);
        return px_list_n((LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v1145}, 1)}, 1);
    }
    px_srcline(117);
    _v1146 = px_list_n((LXValue[]){}, 0);
    px_srcline(118);
    _v1147 = px_int(0LL);
    px_srcline(119);
    while (px_is_truthy(px_lt(_v1147, px_call(px_get_global("len"), (LXValue[]){_v1145}, 1)))) {
        px_srcline(120);
        (void)(px_method(_v1146, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1145, _v1147)}, 1)}, 1));
        px_srcline(121);
         _v1147 = px_add(_v1147, px_int(1LL));
    }
    px_srcline(122);
    return _v1146;
px_err_1148:
    if (px_err_1148_proped) return px_err_1148_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_var");
    LXValue _v1149 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1150 = px_uninit();
    LXValue px_err_1151_val = px_null();
    int px_err_1151_proped = 0;
    px_srcline(124);
    _v1150 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(125);
    px_index_set(px_get_global("cg_vars"), _v1149, _v1150);
    px_srcline(128);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1149}, 1))) {
        px_srcline(129);
        (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v1149}, 1));
    }
    px_srcline(130);
    return _v1150;
px_err_1151:
    if (px_err_1151_proped) return px_err_1151_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_var_of");
    LXValue _v1152 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1153_val = px_null();
    int px_err_1153_proped = 0;
    px_srcline(132);
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1152}, 1))) {
        px_srcline(133);
        return px_index(px_get_global("cg_vars"), _v1152);
    }
    px_srcline(134);
    return px_null();
px_err_1153:
    if (px_err_1153_proped) return px_err_1153_val;
    return px_null();
}

static LXValue fn_cg_load_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_of");
    LXValue _v1154 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1155 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1156_val = px_null();
    int px_err_1156_proped = 0;
    px_srcline(139);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1154}, 1))) {
        px_srcline(140);
        return px_add(px_add(px_str("px_cell_get("), _v1155), px_str(")"));
    }
    px_srcline(141);
    return _v1155;
px_err_1156:
    if (px_err_1156_proped) return px_err_1156_val;
    return px_null();
}

static LXValue fn_cg_store_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_store_of");
    LXValue _v1157 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1158 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1159 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1160_val = px_null();
    int px_err_1160_proped = 0;
    px_srcline(143);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1157}, 1))) {
        px_srcline(144);
        return px_add(px_add(px_add(px_add(px_str("px_cell_set("), _v1158), px_str(", ")), _v1159), px_str(")"));
    }
    px_srcline(145);
    return px_add(px_add(_v1158, px_str(" = ")), _v1159);
px_err_1160:
    if (px_err_1160_proped) return px_err_1160_val;
    return px_null();
}

static LXValue fn_cg_inited_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_new");
    LXValue px_err_1161_val = px_null();
    int px_err_1161_proped = 0;
    px_srcline(148);
    return px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
px_err_1161:
    if (px_err_1161_proped) return px_err_1161_val;
    return px_null();
}

static LXValue fn_cg_inited_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_add");
    LXValue _v1162 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1163_val = px_null();
    int px_err_1163_proped = 0;
    px_srcline(150);
    px_index_set(px_get_global("cg_inited"), _v1162, px_int(1LL));
px_err_1163:
    if (px_err_1163_proped) return px_err_1163_val;
    return px_null();
}

static LXValue fn_cg_inited_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_copy");
    LXValue px_err_1164_val = px_null();
    int px_err_1164_proped = 0;
    px_srcline(152);
    return px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_inited")}, 1);
px_err_1164:
    if (px_err_1164_proped) return px_err_1164_val;
    return px_null();
}

static LXValue fn_cg_inited_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_restore");
    LXValue _v1165 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1166_val = px_null();
    int px_err_1166_proped = 0;
    px_srcline(154);
    px_set_global("cg_inited", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1165}, 1));
px_err_1166:
    if (px_err_1166_proped) return px_err_1166_val;
    return px_null();
}

static LXValue fn_cg_inited_intersect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_intersect");
    LXValue _v1167 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1168 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1169 = px_uninit();
    LXValue _v1170 = px_uninit();
    LXValue _v1171 = px_uninit();
    LXValue px_err_1172_val = px_null();
    int px_err_1172_proped = 0;
    px_srcline(157);
    _v1169 = px_call(px_get_global("cg_inited_new"), (LXValue[]){}, 0);
    px_srcline(158);
    _v1170 = px_method(_v1167, "keys", (LXValue[]){}, 0);
    px_srcline(159);
    _v1171 = px_int(0LL);
    px_srcline(160);
    while (px_is_truthy(px_lt(_v1171, px_call(px_get_global("len"), (LXValue[]){_v1170}, 1)))) {
        px_srcline(161);
        if (px_is_truthy(px_method(_v1168, "has", (LXValue[]){px_index(_v1170, _v1171)}, 1))) {
            px_srcline(162);
            px_index_set(_v1169, px_index(_v1170, _v1171), px_int(1LL));
        }
        px_srcline(163);
         _v1171 = px_add(_v1171, px_int(1LL));
    }
    px_srcline(164);
    return _v1169;
px_err_1172:
    if (px_err_1172_proped) return px_err_1172_val;
    return px_null();
}

static LXValue fn_cg_load_ck(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_ck");
    LXValue _v1173 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1174 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1175_val = px_null();
    int px_err_1175_proped = 0;
    px_srcline(167);
    if (px_is_truthy(px_method(px_get_global("cg_inited"), "has", (LXValue[]){_v1173}, 1))) {
        px_srcline(168);
        return px_call(px_get_global("cg_load_of"), (LXValue[]){_v1173, _v1174}, 2);
    }
    px_srcline(169);
    return px_add(px_add(px_add(px_add(px_str("px_chk_uninit("), px_call(px_get_global("cg_load_of"), (LXValue[]){_v1173, _v1174}, 2)), px_str(", \"")), _v1173), px_str("\")"));
px_err_1175:
    if (px_err_1175_proped) return px_err_1175_val;
    return px_null();
}

static LXValue fn_cg_name_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_name_add");
    LXValue _v1176 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1177 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1178_val = px_null();
    int px_err_1178_proped = 0;
    px_srcline(171);
    if (px_is_truthy(({ LXValue _t1179 = px_ne(_v1177, px_str("")); px_is_truthy(_t1179) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1176, _v1177}, 2)) : _t1179; }))) {
        px_srcline(172);
        (void)(px_method(_v1176, "append", (LXValue[]){_v1177}, 1));
    }
px_err_1178:
    if (px_err_1178_proped) return px_err_1178_val;
    return px_null();
}

static LXValue fn_cg_ast_used(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_used");
    LXValue _v1180 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1181 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1182 = px_uninit();
    LXValue px_err_1183_val = px_null();
    int px_err_1183_proped = 0;
    px_srcline(175);
    if (px_is_truthy(({ LXValue _t1184 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1180}, 1), px_str("list")); px_is_truthy(_t1184) ? _t1184 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1180}, 1), px_int(0LL)); }))) {
        px_srcline(176);
        return px_null();
    }
    px_srcline(177);
    if (px_is_truthy(({ LXValue _t1187 = ({ LXValue _t1186 = ({ LXValue _t1185 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1180, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1185) ? px_eq(px_index(_v1180, px_int(0LL)), px_str("Var")) : _t1185; }); px_is_truthy(_t1186) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1180}, 1), px_int(2LL)) : _t1186; }); px_is_truthy(_t1187) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1180, px_int(1LL))}, 1), px_str("string")) : _t1187; }))) {
        px_srcline(178);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1181, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1180, px_int(1LL))}, 1)}, 2));
        px_srcline(179);
        return px_null();
    }
    px_srcline(180);
    LXValue _t1188 = _v1180;
    int _il7 = (int)px_len(_t1188);
    for (int _t1189 = 0; _t1189 < _il7; _t1189++) {
        px_iter_ck(_t1188, _il7);
        _v1182 = px_iter_at(_t1188, px_int(_t1189));
        px_srcline(181);
        (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1182, _v1181}, 2));
    }
px_err_1183:
    if (px_err_1183_proped) return px_err_1183_val;
    return px_null();
}

static LXValue fn_cg_ast_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_bound");
    LXValue _v1190 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1191 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1192 = px_uninit();
    LXValue _v1193 = px_uninit();
    LXValue _v1194 = px_uninit();
    LXValue _v1195 = px_uninit();
    LXValue _v1196 = px_uninit();
    LXValue px_err_1197_val = px_null();
    int px_err_1197_proped = 0;
    px_srcline(184);
    if (px_is_truthy(({ LXValue _t1198 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1190}, 1), px_str("list")); px_is_truthy(_t1198) ? _t1198 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1190}, 1), px_int(0LL)); }))) {
        px_srcline(185);
        return px_null();
    }
    px_srcline(186);
    _v1192 = px_index(_v1190, px_int(0LL));
    px_srcline(187);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1192}, 1), px_str("string")))) {
        px_srcline(188);
        if (px_is_truthy(px_eq(_v1192, px_str("VarDecl")))) {
            px_srcline(189);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1191, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1190, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1192, px_str("For")))) {
            px_srcline(202);
            _v1193 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1190, px_int(1LL))}, 1);
            px_srcline(203);
            _v1194 = px_int(0LL);
            px_srcline(204);
            while (px_is_truthy(px_lt(_v1194, px_call(px_get_global("len"), (LXValue[]){_v1193}, 1)))) {
                px_srcline(205);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1191, px_index(_v1193, _v1194)}, 2));
                px_srcline(206);
                 _v1194 = px_add(_v1194, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1192, px_str("Closure")))) {
            px_srcline(208);
            LXValue _t1199 = px_index(_v1190, px_int(1LL));
            int _il8 = (int)px_len(_t1199);
            for (int _t1200 = 0; _t1200 < _il8; _t1200++) {
                px_iter_ck(_t1199, _il8);
                _v1195 = px_iter_at(_t1199, px_int(_t1200));
                px_srcline(209);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1191, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1195, px_int(1LL))}, 1)}, 2));
            }
        }
    }
    px_srcline(210);
    LXValue _t1201 = _v1190;
    int _il9 = (int)px_len(_t1201);
    for (int _t1202 = 0; _t1202 < _il9; _t1202++) {
        px_iter_ck(_t1201, _il9);
        _v1196 = px_iter_at(_t1201, px_int(_t1202));
        px_srcline(211);
        (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){_v1196, _v1191}, 2));
    }
px_err_1197:
    if (px_err_1197_proped) return px_err_1197_val;
    return px_null();
}

static LXValue fn_cg_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_closure_caps");
    LXValue _v1203 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1204 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1205 = px_uninit();
    LXValue _v1206 = px_uninit();
    LXValue _v1207 = px_uninit();
    LXValue _v1208 = px_uninit();
    LXValue px_err_1209_val = px_null();
    int px_err_1209_proped = 0;
    px_srcline(214);
    _v1205 = px_list_n((LXValue[]){}, 0);
    px_srcline(215);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){px_index(_v1203, px_int(3LL)), _v1205}, 2));
    px_srcline(216);
    _v1206 = px_list_n((LXValue[]){}, 0);
    px_srcline(217);
    (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){px_index(_v1203, px_int(3LL)), _v1206}, 2));
    px_srcline(218);
    LXValue _t1210 = px_index(_v1203, px_int(1LL));
    int _il10 = (int)px_len(_t1210);
    for (int _t1211 = 0; _t1211 < _il10; _t1211++) {
        px_iter_ck(_t1210, _il10);
        _v1207 = px_iter_at(_t1210, px_int(_t1211));
        px_srcline(219);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1206, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1207, px_int(1LL))}, 1)}, 2));
    }
    px_srcline(220);
    LXValue _t1212 = _v1205;
    int _il11 = (int)px_len(_t1212);
    for (int _t1213 = 0; _t1213 < _il11; _t1213++) {
        px_iter_ck(_t1212, _il11);
        _v1208 = px_iter_at(_t1212, px_int(_t1213));
        px_srcline(221);
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1206, _v1208}, 2)))) {
            px_srcline(222);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1204, _v1208}, 2));
        }
    }
px_err_1209:
    if (px_err_1209_proped) return px_err_1209_val;
    return px_null();
}

static LXValue fn_cg_scan_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_scan_closure_caps");
    LXValue _v1214 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1215 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1216 = px_uninit();
    LXValue px_err_1217_val = px_null();
    int px_err_1217_proped = 0;
    px_srcline(225);
    if (px_is_truthy(({ LXValue _t1218 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1214}, 1), px_str("list")); px_is_truthy(_t1218) ? _t1218 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1214}, 1), px_int(0LL)); }))) {
        px_srcline(226);
        return px_null();
    }
    px_srcline(227);
    if (px_is_truthy(({ LXValue _t1219 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1214, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1219) ? px_eq(px_index(_v1214, px_int(0LL)), px_str("Closure")) : _t1219; }))) {
        px_srcline(228);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){_v1214, _v1215}, 2));
    }
    px_srcline(229);
    if (px_is_truthy(({ LXValue _t1220 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1214, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1220) ? px_eq(px_index(_v1214, px_int(0LL)), px_str("FuncDef")) : _t1220; }))) {
        px_srcline(233);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), px_index(_v1214, px_int(2LL)), px_index(_v1214, px_int(3LL)), px_index(_v1214, px_int(4LL)), px_list_n((LXValue[]){}, 0), px_index(_v1214, px_int(5LL))}, 6), _v1215}, 2));
    }
    px_srcline(234);
    LXValue _t1221 = _v1214;
    int _il12 = (int)px_len(_t1221);
    for (int _t1222 = 0; _t1222 < _il12; _t1222++) {
        px_iter_ck(_t1221, _il12);
        _v1216 = px_iter_at(_t1221, px_int(_t1222));
        px_srcline(235);
        (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1216, _v1215}, 2));
    }
px_err_1217:
    if (px_err_1217_proped) return px_err_1217_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_mark_immutable");
    LXValue _v1223 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1224_val = px_null();
    int px_err_1224_proped = 0;
    px_srcline(238);
    px_index_set(px_get_global("cg_immutables"), _v1223, px_int(1LL));
px_err_1224:
    if (px_err_1224_proped) return px_err_1224_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_immutable");
    LXValue _v1225 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1226_val = px_null();
    int px_err_1226_proped = 0;
    px_srcline(240);
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1225}, 1);
px_err_1226:
    if (px_err_1226_proped) return px_err_1226_val;
    return px_null();
}

static LXValue fn_cg_is_wildcard(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_wildcard");
    LXValue _v1227 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1228_val = px_null();
    int px_err_1228_proped = 0;
    px_srcline(248);
    return px_eq(_v1227, px_str("_"));
px_err_1228:
    if (px_err_1228_proped) return px_err_1228_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_perr");
    LXValue _v1229 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1230 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1231_val = px_null();
    int px_err_1231_proped = 0;
    px_srcline(253);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1229), px_str(": ")), _v1230)}, 1));
    px_srcline(254);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1231:
    if (px_err_1231_proped) return px_err_1231_val;
    return px_null();
}

static LXValue fn_cg_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pwarn");
    LXValue _v1232 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1233_val = px_null();
    int px_err_1233_proped = 0;
    px_srcline(257);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1232)}, 1));
px_err_1233:
    if (px_err_1233_proped) return px_err_1233_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_nonnull_ty");
    LXValue _v1234 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1235_val = px_null();
    int px_err_1235_proped = 0;
    px_srcline(261);
    if (px_is_truthy(px_eq(_v1234, px_null()))) {
        px_srcline(262);
        return px_bool(false);
    }
    px_srcline(263);
    if (px_is_truthy(px_eq(px_index(_v1234, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(264);
        return px_bool(false);
    }
    px_srcline(265);
    return px_bool(true);
px_err_1235:
    if (px_err_1235_proped) return px_err_1235_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_null_lit");
    LXValue _v1236 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1237_val = px_null();
    int px_err_1237_proped = 0;
    px_srcline(268);
    if (px_is_truthy(px_eq(_v1236, px_null()))) {
        px_srcline(269);
        return px_bool(false);
    }
    px_srcline(270);
    if (px_is_truthy(px_eq(px_index(_v1236, px_int(0LL)), px_str("Null")))) {
        px_srcline(271);
        return px_bool(true);
    }
    px_srcline(272);
    return px_bool(false);
px_err_1237:
    if (px_err_1237_proped) return px_err_1237_val;
    return px_null();
}

static LXValue fn_cg_sem_vardecl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_vardecl");
    LXValue _v1238 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1239 = px_uninit();
    LXValue _v1240 = px_uninit();
    LXValue px_err_1241_val = px_null();
    int px_err_1241_proped = 0;
    px_srcline(290);
    _v1239 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1238, px_int(2LL))}, 1);
    px_srcline(292);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1239}, 1))) {
        px_srcline(293);
        return px_null();
    }
    px_srcline(294);
    if (px_is_truthy(({ LXValue _t1242 = px_eq(px_index(_v1238, px_int(1LL)), px_str("Let")); px_is_truthy(_t1242) ? _t1242 : px_eq(px_index(_v1238, px_int(1LL)), px_str("Const")); }))) {
        px_srcline(295);
        (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v1239}, 1));
    }
    px_srcline(296);
    _v1240 = px_index(_v1238, px_int(3LL));
    px_srcline(297);
    if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v1240}, 1))) {
        px_srcline(298);
        px_index_set(px_get_global("cg_nonnull"), _v1239, px_int(1LL));
        px_srcline(299);
        if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v1238, px_int(4LL))}, 1))) {
            px_srcline(300);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(({ LXValue _s189 = px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1240}, 1)), px_str("'（可空类型请用 ")); LXValue _s190 = px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1240}, 1); px_add(_s189, _s190); }), px_str("? 声明）"))}, 2));
        }
    }
px_err_1241:
    if (px_err_1241_proped) return px_err_1241_val;
    return px_null();
}

static LXValue fn_cg_sem_assign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_assign");
    LXValue _v1243 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1244 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1245 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1246 = px_uninit();
    LXValue px_err_1247_val = px_null();
    int px_err_1247_proped = 0;
    px_srcline(304);
    if (px_is_truthy(px_eq(_v1243, px_null()))) {
        px_srcline(305);
        return px_null();
    }
    px_srcline(306);
    if (px_is_truthy(px_ne(px_index(_v1243, px_int(0LL)), px_str("Var")))) {
        px_srcline(307);
        return px_null();
    }
    px_srcline(308);
    _v1246 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1243, px_int(1LL))}, 1);
    px_srcline(310);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1246}, 1))) {
        px_srcline(311);
        return px_null();
    }
    px_srcline(312);
    if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v1246}, 1))) {
        px_srcline(313);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v1246), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
    }
    px_srcline(314);
    if (px_is_truthy(({ LXValue _t1248 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){_v1245}, 1); px_is_truthy(_t1248) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v1246}, 1) : _t1248; }))) {
        px_srcline(315);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v1246), px_str("'（可空类型请声明为 ")), _v1246), px_str(": T?）"))}, 2));
    }
px_err_1247:
    if (px_err_1247_proped) return px_err_1247_val;
    return px_null();
}

static LXValue fn_cg_sem_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_call");
    LXValue _v1249 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1250 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1251 = px_uninit();
    LXValue _v1252 = px_uninit();
    LXValue px_err_1253_val = px_null();
    int px_err_1253_proped = 0;
    px_srcline(318);
    if (px_is_truthy(px_eq(_v1249, px_null()))) {
        px_srcline(319);
        return px_null();
    }
    px_srcline(320);
    if (px_is_truthy(px_ne(px_index(_v1249, px_int(0LL)), px_str("Var")))) {
        px_srcline(321);
        return px_null();
    }
    px_srcline(322);
    _v1251 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1249, px_int(1LL))}, 1);
    px_srcline(323);
    if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v1251}, 1))) {
        px_srcline(324);
        _v1252 = px_index(px_get_global("cg_ffi"), _v1251);
        px_srcline(325);
        if (px_is_truthy(({ LXValue _s191 = px_call(px_get_global("len"), (LXValue[]){_v1250}, 1); LXValue _s192 = px_call(px_get_global("len"), (LXValue[]){_v1252}, 1); px_ne(_s191, _s192); }))) {
            px_srcline(326);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), ({ LXValue _s193 = px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v1251), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1252}, 1)}, 1)), px_str(" 个参数，给出 ")); LXValue _s194 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1250}, 1)}, 1); px_add(_s193, _s194); })}, 2));
        }
    }
px_err_1253:
    if (px_err_1253_proped) return px_err_1253_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ty_name");
    LXValue _v1254 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1255_val = px_null();
    int px_err_1255_proped = 0;
    px_srcline(329);
    if (px_is_truthy(px_eq(_v1254, px_null()))) {
        px_srcline(330);
        return px_str("any");
    }
    px_srcline(331);
    if (px_is_truthy(px_eq(px_index(_v1254, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(332);
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1254, px_int(1LL))}, 1), px_str("?"));
    }
    px_srcline(333);
    if (px_is_truthy(px_eq(px_index(_v1254, px_int(0LL)), px_str("TyNamed")))) {
        px_srcline(334);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1254, px_int(1LL))}, 1);
    }
    px_srcline(335);
    if (px_is_truthy(px_eq(px_index(_v1254, px_int(0LL)), px_str("TyList")))) {
        px_srcline(336);
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1254, px_int(1LL))}, 1)), px_str("]"));
    }
    px_srcline(337);
    if (px_is_truthy(px_eq(px_index(_v1254, px_int(0LL)), px_str("TyDict")))) {
        px_srcline(338);
        return px_add(({ LXValue _s195 = px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1254, px_int(1LL))}, 1)), px_str(": ")); LXValue _s196 = px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1254, px_int(2LL))}, 1); px_add(_s195, _s196); }), px_str("}"));
    }
    px_srcline(339);
    return px_str("any");
px_err_1255:
    if (px_err_1255_proped) return px_err_1255_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_func_cname");
    LXValue _v1256 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1257 = px_uninit();
    LXValue _v1258 = px_uninit();
    LXValue _v1259 = px_uninit();
    LXValue _v1260 = px_uninit();
    LXValue _v1261 = px_uninit();
    LXValue _v1262 = px_uninit();
    LXValue px_err_1263_val = px_null();
    int px_err_1263_proped = 0;
    px_srcline(341);
    _v1257 = px_list_n((LXValue[]){}, 0);
    px_srcline(342);
    _v1258 = px_int(0LL);
    px_srcline(343);
    while (px_is_truthy(px_lt(_v1258, px_call(px_get_global("len"), (LXValue[]){_v1256}, 1)))) {
        px_srcline(344);
        _v1259 = px_index(_v1256, _v1258);
        px_srcline(345);
        _v1260 = ({ LXValue _t1264 = px_ge(_v1259, px_str("a")); px_is_truthy(_t1264) ? px_le(_v1259, px_str("z")) : _t1264; });
        px_srcline(346);
        _v1261 = ({ LXValue _t1265 = px_ge(_v1259, px_str("A")); px_is_truthy(_t1265) ? px_le(_v1259, px_str("Z")) : _t1265; });
        px_srcline(347);
        _v1262 = ({ LXValue _t1266 = px_ge(_v1259, px_str("0")); px_is_truthy(_t1266) ? px_le(_v1259, px_str("9")) : _t1266; });
        px_srcline(348);
        if (px_is_truthy(({ LXValue _t1268 = ({ LXValue _t1267 = _v1260; px_is_truthy(_t1267) ? _t1267 : _v1261; }); px_is_truthy(_t1268) ? _t1268 : _v1262; }))) {
            px_srcline(349);
            (void)(px_method(_v1257, "append", (LXValue[]){_v1259}, 1));
        }
        else {
            px_srcline(351);
            (void)(px_method(_v1257, "append", (LXValue[]){px_str("_")}, 1));
        }
        px_srcline(352);
         _v1258 = px_add(_v1258, px_int(1LL));
    }
    px_srcline(353);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1257}, 2);
px_err_1263:
    if (px_err_1263_proped) return px_err_1263_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find");
    LXValue _v1269 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1270 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1271 = px_uninit();
    LXValue _v1272 = px_uninit();
    LXValue _v1273 = px_uninit();
    LXValue _v1274 = px_uninit();
    LXValue _v1275 = px_uninit();
    LXValue px_err_1276_val = px_null();
    int px_err_1276_proped = 0;
    px_srcline(355);
    _v1271 = px_call(px_get_global("len"), (LXValue[]){_v1269}, 1);
    px_srcline(356);
    _v1272 = px_call(px_get_global("len"), (LXValue[]){_v1270}, 1);
    px_srcline(357);
    _v1273 = px_int(0LL);
    px_srcline(358);
    while (px_is_truthy(px_le(px_add(_v1273, _v1272), _v1271))) {
        px_srcline(359);
        _v1274 = px_int(0LL);
        px_srcline(360);
        _v1275 = px_bool(true);
        px_srcline(361);
        while (px_is_truthy(px_lt(_v1274, _v1272))) {
            px_srcline(362);
            if (px_is_truthy(px_ne(px_index(_v1269, px_add(_v1273, _v1274)), px_index(_v1270, _v1274)))) {
                px_srcline(363);
                 _v1275 = px_bool(false);
                px_srcline(364);
                break;
            }
            px_srcline(365);
             _v1274 = px_add(_v1274, px_int(1LL));
        }
        px_srcline(366);
        if (px_is_truthy(_v1275)) {
            px_srcline(367);
            return _v1273;
        }
        px_srcline(368);
         _v1273 = px_add(_v1273, px_int(1LL));
    }
    px_srcline(369);
    return px_neg(px_int(1LL));
px_err_1276:
    if (px_err_1276_proped) return px_err_1276_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad");
    LXValue _v1277 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1278 = px_uninit();
    LXValue _v1279 = px_uninit();
    LXValue px_err_1280_val = px_null();
    int px_err_1280_proped = 0;
    px_srcline(371);
    _v1278 = px_list_n((LXValue[]){}, 0);
    px_srcline(372);
    _v1279 = px_int(0LL);
    px_srcline(373);
    while (px_is_truthy(px_lt(_v1279, _v1277))) {
        px_srcline(374);
        (void)(px_method(_v1278, "append", (LXValue[]){px_str("    ")}, 1));
        px_srcline(375);
         _v1279 = px_add(_v1279, px_int(1LL));
    }
    px_srcline(376);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1278}, 2);
px_err_1280:
    if (px_err_1280_proped) return px_err_1280_val;
    return px_null();
}

static LXValue fn_cg_nul_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_nul_char");
    LXValue px_err_1281_val = px_null();
    int px_err_1281_proped = 0;
    px_srcline(383);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){px_int(0LL), px_int(1LL)}, 2)}, 1);
px_err_1281:
    if (px_err_1281_proped) return px_err_1281_val;
    return px_null();
}

static LXValue fn_cg_has_nul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_has_nul");
    LXValue _v1282 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1283 = px_uninit();
    LXValue _v1284 = px_uninit();
    LXValue px_err_1285_val = px_null();
    int px_err_1285_proped = 0;
    px_srcline(386);
    _v1283 = px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0);
    px_srcline(387);
    _v1284 = px_int(0LL);
    px_srcline(388);
    while (px_is_truthy(px_lt(_v1284, px_call(px_get_global("len"), (LXValue[]){_v1282}, 1)))) {
        px_srcline(389);
        if (px_is_truthy(px_eq(px_index(_v1282, _v1284), _v1283))) {
            px_srcline(390);
            return px_bool(true);
        }
        px_srcline(391);
         _v1284 = px_add(_v1284, px_int(1LL));
    }
    px_srcline(392);
    return px_bool(false);
px_err_1285:
    if (px_err_1285_proped) return px_err_1285_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_unescape");
    LXValue _v1286 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1287 = px_uninit();
    LXValue _v1288 = px_uninit();
    LXValue _v1289 = px_uninit();
    LXValue _v1290 = px_uninit();
    LXValue _v1291 = px_uninit();
    LXValue _v1292 = px_uninit();
    LXValue _v1293 = px_uninit();
    LXValue px_err_1294_val = px_null();
    int px_err_1294_proped = 0;
    px_srcline(394);
    _v1287 = px_slice(_v1286, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1286}, 1), px_int(1LL)), px_null());
    px_srcline(395);
    _v1288 = px_list_n((LXValue[]){}, 0);
    px_srcline(396);
    _v1289 = px_int(0LL);
    px_srcline(397);
    while (px_is_truthy(px_lt(_v1289, px_call(px_get_global("len"), (LXValue[]){_v1287}, 1)))) {
        px_srcline(398);
        _v1290 = px_index(_v1287, _v1289);
        px_srcline(399);
        if (px_is_truthy(px_eq(_v1290, px_str("\\")))) {
            px_srcline(400);
            _v1291 = px_index(_v1287, px_add(_v1289, px_int(1LL)));
            px_srcline(401);
            if (px_is_truthy(px_eq(_v1291, px_str("n")))) {
                px_srcline(402);
                (void)(px_method(_v1288, "append", (LXValue[]){px_str("\n")}, 1));
                px_srcline(403);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1291, px_str("t")))) {
                px_srcline(405);
                (void)(px_method(_v1288, "append", (LXValue[]){px_str("\t")}, 1));
                px_srcline(406);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1291, px_str("r")))) {
                px_srcline(408);
                (void)(px_method(_v1288, "append", (LXValue[]){px_str("\r")}, 1));
                px_srcline(409);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1291, px_str("0")))) {
                px_srcline(411);
                (void)(px_method(_v1288, "append", (LXValue[]){px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)}, 1));
                px_srcline(412);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1291, px_str("\"")))) {
                px_srcline(414);
                (void)(px_method(_v1288, "append", (LXValue[]){px_str("\"")}, 1));
                px_srcline(415);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1291, px_str("\\")))) {
                px_srcline(417);
                (void)(px_method(_v1288, "append", (LXValue[]){px_str("\\")}, 1));
                px_srcline(418);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1291, px_str("u")))) {
                px_srcline(420);
                _v1292 = px_add(_v1289, px_int(3LL));
                px_srcline(421);
                _v1293 = px_list_n((LXValue[]){}, 0);
                px_srcline(422);
                while (px_is_truthy(({ LXValue _t1295 = px_lt(_v1292, px_call(px_get_global("len"), (LXValue[]){_v1287}, 1)); px_is_truthy(_t1295) ? px_ne(px_index(_v1287, _v1292), px_str("}")) : _t1295; }))) {
                    px_srcline(423);
                    (void)(px_method(_v1293, "append", (LXValue[]){px_index(_v1287, _v1292)}, 1));
                    px_srcline(424);
                     _v1292 = px_add(_v1292, px_int(1LL));
                }
                px_srcline(425);
                (void)(px_method(_v1288, "append", (LXValue[]){px_call(px_get_global("hex_to_char"), (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1293}, 2)}, 1)}, 1));
                px_srcline(426);
                 _v1289 = px_add(_v1292, px_int(1LL));
            }
            else {
                px_srcline(428);
                (void)(px_method(_v1288, "append", (LXValue[]){_v1291}, 1));
                px_srcline(429);
                 _v1289 = px_add(_v1289, px_int(2LL));
            }
        }
        else {
            px_srcline(431);
            (void)(px_method(_v1288, "append", (LXValue[]){_v1290}, 1));
            px_srcline(432);
             _v1289 = px_add(_v1289, px_int(1LL));
        }
    }
    px_srcline(433);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1288}, 2);
px_err_1294:
    if (px_err_1294_proped) return px_err_1294_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_escape_str");
    LXValue _v1296 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1297 = px_uninit();
    LXValue _v1298 = px_uninit();
    LXValue _v1299 = px_uninit();
    LXValue px_err_1300_val = px_null();
    int px_err_1300_proped = 0;
    px_srcline(436);
    _v1297 = px_list_n((LXValue[]){}, 0);
    px_srcline(437);
    _v1298 = px_int(0LL);
    px_srcline(438);
    while (px_is_truthy(px_lt(_v1298, px_call(px_get_global("len"), (LXValue[]){_v1296}, 1)))) {
        px_srcline(439);
        _v1299 = px_index(_v1296, _v1298);
        px_srcline(440);
        if (px_is_truthy(px_eq(_v1299, px_str("\\")))) {
            px_srcline(441);
            (void)(px_method(_v1297, "append", (LXValue[]){px_str("\\\\")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1299, px_str("\"")))) {
            px_srcline(443);
            (void)(px_method(_v1297, "append", (LXValue[]){px_str("\\\"")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1299, px_str("\n")))) {
            px_srcline(445);
            (void)(px_method(_v1297, "append", (LXValue[]){px_str("\\n")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1299, px_str("\r")))) {
            px_srcline(447);
            (void)(px_method(_v1297, "append", (LXValue[]){px_str("\\r")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1299, px_str("\t")))) {
            px_srcline(449);
            (void)(px_method(_v1297, "append", (LXValue[]){px_str("\\t")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1299, px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)))) {
            px_srcline(457);
            (void)(px_method(_v1297, "append", (LXValue[]){px_str("\\000")}, 1));
        }
        else {
            px_srcline(459);
            (void)(px_method(_v1297, "append", (LXValue[]){_v1299}, 1));
        }
        px_srcline(460);
         _v1298 = px_add(_v1298, px_int(1LL));
    }
    px_srcline(461);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1297}, 2);
px_err_1300:
    if (px_err_1300_proped) return px_err_1300_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad_zeros");
    LXValue _v1301 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1302 = px_uninit();
    LXValue _v1303 = px_uninit();
    LXValue px_err_1304_val = px_null();
    int px_err_1304_proped = 0;
    px_srcline(464);
    _v1302 = px_list_n((LXValue[]){}, 0);
    px_srcline(465);
    _v1303 = px_int(0LL);
    px_srcline(466);
    while (px_is_truthy(px_lt(_v1303, _v1301))) {
        px_srcline(467);
        (void)(px_method(_v1302, "append", (LXValue[]){px_str("0")}, 1));
        px_srcline(468);
         _v1303 = px_add(_v1303, px_int(1LL));
    }
    px_srcline(469);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1302}, 2);
px_err_1304:
    if (px_err_1304_proped) return px_err_1304_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_expand_sci");
    LXValue _v1305 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1306 = px_uninit();
    LXValue _v1307 = px_uninit();
    LXValue _v1308 = px_uninit();
    LXValue _v1309 = px_uninit();
    LXValue _v1310 = px_uninit();
    LXValue _v1311 = px_uninit();
    LXValue _v1312 = px_uninit();
    LXValue _v1313 = px_uninit();
    LXValue _v1314 = px_uninit();
    LXValue _v1315 = px_uninit();
    LXValue _v1316 = px_uninit();
    LXValue _v1317 = px_uninit();
    LXValue _v1318 = px_uninit();
    LXValue _v1319 = px_uninit();
    LXValue px_err_1320_val = px_null();
    int px_err_1320_proped = 0;
    px_srcline(471);
    _v1306 = px_neg(px_int(1LL));
    px_srcline(472);
    _v1307 = px_int(0LL);
    px_srcline(473);
    while (px_is_truthy(px_lt(_v1307, px_call(px_get_global("len"), (LXValue[]){_v1305}, 1)))) {
        px_srcline(474);
        if (px_is_truthy(({ LXValue _t1321 = px_eq(px_index(_v1305, _v1307), px_str("e")); px_is_truthy(_t1321) ? _t1321 : px_eq(px_index(_v1305, _v1307), px_str("E")); }))) {
            px_srcline(475);
             _v1306 = _v1307;
            px_srcline(476);
            break;
        }
        px_srcline(477);
         _v1307 = px_add(_v1307, px_int(1LL));
    }
    px_srcline(478);
    if (px_is_truthy(px_lt(_v1306, px_int(0LL)))) {
        px_srcline(479);
        return _v1305;
    }
    px_srcline(480);
    _v1308 = px_slice(_v1305, px_int(0LL), _v1306, px_null());
    px_srcline(481);
    _v1309 = px_slice(_v1305, px_add(_v1306, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1305}, 1), px_null());
    px_srcline(482);
    _v1310 = px_int(1LL);
    px_srcline(483);
    if (px_is_truthy(({ LXValue _t1322 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1309}, 1), px_int(0LL)); px_is_truthy(_t1322) ? px_eq(px_index(_v1309, px_int(0LL)), px_str("+")) : _t1322; }))) {
        px_srcline(484);
         _v1309 = px_slice(_v1309, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1309}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1323 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1309}, 1), px_int(0LL)); px_is_truthy(_t1323) ? px_eq(px_index(_v1309, px_int(0LL)), px_str("-")) : _t1323; }))) {
        px_srcline(486);
         _v1310 = px_neg(px_int(1LL));
        px_srcline(487);
         _v1309 = px_slice(_v1309, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1309}, 1), px_null());
    }
    px_srcline(488);
    _v1311 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1309}, 1), _v1310);
    px_srcline(489);
    _v1312 = px_bool(false);
    px_srcline(490);
    if (px_is_truthy(({ LXValue _t1324 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1308}, 1), px_int(0LL)); px_is_truthy(_t1324) ? px_eq(px_index(_v1308, px_int(0LL)), px_str("-")) : _t1324; }))) {
        px_srcline(491);
         _v1312 = px_bool(true);
        px_srcline(492);
         _v1308 = px_slice(_v1308, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1308}, 1), px_null());
    }
    px_srcline(493);
    _v1313 = px_str("");
    px_srcline(494);
    _v1314 = px_str("");
    px_srcline(495);
    _v1315 = px_neg(px_int(1LL));
    px_srcline(496);
    _v1316 = px_int(0LL);
    px_srcline(497);
    while (px_is_truthy(px_lt(_v1316, px_call(px_get_global("len"), (LXValue[]){_v1308}, 1)))) {
        px_srcline(498);
        if (px_is_truthy(px_eq(px_index(_v1308, _v1316), px_str(".")))) {
            px_srcline(499);
             _v1315 = _v1316;
            px_srcline(500);
            break;
        }
        px_srcline(501);
         _v1316 = px_add(_v1316, px_int(1LL));
    }
    px_srcline(502);
    if (px_is_truthy(px_lt(_v1315, px_int(0LL)))) {
        px_srcline(503);
         _v1313 = _v1308;
    }
    else {
        px_srcline(505);
         _v1313 = px_slice(_v1308, px_int(0LL), _v1315, px_null());
        px_srcline(506);
         _v1314 = px_slice(_v1308, px_add(_v1315, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1308}, 1), px_null());
    }
    px_srcline(507);
    _v1317 = px_add(_v1313, _v1314);
    px_srcline(508);
    _v1318 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1313}, 1), _v1311);
    px_srcline(509);
    _v1319 = px_str("");
    px_srcline(510);
    if (px_is_truthy(px_le(_v1318, px_int(0LL)))) {
        px_srcline(511);
         _v1319 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1318)}, 1)), _v1317);
    }
    else if (px_is_truthy(px_ge(_v1318, px_call(px_get_global("len"), (LXValue[]){_v1317}, 1)))) {
        px_srcline(513);
         _v1319 = px_add(_v1317, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1318, px_call(px_get_global("len"), (LXValue[]){_v1317}, 1))}, 1));
    }
    else {
        px_srcline(515);
         _v1319 = px_add(px_add(px_slice(_v1317, px_int(0LL), _v1318, px_null()), px_str(".")), px_slice(_v1317, _v1318, px_call(px_get_global("len"), (LXValue[]){_v1317}, 1), px_null()));
    }
    px_srcline(516);
    if (px_is_truthy(_v1312)) {
        px_srcline(517);
        return px_add(px_str("-"), _v1319);
    }
    px_srcline(518);
    return _v1319;
px_err_1320:
    if (px_err_1320_proped) return px_err_1320_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_fmt_float");
    LXValue _v1325 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1326 = px_uninit();
    LXValue _v1327 = px_uninit();
    LXValue _v1328 = px_uninit();
    LXValue px_err_1329_val = px_null();
    int px_err_1329_proped = 0;
    px_srcline(521);
    _v1326 = px_call(px_get_global("str"), (LXValue[]){_v1325}, 1);
    px_srcline(522);
    if (px_is_truthy(({ LXValue _t1332 = ({ LXValue _t1331 = ({ LXValue _t1330 = px_eq(_v1326, px_str("inf")); px_is_truthy(_t1330) ? _t1330 : px_eq(_v1326, px_str("-inf")); }); px_is_truthy(_t1331) ? _t1331 : px_eq(_v1326, px_str("nan")); }); px_is_truthy(_t1332) ? _t1332 : px_eq(_v1326, px_str("-nan")); }))) {
        px_srcline(523);
        return _v1326;
    }
    px_srcline(524);
     _v1326 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1326}, 1);
    px_srcline(525);
    _v1327 = px_call(px_get_global("len"), (LXValue[]){_v1326}, 1);
    px_srcline(526);
    if (px_is_truthy(({ LXValue _t1333 = px_ge(_v1327, px_int(2LL)); px_is_truthy(_t1333) ? px_eq(px_slice(_v1326, px_sub(_v1327, px_int(2LL)), _v1327, px_null()), px_str(".0")) : _t1333; }))) {
        px_srcline(532);
        _v1328 = px_slice(_v1326, px_int(0LL), px_sub(_v1327, px_int(2LL)), px_null());
        px_srcline(533);
        if (px_is_truthy(px_eq(_v1328, px_str("-0")))) {
            px_srcline(534);
            return _v1326;
        }
        px_srcline(535);
        return _v1328;
    }
    px_srcline(543);
    if (px_is_truthy(({ LXValue _t1334 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1326, px_str(".")}, 2)); px_is_truthy(_t1334) ? px_ge(_v1327, px_int(19LL)) : _t1334; }))) {
        px_srcline(544);
        return px_add(_v1326, px_str(".0"));
    }
    px_srcline(545);
    return _v1326;
px_err_1329:
    if (px_err_1329_proped) return px_err_1329_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_types");
    LXValue _v1335 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1336 = px_uninit();
    LXValue _v1337 = px_uninit();
    LXValue _v1338 = px_uninit();
    LXValue _v1339 = px_uninit();
    LXValue _v1340 = px_uninit();
    LXValue _v1341 = px_uninit();
    LXValue _v1342 = px_uninit();
    LXValue _v1343 = px_uninit();
    LXValue _v1344 = px_uninit();
    LXValue _v1345 = px_uninit();
    LXValue px_err_1346_val = px_null();
    int px_err_1346_proped = 0;
    px_srcline(548);
    _v1336 = px_index(_v1335, px_int(1LL));
    px_srcline(549);
    _v1337 = px_int(0LL);
    px_srcline(550);
    while (px_is_truthy(px_lt(_v1337, px_call(px_get_global("len"), (LXValue[]){_v1336}, 1)))) {
        px_srcline(551);
        _v1338 = px_index(_v1336, _v1337);
        px_srcline(552);
        _v1339 = px_index(_v1338, px_int(0LL));
        px_srcline(553);
        if (px_is_truthy(px_eq(_v1339, px_str("StructDef")))) {
            px_srcline(554);
            _v1340 = px_list_n((LXValue[]){}, 0);
            px_srcline(555);
            _v1341 = px_int(0LL);
            px_srcline(556);
            while (px_is_truthy(px_lt(_v1341, px_call(px_get_global("len"), (LXValue[]){px_index(_v1338, px_int(2LL))}, 1)))) {
                px_srcline(557);
                (void)(px_method(_v1340, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1338, px_int(2LL)), _v1341), px_int(1LL))}, 1)}, 1));
                px_srcline(558);
                 _v1341 = px_add(_v1341, px_int(1LL));
            }
            px_srcline(559);
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1338, px_int(1LL))}, 1), _v1340);
        }
        else if (px_is_truthy(px_eq(_v1339, px_str("EnumDef")))) {
            px_srcline(561);
            _v1342 = px_list_n((LXValue[]){}, 0);
            px_srcline(562);
            _v1343 = px_int(0LL);
            px_srcline(563);
            while (px_is_truthy(px_lt(_v1343, px_call(px_get_global("len"), (LXValue[]){px_index(_v1338, px_int(2LL))}, 1)))) {
                px_srcline(564);
                (void)(px_method(_v1342, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1338, px_int(2LL)), _v1343), px_int(1LL))}, 1)}, 1));
                px_srcline(565);
                 _v1343 = px_add(_v1343, px_int(1LL));
            }
            px_srcline(566);
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1338, px_int(1LL))}, 1), _v1342);
        }
        else if (px_is_truthy(px_eq(_v1339, px_str("ImplDef")))) {
            px_srcline(568);
            _v1344 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1338, px_int(1LL))}, 1);
            px_srcline(569);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1344}, 1))) {
                px_srcline(570);
                _v1345 = px_int(0LL);
                px_srcline(571);
                while (px_is_truthy(px_lt(_v1345, px_call(px_get_global("len"), (LXValue[]){px_index(_v1338, px_int(3LL))}, 1)))) {
                    px_srcline(572);
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1344), "append", (LXValue[]){px_index(px_index(_v1338, px_int(3LL)), _v1345)}, 1));
                    px_srcline(573);
                     _v1345 = px_add(_v1345, px_int(1LL));
                }
            }
            else {
                px_srcline(575);
                px_index_set(px_get_global("cg_impls"), _v1344, px_index(_v1338, px_int(3LL)));
            }
        }
        px_srcline(576);
         _v1337 = px_add(_v1337, px_int(1LL));
    }
    px_srcline(579);
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1336}, 1));
px_err_1346:
    if (px_err_1346_proped) return px_err_1346_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_consts");
    LXValue _v1347 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1348 = px_uninit();
    LXValue _v1349 = px_uninit();
    LXValue _v1350 = px_uninit();
    LXValue _v1351 = px_uninit();
    LXValue _v1352 = px_uninit();
    LXValue _v1353 = px_uninit();
    LXValue _v1354 = px_uninit();
    LXValue _v1355 = px_uninit();
    LXValue px_err_1356_val = px_null();
    int px_err_1356_proped = 0;
    px_srcline(582);
    _v1348 = px_int(0LL);
    px_srcline(583);
    while (px_is_truthy(px_lt(_v1348, px_call(px_get_global("len"), (LXValue[]){_v1347}, 1)))) {
        px_srcline(584);
        _v1349 = px_index(_v1347, _v1348);
        px_srcline(585);
        _v1350 = px_index(_v1349, px_int(0LL));
        px_srcline(586);
        if (px_is_truthy(px_eq(_v1350, px_str("TypeConst")))) {
            px_srcline(587);
            _v1351 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1349, px_int(1LL))}, 1);
            px_srcline(588);
            _v1352 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            px_srcline(589);
            _v1353 = px_int(0LL);
            px_srcline(590);
            while (px_is_truthy(px_lt(_v1353, px_call(px_get_global("len"), (LXValue[]){px_index(_v1349, px_int(2LL))}, 1)))) {
                px_srcline(591);
                _v1354 = px_index(px_index(_v1349, px_int(2LL)), _v1353);
                px_srcline(592);
                ({ LXValue _s197 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1354, px_int(1LL))}, 1); LXValue _s198 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1354, px_int(2LL))}, 1); px_index_set(_v1352, _s197, _s198); });
                px_srcline(593);
                 _v1353 = px_add(_v1353, px_int(1LL));
            }
            px_srcline(594);
            px_index_set(px_get_global("cg_const_enums"), _v1351, _v1352);
        }
        else if (px_is_truthy(px_eq(_v1350, px_str("FuncDef")))) {
            px_srcline(596);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1349, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1350, px_str("If")))) {
            px_srcline(598);
            _v1355 = px_int(0LL);
            px_srcline(599);
            while (px_is_truthy(px_lt(_v1355, px_call(px_get_global("len"), (LXValue[]){px_index(_v1349, px_int(1LL))}, 1)))) {
                px_srcline(600);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1349, px_int(1LL)), _v1355), px_int(1LL))}, 1));
                px_srcline(601);
                 _v1355 = px_add(_v1355, px_int(1LL));
            }
            px_srcline(602);
            if (px_is_truthy(px_ne(px_index(_v1349, px_int(2LL)), px_null()))) {
                px_srcline(603);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1349, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1350, px_str("For")))) {
            px_srcline(605);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1349, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1350, px_str("While")))) {
            px_srcline(607);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1349, px_int(2LL))}, 1));
        }
        px_srcline(608);
         _v1348 = px_add(_v1348, px_int(1LL));
    }
px_err_1356:
    if (px_err_1356_proped) return px_err_1356_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_hoist_vars");
    LXValue _v1357 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1358 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1359 = px_uninit();
    LXValue _v1360 = px_uninit();
    LXValue _v1361 = px_uninit();
    LXValue _v1362 = px_uninit();
    LXValue _v1363 = px_uninit();
    LXValue _v1364 = px_uninit();
    LXValue _v1365 = px_uninit();
    LXValue _v1366 = px_uninit();
    LXValue _v1367 = px_uninit();
    LXValue px_err_1368_val = px_null();
    int px_err_1368_proped = 0;
    px_srcline(617);
    _v1359 = px_int(0LL);
    px_srcline(618);
    while (px_is_truthy(px_lt(_v1359, px_call(px_get_global("len"), (LXValue[]){_v1357}, 1)))) {
        px_srcline(619);
        _v1360 = px_index(_v1357, _v1359);
        px_srcline(620);
        _v1361 = px_index(_v1360, px_int(0LL));
        px_srcline(621);
        if (px_is_truthy(px_eq(_v1361, px_str("Assign")))) {
            px_srcline(622);
            _v1362 = px_index(_v1360, px_int(1LL));
            px_srcline(623);
            if (px_is_truthy(px_eq(px_index(_v1362, px_int(0LL)), px_str("Var")))) {
                px_srcline(624);
                _v1363 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1362, px_int(1LL))}, 1);
                px_srcline(625);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1358, _v1363}, 2)))) {
                    px_srcline(626);
                    (void)(px_method(_v1358, "append", (LXValue[]){_v1363}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1361, px_str("VarDecl")))) {
            px_srcline(628);
            _v1363 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1360, px_int(2LL))}, 1);
            px_srcline(629);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1358, _v1363}, 2)))) {
                px_srcline(630);
                (void)(px_method(_v1358, "append", (LXValue[]){_v1363}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1361, px_str("FuncDef")))) {
            px_srcline(635);
            _v1363 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1360, px_int(1LL))}, 1);
            px_srcline(636);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1358, _v1363}, 2)))) {
                px_srcline(637);
                (void)(px_method(_v1358, "append", (LXValue[]){_v1363}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1361, px_str("If")))) {
            px_srcline(639);
            _v1364 = px_index(_v1360, px_int(1LL));
            px_srcline(640);
            _v1365 = px_int(0LL);
            px_srcline(641);
            while (px_is_truthy(px_lt(_v1365, px_call(px_get_global("len"), (LXValue[]){_v1364}, 1)))) {
                px_srcline(642);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1364, _v1365), px_int(1LL)), _v1358}, 2));
                px_srcline(643);
                 _v1365 = px_add(_v1365, px_int(1LL));
            }
            px_srcline(644);
            if (px_is_truthy(px_ne(px_index(_v1360, px_int(2LL)), px_null()))) {
                px_srcline(645);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1360, px_int(2LL)), _v1358}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1361, px_str("For")))) {
            px_srcline(648);
            _v1366 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1360, px_int(1LL))}, 1);
            px_srcline(649);
            _v1367 = px_int(0LL);
            px_srcline(650);
            while (px_is_truthy(px_lt(_v1367, px_call(px_get_global("len"), (LXValue[]){_v1366}, 1)))) {
                px_srcline(651);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1358, px_index(_v1366, _v1367)}, 2)))) {
                    px_srcline(652);
                    (void)(px_method(_v1358, "append", (LXValue[]){px_index(_v1366, _v1367)}, 1));
                }
                px_srcline(653);
                 _v1367 = px_add(_v1367, px_int(1LL));
            }
            px_srcline(654);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1360, px_int(3LL)), _v1358}, 2));
        }
        else if (px_is_truthy(px_eq(_v1361, px_str("While")))) {
            px_srcline(656);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1360, px_int(2LL)), _v1358}, 2));
        }
        px_srcline(657);
         _v1359 = px_add(_v1359, px_int(1LL));
    }
px_err_1368:
    if (px_err_1368_proped) return px_err_1368_val;
    return px_null();
}

static LXValue fn_cg_collect_decl_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_decl_vars");
    LXValue _v1369 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1370 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1371 = px_uninit();
    LXValue _v1372 = px_uninit();
    LXValue _v1373 = px_uninit();
    LXValue _v1374 = px_uninit();
    LXValue _v1375 = px_uninit();
    LXValue _v1376 = px_uninit();
    LXValue _v1377 = px_uninit();
    LXValue _v1378 = px_uninit();
    LXValue _v1379 = px_uninit();
    LXValue px_err_1380_val = px_null();
    int px_err_1380_proped = 0;
    px_srcline(668);
    _v1371 = px_int(0LL);
    px_srcline(669);
    while (px_is_truthy(px_lt(_v1371, px_call(px_get_global("len"), (LXValue[]){_v1369}, 1)))) {
        px_srcline(670);
        _v1372 = px_index(_v1369, _v1371);
        px_srcline(671);
        _v1373 = px_index(_v1372, px_int(0LL));
        px_srcline(672);
        if (px_is_truthy(px_eq(_v1373, px_str("VarDecl")))) {
            px_srcline(673);
            _v1374 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1372, px_int(2LL))}, 1);
            px_srcline(674);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1370, _v1374}, 2)))) {
                px_srcline(675);
                (void)(px_method(_v1370, "append", (LXValue[]){_v1374}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1373, px_str("FuncDef")))) {
            px_srcline(677);
            _v1374 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1372, px_int(1LL))}, 1);
            px_srcline(678);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1370, _v1374}, 2)))) {
                px_srcline(679);
                (void)(px_method(_v1370, "append", (LXValue[]){_v1374}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1373, px_str("ChanDecl")))) {
            px_srcline(681);
            _v1375 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1372, px_int(1LL))}, 1);
            px_srcline(682);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1370, _v1375}, 2)))) {
                px_srcline(683);
                (void)(px_method(_v1370, "append", (LXValue[]){_v1375}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1373, px_str("If")))) {
            px_srcline(685);
            _v1376 = px_index(_v1372, px_int(1LL));
            px_srcline(686);
            _v1377 = px_int(0LL);
            px_srcline(687);
            while (px_is_truthy(px_lt(_v1377, px_call(px_get_global("len"), (LXValue[]){_v1376}, 1)))) {
                px_srcline(688);
                (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(px_index(_v1376, _v1377), px_int(1LL)), _v1370}, 2));
                px_srcline(689);
                 _v1377 = px_add(_v1377, px_int(1LL));
            }
            px_srcline(690);
            if (px_is_truthy(px_ne(px_index(_v1372, px_int(2LL)), px_null()))) {
                px_srcline(691);
                (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1372, px_int(2LL)), _v1370}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1373, px_str("For")))) {
            px_srcline(693);
            _v1378 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1372, px_int(1LL))}, 1);
            px_srcline(694);
            _v1379 = px_int(0LL);
            px_srcline(695);
            while (px_is_truthy(px_lt(_v1379, px_call(px_get_global("len"), (LXValue[]){_v1378}, 1)))) {
                px_srcline(696);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1370, px_index(_v1378, _v1379)}, 2)))) {
                    px_srcline(697);
                    (void)(px_method(_v1370, "append", (LXValue[]){px_index(_v1378, _v1379)}, 1));
                }
                px_srcline(698);
                 _v1379 = px_add(_v1379, px_int(1LL));
            }
            px_srcline(699);
            (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1372, px_int(3LL)), _v1370}, 2));
        }
        else if (px_is_truthy(px_eq(_v1373, px_str("While")))) {
            px_srcline(701);
            (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1372, px_int(2LL)), _v1370}, 2));
        }
        px_srcline(702);
         _v1371 = px_add(_v1371, px_int(1LL));
    }
px_err_1380:
    if (px_err_1380_proped) return px_err_1380_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func");
    LXValue _v1381 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1382 = px_uninit();
    LXValue px_err_1383_val = px_null();
    int px_err_1383_proped = 0;
    px_srcline(705);
    _v1382 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1381, px_int(1LL))}, 1)}, 1));
    px_srcline(706);
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1381, _v1382}, 2);
px_err_1383:
    if (px_err_1383_proped) return px_err_1383_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func_named");
    LXValue _v1384 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1385 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1386 = px_uninit();
    LXValue _v1387 = px_uninit();
    LXValue _v1388 = px_uninit();
    LXValue _v1389 = px_uninit();
    LXValue _v1390 = px_uninit();
    LXValue _v1391 = px_uninit();
    LXValue _v1392 = px_uninit();
    LXValue _v1393 = px_uninit();
    LXValue _v1394 = px_uninit();
    LXValue _v1395 = px_uninit();
    LXValue _v1396 = px_uninit();
    LXValue _v1397 = px_uninit();
    LXValue _v1398 = px_uninit();
    LXValue _v1399 = px_uninit();
    LXValue _v1400 = px_uninit();
    LXValue _v1401 = px_uninit();
    LXValue _v1402 = px_uninit();
    LXValue _v1403 = px_uninit();
    LXValue _v1404 = px_uninit();
    LXValue _v1405 = px_uninit();
    LXValue _v1406 = px_uninit();
    LXValue _v1407 = px_uninit();
    LXValue _v1408 = px_uninit();
    LXValue _v1409 = px_uninit();
    LXValue _v1410 = px_uninit();
    LXValue _v1411 = px_uninit();
    LXValue _v1412 = px_uninit();
    LXValue px_err_1413_val = px_null();
    int px_err_1413_proped = 0;
    px_srcline(708);
    _v1386 = px_add(px_add(px_str("static LXValue "), _v1385), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(709);
     _v1386 = px_add(_v1386, px_str("    (void)ctx;\n"));
    px_srcline(712);
     _v1386 = px_add(_v1386, px_add(px_add(px_str("    px_srcfunc(\""), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1384, px_int(1LL))}, 1)), px_str("\");\n")));
    px_srcline(713);
    _v1387 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(714);
    _v1388 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(715);
    _v1389 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(716);
    _v1390 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(718);
    _v1391 = px_get_global("cg_topbody");
    px_srcline(719);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(721);
    _v1392 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(722);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(723);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(724);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(725);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(726);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1390}, 1));
    px_srcline(730);
    _v1393 = px_list_n((LXValue[]){}, 0);
    px_srcline(731);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){px_index(_v1384, px_int(4LL)), _v1393}, 2));
    px_srcline(733);
    _v1394 = px_index(_v1384, px_int(2LL));
    px_srcline(734);
    _v1395 = px_int(0LL);
    px_srcline(735);
    _v1396 = px_list_n((LXValue[]){}, 0);
    px_srcline(736);
    while (px_is_truthy(px_lt(_v1395, px_call(px_get_global("len"), (LXValue[]){_v1394}, 1)))) {
        px_srcline(737);
        _v1397 = px_index(_v1394, _v1395);
        px_srcline(738);
        _v1398 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1397, px_int(1LL))}, 1);
        px_srcline(739);
        _v1399 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1398}, 1);
        px_srcline(740);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1393, _v1398}, 2))) {
            px_srcline(741);
            px_index_set(px_get_global("cg_cells"), _v1398, px_int(1LL));
        }
        px_srcline(742);
        _v1400 = px_str("px_null()");
        px_srcline(743);
        if (px_is_truthy(px_ne(px_index(_v1397, px_int(3LL)), px_null()))) {
            px_srcline(744);
             _v1400 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1397, px_int(3LL))}, 1);
        }
        px_srcline(745);
        (void)(px_method(_v1396, "append", (LXValue[]){px_list_n((LXValue[]){_v1399, _v1398, px_add(px_add(({ LXValue _s199 = px_add(px_add(px_str("(nargs > "), px_call(px_get_global("str"), (LXValue[]){_v1395}, 1)), px_str(") ? args[")); LXValue _s200 = px_call(px_get_global("str"), (LXValue[]){_v1395}, 1); px_add(_s199, _s200); }), px_str("] : ")), _v1400)}, 3)}, 1));
        px_srcline(747);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v1398}, 1));
        px_srcline(748);
         _v1395 = px_add(_v1395, px_int(1LL));
    }
    px_srcline(751);
    _v1401 = px_list_n((LXValue[]){}, 0);
    px_srcline(752);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1384, px_int(4LL)), _v1401}, 2));
    px_srcline(755);
    _v1402 = px_list_n((LXValue[]){}, 0);
    px_srcline(756);
    (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1384, px_int(4LL)), _v1402}, 2));
    px_srcline(757);
    _v1403 = px_list_n((LXValue[]){}, 0);
    px_srcline(758);
    _v1404 = px_int(0LL);
    px_srcline(759);
    while (px_is_truthy(px_lt(_v1404, px_call(px_get_global("len"), (LXValue[]){_v1401}, 1)))) {
        px_srcline(760);
        _v1405 = px_index(_v1401, _v1404);
        px_srcline(761);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1405}, 1), px_null()))) {
            px_srcline(762);
             _v1404 = px_add(_v1404, px_int(1LL));
            px_srcline(763);
            continue;
        }
        px_srcline(764);
        if (px_is_truthy(({ LXValue _t1414 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1402, _v1405}, 2)); px_is_truthy(_t1414) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_topnames"), _v1405}, 2) : _t1414; }))) {
            px_srcline(765);
             _v1404 = px_add(_v1404, px_int(1LL));
            px_srcline(766);
            continue;
        }
        px_srcline(767);
        _v1399 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1405}, 1);
        px_srcline(768);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1393, _v1405}, 2))) {
            px_srcline(769);
            px_index_set(px_get_global("cg_cells"), _v1405, px_int(1LL));
        }
        px_srcline(770);
        (void)(px_method(_v1403, "append", (LXValue[]){px_list_n((LXValue[]){_v1399, _v1405}, 2)}, 1));
        px_srcline(771);
         _v1404 = px_add(_v1404, px_int(1LL));
    }
    px_srcline(773);
    _v1406 = px_int(0LL);
    px_srcline(774);
    while (px_is_truthy(px_lt(_v1406, px_call(px_get_global("len"), (LXValue[]){_v1396}, 1)))) {
        px_srcline(775);
        _v1407 = px_index(_v1396, _v1406);
        px_srcline(776);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1407, px_int(1LL))}, 1))) {
            px_srcline(777);
             _v1386 = px_add(_v1386, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1407, px_int(0LL))), px_str(" = px_cell(")), px_index(_v1407, px_int(2LL))), px_str(");\n")));
        }
        else {
            px_srcline(779);
             _v1386 = px_add(_v1386, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1407, px_int(0LL))), px_str(" = ")), px_index(_v1407, px_int(2LL))), px_str(";\n")));
        }
        px_srcline(780);
         _v1406 = px_add(_v1406, px_int(1LL));
    }
    px_srcline(781);
     _v1406 = px_int(0LL);
    px_srcline(782);
    while (px_is_truthy(px_lt(_v1406, px_call(px_get_global("len"), (LXValue[]){_v1403}, 1)))) {
        px_srcline(783);
        _v1408 = px_index(_v1403, _v1406);
        px_srcline(787);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1408, px_int(1LL))}, 1))) {
            px_srcline(788);
             _v1386 = px_add(_v1386, px_add(px_add(px_str("    LXValue "), px_index(_v1408, px_int(0LL))), px_str(" = px_cell(px_uninit());\n")));
        }
        else {
            px_srcline(790);
             _v1386 = px_add(_v1386, px_add(px_add(px_str("    LXValue "), px_index(_v1408, px_int(0LL))), px_str(" = px_uninit();\n")));
        }
        px_srcline(791);
         _v1406 = px_add(_v1406, px_int(1LL));
    }
    px_srcline(793);
    _v1409 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(794);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1409}, 1));
    px_srcline(795);
     _v1386 = px_add(_v1386, px_add(px_add(px_str("    LXValue "), _v1409), px_str("_val = px_null();\n")));
    px_srcline(796);
     _v1386 = px_add(_v1386, px_add(px_add(px_str("    int "), _v1409), px_str("_proped = 0;\n")));
    px_srcline(800);
    _v1410 = px_index(_v1384, px_int(4LL));
    px_srcline(801);
    _v1411 = px_list_n((LXValue[]){}, 0);
    px_srcline(802);
    _v1412 = px_int(0LL);
    px_srcline(803);
    while (px_is_truthy(px_lt(_v1412, px_call(px_get_global("len"), (LXValue[]){_v1410}, 1)))) {
        px_srcline(804);
        (void)(px_method(_v1411, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1410, _v1412), px_int(1LL)}, 2)}, 1));
        px_srcline(805);
         _v1412 = px_add(_v1412, px_int(1LL));
    }
    px_srcline(806);
     _v1386 = px_add(_v1386, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1411}, 2));
    px_srcline(807);
     _v1386 = px_add(_v1386, px_add(_v1409, px_str(":\n")));
    px_srcline(808);
     _v1386 = px_add(_v1386, px_add(px_add(px_add(px_add(px_str("    if ("), _v1409), px_str("_proped) return ")), _v1409), px_str("_val;\n")));
    px_srcline(809);
     _v1386 = px_add(_v1386, px_str("    return px_null();\n"));
    px_srcline(810);
     _v1386 = px_add(_v1386, px_str("}\n"));
    px_srcline(811);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(812);
    px_set_global("cg_vars", _v1387);
    px_srcline(813);
    px_set_global("cg_var_types", _v1388);
    px_srcline(814);
    px_set_global("cg_cells", _v1389);
    px_srcline(815);
    px_set_global("cg_immutables", _v1390);
    px_srcline(816);
    px_set_global("cg_topbody", _v1391);
    px_srcline(817);
    px_set_global("cg_inited", _v1392);
    px_srcline(818);
    return _v1386;
px_err_1413:
    if (px_err_1413_proped) return px_err_1413_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_generate");
    LXValue _v1415 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1416 = px_uninit();
    LXValue _v1417 = px_uninit();
    LXValue _v1418 = px_uninit();
    LXValue _v1419 = px_uninit();
    LXValue _v1420 = px_uninit();
    LXValue _v1421 = px_uninit();
    LXValue _v1422 = px_uninit();
    LXValue _v1423 = px_uninit();
    LXValue _v1424 = px_uninit();
    LXValue _v1425 = px_uninit();
    LXValue _v1426 = px_uninit();
    LXValue _v1427 = px_uninit();
    LXValue _v1428 = px_uninit();
    LXValue _v1429 = px_uninit();
    LXValue _v1430 = px_uninit();
    LXValue _v1431 = px_uninit();
    LXValue _v1432 = px_uninit();
    LXValue _v1433 = px_uninit();
    LXValue _v1434 = px_uninit();
    LXValue _v1435 = px_uninit();
    LXValue _v1436 = px_uninit();
    LXValue _v1437 = px_uninit();
    LXValue _v1438 = px_uninit();
    LXValue _v1439 = px_uninit();
    LXValue _v1440 = px_uninit();
    LXValue _v1441 = px_uninit();
    LXValue _v1442 = px_uninit();
    LXValue _v1443 = px_uninit();
    LXValue _v1444 = px_uninit();
    LXValue _v1445 = px_uninit();
    LXValue _v1446 = px_uninit();
    LXValue _v1447 = px_uninit();
    LXValue _v1448 = px_uninit();
    LXValue _v1449 = px_uninit();
    LXValue _v1450 = px_uninit();
    LXValue px_err_1451_val = px_null();
    int px_err_1451_proped = 0;
    px_srcline(821);
    _v1416 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_srcline(822);
    px_set_global("cg_closures", px_str(""));
    px_srcline(823);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(824);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(825);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(826);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(827);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(828);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(829);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(830);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(831);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(832);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(833);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(834);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(835);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(836);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(837);
    px_set_global("cg_seq_uid", px_int(0LL));
    px_srcline(838);
    px_set_global("cg_iter_uid", px_int(0LL));
    px_srcline(839);
    px_set_global("cg_unpack_uid", px_int(0LL));
    px_srcline(840);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(842);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(843);
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1415}, 1));
    px_srcline(845);
    _v1417 = px_index(_v1415, px_int(1LL));
    px_srcline(849);
    px_set_global("cg_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(850);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){_v1417, px_get_global("cg_topnames")}, 2));
    px_srcline(851);
    _v1418 = px_int(0LL);
    px_srcline(852);
    while (px_is_truthy(px_lt(_v1418, px_call(px_get_global("len"), (LXValue[]){_v1417}, 1)))) {
        px_srcline(853);
        _v1419 = px_index(_v1417, _v1418);
        px_srcline(854);
        _v1420 = px_index(_v1419, px_int(0LL));
        px_srcline(855);
        if (px_is_truthy(px_eq(_v1420, px_str("FuncDef")))) {
            px_srcline(856);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1419, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1420, px_str("ExternDef")))) {
            px_srcline(859);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1419, px_int(1LL))}, 1), px_index(_v1419, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1420, px_str("VarDecl")))) {
            px_srcline(861);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1419, px_int(2LL))}, 1)}, 1));
            px_srcline(862);
            if (px_is_truthy(({ LXValue _t1452 = px_eq(px_index(_v1419, px_int(1LL)), px_str("Let")); px_is_truthy(_t1452) ? _t1452 : px_eq(px_index(_v1419, px_int(1LL)), px_str("Const")); }))) {
                px_srcline(863);
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1419, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1420, px_str("Assign")))) {
            px_srcline(865);
            _v1421 = px_index(_v1419, px_int(1LL));
            px_srcline(866);
            if (px_is_truthy(px_eq(px_index(_v1421, px_int(0LL)), px_str("Var")))) {
                px_srcline(867);
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1421, px_int(1LL))}, 1)}, 1));
            }
        }
        px_srcline(868);
         _v1418 = px_add(_v1418, px_int(1LL));
    }
    px_srcline(870);
    _v1422 = px_list_n((LXValue[]){}, 0);
    px_srcline(871);
    _v1423 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    px_srcline(872);
    _v1424 = px_int(0LL);
    px_srcline(873);
    while (px_is_truthy(px_lt(_v1424, px_call(px_get_global("len"), (LXValue[]){_v1423}, 1)))) {
        px_srcline(874);
        _v1425 = px_index(_v1423, _v1424);
        px_srcline(875);
        _v1426 = px_index(px_get_global("cg_impls"), _v1425);
        px_srcline(876);
        _v1427 = px_int(0LL);
        px_srcline(877);
        while (px_is_truthy(px_lt(_v1427, px_call(px_get_global("len"), (LXValue[]){_v1426}, 1)))) {
            px_srcline(878);
            (void)(px_method(_v1422, "append", (LXValue[]){px_list_n((LXValue[]){_v1425, px_index(_v1426, _v1427)}, 2)}, 1));
            px_srcline(879);
             _v1427 = px_add(_v1427, px_int(1LL));
        }
        px_srcline(880);
         _v1424 = px_add(_v1424, px_int(1LL));
    }
    px_srcline(882);
    _v1428 = px_int(1LL);
    px_srcline(883);
    while (px_is_truthy(px_lt(_v1428, px_call(px_get_global("len"), (LXValue[]){_v1422}, 1)))) {
        px_srcline(884);
        _v1429 = _v1428;
        px_srcline(885);
        while (px_is_truthy(px_gt(_v1429, px_int(0LL)))) {
            px_srcline(886);
            _v1430 = px_add(px_add(px_index(px_index(_v1422, px_sub(_v1429, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1422, px_sub(_v1429, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(887);
            _v1431 = px_add(px_add(px_index(px_index(_v1422, _v1429), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1422, _v1429), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(888);
            if (px_is_truthy(px_lt(_v1431, _v1430))) {
                px_srcline(889);
                _v1432 = px_index(_v1422, _v1429);
                px_srcline(890);
                px_index_set(_v1422, _v1429, px_index(_v1422, px_sub(_v1429, px_int(1LL))));
                px_srcline(891);
                px_index_set(_v1422, px_sub(_v1429, px_int(1LL)), _v1432);
            }
            px_srcline(892);
             _v1429 = px_sub(_v1429, px_int(1LL));
        }
        px_srcline(893);
         _v1428 = px_add(_v1428, px_int(1LL));
    }
    px_srcline(898);
    _v1433 = px_list_n((LXValue[]){}, 0);
    px_srcline(899);
    _v1434 = px_int(0LL);
    px_srcline(900);
    while (px_is_truthy(px_lt(_v1434, px_call(px_get_global("len"), (LXValue[]){_v1422}, 1)))) {
        px_srcline(901);
        _v1425 = px_index(px_index(_v1422, _v1434), px_int(0LL));
        px_srcline(902);
        _v1435 = px_index(px_index(_v1422, _v1434), px_int(1LL));
        px_srcline(903);
        _v1436 = ({ LXValue _s201 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1425}, 1)), px_str("_")); LXValue _s202 = px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1435, px_int(1LL))}, 1)}, 1); px_add(_s201, _s202); });
        px_srcline(904);
        (void)(px_method(_v1433, "push", (LXValue[]){px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1435, _v1436}, 2)}, 1));
        px_srcline(905);
        (void)(px_method(_v1433, "push", (LXValue[]){px_str("\n")}, 1));
        px_srcline(906);
         _v1434 = px_add(_v1434, px_int(1LL));
    }
    px_srcline(908);
    _v1437 = px_int(0LL);
    px_srcline(909);
    while (px_is_truthy(px_lt(_v1437, px_call(px_get_global("len"), (LXValue[]){_v1417}, 1)))) {
        px_srcline(910);
        _v1419 = px_index(_v1417, _v1437);
        px_srcline(911);
        if (px_is_truthy(px_eq(px_index(_v1419, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(912);
            (void)(px_method(_v1433, "push", (LXValue[]){px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1419}, 1)}, 1));
            px_srcline(913);
            (void)(px_method(_v1433, "push", (LXValue[]){px_str("\n")}, 1));
        }
        px_srcline(914);
         _v1437 = px_add(_v1437, px_int(1LL));
    }
    px_srcline(915);
     _v1416 = px_add(_v1416, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1433}, 2));
    px_srcline(917);
     _v1416 = px_add(_v1416, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(918);
     _v1416 = px_add(_v1416, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(919);
     _v1416 = px_add(_v1416, px_str("    px_register_builtins();\n"));
    px_srcline(921);
    _v1438 = px_int(0LL);
    px_srcline(922);
    while (px_is_truthy(px_lt(_v1438, px_call(px_get_global("len"), (LXValue[]){_v1417}, 1)))) {
        px_srcline(923);
        _v1419 = px_index(_v1417, _v1438);
        px_srcline(924);
        if (px_is_truthy(px_eq(px_index(_v1419, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(925);
            _v1436 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1419, px_int(1LL))}, 1);
            px_srcline(926);
            _v1439 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1436}, 1));
            px_srcline(927);
             _v1416 = px_add(_v1416, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1436), px_str("\", px_func(\"")), _v1436), px_str("\", ")), _v1439), px_str(", NULL));\n")));
        }
        px_srcline(928);
         _v1438 = px_add(_v1438, px_int(1LL));
    }
    px_srcline(930);
    _v1440 = px_int(0LL);
    px_srcline(931);
    while (px_is_truthy(px_lt(_v1440, px_call(px_get_global("len"), (LXValue[]){_v1422}, 1)))) {
        px_srcline(932);
        _v1425 = px_index(px_index(_v1422, _v1440), px_int(0LL));
        px_srcline(933);
        _v1435 = px_index(px_index(_v1422, _v1440), px_int(1LL));
        px_srcline(934);
        _v1441 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1435, px_int(1LL))}, 1);
        px_srcline(935);
        _v1436 = ({ LXValue _s203 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1425}, 1)), px_str("_")); LXValue _s204 = px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1441}, 1); px_add(_s203, _s204); });
        px_srcline(936);
         _v1416 = px_add(_v1416, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1425), px_str(".")), _v1441), px_str("\", px_func(\"")), _v1425), px_str(".")), _v1441), px_str("\", ")), _v1436), px_str(", NULL));\n")));
        px_srcline(937);
         _v1440 = px_add(_v1440, px_int(1LL));
    }
    px_srcline(940);
    _v1442 = px_list_n((LXValue[]){}, 0);
    px_srcline(942);
    px_set_global("cg_topbody", px_bool(true));
    px_srcline(943);
    _v1443 = px_int(0LL);
    px_srcline(944);
    while (px_is_truthy(px_lt(_v1443, px_call(px_get_global("len"), (LXValue[]){_v1417}, 1)))) {
        px_srcline(945);
        _v1419 = px_index(_v1417, _v1443);
        px_srcline(946);
        _v1420 = px_index(_v1419, px_int(0LL));
        px_srcline(947);
        if (px_is_truthy(({ LXValue _t1458 = ({ LXValue _t1457 = ({ LXValue _t1456 = ({ LXValue _t1455 = ({ LXValue _t1454 = ({ LXValue _t1453 = px_ne(_v1420, px_str("FuncDef")); px_is_truthy(_t1453) ? px_ne(_v1420, px_str("StructDef")) : _t1453; }); px_is_truthy(_t1454) ? px_ne(_v1420, px_str("EnumDef")) : _t1454; }); px_is_truthy(_t1455) ? px_ne(_v1420, px_str("TraitDef")) : _t1455; }); px_is_truthy(_t1456) ? px_ne(_v1420, px_str("ImplDef")) : _t1456; }); px_is_truthy(_t1457) ? px_ne(_v1420, px_str("Import")) : _t1457; }); px_is_truthy(_t1458) ? px_ne(_v1420, px_str("ExternDef")) : _t1458; }))) {
            px_srcline(948);
            (void)(px_method(_v1442, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1419, px_int(1LL)}, 2)}, 1));
        }
        px_srcline(949);
         _v1443 = px_add(_v1443, px_int(1LL));
    }
    px_srcline(950);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(951);
     _v1416 = px_add(_v1416, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1442}, 2));
    px_srcline(953);
    _v1444 = px_bool(false);
    px_srcline(954);
    _v1445 = px_int(0LL);
    px_srcline(955);
    while (px_is_truthy(px_lt(_v1445, px_call(px_get_global("len"), (LXValue[]){_v1417}, 1)))) {
        px_srcline(956);
        _v1419 = px_index(_v1417, _v1445);
        px_srcline(957);
        if (px_is_truthy(({ LXValue _t1459 = px_eq(px_index(_v1419, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1459) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1419, px_int(1LL))}, 1), px_str("main")) : _t1459; }))) {
            px_srcline(958);
             _v1444 = px_bool(true);
            px_srcline(959);
            break;
        }
        px_srcline(960);
         _v1445 = px_add(_v1445, px_int(1LL));
    }
    px_srcline(961);
    if (px_is_truthy(_v1444)) {
        px_srcline(962);
        _v1439 = px_str("fn_main");
        px_srcline(963);
         _v1416 = px_add(_v1416, px_add(px_add(px_str("    { LXValue _r = "), _v1439), px_str("(NULL, 0, NULL); int _code = 0;\n")));
        px_srcline(964);
         _v1416 = px_add(_v1416, px_str("      if (px_is_result(_r)) {\n"));
        px_srcline(965);
         _v1416 = px_add(_v1416, px_str("        if (!px_result_ok(_r)) {\n"));
        px_srcline(966);
         _v1416 = px_add(_v1416, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
        px_srcline(967);
         _v1416 = px_add(_v1416, px_str("          _code = 1;\n"));
        px_srcline(968);
         _v1416 = px_add(_v1416, px_str("        } else {\n"));
        px_srcline(969);
         _v1416 = px_add(_v1416, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
        px_srcline(970);
         _v1416 = px_add(_v1416, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
        px_srcline(971);
         _v1416 = px_add(_v1416, px_str("        }\n"));
        px_srcline(972);
         _v1416 = px_add(_v1416, px_str("      } else if (_r.type == PX_INT) {\n"));
        px_srcline(973);
         _v1416 = px_add(_v1416, px_str("        _code = (int)_r.as.i;\n"));
        px_srcline(974);
         _v1416 = px_add(_v1416, px_str("      }\n"));
        px_srcline(975);
         _v1416 = px_add(_v1416, px_str("      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）\n"));
        px_srcline(976);
         _v1416 = px_add(_v1416, px_str("    }\n"));
    }
    else {
        px_srcline(978);
         _v1416 = px_add(_v1416, px_str("    return px_exit_code_final(0);   // M120（qg-issue 76 E1）\n"));
    }
    px_srcline(979);
     _v1416 = px_add(_v1416, px_str("}\n"));
    px_srcline(981);
    _v1446 = px_call(px_get_global("cg_find"), (LXValue[]){_v1416, px_str("int main(")}, 2);
    px_srcline(982);
    if (px_is_truthy(px_ge(_v1446, px_int(0LL)))) {
        px_srcline(983);
        _v1447 = px_slice(_v1416, px_int(0LL), _v1446, px_null());
        px_srcline(984);
        _v1448 = px_slice(_v1416, _v1446, px_call(px_get_global("len"), (LXValue[]){_v1416}, 1), px_null());
        px_srcline(985);
        _v1449 = px_call(px_get_global("cg_find"), (LXValue[]){_v1447, px_str("static LXValue")}, 2);
        px_srcline(986);
        _v1450 = px_str("");
        px_srcline(987);
        if (px_is_truthy(px_ge(_v1449, px_int(0LL)))) {
            px_srcline(988);
             _v1450 = px_add(px_add(px_add(px_add(px_slice(_v1447, px_int(0LL), _v1449, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1447, _v1449, px_call(px_get_global("len"), (LXValue[]){_v1447}, 1), px_null())), _v1448);
        }
        else {
            px_srcline(990);
             _v1450 = px_add(px_add(px_add(_v1447, px_get_global("cg_closures")), px_str("\n")), _v1448);
        }
        px_srcline(991);
        return _v1450;
    }
    px_srcline(992);
    return _v1416;
px_err_1451:
    if (px_err_1451_proped) return px_err_1451_val;
    return px_null();
}

static LXValue fn_bc_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_dict");
    LXValue _v1460 = px_uninit();
    LXValue px_err_1461_val = px_null();
    int px_err_1461_proped = 0;
    px_srcline(41);
    _v1460 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(42);
    (void)(px_method(_v1460, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(43);
    return _v1460;
px_err_1461:
    if (px_err_1461_proped) return px_err_1461_val;
    return px_null();
}

static LXValue fn_bc_k_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_find");
    LXValue _v1462 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1463 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1464 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1465 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1466 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1467 = px_uninit();
    LXValue _v1468 = px_uninit();
    LXValue px_err_1469_val = px_null();
    int px_err_1469_proped = 0;
    px_srcline(47);
    _v1467 = px_int(0LL);
    px_srcline(48);
    while (px_is_truthy(px_lt(_v1467, px_call(px_get_global("len"), (LXValue[]){_v1462}, 1)))) {
        px_srcline(49);
        _v1468 = px_index(_v1462, _v1467);
        px_srcline(50);
        if (px_is_truthy(px_eq(px_index(_v1468, px_str("kind")), _v1463))) {
            px_srcline(51);
            if (px_is_truthy(px_eq(_v1463, px_str("int")))) {
                px_srcline(52);
                if (px_is_truthy(px_eq(px_index(_v1468, px_str("i")), _v1464))) {
                    px_srcline(53);
                    return _v1467;
                }
            }
            else if (px_is_truthy(px_eq(_v1463, px_str("float")))) {
                px_srcline(59);
                if (px_is_truthy(({ LXValue _s205 = px_call(px_get_global("float64_bits"), (LXValue[]){px_index(_v1468, px_str("f"))}, 1); LXValue _s206 = px_call(px_get_global("float64_bits"), (LXValue[]){_v1465}, 1); px_eq(_s205, _s206); }))) {
                    px_srcline(60);
                    return _v1467;
                }
            }
            else if (px_is_truthy(px_eq(_v1463, px_str("str")))) {
                px_srcline(62);
                if (px_is_truthy(px_eq(px_index(_v1468, px_str("s")), _v1466))) {
                    px_srcline(63);
                    return _v1467;
                }
            }
            else {
                px_srcline(66);
                if (px_is_truthy(px_eq(px_index(_v1468, px_str("i")), _v1464))) {
                    px_srcline(67);
                    return _v1467;
                }
            }
        }
        px_srcline(68);
         _v1467 = px_add(_v1467, px_int(1LL));
    }
    px_srcline(69);
    return px_neg(px_int(1LL));
px_err_1469:
    if (px_err_1469_proped) return px_err_1469_val;
    return px_null();
}

static LXValue fn_bc_k_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_add");
    LXValue _v1470 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1471 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1472 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1473 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1474 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1475 = px_uninit();
    LXValue _v1476 = px_uninit();
    LXValue px_err_1477_val = px_null();
    int px_err_1477_proped = 0;
    px_srcline(71);
    _v1475 = px_call(px_get_global("bc_k_find"), (LXValue[]){_v1470, _v1471, _v1472, _v1473, _v1474}, 5);
    px_srcline(72);
    if (px_is_truthy(px_ge(_v1475, px_int(0LL)))) {
        px_srcline(73);
        return _v1475;
    }
    px_srcline(74);
    _v1476 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(75);
    px_index_set(_v1476, px_str("kind"), _v1471);
    px_srcline(76);
    px_index_set(_v1476, px_str("i"), _v1472);
    px_srcline(77);
    px_index_set(_v1476, px_str("f"), _v1473);
    px_srcline(78);
    px_index_set(_v1476, px_str("s"), _v1474);
    px_srcline(79);
    (void)(px_method(_v1470, "push", (LXValue[]){_v1476}, 1));
    px_srcline(80);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1470}, 1), px_int(1LL));
px_err_1477:
    if (px_err_1477_proped) return px_err_1477_val;
    return px_null();
}

static LXValue fn_bc_n_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_n_add");
    LXValue _v1478 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1479 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1480 = px_uninit();
    LXValue px_err_1481_val = px_null();
    int px_err_1481_proped = 0;
    px_srcline(83);
    _v1480 = px_int(0LL);
    px_srcline(84);
    while (px_is_truthy(px_lt(_v1480, px_call(px_get_global("len"), (LXValue[]){_v1478}, 1)))) {
        px_srcline(85);
        if (px_is_truthy(px_eq(px_index(_v1478, _v1480), _v1479))) {
            px_srcline(86);
            return _v1480;
        }
        px_srcline(87);
         _v1480 = px_add(_v1480, px_int(1LL));
    }
    px_srcline(88);
    (void)(px_method(_v1478, "push", (LXValue[]){_v1479}, 1));
    px_srcline(89);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1478}, 1), px_int(1LL));
px_err_1481:
    if (px_err_1481_proped) return px_err_1481_val;
    return px_null();
}

static LXValue fn_bc_g_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_g_add");
    LXValue _v1482 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1483 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1484 = px_uninit();
    LXValue px_err_1485_val = px_null();
    int px_err_1485_proped = 0;
    px_srcline(92);
    _v1484 = px_int(0LL);
    px_srcline(93);
    while (px_is_truthy(px_lt(_v1484, px_call(px_get_global("len"), (LXValue[]){_v1482}, 1)))) {
        px_srcline(94);
        if (px_is_truthy(px_eq(px_index(_v1482, _v1484), _v1483))) {
            px_srcline(95);
            return _v1484;
        }
        px_srcline(96);
         _v1484 = px_add(_v1484, px_int(1LL));
    }
    px_srcline(97);
    (void)(px_method(_v1482, "push", (LXValue[]){_v1483}, 1));
    px_srcline(98);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1482}, 1), px_int(1LL));
px_err_1485:
    if (px_err_1485_proped) return px_err_1485_val;
    return px_null();
}

static LXValue fn_bc_is_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_is_global");
    LXValue _v1486 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1487_val = px_null();
    int px_err_1487_proped = 0;
    px_srcline(101);
    if (px_is_truthy(px_eq(px_get_global("g_bcm"), px_null()))) {
        px_srcline(102);
        return px_bool(false);
    }
    px_srcline(103);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1486}, 2);
px_err_1487:
    if (px_err_1487_proped) return px_err_1487_val;
    return px_null();
}

static LXValue fn_bc_new_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_module");
    LXValue _v1488 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1489 = px_uninit();
    LXValue px_err_1490_val = px_null();
    int px_err_1490_proped = 0;
    px_srcline(106);
    _v1489 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(107);
    px_index_set(_v1489, px_str("name"), _v1488);
    px_srcline(108);
    px_index_set(_v1489, px_str("k_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(109);
    px_index_set(_v1489, px_str("n_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(110);
    px_index_set(_v1489, px_str("globals"), px_list_n((LXValue[]){}, 0));
    px_srcline(111);
    px_index_set(_v1489, px_str("funcs"), px_list_n((LXValue[]){}, 0));
    px_srcline(112);
    px_index_set(_v1489, px_str("top"), px_neg(px_int(1LL)));
    px_srcline(115);
    px_index_set(_v1489, px_str("structs"), px_list_n((LXValue[]){}, 0));
    px_srcline(116);
    px_index_set(_v1489, px_str("enums"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(117);
    px_index_set(_v1489, px_str("consts"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(118);
    px_index_set(_v1489, px_str("nclosure"), px_int(0LL));
    px_srcline(119);
    return _v1489;
px_err_1490:
    if (px_err_1490_proped) return px_err_1490_val;
    return px_null();
}

static LXValue fn_bc_struct_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_struct_index");
    LXValue _v1491 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1492 = px_uninit();
    LXValue px_err_1493_val = px_null();
    int px_err_1493_proped = 0;
    px_srcline(122);
    _v1492 = px_int(0LL);
    px_srcline(123);
    while (px_is_truthy(px_lt(_v1492, px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("structs"))}, 1)))) {
        px_srcline(124);
        if (px_is_truthy(px_eq(px_index(px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1492), px_str("name")), _v1491))) {
            px_srcline(125);
            return _v1492;
        }
        px_srcline(126);
         _v1492 = px_add(_v1492, px_int(1LL));
    }
    px_srcline(127);
    return px_neg(px_int(1LL));
px_err_1493:
    if (px_err_1493_proped) return px_err_1493_val;
    return px_null();
}

static LXValue fn_bc_enum_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_enum_has");
    LXValue _v1494 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1495 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1496_val = px_null();
    int px_err_1496_proped = 0;
    px_srcline(130);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1494}, 1)))) {
        px_srcline(131);
        return px_bool(false);
    }
    px_srcline(132);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_index(px_get_global("g_bcm"), px_str("enums")), _v1494), _v1495}, 2);
px_err_1496:
    if (px_err_1496_proped) return px_err_1496_val;
    return px_null();
}

static LXValue fn_bc_const_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_const_find");
    LXValue _v1497 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1498 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1499 = px_uninit();
    LXValue _v1500 = px_uninit();
    LXValue px_err_1501_val = px_null();
    int px_err_1501_proped = 0;
    px_srcline(135);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1497}, 1)))) {
        px_srcline(136);
        return px_null();
    }
    px_srcline(137);
    _v1499 = px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1497);
    px_srcline(138);
    _v1500 = px_int(0LL);
    px_srcline(139);
    while (px_is_truthy(px_lt(_v1500, px_call(px_get_global("len"), (LXValue[]){_v1499}, 1)))) {
        px_srcline(140);
        if (px_is_truthy(px_eq(px_index(px_index(_v1499, _v1500), px_str("n")), _v1498))) {
            px_srcline(141);
            return px_index(px_index(_v1499, _v1500), px_str("e"));
        }
        px_srcline(142);
         _v1500 = px_add(_v1500, px_int(1LL));
    }
    px_srcline(143);
    return px_null();
px_err_1501:
    if (px_err_1501_proped) return px_err_1501_val;
    return px_null();
}

static LXValue fn_bc_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_consts");
    LXValue _v1502 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1503 = px_uninit();
    LXValue _v1504 = px_uninit();
    LXValue _v1505 = px_uninit();
    LXValue _v1506 = px_uninit();
    LXValue _v1507 = px_uninit();
    LXValue _v1508 = px_uninit();
    LXValue _v1509 = px_uninit();
    LXValue _v1510 = px_uninit();
    LXValue _v1511 = px_uninit();
    LXValue _v1512 = px_uninit();
    LXValue px_err_1513_val = px_null();
    int px_err_1513_proped = 0;
    px_srcline(146);
    _v1503 = px_int(0LL);
    px_srcline(147);
    while (px_is_truthy(px_lt(_v1503, px_call(px_get_global("len"), (LXValue[]){_v1502}, 1)))) {
        px_srcline(148);
        _v1504 = px_index(_v1502, _v1503);
        px_srcline(149);
        _v1505 = px_index(_v1504, px_int(0LL));
        px_srcline(150);
        if (px_is_truthy(px_eq(_v1505, px_str("TypeConst")))) {
            px_srcline(151);
            _v1506 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1504, px_int(1LL))}, 1);
            px_srcline(152);
            if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1506}, 1)))) {
                px_srcline(153);
                px_index_set(px_index(px_get_global("g_bcm"), px_str("consts")), _v1506, px_list_n((LXValue[]){}, 0));
            }
            px_srcline(154);
            _v1507 = px_int(0LL);
            px_srcline(155);
            while (px_is_truthy(px_lt(_v1507, px_call(px_get_global("len"), (LXValue[]){px_index(_v1504, px_int(2LL))}, 1)))) {
                px_srcline(156);
                _v1508 = px_index(px_index(_v1504, px_int(2LL)), _v1507);
                px_srcline(157);
                _v1509 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
                px_srcline(158);
                px_index_set(_v1509, px_str("n"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1508, px_int(1LL))}, 1));
                px_srcline(159);
                px_index_set(_v1509, px_str("e"), px_index(_v1508, px_int(2LL)));
                px_srcline(160);
                (void)(px_method(px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1506), "push", (LXValue[]){_v1509}, 1));
                px_srcline(161);
                 _v1507 = px_add(_v1507, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1505, px_str("FuncDef")))) {
            px_srcline(163);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1504, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1505, px_str("ImplDef")))) {
            px_srcline(165);
            _v1510 = px_int(0LL);
            px_srcline(166);
            while (px_is_truthy(px_lt(_v1510, px_call(px_get_global("len"), (LXValue[]){px_index(_v1504, px_int(3LL))}, 1)))) {
                px_srcline(167);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1504, px_int(3LL)), _v1510), px_int(4LL))}, 1));
                px_srcline(168);
                 _v1510 = px_add(_v1510, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1505, px_str("If")))) {
            px_srcline(170);
            _v1511 = px_index(_v1504, px_int(1LL));
            px_srcline(171);
            _v1512 = px_int(0LL);
            px_srcline(172);
            while (px_is_truthy(px_lt(_v1512, px_call(px_get_global("len"), (LXValue[]){_v1511}, 1)))) {
                px_srcline(173);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(_v1511, _v1512), px_int(1LL))}, 1));
                px_srcline(174);
                 _v1512 = px_add(_v1512, px_int(1LL));
            }
            px_srcline(175);
            if (px_is_truthy(px_ne(px_index(_v1504, px_int(2LL)), px_null()))) {
                px_srcline(176);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1504, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1505, px_str("While")))) {
            px_srcline(178);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1504, px_int(2LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1505, px_str("For")))) {
            px_srcline(180);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1504, px_int(3LL))}, 1));
        }
        px_srcline(181);
         _v1503 = px_add(_v1503, px_int(1LL));
    }
px_err_1513:
    if (px_err_1513_proped) return px_err_1513_val;
    return px_null();
}

static LXValue fn_bc_collect_impl_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_impl_list");
    LXValue _v1514 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1515 = px_uninit();
    LXValue _v1516 = px_uninit();
    LXValue _v1517 = px_uninit();
    LXValue _v1518 = px_uninit();
    LXValue _v1519 = px_uninit();
    LXValue _v1520 = px_uninit();
    LXValue _v1521 = px_uninit();
    LXValue _v1522 = px_uninit();
    LXValue _v1523 = px_uninit();
    LXValue _v1524 = px_uninit();
    LXValue px_err_1525_val = px_null();
    int px_err_1525_proped = 0;
    px_srcline(185);
    _v1515 = px_list_n((LXValue[]){}, 0);
    px_srcline(186);
    _v1516 = px_int(0LL);
    px_srcline(187);
    while (px_is_truthy(px_lt(_v1516, px_call(px_get_global("len"), (LXValue[]){_v1514}, 1)))) {
        px_srcline(188);
        _v1517 = px_index(_v1514, _v1516);
        px_srcline(189);
        if (px_is_truthy(px_eq(px_index(_v1517, px_int(0LL)), px_str("ImplDef")))) {
            px_srcline(190);
            _v1518 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1517, px_int(1LL))}, 1);
            px_srcline(191);
            _v1519 = px_int(0LL);
            px_srcline(192);
            while (px_is_truthy(px_lt(_v1519, px_call(px_get_global("len"), (LXValue[]){px_index(_v1517, px_int(3LL))}, 1)))) {
                px_srcline(193);
                (void)(px_method(_v1515, "append", (LXValue[]){px_list_n((LXValue[]){_v1518, px_index(px_index(_v1517, px_int(3LL)), _v1519)}, 2)}, 1));
                px_srcline(194);
                 _v1519 = px_add(_v1519, px_int(1LL));
            }
        }
        px_srcline(195);
         _v1516 = px_add(_v1516, px_int(1LL));
    }
    px_srcline(197);
    _v1520 = px_int(1LL);
    px_srcline(198);
    while (px_is_truthy(px_lt(_v1520, px_call(px_get_global("len"), (LXValue[]){_v1515}, 1)))) {
        px_srcline(199);
        _v1521 = _v1520;
        px_srcline(200);
        while (px_is_truthy(px_gt(_v1521, px_int(0LL)))) {
            px_srcline(201);
            _v1522 = px_add(px_add(px_index(px_index(_v1515, px_sub(_v1521, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1515, px_sub(_v1521, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(202);
            _v1523 = px_add(px_add(px_index(px_index(_v1515, _v1521), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1515, _v1521), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(203);
            if (px_is_truthy(px_lt(_v1523, _v1522))) {
                px_srcline(204);
                _v1524 = px_index(_v1515, _v1521);
                px_srcline(205);
                px_index_set(_v1515, _v1521, px_index(_v1515, px_sub(_v1521, px_int(1LL))));
                px_srcline(206);
                px_index_set(_v1515, px_sub(_v1521, px_int(1LL)), _v1524);
            }
            px_srcline(207);
             _v1521 = px_sub(_v1521, px_int(1LL));
        }
        px_srcline(208);
         _v1520 = px_add(_v1520, px_int(1LL));
    }
    px_srcline(209);
    return _v1515;
px_err_1525:
    if (px_err_1525_proped) return px_err_1525_val;
    return px_null();
}

static LXValue fn_bc_new_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_func");
    LXValue _v1526 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1527 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1528 = px_uninit();
    LXValue px_err_1529_val = px_null();
    int px_err_1529_proped = 0;
    px_srcline(214);
    _v1528 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(215);
    px_index_set(_v1528, px_str("name"), _v1526);
    px_srcline(216);
    px_index_set(_v1528, px_str("arity"), _v1527);
    px_srcline(217);
    px_index_set(_v1528, px_str("ndefault"), px_int(0LL));
    px_srcline(218);
    px_index_set(_v1528, px_str("nslots"), _v1527);
    px_srcline(219);
    px_index_set(_v1528, px_str("bc"), px_list_n((LXValue[]){}, 0));
    px_srcline(220);
    px_index_set(_v1528, px_str("smap"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(221);
    px_index_set(_v1528, px_str("next_slot"), _v1527);
    px_srcline(222);
    px_index_set(_v1528, px_str("is_top"), px_bool(false));
    px_srcline(223);
    px_index_set(_v1528, px_str("loops"), px_list_n((LXValue[]){}, 0));
    px_srcline(229);
    px_index_set(_v1528, px_str("upnames"), px_list_n((LXValue[]){}, 0));
    px_srcline(230);
    px_index_set(_v1528, px_str("cell"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(231);
    px_index_set(_v1528, px_str("boxed"), px_list_n((LXValue[]){}, 0));
    px_srcline(236);
    px_index_set(_v1528, px_str("inited"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(238);
    px_index_set(_v1528, px_str("hoist_slots"), px_list_n((LXValue[]){}, 0));
    px_srcline(239);
    return _v1528;
px_err_1529:
    if (px_err_1529_proped) return px_err_1529_val;
    return px_null();
}

static LXValue fn_bc_inited_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_copy");
    LXValue _v1530 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1531_val = px_null();
    int px_err_1531_proped = 0;
    px_srcline(244);
    return px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_index(_v1530, px_str("inited"))}, 1);
px_err_1531:
    if (px_err_1531_proped) return px_err_1531_val;
    return px_null();
}

static LXValue fn_bc_inited_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_restore");
    LXValue _v1532 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1533 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1534_val = px_null();
    int px_err_1534_proped = 0;
    px_srcline(246);
    px_index_set(_v1532, px_str("inited"), px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1533}, 1));
px_err_1534:
    if (px_err_1534_proped) return px_err_1534_val;
    return px_null();
}

static LXValue fn_bc_inited_intersect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_intersect");
    LXValue _v1535 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1536 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1537 = px_uninit();
    LXValue _v1538 = px_uninit();
    LXValue _v1539 = px_uninit();
    LXValue px_err_1540_val = px_null();
    int px_err_1540_proped = 0;
    px_srcline(248);
    _v1537 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(249);
    _v1538 = px_method(_v1535, "keys", (LXValue[]){}, 0);
    px_srcline(250);
    _v1539 = px_int(0LL);
    px_srcline(251);
    while (px_is_truthy(px_lt(_v1539, px_call(px_get_global("len"), (LXValue[]){_v1538}, 1)))) {
        px_srcline(252);
        if (px_is_truthy(px_method(_v1536, "has", (LXValue[]){px_index(_v1538, _v1539)}, 1))) {
            px_srcline(253);
            px_index_set(_v1537, px_index(_v1538, _v1539), px_int(1LL));
        }
        px_srcline(254);
         _v1539 = px_add(_v1539, px_int(1LL));
    }
    px_srcline(255);
    return _v1537;
px_err_1540:
    if (px_err_1540_proped) return px_err_1540_val;
    return px_null();
}

static LXValue fn_bc_inited_mark(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_mark");
    LXValue _v1541 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1542 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1543_val = px_null();
    int px_err_1543_proped = 0;
    px_srcline(257);
    px_index_set(px_index(_v1541, px_str("inited")), _v1542, px_int(1LL));
px_err_1543:
    if (px_err_1543_proped) return px_err_1543_val;
    return px_null();
}

static LXValue fn_bc_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_slot");
    LXValue _v1544 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1545 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1546 = px_uninit();
    LXValue px_err_1547_val = px_null();
    int px_err_1547_proped = 0;
    px_srcline(260);
    if (px_is_truthy(px_method(px_index(_v1544, px_str("smap")), "has", (LXValue[]){_v1545}, 1))) {
        px_srcline(261);
        return px_index(px_index(_v1544, px_str("smap")), _v1545);
    }
    px_srcline(262);
    _v1546 = px_index(_v1544, px_str("next_slot"));
    px_srcline(263);
    px_index_set(_v1544, px_str("next_slot"), px_add(_v1546, px_int(1LL)));
    px_srcline(264);
    px_index_set(px_index(_v1544, px_str("smap")), _v1545, _v1546);
    px_srcline(265);
    return _v1546;
px_err_1547:
    if (px_err_1547_proped) return px_err_1547_val;
    return px_null();
}

static LXValue fn_bc_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_tmp");
    LXValue _v1548 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1549 = px_uninit();
    LXValue px_err_1550_val = px_null();
    int px_err_1550_proped = 0;
    px_srcline(268);
    _v1549 = px_index(_v1548, px_str("next_slot"));
    px_srcline(269);
    px_index_set(_v1548, px_str("next_slot"), px_add(_v1549, px_int(1LL)));
    px_srcline(270);
    return _v1549;
px_err_1550:
    if (px_err_1550_proped) return px_err_1550_val;
    return px_null();
}

static LXValue fn_bc_cell_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_has");
    LXValue _v1551 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1552 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1553_val = px_null();
    int px_err_1553_proped = 0;
    px_srcline(275);
    return px_method(px_index(_v1551, px_str("cell")), "has", (LXValue[]){_v1552}, 1);
px_err_1553:
    if (px_err_1553_proped) return px_err_1553_val;
    return px_null();
}

static LXValue fn_bc_cell_mark(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_mark");
    LXValue _v1554 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1555 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1556_val = px_null();
    int px_err_1556_proped = 0;
    px_srcline(277);
    px_index_set(px_index(_v1554, px_str("cell")), _v1555, px_int(1LL));
px_err_1556:
    if (px_err_1556_proped) return px_err_1556_val;
    return px_null();
}

static LXValue fn_bc_load_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_load_var");
    LXValue _v1557 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1558 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1559 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1560 = px_uninit();
    LXValue _v1561 = px_uninit();
    LXValue px_err_1562_val = px_null();
    int px_err_1562_proped = 0;
    px_srcline(280);
    _v1560 = px_index(px_index(_v1557, px_str("smap")), _v1558);
    px_srcline(281);
    if (px_is_truthy(px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1557, _v1558}, 2)))) {
        px_srcline(282);
        if (px_is_truthy(({ LXValue _t1563 = px_ge(_v1559, px_int(0LL)); px_is_truthy(_t1563) ? px_ne(_v1559, _v1560) : _t1563; }))) {
            px_srcline(283);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1557, px_str("MOV"), _v1559, _v1560, px_int(0LL)}, 5));
        }
        px_srcline(284);
        return px_null();
    }
    px_srcline(285);
    if (px_is_truthy(px_lt(_v1559, px_int(0LL)))) {
        px_srcline(286);
        return px_null();
    }
    px_srcline(287);
    if (px_is_truthy(px_eq(_v1559, _v1560))) {
        px_srcline(290);
        _v1561 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1557}, 1);
        px_srcline(291);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1557, px_str("CELLGET"), _v1561, _v1560, px_int(0LL)}, 5));
        px_srcline(292);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1557, px_str("MOV"), _v1560, _v1561, px_int(0LL)}, 5));
        px_srcline(293);
        return px_null();
    }
    px_srcline(294);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1557, px_str("CELLGET"), _v1559, _v1560, px_int(0LL)}, 5));
px_err_1562:
    if (px_err_1562_proped) return px_err_1562_val;
    return px_null();
}

static LXValue fn_bc_load_ck(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_load_ck");
    LXValue _v1564 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1565 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1566 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1567 = px_uninit();
    LXValue _v1568 = px_uninit();
    LXValue px_err_1569_val = px_null();
    int px_err_1569_proped = 0;
    px_srcline(299);
    _v1567 = _v1566;
    px_srcline(300);
    if (px_is_truthy(px_lt(_v1567, px_int(0LL)))) {
        px_srcline(302);
         _v1567 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1564}, 1);
    }
    px_srcline(303);
    (void)(px_call(px_get_global("bc_load_var"), (LXValue[]){_v1564, _v1565, _v1567}, 3));
    px_srcline(304);
    if (px_is_truthy(px_not(px_method(px_index(_v1564, px_str("inited")), "has", (LXValue[]){_v1565}, 1)))) {
        px_srcline(305);
        _v1568 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1565}, 2);
        px_srcline(306);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1564, px_str("CHKINIT"), _v1567, px_int(0LL), _v1568}, 5));
    }
    px_srcline(307);
    return _v1567;
px_err_1569:
    if (px_err_1569_proped) return px_err_1569_val;
    return px_null();
}

static LXValue fn_bc_store_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_store_var");
    LXValue _v1570 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1571 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1572 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1573 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_1574_val = px_null();
    int px_err_1574_proped = 0;
    px_srcline(310);
    if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1570, _v1571}, 2))) {
        px_srcline(311);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1570, px_str("CELLSET"), _v1573, _v1572, px_int(0LL)}, 5));
    }
    else {
        px_srcline(313);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1570, px_str("MOV"), _v1572, _v1573, px_int(0LL)}, 5));
    }
px_err_1574:
    if (px_err_1574_proped) return px_err_1574_val;
    return px_null();
}

static LXValue fn_bc_closure_free(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_closure_free");
    LXValue _v1575 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1576 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1577 = px_uninit();
    LXValue px_err_1578_val = px_null();
    int px_err_1578_proped = 0;
    px_srcline(317);
    _v1577 = px_list_n((LXValue[]){}, 0);
    px_srcline(318);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v1575, px_null(), _v1576, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v1577}, 2));
    px_srcline(319);
    return _v1577;
px_err_1578:
    if (px_err_1578_proped) return px_err_1578_val;
    return px_null();
}

static LXValue fn_bc_lambda_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_hoist");
    LXValue _v1579 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1580 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1581_val = px_null();
    int px_err_1581_proped = 0;
    px_srcline(322);
    if (px_is_truthy(({ LXValue _t1583 = ({ LXValue _t1582 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1579}, 1), px_str("list")); px_is_truthy(_t1582) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1579}, 1), px_int(0LL)) : _t1582; }); px_is_truthy(_t1583) ? px_eq(px_index(_v1579, px_int(0LL)), px_str("Block")) : _t1583; }))) {
        px_srcline(323);
        (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1579, px_int(1LL)), _v1580}, 2));
    }
px_err_1581:
    if (px_err_1581_proped) return px_err_1581_val;
    return px_null();
}

static LXValue fn_bc_lambda_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_decl");
    LXValue _v1584 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1585 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1586_val = px_null();
    int px_err_1586_proped = 0;
    px_srcline(326);
    if (px_is_truthy(({ LXValue _t1588 = ({ LXValue _t1587 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1584}, 1), px_str("list")); px_is_truthy(_t1587) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1584}, 1), px_int(0LL)) : _t1587; }); px_is_truthy(_t1588) ? px_eq(px_index(_v1584, px_int(0LL)), px_str("Block")) : _t1588; }))) {
        px_srcline(327);
        (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1584, px_int(1LL)), _v1585}, 2));
    }
px_err_1586:
    if (px_err_1586_proped) return px_err_1586_val;
    return px_null();
}

static LXValue fn_bc_box_frame(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_box_frame");
    LXValue _v1589 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1590 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1591 = px_uninit();
    LXValue _v1592 = px_uninit();
    LXValue _v1593 = px_uninit();
    LXValue px_err_1594_val = px_null();
    int px_err_1594_proped = 0;
    px_srcline(333);
    _v1591 = px_list_n((LXValue[]){}, 0);
    px_srcline(334);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1590, _v1591}, 2));
    px_srcline(335);
    _v1592 = px_int(0LL);
    px_srcline(336);
    while (px_is_truthy(px_lt(_v1592, px_call(px_get_global("len"), (LXValue[]){_v1591}, 1)))) {
        px_srcline(337);
        _v1593 = px_index(_v1591, _v1592);
        px_srcline(338);
        if (px_is_truthy(({ LXValue _t1595 = px_method(px_index(_v1589, px_str("smap")), "has", (LXValue[]){_v1593}, 1); px_is_truthy(_t1595) ? px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1589, _v1593}, 2)) : _t1595; }))) {
            px_srcline(339);
            (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1589, _v1593}, 2));
            px_srcline(340);
            (void)(px_method(px_index(_v1589, px_str("boxed")), "push", (LXValue[]){_v1593}, 1));
            px_srcline(341);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1589, px_str("CELLNEW"), px_index(px_index(_v1589, px_str("smap")), _v1593), px_index(px_index(_v1589, px_str("smap")), _v1593), px_int(0LL)}, 5));
        }
        px_srcline(342);
         _v1592 = px_add(_v1592, px_int(1LL));
    }
px_err_1594:
    if (px_err_1594_proped) return px_err_1594_val;
    return px_null();
}

static LXValue fn_bc_emit_inst(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_inst");
    LXValue _v1596 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1597 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1598 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1599 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1600 = (nargs > 4) ? args[4] : px_null();
    LXValue px_err_1601_val = px_null();
    int px_err_1601_proped = 0;
    px_srcline(348);
    (void)(px_method(px_index(_v1596, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){_v1597, px_int(0LL), _v1598, _v1599, _v1600}, 5)}, 1));
px_err_1601:
    if (px_err_1601_proped) return px_err_1601_val;
    return px_null();
}

static LXValue fn_bc_patch_off(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_patch_off");
    LXValue _v1602 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1603 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1604 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1605_val = px_null();
    int px_err_1605_proped = 0;
    px_srcline(352);
    px_index_set(px_index(px_index(_v1602, px_str("bc")), _v1603), px_int(3LL), px_sub(_v1604, px_add(_v1603, px_int(1LL))));
px_err_1605:
    if (px_err_1605_proped) return px_err_1605_val;
    return px_null();
}

static LXValue fn_bc_collect_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_hoist");
    LXValue _v1606 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1607 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1608 = px_uninit();
    LXValue _v1609 = px_uninit();
    LXValue _v1610 = px_uninit();
    LXValue _v1611 = px_uninit();
    LXValue _v1612 = px_uninit();
    LXValue _v1613 = px_uninit();
    LXValue _v1614 = px_uninit();
    LXValue _v1615 = px_uninit();
    LXValue _v1616 = px_uninit();
    LXValue _v1617 = px_uninit();
    LXValue px_err_1618_val = px_null();
    int px_err_1618_proped = 0;
    px_srcline(355);
    _v1608 = px_int(0LL);
    px_srcline(356);
    while (px_is_truthy(px_lt(_v1608, px_call(px_get_global("len"), (LXValue[]){_v1606}, 1)))) {
        px_srcline(357);
        _v1609 = px_index(_v1606, _v1608);
        px_srcline(358);
        _v1610 = px_index(_v1609, px_int(0LL));
        px_srcline(359);
        if (px_is_truthy(px_eq(_v1610, px_str("Assign")))) {
            px_srcline(360);
            _v1611 = px_index(_v1609, px_int(1LL));
            px_srcline(361);
            if (px_is_truthy(px_eq(px_index(_v1611, px_int(0LL)), px_str("Var")))) {
                px_srcline(362);
                _v1612 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1611, px_int(1LL))}, 1);
                px_srcline(363);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1607, _v1612}, 2)))) {
                    px_srcline(364);
                    (void)(px_method(_v1607, "append", (LXValue[]){_v1612}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1610, px_str("VarDecl")))) {
            px_srcline(366);
            _v1612 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1609, px_int(2LL))}, 1);
            px_srcline(367);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1607, _v1612}, 2)))) {
                px_srcline(368);
                (void)(px_method(_v1607, "append", (LXValue[]){_v1612}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1610, px_str("FuncDef")))) {
            px_srcline(372);
            _v1612 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1609, px_int(1LL))}, 1);
            px_srcline(373);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1607, _v1612}, 2)))) {
                px_srcline(374);
                (void)(px_method(_v1607, "append", (LXValue[]){_v1612}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1610, px_str("If")))) {
            px_srcline(376);
            _v1613 = px_index(_v1609, px_int(1LL));
            px_srcline(377);
            _v1614 = px_int(0LL);
            px_srcline(378);
            while (px_is_truthy(px_lt(_v1614, px_call(px_get_global("len"), (LXValue[]){_v1613}, 1)))) {
                px_srcline(379);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(px_index(_v1613, _v1614), px_int(1LL)), _v1607}, 2));
                px_srcline(380);
                 _v1614 = px_add(_v1614, px_int(1LL));
            }
            px_srcline(381);
            if (px_is_truthy(px_ne(px_index(_v1609, px_int(2LL)), px_null()))) {
                px_srcline(382);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1609, px_int(2LL)), _v1607}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1610, px_str("While")))) {
            px_srcline(384);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1609, px_int(2LL)), _v1607}, 2));
        }
        else if (px_is_truthy(px_eq(_v1610, px_str("For")))) {
            px_srcline(388);
            _v1615 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1609, px_int(1LL))}, 1);
            px_srcline(389);
            _v1616 = px_int(0LL);
            px_srcline(390);
            while (px_is_truthy(px_lt(_v1616, px_call(px_get_global("len"), (LXValue[]){_v1615}, 1)))) {
                px_srcline(391);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1607, px_index(_v1615, _v1616)}, 2)))) {
                    px_srcline(392);
                    (void)(px_method(_v1607, "append", (LXValue[]){px_index(_v1615, _v1616)}, 1));
                }
                px_srcline(393);
                 _v1616 = px_add(_v1616, px_int(1LL));
            }
            px_srcline(394);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1609, px_int(3LL)), _v1607}, 2));
        }
        else if (px_is_truthy(px_eq(_v1610, px_str("ChanDecl")))) {
            px_srcline(397);
            _v1617 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1609, px_int(1LL))}, 1);
            px_srcline(398);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1607, _v1617}, 2)))) {
                px_srcline(399);
                (void)(px_method(_v1607, "append", (LXValue[]){_v1617}, 1));
            }
        }
        px_srcline(400);
         _v1608 = px_add(_v1608, px_int(1LL));
    }
px_err_1618:
    if (px_err_1618_proped) return px_err_1618_val;
    return px_null();
}

static LXValue fn_bc_collect_decl_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_decl_vars");
    LXValue _v1619 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1620 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1621 = px_uninit();
    LXValue _v1622 = px_uninit();
    LXValue _v1623 = px_uninit();
    LXValue _v1624 = px_uninit();
    LXValue _v1625 = px_uninit();
    LXValue _v1626 = px_uninit();
    LXValue _v1627 = px_uninit();
    LXValue _v1628 = px_uninit();
    LXValue _v1629 = px_uninit();
    LXValue _v1630 = px_uninit();
    LXValue px_err_1631_val = px_null();
    int px_err_1631_proped = 0;
    px_srcline(409);
    _v1621 = px_int(0LL);
    px_srcline(410);
    while (px_is_truthy(px_lt(_v1621, px_call(px_get_global("len"), (LXValue[]){_v1619}, 1)))) {
        px_srcline(411);
        _v1622 = px_index(_v1619, _v1621);
        px_srcline(412);
        _v1623 = px_index(_v1622, px_int(0LL));
        px_srcline(413);
        if (px_is_truthy(px_eq(_v1623, px_str("VarDecl")))) {
            px_srcline(414);
            _v1624 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1622, px_int(2LL))}, 1);
            px_srcline(415);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1620, _v1624}, 2)))) {
                px_srcline(416);
                (void)(px_method(_v1620, "append", (LXValue[]){_v1624}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1623, px_str("FuncDef")))) {
            px_srcline(418);
            _v1625 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1622, px_int(1LL))}, 1);
            px_srcline(419);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1620, _v1625}, 2)))) {
                px_srcline(420);
                (void)(px_method(_v1620, "append", (LXValue[]){_v1625}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1623, px_str("ChanDecl")))) {
            px_srcline(422);
            _v1626 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1622, px_int(1LL))}, 1);
            px_srcline(423);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1620, _v1626}, 2)))) {
                px_srcline(424);
                (void)(px_method(_v1620, "append", (LXValue[]){_v1626}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1623, px_str("If")))) {
            px_srcline(426);
            _v1627 = px_index(_v1622, px_int(1LL));
            px_srcline(427);
            _v1628 = px_int(0LL);
            px_srcline(428);
            while (px_is_truthy(px_lt(_v1628, px_call(px_get_global("len"), (LXValue[]){_v1627}, 1)))) {
                px_srcline(429);
                (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(px_index(_v1627, _v1628), px_int(1LL)), _v1620}, 2));
                px_srcline(430);
                 _v1628 = px_add(_v1628, px_int(1LL));
            }
            px_srcline(431);
            if (px_is_truthy(px_ne(px_index(_v1622, px_int(2LL)), px_null()))) {
                px_srcline(432);
                (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1622, px_int(2LL)), _v1620}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1623, px_str("For")))) {
            px_srcline(434);
            _v1629 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1622, px_int(1LL))}, 1);
            px_srcline(435);
            _v1630 = px_int(0LL);
            px_srcline(436);
            while (px_is_truthy(px_lt(_v1630, px_call(px_get_global("len"), (LXValue[]){_v1629}, 1)))) {
                px_srcline(437);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1620, px_index(_v1629, _v1630)}, 2)))) {
                    px_srcline(438);
                    (void)(px_method(_v1620, "append", (LXValue[]){px_index(_v1629, _v1630)}, 1));
                }
                px_srcline(439);
                 _v1630 = px_add(_v1630, px_int(1LL));
            }
            px_srcline(440);
            (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1622, px_int(3LL)), _v1620}, 2));
        }
        else if (px_is_truthy(px_eq(_v1623, px_str("While")))) {
            px_srcline(442);
            (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1622, px_int(2LL)), _v1620}, 2));
        }
        px_srcline(443);
         _v1621 = px_add(_v1621, px_int(1LL));
    }
px_err_1631:
    if (px_err_1631_proped) return px_err_1631_val;
    return px_null();
}

static LXValue fn_bc_emit_null(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_null");
    LXValue _v1632 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1633 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1634 = px_uninit();
    LXValue px_err_1635_val = px_null();
    int px_err_1635_proped = 0;
    px_srcline(446);
    _v1634 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(447);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1632, px_str("LOADK"), _v1633, _v1634, px_int(0LL)}, 5));
px_err_1635:
    if (px_err_1635_proped) return px_err_1635_val;
    return px_null();
}

static LXValue fn_bc_emit_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_int");
    LXValue _v1636 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1637 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1638 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1639 = px_uninit();
    LXValue px_err_1640_val = px_null();
    int px_err_1640_proped = 0;
    px_srcline(450);
    if (px_is_truthy(({ LXValue _t1641 = px_ge(_v1638, px_neg(px_int(32768LL))); px_is_truthy(_t1641) ? px_le(_v1638, px_int(32767LL)) : _t1641; }))) {
        px_srcline(451);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1636, px_str("IMM"), _v1637, _v1638, px_int(0LL)}, 5));
    }
    else {
        px_srcline(453);
        _v1639 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("int"), _v1638, px_float(0), px_str("")}, 5);
        px_srcline(454);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1636, px_str("LOADK"), _v1637, _v1639, px_int(0LL)}, 5));
    }
px_err_1640:
    if (px_err_1640_proped) return px_err_1640_val;
    return px_null();
}

static LXValue fn_bc_neg_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_neg_float");
    LXValue _v1642 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1643_val = px_null();
    int px_err_1643_proped = 0;
    px_srcline(461);
    return px_call(px_get_global("bits_to_float64"), (LXValue[]){px_bitxor(px_call(px_get_global("float64_bits"), (LXValue[]){_v1642}, 1), px_sub(px_sub(px_int(0LL), px_int(9223372036854775807LL)), px_int(1LL)))}, 1);
px_err_1643:
    if (px_err_1643_proped) return px_err_1643_val;
    return px_null();
}

static LXValue fn_bc_emit_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_float");
    LXValue _v1644 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1645 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1646 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1647 = px_uninit();
    LXValue px_err_1648_val = px_null();
    int px_err_1648_proped = 0;
    px_srcline(464);
    _v1647 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("float"), px_int(0LL), _v1646, px_str("")}, 5);
    px_srcline(465);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1644, px_str("LOADK"), _v1645, _v1647, px_int(0LL)}, 5));
px_err_1648:
    if (px_err_1648_proped) return px_err_1648_val;
    return px_null();
}

static LXValue fn_bc_unop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_unop_op");
    LXValue _v1649 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1650_val = px_null();
    int px_err_1650_proped = 0;
    px_srcline(468);
    if (px_is_truthy(px_eq(_v1649, px_str("Neg")))) {
        px_srcline(469);
        return px_str("NEG");
    }
    px_srcline(470);
    if (px_is_truthy(px_eq(_v1649, px_str("Not")))) {
        px_srcline(471);
        return px_str("NOT");
    }
    px_srcline(472);
    if (px_is_truthy(px_eq(_v1649, px_str("BitNot")))) {
        px_srcline(473);
        return px_str("BITNOT");
    }
    px_srcline(474);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_unop_op 未知一元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1649}, 1))}, 1));
px_err_1650:
    if (px_err_1650_proped) return px_err_1650_val;
    return px_null();
}

static LXValue fn_bc_binop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_binop_op");
    LXValue _v1651 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1652_val = px_null();
    int px_err_1652_proped = 0;
    px_srcline(476);
    if (px_is_truthy(px_eq(_v1651, px_str("Add")))) {
        px_srcline(477);
        return px_str("ADD");
    }
    px_srcline(478);
    if (px_is_truthy(px_eq(_v1651, px_str("Sub")))) {
        px_srcline(479);
        return px_str("SUB");
    }
    px_srcline(480);
    if (px_is_truthy(px_eq(_v1651, px_str("Mul")))) {
        px_srcline(481);
        return px_str("MUL");
    }
    px_srcline(482);
    if (px_is_truthy(px_eq(_v1651, px_str("Div")))) {
        px_srcline(483);
        return px_str("DIV");
    }
    px_srcline(484);
    if (px_is_truthy(px_eq(_v1651, px_str("IntDiv")))) {
        px_srcline(485);
        return px_str("IDIV");
    }
    px_srcline(486);
    if (px_is_truthy(px_eq(_v1651, px_str("Mod")))) {
        px_srcline(487);
        return px_str("MOD");
    }
    px_srcline(488);
    if (px_is_truthy(px_eq(_v1651, px_str("Pow")))) {
        px_srcline(489);
        return px_str("POW");
    }
    px_srcline(490);
    if (px_is_truthy(px_eq(_v1651, px_str("Eq")))) {
        px_srcline(491);
        return px_str("EQ");
    }
    px_srcline(492);
    if (px_is_truthy(px_eq(_v1651, px_str("Ne")))) {
        px_srcline(493);
        return px_str("NE");
    }
    px_srcline(494);
    if (px_is_truthy(px_eq(_v1651, px_str("Lt")))) {
        px_srcline(495);
        return px_str("LT");
    }
    px_srcline(496);
    if (px_is_truthy(px_eq(_v1651, px_str("Le")))) {
        px_srcline(497);
        return px_str("LE");
    }
    px_srcline(498);
    if (px_is_truthy(px_eq(_v1651, px_str("Gt")))) {
        px_srcline(499);
        return px_str("GT");
    }
    px_srcline(500);
    if (px_is_truthy(px_eq(_v1651, px_str("Ge")))) {
        px_srcline(501);
        return px_str("GE");
    }
    px_srcline(502);
    if (px_is_truthy(px_eq(_v1651, px_str("BitAnd")))) {
        px_srcline(503);
        return px_str("BITAND");
    }
    px_srcline(504);
    if (px_is_truthy(px_eq(_v1651, px_str("BitOr")))) {
        px_srcline(505);
        return px_str("BITOR");
    }
    px_srcline(506);
    if (px_is_truthy(px_eq(_v1651, px_str("BitXor")))) {
        px_srcline(507);
        return px_str("BITXOR");
    }
    px_srcline(508);
    if (px_is_truthy(px_eq(_v1651, px_str("Shl")))) {
        px_srcline(509);
        return px_str("SHL");
    }
    px_srcline(510);
    if (px_is_truthy(px_eq(_v1651, px_str("Shr")))) {
        px_srcline(511);
        return px_str("SHR");
    }
    px_srcline(512);
    if (px_is_truthy(px_eq(_v1651, px_str("ShrU")))) {
        px_srcline(513);
        return px_str("SHRU");
    }
    px_srcline(514);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_binop_op 未知二元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1651}, 1))}, 1));
px_err_1652:
    if (px_err_1652_proped) return px_err_1652_val;
    return px_null();
}

static LXValue fn_bc_emit_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_expr");
    LXValue _v1653 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1654 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1655 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1656 = px_uninit();
    LXValue _v1657 = px_uninit();
    LXValue _v1658 = px_uninit();
    LXValue _v1659 = px_uninit();
    LXValue _v1660 = px_uninit();
    LXValue _v1661 = px_uninit();
    LXValue _v1662 = px_uninit();
    LXValue _v1663 = px_uninit();
    LXValue _v1664 = px_uninit();
    LXValue _v1665 = px_uninit();
    LXValue _v1666 = px_uninit();
    LXValue _v1667 = px_uninit();
    LXValue _v1668 = px_uninit();
    LXValue _v1669 = px_uninit();
    LXValue _v1670 = px_uninit();
    LXValue _v1671 = px_uninit();
    LXValue _v1672 = px_uninit();
    LXValue _v1673 = px_uninit();
    LXValue _v1674 = px_uninit();
    LXValue _v1675 = px_uninit();
    LXValue _v1676 = px_uninit();
    LXValue _v1677 = px_uninit();
    LXValue _v1678 = px_uninit();
    LXValue _v1679 = px_uninit();
    LXValue _v1680 = px_uninit();
    LXValue _v1681 = px_uninit();
    LXValue _v1682 = px_uninit();
    LXValue _v1683 = px_uninit();
    LXValue _v1684 = px_uninit();
    LXValue _v1685 = px_uninit();
    LXValue _v1686 = px_uninit();
    LXValue _v1687 = px_uninit();
    LXValue _v1688 = px_uninit();
    LXValue _v1689 = px_uninit();
    LXValue _v1690 = px_uninit();
    LXValue _v1691 = px_uninit();
    LXValue _v1692 = px_uninit();
    LXValue _v1693 = px_uninit();
    LXValue _v1694 = px_uninit();
    LXValue _v1695 = px_uninit();
    LXValue _v1696 = px_uninit();
    LXValue _v1697 = px_uninit();
    LXValue _v1698 = px_uninit();
    LXValue _v1699 = px_uninit();
    LXValue _v1700 = px_uninit();
    LXValue _v1701 = px_uninit();
    LXValue _v1702 = px_uninit();
    LXValue _v1703 = px_uninit();
    LXValue _v1704 = px_uninit();
    LXValue _v1705 = px_uninit();
    LXValue _v1706 = px_uninit();
    LXValue _v1707 = px_uninit();
    LXValue _v1708 = px_uninit();
    LXValue _v1709 = px_uninit();
    LXValue px_err_1710_val = px_null();
    int px_err_1710_proped = 0;
    px_srcline(519);
    _v1656 = px_index(_v1653, px_int(0LL));
    px_srcline(520);
    if (px_is_truthy(px_eq(_v1656, px_str("Int")))) {
        px_srcline(521);
        (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1655, _v1654, px_index(_v1653, px_int(1LL))}, 3));
        px_srcline(522);
        return _v1654;
    }
    px_srcline(523);
    if (px_is_truthy(px_eq(_v1656, px_str("Float")))) {
        px_srcline(524);
        (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1655, _v1654, px_index(_v1653, px_int(1LL))}, 3));
        px_srcline(525);
        return _v1654;
    }
    px_srcline(526);
    if (px_is_truthy(px_eq(_v1656, px_str("Unary")))) {
        px_srcline(529);
        _v1657 = px_index(_v1653, px_int(2LL));
        px_srcline(530);
        _v1658 = px_index(_v1653, px_int(1LL));
        px_srcline(531);
        if (px_is_truthy(({ LXValue _t1711 = px_eq(px_index(_v1657, px_int(0LL)), px_str("Int")); px_is_truthy(_t1711) ? px_eq(_v1658, px_str("Neg")) : _t1711; }))) {
            px_srcline(532);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1655, _v1654, px_sub(px_int(0LL), px_index(_v1657, px_int(1LL)))}, 3));
            px_srcline(533);
            return _v1654;
        }
        px_srcline(534);
        if (px_is_truthy(({ LXValue _t1712 = px_eq(px_index(_v1657, px_int(0LL)), px_str("Float")); px_is_truthy(_t1712) ? px_eq(_v1658, px_str("Neg")) : _t1712; }))) {
            px_srcline(541);
            (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1655, _v1654, px_call(px_get_global("bc_neg_float"), (LXValue[]){px_index(_v1657, px_int(1LL))}, 1)}, 3));
            px_srcline(542);
            return _v1654;
        }
        px_srcline(543);
        _v1659 = _v1654;
        px_srcline(544);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(545);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(546);
        _v1660 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(547);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1657, _v1660, _v1655}, 3));
        px_srcline(548);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_call(px_get_global("bc_unop_op"), (LXValue[]){_v1658}, 1), _v1659, _v1660, px_int(0LL)}, 5));
        px_srcline(549);
        return _v1659;
    }
    px_srcline(550);
    if (px_is_truthy(px_eq(_v1656, px_str("Str")))) {
        px_srcline(551);
        _v1661 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1653, px_int(1LL))}, 1)}, 5);
        px_srcline(552);
        if (px_is_truthy(px_ge(_v1654, px_int(0LL)))) {
            px_srcline(553);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("LOADK"), _v1654, _v1661, px_int(0LL)}, 5));
        }
        px_srcline(554);
        return _v1654;
    }
    px_srcline(555);
    if (px_is_truthy(px_eq(_v1656, px_str("Bool")))) {
        px_srcline(556);
        _v1662 = px_int(0LL);
        px_srcline(557);
        if (px_is_truthy(px_index(_v1653, px_int(1LL)))) {
            px_srcline(558);
             _v1662 = px_int(1LL);
        }
        px_srcline(559);
        _v1661 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("bool"), _v1662, px_float(0), px_str("")}, 5);
        px_srcline(560);
        if (px_is_truthy(px_ge(_v1654, px_int(0LL)))) {
            px_srcline(561);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("LOADK"), _v1654, _v1661, px_int(0LL)}, 5));
        }
        px_srcline(562);
        return _v1654;
    }
    px_srcline(563);
    if (px_is_truthy(px_eq(_v1656, px_str("Null")))) {
        px_srcline(564);
        if (px_is_truthy(px_ge(_v1654, px_int(0LL)))) {
            px_srcline(565);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1655, _v1654}, 2));
        }
        px_srcline(566);
        return _v1654;
    }
    px_srcline(567);
    if (px_is_truthy(px_eq(_v1656, px_str("Var")))) {
        px_srcline(568);
        _v1663 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1653, px_int(1LL))}, 1);
        px_srcline(569);
        if (px_is_truthy(px_method(px_index(_v1655, px_str("smap")), "has", (LXValue[]){_v1663}, 1))) {
            px_srcline(572);
            (void)(px_call(px_get_global("bc_load_ck"), (LXValue[]){_v1655, _v1663, _v1654}, 3));
            px_srcline(573);
            return _v1654;
        }
        px_srcline(575);
        _v1664 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1663}, 2);
        px_srcline(576);
        if (px_is_truthy(px_ge(_v1654, px_int(0LL)))) {
            px_srcline(577);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("GETG"), _v1654, _v1664, px_int(0LL)}, 5));
        }
        px_srcline(578);
        return _v1654;
    }
    px_srcline(579);
    if (px_is_truthy(px_eq(_v1656, px_str("Binary")))) {
        px_srcline(581);
        _v1658 = px_index(_v1653, px_int(1LL));
        px_srcline(582);
        _v1659 = _v1654;
        px_srcline(583);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(584);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(585);
        if (px_is_truthy(px_eq(_v1658, px_str("And")))) {
            px_srcline(587);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1659, _v1655}, 3));
            px_srcline(588);
            _v1665 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
            px_srcline(589);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1659, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(590);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(3LL)), _v1659, _v1655}, 3));
            px_srcline(591);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1665, px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1)}, 3));
            px_srcline(592);
            return _v1659;
        }
        px_srcline(593);
        if (px_is_truthy(px_eq(_v1658, px_str("Or")))) {
            px_srcline(595);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1659, _v1655}, 3));
            px_srcline(596);
            _v1665 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
            px_srcline(597);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPT"), _v1659, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(598);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(3LL)), _v1659, _v1655}, 3));
            px_srcline(599);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1665, px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1)}, 3));
            px_srcline(600);
            return _v1659;
        }
        px_srcline(601);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1659, _v1655}, 3));
        px_srcline(602);
        _v1666 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(603);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(3LL)), _v1666, _v1655}, 3));
        px_srcline(604);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_call(px_get_global("bc_binop_op"), (LXValue[]){_v1658}, 1), _v1659, _v1659, _v1666}, 5));
        px_srcline(605);
        return _v1659;
    }
    px_srcline(606);
    if (px_is_truthy(px_eq(_v1656, px_str("NullCoalesce")))) {
        px_srcline(608);
        _v1659 = _v1654;
        px_srcline(609);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(610);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(611);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1659, _v1655}, 3));
        px_srcline(612);
        _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(613);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1655, _v1667}, 2));
        px_srcline(614);
        _v1668 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(615);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("EQ"), _v1668, _v1659, _v1667}, 5));
        px_srcline(616);
        _v1665 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
        px_srcline(617);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1668, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(618);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1659, _v1655}, 3));
        px_srcline(619);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1665, px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1)}, 3));
        px_srcline(620);
        return _v1659;
    }
    px_srcline(621);
    if (px_is_truthy(px_eq(_v1656, px_str("IfExpr")))) {
        px_srcline(623);
        _v1659 = _v1654;
        px_srcline(624);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(625);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(626);
        _v1669 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(627);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1669, _v1655}, 3));
        px_srcline(628);
        _v1670 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
        px_srcline(629);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1669, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(630);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1659, _v1655}, 3));
        px_srcline(631);
        _v1671 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
        px_srcline(632);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(633);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1670, px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1)}, 3));
        px_srcline(634);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(3LL)), _v1659, _v1655}, 3));
        px_srcline(635);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1671, px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1)}, 3));
        px_srcline(636);
        return _v1659;
    }
    px_srcline(637);
    if (px_is_truthy(px_eq(_v1656, px_str("Try")))) {
        px_srcline(641);
        _v1659 = _v1654;
        px_srcline(642);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(643);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(644);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1659, _v1655}, 3));
        px_srcline(645);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("TRY"), _v1659, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(646);
        return _v1659;
    }
    px_srcline(647);
    if (px_is_truthy(px_eq(_v1656, px_str("ForceUnwrap")))) {
        px_srcline(649);
        _v1659 = _v1654;
        px_srcline(650);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(651);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(652);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1659, _v1655}, 3));
        px_srcline(653);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("FORCE"), _v1659, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(654);
        return _v1659;
    }
    px_srcline(655);
    if (px_is_truthy(({ LXValue _t1713 = px_eq(_v1656, px_str("List")); px_is_truthy(_t1713) ? _t1713 : px_eq(_v1656, px_str("Tuple")); }))) {
        px_srcline(657);
        _v1672 = px_index(_v1653, px_int(1LL));
        px_srcline(658);
        _v1673 = px_call(px_get_global("len"), (LXValue[]){_v1672}, 1);
        px_srcline(659);
        _v1659 = _v1654;
        px_srcline(660);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(661);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(662);
        _v1674 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(663);
        while (px_is_truthy(px_lt(px_index(_v1655, px_str("next_slot")), px_add(_v1674, _v1673)))) {
            px_srcline(664);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1));
        }
        px_srcline(665);
        _v1675 = px_int(0LL);
        px_srcline(666);
        while (px_is_truthy(px_lt(_v1675, _v1673))) {
            px_srcline(667);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1672, _v1675), px_add(_v1674, _v1675), _v1655}, 3));
            px_srcline(668);
             _v1675 = px_add(_v1675, px_int(1LL));
        }
        px_srcline(669);
        if (px_is_truthy(px_eq(_v1656, px_str("List")))) {
            px_srcline(670);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("NEWLIST"), _v1659, _v1674, _v1673}, 5));
        }
        else {
            px_srcline(672);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("NEWTUPLE"), _v1659, _v1674, _v1673}, 5));
        }
        px_srcline(673);
        return _v1659;
    }
    px_srcline(674);
    if (px_is_truthy(px_eq(_v1656, px_str("Dict")))) {
        px_srcline(676);
        _v1676 = px_index(_v1653, px_int(1LL));
        px_srcline(677);
        _v1673 = px_call(px_get_global("len"), (LXValue[]){_v1676}, 1);
        px_srcline(678);
        _v1659 = _v1654;
        px_srcline(679);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(680);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(681);
        _v1674 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(682);
        while (px_is_truthy(px_lt(px_index(_v1655, px_str("next_slot")), px_add(_v1674, px_mul(px_int(2LL), _v1673))))) {
            px_srcline(683);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1));
        }
        px_srcline(684);
        _v1677 = px_int(0LL);
        px_srcline(685);
        while (px_is_truthy(px_lt(_v1677, _v1673))) {
            px_srcline(686);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1676, _v1677), px_int(0LL)), px_add(_v1674, px_mul(px_int(2LL), _v1677)), _v1655}, 3));
            px_srcline(687);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1676, _v1677), px_int(1LL)), px_add(px_add(_v1674, px_mul(px_int(2LL), _v1677)), px_int(1LL)), _v1655}, 3));
            px_srcline(688);
             _v1677 = px_add(_v1677, px_int(1LL));
        }
        px_srcline(689);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("NEWDICT"), _v1659, _v1674, _v1673}, 5));
        px_srcline(690);
        return _v1659;
    }
    px_srcline(691);
    if (px_is_truthy(px_eq(_v1656, px_str("Index")))) {
        px_srcline(693);
        _v1659 = _v1654;
        px_srcline(694);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(695);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(696);
        _v1678 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(697);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1678, _v1655}, 3));
        px_srcline(698);
        _v1679 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(699);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1679, _v1655}, 3));
        px_srcline(700);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("INDEX"), _v1659, _v1678, _v1679}, 5));
        px_srcline(701);
        return _v1659;
    }
    px_srcline(702);
    if (px_is_truthy(px_eq(_v1656, px_str("Slice")))) {
        px_srcline(704);
        _v1659 = _v1654;
        px_srcline(705);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(706);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(707);
        _v1678 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(708);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1678, _v1655}, 3));
        px_srcline(709);
        _v1674 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(710);
        while (px_is_truthy(px_lt(px_index(_v1655, px_str("next_slot")), px_add(_v1674, px_int(3LL))))) {
            px_srcline(711);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1));
        }
        px_srcline(712);
        if (px_is_truthy(px_eq(px_index(_v1653, px_int(2LL)), px_null()))) {
            px_srcline(713);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1655, _v1674}, 2));
        }
        else {
            px_srcline(715);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(2LL)), _v1674, _v1655}, 3));
        }
        px_srcline(716);
        if (px_is_truthy(px_eq(px_index(_v1653, px_int(3LL)), px_null()))) {
            px_srcline(717);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1655, px_add(_v1674, px_int(1LL))}, 2));
        }
        else {
            px_srcline(719);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(3LL)), px_add(_v1674, px_int(1LL)), _v1655}, 3));
        }
        px_srcline(720);
        if (px_is_truthy(px_eq(px_index(_v1653, px_int(4LL)), px_null()))) {
            px_srcline(721);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1655, px_add(_v1674, px_int(2LL))}, 2));
        }
        else {
            px_srcline(723);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(4LL)), px_add(_v1674, px_int(2LL)), _v1655}, 3));
        }
        px_srcline(724);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("SLICE"), _v1659, _v1678, _v1674}, 5));
        px_srcline(725);
        return _v1659;
    }
    px_srcline(726);
    if (px_is_truthy(px_eq(_v1656, px_str("Field")))) {
        px_srcline(730);
        _v1680 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1653, px_int(2LL))}, 1);
        px_srcline(731);
        _v1681 = px_index(_v1653, px_int(1LL));
        px_srcline(732);
        if (px_is_truthy(px_eq(px_index(_v1681, px_int(0LL)), px_str("Var")))) {
            px_srcline(733);
            _v1682 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1681, px_int(1LL))}, 1);
            px_srcline(734);
            _v1683 = px_call(px_get_global("bc_const_find"), (LXValue[]){_v1682, _v1680}, 2);
            px_srcline(735);
            if (px_is_truthy(px_ne(_v1683, px_null()))) {
                px_srcline(736);
                _v1659 = _v1654;
                px_srcline(737);
                if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
                    px_srcline(738);
                     _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
                }
                px_srcline(739);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1683, _v1659, _v1655}, 3));
                px_srcline(740);
                return _v1659;
            }
            px_srcline(741);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1682, _v1680}, 2))) {
                px_srcline(742);
                _v1659 = _v1654;
                px_srcline(743);
                if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
                    px_srcline(744);
                     _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
                }
                px_srcline(745);
                _v1667 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1682}, 2);
                px_srcline(746);
                _v1684 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1680}, 2);
                px_srcline(747);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("NEWENUM"), _v1659, _v1667, _v1684}, 5));
                px_srcline(748);
                return _v1659;
            }
        }
        px_srcline(749);
        _v1659 = _v1654;
        px_srcline(750);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(751);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(752);
        _v1678 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(753);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1678, _v1655}, 3));
        px_srcline(754);
        _v1685 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1680}, 2);
        px_srcline(755);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("GETF"), _v1659, _v1678, _v1685}, 5));
        px_srcline(756);
        return _v1659;
    }
    px_srcline(757);
    if (px_is_truthy(px_eq(_v1656, px_str("OptionalField")))) {
        px_srcline(759);
        _v1659 = _v1654;
        px_srcline(760);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(761);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(762);
        _v1678 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(763);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1678, _v1655}, 3));
        px_srcline(764);
        _v1685 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1653, px_int(2LL))}, 1)}, 2);
        px_srcline(765);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("GETF_OPT"), _v1659, _v1678, _v1685}, 5));
        px_srcline(766);
        return _v1659;
    }
    px_srcline(767);
    if (px_is_truthy(px_eq(_v1656, px_str("Call")))) {
        px_srcline(770);
        _v1686 = px_index(_v1653, px_int(1LL));
        px_srcline(774);
        (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v1686, px_index(_v1653, px_int(2LL))}, 2));
        px_srcline(775);
        if (px_is_truthy(px_eq(px_index(_v1686, px_int(0LL)), px_str("Var")))) {
            px_srcline(776);
            _v1687 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1686, px_int(1LL))}, 1);
            px_srcline(777);
            _v1688 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1687}, 1);
            px_srcline(778);
            if (px_is_truthy(px_ge(_v1688, px_int(0LL)))) {
                px_srcline(779);
                return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1687, _v1688, px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 5);
            }
            px_srcline(780);
            if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1687}, 1))) {
                px_srcline(781);
                return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1687, px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 4);
            }
        }
        px_srcline(782);
        if (px_is_truthy(px_eq(px_index(_v1686, px_int(0LL)), px_str("Field")))) {
            px_srcline(783);
            return px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1686, px_int(1LL)), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1686, px_int(2LL))}, 1), px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 5);
        }
        px_srcline(784);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1686, px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 4);
    }
    px_srcline(785);
    if (px_is_truthy(px_eq(_v1656, px_str("Constructor")))) {
        px_srcline(787);
        _v1687 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1653, px_int(1LL))}, 1);
        px_srcline(788);
        _v1688 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1687}, 1);
        px_srcline(789);
        if (px_is_truthy(px_ge(_v1688, px_int(0LL)))) {
            px_srcline(790);
            return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1687, _v1688, px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 5);
        }
        px_srcline(791);
        if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1687}, 1))) {
            px_srcline(792);
            return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1687, px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 4);
        }
        px_srcline(794);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1653, px_int(1LL)), px_index(_v1653, px_int(2LL)), _v1654, _v1655}, 4);
    }
    px_srcline(795);
    if (px_is_truthy(px_eq(_v1656, px_str("Pipe")))) {
        px_srcline(797);
        _v1689 = px_index(_v1653, px_int(1LL));
        px_srcline(798);
        _v1690 = px_index(_v1653, px_int(2LL));
        px_srcline(799);
        if (px_is_truthy(px_eq(px_index(_v1690, px_int(0LL)), px_str("Call")))) {
            px_srcline(800);
            _v1691 = px_list_n((LXValue[]){}, 0);
            px_srcline(801);
            (void)(px_method(_v1691, "append", (LXValue[]){_v1689}, 1));
            px_srcline(802);
            _v1692 = px_int(0LL);
            px_srcline(803);
            while (px_is_truthy(px_lt(_v1692, px_call(px_get_global("len"), (LXValue[]){px_index(_v1690, px_int(2LL))}, 1)))) {
                px_srcline(804);
                (void)(px_method(_v1691, "append", (LXValue[]){px_index(px_index(_v1690, px_int(2LL)), _v1692)}, 1));
                px_srcline(805);
                 _v1692 = px_add(_v1692, px_int(1LL));
            }
            px_srcline(806);
            return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1690, px_int(1LL)), _v1691, _v1654, _v1655}, 4);
        }
        px_srcline(807);
        _v1691 = px_list_n((LXValue[]){_v1689}, 1);
        px_srcline(808);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1690, _v1691, _v1654, _v1655}, 4);
    }
    px_srcline(809);
    if (px_is_truthy(px_eq(_v1656, px_str("ListComp")))) {
        px_srcline(811);
        _v1693 = px_index(_v1653, px_int(2LL));
        px_srcline(812);
        _v1659 = _v1654;
        px_srcline(813);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(814);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(815);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("NEWLIST"), _v1659, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(816);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1655, px_str("push"), px_list_n((LXValue[]){px_index(_v1653, px_int(1LL))}, 1), px_index(_v1653, px_int(3LL)), _v1659, _v1693, px_int(0LL)}, 7));
        px_srcline(817);
        return _v1659;
    }
    px_srcline(818);
    if (px_is_truthy(px_eq(_v1656, px_str("DictComp")))) {
        px_srcline(821);
        _v1693 = px_index(_v1653, px_int(3LL));
        px_srcline(822);
        _v1659 = _v1654;
        px_srcline(823);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(824);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(825);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("NEWDICT"), _v1659, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(826);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1655, px_str("dict"), px_list_n((LXValue[]){px_index(_v1653, px_int(1LL)), px_index(_v1653, px_int(2LL))}, 2), px_index(_v1653, px_int(4LL)), _v1659, _v1693, px_int(0LL)}, 7));
        px_srcline(827);
        return _v1659;
    }
    px_srcline(828);
    if (px_is_truthy(px_eq(_v1656, px_str("GenExp")))) {
        px_srcline(829);
        return px_call(px_get_global("bc_emit_genexp"), (LXValue[]){_v1653, _v1654, _v1655}, 3);
    }
    px_srcline(830);
    if (px_is_truthy(px_eq(_v1656, px_str("Match")))) {
        px_srcline(833);
        _v1659 = _v1654;
        px_srcline(834);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(835);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(836);
        _v1694 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        px_srcline(837);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1653, px_int(1LL)), _v1694, _v1655}, 3));
        px_srcline(838);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("MOV"), _v1659, _v1694, px_int(0LL)}, 5));
        px_srcline(839);
        _v1695 = px_index(_v1653, px_int(2LL));
        px_srcline(840);
        _v1696 = px_list_n((LXValue[]){}, 0);
        px_srcline(841);
        _v1697 = px_int(0LL);
        px_srcline(842);
        while (px_is_truthy(px_lt(_v1697, px_call(px_get_global("len"), (LXValue[]){_v1695}, 1)))) {
            px_srcline(843);
            _v1698 = px_index(_v1695, _v1697);
            px_srcline(844);
            _v1699 = px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1698, px_int(1LL)), _v1694, _v1655}, 3);
            px_srcline(845);
            _v1700 = px_neg(px_int(1LL));
            px_srcline(846);
            _v1701 = px_neg(px_int(1LL));
            px_srcline(847);
            if (px_is_truthy(px_eq(_v1699, px_null()))) {
                px_srcline(849);
                if (px_is_truthy(px_ne(px_index(_v1698, px_int(2LL)), px_null()))) {
                    px_srcline(850);
                    _v1702 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
                    px_srcline(851);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1698, px_int(2LL)), _v1702, _v1655}, 3));
                    px_srcline(852);
                    _v1703 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
                    px_srcline(853);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1702, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(854);
                     _v1701 = _v1703;
                }
            }
            else {
                px_srcline(856);
                _v1670 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
                px_srcline(857);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1699, px_int(0LL), px_int(0LL)}, 5));
                px_srcline(858);
                 _v1700 = _v1670;
                px_srcline(859);
                if (px_is_truthy(px_ne(px_index(_v1698, px_int(2LL)), px_null()))) {
                    px_srcline(860);
                    _v1702 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
                    px_srcline(861);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1698, px_int(2LL)), _v1702, _v1655}, 3));
                    px_srcline(862);
                    _v1703 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
                    px_srcline(863);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMPF"), _v1702, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(864);
                     _v1701 = _v1703;
                }
            }
            px_srcline(865);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1698, px_int(3LL)), _v1659, _v1655}, 3));
            px_srcline(866);
            _v1671 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
            px_srcline(867);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1655, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(868);
            _v1704 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
            px_srcline(869);
            if (px_is_truthy(px_ge(_v1700, px_int(0LL)))) {
                px_srcline(870);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1700, _v1704}, 3));
            }
            px_srcline(871);
            if (px_is_truthy(px_ge(_v1701, px_int(0LL)))) {
                px_srcline(872);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, _v1701, _v1704}, 3));
            }
            px_srcline(873);
            (void)(px_method(_v1696, "append", (LXValue[]){_v1671}, 1));
            px_srcline(874);
             _v1697 = px_add(_v1697, px_int(1LL));
        }
        px_srcline(875);
        _v1705 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1655, px_str("bc"))}, 1);
        px_srcline(876);
        _v1706 = px_int(0LL);
        px_srcline(877);
        while (px_is_truthy(px_lt(_v1706, px_call(px_get_global("len"), (LXValue[]){_v1696}, 1)))) {
            px_srcline(878);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1655, px_index(_v1696, _v1706), _v1705}, 3));
            px_srcline(879);
             _v1706 = px_add(_v1706, px_int(1LL));
        }
        px_srcline(880);
        return _v1659;
    }
    px_srcline(881);
    if (px_is_truthy(px_eq(_v1656, px_str("Block")))) {
        px_srcline(885);
        _v1659 = _v1654;
        px_srcline(886);
        if (px_is_truthy(px_lt(_v1659, px_int(0LL)))) {
            px_srcline(887);
             _v1659 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1655}, 1);
        }
        px_srcline(888);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1655, _v1659}, 2));
        px_srcline(889);
        _v1707 = px_index(_v1653, px_int(1LL));
        px_srcline(890);
        _v1708 = px_int(0LL);
        px_srcline(891);
        while (px_is_truthy(px_lt(_v1708, px_call(px_get_global("len"), (LXValue[]){_v1707}, 1)))) {
            px_srcline(892);
            _v1709 = px_index(_v1707, _v1708);
            px_srcline(893);
            if (px_is_truthy(px_eq(px_index(_v1709, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(894);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1709, px_int(1LL)), _v1659, _v1655}, 3));
            }
            else {
                px_srcline(896);
                (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){_v1709, _v1655}, 2));
            }
            px_srcline(897);
             _v1708 = px_add(_v1708, px_int(1LL));
        }
        px_srcline(898);
        return _v1659;
    }
    px_srcline(899);
    if (px_is_truthy(px_eq(_v1656, px_str("Closure")))) {
        px_srcline(902);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1653, px_int(1LL)), px_index(_v1653, px_int(3LL)), _v1654, _v1655}, 4));
        px_srcline(903);
        return _v1654;
    }
    px_srcline(904);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_expr 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1653}, 1))}, 1));
px_err_1710:
    if (px_err_1710_proped) return px_err_1710_val;
    return px_null();
}

static LXValue fn_bc_emit_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_call");
    LXValue _v1714 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1715 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1716 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1717 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1718 = px_uninit();
    LXValue _v1719 = px_uninit();
    LXValue _v1720 = px_uninit();
    LXValue _v1721 = px_uninit();
    LXValue _v1722 = px_uninit();
    LXValue px_err_1723_val = px_null();
    int px_err_1723_proped = 0;
    px_srcline(908);
    _v1718 = _v1716;
    px_srcline(909);
    if (px_is_truthy(px_lt(_v1718, px_int(0LL)))) {
        px_srcline(910);
         _v1718 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1717}, 1);
    }
    px_srcline(911);
    _v1719 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1717}, 1);
    px_srcline(912);
    _v1720 = px_call(px_get_global("len"), (LXValue[]){_v1715}, 1);
    px_srcline(913);
    _v1721 = px_add(px_add(_v1719, px_int(1LL)), _v1720);
    px_srcline(914);
    while (px_is_truthy(px_lt(px_index(_v1717, px_str("next_slot")), _v1721))) {
        px_srcline(915);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1717}, 1));
    }
    px_srcline(916);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1714, _v1719, _v1717}, 3));
    px_srcline(917);
    _v1722 = px_int(0LL);
    px_srcline(918);
    while (px_is_truthy(px_lt(_v1722, _v1720))) {
        px_srcline(919);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1715, _v1722), px_add(px_add(_v1719, px_int(1LL)), _v1722), _v1717}, 3));
        px_srcline(920);
         _v1722 = px_add(_v1722, px_int(1LL));
    }
    px_srcline(921);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1717, px_str("CALL"), _v1718, _v1719, _v1720}, 5));
    px_srcline(922);
    return _v1718;
px_err_1723:
    if (px_err_1723_proped) return px_err_1723_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall");
    LXValue _v1724 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1725 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1726 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1727 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1728 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1729 = px_uninit();
    LXValue _v1730 = px_uninit();
    LXValue _v1731 = px_uninit();
    LXValue _v1732 = px_uninit();
    LXValue _v1733 = px_uninit();
    LXValue _v1734 = px_uninit();
    LXValue px_err_1735_val = px_null();
    int px_err_1735_proped = 0;
    px_srcline(928);
    _v1729 = _v1727;
    px_srcline(929);
    if (px_is_truthy(px_lt(_v1729, px_int(0LL)))) {
        px_srcline(930);
         _v1729 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1728}, 1);
    }
    px_srcline(931);
    _v1730 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1728}, 1);
    px_srcline(932);
    _v1731 = px_call(px_get_global("len"), (LXValue[]){_v1726}, 1);
    px_srcline(933);
    _v1732 = px_add(px_add(_v1730, px_int(1LL)), _v1731);
    px_srcline(934);
    while (px_is_truthy(px_lt(px_index(_v1728, px_str("next_slot")), _v1732))) {
        px_srcline(935);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1728}, 1));
    }
    px_srcline(936);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1724, _v1730, _v1728}, 3));
    px_srcline(937);
    _v1733 = px_int(0LL);
    px_srcline(938);
    while (px_is_truthy(px_lt(_v1733, _v1731))) {
        px_srcline(939);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1726, _v1733), px_add(px_add(_v1730, px_int(1LL)), _v1733), _v1728}, 3));
        px_srcline(940);
         _v1733 = px_add(_v1733, px_int(1LL));
    }
    px_srcline(941);
    _v1734 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1725}, 2);
    px_srcline(942);
    (void)(px_method(px_index(_v1728, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1731, _v1729, _v1730, _v1734}, 5)}, 1));
    px_srcline(943);
    return _v1729;
px_err_1735:
    if (px_err_1735_proped) return px_err_1735_val;
    return px_null();
}

static LXValue fn_bc_emit_struct_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_struct_new");
    LXValue _v1736 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1737 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1738 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1739 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1740 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1741 = px_uninit();
    LXValue _v1742 = px_uninit();
    LXValue _v1743 = px_uninit();
    LXValue _v1744 = px_uninit();
    LXValue _v1745 = px_uninit();
    LXValue px_err_1746_val = px_null();
    int px_err_1746_proped = 0;
    px_srcline(947);
    _v1741 = px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1737);
    px_srcline(948);
    _v1742 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1741, px_str("fnames"))}, 1);
    px_srcline(949);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1738}, 1), _v1742))) {
        px_srcline(950);
        (void)(px_call(px_get_global("panic"), (LXValue[]){({ LXValue _s207 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v1736), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){_v1742}, 1)), px_str(" 个字段，给出 ")); LXValue _s208 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1738}, 1)}, 1); px_add(_s207, _s208); })}, 1));
    }
    px_srcline(951);
    _v1743 = _v1739;
    px_srcline(952);
    if (px_is_truthy(px_lt(_v1743, px_int(0LL)))) {
        px_srcline(953);
         _v1743 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1740}, 1);
    }
    px_srcline(954);
    _v1744 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1740}, 1);
    px_srcline(955);
    while (px_is_truthy(px_lt(px_index(_v1740, px_str("next_slot")), px_add(_v1744, _v1742)))) {
        px_srcline(956);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1740}, 1));
    }
    px_srcline(957);
    _v1745 = px_int(0LL);
    px_srcline(958);
    while (px_is_truthy(px_lt(_v1745, _v1742))) {
        px_srcline(959);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1738, _v1745), px_add(_v1744, _v1745), _v1740}, 3));
        px_srcline(960);
         _v1745 = px_add(_v1745, px_int(1LL));
    }
    px_srcline(961);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1740, px_str("NEWSTRUCT"), _v1743, _v1737, _v1744}, 5));
    px_srcline(962);
    return _v1743;
px_err_1746:
    if (px_err_1746_proped) return px_err_1746_val;
    return px_null();
}

static LXValue fn_bc_emit_enum_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_enum_new");
    LXValue _v1747 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1748 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1749 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1750 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1751 = px_uninit();
    LXValue _v1752 = px_uninit();
    LXValue _v1753 = px_uninit();
    LXValue _v1754 = px_uninit();
    LXValue _v1755 = px_uninit();
    LXValue _v1756 = px_uninit();
    LXValue px_err_1757_val = px_null();
    int px_err_1757_proped = 0;
    px_srcline(965);
    _v1751 = px_null();
    px_srcline(966);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1748}, 1), px_int(1LL)))) {
        px_srcline(967);
        _v1752 = px_index(_v1748, px_int(0LL));
        px_srcline(968);
        if (px_is_truthy(px_eq(px_index(_v1752, px_int(0LL)), px_str("Var")))) {
            px_srcline(969);
            _v1753 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1752, px_int(1LL))}, 1);
            px_srcline(970);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1747, _v1753}, 2))) {
                px_srcline(971);
                 _v1751 = _v1753;
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v1752, px_int(0LL)), px_str("Str")))) {
            px_srcline(973);
             _v1751 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1752, px_int(1LL))}, 1);
        }
    }
    px_srcline(974);
    if (px_is_truthy(px_eq(_v1751, px_null()))) {
        px_srcline(975);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit enum 构造未实现（仅支持 Color(Variant)/Color(\"Variant\"): "), px_call(px_get_global("str"), (LXValue[]){_v1747}, 1))}, 1));
    }
    px_srcline(976);
    _v1754 = _v1749;
    px_srcline(977);
    if (px_is_truthy(px_lt(_v1754, px_int(0LL)))) {
        px_srcline(978);
         _v1754 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1750}, 1);
    }
    px_srcline(979);
    _v1755 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1747}, 2);
    px_srcline(980);
    _v1756 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1751}, 2);
    px_srcline(981);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1750, px_str("NEWENUM"), _v1754, _v1755, _v1756}, 5));
    px_srcline(982);
    return _v1754;
px_err_1757:
    if (px_err_1757_proped) return px_err_1757_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall_slot");
    LXValue _v1758 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1759 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1760 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1761 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1762 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1763 = px_uninit();
    LXValue _v1764 = px_uninit();
    LXValue _v1765 = px_uninit();
    LXValue _v1766 = px_uninit();
    LXValue _v1767 = px_uninit();
    LXValue _v1768 = px_uninit();
    LXValue px_err_1769_val = px_null();
    int px_err_1769_proped = 0;
    px_srcline(989);
    _v1763 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1758}, 1);
    px_srcline(990);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1758, px_str("MOV"), _v1763, _v1759, px_int(0LL)}, 5));
    px_srcline(991);
    _v1764 = px_call(px_get_global("len"), (LXValue[]){_v1761}, 1);
    px_srcline(992);
    _v1765 = px_add(px_add(_v1763, px_int(1LL)), _v1764);
    px_srcline(993);
    while (px_is_truthy(px_lt(px_index(_v1758, px_str("next_slot")), _v1765))) {
        px_srcline(994);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1758}, 1));
    }
    px_srcline(995);
    _v1766 = px_int(0LL);
    px_srcline(996);
    while (px_is_truthy(px_lt(_v1766, _v1764))) {
        px_srcline(997);
        _v1767 = px_index(_v1761, _v1766);
        px_srcline(998);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1767}, 1), px_str("int")))) {
            px_srcline(999);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1758, px_str("MOV"), px_add(px_add(_v1763, px_int(1LL)), _v1766), _v1767, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1001);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1767, px_add(px_add(_v1763, px_int(1LL)), _v1766), _v1758}, 3));
        }
        px_srcline(1002);
         _v1766 = px_add(_v1766, px_int(1LL));
    }
    px_srcline(1003);
    _v1768 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1760}, 2);
    px_srcline(1004);
    (void)(px_method(px_index(_v1758, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1764, _v1762, _v1763, _v1768}, 5)}, 1));
    px_srcline(1005);
    return _v1762;
px_err_1769:
    if (px_err_1769_proped) return px_err_1769_val;
    return px_null();
}

static LXValue fn_bc_emit_push_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_push_lambda");
    LXValue _v1770 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1771 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1772 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1773 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1774 = px_uninit();
    LXValue _v1775 = px_uninit();
    LXValue _v1776 = px_uninit();
    LXValue _v1777 = px_uninit();
    LXValue _v1778 = px_uninit();
    LXValue _v1779 = px_uninit();
    LXValue _v1780 = px_uninit();
    LXValue _v1781 = px_uninit();
    LXValue _v1782 = px_uninit();
    LXValue _v1783 = px_uninit();
    LXValue _v1784 = px_uninit();
    LXValue _v1785 = px_uninit();
    LXValue px_err_1786_val = px_null();
    int px_err_1786_proped = 0;
    px_srcline(1009);
    _v1774 = px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("nclosure"))}, 1)), px_str(">"));
    px_srcline(1010);
    px_index_set(px_get_global("g_bcm"), px_str("nclosure"), px_add(px_index(px_get_global("g_bcm"), px_str("nclosure")), px_int(1LL)));
    px_srcline(1011);
    _v1775 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1774, px_call(px_get_global("len"), (LXValue[]){_v1770}, 1)}, 2);
    px_srcline(1012);
    _v1776 = px_int(0LL);
    px_srcline(1013);
    while (px_is_truthy(px_lt(_v1776, px_call(px_get_global("len"), (LXValue[]){_v1770}, 1)))) {
        px_srcline(1014);
        px_index_set(px_index(_v1775, px_str("smap")), px_index(_v1770, _v1776), _v1776);
        px_srcline(1016);
        px_index_set(px_index(_v1775, px_str("inited")), px_index(_v1770, _v1776), px_int(1LL));
        px_srcline(1017);
         _v1776 = px_add(_v1776, px_int(1LL));
    }
    px_srcline(1020);
    _v1777 = px_int(0LL);
    px_srcline(1021);
    while (px_is_truthy(px_lt(_v1777, px_call(px_get_global("len"), (LXValue[]){_v1772}, 1)))) {
        px_srcline(1022);
        _v1778 = px_index(_v1772, _v1777);
        px_srcline(1023);
        px_index_set(px_index(_v1775, px_str("smap")), _v1778, px_add(px_call(px_get_global("len"), (LXValue[]){_v1770}, 1), _v1777));
        px_srcline(1024);
        (void)(px_method(px_index(_v1775, px_str("upnames")), "push", (LXValue[]){_v1778}, 1));
        px_srcline(1025);
        (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1775, _v1778}, 2));
        px_srcline(1030);
        if (px_is_truthy(({ LXValue _t1787 = px_ne(_v1773, px_null()); px_is_truthy(_t1787) ? px_method(_v1773, "has", (LXValue[]){_v1778}, 1) : _t1787; }))) {
            px_srcline(1031);
            px_index_set(px_index(_v1775, px_str("inited")), _v1778, px_int(1LL));
        }
        px_srcline(1032);
         _v1777 = px_add(_v1777, px_int(1LL));
    }
    px_srcline(1033);
    if (px_is_truthy(px_lt(px_index(_v1775, px_str("next_slot")), ({ LXValue _s209 = px_call(px_get_global("len"), (LXValue[]){_v1770}, 1); LXValue _s210 = px_call(px_get_global("len"), (LXValue[]){_v1772}, 1); px_add(_s209, _s210); })))) {
        px_srcline(1034);
        px_index_set(_v1775, px_str("next_slot"), ({ LXValue _s211 = px_call(px_get_global("len"), (LXValue[]){_v1770}, 1); LXValue _s212 = px_call(px_get_global("len"), (LXValue[]){_v1772}, 1); px_add(_s211, _s212); }));
    }
    px_srcline(1035);
    (void)(px_method(px_index(px_get_global("g_bcm"), px_str("funcs")), "push", (LXValue[]){_v1775}, 1));
    px_srcline(1040);
    _v1779 = px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("funcs"))}, 1), px_int(1LL));
    px_srcline(1044);
    _v1780 = px_list_n((LXValue[]){}, 0);
    px_srcline(1045);
    (void)(px_call(px_get_global("bc_lambda_hoist"), (LXValue[]){_v1771, _v1780}, 2));
    px_srcline(1047);
    _v1781 = px_list_n((LXValue[]){}, 0);
    px_srcline(1048);
    (void)(px_call(px_get_global("bc_lambda_decl"), (LXValue[]){_v1771, _v1781}, 2));
    px_srcline(1049);
    _v1782 = px_int(0LL);
    px_srcline(1050);
    while (px_is_truthy(px_lt(_v1782, px_call(px_get_global("len"), (LXValue[]){_v1780}, 1)))) {
        px_srcline(1051);
        _v1783 = px_index(_v1780, _v1782);
        px_srcline(1060);
        if (px_is_truthy(({ LXValue _t1789 = px_not(px_method(px_index(_v1775, px_str("smap")), "has", (LXValue[]){_v1783}, 1)); px_is_truthy(_t1789) ? px_not(({ LXValue _t1788 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1781, _v1783}, 2)); px_is_truthy(_t1788) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v1783}, 2) : _t1788; })) : _t1789; }))) {
            px_srcline(1061);
            (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v1775, _v1783}, 2));
            px_srcline(1062);
            (void)(px_method(px_index(_v1775, px_str("hoist_slots")), "push", (LXValue[]){px_index(px_index(_v1775, px_str("smap")), _v1783)}, 1));
        }
        px_srcline(1063);
         _v1782 = px_add(_v1782, px_int(1LL));
    }
    px_srcline(1065);
    _v1784 = px_int(0LL);
    px_srcline(1066);
    while (px_is_truthy(px_lt(_v1784, px_call(px_get_global("len"), (LXValue[]){px_index(_v1775, px_str("hoist_slots"))}, 1)))) {
        px_srcline(1067);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1775, px_str("UNINIT"), px_index(px_index(_v1775, px_str("hoist_slots")), _v1784), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1068);
         _v1784 = px_add(_v1784, px_int(1LL));
    }
    px_srcline(1070);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v1775, _v1771}, 2));
    px_srcline(1071);
    _v1785 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1775}, 1);
    px_srcline(1072);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1771, _v1785, _v1775}, 3));
    px_srcline(1073);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1775, px_str("RET"), _v1785, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1074);
    px_index_set(_v1775, px_str("nslots"), px_index(_v1775, px_str("next_slot")));
    px_srcline(1075);
    return _v1779;
px_err_1786:
    if (px_err_1786_proped) return px_err_1786_val;
    return px_null();
}

static LXValue fn_bc_emit_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_closure");
    LXValue _v1790 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1791 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1792 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1793 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1794 = px_uninit();
    LXValue _v1795 = px_uninit();
    LXValue _v1796 = px_uninit();
    LXValue _v1797 = px_uninit();
    LXValue _v1798 = px_uninit();
    LXValue _v1799 = px_uninit();
    LXValue _v1800 = px_uninit();
    LXValue _v1801 = px_uninit();
    LXValue _v1802 = px_uninit();
    LXValue _v1803 = px_uninit();
    LXValue _v1804 = px_uninit();
    LXValue _v1805 = px_uninit();
    LXValue _v1806 = px_uninit();
    LXValue px_err_1807_val = px_null();
    int px_err_1807_proped = 0;
    px_srcline(1080);
    _v1794 = px_list_n((LXValue[]){}, 0);
    px_srcline(1081);
    _v1795 = px_int(0LL);
    px_srcline(1082);
    while (px_is_truthy(px_lt(_v1795, px_call(px_get_global("len"), (LXValue[]){_v1790}, 1)))) {
        px_srcline(1083);
        (void)(px_method(_v1794, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1790, _v1795), px_int(1LL))}, 1)}, 1));
        px_srcline(1084);
         _v1795 = px_add(_v1795, px_int(1LL));
    }
    px_srcline(1085);
    _v1796 = px_call(px_get_global("bc_closure_free"), (LXValue[]){_v1790, _v1791}, 2);
    px_srcline(1086);
    _v1797 = px_list_n((LXValue[]){}, 0);
    px_srcline(1087);
    _v1798 = px_int(0LL);
    px_srcline(1088);
    while (px_is_truthy(px_lt(_v1798, px_call(px_get_global("len"), (LXValue[]){_v1796}, 1)))) {
        px_srcline(1089);
        _v1799 = px_index(_v1796, _v1798);
        px_srcline(1090);
        if (px_is_truthy(px_method(px_index(_v1793, px_str("smap")), "has", (LXValue[]){_v1799}, 1))) {
            px_srcline(1091);
            (void)(px_method(_v1797, "append", (LXValue[]){_v1799}, 1));
        }
        px_srcline(1092);
         _v1798 = px_add(_v1798, px_int(1LL));
    }
    px_srcline(1093);
    _v1800 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){_v1794, _v1791, _v1797, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1793}, 1)}, 4);
    px_srcline(1094);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1797}, 1), px_int(0LL)))) {
        px_srcline(1095);
        _v1801 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1800, px_float(0), px_str("")}, 5);
        px_srcline(1096);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1793, px_str("LOADK"), _v1792, _v1801, px_int(0LL)}, 5));
        px_srcline(1097);
        return _v1792;
    }
    px_srcline(1100);
    _v1802 = px_index(_v1793, px_str("next_slot"));
    px_srcline(1101);
    _v1803 = px_int(0LL);
    px_srcline(1102);
    while (px_is_truthy(px_lt(_v1803, px_call(px_get_global("len"), (LXValue[]){_v1797}, 1)))) {
        px_srcline(1103);
        _v1804 = px_index(px_index(_v1793, px_str("smap")), px_index(_v1797, _v1803));
        px_srcline(1104);
        _v1805 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1793}, 1);
        px_srcline(1105);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1793, px_str("MOV"), _v1805, _v1804, px_int(0LL)}, 5));
        px_srcline(1106);
         _v1803 = px_add(_v1803, px_int(1LL));
    }
    px_srcline(1107);
    _v1806 = _v1792;
    px_srcline(1108);
    if (px_is_truthy(px_lt(_v1806, px_int(0LL)))) {
        px_srcline(1109);
         _v1806 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1793}, 1);
    }
    px_srcline(1110);
    (void)(px_method(px_index(_v1793, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1806, _v1800, _v1802}, 5)}, 1));
    px_srcline(1111);
    return _v1806;
px_err_1807:
    if (px_err_1807_proped) return px_err_1807_val;
    return px_null();
}

static LXValue fn_bc_emit_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_comp");
    LXValue _v1808 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1809 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1810 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1811 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1812 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1813 = (nargs > 5) ? args[5] : px_null();
    LXValue _v1814 = (nargs > 6) ? args[6] : px_null();
    LXValue _v1815 = px_uninit();
    LXValue _v1816 = px_uninit();
    LXValue _v1817 = px_uninit();
    LXValue _v1818 = px_uninit();
    LXValue _v1819 = px_uninit();
    LXValue _v1820 = px_uninit();
    LXValue _v1821 = px_uninit();
    LXValue _v1822 = px_uninit();
    LXValue _v1823 = px_uninit();
    LXValue _v1824 = px_uninit();
    LXValue _v1825 = px_uninit();
    LXValue _v1826 = px_uninit();
    LXValue _v1827 = px_uninit();
    LXValue _v1828 = px_uninit();
    LXValue _v1829 = px_uninit();
    LXValue _v1830 = px_uninit();
    LXValue _v1831 = px_uninit();
    LXValue _v1832 = px_uninit();
    LXValue _v1833 = px_uninit();
    LXValue _v1834 = px_uninit();
    LXValue _v1835 = px_uninit();
    LXValue _v1836 = px_uninit();
    LXValue _v1837 = px_uninit();
    LXValue _v1838 = px_uninit();
    LXValue _v1839 = px_uninit();
    LXValue _v1840 = px_uninit();
    LXValue _v1841 = px_uninit();
    LXValue _v1842 = px_uninit();
    LXValue _v1843 = px_uninit();
    LXValue px_err_1844_val = px_null();
    int px_err_1844_proped = 0;
    px_srcline(1118);
    _v1815 = px_call(px_get_global("len"), (LXValue[]){_v1813}, 1);
    px_srcline(1119);
    if (px_is_truthy(px_ge(_v1814, _v1815))) {
        px_srcline(1120);
        _v1816 = px_neg(px_int(1LL));
        px_srcline(1121);
        if (px_is_truthy(px_ne(_v1811, px_null()))) {
            px_srcline(1122);
            _v1817 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
            px_srcline(1123);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1811, _v1817, _v1808}, 3));
            px_srcline(1124);
             _v1816 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1808, px_str("bc"))}, 1);
            px_srcline(1125);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("JMPF"), _v1817, px_int(0LL), px_int(0LL)}, 5));
        }
        px_srcline(1126);
        if (px_is_truthy(px_eq(_v1809, px_str("dict")))) {
            px_srcline(1127);
            _v1818 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
            px_srcline(1128);
            _v1819 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
            px_srcline(1129);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1810, px_int(0LL)), _v1818, _v1808}, 3));
            px_srcline(1130);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1810, px_int(1LL)), _v1819, _v1808}, 3));
            px_srcline(1135);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("DICTSET"), _v1812, _v1818, _v1819}, 5));
        }
        else {
            px_srcline(1137);
            _v1820 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
            px_srcline(1138);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1810, px_int(0LL)), _v1820, _v1808}, 3));
            px_srcline(1139);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("LISTPUSH"), _v1820, _v1812, px_int(0LL)}, 5));
        }
        px_srcline(1140);
        if (px_is_truthy(px_ge(_v1816, px_int(0LL)))) {
            px_srcline(1141);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1808, _v1816, px_call(px_get_global("len"), (LXValue[]){px_index(_v1808, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1142);
        return px_null();
    }
    px_srcline(1143);
    _v1821 = px_index(_v1813, _v1814);
    px_srcline(1144);
    _v1822 = px_index(_v1821, px_int(1LL));
    px_srcline(1150);
    _v1823 = px_list_n((LXValue[]){}, 0);
    px_srcline(1151);
    _v1824 = px_int(0LL);
    px_srcline(1152);
    while (px_is_truthy(px_lt(_v1824, px_call(px_get_global("len"), (LXValue[]){_v1822}, 1)))) {
        px_srcline(1153);
        _v1825 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1822, _v1824)}, 1);
        px_srcline(1154);
        _v1826 = px_neg(px_int(1LL));
        px_srcline(1155);
        if (px_is_truthy(px_method(px_index(_v1808, px_str("smap")), "has", (LXValue[]){_v1825}, 1))) {
            px_srcline(1156);
             _v1826 = px_index(px_index(_v1808, px_str("smap")), _v1825);
        }
        px_srcline(1157);
        _v1827 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
        px_srcline(1158);
        px_index_set(px_index(_v1808, px_str("smap")), _v1825, _v1827);
        px_srcline(1159);
        (void)(px_method(_v1823, "append", (LXValue[]){px_list_n((LXValue[]){_v1825, _v1826}, 2)}, 1));
        px_srcline(1160);
         _v1824 = px_add(_v1824, px_int(1LL));
    }
    px_srcline(1161);
    _v1828 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
    px_srcline(1162);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1821, px_int(2LL)), _v1828, _v1808}, 3));
    px_srcline(1163);
    _v1829 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
    px_srcline(1164);
    _v1830 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
    px_srcline(1165);
    while (px_is_truthy(px_lt(px_index(_v1808, px_str("next_slot")), px_add(_v1829, px_int(2LL))))) {
        px_srcline(1166);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1));
    }
    px_srcline(1167);
    _v1831 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1168);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("GETG"), _v1829, _v1831, px_int(0LL)}, 5));
    px_srcline(1169);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("MOV"), px_add(_v1829, px_int(1LL)), _v1828, px_int(0LL)}, 5));
    px_srcline(1170);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("CALL"), _v1830, _v1829, px_int(1LL)}, 5));
    px_srcline(1171);
    _v1832 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
    px_srcline(1172);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1808, _v1832, px_int(0LL)}, 3));
    px_srcline(1173);
    _v1833 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1808, px_str("bc"))}, 1);
    px_srcline(1174);
    _v1834 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
    px_srcline(1175);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("LT"), _v1834, _v1832, _v1830}, 5));
    px_srcline(1176);
    _v1835 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1808, px_str("bc"))}, 1);
    px_srcline(1177);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("JMPF"), _v1834, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1179);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("ITERLEN"), _v1828, _v1830, px_int(0LL)}, 5));
    px_srcline(1180);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1822}, 1), px_int(1LL)))) {
        px_srcline(1182);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("ITERAT"), px_index(px_index(_v1808, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1822, px_int(0LL))}, 1)), _v1828, _v1832}, 5));
    }
    else {
        px_srcline(1187);
        _v1836 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
        px_srcline(1188);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("ITERAT"), _v1836, _v1828, _v1832}, 5));
        px_srcline(1189);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("UNPACKCK"), _v1836, px_call(px_get_global("len"), (LXValue[]){_v1822}, 1), px_int(0LL)}, 5));
        px_srcline(1190);
        _v1837 = px_int(0LL);
        px_srcline(1191);
        while (px_is_truthy(px_lt(_v1837, px_call(px_get_global("len"), (LXValue[]){_v1822}, 1)))) {
            px_srcline(1192);
            _v1838 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
            px_srcline(1193);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1808, _v1838, _v1837}, 3));
            px_srcline(1194);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("INDEX"), px_index(px_index(_v1808, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1822, _v1837)}, 1)), _v1836, _v1838}, 5));
            px_srcline(1195);
             _v1837 = px_add(_v1837, px_int(1LL));
        }
    }
    px_srcline(1196);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1808, _v1809, _v1810, _v1811, _v1812, _v1813, px_add(_v1814, px_int(1LL))}, 7));
    px_srcline(1197);
    _v1839 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1808}, 1);
    px_srcline(1198);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1808, _v1839, px_int(1LL)}, 3));
    px_srcline(1199);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("ADD"), _v1832, _v1832, _v1839}, 5));
    px_srcline(1200);
    _v1840 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1808, px_str("bc"))}, 1);
    px_srcline(1201);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1808, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1202);
    _v1841 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1808, px_str("bc"))}, 1);
    px_srcline(1203);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1808, _v1840, _v1833}, 3));
    px_srcline(1204);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1808, _v1835, _v1841}, 3));
    px_srcline(1206);
    _v1842 = px_int(0LL);
    px_srcline(1207);
    while (px_is_truthy(px_lt(_v1842, px_call(px_get_global("len"), (LXValue[]){_v1823}, 1)))) {
        px_srcline(1208);
        _v1843 = px_index(_v1823, _v1842);
        px_srcline(1209);
        if (px_is_truthy(px_lt(px_index(_v1843, px_int(1LL)), px_int(0LL)))) {
            px_srcline(1210);
            (void)(px_method(px_index(_v1808, px_str("smap")), "remove", (LXValue[]){px_index(_v1843, px_int(0LL))}, 1));
        }
        else {
            px_srcline(1212);
            px_index_set(px_index(_v1808, px_str("smap")), px_index(_v1843, px_int(0LL)), px_index(_v1843, px_int(1LL)));
        }
        px_srcline(1213);
         _v1842 = px_add(_v1842, px_int(1LL));
    }
px_err_1844:
    if (px_err_1844_proped) return px_err_1844_val;
    return px_null();
}

static LXValue fn_bc_emit_caps_snapshot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_caps_snapshot");
    LXValue _v1845 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1846 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1847 = px_uninit();
    LXValue _v1848 = px_uninit();
    LXValue _v1849 = px_uninit();
    LXValue _v1850 = px_uninit();
    LXValue _v1851 = px_uninit();
    LXValue _v1852 = px_uninit();
    LXValue _v1853 = px_uninit();
    LXValue _v1854 = px_uninit();
    LXValue _v1855 = px_uninit();
    LXValue px_err_1856_val = px_null();
    int px_err_1856_proped = 0;
    px_srcline(1221);
    _v1847 = px_call(px_get_global("len"), (LXValue[]){_v1846}, 1);
    px_srcline(1222);
    _v1848 = px_index(_v1845, px_str("next_slot"));
    px_srcline(1223);
    _v1849 = px_int(0LL);
    px_srcline(1224);
    while (px_is_truthy(px_lt(_v1849, _v1847))) {
        px_srcline(1225);
        _v1850 = px_index(_v1846, _v1849);
        px_srcline(1226);
        _v1851 = px_index(px_index(_v1845, px_str("smap")), _v1850);
        px_srcline(1227);
        _v1852 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1845}, 1);
        px_srcline(1228);
        if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1845, _v1850}, 2))) {
            px_srcline(1229);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1845, px_str("CELLGET"), _v1852, _v1851, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1231);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1845, px_str("MOV"), _v1852, _v1851, px_int(0LL)}, 5));
        }
        px_srcline(1232);
         _v1849 = px_add(_v1849, px_int(1LL));
    }
    px_srcline(1233);
    _v1853 = px_index(_v1845, px_str("next_slot"));
    px_srcline(1234);
    _v1854 = px_int(0LL);
    px_srcline(1235);
    while (px_is_truthy(px_lt(_v1854, _v1847))) {
        px_srcline(1236);
        _v1855 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1845}, 1);
        px_srcline(1237);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1845, px_str("CELLNEW"), _v1855, px_add(_v1848, _v1854), px_int(0LL)}, 5));
        px_srcline(1238);
         _v1854 = px_add(_v1854, px_int(1LL));
    }
    px_srcline(1239);
    return _v1853;
px_err_1856:
    if (px_err_1856_proped) return px_err_1856_val;
    return px_null();
}

static LXValue fn_bc_genexp_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_genexp_caps");
    LXValue _v1857 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1858 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1859 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1860 = px_uninit();
    LXValue _v1861 = px_uninit();
    LXValue _v1862 = px_uninit();
    LXValue _v1863 = px_uninit();
    LXValue px_err_1864_val = px_null();
    int px_err_1864_proped = 0;
    px_srcline(1254);
    _v1860 = px_list_n((LXValue[]){}, 0);
    px_srcline(1255);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1858, _v1860}, 2));
    px_srcline(1256);
    _v1861 = px_list_n((LXValue[]){}, 0);
    px_srcline(1257);
    _v1862 = px_int(0LL);
    px_srcline(1258);
    while (px_is_truthy(px_lt(_v1862, px_call(px_get_global("len"), (LXValue[]){_v1860}, 1)))) {
        px_srcline(1259);
        _v1863 = px_index(_v1860, _v1862);
        px_srcline(1260);
        if (px_is_truthy(({ LXValue _t1865 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1857, _v1863}, 2)); px_is_truthy(_t1865) ? px_method(px_index(_v1859, px_str("smap")), "has", (LXValue[]){_v1863}, 1) : _t1865; }))) {
            px_srcline(1261);
            (void)(px_method(_v1861, "append", (LXValue[]){_v1863}, 1));
        }
        px_srcline(1262);
         _v1862 = px_add(_v1862, px_int(1LL));
    }
    px_srcline(1263);
    return _v1861;
px_err_1864:
    if (px_err_1864_proped) return px_err_1864_val;
    return px_null();
}

static LXValue fn_bc_emit_genexp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_genexp");
    LXValue _v1866 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1867 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1868 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1869 = px_uninit();
    LXValue _v1870 = px_uninit();
    LXValue _v1871 = px_uninit();
    LXValue _v1872 = px_uninit();
    LXValue _v1873 = px_uninit();
    LXValue _v1874 = px_uninit();
    LXValue _v1875 = px_uninit();
    LXValue _v1876 = px_uninit();
    LXValue _v1877 = px_uninit();
    LXValue _v1878 = px_uninit();
    LXValue _v1879 = px_uninit();
    LXValue _v1880 = px_uninit();
    LXValue _v1881 = px_uninit();
    LXValue px_err_1882_val = px_null();
    int px_err_1882_proped = 0;
    px_srcline(1267);
    _v1869 = px_index(_v1866, px_int(2LL));
    px_srcline(1268);
    if (px_is_truthy(({ LXValue _t1883 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1869}, 1), px_int(1LL)); px_is_truthy(_t1883) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1869, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1883; }))) {
        px_srcline(1271);
        _v1870 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1869, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
        px_srcline(1272);
        _v1871 = _v1867;
        px_srcline(1273);
        if (px_is_truthy(px_lt(_v1871, px_int(0LL)))) {
            px_srcline(1274);
             _v1871 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1868}, 1);
        }
        px_srcline(1275);
        _v1872 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1868}, 1);
        px_srcline(1276);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1869, px_int(0LL)), px_int(2LL)), _v1872, _v1868}, 3));
        px_srcline(1282);
        _v1873 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){px_list_n((LXValue[]){_v1870}, 1), px_index(_v1866, px_int(1LL)), _v1868}, 3);
        px_srcline(1283);
        _v1874 = px_list_n((LXValue[]){}, 0);
        px_srcline(1284);
        if (px_is_truthy(px_ne(px_index(_v1866, px_int(3LL)), px_null()))) {
            px_srcline(1285);
             _v1874 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){px_list_n((LXValue[]){_v1870}, 1), px_index(_v1866, px_int(3LL)), _v1868}, 3);
        }
        px_srcline(1286);
        _v1875 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1870}, 1), px_index(_v1866, px_int(1LL)), _v1873, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1868}, 1)}, 4);
        px_srcline(1287);
        _v1876 = px_neg(px_int(1LL));
        px_srcline(1288);
        if (px_is_truthy(px_ne(px_index(_v1866, px_int(3LL)), px_null()))) {
            px_srcline(1289);
             _v1876 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1870}, 1), px_index(_v1866, px_int(3LL)), _v1874, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1868}, 1)}, 4);
        }
        px_srcline(1290);
        _v1877 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1868}, 1);
        px_srcline(1291);
        while (px_is_truthy(px_lt(px_index(_v1868, px_str("next_slot")), px_add(_v1877, px_int(2LL))))) {
            px_srcline(1292);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1868}, 1));
        }
        px_srcline(1295);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1873}, 1), px_int(0LL)))) {
            px_srcline(1296);
            _v1878 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1875, px_float(0), px_str("")}, 5);
            px_srcline(1297);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1868, px_str("LOADK"), _v1877, _v1878, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1299);
            _v1879 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1868, _v1873}, 2);
            px_srcline(1300);
            (void)(px_method(px_index(_v1868, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1877, _v1875, _v1879}, 5)}, 1));
        }
        px_srcline(1301);
        if (px_is_truthy(px_ge(_v1876, px_int(0LL)))) {
            px_srcline(1302);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1874}, 1), px_int(0LL)))) {
                px_srcline(1303);
                _v1880 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1876, px_float(0), px_str("")}, 5);
                px_srcline(1304);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1868, px_str("LOADK"), px_add(_v1877, px_int(1LL)), _v1880, px_int(0LL)}, 5));
            }
            else {
                px_srcline(1306);
                _v1881 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1868, _v1874}, 2);
                px_srcline(1307);
                (void)(px_method(px_index(_v1868, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), px_add(_v1877, px_int(1LL)), _v1876, _v1881}, 5)}, 1));
            }
        }
        else {
            px_srcline(1309);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1868, px_add(_v1877, px_int(1LL))}, 2));
        }
        px_srcline(1310);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1868, px_str("NEWGEN"), _v1871, _v1872, _v1877}, 5));
        px_srcline(1311);
        return _v1871;
    }
    px_srcline(1315);
    _v1871 = _v1867;
    px_srcline(1316);
    if (px_is_truthy(px_lt(_v1871, px_int(0LL)))) {
        px_srcline(1317);
         _v1871 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1868}, 1);
    }
    px_srcline(1318);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1868, px_str("NEWLIST"), _v1871, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1319);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1868, px_str("push"), px_list_n((LXValue[]){px_index(_v1866, px_int(1LL))}, 1), px_index(_v1866, px_int(3LL)), _v1871, _v1869, px_int(0LL)}, 7));
    px_srcline(1320);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1868, px_str("GENFROMLIST"), _v1871, _v1871, px_int(0LL)}, 5));
    px_srcline(1321);
    return _v1871;
px_err_1882:
    if (px_err_1882_proped) return px_err_1882_val;
    return px_null();
}

static LXValue fn_bc_match_enumvar(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_enumvar");
    LXValue _v1884 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1885 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1886 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1887 = px_uninit();
    LXValue _v1888 = px_uninit();
    LXValue _v1889 = px_uninit();
    LXValue _v1890 = px_uninit();
    LXValue px_err_1891_val = px_null();
    int px_err_1891_proped = 0;
    px_srcline(1327);
    _v1887 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1886}, 1);
    px_srcline(1328);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1886, px_str("ENUMVAR"), _v1887, _v1885, px_int(0LL)}, 5));
    px_srcline(1329);
    _v1888 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1884}, 5);
    px_srcline(1330);
    _v1889 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1886}, 1);
    px_srcline(1331);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1886, px_str("LOADK"), _v1889, _v1888, px_int(0LL)}, 5));
    px_srcline(1332);
    _v1890 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1886}, 1);
    px_srcline(1333);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1886, px_str("EQ"), _v1890, _v1887, _v1889}, 5));
    px_srcline(1334);
    return _v1890;
px_err_1891:
    if (px_err_1891_proped) return px_err_1891_val;
    return px_null();
}

static LXValue fn_bc_match_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_cond");
    LXValue _v1892 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1893 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1894 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1895 = px_uninit();
    LXValue _v1896 = px_uninit();
    LXValue _v1897 = px_uninit();
    LXValue _v1898 = px_uninit();
    LXValue _v1899 = px_uninit();
    LXValue px_err_1900_val = px_null();
    int px_err_1900_proped = 0;
    px_srcline(1336);
    _v1895 = px_index(_v1892, px_int(0LL));
    px_srcline(1337);
    if (px_is_truthy(px_eq(_v1895, px_str("PatWildcard")))) {
        px_srcline(1338);
        return px_null();
    }
    px_srcline(1339);
    if (px_is_truthy(px_eq(_v1895, px_str("PatBinding")))) {
        px_srcline(1340);
        _v1896 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1892, px_int(1LL))}, 1);
        px_srcline(1341);
        if (px_is_truthy(({ LXValue _t1902 = ({ LXValue _t1901 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1896}, 1), px_int(0LL)); px_is_truthy(_t1901) ? px_ge(px_index(_v1896, px_int(0LL)), px_str("A")) : _t1901; }); px_is_truthy(_t1902) ? px_le(px_index(_v1896, px_int(0LL)), px_str("Z")) : _t1902; }))) {
            px_srcline(1343);
            return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){_v1896, _v1893, _v1894}, 3);
        }
        px_srcline(1344);
        return px_null();
    }
    px_srcline(1345);
    if (px_is_truthy(px_eq(_v1895, px_str("PatTuple")))) {
        px_srcline(1346);
        _v1897 = px_index(_v1892, px_int(1LL));
        px_srcline(1347);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1897}, 1), px_int(0LL)))) {
            px_srcline(1348);
            return px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1897, px_int(0LL)), _v1893, _v1894}, 3);
        }
        px_srcline(1349);
        return px_null();
    }
    px_srcline(1350);
    if (px_is_truthy(px_eq(_v1895, px_str("PatConstructor")))) {
        px_srcline(1351);
        return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1892, px_int(1LL))}, 1), _v1893, _v1894}, 3);
    }
    px_srcline(1352);
    if (px_is_truthy(px_eq(_v1895, px_str("PatLiteral")))) {
        px_srcline(1353);
        _v1898 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1894}, 1);
        px_srcline(1354);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1892, px_int(1LL)), _v1898, _v1894}, 3));
        px_srcline(1355);
        _v1899 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1894}, 1);
        px_srcline(1356);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1894, px_str("EQ"), _v1899, _v1893, _v1898}, 5));
        px_srcline(1357);
        return _v1899;
    }
    px_srcline(1358);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_match_cond 未知 pattern: "), px_call(px_get_global("str"), (LXValue[]){_v1892}, 1))}, 1));
px_err_1900:
    if (px_err_1900_proped) return px_err_1900_val;
    return px_null();
}

static LXValue fn_bc_chk_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_chk_slot");
    LXValue _v1903 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1904 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1905 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1906 = px_uninit();
    LXValue px_err_1907_val = px_null();
    int px_err_1907_proped = 0;
    px_srcline(1363);
    if (px_is_truthy(px_not(px_method(px_index(_v1903, px_str("inited")), "has", (LXValue[]){_v1904}, 1)))) {
        px_srcline(1364);
        _v1906 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1904}, 2);
        px_srcline(1365);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1903, px_str("CHKINIT"), _v1905, px_int(0LL), _v1906}, 5));
    }
px_err_1907:
    if (px_err_1907_proped) return px_err_1907_val;
    return px_null();
}

static LXValue fn_bc_assign_local_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_local_slot");
    LXValue _v1908 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1909 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1910 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1911 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1912 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1913 = px_uninit();
    LXValue _v1914 = px_uninit();
    LXValue px_err_1915_val = px_null();
    int px_err_1915_proped = 0;
    px_srcline(1367);
    if (px_is_truthy(px_eq(_v1910, px_str("Assign")))) {
        px_srcline(1374);
        _v1913 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1912}, 1);
        px_srcline(1375);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1911, _v1913, _v1912}, 3));
        px_srcline(1376);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1912, px_str("MOV"), _v1908, _v1913, px_int(0LL)}, 5));
        px_srcline(1377);
        return px_null();
    }
    px_srcline(1378);
    _v1914 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1912}, 1);
    px_srcline(1379);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1911, _v1914, _v1912}, 3));
    px_srcline(1382);
    (void)(px_call(px_get_global("bc_chk_slot"), (LXValue[]){_v1912, _v1909, _v1908}, 3));
    px_srcline(1383);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1912, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1910}, 1)}, 1), _v1908, _v1908, _v1914}, 5));
px_err_1915:
    if (px_err_1915_proped) return px_err_1915_val;
    return px_null();
}

static LXValue fn_bc_assign_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_global");
    LXValue _v1916 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1917 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1918 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1919 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1920 = px_uninit();
    LXValue _v1921 = px_uninit();
    LXValue _v1922 = px_uninit();
    LXValue px_err_1923_val = px_null();
    int px_err_1923_proped = 0;
    px_srcline(1385);
    _v1920 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1916}, 2);
    px_srcline(1386);
    if (px_is_truthy(px_eq(_v1917, px_str("Assign")))) {
        px_srcline(1387);
        _v1921 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1919}, 1);
        px_srcline(1388);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1918, _v1921, _v1919}, 3));
        px_srcline(1389);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1919, px_str("SETG"), _v1920, _v1921, px_int(0LL)}, 5));
        px_srcline(1390);
        return px_null();
    }
    px_srcline(1391);
    _v1921 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1919}, 1);
    px_srcline(1392);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1919, px_str("GETG"), _v1921, _v1920, px_int(0LL)}, 5));
    px_srcline(1393);
    _v1922 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1919}, 1);
    px_srcline(1394);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1918, _v1922, _v1919}, 3));
    px_srcline(1395);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1919, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1917}, 1)}, 1), _v1921, _v1921, _v1922}, 5));
    px_srcline(1396);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1919, px_str("SETG"), _v1920, _v1921, px_int(0LL)}, 5));
px_err_1923:
    if (px_err_1923_proped) return px_err_1923_val;
    return px_null();
}

static LXValue fn_bc_assign_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_index");
    LXValue _v1924 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1925 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1926 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1927 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1928 = px_uninit();
    LXValue _v1929 = px_uninit();
    LXValue _v1930 = px_uninit();
    LXValue _v1931 = px_uninit();
    LXValue px_err_1932_val = px_null();
    int px_err_1932_proped = 0;
    px_srcline(1399);
    _v1928 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1927}, 1);
    px_srcline(1400);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1924, px_int(1LL)), _v1928, _v1927}, 3));
    px_srcline(1401);
    _v1929 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1927}, 1);
    px_srcline(1402);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1924, px_int(2LL)), _v1929, _v1927}, 3));
    px_srcline(1403);
    _v1930 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1927}, 1);
    px_srcline(1404);
    if (px_is_truthy(px_eq(_v1925, px_str("Assign")))) {
        px_srcline(1405);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1926, _v1930, _v1927}, 3));
    }
    else {
        px_srcline(1407);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1927, px_str("INDEX"), _v1930, _v1928, _v1929}, 5));
        px_srcline(1408);
        _v1931 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1927}, 1);
        px_srcline(1409);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1926, _v1931, _v1927}, 3));
        px_srcline(1410);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1927, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1925}, 1)}, 1), _v1930, _v1930, _v1931}, 5));
    }
    px_srcline(1411);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1927, px_str("SETIDX"), _v1930, _v1928, _v1929}, 5));
px_err_1932:
    if (px_err_1932_proped) return px_err_1932_val;
    return px_null();
}

static LXValue fn_bc_assign_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_field");
    LXValue _v1933 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1934 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1935 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1936 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1937 = px_uninit();
    LXValue _v1938 = px_uninit();
    LXValue _v1939 = px_uninit();
    LXValue _v1940 = px_uninit();
    LXValue px_err_1941_val = px_null();
    int px_err_1941_proped = 0;
    px_srcline(1414);
    _v1937 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
    px_srcline(1415);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1933, px_int(1LL)), _v1937, _v1936}, 3));
    px_srcline(1416);
    _v1938 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1933, px_int(2LL))}, 1)}, 2);
    px_srcline(1417);
    _v1939 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
    px_srcline(1418);
    if (px_is_truthy(px_eq(_v1934, px_str("Assign")))) {
        px_srcline(1419);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1935, _v1939, _v1936}, 3));
    }
    else {
        px_srcline(1421);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1936, px_str("GETF"), _v1939, _v1937, _v1938}, 5));
        px_srcline(1422);
        _v1940 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
        px_srcline(1423);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1935, _v1940, _v1936}, 3));
        px_srcline(1424);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1936, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1934}, 1)}, 1), _v1939, _v1939, _v1940}, 5));
    }
    px_srcline(1425);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1936, px_str("SETF"), _v1939, _v1937, _v1938}, 5));
px_err_1941:
    if (px_err_1941_proped) return px_err_1941_val;
    return px_null();
}

static LXValue fn_bc_assign_op_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_op_name");
    LXValue _v1942 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1943_val = px_null();
    int px_err_1943_proped = 0;
    px_srcline(1428);
    if (px_is_truthy(px_eq(_v1942, px_str("Plus")))) {
        px_srcline(1429);
        return px_str("Add");
    }
    px_srcline(1430);
    if (px_is_truthy(px_eq(_v1942, px_str("Minus")))) {
        px_srcline(1431);
        return px_str("Sub");
    }
    px_srcline(1432);
    if (px_is_truthy(px_eq(_v1942, px_str("Star")))) {
        px_srcline(1433);
        return px_str("Mul");
    }
    px_srcline(1434);
    if (px_is_truthy(px_eq(_v1942, px_str("Slash")))) {
        px_srcline(1435);
        return px_str("Div");
    }
    px_srcline(1436);
    if (px_is_truthy(px_eq(_v1942, px_str("IntDiv")))) {
        px_srcline(1437);
        return px_str("IntDiv");
    }
    px_srcline(1438);
    if (px_is_truthy(px_eq(_v1942, px_str("Mod")))) {
        px_srcline(1439);
        return px_str("Mod");
    }
    px_srcline(1440);
    if (px_is_truthy(px_eq(_v1942, px_str("Pow")))) {
        px_srcline(1441);
        return px_str("Pow");
    }
    px_srcline(1442);
    if (px_is_truthy(px_eq(_v1942, px_str("BitAnd")))) {
        px_srcline(1443);
        return px_str("BitAnd");
    }
    px_srcline(1444);
    if (px_is_truthy(px_eq(_v1942, px_str("BitOr")))) {
        px_srcline(1445);
        return px_str("BitOr");
    }
    px_srcline(1446);
    if (px_is_truthy(px_eq(_v1942, px_str("BitXor")))) {
        px_srcline(1447);
        return px_str("BitXor");
    }
    px_srcline(1448);
    if (px_is_truthy(px_eq(_v1942, px_str("Shl")))) {
        px_srcline(1449);
        return px_str("Shl");
    }
    px_srcline(1450);
    if (px_is_truthy(px_eq(_v1942, px_str("Shr")))) {
        px_srcline(1451);
        return px_str("Shr");
    }
    px_srcline(1452);
    if (px_is_truthy(px_eq(_v1942, px_str("ShrU")))) {
        px_srcline(1453);
        return px_str("ShrU");
    }
    px_srcline(1454);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_assign_op_name 未知赋值 op: "), px_call(px_get_global("str"), (LXValue[]){_v1942}, 1))}, 1));
px_err_1943:
    if (px_err_1943_proped) return px_err_1943_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt_inner");
    LXValue _v1944 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1945 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1946 = px_uninit();
    LXValue _v1947 = px_uninit();
    LXValue _v1948 = px_uninit();
    LXValue _v1949 = px_uninit();
    LXValue _v1950 = px_uninit();
    LXValue _v1951 = px_uninit();
    LXValue _v1952 = px_uninit();
    LXValue _v1953 = px_uninit();
    LXValue _v1954 = px_uninit();
    LXValue _v1955 = px_uninit();
    LXValue _v1956 = px_uninit();
    LXValue _v1957 = px_uninit();
    LXValue _v1958 = px_uninit();
    LXValue _v1959 = px_uninit();
    LXValue _v1960 = px_uninit();
    LXValue _v1961 = px_uninit();
    LXValue _v1962 = px_uninit();
    LXValue _v1963 = px_uninit();
    LXValue _v1964 = px_uninit();
    LXValue _v1965 = px_uninit();
    LXValue _v1966 = px_uninit();
    LXValue _v1967 = px_uninit();
    LXValue _v1968 = px_uninit();
    LXValue _v1969 = px_uninit();
    LXValue _v1970 = px_uninit();
    LXValue _v1971 = px_uninit();
    LXValue _v1972 = px_uninit();
    LXValue _v1973 = px_uninit();
    LXValue _v1974 = px_uninit();
    LXValue _v1975 = px_uninit();
    LXValue _v1976 = px_uninit();
    LXValue _v1977 = px_uninit();
    LXValue _v1978 = px_uninit();
    LXValue _v1979 = px_uninit();
    LXValue px_err_1980_val = px_null();
    int px_err_1980_proped = 0;
    px_srcline(1457);
    _v1946 = px_index(_v1944, px_int(0LL));
    px_srcline(1458);
    if (px_is_truthy(px_eq(_v1946, px_str("VarDecl")))) {
        px_srcline(1461);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v1944}, 1));
        px_srcline(1463);
        _v1947 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1944, px_int(2LL))}, 1);
        px_srcline(1464);
        if (px_is_truthy(px_eq(px_index(_v1944, px_int(4LL)), px_null()))) {
            px_srcline(1468);
            if (px_is_truthy(px_index(_v1945, px_str("is_top")))) {
                px_srcline(1469);
                _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
                px_srcline(1470);
                (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1945, _v1948}, 2));
                px_srcline(1471);
                _v1949 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1947}, 2);
                px_srcline(1472);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("SETG"), _v1949, _v1948, px_int(0LL)}, 5));
                px_srcline(1473);
                return px_null();
            }
            px_srcline(1474);
            _v1950 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1945, _v1947}, 2);
            px_srcline(1475);
            _v1951 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
            px_srcline(1476);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1945, _v1951}, 2));
            px_srcline(1477);
            (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1945, _v1947, _v1950, _v1951}, 4));
            px_srcline(1478);
            px_index_set(px_index(_v1945, px_str("inited")), _v1947, px_int(1LL));
            px_srcline(1479);
            return px_null();
        }
        px_srcline(1480);
        if (px_is_truthy(px_index(_v1945, px_str("is_top")))) {
            px_srcline(1482);
            _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
            px_srcline(1483);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1944, px_int(4LL)), _v1948, _v1945}, 3));
            px_srcline(1484);
            _v1949 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1947}, 2);
            px_srcline(1485);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("SETG"), _v1949, _v1948, px_int(0LL)}, 5));
            px_srcline(1486);
            return px_null();
        }
        px_srcline(1490);
        _v1952 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1945, _v1947}, 2);
        px_srcline(1491);
        _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
        px_srcline(1492);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1944, px_int(4LL)), _v1948, _v1945}, 3));
        px_srcline(1493);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1945, _v1947, _v1952, _v1948}, 4));
        px_srcline(1495);
        px_index_set(px_index(_v1945, px_str("inited")), _v1947, px_int(1LL));
        px_srcline(1496);
        return px_null();
    }
    px_srcline(1497);
    if (px_is_truthy(px_eq(_v1946, px_str("Assign")))) {
        px_srcline(1500);
        _v1953 = px_index(_v1944, px_int(1LL));
        px_srcline(1501);
        _v1954 = px_index(_v1944, px_int(2LL));
        px_srcline(1502);
        _v1955 = px_index(_v1944, px_int(3LL));
        px_srcline(1503);
        if (px_is_truthy(px_eq(_v1954, px_str("Append")))) {
            px_srcline(1504);
            _v1956 = px_list_n((LXValue[]){_v1955}, 1);
            px_srcline(1505);
            (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){_v1953, px_str("append"), _v1956, px_neg(px_int(1LL)), _v1945}, 5));
            px_srcline(1506);
            return px_null();
        }
        px_srcline(1509);
        (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v1953, _v1954, _v1955}, 3));
        px_srcline(1510);
        _v1957 = px_index(_v1953, px_int(0LL));
        px_srcline(1511);
        if (px_is_truthy(px_eq(_v1957, px_str("Var")))) {
            px_srcline(1512);
            _v1947 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1953, px_int(1LL))}, 1);
            px_srcline(1513);
            if (px_is_truthy(px_method(px_index(_v1945, px_str("smap")), "has", (LXValue[]){_v1947}, 1))) {
                px_srcline(1515);
                if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1945, _v1947}, 2))) {
                    px_srcline(1516);
                    _v1958 = px_index(px_index(_v1945, px_str("smap")), _v1947);
                    px_srcline(1517);
                    if (px_is_truthy(px_eq(_v1954, px_str("Assign")))) {
                        px_srcline(1518);
                        _v1959 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
                        px_srcline(1519);
                        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1955, _v1959, _v1945}, 3));
                        px_srcline(1520);
                        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("CELLSET"), _v1959, _v1958, px_int(0LL)}, 5));
                        px_srcline(1522);
                        px_index_set(px_index(_v1945, px_str("inited")), _v1947, px_int(1LL));
                        px_srcline(1523);
                        return px_null();
                    }
                    px_srcline(1524);
                    _v1960 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
                    px_srcline(1525);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("CELLGET"), _v1960, _v1958, px_int(0LL)}, 5));
                    px_srcline(1527);
                    (void)(px_call(px_get_global("bc_chk_slot"), (LXValue[]){_v1945, _v1947, _v1960}, 3));
                    px_srcline(1528);
                    _v1961 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
                    px_srcline(1529);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1955, _v1961, _v1945}, 3));
                    px_srcline(1530);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1954}, 1)}, 1), _v1960, _v1960, _v1961}, 5));
                    px_srcline(1531);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("CELLSET"), _v1960, _v1958, px_int(0LL)}, 5));
                    px_srcline(1533);
                    px_index_set(px_index(_v1945, px_str("inited")), _v1947, px_int(1LL));
                    px_srcline(1534);
                    return px_null();
                }
                px_srcline(1535);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){px_index(px_index(_v1945, px_str("smap")), _v1947), _v1947, _v1954, _v1955, _v1945}, 5));
                px_srcline(1537);
                px_index_set(px_index(_v1945, px_str("inited")), _v1947, px_int(1LL));
                px_srcline(1538);
                return px_null();
            }
            px_srcline(1539);
            if (px_is_truthy(({ LXValue _t1981 = px_not(px_index(_v1945, px_str("is_top"))); px_is_truthy(_t1981) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v1947}, 2)) : _t1981; }))) {
                px_srcline(1542);
                _v1952 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1945, _v1947}, 2);
                px_srcline(1543);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){_v1952, _v1947, _v1954, _v1955, _v1945}, 5));
                px_srcline(1545);
                px_index_set(px_index(_v1945, px_str("inited")), _v1947, px_int(1LL));
                px_srcline(1546);
                return px_null();
            }
            px_srcline(1548);
            (void)(px_call(px_get_global("bc_assign_global"), (LXValue[]){_v1947, _v1954, _v1955, _v1945}, 4));
            px_srcline(1549);
            return px_null();
        }
        px_srcline(1550);
        if (px_is_truthy(px_eq(_v1957, px_str("Index")))) {
            px_srcline(1551);
            (void)(px_call(px_get_global("bc_assign_index"), (LXValue[]){_v1953, _v1954, _v1955, _v1945}, 4));
            px_srcline(1552);
            return px_null();
        }
        px_srcline(1553);
        if (px_is_truthy(px_eq(_v1957, px_str("Field")))) {
            px_srcline(1554);
            (void)(px_call(px_get_global("bc_assign_field"), (LXValue[]){_v1953, _v1954, _v1955, _v1945}, 4));
            px_srcline(1555);
            return px_null();
        }
        px_srcline(1556);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt Assign 目标未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1953}, 1))}, 1));
    }
    px_srcline(1557);
    if (px_is_truthy(px_eq(_v1946, px_str("Return")))) {
        px_srcline(1558);
        if (px_is_truthy(px_ne(px_index(_v1944, px_int(1LL)), px_null()))) {
            px_srcline(1559);
            _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
            px_srcline(1560);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1944, px_int(1LL)), _v1948, _v1945}, 3));
            px_srcline(1561);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("RET"), _v1948, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1562);
            return px_null();
        }
        px_srcline(1563);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1564);
        return px_null();
    }
    px_srcline(1565);
    if (px_is_truthy(px_eq(_v1946, px_str("ExprStmt")))) {
        px_srcline(1567);
        _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
        px_srcline(1568);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1944, px_int(1LL)), _v1948, _v1945}, 3));
        px_srcline(1569);
        return px_null();
    }
    px_srcline(1570);
    if (px_is_truthy(px_eq(_v1946, px_str("If")))) {
        px_srcline(1571);
        (void)(px_call(px_get_global("bc_emit_if"), (LXValue[]){_v1944, _v1945}, 2));
        px_srcline(1572);
        return px_null();
    }
    px_srcline(1573);
    if (px_is_truthy(px_eq(_v1946, px_str("While")))) {
        px_srcline(1574);
        (void)(px_call(px_get_global("bc_emit_while"), (LXValue[]){_v1944, _v1945}, 2));
        px_srcline(1575);
        return px_null();
    }
    px_srcline(1576);
    if (px_is_truthy(px_eq(_v1946, px_str("For")))) {
        px_srcline(1577);
        (void)(px_call(px_get_global("bc_emit_for"), (LXValue[]){_v1944, _v1945}, 2));
        px_srcline(1578);
        return px_null();
    }
    px_srcline(1579);
    if (px_is_truthy(px_eq(_v1946, px_str("Break")))) {
        px_srcline(1581);
        _v1962 = px_index(_v1945, px_str("loops"));
        px_srcline(1582);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1962}, 1), px_int(0LL)))) {
            px_srcline(1583);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Break 不在循环内")}, 1));
        }
        px_srcline(1584);
        _v1963 = px_index(_v1962, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1962}, 1), px_int(1LL)));
        px_srcline(1585);
        _v1964 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1945, px_str("bc"))}, 1);
        px_srcline(1586);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1587);
        (void)(px_method(px_index(_v1963, px_str("breaks")), "append", (LXValue[]){_v1964}, 1));
        px_srcline(1588);
        return px_null();
    }
    px_srcline(1589);
    if (px_is_truthy(px_eq(_v1946, px_str("Continue")))) {
        px_srcline(1591);
        _v1962 = px_index(_v1945, px_str("loops"));
        px_srcline(1592);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1962}, 1), px_int(0LL)))) {
            px_srcline(1593);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Continue 不在循环内")}, 1));
        }
        px_srcline(1594);
        _v1963 = px_index(_v1962, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1962}, 1), px_int(1LL)));
        px_srcline(1595);
        _v1964 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1945, px_str("bc"))}, 1);
        px_srcline(1596);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1597);
        (void)(px_method(px_index(_v1963, px_str("conts")), "append", (LXValue[]){_v1964}, 1));
        px_srcline(1598);
        return px_null();
    }
    px_srcline(1599);
    if (px_is_truthy(px_eq(_v1946, px_str("Empty")))) {
        px_srcline(1600);
        return px_null();
    }
    px_srcline(1601);
    if (px_is_truthy(px_eq(_v1946, px_str("TypeConst")))) {
        px_srcline(1603);
        return px_null();
    }
    px_srcline(1604);
    if (px_is_truthy(px_eq(_v1946, px_str("ChanDecl")))) {
        px_srcline(1606);
        _v1947 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1944, px_int(1LL))}, 1);
        px_srcline(1607);
        _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
        px_srcline(1608);
        _v1958 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
        px_srcline(1609);
        _v1965 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan")}, 2);
        px_srcline(1610);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("GETG"), _v1958, _v1965, px_int(0LL)}, 5));
        px_srcline(1611);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("CALL"), _v1948, _v1958, px_int(0LL)}, 5));
        px_srcline(1612);
        if (px_is_truthy(px_index(_v1945, px_str("is_top")))) {
            px_srcline(1613);
            _v1949 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1947}, 2);
            px_srcline(1614);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("SETG"), _v1949, _v1948, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1616);
            _v1952 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1945, _v1947}, 2);
            px_srcline(1617);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("MOV"), _v1952, _v1948, px_int(0LL)}, 5));
        }
        px_srcline(1618);
        return px_null();
    }
    px_srcline(1619);
    if (px_is_truthy(px_eq(_v1946, px_str("Send")))) {
        px_srcline(1621);
        _v1966 = px_list_n((LXValue[]){px_index(_v1944, px_int(2LL))}, 1);
        px_srcline(1622);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1944, px_int(1LL)), px_str("send"), _v1966, px_neg(px_int(1LL)), _v1945}, 5));
        px_srcline(1623);
        return px_null();
    }
    px_srcline(1624);
    if (px_is_truthy(px_eq(_v1946, px_str("Recv")))) {
        px_srcline(1626);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1944, px_int(1LL)), px_str("recv"), px_list_n((LXValue[]){}, 0), px_neg(px_int(1LL)), _v1945}, 5));
        px_srcline(1627);
        return px_null();
    }
    px_srcline(1628);
    if (px_is_truthy(px_eq(_v1946, px_str("Spawn")))) {
        px_srcline(1631);
        _v1967 = px_index(_v1944, px_int(1LL));
        px_srcline(1632);
        if (px_is_truthy(({ LXValue _t1982 = px_eq(px_index(_v1967, px_int(0LL)), px_str("Call")); px_is_truthy(_t1982) ? px_eq(px_index(px_index(_v1967, px_int(1LL)), px_int(0LL)), px_str("Var")) : _t1982; }))) {
            px_srcline(1633);
            _v1968 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1967, px_int(1LL)), px_int(1LL))}, 1);
            px_srcline(1634);
            _v1969 = px_index(_v1967, px_int(2LL));
            px_srcline(1635);
            _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
            px_srcline(1636);
            _v1958 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
            px_srcline(1637);
            _v1970 = px_add(px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1969}, 1));
            px_srcline(1638);
            _v1971 = px_add(px_add(_v1958, px_int(1LL)), _v1970);
            px_srcline(1639);
            while (px_is_truthy(px_lt(px_index(_v1945, px_str("next_slot")), _v1971))) {
                px_srcline(1640);
                (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1));
            }
            px_srcline(1641);
            _v1965 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("spawn")}, 2);
            px_srcline(1642);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("GETG"), _v1958, _v1965, px_int(0LL)}, 5));
            px_srcline(1643);
            _v1972 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1968}, 5);
            px_srcline(1644);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("LOADK"), px_add(_v1958, px_int(1LL)), _v1972, px_int(0LL)}, 5));
            px_srcline(1645);
            _v1973 = px_int(0LL);
            px_srcline(1646);
            while (px_is_truthy(px_lt(_v1973, px_call(px_get_global("len"), (LXValue[]){_v1969}, 1)))) {
                px_srcline(1647);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1969, _v1973), px_add(px_add(_v1958, px_int(2LL)), _v1973), _v1945}, 3));
                px_srcline(1648);
                 _v1973 = px_add(_v1973, px_int(1LL));
            }
            px_srcline(1649);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("CALL"), _v1948, _v1958, _v1970}, 5));
            px_srcline(1650);
            return px_null();
        }
        px_srcline(1655);
        _v1974 = px_index(_v1944, px_int(2LL));
        px_srcline(1656);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s213 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1974, px_int(0LL))}, 1)), px_str(":")); LXValue _s214 = px_call(px_get_global("str"), (LXValue[]){px_index(_v1974, px_int(1LL))}, 1); px_add(_s213, _s214); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()），多行匿名函数体见 M118"))}, 1));
        px_srcline(1657);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(1658);
    if (px_is_truthy(px_eq(_v1946, px_str("Select")))) {
        px_srcline(1659);
        (void)(px_call(px_get_global("bc_emit_select"), (LXValue[]){_v1944, _v1945}, 2));
        px_srcline(1660);
        return px_null();
    }
    px_srcline(1661);
    if (px_is_truthy(px_eq(_v1946, px_str("FuncDef")))) {
        px_srcline(1666);
        _v1975 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1944, px_int(1LL))}, 1);
        px_srcline(1667);
        if (px_is_truthy(px_index(_v1945, px_str("is_top")))) {
            px_srcline(1672);
            _v1976 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
            px_srcline(1673);
            (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1944, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1944, px_int(4LL))}, 2), _v1976, _v1945}, 4));
            px_srcline(1674);
            _v1977 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1975}, 2);
            px_srcline(1675);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("SETG"), _v1977, _v1976, px_int(0LL)}, 5));
            px_srcline(1676);
            return px_null();
        }
        px_srcline(1677);
        _v1978 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1945, _v1975}, 2);
        px_srcline(1678);
        _v1979 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
        px_srcline(1679);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1944, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1944, px_int(4LL))}, 2), _v1979, _v1945}, 4));
        px_srcline(1680);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1945, _v1975, _v1978, _v1979}, 4));
        px_srcline(1682);
        px_index_set(px_index(_v1945, px_str("inited")), _v1975, px_int(1LL));
        px_srcline(1683);
        return px_null();
    }
    px_srcline(1684);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1944}, 1))}, 1));
px_err_1980:
    if (px_err_1980_proped) return px_err_1980_val;
    return px_null();
}

static LXValue fn_bc_emit_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_select");
    LXValue _v1983 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1984 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1985 = px_uninit();
    LXValue _v1986 = px_uninit();
    LXValue _v1987 = px_uninit();
    LXValue _v1988 = px_uninit();
    LXValue _v1989 = px_uninit();
    LXValue _v1990 = px_uninit();
    LXValue _v1991 = px_uninit();
    LXValue _v1992 = px_uninit();
    LXValue _v1993 = px_uninit();
    LXValue _v1994 = px_uninit();
    LXValue _v1995 = px_uninit();
    LXValue _v1996 = px_uninit();
    LXValue _v1997 = px_uninit();
    LXValue _v1998 = px_uninit();
    LXValue _v1999 = px_uninit();
    LXValue _v2000 = px_uninit();
    LXValue _v2001 = px_uninit();
    LXValue _v2002 = px_uninit();
    LXValue _v2003 = px_uninit();
    LXValue _v2004 = px_uninit();
    LXValue _v2005 = px_uninit();
    LXValue _v2006 = px_uninit();
    LXValue _v2007 = px_uninit();
    LXValue _v2008 = px_uninit();
    LXValue _v2009 = px_uninit();
    LXValue _v2010 = px_uninit();
    LXValue px_err_2011_val = px_null();
    int px_err_2011_proped = 0;
    px_srcline(1692);
    _v1985 = px_index(_v1983, px_int(1LL));
    px_srcline(1693);
    _v1986 = px_index(_v1983, px_int(2LL));
    px_srcline(1694);
    _v1987 = px_call(px_get_global("len"), (LXValue[]){_v1985}, 1);
    px_srcline(1697);
    _v1988 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1984}, 1);
    px_srcline(1699);
    _v1989 = px_list_n((LXValue[]){}, 0);
    px_srcline(1700);
    _v1990 = px_int(0LL);
    px_srcline(1701);
    while (px_is_truthy(px_lt(_v1990, _v1987))) {
        px_srcline(1702);
        _v1991 = px_index(px_index(_v1985, _v1990), px_int(1LL));
        px_srcline(1704);
        if (px_is_truthy(({ LXValue _t2012 = px_ne(px_index(_v1991, px_int(0LL)), px_str("Call")); px_is_truthy(_t2012) ? _t2012 : px_ne(px_index(px_index(_v1991, px_int(1LL)), px_int(0LL)), px_str("Field")); }))) {
            px_srcline(1705);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit select case 仅支持 ch.recv(): "), px_call(px_get_global("str"), (LXValue[]){_v1991}, 1))}, 1));
        }
        px_srcline(1706);
        _v1992 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1984}, 1);
        px_srcline(1707);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1991, px_int(1LL)), px_int(1LL)), _v1992, _v1984}, 3));
        px_srcline(1708);
        (void)(px_method(_v1989, "append", (LXValue[]){_v1992}, 1));
        px_srcline(1709);
         _v1990 = px_add(_v1990, px_int(1LL));
    }
    px_srcline(1710);
    _v1993 = px_neg(px_int(1LL));
    px_srcline(1711);
    if (px_is_truthy(px_eq(_v1986, px_null()))) {
        px_srcline(1712);
         _v1993 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1);
    }
    px_srcline(1713);
    _v1994 = px_neg(px_int(1LL));
    px_srcline(1714);
    _v1995 = px_list_n((LXValue[]){}, 0);
    px_srcline(1715);
    _v1996 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan_try_recv")}, 2);
    px_srcline(1716);
    _v1997 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(1717);
     _v1990 = px_int(0LL);
    px_srcline(1718);
    while (px_is_truthy(px_lt(_v1990, _v1987))) {
        px_srcline(1719);
        if (px_is_truthy(px_ge(_v1994, px_int(0LL)))) {
            px_srcline(1720);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1984, _v1994, px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1722);
        _v1998 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1984}, 1);
        px_srcline(1723);
        _v1999 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1984}, 1);
        px_srcline(1724);
        _v2000 = px_add(_v1998, px_int(2LL));
        px_srcline(1725);
        while (px_is_truthy(px_lt(px_index(_v1984, px_str("next_slot")), _v2000))) {
            px_srcline(1726);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1984}, 1));
        }
        px_srcline(1727);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("GETG"), _v1998, _v1996, px_int(0LL)}, 5));
        px_srcline(1728);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("MOV"), px_add(_v1998, px_int(1LL)), px_index(_v1989, _v1990), px_int(0LL)}, 5));
        px_srcline(1729);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("CALL"), _v1999, _v1998, px_int(1LL)}, 5));
        px_srcline(1731);
        _v2001 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1984}, 1);
        px_srcline(1732);
        _v2002 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1984}, 1);
        px_srcline(1733);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("LOADK"), _v2002, _v1997, px_int(0LL)}, 5));
        px_srcline(1734);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("EQ"), _v2001, _v1999, _v2002}, 5));
        px_srcline(1735);
        _v2003 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1);
        px_srcline(1736);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("JMPT"), _v2001, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1737);
         _v1994 = _v2003;
        px_srcline(1739);
        _v2004 = px_index(px_index(_v1985, _v1990), px_int(0LL));
        px_srcline(1740);
        if (px_is_truthy(px_ne(_v2004, px_null()))) {
            px_srcline(1741);
            _v2005 = px_call(px_get_global("rust_unescape"), (LXValue[]){_v2004}, 1);
            px_srcline(1742);
            _v2006 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1984, _v2005}, 2);
            px_srcline(1743);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("MOV"), _v2006, _v1999, px_int(0LL)}, 5));
        }
        px_srcline(1744);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(px_index(_v1985, _v1990), px_int(2LL)), _v1984}, 2));
        px_srcline(1745);
        _v2007 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1);
        px_srcline(1746);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1747);
        (void)(px_method(_v1995, "append", (LXValue[]){_v2007}, 1));
        px_srcline(1748);
         _v1990 = px_add(_v1990, px_int(1LL));
    }
    px_srcline(1750);
    if (px_is_truthy(px_ge(_v1994, px_int(0LL)))) {
        px_srcline(1751);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1984, _v1994, px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1)}, 3));
    }
    px_srcline(1752);
    if (px_is_truthy(px_ne(_v1986, px_null()))) {
        px_srcline(1753);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1986, _v1984}, 2));
    }
    else {
        px_srcline(1755);
        _v2008 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1);
        px_srcline(1756);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1984, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1757);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1984, _v2008, _v1993}, 3));
    }
    px_srcline(1758);
    _v2009 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1984, px_str("bc"))}, 1);
    px_srcline(1759);
    _v2010 = px_int(0LL);
    px_srcline(1760);
    while (px_is_truthy(px_lt(_v2010, px_call(px_get_global("len"), (LXValue[]){_v1995}, 1)))) {
        px_srcline(1761);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1984, px_index(_v1995, _v2010), _v2009}, 3));
        px_srcline(1762);
         _v2010 = px_add(_v2010, px_int(1LL));
    }
    px_srcline(1763);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v1984, _v1988}, 2));
px_err_2011:
    if (px_err_2011_proped) return px_err_2011_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt");
    LXValue _v2013 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2014 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2015 = px_uninit();
    LXValue _v2016 = px_uninit();
    LXValue px_err_2017_val = px_null();
    int px_err_2017_proped = 0;
    px_srcline(1766);
    _v2015 = px_int(0LL);
    px_srcline(1767);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v2013}, 1), px_int(0LL)))) {
        px_srcline(1768);
        _v2016 = px_index(_v2013, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2013}, 1), px_int(1LL)));
        px_srcline(1769);
        if (px_is_truthy(({ LXValue _t2019 = ({ LXValue _t2018 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v2016}, 1), px_str("list")); px_is_truthy(_t2018) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v2016}, 1), px_int(1LL)) : _t2018; }); px_is_truthy(_t2019) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v2016, px_int(0LL))}, 1), px_str("int")) : _t2019; }))) {
            px_srcline(1770);
             _v2015 = px_index(_v2016, px_int(0LL));
        }
    }
    px_srcline(1771);
    if (px_is_truthy(px_gt(_v2015, px_int(0LL)))) {
        px_srcline(1772);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2014, px_str("SRCLINE"), px_int(0LL), _v2015, px_int(0LL)}, 5));
    }
    px_srcline(1773);
    (void)(px_call(px_get_global("bc_emit_stmt_inner"), (LXValue[]){_v2013, _v2014}, 2));
px_err_2017:
    if (px_err_2017_proped) return px_err_2017_val;
    return px_null();
}

static LXValue fn_bc_emit_stmts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmts");
    LXValue _v2020 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2021 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2022 = px_uninit();
    LXValue px_err_2023_val = px_null();
    int px_err_2023_proped = 0;
    px_srcline(1775);
    _v2022 = px_int(0LL);
    px_srcline(1776);
    while (px_is_truthy(px_lt(_v2022, px_call(px_get_global("len"), (LXValue[]){_v2020}, 1)))) {
        px_srcline(1777);
        (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){px_index(_v2020, _v2022), _v2021}, 2));
        px_srcline(1778);
         _v2022 = px_add(_v2022, px_int(1LL));
    }
px_err_2023:
    if (px_err_2023_proped) return px_err_2023_val;
    return px_null();
}

static LXValue fn_bc_emit_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_if");
    LXValue _v2024 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2025 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2026 = px_uninit();
    LXValue _v2027 = px_uninit();
    LXValue _v2028 = px_uninit();
    LXValue _v2029 = px_uninit();
    LXValue _v2030 = px_uninit();
    LXValue _v2031 = px_uninit();
    LXValue _v2032 = px_uninit();
    LXValue _v2033 = px_uninit();
    LXValue _v2034 = px_uninit();
    LXValue _v2035 = px_uninit();
    LXValue _v2036 = px_uninit();
    LXValue _v2037 = px_uninit();
    LXValue _v2038 = px_uninit();
    LXValue _v2039 = px_uninit();
    LXValue px_err_2040_val = px_null();
    int px_err_2040_proped = 0;
    px_srcline(1784);
    _v2026 = px_index(_v2024, px_int(1LL));
    px_srcline(1785);
    _v2027 = px_index(_v2024, px_int(2LL));
    px_srcline(1786);
    _v2028 = px_call(px_get_global("len"), (LXValue[]){_v2026}, 1);
    px_srcline(1789);
    _v2029 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2025}, 1);
    px_srcline(1790);
    _v2030 = px_null();
    px_srcline(1791);
    _v2031 = px_list_n((LXValue[]){}, 0);
    px_srcline(1792);
    _v2032 = px_int(0LL);
    px_srcline(1793);
    while (px_is_truthy(px_lt(_v2032, _v2028))) {
        px_srcline(1794);
        _v2033 = px_index(px_index(_v2026, _v2032), px_int(0LL));
        px_srcline(1795);
        _v2034 = px_index(px_index(_v2026, _v2032), px_int(1LL));
        px_srcline(1796);
        (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2025, _v2029}, 2));
        px_srcline(1797);
        _v2035 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2025}, 1);
        px_srcline(1798);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v2033, _v2035, _v2025}, 3));
        px_srcline(1799);
        _v2036 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2025, px_str("bc"))}, 1);
        px_srcline(1800);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2025, px_str("JMPF"), _v2035, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1801);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2034, _v2025}, 2));
        px_srcline(1802);
        if (px_is_truthy(px_eq(_v2030, px_null()))) {
            px_srcline(1803);
             _v2030 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2025}, 1);
        }
        else {
            px_srcline(1805);
             _v2030 = px_call(px_get_global("bc_inited_intersect"), (LXValue[]){_v2030, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2025}, 1)}, 2);
        }
        px_srcline(1806);
        if (px_is_truthy(({ LXValue _t2041 = px_lt(_v2032, px_sub(_v2028, px_int(1LL))); px_is_truthy(_t2041) ? _t2041 : px_ne(_v2027, px_null()); }))) {
            px_srcline(1809);
            _v2037 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2025, px_str("bc"))}, 1);
            px_srcline(1810);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2025, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1811);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2025, _v2036, px_call(px_get_global("len"), (LXValue[]){px_index(_v2025, px_str("bc"))}, 1)}, 3));
            px_srcline(1812);
            (void)(px_method(_v2031, "append", (LXValue[]){_v2037}, 1));
        }
        else {
            px_srcline(1815);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2025, _v2036, px_call(px_get_global("len"), (LXValue[]){px_index(_v2025, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1816);
         _v2032 = px_add(_v2032, px_int(1LL));
    }
    px_srcline(1817);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2025, _v2029}, 2));
    px_srcline(1818);
    if (px_is_truthy(px_ne(_v2027, px_null()))) {
        px_srcline(1819);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2027, _v2025}, 2));
        px_srcline(1820);
         _v2030 = px_call(px_get_global("bc_inited_intersect"), (LXValue[]){_v2030, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2025}, 1)}, 2);
    }
    else {
        px_srcline(1822);
         _v2030 = px_call(px_get_global("bc_inited_intersect"), (LXValue[]){_v2030, _v2029}, 2);
    }
    px_srcline(1823);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2025, _v2030}, 2));
    px_srcline(1824);
    _v2038 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2025, px_str("bc"))}, 1);
    px_srcline(1825);
    _v2039 = px_int(0LL);
    px_srcline(1826);
    while (px_is_truthy(px_lt(_v2039, px_call(px_get_global("len"), (LXValue[]){_v2031}, 1)))) {
        px_srcline(1827);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2025, px_index(_v2031, _v2039), _v2038}, 3));
        px_srcline(1828);
         _v2039 = px_add(_v2039, px_int(1LL));
    }
px_err_2040:
    if (px_err_2040_proped) return px_err_2040_val;
    return px_null();
}

static LXValue fn_bc_emit_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_while");
    LXValue _v2042 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2043 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2044 = px_uninit();
    LXValue _v2045 = px_uninit();
    LXValue _v2046 = px_uninit();
    LXValue _v2047 = px_uninit();
    LXValue _v2048 = px_uninit();
    LXValue _v2049 = px_uninit();
    LXValue _v2050 = px_uninit();
    LXValue _v2051 = px_uninit();
    LXValue _v2052 = px_uninit();
    LXValue _v2053 = px_uninit();
    LXValue px_err_2054_val = px_null();
    int px_err_2054_proped = 0;
    px_srcline(1833);
    _v2044 = px_index(_v2042, px_int(1LL));
    px_srcline(1834);
    _v2045 = px_index(_v2042, px_int(2LL));
    px_srcline(1836);
    _v2046 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2043}, 1);
    px_srcline(1837);
    _v2047 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2043, px_str("bc"))}, 1);
    px_srcline(1838);
    _v2048 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2043}, 1);
    px_srcline(1839);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v2044, _v2048, _v2043}, 3));
    px_srcline(1840);
    _v2049 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2043, px_str("bc"))}, 1);
    px_srcline(1841);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2043, px_str("JMPF"), _v2048, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1842);
    _v2050 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1843);
    px_index_set(_v2050, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1844);
    px_index_set(_v2050, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1845);
    (void)(px_method(px_index(_v2043, px_str("loops")), "push", (LXValue[]){_v2050}, 1));
    px_srcline(1846);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2045, _v2043}, 2));
    px_srcline(1847);
    (void)(px_method(px_index(_v2043, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1848);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2043, _v2046}, 2));
    px_srcline(1849);
    _v2051 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2043, px_str("bc"))}, 1);
    px_srcline(1850);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2043, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1851);
    _v2052 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2043, px_str("bc"))}, 1);
    px_srcline(1852);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2043, _v2051, _v2047}, 3));
    px_srcline(1853);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2043, _v2049, _v2052}, 3));
    px_srcline(1854);
    _v2053 = px_int(0LL);
    px_srcline(1855);
    while (px_is_truthy(px_lt(_v2053, px_call(px_get_global("len"), (LXValue[]){px_index(_v2050, px_str("breaks"))}, 1)))) {
        px_srcline(1856);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2043, px_index(px_index(_v2050, px_str("breaks")), _v2053), _v2052}, 3));
        px_srcline(1857);
         _v2053 = px_add(_v2053, px_int(1LL));
    }
    px_srcline(1858);
     _v2053 = px_int(0LL);
    px_srcline(1859);
    while (px_is_truthy(px_lt(_v2053, px_call(px_get_global("len"), (LXValue[]){px_index(_v2050, px_str("conts"))}, 1)))) {
        px_srcline(1860);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2043, px_index(px_index(_v2050, px_str("conts")), _v2053), _v2047}, 3));
        px_srcline(1861);
         _v2053 = px_add(_v2053, px_int(1LL));
    }
px_err_2054:
    if (px_err_2054_proped) return px_err_2054_val;
    return px_null();
}

static LXValue fn_bc_for_names(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_for_names");
    LXValue _v2055 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2056 = px_uninit();
    LXValue _v2057 = px_uninit();
    LXValue px_err_2058_val = px_null();
    int px_err_2058_proped = 0;
    px_srcline(1870);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v2055}, 1), px_str("string")))) {
        px_srcline(1871);
        return px_list_n((LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v2055}, 1)}, 1);
    }
    px_srcline(1872);
    _v2056 = px_list_n((LXValue[]){}, 0);
    px_srcline(1873);
    _v2057 = px_int(0LL);
    px_srcline(1874);
    while (px_is_truthy(px_lt(_v2057, px_call(px_get_global("len"), (LXValue[]){_v2055}, 1)))) {
        px_srcline(1875);
        (void)(px_method(_v2056, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2055, _v2057)}, 1)}, 1));
        px_srcline(1876);
         _v2057 = px_add(_v2057, px_int(1LL));
    }
    px_srcline(1877);
    return _v2056;
px_err_2058:
    if (px_err_2058_proped) return px_err_2058_val;
    return px_null();
}

static LXValue fn_bc_emit_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_for");
    LXValue _v2059 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2060 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2061 = px_uninit();
    LXValue _v2062 = px_uninit();
    LXValue _v2063 = px_uninit();
    LXValue _v2064 = px_uninit();
    LXValue _v2065 = px_uninit();
    LXValue _v2066 = px_uninit();
    LXValue _v2067 = px_uninit();
    LXValue _v2068 = px_uninit();
    LXValue _v2069 = px_uninit();
    LXValue _v2070 = px_uninit();
    LXValue _v2071 = px_uninit();
    LXValue _v2072 = px_uninit();
    LXValue _v2073 = px_uninit();
    LXValue _v2074 = px_uninit();
    LXValue _v2075 = px_uninit();
    LXValue _v2076 = px_uninit();
    LXValue _v2077 = px_uninit();
    LXValue _v2078 = px_uninit();
    LXValue _v2079 = px_uninit();
    LXValue _v2080 = px_uninit();
    LXValue _v2081 = px_uninit();
    LXValue _v2082 = px_uninit();
    LXValue _v2083 = px_uninit();
    LXValue _v2084 = px_uninit();
    LXValue _v2085 = px_uninit();
    LXValue _v2086 = px_uninit();
    LXValue _v2087 = px_uninit();
    LXValue _v2088 = px_uninit();
    LXValue _v2089 = px_uninit();
    LXValue _v2090 = px_uninit();
    LXValue px_err_2091_val = px_null();
    int px_err_2091_proped = 0;
    px_srcline(1879);
    _v2061 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v2059, px_int(1LL))}, 1);
    px_srcline(1880);
    _v2062 = px_index(_v2059, px_int(2LL));
    px_srcline(1881);
    _v2063 = px_index(_v2059, px_int(3LL));
    px_srcline(1890);
    _v2064 = px_index(_v2060, px_str("is_top"));
    px_srcline(1892);
    _v2065 = px_list_n((LXValue[]){}, 0);
    px_srcline(1893);
    _v2066 = px_int(0LL);
    px_srcline(1894);
    while (px_is_truthy(px_lt(_v2066, px_call(px_get_global("len"), (LXValue[]){_v2061}, 1)))) {
        px_srcline(1895);
        _v2067 = px_neg(px_int(1LL));
        px_srcline(1896);
        if (px_is_truthy(_v2064)) {
            px_srcline(1897);
             _v2067 = px_neg(px_int(1LL));
        }
        else if (px_is_truthy(px_method(px_index(_v2060, px_str("smap")), "has", (LXValue[]){px_index(_v2061, _v2066)}, 1))) {
            px_srcline(1899);
             _v2067 = px_index(px_index(_v2060, px_str("smap")), px_index(_v2061, _v2066));
        }
        else {
            px_srcline(1901);
             _v2067 = px_call(px_get_global("bc_slot"), (LXValue[]){_v2060, px_index(_v2061, _v2066)}, 2);
        }
        px_srcline(1902);
        (void)(px_method(_v2065, "append", (LXValue[]){_v2067}, 1));
        px_srcline(1903);
         _v2066 = px_add(_v2066, px_int(1LL));
    }
    px_srcline(1905);
    _v2068 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
    px_srcline(1906);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v2062, _v2068, _v2060}, 3));
    px_srcline(1908);
    _v2069 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
    px_srcline(1909);
    _v2070 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
    px_srcline(1910);
    while (px_is_truthy(px_lt(px_index(_v2060, px_str("next_slot")), px_add(_v2069, px_int(2LL))))) {
        px_srcline(1911);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1));
    }
    px_srcline(1912);
    _v2071 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("len")}, 2);
    px_srcline(1913);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("GETG"), _v2069, _v2071, px_int(0LL)}, 5));
    px_srcline(1914);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("MOV"), px_add(_v2069, px_int(1LL)), _v2068, px_int(0LL)}, 5));
    px_srcline(1915);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("CALL"), _v2070, _v2069, px_int(1LL)}, 5));
    px_srcline(1917);
    _v2072 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
    px_srcline(1918);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2060, _v2072, px_int(0LL)}, 3));
    px_srcline(1919);
    _v2073 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2060, px_str("bc"))}, 1);
    px_srcline(1921);
    _v2074 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
    px_srcline(1922);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("LT"), _v2074, _v2072, _v2070}, 5));
    px_srcline(1923);
    _v2075 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2060, px_str("bc"))}, 1);
    px_srcline(1924);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("JMPF"), _v2074, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1927);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("ITERLEN"), _v2068, _v2070, px_int(0LL)}, 5));
    px_srcline(1930);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v2061}, 1), px_int(1LL)))) {
        px_srcline(1937);
        if (px_is_truthy(_v2064)) {
            px_srcline(1939);
            _v2076 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
            px_srcline(1940);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("ITERAT"), _v2076, _v2068, _v2072}, 5));
            px_srcline(1941);
            _v2077 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_index(_v2061, px_int(0LL))}, 2);
            px_srcline(1942);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("SETG"), _v2077, _v2076, px_int(0LL)}, 5));
        }
        else if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v2060, px_index(_v2061, px_int(0LL))}, 2))) {
            px_srcline(1944);
            _v2076 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
            px_srcline(1945);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("ITERAT"), _v2076, _v2068, _v2072}, 5));
            px_srcline(1946);
            (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v2060, px_index(_v2061, px_int(0LL)), px_index(_v2065, px_int(0LL)), _v2076}, 4));
        }
        else {
            px_srcline(1948);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("ITERAT"), px_index(_v2065, px_int(0LL)), _v2068, _v2072}, 5));
        }
    }
    else {
        px_srcline(1952);
        _v2078 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
        px_srcline(1953);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("ITERAT"), _v2078, _v2068, _v2072}, 5));
        px_srcline(1954);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("UNPACKCK"), _v2078, px_call(px_get_global("len"), (LXValue[]){_v2061}, 1), px_int(0LL)}, 5));
        px_srcline(1955);
        _v2079 = px_int(0LL);
        px_srcline(1956);
        while (px_is_truthy(px_lt(_v2079, px_call(px_get_global("len"), (LXValue[]){_v2061}, 1)))) {
            px_srcline(1957);
            _v2080 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
            px_srcline(1958);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2060, _v2080, _v2079}, 3));
            px_srcline(1959);
            if (px_is_truthy(_v2064)) {
                px_srcline(1961);
                _v2081 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
                px_srcline(1962);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("INDEX"), _v2081, _v2078, _v2080}, 5));
                px_srcline(1963);
                _v2082 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_index(_v2061, _v2079)}, 2);
                px_srcline(1964);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("SETG"), _v2082, _v2081, px_int(0LL)}, 5));
            }
            else if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v2060, px_index(_v2061, _v2079)}, 2))) {
                px_srcline(1967);
                _v2081 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
                px_srcline(1968);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("INDEX"), _v2081, _v2078, _v2080}, 5));
                px_srcline(1969);
                (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v2060, px_index(_v2061, _v2079), px_index(_v2065, _v2079), _v2081}, 4));
            }
            else {
                px_srcline(1971);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("INDEX"), px_index(_v2065, _v2079), _v2078, _v2080}, 5));
            }
            px_srcline(1972);
             _v2079 = px_add(_v2079, px_int(1LL));
        }
    }
    px_srcline(1973);
    _v2083 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1974);
    px_index_set(_v2083, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1975);
    px_index_set(_v2083, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1976);
    (void)(px_method(px_index(_v2060, px_str("loops")), "push", (LXValue[]){_v2083}, 1));
    px_srcline(1979);
    _v2084 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2060}, 1);
    px_srcline(1980);
    _v2085 = px_int(0LL);
    px_srcline(1981);
    while (px_is_truthy(px_lt(_v2085, px_call(px_get_global("len"), (LXValue[]){_v2061}, 1)))) {
        px_srcline(1982);
        if (px_is_truthy(px_not(_v2064))) {
            px_srcline(1983);
            px_index_set(px_index(_v2060, px_str("inited")), px_index(_v2061, _v2085), px_int(1LL));
        }
        px_srcline(1984);
         _v2085 = px_add(_v2085, px_int(1LL));
    }
    px_srcline(1985);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2063, _v2060}, 2));
    px_srcline(1986);
    (void)(px_method(px_index(_v2060, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1987);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2060, _v2084}, 2));
    px_srcline(1992);
    _v2086 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2060, px_str("bc"))}, 1);
    px_srcline(1993);
    _v2087 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2060}, 1);
    px_srcline(1994);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2060, _v2087, px_int(1LL)}, 3));
    px_srcline(1995);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("ADD"), _v2072, _v2072, _v2087}, 5));
    px_srcline(1996);
    _v2088 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2060, px_str("bc"))}, 1);
    px_srcline(1997);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2060, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1998);
    _v2089 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2060, px_str("bc"))}, 1);
    px_srcline(1999);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2060, _v2088, _v2073}, 3));
    px_srcline(2000);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2060, _v2075, _v2089}, 3));
    px_srcline(2001);
    _v2090 = px_int(0LL);
    px_srcline(2002);
    while (px_is_truthy(px_lt(_v2090, px_call(px_get_global("len"), (LXValue[]){px_index(_v2083, px_str("breaks"))}, 1)))) {
        px_srcline(2003);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2060, px_index(px_index(_v2083, px_str("breaks")), _v2090), _v2089}, 3));
        px_srcline(2004);
         _v2090 = px_add(_v2090, px_int(1LL));
    }
    px_srcline(2005);
     _v2090 = px_int(0LL);
    px_srcline(2006);
    while (px_is_truthy(px_lt(_v2090, px_call(px_get_global("len"), (LXValue[]){px_index(_v2083, px_str("conts"))}, 1)))) {
        px_srcline(2007);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2060, px_index(px_index(_v2083, px_str("conts")), _v2090), _v2086}, 3));
        px_srcline(2008);
         _v2090 = px_add(_v2090, px_int(1LL));
    }
px_err_2091:
    if (px_err_2091_proped) return px_err_2091_val;
    return px_null();
}

static LXValue fn_bc_emit_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_def");
    LXValue _v2092 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2093 = px_uninit();
    LXValue px_err_2094_val = px_null();
    int px_err_2094_proped = 0;
    px_srcline(2012);
    _v2093 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2092, px_int(1LL))}, 1);
    px_srcline(2013);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v2092, _v2093}, 2);
px_err_2094:
    if (px_err_2094_proped) return px_err_2094_val;
    return px_null();
}

static LXValue fn_bc_emit_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_impl_def");
    LXValue _v2095 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2096 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2097 = px_uninit();
    LXValue px_err_2098_val = px_null();
    int px_err_2098_proped = 0;
    px_srcline(2016);
    _v2097 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2095, px_int(1LL))}, 1);
    px_srcline(2017);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v2095, px_add(px_add(_v2096, px_str(".")), _v2097)}, 2);
px_err_2098:
    if (px_err_2098_proped) return px_err_2098_val;
    return px_null();
}

static LXValue fn_bc_emit_func_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_body");
    LXValue _v2099 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2100 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2101 = px_uninit();
    LXValue _v2102 = px_uninit();
    LXValue _v2103 = px_uninit();
    LXValue _v2104 = px_uninit();
    LXValue _v2105 = px_uninit();
    LXValue _v2106 = px_uninit();
    LXValue _v2107 = px_uninit();
    LXValue _v2108 = px_uninit();
    LXValue _v2109 = px_uninit();
    LXValue _v2110 = px_uninit();
    LXValue _v2111 = px_uninit();
    LXValue px_err_2112_val = px_null();
    int px_err_2112_proped = 0;
    px_srcline(2026);
    _v2101 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(2027);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v2101}, 1));
    px_srcline(2028);
    _v2102 = px_index(_v2099, px_int(2LL));
    px_srcline(2033);
    _v2103 = px_call(px_get_global("len"), (LXValue[]){_v2102}, 1);
    px_srcline(2034);
    _v2104 = px_int(0LL);
    px_srcline(2035);
    _v2105 = px_int(0LL);
    px_srcline(2036);
    while (px_is_truthy(px_lt(_v2105, px_call(px_get_global("len"), (LXValue[]){_v2102}, 1)))) {
        px_srcline(2037);
        if (px_is_truthy(px_ne(px_index(px_index(_v2102, _v2105), px_int(3LL)), px_null()))) {
            px_srcline(2038);
            if (px_is_truthy(px_eq(_v2104, px_int(0LL)))) {
                px_srcline(2039);
                 _v2103 = _v2105;
            }
            px_srcline(2040);
             _v2104 = px_add(_v2104, px_int(1LL));
        }
        px_srcline(2041);
         _v2105 = px_add(_v2105, px_int(1LL));
    }
    px_srcline(2042);
    _v2106 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v2100, _v2103}, 2);
    px_srcline(2043);
    px_index_set(_v2106, px_str("ndefault"), _v2104);
    px_srcline(2045);
     _v2105 = px_int(0LL);
    px_srcline(2046);
    while (px_is_truthy(px_lt(_v2105, px_call(px_get_global("len"), (LXValue[]){_v2102}, 1)))) {
        px_srcline(2047);
        px_index_set(px_index(_v2106, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2102, _v2105), px_int(1LL))}, 1), _v2105);
        px_srcline(2049);
        px_index_set(px_index(_v2106, px_str("inited")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2102, _v2105), px_int(1LL))}, 1), px_int(1LL));
        px_srcline(2050);
         _v2105 = px_add(_v2105, px_int(1LL));
    }
    px_srcline(2051);
    if (px_is_truthy(px_lt(px_index(_v2106, px_str("next_slot")), px_call(px_get_global("len"), (LXValue[]){_v2102}, 1)))) {
        px_srcline(2052);
        px_index_set(_v2106, px_str("next_slot"), px_call(px_get_global("len"), (LXValue[]){_v2102}, 1));
    }
    px_srcline(2054);
    _v2107 = px_list_n((LXValue[]){}, 0);
    px_srcline(2055);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v2099, px_int(4LL)), _v2107}, 2));
    px_srcline(2057);
    _v2108 = px_list_n((LXValue[]){}, 0);
    px_srcline(2058);
    (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v2099, px_int(4LL)), _v2108}, 2));
    px_srcline(2059);
    _v2109 = px_int(0LL);
    px_srcline(2060);
    while (px_is_truthy(px_lt(_v2109, px_call(px_get_global("len"), (LXValue[]){_v2107}, 1)))) {
        px_srcline(2061);
        _v2110 = px_index(_v2107, _v2109);
        px_srcline(2062);
        if (px_is_truthy(px_method(px_index(_v2106, px_str("smap")), "has", (LXValue[]){_v2110}, 1))) {
            px_srcline(2063);
             _v2109 = px_add(_v2109, px_int(1LL));
            px_srcline(2064);
            continue;
        }
        px_srcline(2069);
        if (px_is_truthy(({ LXValue _t2113 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v2108, _v2110}, 2)); px_is_truthy(_t2113) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v2110}, 2) : _t2113; }))) {
            px_srcline(2070);
             _v2109 = px_add(_v2109, px_int(1LL));
            px_srcline(2071);
            continue;
        }
        px_srcline(2072);
        (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v2106, _v2110}, 2));
        px_srcline(2073);
        (void)(px_method(px_index(_v2106, px_str("hoist_slots")), "push", (LXValue[]){px_index(px_index(_v2106, px_str("smap")), _v2110)}, 1));
        px_srcline(2074);
         _v2109 = px_add(_v2109, px_int(1LL));
    }
    px_srcline(2078);
    _v2111 = px_int(0LL);
    px_srcline(2079);
    while (px_is_truthy(px_lt(_v2111, px_call(px_get_global("len"), (LXValue[]){px_index(_v2106, px_str("hoist_slots"))}, 1)))) {
        px_srcline(2080);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2106, px_str("UNINIT"), px_index(px_index(_v2106, px_str("hoist_slots")), _v2111), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(2081);
         _v2111 = px_add(_v2111, px_int(1LL));
    }
    px_srcline(2086);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v2106, px_index(_v2099, px_int(4LL))}, 2));
    px_srcline(2088);
    if (px_is_truthy(px_gt(_v2104, px_int(0LL)))) {
        px_srcline(2089);
        (void)(px_call(px_get_global("bc_emit_default_fill"), (LXValue[]){_v2106, _v2102, _v2103}, 3));
    }
    px_srcline(2090);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(_v2099, px_int(4LL)), _v2106}, 2));
    px_srcline(2091);
    px_set_global("cg_immutables", _v2101);
    px_srcline(2092);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2106, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(2093);
    px_index_set(_v2106, px_str("nslots"), px_index(_v2106, px_str("next_slot")));
    px_srcline(2094);
    return _v2106;
px_err_2112:
    if (px_err_2112_proped) return px_err_2112_val;
    return px_null();
}

static LXValue fn_bc_emit_default_fill(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_default_fill");
    LXValue _v2114 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2115 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2116 = (nargs > 2) ? args[2] : px_null();
    LXValue _v2117 = px_uninit();
    LXValue _v2118 = px_uninit();
    LXValue _v2119 = px_uninit();
    LXValue _v2120 = px_uninit();
    LXValue _v2121 = px_uninit();
    LXValue _v2122 = px_uninit();
    LXValue px_err_2123_val = px_null();
    int px_err_2123_proped = 0;
    px_srcline(2100);
    _v2117 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2114}, 1);
    px_srcline(2101);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2114, px_str("NARGS"), _v2117, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(2102);
    _v2118 = _v2116;
    px_srcline(2103);
    while (px_is_truthy(px_lt(_v2118, px_call(px_get_global("len"), (LXValue[]){_v2115}, 1)))) {
        px_srcline(2104);
        if (px_is_truthy(px_ne(px_index(px_index(_v2115, _v2118), px_int(3LL)), px_null()))) {
            px_srcline(2105);
            _v2119 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2114}, 1);
            px_srcline(2106);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2114, _v2119, _v2118}, 3));
            px_srcline(2107);
            _v2120 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2114}, 1);
            px_srcline(2108);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2114, px_str("LT"), _v2120, _v2119, _v2117}, 5));
            px_srcline(2109);
            _v2121 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2114, px_str("bc"))}, 1);
            px_srcline(2110);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2114, px_str("JMPT"), _v2120, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(2113);
            if (px_is_truthy(px_method(px_index(_v2114, px_str("cell")), "has", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2115, _v2118), px_int(1LL))}, 1)}, 1))) {
                px_srcline(2114);
                _v2122 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2114}, 1);
                px_srcline(2115);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2115, _v2118), px_int(3LL)), _v2122, _v2114}, 3));
                px_srcline(2116);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2114, px_str("CELLSET"), _v2122, _v2118, px_int(0LL)}, 5));
            }
            else {
                px_srcline(2118);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2115, _v2118), px_int(3LL)), _v2118, _v2114}, 3));
            }
            px_srcline(2119);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2114, _v2121, px_call(px_get_global("len"), (LXValue[]){px_index(_v2114, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(2120);
         _v2118 = px_add(_v2118, px_int(1LL));
    }
px_err_2123:
    if (px_err_2123_proped) return px_err_2123_val;
    return px_null();
}

static LXValue fn_bc_emit_func_top(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_top");
    LXValue _v2124 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2125 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2126 = (nargs > 2) ? args[2] : px_null();
    LXValue _v2127 = px_uninit();
    LXValue _v2128 = px_uninit();
    LXValue _v2129 = px_uninit();
    LXValue _v2130 = px_uninit();
    LXValue px_err_2131_val = px_null();
    int px_err_2131_proped = 0;
    px_srcline(2125);
    _v2127 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v2125, px_int(0LL)}, 2);
    px_srcline(2126);
    px_index_set(_v2127, px_str("is_top"), px_bool(true));
    px_srcline(2127);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2124, _v2127}, 2));
    px_srcline(2128);
    if (px_is_truthy(_v2126)) {
        px_srcline(2129);
        _v2128 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2127}, 1);
        px_srcline(2130);
        _v2129 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("main")}, 2);
        px_srcline(2131);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2127, px_str("GETG"), _v2128, _v2129, px_int(0LL)}, 5));
        px_srcline(2132);
        _v2130 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2127}, 1);
        px_srcline(2133);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2127, px_str("CALL"), _v2130, _v2128, px_int(0LL)}, 5));
        px_srcline(2134);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2127, px_str("RET"), _v2130, px_int(0LL), px_int(0LL)}, 5));
    }
    else {
        px_srcline(2136);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2127, px_str("HALT"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    }
    px_srcline(2137);
    px_index_set(_v2127, px_str("nslots"), px_index(_v2127, px_str("next_slot")));
    px_srcline(2138);
    return _v2127;
px_err_2131:
    if (px_err_2131_proped) return px_err_2131_val;
    return px_null();
}

static LXValue fn_bc_emit_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_program");
    LXValue _v2132 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2133 = px_uninit();
    LXValue _v2134 = px_uninit();
    LXValue _v2135 = px_uninit();
    LXValue _v2136 = px_uninit();
    LXValue _v2137 = px_uninit();
    LXValue _v2138 = px_uninit();
    LXValue _v2139 = px_uninit();
    LXValue _v2140 = px_uninit();
    LXValue _v2141 = px_uninit();
    LXValue _v2142 = px_uninit();
    LXValue _v2143 = px_uninit();
    LXValue _v2144 = px_uninit();
    LXValue _v2145 = px_uninit();
    LXValue _v2146 = px_uninit();
    LXValue _v2147 = px_uninit();
    LXValue _v2148 = px_uninit();
    LXValue _v2149 = px_uninit();
    LXValue px_err_2150_val = px_null();
    int px_err_2150_proped = 0;
    px_srcline(2142);
    _v2133 = px_index(_v2132, px_int(1LL));
    px_srcline(2143);
    _v2134 = px_call(px_get_global("bc_new_module"), (LXValue[]){px_str("<module>")}, 1);
    px_srcline(2144);
    px_set_global("g_bcm", _v2134);
    px_srcline(2147);
    _v2135 = px_int(0LL);
    px_srcline(2148);
    while (px_is_truthy(px_lt(_v2135, px_call(px_get_global("len"), (LXValue[]){_v2133}, 1)))) {
        px_srcline(2149);
        _v2136 = px_index(_v2133, _v2135);
        px_srcline(2150);
        _v2137 = px_index(_v2136, px_int(0LL));
        px_srcline(2151);
        if (px_is_truthy(px_eq(_v2137, px_str("StructDef")))) {
            px_srcline(2152);
            _v2138 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
            px_srcline(2153);
            px_index_set(_v2138, px_str("name"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2136, px_int(1LL))}, 1));
            px_srcline(2154);
            _v2139 = px_list_n((LXValue[]){}, 0);
            px_srcline(2155);
            _v2140 = px_int(0LL);
            px_srcline(2156);
            while (px_is_truthy(px_lt(_v2140, px_call(px_get_global("len"), (LXValue[]){px_index(_v2136, px_int(2LL))}, 1)))) {
                px_srcline(2157);
                (void)(px_method(_v2139, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v2136, px_int(2LL)), _v2140), px_int(1LL))}, 1)}, 1));
                px_srcline(2158);
                 _v2140 = px_add(_v2140, px_int(1LL));
            }
            px_srcline(2159);
            px_index_set(_v2138, px_str("fnames"), _v2139);
            px_srcline(2160);
            (void)(px_method(px_index(_v2134, px_str("structs")), "push", (LXValue[]){_v2138}, 1));
        }
        else if (px_is_truthy(px_eq(_v2137, px_str("EnumDef")))) {
            px_srcline(2162);
            _v2141 = px_list_n((LXValue[]){}, 0);
            px_srcline(2163);
            _v2142 = px_int(0LL);
            px_srcline(2164);
            while (px_is_truthy(px_lt(_v2142, px_call(px_get_global("len"), (LXValue[]){px_index(_v2136, px_int(2LL))}, 1)))) {
                px_srcline(2165);
                (void)(px_method(_v2141, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v2136, px_int(2LL)), _v2142), px_int(1LL))}, 1)}, 1));
                px_srcline(2166);
                 _v2142 = px_add(_v2142, px_int(1LL));
            }
            px_srcline(2167);
            px_index_set(px_index(_v2134, px_str("enums")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2136, px_int(1LL))}, 1), _v2141);
        }
        px_srcline(2168);
         _v2135 = px_add(_v2135, px_int(1LL));
    }
    px_srcline(2169);
    (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){_v2133}, 1));
    px_srcline(2170);
    _v2143 = px_call(px_get_global("bc_collect_impl_list"), (LXValue[]){_v2133}, 1);
    px_srcline(2177);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2178);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2179);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2180);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2181);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2182);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2183);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2184);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2185);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2186);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(2188);
     _v2135 = px_int(0LL);
    px_srcline(2189);
    while (px_is_truthy(px_lt(_v2135, px_call(px_get_global("len"), (LXValue[]){_v2133}, 1)))) {
        px_srcline(2190);
        _v2136 = px_index(_v2133, _v2135);
        px_srcline(2191);
        _v2137 = px_index(_v2136, px_int(0LL));
        px_srcline(2192);
        if (px_is_truthy(px_eq(_v2137, px_str("FuncDef")))) {
            px_srcline(2193);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2134, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2136, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v2137, px_str("ExternDef")))) {
            px_srcline(2200);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2136, px_int(1LL))}, 1), px_index(_v2136, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v2137, px_str("VarDecl")))) {
            px_srcline(2202);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2134, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2136, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v2137, px_str("Assign")))) {
            px_srcline(2204);
            _v2144 = px_index(_v2136, px_int(1LL));
            px_srcline(2205);
            if (px_is_truthy(px_eq(px_index(_v2144, px_int(0LL)), px_str("Var")))) {
                px_srcline(2206);
                (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2134, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2144, px_int(1LL))}, 1)}, 2));
            }
        }
        px_srcline(2207);
         _v2135 = px_add(_v2135, px_int(1LL));
    }
    px_srcline(2213);
    px_set_global("g_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(2214);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){_v2133, px_get_global("g_topnames")}, 2));
    px_srcline(2217);
     _v2135 = px_int(0LL);
    px_srcline(2218);
    while (px_is_truthy(px_lt(_v2135, px_call(px_get_global("len"), (LXValue[]){_v2143}, 1)))) {
        px_srcline(2219);
        _v2145 = px_index(_v2143, _v2135);
        px_srcline(2220);
        _v2146 = px_call(px_get_global("bc_emit_impl_def"), (LXValue[]){px_index(_v2145, px_int(1LL)), px_index(_v2145, px_int(0LL))}, 2);
        px_srcline(2221);
        (void)(px_method(px_index(_v2134, px_str("funcs")), "push", (LXValue[]){_v2146}, 1));
        px_srcline(2222);
         _v2135 = px_add(_v2135, px_int(1LL));
    }
    px_srcline(2223);
     _v2135 = px_int(0LL);
    px_srcline(2224);
    while (px_is_truthy(px_lt(_v2135, px_call(px_get_global("len"), (LXValue[]){_v2133}, 1)))) {
        px_srcline(2225);
        _v2136 = px_index(_v2133, _v2135);
        px_srcline(2226);
        if (px_is_truthy(px_eq(px_index(_v2136, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(2227);
            _v2146 = px_call(px_get_global("bc_emit_func_def"), (LXValue[]){_v2136}, 1);
            px_srcline(2228);
            (void)(px_method(px_index(_v2134, px_str("funcs")), "push", (LXValue[]){_v2146}, 1));
        }
        px_srcline(2229);
         _v2135 = px_add(_v2135, px_int(1LL));
    }
    px_srcline(2231);
    _v2147 = px_list_n((LXValue[]){}, 0);
    px_srcline(2232);
     _v2135 = px_int(0LL);
    px_srcline(2233);
    while (px_is_truthy(px_lt(_v2135, px_call(px_get_global("len"), (LXValue[]){_v2133}, 1)))) {
        px_srcline(2234);
        _v2136 = px_index(_v2133, _v2135);
        px_srcline(2235);
        _v2137 = px_index(_v2136, px_int(0LL));
        px_srcline(2236);
        if (px_is_truthy(({ LXValue _t2157 = ({ LXValue _t2156 = ({ LXValue _t2155 = ({ LXValue _t2154 = ({ LXValue _t2153 = ({ LXValue _t2152 = ({ LXValue _t2151 = px_ne(_v2137, px_str("FuncDef")); px_is_truthy(_t2151) ? px_ne(_v2137, px_str("StructDef")) : _t2151; }); px_is_truthy(_t2152) ? px_ne(_v2137, px_str("EnumDef")) : _t2152; }); px_is_truthy(_t2153) ? px_ne(_v2137, px_str("TraitDef")) : _t2153; }); px_is_truthy(_t2154) ? px_ne(_v2137, px_str("ImplDef")) : _t2154; }); px_is_truthy(_t2155) ? px_ne(_v2137, px_str("Import")) : _t2155; }); px_is_truthy(_t2156) ? px_ne(_v2137, px_str("ExternDef")) : _t2156; }); px_is_truthy(_t2157) ? px_ne(_v2137, px_str("TypeConst")) : _t2157; }))) {
            px_srcline(2237);
            (void)(px_method(_v2147, "append", (LXValue[]){_v2136}, 1));
        }
        px_srcline(2238);
         _v2135 = px_add(_v2135, px_int(1LL));
    }
    px_srcline(2240);
    _v2148 = px_bool(false);
    px_srcline(2241);
     _v2135 = px_int(0LL);
    px_srcline(2242);
    while (px_is_truthy(px_lt(_v2135, px_call(px_get_global("len"), (LXValue[]){_v2133}, 1)))) {
        px_srcline(2243);
        _v2136 = px_index(_v2133, _v2135);
        px_srcline(2244);
        if (px_is_truthy(({ LXValue _t2158 = px_eq(px_index(_v2136, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t2158) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2136, px_int(1LL))}, 1), px_str("main")) : _t2158; }))) {
            px_srcline(2245);
             _v2148 = px_bool(true);
        }
        px_srcline(2246);
         _v2135 = px_add(_v2135, px_int(1LL));
    }
    px_srcline(2247);
    _v2149 = px_call(px_get_global("bc_emit_func_top"), (LXValue[]){_v2147, px_str("<top>"), _v2148}, 3);
    px_srcline(2248);
    (void)(px_method(px_index(_v2134, px_str("funcs")), "push", (LXValue[]){_v2149}, 1));
    px_srcline(2249);
    px_index_set(_v2134, px_str("top"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v2134, px_str("funcs"))}, 1), px_int(1LL)));
    px_srcline(2250);
    return _v2134;
px_err_2150:
    if (px_err_2150_proped) return px_err_2150_val;
    return px_null();
}

static LXValue fn_bc_dump_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_dump_module");
    LXValue _v2159 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2160 = px_uninit();
    LXValue _v2161 = px_uninit();
    LXValue _v2162 = px_uninit();
    LXValue _v2163 = px_uninit();
    LXValue _v2164 = px_uninit();
    LXValue _v2165 = px_uninit();
    LXValue _v2166 = px_uninit();
    LXValue _v2167 = px_uninit();
    LXValue _v2168 = px_uninit();
    LXValue px_err_2169_val = px_null();
    int px_err_2169_proped = 0;
    px_srcline(2253);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# BCModule "), px_index(_v2159, px_str("name")))}, 1));
    px_srcline(2254);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# K "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2159, px_str("k_pool"))}, 1)}, 1))}, 1));
    px_srcline(2255);
    _v2160 = px_int(0LL);
    px_srcline(2256);
    while (px_is_truthy(px_lt(_v2160, px_call(px_get_global("len"), (LXValue[]){px_index(_v2159, px_str("k_pool"))}, 1)))) {
        px_srcline(2257);
        _v2161 = px_index(px_index(_v2159, px_str("k_pool")), _v2160);
        px_srcline(2258);
        if (px_is_truthy(px_eq(px_index(_v2161, px_str("kind")), px_str("int")))) {
            px_srcline(2259);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s215 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2160}, 1)), px_str(" int ")); LXValue _s216 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2161, px_str("i"))}, 1); px_add(_s215, _s216); })}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2161, px_str("kind")), px_str("float")))) {
            px_srcline(2261);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s217 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2160}, 1)), px_str(" float ")); LXValue _s218 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2161, px_str("f"))}, 1); px_add(_s217, _s218); })}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2161, px_str("kind")), px_str("str")))) {
            px_srcline(2263);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s219 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2160}, 1)), px_str(" str \"")); LXValue _s220 = px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2161, px_str("s"))}, 1); px_add(_s219, _s220); }), px_str("\""))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2161, px_str("kind")), px_str("bool")))) {
            px_srcline(2265);
            _v2162 = px_str("false");
            px_srcline(2266);
            if (px_is_truthy(px_ne(px_index(_v2161, px_str("i")), px_int(0LL)))) {
                px_srcline(2267);
                 _v2162 = px_str("true");
            }
            px_srcline(2268);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2160}, 1)), px_str(" bool ")), _v2162)}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2161, px_str("kind")), px_str("func")))) {
            px_srcline(2270);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s221 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2160}, 1)), px_str(" func ")); LXValue _s222 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2161, px_str("i"))}, 1); px_add(_s221, _s222); })}, 1));
        }
        else {
            px_srcline(2272);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2160}, 1)), px_str(" null"))}, 1));
        }
        px_srcline(2273);
         _v2160 = px_add(_v2160, px_int(1LL));
    }
    px_srcline(2274);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# G "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2159, px_str("globals"))}, 1)}, 1))}, 1));
    px_srcline(2275);
    _v2163 = px_int(0LL);
    px_srcline(2276);
    while (px_is_truthy(px_lt(_v2163, px_call(px_get_global("len"), (LXValue[]){px_index(_v2159, px_str("globals"))}, 1)))) {
        px_srcline(2277);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("G "), px_call(px_get_global("str"), (LXValue[]){_v2163}, 1)), px_str(" ")), px_index(px_index(_v2159, px_str("globals")), _v2163))}, 1));
        px_srcline(2278);
         _v2163 = px_add(_v2163, px_int(1LL));
    }
    px_srcline(2279);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s223 = px_add(px_add(px_str("# funcs "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2159, px_str("funcs"))}, 1)}, 1)), px_str(" top=")); LXValue _s224 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2159, px_str("top"))}, 1); px_add(_s223, _s224); })}, 1));
    px_srcline(2280);
    _v2164 = px_int(0LL);
    px_srcline(2281);
    while (px_is_truthy(px_lt(_v2164, px_call(px_get_global("len"), (LXValue[]){px_index(_v2159, px_str("funcs"))}, 1)))) {
        px_srcline(2282);
        _v2165 = px_index(px_index(_v2159, px_str("funcs")), _v2164);
        px_srcline(2283);
        _v2166 = px_str("");
        px_srcline(2284);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2165, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2285);
             _v2166 = px_add(({ LXValue _s225 = px_add(px_add(px_str(" up="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2165, px_str("upnames"))}, 1)}, 1)), px_str("[")); LXValue _s226 = px_call(px_get_global("join"), (LXValue[]){px_str(","), px_index(_v2165, px_str("upnames"))}, 2); px_add(_s225, _s226); }), px_str("]"));
        }
        px_srcline(2286);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s229 = px_add(({ LXValue _s227 = px_add(px_add(px_add(px_add(px_str("== func "), px_index(_v2165, px_str("name"))), px_str(" arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v2165, px_str("arity"))}, 1)), px_str(" ndefault=")); LXValue _s228 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2165, px_str("ndefault"))}, 1); px_add(_s227, _s228); }), px_str(" nslots=")); LXValue _s230 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2165, px_str("nslots"))}, 1); px_add(_s229, _s230); }), _v2166)}, 1));
        px_srcline(2287);
        _v2167 = px_int(0LL);
        px_srcline(2288);
        while (px_is_truthy(px_lt(_v2167, px_call(px_get_global("len"), (LXValue[]){px_index(_v2165, px_str("bc"))}, 1)))) {
            px_srcline(2289);
            _v2168 = px_index(px_index(_v2165, px_str("bc")), _v2167);
            px_srcline(2290);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s235 = px_add(({ LXValue _s233 = px_add(({ LXValue _s231 = px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v2167}, 1), px_str(":")), px_index(_v2168, px_int(0LL))), px_str(" ")); LXValue _s232 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2168, px_int(2LL))}, 1); px_add(_s231, _s232); }), px_str(" ")); LXValue _s234 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2168, px_int(3LL))}, 1); px_add(_s233, _s234); }), px_str(" ")); LXValue _s236 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2168, px_int(4LL))}, 1); px_add(_s235, _s236); })}, 1));
            px_srcline(2291);
             _v2167 = px_add(_v2167, px_int(1LL));
        }
        px_srcline(2292);
         _v2164 = px_add(_v2164, px_int(1LL));
    }
px_err_2169:
    if (px_err_2169_proped) return px_err_2169_val;
    return px_null();
}

static LXValue fn_bc_emit_c_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_c_program");
    LXValue _v2170 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2171 = px_uninit();
    LXValue _v2172 = px_uninit();
    LXValue _v2173 = px_uninit();
    LXValue _v2174 = px_uninit();
    LXValue _v2175 = px_uninit();
    LXValue _v2176 = px_uninit();
    LXValue _v2177 = px_uninit();
    LXValue _v2178 = px_uninit();
    LXValue _v2179 = px_uninit();
    LXValue _v2180 = px_uninit();
    LXValue _v2181 = px_uninit();
    LXValue _v2182 = px_uninit();
    LXValue _v2183 = px_uninit();
    LXValue _v2184 = px_uninit();
    LXValue _v2185 = px_uninit();
    LXValue _v2186 = px_uninit();
    LXValue _v2187 = px_uninit();
    LXValue px_err_2188_val = px_null();
    int px_err_2188_proped = 0;
    px_srcline(2299);
    _v2171 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2170}, 1);
    px_srcline(2304);
    _v2172 = px_list_n((LXValue[]){px_str("/* 由 bc_emit.px (M89-S3) 自动生成 — 字节码模块（VM 执行） */\n")}, 1);
    px_srcline(2305);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("#include \"runtime.h\"\n#include \"vm.h\"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n")}, 1));
    px_srcline(2306);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const PxBCModule s_mod;\n\n")}, 1));
    px_srcline(2308);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const PxK s_K[] = {\n")}, 1));
    px_srcline(2309);
    _v2173 = px_int(0LL);
    px_srcline(2310);
    while (px_is_truthy(px_lt(_v2173, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("k_pool"))}, 1)))) {
        px_srcline(2311);
        _v2174 = px_index(px_index(_v2171, px_str("k_pool")), _v2173);
        px_srcline(2312);
        if (px_is_truthy(px_eq(px_index(_v2174, px_str("kind")), px_str("int")))) {
            px_srcline(2313);
            (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_INT, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v2174, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2174, px_str("kind")), px_str("float")))) {
            px_srcline(2315);
            (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FLT, 0, "), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v2174, px_str("f"))}, 1)), px_str(", NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2174, px_str("kind")), px_str("str")))) {
            px_srcline(2318);
            if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){px_index(_v2174, px_str("s"))}, 1))) {
                px_srcline(2319);
                (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    PXK_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2174, px_str("s"))}, 1)), px_str("\"),\n"))}, 1));
            }
            else {
                px_srcline(2321);
                (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_STR, 0, 0, \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2174, px_str("s"))}, 1)), px_str("\"},\n"))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v2174, px_str("kind")), px_str("bool")))) {
            px_srcline(2323);
            _v2175 = px_str("0");
            px_srcline(2324);
            if (px_is_truthy(px_ne(px_index(_v2174, px_str("i")), px_int(0LL)))) {
                px_srcline(2325);
                 _v2175 = px_str("1");
            }
            px_srcline(2326);
            (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_BOOL, "), _v2175), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2174, px_str("kind")), px_str("func")))) {
            px_srcline(2328);
            (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FUNC, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v2174, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else {
            px_srcline(2330);
            (void)(px_method(_v2172, "push", (LXValue[]){px_str("    {PXK_NULL, 0, 0, NULL},\n")}, 1));
        }
        px_srcline(2331);
         _v2173 = px_add(_v2173, px_int(1LL));
    }
    px_srcline(2332);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2334);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const char* s_N[] = {\n")}, 1));
    px_srcline(2335);
    _v2176 = px_int(0LL);
    px_srcline(2336);
    while (px_is_truthy(px_lt(_v2176, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("n_pool"))}, 1)))) {
        px_srcline(2337);
        (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2171, px_str("n_pool")), _v2176)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2338);
         _v2176 = px_add(_v2176, px_int(1LL));
    }
    px_srcline(2339);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2341);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const char* s_G[] = {\n")}, 1));
    px_srcline(2342);
    _v2177 = px_int(0LL);
    px_srcline(2343);
    while (px_is_truthy(px_lt(_v2177, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("globals"))}, 1)))) {
        px_srcline(2344);
        (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2171, px_str("globals")), _v2177)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2345);
         _v2177 = px_add(_v2177, px_int(1LL));
    }
    px_srcline(2346);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2348);
    _v2178 = px_int(0LL);
    px_srcline(2349);
    while (px_is_truthy(px_lt(_v2178, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("structs"))}, 1)))) {
        px_srcline(2350);
        _v2179 = px_index(px_index(_v2171, px_str("structs")), _v2178);
        px_srcline(2351);
        (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("static const char* s_st_"), px_call(px_get_global("str"), (LXValue[]){_v2178}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2352);
        _v2180 = px_int(0LL);
        px_srcline(2353);
        while (px_is_truthy(px_lt(_v2180, px_call(px_get_global("len"), (LXValue[]){px_index(_v2179, px_str("fnames"))}, 1)))) {
            px_srcline(2354);
            (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2179, px_str("fnames")), _v2180)}, 1)), px_str("\",\n"))}, 1));
            px_srcline(2355);
             _v2180 = px_add(_v2180, px_int(1LL));
        }
        px_srcline(2356);
        (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2357);
         _v2178 = px_add(_v2178, px_int(1LL));
    }
    px_srcline(2358);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const PxStructDef s_structs[] = {\n")}, 1));
    px_srcline(2359);
     _v2178 = px_int(0LL);
    px_srcline(2360);
    while (px_is_truthy(px_lt(_v2178, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("structs"))}, 1)))) {
        px_srcline(2361);
        _v2179 = px_index(px_index(_v2171, px_str("structs")), _v2178);
        px_srcline(2362);
        (void)(px_method(_v2172, "push", (LXValue[]){px_add(({ LXValue _s239 = px_add(({ LXValue _s237 = px_add(px_add(px_str("    {\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2179, px_str("name"))}, 1)), px_str("\", s_st_")); LXValue _s238 = px_call(px_get_global("str"), (LXValue[]){_v2178}, 1); px_add(_s237, _s238); }), px_str(", ")); LXValue _s240 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2179, px_str("fnames"))}, 1)}, 1); px_add(_s239, _s240); }), px_str("},\n"))}, 1));
        px_srcline(2363);
         _v2178 = px_add(_v2178, px_int(1LL));
    }
    px_srcline(2364);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2366);
    _v2181 = px_int(0LL);
    px_srcline(2367);
    while (px_is_truthy(px_lt(_v2181, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("funcs"))}, 1)))) {
        px_srcline(2368);
        _v2182 = px_index(px_index(_v2171, px_str("funcs")), _v2181);
        px_srcline(2369);
        (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("static const PxInst s_bc_"), px_call(px_get_global("str"), (LXValue[]){_v2181}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2370);
        _v2183 = px_int(0LL);
        px_srcline(2371);
        while (px_is_truthy(px_lt(_v2183, px_call(px_get_global("len"), (LXValue[]){px_index(_v2182, px_str("bc"))}, 1)))) {
            px_srcline(2372);
            _v2184 = px_index(px_index(_v2182, px_str("bc")), _v2183);
            px_srcline(2378);
            (void)(px_method(_v2172, "push", (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), ({ LXValue _s241 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2184, px_int(1LL))}, 1); LXValue _s242 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2184, px_int(2LL))}, 1); LXValue _s243 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2184, px_int(3LL))}, 1); LXValue _s244 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2184, px_int(4LL))}, 1); px_list_n((LXValue[]){px_str("    {PXOP_"), px_index(_v2184, px_int(0LL)), px_str(", "), _s241, px_str(", "), _s242, px_str(", "), _s243, px_str(", "), _s244, px_str("},\n")}, 11); })}, 2)}, 1));
            px_srcline(2379);
             _v2183 = px_add(_v2183, px_int(1LL));
        }
        px_srcline(2380);
        (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2381);
         _v2181 = px_add(_v2181, px_int(1LL));
    }
    px_srcline(2382);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("\n")}, 1));
    px_srcline(2384);
     _v2181 = px_int(0LL);
    px_srcline(2385);
    while (px_is_truthy(px_lt(_v2181, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("funcs"))}, 1)))) {
        px_srcline(2386);
        _v2182 = px_index(px_index(_v2171, px_str("funcs")), _v2181);
        px_srcline(2387);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2182, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2388);
            _v2185 = px_list_n((LXValue[]){}, 0);
            px_srcline(2389);
            _v2186 = px_int(0LL);
            px_srcline(2390);
            while (px_is_truthy(px_lt(_v2186, px_call(px_get_global("len"), (LXValue[]){px_index(_v2182, px_str("upnames"))}, 1)))) {
                px_srcline(2391);
                (void)(px_method(_v2185, "push", (LXValue[]){px_add(px_add(px_str("\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2182, px_str("upnames")), _v2186)}, 1)), px_str("\""))}, 1));
                px_srcline(2392);
                 _v2186 = px_add(_v2186, px_int(1LL));
            }
            px_srcline(2393);
            (void)(px_method(_v2172, "push", (LXValue[]){px_add(({ LXValue _s245 = px_add(px_add(px_str("static const char* s_up_"), px_call(px_get_global("str"), (LXValue[]){_v2181}, 1)), px_str("[] = {")); LXValue _s246 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v2185}, 2); px_add(_s245, _s246); }), px_str("};\n"))}, 1));
        }
        px_srcline(2394);
         _v2181 = px_add(_v2181, px_int(1LL));
    }
    px_srcline(2396);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const PxVMFunc s_funcs[] = {\n")}, 1));
    px_srcline(2397);
     _v2181 = px_int(0LL);
    px_srcline(2398);
    while (px_is_truthy(px_lt(_v2181, px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("funcs"))}, 1)))) {
        px_srcline(2399);
        _v2182 = px_index(px_index(_v2171, px_str("funcs")), _v2181);
        px_srcline(2400);
        _v2187 = px_str(".upvals=NULL");
        px_srcline(2401);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2182, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2402);
             _v2187 = ({ LXValue _s247 = px_add(px_add(px_str(".upvals=s_up_"), px_call(px_get_global("str"), (LXValue[]){_v2181}, 1)), px_str(", .nup=")); LXValue _s248 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2182, px_str("upnames"))}, 1)}, 1); px_add(_s247, _s248); });
        }
        px_srcline(2403);
        (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_add(({ LXValue _s257 = px_add(({ LXValue _s255 = px_add(({ LXValue _s253 = px_add(({ LXValue _s251 = px_add(({ LXValue _s249 = px_add(px_add(px_str("    {.name=\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2182, px_str("name"))}, 1)), px_str("\", .arity=")); LXValue _s250 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2182, px_str("arity"))}, 1); px_add(_s249, _s250); }), px_str(", .ndefault=")); LXValue _s252 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2182, px_str("ndefault"))}, 1); px_add(_s251, _s252); }), px_str(", .nslots=")); LXValue _s254 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2182, px_str("nslots"))}, 1); px_add(_s253, _s254); }), px_str(", .bc=s_bc_")); LXValue _s256 = px_call(px_get_global("str"), (LXValue[]){_v2181}, 1); px_add(_s255, _s256); }), px_str(", .nbc=")); LXValue _s258 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2182, px_str("bc"))}, 1)}, 1); px_add(_s257, _s258); }), px_str(", .mod=&s_mod, ")), _v2187), px_str("},\n"))}, 1));
        px_srcline(2404);
         _v2181 = px_add(_v2181, px_int(1LL));
    }
    px_srcline(2405);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2407);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("static const PxBCModule s_mod = {\n")}, 1));
    px_srcline(2408);
    (void)(px_method(_v2172, "push", (LXValue[]){px_add(({ LXValue _s259 = px_add(px_add(px_str("    .name=\"<module>\", .K=s_K, .nK="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("k_pool"))}, 1)}, 1)), px_str(", .N=s_N, .nN=")); LXValue _s260 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("n_pool"))}, 1)}, 1); px_add(_s259, _s260); }), px_str(",\n"))}, 1));
    px_srcline(2409);
    (void)(px_method(_v2172, "push", (LXValue[]){px_add(({ LXValue _s263 = px_add(({ LXValue _s261 = px_add(px_add(px_str("    .G=s_G, .nG="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("globals"))}, 1)}, 1)), px_str(", .funcs=s_funcs, .nfuncs=")); LXValue _s262 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("funcs"))}, 1)}, 1); px_add(_s261, _s262); }), px_str(", .top_idx=")); LXValue _s264 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2171, px_str("top"))}, 1); px_add(_s263, _s264); }), px_str(",\n"))}, 1));
    px_srcline(2410);
    (void)(px_method(_v2172, "push", (LXValue[]){px_add(px_add(px_str("    .structs=s_structs, .nstructs="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2171, px_str("structs"))}, 1)}, 1)), px_str(",\n"))}, 1));
    px_srcline(2411);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2413);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("int main(int argc, char** argv) {\n")}, 1));
    px_srcline(2414);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    px_args_init(argc, argv);\n")}, 1));
    px_srcline(2415);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    px_register_builtins();\n")}, 1));
    px_srcline(2416);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    px_gc_set_precise(1);   // M92-S2d：VM 轨产物默认 precise 精确根面（退役保守栈扫描；C 轨逃生舱产物不插此调用 → conservative）\n")}, 1));
    px_srcline(2417);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    LXValue _r = px_vm_run_module(px_vm_state(), &s_mod);\n")}, 1));
    px_srcline(2418);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    int _code = 0;\n")}, 1));
    px_srcline(2419);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    if (px_is_result(_r)) {\n")}, 1));
    px_srcline(2420);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        if (!px_result_ok(_r)) {\n")}, 1));
    px_srcline(2421);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("            fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n")}, 1));
    px_srcline(2422);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("            _code = 1;\n")}, 1));
    px_srcline(2423);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        } else {\n")}, 1));
    px_srcline(2424);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("            LXValue _uv = px_result_unwrap(_r);\n")}, 1));
    px_srcline(2425);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("            if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n")}, 1));
    px_srcline(2426);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2427);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    } else if (_r.type == PX_INT) {\n")}, 1));
    px_srcline(2428);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        _code = (int)_r.as.i;\n")}, 1));
    px_srcline(2429);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2430);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    if (getenv(\"PX_BC_DUMP\")) {\n")}, 1));
    px_srcline(2431);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        int _i;\n")}, 1));
    px_srcline(2432);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        for (_i = 0; _i < (int)s_mod.nG; _i++) {\n")}, 1));
    px_srcline(2433);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("            LXValue _v = px_get_global(s_mod.G[_i]);\n")}, 1));
    px_srcline(2434);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("            if (_v.type != PX_FUNC && _v.type != PX_NATIVE)\n")}, 1));
    px_srcline(2435);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("                printf(\"%s=%s\\n\", s_mod.G[_i], px_to_string(_v));\n")}, 1));
    px_srcline(2436);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2437);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2438);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("    return px_exit_code_final(_code);   // M120（qg-issue 76 E1）：协程隔离错误 → 退出码非 0\n")}, 1));
    px_srcline(2439);
    (void)(px_method(_v2172, "push", (LXValue[]){px_str("}\n")}, 1));
    px_srcline(2440);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v2172}, 2);
px_err_2188:
    if (px_err_2188_proped) return px_err_2188_val;
    return px_null();
}

static LXValue fn_bc_basename(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_basename");
    LXValue _v2189 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2190 = px_uninit();
    LXValue _v2191 = px_uninit();
    LXValue px_err_2192_val = px_null();
    int px_err_2192_proped = 0;
    px_srcline(74);
    _v2190 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2189}, 1), px_int(1LL));
    px_srcline(75);
    while (px_is_truthy(px_ge(_v2190, px_int(0LL)))) {
        px_srcline(76);
        if (px_is_truthy(px_eq(px_index(_v2189, _v2190), px_str("/")))) {
            px_srcline(77);
            _v2191 = px_slice(_v2189, px_add(_v2190, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v2189}, 1), px_null());
            px_srcline(78);
            if (px_is_truthy(({ LXValue _t2193 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2191}, 1), px_int(3LL)); px_is_truthy(_t2193) ? px_eq(({ LXValue _s265 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2191}, 1), px_int(3LL)); LXValue _s266 = px_call(px_get_global("len"), (LXValue[]){_v2191}, 1); px_slice(_v2191, _s265, _s266, px_null()); }), px_str(".px")) : _t2193; }))) {
                px_srcline(79);
                return px_slice(_v2191, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2191}, 1), px_int(3LL)), px_null());
            }
            px_srcline(80);
            return _v2191;
        }
        px_srcline(81);
         _v2190 = px_sub(_v2190, px_int(1LL));
    }
    px_srcline(82);
    if (px_is_truthy(({ LXValue _t2194 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2189}, 1), px_int(3LL)); px_is_truthy(_t2194) ? px_eq(({ LXValue _s267 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2189}, 1), px_int(3LL)); LXValue _s268 = px_call(px_get_global("len"), (LXValue[]){_v2189}, 1); px_slice(_v2189, _s267, _s268, px_null()); }), px_str(".px")) : _t2194; }))) {
        px_srcline(83);
        return px_slice(_v2189, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2189}, 1), px_int(3LL)), px_null());
    }
    px_srcline(84);
    return _v2189;
px_err_2192:
    if (px_err_2192_proped) return px_err_2192_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v2195 = px_uninit();
    LXValue _v2196 = px_uninit();
    LXValue _v2197 = px_uninit();
    LXValue _v2198 = px_uninit();
    LXValue _v2199 = px_uninit();
    LXValue _v2200 = px_uninit();
    LXValue _v2201 = px_uninit();
    LXValue _v2202 = px_uninit();
    LXValue _v2203 = px_uninit();
    LXValue _v2204 = px_uninit();
    LXValue _v2205 = px_uninit();
    LXValue px_err_2206_val = px_null();
    int px_err_2206_proped = 0;
    px_srcline(87);
    _v2195 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    px_srcline(89);
    if (px_is_truthy(({ LXValue _t2208 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v2195}, 1), px_int(2LL)); px_is_truthy(_t2208) ? ({ LXValue _t2207 = px_eq(px_index(_v2195, px_int(1LL)), px_str("--version")); px_is_truthy(_t2207) ? _t2207 : px_eq(px_index(_v2195, px_int(1LL)), px_str("-v")); }) : _t2208; }))) {
        px_srcline(90);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        px_srcline(91);
        return px_int(0LL);
    }
    px_srcline(94);
    _v2196 = px_bool(false);
    px_srcline(95);
    _v2197 = px_bool(false);
    px_srcline(96);
    _v2198 = px_int(1LL);
    px_srcline(97);
    while (px_is_truthy(px_lt(_v2198, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2195}, 1), px_int(1LL))))) {
        px_srcline(98);
        if (px_is_truthy(({ LXValue _t2209 = px_eq(px_index(_v2195, _v2198), px_str("bc")); px_is_truthy(_t2209) ? _t2209 : px_eq(px_index(_v2195, _v2198), px_str("--emit-c")); }))) {
            px_srcline(99);
             _v2196 = px_bool(true);
        }
        px_srcline(100);
        if (px_is_truthy(px_eq(px_index(_v2195, _v2198), px_str("--emit-c")))) {
            px_srcline(101);
             _v2197 = px_bool(true);
        }
        px_srcline(102);
         _v2198 = px_add(_v2198, px_int(1LL));
    }
    px_srcline(104);
    _v2199 = px_index(_v2195, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2195}, 1), px_int(1LL)));
    px_srcline(105);
    _v2200 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v2199}, 1);
    px_srcline(106);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v2199}, 1)}, 1));
    px_srcline(107);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(108);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(109);
    _v2201 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(110);
    _v2202 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v2201, _v2200}, 2);
    px_srcline(111);
    if (px_is_truthy(_v2196)) {
        px_srcline(112);
        if (px_is_truthy(_v2197)) {
            px_srcline(113);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("bc_emit_c_program"), (LXValue[]){_v2202}, 1)}, 1));
            px_srcline(114);
            return px_int(0LL);
        }
        px_srcline(115);
        _v2203 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2202}, 1);
        px_srcline(116);
        px_index_set(_v2203, px_str("name"), px_call(px_get_global("bc_basename"), (LXValue[]){_v2199}, 1));
        px_srcline(117);
        (void)(px_call(px_get_global("bc_dump_module"), (LXValue[]){_v2203}, 1));
        px_srcline(118);
        return px_int(0LL);
    }
    px_srcline(119);
    _v2204 = px_call(px_get_global("cg_generate"), (LXValue[]){_v2202}, 1);
    px_srcline(121);
    _v2205 = px_call(px_get_global("len"), (LXValue[]){_v2204}, 1);
    px_srcline(122);
    if (px_is_truthy(({ LXValue _t2210 = px_gt(_v2205, px_int(0LL)); px_is_truthy(_t2210) ? px_eq(px_index(_v2204, px_sub(_v2205, px_int(1LL))), px_str("\n")) : _t2210; }))) {
        px_srcline(123);
         _v2204 = px_slice(_v2204, px_int(0LL), px_sub(_v2205, px_int(1LL)), px_null());
    }
    px_srcline(124);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v2204}, 1));
px_err_2206:
    if (px_err_2206_proped) return px_err_2206_val;
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
    px_set_global("cg_inited_new", px_func("cg_inited_new", fn_cg_inited_new, NULL));
    px_set_global("cg_inited_add", px_func("cg_inited_add", fn_cg_inited_add, NULL));
    px_set_global("cg_inited_copy", px_func("cg_inited_copy", fn_cg_inited_copy, NULL));
    px_set_global("cg_inited_restore", px_func("cg_inited_restore", fn_cg_inited_restore, NULL));
    px_set_global("cg_inited_intersect", px_func("cg_inited_intersect", fn_cg_inited_intersect, NULL));
    px_set_global("cg_load_ck", px_func("cg_load_ck", fn_cg_load_ck, NULL));
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
    px_set_global("bc_inited_copy", px_func("bc_inited_copy", fn_bc_inited_copy, NULL));
    px_set_global("bc_inited_restore", px_func("bc_inited_restore", fn_bc_inited_restore, NULL));
    px_set_global("bc_inited_intersect", px_func("bc_inited_intersect", fn_bc_inited_intersect, NULL));
    px_set_global("bc_inited_mark", px_func("bc_inited_mark", fn_bc_inited_mark, NULL));
    px_set_global("bc_slot", px_func("bc_slot", fn_bc_slot, NULL));
    px_set_global("bc_tmp", px_func("bc_tmp", fn_bc_tmp, NULL));
    px_set_global("bc_cell_has", px_func("bc_cell_has", fn_bc_cell_has, NULL));
    px_set_global("bc_cell_mark", px_func("bc_cell_mark", fn_bc_cell_mark, NULL));
    px_set_global("bc_load_var", px_func("bc_load_var", fn_bc_load_var, NULL));
    px_set_global("bc_load_ck", px_func("bc_load_ck", fn_bc_load_ck, NULL));
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
    px_set_global("bc_chk_slot", px_func("bc_chk_slot", fn_bc_chk_slot, NULL));
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
    px_srcline(75);
    px_set_global("cg_inited", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; }));
    px_srcline(76);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(77);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(78);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(79);
    px_set_global("cg_seq_uid", px_int(0LL));
    px_srcline(80);
    px_set_global("cg_iter_uid", px_int(0LL));
    px_srcline(81);
    px_set_global("cg_unpack_uid", px_int(0LL));
    px_srcline(82);
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
