/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_sv_is_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_is_digit");
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcline(15);
    if (px_is_truthy(({ LXValue _t3 = px_ge(_v1, px_str("0")); px_is_truthy(_t3) ? px_le(_v1, px_str("9")) : _t3; }))) {
        px_srcline(16);
        return px_bool(true);
    }
    px_srcline(17);
    return px_bool(false);
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_sv_is_alnum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_is_alnum");
    LXValue _v4 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(19);
    if (px_is_truthy(px_call(px_get_global("sv_is_digit"), (LXValue[]){_v4}, 1))) {
        px_srcline(20);
        return px_bool(true);
    }
    px_srcline(21);
    if (px_is_truthy(({ LXValue _t6 = px_ge(_v4, px_str("a")); px_is_truthy(_t6) ? px_le(_v4, px_str("z")) : _t6; }))) {
        px_srcline(22);
        return px_bool(true);
    }
    px_srcline(23);
    if (px_is_truthy(({ LXValue _t7 = px_ge(_v4, px_str("A")); px_is_truthy(_t7) ? px_le(_v4, px_str("Z")) : _t7; }))) {
        px_srcline(24);
        return px_bool(true);
    }
    px_srcline(25);
    return px_bool(false);
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_sv_is_all_digits(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_is_all_digits");
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = px_null();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    px_srcline(27);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v8}, 1), px_int(0LL)))) {
        px_srcline(28);
        return px_bool(false);
    }
    px_srcline(29);
    _v9 = px_int(0LL);
    px_srcline(30);
    while (px_is_truthy(px_lt(_v9, px_call(px_get_global("len"), (LXValue[]){_v8}, 1)))) {
        px_srcline(31);
        if (px_is_truthy(px_not(px_call(px_get_global("sv_is_digit"), (LXValue[]){px_index(_v8, _v9)}, 1)))) {
            px_srcline(32);
            return px_bool(false);
        }
        px_srcline(33);
         _v9 = px_add(_v9, px_int(1LL));
    }
    px_srcline(34);
    return px_bool(true);
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_sv_index_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_index_of");
    LXValue _v11 = (nargs > 0) ? args[0] : px_null();
    LXValue _v12 = (nargs > 1) ? args[1] : px_null();
    LXValue _v13 = px_null();
    LXValue px_err_14_val = px_null();
    int px_err_14_proped = 0;
    px_srcline(37);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v12}, 1), px_int(0LL)))) {
        px_srcline(38);
        return px_int(0LL);
    }
    px_srcline(39);
    _v13 = px_int(0LL);
    px_srcline(40);
    while (px_is_truthy(px_le(_v13, px_sub(px_call(px_get_global("len"), (LXValue[]){_v11}, 1), px_call(px_get_global("len"), (LXValue[]){_v12}, 1))))) {
        px_srcline(41);
        if (px_is_truthy(px_eq(px_slice(_v11, _v13, px_add(_v13, px_call(px_get_global("len"), (LXValue[]){_v12}, 1)), px_null()), _v12))) {
            px_srcline(42);
            return _v13;
        }
        px_srcline(43);
         _v13 = px_add(_v13, px_int(1LL));
    }
    px_srcline(44);
    return px_neg(px_int(1LL));
px_err_14:
    if (px_err_14_proped) return px_err_14_val;
    return px_null();
}

