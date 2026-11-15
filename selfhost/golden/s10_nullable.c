/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_maybe(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("maybe");
    if (nargs < 1) px_error("R1005: 调用 %s 参数不足: 需 %d 给 %d", "maybe", 1, nargs);
    if (nargs > 1) px_error("R1005: 调用 %s 参数过多: 最多 %d 给 %d", "maybe", 1, nargs);
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    px_srcpos("maybe", 15);
    return _v1;
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("maybe", px_func("maybe", fn_maybe, NULL));
    px_srcpos("<top>", 3);
    px_set_global("a", px_null());
    px_srcpos("<top>", 4);
    px_set_global("b", px_int(42LL));
    px_srcpos("<top>", 5);
    px_set_global("c", px_int(7LL));
    px_srcpos("<top>", 6);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("a==null:"), px_eq(px_get_global("a"), px_null())}, 2));
    px_srcpos("<top>", 7);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("b??0:"), ({ LXValue _t6 = px_get_global("b"); px_is_null(_t6) ? px_int(0LL) : _t6; })}, 2));
    px_srcpos("<top>", 8);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("b!=null:"), px_ne(px_get_global("b"), px_null())}, 2));
    px_srcpos("<top>", 9);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("c:"), px_get_global("c")}, 2));
    px_srcpos("<top>", 11);
    px_set_global("d", px_null());
    px_srcpos("<top>", 12);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("d==null:"), px_eq(px_get_global("d"), px_null())}, 2));
    px_srcpos("<top>", 16);
    (void)(({ LXValue _s1 = ({ LXValue _t8 = px_call(px_get_global("maybe"), (LXValue[]){px_null()}, 1); px_is_null(_t8) ? px_neg(px_int(1LL)) : _t8; }); LXValue _s2 = ({ LXValue _t9 = px_call(px_get_global("maybe"), (LXValue[]){px_int(9LL)}, 1); px_is_null(_t9) ? px_neg(px_int(1LL)) : _t9; }); px_call(px_get_global("print"), (LXValue[]){px_str("maybe:"), _s1, _s2}, 3); }));
    return px_exit_code_final(0);   // M120（qg-issue 76 E1）
}
