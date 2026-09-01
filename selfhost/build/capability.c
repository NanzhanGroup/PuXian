/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    return ({ LXValue _blk = px_null(); return px_int(42LL);
_blk; });
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v141 = (nargs > 0) ? args[0] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_mul(_v141, px_int(2LL)); _blk; });
}
static LXValue fn_closure_3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v142 = (nargs > 0) ? args[0] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_gt(_v142, px_int(2LL)); _blk; });
}
static LXValue fn_closure_4(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v143 = (nargs > 0) ? args[0] : px_null();
    LXValue _v144 = (nargs > 1) ? args[1] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_add(_v143, _v144); _blk; });
}
static LXValue fn_closure_5(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v168 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_169_val = px_null();
    int px_err_169_proped = 0;
    return px_mul(_v168, px_int(2LL));
px_err_169:
    if (px_err_169_proped) return px_err_169_val;
    return px_null();
}
static LXValue fn_closure_6(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v171 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_172_val = px_null();
    int px_err_172_proped = 0;
    return _v171;
px_err_172:
    if (px_err_172_proped) return px_err_172_val;
    return px_null();
}

static LXValue fn_Point_area(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    return px_mul(px_field(_v1, "x"), px_field(_v1, "y"));
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_each(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v3 = (nargs > 0) ? args[0] : px_null();
    LXValue _v4 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    LXValue _t6 = _v3;
    for (int _t7 = 0; _t7 < px_len(_t6); _t7++) {
        LXValue _v8 = px_index(_t6, px_int(_t7));
        (void)(px_call(_v4, (LXValue[]){_v8}, 1));
    }
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_unique(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v9 = (nargs > 0) ? args[0] : px_null();
    LXValue _v10 = px_null();
    LXValue px_err_11_val = px_null();
    int px_err_11_proped = 0;
     _v10 = px_list_n((LXValue[]){}, 0);
    LXValue _t12 = _v9;
    for (int _t13 = 0; _t13 < px_len(_t12); _t13++) {
        LXValue _v14 = px_index(_t12, px_int(_t13));
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v10, _v14}, 2)))) {
            (void)(px_method(_v10, "append", (LXValue[]){_v14}, 1));
        }
    }
    return _v10;
px_err_11:
    if (px_err_11_proped) return px_err_11_val;
    return px_null();
}

static LXValue fn_flatten(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v15 = (nargs > 0) ? args[0] : px_null();
    LXValue _v16 = px_null();
    LXValue px_err_17_val = px_null();
    int px_err_17_proped = 0;
     _v16 = px_list_n((LXValue[]){}, 0);
    LXValue _t18 = _v15;
    for (int _t19 = 0; _t19 < px_len(_t18); _t19++) {
        LXValue _v20 = px_index(_t18, px_int(_t19));
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v20}, 1), px_str("list")))) {
            LXValue _t21 = _v20;
            for (int _t22 = 0; _t22 < px_len(_t21); _t22++) {
                LXValue _v23 = px_index(_t21, px_int(_t22));
                (void)(px_method(_v16, "append", (LXValue[]){_v23}, 1));
            }
        }
        else {
            (void)(px_method(_v16, "append", (LXValue[]){_v20}, 1));
        }
    }
    return _v16;
px_err_17:
    if (px_err_17_proped) return px_err_17_val;
    return px_null();
}

static LXValue fn_zip_lists(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v24 = (nargs > 0) ? args[0] : px_null();
    LXValue _v25 = (nargs > 1) ? args[1] : px_null();
    LXValue _v26 = px_null();
    LXValue _v27 = px_null();
    LXValue px_err_28_val = px_null();
    int px_err_28_proped = 0;
     _v26 = px_list_n((LXValue[]){}, 0);
     _v27 = px_call(px_get_global("min"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v24}, 1), px_call(px_get_global("len"), (LXValue[]){_v25}, 1)}, 2);
    LXValue _t29 = px_call(px_get_global("range"), (LXValue[]){_v27}, 1);
    for (int _t30 = 0; _t30 < px_len(_t29); _t30++) {
        LXValue _v31 = px_index(_t29, px_int(_t30));
        (void)(px_method(_v26, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v24, _v31), px_index(_v25, _v31)}, 2)}, 1));
    }
    return _v26;
px_err_28:
    if (px_err_28_proped) return px_err_28_val;
    return px_null();
}

static LXValue fn_chunk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v32 = (nargs > 0) ? args[0] : px_null();
    LXValue _v33 = (nargs > 1) ? args[1] : px_null();
    LXValue _v34 = px_null();
    LXValue _v35 = px_null();
    LXValue _v36 = px_null();
    LXValue _v37 = px_null();
    LXValue px_err_38_val = px_null();
    int px_err_38_proped = 0;
     _v34 = px_list_n((LXValue[]){}, 0);
     _v35 = px_int(0LL);
    while (px_is_truthy(px_lt(_v35, px_call(px_get_global("len"), (LXValue[]){_v32}, 1)))) {
         _v36 = px_list_n((LXValue[]){}, 0);
         _v37 = px_int(0LL);
        while (px_is_truthy(({ LXValue _t39 = px_lt(_v37, _v33); px_is_truthy(_t39) ? px_lt(_v35, px_call(px_get_global("len"), (LXValue[]){_v32}, 1)) : _t39; }))) {
            (void)(px_method(_v36, "append", (LXValue[]){px_index(_v32, _v35)}, 1));
             _v35 = px_add(_v35, px_int(1LL));
             _v37 = px_add(_v37, px_int(1LL));
        }
        (void)(px_method(_v34, "append", (LXValue[]){_v36}, 1));
    }
    return _v34;
px_err_38:
    if (px_err_38_proped) return px_err_38_val;
    return px_null();
}

static LXValue fn_group_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v40 = (nargs > 0) ? args[0] : px_null();
    LXValue _v41 = (nargs > 1) ? args[1] : px_null();
    LXValue _v42 = px_null();
    LXValue px_err_43_val = px_null();
    int px_err_43_proped = 0;
     _v42 = ({ LXValue _blk = px_null(); _blk; });
    LXValue _t44 = _v40;
    for (int _t45 = 0; _t45 < px_len(_t44); _t45++) {
        LXValue _v46 = px_index(_t44, px_int(_t45));
        px_set_global("k", px_call(px_get_global("str"), (LXValue[]){px_call(_v41, (LXValue[]){_v46}, 1)}, 1));
        if (px_is_truthy(px_method(_v42, "has", (LXValue[]){px_get_global("k")}, 1))) {
            (void)(px_method(px_index(_v42, px_get_global("k")), "append", (LXValue[]){_v46}, 1));
        }
        else {
            px_index_set(_v42, px_get_global("k"), px_list_n((LXValue[]){_v46}, 1));
        }
    }
    return _v42;
px_err_43:
    if (px_err_43_proped) return px_err_43_val;
    return px_null();
}

