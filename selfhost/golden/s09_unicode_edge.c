/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
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
    px_srcline(3);
    _v1 = px_int(9223372036854775807LL);
    px_srcline(4);
    _v2 = px_int(123LL);
    px_srcline(5);
    _v3 = px_float(inf);
    px_srcline(6);
    _v4 = px_float(250);
    px_srcline(8);
    _v5 = px_float(3.14);
    px_srcline(9);
    _v6 = px_str("�");
    px_srcline(10);
    _v7 = px_str("�");
    px_srcline(11);
    _v8 = px_str("�");
    px_srcline(14);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v1}, 1));
    px_srcline(15);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v2}, 1));
    px_srcline(16);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v3}, 1));
    px_srcline(17);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v4}, 1));
    px_srcline(18);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v5}, 1));
    px_srcline(19);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v6}, 1));
    px_srcline(20);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v7}, 1));
    px_srcline(21);
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
