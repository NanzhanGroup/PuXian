#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v114 = (nargs > 0) ? args[0] : px_null();
    LXValue _v115 = (nargs > 1) ? args[1] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_add(_v114, _v115); _blk; });
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v116 = (nargs > 0) ? args[0] : px_null();
    return px_mul(_v116, _v116);
}
static LXValue fn_closure_3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v117 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_118_val = px_null();
    int px_err_118_proped = 0;
    return px_mul(_v117, _v117);
px_err_118:
    if (px_err_118_proped) return px_err_118_val;
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
    LXValue px_err_44_val = px_null();
    int px_err_44_proped = 0;
     _v41 = ({ LXValue _blk = px_null(); _blk; });
    LXValue _t45 = _v39;
    for (int _t46 = 0; _t46 < px_len(_t45); _t46++) {
        _v42 = px_index(_t45, px_int(_t46));
         _v43 = px_call(px_get_global("str"), (LXValue[]){px_call(_v40, (LXValue[]){_v42}, 1)}, 1);
        if (px_is_truthy(px_method(_v41, "has", (LXValue[]){_v43}, 1))) {
            (void)(px_method(px_index(_v41, _v43), "append", (LXValue[]){_v42}, 1));
        }
        else {
            px_index_set(_v41, _v43, px_list_n((LXValue[]){_v42}, 1));
        }
    }
    return _v41;
px_err_44:
    if (px_err_44_proped) return px_err_44_val;
    return px_null();
}

static LXValue fn_sort_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v47 = (nargs > 0) ? args[0] : px_null();
    LXValue _v48 = (nargs > 1) ? args[1] : px_null();
    LXValue _v49 = px_null();
    LXValue _v50 = px_null();
    LXValue _v51 = px_null();
    LXValue _v52 = px_null();
    LXValue px_err_53_val = px_null();
    int px_err_53_proped = 0;
     _v49 = px_list_n((LXValue[]){}, 0);
    LXValue _t54 = _v47;
    for (int _t55 = 0; _t55 < px_len(_t54); _t55++) {
        _v50 = px_index(_t54, px_int(_t55));
        (void)(px_method(_v49, "append", (LXValue[]){px_list_n((LXValue[]){px_call(_v48, (LXValue[]){_v50}, 1), _v50}, 2)}, 1));
    }
     _v49 = px_call(px_get_global("sorted"), (LXValue[]){_v49}, 1);
     _v51 = px_list_n((LXValue[]){}, 0);
    LXValue _t56 = _v49;
    for (int _t57 = 0; _t57 < px_len(_t56); _t57++) {
        _v52 = px_index(_t56, px_int(_t57));
        (void)(px_method(_v51, "append", (LXValue[]){px_index(_v52, px_int(1LL))}, 1));
    }
    return _v51;
px_err_53:
    if (px_err_53_proped) return px_err_53_val;
    return px_null();
}

static LXValue fn_greet(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v58 = (nargs > 0) ? args[0] : px_null();
    LXValue _v59 = (nargs > 1) ? args[1] : px_int(2LL);
    LXValue _v60 = px_null();
    LXValue _v61 = px_null();
    LXValue px_err_62_val = px_null();
    int px_err_62_proped = 0;
    _v60 = px_add(px_add(px_str("hi "), px_call(px_get_global("str"), (LXValue[]){_v58}, 1)), px_str(""));
    _v61 = px_int(0LL);
    while (px_is_truthy(px_lt(_v61, _v59))) {
         _v60 = px_add(_v60, px_str("!"));
         _v61 = px_add(_v61, px_int(1LL));
    }
    return _v60;
px_err_62:
    if (px_err_62_proped) return px_err_62_val;
    return px_null();
}

static LXValue fn_process(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v63 = (nargs > 0) ? args[0] : px_null();
    LXValue _v64 = px_null();
    LXValue _v65 = px_null();
    LXValue _v66 = px_null();
    LXValue _v67 = px_null();
    LXValue _v68 = px_null();
    LXValue _v69 = px_null();
    LXValue _v70 = px_null();
    LXValue _v71 = px_null();
    LXValue _v72 = px_null();
    LXValue px_err_73_val = px_null();
    int px_err_73_proped = 0;
    _v64 = ({ LXValue _t74 = px_list(0); LXValue _t75 = _v63; for (int _t77=0; _t77<px_len(_t75); _t77++) { LXValue _t76 = px_index(_t75, px_int(_t77)); LXValue _cv78 = _t76; if (px_is_truthy(px_gt(_cv78, px_int(0LL)))) { px_list_push(_t74, px_mul(_cv78, px_int(2LL))); }  }  _t74; });
    _v65 = ({ LXValue _t79 = px_list(0); LXValue _t80 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1); for (int _t82=0; _t82<px_len(_t80); _t82++) { LXValue _t81 = px_index(_t80, px_int(_t82)); LXValue _cv83 = _t81; if (px_is_truthy(({ LXValue _t84 = px_eq(px_mod(_cv83, px_int(2LL)), px_int(1LL)); px_is_truthy(_t84) ? px_ne(_cv83, px_int(5LL)) : _t84; }))) { px_list_push(_t79, _cv83); }  }  _t79; });
    _v66 = ({ LXValue _t85 = px_dict(); LXValue _t86 = px_method(_v63, "items", (LXValue[]){}, 0); for (int _t88=0; _t88<px_len(_t86); _t88++) { LXValue _t87 = px_index(_t86, px_int(_t88)); LXValue _cv89_0 = px_index(_t87, px_int(0)); LXValue _cv90_1 = px_index(_t87, px_int(1)); { LXValue _k = px_call(px_get_global("str"), (LXValue[]){_cv89_0}, 1); LXValue _v = _cv89_0; if (_k.type == PX_STR) px_dict_set(_t85, _k.as.obj->as.str.data, _v); }  }  _t85; });
    _v67 = px_index(_v63, px_int(0LL));
    _v68 = px_index(_v63, px_neg(px_int(1LL)));
    _v69 = px_slice(_v63, px_int(1LL), px_int(3LL), px_null());
    _v70 = px_slice(_v63, px_null(), px_null(), px_null());
    _v71 = px_slice(_v63, px_null(), px_null(), px_neg(px_int(1LL)));
    _v72 = px_slice(_v63, px_null(), px_null(), px_int(2LL));
    return px_list_n((LXValue[]){_v64, _v65, _v66, _v67, _v68, _v69, _v70, _v71, _v72}, 9);
