#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_sv_is_digit(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    if (px_is_truthy(({ LXValue _t3 = px_ge(_v1, px_str("0")); px_is_truthy(_t3) ? px_le(_v1, px_str("9")) : _t3; }))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_sv_is_alnum(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v4 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    if (px_is_truthy(px_call(px_get_global("sv_is_digit"), (LXValue[]){_v4}, 1))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t6 = px_ge(_v4, px_str("a")); px_is_truthy(_t6) ? px_le(_v4, px_str("z")) : _t6; }))) {
        return px_bool(true);
    }
    if (px_is_truthy(({ LXValue _t7 = px_ge(_v4, px_str("A")); px_is_truthy(_t7) ? px_le(_v4, px_str("Z")) : _t7; }))) {
        return px_bool(true);
    }
    return px_bool(false);
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_sv_is_all_digits(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = px_null();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v8}, 1), px_int(0LL)))) {
        return px_bool(false);
    }
    LXValue _v11 = px_int(0LL);
    while (px_is_truthy(px_lt(_v11, px_call(px_get_global("len"), (LXValue[]){_v8}, 1)))) {
        if (px_is_truthy(px_not(px_call(px_get_global("sv_is_digit"), (LXValue[]){px_index(_v8, _v11)}, 1)))) {
            return px_bool(false);
        }
         _v11 = px_add(_v11, px_int(1LL));
    }
    return px_bool(true);
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_sv_index_of(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v12 = (nargs > 0) ? args[0] : px_null();
    LXValue _v13 = (nargs > 1) ? args[1] : px_null();
    LXValue _v14 = px_null();
    LXValue px_err_15_val = px_null();
    int px_err_15_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v13}, 1), px_int(0LL)))) {
        return px_int(0LL);
    }
    LXValue _v16 = px_int(0LL);
    while (px_is_truthy(px_le(_v16, px_sub(px_call(px_get_global("len"), (LXValue[]){_v12}, 1), px_call(px_get_global("len"), (LXValue[]){_v13}, 1))))) {
        if (px_is_truthy(px_eq(px_slice(_v12, _v16, px_add(_v16, px_call(px_get_global("len"), (LXValue[]){_v13}, 1)), px_null()), _v13))) {
            return _v16;
        }
         _v16 = px_add(_v16, px_int(1LL));
    }
    return px_neg(px_int(1LL));
px_err_15:
    if (px_err_15_proped) return px_err_15_val;
    return px_null();
}