static LXValue fn_sv_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_parse");
    LXValue _v15 = (nargs > 0) ? args[0] : px_null();
    LXValue _v16 = px_null();
    LXValue _v17 = px_null();
    LXValue _v18 = px_null();
    LXValue _v19 = px_null();
    LXValue _v20 = px_null();
    LXValue _v21 = px_null();
    LXValue _v22 = px_null();
    LXValue _v23 = px_null();
    LXValue _v24 = px_null();
    LXValue _v25 = px_null();
    LXValue _v26 = px_null();
    LXValue _v27 = px_null();
    LXValue px_err_28_val = px_null();
    int px_err_28_proped = 0;
    px_srcline(47);
    _v16 = _v15;
    px_srcline(48);
    _v17 = px_str("");
    px_srcline(49);
    _v18 = px_call(px_get_global("sv_index_of"), (LXValue[]){_v16, px_str("+")}, 2);
    px_srcline(50);
    if (px_is_truthy(px_ge(_v18, px_int(0LL)))) {
        px_srcline(51);
         _v17 = px_slice(_v16, px_add(_v18, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v16}, 1), px_null());
        px_srcline(52);
         _v16 = px_slice(_v16, px_int(0LL), _v18, px_null());
    }
    px_srcline(53);
    _v19 = px_str("");
    px_srcline(54);
    _v20 = px_call(px_get_global("sv_index_of"), (LXValue[]){_v16, px_str("-")}, 2);
    px_srcline(55);
    if (px_is_truthy(px_ge(_v20, px_int(0LL)))) {
        px_srcline(56);
         _v19 = px_slice(_v16, px_add(_v20, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v16}, 1), px_null());
        px_srcline(57);
         _v16 = px_slice(_v16, px_int(0LL), _v20, px_null());
    }
    px_srcline(58);
    _v21 = px_call(px_get_global("split"), (LXValue[]){_v16, px_str(".")}, 2);
    px_srcline(59);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v21}, 1), px_int(3LL)))) {
        px_srcline(60);
        return px_null();
    }
    px_srcline(61);
    if (px_is_truthy(({ LXValue _t30 = ({ LXValue _t29 = px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, px_int(0LL))}, 1), px_int(0LL)); px_is_truthy(_t29) ? _t29 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, px_int(1LL))}, 1), px_int(0LL)); }); px_is_truthy(_t30) ? _t30 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, px_int(2LL))}, 1), px_int(0LL)); }))) {
        px_srcline(62);
        return px_null();
    }
    px_srcline(64);
    _v22 = px_int(0LL);
    px_srcline(65);
    while (px_is_truthy(px_lt(_v22, px_int(3LL)))) {
        px_srcline(66);
        if (px_is_truthy(px_not(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){px_index(_v21, _v22)}, 1)))) {
            px_srcline(67);
            return px_null();
        }
        px_srcline(68);
        if (px_is_truthy(({ LXValue _t31 = px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, _v22)}, 1), px_int(1LL)); px_is_truthy(_t31) ? px_eq(px_index(px_index(_v21, _v22), px_int(0LL)), px_str("0")) : _t31; }))) {
            px_srcline(69);
            return px_null();
        }
        px_srcline(70);
         _v22 = px_add(_v22, px_int(1LL));
    }
    px_srcline(72);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v19}, 1), px_int(0LL)))) {
        px_srcline(73);
        _v23 = px_call(px_get_global("split"), (LXValue[]){_v19, px_str(".")}, 2);
        px_srcline(74);
        _v24 = px_int(0LL);
        px_srcline(75);
        while (px_is_truthy(px_lt(_v24, px_call(px_get_global("len"), (LXValue[]){_v23}, 1)))) {
            px_srcline(76);
            _v25 = px_index(_v23, _v24);
            px_srcline(77);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v25}, 1), px_int(0LL)))) {
                px_srcline(78);
                return px_null();
            }
            px_srcline(79);
            if (px_is_truthy(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v25}, 1))) {
                px_srcline(80);
                if (px_is_truthy(({ LXValue _t32 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v25}, 1), px_int(1LL)); px_is_truthy(_t32) ? px_eq(px_index(_v25, px_int(0LL)), px_str("0")) : _t32; }))) {
                    px_srcline(81);
                    return px_null();
                }
            }
            else {
                px_srcline(83);
                _v26 = px_int(0LL);
                px_srcline(84);
                while (px_is_truthy(px_lt(_v26, px_call(px_get_global("len"), (LXValue[]){_v25}, 1)))) {
                    px_srcline(85);
                    _v27 = px_index(_v25, _v26);
                    px_srcline(86);
                    if (px_is_truthy(px_not(({ LXValue _t33 = px_call(px_get_global("sv_is_alnum"), (LXValue[]){_v27}, 1); px_is_truthy(_t33) ? _t33 : px_eq(_v27, px_str("-")); })))) {
                        px_srcline(87);
                        return px_null();
                    }
                    px_srcline(88);
                     _v26 = px_add(_v26, px_int(1LL));
                }
            }
            px_srcline(89);
             _v24 = px_add(_v24, px_int(1LL));
        }
    }
    px_srcline(90);
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("major"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("int"), (LXValue[]){px_index(_v21, px_int(0LL))}, 1)); } { LXValue _k = px_str("minor"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("int"), (LXValue[]){px_index(_v21, px_int(1LL))}, 1)); } { LXValue _k = px_str("patch"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("int"), (LXValue[]){px_index(_v21, px_int(2LL))}, 1)); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v19); } { LXValue _k = px_str("build"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v17); } { LXValue _k = px_str("raw"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v15); } _d; });
px_err_28:
    if (px_err_28_proped) return px_err_28_val;
    return px_null();
}

