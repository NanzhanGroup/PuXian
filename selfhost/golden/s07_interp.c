#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    LXValue _v2 = px_str("px");
    LXValue _v3 = px_int(2LL);
    LXValue _v4 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(3LL)); } _d; });
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("hello "), px_call(px_get_global("str"), (LXValue[]){_v2}, 1)), px_str(""))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str("v"), px_call(px_get_global("str"), (LXValue[]){_v3}, 1)), px_str(""))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str(""), px_call(px_get_global("str"), (LXValue[]){px_add(px_int(1LL), px_mul(px_int(2LL), px_int(3LL)))}, 1)), px_str(""))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str(""), px_call(px_get_global("str"), (LXValue[]){px_index(_v4, px_str("k"))}, 1)), px_str(""))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_add(px_str(""), px_call(px_get_global("str"), (LXValue[]){px_method(_v2, "upper", (LXValue[]){}, 0)}, 1)), px_str(""))}, 1));
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
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
