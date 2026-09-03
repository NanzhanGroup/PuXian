#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v133 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_134_val = px_null();
    int px_err_134_proped = 0;
    return _v133;
px_err_134:
    if (px_err_134_proped) return px_err_134_val;
    return px_null();
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v135 = (nargs > 0) ? args[0] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_mul(_v135, px_int(2LL)); _blk; });
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
    LXValue _v4 = px_null();
    LXValue _v5 = px_null();
    LXValue _v6 = px_null();
    LXValue _v7 = px_null();
    LXValue _v8 = px_null();
    LXValue _v9 = px_null();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    _v4 = px_call(px_get_global("type"), (LXValue[]){_v3}, 1);
    if (px_is_truthy(px_eq(_v4, px_str("list")))) {
        _v5 = px_list_n((LXValue[]){}, 0);
        LXValue _t11 = _v3;
        for (int _t12 = 0; _t12 < px_len(_t11); _t12++) {
            _v6 = px_index(_t11, px_int(_t12));
            (void)(px_method(_v5, "append", (LXValue[]){px_call(px_get_global("v_str"), (LXValue[]){_v6}, 1)}, 1));
        }
        return px_add(px_add(px_str("["), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v5}, 2)), px_str("]"));
    }
    else if (px_is_truthy(px_eq(_v4, px_str("tuple")))) {
        _v5 = px_list_n((LXValue[]){}, 0);
        LXValue _t13 = _v3;
        for (int _t14 = 0; _t14 < px_len(_t13); _t14++) {
            _v6 = px_index(_t13, px_int(_t14));
            (void)(px_method(_v5, "append", (LXValue[]){px_call(px_get_global("v_str"), (LXValue[]){_v6}, 1)}, 1));
        }
        return px_add(px_add(px_str("("), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v5}, 2)), px_str(")"));
    }
    else if (px_is_truthy(px_eq(_v4, px_str("dict")))) {
        _v5 = px_list_n((LXValue[]){}, 0);
        _v7 = px_method(_v3, "keys", (LXValue[]){}, 0);
        LXValue _t15 = _v7;
        for (int _t16 = 0; _t16 < px_len(_t15); _t16++) {
            _v8 = px_index(_t15, px_int(_t16));
            (void)(px_method(_v5, "append", (LXValue[]){px_add(px_add(_v8, px_str(": ")), px_call(px_get_global("v_str"), (LXValue[]){px_index(_v3, _v8)}, 1))}, 1));
        }
        _v9 = px_call(px_get_global("sorted"), (LXValue[]){_v5}, 1);
        return px_add(px_add(px_str("{"), px_call(px_get_global("join"), (LXValue[]){px_str(", "), _v9}, 2)), px_str("}"));
    }
    else {
        return px_call(px_get_global("str"), (LXValue[]){_v3}, 1);
    }
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_v_eq(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v17 = (nargs > 0) ? args[0] : px_null();
    LXValue _v18 = (nargs > 1) ? args[1] : px_null();
    LXValue _v19 = px_null();
    LXValue _v20 = px_null();
    LXValue _v21 = px_null();
    LXValue _v22 = px_null();
    LXValue _v23 = px_null();
    LXValue px_err_24_val = px_null();
    int px_err_24_proped = 0;
    _v19 = px_call(px_get_global("type"), (LXValue[]){_v17}, 1);
    _v20 = px_call(px_get_global("type"), (LXValue[]){_v18}, 1);
    if (px_is_truthy(({ LXValue _t25 = px_eq(_v19, px_str("int")); px_is_truthy(_t25) ? px_eq(_v20, px_str("int")) : _t25; }))) {
        return px_eq(_v17, _v18);
    }
    if (px_is_truthy(({ LXValue _t26 = px_eq(_v19, px_str("int")); px_is_truthy(_t26) ? px_eq(_v20, px_str("float")) : _t26; }))) {
        return px_eq(px_call(px_get_global("float"), (LXValue[]){_v17}, 1), _v18);
    }
    if (px_is_truthy(({ LXValue _t27 = px_eq(_v19, px_str("float")); px_is_truthy(_t27) ? px_eq(_v20, px_str("int")) : _t27; }))) {
        return px_eq(_v17, px_call(px_get_global("float"), (LXValue[]){_v18}, 1));
    }
    if (px_is_truthy(({ LXValue _t28 = px_eq(_v19, px_str("float")); px_is_truthy(_t28) ? px_eq(_v20, px_str("float")) : _t28; }))) {
        return px_eq(_v17, _v18);
    }
    if (px_is_truthy(({ LXValue _t29 = px_eq(_v19, px_str("string")); px_is_truthy(_t29) ? px_eq(_v20, px_str("string")) : _t29; }))) {
        return px_eq(_v17, _v18);
    }
    if (px_is_truthy(({ LXValue _t30 = px_eq(_v19, px_str("bool")); px_is_truthy(_t30) ? px_eq(_v20, px_str("bool")) : _t30; }))) {
        return px_eq(_v17, _v18);
    }
    if (px_is_truthy(({ LXValue _t31 = px_eq(_v19, px_str("null")); px_is_truthy(_t31) ? px_eq(_v20, px_str("null")) : _t31; }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t32 = px_eq(_v19, px_str("list")); px_is_truthy(_t32) ? px_eq(_v20, px_str("list")) : _t32; }))) {
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v17}, 1), px_call(px_get_global("len"), (LXValue[]){_v18}, 1)))) {
            return px_bool(false);
        }
        LXValue _t33 = px_call(px_get_global("range"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v17}, 1)}, 1);
        for (int _t34 = 0; _t34 < px_len(_t33); _t34++) {
            _v21 = px_index(_t33, px_int(_t34));
            if (px_is_truthy(px_not(px_call(px_get_global("v_eq"), (LXValue[]){px_index(_v17, _v21), px_index(_v18, _v21)}, 2)))) {
                return px_bool(false);
            }
        }
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t35 = px_eq(_v19, px_str("tuple")); px_is_truthy(_t35) ? px_eq(_v20, px_str("tuple")) : _t35; }))) {
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v17}, 1), px_call(px_get_global("len"), (LXValue[]){_v18}, 1)))) {
            return px_bool(false);
        }
        LXValue _t36 = px_call(px_get_global("range"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v17}, 1)}, 1);
        for (int _t37 = 0; _t37 < px_len(_t36); _t37++) {
            _v21 = px_index(_t36, px_int(_t37));
            if (px_is_truthy(px_not(px_call(px_get_global("v_eq"), (LXValue[]){px_index(_v17, _v21), px_index(_v18, _v21)}, 2)))) {
                return px_bool(false);
            }
        }
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t38 = px_eq(_v19, px_str("dict")); px_is_truthy(_t38) ? px_eq(_v20, px_str("dict")) : _t38; }))) {
        _v22 = px_method(_v17, "keys", (LXValue[]){}, 0);
        if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v22}, 1), px_call(px_get_global("len"), (LXValue[]){px_method(_v18, "keys", (LXValue[]){}, 0)}, 1)))) {
            return px_bool(false);
        }
        LXValue _t39 = _v22;
        for (int _t40 = 0; _t40 < px_len(_t39); _t40++) {
            _v23 = px_index(_t39, px_int(_t40));
            if (px_is_truthy(px_not(px_method(_v18, "has", (LXValue[]){_v23}, 1)))) {
                return px_bool(false);
            }
            if (px_is_truthy(px_not(px_call(px_get_global("v_eq"), (LXValue[]){px_index(_v17, _v23), px_index(_v18, _v23)}, 2)))) {
                return px_bool(false);
            }
        }
        return px_bool(true);
    }
    return px_eq(_v17, _v18);
