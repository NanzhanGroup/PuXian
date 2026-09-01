#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    LXValue _v2 = px_list_n((LXValue[]){px_int(3LL), px_int(1LL), px_int(2LL)}, 3);
    (void)(px_method(_v2, "append", (LXValue[]){px_int(4LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v2}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v2, px_int(0LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v2, px_neg(px_int(1LL)))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("sorted"), (LXValue[]){_v2}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("reversed"), (LXValue[]){_v2}, 1)}, 1));
    LXValue _v3 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("a"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(1LL)); } { LXValue _k = px_str("b"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(2LL)); } _d; });
    px_index_set(_v3, px_str("c"), px_int(3LL));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v3, px_str("c"))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_method(_v3, "has", (LXValue[]){px_str("a")}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("sorted"), (LXValue[]){px_method(_v3, "keys", (LXValue[]){}, 0)}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("sorted"), (LXValue[]){px_method(_v3, "values", (LXValue[]){}, 0)}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_method(_v3, "get", (LXValue[]){px_str("zz"), px_int(9LL)}, 2)}, 1));
px_err_1:
    if (px_err_1_proped) return px_err_1_val;
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
      return _code;
    }
}