static LXValue fn_sv_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v17 = (nargs > 0) ? args[0] : px_null();
    LXValue _v18 = px_null();
    LXValue _v19 = px_null();
    LXValue _v20 = px_null();
    LXValue _v21 = px_null();
    LXValue _v22 = px_null();
    LXValue _v23 = px_null();
    LXValue px_err_24_val = px_null();
    int px_err_24_proped = 0;
    LXValue _v25 = _v17;
    LXValue _v26 = px_str("");
    LXValue _v27 = px_call(px_get_global("sv_index_of"), (LXValue[]){_v25, px_str("+")}, 2);
    if (px_is_truthy(px_ge(_v27, px_int(0LL)))) {
         _v26 = px_slice(_v25, px_add(_v27, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v25}, 1), px_null());
         _v25 = px_slice(_v25, px_int(0LL), _v27, px_null());
    }
    LXValue _v28 = px_str("");
    LXValue _v29 = px_call(px_get_global("sv_index_of"), (LXValue[]){_v25, px_str("-")}, 2);
    if (px_is_truthy(px_ge(_v29, px_int(0LL)))) {
         _v28 = px_slice(_v25, px_add(_v29, px_int(1LL)), px_call(px_get_global("len"), (LXValue[]){_v25}, 1), px_null());
         _v25 = px_slice(_v25, px_int(0LL), _v29, px_null());
    }
    LXValue _v30 = px_call(px_get_global("split"), (LXValue[]){_v25, px_str(".")}, 2);
    if (px_is_truthy(px_ne(px_call(px_get_global("len"), (LXValue[]){_v30}, 1), px_int(3LL)))) {
        return px_null();
    }
    if (px_is_truthy(({ LXValue _t32 = ({ LXValue _t31 = px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v30, px_int(0LL))}, 1), px_int(0LL)); px_is_truthy(_t31) ? _t31 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v30, px_int(1LL))}, 1), px_int(0LL)); }); px_is_truthy(_t32) ? _t32 : px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(_v30, px_int(2LL))}, 1), px_int(0LL)); }))) {
        return px_null();
    }
    LXValue _v33 = px_int(0LL);
    while (px_is_truthy(px_lt(_v33, px_int(3LL)))) {
        if (px_is_truthy(px_not(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){px_index(_v30, _v33)}, 1)))) {
            return px_null();
        }
        if (px_is_truthy(({ LXValue _t34 = px_gt(px_call(px_get_global("len"), (LXValue[]){px_index(_v30, _v33)}, 1), px_int(1LL)); px_is_truthy(_t34) ? px_eq(px_index(px_index(_v30, _v33), px_int(0LL)), px_str("0")) : _t34; }))) {
            return px_null();
        }
         _v33 = px_add(_v33, px_int(1LL));
    }
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v28}, 1), px_int(0LL)))) {
        LXValue _v35 = px_call(px_get_global("split"), (LXValue[]){_v28, px_str(".")}, 2);
        LXValue _v36 = px_int(0LL);
        while (px_is_truthy(px_lt(_v36, px_call(px_get_global("len"), (LXValue[]){_v35}, 1)))) {
            LXValue _v37 = px_index(_v35, _v36);
            if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v37}, 1), px_int(0LL)))) {
                return px_null();
            }
            if (px_is_truthy(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v37}, 1))) {
                if (px_is_truthy(({ LXValue _t38 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v37}, 1), px_int(1LL)); px_is_truthy(_t38) ? px_eq(px_index(_v37, px_int(0LL)), px_str("0")) : _t38; }))) {
                    return px_null();
                }
            }
            else {
                LXValue _v39 = px_int(0LL);
                while (px_is_truthy(px_lt(_v39, px_call(px_get_global("len"), (LXValue[]){_v37}, 1)))) {
                    LXValue _v40 = px_index(_v37, _v39);
                    if (px_is_truthy(px_not(({ LXValue _t41 = px_call(px_get_global("sv_is_alnum"), (LXValue[]){_v40}, 1); px_is_truthy(_t41) ? _t41 : px_eq(_v40, px_str("-")); })))) {
                        return px_null();
                    }
                     _v39 = px_add(_v39, px_int(1LL));
                }
            }
             _v36 = px_add(_v36, px_int(1LL));
        }
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("major"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("int"), (LXValue[]){px_index(_v30, px_int(0LL))}, 1)); } { LXValue _k = px_str("minor"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("int"), (LXValue[]){px_index(_v30, px_int(1LL))}, 1)); } { LXValue _k = px_str("patch"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("int"), (LXValue[]){px_index(_v30, px_int(2LL))}, 1)); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v28); } { LXValue _k = px_str("build"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v26); } { LXValue _k = px_str("raw"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v17); } _d; });
px_err_24:
    if (px_err_24_proped) return px_err_24_val;
    return px_null();
}

