#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_module_search_paths(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    LXValue _v3 = px_list_n((LXValue[]){_v1}, 1);
    LXValue _v4 = px_add(_v1, px_str("/.px_modules"));
    if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v4}, 1))) {
        (void)(px_method(_v3, "append", (LXValue[]){_v4}, 1));
        LXValue _v5 = px_call(px_get_global("list_dir"), (LXValue[]){_v4}, 1);
        LXValue _t6 = _v5;
        for (int _t7 = 0; _t7 < px_len(_t6); _t7++) {
            LXValue _v8 = px_index(_t6, px_int(_t7));
            (void)(px_method(_v3, "append", (LXValue[]){_v8}, 1));
        }
    }
    return _v3;
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_module_candidates(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v9 = (nargs > 0) ? args[0] : px_null();
    LXValue _v10 = (nargs > 1) ? args[1] : px_null();
    LXValue _v11 = (nargs > 2) ? args[2] : px_null();
    LXValue _v12 = px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    LXValue _v14 = px_list_n((LXValue[]){}, 0);
    if (px_is_truthy(({ LXValue _t16 = px_eq(px_call(px_get_global("len"), (LXValue[]){_v9}, 1), px_int(1LL)); px_is_truthy(_t16) ? ({ LXValue _t15 = px_call(px_get_global("contains"), (LXValue[]){px_index(_v9, px_int(0LL)), px_str("/")}, 2); px_is_truthy(_t15) ? _t15 : px_call(px_get_global("contains"), (LXValue[]){px_index(_v9, px_int(0LL)), px_str(".px")}, 2); }) : _t16; }))) {
        (void)(px_method(_v14, "append", (LXValue[]){px_add(px_add(_v10, px_str("/")), px_index(_v9, px_int(0LL)))}, 1));
        return _v14;
    }
    if (px_is_truthy(px_eq(px_index(_v9, px_int(0LL)), px_str("std")))) {
        LXValue _v17 = _v11;
        LXValue _t18 = px_call(px_get_global("range"), (LXValue[]){px_int(1LL), px_call(px_get_global("len"), (LXValue[]){_v9}, 1)}, 2);
        for (int _t19 = 0; _t19 < px_len(_t18); _t19++) {
            LXValue _v20 = px_index(_t18, px_int(_t19));
             _v17 = px_add(px_add(_v17, px_str("/")), px_index(_v9, _v20));
        }
        (void)(px_method(_v14, "append", (LXValue[]){px_add(_v17, px_str(".px"))}, 1));
        (void)(px_method(_v14, "append", (LXValue[]){px_add(_v17, px_str("/mod.px"))}, 1));
        return _v14;
    }
    LXValue _v21 = _v10;
    LXValue _t22 = _v9;
    for (int _t23 = 0; _t23 < px_len(_t22); _t23++) {
        LXValue _v24 = px_index(_t22, px_int(_t23));
         _v21 = px_add(px_add(_v21, px_str("/")), _v24);
    }
    (void)(px_method(_v14, "append", (LXValue[]){px_add(_v21, px_str(".px"))}, 1));
    (void)(px_method(_v14, "append", (LXValue[]){px_add(_v21, px_str("/mod.px"))}, 1));
    return _v14;
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

static LXValue fn_module_find(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v25 = (nargs > 0) ? args[0] : px_null();
    LXValue _v26 = (nargs > 1) ? args[1] : px_null();
    LXValue _v27 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_28_val = px_null();
    int px_err_28_proped = 0;
    LXValue _v29 = px_call(px_get_global("module_candidates"), (LXValue[]){_v25, _v26, _v27}, 3);
    LXValue _t30 = _v29;
    for (int _t31 = 0; _t31 < px_len(_t30); _t31++) {
        LXValue _v32 = px_index(_t30, px_int(_t31));
        if (px_is_truthy(px_call(px_get_global("exists"), (LXValue[]){_v32}, 1))) {
            return _v32;
        }
    }
    return px_null();
px_err_28:
    if (px_err_28_proped) return px_err_28_val;
    return px_null();
}

static LXValue fn_loader_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v33 = (nargs > 0) ? args[0] : px_null();
    LXValue _v34 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_35_val = px_null();
    int px_err_35_proped = 0;
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("base_dir"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v33); } { LXValue _k = px_str("stdlib"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v34); } { LXValue _k = px_str("search_paths"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_call(px_get_global("module_search_paths"), (LXValue[]){_v33}, 1)); } { LXValue _k = px_str("loaded"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; })); } { LXValue _k = px_str("stack"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){}, 0)); } _d; });
px_err_35:
    if (px_err_35_proped) return px_err_35_val;
    return px_null();
}

