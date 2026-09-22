#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_env_new(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_new");
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2 = px_uninit();
    LXValue _v3 = px_uninit();
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    px_srcline(26);
    _v2 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(27);
    (void)(px_method(_v2, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(28);
    _v3 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(29);
    (void)(px_method(_v3, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(30);
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("vars"); LXValue _v = _v2; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("parent"); LXValue _v = _v1; px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("ub"); LXValue _v = _v3; px_dict_set_checked(_d, _k, _v); } _d; });
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

static LXValue fn_env_define(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_define");
    LXValue _v5 = (nargs > 0) ? args[0] : px_null();
    LXValue _v6 = (nargs > 1) ? args[1] : px_null();
    LXValue _v7 = (nargs > 2) ? args[2] : px_null();
    LXValue _v8 = px_uninit();
    LXValue _v9 = px_uninit();
    LXValue px_err_10_val = px_null();
    int px_err_10_proped = 0;
    px_srcline(33);
    _v8 = px_index(_v5, px_str("vars"));
    px_srcline(34);
    px_index_set(_v8, _v6, _v7);
    px_srcline(35);
    _v9 = px_index(_v5, px_str("ub"));
    px_srcline(36);
    if (px_is_truthy(px_method(_v9, "has", (LXValue[]){_v6}, 1))) {
        px_srcline(37);
        (void)(px_method(_v9, "remove", (LXValue[]){_v6}, 1));
    }
px_err_10:
    if (px_err_10_proped) return px_err_10_val;
    return px_null();
}

static LXValue fn_env_mark_unbound(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_mark_unbound");
    LXValue _v11 = (nargs > 0) ? args[0] : px_null();
    LXValue _v12 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    px_srcline(40);
    px_index_set(px_index(_v11, px_str("ub")), _v12, px_int(1LL));
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

static LXValue fn_env_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_get");
    LXValue _v14 = (nargs > 0) ? args[0] : px_null();
    LXValue _v15 = (nargs > 1) ? args[1] : px_null();
    LXValue _v16 = px_uninit();
    LXValue _v17 = px_uninit();
    LXValue px_err_18_val = px_null();
    int px_err_18_proped = 0;
    px_srcline(44);
    _v16 = _v14;
    px_srcline(45);
    while (px_is_truthy(px_ne(_v16, px_null()))) {
        px_srcline(47);
        if (px_is_truthy(px_method(px_index(_v16, px_str("ub")), "has", (LXValue[]){_v15}, 1))) {
            px_srcline(48);
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("found"); LXValue _v = px_bool(true); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("value"); LXValue _v = px_null(); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("unbound"); LXValue _v = px_bool(true); px_dict_set_checked(_d, _k, _v); } _d; });
        }
        px_srcline(49);
        _v17 = px_index(_v16, px_str("vars"));
        px_srcline(50);
        if (px_is_truthy(px_method(_v17, "has", (LXValue[]){_v15}, 1))) {
            px_srcline(51);
            return ({ LXValue _d = px_dict(); { LXValue _k = px_str("found"); LXValue _v = px_bool(true); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("value"); LXValue _v = px_index(_v17, _v15); px_dict_set_checked(_d, _k, _v); } _d; });
        }
        px_srcline(52);
         _v16 = px_index(_v16, px_str("parent"));
    }
    px_srcline(53);
    return ({ LXValue _d = px_dict(); { LXValue _k = px_str("found"); LXValue _v = px_bool(false); px_dict_set_checked(_d, _k, _v); } { LXValue _k = px_str("value"); LXValue _v = px_null(); px_dict_set_checked(_d, _k, _v); } _d; });
px_err_18:
    if (px_err_18_proped) return px_err_18_val;
    return px_null();
}

static LXValue fn_env_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_set");
    LXValue _v19 = (nargs > 0) ? args[0] : px_null();
    LXValue _v20 = (nargs > 1) ? args[1] : px_null();
    LXValue _v21 = (nargs > 2) ? args[2] : px_null();
    LXValue _v22 = px_uninit();
    LXValue _v23 = px_uninit();
    LXValue px_err_24_val = px_null();
    int px_err_24_proped = 0;
    px_srcline(58);
    _v22 = _v19;
    px_srcline(59);
    while (px_is_truthy(px_ne(_v22, px_null()))) {
        px_srcline(60);
        if (px_is_truthy(px_method(px_index(_v22, px_str("ub")), "has", (LXValue[]){_v20}, 1))) {
            px_srcline(61);
            (void)(px_call(px_get_global("env_define"), (LXValue[]){_v22, _v20, _v21}, 3));
            px_srcline(62);
            return px_bool(true);
        }
        px_srcline(63);
        _v23 = px_index(_v22, px_str("vars"));
        px_srcline(64);
        if (px_is_truthy(px_method(_v23, "has", (LXValue[]){_v20}, 1))) {
            px_srcline(65);
            px_index_set(_v23, _v20, _v21);
            px_srcline(66);
            return px_bool(true);
        }
        px_srcline(67);
         _v22 = px_index(_v22, px_str("parent"));
    }
    px_srcline(68);
    return px_bool(false);
