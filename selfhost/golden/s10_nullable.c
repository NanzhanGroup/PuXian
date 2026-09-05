/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_maybe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("maybe");
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcline(15);
    return _v1;
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("maybe", px_func("maybe", fn_maybe, NULL));
    px_srcline(3);
    px_set_global("a", px_null());
    px_srcline(4);
    px_set_global("b", px_int(42LL));
    px_srcline(5);
    px_set_global("c", px_int(7LL));
    px_srcline(6);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("a==null:"), px_eq(px_get_global("a"), px_null())}, 2));
    px_srcline(7);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("b??0:"), ({ LXValue _t3 = px_get_global("b"); px_is_null(_t3) ? px_int(0LL) : _t3; })}, 2));
    px_srcline(8);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("b!=null:"), px_ne(px_get_global("b"), px_null())}, 2));
    px_srcline(9);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("c:"), px_get_global("c")}, 2));
    px_srcline(11);
    px_set_global("d", px_null());
    px_srcline(12);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("d==null:"), px_eq(px_get_global("d"), px_null())}, 2));
    px_srcline(16);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("maybe:"), ({ LXValue _t4 = px_call(px_get_global("maybe"), (LXValue[]){px_null()}, 1); px_is_null(_t4) ? px_neg(px_int(1LL)) : _t4; }), ({ LXValue _t5 = px_call(px_get_global("maybe"), (LXValue[]){px_int(9LL)}, 1); px_is_null(_t5) ? px_neg(px_int(1LL)) : _t5; })}, 3));
    return 0;
}
