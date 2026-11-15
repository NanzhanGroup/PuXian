/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_sv_is_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_is_digit");
    if (nargs < 1) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_is_digit", 1, nargs);
    if (nargs > 1) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_is_digit", 1, nargs);
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcpos("sv_is_digit", 15);
    if (px_is_truthy(({ LXValue _t3 = px_ge(_v1, px_str("0")); px_is_truthy(_t3) ? px_le(_v1, px_str("9")) : _t3; }))) {
        px_srcpos("sv_is_digit", 16);
        return px_bool(true);
    }
    px_srcpos("sv_is_digit", 17);
    return px_bool(false);
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_sv_is_alnum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_is_alnum");
    if (nargs < 1) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_is_alnum", 1, nargs);
    if (nargs > 1) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_is_alnum", 1, nargs);
    LXValue _v4 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcpos("sv_is_alnum", 19);
    if (px_is_truthy(px_call(px_get_global("sv_is_digit"), (LXValue[]){_v4}, 1))) {
        px_srcpos("sv_is_alnum", 20);
        return px_bool(true);
    }
    px_srcpos("sv_is_alnum", 21);
    if (px_is_truthy(({ LXValue _t6 = px_ge(_v4, px_str("a")); px_is_truthy(_t6) ? px_le(_v4, px_str("z")) : _t6; }))) {
        px_srcpos("sv_is_alnum", 22);
        return px_bool(true);
    }
    px_srcpos("sv_is_alnum", 23);
    if (px_is_truthy(({ LXValue _t7 = px_ge(_v4, px_str("A")); px_is_truthy(_t7) ? px_le(_v4, px_str("Z")) : _t7; }))) {
        px_srcpos("sv_is_alnum", 24);
        return px_bool(true);
    }
    px_srcpos("sv_is_alnum", 25);
    return px_bool(false);
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_sv_is_all_digits(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_is_all_digits");
    if (nargs < 1) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_is_all_digits", 1, nargs);
    if (nargs > 1) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_is_all_digits", 1, nargs);
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = px_uninit();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    px_srcpos("sv_is_all_digits", 27);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v8}, 1), px_int(0LL)))) {
        px_srcpos("sv_is_all_digits", 28);
        return px_bool(false);
    }
    px_srcpos("sv_is_all_digits", 29);
    _v9 = px_int(0LL);
    px_srcpos("sv_is_all_digits", 30);
    while (px_is_truthy(px_lt(_v9, px_call(px_get_global("len"), (LXValue[]){_v8}, 1)))) {
        px_srcpos("sv_is_all_digits", 31);
        if (px_is_truthy(px_not(px_call(px_get_global("sv_is_digit"), (LXValue[]){px_index(_v8, _v9)}, 1)))) {
            px_srcpos("sv_is_all_digits", 32);
            return px_bool(false);
        }
        px_srcpos("sv_is_all_digits", 33);
         _v9 = px_add(_v9, px_int(1LL));
    }
    px_srcpos("sv_is_all_digits", 34);
    return px_bool(true);
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_sv_index_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_index_of");
    if (nargs < 2) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_index_of", 2, nargs);
    if (nargs > 2) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_index_of", 2, nargs);
    LXValue _v11 = (nargs > 0) ? args[0] : px_null();
    LXValue _v12 = (nargs > 1) ? args[1] : px_null();
    LXValue _v13 = px_uninit();
    LXValue px_err_14_val = px_null();
    int px_err_14_proped = 0;
    px_srcpos("sv_index_of", 37);
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v12}, 1), px_int(0LL)))) {
        px_srcpos("sv_index_of", 38);
        return px_int(0LL);
    }
    px_srcpos("sv_index_of", 39);
    _v13 = px_int(0LL);
    px_srcpos("sv_index_of", 40);
    while (px_is_truthy(px_le(_v13, ({ LXValue _s1 = px_call(px_get_global("len"), (LXValue[]){_v11}, 1); LXValue _s2 = px_call(px_get_global("len"), (LXValue[]){_v12}, 1); px_sub(_s1, _s2); })))) {
        px_srcpos("sv_index_of", 41);
        if (px_is_truthy(px_eq(px_slice(_v11, _v13, px_add(_v13, px_call(px_get_global("len"), (LXValue[]){_v12}, 1)), px_null()), _v12))) {
            px_srcpos("sv_index_of", 42);
            return _v13;
        }
        px_srcpos("sv_index_of", 43);
         _v13 = px_add(_v13, px_int(1LL));
    }
    px_srcpos("sv_index_of", 44);
    return px_neg(px_int(1LL));
px_err_14:
    if (px_err_14_proped) return px_err_14_val;
    return px_null();
}

