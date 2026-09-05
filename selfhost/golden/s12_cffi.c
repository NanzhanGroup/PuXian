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
    LXValue px_err_5_val = px_null();
    int px_err_5_proped = 0;
    px_srcline(13);
    _v1 = px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_open"), px_list_n((LXValue[]){px_str(":memory:")}, 1)}, 2);
    px_srcline(14);
    (void)(px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_exec"), px_list_n((LXValue[]){_v1, px_str("CREATE TABLE t (id INT, name TEXT)")}, 2)}, 2));
    px_srcline(15);
    (void)(px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_exec"), px_list_n((LXValue[]){_v1, px_str("INSERT INTO t VALUES (1, '甲'), (2, '乙')")}, 2)}, 2));
    px_srcline(16);
    _v2 = px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_query"), px_list_n((LXValue[]){_v1, px_str("SELECT id, name FROM t ORDER BY id")}, 2)}, 2);
    px_srcline(17);
    LXValue _t6 = _v2;
    for (int _t7 = 0; _t7 < px_len(_t6); _t7++) {
        _v3 = px_index(_t6, px_int(_t7));
        px_srcline(18);
        (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v3, px_str("id")), px_index(_v3, px_str("name"))}, 2));
    }
    px_srcline(19);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("escape: "), px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_escape"), px_list_n((LXValue[]){px_str("O'Reilly")}, 1)}, 2))}, 1));
    px_srcline(20);
    _v4 = px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_close"), px_list_n((LXValue[]){_v1}, 1)}, 2);
    px_srcline(21);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("closed: "), px_call(px_get_global("str"), (LXValue[]){_v4}, 1))}, 1));
    px_srcline(22);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("ffi ok")}, 1));
px_err_5:
    if (px_err_5_proped) return px_err_5_val;
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
