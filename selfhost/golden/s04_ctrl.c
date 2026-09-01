#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = px_null();
    LXValue _v2 = px_null();
    LXValue _v3 = px_null();
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    LXValue _v5 = px_int(0LL);
    LXValue _t6 = px_call(px_get_global("range"), (LXValue[]){px_int(5LL)}, 1);
    for (int _t7 = 0; _t7 < px_len(_t6); _t7++) {
        LXValue _v8 = px_index(_t6, px_int(_t7));
         _v5 = px_add(_v5, _v8);
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v5}, 1));
    LXValue _v9 = px_int(0LL);
    while (px_is_truthy(px_lt(_v9, px_int(3LL)))) {
         _v9 = px_add(_v9, px_int(1LL));
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v9}, 1));
    LXValue _v10 = px_int(0LL);
    LXValue _t11 = px_call(px_get_global("range"), (LXValue[]){px_int(10LL)}, 1);
    for (int _t12 = 0; _t12 < px_len(_t11); _t12++) {
        LXValue _v13 = px_index(_t11, px_int(_t12));
        if (px_is_truthy(px_eq(_v13, px_int(2LL)))) {
            continue;
        }
        if (px_is_truthy(px_eq(_v13, px_int(5LL)))) {
            break;
        }
         _v10 = px_add(_v10, px_int(1LL));
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){_v10}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t14 = px_list(0); LXValue _t15 = px_call(px_get_global("range"), (LXValue[]){px_int(5LL)}, 1); for (int _t17=0; _t17<px_len(_t15); _t17++) { LXValue _t16 = px_index(_t15, px_int(_t17)); LXValue _cv18 = _t16; px_list_push(_t14, px_mul(_cv18, _cv18));  }  _t14; })}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t19 = px_list(0); LXValue _t20 = px_call(px_get_global("range"), (LXValue[]){px_int(6LL)}, 1); for (int _t22=0; _t22<px_len(_t20); _t22++) { LXValue _t21 = px_index(_t20, px_int(_t22)); LXValue _cv23 = _t21; if (px_is_truthy(px_eq(px_mod(_cv23, px_int(2LL)), px_int(0LL)))) { px_list_push(_t19, _cv23); }  }  _t19; })}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t24 = px_dict(); LXValue _t25 = px_call(px_get_global("range"), (LXValue[]){px_int(3LL)}, 1); for (int _t27=0; _t27<px_len(_t25); _t27++) { LXValue _t26 = px_index(_t25, px_int(_t27)); LXValue _cv28 = _t26; { LXValue _k = px_call(px_get_global("str"), (LXValue[]){_cv28}, 1); LXValue _v = _cv28; if (_k.type == PX_STR) px_dict_set(_t24, _k.as.obj->as.str.data, _v); }  }  _t24; })}, 1));
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

int main(void) {
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
      return _code;
    }
}