static LXValue fn_sv_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_parse");
    if (nargs < 1) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_parse", 1, nargs);
    if (nargs > 1) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_parse", 1, nargs);
    LXValue _v15 = (nargs > 0) ? args[0] : px_null();
    LXValue _v16 = px_uninit();
    LXValue _v17 = px_uninit();
    LXValue _v18 = px_uninit();
    LXValue _v19 = px_uninit();
    LXValue _v20 = px_uninit();
    LXValue _v21 = px_uninit();
    LXValue _v22 = px_uninit();
    LXValue _v23 = px_uninit();
    LXValue _v24 = px_uninit();
    LXValue _v25 = px_uninit();
    LXValue _v26 = px_uninit();
    LXValue _v27 = px_uninit();
    LXValue px_err_28_val = px_null();
    int px_err_28_proped = 0;
    px_srcpos("sv_parse", 47);
    _v16 = _v15;
    px_srcpos("sv_parse", 48);
    _v17 = px_str("");
    px_srcpos("sv_parse", 49);
    _v18 = px_call(px_get_global("sv_index_of"), (LXValue[]){_v16, px_str("+")}, 2);
    px_srcpos("sv_parse", 50);
    if (px_is_truthy(px_ge(_v18, px_int(0LL)))) {
        px_srcpos("sv_parse", 51);
         _v17 = px_slice(_v16, px_add(_v18, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v16}, 1), px_null());
        px_srcpos("sv_parse", 54);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v17}, 1), px_int(0LL)))) {
            px_srcpos("sv_parse", 55);
            return px_null();
        }
        px_srcpos("sv_parse", 56);
         _v16 = px_slice(_v16, px_int(0LL), _v18, px_null());
    }
    px_srcpos("sv_parse", 57);
    _v19 = px_str("");
    px_srcpos("sv_parse", 58);
    _v20 = px_call(px_get_global("sv_index_of"), (LXValue[]){_v16, px_str("-")}, 2);
    px_srcpos("sv_parse", 59);
    if (px_is_truthy(px_ge(_v20, px_int(0LL)))) {
        px_srcpos("sv_parse", 60);
         _v19 = px_slice(_v16, px_add(_v20, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v16}, 1), px_null());
        px_srcpos("sv_parse", 64);
        if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v19}, 1), px_int(0LL)))) {
            px_srcpos("sv_parse", 65);
            return px_null();
        }
        px_srcpos("sv_parse", 66);
         _v16 = px_slice(_v16, px_int(0LL), _v20, px_null());
    }
    px_srcpos("sv_parse", 67);
    _v21 = px_call(px_get_global("split"), (LXValue[]){_v16, px_str(".")}, 2);
    px_srcpos("sv_parse", 68);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v21}, 1), px_int(3LL)))) {
        px_srcpos("sv_parse", 69);
        return px_null();
    }
    px_srcpos("sv_parse", 70);
    if (px_is_truthy(({ LXValue _t30 = ({ LXValue _t29 = px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, px_int(0LL))}, 1), px_int(0LL)); px_is_truthy(_t29) ? _t29 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, px_int(1LL))}, 1), px_int(0LL)); }); px_is_truthy(_t30) ? _t30 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, px_int(2LL))}, 1), px_int(0LL)); }))) {
        px_srcpos("sv_parse", 71);
        return px_null();
    }
    px_srcpos("sv_parse", 73);
    _v22 = px_int(0LL);
    px_srcpos("sv_parse", 74);
    while (px_is_truthy(px_lt(_v22, px_int(3LL)))) {
        px_srcpos("sv_parse", 75);
        if (px_is_truthy(px_not(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){px_index(_v21, _v22)}, 1)))) {
            px_srcpos("sv_parse", 76);
            return px_null();
        }
        px_srcpos("sv_parse", 77);
        if (px_is_truthy(({ LXValue _t31 = px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v21, _v22)}, 1), px_int(1LL)); px_is_truthy(_t31) ? px_eq(px_index(px_index(_v21, _v22), px_int(0LL)), px_str("0")) : _t31; }))) {
            px_srcpos("sv_parse", 78);
            return px_null();
        }
        px_srcpos("sv_parse", 79);
         _v22 = px_add(_v22, px_int(1LL));
    }
    px_srcpos("sv_parse", 81);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v19}, 1), px_int(0LL)))) {
        px_srcpos("sv_parse", 82);
        _v23 = px_call(px_get_global("split"), (LXValue[]){_v19, px_str(".")}, 2);
        px_srcpos("sv_parse", 83);
        _v24 = px_int(0LL);
        px_srcpos("sv_parse", 84);
        while (px_is_truthy(px_lt(_v24, px_call(px_get_global("len"), (LXValue[]){_v23}, 1)))) {
            px_srcpos("sv_parse", 85);
            _v25 = px_index(_v23, _v24);
            px_srcpos("sv_parse", 86);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v25}, 1), px_int(0LL)))) {
                px_srcpos("sv_parse", 87);
                return px_null();
            }
            px_srcpos("sv_parse", 88);
            if (px_is_truthy(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v25}, 1))) {
                px_srcpos("sv_parse", 89);
                if (px_is_truthy(({ LXValue _t32 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v25}, 1), px_int(1LL)); px_is_truthy(_t32) ? px_eq(px_index(_v25, px_int(0LL)), px_str("0")) : _t32; }))) {
                    px_srcpos("sv_parse", 90);
                    return px_null();
                }
            }
            else {
                px_srcpos("sv_parse", 92);
                _v26 = px_int(0LL);
                px_srcpos("sv_parse", 93);
                while (px_is_truthy(px_lt(_v26, px_call(px_get_global("len"), (LXValue[]){_v25}, 1)))) {
                    px_srcpos("sv_parse", 94);
                    _v27 = px_index(_v25, _v26);
                    px_srcpos("sv_parse", 95);
                    if (px_is_truthy(px_not(({ LXValue _t33 = px_call(px_get_global("sv_is_alnum"), (LXValue[]){_v27}, 1); px_is_truthy(_t33) ? _t33 : px_eq(_v27, px_str("-")); })))) {
                        px_srcpos("sv_parse", 96);
                        return px_null();
                    }
                    px_srcpos("sv_parse", 97);
                     _v26 = px_add(_v26, px_int(1LL));
                }
            }
            px_srcpos("sv_parse", 98);
             _v24 = px_add(_v24, px_int(1LL));
        }
    }
    px_srcpos("sv_parse", 99);
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("major"); LXValue _v = px_call(px_get_global("int"), (LXValue[]){px_index(_v21, px_int(0LL))}, 1); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("minor"); LXValue _v = px_call(px_get_global("int"), (LXValue[]){px_index(_v21, px_int(1LL))}, 1); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("patch"); LXValue _v = px_call(px_get_global("int"), (LXValue[]){px_index(_v21, px_int(2LL))}, 1); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = _v19; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("build"); LXValue _v = _v17; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("raw"); LXValue _v = _v15; px_dict_set_checked(_d, _k, _v); } _d; });
px_err_28:
    if (px_err_28_proped) return px_err_28_val;
    return px_null();
}

