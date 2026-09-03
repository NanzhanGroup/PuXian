#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_desc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2 = px_null();
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    _v2 = ({ LXValue _t4 = px_field(_v1, "kind"); if ((_t4.type == PX_ENUM && strcmp(_t4.as.obj->as.enum_inst.variant, "Int") == 0)) { _t4 = ({ LXValue _blk = px_null(); _blk = px_add(px_str("int:"), px_call(px_get_global("str"), (LXValue[]){px_field(_v1, "value")}, 1)); _blk; }); } else if ((_t4.type == PX_ENUM && strcmp(_t4.as.obj->as.enum_inst.variant, "Str") == 0)) { _t4 = ({ LXValue _blk = px_null(); _blk = px_str("str"); _blk; }); } else if ((_t4.type == PX_ENUM && strcmp(_t4.as.obj->as.enum_inst.variant, "Call") == 0)) { _t4 = ({ LXValue _blk = px_null(); _blk = px_str("call"); _blk; }); } _t4; });
    return _v2;
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v5 = px_null();
    LXValue _v6 = px_null();
    LXValue px_err_7_val = px_null();
    int px_err_7_proped = 0;
    _v5 = px_struct("Node", (char*[]){"kind", "value"}, (LXValue[]){px_enum("Kind", "Int"), px_int(42LL)}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("desc"), (LXValue[]){_v5}, 1)}, 1));
    _v6 = px_struct("Node", (char*[]){"kind", "value"}, (LXValue[]){px_enum("Kind", "Call"), px_int(0LL)}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_call(px_get_global("desc"), (LXValue[]){_v6}, 1)}, 1));
px_err_7:
    if (px_err_7_proped) return px_err_7_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("desc", px_func("desc", fn_desc, NULL));
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