static LXValue fn_sv_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_cmp");
    LXValue _v34 = (nargs > 0) ? args[0] : px_null();
    LXValue _v35 = (nargs > 1) ? args[1] : px_null();
    LXValue _v36 = px_null();
    LXValue _v37 = px_null();
    LXValue _v38 = px_null();
    LXValue _v39 = px_null();
    LXValue _v40 = px_null();
    LXValue _v41 = px_null();
    LXValue _v42 = px_null();
    LXValue _v43 = px_null();
    LXValue _v44 = px_null();
    LXValue _v45 = px_null();
    LXValue _v46 = px_null();
    LXValue px_err_47_val = px_null();
    int px_err_47_proped = 0;
    px_srcline(93);
    _v36 = px_call(px_get_global("sv_parse"), (LXValue[]){_v34}, 1);
    px_srcline(94);
    _v37 = px_call(px_get_global("sv_parse"), (LXValue[]){_v35}, 1);
    px_srcline(95);
    if (px_is_truthy(({ LXValue _t48 = px_eq(_v36, px_null()); px_is_truthy(_t48) ? _t48 : px_eq(_v37, px_null()); }))) {
        px_srcline(96);
        return px_int(0LL);
    }
    px_srcline(97);
    if (px_is_truthy(px_lt(px_index(_v36, px_str("major")), px_index(_v37, px_str("major"))))) {
        px_srcline(98);
        return px_neg(px_int(1LL));
    }
    px_srcline(99);
    if (px_is_truthy(px_gt(px_index(_v36, px_str("major")), px_index(_v37, px_str("major"))))) {
        px_srcline(100);
        return px_int(1LL);
    }
    px_srcline(101);
    if (px_is_truthy(px_lt(px_index(_v36, px_str("minor")), px_index(_v37, px_str("minor"))))) {
        px_srcline(102);
        return px_neg(px_int(1LL));
    }
    px_srcline(103);
    if (px_is_truthy(px_gt(px_index(_v36, px_str("minor")), px_index(_v37, px_str("minor"))))) {
        px_srcline(104);
        return px_int(1LL);
    }
    px_srcline(105);
    if (px_is_truthy(px_lt(px_index(_v36, px_str("patch")), px_index(_v37, px_str("patch"))))) {
        px_srcline(106);
        return px_neg(px_int(1LL));
    }
    px_srcline(107);
    if (px_is_truthy(px_gt(px_index(_v36, px_str("patch")), px_index(_v37, px_str("patch"))))) {
        px_srcline(108);
        return px_int(1LL);
    }
    px_srcline(110);
    _v38 = px_index(_v36, px_str("pre"));
    px_srcline(111);
    _v39 = px_index(_v37, px_str("pre"));
    px_srcline(112);
    if (px_is_truthy(({ LXValue _t49 = px_eq(_v38, px_str("")); px_is_truthy(_t49) ? px_eq(_v39, px_str("")) : _t49; }))) {
        px_srcline(113);
        return px_int(0LL);
    }
    px_srcline(114);
    if (px_is_truthy(px_eq(_v38, px_str("")))) {
        px_srcline(115);
        return px_int(1LL);
    }
    px_srcline(116);
    if (px_is_truthy(px_eq(_v39, px_str("")))) {
        px_srcline(117);
        return px_neg(px_int(1LL));
    }
    px_srcline(118);
    _v40 = px_call(px_get_global("split"), (LXValue[]){_v38, px_str(".")}, 2);
    px_srcline(119);
    _v41 = px_call(px_get_global("split"), (LXValue[]){_v39, px_str(".")}, 2);
    px_srcline(120);
    _v42 = px_int(0LL);
    px_srcline(121);
    while (px_is_truthy(({ LXValue _t50 = px_lt(_v42, px_call(px_get_global("len"), (LXValue[]){_v40}, 1)); px_is_truthy(_t50) ? px_lt(_v42, px_call(px_get_global("len"), (LXValue[]){_v41}, 1)) : _t50; }))) {
        px_srcline(122);
        _v43 = px_index(_v40, _v42);
        px_srcline(123);
        _v44 = px_index(_v41, _v42);
        px_srcline(124);
        if (px_is_truthy(px_eq(_v43, _v44))) {
            px_srcline(125);
             _v42 = px_add(_v42, px_int(1LL));
            px_srcline(126);
            continue;
        }
        px_srcline(127);
        _v45 = px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v43}, 1);
        px_srcline(128);
        _v46 = px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v44}, 1);
        px_srcline(129);
        if (px_is_truthy(({ LXValue _t51 = _v45; px_is_truthy(_t51) ? _v46 : _t51; }))) {
            px_srcline(130);
            if (px_is_truthy(px_lt(px_call(px_get_global("int"), (LXValue[]){_v43}, 1), px_call(px_get_global("int"), (LXValue[]){_v44}, 1)))) {
                px_srcline(131);
                return px_neg(px_int(1LL));
            }
            px_srcline(132);
            return px_int(1LL);
        }
        px_srcline(133);
        if (px_is_truthy(_v45)) {
            px_srcline(134);
            return px_neg(px_int(1LL));
        }
        px_srcline(135);
        if (px_is_truthy(_v46)) {
            px_srcline(136);
            return px_int(1LL);
        }
        px_srcline(137);
        if (px_is_truthy(px_lt(_v43, _v44))) {
            px_srcline(138);
            return px_neg(px_int(1LL));
        }
        px_srcline(139);
        return px_int(1LL);
    }
    px_srcline(140);
    if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v40}, 1), px_call(px_get_global("len"), (LXValue[]){_v41}, 1)))) {
        px_srcline(141);
        return px_neg(px_int(1LL));
    }
    px_srcline(142);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v40}, 1), px_call(px_get_global("len"), (LXValue[]){_v41}, 1)))) {
        px_srcline(143);
        return px_int(1LL);
    }
    px_srcline(144);
    return px_int(0LL);