static LXValue fn_sv_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_cmp");
    if (nargs < 2) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_cmp", 2, nargs);
    if (nargs > 2) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_cmp", 2, nargs);
    LXValue _v34 = (nargs > 0) ? args[0] : px_null();
    LXValue _v35 = (nargs > 1) ? args[1] : px_null();
    LXValue _v36 = px_uninit();
    LXValue _v37 = px_uninit();
    LXValue _v38 = px_uninit();
    LXValue _v39 = px_uninit();
    LXValue _v40 = px_uninit();
    LXValue _v41 = px_uninit();
    LXValue _v42 = px_uninit();
    LXValue _v43 = px_uninit();
    LXValue _v44 = px_uninit();
    LXValue _v45 = px_uninit();
    LXValue _v46 = px_uninit();
    LXValue px_err_47_val = px_null();
    int px_err_47_proped = 0;
    px_srcpos("sv_cmp", 102);
    _v36 = px_call(px_get_global("sv_parse"), (LXValue[]){_v34}, 1);
    px_srcpos("sv_cmp", 103);
    _v37 = px_call(px_get_global("sv_parse"), (LXValue[]){_v35}, 1);
    px_srcpos("sv_cmp", 107);
    if (px_is_truthy(px_eq(_v36, px_null()))) {
        px_srcpos("sv_cmp", 108);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("sv_cmp: 版本号非法: "), px_call(px_get_global("str"), (LXValue[]){_v34}, 1))}, 1));
    }
    px_srcpos("sv_cmp", 109);
    if (px_is_truthy(px_eq(_v37, px_null()))) {
        px_srcpos("sv_cmp", 110);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("sv_cmp: 版本号非法: "), px_call(px_get_global("str"), (LXValue[]){_v35}, 1))}, 1));
    }
    px_srcpos("sv_cmp", 111);
    if (px_is_truthy(px_lt(px_index(_v36, px_str("major")), px_index(_v37, px_str("major"))))) {
        px_srcpos("sv_cmp", 112);
        return px_neg(px_int(1LL));
    }
    px_srcpos("sv_cmp", 113);
    if (px_is_truthy(px_gt(px_index(_v36, px_str("major")), px_index(_v37, px_str("major"))))) {
        px_srcpos("sv_cmp", 114);
        return px_int(1LL);
    }
    px_srcpos("sv_cmp", 115);
    if (px_is_truthy(px_lt(px_index(_v36, px_str("minor")), px_index(_v37, px_str("minor"))))) {
        px_srcpos("sv_cmp", 116);
        return px_neg(px_int(1LL));
    }
    px_srcpos("sv_cmp", 117);
    if (px_is_truthy(px_gt(px_index(_v36, px_str("minor")), px_index(_v37, px_str("minor"))))) {
        px_srcpos("sv_cmp", 118);
        return px_int(1LL);
    }
    px_srcpos("sv_cmp", 119);
    if (px_is_truthy(px_lt(px_index(_v36, px_str("patch")), px_index(_v37, px_str("patch"))))) {
        px_srcpos("sv_cmp", 120);
        return px_neg(px_int(1LL));
    }
    px_srcpos("sv_cmp", 121);
    if (px_is_truthy(px_gt(px_index(_v36, px_str("patch")), px_index(_v37, px_str("patch"))))) {
        px_srcpos("sv_cmp", 122);
        return px_int(1LL);
    }
    px_srcpos("sv_cmp", 124);
    _v38 = px_index(_v36, px_str("pre"));
    px_srcpos("sv_cmp", 125);
    _v39 = px_index(_v37, px_str("pre"));
    px_srcpos("sv_cmp", 126);
    if (px_is_truthy(({ LXValue _t48 = px_eq(_v38, px_str("")); px_is_truthy(_t48) ? px_eq(_v39, px_str("")) : _t48; }))) {
        px_srcpos("sv_cmp", 127);
        return px_int(0LL);
    }
    px_srcpos("sv_cmp", 128);
    if (px_is_truthy(px_eq(_v38, px_str("")))) {
        px_srcpos("sv_cmp", 129);
        return px_int(1LL);
    }
    px_srcpos("sv_cmp", 130);
    if (px_is_truthy(px_eq(_v39, px_str("")))) {
        px_srcpos("sv_cmp", 131);
        return px_neg(px_int(1LL));
    }
    px_srcpos("sv_cmp", 132);
    _v40 = px_call(px_get_global("split"), (LXValue[]){_v38, px_str(".")}, 2);
    px_srcpos("sv_cmp", 133);
    _v41 = px_call(px_get_global("split"), (LXValue[]){_v39, px_str(".")}, 2);
    px_srcpos("sv_cmp", 134);
    _v42 = px_int(0LL);
    px_srcpos("sv_cmp", 135);
    while (px_is_truthy(({ LXValue _t49 = px_lt(_v42, px_call(px_get_global("len"), (LXValue[]){_v40}, 1)); px_is_truthy(_t49) ? px_lt(_v42, px_call(px_get_global("len"), (LXValue[]){_v41}, 1)) : _t49; }))) {
        px_srcpos("sv_cmp", 136);
        _v43 = px_index(_v40, _v42);
        px_srcpos("sv_cmp", 137);
        _v44 = px_index(_v41, _v42);
        px_srcpos("sv_cmp", 138);
        if (px_is_truthy(px_eq(_v43, _v44))) {
            px_srcpos("sv_cmp", 139);
             _v42 = px_add(_v42, px_int(1LL));
            px_srcpos("sv_cmp", 140);
            continue;
        }
        px_srcpos("sv_cmp", 141);
        _v45 = px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v43}, 1);
        px_srcpos("sv_cmp", 142);
        _v46 = px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v44}, 1);
        px_srcpos("sv_cmp", 143);
        if (px_is_truthy(({ LXValue _t50 = _v45; px_is_truthy(_t50) ? _v46 : _t50; }))) {
            px_srcpos("sv_cmp", 144);
            if (px_is_truthy(({ LXValue _s3 = px_call(px_get_global("int"), (LXValue[]){_v43}, 1); LXValue _s4 = px_call(px_get_global("int"), (LXValue[]){_v44}, 1); px_lt(_s3, _s4); }))) {
                px_srcpos("sv_cmp", 145);
                return px_neg(px_int(1LL));
            }
            px_srcpos("sv_cmp", 146);
            return px_int(1LL);
        }
        px_srcpos("sv_cmp", 147);
        if (px_is_truthy(_v45)) {
            px_srcpos("sv_cmp", 148);
            return px_neg(px_int(1LL));
        }
        px_srcpos("sv_cmp", 149);
        if (px_is_truthy(_v46)) {
            px_srcpos("sv_cmp", 150);
            return px_int(1LL);
        }
        px_srcpos("sv_cmp", 151);
        if (px_is_truthy(px_lt(_v43, _v44))) {
            px_srcpos("sv_cmp", 152);
            return px_neg(px_int(1LL));
        }
        px_srcpos("sv_cmp", 153);
        return px_int(1LL);
    }
    px_srcpos("sv_cmp", 154);
    if (px_is_truthy(({ LXValue _s5 = px_call(px_get_global("len"), (LXValue[]){_v40}, 1); LXValue _s6 = px_call(px_get_global("len"), (LXValue[]){_v41}, 1); px_lt(_s5, _s6); }))) {
        px_srcpos("sv_cmp", 155);
        return px_neg(px_int(1LL));
    }
    px_srcpos("sv_cmp", 156);
    if (px_is_truthy(({ LXValue _s7 = px_call(px_get_global("len"), (LXValue[]){_v40}, 1); LXValue _s8 = px_call(px_get_global("len"), (LXValue[]){_v41}, 1); px_gt(_s7, _s8); }))) {
        px_srcpos("sv_cmp", 157);
        return px_int(1LL);
    }
    px_srcpos("sv_cmp", 158);
    return px_int(0LL);