px_err_24:
    if (px_err_24_proped) return px_err_24_val;
    return px_null();
}

static LXValue fn_v_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v41 = (nargs > 0) ? args[0] : px_null();
    LXValue _v42 = (nargs > 1) ? args[1] : px_null();
    LXValue _v43 = px_null();
    LXValue _v44 = px_null();
    LXValue _v45 = px_null();
    LXValue _v46 = px_null();
    LXValue px_err_47_val = px_null();
    int px_err_47_proped = 0;
    _v43 = px_call(px_get_global("type"), (LXValue[]){_v41}, 1);
    _v44 = px_call(px_get_global("type"), (LXValue[]){_v42}, 1);
    if (px_is_truthy(({ LXValue _t48 = px_eq(_v43, px_str("int")); px_is_truthy(_t48) ? px_eq(_v44, px_str("int")) : _t48; }))) {
        if (px_is_truthy(px_lt(_v41, _v42))) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(px_gt(_v41, _v42))) {
            return px_int(1LL);
        }
        return px_int(0LL);
    }
    if (px_is_truthy(({ LXValue _t51 = ({ LXValue _t49 = px_eq(_v43, px_str("int")); px_is_truthy(_t49) ? _t49 : px_eq(_v43, px_str("float")); }); px_is_truthy(_t51) ? ({ LXValue _t50 = px_eq(_v44, px_str("int")); px_is_truthy(_t50) ? _t50 : px_eq(_v44, px_str("float")); }) : _t51; }))) {
        _v45 = _v41;
        _v46 = _v42;
        if (px_is_truthy(px_eq(_v43, px_str("int")))) {
             _v45 = px_call(px_get_global("float"), (LXValue[]){_v41}, 1);
        }
        if (px_is_truthy(px_eq(_v44, px_str("int")))) {
             _v46 = px_call(px_get_global("float"), (LXValue[]){_v42}, 1);
        }
        if (px_is_truthy(px_lt(_v45, _v46))) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(px_gt(_v45, _v46))) {
            return px_int(1LL);
        }
        return px_int(0LL);
    }
    if (px_is_truthy(({ LXValue _t52 = px_eq(_v43, px_str("string")); px_is_truthy(_t52) ? px_eq(_v44, px_str("string")) : _t52; }))) {
        if (px_is_truthy(px_lt(_v41, _v42))) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(px_gt(_v41, _v42))) {
            return px_int(1LL);
        }
        return px_int(0LL);
    }
    return px_null();
