/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v1 = px_null();
    LXValue _v2 = px_null();
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    px_srcline(3);
    _v1 = px_str("hello,world");
    px_srcline(4);
    _v2 = px_call(px_get_global("split"), (LXValue[]){_v1, px_str(",")}, 2);
    px_srcline(5);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v2}, 1)}, 1));
    px_srcline(6);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v2, px_int(0LL))}, 1));
    px_srcline(7);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v2, px_int(1LL))}, 1));
    px_srcline(8);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("join"), (LXValue[]){px_str("-"), _v2}, 2)}, 1));
    px_srcline(9);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("to_upper"), (LXValue[]){_v1}, 1)}, 1));
    px_srcline(10);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("trim"), (LXValue[]){px_str("  px  ")}, 1)}, 1));
    px_srcline(11);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("replace"), (LXValue[]){_v1, px_str("world"), px_str("px")}, 3)}, 1));
    px_srcline(12);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("contains"), (LXValue[]){_v1, px_str("wor")}, 2)}, 1));
    px_srcline(13);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v1}, 1)}, 1));
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
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