px_err_47:
    if (px_err_47_proped) return px_err_47_val;
    return px_null();
}

static LXValue fn_sv_range_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_range_parse");
    if (nargs < 1) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_range_parse", 1, nargs);
    if (nargs > 1) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_range_parse", 1, nargs);
    LXValue _v51 = (nargs > 0) ? args[0] : px_null();
    LXValue _v52 = px_uninit();
    LXValue _v53 = px_uninit();
    LXValue _v54 = px_uninit();
    LXValue _v55 = px_uninit();
    LXValue _v56 = px_uninit();
    LXValue _v57 = px_uninit();
    LXValue _v58 = px_uninit();
    LXValue _v59 = px_uninit();
    LXValue _v60 = px_uninit();
    LXValue _v61 = px_uninit();
    LXValue _v62 = px_uninit();
    LXValue _v63 = px_uninit();
    LXValue _v64 = px_uninit();
    LXValue px_err_65_val = px_null();
    int px_err_65_proped = 0;
    px_srcpos("sv_range_parse", 162);
    _v52 = px_call(px_get_global("trim"), (LXValue[]){_v51}, 1);
    px_srcpos("sv_range_parse", 163);
    if (px_is_truthy(({ LXValue _t66 = px_eq(_v52, px_str("")); px_is_truthy(_t66) ? _t66 : px_eq(_v52, px_str("*")); }))) {
        px_srcpos("sv_range_parse", 164);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_str("0.0.0"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_str("999999999.0.0"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 165);
    _v53 = px_str("");
    px_srcpos("sv_range_parse", 166);
    if (px_is_truthy(px_eq(px_index(_v52, px_int(0LL)), px_str("^")))) {
        px_srcpos("sv_range_parse", 167);
         _v53 = px_str("^");
        px_srcpos("sv_range_parse", 168);
         _v52 = px_call(px_get_global("trim"), (LXValue[]){px_slice(_v52, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v52}, 1), px_null())}, 1);
    }
    else if (px_is_truthy(px_eq(px_index(_v52, px_int(0LL)), px_str("~")))) {
        px_srcpos("sv_range_parse", 170);
         _v53 = px_str("~");
        px_srcpos("sv_range_parse", 171);
         _v52 = px_call(px_get_global("trim"), (LXValue[]){px_slice(_v52, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v52}, 1), px_null())}, 1);
    }
    px_srcpos("sv_range_parse", 172);
    if (px_is_truthy(({ LXValue _t67 = px_eq(_v52, px_str("")); px_is_truthy(_t67) ? _t67 : px_eq(_v52, px_str("*")); }))) {
        px_srcpos("sv_range_parse", 176);
        if (px_is_truthy(px_ne(_v53, px_str("")))) {
            px_srcpos("sv_range_parse", 177);
            return px_null();
        }
        px_srcpos("sv_range_parse", 178);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_str("0.0.0"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_str("999999999.0.0"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 180);
    _v54 = px_call(px_get_global("split"), (LXValue[]){_v52, px_str(".")}, 2);
    px_srcpos("sv_range_parse", 184);
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v54}, 1), px_int(3LL)))) {
        px_srcpos("sv_range_parse", 185);
        return px_null();
    }
    px_srcpos("sv_range_parse", 186);
    _v55 = px_list_n((LXValue[]){}, 0);
    px_srcpos("sv_range_parse", 187);
    _v56 = px_int(0LL);
    px_srcpos("sv_range_parse", 188);
    while (px_is_truthy(({ LXValue _t68 = px_lt(_v56, px_call(px_get_global("len"), (LXValue[]){_v54}, 1)); px_is_truthy(_t68) ? px_lt(_v56, px_int(3LL)) : _t68; }))) {
        px_srcpos("sv_range_parse", 189);
        _v57 = px_index(_v54, _v56);
        px_srcpos("sv_range_parse", 190);
        if (px_is_truthy(({ LXValue _t70 = ({ LXValue _t69 = px_eq(_v57, px_str("x")); px_is_truthy(_t69) ? _t69 : px_eq(_v57, px_str("X")); }); px_is_truthy(_t70) ? _t70 : px_eq(_v57, px_str("*")); }))) {
            px_srcpos("sv_range_parse", 191);
            (void)(px_method(_v55, "append", (LXValue[]){px_neg(px_int(1LL))}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v57}, 1))) {
            px_srcpos("sv_range_parse", 193);
            if (px_is_truthy(({ LXValue _t71 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v57}, 1), px_int(1LL)); px_is_truthy(_t71) ? px_eq(px_index(_v57, px_int(0LL)), px_str("0")) : _t71; }))) {
                px_srcpos("sv_range_parse", 194);
                return px_null();
            }
            px_srcpos("sv_range_parse", 195);
            (void)(px_method(_v55, "append", (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v57}, 1)}, 1));
        }
        else {
            px_srcpos("sv_range_parse", 197);
            return px_null();
        }
        px_srcpos("sv_range_parse", 198);
         _v56 = px_add(_v56, px_int(1LL));
    }
    px_srcpos("sv_range_parse", 200);
    while (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v55}, 1), px_int(3LL)))) {
        px_srcpos("sv_range_parse", 201);
        (void)(px_method(_v55, "append", (LXValue[]){px_neg(px_int(1LL))}, 1));
    }
    px_srcpos("sv_range_parse", 202);
    _v58 = px_index(_v55, px_int(0LL));
    px_srcpos("sv_range_parse", 203);
    _v59 = px_index(_v55, px_int(1LL));
    px_srcpos("sv_range_parse", 204);
    _v60 = px_index(_v55, px_int(2LL));
    px_srcpos("sv_range_parse", 205);
    if (px_is_truthy(px_lt(_v58, px_int(0LL)))) {
        px_srcpos("sv_range_parse", 207);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_str("0.0.0"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_str("999999999.0.0"); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 209);
    if (px_is_truthy(({ LXValue _t73 = ({ LXValue _t72 = px_eq(_v53, px_str("")); px_is_truthy(_t72) ? px_ge(_v59, px_int(0LL)) : _t72; }); px_is_truthy(_t73) ? px_ge(_v60, px_int(0LL)) : _t73; }))) {
        px_srcpos("sv_range_parse", 210);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = _v52; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = ({ LXValue _s11 = px_add(({ LXValue _s9 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s10 = px_call(px_get_global("str"), (LXValue[]){_v59}, 1); px_add(_s9, _s10); }), px_str(".")); LXValue _s12 = px_call(px_get_global("str"), (LXValue[]){px_add(_v60, px_int(1LL))}, 1); px_add(_s11, _s12); }); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 211);
    if (px_is_truthy(({ LXValue _t74 = px_eq(_v53, px_str("")); px_is_truthy(_t74) ? px_lt(_v59, px_int(0LL)) : _t74; }))) {
        px_srcpos("sv_range_parse", 213);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".0.0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v58, px_int(1LL))}, 1), px_str(".0.0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 214);
    if (px_is_truthy(({ LXValue _t75 = px_eq(_v53, px_str("")); px_is_truthy(_t75) ? px_lt(_v60, px_int(0LL)) : _t75; }))) {
        px_srcpos("sv_range_parse", 216);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_add(({ LXValue _s13 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s14 = px_call(px_get_global("str"), (LXValue[]){_v59}, 1); px_add(_s13, _s14); }), px_str(".0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_add(({ LXValue _s15 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s16 = px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1); px_add(_s15, _s16); }), px_str(".0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 218);
    if (px_is_truthy(px_eq(_v53, px_str("^")))) {
        px_srcpos("sv_range_parse", 223);
        _v61 = _v59;
        px_srcpos("sv_range_parse", 224);
        _v62 = _v60;
        px_srcpos("sv_range_parse", 225);
        if (px_is_truthy(px_lt(_v61, px_int(0LL)))) {
            px_srcpos("sv_range_parse", 226);
             _v61 = px_int(0LL);
        }
        px_srcpos("sv_range_parse", 227);
        if (px_is_truthy(px_lt(_v62, px_int(0LL)))) {
            px_srcpos("sv_range_parse", 228);
             _v62 = px_int(0LL);
        }
        px_srcpos("sv_range_parse", 229);
        _v63 = px_str("");
        px_srcpos("sv_range_parse", 230);
        _v64 = px_str("");
        px_srcpos("sv_range_parse", 231);
        if (px_is_truthy(px_gt(_v58, px_int(0LL)))) {
            px_srcpos("sv_range_parse", 232);
             _v63 = ({ LXValue _s19 = px_add(({ LXValue _s17 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s18 = px_call(px_get_global("str"), (LXValue[]){_v61}, 1); px_add(_s17, _s18); }), px_str(".")); LXValue _s20 = px_call(px_get_global("str"), (LXValue[]){_v62}, 1); px_add(_s19, _s20); });
            px_srcpos("sv_range_parse", 233);
             _v64 = px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v58, px_int(1LL))}, 1), px_str(".0.0"));
        }
        else if (px_is_truthy(px_gt(_v59, px_int(0LL)))) {
            px_srcpos("sv_range_parse", 235);
             _v63 = ({ LXValue _s21 = px_add(px_add(px_str("0."), px_call(px_get_global("str"), (LXValue[]){_v59}, 1)), px_str(".")); LXValue _s22 = px_call(px_get_global("str"), (LXValue[]){_v62}, 1); px_add(_s21, _s22); });
            px_srcpos("sv_range_parse", 236);
             _v64 = px_add(px_add(px_str("0."), px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1)), px_str(".0"));
        }
        else if (px_is_truthy(({ LXValue _t76 = px_eq(_v59, px_int(0LL)); px_is_truthy(_t76) ? px_ge(_v60, px_int(0LL)) : _t76; }))) {
            px_srcpos("sv_range_parse", 238);
             _v63 = px_add(px_str("0.0."), px_call(px_get_global("str"), (LXValue[]){_v60}, 1));
            px_srcpos("sv_range_parse", 239);
             _v64 = px_add(px_str("0.0."), px_call(px_get_global("str"), (LXValue[]){px_add(_v60, px_int(1LL))}, 1));
        }
        else if (px_is_truthy(px_eq(_v59, px_int(0LL)))) {
            px_srcpos("sv_range_parse", 241);
             _v63 = px_str("0.0.0");
            px_srcpos("sv_range_parse", 242);
             _v64 = px_str("0.1.0");
        }
        else {
            px_srcpos("sv_range_parse", 244);
             _v63 = px_str("0.0.0");
            px_srcpos("sv_range_parse", 245);
             _v64 = px_str("1.0.0");
        }
        px_srcpos("sv_range_parse", 246);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = _v63; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = _v64; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 248);
    if (px_is_truthy(({ LXValue _t77 = px_ge(_v58, px_int(0LL)); px_is_truthy(_t77) ? px_lt(_v59, px_int(0LL)) : _t77; }))) {
        px_srcpos("sv_range_parse", 250);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".0.0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v58, px_int(1LL))}, 1), px_str(".0.0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 251);
    if (px_is_truthy(({ LXValue _t79 = ({ LXValue _t78 = px_ge(_v58, px_int(0LL)); px_is_truthy(_t78) ? px_ge(_v59, px_int(0LL)) : _t78; }); px_is_truthy(_t79) ? px_lt(_v60, px_int(0LL)) : _t79; }))) {
        px_srcpos("sv_range_parse", 253);
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = px_add(({ LXValue _s23 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s24 = px_call(px_get_global("str"), (LXValue[]){_v59}, 1); px_add(_s23, _s24); }), px_str(".0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_add(({ LXValue _s25 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s26 = px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1); px_add(_s25, _s26); }), px_str(".0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
    }
    px_srcpos("sv_range_parse", 255);
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); LXValue _v = ({ LXValue _s29 = px_add(({ LXValue _s27 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s28 = px_call(px_get_global("str"), (LXValue[]){_v59}, 1); px_add(_s27, _s28); }), px_str(".")); LXValue _s30 = px_call(px_get_global("str"), (LXValue[]){_v60}, 1); px_add(_s29, _s30); }); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("hi"); LXValue _v = px_add(({ LXValue _s31 = px_add(px_call(px_get_global("str"), (LXValue[]){_v58}, 1), px_str(".")); LXValue _s32 = px_call(px_get_global("str"), (LXValue[]){px_add(_v59, px_int(1LL))}, 1); px_add(_s31, _s32); }), px_str(".0")); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("pre"); LXValue _v = px_str(""); px_dict_set_checked(_d, _k, _v); } _d; });
px_err_65:
    if (px_err_65_proped) return px_err_65_val;
    return px_null();
}