px_err_47:
    if (px_err_47_proped) return px_err_47_val;
    return px_null();
}

static LXValue fn_v_arith(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v53 = (nargs > 0) ? args[0] : px_null();
    LXValue _v54 = (nargs > 1) ? args[1] : px_null();
    LXValue _v55 = (nargs > 2) ? args[2] : px_null();
    LXValue _v56 = px_null();
    LXValue _v57 = px_null();
    LXValue _v58 = px_null();
    LXValue _v59 = px_null();
    LXValue px_err_60_val = px_null();
    int px_err_60_proped = 0;
    _v56 = px_call(px_get_global("type"), (LXValue[]){_v54}, 1);
    _v57 = px_call(px_get_global("type"), (LXValue[]){_v55}, 1);
    if (px_is_truthy(px_eq(_v53, px_str("+")))) {
        if (px_is_truthy(({ LXValue _t61 = px_eq(_v56, px_str("int")); px_is_truthy(_t61) ? px_eq(_v57, px_str("int")) : _t61; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v54, _v55)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t62 = px_eq(_v56, px_str("int")); px_is_truthy(_t62) ? px_eq(_v57, px_str("float")) : _t62; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("float"), (LXValue[]){_v54}, 1), _v55)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t63 = px_eq(_v56, px_str("float")); px_is_truthy(_t63) ? px_eq(_v57, px_str("int")) : _t63; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v54, px_call(px_get_global("float"), (LXValue[]){_v55}, 1))); } _d; });
        }
        if (px_is_truthy(({ LXValue _t64 = px_eq(_v56, px_str("float")); px_is_truthy(_t64) ? px_eq(_v57, px_str("float")) : _t64; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v54, _v55)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t65 = px_eq(_v56, px_str("string")); px_is_truthy(_t65) ? px_eq(_v57, px_str("string")) : _t65; }))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(_v54, _v55)); } _d; });
        }
        if (px_is_truthy(({ LXValue _t66 = px_eq(_v56, px_str("list")); px_is_truthy(_t66) ? px_eq(_v57, px_str("list")) : _t66; }))) {
            _v58 = px_list_n((LXValue[]){}, 0);
            LXValue _t67 = _v54;
            for (int _t68 = 0; _t68 < px_len(_t67); _t68++) {
                _v59 = px_index(_t67, px_int(_t68));
                (void)(px_method(_v58, "append", (LXValue[]){_v59}, 1));
            }
            LXValue _t69 = _v55;
            for (int _t70 = 0; _t70 < px_len(_t69); _t70++) {
                _v59 = px_index(_t69, px_int(_t70));
                (void)(px_method(_v58, "append", (LXValue[]){_v59}, 1));
            }
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v58); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_str("+ 不支持: "), _v56), px_str(" + ")), _v57)); } _d; });
    }
    if (px_is_truthy(({ LXValue _t71 = px_eq(_v56, px_str("int")); px_is_truthy(_t71) ? px_eq(_v57, px_str("int")) : _t71; }))) {
        return px_call(px_get_global("v_int_op"), (LXValue[]){_v53, _v54, _v55}, 3);
    }
    return px_call(px_get_global("v_float_op"), (LXValue[]){_v53, _v54, _v55}, 3);
