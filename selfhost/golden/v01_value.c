#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v142 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_143_val = px_null();
    int px_err_143_proped = 0;
    return _v142;
px_err_143:
    if (px_err_143_proped) return px_err_143_val;
    return px_null();
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v144 = (nargs > 0) ? args[0] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_mul(_v144, px_int(2LL)); _blk; });
}

static LXValue fn_v_type(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    return px_call(px_get_global("type"), (LXValue[]){_v1}, 1);
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_v_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v3 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    LXValue _v5 = px_call(px_get_global("type"), (LXValue[]){_v3}, 1);
    if (px_is_truthy(px_eq(_v5, px_str("list")))) {
        LXValue _v6 = px_list_n((LXValue[]){}, 0);
        LXValue _t7 = _v3;
        for (int _t8 = 0; _t8 < px_len(_t7); _t8++) {
            LXValue _v9 = px_index(_t7, px_int(_t8));
            (void)(px_method(_v6, "append", (LXValue[]){px_call(px_get_global("v_str"), (LXValue[]){_v9}, 1)}, 1));
        }
        return px_add(px_add(px_str("["), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v6}, 2)), px_str("]"));
    }
    else if (px_is_truthy(px_eq(_v5, px_str("tuple")))) {
        LXValue _v10 = px_list_n((LXValue[]){}, 0);
        LXValue _t11 = _v3;
        for (int _t12 = 0; _t12 < px_len(_t11); _t12++) {
            LXValue _v13 = px_index(_t11, px_int(_t12));
            (void)(px_method(_v10, "append", (LXValue[]){px_call(px_get_global("v_str"), (LXValue[]){_v13}, 1)}, 1));
        }
        return px_add(px_add(px_str("("), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v10}, 2)), px_str(")"));
    }
    else if (px_is_truthy(px_eq(_v5, px_str("dict")))) {
        LXValue _v14 = px_list_n((LXValue[]){}, 0);
        LXValue _v15 = px_method(_v3, "keys", (LXValue[]){}, 0);
        LXValue _t16 = _v15;
        for (int _t17 = 0; _t17 < px_len(_t16); _t17++) {
            LXValue _v18 = px_index(_t16, px_int(_t17));
            (void)(px_method(_v14, "append", (LXValue[]){px_add(px_add(_v18, px_str(": ")), px_call(px_get_global("v_str"), (LXValue[]){px_index(_v3, _v18)}, 1))}, 1));
        }
        LXValue _v19 = px_call(px_get_global("sorted"), (LXValue[]){_v14}, 1);
        return px_add(px_add(px_str("{"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v19}, 2)), px_str("}"));
    }
    else {
        return px_call(px_get_global("str"), (LXValue[]){_v3}, 1);
    }
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