static LXValue fn_sv_satisfies(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_satisfies");
    if (nargs < 2) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_satisfies", 2, nargs);
    if (nargs > 2) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_satisfies", 2, nargs);
    LXValue _v80 = (nargs > 0) ? args[0] : px_null();
    LXValue _v81 = (nargs > 1) ? args[1] : px_null();
    LXValue _v82 = px_uninit();
    LXValue _v83 = px_uninit();
    LXValue px_err_84_val = px_null();
    int px_err_84_proped = 0;
    px_srcpos("sv_satisfies", 258);
    _v82 = px_call(px_get_global("sv_range_parse"), (LXValue[]){_v81}, 1);
    px_srcpos("sv_satisfies", 259);
    if (px_is_truthy(px_eq(_v82, px_null()))) {
        px_srcpos("sv_satisfies", 260);
        return px_bool(false);
    }
    px_srcpos("sv_satisfies", 261);
    _v83 = px_call(px_get_global("sv_parse"), (LXValue[]){_v80}, 1);
    px_srcpos("sv_satisfies", 262);
    if (px_is_truthy(px_eq(_v83, px_null()))) {
        px_srcpos("sv_satisfies", 263);
        return px_bool(false);
    }
    px_srcpos("sv_satisfies", 265);
    if (px_is_truthy(px_ne(px_index(_v83, px_str("pre")), px_str("")))) {
        px_srcpos("sv_satisfies", 266);
        return px_bool(false);
    }
    px_srcpos("sv_satisfies", 267);
    return ({ LXValue _t85 = px_ge(px_call(px_get_global("sv_cmp"), (LXValue[]){_v80, px_index(_v82, px_str("lo"))}, 2), px_int(0LL)); px_is_truthy(_t85) ? px_lt(px_call(px_get_global("sv_cmp"), (LXValue[]){_v80, px_index(_v82, px_str("hi"))}, 2), px_int(0LL)) : _t85; });
px_err_84:
    if (px_err_84_proped) return px_err_84_val;
    return px_null();
}

