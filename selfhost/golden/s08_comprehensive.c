#include "runtime.h"
#include <string.h>
#include <stdio.h>

static LXValue fn_closure_1(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v117 = (nargs > 0) ? args[0] : px_null();
    LXValue _v118 = (nargs > 1) ? args[1] : px_null();
    LXValue px_cerr_1_val = px_null();
    int px_cerr_1_proped = 0;
    return ({ LXValue _blk = px_null(); _blk = px_add(_v117, _v118); _blk; });
px_cerr_1:
    if (px_cerr_1_proped) return px_cerr_1_val;
    return px_null();
}
static LXValue fn_closure_2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v119 = (nargs > 0) ? args[0] : px_null();
    LXValue px_cerr_2_val = px_null();
    int px_cerr_2_proped = 0;
    return px_mul(_v119, _v119);
px_cerr_2:
    if (px_cerr_2_proped) return px_cerr_2_val;
    return px_null();
}
static LXValue fn_closure_3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v120 = (nargs > 0) ? args[0] : px_null();
    LXValue px_cerr_3_val = px_null();
    int px_cerr_3_proped = 0;
    return px_mul(_v120, _v120);
px_cerr_3:
    if (px_cerr_3_proped) return px_cerr_3_val;
    return px_null();
}

static LXValue fn_Point_area(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("area");
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    px_srcline(20);
    return px_float(1.5);
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
    return px_null();
}