static LXValue fn_v_eq(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v20 = (nargs > 0) ? args[0] : px_null();
    LXValue _v21 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_22_val = px_null();
    int px_err_22_proped = 0;
    LXValue _v23 = px_call(px_get_global("type"), (LXValue[]){_v20}, 1);
    LXValue _v24 = px_call(px_get_global("type"), (LXValue[]){_v21}, 1);
    if (px_is_truthy(({ LXValue _t25 = px_eq(_v23, px_str("int")); px_is_truthy(_t25) ? px_eq(_v24, px_str("int")) : _t25; }))) {
        return px_eq(_v20, _v21);
    }
    if (px_is_truthy(({ LXValue _t26 = px_eq(_v23, px_str("int")); px_is_truthy(_t26) ? px_eq(_v24, px_str("float")) : _t26; }))) {
        return px_eq(px_call(px_get_global("float"), (LXValue[]){_v20}, 1), _v21);
    }
    if (px_is_truthy(({ LXValue _t27 = px_eq(_v23, px_str("float")); px_is_truthy(_t27) ? px_eq(_v24, px_str("int")) : _t27; }))) {
        return px_eq(_v20, px_call(px_get_global("float"), (LXValue[]){_v21}, 1));
    }
    if (px_is_truthy(({ LXValue _t28 = px_eq(_v23, px_str("float")); px_is_truthy(_t28) ? px_eq(_v24, px_str("float")) : _t28; }))) {
        return px_eq(_v20, _v21);
    }
    if (px_is_truthy(({ LXValue _t29 = px_eq(_v23, px_str("string")); px_is_truthy(_t29) ? px_eq(_v24, px_str("string")) : _t29; }))) {
        return px_eq(_v20, _v21);
    }
    if (px_is_truthy(({ LXValue _t30 = px_eq(_v23, px_str("bool")); px_is_truthy(_t30) ? px_eq(_v24, px_str("bool")) : _t30; }))) {
        return px_eq(_v20, _v21);
    }
    if (px_is_truthy(({ LXValue _t31 = px_eq(_v23, px_str("null")); px_is_truthy(_t31) ? px_eq(_v24, px_str("null")) : _t31; }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t32 = px_eq(_v23, px_str("list")); px_is_truthy(_t32) ? px_eq(_v24, px_str("list")) : _t32; }))) {
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v20}, 1), px_call(px_get_global("len"), (LXValue[]){_v21}, 1)))) {
            return px_bool(false);
        }
        LXValue _t33 = px_call(px_get_global("range"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v20}, 1)}, 1);
        for (int _t34 = 0; _t34 < px_len(_t33); _t34++) {
            LXValue _v35 = px_index(_t33, px_int(_t34));
            if (px_is_truthy(px_not(px_call(px_get_global("v_eq"), (LXValue[]){px_index(_v20, _v35), px_index(_v21, _v35)}, 2)))) {
                return px_bool(false);
            }
        }
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t36 = px_eq(_v23, px_str("tuple")); px_is_truthy(_t36) ? px_eq(_v24, px_str("tuple")) : _t36; }))) {
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v20}, 1), px_call(px_get_global("len"), (LXValue[]){_v21}, 1)))) {
            return px_bool(false);
        }
        LXValue _t37 = px_call(px_get_global("range"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v20}, 1)}, 1);
        for (int _t38 = 0; _t38 < px_len(_t37); _t38++) {
            LXValue _v39 = px_index(_t37, px_int(_t38));
            if (px_is_truthy(px_not(px_call(px_get_global("v_eq"), (LXValue[]){px_index(_v20, _v39), px_index(_v21, _v39)}, 2)))) {
                return px_bool(false);
            }
        }
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t40 = px_eq(_v23, px_str("dict")); px_is_truthy(_t40) ? px_eq(_v24, px_str("dict")) : _t40; }))) {
        LXValue _v41 = px_method(_v20, "keys", (LXValue[]){}, 0);
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v41}, 1), px_call(px_get_global("len"), (LXValue[]){px_method(_v21, "keys", (LXValue[]){}, 0)}, 1)))) {
            return px_bool(false);
        }
        LXValue _t42 = _v41;
        for (int _t43 = 0; _t43 < px_len(_t42); _t43++) {
            LXValue _v44 = px_index(_t42, px_int(_t43));
            if (px_is_truthy(px_not(px_method(_v21, "has", (LXValue[]){_v44}, 1)))) {
                return px_bool(false);
            }
            if (px_is_truthy(px_not(px_call(px_get_global("v_eq"), (LXValue[]){px_index(_v20, _v44), px_index(_v21, _v44)}, 2)))) {
                return px_bool(false);
            }
        }
        return px_bool(true);
    }
    return px_eq(_v20, _v21);
px_err_22:
    if (px_err_22_proped) return px_err_22_val;
    return px_null();
}

static LXValue fn_v_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v45 = (nargs > 0) ? args[0] : px_null();
    LXValue _v46 = (nargs > 1) ? args[1] : px_null();
    LXValue _v47 = px_null();
    LXValue _v48 = px_null();
    LXValue px_err_49_val = px_null();
    int px_err_49_proped = 0;
    LXValue _v50 = px_call(px_get_global("type"), (LXValue[]){_v45}, 1);
    LXValue _v51 = px_call(px_get_global("type"), (LXValue[]){_v46}, 1);
    if (px_is_truthy(({ LXValue _t52 = px_eq(_v50, px_str("int")); px_is_truthy(_t52) ? px_eq(_v51, px_str("int")) : _t52; }))) {
        if (px_is_truthy(px_lt(_v45, _v46))) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(px_gt(_v45, _v46))) {
            return px_int(1LL);
        }
        return px_int(0LL);
    }
    if (px_is_truthy(({ LXValue _t55 = ({ LXValue _t53 = px_eq(_v50, px_str("int")); px_is_truthy(_t53) ? _t53 : px_eq(_v50, px_str("float")); }); px_is_truthy(_t55) ? ({ LXValue _t54 = px_eq(_v51, px_str("int")); px_is_truthy(_t54) ? _t54 : px_eq(_v51, px_str("float")); }) : _t55; }))) {
        LXValue _v56 = _v45;
        LXValue _v57 = _v46;
        if (px_is_truthy(px_eq(_v50, px_str("int")))) {
             _v56 = px_call(px_get_global("float"), (LXValue[]){_v45}, 1);
        }
        if (px_is_truthy(px_eq(_v51, px_str("int")))) {
             _v57 = px_call(px_get_global("float"), (LXValue[]){_v46}, 1);
        }
        if (px_is_truthy(px_lt(_v56, _v57))) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(px_gt(_v56, _v57))) {
            return px_int(1LL);
        }
        return px_int(0LL);
    }
    if (px_is_truthy(({ LXValue _t58 = px_eq(_v50, px_str("string")); px_is_truthy(_t58) ? px_eq(_v51, px_str("string")) : _t58; }))) {
        if (px_is_truthy(px_lt(_v45, _v46))) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(px_gt(_v45, _v46))) {
            return px_int(1LL);
        }
        return px_int(0LL);
    }
    return px_null();
px_err_49:
    if (px_err_49_proped) return px_err_49_val;
    return px_null();
}