static LXValue fn_sv_cmp(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v42 = (nargs > 0) ? args[0] : px_null();
    LXValue _v43 = (nargs > 1) ? args[1] : px_null();
    LXValue _v44 = px_null();
    LXValue px_err_45_val = px_null();
    int px_err_45_proped = 0;
    LXValue _v46 = px_call(px_get_global("sv_parse"), (LXValue[]){_v42}, 1);
    LXValue _v47 = px_call(px_get_global("sv_parse"), (LXValue[]){_v43}, 1);
    if (px_is_truthy(({ LXValue _t48 = px_eq(_v46, px_null()); px_is_truthy(_t48) ? _t48 : px_eq(_v47, px_null()); }))) {
        return px_int(0LL);
    }
    if (px_is_truthy(px_lt(px_index(_v46, px_str("major")), px_index(_v47, px_str("major"))))) {
        return px_neg(px_int(1LL));
    }
    if (px_is_truthy(px_gt(px_index(_v46, px_str("major")), px_index(_v47, px_str("major"))))) {
        return px_int(1LL);
    }
    if (px_is_truthy(px_lt(px_index(_v46, px_str("minor")), px_index(_v47, px_str("minor"))))) {
        return px_neg(px_int(1LL));
    }
    if (px_is_truthy(px_gt(px_index(_v46, px_str("minor")), px_index(_v47, px_str("minor"))))) {
        return px_int(1LL);
    }
    if (px_is_truthy(px_lt(px_index(_v46, px_str("patch")), px_index(_v47, px_str("patch"))))) {
        return px_neg(px_int(1LL));
    }
    if (px_is_truthy(px_gt(px_index(_v46, px_str("patch")), px_index(_v47, px_str("patch"))))) {
        return px_int(1LL);
    }
    LXValue _v49 = px_index(_v46, px_str("pre"));
    LXValue _v50 = px_index(_v47, px_str("pre"));
    if (px_is_truthy(({ LXValue _t51 = px_eq(_v49, px_str("")); px_is_truthy(_t51) ? px_eq(_v50, px_str("")) : _t51; }))) {
        return px_int(0LL);
    }
    if (px_is_truthy(px_eq(_v49, px_str("")))) {
        return px_int(1LL);
    }
    if (px_is_truthy(px_eq(_v50, px_str("")))) {
        return px_neg(px_int(1LL));
    }
    LXValue _v52 = px_call(px_get_global("split"), (LXValue[]){_v49, px_str(".")}, 2);
    LXValue _v53 = px_call(px_get_global("split"), (LXValue[]){_v50, px_str(".")}, 2);
    LXValue _v54 = px_int(0LL);
    while (px_is_truthy(({ LXValue _t55 = px_lt(_v54, px_call(px_get_global("len"), (LXValue[]){_v52}, 1)); px_is_truthy(_t55) ? px_lt(_v54, px_call(px_get_global("len"), (LXValue[]){_v53}, 1)) : _t55; }))) {
        LXValue _v56 = px_index(_v52, _v54);
        LXValue _v57 = px_index(_v53, _v54);
        if (px_is_truthy(px_eq(_v56, _v57))) {
             _v54 = px_add(_v54, px_int(1LL));
            continue;
        }
        LXValue _v58 = px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v56}, 1);
        LXValue _v59 = px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v57}, 1);
        if (px_is_truthy(({ LXValue _t60 = _v58; px_is_truthy(_t60) ? _v59 : _t60; }))) {
            if (px_is_truthy(px_lt(px_call(px_get_global("int"), (LXValue[]){_v56}, 1), px_call(px_get_global("int"), (LXValue[]){_v57}, 1)))) {
                return px_neg(px_int(1LL));
            }
            return px_int(1LL);
        }
        if (px_is_truthy(_v58)) {
            return px_neg(px_int(1LL));
        }
        if (px_is_truthy(_v59)) {
            return px_int(1LL);
        }
        if (px_is_truthy(px_lt(_v56, _v57))) {
            return px_neg(px_int(1LL));
        }
        return px_int(1LL);
    }
    if (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v52}, 1), px_call(px_get_global("len"), (LXValue[]){_v53}, 1)))) {
        return px_neg(px_int(1LL));
    }
    if (px_is_truthy(px_gt(px_call(px_get_global("len"), (LXValue[]){_v52}, 1), px_call(px_get_global("len"), (LXValue[]){_v53}, 1)))) {
        return px_int(1LL);
    }
    return px_int(0LL);
px_err_45:
    if (px_err_45_proped) return px_err_45_val;
    return px_null();
}

