#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_add(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2 = (nargs > 1) ? args[1] : px_null();
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    return px_add(_v1, _v2);
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_4_val = px_null();
    int px_err_4_proped = 0;
    LXValue _v5 = px_int(21LL);
    LXValue _v6 = px_call(px_get_global("add"), (LXValue[]){_v5, px_int(21LL)}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v6}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_mul(_v5, px_int(2LL)), px_int(1LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_sub(px_int(100LL), px_mul(px_int(3LL), px_int(30LL)))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_idiv(px_int(7LL), px_int(2LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_mod(px_int(7LL), px_int(3LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_pow(px_int(2LL), px_int(10LL))}, 1));
px_err_4:
    if (px_err_4_proped) return px_err_4_val;
    return px_null();
}

int main(void) {
    px_register_builtins();
    px_set_global("add", px_func("add", fn_add, NULL));
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