px_err_24:
    if (px_err_24_proped) return px_err_24_val;
    return px_null();
}

static LXValue fn_env_has_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_has_local");
    LXValue _v25 = (nargs > 0) ? args[0] : px_null();
    LXValue _v26 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_27_val = px_null();
    int px_err_27_proped = 0;
    px_srcline(71);
    return ({ LXValue _t28 = px_method(px_index(_v25, px_str("vars")), "has", (LXValue[]){_v26}, 1); px_is_truthy(_t28) ? _t28 : px_method(px_index(_v25, px_str("ub")), "has", (LXValue[]){_v26}, 1); });
px_err_27:
    if (px_err_27_proped) return px_err_27_val;
    return px_null();
}

static LXValue fn_env_has(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_has");
    LXValue _v29 = (nargs > 0) ? args[0] : px_null();
    LXValue _v30 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_31_val = px_null();
    int px_err_31_proped = 0;
    px_srcline(74);
    return px_index(px_call(px_get_global("env_get"), (LXValue[]){_v29, _v30}, 2), px_str("found"));
px_err_31:
    if (px_err_31_proped) return px_err_31_val;
    return px_null();
}

static LXValue fn_env_items(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_items");
    LXValue _v32 = (nargs > 0) ? args[0] : px_null();
    LXValue _v33 = px_uninit();
    LXValue _v34 = px_uninit();
    LXValue _v35 = px_uninit();
    LXValue px_err_36_val = px_null();
    int px_err_36_proped = 0;
    px_srcline(77);
    _v33 = px_list_n((LXValue[]){}, 0);
    px_srcline(78);
    _v34 = px_index(_v32, px_str("vars"));
    px_srcline(79);
    LXValue _t37 = px_method(_v34, "keys", (LXValue[]){}, 0);
    int _il1 = (int)px_len(_t37);
    for (int _t38 = 0; _t38 < _il1; _t38++) {
        px_iter_ck(_t37, _il1);
        _v35 = px_iter_at(_t37, px_int(_t38));
        px_srcline(80);
        (void)(px_method(_v33, "append", (LXValue[]){px_list_n((LXValue[]){_v35, px_index(_v34, _v35)}, 2)}, 1));
    }
    px_srcline(81);
    return _v33;
px_err_36:
    if (px_err_36_proped) return px_err_36_val;
    return px_null();
}

static LXValue fn_env_clear(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("env_clear");
    LXValue _v39 = (nargs > 0) ? args[0] : px_null();
    LXValue _v40 = px_uninit();
    LXValue px_err_41_val = px_null();
    int px_err_41_proped = 0;
    px_srcline(84);
    _v40 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("_"); LXValue _v = px_int(0LL); px_dict_set_checked(_d, _k, _v); } _d; });
    px_srcline(85);
    (void)(px_method(_v40, "remove", (LXValue[]){px_str("_")}, 1));
    px_srcline(86);
    px_index_set(_v39, px_str("vars"), _v40);
px_err_41:
    if (px_err_41_proped) return px_err_41_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("check");
    LXValue _v42 = (nargs > 0) ? args[0] : px_null();
    LXValue _v43 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_44_val = px_null();
    int px_err_44_proped = 0;
    px_srcline(15);
    if (px_is_truthy(_v43)) {
        px_srcline(16);
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        px_srcline(17);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v42)}, 1));
    }
    else {
        px_srcline(19);
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        px_srcline(20);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v42)}, 1));
    }