px_err_60:
    if (px_err_60_proped) return px_err_60_val;
    return px_null();
}

static LXValue fn_v_int_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v72 = (nargs > 0) ? args[0] : px_null();
    LXValue _v73 = (nargs > 1) ? args[1] : px_null();
    LXValue _v74 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_75_val = px_null();
    int px_err_75_proped = 0;
    if (px_is_truthy(px_eq(_v72, px_str("-")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_sub(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("*")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mul(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("/")))) {
        if (px_is_truthy(px_eq(_v74, px_int(0LL)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_div(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("//")))) {
        if (px_is_truthy(px_eq(_v74, px_int(0LL)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_idiv(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("%")))) {
        if (px_is_truthy(px_eq(_v74, px_int(0LL)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mod(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("**")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_pow(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("<<")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_shl(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str(">>")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_shr(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str(">>>")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_ushr(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("&")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bitand(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("|")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bitor(_v73, _v74)); } _d; });
    }
    if (px_is_truthy(px_eq(_v72, px_str("^")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bitxor(_v73, _v74)); } _d; });
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_str("未知运算符 "), _v72)); } _d; });
px_err_75:
    if (px_err_75_proped) return px_err_75_val;
    return px_null();
}

static LXValue fn_v_float_op(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v76 = (nargs > 0) ? args[0] : px_null();
    LXValue _v77 = (nargs > 1) ? args[1] : px_null();
    LXValue _v78 = (nargs > 2) ? args[2] : px_null();
    LXValue _v79 = px_null();
    LXValue _v80 = px_null();
    LXValue _v81 = px_null();
    LXValue _v82 = px_null();
    LXValue px_err_83_val = px_null();
    int px_err_83_proped = 0;
    _v79 = px_call(px_get_global("type"), (LXValue[]){_v77}, 1);
    _v80 = px_call(px_get_global("type"), (LXValue[]){_v78}, 1);
    if (px_is_truthy(({ LXValue _t84 = px_ne(_v79, px_str("int")); px_is_truthy(_t84) ? px_ne(_v79, px_str("float")) : _t84; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("需要数值")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t85 = px_ne(_v80, px_str("int")); px_is_truthy(_t85) ? px_ne(_v80, px_str("float")) : _t85; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("需要数值")); } _d; });
    }
    _v81 = _v77;
    _v82 = _v78;
    if (px_is_truthy(px_eq(_v79, px_str("int")))) {
         _v81 = px_call(px_get_global("float"), (LXValue[]){_v77}, 1);
    }
    if (px_is_truthy(px_eq(_v80, px_str("int")))) {
         _v82 = px_call(px_get_global("float"), (LXValue[]){_v78}, 1);
    }
    if (px_is_truthy(px_eq(_v76, px_str("-")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_sub(_v81, _v82)); } _d; });
    }
    if (px_is_truthy(px_eq(_v76, px_str("*")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mul(_v81, _v82)); } _d; });
    }
    if (px_is_truthy(px_eq(_v76, px_str("/")))) {
        if (px_is_truthy(px_eq(_v82, px_float(0)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_div(_v81, _v82)); } _d; });
    }
    if (px_is_truthy(px_eq(_v76, px_str("//")))) {
        if (px_is_truthy(px_eq(_v82, px_float(0)))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("除零错误")); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_idiv(_v81, _v82)); } _d; });
    }
    if (px_is_truthy(px_eq(_v76, px_str("%")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_mod(_v81, _v82)); } _d; });
    }
    if (px_is_truthy(px_eq(_v76, px_str("**")))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("v"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_pow(_v81, _v82)); } _d; });
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("此运算符要求整数操作数")); } _d; });
px_err_83:
    if (px_err_83_proped) return px_err_83_val;
    return px_null();
}

