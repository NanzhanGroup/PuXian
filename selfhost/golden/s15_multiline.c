#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_lookup(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    return px_index(_v1, px_str("k"));
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_add3(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v3 = (nargs > 0) ? args[0] : px_null();
    LXValue _v4 = (nargs > 1) ? args[1] : px_null();
    LXValue _v5 = (nargs > 2) ? args[2] : px_null();
    LXValue px_err_6_val = px_null();
    int px_err_6_proped = 0;
    return px_add(px_add(_v3, _v4), _v5);
px_err_6:
    if (px_err_6_proped) return px_err_6_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v7 = px_null();
    LXValue _v8 = px_null();
    LXValue _v9 = px_null();
    LXValue _v10 = px_null();
    LXValue _v11 = px_null();
    LXValue _v12 = px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    _v7 = px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){_v7}, 1)}, 1));
    _v8 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("x"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(10LL)); } { LXValue _k = px_str("y"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_int(20LL)); } _d; });
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_index(_v8, px_str("x")), px_index(_v8, px_str("y")))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("add3"), (LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL)}, 3)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("max"), (LXValue[]){px_int(8LL), px_int(2LL)}, 2)}, 1));
    _v9 = px_tuple((LXValue[]){px_int(1LL), px_int(2LL)}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_index(_v9, px_int(0LL)), px_index(_v9, px_int(1LL)))}, 1));
    _v10 = px_list_n((LXValue[]){px_int(10LL), px_int(20LL), px_int(30LL), px_int(40LL)}, 4);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v10, px_int(2LL))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_slice(_v10, px_int(1LL), px_int(3LL), px_null())}, 1)}, 1));
    _v11 = ({ LXValue _t14 = px_list(0); LXValue _t15 = px_list_n((LXValue[]){px_int(1LL), px_int(2LL), px_int(3LL), px_int(4LL), px_int(5LL), px_int(6LL)}, 6); for (int _t17=0; _t17<px_len(_t15); _t17++) { LXValue _t16 = px_index(_t15, px_int(_t17)); LXValue _cv18 = _t16; if (px_is_truthy(px_eq(px_mod(_cv18, px_int(2LL)), px_int(0LL)))) { px_list_push(_t14, _cv18); }  }  _t14; });
    (void)(px_call(px_get_global("print"), (LXValue[]){_v11}, 1));
    _v12 = px_call(px_get_global("lookup"), (LXValue[]){({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_int(5LL), px_int(6LL)}, 2)); } _d; })}, 1);
    (void)(px_call(px_get_global("print"), (LXValue[]){_v12}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("len"), (LXValue[]){px_call(px_get_global("lookup"), (LXValue[]){({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){px_int(7LL), px_int(8LL), px_int(9LL)}, 3)); } _d; })}, 1)}, 1)}, 1));
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("lookup", px_func("lookup", fn_lookup, NULL));
    px_set_global("add3", px_func("add3", fn_add3, NULL));
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
