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
    LXValue _v356 = (nargs > 1) ? args[1] : px_null();
    LXValue _v357 = px_uninit();
    LXValue px_err_358_val = px_null();
    int px_err_358_proped = 0;
    px_srcline(172);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(173);
        _v357 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(174);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(175);
        return _v357;
    }
    px_srcline(183);
    if (px_is_truthy(({ LXValue _t360 = px_call(px_get_global("is_name_kind"), (LXValue[]){px_call(px_get_global("pk"), (LXValue[]){}, 0)}, 1); px_is_truthy(_t360) ? ({ LXValue _t359 = _v356; px_is_truthy(_t359) ? _t359 : px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("self")); }) : _t360; }))) {
        px_srcline(184);
        _v357 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(185);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(186);
        return _v357;
    }
    px_srcline(187);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_add(px_add(px_str("期望"), _v355), px_str("，实际得到 ")), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
px_err_358:
    if (px_err_358_proped) return px_err_358_val;
    return px_null();
}

static LXValue fn_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("perr");
    LXValue _v361 = (nargs > 0) ? args[0] : px_null();
    LXValue _v362 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_363_val = px_null();
    int px_err_363_proped = 0;
    px_srcline(191);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_add(({ LXValue _s77 = px_add(px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pline"), (LXValue[]){}, 0)}, 1), px_str(":")); LXValue _s78 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("pcol"), (LXValue[]){}, 0)}, 1); px_add(_s77, _s78); }), px_str(": 语法错误 ")), _v361), px_str(": ")), _v362)}, 1));
    px_srcline(192);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_363:
    if (px_err_363_proped) return px_err_363_val;
    return px_null();
}

static LXValue fn_skip_newlines(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines");
    LXValue px_err_364_val = px_null();
    int px_err_364_proped = 0;
    px_srcline(194);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(195);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_364:
    if (px_err_364_proped) return px_err_364_val;
    return px_null();
}

static LXValue fn_skip_brace_indents(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_brace_indents");
    LXValue px_err_365_val = px_null();
    int px_err_365_proped = 0;
    px_srcline(197);
    while (px_is_truthy(({ LXValue _t366 = px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); px_is_truthy(_t366) ? _t366 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(198);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_365:
    if (px_err_365_proped) return px_err_365_val;
    return px_null();
}

static LXValue fn_skip_expr_ws(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_expr_ws");
    LXValue px_err_367_val = px_null();
    int px_err_367_proped = 0;
    px_srcline(200);
    while (px_is_truthy(({ LXValue _t369 = ({ LXValue _t368 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t368) ? _t368 : px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1); }); px_is_truthy(_t369) ? _t369 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }))) {
        px_srcline(201);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_367:
    if (px_err_367_proped) return px_err_367_val;
    return px_null();
}

static LXValue fn_skip_newlines_in_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("skip_newlines_in_block");
    LXValue px_err_370_val = px_null();
    int px_err_370_proped = 0;
    px_srcline(203);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
        px_srcline(204);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    }
px_err_370:
    if (px_err_370_proped) return px_err_370_val;
    return px_null();
}

static LXValue fn_chk_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chk_op");
    LXValue _v371 = (nargs > 0) ? args[0] : px_null();
    LXValue _v372 = px_uninit();
    LXValue px_err_373_val = px_null();
    int px_err_373_proped = 0;
    px_srcline(216);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){_v371}, 1))) {
        px_srcline(217);
        return px_bool(true);
    }
    px_srcline(218);
    if (px_is_truthy(px_le(px_get_global("p_brack"), px_int(0LL)))) {
        px_srcline(219);
        return px_bool(false);
    }
    px_srcline(220);
    if (px_is_truthy(({ LXValue _t375 = ({ LXValue _t374 = px_eq(_v371, px_str("-")); px_is_truthy(_t374) ? _t374 : px_eq(_v371, px_str("~")); }); px_is_truthy(_t375) ? _t375 : px_eq(_v371, px_str("not")); }))) {
        px_srcline(221);
        return px_bool(false);
    }
    px_srcline(222);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)))) {
        px_srcline(223);
        return px_bool(false);
    }
    px_srcline(224);
    _v372 = px_get_global("p_pos");
    px_srcline(225);
    while (px_is_truthy(({ LXValue _t376 = px_lt(_v372, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)); px_is_truthy(_t376) ? px_eq(px_index(px_index(px_get_global("p_toks"), _v372), px_int(0LL)), px_str("换行")) : _t376; }))) {
        px_srcline(226);
         _v372 = px_add(_v372, px_int(1LL));
    }
    px_srcline(227);
    if (px_is_truthy(({ LXValue _t377 = px_lt(_v372, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)); px_is_truthy(_t377) ? px_eq(px_index(px_index(px_get_global("p_toks"), _v372), px_int(0LL)), _v371) : _t377; }))) {
        px_srcline(228);
        px_set_global("p_pos", _v372);
        px_srcline(229);
        return px_bool(true);
    }
    px_srcline(230);
    return px_bool(false);
px_err_373:
    if (px_err_373_proped) return px_err_373_val;
    return px_null();
}

static LXValue fn_node_pos(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("node_pos");
    LXValue _v378 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_379_val = px_null();
    int px_err_379_proped = 0;
    px_srcline(232);
    return px_index(_v378, px_sub(px_call(px_get_global("len"), (LXValue[]){_v378}, 1), px_int(1LL)));
px_err_379:
    if (px_err_379_proped) return px_err_379_val;
    return px_null();
}

static LXValue fn_qstr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("qstr");
    LXValue _v380 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_381_val = px_null();
    int px_err_381_proped = 0;
    px_srcline(234);
    return px_call(px_get_global("rust_str_debug"), (LXValue[]){_v380}, 1);
px_err_381:
    if (px_err_381_proped) return px_err_381_val;
    return px_null();
}

static LXValue fn_parse_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_program");
    LXValue _v382 = px_uninit();
    LXValue px_err_383_val = px_null();
    int px_err_383_proped = 0;
    px_srcline(237);
    _v382 = px_list_n((LXValue[]){}, 0);
    px_srcline(238);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(239);
    while (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)))) {
        px_srcline(240);
        (void)(px_method(_v382, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(241);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    }
    px_srcline(242);
    return px_list_n((LXValue[]){px_str("Program"), _v382}, 2);
px_err_383:
    if (px_err_383_proped) return px_err_383_val;
    return px_null();
}

static LXValue fn_parse_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_stmt");
    LXValue _v384 = px_uninit();
    LXValue _v385 = px_uninit();
    LXValue _v386 = px_uninit();
    LXValue _v387 = px_uninit();
    LXValue px_err_388_val = px_null();
    int px_err_388_proped = 0;
    px_srcline(245);
    _v384 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(246);
    if (px_is_truthy(px_eq(_v384, px_str("let")))) {
        px_srcline(247);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Let")}, 1);
    }
    px_srcline(248);
    if (px_is_truthy(px_eq(_v384, px_str("var")))) {
        px_srcline(249);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Var")}, 1);
    }
    px_srcline(250);
    if (px_is_truthy(px_eq(_v384, px_str("const")))) {
        px_srcline(251);
        return px_call(px_get_global("parse_var_decl"), (LXValue[]){px_str("Const")}, 1);
    }
    px_srcline(252);
    if (px_is_truthy(px_eq(_v384, px_str("if")))) {
        px_srcline(253);
        return px_call(px_get_global("parse_if"), (LXValue[]){}, 0);
    }
    px_srcline(254);
    if (px_is_truthy(px_eq(_v384, px_str("for")))) {
        px_srcline(255);
        return px_call(px_get_global("parse_for"), (LXValue[]){}, 0);
    }
    px_srcline(256);
    if (px_is_truthy(px_eq(_v384, px_str("while")))) {
        px_srcline(257);
        return px_call(px_get_global("parse_while"), (LXValue[]){}, 0);
    }
    px_srcline(258);
    if (px_is_truthy(px_eq(_v384, px_str("def")))) {
        px_srcline(259);
        return px_call(px_get_global("parse_func_def"), (LXValue[]){}, 0);
    }
    px_srcline(260);
    if (px_is_truthy(px_eq(_v384, px_str("extern")))) {
        px_srcline(261);
        return px_call(px_get_global("parse_extern_def"), (LXValue[]){}, 0);
    }
    px_srcline(262);
    if (px_is_truthy(px_eq(_v384, px_str("struct")))) {
        px_srcline(263);
        return px_call(px_get_global("parse_struct_def"), (LXValue[]){}, 0);
    }
    px_srcline(264);
    if (px_is_truthy(px_eq(_v384, px_str("enum")))) {
        px_srcline(265);
        return px_call(px_get_global("parse_enum_def"), (LXValue[]){}, 0);
    }
    px_srcline(266);
    if (px_is_truthy(({ LXValue _t389 = px_eq(_v384, px_str("标识符")); px_is_truthy(_t389) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("type")) : _t389; }))) {
        px_srcline(269);
        if (px_is_truthy(({ LXValue _t390 = px_call(px_get_global("chk2"), (LXValue[]){px_str("标识符")}, 1); px_is_truthy(_t390) ? px_call(px_get_global("chk3"), (LXValue[]){px_str("const")}, 1) : _t390; }))) {
            px_srcline(270);
            return px_call(px_get_global("parse_type_const"), (LXValue[]){}, 0);
        }
        px_srcline(271);
        return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
    }
    px_srcline(272);
    if (px_is_truthy(px_eq(_v384, px_str("trait")))) {
        px_srcline(273);
        return px_call(px_get_global("parse_trait_def"), (LXValue[]){}, 0);
    }
    px_srcline(274);
    if (px_is_truthy(px_eq(_v384, px_str("impl")))) {
        px_srcline(275);
        return px_call(px_get_global("parse_impl_def"), (LXValue[]){}, 0);
    }
    px_srcline(276);
    if (px_is_truthy(px_eq(_v384, px_str("import")))) {
        px_srcline(277);
        return px_call(px_get_global("parse_import"), (LXValue[]){}, 0);
    }
    px_srcline(278);
    if (px_is_truthy(px_eq(_v384, px_str("from")))) {
        px_srcline(279);
        return px_call(px_get_global("parse_import_from"), (LXValue[]){}, 0);
    }
    px_srcline(280);
    if (px_is_truthy(px_eq(_v384, px_str("return")))) {
        px_srcline(281);
        _v385 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(282);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(283);
        if (px_is_truthy(({ LXValue _t392 = ({ LXValue _t391 = px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1); px_is_truthy(_t391) ? _t391 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t392) ? _t392 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(284);
            return px_list_n((LXValue[]){px_str("Return"), px_null(), _v385}, 3);
        }
        px_srcline(285);
        _v386 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(286);
        return px_list_n((LXValue[]){px_str("Return"), _v386, _v385}, 3);
    }
    px_srcline(287);
    if (px_is_truthy(px_eq(_v384, px_str("break")))) {
        px_srcline(288);
        _v385 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(289);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(290);
        return px_list_n((LXValue[]){px_str("Break"), _v385}, 2);
    }
    px_srcline(291);
    if (px_is_truthy(px_eq(_v384, px_str("continue")))) {
        px_srcline(292);
        _v385 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(293);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(294);
        return px_list_n((LXValue[]){px_str("Continue"), _v385}, 2);
    }
    px_srcline(295);
    if (px_is_truthy(px_eq(_v384, px_str("spawn")))) {
        px_srcline(296);
        _v385 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(297);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(298);
        _v387 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(299);
        return px_list_n((LXValue[]){px_str("Spawn"), _v387, _v385}, 3);
    }
    px_srcline(300);
    if (px_is_truthy(px_eq(_v384, px_str("select")))) {
        px_srcline(301);
        return px_call(px_get_global("parse_select"), (LXValue[]){}, 0);
    }
    px_srcline(302);
    if (px_is_truthy(px_eq(_v384, px_str("fn")))) {
        px_srcline(303);
        _v387 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(304);
        _v385 = px_call(px_get_global("node_pos"), (LXValue[]){_v387}, 1);
        px_srcline(305);
        return px_list_n((LXValue[]){px_str("ExprStmt"), _v387, _v385}, 3);
    }
    px_srcline(306);
    return px_call(px_get_global("parse_assign_or_expr"), (LXValue[]){}, 0);
px_err_388:
    if (px_err_388_proped) return px_err_388_val;
    return px_null();
}

static LXValue fn_parse_var_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_var_decl");
    LXValue _v393 = (nargs > 0) ? args[0] : px_null();
    LXValue _v394 = px_uninit();
    LXValue _v395 = px_uninit();
    LXValue _v396 = px_uninit();
    LXValue _v397 = px_uninit();
    LXValue px_err_398_val = px_null();
    int px_err_398_proped = 0;
    px_srcline(308);
    _v394 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(309);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(311);
    if (px_is_truthy(({ LXValue _t400 = ({ LXValue _t399 = px_eq(_v393, px_str("Let")); px_is_truthy(_t399) ? px_call(px_get_global("chk"), (LXValue[]){px_str("标识符")}, 1) : _t399; }); px_is_truthy(_t400) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("mut")) : _t400; }))) {
        px_srcline(312);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(313);
         _v393 = px_str("Mut");
    }
    px_srcline(314);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
        px_srcline(315);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）")}, 2));
    }
    px_srcline(316);
    _v395 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变量名")}, 1);
    px_srcline(317);
    _v396 = px_null();
    px_srcline(318);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(319);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(320);
         _v396 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(321);
    _v397 = px_null();
    px_srcline(322);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
        px_srcline(323);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(324);
         _v397 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    }
    px_srcline(325);
    return px_list_n((LXValue[]){px_str("VarDecl"), _v393, px_call(px_get_global("qstr"), (LXValue[]){_v395}, 1), _v396, _v397, _v394}, 6);
px_err_398:
    if (px_err_398_proped) return px_err_398_val;
    return px_null();
}

static LXValue fn_parse_assign_or_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_assign_or_expr");
    LXValue _v401 = px_uninit();
    LXValue _v402 = px_uninit();
    LXValue _v403 = px_uninit();
    LXValue _v404 = px_uninit();
    LXValue _v405 = px_uninit();
    LXValue _v406 = px_uninit();
    LXValue _v407 = px_uninit();
    LXValue px_err_408_val = px_null();
    int px_err_408_proped = 0;
    px_srcline(329);
    _v401 = px_null();
    px_srcline(330);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")))) {
        px_srcline(331);
         _v401 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
    }
    px_srcline(332);
    _v402 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(333);
    _v403 = px_call(px_get_global("node_pos"), (LXValue[]){_v402}, 1);
    px_srcline(334);
    _v404 = px_null();
    px_srcline(335);
    _v405 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(336);
    if (px_is_truthy(px_eq(_v405, px_str("=")))) {
        px_srcline(337);
         _v404 = px_str("Assign");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("<-")))) {
        px_srcline(340);
         _v404 = px_str("Append");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("+=")))) {
        px_srcline(342);
         _v404 = px_str("Plus");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("-=")))) {
        px_srcline(344);
         _v404 = px_str("Minus");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("*=")))) {
        px_srcline(346);
         _v404 = px_str("Star");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("/=")))) {
        px_srcline(348);
         _v404 = px_str("Slash");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("//=")))) {
        px_srcline(350);
         _v404 = px_str("IntDiv");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("%=")))) {
        px_srcline(352);
         _v404 = px_str("Mod");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("**=")))) {
        px_srcline(354);
         _v404 = px_str("Pow");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("&=")))) {
        px_srcline(356);
         _v404 = px_str("BitAnd");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("|=")))) {
        px_srcline(358);
         _v404 = px_str("BitOr");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("^=")))) {
        px_srcline(360);
         _v404 = px_str("BitXor");
    }
    else if (px_is_truthy(px_eq(_v405, px_str("<<=")))) {
        px_srcline(362);
         _v404 = px_str("Shl");
    }
    else if (px_is_truthy(px_eq(_v405, px_str(">>=")))) {
        px_srcline(364);
         _v404 = px_str("Shr");
    }
    else if (px_is_truthy(px_eq(_v405, px_str(">>>=")))) {
        px_srcline(366);
         _v404 = px_str("ShrU");
    }
    px_srcline(367);
    if (px_is_truthy(px_ne(_v404, px_null()))) {
        px_srcline(368);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(369);
        _v406 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(370);
        return px_list_n((LXValue[]){px_str("Assign"), _v402, _v404, _v406, _v403}, 5);
    }
    px_srcline(383);
    if (px_is_truthy(px_eq(px_index(_v402, px_int(0LL)), px_str("Var")))) {
        px_srcline(384);
        _v407 = px_null();
        px_srcline(385);
        if (px_is_truthy(px_ne(_v401, px_null()))) {
            px_srcline(386);
             _v407 = _v401;
        }
        else {
            px_srcline(388);
             _v407 = px_index(_v402, px_int(1LL));
        }
        px_srcline(389);
        if (px_is_truthy(px_eq(_v407, px_str("pass")))) {
            px_srcline(390);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2012"), px_str("`pass` 不是 PuXian 关键字（它是普通标识符，`var pass = 0` 合法）：空语句请写 `0`，或直接删掉这一行")}, 2));
        }
        px_srcline(391);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2012"), px_add(px_add(px_str("裸标识符语句没有效果："), _v407), px_str("（调用需要括号 `()`；若想要空语句请写 `0`）"))}, 2));
    }
    px_srcline(392);
    return px_list_n((LXValue[]){px_str("ExprStmt"), _v402, _v403}, 3);
px_err_408:
    if (px_err_408_proped) return px_err_408_val;
    return px_null();
}

static LXValue fn_parse_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if");
    LXValue _v409 = px_uninit();
    LXValue _v410 = px_uninit();
    LXValue _v411 = px_uninit();
    LXValue _v412 = px_uninit();
    LXValue _v413 = px_uninit();
    LXValue _v414 = px_uninit();
    LXValue _v415 = px_uninit();
    LXValue _v416 = px_uninit();
    LXValue px_err_417_val = px_null();
    int px_err_417_proped = 0;
    px_srcline(394);
    _v409 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(395);
    _v410 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(396);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(397);
    _v411 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(398);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(399);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(400);
    _v412 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v410}, 1);
    px_srcline(401);
    _v413 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v411, _v412}, 2)}, 1);
    px_srcline(402);
    _v414 = px_null();
    px_srcline(403);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(404);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("elif")}, 1))) {
            px_srcline(405);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(406);
            _v415 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(407);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(408);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(409);
            _v416 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v410}, 1);
            px_srcline(410);
            (void)(px_method(_v413, "append", (LXValue[]){px_list_n((LXValue[]){_v415, _v416}, 2)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("else")}, 1))) {
            px_srcline(412);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(413);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(414);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
            px_srcline(415);
             _v414 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v410}, 1);
            px_srcline(416);
            break;
        }
        else {
            px_srcline(418);
            break;
        }
    }
    px_srcline(419);
    return px_list_n((LXValue[]){px_str("If"), _v413, _v414, _v409}, 4);
px_err_417:
    if (px_err_417_proped) return px_err_417_val;
    return px_null();
}

static LXValue fn_parse_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_for");
    LXValue _v418 = px_uninit();
    LXValue _v419 = px_uninit();
    LXValue _v420 = px_uninit();
    LXValue _v421 = px_uninit();
    LXValue _v422 = px_uninit();
    LXValue _v423 = px_uninit();
    LXValue _v424 = px_uninit();
    LXValue px_err_425_val = px_null();
    int px_err_425_proped = 0;
    px_srcline(421);
    _v418 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(422);
    _v419 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(423);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(428);
    _v420 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1);
    px_srcline(429);
    _v421 = px_call(px_get_global("qstr"), (LXValue[]){_v420}, 1);
    px_srcline(430);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(431);
        _v422 = px_list_n((LXValue[]){_v421}, 1);
        px_srcline(432);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(433);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(434);
            (void)(px_method(_v422, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("循环变量")}, 1)}, 1)}, 1));
        }
        px_srcline(435);
         _v421 = _v422;
    }
    px_srcline(436);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
    px_srcline(437);
    _v423 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(438);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(439);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(440);
    _v424 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v419}, 1);
    px_srcline(441);
    return px_list_n((LXValue[]){px_str("For"), _v421, _v423, _v424, _v418}, 5);
px_err_425:
    if (px_err_425_proped) return px_err_425_val;
    return px_null();
}

static LXValue fn_parse_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_while");
    LXValue _v426 = px_uninit();
    LXValue _v427 = px_uninit();
    LXValue _v428 = px_uninit();
    LXValue _v429 = px_uninit();
    LXValue px_err_430_val = px_null();
    int px_err_430_proped = 0;
    px_srcline(443);
    _v426 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(444);
    _v427 = px_call(px_get_global("pcol"), (LXValue[]){}, 0);
    px_srcline(445);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(446);
    _v428 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(447);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(448);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(449);
    _v429 = px_call(px_get_global("parse_block_ctxt"), (LXValue[]){_v427}, 1);
    px_srcline(450);
    return px_list_n((LXValue[]){px_str("While"), _v428, _v429, _v426}, 4);
px_err_430:
    if (px_err_430_proped) return px_err_430_val;
    return px_null();
}

static LXValue fn_parse_block_ctxt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block_ctxt");
    LXValue _v431 = (nargs > 0) ? args[0] : px_null();
    LXValue _v432 = px_uninit();
    LXValue _v433 = px_uninit();
    LXValue px_err_434_val = px_null();
    int px_err_434_proped = 0;
    px_srcline(457);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(458);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(459);
    if (px_is_truthy(px_le(px_get_global("p_paren_ctxt"), px_int(0LL)))) {
        px_srcline(460);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
        px_srcline(461);
        return px_list_n((LXValue[]){}, 0);
    }
    px_srcline(462);
    _v432 = px_list_n((LXValue[]){}, 0);
    px_srcline(463);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(464);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(465);
        _v433 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(466);
        if (px_is_truthy(({ LXValue _t439 = ({ LXValue _t438 = ({ LXValue _t437 = ({ LXValue _t436 = ({ LXValue _t435 = px_eq(_v433, px_str("去缩进")); px_is_truthy(_t435) ? _t435 : px_eq(_v433, px_str("EOF")); }); px_is_truthy(_t436) ? _t436 : px_eq(_v433, px_str(")")); }); px_is_truthy(_t437) ? _t437 : px_eq(_v433, px_str("]")); }); px_is_truthy(_t438) ? _t438 : px_eq(_v433, px_str("}")); }); px_is_truthy(_t439) ? _t439 : px_eq(_v433, px_str(",")); }))) {
            px_srcline(467);
            break;
        }
        px_srcline(468);
        if (px_is_truthy(px_le(px_call(px_get_global("pcol"), (LXValue[]){}, 0), _v431))) {
            px_srcline(469);
            break;
        }
        px_srcline(470);
        (void)(px_method(_v432, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(471);
    return _v432;
px_err_434:
    if (px_err_434_proped) return px_err_434_val;
    return px_null();
}

static LXValue fn_parse_block(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_block");
    LXValue _v440 = px_uninit();
    LXValue px_err_441_val = px_null();
    int px_err_441_proped = 0;
    px_srcline(473);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(474);
    _v440 = px_list_n((LXValue[]){}, 0);
    px_srcline(475);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(476);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(477);
        if (px_is_truthy(({ LXValue _t442 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t442) ? _t442 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(478);
            break;
        }
        px_srcline(479);
        (void)(px_method(_v440, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
    }
    px_srcline(480);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(481);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("代码块未正确结束（缺少去缩进）")}, 2));
    }
    px_srcline(482);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(483);
    return _v440;
px_err_441:
    if (px_err_441_proped) return px_err_441_val;
    return px_null();
}

static LXValue fn_parse_type_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_params");
    LXValue _v443 = px_uninit();
    LXValue _v444 = px_uninit();
    LXValue _v445 = px_uninit();
    LXValue _v446 = px_uninit();
    LXValue px_err_447_val = px_null();
    int px_err_447_proped = 0;
    px_srcline(487);
    _v443 = px_list_n((LXValue[]){}, 0);
    px_srcline(488);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1)))) {
        px_srcline(489);
        return _v443;
    }
    px_srcline(490);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(491);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(492);
        _v444 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型参数名")}, 1);
        px_srcline(493);
        _v445 = _v444;
        px_srcline(494);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
            px_srcline(495);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(496);
            _v446 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("泛型约束名")}, 1);
            px_srcline(497);
             _v445 = px_add(px_add(_v444, px_str(": ")), _v446);
        }
        px_srcline(498);
        (void)(px_method(_v443, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){_v445}, 1)}, 1));
        px_srcline(499);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(500);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(501);
            continue;
        }
        px_srcline(502);
        break;
    }
    px_srcline(503);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(504);
    return _v443;
px_err_447:
    if (px_err_447_proped) return px_err_447_val;
    return px_null();
}

static LXValue fn_parse_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_func_def");
    LXValue _v448 = px_uninit();
    LXValue _v449 = px_uninit();
    LXValue _v450 = px_uninit();
    LXValue _v451 = px_uninit();
    LXValue _v452 = px_uninit();
    LXValue _v453 = px_uninit();
    LXValue px_err_454_val = px_null();
    int px_err_454_proped = 0;
    px_srcline(506);
    _v448 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(507);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(508);
    _v449 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(509);
    _v450 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(510);
    _v451 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(511);
    _v452 = px_null();
    px_srcline(512);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(513);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(514);
         _v452 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(515);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(516);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(517);
    _v453 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    px_srcline(518);
    return px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v449}, 1), _v451, _v452, _v453, _v448, _v450}, 7);
px_err_454:
    if (px_err_454_proped) return px_err_454_val;
    return px_null();
}

static LXValue fn_parse_extern_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_extern_def");
    LXValue _v455 = px_uninit();
    LXValue _v456 = px_uninit();
    LXValue _v457 = px_uninit();
    LXValue _v458 = px_uninit();
    LXValue px_err_459_val = px_null();
    int px_err_459_proped = 0;
    px_srcline(522);
    _v455 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(523);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(524);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("def"), px_str("'def'")}, 2));
    px_srcline(525);
    _v456 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("函数名")}, 1);
    px_srcline(526);
    _v457 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(527);
    _v458 = px_null();
    px_srcline(528);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(529);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(530);
         _v458 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(531);
    return px_list_n((LXValue[]){px_str("ExternDef"), px_call(px_get_global("qstr"), (LXValue[]){_v456}, 1), _v457, _v458, _v455}, 5);
px_err_459:
    if (px_err_459_proped) return px_err_459_val;
    return px_null();
}

static LXValue fn_parse_struct_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_struct_def");
    LXValue _v460 = px_uninit();
    LXValue _v461 = px_uninit();
    LXValue _v462 = px_uninit();
    LXValue _v463 = px_uninit();
    LXValue _v464 = px_uninit();
    LXValue _v465 = px_uninit();
    LXValue _v466 = px_uninit();
    LXValue px_err_467_val = px_null();
    int px_err_467_proped = 0;
    px_srcline(533);
    _v460 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(534);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(535);
    _v461 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("结构体名")}, 1);
    px_srcline(536);
    _v462 = px_call(px_get_global("parse_type_params"), (LXValue[]){}, 0);
    px_srcline(537);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(538);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(539);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(540);
    _v463 = px_list_n((LXValue[]){}, 0);
    px_srcline(541);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(542);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(543);
        if (px_is_truthy(({ LXValue _t468 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t468) ? _t468 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(544);
            break;
        }
        px_srcline(545);
        _v464 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(546);
        _v465 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("字段名")}, 1);
        px_srcline(547);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(548);
        _v466 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(549);
        (void)(px_method(_v463, "append", (LXValue[]){px_list_n((LXValue[]){px_str("StructField"), px_call(px_get_global("qstr"), (LXValue[]){_v465}, 1), _v466, _v464}, 4)}, 1));
        px_srcline(550);
        if (px_is_truthy(({ LXValue _t469 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t469) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t469; }))) {
            px_srcline(551);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体字段后期望换行")}, 2));
        }
    }
    px_srcline(552);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(553);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("结构体定义未正确结束")}, 2));
    }
    px_srcline(554);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(555);
    return px_list_n((LXValue[]){px_str("StructDef"), px_call(px_get_global("qstr"), (LXValue[]){_v461}, 1), _v463, _v460, _v462}, 5);
px_err_467:
    if (px_err_467_proped) return px_err_467_val;
    return px_null();
}

static LXValue fn_parse_enum_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_enum_def");
    LXValue _v470 = px_uninit();
    LXValue _v471 = px_uninit();
    LXValue _v472 = px_uninit();
    LXValue _v473 = px_uninit();
    LXValue _v474 = px_uninit();
    LXValue _v475 = px_uninit();
    LXValue px_err_476_val = px_null();
    int px_err_476_proped = 0;
    px_srcline(557);
    _v470 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(558);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(559);
    _v471 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(560);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(561);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(562);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(563);
    _v472 = px_list_n((LXValue[]){}, 0);
    px_srcline(564);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(565);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(566);
        if (px_is_truthy(({ LXValue _t477 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t477) ? _t477 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(567);
            break;
        }
        px_srcline(568);
        _v473 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(569);
        _v474 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("变体名")}, 1);
        px_srcline(570);
        _v475 = px_list_n((LXValue[]){}, 0);
        px_srcline(571);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(572);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(573);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(574);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(575);
                    (void)(px_method(_v475, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                    px_srcline(576);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(577);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(578);
                        continue;
                    }
                    px_srcline(579);
                    break;
                }
            }
            px_srcline(580);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        }
        px_srcline(581);
        (void)(px_method(_v472, "append", (LXValue[]){px_list_n((LXValue[]){px_str("EnumVariant"), px_call(px_get_global("qstr"), (LXValue[]){_v474}, 1), _v475, _v473}, 4)}, 1));
        px_srcline(582);
        if (px_is_truthy(({ LXValue _t478 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t478) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t478; }))) {
            px_srcline(583);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举变体后期望换行")}, 2));
        }
    }
    px_srcline(584);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(585);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("枚举定义未正确结束")}, 2));
    }
    px_srcline(586);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(587);
    return px_list_n((LXValue[]){px_str("EnumDef"), px_call(px_get_global("qstr"), (LXValue[]){_v471}, 1), _v472, _v470}, 4);
px_err_476:
    if (px_err_476_proped) return px_err_476_val;
    return px_null();
}

static LXValue fn_parse_type_const(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_const");
    LXValue _v479 = px_uninit();
    LXValue _v480 = px_uninit();
    LXValue _v481 = px_uninit();
    LXValue _v482 = px_uninit();
    LXValue _v483 = px_uninit();
    LXValue _v484 = px_uninit();
    LXValue px_err_485_val = px_null();
    int px_err_485_proped = 0;
    px_srcline(592);
    _v479 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(593);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(594);
    _v480 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("枚举名")}, 1);
    px_srcline(595);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("const"), px_str("'const'")}, 2));
    px_srcline(596);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(597);
    _v481 = px_list_n((LXValue[]){}, 0);
    px_srcline(598);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(599);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(600);
            _v482 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(601);
            _v483 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("常量名")}, 1);
            px_srcline(602);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
            px_srcline(603);
            _v484 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(604);
            (void)(px_method(_v481, "append", (LXValue[]){px_list_n((LXValue[]){px_str("TypeConstItem"), px_call(px_get_global("qstr"), (LXValue[]){_v483}, 1), _v484, _v482}, 4)}, 1));
            px_srcline(605);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(606);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(607);
                continue;
            }
            px_srcline(608);
            break;
        }
    }
    px_srcline(609);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(610);
    return px_list_n((LXValue[]){px_str("TypeConst"), px_call(px_get_global("qstr"), (LXValue[]){_v480}, 1), _v481, _v479}, 4);
px_err_485:
    if (px_err_485_proped) return px_err_485_val;
    return px_null();
}

static LXValue fn_parse_trait_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_trait_def");
    LXValue _v486 = px_uninit();
    LXValue _v487 = px_uninit();
    LXValue _v488 = px_uninit();
    LXValue _v489 = px_uninit();
    LXValue _v490 = px_uninit();
    LXValue _v491 = px_uninit();
    LXValue _v492 = px_uninit();
    LXValue _v493 = px_uninit();
    LXValue px_err_494_val = px_null();
    int px_err_494_proped = 0;
    px_srcline(612);
    _v486 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(613);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(614);
    _v487 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("trait 名")}, 1);
    px_srcline(615);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(616);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(617);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(618);
    _v488 = px_list_n((LXValue[]){}, 0);
    px_srcline(619);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(620);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(621);
        if (px_is_truthy(({ LXValue _t495 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t495) ? _t495 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(622);
            break;
        }
        px_srcline(623);
        _v489 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(624);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(625);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 内只允许 def 方法")}, 2));
        }
        px_srcline(626);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(627);
        _v490 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(628);
        _v491 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(629);
        _v492 = px_null();
        px_srcline(630);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(631);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(632);
             _v492 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(633);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(634);
        _v493 = px_list_n((LXValue[]){}, 0);
        px_srcline(635);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(636);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(637);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(638);
                 _v493 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            }
        }
        px_srcline(639);
        (void)(px_method(_v488, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v490}, 1), _v491, _v492, _v493, _v489, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
        px_srcline(640);
        if (px_is_truthy(({ LXValue _t496 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1)); px_is_truthy(_t496) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1)) : _t496; }))) {
            px_srcline(641);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 方法后期望换行")}, 2));
        }
    }
    px_srcline(642);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(643);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("trait 定义未正确结束")}, 2));
    }
    px_srcline(644);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(645);
    return px_list_n((LXValue[]){px_str("TraitDef"), px_call(px_get_global("qstr"), (LXValue[]){_v487}, 1), _v488, _v486}, 4);
px_err_494:
    if (px_err_494_proped) return px_err_494_val;
    return px_null();
}

static LXValue fn_parse_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_impl_def");
    LXValue _v497 = px_uninit();
    LXValue _v498 = px_uninit();
    LXValue _v499 = px_uninit();
    LXValue _v500 = px_uninit();
    LXValue _v501 = px_uninit();
    LXValue _v502 = px_uninit();
    LXValue _v503 = px_uninit();
    LXValue _v504 = px_uninit();
    LXValue _v505 = px_uninit();
    LXValue _v506 = px_uninit();
    LXValue px_err_507_val = px_null();
    int px_err_507_proped = 0;
    px_srcline(647);
    _v497 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(648);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(649);
    _v498 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名或 trait 名")}, 1);
    px_srcline(650);
    _v499 = px_null();
    px_srcline(651);
    _v500 = _v498;
    px_srcline(652);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(653);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(654);
         _v500 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("类型名")}, 1);
        px_srcline(655);
         _v499 = px_call(px_get_global("qstr"), (LXValue[]){_v498}, 1);
    }
    px_srcline(656);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(657);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(658);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(659);
    _v501 = px_list_n((LXValue[]){}, 0);
    px_srcline(660);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(661);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(662);
        if (px_is_truthy(({ LXValue _t508 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t508) ? _t508 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(663);
            break;
        }
        px_srcline(664);
        _v502 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(665);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("def")}, 1)))) {
            px_srcline(666);
            (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 内只允许 def 方法")}, 2));
        }
        px_srcline(667);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(668);
        _v503 = px_call(px_get_global("expect_ident"), (LXValue[]){px_str("方法名")}, 1);
        px_srcline(669);
        _v504 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
        px_srcline(670);
        _v505 = px_null();
        px_srcline(671);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(672);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(673);
             _v505 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        }
        px_srcline(674);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(675);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(676);
        _v506 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
        px_srcline(677);
        (void)(px_method(_v501, "append", (LXValue[]){px_list_n((LXValue[]){px_str("FuncDef"), px_call(px_get_global("qstr"), (LXValue[]){_v503}, 1), _v504, _v505, _v506, _v502, px_list_n((LXValue[]){}, 0)}, 7)}, 1));
    }
    px_srcline(678);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(679);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("impl 定义未正确结束")}, 2));
    }
    px_srcline(680);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(681);
    return px_list_n((LXValue[]){px_str("ImplDef"), px_call(px_get_global("qstr"), (LXValue[]){_v500}, 1), _v499, _v501, _v497}, 5);
px_err_507:
    if (px_err_507_proped) return px_err_507_val;
    return px_null();
}

static LXValue fn_parse_import(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import");
    LXValue _v509 = px_uninit();
    LXValue _v510 = px_uninit();
    LXValue _v511 = px_uninit();
    LXValue px_err_512_val = px_null();
    int px_err_512_proped = 0;
    px_srcline(683);
    _v509 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(684);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(685);
    if (px_is_truthy(px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("字符串")))) {
        px_srcline(686);
        _v510 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(687);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(688);
        return px_list_n((LXValue[]){px_str("Import"), px_list_n((LXValue[]){_v510}, 1), px_list_n((LXValue[]){}, 0), _v509}, 4);
    }
    px_srcline(689);
    _v511 = px_list_n((LXValue[]){}, 0);
    px_srcline(690);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(691);
        (void)(px_method(_v511, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(692);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(693);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(694);
            continue;
        }
        px_srcline(695);
        break;
    }
    px_srcline(696);
    return px_list_n((LXValue[]){px_str("Import"), _v511, px_list_n((LXValue[]){}, 0), _v509}, 4);
px_err_512:
    if (px_err_512_proped) return px_err_512_val;
    return px_null();
}

static LXValue fn_parse_import_from(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_import_from");
    LXValue _v513 = px_uninit();
    LXValue _v514 = px_uninit();
    LXValue _v515 = px_uninit();
    LXValue px_err_516_val = px_null();
    int px_err_516_proped = 0;
    px_srcline(698);
    _v513 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(699);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(700);
    _v514 = px_list_n((LXValue[]){}, 0);
    px_srcline(701);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(702);
        (void)(px_method(_v514, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("模块名")}, 1)}, 1)}, 1));
        px_srcline(703);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(704);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(705);
            continue;
        }
        px_srcline(706);
        break;
    }
    px_srcline(707);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("import"), px_str("'import'")}, 2));
    px_srcline(708);
    _v515 = px_list_n((LXValue[]){}, 0);
    px_srcline(709);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(710);
        (void)(px_method(_v515, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("导入名")}, 1)}, 1)}, 1));
        px_srcline(711);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(712);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(713);
            continue;
        }
        px_srcline(714);
        break;
    }
    px_srcline(715);
    return px_list_n((LXValue[]){px_str("Import"), _v514, _v515, _v513}, 4);
px_err_516:
    if (px_err_516_proped) return px_err_516_val;
    return px_null();
}

static LXValue fn_parse_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_select");
    LXValue _v517 = px_uninit();
    LXValue _v518 = px_uninit();
    LXValue _v519 = px_uninit();
    LXValue _v520 = px_uninit();
    LXValue _v521 = px_uninit();
    LXValue _v522 = px_uninit();
    LXValue _v523 = px_uninit();
    LXValue px_err_524_val = px_null();
    int px_err_524_proped = 0;
    px_srcline(717);
    _v517 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(718);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(719);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(720);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(721);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(722);
    _v518 = px_list_n((LXValue[]){}, 0);
    px_srcline(723);
    _v519 = px_null();
    px_srcline(724);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(725);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(726);
        if (px_is_truthy(({ LXValue _t525 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t525) ? _t525 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(727);
            break;
        }
        px_srcline(728);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(729);
        if (px_is_truthy(({ LXValue _t526 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t526) ? px_eq(px_call(px_get_global("pv"), (LXValue[]){}, 0), px_str("_")) : _t526; }))) {
            px_srcline(730);
            _v520 = px_get_global("p_pos");
            px_srcline(731);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(732);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(733);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(734);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
                px_srcline(735);
                 _v519 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
                px_srcline(736);
                continue;
            }
            else {
                px_srcline(738);
                px_set_global("p_pos", _v520);
            }
        }
        px_srcline(739);
        _v521 = px_null();
        px_srcline(740);
        if (px_is_truthy(({ LXValue _t527 = px_eq(px_call(px_get_global("pk"), (LXValue[]){}, 0), px_str("标识符")); px_is_truthy(_t527) ? px_call(px_get_global("chk2"), (LXValue[]){px_str("=")}, 1) : _t527; }))) {
            px_srcline(741);
             _v521 = px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("绑定变量")}, 1)}, 1);
            px_srcline(742);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("="), px_str("'='")}, 2));
        }
        px_srcline(743);
        _v522 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(744);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(745);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(746);
        _v523 = px_call(px_get_global("parse_case_body"), (LXValue[]){}, 0);
        px_srcline(747);
        (void)(px_method(_v518, "append", (LXValue[]){px_list_n((LXValue[]){_v521, _v522, _v523}, 3)}, 1));
    }
    px_srcline(748);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(749);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("select 定义未正确结束")}, 2));
    }
    px_srcline(750);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(751);
    return px_list_n((LXValue[]){px_str("Select"), _v518, _v519, _v517}, 4);
px_err_524:
    if (px_err_524_proped) return px_err_524_val;
    return px_null();
}

static LXValue fn_parse_case_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_case_body");
    LXValue _v528 = px_uninit();
    LXValue _v529 = px_uninit();
    LXValue px_err_530_val = px_null();
    int px_err_530_proped = 0;
    px_srcline(753);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
        px_srcline(754);
        return px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
    }
    px_srcline(755);
    _v528 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(756);
    _v529 = px_call(px_get_global("node_pos"), (LXValue[]){_v528}, 1);
    px_srcline(757);
    return px_list_n((LXValue[]){px_list_n((LXValue[]){px_str("ExprStmt"), _v528, _v529}, 3)}, 1);
px_err_530:
    if (px_err_530_proped) return px_err_530_val;
    return px_null();
}

static LXValue fn_parse_params(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_params");
    LXValue _v531 = px_uninit();
    LXValue _v532 = px_uninit();
    LXValue _v533 = px_uninit();
    LXValue _v534 = px_uninit();
    LXValue _v535 = px_uninit();
    LXValue px_err_536_val = px_null();
    int px_err_536_proped = 0;
    px_srcline(760);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(761);
    _v531 = px_list_n((LXValue[]){}, 0);
    px_srcline(762);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(763);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(764);
            _v532 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(765);
            _v533 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("参数名"), px_bool(false)}, 2);
            px_srcline(766);
            _v534 = px_null();
            px_srcline(767);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(768);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(769);
                 _v534 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            }
            px_srcline(770);
            _v535 = px_null();
            px_srcline(771);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("=")}, 1))) {
                px_srcline(772);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(773);
                 _v535 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            }
            px_srcline(774);
            (void)(px_method(_v531, "append", (LXValue[]){px_list_n((LXValue[]){px_str("Param"), px_call(px_get_global("qstr"), (LXValue[]){_v533}, 1), _v534, _v535, _v532}, 5)}, 1));
            px_srcline(775);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(776);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(777);
                continue;
            }
            px_srcline(778);
            break;
        }
    }
    px_srcline(779);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(780);
    return _v531;
px_err_536:
    if (px_err_536_proped) return px_err_536_val;
    return px_null();
}

static LXValue fn_parse_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_expr");
    LXValue px_err_537_val = px_null();
    int px_err_537_proped = 0;
    px_srcline(783);
    return px_call(px_get_global("parse_pipe"), (LXValue[]){}, 0);
px_err_537:
    if (px_err_537_proped) return px_err_537_val;
    return px_null();
}

static LXValue fn_parse_pipe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pipe");
    LXValue _v538 = px_uninit();
    LXValue _v539 = px_uninit();
    LXValue _v540 = px_uninit();
    LXValue px_err_541_val = px_null();
    int px_err_541_proped = 0;
    px_srcline(785);
    _v538 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
    px_srcline(786);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|>")}, 1))) {
        px_srcline(787);
        _v539 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(788);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(789);
        _v540 = px_call(px_get_global("parse_null_coalesce"), (LXValue[]){}, 0);
        px_srcline(790);
         _v538 = px_list_n((LXValue[]){px_str("Pipe"), _v538, _v540, _v539}, 4);
    }
    px_srcline(791);
    return _v538;
px_err_541:
    if (px_err_541_proped) return px_err_541_val;
    return px_null();
}

static LXValue fn_parse_null_coalesce(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_null_coalesce");
    LXValue _v542 = px_uninit();
    LXValue _v543 = px_uninit();
    LXValue _v544 = px_uninit();
    LXValue px_err_545_val = px_null();
    int px_err_545_proped = 0;
    px_srcline(793);
    _v542 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
    px_srcline(794);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("??")}, 1))) {
        px_srcline(795);
        _v543 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(796);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(797);
        _v544 = px_call(px_get_global("parse_or"), (LXValue[]){}, 0);
        px_srcline(798);
         _v542 = px_list_n((LXValue[]){px_str("NullCoalesce"), _v542, _v544, _v543}, 4);
    }
    px_srcline(799);
    return _v542;
px_err_545:
    if (px_err_545_proped) return px_err_545_val;
    return px_null();
}

static LXValue fn_parse_or(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_or");
    LXValue _v546 = px_uninit();
    LXValue _v547 = px_uninit();
    LXValue _v548 = px_uninit();
    LXValue px_err_549_val = px_null();
    int px_err_549_proped = 0;
    px_srcline(801);
    _v546 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
    px_srcline(802);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("or")}, 1))) {
        px_srcline(803);
        _v547 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(804);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(805);
        _v548 = px_call(px_get_global("parse_and"), (LXValue[]){}, 0);
        px_srcline(806);
         _v546 = px_list_n((LXValue[]){px_str("Binary"), px_str("Or"), _v546, _v548, _v547}, 5);
    }
    px_srcline(807);
    return _v546;
px_err_549:
    if (px_err_549_proped) return px_err_549_val;
    return px_null();
}

static LXValue fn_parse_and(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_and");
    LXValue _v550 = px_uninit();
    LXValue _v551 = px_uninit();
    LXValue _v552 = px_uninit();
    LXValue px_err_553_val = px_null();
    int px_err_553_proped = 0;
    px_srcline(809);
    _v550 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
    px_srcline(810);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("and")}, 1))) {
        px_srcline(811);
        _v551 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(812);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(813);
        _v552 = px_call(px_get_global("parse_comparison"), (LXValue[]){}, 0);
        px_srcline(814);
         _v550 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v550, _v552, _v551}, 5);
    }
    px_srcline(815);
    return _v550;
px_err_553:
    if (px_err_553_proped) return px_err_553_val;
    return px_null();
}

static LXValue fn_parse_comparison(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comparison");
    LXValue _v554 = px_uninit();
    LXValue _v555 = px_uninit();
    LXValue _v556 = px_uninit();
    LXValue _v557 = px_uninit();
    LXValue px_err_558_val = px_null();
    int px_err_558_proped = 0;
    px_srcline(817);
    _v554 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
    px_srcline(818);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(819);
        _v555 = px_null();
        px_srcline(820);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("==")}, 1))) {
            px_srcline(821);
             _v555 = px_str("Eq");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("!=")}, 1))) {
            px_srcline(823);
             _v555 = px_str("Ne");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<")}, 1))) {
            px_srcline(825);
             _v555 = px_str("Lt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<=")}, 1))) {
            px_srcline(827);
             _v555 = px_str("Le");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">")}, 1))) {
            px_srcline(829);
             _v555 = px_str("Gt");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">=")}, 1))) {
            px_srcline(831);
             _v555 = px_str("Ge");
        }
        px_srcline(832);
        if (px_is_truthy(px_eq(_v555, px_null()))) {
            px_srcline(833);
            break;
        }
        px_srcline(834);
        _v556 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(835);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(836);
        _v557 = px_call(px_get_global("parse_bitor"), (LXValue[]){}, 0);
        px_srcline(837);
         _v554 = px_list_n((LXValue[]){px_str("Binary"), _v555, _v554, _v557, _v556}, 5);
    }
    px_srcline(838);
    return _v554;
px_err_558:
    if (px_err_558_proped) return px_err_558_val;
    return px_null();
}

static LXValue fn_parse_bitor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitor");
    LXValue _v559 = px_uninit();
    LXValue _v560 = px_uninit();
    LXValue _v561 = px_uninit();
    LXValue px_err_562_val = px_null();
    int px_err_562_proped = 0;
    px_srcline(840);
    _v559 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
    px_srcline(841);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("|")}, 1))) {
        px_srcline(842);
        _v560 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(843);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(844);
        _v561 = px_call(px_get_global("parse_bitxor"), (LXValue[]){}, 0);
        px_srcline(845);
         _v559 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitOr"), _v559, _v561, _v560}, 5);
    }
    px_srcline(846);
    return _v559;
px_err_562:
    if (px_err_562_proped) return px_err_562_val;
    return px_null();
}

static LXValue fn_parse_bitxor(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitxor");
    LXValue _v563 = px_uninit();
    LXValue _v564 = px_uninit();
    LXValue _v565 = px_uninit();
    LXValue px_err_566_val = px_null();
    int px_err_566_proped = 0;
    px_srcline(848);
    _v563 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
    px_srcline(849);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("^")}, 1))) {
        px_srcline(850);
        _v564 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(851);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(852);
        _v565 = px_call(px_get_global("parse_bitand"), (LXValue[]){}, 0);
        px_srcline(853);
         _v563 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitXor"), _v563, _v565, _v564}, 5);
    }
    px_srcline(854);
    return _v563;
px_err_566:
    if (px_err_566_proped) return px_err_566_val;
    return px_null();
}

static LXValue fn_parse_bitand(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_bitand");
    LXValue _v567 = px_uninit();
    LXValue _v568 = px_uninit();
    LXValue _v569 = px_uninit();
    LXValue px_err_570_val = px_null();
    int px_err_570_proped = 0;
    px_srcline(856);
    _v567 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
    px_srcline(857);
    while (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("&")}, 1))) {
        px_srcline(858);
        _v568 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(859);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(860);
        _v569 = px_call(px_get_global("parse_shift"), (LXValue[]){}, 0);
        px_srcline(861);
         _v567 = px_list_n((LXValue[]){px_str("Binary"), px_str("BitAnd"), _v567, _v569, _v568}, 5);
    }
    px_srcline(862);
    return _v567;
px_err_570:
    if (px_err_570_proped) return px_err_570_val;
    return px_null();
}

static LXValue fn_parse_shift(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_shift");
    LXValue _v571 = px_uninit();
    LXValue _v572 = px_uninit();
    LXValue _v573 = px_uninit();
    LXValue _v574 = px_uninit();
    LXValue px_err_575_val = px_null();
    int px_err_575_proped = 0;
    px_srcline(864);
    _v571 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
    px_srcline(865);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(866);
        _v572 = px_null();
        px_srcline(867);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("<<")}, 1))) {
            px_srcline(868);
             _v572 = px_str("Shl");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>")}, 1))) {
            px_srcline(870);
             _v572 = px_str("Shr");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str(">>>")}, 1))) {
            px_srcline(872);
             _v572 = px_str("ShrU");
        }
        px_srcline(873);
        if (px_is_truthy(px_eq(_v572, px_null()))) {
            px_srcline(874);
            break;
        }
        px_srcline(875);
        _v573 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(876);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(877);
        _v574 = px_call(px_get_global("parse_add"), (LXValue[]){}, 0);
        px_srcline(878);
         _v571 = px_list_n((LXValue[]){px_str("Binary"), _v572, _v571, _v574, _v573}, 5);
    }
    px_srcline(879);
    return _v571;
px_err_575:
    if (px_err_575_proped) return px_err_575_val;
    return px_null();
}

static LXValue fn_parse_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_add");
    LXValue _v576 = px_uninit();
    LXValue _v577 = px_uninit();
    LXValue _v578 = px_uninit();
    LXValue _v579 = px_uninit();
    LXValue px_err_580_val = px_null();
    int px_err_580_proped = 0;
    px_srcline(881);
    _v576 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
    px_srcline(882);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(883);
        _v577 = px_null();
        px_srcline(884);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("+")}, 1))) {
            px_srcline(885);
             _v577 = px_str("Add");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("-")}, 1))) {
            px_srcline(887);
             _v577 = px_str("Sub");
        }
        px_srcline(888);
        if (px_is_truthy(px_eq(_v577, px_null()))) {
            px_srcline(889);
            break;
        }
        px_srcline(890);
        _v578 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(891);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(892);
        _v579 = px_call(px_get_global("parse_mul"), (LXValue[]){}, 0);
        px_srcline(893);
         _v576 = px_list_n((LXValue[]){px_str("Binary"), _v577, _v576, _v579, _v578}, 5);
    }
    px_srcline(894);
    return _v576;
px_err_580:
    if (px_err_580_proped) return px_err_580_val;
    return px_null();
}

static LXValue fn_parse_mul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_mul");
    LXValue _v581 = px_uninit();
    LXValue _v582 = px_uninit();
    LXValue _v583 = px_uninit();
    LXValue _v584 = px_uninit();
    LXValue px_err_585_val = px_null();
    int px_err_585_proped = 0;
    px_srcline(896);
    _v581 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
    px_srcline(897);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(898);
        _v582 = px_null();
        px_srcline(899);
        if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("*")}, 1))) {
            px_srcline(900);
             _v582 = px_str("Mul");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("/")}, 1))) {
            px_srcline(902);
             _v582 = px_str("Div");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("//")}, 1))) {
            px_srcline(904);
             _v582 = px_str("IntDiv");
        }
        else if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("%")}, 1))) {
            px_srcline(906);
             _v582 = px_str("Mod");
        }
        px_srcline(907);
        if (px_is_truthy(px_eq(_v582, px_null()))) {
            px_srcline(908);
            break;
        }
        px_srcline(909);
        _v583 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(910);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(911);
        _v584 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(912);
         _v581 = px_list_n((LXValue[]){px_str("Binary"), _v582, _v581, _v584, _v583}, 5);
    }
    px_srcline(913);
    return _v581;
px_err_585:
    if (px_err_585_proped) return px_err_585_val;
    return px_null();
}

static LXValue fn_parse_pow(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pow");
    LXValue _v586 = px_uninit();
    LXValue _v587 = px_uninit();
    LXValue _v588 = px_uninit();
    LXValue px_err_589_val = px_null();
    int px_err_589_proped = 0;
    px_srcline(915);
    _v586 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
    px_srcline(916);
    if (px_is_truthy(px_call(px_get_global("chk_op"), (LXValue[]){px_str("**")}, 1))) {
        px_srcline(917);
        _v587 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(918);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(919);
        _v588 = px_call(px_get_global("parse_pow"), (LXValue[]){}, 0);
        px_srcline(920);
        return px_list_n((LXValue[]){px_str("Binary"), px_str("Pow"), _v586, _v588, _v587}, 5);
    }
    px_srcline(921);
    return _v586;
px_err_589:
    if (px_err_589_proped) return px_err_589_val;
    return px_null();
}

static LXValue fn_parse_unary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_unary");
    LXValue _v590 = px_uninit();
    LXValue _v591 = px_uninit();
    LXValue _v592 = px_uninit();
    LXValue px_err_593_val = px_null();
    int px_err_593_proped = 0;
    px_srcline(930);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(931);
    _v590 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(932);
    if (px_is_truthy(px_eq(_v590, px_str("-")))) {
        px_srcline(933);
        _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(934);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(935);
        _v592 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(936);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Neg"), _v592, _v591}, 4);
    }
    px_srcline(937);
    if (px_is_truthy(px_eq(_v590, px_str("not")))) {
        px_srcline(938);
        _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(939);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(940);
        _v592 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(941);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("Not"), _v592, _v591}, 4);
    }
    px_srcline(942);
    if (px_is_truthy(px_eq(_v590, px_str("~")))) {
        px_srcline(943);
        _v591 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(944);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(945);
        _v592 = px_call(px_get_global("parse_unary"), (LXValue[]){}, 0);
        px_srcline(946);
        return px_list_n((LXValue[]){px_str("Unary"), px_str("BitNot"), _v592, _v591}, 4);
    }
    px_srcline(947);
    return px_call(px_get_global("parse_postfix"), (LXValue[]){}, 0);
px_err_593:
    if (px_err_593_proped) return px_err_593_val;
    return px_null();
}

static LXValue fn_parse_postfix(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_postfix");
    LXValue _v594 = px_uninit();
    LXValue _v595 = px_uninit();
    LXValue _v596 = px_uninit();
    LXValue _v597 = px_uninit();
    LXValue _v598 = px_uninit();
    LXValue _v599 = px_uninit();
    LXValue _v600 = px_uninit();
    LXValue _v601 = px_uninit();
    LXValue px_err_602_val = px_null();
    int px_err_602_proped = 0;
    px_srcline(949);
    _v594 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
    px_srcline(950);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(951);
        _v595 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
        px_srcline(952);
        if (px_is_truthy(px_eq(_v595, px_str("(")))) {
            px_srcline(953);
            _v596 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(954);
            _v597 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(955);
             _v594 = px_list_n((LXValue[]){px_str("Call"), _v594, _v597, _v596}, 4);
        }
        else if (px_is_truthy(px_eq(_v595, px_str("[")))) {
            px_srcline(957);
            _v596 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(958);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(959);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(960);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                px_srcline(961);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(962);
                _v598 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                px_srcline(963);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(964);
                _v599 = px_null();
                px_srcline(965);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(966);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(967);
                     _v599 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(968);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                }
                px_srcline(969);
                (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                px_srcline(970);
                 _v594 = px_list_n((LXValue[]){px_str("Slice"), _v594, px_null(), _v598, _v599, _v596}, 6);
            }
            else {
                px_srcline(972);
                _v600 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
                px_srcline(973);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(974);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                    px_srcline(975);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(976);
                    _v598 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                    px_srcline(977);
                    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    px_srcline(978);
                    _v599 = px_null();
                    px_srcline(979);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
                        px_srcline(980);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(981);
                         _v599 = px_call(px_get_global("parse_slice_bound"), (LXValue[]){}, 0);
                        px_srcline(982);
                        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                    }
                    px_srcline(983);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(984);
                     _v594 = px_list_n((LXValue[]){px_str("Slice"), _v594, _v600, _v598, _v599, _v596}, 6);
                }
                else {
                    px_srcline(986);
                    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
                    px_srcline(987);
                     _v594 = px_list_n((LXValue[]){px_str("Index"), _v594, _v600, _v596}, 4);
                }
            }
        }
        else if (px_is_truthy(px_eq(_v595, px_str(".")))) {
            px_srcline(989);
            _v596 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(990);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(991);
            _v601 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名"), px_bool(true)}, 2);
            px_srcline(992);
             _v594 = px_list_n((LXValue[]){px_str("Field"), _v594, px_call(px_get_global("qstr"), (LXValue[]){_v601}, 1), _v596}, 4);
        }
        else if (px_is_truthy(px_eq(_v595, px_str("?.")))) {
            px_srcline(994);
            _v596 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(995);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(996);
            _v601 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名"), px_bool(true)}, 2);
            px_srcline(997);
             _v594 = px_list_n((LXValue[]){px_str("OptionalField"), _v594, px_call(px_get_global("qstr"), (LXValue[]){_v601}, 1), _v596}, 4);
        }
        else if (px_is_truthy(px_eq(_v595, px_str("!")))) {
            px_srcline(999);
            _v596 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1000);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1001);
             _v594 = px_list_n((LXValue[]){px_str("ForceUnwrap"), _v594, _v596}, 3);
        }
        else if (px_is_truthy(px_eq(_v595, px_str("?")))) {
            px_srcline(1003);
            _v596 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1004);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1005);
             _v594 = px_list_n((LXValue[]){px_str("Try"), _v594, _v596}, 3);
        }
        else {
            px_srcline(1007);
            break;
        }
    }
    px_srcline(1008);
    return _v594;
px_err_602:
    if (px_err_602_proped) return px_err_602_val;
    return px_null();
}

static LXValue fn_parse_slice_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_slice_bound");
    LXValue px_err_603_val = px_null();
    int px_err_603_proped = 0;
    px_srcline(1010);
    if (px_is_truthy(({ LXValue _t604 = px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); px_is_truthy(_t604) ? _t604 : px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1); }))) {
        px_srcline(1011);
        return px_null();
    }
    px_srcline(1012);
    return px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
px_err_603:
    if (px_err_603_proped) return px_err_603_val;
    return px_null();
}

static LXValue fn_parse_call_args(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_call_args");
    LXValue _v605 = px_uninit();
    LXValue px_err_606_val = px_null();
    int px_err_606_proped = 0;
    px_srcline(1014);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("("), px_str("'('")}, 2));
    px_srcline(1015);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1016);
    _v605 = px_list_n((LXValue[]){}, 0);
    px_srcline(1017);
    if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
        px_srcline(1018);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1019);
            (void)(px_method(_v605, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1020);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1021);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1022);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1023);
                (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
                px_srcline(1024);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                    px_srcline(1025);
                    break;
                }
                px_srcline(1026);
                continue;
            }
            px_srcline(1027);
            break;
        }
    }
    px_srcline(1028);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1029);
    return _v605;
px_err_606:
    if (px_err_606_proped) return px_err_606_val;
    return px_null();
}

static LXValue fn_parse_primary(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_primary");
    LXValue _v607 = px_uninit();
    LXValue _v608 = px_uninit();
    LXValue _v609 = px_uninit();
    LXValue _v610 = px_uninit();
    LXValue _v611 = px_uninit();
    LXValue _v612 = px_uninit();
    LXValue px_err_613_val = px_null();
    int px_err_613_proped = 0;
    px_srcline(1032);
    _v607 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1033);
    if (px_is_truthy(px_eq(_v607, px_str("整数")))) {
        px_srcline(1034);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1035);
        _v609 = px_call(px_get_global("int"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1036);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1037);
        return px_list_n((LXValue[]){px_str("Int"), _v609, _v608}, 3);
    }
    px_srcline(1038);
    if (px_is_truthy(px_eq(_v607, px_str("浮点")))) {
        px_srcline(1039);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1040);
        _v609 = px_call(px_get_global("float"), (LXValue[]){px_call(px_get_global("pv"), (LXValue[]){}, 0)}, 1);
        px_srcline(1041);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1042);
        return px_list_n((LXValue[]){px_str("Float"), _v609, _v608}, 3);
    }
    px_srcline(1043);
    if (px_is_truthy(px_eq(_v607, px_str("字符串")))) {
        px_srcline(1044);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1045);
        _v609 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1046);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1047);
        return px_list_n((LXValue[]){px_str("Str"), _v609, _v608}, 3);
    }
    px_srcline(1048);
    if (px_is_truthy(px_eq(_v607, px_str("true")))) {
        px_srcline(1049);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1050);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1051);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(true), _v608}, 3);
    }
    px_srcline(1052);
    if (px_is_truthy(px_eq(_v607, px_str("false")))) {
        px_srcline(1053);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1054);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1055);
        return px_list_n((LXValue[]){px_str("Bool"), px_bool(false), _v608}, 3);
    }
    px_srcline(1056);
    if (px_is_truthy(px_eq(_v607, px_str("null")))) {
        px_srcline(1057);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1058);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1059);
        return px_list_n((LXValue[]){px_str("Null"), _v608}, 2);
    }
    px_srcline(1060);
    if (px_is_truthy(px_eq(_v607, px_str("self")))) {
        px_srcline(1061);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1062);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1063);
        return px_list_n((LXValue[]){px_str("Var"), px_str("\"self\""), _v608}, 3);
    }
    px_srcline(1064);
    if (px_is_truthy(px_eq(_v607, px_str("标识符")))) {
        px_srcline(1065);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1066);
        _v610 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1067);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1068);
        return px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v610}, 1), _v608}, 3);
    }
    px_srcline(1069);
    if (px_is_truthy(px_eq(_v607, px_str("[")))) {
        px_srcline(1070);
        return px_call(px_get_global("parse_list_or_comp"), (LXValue[]){}, 0);
    }
    px_srcline(1071);
    if (px_is_truthy(px_eq(_v607, px_str("(")))) {
        px_srcline(1072);
        return px_call(px_get_global("parse_paren_or_tuple"), (LXValue[]){}, 0);
    }
    px_srcline(1073);
    if (px_is_truthy(px_eq(_v607, px_str("{")))) {
        px_srcline(1074);
        return px_call(px_get_global("parse_brace"), (LXValue[]){}, 0);
    }
    px_srcline(1075);
    if (px_is_truthy(px_eq(_v607, px_str("fn")))) {
        px_srcline(1076);
        return px_call(px_get_global("parse_closure"), (LXValue[]){}, 0);
    }
    px_srcline(1077);
    if (px_is_truthy(px_eq(_v607, px_str("match")))) {
        px_srcline(1078);
        return px_call(px_get_global("parse_match_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1079);
    if (px_is_truthy(px_eq(_v607, px_str("if")))) {
        px_srcline(1080);
        return px_call(px_get_global("parse_if_expr"), (LXValue[]){}, 0);
    }
    px_srcline(1081);
    if (px_is_truthy(px_eq(_v607, px_str("chan")))) {
        px_srcline(1082);
        _v608 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1083);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1084);
        _v611 = px_list_n((LXValue[]){px_str("Var"), px_str("\"chan\""), _v608}, 3);
        px_srcline(1085);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1086);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1087);
            (void)(px_call(px_get_global("parse_type"), (LXValue[]){}, 0));
            px_srcline(1088);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        }
        px_srcline(1089);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1090);
            _v612 = px_call(px_get_global("parse_call_args"), (LXValue[]){}, 0);
            px_srcline(1091);
             _v611 = px_list_n((LXValue[]){px_str("Call"), _v611, _v612, _v608}, 4);
        }
        px_srcline(1092);
        return _v611;
    }
    px_srcline(1093);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("意外的 token: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1094);
    return px_null();
px_err_613:
    if (px_err_613_proped) return px_err_613_val;
    return px_null();
}

static LXValue fn_parse_list_or_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_list_or_comp");
    LXValue _v614 = px_uninit();
    LXValue _v615 = px_uninit();
    LXValue _v616 = px_uninit();
    LXValue _v617 = px_uninit();
    LXValue px_err_618_val = px_null();
    int px_err_618_proped = 0;
    px_srcline(1096);
    _v614 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1097);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1098);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1099);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
        px_srcline(1100);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1101);
        return px_list_n((LXValue[]){px_str("List"), px_list_n((LXValue[]){}, 0), _v614}, 3);
    }
    px_srcline(1102);
    _v615 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1103);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1104);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1105);
        _v616 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1106);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1107);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1108);
        return px_list_n((LXValue[]){px_str("ListComp"), _v615, px_index(_v616, px_int(0LL)), px_index(_v616, px_int(1LL)), _v614}, 5);
    }
    px_srcline(1109);
    _v617 = px_list_n((LXValue[]){_v615}, 1);
    px_srcline(1110);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1111);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1112);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1113);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1))) {
            px_srcline(1114);
            break;
        }
        px_srcline(1115);
        (void)(px_method(_v617, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        px_srcline(1116);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    }
    px_srcline(1117);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
    px_srcline(1118);
    return px_list_n((LXValue[]){px_str("List"), _v617, _v614}, 3);
px_err_618:
    if (px_err_618_proped) return px_err_618_val;
    return px_null();
}

static LXValue fn_parse_comp_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_vars");
    LXValue _v619 = px_uninit();
    LXValue px_err_620_val = px_null();
    int px_err_620_proped = 0;
    px_srcline(1120);
    _v619 = px_list_n((LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1);
    px_srcline(1121);
    while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1122);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1123);
        (void)(px_method(_v619, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("推导变量")}, 1)}, 1)}, 1));
    }
    px_srcline(1124);
    return _v619;
px_err_620:
    if (px_err_620_proped) return px_err_620_val;
    return px_null();
}

static LXValue fn_parse_comp_clauses(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_comp_clauses");
    LXValue _v621 = px_uninit();
    LXValue _v622 = px_uninit();
    LXValue _v623 = px_uninit();
    LXValue _v624 = px_uninit();
    LXValue px_err_625_val = px_null();
    int px_err_625_proped = 0;
    px_srcline(1126);
    _v621 = px_list_n((LXValue[]){}, 0);
    px_srcline(1127);
    _v622 = px_list_n((LXValue[]){}, 0);
    px_srcline(1128);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1129);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1130);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1131);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1132);
            _v623 = px_call(px_get_global("parse_comp_vars"), (LXValue[]){}, 0);
            px_srcline(1133);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("in"), px_str("'in'")}, 2));
            px_srcline(1134);
            _v624 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1135);
            (void)(px_method(_v621, "append", (LXValue[]){px_list_n((LXValue[]){px_str("CompClause"), _v623, _v624}, 3)}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1137);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1138);
            (void)(px_method(_v622, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
        }
        else {
            px_srcline(1140);
            break;
        }
    }
    px_srcline(1141);
    return px_list_n((LXValue[]){_v621, px_call(px_get_global("fold_comp_conds"), (LXValue[]){_v622}, 1)}, 2);
px_err_625:
    if (px_err_625_proped) return px_err_625_val;
    return px_null();
}

static LXValue fn_fold_comp_conds(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("fold_comp_conds");
    LXValue _v626 = (nargs > 0) ? args[0] : px_null();
    LXValue _v627 = px_uninit();
    LXValue _v628 = px_uninit();
    LXValue _v629 = px_uninit();
    LXValue px_err_630_val = px_null();
    int px_err_630_proped = 0;
    px_srcline(1143);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v626}, 1), px_int(0LL)))) {
        px_srcline(1144);
        return px_null();
    }
    px_srcline(1145);
    _v627 = px_index(_v626, px_int(0LL));
    px_srcline(1146);
    _v628 = px_int(1LL);
    px_srcline(1147);
    while (px_is_truthy(px_lt(_v628, px_call(px_get_global("len"), (LXValue[]){_v626}, 1)))) {
        px_srcline(1148);
        _v629 = px_call(px_get_global("node_pos"), (LXValue[]){_v627}, 1);
        px_srcline(1149);
         _v627 = px_list_n((LXValue[]){px_str("Binary"), px_str("And"), _v627, px_index(_v626, _v628), _v629}, 5);
        px_srcline(1150);
         _v628 = px_add(_v628, px_int(1LL));
    }
    px_srcline(1151);
    return _v627;
px_err_630:
    if (px_err_630_proped) return px_err_630_val;
    return px_null();
}

static LXValue fn_parse_paren_or_tuple(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_paren_or_tuple");
    LXValue _v631 = px_uninit();
    LXValue _v632 = px_uninit();
    LXValue _v633 = px_uninit();
    LXValue _v634 = px_uninit();
    LXValue px_err_635_val = px_null();
    int px_err_635_proped = 0;
    px_srcline(1153);
    _v631 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1154);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1155);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1156);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
        px_srcline(1157);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1158);
        return px_list_n((LXValue[]){px_str("Tuple"), px_list_n((LXValue[]){}, 0), _v631}, 3);
    }
    px_srcline(1159);
    _v632 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1160);
    (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
    px_srcline(1161);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
        px_srcline(1162);
        _v633 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
        px_srcline(1163);
        (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        px_srcline(1164);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1165);
        return px_list_n((LXValue[]){px_str("GenExp"), _v632, px_index(_v633, px_int(0LL)), px_index(_v633, px_int(1LL)), _v631}, 5);
    }
    px_srcline(1166);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
        px_srcline(1167);
        _v634 = px_list_n((LXValue[]){_v632}, 1);
        px_srcline(1168);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1169);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1170);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
            px_srcline(1171);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1))) {
                px_srcline(1172);
                break;
            }
            px_srcline(1173);
            (void)(px_method(_v634, "append", (LXValue[]){px_call(px_get_global("parse_expr"), (LXValue[]){}, 0)}, 1));
            px_srcline(1174);
            (void)(px_call(px_get_global("skip_expr_ws"), (LXValue[]){}, 0));
        }
        px_srcline(1175);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1176);
        return px_list_n((LXValue[]){px_str("Tuple"), _v634, _v631}, 3);
    }
    px_srcline(1177);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
    px_srcline(1178);
    return _v632;
px_err_635:
    if (px_err_635_proped) return px_err_635_val;
    return px_null();
}

static LXValue fn_brace_looks_like_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("brace_looks_like_dict");
    LXValue _v636 = px_uninit();
    LXValue _v637 = px_uninit();
    LXValue _v638 = px_uninit();
    LXValue px_err_639_val = px_null();
    int px_err_639_proped = 0;
    px_srcline(1180);
    _v636 = px_int(0LL);
    px_srcline(1181);
    _v637 = px_get_global("p_pos");
    px_srcline(1182);
    while (px_is_truthy(px_lt(_v637, px_call(px_get_global("len"), (LXValue[]){px_get_global("p_toks")}, 1)))) {
        px_srcline(1183);
        _v638 = px_index(px_index(px_get_global("p_toks"), _v637), px_int(0LL));
        px_srcline(1184);
        if (px_is_truthy(({ LXValue _t640 = px_eq(_v638, px_str(":")); px_is_truthy(_t640) ? px_eq(_v636, px_int(0LL)) : _t640; }))) {
            px_srcline(1185);
            return px_bool(true);
        }
        px_srcline(1186);
        if (px_is_truthy(({ LXValue _t642 = ({ LXValue _t641 = px_eq(_v638, px_str("(")); px_is_truthy(_t641) ? _t641 : px_eq(_v638, px_str("[")); }); px_is_truthy(_t642) ? _t642 : px_eq(_v638, px_str("{")); }))) {
            px_srcline(1187);
             _v636 = px_add(_v636, px_int(1LL));
        }
        else if (px_is_truthy(({ LXValue _t643 = px_eq(_v638, px_str(")")); px_is_truthy(_t643) ? _t643 : px_eq(_v638, px_str("]")); }))) {
            px_srcline(1189);
            if (px_is_truthy(px_gt(_v636, px_int(0LL)))) {
                px_srcline(1190);
                 _v636 = px_sub(_v636, px_int(1LL));
            }
        }
        else if (px_is_truthy(({ LXValue _t644 = px_eq(_v638, px_str("}")); px_is_truthy(_t644) ? px_eq(_v636, px_int(0LL)) : _t644; }))) {
            px_srcline(1192);
            return px_bool(false);
        }
        else if (px_is_truthy(({ LXValue _t647 = ({ LXValue _t646 = ({ LXValue _t645 = px_eq(_v638, px_str(",")); px_is_truthy(_t645) ? _t645 : px_eq(_v638, px_str("换行")); }); px_is_truthy(_t646) ? _t646 : px_eq(_v638, px_str("EOF")); }); px_is_truthy(_t647) ? px_eq(_v636, px_int(0LL)) : _t647; }))) {
            px_srcline(1194);
            return px_bool(false);
        }
        px_srcline(1195);
         _v637 = px_add(_v637, px_int(1LL));
    }
    px_srcline(1196);
    return px_bool(false);
px_err_639:
    if (px_err_639_proped) return px_err_639_val;
    return px_null();
}

static LXValue fn_parse_brace(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_brace");
    LXValue _v648 = px_uninit();
    LXValue _v649 = px_uninit();
    LXValue _v650 = px_uninit();
    LXValue _v651 = px_uninit();
    LXValue _v652 = px_uninit();
    LXValue _v653 = px_uninit();
    LXValue _v654 = px_uninit();
    LXValue _v655 = px_uninit();
    LXValue _v656 = px_uninit();
    LXValue px_err_657_val = px_null();
    int px_err_657_proped = 0;
    px_srcline(1198);
    _v648 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1199);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1200);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1201);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1212);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
        px_srcline(1213);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1214);
        return px_list_n((LXValue[]){px_str("Dict"), px_list_n((LXValue[]){}, 0), _v648}, 3);
    }
    px_srcline(1215);
    _v649 = px_call(px_get_global("brace_looks_like_dict"), (LXValue[]){}, 0);
    px_srcline(1216);
    if (px_is_truthy(_v649)) {
        px_srcline(1217);
        _v650 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1218);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1219);
        _v651 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        px_srcline(1220);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("for")}, 1))) {
            px_srcline(1221);
            _v652 = px_call(px_get_global("parse_comp_clauses"), (LXValue[]){}, 0);
            px_srcline(1222);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
            px_srcline(1223);
            return px_list_n((LXValue[]){px_str("DictComp"), _v650, _v651, px_index(_v652, px_int(0LL)), px_index(_v652, px_int(1LL)), _v648}, 6);
        }
        px_srcline(1224);
        _v653 = px_list_n((LXValue[]){px_list_n((LXValue[]){_v650, _v651}, 2)}, 1);
        px_srcline(1228);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1229);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1230);
        while (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
            px_srcline(1231);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1232);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1233);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
            px_srcline(1234);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1))) {
                px_srcline(1235);
                break;
            }
            px_srcline(1236);
            _v654 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1237);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
            px_srcline(1238);
            _v655 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
            px_srcline(1239);
            (void)(px_method(_v653, "append", (LXValue[]){px_list_n((LXValue[]){_v654, _v655}, 2)}, 1));
            px_srcline(1240);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1241);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1242);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1243);
        return px_list_n((LXValue[]){px_str("Dict"), _v653, _v648}, 3);
    }
    px_srcline(1244);
    _v656 = px_list_n((LXValue[]){}, 0);
    px_srcline(1245);
    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
    px_srcline(1246);
    (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    px_srcline(1247);
    while (px_is_truthy(({ LXValue _t658 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t658) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t658; }))) {
        px_srcline(1248);
        (void)(px_method(_v656, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
        px_srcline(1249);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1250);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
    }
    px_srcline(1251);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
    px_srcline(1252);
    return px_list_n((LXValue[]){px_str("Block"), _v656, _v648}, 3);
px_err_657:
    if (px_err_657_proped) return px_err_657_val;
    return px_null();
}

static LXValue fn_parse_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_closure");
    LXValue _v659 = px_uninit();
    LXValue _v660 = px_uninit();
    LXValue _v661 = px_uninit();
    LXValue _v662 = px_uninit();
    LXValue _v663 = px_uninit();
    LXValue _v664 = px_uninit();
    LXValue _v665 = px_uninit();
    LXValue px_err_666_val = px_null();
    int px_err_666_proped = 0;
    px_srcline(1254);
    _v659 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1255);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1256);
    _v660 = px_call(px_get_global("parse_params"), (LXValue[]){}, 0);
    px_srcline(1257);
    _v661 = px_null();
    px_srcline(1258);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
        px_srcline(1259);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1260);
         _v661 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
    }
    px_srcline(1261);
    _v662 = px_list_n((LXValue[]){}, 0);
    px_srcline(1262);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("capture")}, 1))) {
        px_srcline(1263);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1264);
        while (px_is_truthy(px_bool(true))) {
            px_srcline(1265);
            (void)(px_method(_v662, "append", (LXValue[]){px_call(px_get_global("qstr"), (LXValue[]){px_call(px_get_global("expect_ident"), (LXValue[]){px_str("捕获变量")}, 1)}, 1)}, 1));
            px_srcline(1266);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                px_srcline(1267);
                (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                px_srcline(1268);
                continue;
            }
            px_srcline(1269);
            break;
        }
    }
    px_srcline(1270);
    _v663 = px_null();
    px_srcline(1271);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("{")}, 1))) {
        px_srcline(1272);
        _v664 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1273);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1274);
        _v665 = px_list_n((LXValue[]){}, 0);
        px_srcline(1275);
        (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
        px_srcline(1276);
        (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        px_srcline(1277);
        while (px_is_truthy(({ LXValue _t667 = px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1)); px_is_truthy(_t667) ? px_not(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1)) : _t667; }))) {
            px_srcline(1278);
            (void)(px_method(_v665, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
            px_srcline(1279);
            (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
            px_srcline(1280);
            (void)(px_call(px_get_global("skip_brace_indents"), (LXValue[]){}, 0));
        }
        px_srcline(1281);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1282);
         _v663 = px_list_n((LXValue[]){px_str("Block"), _v665, _v664}, 3);
    }
    else if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(":")}, 1))) {
        px_srcline(1284);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1288);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("换行")}, 1))) {
            px_srcline(1289);
            _v664 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1290);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1291);
            if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
                px_srcline(1292);
                _v665 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
                px_srcline(1293);
                 _v663 = px_list_n((LXValue[]){px_str("Block"), _v665, _v664}, 3);
            }
            else {
                px_srcline(1300);
                _v665 = px_list_n((LXValue[]){}, 0);
                px_srcline(1301);
                px_set_global("p_paren_ctxt", px_add(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1302);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1303);
                    (void)(px_call(px_get_global("skip_newlines"), (LXValue[]){}, 0));
                    px_srcline(1304);
                    if (px_is_truthy(({ LXValue _t672 = ({ LXValue _t671 = ({ LXValue _t670 = ({ LXValue _t669 = ({ LXValue _t668 = px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1); px_is_truthy(_t668) ? _t668 : px_call(px_get_global("chk"), (LXValue[]){px_str("]")}, 1); }); px_is_truthy(_t669) ? _t669 : px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1); }); px_is_truthy(_t670) ? _t670 : px_call(px_get_global("chk"), (LXValue[]){px_str("}")}, 1); }); px_is_truthy(_t671) ? _t671 : px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); }); px_is_truthy(_t672) ? _t672 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
                        px_srcline(1305);
                        break;
                    }
                    px_srcline(1306);
                    (void)(px_method(_v665, "append", (LXValue[]){px_call(px_get_global("parse_stmt"), (LXValue[]){}, 0)}, 1));
                }
                px_srcline(1307);
                px_set_global("p_paren_ctxt", px_sub(px_get_global("p_paren_ctxt"), px_int(1LL)));
                px_srcline(1308);
                 _v663 = px_list_n((LXValue[]){px_str("Block"), _v665, _v664}, 3);
            }
        }
        else {
            px_srcline(1310);
             _v663 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
    }
    else {
        px_srcline(1312);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("匿名函数体期望 '{' 或 ':'")}, 2));
    }
    px_srcline(1313);
    return px_list_n((LXValue[]){px_str("Closure"), _v660, _v661, _v663, _v662, _v659}, 6);
px_err_666:
    if (px_err_666_proped) return px_err_666_val;
    return px_null();
}

static LXValue fn_parse_match_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_match_expr");
    LXValue _v673 = px_uninit();
    LXValue _v674 = px_uninit();
    LXValue _v675 = px_uninit();
    LXValue _v676 = px_uninit();
    LXValue _v677 = px_uninit();
    LXValue _v678 = px_uninit();
    LXValue _v679 = px_uninit();
    LXValue _v680 = px_uninit();
    LXValue px_err_681_val = px_null();
    int px_err_681_proped = 0;
    px_srcline(1315);
    _v673 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1316);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1317);
    _v674 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1318);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1319);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
    px_srcline(1320);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("缩进"), px_str("缩进块")}, 2));
    px_srcline(1321);
    _v675 = px_list_n((LXValue[]){}, 0);
    px_srcline(1322);
    while (px_is_truthy(px_bool(true))) {
        px_srcline(1323);
        (void)(px_call(px_get_global("skip_newlines_in_block"), (LXValue[]){}, 0));
        px_srcline(1324);
        if (px_is_truthy(({ LXValue _t682 = px_call(px_get_global("chk"), (LXValue[]){px_str("去缩进")}, 1); px_is_truthy(_t682) ? _t682 : px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1); }))) {
            px_srcline(1325);
            break;
        }
        px_srcline(1326);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("case"), px_str("'case'")}, 2));
        px_srcline(1327);
        _v676 = px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0);
        px_srcline(1328);
        _v677 = px_null();
        px_srcline(1329);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("if")}, 1))) {
            px_srcline(1330);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1331);
             _v677 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1332);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1333);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("换行"), px_str("换行")}, 2));
        px_srcline(1334);
        _v678 = px_null();
        px_srcline(1335);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("缩进")}, 1))) {
            px_srcline(1336);
            _v679 = px_call(px_get_global("parse_block"), (LXValue[]){}, 0);
            px_srcline(1337);
            _v680 = px_null();
            px_srcline(1338);
            if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v679}, 1), px_int(0LL)))) {
                px_srcline(1339);
                 _v680 = px_call(px_get_global("node_pos"), (LXValue[]){px_index(_v679, px_int(0LL))}, 1);
            }
            else {
                px_srcline(1341);
                 _v680 = _v673;
            }
            px_srcline(1342);
             _v678 = px_list_n((LXValue[]){px_str("Block"), _v679, _v680}, 3);
        }
        else {
            px_srcline(1344);
             _v678 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
        }
        px_srcline(1345);
        (void)(px_method(_v675, "append", (LXValue[]){px_list_n((LXValue[]){px_str("MatchArm"), _v676, _v677, _v678, _v673}, 5)}, 1));
    }
    px_srcline(1346);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("EOF")}, 1))) {
        px_srcline(1347);
        (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_str("match 表达式未正确结束")}, 2));
    }
    px_srcline(1348);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("去缩进"), px_str("去缩进")}, 2));
    px_srcline(1349);
    return px_list_n((LXValue[]){px_str("Match"), _v674, _v675, _v673}, 4);
px_err_681:
    if (px_err_681_proped) return px_err_681_val;
    return px_null();
}

static LXValue fn_parse_if_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_if_expr");
    LXValue _v683 = px_uninit();
    LXValue _v684 = px_uninit();
    LXValue _v685 = px_uninit();
    LXValue _v686 = px_uninit();
    LXValue px_err_687_val = px_null();
    int px_err_687_proped = 0;
    px_srcline(1351);
    _v683 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
    px_srcline(1352);
    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
    px_srcline(1353);
    _v684 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1354);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1355);
    _v685 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1356);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("else"), px_str("'else'")}, 2));
    px_srcline(1357);
    (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
    px_srcline(1358);
    _v686 = px_call(px_get_global("parse_expr"), (LXValue[]){}, 0);
    px_srcline(1359);
    return px_list_n((LXValue[]){px_str("IfExpr"), _v684, _v685, _v686, _v683}, 5);
px_err_687:
    if (px_err_687_proped) return px_err_687_val;
    return px_null();
}

static LXValue fn_parse_pattern(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_pattern");
    LXValue _v688 = px_uninit();
    LXValue _v689 = px_uninit();
    LXValue _v690 = px_uninit();
    LXValue _v691 = px_uninit();
    LXValue _v692 = px_uninit();
    LXValue _v693 = px_uninit();
    LXValue _v694 = px_uninit();
    LXValue _v695 = px_uninit();
    LXValue px_err_696_val = px_null();
    int px_err_696_proped = 0;
    px_srcline(1362);
    _v688 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1363);
    if (px_is_truthy(({ LXValue _t701 = ({ LXValue _t700 = ({ LXValue _t699 = ({ LXValue _t698 = ({ LXValue _t697 = px_eq(_v688, px_str("整数")); px_is_truthy(_t697) ? _t697 : px_eq(_v688, px_str("浮点")); }); px_is_truthy(_t698) ? _t698 : px_eq(_v688, px_str("字符串")); }); px_is_truthy(_t699) ? _t699 : px_eq(_v688, px_str("true")); }); px_is_truthy(_t700) ? _t700 : px_eq(_v688, px_str("false")); }); px_is_truthy(_t701) ? _t701 : px_eq(_v688, px_str("null")); }))) {
        px_srcline(1364);
        _v689 = px_call(px_get_global("parse_primary"), (LXValue[]){}, 0);
        px_srcline(1365);
        return px_list_n((LXValue[]){px_str("PatLiteral"), _v689}, 2);
    }
    px_srcline(1366);
    if (px_is_truthy(px_eq(_v688, px_str("标识符")))) {
        px_srcline(1367);
        _v690 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1368);
        _v691 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1369);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1370);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(".")}, 1))) {
            px_srcline(1372);
            _v692 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
            px_srcline(1373);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1374);
            _v693 = px_call(px_get_global("expect_name"), (LXValue[]){px_str("成员名"), px_bool(true)}, 2);
            px_srcline(1375);
            return px_list_n((LXValue[]){px_str("PatLiteral"), ({ LXValue _s79 = px_list_n((LXValue[]){px_str("Var"), px_call(px_get_global("qstr"), (LXValue[]){_v690}, 1), _v692}, 3); LXValue _s80 = px_call(px_get_global("qstr"), (LXValue[]){_v693}, 1); px_list_n((LXValue[]){px_str("Field"), _s79, _s80, _v692}, 4); })}, 2);
        }
        px_srcline(1376);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("(")}, 1))) {
            px_srcline(1377);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1378);
            _v694 = px_list_n((LXValue[]){}, 0);
            px_srcline(1379);
            if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
                px_srcline(1380);
                while (px_is_truthy(px_bool(true))) {
                    px_srcline(1381);
                    (void)(px_method(_v694, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                    px_srcline(1382);
                    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                        px_srcline(1383);
                        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                        px_srcline(1384);
                        continue;
                    }
                    px_srcline(1385);
                    break;
                }
            }
            px_srcline(1386);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
            px_srcline(1387);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v690}, 1), _v694}, 3);
        }
        px_srcline(1388);
        if (px_is_truthy(px_eq(_v690, px_str("_")))) {
            px_srcline(1389);
            return px_list_n((LXValue[]){px_str("PatWildcard")}, 1);
        }
        px_srcline(1390);
        if (px_is_truthy(px_call(px_get_global("is_upper"), (LXValue[]){_v690}, 1))) {
            px_srcline(1391);
            return px_list_n((LXValue[]){px_str("PatConstructor"), px_call(px_get_global("qstr"), (LXValue[]){_v690}, 1), px_list_n((LXValue[]){}, 0)}, 3);
        }
        px_srcline(1392);
        return px_list_n((LXValue[]){px_str("PatBinding"), px_call(px_get_global("qstr"), (LXValue[]){_v690}, 1)}, 2);
    }
    px_srcline(1393);
    if (px_is_truthy(px_eq(_v688, px_str("(")))) {
        px_srcline(1394);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1395);
        _v695 = px_list_n((LXValue[]){}, 0);
        px_srcline(1396);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1397);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1398);
                (void)(px_method(_v695, "append", (LXValue[]){px_call(px_get_global("parse_pattern"), (LXValue[]){}, 0)}, 1));
                px_srcline(1399);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1400);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1401);
                    continue;
                }
                px_srcline(1402);
                break;
            }
        }
        px_srcline(1403);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1404);
        return px_list_n((LXValue[]){px_str("PatTuple"), _v695}, 2);
    }
    px_srcline(1405);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的模式: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1406);
    return px_null();
px_err_696:
    if (px_err_696_proped) return px_err_696_val;
    return px_null();
}

static LXValue fn_is_upper(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("is_upper");
    LXValue _v702 = (nargs > 0) ? args[0] : px_null();
    LXValue _v703 = px_uninit();
    LXValue px_err_704_val = px_null();
    int px_err_704_proped = 0;
    px_srcline(1408);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v702}, 1), px_int(0LL)))) {
        px_srcline(1409);
        return px_bool(false);
    }
    px_srcline(1410);
    _v703 = px_index(_v702, px_int(0LL));
    px_srcline(1411);
    return ({ LXValue _t705 = px_ge(_v703, px_str("A")); px_is_truthy(_t705) ? px_le(_v703, px_str("Z")) : _t705; });
px_err_704:
    if (px_err_704_proped) return px_err_704_val;
    return px_null();
}

static LXValue fn_parse_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type");
    LXValue _v706 = px_uninit();
    LXValue _v707 = px_uninit();
    LXValue px_err_708_val = px_null();
    int px_err_708_proped = 0;
    px_srcline(1414);
    _v706 = px_call(px_get_global("parse_type_base"), (LXValue[]){}, 0);
    px_srcline(1415);
    if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("?")}, 1))) {
        px_srcline(1416);
        _v707 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1417);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1418);
        return px_list_n((LXValue[]){px_str("TyOptional"), _v706, _v707}, 3);
    }
    px_srcline(1419);
    return _v706;
px_err_708:
    if (px_err_708_proped) return px_err_708_val;
    return px_null();
}

static LXValue fn_parse_type_base(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("parse_type_base");
    LXValue _v709 = px_uninit();
    LXValue _v710 = px_uninit();
    LXValue _v711 = px_uninit();
    LXValue _v712 = px_uninit();
    LXValue _v713 = px_uninit();
    LXValue _v714 = px_uninit();
    LXValue _v715 = px_uninit();
    LXValue _v716 = px_uninit();
    LXValue px_err_717_val = px_null();
    int px_err_717_proped = 0;
    px_srcline(1421);
    _v709 = px_call(px_get_global("pk"), (LXValue[]){}, 0);
    px_srcline(1422);
    if (px_is_truthy(px_eq(_v709, px_str("标识符")))) {
        px_srcline(1423);
        _v710 = px_call(px_get_global("pv"), (LXValue[]){}, 0);
        px_srcline(1424);
        _v711 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1425);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1426);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("[")}, 1))) {
            px_srcline(1427);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1428);
            _v712 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1429);
            (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
            px_srcline(1430);
            if (px_is_truthy(px_eq(_v710, px_str("list")))) {
                px_srcline(1431);
                return px_list_n((LXValue[]){px_str("TyList"), _v712, _v711}, 3);
            }
            px_srcline(1432);
            return px_list_n((LXValue[]){px_str("TyGeneric"), px_call(px_get_global("qstr"), (LXValue[]){_v710}, 1), px_list_n((LXValue[]){_v712}, 1), _v711}, 4);
        }
        px_srcline(1433);
        return px_list_n((LXValue[]){px_str("TyNamed"), px_call(px_get_global("qstr"), (LXValue[]){_v710}, 1), _v711}, 3);
    }
    px_srcline(1434);
    if (px_is_truthy(px_eq(_v709, px_str("[")))) {
        px_srcline(1435);
        _v711 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1436);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1437);
        _v712 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1438);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("]"), px_str("']'")}, 2));
        px_srcline(1439);
        return px_list_n((LXValue[]){px_str("TyList"), _v712, _v711}, 3);
    }
    px_srcline(1440);
    if (px_is_truthy(px_eq(_v709, px_str("{")))) {
        px_srcline(1441);
        _v711 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1442);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1443);
        _v713 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1444);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(":"), px_str("':'")}, 2));
        px_srcline(1445);
        _v714 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
        px_srcline(1446);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str("}"), px_str("'}'")}, 2));
        px_srcline(1447);
        return px_list_n((LXValue[]){px_str("TyDict"), _v713, _v714, _v711}, 4);
    }
    px_srcline(1448);
    if (px_is_truthy(px_eq(_v709, px_str("(")))) {
        px_srcline(1449);
        _v711 = px_call(px_get_global("ppos"), (LXValue[]){}, 0);
        px_srcline(1450);
        (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
        px_srcline(1451);
        _v715 = px_list_n((LXValue[]){}, 0);
        px_srcline(1452);
        if (px_is_truthy(px_not(px_call(px_get_global("chk"), (LXValue[]){px_str(")")}, 1)))) {
            px_srcline(1453);
            while (px_is_truthy(px_bool(true))) {
                px_srcline(1454);
                (void)(px_method(_v715, "append", (LXValue[]){px_call(px_get_global("parse_type"), (LXValue[]){}, 0)}, 1));
                px_srcline(1455);
                if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str(",")}, 1))) {
                    px_srcline(1456);
                    (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
                    px_srcline(1457);
                    continue;
                }
                px_srcline(1458);
                break;
            }
        }
        px_srcline(1459);
        (void)(px_call(px_get_global("expect"), (LXValue[]){px_str(")"), px_str("')'")}, 2));
        px_srcline(1460);
        if (px_is_truthy(px_call(px_get_global("chk"), (LXValue[]){px_str("->")}, 1))) {
            px_srcline(1461);
            (void)(px_call(px_get_global("adv"), (LXValue[]){}, 0));
            px_srcline(1462);
            _v716 = px_call(px_get_global("parse_type"), (LXValue[]){}, 0);
            px_srcline(1463);
            return px_list_n((LXValue[]){px_str("TyFunc"), _v715, _v716, _v711}, 4);
        }
        px_srcline(1464);
        return px_list_n((LXValue[]){px_str("TyTuple"), _v715, _v711}, 3);
    }
    px_srcline(1465);
    (void)(px_call(px_get_global("perr"), (LXValue[]){px_str("E2001"), px_add(px_str("无效的类型: "), px_call(px_get_global("pk_display"), (LXValue[]){}, 0))}, 2));
    px_srcline(1466);
    return px_null();
px_err_717:
    if (px_err_717_proped) return px_err_717_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt_inner");
    LXValue _v718 = (nargs > 0) ? args[0] : px_null();
    LXValue _v719 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v777 = px_uninit();
    LXValue _v778 = px_uninit();
    LXValue _v779 = px_uninit();
    LXValue _v780 = px_uninit();
    LXValue _v781 = px_uninit();
    LXValue px_err_782_val = px_null();
    int px_err_782_proped = 0;
    px_srcline(9);
    _v720 = px_call(px_get_global("cg_pad"), (LXValue[]){_v719}, 1);
    px_srcline(10);
    _v721 = px_index(_v718, px_int(0LL));
    px_srcline(11);
    if (px_is_truthy(px_eq(_v721, px_str("VarDecl")))) {
        px_srcline(12);
        _v722 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v718, px_int(2LL))}, 1);
        px_srcline(15);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v718}, 1));
        px_srcline(16);
        _v723 = px_str("px_null()");
        px_srcline(17);
        if (px_is_truthy(px_ne(px_index(_v718, px_int(4LL)), px_null()))) {
            px_srcline(18);
             _v723 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(4LL))}, 1);
        }
        px_srcline(23);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(26);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(27);
            return px_add(px_add(px_add(px_add(px_add(_v720, px_str("px_set_global(\"")), _v722), px_str("\", ")), _v723), px_str(");\n"));
        }
        px_srcline(28);
        _v724 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v722}, 1);
        px_srcline(29);
        if (px_is_truthy(px_eq(_v724, px_null()))) {
            px_srcline(31);
             _v724 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v722}, 1);
            px_srcline(33);
            if (px_is_truthy(px_ne(px_index(_v718, px_int(4LL)), px_null()))) {
                px_srcline(34);
                _v725 = px_index(_v718, px_int(4LL));
                px_srcline(35);
                _v726 = px_null();
                px_srcline(36);
                if (px_is_truthy(px_eq(px_index(_v725, px_int(0LL)), px_str("Constructor")))) {
                    px_srcline(37);
                     _v726 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v725, px_int(1LL))}, 1);
                }
                else if (px_is_truthy(px_eq(px_index(_v725, px_int(0LL)), px_str("Call")))) {
                    px_srcline(39);
                    _v727 = px_index(_v725, px_int(1LL));
                    px_srcline(40);
                    if (px_is_truthy(px_eq(px_index(_v727, px_int(0LL)), px_str("Var")))) {
                        px_srcline(41);
                         _v726 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v727, px_int(1LL))}, 1);
                    }
                }
                px_srcline(42);
                if (px_is_truthy(px_ne(_v726, px_null()))) {
                    px_srcline(43);
                    if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v726}, 1))) {
                        px_srcline(44);
                        px_index_set(px_get_global("cg_var_types"), _v722, _v726);
                    }
                }
            }
            px_srcline(45);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v722}, 1))) {
                px_srcline(46);
                (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v722}, 1));
                px_srcline(47);
                return px_add(px_add(px_add(px_add(px_add(_v720, px_str("LXValue ")), _v724), px_str(" = px_cell(")), _v723), px_str(");\n"));
            }
            px_srcline(48);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v722}, 1));
            px_srcline(49);
            return px_add(px_add(px_add(px_add(px_add(_v720, px_str("LXValue ")), _v724), px_str(" = ")), _v723), px_str(";\n"));
        }
        px_srcline(52);
        if (px_is_truthy(px_ne(px_index(_v718, px_int(4LL)), px_null()))) {
            px_srcline(53);
            _v725 = px_index(_v718, px_int(4LL));
            px_srcline(54);
            _v726 = px_null();
            px_srcline(55);
            if (px_is_truthy(px_eq(px_index(_v725, px_int(0LL)), px_str("Constructor")))) {
                px_srcline(56);
                 _v726 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v725, px_int(1LL))}, 1);
            }
            else if (px_is_truthy(px_eq(px_index(_v725, px_int(0LL)), px_str("Call")))) {
                px_srcline(58);
                _v727 = px_index(_v725, px_int(1LL));
                px_srcline(59);
                if (px_is_truthy(px_eq(px_index(_v727, px_int(0LL)), px_str("Var")))) {
                    px_srcline(60);
                     _v726 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v727, px_int(1LL))}, 1);
                }
            }
            px_srcline(61);
            if (px_is_truthy(px_ne(_v726, px_null()))) {
                px_srcline(62);
                if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v726}, 1))) {
                    px_srcline(63);
                    px_index_set(px_get_global("cg_var_types"), _v722, _v726);
                }
            }
        }
        px_srcline(64);
        _v728 = px_add(px_add(_v720, px_call(px_get_global("cg_store_of"), (LXValue[]){_v722, _v724, _v723}, 3)), px_str(";\n"));
        px_srcline(66);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v722}, 1));
        px_srcline(67);
        return _v728;
    }
    px_srcline(68);
    if (px_is_truthy(px_eq(_v721, px_str("Assign")))) {
        px_srcline(69);
        _v723 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(3LL))}, 1);
        px_srcline(70);
        _v729 = px_index(_v718, px_int(1LL));
        px_srcline(71);
        _v730 = px_index(_v718, px_int(2LL));
        px_srcline(73);
        if (px_is_truthy(px_eq(_v730, px_str("Append")))) {
            px_srcline(74);
            _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v729}, 1);
            px_srcline(75);
            _v732 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){_v729, px_index(_v718, px_int(3LL))}, 2), px_list_n((LXValue[]){_v731, _v723}, 2)}, 2);
            px_srcline(76);
            _v733 = px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v732, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_method("), px_index(px_index(_v732, px_int(1LL)), px_int(0LL))), px_str(", \"append\", (LXValue[]){")), px_index(px_index(_v732, px_int(1LL)), px_int(1LL))), px_str("}, 1)"))}, 2);
            px_srcline(77);
            return px_add(px_add(px_add(_v720, px_str("(void)(")), _v733), px_str(");\n"));
        }
        px_srcline(78);
        _v734 = px_index(_v729, px_int(0LL));
        px_srcline(79);
        if (px_is_truthy(px_eq(_v734, px_str("Var")))) {
            px_srcline(80);
            _v722 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v729, px_int(1LL))}, 1);
            px_srcline(82);
            (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v729, _v730, px_index(_v718, px_int(3LL))}, 3));
            px_srcline(83);
            _v735 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v722}, 1);
            px_srcline(84);
            if (px_is_truthy(px_eq(_v735, px_null()))) {
                px_srcline(86);
                if (px_is_truthy(px_eq(_v730, px_str("Assign")))) {
                    px_srcline(87);
                    return px_add(px_add(px_add(px_add(px_add(_v720, px_str("px_set_global(\"")), _v722), px_str("\", ")), _v723), px_str(");\n"));
                }
                px_srcline(88);
                _v736 = px_call(px_get_global("cg_assign_op_global"), (LXValue[]){_v730, _v722, _v723}, 3);
                px_srcline(89);
                return px_add(px_add(px_add(px_add(px_add(_v720, px_str("px_set_global(\"")), _v722), px_str("\", ")), _v736), px_str(");\n"));
            }
            px_srcline(90);
            _v736 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v730, px_call(px_get_global("cg_load_ck"), (LXValue[]){_v722, _v735}, 2), _v723}, 3);
            px_srcline(91);
            _v737 = px_add(px_add(px_add(_v720, px_str(" ")), px_call(px_get_global("cg_store_of"), (LXValue[]){_v722, _v735, _v736}, 3)), px_str(";\n"));
            px_srcline(93);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v722}, 1));
            px_srcline(94);
            return _v737;
        }
        px_srcline(95);
        if (px_is_truthy(px_eq(_v734, px_str("Field")))) {
            px_srcline(96);
            _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v729, px_int(1LL))}, 1);
            px_srcline(97);
            _v738 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v729, px_int(2LL))}, 1);
            px_srcline(98);
            if (px_is_truthy(px_ne(_v730, px_str("Assign")))) {
                px_srcline(103);
                _v739 = px_call(px_get_global("cg_seq_new"), (LXValue[]){}, 0);
                px_srcline(104);
                _v736 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v730, px_add(px_add(px_add(px_add(px_str("px_field("), _v739), px_str(", \"")), _v738), px_str("\")")), _v723}, 3);
                px_srcline(105);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v720, px_str("({ LXValue ")), _v739), px_str(" = ")), _v731), px_str("; px_field_set(")), _v739), px_str(", \"")), _v738), px_str("\", ")), _v736), px_str("); });\n"));
            }
            px_srcline(106);
            _v732 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v729, px_int(1LL)), px_index(_v718, px_int(3LL))}, 2), px_list_n((LXValue[]){_v731, _v723}, 2)}, 2);
            px_srcline(107);
            return px_add(px_add(_v720, px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v732, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_field_set("), px_index(px_index(_v732, px_int(1LL)), px_int(0LL))), px_str(", \"")), _v738), px_str("\", ")), px_index(px_index(_v732, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2)), px_str(";\n"));
        }
        px_srcline(108);
        if (px_is_truthy(px_eq(_v734, px_str("Index")))) {
            px_srcline(109);
            _v731 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v729, px_int(1LL))}, 1);
            px_srcline(110);
            _v740 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v729, px_int(2LL))}, 1);
            px_srcline(111);
            if (px_is_truthy(px_ne(_v730, px_str("Assign")))) {
                px_srcline(114);
                _v739 = px_call(px_get_global("cg_seq_new"), (LXValue[]){}, 0);
                px_srcline(115);
                _v741 = px_call(px_get_global("cg_seq_new"), (LXValue[]){}, 0);
                px_srcline(116);
                _v736 = px_call(px_get_global("cg_assign_op_local"), (LXValue[]){_v730, px_add(px_add(px_add(px_add(px_str("px_index("), _v739), px_str(", ")), _v741), px_str(")")), _v723}, 3);
                px_srcline(117);
                return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v720, px_str("({ LXValue ")), _v739), px_str(" = ")), _v731), px_str("; LXValue ")), _v741), px_str(" = ")), _v740), px_str("; px_index_set(")), _v739), px_str(", ")), _v741), px_str(", ")), _v736), px_str("); });\n"));
            }
            px_srcline(118);
            _v732 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v729, px_int(1LL)), px_index(_v729, px_int(2LL)), px_index(_v718, px_int(3LL))}, 3), px_list_n((LXValue[]){_v731, _v740, _v723}, 3)}, 2);
            px_srcline(119);
            return px_add(px_add(_v720, px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v732, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_index_set("), px_index(px_index(_v732, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v732, px_int(1LL)), px_int(1LL))), px_str(", ")), px_index(px_index(_v732, px_int(1LL)), px_int(2LL))), px_str(")"))}, 2)), px_str(";\n"));
        }
        px_srcline(120);
        return px_str("不支持的赋值目标");
    }
    px_srcline(121);
    if (px_is_truthy(px_eq(_v721, px_str("ExprStmt")))) {
        px_srcline(122);
        _v725 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(123);
        return px_add(px_add(px_add(_v720, px_str("(void)(")), _v725), px_str(");\n"));
    }
    px_srcline(124);
    if (px_is_truthy(px_eq(_v721, px_str("If")))) {
        px_srcline(125);
        _v742 = px_str("");
        px_srcline(126);
        _v743 = px_index(_v718, px_int(1LL));
        px_srcline(129);
        _v744 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
        px_srcline(130);
        _v745 = px_null();
        px_srcline(131);
        _v746 = px_int(0LL);
        px_srcline(132);
        while (px_is_truthy(px_lt(_v746, px_call(px_get_global("len"), (LXValue[]){_v743}, 1)))) {
            px_srcline(133);
            (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v744}, 1));
            px_srcline(134);
            _v747 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v743, _v746), px_int(0LL))}, 1);
            px_srcline(135);
            _v748 = px_str("if");
            px_srcline(136);
            if (px_is_truthy(px_gt(_v746, px_int(0LL)))) {
                px_srcline(137);
                 _v748 = px_str("else if");
            }
            px_srcline(138);
             _v742 = px_add(_v742, px_add(px_add(px_add(px_add(_v720, _v748), px_str(" (px_is_truthy(")), _v747), px_str(")) {\n")));
            px_srcline(139);
            _v749 = px_index(px_index(_v743, _v746), px_int(1LL));
            px_srcline(140);
            _v750 = px_int(0LL);
            px_srcline(141);
            while (px_is_truthy(px_lt(_v750, px_call(px_get_global("len"), (LXValue[]){_v749}, 1)))) {
                px_srcline(142);
                 _v742 = px_add(_v742, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v749, _v750), px_add(_v719, px_int(1LL))}, 2));
                px_srcline(143);
                 _v750 = px_add(_v750, px_int(1LL));
            }
            px_srcline(144);
             _v742 = px_add(_v742, px_add(_v720, px_str("}\n")));
            px_srcline(145);
            if (px_is_truthy(px_eq(_v745, px_null()))) {
                px_srcline(146);
                 _v745 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
            }
            else {
                px_srcline(148);
                 _v745 = px_call(px_get_global("cg_inited_intersect"), (LXValue[]){_v745, px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0)}, 2);
            }
            px_srcline(149);
             _v746 = px_add(_v746, px_int(1LL));
        }
        px_srcline(150);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v744}, 1));
        px_srcline(151);
        if (px_is_truthy(px_ne(px_index(_v718, px_int(2LL)), px_null()))) {
            px_srcline(152);
             _v742 = px_add(_v742, px_add(_v720, px_str("else {\n")));
            px_srcline(153);
            _v751 = px_index(_v718, px_int(2LL));
            px_srcline(154);
            _v752 = px_int(0LL);
            px_srcline(155);
            while (px_is_truthy(px_lt(_v752, px_call(px_get_global("len"), (LXValue[]){_v751}, 1)))) {
                px_srcline(156);
                 _v742 = px_add(_v742, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v751, _v752), px_add(_v719, px_int(1LL))}, 2));
                px_srcline(157);
                 _v752 = px_add(_v752, px_int(1LL));
            }
            px_srcline(158);
             _v742 = px_add(_v742, px_add(_v720, px_str("}\n")));
            px_srcline(159);
             _v745 = px_call(px_get_global("cg_inited_intersect"), (LXValue[]){_v745, px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0)}, 2);
        }
        else {
            px_srcline(161);
             _v745 = px_call(px_get_global("cg_inited_intersect"), (LXValue[]){_v745, _v744}, 2);
        }
        px_srcline(162);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v745}, 1));
        px_srcline(163);
        return _v742;
    }
    px_srcline(164);
    if (px_is_truthy(px_eq(_v721, px_str("While")))) {
        px_srcline(166);
        _v753 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
        px_srcline(167);
        _v747 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(168);
        _v742 = px_add(px_add(px_add(_v720, px_str("while (px_is_truthy(")), _v747), px_str(")) {\n"));
        px_srcline(169);
        _v749 = px_index(_v718, px_int(2LL));
        px_srcline(170);
        _v746 = px_int(0LL);
        px_srcline(171);
        while (px_is_truthy(px_lt(_v746, px_call(px_get_global("len"), (LXValue[]){_v749}, 1)))) {
            px_srcline(172);
             _v742 = px_add(_v742, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v749, _v746), px_add(_v719, px_int(1LL))}, 2));
            px_srcline(173);
             _v746 = px_add(_v746, px_int(1LL));
        }
        px_srcline(174);
         _v742 = px_add(_v742, px_add(_v720, px_str("}\n")));
        px_srcline(175);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v753}, 1));
        px_srcline(176);
        return _v742;
    }
    px_srcline(177);
    if (px_is_truthy(px_eq(_v721, px_str("For")))) {
        px_srcline(180);
        _v754 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
        px_srcline(181);
        _v755 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(2LL))}, 1);
        px_srcline(182);
        _v756 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(183);
        _v757 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(185);
        _v758 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(186);
        _v759 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v758}, 1), px_int(1LL));
        px_srcline(187);
        _v760 = px_str("");
        px_srcline(188);
        _v761 = px_int(0LL);
        px_srcline(189);
        if (px_is_truthy(_v759)) {
            px_srcline(190);
             _v760 = px_call(px_get_global("cg_unpack_tmp"), (LXValue[]){}, 0);
            px_srcline(191);
             _v761 = px_call(px_get_global("len"), (LXValue[]){_v758}, 1);
        }
        px_srcline(193);
        _v762 = px_list_n((LXValue[]){}, 0);
        px_srcline(199);
        _v763 = px_int(0LL);
        px_srcline(200);
        while (px_is_truthy(px_lt(_v763, px_call(px_get_global("len"), (LXValue[]){_v758}, 1)))) {
            px_srcline(201);
            _v764 = px_index(_v758, _v763);
            px_srcline(202);
            _v765 = px_str("");
            px_srcline(203);
            if (px_is_truthy(_v759)) {
                px_srcline(205);
                 _v765 = px_add(px_add(px_add(px_add(px_str("px_index("), _v760), px_str(", px_int(")), px_call(px_get_global("str"), (LXValue[]){_v763}, 1)), px_str("))"));
            }
            else {
                px_srcline(209);
                 _v765 = px_add(px_add(px_add(px_add(px_str("px_iter_at("), _v756), px_str(", px_int(")), _v757), px_str("))"));
            }
            px_srcline(210);
            if (px_is_truthy(px_get_global("cg_topbody"))) {
                px_srcline(212);
                (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
                px_srcline(213);
                (void)(px_method(_v762, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("px_set_global(\""), _v764), px_str("\", ")), _v765), px_str(");"))}, 1));
                px_srcline(214);
                 _v763 = px_add(_v763, px_int(1LL));
                px_srcline(215);
                continue;
            }
            px_srcline(216);
            _v766 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v764}, 1);
            px_srcline(217);
            _v767 = px_str("LXValue ");
            px_srcline(218);
            if (px_is_truthy(px_eq(_v766, px_null()))) {
                px_srcline(220);
                 _v766 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v764}, 1);
            }
            else {
                px_srcline(223);
                 _v767 = px_str("");
            }
            px_srcline(224);
            if (px_is_truthy(px_eq(_v767, px_str("LXValue ")))) {
                px_srcline(226);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v764}, 1))) {
                    px_srcline(227);
                    (void)(px_method(_v762, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("LXValue "), _v766), px_str(" = px_cell(")), _v765), px_str(");"))}, 1));
                }
                else {
                    px_srcline(229);
                    (void)(px_method(_v762, "append", (LXValue[]){px_add(px_add(px_add(px_add(px_str("LXValue "), _v766), px_str(" = ")), _v765), px_str(";"))}, 1));
                }
            }
            else {
                px_srcline(232);
                (void)(px_method(_v762, "append", (LXValue[]){px_add(px_call(px_get_global("cg_store_of"), (LXValue[]){_v764, _v766, _v765}, 3), px_str(";"))}, 1));
            }
            px_srcline(233);
             _v763 = px_add(_v763, px_int(1LL));
        }
        px_srcline(234);
        _v742 = px_add(px_add(px_add(px_add(px_add(_v720, px_str("LXValue ")), _v756), px_str(" = ")), _v755), px_str(";\n"));
        px_srcline(237);
        _v768 = px_call(px_get_global("cg_iter_len_tmp"), (LXValue[]){}, 0);
        px_srcline(238);
         _v742 = px_add(_v742, px_add(px_add(px_add(px_add(px_add(_v720, px_str("int ")), _v768), px_str(" = px_iter_prepare(")), _v756), px_str(");\n")));
        px_srcline(239);
         _v742 = px_add(_v742, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v720, px_str("for (int ")), _v757), px_str(" = 0; ")), _v757), px_str(" < ")), _v768), px_str("; ")), _v757), px_str("++) {\n")));
        px_srcline(240);
         _v742 = px_add(_v742, px_add(px_add(px_add(px_add(px_add(_v720, px_str("    px_iter_ck(")), _v756), px_str(", ")), _v768), px_str(");\n")));
        px_srcline(241);
        if (px_is_truthy(_v759)) {
            px_srcline(246);
             _v742 = px_add(_v742, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v720, px_str("    LXValue ")), _v760), px_str(" = px_iter_at(")), _v756), px_str(", px_int(")), _v757), px_str("));\n")));
            px_srcline(247);
             _v742 = px_add(_v742, px_add(px_add(px_add(px_add(px_add(_v720, px_str("    px_unpack_ck(")), _v760), px_str(", ")), px_call(px_get_global("str"), (LXValue[]){_v761}, 1)), px_str(");\n")));
            px_srcline(248);
            _v769 = px_int(0LL);
            px_srcline(249);
            while (px_is_truthy(px_lt(_v769, px_call(px_get_global("len"), (LXValue[]){_v762}, 1)))) {
                px_srcline(250);
                 _v742 = px_add(_v742, px_add(px_add(px_add(_v720, px_str("    ")), px_index(_v762, _v769)), px_str("\n")));
                px_srcline(251);
                 _v769 = px_add(_v769, px_int(1LL));
            }
        }
        else {
            px_srcline(253);
             _v742 = px_add(_v742, px_add(px_add(px_add(_v720, px_str("    ")), px_index(_v762, px_int(0LL))), px_str("\n")));
        }
        px_srcline(254);
        _v749 = px_index(_v718, px_int(3LL));
        px_srcline(256);
        if (px_is_truthy(px_not(px_get_global("cg_topbody")))) {
            px_srcline(257);
            _v770 = px_int(0LL);
            px_srcline(258);
            while (px_is_truthy(px_lt(_v770, px_call(px_get_global("len"), (LXValue[]){_v758}, 1)))) {
                px_srcline(259);
                (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){px_index(_v758, _v770)}, 1));
                px_srcline(260);
                 _v770 = px_add(_v770, px_int(1LL));
            }
        }
        px_srcline(261);
        _v746 = px_int(0LL);
        px_srcline(262);
        while (px_is_truthy(px_lt(_v746, px_call(px_get_global("len"), (LXValue[]){_v749}, 1)))) {
            px_srcline(263);
             _v742 = px_add(_v742, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v749, _v746), px_add(_v719, px_int(1LL))}, 2));
            px_srcline(264);
             _v746 = px_add(_v746, px_int(1LL));
        }
        px_srcline(265);
         _v742 = px_add(_v742, px_add(_v720, px_str("}\n")));
        px_srcline(266);
        (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v754}, 1));
        px_srcline(267);
        return _v742;
    }
    px_srcline(268);
    if (px_is_truthy(px_eq(_v721, px_str("Return")))) {
        px_srcline(269);
        if (px_is_truthy(px_ne(px_index(_v718, px_int(1LL)), px_null()))) {
            px_srcline(270);
            _v725 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
            px_srcline(271);
            return px_add(px_add(px_add(_v720, px_str("return ")), _v725), px_str(";\n"));
        }
        px_srcline(272);
        return px_add(_v720, px_str("return px_null();\n"));
    }
    px_srcline(273);
    if (px_is_truthy(px_eq(_v721, px_str("Break")))) {
        px_srcline(274);
        return px_add(_v720, px_str("break;\n"));
    }
    px_srcline(275);
    if (px_is_truthy(px_eq(_v721, px_str("Continue")))) {
        px_srcline(276);
        return px_add(_v720, px_str("continue;\n"));
    }
    px_srcline(277);
    if (px_is_truthy(px_eq(_v721, px_str("Empty")))) {
        px_srcline(278);
        return px_str("");
    }
    px_srcline(279);
    if (px_is_truthy(px_eq(_v721, px_str("ChanDecl")))) {
        px_srcline(281);
        _v771 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(282);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(283);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(284);
            return px_add(px_add(px_add(_v720, px_str("px_set_global(\"")), _v771), px_str("\", px_chan_create(0));\n"));
        }
        px_srcline(285);
        _v735 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v771}, 1);
        px_srcline(287);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v771}, 1));
        px_srcline(288);
        return px_add(px_add(px_add(_v720, px_str("LXValue ")), _v735), px_str(" = px_chan_create(0);\n"));
    }
    px_srcline(289);
    if (px_is_truthy(px_eq(_v721, px_str("Send")))) {
        px_srcline(290);
        _v747 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(291);
        _v735 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(2LL))}, 1);
        px_srcline(292);
        return px_add(px_add(px_add(px_add(px_add(_v720, px_str("px_chan_send(")), _v747), px_str(", ")), _v735), px_str(");\n"));
    }
    px_srcline(293);
    if (px_is_truthy(px_eq(_v721, px_str("Recv")))) {
        px_srcline(294);
        _v747 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(295);
        return px_add(px_add(px_add(_v720, px_str("px_chan_recv(")), _v747), px_str(");\n"));
    }
    px_srcline(296);
    if (px_is_truthy(px_eq(_v721, px_str("Spawn")))) {
        px_srcline(297);
        _v772 = px_index(_v718, px_int(1LL));
        px_srcline(298);
        if (px_is_truthy(px_eq(px_index(_v772, px_int(0LL)), px_str("Call")))) {
            px_srcline(299);
            _v727 = px_index(_v772, px_int(1LL));
            px_srcline(300);
            if (px_is_truthy(px_eq(px_index(_v727, px_int(0LL)), px_str("Var")))) {
                px_srcline(301);
                _v738 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v727, px_int(1LL))}, 1);
                px_srcline(302);
                _v773 = px_list_n((LXValue[]){}, 0);
                px_srcline(303);
                _v774 = px_index(_v772, px_int(2LL));
                px_srcline(304);
                _v775 = px_int(0LL);
                px_srcline(305);
                while (px_is_truthy(px_lt(_v775, px_call(px_get_global("len"), (LXValue[]){_v774}, 1)))) {
                    px_srcline(306);
                    (void)(px_method(_v773, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v774, _v775)}, 1)}, 1));
                    px_srcline(307);
                     _v775 = px_add(_v775, px_int(1LL));
                }
                px_srcline(308);
                _v732 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v774, _v773}, 2);
                px_srcline(309);
                _v733 = px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v732, px_int(0LL)), px_add(({ LXValue _s81 = px_add(px_add(px_add(px_add(px_str("px_spawn_name(\""), _v738), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v732, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s82 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v773}, 1)}, 1); px_add(_s81, _s82); }), px_str(")"))}, 2);
                px_srcline(310);
                return px_add(px_add(_v720, _v733), px_str(";\n"));
            }
            px_srcline(311);
            _v776 = px_index(_v718, px_int(2LL));
            px_srcline(312);
            (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s83 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v776, px_int(0LL))}, 1)), px_str(":")); LXValue _s84 = px_call(px_get_global("str"), (LXValue[]){px_index(_v776, px_int(1LL))}, 1); px_add(_s83, _s84); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)（方法调用请先绑定命名函数）"))}, 1));
            px_srcline(313);
            (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
        }
        px_srcline(314);
        _v777 = px_index(_v718, px_int(2LL));
        px_srcline(315);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s85 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v777, px_int(0LL))}, 1)), px_str(":")); LXValue _s86 = px_call(px_get_global("str"), (LXValue[]){px_index(_v777, px_int(1LL))}, 1); px_add(_s85, _s86); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()）"))}, 1));
        px_srcline(316);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(317);
    if (px_is_truthy(px_eq(_v721, px_str("Select")))) {
        px_srcline(318);
        return px_call(px_get_global("cg_gen_select"), (LXValue[]){px_index(_v718, px_int(1LL)), px_index(_v718, px_int(2LL)), _v719}, 3);
    }
    px_srcline(319);
    if (px_is_truthy(px_eq(_v721, px_str("Import")))) {
        px_srcline(320);
        return px_add(_v720, px_str("/* import 忽略（MVP） */\n"));
    }
    px_srcline(321);
    if (px_is_truthy(px_eq(_v721, px_str("FuncDef")))) {
        px_srcline(326);
        _v778 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v718, px_int(1LL))}, 1);
        px_srcline(327);
        _v779 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v778}, 1);
        px_srcline(330);
        _v780 = px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v718, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v718, px_int(4LL))}, 2), _v778}, 3);
        px_srcline(331);
        if (px_is_truthy(px_get_global("cg_topbody"))) {
            px_srcline(336);
            (void)(px_call(px_get_global("cg_uid"), (LXValue[]){}, 0));
            px_srcline(337);
            return px_add(px_add(px_add(px_add(px_add(_v720, px_str("px_set_global(\"")), _v778), px_str("\", ")), _v780), px_str(");\n"));
        }
        px_srcline(338);
        if (px_is_truthy(px_ne(_v779, px_null()))) {
            px_srcline(339);
            _v781 = px_add(px_add(_v720, px_call(px_get_global("cg_store_of"), (LXValue[]){_v778, _v779, _v780}, 3)), px_str(";\n"));
            px_srcline(341);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v778}, 1));
            px_srcline(342);
            return _v781;
        }
        px_srcline(343);
        return px_str("");
    }
    px_srcline(345);
    return px_str("");
px_err_782:
    if (px_err_782_proped) return px_err_782_val;
    return px_null();
}

static LXValue fn_cg_gen_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_stmt");
    LXValue _v783 = (nargs > 0) ? args[0] : px_null();
    LXValue _v784 = (nargs > 1) ? args[1] : px_null();
    LXValue _v785 = px_uninit();
    LXValue _v786 = px_uninit();
    LXValue _v787 = px_uninit();
    LXValue px_err_788_val = px_null();
    int px_err_788_proped = 0;
    px_srcline(351);
    _v785 = px_int(0LL);
    px_srcline(352);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v783}, 1), px_int(0LL)))) {
        px_srcline(353);
        _v786 = px_index(_v783, px_sub(px_call(px_get_global("len"), (LXValue[]){_v783}, 1), px_int(1LL)));
        px_srcline(354);
        if (px_is_truthy(({ LXValue _t790 = ({ LXValue _t789 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v786}, 1), px_str("list")); px_is_truthy(_t789) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v786}, 1), px_int(1LL)) : _t789; }); px_is_truthy(_t790) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v786, px_int(0LL))}, 1), px_str("int")) : _t790; }))) {
            px_srcline(355);
             _v785 = px_index(_v786, px_int(0LL));
        }
    }
    px_srcline(356);
    _v787 = px_str("");
    px_srcline(357);
    if (px_is_truthy(px_gt(_v785, px_int(0LL)))) {
        px_srcline(358);
         _v787 = px_add(({ LXValue _s87 = px_add(px_call(px_get_global("cg_pad"), (LXValue[]){_v784}, 1), px_str("px_srcline(")); LXValue _s88 = px_call(px_get_global("str"), (LXValue[]){_v785}, 1); px_add(_s87, _s88); }), px_str(");\n"));
    }
    px_srcline(359);
    return px_add(_v787, px_call(px_get_global("cg_gen_stmt_inner"), (LXValue[]){_v783, _v784}, 2));
px_err_788:
    if (px_err_788_proped) return px_err_788_val;
    return px_null();
}

static LXValue fn_cg_assign_op_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_global");
    LXValue _v791 = (nargs > 0) ? args[0] : px_null();
    LXValue _v792 = (nargs > 1) ? args[1] : px_null();
    LXValue _v793 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_794_val = px_null();
    int px_err_794_proped = 0;
    px_srcline(362);
    if (px_is_truthy(px_eq(_v791, px_str("Assign")))) {
        px_srcline(363);
        return _v793;
    }
    px_srcline(364);
    if (px_is_truthy(px_eq(_v791, px_str("Plus")))) {
        px_srcline(365);
        return px_add(px_add(px_add(px_add(px_str("px_add(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(366);
    if (px_is_truthy(px_eq(_v791, px_str("Minus")))) {
        px_srcline(367);
        return px_add(px_add(px_add(px_add(px_str("px_sub(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(368);
    if (px_is_truthy(px_eq(_v791, px_str("Star")))) {
        px_srcline(369);
        return px_add(px_add(px_add(px_add(px_str("px_mul(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(370);
    if (px_is_truthy(px_eq(_v791, px_str("Slash")))) {
        px_srcline(371);
        return px_add(px_add(px_add(px_add(px_str("px_div(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(372);
    if (px_is_truthy(px_eq(_v791, px_str("IntDiv")))) {
        px_srcline(373);
        return px_add(px_add(px_add(px_add(px_str("px_idiv(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(374);
    if (px_is_truthy(px_eq(_v791, px_str("Mod")))) {
        px_srcline(375);
        return px_add(px_add(px_add(px_add(px_str("px_mod(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(376);
    if (px_is_truthy(px_eq(_v791, px_str("Pow")))) {
        px_srcline(377);
        return px_add(px_add(px_add(px_add(px_str("px_pow(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(378);
    if (px_is_truthy(px_eq(_v791, px_str("BitAnd")))) {
        px_srcline(379);
        return px_add(px_add(px_add(px_add(px_str("px_bitand(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(380);
    if (px_is_truthy(px_eq(_v791, px_str("BitOr")))) {
        px_srcline(381);
        return px_add(px_add(px_add(px_add(px_str("px_bitor(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(382);
    if (px_is_truthy(px_eq(_v791, px_str("BitXor")))) {
        px_srcline(383);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(384);
    if (px_is_truthy(px_eq(_v791, px_str("Shl")))) {
        px_srcline(385);
        return px_add(px_add(px_add(px_add(px_str("px_shl(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(386);
    if (px_is_truthy(px_eq(_v791, px_str("Shr")))) {
        px_srcline(387);
        return px_add(px_add(px_add(px_add(px_str("px_shr(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(388);
    if (px_is_truthy(px_eq(_v791, px_str("ShrU")))) {
        px_srcline(389);
        return px_add(px_add(px_add(px_add(px_str("px_ushr(px_get_global(\""), _v792), px_str("\"), ")), _v793), px_str(")"));
    }
    px_srcline(390);
    return _v793;
px_err_794:
    if (px_err_794_proped) return px_err_794_val;
    return px_null();
}

static LXValue fn_cg_assign_op_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_assign_op_local");
    LXValue _v795 = (nargs > 0) ? args[0] : px_null();
    LXValue _v796 = (nargs > 1) ? args[1] : px_null();
    LXValue _v797 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_798_val = px_null();
    int px_err_798_proped = 0;
    px_srcline(393);
    if (px_is_truthy(px_eq(_v795, px_str("Assign")))) {
        px_srcline(394);
        return _v797;
    }
    px_srcline(395);
    if (px_is_truthy(px_eq(_v795, px_str("Plus")))) {
        px_srcline(396);
        return px_add(px_add(px_add(px_add(px_str("px_add("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(397);
    if (px_is_truthy(px_eq(_v795, px_str("Minus")))) {
        px_srcline(398);
        return px_add(px_add(px_add(px_add(px_str("px_sub("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(399);
    if (px_is_truthy(px_eq(_v795, px_str("Star")))) {
        px_srcline(400);
        return px_add(px_add(px_add(px_add(px_str("px_mul("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(401);
    if (px_is_truthy(px_eq(_v795, px_str("Slash")))) {
        px_srcline(402);
        return px_add(px_add(px_add(px_add(px_str("px_div("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(403);
    if (px_is_truthy(px_eq(_v795, px_str("IntDiv")))) {
        px_srcline(404);
        return px_add(px_add(px_add(px_add(px_str("px_idiv("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(405);
    if (px_is_truthy(px_eq(_v795, px_str("Mod")))) {
        px_srcline(406);
        return px_add(px_add(px_add(px_add(px_str("px_mod("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(407);
    if (px_is_truthy(px_eq(_v795, px_str("Pow")))) {
        px_srcline(408);
        return px_add(px_add(px_add(px_add(px_str("px_pow("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(409);
    if (px_is_truthy(px_eq(_v795, px_str("BitAnd")))) {
        px_srcline(410);
        return px_add(px_add(px_add(px_add(px_str("px_bitand("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(411);
    if (px_is_truthy(px_eq(_v795, px_str("BitOr")))) {
        px_srcline(412);
        return px_add(px_add(px_add(px_add(px_str("px_bitor("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(413);
    if (px_is_truthy(px_eq(_v795, px_str("BitXor")))) {
        px_srcline(414);
        return px_add(px_add(px_add(px_add(px_str("px_bitxor("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(415);
    if (px_is_truthy(px_eq(_v795, px_str("Shl")))) {
        px_srcline(416);
        return px_add(px_add(px_add(px_add(px_str("px_shl("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(417);
    if (px_is_truthy(px_eq(_v795, px_str("Shr")))) {
        px_srcline(418);
        return px_add(px_add(px_add(px_add(px_str("px_shr("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(419);
    if (px_is_truthy(px_eq(_v795, px_str("ShrU")))) {
        px_srcline(420);
        return px_add(px_add(px_add(px_add(px_str("px_ushr("), _v796), px_str(", ")), _v797), px_str(")"));
    }
    px_srcline(421);
    return _v797;
px_err_798:
    if (px_err_798_proped) return px_err_798_val;
    return px_null();
}

static LXValue fn_cg_gen_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_select");
    LXValue _v799 = (nargs > 0) ? args[0] : px_null();
    LXValue _v800 = (nargs > 1) ? args[1] : px_null();
    LXValue _v801 = (nargs > 2) ? args[2] : px_null();
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
    LXValue _v820 = px_uninit();
    LXValue _v821 = px_uninit();
    LXValue _v822 = px_uninit();
    LXValue _v823 = px_uninit();
    LXValue _v824 = px_uninit();
    LXValue px_err_825_val = px_null();
    int px_err_825_proped = 0;
    px_srcline(424);
    _v802 = px_call(px_get_global("cg_pad"), (LXValue[]){_v801}, 1);
    px_srcline(425);
    _v803 = px_call(px_get_global("len"), (LXValue[]){_v799}, 1);
    px_srcline(426);
    if (px_is_truthy(px_eq(_v803, px_int(0LL)))) {
        px_srcline(427);
        return px_str("select 至少需要一个 case 分支");
    }
    px_srcline(428);
    _v804 = px_call(px_get_global("cg_uid"), (LXValue[]){}, 0);
    px_srcline(429);
    _v805 = px_str("");
    px_srcline(432);
    _v806 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(433);
    _v807 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(434);
    _v808 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(435);
    _v809 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(437);
    _v810 = px_list_n((LXValue[]){}, 0);
    px_srcline(438);
    _v811 = px_int(0LL);
    px_srcline(439);
    while (px_is_truthy(px_lt(_v811, _v803))) {
        px_srcline(440);
        _v812 = px_index(px_index(_v799, _v811), px_int(1LL));
        px_srcline(441);
        if (px_is_truthy(px_eq(px_index(_v812, px_int(0LL)), px_str("Call")))) {
            px_srcline(442);
            _v813 = px_index(_v812, px_int(1LL));
            px_srcline(443);
            if (px_is_truthy(px_eq(px_index(_v813, px_int(0LL)), px_str("Field")))) {
                px_srcline(444);
                _v814 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v813, px_int(2LL))}, 1);
                px_srcline(445);
                if (px_is_truthy(px_eq(_v814, px_str("recv")))) {
                    px_srcline(446);
                    (void)(px_method(_v810, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v813, px_int(1LL))}, 1)}, 1));
                    px_srcline(447);
                     _v811 = px_add(_v811, px_int(1LL));
                    px_srcline(448);
                    continue;
                }
                px_srcline(449);
                return px_add(px_add(px_str("select case 仅支持 ch.recv()（不支持 ."), _v814), px_str("）"));
            }
            px_srcline(450);
            return px_str("select case 仅支持 ch.recv()");
        }
        px_srcline(451);
        return px_str("select case 仅支持 ch.recv()");
    }
    px_srcline(452);
     _v805 = px_add(_v805, px_add(({ LXValue _s91 = px_add(({ LXValue _s89 = px_add(px_add(px_add(_v802, px_str("LXValue _chans")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str("[")); LXValue _s90 = px_call(px_get_global("str"), (LXValue[]){_v803}, 1); px_add(_s89, _s90); }), px_str("] = {")); LXValue _s92 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v810}, 2); px_add(_s91, _s92); }), px_str("};\n")));
    px_srcline(453);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("_sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(": {\n")));
    px_srcline(454);
    _v815 = px_list_n((LXValue[]){}, 0);
    px_srcline(455);
    _v816 = px_int(0LL);
    px_srcline(456);
    while (px_is_truthy(px_lt(_v816, _v803))) {
        px_srcline(457);
        (void)(px_method(_v815, "append", (LXValue[]){px_call(px_get_global("str"), (LXValue[]){_v816}, 1)}, 1));
        px_srcline(458);
         _v816 = px_add(_v816, px_int(1LL));
    }
    px_srcline(459);
     _v805 = px_add(_v805, px_add(({ LXValue _s95 = px_add(({ LXValue _s93 = px_add(px_add(px_add(_v802, px_str("    int _ord")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str("[")); LXValue _s94 = px_call(px_get_global("str"), (LXValue[]){_v803}, 1); px_add(_s93, _s94); }), px_str("] = {")); LXValue _s96 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v815}, 2); px_add(_s95, _s96); }), px_str("};\n")));
    px_srcline(460);
    if (px_is_truthy(px_gt(_v803, px_int(1LL)))) {
        px_srcline(461);
        _v817 = px_add(({ LXValue _s101 = px_add(({ LXValue _s99 = px_add(({ LXValue _s97 = px_add(px_add(px_add(_v802, px_str("    for (int _i")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = ")); LXValue _s98 = px_call(px_get_global("str"), (LXValue[]){_v803}, 1); px_add(_s97, _s98); }), px_str(" - 1; _i")); LXValue _s100 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s99, _s100); }), px_str(" > 0; _i")); LXValue _s102 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s101, _s102); }), px_str("--) { "));
        px_srcline(462);
         _v817 = px_add(_v817, px_add(({ LXValue _s103 = px_add(px_add(px_str("int _j"), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = rand() % (_i")); LXValue _s104 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s103, _s104); }), px_str(" + 1); ")));
        px_srcline(463);
         _v817 = px_add(_v817, px_add(({ LXValue _s121 = px_add(({ LXValue _s119 = px_add(({ LXValue _s117 = px_add(({ LXValue _s115 = px_add(({ LXValue _s113 = px_add(({ LXValue _s111 = px_add(({ LXValue _s109 = px_add(({ LXValue _s107 = px_add(({ LXValue _s105 = px_add(px_add(px_str("int _t"), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = _ord")); LXValue _s106 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s105, _s106); }), px_str("[_i")); LXValue _s108 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s107, _s108); }), px_str("]; _ord")); LXValue _s110 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s109, _s110); }), px_str("[_i")); LXValue _s112 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s111, _s112); }), px_str("] = _ord")); LXValue _s114 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s113, _s114); }), px_str("[_j")); LXValue _s116 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s115, _s116); }), px_str("]; _ord")); LXValue _s118 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s117, _s118); }), px_str("[_j")); LXValue _s120 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s119, _s120); }), px_str("] = _t")); LXValue _s122 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s121, _s122); }), px_str("; ")));
        px_srcline(464);
         _v817 = px_add(_v817, px_str("}\n"));
        px_srcline(465);
         _v805 = px_add(_v805, _v817);
    }
    px_srcline(466);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("    LXValue _rv")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = px_null();\n")));
    px_srcline(467);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("    int _picked")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = -1;\n")));
    px_srcline(468);
     _v805 = px_add(_v805, px_add(({ LXValue _s127 = px_add(({ LXValue _s125 = px_add(({ LXValue _s123 = px_add(px_add(px_add(_v802, px_str("    for (int _k")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = 0; _k")); LXValue _s124 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s123, _s124); }), px_str(" < ")); LXValue _s126 = px_call(px_get_global("str"), (LXValue[]){_v803}, 1); px_add(_s125, _s126); }), px_str("; _k")); LXValue _s128 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s127, _s128); }), px_str("++) {\n")));
    px_srcline(469);
     _v805 = px_add(_v805, px_add(({ LXValue _s131 = px_add(({ LXValue _s129 = px_add(px_add(px_add(_v802, px_str("        int _idx")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" = _ord")); LXValue _s130 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s129, _s130); }), px_str("[_k")); LXValue _s132 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s131, _s132); }), px_str("];\n")));
    px_srcline(470);
     _v805 = px_add(_v805, px_add(({ LXValue _s139 = px_add(({ LXValue _s137 = px_add(({ LXValue _s135 = px_add(({ LXValue _s133 = px_add(px_add(px_add(_v802, px_str("        if (px_chan_try_recv(_chans")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str("[_idx")); LXValue _s134 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s133, _s134); }), px_str("], &_rv")); LXValue _s136 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s135, _s136); }), px_str(")) { _picked")); LXValue _s138 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s137, _s138); }), px_str(" = _idx")); LXValue _s140 = px_call(px_get_global("str"), (LXValue[]){_v804}, 1); px_add(_s139, _s140); }), px_str("; break; }\n")));
    px_srcline(471);
     _v805 = px_add(_v805, px_add(_v802, px_str("    }\n")));
    px_srcline(473);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("    if (_picked")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" >= 0) {\n")));
    px_srcline(474);
    _v818 = px_int(0LL);
    px_srcline(475);
    while (px_is_truthy(px_lt(_v818, _v803))) {
        px_srcline(476);
        _v819 = px_index(px_index(_v799, _v818), px_int(0LL));
        px_srcline(477);
        _v820 = px_index(px_index(_v799, _v818), px_int(2LL));
        px_srcline(478);
        _v821 = px_add(({ LXValue _s141 = px_add(px_add(px_str("if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" == ")); LXValue _s142 = px_call(px_get_global("str"), (LXValue[]){_v818}, 1); px_add(_s141, _s142); }), px_str(")"));
        px_srcline(479);
        if (px_is_truthy(px_gt(_v818, px_int(0LL)))) {
            px_srcline(480);
             _v821 = px_add(({ LXValue _s143 = px_add(px_add(px_str("else if (_picked"), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(" == ")); LXValue _s144 = px_call(px_get_global("str"), (LXValue[]){_v818}, 1); px_add(_s143, _s144); }), px_str(")"));
        }
        px_srcline(481);
         _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("        ")), _v821), px_str(" {\n")));
        px_srcline(482);
        if (px_is_truthy(px_ne(_v819, px_null()))) {
            px_srcline(483);
            _v822 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v819}, 1)}, 1);
            px_srcline(484);
             _v805 = px_add(_v805, px_add(px_add(px_add(px_add(px_add(_v802, px_str("            LXValue ")), _v822), px_str(" = _rv")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(";\n")));
        }
        px_srcline(485);
        _v823 = px_int(0LL);
        px_srcline(486);
        while (px_is_truthy(px_lt(_v823, px_call(px_get_global("len"), (LXValue[]){_v820}, 1)))) {
            px_srcline(487);
             _v805 = px_add(_v805, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v820, _v823), px_add(_v801, px_int(3LL))}, 2));
            px_srcline(488);
             _v823 = px_add(_v823, px_int(1LL));
        }
        px_srcline(489);
         _v805 = px_add(_v805, px_add(_v802, px_str("        }\n")));
        px_srcline(490);
         _v818 = px_add(_v818, px_int(1LL));
    }
    px_srcline(491);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(";\n")));
    px_srcline(492);
     _v805 = px_add(_v805, px_add(_v802, px_str("    }\n")));
    px_srcline(494);
    if (px_is_truthy(px_ne(_v800, px_null()))) {
        px_srcline(495);
         _v805 = px_add(_v805, px_add(_v802, px_str("    {\n")));
        px_srcline(496);
        _v824 = px_int(0LL);
        px_srcline(497);
        while (px_is_truthy(px_lt(_v824, px_call(px_get_global("len"), (LXValue[]){_v800}, 1)))) {
            px_srcline(498);
             _v805 = px_add(_v805, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v800, _v824), px_add(_v801, px_int(2LL))}, 2));
            px_srcline(499);
             _v824 = px_add(_v824, px_int(1LL));
        }
        px_srcline(500);
         _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("        goto _sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(";\n")));
        px_srcline(501);
         _v805 = px_add(_v805, px_add(_v802, px_str("    }\n")));
    }
    px_srcline(503);
     _v805 = px_add(_v805, px_add(_v802, px_str("    px_select_wait();\n")));
    px_srcline(504);
     _v805 = px_add(_v805, px_add(_v802, px_str("}\n")));
    px_srcline(505);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("goto _sel_retry_")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(";\n")));
    px_srcline(506);
     _v805 = px_add(_v805, px_add(px_add(px_add(_v802, px_str("_sel_done_")), px_call(px_get_global("str"), (LXValue[]){_v804}, 1)), px_str(": ;\n")));
    px_srcline(507);
    px_set_global("cg_vars", _v807);
    px_srcline(508);
    px_set_global("cg_var_types", _v808);
    px_srcline(509);
    px_set_global("cg_cells", _v809);
    px_srcline(510);
    (void)(px_call(px_get_global("cg_inited_restore"), (LXValue[]){_v806}, 1));
    px_srcline(511);
    return _v805;
px_err_825:
    if (px_err_825_proped) return px_err_825_val;
    return px_null();
}

static LXValue fn_cg_comp_collect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_collect");
    LXValue _v826 = (nargs > 0) ? args[0] : px_null();
    LXValue _v827 = px_uninit();
    LXValue _v828 = px_uninit();
    LXValue _v829 = px_uninit();
    LXValue _v830 = px_uninit();
    LXValue _v831 = px_uninit();
    LXValue _v832 = px_uninit();
    LXValue _v833 = px_uninit();
    LXValue _v834 = px_uninit();
    LXValue _v835 = px_uninit();
    LXValue _v836 = px_uninit();
    LXValue px_err_837_val = px_null();
    int px_err_837_proped = 0;
    px_srcline(9);
    _v827 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(10);
    (void)(px_method(_v827, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(11);
    px_index_set(_v827, px_str("its"), px_list_n((LXValue[]){}, 0));
    px_srcline(12);
    px_index_set(_v827, px_str("ivs"), px_list_n((LXValue[]){}, 0));
    px_srcline(13);
    px_index_set(_v827, px_str("itms"), px_list_n((LXValue[]){}, 0));
    px_srcline(14);
    px_index_set(_v827, px_str("idxs"), px_list_n((LXValue[]){}, 0));
    px_srcline(15);
    px_index_set(_v827, px_str("binds"), px_list_n((LXValue[]){}, 0));
    px_srcline(16);
    px_index_set(_v827, px_str("saved_all"), px_list_n((LXValue[]){}, 0));
    px_srcline(17);
    _v828 = px_int(0LL);
    px_srcline(18);
    while (px_is_truthy(px_lt(_v828, px_call(px_get_global("len"), (LXValue[]){_v826}, 1)))) {
        px_srcline(19);
        _v829 = px_index(_v826, _v828);
        px_srcline(20);
        _v830 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v829, px_int(2LL))}, 1);
        px_srcline(21);
        (void)(px_method(px_index(_v827, px_str("its")), "append", (LXValue[]){_v830}, 1));
        px_srcline(22);
        (void)(px_method(px_index(_v827, px_str("ivs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(23);
        (void)(px_method(px_index(_v827, px_str("itms")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(24);
        (void)(px_method(px_index(_v827, px_str("idxs")), "append", (LXValue[]){px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0)}, 1));
        px_srcline(25);
        _v831 = px_str("");
        px_srcline(26);
        _v832 = px_list_n((LXValue[]){}, 0);
        px_srcline(27);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v829, px_int(1LL))}, 1), px_int(1LL)))) {
            px_srcline(28);
            _v833 = px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
            px_srcline(29);
            _v834 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v829, px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(30);
            _v835 = px_null();
            px_srcline(31);
            if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v834}, 1))) {
                px_srcline(32);
                 _v835 = px_index(px_get_global("cg_vars"), _v834);
            }
            px_srcline(33);
            px_index_set(px_get_global("cg_vars"), _v834, _v833);
            px_srcline(34);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v834}, 1))) {
                px_srcline(35);
                (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v834}, 1));
            }
            px_srcline(36);
            (void)(px_method(_v832, "append", (LXValue[]){px_list_n((LXValue[]){_v834, _v835}, 2)}, 1));
            px_srcline(37);
             _v831 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v833), px_str(" = ")), px_index(px_index(_v827, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v827, px_str("itms"))}, 1), px_int(1LL)))), px_str("; "));
        }
        else {
            px_srcline(39);
            _v836 = px_int(0LL);
            px_srcline(44);
             _v831 = px_add(({ LXValue _s145 = px_add(px_add(px_str("px_unpack_ck("), px_index(px_index(_v827, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v827, px_str("itms"))}, 1), px_int(1LL)))), px_str(", ")); LXValue _s146 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v829, px_int(1LL))}, 1)}, 1); px_add(_s145, _s146); }), px_str("); "));
            px_srcline(45);
            while (px_is_truthy(px_lt(_v836, px_call(px_get_global("len"), (LXValue[]){px_index(_v829, px_int(1LL))}, 1)))) {
                px_srcline(46);
                _v834 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v829, px_int(1LL)), _v836)}, 1);
                px_srcline(47);
                _v833 = ({ LXValue _s147 = px_add(px_add(px_str("_cv"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1)), px_str("_")); LXValue _s148 = px_call(px_get_global("str"), (LXValue[]){_v836}, 1); px_add(_s147, _s148); });
                px_srcline(48);
                _v835 = px_null();
                px_srcline(49);
                if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v834}, 1))) {
                    px_srcline(50);
                     _v835 = px_index(px_get_global("cg_vars"), _v834);
                }
                px_srcline(51);
                px_index_set(px_get_global("cg_vars"), _v834, _v833);
                px_srcline(52);
                if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v834}, 1))) {
                    px_srcline(53);
                    (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v834}, 1));
                }
                px_srcline(54);
                (void)(px_method(_v832, "append", (LXValue[]){px_list_n((LXValue[]){_v834, _v835}, 2)}, 1));
                px_srcline(55);
                 _v831 = px_add(_v831, px_add(({ LXValue _s149 = px_add(px_add(px_add(px_add(px_str("LXValue "), _v833), px_str(" = px_index(")), px_index(px_index(_v827, px_str("itms")), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v827, px_str("itms"))}, 1), px_int(1LL)))), px_str(", px_int(")); LXValue _s150 = px_call(px_get_global("str"), (LXValue[]){_v836}, 1); px_add(_s149, _s150); }), px_str(")); ")));
                px_srcline(56);
                 _v836 = px_add(_v836, px_int(1LL));
            }
        }
        px_srcline(57);
        (void)(px_method(px_index(_v827, px_str("binds")), "append", (LXValue[]){_v831}, 1));
        px_srcline(58);
        (void)(px_method(px_index(_v827, px_str("saved_all")), "append", (LXValue[]){_v832}, 1));
        px_srcline(59);
         _v828 = px_add(_v828, px_int(1LL));
    }
    px_srcline(60);
    return _v827;
px_err_837:
    if (px_err_837_proped) return px_err_837_val;
    return px_null();
}

static LXValue fn_cg_comp_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_restore");
    LXValue _v838 = (nargs > 0) ? args[0] : px_null();
    LXValue _v839 = px_uninit();
    LXValue _v840 = px_uninit();
    LXValue _v841 = px_uninit();
    LXValue _v842 = px_uninit();
    LXValue _v843 = px_uninit();
    LXValue px_err_844_val = px_null();
    int px_err_844_proped = 0;
    px_srcline(62);
    _v839 = px_int(0LL);
    px_srcline(63);
    while (px_is_truthy(px_lt(_v839, px_call(px_get_global("len"), (LXValue[]){_v838}, 1)))) {
        px_srcline(64);
        _v840 = px_index(_v838, _v839);
        px_srcline(65);
        _v841 = px_int(0LL);
        px_srcline(66);
        while (px_is_truthy(px_lt(_v841, px_call(px_get_global("len"), (LXValue[]){_v840}, 1)))) {
            px_srcline(67);
            _v842 = px_index(px_index(_v840, _v841), px_int(0LL));
            px_srcline(68);
            _v843 = px_index(px_index(_v840, _v841), px_int(1LL));
            px_srcline(69);
            if (px_is_truthy(px_eq(_v843, px_null()))) {
                px_srcline(70);
                (void)(px_method(px_get_global("cg_vars"), "remove", (LXValue[]){_v842}, 1));
            }
            else {
                px_srcline(72);
                px_index_set(px_get_global("cg_vars"), _v842, _v843);
            }
            px_srcline(73);
             _v841 = px_add(_v841, px_int(1LL));
        }
        px_srcline(74);
         _v839 = px_add(_v839, px_int(1LL));
    }
px_err_844:
    if (px_err_844_proped) return px_err_844_val;
    return px_null();
}

static LXValue fn_cg_comp_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_comp_body");
    LXValue _v845 = (nargs > 0) ? args[0] : px_null();
    LXValue _v846 = (nargs > 1) ? args[1] : px_null();
    LXValue _v847 = (nargs > 2) ? args[2] : px_null();
    LXValue _v848 = px_uninit();
    LXValue _v849 = px_uninit();
    LXValue _v850 = px_uninit();
    LXValue _v851 = px_uninit();
    LXValue _v852 = px_uninit();
    LXValue px_err_853_val = px_null();
    int px_err_853_proped = 0;
    px_srcline(77);
    _v848 = px_str("");
    px_srcline(78);
    if (px_is_truthy(px_ne(_v846, px_null()))) {
        px_srcline(79);
         _v848 = px_add(px_add(px_add(px_add(px_str("if (px_is_truthy("), _v846), px_str(")) { ")), _v847), px_str("} "));
    }
    else {
        px_srcline(81);
         _v848 = _v847;
    }
    px_srcline(82);
    _v849 = px_call(px_get_global("len"), (LXValue[]){px_index(_v845, px_str("its"))}, 1);
    px_srcline(83);
    _v850 = px_sub(_v849, px_int(1LL));
    px_srcline(84);
    while (px_is_truthy(px_ge(_v850, px_int(0LL)))) {
        px_srcline(85);
        _v851 = px_str("");
        px_srcline(86);
        if (px_is_truthy(px_lt(px_add(_v850, px_int(1LL)), _v849))) {
            px_srcline(87);
             _v851 = px_add(px_add(px_add(px_add(px_str("LXValue "), px_index(px_index(_v845, px_str("ivs")), px_add(_v850, px_int(1LL)))), px_str(" = ")), px_index(px_index(_v845, px_str("its")), px_add(_v850, px_int(1LL)))), px_str("; "));
        }
        px_srcline(92);
        _v852 = px_call(px_get_global("cg_iter_len_tmp"), (LXValue[]){}, 0);
        px_srcline(93);
         _v848 = px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("for (int "), _v852), px_str(" = px_iter_prepare(")), px_index(px_index(_v845, px_str("ivs")), _v850)), px_str("), ")), px_index(px_index(_v845, px_str("idxs")), _v850)), px_str("=0; ")), px_index(px_index(_v845, px_str("idxs")), _v850)), px_str("<")), _v852), px_str("; ")), px_index(px_index(_v845, px_str("idxs")), _v850)), px_str("++) { px_iter_ck(")), px_index(px_index(_v845, px_str("ivs")), _v850)), px_str(", ")), _v852), px_str("); LXValue ")), px_index(px_index(_v845, px_str("itms")), _v850)), px_str(" = px_iter_at(")), px_index(px_index(_v845, px_str("ivs")), _v850)), px_str(", px_int(")), px_index(px_index(_v845, px_str("idxs")), _v850)), px_str(")); ")), px_index(px_index(_v845, px_str("binds")), _v850)), _v851), _v848), px_str(" } "));
        px_srcline(94);
         _v850 = px_sub(_v850, px_int(1LL));
    }
    px_srcline(95);
    return _v848;
px_err_853:
    if (px_err_853_proped) return px_err_853_val;
    return px_null();
}

static LXValue fn_cg_gen_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_closure");
    LXValue _v854 = (nargs > 0) ? args[0] : px_null();
    LXValue _v855 = (nargs > 1) ? args[1] : px_null();
    LXValue _v856 = (nargs > 2) ? args[2] : px_null();
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
    LXValue _v880 = px_uninit();
    LXValue _v881 = px_uninit();
    LXValue _v882 = px_uninit();
    LXValue _v883 = px_uninit();
    LXValue _v884 = px_uninit();
    LXValue _v885 = px_uninit();
    LXValue px_err_886_val = px_null();
    int px_err_886_proped = 0;
    px_srcline(102);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(103);
    _v857 = px_get_global("cg_closure_id");
    px_srcline(104);
    _v858 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v857}, 1));
    px_srcline(106);
    _v859 = px_list_n((LXValue[]){}, 0);
    px_srcline(107);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v854, px_null(), _v855, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v859}, 2));
    px_srcline(108);
    _v860 = px_list_n((LXValue[]){}, 0);
    px_srcline(109);
    LXValue _t887 = _v859;
    int _il1 = px_iter_prepare(_t887);
    for (int _t888 = 0; _t888 < _il1; _t888++) {
        px_iter_ck(_t887, _il1);
        _v861 = px_iter_at(_t887, px_int(_t888));
        px_srcline(110);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v861}, 1), px_null()))) {
            px_srcline(111);
            (void)(px_method(_v860, "append", (LXValue[]){_v861}, 1));
        }
    }
    px_srcline(113);
    _v862 = px_str("px_null()");
    px_srcline(114);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v860}, 1), px_int(0LL)))) {
        px_srcline(115);
        _v863 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(116);
        LXValue _t889 = _v860;
        int _il2 = px_iter_prepare(_t889);
        for (int _t890 = 0; _t890 < _il2; _t890++) {
            px_iter_ck(_t889, _il2);
            _v861 = px_iter_at(_t889, px_int(_t890));
            px_srcline(117);
            _v864 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v861}, 1);
            px_srcline(120);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v861}, 1))) {
                px_srcline(121);
                 _v863 = px_add(_v863, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v861), px_str("\", ")), _v864), px_str("); ")));
            }
            else {
                px_srcline(123);
                 _v863 = px_add(_v863, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v861), px_str("\", px_cell(")), _v864), px_str(")); ")));
            }
        }
        px_srcline(124);
         _v863 = px_add(_v863, px_str("_capenv; })"));
        px_srcline(125);
         _v862 = _v863;
    }
    px_srcline(126);
    _v865 = px_add(px_add(px_str("static LXValue "), _v858), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(127);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v860}, 1), px_int(0LL)))) {
        px_srcline(128);
         _v865 = px_add(_v865, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(130);
         _v865 = px_add(_v865, px_str("    (void)ctx;\n"));
    }
    px_srcline(131);
    _v866 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(132);
    _v867 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(133);
    _v868 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(136);
    _v869 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(137);
    _v870 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(138);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(140);
    _v871 = px_get_global("cg_topbody");
    px_srcline(141);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(142);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(143);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(144);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(145);
    _v872 = px_int(0LL);
    px_srcline(146);
    while (px_is_truthy(px_lt(_v872, px_call(px_get_global("len"), (LXValue[]){_v854}, 1)))) {
        px_srcline(147);
        _v873 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v854, _v872), px_int(1LL))}, 1);
        px_srcline(148);
        _v874 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v873}, 1);
        px_srcline(149);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v873}, 1));
        px_srcline(150);
         _v865 = px_add(_v865, px_add(({ LXValue _s151 = px_add(px_add(px_add(px_add(px_str("    LXValue "), _v874), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v872}, 1)), px_str(") ? args[")); LXValue _s152 = px_call(px_get_global("str"), (LXValue[]){_v872}, 1); px_add(_s151, _s152); }), px_str("] : px_null();\n")));
        px_srcline(151);
         _v872 = px_add(_v872, px_int(1LL));
    }
    px_srcline(153);
    LXValue _t891 = _v860;
    int _il3 = px_iter_prepare(_t891);
    for (int _t892 = 0; _t892 < _il3; _t892++) {
        px_iter_ck(_t891, _il3);
        _v861 = px_iter_at(_t891, px_int(_t892));
        px_srcline(154);
        _v875 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v861}, 1);
        px_srcline(155);
        px_index_set(px_get_global("cg_cells"), _v861, px_int(1LL));
        px_srcline(156);
        if (px_is_truthy(px_method(_v870, "has", (LXValue[]){_v861}, 1))) {
            px_srcline(157);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v861}, 1));
        }
        px_srcline(158);
         _v865 = px_add(_v865, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v875), px_str(" = px_env_lookup(ctx, \"")), _v861), px_str("\");\n")));
    }
    px_srcline(168);
    _v876 = px_list_n((LXValue[]){}, 0);
    px_srcline(169);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v855, _v876}, 2));
    px_srcline(170);
    _v877 = px_list_n((LXValue[]){}, 0);
    px_srcline(171);
    _v878 = px_list_n((LXValue[]){}, 0);
    px_srcline(172);
    if (px_is_truthy(({ LXValue _t894 = ({ LXValue _t893 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v855}, 1), px_str("list")); px_is_truthy(_t893) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v855}, 1), px_int(0LL)) : _t893; }); px_is_truthy(_t894) ? px_eq(px_index(_v855, px_int(0LL)), px_str("Block")) : _t894; }))) {
        px_srcline(173);
        (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v855, px_int(1LL)), _v877}, 2));
        px_srcline(174);
        (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v855, px_int(1LL)), _v878}, 2));
    }
    px_srcline(175);
    _v879 = px_list_n((LXValue[]){}, 0);
    px_srcline(176);
    _v880 = px_int(0LL);
    px_srcline(177);
    while (px_is_truthy(px_lt(_v880, px_call(px_get_global("len"), (LXValue[]){_v877}, 1)))) {
        px_srcline(178);
        _v881 = px_index(_v877, _v880);
        px_srcline(180);
        if (px_is_truthy(({ LXValue _t896 = px_eq(px_call(px_get_global("cg_var_of"), (LXValue[]){_v881}, 1), px_null()); px_is_truthy(_t896) ? px_not(({ LXValue _t895 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v878, _v881}, 2)); px_is_truthy(_t895) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_topnames"), _v881}, 2) : _t895; })) : _t896; }))) {
            px_srcline(181);
            _v882 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v881}, 1);
            px_srcline(182);
            if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v876, _v881}, 2))) {
                px_srcline(183);
                px_index_set(px_get_global("cg_cells"), _v881, px_int(1LL));
            }
            px_srcline(184);
            (void)(px_method(_v879, "append", (LXValue[]){px_list_n((LXValue[]){_v882, _v881}, 2)}, 1));
        }
        px_srcline(185);
         _v880 = px_add(_v880, px_int(1LL));
    }
    px_srcline(186);
    _v883 = px_int(0LL);
    px_srcline(187);
    while (px_is_truthy(px_lt(_v883, px_call(px_get_global("len"), (LXValue[]){_v879}, 1)))) {
        px_srcline(188);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(px_index(_v879, _v883), px_int(1LL))}, 1))) {
            px_srcline(189);
             _v865 = px_add(_v865, px_add(px_add(px_str("    LXValue "), px_index(px_index(_v879, _v883), px_int(0LL))), px_str(" = px_cell(px_uninit());\n")));
        }
        else {
            px_srcline(191);
             _v865 = px_add(_v865, px_add(px_add(px_str("    LXValue "), px_index(px_index(_v879, _v883), px_int(0LL))), px_str(" = px_uninit();\n")));
        }
        px_srcline(192);
         _v883 = px_add(_v883, px_int(1LL));
    }
    px_srcline(201);
    _v884 = px_call(px_get_global("cg_cerr_tag"), (LXValue[]){}, 0);
    px_srcline(202);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v884}, 1));
    px_srcline(203);
     _v865 = px_add(_v865, px_add(px_add(px_str("    LXValue "), _v884), px_str("_val = px_null();\n")));
    px_srcline(204);
     _v865 = px_add(_v865, px_add(px_add(px_str("    int "), _v884), px_str("_proped = 0;\n")));
    px_srcline(205);
    _v885 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v855}, 1);
    px_srcline(206);
     _v865 = px_add(_v865, px_add(px_add(px_str("    return "), _v885), px_str(";\n")));
    px_srcline(207);
     _v865 = px_add(_v865, px_add(_v884, px_str(":\n")));
    px_srcline(208);
     _v865 = px_add(_v865, px_add(px_add(px_add(px_add(px_str("    if ("), _v884), px_str("_proped) return ")), _v884), px_str("_val;\n")));
    px_srcline(209);
     _v865 = px_add(_v865, px_str("    return px_null();\n"));
    px_srcline(210);
     _v865 = px_add(_v865, px_str("}\n"));
    px_srcline(211);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(212);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v865));
    px_srcline(213);
    px_set_global("cg_vars", _v866);
    px_srcline(214);
    px_set_global("cg_var_types", _v867);
    px_srcline(215);
    px_set_global("cg_cells", _v868);
    px_srcline(216);
    px_set_global("cg_topbody", _v871);
    px_srcline(217);
    px_set_global("cg_inited", _v869);
    px_srcline(221);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v860}, 1), px_int(0LL)))) {
        px_srcline(222);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\""), _v856), px_str("\", ")), _v858), px_str(", ")), _v862), px_str(")"));
    }
    px_srcline(223);
    return px_add(px_add(px_add(px_add(px_str("px_func(\""), _v856), px_str("\", ")), _v858), px_str(", NULL)"));
px_err_886:
    if (px_err_886_proped) return px_err_886_val;
    return px_null();
}

static LXValue fn_cg_side_effect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_side_effect");
    LXValue _v897 = (nargs > 0) ? args[0] : px_null();
    LXValue _v898 = px_uninit();
    LXValue _v899 = px_uninit();
    LXValue px_err_900_val = px_null();
    int px_err_900_proped = 0;
    px_srcline(237);
    if (px_is_truthy(({ LXValue _t901 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v897}, 1), px_str("list")); px_is_truthy(_t901) ? _t901 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v897}, 1), px_int(0LL)); }))) {
        px_srcline(238);
        return px_bool(false);
    }
    px_srcline(239);
    _v898 = px_index(_v897, px_int(0LL));
    px_srcline(240);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v898}, 1), px_str("string")))) {
        px_srcline(241);
        if (px_is_truthy(({ LXValue _t905 = ({ LXValue _t904 = ({ LXValue _t903 = ({ LXValue _t902 = px_eq(_v898, px_str("Call")); px_is_truthy(_t902) ? _t902 : px_eq(_v898, px_str("Pipe")); }); px_is_truthy(_t903) ? _t903 : px_eq(_v898, px_str("ListComp")); }); px_is_truthy(_t904) ? _t904 : px_eq(_v898, px_str("DictComp")); }); px_is_truthy(_t905) ? _t905 : px_eq(_v898, px_str("GenExp")); }))) {
            px_srcline(242);
            return px_bool(true);
        }
        px_srcline(245);
        if (px_is_truthy(px_eq(_v898, px_str("Closure")))) {
            px_srcline(246);
            return px_bool(false);
        }
    }
    px_srcline(247);
    _v899 = px_int(0LL);
    px_srcline(248);
    while (px_is_truthy(px_lt(_v899, px_call(px_get_global("len"), (LXValue[]){_v897}, 1)))) {
        px_srcline(249);
        if (px_is_truthy(px_call(px_get_global("cg_side_effect"), (LXValue[]){px_index(_v897, _v899)}, 1))) {
            px_srcline(250);
            return px_bool(true);
        }
        px_srcline(251);
         _v899 = px_add(_v899, px_int(1LL));
    }
    px_srcline(252);
    return px_bool(false);
px_err_900:
    if (px_err_900_proped) return px_err_900_val;
    return px_null();
}

static LXValue fn_cg_seq_operands(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_operands");
    LXValue _v906 = (nargs > 0) ? args[0] : px_null();
    LXValue _v907 = (nargs > 1) ? args[1] : px_null();
    LXValue _v908 = px_uninit();
    LXValue _v909 = px_uninit();
    LXValue _v910 = px_uninit();
    LXValue _v911 = px_uninit();
    LXValue _v912 = px_uninit();
    LXValue px_err_913_val = px_null();
    int px_err_913_proped = 0;
    px_srcline(255);
    _v908 = px_int(0LL);
    px_srcline(256);
    _v909 = px_int(0LL);
    px_srcline(257);
    while (px_is_truthy(px_lt(_v909, px_call(px_get_global("len"), (LXValue[]){_v907}, 1)))) {
        px_srcline(258);
        if (px_is_truthy(px_index(_v907, _v909))) {
            px_srcline(259);
             _v908 = px_add(_v908, px_int(1LL));
        }
        px_srcline(260);
         _v909 = px_add(_v909, px_int(1LL));
    }
    px_srcline(261);
    if (px_is_truthy(px_lt(_v908, px_int(2LL)))) {
        px_srcline(262);
        return px_list_n((LXValue[]){px_null(), px_null()}, 2);
    }
    px_srcline(263);
    _v910 = px_str("");
    px_srcline(264);
    _v911 = px_list_n((LXValue[]){}, 0);
    px_srcline(265);
     _v909 = px_int(0LL);
    px_srcline(266);
    while (px_is_truthy(px_lt(_v909, px_call(px_get_global("len"), (LXValue[]){_v906}, 1)))) {
        px_srcline(267);
        if (px_is_truthy(px_index(_v907, _v909))) {
            px_srcline(271);
            px_set_global("cg_seq_uid", px_add(px_get_global("cg_seq_uid"), px_int(1LL)));
            px_srcline(272);
            _v912 = px_add(px_str("_s"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_seq_uid")}, 1));
            px_srcline(273);
             _v910 = px_add(px_add(px_add(px_add(px_add(_v910, px_str("LXValue ")), _v912), px_str(" = ")), px_index(_v906, _v909)), px_str("; "));
            px_srcline(274);
            (void)(px_method(_v911, "append", (LXValue[]){_v912}, 1));
        }
        else {
            px_srcline(276);
            (void)(px_method(_v911, "append", (LXValue[]){px_index(_v906, _v909)}, 1));
        }
        px_srcline(277);
         _v909 = px_add(_v909, px_int(1LL));
    }
    px_srcline(278);
    return px_list_n((LXValue[]){_v910, _v911}, 2);
px_err_913:
    if (px_err_913_proped) return px_err_913_val;
    return px_null();
}

static LXValue fn_cg_seq_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_new");
    LXValue px_err_914_val = px_null();
    int px_err_914_proped = 0;
    px_srcline(287);
    px_set_global("cg_seq_uid", px_add(px_get_global("cg_seq_uid"), px_int(1LL)));
    px_srcline(288);
    return px_add(px_str("_s"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_seq_uid")}, 1));
px_err_914:
    if (px_err_914_proped) return px_err_914_val;
    return px_null();
}

static LXValue fn_cg_seq_join(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_join");
    LXValue _v915 = (nargs > 0) ? args[0] : px_null();
    LXValue _v916 = (nargs > 1) ? args[1] : px_null();
    LXValue _v917 = px_uninit();
    LXValue _v918 = px_uninit();
    LXValue _v919 = px_uninit();
    LXValue px_err_920_val = px_null();
    int px_err_920_proped = 0;
    px_srcline(290);
    _v917 = px_list_n((LXValue[]){}, 0);
    px_srcline(291);
    _v918 = px_int(0LL);
    px_srcline(292);
    while (px_is_truthy(px_lt(_v918, px_call(px_get_global("len"), (LXValue[]){_v915}, 1)))) {
        px_srcline(293);
        (void)(px_method(_v917, "append", (LXValue[]){px_call(px_get_global("cg_side_effect"), (LXValue[]){px_index(_v915, _v918)}, 1)}, 1));
        px_srcline(294);
         _v918 = px_add(_v918, px_int(1LL));
    }
    px_srcline(295);
    _v919 = px_call(px_get_global("cg_seq_operands"), (LXValue[]){_v916, _v917}, 2);
    px_srcline(296);
    if (px_is_truthy(px_eq(px_index(_v919, px_int(0LL)), px_null()))) {
        px_srcline(297);
        return px_list_n((LXValue[]){px_null(), _v916}, 2);
    }
    px_srcline(298);
    return px_list_n((LXValue[]){px_index(_v919, px_int(0LL)), px_index(_v919, px_int(1LL))}, 2);
px_err_920:
    if (px_err_920_proped) return px_err_920_val;
    return px_null();
}

static LXValue fn_cg_seq_wrap(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_seq_wrap");
    LXValue _v921 = (nargs > 0) ? args[0] : px_null();
    LXValue _v922 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_923_val = px_null();
    int px_err_923_proped = 0;
    px_srcline(301);
    if (px_is_truthy(px_eq(_v921, px_null()))) {
        px_srcline(302);
        return _v922;
    }
    px_srcline(303);
    return px_add(px_add(px_add(px_str("({ "), _v921), _v922), px_str("; })"));
px_err_923:
    if (px_err_923_proped) return px_err_923_val;
    return px_null();
}

static LXValue fn_cg_gen_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_expr");
    LXValue _v924 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v979 = px_uninit();
    LXValue _v980 = px_uninit();
    LXValue _v981 = px_uninit();
    LXValue _v982 = px_uninit();
    LXValue _v983 = px_uninit();
    LXValue _v984 = px_uninit();
    LXValue _v985 = px_uninit();
    LXValue px_err_986_val = px_null();
    int px_err_986_proped = 0;
    px_srcline(305);
    _v925 = px_index(_v924, px_int(0LL));
    px_srcline(306);
    if (px_is_truthy(px_eq(_v925, px_str("Int")))) {
        px_srcline(307);
        return px_add(px_add(px_str("px_int("), px_call(px_get_global("str"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1)), px_str("LL)"));
    }
    px_srcline(308);
    if (px_is_truthy(px_eq(_v925, px_str("Float")))) {
        px_srcline(309);
        return px_add(px_add(px_str("px_float("), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1)), px_str(")"));
    }
    px_srcline(310);
    if (px_is_truthy(px_eq(_v925, px_str("Str")))) {
        px_srcline(311);
        _v926 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(315);
        if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){_v926}, 1))) {
            px_srcline(316);
            return px_add(px_add(px_str("PX_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v926}, 1)), px_str("\")"));
        }
        px_srcline(317);
        return px_add(px_add(px_str("px_str(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){_v926}, 1)), px_str("\")"));
    }
    px_srcline(318);
    if (px_is_truthy(px_eq(_v925, px_str("Bool")))) {
        px_srcline(319);
        if (px_is_truthy(px_index(_v924, px_int(1LL)))) {
            px_srcline(320);
            return px_str("px_bool(true)");
        }
        px_srcline(321);
        return px_str("px_bool(false)");
    }
    px_srcline(322);
    if (px_is_truthy(px_eq(_v925, px_str("Null")))) {
        px_srcline(323);
        return px_str("px_null()");
    }
    px_srcline(324);
    if (px_is_truthy(px_eq(_v925, px_str("List")))) {
        px_srcline(325);
        _v927 = px_list_n((LXValue[]){}, 0);
        px_srcline(326);
        _v928 = px_index(_v924, px_int(1LL));
        px_srcline(327);
        _v929 = px_int(0LL);
        px_srcline(328);
        while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v928}, 1)))) {
            px_srcline(329);
            (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v928, _v929)}, 1)}, 1));
            px_srcline(330);
             _v929 = px_add(_v929, px_int(1LL));
        }
        px_srcline(331);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v928, _v927}, 2);
        px_srcline(332);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s153 = px_add(px_add(px_str("px_list_n((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s154 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1); px_add(_s153, _s154); }), px_str(")"))}, 2);
    }
    px_srcline(333);
    if (px_is_truthy(px_eq(_v925, px_str("Tuple")))) {
        px_srcline(334);
        _v927 = px_list_n((LXValue[]){}, 0);
        px_srcline(335);
        _v928 = px_index(_v924, px_int(1LL));
        px_srcline(336);
        _v929 = px_int(0LL);
        px_srcline(337);
        while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v928}, 1)))) {
            px_srcline(338);
            (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v928, _v929)}, 1)}, 1));
            px_srcline(339);
             _v929 = px_add(_v929, px_int(1LL));
        }
        px_srcline(340);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v928, _v927}, 2);
        px_srcline(341);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s155 = px_add(px_add(px_str("px_tuple((LXValue[]){"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s156 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1); px_add(_s155, _s156); }), px_str(")"))}, 2);
    }
    px_srcline(342);
    if (px_is_truthy(px_eq(_v925, px_str("Dict")))) {
        px_srcline(343);
        _v931 = px_str("({ LXValue _d = px_dict(); ");
        px_srcline(344);
        _v932 = px_index(_v924, px_int(1LL));
        px_srcline(345);
        _v929 = px_int(0LL);
        px_srcline(346);
        while (px_is_truthy(px_lt(_v929, px_call(px_get_global("len"), (LXValue[]){_v932}, 1)))) {
            px_srcline(347);
            _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v932, _v929), px_int(0LL))}, 1);
            px_srcline(348);
            _v934 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v932, _v929), px_int(1LL))}, 1);
            px_srcline(353);
             _v931 = px_add(_v931, px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v933), px_str("; LXValue _v = ")), _v934), px_str("; px_dict_set_checked(_d, _k, _v); } ")));
            px_srcline(354);
             _v929 = px_add(_v929, px_int(1LL));
        }
        px_srcline(355);
         _v931 = px_add(_v931, px_str("_d; })"));
        px_srcline(356);
        return _v931;
    }
    px_srcline(357);
    if (px_is_truthy(px_eq(_v925, px_str("Var")))) {
        px_srcline(358);
        _v935 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(359);
        _v936 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v935}, 1);
        px_srcline(360);
        if (px_is_truthy(px_ne(_v936, px_null()))) {
            px_srcline(363);
            return px_call(px_get_global("cg_load_ck"), (LXValue[]){_v935, _v936}, 2);
        }
        px_srcline(364);
        return px_add(px_add(px_str("px_get_global(\""), _v935), px_str("\")"));
    }
    px_srcline(365);
    if (px_is_truthy(px_eq(_v925, px_str("Field")))) {
        px_srcline(366);
        _v937 = px_index(_v924, px_int(1LL));
        px_srcline(367);
        _v938 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(369);
        if (px_is_truthy(px_eq(px_index(_v937, px_int(0LL)), px_str("Var")))) {
            px_srcline(370);
            _v939 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v937, px_int(1LL))}, 1);
            px_srcline(371);
            if (px_is_truthy(({ LXValue _t987 = px_method(px_get_global("cg_const_enums"), "has", (LXValue[]){_v939}, 1); px_is_truthy(_t987) ? px_method(px_index(px_get_global("cg_const_enums"), _v939), "has", (LXValue[]){_v938}, 1) : _t987; }))) {
                px_srcline(372);
                return px_index(px_index(px_get_global("cg_const_enums"), _v939), _v938);
            }
        }
        px_srcline(374);
        if (px_is_truthy(px_eq(px_index(_v937, px_int(0LL)), px_str("Var")))) {
            px_srcline(375);
            _v939 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v937, px_int(1LL))}, 1);
            px_srcline(376);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v939}, 1))) {
                px_srcline(377);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v939), px_str("\", \"")), _v938), px_str("\")"));
            }
        }
        px_srcline(378);
        _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v937}, 1);
        px_srcline(379);
        return px_add(px_add(px_add(px_add(px_str("px_field("), _v940), px_str(", \"")), _v938), px_str("\")"));
    }
    px_srcline(380);
    if (px_is_truthy(px_eq(_v925, px_str("OptionalField")))) {
        px_srcline(385);
        _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(386);
        _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(387);
        _v938 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(388);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v940), px_str("; px_is_null(")), _v941), px_str(") ? px_null() : px_field(")), _v941), px_str(", \"")), _v938), px_str("\"); })"));
    }
    px_srcline(389);
    if (px_is_truthy(px_eq(_v925, px_str("Index")))) {
        px_srcline(390);
        _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(391);
        _v929 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(392);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v924, px_int(1LL)), px_index(_v924, px_int(2LL))}, 2), px_list_n((LXValue[]){_v940, _v929}, 2)}, 2);
        px_srcline(393);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_index("), px_index(px_index(_v930, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v930, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2);
    }
    px_srcline(394);
    if (px_is_truthy(px_eq(_v925, px_str("Slice")))) {
        px_srcline(395);
        _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(396);
        _v931 = px_str("px_null()");
        px_srcline(397);
        if (px_is_truthy(px_ne(px_index(_v924, px_int(2LL)), px_null()))) {
            px_srcline(398);
             _v931 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        }
        px_srcline(399);
        _v942 = px_str("px_null()");
        px_srcline(400);
        if (px_is_truthy(px_ne(px_index(_v924, px_int(3LL)), px_null()))) {
            px_srcline(401);
             _v942 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(3LL))}, 1);
        }
        px_srcline(402);
        _v943 = px_str("px_null()");
        px_srcline(403);
        if (px_is_truthy(px_ne(px_index(_v924, px_int(4LL)), px_null()))) {
            px_srcline(404);
             _v943 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(4LL))}, 1);
        }
        px_srcline(405);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v924, px_int(1LL)), px_index(_v924, px_int(2LL)), px_index(_v924, px_int(3LL)), px_index(_v924, px_int(4LL))}, 4), px_list_n((LXValue[]){_v940, _v931, _v942, _v943}, 4)}, 2);
        px_srcline(406);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_slice("), px_index(px_index(_v930, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v930, px_int(1LL)), px_int(1LL))), px_str(", ")), px_index(px_index(_v930, px_int(1LL)), px_int(2LL))), px_str(", ")), px_index(px_index(_v930, px_int(1LL)), px_int(3LL))), px_str(")"))}, 2);
    }
    px_srcline(407);
    if (px_is_truthy(px_eq(_v925, px_str("Call")))) {
        px_srcline(408);
        _v944 = px_index(_v924, px_int(1LL));
        px_srcline(409);
        _v945 = px_index(_v924, px_int(2LL));
        px_srcline(410);
        if (px_is_truthy(px_eq(px_index(_v944, px_int(0LL)), px_str("Var")))) {
            px_srcline(411);
            _v946 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v944, px_int(1LL))}, 1);
            px_srcline(414);
            (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v944, _v945}, 2));
            px_srcline(415);
            if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v946}, 1))) {
                px_srcline(416);
                _v947 = px_index(px_get_global("cg_ffi"), _v946);
                px_srcline(417);
                _v927 = px_list_n((LXValue[]){}, 0);
                px_srcline(418);
                _v948 = px_int(0LL);
                px_srcline(419);
                while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
                    px_srcline(420);
                    (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
                    px_srcline(421);
                     _v948 = px_add(_v948, px_int(1LL));
                }
                px_srcline(422);
                _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v945, _v927}, 2);
                px_srcline(423);
                return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s157 = px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\"ffi_call\"), (LXValue[]){px_str(\""), _v946), px_str("\"), px_list_n((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s158 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1); px_add(_s157, _s158); }), px_str(")}, 2)"))}, 2);
            }
            px_srcline(424);
            if (px_is_truthy(px_eq(_v946, px_str("chan")))) {
                px_srcline(425);
                _v949 = px_str("0");
                px_srcline(426);
                if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v945}, 1), px_int(0LL)))) {
                    px_srcline(427);
                    if (px_is_truthy(px_eq(px_index(px_index(_v945, px_int(0LL)), px_int(0LL)), px_str("Int")))) {
                        px_srcline(428);
                         _v949 = px_call(px_get_global("str"), (LXValue[]){px_index(px_index(_v945, px_int(0LL)), px_int(1LL))}, 1);
                    }
                    else {
                        px_srcline(430);
                         _v949 = px_add(px_add(px_str("(int)("), px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, px_int(0LL))}, 1)), px_str(").as.i"));
                    }
                }
                px_srcline(431);
                return px_add(px_add(px_str("px_chan_create("), _v949), px_str(")"));
            }
            px_srcline(432);
            if (px_is_truthy(px_eq(_v946, px_str("mutex")))) {
                px_srcline(433);
                return px_str("px_mutex_create()");
            }
            px_srcline(434);
            if (px_is_truthy(px_eq(_v946, px_str("rwlock")))) {
                px_srcline(435);
                return px_str("px_rwlock_create()");
            }
            px_srcline(437);
            if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v946}, 1))) {
                px_srcline(438);
                _v950 = px_index(px_get_global("cg_structs"), _v946);
                px_srcline(439);
                if (px_is_truthy(({ LXValue _s159 = px_call(px_get_global("len"), (LXValue[]){_v950}, 1); LXValue _s160 = px_call(px_get_global("len"), (LXValue[]){_v945}, 1); px_ne(_s159, _s160); }))) {
                    px_srcline(440);
                    return ({ LXValue _s161 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v946), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v950}, 1)}, 1)), px_str(" 个字段，给出 ")); LXValue _s162 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v945}, 1)}, 1); px_add(_s161, _s162); });
                }
                px_srcline(441);
                _v927 = px_list_n((LXValue[]){}, 0);
                px_srcline(442);
                _v948 = px_int(0LL);
                px_srcline(443);
                while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
                    px_srcline(444);
                    (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
                    px_srcline(445);
                     _v948 = px_add(_v948, px_int(1LL));
                }
                px_srcline(446);
                _v951 = px_list_n((LXValue[]){}, 0);
                px_srcline(447);
                _v952 = px_int(0LL);
                px_srcline(448);
                while (px_is_truthy(px_lt(_v952, px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
                    px_srcline(449);
                    (void)(px_method(_v951, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v950, _v952)), px_str("\""))}, 1));
                    px_srcline(450);
                     _v952 = px_add(_v952, px_int(1LL));
                }
                px_srcline(451);
                _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v945, _v927}, 2);
                px_srcline(452);
                return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s165 = px_add(({ LXValue _s163 = px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v946), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v951}, 2)), px_str("}, (LXValue[]){")); LXValue _s164 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2); px_add(_s163, _s164); }), px_str("}, ")); LXValue _s166 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v950}, 1)}, 1); px_add(_s165, _s166); }), px_str(")"))}, 2);
            }
            px_srcline(453);
            if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v946}, 1))) {
                px_srcline(454);
                if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v945}, 1), px_int(1LL)))) {
                    px_srcline(455);
                    return px_add(px_add(px_str("枚举 "), _v946), px_str(" 构造需要一个变体名"));
                }
                px_srcline(456);
                _v936 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, px_int(0LL))}, 1);
                px_srcline(457);
                return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v946), px_str("\", (")), _v936), px_str(").as.obj->as.enum_inst.variant)"));
            }
        }
        px_srcline(459);
        if (px_is_truthy(px_eq(px_index(_v944, px_int(0LL)), px_str("Field")))) {
            px_srcline(460);
            _v937 = px_index(_v944, px_int(1LL));
            px_srcline(461);
            _v953 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v944, px_int(2LL))}, 1);
            px_srcline(463);
            _v954 = px_null();
            px_srcline(464);
            if (px_is_truthy(px_eq(px_index(_v937, px_int(0LL)), px_str("Var")))) {
                px_srcline(465);
                _v939 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v937, px_int(1LL))}, 1);
                px_srcline(466);
                if (px_is_truthy(px_method(px_get_global("cg_var_types"), "has", (LXValue[]){_v939}, 1))) {
                    px_srcline(467);
                     _v954 = px_index(px_get_global("cg_var_types"), _v939);
                }
            }
            px_srcline(468);
            if (px_is_truthy(({ LXValue _t988 = px_ne(_v954, px_null()); px_is_truthy(_t988) ? px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v954}, 1) : _t988; }))) {
                px_srcline(469);
                _v955 = px_index(px_get_global("cg_impls"), _v954);
                px_srcline(470);
                _v956 = px_bool(false);
                px_srcline(471);
                _v957 = px_int(0LL);
                px_srcline(472);
                while (px_is_truthy(px_lt(_v957, px_call(px_get_global("len"), (LXValue[]){_v955}, 1)))) {
                    px_srcline(473);
                    if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v955, _v957), px_int(1LL))}, 1), _v953))) {
                        px_srcline(474);
                         _v956 = px_bool(true);
                        px_srcline(475);
                        break;
                    }
                    px_srcline(476);
                     _v957 = px_add(_v957, px_int(1LL));
                }
                px_srcline(477);
                if (px_is_truthy(_v956)) {
                    px_srcline(478);
                    _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v937}, 1);
                    px_srcline(479);
                    _v927 = px_list_n((LXValue[]){_v940}, 1);
                    px_srcline(480);
                    _v958 = px_list_n((LXValue[]){_v937}, 1);
                    px_srcline(481);
                    _v948 = px_int(0LL);
                    px_srcline(482);
                    while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
                        px_srcline(483);
                        (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
                        px_srcline(484);
                        (void)(px_method(_v958, "append", (LXValue[]){px_index(_v945, _v948)}, 1));
                        px_srcline(485);
                         _v948 = px_add(_v948, px_int(1LL));
                    }
                    px_srcline(486);
                    _v959 = ({ LXValue _s167 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v954}, 1)), px_str("_")); LXValue _s168 = px_call(px_get_global("cg_func_cname"), (LXValue[]){_v953}, 1); px_add(_s167, _s168); });
                    px_srcline(487);
                    _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v958, _v927}, 2);
                    px_srcline(488);
                    return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s169 = px_add(px_add(px_add(_v959, px_str("((LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s170 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1); px_add(_s169, _s170); }), px_str(", NULL)"))}, 2);
                }
            }
            px_srcline(492);
            _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v937}, 1);
            px_srcline(493);
            _v927 = px_list_n((LXValue[]){_v940}, 1);
            px_srcline(494);
            _v958 = px_list_n((LXValue[]){_v937}, 1);
            px_srcline(495);
            _v948 = px_int(0LL);
            px_srcline(496);
            while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
                px_srcline(497);
                (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
                px_srcline(498);
                (void)(px_method(_v958, "append", (LXValue[]){px_index(_v945, _v948)}, 1));
                px_srcline(499);
                 _v948 = px_add(_v948, px_int(1LL));
            }
            px_srcline(500);
            _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v958, _v927}, 2);
            px_srcline(501);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s171 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_method("), px_index(px_index(_v930, px_int(1LL)), px_int(0LL))), px_str(", \"")), _v953), px_str("\", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_slice(px_index(_v930, px_int(1LL)), px_int(1LL), px_null(), px_null())}, 2)), px_str("}, ")); LXValue _s172 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v945}, 1)}, 1); px_add(_s171, _s172); }), px_str(")"))}, 2);
        }
        px_srcline(505);
        _v960 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v944}, 1);
        px_srcline(506);
        _v927 = px_list_n((LXValue[]){_v960}, 1);
        px_srcline(507);
        _v958 = px_list_n((LXValue[]){_v944}, 1);
        px_srcline(508);
        _v948 = px_int(0LL);
        px_srcline(509);
        while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
            px_srcline(510);
            (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
            px_srcline(511);
            (void)(px_method(_v958, "append", (LXValue[]){px_index(_v945, _v948)}, 1));
            px_srcline(512);
             _v948 = px_add(_v948, px_int(1LL));
        }
        px_srcline(513);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v958, _v927}, 2);
        px_srcline(514);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s173 = px_add(px_add(px_add(px_add(px_str("px_call("), px_index(px_index(_v930, px_int(1LL)), px_int(0LL))), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_slice(px_index(_v930, px_int(1LL)), px_int(1LL), px_null(), px_null())}, 2)), px_str("}, ")); LXValue _s174 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v945}, 1)}, 1); px_add(_s173, _s174); }), px_str(")"))}, 2);
    }
    px_srcline(515);
    if (px_is_truthy(px_eq(_v925, px_str("Unary")))) {
        px_srcline(516);
        _v940 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(517);
        _v961 = px_index(_v924, px_int(1LL));
        px_srcline(518);
        if (px_is_truthy(px_eq(_v961, px_str("Neg")))) {
            px_srcline(519);
            return px_add(px_add(px_str("px_neg("), _v940), px_str(")"));
        }
        px_srcline(520);
        if (px_is_truthy(px_eq(_v961, px_str("Not")))) {
            px_srcline(521);
            return px_add(px_add(px_str("px_not("), _v940), px_str(")"));
        }
        px_srcline(522);
        return px_add(px_add(px_str("px_bitnot("), _v940), px_str(")"));
    }
    px_srcline(523);
    if (px_is_truthy(px_eq(_v925, px_str("Binary")))) {
        px_srcline(524);
        _v962 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(525);
        _v963 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(3LL))}, 1);
        px_srcline(526);
        _v961 = px_index(_v924, px_int(1LL));
        px_srcline(527);
        if (px_is_truthy(px_eq(_v961, px_str("And")))) {
            px_srcline(528);
            _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(529);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v962), px_str("; px_is_truthy(")), _v941), px_str(") ? ")), _v963), px_str(" : ")), _v941), px_str("; })"));
        }
        px_srcline(530);
        if (px_is_truthy(px_eq(_v961, px_str("Or")))) {
            px_srcline(531);
            _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
            px_srcline(532);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v962), px_str("; px_is_truthy(")), _v941), px_str(") ? ")), _v941), px_str(" : ")), _v963), px_str("; })"));
        }
        px_srcline(533);
        _v964 = px_call(px_get_global("cg_binop_cname"), (LXValue[]){_v961}, 1);
        px_srcline(534);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v924, px_int(2LL)), px_index(_v924, px_int(3LL))}, 2), px_list_n((LXValue[]){_v962, _v963}, 2)}, 2);
        px_srcline(535);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(px_add(px_add(px_add(px_add(_v964, px_str("(")), px_index(px_index(_v930, px_int(1LL)), px_int(0LL))), px_str(", ")), px_index(px_index(_v930, px_int(1LL)), px_int(1LL))), px_str(")"))}, 2);
    }
    px_srcline(536);
    if (px_is_truthy(px_eq(_v925, px_str("Pipe")))) {
        px_srcline(537);
        _v936 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(538);
        _v965 = px_index(_v924, px_int(2LL));
        px_srcline(539);
        if (px_is_truthy(px_eq(px_index(_v965, px_int(0LL)), px_str("Call")))) {
            px_srcline(540);
            _v960 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v965, px_int(1LL))}, 1);
            px_srcline(541);
            _v927 = px_list_n((LXValue[]){_v936}, 1);
            px_srcline(542);
            _v958 = px_list_n((LXValue[]){px_index(_v924, px_int(1LL))}, 1);
            px_srcline(543);
            _v948 = px_int(0LL);
            px_srcline(544);
            while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){px_index(_v965, px_int(2LL))}, 1)))) {
                px_srcline(545);
                (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v965, px_int(2LL)), _v948)}, 1)}, 1));
                px_srcline(546);
                (void)(px_method(_v958, "append", (LXValue[]){px_index(px_index(_v965, px_int(2LL)), _v948)}, 1));
                px_srcline(547);
                 _v948 = px_add(_v948, px_int(1LL));
            }
            px_srcline(548);
            _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v958, _v927}, 2);
            px_srcline(549);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s175 = px_add(px_add(px_add(px_add(px_str("px_call("), _v960), px_str(", (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s176 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1); px_add(_s175, _s176); }), px_str(")"))}, 2);
        }
        px_srcline(550);
        _v964 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v965}, 1);
        px_srcline(551);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){px_list_n((LXValue[]){px_index(_v924, px_int(1LL)), px_index(_v924, px_int(2LL))}, 2), px_list_n((LXValue[]){_v936, _v964}, 2)}, 2);
        px_srcline(552);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(px_add(px_add(px_add(px_str("px_call("), px_index(px_index(_v930, px_int(1LL)), px_int(1LL))), px_str(", (LXValue[]){")), px_index(px_index(_v930, px_int(1LL)), px_int(0LL))), px_str("}, 1)"))}, 2);
    }
    px_srcline(553);
    if (px_is_truthy(px_eq(_v925, px_str("NullCoalesce")))) {
        px_srcline(554);
        _v962 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(555);
        _v963 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(556);
        _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(557);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v962), px_str("; px_is_null(")), _v941), px_str(") ? ")), _v963), px_str(" : ")), _v941), px_str("; })"));
    }
    px_srcline(558);
    if (px_is_truthy(px_eq(_v925, px_str("Try")))) {
        px_srcline(559);
        _v942 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(560);
        _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(561);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(0LL)))) {
            px_srcline(562);
            _v966 = px_index(px_get_global("cg_err_labels"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)));
            px_srcline(563);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v942), px_str("; if (px_is_result(")), _v941), px_str(")) { if (!px_result_ok(")), _v941), px_str(")) { ")), _v966), px_str("_val = ")), _v941), px_str("; ")), _v966), px_str("_proped = 1; goto ")), _v966), px_str("; } ")), _v941), px_str(" = px_result_unwrap(")), _v941), px_str("); } else if (px_is_null(")), _v941), px_str(")) { ")), _v966), px_str("_val = px_null(); ")), _v966), px_str("_proped = 1; goto ")), _v966), px_str("; } ")), _v941), px_str("; })"));
        }
        px_srcline(567);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v942), px_str("; if (px_is_result(")), _v941), px_str(") && !px_result_ok(")), _v941), px_str(")) px_error(\"R1004: 顶层不能使用错误传播 ?（仅函数内可用）\"); if (px_is_null(")), _v941), px_str(")) px_error(\"R1004: 顶层不能使用错误传播 ?（仅函数内可用）\"); if (px_is_result(")), _v941), px_str(")) ")), _v941), px_str(" = px_result_unwrap(")), _v941), px_str("); ")), _v941), px_str("; })"));
    }
    px_srcline(568);
    if (px_is_truthy(px_eq(_v925, px_str("ForceUnwrap")))) {
        px_srcline(569);
        _v942 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(570);
        _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(574);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v942), px_str("; if (px_is_result(")), _v941), px_str(")) { if (!px_result_ok(")), _v941), px_str(")) px_error(\"R1004: 强制解包 !: 值为 Err(%s)\", px_to_string(px_result_unwrap(")), _v941), px_str("))); ")), _v941), px_str(" = px_result_unwrap(")), _v941), px_str("); } if (px_is_null(")), _v941), px_str(")) px_error(\"R1004: 强制解包 !: 值为 null\"); ")), _v941), px_str("; })"));
    }
    px_srcline(575);
    if (px_is_truthy(px_eq(_v925, px_str("IfExpr")))) {
        px_srcline(576);
        _v960 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(577);
        _v967 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(578);
        _v968 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(3LL))}, 1);
        px_srcline(579);
        _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(580);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str("; if (px_is_truthy(")), _v960), px_str(")) { ")), _v941), px_str(" = ")), _v967), px_str("; } else { ")), _v941), px_str(" = ")), _v968), px_str("; } ")), _v941), px_str("; })"));
    }
    px_srcline(581);
    if (px_is_truthy(px_eq(_v925, px_str("ListComp")))) {
        px_srcline(582);
        _v969 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(583);
        _v963 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(584);
        _v942 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(585);
        _v970 = px_null();
        px_srcline(586);
        if (px_is_truthy(px_ne(px_index(_v924, px_int(3LL)), px_null()))) {
            px_srcline(587);
             _v970 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(3LL))}, 1);
        }
        px_srcline(588);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v963, px_str("saved_all"))}, 1));
        px_srcline(589);
        _v971 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v969), px_str(", ")), _v942), px_str("); "));
        px_srcline(590);
        _v972 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v963, _v970, _v971}, 3);
        px_srcline(591);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v969), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v963, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v963, px_str("its")), px_int(0LL))), px_str("; ")), _v972), px_str(" ")), _v969), px_str("; })"));
    }
    px_srcline(592);
    if (px_is_truthy(px_eq(_v925, px_str("GenExp")))) {
        px_srcline(593);
        _v973 = px_index(_v924, px_int(2LL));
        px_srcline(594);
        if (px_is_truthy(({ LXValue _t989 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v973}, 1), px_int(1LL)); px_is_truthy(_t989) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v973, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t989; }))) {
            px_srcline(595);
            _v974 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v973, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
            px_srcline(596);
            _v975 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v973, px_int(0LL)), px_int(2LL))}, 1);
            px_srcline(597);
            _v976 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v974}, 1), px_index(_v924, px_int(1LL))}, 2);
            px_srcline(598);
            _v977 = px_str("px_null()");
            px_srcline(599);
            if (px_is_truthy(px_ne(px_index(_v924, px_int(3LL)), px_null()))) {
                px_srcline(600);
                 _v977 = px_call(px_get_global("cg_gen_lambda"), (LXValue[]){px_list_n((LXValue[]){_v974}, 1), px_index(_v924, px_int(3LL))}, 2);
            }
            px_srcline(601);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_gen_lazy("), _v975), px_str(", ")), _v976), px_str(", ")), _v977), px_str(")"));
        }
        px_srcline(603);
        _v969 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(604);
        _v963 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){_v973}, 1);
        px_srcline(605);
        _v942 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(606);
        _v970 = px_null();
        px_srcline(607);
        if (px_is_truthy(px_ne(px_index(_v924, px_int(3LL)), px_null()))) {
            px_srcline(608);
             _v970 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(3LL))}, 1);
        }
        px_srcline(609);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v963, px_str("saved_all"))}, 1));
        px_srcline(610);
        _v971 = px_add(px_add(px_add(px_add(px_str("px_list_push("), _v969), px_str(", ")), _v942), px_str("); "));
        px_srcline(611);
        _v972 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v963, _v970, _v971}, 3);
        px_srcline(612);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v969), px_str(" = px_list(0); LXValue ")), px_index(px_index(_v963, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v963, px_str("its")), px_int(0LL))), px_str("; ")), _v972), px_str(" px_gen_from_list(")), _v969), px_str("); })"));
    }
    px_srcline(613);
    if (px_is_truthy(px_eq(_v925, px_str("DictComp")))) {
        px_srcline(614);
        _v969 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(615);
        _v963 = px_call(px_get_global("cg_comp_collect"), (LXValue[]){px_index(_v924, px_int(3LL))}, 1);
        px_srcline(616);
        _v933 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(617);
        _v934 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(2LL))}, 1);
        px_srcline(618);
        _v970 = px_null();
        px_srcline(619);
        if (px_is_truthy(px_ne(px_index(_v924, px_int(4LL)), px_null()))) {
            px_srcline(620);
             _v970 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(4LL))}, 1);
        }
        px_srcline(621);
        (void)(px_call(px_get_global("cg_comp_restore"), (LXValue[]){px_index(_v963, px_str("saved_all"))}, 1));
        px_srcline(622);
        _v971 = px_add(px_add(px_add(px_add(px_add(px_add(px_str("{ LXValue _k = "), _v933), px_str("; LXValue _v = ")), _v934), px_str("; px_dict_set_checked(")), _v969), px_str(", _k, _v); } "));
        px_srcline(623);
        _v972 = px_call(px_get_global("cg_comp_body"), (LXValue[]){_v963, _v970, _v971}, 3);
        px_srcline(624);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v969), px_str(" = px_dict(); LXValue ")), px_index(px_index(_v963, px_str("ivs")), px_int(0LL))), px_str(" = ")), px_index(px_index(_v963, px_str("its")), px_int(0LL))), px_str("; ")), _v972), px_str(" ")), _v969), px_str("; })"));
    }
    px_srcline(625);
    if (px_is_truthy(px_eq(_v925, px_str("Closure")))) {
        px_srcline(628);
        return px_call(px_get_global("cg_gen_closure"), (LXValue[]){px_index(_v924, px_int(1LL)), px_index(_v924, px_int(3LL)), px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_add(px_get_global("cg_closure_id"), px_int(1LL))}, 1)), px_str(">"))}, 3);
    }
    px_srcline(629);
    if (px_is_truthy(px_eq(_v925, px_str("Block")))) {
        px_srcline(630);
        _v931 = px_str("({ ");
        px_srcline(631);
         _v931 = px_add(_v931, px_str("LXValue _blk = px_null(); "));
        px_srcline(632);
        _v978 = px_index(_v924, px_int(1LL));
        px_srcline(633);
        _v979 = px_int(0LL);
        px_srcline(634);
        while (px_is_truthy(px_lt(_v979, px_call(px_get_global("len"), (LXValue[]){_v978}, 1)))) {
            px_srcline(635);
            _v980 = px_index(_v978, _v979);
            px_srcline(636);
            if (px_is_truthy(px_eq(px_index(_v980, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(637);
                _v942 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v980, px_int(1LL))}, 1);
                px_srcline(638);
                 _v931 = px_add(_v931, px_add(px_add(px_str("_blk = "), _v942), px_str("; ")));
            }
            else {
                px_srcline(640);
                 _v931 = px_add(_v931, px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v980, px_int(0LL)}, 2));
            }
            px_srcline(641);
             _v979 = px_add(_v979, px_int(1LL));
        }
        px_srcline(642);
         _v931 = px_add(_v931, px_str("_blk; })"));
        px_srcline(643);
        return _v931;
    }
    px_srcline(644);
    if (px_is_truthy(px_eq(_v925, px_str("Match")))) {
        px_srcline(645);
        _v981 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(646);
        _v941 = px_call(px_get_global("cg_tmp"), (LXValue[]){}, 0);
        px_srcline(647);
        _v931 = px_add(px_add(px_add(px_add(px_str("({ LXValue "), _v941), px_str(" = ")), _v981), px_str("; "));
        px_srcline(648);
        _v982 = px_index(_v924, px_int(2LL));
        px_srcline(649);
        _v983 = px_bool(true);
        px_srcline(650);
        _v948 = px_int(0LL);
        px_srcline(651);
        while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v982}, 1)))) {
            px_srcline(652);
            _v970 = px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(px_index(_v982, _v948), px_int(1LL)), _v941}, 2);
            px_srcline(653);
            _v984 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(px_index(_v982, _v948), px_int(3LL))}, 1);
            px_srcline(654);
            _v985 = px_str("if");
            px_srcline(655);
            if (px_is_truthy(px_not(_v983))) {
                px_srcline(656);
                 _v985 = px_str("else if");
            }
            px_srcline(657);
             _v931 = px_add(_v931, px_add(px_add(px_add(px_add(px_add(px_add(px_add(_v985, px_str(" (")), _v970), px_str(") { ")), _v941), px_str(" = ")), _v984), px_str("; } ")));
            px_srcline(658);
             _v983 = px_bool(false);
            px_srcline(659);
             _v948 = px_add(_v948, px_int(1LL));
        }
        px_srcline(660);
         _v931 = px_add(_v931, px_add(_v941, px_str("; })")));
        px_srcline(661);
        return _v931;
    }
    px_srcline(662);
    if (px_is_truthy(px_eq(_v925, px_str("Constructor")))) {
        px_srcline(663);
        _v935 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v924, px_int(1LL))}, 1);
        px_srcline(664);
        _v945 = px_index(_v924, px_int(2LL));
        px_srcline(665);
        if (px_is_truthy(px_method(px_get_global("cg_structs"), "has", (LXValue[]){_v935}, 1))) {
            px_srcline(666);
            _v950 = px_index(px_get_global("cg_structs"), _v935);
            px_srcline(667);
            if (px_is_truthy(({ LXValue _s177 = px_call(px_get_global("len"), (LXValue[]){_v950}, 1); LXValue _s178 = px_call(px_get_global("len"), (LXValue[]){_v945}, 1); px_ne(_s177, _s178); }))) {
                px_srcline(668);
                return ({ LXValue _s179 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v935), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v950}, 1)}, 1)), px_str(" 个字段，给出 ")); LXValue _s180 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v945}, 1)}, 1); px_add(_s179, _s180); });
            }
            px_srcline(669);
            _v927 = px_list_n((LXValue[]){}, 0);
            px_srcline(670);
            _v948 = px_int(0LL);
            px_srcline(671);
            while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
                px_srcline(672);
                (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
                px_srcline(673);
                 _v948 = px_add(_v948, px_int(1LL));
            }
            px_srcline(674);
            _v951 = px_list_n((LXValue[]){}, 0);
            px_srcline(675);
            _v952 = px_int(0LL);
            px_srcline(676);
            while (px_is_truthy(px_lt(_v952, px_call(px_get_global("len"), (LXValue[]){_v950}, 1)))) {
                px_srcline(677);
                (void)(px_method(_v951, "append", (LXValue[]){px_add(px_add(px_str("\""), px_index(_v950, _v952)), px_str("\""))}, 1));
                px_srcline(678);
                 _v952 = px_add(_v952, px_int(1LL));
            }
            px_srcline(679);
            _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v945, _v927}, 2);
            px_srcline(680);
            return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s183 = px_add(({ LXValue _s181 = px_add(px_add(px_add(px_add(px_str("px_struct(\""), _v935), px_str("\", (char*[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v951}, 2)), px_str("}, (LXValue[]){")); LXValue _s182 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2); px_add(_s181, _s182); }), px_str("}, ")); LXValue _s184 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v950}, 1)}, 1); px_add(_s183, _s184); }), px_str(")"))}, 2);
        }
        px_srcline(681);
        if (px_is_truthy(px_method(px_get_global("cg_enums"), "has", (LXValue[]){_v935}, 1))) {
            px_srcline(682);
            if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v945}, 1), px_int(1LL)))) {
                px_srcline(683);
                return px_add(px_add(px_str("枚举 "), _v935), px_str(" 构造需要一个变体名"));
            }
            px_srcline(684);
            _v936 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, px_int(0LL))}, 1);
            px_srcline(685);
            return px_add(px_add(px_add(px_add(px_str("px_enum(\""), _v935), px_str("\", (")), _v936), px_str(").as.obj->as.enum_inst.variant)"));
        }
        px_srcline(686);
        _v927 = px_list_n((LXValue[]){}, 0);
        px_srcline(687);
        _v948 = px_int(0LL);
        px_srcline(688);
        while (px_is_truthy(px_lt(_v948, px_call(px_get_global("len"), (LXValue[]){_v945}, 1)))) {
            px_srcline(689);
            (void)(px_method(_v927, "append", (LXValue[]){px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v945, _v948)}, 1)}, 1));
            px_srcline(690);
             _v948 = px_add(_v948, px_int(1LL));
        }
        px_srcline(691);
        _v930 = px_call(px_get_global("cg_seq_join"), (LXValue[]){_v945, _v927}, 2);
        px_srcline(692);
        return px_call(px_get_global("cg_seq_wrap"), (LXValue[]){px_index(_v930, px_int(0LL)), px_add(({ LXValue _s185 = px_add(px_add(px_add(px_add(px_str("px_call(px_get_global(\""), _v935), px_str("\"), (LXValue[]){")), px_call(px_get_global("join"), (LXValue[]){px_str(", "), px_index(_v930, px_int(1LL))}, 2)), px_str("}, ")); LXValue _s186 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v927}, 1)}, 1); px_add(_s185, _s186); }), px_str(")"))}, 2);
    }
    px_srcline(693);
    return px_str("px_null()");
px_err_986:
    if (px_err_986_proped) return px_err_986_val;
    return px_null();
}

static LXValue fn_cg_binop_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_binop_cname");
    LXValue _v990 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_991_val = px_null();
    int px_err_991_proped = 0;
    px_srcline(696);
    if (px_is_truthy(px_eq(_v990, px_str("Add")))) {
        px_srcline(697);
        return px_str("px_add");
    }
    px_srcline(698);
    if (px_is_truthy(px_eq(_v990, px_str("Sub")))) {
        px_srcline(699);
        return px_str("px_sub");
    }
    px_srcline(700);
    if (px_is_truthy(px_eq(_v990, px_str("Mul")))) {
        px_srcline(701);
        return px_str("px_mul");
    }
    px_srcline(702);
    if (px_is_truthy(px_eq(_v990, px_str("Div")))) {
        px_srcline(703);
        return px_str("px_div");
    }
    px_srcline(704);
    if (px_is_truthy(px_eq(_v990, px_str("IntDiv")))) {
        px_srcline(705);
        return px_str("px_idiv");
    }
    px_srcline(706);
    if (px_is_truthy(px_eq(_v990, px_str("Mod")))) {
        px_srcline(707);
        return px_str("px_mod");
    }
    px_srcline(708);
    if (px_is_truthy(px_eq(_v990, px_str("Pow")))) {
        px_srcline(709);
        return px_str("px_pow");
    }
    px_srcline(710);
    if (px_is_truthy(px_eq(_v990, px_str("Eq")))) {
        px_srcline(711);
        return px_str("px_eq");
    }
    px_srcline(712);
    if (px_is_truthy(px_eq(_v990, px_str("Ne")))) {
        px_srcline(713);
        return px_str("px_ne");
    }
    px_srcline(714);
    if (px_is_truthy(px_eq(_v990, px_str("Lt")))) {
        px_srcline(715);
        return px_str("px_lt");
    }
    px_srcline(716);
    if (px_is_truthy(px_eq(_v990, px_str("Le")))) {
        px_srcline(717);
        return px_str("px_le");
    }
    px_srcline(718);
    if (px_is_truthy(px_eq(_v990, px_str("Gt")))) {
        px_srcline(719);
        return px_str("px_gt");
    }
    px_srcline(720);
    if (px_is_truthy(px_eq(_v990, px_str("Ge")))) {
        px_srcline(721);
        return px_str("px_ge");
    }
    px_srcline(722);
    if (px_is_truthy(px_eq(_v990, px_str("BitAnd")))) {
        px_srcline(723);
        return px_str("px_bitand");
    }
    px_srcline(724);
    if (px_is_truthy(px_eq(_v990, px_str("BitOr")))) {
        px_srcline(725);
        return px_str("px_bitor");
    }
    px_srcline(726);
    if (px_is_truthy(px_eq(_v990, px_str("BitXor")))) {
        px_srcline(727);
        return px_str("px_bitxor");
    }
    px_srcline(728);
    if (px_is_truthy(px_eq(_v990, px_str("Shl")))) {
        px_srcline(729);
        return px_str("px_shl");
    }
    px_srcline(730);
    if (px_is_truthy(px_eq(_v990, px_str("Shr")))) {
        px_srcline(731);
        return px_str("px_shr");
    }
    px_srcline(732);
    if (px_is_truthy(px_eq(_v990, px_str("ShrU")))) {
        px_srcline(733);
        return px_str("px_ushr");
    }
    px_srcline(734);
    return px_str("px_add");
px_err_991:
    if (px_err_991_proped) return px_err_991_val;
    return px_null();
}

static LXValue fn_cg_gen_pattern_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_pattern_cond");
    LXValue _v992 = (nargs > 0) ? args[0] : px_null();
    LXValue _v993 = (nargs > 1) ? args[1] : px_null();
    LXValue _v994 = px_uninit();
    LXValue _v995 = px_uninit();
    LXValue _v996 = px_uninit();
    LXValue _v997 = px_uninit();
    LXValue px_err_998_val = px_null();
    int px_err_998_proped = 0;
    px_srcline(737);
    _v994 = px_index(_v992, px_int(0LL));
    px_srcline(738);
    if (px_is_truthy(px_eq(_v994, px_str("PatLiteral")))) {
        px_srcline(739);
        _v995 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v992, px_int(1LL))}, 1);
        px_srcline(740);
        return px_add(px_add(px_add(px_add(px_str("px_is_truthy(px_eq("), _v993), px_str(", ")), _v995), px_str("))"));
    }
    px_srcline(741);
    if (px_is_truthy(px_eq(_v994, px_str("PatBinding")))) {
        px_srcline(742);
        _v996 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v992, px_int(1LL))}, 1);
        px_srcline(743);
        if (px_is_truthy(({ LXValue _t1000 = ({ LXValue _t999 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v996}, 1), px_int(0LL)); px_is_truthy(_t999) ? px_ge(px_index(_v996, px_int(0LL)), px_str("A")) : _t999; }); px_is_truthy(_t1000) ? px_le(px_index(_v996, px_int(0LL)), px_str("Z")) : _t1000; }))) {
            px_srcline(744);
            return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v993), px_str(".type == PX_ENUM && strcmp(")), _v993), px_str(".as.obj->as.enum_inst.variant, \"")), _v996), px_str("\") == 0)"));
        }
        px_srcline(745);
        return px_str("true");
    }
    px_srcline(746);
    if (px_is_truthy(px_eq(_v994, px_str("PatWildcard")))) {
        px_srcline(747);
        return px_str("true");
    }
    px_srcline(748);
    if (px_is_truthy(px_eq(_v994, px_str("PatTuple")))) {
        px_srcline(749);
        _v997 = px_index(_v992, px_int(1LL));
        px_srcline(750);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v997}, 1), px_int(0LL)))) {
            px_srcline(751);
            return px_call(px_get_global("cg_gen_pattern_cond"), (LXValue[]){px_index(_v997, px_int(0LL)), _v993}, 2);
        }
        px_srcline(752);
        return px_str("true");
    }
    px_srcline(753);
    if (px_is_truthy(px_eq(_v994, px_str("PatConstructor")))) {
        px_srcline(754);
        _v996 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v992, px_int(1LL))}, 1);
        px_srcline(755);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("("), _v993), px_str(".type == PX_ENUM && strcmp(")), _v993), px_str(".as.obj->as.enum_inst.variant, \"")), _v996), px_str("\") == 0)"));
    }
    px_srcline(756);
    return px_str("true");
px_err_998:
    if (px_err_998_proped) return px_err_998_val;
    return px_null();
}

static LXValue fn_cg_gen_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_lambda");
    LXValue _v1001 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1002 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v1015 = px_uninit();
    LXValue _v1016 = px_uninit();
    LXValue _v1017 = px_uninit();
    LXValue _v1018 = px_uninit();
    LXValue _v1019 = px_uninit();
    LXValue _v1020 = px_uninit();
    LXValue _v1021 = px_uninit();
    LXValue px_err_1022_val = px_null();
    int px_err_1022_proped = 0;
    px_srcline(759);
    px_set_global("cg_closure_id", px_add(px_get_global("cg_closure_id"), px_int(1LL)));
    px_srcline(760);
    _v1003 = px_get_global("cg_closure_id");
    px_srcline(761);
    _v1004 = px_add(px_str("fn_closure_"), px_call(px_get_global("str"), (LXValue[]){_v1003}, 1));
    px_srcline(763);
    _v1005 = px_list_n((LXValue[]){}, 0);
    px_srcline(764);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1002, _v1005}, 2));
    px_srcline(765);
    _v1006 = px_list_n((LXValue[]){}, 0);
    px_srcline(766);
    LXValue _t1023 = _v1005;
    int _il4 = px_iter_prepare(_t1023);
    for (int _t1024 = 0; _t1024 < _il4; _t1024++) {
        px_iter_ck(_t1023, _il4);
        _v1007 = px_iter_at(_t1023, px_int(_t1024));
        px_srcline(767);
        if (px_is_truthy(({ LXValue _t1025 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1001, _v1007}, 2)); px_is_truthy(_t1025) ? px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1007}, 1), px_null()) : _t1025; }))) {
            px_srcline(768);
            (void)(px_method(_v1006, "append", (LXValue[]){_v1007}, 1));
        }
    }
    px_srcline(769);
    _v1008 = px_str("px_null()");
    px_srcline(770);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1006}, 1), px_int(0LL)))) {
        px_srcline(771);
        _v1009 = px_str("({ LXValue _capenv = px_dict(); ");
        px_srcline(772);
        LXValue _t1026 = _v1006;
        int _il5 = px_iter_prepare(_t1026);
        for (int _t1027 = 0; _t1027 < _il5; _t1027++) {
            px_iter_ck(_t1026, _il5);
            _v1007 = px_iter_at(_t1026, px_int(_t1027));
            px_srcline(773);
            _v1010 = px_call(px_get_global("cg_var_of"), (LXValue[]){_v1007}, 1);
            px_srcline(777);
            if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1007}, 1))) {
                px_srcline(778);
                 _v1009 = px_add(_v1009, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v1007), px_str("\", px_cell(px_cell_get(")), _v1010), px_str("))); ")));
            }
            else {
                px_srcline(780);
                 _v1009 = px_add(_v1009, px_add(px_add(px_add(px_add(px_str("px_dict_set(_capenv, \""), _v1007), px_str("\", px_cell(")), _v1010), px_str(")); ")));
            }
        }
        px_srcline(781);
         _v1009 = px_add(_v1009, px_str("_capenv; })"));
        px_srcline(782);
         _v1008 = _v1009;
    }
    px_srcline(783);
    _v1011 = px_add(px_add(px_str("static LXValue "), _v1004), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(784);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1006}, 1), px_int(0LL)))) {
        px_srcline(785);
         _v1011 = px_add(_v1011, px_str("    (void)nargs;\n"));
    }
    else {
        px_srcline(787);
         _v1011 = px_add(_v1011, px_str("    (void)ctx;\n"));
    }
    px_srcline(788);
    _v1012 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(789);
    _v1013 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(790);
    _v1014 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(792);
    _v1015 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(793);
    _v1016 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(794);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(795);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(796);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(797);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(798);
    _v1017 = px_int(0LL);
    px_srcline(799);
    while (px_is_truthy(px_lt(_v1017, px_call(px_get_global("len"), (LXValue[]){_v1001}, 1)))) {
        px_srcline(800);
        _v1018 = px_call(px_get_global("cg_new_var"), (LXValue[]){px_index(_v1001, _v1017)}, 1);
        px_srcline(801);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){px_index(_v1001, _v1017)}, 1));
        px_srcline(802);
         _v1011 = px_add(_v1011, px_add(({ LXValue _s187 = px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1018), px_str(" = (nargs > ")), px_call(px_get_global("str"), (LXValue[]){_v1017}, 1)), px_str(") ? args[")); LXValue _s188 = px_call(px_get_global("str"), (LXValue[]){_v1017}, 1); px_add(_s187, _s188); }), px_str("] : px_null();\n")));
        px_srcline(803);
         _v1017 = px_add(_v1017, px_int(1LL));
    }
    px_srcline(804);
    LXValue _t1028 = _v1006;
    int _il6 = px_iter_prepare(_t1028);
    for (int _t1029 = 0; _t1029 < _il6; _t1029++) {
        px_iter_ck(_t1028, _il6);
        _v1007 = px_iter_at(_t1028, px_int(_t1029));
        px_srcline(805);
        _v1019 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1007}, 1);
        px_srcline(806);
        px_index_set(px_get_global("cg_cells"), _v1007, px_int(1LL));
        px_srcline(807);
        if (px_is_truthy(px_method(_v1016, "has", (LXValue[]){_v1007}, 1))) {
            px_srcline(808);
            (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v1007}, 1));
        }
        px_srcline(809);
         _v1011 = px_add(_v1011, px_add(px_add(px_add(px_add(px_str("    LXValue "), _v1019), px_str(" = px_env_lookup(ctx, \"")), _v1007), px_str("\");\n")));
    }
    px_srcline(814);
    _v1020 = px_call(px_get_global("cg_cerr_tag"), (LXValue[]){}, 0);
    px_srcline(815);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1020}, 1));
    px_srcline(816);
     _v1011 = px_add(_v1011, px_add(px_add(px_str("    LXValue "), _v1020), px_str("_val = px_null();\n")));
    px_srcline(817);
     _v1011 = px_add(_v1011, px_add(px_add(px_str("    int "), _v1020), px_str("_proped = 0;\n")));
    px_srcline(818);
    _v1021 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){_v1002}, 1);
    px_srcline(819);
     _v1011 = px_add(_v1011, px_add(px_add(px_str("    return "), _v1021), px_str(";\n")));
    px_srcline(820);
     _v1011 = px_add(_v1011, px_add(_v1020, px_str(":\n")));
    px_srcline(821);
     _v1011 = px_add(_v1011, px_add(px_add(px_add(px_add(px_str("    if ("), _v1020), px_str("_proped) return ")), _v1020), px_str("_val;\n")));
    px_srcline(822);
     _v1011 = px_add(_v1011, px_str("    return px_null();\n"));
    px_srcline(823);
     _v1011 = px_add(_v1011, px_str("}\n"));
    px_srcline(824);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(825);
    px_set_global("cg_closures", px_add(px_get_global("cg_closures"), _v1011));
    px_srcline(826);
    px_set_global("cg_vars", _v1012);
    px_srcline(827);
    px_set_global("cg_var_types", _v1013);
    px_srcline(828);
    px_set_global("cg_cells", _v1014);
    px_srcline(829);
    px_set_global("cg_inited", _v1015);
    px_srcline(831);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1006}, 1), px_int(0LL)))) {
        px_srcline(832);
        return px_add(px_add(px_add(px_add(px_add(px_add(px_str("px_func_env(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1003}, 1)), px_str(">\", ")), _v1004), px_str(", ")), _v1008), px_str(")"));
    }
    px_srcline(833);
    return px_add(px_add(px_add(px_add(px_str("px_func(\"<closure"), px_call(px_get_global("str"), (LXValue[]){_v1003}, 1)), px_str(">\", ")), _v1004), px_str(", NULL)"));
px_err_1022:
    if (px_err_1022_proped) return px_err_1022_val;
    return px_null();
}

static LXValue fn_cgm_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_perr");
    LXValue _v1030 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1031 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1032_val = px_null();
    int px_err_1032_proped = 0;
    px_srcline(20);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1030), px_str(": ")), _v1031)}, 1));
    px_srcline(21);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1032:
    if (px_err_1032_proped) return px_err_1032_val;
    return px_null();
}

static LXValue fn_cgm_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cgm_pwarn");
    LXValue _v1033 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1034_val = px_null();
    int px_err_1034_proped = 0;
    px_srcline(23);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1033)}, 1));
px_err_1034:
    if (px_err_1034_proped) return px_err_1034_val;
    return px_null();
}

static LXValue fn_cg_dirname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dirname");
    LXValue _v1035 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1036 = px_uninit();
    LXValue px_err_1037_val = px_null();
    int px_err_1037_proped = 0;
    px_srcline(26);
    _v1036 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v1035}, 1), px_int(1LL));
    px_srcline(27);
    while (px_is_truthy(px_ge(_v1036, px_int(0LL)))) {
        px_srcline(28);
        if (px_is_truthy(px_eq(px_index(_v1035, _v1036), px_str("/")))) {
            px_srcline(29);
            if (px_is_truthy(px_eq(_v1036, px_int(0LL)))) {
                px_srcline(30);
                return px_str("/");
            }
            px_srcline(31);
            return px_slice(_v1035, px_int(0LL), _v1036, px_null());
        }
        px_srcline(32);
         _v1036 = px_sub(_v1036, px_int(1LL));
    }
    px_srcline(33);
    return px_str(".");
px_err_1037:
    if (px_err_1037_proped) return px_err_1037_val;
    return px_null();
}

static LXValue fn_cg_norm_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_norm_path");
    LXValue _v1038 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1039 = px_uninit();
    LXValue _v1040 = px_uninit();
    LXValue _v1041 = px_uninit();
    LXValue _v1042 = px_uninit();
    LXValue _v1043 = px_uninit();
    LXValue _v1044 = px_uninit();
    LXValue px_err_1045_val = px_null();
    int px_err_1045_proped = 0;
    px_srcline(37);
    _v1039 = ({ LXValue _t1046 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1038}, 1), px_int(0LL)); px_is_truthy(_t1046) ? px_eq(px_index(_v1038, px_int(0LL)), px_str("/")) : _t1046; });
    px_srcline(38);
    _v1040 = px_list_n((LXValue[]){}, 0);
    px_srcline(39);
    _v1041 = px_str("");
    px_srcline(40);
    _v1042 = px_int(0LL);
    px_srcline(41);
    while (px_is_truthy(px_le(_v1042, px_call(px_get_global("len"), (LXValue[]){_v1038}, 1)))) {
        px_srcline(42);
        if (px_is_truthy(({ LXValue _t1047 = px_eq(_v1042, px_call(px_get_global("len"), (LXValue[]){_v1038}, 1)); px_is_truthy(_t1047) ? _t1047 : px_eq(px_index(_v1038, _v1042), px_str("/")); }))) {
            px_srcline(43);
            if (px_is_truthy(px_eq(_v1041, px_str("..")))) {
                px_srcline(44);
                if (px_is_truthy(({ LXValue _t1048 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1040}, 1), px_int(0LL)); px_is_truthy(_t1048) ? px_ne(px_index(_v1040, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1040}, 1), px_int(1LL))), px_str("..")) : _t1048; }))) {
                    px_srcline(45);
                    (void)(px_method(_v1040, "pop", (LXValue[]){}, 0));
                }
                else if (px_is_truthy(px_not(_v1039))) {
                    px_srcline(47);
                    (void)(px_method(_v1040, "append", (LXValue[]){px_str("..")}, 1));
                }
            }
            else if (px_is_truthy(({ LXValue _t1049 = px_ne(_v1041, px_str("")); px_is_truthy(_t1049) ? px_ne(_v1041, px_str(".")) : _t1049; }))) {
                px_srcline(49);
                (void)(px_method(_v1040, "append", (LXValue[]){_v1041}, 1));
            }
            px_srcline(50);
             _v1041 = px_str("");
        }
        else {
            px_srcline(52);
             _v1041 = px_add(_v1041, px_index(_v1038, _v1042));
        }
        px_srcline(53);
         _v1042 = px_add(_v1042, px_int(1LL));
    }
    px_srcline(54);
    _v1043 = px_str("");
    px_srcline(55);
    _v1044 = px_int(0LL);
    px_srcline(56);
    while (px_is_truthy(px_lt(_v1044, px_call(px_get_global("len"), (LXValue[]){_v1040}, 1)))) {
        px_srcline(57);
         _v1043 = px_add(_v1043, px_add(px_str("/"), px_index(_v1040, _v1044)));
        px_srcline(58);
         _v1044 = px_add(_v1044, px_int(1LL));
    }
    px_srcline(59);
    if (px_is_truthy(_v1039)) {
        px_srcline(60);
        if (px_is_truthy(px_eq(_v1043, px_str("")))) {
            px_srcline(61);
            return px_str("/");
        }
        px_srcline(62);
        return _v1043;
    }
    px_srcline(63);
    if (px_is_truthy(px_eq(_v1043, px_str("")))) {
        px_srcline(64);
        return px_str(".");
    }
    px_srcline(65);
    return px_slice(_v1043, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1043}, 1), px_null());
px_err_1045:
    if (px_err_1045_proped) return px_err_1045_val;
    return px_null();
}

static LXValue fn_cg_stdlib_dir(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_stdlib_dir");
    LXValue _v1050 = px_uninit();
    LXValue _v1051 = px_uninit();
    LXValue _v1052 = px_uninit();
    LXValue _v1053 = px_uninit();
    LXValue _v1054 = px_uninit();
    LXValue _v1055 = px_uninit();
    LXValue px_err_1056_val = px_null();
    int px_err_1056_proped = 0;
    px_srcline(75);
    _v1050 = px_call(px_get_global("env"), (LXValue[]){px_str("PX_STDLIB")}, 1);
    px_srcline(76);
    if (px_is_truthy(({ LXValue _t1057 = px_ne(_v1050, px_null()); px_is_truthy(_t1057) ? px_call(px_get_global("exists"), (LXValue[]){_v1050}, 1) : _t1057; }))) {
        px_srcline(77);
        return _v1050;
    }
    px_srcline(78);
    _v1051 = px_call(px_get_global("os_self_path"), (LXValue[]){}, 0);
    px_srcline(79);
    if (px_is_truthy(({ LXValue _t1058 = px_ne(_v1051, px_null()); px_is_truthy(_t1058) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1051}, 1), px_int(0LL)) : _t1058; }))) {
        px_srcline(80);
        _v1052 = px_call(px_get_global("cg_norm_path"), (LXValue[]){px_add(px_call(px_get_global("cg_dirname"), (LXValue[]){_v1051}, 1), px_str("/../stdlib"))}, 1);
        px_srcline(81);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1052}, 1))) {
            px_srcline(82);
            return _v1052;
        }
    }
    px_srcline(83);
    _v1053 = px_list_n((LXValue[]){px_str("../stdlib"), px_str("stdlib"), px_str("./stdlib"), px_str("../../stdlib")}, 4);
    px_srcline(84);
    _v1054 = px_int(0LL);
    px_srcline(85);
    while (px_is_truthy(px_lt(_v1054, px_call(px_get_global("len"), (LXValue[]){_v1053}, 1)))) {
        px_srcline(86);
        _v1055 = px_index(_v1053, _v1054);
        px_srcline(87);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1055}, 1))) {
            px_srcline(88);
            return _v1055;
        }
        px_srcline(89);
         _v1054 = px_add(_v1054, px_int(1LL));
    }
    px_srcline(90);
    return px_null();
px_err_1056:
    if (px_err_1056_proped) return px_err_1056_val;
    return px_null();
}

static LXValue fn_cg_find_module_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find_module_path");
    LXValue _v1059 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1060 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1061 = px_uninit();
    LXValue _v1062 = px_uninit();
    LXValue _v1063 = px_uninit();
    LXValue _v1064 = px_uninit();
    LXValue _v1065 = px_uninit();
    LXValue _v1066 = px_uninit();
    LXValue _v1067 = px_uninit();
    LXValue _v1068 = px_uninit();
    LXValue _v1069 = px_uninit();
    LXValue _v1070 = px_uninit();
    LXValue _v1071 = px_uninit();
    LXValue _v1072 = px_uninit();
    LXValue _v1073 = px_uninit();
    LXValue _v1074 = px_uninit();
    LXValue _v1075 = px_uninit();
    LXValue px_err_1076_val = px_null();
    int px_err_1076_proped = 0;
    px_srcline(93);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1059}, 1), px_int(0LL)))) {
        px_srcline(94);
        return px_null();
    }
    px_srcline(96);
    if (px_is_truthy(({ LXValue _t1078 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1059}, 1), px_int(1LL)); px_is_truthy(_t1078) ? ({ LXValue _t1077 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v1059, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t1077) ? _t1077 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v1059, px_int(0LL)), px_str(".px")}, 2); }) : _t1078; }))) {
        px_srcline(97);
        _v1061 = px_index(_v1059, px_int(0LL));
        px_srcline(98);
        _v1062 = _v1061;
        px_srcline(99);
        if (px_is_truthy(px_not(({ LXValue _t1079 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1061}, 1), px_int(0LL)); px_is_truthy(_t1079) ? px_eq(px_index(_v1061, px_int(0LL)), px_str("/")) : _t1079; })))) {
            px_srcline(100);
             _v1062 = px_add(px_add(_v1060, px_str("/")), _v1061);
        }
        px_srcline(101);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1062}, 1))) {
            px_srcline(102);
            return _v1062;
        }
        px_srcline(103);
        return px_null();
    }
    px_srcline(105);
    if (px_is_truthy(px_eq(px_index(_v1059, px_int(0LL)), px_str("std")))) {
        px_srcline(106);
        if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v1059}, 1), px_int(2LL)))) {
            px_srcline(107);
            return px_null();
        }
        px_srcline(108);
        _v1063 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(109);
        if (px_is_truthy(px_eq(_v1063, px_null()))) {
            px_srcline(110);
            return px_null();
        }
        px_srcline(111);
        _v1062 = _v1063;
        px_srcline(112);
        _v1064 = px_int(1LL);
        px_srcline(113);
        while (px_is_truthy(px_lt(_v1064, px_call(px_get_global("len"), (LXValue[]){_v1059}, 1)))) {
            px_srcline(114);
             _v1062 = px_add(_v1062, px_add(px_str("/"), px_index(_v1059, _v1064)));
            px_srcline(115);
             _v1064 = px_add(_v1064, px_int(1LL));
        }
        px_srcline(116);
        _v1065 = px_add(_v1062, px_str(".px"));
        px_srcline(117);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1065}, 1))) {
            px_srcline(118);
            return _v1065;
        }
        px_srcline(119);
        _v1066 = px_add(_v1062, px_str("/mod.px"));
        px_srcline(120);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1066}, 1))) {
            px_srcline(121);
            return _v1066;
        }
        px_srcline(122);
        return px_null();
    }
    px_srcline(124);
    _v1067 = px_list_n((LXValue[]){_v1060}, 1);
    px_srcline(125);
    _v1068 = px_add(_v1060, px_str("/.px_modules"));
    px_srcline(126);
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1068}, 1))) {
        px_srcline(127);
        (void)(px_method(_v1067, "append", (LXValue[]){_v1068}, 1));
        px_srcline(128);
        _v1069 = px_call(px_get_global("list_dir"), (LXValue[]){_v1068}, 1);
        px_srcline(129);
        _v1070 = px_int(0LL);
        px_srcline(130);
        while (px_is_truthy(px_lt(_v1070, px_call(px_get_global("len"), (LXValue[]){_v1069}, 1)))) {
            px_srcline(131);
            _v1071 = px_index(_v1069, _v1070);
            px_srcline(132);
            _v1072 = px_add(px_add(_v1068, px_str("/")), _v1071);
            px_srcline(133);
            if (px_is_truthy(({ LXValue _t1080 = px_call(px_get_global("exists"), (LXValue[]){_v1072}, 1); px_is_truthy(_t1080) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1071, px_str(".")}, 2)) : _t1080; }))) {
                px_srcline(134);
                (void)(px_method(_v1067, "append", (LXValue[]){_v1072}, 1));
            }
            px_srcline(135);
             _v1070 = px_add(_v1070, px_int(1LL));
        }
    }
    px_srcline(136);
    _v1073 = px_int(0LL);
    px_srcline(137);
    while (px_is_truthy(px_lt(_v1073, px_call(px_get_global("len"), (LXValue[]){_v1067}, 1)))) {
        px_srcline(138);
        _v1074 = px_index(_v1067, _v1073);
        px_srcline(139);
        _v1062 = _v1074;
        px_srcline(140);
        _v1064 = px_int(0LL);
        px_srcline(141);
        while (px_is_truthy(px_lt(_v1064, px_call(px_get_global("len"), (LXValue[]){_v1059}, 1)))) {
            px_srcline(142);
             _v1062 = px_add(_v1062, px_add(px_str("/"), px_index(_v1059, _v1064)));
            px_srcline(143);
             _v1064 = px_add(_v1064, px_int(1LL));
        }
        px_srcline(144);
        _v1065 = px_add(_v1062, px_str(".px"));
        px_srcline(145);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1065}, 1))) {
            px_srcline(146);
            return _v1065;
        }
        px_srcline(147);
        _v1066 = px_add(_v1062, px_str("/mod.px"));
        px_srcline(148);
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1066}, 1))) {
            px_srcline(149);
            return _v1066;
        }
        px_srcline(150);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1059}, 1), px_int(1LL)))) {
            px_srcline(151);
            _v1075 = px_add(px_add(px_add(_v1074, px_str("/")), px_index(_v1059, px_int(0LL))), px_str(".px"));
            px_srcline(152);
            if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v1075}, 1))) {
                px_srcline(153);
                return _v1075;
            }
        }
        px_srcline(154);
         _v1073 = px_add(_v1073, px_int(1LL));
    }
    px_srcline(155);
    return px_null();
px_err_1076:
    if (px_err_1076_proped) return px_err_1076_val;
    return px_null();
}

static LXValue fn_cg_is_definition(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_definition");
    LXValue _v1081 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1082 = px_uninit();
    LXValue px_err_1083_val = px_null();
    int px_err_1083_proped = 0;
    px_srcline(158);
    _v1082 = px_index(_v1081, px_int(0LL));
    px_srcline(159);
    if (px_is_truthy(px_eq(_v1082, px_str("FuncDef")))) {
        px_srcline(161);
        if (px_is_truthy(px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1081, px_int(1LL))}, 1), px_str("main")))) {
            px_srcline(162);
            return px_bool(false);
        }
        px_srcline(163);
        return px_bool(true);
    }
    px_srcline(164);
    if (px_is_truthy(px_eq(_v1082, px_str("ExternDef")))) {
        px_srcline(165);
        return px_bool(true);
    }
    px_srcline(166);
    if (px_is_truthy(({ LXValue _t1086 = ({ LXValue _t1085 = ({ LXValue _t1084 = px_eq(_v1082, px_str("StructDef")); px_is_truthy(_t1084) ? _t1084 : px_eq(_v1082, px_str("EnumDef")); }); px_is_truthy(_t1085) ? _t1085 : px_eq(_v1082, px_str("TraitDef")); }); px_is_truthy(_t1086) ? _t1086 : px_eq(_v1082, px_str("ImplDef")); }))) {
        px_srcline(167);
        return px_bool(true);
    }
    px_srcline(168);
    if (px_is_truthy(px_eq(_v1082, px_str("VarDecl")))) {
        px_srcline(172);
        return px_bool(true);
    }
    px_srcline(173);
    return px_bool(false);
px_err_1083:
    if (px_err_1083_proped) return px_err_1083_val;
    return px_null();
}

static LXValue fn_cg_def_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_def_name");
    LXValue _v1087 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1088 = px_uninit();
    LXValue _v1089 = px_uninit();
    LXValue _v1090 = px_uninit();
    LXValue _v1091 = px_uninit();
    LXValue px_err_1092_val = px_null();
    int px_err_1092_proped = 0;
    px_srcline(176);
    _v1088 = px_index(_v1087, px_int(0LL));
    px_srcline(177);
    if (px_is_truthy(({ LXValue _t1096 = ({ LXValue _t1095 = ({ LXValue _t1094 = ({ LXValue _t1093 = px_eq(_v1088, px_str("FuncDef")); px_is_truthy(_t1093) ? _t1093 : px_eq(_v1088, px_str("StructDef")); }); px_is_truthy(_t1094) ? _t1094 : px_eq(_v1088, px_str("EnumDef")); }); px_is_truthy(_t1095) ? _t1095 : px_eq(_v1088, px_str("TraitDef")); }); px_is_truthy(_t1096) ? _t1096 : px_eq(_v1088, px_str("ExternDef")); }))) {
        px_srcline(178);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1087, px_int(1LL))}, 1);
    }
    px_srcline(179);
    if (px_is_truthy(px_eq(_v1088, px_str("VarDecl")))) {
        px_srcline(180);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1087, px_int(2LL))}, 1);
    }
    px_srcline(181);
    if (px_is_truthy(px_eq(_v1088, px_str("ImplDef")))) {
        px_srcline(182);
        _v1089 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1087, px_int(1LL))}, 1);
        px_srcline(183);
        _v1090 = px_index(_v1087, px_int(2LL));
        px_srcline(184);
        _v1091 = px_str("None");
        px_srcline(185);
        if (px_is_truthy(px_ne(_v1090, px_null()))) {
            px_srcline(186);
             _v1091 = px_add(px_add(px_str("Some("), _v1090), px_str(")"));
        }
        px_srcline(187);
        return px_add(px_add(px_add(px_str("impl::"), _v1089), px_str("::")), _v1091);
    }
    px_srcline(188);
    return px_null();
px_err_1092:
    if (px_err_1092_proped) return px_err_1092_val;
    return px_null();
}

static LXValue fn_cg_load_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_module");
    LXValue _v1097 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1098 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1099 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1100 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1101 = (nargs > 4) ? args[4] : px_null();
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
    LXValue _v1115 = px_uninit();
    LXValue _v1116 = px_uninit();
    LXValue _v1117 = px_uninit();
    LXValue _v1118 = px_uninit();
    LXValue _v1119 = px_uninit();
    LXValue _v1120 = px_uninit();
    LXValue _v1121 = px_uninit();
    LXValue px_err_1122_val = px_null();
    int px_err_1122_proped = 0;
    px_srcline(192);
    _v1102 = px_list_n((LXValue[]){}, 0);
    px_srcline(193);
    _v1103 = px_int(0LL);
    px_srcline(194);
    while (px_is_truthy(px_lt(_v1103, px_call(px_get_global("len"), (LXValue[]){_v1097}, 1)))) {
        px_srcline(195);
        (void)(px_method(_v1102, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1097, _v1103)}, 1)}, 1));
        px_srcline(196);
         _v1103 = px_add(_v1103, px_int(1LL));
    }
    px_srcline(197);
     _v1097 = _v1102;
    px_srcline(198);
    _v1104 = px_list_n((LXValue[]){}, 0);
    px_srcline(199);
    _v1105 = px_int(0LL);
    px_srcline(200);
    while (px_is_truthy(px_lt(_v1105, px_call(px_get_global("len"), (LXValue[]){_v1098}, 1)))) {
        px_srcline(201);
        (void)(px_method(_v1104, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1098, _v1105)}, 1)}, 1));
        px_srcline(202);
         _v1105 = px_add(_v1105, px_int(1LL));
    }
    px_srcline(203);
     _v1098 = _v1104;
    px_srcline(204);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1097}, 1), px_int(0LL)))) {
        px_srcline(205);
        return px_null();
    }
    px_srcline(207);
    if (px_is_truthy(({ LXValue _t1125 = ({ LXValue _t1124 = ({ LXValue _t1123 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1097}, 1), px_int(1LL)); px_is_truthy(_t1123) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v1097, px_int(0LL))}, 1), px_int(2LL)) : _t1123; }); px_is_truthy(_t1124) ? px_eq(px_slice(px_index(_v1097, px_int(0LL)), px_int(0LL), px_int(2LL), px_null()), px_str("c/")) : _t1124; }); px_is_truthy(_t1125) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_index(_v1097, px_int(0LL)), px_str(".px")}, 2)) : _t1125; }))) {
        px_srcline(208);
        return px_null();
    }
    px_srcline(209);
    _v1106 = px_eq(px_index(_v1097, px_int(0LL)), px_str("std"));
    px_srcline(210);
    _v1107 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v1097}, 2);
    px_srcline(211);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1107}, 1))) {
        px_srcline(212);
        return px_null();
    }
    px_srcline(213);
    _v1108 = px_call(px_get_global("cg_find_module_path"), (LXValue[]){_v1097, _v1099}, 2);
    px_srcline(214);
    if (px_is_truthy(px_eq(_v1108, px_null()))) {
        px_srcline(221);
        _v1109 = px_call(px_get_global("cg_stdlib_dir"), (LXValue[]){}, 0);
        px_srcline(222);
        _v1110 = px_str("");
        px_srcline(223);
        if (px_is_truthy(({ LXValue _t1126 = px_eq(px_index(_v1097, px_int(0LL)), px_str("std")); px_is_truthy(_t1126) ? px_eq(_v1109, px_null()) : _t1126; }))) {
            px_srcline(224);
             _v1110 = px_str("；stdlib 未找到（设 PX_STDLIB 或放置 stdlib/）");
        }
        px_srcline(225);
        if (px_is_truthy(px_ne(px_call(px_get_global("env"), (LXValue[]){px_str("PX_STRICT_MODULE")}, 1), px_null()))) {
            px_srcline(226);
            (void)(px_call(px_get_global("cgm_perr"), (LXValue[]){px_str("E3005"), px_add(px_add(px_add(px_str("找不到模块 '"), _v1107), px_str("'")), _v1110)}, 2));
        }
        px_srcline(227);
        (void)(px_call(px_get_global("cgm_pwarn"), (LXValue[]){px_add(px_add(px_add(px_str("找不到模块 '"), _v1107), px_str("'（已跳过 → 运行期将报未定义）")), _v1110)}, 1));
        px_srcline(228);
        return px_null();
    }
    px_srcline(232);
    _v1111 = px_add(px_str("#"), px_call(px_get_global("cg_norm_path"), (LXValue[]){_v1108}, 1));
    px_srcline(233);
    if (px_is_truthy(px_method(px_get_global("loaded"), "has", (LXValue[]){_v1111}, 1))) {
        px_srcline(234);
        return px_null();
    }
    px_srcline(235);
    px_index_set(px_get_global("loaded"), _v1107, _v1108);
    px_srcline(236);
    px_index_set(px_get_global("loaded"), _v1111, _v1108);
    px_srcline(237);
    _v1112 = px_call(px_get_global("read_file"), (LXValue[]){_v1108}, 1);
    px_srcline(238);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){_v1112}, 1));
    px_srcline(239);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(240);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(241);
    _v1113 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(243);
    _v1114 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v1108}, 1);
    px_srcline(244);
    _v1115 = px_list_n((LXValue[]){}, 0);
    px_srcline(245);
    _v1116 = px_int(0LL);
    px_srcline(246);
    while (px_is_truthy(px_lt(_v1116, px_call(px_get_global("len"), (LXValue[]){px_index(_v1113, px_int(1LL))}, 1)))) {
        px_srcline(247);
        _v1117 = px_index(px_index(_v1113, px_int(1LL)), _v1116);
        px_srcline(248);
        if (px_is_truthy(px_eq(px_index(_v1117, px_int(0LL)), px_str("Import")))) {
            px_srcline(249);
            (void)(px_method(_v1115, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1117, px_int(1LL)), px_index(_v1117, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(250);
         _v1116 = px_add(_v1116, px_int(1LL));
    }
    px_srcline(251);
    _v1118 = px_int(0LL);
    px_srcline(252);
    while (px_is_truthy(px_lt(_v1118, px_call(px_get_global("len"), (LXValue[]){_v1115}, 1)))) {
        px_srcline(253);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1115, _v1118), px_int(0LL)), px_index(px_index(_v1115, _v1118), px_int(1LL)), _v1114, _v1100, _v1101}, 5));
        px_srcline(254);
         _v1118 = px_add(_v1118, px_int(1LL));
    }
    px_srcline(256);
    _v1119 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1098}, 1), px_int(0LL));
    px_srcline(257);
    _v1120 = px_int(0LL);
    px_srcline(258);
    while (px_is_truthy(px_lt(_v1120, px_call(px_get_global("len"), (LXValue[]){px_index(_v1113, px_int(1LL))}, 1)))) {
        px_srcline(259);
        _v1117 = px_index(px_index(_v1113, px_int(1LL)), _v1120);
        px_srcline(260);
        if (px_is_truthy(px_eq(px_index(_v1117, px_int(0LL)), px_str("Import")))) {
            px_srcline(261);
             _v1120 = px_add(_v1120, px_int(1LL));
            px_srcline(262);
            continue;
        }
        px_srcline(263);
        if (px_is_truthy(px_not(px_call(px_get_global("cg_is_definition"), (LXValue[]){_v1117}, 1)))) {
            px_srcline(264);
             _v1120 = px_add(_v1120, px_int(1LL));
            px_srcline(265);
            continue;
        }
        px_srcline(266);
        _v1121 = px_call(px_get_global("cg_def_name"), (LXValue[]){_v1117}, 1);
        px_srcline(267);
        if (px_is_truthy(px_eq(_v1121, px_null()))) {
            px_srcline(268);
            (void)(px_method(_v1100, "append", (LXValue[]){_v1117}, 1));
        }
        else {
            px_srcline(270);
            if (px_is_truthy(_v1119)) {
                px_srcline(271);
                if (px_is_truthy(({ LXValue _t1127 = px_ge(px_call(px_get_global("len"), (LXValue[]){_v1121}, 1), px_int(5LL)); px_is_truthy(_t1127) ? px_eq(px_slice(_v1121, px_int(0LL), px_int(5LL), px_null()), px_str("impl::")) : _t1127; }))) {
                    px_srcline(272);
                     _v1120 = px_add(_v1120, px_int(1LL));
                    px_srcline(273);
                    continue;
                }
                px_srcline(274);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1098, _v1121}, 2)))) {
                    px_srcline(275);
                     _v1120 = px_add(_v1120, px_int(1LL));
                    px_srcline(276);
                    continue;
                }
            }
            px_srcline(277);
            if (px_is_truthy(_v1106)) {
                px_srcline(278);
                if (px_is_truthy(px_method(_v1101, "has", (LXValue[]){_v1121}, 1))) {
                    px_srcline(279);
                     _v1120 = px_add(_v1120, px_int(1LL));
                    px_srcline(280);
                    continue;
                }
                px_srcline(281);
                px_index_set(_v1101, _v1121, px_bool(true));
            }
            px_srcline(282);
            (void)(px_method(_v1100, "append", (LXValue[]){_v1117}, 1));
        }
        px_srcline(283);
         _v1120 = px_add(_v1120, px_int(1LL));
    }
px_err_1122:
    if (px_err_1122_proped) return px_err_1122_val;
    return px_null();
}

static LXValue fn_cg_resolve_modules(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_resolve_modules");
    LXValue _v1128 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1129 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1130 = px_uninit();
    LXValue _v1131 = px_uninit();
    LXValue _v1132 = px_uninit();
    LXValue _v1133 = px_uninit();
    LXValue _v1134 = px_uninit();
    LXValue _v1135 = px_uninit();
    LXValue _v1136 = px_uninit();
    LXValue _v1137 = px_uninit();
    LXValue _v1138 = px_uninit();
    LXValue _v1139 = px_uninit();
    LXValue px_err_1140_val = px_null();
    int px_err_1140_proped = 0;
    px_srcline(286);
    _v1130 = px_index(_v1128, px_int(1LL));
    px_srcline(287);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1130}, 1), px_int(0LL)))) {
        px_srcline(288);
        return _v1128;
    }
    px_srcline(289);
    _v1131 = px_list_n((LXValue[]){}, 0);
    px_srcline(290);
    _v1132 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(291);
    px_set_global("loaded", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(292);
    _v1133 = px_list_n((LXValue[]){}, 0);
    px_srcline(293);
    _v1134 = px_int(0LL);
    px_srcline(294);
    while (px_is_truthy(px_lt(_v1134, px_call(px_get_global("len"), (LXValue[]){_v1130}, 1)))) {
        px_srcline(295);
        _v1135 = px_index(_v1130, _v1134);
        px_srcline(296);
        if (px_is_truthy(px_eq(px_index(_v1135, px_int(0LL)), px_str("Import")))) {
            px_srcline(297);
            (void)(px_method(_v1133, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v1135, px_int(1LL)), px_index(_v1135, px_int(2LL))}, 2)}, 1));
        }
        px_srcline(298);
         _v1134 = px_add(_v1134, px_int(1LL));
    }
    px_srcline(299);
    _v1136 = px_int(0LL);
    px_srcline(300);
    while (px_is_truthy(px_lt(_v1136, px_call(px_get_global("len"), (LXValue[]){_v1133}, 1)))) {
        px_srcline(301);
        (void)(px_call(px_get_global("cg_load_module"), (LXValue[]){px_index(px_index(_v1133, _v1136), px_int(0LL)), px_index(px_index(_v1133, _v1136), px_int(1LL)), _v1129, _v1131, _v1132}, 5));
        px_srcline(302);
         _v1136 = px_add(_v1136, px_int(1LL));
    }
    px_srcline(303);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1131}, 1), px_int(0LL)))) {
        px_srcline(304);
        return _v1128;
    }
    px_srcline(305);
    _v1137 = px_list_n((LXValue[]){}, 0);
    px_srcline(306);
    _v1138 = px_int(0LL);
    px_srcline(307);
    while (px_is_truthy(px_lt(_v1138, px_call(px_get_global("len"), (LXValue[]){_v1131}, 1)))) {
        px_srcline(308);
        (void)(px_method(_v1137, "append", (LXValue[]){px_index(_v1131, _v1138)}, 1));
        px_srcline(309);
         _v1138 = px_add(_v1138, px_int(1LL));
    }
    px_srcline(310);
    _v1139 = px_int(0LL);
    px_srcline(311);
    while (px_is_truthy(px_lt(_v1139, px_call(px_get_global("len"), (LXValue[]){_v1130}, 1)))) {
        px_srcline(312);
        (void)(px_method(_v1137, "append", (LXValue[]){px_index(_v1130, _v1139)}, 1));
        px_srcline(313);
         _v1139 = px_add(_v1139, px_int(1LL));
    }
    px_srcline(314);
    return px_list_n((LXValue[]){px_str("Program"), _v1137}, 2);
px_err_1140:
    if (px_err_1140_proped) return px_err_1140_val;
    return px_null();
}

static LXValue fn_cg_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_dict");
    LXValue _v1141 = px_uninit();
    LXValue px_err_1142_val = px_null();
    int px_err_1142_proped = 0;
    px_srcline(87);
    _v1141 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(88);
    (void)(px_method(_v1141, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(89);
    return _v1141;
px_err_1142:
    if (px_err_1142_proped) return px_err_1142_val;
    return px_null();
}

static LXValue fn_cg_dict_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_dict_copy");
    LXValue _v1143 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1144 = px_uninit();
    LXValue _v1145 = px_uninit();
    LXValue _v1146 = px_uninit();
    LXValue px_err_1147_val = px_null();
    int px_err_1147_proped = 0;
    px_srcline(91);
    _v1144 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
    px_srcline(92);
    _v1145 = px_method(_v1143, "keys", (LXValue[]){}, 0);
    px_srcline(93);
    _v1146 = px_int(0LL);
    px_srcline(94);
    while (px_is_truthy(px_lt(_v1146, px_call(px_get_global("len"), (LXValue[]){_v1145}, 1)))) {
        px_srcline(95);
        px_index_set(_v1144, px_index(_v1145, _v1146), px_index(_v1143, px_index(_v1145, _v1146)));
        px_srcline(96);
         _v1146 = px_add(_v1146, px_int(1LL));
    }
    px_srcline(97);
    return _v1144;
px_err_1147:
    if (px_err_1147_proped) return px_err_1147_val;
    return px_null();
}

static LXValue fn_cg_uid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_uid");
    LXValue px_err_1148_val = px_null();
    int px_err_1148_proped = 0;
    px_srcline(99);
    px_set_global("cg_uidc", px_add(px_get_global("cg_uidc"), px_int(1LL)));
    px_srcline(100);
    return px_get_global("cg_uidc");
px_err_1148:
    if (px_err_1148_proped) return px_err_1148_val;
    return px_null();
}

static LXValue fn_cg_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_tmp");
    LXValue px_err_1149_val = px_null();
    int px_err_1149_proped = 0;
    px_srcline(102);
    return px_add(px_str("_t"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
px_err_1149:
    if (px_err_1149_proped) return px_err_1149_val;
    return px_null();
}

static LXValue fn_cg_iter_len_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_iter_len_tmp");
    LXValue px_err_1150_val = px_null();
    int px_err_1150_proped = 0;
    px_srcline(107);
    px_set_global("cg_iter_uid", px_add(px_get_global("cg_iter_uid"), px_int(1LL)));
    px_srcline(108);
    return px_add(px_str("_il"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_iter_uid")}, 1));
px_err_1150:
    if (px_err_1150_proped) return px_err_1150_val;
    return px_null();
}

static LXValue fn_cg_unpack_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_unpack_tmp");
    LXValue px_err_1151_val = px_null();
    int px_err_1151_proped = 0;
    px_srcline(112);
    px_set_global("cg_unpack_uid", px_add(px_get_global("cg_unpack_uid"), px_int(1LL)));
    px_srcline(113);
    return px_add(px_str("_up"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_unpack_uid")}, 1));
px_err_1151:
    if (px_err_1151_proped) return px_err_1151_val;
    return px_null();
}

static LXValue fn_cg_cerr_tag(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_cerr_tag");
    LXValue px_err_1152_val = px_null();
    int px_err_1152_proped = 0;
    px_srcline(122);
    px_set_global("cg_cerr_uid", px_add(px_get_global("cg_cerr_uid"), px_int(1LL)));
    px_srcline(123);
    return px_add(px_str("px_cerr_"), px_call(px_get_global("str"), (LXValue[]){px_get_global("cg_cerr_uid")}, 1));
px_err_1152:
    if (px_err_1152_proped) return px_err_1152_val;
    return px_null();
}

static LXValue fn_cg_for_names(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_for_names");
    LXValue _v1153 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1154 = px_uninit();
    LXValue _v1155 = px_uninit();
    LXValue px_err_1156_val = px_null();
    int px_err_1156_proped = 0;
    px_srcline(126);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1153}, 1), px_str("string")))) {
        px_srcline(127);
        return px_list_n((LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v1153}, 1)}, 1);
    }
    px_srcline(128);
    _v1154 = px_list_n((LXValue[]){}, 0);
    px_srcline(129);
    _v1155 = px_int(0LL);
    px_srcline(130);
    while (px_is_truthy(px_lt(_v1155, px_call(px_get_global("len"), (LXValue[]){_v1153}, 1)))) {
        px_srcline(131);
        (void)(px_method(_v1154, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1153, _v1155)}, 1)}, 1));
        px_srcline(132);
         _v1155 = px_add(_v1155, px_int(1LL));
    }
    px_srcline(133);
    return _v1154;
px_err_1156:
    if (px_err_1156_proped) return px_err_1156_val;
    return px_null();
}

static LXValue fn_cg_new_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_new_var");
    LXValue _v1157 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1158 = px_uninit();
    LXValue px_err_1159_val = px_null();
    int px_err_1159_proped = 0;
    px_srcline(135);
    _v1158 = px_add(px_str("_v"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(136);
    px_index_set(px_get_global("cg_vars"), _v1157, _v1158);
    px_srcline(139);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1157}, 1))) {
        px_srcline(140);
        (void)(px_method(px_get_global("cg_cells"), "remove", (LXValue[]){_v1157}, 1));
    }
    px_srcline(141);
    return _v1158;
px_err_1159:
    if (px_err_1159_proped) return px_err_1159_val;
    return px_null();
}

static LXValue fn_cg_var_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_var_of");
    LXValue _v1160 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1161_val = px_null();
    int px_err_1161_proped = 0;
    px_srcline(143);
    if (px_is_truthy(px_method(px_get_global("cg_vars"), "has", (LXValue[]){_v1160}, 1))) {
        px_srcline(144);
        return px_index(px_get_global("cg_vars"), _v1160);
    }
    px_srcline(145);
    return px_null();
px_err_1161:
    if (px_err_1161_proped) return px_err_1161_val;
    return px_null();
}

static LXValue fn_cg_load_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_of");
    LXValue _v1162 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1163 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1164_val = px_null();
    int px_err_1164_proped = 0;
    px_srcline(150);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1162}, 1))) {
        px_srcline(151);
        return px_add(px_add(px_str("px_cell_get("), _v1163), px_str(")"));
    }
    px_srcline(152);
    return _v1163;
px_err_1164:
    if (px_err_1164_proped) return px_err_1164_val;
    return px_null();
}

static LXValue fn_cg_store_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_store_of");
    LXValue _v1165 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1166 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1167 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1168_val = px_null();
    int px_err_1168_proped = 0;
    px_srcline(154);
    if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){_v1165}, 1))) {
        px_srcline(155);
        return px_add(px_add(px_add(px_add(px_str("px_cell_set("), _v1166), px_str(", ")), _v1167), px_str(")"));
    }
    px_srcline(156);
    return px_add(px_add(_v1166, px_str(" = ")), _v1167);
px_err_1168:
    if (px_err_1168_proped) return px_err_1168_val;
    return px_null();
}

static LXValue fn_cg_inited_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_new");
    LXValue px_err_1169_val = px_null();
    int px_err_1169_proped = 0;
    px_srcline(159);
    return px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
px_err_1169:
    if (px_err_1169_proped) return px_err_1169_val;
    return px_null();
}

static LXValue fn_cg_inited_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_add");
    LXValue _v1170 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1171_val = px_null();
    int px_err_1171_proped = 0;
    px_srcline(161);
    px_index_set(px_get_global("cg_inited"), _v1170, px_int(1LL));
px_err_1171:
    if (px_err_1171_proped) return px_err_1171_val;
    return px_null();
}

static LXValue fn_cg_inited_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_copy");
    LXValue px_err_1172_val = px_null();
    int px_err_1172_proped = 0;
    px_srcline(163);
    return px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_inited")}, 1);
px_err_1172:
    if (px_err_1172_proped) return px_err_1172_val;
    return px_null();
}

static LXValue fn_cg_inited_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_restore");
    LXValue _v1173 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1174_val = px_null();
    int px_err_1174_proped = 0;
    px_srcline(165);
    px_set_global("cg_inited", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1173}, 1));
px_err_1174:
    if (px_err_1174_proped) return px_err_1174_val;
    return px_null();
}

static LXValue fn_cg_inited_intersect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_inited_intersect");
    LXValue _v1175 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1176 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1177 = px_uninit();
    LXValue _v1178 = px_uninit();
    LXValue _v1179 = px_uninit();
    LXValue px_err_1180_val = px_null();
    int px_err_1180_proped = 0;
    px_srcline(168);
    _v1177 = px_call(px_get_global("cg_inited_new"), (LXValue[]){}, 0);
    px_srcline(169);
    _v1178 = px_method(_v1175, "keys", (LXValue[]){}, 0);
    px_srcline(170);
    _v1179 = px_int(0LL);
    px_srcline(171);
    while (px_is_truthy(px_lt(_v1179, px_call(px_get_global("len"), (LXValue[]){_v1178}, 1)))) {
        px_srcline(172);
        if (px_is_truthy(px_method(_v1176, "has", (LXValue[]){px_index(_v1178, _v1179)}, 1))) {
            px_srcline(173);
            px_index_set(_v1177, px_index(_v1178, _v1179), px_int(1LL));
        }
        px_srcline(174);
         _v1179 = px_add(_v1179, px_int(1LL));
    }
    px_srcline(175);
    return _v1177;
px_err_1180:
    if (px_err_1180_proped) return px_err_1180_val;
    return px_null();
}

static LXValue fn_cg_load_ck(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_load_ck");
    LXValue _v1181 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1182 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1183_val = px_null();
    int px_err_1183_proped = 0;
    px_srcline(178);
    if (px_is_truthy(px_method(px_get_global("cg_inited"), "has", (LXValue[]){_v1181}, 1))) {
        px_srcline(179);
        return px_call(px_get_global("cg_load_of"), (LXValue[]){_v1181, _v1182}, 2);
    }
    px_srcline(180);
    return px_add(px_add(px_add(px_add(px_str("px_chk_uninit("), px_call(px_get_global("cg_load_of"), (LXValue[]){_v1181, _v1182}, 2)), px_str(", \"")), _v1181), px_str("\")"));
px_err_1183:
    if (px_err_1183_proped) return px_err_1183_val;
    return px_null();
}

static LXValue fn_cg_name_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_name_add");
    LXValue _v1184 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1185 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1186_val = px_null();
    int px_err_1186_proped = 0;
    px_srcline(182);
    if (px_is_truthy(({ LXValue _t1187 = px_ne(_v1185, px_str("")); px_is_truthy(_t1187) ? px_not(px_call(px_get_global("contains"), (LXValue[]){_v1184, _v1185}, 2)) : _t1187; }))) {
        px_srcline(183);
        (void)(px_method(_v1184, "append", (LXValue[]){_v1185}, 1));
    }
px_err_1186:
    if (px_err_1186_proped) return px_err_1186_val;
    return px_null();
}

static LXValue fn_cg_ast_used(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_used");
    LXValue _v1188 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1189 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1190 = px_uninit();
    LXValue px_err_1191_val = px_null();
    int px_err_1191_proped = 0;
    px_srcline(186);
    if (px_is_truthy(({ LXValue _t1192 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1188}, 1), px_str("list")); px_is_truthy(_t1192) ? _t1192 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1188}, 1), px_int(0LL)); }))) {
        px_srcline(187);
        return px_null();
    }
    px_srcline(188);
    if (px_is_truthy(({ LXValue _t1195 = ({ LXValue _t1194 = ({ LXValue _t1193 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1188, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1193) ? px_eq(px_index(_v1188, px_int(0LL)), px_str("Var")) : _t1193; }); px_is_truthy(_t1194) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v1188}, 1), px_int(2LL)) : _t1194; }); px_is_truthy(_t1195) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1188, px_int(1LL))}, 1), px_str("string")) : _t1195; }))) {
        px_srcline(189);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1189, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1188, px_int(1LL))}, 1)}, 2));
        px_srcline(190);
        return px_null();
    }
    px_srcline(191);
    LXValue _t1196 = _v1188;
    int _il7 = px_iter_prepare(_t1196);
    for (int _t1197 = 0; _t1197 < _il7; _t1197++) {
        px_iter_ck(_t1196, _il7);
        _v1190 = px_iter_at(_t1196, px_int(_t1197));
        px_srcline(192);
        (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1190, _v1189}, 2));
    }
px_err_1191:
    if (px_err_1191_proped) return px_err_1191_val;
    return px_null();
}

static LXValue fn_cg_ast_bound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ast_bound");
    LXValue _v1198 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1199 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1200 = px_uninit();
    LXValue _v1201 = px_uninit();
    LXValue _v1202 = px_uninit();
    LXValue _v1203 = px_uninit();
    LXValue _v1204 = px_uninit();
    LXValue px_err_1205_val = px_null();
    int px_err_1205_proped = 0;
    px_srcline(195);
    if (px_is_truthy(({ LXValue _t1206 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1198}, 1), px_str("list")); px_is_truthy(_t1206) ? _t1206 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1198}, 1), px_int(0LL)); }))) {
        px_srcline(196);
        return px_null();
    }
    px_srcline(197);
    _v1200 = px_index(_v1198, px_int(0LL));
    px_srcline(198);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1200}, 1), px_str("string")))) {
        px_srcline(199);
        if (px_is_truthy(px_eq(_v1200, px_str("VarDecl")))) {
            px_srcline(200);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1199, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1198, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v1200, px_str("For")))) {
            px_srcline(213);
            _v1201 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1198, px_int(1LL))}, 1);
            px_srcline(214);
            _v1202 = px_int(0LL);
            px_srcline(215);
            while (px_is_truthy(px_lt(_v1202, px_call(px_get_global("len"), (LXValue[]){_v1201}, 1)))) {
                px_srcline(216);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1199, px_index(_v1201, _v1202)}, 2));
                px_srcline(217);
                 _v1202 = px_add(_v1202, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1200, px_str("Closure")))) {
            px_srcline(219);
            LXValue _t1207 = px_index(_v1198, px_int(1LL));
            int _il8 = px_iter_prepare(_t1207);
            for (int _t1208 = 0; _t1208 < _il8; _t1208++) {
                px_iter_ck(_t1207, _il8);
                _v1203 = px_iter_at(_t1207, px_int(_t1208));
                px_srcline(220);
                (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1199, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1203, px_int(1LL))}, 1)}, 2));
            }
        }
    }
    px_srcline(221);
    LXValue _t1209 = _v1198;
    int _il9 = px_iter_prepare(_t1209);
    for (int _t1210 = 0; _t1210 < _il9; _t1210++) {
        px_iter_ck(_t1209, _il9);
        _v1204 = px_iter_at(_t1209, px_int(_t1210));
        px_srcline(222);
        (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){_v1204, _v1199}, 2));
    }
px_err_1205:
    if (px_err_1205_proped) return px_err_1205_val;
    return px_null();
}

static LXValue fn_cg_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_closure_caps");
    LXValue _v1211 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1212 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1213 = px_uninit();
    LXValue _v1214 = px_uninit();
    LXValue _v1215 = px_uninit();
    LXValue _v1216 = px_uninit();
    LXValue px_err_1217_val = px_null();
    int px_err_1217_proped = 0;
    px_srcline(225);
    _v1213 = px_list_n((LXValue[]){}, 0);
    px_srcline(226);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){px_index(_v1211, px_int(3LL)), _v1213}, 2));
    px_srcline(227);
    _v1214 = px_list_n((LXValue[]){}, 0);
    px_srcline(228);
    (void)(px_call(px_get_global("cg_ast_bound"), (LXValue[]){px_index(_v1211, px_int(3LL)), _v1214}, 2));
    px_srcline(229);
    LXValue _t1218 = px_index(_v1211, px_int(1LL));
    int _il10 = px_iter_prepare(_t1218);
    for (int _t1219 = 0; _t1219 < _il10; _t1219++) {
        px_iter_ck(_t1218, _il10);
        _v1215 = px_iter_at(_t1218, px_int(_t1219));
        px_srcline(230);
        (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1214, px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1215, px_int(1LL))}, 1)}, 2));
    }
    px_srcline(231);
    LXValue _t1220 = _v1213;
    int _il11 = px_iter_prepare(_t1220);
    for (int _t1221 = 0; _t1221 < _il11; _t1221++) {
        px_iter_ck(_t1220, _il11);
        _v1216 = px_iter_at(_t1220, px_int(_t1221));
        px_srcline(232);
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1214, _v1216}, 2)))) {
            px_srcline(233);
            (void)(px_call(px_get_global("cg_name_add"), (LXValue[]){_v1212, _v1216}, 2));
        }
    }
px_err_1217:
    if (px_err_1217_proped) return px_err_1217_val;
    return px_null();
}

static LXValue fn_cg_scan_closure_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_scan_closure_caps");
    LXValue _v1222 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1223 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1224 = px_uninit();
    LXValue px_err_1225_val = px_null();
    int px_err_1225_proped = 0;
    px_srcline(236);
    if (px_is_truthy(({ LXValue _t1226 = px_ne(px_call(px_get_global("type"), (LXValue[]){_v1222}, 1), px_str("list")); px_is_truthy(_t1226) ? _t1226 : px_eq(px_call(px_get_global("len"), (LXValue[]){_v1222}, 1), px_int(0LL)); }))) {
        px_srcline(237);
        return px_null();
    }
    px_srcline(238);
    if (px_is_truthy(({ LXValue _t1227 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1222, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1227) ? px_eq(px_index(_v1222, px_int(0LL)), px_str("Closure")) : _t1227; }))) {
        px_srcline(239);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){_v1222, _v1223}, 2));
    }
    px_srcline(240);
    if (px_is_truthy(({ LXValue _t1228 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v1222, px_int(0LL))}, 1), px_str("string")); px_is_truthy(_t1228) ? px_eq(px_index(_v1222, px_int(0LL)), px_str("FuncDef")) : _t1228; }))) {
        px_srcline(244);
        (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), px_index(_v1222, px_int(2LL)), px_index(_v1222, px_int(3LL)), px_index(_v1222, px_int(4LL)), px_list_n((LXValue[]){}, 0), px_index(_v1222, px_int(5LL))}, 6), _v1223}, 2));
    }
    px_srcline(245);
    LXValue _t1229 = _v1222;
    int _il12 = px_iter_prepare(_t1229);
    for (int _t1230 = 0; _t1230 < _il12; _t1230++) {
        px_iter_ck(_t1229, _il12);
        _v1224 = px_iter_at(_t1229, px_int(_t1230));
        px_srcline(246);
        (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1224, _v1223}, 2));
    }
px_err_1225:
    if (px_err_1225_proped) return px_err_1225_val;
    return px_null();
}

static LXValue fn_cg_mark_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_mark_immutable");
    LXValue _v1231 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1232_val = px_null();
    int px_err_1232_proped = 0;
    px_srcline(249);
    px_index_set(px_get_global("cg_immutables"), _v1231, px_int(1LL));
px_err_1232:
    if (px_err_1232_proped) return px_err_1232_val;
    return px_null();
}

static LXValue fn_cg_is_immutable(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_immutable");
    LXValue _v1233 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1234_val = px_null();
    int px_err_1234_proped = 0;
    px_srcline(251);
    return px_method(px_get_global("cg_immutables"), "has", (LXValue[]){_v1233}, 1);
px_err_1234:
    if (px_err_1234_proped) return px_err_1234_val;
    return px_null();
}

static LXValue fn_cg_is_wildcard(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_wildcard");
    LXValue _v1235 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1236_val = px_null();
    int px_err_1236_proped = 0;
    px_srcline(259);
    return px_eq(_v1235, px_str("_"));
px_err_1236:
    if (px_err_1236_proped) return px_err_1236_val;
    return px_null();
}

static LXValue fn_cg_perr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_perr");
    LXValue _v1237 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1238 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1239_val = px_null();
    int px_err_1239_proped = 0;
    px_srcline(264);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_add(px_add(px_str("编译错误 "), _v1237), px_str(": ")), _v1238)}, 1));
    px_srcline(265);
    (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
px_err_1239:
    if (px_err_1239_proped) return px_err_1239_val;
    return px_null();
}

static LXValue fn_cg_pwarn(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pwarn");
    LXValue _v1240 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1241_val = px_null();
    int px_err_1241_proped = 0;
    px_srcline(268);
    (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(px_str("[警告] "), _v1240)}, 1));
px_err_1241:
    if (px_err_1241_proped) return px_err_1241_val;
    return px_null();
}

static LXValue fn_cg_is_nonnull_ty(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_nonnull_ty");
    LXValue _v1242 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1243_val = px_null();
    int px_err_1243_proped = 0;
    px_srcline(272);
    if (px_is_truthy(px_eq(_v1242, px_null()))) {
        px_srcline(273);
        return px_bool(false);
    }
    px_srcline(274);
    if (px_is_truthy(px_eq(px_index(_v1242, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(275);
        return px_bool(false);
    }
    px_srcline(276);
    return px_bool(true);
px_err_1243:
    if (px_err_1243_proped) return px_err_1243_val;
    return px_null();
}

static LXValue fn_cg_is_null_lit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_is_null_lit");
    LXValue _v1244 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1245_val = px_null();
    int px_err_1245_proped = 0;
    px_srcline(279);
    if (px_is_truthy(px_eq(_v1244, px_null()))) {
        px_srcline(280);
        return px_bool(false);
    }
    px_srcline(281);
    if (px_is_truthy(px_eq(px_index(_v1244, px_int(0LL)), px_str("Null")))) {
        px_srcline(282);
        return px_bool(true);
    }
    px_srcline(283);
    return px_bool(false);
px_err_1245:
    if (px_err_1245_proped) return px_err_1245_val;
    return px_null();
}

static LXValue fn_cg_sem_vardecl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_vardecl");
    LXValue _v1246 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1247 = px_uninit();
    LXValue _v1248 = px_uninit();
    LXValue px_err_1249_val = px_null();
    int px_err_1249_proped = 0;
    px_srcline(301);
    _v1247 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1246, px_int(2LL))}, 1);
    px_srcline(303);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1247}, 1))) {
        px_srcline(304);
        return px_null();
    }
    px_srcline(305);
    if (px_is_truthy(({ LXValue _t1250 = px_eq(px_index(_v1246, px_int(1LL)), px_str("Let")); px_is_truthy(_t1250) ? _t1250 : px_eq(px_index(_v1246, px_int(1LL)), px_str("Const")); }))) {
        px_srcline(306);
        (void)(px_call(px_get_global("cg_mark_immutable"), (LXValue[]){_v1247}, 1));
    }
    px_srcline(307);
    _v1248 = px_index(_v1246, px_int(3LL));
    px_srcline(308);
    if (px_is_truthy(px_call(px_get_global("cg_is_nonnull_ty"), (LXValue[]){_v1248}, 1))) {
        px_srcline(309);
        px_index_set(px_get_global("cg_nonnull"), _v1247, px_int(1LL));
        px_srcline(310);
        if (px_is_truthy(px_call(px_get_global("cg_is_null_lit"), (LXValue[]){px_index(_v1246, px_int(4LL))}, 1))) {
            px_srcline(311);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(({ LXValue _s189 = px_add(px_add(px_str("无法将 null 赋给非可空类型 '"), px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1248}, 1)), px_str("'（可空类型请用 ")); LXValue _s190 = px_call(px_get_global("cg_ty_name"), (LXValue[]){_v1248}, 1); px_add(_s189, _s190); }), px_str("? 声明）"))}, 2));
        }
    }
px_err_1249:
    if (px_err_1249_proped) return px_err_1249_val;
    return px_null();
}

static LXValue fn_cg_sem_assign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_assign");
    LXValue _v1251 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1252 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1253 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1254 = px_uninit();
    LXValue px_err_1255_val = px_null();
    int px_err_1255_proped = 0;
    px_srcline(315);
    if (px_is_truthy(px_eq(_v1251, px_null()))) {
        px_srcline(316);
        return px_null();
    }
    px_srcline(317);
    if (px_is_truthy(px_ne(px_index(_v1251, px_int(0LL)), px_str("Var")))) {
        px_srcline(318);
        return px_null();
    }
    px_srcline(319);
    _v1254 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1251, px_int(1LL))}, 1);
    px_srcline(321);
    if (px_is_truthy(px_call(px_get_global("cg_is_wildcard"), (LXValue[]){_v1254}, 1))) {
        px_srcline(322);
        return px_null();
    }
    px_srcline(323);
    if (px_is_truthy(px_call(px_get_global("cg_is_immutable"), (LXValue[]){_v1254}, 1))) {
        px_srcline(324);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3002"), px_add(px_add(px_str("对不可变变量 '"), _v1254), px_str("' 赋值（let 默认不可变，需用 let mut/var 声明可变）"))}, 2));
    }
    px_srcline(325);
    if (px_is_truthy(({ LXValue _t1256 = px_call(px_get_global("cg_is_null_lit"), (LXValue[]){_v1253}, 1); px_is_truthy(_t1256) ? px_method(px_get_global("cg_nonnull"), "has", (LXValue[]){_v1254}, 1) : _t1256; }))) {
        px_srcline(326);
        (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3003"), px_add(px_add(px_add(px_add(px_str("无法将 null 赋给非可空类型变量 '"), _v1254), px_str("'（可空类型请声明为 ")), _v1254), px_str(": T?）"))}, 2));
    }
px_err_1255:
    if (px_err_1255_proped) return px_err_1255_val;
    return px_null();
}

static LXValue fn_cg_sem_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_sem_call");
    LXValue _v1257 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1258 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1259 = px_uninit();
    LXValue _v1260 = px_uninit();
    LXValue px_err_1261_val = px_null();
    int px_err_1261_proped = 0;
    px_srcline(329);
    if (px_is_truthy(px_eq(_v1257, px_null()))) {
        px_srcline(330);
        return px_null();
    }
    px_srcline(331);
    if (px_is_truthy(px_ne(px_index(_v1257, px_int(0LL)), px_str("Var")))) {
        px_srcline(332);
        return px_null();
    }
    px_srcline(333);
    _v1259 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1257, px_int(1LL))}, 1);
    px_srcline(334);
    if (px_is_truthy(px_method(px_get_global("cg_ffi"), "has", (LXValue[]){_v1259}, 1))) {
        px_srcline(335);
        _v1260 = px_index(px_get_global("cg_ffi"), _v1259);
        px_srcline(336);
        if (px_is_truthy(({ LXValue _s191 = px_call(px_get_global("len"), (LXValue[]){_v1258}, 1); LXValue _s192 = px_call(px_get_global("len"), (LXValue[]){_v1260}, 1); px_ne(_s191, _s192); }))) {
            px_srcline(337);
            (void)(px_call(px_get_global("cg_perr"), (LXValue[]){px_str("E3004"), ({ LXValue _s193 = px_add(px_add(px_add(px_add(px_str("FFI 函数 "), _v1259), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1260}, 1)}, 1)), px_str(" 个参数，给出 ")); LXValue _s194 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1258}, 1)}, 1); px_add(_s193, _s194); })}, 2));
        }
    }
px_err_1261:
    if (px_err_1261_proped) return px_err_1261_val;
    return px_null();
}

static LXValue fn_cg_ty_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_ty_name");
    LXValue _v1262 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1263_val = px_null();
    int px_err_1263_proped = 0;
    px_srcline(340);
    if (px_is_truthy(px_eq(_v1262, px_null()))) {
        px_srcline(341);
        return px_str("any");
    }
    px_srcline(342);
    if (px_is_truthy(px_eq(px_index(_v1262, px_int(0LL)), px_str("TyOptional")))) {
        px_srcline(343);
        return px_add(px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1262, px_int(1LL))}, 1), px_str("?"));
    }
    px_srcline(344);
    if (px_is_truthy(px_eq(px_index(_v1262, px_int(0LL)), px_str("TyNamed")))) {
        px_srcline(345);
        return px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1262, px_int(1LL))}, 1);
    }
    px_srcline(346);
    if (px_is_truthy(px_eq(px_index(_v1262, px_int(0LL)), px_str("TyList")))) {
        px_srcline(347);
        return px_add(px_add(px_str("list["), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1262, px_int(1LL))}, 1)), px_str("]"));
    }
    px_srcline(348);
    if (px_is_truthy(px_eq(px_index(_v1262, px_int(0LL)), px_str("TyDict")))) {
        px_srcline(349);
        return px_add(({ LXValue _s195 = px_add(px_add(px_str("{"), px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1262, px_int(1LL))}, 1)), px_str(": ")); LXValue _s196 = px_call(px_get_global("cg_ty_name"), (LXValue[]){px_index(_v1262, px_int(2LL))}, 1); px_add(_s195, _s196); }), px_str("}"));
    }
    px_srcline(350);
    return px_str("any");
px_err_1263:
    if (px_err_1263_proped) return px_err_1263_val;
    return px_null();
}

static LXValue fn_cg_func_cname(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_func_cname");
    LXValue _v1264 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1265 = px_uninit();
    LXValue _v1266 = px_uninit();
    LXValue _v1267 = px_uninit();
    LXValue _v1268 = px_uninit();
    LXValue _v1269 = px_uninit();
    LXValue _v1270 = px_uninit();
    LXValue px_err_1271_val = px_null();
    int px_err_1271_proped = 0;
    px_srcline(352);
    _v1265 = px_list_n((LXValue[]){}, 0);
    px_srcline(353);
    _v1266 = px_int(0LL);
    px_srcline(354);
    while (px_is_truthy(px_lt(_v1266, px_call(px_get_global("len"), (LXValue[]){_v1264}, 1)))) {
        px_srcline(355);
        _v1267 = px_index(_v1264, _v1266);
        px_srcline(356);
        _v1268 = ({ LXValue _t1272 = px_ge(_v1267, px_str("a")); px_is_truthy(_t1272) ? px_le(_v1267, px_str("z")) : _t1272; });
        px_srcline(357);
        _v1269 = ({ LXValue _t1273 = px_ge(_v1267, px_str("A")); px_is_truthy(_t1273) ? px_le(_v1267, px_str("Z")) : _t1273; });
        px_srcline(358);
        _v1270 = ({ LXValue _t1274 = px_ge(_v1267, px_str("0")); px_is_truthy(_t1274) ? px_le(_v1267, px_str("9")) : _t1274; });
        px_srcline(359);
        if (px_is_truthy(({ LXValue _t1276 = ({ LXValue _t1275 = _v1268; px_is_truthy(_t1275) ? _t1275 : _v1269; }); px_is_truthy(_t1276) ? _t1276 : _v1270; }))) {
            px_srcline(360);
            (void)(px_method(_v1265, "append", (LXValue[]){_v1267}, 1));
        }
        else {
            px_srcline(362);
            (void)(px_method(_v1265, "append", (LXValue[]){px_str("_")}, 1));
        }
        px_srcline(363);
         _v1266 = px_add(_v1266, px_int(1LL));
    }
    px_srcline(364);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1265}, 2);
px_err_1271:
    if (px_err_1271_proped) return px_err_1271_val;
    return px_null();
}

static LXValue fn_cg_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_find");
    LXValue _v1277 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1278 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1279 = px_uninit();
    LXValue _v1280 = px_uninit();
    LXValue _v1281 = px_uninit();
    LXValue _v1282 = px_uninit();
    LXValue _v1283 = px_uninit();
    LXValue px_err_1284_val = px_null();
    int px_err_1284_proped = 0;
    px_srcline(366);
    _v1279 = px_call(px_get_global("len"), (LXValue[]){_v1277}, 1);
    px_srcline(367);
    _v1280 = px_call(px_get_global("len"), (LXValue[]){_v1278}, 1);
    px_srcline(368);
    _v1281 = px_int(0LL);
    px_srcline(369);
    while (px_is_truthy(px_le(px_add(_v1281, _v1280), _v1279))) {
        px_srcline(370);
        _v1282 = px_int(0LL);
        px_srcline(371);
        _v1283 = px_bool(true);
        px_srcline(372);
        while (px_is_truthy(px_lt(_v1282, _v1280))) {
            px_srcline(373);
            if (px_is_truthy(px_ne(px_index(_v1277, px_add(_v1281, _v1282)), px_index(_v1278, _v1282)))) {
                px_srcline(374);
                 _v1283 = px_bool(false);
                px_srcline(375);
                break;
            }
            px_srcline(376);
             _v1282 = px_add(_v1282, px_int(1LL));
        }
        px_srcline(377);
        if (px_is_truthy(_v1283)) {
            px_srcline(378);
            return _v1281;
        }
        px_srcline(379);
         _v1281 = px_add(_v1281, px_int(1LL));
    }
    px_srcline(380);
    return px_neg(px_int(1LL));
px_err_1284:
    if (px_err_1284_proped) return px_err_1284_val;
    return px_null();
}

static LXValue fn_cg_pad(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad");
    LXValue _v1285 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1286 = px_uninit();
    LXValue _v1287 = px_uninit();
    LXValue px_err_1288_val = px_null();
    int px_err_1288_proped = 0;
    px_srcline(382);
    _v1286 = px_list_n((LXValue[]){}, 0);
    px_srcline(383);
    _v1287 = px_int(0LL);
    px_srcline(384);
    while (px_is_truthy(px_lt(_v1287, _v1285))) {
        px_srcline(385);
        (void)(px_method(_v1286, "append", (LXValue[]){px_str("    ")}, 1));
        px_srcline(386);
         _v1287 = px_add(_v1287, px_int(1LL));
    }
    px_srcline(387);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1286}, 2);
px_err_1288:
    if (px_err_1288_proped) return px_err_1288_val;
    return px_null();
}

static LXValue fn_cg_nul_char(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_nul_char");
    LXValue px_err_1289_val = px_null();
    int px_err_1289_proped = 0;
    px_srcline(394);
    return px_call(px_get_global("bytes_to_str"), (LXValue[]){px_call(px_get_global("int_to_bytes"), (LXValue[]){px_int(0LL), px_int(1LL)}, 2)}, 1);
px_err_1289:
    if (px_err_1289_proped) return px_err_1289_val;
    return px_null();
}

static LXValue fn_cg_has_nul(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_has_nul");
    LXValue _v1290 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1291 = px_uninit();
    LXValue _v1292 = px_uninit();
    LXValue px_err_1293_val = px_null();
    int px_err_1293_proped = 0;
    px_srcline(397);
    _v1291 = px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0);
    px_srcline(398);
    _v1292 = px_int(0LL);
    px_srcline(399);
    while (px_is_truthy(px_lt(_v1292, px_call(px_get_global("len"), (LXValue[]){_v1290}, 1)))) {
        px_srcline(400);
        if (px_is_truthy(px_eq(px_index(_v1290, _v1292), _v1291))) {
            px_srcline(401);
            return px_bool(true);
        }
        px_srcline(402);
         _v1292 = px_add(_v1292, px_int(1LL));
    }
    px_srcline(403);
    return px_bool(false);
px_err_1293:
    if (px_err_1293_proped) return px_err_1293_val;
    return px_null();
}

static LXValue fn_rust_unescape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("rust_unescape");
    LXValue _v1294 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1295 = px_uninit();
    LXValue _v1296 = px_uninit();
    LXValue _v1297 = px_uninit();
    LXValue _v1298 = px_uninit();
    LXValue _v1299 = px_uninit();
    LXValue _v1300 = px_uninit();
    LXValue _v1301 = px_uninit();
    LXValue px_err_1302_val = px_null();
    int px_err_1302_proped = 0;
    px_srcline(405);
    _v1295 = px_slice(_v1294, px_int(1LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v1294}, 1), px_int(1LL)), px_null());
    px_srcline(406);
    _v1296 = px_list_n((LXValue[]){}, 0);
    px_srcline(407);
    _v1297 = px_int(0LL);
    px_srcline(408);
    while (px_is_truthy(px_lt(_v1297, px_call(px_get_global("len"), (LXValue[]){_v1295}, 1)))) {
        px_srcline(409);
        _v1298 = px_index(_v1295, _v1297);
        px_srcline(410);
        if (px_is_truthy(px_eq(_v1298, px_str("\\")))) {
            px_srcline(411);
            _v1299 = px_index(_v1295, px_add(_v1297, px_int(1LL)));
            px_srcline(412);
            if (px_is_truthy(px_eq(_v1299, px_str("n")))) {
                px_srcline(413);
                (void)(px_method(_v1296, "append", (LXValue[]){px_str("\n")}, 1));
                px_srcline(414);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1299, px_str("t")))) {
                px_srcline(416);
                (void)(px_method(_v1296, "append", (LXValue[]){px_str("\t")}, 1));
                px_srcline(417);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1299, px_str("r")))) {
                px_srcline(419);
                (void)(px_method(_v1296, "append", (LXValue[]){px_str("\r")}, 1));
                px_srcline(420);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1299, px_str("0")))) {
                px_srcline(422);
                (void)(px_method(_v1296, "append", (LXValue[]){px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)}, 1));
                px_srcline(423);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1299, px_str("\"")))) {
                px_srcline(425);
                (void)(px_method(_v1296, "append", (LXValue[]){px_str("\"")}, 1));
                px_srcline(426);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1299, px_str("\\")))) {
                px_srcline(428);
                (void)(px_method(_v1296, "append", (LXValue[]){px_str("\\")}, 1));
                px_srcline(429);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
            else if (px_is_truthy(px_eq(_v1299, px_str("u")))) {
                px_srcline(431);
                _v1300 = px_add(_v1297, px_int(3LL));
                px_srcline(432);
                _v1301 = px_list_n((LXValue[]){}, 0);
                px_srcline(433);
                while (px_is_truthy(({ LXValue _t1303 = px_lt(_v1300, px_call(px_get_global("len"), (LXValue[]){_v1295}, 1)); px_is_truthy(_t1303) ? px_ne(px_index(_v1295, _v1300), px_str("}")) : _t1303; }))) {
                    px_srcline(434);
                    (void)(px_method(_v1301, "append", (LXValue[]){px_index(_v1295, _v1300)}, 1));
                    px_srcline(435);
                     _v1300 = px_add(_v1300, px_int(1LL));
                }
                px_srcline(436);
                (void)(px_method(_v1296, "append", (LXValue[]){px_call(px_get_global("hex_to_char"), (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1301}, 2)}, 1)}, 1));
                px_srcline(437);
                 _v1297 = px_add(_v1300, px_int(1LL));
            }
            else {
                px_srcline(439);
                (void)(px_method(_v1296, "append", (LXValue[]){_v1299}, 1));
                px_srcline(440);
                 _v1297 = px_add(_v1297, px_int(2LL));
            }
        }
        else {
            px_srcline(442);
            (void)(px_method(_v1296, "append", (LXValue[]){_v1298}, 1));
            px_srcline(443);
             _v1297 = px_add(_v1297, px_int(1LL));
        }
    }
    px_srcline(444);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1296}, 2);
px_err_1302:
    if (px_err_1302_proped) return px_err_1302_val;
    return px_null();
}

static LXValue fn_cg_escape_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_escape_str");
    LXValue _v1304 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1305 = px_uninit();
    LXValue _v1306 = px_uninit();
    LXValue _v1307 = px_uninit();
    LXValue px_err_1308_val = px_null();
    int px_err_1308_proped = 0;
    px_srcline(447);
    _v1305 = px_list_n((LXValue[]){}, 0);
    px_srcline(448);
    _v1306 = px_int(0LL);
    px_srcline(449);
    while (px_is_truthy(px_lt(_v1306, px_call(px_get_global("len"), (LXValue[]){_v1304}, 1)))) {
        px_srcline(450);
        _v1307 = px_index(_v1304, _v1306);
        px_srcline(451);
        if (px_is_truthy(px_eq(_v1307, px_str("\\")))) {
            px_srcline(452);
            (void)(px_method(_v1305, "append", (LXValue[]){px_str("\\\\")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("\"")))) {
            px_srcline(454);
            (void)(px_method(_v1305, "append", (LXValue[]){px_str("\\\"")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("\n")))) {
            px_srcline(456);
            (void)(px_method(_v1305, "append", (LXValue[]){px_str("\\n")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("\r")))) {
            px_srcline(458);
            (void)(px_method(_v1305, "append", (LXValue[]){px_str("\\r")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1307, px_str("\t")))) {
            px_srcline(460);
            (void)(px_method(_v1305, "append", (LXValue[]){px_str("\\t")}, 1));
        }
        else if (px_is_truthy(px_eq(_v1307, px_call(px_get_global("cg_nul_char"), (LXValue[]){}, 0)))) {
            px_srcline(468);
            (void)(px_method(_v1305, "append", (LXValue[]){px_str("\\000")}, 1));
        }
        else {
            px_srcline(470);
            (void)(px_method(_v1305, "append", (LXValue[]){_v1307}, 1));
        }
        px_srcline(471);
         _v1306 = px_add(_v1306, px_int(1LL));
    }
    px_srcline(472);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1305}, 2);
px_err_1308:
    if (px_err_1308_proped) return px_err_1308_val;
    return px_null();
}

static LXValue fn_cg_pad_zeros(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_pad_zeros");
    LXValue _v1309 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1310 = px_uninit();
    LXValue _v1311 = px_uninit();
    LXValue px_err_1312_val = px_null();
    int px_err_1312_proped = 0;
    px_srcline(475);
    _v1310 = px_list_n((LXValue[]){}, 0);
    px_srcline(476);
    _v1311 = px_int(0LL);
    px_srcline(477);
    while (px_is_truthy(px_lt(_v1311, _v1309))) {
        px_srcline(478);
        (void)(px_method(_v1310, "append", (LXValue[]){px_str("0")}, 1));
        px_srcline(479);
         _v1311 = px_add(_v1311, px_int(1LL));
    }
    px_srcline(480);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1310}, 2);
px_err_1312:
    if (px_err_1312_proped) return px_err_1312_val;
    return px_null();
}

static LXValue fn_cg_expand_sci(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_expand_sci");
    LXValue _v1313 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1314 = px_uninit();
    LXValue _v1315 = px_uninit();
    LXValue _v1316 = px_uninit();
    LXValue _v1317 = px_uninit();
    LXValue _v1318 = px_uninit();
    LXValue _v1319 = px_uninit();
    LXValue _v1320 = px_uninit();
    LXValue _v1321 = px_uninit();
    LXValue _v1322 = px_uninit();
    LXValue _v1323 = px_uninit();
    LXValue _v1324 = px_uninit();
    LXValue _v1325 = px_uninit();
    LXValue _v1326 = px_uninit();
    LXValue _v1327 = px_uninit();
    LXValue px_err_1328_val = px_null();
    int px_err_1328_proped = 0;
    px_srcline(482);
    _v1314 = px_neg(px_int(1LL));
    px_srcline(483);
    _v1315 = px_int(0LL);
    px_srcline(484);
    while (px_is_truthy(px_lt(_v1315, px_call(px_get_global("len"), (LXValue[]){_v1313}, 1)))) {
        px_srcline(485);
        if (px_is_truthy(({ LXValue _t1329 = px_eq(px_index(_v1313, _v1315), px_str("e")); px_is_truthy(_t1329) ? _t1329 : px_eq(px_index(_v1313, _v1315), px_str("E")); }))) {
            px_srcline(486);
             _v1314 = _v1315;
            px_srcline(487);
            break;
        }
        px_srcline(488);
         _v1315 = px_add(_v1315, px_int(1LL));
    }
    px_srcline(489);
    if (px_is_truthy(px_lt(_v1314, px_int(0LL)))) {
        px_srcline(490);
        return _v1313;
    }
    px_srcline(491);
    _v1316 = px_slice(_v1313, px_int(0LL), _v1314, px_null());
    px_srcline(492);
    _v1317 = px_slice(_v1313, px_add(_v1314, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1313}, 1), px_null());
    px_srcline(493);
    _v1318 = px_int(1LL);
    px_srcline(494);
    if (px_is_truthy(({ LXValue _t1330 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1317}, 1), px_int(0LL)); px_is_truthy(_t1330) ? px_eq(px_index(_v1317, px_int(0LL)), px_str("+")) : _t1330; }))) {
        px_srcline(495);
         _v1317 = px_slice(_v1317, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1317}, 1), px_null());
    }
    else if (px_is_truthy(({ LXValue _t1331 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1317}, 1), px_int(0LL)); px_is_truthy(_t1331) ? px_eq(px_index(_v1317, px_int(0LL)), px_str("-")) : _t1331; }))) {
        px_srcline(497);
         _v1318 = px_neg(px_int(1LL));
        px_srcline(498);
         _v1317 = px_slice(_v1317, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1317}, 1), px_null());
    }
    px_srcline(499);
    _v1319 = px_mul(px_call(px_get_global("int"), (LXValue[]){_v1317}, 1), _v1318);
    px_srcline(500);
    _v1320 = px_bool(false);
    px_srcline(501);
    if (px_is_truthy(({ LXValue _t1332 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1316}, 1), px_int(0LL)); px_is_truthy(_t1332) ? px_eq(px_index(_v1316, px_int(0LL)), px_str("-")) : _t1332; }))) {
        px_srcline(502);
         _v1320 = px_bool(true);
        px_srcline(503);
         _v1316 = px_slice(_v1316, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1316}, 1), px_null());
    }
    px_srcline(504);
    _v1321 = px_str("");
    px_srcline(505);
    _v1322 = px_str("");
    px_srcline(506);
    _v1323 = px_neg(px_int(1LL));
    px_srcline(507);
    _v1324 = px_int(0LL);
    px_srcline(508);
    while (px_is_truthy(px_lt(_v1324, px_call(px_get_global("len"), (LXValue[]){_v1316}, 1)))) {
        px_srcline(509);
        if (px_is_truthy(px_eq(px_index(_v1316, _v1324), px_str(".")))) {
            px_srcline(510);
             _v1323 = _v1324;
            px_srcline(511);
            break;
        }
        px_srcline(512);
         _v1324 = px_add(_v1324, px_int(1LL));
    }
    px_srcline(513);
    if (px_is_truthy(px_lt(_v1323, px_int(0LL)))) {
        px_srcline(514);
         _v1321 = _v1316;
    }
    else {
        px_srcline(516);
         _v1321 = px_slice(_v1316, px_int(0LL), _v1323, px_null());
        px_srcline(517);
         _v1322 = px_slice(_v1316, px_add(_v1323, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v1316}, 1), px_null());
    }
    px_srcline(518);
    _v1325 = px_add(_v1321, _v1322);
    px_srcline(519);
    _v1326 = px_add(px_call(px_get_global("len"), (LXValue[]){_v1321}, 1), _v1319);
    px_srcline(520);
    _v1327 = px_str("");
    px_srcline(521);
    if (px_is_truthy(px_le(_v1326, px_int(0LL)))) {
        px_srcline(522);
         _v1327 = px_add(px_add(px_str("0."), px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(px_int(0LL), _v1326)}, 1)), _v1325);
    }
    else if (px_is_truthy(px_ge(_v1326, px_call(px_get_global("len"), (LXValue[]){_v1325}, 1)))) {
        px_srcline(524);
         _v1327 = px_add(_v1325, px_call(px_get_global("cg_pad_zeros"), (LXValue[]){px_sub(_v1326, px_call(px_get_global("len"), (LXValue[]){_v1325}, 1))}, 1));
    }
    else {
        px_srcline(526);
         _v1327 = px_add(px_add(px_slice(_v1325, px_int(0LL), _v1326, px_null()), px_str(".")), px_slice(_v1325, _v1326, px_call(px_get_global("len"), (LXValue[]){_v1325}, 1), px_null()));
    }
    px_srcline(527);
    if (px_is_truthy(_v1320)) {
        px_srcline(528);
        return px_add(px_str("-"), _v1327);
    }
    px_srcline(529);
    return _v1327;
px_err_1328:
    if (px_err_1328_proped) return px_err_1328_val;
    return px_null();
}

static LXValue fn_cg_fmt_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_fmt_float");
    LXValue _v1333 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1334 = px_uninit();
    LXValue _v1335 = px_uninit();
    LXValue _v1336 = px_uninit();
    LXValue px_err_1337_val = px_null();
    int px_err_1337_proped = 0;
    px_srcline(532);
    _v1334 = px_call(px_get_global("str"), (LXValue[]){_v1333}, 1);
    px_srcline(533);
    if (px_is_truthy(({ LXValue _t1340 = ({ LXValue _t1339 = ({ LXValue _t1338 = px_eq(_v1334, px_str("inf")); px_is_truthy(_t1338) ? _t1338 : px_eq(_v1334, px_str("-inf")); }); px_is_truthy(_t1339) ? _t1339 : px_eq(_v1334, px_str("nan")); }); px_is_truthy(_t1340) ? _t1340 : px_eq(_v1334, px_str("-nan")); }))) {
        px_srcline(534);
        return _v1334;
    }
    px_srcline(535);
     _v1334 = px_call(px_get_global("cg_expand_sci"), (LXValue[]){_v1334}, 1);
    px_srcline(536);
    _v1335 = px_call(px_get_global("len"), (LXValue[]){_v1334}, 1);
    px_srcline(537);
    if (px_is_truthy(({ LXValue _t1341 = px_ge(_v1335, px_int(2LL)); px_is_truthy(_t1341) ? px_eq(px_slice(_v1334, px_sub(_v1335, px_int(2LL)), _v1335, px_null()), px_str(".0")) : _t1341; }))) {
        px_srcline(543);
        _v1336 = px_slice(_v1334, px_int(0LL), px_sub(_v1335, px_int(2LL)), px_null());
        px_srcline(544);
        if (px_is_truthy(px_eq(_v1336, px_str("-0")))) {
            px_srcline(545);
            return _v1334;
        }
        px_srcline(546);
        return _v1336;
    }
    px_srcline(554);
    if (px_is_truthy(({ LXValue _t1342 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1334, px_str(".")}, 2)); px_is_truthy(_t1342) ? px_ge(_v1335, px_int(19LL)) : _t1342; }))) {
        px_srcline(555);
        return px_add(_v1334, px_str(".0"));
    }
    px_srcline(556);
    return _v1334;
px_err_1337:
    if (px_err_1337_proped) return px_err_1337_val;
    return px_null();
}

static LXValue fn_cg_collect_types(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_types");
    LXValue _v1343 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1344 = px_uninit();
    LXValue _v1345 = px_uninit();
    LXValue _v1346 = px_uninit();
    LXValue _v1347 = px_uninit();
    LXValue _v1348 = px_uninit();
    LXValue _v1349 = px_uninit();
    LXValue _v1350 = px_uninit();
    LXValue _v1351 = px_uninit();
    LXValue _v1352 = px_uninit();
    LXValue _v1353 = px_uninit();
    LXValue px_err_1354_val = px_null();
    int px_err_1354_proped = 0;
    px_srcline(559);
    _v1344 = px_index(_v1343, px_int(1LL));
    px_srcline(560);
    _v1345 = px_int(0LL);
    px_srcline(561);
    while (px_is_truthy(px_lt(_v1345, px_call(px_get_global("len"), (LXValue[]){_v1344}, 1)))) {
        px_srcline(562);
        _v1346 = px_index(_v1344, _v1345);
        px_srcline(563);
        _v1347 = px_index(_v1346, px_int(0LL));
        px_srcline(564);
        if (px_is_truthy(px_eq(_v1347, px_str("StructDef")))) {
            px_srcline(565);
            _v1348 = px_list_n((LXValue[]){}, 0);
            px_srcline(566);
            _v1349 = px_int(0LL);
            px_srcline(567);
            while (px_is_truthy(px_lt(_v1349, px_call(px_get_global("len"), (LXValue[]){px_index(_v1346, px_int(2LL))}, 1)))) {
                px_srcline(568);
                (void)(px_method(_v1348, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1346, px_int(2LL)), _v1349), px_int(1LL))}, 1)}, 1));
                px_srcline(569);
                 _v1349 = px_add(_v1349, px_int(1LL));
            }
            px_srcline(570);
            px_index_set(px_get_global("cg_structs"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1346, px_int(1LL))}, 1), _v1348);
        }
        else if (px_is_truthy(px_eq(_v1347, px_str("EnumDef")))) {
            px_srcline(572);
            _v1350 = px_list_n((LXValue[]){}, 0);
            px_srcline(573);
            _v1351 = px_int(0LL);
            px_srcline(574);
            while (px_is_truthy(px_lt(_v1351, px_call(px_get_global("len"), (LXValue[]){px_index(_v1346, px_int(2LL))}, 1)))) {
                px_srcline(575);
                (void)(px_method(_v1350, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1346, px_int(2LL)), _v1351), px_int(1LL))}, 1)}, 1));
                px_srcline(576);
                 _v1351 = px_add(_v1351, px_int(1LL));
            }
            px_srcline(577);
            px_index_set(px_get_global("cg_enums"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1346, px_int(1LL))}, 1), _v1350);
        }
        else if (px_is_truthy(px_eq(_v1347, px_str("ImplDef")))) {
            px_srcline(579);
            _v1352 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1346, px_int(1LL))}, 1);
            px_srcline(580);
            if (px_is_truthy(px_method(px_get_global("cg_impls"), "has", (LXValue[]){_v1352}, 1))) {
                px_srcline(581);
                _v1353 = px_int(0LL);
                px_srcline(582);
                while (px_is_truthy(px_lt(_v1353, px_call(px_get_global("len"), (LXValue[]){px_index(_v1346, px_int(3LL))}, 1)))) {
                    px_srcline(583);
                    (void)(px_method(px_index(px_get_global("cg_impls"), _v1352), "append", (LXValue[]){px_index(px_index(_v1346, px_int(3LL)), _v1353)}, 1));
                    px_srcline(584);
                     _v1353 = px_add(_v1353, px_int(1LL));
                }
            }
            else {
                px_srcline(586);
                px_index_set(px_get_global("cg_impls"), _v1352, px_index(_v1346, px_int(3LL)));
            }
        }
        px_srcline(587);
         _v1345 = px_add(_v1345, px_int(1LL));
    }
    px_srcline(590);
    (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){_v1344}, 1));
px_err_1354:
    if (px_err_1354_proped) return px_err_1354_val;
    return px_null();
}

static LXValue fn_cg_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_consts");
    LXValue _v1355 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1356 = px_uninit();
    LXValue _v1357 = px_uninit();
    LXValue _v1358 = px_uninit();
    LXValue _v1359 = px_uninit();
    LXValue _v1360 = px_uninit();
    LXValue _v1361 = px_uninit();
    LXValue _v1362 = px_uninit();
    LXValue _v1363 = px_uninit();
    LXValue px_err_1364_val = px_null();
    int px_err_1364_proped = 0;
    px_srcline(593);
    _v1356 = px_int(0LL);
    px_srcline(594);
    while (px_is_truthy(px_lt(_v1356, px_call(px_get_global("len"), (LXValue[]){_v1355}, 1)))) {
        px_srcline(595);
        _v1357 = px_index(_v1355, _v1356);
        px_srcline(596);
        _v1358 = px_index(_v1357, px_int(0LL));
        px_srcline(597);
        if (px_is_truthy(px_eq(_v1358, px_str("TypeConst")))) {
            px_srcline(598);
            _v1359 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1357, px_int(1LL))}, 1);
            px_srcline(599);
            _v1360 = px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0);
            px_srcline(600);
            _v1361 = px_int(0LL);
            px_srcline(601);
            while (px_is_truthy(px_lt(_v1361, px_call(px_get_global("len"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1)))) {
                px_srcline(602);
                _v1362 = px_index(px_index(_v1357, px_int(2LL)), _v1361);
                px_srcline(603);
                ({ LXValue _s197 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1362, px_int(1LL))}, 1); LXValue _s198 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1362, px_int(2LL))}, 1); px_index_set(_v1360, _s197, _s198); });
                px_srcline(604);
                 _v1361 = px_add(_v1361, px_int(1LL));
            }
            px_srcline(605);
            px_index_set(px_get_global("cg_const_enums"), _v1359, _v1360);
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("FuncDef")))) {
            px_srcline(607);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1357, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("If")))) {
            px_srcline(609);
            _v1363 = px_int(0LL);
            px_srcline(610);
            while (px_is_truthy(px_lt(_v1363, px_call(px_get_global("len"), (LXValue[]){px_index(_v1357, px_int(1LL))}, 1)))) {
                px_srcline(611);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1357, px_int(1LL)), _v1363), px_int(1LL))}, 1));
                px_srcline(612);
                 _v1363 = px_add(_v1363, px_int(1LL));
            }
            px_srcline(613);
            if (px_is_truthy(px_ne(px_index(_v1357, px_int(2LL)), px_null()))) {
                px_srcline(614);
                (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("For")))) {
            px_srcline(616);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1357, px_int(3LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1358, px_str("While")))) {
            px_srcline(618);
            (void)(px_call(px_get_global("cg_collect_consts"), (LXValue[]){px_index(_v1357, px_int(2LL))}, 1));
        }
        px_srcline(619);
         _v1356 = px_add(_v1356, px_int(1LL));
    }
px_err_1364:
    if (px_err_1364_proped) return px_err_1364_val;
    return px_null();
}

static LXValue fn_cg_collect_hoist_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_hoist_vars");
    LXValue _v1365 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1366 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1367 = px_uninit();
    LXValue _v1368 = px_uninit();
    LXValue _v1369 = px_uninit();
    LXValue _v1370 = px_uninit();
    LXValue _v1371 = px_uninit();
    LXValue _v1372 = px_uninit();
    LXValue _v1373 = px_uninit();
    LXValue _v1374 = px_uninit();
    LXValue _v1375 = px_uninit();
    LXValue px_err_1376_val = px_null();
    int px_err_1376_proped = 0;
    px_srcline(628);
    _v1367 = px_int(0LL);
    px_srcline(629);
    while (px_is_truthy(px_lt(_v1367, px_call(px_get_global("len"), (LXValue[]){_v1365}, 1)))) {
        px_srcline(630);
        _v1368 = px_index(_v1365, _v1367);
        px_srcline(631);
        _v1369 = px_index(_v1368, px_int(0LL));
        px_srcline(632);
        if (px_is_truthy(px_eq(_v1369, px_str("Assign")))) {
            px_srcline(633);
            _v1370 = px_index(_v1368, px_int(1LL));
            px_srcline(634);
            if (px_is_truthy(px_eq(px_index(_v1370, px_int(0LL)), px_str("Var")))) {
                px_srcline(635);
                _v1371 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1370, px_int(1LL))}, 1);
                px_srcline(636);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1366, _v1371}, 2)))) {
                    px_srcline(637);
                    (void)(px_method(_v1366, "append", (LXValue[]){_v1371}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1369, px_str("VarDecl")))) {
            px_srcline(639);
            _v1371 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1368, px_int(2LL))}, 1);
            px_srcline(640);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1366, _v1371}, 2)))) {
                px_srcline(641);
                (void)(px_method(_v1366, "append", (LXValue[]){_v1371}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1369, px_str("FuncDef")))) {
            px_srcline(646);
            _v1371 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1368, px_int(1LL))}, 1);
            px_srcline(647);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1366, _v1371}, 2)))) {
                px_srcline(648);
                (void)(px_method(_v1366, "append", (LXValue[]){_v1371}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1369, px_str("If")))) {
            px_srcline(650);
            _v1372 = px_index(_v1368, px_int(1LL));
            px_srcline(651);
            _v1373 = px_int(0LL);
            px_srcline(652);
            while (px_is_truthy(px_lt(_v1373, px_call(px_get_global("len"), (LXValue[]){_v1372}, 1)))) {
                px_srcline(653);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(px_index(_v1372, _v1373), px_int(1LL)), _v1366}, 2));
                px_srcline(654);
                 _v1373 = px_add(_v1373, px_int(1LL));
            }
            px_srcline(655);
            if (px_is_truthy(px_ne(px_index(_v1368, px_int(2LL)), px_null()))) {
                px_srcline(656);
                (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1368, px_int(2LL)), _v1366}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1369, px_str("For")))) {
            px_srcline(659);
            _v1374 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1368, px_int(1LL))}, 1);
            px_srcline(660);
            _v1375 = px_int(0LL);
            px_srcline(661);
            while (px_is_truthy(px_lt(_v1375, px_call(px_get_global("len"), (LXValue[]){_v1374}, 1)))) {
                px_srcline(662);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1366, px_index(_v1374, _v1375)}, 2)))) {
                    px_srcline(663);
                    (void)(px_method(_v1366, "append", (LXValue[]){px_index(_v1374, _v1375)}, 1));
                }
                px_srcline(664);
                 _v1375 = px_add(_v1375, px_int(1LL));
            }
            px_srcline(665);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1368, px_int(3LL)), _v1366}, 2));
        }
        else if (px_is_truthy(px_eq(_v1369, px_str("While")))) {
            px_srcline(667);
            (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1368, px_int(2LL)), _v1366}, 2));
        }
        px_srcline(668);
         _v1367 = px_add(_v1367, px_int(1LL));
    }
px_err_1376:
    if (px_err_1376_proped) return px_err_1376_val;
    return px_null();
}

static LXValue fn_cg_collect_decl_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_collect_decl_vars");
    LXValue _v1377 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1378 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1379 = px_uninit();
    LXValue _v1380 = px_uninit();
    LXValue _v1381 = px_uninit();
    LXValue _v1382 = px_uninit();
    LXValue _v1383 = px_uninit();
    LXValue _v1384 = px_uninit();
    LXValue _v1385 = px_uninit();
    LXValue _v1386 = px_uninit();
    LXValue _v1387 = px_uninit();
    LXValue px_err_1388_val = px_null();
    int px_err_1388_proped = 0;
    px_srcline(679);
    _v1379 = px_int(0LL);
    px_srcline(680);
    while (px_is_truthy(px_lt(_v1379, px_call(px_get_global("len"), (LXValue[]){_v1377}, 1)))) {
        px_srcline(681);
        _v1380 = px_index(_v1377, _v1379);
        px_srcline(682);
        _v1381 = px_index(_v1380, px_int(0LL));
        px_srcline(683);
        if (px_is_truthy(px_eq(_v1381, px_str("VarDecl")))) {
            px_srcline(684);
            _v1382 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1380, px_int(2LL))}, 1);
            px_srcline(685);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1378, _v1382}, 2)))) {
                px_srcline(686);
                (void)(px_method(_v1378, "append", (LXValue[]){_v1382}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1381, px_str("FuncDef")))) {
            px_srcline(688);
            _v1382 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1380, px_int(1LL))}, 1);
            px_srcline(689);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1378, _v1382}, 2)))) {
                px_srcline(690);
                (void)(px_method(_v1378, "append", (LXValue[]){_v1382}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1381, px_str("ChanDecl")))) {
            px_srcline(692);
            _v1383 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1380, px_int(1LL))}, 1);
            px_srcline(693);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1378, _v1383}, 2)))) {
                px_srcline(694);
                (void)(px_method(_v1378, "append", (LXValue[]){_v1383}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1381, px_str("If")))) {
            px_srcline(696);
            _v1384 = px_index(_v1380, px_int(1LL));
            px_srcline(697);
            _v1385 = px_int(0LL);
            px_srcline(698);
            while (px_is_truthy(px_lt(_v1385, px_call(px_get_global("len"), (LXValue[]){_v1384}, 1)))) {
                px_srcline(699);
                (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(px_index(_v1384, _v1385), px_int(1LL)), _v1378}, 2));
                px_srcline(700);
                 _v1385 = px_add(_v1385, px_int(1LL));
            }
            px_srcline(701);
            if (px_is_truthy(px_ne(px_index(_v1380, px_int(2LL)), px_null()))) {
                px_srcline(702);
                (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1380, px_int(2LL)), _v1378}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1381, px_str("For")))) {
            px_srcline(704);
            _v1386 = px_call(px_get_global("cg_for_names"), (LXValue[]){px_index(_v1380, px_int(1LL))}, 1);
            px_srcline(705);
            _v1387 = px_int(0LL);
            px_srcline(706);
            while (px_is_truthy(px_lt(_v1387, px_call(px_get_global("len"), (LXValue[]){_v1386}, 1)))) {
                px_srcline(707);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1378, px_index(_v1386, _v1387)}, 2)))) {
                    px_srcline(708);
                    (void)(px_method(_v1378, "append", (LXValue[]){px_index(_v1386, _v1387)}, 1));
                }
                px_srcline(709);
                 _v1387 = px_add(_v1387, px_int(1LL));
            }
            px_srcline(710);
            (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1380, px_int(3LL)), _v1378}, 2));
        }
        else if (px_is_truthy(px_eq(_v1381, px_str("While")))) {
            px_srcline(712);
            (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1380, px_int(2LL)), _v1378}, 2));
        }
        px_srcline(713);
         _v1379 = px_add(_v1379, px_int(1LL));
    }
px_err_1388:
    if (px_err_1388_proped) return px_err_1388_val;
    return px_null();
}

static LXValue fn_cg_gen_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func");
    LXValue _v1389 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1390 = px_uninit();
    LXValue px_err_1391_val = px_null();
    int px_err_1391_proped = 0;
    px_srcline(716);
    _v1390 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1389, px_int(1LL))}, 1)}, 1));
    px_srcline(717);
    return px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1389, _v1390}, 2);
px_err_1391:
    if (px_err_1391_proped) return px_err_1391_val;
    return px_null();
}

static LXValue fn_cg_gen_func_named(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_gen_func_named");
    LXValue _v1392 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1393 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v1413 = px_uninit();
    LXValue _v1414 = px_uninit();
    LXValue _v1415 = px_uninit();
    LXValue _v1416 = px_uninit();
    LXValue _v1417 = px_uninit();
    LXValue _v1418 = px_uninit();
    LXValue _v1419 = px_uninit();
    LXValue _v1420 = px_uninit();
    LXValue px_err_1421_val = px_null();
    int px_err_1421_proped = 0;
    px_srcline(719);
    _v1394 = px_add(px_add(px_str("static LXValue "), _v1393), px_str("(LXValue* args, int nargs, void* ctx) {\n"));
    px_srcline(720);
     _v1394 = px_add(_v1394, px_str("    (void)ctx;\n"));
    px_srcline(723);
     _v1394 = px_add(_v1394, px_add(px_add(px_str("    px_srcfunc(\""), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1392, px_int(1LL))}, 1)), px_str("\");\n")));
    px_srcline(724);
    _v1395 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_vars")}, 1);
    px_srcline(725);
    _v1396 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_var_types")}, 1);
    px_srcline(726);
    _v1397 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_cells")}, 1);
    px_srcline(727);
    _v1398 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(729);
    _v1399 = px_get_global("cg_topbody");
    px_srcline(730);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(732);
    _v1400 = px_call(px_get_global("cg_inited_copy"), (LXValue[]){}, 0);
    px_srcline(733);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(734);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(735);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(736);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(737);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1398}, 1));
    px_srcline(741);
    _v1401 = px_list_n((LXValue[]){}, 0);
    px_srcline(742);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){px_index(_v1392, px_int(4LL)), _v1401}, 2));
    px_srcline(744);
    _v1402 = px_index(_v1392, px_int(2LL));
    px_srcline(745);
    _v1403 = px_int(0LL);
    px_srcline(746);
    _v1404 = px_list_n((LXValue[]){}, 0);
    px_srcline(747);
    while (px_is_truthy(px_lt(_v1403, px_call(px_get_global("len"), (LXValue[]){_v1402}, 1)))) {
        px_srcline(748);
        _v1405 = px_index(_v1402, _v1403);
        px_srcline(749);
        _v1406 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1405, px_int(1LL))}, 1);
        px_srcline(750);
        _v1407 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1406}, 1);
        px_srcline(751);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1401, _v1406}, 2))) {
            px_srcline(752);
            px_index_set(px_get_global("cg_cells"), _v1406, px_int(1LL));
        }
        px_srcline(753);
        _v1408 = px_str("px_null()");
        px_srcline(754);
        if (px_is_truthy(px_ne(px_index(_v1405, px_int(3LL)), px_null()))) {
            px_srcline(755);
             _v1408 = px_call(px_get_global("cg_gen_expr"), (LXValue[]){px_index(_v1405, px_int(3LL))}, 1);
        }
        px_srcline(756);
        (void)(px_method(_v1404, "append", (LXValue[]){px_list_n((LXValue[]){_v1407, _v1406, px_add(px_add(({ LXValue _s199 = px_add(px_add(px_str("(nargs > "), px_call(px_get_global("str"), (LXValue[]){_v1403}, 1)), px_str(") ? args[")); LXValue _s200 = px_call(px_get_global("str"), (LXValue[]){_v1403}, 1); px_add(_s199, _s200); }), px_str("] : ")), _v1408)}, 3)}, 1));
        px_srcline(758);
        (void)(px_call(px_get_global("cg_inited_add"), (LXValue[]){_v1406}, 1));
        px_srcline(759);
         _v1403 = px_add(_v1403, px_int(1LL));
    }
    px_srcline(762);
    _v1409 = px_list_n((LXValue[]){}, 0);
    px_srcline(763);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){px_index(_v1392, px_int(4LL)), _v1409}, 2));
    px_srcline(766);
    _v1410 = px_list_n((LXValue[]){}, 0);
    px_srcline(767);
    (void)(px_call(px_get_global("cg_collect_decl_vars"), (LXValue[]){px_index(_v1392, px_int(4LL)), _v1410}, 2));
    px_srcline(768);
    _v1411 = px_list_n((LXValue[]){}, 0);
    px_srcline(769);
    _v1412 = px_int(0LL);
    px_srcline(770);
    while (px_is_truthy(px_lt(_v1412, px_call(px_get_global("len"), (LXValue[]){_v1409}, 1)))) {
        px_srcline(771);
        _v1413 = px_index(_v1409, _v1412);
        px_srcline(772);
        if (px_is_truthy(px_ne(px_call(px_get_global("cg_var_of"), (LXValue[]){_v1413}, 1), px_null()))) {
            px_srcline(773);
             _v1412 = px_add(_v1412, px_int(1LL));
            px_srcline(774);
            continue;
        }
        px_srcline(775);
        if (px_is_truthy(({ LXValue _t1422 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1410, _v1413}, 2)); px_is_truthy(_t1422) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("cg_topnames"), _v1413}, 2) : _t1422; }))) {
            px_srcline(776);
             _v1412 = px_add(_v1412, px_int(1LL));
            px_srcline(777);
            continue;
        }
        px_srcline(778);
        _v1407 = px_call(px_get_global("cg_new_var"), (LXValue[]){_v1413}, 1);
        px_srcline(779);
        if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){_v1401, _v1413}, 2))) {
            px_srcline(780);
            px_index_set(px_get_global("cg_cells"), _v1413, px_int(1LL));
        }
        px_srcline(781);
        (void)(px_method(_v1411, "append", (LXValue[]){px_list_n((LXValue[]){_v1407, _v1413}, 2)}, 1));
        px_srcline(782);
         _v1412 = px_add(_v1412, px_int(1LL));
    }
    px_srcline(784);
    _v1414 = px_int(0LL);
    px_srcline(785);
    while (px_is_truthy(px_lt(_v1414, px_call(px_get_global("len"), (LXValue[]){_v1404}, 1)))) {
        px_srcline(786);
        _v1415 = px_index(_v1404, _v1414);
        px_srcline(787);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1415, px_int(1LL))}, 1))) {
            px_srcline(788);
             _v1394 = px_add(_v1394, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1415, px_int(0LL))), px_str(" = px_cell(")), px_index(_v1415, px_int(2LL))), px_str(");\n")));
        }
        else {
            px_srcline(790);
             _v1394 = px_add(_v1394, px_add(px_add(px_add(px_add(px_str("    LXValue "), px_index(_v1415, px_int(0LL))), px_str(" = ")), px_index(_v1415, px_int(2LL))), px_str(";\n")));
        }
        px_srcline(791);
         _v1414 = px_add(_v1414, px_int(1LL));
    }
    px_srcline(792);
     _v1414 = px_int(0LL);
    px_srcline(793);
    while (px_is_truthy(px_lt(_v1414, px_call(px_get_global("len"), (LXValue[]){_v1411}, 1)))) {
        px_srcline(794);
        _v1416 = px_index(_v1411, _v1414);
        px_srcline(798);
        if (px_is_truthy(px_method(px_get_global("cg_cells"), "has", (LXValue[]){px_index(_v1416, px_int(1LL))}, 1))) {
            px_srcline(799);
             _v1394 = px_add(_v1394, px_add(px_add(px_str("    LXValue "), px_index(_v1416, px_int(0LL))), px_str(" = px_cell(px_uninit());\n")));
        }
        else {
            px_srcline(801);
             _v1394 = px_add(_v1394, px_add(px_add(px_str("    LXValue "), px_index(_v1416, px_int(0LL))), px_str(" = px_uninit();\n")));
        }
        px_srcline(802);
         _v1414 = px_add(_v1414, px_int(1LL));
    }
    px_srcline(804);
    _v1417 = px_add(px_str("px_err_"), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("cg_uid"), (LXValue[]){}, 0)}, 1));
    px_srcline(805);
    (void)(px_method(px_get_global("cg_err_labels"), "append", (LXValue[]){_v1417}, 1));
    px_srcline(806);
     _v1394 = px_add(_v1394, px_add(px_add(px_str("    LXValue "), _v1417), px_str("_val = px_null();\n")));
    px_srcline(807);
     _v1394 = px_add(_v1394, px_add(px_add(px_str("    int "), _v1417), px_str("_proped = 0;\n")));
    px_srcline(811);
    _v1418 = px_index(_v1392, px_int(4LL));
    px_srcline(812);
    _v1419 = px_list_n((LXValue[]){}, 0);
    px_srcline(813);
    _v1420 = px_int(0LL);
    px_srcline(814);
    while (px_is_truthy(px_lt(_v1420, px_call(px_get_global("len"), (LXValue[]){_v1418}, 1)))) {
        px_srcline(815);
        (void)(px_method(_v1419, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){px_index(_v1418, _v1420), px_int(1LL)}, 2)}, 1));
        px_srcline(816);
         _v1420 = px_add(_v1420, px_int(1LL));
    }
    px_srcline(817);
     _v1394 = px_add(_v1394, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1419}, 2));
    px_srcline(818);
     _v1394 = px_add(_v1394, px_add(_v1417, px_str(":\n")));
    px_srcline(819);
     _v1394 = px_add(_v1394, px_add(px_add(px_add(px_add(px_str("    if ("), _v1417), px_str("_proped) return ")), _v1417), px_str("_val;\n")));
    px_srcline(820);
     _v1394 = px_add(_v1394, px_str("    return px_null();\n"));
    px_srcline(821);
     _v1394 = px_add(_v1394, px_str("}\n"));
    px_srcline(822);
    px_set_global("cg_err_labels", px_slice(px_get_global("cg_err_labels"), px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){px_get_global("cg_err_labels")}, 1), px_int(1LL)), px_null()));
    px_srcline(823);
    px_set_global("cg_vars", _v1395);
    px_srcline(824);
    px_set_global("cg_var_types", _v1396);
    px_srcline(825);
    px_set_global("cg_cells", _v1397);
    px_srcline(826);
    px_set_global("cg_immutables", _v1398);
    px_srcline(827);
    px_set_global("cg_topbody", _v1399);
    px_srcline(828);
    px_set_global("cg_inited", _v1400);
    px_srcline(829);
    return _v1394;
px_err_1421:
    if (px_err_1421_proped) return px_err_1421_val;
    return px_null();
}

static LXValue fn_cg_generate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("cg_generate");
    LXValue _v1423 = (nargs > 0) ? args[0] : px_null();
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
    LXValue _v1451 = px_uninit();
    LXValue _v1452 = px_uninit();
    LXValue _v1453 = px_uninit();
    LXValue _v1454 = px_uninit();
    LXValue _v1455 = px_uninit();
    LXValue _v1456 = px_uninit();
    LXValue _v1457 = px_uninit();
    LXValue _v1458 = px_uninit();
    LXValue px_err_1459_val = px_null();
    int px_err_1459_proped = 0;
    px_srcline(832);
    _v1424 = px_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n#include \"runtime.h\"\n#include <string.h>\n#include <stdio.h>\n\n");
    px_srcline(833);
    px_set_global("cg_closures", px_str(""));
    px_srcline(834);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(835);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(836);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(837);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(838);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(839);
    px_set_global("cg_cells", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(840);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(841);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(842);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(843);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(844);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(845);
    px_set_global("cg_err_labels", px_list_n((LXValue[]){}, 0));
    px_srcline(846);
    px_set_global("cg_uidc", px_int(0LL));
    px_srcline(847);
    px_set_global("cg_closure_id", px_int(0LL));
    px_srcline(848);
    px_set_global("cg_seq_uid", px_int(0LL));
    px_srcline(849);
    px_set_global("cg_iter_uid", px_int(0LL));
    px_srcline(850);
    px_set_global("cg_unpack_uid", px_int(0LL));
    px_srcline(851);
    px_set_global("cg_cerr_uid", px_int(0LL));
    px_srcline(852);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(854);
    px_set_global("cg_inited", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(855);
    (void)(px_call(px_get_global("cg_collect_types"), (LXValue[]){_v1423}, 1));
    px_srcline(857);
    _v1425 = px_index(_v1423, px_int(1LL));
    px_srcline(861);
    px_set_global("cg_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(862);
    (void)(px_call(px_get_global("cg_collect_hoist_vars"), (LXValue[]){_v1425, px_get_global("cg_topnames")}, 2));
    px_srcline(863);
    _v1426 = px_int(0LL);
    px_srcline(864);
    while (px_is_truthy(px_lt(_v1426, px_call(px_get_global("len"), (LXValue[]){_v1425}, 1)))) {
        px_srcline(865);
        _v1427 = px_index(_v1425, _v1426);
        px_srcline(866);
        _v1428 = px_index(_v1427, px_int(0LL));
        px_srcline(867);
        if (px_is_truthy(px_eq(_v1428, px_str("FuncDef")))) {
            px_srcline(868);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1427, px_int(1LL))}, 1)}, 1));
        }
        else if (px_is_truthy(px_eq(_v1428, px_str("ExternDef")))) {
            px_srcline(871);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1427, px_int(1LL))}, 1), px_index(_v1427, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v1428, px_str("VarDecl")))) {
            px_srcline(873);
            (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1427, px_int(2LL))}, 1)}, 1));
            px_srcline(874);
            if (px_is_truthy(({ LXValue _t1460 = px_eq(px_index(_v1427, px_int(1LL)), px_str("Let")); px_is_truthy(_t1460) ? _t1460 : px_eq(px_index(_v1427, px_int(1LL)), px_str("Const")); }))) {
                px_srcline(875);
                px_index_set(px_get_global("cg_immutables"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1427, px_int(2LL))}, 1), px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1428, px_str("Assign")))) {
            px_srcline(877);
            _v1429 = px_index(_v1427, px_int(1LL));
            px_srcline(878);
            if (px_is_truthy(px_eq(px_index(_v1429, px_int(0LL)), px_str("Var")))) {
                px_srcline(879);
                (void)(px_method(px_get_global("cg_globals"), "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1429, px_int(1LL))}, 1)}, 1));
            }
        }
        px_srcline(880);
         _v1426 = px_add(_v1426, px_int(1LL));
    }
    px_srcline(882);
    _v1430 = px_list_n((LXValue[]){}, 0);
    px_srcline(883);
    _v1431 = px_method(px_get_global("cg_impls"), "keys", (LXValue[]){}, 0);
    px_srcline(884);
    _v1432 = px_int(0LL);
    px_srcline(885);
    while (px_is_truthy(px_lt(_v1432, px_call(px_get_global("len"), (LXValue[]){_v1431}, 1)))) {
        px_srcline(886);
        _v1433 = px_index(_v1431, _v1432);
        px_srcline(887);
        _v1434 = px_index(px_get_global("cg_impls"), _v1433);
        px_srcline(888);
        _v1435 = px_int(0LL);
        px_srcline(889);
        while (px_is_truthy(px_lt(_v1435, px_call(px_get_global("len"), (LXValue[]){_v1434}, 1)))) {
            px_srcline(890);
            (void)(px_method(_v1430, "append", (LXValue[]){px_list_n((LXValue[]){_v1433, px_index(_v1434, _v1435)}, 2)}, 1));
            px_srcline(891);
             _v1435 = px_add(_v1435, px_int(1LL));
        }
        px_srcline(892);
         _v1432 = px_add(_v1432, px_int(1LL));
    }
    px_srcline(894);
    _v1436 = px_int(1LL);
    px_srcline(895);
    while (px_is_truthy(px_lt(_v1436, px_call(px_get_global("len"), (LXValue[]){_v1430}, 1)))) {
        px_srcline(896);
        _v1437 = _v1436;
        px_srcline(897);
        while (px_is_truthy(px_gt(_v1437, px_int(0LL)))) {
            px_srcline(898);
            _v1438 = px_add(px_add(px_index(px_index(_v1430, px_sub(_v1437, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1430, px_sub(_v1437, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(899);
            _v1439 = px_add(px_add(px_index(px_index(_v1430, _v1437), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1430, _v1437), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(900);
            if (px_is_truthy(px_lt(_v1439, _v1438))) {
                px_srcline(901);
                _v1440 = px_index(_v1430, _v1437);
                px_srcline(902);
                px_index_set(_v1430, _v1437, px_index(_v1430, px_sub(_v1437, px_int(1LL))));
                px_srcline(903);
                px_index_set(_v1430, px_sub(_v1437, px_int(1LL)), _v1440);
            }
            px_srcline(904);
             _v1437 = px_sub(_v1437, px_int(1LL));
        }
        px_srcline(905);
         _v1436 = px_add(_v1436, px_int(1LL));
    }
    px_srcline(910);
    _v1441 = px_list_n((LXValue[]){}, 0);
    px_srcline(911);
    _v1442 = px_int(0LL);
    px_srcline(912);
    while (px_is_truthy(px_lt(_v1442, px_call(px_get_global("len"), (LXValue[]){_v1430}, 1)))) {
        px_srcline(913);
        _v1433 = px_index(px_index(_v1430, _v1442), px_int(0LL));
        px_srcline(914);
        _v1443 = px_index(px_index(_v1430, _v1442), px_int(1LL));
        px_srcline(915);
        _v1444 = ({ LXValue _s201 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1433}, 1)), px_str("_")); LXValue _s202 = px_call(px_get_global("cg_func_cname"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1443, px_int(1LL))}, 1)}, 1); px_add(_s201, _s202); });
        px_srcline(916);
        (void)(px_method(_v1441, "push", (LXValue[]){px_call(px_get_global("cg_gen_func_named"), (LXValue[]){_v1443, _v1444}, 2)}, 1));
        px_srcline(917);
        (void)(px_method(_v1441, "push", (LXValue[]){px_str("\n")}, 1));
        px_srcline(918);
         _v1442 = px_add(_v1442, px_int(1LL));
    }
    px_srcline(920);
    _v1445 = px_int(0LL);
    px_srcline(921);
    while (px_is_truthy(px_lt(_v1445, px_call(px_get_global("len"), (LXValue[]){_v1425}, 1)))) {
        px_srcline(922);
        _v1427 = px_index(_v1425, _v1445);
        px_srcline(923);
        if (px_is_truthy(px_eq(px_index(_v1427, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(924);
            (void)(px_method(_v1441, "push", (LXValue[]){px_call(px_get_global("cg_gen_func"), (LXValue[]){_v1427}, 1)}, 1));
            px_srcline(925);
            (void)(px_method(_v1441, "push", (LXValue[]){px_str("\n")}, 1));
        }
        px_srcline(926);
         _v1445 = px_add(_v1445, px_int(1LL));
    }
    px_srcline(927);
     _v1424 = px_add(_v1424, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1441}, 2));
    px_srcline(929);
     _v1424 = px_add(_v1424, px_str("int main(int argc, char** argv) {\n"));
    px_srcline(930);
     _v1424 = px_add(_v1424, px_str("    px_args_init(argc, argv);\n"));
    px_srcline(931);
     _v1424 = px_add(_v1424, px_str("    px_register_builtins();\n"));
    px_srcline(933);
    _v1446 = px_int(0LL);
    px_srcline(934);
    while (px_is_truthy(px_lt(_v1446, px_call(px_get_global("len"), (LXValue[]){_v1425}, 1)))) {
        px_srcline(935);
        _v1427 = px_index(_v1425, _v1446);
        px_srcline(936);
        if (px_is_truthy(px_eq(px_index(_v1427, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(937);
            _v1444 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1427, px_int(1LL))}, 1);
            px_srcline(938);
            _v1447 = px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1444}, 1));
            px_srcline(939);
             _v1424 = px_add(_v1424, px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1444), px_str("\", px_func(\"")), _v1444), px_str("\", ")), _v1447), px_str(", NULL));\n")));
        }
        px_srcline(940);
         _v1446 = px_add(_v1446, px_int(1LL));
    }
    px_srcline(942);
    _v1448 = px_int(0LL);
    px_srcline(943);
    while (px_is_truthy(px_lt(_v1448, px_call(px_get_global("len"), (LXValue[]){_v1430}, 1)))) {
        px_srcline(944);
        _v1433 = px_index(px_index(_v1430, _v1448), px_int(0LL));
        px_srcline(945);
        _v1443 = px_index(px_index(_v1430, _v1448), px_int(1LL));
        px_srcline(946);
        _v1449 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1443, px_int(1LL))}, 1);
        px_srcline(947);
        _v1444 = ({ LXValue _s203 = px_add(px_add(px_str("fn_"), px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1433}, 1)), px_str("_")); LXValue _s204 = px_call(px_get_global("cg_func_cname"), (LXValue[]){_v1449}, 1); px_add(_s203, _s204); });
        px_srcline(948);
         _v1424 = px_add(_v1424, px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_add(px_str("    px_set_global(\""), _v1433), px_str(".")), _v1449), px_str("\", px_func(\"")), _v1433), px_str(".")), _v1449), px_str("\", ")), _v1444), px_str(", NULL));\n")));
        px_srcline(949);
         _v1448 = px_add(_v1448, px_int(1LL));
    }
    px_srcline(952);
    _v1450 = px_list_n((LXValue[]){}, 0);
    px_srcline(954);
    px_set_global("cg_topbody", px_bool(true));
    px_srcline(955);
    _v1451 = px_int(0LL);
    px_srcline(956);
    while (px_is_truthy(px_lt(_v1451, px_call(px_get_global("len"), (LXValue[]){_v1425}, 1)))) {
        px_srcline(957);
        _v1427 = px_index(_v1425, _v1451);
        px_srcline(958);
        _v1428 = px_index(_v1427, px_int(0LL));
        px_srcline(959);
        if (px_is_truthy(({ LXValue _t1466 = ({ LXValue _t1465 = ({ LXValue _t1464 = ({ LXValue _t1463 = ({ LXValue _t1462 = ({ LXValue _t1461 = px_ne(_v1428, px_str("FuncDef")); px_is_truthy(_t1461) ? px_ne(_v1428, px_str("StructDef")) : _t1461; }); px_is_truthy(_t1462) ? px_ne(_v1428, px_str("EnumDef")) : _t1462; }); px_is_truthy(_t1463) ? px_ne(_v1428, px_str("TraitDef")) : _t1463; }); px_is_truthy(_t1464) ? px_ne(_v1428, px_str("ImplDef")) : _t1464; }); px_is_truthy(_t1465) ? px_ne(_v1428, px_str("Import")) : _t1465; }); px_is_truthy(_t1466) ? px_ne(_v1428, px_str("ExternDef")) : _t1466; }))) {
            px_srcline(960);
            (void)(px_method(_v1450, "push", (LXValue[]){px_call(px_get_global("cg_gen_stmt"), (LXValue[]){_v1427, px_int(1LL)}, 2)}, 1));
        }
        px_srcline(961);
         _v1451 = px_add(_v1451, px_int(1LL));
    }
    px_srcline(962);
    px_set_global("cg_topbody", px_bool(false));
    px_srcline(963);
     _v1424 = px_add(_v1424, px_call(px_get_global("join"), (LXValue[]){px_str(""), _v1450}, 2));
    px_srcline(965);
    _v1452 = px_bool(false);
    px_srcline(966);
    _v1453 = px_int(0LL);
    px_srcline(967);
    while (px_is_truthy(px_lt(_v1453, px_call(px_get_global("len"), (LXValue[]){_v1425}, 1)))) {
        px_srcline(968);
        _v1427 = px_index(_v1425, _v1453);
        px_srcline(969);
        if (px_is_truthy(({ LXValue _t1467 = px_eq(px_index(_v1427, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t1467) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1427, px_int(1LL))}, 1), px_str("main")) : _t1467; }))) {
            px_srcline(970);
             _v1452 = px_bool(true);
            px_srcline(971);
            break;
        }
        px_srcline(972);
         _v1453 = px_add(_v1453, px_int(1LL));
    }
    px_srcline(973);
    if (px_is_truthy(_v1452)) {
        px_srcline(974);
        _v1447 = px_str("fn_main");
        px_srcline(975);
         _v1424 = px_add(_v1424, px_add(px_add(px_str("    { LXValue _r = "), _v1447), px_str("(NULL, 0, NULL); int _code = 0;\n")));
        px_srcline(976);
         _v1424 = px_add(_v1424, px_str("      if (px_is_result(_r)) {\n"));
        px_srcline(977);
         _v1424 = px_add(_v1424, px_str("        if (!px_result_ok(_r)) {\n"));
        px_srcline(978);
         _v1424 = px_add(_v1424, px_str("          fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n"));
        px_srcline(979);
         _v1424 = px_add(_v1424, px_str("          _code = 1;\n"));
        px_srcline(980);
         _v1424 = px_add(_v1424, px_str("        } else {\n"));
        px_srcline(981);
         _v1424 = px_add(_v1424, px_str("          LXValue _uv = px_result_unwrap(_r);\n"));
        px_srcline(982);
         _v1424 = px_add(_v1424, px_str("          if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n"));
        px_srcline(983);
         _v1424 = px_add(_v1424, px_str("        }\n"));
        px_srcline(984);
         _v1424 = px_add(_v1424, px_str("      } else if (_r.type == PX_INT) {\n"));
        px_srcline(985);
         _v1424 = px_add(_v1424, px_str("        _code = (int)_r.as.i;\n"));
        px_srcline(986);
         _v1424 = px_add(_v1424, px_str("      }\n"));
        px_srcline(987);
         _v1424 = px_add(_v1424, px_str("      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）\n"));
        px_srcline(988);
         _v1424 = px_add(_v1424, px_str("    }\n"));
    }
    else {
        px_srcline(990);
         _v1424 = px_add(_v1424, px_str("    return px_exit_code_final(0);   // M120（qg-issue 76 E1）\n"));
    }
    px_srcline(991);
     _v1424 = px_add(_v1424, px_str("}\n"));
    px_srcline(993);
    _v1454 = px_call(px_get_global("cg_find"), (LXValue[]){_v1424, px_str("int main(")}, 2);
    px_srcline(994);
    if (px_is_truthy(px_ge(_v1454, px_int(0LL)))) {
        px_srcline(995);
        _v1455 = px_slice(_v1424, px_int(0LL), _v1454, px_null());
        px_srcline(996);
        _v1456 = px_slice(_v1424, _v1454, px_call(px_get_global("len"), (LXValue[]){_v1424}, 1), px_null());
        px_srcline(997);
        _v1457 = px_call(px_get_global("cg_find"), (LXValue[]){_v1455, px_str("static LXValue")}, 2);
        px_srcline(998);
        _v1458 = px_str("");
        px_srcline(999);
        if (px_is_truthy(px_ge(_v1457, px_int(0LL)))) {
            px_srcline(1000);
             _v1458 = px_add(px_add(px_add(px_add(px_slice(_v1455, px_int(0LL), _v1457, px_null()), px_get_global("cg_closures")), px_str("\n")), px_slice(_v1455, _v1457, px_call(px_get_global("len"), (LXValue[]){_v1455}, 1), px_null())), _v1456);
        }
        else {
            px_srcline(1002);
             _v1458 = px_add(px_add(px_add(_v1455, px_get_global("cg_closures")), px_str("\n")), _v1456);
        }
        px_srcline(1003);
        return _v1458;
    }
    px_srcline(1004);
    return _v1424;
px_err_1459:
    if (px_err_1459_proped) return px_err_1459_val;
    return px_null();
}

static LXValue fn_bc_new_dict(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_dict");
    LXValue _v1468 = px_uninit();
    LXValue px_err_1469_val = px_null();
    int px_err_1469_proped = 0;
    px_srcline(41);
    _v1468 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(42);
    (void)(px_method(_v1468, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(43);
    return _v1468;
px_err_1469:
    if (px_err_1469_proped) return px_err_1469_val;
    return px_null();
}

static LXValue fn_bc_k_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_find");
    LXValue _v1470 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1471 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1472 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1473 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1474 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1475 = px_uninit();
    LXValue _v1476 = px_uninit();
    LXValue px_err_1477_val = px_null();
    int px_err_1477_proped = 0;
    px_srcline(47);
    _v1475 = px_int(0LL);
    px_srcline(48);
    while (px_is_truthy(px_lt(_v1475, px_call(px_get_global("len"), (LXValue[]){_v1470}, 1)))) {
        px_srcline(49);
        _v1476 = px_index(_v1470, _v1475);
        px_srcline(50);
        if (px_is_truthy(px_eq(px_index(_v1476, px_str("kind")), _v1471))) {
            px_srcline(51);
            if (px_is_truthy(px_eq(_v1471, px_str("int")))) {
                px_srcline(52);
                if (px_is_truthy(px_eq(px_index(_v1476, px_str("i")), _v1472))) {
                    px_srcline(53);
                    return _v1475;
                }
            }
            else if (px_is_truthy(px_eq(_v1471, px_str("float")))) {
                px_srcline(59);
                if (px_is_truthy(({ LXValue _s205 = px_call(px_get_global("float64_bits"), (LXValue[]){px_index(_v1476, px_str("f"))}, 1); LXValue _s206 = px_call(px_get_global("float64_bits"), (LXValue[]){_v1473}, 1); px_eq(_s205, _s206); }))) {
                    px_srcline(60);
                    return _v1475;
                }
            }
            else if (px_is_truthy(px_eq(_v1471, px_str("str")))) {
                px_srcline(62);
                if (px_is_truthy(px_eq(px_index(_v1476, px_str("s")), _v1474))) {
                    px_srcline(63);
                    return _v1475;
                }
            }
            else {
                px_srcline(66);
                if (px_is_truthy(px_eq(px_index(_v1476, px_str("i")), _v1472))) {
                    px_srcline(67);
                    return _v1475;
                }
            }
        }
        px_srcline(68);
         _v1475 = px_add(_v1475, px_int(1LL));
    }
    px_srcline(69);
    return px_neg(px_int(1LL));
px_err_1477:
    if (px_err_1477_proped) return px_err_1477_val;
    return px_null();
}

static LXValue fn_bc_k_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_k_add");
    LXValue _v1478 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1479 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1480 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1481 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1482 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1483 = px_uninit();
    LXValue _v1484 = px_uninit();
    LXValue px_err_1485_val = px_null();
    int px_err_1485_proped = 0;
    px_srcline(71);
    _v1483 = px_call(px_get_global("bc_k_find"), (LXValue[]){_v1478, _v1479, _v1480, _v1481, _v1482}, 5);
    px_srcline(72);
    if (px_is_truthy(px_ge(_v1483, px_int(0LL)))) {
        px_srcline(73);
        return _v1483;
    }
    px_srcline(74);
    _v1484 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(75);
    px_index_set(_v1484, px_str("kind"), _v1479);
    px_srcline(76);
    px_index_set(_v1484, px_str("i"), _v1480);
    px_srcline(77);
    px_index_set(_v1484, px_str("f"), _v1481);
    px_srcline(78);
    px_index_set(_v1484, px_str("s"), _v1482);
    px_srcline(79);
    (void)(px_method(_v1478, "push", (LXValue[]){_v1484}, 1));
    px_srcline(80);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1478}, 1), px_int(1LL));
px_err_1485:
    if (px_err_1485_proped) return px_err_1485_val;
    return px_null();
}

static LXValue fn_bc_n_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_n_add");
    LXValue _v1486 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1487 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1488 = px_uninit();
    LXValue px_err_1489_val = px_null();
    int px_err_1489_proped = 0;
    px_srcline(83);
    _v1488 = px_int(0LL);
    px_srcline(84);
    while (px_is_truthy(px_lt(_v1488, px_call(px_get_global("len"), (LXValue[]){_v1486}, 1)))) {
        px_srcline(85);
        if (px_is_truthy(px_eq(px_index(_v1486, _v1488), _v1487))) {
            px_srcline(86);
            return _v1488;
        }
        px_srcline(87);
         _v1488 = px_add(_v1488, px_int(1LL));
    }
    px_srcline(88);
    (void)(px_method(_v1486, "push", (LXValue[]){_v1487}, 1));
    px_srcline(89);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1486}, 1), px_int(1LL));
px_err_1489:
    if (px_err_1489_proped) return px_err_1489_val;
    return px_null();
}

static LXValue fn_bc_g_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_g_add");
    LXValue _v1490 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1491 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1492 = px_uninit();
    LXValue px_err_1493_val = px_null();
    int px_err_1493_proped = 0;
    px_srcline(92);
    _v1492 = px_int(0LL);
    px_srcline(93);
    while (px_is_truthy(px_lt(_v1492, px_call(px_get_global("len"), (LXValue[]){_v1490}, 1)))) {
        px_srcline(94);
        if (px_is_truthy(px_eq(px_index(_v1490, _v1492), _v1491))) {
            px_srcline(95);
            return _v1492;
        }
        px_srcline(96);
         _v1492 = px_add(_v1492, px_int(1LL));
    }
    px_srcline(97);
    (void)(px_method(_v1490, "push", (LXValue[]){_v1491}, 1));
    px_srcline(98);
    return px_sub(px_call(px_get_global("len"), (LXValue[]){_v1490}, 1), px_int(1LL));
px_err_1493:
    if (px_err_1493_proped) return px_err_1493_val;
    return px_null();
}

static LXValue fn_bc_is_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_is_global");
    LXValue _v1494 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1495_val = px_null();
    int px_err_1495_proped = 0;
    px_srcline(101);
    if (px_is_truthy(px_eq(px_get_global("g_bcm"), px_null()))) {
        px_srcline(102);
        return px_bool(false);
    }
    px_srcline(103);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1494}, 2);
px_err_1495:
    if (px_err_1495_proped) return px_err_1495_val;
    return px_null();
}

static LXValue fn_bc_new_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_module");
    LXValue _v1496 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1497 = px_uninit();
    LXValue px_err_1498_val = px_null();
    int px_err_1498_proped = 0;
    px_srcline(106);
    _v1497 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(107);
    px_index_set(_v1497, px_str("name"), _v1496);
    px_srcline(108);
    px_index_set(_v1497, px_str("k_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(109);
    px_index_set(_v1497, px_str("n_pool"), px_list_n((LXValue[]){}, 0));
    px_srcline(110);
    px_index_set(_v1497, px_str("globals"), px_list_n((LXValue[]){}, 0));
    px_srcline(111);
    px_index_set(_v1497, px_str("funcs"), px_list_n((LXValue[]){}, 0));
    px_srcline(112);
    px_index_set(_v1497, px_str("top"), px_neg(px_int(1LL)));
    px_srcline(115);
    px_index_set(_v1497, px_str("structs"), px_list_n((LXValue[]){}, 0));
    px_srcline(116);
    px_index_set(_v1497, px_str("enums"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(117);
    px_index_set(_v1497, px_str("consts"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(118);
    px_index_set(_v1497, px_str("nclosure"), px_int(0LL));
    px_srcline(119);
    return _v1497;
px_err_1498:
    if (px_err_1498_proped) return px_err_1498_val;
    return px_null();
}

static LXValue fn_bc_struct_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_struct_index");
    LXValue _v1499 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1500 = px_uninit();
    LXValue px_err_1501_val = px_null();
    int px_err_1501_proped = 0;
    px_srcline(122);
    _v1500 = px_int(0LL);
    px_srcline(123);
    while (px_is_truthy(px_lt(_v1500, px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("structs"))}, 1)))) {
        px_srcline(124);
        if (px_is_truthy(px_eq(px_index(px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1500), px_str("name")), _v1499))) {
            px_srcline(125);
            return _v1500;
        }
        px_srcline(126);
         _v1500 = px_add(_v1500, px_int(1LL));
    }
    px_srcline(127);
    return px_neg(px_int(1LL));
px_err_1501:
    if (px_err_1501_proped) return px_err_1501_val;
    return px_null();
}

static LXValue fn_bc_enum_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_enum_has");
    LXValue _v1502 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1503 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1504_val = px_null();
    int px_err_1504_proped = 0;
    px_srcline(130);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1502}, 1)))) {
        px_srcline(131);
        return px_bool(false);
    }
    px_srcline(132);
    return px_call(px_get_global("contains"), (LXValue[]){px_index(px_index(px_get_global("g_bcm"), px_str("enums")), _v1502), _v1503}, 2);
px_err_1504:
    if (px_err_1504_proped) return px_err_1504_val;
    return px_null();
}

static LXValue fn_bc_const_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_const_find");
    LXValue _v1505 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1506 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1507 = px_uninit();
    LXValue _v1508 = px_uninit();
    LXValue px_err_1509_val = px_null();
    int px_err_1509_proped = 0;
    px_srcline(135);
    if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1505}, 1)))) {
        px_srcline(136);
        return px_null();
    }
    px_srcline(137);
    _v1507 = px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1505);
    px_srcline(138);
    _v1508 = px_int(0LL);
    px_srcline(139);
    while (px_is_truthy(px_lt(_v1508, px_call(px_get_global("len"), (LXValue[]){_v1507}, 1)))) {
        px_srcline(140);
        if (px_is_truthy(px_eq(px_index(px_index(_v1507, _v1508), px_str("n")), _v1506))) {
            px_srcline(141);
            return px_index(px_index(_v1507, _v1508), px_str("e"));
        }
        px_srcline(142);
         _v1508 = px_add(_v1508, px_int(1LL));
    }
    px_srcline(143);
    return px_null();
px_err_1509:
    if (px_err_1509_proped) return px_err_1509_val;
    return px_null();
}

static LXValue fn_bc_collect_consts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_consts");
    LXValue _v1510 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1511 = px_uninit();
    LXValue _v1512 = px_uninit();
    LXValue _v1513 = px_uninit();
    LXValue _v1514 = px_uninit();
    LXValue _v1515 = px_uninit();
    LXValue _v1516 = px_uninit();
    LXValue _v1517 = px_uninit();
    LXValue _v1518 = px_uninit();
    LXValue _v1519 = px_uninit();
    LXValue _v1520 = px_uninit();
    LXValue px_err_1521_val = px_null();
    int px_err_1521_proped = 0;
    px_srcline(146);
    _v1511 = px_int(0LL);
    px_srcline(147);
    while (px_is_truthy(px_lt(_v1511, px_call(px_get_global("len"), (LXValue[]){_v1510}, 1)))) {
        px_srcline(148);
        _v1512 = px_index(_v1510, _v1511);
        px_srcline(149);
        _v1513 = px_index(_v1512, px_int(0LL));
        px_srcline(150);
        if (px_is_truthy(px_eq(_v1513, px_str("TypeConst")))) {
            px_srcline(151);
            _v1514 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1512, px_int(1LL))}, 1);
            px_srcline(152);
            if (px_is_truthy(px_not(px_method(px_index(px_get_global("g_bcm"), px_str("consts")), "has", (LXValue[]){_v1514}, 1)))) {
                px_srcline(153);
                px_index_set(px_index(px_get_global("g_bcm"), px_str("consts")), _v1514, px_list_n((LXValue[]){}, 0));
            }
            px_srcline(154);
            _v1515 = px_int(0LL);
            px_srcline(155);
            while (px_is_truthy(px_lt(_v1515, px_call(px_get_global("len"), (LXValue[]){px_index(_v1512, px_int(2LL))}, 1)))) {
                px_srcline(156);
                _v1516 = px_index(px_index(_v1512, px_int(2LL)), _v1515);
                px_srcline(157);
                _v1517 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
                px_srcline(158);
                px_index_set(_v1517, px_str("n"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1516, px_int(1LL))}, 1));
                px_srcline(159);
                px_index_set(_v1517, px_str("e"), px_index(_v1516, px_int(2LL)));
                px_srcline(160);
                (void)(px_method(px_index(px_index(px_get_global("g_bcm"), px_str("consts")), _v1514), "push", (LXValue[]){_v1517}, 1));
                px_srcline(161);
                 _v1515 = px_add(_v1515, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1513, px_str("FuncDef")))) {
            px_srcline(163);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1512, px_int(4LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1513, px_str("ImplDef")))) {
            px_srcline(165);
            _v1518 = px_int(0LL);
            px_srcline(166);
            while (px_is_truthy(px_lt(_v1518, px_call(px_get_global("len"), (LXValue[]){px_index(_v1512, px_int(3LL))}, 1)))) {
                px_srcline(167);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(px_index(_v1512, px_int(3LL)), _v1518), px_int(4LL))}, 1));
                px_srcline(168);
                 _v1518 = px_add(_v1518, px_int(1LL));
            }
        }
        else if (px_is_truthy(px_eq(_v1513, px_str("If")))) {
            px_srcline(170);
            _v1519 = px_index(_v1512, px_int(1LL));
            px_srcline(171);
            _v1520 = px_int(0LL);
            px_srcline(172);
            while (px_is_truthy(px_lt(_v1520, px_call(px_get_global("len"), (LXValue[]){_v1519}, 1)))) {
                px_srcline(173);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(px_index(_v1519, _v1520), px_int(1LL))}, 1));
                px_srcline(174);
                 _v1520 = px_add(_v1520, px_int(1LL));
            }
            px_srcline(175);
            if (px_is_truthy(px_ne(px_index(_v1512, px_int(2LL)), px_null()))) {
                px_srcline(176);
                (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1512, px_int(2LL))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1513, px_str("While")))) {
            px_srcline(178);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1512, px_int(2LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v1513, px_str("For")))) {
            px_srcline(180);
            (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){px_index(_v1512, px_int(3LL))}, 1));
        }
        px_srcline(181);
         _v1511 = px_add(_v1511, px_int(1LL));
    }
px_err_1521:
    if (px_err_1521_proped) return px_err_1521_val;
    return px_null();
}

static LXValue fn_bc_collect_impl_list(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_impl_list");
    LXValue _v1522 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1523 = px_uninit();
    LXValue _v1524 = px_uninit();
    LXValue _v1525 = px_uninit();
    LXValue _v1526 = px_uninit();
    LXValue _v1527 = px_uninit();
    LXValue _v1528 = px_uninit();
    LXValue _v1529 = px_uninit();
    LXValue _v1530 = px_uninit();
    LXValue _v1531 = px_uninit();
    LXValue _v1532 = px_uninit();
    LXValue px_err_1533_val = px_null();
    int px_err_1533_proped = 0;
    px_srcline(185);
    _v1523 = px_list_n((LXValue[]){}, 0);
    px_srcline(186);
    _v1524 = px_int(0LL);
    px_srcline(187);
    while (px_is_truthy(px_lt(_v1524, px_call(px_get_global("len"), (LXValue[]){_v1522}, 1)))) {
        px_srcline(188);
        _v1525 = px_index(_v1522, _v1524);
        px_srcline(189);
        if (px_is_truthy(px_eq(px_index(_v1525, px_int(0LL)), px_str("ImplDef")))) {
            px_srcline(190);
            _v1526 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1525, px_int(1LL))}, 1);
            px_srcline(191);
            _v1527 = px_int(0LL);
            px_srcline(192);
            while (px_is_truthy(px_lt(_v1527, px_call(px_get_global("len"), (LXValue[]){px_index(_v1525, px_int(3LL))}, 1)))) {
                px_srcline(193);
                (void)(px_method(_v1523, "append", (LXValue[]){px_list_n((LXValue[]){_v1526, px_index(px_index(_v1525, px_int(3LL)), _v1527)}, 2)}, 1));
                px_srcline(194);
                 _v1527 = px_add(_v1527, px_int(1LL));
            }
        }
        px_srcline(195);
         _v1524 = px_add(_v1524, px_int(1LL));
    }
    px_srcline(197);
    _v1528 = px_int(1LL);
    px_srcline(198);
    while (px_is_truthy(px_lt(_v1528, px_call(px_get_global("len"), (LXValue[]){_v1523}, 1)))) {
        px_srcline(199);
        _v1529 = _v1528;
        px_srcline(200);
        while (px_is_truthy(px_gt(_v1529, px_int(0LL)))) {
            px_srcline(201);
            _v1530 = px_add(px_add(px_index(px_index(_v1523, px_sub(_v1529, px_int(1LL))), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1523, px_sub(_v1529, px_int(1LL))), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(202);
            _v1531 = px_add(px_add(px_index(px_index(_v1523, _v1529), px_int(0LL)), px_str(".")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1523, _v1529), px_int(1LL)), px_int(1LL))}, 1));
            px_srcline(203);
            if (px_is_truthy(px_lt(_v1531, _v1530))) {
                px_srcline(204);
                _v1532 = px_index(_v1523, _v1529);
                px_srcline(205);
                px_index_set(_v1523, _v1529, px_index(_v1523, px_sub(_v1529, px_int(1LL))));
                px_srcline(206);
                px_index_set(_v1523, px_sub(_v1529, px_int(1LL)), _v1532);
            }
            px_srcline(207);
             _v1529 = px_sub(_v1529, px_int(1LL));
        }
        px_srcline(208);
         _v1528 = px_add(_v1528, px_int(1LL));
    }
    px_srcline(209);
    return _v1523;
px_err_1533:
    if (px_err_1533_proped) return px_err_1533_val;
    return px_null();
}

static LXValue fn_bc_new_func(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_new_func");
    LXValue _v1534 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1535 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1536 = px_uninit();
    LXValue px_err_1537_val = px_null();
    int px_err_1537_proped = 0;
    px_srcline(214);
    _v1536 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(215);
    px_index_set(_v1536, px_str("name"), _v1534);
    px_srcline(216);
    px_index_set(_v1536, px_str("arity"), _v1535);
    px_srcline(217);
    px_index_set(_v1536, px_str("ndefault"), px_int(0LL));
    px_srcline(218);
    px_index_set(_v1536, px_str("nslots"), _v1535);
    px_srcline(219);
    px_index_set(_v1536, px_str("bc"), px_list_n((LXValue[]){}, 0));
    px_srcline(220);
    px_index_set(_v1536, px_str("smap"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(221);
    px_index_set(_v1536, px_str("next_slot"), _v1535);
    px_srcline(222);
    px_index_set(_v1536, px_str("is_top"), px_bool(false));
    px_srcline(223);
    px_index_set(_v1536, px_str("loops"), px_list_n((LXValue[]){}, 0));
    px_srcline(229);
    px_index_set(_v1536, px_str("upnames"), px_list_n((LXValue[]){}, 0));
    px_srcline(230);
    px_index_set(_v1536, px_str("cell"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(231);
    px_index_set(_v1536, px_str("boxed"), px_list_n((LXValue[]){}, 0));
    px_srcline(236);
    px_index_set(_v1536, px_str("inited"), px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0));
    px_srcline(238);
    px_index_set(_v1536, px_str("hoist_slots"), px_list_n((LXValue[]){}, 0));
    px_srcline(239);
    return _v1536;
px_err_1537:
    if (px_err_1537_proped) return px_err_1537_val;
    return px_null();
}

static LXValue fn_bc_inited_copy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_copy");
    LXValue _v1538 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1539_val = px_null();
    int px_err_1539_proped = 0;
    px_srcline(244);
    return px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_index(_v1538, px_str("inited"))}, 1);
px_err_1539:
    if (px_err_1539_proped) return px_err_1539_val;
    return px_null();
}

static LXValue fn_bc_inited_restore(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_restore");
    LXValue _v1540 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1541 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1542_val = px_null();
    int px_err_1542_proped = 0;
    px_srcline(246);
    px_index_set(_v1540, px_str("inited"), px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v1541}, 1));
px_err_1542:
    if (px_err_1542_proped) return px_err_1542_val;
    return px_null();
}

static LXValue fn_bc_inited_intersect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_intersect");
    LXValue _v1543 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1544 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1545 = px_uninit();
    LXValue _v1546 = px_uninit();
    LXValue _v1547 = px_uninit();
    LXValue px_err_1548_val = px_null();
    int px_err_1548_proped = 0;
    px_srcline(248);
    _v1545 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(249);
    _v1546 = px_method(_v1543, "keys", (LXValue[]){}, 0);
    px_srcline(250);
    _v1547 = px_int(0LL);
    px_srcline(251);
    while (px_is_truthy(px_lt(_v1547, px_call(px_get_global("len"), (LXValue[]){_v1546}, 1)))) {
        px_srcline(252);
        if (px_is_truthy(px_method(_v1544, "has", (LXValue[]){px_index(_v1546, _v1547)}, 1))) {
            px_srcline(253);
            px_index_set(_v1545, px_index(_v1546, _v1547), px_int(1LL));
        }
        px_srcline(254);
         _v1547 = px_add(_v1547, px_int(1LL));
    }
    px_srcline(255);
    return _v1545;
px_err_1548:
    if (px_err_1548_proped) return px_err_1548_val;
    return px_null();
}

static LXValue fn_bc_inited_mark(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_inited_mark");
    LXValue _v1549 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1550 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1551_val = px_null();
    int px_err_1551_proped = 0;
    px_srcline(257);
    px_index_set(px_index(_v1549, px_str("inited")), _v1550, px_int(1LL));
px_err_1551:
    if (px_err_1551_proped) return px_err_1551_val;
    return px_null();
}

static LXValue fn_bc_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_slot");
    LXValue _v1552 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1553 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1554 = px_uninit();
    LXValue px_err_1555_val = px_null();
    int px_err_1555_proped = 0;
    px_srcline(260);
    if (px_is_truthy(px_method(px_index(_v1552, px_str("smap")), "has", (LXValue[]){_v1553}, 1))) {
        px_srcline(261);
        return px_index(px_index(_v1552, px_str("smap")), _v1553);
    }
    px_srcline(262);
    _v1554 = px_index(_v1552, px_str("next_slot"));
    px_srcline(263);
    px_index_set(_v1552, px_str("next_slot"), px_add(_v1554, px_int(1LL)));
    px_srcline(264);
    px_index_set(px_index(_v1552, px_str("smap")), _v1553, _v1554);
    px_srcline(265);
    return _v1554;
px_err_1555:
    if (px_err_1555_proped) return px_err_1555_val;
    return px_null();
}

static LXValue fn_bc_tmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_tmp");
    LXValue _v1556 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1557 = px_uninit();
    LXValue px_err_1558_val = px_null();
    int px_err_1558_proped = 0;
    px_srcline(268);
    _v1557 = px_index(_v1556, px_str("next_slot"));
    px_srcline(269);
    px_index_set(_v1556, px_str("next_slot"), px_add(_v1557, px_int(1LL)));
    px_srcline(270);
    return _v1557;
px_err_1558:
    if (px_err_1558_proped) return px_err_1558_val;
    return px_null();
}

static LXValue fn_bc_cell_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_has");
    LXValue _v1559 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1560 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1561_val = px_null();
    int px_err_1561_proped = 0;
    px_srcline(275);
    return px_method(px_index(_v1559, px_str("cell")), "has", (LXValue[]){_v1560}, 1);
px_err_1561:
    if (px_err_1561_proped) return px_err_1561_val;
    return px_null();
}

static LXValue fn_bc_cell_mark(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_cell_mark");
    LXValue _v1562 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1563 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1564_val = px_null();
    int px_err_1564_proped = 0;
    px_srcline(277);
    px_index_set(px_index(_v1562, px_str("cell")), _v1563, px_int(1LL));
px_err_1564:
    if (px_err_1564_proped) return px_err_1564_val;
    return px_null();
}

static LXValue fn_bc_load_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_load_var");
    LXValue _v1565 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1566 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1567 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1568 = px_uninit();
    LXValue _v1569 = px_uninit();
    LXValue px_err_1570_val = px_null();
    int px_err_1570_proped = 0;
    px_srcline(280);
    _v1568 = px_index(px_index(_v1565, px_str("smap")), _v1566);
    px_srcline(281);
    if (px_is_truthy(px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1565, _v1566}, 2)))) {
        px_srcline(282);
        if (px_is_truthy(({ LXValue _t1571 = px_ge(_v1567, px_int(0LL)); px_is_truthy(_t1571) ? px_ne(_v1567, _v1568) : _t1571; }))) {
            px_srcline(283);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1565, px_str("MOV"), _v1567, _v1568, px_int(0LL)}, 5));
        }
        px_srcline(284);
        return px_null();
    }
    px_srcline(285);
    if (px_is_truthy(px_lt(_v1567, px_int(0LL)))) {
        px_srcline(286);
        return px_null();
    }
    px_srcline(287);
    if (px_is_truthy(px_eq(_v1567, _v1568))) {
        px_srcline(290);
        _v1569 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1565}, 1);
        px_srcline(291);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1565, px_str("CELLGET"), _v1569, _v1568, px_int(0LL)}, 5));
        px_srcline(292);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1565, px_str("MOV"), _v1568, _v1569, px_int(0LL)}, 5));
        px_srcline(293);
        return px_null();
    }
    px_srcline(294);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1565, px_str("CELLGET"), _v1567, _v1568, px_int(0LL)}, 5));
px_err_1570:
    if (px_err_1570_proped) return px_err_1570_val;
    return px_null();
}

static LXValue fn_bc_load_ck(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_load_ck");
    LXValue _v1572 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1573 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1574 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1575 = px_uninit();
    LXValue _v1576 = px_uninit();
    LXValue px_err_1577_val = px_null();
    int px_err_1577_proped = 0;
    px_srcline(299);
    _v1575 = _v1574;
    px_srcline(300);
    if (px_is_truthy(px_lt(_v1575, px_int(0LL)))) {
        px_srcline(302);
         _v1575 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1572}, 1);
    }
    px_srcline(303);
    (void)(px_call(px_get_global("bc_load_var"), (LXValue[]){_v1572, _v1573, _v1575}, 3));
    px_srcline(304);
    if (px_is_truthy(px_not(px_method(px_index(_v1572, px_str("inited")), "has", (LXValue[]){_v1573}, 1)))) {
        px_srcline(305);
        _v1576 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1573}, 2);
        px_srcline(306);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1572, px_str("CHKINIT"), _v1575, px_int(0LL), _v1576}, 5));
    }
    px_srcline(307);
    return _v1575;
px_err_1577:
    if (px_err_1577_proped) return px_err_1577_val;
    return px_null();
}

static LXValue fn_bc_store_var(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_store_var");
    LXValue _v1578 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1579 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1580 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1581 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_1582_val = px_null();
    int px_err_1582_proped = 0;
    px_srcline(310);
    if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1578, _v1579}, 2))) {
        px_srcline(311);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1578, px_str("CELLSET"), _v1581, _v1580, px_int(0LL)}, 5));
    }
    else {
        px_srcline(313);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1578, px_str("MOV"), _v1580, _v1581, px_int(0LL)}, 5));
    }
px_err_1582:
    if (px_err_1582_proped) return px_err_1582_val;
    return px_null();
}

static LXValue fn_bc_closure_free(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_closure_free");
    LXValue _v1583 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1584 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1585 = px_uninit();
    LXValue px_err_1586_val = px_null();
    int px_err_1586_proped = 0;
    px_srcline(317);
    _v1585 = px_list_n((LXValue[]){}, 0);
    px_srcline(318);
    (void)(px_call(px_get_global("cg_closure_caps"), (LXValue[]){px_list_n((LXValue[]){px_str("Closure"), _v1583, px_null(), _v1584, px_list_n((LXValue[]){}, 0), px_null()}, 6), _v1585}, 2));
    px_srcline(319);
    return _v1585;
px_err_1586:
    if (px_err_1586_proped) return px_err_1586_val;
    return px_null();
}

static LXValue fn_bc_lambda_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_hoist");
    LXValue _v1587 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1588 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1589_val = px_null();
    int px_err_1589_proped = 0;
    px_srcline(322);
    if (px_is_truthy(({ LXValue _t1591 = ({ LXValue _t1590 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1587}, 1), px_str("list")); px_is_truthy(_t1590) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1587}, 1), px_int(0LL)) : _t1590; }); px_is_truthy(_t1591) ? px_eq(px_index(_v1587, px_int(0LL)), px_str("Block")) : _t1591; }))) {
        px_srcline(323);
        (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1587, px_int(1LL)), _v1588}, 2));
    }
px_err_1589:
    if (px_err_1589_proped) return px_err_1589_val;
    return px_null();
}

static LXValue fn_bc_lambda_decl(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_lambda_decl");
    LXValue _v1592 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1593 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_1594_val = px_null();
    int px_err_1594_proped = 0;
    px_srcline(326);
    if (px_is_truthy(({ LXValue _t1596 = ({ LXValue _t1595 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v1592}, 1), px_str("list")); px_is_truthy(_t1595) ? px_gt(px_call(px_get_global("len"), (LXValue[]){_v1592}, 1), px_int(0LL)) : _t1595; }); px_is_truthy(_t1596) ? px_eq(px_index(_v1592, px_int(0LL)), px_str("Block")) : _t1596; }))) {
        px_srcline(327);
        (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1592, px_int(1LL)), _v1593}, 2));
    }
px_err_1594:
    if (px_err_1594_proped) return px_err_1594_val;
    return px_null();
}

static LXValue fn_bc_box_frame(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_box_frame");
    LXValue _v1597 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1598 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1599 = px_uninit();
    LXValue _v1600 = px_uninit();
    LXValue _v1601 = px_uninit();
    LXValue px_err_1602_val = px_null();
    int px_err_1602_proped = 0;
    px_srcline(333);
    _v1599 = px_list_n((LXValue[]){}, 0);
    px_srcline(334);
    (void)(px_call(px_get_global("cg_scan_closure_caps"), (LXValue[]){_v1598, _v1599}, 2));
    px_srcline(335);
    _v1600 = px_int(0LL);
    px_srcline(336);
    while (px_is_truthy(px_lt(_v1600, px_call(px_get_global("len"), (LXValue[]){_v1599}, 1)))) {
        px_srcline(337);
        _v1601 = px_index(_v1599, _v1600);
        px_srcline(338);
        if (px_is_truthy(({ LXValue _t1603 = px_method(px_index(_v1597, px_str("smap")), "has", (LXValue[]){_v1601}, 1); px_is_truthy(_t1603) ? px_not(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1597, _v1601}, 2)) : _t1603; }))) {
            px_srcline(339);
            (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1597, _v1601}, 2));
            px_srcline(340);
            (void)(px_method(px_index(_v1597, px_str("boxed")), "push", (LXValue[]){_v1601}, 1));
            px_srcline(341);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1597, px_str("CELLNEW"), px_index(px_index(_v1597, px_str("smap")), _v1601), px_index(px_index(_v1597, px_str("smap")), _v1601), px_int(0LL)}, 5));
        }
        px_srcline(342);
         _v1600 = px_add(_v1600, px_int(1LL));
    }
px_err_1602:
    if (px_err_1602_proped) return px_err_1602_val;
    return px_null();
}

static LXValue fn_bc_emit_inst(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_inst");
    LXValue _v1604 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1605 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1606 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1607 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1608 = (nargs > 4) ? args[4] : px_null();
    LXValue px_err_1609_val = px_null();
    int px_err_1609_proped = 0;
    px_srcline(348);
    (void)(px_method(px_index(_v1604, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){_v1605, px_int(0LL), _v1606, _v1607, _v1608}, 5)}, 1));
px_err_1609:
    if (px_err_1609_proped) return px_err_1609_val;
    return px_null();
}

static LXValue fn_bc_patch_off(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_patch_off");
    LXValue _v1610 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1611 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1612 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_1613_val = px_null();
    int px_err_1613_proped = 0;
    px_srcline(352);
    px_index_set(px_index(px_index(_v1610, px_str("bc")), _v1611), px_int(3LL), px_sub(_v1612, px_add(_v1611, px_int(1LL))));
px_err_1613:
    if (px_err_1613_proped) return px_err_1613_val;
    return px_null();
}

static LXValue fn_bc_collect_hoist(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_hoist");
    LXValue _v1614 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1615 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1616 = px_uninit();
    LXValue _v1617 = px_uninit();
    LXValue _v1618 = px_uninit();
    LXValue _v1619 = px_uninit();
    LXValue _v1620 = px_uninit();
    LXValue _v1621 = px_uninit();
    LXValue _v1622 = px_uninit();
    LXValue _v1623 = px_uninit();
    LXValue _v1624 = px_uninit();
    LXValue _v1625 = px_uninit();
    LXValue px_err_1626_val = px_null();
    int px_err_1626_proped = 0;
    px_srcline(355);
    _v1616 = px_int(0LL);
    px_srcline(356);
    while (px_is_truthy(px_lt(_v1616, px_call(px_get_global("len"), (LXValue[]){_v1614}, 1)))) {
        px_srcline(357);
        _v1617 = px_index(_v1614, _v1616);
        px_srcline(358);
        _v1618 = px_index(_v1617, px_int(0LL));
        px_srcline(359);
        if (px_is_truthy(px_eq(_v1618, px_str("Assign")))) {
            px_srcline(360);
            _v1619 = px_index(_v1617, px_int(1LL));
            px_srcline(361);
            if (px_is_truthy(px_eq(px_index(_v1619, px_int(0LL)), px_str("Var")))) {
                px_srcline(362);
                _v1620 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1619, px_int(1LL))}, 1);
                px_srcline(363);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1615, _v1620}, 2)))) {
                    px_srcline(364);
                    (void)(px_method(_v1615, "append", (LXValue[]){_v1620}, 1));
                }
            }
        }
        else if (px_is_truthy(px_eq(_v1618, px_str("VarDecl")))) {
            px_srcline(366);
            _v1620 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1617, px_int(2LL))}, 1);
            px_srcline(367);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1615, _v1620}, 2)))) {
                px_srcline(368);
                (void)(px_method(_v1615, "append", (LXValue[]){_v1620}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1618, px_str("FuncDef")))) {
            px_srcline(372);
            _v1620 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1617, px_int(1LL))}, 1);
            px_srcline(373);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1615, _v1620}, 2)))) {
                px_srcline(374);
                (void)(px_method(_v1615, "append", (LXValue[]){_v1620}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1618, px_str("If")))) {
            px_srcline(376);
            _v1621 = px_index(_v1617, px_int(1LL));
            px_srcline(377);
            _v1622 = px_int(0LL);
            px_srcline(378);
            while (px_is_truthy(px_lt(_v1622, px_call(px_get_global("len"), (LXValue[]){_v1621}, 1)))) {
                px_srcline(379);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(px_index(_v1621, _v1622), px_int(1LL)), _v1615}, 2));
                px_srcline(380);
                 _v1622 = px_add(_v1622, px_int(1LL));
            }
            px_srcline(381);
            if (px_is_truthy(px_ne(px_index(_v1617, px_int(2LL)), px_null()))) {
                px_srcline(382);
                (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1617, px_int(2LL)), _v1615}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1618, px_str("While")))) {
            px_srcline(384);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1617, px_int(2LL)), _v1615}, 2));
        }
        else if (px_is_truthy(px_eq(_v1618, px_str("For")))) {
            px_srcline(388);
            _v1623 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1617, px_int(1LL))}, 1);
            px_srcline(389);
            _v1624 = px_int(0LL);
            px_srcline(390);
            while (px_is_truthy(px_lt(_v1624, px_call(px_get_global("len"), (LXValue[]){_v1623}, 1)))) {
                px_srcline(391);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1615, px_index(_v1623, _v1624)}, 2)))) {
                    px_srcline(392);
                    (void)(px_method(_v1615, "append", (LXValue[]){px_index(_v1623, _v1624)}, 1));
                }
                px_srcline(393);
                 _v1624 = px_add(_v1624, px_int(1LL));
            }
            px_srcline(394);
            (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v1617, px_int(3LL)), _v1615}, 2));
        }
        else if (px_is_truthy(px_eq(_v1618, px_str("ChanDecl")))) {
            px_srcline(397);
            _v1625 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1617, px_int(1LL))}, 1);
            px_srcline(398);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1615, _v1625}, 2)))) {
                px_srcline(399);
                (void)(px_method(_v1615, "append", (LXValue[]){_v1625}, 1));
            }
        }
        px_srcline(400);
         _v1616 = px_add(_v1616, px_int(1LL));
    }
px_err_1626:
    if (px_err_1626_proped) return px_err_1626_val;
    return px_null();
}

static LXValue fn_bc_collect_decl_vars(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_collect_decl_vars");
    LXValue _v1627 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1628 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1629 = px_uninit();
    LXValue _v1630 = px_uninit();
    LXValue _v1631 = px_uninit();
    LXValue _v1632 = px_uninit();
    LXValue _v1633 = px_uninit();
    LXValue _v1634 = px_uninit();
    LXValue _v1635 = px_uninit();
    LXValue _v1636 = px_uninit();
    LXValue _v1637 = px_uninit();
    LXValue _v1638 = px_uninit();
    LXValue px_err_1639_val = px_null();
    int px_err_1639_proped = 0;
    px_srcline(409);
    _v1629 = px_int(0LL);
    px_srcline(410);
    while (px_is_truthy(px_lt(_v1629, px_call(px_get_global("len"), (LXValue[]){_v1627}, 1)))) {
        px_srcline(411);
        _v1630 = px_index(_v1627, _v1629);
        px_srcline(412);
        _v1631 = px_index(_v1630, px_int(0LL));
        px_srcline(413);
        if (px_is_truthy(px_eq(_v1631, px_str("VarDecl")))) {
            px_srcline(414);
            _v1632 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1630, px_int(2LL))}, 1);
            px_srcline(415);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1628, _v1632}, 2)))) {
                px_srcline(416);
                (void)(px_method(_v1628, "append", (LXValue[]){_v1632}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1631, px_str("FuncDef")))) {
            px_srcline(418);
            _v1633 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1630, px_int(1LL))}, 1);
            px_srcline(419);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1628, _v1633}, 2)))) {
                px_srcline(420);
                (void)(px_method(_v1628, "append", (LXValue[]){_v1633}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1631, px_str("ChanDecl")))) {
            px_srcline(422);
            _v1634 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1630, px_int(1LL))}, 1);
            px_srcline(423);
            if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1628, _v1634}, 2)))) {
                px_srcline(424);
                (void)(px_method(_v1628, "append", (LXValue[]){_v1634}, 1));
            }
        }
        else if (px_is_truthy(px_eq(_v1631, px_str("If")))) {
            px_srcline(426);
            _v1635 = px_index(_v1630, px_int(1LL));
            px_srcline(427);
            _v1636 = px_int(0LL);
            px_srcline(428);
            while (px_is_truthy(px_lt(_v1636, px_call(px_get_global("len"), (LXValue[]){_v1635}, 1)))) {
                px_srcline(429);
                (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(px_index(_v1635, _v1636), px_int(1LL)), _v1628}, 2));
                px_srcline(430);
                 _v1636 = px_add(_v1636, px_int(1LL));
            }
            px_srcline(431);
            if (px_is_truthy(px_ne(px_index(_v1630, px_int(2LL)), px_null()))) {
                px_srcline(432);
                (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1630, px_int(2LL)), _v1628}, 2));
            }
        }
        else if (px_is_truthy(px_eq(_v1631, px_str("For")))) {
            px_srcline(434);
            _v1637 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v1630, px_int(1LL))}, 1);
            px_srcline(435);
            _v1638 = px_int(0LL);
            px_srcline(436);
            while (px_is_truthy(px_lt(_v1638, px_call(px_get_global("len"), (LXValue[]){_v1637}, 1)))) {
                px_srcline(437);
                if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v1628, px_index(_v1637, _v1638)}, 2)))) {
                    px_srcline(438);
                    (void)(px_method(_v1628, "append", (LXValue[]){px_index(_v1637, _v1638)}, 1));
                }
                px_srcline(439);
                 _v1638 = px_add(_v1638, px_int(1LL));
            }
            px_srcline(440);
            (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1630, px_int(3LL)), _v1628}, 2));
        }
        else if (px_is_truthy(px_eq(_v1631, px_str("While")))) {
            px_srcline(442);
            (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v1630, px_int(2LL)), _v1628}, 2));
        }
        px_srcline(443);
         _v1629 = px_add(_v1629, px_int(1LL));
    }
px_err_1639:
    if (px_err_1639_proped) return px_err_1639_val;
    return px_null();
}

static LXValue fn_bc_emit_null(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_null");
    LXValue _v1640 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1641 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1642 = px_uninit();
    LXValue px_err_1643_val = px_null();
    int px_err_1643_proped = 0;
    px_srcline(446);
    _v1642 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(447);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1640, px_str("LOADK"), _v1641, _v1642, px_int(0LL)}, 5));
px_err_1643:
    if (px_err_1643_proped) return px_err_1643_val;
    return px_null();
}

static LXValue fn_bc_emit_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_int");
    LXValue _v1644 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1645 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1646 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1647 = px_uninit();
    LXValue px_err_1648_val = px_null();
    int px_err_1648_proped = 0;
    px_srcline(450);
    if (px_is_truthy(({ LXValue _t1649 = px_ge(_v1646, px_neg(px_int(32768LL))); px_is_truthy(_t1649) ? px_le(_v1646, px_int(32767LL)) : _t1649; }))) {
        px_srcline(451);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1644, px_str("IMM"), _v1645, _v1646, px_int(0LL)}, 5));
    }
    else {
        px_srcline(453);
        _v1647 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("int"), _v1646, px_float(0), px_str("")}, 5);
        px_srcline(454);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1644, px_str("LOADK"), _v1645, _v1647, px_int(0LL)}, 5));
    }
px_err_1648:
    if (px_err_1648_proped) return px_err_1648_val;
    return px_null();
}

static LXValue fn_bc_neg_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_neg_float");
    LXValue _v1650 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1651_val = px_null();
    int px_err_1651_proped = 0;
    px_srcline(461);
    return px_call(px_get_global("bits_to_float64"), (LXValue[]){px_bitxor(px_call(px_get_global("float64_bits"), (LXValue[]){_v1650}, 1), px_sub(px_sub(px_int(0LL), px_int(9223372036854775807LL)), px_int(1LL)))}, 1);
px_err_1651:
    if (px_err_1651_proped) return px_err_1651_val;
    return px_null();
}

static LXValue fn_bc_emit_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_float");
    LXValue _v1652 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1653 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1654 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1655 = px_uninit();
    LXValue px_err_1656_val = px_null();
    int px_err_1656_proped = 0;
    px_srcline(464);
    _v1655 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("float"), px_int(0LL), _v1654, px_str("")}, 5);
    px_srcline(465);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1652, px_str("LOADK"), _v1653, _v1655, px_int(0LL)}, 5));
px_err_1656:
    if (px_err_1656_proped) return px_err_1656_val;
    return px_null();
}

static LXValue fn_bc_unop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_unop_op");
    LXValue _v1657 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1658_val = px_null();
    int px_err_1658_proped = 0;
    px_srcline(468);
    if (px_is_truthy(px_eq(_v1657, px_str("Neg")))) {
        px_srcline(469);
        return px_str("NEG");
    }
    px_srcline(470);
    if (px_is_truthy(px_eq(_v1657, px_str("Not")))) {
        px_srcline(471);
        return px_str("NOT");
    }
    px_srcline(472);
    if (px_is_truthy(px_eq(_v1657, px_str("BitNot")))) {
        px_srcline(473);
        return px_str("BITNOT");
    }
    px_srcline(474);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_unop_op 未知一元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1657}, 1))}, 1));
px_err_1658:
    if (px_err_1658_proped) return px_err_1658_val;
    return px_null();
}

static LXValue fn_bc_binop_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_binop_op");
    LXValue _v1659 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1660_val = px_null();
    int px_err_1660_proped = 0;
    px_srcline(476);
    if (px_is_truthy(px_eq(_v1659, px_str("Add")))) {
        px_srcline(477);
        return px_str("ADD");
    }
    px_srcline(478);
    if (px_is_truthy(px_eq(_v1659, px_str("Sub")))) {
        px_srcline(479);
        return px_str("SUB");
    }
    px_srcline(480);
    if (px_is_truthy(px_eq(_v1659, px_str("Mul")))) {
        px_srcline(481);
        return px_str("MUL");
    }
    px_srcline(482);
    if (px_is_truthy(px_eq(_v1659, px_str("Div")))) {
        px_srcline(483);
        return px_str("DIV");
    }
    px_srcline(484);
    if (px_is_truthy(px_eq(_v1659, px_str("IntDiv")))) {
        px_srcline(485);
        return px_str("IDIV");
    }
    px_srcline(486);
    if (px_is_truthy(px_eq(_v1659, px_str("Mod")))) {
        px_srcline(487);
        return px_str("MOD");
    }
    px_srcline(488);
    if (px_is_truthy(px_eq(_v1659, px_str("Pow")))) {
        px_srcline(489);
        return px_str("POW");
    }
    px_srcline(490);
    if (px_is_truthy(px_eq(_v1659, px_str("Eq")))) {
        px_srcline(491);
        return px_str("EQ");
    }
    px_srcline(492);
    if (px_is_truthy(px_eq(_v1659, px_str("Ne")))) {
        px_srcline(493);
        return px_str("NE");
    }
    px_srcline(494);
    if (px_is_truthy(px_eq(_v1659, px_str("Lt")))) {
        px_srcline(495);
        return px_str("LT");
    }
    px_srcline(496);
    if (px_is_truthy(px_eq(_v1659, px_str("Le")))) {
        px_srcline(497);
        return px_str("LE");
    }
    px_srcline(498);
    if (px_is_truthy(px_eq(_v1659, px_str("Gt")))) {
        px_srcline(499);
        return px_str("GT");
    }
    px_srcline(500);
    if (px_is_truthy(px_eq(_v1659, px_str("Ge")))) {
        px_srcline(501);
        return px_str("GE");
    }
    px_srcline(502);
    if (px_is_truthy(px_eq(_v1659, px_str("BitAnd")))) {
        px_srcline(503);
        return px_str("BITAND");
    }
    px_srcline(504);
    if (px_is_truthy(px_eq(_v1659, px_str("BitOr")))) {
        px_srcline(505);
        return px_str("BITOR");
    }
    px_srcline(506);
    if (px_is_truthy(px_eq(_v1659, px_str("BitXor")))) {
        px_srcline(507);
        return px_str("BITXOR");
    }
    px_srcline(508);
    if (px_is_truthy(px_eq(_v1659, px_str("Shl")))) {
        px_srcline(509);
        return px_str("SHL");
    }
    px_srcline(510);
    if (px_is_truthy(px_eq(_v1659, px_str("Shr")))) {
        px_srcline(511);
        return px_str("SHR");
    }
    px_srcline(512);
    if (px_is_truthy(px_eq(_v1659, px_str("ShrU")))) {
        px_srcline(513);
        return px_str("SHRU");
    }
    px_srcline(514);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_binop_op 未知二元 op: "), px_call(px_get_global("str"), (LXValue[]){_v1659}, 1))}, 1));
px_err_1660:
    if (px_err_1660_proped) return px_err_1660_val;
    return px_null();
}

static LXValue fn_bc_emit_expr(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_expr");
    LXValue _v1661 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1662 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1663 = (nargs > 2) ? args[2] : px_null();
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
    LXValue _v1710 = px_uninit();
    LXValue _v1711 = px_uninit();
    LXValue _v1712 = px_uninit();
    LXValue _v1713 = px_uninit();
    LXValue _v1714 = px_uninit();
    LXValue _v1715 = px_uninit();
    LXValue _v1716 = px_uninit();
    LXValue _v1717 = px_uninit();
    LXValue _v1718 = px_uninit();
    LXValue px_err_1719_val = px_null();
    int px_err_1719_proped = 0;
    px_srcline(519);
    _v1664 = px_index(_v1661, px_int(0LL));
    px_srcline(520);
    if (px_is_truthy(px_eq(_v1664, px_str("Int")))) {
        px_srcline(521);
        (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1663, _v1662, px_index(_v1661, px_int(1LL))}, 3));
        px_srcline(522);
        return _v1662;
    }
    px_srcline(523);
    if (px_is_truthy(px_eq(_v1664, px_str("Float")))) {
        px_srcline(524);
        (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1663, _v1662, px_index(_v1661, px_int(1LL))}, 3));
        px_srcline(525);
        return _v1662;
    }
    px_srcline(526);
    if (px_is_truthy(px_eq(_v1664, px_str("Unary")))) {
        px_srcline(529);
        _v1665 = px_index(_v1661, px_int(2LL));
        px_srcline(530);
        _v1666 = px_index(_v1661, px_int(1LL));
        px_srcline(531);
        if (px_is_truthy(({ LXValue _t1720 = px_eq(px_index(_v1665, px_int(0LL)), px_str("Int")); px_is_truthy(_t1720) ? px_eq(_v1666, px_str("Neg")) : _t1720; }))) {
            px_srcline(532);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1663, _v1662, px_sub(px_int(0LL), px_index(_v1665, px_int(1LL)))}, 3));
            px_srcline(533);
            return _v1662;
        }
        px_srcline(534);
        if (px_is_truthy(({ LXValue _t1721 = px_eq(px_index(_v1665, px_int(0LL)), px_str("Float")); px_is_truthy(_t1721) ? px_eq(_v1666, px_str("Neg")) : _t1721; }))) {
            px_srcline(541);
            (void)(px_call(px_get_global("bc_emit_float"), (LXValue[]){_v1663, _v1662, px_call(px_get_global("bc_neg_float"), (LXValue[]){px_index(_v1665, px_int(1LL))}, 1)}, 3));
            px_srcline(542);
            return _v1662;
        }
        px_srcline(543);
        _v1667 = _v1662;
        px_srcline(544);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(545);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(546);
        _v1668 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(547);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1665, _v1668, _v1663}, 3));
        px_srcline(548);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_call(px_get_global("bc_unop_op"), (LXValue[]){_v1666}, 1), _v1667, _v1668, px_int(0LL)}, 5));
        px_srcline(549);
        return _v1667;
    }
    px_srcline(550);
    if (px_is_truthy(px_eq(_v1664, px_str("Str")))) {
        px_srcline(551);
        _v1669 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1661, px_int(1LL))}, 1)}, 5);
        px_srcline(552);
        if (px_is_truthy(px_ge(_v1662, px_int(0LL)))) {
            px_srcline(553);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("LOADK"), _v1662, _v1669, px_int(0LL)}, 5));
        }
        px_srcline(554);
        return _v1662;
    }
    px_srcline(555);
    if (px_is_truthy(px_eq(_v1664, px_str("Bool")))) {
        px_srcline(556);
        _v1670 = px_int(0LL);
        px_srcline(557);
        if (px_is_truthy(px_index(_v1661, px_int(1LL)))) {
            px_srcline(558);
             _v1670 = px_int(1LL);
        }
        px_srcline(559);
        _v1669 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("bool"), _v1670, px_float(0), px_str("")}, 5);
        px_srcline(560);
        if (px_is_truthy(px_ge(_v1662, px_int(0LL)))) {
            px_srcline(561);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("LOADK"), _v1662, _v1669, px_int(0LL)}, 5));
        }
        px_srcline(562);
        return _v1662;
    }
    px_srcline(563);
    if (px_is_truthy(px_eq(_v1664, px_str("Null")))) {
        px_srcline(564);
        if (px_is_truthy(px_ge(_v1662, px_int(0LL)))) {
            px_srcline(565);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1663, _v1662}, 2));
        }
        px_srcline(566);
        return _v1662;
    }
    px_srcline(567);
    if (px_is_truthy(px_eq(_v1664, px_str("Var")))) {
        px_srcline(568);
        _v1671 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1661, px_int(1LL))}, 1);
        px_srcline(569);
        if (px_is_truthy(px_method(px_index(_v1663, px_str("smap")), "has", (LXValue[]){_v1671}, 1))) {
            px_srcline(572);
            (void)(px_call(px_get_global("bc_load_ck"), (LXValue[]){_v1663, _v1671, _v1662}, 3));
            px_srcline(573);
            return _v1662;
        }
        px_srcline(575);
        _v1672 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1671}, 2);
        px_srcline(576);
        if (px_is_truthy(px_ge(_v1662, px_int(0LL)))) {
            px_srcline(577);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("GETG"), _v1662, _v1672, px_int(0LL)}, 5));
        }
        px_srcline(578);
        return _v1662;
    }
    px_srcline(579);
    if (px_is_truthy(px_eq(_v1664, px_str("Binary")))) {
        px_srcline(581);
        _v1666 = px_index(_v1661, px_int(1LL));
        px_srcline(582);
        _v1667 = _v1662;
        px_srcline(583);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(584);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(585);
        if (px_is_truthy(px_eq(_v1666, px_str("And")))) {
            px_srcline(587);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1667, _v1663}, 3));
            px_srcline(588);
            _v1673 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
            px_srcline(589);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPF"), _v1667, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(590);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(3LL)), _v1667, _v1663}, 3));
            px_srcline(591);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1673, px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1)}, 3));
            px_srcline(592);
            return _v1667;
        }
        px_srcline(593);
        if (px_is_truthy(px_eq(_v1666, px_str("Or")))) {
            px_srcline(595);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1667, _v1663}, 3));
            px_srcline(596);
            _v1673 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
            px_srcline(597);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPT"), _v1667, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(598);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(3LL)), _v1667, _v1663}, 3));
            px_srcline(599);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1673, px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1)}, 3));
            px_srcline(600);
            return _v1667;
        }
        px_srcline(601);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1667, _v1663}, 3));
        px_srcline(602);
        _v1674 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(603);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(3LL)), _v1674, _v1663}, 3));
        px_srcline(604);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_call(px_get_global("bc_binop_op"), (LXValue[]){_v1666}, 1), _v1667, _v1667, _v1674}, 5));
        px_srcline(605);
        return _v1667;
    }
    px_srcline(606);
    if (px_is_truthy(px_eq(_v1664, px_str("NullCoalesce")))) {
        px_srcline(608);
        _v1667 = _v1662;
        px_srcline(609);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(610);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(611);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1667, _v1663}, 3));
        px_srcline(612);
        _v1675 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(613);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1663, _v1675}, 2));
        px_srcline(614);
        _v1676 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(615);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("EQ"), _v1676, _v1667, _v1675}, 5));
        px_srcline(616);
        _v1673 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
        px_srcline(617);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPF"), _v1676, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(618);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1667, _v1663}, 3));
        px_srcline(619);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1673, px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1)}, 3));
        px_srcline(620);
        return _v1667;
    }
    px_srcline(621);
    if (px_is_truthy(px_eq(_v1664, px_str("IfExpr")))) {
        px_srcline(623);
        _v1667 = _v1662;
        px_srcline(624);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(625);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(626);
        _v1677 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(627);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1677, _v1663}, 3));
        px_srcline(628);
        _v1678 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
        px_srcline(629);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPF"), _v1677, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(630);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1667, _v1663}, 3));
        px_srcline(631);
        _v1679 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
        px_srcline(632);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(633);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1678, px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1)}, 3));
        px_srcline(634);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(3LL)), _v1667, _v1663}, 3));
        px_srcline(635);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1679, px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1)}, 3));
        px_srcline(636);
        return _v1667;
    }
    px_srcline(637);
    if (px_is_truthy(px_eq(_v1664, px_str("Try")))) {
        px_srcline(644);
        _v1667 = _v1662;
        px_srcline(645);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(646);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(647);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1667, _v1663}, 3));
        px_srcline(648);
        _v1680 = px_int(0LL);
        px_srcline(649);
        if (px_is_truthy(px_index(_v1663, px_str("is_top")))) {
            px_srcline(650);
             _v1680 = px_int(1LL);
        }
        px_srcline(651);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("TRY"), _v1667, _v1680, px_int(0LL)}, 5));
        px_srcline(652);
        return _v1667;
    }
    px_srcline(653);
    if (px_is_truthy(px_eq(_v1664, px_str("ForceUnwrap")))) {
        px_srcline(655);
        _v1667 = _v1662;
        px_srcline(656);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(657);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(658);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1667, _v1663}, 3));
        px_srcline(659);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("FORCE"), _v1667, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(660);
        return _v1667;
    }
    px_srcline(661);
    if (px_is_truthy(({ LXValue _t1722 = px_eq(_v1664, px_str("List")); px_is_truthy(_t1722) ? _t1722 : px_eq(_v1664, px_str("Tuple")); }))) {
        px_srcline(663);
        _v1681 = px_index(_v1661, px_int(1LL));
        px_srcline(664);
        _v1682 = px_call(px_get_global("len"), (LXValue[]){_v1681}, 1);
        px_srcline(665);
        _v1667 = _v1662;
        px_srcline(666);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(667);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(668);
        _v1683 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(669);
        while (px_is_truthy(px_lt(px_index(_v1663, px_str("next_slot")), px_add(_v1683, _v1682)))) {
            px_srcline(670);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1));
        }
        px_srcline(671);
        _v1684 = px_int(0LL);
        px_srcline(672);
        while (px_is_truthy(px_lt(_v1684, _v1682))) {
            px_srcline(673);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1681, _v1684), px_add(_v1683, _v1684), _v1663}, 3));
            px_srcline(674);
             _v1684 = px_add(_v1684, px_int(1LL));
        }
        px_srcline(675);
        if (px_is_truthy(px_eq(_v1664, px_str("List")))) {
            px_srcline(676);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("NEWLIST"), _v1667, _v1683, _v1682}, 5));
        }
        else {
            px_srcline(678);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("NEWTUPLE"), _v1667, _v1683, _v1682}, 5));
        }
        px_srcline(679);
        return _v1667;
    }
    px_srcline(680);
    if (px_is_truthy(px_eq(_v1664, px_str("Dict")))) {
        px_srcline(682);
        _v1685 = px_index(_v1661, px_int(1LL));
        px_srcline(683);
        _v1682 = px_call(px_get_global("len"), (LXValue[]){_v1685}, 1);
        px_srcline(684);
        _v1667 = _v1662;
        px_srcline(685);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(686);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(687);
        _v1683 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(688);
        while (px_is_truthy(px_lt(px_index(_v1663, px_str("next_slot")), px_add(_v1683, px_mul(px_int(2LL), _v1682))))) {
            px_srcline(689);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1));
        }
        px_srcline(690);
        _v1686 = px_int(0LL);
        px_srcline(691);
        while (px_is_truthy(px_lt(_v1686, _v1682))) {
            px_srcline(692);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1685, _v1686), px_int(0LL)), px_add(_v1683, px_mul(px_int(2LL), _v1686)), _v1663}, 3));
            px_srcline(693);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1685, _v1686), px_int(1LL)), px_add(px_add(_v1683, px_mul(px_int(2LL), _v1686)), px_int(1LL)), _v1663}, 3));
            px_srcline(694);
             _v1686 = px_add(_v1686, px_int(1LL));
        }
        px_srcline(695);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("NEWDICT"), _v1667, _v1683, _v1682}, 5));
        px_srcline(696);
        return _v1667;
    }
    px_srcline(697);
    if (px_is_truthy(px_eq(_v1664, px_str("Index")))) {
        px_srcline(699);
        _v1667 = _v1662;
        px_srcline(700);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(701);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(702);
        _v1687 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(703);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1687, _v1663}, 3));
        px_srcline(704);
        _v1688 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(705);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1688, _v1663}, 3));
        px_srcline(706);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("INDEX"), _v1667, _v1687, _v1688}, 5));
        px_srcline(707);
        return _v1667;
    }
    px_srcline(708);
    if (px_is_truthy(px_eq(_v1664, px_str("Slice")))) {
        px_srcline(710);
        _v1667 = _v1662;
        px_srcline(711);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(712);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(713);
        _v1687 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(714);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1687, _v1663}, 3));
        px_srcline(715);
        _v1683 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(716);
        while (px_is_truthy(px_lt(px_index(_v1663, px_str("next_slot")), px_add(_v1683, px_int(3LL))))) {
            px_srcline(717);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1));
        }
        px_srcline(718);
        if (px_is_truthy(px_eq(px_index(_v1661, px_int(2LL)), px_null()))) {
            px_srcline(719);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1663, _v1683}, 2));
        }
        else {
            px_srcline(721);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(2LL)), _v1683, _v1663}, 3));
        }
        px_srcline(722);
        if (px_is_truthy(px_eq(px_index(_v1661, px_int(3LL)), px_null()))) {
            px_srcline(723);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1663, px_add(_v1683, px_int(1LL))}, 2));
        }
        else {
            px_srcline(725);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(3LL)), px_add(_v1683, px_int(1LL)), _v1663}, 3));
        }
        px_srcline(726);
        if (px_is_truthy(px_eq(px_index(_v1661, px_int(4LL)), px_null()))) {
            px_srcline(727);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1663, px_add(_v1683, px_int(2LL))}, 2));
        }
        else {
            px_srcline(729);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(4LL)), px_add(_v1683, px_int(2LL)), _v1663}, 3));
        }
        px_srcline(730);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("SLICE"), _v1667, _v1687, _v1683}, 5));
        px_srcline(731);
        return _v1667;
    }
    px_srcline(732);
    if (px_is_truthy(px_eq(_v1664, px_str("Field")))) {
        px_srcline(736);
        _v1689 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1661, px_int(2LL))}, 1);
        px_srcline(737);
        _v1690 = px_index(_v1661, px_int(1LL));
        px_srcline(738);
        if (px_is_truthy(px_eq(px_index(_v1690, px_int(0LL)), px_str("Var")))) {
            px_srcline(739);
            _v1691 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1690, px_int(1LL))}, 1);
            px_srcline(740);
            _v1692 = px_call(px_get_global("bc_const_find"), (LXValue[]){_v1691, _v1689}, 2);
            px_srcline(741);
            if (px_is_truthy(px_ne(_v1692, px_null()))) {
                px_srcline(742);
                _v1667 = _v1662;
                px_srcline(743);
                if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
                    px_srcline(744);
                     _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
                }
                px_srcline(745);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1692, _v1667, _v1663}, 3));
                px_srcline(746);
                return _v1667;
            }
            px_srcline(747);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1691, _v1689}, 2))) {
                px_srcline(748);
                _v1667 = _v1662;
                px_srcline(749);
                if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
                    px_srcline(750);
                     _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
                }
                px_srcline(751);
                _v1675 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1691}, 2);
                px_srcline(752);
                _v1693 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1689}, 2);
                px_srcline(753);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("NEWENUM"), _v1667, _v1675, _v1693}, 5));
                px_srcline(754);
                return _v1667;
            }
        }
        px_srcline(755);
        _v1667 = _v1662;
        px_srcline(756);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(757);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(758);
        _v1687 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(759);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1687, _v1663}, 3));
        px_srcline(760);
        _v1694 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1689}, 2);
        px_srcline(761);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("GETF"), _v1667, _v1687, _v1694}, 5));
        px_srcline(762);
        return _v1667;
    }
    px_srcline(763);
    if (px_is_truthy(px_eq(_v1664, px_str("OptionalField")))) {
        px_srcline(765);
        _v1667 = _v1662;
        px_srcline(766);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(767);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(768);
        _v1687 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(769);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1687, _v1663}, 3));
        px_srcline(770);
        _v1694 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1661, px_int(2LL))}, 1)}, 2);
        px_srcline(771);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("GETF_OPT"), _v1667, _v1687, _v1694}, 5));
        px_srcline(772);
        return _v1667;
    }
    px_srcline(773);
    if (px_is_truthy(px_eq(_v1664, px_str("Call")))) {
        px_srcline(776);
        _v1695 = px_index(_v1661, px_int(1LL));
        px_srcline(780);
        (void)(px_call(px_get_global("cg_sem_call"), (LXValue[]){_v1695, px_index(_v1661, px_int(2LL))}, 2));
        px_srcline(781);
        if (px_is_truthy(px_eq(px_index(_v1695, px_int(0LL)), px_str("Var")))) {
            px_srcline(782);
            _v1696 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1695, px_int(1LL))}, 1);
            px_srcline(783);
            _v1697 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1696}, 1);
            px_srcline(784);
            if (px_is_truthy(px_ge(_v1697, px_int(0LL)))) {
                px_srcline(785);
                return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1696, _v1697, px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 5);
            }
            px_srcline(786);
            if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1696}, 1))) {
                px_srcline(787);
                return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1696, px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 4);
            }
        }
        px_srcline(788);
        if (px_is_truthy(px_eq(px_index(_v1695, px_int(0LL)), px_str("Field")))) {
            px_srcline(789);
            return px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1695, px_int(1LL)), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1695, px_int(2LL))}, 1), px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 5);
        }
        px_srcline(790);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1695, px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 4);
    }
    px_srcline(791);
    if (px_is_truthy(px_eq(_v1664, px_str("Constructor")))) {
        px_srcline(793);
        _v1696 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1661, px_int(1LL))}, 1);
        px_srcline(794);
        _v1697 = px_call(px_get_global("bc_struct_index"), (LXValue[]){_v1696}, 1);
        px_srcline(795);
        if (px_is_truthy(px_ge(_v1697, px_int(0LL)))) {
            px_srcline(796);
            return px_call(px_get_global("bc_emit_struct_new"), (LXValue[]){_v1696, _v1697, px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 5);
        }
        px_srcline(797);
        if (px_is_truthy(px_method(px_index(px_get_global("g_bcm"), px_str("enums")), "has", (LXValue[]){_v1696}, 1))) {
            px_srcline(798);
            return px_call(px_get_global("bc_emit_enum_new"), (LXValue[]){_v1696, px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 4);
        }
        px_srcline(800);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1661, px_int(1LL)), px_index(_v1661, px_int(2LL)), _v1662, _v1663}, 4);
    }
    px_srcline(801);
    if (px_is_truthy(px_eq(_v1664, px_str("Pipe")))) {
        px_srcline(803);
        _v1698 = px_index(_v1661, px_int(1LL));
        px_srcline(804);
        _v1699 = px_index(_v1661, px_int(2LL));
        px_srcline(805);
        if (px_is_truthy(px_eq(px_index(_v1699, px_int(0LL)), px_str("Call")))) {
            px_srcline(806);
            _v1700 = px_list_n((LXValue[]){}, 0);
            px_srcline(807);
            (void)(px_method(_v1700, "append", (LXValue[]){_v1698}, 1));
            px_srcline(808);
            _v1701 = px_int(0LL);
            px_srcline(809);
            while (px_is_truthy(px_lt(_v1701, px_call(px_get_global("len"), (LXValue[]){px_index(_v1699, px_int(2LL))}, 1)))) {
                px_srcline(810);
                (void)(px_method(_v1700, "append", (LXValue[]){px_index(px_index(_v1699, px_int(2LL)), _v1701)}, 1));
                px_srcline(811);
                 _v1701 = px_add(_v1701, px_int(1LL));
            }
            px_srcline(812);
            return px_call(px_get_global("bc_emit_call"), (LXValue[]){px_index(_v1699, px_int(1LL)), _v1700, _v1662, _v1663}, 4);
        }
        px_srcline(813);
        _v1700 = px_list_n((LXValue[]){_v1698}, 1);
        px_srcline(814);
        return px_call(px_get_global("bc_emit_call"), (LXValue[]){_v1699, _v1700, _v1662, _v1663}, 4);
    }
    px_srcline(815);
    if (px_is_truthy(px_eq(_v1664, px_str("ListComp")))) {
        px_srcline(817);
        _v1702 = px_index(_v1661, px_int(2LL));
        px_srcline(818);
        _v1667 = _v1662;
        px_srcline(819);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(820);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(821);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("NEWLIST"), _v1667, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(822);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1663, px_str("push"), px_list_n((LXValue[]){px_index(_v1661, px_int(1LL))}, 1), px_index(_v1661, px_int(3LL)), _v1667, _v1702, px_int(0LL)}, 7));
        px_srcline(823);
        return _v1667;
    }
    px_srcline(824);
    if (px_is_truthy(px_eq(_v1664, px_str("DictComp")))) {
        px_srcline(827);
        _v1702 = px_index(_v1661, px_int(3LL));
        px_srcline(828);
        _v1667 = _v1662;
        px_srcline(829);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(830);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(831);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("NEWDICT"), _v1667, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(832);
        (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1663, px_str("dict"), px_list_n((LXValue[]){px_index(_v1661, px_int(1LL)), px_index(_v1661, px_int(2LL))}, 2), px_index(_v1661, px_int(4LL)), _v1667, _v1702, px_int(0LL)}, 7));
        px_srcline(833);
        return _v1667;
    }
    px_srcline(834);
    if (px_is_truthy(px_eq(_v1664, px_str("GenExp")))) {
        px_srcline(835);
        return px_call(px_get_global("bc_emit_genexp"), (LXValue[]){_v1661, _v1662, _v1663}, 3);
    }
    px_srcline(836);
    if (px_is_truthy(px_eq(_v1664, px_str("Match")))) {
        px_srcline(839);
        _v1667 = _v1662;
        px_srcline(840);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(841);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(842);
        _v1703 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        px_srcline(843);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1661, px_int(1LL)), _v1703, _v1663}, 3));
        px_srcline(844);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("MOV"), _v1667, _v1703, px_int(0LL)}, 5));
        px_srcline(845);
        _v1704 = px_index(_v1661, px_int(2LL));
        px_srcline(846);
        _v1705 = px_list_n((LXValue[]){}, 0);
        px_srcline(847);
        _v1706 = px_int(0LL);
        px_srcline(848);
        while (px_is_truthy(px_lt(_v1706, px_call(px_get_global("len"), (LXValue[]){_v1704}, 1)))) {
            px_srcline(849);
            _v1707 = px_index(_v1704, _v1706);
            px_srcline(850);
            _v1708 = px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1707, px_int(1LL)), _v1703, _v1663}, 3);
            px_srcline(851);
            _v1709 = px_neg(px_int(1LL));
            px_srcline(852);
            _v1710 = px_neg(px_int(1LL));
            px_srcline(853);
            if (px_is_truthy(px_eq(_v1708, px_null()))) {
                px_srcline(855);
                if (px_is_truthy(px_ne(px_index(_v1707, px_int(2LL)), px_null()))) {
                    px_srcline(856);
                    _v1711 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
                    px_srcline(857);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1707, px_int(2LL)), _v1711, _v1663}, 3));
                    px_srcline(858);
                    _v1712 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
                    px_srcline(859);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPF"), _v1711, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(860);
                     _v1710 = _v1712;
                }
            }
            else {
                px_srcline(862);
                _v1678 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
                px_srcline(863);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPF"), _v1708, px_int(0LL), px_int(0LL)}, 5));
                px_srcline(864);
                 _v1709 = _v1678;
                px_srcline(865);
                if (px_is_truthy(px_ne(px_index(_v1707, px_int(2LL)), px_null()))) {
                    px_srcline(866);
                    _v1711 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
                    px_srcline(867);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1707, px_int(2LL)), _v1711, _v1663}, 3));
                    px_srcline(868);
                    _v1712 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
                    px_srcline(869);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMPF"), _v1711, px_int(0LL), px_int(0LL)}, 5));
                    px_srcline(870);
                     _v1710 = _v1712;
                }
            }
            px_srcline(871);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1707, px_int(3LL)), _v1667, _v1663}, 3));
            px_srcline(872);
            _v1679 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
            px_srcline(873);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1663, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(874);
            _v1713 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
            px_srcline(875);
            if (px_is_truthy(px_ge(_v1709, px_int(0LL)))) {
                px_srcline(876);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1709, _v1713}, 3));
            }
            px_srcline(877);
            if (px_is_truthy(px_ge(_v1710, px_int(0LL)))) {
                px_srcline(878);
                (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, _v1710, _v1713}, 3));
            }
            px_srcline(879);
            (void)(px_method(_v1705, "append", (LXValue[]){_v1679}, 1));
            px_srcline(880);
             _v1706 = px_add(_v1706, px_int(1LL));
        }
        px_srcline(881);
        _v1714 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1663, px_str("bc"))}, 1);
        px_srcline(882);
        _v1715 = px_int(0LL);
        px_srcline(883);
        while (px_is_truthy(px_lt(_v1715, px_call(px_get_global("len"), (LXValue[]){_v1705}, 1)))) {
            px_srcline(884);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1663, px_index(_v1705, _v1715), _v1714}, 3));
            px_srcline(885);
             _v1715 = px_add(_v1715, px_int(1LL));
        }
        px_srcline(886);
        return _v1667;
    }
    px_srcline(887);
    if (px_is_truthy(px_eq(_v1664, px_str("Block")))) {
        px_srcline(891);
        _v1667 = _v1662;
        px_srcline(892);
        if (px_is_truthy(px_lt(_v1667, px_int(0LL)))) {
            px_srcline(893);
             _v1667 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1663}, 1);
        }
        px_srcline(894);
        (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1663, _v1667}, 2));
        px_srcline(895);
        _v1716 = px_index(_v1661, px_int(1LL));
        px_srcline(896);
        _v1717 = px_int(0LL);
        px_srcline(897);
        while (px_is_truthy(px_lt(_v1717, px_call(px_get_global("len"), (LXValue[]){_v1716}, 1)))) {
            px_srcline(898);
            _v1718 = px_index(_v1716, _v1717);
            px_srcline(899);
            if (px_is_truthy(px_eq(px_index(_v1718, px_int(0LL)), px_str("ExprStmt")))) {
                px_srcline(900);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1718, px_int(1LL)), _v1667, _v1663}, 3));
            }
            else {
                px_srcline(902);
                (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){_v1718, _v1663}, 2));
            }
            px_srcline(903);
             _v1717 = px_add(_v1717, px_int(1LL));
        }
        px_srcline(904);
        return _v1667;
    }
    px_srcline(905);
    if (px_is_truthy(px_eq(_v1664, px_str("Closure")))) {
        px_srcline(908);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1661, px_int(1LL)), px_index(_v1661, px_int(3LL)), _v1662, _v1663}, 4));
        px_srcline(909);
        return _v1662;
    }
    px_srcline(910);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_expr 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1661}, 1))}, 1));
px_err_1719:
    if (px_err_1719_proped) return px_err_1719_val;
    return px_null();
}

static LXValue fn_bc_emit_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_call");
    LXValue _v1723 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1724 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1725 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1726 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1727 = px_uninit();
    LXValue _v1728 = px_uninit();
    LXValue _v1729 = px_uninit();
    LXValue _v1730 = px_uninit();
    LXValue _v1731 = px_uninit();
    LXValue px_err_1732_val = px_null();
    int px_err_1732_proped = 0;
    px_srcline(914);
    _v1727 = _v1725;
    px_srcline(915);
    if (px_is_truthy(px_lt(_v1727, px_int(0LL)))) {
        px_srcline(916);
         _v1727 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1726}, 1);
    }
    px_srcline(917);
    _v1728 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1726}, 1);
    px_srcline(918);
    _v1729 = px_call(px_get_global("len"), (LXValue[]){_v1724}, 1);
    px_srcline(919);
    _v1730 = px_add(px_add(_v1728, px_int(1LL)), _v1729);
    px_srcline(920);
    while (px_is_truthy(px_lt(px_index(_v1726, px_str("next_slot")), _v1730))) {
        px_srcline(921);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1726}, 1));
    }
    px_srcline(922);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1723, _v1728, _v1726}, 3));
    px_srcline(923);
    _v1731 = px_int(0LL);
    px_srcline(924);
    while (px_is_truthy(px_lt(_v1731, _v1729))) {
        px_srcline(925);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1724, _v1731), px_add(px_add(_v1728, px_int(1LL)), _v1731), _v1726}, 3));
        px_srcline(926);
         _v1731 = px_add(_v1731, px_int(1LL));
    }
    px_srcline(927);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1726, px_str("CALL"), _v1727, _v1728, _v1729}, 5));
    px_srcline(928);
    return _v1727;
px_err_1732:
    if (px_err_1732_proped) return px_err_1732_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall");
    LXValue _v1733 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1734 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1735 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1736 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1737 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1738 = px_uninit();
    LXValue _v1739 = px_uninit();
    LXValue _v1740 = px_uninit();
    LXValue _v1741 = px_uninit();
    LXValue _v1742 = px_uninit();
    LXValue _v1743 = px_uninit();
    LXValue px_err_1744_val = px_null();
    int px_err_1744_proped = 0;
    px_srcline(934);
    _v1738 = _v1736;
    px_srcline(935);
    if (px_is_truthy(px_lt(_v1738, px_int(0LL)))) {
        px_srcline(936);
         _v1738 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1737}, 1);
    }
    px_srcline(937);
    _v1739 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1737}, 1);
    px_srcline(938);
    _v1740 = px_call(px_get_global("len"), (LXValue[]){_v1735}, 1);
    px_srcline(939);
    _v1741 = px_add(px_add(_v1739, px_int(1LL)), _v1740);
    px_srcline(940);
    while (px_is_truthy(px_lt(px_index(_v1737, px_str("next_slot")), _v1741))) {
        px_srcline(941);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1737}, 1));
    }
    px_srcline(942);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1733, _v1739, _v1737}, 3));
    px_srcline(943);
    _v1742 = px_int(0LL);
    px_srcline(944);
    while (px_is_truthy(px_lt(_v1742, _v1740))) {
        px_srcline(945);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1735, _v1742), px_add(px_add(_v1739, px_int(1LL)), _v1742), _v1737}, 3));
        px_srcline(946);
         _v1742 = px_add(_v1742, px_int(1LL));
    }
    px_srcline(947);
    _v1743 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1734}, 2);
    px_srcline(948);
    (void)(px_method(px_index(_v1737, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1740, _v1738, _v1739, _v1743}, 5)}, 1));
    px_srcline(949);
    return _v1738;
px_err_1744:
    if (px_err_1744_proped) return px_err_1744_val;
    return px_null();
}

static LXValue fn_bc_emit_struct_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_struct_new");
    LXValue _v1745 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1746 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1747 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1748 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1749 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1750 = px_uninit();
    LXValue _v1751 = px_uninit();
    LXValue _v1752 = px_uninit();
    LXValue _v1753 = px_uninit();
    LXValue _v1754 = px_uninit();
    LXValue px_err_1755_val = px_null();
    int px_err_1755_proped = 0;
    px_srcline(953);
    _v1750 = px_index(px_index(px_get_global("g_bcm"), px_str("structs")), _v1746);
    px_srcline(954);
    _v1751 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1750, px_str("fnames"))}, 1);
    px_srcline(955);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v1747}, 1), _v1751))) {
        px_srcline(956);
        (void)(px_call(px_get_global("panic"), (LXValue[]){({ LXValue _s207 = px_add(px_add(px_add(px_add(px_str("结构体 "), _v1745), px_str(" 需要 ")), px_call(px_get_global("str"), (LXValue[]){_v1751}, 1)), px_str(" 个字段，给出 ")); LXValue _s208 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1747}, 1)}, 1); px_add(_s207, _s208); })}, 1));
    }
    px_srcline(957);
    _v1752 = _v1748;
    px_srcline(958);
    if (px_is_truthy(px_lt(_v1752, px_int(0LL)))) {
        px_srcline(959);
         _v1752 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1749}, 1);
    }
    px_srcline(960);
    _v1753 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1749}, 1);
    px_srcline(961);
    while (px_is_truthy(px_lt(px_index(_v1749, px_str("next_slot")), px_add(_v1753, _v1751)))) {
        px_srcline(962);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1749}, 1));
    }
    px_srcline(963);
    _v1754 = px_int(0LL);
    px_srcline(964);
    while (px_is_truthy(px_lt(_v1754, _v1751))) {
        px_srcline(965);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1747, _v1754), px_add(_v1753, _v1754), _v1749}, 3));
        px_srcline(966);
         _v1754 = px_add(_v1754, px_int(1LL));
    }
    px_srcline(967);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1749, px_str("NEWSTRUCT"), _v1752, _v1746, _v1753}, 5));
    px_srcline(968);
    return _v1752;
px_err_1755:
    if (px_err_1755_proped) return px_err_1755_val;
    return px_null();
}

static LXValue fn_bc_emit_enum_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_enum_new");
    LXValue _v1756 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1757 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1758 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1759 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1760 = px_uninit();
    LXValue _v1761 = px_uninit();
    LXValue _v1762 = px_uninit();
    LXValue _v1763 = px_uninit();
    LXValue _v1764 = px_uninit();
    LXValue _v1765 = px_uninit();
    LXValue px_err_1766_val = px_null();
    int px_err_1766_proped = 0;
    px_srcline(971);
    _v1760 = px_null();
    px_srcline(972);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1757}, 1), px_int(1LL)))) {
        px_srcline(973);
        _v1761 = px_index(_v1757, px_int(0LL));
        px_srcline(974);
        if (px_is_truthy(px_eq(px_index(_v1761, px_int(0LL)), px_str("Var")))) {
            px_srcline(975);
            _v1762 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1761, px_int(1LL))}, 1);
            px_srcline(976);
            if (px_is_truthy(px_call(px_get_global("bc_enum_has"), (LXValue[]){_v1756, _v1762}, 2))) {
                px_srcline(977);
                 _v1760 = _v1762;
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v1761, px_int(0LL)), px_str("Str")))) {
            px_srcline(979);
             _v1760 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1761, px_int(1LL))}, 1);
        }
    }
    px_srcline(980);
    if (px_is_truthy(px_eq(_v1760, px_null()))) {
        px_srcline(981);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit enum 构造未实现（仅支持 Color(Variant)/Color(\"Variant\"): "), px_call(px_get_global("str"), (LXValue[]){_v1756}, 1))}, 1));
    }
    px_srcline(982);
    _v1763 = _v1758;
    px_srcline(983);
    if (px_is_truthy(px_lt(_v1763, px_int(0LL)))) {
        px_srcline(984);
         _v1763 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1759}, 1);
    }
    px_srcline(985);
    _v1764 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1756}, 2);
    px_srcline(986);
    _v1765 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1760}, 2);
    px_srcline(987);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1759, px_str("NEWENUM"), _v1763, _v1764, _v1765}, 5));
    px_srcline(988);
    return _v1763;
px_err_1766:
    if (px_err_1766_proped) return px_err_1766_val;
    return px_null();
}

static LXValue fn_bc_emit_methodcall_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_methodcall_slot");
    LXValue _v1767 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1768 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1769 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1770 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1771 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1772 = px_uninit();
    LXValue _v1773 = px_uninit();
    LXValue _v1774 = px_uninit();
    LXValue _v1775 = px_uninit();
    LXValue _v1776 = px_uninit();
    LXValue _v1777 = px_uninit();
    LXValue px_err_1778_val = px_null();
    int px_err_1778_proped = 0;
    px_srcline(995);
    _v1772 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1767}, 1);
    px_srcline(996);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1767, px_str("MOV"), _v1772, _v1768, px_int(0LL)}, 5));
    px_srcline(997);
    _v1773 = px_call(px_get_global("len"), (LXValue[]){_v1770}, 1);
    px_srcline(998);
    _v1774 = px_add(px_add(_v1772, px_int(1LL)), _v1773);
    px_srcline(999);
    while (px_is_truthy(px_lt(px_index(_v1767, px_str("next_slot")), _v1774))) {
        px_srcline(1000);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1767}, 1));
    }
    px_srcline(1001);
    _v1775 = px_int(0LL);
    px_srcline(1002);
    while (px_is_truthy(px_lt(_v1775, _v1773))) {
        px_srcline(1003);
        _v1776 = px_index(_v1770, _v1775);
        px_srcline(1004);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v1776}, 1), px_str("int")))) {
            px_srcline(1005);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1767, px_str("MOV"), px_add(px_add(_v1772, px_int(1LL)), _v1775), _v1776, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1007);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1776, px_add(px_add(_v1772, px_int(1LL)), _v1775), _v1767}, 3));
        }
        px_srcline(1008);
         _v1775 = px_add(_v1775, px_int(1LL));
    }
    px_srcline(1009);
    _v1777 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1769}, 2);
    px_srcline(1010);
    (void)(px_method(px_index(_v1767, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("CALLM"), _v1773, _v1771, _v1772, _v1777}, 5)}, 1));
    px_srcline(1011);
    return _v1771;
px_err_1778:
    if (px_err_1778_proped) return px_err_1778_val;
    return px_null();
}

static LXValue fn_bc_emit_push_lambda(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_push_lambda");
    LXValue _v1779 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1780 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1781 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1782 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1783 = px_uninit();
    LXValue _v1784 = px_uninit();
    LXValue _v1785 = px_uninit();
    LXValue _v1786 = px_uninit();
    LXValue _v1787 = px_uninit();
    LXValue _v1788 = px_uninit();
    LXValue _v1789 = px_uninit();
    LXValue _v1790 = px_uninit();
    LXValue _v1791 = px_uninit();
    LXValue _v1792 = px_uninit();
    LXValue _v1793 = px_uninit();
    LXValue _v1794 = px_uninit();
    LXValue px_err_1795_val = px_null();
    int px_err_1795_proped = 0;
    px_srcline(1015);
    _v1783 = px_add(px_add(px_str("<closure"), px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("nclosure"))}, 1)), px_str(">"));
    px_srcline(1016);
    px_index_set(px_get_global("g_bcm"), px_str("nclosure"), px_add(px_index(px_get_global("g_bcm"), px_str("nclosure")), px_int(1LL)));
    px_srcline(1017);
    _v1784 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v1783, px_call(px_get_global("len"), (LXValue[]){_v1779}, 1)}, 2);
    px_srcline(1018);
    _v1785 = px_int(0LL);
    px_srcline(1019);
    while (px_is_truthy(px_lt(_v1785, px_call(px_get_global("len"), (LXValue[]){_v1779}, 1)))) {
        px_srcline(1020);
        px_index_set(px_index(_v1784, px_str("smap")), px_index(_v1779, _v1785), _v1785);
        px_srcline(1022);
        px_index_set(px_index(_v1784, px_str("inited")), px_index(_v1779, _v1785), px_int(1LL));
        px_srcline(1023);
         _v1785 = px_add(_v1785, px_int(1LL));
    }
    px_srcline(1026);
    _v1786 = px_int(0LL);
    px_srcline(1027);
    while (px_is_truthy(px_lt(_v1786, px_call(px_get_global("len"), (LXValue[]){_v1781}, 1)))) {
        px_srcline(1028);
        _v1787 = px_index(_v1781, _v1786);
        px_srcline(1029);
        px_index_set(px_index(_v1784, px_str("smap")), _v1787, px_add(px_call(px_get_global("len"), (LXValue[]){_v1779}, 1), _v1786));
        px_srcline(1030);
        (void)(px_method(px_index(_v1784, px_str("upnames")), "push", (LXValue[]){_v1787}, 1));
        px_srcline(1031);
        (void)(px_call(px_get_global("bc_cell_mark"), (LXValue[]){_v1784, _v1787}, 2));
        px_srcline(1036);
        if (px_is_truthy(({ LXValue _t1796 = px_ne(_v1782, px_null()); px_is_truthy(_t1796) ? px_method(_v1782, "has", (LXValue[]){_v1787}, 1) : _t1796; }))) {
            px_srcline(1037);
            px_index_set(px_index(_v1784, px_str("inited")), _v1787, px_int(1LL));
        }
        px_srcline(1038);
         _v1786 = px_add(_v1786, px_int(1LL));
    }
    px_srcline(1039);
    if (px_is_truthy(px_lt(px_index(_v1784, px_str("next_slot")), ({ LXValue _s209 = px_call(px_get_global("len"), (LXValue[]){_v1779}, 1); LXValue _s210 = px_call(px_get_global("len"), (LXValue[]){_v1781}, 1); px_add(_s209, _s210); })))) {
        px_srcline(1040);
        px_index_set(_v1784, px_str("next_slot"), ({ LXValue _s211 = px_call(px_get_global("len"), (LXValue[]){_v1779}, 1); LXValue _s212 = px_call(px_get_global("len"), (LXValue[]){_v1781}, 1); px_add(_s211, _s212); }));
    }
    px_srcline(1041);
    (void)(px_method(px_index(px_get_global("g_bcm"), px_str("funcs")), "push", (LXValue[]){_v1784}, 1));
    px_srcline(1046);
    _v1788 = px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("funcs"))}, 1), px_int(1LL));
    px_srcline(1050);
    _v1789 = px_list_n((LXValue[]){}, 0);
    px_srcline(1051);
    (void)(px_call(px_get_global("bc_lambda_hoist"), (LXValue[]){_v1780, _v1789}, 2));
    px_srcline(1053);
    _v1790 = px_list_n((LXValue[]){}, 0);
    px_srcline(1054);
    (void)(px_call(px_get_global("bc_lambda_decl"), (LXValue[]){_v1780, _v1790}, 2));
    px_srcline(1055);
    _v1791 = px_int(0LL);
    px_srcline(1056);
    while (px_is_truthy(px_lt(_v1791, px_call(px_get_global("len"), (LXValue[]){_v1789}, 1)))) {
        px_srcline(1057);
        _v1792 = px_index(_v1789, _v1791);
        px_srcline(1066);
        if (px_is_truthy(({ LXValue _t1798 = px_not(px_method(px_index(_v1784, px_str("smap")), "has", (LXValue[]){_v1792}, 1)); px_is_truthy(_t1798) ? px_not(({ LXValue _t1797 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1790, _v1792}, 2)); px_is_truthy(_t1797) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v1792}, 2) : _t1797; })) : _t1798; }))) {
            px_srcline(1067);
            (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v1784, _v1792}, 2));
            px_srcline(1068);
            (void)(px_method(px_index(_v1784, px_str("hoist_slots")), "push", (LXValue[]){px_index(px_index(_v1784, px_str("smap")), _v1792)}, 1));
        }
        px_srcline(1069);
         _v1791 = px_add(_v1791, px_int(1LL));
    }
    px_srcline(1071);
    _v1793 = px_int(0LL);
    px_srcline(1072);
    while (px_is_truthy(px_lt(_v1793, px_call(px_get_global("len"), (LXValue[]){px_index(_v1784, px_str("hoist_slots"))}, 1)))) {
        px_srcline(1073);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1784, px_str("UNINIT"), px_index(px_index(_v1784, px_str("hoist_slots")), _v1793), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1074);
         _v1793 = px_add(_v1793, px_int(1LL));
    }
    px_srcline(1076);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v1784, _v1780}, 2));
    px_srcline(1077);
    _v1794 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1784}, 1);
    px_srcline(1078);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1780, _v1794, _v1784}, 3));
    px_srcline(1079);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1784, px_str("RET"), _v1794, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1080);
    px_index_set(_v1784, px_str("nslots"), px_index(_v1784, px_str("next_slot")));
    px_srcline(1081);
    return _v1788;
px_err_1795:
    if (px_err_1795_proped) return px_err_1795_val;
    return px_null();
}

static LXValue fn_bc_emit_closure(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_closure");
    LXValue _v1799 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1800 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1801 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1802 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1803 = px_uninit();
    LXValue _v1804 = px_uninit();
    LXValue _v1805 = px_uninit();
    LXValue _v1806 = px_uninit();
    LXValue _v1807 = px_uninit();
    LXValue _v1808 = px_uninit();
    LXValue _v1809 = px_uninit();
    LXValue _v1810 = px_uninit();
    LXValue _v1811 = px_uninit();
    LXValue _v1812 = px_uninit();
    LXValue _v1813 = px_uninit();
    LXValue _v1814 = px_uninit();
    LXValue _v1815 = px_uninit();
    LXValue px_err_1816_val = px_null();
    int px_err_1816_proped = 0;
    px_srcline(1086);
    _v1803 = px_list_n((LXValue[]){}, 0);
    px_srcline(1087);
    _v1804 = px_int(0LL);
    px_srcline(1088);
    while (px_is_truthy(px_lt(_v1804, px_call(px_get_global("len"), (LXValue[]){_v1799}, 1)))) {
        px_srcline(1089);
        (void)(px_method(_v1803, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1799, _v1804), px_int(1LL))}, 1)}, 1));
        px_srcline(1090);
         _v1804 = px_add(_v1804, px_int(1LL));
    }
    px_srcline(1091);
    _v1805 = px_call(px_get_global("bc_closure_free"), (LXValue[]){_v1799, _v1800}, 2);
    px_srcline(1092);
    _v1806 = px_list_n((LXValue[]){}, 0);
    px_srcline(1093);
    _v1807 = px_int(0LL);
    px_srcline(1094);
    while (px_is_truthy(px_lt(_v1807, px_call(px_get_global("len"), (LXValue[]){_v1805}, 1)))) {
        px_srcline(1095);
        _v1808 = px_index(_v1805, _v1807);
        px_srcline(1096);
        if (px_is_truthy(px_method(px_index(_v1802, px_str("smap")), "has", (LXValue[]){_v1808}, 1))) {
            px_srcline(1097);
            (void)(px_method(_v1806, "append", (LXValue[]){_v1808}, 1));
        }
        px_srcline(1098);
         _v1807 = px_add(_v1807, px_int(1LL));
    }
    px_srcline(1099);
    _v1809 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){_v1803, _v1800, _v1806, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1802}, 1)}, 4);
    px_srcline(1100);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1806}, 1), px_int(0LL)))) {
        px_srcline(1101);
        _v1810 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1809, px_float(0), px_str("")}, 5);
        px_srcline(1102);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1802, px_str("LOADK"), _v1801, _v1810, px_int(0LL)}, 5));
        px_srcline(1103);
        return _v1801;
    }
    px_srcline(1106);
    _v1811 = px_index(_v1802, px_str("next_slot"));
    px_srcline(1107);
    _v1812 = px_int(0LL);
    px_srcline(1108);
    while (px_is_truthy(px_lt(_v1812, px_call(px_get_global("len"), (LXValue[]){_v1806}, 1)))) {
        px_srcline(1109);
        _v1813 = px_index(px_index(_v1802, px_str("smap")), px_index(_v1806, _v1812));
        px_srcline(1110);
        _v1814 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1802}, 1);
        px_srcline(1111);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1802, px_str("MOV"), _v1814, _v1813, px_int(0LL)}, 5));
        px_srcline(1112);
         _v1812 = px_add(_v1812, px_int(1LL));
    }
    px_srcline(1113);
    _v1815 = _v1801;
    px_srcline(1114);
    if (px_is_truthy(px_lt(_v1815, px_int(0LL)))) {
        px_srcline(1115);
         _v1815 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1802}, 1);
    }
    px_srcline(1116);
    (void)(px_method(px_index(_v1802, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1815, _v1809, _v1811}, 5)}, 1));
    px_srcline(1117);
    return _v1815;
px_err_1816:
    if (px_err_1816_proped) return px_err_1816_val;
    return px_null();
}

static LXValue fn_bc_emit_comp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_comp");
    LXValue _v1817 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1818 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1819 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1820 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1821 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1822 = (nargs > 5) ? args[5] : px_null();
    LXValue _v1823 = (nargs > 6) ? args[6] : px_null();
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
    LXValue _v1844 = px_uninit();
    LXValue _v1845 = px_uninit();
    LXValue _v1846 = px_uninit();
    LXValue _v1847 = px_uninit();
    LXValue _v1848 = px_uninit();
    LXValue _v1849 = px_uninit();
    LXValue _v1850 = px_uninit();
    LXValue _v1851 = px_uninit();
    LXValue _v1852 = px_uninit();
    LXValue px_err_1853_val = px_null();
    int px_err_1853_proped = 0;
    px_srcline(1124);
    _v1824 = px_call(px_get_global("len"), (LXValue[]){_v1822}, 1);
    px_srcline(1125);
    if (px_is_truthy(px_ge(_v1823, _v1824))) {
        px_srcline(1126);
        _v1825 = px_neg(px_int(1LL));
        px_srcline(1127);
        if (px_is_truthy(px_ne(_v1820, px_null()))) {
            px_srcline(1128);
            _v1826 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
            px_srcline(1129);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1820, _v1826, _v1817}, 3));
            px_srcline(1130);
             _v1825 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1817, px_str("bc"))}, 1);
            px_srcline(1131);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("JMPF"), _v1826, px_int(0LL), px_int(0LL)}, 5));
        }
        px_srcline(1132);
        if (px_is_truthy(px_eq(_v1818, px_str("dict")))) {
            px_srcline(1133);
            _v1827 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
            px_srcline(1134);
            _v1828 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
            px_srcline(1135);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1819, px_int(0LL)), _v1827, _v1817}, 3));
            px_srcline(1136);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1819, px_int(1LL)), _v1828, _v1817}, 3));
            px_srcline(1141);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("DICTSET"), _v1821, _v1827, _v1828}, 5));
        }
        else {
            px_srcline(1143);
            _v1829 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
            px_srcline(1144);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1819, px_int(0LL)), _v1829, _v1817}, 3));
            px_srcline(1145);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("LISTPUSH"), _v1829, _v1821, px_int(0LL)}, 5));
        }
        px_srcline(1146);
        if (px_is_truthy(px_ge(_v1825, px_int(0LL)))) {
            px_srcline(1147);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1817, _v1825, px_call(px_get_global("len"), (LXValue[]){px_index(_v1817, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1148);
        return px_null();
    }
    px_srcline(1149);
    _v1830 = px_index(_v1822, _v1823);
    px_srcline(1150);
    _v1831 = px_index(_v1830, px_int(1LL));
    px_srcline(1156);
    _v1832 = px_list_n((LXValue[]){}, 0);
    px_srcline(1157);
    _v1833 = px_int(0LL);
    px_srcline(1158);
    while (px_is_truthy(px_lt(_v1833, px_call(px_get_global("len"), (LXValue[]){_v1831}, 1)))) {
        px_srcline(1159);
        _v1834 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1831, _v1833)}, 1);
        px_srcline(1160);
        _v1835 = px_neg(px_int(1LL));
        px_srcline(1161);
        if (px_is_truthy(px_method(px_index(_v1817, px_str("smap")), "has", (LXValue[]){_v1834}, 1))) {
            px_srcline(1162);
             _v1835 = px_index(px_index(_v1817, px_str("smap")), _v1834);
        }
        px_srcline(1163);
        _v1836 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
        px_srcline(1164);
        px_index_set(px_index(_v1817, px_str("smap")), _v1834, _v1836);
        px_srcline(1165);
        (void)(px_method(_v1832, "append", (LXValue[]){px_list_n((LXValue[]){_v1834, _v1835}, 2)}, 1));
        px_srcline(1166);
         _v1833 = px_add(_v1833, px_int(1LL));
    }
    px_srcline(1167);
    _v1837 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
    px_srcline(1168);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1830, px_int(2LL)), _v1837, _v1817}, 3));
    px_srcline(1169);
    _v1838 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
    px_srcline(1170);
    _v1839 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
    px_srcline(1171);
    while (px_is_truthy(px_lt(px_index(_v1817, px_str("next_slot")), px_add(_v1838, px_int(2LL))))) {
        px_srcline(1172);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1));
    }
    px_srcline(1173);
    _v1840 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("__iter_len")}, 2);
    px_srcline(1174);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("GETG"), _v1838, _v1840, px_int(0LL)}, 5));
    px_srcline(1175);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("MOV"), px_add(_v1838, px_int(1LL)), _v1837, px_int(0LL)}, 5));
    px_srcline(1176);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("CALL"), _v1839, _v1838, px_int(1LL)}, 5));
    px_srcline(1177);
    _v1841 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
    px_srcline(1178);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1817, _v1841, px_int(0LL)}, 3));
    px_srcline(1179);
    _v1842 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1817, px_str("bc"))}, 1);
    px_srcline(1180);
    _v1843 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
    px_srcline(1181);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("LT"), _v1843, _v1841, _v1839}, 5));
    px_srcline(1182);
    _v1844 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1817, px_str("bc"))}, 1);
    px_srcline(1183);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("JMPF"), _v1843, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1185);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("ITERLEN"), _v1837, _v1839, px_int(0LL)}, 5));
    px_srcline(1186);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1831}, 1), px_int(1LL)))) {
        px_srcline(1188);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("ITERAT"), px_index(px_index(_v1817, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1831, px_int(0LL))}, 1)), _v1837, _v1841}, 5));
    }
    else {
        px_srcline(1193);
        _v1845 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
        px_srcline(1194);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("ITERAT"), _v1845, _v1837, _v1841}, 5));
        px_srcline(1195);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("UNPACKCK"), _v1845, px_call(px_get_global("len"), (LXValue[]){_v1831}, 1), px_int(0LL)}, 5));
        px_srcline(1196);
        _v1846 = px_int(0LL);
        px_srcline(1197);
        while (px_is_truthy(px_lt(_v1846, px_call(px_get_global("len"), (LXValue[]){_v1831}, 1)))) {
            px_srcline(1198);
            _v1847 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
            px_srcline(1199);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1817, _v1847, _v1846}, 3));
            px_srcline(1200);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("INDEX"), px_index(px_index(_v1817, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1831, _v1846)}, 1)), _v1845, _v1847}, 5));
            px_srcline(1201);
             _v1846 = px_add(_v1846, px_int(1LL));
        }
    }
    px_srcline(1202);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1817, _v1818, _v1819, _v1820, _v1821, _v1822, px_add(_v1823, px_int(1LL))}, 7));
    px_srcline(1203);
    _v1848 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1817}, 1);
    px_srcline(1204);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v1817, _v1848, px_int(1LL)}, 3));
    px_srcline(1205);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("ADD"), _v1841, _v1841, _v1848}, 5));
    px_srcline(1206);
    _v1849 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1817, px_str("bc"))}, 1);
    px_srcline(1207);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1817, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1208);
    _v1850 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1817, px_str("bc"))}, 1);
    px_srcline(1209);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1817, _v1849, _v1842}, 3));
    px_srcline(1210);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1817, _v1844, _v1850}, 3));
    px_srcline(1212);
    _v1851 = px_int(0LL);
    px_srcline(1213);
    while (px_is_truthy(px_lt(_v1851, px_call(px_get_global("len"), (LXValue[]){_v1832}, 1)))) {
        px_srcline(1214);
        _v1852 = px_index(_v1832, _v1851);
        px_srcline(1215);
        if (px_is_truthy(px_lt(px_index(_v1852, px_int(1LL)), px_int(0LL)))) {
            px_srcline(1216);
            (void)(px_method(px_index(_v1817, px_str("smap")), "remove", (LXValue[]){px_index(_v1852, px_int(0LL))}, 1));
        }
        else {
            px_srcline(1218);
            px_index_set(px_index(_v1817, px_str("smap")), px_index(_v1852, px_int(0LL)), px_index(_v1852, px_int(1LL)));
        }
        px_srcline(1219);
         _v1851 = px_add(_v1851, px_int(1LL));
    }
px_err_1853:
    if (px_err_1853_proped) return px_err_1853_val;
    return px_null();
}

static LXValue fn_bc_emit_caps_snapshot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_caps_snapshot");
    LXValue _v1854 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1855 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1856 = px_uninit();
    LXValue _v1857 = px_uninit();
    LXValue _v1858 = px_uninit();
    LXValue _v1859 = px_uninit();
    LXValue _v1860 = px_uninit();
    LXValue _v1861 = px_uninit();
    LXValue _v1862 = px_uninit();
    LXValue _v1863 = px_uninit();
    LXValue _v1864 = px_uninit();
    LXValue px_err_1865_val = px_null();
    int px_err_1865_proped = 0;
    px_srcline(1227);
    _v1856 = px_call(px_get_global("len"), (LXValue[]){_v1855}, 1);
    px_srcline(1228);
    _v1857 = px_index(_v1854, px_str("next_slot"));
    px_srcline(1229);
    _v1858 = px_int(0LL);
    px_srcline(1230);
    while (px_is_truthy(px_lt(_v1858, _v1856))) {
        px_srcline(1231);
        _v1859 = px_index(_v1855, _v1858);
        px_srcline(1232);
        _v1860 = px_index(px_index(_v1854, px_str("smap")), _v1859);
        px_srcline(1233);
        _v1861 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1854}, 1);
        px_srcline(1234);
        if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1854, _v1859}, 2))) {
            px_srcline(1235);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1854, px_str("CELLGET"), _v1861, _v1860, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1237);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1854, px_str("MOV"), _v1861, _v1860, px_int(0LL)}, 5));
        }
        px_srcline(1238);
         _v1858 = px_add(_v1858, px_int(1LL));
    }
    px_srcline(1239);
    _v1862 = px_index(_v1854, px_str("next_slot"));
    px_srcline(1240);
    _v1863 = px_int(0LL);
    px_srcline(1241);
    while (px_is_truthy(px_lt(_v1863, _v1856))) {
        px_srcline(1242);
        _v1864 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1854}, 1);
        px_srcline(1243);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1854, px_str("CELLNEW"), _v1864, px_add(_v1857, _v1863), px_int(0LL)}, 5));
        px_srcline(1244);
         _v1863 = px_add(_v1863, px_int(1LL));
    }
    px_srcline(1245);
    return _v1862;
px_err_1865:
    if (px_err_1865_proped) return px_err_1865_val;
    return px_null();
}

static LXValue fn_bc_genexp_caps(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_genexp_caps");
    LXValue _v1866 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1867 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1868 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1869 = px_uninit();
    LXValue _v1870 = px_uninit();
    LXValue _v1871 = px_uninit();
    LXValue _v1872 = px_uninit();
    LXValue px_err_1873_val = px_null();
    int px_err_1873_proped = 0;
    px_srcline(1260);
    _v1869 = px_list_n((LXValue[]){}, 0);
    px_srcline(1261);
    (void)(px_call(px_get_global("cg_ast_used"), (LXValue[]){_v1867, _v1869}, 2));
    px_srcline(1262);
    _v1870 = px_list_n((LXValue[]){}, 0);
    px_srcline(1263);
    _v1871 = px_int(0LL);
    px_srcline(1264);
    while (px_is_truthy(px_lt(_v1871, px_call(px_get_global("len"), (LXValue[]){_v1869}, 1)))) {
        px_srcline(1265);
        _v1872 = px_index(_v1869, _v1871);
        px_srcline(1266);
        if (px_is_truthy(({ LXValue _t1874 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v1866, _v1872}, 2)); px_is_truthy(_t1874) ? px_method(px_index(_v1868, px_str("smap")), "has", (LXValue[]){_v1872}, 1) : _t1874; }))) {
            px_srcline(1267);
            (void)(px_method(_v1870, "append", (LXValue[]){_v1872}, 1));
        }
        px_srcline(1268);
         _v1871 = px_add(_v1871, px_int(1LL));
    }
    px_srcline(1269);
    return _v1870;
px_err_1873:
    if (px_err_1873_proped) return px_err_1873_val;
    return px_null();
}

static LXValue fn_bc_emit_genexp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_genexp");
    LXValue _v1875 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1876 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1877 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1878 = px_uninit();
    LXValue _v1879 = px_uninit();
    LXValue _v1880 = px_uninit();
    LXValue _v1881 = px_uninit();
    LXValue _v1882 = px_uninit();
    LXValue _v1883 = px_uninit();
    LXValue _v1884 = px_uninit();
    LXValue _v1885 = px_uninit();
    LXValue _v1886 = px_uninit();
    LXValue _v1887 = px_uninit();
    LXValue _v1888 = px_uninit();
    LXValue _v1889 = px_uninit();
    LXValue _v1890 = px_uninit();
    LXValue px_err_1891_val = px_null();
    int px_err_1891_proped = 0;
    px_srcline(1273);
    _v1878 = px_index(_v1875, px_int(2LL));
    px_srcline(1274);
    if (px_is_truthy(({ LXValue _t1892 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v1878}, 1), px_int(1LL)); px_is_truthy(_t1892) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_index(_v1878, px_int(0LL)), px_int(1LL))}, 1), px_int(1LL)) : _t1892; }))) {
        px_srcline(1277);
        _v1879 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v1878, px_int(0LL)), px_int(1LL)), px_int(0LL))}, 1);
        px_srcline(1278);
        _v1880 = _v1876;
        px_srcline(1279);
        if (px_is_truthy(px_lt(_v1880, px_int(0LL)))) {
            px_srcline(1280);
             _v1880 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1877}, 1);
        }
        px_srcline(1281);
        _v1881 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1877}, 1);
        px_srcline(1282);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v1878, px_int(0LL)), px_int(2LL)), _v1881, _v1877}, 3));
        px_srcline(1288);
        _v1882 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){px_list_n((LXValue[]){_v1879}, 1), px_index(_v1875, px_int(1LL)), _v1877}, 3);
        px_srcline(1289);
        _v1883 = px_list_n((LXValue[]){}, 0);
        px_srcline(1290);
        if (px_is_truthy(px_ne(px_index(_v1875, px_int(3LL)), px_null()))) {
            px_srcline(1291);
             _v1883 = px_call(px_get_global("bc_genexp_caps"), (LXValue[]){px_list_n((LXValue[]){_v1879}, 1), px_index(_v1875, px_int(3LL)), _v1877}, 3);
        }
        px_srcline(1292);
        _v1884 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1879}, 1), px_index(_v1875, px_int(1LL)), _v1882, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1877}, 1)}, 4);
        px_srcline(1293);
        _v1885 = px_neg(px_int(1LL));
        px_srcline(1294);
        if (px_is_truthy(px_ne(px_index(_v1875, px_int(3LL)), px_null()))) {
            px_srcline(1295);
             _v1885 = px_call(px_get_global("bc_emit_push_lambda"), (LXValue[]){px_list_n((LXValue[]){_v1879}, 1), px_index(_v1875, px_int(3LL)), _v1883, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1877}, 1)}, 4);
        }
        px_srcline(1296);
        _v1886 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1877}, 1);
        px_srcline(1297);
        while (px_is_truthy(px_lt(px_index(_v1877, px_str("next_slot")), px_add(_v1886, px_int(2LL))))) {
            px_srcline(1298);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1877}, 1));
        }
        px_srcline(1301);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1882}, 1), px_int(0LL)))) {
            px_srcline(1302);
            _v1887 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1884, px_float(0), px_str("")}, 5);
            px_srcline(1303);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("LOADK"), _v1886, _v1887, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1305);
            _v1888 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1877, _v1882}, 2);
            px_srcline(1306);
            (void)(px_method(px_index(_v1877, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), _v1886, _v1884, _v1888}, 5)}, 1));
        }
        px_srcline(1307);
        if (px_is_truthy(px_ge(_v1885, px_int(0LL)))) {
            px_srcline(1308);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1883}, 1), px_int(0LL)))) {
                px_srcline(1309);
                _v1889 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("func"), _v1885, px_float(0), px_str("")}, 5);
                px_srcline(1310);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("LOADK"), px_add(_v1886, px_int(1LL)), _v1889, px_int(0LL)}, 5));
            }
            else {
                px_srcline(1312);
                _v1890 = px_call(px_get_global("bc_emit_caps_snapshot"), (LXValue[]){_v1877, _v1883}, 2);
                px_srcline(1313);
                (void)(px_method(px_index(_v1877, px_str("bc")), "push", (LXValue[]){px_list_n((LXValue[]){px_str("MKCLO"), px_int(0LL), px_add(_v1886, px_int(1LL)), _v1885, _v1890}, 5)}, 1));
            }
        }
        else {
            px_srcline(1315);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1877, px_add(_v1886, px_int(1LL))}, 2));
        }
        px_srcline(1316);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("NEWGEN"), _v1880, _v1881, _v1886}, 5));
        px_srcline(1317);
        return _v1880;
    }
    px_srcline(1321);
    _v1880 = _v1876;
    px_srcline(1322);
    if (px_is_truthy(px_lt(_v1880, px_int(0LL)))) {
        px_srcline(1323);
         _v1880 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1877}, 1);
    }
    px_srcline(1324);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("NEWLIST"), _v1880, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1325);
    (void)(px_call(px_get_global("bc_emit_comp"), (LXValue[]){_v1877, px_str("push"), px_list_n((LXValue[]){px_index(_v1875, px_int(1LL))}, 1), px_index(_v1875, px_int(3LL)), _v1880, _v1878, px_int(0LL)}, 7));
    px_srcline(1326);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1877, px_str("GENFROMLIST"), _v1880, _v1880, px_int(0LL)}, 5));
    px_srcline(1327);
    return _v1880;
px_err_1891:
    if (px_err_1891_proped) return px_err_1891_val;
    return px_null();
}

static LXValue fn_bc_match_enumvar(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_enumvar");
    LXValue _v1893 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1894 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1895 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1896 = px_uninit();
    LXValue _v1897 = px_uninit();
    LXValue _v1898 = px_uninit();
    LXValue _v1899 = px_uninit();
    LXValue px_err_1900_val = px_null();
    int px_err_1900_proped = 0;
    px_srcline(1333);
    _v1896 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1895}, 1);
    px_srcline(1334);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1895, px_str("ENUMVAR"), _v1896, _v1894, px_int(0LL)}, 5));
    px_srcline(1335);
    _v1897 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1893}, 5);
    px_srcline(1336);
    _v1898 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1895}, 1);
    px_srcline(1337);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1895, px_str("LOADK"), _v1898, _v1897, px_int(0LL)}, 5));
    px_srcline(1338);
    _v1899 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1895}, 1);
    px_srcline(1339);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1895, px_str("EQ"), _v1899, _v1896, _v1898}, 5));
    px_srcline(1340);
    return _v1899;
px_err_1900:
    if (px_err_1900_proped) return px_err_1900_val;
    return px_null();
}

static LXValue fn_bc_match_cond(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_match_cond");
    LXValue _v1901 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1902 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1903 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1904 = px_uninit();
    LXValue _v1905 = px_uninit();
    LXValue _v1906 = px_uninit();
    LXValue _v1907 = px_uninit();
    LXValue _v1908 = px_uninit();
    LXValue px_err_1909_val = px_null();
    int px_err_1909_proped = 0;
    px_srcline(1342);
    _v1904 = px_index(_v1901, px_int(0LL));
    px_srcline(1343);
    if (px_is_truthy(px_eq(_v1904, px_str("PatWildcard")))) {
        px_srcline(1344);
        return px_null();
    }
    px_srcline(1345);
    if (px_is_truthy(px_eq(_v1904, px_str("PatBinding")))) {
        px_srcline(1346);
        _v1905 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1901, px_int(1LL))}, 1);
        px_srcline(1347);
        if (px_is_truthy(({ LXValue _t1911 = ({ LXValue _t1910 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v1905}, 1), px_int(0LL)); px_is_truthy(_t1910) ? px_ge(px_index(_v1905, px_int(0LL)), px_str("A")) : _t1910; }); px_is_truthy(_t1911) ? px_le(px_index(_v1905, px_int(0LL)), px_str("Z")) : _t1911; }))) {
            px_srcline(1349);
            return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){_v1905, _v1902, _v1903}, 3);
        }
        px_srcline(1350);
        return px_null();
    }
    px_srcline(1351);
    if (px_is_truthy(px_eq(_v1904, px_str("PatTuple")))) {
        px_srcline(1352);
        _v1906 = px_index(_v1901, px_int(1LL));
        px_srcline(1353);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v1906}, 1), px_int(0LL)))) {
            px_srcline(1354);
            return px_call(px_get_global("bc_match_cond"), (LXValue[]){px_index(_v1906, px_int(0LL)), _v1902, _v1903}, 3);
        }
        px_srcline(1355);
        return px_null();
    }
    px_srcline(1356);
    if (px_is_truthy(px_eq(_v1904, px_str("PatConstructor")))) {
        px_srcline(1357);
        return px_call(px_get_global("bc_match_enumvar"), (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1901, px_int(1LL))}, 1), _v1902, _v1903}, 3);
    }
    px_srcline(1358);
    if (px_is_truthy(px_eq(_v1904, px_str("PatLiteral")))) {
        px_srcline(1359);
        _v1907 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1903}, 1);
        px_srcline(1360);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1901, px_int(1LL)), _v1907, _v1903}, 3));
        px_srcline(1361);
        _v1908 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1903}, 1);
        px_srcline(1362);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1903, px_str("EQ"), _v1908, _v1902, _v1907}, 5));
        px_srcline(1363);
        return _v1908;
    }
    px_srcline(1364);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_match_cond 未知 pattern: "), px_call(px_get_global("str"), (LXValue[]){_v1901}, 1))}, 1));
px_err_1909:
    if (px_err_1909_proped) return px_err_1909_val;
    return px_null();
}

static LXValue fn_bc_chk_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_chk_slot");
    LXValue _v1912 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1913 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1914 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1915 = px_uninit();
    LXValue px_err_1916_val = px_null();
    int px_err_1916_proped = 0;
    px_srcline(1369);
    if (px_is_truthy(px_not(px_method(px_index(_v1912, px_str("inited")), "has", (LXValue[]){_v1913}, 1)))) {
        px_srcline(1370);
        _v1915 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), _v1913}, 2);
        px_srcline(1371);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1912, px_str("CHKINIT"), _v1914, px_int(0LL), _v1915}, 5));
    }
px_err_1916:
    if (px_err_1916_proped) return px_err_1916_val;
    return px_null();
}

static LXValue fn_bc_assign_local_slot(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_local_slot");
    LXValue _v1917 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1918 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1919 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1920 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1921 = (nargs > 4) ? args[4] : px_null();
    LXValue _v1922 = px_uninit();
    LXValue _v1923 = px_uninit();
    LXValue px_err_1924_val = px_null();
    int px_err_1924_proped = 0;
    px_srcline(1373);
    if (px_is_truthy(px_eq(_v1919, px_str("Assign")))) {
        px_srcline(1380);
        _v1922 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1921}, 1);
        px_srcline(1381);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1920, _v1922, _v1921}, 3));
        px_srcline(1382);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1921, px_str("MOV"), _v1917, _v1922, px_int(0LL)}, 5));
        px_srcline(1383);
        return px_null();
    }
    px_srcline(1384);
    _v1923 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1921}, 1);
    px_srcline(1385);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1920, _v1923, _v1921}, 3));
    px_srcline(1388);
    (void)(px_call(px_get_global("bc_chk_slot"), (LXValue[]){_v1921, _v1918, _v1917}, 3));
    px_srcline(1389);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1921, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1919}, 1)}, 1), _v1917, _v1917, _v1923}, 5));
px_err_1924:
    if (px_err_1924_proped) return px_err_1924_val;
    return px_null();
}

static LXValue fn_bc_assign_global(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_global");
    LXValue _v1925 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1926 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1927 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1928 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1929 = px_uninit();
    LXValue _v1930 = px_uninit();
    LXValue _v1931 = px_uninit();
    LXValue px_err_1932_val = px_null();
    int px_err_1932_proped = 0;
    px_srcline(1391);
    _v1929 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1925}, 2);
    px_srcline(1392);
    if (px_is_truthy(px_eq(_v1926, px_str("Assign")))) {
        px_srcline(1393);
        _v1930 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
        px_srcline(1394);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1927, _v1930, _v1928}, 3));
        px_srcline(1395);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("SETG"), _v1929, _v1930, px_int(0LL)}, 5));
        px_srcline(1396);
        return px_null();
    }
    px_srcline(1397);
    _v1930 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
    px_srcline(1398);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("GETG"), _v1930, _v1929, px_int(0LL)}, 5));
    px_srcline(1399);
    _v1931 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1928}, 1);
    px_srcline(1400);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1927, _v1931, _v1928}, 3));
    px_srcline(1401);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1926}, 1)}, 1), _v1930, _v1930, _v1931}, 5));
    px_srcline(1402);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1928, px_str("SETG"), _v1929, _v1930, px_int(0LL)}, 5));
px_err_1932:
    if (px_err_1932_proped) return px_err_1932_val;
    return px_null();
}

static LXValue fn_bc_assign_index(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_index");
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
    px_srcline(1405);
    _v1937 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
    px_srcline(1406);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1933, px_int(1LL)), _v1937, _v1936}, 3));
    px_srcline(1407);
    _v1938 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
    px_srcline(1408);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1933, px_int(2LL)), _v1938, _v1936}, 3));
    px_srcline(1409);
    _v1939 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
    px_srcline(1410);
    if (px_is_truthy(px_eq(_v1934, px_str("Assign")))) {
        px_srcline(1411);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1935, _v1939, _v1936}, 3));
    }
    else {
        px_srcline(1413);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1936, px_str("INDEX"), _v1939, _v1937, _v1938}, 5));
        px_srcline(1414);
        _v1940 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1936}, 1);
        px_srcline(1415);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1935, _v1940, _v1936}, 3));
        px_srcline(1416);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1936, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1934}, 1)}, 1), _v1939, _v1939, _v1940}, 5));
    }
    px_srcline(1417);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1936, px_str("SETIDX"), _v1939, _v1937, _v1938}, 5));
px_err_1941:
    if (px_err_1941_proped) return px_err_1941_val;
    return px_null();
}

static LXValue fn_bc_assign_field(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_field");
    LXValue _v1942 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1943 = (nargs > 1) ? args[1] : px_null();
    LXValue _v1944 = (nargs > 2) ? args[2] : px_null();
    LXValue _v1945 = (nargs > 3) ? args[3] : px_null();
    LXValue _v1946 = px_uninit();
    LXValue _v1947 = px_uninit();
    LXValue _v1948 = px_uninit();
    LXValue _v1949 = px_uninit();
    LXValue px_err_1950_val = px_null();
    int px_err_1950_proped = 0;
    px_srcline(1420);
    _v1946 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
    px_srcline(1421);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1942, px_int(1LL)), _v1946, _v1945}, 3));
    px_srcline(1422);
    _v1947 = px_call(px_get_global("bc_n_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("n_pool")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1942, px_int(2LL))}, 1)}, 2);
    px_srcline(1423);
    _v1948 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
    px_srcline(1424);
    if (px_is_truthy(px_eq(_v1943, px_str("Assign")))) {
        px_srcline(1425);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1944, _v1948, _v1945}, 3));
    }
    else {
        px_srcline(1427);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("GETF"), _v1948, _v1946, _v1947}, 5));
        px_srcline(1428);
        _v1949 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1945}, 1);
        px_srcline(1429);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1944, _v1949, _v1945}, 3));
        px_srcline(1430);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1943}, 1)}, 1), _v1948, _v1948, _v1949}, 5));
    }
    px_srcline(1431);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1945, px_str("SETF"), _v1948, _v1946, _v1947}, 5));
px_err_1950:
    if (px_err_1950_proped) return px_err_1950_val;
    return px_null();
}

static LXValue fn_bc_assign_op_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_assign_op_name");
    LXValue _v1951 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_1952_val = px_null();
    int px_err_1952_proped = 0;
    px_srcline(1434);
    if (px_is_truthy(px_eq(_v1951, px_str("Plus")))) {
        px_srcline(1435);
        return px_str("Add");
    }
    px_srcline(1436);
    if (px_is_truthy(px_eq(_v1951, px_str("Minus")))) {
        px_srcline(1437);
        return px_str("Sub");
    }
    px_srcline(1438);
    if (px_is_truthy(px_eq(_v1951, px_str("Star")))) {
        px_srcline(1439);
        return px_str("Mul");
    }
    px_srcline(1440);
    if (px_is_truthy(px_eq(_v1951, px_str("Slash")))) {
        px_srcline(1441);
        return px_str("Div");
    }
    px_srcline(1442);
    if (px_is_truthy(px_eq(_v1951, px_str("IntDiv")))) {
        px_srcline(1443);
        return px_str("IntDiv");
    }
    px_srcline(1444);
    if (px_is_truthy(px_eq(_v1951, px_str("Mod")))) {
        px_srcline(1445);
        return px_str("Mod");
    }
    px_srcline(1446);
    if (px_is_truthy(px_eq(_v1951, px_str("Pow")))) {
        px_srcline(1447);
        return px_str("Pow");
    }
    px_srcline(1448);
    if (px_is_truthy(px_eq(_v1951, px_str("BitAnd")))) {
        px_srcline(1449);
        return px_str("BitAnd");
    }
    px_srcline(1450);
    if (px_is_truthy(px_eq(_v1951, px_str("BitOr")))) {
        px_srcline(1451);
        return px_str("BitOr");
    }
    px_srcline(1452);
    if (px_is_truthy(px_eq(_v1951, px_str("BitXor")))) {
        px_srcline(1453);
        return px_str("BitXor");
    }
    px_srcline(1454);
    if (px_is_truthy(px_eq(_v1951, px_str("Shl")))) {
        px_srcline(1455);
        return px_str("Shl");
    }
    px_srcline(1456);
    if (px_is_truthy(px_eq(_v1951, px_str("Shr")))) {
        px_srcline(1457);
        return px_str("Shr");
    }
    px_srcline(1458);
    if (px_is_truthy(px_eq(_v1951, px_str("ShrU")))) {
        px_srcline(1459);
        return px_str("ShrU");
    }
    px_srcline(1460);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_assign_op_name 未知赋值 op: "), px_call(px_get_global("str"), (LXValue[]){_v1951}, 1))}, 1));
px_err_1952:
    if (px_err_1952_proped) return px_err_1952_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt_inner(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt_inner");
    LXValue _v1953 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1954 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v1980 = px_uninit();
    LXValue _v1981 = px_uninit();
    LXValue _v1982 = px_uninit();
    LXValue _v1983 = px_uninit();
    LXValue _v1984 = px_uninit();
    LXValue _v1985 = px_uninit();
    LXValue _v1986 = px_uninit();
    LXValue _v1987 = px_uninit();
    LXValue _v1988 = px_uninit();
    LXValue px_err_1989_val = px_null();
    int px_err_1989_proped = 0;
    px_srcline(1463);
    _v1955 = px_index(_v1953, px_int(0LL));
    px_srcline(1464);
    if (px_is_truthy(px_eq(_v1955, px_str("VarDecl")))) {
        px_srcline(1467);
        (void)(px_call(px_get_global("cg_sem_vardecl"), (LXValue[]){_v1953}, 1));
        px_srcline(1469);
        _v1956 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1953, px_int(2LL))}, 1);
        px_srcline(1470);
        if (px_is_truthy(px_eq(px_index(_v1953, px_int(4LL)), px_null()))) {
            px_srcline(1474);
            if (px_is_truthy(px_index(_v1954, px_str("is_top")))) {
                px_srcline(1475);
                _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
                px_srcline(1476);
                (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1954, _v1957}, 2));
                px_srcline(1477);
                _v1958 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1956}, 2);
                px_srcline(1478);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("SETG"), _v1958, _v1957, px_int(0LL)}, 5));
                px_srcline(1479);
                return px_null();
            }
            px_srcline(1480);
            _v1959 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1954, _v1956}, 2);
            px_srcline(1481);
            _v1960 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
            px_srcline(1482);
            (void)(px_call(px_get_global("bc_emit_null"), (LXValue[]){_v1954, _v1960}, 2));
            px_srcline(1483);
            (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1954, _v1956, _v1959, _v1960}, 4));
            px_srcline(1484);
            px_index_set(px_index(_v1954, px_str("inited")), _v1956, px_int(1LL));
            px_srcline(1485);
            return px_null();
        }
        px_srcline(1486);
        if (px_is_truthy(px_index(_v1954, px_str("is_top")))) {
            px_srcline(1488);
            _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
            px_srcline(1489);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1953, px_int(4LL)), _v1957, _v1954}, 3));
            px_srcline(1490);
            _v1958 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1956}, 2);
            px_srcline(1491);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("SETG"), _v1958, _v1957, px_int(0LL)}, 5));
            px_srcline(1492);
            return px_null();
        }
        px_srcline(1496);
        _v1961 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1954, _v1956}, 2);
        px_srcline(1497);
        _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
        px_srcline(1498);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1953, px_int(4LL)), _v1957, _v1954}, 3));
        px_srcline(1499);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1954, _v1956, _v1961, _v1957}, 4));
        px_srcline(1501);
        px_index_set(px_index(_v1954, px_str("inited")), _v1956, px_int(1LL));
        px_srcline(1502);
        return px_null();
    }
    px_srcline(1503);
    if (px_is_truthy(px_eq(_v1955, px_str("Assign")))) {
        px_srcline(1506);
        _v1962 = px_index(_v1953, px_int(1LL));
        px_srcline(1507);
        _v1963 = px_index(_v1953, px_int(2LL));
        px_srcline(1508);
        _v1964 = px_index(_v1953, px_int(3LL));
        px_srcline(1509);
        if (px_is_truthy(px_eq(_v1963, px_str("Append")))) {
            px_srcline(1510);
            _v1965 = px_list_n((LXValue[]){_v1964}, 1);
            px_srcline(1511);
            (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){_v1962, px_str("append"), _v1965, px_neg(px_int(1LL)), _v1954}, 5));
            px_srcline(1512);
            return px_null();
        }
        px_srcline(1515);
        (void)(px_call(px_get_global("cg_sem_assign"), (LXValue[]){_v1962, _v1963, _v1964}, 3));
        px_srcline(1516);
        _v1966 = px_index(_v1962, px_int(0LL));
        px_srcline(1517);
        if (px_is_truthy(px_eq(_v1966, px_str("Var")))) {
            px_srcline(1518);
            _v1956 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1962, px_int(1LL))}, 1);
            px_srcline(1519);
            if (px_is_truthy(px_method(px_index(_v1954, px_str("smap")), "has", (LXValue[]){_v1956}, 1))) {
                px_srcline(1521);
                if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v1954, _v1956}, 2))) {
                    px_srcline(1522);
                    _v1967 = px_index(px_index(_v1954, px_str("smap")), _v1956);
                    px_srcline(1523);
                    if (px_is_truthy(px_eq(_v1963, px_str("Assign")))) {
                        px_srcline(1524);
                        _v1968 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
                        px_srcline(1525);
                        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1964, _v1968, _v1954}, 3));
                        px_srcline(1526);
                        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("CELLSET"), _v1968, _v1967, px_int(0LL)}, 5));
                        px_srcline(1528);
                        px_index_set(px_index(_v1954, px_str("inited")), _v1956, px_int(1LL));
                        px_srcline(1529);
                        return px_null();
                    }
                    px_srcline(1530);
                    _v1969 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
                    px_srcline(1531);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("CELLGET"), _v1969, _v1967, px_int(0LL)}, 5));
                    px_srcline(1533);
                    (void)(px_call(px_get_global("bc_chk_slot"), (LXValue[]){_v1954, _v1956, _v1969}, 3));
                    px_srcline(1534);
                    _v1970 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
                    px_srcline(1535);
                    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v1964, _v1970, _v1954}, 3));
                    px_srcline(1536);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_call(px_get_global("bc_binop_op"), (LXValue[]){px_call(px_get_global("bc_assign_op_name"), (LXValue[]){_v1963}, 1)}, 1), _v1969, _v1969, _v1970}, 5));
                    px_srcline(1537);
                    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("CELLSET"), _v1969, _v1967, px_int(0LL)}, 5));
                    px_srcline(1539);
                    px_index_set(px_index(_v1954, px_str("inited")), _v1956, px_int(1LL));
                    px_srcline(1540);
                    return px_null();
                }
                px_srcline(1541);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){px_index(px_index(_v1954, px_str("smap")), _v1956), _v1956, _v1963, _v1964, _v1954}, 5));
                px_srcline(1543);
                px_index_set(px_index(_v1954, px_str("inited")), _v1956, px_int(1LL));
                px_srcline(1544);
                return px_null();
            }
            px_srcline(1545);
            if (px_is_truthy(({ LXValue _t1990 = px_not(px_index(_v1954, px_str("is_top"))); px_is_truthy(_t1990) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v1956}, 2)) : _t1990; }))) {
                px_srcline(1548);
                _v1961 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1954, _v1956}, 2);
                px_srcline(1549);
                (void)(px_call(px_get_global("bc_assign_local_slot"), (LXValue[]){_v1961, _v1956, _v1963, _v1964, _v1954}, 5));
                px_srcline(1551);
                px_index_set(px_index(_v1954, px_str("inited")), _v1956, px_int(1LL));
                px_srcline(1552);
                return px_null();
            }
            px_srcline(1554);
            (void)(px_call(px_get_global("bc_assign_global"), (LXValue[]){_v1956, _v1963, _v1964, _v1954}, 4));
            px_srcline(1555);
            return px_null();
        }
        px_srcline(1556);
        if (px_is_truthy(px_eq(_v1966, px_str("Index")))) {
            px_srcline(1557);
            (void)(px_call(px_get_global("bc_assign_index"), (LXValue[]){_v1962, _v1963, _v1964, _v1954}, 4));
            px_srcline(1558);
            return px_null();
        }
        px_srcline(1559);
        if (px_is_truthy(px_eq(_v1966, px_str("Field")))) {
            px_srcline(1560);
            (void)(px_call(px_get_global("bc_assign_field"), (LXValue[]){_v1962, _v1963, _v1964, _v1954}, 4));
            px_srcline(1561);
            return px_null();
        }
        px_srcline(1562);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt Assign 目标未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1962}, 1))}, 1));
    }
    px_srcline(1563);
    if (px_is_truthy(px_eq(_v1955, px_str("Return")))) {
        px_srcline(1564);
        if (px_is_truthy(px_ne(px_index(_v1953, px_int(1LL)), px_null()))) {
            px_srcline(1565);
            _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
            px_srcline(1566);
            (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1953, px_int(1LL)), _v1957, _v1954}, 3));
            px_srcline(1567);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("RET"), _v1957, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1568);
            return px_null();
        }
        px_srcline(1569);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1570);
        return px_null();
    }
    px_srcline(1571);
    if (px_is_truthy(px_eq(_v1955, px_str("ExprStmt")))) {
        px_srcline(1573);
        _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
        px_srcline(1574);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1953, px_int(1LL)), _v1957, _v1954}, 3));
        px_srcline(1575);
        return px_null();
    }
    px_srcline(1576);
    if (px_is_truthy(px_eq(_v1955, px_str("If")))) {
        px_srcline(1577);
        (void)(px_call(px_get_global("bc_emit_if"), (LXValue[]){_v1953, _v1954}, 2));
        px_srcline(1578);
        return px_null();
    }
    px_srcline(1579);
    if (px_is_truthy(px_eq(_v1955, px_str("While")))) {
        px_srcline(1580);
        (void)(px_call(px_get_global("bc_emit_while"), (LXValue[]){_v1953, _v1954}, 2));
        px_srcline(1581);
        return px_null();
    }
    px_srcline(1582);
    if (px_is_truthy(px_eq(_v1955, px_str("For")))) {
        px_srcline(1583);
        (void)(px_call(px_get_global("bc_emit_for"), (LXValue[]){_v1953, _v1954}, 2));
        px_srcline(1584);
        return px_null();
    }
    px_srcline(1585);
    if (px_is_truthy(px_eq(_v1955, px_str("Break")))) {
        px_srcline(1587);
        _v1971 = px_index(_v1954, px_str("loops"));
        px_srcline(1588);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1971}, 1), px_int(0LL)))) {
            px_srcline(1589);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Break 不在循环内")}, 1));
        }
        px_srcline(1590);
        _v1972 = px_index(_v1971, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1971}, 1), px_int(1LL)));
        px_srcline(1591);
        _v1973 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1954, px_str("bc"))}, 1);
        px_srcline(1592);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1593);
        (void)(px_method(px_index(_v1972, px_str("breaks")), "append", (LXValue[]){_v1973}, 1));
        px_srcline(1594);
        return px_null();
    }
    px_srcline(1595);
    if (px_is_truthy(px_eq(_v1955, px_str("Continue")))) {
        px_srcline(1597);
        _v1971 = px_index(_v1954, px_str("loops"));
        px_srcline(1598);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v1971}, 1), px_int(0LL)))) {
            px_srcline(1599);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_str("bc_emit Continue 不在循环内")}, 1));
        }
        px_srcline(1600);
        _v1972 = px_index(_v1971, px_sub(px_call(px_get_global("len"), (LXValue[]){_v1971}, 1), px_int(1LL)));
        px_srcline(1601);
        _v1973 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1954, px_str("bc"))}, 1);
        px_srcline(1602);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1603);
        (void)(px_method(px_index(_v1972, px_str("conts")), "append", (LXValue[]){_v1973}, 1));
        px_srcline(1604);
        return px_null();
    }
    px_srcline(1605);
    if (px_is_truthy(px_eq(_v1955, px_str("Empty")))) {
        px_srcline(1606);
        return px_null();
    }
    px_srcline(1607);
    if (px_is_truthy(px_eq(_v1955, px_str("TypeConst")))) {
        px_srcline(1609);
        return px_null();
    }
    px_srcline(1610);
    if (px_is_truthy(px_eq(_v1955, px_str("ChanDecl")))) {
        px_srcline(1612);
        _v1956 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1953, px_int(1LL))}, 1);
        px_srcline(1613);
        _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
        px_srcline(1614);
        _v1967 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
        px_srcline(1615);
        _v1974 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan")}, 2);
        px_srcline(1616);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("GETG"), _v1967, _v1974, px_int(0LL)}, 5));
        px_srcline(1617);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("CALL"), _v1957, _v1967, px_int(0LL)}, 5));
        px_srcline(1618);
        if (px_is_truthy(px_index(_v1954, px_str("is_top")))) {
            px_srcline(1619);
            _v1958 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1956}, 2);
            px_srcline(1620);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("SETG"), _v1958, _v1957, px_int(0LL)}, 5));
        }
        else {
            px_srcline(1622);
            _v1961 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1954, _v1956}, 2);
            px_srcline(1623);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("MOV"), _v1961, _v1957, px_int(0LL)}, 5));
        }
        px_srcline(1624);
        return px_null();
    }
    px_srcline(1625);
    if (px_is_truthy(px_eq(_v1955, px_str("Send")))) {
        px_srcline(1627);
        _v1975 = px_list_n((LXValue[]){px_index(_v1953, px_int(2LL))}, 1);
        px_srcline(1628);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1953, px_int(1LL)), px_str("send"), _v1975, px_neg(px_int(1LL)), _v1954}, 5));
        px_srcline(1629);
        return px_null();
    }
    px_srcline(1630);
    if (px_is_truthy(px_eq(_v1955, px_str("Recv")))) {
        px_srcline(1632);
        (void)(px_call(px_get_global("bc_emit_methodcall"), (LXValue[]){px_index(_v1953, px_int(1LL)), px_str("recv"), px_list_n((LXValue[]){}, 0), px_neg(px_int(1LL)), _v1954}, 5));
        px_srcline(1633);
        return px_null();
    }
    px_srcline(1634);
    if (px_is_truthy(px_eq(_v1955, px_str("Spawn")))) {
        px_srcline(1637);
        _v1976 = px_index(_v1953, px_int(1LL));
        px_srcline(1638);
        if (px_is_truthy(({ LXValue _t1991 = px_eq(px_index(_v1976, px_int(0LL)), px_str("Call")); px_is_truthy(_t1991) ? px_eq(px_index(px_index(_v1976, px_int(1LL)), px_int(0LL)), px_str("Var")) : _t1991; }))) {
            px_srcline(1639);
            _v1977 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v1976, px_int(1LL)), px_int(1LL))}, 1);
            px_srcline(1640);
            _v1978 = px_index(_v1976, px_int(2LL));
            px_srcline(1641);
            _v1957 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
            px_srcline(1642);
            _v1967 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
            px_srcline(1643);
            _v1979 = px_add(px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v1978}, 1));
            px_srcline(1644);
            _v1980 = px_add(px_add(_v1967, px_int(1LL)), _v1979);
            px_srcline(1645);
            while (px_is_truthy(px_lt(px_index(_v1954, px_str("next_slot")), _v1980))) {
                px_srcline(1646);
                (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1));
            }
            px_srcline(1647);
            _v1974 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("spawn")}, 2);
            px_srcline(1648);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("GETG"), _v1967, _v1974, px_int(0LL)}, 5));
            px_srcline(1649);
            _v1981 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("str"), px_int(0LL), px_float(0), _v1977}, 5);
            px_srcline(1650);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("LOADK"), px_add(_v1967, px_int(1LL)), _v1981, px_int(0LL)}, 5));
            px_srcline(1651);
            _v1982 = px_int(0LL);
            px_srcline(1652);
            while (px_is_truthy(px_lt(_v1982, px_call(px_get_global("len"), (LXValue[]){_v1978}, 1)))) {
                px_srcline(1653);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(_v1978, _v1982), px_add(px_add(_v1967, px_int(2LL)), _v1982), _v1954}, 3));
                px_srcline(1654);
                 _v1982 = px_add(_v1982, px_int(1LL));
            }
            px_srcline(1655);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("CALL"), _v1957, _v1967, _v1979}, 5));
            px_srcline(1656);
            return px_null();
        }
        px_srcline(1661);
        _v1983 = px_index(_v1953, px_int(2LL));
        px_srcline(1662);
        (void)(px_call(px_get_global("print_err"), (LXValue[]){px_add(({ LXValue _s213 = px_add(px_add(px_str("错误: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v1983, px_int(0LL))}, 1)), px_str(":")); LXValue _s214 = px_call(px_get_global("str"), (LXValue[]){px_index(_v1983, px_int(1LL))}, 1); px_add(_s213, _s214); }), px_str(": 语义错误 E2011: spawn 只支持「直接函数调用」：spawn f(args)；匿名函数请先绑定命名函数（def work(): ... 然后 spawn work()），多行匿名函数体见 M118"))}, 1));
        px_srcline(1663);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    px_srcline(1664);
    if (px_is_truthy(px_eq(_v1955, px_str("Select")))) {
        px_srcline(1665);
        (void)(px_call(px_get_global("bc_emit_select"), (LXValue[]){_v1953, _v1954}, 2));
        px_srcline(1666);
        return px_null();
    }
    px_srcline(1667);
    if (px_is_truthy(px_eq(_v1955, px_str("FuncDef")))) {
        px_srcline(1672);
        _v1984 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v1953, px_int(1LL))}, 1);
        px_srcline(1673);
        if (px_is_truthy(px_index(_v1954, px_str("is_top")))) {
            px_srcline(1678);
            _v1985 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
            px_srcline(1679);
            (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1953, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1953, px_int(4LL))}, 2), _v1985, _v1954}, 4));
            px_srcline(1680);
            _v1986 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), _v1984}, 2);
            px_srcline(1681);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1954, px_str("SETG"), _v1986, _v1985, px_int(0LL)}, 5));
            px_srcline(1682);
            return px_null();
        }
        px_srcline(1683);
        _v1987 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1954, _v1984}, 2);
        px_srcline(1684);
        _v1988 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1954}, 1);
        px_srcline(1685);
        (void)(px_call(px_get_global("bc_emit_closure"), (LXValue[]){px_index(_v1953, px_int(2LL)), px_list_n((LXValue[]){px_str("Block"), px_index(_v1953, px_int(4LL))}, 2), _v1988, _v1954}, 4));
        px_srcline(1686);
        (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v1954, _v1984, _v1987, _v1988}, 4));
        px_srcline(1688);
        px_index_set(px_index(_v1954, px_str("inited")), _v1984, px_int(1LL));
        px_srcline(1689);
        return px_null();
    }
    px_srcline(1690);
    (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit_stmt 未实现: "), px_call(px_get_global("str"), (LXValue[]){_v1953}, 1))}, 1));
px_err_1989:
    if (px_err_1989_proped) return px_err_1989_val;
    return px_null();
}

static LXValue fn_bc_emit_select(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_select");
    LXValue _v1992 = (nargs > 0) ? args[0] : px_null();
    LXValue _v1993 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v2011 = px_uninit();
    LXValue _v2012 = px_uninit();
    LXValue _v2013 = px_uninit();
    LXValue _v2014 = px_uninit();
    LXValue _v2015 = px_uninit();
    LXValue _v2016 = px_uninit();
    LXValue _v2017 = px_uninit();
    LXValue _v2018 = px_uninit();
    LXValue _v2019 = px_uninit();
    LXValue px_err_2020_val = px_null();
    int px_err_2020_proped = 0;
    px_srcline(1698);
    _v1994 = px_index(_v1992, px_int(1LL));
    px_srcline(1699);
    _v1995 = px_index(_v1992, px_int(2LL));
    px_srcline(1700);
    _v1996 = px_call(px_get_global("len"), (LXValue[]){_v1994}, 1);
    px_srcline(1703);
    _v1997 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v1993}, 1);
    px_srcline(1705);
    _v1998 = px_list_n((LXValue[]){}, 0);
    px_srcline(1706);
    _v1999 = px_int(0LL);
    px_srcline(1707);
    while (px_is_truthy(px_lt(_v1999, _v1996))) {
        px_srcline(1708);
        _v2000 = px_index(px_index(_v1994, _v1999), px_int(1LL));
        px_srcline(1710);
        if (px_is_truthy(({ LXValue _t2021 = px_ne(px_index(_v2000, px_int(0LL)), px_str("Call")); px_is_truthy(_t2021) ? _t2021 : px_ne(px_index(px_index(_v2000, px_int(1LL)), px_int(0LL)), px_str("Field")); }))) {
            px_srcline(1711);
            (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("bc_emit select case 仅支持 ch.recv(): "), px_call(px_get_global("str"), (LXValue[]){_v2000}, 1))}, 1));
        }
        px_srcline(1712);
        _v2001 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1993}, 1);
        px_srcline(1713);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2000, px_int(1LL)), px_int(1LL)), _v2001, _v1993}, 3));
        px_srcline(1714);
        (void)(px_method(_v1998, "append", (LXValue[]){_v2001}, 1));
        px_srcline(1715);
         _v1999 = px_add(_v1999, px_int(1LL));
    }
    px_srcline(1716);
    _v2002 = px_neg(px_int(1LL));
    px_srcline(1717);
    if (px_is_truthy(px_eq(_v1995, px_null()))) {
        px_srcline(1718);
         _v2002 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1);
    }
    px_srcline(1719);
    _v2003 = px_neg(px_int(1LL));
    px_srcline(1720);
    _v2004 = px_list_n((LXValue[]){}, 0);
    px_srcline(1721);
    _v2005 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("chan_try_recv")}, 2);
    px_srcline(1722);
    _v2006 = px_call(px_get_global("bc_k_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("k_pool")), px_str("null"), px_int(0LL), px_float(0), px_str("")}, 5);
    px_srcline(1723);
     _v1999 = px_int(0LL);
    px_srcline(1724);
    while (px_is_truthy(px_lt(_v1999, _v1996))) {
        px_srcline(1725);
        if (px_is_truthy(px_ge(_v2003, px_int(0LL)))) {
            px_srcline(1726);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1993, _v2003, px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1728);
        _v2007 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1993}, 1);
        px_srcline(1729);
        _v2008 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1993}, 1);
        px_srcline(1730);
        _v2009 = px_add(_v2007, px_int(2LL));
        px_srcline(1731);
        while (px_is_truthy(px_lt(px_index(_v1993, px_str("next_slot")), _v2009))) {
            px_srcline(1732);
            (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v1993}, 1));
        }
        px_srcline(1733);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("GETG"), _v2007, _v2005, px_int(0LL)}, 5));
        px_srcline(1734);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("MOV"), px_add(_v2007, px_int(1LL)), px_index(_v1998, _v1999), px_int(0LL)}, 5));
        px_srcline(1735);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("CALL"), _v2008, _v2007, px_int(1LL)}, 5));
        px_srcline(1737);
        _v2010 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1993}, 1);
        px_srcline(1738);
        _v2011 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v1993}, 1);
        px_srcline(1739);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("LOADK"), _v2011, _v2006, px_int(0LL)}, 5));
        px_srcline(1740);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("EQ"), _v2010, _v2008, _v2011}, 5));
        px_srcline(1741);
        _v2012 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1);
        px_srcline(1742);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("JMPT"), _v2010, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1743);
         _v2003 = _v2012;
        px_srcline(1745);
        _v2013 = px_index(px_index(_v1994, _v1999), px_int(0LL));
        px_srcline(1746);
        if (px_is_truthy(px_ne(_v2013, px_null()))) {
            px_srcline(1747);
            _v2014 = px_call(px_get_global("rust_unescape"), (LXValue[]){_v2013}, 1);
            px_srcline(1748);
            _v2015 = px_call(px_get_global("bc_slot"), (LXValue[]){_v1993, _v2014}, 2);
            px_srcline(1749);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("MOV"), _v2015, _v2008, px_int(0LL)}, 5));
        }
        px_srcline(1750);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(px_index(_v1994, _v1999), px_int(2LL)), _v1993}, 2));
        px_srcline(1751);
        _v2016 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1);
        px_srcline(1752);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1753);
        (void)(px_method(_v2004, "append", (LXValue[]){_v2016}, 1));
        px_srcline(1754);
         _v1999 = px_add(_v1999, px_int(1LL));
    }
    px_srcline(1756);
    if (px_is_truthy(px_ge(_v2003, px_int(0LL)))) {
        px_srcline(1757);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1993, _v2003, px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1)}, 3));
    }
    px_srcline(1758);
    if (px_is_truthy(px_ne(_v1995, px_null()))) {
        px_srcline(1759);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v1995, _v1993}, 2));
    }
    else {
        px_srcline(1761);
        _v2017 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1);
        px_srcline(1762);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v1993, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1763);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1993, _v2017, _v2002}, 3));
    }
    px_srcline(1764);
    _v2018 = px_call(px_get_global("len"), (LXValue[]){px_index(_v1993, px_str("bc"))}, 1);
    px_srcline(1765);
    _v2019 = px_int(0LL);
    px_srcline(1766);
    while (px_is_truthy(px_lt(_v2019, px_call(px_get_global("len"), (LXValue[]){_v2004}, 1)))) {
        px_srcline(1767);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v1993, px_index(_v2004, _v2019), _v2018}, 3));
        px_srcline(1768);
         _v2019 = px_add(_v2019, px_int(1LL));
    }
    px_srcline(1769);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v1993, _v1997}, 2));
px_err_2020:
    if (px_err_2020_proped) return px_err_2020_val;
    return px_null();
}

static LXValue fn_bc_emit_stmt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmt");
    LXValue _v2022 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2023 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2024 = px_uninit();
    LXValue _v2025 = px_uninit();
    LXValue px_err_2026_val = px_null();
    int px_err_2026_proped = 0;
    px_srcline(1772);
    _v2024 = px_int(0LL);
    px_srcline(1773);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v2022}, 1), px_int(0LL)))) {
        px_srcline(1774);
        _v2025 = px_index(_v2022, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2022}, 1), px_int(1LL)));
        px_srcline(1775);
        if (px_is_truthy(({ LXValue _t2028 = ({ LXValue _t2027 = px_eq(px_call(px_get_global("type"), (LXValue[]){_v2025}, 1), px_str("list")); px_is_truthy(_t2027) ? px_ge(px_call(px_get_global("len"), (LXValue[]){_v2025}, 1), px_int(1LL)) : _t2027; }); px_is_truthy(_t2028) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_index(_v2025, px_int(0LL))}, 1), px_str("int")) : _t2028; }))) {
            px_srcline(1776);
             _v2024 = px_index(_v2025, px_int(0LL));
        }
    }
    px_srcline(1777);
    if (px_is_truthy(px_gt(_v2024, px_int(0LL)))) {
        px_srcline(1778);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2023, px_str("SRCLINE"), px_int(0LL), _v2024, px_int(0LL)}, 5));
    }
    px_srcline(1779);
    (void)(px_call(px_get_global("bc_emit_stmt_inner"), (LXValue[]){_v2022, _v2023}, 2));
px_err_2026:
    if (px_err_2026_proped) return px_err_2026_val;
    return px_null();
}

static LXValue fn_bc_emit_stmts(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_stmts");
    LXValue _v2029 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2030 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2031 = px_uninit();
    LXValue px_err_2032_val = px_null();
    int px_err_2032_proped = 0;
    px_srcline(1781);
    _v2031 = px_int(0LL);
    px_srcline(1782);
    while (px_is_truthy(px_lt(_v2031, px_call(px_get_global("len"), (LXValue[]){_v2029}, 1)))) {
        px_srcline(1783);
        (void)(px_call(px_get_global("bc_emit_stmt"), (LXValue[]){px_index(_v2029, _v2031), _v2030}, 2));
        px_srcline(1784);
         _v2031 = px_add(_v2031, px_int(1LL));
    }
px_err_2032:
    if (px_err_2032_proped) return px_err_2032_val;
    return px_null();
}

static LXValue fn_bc_emit_if(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_if");
    LXValue _v2033 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2034 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2035 = px_uninit();
    LXValue _v2036 = px_uninit();
    LXValue _v2037 = px_uninit();
    LXValue _v2038 = px_uninit();
    LXValue _v2039 = px_uninit();
    LXValue _v2040 = px_uninit();
    LXValue _v2041 = px_uninit();
    LXValue _v2042 = px_uninit();
    LXValue _v2043 = px_uninit();
    LXValue _v2044 = px_uninit();
    LXValue _v2045 = px_uninit();
    LXValue _v2046 = px_uninit();
    LXValue _v2047 = px_uninit();
    LXValue _v2048 = px_uninit();
    LXValue px_err_2049_val = px_null();
    int px_err_2049_proped = 0;
    px_srcline(1790);
    _v2035 = px_index(_v2033, px_int(1LL));
    px_srcline(1791);
    _v2036 = px_index(_v2033, px_int(2LL));
    px_srcline(1792);
    _v2037 = px_call(px_get_global("len"), (LXValue[]){_v2035}, 1);
    px_srcline(1795);
    _v2038 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2034}, 1);
    px_srcline(1796);
    _v2039 = px_null();
    px_srcline(1797);
    _v2040 = px_list_n((LXValue[]){}, 0);
    px_srcline(1798);
    _v2041 = px_int(0LL);
    px_srcline(1799);
    while (px_is_truthy(px_lt(_v2041, _v2037))) {
        px_srcline(1800);
        _v2042 = px_index(px_index(_v2035, _v2041), px_int(0LL));
        px_srcline(1801);
        _v2043 = px_index(px_index(_v2035, _v2041), px_int(1LL));
        px_srcline(1802);
        (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2034, _v2038}, 2));
        px_srcline(1803);
        _v2044 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2034}, 1);
        px_srcline(1804);
        (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v2042, _v2044, _v2034}, 3));
        px_srcline(1805);
        _v2045 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2034, px_str("bc"))}, 1);
        px_srcline(1806);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2034, px_str("JMPF"), _v2044, px_int(0LL), px_int(0LL)}, 5));
        px_srcline(1807);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2043, _v2034}, 2));
        px_srcline(1808);
        if (px_is_truthy(px_eq(_v2039, px_null()))) {
            px_srcline(1809);
             _v2039 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2034}, 1);
        }
        else {
            px_srcline(1811);
             _v2039 = px_call(px_get_global("bc_inited_intersect"), (LXValue[]){_v2039, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2034}, 1)}, 2);
        }
        px_srcline(1812);
        if (px_is_truthy(({ LXValue _t2050 = px_lt(_v2041, px_sub(_v2037, px_int(1LL))); px_is_truthy(_t2050) ? _t2050 : px_ne(_v2036, px_null()); }))) {
            px_srcline(1815);
            _v2046 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2034, px_str("bc"))}, 1);
            px_srcline(1816);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2034, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
            px_srcline(1817);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2034, _v2045, px_call(px_get_global("len"), (LXValue[]){px_index(_v2034, px_str("bc"))}, 1)}, 3));
            px_srcline(1818);
            (void)(px_method(_v2040, "append", (LXValue[]){_v2046}, 1));
        }
        else {
            px_srcline(1821);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2034, _v2045, px_call(px_get_global("len"), (LXValue[]){px_index(_v2034, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(1822);
         _v2041 = px_add(_v2041, px_int(1LL));
    }
    px_srcline(1823);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2034, _v2038}, 2));
    px_srcline(1824);
    if (px_is_truthy(px_ne(_v2036, px_null()))) {
        px_srcline(1825);
        (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2036, _v2034}, 2));
        px_srcline(1826);
         _v2039 = px_call(px_get_global("bc_inited_intersect"), (LXValue[]){_v2039, px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2034}, 1)}, 2);
    }
    else {
        px_srcline(1828);
         _v2039 = px_call(px_get_global("bc_inited_intersect"), (LXValue[]){_v2039, _v2038}, 2);
    }
    px_srcline(1829);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2034, _v2039}, 2));
    px_srcline(1830);
    _v2047 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2034, px_str("bc"))}, 1);
    px_srcline(1831);
    _v2048 = px_int(0LL);
    px_srcline(1832);
    while (px_is_truthy(px_lt(_v2048, px_call(px_get_global("len"), (LXValue[]){_v2040}, 1)))) {
        px_srcline(1833);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2034, px_index(_v2040, _v2048), _v2047}, 3));
        px_srcline(1834);
         _v2048 = px_add(_v2048, px_int(1LL));
    }
px_err_2049:
    if (px_err_2049_proped) return px_err_2049_val;
    return px_null();
}

static LXValue fn_bc_emit_while(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_while");
    LXValue _v2051 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2052 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2053 = px_uninit();
    LXValue _v2054 = px_uninit();
    LXValue _v2055 = px_uninit();
    LXValue _v2056 = px_uninit();
    LXValue _v2057 = px_uninit();
    LXValue _v2058 = px_uninit();
    LXValue _v2059 = px_uninit();
    LXValue _v2060 = px_uninit();
    LXValue _v2061 = px_uninit();
    LXValue _v2062 = px_uninit();
    LXValue px_err_2063_val = px_null();
    int px_err_2063_proped = 0;
    px_srcline(1839);
    _v2053 = px_index(_v2051, px_int(1LL));
    px_srcline(1840);
    _v2054 = px_index(_v2051, px_int(2LL));
    px_srcline(1842);
    _v2055 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2052}, 1);
    px_srcline(1843);
    _v2056 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2052, px_str("bc"))}, 1);
    px_srcline(1844);
    _v2057 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2052}, 1);
    px_srcline(1845);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v2053, _v2057, _v2052}, 3));
    px_srcline(1846);
    _v2058 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2052, px_str("bc"))}, 1);
    px_srcline(1847);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2052, px_str("JMPF"), _v2057, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1848);
    _v2059 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1849);
    px_index_set(_v2059, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1850);
    px_index_set(_v2059, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1851);
    (void)(px_method(px_index(_v2052, px_str("loops")), "push", (LXValue[]){_v2059}, 1));
    px_srcline(1852);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2054, _v2052}, 2));
    px_srcline(1853);
    (void)(px_method(px_index(_v2052, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1854);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2052, _v2055}, 2));
    px_srcline(1855);
    _v2060 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2052, px_str("bc"))}, 1);
    px_srcline(1856);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2052, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1857);
    _v2061 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2052, px_str("bc"))}, 1);
    px_srcline(1858);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2052, _v2060, _v2056}, 3));
    px_srcline(1859);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2052, _v2058, _v2061}, 3));
    px_srcline(1860);
    _v2062 = px_int(0LL);
    px_srcline(1861);
    while (px_is_truthy(px_lt(_v2062, px_call(px_get_global("len"), (LXValue[]){px_index(_v2059, px_str("breaks"))}, 1)))) {
        px_srcline(1862);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2052, px_index(px_index(_v2059, px_str("breaks")), _v2062), _v2061}, 3));
        px_srcline(1863);
         _v2062 = px_add(_v2062, px_int(1LL));
    }
    px_srcline(1864);
     _v2062 = px_int(0LL);
    px_srcline(1865);
    while (px_is_truthy(px_lt(_v2062, px_call(px_get_global("len"), (LXValue[]){px_index(_v2059, px_str("conts"))}, 1)))) {
        px_srcline(1866);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2052, px_index(px_index(_v2059, px_str("conts")), _v2062), _v2056}, 3));
        px_srcline(1867);
         _v2062 = px_add(_v2062, px_int(1LL));
    }
px_err_2063:
    if (px_err_2063_proped) return px_err_2063_val;
    return px_null();
}

static LXValue fn_bc_for_names(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_for_names");
    LXValue _v2064 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2065 = px_uninit();
    LXValue _v2066 = px_uninit();
    LXValue px_err_2067_val = px_null();
    int px_err_2067_proped = 0;
    px_srcline(1876);
    if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v2064}, 1), px_str("string")))) {
        px_srcline(1877);
        return px_list_n((LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){_v2064}, 1)}, 1);
    }
    px_srcline(1878);
    _v2065 = px_list_n((LXValue[]){}, 0);
    px_srcline(1879);
    _v2066 = px_int(0LL);
    px_srcline(1880);
    while (px_is_truthy(px_lt(_v2066, px_call(px_get_global("len"), (LXValue[]){_v2064}, 1)))) {
        px_srcline(1881);
        (void)(px_method(_v2065, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2064, _v2066)}, 1)}, 1));
        px_srcline(1882);
         _v2066 = px_add(_v2066, px_int(1LL));
    }
    px_srcline(1883);
    return _v2065;
px_err_2067:
    if (px_err_2067_proped) return px_err_2067_val;
    return px_null();
}

static LXValue fn_bc_emit_for(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_for");
    LXValue _v2068 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2069 = (nargs > 1) ? args[1] : px_null();
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
    LXValue _v2091 = px_uninit();
    LXValue _v2092 = px_uninit();
    LXValue _v2093 = px_uninit();
    LXValue _v2094 = px_uninit();
    LXValue _v2095 = px_uninit();
    LXValue _v2096 = px_uninit();
    LXValue _v2097 = px_uninit();
    LXValue _v2098 = px_uninit();
    LXValue _v2099 = px_uninit();
    LXValue px_err_2100_val = px_null();
    int px_err_2100_proped = 0;
    px_srcline(1885);
    _v2070 = px_call(px_get_global("bc_for_names"), (LXValue[]){px_index(_v2068, px_int(1LL))}, 1);
    px_srcline(1886);
    _v2071 = px_index(_v2068, px_int(2LL));
    px_srcline(1887);
    _v2072 = px_index(_v2068, px_int(3LL));
    px_srcline(1896);
    _v2073 = px_index(_v2069, px_str("is_top"));
    px_srcline(1898);
    _v2074 = px_list_n((LXValue[]){}, 0);
    px_srcline(1899);
    _v2075 = px_int(0LL);
    px_srcline(1900);
    while (px_is_truthy(px_lt(_v2075, px_call(px_get_global("len"), (LXValue[]){_v2070}, 1)))) {
        px_srcline(1901);
        _v2076 = px_neg(px_int(1LL));
        px_srcline(1902);
        if (px_is_truthy(_v2073)) {
            px_srcline(1903);
             _v2076 = px_neg(px_int(1LL));
        }
        else if (px_is_truthy(px_method(px_index(_v2069, px_str("smap")), "has", (LXValue[]){px_index(_v2070, _v2075)}, 1))) {
            px_srcline(1905);
             _v2076 = px_index(px_index(_v2069, px_str("smap")), px_index(_v2070, _v2075));
        }
        else {
            px_srcline(1907);
             _v2076 = px_call(px_get_global("bc_slot"), (LXValue[]){_v2069, px_index(_v2070, _v2075)}, 2);
        }
        px_srcline(1908);
        (void)(px_method(_v2074, "append", (LXValue[]){_v2076}, 1));
        px_srcline(1909);
         _v2075 = px_add(_v2075, px_int(1LL));
    }
    px_srcline(1911);
    _v2077 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
    px_srcline(1912);
    (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){_v2071, _v2077, _v2069}, 3));
    px_srcline(1914);
    _v2078 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
    px_srcline(1915);
    _v2079 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
    px_srcline(1916);
    while (px_is_truthy(px_lt(px_index(_v2069, px_str("next_slot")), px_add(_v2078, px_int(2LL))))) {
        px_srcline(1917);
        (void)(px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1));
    }
    px_srcline(1918);
    _v2080 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("__iter_len")}, 2);
    px_srcline(1919);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("GETG"), _v2078, _v2080, px_int(0LL)}, 5));
    px_srcline(1920);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("MOV"), px_add(_v2078, px_int(1LL)), _v2077, px_int(0LL)}, 5));
    px_srcline(1921);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("CALL"), _v2079, _v2078, px_int(1LL)}, 5));
    px_srcline(1923);
    _v2081 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
    px_srcline(1924);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2069, _v2081, px_int(0LL)}, 3));
    px_srcline(1925);
    _v2082 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2069, px_str("bc"))}, 1);
    px_srcline(1927);
    _v2083 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
    px_srcline(1928);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("LT"), _v2083, _v2081, _v2079}, 5));
    px_srcline(1929);
    _v2084 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2069, px_str("bc"))}, 1);
    px_srcline(1930);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("JMPF"), _v2083, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(1933);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("ITERLEN"), _v2077, _v2079, px_int(0LL)}, 5));
    px_srcline(1936);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v2070}, 1), px_int(1LL)))) {
        px_srcline(1943);
        if (px_is_truthy(_v2073)) {
            px_srcline(1945);
            _v2085 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
            px_srcline(1946);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("ITERAT"), _v2085, _v2077, _v2081}, 5));
            px_srcline(1947);
            _v2086 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_index(_v2070, px_int(0LL))}, 2);
            px_srcline(1948);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("SETG"), _v2086, _v2085, px_int(0LL)}, 5));
        }
        else if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v2069, px_index(_v2070, px_int(0LL))}, 2))) {
            px_srcline(1950);
            _v2085 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
            px_srcline(1951);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("ITERAT"), _v2085, _v2077, _v2081}, 5));
            px_srcline(1952);
            (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v2069, px_index(_v2070, px_int(0LL)), px_index(_v2074, px_int(0LL)), _v2085}, 4));
        }
        else {
            px_srcline(1954);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("ITERAT"), px_index(_v2074, px_int(0LL)), _v2077, _v2081}, 5));
        }
    }
    else {
        px_srcline(1958);
        _v2087 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
        px_srcline(1959);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("ITERAT"), _v2087, _v2077, _v2081}, 5));
        px_srcline(1960);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("UNPACKCK"), _v2087, px_call(px_get_global("len"), (LXValue[]){_v2070}, 1), px_int(0LL)}, 5));
        px_srcline(1961);
        _v2088 = px_int(0LL);
        px_srcline(1962);
        while (px_is_truthy(px_lt(_v2088, px_call(px_get_global("len"), (LXValue[]){_v2070}, 1)))) {
            px_srcline(1963);
            _v2089 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
            px_srcline(1964);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2069, _v2089, _v2088}, 3));
            px_srcline(1965);
            if (px_is_truthy(_v2073)) {
                px_srcline(1967);
                _v2090 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
                px_srcline(1968);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("INDEX"), _v2090, _v2087, _v2089}, 5));
                px_srcline(1969);
                _v2091 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_index(_v2070, _v2088)}, 2);
                px_srcline(1970);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("SETG"), _v2091, _v2090, px_int(0LL)}, 5));
            }
            else if (px_is_truthy(px_call(px_get_global("bc_cell_has"), (LXValue[]){_v2069, px_index(_v2070, _v2088)}, 2))) {
                px_srcline(1973);
                _v2090 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
                px_srcline(1974);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("INDEX"), _v2090, _v2087, _v2089}, 5));
                px_srcline(1975);
                (void)(px_call(px_get_global("bc_store_var"), (LXValue[]){_v2069, px_index(_v2070, _v2088), px_index(_v2074, _v2088), _v2090}, 4));
            }
            else {
                px_srcline(1977);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("INDEX"), px_index(_v2074, _v2088), _v2087, _v2089}, 5));
            }
            px_srcline(1978);
             _v2088 = px_add(_v2088, px_int(1LL));
        }
    }
    px_srcline(1979);
    _v2092 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
    px_srcline(1980);
    px_index_set(_v2092, px_str("breaks"), px_list_n((LXValue[]){}, 0));
    px_srcline(1981);
    px_index_set(_v2092, px_str("conts"), px_list_n((LXValue[]){}, 0));
    px_srcline(1982);
    (void)(px_method(px_index(_v2069, px_str("loops")), "push", (LXValue[]){_v2092}, 1));
    px_srcline(1985);
    _v2093 = px_call(px_get_global("bc_inited_copy"), (LXValue[]){_v2069}, 1);
    px_srcline(1986);
    _v2094 = px_int(0LL);
    px_srcline(1987);
    while (px_is_truthy(px_lt(_v2094, px_call(px_get_global("len"), (LXValue[]){_v2070}, 1)))) {
        px_srcline(1988);
        if (px_is_truthy(px_not(_v2073))) {
            px_srcline(1989);
            px_index_set(px_index(_v2069, px_str("inited")), px_index(_v2070, _v2094), px_int(1LL));
        }
        px_srcline(1990);
         _v2094 = px_add(_v2094, px_int(1LL));
    }
    px_srcline(1991);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2072, _v2069}, 2));
    px_srcline(1992);
    (void)(px_method(px_index(_v2069, px_str("loops")), "pop", (LXValue[]){}, 0));
    px_srcline(1993);
    (void)(px_call(px_get_global("bc_inited_restore"), (LXValue[]){_v2069, _v2093}, 2));
    px_srcline(1998);
    _v2095 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2069, px_str("bc"))}, 1);
    px_srcline(1999);
    _v2096 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2069}, 1);
    px_srcline(2000);
    (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2069, _v2096, px_int(1LL)}, 3));
    px_srcline(2001);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("ADD"), _v2081, _v2081, _v2096}, 5));
    px_srcline(2002);
    _v2097 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2069, px_str("bc"))}, 1);
    px_srcline(2003);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2069, px_str("JMP"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(2004);
    _v2098 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2069, px_str("bc"))}, 1);
    px_srcline(2005);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2069, _v2097, _v2082}, 3));
    px_srcline(2006);
    (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2069, _v2084, _v2098}, 3));
    px_srcline(2007);
    _v2099 = px_int(0LL);
    px_srcline(2008);
    while (px_is_truthy(px_lt(_v2099, px_call(px_get_global("len"), (LXValue[]){px_index(_v2092, px_str("breaks"))}, 1)))) {
        px_srcline(2009);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2069, px_index(px_index(_v2092, px_str("breaks")), _v2099), _v2098}, 3));
        px_srcline(2010);
         _v2099 = px_add(_v2099, px_int(1LL));
    }
    px_srcline(2011);
     _v2099 = px_int(0LL);
    px_srcline(2012);
    while (px_is_truthy(px_lt(_v2099, px_call(px_get_global("len"), (LXValue[]){px_index(_v2092, px_str("conts"))}, 1)))) {
        px_srcline(2013);
        (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2069, px_index(px_index(_v2092, px_str("conts")), _v2099), _v2095}, 3));
        px_srcline(2014);
         _v2099 = px_add(_v2099, px_int(1LL));
    }
px_err_2100:
    if (px_err_2100_proped) return px_err_2100_val;
    return px_null();
}

static LXValue fn_bc_emit_func_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_def");
    LXValue _v2101 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2102 = px_uninit();
    LXValue px_err_2103_val = px_null();
    int px_err_2103_proped = 0;
    px_srcline(2018);
    _v2102 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2101, px_int(1LL))}, 1);
    px_srcline(2019);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v2101, _v2102}, 2);
px_err_2103:
    if (px_err_2103_proped) return px_err_2103_val;
    return px_null();
}

static LXValue fn_bc_emit_impl_def(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_impl_def");
    LXValue _v2104 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2105 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2106 = px_uninit();
    LXValue px_err_2107_val = px_null();
    int px_err_2107_proped = 0;
    px_srcline(2022);
    _v2106 = px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2104, px_int(1LL))}, 1);
    px_srcline(2023);
    return px_call(px_get_global("bc_emit_func_body"), (LXValue[]){_v2104, px_add(px_add(_v2105, px_str(".")), _v2106)}, 2);
px_err_2107:
    if (px_err_2107_proped) return px_err_2107_val;
    return px_null();
}

static LXValue fn_bc_emit_func_body(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_body");
    LXValue _v2108 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2109 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2110 = px_uninit();
    LXValue _v2111 = px_uninit();
    LXValue _v2112 = px_uninit();
    LXValue _v2113 = px_uninit();
    LXValue _v2114 = px_uninit();
    LXValue _v2115 = px_uninit();
    LXValue _v2116 = px_uninit();
    LXValue _v2117 = px_uninit();
    LXValue _v2118 = px_uninit();
    LXValue _v2119 = px_uninit();
    LXValue _v2120 = px_uninit();
    LXValue px_err_2121_val = px_null();
    int px_err_2121_proped = 0;
    px_srcline(2032);
    _v2110 = px_call(px_get_global("cg_dict_copy"), (LXValue[]){px_get_global("cg_immutables")}, 1);
    px_srcline(2033);
    px_set_global("cg_immutables", px_call(px_get_global("cg_dict_copy"), (LXValue[]){_v2110}, 1));
    px_srcline(2034);
    _v2111 = px_index(_v2108, px_int(2LL));
    px_srcline(2039);
    _v2112 = px_call(px_get_global("len"), (LXValue[]){_v2111}, 1);
    px_srcline(2040);
    _v2113 = px_int(0LL);
    px_srcline(2041);
    _v2114 = px_int(0LL);
    px_srcline(2042);
    while (px_is_truthy(px_lt(_v2114, px_call(px_get_global("len"), (LXValue[]){_v2111}, 1)))) {
        px_srcline(2043);
        if (px_is_truthy(px_ne(px_index(px_index(_v2111, _v2114), px_int(3LL)), px_null()))) {
            px_srcline(2044);
            if (px_is_truthy(px_eq(_v2113, px_int(0LL)))) {
                px_srcline(2045);
                 _v2112 = _v2114;
            }
            px_srcline(2046);
             _v2113 = px_add(_v2113, px_int(1LL));
        }
        px_srcline(2047);
         _v2114 = px_add(_v2114, px_int(1LL));
    }
    px_srcline(2048);
    _v2115 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v2109, _v2112}, 2);
    px_srcline(2049);
    px_index_set(_v2115, px_str("ndefault"), _v2113);
    px_srcline(2051);
     _v2114 = px_int(0LL);
    px_srcline(2052);
    while (px_is_truthy(px_lt(_v2114, px_call(px_get_global("len"), (LXValue[]){_v2111}, 1)))) {
        px_srcline(2053);
        px_index_set(px_index(_v2115, px_str("smap")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2111, _v2114), px_int(1LL))}, 1), _v2114);
        px_srcline(2055);
        px_index_set(px_index(_v2115, px_str("inited")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2111, _v2114), px_int(1LL))}, 1), px_int(1LL));
        px_srcline(2056);
         _v2114 = px_add(_v2114, px_int(1LL));
    }
    px_srcline(2057);
    if (px_is_truthy(px_lt(px_index(_v2115, px_str("next_slot")), px_call(px_get_global("len"), (LXValue[]){_v2111}, 1)))) {
        px_srcline(2058);
        px_index_set(_v2115, px_str("next_slot"), px_call(px_get_global("len"), (LXValue[]){_v2111}, 1));
    }
    px_srcline(2060);
    _v2116 = px_list_n((LXValue[]){}, 0);
    px_srcline(2061);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){px_index(_v2108, px_int(4LL)), _v2116}, 2));
    px_srcline(2063);
    _v2117 = px_list_n((LXValue[]){}, 0);
    px_srcline(2064);
    (void)(px_call(px_get_global("bc_collect_decl_vars"), (LXValue[]){px_index(_v2108, px_int(4LL)), _v2117}, 2));
    px_srcline(2065);
    _v2118 = px_int(0LL);
    px_srcline(2066);
    while (px_is_truthy(px_lt(_v2118, px_call(px_get_global("len"), (LXValue[]){_v2116}, 1)))) {
        px_srcline(2067);
        _v2119 = px_index(_v2116, _v2118);
        px_srcline(2068);
        if (px_is_truthy(px_method(px_index(_v2115, px_str("smap")), "has", (LXValue[]){_v2119}, 1))) {
            px_srcline(2069);
             _v2118 = px_add(_v2118, px_int(1LL));
            px_srcline(2070);
            continue;
        }
        px_srcline(2075);
        if (px_is_truthy(({ LXValue _t2122 = px_not(px_call(px_get_global("contains"), (LXValue[]){_v2117, _v2119}, 2)); px_is_truthy(_t2122) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("g_topnames"), _v2119}, 2) : _t2122; }))) {
            px_srcline(2076);
             _v2118 = px_add(_v2118, px_int(1LL));
            px_srcline(2077);
            continue;
        }
        px_srcline(2078);
        (void)(px_call(px_get_global("bc_slot"), (LXValue[]){_v2115, _v2119}, 2));
        px_srcline(2079);
        (void)(px_method(px_index(_v2115, px_str("hoist_slots")), "push", (LXValue[]){px_index(px_index(_v2115, px_str("smap")), _v2119)}, 1));
        px_srcline(2080);
         _v2118 = px_add(_v2118, px_int(1LL));
    }
    px_srcline(2084);
    _v2120 = px_int(0LL);
    px_srcline(2085);
    while (px_is_truthy(px_lt(_v2120, px_call(px_get_global("len"), (LXValue[]){px_index(_v2115, px_str("hoist_slots"))}, 1)))) {
        px_srcline(2086);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2115, px_str("UNINIT"), px_index(px_index(_v2115, px_str("hoist_slots")), _v2120), px_int(0LL), px_int(0LL)}, 5));
        px_srcline(2087);
         _v2120 = px_add(_v2120, px_int(1LL));
    }
    px_srcline(2092);
    (void)(px_call(px_get_global("bc_box_frame"), (LXValue[]){_v2115, px_index(_v2108, px_int(4LL))}, 2));
    px_srcline(2094);
    if (px_is_truthy(px_gt(_v2113, px_int(0LL)))) {
        px_srcline(2095);
        (void)(px_call(px_get_global("bc_emit_default_fill"), (LXValue[]){_v2115, _v2111, _v2112}, 3));
    }
    px_srcline(2096);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){px_index(_v2108, px_int(4LL)), _v2115}, 2));
    px_srcline(2097);
    px_set_global("cg_immutables", _v2110);
    px_srcline(2098);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2115, px_str("RET0"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    px_srcline(2099);
    px_index_set(_v2115, px_str("nslots"), px_index(_v2115, px_str("next_slot")));
    px_srcline(2100);
    return _v2115;
px_err_2121:
    if (px_err_2121_proped) return px_err_2121_val;
    return px_null();
}

static LXValue fn_bc_emit_default_fill(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_default_fill");
    LXValue _v2123 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2124 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2125 = (nargs > 2) ? args[2] : px_null();
    LXValue _v2126 = px_uninit();
    LXValue _v2127 = px_uninit();
    LXValue _v2128 = px_uninit();
    LXValue _v2129 = px_uninit();
    LXValue _v2130 = px_uninit();
    LXValue _v2131 = px_uninit();
    LXValue px_err_2132_val = px_null();
    int px_err_2132_proped = 0;
    px_srcline(2106);
    _v2126 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2123}, 1);
    px_srcline(2107);
    (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2123, px_str("NARGS"), _v2126, px_int(0LL), px_int(0LL)}, 5));
    px_srcline(2108);
    _v2127 = _v2125;
    px_srcline(2109);
    while (px_is_truthy(px_lt(_v2127, px_call(px_get_global("len"), (LXValue[]){_v2124}, 1)))) {
        px_srcline(2110);
        if (px_is_truthy(px_ne(px_index(px_index(_v2124, _v2127), px_int(3LL)), px_null()))) {
            px_srcline(2111);
            _v2128 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2123}, 1);
            px_srcline(2112);
            (void)(px_call(px_get_global("bc_emit_int"), (LXValue[]){_v2123, _v2128, _v2127}, 3));
            px_srcline(2113);
            _v2129 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2123}, 1);
            px_srcline(2114);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2123, px_str("LT"), _v2129, _v2128, _v2126}, 5));
            px_srcline(2115);
            _v2130 = px_call(px_get_global("len"), (LXValue[]){px_index(_v2123, px_str("bc"))}, 1);
            px_srcline(2116);
            (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2123, px_str("JMPT"), _v2129, px_int(0LL), px_int(0LL)}, 5));
            px_srcline(2119);
            if (px_is_truthy(px_method(px_index(_v2123, px_str("cell")), "has", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(_v2124, _v2127), px_int(1LL))}, 1)}, 1))) {
                px_srcline(2120);
                _v2131 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2123}, 1);
                px_srcline(2121);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2124, _v2127), px_int(3LL)), _v2131, _v2123}, 3));
                px_srcline(2122);
                (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2123, px_str("CELLSET"), _v2131, _v2127, px_int(0LL)}, 5));
            }
            else {
                px_srcline(2124);
                (void)(px_call(px_get_global("bc_emit_expr"), (LXValue[]){px_index(px_index(_v2124, _v2127), px_int(3LL)), _v2127, _v2123}, 3));
            }
            px_srcline(2125);
            (void)(px_call(px_get_global("bc_patch_off"), (LXValue[]){_v2123, _v2130, px_call(px_get_global("len"), (LXValue[]){px_index(_v2123, px_str("bc"))}, 1)}, 3));
        }
        px_srcline(2126);
         _v2127 = px_add(_v2127, px_int(1LL));
    }
px_err_2132:
    if (px_err_2132_proped) return px_err_2132_val;
    return px_null();
}

static LXValue fn_bc_emit_func_top(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_func_top");
    LXValue _v2133 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2134 = (nargs > 1) ? args[1] : px_null();
    LXValue _v2135 = (nargs > 2) ? args[2] : px_null();
    LXValue _v2136 = px_uninit();
    LXValue _v2137 = px_uninit();
    LXValue _v2138 = px_uninit();
    LXValue _v2139 = px_uninit();
    LXValue px_err_2140_val = px_null();
    int px_err_2140_proped = 0;
    px_srcline(2131);
    _v2136 = px_call(px_get_global("bc_new_func"), (LXValue[]){_v2134, px_int(0LL)}, 2);
    px_srcline(2132);
    px_index_set(_v2136, px_str("is_top"), px_bool(true));
    px_srcline(2133);
    (void)(px_call(px_get_global("bc_emit_stmts"), (LXValue[]){_v2133, _v2136}, 2));
    px_srcline(2134);
    if (px_is_truthy(_v2135)) {
        px_srcline(2135);
        _v2137 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2136}, 1);
        px_srcline(2136);
        _v2138 = px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(px_get_global("g_bcm"), px_str("globals")), px_str("main")}, 2);
        px_srcline(2137);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2136, px_str("GETG"), _v2137, _v2138, px_int(0LL)}, 5));
        px_srcline(2138);
        _v2139 = px_call(px_get_global("bc_tmp"), (LXValue[]){_v2136}, 1);
        px_srcline(2139);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2136, px_str("CALL"), _v2139, _v2137, px_int(0LL)}, 5));
        px_srcline(2140);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2136, px_str("RET"), _v2139, px_int(0LL), px_int(0LL)}, 5));
    }
    else {
        px_srcline(2142);
        (void)(px_call(px_get_global("bc_emit_inst"), (LXValue[]){_v2136, px_str("HALT"), px_int(0LL), px_int(0LL), px_int(0LL)}, 5));
    }
    px_srcline(2143);
    px_index_set(_v2136, px_str("nslots"), px_index(_v2136, px_str("next_slot")));
    px_srcline(2144);
    return _v2136;
px_err_2140:
    if (px_err_2140_proped) return px_err_2140_val;
    return px_null();
}

static LXValue fn_bc_emit_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_program");
    LXValue _v2141 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2142 = px_uninit();
    LXValue _v2143 = px_uninit();
    LXValue _v2144 = px_uninit();
    LXValue _v2145 = px_uninit();
    LXValue _v2146 = px_uninit();
    LXValue _v2147 = px_uninit();
    LXValue _v2148 = px_uninit();
    LXValue _v2149 = px_uninit();
    LXValue _v2150 = px_uninit();
    LXValue _v2151 = px_uninit();
    LXValue _v2152 = px_uninit();
    LXValue _v2153 = px_uninit();
    LXValue _v2154 = px_uninit();
    LXValue _v2155 = px_uninit();
    LXValue _v2156 = px_uninit();
    LXValue _v2157 = px_uninit();
    LXValue _v2158 = px_uninit();
    LXValue px_err_2159_val = px_null();
    int px_err_2159_proped = 0;
    px_srcline(2148);
    _v2142 = px_index(_v2141, px_int(1LL));
    px_srcline(2149);
    _v2143 = px_call(px_get_global("bc_new_module"), (LXValue[]){px_str("<module>")}, 1);
    px_srcline(2150);
    px_set_global("g_bcm", _v2143);
    px_srcline(2153);
    _v2144 = px_int(0LL);
    px_srcline(2154);
    while (px_is_truthy(px_lt(_v2144, px_call(px_get_global("len"), (LXValue[]){_v2142}, 1)))) {
        px_srcline(2155);
        _v2145 = px_index(_v2142, _v2144);
        px_srcline(2156);
        _v2146 = px_index(_v2145, px_int(0LL));
        px_srcline(2157);
        if (px_is_truthy(px_eq(_v2146, px_str("StructDef")))) {
            px_srcline(2158);
            _v2147 = px_call(px_get_global("bc_new_dict"), (LXValue[]){}, 0);
            px_srcline(2159);
            px_index_set(_v2147, px_str("name"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2145, px_int(1LL))}, 1));
            px_srcline(2160);
            _v2148 = px_list_n((LXValue[]){}, 0);
            px_srcline(2161);
            _v2149 = px_int(0LL);
            px_srcline(2162);
            while (px_is_truthy(px_lt(_v2149, px_call(px_get_global("len"), (LXValue[]){px_index(_v2145, px_int(2LL))}, 1)))) {
                px_srcline(2163);
                (void)(px_method(_v2148, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v2145, px_int(2LL)), _v2149), px_int(1LL))}, 1)}, 1));
                px_srcline(2164);
                 _v2149 = px_add(_v2149, px_int(1LL));
            }
            px_srcline(2165);
            px_index_set(_v2147, px_str("fnames"), _v2148);
            px_srcline(2166);
            (void)(px_method(px_index(_v2143, px_str("structs")), "push", (LXValue[]){_v2147}, 1));
        }
        else if (px_is_truthy(px_eq(_v2146, px_str("EnumDef")))) {
            px_srcline(2168);
            _v2150 = px_list_n((LXValue[]){}, 0);
            px_srcline(2169);
            _v2151 = px_int(0LL);
            px_srcline(2170);
            while (px_is_truthy(px_lt(_v2151, px_call(px_get_global("len"), (LXValue[]){px_index(_v2145, px_int(2LL))}, 1)))) {
                px_srcline(2171);
                (void)(px_method(_v2150, "append", (LXValue[]){px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(px_index(px_index(_v2145, px_int(2LL)), _v2151), px_int(1LL))}, 1)}, 1));
                px_srcline(2172);
                 _v2151 = px_add(_v2151, px_int(1LL));
            }
            px_srcline(2173);
            px_index_set(px_index(_v2143, px_str("enums")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2145, px_int(1LL))}, 1), _v2150);
        }
        px_srcline(2174);
         _v2144 = px_add(_v2144, px_int(1LL));
    }
    px_srcline(2175);
    (void)(px_call(px_get_global("bc_collect_consts"), (LXValue[]){_v2142}, 1));
    px_srcline(2176);
    _v2152 = px_call(px_get_global("bc_collect_impl_list"), (LXValue[]){_v2142}, 1);
    px_srcline(2183);
    px_set_global("cg_structs", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2184);
    px_set_global("cg_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2185);
    px_set_global("cg_impls", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2186);
    px_set_global("cg_vars", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2187);
    px_set_global("cg_var_types", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2188);
    px_set_global("cg_immutables", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2189);
    px_set_global("cg_nonnull", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2190);
    px_set_global("cg_ffi", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2191);
    px_set_global("cg_const_enums", px_call(px_get_global("cg_new_dict"), (LXValue[]){}, 0));
    px_srcline(2192);
    px_set_global("cg_globals", px_list_n((LXValue[]){}, 0));
    px_srcline(2194);
     _v2144 = px_int(0LL);
    px_srcline(2195);
    while (px_is_truthy(px_lt(_v2144, px_call(px_get_global("len"), (LXValue[]){_v2142}, 1)))) {
        px_srcline(2196);
        _v2145 = px_index(_v2142, _v2144);
        px_srcline(2197);
        _v2146 = px_index(_v2145, px_int(0LL));
        px_srcline(2198);
        if (px_is_truthy(px_eq(_v2146, px_str("FuncDef")))) {
            px_srcline(2199);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2143, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2145, px_int(1LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v2146, px_str("ExternDef")))) {
            px_srcline(2206);
            px_index_set(px_get_global("cg_ffi"), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2145, px_int(1LL))}, 1), px_index(_v2145, px_int(2LL)));
        }
        else if (px_is_truthy(px_eq(_v2146, px_str("VarDecl")))) {
            px_srcline(2208);
            (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2143, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2145, px_int(2LL))}, 1)}, 2));
        }
        else if (px_is_truthy(px_eq(_v2146, px_str("Assign")))) {
            px_srcline(2210);
            _v2153 = px_index(_v2145, px_int(1LL));
            px_srcline(2211);
            if (px_is_truthy(px_eq(px_index(_v2153, px_int(0LL)), px_str("Var")))) {
                px_srcline(2212);
                (void)(px_call(px_get_global("bc_g_add"), (LXValue[]){px_index(_v2143, px_str("globals")), px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2153, px_int(1LL))}, 1)}, 2));
            }
        }
        px_srcline(2213);
         _v2144 = px_add(_v2144, px_int(1LL));
    }
    px_srcline(2219);
    px_set_global("g_topnames", px_list_n((LXValue[]){}, 0));
    px_srcline(2220);
    (void)(px_call(px_get_global("bc_collect_hoist"), (LXValue[]){_v2142, px_get_global("g_topnames")}, 2));
    px_srcline(2223);
     _v2144 = px_int(0LL);
    px_srcline(2224);
    while (px_is_truthy(px_lt(_v2144, px_call(px_get_global("len"), (LXValue[]){_v2152}, 1)))) {
        px_srcline(2225);
        _v2154 = px_index(_v2152, _v2144);
        px_srcline(2226);
        _v2155 = px_call(px_get_global("bc_emit_impl_def"), (LXValue[]){px_index(_v2154, px_int(1LL)), px_index(_v2154, px_int(0LL))}, 2);
        px_srcline(2227);
        (void)(px_method(px_index(_v2143, px_str("funcs")), "push", (LXValue[]){_v2155}, 1));
        px_srcline(2228);
         _v2144 = px_add(_v2144, px_int(1LL));
    }
    px_srcline(2229);
     _v2144 = px_int(0LL);
    px_srcline(2230);
    while (px_is_truthy(px_lt(_v2144, px_call(px_get_global("len"), (LXValue[]){_v2142}, 1)))) {
        px_srcline(2231);
        _v2145 = px_index(_v2142, _v2144);
        px_srcline(2232);
        if (px_is_truthy(px_eq(px_index(_v2145, px_int(0LL)), px_str("FuncDef")))) {
            px_srcline(2233);
            _v2155 = px_call(px_get_global("bc_emit_func_def"), (LXValue[]){_v2145}, 1);
            px_srcline(2234);
            (void)(px_method(px_index(_v2143, px_str("funcs")), "push", (LXValue[]){_v2155}, 1));
        }
        px_srcline(2235);
         _v2144 = px_add(_v2144, px_int(1LL));
    }
    px_srcline(2237);
    _v2156 = px_list_n((LXValue[]){}, 0);
    px_srcline(2238);
     _v2144 = px_int(0LL);
    px_srcline(2239);
    while (px_is_truthy(px_lt(_v2144, px_call(px_get_global("len"), (LXValue[]){_v2142}, 1)))) {
        px_srcline(2240);
        _v2145 = px_index(_v2142, _v2144);
        px_srcline(2241);
        _v2146 = px_index(_v2145, px_int(0LL));
        px_srcline(2242);
        if (px_is_truthy(({ LXValue _t2166 = ({ LXValue _t2165 = ({ LXValue _t2164 = ({ LXValue _t2163 = ({ LXValue _t2162 = ({ LXValue _t2161 = ({ LXValue _t2160 = px_ne(_v2146, px_str("FuncDef")); px_is_truthy(_t2160) ? px_ne(_v2146, px_str("StructDef")) : _t2160; }); px_is_truthy(_t2161) ? px_ne(_v2146, px_str("EnumDef")) : _t2161; }); px_is_truthy(_t2162) ? px_ne(_v2146, px_str("TraitDef")) : _t2162; }); px_is_truthy(_t2163) ? px_ne(_v2146, px_str("ImplDef")) : _t2163; }); px_is_truthy(_t2164) ? px_ne(_v2146, px_str("Import")) : _t2164; }); px_is_truthy(_t2165) ? px_ne(_v2146, px_str("ExternDef")) : _t2165; }); px_is_truthy(_t2166) ? px_ne(_v2146, px_str("TypeConst")) : _t2166; }))) {
            px_srcline(2243);
            (void)(px_method(_v2156, "append", (LXValue[]){_v2145}, 1));
        }
        px_srcline(2244);
         _v2144 = px_add(_v2144, px_int(1LL));
    }
    px_srcline(2246);
    _v2157 = px_bool(false);
    px_srcline(2247);
     _v2144 = px_int(0LL);
    px_srcline(2248);
    while (px_is_truthy(px_lt(_v2144, px_call(px_get_global("len"), (LXValue[]){_v2142}, 1)))) {
        px_srcline(2249);
        _v2145 = px_index(_v2142, _v2144);
        px_srcline(2250);
        if (px_is_truthy(({ LXValue _t2167 = px_eq(px_index(_v2145, px_int(0LL)), px_str("FuncDef")); px_is_truthy(_t2167) ? px_eq(px_call(px_get_global("rust_unescape"), (LXValue[]){px_index(_v2145, px_int(1LL))}, 1), px_str("main")) : _t2167; }))) {
            px_srcline(2251);
             _v2157 = px_bool(true);
        }
        px_srcline(2252);
         _v2144 = px_add(_v2144, px_int(1LL));
    }
    px_srcline(2253);
    _v2158 = px_call(px_get_global("bc_emit_func_top"), (LXValue[]){_v2156, px_str("<top>"), _v2157}, 3);
    px_srcline(2254);
    (void)(px_method(px_index(_v2143, px_str("funcs")), "push", (LXValue[]){_v2158}, 1));
    px_srcline(2255);
    px_index_set(_v2143, px_str("top"), px_sub(px_call(px_get_global("len"), (LXValue[]){px_index(_v2143, px_str("funcs"))}, 1), px_int(1LL)));
    px_srcline(2256);
    return _v2143;
px_err_2159:
    if (px_err_2159_proped) return px_err_2159_val;
    return px_null();
}

static LXValue fn_bc_dump_module(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_dump_module");
    LXValue _v2168 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2169 = px_uninit();
    LXValue _v2170 = px_uninit();
    LXValue _v2171 = px_uninit();
    LXValue _v2172 = px_uninit();
    LXValue _v2173 = px_uninit();
    LXValue _v2174 = px_uninit();
    LXValue _v2175 = px_uninit();
    LXValue _v2176 = px_uninit();
    LXValue _v2177 = px_uninit();
    LXValue px_err_2178_val = px_null();
    int px_err_2178_proped = 0;
    px_srcline(2259);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# BCModule "), px_index(_v2168, px_str("name")))}, 1));
    px_srcline(2260);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# K "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2168, px_str("k_pool"))}, 1)}, 1))}, 1));
    px_srcline(2261);
    _v2169 = px_int(0LL);
    px_srcline(2262);
    while (px_is_truthy(px_lt(_v2169, px_call(px_get_global("len"), (LXValue[]){px_index(_v2168, px_str("k_pool"))}, 1)))) {
        px_srcline(2263);
        _v2170 = px_index(px_index(_v2168, px_str("k_pool")), _v2169);
        px_srcline(2264);
        if (px_is_truthy(px_eq(px_index(_v2170, px_str("kind")), px_str("int")))) {
            px_srcline(2265);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s215 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2169}, 1)), px_str(" int ")); LXValue _s216 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2170, px_str("i"))}, 1); px_add(_s215, _s216); })}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2170, px_str("kind")), px_str("float")))) {
            px_srcline(2267);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s217 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2169}, 1)), px_str(" float ")); LXValue _s218 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2170, px_str("f"))}, 1); px_add(_s217, _s218); })}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2170, px_str("kind")), px_str("str")))) {
            px_srcline(2269);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s219 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2169}, 1)), px_str(" str \"")); LXValue _s220 = px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2170, px_str("s"))}, 1); px_add(_s219, _s220); }), px_str("\""))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2170, px_str("kind")), px_str("bool")))) {
            px_srcline(2271);
            _v2171 = px_str("false");
            px_srcline(2272);
            if (px_is_truthy(px_ne(px_index(_v2170, px_str("i")), px_int(0LL)))) {
                px_srcline(2273);
                 _v2171 = px_str("true");
            }
            px_srcline(2274);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2169}, 1)), px_str(" bool ")), _v2171)}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2170, px_str("kind")), px_str("func")))) {
            px_srcline(2276);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s221 = px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2169}, 1)), px_str(" func ")); LXValue _s222 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2170, px_str("i"))}, 1); px_add(_s221, _s222); })}, 1));
        }
        else {
            px_srcline(2278);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("K "), px_call(px_get_global("str"), (LXValue[]){_v2169}, 1)), px_str(" null"))}, 1));
        }
        px_srcline(2279);
         _v2169 = px_add(_v2169, px_int(1LL));
    }
    px_srcline(2280);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("# G "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2168, px_str("globals"))}, 1)}, 1))}, 1));
    px_srcline(2281);
    _v2172 = px_int(0LL);
    px_srcline(2282);
    while (px_is_truthy(px_lt(_v2172, px_call(px_get_global("len"), (LXValue[]){px_index(_v2168, px_str("globals"))}, 1)))) {
        px_srcline(2283);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("G "), px_call(px_get_global("str"), (LXValue[]){_v2172}, 1)), px_str(" ")), px_index(px_index(_v2168, px_str("globals")), _v2172))}, 1));
        px_srcline(2284);
         _v2172 = px_add(_v2172, px_int(1LL));
    }
    px_srcline(2285);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s223 = px_add(px_add(px_str("# funcs "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2168, px_str("funcs"))}, 1)}, 1)), px_str(" top=")); LXValue _s224 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2168, px_str("top"))}, 1); px_add(_s223, _s224); })}, 1));
    px_srcline(2286);
    _v2173 = px_int(0LL);
    px_srcline(2287);
    while (px_is_truthy(px_lt(_v2173, px_call(px_get_global("len"), (LXValue[]){px_index(_v2168, px_str("funcs"))}, 1)))) {
        px_srcline(2288);
        _v2174 = px_index(px_index(_v2168, px_str("funcs")), _v2173);
        px_srcline(2289);
        _v2175 = px_str("");
        px_srcline(2290);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2174, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2291);
             _v2175 = px_add(({ LXValue _s225 = px_add(px_add(px_str(" up="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2174, px_str("upnames"))}, 1)}, 1)), px_str("[")); LXValue _s226 = px_call(px_get_global("join"), (LXValue[]){px_str(","), px_index(_v2174, px_str("upnames"))}, 2); px_add(_s225, _s226); }), px_str("]"));
        }
        px_srcline(2292);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s229 = px_add(({ LXValue _s227 = px_add(px_add(px_add(px_add(px_str("== func "), px_index(_v2174, px_str("name"))), px_str(" arity=")), px_call(px_get_global("str"), (LXValue[]){px_index(_v2174, px_str("arity"))}, 1)), px_str(" ndefault=")); LXValue _s228 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2174, px_str("ndefault"))}, 1); px_add(_s227, _s228); }), px_str(" nslots=")); LXValue _s230 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2174, px_str("nslots"))}, 1); px_add(_s229, _s230); }), _v2175)}, 1));
        px_srcline(2293);
        _v2176 = px_int(0LL);
        px_srcline(2294);
        while (px_is_truthy(px_lt(_v2176, px_call(px_get_global("len"), (LXValue[]){px_index(_v2174, px_str("bc"))}, 1)))) {
            px_srcline(2295);
            _v2177 = px_index(px_index(_v2174, px_str("bc")), _v2176);
            px_srcline(2296);
            (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s235 = px_add(({ LXValue _s233 = px_add(({ LXValue _s231 = px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v2176}, 1), px_str(":")), px_index(_v2177, px_int(0LL))), px_str(" ")); LXValue _s232 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2177, px_int(2LL))}, 1); px_add(_s231, _s232); }), px_str(" ")); LXValue _s234 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2177, px_int(3LL))}, 1); px_add(_s233, _s234); }), px_str(" ")); LXValue _s236 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2177, px_int(4LL))}, 1); px_add(_s235, _s236); })}, 1));
            px_srcline(2297);
             _v2176 = px_add(_v2176, px_int(1LL));
        }
        px_srcline(2298);
         _v2173 = px_add(_v2173, px_int(1LL));
    }
px_err_2178:
    if (px_err_2178_proped) return px_err_2178_val;
    return px_null();
}

static LXValue fn_bc_emit_c_program(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_emit_c_program");
    LXValue _v2179 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2180 = px_uninit();
    LXValue _v2181 = px_uninit();
    LXValue _v2182 = px_uninit();
    LXValue _v2183 = px_uninit();
    LXValue _v2184 = px_uninit();
    LXValue _v2185 = px_uninit();
    LXValue _v2186 = px_uninit();
    LXValue _v2187 = px_uninit();
    LXValue _v2188 = px_uninit();
    LXValue _v2189 = px_uninit();
    LXValue _v2190 = px_uninit();
    LXValue _v2191 = px_uninit();
    LXValue _v2192 = px_uninit();
    LXValue _v2193 = px_uninit();
    LXValue _v2194 = px_uninit();
    LXValue _v2195 = px_uninit();
    LXValue _v2196 = px_uninit();
    LXValue px_err_2197_val = px_null();
    int px_err_2197_proped = 0;
    px_srcline(2305);
    _v2180 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2179}, 1);
    px_srcline(2310);
    _v2181 = px_list_n((LXValue[]){px_str("/* 由 bc_emit.px (M89-S3) 自动生成 — 字节码模块（VM 执行） */\n")}, 1);
    px_srcline(2311);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("#include \"runtime.h\"\n#include \"vm.h\"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n")}, 1));
    px_srcline(2312);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const PxBCModule s_mod;\n\n")}, 1));
    px_srcline(2314);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const PxK s_K[] = {\n")}, 1));
    px_srcline(2315);
    _v2182 = px_int(0LL);
    px_srcline(2316);
    while (px_is_truthy(px_lt(_v2182, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("k_pool"))}, 1)))) {
        px_srcline(2317);
        _v2183 = px_index(px_index(_v2180, px_str("k_pool")), _v2182);
        px_srcline(2318);
        if (px_is_truthy(px_eq(px_index(_v2183, px_str("kind")), px_str("int")))) {
            px_srcline(2319);
            (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_INT, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v2183, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2183, px_str("kind")), px_str("float")))) {
            px_srcline(2321);
            (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FLT, 0, "), px_call(px_get_global("cg_fmt_float"), (LXValue[]){px_index(_v2183, px_str("f"))}, 1)), px_str(", NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2183, px_str("kind")), px_str("str")))) {
            px_srcline(2324);
            if (px_is_truthy(px_call(px_get_global("cg_has_nul"), (LXValue[]){px_index(_v2183, px_str("s"))}, 1))) {
                px_srcline(2325);
                (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    PXK_STR_LIT(\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2183, px_str("s"))}, 1)), px_str("\"),\n"))}, 1));
            }
            else {
                px_srcline(2327);
                (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_STR, 0, 0, \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2183, px_str("s"))}, 1)), px_str("\"},\n"))}, 1));
            }
        }
        else if (px_is_truthy(px_eq(px_index(_v2183, px_str("kind")), px_str("bool")))) {
            px_srcline(2329);
            _v2184 = px_str("0");
            px_srcline(2330);
            if (px_is_truthy(px_ne(px_index(_v2183, px_str("i")), px_int(0LL)))) {
                px_srcline(2331);
                 _v2184 = px_str("1");
            }
            px_srcline(2332);
            (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_BOOL, "), _v2184), px_str(", 0, NULL},\n"))}, 1));
        }
        else if (px_is_truthy(px_eq(px_index(_v2183, px_str("kind")), px_str("func")))) {
            px_srcline(2334);
            (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    {PXK_FUNC, "), px_call(px_get_global("str"), (LXValue[]){px_index(_v2183, px_str("i"))}, 1)), px_str(", 0, NULL},\n"))}, 1));
        }
        else {
            px_srcline(2336);
            (void)(px_method(_v2181, "push", (LXValue[]){px_str("    {PXK_NULL, 0, 0, NULL},\n")}, 1));
        }
        px_srcline(2337);
         _v2182 = px_add(_v2182, px_int(1LL));
    }
    px_srcline(2338);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2340);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const char* s_N[] = {\n")}, 1));
    px_srcline(2341);
    _v2185 = px_int(0LL);
    px_srcline(2342);
    while (px_is_truthy(px_lt(_v2185, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("n_pool"))}, 1)))) {
        px_srcline(2343);
        (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2180, px_str("n_pool")), _v2185)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2344);
         _v2185 = px_add(_v2185, px_int(1LL));
    }
    px_srcline(2345);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2347);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const char* s_G[] = {\n")}, 1));
    px_srcline(2348);
    _v2186 = px_int(0LL);
    px_srcline(2349);
    while (px_is_truthy(px_lt(_v2186, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("globals"))}, 1)))) {
        px_srcline(2350);
        (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2180, px_str("globals")), _v2186)}, 1)), px_str("\",\n"))}, 1));
        px_srcline(2351);
         _v2186 = px_add(_v2186, px_int(1LL));
    }
    px_srcline(2352);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2354);
    _v2187 = px_int(0LL);
    px_srcline(2355);
    while (px_is_truthy(px_lt(_v2187, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("structs"))}, 1)))) {
        px_srcline(2356);
        _v2188 = px_index(px_index(_v2180, px_str("structs")), _v2187);
        px_srcline(2357);
        (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("static const char* s_st_"), px_call(px_get_global("str"), (LXValue[]){_v2187}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2358);
        _v2189 = px_int(0LL);
        px_srcline(2359);
        while (px_is_truthy(px_lt(_v2189, px_call(px_get_global("len"), (LXValue[]){px_index(_v2188, px_str("fnames"))}, 1)))) {
            px_srcline(2360);
            (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    \""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2188, px_str("fnames")), _v2189)}, 1)), px_str("\",\n"))}, 1));
            px_srcline(2361);
             _v2189 = px_add(_v2189, px_int(1LL));
        }
        px_srcline(2362);
        (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2363);
         _v2187 = px_add(_v2187, px_int(1LL));
    }
    px_srcline(2364);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const PxStructDef s_structs[] = {\n")}, 1));
    px_srcline(2365);
     _v2187 = px_int(0LL);
    px_srcline(2366);
    while (px_is_truthy(px_lt(_v2187, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("structs"))}, 1)))) {
        px_srcline(2367);
        _v2188 = px_index(px_index(_v2180, px_str("structs")), _v2187);
        px_srcline(2368);
        (void)(px_method(_v2181, "push", (LXValue[]){px_add(({ LXValue _s239 = px_add(({ LXValue _s237 = px_add(px_add(px_str("    {\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2188, px_str("name"))}, 1)), px_str("\", s_st_")); LXValue _s238 = px_call(px_get_global("str"), (LXValue[]){_v2187}, 1); px_add(_s237, _s238); }), px_str(", ")); LXValue _s240 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2188, px_str("fnames"))}, 1)}, 1); px_add(_s239, _s240); }), px_str("},\n"))}, 1));
        px_srcline(2369);
         _v2187 = px_add(_v2187, px_int(1LL));
    }
    px_srcline(2370);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2372);
    _v2190 = px_int(0LL);
    px_srcline(2373);
    while (px_is_truthy(px_lt(_v2190, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("funcs"))}, 1)))) {
        px_srcline(2374);
        _v2191 = px_index(px_index(_v2180, px_str("funcs")), _v2190);
        px_srcline(2375);
        (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("static const PxInst s_bc_"), px_call(px_get_global("str"), (LXValue[]){_v2190}, 1)), px_str("[] = {\n"))}, 1));
        px_srcline(2376);
        _v2192 = px_int(0LL);
        px_srcline(2377);
        while (px_is_truthy(px_lt(_v2192, px_call(px_get_global("len"), (LXValue[]){px_index(_v2191, px_str("bc"))}, 1)))) {
            px_srcline(2378);
            _v2193 = px_index(px_index(_v2191, px_str("bc")), _v2192);
            px_srcline(2384);
            (void)(px_method(_v2181, "push", (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str(""), ({ LXValue _s241 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2193, px_int(1LL))}, 1); LXValue _s242 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2193, px_int(2LL))}, 1); LXValue _s243 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2193, px_int(3LL))}, 1); LXValue _s244 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2193, px_int(4LL))}, 1); px_list_n((LXValue[]){px_str("    {PXOP_"), px_index(_v2193, px_int(0LL)), px_str(", "), _s241, px_str(", "), _s242, px_str(", "), _s243, px_str(", "), _s244, px_str("},\n")}, 11); })}, 2)}, 1));
            px_srcline(2385);
             _v2192 = px_add(_v2192, px_int(1LL));
        }
        px_srcline(2386);
        (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n")}, 1));
        px_srcline(2387);
         _v2190 = px_add(_v2190, px_int(1LL));
    }
    px_srcline(2388);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("\n")}, 1));
    px_srcline(2390);
     _v2190 = px_int(0LL);
    px_srcline(2391);
    while (px_is_truthy(px_lt(_v2190, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("funcs"))}, 1)))) {
        px_srcline(2392);
        _v2191 = px_index(px_index(_v2180, px_str("funcs")), _v2190);
        px_srcline(2393);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2191, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2394);
            _v2194 = px_list_n((LXValue[]){}, 0);
            px_srcline(2395);
            _v2195 = px_int(0LL);
            px_srcline(2396);
            while (px_is_truthy(px_lt(_v2195, px_call(px_get_global("len"), (LXValue[]){px_index(_v2191, px_str("upnames"))}, 1)))) {
                px_srcline(2397);
                (void)(px_method(_v2194, "push", (LXValue[]){px_add(px_add(px_str("\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(px_index(_v2191, px_str("upnames")), _v2195)}, 1)), px_str("\""))}, 1));
                px_srcline(2398);
                 _v2195 = px_add(_v2195, px_int(1LL));
            }
            px_srcline(2399);
            (void)(px_method(_v2181, "push", (LXValue[]){px_add(({ LXValue _s245 = px_add(px_add(px_str("static const char* s_up_"), px_call(px_get_global("str"), (LXValue[]){_v2190}, 1)), px_str("[] = {")); LXValue _s246 = px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v2194}, 2); px_add(_s245, _s246); }), px_str("};\n"))}, 1));
        }
        px_srcline(2400);
         _v2190 = px_add(_v2190, px_int(1LL));
    }
    px_srcline(2402);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const PxVMFunc s_funcs[] = {\n")}, 1));
    px_srcline(2403);
     _v2190 = px_int(0LL);
    px_srcline(2404);
    while (px_is_truthy(px_lt(_v2190, px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("funcs"))}, 1)))) {
        px_srcline(2405);
        _v2191 = px_index(px_index(_v2180, px_str("funcs")), _v2190);
        px_srcline(2406);
        _v2196 = px_str(".upvals=NULL");
        px_srcline(2407);
        if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v2191, px_str("upnames"))}, 1), px_int(0LL)))) {
            px_srcline(2408);
             _v2196 = ({ LXValue _s247 = px_add(px_add(px_str(".upvals=s_up_"), px_call(px_get_global("str"), (LXValue[]){_v2190}, 1)), px_str(", .nup=")); LXValue _s248 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2191, px_str("upnames"))}, 1)}, 1); px_add(_s247, _s248); });
        }
        px_srcline(2409);
        (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_add(({ LXValue _s257 = px_add(({ LXValue _s255 = px_add(({ LXValue _s253 = px_add(({ LXValue _s251 = px_add(({ LXValue _s249 = px_add(px_add(px_str("    {.name=\""), px_call(px_get_global("cg_escape_str"), (LXValue[]){px_index(_v2191, px_str("name"))}, 1)), px_str("\", .arity=")); LXValue _s250 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2191, px_str("arity"))}, 1); px_add(_s249, _s250); }), px_str(", .ndefault=")); LXValue _s252 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2191, px_str("ndefault"))}, 1); px_add(_s251, _s252); }), px_str(", .nslots=")); LXValue _s254 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2191, px_str("nslots"))}, 1); px_add(_s253, _s254); }), px_str(", .bc=s_bc_")); LXValue _s256 = px_call(px_get_global("str"), (LXValue[]){_v2190}, 1); px_add(_s255, _s256); }), px_str(", .nbc=")); LXValue _s258 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2191, px_str("bc"))}, 1)}, 1); px_add(_s257, _s258); }), px_str(", .mod=&s_mod, ")), _v2196), px_str("},\n"))}, 1));
        px_srcline(2410);
         _v2190 = px_add(_v2190, px_int(1LL));
    }
    px_srcline(2411);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2413);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("static const PxBCModule s_mod = {\n")}, 1));
    px_srcline(2414);
    (void)(px_method(_v2181, "push", (LXValue[]){px_add(({ LXValue _s259 = px_add(px_add(px_str("    .name=\"<module>\", .K=s_K, .nK="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("k_pool"))}, 1)}, 1)), px_str(", .N=s_N, .nN=")); LXValue _s260 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("n_pool"))}, 1)}, 1); px_add(_s259, _s260); }), px_str(",\n"))}, 1));
    px_srcline(2415);
    (void)(px_method(_v2181, "push", (LXValue[]){px_add(({ LXValue _s263 = px_add(({ LXValue _s261 = px_add(px_add(px_str("    .G=s_G, .nG="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("globals"))}, 1)}, 1)), px_str(", .funcs=s_funcs, .nfuncs=")); LXValue _s262 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("funcs"))}, 1)}, 1); px_add(_s261, _s262); }), px_str(", .top_idx=")); LXValue _s264 = px_call(px_get_global("str"), (LXValue[]){px_index(_v2180, px_str("top"))}, 1); px_add(_s263, _s264); }), px_str(",\n"))}, 1));
    px_srcline(2416);
    (void)(px_method(_v2181, "push", (LXValue[]){px_add(px_add(px_str("    .structs=s_structs, .nstructs="), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_index(_v2180, px_str("structs"))}, 1)}, 1)), px_str(",\n"))}, 1));
    px_srcline(2417);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("};\n\n")}, 1));
    px_srcline(2419);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("int main(int argc, char** argv) {\n")}, 1));
    px_srcline(2420);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    px_args_init(argc, argv);\n")}, 1));
    px_srcline(2421);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    px_register_builtins();\n")}, 1));
    px_srcline(2422);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    px_gc_set_precise(1);   // M92-S2d：VM 轨产物默认 precise 精确根面（退役保守栈扫描；C 轨逃生舱产物不插此调用 → conservative）\n")}, 1));
    px_srcline(2423);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    LXValue _r = px_vm_run_module(px_vm_state(), &s_mod);\n")}, 1));
    px_srcline(2424);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    int _code = 0;\n")}, 1));
    px_srcline(2425);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    if (px_is_result(_r)) {\n")}, 1));
    px_srcline(2426);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        if (!px_result_ok(_r)) {\n")}, 1));
    px_srcline(2427);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("            fprintf(stderr, \"错误: %s\\n\", px_to_string(px_result_unwrap(_r)));\n")}, 1));
    px_srcline(2428);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("            _code = 1;\n")}, 1));
    px_srcline(2429);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        } else {\n")}, 1));
    px_srcline(2430);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("            LXValue _uv = px_result_unwrap(_r);\n")}, 1));
    px_srcline(2431);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("            if (_uv.type == PX_INT) _code = (int)_uv.as.i;\n")}, 1));
    px_srcline(2432);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2433);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    } else if (_r.type == PX_INT) {\n")}, 1));
    px_srcline(2434);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        _code = (int)_r.as.i;\n")}, 1));
    px_srcline(2435);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2436);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    if (getenv(\"PX_BC_DUMP\")) {\n")}, 1));
    px_srcline(2437);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        int _i;\n")}, 1));
    px_srcline(2438);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        for (_i = 0; _i < (int)s_mod.nG; _i++) {\n")}, 1));
    px_srcline(2439);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("            LXValue _v = px_get_global(s_mod.G[_i]);\n")}, 1));
    px_srcline(2440);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("            if (_v.type != PX_FUNC && _v.type != PX_NATIVE)\n")}, 1));
    px_srcline(2441);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("                printf(\"%s=%s\\n\", s_mod.G[_i], px_to_string(_v));\n")}, 1));
    px_srcline(2442);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("        }\n")}, 1));
    px_srcline(2443);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    }\n")}, 1));
    px_srcline(2444);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("    return px_exit_code_final(_code);   // M120（qg-issue 76 E1）：协程隔离错误 → 退出码非 0\n")}, 1));
    px_srcline(2445);
    (void)(px_method(_v2181, "push", (LXValue[]){px_str("}\n")}, 1));
    px_srcline(2446);
    return px_call(px_get_global("join"), (LXValue[]){px_str(""), _v2181}, 2);
px_err_2197:
    if (px_err_2197_proped) return px_err_2197_val;
    return px_null();
}

static LXValue fn_bc_basename(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("bc_basename");
    LXValue _v2198 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2199 = px_uninit();
    LXValue _v2200 = px_uninit();
    LXValue px_err_2201_val = px_null();
    int px_err_2201_proped = 0;
    px_srcline(74);
    _v2199 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2198}, 1), px_int(1LL));
    px_srcline(75);
    while (px_is_truthy(px_ge(_v2199, px_int(0LL)))) {
        px_srcline(76);
        if (px_is_truthy(px_eq(px_index(_v2198, _v2199), px_str("/")))) {
            px_srcline(77);
            _v2200 = px_slice(_v2198, px_add(_v2199, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v2198}, 1), px_null());
            px_srcline(78);
            if (px_is_truthy(({ LXValue _t2202 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2200}, 1), px_int(3LL)); px_is_truthy(_t2202) ? px_eq(({ LXValue _s265 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2200}, 1), px_int(3LL)); LXValue _s266 = px_call(px_get_global("len"), (LXValue[]){_v2200}, 1); px_slice(_v2200, _s265, _s266, px_null()); }), px_str(".px")) : _t2202; }))) {
                px_srcline(79);
                return px_slice(_v2200, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2200}, 1), px_int(3LL)), px_null());
            }
            px_srcline(80);
            return _v2200;
        }
        px_srcline(81);
         _v2199 = px_sub(_v2199, px_int(1LL));
    }
    px_srcline(82);
    if (px_is_truthy(({ LXValue _t2203 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v2198}, 1), px_int(3LL)); px_is_truthy(_t2203) ? px_eq(({ LXValue _s267 = px_sub(px_call(px_get_global("len"), (LXValue[]){_v2198}, 1), px_int(3LL)); LXValue _s268 = px_call(px_get_global("len"), (LXValue[]){_v2198}, 1); px_slice(_v2198, _s267, _s268, px_null()); }), px_str(".px")) : _t2203; }))) {
        px_srcline(83);
        return px_slice(_v2198, px_int(0LL), px_sub(px_call(px_get_global("len"), (LXValue[]){_v2198}, 1), px_int(3LL)), px_null());
    }
    px_srcline(84);
    return _v2198;
px_err_2201:
    if (px_err_2201_proped) return px_err_2201_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v2204 = px_uninit();
    LXValue _v2205 = px_uninit();
    LXValue _v2206 = px_uninit();
    LXValue _v2207 = px_uninit();
    LXValue _v2208 = px_uninit();
    LXValue _v2209 = px_uninit();
    LXValue _v2210 = px_uninit();
    LXValue _v2211 = px_uninit();
    LXValue _v2212 = px_uninit();
    LXValue _v2213 = px_uninit();
    LXValue _v2214 = px_uninit();
    LXValue px_err_2215_val = px_null();
    int px_err_2215_proped = 0;
    px_srcline(87);
    _v2204 = px_call(px_get_global("args"), (LXValue[]){}, 0);
    px_srcline(89);
    if (px_is_truthy(({ LXValue _t2217 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v2204}, 1), px_int(2LL)); px_is_truthy(_t2217) ? ({ LXValue _t2216 = px_eq(px_index(_v2204, px_int(1LL)), px_str("--version")); px_is_truthy(_t2216) ? _t2216 : px_eq(px_index(_v2204, px_int(1LL)), px_str("-v")); }) : _t2217; }))) {
        px_srcline(90);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("pxc "), px_get_global("PXC_VER")), px_str(" (普贤 PuXian · selfhosted ")), px_get_global("PXC_MS")), px_str(")"))}, 1));
        px_srcline(91);
        return px_int(0LL);
    }
    px_srcline(94);
    _v2205 = px_bool(false);
    px_srcline(95);
    _v2206 = px_bool(false);
    px_srcline(96);
    _v2207 = px_int(1LL);
    px_srcline(97);
    while (px_is_truthy(px_lt(_v2207, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2204}, 1), px_int(1LL))))) {
        px_srcline(98);
        if (px_is_truthy(({ LXValue _t2218 = px_eq(px_index(_v2204, _v2207), px_str("bc")); px_is_truthy(_t2218) ? _t2218 : px_eq(px_index(_v2204, _v2207), px_str("--emit-c")); }))) {
            px_srcline(99);
             _v2205 = px_bool(true);
        }
        px_srcline(100);
        if (px_is_truthy(px_eq(px_index(_v2204, _v2207), px_str("--emit-c")))) {
            px_srcline(101);
             _v2206 = px_bool(true);
        }
        px_srcline(102);
         _v2207 = px_add(_v2207, px_int(1LL));
    }
    px_srcline(104);
    _v2208 = px_index(_v2204, px_sub(px_call(px_get_global("len"), (LXValue[]){_v2204}, 1), px_int(1LL)));
    px_srcline(105);
    _v2209 = px_call(px_get_global("cg_dirname"), (LXValue[]){_v2208}, 1);
    px_srcline(106);
    px_set_global("p_toks", px_call(px_get_global("lex_tokens"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){_v2208}, 1)}, 1));
    px_srcline(107);
    px_set_global("p_pos", px_int(0LL));
    px_srcline(108);
    px_set_global("p_brack", px_int(0LL));
    px_srcline(109);
    _v2210 = px_call(px_get_global("parse_program"), (LXValue[]){}, 0);
    px_srcline(110);
    _v2211 = px_call(px_get_global("cg_resolve_modules"), (LXValue[]){_v2210, _v2209}, 2);
    px_srcline(111);
    if (px_is_truthy(_v2205)) {
        px_srcline(112);
        if (px_is_truthy(_v2206)) {
            px_srcline(113);
            (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("bc_emit_c_program"), (LXValue[]){_v2211}, 1)}, 1));
            px_srcline(114);
            return px_int(0LL);
        }
        px_srcline(115);
        _v2212 = px_call(px_get_global("bc_emit_program"), (LXValue[]){_v2211}, 1);
        px_srcline(116);
        px_index_set(_v2212, px_str("name"), px_call(px_get_global("bc_basename"), (LXValue[]){_v2208}, 1));
        px_srcline(117);
        (void)(px_call(px_get_global("bc_dump_module"), (LXValue[]){_v2212}, 1));
        px_srcline(118);
        return px_int(0LL);
    }
    px_srcline(119);
    _v2213 = px_call(px_get_global("cg_generate"), (LXValue[]){_v2211}, 1);
    px_srcline(121);
    _v2214 = px_call(px_get_global("len"), (LXValue[]){_v2213}, 1);
    px_srcline(122);
    if (px_is_truthy(({ LXValue _t2219 = px_gt(_v2214, px_int(0LL)); px_is_truthy(_t2219) ? px_eq(px_index(_v2213, px_sub(_v2214, px_int(1LL))), px_str("\n")) : _t2219; }))) {
        px_srcline(123);
         _v2213 = px_slice(_v2213, px_int(0LL), px_sub(_v2214, px_int(1LL)), px_null());
    }
    px_srcline(124);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v2213}, 1));
px_err_2215:
    if (px_err_2215_proped) return px_err_2215_val;
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
    px_set_global("cg_seq_new", px_func("cg_seq_new", fn_cg_seq_new, NULL));
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
    px_set_global("cg_cerr_tag", px_func("cg_cerr_tag", fn_cg_cerr_tag, NULL));
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
    px_set_global("cg_cerr_uid", px_int(0LL));
    px_srcline(83);
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