static LXValue fn_v_truthy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v86 = (nargs > 0) ? args[0] : px_null();
    LXValue _v87 = px_null();
    LXValue px_err_88_val = px_null();
    int px_err_88_proped = 0;
    _v87 = px_call(px_get_global("type"), (LXValue[]){_v86}, 1);
    if (px_is_truthy(px_eq(_v87, px_str("null")))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_eq(_v87, px_str("bool")))) {
        return _v86;
    }
    if (px_is_truthy(px_eq(_v87, px_str("int")))) {
        return px_ne(_v86, px_int(0LL));
    }
    if (px_is_truthy(px_eq(_v87, px_str("float")))) {
        return px_ne(_v86, px_float(0));
    }
    if (px_is_truthy(px_eq(_v87, px_str("string")))) {
        return px_gt(px_call(px_get_global("len"), (LXValue[]){_v86}, 1), px_int(0LL));
    }
    if (px_is_truthy(px_eq(_v87, px_str("list")))) {
        return px_gt(px_call(px_get_global("len"), (LXValue[]){_v86}, 1), px_int(0LL));
    }
    if (px_is_truthy(px_eq(_v87, px_str("dict")))) {
        return px_gt(px_call(px_get_global("len"), (LXValue[]){_v86}, 1), px_int(0LL));
    }
    return px_bool(true);
px_err_88:
    if (px_err_88_proped) return px_err_88_val;
    return px_null();
}

static LXValue fn_v_int(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v89 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_90_val = px_null();
    int px_err_90_proped = 0;
    return px_call(px_get_global("int"), (LXValue[]){_v89}, 1);
px_err_90:
    if (px_err_90_proped) return px_err_90_val;
    return px_null();
}

static LXValue fn_v_float(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v91 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_92_val = px_null();
    int px_err_92_proped = 0;
    return px_call(px_get_global("float"), (LXValue[]){_v91}, 1);
px_err_92:
    if (px_err_92_proped) return px_err_92_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v93 = (nargs > 0) ? args[0] : px_null();
    LXValue _v94 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_95_val = px_null();
    int px_err_95_proped = 0;
    if (px_is_truthy(_v94)) {
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v93)}, 1));
    }
    else {
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v93)}, 1));
    }
px_err_95:
    if (px_err_95_proped) return px_err_95_val;
    return px_null();
}

static LXValue fn_check_value(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v96 = (nargs > 0) ? args[0] : px_null();
    LXValue _v97 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_98_val = px_null();
    int px_err_98_proped = 0;
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v96, px_str(" v_type")), px_eq(px_call(px_get_global("v_type"), (LXValue[]){_v97}, 1), px_call(px_get_global("type"), (LXValue[]){_v97}, 1))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v96, px_str(" v_str")), px_eq(px_call(px_get_global("v_str"), (LXValue[]){_v97}, 1), px_call(px_get_global("str"), (LXValue[]){_v97}, 1))}, 2));
px_err_98:
    if (px_err_98_proped) return px_err_98_val;
    return px_null();
}

static LXValue fn_check_eq(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v99 = (nargs > 0) ? args[0] : px_null();
    LXValue _v100 = (nargs > 1) ? args[1] : px_null();
    LXValue _v101 = (nargs > 2) ? args[2] : px_null();
    LXValue _v102 = (nargs > 3) ? args[3] : px_null();
    LXValue px_err_103_val = px_null();
    int px_err_103_proped = 0;
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v99, px_str(" v_eq")), px_eq(px_call(px_get_global("v_eq"), (LXValue[]){_v100, _v101}, 2), _v102)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v99, px_str(" 内置")), px_eq(px_eq(_v100, _v101), _v102)}, 2));
px_err_103:
    if (px_err_103_proped) return px_err_103_val;
    return px_null();
}

static LXValue fn_check_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v104 = (nargs > 0) ? args[0] : px_null();
    LXValue _v105 = (nargs > 1) ? args[1] : px_null();
    LXValue _v106 = (nargs > 2) ? args[2] : px_null();
    LXValue _v107 = (nargs > 3) ? args[3] : px_null();
    LXValue _v108 = px_null();
    LXValue px_err_109_val = px_null();
    int px_err_109_proped = 0;
    _v108 = px_call(px_get_global("v_cmp"), (LXValue[]){_v105, _v106}, 2);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v104, px_str(" v_cmp")), px_eq(_v108, _v107)}, 2));
    if (px_is_truthy(px_eq(_v107, px_neg(px_int(1LL))))) {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v104, px_str(" 内置<")), ({ LXValue _t110 = px_eq(px_lt(_v105, _v106), px_bool(true)); px_is_truthy(_t110) ? px_eq(px_gt(_v105, _v106), px_bool(false)) : _t110; })}, 2));
    }
    else if (px_is_truthy(px_eq(_v107, px_int(1LL)))) {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v104, px_str(" 内置>")), ({ LXValue _t111 = px_eq(px_gt(_v105, _v106), px_bool(true)); px_is_truthy(_t111) ? px_eq(px_lt(_v105, _v106), px_bool(false)) : _t111; })}, 2));
    }
    else {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v104, px_str(" 内置==")), ({ LXValue _t112 = px_eq(px_lt(_v105, _v106), px_bool(false)); px_is_truthy(_t112) ? px_eq(px_gt(_v105, _v106), px_bool(false)) : _t112; })}, 2));
    }
