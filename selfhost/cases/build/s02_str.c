/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    LXValue _v2 = px_str("hello,world");
    LXValue _v3 = px_call(px_get_global("split"), (LXValue[]){_v2, px_str(",")}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v3}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v3, px_int(0LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v3, px_int(1LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str("-"), _v3}, 2)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("to_upper"), (LXValue[]){_v2}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("trim"), (LXValue[]){px_str("  px  ")}, 1)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("replace"), (LXValue[]){_v2, px_str("world"), px_str("px")}, 3)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("contains"), (LXValue[]){_v2, px_str("wor")}, 2)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v2}, 1)}, 1));
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