static LXValue fn_each(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("each");
    LXValue _v2 = (nargs > 0) ? args[0] : px_null();
    LXValue _v3 = (nargs > 1) ? args[1] : px_null();
    LXValue _v4 = px_uninit();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(10);
    LXValue _t6 = _v2;
    int _il1 = px_iter_prepare(_t6);
    for (int _t7 = 0; _t7 < _il1; _t7++) {
        px_iter_ck(_t6, _il1);
        _v4 = px_iter_at(_t6, px_int(_t7));
        px_srcline(11);
        (void)(px_call(_v3, (LXValue[]){_v4}, 1));
    }
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_unique(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("unique");
    LXValue _v8 = (nargs > 0) ? args[0] : px_null();
    LXValue _v9 = px_uninit();
    LXValue _v10 = px_uninit();
    LXValue px_err_11_val = px_null();
    int px_err_11_proped = 0;
    px_srcline(14);
     _v9 = px_list_n((LXValue[]){}, 0);
    px_srcline(15);
    LXValue _t12 = _v8;
    int _il2 = px_iter_prepare(_t12);
    for (int _t13 = 0; _t13 < _il2; _t13++) {
        px_iter_ck(_t12, _il2);
        _v10 = px_iter_at(_t12, px_int(_t13));
        px_srcline(16);
        if (px_is_truthy(px_not(px_call(px_get_global("contains"), (LXValue[]){_v9, _v10}, 2)))) {
            px_srcline(17);
            (void)(px_method(_v9, "append", (LXValue[]){_v10}, 1));
        }
    }
    px_srcline(18);
    return _v9;
px_err_11:
    if (px_err_11_proped) return px_err_11_val;
    return px_null();
}

static LXValue fn_flatten(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("flatten");
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue _v15 = px_uninit();
    LXValue _v16 = px_uninit();
    LXValue _v17 = px_uninit();
    LXValue px_err_18_val = px_null();
    int px_err_18_proped = 0;
    px_srcline(21);
     _v15 = px_list_n((LXValue[]){}, 0);
    px_srcline(22);
    LXValue _t19 = _v14;
    int _il3 = px_iter_prepare(_t19);
    for (int _t20 = 0; _t20 < _il3; _t20++) {
        px_iter_ck(_t19, _il3);
        _v16 = px_iter_at(_t19, px_int(_t20));
        px_srcline(23);
        if (px_is_truthy(px_eq(px_call(px_get_global("type"), (LXValue[]){_v16}, 1), px_str("list")))) {
            px_srcline(24);
            LXValue _t21 = _v16;
            int _il4 = px_iter_prepare(_t21);
            for (int _t22 = 0; _t22 < _il4; _t22++) {
                px_iter_ck(_t21, _il4);
                _v17 = px_iter_at(_t21, px_int(_t22));
                px_srcline(25);
                (void)(px_method(_v15, "append", (LXValue[]){_v17}, 1));
            }
        }
        else {
            px_srcline(27);
            (void)(px_method(_v15, "append", (LXValue[]){_v16}, 1));
        }
    }
    px_srcline(28);
    return _v15;
px_err_18:
    if (px_err_18_proped) return px_err_18_val;
    return px_null();
}

static LXValue fn_zip_lists(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("zip_lists");
    LXValue _v23 = (nargs > 0) ? args[0] : px_null();
    LXValue _v24 = (nargs > 1) ? args[1] : px_null();
    LXValue _v25 = px_uninit();
    LXValue _v26 = px_uninit();
    LXValue _v27 = px_uninit();
    LXValue px_err_28_val = px_null();
    int px_err_28_proped = 0;
    px_srcline(31);
     _v25 = px_list_n((LXValue[]){}, 0);
    px_srcline(32);
     _v26 = ({ LXValue _s1 = px_call(px_get_global("len"), (LXValue[]){_v23}, 1); LXValue _s2 = px_call(px_get_global("len"), (LXValue[]){_v24}, 1); px_call(px_get_global("min"), (LXValue[]){_s1, _s2}, 2); });
    px_srcline(33);
    LXValue _t29 = px_call(px_get_global("range"), (LXValue[]){_v26}, 1);
    int _il5 = px_iter_prepare(_t29);
    for (int _t30 = 0; _t30 < _il5; _t30++) {
        px_iter_ck(_t29, _il5);
        _v27 = px_iter_at(_t29, px_int(_t30));
        px_srcline(34);
        (void)(px_method(_v25, "append", (LXValue[]){px_list_n((LXValue[]){px_index(_v23, _v27), px_index(_v24, _v27)}, 2)}, 1));
    }
    px_srcline(35);
    return _v25;
px_err_28:
    if (px_err_28_proped) return px_err_28_val;
    return px_null();
}

static LXValue fn_chunk(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("chunk");
    LXValue _v31 = (nargs > 0) ? args[0] : px_null();
    LXValue _v32 = (nargs > 1) ? args[1] : px_null();
    LXValue _v33 = px_uninit();
    LXValue _v34 = px_uninit();
    LXValue _v35 = px_uninit();
    LXValue _v36 = px_uninit();
    LXValue _v37 = px_uninit();
    LXValue _v38 = px_uninit();
    LXValue px_err_39_val = px_null();
    int px_err_39_proped = 0;
    px_srcline(47);
    if (px_is_truthy(px_ne(px_call(px_get_global("type"), (LXValue[]){_v32}, 1), px_str("int")))) {
        px_srcline(48);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("chunk 的 n 需要 int，实际是 "), px_call(px_get_global("type"), (LXValue[]){_v32}, 1))}, 1));
    }
    px_srcline(49);
    if (px_is_truthy(px_le(_v32, px_int(0LL)))) {
        px_srcline(50);
        (void)(px_call(px_get_global("panic"), (LXValue[]){px_add(px_str("chunk 的 n 需要正整数，实际是 "), px_call(px_get_global("str"), (LXValue[]){_v32}, 1))}, 1));
    }
    px_srcline(51);
     _v33 = px_list_n((LXValue[]){}, 0);
    px_srcline(52);
    _v34 = px_call(px_get_global("len"), (LXValue[]){_v31}, 1);
    px_srcline(53);
    _v35 = px_int(0LL);
    px_srcline(54);
    while (px_is_truthy(px_lt(_v35, _v34))) {
        px_srcline(55);
        _v36 = _v32;
        px_srcline(56);
        if (px_is_truthy(px_gt(px_add(_v35, _v36), _v34))) {
            px_srcline(57);
             _v36 = px_sub(_v34, _v35);
        }
        px_srcline(58);
        _v37 = px_list_n((LXValue[]){}, 0);
        px_srcline(59);
        _v38 = px_int(0LL);
        px_srcline(60);
        while (px_is_truthy(px_lt(_v38, _v36))) {
            px_srcline(61);
            (void)(px_method(_v37, "append", (LXValue[]){px_index(_v31, px_add(_v35, _v38))}, 1));
            px_srcline(62);
             _v38 = px_add(_v38, px_int(1LL));
        }
        px_srcline(63);
         _v35 = px_add(_v35, _v36);
        px_srcline(64);
        (void)(px_method(_v33, "append", (LXValue[]){_v37}, 1));
    }
    px_srcline(65);
    return _v33;
