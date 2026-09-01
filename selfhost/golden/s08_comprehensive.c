#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v113 = (nargs > 0) ? args[0] : px_null();
    LXValue _v114 = (nargs > 1) ? args[1] : px_null();
    return ({ LXValue _blk = px_null(); _blk = px_add(_v113, _v114); _blk; });
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v116 = (nargs > 0) ? args[0] : px_null();
    return px_mul(_v116, _v116);
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
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    LXValue _t5 = _v2;
    for (int _t6 = 0; _t6 < px_len(_t5); _t6++) {
        LXValue _v7 = px_index(_t5, px_int(_t6));
        (void)(px_call(_v3, (LXValue[]){_v7}, 1));
    }
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

static LXValue fn_unique(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = px_null();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
     _v9 = px_list_n((LXValue[]){}, 0);
    LXValue _t11 = _v8;
    for (int _t12 = 0; _t12 < px_len(_t11); _t12++) {
        LXValue _v13 = px_index(_t11, px_int(_t12));
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v9, _v13}, 2)))) {
            (void)(px_method(_v9, "append", (LXValue[]){_v13}, 1));
        }
    }
    return _v9;
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_flatten(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue _v15 = px_null();
    LXValue px_err_16_val = px_null();
    int px_err_16_proped = 0;
     _v15 = px_list_n((LXValue[]){}, 0);
    LXValue _t17 = _v14;
    for (int _t18 = 0; _t18 < px_len(_t17); _t18++) {
        LXValue _v19 = px_index(_t17, px_int(_t18));
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v19}, 1), px_str("list")))) {
            LXValue _t20 = _v19;
            for (int _t21 = 0; _t21 < px_len(_t20); _t21++) {
                LXValue _v22 = px_index(_t20, px_int(_t21));
                (void)(px_method(_v15, "append", (LXValue[]){_v22}, 1));
            }
        }
        else {
            (void)(px_method(_v15, "append", (LXValue[]){_v19}, 1));
        }
    }
    return _v15;
px_err_16:
    if (px_err_16_proped) return px_err_16_val;
    return px_null();
}

static LXValue fn_zip_lists(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v23 = (nargs > 0) ? args[0] : px_null();
    LXValue _v24 = (nargs > 1) ? args[1] : px_null();
    LXValue _v25 = px_null();
    LXValue _v26 = px_null();
    LXValue px_err_27_val = px_null();
    int px_err_27_proped = 0;
     _v25 = px_list_n((LXValue[]){}, 0);
     _v26 = px_call(px_get_global("min"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v23}, 1), px_call(px_get_global("len"), (LXValue[]){_v24}, 1)}, 2);
    LXValue _t28 = px_call(px_get_global("range"), (LXValue[]){_v26}, 1);
    for (int _t29 = 0; _t29 < px_len(_t28); _t29++) {
        LXValue _v30 = px_index(_t28, px_int(_t29));
        (void)(px_method(_v25, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v23, _v30), px_index(_v24, _v30)}, 2)}, 1));
    }
    return _v25;
px_err_27:
    if (px_err_27_proped) return px_err_27_val;
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
    LXValue px_err_43_val = px_null();
    int px_err_43_proped = 0;
     _v41 = ({ LXValue _blk = px_null(); _blk; });
    LXValue _t44 = _v39;
    for (int _t45 = 0; _t45 < px_len(_t44); _t45++) {
        LXValue _v46 = px_index(_t44, px_int(_t45));
         _v42 = px_call(px_get_global("str"), (LXValue[]){px_call(_v40, (LXValue[]){_v46}, 1)}, 1);
        if (px_is_truthy(px_method(_v41, "has", (LXValue[]){_v42}, 1))) {
            (void)(px_method(px_index(_v41, _v42), "append", (LXValue[]){_v46}, 1));
        }
        else {
            px_index_set(_v41, _v42, px_list_n((LXValue[]){_v46}, 1));
        }
    }
    return _v41;
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

static LXValue fn_greet(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v58 = (nargs > 0) ? args[0] : px_null();
    LXValue _v59 = (nargs > 1) ? args[1] : px_int(2LL);
    LXValue _v60 = px_null();
    LXValue _v61 = px_null();
    LXValue px_err_62_val = px_null();
    int px_err_62_proped = 0;
    LXValue _v63 = px_add(px_add(px_str("hi "), px_call(px_get_global("str"), (LXValue[]){_v58}, 1)), px_str(""));
    LXValue _v64 = px_int(0LL);
    while (px_is_truthy(px_lt(_v64, _v59))) {
         _v63 = px_add(_v63, px_str("!"));
         _v64 = px_add(_v64, px_int(1LL));
    }
    return _v63;
px_err_62:
    if (px_err_62_proped) return px_err_62_val;
    return px_null();
}

