#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v115 = (nargs > 0) ? args[0] : px_null();
    LXValue _v116 = (nargs > 1) ? args[1] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_add(_v115, _v116); _blk; });
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v117 = (nargs > 0) ? args[0] : px_null();
    return px_mul(_v117, _v117);
}
static LXValue fn_closure_3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v118 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_119_val = px_null();
    int px_err_119_proped = 0;
    return px_mul(_v118, _v118);
px_err_119:
    if (px_err_119_proped) return px_err_119_val;
    return px_null();
}

static LXValue fn_Point_area(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    return px_float(1.5);
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
    return px_null();
}

static LXValue fn_each(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v2 = (nargs > 0) ? args[0] : px_null();
    LXValue _v3 = (nargs > 1) ? args[1] : px_null();
    LXValue _v4 = px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    LXValue _t6 = _v2;
    for (int _t7 = 0; _t7 < px_len(_t6); _t7++) {
        _v4 = px_index(_t6, px_int(_t7));
        (void)(px_call(_v3, (LXValue[]){_v4}, 1));
    }
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_unique(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = px_null();
    LXValue _v10 = px_null();
    LXValue px_err_11_val = px_null();
    int px_err_11_proped = 0;
     _v9 = px_list_n((LXValue[]){}, 0);
    LXValue _t12 = _v8;
    for (int _t13 = 0; _t13 < px_len(_t12); _t13++) {
        _v10 = px_index(_t12, px_int(_t13));
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v9, _v10}, 2)))) {
            (void)(px_method(_v9, "append", (LXValue[]){_v10}, 1));
        }
    }
    return _v9;
px_err_11:
    if (px_err_11_proped) return px_err_11_val;
    return px_null();
}

static LXValue fn_flatten(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue _v15 = px_null();
    LXValue _v16 = px_null();
    LXValue _v17 = px_null();
    LXValue px_err_18_val = px_null();
    int px_err_18_proped = 0;
     _v15 = px_list_n((LXValue[]){}, 0);
    LXValue _t19 = _v14;
    for (int _t20 = 0; _t20 < px_len(_t19); _t20++) {
        _v16 = px_index(_t19, px_int(_t20));
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v16}, 1), px_str("list")))) {
            LXValue _t21 = _v16;
            for (int _t22 = 0; _t22 < px_len(_t21); _t22++) {
                _v17 = px_index(_t21, px_int(_t22));
                (void)(px_method(_v15, "append", (LXValue[]){_v17}, 1));
            }
        }
        else {
            (void)(px_method(_v15, "append", (LXValue[]){_v16}, 1));
        }
    }
    return _v15;
px_err_18:
    if (px_err_18_proped) return px_err_18_val;
    return px_null();
}

static LXValue fn_zip_lists(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v23 = (nargs > 0) ? args[0] : px_null();
    LXValue _v24 = (nargs > 1) ? args[1] : px_null();
    LXValue _v25 = px_null();
    LXValue _v26 = px_null();
    LXValue _v27 = px_null();
    LXValue px_err_28_val = px_null();
    int px_err_28_proped = 0;
     _v25 = px_list_n((LXValue[]){}, 0);
     _v26 = px_call(px_get_global("min"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v23}, 1), px_call(px_get_global("len"), (LXValue[]){_v24}, 1)}, 2);
    LXValue _t29 = px_call(px_get_global("range"), (LXValue[]){_v26}, 1);
    for (int _t30 = 0; _t30 < px_len(_t29); _t30++) {
        _v27 = px_index(_t29, px_int(_t30));
        (void)(px_method(_v25, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v23, _v27), px_index(_v24, _v27)}, 2)}, 1));
    }
    return _v25;
px_err_28:
    if (px_err_28_proped) return px_err_28_val;
    return px_null();
}