static LXValue fn_sv_best(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sv_best");
    if (nargs < 2) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "sv_best", 2, nargs);
    if (nargs > 2) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "sv_best", 2, nargs);
    LXValue _v86 = (nargs > 0) ? args[0] : px_null();
    LXValue _v87 = (nargs > 1) ? args[1] : px_null();
    LXValue _v88 = px_uninit();
    LXValue _v89 = px_uninit();
    LXValue _v90 = px_uninit();
    LXValue px_err_91_val = px_null();
    int px_err_91_proped = 0;
    px_srcpos("sv_best", 270);
    _v88 = px_null();
    px_srcpos("sv_best", 271);
    _v89 = px_int(0LL);
    px_srcpos("sv_best", 272);
    while (px_is_truthy(px_lt(_v89, px_call(px_get_global("len"), (LXValue[]){_v86}, 1)))) {
        px_srcpos("sv_best", 273);
        _v90 = px_index(_v86, _v89);
        px_srcpos("sv_best", 274);
        if (px_is_truthy(px_call(px_get_global("sv_satisfies"), (LXValue[]){_v90, _v87}, 2))) {
            px_srcpos("sv_best", 275);
            if (px_is_truthy(({ LXValue _t92 = px_eq(_v88, px_null()); px_is_truthy(_t92) ? _t92 : px_gt(px_call(px_get_global("sv_cmp"), (LXValue[]){_v90, _v88}, 2), px_int(0LL)); }))) {
                px_srcpos("sv_best", 276);
                 _v88 = _v90;
            }
        }
        px_srcpos("sv_best", 277);
         _v89 = px_add(_v89, px_int(1LL));
    }
    px_srcpos("sv_best", 278);
    return _v88;
