#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v1 = px_uninit();
    LXValue _v2 = px_uninit();
    LXValue _v3 = px_uninit();
    LXValue _v4 = px_uninit();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(3);
    _v1 = px_int(0LL);
    px_srcline(4);
    LXValue _t6 = px_call(px_get_global("range"), (LXValue[]){px_int(5LL)}, 1);
    int _il1 = px_iter_prepare(_t6);
    for (int _t7 = 0; _t7 < _il1; _t7++) {
        px_iter_ck(_t6, _il1);
        _v2 = px_iter_at(_t6, px_int(_t7));
        px_srcline(5);
         _v1 = px_add(_v1, _v2);
    }
    px_srcline(6);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1}, 1));
    px_srcline(7);
    _v3 = px_int(0LL);
    px_srcline(8);
    while (px_is_truthy(px_lt(_v3, px_int(3LL)))) {
        px_srcline(9);
         _v3 = px_add(_v3, px_int(1LL));
    }
    px_srcline(10);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v3}, 1));
    px_srcline(11);
    _v4 = px_int(0LL);
    px_srcline(12);
    LXValue _t8 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1);
    int _il2 = px_iter_prepare(_t8);
    for (int _t9 = 0; _t9 < _il2; _t9++) {
        px_iter_ck(_t8, _il2);
        _v2 = px_iter_at(_t8, px_int(_t9));
        px_srcline(13);
        if (px_is_truthy(px_eq(_v2, px_int(2LL)))) {
            px_srcline(14);
            continue;
        }
        px_srcline(15);
        if (px_is_truthy(px_eq(_v2, px_int(5LL)))) {
            px_srcline(16);
            break;
        }
        px_srcline(17);
         _v4 = px_add(_v4, px_int(1LL));
    }
    px_srcline(18);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v4}, 1));
    px_srcline(19);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t10 = px_list(0); LXValue _t11 = px_call(px_get_global("range"), (LXValue[]){px_int(5LL)}, 1); for (int _il3 = px_iter_prepare(_t11), _t13=0; _t13<_il3; _t13++) { px_iter_ck(_t11, _il3); LXValue _t12 = px_iter_at(_t11, px_int(_t13)); LXValue _cv14 = _t12; px_list_push(_t10, px_mul(px_chk_uninit(_cv14, "x"), px_chk_uninit(_cv14, "x")));  }  _t10; })}, 1));
    px_srcline(20);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t15 = px_list(0); LXValue _t16 = px_call(px_get_global("range"), (LXValue[]){px_int(6LL)}, 1); for (int _il4 = px_iter_prepare(_t16), _t18=0; _t18<_il4; _t18++) { px_iter_ck(_t16, _il4); LXValue _t17 = px_iter_at(_t16, px_int(_t18)); LXValue _cv19 = _t17; if (px_is_truthy(px_eq(px_mod(px_chk_uninit(_cv19, "x"), px_int(2LL)), px_int(0LL)))) { px_list_push(_t15, px_chk_uninit(_cv19, "x")); }  }  _t15; })}, 1));
    px_srcline(21);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t20 = px_dict(); LXValue _t21 = px_call(px_get_global("range"), (LXValue[]){px_int(3LL)}, 1); for (int _il5 = px_iter_prepare(_t21), _t23=0; _t23<_il5; _t23++) { px_iter_ck(_t21, _il5); LXValue _t22 = px_iter_at(_t21, px_int(_t23)); LXValue _cv24 = _t22; { LXValue _k = px_call(px_get_global("str"), (LXValue[]){px_chk_uninit(_cv24, "x")}, 1); LXValue _v = px_chk_uninit(_cv24, "x"); px_dict_set_checked(_t20, _k, _v); }  }  _t20; })}, 1));
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("main", px_func("main", fn_main, NULL));
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