px_err_47:
    if (px_err_47_proped) return px_err_47_val;
    return px_null();
}

static LXValue fn_sv_range_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_range_parse");
    LXValue _v52 = (nargs > 0) ? args[0] : px_null();
    LXValue _v53 = px_null();
    LXValue _v54 = px_null();
    LXValue _v55 = px_null();
    LXValue _v56 = px_null();
    LXValue _v57 = px_null();
    LXValue _v58 = px_null();
    LXValue _v59 = px_null();
    LXValue _v60 = px_null();
    LXValue _v61 = px_null();
    LXValue _v62 = px_null();
    LXValue _v63 = px_null();
    LXValue px_err_64_val = px_null();
    int px_err_64_proped = 0;
    px_srcline(148);
    _v53 = px_call(px_get_global("trim"), (LXValue[]){_v52}, 1);
    px_srcline(149);
    if (px_is_truthy(({ LXValue _t65 = px_eq(_v53, px_str("")); px_is_truthy(_t65) ? _t65 : px_eq(_v53, px_str("*")); }))) {
        px_srcline(150);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("0.0.0")); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("999999999.0.0")); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(151);
    _v54 = px_str("");
    px_srcline(152);
    if (px_is_truthy(px_eq(px_index(_v53, px_int(0LL)), px_str("^")))) {
        px_srcline(153);
         _v54 = px_str("^");
        px_srcline(154);
         _v53 = px_call(px_get_global("trim"), (LXValue[]){px_slice(_v53, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v53}, 1), px_null())}, 1);
    }
    else if (px_is_truthy(px_eq(px_index(_v53, px_int(0LL)), px_str("~")))) {
        px_srcline(156);
         _v54 = px_str("~");
        px_srcline(157);
         _v53 = px_call(px_get_global("trim"), (LXValue[]){px_slice(_v53, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v53}, 1), px_null())}, 1);
    }
    px_srcline(158);
    if (px_is_truthy(({ LXValue _t66 = px_eq(_v53, px_str("")); px_is_truthy(_t66) ? _t66 : px_eq(_v53, px_str("*")); }))) {
        px_srcline(159);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("0.0.0")); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("999999999.0.0")); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(161);
    _v55 = px_call(px_get_global("split"), (LXValue[]){_v53, px_str(".")}, 2);
    px_srcline(162);
    _v56 = px_list_n((LXValue[]){}, 0);
    px_srcline(163);
    _v57 = px_int(0LL);
    px_srcline(164);
    while (px_is_truthy(({ LXValue _t67 = px_lt(_v57, px_call(px_get_global("len"), (LXValue[]){_v55}, 1)); px_is_truthy(_t67) ? px_lt(_v57, px_int(3LL)) : _t67; }))) {
        px_srcline(165);
        _v58 = px_index(_v55, _v57);
        px_srcline(166);
        if (px_is_truthy(({ LXValue _t69 = ({ LXValue _t68 = px_eq(_v58, px_str("x")); px_is_truthy(_t68) ? _t68 : px_eq(_v58, px_str("X")); }); px_is_truthy(_t69) ? _t69 : px_eq(_v58, px_str("*")); }))) {
            px_srcline(167);
            (void)(px_method(_v56, "append", (LXValue[]){px_neg(px_int(1LL))}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v58}, 1))) {
            px_srcline(169);
            if (px_is_truthy(({ LXValue _t70 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v58}, 1), px_int(1LL)); px_is_truthy(_t70) ? px_eq(px_index(_v58, px_int(0LL)), px_str("0")) : _t70; }))) {
                px_srcline(170);
                return px_null();
            }
            px_srcline(171);
            (void)(px_method(_v56, "append", (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v58}, 1)}, 1));
        }
        else {
            px_srcline(173);
            return px_null();
        }
        px_srcline(174);
         _v57 = px_add(_v57, px_int(1LL));
    }
    px_srcline(176);
    while (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v56}, 1), px_int(3LL)))) {
        px_srcline(177);
        (void)(px_method(_v56, "append", (LXValue[]){px_neg(px_int(1LL))}, 1));
    }
    px_srcline(178);
    _v59 = px_index(_v56, px_int(0LL));
    px_srcline(179);
    _v60 = px_index(_v56, px_int(1LL));
    px_srcline(180);
    _v61 = px_index(_v56, px_int(2LL));
    px_srcline(181);
    if (px_is_truthy(px_lt(_v59, px_int(0LL)))) {
        px_srcline(183);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("0.0.0")); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("999999999.0.0")); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(185);
    if (px_is_truthy(({ LXValue _t72 = ({ LXValue _t71 = px_eq(_v54, px_str("")); px_is_truthy(_t71) ? px_ge(_v60, px_int(0LL)) : _t71; }); px_is_truthy(_t72) ? px_ge(_v61, px_int(0LL)) : _t72; }))) {
        px_srcline(186);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v53); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v61, px_int(1LL))}, 1))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(187);
    if (px_is_truthy(({ LXValue _t73 = px_eq(_v54, px_str("")); px_is_truthy(_t73) ? px_lt(_v60, px_int(0LL)) : _t73; }))) {
        px_srcline(189);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".0.0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1), px_str(".0.0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(190);
    if (px_is_truthy(({ LXValue _t74 = px_eq(_v54, px_str("")); px_is_truthy(_t74) ? px_lt(_v61, px_int(0LL)) : _t74; }))) {
        px_srcline(192);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(".0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v60, px_int(1LL))}, 1)), px_str(".0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(194);
    if (px_is_truthy(px_eq(_v54, px_str("^")))) {
        px_srcline(196);
        _v62 = px_str("");
        px_srcline(197);
        _v63 = px_str("");
        px_srcline(198);
        if (px_is_truthy(px_gt(_v59, px_int(0LL)))) {
            px_srcline(199);
             _v62 = px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v61}, 1));
            px_srcline(200);
             _v63 = px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1), px_str(".0.0"));
        }
        else if (px_is_truthy(px_gt(_v60, px_int(0LL)))) {
            px_srcline(202);
             _v62 = px_add(px_add(px_add(px_str("0."), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v61}, 1));
            px_srcline(203);
             _v63 = px_add(px_add(px_str("0."), px_call(px_get_global("str"), (LXValue[]){px_add(_v60, px_int(1LL))}, 1)), px_str(".0"));
        }
        else {
            px_srcline(205);
             _v62 = px_add(px_str("0.0."), px_call(px_get_global("str"), (LXValue[]){_v61}, 1));
            px_srcline(206);
             _v63 = px_add(px_str("0.0."), px_call(px_get_global("str"), (LXValue[]){px_add(_v61, px_int(1LL))}, 1));
        }
        px_srcline(207);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v62); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v63); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(209);
    if (px_is_truthy(({ LXValue _t75 = px_ge(_v59, px_int(0LL)); px_is_truthy(_t75) ? px_lt(_v60, px_int(0LL)) : _t75; }))) {
        px_srcline(211);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".0.0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1), px_str(".0.0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(212);
    if (px_is_truthy(({ LXValue _t77 = ({ LXValue _t76 = px_ge(_v59, px_int(0LL)); px_is_truthy(_t76) ? px_ge(_v60, px_int(0LL)) : _t76; }); px_is_truthy(_t77) ? px_lt(_v61, px_int(0LL)) : _t77; }))) {
        px_srcline(214);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(".0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v60, px_int(1LL))}, 1)), px_str(".0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    px_srcline(216);
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v61}, 1))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v59}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v60, px_int(1LL))}, 1)), px_str(".0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
px_err_64:
    if (px_err_64_proped) return px_err_64_val;
    return px_null();
}