static LXValue fn_v_arith(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v59 = (nargs > 0) ? args[0] : px_null();
    LXValue _v60 = (nargs > 1) ? args[1] : px_null();
    LXValue _v61 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_62_val = px_null();
    int px_err_62_proped = 0;
    LXValue _v63 = px_call(px_get_global("type"), (LXValue[]){_v60}, 1);
    LXValue _v64 = px_call(px_get_global("type"), (LXValue[]){_v61}, 1);
    if (px_is_truthy(px_eq(_v59, px_str("+")))) {
        if (px_is_truthy(({ LXValue _t65 = px_eq(_v63, px_str("int")); px_is_truthy(_t65) ? px_eq(_v64, px_str("int")) : _t65; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v60, _v61)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t66 = px_eq(_v63, px_str("int")); px_is_truthy(_t66) ? px_eq(_v64, px_str("float")) : _t66; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("float"), (LXValue[]){_v60}, 1), _v61)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t67 = px_eq(_v63, px_str("float")); px_is_truthy(_t67) ? px_eq(_v64, px_str("int")) : _t67; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v60, px_call(px_get_global("float"), (LXValue[]){_v61}, 1))); } _d; });
        }
        if (px_is_truthy(({ LXValue _t68 = px_eq(_v63, px_str("float")); px_is_truthy(_t68) ? px_eq(_v64, px_str("float")) : _t68; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v60, _v61)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t69 = px_eq(_v63, px_str("string")); px_is_truthy(_t69) ? px_eq(_v64, px_str("string")) : _t69; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v60, _v61)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t70 = px_eq(_v63, px_str("list")); px_is_truthy(_t70) ? px_eq(_v64, px_str("list")) : _t70; }))) {
            LXValue _v71 = px_list_n((LXValue[]){}, 0);
            LXValue _t72 = _v60;
            for (int _t73 = 0; _t73 < px_len(_t72); _t73++) {
                LXValue _v74 = px_index(_t72, px_int(_t73));
                (void)(px_method(_v71, "append", (LXValue[]){_v74}, 1));
            }
            LXValue _t75 = _v61;
            for (int _t76 = 0; _t76 < px_len(_t75); _t76++) {
                LXValue _v77 = px_index(_t75, px_int(_t76));
                (void)(px_method(_v71, "append", (LXValue[]){_v77}, 1));
            }
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v71); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_str("+ 不支持: "), _v63), px_str(" + ")), _v64)); } _d; });
    }
    if (px_is_truthy(({ LXValue _t78 = px_eq(_v63, px_str("int")); px_is_truthy(_t78) ? px_eq(_v64, px_str("int")) : _t78; }))) {
        return px_call(px_get_global("v_int_op"), (LXValue[]){_v59, _v60, _v61}, 3);
    }
    return px_call(px_get_global("v_float_op"), (LXValue[]){_v59, _v60, _v61}, 3);
px_err_62:
    if (px_err_62_proped) return px_err_62_val;
    return px_null();
}

static LXValue fn_v_int_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v79 = (nargs > 0) ? args[0] : px_null();
    LXValue _v80 = (nargs > 1) ? args[1] : px_null();
    LXValue _v81 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_82_val = px_null();
    int px_err_82_proped = 0;
    if (px_is_truthy(px_eq(_v79, px_str("-")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_sub(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("*")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mul(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("/")))) {
        if (px_is_truthy(px_eq(_v81, px_int(0LL)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_div(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("//")))) {
        if (px_is_truthy(px_eq(_v81, px_int(0LL)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_idiv(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("%")))) {
        if (px_is_truthy(px_eq(_v81, px_int(0LL)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mod(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("**")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_pow(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("<<")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_shl(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str(">>")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_shr(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str(">>>")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_ushr(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("&")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bitand(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("|")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bitor(_v80, _v81)); } _d; });
    }
    if (px_is_truthy(px_eq(_v79, px_str("^")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bitxor(_v80, _v81)); } _d; });
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_str("未知运算符 "), _v79)); } _d; });
px_err_82:
    if (px_err_82_proped) return px_err_82_val;
    return px_null();
}

static LXValue fn_v_float_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v83 = (nargs > 0) ? args[0] : px_null();
    LXValue _v84 = (nargs > 1) ? args[1] : px_null();
    LXValue _v85 = (nargs > 2) ? args[2] : px_null();
    LXValue _v86 = px_null();
    LXValue _v87 = px_null();
    LXValue px_err_88_val = px_null();
    int px_err_88_proped = 0;
    LXValue _v89 = px_call(px_get_global("type"), (LXValue[]){_v84}, 1);
    LXValue _v90 = px_call(px_get_global("type"), (LXValue[]){_v85}, 1);
    if (px_is_truthy(({ LXValue _t91 = px_ne(_v89, px_str("int")); px_is_truthy(_t91) ? px_ne(_v89, px_str("float")) : _t91; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("需要数值")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t92 = px_ne(_v90, px_str("int")); px_is_truthy(_t92) ? px_ne(_v90, px_str("float")) : _t92; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("需要数值")); } _d; });
    }
    LXValue _v93 = _v84;
    LXValue _v94 = _v85;
    if (px_is_truthy(px_eq(_v89, px_str("int")))) {
         _v93 = px_call(px_get_global("float"), (LXValue[]){_v84}, 1);
    }
    if (px_is_truthy(px_eq(_v90, px_str("int")))) {
         _v94 = px_call(px_get_global("float"), (LXValue[]){_v85}, 1);
    }
    if (px_is_truthy(px_eq(_v83, px_str("-")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_sub(_v93, _v94)); } _d; });
    }
    if (px_is_truthy(px_eq(_v83, px_str("*")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mul(_v93, _v94)); } _d; });
    }
    if (px_is_truthy(px_eq(_v83, px_str("/")))) {
        if (px_is_truthy(px_eq(_v94, px_float(0)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_div(_v93, _v94)); } _d; });
    }
    if (px_is_truthy(px_eq(_v83, px_str("//")))) {
        if (px_is_truthy(px_eq(_v94, px_float(0)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_idiv(_v93, _v94)); } _d; });
    }
    if (px_is_truthy(px_eq(_v83, px_str("%")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mod(_v93, _v94)); } _d; });
    }
    if (px_is_truthy(px_eq(_v83, px_str("**")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_pow(_v93, _v94)); } _d; });
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("此运算符要求整数操作数")); } _d; });
px_err_88:
    if (px_err_88_proped) return px_err_88_val;
    return px_null();
}