px_err_44:
    if (px_err_44_proped) return px_err_44_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("env_new", px_func("env_new", fn_env_new, NULL));
    px_set_global("env_define", px_func("env_define", fn_env_define, NULL));
    px_set_global("env_mark_unbound", px_func("env_mark_unbound", fn_env_mark_unbound, NULL));
    px_set_global("env_get", px_func("env_get", fn_env_get, NULL));
    px_set_global("env_set", px_func("env_set", fn_env_set, NULL));
    px_set_global("env_has_local", px_func("env_has_local", fn_env_has_local, NULL));
    px_set_global("env_has", px_func("env_has", fn_env_has, NULL));
    px_set_global("env_items", px_func("env_items", fn_env_items, NULL));
    px_set_global("env_clear", px_func("env_clear", fn_env_clear, NULL));
    px_set_global("check", px_func("check", fn_check, NULL));
    px_srcline(11);
    px_set_global("g_pass", px_int(0LL));
    px_srcline(12);
    px_set_global("g_fail", px_int(0LL));
    px_srcline(23);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_new 创建 ==")}, 1));
    px_srcline(24);
    px_set_global("e", px_call(px_get_global("env_new"), (LXValue[]){px_null()}, 1));
    px_srcline(25);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env 类型 dict"), px_eq(px_call(px_get_global("type"), (LXValue[]){px_get_global("e")}, 1), px_str("dict"))}, 2));
    px_srcline(26);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env 有 vars"), px_ne(px_index(px_get_global("e"), px_str("vars")), px_null())}, 2));
    px_srcline(27);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env parent null"), px_eq(px_index(px_get_global("e"), px_str("parent")), px_null())}, 2));
    px_srcline(28);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("env vars 空"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("e"), px_str("vars"))}, 1), px_int(0LL))}, 2));
    px_srcline(31);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_define / env_get（单层） ==")}, 1));
    px_srcline(32);
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("e"), px_str("x"), px_int(42LL)}, 3));
    px_srcline(33);
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("e"), px_str("name"), px_str("px")}, 3));
    px_srcline(34);
    px_set_global("g1", px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2));
    px_srcline(35);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get x found"), px_index(px_get_global("g1"), px_str("found"))}, 2));
    px_srcline(36);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get x value"), px_eq(px_index(px_get_global("g1"), px_str("value")), px_int(42LL))}, 2));
    px_srcline(37);
    px_set_global("g2", px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("name")}, 2));
    px_srcline(38);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get name value"), px_eq(px_index(px_get_global("g2"), px_str("value")), px_str("px"))}, 2));
    px_srcline(39);
    px_set_global("g3", px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("nope")}, 2));
    px_srcline(40);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get 不存在 found=false"), px_not(px_index(px_get_global("g3"), px_str("found")))}, 2));
    px_srcline(41);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("get 不存在 value=null"), px_eq(px_index(px_get_global("g3"), px_str("value")), px_null())}, 2));
    px_srcline(42);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has x"), px_call(px_get_global("env_has"), (LXValue[]){px_get_global("e"), px_str("x")}, 2)}, 2));
    px_srcline(43);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has nope"), px_not(px_call(px_get_global("env_has"), (LXValue[]){px_get_global("e"), px_str("nope")}, 2))}, 2));
    px_srcline(44);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has_local x"), px_call(px_get_global("env_has_local"), (LXValue[]){px_get_global("e"), px_str("x")}, 2)}, 2));
    px_srcline(45);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("has_local nope"), px_not(px_call(px_get_global("env_has_local"), (LXValue[]){px_get_global("e"), px_str("nope")}, 2))}, 2));
    px_srcline(48);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== 父子链（子可读父） ==")}, 1));
    px_srcline(49);
    px_set_global("child", px_call(px_get_global("env_new"), (LXValue[]){px_get_global("e")}, 1));
    px_srcline(52);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child 读父变量"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(42LL))}, 2));
    px_srcline(53);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child 读父 name"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("name")}, 2), px_str("value")), px_str("px"))}, 2));
    px_srcline(54);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child has 父变量"), px_call(px_get_global("env_has"), (LXValue[]){px_get_global("child"), px_str("x")}, 2)}, 2));
    px_srcline(55);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("child has_local 父变量=否"), px_not(px_call(px_get_global("env_has_local"), (LXValue[]){px_get_global("child"), px_str("x")}, 2))}, 2));
    px_srcline(58);
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("child"), px_str("only_child"), px_int(7LL)}, 3));
    px_srcline(59);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父读子变量=否"), px_not(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("only_child")}, 2), px_str("found")))}, 2));
    px_srcline(60);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 has 子变量=否"), px_not(px_call(px_get_global("env_has"), (LXValue[]){px_get_global("e"), px_str("only_child")}, 2))}, 2));
    px_srcline(63);
    (void)(px_call(px_get_global("env_define"), (LXValue[]){px_get_global("child"), px_str("x"), px_int(100LL)}, 3));
    px_srcline(64);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("子遮蔽 x"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(100LL))}, 2));
    px_srcline(65);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 x 不变"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2), px_str("value")), px_int(42LL))}, 2));
    px_srcline(68);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_set 就近赋值 ==")}, 1));
    px_srcline(70);
    px_set_global("ok1", px_call(px_get_global("env_set"), (LXValue[]){px_get_global("child"), px_str("x"), px_int(999LL)}, 3));
    px_srcline(71);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("set 子 x 成功"), px_get_global("ok1")}, 2));
    px_srcline(72);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("子 x=999"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(999LL))}, 2));
    px_srcline(73);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 x 仍 42"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2), px_str("value")), px_int(42LL))}, 2));
    px_srcline(76);
    px_set_global("ok2", px_call(px_get_global("env_set"), (LXValue[]){px_get_global("child"), px_str("name"), px_str("newpx")}, 3));
    px_srcline(77);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("set name 成功"), px_get_global("ok2")}, 2));
    px_srcline(78);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("父 name 被改"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("name")}, 2), px_str("value")), px_str("newpx"))}, 2));
    px_srcline(81);
    px_set_global("ok3", px_call(px_get_global("env_set"), (LXValue[]){px_get_global("child"), px_str("ghost"), px_int(1LL)}, 3));
    px_srcline(82);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("set 不存在=false"), px_not(px_get_global("ok3"))}, 2));
    px_srcline(83);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("ghost 未创建"), px_not(px_call(px_get_global("env_has"), (LXValue[]){px_get_global("child"), px_str("ghost")}, 2))}, 2));
    px_srcline(86);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== 三层链 ==")}, 1));
    px_srcline(87);
    px_set_global("grand", px_call(px_get_global("env_new"), (LXValue[]){px_get_global("child")}, 1));
    px_srcline(88);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("孙读爷"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("grand"), px_str("x")}, 2), px_str("value")), px_int(999LL))}, 2));
    px_srcline(89);
    (void)(px_call(px_get_global("env_set"), (LXValue[]){px_get_global("grand"), px_str("name"), px_str("deep")}, 3));
    px_srcline(90);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("孙就近改爷 name"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("name")}, 2), px_str("value")), px_str("deep"))}, 2));
    px_srcline(91);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("孙改的是爷层"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("name")}, 2), px_str("value")), px_str("deep"))}, 2));
    px_srcline(94);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("== env_items / env_clear ==")}, 1));
    px_srcline(95);
    px_set_global("items", px_call(px_get_global("env_items"), (LXValue[]){px_get_global("e")}, 1));
    px_srcline(96);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 是 list"), px_eq(px_call(px_get_global("type"), (LXValue[]){px_get_global("items")}, 1), px_str("list"))}, 2));
    px_srcline(98);
    px_set_global("item_names", px_list_n((LXValue[]){}, 0));
    px_srcline(99);
    LXValue _t58 = px_get_global("items");
    int _il2 = (int)px_len(_t58);
    for (int _t59 = 0; _t59 < _il2; _t59++) {
        px_iter_ck(_t58, _il2);
        px_set_global("it", px_iter_at(_t58, px_int(_t59)));
        px_srcline(100);
        (void)(px_method(px_get_global("item_names"), "append", (LXValue[]){px_index(px_get_global("it"), px_int(0LL))}, 1));
    }
    px_srcline(101);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 含 x"), px_call(px_get_global("contains"), (LXValue[]){px_get_global("item_names"), px_str("x")}, 2)}, 2));
    px_srcline(102);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 含 name"), px_call(px_get_global("contains"), (LXValue[]){px_get_global("item_names"), px_str("name")}, 2)}, 2));
    px_srcline(103);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("items 数量 2"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_get_global("items")}, 1), px_int(2LL))}, 2));
    px_srcline(105);
    (void)(px_call(px_get_global("env_clear"), (LXValue[]){px_get_global("e")}, 1));
    px_srcline(106);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后 vars 空"), px_eq(px_call(px_get_global("len"), (LXValue[]){px_index(px_get_global("e"), px_str("vars"))}, 1), px_int(0LL))}, 2));
    px_srcline(107);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后 get x 未找到"), px_not(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("e"), px_str("x")}, 2), px_str("found")))}, 2));
    px_srcline(109);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后子读自己 x"), px_eq(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("x")}, 2), px_str("value")), px_int(999LL))}, 2));
    px_srcline(111);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("clear 后子读不到爷 name"), px_not(px_index(px_call(px_get_global("env_get"), (LXValue[]){px_get_global("child"), px_str("name")}, 2), px_str("found")))}, 2));
    px_srcline(114);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("")}, 1));
    px_srcline(115);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(({ LXValue _s1 = px_add(px_add(px_str("结果: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_pass")}, 1)), px_str(" PASS / ")); LXValue _s2 = px_call(px_get_global("str"), (LXValue[]){px_get_global("g_fail")}, 1); px_add(_s1, _s2); }), px_str(" FAIL"))}, 1));
    px_srcline(116);
    if (px_is_truthy(px_gt(px_get_global("g_fail"), px_int(0LL)))) {
        px_srcline(117);
        (void)(px_call(px_get_global("exit"), (LXValue[]){px_int(1LL)}, 1));
    }
    return px_exit_code_final(0);   // M120（qg-issue 76 E1）
}