px_err_73:
    if (px_err_73_proped) return px_err_73_val;
    return px_null();
}

static LXValue fn_use_ops(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v91 = (nargs > 0) ? args[0] : px_null();
    LXValue _v92 = (nargs > 1) ? args[1] : px_null();
    LXValue _v93 = px_null();
    LXValue _v94 = px_null();
    LXValue _v95 = px_null();
    LXValue _v96 = px_null();
    LXValue _v97 = px_null();
    LXValue _v98 = px_null();
    LXValue px_err_99_val = px_null();
    int px_err_99_proped = 0;
    _v93 = px_call(px_get_global("double"), (LXValue[]){_v91}, 1);
    _v94 = ({ LXValue _t100 = _v91; px_is_null(_t100) ? px_int(42LL) : _t100; });
    _v95 = ({ LXValue _t101 = _v91; px_is_null(_t101) ? px_null() : px_field(_v91, "name"); });
    _v96 = ({ LXValue _t102 = _v91; if (px_is_result(_t102)) { if (!px_result_ok(_t102)) px_error("force unwrap Err"); _t102 = px_result_unwrap(_t102); } if (px_is_null(_t102)) px_error("force unwrap null"); _t102; });
    _v97 = px_field(px_field(({ LXValue _t103 = _v91; if (px_is_result(_t103)) { if (!px_result_ok(_t103)) px_error("force unwrap Err"); _t103 = px_result_unwrap(_t103); } if (px_is_null(_t103)) px_error("force unwrap null"); _t103; }), "b"), "c");
    _v98 = px_index(px_index(_v91, px_int(0LL)), px_int(1LL));
    return px_add(px_add(px_add(px_add(px_add(_v93, _v94), px_call(px_get_global("len"), (LXValue[]){_v95}, 1)), _v96), _v97), _v98);
px_err_99:
    if (px_err_99_proped) return px_err_99_val;
    return px_null();
}

static LXValue fn_double(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v104 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_105_val = px_null();
    int px_err_105_proped = 0;
    return px_mul(_v104, px_int(2LL));
px_err_105:
    if (px_err_105_proped) return px_err_105_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v106 = px_null();
    LXValue _v107 = px_null();
    LXValue _v108 = px_null();
    LXValue _v109 = px_null();
    LXValue _v110 = px_null();
    LXValue _v111 = px_null();
    LXValue px_err_112_val = px_null();
    int px_err_112_proped = 0;
    _v106 = px_list_n((LXValue[]){px_enum("Shape", "Circle"), px_enum("Shape", "Square")}, 2);
    _v107 = px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_int(3LL), px_int(4LL)}, 2);
    _v108 = ({ LXValue _t113 = px_index(_v106, px_int(0LL)); if ((_t113.type == PX_ENUM && strcmp(_t113.as.obj->as.enum_inst.variant, "Circle") == 0)) { _t113 = ({ LXValue _blk = px_null(); _blk = px_str("circle"); _blk; }); } else if ((_t113.type == PX_ENUM && strcmp(_t113.as.obj->as.enum_inst.variant, "Square") == 0)) { _t113 = ({ LXValue _blk = px_null(); _blk = px_str("square"); _blk; }); } _t113; });
    (void)(px_call(px_get_global("print"), (LXValue[]){_v108}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){fn_Point_area((LXValue[]){_v107}, 1, NULL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px")}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px"), px_int(3LL)}, 2)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("process"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_neg(px_int(2LL)), px_int(3LL), px_int(4LL)}, 4)}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("use_ops"), (LXValue[]){px_int(5LL), px_int(6LL)}, 2)}, 1));
    _v109 = px_func("<closure1>", fn_closure_1, NULL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v109, (LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 1));
    _v110 = px_func("<closure2>", fn_closure_2, NULL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v110, (LXValue[]){px_int(9LL)}, 1)}, 1));
    _v111 = px_gen_lazy(px_call(px_get_global("range"), (LXValue[]){px_int(4LL)}, 1), px_func("<closure3>", fn_closure_3, NULL), px_null());
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("list"), (LXValue[]){_v111}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(31LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(10LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_float(250)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_neg(px_int(7LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_not(px_bool(true))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_idiv(px_int(5LL), px_int(2LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_pow(px_int(2LL), px_int(8LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shl(px_int(1LL), px_int(4LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shr(px_int(255LL), px_int(2LL))}, 1));
px_err_112:
    if (px_err_112_proped) return px_err_112_val;
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