static LXValue fn_chunk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v31 = (nargs > 0) ? args[0] : px_null();
    LXValue _v32 = (nargs > 1) ? args[1] : px_null();
    LXValue _v33 = px_null();
    LXValue _v34 = px_null();
    LXValue _v35 = px_null();
    LXValue _v36 = px_null();
    LXValue px_err_37_val = px_null();
    int px_err_37_proped = 0;
     _v33 = px_list_n((LXValue[]){}, 0);
     _v34 = px_int(0LL);
    while (px_is_truthy(px_lt(_v34, px_call(px_get_global("len"), (LXValue[]){_v31}, 1)))) {
         _v35 = px_list_n((LXValue[]){}, 0);
         _v36 = px_int(0LL);
        while (px_is_truthy(({ LXValue _t38 = px_lt(_v36, _v32); px_is_truthy(_t38) ? px_lt(_v34, px_call(px_get_global("len"), (LXValue[]){_v31}, 1)) : _t38; }))) {
            (void)(px_method(_v35, "append", (LXValue[]){px_index(_v31, _v34)}, 1));
             _v34 = px_add(_v34, px_int(1LL));
             _v36 = px_add(_v36, px_int(1LL));
        }
        (void)(px_method(_v33, "append", (LXValue[]){_v35}, 1));
    }
    return _v33;
px_err_37:
    if (px_err_37_proped) return px_err_37_val;
    return px_null();
}

static LXValue fn_group_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v39 = (nargs > 0) ? args[0] : px_null();
    LXValue _v40 = (nargs > 1) ? args[1] : px_null();
    LXValue _v41 = px_null();
    LXValue _v42 = px_null();
    LXValue _v43 = px_null();
    LXValue _v44 = px_null();
    LXValue px_err_45_val = px_null();
    int px_err_45_proped = 0;
     _v41 = px_call(px_get_global("json_parse"), (LXValue[]){px_str("{}")}, 1);
    LXValue _t46 = _v39;
    for (int _t47 = 0; _t47 < px_len(_t46); _t47++) {
        _v42 = px_index(_t46, px_int(_t47));
         _v43 = px_call(px_get_global("str"), (LXValue[]){px_call(_v40, (LXValue[]){_v42}, 1)}, 1);
        if (px_is_truthy(px_method(_v41, "has", (LXValue[]){_v43}, 1))) {
            _v44 = px_index(_v41, _v43);
            (void)(px_method(_v44, "append", (LXValue[]){_v42}, 1));
            (void)(px_method(_v41, "set", (LXValue[]){_v43, _v44}, 2));
        }
        else {
            (void)(px_method(_v41, "set", (LXValue[]){_v43, px_list_n((LXValue[]){_v42}, 1)}, 2));
        }
    }
    return _v41;
px_err_45:
    if (px_err_45_proped) return px_err_45_val;
    return px_null();
}

static LXValue fn_sort_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v48 = (nargs > 0) ? args[0] : px_null();
    LXValue _v49 = (nargs > 1) ? args[1] : px_null();
    LXValue _v50 = px_null();
    LXValue _v51 = px_null();
    LXValue _v52 = px_null();
    LXValue _v53 = px_null();
    LXValue px_err_54_val = px_null();
    int px_err_54_proped = 0;
     _v50 = px_list_n((LXValue[]){}, 0);
    LXValue _t55 = _v48;
    for (int _t56 = 0; _t56 < px_len(_t55); _t56++) {
        _v51 = px_index(_t55, px_int(_t56));
        (void)(px_method(_v50, "append", (LXValue[]){px_list_n((LXValue[]){px_call(_v49, (LXValue[]){_v51}, 1), _v51}, 2)}, 1));
    }
     _v50 = px_call(px_get_global("sorted"), (LXValue[]){_v50}, 1);
     _v52 = px_list_n((LXValue[]){}, 0);
    LXValue _t57 = _v50;
    for (int _t58 = 0; _t58 < px_len(_t57); _t58++) {
        _v53 = px_index(_t57, px_int(_t58));
        (void)(px_method(_v52, "append", (LXValue[]){px_index(_v53, px_int(1LL))}, 1));
    }
    return _v52;
px_err_54:
    if (px_err_54_proped) return px_err_54_val;
    return px_null();
}

static LXValue fn_greet(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v59 = (nargs > 0) ? args[0] : px_null();
    LXValue _v60 = (nargs > 1) ? args[1] : px_int(2LL);
    LXValue _v61 = px_null();
    LXValue _v62 = px_null();
    LXValue px_err_63_val = px_null();
    int px_err_63_proped = 0;
    _v61 = px_add(px_add(px_str("hi "), px_call(px_get_global("str"), (LXValue[]){_v59}, 1)), px_str(""));
    _v62 = px_int(0LL);
    while (px_is_truthy(px_lt(_v62, _v60))) {
         _v61 = px_add(_v61, px_str("!"));
         _v62 = px_add(_v62, px_int(1LL));
    }
    return _v61;
px_err_63:
    if (px_err_63_proped) return px_err_63_val;
    return px_null();
}