px_err_39:
    if (px_err_39_proped) return px_err_39_val;
    return px_null();
}

static LXValue fn_group_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("group_by");
    LXValue _v40 = (nargs > 0) ? args[0] : px_null();
    LXValue _v41 = (nargs > 1) ? args[1] : px_null();
    LXValue _v42 = px_uninit();
    LXValue _v43 = px_uninit();
    LXValue _v44 = px_uninit();
    LXValue _v45 = px_uninit();
    LXValue px_err_46_val = px_null();
    int px_err_46_proped = 0;
    px_srcline(69);
     _v42 = px_call(px_get_global("json_parse"), (LXValue[]){px_str("{}")}, 1);
    px_srcline(70);
    LXValue _t47 = _v40;
    int _il6 = px_iter_prepare(_t47);
    for (int _t48 = 0; _t48 < _il6; _t48++) {
        px_iter_ck(_t47, _il6);
        _v43 = px_iter_at(_t47, px_int(_t48));
        px_srcline(71);
         _v44 = px_call(px_get_global("str"), (LXValue[]){px_call(_v41, (LXValue[]){_v43}, 1)}, 1);
        px_srcline(72);
        if (px_is_truthy(px_method(_v42, "has", (LXValue[]){_v44}, 1))) {
            px_srcline(73);
            _v45 = px_index(_v42, _v44);
            px_srcline(74);
            (void)(px_method(_v45, "append", (LXValue[]){_v43}, 1));
            px_srcline(75);
            (void)(px_method(_v42, "set", (LXValue[]){_v44, _v45}, 2));
        }
        else {
            px_srcline(77);
            (void)(px_method(_v42, "set", (LXValue[]){_v44, px_list_n((LXValue[]){_v43}, 1)}, 2));
        }
    }
    px_srcline(78);
    return _v42;
px_err_46:
    if (px_err_46_proped) return px_err_46_val;
    return px_null();
}

static LXValue fn_sort_by(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("sort_by");
    LXValue _v49 = (nargs > 0) ? args[0] : px_null();
    LXValue _v50 = (nargs > 1) ? args[1] : px_null();
    LXValue _v51 = px_uninit();
    LXValue _v52 = px_uninit();
    LXValue _v53 = px_uninit();
    LXValue _v54 = px_uninit();
    LXValue px_err_55_val = px_null();
    int px_err_55_proped = 0;
    px_srcline(81);
     _v51 = px_list_n((LXValue[]){}, 0);
    px_srcline(82);
    LXValue _t56 = _v49;
    int _il7 = px_iter_prepare(_t56);
    for (int _t57 = 0; _t57 < _il7; _t57++) {
        px_iter_ck(_t56, _il7);
        _v52 = px_iter_at(_t56, px_int(_t57));
        px_srcline(83);
        (void)(px_method(_v51, "append", (LXValue[]){px_list_n((LXValue[]){px_call(_v50, (LXValue[]){_v52}, 1), _v52}, 2)}, 1));
    }
    px_srcline(84);
     _v51 = px_call(px_get_global("sorted"), (LXValue[]){_v51}, 1);
    px_srcline(85);
     _v53 = px_list_n((LXValue[]){}, 0);
    px_srcline(86);
    LXValue _t58 = _v51;
    int _il8 = px_iter_prepare(_t58);
    for (int _t59 = 0; _t59 < _il8; _t59++) {
        px_iter_ck(_t58, _il8);
        _v54 = px_iter_at(_t58, px_int(_t59));
        px_srcline(87);
        (void)(px_method(_v53, "append", (LXValue[]){px_index(_v54, px_int(1LL))}, 1));
    }
    px_srcline(88);
    return _v53;
px_err_55:
    if (px_err_55_proped) return px_err_55_val;
    return px_null();
}