static LXValue fn_v_truthy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v95 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_96_val = px_null();
    int px_err_96_proped = 0;
    LXValue _v97 = px_call(px_get_global("type"), (LXValue[]){_v95}, 1);
    if (px_is_truthy(px_eq(_v97, px_str("null")))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(_v97, px_str("bool")))) {
        return _v95;
    }
    if (px_is_truthy(px_eq(_v97, px_str("int")))) {
        return px_ne(_v95, px_int(0LL));
    }
    if (px_is_truthy(px_eq(_v97, px_str("float")))) {
        return px_ne(_v95, px_float(0));
    }
    if (px_is_truthy(px_eq(_v97, px_str("string")))) {
        return px_gt(px_call(px_get_global("len"), (LXValue[]){_v95}, 1), px_int(0LL));
    }
    if (px_is_truthy(px_eq(_v97, px_str("list")))) {
        return px_gt(px_call(px_get_global("len"), (LXValue[]){_v95}, 1), px_int(0LL));
    }
    if (px_is_truthy(px_eq(_v97, px_str("dict")))) {
        return px_gt(px_call(px_get_global("len"), (LXValue[]){_v95}, 1), px_int(0LL));
    }
    return px_bool(true);
px_err_96:
    if (px_err_96_proped) return px_err_96_val;
    return px_null();
}

static LXValue fn_v_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v98 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_99_val = px_null();
    int px_err_99_proped = 0;
    return px_call(px_get_global("int"), (LXValue[]){_v98}, 1);
px_err_99:
    if (px_err_99_proped) return px_err_99_val;
    return px_null();
}

static LXValue fn_v_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v100 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_101_val = px_null();
    int px_err_101_proped = 0;
    return px_call(px_get_global("float"), (LXValue[]){_v100}, 1);
px_err_101:
    if (px_err_101_proped) return px_err_101_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v102 = (nargs > 0) ? args[0] : px_null();
    LXValue _v103 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_104_val = px_null();
    int px_err_104_proped = 0;
    if (px_is_truthy(_v103)) {
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v102)}, 1));
    }
    else {
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v102)}, 1));
    }
px_err_104:
    if (px_err_104_proped) return px_err_104_val;
    return px_null();
}

static LXValue fn_check_value(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v105 = (nargs > 0) ? args[0] : px_null();
    LXValue _v106 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_107_val = px_null();
    int px_err_107_proped = 0;
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v105, px_str(" v_type")), px_eq(px_call(px_get_global("v_type"), (LXValue[]){_v106}, 1), px_call(px_get_global("type"), (LXValue[]){_v106}, 1))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v105, px_str(" v_str")), px_eq(px_call(px_get_global("v_str"), (LXValue[]){_v106}, 1), px_call(px_get_global("str"), (LXValue[]){_v106}, 1))}, 2));
px_err_107:
    if (px_err_107_proped) return px_err_107_val;
    return px_null();
}

static LXValue fn_check_eq(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v108 = (nargs > 0) ? args[0] : px_null();
    LXValue _v109 = (nargs > 1) ? args[1] : px_null();
    LXValue _v110 = (nargs > 2) ? args[2] : px_null();
    LXValue _v111 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_112_val = px_null();
    int px_err_112_proped = 0;
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v108, px_str(" v_eq")), px_eq(px_call(px_get_global("v_eq"), (LXValue[]){_v109, _v110}, 2), _v111)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v108, px_str(" 内置")), px_eq(px_eq(_v109, _v110), _v111)}, 2));
px_err_112:
    if (px_err_112_proped) return px_err_112_val;
    return px_null();
}