static LXValue fn_loader_mark_loaded(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v36 = (nargs > 0) ? args[0] : px_null();
    LXValue _v37 = (nargs > 1) ? args[1] : px_null();
    LXValue _v38 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_39_val = px_null();
    int px_err_39_proped = 0;
    LXValue _v40 = px_index(_v36, px_str("loaded"));
    if (px_is_truthy(px_method(_v40, "has", (LXValue[]){px_str("_")}, 1))) {
        (void)(px_method(_v40, "remove", (LXValue[]){px_str("_")}, 1));
    }
    px_index_set(_v40, _v37, _v38);
px_err_39:
    if (px_err_39_proped) return px_err_39_val;
    return px_null();
}

static LXValue fn_loader_is_loaded(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v41 = (nargs > 0) ? args[0] : px_null();
    LXValue _v42 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_43_val = px_null();
    int px_err_43_proped = 0;
    LXValue _v44 = px_index(_v41, px_str("loaded"));
    if (px_is_truthy(px_method(_v44, "has", (LXValue[]){px_str("_")}, 1))) {
        return px_bool(false);
    }
    return px_method(_v44, "has", (LXValue[]){_v42}, 1);
px_err_43:
    if (px_err_43_proped) return px_err_43_val;
    return px_null();
}

static LXValue fn_loader_load(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v45 = (nargs > 0) ? args[0] : px_null();
    LXValue _v46 = (nargs > 1) ? args[1] : px_null();
    LXValue _v47 = (nargs > 2) ? args[2] : px_null();
    LXValue _v48 = px_null();
    LXValue px_err_49_val = px_null();
    int px_err_49_proped = 0;
    if (px_is_truthy(px_eq(px_call(px_get_global("len"), (LXValue[]){_v46}, 1), px_int(0LL)))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("空模块")); } _d; });
    }
    LXValue _v50 = px_call(px_get_global("join"), (LXValue[]){px_str("."), _v46}, 2);
    if (px_is_truthy(px_call(px_get_global("loader_is_loaded"), (LXValue[]){_v45, _v50}, 2))) {
        LXValue _v51 = px_index(_v45, px_str("loaded"));
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("path"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_index(_v51, _v50)); } { LXValue _k = px_str("cached"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } _d; });
    }
    if (px_is_truthy(px_call(px_get_global("contains"), (LXValue[]){px_index(_v45, px_str("stack")), _v50}, 2))) {
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_str("循环导入: "), _v50)); } _d; });
    }
    LXValue _v52 = px_null();
    if (px_is_truthy(px_eq(px_index(_v46, px_int(0LL)), px_str("std")))) {
         _v52 = px_call(px_get_global("module_find"), (LXValue[]){_v46, _v47, px_index(_v45, px_str("stdlib"))}, 3);
    }
    else {
        LXValue _t53 = px_index(_v45, px_str("search_paths"));
        for (int _t54 = 0; _t54 < px_len(_t53); _t54++) {
            LXValue _v55 = px_index(_t53, px_int(_t54));
             _v52 = px_call(px_get_global("module_find"), (LXValue[]){_v46, _v55, px_index(_v45, px_str("stdlib"))}, 3);
            if (px_is_truthy(px_ne(_v52, px_null()))) {
                break;
            }
        }
    }
    if (px_is_truthy(px_eq(_v52, px_null()))) {
        if (px_is_truthy(px_ne(px_index(_v46, px_int(0LL)), px_str("std")))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_add(px_add(px_str("[module] 警告: 找不到模块 '"), _v50), px_str("'（已跳过）"))); } _d; });
        }
        return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("err"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_str("")); } _d; });
    }
    (void)(px_call(px_get_global("loader_mark_loaded"), (LXValue[]){_v45, _v50, _v52}, 3));
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("ok"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("path"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v52); } { LXValue _k = px_str("cached"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } _d; });
px_err_49:
    if (px_err_49_proped) return px_err_49_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v56 = (nargs > 0) ? args[0] : px_null();
    LXValue _v57 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_58_val = px_null();
    int px_err_58_proped = 0;
    if (px_is_truthy(_v57)) {
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v56)}, 1));
    }
    else {
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v56)}, 1));
    }