static LXValue fn_greet(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("greet");
    LXValue _v60 = (nargs > 0) ? args[0] : px_null();
    LXValue _v61 = (nargs > 1) ? args[1] : px_int(2LL);
    LXValue _v62 = px_uninit();
    LXValue _v63 = px_uninit();
    LXValue px_err_64_val = px_null();
    int px_err_64_proped = 0;
    px_srcline(23);
    _v62 = px_add(px_add(px_str("hi "), px_call(px_get_global("str"), (LXValue[]){_v60}, 1)), px_str(""));
    px_srcline(24);
    _v63 = px_int(0LL);
    px_srcline(25);
    while (px_is_truthy(px_lt(_v63, _v61))) {
        px_srcline(26);
         _v62 = px_add(_v62, px_str("!"));
        px_srcline(27);
         _v63 = px_add(_v63, px_int(1LL));
    }
    px_srcline(28);
    return _v62;
px_err_64:
    if (px_err_64_proped) return px_err_64_val;
    return px_null();
}

static LXValue fn_process(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("process");
    LXValue _v65 = (nargs > 0) ? args[0] : px_null();
    LXValue _v66 = px_uninit();
    LXValue _v67 = px_uninit();
    LXValue _v68 = px_uninit();
    LXValue _v69 = px_uninit();
    LXValue _v70 = px_uninit();
    LXValue _v71 = px_uninit();
    LXValue _v72 = px_uninit();
    LXValue _v73 = px_uninit();
    LXValue _v74 = px_uninit();
    LXValue px_err_75_val = px_null();
    int px_err_75_proped = 0;
    px_srcline(31);
    _v66 = ({ LXValue _t76 = px_list(0); LXValue _t77 = _v65; for (int _il9 = px_iter_prepare(_t77), _t79=0; _t79<_il9; _t79++) { px_iter_ck(_t77, _il9); LXValue _t78 = px_iter_at(_t77, px_int(_t79)); LXValue _cv80 = _t78; if (px_is_truthy(px_gt(px_chk_uninit(_cv80, "x"), px_int(0LL)))) { px_list_push(_t76, px_mul(px_chk_uninit(_cv80, "x"), px_int(2LL))); }  }  _t76; });
    px_srcline(32);
    _v67 = ({ LXValue _t81 = px_list(0); LXValue _t82 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1); for (int _il10 = px_iter_prepare(_t82), _t84=0; _t84<_il10; _t84++) { px_iter_ck(_t82, _il10); LXValue _t83 = px_iter_at(_t82, px_int(_t84)); LXValue _cv85 = _t83; if (px_is_truthy(({ LXValue _t86 = px_eq(px_mod(px_chk_uninit(_cv85, "x"), px_int(2LL)), px_int(1LL)); px_is_truthy(_t86) ? px_ne(px_chk_uninit(_cv85, "x"), px_int(5LL)) : _t86; }))) { px_list_push(_t81, px_chk_uninit(_cv85, "x")); }  }  _t81; });
    px_srcline(33);
    _v68 = ({ LXValue _t87 = px_dict(); LXValue _t88 = px_method(_v65, "items", (LXValue[]){}, 0); for (int _il11 = px_iter_prepare(_t88), _t90=0; _t90<_il11; _t90++) { px_iter_ck(_t88, _il11); LXValue _t89 = px_iter_at(_t88, px_int(_t90)); px_unpack_ck(_t89, 2); LXValue _cv91_0 = px_index(_t89, px_int(0)); LXValue _cv92_1 = px_index(_t89, px_int(1)); { LXValue _k = px_call(px_get_global("str"), (LXValue[]){px_chk_uninit(_cv91_0, "k")}, 1); LXValue _v = px_chk_uninit(_cv91_0, "k"); px_dict_set_checked(_t87, _k, _v); }  }  _t87; });
    px_srcline(34);
    _v69 = px_index(_v65, px_int(0LL));
    px_srcline(35);
    _v70 = px_index(_v65, px_neg(px_int(1LL)));
    px_srcline(36);
    _v71 = px_slice(_v65, px_int(1LL), px_int(3LL), px_null());
    px_srcline(37);
    _v72 = px_slice(_v65, px_null(), px_null(), px_null());
    px_srcline(38);
    _v73 = px_slice(_v65, px_null(), px_null(), px_neg(px_int(1LL)));
    px_srcline(39);
    _v74 = px_slice(_v65, px_null(), px_null(), px_int(2LL));
    px_srcline(40);
    return px_list_n((LXValue[]){_v66, _v67, _v68, _v69, _v70, _v71, _v72, _v73, _v74}, 9);