static LXValue fn_process(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v65 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_66_val = px_null();
    int px_err_66_proped = 0;
    LXValue _v72 = ({ LXValue _t67 = px_list(0); LXValue _t68 = _v65; for (int _t70=0; _t70<px_len(_t68); _t70++) { LXValue _t69 = px_index(_t68, px_int(_t70)); LXValue _cv71 = _t69; if (px_is_truthy(px_gt(_cv71, px_int(0LL)))) { px_list_push(_t67, px_mul(_cv71, px_int(2LL))); }  }  _t67; });
    LXValue _v79 = ({ LXValue _t73 = px_list(0); LXValue _t74 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1); for (int _t76=0; _t76<px_len(_t74); _t76++) { LXValue _t75 = px_index(_t74, px_int(_t76)); LXValue _cv77 = _t75; if (px_is_truthy(({ LXValue _t78 = px_eq(px_mod(_cv77, px_int(2LL)), px_int(1LL)); px_is_truthy(_t78) ? px_ne(_cv77, px_int(5LL)) : _t78; }))) { px_list_push(_t73, _cv77); }  }  _t73; });
    LXValue _v86 = ({ LXValue _t80 = px_dict(); LXValue _t81 = px_method(_v65, "items", (LXValue[]){}, 0); for (int _t83=0; _t83<px_len(_t81); _t83++) { LXValue _t82 = px_index(_t81, px_int(_t83)); LXValue _cv84_0 = px_index(_t82, px_int(0)); LXValue _cv85_1 = px_index(_t82, px_int(1)); { LXValue _k = px_call(px_get_global("str"), (LXValue[]){_cv84_0}, 1); LXValue _v = _cv84_0; if (_k.type == PX_STR) px_dict_set(_t80, _k.as.obj->as.str.data, _v); }  }  _t80; });
    LXValue _v87 = px_index(_v65, px_int(0LL));
    LXValue _v88 = px_index(_v65, px_neg(px_int(1LL)));
    LXValue _v89 = px_slice(_v65, px_int(1LL), px_int(3LL), px_null());
    LXValue _v90 = px_slice(_v65, px_null(), px_null(), px_null());
    LXValue _v91 = px_slice(_v65, px_null(), px_null(), px_neg(px_int(1LL)));
    LXValue _v92 = px_slice(_v65, px_null(), px_null(), px_int(2LL));
    return px_list_n((LXValue[]){_v72, _v79, _v86, _v87, _v88, _v89, _v90, _v91, _v92}, 9);
px_err_66:
    if (px_err_66_proped) return px_err_66_val;
    return px_null();
}

static LXValue fn_use_ops(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v93 = (nargs > 0) ? args[0] : px_null();
    LXValue _v94 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_95_val = px_null();
    int px_err_95_proped = 0;
    LXValue _v96 = px_call(px_get_global("double"), (LXValue[]){_v93}, 1);
    LXValue _v98 = ({ LXValue _t97 = _v93; px_is_null(_t97) ? px_int(42LL) : _t97; });
    LXValue _v100 = ({ LXValue _t99 = _v93; px_is_null(_t99) ? px_null() : px_field(_v93, "name"); });
    LXValue _v102 = ({ LXValue _t101 = _v93; if (px_is_result(_t101)) { if (!px_result_ok(_t101)) px_error("force unwrap Err"); _t101 = px_result_unwrap(_t101); } if (px_is_null(_t101)) px_error("force unwrap null"); _t101; });
    LXValue _v104 = px_field(px_field(({ LXValue _t103 = _v93; if (px_is_result(_t103)) { if (!px_result_ok(_t103)) px_error("force unwrap Err"); _t103 = px_result_unwrap(_t103); } if (px_is_null(_t103)) px_error("force unwrap null"); _t103; }), "b"), "c");
    LXValue _v105 = px_index(px_index(_v93, px_int(0LL)), px_int(1LL));
    return px_add(px_add(px_add(px_add(px_add(_v96, _v98), px_call(px_get_global("len"), (LXValue[]){_v100}, 1)), _v102), _v104), _v105);
px_err_95:
    if (px_err_95_proped) return px_err_95_val;
    return px_null();
}

static LXValue fn_double(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v106 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_107_val = px_null();
    int px_err_107_proped = 0;
    return px_mul(_v106, px_int(2LL));
px_err_107:
    if (px_err_107_proped) return px_err_107_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_108_val = px_null();
    int px_err_108_proped = 0;
    LXValue _v109 = px_list_n((LXValue[]){px_enum("Shape", "Circle"), px_enum("Shape", "Square")}, 2);
    LXValue _v110 = px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_int(3LL), px_int(4LL)}, 2);
    LXValue _v112 = ({ LXValue _t111 = px_index(_v109, px_int(0LL)); if ((_t111.type == PX_ENUM && strcmp(_t111.as.obj->as.enum_inst.variant, "Circle") == 0)) { _t111 = ({ LXValue _blk = px_null(); _blk = px_str("circle"); _blk; }); } else if ((_t111.type == PX_ENUM && strcmp(_t111.as.obj->as.enum_inst.variant, "Square") == 0)) { _t111 = ({ LXValue _blk = px_null(); _blk = px_str("square"); _blk; }); } _t111; });
    (void)(px_call(px_get_global("print"), (LXValue[]){_v112}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){fn_Point_area((LXValue[]){_v110}, 1, NULL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px")}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px"), px_int(3LL)}, 2)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("process"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_neg(px_int(2LL)), px_int(3LL), px_int(4LL)}, 4)}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("use_ops"), (LXValue[]){px_int(5LL), px_int(6LL)}, 2)}, 1));
    LXValue _v115 = px_func("<closure1>", fn_closure_1, NULL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v115, (LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 1));
    LXValue _v117 = px_func("<closure2>", fn_closure_2, NULL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v117, (LXValue[]){px_int(9LL)}, 1)}, 1));
    LXValue _v120 = px_gen_lazy(px_call(px_get_global("range"), (LXValue[]){px_int(4LL)}, 1), px_func("<closure3>", fn_closure_3, NULL), px_null());
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("list"), (LXValue[]){_v120}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(31LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(10LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_float(250)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_neg(px_int(7LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_not(px_bool(true))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_idiv(px_int(5LL), px_int(2LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_pow(px_int(2LL), px_int(8LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shl(px_int(1LL), px_int(4LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shr(px_int(255LL), px_int(2LL))}, 1));
px_err_108:
    if (px_err_108_proped) return px_err_108_val;
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