static LXValue fn_sv_satisfies(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_satisfies");
    LXValue _v78 = (nargs > 0) ? args[0] : px_null();
    LXValue _v79 = (nargs > 1) ? args[1] : px_null();
    LXValue _v80 = px_null();
    LXValue _v81 = px_null();
    LXValue px_err_82_val = px_null();
    int px_err_82_proped = 0;
    px_srcline(219);
    _v80 = px_call(px_get_global("sv_range_parse"), (LXValue[]){_v79}, 1);
    px_srcline(220);
    if (px_is_truthy(px_eq(_v80, px_null()))) {
        px_srcline(221);
        return px_bool(false);
    }
    px_srcline(222);
    _v81 = px_call(px_get_global("sv_parse"), (LXValue[]){_v78}, 1);
    px_srcline(223);
    if (px_is_truthy(px_eq(_v81, px_null()))) {
        px_srcline(224);
        return px_bool(false);
    }
    px_srcline(226);
    if (px_is_truthy(px_ne(px_index(_v81, px_str("pre")), px_str("")))) {
        px_srcline(227);
        return px_bool(false);
    }
    px_srcline(228);
    return ({ LXValue _t83 = px_ge(px_call(px_get_global("sv_cmp"), (LXValue[]){_v78, px_index(_v80, px_str("lo"))}, 2), px_int(0LL)); px_is_truthy(_t83) ? px_lt(px_call(px_get_global("sv_cmp"), (LXValue[]){_v78, px_index(_v80, px_str("hi"))}, 2), px_int(0LL)) : _t83; });
px_err_82:
    if (px_err_82_proped) return px_err_82_val;
    return px_null();
}