static LXValue fn_check_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v113 = (nargs > 0) ? args[0] : px_null();
    LXValue _v114 = (nargs > 1) ? args[1] : px_null();
    LXValue _v115 = (nargs > 2) ? args[2] : px_null();
    LXValue _v116 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_117_val = px_null();
    int px_err_117_proped = 0;
    LXValue _v118 = px_call(px_get_global("v_cmp"), (LXValue[]){_v114, _v115}, 2);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" v_cmp")), px_eq(_v118, _v116)}, 2));
    if (px_is_truthy(px_eq(_v116, px_neg(px_int(1LL))))) {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" 内置<")), ({ LXValue _t119 = px_eq(px_lt(_v114, _v115), px_bool(true)); px_is_truthy(_t119) ? px_eq(px_gt(_v114, _v115), px_bool(false)) : _t119; })}, 2));
    }
    else if (px_is_truthy(px_eq(_v116, px_int(1LL)))) {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" 内置>")), ({ LXValue _t120 = px_eq(px_gt(_v114, _v115), px_bool(true)); px_is_truthy(_t120) ? px_eq(px_lt(_v114, _v115), px_bool(false)) : _t120; })}, 2));
    }
    else {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" 内置==")), ({ LXValue _t121 = px_eq(px_lt(_v114, _v115), px_bool(false)); px_is_truthy(_t121) ? px_eq(px_gt(_v114, _v115), px_bool(false)) : _t121; })}, 2));
    }
px_err_117:
    if (px_err_117_proped) return px_err_117_val;
    return px_null();
}

static LXValue fn_check_arith(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v122 = (nargs > 0) ? args[0] : px_null();
    LXValue _v123 = (nargs > 1) ? args[1] : px_null();
    LXValue _v124 = (nargs > 2) ? args[2] : px_null();
    LXValue _v125 = (nargs > 3) ? args[3] : px_null();
    LXValue _v126 = (nargs > 4) ? args[4] : px_null();
    LXValue px_err_127_val = px_null();
    int px_err_127_proped = 0;
    LXValue _v128 = px_call(px_get_global("v_arith"), (LXValue[]){_v123, _v124, _v125}, 3);
    if (px_is_truthy(px_index(_v128, px_str("ok")))) {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v122, px_str(" v_arith")), px_eq(px_index(_v128, px_str("v")), _v126)}, 2));
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v122, px_str(" str")), px_eq(px_call(px_get_global("str"), (LXValue[]){px_index(_v128, px_str("v"))}, 1), px_call(px_get_global("str"), (LXValue[]){_v126}, 1))}, 2));
    }
    else {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v122, px_str(" v_arith 错误")), px_bool(false)}, 2));
    }
px_err_127:
    if (px_err_127_proped) return px_err_127_val;
    return px_null();
}

static LXValue fn_check_arith_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v129 = (nargs > 0) ? args[0] : px_null();
    LXValue _v130 = (nargs > 1) ? args[1] : px_null();
    LXValue _v131 = (nargs > 2) ? args[2] : px_null();
    LXValue _v132 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_133_val = px_null();
    int px_err_133_proped = 0;
    LXValue _v134 = px_call(px_get_global("v_arith"), (LXValue[]){_v130, _v131, _v132}, 3);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v129, px_str(" 报错")), px_not(px_index(_v134, px_str("ok")))}, 2));
px_err_133:
    if (px_err_133_proped) return px_err_133_val;
    return px_null();
}