px_err_91:
    if (px_err_91_proped) return px_err_91_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    if (nargs > 0) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "main", 0, nargs);
    LXValue _v93 = px_uninit();
    LXValue _v94 = px_uninit();
    LXValue px_err_95_val = px_null();
    int px_err_95_proped = 0;
    px_srcpos("main", 7);
    _v93 = px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2.3-alpha.1+build5")}, 1);
    px_srcpos("main", 8);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s35 = px_add(({ LXValue _s33 = px_add(px_add(px_str("major: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v93, px_str("major"))}, 1)), px_str(" minor: ")); LXValue _s34 = px_call(px_get_global("str"), (LXValue[]){px_index(_v93, px_str("minor"))}, 1); px_add(_s33, _s34); }), px_str(" patch: ")); LXValue _s36 = px_call(px_get_global("str"), (LXValue[]){px_index(_v93, px_str("patch"))}, 1); px_add(_s35, _s36); })}, 1));
    px_srcpos("main", 9);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("pre: "), px_index(_v93, px_str("pre"))), px_str(" build: ")), px_index(_v93, px_str("build")))}, 1));
    px_srcpos("main", 10);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s39 = px_add(({ LXValue _s37 = px_add(px_add(px_str("bad: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2")}, 1), px_null())}, 1)), px_str(" ")); LXValue _s38 = px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("01.2.3")}, 1), px_null())}, 1); px_add(_s37, _s38); }), px_str(" ")); LXValue _s40 = px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2.3-01")}, 1), px_null())}, 1); px_add(_s39, _s40); })}, 1));
    px_srcpos("main", 13);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1.2.3 vs 1.2.10: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.2.3"), px_str("1.2.10")}, 2)}, 1))}, 1));
    px_srcpos("main", 14);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1.0.0-alpha vs 1.0.0: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-alpha"), px_str("1.0.0")}, 2)}, 1))}, 1));
    px_srcpos("main", 15);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp alpha.2 vs alpha.10: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-alpha.2"), px_str("1.0.0-alpha.10")}, 2)}, 1))}, 1));
    px_srcpos("main", 16);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1 vs alpha: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-1"), px_str("1.0.0-alpha")}, 2)}, 1))}, 1));
    px_srcpos("main", 19);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s41 = px_add(px_add(px_str("^1.2.0: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.5"), px_str("^1.2.0")}, 2)}, 1)), px_str(" ")); LXValue _s42 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("2.0.0"), px_str("^1.2.0")}, 2)}, 1); px_add(_s41, _s42); })}, 1));
    px_srcpos("main", 20);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s43 = px_add(px_add(px_str("^0.2.3: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("0.2.9"), px_str("^0.2.3")}, 2)}, 1)), px_str(" ")); LXValue _s44 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("0.3.0"), px_str("^0.2.3")}, 2)}, 1); px_add(_s43, _s44); })}, 1));
    px_srcpos("main", 21);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s45 = px_add(px_add(px_str("~1.2.3: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.9"), px_str("~1.2.3")}, 2)}, 1)), px_str(" ")); LXValue _s46 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.3.0"), px_str("~1.2.3")}, 2)}, 1); px_add(_s45, _s46); })}, 1));
    px_srcpos("main", 22);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s47 = px_add(px_add(px_str("1.2.x: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.9"), px_str("1.2.x")}, 2)}, 1)), px_str(" ")); LXValue _s48 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.3.0"), px_str("1.2.x")}, 2)}, 1); px_add(_s47, _s48); })}, 1));
    px_srcpos("main", 23);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _s49 = px_add(px_add(px_str("exact: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.3"), px_str("1.2.3")}, 2)}, 1)), px_str(" ")); LXValue _s50 = px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.4"), px_str("1.2.3")}, 2)}, 1); px_add(_s49, _s50); })}, 1));
    px_srcpos("main", 24);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("pre reject: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.3-alpha"), px_str("1.2.3")}, 2)}, 1))}, 1));
    px_srcpos("main", 27);
    _v94 = px_list_n((LXValue[]){px_str("1.2.0"), px_str("1.2.5"), px_str("2.0.0"), px_str("1.0.0")}, 4);
    px_srcpos("main", 28);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^1.2.0: "), px_call(px_get_global("sv_best"), (LXValue[]){_v94, px_str("^1.2.0")}, 2))}, 1));
    px_srcpos("main", 29);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^2.0.0: "), px_call(px_get_global("sv_best"), (LXValue[]){_v94, px_str("^2.0.0")}, 2))}, 1));
    px_srcpos("main", 30);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^0.5.0: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_best"), (LXValue[]){_v94, px_str("^0.5.0")}, 2), px_null())}, 1))}, 1));
    px_srcpos("main", 31);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ~1.2: "), px_call(px_get_global("sv_best"), (LXValue[]){px_list_n((LXValue[]){px_str("1.2.0"), px_str("1.2.9"), px_str("1.3.0")}, 3), px_str("~1.2")}, 2))}, 1));
    px_srcpos("main", 32);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("semver ok")}, 1));
px_err_95:
    if (px_err_95_proped) return px_err_95_val;
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
      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）
    }
}
