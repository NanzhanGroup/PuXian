#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_env_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2 = px_null();
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    _v2 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v2, "remove", (LXValue[]){px_str("_")}, 1));
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("vars"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v2); } { LXValue _k = px_str("parent"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, _v1); } _d; });
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_env_define(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v4 = (nargs > 0) ? args[0] : px_null();
    LXValue _v5 = (nargs > 1) ? args[1] : px_null();
    LXValue _v6 = (nargs > 2) ? args[2] : px_null();
    LXValue _v7 = px_null();
    LXValue px_err_8_val = px_null();
    int px_err_8_proped = 0;
    _v7 = px_index(_v4, px_str("vars"));
    px_index_set(_v7, _v5, _v6);
px_err_8:
    if (px_err_8_proped) return px_err_8_val;
    return px_null();
}

static LXValue fn_env_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v9 = (nargs > 0) ? args[0] : px_null();
    LXValue _v10 = (nargs > 1) ? args[1] : px_null();
    LXValue _v11 = px_null();
    LXValue _v12 = px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    _v11 = _v9;
    while (px_is_truthy(px_ne(_v11, px_null()))) {
        _v12 = px_index(_v11, px_str("vars"));
        if (px_is_truthy(px_method(_v12, "has", (LXValue[]){_v10}, 1))) {
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("found"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(true)); } { LXValue _k = px_str("value"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_index(_v12, _v10)); } _d; });
        }
         _v11 = px_index(_v11, px_str("parent"));
    }
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("found"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_bool(false)); } { LXValue _k = px_str("value"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_null()); } _d; });
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

static LXValue fn_env_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue _v15 = (nargs > 1) ? args[1] : px_null();
    LXValue _v16 = (nargs > 2) ? args[2] : px_null();
    LXValue _v17 = px_null();
    LXValue _v18 = px_null();
    LXValue px_err_19_val = px_null();
    int px_err_19_proped = 0;
    _v17 = _v14;
    while (px_is_truthy(px_ne(_v17, px_null()))) {
        _v18 = px_index(_v17, px_str("vars"));
        if (px_is_truthy(px_method(_v18, "has", (LXValue[]){_v15}, 1))) {
            px_index_set(_v18, _v15, _v16);
            return px_bool(true);
        }
         _v17 = px_index(_v17, px_str("parent"));
    }
    return px_bool(false);
px_err_19:
    if (px_err_19_proped) return px_err_19_val;
    return px_null();
}

static LXValue fn_env_has_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v20 = (nargs > 0) ? args[0] : px_null();
    LXValue _v21 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_22_val = px_null();
    int px_err_22_proped = 0;
    return px_method(px_index(_v20, px_str("vars")), "has", (LXValue[]){_v21}, 1);
px_err_22:
    if (px_err_22_proped) return px_err_22_val;
    return px_null();
}

static LXValue fn_env_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v23 = (nargs > 0) ? args[0] : px_null();
    LXValue _v24 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_25_val = px_null();
    int px_err_25_proped = 0;
    return px_index(px_call(px_get_global("env_get"), (LXValue[]){_v23, _v24}, 2), px_str("found"));
px_err_25:
    if (px_err_25_proped) return px_err_25_val;
    return px_null();
}

static LXValue fn_env_items(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v26 = (nargs > 0) ? args[0] : px_null();
    LXValue _v27 = px_null();
    LXValue _v28 = px_null();
    LXValue _v29 = px_null();
    LXValue px_err_30_val = px_null();
    int px_err_30_proped = 0;
    _v27 = px_list_n((LXValue[]){}, 0);
    _v28 = px_index(_v26, px_str("vars"));
    LXValue _t31 = px_method(_v28, "keys", (LXValue[]){}, 0);
    for (int _t32 = 0; _t32 < px_len(_t31); _t32++) {
        _v29 = px_index(_t31, px_int(_t32));
        (void)(px_method(_v27, "append", (LXValue[]){px_list_n((LXValue[]){_v29, px_index(_v28, _v29)}, 2)}, 1));
    }
    return _v27;
px_err_30:
    if (px_err_30_proped) return px_err_30_val;
    return px_null();
}