static LXValue fn_check_truthy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v135 = (nargs > 0) ? args[0] : px_null();
    LXValue _v136 = (nargs > 1) ? args[1] : px_null();
    LXValue _v137 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_138_val = px_null();
    int px_err_138_proped = 0;
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v135, px_str(" v_truthy")), px_eq(px_call(px_get_global("v_truthy"), (LXValue[]){_v136}, 1), _v137)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v135, px_str(" 内置bool")), px_eq(px_call(px_get_global("bool"), (LXValue[]){_v136}, 1), _v137)}, 2));
px_err_138:
    if (px_err_138_proped) return px_err_138_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("v_type", px_func("v_type", fn_v_type, NULL));
    px_set_global("v_str", px_func("v_str", fn_v_str, NULL));
    px_set_global("v_eq", px_func("v_eq", fn_v_eq, NULL));
    px_set_global("v_cmp", px_func("v_cmp", fn_v_cmp, NULL));
    px_set_global("v_arith", px_func("v_arith", fn_v_arith, NULL));
    px_set_global("v_int_op", px_func("v_int_op", fn_v_int_op, NULL));
    px_set_global("v_float_op", px_func("v_float_op", fn_v_float_op, NULL));
    px_set_global("v_truthy", px_func("v_truthy", fn_v_truthy, NULL));
    px_set_global("v_int", px_func("v_int", fn_v_int, NULL));
    px_set_global("v_float", px_func("v_float", fn_v_float, NULL));
    px_set_global("check", px_func("check", fn_check, NULL));
    px_set_global("check_value", px_func("check_value", fn_check_value, NULL));
    px_set_global("check_eq", px_func("check_eq", fn_check_eq, NULL));
    px_set_global("check_cmp", px_func("check_cmp", fn_check_cmp, NULL));
    px_set_global("check_arith", px_func("check_arith", fn_check_arith, NULL));
    px_set_global("check_arith_err", px_func("check_arith_err", fn_check_arith_err, NULL));
    px_set_global("check_truthy", px_func("check_truthy", fn_check_truthy, NULL));
    px_set_global("g_pass", px_int(0LL));
    px_set_global("g_fail", px_int(0LL));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_type / v_str 原子类型 ==")}, 1));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("int"), px_int(42LL)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("neg int"), px_neg(px_int(7LL))}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 整"), px_float(3)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 小"), px_float(3.14)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 大"), px_float(1000000000000000)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 超大"), px_float(10000000000000000)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 负"), px_neg(px_float(2.5))}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 0.5"), px_float(0.5)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float sci"), px_float(15000000000)}, 2));
    px_set_global("bigv", px_float(10000000000000000));
    px_set_global("f_inf", px_get_global("bigv"));
    LXValue _t139 = px_call(px_get_global("range"), (LXValue[]){px_int(40LL)}, 1);
    for (int _t140 = 0; _t140 < px_len(_t139); _t140++) {
        LXValue _v141 = px_index(_t139, px_int(_t140));
        px_set_global("f_inf", px_mul(px_get_global("f_inf"), px_get_global("bigv")));
    }
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float inf"), px_get_global("f_inf")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float -inf"), px_sub(px_int(0LL), px_get_global("f_inf"))}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float 0.0"), px_float(0)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float -0.0"), px_neg(px_float(0))}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("float tiny"), px_float(0.0000000001)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("str"), px_str("hello")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("str 中文"), px_str("普贤px")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("str 转义"), px_str("a\"b\\c\nd\te")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("bool true"), px_bool(true)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("bool false"), px_bool(false)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("null"), px_null()}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("int max"), px_int(9223372036854775807LL)}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_str 容器 ==")}, 1));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("list"), px_list_n((LXValue[]){px_int(1LL), px_float(2.5), px_str("a"), px_bool(true), px_null()}, 5)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("list 空"), px_list_n((LXValue[]){}, 0)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("list 嵌套"), px_list_n((LXValue[]){px_int(1LL), px_list_n((LXValue[]){px_int(2LL), px_int(3LL)}, 2), ({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_int(4LL), px_int(5LL)}, 2)); } _d; })}, 3)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("list 字符串元素"), px_list_n((LXValue[]){px_str("a\"b"), px_str("x\ny")}, 2)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("tuple"), px_tuple((LXValue[]){px_int(1LL), px_str("x")}, 2)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("tuple 单元素"), px_tuple((LXValue[]){px_int(1LL)}, 1)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("tuple 空"), px_tuple((LXValue[]){}, 0)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("dict"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; })}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("dict 键排序边界"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("a1"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; })}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("dict 键排序2"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("ab"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; })}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("dict 嵌套"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ({ LXValue _d = px_dict(); { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; })); } { LXValue _k = px_str("c"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2)); } _d; })}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("dict 多键"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("z"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("y"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(3LL)); } _d; })}, 2));
    px_set_global("ed", ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; }));
    (void)(px_method(px_get_global("ed"), "remove", (LXValue[]){px_str("_")}, 1));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("dict 空"), px_get_global("ed")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("range"), px_call(px_get_global("range"), (LXValue[]){px_int(1LL), px_int(5LL), px_int(2LL)}, 3)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("range 默认"), px_call(px_get_global("range"), (LXValue[]){px_int(3LL)}, 1)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("gen"), px_gen_lazy(px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3), px_func("<closure1>", fn_closure_1, NULL), px_null())}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("闭包 fn"), px_func("<closure2>", fn_closure_2, NULL)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("builtin"), px_get_global("print")}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_type 对象类型 ==")}, 1));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("enum 值"), px_enum("Color", "Red")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("enum type"), px_enum("Color", "Green")}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("struct 实例"), px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("struct type"), px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_int(3LL), px_int(4LL)}, 2)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("result Ok"), px_call(px_get_global("Ok"), (LXValue[]){px_int(42LL)}, 1)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("result Err"), px_call(px_get_global("Err"), (LXValue[]){px_str("bad")}, 1)}, 2));
    (void)(px_call(px_get_global("check_value"), (LXValue[]){px_str("result 嵌套"), px_call(px_get_global("Ok"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 1)}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_eq 相等矩阵 ==")}, 1));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("int==int"), px_int(1LL), px_int(1LL), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("int!=int"), px_int(1LL), px_int(2LL), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("int==float"), px_int(1LL), px_float(1), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("int!=float"), px_int(1LL), px_float(1.5), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("float==float"), px_float(2.5), px_float(2.5), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("str==str"), px_str("a"), px_str("a"), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("null==null"), px_null(), px_null(), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("bool==bool"), px_bool(true), px_bool(true), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("bool!=int"), px_bool(true), px_int(1LL), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("list==list"), px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("list!=list"), px_list_n((LXValue[]){px_int(1LL)}, 1), px_list_n((LXValue[]){px_int(2LL)}, 1), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("list 嵌套"), px_list_n((LXValue[]){px_int(1LL), px_list_n((LXValue[]){px_int(2LL)}, 1)}, 2), px_list_n((LXValue[]){px_int(1LL), px_list_n((LXValue[]){px_int(2LL)}, 1)}, 2), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("list 嵌套不等"), px_list_n((LXValue[]){px_int(1LL), px_list_n((LXValue[]){px_int(2LL)}, 1)}, 2), px_list_n((LXValue[]){px_int(1LL), px_list_n((LXValue[]){px_int(3LL)}, 1)}, 2), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("dict==dict"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("dict!=dict"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; }), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("dict 键序无关"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; }), ({ LXValue _d = px_dict(); { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("enum==enum"), px_enum("Color", "Red"), px_enum("Color", "Red"), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("enum!=enum"), px_enum("Color", "Red"), px_enum("Color", "Blue"), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("Ok==Ok"), px_call(px_get_global("Ok"), (LXValue[]){px_int(1LL)}, 1), px_call(px_get_global("Ok"), (LXValue[]){px_int(1LL)}, 1), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("Ok!=Err"), px_call(px_get_global("Ok"), (LXValue[]){px_int(1LL)}, 1), px_call(px_get_global("Err"), (LXValue[]){px_int(1LL)}, 1), px_bool(false)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("tuple==tuple"), px_tuple((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_tuple((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_bool(true)}, 4));
    (void)(px_call(px_get_global("check_eq"), (LXValue[]){px_str("str!=int"), px_str("1"), px_int(1LL), px_bool(false)}, 4));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_cmp 顺序比较 ==")}, 1));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("int<int"), px_int(1LL), px_int(2LL), px_neg(px_int(1LL))}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("int>int"), px_int(3LL), px_int(2LL), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("int==int"), px_int(5LL), px_int(5LL), px_int(0LL)}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("int<float"), px_int(1LL), px_float(1.5), px_neg(px_int(1LL))}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("float<int"), px_float(2.5), px_int(2LL), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("float<float"), px_float(1), px_float(1), px_int(0LL)}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("str<str"), px_str("abc"), px_str("abd"), px_neg(px_int(1LL))}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("str>str"), px_str("z"), px_str("a"), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("负数"), px_neg(px_int(3LL)), px_neg(px_int(2LL)), px_neg(px_int(1LL))}, 4));
    (void)(px_call(px_get_global("check_cmp"), (LXValue[]){px_str("负float"), px_neg(px_float(1.5)), px_neg(px_float(1)), px_neg(px_int(1LL))}, 4));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list cmp 不支持"), px_eq(px_call(px_get_global("v_cmp"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL)}, 1), px_list_n((LXValue[]){px_int(2LL)}, 1)}, 2), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict cmp 不支持"), px_eq(px_call(px_get_global("v_cmp"), (LXValue[]){({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; })}, 2), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("bool cmp 不支持"), px_eq(px_call(px_get_global("v_cmp"), (LXValue[]){px_bool(true), px_bool(false)}, 2), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("null cmp 不支持"), px_eq(px_call(px_get_global("v_cmp"), (LXValue[]){px_null(), px_null()}, 2), px_null())}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_arith 算术 ==")}, 1));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int+int"), px_str("+"), px_int(7LL), px_int(2LL), px_int(9LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int+float"), px_str("+"), px_int(1LL), px_float(2.5), px_float(3.5)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float+float"), px_str("+"), px_float(1.5), px_float(2.5), px_float(4)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("str+str"), px_str("+"), px_str("a"), px_str("b"), px_str("ab")}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("list+list"), px_str("+"), px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_list_n((LXValue[]){px_int(3LL)}, 1), px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int-int"), px_str("-"), px_int(7LL), px_int(2LL), px_int(5LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int*int"), px_str("*"), px_int(7LL), px_int(2LL), px_int(14LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int/int"), px_str("/"), px_int(7LL), px_int(2LL), px_float(3.5)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int//int"), px_str("//"), px_int(7LL), px_int(2LL), px_int(3LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("neg//"), px_str("//"), px_neg(px_int(7LL)), px_int(2LL), px_neg(px_int(4LL))}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("neg//2"), px_str("//"), px_int(7LL), px_neg(px_int(2LL)), px_neg(px_int(3LL))}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("int%int"), px_str("%"), px_int(7LL), px_int(3LL), px_int(1LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("neg%"), px_str("%"), px_neg(px_int(7LL)), px_int(3LL), px_int(2LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("pow"), px_str("**"), px_int(2LL), px_int(10LL), px_int(1024LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("pow 负指数"), px_str("**"), px_int(2LL), px_neg(px_int(1LL)), px_float(0.5)}, 5));
    px_set_global("rb_big", px_call(px_get_global("v_arith"), (LXValue[]){px_str("**"), px_int(10LL), px_int(20LL)}, 3));
    px_set_global("native_big", px_pow(px_int(10LL), px_int(20LL)));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("pow 大 ok"), px_index(px_get_global("rb_big"), px_str("ok"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("pow 大 值一致"), px_eq(px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("rb_big"), px_str("v"))}, 1), px_call(px_get_global("str"), (LXValue[]){px_get_global("native_big")}, 1))}, 2));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("shl"), px_str("<<"), px_int(1LL), px_int(4LL), px_int(16LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("shr"), px_str(">>"), px_int(16LL), px_int(2LL), px_int(4LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("shru"), px_str(">>>"), px_neg(px_int(1LL)), px_int(1LL), px_int(9223372036854775807LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("and"), px_str("&"), px_int(5LL), px_int(3LL), px_int(1LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("or"), px_str("|"), px_int(5LL), px_int(3LL), px_int(7LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("xor"), px_str("^"), px_int(5LL), px_int(3LL), px_int(6LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float+int"), px_str("+"), px_float(2.5), px_int(1LL), px_float(3.5)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float*float"), px_str("*"), px_float(2.5), px_float(2), px_float(5)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float/"), px_str("/"), px_float(7), px_int(2LL), px_float(3.5)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float//"), px_str("//"), px_float(5.5), px_int(2LL), px_int(2LL)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float//neg"), px_str("//"), px_neg(px_float(5.5)), px_int(2LL), px_neg(px_int(3LL))}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float%"), px_str("%"), px_float(5.5), px_int(2LL), px_float(1.5)}, 5));
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float**"), px_str("**"), px_float(2), px_float(0.5), px_float(1.4142135623730951)}, 5));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("div by zero int"), px_str("/"), px_int(7LL), px_int(0LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("idiv by zero"), px_str("//"), px_int(7LL), px_int(0LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("mod by zero"), px_str("%"), px_int(7LL), px_int(0LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("div by zero float"), px_str("/"), px_float(7), px_float(0)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("str*int 不支持"), px_str("*"), px_str("ab"), px_int(3LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("int+str 不支持"), px_str("+"), px_int(1LL), px_str("a")}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("str+int 不支持"), px_str("+"), px_str("a"), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("list+int 不支持"), px_str("+"), px_list_n((LXValue[]){px_int(1LL)}, 1), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("str>> 不支持"), px_str(">>"), px_str("a"), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("float&int 位运算不支持"), px_str("&"), px_float(1.5), px_int(1LL)}, 4));
    (void)(px_call(px_get_global("check_arith_err"), (LXValue[]){px_str("dict+dict 不支持"), px_str("+"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), ({ LXValue _d = px_dict(); { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; })}, 4));
    px_set_global("r0", px_call(px_get_global("v_arith"), (LXValue[]){px_str("/"), px_int(7LL), px_int(0LL)}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("除零错误消息"), ({ LXValue _t145 = px_not(px_index(px_get_global("r0"), px_str("ok"))); px_is_truthy(_t145) ? px_eq(px_index(px_get_global("r0"), px_str("err")), px_str("除零错误")) : _t145; })}, 2));
    px_set_global("r1", px_call(px_get_global("v_arith"), (LXValue[]){px_str("+"), px_int(1LL), px_str("a")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("+ 不支持消息"), ({ LXValue _t146 = px_not(px_index(px_get_global("r1"), px_str("ok"))); px_is_truthy(_t146) ? px_eq(px_index(px_get_global("r1"), px_str("err")), px_str("+ 不支持: int + string")) : _t146; })}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== v_truthy 真值 ==")}, 1));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("null"), px_null(), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("false"), px_bool(false), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("true"), px_bool(true), px_bool(true)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("0"), px_int(0LL), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("1"), px_int(1LL), px_bool(true)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("-1"), px_neg(px_int(1LL)), px_bool(true)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("0.0"), px_float(0), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("0.5"), px_float(0.5), px_bool(true)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("空串"), px_str(""), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("非空串"), px_str("a"), px_bool(true)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("空list"), px_list_n((LXValue[]){}, 0), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("非空list"), px_list_n((LXValue[]){px_int(0LL)}, 1), px_bool(true)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("空dict"), px_get_global("ed"), px_bool(false)}, 3));
    (void)(px_call(px_get_global("check_truthy"), (LXValue[]){px_str("非空dict"), ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } _d; }), px_bool(true)}, 3));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("结果: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_pass")}, 1)), px_str(" PASS / ")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_fail")}, 1)), px_str(" FAIL"))}, 1));
    if (px_is_truthy(px_gt(px_get_global("g_fail"), px_int(0LL)))) {
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    return 0;
}