px_err_75:
    if (px_err_75_proped) return px_err_75_val;
    return px_null();
}

static LXValue fn_use_ops(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("use_ops");
    LXValue _v93 = (nargs > 0) ? args[0] : px_null();
    LXValue _v94 = (nargs > 1) ? args[1] : px_null();
    LXValue _v95 = px_uninit();
    LXValue _v96 = px_uninit();
    LXValue _v97 = px_uninit();
    LXValue _v98 = px_uninit();
    LXValue _v99 = px_uninit();
    LXValue _v100 = px_uninit();
    LXValue px_err_101_val = px_null();
    int px_err_101_proped = 0;
    px_srcline(43);
    _v95 = px_call(px_get_global("double"), (LXValue[]){_v93}, 1);
    px_srcline(44);
    _v96 = ({ LXValue _t102 = _v93; px_is_null(_t102) ? px_int(42LL) : _t102; });
    px_srcline(45);
    _v97 = ({ LXValue _t103 = _v93; px_is_null(_t103) ? px_null() : px_field(_t103, "name"); });
    px_srcline(46);
    _v98 = ({ LXValue _t104 = _v93; if (px_is_result(_t104)) { if (!px_result_ok(_t104)) px_error("R1004: 强制解包 !: 值为 Err(%s)", px_to_string(px_result_unwrap(_t104))); _t104 = px_result_unwrap(_t104); } if (px_is_null(_t104)) px_error("R1004: 强制解包 !: 值为 null"); _t104; });
    px_srcline(47);
    _v99 = px_field(px_field(({ LXValue _t105 = _v93; if (px_is_result(_t105)) { if (!px_result_ok(_t105)) px_error("R1004: 强制解包 !: 值为 Err(%s)", px_to_string(px_result_unwrap(_t105))); _t105 = px_result_unwrap(_t105); } if (px_is_null(_t105)) px_error("R1004: 强制解包 !: 值为 null"); _t105; }), "b"), "c");
    px_srcline(48);
    _v100 = px_index(px_index(_v93, px_int(0LL)), px_int(1LL));
    px_srcline(49);
    return px_add(px_add(px_add(px_add(px_add(_v95, _v96), px_call(px_get_global("len"), (LXValue[]){_v97}, 1)), _v98), _v99), _v100);
px_err_101:
    if (px_err_101_proped) return px_err_101_val;
    return px_null();
}

