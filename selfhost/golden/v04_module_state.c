#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_counter_inc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("counter_inc");
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    px_srcline(13);
    px_set_global("g_counter", px_add(px_get_global("g_counter"), px_int(1LL)));
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
    return px_null();
}

static LXValue fn_counter_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("counter_get");
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcline(16);
    return px_get_global("g_counter");
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_label_set(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("label_set");
    LXValue _v3 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    px_srcline(19);
    px_set_global("g_label", _v3);
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

static LXValue fn_label_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("label_get");
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(22);
    return px_get_global("g_label");
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_limit_get(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("limit_get");
    LXValue px_err_6_val = px_null();
    int px_err_6_proped = 0;
    px_srcline(25);
    return px_get_global("CFG_LIMIT");
px_err_6:
    if (px_err_6_proped) return px_err_6_val;
    return px_null();
}

static LXValue fn_check(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("check");
    LXValue _v7 = (nargs > 0) ? args[0] : px_null();
    LXValue _v8 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_9_val = px_null();
    int px_err_9_proped = 0;
    px_srcline(17);
    if (px_is_truthy(_v8)) {
        px_srcline(18);
        px_set_global("g_pass", px_add(px_get_global("g_pass"), px_int(1LL)));
        px_srcline(19);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[PASS] "), _v7)}, 1));
    }
    else {
        px_srcline(21);
        px_set_global("g_fail", px_add(px_get_global("g_fail"), px_int(1LL)));
        px_srcline(22);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("[FAIL] "), _v7)}, 1));
    }
px_err_9:
    if (px_err_9_proped) return px_err_9_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("counter_inc", px_func("counter_inc", fn_counter_inc, NULL));
    px_set_global("counter_get", px_func("counter_get", fn_counter_get, NULL));
    px_set_global("label_set", px_func("label_set", fn_label_set, NULL));
    px_set_global("label_get", px_func("label_get", fn_label_get, NULL));
    px_set_global("limit_get", px_func("limit_get", fn_limit_get, NULL));
    px_set_global("check", px_func("check", fn_check, NULL));
    px_srcline(7);
    px_set_global("g_counter", px_int(0LL));
    px_srcline(8);
    px_set_global("g_label", px_str("init"));
    px_srcline(10);
    px_set_global("CFG_LIMIT", px_int(100LL));
    px_srcline(13);
    px_set_global("g_pass", px_int(0LL));
    px_srcline(14);
    px_set_global("g_fail", px_int(0LL));
    px_srcline(25);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("模块 var 初始值"), px_eq(px_call(px_get_global("counter_get"), (LXValue[]){}, 0), px_int(0LL))}, 2));
    px_srcline(28);
    (void)(px_call(px_get_global("counter_inc"), (LXValue[]){}, 0));
    px_srcline(29);
    (void)(px_call(px_get_global("counter_inc"), (LXValue[]){}, 0));
    px_srcline(30);
    (void)(px_call(px_get_global("counter_inc"), (LXValue[]){}, 0));
    px_srcline(31);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("模块函数自增 3 次"), px_eq(px_call(px_get_global("counter_get"), (LXValue[]){}, 0), px_int(3LL))}, 2));
    px_srcline(34);
    px_set_global("g_counter", px_int(100LL));
    px_srcline(35);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("主程序直名写模块 var"), px_eq(px_call(px_get_global("counter_get"), (LXValue[]){}, 0), px_int(100LL))}, 2));
    px_srcline(36);
    px_set_global("g_counter", px_add(px_get_global("g_counter"), px_int(1LL)));
    px_srcline(37);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("主程序直名读+写"), px_eq(px_call(px_get_global("counter_get"), (LXValue[]){}, 0), px_int(101LL))}, 2));
    px_srcline(40);
    (void)(px_call(px_get_global("label_set"), (LXValue[]){px_str("hello")}, 1));
    px_srcline(41);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("模块函数写 label"), px_eq(px_call(px_get_global("label_get"), (LXValue[]){}, 0), px_str("hello"))}, 2));
    px_srcline(42);
    px_set_global("g_label", px_str("main-write"));
    px_srcline(43);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("主程序直名写 label"), px_eq(px_call(px_get_global("label_get"), (LXValue[]){}, 0), px_str("main-write"))}, 2));
    px_srcline(46);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("模块 let 可读"), px_eq(px_get_global("CFG_LIMIT"), px_int(100LL))}, 2));
    px_srcline(47);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("模块 let 经函数读"), px_eq(px_call(px_get_global("limit_get"), (LXValue[]){}, 0), px_int(100LL))}, 2));
    px_srcline(50);
    (void)(px_call(px_get_global("check"), (LXValue[]){px_str("g_counter 精确值"), px_eq(px_get_global("g_counter"), px_int(101LL))}, 2));
    px_srcline(52);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_add(px_add(px_str("结果: "), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_pass")}, 1)), px_str(" PASS / ")), px_call(px_get_global("str"), (LXValue[]){px_get_global("g_fail")}, 1)), px_str(" FAIL"))}, 1));
    return 0;
}