static LXValue fn_sv_best(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_best");
    LXValue _v84 = (nargs > 0) ? args[0] : px_null();
    LXValue _v85 = (nargs > 1) ? args[1] : px_null();
    LXValue _v86 = px_null();
    LXValue _v87 = px_null();
    LXValue _v88 = px_null();
    LXValue px_err_89_val = px_null();
    int px_err_89_proped = 0;
    px_srcline(231);
    _v86 = px_null();
    px_srcline(232);
    _v87 = px_int(0LL);
    px_srcline(233);
    while (px_is_truthy(px_lt(_v87, px_call(px_get_global("len"), (LXValue[]){_v84}, 1)))) {
        px_srcline(234);
        _v88 = px_index(_v84, _v87);
        px_srcline(235);
        if (px_is_truthy(px_call(px_get_global("sv_satisfies"), (LXValue[]){_v88, _v85}, 2))) {
            px_srcline(236);
            if (px_is_truthy(({ LXValue _t90 = px_eq(_v86, px_null()); px_is_truthy(_t90) ? _t90 : px_gt(px_call(px_get_global("sv_cmp"), (LXValue[]){_v88, _v86}, 2), px_int(0LL)); }))) {
                px_srcline(237);
                 _v86 = _v88;
            }
        }
        px_srcline(238);
         _v87 = px_add(_v87, px_int(1LL));
    }
    px_srcline(239);
    return _v86;
px_err_89:
    if (px_err_89_proped) return px_err_89_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v91 = px_null();
    LXValue _v92 = px_null();
    LXValue px_err_93_val = px_null();
    int px_err_93_proped = 0;
    px_srcline(7);
    _v91 = px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2.3-alpha.1+build5")}, 1);
    px_srcline(8);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_str("major: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v91, px_str("major"))}, 1)), px_str(" minor: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v91, px_str("minor"))}, 1)), px_str(" patch: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v91, px_str("patch"))}, 1))}, 1));
    px_srcline(9);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("pre: "), px_index(_v91, px_str("pre"))), px_str(" build: ")), px_index(_v91, px_str("build")))}, 1));
    px_srcline(10);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_str("bad: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2")}, 1), px_null())}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("01.2.3")}, 1), px_null())}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2.3-01")}, 1), px_null())}, 1))}, 1));
    px_srcline(13);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1.2.3 vs 1.2.10: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.2.3"), px_str("1.2.10")}, 2)}, 1))}, 1));
    px_srcline(14);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1.0.0-alpha vs 1.0.0: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-alpha"), px_str("1.0.0")}, 2)}, 1))}, 1));
    px_srcline(15);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp alpha.2 vs alpha.10: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-alpha.2"), px_str("1.0.0-alpha.10")}, 2)}, 1))}, 1));
    px_srcline(16);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1 vs alpha: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-1"), px_str("1.0.0-alpha")}, 2)}, 1))}, 1));
    px_srcline(19);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("^1.2.0: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.5"), px_str("^1.2.0")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("2.0.0"), px_str("^1.2.0")}, 2)}, 1))}, 1));
    px_srcline(20);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("^0.2.3: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("0.2.9"), px_str("^0.2.3")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("0.3.0"), px_str("^0.2.3")}, 2)}, 1))}, 1));
    px_srcline(21);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("~1.2.3: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.9"), px_str("~1.2.3")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.3.0"), px_str("~1.2.3")}, 2)}, 1))}, 1));
    px_srcline(22);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("1.2.x: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.9"), px_str("1.2.x")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.3.0"), px_str("1.2.x")}, 2)}, 1))}, 1));
    px_srcline(23);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("exact: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.3"), px_str("1.2.3")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.4"), px_str("1.2.3")}, 2)}, 1))}, 1));
    px_srcline(24);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("pre reject: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.3-alpha"), px_str("1.2.3")}, 2)}, 1))}, 1));
    px_srcline(27);
    _v92 = px_list_n((LXValue[]){px_str("1.2.0"), px_str("1.2.5"), px_str("2.0.0"), px_str("1.0.0")}, 4);
    px_srcline(28);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^1.2.0: "), px_call(px_get_global("sv_best"), (LXValue[]){_v92, px_str("^1.2.0")}, 2))}, 1));
    px_srcline(29);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^2.0.0: "), px_call(px_get_global("sv_best"), (LXValue[]){_v92, px_str("^2.0.0")}, 2))}, 1));
    px_srcline(30);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^0.5.0: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_best"), (LXValue[]){_v92, px_str("^0.5.0")}, 2), px_null())}, 1))}, 1));
    px_srcline(31);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ~1.2: "), px_call(px_get_global("sv_best"), (LXValue[]){px_list_n((LXValue[]){px_str("1.2.0"), px_str("1.2.9"), px_str("1.3.0")}, 3), px_str("~1.2")}, 2))}, 1));
    px_srcline(32);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("semver ok")}, 1));
px_err_93:
    if (px_err_93_proped) return px_err_93_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("sv_is_digit", px_func("sv_is_digit", fn_sv_is_digit, NULL));
    px_set_global("sv_is_alnum", px_func("sv_is_alnum", fn_sv_is_alnum, NULL));
    px_set_global("sv_is_all_digits", px_func("sv_is_all_digits", fn_sv_is_all_digits, NULL));
    px_set_global("sv_index_of", px_func("sv_index_of", fn_sv_index_of, NULL));
    px_set_global("sv_parse", px_func("sv_parse", fn_sv_parse, NULL));
    px_set_global("sv_cmp", px_func("sv_cmp", fn_sv_cmp, NULL));
    px_set_global("sv_range_parse", px_func("sv_range_parse", fn_sv_range_parse, NULL));
    px_set_global("sv_satisfies", px_func("sv_satisfies", fn_sv_satisfies, NULL));
    px_set_global("sv_best", px_func("sv_best", fn_sv_best, NULL));
    px_set_global("main", px_func("main", fn_main, NULL));
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