static LXValue fn_sv_range_parse(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v61 = (nargs > 0) ? args[0] : px_null();
    LXValue _v62 = px_null();
    LXValue _v63 = px_null();
    LXValue _v64 = px_null();
    LXValue _v65 = px_null();
    LXValue _v66 = px_null();
    LXValue px_err_67_val = px_null();
    int px_err_67_proped = 0;
    LXValue _v68 = px_call(px_get_global("trim"), (LXValue[]){_v61}, 1);
    if (px_is_truthy(({ LXValue _t69 = px_eq(_v68, px_str("")); px_is_truthy(_t69) ? _t69 : px_eq(_v68, px_str("*")); }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("0.0.0")); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("999999999.0.0")); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    LXValue _v70 = px_str("");
    if (px_is_truthy(px_eq(px_index(_v68, px_int(0LL)), px_str("^")))) {
         _v70 = px_str("^");
         _v68 = px_call(px_get_global("trim"), (LXValue[]){px_slice(_v68, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v68}, 1), px_null())}, 1);
    }
    else if (px_is_truthy(px_eq(px_index(_v68, px_int(0LL)), px_str("~")))) {
         _v70 = px_str("~");
         _v68 = px_call(px_get_global("trim"), (LXValue[]){px_slice(_v68, px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v68}, 1), px_null())}, 1);
    }
    if (px_is_truthy(({ LXValue _t71 = px_eq(_v68, px_str("")); px_is_truthy(_t71) ? _t71 : px_eq(_v68, px_str("*")); }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("0.0.0")); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("999999999.0.0")); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    LXValue _v72 = px_call(px_get_global("split"), (LXValue[]){_v68, px_str(".")}, 2);
    LXValue _v73 = px_list_n((LXValue[]){}, 0);
    LXValue _v74 = px_int(0LL);
    while (px_is_truthy(({ LXValue _t75 = px_lt(_v74, px_call(px_get_global("len"), (LXValue[]){_v72}, 1)); px_is_truthy(_t75) ? px_lt(_v74, px_int(3LL)) : _t75; }))) {
        LXValue _v76 = px_index(_v72, _v74);
        if (px_is_truthy(({ LXValue _t78 = ({ LXValue _t77 = px_eq(_v76, px_str("x")); px_is_truthy(_t77) ? _t77 : px_eq(_v76, px_str("X")); }); px_is_truthy(_t78) ? _t78 : px_eq(_v76, px_str("*")); }))) {
            (void)(px_method(_v73, "append", (LXValue[]){px_neg(px_int(1LL))}, 1));
        }
        else if (px_is_truthy(px_call(px_get_global("sv_is_all_digits"), (LXValue[]){_v76}, 1))) {
            if (px_is_truthy(({ LXValue _t79 = px_gt(px_call(px_get_global("len"), (LXValue[]){_v76}, 1), px_int(1LL)); px_is_truthy(_t79) ? px_eq(px_index(_v76, px_int(0LL)), px_str("0")) : _t79; }))) {
                return px_null();
            }
            (void)(px_method(_v73, "append", (LXValue[]){px_call(px_get_global("int"), (LXValue[]){_v76}, 1)}, 1));
        }
        else {
            return px_null();
        }
         _v74 = px_add(_v74, px_int(1LL));
    }
    while (px_is_truthy(px_lt(px_call(px_get_global("len"), (LXValue[]){_v73}, 1), px_int(3LL)))) {
        (void)(px_method(_v73, "append", (LXValue[]){px_neg(px_int(1LL))}, 1));
    }
    LXValue _v80 = px_index(_v73, px_int(0LL));
    LXValue _v81 = px_index(_v73, px_int(1LL));
    LXValue _v82 = px_index(_v73, px_int(2LL));
    if (px_is_truthy(px_lt(_v80, px_int(0LL)))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("0.0.0")); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("999999999.0.0")); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t84 = ({ LXValue _t83 = px_eq(_v70, px_str("")); px_is_truthy(_t83) ? px_ge(_v81, px_int(0LL)) : _t83; }); px_is_truthy(_t84) ? px_ge(_v82, px_int(0LL)) : _t84; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v68); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v81}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v82, px_int(1LL))}, 1))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t85 = px_eq(_v70, px_str("")); px_is_truthy(_t85) ? px_lt(_v81, px_int(0LL)) : _t85; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".0.0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v80, px_int(1LL))}, 1), px_str(".0.0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t86 = px_eq(_v70, px_str("")); px_is_truthy(_t86) ? px_lt(_v82, px_int(0LL)) : _t86; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v81}, 1)), px_str(".0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v81, px_int(1LL))}, 1)), px_str(".0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    if (px_is_truthy(px_eq(_v70, px_str("^")))) {
        LXValue _v87 = px_str("");
        LXValue _v88 = px_str("");
        if (px_is_truthy(px_gt(_v80, px_int(0LL)))) {
             _v87 = px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v81}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v82}, 1));
             _v88 = px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v80, px_int(1LL))}, 1), px_str(".0.0"));
        }
        else if (px_is_truthy(px_gt(_v81, px_int(0LL)))) {
             _v87 = px_add(px_add(px_add(px_str("0."), px_call(px_get_global("str"), (LXValue[]){_v81}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v82}, 1));
             _v88 = px_add(px_add(px_str("0."), px_call(px_get_global("str"), (LXValue[]){px_add(_v81, px_int(1LL))}, 1)), px_str(".0"));
        }
        else {
             _v87 = px_add(px_str("0.0."), px_call(px_get_global("str"), (LXValue[]){_v82}, 1));
             _v88 = px_add(px_str("0.0."), px_call(px_get_global("str"), (LXValue[]){px_add(_v82, px_int(1LL))}, 1));
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v87); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v88); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t89 = px_ge(_v80, px_int(0LL)); px_is_truthy(_t89) ? px_lt(_v81, px_int(0LL)) : _t89; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".0.0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_call(px_get_global("str"), (LXValue[]){px_add(_v80, px_int(1LL))}, 1), px_str(".0.0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    if (px_is_truthy(({ LXValue _t91 = ({ LXValue _t90 = px_ge(_v80, px_int(0LL)); px_is_truthy(_t90) ? px_ge(_v81, px_int(0LL)) : _t90; }); px_is_truthy(_t91) ? px_lt(_v82, px_int(0LL)) : _t91; }))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v81}, 1)), px_str(".0"))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v81, px_int(1LL))}, 1)), px_str(".0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("lo"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v81}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){_v82}, 1))); } { LXValue _k = px_str("hi"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_add(px_call(px_get_global("str"), (LXValue[]){_v80}, 1), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_add(_v81, px_int(1LL))}, 1)), px_str(".0"))); } { LXValue _k = px_str("pre"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
px_err_67:
    if (px_err_67_proped) return px_err_67_val;
    return px_null();
}