static LXValue fn_sort_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v47 = (nargs > 0) ? args[0] : px_null();
    LXValue _v48 = (nargs > 1) ? args[1] : px_null();
    LXValue _v49 = px_null();
    LXValue _v50 = px_null();
    LXValue px_err_51_val = px_null();
    int px_err_51_proped = 0;
     _v49 = px_list_n((LXValue[]){}, 0);
    LXValue _t52 = _v47;
    for (int _t53 = 0; _t53 < px_len(_t52); _t53++) {
        LXValue _v54 = px_index(_t52, px_int(_t53));
        (void)(px_method(_v49, "append", (LXValue[]){px_list_n((LXValue[]){px_call(_v48, (LXValue[]){_v54}, 1), _v54}, 2)}, 1));
    }
     _v49 = px_call(px_get_global("sorted"), (LXValue[]){_v49}, 1);
     _v50 = px_list_n((LXValue[]){}, 0);
    LXValue _t55 = _v49;
    for (int _t56 = 0; _t56 < px_len(_t55); _t56++) {
        LXValue _v57 = px_index(_t55, px_int(_t56));
        (void)(px_method(_v50, "append", (LXValue[]){px_index(_v57, px_int(1LL))}, 1));
    }
    return _v50;
px_err_51:
    if (px_err_51_proped) return px_err_51_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v58 = (nargs > 0) ? args[0] : px_null();
    LXValue _v59 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_60_val = px_null();
    int px_err_60_proped = 0;
    if (px_is_truthy(_v59)) {
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v58)}, 1));
    }
    else {
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        (void)(px_method(px_get_global("g_fails"), "append", (LXValue[]){_v58}, 1));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v58)}, 1));
    }
px_err_60:
    if (px_err_60_proped) return px_err_60_val;
    return px_null();
}

static LXValue fn_section(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v61 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_62_val = px_null();
    int px_err_62_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("== "), _v61), px_str(" =="))}, 1));
px_err_62:
    if (px_err_62_proped) return px_err_62_val;
    return px_null();
}

static LXValue fn_repeat_str(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v63 = (nargs > 0) ? args[0] : px_null();
    LXValue _v64 = (nargs > 1) ? args[1] : px_null();
    LXValue _v65 = px_null();
    LXValue px_err_66_val = px_null();
    int px_err_66_proped = 0;
    LXValue _v67 = px_str("");
    LXValue _t68 = px_call(px_get_global("range"), (LXValue[]){_v64}, 1);
    for (int _t69 = 0; _t69 < px_len(_t68); _t69++) {
        LXValue _v70 = px_index(_t68, px_int(_t69));
         _v67 = px_add(_v67, _v63);
    }
    return _v67;
px_err_66:
    if (px_err_66_proped) return px_err_66_val;
    return px_null();
}

static LXValue fn_try_div(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v71 = (nargs > 0) ? args[0] : px_null();
    LXValue _v72 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_73_val = px_null();
    int px_err_73_proped = 0;
    if (px_is_truthy(px_eq(_v72, px_int(0LL)))) {
        return px_call(px_get_global("Err"), (LXValue[]){px_str("div0")}, 1);
    }
    return px_call(px_get_global("Ok"), (LXValue[]){px_idiv(_v71, _v72)}, 1);
px_err_73:
    if (px_err_73_proped) return px_err_73_val;
    return px_null();
}

static LXValue fn_chain(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v74 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_75_val = px_null();
    int px_err_75_proped = 0;
    LXValue _v77 = ({ LXValue _t76 = px_call(px_get_global("try_div"), (LXValue[]){_v74, px_int(2LL)}, 2); if (px_is_result(_t76)) { if (!px_result_ok(_t76)) { px_err_75_val = _t76; px_err_75_proped = 1; goto px_err_75; } _t76 = px_result_unwrap(_t76); } else if (px_is_null(_t76)) { px_err_75_val = px_null(); px_err_75_proped = 1; goto px_err_75; } _t76; });
    LXValue _v79 = ({ LXValue _t78 = px_call(px_get_global("try_div"), (LXValue[]){_v77, px_int(3LL)}, 2); if (px_is_result(_t78)) { if (!px_result_ok(_t78)) { px_err_75_val = _t78; px_err_75_proped = 1; goto px_err_75; } _t78 = px_result_unwrap(_t78); } else if (px_is_null(_t78)) { px_err_75_val = px_null(); px_err_75_proped = 1; goto px_err_75; } _t78; });
    return px_call(px_get_global("Ok"), (LXValue[]){px_add(_v79, px_int(1LL))}, 1);
px_err_75:
    if (px_err_75_proped) return px_err_75_val;
    return px_null();
}

static LXValue fn_chain_err(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_80_val = px_null();
    int px_err_80_proped = 0;
    LXValue _v82 = ({ LXValue _t81 = px_call(px_get_global("try_div"), (LXValue[]){px_int(10LL), px_int(0LL)}, 2); if (px_is_result(_t81)) { if (!px_result_ok(_t81)) { px_err_80_val = _t81; px_err_80_proped = 1; goto px_err_80; } _t81 = px_result_unwrap(_t81); } else if (px_is_null(_t81)) { px_err_80_val = px_null(); px_err_80_proped = 1; goto px_err_80; } _t81; });
    return px_call(px_get_global("Ok"), (LXValue[]){_v82}, 1);
px_err_80:
    if (px_err_80_proped) return px_err_80_val;
    return px_null();
}

static LXValue fn_kind_name(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v83 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_84_val = px_null();
    int px_err_84_proped = 0;
    LXValue _v86 = ({ LXValue _t85 = _v83; if ((_t85.type == PX_ENUM && strcmp(_t85.as.obj->as.enum_inst.variant, "Int") == 0)) { _t85 = ({ LXValue _blk = px_null(); _blk = px_str("int"); _blk; }); } else if ((_t85.type == PX_ENUM && strcmp(_t85.as.obj->as.enum_inst.variant, "Ident") == 0)) { _t85 = ({ LXValue _blk = px_null(); _blk = px_str("ident"); _blk; }); } else if ((_t85.type == PX_ENUM && strcmp(_t85.as.obj->as.enum_inst.variant, "Str") == 0)) { _t85 = ({ LXValue _blk = px_null(); _blk = px_str("str"); _blk; }); } else if ((_t85.type == PX_ENUM && strcmp(_t85.as.obj->as.enum_inst.variant, "Kw") == 0)) { _t85 = ({ LXValue _blk = px_null(); _blk = px_str("kw"); _blk; }); } else if ((_t85.type == PX_ENUM && strcmp(_t85.as.obj->as.enum_inst.variant, "Eof") == 0)) { _t85 = ({ LXValue _blk = px_null(); _blk = px_str("eof"); _blk; }); } _t85; });
    return _v86;
px_err_84:
    if (px_err_84_proped) return px_err_84_val;
    return px_null();
}