static LXValue fn_double(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("double");
    LXValue _v106 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_107_val = px_null();
    int px_err_107_proped = 0;
    px_srcline(52);
    return px_mul(_v106, px_int(2LL));
px_err_107:
    if (px_err_107_proped) return px_err_107_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v108 = px_uninit();
    LXValue _v109 = px_uninit();
    LXValue _v110 = px_uninit();
    LXValue _v111 = px_uninit();
    LXValue _v112 = px_uninit();
    LXValue _v113 = px_uninit();
    LXValue px_err_114_val = px_null();
    int px_err_114_proped = 0;
    px_srcline(55);
    _v108 = px_list_n((LXValue[]){px_enum_checked("Shape", "Circle", 1), px_enum_checked("Shape", "Square", 1)}, 2);
    px_srcline(56);
    _v109 = px_struct("Point", (char*[]){"x", "y"}, (LXValue[]){px_int(3LL), px_int(4LL)}, 2);
    px_srcline(57);
    _v110 = ({ LXValue _t115 = px_index(_v108, px_int(0LL)); LXValue _t116 = px_null(); if ((_t115.type == PX_ENUM && strcmp(_t115.as.obj->as.enum_inst.variant, "Circle") == 0)) { _t116 = ({ LXValue _blk = px_null(); _blk = px_str("circle"); _blk; }); } else if ((_t115.type == PX_ENUM && strcmp(_t115.as.obj->as.enum_inst.variant, "Square") == 0)) { _t116 = ({ LXValue _blk = px_null(); _blk = px_str("square"); _blk; }); } else { px_match_fail(_t115); } _t116; });
    px_srcline(62);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v110}, 1));
    px_srcline(63);
    (void)(px_call(px_get_global("print"), (LXValue[]){fn_Point_area((LXValue[]){_v109}, 1, NULL)}, 1));
    px_srcline(64);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px")}, 1)}, 1));
    px_srcline(65);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("greet"), (LXValue[]){px_str("px"), px_int(3LL)}, 2)}, 1));
    px_srcline(66);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("process"), (LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_neg(px_int(2LL)), px_int(3LL), px_int(4LL)}, 4)}, 1)}, 1));
    px_srcline(67);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("use_ops"), (LXValue[]){px_int(5LL), px_int(6LL)}, 2)}, 1));
    px_srcline(68);
    _v111 = px_func("<closure1>", fn_closure_1, NULL);
    px_srcline(69);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v111, (LXValue[]){px_int(1LL), px_int(2LL)}, 2)}, 1));
    px_srcline(70);
    _v112 = px_func("<closure2>", fn_closure_2, NULL);
    px_srcline(71);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(_v112, (LXValue[]){px_int(9LL)}, 1)}, 1));
    px_srcline(72);
    _v113 = px_gen_lazy(px_call(px_get_global("range"), (LXValue[]){px_int(4LL)}, 1), px_func("<closure3>", fn_closure_3, NULL), px_null());
    px_srcline(73);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("list"), (LXValue[]){_v113}, 1)}, 1));
    px_srcline(74);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(31LL)}, 1));
    px_srcline(75);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_int(10LL)}, 1));
    px_srcline(76);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_float(250)}, 1));
    px_srcline(77);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_neg(px_int(7LL))}, 1));
    px_srcline(78);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_not(px_bool(true))}, 1));
    px_srcline(79);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_idiv(px_int(5LL), px_int(2LL))}, 1));
    px_srcline(80);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_pow(px_int(2LL), px_int(8LL))}, 1));
    px_srcline(81);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shl(px_int(1LL), px_int(4LL))}, 1));
    px_srcline(82);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_shr(px_int(255LL), px_int(2LL))}, 1));
px_err_114:
    if (px_err_114_proped) return px_err_114_val;
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
    px_srcline(5);
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
      return px_exit_code_final(_code);   // M120（qg-issue 76 E1）
    }
}