static LXValue fn_sv_satisfies(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v92 = (nargs > 0) ? args[0] : px_null();
    LXValue _v93 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_94_val = px_null();
    int px_err_94_proped = 0;
    LXValue _v95 = px_call(px_get_global("sv_range_parse"), (LXValue[]){_v93}, 1);
    if (px_is_truthy(px_eq(_v95, px_null()))) {
        return px_bool(false);
    }
    LXValue _v96 = px_call(px_get_global("sv_parse"), (LXValue[]){_v92}, 1);
    if (px_is_truthy(px_eq(_v96, px_null()))) {
        return px_bool(false);
    }
    if (px_is_truthy(px_ne(px_index(_v96, px_str("pre")), px_str("")))) {
        return px_bool(false);
    }
    return ({ LXValue _t97 = px_ge(px_call(px_get_global("sv_cmp"), (LXValue[]){_v92, px_index(_v95, px_str("lo"))}, 2), px_int(0LL)); px_is_truthy(_t97) ? px_lt(px_call(px_get_global("sv_cmp"), (LXValue[]){_v92, px_index(_v95, px_str("hi"))}, 2), px_int(0LL)) : _t97; });
px_err_94:
    if (px_err_94_proped) return px_err_94_val;
    return px_null();
}

static LXValue fn_sv_best(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v98 = (nargs > 0) ? args[0] : px_null();
    LXValue _v99 = (nargs > 1) ? args[1] : px_null();
    LXValue _v100 = px_null();
    LXValue _v101 = px_null();
    LXValue px_err_102_val = px_null();
    int px_err_102_proped = 0;
    LXValue _v103 = px_null();
    LXValue _v104 = px_int(0LL);
    while (px_is_truthy(px_lt(_v104, px_call(px_get_global("len"), (LXValue[]){_v98}, 1)))) {
        LXValue _v105 = px_index(_v98, _v104);
        if (px_is_truthy(px_call(px_get_global("sv_satisfies"), (LXValue[]){_v105, _v99}, 2))) {
            if (px_is_truthy(({ LXValue _t106 = px_eq(_v103, px_null()); px_is_truthy(_t106) ? _t106 : px_gt(px_call(px_get_global("sv_cmp"), (LXValue[]){_v105, _v103}, 2), px_int(0LL)); }))) {
                 _v103 = _v105;
            }
        }
         _v104 = px_add(_v104, px_int(1LL));
    }
    return _v103;
px_err_102:
    if (px_err_102_proped) return px_err_102_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_107_val = px_null();
    int px_err_107_proped = 0;
    LXValue _v108 = px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2.3-alpha.1+build5")}, 1);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_str("major: "), px_call(px_get_global("str"), (LXValue[]){px_index(_v108, px_str("major"))}, 1)), px_str(" minor: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v108, px_str("minor"))}, 1)), px_str(" patch: ")), px_call(px_get_global("str"), (LXValue[]){px_index(_v108, px_str("patch"))}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("pre: "), px_index(_v108, px_str("pre"))), px_str(" build: ")), px_index(_v108, px_str("build")))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_add(px_str("bad: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2")}, 1), px_null())}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("01.2.3")}, 1), px_null())}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_parse"), (LXValue[]){px_str("1.2.3-01")}, 1), px_null())}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1.2.3 vs 1.2.10: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.2.3"), px_str("1.2.10")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1.0.0-alpha vs 1.0.0: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-alpha"), px_str("1.0.0")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp alpha.2 vs alpha.10: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-alpha.2"), px_str("1.0.0-alpha.10")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp 1 vs alpha: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_cmp"), (LXValue[]){px_str("1.0.0-1"), px_str("1.0.0-alpha")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("^1.2.0: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.5"), px_str("^1.2.0")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("2.0.0"), px_str("^1.2.0")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("^0.2.3: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("0.2.9"), px_str("^0.2.3")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("0.3.0"), px_str("^0.2.3")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("~1.2.3: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.9"), px_str("~1.2.3")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.3.0"), px_str("~1.2.3")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("1.2.x: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.9"), px_str("1.2.x")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.3.0"), px_str("1.2.x")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("exact: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.3"), px_str("1.2.3")}, 2)}, 1)), px_str(" ")), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.4"), px_str("1.2.3")}, 2)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("pre reject: "), px_call(px_get_global("str"), (LXValue[]){px_call(px_get_global("sv_satisfies"), (LXValue[]){px_str("1.2.3-alpha"), px_str("1.2.3")}, 2)}, 1))}, 1));
    LXValue _v109 = px_list_n((LXValue[]){px_str("1.2.0"), px_str("1.2.5"), px_str("2.0.0"), px_str("1.0.0")}, 4);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^1.2.0: "), px_call(px_get_global("sv_best"), (LXValue[]){_v109, px_str("^1.2.0")}, 2))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^2.0.0: "), px_call(px_get_global("sv_best"), (LXValue[]){_v109, px_str("^2.0.0")}, 2))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ^0.5.0: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_call(px_get_global("sv_best"), (LXValue[]){_v109, px_str("^0.5.0")}, 2), px_null())}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("best ~1.2: "), px_call(px_get_global("sv_best"), (LXValue[]){px_list_n((LXValue[]){px_str("1.2.0"), px_str("1.2.9"), px_str("1.3.0")}, 3), px_str("~1.2")}, 2))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("semver ok")}, 1));
px_err_107:
    if (px_err_107_proped) return px_err_107_val;
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