static LXValue fn_node_desc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v87 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_88_val = px_null();
    int px_err_88_proped = 0;
    LXValue _v90 = ({ LXValue _t89 = px_field(_v87, "kind"); if ((_t89.type == PX_ENUM && strcmp(_t89.as.obj->as.enum_inst.variant, "Number") == 0)) { _t89 = ({ LXValue _blk = px_null(); _blk = px_add(px_str("num:"), px_call(px_get_global("str"), (LXValue[]){px_field(_v87, "value")}, 1)); _blk; }); } else if ((_t89.type == PX_ENUM && strcmp(_t89.as.obj->as.enum_inst.variant, "StrLit") == 0)) { _t89 = ({ LXValue _blk = px_null(); _blk = px_str("str"); _blk; }); } else if ((_t89.type == PX_ENUM && strcmp(_t89.as.obj->as.enum_inst.variant, "Call") == 0)) { _t89 = ({ LXValue _blk = px_null(); _blk = px_add(px_str("call:"), px_field(_v87, "name")); _blk; }); } else if ((_t89.type == PX_ENUM && strcmp(_t89.as.obj->as.enum_inst.variant, "BinOp") == 0)) { _t89 = ({ LXValue _blk = px_null(); _blk = px_str("bin"); _blk; }); } _t89; });
    return _v90;
px_err_88:
    if (px_err_88_proped) return px_err_88_val;
    return px_null();
}

static LXValue fn_fact(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v91 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_92_val = px_null();
    int px_err_92_proped = 0;
    if (px_is_truthy(px_le(_v91, px_int(1LL)))) {
        return px_int(1LL);
    }
    return px_mul(_v91, px_call(px_get_global("fact"), (LXValue[]){px_sub(_v91, px_int(1LL))}, 1));
px_err_92:
    if (px_err_92_proped) return px_err_92_val;
    return px_null();
}

static LXValue fn_make_outer(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_93_val = px_null();
    int px_err_93_proped = 0;
    LXValue _v94 = px_func("<closure1>", fn_closure_1, NULL);
    return _v94;
px_err_93:
    if (px_err_93_proped) return px_err_93_val;
    return px_null();
}

static LXValue fn_greet(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v95 = (nargs > 0) ? args[0] : px_null();
    LXValue _v96 = (nargs > 1) ? args[1] : px_str("!");
    LXValue px_err_97_val = px_null();
    int px_err_97_proped = 0;
    return px_add(px_add(px_str("hi "), _v95), _v96);
px_err_97:
    if (px_err_97_proped) return px_err_97_val;
    return px_null();
}

static LXValue fn_sum_all(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v98 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_99_val = px_null();
    int px_err_99_proped = 0;
    LXValue _v100 = px_int(0LL);
    LXValue _t101 = _v98;
    for (int _t102 = 0; _t102 < px_len(_t101); _t102++) {
        LXValue _v103 = px_index(_t101, px_int(_t102));
         _v100 = px_add(_v100, _v103);
    }
    return _v100;
px_err_99:
    if (px_err_99_proped) return px_err_99_val;
    return px_null();
}

static LXValue fn_cfg(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v104 = (nargs > 0) ? args[0] : px_null();
    LXValue _v105 = (nargs > 1) ? args[1] : px_int(8080LL);
    LXValue _v106 = (nargs > 2) ? args[2] : px_bool(false);
    LXValue px_err_107_val = px_null();
    int px_err_107_proped = 0;
    if (px_is_truthy(_v106)) {
        return px_add(px_add(px_add(_v104, px_str(":")), px_call(px_get_global("str"), (LXValue[]){_v105}, 1)), px_str("s"));
    }
    return px_add(px_add(_v104, px_str(":")), px_call(px_get_global("str"), (LXValue[]){_v105}, 1));
px_err_107:
    if (px_err_107_proped) return px_err_107_val;
    return px_null();
}

static LXValue fn_typed(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v108 = (nargs > 0) ? args[0] : px_null();
    LXValue _v109 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_110_val = px_null();
    int px_err_110_proped = 0;
    return px_add(_v109, px_call(px_get_global("str"), (LXValue[]){_v108}, 1));
px_err_110:
    if (px_err_110_proped) return px_err_110_val;
    return px_null();
}