px_err_109:
    if (px_err_109_proped) return px_err_109_val;
    return px_null();
}

static LXValue fn_check_arith(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v113 = (nargs > 0) ? args[0] : px_null();
    LXValue _v114 = (nargs > 1) ? args[1] : px_null();
    LXValue _v115 = (nargs > 2) ? args[2] : px_null();
    LXValue _v116 = (nargs > 3) ? args[3] : px_null();
    LXValue _v117 = (nargs > 4) ? args[4] : px_null();
    LXValue _v118 = px_null();
    LXValue px_err_119_val = px_null();
    int px_err_119_proped = 0;
    _v118 = px_call(px_get_global("v_arith"), (LXValue[]){_v114, _v115, _v116}, 3);
    if (px_is_truthy(px_index(_v118, px_str("ok")))) {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" v_arith")), px_eq(px_index(_v118, px_str("v")), _v117)}, 2));
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" str")), px_eq(px_call(px_get_global("str"), (LXValue[]){px_index(_v118, px_str("v"))}, 1), px_call(px_get_global("str"), (LXValue[]){_v117}, 1))}, 2));
    }
    else {
        (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v113, px_str(" v_arith 错误")), px_bool(false)}, 2));
    }
px_err_119:
    if (px_err_119_proped) return px_err_119_val;
    return px_null();
}

static LXValue fn_check_arith_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v120 = (nargs > 0) ? args[0] : px_null();
    LXValue _v121 = (nargs > 1) ? args[1] : px_null();
    LXValue _v122 = (nargs > 2) ? args[2] : px_null();
    LXValue _v123 = (nargs > 3) ? args[3] : px_null();
    LXValue _v124 = px_null();
    LXValue px_err_125_val = px_null();
    int px_err_125_proped = 0;
    _v124 = px_call(px_get_global("v_arith"), (LXValue[]){_v121, _v122, _v123}, 3);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v120, px_str(" 报错")), px_not(px_index(_v124, px_str("ok")))}, 2));
px_err_125:
    if (px_err_125_proped) return px_err_125_val;
    return px_null();
}

static LXValue fn_check_truthy(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v126 = (nargs > 0) ? args[0] : px_null();
    LXValue _v127 = (nargs > 1) ? args[1] : px_null();
    LXValue _v128 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_129_val = px_null();
    int px_err_129_proped = 0;
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v126, px_str(" v_truthy")), px_eq(px_call(px_get_global("v_truthy"), (LXValue[]){_v127}, 1), _v128)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_add(_v126, px_str(" 内置bool")), px_eq(px_call(px_get_global("bool"), (LXValue[]){_v127}, 1), _v128)}, 2));
px_err_129:
    if (px_err_129_proped) return px_err_129_val;
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
    LXValue _t130 = px_call(px_get_global("range"), (LXValue[]){px_int(40LL)}, 1);
    for (int _t131 = 0; _t131 < px_len(_t130); _t131++) {
        LXValue _v132 = px_index(_t130, px_int(_t131));
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
    (void)(px_call(px_get_global("check_arith"), (LXValue[]){px_str("float**"), px_str("**"), px_float(2), px_float(0.5), px_float(1.41421)}, 5));
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
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("除零错误消息"), ({ LXValue _t136 = px_not(px_index(px_get_global("r0"), px_str("ok"))); px_is_truthy(_t136) ? px_eq(px_index(px_get_global("r0"), px_str("err")), px_str("除零错误")) : _t136; })}, 2));
    px_set_global("r1", px_call(px_get_global("v_arith"), (LXValue[]){px_str("+"), px_int(1LL), px_str("a")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("+ 不支持消息"), ({ LXValue _t137 = px_not(px_index(px_get_global("r1"), px_str("ok"))); px_is_truthy(_t137) ? px_eq(px_index(px_get_global("r1"), px_str("err")), px_str("+ 不支持: int + string")) : _t137; })}, 2));
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
