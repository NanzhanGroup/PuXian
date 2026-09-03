#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_first(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    return px_index(_v1, px_int(0LL));
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_swap2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v3 = (nargs > 0) ? args[0] : px_null();
    LXValue _v4 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    return _v4;
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_max2(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v6 = (nargs > 0) ? args[0] : px_null();
    LXValue _v7 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_8_val = px_null();
    int px_err_8_proped = 0;
    if (px_is_truthy(px_gt(_v6, _v7))) {
        return _v6;
    }
    return _v7;
px_err_8:
    if (px_err_8_proped) return px_err_8_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("first", px_func("first", fn_first, NULL));
    px_set_global("swap2", px_func("swap2", fn_swap2, NULL));
    px_set_global("max2", px_func("max2", fn_max2, NULL));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("first-int:"), px_call(px_get_global("first"), (LXValue[]){px_list_n((LXValue[]){px_int(10LL), px_int(20LL), px_int(30LL)}, 3)}, 1)}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("first-str:"), px_call(px_get_global("first"), (LXValue[]){px_list_n((LXValue[]){px_str("甲"), px_str("乙"), px_str("丙")}, 3)}, 1)}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("first-nested:"), px_index(px_call(px_get_global("first"), (LXValue[]){px_list_n((LXValue[]){px_list_n((LXValue[]){px_int(1LL), px_int(2LL)}, 2), px_list_n((LXValue[]){px_int(3LL), px_int(4LL)}, 2)}, 2)}, 1), px_int(1LL))}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("swap:"), px_call(px_get_global("swap2"), (LXValue[]){px_int(1LL), px_str("one")}, 2)}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("max2-int:"), px_call(px_get_global("max2"), (LXValue[]){px_int(3LL), px_int(7LL)}, 2)}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("max2-str:"), px_call(px_get_global("max2"), (LXValue[]){px_str("香蕉"), px_str("苹果")}, 2)}, 2));
    px_set_global("bx", px_struct("Box", (char*[]){"value", "tag"}, (LXValue[]){px_int(42LL), px_str("盒子")}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("Box.value:"), px_field(px_get_global("bx"), "value")}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("Box.tag:"), px_field(px_get_global("bx"), "tag")}, 2));
    px_set_global("bx2", px_struct("Box", (char*[]){"value", "tag"}, (LXValue[]){px_str("内层"), px_str("s")}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("Box2.value:"), px_field(px_get_global("bx2"), "value")}, 2));
    px_set_global("pair", px_struct("Pair", (char*[]){"x", "y"}, (LXValue[]){px_int(1LL), px_str("一")}, 2));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("Pair:"), px_field(px_get_global("pair"), "x"), px_field(px_get_global("pair"), "y")}, 3));
    px_set_global("nums", px_list_n((LXValue[]){px_int(5LL), px_int(6LL), px_int(7LL)}, 3));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("typed-first:"), px_call(px_get_global("first"), (LXValue[]){px_get_global("nums")}, 1)}, 2));
    return 0;
}