px_err_58:
    if (px_err_58_proped) return px_err_58_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("module_search_paths", px_func("module_search_paths", fn_module_search_paths, NULL));
    px_set_global("module_candidates", px_func("module_candidates", fn_module_candidates, NULL));
    px_set_global("module_find", px_func("module_find", fn_module_find, NULL));
    px_set_global("loader_new", px_func("loader_new", fn_loader_new, NULL));
    px_set_global("loader_mark_loaded", px_func("loader_mark_loaded", fn_loader_mark_loaded, NULL));
    px_set_global("loader_is_loaded", px_func("loader_is_loaded", fn_loader_is_loaded, NULL));
    px_set_global("loader_load", px_func("loader_load", fn_loader_load, NULL));
    px_set_global("check", px_func("check", fn_check, NULL));
    px_set_global("g_pass", px_int(0LL));
    px_set_global("g_fail", px_int(0LL));
    px_set_global("BASE", px_str("/tmp/px_m5_modtest"));
    px_set_global("STDLIB", px_add(px_get_global("BASE"), px_str("/stdlib")));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== module_candidates 候选路径 ==")}, 1));
    px_set_global("c1", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("sub/file.px")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("相对路径候选"), px_eq(px_get_global("c1"), px_list_n((LXValue[]){px_add(px_get_global("BASE"), px_str("/sub/file.px"))}, 1))}, 2));
    px_set_global("c1b", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("dir/x.px")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("相对路径含.px"), px_eq(px_get_global("c1b"), px_list_n((LXValue[]){px_add(px_get_global("BASE"), px_str("/dir/x.px"))}, 1))}, 2));
    px_set_global("c2", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("std"), px_str("a"), px_str("b")}, 3), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("std 候选"), px_eq(px_get_global("c2"), px_list_n((LXValue[]){px_add(px_get_global("STDLIB"), px_str("/a/b.px")), px_add(px_get_global("STDLIB"), px_str("/a/b/mod.px"))}, 2))}, 2));
    px_set_global("c2b", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("std"), px_str("collections")}, 2), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("std 单层候选"), px_eq(px_get_global("c2b"), px_list_n((LXValue[]){px_add(px_get_global("STDLIB"), px_str("/collections.px")), px_add(px_get_global("STDLIB"), px_str("/collections/mod.px"))}, 2))}, 2));
    px_set_global("c3", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("pkg"), px_str("tools")}, 2), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("用户点分候选"), px_eq(px_get_global("c3"), px_list_n((LXValue[]){px_add(px_get_global("BASE"), px_str("/pkg/tools.px")), px_add(px_get_global("BASE"), px_str("/pkg/tools/mod.px"))}, 2))}, 2));
    px_set_global("c4", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("mylib")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("用户单段候选"), px_eq(px_get_global("c4"), px_list_n((LXValue[]){px_add(px_get_global("BASE"), px_str("/mylib.px")), px_add(px_get_global("BASE"), px_str("/mylib/mod.px"))}, 2))}, 2));
    px_set_global("c5", px_call(px_get_global("module_candidates"), (LXValue[]){px_list_n((LXValue[]){px_str("lib")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("用户 mod.px 候选"), px_eq(px_get_global("c5"), px_list_n((LXValue[]){px_add(px_get_global("BASE"), px_str("/lib.px")), px_add(px_get_global("BASE"), px_str("/lib/mod.px"))}, 2))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== module_find 存在性查找 ==")}, 1));
    px_set_global("f1", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("mylib")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find 单段文件"), px_eq(px_get_global("f1"), px_add(px_get_global("BASE"), px_str("/mylib.px")))}, 2));
    px_set_global("f2", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("pkg"), px_str("tools")}, 2), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find 点分文件"), px_eq(px_get_global("f2"), px_add(px_get_global("BASE"), px_str("/pkg/tools.px")))}, 2));
    px_set_global("f3", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("lib")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find mod.px 目录"), px_eq(px_get_global("f3"), px_add(px_get_global("BASE"), px_str("/lib/mod.px")))}, 2));
    px_set_global("f4", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("std"), px_str("collections")}, 2), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find std 模块"), px_eq(px_get_global("f4"), px_add(px_get_global("STDLIB"), px_str("/collections.px")))}, 2));
    px_set_global("f5", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("nope")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find 不存在 null"), px_eq(px_get_global("f5"), px_null())}, 2));
    px_set_global("f6", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("missing/file.px")}, 1), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find 相对路径不存在"), px_eq(px_get_global("f6"), px_null())}, 2));
    px_set_global("f7", px_call(px_get_global("module_find"), (LXValue[]){px_list_n((LXValue[]){px_str("std"), px_str("nope")}, 2), px_get_global("BASE"), px_get_global("STDLIB")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("find std 不存在"), px_eq(px_get_global("f7"), px_null())}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== loader 缓存与循环检测 ==")}, 1));
    px_set_global("ld", px_call(px_get_global("loader_new"), (LXValue[]){px_get_global("BASE"), px_get_global("STDLIB")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("loader loaded 初始未加载"), px_not(px_call(px_get_global("loader_is_loaded"), (LXValue[]){px_get_global("ld"), px_str("mylib")}, 2))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("loader search_paths 含 base"), px_eq(px_index(px_index(px_get_global("ld"), px_str("search_paths")), px_int(0LL)), px_get_global("BASE"))}, 2));
    px_set_global("r1", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld"), px_list_n((LXValue[]){px_str("mylib")}, 1), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("首次加载 ok"), px_index(px_get_global("r1"), px_str("ok"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("首次加载非缓存"), px_not(px_index(px_get_global("r1"), px_str("cached")))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("首次加载路径"), px_eq(px_index(px_get_global("r1"), px_str("path")), px_add(px_get_global("BASE"), px_str("/mylib.px")))}, 2));
    px_set_global("r2", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld"), px_list_n((LXValue[]){px_str("mylib")}, 1), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("二次加载 cached"), ({ LXValue _t59 = px_index(px_get_global("r2"), px_str("ok")); px_is_truthy(_t59) ? px_index(px_get_global("r2"), px_str("cached")) : _t59; })}, 2));
    px_set_global("r3", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld"), px_list_n((LXValue[]){px_str("ghost")}, 1), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("缺失用户模块 err"), px_not(px_index(px_get_global("r3"), px_str("ok")))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("缺失警告消息"), px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("r3"), px_str("err")), px_str("找不到模块 'ghost'")}, 2)}, 2));
    px_set_global("r4", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld"), px_list_n((LXValue[]){px_str("std"), px_str("nope")}, 2), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("std 缺失静默"), ({ LXValue _t60 = px_not(px_index(px_get_global("r4"), px_str("ok"))); px_is_truthy(_t60) ? px_eq(px_index(px_get_global("r4"), px_str("err")), px_str("")) : _t60; })}, 2));
    px_set_global("ld2", px_call(px_get_global("loader_new"), (LXValue[]){px_get_global("BASE"), px_get_global("STDLIB")}, 2));
    (void)(px_method(px_index(px_get_global("ld2"), px_str("stack")), "append", (LXValue[]){px_str("a.b")}, 1));
    px_set_global("r5", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld2"), px_list_n((LXValue[]){px_str("a"), px_str("b")}, 2), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("循环导入检测"), ({ LXValue _t61 = px_not(px_index(px_get_global("r5"), px_str("ok"))); px_is_truthy(_t61) ? px_call(px_get_global("contains"), (LXValue[]){px_index(px_get_global("r5"), px_str("err")), px_str("循环导入: a.b")}, 2) : _t61; })}, 2));
    px_index_set(px_get_global("ld2"), px_str("stack"), px_list_n((LXValue[]){}, 0));
    px_set_global("r6", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld2"), px_list_n((LXValue[]){px_str("mylib")}, 1), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("弹栈后可加载"), px_index(px_get_global("r6"), px_str("ok"))}, 2));
    px_set_global("r7", px_call(px_get_global("loader_load"), (LXValue[]){px_get_global("ld2"), px_list_n((LXValue[]){}, 0), px_get_global("BASE")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("空模块 err"), px_not(px_index(px_get_global("r7"), px_str("ok")))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("结果: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_pass")}, 1)), px_str(" PASS / ")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_fail")}, 1)), px_str(" FAIL"))}, 1));
    if (px_is_truthy(px_gt(px_get_global("g_fail"), px_int(0LL)))) {
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    return 0;
}