static LXValue fn_env_clear(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v33 = (nargs > 0) ? args[0] : px_null();
    LXValue _v34 = px_null();
    LXValue px_err_35_val = px_null();
    int px_err_35_proped = 0;
    _v34 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(0LL)); } _d; });
    (void)(px_method(_v34, "remove", (LXValue[]){px_str("_")}, 1));
    px_index_set(_v33, px_str("vars"), _v34);
px_err_35:
    if (px_err_35_proped) return px_err_35_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v36 = (nargs > 0) ? args[0] : px_null();
    LXValue _v37 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_38_val = px_null();
    int px_err_38_proped = 0;
    if (px_is_truthy(_v37)) {
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v36)}, 1));
    }
    else {
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v36)}, 1));
    }
px_err_38:
    if (px_err_38_proped) return px_err_38_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("env_new", px_func("env_new", fn_env_new, NULL));
    px_set_global("env_define", px_func("env_define", fn_env_define, NULL));
    px_set_global("env_get", px_func("env_get", fn_env_get, NULL));
    px_set_global("env_set", px_func("env_set", fn_env_set, NULL));
    px_set_global("env_has_local", px_func("env_has_local", fn_env_has_local, NULL));
    px_set_global("env_has", px_func("env_has", fn_env_has, NULL));
    px_set_global("env_items", px_func("env_items", fn_env_items, NULL));
    px_set_global("env_clear", px_func("env_clear", fn_env_clear, NULL));
    px_set_global("check", px_func("check", fn_check, NULL));
    px_set_global("g_pass", px_int(0LL));
    px_set_global("g_fail", px_int(0LL));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_new 创建 ==")}, 1));
    px_set_global("e", px_call(px_get_global("env_new"), (LXValue[]){px_null()}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env 类型 dict"), px_eq(px_call(px_get_global("type"), (LXValue[]){px_get_global("e")}, 1), px_str("dict"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env 有 vars"), px_ne(px_index(px_get_global("e"), px_str("vars")), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env parent null"), px_eq(px_index(px_get_global("e"), px_str("parent")), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env vars 空"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("e"), px_str("vars"))}, 1), px_int(0LL))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_define / env_get（单层） ==")}, 1));
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("e"), px_str("x"), px_int(42LL)}, 3));
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("e"), px_str("name"), px_str("px")}, 3));
    px_set_global("g1", px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get x found"), px_index(px_get_global("g1"), px_str("found"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get x value"), px_eq(px_index(px_get_global("g1"), px_str("value")), px_int(42LL))}, 2));
    px_set_global("g2", px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("name")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get name value"), px_eq(px_index(px_get_global("g2"), px_str("value")), px_str("px"))}, 2));
    px_set_global("g3", px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("nope")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get 不存在 found=false"), px_not(px_index(px_get_global("g3"), px_str("found")))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get 不存在 value=null"), px_eq(px_index(px_get_global("g3"), px_str("value")), px_null())}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has x"), px_call(px_get_global("env_has"), (LXValue[]){px_get_global("e"), px_str("x")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has nope"), px_not(px_call(px_get_global("env_has"), (LXValue[]){px_get_global("e"), px_str("nope")}, 2))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has_local x"), px_call(px_get_global("env_has_local"), (LXValue[]){px_get_global("e"), px_str("x")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has_local nope"), px_not(px_call(px_get_global("env_has_local"), (LXValue[]){px_get_global("e"), px_str("nope")}, 2))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== 父子链（子可读父） ==")}, 1));
    px_set_global("child", px_call(px_get_global("env_new"), (LXValue[]){px_get_global("e")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child 读父变量"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(42LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child 读父 name"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("name")}, 2), px_str("value")), px_str("px"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child has 父变量"), px_call(px_get_global("env_has"), (LXValue[]){px_get_global("child"), px_str("x")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child has_local 父变量=否"), px_not(px_call(px_get_global("env_has_local"), (LXValue[]){px_get_global("child"), px_str("x")}, 2))}, 2));
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("child"), px_str("only_child"), px_int(7LL)}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父读子变量=否"), px_not(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("only_child")}, 2), px_str("found")))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 has 子变量=否"), px_not(px_call(px_get_global("env_has"), (LXValue[]){px_get_global("e"), px_str("only_child")}, 2))}, 2));
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("child"), px_str("x"), px_int(100LL)}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("子遮蔽 x"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(100LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 x 不变"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2), px_str("value")), px_int(42LL))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_set 就近赋值 ==")}, 1));
    px_set_global("ok1", px_call(px_get_global("env_set"), (LXValue[]){px_get_global("child"), px_str("x"), px_int(999LL)}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("set 子 x 成功"), px_get_global("ok1")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("子 x=999"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(999LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 x 仍 42"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2), px_str("value")), px_int(42LL))}, 2));
    px_set_global("ok2", px_call(px_get_global("env_set"), (LXValue[]){px_get_global("child"), px_str("name"), px_str("newpx")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("set name 成功"), px_get_global("ok2")}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 name 被改"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("name")}, 2), px_str("value")), px_str("newpx"))}, 2));
    px_set_global("ok3", px_call(px_get_global("env_set"), (LXValue[]){px_get_global("child"), px_str("ghost"), px_int(1LL)}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("set 不存在=false"), px_not(px_get_global("ok3"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("ghost 未创建"), px_not(px_call(px_get_global("env_has"), (LXValue[]){px_get_global("child"), px_str("ghost")}, 2))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== 三层链 ==")}, 1));
    px_set_global("grand", px_call(px_get_global("env_new"), (LXValue[]){px_get_global("child")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("孙读爷"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("grand"), px_str("x")}, 2), px_str("value")), px_int(999LL))}, 2));
    (void)(px_call(px_get_global("env_set"), (LXValue[]){px_get_global("grand"), px_str("name"), px_str("deep")}, 3));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("孙就近改爷 name"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("name")}, 2), px_str("value")), px_str("deep"))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("孙改的是爷层"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("name")}, 2), px_str("value")), px_str("deep"))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_items / env_clear ==")}, 1));
    px_set_global("items", px_call(px_get_global("env_items"), (LXValue[]){px_get_global("e")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 是 list"), px_eq(px_call(px_get_global("type"), (LXValue[]){px_get_global("items")}, 1), px_str("list"))}, 2));
    px_set_global("item_names", px_list_n((LXValue[]){}, 0));
    LXValue _t39 = px_get_global("items");
    for (int _t40 = 0; _t40 < px_len(_t39); _t40++) {
        LXValue _v41 = px_index(_t39, px_int(_t40));
        (void)(px_method(px_get_global("item_names"), "append", (LXValue[]){px_index(_v41, px_int(0LL))}, 1));
    }
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 含 x"), px_call(px_get_global("contains"), (LXValue[]){px_get_global("item_names"), px_str("x")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 含 name"), px_call(px_get_global("contains"), (LXValue[]){px_get_global("item_names"), px_str("name")}, 2)}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 数量 2"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("items")}, 1), px_int(2LL))}, 2));
    (void)(px_call(px_get_global("env_clear"), (LXValue[]){px_get_global("e")}, 1));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后 vars 空"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("e"), px_str("vars"))}, 1), px_int(0LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后 get x 未找到"), px_not(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2), px_str("found")))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后子读自己 x"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(999LL))}, 2));
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后子读不到爷 name"), px_not(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("name")}, 2), px_str("found")))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("结果: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_pass")}, 1)), px_str(" PASS / ")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_fail")}, 1)), px_str(" FAIL"))}, 1));
    if (px_is_truthy(px_gt(px_get_global("g_fail"), px_int(0LL)))) {
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    return 0;
}