int main(void) {
    px_register_builtins();
    px_set_global("each", px_func("each", fn_each, NULL));
    px_set_global("unique", px_func("unique", fn_unique, NULL));
    px_set_global("flatten", px_func("flatten", fn_flatten, NULL));
    px_set_global("zip_lists", px_func("zip_lists", fn_zip_lists, NULL));
    px_set_global("chunk", px_func("chunk", fn_chunk, NULL));
    px_set_global("group_by", px_func("group_by", fn_group_by, NULL));
    px_set_global("sort_by", px_func("sort_by", fn_sort_by, NULL));
    px_set_global("check", px_func("check", fn_check, NULL));
    px_set_global("section", px_func("section", fn_section, NULL));
    px_set_global("repeat_str", px_func("repeat_str", fn_repeat_str, NULL));
    px_set_global("try_div", px_func("try_div", fn_try_div, NULL));
    px_set_global("chain", px_func("chain", fn_chain, NULL));
    px_set_global("chain_err", px_func("chain_err", fn_chain_err, NULL));
    px_set_global("kind_name", px_func("kind_name", fn_kind_name, NULL));
    px_set_global("node_desc", px_func("node_desc", fn_node_desc, NULL));
    px_set_global("fact", px_func("fact", fn_fact, NULL));
    px_set_global("make_outer", px_func("make_outer", fn_make_outer, NULL));
    px_set_global("greet", px_func("greet", fn_greet, NULL));
    px_set_global("sum_all", px_func("sum_all", fn_sum_all, NULL));
    px_set_global("cfg", px_func("cfg", fn_cfg, NULL));
    px_set_global("typed", px_func("typed", fn_typed, NULL));
    px_set_global("Point.area", px_func("Point.area", fn_Point_area, NULL));
    px_set_global("g_pass", px_int(0LL));
    px_set_global("g_fail", px_int(0LL));
    px_set_global("g_fails", px_list_n((LXValue[]){}, 0));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("1. 字符串处理")}, 1));
    px_set_global("s", px_str("  hello,world,px  "));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("trim 去除首尾空格"), px_eq(px_call(px_get_global("trim"), (LXValue[]){px_get_global("s")}, 1), px_str("hello,world,px"))}, 2));
    px_set_global("parts", px_call(px_get_global("split"), (LXValue[]){px_str("a,b,c"), px_str(",")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("split 分割"), ({ LXValue _t112 = ({ LXValue _t111 = px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("parts")}, 1), px_int(3LL)); px_is_truthy(_t111) ? px_eq(px_index(px_get_global("parts"), px_int(0LL)), px_str("a")) : _t111; }); px_is_truthy(_t112) ? px_eq(px_index(px_get_global("parts"), px_int(2LL)), px_str("c")) : _t112; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("join 拼接"), px_eq(px_call(px_get_global("join"), (LXValue[]){px_str("-"), px_list_n((LXValue[]){px_str("x"), px_str("y"), px_str("z")}, 3)}, 2), px_str("x-y-z"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("replace 替换"), px_eq(px_call(px_get_global("replace"), (LXValue[]){px_str("hello world"), px_str("world"), px_str("px")}, 3), px_str("hello px"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("to_upper"), px_eq(px_call(px_get_global("to_upper"), (LXValue[]){px_str("abc")}, 1), px_str("ABC"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("to_lower"), px_eq(px_call(px_get_global("to_lower"), (LXValue[]){px_str("XYZ")}, 1), px_str("xyz"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("startswith"), px_call(px_get_global("starts_with"), (LXValue[]){px_str("puxian"), px_str("pu")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("endswith"), px_call(px_get_global("ends_with"), (LXValue[]){px_str("puxian"), px_str("ian")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("contains 子串"), px_call(px_get_global("contains"), (LXValue[]){px_str("puxian"), px_str("xi")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("len 字符串长度"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_str("普贤px")}, 1), px_int(4LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("str 数值转字符串"), ({ LXValue _t113 = px_eq(px_call(px_get_global("str"), (LXValue[]){px_int(42LL)}, 1), px_str("42")); px_is_truthy(_t113) ? px_eq(px_call(px_get_global("str"), (LXValue[]){px_float(3.5)}, 1), px_str("3.5")) : _t113; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("int 字符串转整数"), px_eq(px_call(px_get_global("int"), (LXValue[]){px_str("42")}, 1), px_int(42LL))}, 2));
    px_set_global("t", px_str("abcdef"));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("字符串索引"), ({ LXValue _t114 = px_eq(px_index(px_get_global("t"), px_int(0LL)), px_str("a")); px_is_truthy(_t114) ? px_eq(px_index(px_get_global("t"), px_int(5LL)), px_str("f")) : _t114; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("字符串切片"), px_eq(px_slice(px_get_global("t"), px_int(1LL), px_int(4LL), px_null()), px_str("bcd"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("字符串负数索引"), px_eq(px_index(px_get_global("t"), px_neg(px_int(1LL))), px_str("f"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("字符串 + 拼接"), px_eq(px_add(px_add(px_str("px"), px_str("-")), px_str("lang")), px_str("px-lang"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("字符串重复(循环)"), px_eq(px_call(px_get_global("repeat_str"), (LXValue[]){px_str("ab"), px_int(3LL)}, 2), px_str("ababab"))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("2. 正则表达式（自研引擎）")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("regex_match 匹配"), px_call(px_get_global("regex_match"), (LXValue[]){px_str("^[a-z]+$"), px_str("puxian")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("regex_match 不匹配"), px_not(px_call(px_get_global("regex_match"), (LXValue[]){px_str("^[0-9]+$"), px_str("puxian")}, 2))}, 2));
    px_set_global("m", px_call(px_get_global("regex_search"), (LXValue[]){px_str("(\\w+)@(\\w+)"), px_str("contact a@b.com now")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("regex_search 分组"), ({ LXValue _t117 = ({ LXValue _t116 = ({ LXValue _t115 = px_ne(px_get_global("m"), px_null()); px_is_truthy(_t115) ? px_eq(px_index(px_get_global("m"), px_str("match")), px_str("a@b")) : _t115; }); px_is_truthy(_t116) ? px_eq(px_index(px_index(px_get_global("m"), px_str("groups")), px_int(0LL)), px_str("a")) : _t116; }); px_is_truthy(_t117) ? px_eq(px_index(px_index(px_get_global("m"), px_str("groups")), px_int(1LL)), px_str("b")) : _t117; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("regex_replace 替换"), px_eq(px_call(px_get_global("regex_replace"), (LXValue[]){px_str("\\d+"), px_str("id-123"), px_str("#")}, 3), px_str("id-#"))}, 2));
    px_set_global("rs", px_call(px_get_global("regex_split"), (LXValue[]){px_str("[,\\s]+"), px_str("a, b  c")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("regex_split 分割"), ({ LXValue _t118 = px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("rs")}, 1), px_int(3LL)); px_is_truthy(_t118) ? px_eq(px_index(px_get_global("rs"), px_int(2LL)), px_str("c")) : _t118; })}, 2));
    px_set_global("all", px_call(px_get_global("regex_find_all"), (LXValue[]){px_str("\\d+"), px_str("a1b22c333")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("regex_find_all 全部"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("all")}, 1), px_int(3LL))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("3. JSON")}, 1));
    px_set_global("obj", px_call(px_get_global("json_parse"), (LXValue[]){px_str("{\"name\": \"px\", \"v\": [1, 2, 3], \"ok\": true}")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("json_parse 对象"), ({ LXValue _t119 = px_eq(px_index(px_get_global("obj"), px_str("name")), px_str("px")); px_is_truthy(_t119) ? px_eq(px_index(px_get_global("obj"), px_str("ok")), px_bool(true)) : _t119; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("json_parse 数组"), px_eq(px_index(px_index(px_get_global("obj"), px_str("v")), px_int(2LL)), px_int(3LL))}, 2));
    px_set_global("enc", px_call(px_get_global("json_stringify"), (LXValue[]){({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_bool(true), px_null(), px_str("x")}, 3)); } _d; })}, 1));
    px_set_global("back", px_call(px_get_global("json_parse"), (LXValue[]){px_get_global("enc")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("json_stringify/decode 往返"), ({ LXValue _t121 = ({ LXValue _t120 = px_eq(px_index(px_get_global("back"), px_str("a")), px_int(1LL)); px_is_truthy(_t120) ? px_eq(px_index(px_index(px_get_global("back"), px_str("b")), px_int(0LL)), px_bool(true)) : _t120; }); px_is_truthy(_t121) ? px_eq(px_index(px_index(px_get_global("back"), px_str("b")), px_int(1LL)), px_null()) : _t121; })}, 2));
    px_set_global("jp", px_call(px_get_global("json_path"), (LXValue[]){px_str("{\"user\": {\"name\": \"px\", \"tags\": [\"c\", \"go\"]}}"), px_str("$.user.tags[1]")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("json_path 路径查询"), px_eq(px_get_global("jp"), px_str("go"))}, 2));
    px_set_global("jp2", px_call(px_get_global("json_path"), (LXValue[]){px_get_global("enc"), px_str("$.b[2]")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("json_path 字符串"), px_eq(px_get_global("jp2"), px_str("x"))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("4. 文件 IO")}, 1));
    px_set_global("tmp", px_str("/tmp/px_b1_cap.txt"));
    (void)(px_call(px_get_global("write_file"), (LXValue[]){px_get_global("tmp"), px_str("line1\nline2\n")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("write_file + read_file"), px_eq(px_call(px_get_global("read_file"), (LXValue[]){px_get_global("tmp")}, 1), px_str("line1\nline2\n"))}, 2));
    (void)(px_call(px_get_global("append_file"), (LXValue[]){px_get_global("tmp"), px_str("line3\n")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("append_file 追加"), px_eq(px_call(px_get_global("read_file"), (LXValue[]){px_get_global("tmp")}, 1), px_str("line1\nline2\nline3\n"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("exists 存在"), px_call(px_get_global("exists"), (LXValue[]){px_get_global("tmp")}, 1)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("file_size 大小"), px_eq(px_call(px_get_global("file_size"), (LXValue[]){px_get_global("tmp")}, 1), px_int(18LL))}, 2));
    px_set_global("lines", px_call(px_get_global("split"), (LXValue[]){px_call(px_get_global("read_file"), (LXValue[]){px_get_global("tmp")}, 1), px_str("\n")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("读文件按行分割"), ({ LXValue _t122 = px_eq(px_index(px_get_global("lines"), px_int(0LL)), px_str("line1")); px_is_truthy(_t122) ? px_eq(px_index(px_get_global("lines"), px_int(2LL)), px_str("line3")) : _t122; })}, 2));
    (void)(px_call(px_get_global("remove"), (LXValue[]){px_get_global("tmp")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("remove 删除"), px_not(px_call(px_get_global("exists"), (LXValue[]){px_get_global("tmp")}, 1))}, 2));
    px_set_global("d", px_str("/tmp/px_b1_dir"));
    (void)(px_call(px_get_global("mkdir"), (LXValue[]){px_get_global("d")}, 1));
    (void)(px_call(px_get_global("write_file"), (LXValue[]){px_add(px_get_global("d"), px_str("/a.txt")), px_str("A")}, 2));
    (void)(px_call(px_get_global("write_file"), (LXValue[]){px_add(px_get_global("d"), px_str("/b.txt")), px_str("B")}, 2));
    px_set_global("entries", px_call(px_get_global("list_dir"), (LXValue[]){px_get_global("d")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list_dir 目录列举"), px_ge(px_call(px_get_global("len"), (LXValue[]){px_get_global("entries")}, 1), px_int(2LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list_dir 包含文件"), ({ LXValue _t123 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("entries"), px_str("a.txt")}, 2); px_is_truthy(_t123) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("entries"), px_str("b.txt")}, 2) : _t123; })}, 2));
    (void)(px_call(px_get_global("remove"), (LXValue[]){px_add(px_get_global("d"), px_str("/a.txt"))}, 1));
    (void)(px_call(px_get_global("remove"), (LXValue[]){px_add(px_get_global("d"), px_str("/b.txt"))}, 1));
    (void)(px_call(px_get_global("remove"), (LXValue[]){px_get_global("d")}, 1));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("5. 进程调用（os_spawn → gcc 工具链）")}, 1));
    px_set_global("pid", px_call(px_get_global("os_spawn"), (LXValue[]){px_str("/bin/echo"), px_list_n((LXValue[]){px_str("b1-proc-ok")}, 1)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("os_spawn 返回 PID"), ({ LXValue _t124 = px_ne(px_get_global("pid"), px_null()); px_is_truthy(_t124) ? px_gt(px_get_global("pid"), px_int(0LL)) : _t124; })}, 2));
    px_set_global("code", px_call(px_get_global("os_wait"), (LXValue[]){px_get_global("pid")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("os_wait 退出码"), px_eq(px_get_global("code"), px_int(0LL))}, 2));
    px_set_global("gcc_pid", px_call(px_get_global("os_spawn"), (LXValue[]){px_str("gcc"), px_list_n((LXValue[]){px_str("--version")}, 1)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("gcc 可用（自举后端）"), px_ne(px_get_global("gcc_pid"), px_null())}, 2));
    (void)(px_call(px_get_global("os_wait"), (LXValue[]){px_get_global("gcc_pid")}, 1));
    px_set_global("home", px_call(px_get_global("env"), (LXValue[]){px_str("HOME")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env 环境变量"), ({ LXValue _t125 = px_ne(px_get_global("home"), px_null()); px_is_truthy(_t125) ? px_gt(px_call(px_get_global("len"), (LXValue[]){px_get_global("home")}, 1), px_int(0LL)) : _t125; })}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("6. Result / Option 错误处理（M39）")}, 1));
    px_set_global("a", px_call(px_get_global("Ok"), (LXValue[]){px_int(42LL)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Ok 构造"), ({ LXValue _t126 = px_eq(px_call(px_get_global("type"), (LXValue[]){px_get_global("a")}, 1), px_str("result")); px_is_truthy(_t126) ? px_method(px_get_global("a"), "is_ok", (LXValue[]){}, 0) : _t126; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Result 解包 unwrap"), px_eq(px_method(px_get_global("a"), "unwrap", (LXValue[]){}, 0), px_int(42LL))}, 2));
    px_set_global("b", px_call(px_get_global("Err"), (LXValue[]){px_str("boom")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Err 构造"), px_method(px_get_global("b"), "is_err", (LXValue[]){}, 0)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Result err 方法"), px_eq(px_method(px_get_global("b"), "err", (LXValue[]){}, 0), px_str("boom"))}, 2));
    px_set_global("c", px_call(px_get_global("Ok"), (LXValue[]){px_call(px_get_global("Ok"), (LXValue[]){px_int(7LL)}, 1)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Result 嵌套"), px_eq(px_method(px_method(px_get_global("c"), "unwrap", (LXValue[]){}, 0), "unwrap", (LXValue[]){}, 0), px_int(7LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Some 构造"), ({ LXValue _t127 = px_eq(px_call(px_get_global("Some"), (LXValue[]){px_int(5LL)}, 1), px_int(5LL)); px_is_truthy(_t127) ? px_eq(px_call(px_get_global("Some"), (LXValue[]){px_null()}, 1), px_null()) : _t127; })}, 2));
    px_set_global("r1", px_call(px_get_global("chain"), (LXValue[]){px_int(12LL)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("? 传播成功链"), ({ LXValue _t128 = px_method(px_get_global("r1"), "is_ok", (LXValue[]){}, 0); px_is_truthy(_t128) ? px_eq(px_method(px_get_global("r1"), "unwrap", (LXValue[]){}, 0), px_int(3LL)) : _t128; })}, 2));
    px_set_global("r2", px_call(px_get_global("chain_err"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("? 传播错误返回"), ({ LXValue _t129 = px_method(px_get_global("r2"), "is_err", (LXValue[]){}, 0); px_is_truthy(_t129) ? px_eq(px_method(px_get_global("r2"), "err", (LXValue[]){}, 0), px_str("div0")) : _t129; })}, 2));
    px_set_global("d", px_call(px_get_global("Ok"), (LXValue[]){px_int(99LL)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("! 强制解包"), px_eq(({ LXValue _t130 = px_get_global("d"); if (px_is_result(_t130)) { if (!px_result_ok(_t130)) px_error("force unwrap Err"); _t130 = px_result_unwrap(_t130); } if (px_is_null(_t130)) px_error("force unwrap null"); _t130; }), px_int(99LL))}, 2));
    px_set_global("ej", px_call(px_get_global("json_stringify"), (LXValue[]){px_call(px_get_global("Ok"), (LXValue[]){px_int(5LL)}, 1)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("Result json"), ({ LXValue _t131 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("ej"), px_str("\"ok\"")}, 2); px_is_truthy(_t131) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("ej"), px_str("5")}, 2) : _t131; })}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("7. 字符串插值（M40）")}, 1));
    px_set_global("name", px_str("px"));
    px_set_global("ver", px_int(1LL));
    px_set_global("kv", ({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } { LXValue _k = px_str("k1"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(3LL)); } _d; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("插值-变量"), px_eq(px_add(px_add(px_str("hello "), px_call(px_get_global("str"), (LXValue[]){px_get_global("name")}, 1)), px_str("")), px_str("hello px"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("插值-表达式"), px_eq(px_add(px_add(px_str(""), px_call(px_get_global("str"), (LXValue[]){px_add(px_int(1LL), px_mul(px_int(2LL), px_int(3LL)))}, 1)), px_str("")), px_str("7"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("插值-混合"), px_eq(px_add(px_add(px_add(px_add(px_str("v"), px_call(px_get_global("str"), (LXValue[]){px_get_global("ver")}, 1)), px_str(".")), px_call(px_get_global("str"), (LXValue[]){px_get_global("name")}, 1)), px_str("")), px_str("v1.px"))}, 2));
    px_set_global("kv_v", px_index(px_get_global("kv"), px_str("k")));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("插值-属性/索引"), px_eq(px_add(px_add(px_str(""), px_call(px_get_global("str"), (LXValue[]){px_get_global("kv_v")}, 1)), px_str("")), px_str("2"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("插值-转义"), px_eq(px_str("${name}"), px_add(px_str("$"), px_str("{name}")))}, 2));
    px_set_global("kn", px_add(px_str("k"), px_call(px_get_global("str"), (LXValue[]){px_get_global("ver")}, 1)));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("插值-嵌套"), px_eq(px_add(px_add(px_str(""), px_call(px_get_global("str"), (LXValue[]){px_index(px_get_global("kv"), px_get_global("kn"))}, 1)), px_str("")), px_str("3"))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("8. enum + match（AST 表示）")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("enum 构造+match 基础"), px_eq(px_call(px_get_global("kind_name"), (LXValue[]){px_enum("TokenKind", "Ident")}, 1), px_str("ident"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("enum 相等比较"), px_eq(px_enum("TokenKind", "Int"), px_enum("TokenKind", "Int"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("enum 不相等"), px_ne(px_enum("TokenKind", "Int"), px_enum("TokenKind", "Eof"))}, 2));
    px_set_global("n1", px_struct("AstNode", (char*[]){"kind", "value", "name", "args"}, (LXValue[]){px_enum("ExprKind", "Number"), px_int(7LL), px_str(""), px_list_n((LXValue[]){}, 0)}, 4));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("enum+struct 表示 AST 节点"), ({ LXValue _t132 = px_eq(px_field(px_get_global("n1"), "kind"), px_enum("ExprKind", "Number")); px_is_truthy(_t132) ? px_eq(px_field(px_get_global("n1"), "value"), px_int(7LL)) : _t132; })}, 2));
    px_set_global("call", px_struct("AstNode", (char*[]){"kind", "value", "name", "args"}, (LXValue[]){px_enum("ExprKind", "Call"), px_int(0LL), px_str("fib"), px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 4));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("struct 载荷 args"), ({ LXValue _t133 = px_eq(px_field(px_get_global("call"), "name"), px_str("fib")); px_is_truthy(_t133) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_field(px_get_global("call"), "args")}, 1), px_int(2LL)) : _t133; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("match 分发 AST 节点"), px_eq(px_call(px_get_global("node_desc"), (LXValue[]){px_get_global("n1")}, 1), px_str("num:7"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("match 分发 call"), px_eq(px_call(px_get_global("node_desc"), (LXValue[]){px_get_global("call")}, 1), px_str("call:fib"))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("9. list / dict 容器")}, 1));
    px_set_global("lst", px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3));
    (void)(px_method(px_get_global("lst"), "append", (LXValue[]){px_int(4LL)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list append"), ({ LXValue _t134 = px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("lst")}, 1), px_int(4LL)); px_is_truthy(_t134) ? px_eq(px_index(px_get_global("lst"), px_int(3LL)), px_int(4LL)) : _t134; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list 切片"), px_eq(px_slice(px_get_global("lst"), px_int(1LL), px_int(3LL), px_null()), px_list_n((LXValue[]){px_int(2LL), px_int(3LL)}, 2))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list 负数索引"), px_eq(px_index(px_get_global("lst"), px_neg(px_int(1LL))), px_int(4LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list contains"), ({ LXValue _t135 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("lst"), px_int(3LL)}, 2); px_is_truthy(_t135) ? px_not(px_call(px_get_global("contains"), (LXValue[]){px_get_global("lst"), px_int(99LL)}, 2)) : _t135; })}, 2));
    px_set_global("rev", px_call(px_get_global("reversed"), (LXValue[]){px_get_global("lst")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("reversed 反转"), ({ LXValue _t136 = px_eq(px_index(px_get_global("rev"), px_int(0LL)), px_int(4LL)); px_is_truthy(_t136) ? px_eq(px_index(px_get_global("rev"), px_int(3LL)), px_int(1LL)) : _t136; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("sorted 排序"), px_eq(px_call(px_get_global("sorted"), (LXValue[]){px_list_n((LXValue[]){px_int(3LL), px_int(1LL), px_int(2LL)}, 3)}, 1), px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3))}, 2));
    px_set_global("dct", ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; }));
    px_index_set(px_get_global("dct"), px_str("c"), px_int(3LL));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict 增改"), ({ LXValue _t137 = px_eq(px_index(px_get_global("dct"), px_str("c")), px_int(3LL)); px_is_truthy(_t137) ? px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("dct")}, 1), px_int(3LL)) : _t137; })}, 2));
    px_set_global("ks", px_method(px_get_global("dct"), "keys", (LXValue[]){}, 0));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict keys"), ({ LXValue _t138 = px_call(px_get_global("contains"), (LXValue[]){px_get_global("ks"), px_str("a")}, 2); px_is_truthy(_t138) ? px_call(px_get_global("contains"), (LXValue[]){px_get_global("ks"), px_str("c")}, 2) : _t138; })}, 2));
    px_set_global("vs", px_method(px_get_global("dct"), "values", (LXValue[]){}, 0));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict values"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("vs")}, 1), px_int(3LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict has"), ({ LXValue _t139 = px_method(px_get_global("dct"), "has", (LXValue[]){px_str("b")}, 1); px_is_truthy(_t139) ? px_not(px_method(px_get_global("dct"), "has", (LXValue[]){px_str("z")}, 1)) : _t139; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict remove"), ({ LXValue _t140 = px_method(px_get_global("dct"), "remove", (LXValue[]){px_str("a")}, 1); px_is_truthy(_t140) ? px_not(px_method(px_get_global("dct"), "has", (LXValue[]){px_str("a")}, 1)) : _t140; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict get 默认值"), px_eq(px_method(px_get_global("dct"), "get", (LXValue[]){px_str("zz"), px_int(9LL)}, 2), px_int(9LL))}, 2));
    px_set_global("matrix", px_list_n((LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_list_n((LXValue[]){px_int(3LL), px_int(4LL)}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("二维列表"), px_eq(px_index(px_index(px_get_global("matrix"), px_int(1LL)), px_int(0LL)), px_int(3LL))}, 2));
    px_set_global("deep", ({ LXValue _d = px_dict(); { LXValue _k = px_str("x"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ({ LXValue _d = px_dict(); { LXValue _k = px_str("y"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){({ LXValue _d = px_dict(); { LXValue _k = px_str("z"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(42LL)); } _d; })}, 1)); } _d; })); } _d; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("深嵌套访问"), px_eq(px_index(px_index(px_index(px_index(px_get_global("deep"), px_str("x")), px_str("y")), px_int(0LL)), px_str("z")), px_int(42LL))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("10. 函数/闭包/高阶函数/递归")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("递归 fact"), px_eq(px_call(px_get_global("fact"), (LXValue[]){px_int(5LL)}, 1), px_int(120LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("map"), px_eq(px_call(px_get_global("map"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3), px_func("<closure2>", fn_closure_2, NULL)}, 2), px_list_n((LXValue[]){px_int(2LL), px_int(4LL), px_int(6LL)}, 3))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("filter"), px_eq(px_call(px_get_global("filter"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL), px_int(4LL)}, 4), px_func("<closure3>", fn_closure_3, NULL)}, 2), px_list_n((LXValue[]){px_int(3LL), px_int(4LL)}, 2))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("reduce"), px_eq(px_call(px_get_global("reduce"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL), px_int(4LL)}, 4), px_func("<closure4>", fn_closure_4, NULL), px_int(0LL)}, 3), px_int(10LL))}, 2));
    px_set_global("k", px_call(px_get_global("make_outer"), (LXValue[]){}, 0));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("闭包（无捕获）"), px_eq(px_call(px_get_global("k"), (LXValue[]){}, 0), px_int(42LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("默认参数"), ({ LXValue _t145 = px_eq(px_call(px_get_global("greet"), (LXValue[]){px_str("px")}, 1), px_str("hi px!")); px_is_truthy(_t145) ? px_eq(px_call(px_get_global("greet"), (LXValue[]){px_str("px"), px_str("?")}, 2), px_str("hi px?")) : _t145; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("list 参数聚合"), px_eq(px_call(px_get_global("sum_all"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL), px_int(4LL)}, 4)}, 1), px_int(10LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("关键字参数"), px_eq(px_call(px_get_global("cfg"), (LXValue[]){px_str("localhost")}, 1), px_str("localhost:8080"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("关键字参数-tls"), px_eq(px_call(px_get_global("cfg"), (LXValue[]){px_str("localhost"), px_int(8080LL), px_bool(true)}, 3), px_str("localhost:8080s"))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("11. 控制流")}, 1));
    px_set_global("acc", px_int(0LL));
    LXValue _t146 = px_call(px_get_global("range"), (LXValue[]){px_int(5LL)}, 1);
    for (int _t147 = 0; _t147 < px_len(_t146); _t147++) {
        LXValue _v148 = px_index(_t146, px_int(_t147));
        px_set_global("acc", px_add(px_get_global("acc"), _v148));
    }
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("for range 累加"), px_eq(px_get_global("acc"), px_int(10LL))}, 2));
    px_set_global("w", px_int(0LL));
    while (px_is_truthy(px_lt(px_get_global("w"), px_int(3LL)))) {
        px_set_global("w", px_add(px_get_global("w"), px_int(1LL)));
    }
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("while 循环"), px_eq(px_get_global("w"), px_int(3LL))}, 2));
    px_set_global("bc", px_int(0LL));
    LXValue _t149 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1);
    for (int _t150 = 0; _t150 < px_len(_t149); _t150++) {
        LXValue _v151 = px_index(_t149, px_int(_t150));
        if (px_is_truthy(px_eq(_v151, px_int(2LL)))) {
            continue;
        }
        if (px_is_truthy(px_eq(_v151, px_int(5LL)))) {
            break;
        }
        px_set_global("bc", px_add(px_get_global("bc"), px_int(1LL)));
    }
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("break/continue"), px_eq(px_get_global("bc"), px_int(4LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("列表推导式"), px_eq(({ LXValue _t152 = px_list(0); LXValue _t153 = px_call(px_get_global("range"), (LXValue[]){px_int(5LL)}, 1); for (int _t155=0; _t155<px_len(_t153); _t155++) { LXValue _t154 = px_index(_t153, px_int(_t155)); LXValue _cv156 = _t154; px_list_push(_t152, px_mul(_cv156, _cv156));  }  _t152; }), px_list_n((LXValue[]){px_int(0LL), px_int(1LL), px_int(4LL), px_int(9LL), px_int(16LL)}, 5))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("推导式带条件"), px_eq(({ LXValue _t157 = px_list(0); LXValue _t158 = px_call(px_get_global("range"), (LXValue[]){px_int(6LL)}, 1); for (int _t160=0; _t160<px_len(_t158); _t160++) { LXValue _t159 = px_index(_t158, px_int(_t160)); LXValue _cv161 = _t159; if (px_is_truthy(px_eq(px_mod(_cv161, px_int(2LL)), px_int(0LL)))) { px_list_push(_t157, _cv161); }  }  _t157; }), px_list_n((LXValue[]){px_int(0LL), px_int(2LL), px_int(4LL)}, 3))}, 2));
    px_set_global("dc", ({ LXValue _t162 = px_dict(); LXValue _t163 = px_call(px_get_global("range"), (LXValue[]){px_int(3LL)}, 1); for (int _t165=0; _t165<px_len(_t163); _t165++) { LXValue _t164 = px_index(_t163, px_int(_t165)); LXValue _cv166 = _t164; { LXValue _k = px_call(px_get_global("str"), (LXValue[]){_cv166}, 1); LXValue _v = _cv166; if (_k.type == PX_STR) px_dict_set(_t162, _k.as.obj->as.str.data, _v); }  }  _t162; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("dict 推导式"), ({ LXValue _t167 = px_eq(px_index(px_get_global("dc"), px_str("1")), px_int(1LL)); px_is_truthy(_t167) ? px_eq(px_index(px_get_global("dc"), px_str("2")), px_int(2LL)) : _t167; })}, 2));
    px_set_global("g", px_gen_lazy(px_call(px_get_global("range"), (LXValue[]){px_int(4LL)}, 1), px_func("<closure5>", fn_closure_5, NULL), px_null()));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("生成器 gen_next"), ({ LXValue _t170 = px_eq(px_call(px_get_global("gen_next"), (LXValue[]){px_get_global("g")}, 1), px_int(0LL)); px_is_truthy(_t170) ? px_eq(px_call(px_get_global("gen_next"), (LXValue[]){px_get_global("g")}, 1), px_int(2LL)) : _t170; })}, 2));
    px_set_global("g2", px_gen_lazy(px_call(px_get_global("range"), (LXValue[]){px_int(3LL)}, 1), px_func("<closure6>", fn_closure_6, NULL), px_null()));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("生成器 list 转换"), px_eq(px_call(px_get_global("list"), (LXValue[]){px_get_global("g2")}, 1), px_list_n((LXValue[]){px_int(0LL), px_int(1LL), px_int(2LL)}, 3))}, 2));
    px_set_global("t1", ({ LXValue _t173; if (px_is_truthy(px_bool(true))) { _t173 = px_int(5LL); } else { _t173 = px_int(9LL); } _t173; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("三元表达式"), px_eq(px_get_global("t1"), px_int(5LL))}, 2));
    px_set_global("d1", ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ({ LXValue _d = px_dict(); { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(42LL)); } _d; })); } _d; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("可选链 ?."), px_eq(({ LXValue _t175 = ({ LXValue _t174 = px_get_global("d1"); px_is_null(_t174) ? px_null() : px_field(px_get_global("d1"), "a"); }); px_is_null(_t175) ? px_null() : px_field(({ LXValue _t174 = px_get_global("d1"); px_is_null(_t174) ? px_null() : px_field(px_get_global("d1"), "a"); }), "b"); }), px_int(42LL))}, 2));
    px_set_global("d2", ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_null()); } _d; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("可选链短路"), px_eq(({ LXValue _t177 = ({ LXValue _t176 = px_get_global("d2"); px_is_null(_t176) ? px_null() : px_field(px_get_global("d2"), "a"); }); px_is_null(_t177) ? px_null() : px_field(({ LXValue _t176 = px_get_global("d2"); px_is_null(_t176) ? px_null() : px_field(px_get_global("d2"), "a"); }), "b"); }), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("空合并 ??"), px_eq(({ LXValue _t178 = px_index(px_get_global("d2"), px_str("a")); px_is_null(_t178) ? px_int(99LL) : _t178; }), px_int(99LL))}, 2));
    px_set_global("mv", ({ LXValue _t179 = px_int(3LL); if (px_is_truthy(px_eq(_t179, px_int(1LL)))) { _t179 = ({ LXValue _blk = px_null(); _blk = px_str("one"); _blk; }); } else if (px_is_truthy(px_eq(_t179, px_int(3LL)))) { _t179 = ({ LXValue _blk = px_null(); _blk = px_str("three"); _blk; }); } else if (true) { _t179 = ({ LXValue _blk = px_null(); _blk = px_str("other"); _blk; }); } _t179; }));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("match 表达式"), px_eq(px_get_global("mv"), px_str("three"))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("12. 模块 / import")}, 1));
    px_set_global("uq", px_call(px_get_global("unique"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(2LL), px_int(3LL), px_int(3LL), px_int(3LL)}, 6)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("import 模块函数"), px_eq(px_get_global("uq"), px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3))}, 2));
    px_set_global("fl", px_call(px_get_global("flatten"), (LXValue[]){px_list_n((LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_list_n((LXValue[]){px_int(3LL)}, 1)}, 2)}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("import flatten"), px_eq(px_get_global("fl"), px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3))}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("13. 类型系统（Rust 风格）")}, 1));
    px_set_global("p", px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_float(3), px_float(4)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("struct 构造"), ({ LXValue _t180 = px_eq(px_field(px_get_global("p"), "x"), px_float(3)); px_is_truthy(_t180) ? px_eq(px_field(px_get_global("p"), "y"), px_float(4)) : _t180; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("trait impl 扩展方法"), px_eq(px_method(px_get_global("p"), "area", (LXValue[]){}, 0), px_float(12))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("类型标注函数"), px_eq(px_call(px_get_global("typed"), (LXValue[]){px_int(5LL), px_str("n=")}, 2), px_str("n=5"))}, 2));
    px_set_global("lv", px_int(1LL));
    px_set_global("cv", px_int(2LL));
    px_set_global("vv", px_int(3LL));
    px_set_global("vv", px_add(px_get_global("vv"), px_int(1LL)));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("let/const/var"), ({ LXValue _t182 = ({ LXValue _t181 = px_eq(px_get_global("lv"), px_int(1LL)); px_is_truthy(_t181) ? px_eq(px_get_global("cv"), px_int(2LL)) : _t181; }); px_is_truthy(_t182) ? px_eq(px_get_global("vv"), px_int(4LL)) : _t182; })}, 2));
    (void)(px_call(px_get_global("section"), (LXValue[]){px_str("14. 位运算 / 进制")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("位与"), px_eq(px_bitand(px_int(12LL), px_int(10LL)), px_int(8LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("位或"), px_eq(px_bitor(px_int(12LL), px_int(10LL)), px_int(14LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("位移"), ({ LXValue _t183 = px_eq(px_shl(px_int(1LL), px_int(4LL)), px_int(16LL)); px_is_truthy(_t183) ? px_eq(px_shr(px_int(16LL), px_int(2LL)), px_int(4LL)) : _t183; })}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("hex 转换"), ({ LXValue _t184 = px_eq(px_call(px_get_global("hex_to_int"), (LXValue[]){px_str("ff")}, 1), px_int(255LL)); px_is_truthy(_t184) ? px_eq(px_call(px_get_global("int_to_hex"), (LXValue[]){px_int(255LL), px_int(2LL)}, 2), px_str("ff")) : _t184; })}, 2));
    px_set_global("bb", px_call(px_get_global("int_to_bytes"), (LXValue[]){px_int(258LL), px_int(2LL)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("bytes 转换"), ({ LXValue _t185 = px_eq(px_call(px_get_global("bytes_to_int"), (LXValue[]){px_get_global("bb")}, 1), px_int(258LL)); px_is_truthy(_t185) ? px_eq(px_call(px_get_global("type"), (LXValue[]){px_get_global("bb")}, 1), px_str("bytes")) : _t185; })}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("========== 能力自检汇总 ==========")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_str("PASS: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_pass")}, 1)), px_str("  FAIL: ")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_fail")}, 1))}, 1));
    if (px_is_truthy(px_gt(px_get_global("g_fail"), px_int(0LL)))) {
        (void)(px_call(px_get_global("print"), (LXValue[]){px_str("失败项：")}, 1));
        LXValue _t186 = px_get_global("g_fails");
        for (int _t187 = 0; _t187 < px_len(_t186); _t187++) {
            LXValue _v188 = px_index(_t186, px_int(_t187));
            (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("  - "), _v188)}, 1));
        }
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    else {
        (void)(px_call(px_get_global("print"), (LXValue[]){px_str("门禁通过：写编译器所需能力全部具备 ✅")}, 1));
    }
    return 0;
}
