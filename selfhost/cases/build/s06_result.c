/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_try_div(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    if (px_is_truthy(px_eq(_v2, px_int(0LL)))) {
        return px_call(px_get_global("Err"), (LXValue[]){px_str("div0")}, 1);
    }
    return px_call(px_get_global("Ok"), (LXValue[]){px_idiv(_v1, _v2)}, 1);
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_calc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v4 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    LXValue _v7 = ({ LXValue _t6 = px_call(px_get_global("try_div"), (LXValue[]){_v4, px_int(2LL)}, 2); if (px_is_result(_t6)) { if (!px_result_ok(_t6)) { px_err_5_val = _t6; px_err_5_proped = 1; goto px_err_5; } _t6 = px_result_unwrap(_t6); } else if (px_is_null(_t6)) { px_err_5_val = px_null(); px_err_5_proped = 1; goto px_err_5; } _t6; });
    return px_call(px_get_global("Ok"), (LXValue[]){px_add(_v7, px_int(1LL))}, 1);
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_8_val = px_null();
    int px_err_8_proped = 0;
    LXValue _v9 = px_call(px_get_global("calc"), (LXValue[]){px_int(10LL)}, 1);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_method(_v9, "is_ok", (LXValue[]){}, 0)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_method(_v9, "unwrap", (LXValue[]){}, 0)}, 1));
    LXValue _v10 = px_call(px_get_global("try_div"), (LXValue[]){px_int(1LL), px_int(0LL)}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_method(_v10, "is_err", (LXValue[]){}, 0)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_method(_v10, "err", (LXValue[]){}, 0)}, 1));
    LXValue _v11 = px_call(px_get_global("Ok"), (LXValue[]){px_int(7LL)}, 1);
    (void)(px_call(px_get_global("print"), (LXValue[]){({ LXValue _t12 = _v11; if (px_is_result(_t12)) { if (!px_result_ok(_t12)) px_error("force unwrap Err"); _t12 = px_result_unwrap(_t12); } if (px_is_null(_t12)) px_error("force unwrap null"); _t12; })}, 1));
px_err_8:
    if (px_err_8_proped) return px_err_8_val;
    return px_null();
}

int main(void) {
    px_register_builtins();
    px_set_global("try_div", px_func("try_div", fn_try_div, NULL));
    px_set_global("calc", px_func("calc", fn_calc, NULL));
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