static LXValue fn_process(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v64 = (nargs > 0) ? args[0] : px_null();
    LXValue _v65 = px_null();
    LXValue _v66 = px_null();
    LXValue _v67 = px_null();
    LXValue _v68 = px_null();
    LXValue _v69 = px_null();
    LXValue _v70 = px_null();
    LXValue _v71 = px_null();
    LXValue _v72 = px_null();
    LXValue _v73 = px_null();
    LXValue px_err_74_val = px_null();
    int px_err_74_proped = 0;
    _v65 = ({ LXValue _t75 = px_list(0); LXValue _t76 = _v64; for (int _t78=0; _t78<px_len(_t76); _t78++) { LXValue _t77 = px_index(_t76, px_int(_t78)); LXValue _cv79 = _t77; if (px_is_truthy(px_gt(_cv79, px_int(0LL)))) { px_list_push(_t75, px_mul(_cv79, px_int(2LL))); }  }  _t75; });
    _v66 = ({ LXValue _t80 = px_list(0); LXValue _t81 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1); for (int _t83=0; _t83<px_len(_t81); _t83++) { LXValue _t82 = px_index(_t81, px_int(_t83)); LXValue _cv84 = _t82; if (px_is_truthy(({ LXValue _t85 = px_eq(px_mod(_cv84, px_int(2LL)), px_int(1LL)); px_is_truthy(_t85) ? px_ne(_cv84, px_int(5LL)) : _t85; }))) { px_list_push(_t80, _cv84); }  }  _t80; });
    _v67 = ({ LXValue _t86 = px_dict(); LXValue _t87 = px_method(_v64, "items", (LXValue[]){}, 0); for (int _t89=0; _t89<px_len(_t87); _t89++) { LXValue _t88 = px_index(_t87, px_int(_t89)); LXValue _cv90_0 = px_index(_t88, px_int(0)); LXValue _cv91_1 = px_index(_t88, px_int(1)); { LXValue _k = px_call(px_get_global("str"), (LXValue[]){_cv90_0}, 1); LXValue _v = _cv90_0; if (_k.type == PX_STR) px_dict_set(_t86, _k.as.obj->as.str.data, _v); }  }  _t86; });
    _v68 = px_index(_v64, px_int(0LL));
    _v69 = px_index(_v64, px_neg(px_int(1LL)));
    _v70 = px_slice(_v64, px_int(1LL), px_int(3LL), px_null());
    _v71 = px_slice(_v64, px_null(), px_null(), px_null());
    _v72 = px_slice(_v64, px_null(), px_null(), px_neg(px_int(1LL)));
    _v73 = px_slice(_v64, px_null(), px_null(), px_int(2LL));
    return px_list_n((LXValue[]){_v65, _v66, _v67, _v68, _v69, _v70, _v71, _v72, _v73}, 9);
px_err_74:
    if (px_err_74_proped) return px_err_74_val;
    return px_null();
}

static LXValue fn_use_ops(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v92 = (nargs > 0) ? args[0] : px_null();
    LXValue _v93 = (nargs > 1) ? args[1] : px_null();
    LXValue _v94 = px_null();
    LXValue _v95 = px_null();
    LXValue _v96 = px_null();
    LXValue _v97 = px_null();
    LXValue _v98 = px_null();
    LXValue _v99 = px_null();
    LXValue px_err_100_val = px_null();
    int px_err_100_proped = 0;
    _v94 = px_call(px_get_global("double"), (LXValue[]){_v92}, 1);
    _v95 = ({ LXValue _t101 = _v92; px_is_null(_t101) ? px_int(42LL) : _t101; });
    _v96 = ({ LXValue _t102 = _v92; px_is_null(_t102) ? px_null() : px_field(_v92, "name"); });
    _v97 = ({ LXValue _t103 = _v92; if (px_is_result(_t103)) { if (!px_result_ok(_t103)) px_error("force unwrap Err"); _t103 = px_result_unwrap(_t103); } if (px_is_null(_t103)) px_error("force unwrap null"); _t103; });
    _v98 = px_field(px_field(({ LXValue _t104 = _v92; if (px_is_result(_t104)) { if (!px_result_ok(_t104)) px_error("force unwrap Err"); _t104 = px_result_unwrap(_t104); } if (px_is_null(_t104)) px_error("force unwrap null"); _t104; }), "b"), "c");
    _v99 = px_index(px_index(_v92, px_int(0LL)), px_int(1LL));
    return px_add(px_add(px_add(px_add(px_add(_v94, _v95), px_call(px_get_global("len"), (LXValue[]){_v96}, 1)), _v97), _v98), _v99);
px_err_100:
    if (px_err_100_proped) return px_err_100_val;
    return px_null();
}

