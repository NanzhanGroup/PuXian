#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = px_null();
    LXValue _v2 = px_null();
    LXValue _v3 = px_null();
    LXValue _v4 = px_null();
    LXValue _v5 = px_null();
    LXValue _v6 = px_null();
    LXValue _v7 = px_null();
    LXValue _v8 = px_null();
    LXValue px_err_9_val = px_null();
    int px_err_9_proped = 0;
    _v1 = px_int(9223372036854775807LL);
    _v2 = px_int(123LL);
    _v3 = px_float(inf);
    _v4 = px_float(250);
    _v5 = px_float(3.14);
    _v6 = px_str("�");
    _v7 = px_str("�");
    _v8 = px_str("�");
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v2}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v3}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v4}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v5}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v6}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v7}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){_v8}, 1));
px_err_9:
    if (px_err_9_proped) return px_err_9_val;
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