static LXValue fn_double(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v105 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_106_val = px_null();
    int px_err_106_proped = 0;
    return px_mul(_v105, px_int(2LL));
px_err_106:
    if (px_err_106_proped) return px_err_106_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v107 = px_null();
    LXValue _v108 = px_null();
    LXValue _v109 = px_null();
    LXValue _v110 = px_null();
    LXValue _v111 = px_null();
    LXValue _v112 = px_null();
    LXValue px_err_113_val = px_null();
    int px_err_113_proped = 0;
    _v107 = px_list_n((LXValue[]){px_enum("Shape", "Circle"), px_enum("Shape", "Square")}, 2);
    _v108 = px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_int(3LL), px_int(4LL)}, 2);
    _v109 = ({ LXValue _t114 = px_index(_v107, px_int(0LL)); if ((_t114.type == PX_ENUM && strcmp(_t114.as.obj->as.enum_inst.variant, "Circle") == 0)) { _t114 = ({ LXValue _blk = px_null(); _blk = px_str("circle"); _blk; }); } else if ((_t114.type == PX_ENUM && strcmp(_t114.as.obj->as.enum_inst.variant, "Square") == 0)) { _t114 = ({ LXValue _blk = px_null(); _blk = px_str("square"); _blk; }); } _t114; });
    (void)(px_call(px_get_global("print"), (LXValue[]){_v109}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){fn_Point_area((LXValue[]){_v108}, 1, NULL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px")}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px"), px_int(3LL)}, 2)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("process"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_neg(px_int(2LL)), px_int(3LL), px_int(4LL)}, 4)}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("use_ops"), (LXValue[]){px_int(5LL), px_int(6LL)}, 2)}, 1));
    _v110 = px_func("<closure1>", fn_closure_1, NULL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v110, (LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 1));
    _v111 = px_func("<closure2>", fn_closure_2, NULL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v111, (LXValue[]){px_int(9LL)}, 1)}, 1));
    _v112 = px_gen_lazy(px_call(px_get_global("range"), (LXValue[]){px_int(4LL)}, 1), px_func("<closure3>", fn_closure_3, NULL), px_null());
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("list"), (LXValue[]){_v112}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(31LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(10LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_float(250)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_neg(px_int(7LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_not(px_bool(true))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_idiv(px_int(5LL), px_int(2LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_pow(px_int(2LL), px_int(8LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shl(px_int(1LL), px_int(4LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shr(px_int(255LL), px_int(2LL))}, 1));
px_err_113:
    if (px_err_113_proped) return px_err_113_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("each", px_func("each", fn_each, NULL));
    px_set_global("unique", px_func("unique", fn_unique, NULL));
    px_set_global("flatten", px_func("flatten", fn_flatten, NULL));
    px_set_global("zip_lists", px_func("zip_lists", fn_zip_lists, NULL));
    px_set_global("chunk", px_func("chunk", fn_chunk, NULL));
    px_set_global("group_by", px_func("group_by", fn_group_by, NULL));
    px_set_global("sort_by", px_func("sort_by", fn_sort_by, NULL));
    px_set_global("greet", px_func("greet", fn_greet, NULL));
    px_set_global("process", px_func("process", fn_process, NULL));
    px_set_global("use_ops", px_func("use_ops", fn_use_ops, NULL));
    px_set_global("double", px_func("double", fn_double, NULL));
    px_set_global("main", px_func("main", fn_main, NULL));
    px_set_global("Point.area", px_func("Point.area", fn_Point_area, NULL));
    px_set_global("MAX_N", px_int(100LL));
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
